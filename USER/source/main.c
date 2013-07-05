/*******************************************************************************
* File Name       main.c
* Anthor          WXJ     
* Version         V1.00
* Date            2013/4/11
* Description     --    
*******************************************************************************/

//---- Include -----------------------------------------------------------------
#include "common.h"
#include "stm32f10x.h"
#include "fsmc_nand.h"

/*******************************************************************************
* Function Nmae     main
* Date              2013/4/9
* Anthor            WXJ
* Input Parameter   None
* Output Parameter  None
* Description       --
* File              main.c
*******************************************************************************/
void main ( void )
{
    
    /* Initialize the system */
    SysInit ( ); 
    /* Call the function to test the fatfs */
//    while (1)
//    {    TestFatfs ( );
////        FR_Test( );
//    }
   TEST_PCCMD();
}

/***************************** End of File ************************************/