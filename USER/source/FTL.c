
/**
  ******************************************************************************
  * @file		FTL.c
  * @brief		Flash Translation Layer  FLASH�����,�ṩ�����޹صĽӿ� .
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
  * @brief 		Flash Translation Layer �ڲ� .
  * @{
  */
  
FTL_INFO_TypeDef FTL_Info;							/**< �����Ϣ */  
u32 InvalidBlockTab[MAX_INVALID_BLOCK_COUNT];		/**< ������� */
u8 BlockBuf[NF_PAGE_PER_BLOCK][BLOCK_BUF_UNIT_SIZE];/**< BLOCK�������� */
u32 BlockBufAddr;									/**< BLOCK�������� �ĵ�ǰ���ַ */					
													
/**
  * @}
  */


/**
  ******************************************************************************	
  * @brief  	��ӳ�� .
  * @param  	BlockAddr : ���ַ
  * @retval 	��ȷ�Ŀ��ַ
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
  * @brief  	����ӳ�� .
  * @param  	SectorAddr : ������ַ
  * @retval 	��ȷ��������ַ
  * @note		�ڶ�д�����ĺ����ж�Ҫ���ô˺���
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
  * @brief  	�򻵿�����һ������ӳ��,����д��NandFlash .
  * @param  	BlockAddr : �����ַ
  * @retval 	��ȷ��������ַ
  * @note		�˺�������RAM�еĻ����,��������FLASH
  ******************************************************************************
  */
static FTL_STATUS_TypeDef FTL_InsertInvalidBlock(u32 BlockAddr)
{
	u32 i;
	
	for(i = 0; i < MAX_INVALID_BLOCK_COUNT; i++)
	{
		if((InvalidBlockTab[i] >> 16) == 0xFFFF)	//δʹ��
		{
			InvalidBlockTab[i] &= 0x0000FFFF;
			InvalidBlockTab[i] |= (BlockAddr << 16);
			NFWritePage((u8*)(&InvalidBlockTab[i]), (NF_BLOCK_COUNT - 1)\
	 					* NF_PAGE_PER_BLOCK, i << 2, 4);	//����
			return FTL_STATUS_SUCCESS;
		}	
	}
	
	return  FTL_STATUS_IBT_FULL;
}

/**
  ******************************************************************************	
  * @brief  	�ж�һ�����Ƿ񻵿� .
  * @param  	BlockAddr: ���ַ
  * @retval 	TURE ���� FALSE ����
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
  * @brief  	ɨ��FLASH������(������������)�Ŀ� .
  * @param  	��
  * @retval 	ɨ�赽�Ļ�����
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
				return (MAX_INVALID_BLOCK_COUNT + 1);	//̫�໵��
			}
		}
	}
	
	return i;	
}

/**
  ******************************************************************************	
  * @brief  	��FLASH�Ͻ�������� .
  * @param  	��
  * @retval 	�ο� FTL_STATUS_TypeDef ���� (FTL.h)
  * @note		��FLASH�ϲ����ڻ����ʱ,�ú�����ִ��
  ******************************************************************************
  */
static FTL_STATUS_TypeDef FTL_BuildInvalidBlockTab(void)
{
	NF_STATUS_TypeDef NFStatus;
	u32 CurBlock = NF_BLOCK_COUNT - 2;
	u32 i;
			
	for(i = 0; i < MAX_INVALID_BLOCK_COUNT;)	//��ʼ�������
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
	
	if(FTL_ScanInvalidBlock() > MAX_INVALID_BLOCK_COUNT)		//ɨ�軵��
	{															//����̫��
		return FTL_STATUS_IBT_FULL;
	}
		
	NFStatus = NFWritePage((u8*)InvalidBlockTab, (NF_BLOCK_COUNT - 1)\
	 					* NF_PAGE_PER_BLOCK, 0, sizeof(InvalidBlockTab));
												//д�뻵���
	
	return (NFStatus == NF_STATUS_SUCCESS) \
	 					? FTL_STATUS_SUCCESS : FTL_STATUS_FAILED;
}

/**
  ******************************************************************************	
  * @brief  	�����滻 .
  * @param  	BlockAddr : �����ַ������ִ����ɺ󣬱���������ĵ�ַ
  * @retval 	�ο� FTL_STATUS_TypeDef ���� (FTL.h)
  * @note		��ǻ��飬���»���ӳ�������������
  ******************************************************************************
  */
static FTL_STATUS_TypeDef FTL_ReplaceInvalidBlock(u32 *  BlockAddr )
{
	FTL_STATUS_TypeDef FTLStatus;
	
	FTLStatus = FTL_InsertInvalidBlock(*BlockAddr);	//�򻵿�ӳ����������
	if(FTLStatus != FTL_STATUS_SUCCESS)
	{
		return FTL_STATUS_FAILED;
	}
	
	*BlockAddr = FTL_BlockMap(*BlockAddr);			//����ӳ��
	
	return  FTL_STATUS_SUCCESS;
}

/**
  * @addtogroup 	FTLPublic
  * @{
  */ 
  
/**
  ******************************************************************************	
  * @brief  	���黺��������д��FLASH .
  * @param  	�� 
  * @retval 	�ο� FTL_STATUS_TypeDef ���� (FTL.h)
  * @note		
  ******************************************************************************
  */
FTL_STATUS_TypeDef FTL_FlushBlockBuf(void)
{
	u32 i;
	u8 DirtyByte;
	u32 BlockAddrBdry;
	NF_STATUS_TypeDef NFStatus;
	
	BlockAddrBdry = BlockBufAddr << NF_PAGE_PER_BLOCK_BIT;	//���뵽��߽������
	
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
														 //���Ϊ��Ч
			}
		}
		
	}
	
	NFStatus = NFErase(BlockBufAddr);						//������
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
  * @brief  	FTL��ʼ�� .
  * @param  	�� 
  * @retval 	�ο� FTL_STATUS_TypeDef ���� (FTL.h)
  * @note		��Ҫ���ڳ�ʼ�������
  ******************************************************************************
  */
FTL_STATUS_TypeDef FTL_Init(void)
{
	NF_STATUS_TypeDef status;
	u32 i;
		
	FTL_Info.SectorSize = NF_PAGE_SIZE;
	FTL_Info.SectorPerBlock = NF_PAGE_PER_BLOCK;
	
	status = NFReadPage((u8*)InvalidBlockTab, INVALID_BLOCK_TAB_ADDR \
						, 0, sizeof(InvalidBlockTab));	//��ȡ���������
	if(status == NF_STATUS_SUCCESS)
	{
		if(InvalidBlockTab[0] == 0xFFFFFFFF)			//�����ڻ����
		{
			FTL_BuildInvalidBlockTab();					//���������
		}
	}
	
	FTL_Info.BlockCount = (InvalidBlockTab[MAX_INVALID_BLOCK_COUNT - 1] \
							 & 0x0000FFFF) + 1;
	
	for(i = 0; i < NF_PAGE_PER_BLOCK; i++)
	{
		BlockBuf[i][BLOCK_BUF_UNIT_SIZE-1] = BLOCK_BUF_PAGE_INVALID;
	}	
														//��������Ϊ��Ч
	BlockBufAddr = 0;									//����� ��ַ��ʼ��Ϊ0
	
	return FTL_STATUS_SUCCESS;
}

/**
  ******************************************************************************	
  * @brief  	��ȡ���� .
  * @param  	Buffer : ��������ַ 
  * @param  	SectorAddr : ������ַ
  * @retval 	�ο� FTL_STATUS_TypeDef ���� (FTL.h)
  * @note		�����ṩ�ȶ��ɿ�������
  ******************************************************************************
  */
FTL_STATUS_TypeDef FTL_ReadSector(u8 * Buffer, u32 SectorAddr)
{
	NF_STATUS_TypeDef NFStatus;
	u32 BlockAddr;
	u32 PageAddr;
	
	SectorAddr = FTL_SectorMap(SectorAddr);				//����ӳ��
	BlockAddr = SectorAddr >> NF_PAGE_PER_BLOCK_BIT;	//���������ڿ��ַ
	PageAddr = SectorAddr & 0x0000001f;					//�������Ŀ���ҳ��ַ
	
	if((BlockAddr == BlockBufAddr) 				//��������Ϊ��ǰҳ���ڿ����
	&& ((BlockBuf[PageAddr][BLOCK_BUF_UNIT_SIZE - 1] == BLOCK_BUF_PAGE_VALID)))			
														//������Ч
	{
		memcpy(Buffer, (u8 *)BlockBuf[PageAddr], NF_PAGE_SIZE);//ֱ�ӿ���
	}
	else									
	{
		NFStatus = NFReadPage(Buffer, SectorAddr, 0, NF_PAGE_SIZE);
														//��FLASH��ȡҳ����
		if(NFStatus != NF_STATUS_SUCCESS)
		{
			return FTL_STATUS_FAILED;
		}
	}

	return  FTL_STATUS_SUCCESS;										   
}

/**
  ******************************************************************************	
  * @brief  	д���� .
  * @param  	Buffer : ��������ַ 
  * @param  	SectorAddr : ������ַ
  * @retval 	�ο� FTL_STATUS_TypeDef ���� (FTL.h)
  * @note		���Ա�֤��������ȷ��д������
  ******************************************************************************
  */
FTL_STATUS_TypeDef FTL_WriteSector(u8 * Buffer, u32 SectorAddr)
{
	FTL_STATUS_TypeDef FTLStatus;						//ִ��״̬
	u32 BlockAddr;										//���ַ
	u32 PageAddr;										//ҳ��ַ
	
	SectorAddr = FTL_SectorMap(SectorAddr);				//����ӳ��
	BlockAddr = SectorAddr >> NF_PAGE_PER_BLOCK_BIT;	//���������ڿ��ַ
	PageAddr = SectorAddr & 0x0000001f;					//�������Ŀ���ҳ��ַ
	
	if(BlockAddr != BlockBufAddr)						//��Ҫ��д����
	{
		FTLStatus = FTL_FlushBlockBuf();				//��д����
		
		while(FTLStatus != FTL_STATUS_SUCCESS)			//������
		{
			FTLStatus = FTL_ReplaceInvalidBlock(&BlockAddr);//�����滻
			if(FTLStatus != FTL_STATUS_SUCCESS)
			{
				return FTL_STATUS_IBT_FULL;				//���ÿ�����
			}
			FTLStatus = FTL_FlushBlockBuf();			//��д����
		}
		
		BlockBufAddr = BlockAddr;						//���»����ַ
	}
	memcpy((u8 *)BlockBuf[PageAddr], Buffer, NF_PAGE_SIZE);	
	BlockBuf[PageAddr][BLOCK_BUF_UNIT_SIZE-1] = BLOCK_BUF_PAGE_VALID;
														//���������Ч
	return FTL_STATUS_SUCCESS;
}

/**
  * @}
  */
  
