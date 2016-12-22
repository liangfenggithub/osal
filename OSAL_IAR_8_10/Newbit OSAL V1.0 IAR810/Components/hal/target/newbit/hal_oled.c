/**************************************************************************************************
  Filename:       hal_oled.c
  Revised:        $Date: 2016-09-21 17:31:27 -0700 (Mon, 21 Jun 2010) $
  Revision:       $Revision: 22794 $

  Description:    This file contains the interface to the HAL LCD Service.

  Should you have any questions regarding your right to use this Software,
  contact Newbit Studio at www.newbitstudio.com
**************************************************************************************************/

/**************************************************************************************************
 *                                           INCLUDES
 **************************************************************************************************/
#include "hal_types.h"
#include "hal_lcd.h"
#include "font.h"
#include "OSAL.h"
#include "OnBoard.h"
#include "hal_assert.h"
   
#include "hal_flash.h"

#if defined (ZTOOL_P1) || defined (ZTOOL_P2)
  #include "DebugTrace.h"
#endif

/**************************************************************************************************
 *                                          CONSTANTS
 **************************************************************************************************/
/*
  LCD pins

  //control
  P0.0 - LCD_MODE
  P1.1 - LCD_FLASH_RESET
  P1.2 - LCD_CS

  //spi
  P1.5 - CLK
  P1.6 - MOSI
  P1.7 - MISO
*/

/* LCD Control lines */

#define HAL_LCD_CS_PORT 1
#define HAL_LCD_CS_PIN  2

/* LCD SPI lines */
#define HAL_LCD_CLK_PORT 1
#define HAL_LCD_CLK_PIN  5

#define HAL_LCD_MOSI_PORT 1
#define HAL_LCD_MOSI_PIN  6

#define HAL_LCD_MISO_PORT 1
#define HAL_LCD_MISO_PIN  7

/* SPI settings */
#define HAL_SPI_CLOCK_POL_LO       0x00
#define HAL_SPI_CLOCK_PHA_0        0x00
#define HAL_SPI_TRANSFER_MSB_FIRST 0x20

/* LCD lines */
#define LCD_MAX_LINE_COUNT              7
#define LCD_MAX_LINE_LENGTH             16
#define LCD_MAX_BUF                     25

/* Defines for HW LCD */

#define DC_TYPE_CMD                     0X00
#define DC_TYPE_DATA                    0X01



/* SSD 1306 Command table */
#define SSD1306_CONTRAST_CTRL   0x81





/*  Define for Fonts */   
#define    FONT_FLASH_PAGE  8
   

/**************************************************************************************************
 *                                           MACROS
 **************************************************************************************************/

#define HAL_IO_SET(port, pin, val)        HAL_IO_SET_PREP(port, pin, val)
#define HAL_IO_SET_PREP(port, pin, val)   st( P##port##_##pin## = val; )

#define HAL_CONFIG_IO_OUTPUT(port, pin, val)      HAL_CONFIG_IO_OUTPUT_PREP(port, pin, val)
#define HAL_CONFIG_IO_OUTPUT_PREP(port, pin, val) st( P##port##SEL &= ~BV(pin); \
                                                      P##port##_##pin## = val; \
                                                      P##port##DIR |= BV(pin); )

#define HAL_CONFIG_IO_PERIPHERAL(port, pin)      HAL_CONFIG_IO_PERIPHERAL_PREP(port, pin)
#define HAL_CONFIG_IO_PERIPHERAL_PREP(port, pin) st( P##port##SEL |= BV(pin); )



/* SPI interface control */
#define LCD_SPI_BEGIN()     HAL_IO_SET(HAL_LCD_CS_PORT,  HAL_LCD_CS_PIN,  0); /* chip select */
#define LCD_SPI_END()                                                         \
{                                                                             \
  asm("NOP");                                                                 \
  asm("NOP");                                                                 \
  asm("NOP");                                                                 \
  asm("NOP");                                                                 \
  HAL_IO_SET(HAL_LCD_CS_PORT,  HAL_LCD_CS_PIN,  1); /* chip select */         \
}

#define LCD_SPI_CLK_LOW()     HAL_IO_SET(HAL_LCD_CLK_PORT,  HAL_LCD_CLK_PIN,  0)
#define LCD_SPI_CLK_HIGH()     HAL_IO_SET(HAL_LCD_CLK_PORT,  HAL_LCD_CLK_PIN,  1)

#define LCD_SPI_MO_SET(x)     HAL_IO_SET(HAL_LCD_MOSI_PORT,  HAL_LCD_MOSI_PIN,  x)


/* clear the received and transmit byte status, write tx data to buffer, wait till transmit done */
#define LCD_SPI_TX(x)                   { U1CSR &= ~(BV(2) | BV(1)); U1DBUF = x; while( !(U1CSR & BV(1)) ); }
#define LCD_SPI_WAIT_RXRDY()            { while(!(U1CSR & BV(1))); }





/* Control macros */
#define LCD_DO_WRITE()        HAL_IO_SET(HAL_LCD_MODE_PORT,  HAL_LCD_MODE_PIN,  1);
#define LCD_DO_CONTROL()      HAL_IO_SET(HAL_LCD_MODE_PORT,  HAL_LCD_MODE_PIN,  0);

#define LCD_ACTIVATE_RESET()  HAL_IO_SET(HAL_LCD_RESET_PORT, HAL_LCD_RESET_PIN, 0);
#define LCD_RELEASE_RESET()   HAL_IO_SET(HAL_LCD_RESET_PORT, HAL_LCD_RESET_PIN, 1);


#define LCD_SAMPLE_DELAY()      HalLcd_HW_WaitUs(0)

#if (HAL_LCD == TRUE)
/**************************************************************************************************
 *                                       LOCAL VARIABLES
 **************************************************************************************************/

static uint8 *Lcd_Line1;

/**************************************************************************************************
 *                                       FUNCTIONS - API
 **************************************************************************************************/

void HalLcd_HW_Init(void);
void HalLcd_HW_WaitUs(uint16 i);
void HalLcd_HW_Clear(void);
void HalLcd_HW_ClearAllSpecChars(void);
void HalLcd_HW_Control(uint8 cmd);
void HalLcd_HW_Write(uint8 data);
void HalLcd_HW_SetContrast(uint8 value);
void HalLcd_HW_WriteChar(uint8 line, uint8 col, char text);
void HalLcd_HW_WriteLine(uint8 line, const char *pText);

void HalLcd_HW_SetCursor(uint8 line, uint8 col);

static void halLcd_Write(uint8 dc, uint8 byte);
#endif //LCD

/**************************************************************************************************
 * @fn      HalLcdInit
 *
 * @brief   Initilize LCD Service
 *
 * @param   init - pointer to void that contains the initialized value
 *
 * @return  None
 **************************************************************************************************/
void HalLcdInit(void)
{
#if (HAL_LCD == TRUE)
  Lcd_Line1 = NULL;
  HalLcd_HW_Init();
#endif
}

/*************************************************************************************************
 *                    LCD EMULATION FUNCTIONS
 *
 * Some evaluation boards are equipped with Liquid Crystal Displays
 * (LCD) which may be used to display diagnostic information. These
 * functions provide LCD emulation, sending the diagnostic strings
 * to Z-Tool via the RS232 serial port. These functions are enabled
 * when the "LCD_SUPPORTED" compiler flag is placed in the makefile.
 *
 * Most applications update both lines (1 and 2) of the LCD whenever
 * text is posted to the device. This emulator assumes that line 1 is
 * updated first (saved locally) and the formatting and send operation
 * is triggered by receipt of line 2. Nothing will be transmitted if
 * only line 1 is updated.
 *
 *************************************************************************************************/


/**************************************************************************************************
 * @fn      HalLcdWriteString
 *
 * @brief   Write a string to the LCD
 *
 * @param   str    - pointer to the string that will be displayed
 *          option - display options
 *
 * @return  None
 **************************************************************************************************/
void HalLcdWriteString ( char *str, uint8 option)
{
#if (HAL_LCD == TRUE)

  uint8 strLen = 0;
  uint8 totalLen = 0;
  uint8 *buf;
  uint8 tmpLen;

  if ( Lcd_Line1 == NULL )
  {
    Lcd_Line1 = osal_mem_alloc( HAL_LCD_MAX_CHARS+1 );
    HalLcdWriteString( "NEWBIT STUDIO", 1 );
  }

  strLen = (uint8)osal_strlen( (char*)str );

  /* Check boundries */
  if ( strLen > HAL_LCD_MAX_CHARS )
    strLen = HAL_LCD_MAX_CHARS;

  if ( option == HAL_LCD_LINE_1 )
  {
    /* Line 1 gets saved for later */
    osal_memcpy( Lcd_Line1, str, strLen );
    Lcd_Line1[strLen] = '\0';
  }
  else
  {
    /* Line 2 triggers action */
    tmpLen = (uint8)osal_strlen( (char*)Lcd_Line1 );
    totalLen =  tmpLen + 1 + strLen + 1;
    buf = osal_mem_alloc( totalLen );
    if ( buf != NULL )
    {
      /* Concatenate strings */
      osal_memcpy( buf, Lcd_Line1, tmpLen );
      buf[tmpLen++] = ' ';
      osal_memcpy( &buf[tmpLen], str, strLen );
      buf[tmpLen+strLen] = '\0';

      /* Send it out */
#if defined (ZTOOL_P1) || defined (ZTOOL_P2)

#if defined(SERIAL_DEBUG_SUPPORTED)
      debug_str( (uint8*)buf );
#endif //LCD_SUPPORTED

#endif //ZTOOL_P1

      /* Free mem */
      osal_mem_free( buf );
    }
  }

  /* Display the string */
  HalLcd_HW_WriteLine (option-1, str);

#endif //HAL_LCD

}

/**************************************************************************************************
 * @fn      HalLcdWriteValue
 *
 * @brief   Write a value to the LCD
 *
 * @param   value  - value that will be displayed
 *          radix  - 8, 10, 16
 *          option - display options
 *
 * @return  None
 **************************************************************************************************/
void HalLcdWriteValue ( uint32 value, const uint8 radix, uint8 option)
{
#if (HAL_LCD == TRUE)
  uint8 buf[LCD_MAX_BUF];

  _ltoa( value, &buf[0], radix );
  HalLcdWriteString( (char*)buf, option );
#endif
}

/**************************************************************************************************
 * @fn      HalLcdWriteScreen
 *
 * @brief   Write a value to the LCD
 *
 * @param   line1  - string that will be displayed on line 1
 *          line2  - string that will be displayed on line 2
 *
 * @return  None
 **************************************************************************************************/
void HalLcdWriteScreen( char *line1, char *line2 )
{
#if (HAL_LCD == TRUE)
  HalLcdWriteString( line1, 1 );
  HalLcdWriteString( line2, 2 );
#endif
}

/**************************************************************************************************
 * @fn      HalLcdWriteStringValue
 *
 * @brief   Write a string followed by a value to the LCD
 *
 * @param   title  - Title that will be displayed before the value
 *          value  - value
 *          format - redix
 *          line   - line number
 *
 * @return  None
 **************************************************************************************************/
void HalLcdWriteStringValue( char *title, uint16 value, uint8 format, uint8 line )
{
#if (HAL_LCD == TRUE)
  uint8 tmpLen;
  uint8 buf[LCD_MAX_BUF];
  uint32 err;

  tmpLen = (uint8)osal_strlen( (char*)title );
  osal_memcpy( buf, title, tmpLen );
  buf[tmpLen] = ' ';
  err = (uint32)(value);
  _ltoa( err, &buf[tmpLen+1], format );
  HalLcdWriteString( (char*)buf, line );		
#endif
}

/**************************************************************************************************
 * @fn      HalLcdWriteStringValue
 *
 * @brief   Write a string followed by a value to the LCD
 *
 * @param   title   - Title that will be displayed before the value
 *          value1  - value #1
 *          format1 - redix of value #1
 *          value2  - value #2
 *          format2 - redix of value #2
 *          line    - line number
 *
 * @return  None
 **************************************************************************************************/
void HalLcdWriteStringValueValue( char *title, uint16 value1, uint8 format1,
                                  uint16 value2, uint8 format2, uint8 line )
{

#if (HAL_LCD == TRUE)

  uint8 tmpLen;
  uint8 buf[LCD_MAX_BUF];
  uint32 err;

  tmpLen = (uint8)osal_strlen( (char*)title );
  if ( tmpLen )
  {
    osal_memcpy( buf, title, tmpLen );
    buf[tmpLen++] = ' ';
  }

  err = (uint32)(value1);
  _ltoa( err, &buf[tmpLen], format1 );
  tmpLen = (uint8)osal_strlen( (char*)buf );

  buf[tmpLen++] = ',';
  buf[tmpLen++] = ' ';
  err = (uint32)(value2);
  _ltoa( err, &buf[tmpLen], format2 );

  HalLcdWriteString( (char *)buf, line );		

#endif
}

/**************************************************************************************************
 * @fn      HalLcdDisplayPercentBar
 *
 * @brief   Display percentage bar on the LCD
 *
 * @param   line   -  0-7
 *          value   -
 *
 * @return  None
 **************************************************************************************************/
void HalLcdDisplayPercentBar( uint8 line, uint8 value )
{
#if (HAL_LCD == TRUE)
  
  uint8 scroll = 14 + value;
  
  HalLcd_HW_SetCursor( line, 0);  
  uint8 i = 0;
  
  for ( i = 0; i < 13; i ++ )
  {
    HalLcd_HW_Write(0xFF);
  }
  HalLcd_HW_Write(0x81);
  
  for ( ; i < scroll; i ++ )
  {
    HalLcd_HW_Write(0x81 | 0x3c);
  }
  
  for ( ; i < 114; i++ )
  {
    HalLcd_HW_Write(0x81);
  }
  
  HalLcd_HW_Write(0x81);
  for ( ; i < 128; i ++ )
  {
    HalLcd_HW_Write(0xFF);
  }
  
 
#else
 (void)option;
 (void)value);
#endif

}

#if (HAL_LCD == TRUE)
/**************************************************************************************************
 *                                    HARDWARE LCD
 **************************************************************************************************/

/**************************************************************************************************
 * @fn      halLcd_ConfigIO
 *
 * @brief   Configure IO lines needed for LCD control.
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
static void halLcd_ConfigIO(void)
{
  /* GPIO configuration */
  HAL_CONFIG_IO_OUTPUT(HAL_LCD_CS_PORT,    HAL_LCD_CS_PIN,    1);
}

/**************************************************************************************************
 * @fn      halLcd_ConfigSPI
 *
 * @brief   Configure SPI lines needed for talking to LCD.
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
static void halLcd_ConfigSPI(void)
{
  // 3-WIRE SPI , 每次都需要传输9bit, 因为使用模块SPI更加方便
  
  HAL_CONFIG_IO_OUTPUT(HAL_LCD_CLK_PORT,     HAL_LCD_CLK_PIN,    1);
  HAL_CONFIG_IO_OUTPUT(HAL_LCD_MOSI_PORT,    HAL_LCD_MOSI_PIN,    1);
  
  // MISO 不需要
  
#if 0
  
  /* UART/SPI Peripheral configuration */

   uint8 baud_exponent;
   uint8 baud_mantissa;

  /* Set SPI on UART 1 alternative 2 */
  PERCFG |= 0x02;

  /* Configure clk, master out and master in lines */
  HAL_CONFIG_IO_PERIPHERAL(HAL_LCD_CLK_PORT,  HAL_LCD_CLK_PIN);
  HAL_CONFIG_IO_PERIPHERAL(HAL_LCD_MOSI_PORT, HAL_LCD_MOSI_PIN);
  //HAL_CONFIG_IO_PERIPHERAL(HAL_LCD_MISO_PORT, HAL_LCD_MISO_PIN);


  /* Set SPI speed to 1 MHz (the values assume system clk of 32MHz)
   * Confirm on board that this results in 1MHz spi clk.
   */
  baud_exponent = 15;
  baud_mantissa =  0;

  /* Configure SPI */
  U1UCR  = 0x80;      /* Flush and goto IDLE state. 8-N-1. */
  U1CSR  = 0x00;      /* SPI mode, master. */
  U1GCR  = HAL_SPI_TRANSFER_MSB_FIRST | HAL_SPI_CLOCK_PHA_0 | HAL_SPI_CLOCK_POL_LO | baud_exponent;
  U1BAUD = baud_mantissa;
  
#endif
}



/**************************************************************************************************
 * @fn      halLcd_Write
 *
 * @brief   Write 9 bits to LCD
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
static void halLcd_Write(uint8 dc, uint8 byte)
{
  uint8 i = 8;
  
  uint8 bit = dc ? 1 : 0;
  
  LCD_SPI_CLK_LOW();       // falling  
  LCD_SPI_MO_SET( bit );  
  LCD_SAMPLE_DELAY();
  LCD_SPI_CLK_HIGH();        // rising
  LCD_SAMPLE_DELAY();
  
  while(i--)
  {
    bit = ( byte & 0x80 ) ? 1 : 0;
    LCD_SPI_CLK_LOW();       // falling  
    LCD_SPI_MO_SET( bit );  
    LCD_SAMPLE_DELAY();
    LCD_SPI_CLK_HIGH();        // rising
    LCD_SAMPLE_DELAY();
    byte <<= 1;
  }
}

/**************************************************************************************************
 * @fn      HalLcd_HW_Init
 *
 * @brief   Initilize HW LCD Driver.
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
void HalLcd_HW_Init(void)
{
  /* Initialize LCD IO lines */
  halLcd_ConfigIO();

  /* Initialize SPI */
  halLcd_ConfigSPI();
  
  HalLcd_HW_WaitUs(6000);
  
  /* Perform the initialization sequence */
#if  1
//
//       HalLcd_HW_Control(0xAE);    /*display off*/ 
//      
//       HalLcd_HW_Control(0x00);    /*set lower column address*/       
//       HalLcd_HW_Control(0x10);    /*set higher column address*/       
//
//       HalLcd_HW_Control(0x40);    /*set display start line*/ 
//      
//       HalLcd_HW_Control(0xB0);    /*set page address*/     
//  
//       HalLcd_HW_Control(0x81);    /*contract control*/
//       HalLcd_HW_Control(0x66);    /*128*/  
//     
//       HalLcd_HW_Control(0xA1);    /*set segment remap   0XA0*/ 
//     
//       HalLcd_HW_Control(0xA6);    /*normal / reverse*/  
//     
//       HalLcd_HW_Control(0xA8);    /*multiplex ratio*/
//       HalLcd_HW_Control(0x3F);    /*duty = 1/64*/  
//     
//       HalLcd_HW_Control(0xC8);    /*Com scan direction  0XC0*/   
//    
//       HalLcd_HW_Control(0xD3);    /*set display offset*/
//       HalLcd_HW_Control(0x00);       
//
//       HalLcd_HW_Control(0xD5);    /*set osc division*/
//       HalLcd_HW_Control(0x80);          
//
//       HalLcd_HW_Control(0xD9);    /*set pre-charge period*/
//       HalLcd_HW_Control(0x1f);      
// 
//       HalLcd_HW_Control(0xDA);    /*set COM pins*/
//       HalLcd_HW_Control(0x12);      
// 
//       HalLcd_HW_Control(0xdb);    /*set vcomh*/
//       HalLcd_HW_Control(0x30);       
//
//       HalLcd_HW_Control(0x8d);    /*set charge pump disable*/
//       HalLcd_HW_Control(0x14);      
// 
//       HalLcd_HW_Control(0xAF);    /*display ON*/   
       
       HalLcd_HW_Control(0xAE);    /*display off*/


       HalLcd_HW_Control(0x02);    /*set lower column address*/
       HalLcd_HW_Control(0x10);    /*set higher column address*/

       HalLcd_HW_Control(0x40);    /*set display start line*/

       HalLcd_HW_Control(0xB0);    /*set page address*/

       HalLcd_HW_Control(0x81);    /*contract control*/
       HalLcd_HW_Control(0xcf);    /*128*/

       HalLcd_HW_Control(0xA1);    /*set segment remap*/

       HalLcd_HW_Control(0xA6);    /*normal / reverse*/

       HalLcd_HW_Control(0xA8);    /*multiplex ratio*/
       HalLcd_HW_Control(0x3F);    /*duty = 1/64*/

       HalLcd_HW_Control(0xC8);    /*Com scan direction*/

       HalLcd_HW_Control(0xD3);    /*set display offset*/
       HalLcd_HW_Control(0x00);

       HalLcd_HW_Control(0xD5);    /*set osc division*/
       HalLcd_HW_Control(0x80);

       HalLcd_HW_Control(0xD9);    /*set pre-charge period*/
       HalLcd_HW_Control(0xf1);

       HalLcd_HW_Control(0xDA);    /*set COM pins*/
       HalLcd_HW_Control(0x12);

        HalLcd_HW_Control(0xdb);    /*set vcomh*/
       HalLcd_HW_Control(0x40);

        HalLcd_HW_Control(0x8d);    /*set charge pump enable*/
       HalLcd_HW_Control(0x14);

       HalLcd_HW_Control(0xAF);    /*display ON*/
  

#else
  FUNCTION_SET(CGRAM | COM_FORWARD | THREE_LINE);
  
  /* Set contrast */
  HalLcd_HW_SetContrast(15);

  /* Set power */
  SET_POWER_SAVE_MODE(OSC_OFF | POWER_SAVE_ON);
  SET_POWER_CTRL(VOLTAGE_DIVIDER_ON | CONVERTER_AND_REG_ON);
  SET_BIAS_CTRL(BIAS_1_5);
  HalLcd_HW_WaitUs(21000);// 21 ms

  /* Clear the display */
  HalLcd_HW_Clear();
  HalLcd_HW_ClearAllSpecChars();
  SET_DISPLAY_CTRL(DISPLAY_CTRL_ON | DISPLAY_CTRL_BLINK_OFF | DISPLAY_CTRL_CURSOR_OFF);
#endif
  
  HalLcd_HW_Clear();
  
  //HalLcd_HW_WriteLine(0, "!1234Aa~");
  
}

/**************************************************************************************************
 * @fn      HalLcd_HW_Control
 *
 * @brief   Write 1 command to the LCD
 *
 * @param   uint8 cmd - command to be written to the LCD
 *
 * @return  None
 **************************************************************************************************/
void HalLcd_HW_Control(uint8 cmd)
{
  LCD_SPI_BEGIN();
  //LCD_SPI_TX(cmd);
  //LCD_SPI_WAIT_RXRDY();
  halLcd_Write(DC_TYPE_CMD, cmd );
  LCD_SPI_END();
}

/**************************************************************************************************
 * @fn      HalLcd_HW_Write
 *
 * @brief   Write 1 byte to the LCD
 *
 * @param   uint8 data - data to be written to the LCD
 *
 * @return  None
 **************************************************************************************************/
void HalLcd_HW_Write(uint8 data)
{
  LCD_SPI_BEGIN();
  //LCD_DO_WRITE();
  //LCD_SPI_TX(data);
  //LCD_SPI_WAIT_RXRDY();
  halLcd_Write(DC_TYPE_DATA, data );
  LCD_SPI_END();
}

/**************************************************************************************************
 * @fn          HalLcd_HW_SetContrast
 *
 * @brief       Set display contrast
 *
 * @param       uint8 value - contrast value 0x00- 0xff (defalt 0x7f)
 *
 * @return      none
 **************************************************************************************************/
void HalLcd_HW_SetContrast(uint8 value)
{
  HalLcd_HW_Control( SSD1306_CONTRAST_CTRL );
  HalLcd_HW_Control(value);
}

/**************************************************************************************************
 * @fn      HalLcd_HW_Clear
 *
 * @brief   Clear the HW LCD
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
void HalLcd_HW_Clear(void)
{  
  HalLcd_HW_Control(0x00);    /*set lower column address*/
  HalLcd_HW_Control(0x10);    /*set higher column address*/
  //HalLcd_HW_Control(0xB0);    /*set page address*/
  
  uint8 pagectrl = 0xB0;
  
  uint16 x,y;
  
  for ( y = 0; y < 8; y++ )
  {
    HalLcd_HW_Control(pagectrl++);
    for ( x = 0; x < 128; x++ )
    {
      HalLcd_HW_Write(0x00);
    }
  }
}

/**************************************************************************************************
 * @fn      HalLcd_HW_ClearAllSpecChars
 *
 * @brief   Clear all special chars
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
void HalLcd_HW_ClearAllSpecChars1(void)
{

}


/**************************************************************************************************
 * @fn      HalLcd_HW_SetCursor
 *
 * @brief   Move cursor to (line, col)
 *
 * @param   uint8 line - line number that the char will be displayed
 *          uint8 col - colum where the char will be displayed
 *
 * @return  None
 **************************************************************************************************/
void HalLcd_HW_SetCursor(uint8 line, uint8 col)
{
  line &= 0x07;
  col &= 0x7f;
    
  uint8 x = line;
  uint8 y0 = col & 0x0f;
  
  uint8 y1 = (col >> 4) & 0x0f;
  
  HalLcd_HW_Control(0x00|y0);    /*set lower column address*/
  HalLcd_HW_Control(0x10|y1);    /*set higher column address*/
  
  HalLcd_HW_Control(0xB0|x);
}

/**************************************************************************************************
 * @fn      HalLcd_HW_Write8_8
 *
 * @brief   Write a char ( 8*8 ) to OLED
 *
 * @param   char ch - the printable char that will be displayed
 *
 * @return  None
 **************************************************************************************************/
void HalLcd_HW_Write8_8(char ch)
{
  if ( ( ch < 32 ) || ( ch > 126 ) )
  {
    return;
  }
  
  uint8 chardot[8];
  
  uint16 idx = ch - 32; // offset in font table
  uint16 offset = idx * 8;
  
  HalFlashRead(FONT_FLASH_PAGE, offset, chardot, 8);
  
  uint8* pCh = chardot;
  
  
  
  uint8 i = 0;
  for ( ; i < 8; i++ )
    HalLcd_HW_Write( *pCh++ );
    //HalLcd_HW_Write( 0xFF );
  
}



/**************************************************************************************************
 * @fn      HalLcd_HW_Write16_8
 *
 * @brief   Write a char ( 16*8 ) to OLED
 *
 * @param   char ch - the printable char that will be displayed
 *
 * @return  None
 **************************************************************************************************/
void HalLcd_HW_Write16_8(uint8 line, uint8 col, char ch)
{
  if ( ( ch < 32 ) || ( ch > 126 ) )
  {
    return;
  }
  
  uint8 chardot[16];
  
  uint16 idx = ch - 32; // offset in font table  
   
  uint16 offset = 0x300 + idx * 16;
  
  HalFlashRead(FONT_FLASH_PAGE, offset, chardot, 16);
  
  uint8* pCh = chardot;
  
  uint8 y, x;
  y = line * 2;  
  x  = col * 8;
  
  uint8 i = 0;
  
  HalLcd_HW_SetCursor(y, x);   
  for ( i = 0; i < 8; i++ )
    HalLcd_HW_Write( *pCh++ );
  
  HalLcd_HW_SetCursor(y+1, x);
  for ( i = 0; i < 8; i++ )
    HalLcd_HW_Write( *pCh++ );  
  
}

/**************************************************************************************************
 * @fn      HalLcd_HW_WriteChar
 *
 * @brief   Write one char to the display
 *
 * @param   uint8 line - line number that the char will be displayed
 *          uint8 col - colum where the char will be displayed
 *
 * @return  None
 **************************************************************************************************/
void HalLcd_HW_WriteChar(uint8 line, uint8 col, char text)
{
  if ( line > LCD_MAX_LINE_COUNT )
  {
    return;
  }
  
  if (col < LCD_MAX_LINE_LENGTH)
  {
    col *= 8;
    
    //HalLcd_HW_SetCursor(line, col);
    
    HalLcd_HW_Write16_8(line, col, text);
  }
  else
  {
    return;
  }
}

/**************************************************************************************************
 * @fn          halLcdWriteLine
 *
 * @brief       Write one line on display
 *
 * @param       uint8 line - display line
 *              char *pText - text buffer to write
 *
 * @return      none
 **************************************************************************************************/
void HalLcd_HW_WriteLine(uint8 line, const char *pText)
{
  uint8 count;
  uint8 totalLength = (uint8)osal_strlen( (char *)pText );
  
  if ( totalLength > 16 )
    totalLength = 16;
  
  //HalLcd_HW_SetCursor(line, 0);
  
  for ( count = 0; count < totalLength; count ++ )
  {
    //HalLcd_HW_Write8_8(*pText++);
    HalLcd_HW_Write16_8(line, count, *pText++);
  }
  
  

#if 0
  
  /* Write the content first */
  for (count=0; count<totalLength; count++)
  {
    HalLcd_HW_WriteChar(line, count, (*(pText++)));
  }

  /* Write blank spaces to rest of the line */
  for(count=totalLength; count<LCD_MAX_LINE_LENGTH;count++)
  {
    HalLcd_HW_WriteChar(line, count, ' ');
  }
#endif
}

/**************************************************************************************************
 * @fn      HalLcd_HW_WaitUs
 *
 * @brief   wait for x us. @ 32MHz MCU clock it takes 32 "nop"s for 1 us delay.
 *
 * @param   x us. range[0-65536]
 *
 * @return  None
 **************************************************************************************************/
void HalLcd_HW_WaitUs(uint16 microSecs)
{
  while(microSecs--)
  {
    /* 32 NOPs == 1 usecs */
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop");
  }
}
#endif


/**************************************************************************************************
**************************************************************************************************/



