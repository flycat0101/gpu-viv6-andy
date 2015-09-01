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


#ifndef _LINUX_

/* X86 OpenGL dispatch function */

#include "gc_gl_context.h"
#include "g_disp.h"
#include "glapioffsets.h"
#include "g_asmoff.h"

#ifndef _WIN64

#define SZPTR (4)
#define SZINT (4)
#define __GL_GET_DISPATCH_TABLE()                           \
        __GL_GET_TLSCX_VALUE()                              \
        __asm add eax, DWORD PTR [eax + __GL_CURRENT_DISPATCH_OFFSET]

#else /*_WIN64*/

#define SZPTR (8)
#define SZINT (8)
#define __GL_GET_DISPATCH_TABLE \
    ((struct __GLdispatchTableRec *)(__GL_GET_TLSCX_VALUE() + (*(DWORD *)(__GL_GET_TLSCX_VALUE() + __GL_CURRENT_DISPATCH_OFFSET))))

#endif /*ifndef _WIN64*/

#if defined(_WIN32)
#pragma warning (disable : 4273 )
#pragma warning (disable : 4035 )
#pragma warning (disable : 4409 )
#pragma warning (disable : 4410 )
#endif

#if defined (_X86_)

#include "g_api_x86.c"

#else  /*_X86_*/

#define __GLVIV_FUNC(func) __glVIV_##func
#include "g_api_amd64.c"

#endif

#if defined(_WIN32)
#pragma warning (default : 4273 )
#pragma warning (default : 4035 )
#pragma warning (default : 4409 )
#pragma warning (default : 4410 )
#endif


/***********************************************************************************************/
/*
** Vivante OpenGL API entry function table which is used by MS opengl32.dll to dispatch OpenGL APIs.
*/
__GLdispatchState __glVIV_DispatchFuncTable = {
    /*
    ** number of entries
    */
    OPENGL_VERSION_110_ENTRIES,

    /*
    ** dispatch
    */
    {
        __GL_API_ENTRIES(glVIV)
    }
};

/***********************************************************************************************/

#endif /* _LINUX_ */
