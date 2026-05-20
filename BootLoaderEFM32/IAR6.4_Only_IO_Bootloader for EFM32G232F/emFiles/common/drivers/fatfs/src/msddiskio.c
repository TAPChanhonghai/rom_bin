/*------------------------------------------------------------------------/
/  MMCv3/SDv1/SDv2 (in SPI mode) control module
/-------------------------------------------------------------------------/
/
/  Copyright (C) 2010, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-------------------------------------------------------------------------*/

#include "em_usb.h"
#include "diskio.h"
#include "msdh.h"
#include "RTC_1381.h"

volatile DSTATUS Stat = STA_NOINIT;	/* Disk status */

/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE drv		/* Physical drive nmuber (0) */
)
{
	if (drv) return STA_NOINIT;			/* Supports only single drive */
	if (Stat & STA_NODISK) return Stat;	/* No card in the socket */

	/* Initialization (always) succeded */
	Stat &= ~STA_NOINIT;	/* Clear STA_NOINIT */

	return Stat;
}

/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0) */
)
{
	if (drv) return STA_NOINIT;		/* Supports only single drive */
	return Stat;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0) */
	BYTE *buff,		/* Pointer to the data buffer to store read data */
	DWORD sector,	/* Start sector number (LBA) */
	BYTE count		/* Sector count (1..255) */
)
{
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	return MSDH_ReadSectors( sector, count, buff ) ? RES_OK : RES_ERROR;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,				/* Physical drive nmuber (0) */
	const BYTE *buff,		/* Pointer to the data to be written */
	DWORD sector,			/* Start sector number (LBA) */
	BYTE count				/* Sector count (1..255) */
)
{
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;
	if (Stat & STA_PROTECT) return RES_WRPRT;

	return MSDH_WriteSectors( sector, count, buff ) ? RES_OK : RES_ERROR;
}
#endif /* _READONLY */

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive data block */
)
{
	DRESULT res;

	if (drv) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	res = RES_ERROR;

	switch (ctrl) {
		case CTRL_SYNC :	/* Flush dirty buffer if present */
		  /* Not implemented, should be SCSI cmd SYNC_CACHE. */
		  res = RES_OK;
			break;

		case CTRL_INVALIDATE :	/* Used when unmounting */
		  Stat = STA_NOINIT;	  /* Set disk status */
		  res = RES_OK;
		  break;

		case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
      if ( MSDH_GetSectorCount( buff ) )
      {
        res = RES_OK;
      }
			break;

		case GET_SECTOR_SIZE :	/* Get sectors on the disk (WORD) */
      if ( MSDH_GetSectorSize( buff ) )
      {
        res = RES_OK;
      }
			break;

		case GET_BLOCK_SIZE :	/* Get erase block size in unit of sectors (DWORD) */
		  /* Use sector size */
      if ( MSDH_GetBlockSize( buff ) )
      {
        res = RES_OK;
      }
			break;

		case MMC_GET_TYPE :		/* Get card type flags (1 byte) */
		case MMC_GET_CSD :	/* Receive CSD as a data block (16 bytes) */
		case MMC_GET_CID :	/* Receive CID as a data block (16 bytes) */
		case MMC_GET_OCR :	/* Receive OCR as an R3 resp (4 bytes) */
		case MMC_GET_SDSTAT :	/* Receive SD statsu as a data block (64 bytes) */
		default:
			res = RES_PARERR;
	}

	return res;
}

DWORD get_fattime(void)
{
    DateTime_T rtc;
    DWORD lyear;
    DWORD Out =0;
    RTC_Read_Time(&rtc);
    lyear = rtc.year+2000;
    if(lyear >= 2100)lyear = 2010;
    if(rtc.month > 12)rtc.month = 1;
    if(rtc.day > 12)rtc.day = 1;
    if(rtc.hours > 12)rtc.hours = 1;
    if(rtc.minutes >= 60)rtc.minutes = 0;
    if(rtc.seconds >= 60)rtc.seconds = 0;
    Out |= ((DWORD)(lyear-1980)) << 25;// Year = 2007
    Out |= ((DWORD)rtc.month) << 21;// Month = June
    Out |= ((DWORD)rtc.day) << 16;// Day = 5
    Out |= ((DWORD)rtc.hours) << 11;// Hour = 11
    Out |= ((DWORD)rtc.minutes) << 5;// Min = 38
    Out |= ((DWORD)rtc.seconds) >> 1;// Sec = 0
    return Out;
}
