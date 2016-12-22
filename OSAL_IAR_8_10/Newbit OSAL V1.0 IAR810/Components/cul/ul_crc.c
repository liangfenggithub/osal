/******************************************************************************
Filename:     ul_crc.c
Target:                                
Author:       emot
Version:      1.0
Release:      
Revised:      

----------------------------------------------------------------------
|时间           原因                  修订人
|2009年5月13日  初写                  emot
|
|
**********************************************************************/


/**********************************************************************
*头文件
*/
#include "hal_types.h"
#include "string.h"

/*----------------------------------宏----------------------------------*/
#define  CRC_POLYNOM	        0x8408  //CRC魔法值
#define  CRC_PRESET		0xFFFF  //CRC初始值

/*******************************************************************************
 * @fn          ulCalcCrc()
 *
 * @brief       计算CRC校验值，魔法值0x8408
 *
 * @param       uint8 *msg_ptr - 求校验值数据
 *              uint8 len - 求校验数据长度
 *
 * @return      CRC校验结果
 * 
*******************************************************************************/
uint16 ulCalcCrc(uint8 *msg_ptr,uint8 len)
{
  uint8 i,j;
  uint16 CRM_Check;
  uint8 CRC_BUFF[100];  //目前最多可以给100个字节长度的数据作校验
  
  memcpy(CRC_BUFF,msg_ptr,len);  
  
  CRM_Check = CRC_PRESET;
  for (i = 0; i < len; i++)
  {
          CRM_Check ^= (uint16)CRC_BUFF[i];
          for (j = 0; j < 8; j++)
          {
                  if (CRM_Check & 0x0001)
                          CRM_Check = (CRM_Check >> 1) ^ CRC_POLYNOM;
                  else
                          CRM_Check = (CRM_Check >> 1);
          }
  }
  return CRM_Check;
}
