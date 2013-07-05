
/**
  ******************************************************************************
  * @file		NandFlash.c
  * @brief		NandFlash驱动程序 源文件，用户不应该直接调用该文件中的函数 .
  * @version  	1.0
  ******************************************************************************
  */

#include "nandflash.h"



//#define 

void NAND_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure; 
  FSMC_NAND_PCCARDTimingInitTypeDef  p;
  FSMC_NANDInitTypeDef FSMC_NANDInitStructure;
  
  /*FSMC总线使用的GPIO组时钟使能*/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | 
                         RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG, ENABLE);
  
 /*FSMC CLE, ALE, D0->D3, NOE, NWE and NCE2初始化，推挽复用输出*/
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_14 | GPIO_Pin_15 |  
                                 GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | 
                                 GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;

  GPIO_Init(GPIOD, &GPIO_InitStructure); 

  /*FSMC数据线FSMC_D[4:7]初始化，推挽复用输出*/ 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;

  GPIO_Init(GPIOE, &GPIO_InitStructure);

  /*FSMC NWAIT初始化，输入上拉*/ 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;

  GPIO_Init(GPIOD, &GPIO_InitStructure); 
  /*FSMC INT2初始化，输入上拉*/ 

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_Init(GPIOG, &GPIO_InitStructure);

  /*--------------FSMC 总线 存储器参数配置------------------------------*/
  p.FSMC_SetupTime = 0x1;         //建立时间
  p.FSMC_WaitSetupTime = 0x3;     //等待时间
  p.FSMC_HoldSetupTime = 0x2;     //保持时间
  p.FSMC_HiZSetupTime = 0x1;      //高阻建立时间

  FSMC_NANDInitStructure.FSMC_Bank = FSMC_Bank2_NAND; //使用FSMC BANK2
  FSMC_NANDInitStructure.FSMC_Waitfeature = FSMC_Waitfeature_Enable; //使能FSMC的等待功能
  FSMC_NANDInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_8b; //NAND Flash的数据宽度为8位
  FSMC_NANDInitStructure.FSMC_ECC = FSMC_ECC_Enable;                  //使能ECC特性
  FSMC_NANDInitStructure.FSMC_ECCPageSize = FSMC_ECCPageSize_2048Bytes; //ECC页大小2048
  FSMC_NANDInitStructure.FSMC_TCLRSetupTime = 0x00;             
  FSMC_NANDInitStructure.FSMC_TARSetupTime = 0x00;
  FSMC_NANDInitStructure.FSMC_CommonSpaceTimingStruct = &p;
  FSMC_NANDInitStructure.FSMC_AttributeSpaceTimingStruct = &p;

  FSMC_NANDInit(&FSMC_NANDInitStructure);

  /*!使能FSMC BANK2 */
  FSMC_NANDCmd(FSMC_Bank2_NAND, ENABLE);
}
/**
  * @defgroup 	NandFlashPrivate
  * @brief 		NandFlash驱动程序 内部 .
  * @{
  */

/**
  ******************************************************************************	
  * @brief  	片选 .
  * @param  	无 
  * @retval 	无
  * @note		
  ******************************************************************************
  */
static void NFChipSel(void)
{
	GPIO_ResetBits(GPIOD,GPIO_Pin_7);
    //GPIO_BitWriteSafe(GPIO2, PIN_NUM_NAND_CE, 0);
}

/**
  ******************************************************************************	
  * @brief  	取消片选 .
  * @param  	无 
  * @retval 	无
  * @note		
  ******************************************************************************
  */
static void NFChipDesel(void)
{
    GPIO_SetBits(GPIOD,GPIO_Pin_7);
	//GPIO_BitWriteSafe(GPIO2, PIN_NUM_NAND_CE, 1);	
}

/**
  ******************************************************************************	
  * @brief  	等待NandFlash完成内部操作 .
  * @param  	TimeOut : 超时循环计数次数 
  * @retval 	参考 NF_STATUS_TypeDef 类型 (NandFlash.h)
  * @note		RB引脚变高表示内部操作完成
  ******************************************************************************
  */
static NF_STATUS_TypeDef NFWaitBusy(u32 TimeOut)
{		
	
    while((!(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7))) && (--TimeOut));	
    
    //while((!(GPIO_BitRead(GPIO2, PIN_NUM_NAND_RB))) && (--TimeOut));	
								 	
	return TimeOut ? NF_STATUS_SUCCESS : NF_STATUS_TIME_OUT;
										//TimeOut 大于0 表示没有超时
}

/**
  * @}
  */


/**
  * @addtogroup 	NandFlashPublic
  * @{
  */ 

/**
  ******************************************************************************	
  * @brief  	从某页中读取若干字节 .
  * @param  	Buffer : 缓冲区地址 
  * @param  	PageAddr : 页地址
  * @param  	ByteAddr : 页内字节地址
  * @param  	NumToRead : 需要读取的字节数
  * @retval 	参考 NF_STATUS_TypeDef 类型 (NandFlash.h) 			
  * @note		页指的是528字节页，读取数据必须在同一页中
  ******************************************************************************
  */
NF_STATUS_TypeDef 
NFReadPage(u8 * Buffer, u32 PageAddr, u32 ByteAddr, u32 NumToRead)
{
	NF_STATUS_TypeDef status;
	
	if(PageAddr >= NF_PAGE_COUNT)  				//页地址超出范围
	{
		return NF_STATUS_PAGE_OUT;	
	}

	if((ByteAddr + NumToRead) > (NF_PAGE_SIZE + NF_SPARE_SIZE))
	{											//读取的数据超出本页
		return NF_STATUS_BYTE_OUT;	
	}
		
	//NFChipSel();								//片选		

	if(ByteAddr & NF_AREA_C_MASK)				//字节地址位于 C区
	{
		NFCmdReg = NF_CMD_ADDR_AREA_C;
	}
	else if(ByteAddr & NF_AREA_B_MASK)			//字节地址位于 B区
	{
		NFCmdReg = NF_CMD_ADDR_AREA_B;
	}
	else							 			//字节地址位于 A区
	{
		NFCmdReg = NF_CMD_ADDR_AREA_A;
	}
	
	NFAddrReg = (u8)ByteAddr;					//发送地址			
	NFAddrReg = (u8)PageAddr; 
	NFAddrReg = (u8)(PageAddr >> 8);
	NFAddrReg = (u8)(PageAddr >> 16);
	
	status = NFWaitBusy(NF_TIME_OUT_READ);		//读忙
	
	if(status == NF_STATUS_SUCCESS)
	{
		while(NumToRead--)			 			//接收数据
		{
			*(Buffer++) = NFDataReg;
		}
		status = NFWaitBusy(NF_TIME_OUT_READ);	//读忙
	}

	//NFChipDesel(); 								//取消片选

	return status;
}

/**
  ******************************************************************************	
  * @brief  	向某页写入若干字节 .
  * @param  	Buffer : 缓冲区地址 
  * @param  	PageAddr : 页地址
  * @param  	ByteAddr : 页内字节地址
  * @param  	NumToRead : 需要写入的字节数
  * @retval 	参考 NF_STATUS_TypeDef 类型 (NandFlash.h) 
  * @note		页指的是528字节页，写入的数据必须在同一页中
  ******************************************************************************
  */
NF_STATUS_TypeDef 
NFWritePage(u8 * Buffer, u32 PageAddr, u32 ByteAddr, u32 NumToWrite)
{
	NF_STATUS_TypeDef status = NF_STATUS_TIME_OUT;

	if(PageAddr >= NF_PAGE_COUNT)  				//页地址超出范围
	{
		return NF_STATUS_PAGE_OUT;	
	}

	if((ByteAddr + NumToWrite) > (NF_PAGE_SIZE + NF_SPARE_SIZE))
	{											//读取的数据超出本页
		return NF_STATUS_BYTE_OUT;	
	}
		
	//NFChipSel();								//片选		

	if(ByteAddr & NF_AREA_C_MASK)				//字节地址位于 C区
	{
		NFCmdReg = NF_CMD_ADDR_AREA_C;
	}
	else if(ByteAddr & NF_AREA_B_MASK)			//字节地址位于 B区
	{
		NFCmdReg = NF_CMD_ADDR_AREA_B;
	}
	else							 			//字节地址位于 A区
	{
		NFCmdReg = NF_CMD_ADDR_AREA_A;
	}

	NFCmdReg = NF_CMD_PROG_TRUE_1;

	NFAddrReg = (u8)ByteAddr;					//发送地址			
	NFAddrReg = (u8)PageAddr; 
	NFAddrReg = (u8)(PageAddr >> 8);
	NFAddrReg = (u8)(PageAddr >> 16);

	while(NumToWrite--)			 				//接收数据
	{
		NFDataReg = *(Buffer++);
	}

	NFCmdReg = NF_CMD_PROG_TRUE_2; 				

	status = NFWaitBusy(NF_TIME_OUT_PROG);		//读忙
	
	if(status == NF_STATUS_SUCCESS)
	{
		NFCmdReg = NF_CMD_READ_STATUS;			//读取编程状态
		status = (NFDataReg & NF_ERROR_PROG_MASK) ? NF_STATUS_PROG_ERROR \
												  : NF_STATUS_SUCCESS;
	}	

	//NFChipDesel(); 								//取消片选

	return status;
}

#if 0
NF_STATUS_TypeDef NFWritePage(u8 * Buffer, u32 PageAddr, u32 ByteAddr, u32 NumToWrite)
uint32_t NAND_WriteSmallPage(uint8_t *pBuffer, NAND_ADDRESS Address, uint32_t NumPageToWrite)
{
  uint32_t index = 0x00, numpagewritten = 0x00, addressstatus = NAND_VALID_ADDRESS;
  uint32_t status = NAND_READY, size = 0x00;

  NF_STATUS_TypeDef status = NF_STATUS_TIME_OUT;
  while(( NumToWrite != 0x00) && (addressstatus == NAND_VALID_ADDRESS) && (status == NAND_READY))
  {
    /*!< Page write command and address */
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_AREA_A;
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_WRITE0;

    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = (u8)ByteAddr;
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = (u8)PageAddr; 
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = (u8)(PageAddr >> 8);
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = (u8)(PageAddr >> 16);

    /*!< Calculate the size */
    size = NAND_PAGE_SIZE + (NAND_PAGE_SIZE * numpagewritten);

    /*!< Write data */
 while(NumToWrite--)
 {
      *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA) = pBuffer[index++];
    }
    
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_WRITE_TRUE1;

    /*!< Check status for successful operation */
    status = NAND_GetStatus();  
  }
  if (status == SUCCESS )status = NF_STATUS_SUCCESS;
  return status;
}

#endif
/**
  ******************************************************************************	
  * @brief  	块擦除 .
  * @param  	BlockAddr : 块地址
  * @retval 	参考 NF_STATUS_TypeDef 类型 (NandFlash.h) 
  * @note		
  ******************************************************************************
  */
NF_STATUS_TypeDef NFErase(u32 BlockAddr)
{
	NF_STATUS_TypeDef status;

	if(BlockAddr >= NF_BLOCK_COUNT)  			//块地址超出范围
	{
		return NF_STATUS_BLOCK_OUT;	
	}
		
	//NFChipSel();								//片选		

	NFCmdReg = NF_CMD_ERASE_1;
	
	BlockAddr <<= NF_PAGE_PER_BLOCK_BIT;		
	NFAddrReg = (u8)BlockAddr ; 
	NFAddrReg = (u8)(BlockAddr >> 8);
	NFAddrReg = (u8)(BlockAddr >> 16);

	NFCmdReg = NF_CMD_ERASE_2; 				

	status = NFWaitBusy(NF_TIME_OUT_ERASE);		//读忙
	
	if(status == NF_STATUS_SUCCESS)
	{
		NFCmdReg = NF_CMD_READ_STATUS;			//读取编程状态
		status = (NFDataReg & NF_ERROR_ERASE_MASK) ? NF_STATUS_ERASE_ERROR \
												   : NF_STATUS_SUCCESS;
	}	

	//NFChipDesel(); 								//取消片选

	return status;	
}
/**
  ******************************************************************************	
  * @brief  	读取ID .
  * @param  	Buffer : 缓冲区地址,指向4字节的缓冲区 
  * @retval 	参考 NF_STATUS_TypeDef 类型 (NandFlash.h)
  * @note		
  ******************************************************************************
  */

NF_STATUS_TypeDef NFReadID(u8 * Buffer)
{	
	NF_STATUS_TypeDef status= NF_STATUS_FAILURE;	
	u32 i = 4;
	
	//NFChipSel();								//片选
	
	NFCmdReg = NF_CMD_READ_ID;
	NFAddrReg = 0;					
	while(i--)
	{
		*(Buffer++) = NFDataReg;
	}
	
	//NFChipDesel(); 								//取消片选
	
	status = NF_STATUS_SUCCESS;
	
	return status;
}

#if 0
NF_STATUS_TypeDef NFReadID(u8 * Buffer)
{
  	NF_STATUS_TypeDef status= NF_STATUS_FAILURE;	
    uint32_t data = 0;

  /*!< Send Command to the command area */
  *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = 0x90;
  *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;

   /*!< Sequence to read ID from NAND flash */
   data = *(__IO uint32_t *)(Bank_NAND_ADDR | DATA_AREA);

   *(Buffer+3)   = ADDR_1st_CYCLE (data);
   *(Buffer+2)  = ADDR_2nd_CYCLE (data);
   *(Buffer+1)   = ADDR_3rd_CYCLE (data);
   *Buffer  = ADDR_4th_CYCLE (data);
   
	status = NF_STATUS_SUCCESS;
	
	return status;
}

/**
  ******************************************************************************	
  * @brief  	复位 .
  * @param  	无 
  * @retval 	参考 NF_STATUS_TypeDef 类型 (NandFlash.h)
  * @note		
  ******************************************************************************
  */
#endif
NF_STATUS_TypeDef NFReset(void)
{
	NF_STATUS_TypeDef status;
	
	//NFChipSel();								//片选
	
	NFCmdReg = NF_CMD_RESET;
	
	status = NFWaitBusy(NF_TIME_OUT_RESET);		//读忙
	//
	//NFChipDesel();
	
	return status; 	
}

/**
  * @}
  */
  

#if 0
/**
  ******************************************************************************
  * @file    stm3210e_eval_fsmc_nand.c
  * @author  MCD Application Team
  * @version V4.2.0
  * @date    04/16/2010
  * @brief   This file provides a set of functions needed to drive the
  *          NAND512W3A2 memory mounted on STM3210E-EVAL board.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */ 

/* Includes ------------------------------------------------------------------*/
#include "fsmc_nand.h"

/** @addtogroup Utilities
  * @{
  */
  
/** @addtogroup STM32_EVAL
  * @{
  */ 

/** @addtogroup STM3210E_EVAL
  * @{
  */
  
/** @addtogroup STM3210E_EVAL_FSMC_NAND
  * @brief      This file provides a set of functions needed to drive the
  *             NAND512W3A2 memory mounted on STM3210E-EVAL board.
  * @{
  */ 

/** @defgroup STM3210E_EVAL_FSMC_NAND_Private_Types
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup STM3210E_EVAL_FSMC_NAND_Private_Defines
  * @{
  */ 
/** 
  * @brief  FSMC Bank 2 
  */
#define FSMC_Bank_NAND     FSMC_Bank2_NAND
#define Bank_NAND_ADDR     Bank2_NAND_ADDR 
#define Bank2_NAND_ADDR    ((uint32_t)0x70000000)     
/**
  * @}
  */ 

/** @defgroup STM3210E_EVAL_FSMC_NAND_Private_Macros
  * @{
  */
#define ROW_ADDRESS (Address.Page + (Address.Block + (Address.Zone * NAND_ZONE_SIZE)) * NAND_BLOCK_SIZE)  


uint32_t NAND_WriteSmallPage(uint8_t *pBuffer, NAND_ADDRESS Address, uint32_t NumPageToWrite)
{
  uint32_t index = 0x00, numpagewritten = 0x00, addressstatus = NAND_VALID_ADDRESS;
  uint32_t status = NAND_READY, size = 0x00;

  while((NumPageToWrite != 0x00) && (addressstatus == NAND_VALID_ADDRESS) && (status == NAND_READY))
  {
    /*!< Page write command and address */
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_AREA_A;
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_WRITE0;

    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(ROW_ADDRESS);
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(ROW_ADDRESS);
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(ROW_ADDRESS);

    /*!< Calculate the size */
    size = NAND_PAGE_SIZE + (NAND_PAGE_SIZE * numpagewritten);

    /*!< Write data */
    for(; index < size; index++)
    {
      *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA) = pBuffer[index];
    }
    
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_WRITE_TRUE1;

    /*!< Check status for successful operation */
    status = NAND_GetStatus();
    
    if(status == NAND_READY)
    {
      numpagewritten++;

      NumPageToWrite--;

      /*!< Calculate Next small page Address */
      addressstatus = NAND_AddressIncrement(&Address);
    }
  }
  
  return (status | addressstatus);
}

/**
  * @brief  This routine is for sequential read from one or several 512 Bytes Page size.
  * @param  pBuffer: pointer on the Buffer to fill
  * @param  Address: First page address
  * @param  NumPageToRead: Number of page to read  
  * @retval New status of the NAND operation. This parameter can be:
  *              - NAND_TIMEOUT_ERROR: when the previous operation generate 
  *                a Timeout error
  *              - NAND_READY: when memory is ready for the next operation 
  *                And the new status of the increment address operation. It can be:
  *              - NAND_VALID_ADDRESS: When the new address is valid address
  *              - NAND_INVALID_ADDRESS: When the new address is invalid address
  */
uint32_t NAND_ReadSmallPage(uint8_t *pBuffer, NAND_ADDRESS Address, uint32_t NumPageToRead)
{
  uint32_t index = 0x00, numpageread = 0x00, addressstatus = NAND_VALID_ADDRESS;
  uint32_t status = NAND_READY, size = 0x00;

  while((NumPageToRead != 0x0) && (addressstatus == NAND_VALID_ADDRESS))
  {
    /*!< Page Read command and page address */
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_AREA_A;
   
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00; 
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(ROW_ADDRESS);
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(ROW_ADDRESS);
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(ROW_ADDRESS);
    
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_AREA_TRUE1;

    /*!< Calculate the size */
    size = NAND_PAGE_SIZE + (NAND_PAGE_SIZE * numpageread);
    
    /*!< Get Data into Buffer */    
    for(; index < size; index++)
    {
      pBuffer[index]= *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA);
    }

    numpageread++;
    
    NumPageToRead--;

    /*!< Calculate page address */
    addressstatus = NAND_AddressIncrement(&Address);
  }

  status = NAND_GetStatus();
  
  return (status | addressstatus);
}

/**
  * @brief  This routine write the spare area information for the specified
  *         pages addresses.  
  * @param  pBuffer: pointer on the Buffer containing data to be written 
  * @param  Address: First page address
  * @param  NumSpareAreaTowrite: Number of Spare Area to write
  * @retval New status of the NAND operation. This parameter can be:
  *              - NAND_TIMEOUT_ERROR: when the previous operation generate 
  *                a Timeout error
  *              - NAND_READY: when memory is ready for the next operation 
  *                And the new status of the increment address operation. It can be:
  *              - NAND_VALID_ADDRESS: When the new address is valid address
  *              - NAND_INVALID_ADDRESS: When the new address is invalid address
  */
uint32_t NAND_WriteSpareArea(uint8_t *pBuffer, NAND_ADDRESS Address, uint32_t NumSpareAreaTowrite)
{
  uint32_t index = 0x00, numsparesreawritten = 0x00, addressstatus = NAND_VALID_ADDRESS;
  uint32_t status = NAND_READY, size = 0x00; 

  while((NumSpareAreaTowrite != 0x00) && (addressstatus == NAND_VALID_ADDRESS) && (status == NAND_READY))
  {
    /*!< Page write Spare area command and address */
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_AREA_C;
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_WRITE0;

    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00; 
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(ROW_ADDRESS);
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(ROW_ADDRESS);
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(ROW_ADDRESS);

    /*!< Calculate the size */ 
    size = NAND_SPARE_AREA_SIZE + (NAND_SPARE_AREA_SIZE * numsparesreawritten);

    /*!< Write the data */ 
    for(; index < size; index++)
    {
      *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA) = pBuffer[index];
    }
    
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_WRITE_TRUE1;

    /*!< Check status for successful operation */
    status = NAND_GetStatus();

    if(status == NAND_READY)
    {
      numsparesreawritten++;

      NumSpareAreaTowrite--;
    
      /*!< Calculate Next page Address */
      addressstatus = NAND_AddressIncrement(&Address);
    }
  }
  
  return (status | addressstatus);
}

/**
  * @brief  This routine read the spare area information from the specified
  *         pages addresses.  
  * @param  pBuffer: pointer on the Buffer to fill 
  * @param  Address: First page address
  * @param  NumSpareAreaToRead: Number of Spare Area to read
  * @retval New status of the NAND operation. This parameter can be:
  *              - NAND_TIMEOUT_ERROR: when the previous operation generate 
  *                a Timeout error
  *              - NAND_READY: when memory is ready for the next operation 
  *                And the new status of the increment address operation. It can be:
  *              - NAND_VALID_ADDRESS: When the new address is valid address
  *              - NAND_INVALID_ADDRESS: When the new address is invalid address
  */
uint32_t NAND_ReadSpareArea(uint8_t *pBuffer, NAND_ADDRESS Address, uint32_t NumSpareAreaToRead)
{
  uint32_t numsparearearead = 0x00, index = 0x00, addressstatus = NAND_VALID_ADDRESS;
  uint32_t status = NAND_READY, size = 0x00;

  while((NumSpareAreaToRead != 0x0) && (addressstatus == NAND_VALID_ADDRESS))
  {     
    /*!< Page Read command and page address */
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_AREA_C;

    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00; 
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(ROW_ADDRESS);
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(ROW_ADDRESS);
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(ROW_ADDRESS);

    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_AREA_TRUE1;

    /*!< Data Read */
    size = NAND_SPARE_AREA_SIZE +  (NAND_SPARE_AREA_SIZE * numsparearearead);

    /*!< Get Data into Buffer */
    for ( ;index < size; index++)
    {
      pBuffer[index] = *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA);
    }
    
    numsparearearead++;
    
    NumSpareAreaToRead--;

    /*!< Calculate page address */
    addressstatus = NAND_AddressIncrement(&Address);
  }

  status = NAND_GetStatus();

  return (status | addressstatus);
}

/**
  * @brief  This routine erase complete block from NAND FLASH
  * @param  Address: Any address into block to be erased
  * @retval New status of the NAND operation. This parameter can be:
  *              - NAND_TIMEOUT_ERROR: when the previous operation generate 
  *                a Timeout error
  *              - NAND_READY: when memory is ready for the next operation 
  */
uint32_t NAND_EraseBlock(NAND_ADDRESS Address)
{
  *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_ERASE0;

  *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(ROW_ADDRESS);
  *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(ROW_ADDRESS);
  *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(ROW_ADDRESS);

  *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_ERASE1; 

  return (NAND_GetStatus());
}

/**
  * @brief  This routine reset the NAND FLASH.
  * @param  None
  * @retval NAND_READY
  */
uint32_t NAND_Reset(void)
{
  *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_RESET;

  return (NAND_READY);
}

/**
  * @brief  Get the NAND operation status.
  * @param  None
  * @retval New status of the NAND operation. This parameter can be:
  *              - NAND_TIMEOUT_ERROR: when the previous operation generate
  *                a Timeout error
  *              - NAND_READY: when memory is ready for the next operation
  */
uint32_t NAND_GetStatus(void)
{
  uint32_t timeout = 0x1000000, status = NAND_READY;

  status = NAND_ReadStatus();

  /*!< Wait for a NAND operation to complete or a TIMEOUT to occur */
  while ((status != NAND_READY) &&( timeout != 0x00))
  {
     status = NAND_ReadStatus();
     timeout --;
  }

  if(timeout == 0x00)
  {
    status =  NAND_TIMEOUT_ERROR;
  }

  /*!< Return the operation status */
  return (status);
}
  
/**
  * @brief  Reads the NAND memory status using the Read status command. 
  * @param  None
  * @retval The status of the NAND memory. This parameter can be:
  *              - NAND_BUSY: when memory is busy
  *              - NAND_READY: when memory is ready for the next operation
  *              - NAND_ERROR: when the previous operation gererates error
  */
uint32_t NAND_ReadStatus(void)
{
  uint32_t data = 0x00, status = NAND_BUSY;

  /*!< Read status operation ------------------------------------ */
  *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_STATUS;
  data = *(__IO uint8_t *)(Bank_NAND_ADDR);

  if((data & NAND_ERROR) == NAND_ERROR)
  {
    status = NAND_ERROR;
  } 
  else if((data & NAND_READY) == NAND_READY)
  {
    status = NAND_READY;
  }
  else
  {
    status = NAND_BUSY; 
  }
  
  return (status);
}

/**
  * @brief  Increment the NAND memory address. 
  * @param  Address: address to be incremented.
  * @retval The new status of the increment address operation. It can be:
  *              - NAND_VALID_ADDRESS: When the new address is valid address
  *              - NAND_INVALID_ADDRESS: When the new address is invalid address   
  */
uint32_t NAND_AddressIncrement(NAND_ADDRESS* Address)
{
  uint32_t status = NAND_VALID_ADDRESS;
 
  Address->Page++;

  if(Address->Page == NAND_BLOCK_SIZE)
  {
    Address->Page = 0;
    Address->Block++;
    
    if(Address->Block == NAND_ZONE_SIZE)
    {
      Address->Block = 0;
      Address->Zone++;

      if(Address->Zone == NAND_MAX_ZONE)
      {
        status = NAND_INVALID_ADDRESS;
      }
    }
  } 
  
  return (status);
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */  

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
#endif