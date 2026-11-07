/*
 * Copyright (C) 2017 Spreadtrum Communications Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "sprd_panel.h"
#include "sprd_dsi.h"
#include "dsi/mipi_dsi_api.h"
#include "sprd_dphy.h"

/* init sequence extracted from vendor uboot binary (960 BOE panel, page 0x13) */
static uint8_t init_data[] = {
	0x39, 0x00, 0x00, 0x03, 0xe7, 0x44, 0x44,
	0x39, 0x00, 0x00, 0x11, 0xe8, 0x0e, 0xd5, 0x28, 0x8c, 0x10, 0xd7, 0x28, 0x8c, 0x0a, 0xd1, 0x28, 0x8c, 0x0c, 0xd3, 0x28, 0x8c,
	0x39, 0x00, 0x00, 0x07, 0xeb, 0x00, 0x01, 0xe4, 0xe4, 0x44, 0x00,
	0x39, 0x00, 0x00, 0x11, 0xed, 0xf3, 0xc1, 0xba, 0x0f, 0x66, 0x77, 0x44, 0x55, 0x55, 0x44, 0x77, 0x66, 0xf0, 0xab, 0x1c, 0x3f,
	0x39, 0x00, 0x00, 0x07, 0xef, 0x10, 0x0d, 0x04, 0x08, 0x3f, 0x1f,
	0x39, 0x00, 0x00, 0x06, 0xff, 0x77, 0x01, 0x00, 0x00, 0x13,
	0x39, 0x00, 0x00, 0x03, 0xe8, 0x00, 0x0e,
	0x05, 0x78, 0x00, 0x01, 0x11,
	0x39, 0x64, 0x00, 0x03, 0xe8, 0x00, 0x0c,
	0x39, 0x00, 0x00, 0x03, 0xe8, 0x40, 0x00,
	0x39, 0x00, 0x00, 0x06, 0xff, 0x77, 0x01, 0x00, 0x00, 0x00,
	0x15, 0x00, 0x00, 0x02, 0x36, 0x00,
	0x05, 0x78, 0x00, 0x01, 0x29,
	0x39, 0x00, 0x00, 0x06, 0xff, 0x77, 0x01, 0x00, 0x00, 0x10,
	0x15, 0x00, 0x00, 0x03, 0xe5, 0x00, 0x00,
	CMD_END
};

static int mipi_dsi_send_cmds(struct sprd_dsi *dsi, void *data)
{
	uint16_t len;
	struct dsi_cmd_desc *cmds = data;

	if ((cmds == NULL) || (dsi == NULL))
		return -1;

	for (; cmds->data_type != CMD_END;) {
		len = (cmds->wc_h << 8) | cmds->wc_l;
		mipi_dsi_gen_write(dsi, cmds->payload, len);
		if (cmds->wait)
			msleep(cmds->wait);
		cmds = (struct dsi_cmd_desc *)(cmds->payload + len);
	}
	return 0;
}

static int st7701_init(void)
{
	struct sprd_dsi *dsi = &dsi_device;
	struct sprd_dphy *dphy = &dphy_device;

	mipi_dsi_lp_cmd_enable(dsi, true);
	mipi_dsi_send_cmds(dsi, init_data);
	mipi_dsi_set_work_mode(dsi, SPRD_MIPI_MODE_VIDEO);
	mipi_dsi_state_reset(dsi);
	mipi_dphy_hs_clk_en(dphy, true);

	return 0;
}

static int st7701_readid(void)
{
	struct sprd_dsi *dsi = &dsi_device;
	uint8_t read_buf[4] = {0};
	int value = 0 ;

	mdelay(150);
	sprd_gpio_request(NULL, CONFIG_LCM_GPIO_ID);
	sprd_gpio_direction_input(NULL, CONFIG_LCM_GPIO_ID);
	value = sprd_gpio_get(NULL,CONFIG_LCM_GPIO_ID);

	if(value == 0x0) {
		pr_err("st7701 boe match value failed!\n");
		return -1;
	}

	mipi_dsi_lp_cmd_enable(dsi, true);
	mipi_dsi_set_max_return_size(dsi, 2);
	mipi_dsi_dcs_read(dsi, 0xa1, &read_buf[0], 2);

	if ((read_buf[0] == 0x88) && (read_buf[1] == 0x02)) {
		pr_info("st7701 boe read id success!\n");
		return 0x7701;	/* probe loop 需 >0; 0x7701=lcd_id, 与原厂 lcd_id=ID7701 一致 */
	}

	pr_err("st7701 boe read id failed!\n");
	return -1;
}

static int st7701_power(int on)
{
	if (on) {
		sprd_gpio_request(NULL, CONFIG_LCM_GPIO_RSTN);
		sprd_gpio_direction_output(NULL, CONFIG_LCM_GPIO_RSTN, 1);
		mdelay(50);
		sprd_gpio_direction_output(NULL, CONFIG_LCM_GPIO_RSTN, 0);
		mdelay(50);
		sprd_gpio_direction_output(NULL, CONFIG_LCM_GPIO_RSTN, 1);
		mdelay(150);
	} else {
		sprd_gpio_direction_output(NULL, CONFIG_LCM_GPIO_RSTN, 0);
		mdelay(5);
	}

	return 0;
}

static struct panel_ops st7701_ops = {
	.init = st7701_init,
	.read_id = st7701_readid,
	.power = st7701_power,
};

static struct panel_info st7701_info = {
	/* common parameters */
	.lcd_name = "lcd_st7701_boe_mipi_wvga",
	.type = SPRD_PANEL_TYPE_MIPI,
	.bpp = 24,
//	.fps = 3,
	.width = 480,
	.height = 960, /* from vendor binary: 480x960 (cmdline 960x480 = HxW) */

	/* DPI specific parameters */
	.pixel_clk = 32000000,
	.rgb_timing = {
		.hfp = 20, /* vendor: 20,20,8,20,20,8 */
		.hbp = 20,
		.hsync = 8,
		.vfp = 20,
		.vbp = 20,
		.vsync = 8,
	},

	/* MIPI DSI specific parameters */
	.phy_freq = 456000, /* vendor: 0x6f540 */
	.lane_num = 2,
	.work_mode = SPRD_MIPI_MODE_VIDEO,
	.burst_mode = PANEL_VIDEO_BURST_MODE,
	.nc_clk_en = false,
};

struct panel_driver st7701_boe_driver = {
	.info = &st7701_info,
	.ops = &st7701_ops,
};
