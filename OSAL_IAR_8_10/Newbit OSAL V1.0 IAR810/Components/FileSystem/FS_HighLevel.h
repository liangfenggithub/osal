/**************************************************************************************************
Filename:       FS_HighLevel.h.h
Editor:         Tome @ newbit
Revised:        $Date: 2014-6-7 11:20:02 +0800  $
Revision:       $Revision: 00001 $

Description:    
History:        
Notes:          



**************************************************************************************************/



/**************************************************************************************************
// INCLUDES
**************************************************************************************************/
#include "hal_types.h"
#include "FS_Lowlevel.h"


#ifndef FS_HIGH_LEVEL_H
#define FS_HIGH_LEVEL_H


/**************************************************************************************************
// TYPEDEF
**************************************************************************************************/
typedef struct 
{
  uint32 FileIdentifier;
  uint8  FileVersion[4];
  uint16  ImageType;
  uint32 ImageTotalSize;
  uint8  Target;
  uint16 TargetFlashMin;
  uint16 TargetRamMin;
  uint16 FileChecksum;	// and the end of the file
  uint32 ManuCode;
  uint8  DebugLock;
  uint8  Trial;  
  uint8  ProgramInfo;
  uint8 WriteProtected;
  uint8   Security;
  uint8   AddressAppended;
  uint8   ConfigData;
  uint32  TrialTimeLimit;
  //  uint32  TargetBankSize;, depend on  target
  uint32  TargetAddressStart;
  uint16  WPCount;
  uint8*  pWPPages;
  uint8   SecurityMode;
  uint8   PublicKey[8];
  uint32  AddressAreaStart;
  uint8   DefaultAddress[8];
  uint32  ConfigAreaStart;
  uint16  ConfigDataBytes;
  uint8   *pConfigDataField;
}FSNew_t;


/**************************************************************************************************
// CONSTANTS
**************************************************************************************************/
#define portZERO_SECTER_SIZE          4096
#define portZERO_SECTER_COVER_PAGES   16






/**************************************************************************************************
// GLOABAL VERIABLE
**************************************************************************************************/



/**************************************************************************************************
// MICROS
**************************************************************************************************/
// #define MAX((a),(b)) (a)>(b)?(a):(b)
//#define FiledCopy(dest,src,filed) ((dest)->filed) = ((src)->filed)
#define FiledCopy(dest,src,filed) osal_memcpy( &((dest)->filed), &((src)->filed), sizeof((src)->filed))


/**************************************************************************************************
// FUNCTIONS
**************************************************************************************************/
// create a file, should supply many info
uint8 FileCreate(FSNew_t  *nf);

uint8 FileHandleCreate(FSNew_t *nf);


#endif // FS_HIGH_LEVEL_H_H

/**************************************************************************************************
Copyright 2014 Newbit Studio. All rights reserved.
**************************************************************************************************/





