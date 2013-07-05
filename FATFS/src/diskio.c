/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* by grqd_xp                                                            */
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "fsmc_nand.h"
#include "diskio.h"
#include "macro.h"


/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */
/* Note that Tiny-FatFs supports only single drive and always            */
/* accesses drive number 0.                                              */


/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */

DSTATUS disk_initialize (
	BYTE drv				/* Physical drive nmuber (0..) */
)
{	
//	FTL_Init();
	
	return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0..) */
)
{	
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
DRESULT disk_read (BYTE drv,BYTE *buff,DWORD sector,BYTE count)
  {
   uint32_t result;

      if (drv || !count){  return RES_PARERR;}
   result = FSMC_NAND_ReadSmallPage(buff, sector, count);                                                
      if(result & NAND_READY){  return RES_OK; }
      else { return RES_ERROR;  }
  }



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if _READONLY == 0
#if _BACKUP == 1
  DRESULT disk_write (BYTE drv,const BYTE *buff,DWORD sector,BYTE count)
  {
   uint32_t result;
   uint32_t BackupBlockAddr;
   uint32_t WriteBlockAddr;
   uint16_t IndexTmp = 0;
   uint16_t OffsetPage;
  
   /* NAND memory write page at block address*/
   WriteBlockAddr = (sector/NAND_BLOCK_SIZE);
   /* NAND memory backup block address*/
   BackupBlockAddr = (WriteBlockAddr + (NAND_MAX_ZONE/2)*NAND_ZONE_SIZE);
   OffsetPage = sector%NAND_BLOCK_SIZE;
  
      if (drv || !count){  return RES_PARERR;}
    
   /* Erase the NAND backup Block */
      result = FSMC_NAND_EraseBlock(BackupBlockAddr*NAND_BLOCK_SIZE);
  
      /* Backup the NAND Write Block to High zone*/
  
   for (IndexTmp = 0; IndexTmp < NAND_BLOCK_SIZE; IndexTmp++ )
   {
    FSMC_NAND_MoveSmallPage (WriteBlockAddr*NAND_BLOCK_SIZE+IndexTmp,BackupBlockAddr*NAND_BLOCK_SIZE+IndexTmp);
   }
  
   /* Erase the NAND Write Block */
   result = FSMC_NAND_EraseBlock(WriteBlockAddr*NAND_BLOCK_SIZE);
  
      /*return write the block  with modify*/
   for (IndexTmp = 0; IndexTmp < NAND_BLOCK_SIZE; IndexTmp++ )
   {
    if((IndexTmp>=OffsetPage)&&(IndexTmp < (OffsetPage+count)))
    {
     FSMC_NAND_WriteSmallPage((uint8_t *)buff, WriteBlockAddr*NAND_BLOCK_SIZE+IndexTmp, 1);
     buff = (uint8_t *)buff + NAND_PAGE_SIZE;
       }
    else
    {
          FSMC_NAND_MoveSmallPage (BackupBlockAddr*NAND_BLOCK_SIZE+IndexTmp,WriteBlockAddr*NAND_BLOCK_SIZE+IndexTmp);
       }
   }    
   
      if(result == NAND_READY){   return RES_OK;}
      else {   return RES_ERROR;}
  }
#else
    DRESULT disk_write (BYTE drv,const BYTE *buff,DWORD sector,BYTE count)
  {
   uint32_t result;
   uint32_t BackupBlockAddr;
   uint32_t WriteBlockAddr;
   uint16_t IndexTmp = 0;
   uint16_t OffsetPage;
  
   /* NAND memory write page at block address*/
   WriteBlockAddr = (sector/NAND_BLOCK_SIZE);
   /* NAND memory backup block address*/
   BackupBlockAddr = (NAND_MAX_ZONE*NAND_ZONE_SIZE-1);
   OffsetPage = sector%NAND_BLOCK_SIZE;
  
      if (drv || !count){  return RES_PARERR;}
    
   /* Erase the NAND backup Block */
     result = FSMC_NAND_EraseBlock(BackupBlockAddr*NAND_BLOCK_SIZE);
  
      /* Backup the NAND Write Block to High zone*/
//  
   for (IndexTmp = 0; IndexTmp < NAND_BLOCK_SIZE; IndexTmp++ )
   {
    FSMC_NAND_MoveSmallPage (WriteBlockAddr*NAND_BLOCK_SIZE+IndexTmp,BackupBlockAddr*NAND_BLOCK_SIZE+IndexTmp);
   }

   /* Erase the NAND Write Block */
   result = FSMC_NAND_EraseBlock(WriteBlockAddr*NAND_BLOCK_SIZE);
  
      /*return write the block  with modify*/
   for (IndexTmp = 0; IndexTmp < NAND_BLOCK_SIZE; IndexTmp++ )
   {
    if((IndexTmp>=OffsetPage)&&(IndexTmp < (OffsetPage+count)))
    {
     FSMC_NAND_WriteSmallPage((uint8_t *)buff, WriteBlockAddr*NAND_BLOCK_SIZE+IndexTmp, 1);
     buff = (uint8_t *)buff + NAND_PAGE_SIZE;
       }
    else
    {
          FSMC_NAND_MoveSmallPage (BackupBlockAddr*NAND_BLOCK_SIZE+IndexTmp,WriteBlockAddr*NAND_BLOCK_SIZE+IndexTmp);
       }
   }    
   
      if(result == NAND_READY){   return RES_OK;}
      else {   return RES_ERROR;}
  }
#endif 

#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
#if 0
DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	switch(ctrl)
	{
	case CTRL_SYNC:
		FTL_FlushBlockBuf();						//将缓存写回FLASH
		break;
		
	case GET_SECTOR_COUNT:
		*(u32 *)buff =  FTL_Info.SectorPerBlock * FTL_Info.BlockCount;
		break;										//获取扇区数
		
	case GET_SECTOR_SIZE:
		*(u32 *)buff = FTL_Info.SectorSize ;		//获取扇区字节数
		break;
		
	case GET_BLOCK_SIZE:
		*(u32 *)buff = FTL_Info.SectorPerBlock;		//获取每个块的扇区数
		break;
		
	default :
		*(u32 *)buff = NULL;
		break;
	}
	
	return RES_OK;
}
#endif
DRESULT disk_ioctl (BYTE drv,BYTE ctrl, void *buff)
  {
   DRESULT res = RES_OK;


//      if (drv){ return RES_PARERR;}
    
      switch(ctrl)
      {
       case CTRL_SYNC:
           break;
    case GET_BLOCK_SIZE:
           *(DWORD*)buff = NAND_BLOCK_SIZE;
           break;
    case GET_SECTOR_COUNT:
           *(DWORD*)buff = (int32_t)NAND_BLOCK_SIZE * (int32_t)NAND_ZONE_SIZE *(int32_t)NAND_MAX_ZONE;
           break;
    case GET_SECTOR_SIZE:
           *(WORD*)buff = NAND_PAGE_SIZE;
           break;
       default:
           res = RES_PARERR;
           break;
   }
      return res;
  }


DWORD get_fattime(void)
{
	return 0;
}























