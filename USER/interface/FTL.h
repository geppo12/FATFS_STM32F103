
#ifndef FTL_H
#define FTL_H

/**
  ******************************************************************************
  * @file		FTL.h
  * @brief		Flash Translation Layer �ṩ�����޹صĽӿ�  .
  * @version  	1.0
  ******************************************************************************
  */

/**
  * @defgroup 	FTLPrivate
  * @brief 		 .
  * @{
  */

/**
  * @}
  */

/**
  * @addtogroup 	FTLPublic
  * @{
  */ 
  
#define MAX_INVALID_BLOCK_COUNT		128	/**< �������Ļ����� */
#define INVALID_BLOCK_TAB_ADDR		((NF_BLOCK_COUNT - 1) * NF_PAGE_PER_BLOCK)
										/**< ������ҳ��ַ */	
										
#define BLOCK_DIRTY_BYTE_ADDR		515	/**< ��ǰҳ�Ƿ�д������� ��Ϣ ��ַ */										
#define BLOCK_INFO_BYTE_ADDR		517	/**< ������Ϣ��ַ */

#define BLOCK_BUF_UNIT_SIZE			(NF_PAGE_SIZE + NF_SPARE_SIZE + 1)	
										/**< �鿽������ ÿ����Ԫ�Ĵ�С */
#define BLOCK_BUF_PAGE_VALID		0	/**< �鿽������ ҳ������Ч */
#define BLOCK_BUF_PAGE_INVALID		0xff/**< �鿽������ ҳ������Ч */												

/** FTL ����ִ�з���״̬ */        
typedef enum 
{
	FTL_STATUS_SUCCESS	= 0,			/**< ִ�гɹ� */
	FTL_STATUS_FAILED,					/**< ִ��ʧ�� */
	
	FTL_STATUS_IBT_FULL,				/**< ��������� */
		
	FTL_STATUS_INVALID_BLOCK			/**< ���� */
}FTL_STATUS_TypeDef;

/** FTL ��Ϣ�ṹ�嶨�� */        
typedef struct 
{
	u32 SectorSize;						/**< ������С ��λ:�ֽ� */
	u32 SectorPerBlock;					/**< ÿ��������� */	
	u32 BlockCount;						/**< ���� */
}FTL_INFO_TypeDef;


extern u32 InvalidBlockTab[MAX_INVALID_BLOCK_COUNT];	/**< ������� */
extern FTL_INFO_TypeDef FTL_Info;						/**< �����Ϣ */
/**
  * @}
  */
  
FTL_STATUS_TypeDef FTL_Init(void);
FTL_STATUS_TypeDef FTL_Format(void);
FTL_STATUS_TypeDef FTL_FlushBlockBuf(void);
FTL_STATUS_TypeDef FTL_ReadSector(u8 * Buffer, u32 SectorAddr);
FTL_STATUS_TypeDef FTL_WriteSector(u8 * Buffer, u32 SectorAddr);


#endif 	//#ifndef XXX_H
