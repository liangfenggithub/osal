
#ifndef AD_SENSOR_DEMO
#define AD_SENSOR_DEMO


/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"



// These constants are only for example and should be changed to the
// device's needs

#define POTEN_SAMPLE_EVT             0x0001
#define PHOTO_SAMPLE_EVT             0x0002








void adSensorDemo_Init( uint8 task_id );
uint16 adSensorDemo_ProcessEvent( uint8 task_id, uint16 events );






#endif // AD_SENSOR_DEMO