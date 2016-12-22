/**************************************************************************************************
Filename:       hal_dht11.c
Editor:         Tome @ newbit studio, www.newbitstudio.com
Revised:        $Date: 2016-9-24 11:20:02 +0800  $
Revision:       $Revision: 00001 $

Description:    
History:        
Notes:          

**************************************************************************************************/



/**************************************************************************************************
// INCLUDES
**************************************************************************************************/
#include "hal_mcu.h"
#include "hal_defs.h"
#include "hal_types.h"
#include "hal_board.h"
#include "hal_drivers.h"

#include "osal.h"

#include "hal_dht11.h"


/**************************************************************************************************
// TYPEDEF
**************************************************************************************************/
typedef enum _onebuState_e
{
  ONEBUS_UNINIT,
  ONEBUS_START_LO,
  ONEBUS_START_HI,
  ONEBUS_ACK_LO,
  ONEBUS_ACK_HI,
  ONEBUS_DATA_LO,
  ONEBUS_DATA_HI
}onebuState_e;


/**************************************************************************************************
// CONSTANTS
**************************************************************************************************/
#define HAL_1BUS_RISING_EDGE   0
#define HAL_1BUS_FALLING_EDGE  1



/* CPU port interrupt */
#define HAL_1BUS_CPU_PORT_0_IF P0IF
#define HAL_1BUS_CPU_PORT_1_IF P1IF
#define HAL_1BUS_CPU_PORT_2_IF P2IF


/* DHT11 on P0.1 */

#define HAL_1BUS_PORT   P0
#define HAL_1BUS_BIT    BV(1)
#define HAL_1BUS_SEL    P0SEL
#define HAL_1BUS_DIR    P0DIR   

#define HAL_1BUS_IEN      IEN1  /* CPU interrupt mask register */
#define HAL_1BUS_IENBIT   BV(5) /* Mask bit for all of Port_0 */
#define HAL_1BUS_ICTL     P0IEN /* Port Interrupt Control register */
#define HAL_1BUS_ICTLBIT  BV(1) /* P0IEN - P0.1 enable/disable bit */
#define HAL_1BUS_PXIFG    P0IFG /* Interrupt flag at source */ 

#define HAL_1BUS_EDGEBIT                BV(0)
#define HAL_1BUS_EDGE                   HAL_1BUS_FALLING_EDGE

#define HAL_DHT11_POLARITY              ACTIVE_HIGH          
#define HAL_DHT11_SBIT                  P0_1

#define HAL_1BUS_STATE()                (HAL_DHT11_POLARITY (HAL_DHT11_SBIT))

#define HAL_1BUS_OUTPUT()               st( HAL_1BUS_DIR |= HAL_1BUS_BIT;)
#define HAL_1BUS_INPUT()                st( HAL_1BUS_DIR &= ~HAL_1BUS_BIT;)

#define HAL_1BUS_SET(x)                 \
st (                                    \
    if (x) HAL_DHT11_SBIT=1;            \
    else HAL_DHT11_SBIT=0;              \
    )

#define HAL_DHT11_DATA_BITS             40
#define HAL_DHT11_DATA_BYTES            (HAL_DHT11_DATA_BITS>>3) // bits/8


/******************* TIMER  **********************/

#define TIMERx_CTL              T4CTL
/* BITS IN TxCTL */
#define CTL_BIT_DIV             0xE0
#define CTL_BIT_START           0x10
#define CTL_BIT_OVFIM           0x08
#define CTL_BIT_CLEAR           0x04
#define CTL_BIT_MODE            0x03

/* TIMER CLOCK DIV */
#define TIMER_CLK_DIV_1         (0 << 5)
#define TIMER_CLK_DIV_2         (1 << 5)
#define TIMER_CLK_DIV_4         (2 << 5)
#define TIMER_CLK_DIV_8         (3 << 5)
#define TIMER_CLK_DIV_16         (4 << 5)       // 2mhz
#define TIMER_CLK_DIV_32         (5 << 5)
#define TIMER_CLK_DIV_64         (6 << 5)
#define TIMER_CLK_DIV_128         (7 << 5)

/* TIMER MODE */
#define TIMER_MODE_FREERUN      0       // repeatedly count from 0 to 0xFF
#define TIMER_MODE_DOWN         1       // count from TnCC0 to 0
#define TIMER_MODE_MODULE       2       // repeatedly count from 0 to TnCC0
#define TIMER_MODE_UPDOWN       3       // repeatedly count from 0 to TnCC0 and down to 0


#define TIMERx_START()          st( TIMERx_CTL |= CTL_BIT_START; )
#define TIMERx_STOP()           st( TIMERx_CTL &= ~CTL_BIT_START; )
#define TIMERx_CLEAR()          st( TIMERx_CTL |= CTL_BIT_CLEAR; )

#define TIMERx_COUNT()          T4CNT

/**************************************************************************************************
// LOCAL VERIABLE
**************************************************************************************************/
static uint8 sensor[HAL_DHT11_DATA_BYTES];// 40 bit, for dht11
static uint8 rxBitIdx = 0;    // 0 - 39, for dht11
static onebuState_e onebusState = ONEBUS_UNINIT;
static uint8 registerOnebusTaskId = 0xFF;

/**************************************************************************************************
// FUNCTIONS DECLERATION
**************************************************************************************************/
static uint8 HalOneBus_SendValue( void );


/**************************************************************************************************
// FUNCTIONS
**************************************************************************************************/


/**************************************************************************************************
* @fn      HalOneBusInit
*
* @brief   Initilize One bus
*
* @param   none
*
* @return  None
**************************************************************************************************/
void HalOneBusInit( void )
{
  HAL_1BUS_SEL &= ~(HAL_1BUS_BIT);    /* Set pin function to GPIO */
  HAL_1BUS_DIR &= ~(HAL_1BUS_BIT);    /* Set pin direction to Input */  
  
  /* Rising/Falling edge configuratinn */        
  PICTL &= ~(HAL_1BUS_EDGEBIT);/* Clear the edge bit */
  
  /* For falling edge, the bit must be set. */
#if (HAL_1BUS_EDGE == HAL_1BUS_FALLING_EDGE)
  PICTL |= HAL_1BUS_EDGEBIT;
#endif     
  
  HAL_1BUS_IEN |= HAL_1BUS_IENBIT;
  
  /* Use timer 4 to refine data bits  , 26-28us(0), 70us(1) */
  TIMERx_CTL = 0;
  
  TIMERx_CTL |= TIMER_CLK_DIV_16;
  
  
}



/**************************************************************************************************
* @fn      HalOneBusConfig
*
* @brief   Configure the one bus IO interrupt
*
* @param   interruptEnable - TRUE/FALSE, enable/disable interrupt
*
* @return  None
**************************************************************************************************/
void HalOneBusConfig (bool interruptEnable)
{
  if ( interruptEnable )
  { 

    /* Interrupt configuration:
    * - Clear any pending interrupt
    * - Enable interrupt generation at the port
    * - Enable CPU interrupt    
    */
    
    //HAL_1BUS_CPU_PORT_0_IF = 0;
    HAL_1BUS_PXIFG = ~(HAL_1BUS_BIT);
    HAL_1BUS_ICTL |= HAL_1BUS_ICTLBIT;
    //HAL_1BUS_IEN |= HAL_1BUS_IENBIT;
  }
  else
  {
    HAL_1BUS_ICTL &= ~(HAL_1BUS_ICTLBIT); /* don't generate interrupt */
    // HAL_1BUS_IEN &= ~(HAL_1BUS_IENBIT);   /* Clear interrupt enable bit */
    // KEYs also on P0, keep P0 port IEN 
  }
}

/**************************************************************************************************
* @fn      HalOneBusStart
*
* @brief   Initilize One bus
*
* @param   none
*
* @return  uint8 - the bit has  been added
**************************************************************************************************/
void HalOneBusStart(uint8 taskId)
{
  HAL_1BUS_OUTPUT();
  HAL_1BUS_SET(0);
  
  osal_start_timerEx( Hal_TaskID, HAL_1BUS_EVENT, 40); // 18ms at list  
  onebusState = ONEBUS_START_LO;  
  
  registerOnebusTaskId = taskId;
}


/**************************************************************************************************
* @fn      HalOneBusPoll
*
* @brief   set the bus signal to HIGH, and enable interrupt to receive ack signal
*
* @param   none
*
* @return  None
**************************************************************************************************/
void HalOneBusPoll(void)
{
  if ( onebusState == ONEBUS_START_LO )
  {
    onebusState = ONEBUS_START_HI;
    HalOneBusConfig(TRUE);
    HAL_1BUS_SET(1);
    
  }
  
}


/**************************************************************************************************
* @fn      HalOneBusInit
*
* @brief   Initilize One bus
*
* @param   none
*
* @return  uint8 - the bit has  been added
**************************************************************************************************/
uint8 HalOneBusAddBit( uint8 bit )
{
  uint8 rxByteIdx = rxBitIdx / 8;
  
  if ( rxByteIdx >= HAL_DHT11_DATA_BYTES )
    return 0xFF;
  
  {
    sensor[rxByteIdx] <<= 1;
    
    if (bit)
      sensor[rxByteIdx] |= 0x01;
    else
      ;//
    
    rxBitIdx ++;
  }
  
  return rxBitIdx;
}

/**************************************************************************************************
* @fn      HalOneBusResult
*
* @brief   Read sensor data
*
* @param   none
*
* @return  None
**************************************************************************************************/
void HalOneBusResult(uint8* buf)
{
  osal_memcpy(buf, sensor, HAL_DHT11_DATA_BYTES);
}





/**************************************************************************************************
* @fn      halProcessOneBusInterrupt
*
* @brief   
*
* @param
*
* @return
**************************************************************************************************/
void halProcessOneBusInterrupt (void)
{

  uint8 pxifg = HAL_1BUS_PXIFG;
  uint8 bits;
  HAL_1BUS_PXIFG &= ~HAL_1BUS_BIT;
  
 // if ( onebusState != ONEBUS_START_HI )
  if ( onebusState == ONEBUS_UNINIT )
    return;
  
   if (!( pxifg & HAL_1BUS_BIT ))
  {
    return;
  }
  
  do
  {
  
    if ( onebusState == ONEBUS_START_HI )
    {
      onebusState = ONEBUS_ACK_LO;
      break;
    }
    
    if ( onebusState == ONEBUS_ACK_LO )
    {
      onebusState = ONEBUS_DATA_LO;
      TIMERx_START();
      break;
    }
  
    if ( onebusState == ONEBUS_DATA_LO )
    {
      uint8 time = TIMERx_COUNT();
      uint8 bit = 1;
      TIMERx_CLEAR();
      if ( time <  200 )
      {
        bit = 0;
      }
      bits = HalOneBusAddBit(bit);
      if (bits >= HAL_DHT11_DATA_BITS )
      {
        // finish
        HalOneBusConfig(FALSE);
        TIMERx_STOP();
        
        // NOTIFY
        HalOneBus_SendValue();
        
        rxBitIdx = 0;   
        onebusState = ONEBUS_UNINIT;  
      }
    }
  
  
  }while(0);

  
  
  
}



/*********************************************************************
 * @fn      HalOneBus_SendValue
 *
 * @brief   Send onebus value message to application.
 *
 * @param   void
 *
 * @return  status
 *********************************************************************/
static uint8 HalOneBus_SendValue( void )
{
  oneBusValue_t *msgPtr;

  if ( registerOnebusTaskId != 0xff )
  {
    // Send the address to the task
    msgPtr = (oneBusValue_t *)osal_msg_allocate( sizeof(oneBusValue_t) );
    if ( msgPtr )
    {
      msgPtr->hdr.event = ONEBUS_EVENT;
      osal_memcpy( msgPtr->value, sensor, HAL_DHT11_DATA_BYTES);

      osal_msg_send( registerOnebusTaskId, (uint8 *)msgPtr );
    }
    return ( 0 );
  }
  else
    return ( 1 );
}

/**************************************************************************************************
Copyright 2016 Newbit Studio. All rights reserved.
**************************************************************************************************/



