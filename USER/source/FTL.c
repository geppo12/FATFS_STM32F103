
/**
  ******************************************************************************
  * @file		FTL.c
  * @brief		Flash Translation Layer  FLASH翻译层,提供介质无关的接口 .
  * @version  	1.0
  ******************************************************************************
  */
#include "stm32f10x.h"
#include "FTL.h"
#include "ff.h"
#include <string.h>
//#include "nandflash.h"
#include "fsmc_nand.h"

/**
  * @defgroup 	FTLPrivate
  * @brief 		Flash Translation Layer 内部 .
  * @{
  */
  
FTL_INFO_TypeDef FTL_Info;							/**< 相关信息 */  
u32 InvalidBlockTab[MAX_INVALID_BLOCK_COUNT];		/**< 坏块表缓存 */
u8 BlockBuf[NF_PAGE_PER_BLOCK][BLOCK_BUF_UNIT_SIZE];/**< BLOCK拷贝缓存 */
u32 BlockBufAddr;									/**< BLOCK拷贝缓存 的当前块地址 */					
													
/**
  * @}
  */


/**
  ******************************************************************************	
  * @brief  	块映射 .
  * @param  	BlockAddr : 块地址
  * @retval 	正确的块地址
  * @note		
  ******************************************************************************
  */
static u32 FTL_BlockMap(u32 BlockAddr)
{
	u32 i;
	
	for(i = 0; i < MAX_INVALID_BLOCK_COUNT; i++)
	{
		if((InvalidBlockTab[i] >> 16) == BlockAddr)
		{
			return (InvalidBlockTab[i] & 0x0000FFFF);
		}	
	}
	
	return  BlockAddr;
}

 /**
  ******************************************************************************	
  * @brief  	扇区映射 .
  * @param  	SectorAddr : 扇区地址
  * @retval 	正确的扇区地址
  * @note		在读写扇区的函数中都要调用此函数
  ******************************************************************************
  */
static u32 FTL_SectorMap(u32 SectorAddr)
{
	u32 BlockAddr = SectorAddr >> NF_PAGE_PER_BLOCK_BIT;
	BlockAddr = FTL_BlockMap(BlockAddr);					
	SectorAddr = (BlockAddr << NF_PAGE_PER_BLOCK_BIT) 
			   | (SectorAddr & 0x0000001f);				
	
	return  SectorAddr;
}

/**
  ******************************************************************************	
  * @brief  	向坏块表插入一个坏块映射,但不写入NandFlash .
  * @param  	BlockAddr : 坏块地址
  * @retval 	正确的扇区地址
  * @note		此函数操作RAM中的坏块表,并不更新FLASH
  ******************************************************************************
  */
static FTL_STATUS_TypeDef FTL_InsertInvalidBlock(u32 BlockAddr)
{
	u32 i;
	
	for(i = 0; i < MAX_INVALID_BLOCK_COUNT; i++)
	{
		if((InvalidBlockTab[i] >> 16) == 0xFFFF)	//未使用
		{
			InvalidBlockTab[i] &= 0x0000FFFF;
			InvalidBlockTab[i] |= (BlockAddr << 16);
			NFWritePage((u8*)(&InvalidBlockTab[i]), (NF_BLOCK_COUNT - 1)\
	 					* NF_PAGE_PER_BLOCK, i << 2, 4);	//更新
			return FTL_STATUS_SUCCESS;
		}	
	}
	
	return  FTL_STATUS_IBT_FULL;
}

/**
  ******************************************************************************	
  * @brief  	判断一个块是否坏块 .
  * @param  	BlockAddr: 块地址
  * @retval 	TURE 可用 FALSE 坏块
  * @note		
  ******************************************************************************
  */
static bool FTL_BlockIsInvalid(u32 BlockAddr)
{
	u8 BI[2];
	
	NFReadPage(BI, BlockAddr * NF_PAGE_PER_BLOCK, BLOCK_INFO_BYTE_ADDR, 1);	
																	//page0
	NFReadPage(BI+1, BlockAddr * NF_PAGE_PER_BLOCK + 1, BLOCK_INFO_BYTE_ADDR, 1);
																	//page1
											
	if((BI[0] != 0xFF) || (BI[1] != 0xFF))
	{
		return FALSE;	
	}
	else
	{
		return TRUE;
	}	
}

/**
  ******************************************************************************	
  * @brief  	扫描FLASH数据区(不包括备用区)的块 .
  * @param  	无
  * @retval 	扫描到的坏块数
  * @note		
  ******************************************************************************
  */
static u32 FTL_ScanInvalidBlock(void)
{
	u32 i;
	
	for(i = 0; i < FTL_Info.BlockCount; i++)
	{
		if(!FTL_BlockIsInvalid(i))
		{
			if(FTL_InsertInvalidBlock(i) == FTL_STATUS_IBT_FULL);
			{
				return (MAX_INVALID_BLOCK_COUNT + 1);	//太多坏块
			}
		}
	}
	
	return i;	
}

/**
  ******************************************************************************	
  * @brief  	在FLASH上建立坏块表 .
  * @param  	无
  * @retval 	参考 FTL_STATUS_TypeDef 类型 (FTL.h)
  * @note		当FLASH上不存在坏块表时,该函数会执行
  ******************************************************************************
  */
static FTL_STATUS_TypeDef FTL_BuildInvalidBlockTab(void)
{
	NF_STATUS_TypeDef NFStatus;
	u32 CurBlock = NF_BLOCK_COUNT - 2;
	u32 i;
			
	for(i = 0; i < MAX_INVALID_BLOCK_COUNT;)	//初始化坏块表
	{
		if(FTL_BlockIsInvalid(CurBlock))
		{
			InvalidBlockTab[i] = 0xFFFF0000 | CurBlock;
			CurBlock--;
			i++;	
		}
		else
		{
			CurBlock--;
			continue;
		}
		FTL_Info.BlockCount = (InvalidBlockTab[MAX_INVALID_BLOCK_COUNT - 1] \
							 & 0x0000FFFF) + 1;											
	}
	
	if(FTL_ScanInvalidBlock() > MAX_INVALID_BLOCK_COUNT)		//扫描坏块
	{															//坏块太多
		return FTL_STATUS_IBT_FULL;
	}
		
	NFStatus = NFWritePage((u8*)InvalidBlockTab, (NF_BLOCK_COUNT - 1)\
	 					* NF_PAGE_PER_BLOCK, 0, sizeof(InvalidBlockTab));
												//写入坏块表
	
	return (NFStatus == NF_STATUS_SUCCESS) \
	 					? FTL_STATUS_SUCCESS : FTL_STATUS_FAILED;
}

/**
  ******************************************************************************	
  * @brief  	坏块替换 .
  * @param  	BlockAddr : 坏块地址，函数执行完成后，保存正常块的地址
  * @retval 	参考 FTL_STATUS_TypeDef 类型 (FTL.h)
  * @note		标记坏块，更新坏块映射表，返回正常块
  ******************************************************************************
  */
static FTL_STATUS_TypeDef FTL_ReplaceInvalidBlock(u32 *  BlockAddr )
{
	FTL_STATUS_TypeDef FTLStatus;
	
	FTLStatus = FTL_InsertInvalidBlock(*BlockAddr);	//向坏块映射表插入新项
	if(FTLStatus != FTL_STATUS_SUCCESS)
	{
		return FTL_STATUS_FAILED;
	}
	
	*BlockAddr = FTL_BlockMap(*BlockAddr);			//坏块映射
	
	return  FTL_STATUS_SUCCESS;
}

/**
  * @addtogroup 	FTLPublic
  * @{
  */ 
  
/**
  ******************************************************************************	
  * @brief  	将块缓冲区数据写入FLASH .
  * @param  	无 
  * @retval 	参考 FTL_STATUS_TypeDef 类型 (FTL.h)
  * @note		
  ******************************************************************************
  */
FTL_STATUS_TypeDef FTL_FlushBlockBuf(void)
{
	u32 i;
	u8 DirtyByte;
	u32 BlockAddrBdry;
	NF_STATUS_TypeDef NFStatus;
	
	BlockAddrBdry = BlockBufAddr << NF_PAGE_PER_BLOCK_BIT;	//对齐到块边界的扇区
	
	for(i = 0; i < NF_PAGE_PER_BLOCK; i++)
	{
		if(BlockBuf[i][BLOCK_BUF_UNIT_SIZE - 1] == BLOCK_BUF_PAGE_INVALID)
		{
			NFReadPage(&DirtyByte, BlockAddrBdry + i, BLOCK_DIRTY_BYTE_ADDR, 1);
			if(DirtyByte != 0xFF)
			{
				NFReadPage((u8 *)(BlockBuf + i) ,BlockAddrBdry + i,\
							 0, BLOCK_BUF_UNIT_SIZE - 1);
				BlockBuf[i][BLOCK_BUF_UNIT_SIZE - 1] = BLOCK_BUF_PAGE_VALID;
														 //标记为有效
			}
		}
		
	}
	
	NFStatus = NFErase(BlockBufAddr);						//擦除块
	if(NFStatus != NF_STATUS_SUCCESS)
	{
		return FTL_STATUS_FAILED;
	}
		
	for(i = 0; i <  NF_PAGE_PER_BLOCK; i++)
	{			
		if((BlockBuf[i][BLOCK_BUF_UNIT_SIZE - 1] == BLOCK_BUF_PAGE_VALID))
		{
			NFStatus = NFWritePage((u8 *)(BlockBuf + i), BlockAddrBdry + i, \
								 0, BLOCK_BUF_UNIT_SIZE - 1);
			if(NFStatus != NF_STATUS_SUCCESS)
			{
				return FTL_STATUS_FAILED;
			}
			BlockBuf[i][BLOCK_BUF_UNIT_SIZE-1] = BLOCK_BUF_PAGE_INVALID;
		}
	}
	
	return FTL_STATUS_SUCCESS;
}

/**
  ******************************************************************************	
  * @brief  	FTL初始化 .
  * @param  	无 
  * @retval 	参考 FTL_STATUS_TypeDef 类型 (FTL.h)
  * @note		主要用于初始化坏块表
  ******************************************************************************
  */
FTL_STATUS_TypeDef FTL_Init(void)
{
	NF_STATUS_TypeDef status;
	u32 i;
		
	FTL_Info.SectorSize = NF_PAGE_SIZE;
	FTL_Info.SectorPerBlock = NF_PAGE_PER_BLOCK;
	
	status = NFReadPage((u8*)InvalidBlockTab, INVALID_BLOCK_TAB_ADDR \
						, 0, sizeof(InvalidBlockTab));	//读取坏块表扇区
	if(status == NF_STATUS_SUCCESS)
	{
		if(InvalidBlockTab[0] == 0xFFFFFFFF)			//不存在坏块表
		{
			FTL_BuildInvalidBlockTab();					//建立坏块表
		}
	}
	
	FTL_Info.BlockCount = (InvalidBlockTab[MAX_INVALID_BLOCK_COUNT - 1] \
							 & 0x0000FFFF) + 1;
	
	for(i = 0; i < NF_PAGE_PER_BLOCK; i++)
	{
		BlockBuf[i][BLOCK_BUF_UNIT_SIZE-1] = BLOCK_BUF_PAGE_INVALID;
	}	
														//将缓存标记为无效
	BlockBufAddr = 0;									//缓存块 地址初始化为0
	
	return FTL_STATUS_SUCCESS;
}

/**
  ******************************************************************************	
  * @brief  	读取扇区 .
  * @param  	Buffer : 缓冲区地址 
  * @param  	SectorAddr : 扇区地址
  * @retval 	参考 FTL_STATUS_TypeDef 类型 (FTL.h)
  * @note		可以提供稳定可靠的数据
  ******************************************************************************
  */
FTL_STATUS_TypeDef FTL_ReadSector(u8 * Buffer, u32 SectorAddr)
{
	NF_STATUS_TypeDef NFStatus;
	u32 BlockAddr;
	u32 PageAddr;
	
	SectorAddr = FTL_SectorMap(SectorAddr);				//扇区映射
	BlockAddr = SectorAddr >> NF_PAGE_PER_BLOCK_BIT;	//该扇区所在块地址
	PageAddr = SectorAddr & 0x0000001f;					//该扇区的块内页地址
	
	if((BlockAddr == BlockBufAddr) 				//缓存正在为当前页所在块服务
	&& ((BlockBuf[PageAddr][BLOCK_BUF_UNIT_SIZE - 1] == BLOCK_BUF_PAGE_VALID)))			
														//缓存有效
	{
		memcpy(Buffer, (u8 *)BlockBuf[PageAddr], NF_PAGE_SIZE);//直接拷贝
	}
	else									
	{
		NFStatus = NFReadPage(Buffer, SectorAddr, 0, NF_PAGE_SIZE);
														//从FLASH读取页数据
		if(NFStatus != NF_STATUS_SUCCESS)
		{
			return FTL_STATUS_FAILED;
		}
	}

	return  FTL_STATUS_SUCCESS;										   
}

/**
  ******************************************************************************	
  * @brief  	写扇区 .
  * @param  	Buffer : 缓冲区地址 
  * @param  	SectorAddr : 扇区地址
  * @retval 	参考 FTL_STATUS_TypeDef 类型 (FTL.h)
  * @note		可以保证将数据正确地写入扇区
  ******************************************************************************
  */
FTL_STATUS_TypeDef FTL_WriteSector(u8 * Buffer, u32 SectorAddr)
{
	FTL_STATUS_TypeDef FTLStatus;						//执行状态
	u32 BlockAddr;										//块地址
	u32 PageAddr;										//页地址
	
	SectorAddr = FTL_SectorMap(SectorAddr);				//扇区映射
	BlockAddr = SectorAddr >> NF_PAGE_PER_BLOCK_BIT;	//该扇区所在块地址
	PageAddr = SectorAddr & 0x0000001f;					//该扇区的块内页地址
	
	if(BlockAddr != BlockBufAddr)						//需要回写缓存
	{
		FTLStatus = FTL_FlushBlockBuf();				//回写缓存
		
		while(FTLStatus != FTL_STATUS_SUCCESS)			//错误处理
		{
			FTLStatus = FTL_ReplaceInvalidBlock(&BlockAddr);//坏块替换
			if(FTLStatus != FTL_STATUS_SUCCESS)
			{
				return FTL_STATUS_IBT_FULL;				//备用块已满
			}
			FTLStatus = FTL_FlushBlockBuf();			//回写缓存
		}
		
		BlockBufAddr = BlockAddr;						//更新缓存地址
	}
	memcpy((u8 *)BlockBuf[PageAddr], Buffer, NF_PAGE_SIZE);	
	BlockBuf[PageAddr][BLOCK_BUF_UNIT_SIZE-1] = BLOCK_BUF_PAGE_VALID;
														//标记数据有效
	return FTL_STATUS_SUCCESS;
}

/**
  * @}
  */
  
