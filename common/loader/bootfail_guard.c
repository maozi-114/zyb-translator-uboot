/*
 * bootfail_guard.c - consecutive boot-failure watchdog for SPRD u-boot
 *
 * Counts kernel boot attempts that happen within a short RTC time window.
 * If the kernel fails to boot, the device reboots and u-boot runs again
 * within seconds. When BOOTFAIL_LIMIT rapid attempts are seen, drop into
 * fastboot automatically so the host can re-flash.
 *
 * The counter record lives in the tail (last 512 bytes) of the "uboot"
 * partition: far outside the image covered by the DHTB mImgSize field,
 * so it never interferes with the boot image itself.
 */
#include <common.h>
#include <boot_mode.h>
#include <sprd_common_rw.h>
#include <mmc.h>
#include <part_efi.h>

#define BOOTFAIL_MAGIC		0x42464731	/* "BFG1" */
#define BOOTFAIL_WINDOW_SEC	120		/* gap below this = failed boot */
#define BOOTFAIL_LIMIT		3		/* consecutive failures -> fastboot */
#define BOOTFAIL_REC_SIZE	512		/* record size, tail of "uboot" */
#define BOOTFAIL_SECTOR		512		/* eMMC LBA size */

struct bootfail_rec {
	uint32_t magic;
	uint32_t count;
	uint32_t last_sec;
	uint32_t reserved[125];
};

extern unsigned long sprd_rtc_get_sec(void);

static uint64_t bootfail_rec_offset(void)
{
	block_dev_desc_t *dev;
	disk_partition_t info;

	dev = get_dev("mmc", 0);
	if (!dev)
		return (uint64_t)-1;
	if (get_partition_info_by_name(dev, "uboot", &info))
		return (uint64_t)-1;

	return (uint64_t)info.size * BOOTFAIL_SECTOR - BOOTFAIL_REC_SIZE;
}

boot_mode_enum_type bootfail_guard_check(boot_mode_enum_type mode)
{
	struct bootfail_rec rec;
	uint64_t off;
	uint32_t now, count = 0, gap = 0;

	/* only guard real kernel hand-off modes */
	if ((mode != CMD_NORMAL_MODE) && (mode != CMD_CHARGE_MODE))
		return mode;

	off = bootfail_rec_offset();
	if (off == (uint64_t)-1)
		return mode;	/* cannot locate storage: stay safe */

	memset(&rec, 0, sizeof(rec));
	if (common_raw_read("uboot", BOOTFAIL_REC_SIZE, off, (char *)&rec))
		return mode;

	now = (uint32_t)sprd_rtc_get_sec();

	if (rec.magic == BOOTFAIL_MAGIC) {
		gap = now - rec.last_sec;
		if (now == rec.last_sec) {
			/* RTC not ticking (or same second): keep old count,
			 * do not judge either way */
			count = rec.count;
			gap = 0;
		} else if ((int32_t)gap > 0 && gap < BOOTFAIL_WINDOW_SEC) {
			count = rec.count + 1;	/* rapid reboot: likely a crash */
		} else {
			count = 1;		/* long uptime / RTC reset: fresh */
		}
	} else {
		count = 1;
	}

	if (count >= BOOTFAIL_LIMIT) {
		printf("bootfail_guard: %d rapid reboots (gap %us), enter fastboot!\n",
			count, gap);
		rec.magic = BOOTFAIL_MAGIC;
		rec.count = 0;		/* reset so next boot starts fresh */
		rec.last_sec = now;
		memset(rec.reserved, 0, sizeof(rec.reserved));
		common_raw_write("uboot", BOOTFAIL_REC_SIZE, (uint64_t)0, off,
			(char *)&rec);
		return CMD_FASTBOOT_MODE;
	}

	debugf("bootfail_guard: attempt %d, gap %us\n", count, gap);

	rec.magic = BOOTFAIL_MAGIC;
	rec.count = count;
	rec.last_sec = now;
	memset(rec.reserved, 0, sizeof(rec.reserved));
	if (common_raw_write("uboot", BOOTFAIL_REC_SIZE, (uint64_t)0, off,
		(char *)&rec)) {
		/* counter write failed: do not act on stale state */
		return mode;
	}

	return mode;
}
