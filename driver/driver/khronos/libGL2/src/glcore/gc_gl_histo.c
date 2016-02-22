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


#include "glcore/gc_gl_context.h"
#include "glcore/gc_gl_debug.h"

/*
** Helper funtion
*/
extern GLint __glElementsPerGroup(GLenum format, GLenum type);
extern GLint __glBytesPerElement(GLenum type);
extern GLvoid __glInitMemGet(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                    GLsizei width, GLsizei height, GLsizei depth,
                    GLenum format, GLenum type, const GLvoid *buf);

extern GLvoid __glInitMemPack(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                     GLenum format, GLenum type, const GLvoid *buf);

extern GLvoid __glGenericPickCopyImage(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                              GLboolean applyPixelTransfer);


static GLboolean AllocHistogramArray(__GLcontext *gc, __GLhistogram *hg, GLboolean isProxy, GLsizei width,
                                     GLenum internalFormat, GLboolean sink)
{
    GLenum baseFormat, type;
    GLint redSize, greenSize, blueSize, alphaSize, luminanceSize;
    GLint bufferSize;

    switch (internalFormat)
    {
    case GL_ALPHA:
    case GL_ALPHA4:
    case GL_ALPHA8:
    case GL_ALPHA12:
    case GL_ALPHA16:
        baseFormat = GL_ALPHA;
        type = GL_UNSIGNED_INT;
        redSize = 0;
        greenSize = 0;
        blueSize = 0;
        alphaSize = sizeof(GLuint) * 8;
        luminanceSize = 0;
        break;
    case GL_LUMINANCE:
    case GL_LUMINANCE4:
    case GL_LUMINANCE8:
    case GL_LUMINANCE12:
    case GL_LUMINANCE16:
        baseFormat = GL_LUMINANCE;
        type = GL_UNSIGNED_INT;
        redSize = 0;
        greenSize = 0;
        blueSize = 0;
        alphaSize = 0;
        luminanceSize = sizeof(GLuint) * 8;
        break;
    case GL_LUMINANCE_ALPHA:
    case GL_LUMINANCE4_ALPHA4:
    case GL_LUMINANCE6_ALPHA2:
    case GL_LUMINANCE8_ALPHA8:
    case GL_LUMINANCE12_ALPHA4:
    case GL_LUMINANCE12_ALPHA12:
    case GL_LUMINANCE16_ALPHA16:
        baseFormat = GL_LUMINANCE_ALPHA;
        type = GL_UNSIGNED_INT;
        redSize = 0;
        greenSize = 0;
        blueSize = 0;
        alphaSize = sizeof(GLuint) * 8;
        luminanceSize = sizeof(GLuint) * 8;
        break;
    case GL_RGB:
    case GL_R3_G3_B2:
    case GL_RGB4:
    case GL_RGB5:
    case GL_RGB8:
    case GL_RGB10:
    case GL_RGB12:
    case GL_RGB16:
        baseFormat = GL_RGB;
        type = GL_UNSIGNED_INT;
        redSize = sizeof(GLuint) * 8;
        greenSize = sizeof(GLuint) * 8;
        blueSize = sizeof(GLuint) * 8;
        alphaSize = 0;
        luminanceSize = 0;
        break;
    case GL_RGBA:
    case GL_RGBA2:
    case GL_RGBA4:
    case GL_RGBA8:
    case GL_RGBA12:
    case GL_RGBA16:
    case GL_RGB5_A1:
    case GL_RGB10_A2:
        baseFormat = GL_RGBA;
        type = GL_UNSIGNED_INT;
        redSize = sizeof(GLuint) * 8;
        greenSize = sizeof(GLuint) * 8;
        blueSize = sizeof(GLuint) * 8;
        alphaSize = sizeof(GLuint) * 8;
        luminanceSize = 0;
        break;
    default:
        GL_ASSERT(0);
        return GL_FALSE;
    }

    bufferSize = width *
        __glElementsPerGroup(baseFormat, type) *
        __glBytesPerElement(type);

    /* Limit histogram array size so that pixel operations can be used */
    if (bufferSize > __GL_MAX_SPAN_SIZE)
    {
        hg->width = 0;
        hg->format = 0;
        hg->formatReturn = 0;
        hg->baseFormat = 0;
        hg->type = 0;
        hg->redSize = 0;
        hg->greenSize = 0;
        hg->blueSize = 0;
        hg->alphaSize = 0;
        hg->luminanceSize = 0;
        if (!isProxy)
        {
            __glSetError(GL_TABLE_TOO_LARGE);
        }
        return GL_FALSE;
    }

    if (!isProxy)
    {
        /* histogram pointer is initialized to zero */
        hg->array = (GLuint *)(*gc->imports.malloc)(gc, bufferSize );
        if (!hg->array && bufferSize > 0)
        {
            __glSetError(GL_OUT_OF_MEMORY);
            return GL_FALSE;
        }
    }

    hg->width = width;
    hg->formatReturn = internalFormat;
    hg->format =
        hg->baseFormat = baseFormat;
    hg->type = type;

    hg->redSize = redSize;
    hg->greenSize = greenSize;
    hg->blueSize = blueSize;
    hg->alphaSize = alphaSize;
    hg->luminanceSize = luminanceSize;

    hg->sink = sink;
    hg->arraySize = bufferSize;
    return GL_TRUE;
}

static __GLhistogram *LookupHistogram(__GLcontext *gc, GLenum target, GLboolean *isProxy)
{
    __GLhistogram *hg;

    switch (target)
    {
    case GL_HISTOGRAM:
        *isProxy = GL_FALSE;
        hg = &gc->state.pixel.histogram;
        break;
    case GL_PROXY_HISTOGRAM:
        *isProxy = GL_TRUE;
        hg = &gc->state.pixel.proxyHistogram;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return NULL;
    }
    return hg;
}

static GLenum CheckHistogramArgs(__GLcontext *gc, GLenum target, GLsizei width,
                                 GLenum internalFormat, GLboolean sink)
{
    switch (target)
    {
    case GL_HISTOGRAM:
    case GL_PROXY_HISTOGRAM:
        break;
    default:
        return GL_INVALID_ENUM;
    }

    switch (internalFormat)
    {
    case GL_ALPHA:
    case GL_ALPHA4:
    case GL_ALPHA8:
    case GL_ALPHA12:
    case GL_ALPHA16:
        break;
    case GL_LUMINANCE:
    case GL_LUMINANCE4:
    case GL_LUMINANCE8:
    case GL_LUMINANCE12:
    case GL_LUMINANCE16:
        break;
    case GL_LUMINANCE_ALPHA:
    case GL_LUMINANCE4_ALPHA4:
    case GL_LUMINANCE6_ALPHA2:
    case GL_LUMINANCE8_ALPHA8:
    case GL_LUMINANCE12_ALPHA4:
    case GL_LUMINANCE12_ALPHA12:
    case GL_LUMINANCE16_ALPHA16:
        break;
    case GL_RGB:
    case GL_R3_G3_B2:
    case GL_RGB4:
    case GL_RGB5:
    case GL_RGB8:
    case GL_RGB10:
    case GL_RGB12:
    case GL_RGB16:
        break;
    case GL_RGBA:
    case GL_RGBA2:
    case GL_RGBA4:
    case GL_RGBA8:
    case GL_RGBA12:
    case GL_RGBA16:
    case GL_RGB5_A1:
    case GL_RGB10_A2:
        break;
    default:
        return GL_INVALID_ENUM;
    }

    if ((width < 0) || (width & (width - 1)))
    {
        return GL_INVALID_VALUE;
    }

    return 0;
}

static GLvoid GetHistogramParameters(GLenum target, GLenum pname, GLvoid *params, GLenum return_type)
{
    __GLhistogram *hg;
    GLboolean isProxy;
    GLint value = 0;
    __GL_SETUP_NOT_IN_BEGIN();

    __GL_VERTEX_BUFFER_FLUSH(gc);

    if ((hg = LookupHistogram(gc, target, &isProxy)) == NULL)
    {
        return;
    }

    switch (pname)
    {
    case GL_HISTOGRAM_WIDTH:
        value = (GLint)hg->width;
        break;
    case GL_HISTOGRAM_FORMAT:
        value = (GLint)hg->formatReturn;
        break;
    case GL_HISTOGRAM_RED_SIZE:
        value = (GLint)hg->redSize;
        break;
    case GL_HISTOGRAM_GREEN_SIZE:
        value = (GLint)hg->greenSize;
        break;
    case GL_HISTOGRAM_BLUE_SIZE:
        value = (GLint)hg->blueSize;
        break;
    case GL_HISTOGRAM_ALPHA_SIZE:
        value = (GLint)hg->alphaSize;
        break;
    case GL_HISTOGRAM_LUMINANCE_SIZE:
        value = (GLint)hg->luminanceSize;
        break;
    case GL_HISTOGRAM_SINK:
        value = (GLint)hg->sink;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        break;
    }

    switch (return_type)
    {
    case GL_INT:
        *(GLint*)params = value;
        break;
    case GL_FLOAT:
        *(GLfloat*)params = (GLfloat)value;
        break;
    default:
        /* did __glim_GetHistogramParameter{if}v pass wrong return type? */
        GL_ASSERT(0);
    }
}

static GLvoid ResetHistogramArray(__GLcontext *gc, __GLhistogram *hg)
{
    __GL_MEMZERO(hg->array, hg->width *
        __glElementsPerGroup(hg->baseFormat, hg->type) *
        __glBytesPerElement(hg->type));
}

/*
** Begin GL_EXT_histogram APIs
*/
GLvoid APIENTRY __glim_Histogram(GLenum target, GLsizei width, GLenum internalFormat, GLboolean sink)
{
    __GLhistogram* hg;
    GLboolean isProxy;
    GLenum rv;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Histogram", DT_GLenum, target, DT_GLsizei, width, DT_GLenum, internalFormat, DT_GLboolean, sink, DT_GLnull);
#endif

    if ((rv = CheckHistogramArgs(gc, target, width, internalFormat, sink)))
    {
        __glSetError(rv);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    hg = LookupHistogram(gc, target, &isProxy);
    if (!AllocHistogramArray(gc, hg, isProxy, width, internalFormat, sink))
    {
        return;
    }
    if (isProxy || width == 0) {
        return;
    }

    ResetHistogramArray(gc, hg);
    (*gc->dp.histogram)(gc, target, width, internalFormat, sink);
    __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_HISTOGRAM_BIT);

}

GLvoid APIENTRY __glim_ResetHistogram(GLenum target)
{
    __GLhistogram *hg;
    GLboolean isProxy;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ResetHistogram", DT_GLenum, target, DT_GLnull);
#endif

    switch (target)
    {
    case GL_HISTOGRAM:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    hg = LookupHistogram(gc, target, &isProxy);
    ResetHistogramArray(gc, hg);

    (*gc->dp.resetHistogram)(gc, target);
    __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_HISTOGRAM_BIT);
}

GLvoid APIENTRY __glim_GetHistogram(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values)
{
    __GLhistogram *hg;
    GLboolean isProxy;
    __GLpixelSpanInfo *spanInfo;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetHistogram", DT_GLenum, target, DT_GLboolean, reset, DT_GLenum, format, DT_GLenum, type, DT_GLvoid_ptr, values, DT_GLnull);
#endif

    spanInfo = gc->pixel.spanInfo;
    switch (target)
    {
    case GL_HISTOGRAM:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    switch (format)
    { /* smaller set of allowable formats than HistogramEXT */
    case GL_RED:
    case GL_GREEN:
    case GL_BLUE:
    case GL_ALPHA:
    case GL_RGB:
    case GL_BGR_EXT:
    case GL_RGBA:
    case GL_BGRA_EXT:
    case GL_ABGR_EXT:
    case GL_LUMINANCE:
    case GL_LUMINANCE_ALPHA:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    switch (type)
    {
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
    case GL_UNSIGNED_SHORT:
    case GL_SHORT:
    case GL_UNSIGNED_INT:
    case GL_INT:
    case GL_FLOAT:
        break;
#if GL_EXT_packed_pixels
    case GL_UNSIGNED_BYTE_3_3_2:
    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_BYTE_2_3_3_REV:
    case GL_UNSIGNED_SHORT_5_6_5_REV:
        switch (format)
        {
        case GL_RGB:
        case GL_BGR_EXT:
            break;
        default:
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        break;
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_5_5_5_1:
    case GL_UNSIGNED_INT_8_8_8_8:
    case GL_UNSIGNED_INT_10_10_10_2:
    case GL_UNSIGNED_SHORT_4_4_4_4_REV:
    case GL_UNSIGNED_SHORT_1_5_5_5_REV:
    case GL_UNSIGNED_INT_8_8_8_8_REV:
    case GL_UNSIGNED_INT_2_10_10_10_REV:
        switch (format)
        {
        case GL_RGBA:
        case GL_BGRA_EXT:
        case GL_ABGR_EXT:
            break;
        default:
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        break;
#endif
    case GL_UNSIGNED_INT_5_9_9_9_REV_EXT:
        if(!__glExtension[INDEX_EXT_texture_shared_exponent].bEnabled)
        {
            __glSetError(GL_INVALID_ENUM);
            return;
        }

        if(format != GL_RGB)
        {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        break;
    case GL_HALF_FLOAT_ARB:
        if (!__glExtension[INDEX_ARB_half_float_pixel].bEnabled)
        {
            __glSetError(GL_INVALID_ENUM);
            return;
        }
        break;

    case GL_UNSIGNED_INT_10F_11F_11F_REV_EXT:
        if(!__glExtension[INDEX_EXT_packed_float].bEnabled)
        {
            __glSetError(GL_INVALID_ENUM);
            return;
        }

        if(format != GL_RGB)
        {
            __glSetError(GL_INVALID_ENUM);
            return;
        }
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    hg = LookupHistogram(gc, target, &isProxy);

    if (!hg->array)
    {
        return;
    }

    __RAW_INIT_SPANINFO();
    __glInitMemGet(gc, spanInfo, hg->width, 1, 0, hg->baseFormat, hg->type, hg->array);
    __glInitMemPack(gc, spanInfo, format, type, values);

    spanInfo->applySrcClamp = GL_FALSE;
    spanInfo->applyDstClamp = GL_FALSE;
    spanInfo->applyPixelTransfer = GL_FALSE;
    spanInfo->nonColorComp = GL_TRUE;

    __glGenericPickCopyImage(gc, spanInfo, spanInfo->applyPixelTransfer);

    if (reset)
    {
        ResetHistogramArray(gc, hg);
    }
}

GLvoid APIENTRY __glim_GetHistogramParameteriv(GLenum target, GLenum pname, GLint *params)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetHistogramParameteriv", DT_GLenum, target, DT_GLenum, pname, DT_GLint_ptr, params, DT_GLnull);
#endif

    GetHistogramParameters(target, pname, params, GL_INT);
}

GLvoid APIENTRY __glim_GetHistogramParameterfv(GLenum target, GLenum pname,
                                               GLfloat *params)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetHistogramParameterfv", DT_GLenum, target, DT_GLenum, pname, DT_GLfloat_ptr, params, DT_GLnull);
#endif

    GetHistogramParameters(target, pname, params, GL_FLOAT);
}

/*
** Minmax helper functions
*/
static __GLminmax *LookupMinmax(__GLcontext *gc, GLenum target)
{
    __GLminmax *mm;

    switch (target)
    {
    case GL_MINMAX:
        mm = &gc->state.pixel.minmax;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return NULL;
    }
    return mm;
}

static GLvoid InitMinmaxArray(__GLcontext *gc, __GLminmax *mm, GLenum internalFormat, GLboolean sink)
{
    switch (internalFormat)
    {
    case GL_ALPHA:
    case GL_ALPHA4:
    case GL_ALPHA8:
    case GL_ALPHA12:
    case GL_ALPHA16:
        mm->format =
            mm->baseFormat = GL_ALPHA;
        mm->type = GL_FLOAT;
        break;
    case GL_LUMINANCE:
    case GL_LUMINANCE4:
    case GL_LUMINANCE8:
    case GL_LUMINANCE12:
    case GL_LUMINANCE16:
        mm->format =
            mm->baseFormat = GL_LUMINANCE;
        mm->type = GL_FLOAT;
        break;
    case GL_LUMINANCE_ALPHA:
    case GL_LUMINANCE4_ALPHA4:
    case GL_LUMINANCE6_ALPHA2:
    case GL_LUMINANCE8_ALPHA8:
    case GL_LUMINANCE12_ALPHA4:
    case GL_LUMINANCE12_ALPHA12:
    case GL_LUMINANCE16_ALPHA16:
        mm->format =
            mm->baseFormat = GL_LUMINANCE_ALPHA;
        mm->type = GL_FLOAT;
        break;
    case GL_RGB:
    case GL_R3_G3_B2:
    case GL_RGB4:
    case GL_RGB5:
    case GL_RGB8:
    case GL_RGB10:
    case GL_RGB12:
    case GL_RGB16:
        mm->format =
            mm->baseFormat = GL_RGB;
        mm->type = GL_FLOAT;
        break;
    case GL_RGBA:
    case GL_RGBA2:
    case GL_RGBA4:
    case GL_RGBA8:
    case GL_RGBA12:
    case GL_RGBA16:
    case GL_RGB5_A1:
    case GL_RGB10_A2:
        mm->format =
            mm->baseFormat = GL_RGBA;
        mm->type = GL_FLOAT;
        break;
    default:
        GL_ASSERT(0);
        return;
    }

    mm->formatReturn = internalFormat;
    mm->sink = sink;
}

static GLenum CheckMinmaxArgs(__GLcontext *gc, GLenum target, GLenum internalFormat, GLboolean sink)
{
    switch (target)
    {
    case GL_MINMAX:
        break;
    default:
        return GL_INVALID_ENUM;
    }

    switch (internalFormat)
    {
    case GL_ALPHA:
    case GL_ALPHA4:
    case GL_ALPHA8:
    case GL_ALPHA12:
    case GL_ALPHA16:
        break;
    case GL_LUMINANCE:
    case GL_LUMINANCE4:
    case GL_LUMINANCE8:
    case GL_LUMINANCE12:
    case GL_LUMINANCE16:
        break;
    case GL_LUMINANCE_ALPHA:
    case GL_LUMINANCE4_ALPHA4:
    case GL_LUMINANCE6_ALPHA2:
    case GL_LUMINANCE8_ALPHA8:
    case GL_LUMINANCE12_ALPHA4:
    case GL_LUMINANCE12_ALPHA12:
    case GL_LUMINANCE16_ALPHA16:
        break;
    case GL_RGB:
    case GL_R3_G3_B2:
    case GL_RGB4:
    case GL_RGB5:
    case GL_RGB8:
    case GL_RGB10:
    case GL_RGB12:
    case GL_RGB16:
        break;
    case GL_RGBA:
    case GL_RGBA2:
    case GL_RGBA4:
    case GL_RGBA8:
    case GL_RGBA12:
    case GL_RGBA16:
    case GL_RGB5_A1:
    case GL_RGB10_A2:
        break;
    default:
        return GL_INVALID_ENUM;
    }

    return 0;
}

GLvoid ResetMinmaxArray(__GLcontext *gc, __GLminmax *mm)
{
    GLint lastMinValue, lastMaxValue;
    GLint i;

    switch (mm->baseFormat)
    {
    case GL_ALPHA:
        lastMinValue = 0;
        lastMaxValue = 1;
        break;
    case GL_LUMINANCE:
        lastMinValue = 0;
        lastMaxValue = 1;
        break;
    case GL_LUMINANCE_ALPHA:
        lastMinValue = 1;
        lastMaxValue = 2;
        break;
    case GL_RGB:
        lastMinValue = 2;
        lastMaxValue = 5;
        break;
    case GL_RGBA:
        lastMinValue = 3;
        lastMaxValue = 7;
        break;
    default:
        GL_ASSERT(0);
        return;
    }
    GL_ASSERT(mm->type == GL_FLOAT);

    /* set minimums to largest possible value */
    for (i=0; i<=lastMinValue; ++i)
    {
        mm->array[i] = __GL_MAX_FLOAT;
    }

    /* set maximums to smallest possible value */
    for (i=lastMinValue+1; i<=lastMaxValue; ++i)
    {
        mm->array[i] = -__GL_MAX_FLOAT;
    }
}

static GLvoid GetMinmaxParameters(GLenum target, GLenum pname, GLvoid *values, GLenum return_type)
{
    __GLminmax *mm;
    GLint return_value;

    __GL_SETUP_NOT_IN_BEGIN();

    if ((mm = LookupMinmax(gc, target)) == NULL)
    {
        return;
    }

    switch (pname)
    {
    case GL_MINMAX_FORMAT:
        return_value = (GLint)mm->formatReturn;
        break;
    case GL_MINMAX_SINK:
        return_value = (GLint)mm->sink;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    switch (return_type)
    {
    case GL_INT:
        *(GLint *)values = return_value;
        break;
    case GL_FLOAT:
        *(GLfloat *)values = (GLfloat)return_value;
        break;
    default:
        GL_ASSERT(0);
        break;
    }
}

/*
** Minimum and maximum values are stored in an 8-element array in whatever
** the current format is. This makes it easier to handle getminmax().
*/
GLvoid APIENTRY __glim_Minmax(GLenum target, GLenum internalFormat, GLboolean sink)
{
    __GLminmax* mm;
    GLenum rv;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Minmax", DT_GLenum, target, DT_GLenum, internalFormat, DT_GLboolean, sink, DT_GLnull);
#endif

    if ((rv = CheckMinmaxArgs(gc, target, internalFormat, sink)))
    {
        __glSetError(rv);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    mm = LookupMinmax(gc, target);
    InitMinmaxArray(gc, mm, internalFormat, sink);
    ResetMinmaxArray(gc, mm);

    (*gc->dp.minmax)(gc, target, internalFormat, sink);
    __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_MINMAX_BIT);
}

GLvoid APIENTRY __glim_ResetMinmax(GLenum target)
{
    __GLminmax* mm;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ResetMinmax", DT_GLenum, target, DT_GLnull);
#endif

    switch (target)
    {
    case GL_MINMAX:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    mm = LookupMinmax(gc, target);
    ResetMinmaxArray(gc, mm);

    (*gc->dp.resetMinmax)(gc, target);
    __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_HISTOGRAM_BIT);
}

GLvoid APIENTRY __glim_GetMinmax(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values)
{
    __GLminmax *mm;
    __GLpixelSpanInfo *spanInfo;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetMinmax", DT_GLenum, target, DT_GLboolean, reset, DT_GLenum, format, DT_GLenum, type, DT_GLvoid_ptr, values, DT_GLnull);
#endif

    spanInfo = gc->pixel.spanInfo;

    switch (target)
    {
    case GL_MINMAX:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    switch (format)
    {
    case GL_RED:
    case GL_GREEN:
    case GL_BLUE:
    case GL_ALPHA:
    case GL_RGB:
    case GL_BGR_EXT:
    case GL_RGBA:
    case GL_BGRA_EXT:
    case GL_ABGR_EXT:
    case GL_LUMINANCE:
    case GL_LUMINANCE_ALPHA:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    switch (type)
    {
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
    case GL_UNSIGNED_SHORT:
    case GL_SHORT:
    case GL_UNSIGNED_INT:
    case GL_INT:
    case GL_FLOAT:
        break;
#if GL_EXT_packed_pixels
    case GL_UNSIGNED_BYTE_3_3_2:
    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_BYTE_2_3_3_REV:
    case GL_UNSIGNED_SHORT_5_6_5_REV:
        switch (format)
        {
        case GL_RGB:
        case GL_BGR_EXT:
            break;
        default:
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        break;
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_5_5_5_1:
    case GL_UNSIGNED_INT_8_8_8_8:
    case GL_UNSIGNED_INT_10_10_10_2:
    case GL_UNSIGNED_SHORT_4_4_4_4_REV:
    case GL_UNSIGNED_SHORT_1_5_5_5_REV:
    case GL_UNSIGNED_INT_8_8_8_8_REV:
    case GL_UNSIGNED_INT_2_10_10_10_REV:
        switch (format)
        {
        case GL_RGBA:
        case GL_BGRA_EXT:
        case GL_ABGR_EXT:
            break;
        default:
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        break;
#endif
    case GL_UNSIGNED_INT_5_9_9_9_REV_EXT:
        if(!__glExtension[INDEX_EXT_texture_shared_exponent].bEnabled)
        {
            __glSetError(GL_INVALID_ENUM);
            return;
        }

        if(format != GL_RGB)
        {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        break;
    case GL_HALF_FLOAT_ARB:
        if (!__glExtension[INDEX_ARB_half_float_pixel].bEnabled)
        {
            __glSetError(GL_INVALID_ENUM);
            return;
        }
        break;

    case GL_UNSIGNED_INT_10F_11F_11F_REV_EXT:
        if(!__glExtension[INDEX_EXT_packed_float].bEnabled)
        {
            __glSetError(GL_INVALID_ENUM);
            return;
        }

        if(format != GL_RGB)
        {
            __glSetError(GL_INVALID_ENUM);
            return;
        }
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    mm = LookupMinmax(gc, target);

    __RAW_INIT_SPANINFO();
    __glInitMemGet(gc, spanInfo, 2, 1, 0, mm->baseFormat, mm->type, mm->array);
    __glInitMemPack(gc, spanInfo, format, type, values);

    spanInfo->applySrcClamp = GL_FALSE;
    spanInfo->applyDstClamp = GL_FALSE;
    spanInfo->applyPixelTransfer = GL_FALSE;

    __glGenericPickCopyImage(gc, spanInfo, spanInfo->applyPixelTransfer);

    if (reset)
    {
        ResetMinmaxArray(gc, mm);
    }
}

GLvoid APIENTRY __glim_GetMinmaxParameteriv(GLenum target, GLenum pname,
                                            GLint *values)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetMinmaxParameteriv", DT_GLenum, target, DT_GLenum, pname, DT_GLint_ptr, values, DT_GLnull);
#endif

    GetMinmaxParameters(target, pname, values, GL_INT);
}

GLvoid APIENTRY __glim_GetMinmaxParameterfv(GLenum target, GLenum pname, GLfloat *values)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetMinmaxParameterfv", DT_GLenum, target, DT_GLenum, pname, DT_GLfloat_ptr, values, DT_GLnull);
#endif

    GetMinmaxParameters(target, pname, values, GL_FLOAT);
}
/*
** End GL_EXT_histogram APIs
*/

