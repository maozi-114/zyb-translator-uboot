#ifndef __UDC_H__
#define __UDC_H__

#include <common.h>
#include <lcd.h>
#include <asm/io.h>
#include <asm/errno.h>

#ifdef CONFIG_UDC_SC9850
#include <../drivers/video/sprd/sprd_dphy.h>
#include <../drivers/video/sprd/sprd_dsi.h>
#include <../drivers/video/sprd/sprd_panel.h>
#endif

#ifdef CONFIG_UDC_SC7731E
#include <../drivers/video/sprd/sprd_dphy.h>
#include <../drivers/video/sprd/sprd_dsi.h>
#include <../drivers/video/sprd/sprd_panel.h>
#endif


#ifdef CONFIG_UDC_SC9832E
#include <../drivers/video/sprd/sprd_dphy.h>
#include <../drivers/video/sprd/sprd_dsi.h>
#include <../drivers/video/sprd/sprd_panel.h>
#endif

#ifdef CONFIG_UDC_SC9830
#include <../drivers/video/sprdfb/sprdfb_chip_common.h>
#include <../drivers/video/sprdfb/sprdfb.h>
#include <../drivers/video/sprdfb/sprdfb_panel.h>
#endif
#ifdef CONFIG_UDC_SC7731
//#include <asm/arch-sc8830/sprd_reg_base.h>

#include <../drivers/video/sprdfb/sprdfb_chip_common.h>
#include <../drivers/video/sprdfb/sprdfb.h>
#include <../drivers/video/sprdfb/sprdfb_panel.h>
#endif
#ifdef CONFIG_UDC_SC7731C
#include <../drivers/video/sprdfb/sprdfb_chip_common.h>
#include <../drivers/video/sprdfb/sprdfb.h>
#include <../drivers/video/sprdfb/sprdfb_panel.h>
#endif

#ifdef CONFIG_UDC_SC8825
#include <asm/arch/tiger_lcd.h>
#elif defined CONFIG_UDC_SC8810
#include <asm/arch/sc8810_lcd.h>
#elif defined CONFIG_UDC_SC7710
#include <asm/arch/sprd_lcd.h>
#endif

//#include <asm/arch/lcdc_reg_v3.h>
//#include <asm/arch/lcm_reg_v3.h>

#include <asm/arch-sc9833/mfp.h>
//#include <asm/arch/adi_hal_internal.h>
//#include <asm/arch/regs_ana.h>
//#include <asm/arch/analog_reg_v3.h>

//#include <asm/arch/sc8810_reg_ahb.h>
//#include <asm/arch/sc8810_reg_global.h>

//#include <asm/arch/gpio_drvapi.h>

//#include <asm/arch/regs_global.h>
//#include <asm/arch/regs_cpc.h>

#include <asm/io.h>
#include <asm/errno.h>
//#include <asm/arch/sc8810_reg_base.h>
//#include <asm/arch/lcdc_reg_v3.h>
//#include <asm/arch/lcm_reg_v3.h>

//#include <asm/arch-sc8830/mfp.h>
#include <config.h>
#include <common.h>
#include <linux/types.h>
//#include <asm/arch/bits.h>
#include <linux/string.h>
#include <android_bootimg.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <nand.h>
//#include <android_boot.h>
#include <environment.h>
#include <jffs2/jffs2.h>
#include <boot_mode.h>
#include <malloc.h>
//#include <lcd.h>
//#include <nand.h>
#ifdef CONFIG_EMMC_BOOT
//#include "../disk/part_uefi.h"
//#include "../drivers/mmc/card_sdio.h"
//#include <asm/arch/sci_types.h>
#include <asm/arch/common.h>

#include <ext_common.h>
#include <ext4fs.h>
#ifdef BOOTING_BACKUP_NVCALIBRATION
#include "../../property/backupnvitem.h"
#endif
#endif


#ifdef CONFIG_UDC_SC9832E
#include <../drivers/udc_sharkle/udc_base.h>
#else
#include <../drivers/udc/udc_base.h>	
#endif
#ifdef CONFIG_UDC_PINMAP
#include <../drivers/udc/udc_pinmap.h>
#endif

#ifdef CONFIG_UDC_BACKLIGHT
#include <../drivers/udc/udc_backlight.h>
#endif

#ifdef CONFIG_UDC_LCD
#ifdef CONFIG_UDC_SC9832E
#include <../drivers/udc_sharkle/udc_lcd.h>
#else
#include <../drivers/udc/udc_lcd.h>
#endif
#endif

#endif

 
