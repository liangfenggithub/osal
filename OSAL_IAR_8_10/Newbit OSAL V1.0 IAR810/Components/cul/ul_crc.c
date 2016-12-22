/******************************************************************************
Filename:     ul_crc.c
Target:                                
Author:       emot
Version:      1.0
Release:      
Revised:      

----------------------------------------------------------------------
|ʱ��           ԭ��                  �޶���
|2009��5��13��  ��д                  emot
|
|
**********************************************************************/


/**********************************************************************
*ͷ�ļ�
*/
#include "hal_types.h"
#include "string.h"

/*----------------------------------��----------------------------------*/
#define  CRC_POLYNOM	        0x8408  //CRCħ��ֵ
#define  CRC_PRESET		0xFFFF  //CRC��ʼֵ

/*******************************************************************************
 * @fn          ulCalcCrc()
 *
 * @brief       ����CRCУ��ֵ��ħ��ֵ0x8408
 *
 * @param       uint8 *msg_ptr - ��У��ֵ����
 *              uint8 len - ��У�����ݳ���
 *
 * @return      CRCУ����
 * 
*******************************************************************************/
uint16 ulCalcCrc(uint8 *msg_ptr,uint8 len)
{
  uint8 i,j;
  uint16 CRM_Check;
  uint8 CRC_BUFF[100];  //Ŀǰ�����Ը�100���ֽڳ��ȵ�������У��
  
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
