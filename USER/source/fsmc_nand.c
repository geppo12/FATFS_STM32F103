/*******************************************************************************
* File Name       fsmc_nand.c
* Anthor          WXJ     
* Version         V1.00
* Date            2013/4/11
* Description     --    
*******************************************************************************/

//---- Include -----------------------------------------------------------------
#include "macro.h"
#include "typedef.h"
 
//---- Private Function Prototypes ---------------------------------------------
uint32_t  FSMC_NAND_Reset      ( void );
uint32_t  FSMC_NAND_GetStatus  ( void );
uint32_t  FSMC_NAND_ReadStatus ( void ); 

//---- Private Function --------------------------------------------------------
/*******************************************************************************
* Function Nmae     FSMC_NAND_Init
* Date              2013/4/11
* Anthor            WXJ
* Input Parameter   None
* Output Parameter  None
* Description       --
* File              fsmc_nand.c
*******************************************************************************/
void FSMC_NAND_Init(void)
{
    GPIO_InitTypeDef        GPIO_InitStructure; 
    FSMC_NANDInitTypeDef    FSMC_NANDInitStructure;
    FSMC_NAND_PCCARDTimingInitTypeDef  p;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | 
                           RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG, ENABLE);
    

    /* GPIO Gonfigure */
    /* CLE, ALE, D0->D3, NOE, NWE and NCE2  NAND pin configuration  */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_14 | 
                                  GPIO_Pin_15 | GPIO_Pin_0 | GPIO_Pin_1 |
                                  GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7;                                  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    
    GPIO_Init(GPIOD, &GPIO_InitStructure); 
    
    /* D4->D7 NAND pin configuration  */  
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | 
                                  GPIO_Pin_10;
    
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    
    
    /* NWAIT NAND pin configuration */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;           
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    
    GPIO_Init(GPIOD, &GPIO_InitStructure); 
    
    /* INT2 NAND pin configuration */  
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;           
    GPIO_Init(GPIOG, &GPIO_InitStructure);
    
    /* Fsmc Configrue */
    p.FSMC_SetupTime = 0x1;
    p.FSMC_WaitSetupTime = 0x3;
    p.FSMC_HoldSetupTime = 0x2;
    p.FSMC_HiZSetupTime = 0x1;
    
    FSMC_NANDInitStructure.FSMC_Bank = FSMC_Bank2_NAND;
    FSMC_NANDInitStructure.FSMC_Waitfeature = FSMC_Waitfeature_Enable;
    FSMC_NANDInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_8b;
    FSMC_NANDInitStructure.FSMC_ECC = FSMC_ECC_Enable;
    FSMC_NANDInitStructure.FSMC_ECCPageSize = FSMC_ECCPageSize_512Bytes;
    FSMC_NANDInitStructure.FSMC_TCLRSetupTime = 0x00;
    FSMC_NANDInitStructure.FSMC_TARSetupTime = 0x00;
    FSMC_NANDInitStructure.FSMC_CommonSpaceTimingStruct = &p;
    FSMC_NANDInitStructure.FSMC_AttributeSpaceTimingStruct = &p;
    
    FSMC_NANDInit(&FSMC_NANDInitStructure);
    
    /* FSMC NAND Bank Cmd Test */
    FSMC_NANDCmd(FSMC_Bank2_NAND, ENABLE);
} /* End of FSMC_NAND_Init */
    
/*******************************************************************************
* Function Nmae     FSMC_NAND_ReadID
* Date              2013/4/11
* Anthor            WXJ
* Input Parameter   None
* Output Parameter  None
* Description       --
* File              fsmc_nand.c
*******************************************************************************/
void FSMC_NAND_ReadID(NAND_IDTypeDef* NAND_ID)
{
    uint32_t data = 0;
    
    /* Send Command to the command area */  
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA)  = NAND_CMD_READID;
    /* Send Address to the address area */ 
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = NAND_CMD_IDADDR;
    
    /* Sequence to read ID from NAND flash */ 
    data = *(__IO uint32_t *)(Bank_NAND_ADDR | DATA_AREA);
    
    NAND_ID->Maker_ID   = DATA_1st_CYCLE (data);
    NAND_ID->Device_ID  = DATA_2nd_CYCLE (data);
    NAND_ID->Third_ID   = DATA_3rd_CYCLE (data);
    NAND_ID->Fourth_ID  = DATA_4th_CYCLE (data);  
}
  
/*******************************************************************************
* Function Nmae     FSMC_NAND_MoveSmallPage
* Date              2013/4/11
* Anthor            WXJ
* Input Parameter   None
* Output Parameter  None
* Description       --
* File              fsmc_nand.c
*******************************************************************************/
uint32_t FSMC_NAND_MoveSmallPage(uint32_t SourcePageAddress, uint32_t TargetPageAddress)
{
    uint32_t status = NAND_READY ;
    uint32_t data = 0xff;
    
    /* Page write command and address */
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_MOVE0;
    
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(SourcePageAddress);  
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(SourcePageAddress);  
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(SourcePageAddress);  
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_4th_CYCLE(SourcePageAddress); 
    
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_MOVE1;
    
    while( GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_6) == 0 );
    
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_MOVE2;
    
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(TargetPageAddress);  
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(TargetPageAddress);  
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(TargetPageAddress);  
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_4th_CYCLE(TargetPageAddress); 
    
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_MOVE3;
    
    while( GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_6) == 0 );   
    
    /* Check status for successful operation */
    status = FSMC_NAND_GetStatus();
    
    data = *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA);
    if(!(data&0x1)) status = NAND_READY;
    
    return (status);
}
/**
* @brief  This routine is for writing one or several 2048 Bytes Page size.
* @param  pBuffer: pointer on the Buffer containing data to be written 
* @param  PageAddress: First page address
* @param  NumPageToWrite: Number of page to write  
* @retval : New status of the NAND operation. This parameter can be:
*              - NAND_TIMEOUT_ERROR: when the previous operation generate 
*                a Timeout error
*              - NAND_READY: when memory is ready for the next operation 
*                And the new status of the increment address operation. It can be:
*              - NAND_VALID_ADDRESS: When the new address is valid address
*              - NAND_INVALID_ADDRESS: When the new address is invalid address  
*/

uint32_t FSMC_NAND_WriteSmallPage(uint8_t *pBuffer, uint32_t PageAddress, uint32_t NumPageToWrite)
{
    uint32_t index = 0x00, numpagewritten = 0x00,addressstatus = NAND_VALID_ADDRESS;
    uint32_t status = NAND_READY, size = 0x00;
    uint32_t data = 0xff;
    
    while((NumPageToWrite != 0x00) && (addressstatus == NAND_VALID_ADDRESS) && (status == NAND_READY))
    {
        /* Page write command and address */
        *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_WRITE0;
        
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(PageAddress);  
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(PageAddress);  
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(PageAddress);  
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_4th_CYCLE(PageAddress); 
        
        /* Calculate the size */
        size = NAND_PAGE_SIZE + (NAND_PAGE_SIZE * numpagewritten);
        
        /* Write data */
        for(; index < size; index++)
        {
            *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA) = pBuffer[index];
        }
        
        *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_WRITE1;
        
        while( GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_6) == 0 );
        
        /* Check status for successful operation */
        status = FSMC_NAND_GetStatus();
        
        data = *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA);
        if(!(data&0x1)) status = NAND_READY;
        
        if(status == NAND_READY)
        {
            numpagewritten++; NumPageToWrite--;
            
            /* Calculate Next small page Address */
            if(PageAddress++ > (NAND_MAX_ZONE*NAND_ZONE_SIZE*NAND_BLOCK_SIZE))
            { addressstatus = NAND_INVALID_ADDRESS;}  
        }    
    }
    
    return (status | addressstatus);
}

/**
* @brief  This routine is for sequential read from one or several
*         2048 Bytes Page size.  
* @param  pBuffer: pointer on the Buffer to fill
* @param  PageAddress: First page address
* @param  NumPageToRead: Number of page to read  
* @retval : New status of the NAND operation. This parameter can be:
*              - NAND_TIMEOUT_ERROR: when the previous operation generate 
*                a Timeout error
*              - NAND_READY: when memory is ready for the next operation 
*                And the new status of the increment address operation. It can be:
*              - NAND_VALID_ADDRESS: When the new address is valid address
*              - NAND_INVALID_ADDRESS: When the new address is invalid address
*/


uint32_t FSMC_NAND_ReadSmallPage(uint8_t *pBuffer, uint32_t PageAddress, uint32_t NumPageToRead)
{
    uint32_t index = 0x00, numpageread = 0x00, addressstatus = NAND_VALID_ADDRESS;
    uint32_t status = NAND_READY, size = 0x00;
    
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_READ1; 
    
    while((NumPageToRead != 0x0) && (addressstatus == NAND_VALID_ADDRESS))
    {    
        /* Page Read command and page address */
        
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(PageAddress); 
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(PageAddress); 
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(PageAddress); 
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_4th_CYCLE(PageAddress); 
        
        *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_READ2; 
        
        while( GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_6) == 0 );
        
        /* Calculate the size */
        size = NAND_PAGE_SIZE + (NAND_PAGE_SIZE * numpageread);
        
        /* Get Data into Buffer */    
        for(; index < size; index++)
        {
            pBuffer[index]= *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA);
        }
        
        numpageread++; NumPageToRead--;
        
        /* Calculate page address */               
        if(PageAddress++ > (NAND_MAX_ZONE*NAND_ZONE_SIZE*NAND_BLOCK_SIZE))
        { addressstatus = NAND_INVALID_ADDRESS;} 
    }
    
    status = FSMC_NAND_GetStatus();
    
    return (status | addressstatus);
}

/**
* @brief  This routine erase complete block from NAND FLASH
* @param  PageAddress: Any address into block to be erased
* @retval :New status of the NAND operation. This parameter can be:
*              - NAND_TIMEOUT_ERROR: when the previous operation generate 
*                a Timeout error
*              - NAND_READY: when memory is ready for the next operation 
*/

uint32_t FSMC_NAND_EraseBlock(uint32_t PageAddress)
{
    uint32_t data = 0xff, status = NAND_ERROR;
    
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_ERASE0;
    
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(PageAddress);
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_4th_CYCLE(PageAddress);
    
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_ERASE1; 
    
    while( GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_6) == 0 );
    
    /* Read status operation ------------------------------------ */  
    FSMC_NAND_GetStatus();
    
    data = *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA);
    
    if(!(data&0x1)) status = NAND_READY;
    
    return (status);
}

/**
* @brief  This routine reset the NAND FLASH
* @param  None
* @retval :NAND_READY
*/

uint32_t FSMC_NAND_Reset(void)
{
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_RESET;
    
    return (NAND_READY);
}

/**
* @brief  Get the NAND operation status
* @param  None
* @retval :New status of the NAND operation. This parameter can be:
*              - NAND_TIMEOUT_ERROR: when the previous operation generate 
*                a Timeout error
*              - NAND_READY: when memory is ready for the next operation    
*/


uint32_t FSMC_NAND_GetStatus(void)
{
    uint32_t timeout = 0x1000000, status = NAND_READY;
    
    status = FSMC_NAND_ReadStatus(); 
    
    /* Wait for a NAND operation to complete or a TIMEOUT to occur */
    while ((status != NAND_READY) &&( timeout != 0x00))
    {
        status = FSMC_NAND_ReadStatus();
        timeout --;      
    }
    
    if(timeout == 0x00)
    {          
        status =  NAND_TIMEOUT_ERROR;      
    } 
    
    /* Return the operation status */
    return (status);      
}

/**
* @brief  Reads the NAND memory status using the Read status command 
* @param  None
* @retval :The status of the NAND memory. This parameter can be:
*              - NAND_BUSY: when memory is busy
*              - NAND_READY: when memory is ready for the next operation    
*              - NAND_ERROR: when the previous operation gererates error     
*/

uint32_t FSMC_NAND_ReadStatus(void)
{
    uint32_t data = 0x00, status = NAND_BUSY;
    
    /* Read status operation ------------------------------------ */
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

/***************************** End of File ************************************/