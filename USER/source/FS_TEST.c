/*******************************************************************************
* File Name       FS_TEST.c
* Anthor          WXJ     
* Version         V1.00
* Date            2013/4/11
* Description     --    
*******************************************************************************/

//---- Include -----------------------------------------------------------------
#include "macro.h"
#include "typedef.h"
#include "stdio.h"
#include "ff.h"
#include "string.h"

//---- Private Variables -------------------------------------------------------
FATFS       fs;
FIL         fl;
bool 	    FileNameFlag = FALSE, 
            USART1_CmdFlag = FALSE;

uint32_t    USART1_FileNameTemp = 0;
uint8_t	    USART1_CmdCounter = 0, 
            USART1_CmdBuffer[FS_FILE_NAME_SIZE] = {0},
            RxBuffer[NAND_PAGE_SIZE],
            FileBuffer[NAND_PAGE_SIZE];
        
//---- Private Function Prototypes ---------------------------------------------
bool CheckFileName  ( uint8_t * FileName, uint8_t length );
bool GetFileName    ( uint8_t * FileName, uint8_t * length );
//---- Private Function --------------------------------------------------------
/*******************************************************************************
* Function Nmae     FormatDisk
* Date              2013/4/11
* Anthor            WXJ
* Input Parameter   None
* Output Parameter  None
* Description       --
* File              FS_TEST.c
*******************************************************************************/
void FormatDisk (void)    
{  
    FRESULT  res; 
    
    res = f_mount(0,&fs);
    if (res != FR_OK)
    {
        printf("\r\nMount the file system fails! ERROR CODE: %u",res);
        return;
    }
    
    printf ("\r\nFormatting...");
    res=f_mkfs(0,0,2048);
    printf ("\r\nComplete,f_mkfs = %x",res);
    
    f_mount(0,NULL);
    printf ("\r\nComplete,f_mount = %x",res);
} /* End of FormatDisk */

/*******************************************************************************
* Function Nmae     GetDiskInfo
* Date              2013/4/11
* Anthor            WXJ
* Input Parameter   None
* Output Parameter  None
* Description       --
* File              FS_TEST.c
*******************************************************************************/
bool GetDiskInfo ( void )
{
    FATFS    *pfs;
    DWORD    clust;
    FRESULT  res;
    bool     RESULT;
    
    res = f_mount(0,&fs);
    if (res != FR_OK)
    {
        printf("\r\nLoad the file system fails! ERROR CODE: %u",res);
    }
    
    pfs=&fs;
    res = f_getfree("/", &clust, &pfs);
    
    if ( res ==FR_OK )
    {
        printf("\r\n%lu MB total drive space."
               "\r\n%lu MB available.\n\r",
               (DWORD)(pfs->n_fatent - 2) * pfs->csize *2/1024,
               clust * pfs->csize *2/1024); 
        RESULT = TRUE;
    }
    else
    {
        printf("\r\nCan not get the disk information! ERROR CODE: %u",res);
    }
    
    f_mount(0,NULL);
    
    return RESULT;
} /* End of GetDiskInfo */

/*******************************************************************************
* Function Nmae     USART_GetCmd
* Date              2013/4/11
* Anthor            WXJ
* Input Parameter   None
* Output Parameter  None
* Description       --
* File              FS_TEST.c
*******************************************************************************/
FS_CMD USART1_GetCmd ( void )
{   
    uint8_t USART1_CmdTemp = 'a';
    
   while ( 1 )
    {
        if ( USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET)
        {
            USART1_CmdTemp = ( uint8_t )USART_ReceiveData ( USART1 );	
            break;
        }
    } 
    
    switch ( USART1_CmdTemp )
    {
    case 'F' :
    case 'f' :    
        return FS_FORMAT_DISK;
    case 'O' :
    case 'o' :
        return FS_OPEN_FILE;
    case 'R' :
    case 'r' :
        return FS_READ_FILE;
    case 'D' :
    case 'd' :
        return FS_DELETE_FILE;
    case 'N':
    case 'n' :
        return FS_CREATE_FILE;
    case 'G':
    case 'g' :
        return FS_GET_DISK_INFO;
    case 'S':
    case 's' :
        return	FS_SYS_RESET;
    case 'E':
    case 'e' :
        return FS_EDIT_FILE;
    case 'L':
    case 'l' :
        return FS_FILE_LIST;
    case 'H':
    case 'h' :
        return FS_HELP;
    default:
        return ( FS_CMD ) USART1_CmdTemp;        
    } /* End of switch */   
} /* End of USART_GetCmd */

/*******************************************************************************
* Function Nmae     PrintCommandList
* Date              2013/4/11
* Anthor            WXJ
* Input Parameter   None
* Output Parameter  None
* Description       --
* File              FS_TEST.c
*******************************************************************************/
void PrintCommanList ( void )
{
    printf ("\r\nGet command list...");
    printf ("\r\n----------------- COMMAND LIST  ------------------");
    
    printf ("\r\n  F ----- FS_FORMAT_DISK.");
//    printf ("\r\n  O ----- FS_OPEN_FILE.");
    printf ("\r\n  R ----- FS_READ_FILE.");
    printf ("\r\n  D ----- FS_DELETE_FILE.");
    printf ("\r\n  N ----- FS_CREATE_FILE.");
    printf ("\r\n  G ----- FS_GET_DISK_INFO.");
    printf ("\r\n  S ----- FS_SYS_RESET.");
    printf ("\r\n  E ----- FS_EDIT_FILE.");
    printf ("\r\n  L ----- FS_FILE_LIST.");
    printf ("\r\n  H ----- FS_HELP.");
    printf ("\r\n  others -undefined.");
    
    printf ("\r\n---------------------- END --------------------- |");    
} /* End of PrintCommandList */

/*******************************************************************************
* Function Nmae     ListFile
* Date              2013/4/11
* Anthor            WXJ
* Input Parameter   None
* Output Parameter  None
* Description       --
* File              FS_TEST.c
*******************************************************************************/
void ListFile ( void )
{
    FILINFO finfo;
    FRESULT res;
    DIR     dirs;
    int     i,
            files_num=0;
    
    res = f_mount(0,&fs);
    if (res != FR_OK)
    {
        printf("\nLoad the file system fails! ERROR CODE: %u",res);
        return;
    }
    
    /* Create a Directroy Object */
    res = f_opendir(&dirs,"/");							
    printf("\r\n------------File List------------");
    
    if (res == FR_OK)
    {
        /* Read Directory Entry in Sequense */
        while ((f_readdir(&dirs, &finfo) == FR_OK) && finfo.fname[0]) 	 
        {
            i = strlen(finfo.fname);
            
            /* It is a folder */
            if (finfo.fattrib & AM_DIR)
            {
                files_num++;
                printf("\r\n/%s", &finfo.fname[0]);
                
                /* for align left*/
                switch(i)
                {
                case 1:printf(" ");
                case 2:printf(" ");
                case 3:printf(" ");
                case 4:printf(" ");
                case 5:printf(" ");
                case 6:printf(" ");
                case 7:printf(" ");
                case 8:printf("%15s"," ");
                } /* End of Switch */
            }
            else 
            {
                continue;
            }  /* End of  if (finfo.fattrib & AM_DIR) */
        } /* End of while */
    }
    else
    {
        printf("\r\nCan not open the file list! ERROR CODE: %u",res);
    } /* En dof if (res == FR_OK) */
    
    res = f_opendir(&dirs, "/");
    
    if (res == FR_OK) 
    {
        while ((f_readdir(&dirs, &finfo) == FR_OK) && finfo.fname[0]) 
        {
            if (finfo.fattrib & AM_DIR) 
            { 
                continue;
            } 
            else 
            {   
                files_num++;				
                printf("\r\n/.%12s%7ld Byte ",  &finfo.fname[0],(finfo.fsize+512));				
            } /* End of if (finfo.fattrib & AM_DIR) */
        } /* End of while */
        
        /* No files */
        if( files_num==0 )
        {
            printf("\r\nNo files, you should to create a file! ");   
        }
        else 
        {
            printf("\r\n----------- List End -----------");
        }
    }
    else
    {
        printf("\r\nOpen the root failure!  ERROR CODE: %u",res);
    } /* End of if (res == FR_OK) */
    
    f_mount(0,NULL);   
} /* End of ListFile */

/*******************************************************************************
* Function Nmae     CheckFileName
* Date              2013/4/11
* Anthor            WXJ
* Input Parameter   None
* Output Parameter  None
* Description       --
* File              FS_TEST.c
*******************************************************************************/
bool CheckFileName ( uint8_t * FileName, uint8_t length )
{
    
    uint8_t i, 
            flag=0,
            temp=0,	
            *Ptr = FileName;
    
    if ( length > FS_FILE_NAME_SIZE ) 
    {
        return FALSE;
    }
    
    for( i = 0; i < length; i++ )
    {
	if ( *( Ptr + i ) == '.' )
	{
            flag++;
            temp = i;
	}
    } /* End of for loop */
    
    if (( flag != 1 ) || ( temp < 1 ) || ( temp > 8 ) || 
                        ( ( length - temp ) > 4 ) || ( length - temp ) <= 1 )
    {
	return FALSE;
    }
    
    return TRUE;
} /* End of CheckFileName */

/*******************************************************************************
* Function Nmae     GetFileName
* Date              2013/4/11
* Anthor            WXJ
* Input Parameter   None
* Output Parameter  None
* Description       --
* File              FS_TEST.c
*******************************************************************************/
bool GetFileName ( uint8_t * FileName, uint8_t * length)
{   
    uint8_t     i,
                Counter = 0,
                FileNameBuffer[FS_FILE_NAME_SIZE] = {0};
    uint32_t    delay_i = 0;
    
    printf("\r\nPlease input file name, and press 'Q' to quit...\r\n" );
    
    while ( 1 )
    {        
        delay_i++;
        
        if ( USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET)
        {
            FileNameBuffer[Counter++] = ( uint8_t )USART_ReceiveData ( USART1 );
            delay_i = 0;           
        }
        
        if ( ( delay_i >= 0XBFF) && (Counter > 0 ) )
        {
            *length = Counter;
            Counter = 0;
            
            if (( *length == 1 ) && (( FileNameBuffer[0] == 'Q') || 
                                                ( FileNameBuffer[0] == 'q')))
            {
                return FALSE;
            }
            
            for ( i = 0; i < *length; i++ )
            {
                *( FileName + i ) = FileNameBuffer[i];
                printf ("%c",FileNameBuffer[i]);
            }
            
            if ( !CheckFileName ( FileName, *length ))
            {
                printf ("\r\nFile name error!");
                printf("\r\nPlease input angin,and press 'Q' to quit...\r\n");                    
            }
            else 
            {	
                return TRUE;
            } /* End of if ( !CheckFileName ( FileName, *length )) */
        } /* End of if ( ( delay_i >= 0XBFF)... */       
    } /* End of while */    
} /* End of GetFileName */
                        
/*******************************************************************************
* Function Nmae     CreateFile
* Date              2013/4/11
* Anthor            WXJ
* Input Parameter   None
* Output Parameter  None
* Description       --
* File              FS_TEST.c
*******************************************************************************/
void CreateFile ( void )
{
    FRESULT res;
    uint8_t FileName[FS_FILE_NAME_SIZE]={'\0'},
            length;
    
    res = f_mount(0,&fs);
    if (res != FR_OK)
    {
        printf("\r\nLoad the file system fails! ERROR CODE: %u",res);
        return;
    }
    
    if ( GetFileName ( FileName, &length))
    {       
        res=f_open(&fl,(TCHAR *)FileName,FA_CREATE_NEW | FA_WRITE);
        
        if ( res == FR_EXIST )
        {
            printf("\nCreate file failure! ERROR CODE: %u",res);
            printf ("\r\nFile already exists");      
        }
        else if ( res == FR_OK )
        {
            printf ("\r\nCreate file success");
        }
        else
        {
            printf("\r\nCreate file Ffailure! ERROR CODE: %u",res);	
        } /* End of if ( res == FR_EXIST ) */
    }
    else
    {
        printf("\r\nCreate file failure,you should input a right file name."); 
    }  /* if ( GetFileName ( FileNa... */ 
    
    f_close(&fl);
    f_mount(0,NULL);                           
} /* End of CreateFile */
                        
/*******************************************************************************
* Function Nmae     SysReset
* Date              2013/4/11
* Anthor            WXJ
* Input Parameter   None
* Output Parameter  None
* Description       --
* File              FS_TEST.c
*******************************************************************************/
void SysReset ( void )
{
    SCB->AIRCR =0X05FA0000|(u32)0x04;	    
} /* End of SysReset */

/*******************************************************************************
* Function Nmae     EditFile
* Date              2013/4/11
* Anthor            WXJ
* Input Parameter   None
* Output Parameter  None
* Description       --
* File              FS_TEST.c
*******************************************************************************/
void EditFile ( void )
{
    FIL         edit_file; 
    uint32_t    reindex,
                i=0,
                j,
                File_Counter=0;
    FRESULT     res;
    uint8_t     FileName[FS_FILE_NAME_SIZE]={'\0'},
                length;
     
    ListFile ( );
    
    res = f_mount(0,&fs);
    if (res != FR_OK)
    {
        printf("\nLoad the file system fails! ERROR CODE: %u",res);
        return;
    }
    
    GetFileName (FileName, &length);
    
    res=f_open(&edit_file,(TCHAR *)FileName,FA_OPEN_ALWAYS | FA_WRITE);
    
    if(res==FR_OK)
    {       
        printf ("\r\nOpend file : /%s...\r\nPlease input the data...",
                                                            (TCHAR *)FileName);
        while ( 1 )
        {
            i++;
            if (( i > 0x3FFF) && ( File_Counter > 0) )
            {
                if (!( j % 50 ))
                {
                    printf("\r\n");
                }
                for( ; j < File_Counter; j++)
                {
                    printf("%c",FileBuffer[j]);

                } /* End of for loop */
                i=0;
            }
            if ( USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET)                  
            {   
                i = 0;
                FileBuffer[File_Counter] = (uint8_t)USART_ReceiveData (USART1);
                
                
                if ( FileBuffer[File_Counter] == 0X1B)                   
                {
                    USART_ReceiveData ( USART1 );
                    //f_close(&fl);
                    File_Counter = 0x00;
                    break;
                }
                else if ( FileBuffer[File_Counter] == 0X13)
                {
                    USART_ReceiveData ( USART1 );

                    printf("\r\nFinish edit");
                    printf("\r\nSave data...");
                    res = f_write(&edit_file,FileBuffer,File_Counter,&reindex);
                    
                    if ((res == FR_OK) && (reindex == File_Counter))
                    {
                        if( !File_Counter )printf("\r\nNo data be written.");
                        else printf("\r\nSave Data Success!");
                        //f_close(&fl);
                        File_Counter = 0x00;									
                    }
                    else
                    {
                        printf("\r\nSave Data Failure!   ERROR CODE: %u",res);									
                    } /* End of if ((res == FR_OK) && (rei... */
                    
                    break;
                }
                else 
                {
                    File_Counter++;
                } /* End of if ( FileBuffer... */
            } /* End of if ( USART_GetFlagStatus(USART1... */
        } /* End  of while loop */       
    }
    else
    {
        printf("\r\nCan not open the file. ERROR CODE: %u",res);
    }/* End of if(res==FR_OK) */
    
    f_close(&edit_file);
    f_mount(0,NULL);   
} /* End of EditFile */

/*******************************************************************************
* Function Nmae     ReadFile
* Date              2013/4/11
* Anthor            WXJ
* Input Parameter   None
* Output Parameter  None
* Description       --
* File              FS_TEST.c
*******************************************************************************/
void ReadFile(void)
{
    FRESULT     res;
    uint32_t    r,i;   
    uint8_t     FileName[FS_FILE_NAME_SIZE]={'\0'},
                length;
    
    ListFile ( );
    
    res = f_mount(0,&fs);
    if (res != FR_OK)
    {
        printf("\nLoad the file system fails! ERROR CODE: %u",res);
        return;
    }
    
    GetFileName(FileName, &length);
    
    res=f_open(&fl,(TCHAR *)FileName,FA_OPEN_EXISTING | FA_READ);
    
    if(res==FR_OK)
    {
        printf ("\r\nOpend file : /%s...",(TCHAR *)FileName);
        for(r = 0; r < NAND_PAGE_SIZE; r++)
        {
            RxBuffer[r]= 0xff;
        }
        
        res=f_read(&fl,RxBuffer,NAND_PAGE_SIZE,&r);
        if (res == FR_OK)
        {
            printf("\r\nThe Data of file in char");
            for(i = 0; i < r; i++)
            {
                if (!( i % 50 ))
                {
                    printf("\r\n");
                }
                printf("%c",RxBuffer[i]);
                
            } /* End of for loop */
        }
        else 
        {
            printf("\r\nCan not read the data. ERROR CODE: %u",res);
        }
    }
    else
    {
     printf("\r\nCan not open the file. ERROR CODE: %u",res);   
    }
                   
    res=f_close(&fl);
    f_mount(0,NULL);
} /* End of ReadFile */

/*******************************************************************************
* Function Nmae     DeleteFile
* Date              2013/4/11
* Anthor            WXJ
* Input Parameter   None
* Output Parameter  None
* Description       --
* File              FS_TEST.c
*******************************************************************************/
void DeleteFile ( void )
{
    
    FRESULT     res;
    uint8_t     FileName[FS_FILE_NAME_SIZE]={'\0'},
                length = 0;
    
    ListFile ( );
    
    /* Mount a Logical Drive 0*/	
    res = f_mount(0,&fs);		       
    if (res != FR_OK)
    {
        printf("\nLoad the file system fails! ERROR CODE: %u",res);
        return;
    }
  
    GetFileName(FileName, &length);
    
    /* Delete a File or Directory */
    res = f_unlink((TCHAR *)FileName);	  
    if (res == FR_OK)
    {
        printf("\r\nDelete File Success! ");
    }
    else if (res == FR_NO_FILE)
    {
        printf("\r\nCan not find the file! ");
    }
    else if (res == FR_NO_PATH)
    {
        printf("\r\nCan not find the path! ");
    }
    else
    {
        printf("Can not delete the file, ERROR CODE: %u",res);
    } /* End of if (res == FR_OK) */
    
    f_mount(0,NULL);
} /* End of DeleteFile */



bool GetDiskInfo_1 ( void )
{
    FATFS    *pfs;
    DWORD    clust;
    FRESULT  res;
    bool     RESULT;
    
    res = f_mount(0,&fs);
   
    pfs=&fs;
    res = f_getfree("/", &clust, &pfs);
    
    if ( res ==FR_OK )
    {
        RESULT = TRUE;
    }
   
    f_mount(0,NULL);
    
    return RESULT;
} /* End of GetDiskInfo */

bool DeleteFile_1 ( void )
{
    
    FRESULT     res;
    
    /* Mount a Logical Drive 0*/	
    res = f_mount(0,&fs);		       
    if (res != FR_OK)
    {
        
    }
  
    
    /* Delete a File or Directory */
    res = f_unlink("/test.txt");	  
    if (res == FR_OK)
    {
       
    }
    else if (res == FR_NO_FILE)
    {
        return FALSE;
    }
    else
    {
        return FALSE;
    } /* End of if (res == FR_OK) */
    
    f_mount(0,NULL);
     return TRUE;
    
} /* End of DeleteFile */

bool ReadFile_1(void)
{
    FRESULT     res;
    uint32_t    r;   

    res = f_mount(0,&fs);
    if (res != FR_OK)
    {
        return FALSE;
    }
    

    
    res=f_open(&fl,"/test.txt",FA_OPEN_EXISTING | FA_READ);
    
    if(res==FR_OK)
    {
        for(r = 0; r < NAND_PAGE_SIZE; r++)
        {
            RxBuffer[r]= 0xff;
        }
        
        res=f_read(&fl,RxBuffer,NAND_PAGE_SIZE,&r);
        if (res == FR_OK)
        {

        }
        else 
        {
            return FALSE;
        }
    }
    else
    {
     return FALSE;   
    }
                   
    res=f_close(&fl);
    f_mount(0,NULL);
    return TRUE;
} /* End of ReadFile */

bool EditFile_1 ( void )
{
    FIL         edit_file; 
    uint32_t    reindex,
                i=0;
    FRESULT     res;

    res = f_mount(0,&fs);
    if (res != FR_OK)
    {
return FALSE; 
    }
    

    
    res=f_open(&edit_file,"/test.txt",FA_OPEN_ALWAYS | FA_WRITE);
    
    if(res==FR_OK)
    {       
        
        for (i =0;i<512;i++)FileBuffer[i]=i;
        res = f_write(&edit_file,FileBuffer,512,&reindex);
        if ((res == FR_OK) && (reindex == 512))
        {
            
        }
        else
        {
            return FALSE;							
        } /* End o */       
    } /* End of if ( USART_GetFlagStatus(USART1... */
    else
    {
        return FALSE;
    }/* End of if(res==FR_OK) */
    
    f_close(&edit_file);
    f_mount(0,NULL); 
    return TRUE;
    
} /* End of EditFile */
/***************************** End of File ************************************/