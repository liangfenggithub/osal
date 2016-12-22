/**************************************************************************************************
Filename:       FS_lowlevel.h
Editor:         Tome @ newbit
Revised:        $Date: 2014-5-8 11:20:02 +0800  $
Revision:       $Revision: 00001 $

Description:    
History:        
Notes:          



**************************************************************************************************/



/**************************************************************************************************
// INCLUDES
**************************************************************************************************/
#include "hal_types.h"


#ifndef FS_LOW_LVL_H
#define FS_LOW_LVL_H


/**************************************************************************************************
// TYPEDEF
**************************************************************************************************/
typedef struct
{
   uint8 JumpBoot[3];
   uint8 OEMName[8];
   uint16 BytesPerPage;
   uint16 PagesPerSubsector;
   uint16 SubsPerSector;
   uint16 SectorPerClus;  
   uint8 MediaType;
   uint8 MediaModel[8];
}BPBStruct_t;
#define BPB_BASE_ADDRESS   0
#define BBP_JUMPBOOT_ADDR        BPB_BASE_ADDRESS
#define BPB_JUMPBOOT_LEN    3
#define BPB_OEM_NAME_ADDR    (BBP_JUMPBOOT_ADDR + BPB_JUMPBOOT_LEN) 
#define BPB_OEM_NAME_LEN    8

#define BPB_SIZE   sizeof(BPBStruct_t)


typedef struct
{
  uint8   FileValid;  
  uint32  FileIdentifier;
  uint8   FileVersion[4];
  uint16  ImageType;
  uint16  HeaderLength;
  uint32  ImageTotalSize;
  uint16  ImageStorePagesStart;
  uint16  ImageStorePagesEnd;
  uint8   Target;
  uint16  TargetFlashMin;
  uint16  TargetRamMin;
  uint16  FileChecksum;
  uint32  ManuCode;
  uint8   DebugLock;
  uint8   Trial;
  uint8   ProgramInfo;
  uint8   WriteProtected;
  uint8   Security;
  uint8   AddressAppended;
  uint8   ConfigData;
  uint32  TrialTimeLimit;
  uint32  TargetBankSize;
  uint32  TargetAddressStart;
  uint16  WPCount;
  uint8   *pWPPages;
  uint8   SecurityMode;
  uint8   PublicKey[8];
  uint32  AddressAreaStart;
  uint8   DefaultAddress[8];
  uint32  ConfigAreaStart;
  uint16  ConfigDataBytes;
  uint8  *pConfigDataField;
  uint16  HeaderChecksum;
}FileDirecotryStruct_t;

#define FD_VALID_OFFSET     0
#define FD_HEAD_LEN_OFFSET  11
#define FD_IM_TOTAL_SIZE_OFFSET 13

#define FD_HEAD_LEN_TOTAL(x)  ( *((uint16*)(x+FD_HEAD_LEN_OFFSET)) + 13)


/**************************************************************************************************
// CONSTANTS
**************************************************************************************************/
#define BPB_SECTORS_IN_VOL    4
#define BPB_SUBS_IN_SECTOR     16
#define BPB_PAGES_IN_SUB      16
#define BPB_BYTES_IN_PAGE     256


#define FD_FILE_VALID       0xA5
#define FD_FILE_INCOMPLETE  0xAF
#define FLASH_NON_WRITED  0xFF

#define FLASH_TYPE_NOR    0xF0
#define FLASH_TYPE_NAN    0xF1

/**************************************************************************************************
// GLOABAL VERIABLE
**************************************************************************************************/



/**************************************************************************************************
// MICROS
**************************************************************************************************/
// #define MAX((a),(b)) (a)>(b)?(a):(b)

#define FO_SUCCESS  0
#define FO_FAILURE  1

#define FO_ERR_CODE               0xF0
#define FO_ANOTHER_IN_OPERATING   0xF0
#define FO_OPERATION_DISABLE      0xF1
#define FO_DULPICATE_WRITED       0xF2
#define FO_TARGET_ERROR           0xF3
#define FO_DULPICATE_OPERATION    0xF4
#define FO_NO_IMAGE               0xF5
#define FO_IMAGE_INVALID          0xF6

#define FO_FAILURE_UNKNOWN        0xFF



/**************************************************************************************************
// FUNCTIONS
**************************************************************************************************/
uint8 FSL_Dbg_Write(void);

uint32 FSL_0sectorAvailableAddr(void);
uint8 FSL_formatVolume(void);
uint8  FSL_Inital(void);

uint8 FSL_WriteHeader(uint8 *buffer, uint8 size);

#endif // FS_LOW_LVL_H

/**************************************************************************************************
Copyright 2014 Newbit Studio. All rights reserved.
**************************************************************************************************/





