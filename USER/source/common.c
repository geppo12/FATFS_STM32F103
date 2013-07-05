/*******************************************************************************
* File Name       common.c
* Anthor          WXJ     
* Version         V1.00
* Date            2013/4/11
* Description     --    
*******************************************************************************/

//---- Include -----------------------------------------------------------------
#include "macro.h"
#include "typedef.h"
#include "stdio.h"
#include "stm32_eval.h"
#include "fsmc_nand.h"
#include "fs_test.h"
        
//---- Private Function Prototypes ---------------------------------------------
void    Usart1Init      ( void );
void    GpioLedInit     ( void );
void    NandInit        ( void );

void    assert_failed   ( uint8_t* file, uint32_t line );
PUTCHAR_PROTOTYPE;
void FR_Test (  uint8_t flag );
//---- Private Function --------------------------------------------------------
/*******************************************************************************
* Function Nmae     SysInit
* Date              2013/4/9
* Anthor            WXJ
* Input Parameter   None
* Output Parameter  None
* Description       --
* File              common.c
*******************************************************************************/
void SysInit ( void )
{
    /* Initialize usart1 */      
    Usart1Init ( );    
    printf ("\r\n\r\n");
    printf ("\r\n------------ SYSTEM INITIALIZATION  --------------");
    printf ("\r\nBegin...\r\nUsart1 initializing...\r\nComplete");
    
    printf ("\r\nGpio ininializing...");
    GpioLedInit ( );
    printf ("\r\nComplete");
    
    printf ("\r\nNand flash initializing..."); 
    /* Configure the NAND FLASH */
//    NandInit ( ); 
    printf ("\r\nComplete\r\nFinish");
    
    printf ("\r\n--------------   INITIALIZE END   ----------------");
} /* End of SysInit */

/*******************************************************************************
* Function Nmae     Usart1Init
* Date              2013/4/11
* Anthor            WXJ
* Input Parameter   None
* Output Parameter  None
* Description       --
* File              common.c
*******************************************************************************/
//    - USARTx configured as follow:
//    - BaudRate = 115200 baud  
//    - Word Length = 8 Bits
//    - One Stop Bit
//    - No parity
//    - Hardware flow control disabled (RTS and CTS signals)
//    - Receive and transmit enabled
void Usart1Init ( void )
{
    USART_InitTypeDef USART_InitStructure;
    
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;  
    
    STM_EVAL_COMInit(COM1, &USART_InitStructure);       
} /* End of Usart1Init */

/*******************************************************************************
* Function Nmae     GpioLedInit
* Date              2013/4/11
* Anthor            WXJ
* Input Parameter   None
* Output Parameter  None
* Description       --
* File              common.c
*******************************************************************************/
void GpioLedInit ( void )
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_GPIO_LED, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = DS1_PIN | DS2_PIN | DS3_PIN | DS4_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    
    GPIO_Init(GPIO_LED, &GPIO_InitStructure);
    
    /* Turn off all leds */
    GPIO_SetBits(GPIO_LED, DS1_PIN | DS2_PIN | DS3_PIN | DS4_PIN);   
} /* End of  GpioLedInit */

/*******************************************************************************
* Function Nmae     NandInit
* Date              2013/4/11
* Anthor            WXJ
* Input Parameter   None
* Output Parameter  None
* Description       --
* File              common.c
*******************************************************************************/
void NandInit ( void )
{
    NAND_IDTypeDef NAND_ID;
    
    /* Enable the FSMC Clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
    
    /* FSMC Initialization */
    FSMC_NAND_Init();
    
    printf("\r\nCheck nandflash ID...");
    /* NAND read ID command */
    FSMC_NAND_ReadID(&NAND_ID);
    
    
    /* Verify the NAND ID */
    if((NAND_ID.Maker_ID == NAND_ST_MakerID) \
                                    && (NAND_ID.Device_ID == NAND_ST_DeviceID))
    {
        printf("\r\nST NANDFLASH");
    }
    else if((NAND_ID.Maker_ID == NAND_HY_MakerID) \
                                    && (NAND_ID.Device_ID == NAND_HY_DeviceID))
    {
        printf("\r\nHY27UF081G2A-TPCB");
    }
    
    printf("\r\nID = %x-%x-%x-%x",NAND_ID.Maker_ID, \
                         NAND_ID.Device_ID,NAND_ID.Third_ID,NAND_ID.Fourth_ID);        
} /* End of NandInit */

/*******************************************************************************
* Function Nmae     Test_Fatfs
* Date              2013/4/11
* Anthor            WXJ
* Input Parameter   None
* Output Parameter  None
* Description       --
* File              common.c
*******************************************************************************/
void TestFatfs ( void )
{
    FS_CMD   cmd;
    uint8_t flag=0;
    printf ("\r\n\r\n");
    printf ("\r\n--------------- FATFS TEST BEGIN  ----------------");
    
    /* New nandflash must be format */ 
      FormatDisk ( ); 
    
    if ( !GetDiskInfo ( ) )
    {
        FormatDisk ( );  
    }
    
    while  (1)
    {         
        printf ("\r\n\r\nPlease input test command,Input 'H' to get help.\r\n");
        
        cmd = USART1_GetCmd ( );
       
        switch ( cmd )
        {
        case FS_HELP : 
            printf ("\r\n-----CMD :HELP-----\t\n");
            PrintCommanList ( );
            break;
        case FS_FORMAT_DISK :
            printf ("\r\n-----CMD :FORMAT DISK-----\r\n");
            FormatDisk ( );
            GetDiskInfo ( );
            break;                            
        case  FS_READ_FILE:
            printf ("\r\n-----CMD :READ FILE-----\r\n");
            ReadFile();
            break;
        case  FS_DELETE_FILE:
            printf ("\r\n-----CMD :DELETE FILE-----\r\n");
            DeleteFile ();
            break;
        case  FS_CREATE_FILE:
            printf ("\r\n-----CMD :CTEATE_FILE-----\r\n");
            CreateFile ();
            break;
        case  FS_GET_DISK_INFO:
            printf ("\r\n-----CMD :GET DISK INFORMATION-----\r\n");
            GetDiskInfo ( );
            break;
        case  FS_SYS_RESET:
            printf ("\r\n-----CMD :RESTART SYSTEM-----\r\n");
            SysReset ( );
            break;
        case  FS_EDIT_FILE:
            printf ("\r\n-----CMD :EDIT FILE-----\r\n");
            EditFile ( ); 
            break;
        case  FS_FILE_LIST:
            printf ("\r\n-----CMD :GET FILE LIST-----\r\n");
            ListFile ( );
            break;         
//        case : FS_OPEN_FILE    
        case FS_UNDEF_CMD:
        default:
            printf ("\r\nWarning : undefined Command");           
        }
        if(cmd == FS_OPEN_FILE)
        {
            printf("\r\nsetup test loop....");
            while(1)
            {
            cmd = USART1_GetCmd ( );
       
        switch (cmd)
        {
        case  FS_GET_DISK_INFO:
            if(flag & 0x01) {flag&= ~(0x01);GPIO_SetBits(GPIO_LED, DS1_PIN);}
            else {flag|= 0x01;GPIO_ResetBits(GPIO_LED, DS1_PIN);}
            
            break;
        case  FS_EDIT_FILE:
             if(flag & 0x02)  {flag&= ~(0x02);GPIO_SetBits(GPIO_LED, DS2_PIN);}
            else {flag|= 0x02;GPIO_ResetBits(GPIO_LED, DS2_PIN);}
            break;  
        case  FS_READ_FILE:
             if(flag & 0x04)  {flag&= ~(0x04);GPIO_SetBits(GPIO_LED, DS3_PIN);}
            else {flag|= 0x04;GPIO_ResetBits(GPIO_LED, DS3_PIN);}
            break;     
        case  FS_DELETE_FILE:
            if(flag & 0x08) {flag&= ~(0x08);GPIO_SetBits(GPIO_LED, DS4_PIN);}
            else {flag|= 0x08;GPIO_ResetBits(GPIO_LED, DS4_PIN);}
            break;
            }
         if(cmd == FS_OPEN_FILE)
         {
             printf("\r\nStar test loop :test task code : 0x%2x\r\n Testing...",flag);
             FR_Test ( flag );
             return;
         }
            }
            
        }
        printf ("\r\n----CMD OPERATE END ---- \r\n");
        
    } 
   printf ("\r\n----TEST START ---- \r\n");
} /* End of TestFatfs */


void ledctrl (uint8_t num)
{
    uint32_t i=0x3ff;
   uint8_t USART1_CmdTemp;
    while (1)
    
    {     
        switch (num)
        {
        case 1 :        
        GPIO_SetBits(GPIO_LED, DS1_PIN);
        break;
        case 2:        
            GPIO_SetBits(GPIO_LED, DS2_PIN);
            break;
        case 4:        
            GPIO_SetBits(GPIO_LED, DS3_PIN);
            break;
        case 8:
            GPIO_SetBits(GPIO_LED, DS4_PIN);
            break;
        }
        for(i=0;i<0xfffff;i++){
        if ( USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET)
        {
            USART1_CmdTemp = ( uint8_t )USART_ReceiveData ( USART1 );	
            
        }
        }
        if (USART1_CmdTemp == 'o' || USART1_CmdTemp == 'O')return;
        switch (num)
        {
        case 1 :        
        GPIO_SetBits(GPIO_LED, DS1_PIN);
        break;
        case 2:        
            GPIO_ResetBits(GPIO_LED, DS2_PIN);
            break;
        case 4:        
            GPIO_ResetBits(GPIO_LED, DS3_PIN);
            break;
        case 8:
            GPIO_ResetBits(GPIO_LED, DS4_PIN);
            break;
        }
       for(i=0;i<0xfffff;i++){
        if ( USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET)
        {
            USART1_CmdTemp = ( uint8_t )USART_ReceiveData ( USART1 );	
            
        }
        }
        if (USART1_CmdTemp == 'o' || USART1_CmdTemp == 'O')return;
    }
}
/*******************************************************************************
* Function Nmae     FR_Test
* Date              2013/4/12
* Anthor            WXJ
* Input Parameter   None
* Output Parameter  None
* Description       --
* File              common.c
*******************************************************************************/
void FR_Test (  uint8_t flag )
{
    uint8_t error_flag=0,format_flag= 0,USART1_CmdTemp;
    uint32_t i = 100000,num=0;
    bool res= TRUE;
    
    while (--i)
    {
        if ( USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET)
        {
            USART1_CmdTemp = ( uint8_t )USART_ReceiveData ( USART1 );	
            
        }
        if ( (USART1_CmdTemp == 'o') || (USART1_CmdTemp == 'O'))break;  
        if(flag & 0x01){           
     res = GetDiskInfo_1 ( );
     if ( res == FALSE )
     {
        error_flag |= 0x01;
//        printf ("\r\nGetDiskInfo ERROR :CODE : %d",res);
//        ledctrl (1);
//        break;
     }}
     if(flag & 0x02){
     res =EditFile_1 ( );
     if (  res == FALSE )
     {
         error_flag |= 0x02;
//         printf ("\r\nEditFile ERROR :CODE : %d",res);
//        ledctrl (2);
//        break;
     }}
     if(flag & 0x04){
     res =ReadFile_1 ( );
     if (  res == FALSE)
     {
        error_flag |= 0x04;
//         printf ("\r\nReadFile ERROR :CODE : %d",res);
//        ledctrl (3);
//        break;
     }}
     if(flag & 0x08){res = DeleteFile_1 ();
     if (  res == FALSE )
     {
        error_flag |= 0x08;

//        break;
     } }
     num++;
     if ((error_flag) )
     {
        if( !format_flag )
        {
         FormatDisk ( );
         format_flag =1;
         error_flag = 0;
        }
        else break;
     }    
    } 
        if ((error_flag) )           
        {
        printf("\r\nTest finish,loop number : %d ",num);
        printf ("\r\nDeleteFile ERROR :CODE : 0x%2x",error_flag);
        ledctrl (error_flag); 
        }
        else printf("\r\nTest finish,loop number : %d ",num);
} /* End of FR_Test */


/*******************************************************************************
* Function Nmae     PUTCHAR_PROTOTYPE
* Date              2013/4/9
* Anthor            WXJ
* Input Parameter   None
* Output Parameter  None
* Description       Retargets the C library printf function to the USART.
* File              main.c
*******************************************************************************/
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
  USART_SendData(EVAL_COM1, (uint8_t) ch);
  /* Loop until the end of transmission */
  while (USART_GetFlagStatus(EVAL_COM1, USART_FLAG_TC) == RESET)
  {}

  return ch;
} /* End of PUTCHAR_PROTOTYPE */


#ifdef  USE_FULL_ASSERT
/*******************************************************************************
* Function Nmae     
* Date              2013/4/11
* Anthor            WXJ
* Input Parameter   file: pointer to the source file name
                    line: assert_param error line source number
* Output Parameter  None
* Description       Reports the name of the source file and the source line 
                    number where the assert_param error has occurred.
* File              common.c
*******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/***************************** End of File ************************************/