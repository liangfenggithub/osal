/**************************************************************************************************
Filename:       Arduino.c
Editor:         Tome @ newbit
Revised:        $Date: 2016/11/28 11:20:02 +0800  $
Revision:       $Revision: 00001 $

Description:    
History:        
Notes:          

**************************************************************************************************/



/**************************************************************************************************
// INCLUDES
**************************************************************************************************/

#include "OnBoard.h"
#include "OSAL_Tasks.h"
#include "OSAL_Clock.h"
#include "hal_drivers.h"

#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"

#include "OSAL_PwrMgr.h"
#include "ul_strings.h"

#include "arduino.h"


/**************************************************************************************************
// TYPEDEF
**************************************************************************************************/



/**************************************************************************************************
// CONSTANTS
**************************************************************************************************/
#define TASK_ID_SELF Arduino_TaskID

#if !defined( BASE_APP_PORT )
#define BASE_APP_PORT  0
#endif


/**************************************************************************************************
// LOCAL VERIABLE
**************************************************************************************************/
uint8 Arduino_TaskID;    // Task ID for internal task/event processing.


uint8 UartSendCounter = 0;
uint8 UartSending = 0;


/**************************************************************************************************
// FUNCTIONS DECLERATION
**************************************************************************************************/
static void Arduino_UartCallBack(uint8 port, uint8 event);
static void Arduino_HandleUarts ( uint8 event );
static void Arduino_ProcessOSALMsg(osal_event_hdr_t *pMsg );
static void Arduino_HandleKeys( uint8 shift, uint8 keys );



/**************************************************************************************************
// FUNCTIONS
**************************************************************************************************/
void Arduino_Init( uint8 task_id )
{
  Arduino_TaskID = task_id;
  
  halUARTCfg_t uartConfig;
  uartConfig.configured           = TRUE;              // 2x30 don't care - see uart driver.
  uartConfig.baudRate             = HAL_UART_BR_9600;
  uartConfig.flowControl          = FALSE;
  uartConfig.flowControlThreshold = 0; // 2x30 don't care - see uart driver.
  uartConfig.rx.maxBufSize        = 0;  // 2x30 don't care - see uart driver.
  uartConfig.tx.maxBufSize        = 0;  // 2x30 don't care - see uart driver.
  uartConfig.idleTimeout          = 10;   // 2x30 don't care - see uart driver.
  uartConfig.intEnable            = TRUE;              // 2x30 don't care - see uart driver.
  uartConfig.callBackFunc         = Arduino_UartCallBack;
  HalUARTOpen (BASE_APP_PORT, &uartConfig);
  
      // 注册按键
  RegisterForKeys( task_id );
  
  //osal_start_reload_timer(TASK_ID_SELF, ARD_EVT_UART, 1000 ); 
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
UINT16 Arduino_ProcessEvent( uint8 task_id, UINT16 events )
{
  (void)task_id;  // Intentionally unreferenced parameter
  if ( events & SYS_EVENT_MSG )
  {
    uint8 *pMsg;
    
    // 将本任务收到的所有消息读出并处理
    while( (pMsg = osal_msg_receive( task_id )) != NULL )
    {       
      Arduino_ProcessOSALMsg( (osal_event_hdr_t *)pMsg );       
      
      // Release the OSAL message
      VOID osal_msg_deallocate( pMsg );
    }
    return ( events ^ SYS_EVENT_MSG );
  }
  
  
  if ( events & ARD_EVT_UART )
  {
    HalUARTWrite( BASE_APP_PORT, &UartSendCounter, 1);
    UartSendCounter++;
    
    if ( UartSendCounter > 9 )
      UartSendCounter = 0;
    
    return ( events ^ ARD_EVT_UART );
  }
    
  return ( 0 );  // Discard unknown events.
}




/******************************************************************************
* @fn        Arduino_ProcessOSALMsg
*
* @brief     osal system events handler
*
* @param     
*
* @return    none
*/
static void Arduino_ProcessOSALMsg(osal_event_hdr_t *pMsg )
{
switch ( pMsg->event )
  {
  case UART_EVENT:
    Arduino_HandleUarts ( ((osalUartEvt_t*)pMsg)->event );
    
    break;
    
  case KEY_CHANGE:
    Arduino_HandleKeys(  ((keyChange_t*)pMsg)->state, ((keyChange_t*)pMsg)->keys );
    break;
    
  default:
    break;
  }  
}



/*********************************************************************
 * @fn      Arduino_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys  - bit field for key events.
 *
 * @return  none
 */
static void Arduino_HandleKeys( uint8 shift, uint8 keys )
{
  
  if ( shift )
  {
    
  }
  else
  {
    if ( keys & HAL_KEY_SW_0 )
    {
      if ( UartSending == 0 )
      {
        osal_start_reload_timer(TASK_ID_SELF, ARD_EVT_UART, 1000 ); 
      }
      else
      {
        osal_stop_timerEx(TASK_ID_SELF, ARD_EVT_UART);
      }
      UartSending = !UartSending;
    }
  }
}

#define SampleApp_UART_RX_MAX  64
static void Arduino_HandleUarts ( uint8 event )
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
      
      if ( len == 1)
      {
        UartSendCounter = uRxBuf[0] % 10;
      }
      
      
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
 * @fn      Arduino_UartCallBack
 *
 * @brief   Send data OTA.
 *
 * @param   port - UART port.
 * @param   event - the UART port event flag.
 *
 * @return  none
 */
static void Arduino_UartCallBack(uint8 port, uint8 event)
{
  osalUartEvt_t *uartEvtPtr; 
  uartEvtPtr = (osalUartEvt_t *)osal_msg_allocate( sizeof(osalUartEvt_t) );
  
  if (uartEvtPtr != NULL)
  {
    uartEvtPtr->hdr.event = UART_EVENT;
    uartEvtPtr->port   = port;
    uartEvtPtr->event  = event;
    
    osal_msg_send( TASK_ID_SELF, (uint8 *)uartEvtPtr );
  }
}


/**************************************************************************************************
Copyright 2016 Newbit Studio. All rights reserved.
**************************************************************************************************/



