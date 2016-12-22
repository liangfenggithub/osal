/**************************************************************************************************
Filename:       FS_HighLevel.c
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
#include "FS_HighLevel.h"
#include "hal_eflash.h"
#include "osal.h"

#include "ul_crc.h"


/**************************************************************************************************
// TYPEDEF
**************************************************************************************************/
#define FD uint8 //debug usage


/**************************************************************************************************
// MICRO
**************************************************************************************************/
#define FD_SIZE(filed)  sizeof(nf->filed)

#define FD_COPY(filed)  osal_memcpy(newfd, (void*)&nf->filed, FD_SIZE(filed)); \
                        newfd += FD_SIZE(filed)

#define FD_COPY_POINTER(filed, size) osal_memcpy(newfd, (void*)nf->filed, size); \
                        newfd += size

/**************************************************************************************************
// CONSTANTS
**************************************************************************************************/



/**************************************************************************************************
// LOCAL VERIABLE
**************************************************************************************************/



/**************************************************************************************************
// FUNCTIONS DECLERATION
**************************************************************************************************/



/**************************************************************************************************
// FUNCTIONS
**************************************************************************************************/
/**************************************************************************************************
* @fn      FileCreate
*
* @brief   Create a file header and allocate space for this file
*
* @param   FSNew_t  *nf
*
* @return  uint8
**************************************************************************************************/
uint8 FileCreate(FSNew_t  *nf)
{
  uint8 *newfd = NULL;
  uint8 *p = NULL;
  uint16 len = sizeof(FileDirecotryStruct_t);
  len += nf->WPCount;
  len += nf->ConfigDataBytes;
  newfd = osal_mem_alloc( len );
  
  if ( newfd == NULL )
    return BUFFER_NOT_AVAIL;
  
  p = newfd;
  
  uint16 *pHeadLength = 0;
  
  *newfd++ = FD_FILE_INCOMPLETE;   // new file is missing, this is just file directory
  FD_COPY(FileIdentifier);
  FD_COPY_POINTER(FileVersion, 4);
  FD_COPY(ImageType);
  
  pHeadLength = (uint16*)newfd;
  *pHeadLength = len;// - 13;
  newfd += 2;
  
  FD_COPY(ImageTotalSize);
  
  *(uint16*)newfd = portZERO_SECTER_COVER_PAGES;  //ImageStorePagesStart
  newfd += 2;
  
  uint16 pages = nf->ImageTotalSize / 256;
  if ( nf->ImageTotalSize / 256 )
    pages ++;
  
  *(uint16*)newfd = portZERO_SECTER_COVER_PAGES + pages;  //ImageStorePagesEnd
  newfd += 2;

  FD_COPY( Target );
  FD_COPY( TargetFlashMin );
  FD_COPY( TargetRamMin );
  FD_COPY( FileChecksum );
  FD_COPY( ManuCode );
  FD_COPY( DebugLock );
  FD_COPY( Trial );
  FD_COPY( ProgramInfo );
  FD_COPY( WriteProtected );
  FD_COPY( Security );
  if ( nf->Security == 0)
    *pHeadLength -= 9;
  FD_COPY( AddressAppended );
  FD_COPY( ConfigData );
  FD_COPY( TrialTimeLimit );
  if ( nf-> ProgramInfo )
  {
    newfd += 4; // TragetBankSize
    newfd += 4; // TargetAddressStart
  }
  else
  {
    //*pHeadLength -= 8;
    len -= 8;
  }
  
  if ( nf->WriteProtected )
  {
    FD_COPY( WPCount );
    FD_COPY_POINTER( pWPPages, nf->WPCount );
  }
  else
  {
    //*pHeadLength -= 1;
    len --;
  }
  
  if ( nf->Security )
  {
    FD_COPY( SecurityMode );
    FD_COPY_POINTER( PublicKey, 8 );
  }
  else
  {
    //*pHeadLength -= 9;
    len -= 9;
  }
  
  if ( nf->AddressAppended )
  {
    FD_COPY( AddressAreaStart );
    FD_COPY_POINTER( DefaultAddress, 8 );
  }
  else
  {
    //*pHeadLength -= 12;
    len -= 12;
  }
  
  
  if ( nf->ConfigData )
  {
    FD_COPY( ConfigAreaStart );  
    FD_COPY( ConfigDataBytes );
    FD_COPY_POINTER( pConfigDataField, nf->ConfigDataBytes );
  }
  else
  {
    len -= 5;
  }
  *pHeadLength = len - 13;
   
  // header checksum
  
  // WRITE into flash
  
  FSL_WriteHeader( p,  len);

  osal_mem_free( newfd );
  return SUCCESS;
  
}


/**************************************************************************************************
* @fn      FileHandleCreate(FSNew_t *nf)
*
* @brief   
*
* @param   
*
* @return  uint8
**************************************************************************************************/
uint8 FileHandleCreate(FSNew_t *nf)
{
  osal_memset( (void*)nf, 0xff ,sizeof(FSNew_t));
  nf->FileVersion[0] = 0;
  nf->FileVersion[1] = 1;
  nf->FileVersion[2] = 0;
  nf->FileVersion[3] = 1;
  
  nf->ImageType = 1;  // embeded fw
  nf->Target = 0x01;
  nf->TargetFlashMin = 32;
  nf->TargetRamMin = 4;
  
  nf->ManuCode = 0x00000008; //new bit studio
  nf->DebugLock = 0x0F0;    // non-zero
  nf->Trial = 0x00; 
  nf->ProgramInfo = 0;
  nf->WriteProtected = 0;
  nf->Security = 0;
  nf->AddressAppended = 0;
  nf->ConfigData = 0;
  
  return 0;
}






/**************************************************************************************************
* @fn      FileWriteAppend
*
* @brief   Write data to the end of file
*
* @param   uint8 fileHandle, uint16 dataLength, uint8* data
*
* @return  uint8
**************************************************************************************************/
uint8 FileWriteAppend(uint8 fileHandle, uint16 dataLength, uint8* data)
{
  return 0;
}


/**************************************************************************************************
* @fn      FileWrite
*
* @brief   write data to the destinate file at offset location
*
* @param   uint8 fileHandle, uint16 dataLength, uint8*data, uint32 offset
*
* @return  uint8
**************************************************************************************************/
uint8 FileWrite(uint8 fileHandle, uint16 dataLength, uint8*data, uint32 offset)
{
  return 0;  
}
                 
/**************************************************************************************************
* @fn      FileRead
*
* @brief   Read the next block from input file
*
* @param   uint8 fileHandle, uint16* readLength, uint8* buffer
*
* @return  uint8
**************************************************************************************************/
uint8 FileRead(uint8 fileHandle, uint16* readLength, uint8* buffer)
{
  return 0;  
}


/**************************************************************************************************
* @fn      FileReadPeek
*
* @brief   read a block,but the pointer dosen't move
*
* @param   uint8 fileHandle, uint16* readLength, uint8* buffer
*
* @return  uint8
**************************************************************************************************/
uint8 FileReadPeek(uint8 fileHandle, uint16* readLength, uint8* buffer)
{
  return 0;
}


/**************************************************************************************************
* @fn      FileDirectory
*
* @brief   
*
* @param   FD* list
*
* @return  uint8
**************************************************************************************************/
uint8 FileDirectory(FD* list)
{
  return 0;
}

/**************************************************************************************************
* @fn      FileDelete
*
* @brief   
*
* @param   uint8 fileHandle
*
* @return  uint8
**************************************************************************************************/
uint8 FileDelete(uint8 fileHandle)
{
  return 0;
}


/**************************************************************************************************
* @fn      FileRegisterTask
*
* @brief   
*
* @param   uint8 task_id
*
* @return  uint8
**************************************************************************************************/
uint8 FileRegisterTask(uint8 task_id)
{
  return 0;
}


/**************************************************************************************************
* @fn      FileMovePointer
*
* @brief   
*
* @param   uint8 fileHandle, uint32 offset
*
* @return  uint8
**************************************************************************************************/
uint8 FileMovePointer(uint8 fileHandle, uint32 offset)
{
  return 0;
}

/**************************************************************************************************
* @fn      FSL_Dbg_Write
*
* @brief   write a dummy file header into flash
*
* @param   void
*
* @return  uint8
**************************************************************************************************/
uint8 FSL_Dbg_Write(void)
{
  FSNew_t fsn;
  FileHandleCreate( &fsn );
  FileCreate(&fsn );
  
  
  
  return SUCCESS;
}


/**************************************************************************************************
Copyright 2014 Newbit Studio. All rights reserved.
**************************************************************************************************/



