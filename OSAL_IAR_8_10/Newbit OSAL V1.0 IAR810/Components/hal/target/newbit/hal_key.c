/**************************************************************************************************
Filename:       hal_key.c
Revised:        $Date: 2010-09-15 19:02:45 -0700 (Wed, 15 Sep 2010) $
Revision:       $Revision: 23815 $

Description:    This file contains the interface to the HAL KEY Service.


Copyright 2006-2010 Texas Instruments Incorporated. All rights reserved.

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
PROVIDED �AS IS� WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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
NOTE: If polling is used, the hal_driver task schedules the KeyRead()
to occur every 100ms.  This should be long enough to naturally
debounce the keys.  The KeyRead() function remembers the key
state of the previous poll and will only return a non-zero
value if the key state changes.

NOTE: If interrupts are used, the KeyRead() function is scheduled
25ms after the interrupt occurs by the ISR.  This delay is used
for key debouncing.  The ISR disables any further Key interrupt
until KeyRead() is executed.  KeyRead() will re-enable Key
interrupts after executing.  Unlike polling, when interrupts
are enabled, the previous key state is not remembered.  This
means that KeyRead() will return the current state of the keys
(not a change in state of the keys).

NOTE: If interrupts are used, the KeyRead() fucntion is scheduled by
the ISR.  Therefore, the joystick movements will only be detected
during a pushbutton interrupt caused by S1 or the center joystick
pushbutton.

NOTE: When a switch like S1 is pushed, the S1 signal goes from a normally
high state to a low state.  This transition is typically clean.  The
duration of the low state is around 200ms.  When the signal returns
to the high state, there is a high likelihood of signal bounce, which
causes a unwanted interrupts.  Normally, we would set the interrupt
edge to falling edge to generate an interrupt when S1 is pushed, but
because of the signal bounce, it is better to set the edge to rising
edge to generate an interrupt when S1 is released.  The debounce logic
can then filter out the signal bounce.  The result is that we typically
get only 1 interrupt per button push.  This mechanism is not totally
foolproof because occasionally, signal bound occurs during the falling
edge as well.  A similar mechanism is used to handle the joystick
pushbutton on the DB.  For the EB, we do not have independent control
of the interrupt edge for the S1 and center joystick pushbutton.  As
a result, only one or the other pushbuttons work reasonably well with
interrupts.  The default is the make the S1 switch on the EB work more
reliably.

*********************************************************************/

/**************************************************************************************************
*                                            INCLUDES
**************************************************************************************************/
#include "hal_mcu.h"
#include "hal_defs.h"
#include "hal_types.h"
#include "hal_board.h"
#include "hal_drivers.h"
#include "hal_adc.h"
#include "hal_key.h"
#include "osal.h"

#if (defined HAL_KEY) && (HAL_KEY == TRUE)

/**************************************************************************************************
*                                              MACROS
**************************************************************************************************/

/**************************************************************************************************
*                                            CONSTANTS
**************************************************************************************************/
#define HAL_KEY_RISING_EDGE   0
#define HAL_KEY_FALLING_EDGE  1

#define HAL_KEY_DEBOUNCE_VALUE  25

/* CPU port interrupt */
#define HAL_KEY_CPU_PORT_0_IF P0IF
#define HAL_KEY_CPU_PORT_1_IF P1IF
#define HAL_KEY_CPU_PORT_2_IF P2IF

/*  Key group  */
// Group A: S0,S1,S2
// Group B��S3
// Group C��S4,S5

//#define NEWBIT_BOARD_RADIO_SHIELD
#define NEWBIT_BOARD_OLED_SHIELD   

#ifdef NEWBIT_BOARD_OLED_SHIELD
#define KEY_GROUP_B   
#define KEY_GROUP_C
#endif

// Group A Confingure   

/* SW_0 is at P0.0 */
#define HAL_KEY_SW_0_PORT   P0
#define HAL_KEY_SW_0_BIT    BV(0)
#define HAL_KEY_SW_0_SEL    P0SEL
#define HAL_KEY_SW_0_DIR    P0DIR   

#define HAL_KEY_SW_0_IEN      IEN1  /* CPU interrupt mask register */
#define HAL_KEY_SW_0_IENBIT   BV(5) /* Mask bit for all of Port_0 */
#define HAL_KEY_SW_0_ICTL     P0IEN /* Port Interrupt Control register */
#define HAL_KEY_SW_0_ICTLBIT  BV(0) /* P0IEN - P0.0 enable/disable bit */
#define HAL_KEY_SW_0_PXIFG    P0IFG /* Interrupt flag at source */ 

/* SW_1 is at P0.4 */
#define HAL_KEY_SW_1_PORT   P0
#define HAL_KEY_SW_1_BIT    BV(4)
#define HAL_KEY_SW_1_SEL    P0SEL
#define HAL_KEY_SW_1_DIR    P0DIR     

#define HAL_KEY_SW_1_IEN      IEN1  /* CPU interrupt mask register */
#define HAL_KEY_SW_1_IENBIT   BV(5) /* Mask bit for all of Port_0 */
#define HAL_KEY_SW_1_ICTL     P0IEN /* Port Interrupt Control register */
#define HAL_KEY_SW_1_ICTLBIT  BV(4) /* P0IEN - P0.4 enable/disable bit */
#define HAL_KEY_SW_1_PXIFG    P0IFG /* Interrupt flag at source */   

/* SW_2 is at P0.5 */
#define HAL_KEY_SW_2_PORT   P0
#define HAL_KEY_SW_2_BIT    BV(5)
#define HAL_KEY_SW_2_SEL    P0SEL
#define HAL_KEY_SW_2_DIR    P0DIR      

#define HAL_KEY_SW_2_IEN      IEN1  /* CPU interrupt mask register */
#define HAL_KEY_SW_2_IENBIT   BV(5) /* Mask bit for all of Port_0 */
#define HAL_KEY_SW_2_ICTL     P0IEN /* Port Interrupt Control register */
#define HAL_KEY_SW_2_ICTLBIT  BV(5) /* P0IEN - P0.5 enable/disable bit */
#define HAL_KEY_SW_2_PXIFG    P0IFG /* Interrupt flag at source */      

/* edge interrupt */
#define HAL_KEY_SW_GROUP_A_EDGEBIT  BV(0)
#define HAL_KEY_SW_GROUP_A_EDGE     HAL_KEY_FALLING_EDGE   

#define HAL_KEY_SW_GROUP_A_PXIFG    HAL_KEY_SW_2_PXIFG /* Interrupt flag at source */ 


// Group B Confingure   

#ifdef KEY_GROUP_B

/* SW_3 is at P1.0 */
#define HAL_KEY_SW_3_PORT   P1
#define HAL_KEY_SW_3_BIT    BV(0)
#define HAL_KEY_SW_3_SEL    P1SEL
#define HAL_KEY_SW_3_DIR    P1DIR 

#define HAL_KEY_SW_3_IEN      IEN2  /* CPU interrupt mask register */
#define HAL_KEY_SW_3_IENBIT   BV(4) /* Mask bit for all of Port_1 */
#define HAL_KEY_SW_3_ICTL     P1IEN /* Port Interrupt Control register */
#define HAL_KEY_SW_3_ICTLBIT  BV(0) /* P1IEN - P1.0 enable/disable bit */
#define HAL_KEY_SW_3_PXIFG    P1IFG /* Interrupt flag at source */ 

#define HAL_KEY_SW_GROUP_B_EDGEBIT  BV(1)
#define HAL_KEY_SW_GROUP_B_EDGE     HAL_KEY_FALLING_EDGE      

#define HAL_KEY_SW_GROUP_B_PXIFG    HAL_KEY_SW_3_PXIFG /* Interrupt flag at source */ 

#endif   

// Group C Confingure    

#ifdef KEY_GROUP_C

/* SW_4 is at P1.3 */
#define HAL_KEY_SW_4_PORT   P1
#define HAL_KEY_SW_4_BIT    BV(3)
#define HAL_KEY_SW_4_SEL    P1SEL
#define HAL_KEY_SW_4_DIR    P1DIR     

#define HAL_KEY_SW_4_IEN      IEN2  /* CPU interrupt mask register */
#define HAL_KEY_SW_4_IENBIT   BV(4) /* Mask bit for all of Port_1 */
#define HAL_KEY_SW_4_ICTL     P1IEN /* Port Interrupt Control register */
#define HAL_KEY_SW_4_ICTLBIT  BV(3) /* P0IEN - P1.3 enable/disable bit */
#define HAL_KEY_SW_4_PXIFG    P1IFG /* Interrupt flag at source */   

/* SW_5 is at P1.4 */
#define HAL_KEY_SW_5_PORT   P1
#define HAL_KEY_SW_5_BIT    BV(4)
#define HAL_KEY_SW_5_SEL    P1SEL
#define HAL_KEY_SW_5_DIR    P1DIR      

#define HAL_KEY_SW_5_IEN      IEN2  /* CPU interrupt mask register */
#define HAL_KEY_SW_5_IENBIT   BV(4) /* Mask bit for all of Port_0 */
#define HAL_KEY_SW_5_ICTL     P1IEN /* Port Interrupt Control register */
#define HAL_KEY_SW_5_ICTLBIT  BV(4) /* P1IEN - P1.4 enable/disable bit */
#define HAL_KEY_SW_5_PXIFG    P1IFG /* Interrupt flag at source */      

/* edge interrupt */
#define HAL_KEY_SW_GROUP_C_EDGEBIT  BV(2)
#define HAL_KEY_SW_GROUP_C_EDGE     HAL_KEY_FALLING_EDGE   

#define HAL_KEY_SW_GROUP_C_PXIFG    HAL_KEY_SW_4_PXIFG /* Interrupt flag at source */ 

#endif


#if 0

/* SW_6 is at P0.1 */
#define HAL_KEY_SW_6_PORT   P0
#define HAL_KEY_SW_6_BIT    BV(1)
#define HAL_KEY_SW_6_SEL    P0SEL
#define HAL_KEY_SW_6_DIR    P0DIR

/* edge interrupt */
#define HAL_KEY_SW_6_EDGEBIT  BV(0)
#define HAL_KEY_SW_6_EDGE     HAL_KEY_FALLING_EDGE


/* SW_6 interrupts */
#define HAL_KEY_SW_6_IEN      IEN1  /* CPU interrupt mask register */
#define HAL_KEY_SW_6_IENBIT   BV(5) /* Mask bit for all of Port_0 */
#define HAL_KEY_SW_6_ICTL     P0IEN /* Port Interrupt Control register */
#define HAL_KEY_SW_6_ICTLBIT  BV(1) /* P0IEN - P0.1 enable/disable bit */
#define HAL_KEY_SW_6_PXIFG    P0IFG /* Interrupt flag at source */

#endif

#define HAL_KEY_JOY_CHN   HAL_ADC_CHANNEL_6


/**************************************************************************************************
*                                            TYPEDEFS
**************************************************************************************************/


/**************************************************************************************************
*                                        GLOBAL VARIABLES
**************************************************************************************************/
static uint8 halKeySavedKeys;     /* used to store previous key state in polling mode */
static halKeyCBack_t pHalKeyProcessFunction;
static uint8 HalKeyConfigured;
bool Hal_KeyIntEnable;            /* interrupt enable/disable flag */

/**************************************************************************************************
*                                        FUNCTIONS - Local
**************************************************************************************************/
void halProcessKeyInterrupt(void);
uint8 halGetJoyKeyInput(void);



/**************************************************************************************************
*                                        FUNCTIONS - API
**************************************************************************************************/


/**************************************************************************************************
* @fn      HalKeyInit
*
* @brief   Initilize Key Service
*
* @param   none
*
* @return  None
**************************************************************************************************/
void HalKeyInit( void )
{
  /* Initialize previous key to 0 */
  halKeySavedKeys = 0;
  
  HAL_KEY_SW_0_SEL &= ~(HAL_KEY_SW_0_BIT);    /* Set pin function to GPIO */
  HAL_KEY_SW_0_DIR &= ~(HAL_KEY_SW_0_BIT);    /* Set pin direction to Input */  
  
  HAL_KEY_SW_1_SEL &= ~(HAL_KEY_SW_1_BIT);    /* Set pin function to GPIO */
  HAL_KEY_SW_1_DIR &= ~(HAL_KEY_SW_1_BIT);    /* Set pin direction to Input */
  
  HAL_KEY_SW_2_SEL &= ~(HAL_KEY_SW_2_BIT);    /* Set pin function to GPIO */
  HAL_KEY_SW_2_DIR &= ~(HAL_KEY_SW_2_BIT);    /* Set pin direction to Input */
  
#ifdef KEY_GROUP_B
  HAL_KEY_SW_3_SEL &= ~(HAL_KEY_SW_3_BIT);    /* Set pin function to GPIO */
  HAL_KEY_SW_3_DIR &= ~(HAL_KEY_SW_3_BIT);    /* Set pin direction to Input */
#endif
  
#ifdef KEY_GROUP_C
  HAL_KEY_SW_4_SEL &= ~(HAL_KEY_SW_4_BIT);    /* Set pin function to GPIO */
  HAL_KEY_SW_4_DIR &= ~(HAL_KEY_SW_4_BIT);    /* Set pin direction to Input */
  
  HAL_KEY_SW_5_SEL &= ~(HAL_KEY_SW_5_BIT);    /* Set pin function to GPIO */
  HAL_KEY_SW_5_DIR &= ~(HAL_KEY_SW_5_BIT);    /* Set pin direction to Input */
#endif
  
  /* Initialize callback function */
  pHalKeyProcessFunction  = NULL;
  
  /* Start with key is not configured */
  HalKeyConfigured = FALSE;
  
  
  
  /* Rising/Falling edge configuratinn */    
  PICTL &= ~(HAL_KEY_SW_GROUP_A_EDGEBIT);/* Clear the edge bit */
  
  /* For falling edge, the bit must be set. */
#if (HAL_KEY_SW_GROUP_A_EDGE == HAL_KEY_FALLING_EDGE)
  PICTL |= HAL_KEY_SW_GROUP_A_EDGEBIT;
#endif
  
#ifdef KEY_GROUP_B
  PICTL &= ~(HAL_KEY_SW_GROUP_B_EDGEBIT);    
#if (HAL_KEY_SW_GROUP_B_EDGE == HAL_KEY_FALLING_EDGE)
  PICTL |= HAL_KEY_SW_GROUP_B_EDGEBIT;
#endif
#endif
  
  
#ifdef KEY_GROUP_C
  PICTL &= ~(HAL_KEY_SW_GROUP_C_EDGEBIT);    
#if (HAL_KEY_SW_GROUP_C_EDGE == HAL_KEY_FALLING_EDGE)
  PICTL |= HAL_KEY_SW_GROUP_C_EDGEBIT;
#endif    
#endif    
  
  HAL_KEY_SW_0_IEN |= HAL_KEY_SW_0_IENBIT;
  HAL_KEY_SW_1_IEN |= HAL_KEY_SW_1_IENBIT;
  HAL_KEY_SW_2_IEN |= HAL_KEY_SW_2_IENBIT;
  HAL_KEY_SW_3_IEN |= HAL_KEY_SW_3_IENBIT;
  HAL_KEY_SW_4_IEN |= HAL_KEY_SW_4_IENBIT;
  HAL_KEY_SW_5_IEN |= HAL_KEY_SW_5_IENBIT;
  
}


/**************************************************************************************************
* @fn      HalKeyConfig
*
* @brief   Configure the Key serivce
*
* @param   interruptEnable - TRUE/FALSE, enable/disable interrupt
*          cback - pointer to the CallBack function
*
* @return  None
**************************************************************************************************/
void HalKeyConfig (bool interruptEnable, halKeyCBack_t cback)
{
  /* Enable/Disable Interrupt or */
  Hal_KeyIntEnable = interruptEnable;
  
  /* Register the callback fucntion */
  pHalKeyProcessFunction = cback;
  
  /* Determine if interrupt is enable or not */
  if (Hal_KeyIntEnable)
  {
    
    
    /* Interrupt configuration:
    * - Enable interrupt generation at the port
    * - Enable CPU interrupt
    * - Clear any pending interrupt
    */
    HAL_KEY_SW_0_ICTL |= HAL_KEY_SW_0_ICTLBIT;
    //HAL_KEY_SW_0_IEN |= HAL_KEY_SW_0_IENBIT;
    HAL_KEY_SW_0_PXIFG = ~(HAL_KEY_SW_0_BIT);
    
    HAL_KEY_SW_1_ICTL |= HAL_KEY_SW_1_ICTLBIT;
    //HAL_KEY_SW_1_IEN |= HAL_KEY_SW_1_IENBIT;
    HAL_KEY_SW_1_PXIFG = ~(HAL_KEY_SW_1_BIT);
    
    HAL_KEY_SW_2_ICTL |= HAL_KEY_SW_2_ICTLBIT;
    //HAL_KEY_SW_2_IEN |= HAL_KEY_SW_2_IENBIT;
    HAL_KEY_SW_2_PXIFG = ~(HAL_KEY_SW_2_BIT);
    
#ifdef KEY_GROUP_B
    HAL_KEY_SW_3_ICTL |= HAL_KEY_SW_3_ICTLBIT;
    //HAL_KEY_SW_3_IEN |=  HAL_KEY_SW_3_IENBIT;
    HAL_KEY_SW_3_PXIFG = ~(HAL_KEY_SW_3_BIT);
#endif

#ifdef KEY_GROUP_C
    HAL_KEY_SW_4_ICTL |= HAL_KEY_SW_4_ICTLBIT;
    //HAL_KEY_SW_4_IEN |=  HAL_KEY_SW_4_IENBIT;
    HAL_KEY_SW_4_PXIFG = ~(HAL_KEY_SW_4_BIT);
    
    HAL_KEY_SW_5_ICTL |= HAL_KEY_SW_5_ICTLBIT;
    //HAL_KEY_SW_5_IEN |=  HAL_KEY_SW_5_IENBIT;
    HAL_KEY_SW_5_PXIFG = ~(HAL_KEY_SW_5_BIT);    
#endif
    
    /* Do this only after the hal_key is configured - to work with sleep stuff */
    if (HalKeyConfigured == TRUE)
    {
      osal_stop_timerEx(Hal_TaskID, HAL_KEY_EVENT);  /* Cancel polling if active */
    }
  }
  else    /* Interrupts NOT enabled */
  {
    HAL_KEY_SW_0_ICTL &= ~(HAL_KEY_SW_0_ICTLBIT); /* don't generate interrupt */
    //HAL_KEY_SW_0_IEN &= ~(HAL_KEY_SW_0_IENBIT);   /* Clear interrupt enable bit */
    
    HAL_KEY_SW_1_ICTL &= ~(HAL_KEY_SW_1_ICTLBIT); /* don't generate interrupt */
    //HAL_KEY_SW_1_IEN &= ~(HAL_KEY_SW_1_IENBIT);   /* Clear interrupt enable bit */
    
    HAL_KEY_SW_2_ICTL &= ~(HAL_KEY_SW_2_ICTLBIT); /* don't generate interrupt */
    //HAL_KEY_SW_2_IEN &= ~(HAL_KEY_SW_2_IENBIT);   /* Clear interrupt enable bit */
    
#ifdef KEY_GROUP_B
    HAL_KEY_SW_3_ICTL &= ~(HAL_KEY_SW_3_ICTLBIT); /* don't generate interrupt */
    //HAL_KEY_SW_3_IEN &= ~(HAL_KEY_SW_3_IENBIT);   /* Clear interrupt enable bit */
#endif

#ifdef KEY_GROUP_C
    HAL_KEY_SW_4_ICTL &= ~(HAL_KEY_SW_4_ICTLBIT); /* don't generate interrupt */
    //HAL_KEY_SW_4_IEN &= ~(HAL_KEY_SW_4_IENBIT);   /* Clear interrupt enable bit */
    
    HAL_KEY_SW_5_ICTL &= ~(HAL_KEY_SW_5_ICTLBIT); /* don't generate interrupt */
    //HAL_KEY_SW_5_IEN &= ~(HAL_KEY_SW_5_IENBIT);   /* Clear interrupt enable bit */    
#endif 
    
    osal_set_event(Hal_TaskID, HAL_KEY_EVENT);
  }
  
  /* Key now is configured */
  HalKeyConfigured = TRUE;
}


/**************************************************************************************************
* @fn      HalKeyRead
*
* @brief   Read the current value of a key
*
* @param   None
*
* @return  keys - current keys status
**************************************************************************************************/
uint8 HalKeyRead ( void )
{
  uint8 keys = 0;
  
  if (HAL_PUSH_BUTTON0())
  {
    keys |= HAL_KEY_SW_0;
  }
  
  if (HAL_PUSH_BUTTON1())
  {
    keys |= HAL_KEY_SW_1;
  }
  
  if (HAL_PUSH_BUTTON2())
  {
    keys |= HAL_KEY_SW_2;
  }  
  
  
#ifdef KEY_GROUP_B
  if (HAL_PUSH_BUTTON3())
  {
    keys |= HAL_KEY_SW_3;
  } 
#endif
  
  
#ifdef KEY_GROUP_C
  if (HAL_PUSH_BUTTON4())
    keys |= HAL_KEY_SW_4;

  if (HAL_PUSH_BUTTON5())
    keys |= HAL_KEY_SW_5;  
  
#endif  
  
  return keys;
}


/**************************************************************************************************
* @fn      HalKeyPoll
*
* @brief   Called by hal_driver to poll the keys
*
* @param   None
*
* @return  None
**************************************************************************************************/
void HalKeyPoll (void)
{
  uint8 keys = 0;
  
  keys = HalKeyRead();
  
  /* If interrupts are not enabled, previous key status and current key status
  * are compared to find out if a key has changed status.
  */
  if (!Hal_KeyIntEnable)
  {
    if (keys == halKeySavedKeys)
    {
      /* Exit - since no keys have changed */
      return;
    }
    /* Store the current keys for comparation next time */
    halKeySavedKeys = keys;
  }
  else
  {
    /* Key interrupt handled here */
  }
  
  /* Invoke Callback if new keys were depressed */
  if (keys && (pHalKeyProcessFunction))
  {
    (pHalKeyProcessFunction) (keys, HAL_KEY_STATE_NORMAL);
  }
}

/**************************************************************************************************
* @fn      halGetJoyKeyInput
*
* @brief   Map the ADC value to its corresponding key.
*
* @param   None
*
* @return  keys - current joy key status
**************************************************************************************************/
uint8 halGetJoyKeyInput(void)
{
  /* The joystick control is encoded as an analog voltage.
  * Read the JOY_LEVEL analog value and map it to joy movement.
  */
  uint8 adc;
  uint8 ksave0 = 0;
  uint8 ksave1;
  
  /* Keep on reading the ADC until two consecutive key decisions are the same. */
  do
  {
    ksave1 = ksave0;    /* save previouse key reading */
    
    adc = HalAdcRead (HAL_KEY_JOY_CHN, HAL_ADC_RESOLUTION_8);
    
    if ((adc >= 2) && (adc <= 38))
    {
      ksave0 |= HAL_KEY_UP;
    }
    else if ((adc >= 74) && (adc <= 88))
    {
      ksave0 |= HAL_KEY_RIGHT;
    }
    else if ((adc >= 60) && (adc <= 73))
    {
      ksave0 |= HAL_KEY_LEFT;
    }
    else if ((adc >= 39) && (adc <= 59))
    {
      ksave0 |= HAL_KEY_DOWN;
    }
    else if ((adc >= 89) && (adc <= 100))
    {
      ksave0 |= HAL_KEY_CENTER;
    }
  } while (ksave0 != ksave1);
  
  return ksave0;
}





/**************************************************************************************************
* @fn      halProcessKeyInterrupt
*
* @brief   Checks to see if it's a valid key interrupt, saves interrupt driven key states for
*          processing by HalKeyRead(), and debounces keys by scheduling HalKeyRead() 25ms later.
*
* @param
*
* @return
**************************************************************************************************/
void halProcessKeyInterrupt (void)
{
  bool valid=FALSE;
  
  if (HAL_KEY_SW_0_PXIFG & HAL_KEY_SW_0_BIT)  /* Interrupt Flag has been set */
  {
    HAL_KEY_SW_0_PXIFG = ~(HAL_KEY_SW_0_BIT); /* Clear Interrupt Flag */
    valid = TRUE;
  }
  
  if (HAL_KEY_SW_1_PXIFG & HAL_KEY_SW_1_BIT)  /* Interrupt Flag has been set */
  {
    HAL_KEY_SW_1_PXIFG = ~(HAL_KEY_SW_1_BIT); /* Clear Interrupt Flag */
    valid = TRUE;
  }
  
  
  if (HAL_KEY_SW_2_PXIFG & HAL_KEY_SW_2_BIT)  /* Interrupt Flag has been set */
  {
    HAL_KEY_SW_2_PXIFG = ~(HAL_KEY_SW_2_BIT); /* Clear Interrupt Flag */
    valid = TRUE;
  }  
  
#ifdef KEY_GROUP_B
  if (HAL_KEY_SW_3_PXIFG & HAL_KEY_SW_3_BIT)  /* Interrupt Flag has been set */
  {
    HAL_KEY_SW_3_PXIFG = ~(HAL_KEY_SW_3_BIT); /* Clear Interrupt Flag */
    valid = TRUE;
  }  
#endif

#ifdef KEY_GROUP_C
  if (HAL_KEY_SW_4_PXIFG & HAL_KEY_SW_4_BIT)  /* Interrupt Flag has been set */
  {
    HAL_KEY_SW_4_PXIFG = ~(HAL_KEY_SW_4_BIT); /* Clear Interrupt Flag */
    valid = TRUE;
  } 
 
  if (HAL_KEY_SW_5_PXIFG & HAL_KEY_SW_5_BIT)  /* Interrupt Flag has been set */
  {
    HAL_KEY_SW_5_PXIFG = ~(HAL_KEY_SW_5_BIT); /* Clear Interrupt Flag */
    valid = TRUE;
  }  
#endif
  
  if (valid)
  {
    osal_start_timerEx (Hal_TaskID, HAL_KEY_EVENT, HAL_KEY_DEBOUNCE_VALUE);
  }
}

/**************************************************************************************************
* @fn      HalKeyEnterSleep
*
* @brief  - Get called to enter sleep mode
*
* @param
*
* @return
**************************************************************************************************/
void HalKeyEnterSleep ( void )
{
}

/**************************************************************************************************
* @fn      HalKeyExitSleep
*
* @brief   - Get called when sleep is over
*
* @param
*
* @return  - return saved keys
**************************************************************************************************/
uint8 HalKeyExitSleep ( void )
{
  /* Wake up and read keys */
  return ( HalKeyRead () );
}

/***************************************************************************************************
*                                    INTERRUPT SERVICE ROUTINE
***************************************************************************************************/

/**************************************************************************************************
* @fn      halKeyPort0Isr
*
* @brief   Port0 ISR
*
* @param
*
* @return
**************************************************************************************************/
HAL_ISR_FUNCTION( halKeyPort0Isr, P0INT_VECTOR )
{
  HAL_ENTER_ISR();
  
  extern void halProcessOneBusInterrupt (void);
  halProcessOneBusInterrupt();
  
  
  if (HAL_KEY_SW_0_PXIFG & HAL_KEY_SW_0_BIT)
  {
    halProcessKeyInterrupt();
  }
  
  if (HAL_KEY_SW_1_PXIFG & HAL_KEY_SW_1_BIT)
  {
    halProcessKeyInterrupt();
  }  
  
  if (HAL_KEY_SW_2_PXIFG & HAL_KEY_SW_2_BIT)
  {
    halProcessKeyInterrupt();
  }
  
  /*
  Clear the CPU interrupt flag for Port_0
  PxIFG has to be cleared before PxIF
  */
  HAL_KEY_SW_GROUP_A_PXIFG = 0;
  HAL_KEY_CPU_PORT_0_IF = 0;  
  
  CLEAR_SLEEP_MODE();
  HAL_EXIT_ISR();
}

/**************************************************************************************************
* @fn      halKeyPort1Isr
*
* @brief   Port1 ISR
*
* @param
*
* @return
**************************************************************************************************/
HAL_ISR_FUNCTION( halKeyPort1Isr, P1INT_VECTOR )
{
  HAL_ENTER_ISR();
  
#ifdef KEY_GROUP_B
  if (HAL_KEY_SW_3_PXIFG & HAL_KEY_SW_3_BIT)
  {
    halProcessKeyInterrupt();
  }
#endif
  
  
#ifdef KEY_GROUP_C
  if (HAL_KEY_SW_4_PXIFG & HAL_KEY_SW_4_BIT)
  {
    halProcessKeyInterrupt();
  }  
  
  if (HAL_KEY_SW_5_PXIFG & HAL_KEY_SW_5_BIT)
  {
    halProcessKeyInterrupt();
  }
#endif
  
  /*
  Clear the CPU interrupt flag for Port_0
  PxIFG has to be cleared before PxIF
  */
  HAL_KEY_SW_GROUP_B_PXIFG = 0;
  HAL_KEY_CPU_PORT_1_IF = 0;  
  
  CLEAR_SLEEP_MODE();
  HAL_EXIT_ISR();
}

/**************************************************************************************************
* @fn      halKeyPort2Isr
*
* @brief   Port2 ISR
*
* @param
*
* @return
**************************************************************************************************/
HAL_ISR_FUNCTION( halKeyPort2Isr, P2INT_VECTOR )
{
  HAL_ENTER_ISR();
  
  
  CLEAR_SLEEP_MODE();
  HAL_EXIT_ISR();
}

#else


void HalKeyInit(void){}
void HalKeyConfig(bool interruptEnable, halKeyCBack_t cback){}
uint8 HalKeyRead(void){ return 0;}
void HalKeyPoll(void){}

#endif /* HAL_KEY */





/**************************************************************************************************
**************************************************************************************************/



