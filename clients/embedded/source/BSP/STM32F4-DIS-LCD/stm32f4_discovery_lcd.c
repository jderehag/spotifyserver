/**
  ******************************************************************************
  * @file    stm324xg_discovery_lcd.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    30-September-2011
  * @brief   This file includes the LCD driver for AM-240320L8TNQW00H (LCD_ILI9320)
  *          and AM240320D5TOQW01H (LCD_ILI9325) Liquid Crystal Display Modules
  *          of STM324xG-EVAL evaluation board(MB786) RevB.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; Portions COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************  
  */ 
/**
  ******************************************************************************
  * <h2><center>&copy; Portions COPYRIGHT 2012 Embest Tech. Co., Ltd.</center></h2>
  * @file    stm32f4_discovery_lcd.c
  * @author  CMP Team
  * @version V1.0.0
  * @date    28-December-2012
  * @brief   LCD LOW_LEVEL Drive 
  *          Modified to support the STM32F4DISCOVERY, STM32F4DIS-BB, STM32F4DIS-CAM
  *          and STM32F4DIS-LCD modules.     
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, Embest SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT
  * OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
  * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
  * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4_discovery.h"
#include "stm32f4_discovery_lcd.h"
#include "fonts.h"
#include "applog.h"
/** @addtogroup Utilities
  * @{
  */ 

/** @addtogroup STM32F4_DISCOVERY
  * @{
  */
    
/** @defgroup stm32f4_discovery_LCD 
  * @brief This file includes the LCD driver for (LCDSSD2119)
  * @{
  */ 

/** @defgroup stm32f4_discovery_LCD_Private_TypesDef
  * @{
  */

/** @defgroup stm32f4_discovery_LCD_Private define
  * @{
  */
#define LCD_RST_PIN                  (GPIO_PIN_3)
#define LCD_RST_PORT                 (GPIOD)

#define LCD_PWM_PIN                  (GPIO_PIN_13)
#define LCD_PWM_PORT                 (GPIOD)

/* Note: LCD /CS is NE1 - Bank 1 of NOR/SRAM Bank 1~4 */
#define  LCD_BASE_Data               ((uint32_t)(0x60000000|0x00100000))
#define  LCD_BASE_Addr               ((uint32_t)(0x60000000|0x00000000))
#define  LCD_CMD                     (*(__IO uint16_t *)LCD_BASE_Addr)
#define  LCD_Data                    (*(__IO uint16_t *)LCD_BASE_Data)

#define MAX_POLY_CORNERS             200
#define POLY_Y(Z)                    ((int32_t)((Points + Z)->X))
#define POLY_X(Z)                    ((int32_t)((Points + Z)->Y))
/**
  * @}
  */ 

/** @defgroup stm32f4_discovery_LCD_Private_Macros
  * @{
  */
#define ABS(X)  ((X) > 0 ? (X) : -(X))     
/**
  * @}
  */ 
    
/** @defgroup stm32f4_discovery_LCD_Private_Variables
  * @{
  */ 


  
/**
  * @}
  */ 

/** @defgroup stm32f4_discovery_LCD_Private_FunctionPrototypes
  * @{
  */ 
#ifndef USE_Delay
static void delay(__IO uint32_t nCount);
#endif /* USE_Delay*/

static void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue);
static uint16_t LCD_ReadReg(uint8_t LCD_Reg);
static void LCD_WriteRAM_Prepare(void);
static void LCD_WriteRAM(uint16_t RGB_Code);
static uint16_t LCD_ReadRAM(void);
static void PutPixel(int16_t x, int16_t y, uint16_t Colour);
static void LCD_PolyLineRelativeClosed(pPoint Points, uint16_t PointCount, uint16_t Closed, uint16_t Colour);
/**
  * @}
  */ 

/**
  * @brief  LCD Default FSMC Init
  * @param  None
  * @retval None
  */
#if 0
void LCD_DeInit(void)
{ 
  GPIO_InitTypeDef GPIO_InitStructure;

  /*!< LCD Display Off */
  LCD_DisplayOff();

  /* BANK 3 (of NOR/SRAM Bank 1~4) is disabled */
  HAL_FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM3, ENABLE);
  
  /*!< LCD_SPI DeInit */
  FSMC_NORSRAMDeInit(FSMC_Bank1_NORSRAM3);
   
/*-- GPIO Configuration ------------------------------------------------------*/
  /* SRAM Data lines configuration */
  GPIO_InitStructure.GPIO_Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8 | GPIO_PIN_9 |
                                GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
 
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_MCO);


  GPIO_InitStructure.GPIO_Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
                                GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
                                GPIO_PIN_15;

  GPIO_Init(GPIOE, &GPIO_InitStructure);

  GPIO_PinAFConfig(GPIOE, GPIO_PinSource7 , GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource8 , GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource9 , GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource10 , GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource11 , GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource12 , GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource13 , GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource14 , GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource15 , GPIO_AF_MCO);

  /* SRAM Address lines configuration */
  GPIO_InitStructure.GPIO_Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
                                GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_12 | GPIO_PIN_13 |
                                GPIO_PIN_14 | GPIO_PIN_15;
  GPIO_Init(GPIOF, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOF,GPIO_PinSource0, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOF,GPIO_PinSource1, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOF,GPIO_PinSource2, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOF,GPIO_PinSource3, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOF,GPIO_PinSource4, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOF,GPIO_PinSource5, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOF,GPIO_PinSource12, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOF,GPIO_PinSource13, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOF,GPIO_PinSource14, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOF,GPIO_PinSource15, GPIO_AF_MCO);


  GPIO_InitStructure.GPIO_Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
                                GPIO_PIN_4 | GPIO_PIN_5;

  GPIO_Init(GPIOG, &GPIO_InitStructure);

  GPIO_PinAFConfig(GPIOG,GPIO_PinSource0, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOG,GPIO_PinSource1, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOG,GPIO_PinSource2, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOG,GPIO_PinSource3, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOG,GPIO_PinSource4, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOG,GPIO_PinSource5, GPIO_AF_MCO);

  GPIO_InitStructure.GPIO_Pin = GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;

  GPIO_Init(GPIOD, &GPIO_InitStructure);

  GPIO_PinAFConfig(GPIOD,GPIO_PinSource11, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource12, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource13, GPIO_AF_MCO);

  /* NOE and NWE configuration */  
  GPIO_InitStructure.GPIO_Pin = GPIO_PIN_4 |GPIO_PIN_5;

  GPIO_Init(GPIOD, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource4, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource5, GPIO_AF_MCO);

  /* NE3 configuration */
  GPIO_InitStructure.GPIO_Pin = GPIO_PIN_10;

  GPIO_Init(GPIOG, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource12, GPIO_AF_MCO);

  /* NBL0, NBL1 configuration */
  GPIO_InitStructure.GPIO_Pin = GPIO_PIN_0 | GPIO_PIN_1;
  GPIO_Init(GPIOE, &GPIO_InitStructure); 

  GPIO_PinAFConfig(GPIOE,GPIO_PinSource0, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource1, GPIO_AF_MCO);
}
#endif
/**
  * @brief  Configures LCD Control lines (FSMC Pins) in alternate function mode.
  * @param  None
  * @retval None
  */
static void LCD_CtrlLinesConfig(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable GPIO clocks */
    __GPIOB_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();
    __GPIOE_CLK_ENABLE();
    __GPIOF_CLK_ENABLE();

    /*-- GPIO Configuration ------------------------------------------------------*/
    /* SRAM Data lines,  NOE and NWE configuration */
    GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8 | GPIO_PIN_9 |
                             GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15 |
                             GPIO_PIN_4 |GPIO_PIN_5;
    GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
    GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Alternate = GPIO_AF12_FSMC;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
                             GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
                             GPIO_PIN_15;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);

    /* SRAM Address lines configuration LCD-DC */
    GPIO_InitStructure.Pin = GPIO_PIN_3;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);

    /* NE3 configuration */
    GPIO_InitStructure.Pin = GPIO_PIN_7;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* LCD RST configuration */
    GPIO_InitStructure.Pin = LCD_RST_PIN;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Alternate = 0;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(LCD_RST_PORT, &GPIO_InitStructure);

    /* LCD pwm configuration */
    GPIO_InitStructure.Pin = LCD_PWM_PIN;
    GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Alternate = 0;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(LCD_PWM_PORT, &GPIO_InitStructure);

    HAL_GPIO_WritePin( LCD_PWM_PORT, LCD_PWM_PIN, GPIO_PIN_SET );
}

/**
  * @brief  Configures the Parallel interface (FSMC) for LCD(Parallel mode)
  * @param  None
  * @retval None
  */
static void LCD_FSMCConfig(void)
{
    FSMC_NORSRAM_InitTypeDef    NORSRAMInit;
    FSMC_NORSRAM_TimingTypeDef  TimingInit;

    /* Enable FSMC clock */
    __FSMC_CLK_ENABLE();

    /*-- FSMC Configuration ------------------------------------------------------*/
    /*----------------------- SRAM Bank 1 ----------------------------------------*/
    /* FSMC_Bank1_NORSRAM4 configuration */
    TimingInit.AddressSetupTime = 1;
    TimingInit.AddressHoldTime = 0xf;
    TimingInit.DataSetupTime = 9;
    TimingInit.BusTurnAroundDuration = 0;
    TimingInit.CLKDivision = 16;
    TimingInit.DataLatency = 17;
    TimingInit.AccessMode = FSMC_ACCESS_MODE_A;
    /* Color LCD configuration ------------------------------------
     LCD configured as follow:
        - Data/Address MUX = Disable
        - Memory Type = SRAM
        - Data Width = 16bit
        - Write Operation = Enable
        - Extended Mode = Enable
        - Asynchronous Wait = Disable */

    NORSRAMInit.NSBank = FSMC_NORSRAM_BANK1;
    NORSRAMInit.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
    NORSRAMInit.MemoryType = FSMC_MEMORY_TYPE_SRAM;
    NORSRAMInit.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
    NORSRAMInit.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
    NORSRAMInit.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
    NORSRAMInit.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
    NORSRAMInit.WrapMode = FSMC_WRAP_MODE_DISABLE;
    NORSRAMInit.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
    NORSRAMInit.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
    NORSRAMInit.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
    NORSRAMInit.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
    NORSRAMInit.WriteBurst = FSMC_WRITE_BURST_DISABLE;

    FSMC_NORSRAM_Init( FSMC_Bank1, &NORSRAMInit );
    FSMC_NORSRAM_Timing_Init( FSMC_Bank1, &TimingInit, 0 );

    /* Enable FSMC NOR/SRAM Bank1 */
    __FSMC_NORSRAM_ENABLE( FSMC_Bank1, 0 );
}

/**
  * @brief  LCD Init.
  * @retval None
  */
void BSP_LCD_Init(void)
{ 
    unsigned long ulCount;

    /* Configure the LCD Control pins */
    LCD_CtrlLinesConfig();

    /* Configure the FSMC Parallel interface */
    LCD_FSMCConfig();

    _delay_(5);

    /* Reset LCD */
    HAL_GPIO_WritePin( LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_RESET );
    _delay_(10);
    HAL_GPIO_WritePin( LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET );

    /*
    SSD2119Init(void)
    */
    /* Enter sleep mode (if we are not already there).*/
    LCD_WriteReg(SSD2119_SLEEP_MODE_1_REG, 0x0001);

    /* Set initial power parameters. */
    LCD_WriteReg(SSD2119_PWR_CTRL_5_REG, 0x00B2);
    LCD_WriteReg(SSD2119_VCOM_OTP_1_REG, 0x0006);

    /* Start the oscillator.*/
    LCD_WriteReg(SSD2119_OSC_START_REG, 0x0001);

    /* Set pixel format and basic display orientation (scanning direction).*/
    LCD_WriteReg(SSD2119_OUTPUT_CTRL_REG, 0x30EF);
    LCD_WriteReg(SSD2119_LCD_DRIVE_AC_CTRL_REG, 0x0600);

    /* Exit sleep mode.*/
    LCD_WriteReg(SSD2119_SLEEP_MODE_1_REG, 0x0000);
    _delay_(5);

    /* Configure pixel color format and MCU interface parameters.*/
    LCD_WriteReg(SSD2119_ENTRY_MODE_REG, ENTRY_MODE_DEFAULT);

    /* Set analog parameters */
    LCD_WriteReg(SSD2119_SLEEP_MODE_2_REG, 0x0999);
    LCD_WriteReg(SSD2119_ANALOG_SET_REG, 0x3800);

    /* Enable the display */
    LCD_WriteReg(SSD2119_DISPLAY_CTRL_REG, 0x0033);

    /* Set VCIX2 voltage to 6.1V.*/
    LCD_WriteReg(SSD2119_PWR_CTRL_2_REG, 0x0005);

    /* Configure gamma correction.*/
    LCD_WriteReg(SSD2119_GAMMA_CTRL_1_REG, 0x0000);
    LCD_WriteReg(SSD2119_GAMMA_CTRL_2_REG, 0x0303);
    LCD_WriteReg(SSD2119_GAMMA_CTRL_3_REG, 0x0407);
    LCD_WriteReg(SSD2119_GAMMA_CTRL_4_REG, 0x0301);
    LCD_WriteReg(SSD2119_GAMMA_CTRL_5_REG, 0x0301);
    LCD_WriteReg(SSD2119_GAMMA_CTRL_6_REG, 0x0403);
    LCD_WriteReg(SSD2119_GAMMA_CTRL_7_REG, 0x0707);
    LCD_WriteReg(SSD2119_GAMMA_CTRL_8_REG, 0x0400);
    LCD_WriteReg(SSD2119_GAMMA_CTRL_9_REG, 0x0a00);
    LCD_WriteReg(SSD2119_GAMMA_CTRL_10_REG, 0x1000);

    /* Configure Vlcd63 and VCOMl */
    LCD_WriteReg(SSD2119_PWR_CTRL_3_REG, 0x000A);
    LCD_WriteReg(SSD2119_PWR_CTRL_4_REG, 0x2E00);

    /* Set the display size and ensure that the GRAM window is set to allow
     access to the full display buffer.*/
    LCD_WriteReg(SSD2119_V_RAM_POS_REG, (LCD_PIXEL_HEIGHT-1) << 8);
    LCD_WriteReg(SSD2119_H_RAM_START_REG, 0x0000);
    LCD_WriteReg(SSD2119_H_RAM_END_REG, LCD_PIXEL_WIDTH-1);

    LCD_WriteReg(SSD2119_X_RAM_ADDR_REG, 0x00);
    LCD_WriteReg(SSD2119_Y_RAM_ADDR_REG, 0x00);

    /* clear the lcd  */
    LCD_WriteReg(SSD2119_RAM_DATA_REG, 0x0000);
    for( ulCount = 0; ulCount < (LCD_PIXEL_WIDTH * LCD_PIXEL_HEIGHT); ulCount++ )
    {
        LCD_WriteRAM(0x0000);
    }
}


/**
  * @brief  Sets the cursor position.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position. 
  * @retval None
  */
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
  /* Set the X address of the display cursor.*/
  LCD_WriteReg(SSD2119_X_RAM_ADDR_REG, Xpos);
	
  /* Set the Y address of the display cursor.*/
  LCD_WriteReg(SSD2119_Y_RAM_ADDR_REG, Ypos);
}

/**
  * @brief  Writes to the selected LCD register.
  * @param  LCD_Reg: address of the selected register.
  * @param  LCD_RegValue: value to write to the selected register.
  * @retval None
  */
static void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue)
{
  /* Write 16-bit Index, then Write Reg */
  LCD_CMD = LCD_Reg;
  /* Write 16-bit Reg */
  LCD_Data = LCD_RegValue;
}

/**
  * @brief  Reads the selected LCD Register.
  * @param  LCD_Reg: address of the selected register.
  * @retval LCD Register Value.
  */
static uint16_t LCD_ReadReg(uint8_t LCD_Reg)
{
  /* Write 16-bit Index (then Read Reg) */
  LCD_CMD = LCD_Reg;
  /* Read 16-bit Reg */
  return (LCD_Data);
}

/**
  * @brief  Prepare to write to the LCD RAM.
  * @param  None
  * @retval None
  */
static void LCD_WriteRAM_Prepare(void)
{
	LCD_CMD = SSD2119_RAM_DATA_REG;
}

/**
  * @brief  Writes to the LCD RAM.
  * @param  RGB_Code: the pixel color in RGB mode (5-6-5).
  * @retval None
  */
static void LCD_WriteRAM(uint16_t RGB_Code)
{
  /* Write 16-bit GRAM Reg */
  LCD_Data = RGB_Code;
}

/**
  * @brief  Reads the LCD RAM.
  * @param  None
  * @retval LCD RAM Value.
  */
static uint16_t LCD_ReadRAM(void)
{
  /* Write 16-bit Index (then Read Reg) */
//  LCD_CMD = SSD2119_RAM_DATA_REG; /* Select GRAM Reg */
  /* Read 16-bit Reg */
  return LCD_Data;
}

/**
  * @brief  Test LCD Display
  * @retval None
  */
void LCD_RGB_Test(void)
{
  uint32_t index;

  LCD_SetCursor(0x00, 0x00); 
  LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */

	/* R */
  for(index = 0; index < (LCD_PIXEL_HEIGHT*LCD_PIXEL_WIDTH)/3; index++)
  {
    LCD_Data = LCD_COLOR_RED;
  }
	  
  /* G */
  for(;index < 2*(LCD_PIXEL_HEIGHT*LCD_PIXEL_WIDTH)/3; index++)
  {
    LCD_Data = LCD_COLOR_GREEN;
  }
	  
	/* B */
  for(; index < LCD_PIXEL_HEIGHT*LCD_PIXEL_WIDTH; index++)
  {
    LCD_Data = LCD_COLOR_BLUE;
  }
}

/**
  * @brief  Clears the selected line.
  * @param  Line: the top x coordinate of the line to be cleared.
  * @retval None
  */
void LCD_ClearLine( uint16_t Line, uint16_t Colour, sFONT* Font )
{
  uint16_t refcolumn = 0;

  do {
       /* Display one character on LCD */
    LCD_DisplayChar(Line, refcolumn, ' ', Colour, Colour, Font);
    /* Decrement the column position by 16 */
    refcolumn += Font->Width;
  } while (refcolumn < LCD_PIXEL_WIDTH);	
}

/**
  * @brief  Clears the whole LCD.
  * @param  Color: the color of the background.
  * @retval None
  */
void LCD_Clear(uint16_t Color)
{
  uint32_t index = 0;
  
  LCD_SetCursor(0x00, 0x00); 
  LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
  for(index = 0; index < LCD_PIXEL_HEIGHT*LCD_PIXEL_WIDTH; index++)
  {
    LCD_Data = Color;
  }  
}

/**
  * @brief  Displays a pixel.
  * @param  x: pixel x.
  * @param  y: pixel y.  
  * @retval None
  */
static void PutPixel(int16_t x, int16_t y, uint16_t Colour)
{ 
  if(x < 0 || x > LCD_PIXEL_WIDTH-1 || y < 0 || y > LCD_PIXEL_HEIGHT-1)
  {
    return;  
  }
  LCD_DrawLine(x, y, 1, LCD_DIR_HORIZONTAL, Colour);
}

/**
  * @brief  Draws a character on LCD.
  * @param  Xpos: the Line where to display the character shape.
  * @param  Ypos: start column address.
  * @param  c: pointer to the character data.
  * @retval None
  */
void LCD_DrawChar(uint16_t Xpos, uint16_t Ypos, const uint16_t *c, uint16_t Colour, uint16_t BackColour, uint16_t Height, uint16_t Width)
{
  uint32_t index = 0, i = 0;
  uint16_t  Xaddress = 0;
  Xaddress = Xpos;
  
  LCD_SetCursor(Ypos, Xaddress);
  
  for(index = 0; index < Height; index++)
  {
    LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
    for(i = 0; i < Width; i++)
    {
  
      if((((c[index] & ((0x80 << ((Width / 12 ) * 8 ) ) >> i)) == 0x00) &&(Width <= 12))||
        (((c[index] & (0x1 << i)) == 0x00)&&(Width > 12 )))

      {
        LCD_WriteRAM(BackColour);
      }
      else
      {
        LCD_WriteRAM(Colour);
      } 
    }
    Xaddress++;
    LCD_SetCursor(Ypos, Xaddress);
  }
}

/**
  * @brief  Displays one character (16dots width, 24dots height).
  * @param  Line: the Line where to display the character shape .
  *   This parameter can be one of the following values:
  *     @arg Linex: where x can be 0..9
  * @param  Column: start column address.
  * @param  Ascii: character ascii code, must be between 0x20 and 0x7E.
  * @retval None
  */
void LCD_DisplayChar(uint16_t Line, uint16_t Column, uint8_t Ascii, uint16_t Colour, uint16_t BackColour, sFONT* Font)
{
  Ascii -= 32;
  LCD_DrawChar(Line, Column, &Font->table[Ascii * Font->Height], Colour, BackColour, Font->Height, Font->Width);
}

/**
  * @brief  Displays a maximum of 20 char on the LCD.
  * @param  Line: the Line where to display the character shape .
  *   This parameter can be one of the following values:
  *     @arg Linex: where x can be 0..9
  * @param  *ptr: pointer to string to display on LCD.
  * @retval None
  */
void LCD_DisplayStringLine(uint16_t Line, const char *ptr, uint16_t Colour, uint16_t BackColour, sFONT* Font )
{
    LCD_DisplayStringLineCol( Line, 0, ptr, Colour, BackColour, Font );
}
void LCD_DisplayStringLineCol(uint16_t Line, uint16_t Column, const char *ptr, uint16_t Colour, uint16_t BackColour, sFONT* Font)
{

  uint16_t refcolumn = Column;

  /* Send the string character by character on lCD */
  while (*ptr != 0)
  {
    /* Display one character on LCD */
    LCD_DisplayChar(Line, refcolumn, (uint8_t)*ptr, Colour, BackColour, Font);
    /* Decrement the column position by 16 */
    refcolumn += Font->Width;
	if (refcolumn > (LCD_PIXEL_WIDTH - Font->Width) ) {
		break;
	}
    /* Point on the next character */
    ptr++;
  }
}

/**
  * @brief  Sets a display window
  * @param  Xpos: specifies the X bottom left position.
  * @param  Ypos: specifies the Y bottom left position.
  * @param  Height: display window width.
  * @param  Width: display window Height.
  * @retval None
  */
void LCD_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t width, uint16_t Height)
{
  uint32_t value = 0;	

  LCD_WriteReg(SSD2119_H_RAM_START_REG, Xpos);
	
  if ((Xpos+width) >= LCD_PIXEL_WIDTH) {
    LCD_WriteReg(SSD2119_H_RAM_END_REG, LCD_PIXEL_WIDTH-1);	
  } else {
    LCD_WriteReg(SSD2119_H_RAM_END_REG, Xpos+width);		
  }
  
  if ((Ypos+Height) >= LCD_PIXEL_HEIGHT) {
    value = (LCD_PIXEL_HEIGHT-1) << 8;	
  } else {
    value = (Ypos+Height) << 8;	
  }
  value |= Xpos;
  LCD_WriteReg(SSD2119_V_RAM_POS_REG, value);
  LCD_SetCursor(Xpos, Ypos);
}

/**
  * @brief  Disables LCD Window mode.
  * @param  None
  * @retval None
  */
void LCD_WindowModeDisable(void)
{
#if 0
  LCD_SetDisplayWindow(239, 0x13F, 240, 320);
  LCD_WriteReg(LCD_REG_3, 0x1018);    
#endif
}

/**
  * @brief  Displays a line.
  * @param Xpos: specifies the X position.
  * @param Ypos: specifies the Y position.
  * @param Length: line length.
  * @param Direction: line direction.
  *   This parameter can be one of the following values: Vertical or Horizontal.
  * @retval None
  */
void LCD_DrawLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length, uint8_t Direction, uint16_t Colour)
{
  uint32_t i = 0;
  
  if( Xpos >= LCD_PIXEL_WIDTH || Ypos >= LCD_PIXEL_HEIGHT )
      return;

  LCD_SetCursor(Xpos, Ypos);
  if(Direction == LCD_DIR_HORIZONTAL)
  {
    if( Xpos + Length > LCD_PIXEL_WIDTH )
      Length = LCD_PIXEL_WIDTH - Xpos;
    LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
    for(i = 0; i < Length; i++)
    {
      LCD_WriteRAM(Colour);
    }
  }
  else
  {
    if( Ypos + Length > LCD_PIXEL_HEIGHT )
      Length = LCD_PIXEL_HEIGHT - Ypos;
    for(i = 0; i < Length; i++)
    {
      LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
      LCD_WriteRAM(Colour);
      Ypos++;
      LCD_SetCursor(Xpos, Ypos);
    }
  }
}

/**
  * @brief  Displays a rectangle.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @param  Height: display rectangle height.
  * @param  Width: display rectangle width.
  * @retval None
  */
void LCD_DrawRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height, uint16_t Colour)
{
  if ( Width == 0 || Height == 0)
    return;

  LCD_DrawLine(Xpos, Ypos, Height, LCD_DIR_VERTICAL, Colour);
  LCD_DrawLine((Xpos + Width-1), Ypos, Height, LCD_DIR_VERTICAL, Colour);
  
  LCD_DrawLine(Xpos, Ypos, Width, LCD_DIR_HORIZONTAL, Colour);
  LCD_DrawLine(Xpos, (Ypos + Height-1), Width, LCD_DIR_HORIZONTAL, Colour);
}

/**
  * @brief  Displays a circle.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @param  Radius
  * @retval None
  */
void LCD_DrawCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius, uint16_t Colour)
{
  int32_t  D;/* Decision Variable */ 
  uint32_t  CurX;/* Current X Value */
  uint32_t  CurY;/* Current Y Value */ 
  
  D = 3 - (Radius << 1);
  CurX = 0;
  CurY = Radius;
  
  while (CurX <= CurY)
  {
    LCD_SetCursor(Xpos + CurX, Ypos + CurY);
    LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
    LCD_WriteRAM(Colour);
    LCD_SetCursor(Xpos + CurX, Ypos - CurY);
    LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
    LCD_WriteRAM(Colour);
    LCD_SetCursor(Xpos - CurX, Ypos + CurY);
    LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
    LCD_WriteRAM(Colour);
    LCD_SetCursor(Xpos - CurX, Ypos - CurY);
    LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
    LCD_WriteRAM(Colour);
    LCD_SetCursor(Xpos + CurY, Ypos + CurX);
    LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
    LCD_WriteRAM(Colour);
    LCD_SetCursor(Xpos + CurY, Ypos - CurX);
    LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
    LCD_WriteRAM(Colour);
    LCD_SetCursor(Xpos - CurY, Ypos + CurX);
    LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
    LCD_WriteRAM(Colour);
    LCD_SetCursor(Xpos - CurY, Ypos - CurX);
    LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
    LCD_WriteRAM(Colour);
    if (D < 0)
    { 
      D += (CurX << 2) + 6;
    }
    else
    {
      D += ((CurX - CurY) << 2) + 10;
      CurY--;
    }
    CurX++;
  }
}

/**
  * @brief  Displays a mono-color picture.
  * @param  Pict: pointer to the picture array.
  * @retval None
  */
void LCD_DrawMonoPict(const uint32_t *Pict, uint16_t Colour, uint16_t BackColour)
{
  uint32_t index = 0, i = 0;
  LCD_SetCursor(0, (LCD_PIXEL_WIDTH - 1)); 
  LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
  for(index = 0; index < 2400; index++)
  {
    for(i = 0; i < 32; i++)
    {
      if((Pict[index] & (1 << i)) == 0x00)
      {
        LCD_WriteRAM(BackColour);
      }
      else
      {
        LCD_WriteRAM(Colour);
      }
    }
  }
}

/**
  * @brief  Displays a bitmap picture loaded in the internal Flash.
  * @param  BmpAddress: Bmp picture address in the internal Flash.
  * @retval None
  */
void LCD_WriteBMP(uint32_t BmpAddress)
{
#if 0
  uint32_t index = 0, size = 0;
  /* Read bitmap size */
  size = *(__IO uint16_t *) (BmpAddress + 2);
  size |= (*(__IO uint16_t *) (BmpAddress + 4)) << 16;
  /* Get bitmap data address offset */
  index = *(__IO uint16_t *) (BmpAddress + 10);
  index |= (*(__IO uint16_t *) (BmpAddress + 12)) << 16;
  size = (size - index)/2;
  BmpAddress += index;
  /* Set GRAM write direction and BGR = 1 */
  /* I/D=00 (Horizontal : decrement, Vertical : decrement) */
  /* AM=1 (address is updated in vertical writing direction) */
  LCD_WriteReg(LCD_REG_3, 0x1008);
 
  LCD_WriteRAM_Prepare();
 
  for(index = 0; index < size; index++)
  {
    LCD_WriteRAM(*(__IO uint16_t *)BmpAddress);
    BmpAddress += 2;
  }
 
  /* Set GRAM write direction and BGR = 1 */
  /* I/D = 01 (Horizontal : increment, Vertical : decrement) */
  /* AM = 1 (address is updated in vertical writing direction) */
  LCD_WriteReg(LCD_REG_3, 0x1018);
#endif
}

/**
  * @brief  Displays a full rectangle.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @param  Height: rectangle height.
  * @param  Width: rectangle width.
  * @retval None
  */
void LCD_DrawFullRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height, uint16_t FrameColour, uint16_t FillColour )
{
  if ( Width == 0 || Height == 0)
    return;

  LCD_DrawRect( Xpos, Ypos, Width, Height, FrameColour );

  if ( Width > 2 && Height > 2)
  {
      Height-=2;
      Ypos++;

      while(Height--)
      {
        LCD_DrawLine(Xpos+1, Ypos++, Width-2, LCD_DIR_HORIZONTAL, FillColour);
      }
  }
}

#if 0 //this function is broken too..
/**
  * @brief  Displays a full circle.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @param  Radius
  * @retval None
  */
void LCD_DrawFullCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius, uint16_t Colour )
{
  int32_t  D;    /* Decision Variable */ 
  uint32_t  CurX;/* Current X Value */
  uint32_t  CurY;/* Current Y Value */ 
  
  D = 3 - (Radius << 1);

  CurX = 0;
  CurY = Radius;
  
  while (CurX <= CurY)
  {
    if(CurY > 0) 
    {
      LCD_DrawLine(Xpos - CurX, Ypos + CurY, 2*CurX, LCD_DIR_HORIZONTAL, Colour);
      LCD_DrawLine(Xpos + CurX, Ypos + CurY, 2*CurX, LCD_DIR_HORIZONTAL, Colour);
    }

    if(CurX > 0) 
    {
      LCD_DrawLine(Xpos - CurY, Ypos + CurX, 2*CurY, LCD_DIR_HORIZONTAL, Colour);
      LCD_DrawLine(Xpos + CurY, Ypos + CurX, 2*CurY, LCD_DIR_HORIZONTAL, Colour);
    }
    if (D < 0)
    { 
      D += (CurX << 2) + 6;
    }
    else
    {
      D += ((CurX - CurY) << 2) + 10;
      CurY--;
    }
    CurX++;
  }

  LCD_DrawCircle(Xpos, Ypos, Radius, Colour);
}
#endif
/**
  * @brief  Displays an uni-line (between two points).
  * @param  x1: specifies the point 1 x position.
  * @param  y1: specifies the point 1 y position.
  * @param  x2: specifies the point 2 x position.
  * @param  y2: specifies the point 2 y position.
  * @retval None
  */
void LCD_DrawUniLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t Colour)
{
  int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, 
  yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0, 
  curpixel = 0;
  
  deltax = ABS(x2 - x1);        /* The difference between the x's */
  deltay = ABS(y2 - y1);        /* The difference between the y's */
  x = x1;                       /* Start x off at the first pixel */
  y = y1;                       /* Start y off at the first pixel */
  
  if (x2 >= x1)                 /* The x-values are increasing */
  {
    xinc1 = 1;
    xinc2 = 1;
  }
  else                          /* The x-values are decreasing */
  {
    xinc1 = -1;
    xinc2 = -1;
  }
  
  if (y2 >= y1)                 /* The y-values are increasing */
  {
    yinc1 = 1;
    yinc2 = 1;
  }
  else                          /* The y-values are decreasing */
  {
    yinc1 = -1;
    yinc2 = -1;
  }
  
  if (deltax >= deltay)         /* There is at least one x-value for every y-value */
  {
    xinc1 = 0;                  /* Don't change the x when numerator >= denominator */
    yinc2 = 0;                  /* Don't change the y for every iteration */
    den = deltax;
    num = deltax / 2;
    numadd = deltay;
    numpixels = deltax;         /* There are more x-values than y-values */
  }
  else                          /* There is at least one y-value for every x-value */
  {
    xinc2 = 0;                  /* Don't change the x for every iteration */
    yinc1 = 0;                  /* Don't change the y when numerator >= denominator */
    den = deltay;
    num = deltay / 2;
    numadd = deltax;
    numpixels = deltay;         /* There are more y-values than x-values */
  }
  
  for (curpixel = 0; curpixel <= numpixels; curpixel++)
  {
    PutPixel(x, y, Colour);             /* Draw the current pixel */
    num += numadd;              /* Increase the numerator by the top of the fraction */
    if (num >= den)             /* Check if numerator >= denominator */
    {
      num -= den;               /* Calculate the new numerator value */
      x += xinc1;               /* Change the x as appropriate */
      y += yinc1;               /* Change the y as appropriate */
    }
    x += xinc2;                 /* Change the x as appropriate */
    y += yinc2;                 /* Change the y as appropriate */
  }
}

/**
  * @brief  Displays an poly-line (between many points).
  * @param  Points: pointer to the points array.
  * @param  PointCount: Number of points.
  * @retval None
  */
void LCD_PolyLine(pPoint Points, uint16_t PointCount, uint16_t Colour)
{
  int16_t X = 0, Y = 0;

  if(PointCount < 2)
  {
    return;
  }

  while(--PointCount)
  {
    X = Points->X;
    Y = Points->Y;
    Points++;
    LCD_DrawUniLine(X, Y, Points->X, Points->Y, Colour);
  }
}

/**
  * @brief  Displays an relative poly-line (between many points).
  * @param  Points: pointer to the points array.
  * @param  PointCount: Number of points.
  * @param  Closed: specifies if the draw is closed or not.
  *           1: closed, 0 : not closed.
  * @retval None
  */
static void LCD_PolyLineRelativeClosed(pPoint Points, uint16_t PointCount, uint16_t Closed, uint16_t Colour)
{
  int16_t X = 0, Y = 0;
  pPoint First = Points;

  if(PointCount < 2)
  {
    return;
  }  
  X = Points->X;
  Y = Points->Y;
  while(--PointCount)
  {
    Points++;
    LCD_DrawUniLine(X, Y, X + Points->X, Y + Points->Y, Colour);
    X = X + Points->X;
    Y = Y + Points->Y;
  }
  if(Closed)
  {
    LCD_DrawUniLine(First->X, First->Y, X, Y, Colour);
  }  
}

/**
  * @brief  Displays a closed poly-line (between many points).
  * @param  Points: pointer to the points array.
  * @param  PointCount: Number of points.
  * @retval None
  */
void LCD_ClosedPolyLine(pPoint Points, uint16_t PointCount, uint16_t Colour)
{
  LCD_PolyLine(Points, PointCount, Colour);
  LCD_DrawUniLine(Points->X, Points->Y, (Points+PointCount-1)->X, (Points+PointCount-1)->Y, Colour);
}

/**
  * @brief  Displays a relative poly-line (between many points).
  * @param  Points: pointer to the points array.
  * @param  PointCount: Number of points.
  * @retval None
  */
void LCD_PolyLineRelative(pPoint Points, uint16_t PointCount, uint16_t Colour)
{
  LCD_PolyLineRelativeClosed(Points, PointCount, 0, Colour);
}

/**
  * @brief  Displays a closed relative poly-line (between many points).
  * @param  Points: pointer to the points array.
  * @param  PointCount: Number of points.
  * @retval None
  */
void LCD_ClosedPolyLineRelative(pPoint Points, uint16_t PointCount, uint16_t Colour)
{
  LCD_PolyLineRelativeClosed(Points, PointCount, 1, Colour);
}

/**
  * @brief  Displays a  full poly-line (between many points).
  * @param  Points: pointer to the points array.
  * @param  PointCount: Number of points.
  * @retval None
  */
void LCD_FillPolyLine(pPoint Points, uint16_t PointCount, uint16_t Colour )
{
  /*  public-domain code by Darel Rex Finley, 2007 */
  uint16_t  nodes = 0, nodeX[MAX_POLY_CORNERS], pixelX = 0, pixelY = 0, i = 0,
  j = 0, swap = 0;
  uint16_t  IMAGE_LEFT = 0, IMAGE_RIGHT = 0, IMAGE_TOP = 0, IMAGE_BOTTOM = 0;

  IMAGE_LEFT = IMAGE_RIGHT = Points->X;
  IMAGE_TOP= IMAGE_BOTTOM = Points->Y;

  for(i = 1; i < PointCount; i++)
  {
    pixelX = POLY_X(i);
    if(pixelX < IMAGE_LEFT)
    {
      IMAGE_LEFT = pixelX;
    }
    if(pixelX > IMAGE_RIGHT)
    {
      IMAGE_RIGHT = pixelX;
    }
    
    pixelY = POLY_Y(i);
    if(pixelY < IMAGE_TOP)
    { 
      IMAGE_TOP = pixelY;
    }
    if(pixelY > IMAGE_BOTTOM)
    {
      IMAGE_BOTTOM = pixelY;
    }
  }
  
  /*  Loop through the rows of the image. */
  for (pixelY = IMAGE_TOP; pixelY < IMAGE_BOTTOM; pixelY++) 
  {  
    /* Build a list of nodes. */
    nodes = 0; j = PointCount-1;

    for (i = 0; i < PointCount; i++) 
    {
      if (((POLY_Y(i)<(double) pixelY) && (POLY_Y(j)>=(double) pixelY)) || \
          ((POLY_Y(j)<(double) pixelY) && (POLY_Y(i)>=(double) pixelY)))
      {
        nodeX[nodes++]=(int) (POLY_X(i)+((pixelY-POLY_Y(i))*(POLY_X(j)-POLY_X(i)))/(POLY_Y(j)-POLY_Y(i))); 
      }
      j = i; 
    }
  
    /* Sort the nodes, via a simple "Bubble" sort. */
    i = 0;
    while (i < nodes-1) 
    {
      if (nodeX[i]>nodeX[i+1]) 
      {
        swap = nodeX[i]; 
        nodeX[i] = nodeX[i+1]; 
        nodeX[i+1] = swap; 
        if(i)
        {
          i--; 
        }
      }
      else 
      {
        i++;
      }
    }
  
    /*  Fill the pixels between node pairs. */
    for (i = 0; i < nodes; i+=2) 
    {
      if(nodeX[i] >= IMAGE_RIGHT) 
      {
        break;
      }
      if(nodeX[i+1] > IMAGE_LEFT) 
      {
        if (nodeX[i] < IMAGE_LEFT)
        {
          nodeX[i]=IMAGE_LEFT;
        }
        if(nodeX[i+1] > IMAGE_RIGHT)
        {
          nodeX[i+1] = IMAGE_RIGHT;
        }
        //LCD_SetTextColor(BackColor);
        LCD_DrawLine(pixelY, nodeX[i+1], nodeX[i+1] - nodeX[i], LCD_DIR_HORIZONTAL, Colour);
        //LCD_SetTextColor(TextColor);
        PutPixel(pixelY, nodeX[i+1], Colour);
        PutPixel(pixelY, nodeX[i], Colour);
        /* for (j=nodeX[i]; j<nodeX[i+1]; j++) PutPixel(j,pixelY); */
      }
    }
  } 
  /* draw the edges */
  //LCD_SetTextColor(TextColor);
}

#ifndef USE_Delay
/**
  * @brief  Inserts a delay time.
  * @param  nCount: specifies the delay time length.
  * @retval None
  */
static void delay(__IO uint32_t nCount)
{
  __IO uint32_t index = 0; 
  for(index = (10000 * nCount); index != 0; index--)
  {
  }
}
#endif /* USE_Delay*/
/**
  * @}
  */ 

/**
  * @}
  */ 
  
/**
  * @}
  */ 

/**
  * @}
  */ 
  
/**
  * @}
  */  

/*********** Portions COPYRIGHT 2012 Embest Tech. Co., Ltd.*****END OF FILE****/
