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


#include "gc_es_context.h"
#include "gc_gl_debug.h"


#if defined(_WIN32)
#pragma warning(disable: 4244)
#endif


#define __glIndexf(gc, r)

/* OpenGL index color APIs, but now color index does not support */

GLvoid APIENTRY __glim_Indexd(__GLcontext *gc, GLdouble c)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Indexd", DT_GLdouble, c, DT_GLnull);
#endif
    __glIndexf(gc, (GLfloat)c);
}

GLvoid APIENTRY __glim_Indexf(__GLcontext *gc, GLfloat c)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Indexf", DT_GLfloat, c, DT_GLnull);
#endif
    __glIndexf(gc, c);
}

GLvoid APIENTRY __glim_Indexi(__GLcontext *gc, GLint c)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Indexi", DT_GLint, c, DT_GLnull);
#endif

    __glIndexf(gc, (GLfloat)c);
}

GLvoid APIENTRY __glim_Indexs(__GLcontext *gc, GLshort c)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Indexs", DT_GLshort, c, DT_GLnull);
#endif

    __glIndexf(gc, (GLfloat)c);
}

GLvoid APIENTRY __glim_Indexub(__GLcontext *gc, GLubyte c)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Indexub", DT_GLubyte, c, DT_GLnull);
#endif
    __glIndexf(gc, (GLfloat)c);
}

GLvoid APIENTRY __glim_Indexdv(__GLcontext *gc, const GLdouble *c)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Indexdv", DT_GLdouble_ptr, c, DT_GLnull);
#endif

    __glIndexf(gc, (GLfloat)c[0]);
}

GLvoid APIENTRY __glim_Indexfv(__GLcontext *gc, const GLfloat *c)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Indexfv", DT_GLfloat_ptr, c, DT_GLnull);
#endif

    __glIndexf(gc, (GLfloat)c[0]);
}

GLvoid APIENTRY __glim_Indexiv(__GLcontext *gc, const GLint *c)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Indexiv", DT_GLint_ptr, c, DT_GLnull);
#endif

    __glIndexf(gc, (GLfloat)c[0]);
}

GLvoid APIENTRY __glim_Indexsv(__GLcontext *gc, const GLshort *c)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Indexsv", DT_GLshort_ptr, c, DT_GLnull);
#endif

    __glIndexf(gc, (GLfloat)c[0]);
}

GLvoid APIENTRY __glim_Indexubv(__GLcontext *gc, const GLubyte *c)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Indexubv", DT_GLubyte_ptr, c, DT_GLnull);
#endif

    __glIndexf(gc, (GLfloat)c[0]);
}

#if defined(_WIN32)
#pragma warning(default: 4244)
#endif
