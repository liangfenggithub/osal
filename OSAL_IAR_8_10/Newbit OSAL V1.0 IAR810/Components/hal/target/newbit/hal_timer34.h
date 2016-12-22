/**************************************************************************************************
Filename:       hal_timer34.h
Editor:         Tome @ newbit / www.newbitstudio.com
Revised:        $Date: 2016/10/16 11:20:02 +0800  $
Revision:       $Revision: 00001 $

Description:    
History:        
Notes:          



**************************************************************************************************/



/**************************************************************************************************
// INCLUDES
**************************************************************************************************/
#include "hal_defs.h"


#ifndef HAL_TIMER34_H
#define HAL_TIMER34_H


/**************************************************************************************************
// TYPEDEF
**************************************************************************************************/



/**************************************************************************************************
// CONSTANTS
**************************************************************************************************/



/**************************************************************************************************
// GLOABAL VERIABLE
**************************************************************************************************/



/**************************************************************************************************
// MICROS
**************************************************************************************************/
// #define MAX((a),(b)) (a)>(b)?(a):(b)




// Macro for initialising timer 3 or 4
#define HAL_TIMER34_INIT_REG(timer)     \
st(                                 \
  T##timer##CTL = 0x08;             \
    T##timer##CCTL0 = 0x40;           \
      T##timer##CC0 = 0;                \
        T##timer##CCTL1 = 0x40;           \
          T##timer##CC1 = 0;                \
            )


#define HAL_TIMER34_INIT(t)          \
st(                                \
  if ((t) == 4)                    \
    {  HAL_TIMER34_INIT_REG(4);}     \
  else                             \
    {  HAL_TIMER34_INIT_REG(3);}     \
  )


//Macro for enabling overflow interrupt
#define TIMER34_ENABLE_OVERFLOW_INT(timer,val) \
(T##timer##CTL =  (val) ? T##timer##CTL | 0x08 : T##timer##CTL & ~0x08)

//Macor for clearing counter for timer3,4
#define TIMER34_CLEAR(timer) ( T##timer##CTL |= 0X04)


// Macro for configuring channel 1 of timer 3 or 4 for PWM mode.
#define TIMER34_PWM_CONFIG(timer)                 \
do{                                            \
  T##timer##CCTL1 = 0x24;                     \
    if(timer == 3){                             \
      if(PERCFG & 0x20) {                      \
        IO_FUNC_PORT_PIN(1,7,IO_FUNC_PERIPH); \
      }                                        \
      else {                                   \
        IO_FUNC_PORT_PIN(1,4,IO_FUNC_PERIPH); \
      }                                        \
    }                                           \
    else {                                      \
      if(PERCFG & 0x10) {                      \
        IO_FUNC_PORT_PIN(2,3,IO_FUNC_PERIPH);\
      }                                        \
      else {                                   \
        IO_FUNC_PORT_PIN(1,1,IO_FUNC_PERIPH); \
      }                                        \
    }                                           \
} while(0)

// Macro for setting pulse length of the timer in PWM mode
#define TIMER34_SET_PWM_PULSE_LENGTH(timer, value) \
do {                                            \
  T##timer##CC1 = (BYTE)value;                 \
} while (0)


#define HAL_TIMER_CAPTURE_EDGE_NONE             0
#define HAL_TIMER_CAPTURE_EDGE_RISING           1
#define HAL_TIMER_CAPTURE_EDGE_FALLING          2
#define HAL_TIMER_CAPTURE_EDGE_BOTH             3

// Macro for setting timer 3 or 4 as a capture timer
#define TIMER34_CH1_CAPTURE(timer,edge)          \
do{                                             \
  T##timer##CCTL1 |= edge;                      \
    if(timer == 3){                              \
      if(PERCFG & 0x20) {                       \
        P1SEL |= BV(7);  \
      }                                         \
      else {                                    \
        P1SEL |= BV(4);  \
      }                                         \
    }                                            \
    else {                                       \
      if(PERCFG & 0x10) {                       \
        P2SEL |= BV(3);  \
      }                                         \
      else {                                     \
        P1SEL |= BV(1);   \
      }                                          \
    }                                             \
}while(0)


// Macros for turning timers on or off
//#define TIMER1_RUN(value)      (T1CTL = (value) ? T1CTL|0x02 : T1CTL&~0x03)
//#define TIMER3_RUN(value)      (T3CTL = (value) ? T3CTL|0x10 : T3CTL&~0x10)
//#define TIMER4_RUN(value)      (T4CTL = (value) ? T4CTL|0x10 : T4CTL&~0x10)
#define TIMER34_RUN(timer)        T##timer##CTL |= 0x10;

// Macro for enabling/ disabling interrupts from the channels of timer 1, 3 or 4.
#define TIMER_CHANNEL_INTERRUPT_ENABLE(timer, channel, value) \
do{                                                        \
  if(value){                                              \
    T##timer##CCTL##channel## |= 0x40;                   \
  } else {                                                \
    T##timer##CCTL##channel## &= ~0x40;                  \
  }                                                       \
} while(0)



// Micro for interrupt flag read and clear
#define TIMER_IFG(timer)              (TIMIF & (0x7 << (timer-3)))
#define TnIF(timer)                   (IRCON & ( 0x1 << (timer)))


#define TIMER_IFG_CLEAR(timer)         (TIMIF &= ~(0x7 << (timer-3)))



// Define timer mode for tiemr3 & 4
#define HAL_TIMER_MODE_FREE             0
#define HAL_TIMER_MODE_DOWN             1
#define HAL_TIMER_MODE_MODULO           2
#define HAL_TIMER_MODE_UP_DOWN          3


#define HAL_TIMER_MODE_SET(timer, mode) \
    st(                                 \
       T##timer##CTL &= ~0X03;          \
       T##timer##CTL |= mode;           \
      )
         




/**************************************************************************************************
// FUNCTIONS
**************************************************************************************************/
extern void HalTimer34Init(uint8 timer);

#endif // HAL_TIMER34_H

/**************************************************************************************************
Copyright 2016 Newbit Studio. All rights reserved.
**************************************************************************************************/






