
#ifndef FTL_H
#define FTL_H

/**
  ******************************************************************************
  * @file		FTL.h
  * @brief		Flash Translation Layer 提供介质无关的接口  .
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
  
#define MAX_INVALID_BLOCK_COUNT		128	/**< 最大允许的坏块数 */
#define INVALID_BLOCK_TAB_ADDR		((NF_BLOCK_COUNT - 1) * NF_PAGE_PER_BLOCK)
										/**< 坏块表的页地址 */	
										
#define BLOCK_DIRTY_BYTE_ADDR		515	/**< 当前页是否写入过数据 信息 地址 */										
#define BLOCK_INFO_BYTE_ADDR		517	/**< 坏块信息地址 */

#define BLOCK_BUF_UNIT_SIZE			(NF_PAGE_SIZE + NF_SPARE_SIZE + 1)	
										/**< 块拷贝缓存 每个单元的大小 */
#define BLOCK_BUF_PAGE_VALID		0	/**< 块拷贝缓存 页数据有效 */
#define BLOCK_BUF_PAGE_INVALID		0xff/**< 块拷贝缓存 页数据无效 */												

/** FTL 函数执行返回状态 */        
typedef enum 
{
	FTL_STATUS_SUCCESS	= 0,			/**< 执行成功 */
	FTL_STATUS_FAILED,					/**< 执行失败 */
	
	FTL_STATUS_IBT_FULL,				/**< 坏块表已满 */
		
	FTL_STATUS_INVALID_BLOCK			/**< 坏块 */
}FTL_STATUS_TypeDef;

/** FTL 信息结构体定义 */        
typedef struct 
{
	u32 SectorSize;						/**< 扇区大小 单位:字节 */
	u32 SectorPerBlock;					/**< 每块的扇区数 */	
	u32 BlockCount;						/**< 块数 */
}FTL_INFO_TypeDef;


extern u32 InvalidBlockTab[MAX_INVALID_BLOCK_COUNT];	/**< 坏块表缓存 */
extern FTL_INFO_TypeDef FTL_Info;						/**< 相关信息 */
/**
  * @}
  */
  
FTL_STATUS_TypeDef FTL_Init(void);
FTL_STATUS_TypeDef FTL_Format(void);
FTL_STATUS_TypeDef FTL_FlushBlockBuf(void);
FTL_STATUS_TypeDef FTL_ReadSector(u8 * Buffer, u32 SectorAddr);
FTL_STATUS_TypeDef FTL_WriteSector(u8 * Buffer, u32 SectorAddr);


#endif 	//#ifndef XXX_H
