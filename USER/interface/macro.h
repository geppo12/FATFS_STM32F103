

//---- Private Function Prototypes ---------------------------------------------
#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

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

  /* NAND Area definition  for STM3210E-EVAL Board RevD */
  #define CMD_AREA                   (uint32_t)(1<<16)  /* A16 = CLE high */
  #define ADDR_AREA                  (uint32_t)(1<<17)  /* A17 = ALE high */
  #define DATA_AREA                  ((uint32_t)0x00000000) 
  
  /* FSMC NAND memory command */
  #define NAND_CMD_READ1             ((uint8_t)0x00)
  #define NAND_CMD_READ2            ((uint8_t)0x30)
  
  #define NAND_CMD_WRITE0            ((uint8_t)0x80)
  #define NAND_CMD_WRITE1            ((uint8_t)0x10)
  
  #define NAND_CMD_MOVE0             ((uint8_t)0x00)
  #define NAND_CMD_MOVE1             ((uint8_t)0x35)
  #define NAND_CMD_MOVE2             ((uint8_t)0x85)
  #define NAND_CMD_MOVE3             ((uint8_t)0x10)
   
  #define NAND_CMD_ERASE0            ((uint8_t)0x60)
  #define NAND_CMD_ERASE1            ((uint8_t)0xD0)  
  
  #define NAND_CMD_READID            ((uint8_t)0x90)
  #define NAND_CMD_IDADDR            ((uint8_t)0x00)
   
  #define NAND_CMD_STATUS            ((uint8_t)0x70)
  #define NAND_CMD_RESET             ((uint8_t)0xFF)
  
  /* NAND memory status */
  #define NAND_VALID_ADDRESS         ((uint32_t)0x00000100)
  #define NAND_INVALID_ADDRESS       ((uint32_t)0x00000200)
  #define NAND_TIMEOUT_ERROR         ((uint32_t)0x00000400)
  #define NAND_BUSY                  ((uint32_t)0x00000000)
  #define NAND_ERROR                 ((uint32_t)0x00000001)
  #define NAND_READY                 ((uint32_t)0x00000040)
  
  /* FSMC NAND memory parameters */
  //#define NAND_PAGE_SIZE             ((uint16_t)0x0200) /* 512 bytes per page w/o Spare Area */
  //#define NAND_BLOCK_SIZE            ((uint16_t)0x0020) /* 32x512 bytes pages per block */
  //#define NAND_ZONE_SIZE             ((uint16_t)0x0400) /* 1024 Block per zone */
  //#define NAND_SPARE_AREA_SIZE       ((uint16_t)0x0010) /* last 16 bytes as spare area */
  //#define NAND_MAX_ZONE              ((uint16_t)0x0004) /* 4 zones of 1024 block */
  
  /* FSMC NAND memory HY27UF081G2A-TPCB parameters */
  #define NAND_PAGE_SIZE             ((uint16_t)0x0800) /* 2048 bytes per page w/o Spare Area */
  #define NAND_BLOCK_SIZE            ((uint16_t)0x0040) /* 64x2048 bytes pages per block */
  #define NAND_ZONE_SIZE             ((uint16_t)0x0200) /* 512 Block per zone */
  #define NAND_SPARE_AREA_SIZE       ((uint16_t)0x0040) /* last 64 bytes as spare area */
  #define NAND_MAX_ZONE              ((uint16_t)0x0002) /* 2 zones of 1024 block */
  
  /* FSMC NAND memory data computation */
  #define DATA_1st_CYCLE(DATA)       (uint8_t)((DATA)& 0xFF)               /* 1st data cycle */
  #define DATA_2nd_CYCLE(DATA)       (uint8_t)(((DATA)& 0xFF00) >> 8)      /* 2nd data cycle */
  #define DATA_3rd_CYCLE(DATA)       (uint8_t)(((DATA)& 0xFF0000) >> 16)   /* 3rd data cycle */
  #define DATA_4th_CYCLE(DATA)       (uint8_t)(((DATA)& 0xFF000000) >> 24) /* 4th data cycle */
  
  /* FSMC NAND memory HY27UF081G2A-TPCB address computation */
  #define ADDR_1st_CYCLE(PADDR)       (uint8_t)(0x0)          /* 1st addressing cycle */
  #define ADDR_2nd_CYCLE(PADDR)       (uint8_t)(0x0)     /* 2nd addressing cycle */
  #define ADDR_3rd_CYCLE(PADDR)       (uint8_t)(PADDR & 0xFF)      /* 3rd addressing cycle */
  #define ADDR_4th_CYCLE(PADDR)       (uint8_t)((PADDR>>8) & 0xFF) /* 4th addressing cycle */

  #define FSMC_Bank_NAND     FSMC_Bank2_NAND
  #define Bank_NAND_ADDR     Bank2_NAND_ADDR 
  #define Bank2_NAND_ADDR    ((uint32_t)0x70000000)


#define 	FS_FILE_NAME_SIZE	12

#define         _BACKUP                 0