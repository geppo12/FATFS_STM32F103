
#define RCC_GPIO_LED                    RCC_APB2Periph_GPIOF 
#define LEDn                            4                      
#define GPIO_LED                        GPIOF                  

#define DS1_PIN                         GPIO_Pin_6      /*DS1使用的GPIO管脚*/
#define DS2_PIN                         GPIO_Pin_7	/*DS2使用的GPIO管脚*/
#define DS3_PIN                         GPIO_Pin_8  	/*DS3使用的GPIO管脚*/
#define DS4_PIN                         GPIO_Pin_9	/*DS4使用的GPIO管脚*/
/* Values magic to the Board keys */
#define  NOKEY  0
#define  KEY1   1
#define  KEY2   2
#define  KEY3   3
#define  KEY4   4

#define RCC_KEY1                                    RCC_APB2Periph_GPIOD
#define GPIO_KEY1_PORT                              GPIOD    
#define GPIO_KEY1                                   GPIO_Pin_3

#define RCC_KEY2                                    RCC_APB2Periph_GPIOA
#define GPIO_KEY2_PORT                              GPIOA  
#define GPIO_KEY2                                   GPIO_Pin_8

#define RCC_KEY3                                    RCC_APB2Periph_GPIOC
#define GPIO_KEY3_PORT                              GPIOC    
#define GPIO_KEY3                                   GPIO_Pin_13 

#define RCC_KEY4                                    RCC_APB2Periph_GPIOA
#define GPIO_KEY4_PORT                              GPIOA    
#define GPIO_KEY4                                   GPIO_Pin_0 

#define GPIO_KEY_ANTI_TAMP                          GPIO_KEY3
#define GPIO_KEY_WEAK_UP                            GPIO_KEY4


#define BUFFER_SIZE         0x400
#define NAND_HY_MakerID     0xAD
#define NAND_HY_DeviceID    0xF1
#define NAND_ST_MakerID     0x20
#define NAND_ST_DeviceID    0x76


#define 	FS_FILE_NAME_SIZE	12