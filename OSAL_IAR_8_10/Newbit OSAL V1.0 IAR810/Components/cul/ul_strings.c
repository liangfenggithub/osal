/**************************************************************************************************
Filename:       ul_strings.c
Editor:         Tome @ newbit
Revised:        $Date: 2014-4-22 21:20:02 +0800  $
Revision:       $Revision: 00001 $

Description:    
History:        
Notes:          

**************************************************************************************************/


/*----------------------------------头文件----------------------------------*/
#include "hal_defs.h"
#include "hal_types.h"

/*----------------------------------本地函数----------------------------------*/
uint8 * ul_searchChar(uint8  *str, uint8 len, char ch);
void ul_stringUpCase(uint8  *str, uint8 len);

/*******************************************************************************
 * @fn          ul_sprintstr()
 *
 * @brief       将数组打印为１６进制串,如果 arr[2] = {0xab,0xcd}打印为ABCD
 *              
 * Parameters:
 *
 * @param       uint8  *dest - 打印结果存放缓存
 * @param       uint8  *src - 待打印字串
 * @param       uint8  len - 打印长度
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
    //高字节，高位在先
    //如0XABCD,打印为"ABCD"
    //temp = *(scr+len-u8i);
    temp = *(src+u8i);
    temp = (temp>>4)&0x0f;
    if(temp<10)temp += 0x30;
    else
      temp += 0x37;
    *(dest+2*u8i) = temp;
    /**********************************************************************/
    //字节低位在后
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
 * @brief       将字打印为字串
 *              
 * Parameters:
 *
 * @param       uint8  *dest - 打印结果存放缓存
 * @param       uint8  n - 待打印字
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
 * @brief       将字打印为字串
 *              
 * Parameters:
 *
 * @param       uint8  *dest - 打印结果存放缓存
 * @param       uint8  n - 待打印字
 *
 * @return      none
 * 
*******************************************************************************/
void ul_sprintUint16ToDec(uint8  *dest, uint16 n)
{
  uint8  tmp;
  uint8  z_space = 1;//置1表示将0打印为空格
  
  /*万位*/
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
  
  /*千位*/
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
  
  /*百位*/
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
  
  /*十位*/
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
  
  /*个位*/
  tmp = (n % 10);
  *dest++ = tmp + '0';
  
  *dest = '\n';  //共计6uint8 s
}



/*******************************************************************************
 * @fn          ul_searchChar
 *
 * @brief       在字串里搜索字符第一次出现的位置
 *              
 * Parameters:
 *
 * @param       uint8  *str - 用于搜索的字串
 * @param       uint8 len - 字串长度,限定搜索范围
 * @param       char ch - 搜索的字符
 *
 * @return      uint8* ch所在地址，如果没有找到，返回NULL
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
 * @brief       将字串中的小写字母改为大写
 *              
 * Parameters:
 *
 * @param       uint8  *str - 源字串
 * @param       uint8 len - 字串长度
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
  * @brief      将字串中的大写字母改为小写
 *              
 * Parameters:
 *
 * @param       uint8  *str - 源字串
 * @param       uint8 len - 字串长度
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
 * @brief       提取字串所显示的数字,如"123"->123,如果字串长于4,只取前4个数字
 *              
 * Parameters:
 *
 * @param       uint8  *str - 源字串
 * @param       uint8 len - 字串长度
 * @param       uint16 *n - 存放生成结果
 *
 * @return      uint8  成功返回1,失败返回0
 * 
*******************************************************************************/
uint8  ul_string2Uint16(uint8  *str,uint8 len,uint16 *n)
{
  uint8 deal_len;//本次处理的长度,它代表去除左边空格后及右边无效串后的长度
  uint8  *start;
  uint8  data[4];
  uint8 u8i;
  uint8  hex;
  uint16 res = 0x0000;
  
  
  ul_stringUpCase(str,len);
  /*去左边无效数据*/
  start = str;
  u8i = len;
  while (u8i--)
  {
    if (*start == ' ')start++;
    else
      break;
  }
  
  /*确定数据位数,此时start指向空格后的第一字节*/  
  hex = 0;
  deal_len = 0;
  if ((*(start) == '0') && (*(start+1) == 'X'))
  {
    start += 2;
    hex = 1;                               //16进制的数
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
  
  /*检查数据中否可提取数据*/
  if (deal_len == 0)
  {
    return FALSE;
  }
  
  /*计算结果*/
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
 * @brief       比较两段内存是否一样
 *              
 * Parameters:
 *
 * @param       uint8  *str1 - 字串1
 * @param       uint8  *str2 - 字串2
 * @param       uint16 length - 比较的长度
 *
 * @return      相等返回0，不等则返回指针
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
  
  return 0; //字串相同则返回0  
}

/*******************************************************************************
 * @fn          ul_memorySet(uint8 * str, uint8  ch, uint16 length)
 *
 * @brief       将某段内存设置为连续相同的值
 *              
 * Parameters:
 *
 * @param       uint8  *str1 - 字串1
 *              uint8  ch - 填充的内容
 *              uint16 length - 填充的长度
 *
 * @return      总是返回0
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
 * @brief       将srcStr指针的内存内容复制到destStr,复制长度由length指定
 *              
 * Parameters:
 *
 * @param       uint8  *str1 - 字串1
 * @param       uint8  *str2 - 字串2
 * @param       uint16 length - 比较的长度
 *
 * @return      相等返回0，不等则返回指针
 * 
*******************************************************************************/
uint8 ul_memoryCopy(uint8 * destStr, const uint8 * srcStr, uint16 length)
{
  uint16 i;
  
  for (i = 0; i < length; i++)
  {
    *destStr++ = *srcStr++;
  }
  
  return 0; //字串相同则返回0  
}

/**************************************************************************************************
Copyright 2014 Newbit Studio. All rights reserved.
**************************************************************************************************/



