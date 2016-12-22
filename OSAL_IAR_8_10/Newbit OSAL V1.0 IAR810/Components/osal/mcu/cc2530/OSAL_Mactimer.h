//Osal_Mactimer.h

#ifndef OSAL_MAC_TIMER_H
#define OSAL_MAC_TIMER_H







/* ------------------------------------------------------------------------------------------------
 *                                     Compiler Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_mcu.h"
#include "hal_types.h"
#include "hal_defs.h"
#include "hal_board.h"
//#include "mac_high_level.h"


/* ------------------------------------------------------------------------------------------------
 *                                    Target Specific Defines
 * ------------------------------------------------------------------------------------------------
 */
/* IP0, IP1 */
#define IPX_0                 BV(0)
#define IPX_1                 BV(1)
#define IPX_2                 BV(2)
#define IP_RFERR_RF_DMA_BV    IPX_0
#define IP_RXTX0_T2_BV        IPX_2

/* T2CTRL */
#define LATCH_MODE            BV(3)
#define TIMER2_STATE          BV(2)
#define TIMER2_SYNC           BV(1)
#define TIMER2_RUN            BV(0)

/* T2IRQF */
#define TIMER2_OVF_COMPARE2F  BV(5)
#define TIMER2_OVF_COMPARE1F  BV(4)
#define TIMER2_OVF_PERF       BV(3)
#define TIMER2_COMPARE2F      BV(2)
#define TIMER2_COMPARE1F      BV(1)
#define TIMER2_PERF           BV(0)

/* T2IRQM */
#define TIMER2_OVF_COMPARE2M  BV(5)
#define TIMER2_OVF_COMPARE1M  BV(4)
#define TIMER2_OVF_PERM       BV(3)
#define TIMER2_COMPARE2M      BV(2)
#define TIMER2_COMPARE1M      BV(1)
#define TIMER2_PERM           BV(0)

/* T2PEROF2 */
#define CMPIM           BV(7)
#define PERIM           BV(6)
#define OFCMPIM         BV(5)
#define PEROF2_BITS     (BV(3) | BV(2) | BV(1) | BV(0))

/* RFIRQF0 */
#define IRQ_SFD         BV(1)
#define IRQ_FIFOP       BV(2)

/* RFIRQF1 */
#define IRQ_TXACKDONE   BV(0)
#define IRQ_TXDONE      BV(1)
#define IRQ_CSP_MANINT  BV(3)
#define IRQ_CSP_STOP    BV(4)

/* RFIRQM0 */
#define IM_SFD          BV(1)
#define IM_FIFOP        BV(2)

/* RFIRQM1 */
#define IM_TXACKDONE    BV(0)
#define IM_TXDONE       BV(1)
#define IM_CSP_MANINT   BV(3)
#define IM_CSP_STOP     BV(4)

/* IRQSRC */
#define TXACK           BV(0)

/* RFERRM and RFERRF */
#define RFERR_RXOVERF   BV(2)


/* ------------------------------------------------------------------------------------------------
 *                                       Interrupt Macros
 * ------------------------------------------------------------------------------------------------
 */
#define MAC_MCU_WRITE_RFIRQF0(x)      HAL_CRITICAL_STATEMENT( S1CON = 0x00; RFIRQF0 = x; )
#define MAC_MCU_WRITE_RFIRQF1(x)      HAL_CRITICAL_STATEMENT( S1CON = 0x00; RFIRQF1 = x; )
#define MAC_MCU_OR_RFIRQM0(x)         st( RFIRQM0 |= x; )  /* compiler must use atomic ORL instruction */
#define MAC_MCU_AND_RFIRQM0(x)        st( RFIRQM0 &= x; )  /* compiler must use atomic ANL instruction */
#define MAC_MCU_OR_RFIRQM1(x)         st( RFIRQM1 |= x; )  /* compiler must use atomic ORL instruction */
#define MAC_MCU_AND_RFIRQM1(x)        st( RFIRQM1 &= x; )  /* compiler must use atomic ANL instruction */

#define MAC_MCU_FIFOP_ENABLE_INTERRUPT()              MAC_MCU_OR_RFIRQM0(IM_FIFOP)
#define MAC_MCU_FIFOP_DISABLE_INTERRUPT()             MAC_MCU_AND_RFIRQM0(~IM_FIFOP)
#define MAC_MCU_FIFOP_CLEAR_INTERRUPT()               MAC_MCU_WRITE_RFIRQF0(~IRQ_FIFOP)

#define MAC_MCU_TXACKDONE_ENABLE_INTERRUPT()          MAC_MCU_OR_RFIRQM1(IM_TXACKDONE)
#define MAC_MCU_TXACKDONE_DISABLE_INTERRUPT()         MAC_MCU_AND_RFIRQM1(~IM_TXACKDONE)
#define MAC_MCU_TXACKDONE_CLEAR_INTERRUPT()           MAC_MCU_WRITE_RFIRQF1(~IRQ_TXACKDONE)

#define MAC_MCU_CSP_STOP_ENABLE_INTERRUPT()           MAC_MCU_OR_RFIRQM1(IM_CSP_STOP)
#define MAC_MCU_CSP_STOP_DISABLE_INTERRUPT()          MAC_MCU_AND_RFIRQM1(~IM_CSP_STOP)
#define MAC_MCU_CSP_STOP_CLEAR_INTERRUPT()            MAC_MCU_WRITE_RFIRQF1(~IRQ_CSP_STOP)
#define MAC_MCU_CSP_STOP_INTERRUPT_IS_ENABLED()       (RFIRQM1 & IM_CSP_STOP)

#define MAC_MCU_CSP_INT_ENABLE_INTERRUPT()            MAC_MCU_OR_RFIRQM1(IM_CSP_MANINT)
#define MAC_MCU_CSP_INT_DISABLE_INTERRUPT()           MAC_MCU_AND_RFIRQM1(~IM_CSP_MANINT)
#define MAC_MCU_CSP_INT_CLEAR_INTERRUPT()             MAC_MCU_WRITE_RFIRQF1(~IRQ_CSP_MANINT)
#define MAC_MCU_CSP_INT_INTERRUPT_IS_ENABLED()        (RFIRQM1 & IM_CSP_MANINT)

#define MAC_MCU_RFERR_ENABLE_INTERRUPT()              st( RFERRM |=  RFERR_RXOVERF; )
#define MAC_MCU_RFERR_DISABLE_INTERRUPT()             st( RFERRM &= ~RFERR_RXOVERF; )

/* ------------------------------------------------------------------------------------------------
 *                                       MAC Timer Macros
 * ------------------------------------------------------------------------------------------------
 */
#define T2M_OVF_BITS    (BV(6) | BV(5) | BV(4))
#define T2M_BITS        (BV(2) | BV(1) | BV(0))

#define T2M_OVFSEL(x)   ((x) << 4)
#define T2M_SEL(x)      (x)

#define T2M_T2OVF       T2M_OVFSEL(0)
#define T2M_T2OVF_CAP   T2M_OVFSEL(1)
#define T2M_T2OVF_PER   T2M_OVFSEL(2)
#define T2M_T2OVF_CMP1  T2M_OVFSEL(3)
#define T2M_T2OVF_CMP2  T2M_OVFSEL(4)

#define T2M_T2TIM       T2M_SEL(0)
#define T2M_T2_CAP      T2M_SEL(1)
#define T2M_T2_PER      T2M_SEL(2)
#define T2M_T2_CMP1     T2M_SEL(3)
#define T2M_T2_CMP2     T2M_SEL(4)

#define MAC_MCU_T2_ACCESS_OVF_COUNT_VALUE()   st( T2MSEL = T2M_T2OVF; )
#define MAC_MCU_T2_ACCESS_OVF_CAPTURE_VALUE() st( T2MSEL = T2M_T2OVF_CAP; )
#define MAC_MCU_T2_ACCESS_OVF_PERIOD_VALUE()  st( T2MSEL = T2M_T2OVF_PER; )
#define MAC_MCU_T2_ACCESS_OVF_CMP1_VALUE()    st( T2MSEL = T2M_T2OVF_CMP1; )
#define MAC_MCU_T2_ACCESS_OVF_CMP2_VALUE()    st( T2MSEL = T2M_T2OVF_CMP2; )

#define MAC_MCU_T2_ACCESS_COUNT_VALUE()       st( T2MSEL = T2M_T2TIM; )
#define MAC_MCU_T2_ACCESS_CAPTURE_VALUE()     st( T2MSEL = T2M_T2_CAP; )
#define MAC_MCU_T2_ACCESS_PERIOD_VALUE()      st( T2MSEL = T2M_T2_PER; )
#define MAC_MCU_T2_ACCESS_CMP1_VALUE()        st( T2MSEL = T2M_T2_CMP1; )
#define MAC_MCU_T2_ACCESS_CMP2_VALUE()        st( T2MSEL = T2M_T2_CMP2; )

#define MAC_MCU_CONFIG_CSP_EVENT1()           st( T2CSPCFG = 1; )


/* ------------------------------------------------------------------------------------------------
 *                                   Global Variable Externs
 * ------------------------------------------------------------------------------------------------
 */
extern uint8 macChipVersion;


/* Tome addition */

/* Function pointer for the 16 byte random seed callback */
typedef void (*macRNGFcn_t )(uint8* seed);


#define CORR_THR                      0x14
#define CCA_THR                       0xFC   /* -4-76=-80dBm when CC2530 operated alone or with CC2591 in LGM */
#define TXFILTCFG_RESET_VALUE         0x09

/* IEN2 */
#define RFIE                          BV(0)

/* The number of symbols forming a basic CSMA-CA time period */
#define MAC_A_UNIT_BACKOFF_PERIOD       20
/* Microseconds in one symbol */
#define MAC_SPEC_USECS_PER_SYMBOL           16

#define MAC_SPEC_USECS_PER_BACKOFF          (MAC_SPEC_USECS_PER_SYMBOL * MAC_A_UNIT_BACKOFF_PERIOD)
#define MAC_RADIO_TIMER_TICKS_PER_BACKOFF()           (HAL_CPU_CLOCK_MHZ * MAC_SPEC_USECS_PER_BACKOFF)


#define MAC_RADIO_TIMER_WAKE_UP()                     st( HAL_CLOCK_STABLE(); \
                                                          T2CTRL |= (TIMER2_RUN | TIMER2_SYNC); \
                                                          while(!(T2CTRL & TIMER2_STATE)); )

#define RX_MODE(x)                    ((x) << 2)
#define FRMCTRL0_RESET_VALUE          0x40
#define RX_MODE_INFINITE_RECEPTION    RX_MODE(2)



/* ------------------------------------------------------------------------------------------------
 *                                       Prototypes
 * ------------------------------------------------------------------------------------------------
 */
#define MAC_INTERNAL_API
MAC_INTERNAL_API void macMcuInit(void);
MAC_INTERNAL_API uint8 macMcuRandomByte(void);
MAC_INTERNAL_API uint16 macMcuRandomWord(void);
MAC_INTERNAL_API void macMcuTimerForceDelay(uint16 count);
MAC_INTERNAL_API uint16 macMcuTimerCapture(void);
MAC_INTERNAL_API uint32 macMcuOverflowCount(void);
MAC_INTERNAL_API uint32 macMcuOverflowCapture(void);
MAC_INTERNAL_API void macMcuOverflowSetCount(uint32 count);
MAC_INTERNAL_API void macMcuOverflowSetCompare(uint32 count);
MAC_INTERNAL_API void macMcuOverflowSetPeriod(uint32 count);
MAC_INTERNAL_API void macMcuRecordMaxRssiStart(void);
MAC_INTERNAL_API int8 macMcuRecordMaxRssiStop(void);
MAC_INTERNAL_API void macMcuRecordMaxRssiIsr(void);
MAC_INTERNAL_API void macMcuAccumulatedOverFlow(void);
uint32 macMcuPrecisionCount(void);
void macMcuTimer2OverflowWorkaround(void);


/**************************************************************************************************
 */












#endif