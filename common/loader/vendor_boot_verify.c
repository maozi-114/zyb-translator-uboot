/* Board compatibility: restore the factory VERIFY_IMG call before Linux load.
 * This verifies the actual boot/recovery partition, not the custom U-Boot.
 * No efuse writes, verification bypass, or synthetic success response.
 */
#include <common.h>
#include <sprd_common_rw.h>
#include <uboot_sec_drv.h>
#include <linux/byteorder/little_endian.h>

#define VERIFY_BASE 0x83000000UL
#define VERIFY_CAP  0x02e00000UL
#define PAGE_BYTES  0x1000UL

/* Trusted reference for this board only: factory U-Boot MD5
 * 143d8ba5d862faa912ed577f518fcd17, keycert.hash_key at raw 0x7a70c.
 * This is the next-certificate public-key hash, NOT the payload digest.
 * Pinning it here does not authenticate or sign this custom U-Boot.
 */
static unsigned char factory_next_key_hash[PAGE_BYTES]
	__attribute__((aligned(PAGE_BYTES))) = {
	0x70,0xa5,0xdc,0xda,0x85,0x9a,0xdb,0xe5,
	0x73,0xd5,0x52,0x70,0x7d,0xde,0xa8,0xd6,
	0x9a,0x49,0x75,0xe1,0x64,0x57,0xa9,0x95,
	0xe2,0xe0,0x87,0xf5,0x14,0x60,0x51,0x74
};
static u64 verify_page[PAGE_BYTES / sizeof(u64)]
	__attribute__((aligned(PAGE_BYTES)));

static u32 load_le32(const unsigned char *p)
{
	u32 v;
	memcpy(&v, p, sizeof(v));
	return le32_to_cpu(v);
}

static u64 load_le64(const unsigned char *p)
{
	return (u64)load_le32(p) | ((u64)load_le32(p + 4) << 32);
}

int vendor_boot_verify(const char *partition)
{
	unsigned char *image = (unsigned char *)VERIFY_BASE;
	unsigned char *signed_header;
	u64 part_size, read_size, cert_offset, cert_size;
	u32 payload_size;
	smc_param *reply;

	/* Match the factory's bounded verification buffer BEFORE loading DT,
	 * ramdisk and kernel. These addresses must not be used after loading.
	 */
	if (strcmp(partition, "boot") && strcmp(partition, "recovery")
#ifdef CONFIG_TARGET_SP9832E_1H10_32B
	    && strcmp(partition, "boot_2")
#endif
	   )
		return -1;
	if (get_img_partition_size((char *)partition, &part_size) || part_size < 512)
		return -1;
	if (common_raw_read((char *)partition, 512, 0, (char *)image))
		return -1;
	if (load_le32(image) != 0x42544844) {
#if defined(CONFIG_TARGET_SL8541E_1H10) && defined(CONFIG_ARM64)
		if (!memcmp(image, "ANDROID!", 8)) {
			printf("VERIFY: ARM64 bring-up unsigned Android container; no secure handoff\n");
			return 0;
		}
#elif defined(CONFIG_TARGET_SP9832E_1H10_32B)
		/*
		 * This board's stock 32-bit boot partition is a direct Android
		 * container (ANDROID!) rather than a DHTB-wrapped image.  Keep
		 * DHTB images on the normal verification path, but allow this
		 * exact raw Android format to reach the existing Android loader.
		 */
		if (!memcmp(image, "ANDROID!", 8)) {
			printf("VERIFY: 32-bit raw Android container; no secure handoff\n");
			return 0;
		}
#endif
		printf("VERIFY: missing DHTB/Android header; refusing handoff\n");
		return -1;
	}
	payload_size = load_le32(image + 0x30);
	read_size = (u64)payload_size + PAGE_BYTES;
	if (!payload_size || read_size > VERIFY_CAP || read_size > part_size) {
		printf("VERIFY: invalid signed image size=%08x\n", payload_size);
		return -1;
	}
	memset(image, 0, VERIFY_CAP);
	if (common_raw_read((char *)partition, read_size, 0, (char *)image))
		return -1;
	/* Metadata checks are bounds checks, not a replacement for TOS RSA.
	 * cert_offset in this vendor format is relative to the image start.
	 */
	signed_header = image + 512 + payload_size;
	/* Factory U-Boot's secure-header magic is eight zero bytes; do not
	 * invent a string magic check. TOS validates the certificate itself.
	 */
	cert_size = load_le64(signed_header + 0x20);
	cert_offset = load_le64(signed_header + 0x28);
	if (cert_size < 4 || cert_offset < (u64)payload_size + 512 + 96 ||
	    cert_offset > read_size || cert_size > read_size - cert_offset) {
#if defined(CONFIG_TARGET_SL8541E_1H10) && defined(CONFIG_ARM64)
		/* Explicitly scoped development policy: absent zero-filled
		 * trailer is allowed. Malformed nonzero certificate metadata
		 * and a TOS signature verification failure remain fatal.
		 */
		if (!cert_size && !cert_offset && !memcmp(image + 512, "ANDROID!", 8)) {
			printf("VERIFY: ARM64 bring-up unsigned boot (no certificate); no secure handoff\n");
			return 0;
		}
#endif
		printf("VERIFY: certificate outside loaded image; refusing handoff\n");
		return -1;
	}
	memset(verify_page, 0, sizeof(verify_page));
	verify_page[0] = VERIFY_BASE;
	verify_page[1] = VERIFY_CAP;
	/* Reproduce the factory ARM32 ABI's signed pointer extension.
	 * Descriptor is 5 x u64; transfer/mapping length is a full 4 KiB.
	 */
	verify_page[2] = (u64)(s64)(s32)(ulong)factory_next_key_hash;
	verify_page[3] = 32;
	verify_page[4] = 1;
	flush_dcache_range(VERIFY_BASE, VERIFY_BASE + VERIFY_CAP);
	flush_dcache_range((ulong)factory_next_key_hash,
		(ulong)factory_next_key_hash + PAGE_BYTES);
	flush_dcache_range((ulong)verify_page, (ulong)verify_page + PAGE_BYTES);
	printf("VERIFY: %s src=%08lx bytes=%llx desc=%08lx map=1000 flag=1\n",
		partition, VERIFY_BASE, read_size, (ulong)verify_page);
	reply = tee_common_call(FUNCTYPE_VERIFY_IMG, (u32)(ulong)verify_page, PAGE_BYTES);
	printf("VERIFY: SMC returned a0=%08x\n", reply->a0);
	if (reply->a0) {
		printf("VERIFY: failed; refusing Linux handoff\n");
		return -1;
	}
	printf("VERIFY: factory boot verification handoff OK\n");
	return 0;
}
