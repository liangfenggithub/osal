/**************************************************************************************************
Filename:       FS_LowLevel.c
Editor:         Tome @ newbit
Revised:        $Date: 2014-4-20 11:20:02 +0800  $
Revision:       $Revision: 00001 $

Description:    
History:        
Notes:          

**************************************************************************************************/



/**************************************************************************************************
// INCLUDES
**************************************************************************************************/
#include "FS_Lowlevel.h"
#include "hal_eflash.h"



/**************************************************************************************************
// TYPEDEF
**************************************************************************************************/



/**************************************************************************************************
// CONSTANTS
**************************************************************************************************/



/**************************************************************************************************
// LOCAL VERIABLE
**************************************************************************************************/
static int8 gu8_fileNumber = 0;
static uint32 gu32_flashFirstAvailableAddress = 0x1000;//  /256 = pages
static uint32 gu32_0sectoerAvailableAddr  = 0;



static uint8 gb_fileIsOpen = FALSE;
static uint8 gu8_fileInOperation = FO_FAILURE_UNKNOWN;
static uint32 gu32_fileWriteAreaStart = 0;  // where to start storage the file
static uint32 gu32_fileWriteAreaEnd = 0;    // the pos of the rear of the file
static uint32 gu32_fileHasWritedBytes = 0;  // 


static uint8 gu8_fdAccessing  = FO_FAILURE_UNKNOWN;


static const char* const_BPB_OEMName = "MSWIN4.1";


/**************************************************************************************************
// FUNCTIONS DECLERATION
**************************************************************************************************/
uint8  FSL_Inital(void);


/**************************************************************************************************
// FUNCTIONS
**************************************************************************************************/
/**************************************************************************************************
* @fn      FSL_formatVolume
*
* @brief   Format whole volume
*
* @param   void
*
* @return  uint8
**************************************************************************************************/
uint8 FSL_formatVolume(void)
{
  BPBStruct_t bpb;
   halEraseeFlash();

   uint8 boot[3] = {02, 05, 00};
   osal_memcpy(bpb.JumpBoot, boot, 3);
   osal_memcpy(bpb.OEMName, const_BPB_OEMName, BPB_OEM_NAME_LEN);
   // now , EM25P20
   bpb.BytesPerPage = 256;
   bpb.PagesPerSubsector = 16;
   bpb.SubsPerSector = 16;
   bpb.SectorPerClus = 4;
   bpb.MediaType = FLASH_TYPE_NOR;
   const char* model = "EM25P20 ";
   osal_memcpy(bpb.MediaModel, model, 8);
   
   //halWriteeFlash(BPB_BASE_ADDRESS, (void*)&bpb, sizeof(bpb));
   
   return SUCCESS;
}


/**************************************************************************************************
* @fn      fsl_nextFileDirectory
*
* @brief   find the address of next file direcotry
*
* @param   uint8 oldfd,uint32* addr
*
* @return  uint8
**************************************************************************************************/
uint8 fsl_nextFileDirectory(uint8 oldfd, uint32* addr)
{
  return 0;
}


/**************************************************************************************************
* @fn      fsl_nextFileDirectory
*
* @brief   find the address of first file direcotry
*
* @param   uint32* addr
*
* @return  uint8
**************************************************************************************************/
uint8 fsl_fisrtFileDirectory(uint8 oldfd, uint32* addr)
{
  uint8 readBuffer[15];
  uint32 ad = BPB_BASE_ADDRESS + sizeof(BPBStruct_t);
  halReadeFlash(ad, readBuffer, 13);
  if( readBuffer[FD_VALID_OFFSET] == FLASH_NON_WRITED )
  {
    return FO_NO_IMAGE;
  }
  else if ( readBuffer[FD_VALID_OFFSET] == FD_FILE_INCOMPLETE )
  {
    return FO_IMAGE_INVALID;
  }
  else if ( readBuffer[FD_VALID_OFFSET] == FD_FILE_VALID )
  {
    if ( addr )
      *addr = ad;
    return 0; // fd
  }
  else
  {
    return FO_IMAGE_INVALID;
  }
    
}


/**************************************************************************************************
* @fn      FSL_Inital
*
* @brief   call to check it this volume is format and any image ro file had writed into
*
* @param   void
*
* @return  uint8 
**************************************************************************************************/
uint8  FSL_Inital(void)
{
  hal_eFlashInit();
  
  return  0;
  
  uint8 readBuffer[15];
  
  gu32_0sectoerAvailableAddr = BPB_SIZE;
  
  halReadeFlash(BPB_OEM_NAME_ADDR, readBuffer, BPB_OEM_NAME_LEN);
  if ( !osal_memcmp(readBuffer, const_BPB_OEMName, BPB_OEM_NAME_LEN))
  {
    // this volume is brand-new one, so format it first
    FSL_formatVolume();
    return SUCCESS;
  }
  
  // if volume is formated before...
  // how many files location on it now?
  uint8 noexit = 1;
  uint32 addr = BPB_BASE_ADDRESS + sizeof(BPBStruct_t);
  while(noexit)
  {
    halReadeFlash(addr, readBuffer, 13);
    
    if( readBuffer[FD_VALID_OFFSET] == FLASH_NON_WRITED )
    {
      noexit = 0;
    }
    else
    {
      if( readBuffer[FD_VALID_OFFSET] == FD_FILE_VALID )
      {
        gu8_fileNumber ++;
      }
      addr += FD_HEAD_LEN_TOTAL(readBuffer);
    }
  }
  gu32_0sectoerAvailableAddr = addr;
  
  return SUCCESS;
}

/**************************************************************************************************
* @fn      FSL_0sectorAvailableAddr
*
* @brief   文件头可以写入的下一个位置
*
* @param   void
*
* @return  uint32
**************************************************************************************************/
uint32 FSL_0sectorAvailableAddr(void)
{
  return gu32_0sectoerAvailableAddr;
}


/**************************************************************************************************
* @fn      FSL_FileClose
*
* @brief   
*
* @param   uint8 fd
*
* @return  uint8
**************************************************************************************************/
uint8 FSL_FileClose(uint8 fd)
{
  if ( fd >= FO_ERR_CODE )
    return FO_TARGET_ERROR;
  
  if ( gu8_fdAccessing == FO_FAILURE_UNKNOWN )
    return FO_DULPICATE_OPERATION;
  
  if (fd == gu8_fdAccessing)
  {  
    gu8_fdAccessing = FO_FAILURE_UNKNOWN;
  }
  else
  {
    return FO_TARGET_ERROR;
  }
  
  return FO_SUCCESS;
}



/**************************************************************************************************
* @fn      FSL_FileExsit
*
* @brief   当前存放在flash文件的数量
*
* @param   void
*
* @return  uint8
**************************************************************************************************/
uint8 FSL_FileExsit(void)
{
  return gu8_fileNumber;
}


/**************************************************************************************************
* @fn      FSL_FileInOperation
*
* @brief   return the file hander number in operating
*
* @param   void
*
* @return  uint8
**************************************************************************************************/
uint8 FSL_FileInOperation(void)
{
  return gu8_fileInOperation;
}







/**************************************************************************************************
* @fn      FSL_WriteHeader
*
* @brief   write a file directory into flash
*
* @param   
*
* @return  uint8
**************************************************************************************************/
uint8   FSL_WriteHeader(uint8 *buffer, uint8 size)
{
  if ( gb_fileIsOpen )
    return FO_ANOTHER_IN_OPERATING;
  
  uint8 fd = gu8_fileNumber;
  
  uint32 addr = gu32_0sectoerAvailableAddr;
  
  halWriteeFlash(addr, buffer,  size);
  
  gu32_0sectoerAvailableAddr += size;
  gu8_fileNumber ++;
  
  uint32 imageTotalSize = 
    BUILD_UINT32(buffer[FD_IM_TOTAL_SIZE_OFFSET],
    buffer[FD_IM_TOTAL_SIZE_OFFSET+1],
    buffer[FD_IM_TOTAL_SIZE_OFFSET+2],
    buffer[FD_IM_TOTAL_SIZE_OFFSET+3]);

  gb_fileIsOpen = TRUE;
  gu32_fileWriteAreaStart = gu32_flashFirstAvailableAddress;
  gu32_fileWriteAreaEnd = gu32_flashFirstAvailableAddress
    + imageTotalSize;
  gu32_fileHasWritedBytes = 0;

  return fd;
}


/**************************************************************************************************
* @fn      FSL_writeImage
*
* @brief   write a part of file into flash
*
* @param   uint8 fd, uint8 *buffer, uint16 len
*
* @return  uint8
**************************************************************************************************/
uint8 FSL_writeImage(uint8 fd, uint8 *buffer, uint16 len)
{
  if ( !gb_fileIsOpen )
    return FO_OPERATION_DISABLE;
  
  if ( fd != gu8_fileInOperation )
    return FO_TARGET_ERROR;
  
  
  uint32 addr = gu32_fileWriteAreaStart 
    + gu32_fileHasWritedBytes;
  
  if ( (addr + len) > gu32_fileWriteAreaEnd )
  {
    len = gu32_fileWriteAreaEnd - addr;
  }
  
  halWriteeFlash(addr, buffer,  len);
  
  gu32_fileHasWritedBytes += len;
  
  return fd;
}


/**************************************************************************************************
* @fn      FSL_writeComplated
*
* @brief   sign this file is valid, and do something more...
*
* @param   (uint8 fd)
*
* @return  uint8
**************************************************************************************************/
uint8 FSL_writeComplated(uint8 fd)
{
  if ( !gb_fileIsOpen )
    return FO_OPERATION_DISABLE;
  
  if ( fd != gu8_fileInOperation )
    return FO_TARGET_ERROR;
  
  if ( gu32_fileWriteAreaStart + gu32_fileHasWritedBytes
       == gu32_fileWriteAreaEnd )
  {
    
    
    /* check the content if necessary */
    return FO_SUCCESS;
  }
  else
  {
    return 0;
  }
  
}





/**************************************************************************************************
Copyright 2014 Newbit Studio. All rights reserved.
**************************************************************************************************/



