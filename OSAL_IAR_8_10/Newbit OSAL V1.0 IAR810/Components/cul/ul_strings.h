/**************************************************************************************************
Filename:       ul_string.h
Editor:         Tome @ newbit
Revised:        $Date: 2014-4-22 21:20:02 +0800  $
Revision:       $Revision: 00001 $

Description:    
History:        
Notes:          



**************************************************************************************************/



/**************************************************************************************************
// INCLUDES
**************************************************************************************************/



#ifndef UL_STRING_H
#define UL_STRING_H

/*----------------------------------ͷ�ļ�----------------------------------*/
#include "hal_types.h"

/*----------------------------------CRC----------------------------------*/
#define  CRC_POLYNOM	        0x8408  //CRCħ��ֵ
#define  CRC_PRESET		0xFFFF  //CRC��ʼֵ

extern uint16 ul_calc_crc(uint8 *msg_ptr,uint8 len);



/*----------------------------------��ֵ�Ƚ�----------------------------------*/
//extern uint8 ul_max(uint8 n,...);


/*----------------------------------�ڴ��ӡ----------------------------------*/
extern void ul_sprintStr(uint8 * dest, uint8  *scr,uint8  len);
extern void ul_sprintWord2Hex(uint8 *dest, uint16 n);
extern void ul_sprintUint16ToDec(uint8 *dest, uint16 n);
extern void ul_stringUpCase(uint8 *str, uint8 len);
extern void ul_stringLowCase(uint8 *str, uint8 len);
extern uint8* ul_searchChar(uint8 *str, uint8 len, char ch);
extern uint8 ul_string2Uint16(uint8 *str,uint8 len,uint16 *n);
extern uint16 ul_memoryCompare(uint8* str1, uint8* str2, uint16 length);
extern uint8 ul_memorySet(uint8* str, uint8 ch, uint16 length);
extern uint8 ul_memoryCopy(uint8* destStr, const uint8* srcStr, uint16 length);

#endif // UL_STRING_H

/**************************************************************************************************
Copyright 2014 Newbit Studio. All rights reserved.
**************************************************************************************************/





