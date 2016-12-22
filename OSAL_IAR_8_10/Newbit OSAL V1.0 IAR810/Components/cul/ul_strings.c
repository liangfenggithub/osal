/**************************************************************************************************
Filename:       ul_strings.c
Editor:         Tome @ newbit
Revised:        $Date: 2014-4-22 21:20:02 +0800  $
Revision:       $Revision: 00001 $

Description:    
History:        
Notes:          

**************************************************************************************************/


/*----------------------------------ͷ�ļ�----------------------------------*/
#include "hal_defs.h"
#include "hal_types.h"

/*----------------------------------���غ���----------------------------------*/
uint8 * ul_searchChar(uint8  *str, uint8 len, char ch);
void ul_stringUpCase(uint8  *str, uint8 len);

/*******************************************************************************
 * @fn          ul_sprintstr()
 *
 * @brief       �������ӡΪ�������ƴ�,��� arr[2] = {0xab,0xcd}��ӡΪABCD
 *              
 * Parameters:
 *
 * @param       uint8  *dest - ��ӡ�����Ż���
 * @param       uint8  *src - ����ӡ�ִ�
 * @param       uint8  len - ��ӡ����
 *
 * @return      none
 * 
*******************************************************************************/
void ul_sprintStr(uint8 * dest, uint8  *src,uint8  len)
{
  uint8  u8i;
  uint8  temp;
  for(u8i=0;u8i<len;u8i++)
  {
    /**********************************************************************/
    //���ֽڣ���λ����
    //��0XABCD,��ӡΪ"ABCD"
    //temp = *(scr+len-u8i);
    temp = *(src+u8i);
    temp = (temp>>4)&0x0f;
    if(temp<10)temp += 0x30;
    else
      temp += 0x37;
    *(dest+2*u8i) = temp;
    /**********************************************************************/
    //�ֽڵ�λ�ں�
    //temp = *(scr+len-u8i);
    temp = *(src+u8i);
    temp = temp&0x0f;
    if(temp<10)temp += 0x30;
    else
      temp += 0x37;
    *(dest+2*u8i+1) = temp;
  }
}

/*******************************************************************************
 * @fn          ul_sprintWord2Hex()
 *
 * @brief       ���ִ�ӡΪ�ִ�
 *              
 * Parameters:
 *
 * @param       uint8  *dest - ��ӡ�����Ż���
 * @param       uint8  n - ����ӡ��
 *
 * @return      none
 * 
*******************************************************************************/
void ul_sprintWord2Hex(uint8  *dest, uint16 n)
{
  uint8  tmp;
  
  *dest++ = '0';
  *dest++ = 'x';
  tmp = (uint8 )((n&0xf000)>>12);
  *dest++ = (tmp < 0x0a) ? (tmp + '0') : (tmp - 10 + 'A');
  tmp = (uint8 )((n&0x0f00)>>8);
  *dest++ = (tmp < 0x0a) ? (tmp + '0') : (tmp - 10 + 'A');
  tmp = (uint8 )((n&0x00f0)>>4);
  *dest++ = (tmp < 0x0a) ? (tmp + '0') : (tmp - 10 + 'A');
  tmp = (uint8 )(n & 0x000f);
  *dest++ = (tmp < 0x0a) ? (tmp + '0') : (tmp - 10 + 'A');
  *dest = '\n';
}

/*******************************************************************************
 * @fn          ul_sprintUint16ToDec
 *
 * @brief       ���ִ�ӡΪ�ִ�
 *              
 * Parameters:
 *
 * @param       uint8  *dest - ��ӡ�����Ż���
 * @param       uint8  n - ����ӡ��
 *
 * @return      none
 * 
*******************************************************************************/
void ul_sprintUint16ToDec(uint8  *dest, uint16 n)
{
  uint8  tmp;
  uint8  z_space = 1;//��1��ʾ��0��ӡΪ�ո�
  
  /*��λ*/
  tmp = n/10000;
  if(tmp)
  {
    *dest++ = tmp + '0';
    z_space = 0;
  }
  else
  {
    *dest++ = ' ';
  }
  
  /*ǧλ*/
  tmp = (n % 10000) / 1000;
  if(tmp)
  {
    *dest++ = tmp + '0';
    z_space = 0;
  }
  else
  {
    if (z_space)
    {
      *dest++ = ' ';
    }
    else
    {
      *dest++ = '0';
    }
  }
  
  /*��λ*/
  tmp = (n % 1000) / 100;
  if(tmp)
  {
    *dest++ = tmp + '0';
    z_space = 0;
  }
  else
  {
    if (z_space)
    {
      *dest++ = ' ';
    }
    else
    {
      *dest++ = '0';
    }
  }
  
  /*ʮλ*/
  tmp = (n % 100) / 10;
  if(tmp)
  {
    *dest++ = tmp + '0';
    z_space = 0;
  }
  else
  {
    if (z_space)
    {
      *dest++ = ' ';
    }
    else
    {
      *dest++ = '0';
    }
  }
  
  /*��λ*/
  tmp = (n % 10);
  *dest++ = tmp + '0';
  
  *dest = '\n';  //����6uint8 s
}



/*******************************************************************************
 * @fn          ul_searchChar
 *
 * @brief       ���ִ��������ַ���һ�γ��ֵ�λ��
 *              
 * Parameters:
 *
 * @param       uint8  *str - �����������ִ�
 * @param       uint8 len - �ִ�����,�޶�������Χ
 * @param       char ch - �������ַ�
 *
 * @return      uint8* ch���ڵ�ַ�����û���ҵ�������NULL
 * 
*******************************************************************************/
uint8 * ul_searchChar(uint8  *str, uint8 len, char ch)
{
  uint8 pos;
  uint8  *p = NULL;
  
  for (pos = 0; pos < len; pos++)
  {
    if ( *(pos + str) == ch)
    {
      p = str + pos;
      break;
    }
  }
  
  return p;
}


/*******************************************************************************
 * @fn          ul_stringUpCase
 *
 * @brief       ���ִ��е�Сд��ĸ��Ϊ��д
 *              
 * Parameters:
 *
 * @param       uint8  *str - Դ�ִ�
 * @param       uint8 len - �ִ�����
 *
 * @return      none
 * 
*******************************************************************************/
void ul_stringUpCase(uint8  *str, uint8 len)
{
  uint8 u8i;
  
  for (u8i = 0; u8i < len; u8i++)
  {
    if ((str[u8i] <= 'z') && (str[u8i] >= 'a'))
    {
      str[u8i] -= 32;
    }
  }
}


/*******************************************************************************
 * @fn          ul_stringLowCase
 *
  * @brief      ���ִ��еĴ�д��ĸ��ΪСд
 *              
 * Parameters:
 *
 * @param       uint8  *str - Դ�ִ�
 * @param       uint8 len - �ִ�����
 *
 * @return      none
 * 
*******************************************************************************/
void ul_stringLowCase(uint8  *str, uint8 len)
{
  uint8 u8i;
  
  for (u8i = 0; u8i < len; u8i++)
  {
    if ((str[u8i] <= 'Z') && (str[u8i] >= 'A'))
    {
      str[u8i] += 32;
    }
  }
}

/*******************************************************************************
 * @fn          ul_string2Uint16
 *
 * @brief       ��ȡ�ִ�����ʾ������,��"123"->123,����ִ�����4,ֻȡǰ4������
 *              
 * Parameters:
 *
 * @param       uint8  *str - Դ�ִ�
 * @param       uint8 len - �ִ�����
 * @param       uint16 *n - ������ɽ��
 *
 * @return      uint8  �ɹ�����1,ʧ�ܷ���0
 * 
*******************************************************************************/
uint8  ul_string2Uint16(uint8  *str,uint8 len,uint16 *n)
{
  uint8 deal_len;//���δ���ĳ���,������ȥ����߿ո���ұ���Ч����ĳ���
  uint8  *start;
  uint8  data[4];
  uint8 u8i;
  uint8  hex;
  uint16 res = 0x0000;
  
  
  ul_stringUpCase(str,len);
  /*ȥ�����Ч����*/
  start = str;
  u8i = len;
  while (u8i--)
  {
    if (*start == ' ')start++;
    else
      break;
  }
  
  /*ȷ������λ��,��ʱstartָ��ո��ĵ�һ�ֽ�*/  
  hex = 0;
  deal_len = 0;
  if ((*(start) == '0') && (*(start+1) == 'X'))
  {
    start += 2;
    hex = 1;                               //16���Ƶ���
  }
  u8i = 4;
  while(u8i && (start < (str + len)))
  {
    if (( *(start) >= '0') && ( *(start) <= '9'))
    {
      data[4-u8i] = *(start) - '0';
      deal_len++;
    }
    else if ( (hex == 1) &&( *(start) >= 'A') && ( *(start) <= 'F'))
    {
      data[4-u8i] = *(start) - 'A' + 0x0a;
      deal_len++;
    }
    else
    {
      break;
    }    
    start++;
    u8i--;
  }
  
  /*��������з����ȡ����*/
  if (deal_len == 0)
  {
    return FALSE;
  }
  
  /*������*/
  if (hex == 1)
  {
    u8i = deal_len;
    while(u8i)
    {
      res |= (uint16)data[deal_len-u8i] << 4 * (u8i);
      u8i--;
    }
  }
  else 
  {
    u8i = deal_len;
    while(u8i)
    {
      res *= 10;
      res += data[deal_len-u8i];
      if (--u8i == 0)break;      
    }
  }
  
  *n = res;
  return TRUE;
}



/*******************************************************************************
 * @fn          ul_string_compare(uint8 * str1, uint8 * str2, uint16 length)
 *
 * @brief       �Ƚ������ڴ��Ƿ�һ��
 *              
 * Parameters:
 *
 * @param       uint8  *str1 - �ִ�1
 * @param       uint8  *str2 - �ִ�2
 * @param       uint16 length - �Ƚϵĳ���
 *
 * @return      ��ȷ���0�������򷵻�ָ��
 * 
*******************************************************************************/
uint16 ul_memoryCompare(uint8 * str1, uint8 * str2, uint16 length)
{
  uint16 i;
  
  for (i = 0; i < length; i++)
  {
    if (*str1++ != *str2++) 
    {
      return ((uint16) str1);
    }    
  }
  
  return 0; //�ִ���ͬ�򷵻�0  
}

/*******************************************************************************
 * @fn          ul_memorySet(uint8 * str, uint8  ch, uint16 length)
 *
 * @brief       ��ĳ���ڴ�����Ϊ������ͬ��ֵ
 *              
 * Parameters:
 *
 * @param       uint8  *str1 - �ִ�1
 *              uint8  ch - ��������
 *              uint16 length - ���ĳ���
 *
 * @return      ���Ƿ���0
 * 
*******************************************************************************/
uint8 ul_memorySet(uint8 * str, uint8  ch, uint16 length)
{
  uint16 i;
  
  for (i = 0; i < length; i++)
  {
    *str++ = ch; 
  }
  return 0;
}



/*******************************************************************************
 * @fn          ul_memoryCopy(uint8 * destStr, uint8 * srcStr, uint16 length)
 *
 * @brief       ��srcStrָ����ڴ����ݸ��Ƶ�destStr,���Ƴ�����lengthָ��
 *              
 * Parameters:
 *
 * @param       uint8  *str1 - �ִ�1
 * @param       uint8  *str2 - �ִ�2
 * @param       uint16 length - �Ƚϵĳ���
 *
 * @return      ��ȷ���0�������򷵻�ָ��
 * 
*******************************************************************************/
uint8 ul_memoryCopy(uint8 * destStr, const uint8 * srcStr, uint16 length)
{
  uint16 i;
  
  for (i = 0; i < length; i++)
  {
    *destStr++ = *srcStr++;
  }
  
  return 0; //�ִ���ͬ�򷵻�0  
}

/**************************************************************************************************
Copyright 2014 Newbit Studio. All rights reserved.
**************************************************************************************************/



