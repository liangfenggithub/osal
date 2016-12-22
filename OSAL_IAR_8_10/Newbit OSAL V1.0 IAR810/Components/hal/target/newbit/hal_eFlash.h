/**************************************************************************************************
Filename:       hal_eFlash.h
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
#include "onBoard.h"


#ifndef E_FLASH_H
#define E_FLASH_H


/**************************************************************************************************
// TYPEDEF
**************************************************************************************************/



/**************************************************************************************************
// CONSTANTS
**************************************************************************************************/
#define XNV_STAT_CMD  0x05    //read state register
#define XNV_WRST_CMD  0x01    //write state register
#define XNV_IDEN_CMD  0x9F    //read identification

#define XNV_WRLK_CMD  0xE5    //write to lock register
#define XNV_RDLK_CMD  0xE8    //read lock register

#define XNV_WREN_CMD  0x06    //write enable
#define XNV_WRDE_CMD  0x04    //write disable


#define XNV_WRPG_CMD  0x0A    //page write, It also can change bits 0 to 1
#define XNV_READ_CMD  0x0B    //read bytes at a higher speed
#define XNV_PRPG_CMD  0x02    //page program, it is the fast way to modify data, only change bits 1 to 0

#define XNV_ERPG_CMD  0xDB    //page erase,
#define XNV_ERSS_CMD  0x20    //subsector erase
#define XNV_ERSC_CMD  0xD8    //sector erase
#define XNV_ERBK_CMD  0xC7    //bulk erase

#define XNV_DOWN_CMD  0xB9    //deep power-down
#define XNV_WAKE_CMD  0xAB    //release form deep power-down mode

#define XNV_STAT_WIP  0x01
#define XNV_STAT_WEL  0x02


/**************************************************************************************************
// GLOABAL VERIABLE
**************************************************************************************************/



/**************************************************************************************************
// MICROS
**************************************************************************************************/
// #define MAX((a),(b)) (a)>(b)?(a):(b)



/**************************************************************************************************
// FUNCTIONS
**************************************************************************************************/
void hal_eFlashInit(void);
void halEraseeFlash(void);
void halReadeFlash(uint32 addr, uint8* pbuf, uint16 len);
void halWriteeFlash(uint32 addr, uint8* pBuf, uint16 len);
void halProgrameFlash(uint32 addr,uint8* pBuf, uint16 len);

uint8 halErasePage(uint16 page);
uint8 halEraseSubsector(uint8 sub);
uint8 halEraseSector(uint8 sec);

// DEBUG FUNCTION
void dbgFlashMain(void);

#endif // E_FLASH_H

/**************************************************************************************************
Copyright 2014 Newbit Studio. All rights reserved.
**************************************************************************************************/





