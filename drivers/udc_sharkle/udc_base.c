

#include <udc.h>
#include <mmc.h>


extern nand_info_t nand_info[];


udc_s g_udc;


int udc_add_cmdline(char* cmdline)
{
	
       int str_len;
	udc_s* udc = &g_udc;
	str_len = strlen(cmdline);

	#ifdef CONFIG_UDC_LCD
       sprintf(&cmdline[str_len], " udc=0x%x, 0x%x, 0x%x ", (long) udc->buffer,  udc->length, udc_get_lcd_offset());
	#else
	sprintf(&cmdline[str_len], " udc=0x%x, 0x%x ", (long) udc->buffer,  udc->length);
       #endif
	   
	str_len = strlen(cmdline);
	return str_len;

} 


udc_t udc_get_item_value(udc_item_s*item, udc_t key_id, udc_t* value, udc_t max_count)
{
       udc_t i = 0;
	udc_t count;  
	udc_t value_count = item[key_id].value_count;


       count = MIN(value_count, max_count);
	   
       if ( count <= 0 )
	{   	
		return 0;
       } 

	for ( i = 0; i < count; i++ )
       {
              value[i] = item[key_id].value[i];
       } 
	   
	return count;
	
}


/****************************************************************
 * udc_search_first_section: search next section by section id
 * 	in:
 *		@udc		: pointer to udc_s struct
 *		@section_id	: section id to search
 *	out:
 *		@section	       : section
 *	return:
 *		1	:            find the section
 *		0      :           no section
*****************************************************************/
udc_t udc_search_first_section(udc_s*udc,  udc_t section_id,  udc_section* section)
{

     udc_t*buffer = udc->buffer;
     udc_t udc_section_id, udc_section_size, count;
    	 
      section->size  = 0;

       UDC_TRACE("%s:section_id = 0x%x,  udc->buffer = 0x%x\n", __func__, section_id,  udc->buffer);
    
      if ( NULL == buffer )
	  	return 0;

	//stetp1: skip time stemp 2B
       buffer += 2;



       while ( 1 )
       {
		 //step2 get sec id and size
		udc_section_id =  *buffer++;
		udc_section_size = *buffer++;

	       UDC_TRACE("%s: udc_section_id = 0x%x, udc_section_size = 0x%x\n", __func__,  udc_section_id, udc_section_size);
	     
		   
	      //end of udc img
		if ( 0xffff == udc_section_id && 0xffff == udc_section_size) 
		{
	             goto SECTION_OUT;

		}
		
		//step3:major section id matched
              if  ( (udc_section_id & 0xff00) == section_id)
	       {
		       buffer -=2;
                     section->id = udc_section_id;
			section->size = udc_section_size;
			section->buffer = buffer;
			
			UDC_TRACE("%s: find section = 0x%x, udc_section_size = 0x%x\n", __func__,  buffer, udc_section_size);
			return 1;
			
	       }	
		else
		{
		    
			buffer += ( udc_section_size/sizeof(udc_t) );
			 
		}

       }

SECTION_OUT:
	UDC_TRACE("%s: NO section : 0x%x \n", __func__,  section_id);
	
      section->size  = 0;
      return 0;


}


/****************************************************************
 * udc_search_next_section: search next section by section id
 * 	in:
 *		@udc		: pointer to udc_s struct
 *		@current	       : current section
 *	out:
 *		@next	       : next section
 *	return:
 *		1	:            find the section
 *		0      :           no section
*****************************************************************/
udc_t udc_search_next_section(udc_s*udc,  udc_section* current ,  udc_section* next)
{
       udc_t ret = 0;
       udc_t udc_section_id, udc_section_size, count;
       udc_t*buffer = current->buffer;
     
	udc_section_id =  current->id;
	udc_section_size = current->size;
	buffer = current->buffer;	
     
      UDC_TRACE("%s: udc_section_id = 0x%x, buffer = 0x%x\n", __func__,  udc_section_id, buffer);
  
      if ( NULL == buffer)
	  	return 0;

	//stetp1: skip current section
       while(1)
      	{
		buffer += 2;
	       buffer += ( udc_section_size/sizeof(udc_t) );
	      
		if ( buffer - udc->buffer >  udc->length )
		{
	             UDC_TRACE("%s: buffer = 0x%x out of file size %d \n", __func__,   buffer - udc->buffer, udc->length);
		      return 0;
		}
        
       	 //step2 get sec id and size
		udc_section_id =  buffer[0];
		udc_section_size = buffer[1];

	       UDC_TRACE("%s: udc_section_id = 0x%x, udc_section_size = 0x%x\n", __func__,  udc_section_id, udc_section_size);
	     
		   
	      //end of udc img
		if ( 0xffff == udc_section_id && 0xffff == udc_section_size) 
		{
	             goto SECTION_OUT;

		}
		
		//step3:major section id matched
              if  ( (udc_section_id & 0xff00) == (current->id & 0xff00) )
	       {
		       next->id = udc_section_id;
			next->size = udc_section_size;
			next->buffer = buffer;
			UDC_TRACE("%s: find section = 0x%x, udc_section_size = 0x%x\n", __func__,  buffer, udc_section_size);
			return 1;
			
	       }	
	}

SECTION_OUT:
	UDC_TRACE("%s: NO section :0x%x \n", __func__,  current->id);
	
       return 0;


}


udc_t  udc_search_value(udc_s*udc, udc_section* section,  udc_t key_id, udc_item_s* item )
{
     udc_t*pValue = NULL;
     udc_t udc_section_size, count;
     udc_t udc_key_id, udc_keySize;
	 
 
      item->value_count = 0;
      udc_section_size = section->size;
      pValue = section->buffer;
	  
       UDC_TRACE("%s:section->id = 0x%x,  section->buffer = 0x%x\n", __func__, section->id,  section->buffer);
  
  
      if ( NULL == pValue || section->size == 0)
	  	return 0;

   
       //stetp1: skip section id and size: 2B
       pValue += 2;
	count = 0;

	while( count < udc_section_size)
	{

      		udc_key_id =  *pValue ++;
	       udc_keySize = *pValue ++;
		 //end of udc img
		if ( 0xffff == udc_key_id && 0xffff == udc_keySize) 
		{
	             goto VALUE_OUT;
		}  
   
     
	       if  (  key_id == udc_key_id )
	       {
                     item->key_id = udc_key_id;
			item->value_count =  udc_keySize/sizeof(udc_t);
			item->value = pValue;
			return 1;
		}
		  
	     	count +=  ( 4 + udc_keySize );
		pValue += (udc_keySize/sizeof(udc_t));

	}

		
VALUE_OUT:
	UDC_TRACE("%s: section_id :0x%x , NO key :0x%x\n", __func__,  section->id, key_id);

       item->value_count = 0;
       return 0;

}


udc_t  udc_match_item(udc_s*udc, udc_section* section,  udc_item_s* item )
{
   	udc_item_s find_item;
	udc_t ret = 0;

	if ( item == NULL )
	    return 0;      	
	   
	while( item && 0xffff != item->key_id )
       {
	       ret = udc_search_value(udc, section, item->key_id, &find_item);

		if ( ret > 0 ) 
		{
		      item->value = find_item.value;
		      item->value_count = find_item.value_count;
       	}
		else
		{
			item->value = NULL;
			item->value_count = 0;
		}
		item++;
	}
		
	
	return 1;

}

/****************************************************************
 * udc_get_file_size: 	caculate the udc data size (the size of udc->buffer),
 *					save the length to udc->length
 * 	in:
 *		@udc		: pointer to udc_s struct
 *	return:
 *		the length of the udc data
*****************************************************************/
udc32_t udc_get_file_size(udc_s*udc)
{
      udc32_t length = 0; 
     
     udc_t*section = udc->buffer;
     udc_t section_id, section_size;
    	 
     
      UDC_TRACE("%s: buffer = 0x%x\n", __func__,  udc->buffer);
    
      if ( NULL == section )
	  	return 0;

	//TODO:  check time stamp
	section += 2;
       length = 4;
 
       
       while(1)
       {
		 //step2 get sec id and size
		section_id = *section++;
		section_size = *section++;
		//end of file
		if ( 0xffff == section_id && 0xffff == section_size ) 
			break;
             
		section += ( section_size / ( sizeof(udc_t) ) ) ;
			  
	       length += (4 + section_size);
	       
	}

       length += 4; //end flag
	udc->length = length;

       UDC_TRACE("%s: udc->length = 0x%x\n", __func__, udc->length);

	return length;

}

udc_s* udc_get_udc (void )
{
	return &g_udc;
}


#define UDC_CONFIG "params"

/******************************************************************
 * 	udc_load_config	: load udc data from nand flash fastboot_logo area to buffer
 * 	in:
 *		@max_size	: the max udc data size ,should be UDC_MEM_SIZE
 *	out:
 *		@buffer		: the address save the data
*****************************************************************/
#ifdef CONFIG_EMMC_BOOT
#define EMMC_SECTOR_SIZE 512
extern int Emmc_Read(char  part_num, lbaint_t startBlock,lbaint_t num,uint8_t* buf);
void udc_load_config(char* buffer, size_t max_size)
{
	size_t size;
	block_dev_desc_t *p_block_dev = NULL;    
	disk_partition_t info;

	UDC_TRACE("%s begin:Reading udc_config to 0x%08x\n",__func__,buffer);

	p_block_dev = get_dev("mmc", 0);
	if(NULL == p_block_dev)
		return;

	size = (max_size +(EMMC_SECTOR_SIZE - 1)) & (~(EMMC_SECTOR_SIZE - 1));
	if(size <= 0) {
		printf("udc_config image should not be zero\n");
		return;
	}
	memset(&info, 0, sizeof(info));
	if (!get_partition_info_by_name(p_block_dev, "UDC_config", &info)){
  //	 printf("udc_load_config sizeof(lbaint_t):%d startBlock="LBAF",num=%d,buf=%lx\n",
  //                          sizeof(lbaint_t), info.start, size/EMMC_SECTOR_SIZE, buffer);
		if( FALSE ==  Emmc_Read(PARTITION_USER, info.start, size/EMMC_SECTOR_SIZE, (u8*)buffer)){
			printf("udc_config nand read error \n");
			return;
		}
	} 
	else
		return;
		
     UDC_TRACE("EMMC:%s(): line = %d,  buffer = 0x%08x, value: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",__func__, __LINE__, buffer, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
  
		//secure_check(FASTBOOT_PART, 0, MODEM_ADR + MODEM_SIZE - VLR_INFO_OFF, CONFIG_SYS_NAND_U_BOOT_DST + CONFIG_SYS_NAND_U_BOOT_SIZE - KEY_INFO_SIZ - VLR_INFO_OFF);
}	 

#else
void udc_load_config(char* buffer, size_t max_size)
{
       int ret;
	struct mtd_info *nand;
       struct mtd_device *dev;
       struct part_info *part;
       u8 pnum;
       size_t size = max_size;
       loff_t off = 0;

       UDC_TRACE("udc: load config begin\n");
       ret = mtdparts_init();
       if (ret != 0) {
               UDC_TRACE("hwinfo:mtdparts init error %d\n", ret);
               return;
       }

       ret = find_dev_and_part(UDC_CONFIG, &dev, &pnum, &part);
       if(ret){
               UDC_TRACE("udc:No partition named %s\n", UDC_CONFIG);
               return;
       }else if(dev->id->type != MTD_DEV_TYPE_NAND){
               UDC_TRACE("udc:Partition %s not a NAND device\n", UDC_CONFIG);
               return;
       }

       off = part->offset;
		
       nand = &nand_info[dev->id->num];

	   
        ret = nand_read_offset_ret(nand, off, &size, (void *)buffer, &off);
       if(ret != 0){
               UDC_TRACE("udc:Nand read error %d\n", ret);
               return;
       }
	UDC_TRACE("%s(): line = %d, buffer = 0x%08x, value:  0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",__func__, __LINE__, buffer, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
   
       UDC_TRACE("udc: loading end\n");
} 
#endif

udc_item_s g_udc_board_item[] = {
  {	BOARD_FM_LDO             , 0, 0},
  {	BOARD_FM_LDO_LEVEL       , 0, 0},
  {	BOARD_FM_I2C_BUS_NUM 	   , 0, 0},
  {	BOARD_GSENSOR_LDO        , 0, 0},
  {	BOARD_GSENSOR_LDO_LEVEL  , 0, 0},
  {	BOARD_GSENSOR_I2C_BUS_NUM, 0, 0},
  {	BOARD_MSENSOR_LDO        , 0, 0},
  {	BOARD_MSENSOR_LDO_LEVEL  , 0, 0},
  {	BOARD_MSENSOR_I2C_BUS_NUM, 0, 0},
  {	BOARD_LSENSOR_LDO        , 0, 0},
  {	BOARD_LSENSOR_LDO_LEVEL  , 0, 0},
  {	BOARD_LSENSOR_I2C_BUS_NUM, 0, 0},
  {	BOARD_PSENSOR_LDO        , 0, 0},
  {	BOARD_PSENSOR_LDO_LEVEL  , 0, 0}, 
  {	BOARD_PSENSOR_I2C_BUS_NUM, 0, 0},
  {	BOARD_TP_LDO             , 0, 0},
  {	BOARD_TP_LDO_LEVEL       , 0, 0},
  {	BOARD_TP_I2C_BUS_NUM 	   , 0, 0},
  {	BOARD_LCD_CS 	   		, 0, 0},
  {	BOARD_BACKLIGHT_IC_GPIO, 0, 0},
  {	BOARD_CAMERA_IDENTIFY_FLAG, 0, 0},
  {   BOARD_INVALID ,            0, 0}      
};  

udc_board g_udc_board;

static udc_t udc_board_get_value(udc_t key_id, udc_t* value, udc_t value_count)
{
       udc_board* board = &g_udc_board;
	return udc_get_item_value(board->item, key_id, value, value_count);
}

static void udc_board_init (udc_board* board)
{
       udc_t param;
	  
	udc_board_config* config = &board->config;
	config->lcd_cs = 0;   //CS0
	config->backlight_ic_gpio = 143;

	if ( udc_board_get_value(BOARD_LCD_CS, &param, 1) )
	{
		config->lcd_cs= param;
		UDC_TRACE("%s: line = %d, lcd_cs %d\n", __func__, __LINE__, config->lcd_cs);
    
	}
	
	if ( udc_board_get_value(BOARD_BACKLIGHT_IC_GPIO, &param, 1) )
	{
		config->backlight_ic_gpio= param;
		UDC_TRACE("%s: line = %d, backlight_ic_gpio %d\n", __func__, __LINE__, config->backlight_ic_gpio);
    
	}
	

}
  
static udc_board* udc_board_create(udc_t section_id)
{
       udc_t ret = 0;
	udc_s* udc;
	udc_section section;
	udc_board* board = &g_udc_board;
       
	
       udc = udc_get_udc();

	board->udc = udc;
	board->item = g_udc_board_item;
	
       ret = udc_search_first_section(udc, section_id, &section);
	   
	ret = udc_match_item(udc, &section, board->item);

	
	udc_board_init(board);	
	
	return board;

}

	unsigned short	* udc_buffer;               
	unsigned long int		 udc_length; 
static void udc_init (udc_s*udc, char* buffer )
{
	udc->buffer = buffer;
	udc_buffer = udc->buffer;

	udc->length = 0;
       udc_load_config(buffer, UDC_MEM_SIZE);
 
   //printf("%s: line = %d, buffer: 0x%x, 0x%x, 0x%x, 0x%x\n", __func__, __LINE__, udc->buffer[0], udc->buffer[1], udc->buffer[2], udc->buffer[3]);  

       udc->length = udc_get_file_size(udc);
	   udc_length = udc->length;
       
	udc->board = udc_board_create(SEC_BOARD);
}


udc_s* udc_create (char* buffer )
{
	udc_s* udc = &g_udc;
       
	udc_init(udc, buffer);

	return udc;

}


