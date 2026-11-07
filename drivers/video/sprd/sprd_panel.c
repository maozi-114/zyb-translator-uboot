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
#include "lcd/panel_cfg.h"
#include "sprd_dsi.h"
#include "sprd_dphy.h"

static uint32_t lcd_id_to_kernel;
static struct panel_driver *panel_drv;

#ifdef CONFIG_BACKLIGHT_DSI
void set_backlight(int brightness)
{
	if (panel_drv && panel_drv->ops->set_brightness) {
		panel_drv->ops->set_brightness(brightness);
	}
	pr_info("lcd cabc backlight brightness==%d\n",brightness);
	return;
}
#endif

struct panel_info *panel_info_attach(void)
{
	return panel_drv->info;
}

uint32_t lcd_get_pixel_clock(void)
{
	if (panel_drv && panel_drv->info)
		return panel_drv->info->pixel_clk;
	else
		return 0;
}

uint32_t lcd_get_bpix(void)
{
	return panel_info.vl_bpix;
}

const char *lcd_get_name(void)
{
	if (panel_drv && panel_drv->info)
		return panel_drv->info->lcd_name;
	else
		return NULL;
}

uint32_t load_lcd_id_to_kernel(void)
{
	return lcd_id_to_kernel;
}

/* WORKAROUND: to keep the same order with sprdfb_panel.c */
uint32_t load_lcd_width_to_kernel(void)
{
	if (panel_drv && panel_drv->info)
		return panel_drv->info->height;
	else
		return 0;
}

/* WORKAROUND: to keep the same order with sprdfb_panel.c */
uint32_t load_lcd_hight_to_kernel(void)
{
	if (panel_drv && panel_drv->info)
		return panel_drv->info->width;
	else
		return 0;
}

static int panel_if_init(void)
{
	int type = panel_drv->info->type;

	switch (type) {
	case SPRD_PANEL_TYPE_MIPI:
		sprd_dsi_probe();
		sprd_dphy_probe();
		return 0;

	case SPRD_PANEL_TYPE_RGB:
		return 0;

	default:
		pr_err("doesn't support current interface type %d\n", type);
		return -1;
	}
}

static int panel_if_uinit(void)
{
	int type = panel_drv->info->type;

	switch (type) {
	case SPRD_PANEL_TYPE_MIPI:
		sprd_dphy_suspend(&dphy_device);
		sprd_dsi_suspend(&dsi_device);
		return 0;

	case SPRD_PANEL_TYPE_RGB:
		return 0;

	default:
		pr_err("doesn't support current interface type %d\n", type);
		return -1;
	}
}
#ifdef CONFIG_UDC_LCD

#if 0
static int panel_reset(int on)
{
	if (on) {
		sprd_gpio_request(NULL, CONFIG_LCM_GPIO_RSTN);
		sprd_gpio_direction_output(NULL, CONFIG_LCM_GPIO_RSTN, 1);
		mdelay(5);
		sprd_gpio_direction_output(NULL, CONFIG_LCM_GPIO_RSTN, 0);
		mdelay(5);
		sprd_gpio_direction_output(NULL, CONFIG_LCM_GPIO_RSTN, 1);
		mdelay(20);
	} else {
		sprd_gpio_direction_output(NULL, CONFIG_LCM_GPIO_RSTN, 0);
		mdelay(5);
	}

	return 0;
}
#endif

int sprd_panel_probe(void)
{
	struct panel_info *info;
	struct panel_ops *ops;
	//int i = 0;
	uint32_t id;	
	uint16_t sec_id = SEC_LCD0;
	uint16_t ret = 0;
	boot_mode_t boot_role;
	chipram_env_t* cr_env = get_chipram_env();
	boot_role = cr_env->mode;
	if(boot_role == BOOTLOADER_MODE_DOWNLOAD)
	{
		printf("%s: download mode\r\n");
		return NULL;
	}
	g_sc9850_udc_lcd = udc_lcd_create(SEC_LCD0, &lcd_panel[0]); 

	while(1)
	{        
		ret = udc_lcd_config_panel(g_sc9850_udc_lcd, sec_id++);
		
		panel_drv = lcd_panel[0].drv;
		info = panel_drv->info;
		ops = panel_drv->ops;
		
		
////////////////read id start/////////////////////////////////////////////		
		panel_if_init();

		if (ops && ops->power)
			ops->power(true);
  		//panel_reset(true);
		if(!ret)//not find lcd then config the last lcd to panle
		{
		  id = 0xffff;
		  break;
		}
		else
		{
		  id = ops->read_id();
		}

		if(id == lcd_panel[0].lcd_id) 
		{
		 break;
	 	}
		//panel_reset(false);
		if (ops && ops->power)
			ops->power(false);

		panel_if_uinit();

		pr_err("attach panel 0x%x failed, try next...\n",
			lcd_panel[0].lcd_id);
	}

	if (ops && ops->init)
		ops->init();

	panel_info.vl_row = info->height;
	panel_info.vl_col = info->width;
	lcd_id_to_kernel = lcd_panel[0].lcd_id;///supported_panel[i].lcd_id    add by jinq for script modify
	
//ave_lcd_id_to_kernel(lcd_panel[0].lcd_id);
//ave_lcd_size_to_kernel(info->height, info->width);

	return 0;
}
#else
int sprd_panel_probe(void)
{
	struct panel_info *info;
	struct panel_ops *ops;
	int ret;
	int i;

	for (i = 0; i < ARRAY_SIZE(supported_panel); i++) {
		panel_drv = supported_panel[i].drv;
		info = panel_drv->info;
		ops = panel_drv->ops;

		panel_if_init();

		if (ops && ops->power)
			ops->power(true);
		if (ops && ops->read_id) {
			ret = ops->read_id();
			if(ret > 0) {   //add by jinq for script modify
			//if (!ret) {
				pr_info("attach panel 0x%x success\n",
					supported_panel[i].lcd_id);
				break;
			}
		}
		if (ops && ops->power)
			ops->power(false);

		panel_if_uinit();

		pr_err("attach panel 0x%x failed, try next...\n",
			supported_panel[i].lcd_id);
	}

	if (ops && ops->init)
		ops->init();

	panel_info.vl_row = info->height;
	panel_info.vl_col = info->width;
	lcd_id_to_kernel = ret;///supported_panel[i].lcd_id    add by jinq for script modify

	return 0;
}

#endif
	

