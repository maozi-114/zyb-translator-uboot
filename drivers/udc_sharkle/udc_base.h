
#ifndef __UDC_BASE_H__
#define __UDC_BASE_H__

typedef unsigned short int      udc_t;  /* 16 bits */
typedef unsigned long int       udc32_t;  /* 32 bits */

#define  UDC_MEM_SIZE			0x00010000 /* max 64k */
//#define UDC_MEM_ADDR	 		0x04BF0000
#ifdef CONFIG_UDC_VER_2_3_5
#define UDC_MEM_ADDR	 		0x0f000000// 256M - 16M

#else
#ifdef CONFIG_TIGER
#define UDC_MEM_ADDR	 		0x0e000000//0x8e000000

#else
#define UDC_MEM_ADDR	 		0x93000000 //0x8e000000//0x0e000000

#endif
#endif

#ifndef MIN
# define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#ifndef MAX
# define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

typedef enum _udc_section_id_e{
	SEC_PINMAP            = 0x0100,
	SEC_GPIO		    = 0x0200,
	SEC_BOARD		    = 0x0300,
	SEC_BACKLIGHT      = 0x0400,
	SEC_LCD0		    = 0x0500,
	SEC_LCD1		    = 0x0600,
	SEC_TP			    = 0x0700,
	SEC_GSENSOR         = 0x0800,
	SEC_MSENSOR         = 0x0900,
	SEC_LSENSOR          = 0x0a00,
	SEC_PSENSOR          = 0x0b00,
	SEC_FM			    = 0x0c00,
	SEC_MAIN_CAMERA = 0x0d00,
	SEC_SUB_CAMERA   = 0x0e00,
	SEC_ATV                  = 0x0f00,

SEC_INVALID = 0XFFFF ,
      
}udc_section_id_e;


typedef struct _udc_section{
	udc_t id;
	udc_t size;
	udc_t*buffer;
}udc_section;

typedef struct _udc_item_s{
	udc_t key_id;
	udc_t value_count;
	udc_t* value;
}udc_item_s;

typedef struct _udc_s{
	unsigned short	*buffer;               
	udc32_t			 length; 
	struct _udc_board* board;
}udc_s;


extern udc_s* udc_create (char* buffer );
extern int udc_add_cmdline(char* cmdline);
extern udc_s* udc_get_udc (void );
extern udc_t udc_get_item_value(udc_item_s*item, udc_t key_id, udc_t* value, udc_t max_count);
extern udc_t udc_search_first_section(udc_s*udc,  udc_t section_id,  udc_section* section);
extern udc_t udc_search_next_section(udc_s*udc,  udc_section* current , udc_section* next);
extern udc_t udc_search_value(udc_s*udc, udc_section* section,  udc_t key_id, udc_item_s* item );
extern udc_t udc_match_item(udc_s*udc, udc_section* section,  udc_item_s* item );


typedef enum _udc_board_keyid{
	BOARD_FM_LDO             ,
	BOARD_FM_LDO_LEVEL       ,
	BOARD_FM_I2C_BUS_NUM 	 ,
	BOARD_GSENSOR_LDO        ,
	BOARD_GSENSOR_LDO_LEVEL  ,
	BOARD_GSENSOR_I2C_BUS_NUM,
	BOARD_MSENSOR_LDO        ,
	BOARD_MSENSOR_LDO_LEVEL  ,
	BOARD_MSENSOR_I2C_BUS_NUM,
	BOARD_LSENSOR_LDO        ,
	BOARD_LSENSOR_LDO_LEVEL  ,
	BOARD_LSENSOR_I2C_BUS_NUM,
	BOARD_PSENSOR_LDO        ,
	BOARD_PSENSOR_LDO_LEVEL  ,
	BOARD_PSENSOR_I2C_BUS_NUM,
	BOARD_TP_LDO             ,
	BOARD_TP_LDO_LEVEL       ,
	BOARD_TP_I2C_BUS_NUM 	 ,
	BOARD_LCD_CS				,
	BOARD_BACKLIGHT_IC_GPIO   ,
	BOARD_CAMERA_IDENTIFY_FLAG ,
	BOARD_INVALID = 0xffff		,
      
}udc_board_keyid;

typedef struct _udc_board_config {
	udc_t fm_ldo               ;	
	udc_t fm_ldo_level         ;	
	udc_t fm_i2c_bus_num  ;
	udc_t gsensor_ldo          ; 
	udc_t gsensor_ldo_level    ; 
	udc_t gsensor_i2c_bus_num  ; 
	udc_t msensor_ldo          ; 
	udc_t msensor_ldo_level    ; 
	udc_t msensor_i2c_bus_num  ; 
	udc_t lsensor_ldo          ; 
	udc_t lsensor_ldo_level    ; 
	udc_t lsensor_i2c_bus_num  ;  
	udc_t psensor_ldo          ; 
	udc_t psensor_ldo_level    ; 
	udc_t psensor_i2c_bus_num  ; 
	udc_t tp_ldo               ;	
	udc_t tp_ldo_level         ;	
	udc_t tp_i2c_bus_num       ;
	udc_t lcd_cs			;
	udc_t backlight_ic_gpio	;
	udc_t camera_identify_flag;
}udc_board_config;



typedef struct _udc_board{
	udc_s* udc;
	udc_item_s* item;
	udc_board_config config;
}udc_board;

#define  UDC_DEBUG 1
#ifdef UDC_DEBUG
#define UDC_TRACE printf
#else
#define UDC_TRACE(...)
#endif


#endif

 
