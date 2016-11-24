/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifdef OPENGL40
#ifndef __gl_debug_debug_h_
#define __gl_debug_debug_h_

/* include files*/
#include <stdio.h>
#include <assert.h>
#ifdef _LINUX_
#include "stdarg.h"
#define OutputDebugString(szBuff) fprintf(stderr, szBuff)
#define wvsprintf(buff, format, va) sprintf(buff, format, va)
#else
#include <windows.h>
#include <wtypes.h>
#endif
#include "gc_es_context.h"

#if (defined(_DEBUG) || defined(DEBUG))

/*global debug variables*/
EXTERN_C GLvoid*   gdbg_pTargetDrawable;
EXTERN_C GLuint    gdbg_bOnlyAcceptSpecifiedDrawable;

/*counters*/
EXTERN_C GLuint    gdbg_drawCount;
EXTERN_C GLuint    gdbg_rasterCount;
EXTERN_C GLuint    gdbg_drawImmeCount;
EXTERN_C GLuint    gdbg_drawArrayCount;
EXTERN_C GLuint    gdbg_drawDlistCount;
EXTERN_C GLuint    gdbg_frameCount;
EXTERN_C GLuint    gdbg_frameCountTarget;

EXTERN_C GLuint    gdbg_drawTarget;
EXTERN_C GLuint    gdbg_skipTarget;
EXTERN_C GLuint    gdbg_nameTarget;

/*log file*/
EXTERN_C GLuint    gdbg_logFrameDrawCount;
EXTERN_C GLuint    gdbg_logFrameStartIndex;
EXTERN_C GLuint    gdbg_logFrameEndIndex;
EXTERN_C FILE*     gdbg_logFrameFile;

/*debug helper filters*/
#define GL_DBBUG_PRINT_API          0x00000001
#define GL_DBBUG_PRINT_API_PARAM    0x00000002
#define GL_DBBUG_PRINT_API_FLUSH    0x80000000

/*log, msg fitlers*/
EXTERN_C GLuint dbg_logAPIFilter;
EXTERN_C GLuint dbg_bLogAPIToFile;

/*basic message functions*/
EXTERN_C GLvoid dbgPrint(DWORD level, GLbyte *szFormat, ... );
EXTERN_C GLvoid dbgMessage(GLbyte *szFormat, ... );
EXTERN_C GLvoid dbgError(GLbyte *szFormat, ... );

/*basic log functions*/
enum
{
    DT_GLnull           = 0,
    DT_GLenum           = 1,
    DT_GLboolean        = 2,
    DT_GLbitfield       = 3,
    DT_GLbyte           = 4,
    DT_GLshort          = 5,
    DT_GLsizei          = 6,
    DT_GLubyte          = 7,
    DT_GLushort         = 8,
    DT_GLfloat          = 9,
    DT_GLclampf         = 10,
    DT_GLdouble         = 11,
    DT_GLclampd         = 12,
    DT_GLvoid           = 13,
    DT_GLuint64         = 14,
    DT_GLint64          = 15,
    DT_GLint            = 16,
    DT_GLuint           = 17,
    DT_GLenum_ptr       = 101,
    DT_GLboolean_ptr    = 102,
    DT_GLbitfield_ptr   = 103,
    DT_GLbyte_ptr       = 104,
    DT_GLshort_ptr      = 105,
    DT_GLsizei_ptr      = 106,
    DT_GLubyte_ptr      = 107,
    DT_GLushort_ptr     = 108,
    DT_GLfloat_ptr      = 109,
    DT_GLclampf_ptr     = 110,
    DT_GLdouble_ptr     = 111,
    DT_GLclampd_ptr     = 112,
    DT_GLvoid_ptr       = 113,
    DT_GLuint64_ptr     = 114,
    DT_GLint64_ptr      = 115,
    DT_GLint_ptr        = 116,
    DT_GLuint_ptr       = 117,
    DT_FORCEDWORD       = 0xffffffff
};

typedef struct
{
    GLenum          value;
    const GLbyte*   name;
}__GLenumName;

EXTERN_C GLvoid dbgLogFileOpen();
EXTERN_C GLvoid dbgLogFilePrintf(GLbyte *buf);
EXTERN_C GLvoid dbgLogFileFlush();
EXTERN_C GLvoid dbgLogFileClose();

EXTERN_C GLvoid dbgLogFullApi(GLbyte *szAPI, ...);
EXTERN_C GLvoid dbgLogFullApiBegin();
EXTERN_C GLvoid dbgLogFullApiEnd();
EXTERN_C GLvoid dbgLogFullApiRet(GLbyte *szFormat, ... );
EXTERN_C GLvoid dbgWGLLogAPIBegin();
EXTERN_C GLvoid dbgWGLLogAPIEnd();
//EXTERN_C const GLbyte * dbgQueryTexFmt(__GLdeviceFormat uFmt);

/*extended debug helper*/
EXTERN_C GLvoid dbgGCState(__GLcontext* gc);

#else  /*release version of prototype*/

#endif

#endif

#endif