/**************************************************************************************************
  Filename:       BaseApp.c
  Revised:        $Date: 2009-03-29 10:51:47 -0700 (Sun, 29 Mar 2009) $
  Revision:       $Revision: 19585 $

  Description -   Serial Transfer Application (no Profile).


  Copyright 2004-2009 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED 揂S IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, 
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE, 
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com. 
**************************************************************************************************/

/*********************************************************************
  This sample application is basically a cable replacement
  and it should be customized for your application. A PC
  (or other device) sends data via the serial port to this
  application's device.  This device transmits the message
  to another device with the same application running. The
  other device receives the over-the-air message and sends
  it to a PC (or other device) connected to its serial port.
				
  This application doesn't have a profile, so it handles everything directly.

  Key control:
    SW1:
    SW2:  
    SW3:
    SW4:  
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */


#include "OnBoard.h"
#include "OSAL_Tasks.h"
#include "OSAL_Clock.h"
#include "BaseApp.h"

#include "hal_drivers.h"
#include "hal_key.h"
//#if defined ( LCD_SUPPORTED )
  #include "hal_lcd.h"
//#endif
#include "hal_led.h"
#include "hal_uart.h"

#include "OSAL_PwrMgr.h"
#include "hal_eflash.h"
#include "ul_strings.h"

// file system
//#include "FS_Lowlevel.h"
#include "FS_Highlevel.h"

   
#include "hal_timer34.h"

/*********************************************************************
 * MACROS
 */
#define HALFBYTE_TO_ASCII(a, b)     \
  do {                              \
  a = b & 0x0f;                     \
  a += (a < 10) ? '0' : ('A'-10);         \
  }while(0)

#define NUM_TO_ASCII(a, n) \
  do {                              \
  a = n % 10;                     \
  a += '0';       \
  }while(0)

/*********************************************************************
 * CONSTANTS
 */

#if !defined( BASE_APP_PORT )
#define BASE_APP_PORT  0
#endif

#if !defined( SERIAL_APP_BAUD )
#define SERIAL_APP_BAUD  HAL_UART_BR_9600
//#define SERIAL_APP_BAUD  HAL_UART_BR_115200
#endif

// When the Rx buf space is less than this threshold, invoke the Rx callback.
#if !defined( SERIAL_APP_THRESH )
#define SERIAL_APP_THRESH  64
#endif

#if !defined( SERIAL_APP_RX_SZ )
#define SERIAL_APP_RX_SZ  128
#endif

#if !defined( SERIAL_APP_TX_SZ )
#define SERIAL_APP_TX_SZ  128
#endif

// Millisecs of idle time after a byte is received before invoking Rx callback.
#if !defined( SERIAL_APP_IDLE )
#define SERIAL_APP_IDLE  10
#endif

// Loopback Rx bytes to Tx for throughput testing.
#if !defined( SERIAL_APP_LOOPBACK )
#define SERIAL_APP_LOOPBACK  FALSE
#endif

// This is the max byte count per OTA message.
#if !defined( SERIAL_APP_TX_MAX )
#define SERIAL_APP_TX_MAX  80
#endif

/*********************************************************************
 * TYPEDEFS
 */
//typedef struct

/*********************************************************************
 * GLOBAL VARIABLES
 */

uint8 Base_TaskID;    // Task ID for internal task/event processing.

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

static uint8 Time[4] = {0,0,0,0};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void BaseApp_HandleKeys( uint8 shift, uint8 keys );
static void BaseApp_HandleUarts ( uint8 event );
static void BaseApp_UartCallBack(uint8 port, uint8 event);
static void BaseApp_ClockDemo( void );
static void BaseApp_ProcessOSALMsg(osal_event_hdr_t *pMsg );

/*********************************************************************
 * @fn      BaseApp_Init
 *
 * @brief   This is called during OSAL tasks' initialization.
 *
 * @param   task_id - the Task ID assigned by OSAL.
 *
 * @return  none
 */
void BaseApp_Init( uint8 task_id )
{
  halUARTCfg_t uartConfig;

  Base_TaskID = task_id;

  // 注册按键
  RegisterForKeys( task_id );

  uartConfig.configured           = TRUE;              // 2x30 don't care - see uart driver.
  uartConfig.baudRate             = SERIAL_APP_BAUD;
  uartConfig.flowControl          = FALSE;
  uartConfig.flowControlThreshold = SERIAL_APP_THRESH; // 2x30 don't care - see uart driver.
  uartConfig.rx.maxBufSize        = SERIAL_APP_RX_SZ;  // 2x30 don't care - see uart driver.
  uartConfig.tx.maxBufSize        = SERIAL_APP_TX_SZ;  // 2x30 don't care - see uart driver.
  uartConfig.idleTimeout          = SERIAL_APP_IDLE;   // 2x30 don't care - see uart driver.
  uartConfig.intEnable            = TRUE;              // 2x30 don't care - see uart driver.
  uartConfig.callBackFunc         = BaseApp_UartCallBack;
  HalUARTOpen (BASE_APP_PORT, &uartConfig);
  
  
  //osal_start_reload_timer(Base_TaskID, BASEAPP_FLASH_EVT, 1000 );
  //osal_start_reload_timer(Base_TaskID, BASEAPP_CLOCK_EVT, 1000 );

#if defined ( LCD_SUPPORTED )
  //HalLcdWriteString( "Newbit Studio", HAL_LCD_LINE_1 );
  HalLcdWriteString( "BaseApp", HAL_LCD_LINE_2 );
  HalLcdWriteString( "newbitstudio.com", HAL_LCD_LINE_3 );
  HalLcdWriteString( "", HAL_LCD_LINE_4 );
#endif  
  
  HalUARTWrite(0,"abcd",4);

  //FSL_Inital();
  
  
  

  //FSL_Dbg_Write();
  
  
  
  //dbgFlashMain();
  
  
  // test timer 3 down mode

#if 0  
  HalTimer34Init(3);
  
  /*
  //HAL_TIMER_MODE_SET(3,HAL_TIMER_MODE_DOWN);
  //T3CC0 = 200;
  */
  
  //TIMER34_CH1_CAPTURE(3,HAL_TIMER_CAPTURE_EDGE_FALLING);
  
  P1SEL |= BV(3);
  T3CCTL0 |= HAL_TIMER_CAPTURE_EDGE_FALLING;
  
  
  TIMER_CHANNEL_INTERRUPT_ENABLE(3,0,TRUE);
  TIMER34_ENABLE_OVERFLOW_INT(3,FALSE);
  T3IE = 1;
  TIMER34_RUN(3);
#endif
}

/*********************************************************************
 * @fn      SerialApp_ProcessEvent
 *
 * @brief   Generic Application Task event processor.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events   - Bit map of events to process.
 *
 * @return  Event flags of all unprocessed events.
 */
UINT16 BaseApp_ProcessEvent( uint8 task_id, UINT16 events )
{
  (void)task_id;  // Intentionally unreferenced parameter
  if ( events & SYS_EVENT_MSG )
  {
    uint8 *pMsg;
    
    // 将本任务收到的所有消息读出并处理
    while( (pMsg = osal_msg_receive( task_id )) != NULL )
    {       
      BaseApp_ProcessOSALMsg( (osal_event_hdr_t *)pMsg );       
      
      // Release the OSAL message
      VOID osal_msg_deallocate( pMsg );
    }
    return ( events ^ SYS_EVENT_MSG );
  }
  
  // 闪灯实验
  if ( events & BASEAPP_FLASH_EVT )
  {
    //HalLedSet(HAL_LED_2 | HAL_LED_1, HAL_LED_MODE_TOGGLE);
    return ( events ^ BASEAPP_FLASH_EVT );
  }
  
  
  // 时钟实验
  if ( events & BASEAPP_CLOCK_EVT )
  {
    //BaseApp_ClockDemo();      
    return ( events ^ BASEAPP_CLOCK_EVT );
  }

  return ( 0 );  // Discard unknown events.
}


/******************************************************************************
* @fn        BaseApp_ProcessOSALMsg
*
* @brief     osal system events handler
*
* @param     
*
* @return    none
*/
static void BaseApp_ProcessOSALMsg(osal_event_hdr_t *pMsg )
{
switch ( pMsg->event )
  {
  case UART_EVENT:
    BaseApp_HandleUarts ( ((osalUartEvt_t*)pMsg)->event );
    
    break;
    
  case RF_EVENT:
    //BaseApp_MessageMSGCB( ((phyEvent_t *)pMsg)->rfEvent);
    break;
    
  case KEY_CHANGE:
    BaseApp_HandleKeys(  ((keyChange_t*)pMsg)->state, ((keyChange_t*)pMsg)->keys );
    break;
    
    
  default:
    break;
  }  
}


#define SampleApp_UART_RX_MAX  64
static void BaseApp_HandleUarts ( uint8 event )
{
  uint8 uRxBuf[SampleApp_UART_RX_MAX+1];
  uint8 len = SampleApp_UART_RX_MAX;
  switch (event)
  {
  case HAL_UART_RX_FULL:
  case HAL_UART_RX_ABOUT_FULL:
  case HAL_UART_RX_TIMEOUT:
    // Read bytes in uart rx fifo
    //len = Hal_UART_RxBufLen(BASE_APP_PORT);
    //if ( len > 20 )
    //{
      len = SampleApp_UART_RX_MAX;
      len = HalUARTRead(BASE_APP_PORT, uRxBuf, len);
      HalUARTWrite(BASE_APP_PORT, uRxBuf, len);
    //}
    break;
    
  case HAL_UART_TX_EMPTY:
#ifdef POWER_SAVING
  osal_pwrmgr_task_state(Base_TaskID, PWRMGR_CONSERVE );
#endif    
    break;
    
  default :
    break;
  }  
}


/*********************************************************************
 * @fn      BaseApp_ClockDemo
 *
 * @brief   Show a clock on lcd
 *
 * @param   void
 *
 * @return  none
 */
static void BaseApp_ClockDemo( void )
{
  Time[3] ++;
  if (Time[3] > 59)
  {
    Time[3] = 0;
    Time[2]++;
    if (Time[2] > 59)
    {
      Time[2] = 0;
      Time[1]++;
      
      if (Time[1] > 23)
        Time[1] = 0;
    }
  }
  
  
# ifdef LCD_SUPPORTED
  // Print on LCD
  char tmpstr[9] = {0};
  uint8 idx = 0;
  NUM_TO_ASCII(tmpstr[idx], Time[1]/10);    //hour
  idx++;
  NUM_TO_ASCII(tmpstr[idx], Time[1]);
  idx++;
  tmpstr[idx] = ':';
  idx++;
  NUM_TO_ASCII(tmpstr[idx], Time[2]/10);    // minute
  idx++;
  NUM_TO_ASCII(tmpstr[idx], Time[2]);
  idx++;
  tmpstr[idx] = ':';
  idx++;
  NUM_TO_ASCII(tmpstr[idx], Time[3]/10);    //second
  idx++;
  NUM_TO_ASCII(tmpstr[idx], Time[3]);
  idx++;
  tmpstr[idx] = 0;  //end char
  
  HalLcdWriteString(tmpstr, HAL_LCD_LINE_3 );
  
#endif
}




/*********************************************************************
 * @fn      BaseApp_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys  - bit field for key events.
 *
 * @return  none
 */
void BaseApp_HandleKeys( uint8 shift, uint8 keys )
{
  
  if ( shift )
  {
    
  }
  else
  {
    if ( keys & HAL_KEY_SW_0 )
    {
      HalLedSet(HAL_LED_ALL, HAL_LED_MODE_TOGGLE);        
      HalLcdWriteString( "S0", HAL_LCD_LINE_4);
    }
    if ( keys & HAL_KEY_SW_1 )
    {
      HalLedSet(HAL_LED_1, HAL_LED_MODE_TOGGLE);  
      HalLcdWriteString( "S1", HAL_LCD_LINE_4);      
    }
    if ( keys & HAL_KEY_SW_2 )
    {
       HalLedSet(HAL_LED_2 , HAL_LED_MODE_TOGGLE);
       HalLcdWriteString( "S2", HAL_LCD_LINE_4);
    }
    
    if ( keys & HAL_KEY_SW_3 )
    {
       HalLcdWriteString( "S3", HAL_LCD_LINE_4);
    }
    if ( keys & HAL_KEY_SW_4 )
    {
       HalLcdWriteString( "S4", HAL_LCD_LINE_4);
    }
    
    if ( keys & HAL_KEY_SW_5 )
    {
       HalLcdWriteString( "S5", HAL_LCD_LINE_4);
    }
    

    
    if ( keys & ( HAL_KEY_SW_3 | HAL_KEY_SW_4 | HAL_KEY_SW_5 ) )
    {
      HalLedSet(HAL_LED_ALL, HAL_LED_MODE_TOGGLE);
    }
  }
  
#ifdef POWER_SAVING
  osal_pwrmgr_task_state(Base_TaskID, PWRMGR_HOLD );
#endif
}


/*********************************************************************
 * @fn      BaseApp_UartCallBack
 *
 * @brief   Send data OTA.
 *
 * @param   port - UART port.
 * @param   event - the UART port event flag.
 *
 * @return  none
 */
static void BaseApp_UartCallBack(uint8 port, uint8 event)
{
  osalUartEvt_t *uartEvtPtr; 
  uartEvtPtr = (osalUartEvt_t *)osal_msg_allocate( sizeof(osalUartEvt_t) );
  
  if (uartEvtPtr != NULL)
  {
    uartEvtPtr->hdr.event = UART_EVENT;
    uartEvtPtr->port   = port;
    uartEvtPtr->event  = event;
    
    osal_msg_send( Base_TaskID, (uint8 *)uartEvtPtr );
  }
}

/*********************************************************************
*********************************************************************/
