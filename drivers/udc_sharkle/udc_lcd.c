/*
 * Copyright (C) 2010 Spreadtrum
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

#include <udc.h>



#define mdelay(a) udelay(a * 1000)

static int32_t udc_lcd_init(udc_panel *self);
static uint32_t udc_lcd_read_id(udc_panel*self);
static int32_t udc_lcd_set_window(udc_panel *self, uint16_t left, uint16_t top, uint16_t right, uint16_t bottom);
static int32_t udc_lcd_invalidate(udc_panel *self);
static int32_t udc_lcd_invalidate_rect(udc_panel *self, uint16_t left, uint16_t top, uint16_t right, uint16_t bottom);
static int32_t udc_lcd_set_direction(udc_panel *self, uint16_t direction);
static int32_t udc_lcd_enter_sleep(udc_panel *self, uint8_t is_sleep);
static int udc_lcd_set_pannel_type(udc_lcd* lcd, udc_t type);
static udc_t udc_lcd_get_pannel_type(udc_lcd* lcd);
static int udc_lcd_power(udc_panel *self,int on);


static udc_panel_operations lcd_udc_lcd_operations = {

#if defined (CONFIG_UDC_VER_4_0_1 ) || defined (CONFIG_UDC_VER_5_1_3 )	
	.panel_init = udc_lcd_init,
	.panel_set_window = udc_lcd_set_window,
	.panel_invalidate = udc_lcd_invalidate,
	.panel_invalidate_rect = udc_lcd_invalidate_rect,
	.panel_set_direction = udc_lcd_set_direction,
	.panel_enter_sleep = udc_lcd_enter_sleep,
	.panel_readid = udc_lcd_read_id,
	.power = udc_lcd_power,
#else
	.lcd_init = udc_lcd_init,
	.lcd_set_window = udc_lcd_set_window,
	.lcd_invalidate = udc_lcd_invalidate,
	.lcd_invalidate_rect = udc_lcd_invalidate_rect,
	.lcd_set_direction = udc_lcd_set_direction,
	.lcd_enter_sleep = udc_lcd_enter_sleep,
	.lcd_readid = udc_lcd_read_id,
	.power = udc_lcd_power,

#endif
};

udc_panel lcd_panel_udc_lcd = {
       .width = 480,
	.height = 800,
#if defined (CONFIG_UDC_VER_4_0_1 ) || defined (CONFIG_UDC_VER_5_1_3)	


#if defined (CONFIG_UDC_LCD_MCU)	
	.type = LCD_MODE_MCU,
#elif defined (CONFIG_UDC_LCD_RGB)	
       .fps = 58,
	.type = LCD_MODE_RGB,
#else
	.type = LCD_MODE_DSI,
	.fps = 60,
#endif

#elif defined (CONFIG_UDC_SC8810)
      .mode = LCD_MODE_MCU,
#endif
	.direction = LCD_DIRECT_NORMAL,
	.ops = &lcd_udc_lcd_operations,
};

static udc_item_s g_udc_lcd_item[] = {
	{LCD_FMARK               ,   0,   0},	
	{LCD_NAME               ,   0,   0},
	{LCD_ID               ,   0,   0},
	{LCD_WIDTH            ,   0,   0},
	{LCD_HIGHT            ,   0,   0},
	{LCD_MODE             ,   0,   0},
	{LCD_DIRECTION        ,   0,   0},
	{LCD_BUS_MODE         ,   0,   0},
	{LCD_BUS_WIDTH        ,   0,   0},
	{LCD_TIMING0          ,   0,   0},
	{LCD_TIMING1          ,   0,   0},
	{LCD_READ_ID          ,   0,   0},
	{LCD_INIT_PARA        ,   0,   0},
	{LCD_SET_WINDOW       ,   0,   0},
	{LCD_INVALIDATE_RECT  ,   0,   0},
	{LCD_DIR_NORMAL       ,   0,   0},
	{LCD_DIR_ROT90        ,   0,   0},
	{LCD_DIR_ROT180       ,   0,   0},
	{LCD_DIR_ROT270       ,   0,   0},
	{LCD_DIR_MIRH         ,   0,   0},
	{LCD_DIR_MIRV         ,   0,   0},
	{LCD_DIR_MIRHV        ,   0,   0},
	{LCD_ENTER_SLEEP      ,   0,   0},
	{LCD_EXIST_SLEEP      ,   0,   0},
	{LCD_WORK_MODE       ,   0,   0},
	{LCD_LAN_NUM         ,   0,   0},
	{LCD_PHY_FEQ         ,   0,   0},
	{LCD_H_SYNC_POL      ,   0,   0},
	{LCD_V_SYNC_POL      ,   0,   0},
	{LCD_DE_POL          ,   0,   0},
	{LCD_TE_POL          ,   0,   0},
	{LCD_COLOR_MODE_POL  ,   0,   0},
	{LCD_SHUT_DOWN_POL   ,   0,   0},
	{LCD_POWER_MODE      ,   0,   0},
	{LCD_READ_POWERMODE   ,   0,   0},
	{LCD_SPEED_MODE   ,   0,   0},
	{LCD_FPS   ,   0,   0},  
	{LCD_SUSPEND_MODE   ,   0,   0}, 
	{LCD_DSC_COMPRESSION   ,   0,   0},
	{LCD_POWER_ON_SEQUENCE   ,   0,	0},
	{LCD_POWER_OFF_SEQUENCE   ,   0,	0},
	{0XFFFF               ,   0,   0}
};


static udc_lcd g_udc_lcd;




#if defined (CONFIG_UDC_LCD_MCU)


static struct timing_mcu udc_lcd_mcu_timing[] = {
[0] = {    /* read/write register timing (ns) */
		.rcss = 25,  /* 25 ns */
		.rlpw = 70,
		.rhpw = 70,
		.wcss = 10,
		.wlpw = 50,
		.whpw = 50,
	},
[1] = {        /* read/write gram timing (ns) */
		.rcss = 25,
		.rlpw = 70,
		.rhpw = 70,
		.wcss = 0,
		.wlpw = 15,
		.whpw = 24,
	}
};

static struct info_mcu udc_lcd_info_mcu = {
       .bus_mode = LCD_BUS_8080,
	.bus_width = 24,
#if defined (CONFIG_UDC_VER_4_0_1 ) || defined (CONFIG_UDC_VER_5_1_3)
	.bpp = 24, /*RGB88*/
#endif	
	.timing = udc_lcd_mcu_timing,
	.ops = NULL,
};

static uint32_t udc_calc_lcm_cs0(udc_panel *panel)
{
	uint32_t reg_val = 0;

	/* CS0 bus mode [BIT0]: 8080/6800 */
	switch (panel->info.mcu->bus_mode) {
	case LCD_BUS_8080:

		break;
	case LCD_BUS_6800:
		reg_val  |= 1;
		break;
	default:
		break;
	}
	/* CS0 bus width [BIT2:1] */
	switch (panel->info.mcu->bus_width) {
	case 8:
		break;
	case 9:
		reg_val  |= ((1 << 1) | (1 << 4));
		break;
	case 16:
		reg_val  |= (2 << 1);
		break;
	case 18:
		reg_val  |= ((3 << 1) | (1 << 4));
		break;
	case 24:
		reg_val  |= ((4 << 1) | (2 << 4));
		break;
	default:
		reg_val  |= (2 << 1);
		break;
	}

      UDC_LCD_TRACE("sprd:[%s] cs0: reg_val = 0x%x\n", __FUNCTION__, reg_val);

	return reg_val;
}
static uint32_t udc_calc_lcm_cs1(udc_panel * panel)
{
	uint32_t reg_val = 0;

	UDC_LCD_TRACE("sprd:[%s]\n", __FUNCTION__);

	/* CS1 bus mode [BIT8]: 8080/6800 */
	switch (panel->info.mcu->bus_mode) {
	case LCD_BUS_8080:

		break;
	case LCD_BUS_6800:
		reg_val  |= (1 << 8);
		break;
	default:
		break;
	}
	/* CS1 bus width [BIT10:9] */
	switch (panel->info.mcu->bus_width) {
	case 8:
		break;
	case 9:
		reg_val  |= ((1 << 9) | (1 << 12));
		break;
	case 16:
		reg_val  |= (2 << 9);
		break;
	case 18:
		reg_val  |= ((3 << 9) | (1 << 12));
		break;
	case 24:
		reg_val  |= ((4 << 9) | (2 << 12));
		break;
	default:
		reg_val  |= (2 << 9);
		break;

	}
	reg_val  |= (1 << 16);

	 UDC_LCD_TRACE("sprd:[%s] cs1: reg_val = 0x%x\n", __FUNCTION__, reg_val);

	return reg_val;
}

static udc32_t udc_config_lcm(udc_lcd *lcd)
{
	uint32_t reg_val = 0;
	uint16_t cs = lcd->lcd_cs;
	
	udc_panel* panel = lcd->cfg->panel;

	if( 1 == cs){
		reg_val = udc_calc_lcm_cs1(panel);
	}
	else{
		reg_val = udc_calc_lcm_cs0(panel);
	}
	udc_lcd_writel(reg_val, LCM_CTRL);

	UDC_LCD_TRACE("[%s] LCM_CTRL: 0x%x\n", __FUNCTION__, __raw_readl(LCM_CTRL));
	return reg_val;
}

static void udc_update_lcm(udc_lcd *lcd, uint32_t value)
{
        volatile uint32_t reg = 0;

	 uint16_t cs = lcd->lcd_cs ;
	
	
	
	if (1 == cs) {
		reg = LCM_PARAMETER1;
	}
	else{
		reg = LCM_PARAMETER0;
	}
	udc_lcd_writel(value, reg); 
	UDC_LCD_TRACE("[%s] reg : 0x%x = 0x%x\n", __FUNCTION__, reg, udc_lcd_readl(reg));
		
	
}
void udc_lcd_set_fmark(struct _udc_lcd* lcd)
{
	uint32_t reg_val = 0;
	uint16_t lcd_fmark = lcd->lcd_fmark & 0x3;
	reg_val = udc_lcd_readl(LCDC_CTRL);
	reg_val &= ~(0x3<< 1) ;  //clear bit1,bit2
	/*FMARK mode*/
	reg_val |= (lcd_fmark<<1);  

	 udc_lcd_writel(reg_val, LCDC_CTRL);
	   
	UDC_LCD_TRACE("[%s] reg_val =0x%x, LCDC_CTRL: 0x%x\n", __func__, reg_val, udc_lcd_readl(LCDC_CTRL));

}


static int udc_lcd_config_lcm_timing_mcu(udc_t key_id, struct timing_mcu *timing)
{
	unsigned short mcu_timing[6];
	
	if (  udc_lcd_get_value(key_id, mcu_timing, 6) )
	{
		timing->rcss = mcu_timing[0];
		timing->rlpw = mcu_timing[1];
		timing->rhpw = mcu_timing[2];
		timing->wcss = mcu_timing[3];
		timing->wlpw = mcu_timing[4];
		timing->whpw = mcu_timing[5];
		UDC_LCD_TRACE("key_id = %d rcss %d, rlpw %d, rhpw %d, wcss %d, wlpw %d, whpw %d\n", key_id,
		timing->rcss, timing->rlpw,	timing->rhpw, timing->wcss, timing->wlpw, timing->whpw);
		return 1;
	}
	return 0;
	
}

static int udc_lcd_config_mcu_panel(udc_lcd* lcd)
{     
       udc_t param = 0;
       udc_panel_cfg *cfg = lcd->cfg;
	struct info_mcu *mcu = NULL;
	
	cfg->panel->info.mcu  = &udc_lcd_info_mcu;
       mcu = cfg->panel->info.mcu;	

       if ( udc_lcd_get_value(LCD_FMARK, &param, 1) )
		lcd->lcd_fmark = param;
   	
	if ( udc_lcd_get_value(LCD_BUS_MODE, &param, 1) )
		mcu->bus_mode = param;

	if ( udc_lcd_get_value(LCD_BUS_WIDTH, &param, 1) )
		mcu->bus_width= param;

#if defined (CONFIG_UDC_VER_4_0_1 ) || defined (CONFIG_UDC_VER_5_1_3)
	if (16 == mcu->bus_width)
		mcu->bpp= param;
	UDC_LCD_TRACE("bpp 0x%x\n", mcu->bpp);
#endif
   	
	UDC_LCD_TRACE("lcd_fmark: 0x%x\n", param);
    UDC_LCD_TRACE("bus_mode: %d\n", mcu->bus_mode);
    UDC_LCD_TRACE("bus_width 0x%x\n", mcu->bus_width);
	

	 udc_lcd_config_lcm_timing_mcu(LCD_TIMING0, &(mcu->timing[0]));
	
	if  ( !udc_lcd_config_lcm_timing_mcu(LCD_TIMING1, &(mcu->timing[1])) )
		memcpy( &(mcu->timing[1]), &(mcu->timing[0]),sizeof(struct timing_mcu));
	
		
	lcd->set_fmark(lcd);		


	return 1;

}


static uint32_t udc_lcd_mcu_do(struct ops_mcu *ops, unsigned short *value, unsigned short value_count,
		                                          uint16_t left, uint16_t top,uint16_t right,uint16_t bottom)
{
	int i;
	unsigned short *ptr;
	uint16_t cmd, param, shift, mask;
	uint32_t ret_data = 0;
	uint32_t data = 0;
	uint32_t read_data = 0;

      UDC_LCD_TRACE("%s: value = 0x%x, value_count = %d\n", __func__, *value, value_count);
	for (i=0,ptr=value; i<value_count; i+=2, ptr++) {

		cmd = *ptr++;
		switch (cmd&0xf000) {
		case UDC_LCD_SEND_CMD:
			ops->send_cmd(*ptr);
			UDC_LCD_TRACE("send_cmd 0x%X\n", *ptr);
			break;
		case UDC_LCD_SEND_DAT:
			shift = cmd&0x003f;
			mask = *ptr;
			switch (cmd&0x0f00) {
			case UDC_LCD_DIR_NONE:
				param = *ptr;
				break;
			case UDC_LCD_DIR_LEFT:
				param = (left>>shift)&mask;
				break;
			case UDC_LCD_DIR_TOP:
				param = (top>>shift)&mask;
				break;
			case UDC_LCD_DIR_RIGHT:
				param = (right>>shift)&mask;
				break;
			case UDC_LCD_DIR_BOTTOM:
				param = (bottom>>shift)&mask;
				break;
			default:
				UDC_LCD_TRACE( "%s invalid config data 0x%x 0x%x\n", __func__, cmd, *ptr);
				break;
			}
			ops->send_data(param);
			UDC_LCD_TRACE("send_data 0x%X\n", param);
			break;
		case UDC_LCD_DELAY_MS:
			LCD_DelayMS(*ptr);
			UDC_LCD_TRACE("LCD_DelayMS %d\n", *ptr);
			break;
		case UDC_LCD_READ_DAT:
			shift = cmd & 0x003f;
			mask = *ptr;

                     read_data = ops->read_data();
					 
			data =  (read_data & mask) << shift;
			
			ret_data |= data;
			
			UDC_LCD_TRACE("%s(): mask = 0x%x, shift = 0x%x,  data = 0x%x, read_param = 0x%x\n", __func__, mask, shift, data, ret_data);
			
			break;
                case UDC_LCD_RETURN_ID:
				ret_data = *ptr;
				UDC_LCD_TRACE("send_cmd 0x%X\n", *ptr);
				break;
		default:
			UDC_LCD_TRACE( "%s invalid config data 0x%x 0x%x\n", __func__, cmd, *ptr);
			break;
		}
	}
	
	return ret_data;
}

#endif


#if defined (CONFIG_UDC_LCD_RGB)

static struct timing_rgb udc_lcd_rgb_timing = {
	.hfp = 10,  /* unit: pixel */
	.hbp = 6,
	.hsync = 8,
	.vfp = 3, /*unit: line*/
	.vbp = 3,
	.vsync = 4,
};

static struct spi_info udc_lcd_info_rgb_spi = {
	.ops = NULL,
};

static struct info_rgb udc_lcd_info_rgb = {
	.cmd_bus_mode  = SPRDFB_RGB_BUS_TYPE_SPI,
	.video_bus_width = 24, /*18,16*/
	.h_sync_pol = SPRDFB_POLARITY_NEG,
	.v_sync_pol = SPRDFB_POLARITY_NEG,
	.de_pol = SPRDFB_POLARITY_POS,
	.timing = &udc_lcd_rgb_timing,
	.bus_info = {
		.spi = &udc_lcd_info_rgb_spi,
	}
};

static int udc_lcd_config_lcm_timing_rgb(udc_t section_id, struct timing_rgb *timing)
{
	unsigned short rgb_timing[6];
	if (udc_lcd_get_value(section_id, rgb_timing, 6) )
	{
		timing->hfp      = rgb_timing[0];
		timing->hbp     = rgb_timing[1];
		timing->hsync  = rgb_timing[2];
		timing->vfp      = rgb_timing[3];
		timing->vbp     = rgb_timing[4];
		timing->vsync  = rgb_timing[5];
		UDC_LCD_TRACE("id = %d hfp %d, hbp %d, hsync %d, vfp %d, vbp %d, vsync %d\n", section_id,
			timing->hfp, timing->hbp,	timing->hsync, timing->vfp, timing->vbp, timing->vsync);
		return 1;
	}
	return 0;
}

static int udc_lcd_config_rgb_panel(udc_lcd* lcd)
{     
       udc_t param = 0;
       udc_panel_cfg *cfg = lcd->cfg;
	struct info_rgb *rgb = NULL;

	cfg->panel->info.rgb  = &udc_lcd_info_rgb;
	rgb = cfg->panel->info.rgb;

          if ( udc_lcd_get_value(LCD_BUS_MODE, &param, 1) )
          {
          		switch(param)
          		{
			case 2:
				rgb->cmd_bus_mode = SPRDFB_RGB_BUS_TYPE_SPI;
				break;
			case 3:
				rgb->cmd_bus_mode = SPRDFB_RGB_BUS_TYPE_I2C;
				break;
			default:
				rgb->cmd_bus_mode = param;
				break;
			
			}
	  }			
		
	
	if ( udc_lcd_get_value(LCD_BUS_WIDTH, &param, 1) )
		rgb->video_bus_width = param;

	if ( udc_lcd_get_value(LCD_H_SYNC_POL, &param, 1) )
		rgb->h_sync_pol = param;

	if ( udc_lcd_get_value(LCD_V_SYNC_POL, &param, 1) )
		rgb->v_sync_pol = param;
	
	if ( udc_lcd_get_value(LCD_DE_POL, &param, 1) )
		rgb->de_pol = param;

       UDC_LCD_TRACE("rgb->cmd_bus_mode 0x%x\n", rgb->cmd_bus_mode);
 
	UDC_LCD_TRACE("rgb->video_bus_width 0x%x\n", rgb->video_bus_width);

	UDC_LCD_TRACE("rgb->h_sync_pol 0x%x\n", rgb->h_sync_pol);	

	UDC_LCD_TRACE("rgb->v_sync_pol 0x%x\n", rgb->v_sync_pol);

	UDC_LCD_TRACE("rgb->de_pol 0x%x\n", rgb->de_pol);

	udc_lcd_config_lcm_timing_rgb(LCD_TIMING0, rgb->timing);

	return 1;

}

static uint32_t udc_lcd_rgb_do(struct ops_spi *ops, unsigned short *value, unsigned short value_count,
		                                        uint16_t left, uint16_t top,uint16_t right,uint16_t bottom)
{
	int i;
	unsigned short *ptr;
	uint16_t cmd, param, shift, mask;
	uint32_t ret_data = 0;
	uint32_t data = 0;
	uint32_t read_data = 0;
	spi_send_cmd_t spi_send_cmd = ops->spi_send_cmd; 
	spi_send_data_t spi_send_data = ops->spi_send_data; 
	spi_read_t spi_read = ops->spi_read; 
	
      UDC_LCD_TRACE("%s: value = 0x%x, value_count = %d,  ops = 0x%x \n", __func__, *value, value_count, ops);
	for (i=0,ptr=value; i<value_count; i+=2, ptr++) {

		cmd = *ptr++;
		switch (cmd&0xf000) {
		case UDC_LCD_SEND_CMD:
			spi_send_cmd( *ptr);
			UDC_LCD_TRACE("send_cmd 0x%X\n", *ptr);
			break;
		case UDC_LCD_SEND_DAT:
			shift = cmd&0x003f;
			mask = *ptr;
			switch (cmd&0x0f00) {
			case UDC_LCD_DIR_NONE:
				param = *ptr;
				break;
			case UDC_LCD_DIR_LEFT:
				param = (left>>shift)&mask;
				break;
			case UDC_LCD_DIR_TOP:
				param = (top>>shift)&mask;
				break;
			case UDC_LCD_DIR_RIGHT:
				param = (right>>shift)&mask;
				break;
			case UDC_LCD_DIR_BOTTOM:
				param = (bottom>>shift)&mask;
				break;
			default:
				UDC_LCD_TRACE( "%s invalid config data 0x%x 0x%x\n", __func__, cmd, *ptr);
				break;
			}
			spi_send_data( param );
			UDC_LCD_TRACE("send_data 0x%X\n", param);
			break;
		case UDC_LCD_DELAY_MS:
			LCD_DelayMS(*ptr);
			UDC_LCD_TRACE("LCD_DelayMS %d\n", *ptr);
			break;
		case UDC_LCD_READ_DAT:
			shift = cmd & 0x003f;
			mask = *ptr;

                      spi_read(&read_data);
					 
			data =  (read_data & mask) << shift;
			
			ret_data |= data;
			
			UDC_LCD_TRACE("%s(): mask = 0x%x, shift = 0x%x,  data = 0x%x, read_param = 0x%x\n", __func__, mask, shift, data, ret_data);
			
			break;
                 case UDC_LCD_RETURN_ID:
				ret_data = *ptr;
				UDC_LCD_TRACE("****send_cmd 0x%X\n", *ptr);
				break;
		default:
			UDC_LCD_TRACE( "%s invalid config data 0x%x 0x%x\n", __func__, cmd, *ptr);
			break;
		}
	}
	
	return ret_data;
}

#endif


#ifdef CONFIG_UDC_LCD_MIPI

static struct timing_rgb udc_lcd_timing_mipi = {
	.hfp = 20,  /* unit: pixel */
	.hbp = 20,
	.hsync = 4,
	.vfp = 10, /*unit: line*/
	.vbp = 10,
	.vsync = 6,
};

static struct info_mipi lcd_udc_lcd_info_mipi = {
	.work_mode  = SPRDFB_MIPI_MODE_CMD,
	.video_bus_width = 24, /*18,16*/
	.lan_number = 2,
	.phy_feq = 500*1000,
	.h_sync_pol = SPRDFB_POLARITY_POS,
	.v_sync_pol = SPRDFB_POLARITY_POS,
	.de_pol = SPRDFB_POLARITY_POS,
	.te_pol = SPRDFB_POLARITY_POS,
	.color_mode_pol = SPRDFB_POLARITY_NEG,
	.shut_down_pol = SPRDFB_POLARITY_NEG,
	.timing = &udc_lcd_timing_mipi,
	.ops = NULL,
};

static int udc_lcd_config_lcm_timing_mipi(udc_t key_id, struct timing_rgb *timing)
{
	unsigned short mipi_timing[6];
	if (udc_lcd_get_value(key_id, mipi_timing, 6) )
	{
		timing->hfp = mipi_timing[0];
		timing->hbp = mipi_timing[1];
		timing->hsync = mipi_timing[2];
		timing->vfp = mipi_timing[3];
		timing->vbp = mipi_timing[4];
		timing->vsync = mipi_timing[5];
		UDC_LCD_TRACE("id = %d hfp %d, hbp %d, hsync %d, vfp %d, vbp %d, vsync %d\n", key_id,
			timing->hfp, timing->hbp,	timing->hsync, timing->vfp, timing->vbp, timing->vsync);
		return 1;
	}
	return 0;
}

static int udc_lcd_config_mipi_panel(udc_lcd* lcd)
{     
       udc_t param = 0;
       udc_panel_cfg *cfg = lcd->cfg;
	struct info_mipi *mipi = NULL;
      
       cfg->panel->info.mipi  = &lcd_udc_lcd_info_mipi;
       mipi = cfg->panel->info.mipi;		

      if ( udc_lcd_get_value(LCD_WORK_MODE, &param, 1) )
		mipi->work_mode  = param;
	UDC_LCD_TRACE("work_mode 0x%x\n", mipi->work_mode);

	if ( udc_lcd_get_value(LCD_BUS_WIDTH, &param, 1) )
		mipi->video_bus_width = param;
	UDC_LCD_TRACE("video_bus_width 0x%x\n", mipi->video_bus_width);
	
	if ( udc_lcd_get_value(LCD_LAN_NUM, &param, 1) )
		mipi->lan_number = param;
	UDC_LCD_TRACE("lan_number 0x%x\n", mipi->lan_number);
	
	if ( udc_lcd_get_value(LCD_PHY_FEQ, &param, 1) )
		mipi->phy_feq = param*1000;
	UDC_LCD_TRACE("phy_feq 0x%x\n", mipi->phy_feq);

	if ( udc_lcd_get_value(LCD_H_SYNC_POL, &param, 1) )
		mipi->h_sync_pol = param;
	UDC_LCD_TRACE("h_sync_pol 0x%x\n", mipi->h_sync_pol);	

	if ( udc_lcd_get_value(LCD_V_SYNC_POL, &param, 1) )
		mipi->v_sync_pol = param;
	UDC_LCD_TRACE("v_sync_pol 0x%x\n", mipi->v_sync_pol);
	
	if ( udc_lcd_get_value(LCD_DE_POL, &param, 1) )
		mipi->de_pol = param;
	UDC_LCD_TRACE("de_pol 0x%x\n", mipi->de_pol);
	
	if ( udc_lcd_get_value(LCD_TE_POL, &param, 1) )
		mipi->te_pol = param;
	UDC_LCD_TRACE("te_pol 0x%x\n", mipi->te_pol);

	if ( udc_lcd_get_value(LCD_COLOR_MODE_POL, &param, 1) )
		mipi->color_mode_pol = param;
	UDC_LCD_TRACE("color_mode_pol 0x%x\n", mipi->color_mode_pol);
	
	if ( udc_lcd_get_value(LCD_SHUT_DOWN_POL, &param, 1) )
		mipi->shut_down_pol = param;
	UDC_LCD_TRACE("shut_down_pol 0x%x\n", mipi->shut_down_pol);

	if ( udc_lcd_get_value(LCD_POWER_MODE, &param, 1) )
		lcd->power_mode = param;

	UDC_LCD_TRACE("power_mode 0x%x\n", lcd->power_mode);
	
	udc_lcd_config_lcm_timing_mipi(LCD_TIMING0, &(mipi->timing[0]));

	
	
	
	return 1;

}

static uint32_t udc_lcd_mipi_do(struct ops_mipi *ops, unsigned short *value, unsigned short value_count,
		uint16_t left, uint16_t top,uint16_t right,uint16_t bottom)
{
	#define MAX_DATA   100
	unsigned int tag = 0;
	uint32_t i=0, j=0, x=0;
	uint8_t read_data[MAX_DATA];
	uint8_t data[MAX_DATA];
	uint16_t cmd,len;
	uint8_t datatype;
	unsigned int items;
	struct gpio_timing *timing;
	unsigned int *p;
	mipi_set_cmd_mode_t mipi_set_cmd_mode = ops->mipi_set_cmd_mode;
	mipi_force_write_t mipi_force_write = ops->mipi_force_write;
	mipi_gen_write_t mipi_gen_write = ops->mipi_gen_write;
	mipi_force_read_t mipi_force_read = ops->mipi_force_read;
	uint16_t lcd_speed_mode = 0;
	UDC_LCD_TRACE("%s: value = 0x%x, value_count = %d\n", __func__, *value, value_count);
	
  for (i=0; i<value_count;) 
	{
		cmd = value[i];

		if(UDC_LCD_POWER_ON == cmd)
		{
			items = g_udc_lcd.pwr_on_seq.items;
			timing = g_udc_lcd.pwr_on_seq.timing;
			items= value[i+1]/3;
			g_udc_lcd.pwr_on_seq.items=items;
			p = kzalloc(value[i+1]*sizeof(unsigned int), GFP_KERNEL);
			if (!p) {
				UDC_LCD_TRACE("error udc_lcd_mipi_do 1 alloc interface fail\n");
			}

			for(j = 0; j < value[i+1]; j++)
			{
				p[j] = (uint8_t)value[i+2+j];
				UDC_LCD_TRACE("UDC_LCD_POWER_ON  data[%d]= %d\n",j,p[j]);
			}
			(g_udc_lcd.pwr_on_seq.timing)=(struct gpio_timing *)p;
			i =value[i+1]+2;
			
			for (x = 0; x < items; x++) {
				UDC_LCD_TRACE("CONTRL GPIO gpio = %d, level = %d, delay = %d\n",
						g_udc_lcd.pwr_on_seq.timing[x].gpio,g_udc_lcd.pwr_on_seq.timing[x].level,g_udc_lcd.pwr_on_seq.timing[x].delay);
				sprd_gpio_request(NULL,g_udc_lcd.pwr_on_seq.timing[x].gpio);
				sprd_gpio_direction_output(NULL,g_udc_lcd.pwr_on_seq.timing[x].gpio, g_udc_lcd.pwr_on_seq.timing[x].level);
				mdelay(g_udc_lcd.pwr_on_seq.timing[x].delay);
			}
			//kfree(p);

		}
		
		if(UDC_LCD_POWER_OFF == cmd)
		{
			items = g_udc_lcd.pwr_off_seq.items;
			timing = g_udc_lcd.pwr_off_seq.timing;
			items= value[i+1]/3;
			g_udc_lcd.pwr_off_seq.items=items;
			p = kzalloc(value[i+1]*sizeof(unsigned int), GFP_KERNEL);
			if (!p) {
				UDC_LCD_TRACE("error udc_lcd_mipi_do 2 alloc interface fail\n");
			}
			
			for(j = 0; j < value[i+1]; j++)
			{
				p[j] = (uint8_t)value[i+2+j];
				UDC_LCD_TRACE("UDC_LCD_POWER_OFF  data[%d]= %d\n",j,p[j]);
			}
			(g_udc_lcd.pwr_off_seq.timing)=(struct gpio_timing *)p;
			i =value[i+1]+2;
			
			for (x = 0; x < items; x++) {
				sprd_gpio_direction_output(NULL,g_udc_lcd.pwr_on_seq.timing[x].gpio, g_udc_lcd.pwr_on_seq.timing[x].level);
				mdelay(g_udc_lcd.pwr_on_seq.timing[x].delay);
			}
			//kfree(p);
		}

		
		if(UDC_LCD_MIPI_SET_LP_MODE == cmd)
		{
		ops->mipi_set_lp_mode();
			i += 1;
		}
  
		if(UDC_LCD_MIPI_SET_HS_MODE == cmd)
		{
			ops->mipi_set_hs_mode();
			i += 1;
		}
        
		if(UDC_LCD_MIPI_SET_CMD_MODE == cmd)
		{
			ops->mipi_set_cmd_mode();
			i += 1;
		}		
		if(UDC_LCD_MIPI_EOTP_SET == cmd)
		{
		    UDC_LCD_TRACE("param1 = %d,param2 = %d\n",value[i+1],value[i+2]);
			ops->mipi_eotp_set(value[i+1],value[i+2]);
			i += 3;
		}
		if(UDC_LCD_RETURN_ID == cmd)
		{	 
			read_data[0] = value[i+1];
			UDC_LCD_TRACE("return id 0x%X\n", read_data[0]);
		}
		if(UDC_LCD_MIPI_FORCE_WRITE ==cmd)
		{
			len = value[i+2];
			datatype = (uint8_t)value[i+1];
			for(j = 0; j < len; j++)
			{
				data[j] = (uint8_t)value[i+3+j];
			}
			UDC_LCD_TRACE("data_type = 0x%x,len = %d,param = 0x%x\n",datatype,len,data[0]);
			mipi_force_write(datatype, data, len);//type,data,len
			i += len+3;
		}
		if(UDC_LCD_MIPI_GEN_WRITE ==cmd)
		{
			len = value[i+1];
			for(j = 0; j < len; j++)
			{
				data[j] = (uint8_t)value[i+2+j];
			}
			UDC_LCD_TRACE("len = %d,param = 0x%x\n",len,data[0]);
			mipi_gen_write(data, len);//data,len
			i += len+2;
		}
		if(UDC_LCD_MIPI_FORCE_READ ==cmd)
		{ 
			len = value[i+2];
			data[0] = (uint8_t)value[i+1];
			UDC_LCD_TRACE("addr = 0x%x,len = %d\n",data[0],len);
			mipi_force_read(data[0], len,read_data);//addr,len,buf
			read_data[0] = read_data[value[i+3]];   //?DD?y?Y
			i += 4;
		}
		if(UDC_LCD_MIPI_GEN_READ ==cmd)
		{
				//mipi_gen_read(data, len));
		}
		if(UDC_LCD_DELAY_MS == cmd)
		{
			mdelay(value[i+1]);
			i += 2;
		}
		udelay(30);
	}
	UDC_LCD_TRACE("read_data = 0x%x\n",read_data[0]);
	
	return read_data[0];
}

#endif




static uint32_t udc_lcd_start(udc_panel *self, udc_t key_id,
	                                          uint16_t left, uint16_t top,uint16_t right,uint16_t bottom)
{
	udc_t *value;
	uint32_t read_param = 0;
	unsigned short value_count;
       udc_lcd* lcd = udc_get_lcd(SEC_LCD0);
	udc_t pannel_type =  udc_lcd_get_pannel_type(lcd);
	   
	  
       UDC_LCD_TRACE("%s: key_id = %d, pannel_type = %d \n", __func__, key_id, pannel_type);
	   
	if (key_id >= CONFIG_MAX_ID) {
		UDC_LCD_TRACE("%s invalid sub name id %d\n", __func__, key_id);
		return (uint32_t)-EINVAL;
	}
        
	value =   lcd->item[key_id].value;
	value_count = lcd->item[key_id].value_count;

	UDC_LCD_TRACE("%s: value = 0x%x, value_count = %d\n", __func__, *value, value_count);
	if ((value != NULL) && (value_count>0 ))
	{
#ifdef CONFIG_UDC_LCD_MCU
		if ( LCD_MODE_MCU ==  pannel_type )
		{
		       struct ops_mcu* ops = self->info.mcu->ops;
			read_param = udc_lcd_mcu_do(ops, value, value_count, left, top, right, bottom);
		}
#endif
#ifdef CONFIG_UDC_LCD_RGB
		if ( LCD_MODE_RGB ==  pannel_type )
		{
			struct ops_spi*ops = self->info.rgb->bus_info.spi->ops;
			read_param = udc_lcd_rgb_do(ops, value, value_count, left, top, right, bottom);
			
		}
#endif
#ifdef CONFIG_UDC_LCD_MIPI
		 if ( LCD_MODE_DSI ==  pannel_type )
		{
			struct ops_mipi*ops = self->info.mipi->ops;
			read_param = udc_lcd_mipi_do(ops, value, value_count, left, top, right, bottom);

		}
#endif		
	
	}

	return read_param;
}


static int32_t udc_lcd_init(udc_panel *self)
{
	
	UDC_LCD_TRACE("%s\n", __func__);
	
	udc_lcd_start(self,LCD_INIT_PARA,0,0,0,0);

	return 0;
}

static int32_t udc_lcd_set_window(udc_panel *self,
		uint16_t left, uint16_t top, uint16_t right, uint16_t bottom)
{
	UDC_LCD_TRACE("%s\n", __func__);
	udc_lcd_start(self,LCD_SET_WINDOW,left,top,right,bottom);
	return 0;
}

static int32_t udc_lcd_invalidate(udc_panel *self)
{
	UDC_LCD_TRACE("%s\n", __func__);
	return udc_lcd_set_window(self, 0, 0, 
			self->width-1, self->height-1);
}

static int32_t udc_lcd_invalidate_rect(udc_panel *self,
				uint16_t left, uint16_t top,
				uint16_t right, uint16_t bottom)
{
	UDC_LCD_TRACE("%s\n", __func__);
	udc_lcd_start(self,LCD_INVALIDATE_RECT,left,top,right,bottom);
	return udc_lcd_set_window(self, left, top, right, bottom);
}

static int32_t udc_lcd_set_direction(udc_panel *self, uint16_t direction)
{
	udc_t key_id;
	UDC_LCD_TRACE("%s\n", __func__);
	switch (direction) {
	case LCD_DIRECT_NORMAL:
		key_id = LCD_DIR_NORMAL;
		break;
	case LCD_DIRECT_ROT_90:
		key_id = LCD_DIR_ROT90;
		break;
	case LCD_DIRECT_ROT_180:
		key_id = LCD_DIR_ROT180;
		break;
	case LCD_DIRECT_ROT_270:
		key_id = LCD_DIR_ROT270;
		break;
	case LCD_DIRECT_MIR_H:
		key_id = LCD_DIR_MIRH;
		break;
	case LCD_DIRECT_MIR_V:
		key_id = LCD_DIR_MIRV;
		break;
	case LCD_DIRECT_MIR_HV:
		key_id = LCD_DIR_MIRHV;
		break;
	default:
		key_id = LCD_DIR_NORMAL;
		break;
	}
	
	udc_lcd_start(self,key_id,0,0,0,0);

	self->direction = direction;
	
	return 0;
}

static int32_t udc_lcd_enter_sleep(udc_panel *self, uint8_t is_sleep)
{
	UDC_LCD_TRACE("%s\n", __func__);
	if (is_sleep)
		udc_lcd_start(self,LCD_ENTER_SLEEP,0,0,0,0);
	else
		udc_lcd_start(self,LCD_EXIST_SLEEP,0,0,0,0);
	return 0;
}

static uint32_t udc_lcd_read_id(udc_panel*self)
{
	udc_lcd	*lcd = udc_get_lcd(SEC_LCD0);
	int	j = 0;
	int lcm_id, lcm_cfg_id;
	
	printf("%s\n", __func__);
	lcm_cfg_id = lcd->cfg->lcd_id;
	
	printf("%s lcm_cfg_id=0x%x\n", __func__, lcm_cfg_id);

	for(j = 0; j < 4; j++){

	lcm_id = udc_lcd_start(self,LCD_READ_ID,0,0,0,0);

	if (lcm_id <= 0)
		lcm_id = UDC_LCM_ID;
	
	printf("%s 0x%x\n", __func__, lcm_id);

	if (lcm_id == lcm_cfg_id)
		break;

	}
	return lcm_id;
}


static int udc_lcd_power(udc_panel *self,int on)
{
	if (on) {
	udc_lcd_start(self,LCD_POWER_ON_SEQUENCE,0,0,0,0);
	} else {
	udc_lcd_start(self,LCD_POWER_OFF_SEQUENCE,0,0,0,0);
	}
	return 0;
}

static int udc_lcd_set_pannel_type(udc_lcd* lcd, udc_t type)
{
       udc_t pannel_type = type;
	udc_panel_cfg *cfg = lcd->cfg;
 
#if defined (CONFIG_UDC_VER_4_0_1 ) || defined (CONFIG_UDC_VER_5_1_3)
	 if ( LCD_MODE_MCU == type )
	        pannel_type = LCD_MODE_RGB;
	   else if ( LCD_MODE_RGB == type)
	   	 pannel_type = LCD_MODE_MCU;
	   
	   cfg->panel->type = pannel_type;
#else
          cfg->panel->mode = pannel_type;
#endif

      UDC_LCD_TRACE("%s () line = %d, pannel_type = %d\n", __func__, __LINE__, pannel_type);
      return 1;

}


static udc_t udc_lcd_get_pannel_type(udc_lcd* lcd)
{
       udc_t pannel_type = 0;
	udc_panel_cfg *cfg = lcd->cfg;
 
#if defined (CONFIG_UDC_VER_4_0_1 ) || defined (CONFIG_UDC_VER_5_1_3) 
	   pannel_type = cfg->panel->type;
#else
          pannel_type = cfg->panel->mode;
#endif

      UDC_LCD_TRACE("%s () line = %d, pannel_type = %d\n", __func__, __LINE__, pannel_type);

      return pannel_type;

}

udc_t udc_lcd_get_value(udc_t key_id, udc_t* value, udc_t value_count)
{
	udc_lcd* lcd = &g_udc_lcd;
	return udc_get_item_value(lcd->item, key_id, value, value_count);
}

udc_lcd* udc_get_lcd(udc_t section_id)
{
	return &g_udc_lcd;
}

udc_t udc_get_lcd_offset(void)
{
       udc_lcd* lcd = udc_get_lcd(SEC_LCD0);
	return lcd->current_lcd_offset;
}

static int udc_lcd_search_section(udc_lcd* lcd, udc_t section_id, udc_section* section)
{
        udc_t ret;
        udc_s* udc = lcd->udc;
		
	if(!(section_id&0x00ff))	//first section of lcd
	{
       	ret = udc_search_first_section(udc, section_id, section);
	}
	else
	{
		ret = udc_search_next_section(udc, &lcd->current_section, section);
	}

	UDC_LCD_TRACE("%s () line = %d, ret = %d\n", __func__, __LINE__, ret);

	if  ( !ret )
	{   
             return 0;	 
	}
		


	lcd->current_section = *section;
       lcd->current_lcd_offset = section->buffer  - udc->buffer;	

	return 1;   
	   
}

int udc_lcd_config_panel(udc_lcd* lcd, udc_t section_id)
{
       udc_t ret;
	udc_t param;
	udc_t pannel_type;
   	udc_panel_cfg  *cfg = lcd->cfg;
	udc_section section;


	ret = udc_lcd_search_section(lcd, section_id, &section);
	UDC_LCD_TRACE("%s () line = %d, ret = %d\n", __func__, __LINE__, ret);
       if ( !ret )
	    return 0;	
	   
	ret = udc_match_item(lcd->udc, &lcd->current_section, lcd->item);
	UDC_LCD_TRACE("%s () line = %d, ret = %d\n", __func__, __LINE__, ret);
       if ( !ret )
	    return 0;	
	   
		
	if ( udc_lcd_get_value(LCD_ID, &param, 1) )
		cfg->lcd_id = param;
	
	
	if ( udc_lcd_get_value(LCD_WIDTH,  &param, 1) )
		cfg->panel->width = param;
       
	if ( udc_lcd_get_value(LCD_HIGHT, &param, 1) )
		cfg->panel->height = param;
      
	if ( udc_lcd_get_value(LCD_DIRECTION, &param, 1) )
		cfg->panel->direction= param;

	UDC_LCD_TRACE("lcd_id 0x%x\n", cfg->lcd_id);

	UDC_LCD_TRACE("lcd_width %d\n", cfg->panel->width);

	UDC_LCD_TRACE("lcd_hight %d\n", cfg->panel->height);

	UDC_LCD_TRACE("lcd_direction: %d\n", cfg->panel->direction);


	if ( udc_lcd_get_value(LCD_MODE, &param, 1) )
	{
		udc_lcd_set_pannel_type(lcd, param);
	}

      pannel_type = udc_lcd_get_pannel_type(lcd);
	  
#ifdef CONFIG_UDC_LCD_MCU		
	if  (pannel_type == LCD_MODE_MCU) {

		udc_lcd_config_mcu_panel(lcd);
		
	} 
#endif	
#ifdef CONFIG_UDC_LCD_RGB	
	if (pannel_type == LCD_MODE_RGB) {

		udc_lcd_config_rgb_panel(lcd);
		if ( udc_lcd_get_value(LCD_FPS, &param, 1) )
			cfg->panel->fps = param;
		UDC_LCD_TRACE("lcd_fps %d\n", cfg->panel->fps);		
	}
#endif	
#ifdef CONFIG_UDC_LCD_MIPI
	if (pannel_type == LCD_MODE_DSI) 
	{
            
		udc_lcd_config_mipi_panel(lcd);
		if ( udc_lcd_get_value(LCD_FPS, &param, 1) )
			cfg->panel->fps = param;	
		UDC_LCD_TRACE("lcd_fps %d\n", cfg->panel->fps);	
	}
#endif
	
	return 1;

}


udc_lcd* udc_lcd_create(udc_t section_id, udc_panel_cfg *cfg)
{
       udc_lcd* lcd = &g_udc_lcd;
	



	lcd->udc = udc_get_udc();

	lcd->item = g_udc_lcd_item;
	
	lcd->cfg = cfg;
	
#ifdef CONFIG_UDC_LCD_MCU
	{
		udc_t main_lcd_cs = lcd->udc->board->config.lcd_cs;

		lcd->lcd_fmark = 0x01;
	       cfg->lcd_id = UDC_LCM_ID;
		
		if ( SEC_LCD0 == (section_id & 0xff00) )
			lcd->lcd_cs = main_lcd_cs;
		else
			lcd->lcd_cs = (~main_lcd_cs)&0x01;
		lcd->config_lcm = udc_config_lcm;
		lcd->update_lcm = udc_update_lcm;
		lcd->set_fmark = udc_lcd_set_fmark;
	}
#endif	

	
	
	
	return lcd;
}


