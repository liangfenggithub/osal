



/*********************************************************************
* INCLUDES
*/


#include "OnBoard.h"
#include "OSAL_Tasks.h"
#include "OSAL_Clock.h"
#include "adSensorDemo.h"

#include "hal_drivers.h"
#include "hal_key.h"
#include "hal_adc.h"

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


/*********************************************************************
* MACROS
*/

// poten @ p06
// photo @ p07
#define POTEN_CHNL  HAL_ADC_CHANNEL_6
#define PHOTO_CHNL  HAL_ADC_CHANNEL_7


#define RESOLUTION_SEL  HAL_ADC_RESOLUTION_12


/*********************************************************************
* TYPEDEFS
*/
//typedef struct

/*********************************************************************
* GLOBAL VARIABLES
*/

uint8 adSensorDemo_TaskID;    // Task ID for internal task/event processing.



/*********************************************************************
* EXTERNAL VARIABLES
*/

/*********************************************************************
* EXTERNAL FUNCTIONS
*/

/*********************************************************************
* LOCAL VARIABLES
*/


/*********************************************************************
* LOCAL FUNCTIONS
*/

//static void adSensorDemo_ProcessOSALMsg(osal_event_hdr_t *pMsg );
void adSensorDemo_PhotoSampleEventHanle(void);
void adSensorDemo_PotenSampleEventHanle(void);


/*********************************************************************
* @fn      adSensorDemo_Init
*
* @brief   This is called during OSAL tasks' initialization.
*
* @param   task_id - the Task ID assigned by OSAL.
*
* @return  none
*/
void adSensorDemo_Init( uint8 task_id )
{
  
  adSensorDemo_TaskID = task_id;
  
  ADCCFG |= 0xC0;
  //ADCCFG |= PHOTO_CHNL;
  
#ifdef LCD_SUPPORT
  HalLcdWriteString( "AD Sensor Demo", HAL_LCD_LINE_1 );
  
#endif
  
  //HalLcdDisplayPicture(0);
  
  
  osal_start_reload_timer(adSensorDemo_TaskID, POTEN_SAMPLE_EVT, 100 );
  osal_start_reload_timer(adSensorDemo_TaskID, PHOTO_SAMPLE_EVT, 100 );

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
uint16 adSensorDemo_ProcessEvent( uint8 task_id, uint16 events )
{
  (void)task_id;  // Intentionally unreferenced parameter
  if ( events & SYS_EVENT_MSG )
  {
    uint8 *pMsg;
    
    // 将本任务收到的所有消息读出并处理
    while( (pMsg = osal_msg_receive( task_id )) != NULL )
    {       
      //BaseApp_ProcessOSALMsg( (osal_event_hdr_t *)pMsg );       
      
      // Release the OSAL message
      VOID osal_msg_deallocate( pMsg );
    }
    return ( events ^ SYS_EVENT_MSG );
  }
  
  
  if ( events & POTEN_SAMPLE_EVT )
  {
    adSensorDemo_PotenSampleEventHanle();
    return ( events ^ POTEN_SAMPLE_EVT );
  }
  
  
  if ( events & PHOTO_SAMPLE_EVT )
  {
    adSensorDemo_PhotoSampleEventHanle();
    return ( events ^ PHOTO_SAMPLE_EVT );
  }
  
  
  return 0;
}


/******************************************************************************
* @fn        adSensorDemo_PhotoSampleEventHanle
*
* @brief     osal system events handler
*
* @param     
*
* @return    none
*/
void adSensorDemo_PhotoSampleEventHanle(void)
{
  
  
  uint16 adc = HalAdcRead(PHOTO_CHNL, RESOLUTION_SEL  );
  
  static uint16 last = 0xFFFF;
  
  if ( last != 0xffff )
    if ( ( MAX(last,adc) - MIN(last,adc) ) < 5 )
    {    
      return;
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
  HalLcdWriteString(str, HAL_LCD_LINE_2);
  
}


/******************************************************************************
* @fn        adSensorDemo_PotenSampleEventHanle
*
* @brief     osal system events handler
*
* @param     
*
* @return    none
*/
void adSensorDemo_PotenSampleEventHanle(void)
{
  uint16 adc = HalAdcRead(POTEN_CHNL, RESOLUTION_SEL  );
  
  static uint16 last = 0xFFFF;
  
  if ( last != 0xffff )
    if ( ( MAX(last,adc) - MIN(last,adc) ) < 5 )
    {    
      return;
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
  
  HalLcdWriteString(str, HAL_LCD_LINE_4);
}


#if 0
/******************************************************************************
* @fn        adSensorDemo_ProcessOSALMsg
*
* @brief     osal system events handler
*
* @param     
*
* @return    none
*/
static void adSensorDemo_ProcessOSALMsg(osal_event_hdr_t *pMsg )
{
  switch ( pMsg->event )
  {
    
    
    //case KEY_CHANGE:
    //  adSensorDemo_HandleKeys(  ((keyChange_t*)pMsg)->state, ((keyChange_t*)pMsg)->keys );
    break;
    
    
  default:
    break;
  }  
}
#endif
