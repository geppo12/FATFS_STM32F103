/*******************************************************************************
* File Name       fsmc_nand.h
* Anthor          WXJ     
* Version         V1.00
* Date            2013/4/11
* Description     --    
*******************************************************************************/

#ifndef _fsmc_nand_h_
#define _fsmc_nand_h_

//---- Include -----------------------------------------------------------------
#include "typedef.h"

//---- Exported Funcation ------------------------------------------------------
  void      FSMC_NAND_Init          ( void );
  void      FSMC_NAND_ReadID        ( NAND_IDTypeDef* NAND_ID );
  uint32_t  FSMC_NAND_WriteSmallPage( uint8_t *pBuffer,
                                      uint32_t Address, 
                                      uint32_t NumPageToWrite );
  uint32_t  FSMC_NAND_ReadSmallPage ( uint8_t *pBuffer, 
                                      uint32_t Address, 
                                      uint32_t NumPageToRead );
  uint32_t  FSMC_NAND_MoveSmallPage ( uint32_t SourcePageAddress, 
                                      uint32_t TargetPageAddress );
  uint32_t  FSMC_NAND_EraseBlock    ( uint32_t Address );
  
#endif

/***************************** End of File ************************************/