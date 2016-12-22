/**************************************************************************************************
Filename:       hal_eFlash.c
Editor:         Tome @ newbit
Revised:        $Date: 2014-4-20 11:20:02 +0800  $
Revision:       $Revision: 00001 $

Description:    
History:        
Notes:          

**************************************************************************************************/



/**************************************************************************************************
// INCLUDES
**************************************************************************************************/
#include "hal_eflash.h"

#include "hal_mcu.h"


/**************************************************************************************************
// TYPEDEF
**************************************************************************************************/



/**************************************************************************************************
// CONSTANTS
**************************************************************************************************/
/* CS_FLASH is at P1.3 */
#define HAL_CS_FLASH_PORT   P1
#define HAL_CS_FLASH_BIT    BV(3)
#define HAL_CS_FLASH_SEL    P1SEL
#define HAL_CS_FLASH_DIR    P1DIR


// MICRO
#define NOP  asm("NOP")
uint16 loops = 5200; // test value

#define XNV_DELAY_PRPG()    halMcuWaitMs(1)     //Program page
#define XNV_DELAY_WRPG()    halMcuWaitMs(11)    //write page

/* 
 erase time
----------------------
 page       10-20ms
 subsector  80-150ms
 sector     1.5-5s
 bulk       4.5-10s
*/
#define XNV_DELAY_ERPG() halMcuWaitMs(10)
#define XNV_DELAY_ERSS() halMcuWaitMs(80)
#define XNV_DELAY_ERSC() halMcuWaitMs(1500)
#define XNV_DELAY_ERBK() halMcuWaitMs(10000)



// for M25P80
#define X_FLASH_PAGE_SIZE        256            // bytes in a page
#define X_FLASH_PAGES            4096           // total pages in x flash
#define X_FLASH_SECTORS          16             // total sectors in x flash
#define X_FALSH_ADDR_MASK        0X0FFFFF       // the address of the end




/**************************************************************************************************
// LOCAL VERIABLE
**************************************************************************************************/



/**************************************************************************************************
// FUNCTIONS DECLERATION
**************************************************************************************************/
static void xnvSPIWrite(uint8 ch);
void halReadeFlash(uint32 addr, uint8* pbuf, uint16 len);
void halMove2Flash(uint32 addr,uint8* pBuf, uint16 len,uint8 cmd);
void halEraseCommon(uint32 addr, uint8 cmd);


/**************************************************************************************************
// FUNCTIONS
**************************************************************************************************/
/**************************************************************************************************
* @fn      hal_eFlashInit
*
* @brief   Initial IOs connected to X FLASH
*
* @param   void
*
* @return  hal_eFlashInit
**************************************************************************************************/
void hal_eFlashInit(void)
{
  HAL_CS_FLASH_SEL &= ~(HAL_CS_FLASH_BIT);    /* Set pin function to GPIO */
  HAL_CS_FLASH_DIR |= (HAL_CS_FLASH_BIT);    /* Set pin direction to Output */
  
  //XNV_SPI_INIT();
  
}




/**************************************************************************************************
* @fn      halReadeFlash
*
* @brief   Read from the external NV storage vis SPI
*
* @param   uint32 addr,  offset to the external NV
           uint8* pbuf,   pointer to the buffer of copy the bytes read from external
           uint16 len,   number of bytes to read from external NV
*
* @return  
**************************************************************************************************/
void halReadeFlash(uint32 addr, uint8* pbuf, uint16 len)
{
  if (len < 1)
    return;
  
  XNV_SPI_BEGIN();
  
  do
  {
    xnvSPIWrite(XNV_STAT_CMD);
  }while(XNV_SPI_RX() & XNV_STAT_WIP);//;    // wait if xnv is busing.
  XNV_SPI_END();
  asm("NOP");asm("NOP");
  
  XNV_SPI_BEGIN();
  xnvSPIWrite(XNV_READ_CMD);
  xnvSPIWrite(BREAK_UINT32(addr,2));
  xnvSPIWrite(BREAK_UINT32(addr,1));
  xnvSPIWrite(BREAK_UINT32(addr,0));
  xnvSPIWrite(0);
  
  while(len--)
  {
    xnvSPIWrite(0);
    *pbuf++ = XNV_SPI_RX();
  };
  
  XNV_SPI_END();  
}


/**************************************************************************************************
* @fn      halWriteeFlash
*
* @brief   write bytes to external flash via SPI
*
* @param   uint32 addr, uint* pBuf, uint16 len
*
* @return  void
**************************************************************************************************/
void halWriteeFlash(uint32 addr, uint8* pBuf, uint16 len)
{
  halMove2Flash(addr, pBuf, len, XNV_WRPG_CMD);
}




/**************************************************************************************************
* @fn      halProgrameFlash
*
* @brief   write bytes to external flash vis SPI, only bits 1 are changed to 0 if necessary
*
* @param   uint32 addr,uint8* pBuf, uint16 len
*
* @return  void
**************************************************************************************************/
void halProgrameFlash(uint32 addr,uint8* pBuf, uint16 len)
{
  halMove2Flash(addr, pBuf, len, XNV_PRPG_CMD);
}


/**************************************************************************************************
* @fn      halEraseCommon
*
* @brief   send a erase command to X flash via SPI
*
* @param   uint32 addr, uint8 cmd
*
* @return  void
**************************************************************************************************/
void halEraseCommon(uint32 addr, uint8 cmd)
{
  XNV_SPI_BEGIN();
  uint8 sta;
  do
  {
    xnvSPIWrite(XNV_STAT_CMD);
    sta = XNV_SPI_RX();
  }while( sta & XNV_STAT_WIP );//;    // wait if xnv is busing.
  XNV_SPI_END();
  
  
  XNV_SPI_BEGIN();
  xnvSPIWrite(XNV_WREN_CMD);
  XNV_SPI_END();
  NOP;NOP;
  
  XNV_SPI_BEGIN();
  xnvSPIWrite(cmd);
  xnvSPIWrite(BREAK_UINT32(addr,2));
  xnvSPIWrite(BREAK_UINT32(addr,1));
  xnvSPIWrite(BREAK_UINT32(addr,0));
  XNV_SPI_END();
  NOP;NOP;  
  
  if (cmd == XNV_ERPG_CMD )
    XNV_DELAY_ERPG();
  else if( cmd == XNV_ERSS_CMD )
    XNV_DELAY_ERSS();
  else if( cmd == XNV_ERSC_CMD )
    XNV_DELAY_ERSC();
}



/**************************************************************************************************
* @fn      halErasePage
*
* @brief   erase the page of the X flash, 
*
* @param   uint16 page
*
* @return  uint8
**************************************************************************************************/
uint8 halErasePage(uint16 page)
{
  // there are 1024 pages in M25PE20, 
  
  if ( page > 1023) // if the page number > 1023 , do nothing
    return INVALIDPARAMETER;
  
  // convert page number to address value
  uint32 addr = (uint32)page << 8; 
  halEraseCommon(addr, XNV_ERPG_CMD );  
  
  return SUCCESS;
}


/**************************************************************************************************
* @fn      halEraseSubsector
*
* @brief   
*
* @param   uint8 sub
*
* @return  void
**************************************************************************************************/
uint8 halEraseSubsector(uint8 sub)
{
if ( sub > 63) // if the subsector number > 63 , do nothing
    return INVALIDPARAMETER;
  
  // convert sub number to address value
  uint32 addr = (uint32)sub << 12; 
  halEraseCommon(addr, XNV_ERSS_CMD );  
  
  return SUCCESS;  
}

/**************************************************************************************************
* @fn      halEraseSector
*
* @brief   
*
* @param   uint8 sec
*
* @return  uint8
**************************************************************************************************/
uint8 halEraseSector(uint8 sec)
{
  if ( sec > 3) // if the sector number > 3 , do nothing
    return INVALIDPARAMETER;
  
  // convert sector number to address value
  uint32 addr = (uint32)sec << 16; 
  halEraseCommon(addr, XNV_ERSC_CMD );  
  
  return SUCCESS;
}


/**************************************************************************************************
* @fn      halMove2Flash
*
* @brief   write X flash depond on cmd
*
* @param   uint32 addr,uint8* pBuf, uint16 len,uint8 cmd
*
* @return  void
**************************************************************************************************/
void halMove2Flash(uint32 addr,uint8* pBuf, uint16 len,uint8 cmd)
{
uint8 cnt;
  
  while ( len )
  {
    XNV_SPI_BEGIN();
    do
    {
      xnvSPIWrite(XNV_STAT_CMD);
    }while(XNV_SPI_RX() & XNV_STAT_WIP);//;    // wait if xnv is busing.
    XNV_SPI_END();
    asm("NOP");asm("NOP");
    
    
    XNV_SPI_BEGIN();
    xnvSPIWrite(XNV_WREN_CMD);
    XNV_SPI_END();
    NOP;NOP;
    
    XNV_SPI_BEGIN();
    xnvSPIWrite(cmd);
    xnvSPIWrite(BREAK_UINT32(addr,2));
    xnvSPIWrite(BREAK_UINT32(addr,1));
    xnvSPIWrite(BREAK_UINT32(addr,0));
    
    cnt = 0 - BREAK_UINT32(addr,0);
    if ( cnt )
    {
      addr += cnt;
    }
    else
    {
      addr += 256;
    }
    
    
    do
    {
      xnvSPIWrite(*pBuf++);
      cnt--;
      len--;
    }while( cnt && len);
    
    XNV_SPI_END();
    
    // program one page will take 0.8ms typically, maximum 3ms
    // wirte one page will take 11ms typically, maximum 23ms
    if (len)
    {
        if ( XNV_PRPG_CMD == cmd )
          XNV_DELAY_PRPG();
        else
          XNV_DELAY_WRPG();
          
    }
  }
}


/**************************************************************************************************
* @fn      halEraseeFlash
*
* @brief   Clear whole X flash 
*
* @param   void
*
* @return  void
**************************************************************************************************/
void halEraseeFlash(void)
{
    do
    {
      xnvSPIWrite(XNV_STAT_CMD);
    }while(XNV_SPI_RX() & XNV_STAT_WIP);//;    // wait if xnv is busing.
    XNV_SPI_END();
    asm("NOP");asm("NOP");
    
    
    XNV_SPI_BEGIN();
    xnvSPIWrite(XNV_WREN_CMD);
    XNV_SPI_END();
    NOP;NOP;
    
    
    
    XNV_SPI_BEGIN();
    xnvSPIWrite(XNV_ERBK_CMD);
    XNV_SPI_END();
    NOP;NOP;
}




/**************************************************************************************************
* @fn      xnvSPIWrite
*
* @brief   spi write sequence for code size saving.
*
* @param   uint8 ch - the byte write to the spi
*
* @return  void
**************************************************************************************************/
static void xnvSPIWrite(uint8 ch)
{
  XNV_SPI_TX(ch);
  XNV_SPI_WAIT_RXRDY();  
}

#define DEBUG_
#ifdef DEBUG_

#include "hal_lcd.h"
#include "ul_strings.h"

/**************************************************************************************************
* @fn      dbgFlashMain
*
* @brief   test X flash drivers or check if it is damaged.
*
* @param   void
*
* @return  void
**************************************************************************************************/
void dbgFlashMain(void)
{
  uint8 str[16] = {0};
  
#define ERASE_WHOLE_X_FLASH
#define WRITE_WHOLE_X_FLASH
#define READ_FIVE_BYTES
  
#ifdef ERASE_WHOLE_X_FLASH
  HalLcdWriteString("Erasing flash", HAL_LCD_LINE_4 );
  // erase whole x flash
  halEraseeFlash();
  XNV_DELAY_ERBK();
#endif
  
#ifdef WRITE_WHOLE_X_FLASH
  HalLcdWriteString("Writing flash", HAL_LCD_LINE_4 );
  uint8 buf[X_FLASH_PAGE_SIZE];
  for(int i = 0; i < X_FLASH_PAGE_SIZE; ++i)
  {
    buf[i] = (uint8)i;
  }
  
  osal_memcpy( str, "WR", 2);
  
  uint32 addr = 0;
  for(uint16 page = 0; page < X_FLASH_PAGES; ++page)
  {
    halProgrameFlash(addr, buf, X_FLASH_PAGE_SIZE);
    addr += X_FLASH_PAGE_SIZE;
    
    // lcd indication
    ul_sprintUint16ToDec(str+2, page);
    str[7] = 0;
    HalLcdWriteString((char*)str, HAL_LCD_LINE_4 );
  }
  
#endif
  
#ifdef READ_FIVE_BYTES
  HalLcdWriteString("Reading flash", HAL_LCD_LINE_4 );
  uint8 readbuf[5] = {0};
  
  uint8 ref[5] = {0xFE, 0XFF, 0,1,2};  
  uint32 readaddr = 0xfe;
  
  //uint8 ref[5] = {0,1,2, 3,4};  
  //uint32 readaddr = 0x0;
  
  osal_memcpy( str, "RD", 2);
  
  uint8 pass = 1;
  
  
  for(uint16 page = 0; page < X_FLASH_PAGES; ++page)
  {
    halReadeFlash(readaddr, readbuf, 5);
    if (ul_memoryCompare(readbuf,ref,5))
    {
      //HalLcdWriteString("ERROR", HAL_LCD_LINE_3 );
      pass = 0;
      break;
    }
    // lcd indication
    ul_sprintUint16ToDec(str+2, page);
    str[7] = 0;
    HalLcdWriteString((char*)str, HAL_LCD_LINE_4 );
    
    //ul_sprintStr(str, readbuf, 5);
    //str[10]=0;
    //HalLcdWriteString((char*)str, HAL_LCD_LINE_2 );
    
    readaddr += X_FLASH_PAGE_SIZE;
    if (readaddr & ~X_FALSH_ADDR_MASK)
      break;
    
    
  }
  
  if ( pass )
    HalLcdWriteString("X FLASH CHECKED!", HAL_LCD_LINE_4 );
  else
    HalLcdWriteString("X FLASH ERROR!", HAL_LCD_LINE_4 );
  
  
  
#endif  //READ_FIVE_BYTES
  
  
}

#endif //debug


/**************************************************************************************************
Copyright 2014 Newbit Studio. All rights reserved.
**************************************************************************************************/
