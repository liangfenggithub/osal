/**************************************************************************************************
Filename:       nbTest.c
Editor:         Tome @ newbit
Revised:        $Date: 2016/11/3 11:20:02 +0800  $
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
#include "nbTest.h"

#include "hal_drivers.h"
#include "hal_key.h"
//#if defined ( LCD_SUPPORTED )
  #include "hal_lcd.h"
//#endif
#include "hal_led.h"
#include "hal_uart.h"
#include "hal_adc.h"
#include "hal_dht11.h"

#include "OSAL_PwrMgr.h"
#include "hal_eflash.h"
#include "ul_strings.h"

// file system
//#include "FS_Lowlevel.h"
#include "FS_Highlevel.h"

   
#include "hal_timer34.h"


/**************************************************************************************************
// TYPEDEF
**************************************************************************************************/



/**************************************************************************************************
// CONSTANTS
**************************************************************************************************/
#define TASK_ID_SELF NBTest_TaskID


#if !defined( BASE_APP_PORT )
#define BASE_APP_PORT  0
#endif

#define UART_TEST_MESSAGE   "This is a UART test massage, baudrate 9600"
#define UART_TEST_MSG_LEN   sizeof(UART_TEST_MESSAGE)


#define POTEN_CHNL  HAL_ADC_CHANNEL_6
#define PHOTO_CHNL  HAL_ADC_CHANNEL_7


#define RESOLUTION_SEL  HAL_ADC_RESOLUTION_12


/**************************************************************************************************
// LOCAL VERIABLE
**************************************************************************************************/
uint8 NBTest_TaskID;    // Task ID for internal task/event processing.


/**************************************************************************************************
// FUNCTIONS DECLERATION
**************************************************************************************************/

UINT16 NBTest_ProcessEvent( uint8 task_id, UINT16 events );
static void NBTest_ProcessOSALMsg(osal_event_hdr_t *pMsg );
static void NBTest_HandleUarts ( uint8 event );
static void NBTest_HandleKeys( uint8 shift, uint8 keys );
static void NBTest_UartCallBack(uint8 port, uint8 event);

void adSensorPhotoSampleEventHanle(void);
void adSensorPotenSampleEventHanle(void);
static void dht11_process(uint8* value);


/**************************************************************************************************
// FUNCTIONS
**************************************************************************************************/

/*********************************************************************
 * @fn      NBTest_Init
 *
 * @brief   This is called during OSAL tasks' initialization.
 *
 * @param   task_id - the Task ID assigned by OSAL.
 *
 * @return  none
 */
void NBTest_Init( uint8 task_id )
{
  NBTest_TaskID = task_id;
  
  halUARTCfg_t uartConfig;

  uartConfig.configured           = TRUE;              // 2x30 don't care - see uart driver.
  uartConfig.baudRate             = HAL_UART_BR_9600;
  uartConfig.flowControl          = FALSE;
  uartConfig.flowControlThreshold = 0; // 2x30 don't care - see uart driver.
  uartConfig.rx.maxBufSize        = 0;  // 2x30 don't care - see uart driver.
  uartConfig.tx.maxBufSize        = 0;  // 2x30 don't care - see uart driver.
  uartConfig.idleTimeout          = 10;   // 2x30 don't care - see uart driver.
  uartConfig.intEnable            = TRUE;              // 2x30 don't care - see uart driver.
  uartConfig.callBackFunc         = NBTest_UartCallBack;
  HalUARTOpen (BASE_APP_PORT, &uartConfig);
  
  
    // 注册按键
  RegisterForKeys( task_id );
    
  
#if defined ( LCD_SUPPORTED )
  HalLcdWriteString( "Newbit KIT 5 ", HAL_LCD_LINE_1 );
  //HalLcdWriteString( "Newbit Test", HAL_LCD_LINE_2 );
  HalLcdWriteString( "newbitstudio.com", HAL_LCD_LINE_3 );
  HalLcdWriteString( "", HAL_LCD_LINE_4 );
#endif    
  
  //AD
  //
  
  
  // 
  hal_eFlashInit();
  
  
  
  //HalLcdDisplayPicture(0);
  //while(1);
  
  
  osal_start_reload_timer(TASK_ID_SELF, NBT_EVT_UART, 1000 ); 
  osal_start_reload_timer(TASK_ID_SELF, NBT_EVT_AD, 100 );
  osal_start_timerEx( TASK_ID_SELF, NBT_EVT_DHT11, 1000);
  
  osal_start_timerEx( TASK_ID_SELF, NBT_EVT_LED, 3000 );
  HalLedBlink(HAL_LED_ALL, 10, 50, 500 );
  
  
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
UINT16 NBTest_ProcessEvent( uint8 task_id, UINT16 events )
{
  (void)task_id;  // Intentionally unreferenced parameter
  if ( events & SYS_EVENT_MSG )
  {
    uint8 *pMsg;
    
    // 将本任务收到的所有消息读出并处理
    while( (pMsg = osal_msg_receive( task_id )) != NULL )
    {       
      NBTest_ProcessOSALMsg( (osal_event_hdr_t *)pMsg );       
      
      // Release the OSAL message
      VOID osal_msg_deallocate( pMsg );
    }
    return ( events ^ SYS_EVENT_MSG );
  }
  
  
  if ( events & NBT_EVT_UART )
  {
    HalUARTWrite( 0,UART_TEST_MESSAGE,UART_TEST_MSG_LEN );
    
    return ( events ^ NBT_EVT_UART );
  }
  
  if ( events & NBT_EVT_LED )
  {
    HalLedSet(HAL_LED_ALL, HAL_LED_MODE_OFF );
    ADCCFG |= 0xC0;
    return ( events ^ NBT_EVT_LED );
  }
  
  if ( events & NBT_EVT_AD )
  {
    adSensorPhotoSampleEventHanle();
    adSensorPotenSampleEventHanle();
    
    return ( events ^ NBT_EVT_AD );
  }
  
  if ( events & NBT_EVT_DHT11 )
  {
    HalOneBusStart( TASK_ID_SELF );
    return ( events ^ NBT_EVT_DHT11 );
  }
  
  
  return ( 0 );  // Discard unknown events.
}



/******************************************************************************
* @fn        NBTest_ProcessOSALMsg
*
* @brief     osal system events handler
*
* @param     
*
* @return    none
*/
static void NBTest_ProcessOSALMsg(osal_event_hdr_t *pMsg )
{
switch ( pMsg->event )
  {
  case UART_EVENT:
    NBTest_HandleUarts ( ((osalUartEvt_t*)pMsg)->event );
    
    break;
    
  case RF_EVENT:
    //NBTest_MessageMSGCB( ((phyEvent_t *)pMsg)->rfEvent);
    break;
    
  case KEY_CHANGE:
    NBTest_HandleKeys(  ((keyChange_t*)pMsg)->state, ((keyChange_t*)pMsg)->keys );
    break;
    
  case ONEBUS_EVENT:
    
    dht11_process( ((oneBusValue_t*)pMsg)->value );
    
    break;    
    
  default:
    break;
  }  
}


#define SampleApp_UART_RX_MAX  64
static void NBTest_HandleUarts ( uint8 event )
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
 * @fn      NBTest_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys  - bit field for key events.
 *
 * @return  none
 */
static void NBTest_HandleKeys( uint8 shift, uint8 keys )
{
  
  if ( shift )
  {
    
  }
  else
  {
    HalLcdWriteString( "                ", HAL_LCD_LINE_4);
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
    
    if (( keys & (HAL_KEY_SW_4 | HAL_KEY_SW_5) ) == (HAL_KEY_SW_4 | HAL_KEY_SW_5))
    {
      dbgFlashMain();
    }
    
    if (( keys & (HAL_KEY_SW_4 | HAL_KEY_SW_0) ) == (HAL_KEY_SW_4 | HAL_KEY_SW_0))
    {
      dbgFlashMain();
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
 * @fn      NBTest_UartCallBack
 *
 * @brief   Send data OTA.
 *
 * @param   port - UART port.
 * @param   event - the UART port event flag.
 *
 * @return  none
 */
static void NBTest_UartCallBack(uint8 port, uint8 event)
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




/******************************************************************************
* @fn        adSensorPhotoSampleEventHanle
*
* @brief     osal system events handler
*
* @param     
*
* @return    none
*/
void adSensorPhotoSampleEventHanle(void)
{
  
  
  uint16 adc = HalAdcRead(PHOTO_CHNL, RESOLUTION_SEL  );
  
  static uint16 last = 0xFFFF;
  
  if ( last != 0xffff )
    if ( ( MAX(last,adc) - MIN(last,adc) ) < 5 )
    {    
      //return;
    }
  last = adc;
  
  char str[15] = "LIGHT:";
  
  // 参考电压是 3.3V , ADC 取样为12位
  
  float vol =  (float)adc * 3.3 / 2048.0;
  
  uint8 idx = 6;
  str[idx++] = (char)vol + '0';
  str[idx++] = '.';
  str[idx++] =  (char)((uint16)(vol * 10 ) % 10) + '0';
  str[idx++] =  (char)((uint16)(vol * 100 ) % 10) + '0';
  str[idx++] = 'V';
  str[idx++] = 0;
  
  uint8 percent = (uint8)(vol * 100.0 / 3.3 );
  
  HalLcdDisplayPercentBar(4,percent);
  //HalLcdWriteString(str, HAL_LCD_LINE_2);
  
}


/******************************************************************************
* @fn        adSensorPotenSampleEventHanle
*
* @brief     osal system events handler
*
* @param     
*
* @return    none
*/
void adSensorPotenSampleEventHanle(void)
{
  uint16 adc = HalAdcRead(POTEN_CHNL, RESOLUTION_SEL  );
  
  static uint16 last = 0xFFFF;
  
  if ( last != 0xffff )
    if ( ( MAX(last,adc) - MIN(last,adc) ) < 5 )
    {    
      //return;
    }  
  last = adc;
  
  char str[15] = "POTEN:";
  
  // 参考电压是 3.3V , ADC 取样为12位
  
  float vol =  (float)adc * 3.3 / 2048.0;
  
  uint8 idx = 6;
  str[idx++] = (char)vol + '0';
  str[idx++] = '.';
  str[idx++] =  (char)((uint16)(vol * 10 ) % 10) + '0';
  str[idx++] =  (char)((uint16)(vol * 100 ) % 10) + '0';
  str[idx++] = 'V';
  str[idx++] = 0;
  
  uint8 percent = (uint8)(vol * 100.0 / 2.76 );
  
  HalLcdDisplayPercentBar(5,percent);
  
  //HalLcdWriteString(str, HAL_LCD_LINE_4);
}




/******************************************************************************
* @fn        dht11_process
*
* @brief     osal system events handler
*
* @param     
*
* @return    none
*/
static void dht11_process(uint8* value)
{
  // hh.hh,  tt.tt , crc
  
  char str[16] = "HU:    TE:";
  
  char* p = str + 3;
  uint8 t = value[0];
  *p++ = t / 10 + '0';
  *p++ = t % 10 + '0';
  *p++ = '%';
  *p++ = ' ';
  
  
  p = str + 10;
  t = value[2];
  *p++ = t / 10 + '0';
  *p++ = t % 10 + '0';
  *p++ = 'C';
  *p++ = ' ';
  
  HalLcdWriteString(str, HAL_LCD_LINE_2);
  
}

/**************************************************************************************************
Copyright 2016 Newbit Studio. All rights reserved.
**************************************************************************************************/



