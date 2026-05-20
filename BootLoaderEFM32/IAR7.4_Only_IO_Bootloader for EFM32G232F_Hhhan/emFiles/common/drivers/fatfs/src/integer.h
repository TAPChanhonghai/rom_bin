/*-------------------------------------------*/
/* Integer type definitions for FatFs module */
/*-------------------------------------------*/

#ifndef _INTEGER
#define _INTEGER

#ifdef _WIN32	/* FatFs development platform */

#include <windows.h>
#include <tchar.h>

#else			/* Embedded platform */

//#include "stdint.h"

/* These types must be 16-bit, 32-bit or larger integer */
typedef short int 	INT;
typedef unsigned short int	UINT;

/* These types must be 8-bit integer */
typedef char		CHAR;
typedef unsigned char	        UCHAR;
typedef unsigned char	        BYTE;

/* These types must be 16-bit integer */
typedef short int		SHORT;
typedef unsigned short int	USHORT;
typedef unsigned short int	WORD;
typedef unsigned short int	WCHAR;

/* These types must be 32-bit integer */
typedef long int		LONG;
typedef unsigned long int	ULONG;
typedef unsigned long int	DWORD;

#endif

#endif
