/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef _WIN2UNIX_H
#define _WIN2UNIX_H

#include <string.h>

// main Windows types
typedef unsigned long       DWORD;
typedef int                 INT;
typedef unsigned int        UINT;
typedef int                 LONG;
typedef unsigned long       ULONGLONG;

typedef unsigned long  *    ULONG_PTR;
typedef LONG                HRESULT;
typedef unsigned int *      UINT_PTR;
#define WINAPI

#ifdef __cplusplus
    #define EXTERN_C    extern "C"
#else
    #define EXTERN_C    extern
#endif

typedef struct tagRECT
{
    LONG    left;
    LONG    top;
    LONG    right;
    LONG    bottom;
} RECT, *PRECT, *LPRECT;

typedef struct _RGNDATAHEADER {
    DWORD   dwSize;
    DWORD   iType;
    DWORD   nCount;
    DWORD   nRgnSize;
    RECT    rcBound;
} RGNDATAHEADER, *PRGNDATAHEADER;

typedef struct _RGNDATA {
    RGNDATAHEADER   rdh;
    GLbyte            Buffer[1];
} RGNDATA, *PRGNDATA, *LPRGNDATA;

#define CDECL

#endif



