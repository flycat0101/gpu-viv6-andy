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


#include "gc_gl_context.h"
#include "gc_gl_debug.h"


#if defined(_WIN32)
#pragma warning(disable: 4244)
#endif


#define __glIndexf(r)

/* OpenGL index color APIs, but now color index does not support */

GLvoid APIENTRY __glim_Indexd(GLdouble c)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Indexd", DT_GLdouble, c, DT_GLnull);
#endif
    __glIndexf((GLfloat)c);
}

GLvoid APIENTRY __glim_Indexf(GLfloat c)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Indexf", DT_GLfloat, c, DT_GLnull);
#endif
    __glIndexf(c);
}

GLvoid APIENTRY __glim_Indexi(GLint c)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Indexi", DT_GLint, c, DT_GLnull);
#endif

    __glIndexf((GLfloat)c);
}

GLvoid APIENTRY __glim_Indexs(GLshort c)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Indexs", DT_GLshort, c, DT_GLnull);
#endif

    __glIndexf((GLfloat)c);
}

GLvoid APIENTRY __glim_Indexub(GLubyte c)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Indexub", DT_GLubyte, c, DT_GLnull);
#endif
    __glIndexf((GLfloat)c);
}

GLvoid APIENTRY __glim_Indexdv(const GLdouble *c)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Indexdv", DT_GLdouble_ptr, c, DT_GLnull);
#endif

    __glIndexf((GLfloat)c[0]);
}

GLvoid APIENTRY __glim_Indexfv(const GLfloat *c)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Indexfv", DT_GLfloat_ptr, c, DT_GLnull);
#endif

    __glIndexf((GLfloat)c[0]);
}

GLvoid APIENTRY __glim_Indexiv(const GLint *c)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Indexiv", DT_GLint_ptr, c, DT_GLnull);
#endif

    __glIndexf((GLfloat)c[0]);
}

GLvoid APIENTRY __glim_Indexsv(const GLshort *c)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Indexsv", DT_GLshort_ptr, c, DT_GLnull);
#endif

    __glIndexf((GLfloat)c[0]);
}

GLvoid APIENTRY __glim_Indexubv(const GLubyte *c)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Indexubv", DT_GLubyte_ptr, c, DT_GLnull);
#endif

    __glIndexf((GLfloat)c[0]);
}

#if defined(_WIN32)
#pragma warning(default: 4244)
#endif
