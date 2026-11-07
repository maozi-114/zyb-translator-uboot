#ifndef __SOC_SPRD_BOARD_H
#define __SOC_SPRD_BOARD_H
/* board-specific gpio/config for SL8541E-1H10 (from sprd-linux/kernel4.14) */
#define GPIO_TOUCH_RESET 145
#define GPIO_TOUCH_IRQ   144
#define ZCFG_LCM_WIDTH  480
#define ZCFG_LCM_HEIGHT 854
/* SL8541E uses TSX (32k-less) clock solution */
#define ZCFG_TSX_32KLESS
#endif
