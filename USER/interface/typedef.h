/*******************************************************************************
* File Name       typedef.h
* Anthor          WXJ     
* Version         V1.00
* Date            2013/4/11
* Description     --    
*******************************************************************************/
#ifndef _typedef_h_
#define _typedef_h_

#include "stm32f10x.h"

typedef enum
{
    FS_FORMAT_DISK = 0,
    FS_OPEN_FILE,
    FS_READ_FILE,
    FS_DELETE_FILE,
    FS_CREATE_FILE,
    FS_GET_DISK_INFO,
    FS_SYS_RESET,
    FS_EDIT_FILE,
    FS_FILE_LIST,
    FS_HELP,
    FS_UNDEF_CMD	
} FS_CMD;

  typedef struct
  {
    uint8_t Maker_ID;
    uint8_t Device_ID;
    uint8_t Third_ID;
    uint8_t Fourth_ID;
  }NAND_IDTypeDef;
  
  typedef struct 
  {
    uint16_t Zone;
    uint16_t Block;
    uint16_t Page;
  } NAND_ADDRESS;

#endif

/***************************** End of File ************************************/