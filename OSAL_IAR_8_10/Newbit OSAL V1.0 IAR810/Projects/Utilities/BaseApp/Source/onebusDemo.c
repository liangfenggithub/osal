/**************************************************************************************************
Filename:       onebusDemo.c
Editor:         Tome @ newbit
Revised:        $Date: 2016-9-23 11:20:02 +0800  $
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
#include "onebusDemo.h"

#include "hal_drivers.h"
#include "hal_key.h"
#include "hal_adc.h"

//#if defined ( LCD_SUPPORTED )
  #include "hal_lcd.h"
//#endif
#include "hal_led.h"
#include "hal_uart.h"

#include "OSAL_PwrMgr.h"
#include "hal_dht11.h"
#include "ul_strings.h"


/**************************************************************************************************
// TYPEDEF
**************************************************************************************************/



/**************************************************************************************************
// CONSTANTS
**************************************************************************************************/



/**************************************************************************************************
// LOCAL VERIABLE
**************************************************************************************************/
uint8 onebusDemo_TaskID;


/**************************************************************************************************
// FUNCTIONS DECLERATION
**************************************************************************************************/
static void onebusDemo_ProcessOSALMsg(osal_event_hdr_t *pMsg );
void onebusDemo_HandleKeys( uint8 shift, uint8 keys );
static void dht11_process(uint8* value);



/**************************************************************************************************
// FUNCTIONS
**************************************************************************************************/

/*********************************************************************
 * @fn      onebusDemo_Init
 *
 * @brief   This is called during OSAL tasks' initialization.
 *
 * @param   task_id - the Task ID assigned by OSAL.
 *
 * @return  none
 */
void onebusDemo_Init( uint8 task_id )
{

  onebusDemo_TaskID = task_id;
  
 
  
#ifdef LCD_SUPPORTED
  HalLcdWriteString( "ONE BUS Demo", HAL_LCD_LINE_1 );
  
#endif
  
  RegisterForKeys(onebusDemo_TaskID);
  
  //
  
  osal_start_timerEx( task_id, ONEBUS_START_EVT, 1000);
  
  

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
uint16 onebusDemo_ProcessEvent( uint8 task_id, uint16 events )
{
  (void)task_id;  // Intentionally unreferenced parameter
  if ( events & SYS_EVENT_MSG )
  {
    uint8 *pMsg;
    
    // 将本任务收到的所有消息读出并处理
    while( (pMsg = osal_msg_receive( task_id )) != NULL )
    {       
      onebusDemo_ProcessOSALMsg( (osal_event_hdr_t *)pMsg );       
      
      // Release the OSAL message
      VOID osal_msg_deallocate( pMsg );
    }
    return ( events ^ SYS_EVENT_MSG );
  }
  
  if ( events & ONEBUS_START_EVT  )
  {
      HalOneBusStart( onebusDemo_TaskID );
      
      
      osal_start_timerEx( onebusDemo_TaskID, ONEBUS_START_EVT, 1000);
      
      return ( events ^ ONEBUS_START_EVT );
      //osal_start_reload_timer(Base_TaskID, BASEAPP_CLOCK_EVT, 1000 );
  }
  
  return 0;
}



/******************************************************************************
* @fn        onebusDemo_ProcessOSALMsg
*
* @brief     osal system events handler
*
* @param     
*
* @return    none
*/
static void onebusDemo_ProcessOSALMsg(osal_event_hdr_t *pMsg )
{
switch ( pMsg->event )
  {
  case UART_EVENT:
    //onebusDemo_HandleUarts ( ((osalUartEvt_t*)pMsg)->event );
    
    break;
    
  case RF_EVENT:
    //onebusDemo_MessageMSGCB( ((phyEvent_t *)pMsg)->rfEvent);
    break;
    
  case KEY_CHANGE:
    onebusDemo_HandleKeys(  ((keyChange_t*)pMsg)->state, ((keyChange_t*)pMsg)->keys );
    break;
    
  case ONEBUS_EVENT:
    
    dht11_process( ((oneBusValue_t*)pMsg)->value );
    
    break;
    
    
  default:
    break;
  }  
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
  
  char hstr[12] = "HUMI:";
  char tstr[12] = "TEMP:";
  
  char* p = hstr + 5;
  uint8 t = value[0];
  *p++ = t / 10 + '0';
  *p++ = t % 10 + '0';
  *p++ = 0;
  
  
  p = tstr + 5;
  t = value[2];
  *p++ = t / 10 + '0';
  *p++ = t % 10 + '0';
  *p++ = 0;
  
  HalLcdWriteString(hstr, HAL_LCD_LINE_3);
  HalLcdWriteString(tstr, HAL_LCD_LINE_4);
  
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
void onebusDemo_HandleKeys( uint8 shift, uint8 keys )
{
  
  if ( shift )
  {
    
  }
  else
  {
    if ( keys & HAL_KEY_SW_0 )
    {
      //HalLedSet(HAL_LED_ALL, HAL_LED_MODE_TOGGLE);        
      //HalLcdWriteString( "S0", HAL_LCD_LINE_4);
    }
    if ( keys & HAL_KEY_SW_1 )
    {
      //HalLedSet(HAL_LED_1, HAL_LED_MODE_TOGGLE);  
      //HalLcdWriteString( "S1", HAL_LCD_LINE_4);      
    }
    if ( keys & HAL_KEY_SW_2 )
    {
       //HalLedSet(HAL_LED_2 , HAL_LED_MODE_TOGGLE);
       //HalLcdWriteString( "S2", HAL_LCD_LINE_4);
    }
    
    if ( keys & HAL_KEY_SW_3 )
    {
       //HalLcdWriteString( "S3", HAL_LCD_LINE_4);
    }
    if ( keys & HAL_KEY_SW_4 )
    {
       //HalLcdWriteString( "S4", HAL_LCD_LINE_4);
    }
    
    if ( keys & HAL_KEY_SW_5 )
    {
       //HalLcdWriteString( "S5", HAL_LCD_LINE_4);
      HalOneBusStart( onebusDemo_TaskID );
    }
    

    
    if ( keys & ( HAL_KEY_SW_3 | HAL_KEY_SW_4 | HAL_KEY_SW_5 ) )
    {
      //HalLedSet(HAL_LED_ALL, HAL_LED_MODE_TOGGLE);
    }
  }
  
#ifdef POWER_SAVING
  osal_pwrmgr_task_state(Base_TaskID, PWRMGR_HOLD );
#endif
}


/**************************************************************************************************
Copyright 2016 Newbit Studio. All rights reserved.
**************************************************************************************************/



