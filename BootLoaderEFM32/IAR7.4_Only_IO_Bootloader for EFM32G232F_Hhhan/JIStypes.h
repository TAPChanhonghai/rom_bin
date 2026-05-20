/****************************************************************************
*
* FILENAME
*     JIStypes.h
*
* VERSION
*     2.0
*
* DESCRIPTION
*     This PreDefined data types for JIS RD
*
* DATA STRUCTURES
*     None
*
* FUNCTIONS
*     None
*
* HISTORY
*     08/10/29       Ver 1.0 Created by Liugt
*			08/12/02       modified by Liugt
* REMARK
*     None
**************************************************************************/

#ifndef _JISTYPES_H_
#define _JISTYPES_H_

#include "string.h"
//#include "integer.h"

typedef unsigned char       UCHAR;
typedef unsigned char       BYTE;

typedef unsigned short      USHORT;
typedef unsigned long       DWORD;
typedef unsigned short      WORD;

typedef char                SCHAR;

typedef void                VOID;
typedef void *              PVOID;

typedef signed char         INT8;
typedef signed char *       PINT8;
typedef unsigned char       UINT8;
typedef unsigned char *     PUINT8;


typedef signed   int*       PINT;

typedef unsigned   int*     PUINT;

typedef signed char *       PCHAR;

typedef unsigned char *     PUCHAR;
typedef signed char *       PSTR;
typedef const signed char * PCSTR;

typedef short               INT16;
typedef short *             PINT16;
typedef unsigned short      UINT16;
typedef unsigned short *    PUINT16;

typedef signed   int        INT;
typedef unsigned int        UINT;
typedef signed   int        INT32;
typedef signed   int *      PINT32;
typedef unsigned int        UINT32;
typedef unsigned int *      PUINT32;



typedef float               FLOAT;
typedef float *             PFLOAT;

typedef double              DOUBLE;
typedef double *            PDOUBLE;



#endif /* _JISTYPES_H */
