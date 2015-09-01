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


#include "gc_gl_context.h"
#include "gl/gl_device.h"
#include "gc_gl_debug.h"
#include "gc_gl_names_inline.c"

#ifndef GL_MIN_PROGRAM_TEXEL_OFFSET_EXT
#define GL_MIN_PROGRAM_TEXEL_OFFSET_EXT   0x8904
#endif

#ifndef GL_MAX_PROGRAM_TEXEL_OFFSET_EXT
#define GL_MAX_PROGRAM_TEXEL_OFFSET_EXT   0x8905
#endif

#define __CHECK_GET_COLOR_CLAMP(dstColor, srcColor) \
    { \
        GLboolean clamp; \
        if(gc->state.raster.clampFragColor == GL_FIXED_ONLY_ARB) \
        { \
            clamp = !gc->modes.rgbFloatMode; \
        } \
        else \
        { \
           clamp = (GLboolean)(gc->state.raster.clampFragColor); \
        } \
       if(clamp) \
       { \
           __glClampColor(dstColor, srcColor); \
       } \
       else \
       { \
           *dstColor = *srcColor; \
       } \
    } \



#if defined(_WIN32)
#pragma warning(disable: 4244)
#endif


GLuint __glGetCurrentMatrixStackDepth(__GLcontext * gc);
GLvoid __glGetCurrentTransposeMatrix(__GLcontext * gc, GLfloat *m);
GLvoid __glGetCurrentMatrix(__GLcontext * gc, GLfloat *m);
/*
** Convert the results of a query from one type to another.
*/
GLvoid __glConvertResult(__GLcontext *gc, GLint fromType, const GLvoid *rawdata,
                       GLint toType, GLvoid *result, GLint size)
{
    GLfloat rscale = 0/*gc->drawablePrivate->frontBuffer.oneOverRedScale*/;
    GLfloat gscale = 0/*gc->drawablePrivate->frontBuffer.oneOverGreenScale*/;
    GLfloat bscale = 0/*gc->drawablePrivate->frontBuffer.oneOverBlueScale*/;
    GLfloat ascale = 0/*gc->drawablePrivate->frontBuffer.oneOverAlphaScale*/;
    GLint i;

    switch (fromType) {
      case __GL_FLOAT:
          switch (toType) {
      case __GL_FLOAT32:
          for (i=0; i < size; i++) {
              ((GLfloat *)result)[i] = ((const GLfloat *)rawdata)[i];
          }
          break;
      case __GL_FLOAT64:
          for (i=0; i < size; i++) {
              ((GLdouble *)result)[i] = ((const GLfloat *)rawdata)[i];
          }
          break;
      case __GL_INT32:
          for (i=0; i < size; i++) {
              ((GLint *)result)[i] =
                  (GLint)(((const GLfloat *)rawdata)[i] >= 0.0 ?
                  ((const GLfloat *)rawdata)[i] + __glHalf:
              ((const GLfloat *)rawdata)[i] - __glHalf);
          }
          break;
      case __GL_BOOLEAN:
          for (i=0; i < size; i++) {
              ((GLboolean *)result)[i] =
                  ((const GLfloat *)rawdata)[i] ? 1 : 0;
          }
          break;
          }
          break;

      case __GL_COLOR:
          switch (toType) {
      case __GL_FLOAT32:
          for (i=0; i < size; i++) {
              ((GLfloat *)result)[i] = ((const GLfloat *)rawdata)[i];
          }
          break;
      case __GL_FLOAT64:
          for (i=0; i < size; i++) {
              ((GLdouble *)result)[i] = ((const GLfloat *)rawdata)[i];
          }
          break;
      case __GL_INT32:
          for (i=0; i < size; i++) {
              ((GLint *)result)[i] =
                  __GL_FLOAT_TO_I(((const GLfloat *)rawdata)[i]);
          }
          break;
      case __GL_BOOLEAN:
          for (i=0; i < size; i++) {
              ((GLboolean *)result)[i] =
                  ((const GLfloat *)rawdata)[i] ? 1 : 0;
          }
          break;
          }
          break;

      case __GL_SCOLOR:
          switch (toType) {
      case __GL_FLOAT32:
          ((GLfloat *)result)[0] =
              ((const GLfloat *)rawdata)[0] * rscale;
          ((GLfloat *)result)[1] =
              ((const GLfloat *)rawdata)[1] * gscale;
          ((GLfloat *)result)[2] =
              ((const GLfloat *)rawdata)[2] * bscale;
          ((GLfloat *)result)[3] =
              ((const GLfloat *)rawdata)[3] * ascale;
          break;
      case __GL_FLOAT64:
          ((GLdouble *)result)[0] =
              ((const GLfloat *)rawdata)[0] * rscale;
          ((GLdouble *)result)[1] =
              ((const GLfloat *)rawdata)[1] * gscale;
          ((GLdouble *)result)[2] =
              ((const GLfloat *)rawdata)[2] * bscale;
          ((GLdouble *)result)[3] =
              ((const GLfloat *)rawdata)[3] * ascale;
          break;
      case __GL_INT32:
          ((GLint *)result)[0] =
              __GL_FLOAT_TO_I(((const GLfloat *)rawdata)[0] *
              rscale);
          ((GLint *)result)[1] =
              __GL_FLOAT_TO_I(((const GLfloat *)rawdata)[1] *
              gscale);
          ((GLint *)result)[2] =
              __GL_FLOAT_TO_I(((const GLfloat *)rawdata)[2] *
              bscale);
          ((GLint *)result)[3] =
              __GL_FLOAT_TO_I(((const GLfloat *)rawdata)[3] *
              ascale);
          break;
      case __GL_BOOLEAN:
          for (i=0; i < size; i++) {
              ((GLboolean *)result)[i] =
                  ((const GLfloat *)rawdata)[i] ? 1 : 0;
          }
          break;
          }
          break;

      case __GL_INT32:
          switch (toType) {
      case __GL_FLOAT32:
          for (i=0; i < size; i++) {
              ((GLfloat *)result)[i] = ((const GLint *)rawdata)[i];
          }
          break;
      case __GL_FLOAT64:
          for (i=0; i < size; i++) {
              ((GLdouble *)result)[i] = ((const GLint *)rawdata)[i];
          }
          break;
      case __GL_INT32:
          for (i=0; i < size; i++) {
              ((GLint *)result)[i] = ((const GLint *)rawdata)[i];
          }
          break;
      case __GL_BOOLEAN:
          for (i=0; i < size; i++) {
              ((GLboolean *)result)[i] = ((const GLint *)rawdata)[i] ? 1 : 0;
          }
          break;
          }
          break;

      case __GL_BOOLEAN:
          switch (toType) {
      case __GL_FLOAT32:
          for (i=0; i < size; i++) {
              ((GLfloat *)result)[i] = ((const GLboolean *)rawdata)[i];
          }
          break;
      case __GL_FLOAT64:
          for (i=0; i < size; i++) {
              ((GLdouble *)result)[i] = ((const GLboolean *)rawdata)[i];
          }
          break;
      case __GL_INT32:
          for (i=0; i < size; i++) {
              ((GLint *)result)[i] = ((const GLboolean *)rawdata)[i];
          }
          break;
      case __GL_BOOLEAN:
          for (i=0; i < size; i++) {
              ((GLboolean *)result)[i] =
                  ((const GLboolean *)rawdata)[i] ? 1 : 0;
          }
          break;
          }
          break;
    }
}


GLvoid APIENTRY __glim_GetPolygonStipple(GLubyte *outImage)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetPolygonStipple", DT_GLubyte_ptr, outImage, DT_GLnull);
#endif


    __glEmptyImage(gc, 32, 32, GL_COLOR_INDEX, GL_BITMAP,
        &gc->state.polygonStipple.stipple[0], outImage);
}


/*
** Helper function to remove specific value from given buf.
** @Input :
**      buf     :   pointer to given buf
**      size    :   buf size
**      value   :   the value want to remove
** @Return :
**      buf size after remove the value
** @Note :
**      If find the value want to remove , all other values will be move to
**      the front of buf, value in left buf is undefined. If not find, the
**      original buf won't change.
*/
GLint __glRemoveInteger(GLint* buf, GLint size, GLint value)
{
    GLint targetIdx = 0;
    GLboolean found = GL_FALSE;

    GL_ASSERT(buf);
    for( targetIdx = 0; targetIdx< size; targetIdx++ )
    {
        if( buf[targetIdx] == value )
        {
            found = GL_TRUE;
            break;
        }
    }

    if(found)
    {
        GLint i = 0;
        for(i = targetIdx; i < size-1; i++)
        {
            buf[i] = buf[i+1];
        }
        return size-1;
    }
    else
    {
        return size;
    }

}

__GL_INLINE GLint __glGetFboColorBits(__GLcontext *gc, GLenum bitType)
{
    __GLframebufferObject *framebufferObj = gc->frameBuffer.drawFramebufObj;
    __GLrenderbufferObject *rbo = NULL;
    __GLtextureObject *tex = NULL;
    __GLmipMapLevel *mipmap = NULL;
    __GLfboAttachPoint *attachPoint = NULL;
    const __GLdeviceFormatInfo * deviceFormatInfo = NULL;
    GLint i;
    GLint redBits = 0, greenBits = 0, blueBits = 0, alphaBits = 0;
    GLint depthBits = 0, stencilBits = 0;
    if ((bitType == GL_RED_BITS) || (bitType == GL_BLUE_BITS) ||
        (bitType == GL_GREEN_BITS) || (bitType == GL_ALPHA_BITS))
    {
        for (i = 0; i < __GL_MAX_COLOR_ATTACHMENTS; i++)
        {
            attachPoint = &framebufferObj->attachPoint[i];

            /* Attach point with type NONE is framebuffer attachment complete. */
            if(attachPoint->objectType == GL_NONE)
                continue;
            switch(attachPoint->objectType)
            {
            case GL_RENDERBUFFER_EXT:
                rbo = (__GLrenderbufferObject *)__glGetObject(gc, gc->frameBuffer.rboShared, attachPoint->objName);
                if (!rbo)
                {
                    GL_ASSERT(0); /* should not happen */
                    continue;
                }
                deviceFormatInfo = rbo->deviceFormatInfo;
                break;
            case GL_TEXTURE:
                tex = (__GLtextureObject *)__glGetObject(gc, gc->texture.shared, attachPoint->objName);
                if (!tex)
                {
                    GL_ASSERT(0);
                    continue;
                }
                mipmap = &tex->faceMipmap[attachPoint->face][attachPoint->level];
                deviceFormatInfo = mipmap->deviceFormat;
                break;
            }

            if (deviceFormatInfo)
                break;
        }
        if (deviceFormatInfo)
        {
            redBits   = deviceFormatInfo->redSize;
            blueBits  = deviceFormatInfo->blueSize;
            greenBits = deviceFormatInfo->greenSize;
            alphaBits = deviceFormatInfo->alphaSize;
        }
        switch(bitType)
        {
        case GL_RED_BITS:
            return redBits;
        case GL_GREEN_BITS:
            return greenBits;
        case GL_BLUE_BITS:
            return blueBits;
        case GL_ALPHA_BITS:
            return alphaBits;
        default:
            GL_ASSERT(0);
        }

    }
    else if (bitType == GL_DEPTH_BITS)
    {
        /*depth attachpoint*/
        attachPoint = &framebufferObj->attachPoint[__GL_MAX_COLOR_ATTACHMENTS];
        if (attachPoint)
        {
            switch(attachPoint->objectType)
            {
            case GL_RENDERBUFFER_EXT:
                rbo = (__GLrenderbufferObject *)__glGetObject(gc, gc->frameBuffer.rboShared, attachPoint->objName);
                if (!rbo)
                {
                    GL_ASSERT(0); /* should not happen */
                }
                deviceFormatInfo = rbo->deviceFormatInfo;
                break;
            case GL_TEXTURE:
                tex = (__GLtextureObject *)__glGetObject(gc, gc->texture.shared, attachPoint->objName);
                if (!tex)
                {
                    GL_ASSERT(0);
                }
                mipmap = &tex->faceMipmap[attachPoint->face][attachPoint->level];
                deviceFormatInfo = mipmap->deviceFormat;
                break;
            }
        }
        if (deviceFormatInfo)
        {
            depthBits   = deviceFormatInfo->depthSize;
        }
        return depthBits;
    }
    else if (bitType == GL_STENCIL_BITS)
    {
        /*stencil attachpoint*/
        attachPoint = &framebufferObj->attachPoint[__GL_MAX_COLOR_ATTACHMENTS + 1];
        if (attachPoint)
        {
            switch(attachPoint->objectType)
            {
            case GL_RENDERBUFFER_EXT:
                rbo = (__GLrenderbufferObject *)__glGetObject(gc, gc->frameBuffer.rboShared, attachPoint->objName);
                if (!rbo)
                {
                    GL_ASSERT(0); /* should not happen */
                }
                deviceFormatInfo = rbo->deviceFormatInfo;
                break;
            case GL_TEXTURE:
                tex = (__GLtextureObject *)__glGetObject(gc, gc->texture.shared, attachPoint->objName);
                if (!tex)
                {
                    GL_ASSERT(0);
                }
                mipmap = &tex->faceMipmap[attachPoint->face][attachPoint->level];
                deviceFormatInfo = mipmap->deviceFormat;
                break;
            }
        }

        if (deviceFormatInfo)
        {
            stencilBits   = deviceFormatInfo->stencilSize;
        }
        return stencilBits;
    }

    GL_ASSERT(0); /*should not reach here*/
    return 0;
}

/*
** Fetch the data for a query in its internal type, then convert it to the
** type that the user asked for.
*/
__GL_INLINE GLvoid __glDoGet(GLenum sq, GLvoid *result, GLint type, const GLbyte *procName)
{
    GLint index;
    GLfloat ftemp[100], *fp = ftemp;          /* NOTE: for floats */
    GLfloat ctemp[100], *cp = ctemp;          /* NOTE: for colors */
    GLfloat sctemp[100], *scp = sctemp;       /* NOTE: for scaled colors */
    GLint itemp[100], *ip = itemp;              /* NOTE: for ints */
    GLboolean ltemp[100], *lp = ltemp;          /* NOTE: for logicals */
    GLfloat *mp;
    __GL_SETUP_NOT_IN_BEGIN();

    __GL_VERTEX_BUFFER_FLUSH(gc);

    switch (sq)
    {
    case GL_ALPHA_TEST:
    case GL_BLEND:
    case GL_COLOR_MATERIAL:
    case GL_CULL_FACE:
    case GL_DEPTH_TEST:
    case GL_DITHER:
    case GL_FOG:
    case GL_LIGHTING:
    case GL_LINE_SMOOTH:
    case GL_LINE_STIPPLE:
    case GL_INDEX_LOGIC_OP:
    case GL_COLOR_LOGIC_OP:
    case GL_NORMALIZE:
    case GL_POINT_SMOOTH:
    case GL_POLYGON_SMOOTH:
    case GL_POLYGON_STIPPLE:
    case GL_SCISSOR_TEST:
    case GL_STENCIL_TEST:
    case GL_TEXTURE_1D:
    case GL_TEXTURE_2D:
    case GL_TEXTURE_3D:
    case GL_TEXTURE_CUBE_MAP:
#if GL_ARB_texture_rectangle
    case GL_TEXTURE_RECTANGLE_ARB:
#endif
    case GL_AUTO_NORMAL:
    case GL_TEXTURE_GEN_S:
    case GL_TEXTURE_GEN_T:
    case GL_TEXTURE_GEN_R:
    case GL_TEXTURE_GEN_Q:
    case GL_CLIP_PLANE0:
    case GL_CLIP_PLANE1:
    case GL_CLIP_PLANE2:
    case GL_CLIP_PLANE3:
    case GL_CLIP_PLANE4:
    case GL_CLIP_PLANE5:
    case GL_LIGHT0:
    case GL_LIGHT1:
    case GL_LIGHT2:
    case GL_LIGHT3:
    case GL_LIGHT4:
    case GL_LIGHT5:
    case GL_LIGHT6:
    case GL_LIGHT7:
    case GL_MAP1_COLOR_4:
    case GL_MAP1_NORMAL:
    case GL_MAP1_INDEX:
    case GL_MAP1_TEXTURE_COORD_1:
    case GL_MAP1_TEXTURE_COORD_2:
    case GL_MAP1_TEXTURE_COORD_3:
    case GL_MAP1_TEXTURE_COORD_4:
    case GL_MAP1_VERTEX_3:
    case GL_MAP1_VERTEX_4:
    case GL_MAP2_COLOR_4:
    case GL_MAP2_NORMAL:
    case GL_MAP2_INDEX:
    case GL_MAP2_TEXTURE_COORD_1:
    case GL_MAP2_TEXTURE_COORD_2:
    case GL_MAP2_TEXTURE_COORD_3:
    case GL_MAP2_TEXTURE_COORD_4:
    case GL_MAP2_VERTEX_3:
    case GL_MAP2_VERTEX_4:
    case GL_POLYGON_OFFSET_POINT:
    case GL_POLYGON_OFFSET_LINE:
    case GL_POLYGON_OFFSET_FILL:
    case GL_VERTEX_ARRAY:
    case GL_NORMAL_ARRAY:
    case GL_COLOR_ARRAY:
    case GL_INDEX_ARRAY:
    case GL_TEXTURE_COORD_ARRAY:
    case GL_EDGE_FLAG_ARRAY:
    case GL_SECONDARY_COLOR_ARRAY:
    case GL_COLOR_SUM:
    case GL_FOG_COORDINATE_ARRAY:
    case GL_COLOR_TABLE:
    case GL_POST_CONVOLUTION_COLOR_TABLE:
    case GL_POST_COLOR_MATRIX_COLOR_TABLE:
    case GL_CONVOLUTION_1D:
    case GL_CONVOLUTION_2D:
    case GL_SEPARABLE_2D:
    case GL_HISTOGRAM:
    case GL_MINMAX:
#if GL_ARB_vertex_program
    case GL_VERTEX_PROGRAM_ARB:
    case GL_VERTEX_PROGRAM_POINT_SIZE_ARB:
    case GL_VERTEX_PROGRAM_TWO_SIDE_ARB:
#endif
#if GL_ARB_fragment_program
    case GL_FRAGMENT_PROGRAM_ARB:
#endif
#if GL_EXT_depth_bounds_test
    case GL_DEPTH_BOUNDS_TEST_EXT:
#endif
    case GL_POINT_SPRITE:
    case GL_RESCALE_NORMAL:
        *lp++ = __glim_IsEnabled(sq);
        break;

#if GL_ARB_vertex_program
    case GL_PROGRAM_ERROR_POSITION_ARB:
        *ip++ = gc->program.errorPos;
        break;

    case GL_CURRENT_MATRIX_ARB:
        __glGetCurrentMatrix(gc, fp);
        fp += 16;
        break;

    case GL_TRANSPOSE_CURRENT_MATRIX_ARB:
        __glGetCurrentTransposeMatrix(gc, fp);
        fp += 16;
        break;

    case GL_CURRENT_MATRIX_STACK_DEPTH_ARB:
        *ip++ = __glGetCurrentMatrixStackDepth(gc);
        break;

    case GL_MAX_VERTEX_ATTRIBS_ARB:
        *ip++ = __GL_MAX_PROGRAM_VERTEX_ATTRIBUTES;
        break;

    case GL_MAX_PROGRAM_MATRICES_ARB:
        *ip++ = __GL_MAX_PROGRAM_MATRICES;
        break;

    case GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB:
        *ip++ = __GL_MAX_PROGRAM_STACK_DEPTH;
        break;
#endif

    case GL_MAX_3D_TEXTURE_SIZE:
    case GL_MAX_CUBE_MAP_TEXTURE_SIZE:
    case GL_MAX_TEXTURE_SIZE:
        *ip++ = gc->constants.maxTextureSize;
        break;
    case GL_SUBPIXEL_BITS:
        *ip++ = gc->constants.subpixelBits;
        break;
    case GL_MAX_LIST_NESTING:
        *ip++ = gc->constants.maxListNesting;
        break;
    case GL_CURRENT_COLOR:
        *cp++ = gc->state.current.color.r;
        *cp++ = gc->state.current.color.g;
        *cp++ = gc->state.current.color.b;
        *cp++ = gc->state.current.color.a;
        break;
    case GL_CURRENT_INDEX:
        *fp++ = gc->state.current.colorIndex;
        break;
    case GL_CURRENT_NORMAL:
        *cp++ = gc->state.current.normal.x;
        *cp++ = gc->state.current.normal.y;
        *cp++ = gc->state.current.normal.z;
        break;
    case GL_CURRENT_TEXTURE_COORDS:
        index = gc->state.texture.activeTexIndex;
        *fp++ = gc->state.current.texture[index].x;
        *fp++ = gc->state.current.texture[index].y;
        *fp++ = gc->state.current.texture[index].z;
        *fp++ = gc->state.current.texture[index].w;
        break;
    case GL_CURRENT_RASTER_INDEX:
        /* Always return 1 */
        *fp++ = 1.0;
        break;
    case GL_CURRENT_RASTER_COLOR:
        *cp++ = gc->state.rasterPos.rPos.colors[__GL_FRONTFACE].r;
        *cp++ = gc->state.rasterPos.rPos.colors[__GL_FRONTFACE].g;
        *cp++ = gc->state.rasterPos.rPos.colors[__GL_FRONTFACE].b;
        *cp++ = gc->state.rasterPos.rPos.colors[__GL_FRONTFACE].a;
        break;
    case GL_CURRENT_RASTER_TEXTURE_COORDS:
        index = gc->state.texture.activeTexIndex;
        *fp++ = gc->state.rasterPos.rPos.texture[index].x;
        *fp++ = gc->state.rasterPos.rPos.texture[index].y;
        *fp++ = gc->state.rasterPos.rPos.texture[index].z;
        *fp++ = gc->state.rasterPos.rPos.texture[index].w;
        break;
    case GL_CURRENT_RASTER_POSITION:
        if(DRAW_FRAMEBUFFER_BINDING_NAME == 0 && gc->drawablePrivate->yInverted)
        {
            *fp++ = gc->state.rasterPos.rPos.winPos.x;
            *fp++ = ((GLfloat)gc->drawablePrivate->height - gc->state.rasterPos.rPos.winPos.y);
            *fp++ = gc->state.rasterPos.rPos.winPos.z;
            *fp++ = gc->state.rasterPos.rPos.clipW;
        }else
        {
            *fp++ = gc->state.rasterPos.rPos.winPos.x;
            *fp++ = gc->state.rasterPos.rPos.winPos.y;
            *fp++ = gc->state.rasterPos.rPos.winPos.z;
            *fp++ = gc->state.rasterPos.rPos.clipW;
        }
        break;
    case GL_CURRENT_RASTER_POSITION_VALID:
        *lp++ = gc->state.rasterPos.validRasterPos;
        break;
    case GL_CURRENT_RASTER_DISTANCE:
        *fp++ = gc->state.rasterPos.rPos.eyeDistance;
        break;
    case GL_POINT_SIZE:
        *fp++ = gc->state.point.requestedSize;
        break;
    case GL_ALIASED_POINT_SIZE_RANGE:
        *fp++ = gc->state.point.sizeMin;
        *fp++ = gc->state.point.sizeMax;
        break;
    case GL_POINT_SIZE_RANGE:
        *fp++ = gc->constants.pointSizeMinimum;
        *fp++ = gc->constants.pointSizeMaximum;
        break;
    case GL_POINT_SIZE_MIN:
        *fp++ = gc->constants.pointSizeMinimum;
        break;
    case GL_POINT_SIZE_MAX:
        *fp++ = gc->constants.pointSizeMaximum;
        break;
    case GL_POINT_SIZE_GRANULARITY:
        *fp++ = gc->constants.pointSizeGranularity;
        break;
    case GL_LINE_WIDTH:
        *fp++ = gc->state.line.requestedWidth;
        break;
    case GL_LINE_WIDTH_RANGE:
    case GL_ALIASED_LINE_WIDTH_RANGE:
        *fp++ = gc->constants.lineWidthMinimum;
        *fp++ = gc->constants.lineWidthMaximum;
        break;
    case GL_LINE_WIDTH_GRANULARITY:
        *fp++ = gc->constants.lineWidthGranularity;
        break;
    case GL_LINE_STIPPLE_PATTERN:
        *ip++ = gc->state.line.stipple;
        break;
    case GL_LINE_STIPPLE_REPEAT:
        *ip++ = gc->state.line.stippleRepeat;
        break;
    case GL_POLYGON_MODE:
        *ip++ = gc->state.polygon.frontMode;
        *ip++ = gc->state.polygon.backMode;
        break;
    case GL_EDGE_FLAG:
        *lp++ = gc->state.current.edgeflag;
        break;
    case GL_CULL_FACE_MODE:
        *ip++ = gc->state.polygon.cullFace;
        break;
    case GL_FRONT_FACE:
        *ip++ = gc->state.polygon.frontFace;
        break;
    case GL_LIGHT_MODEL_LOCAL_VIEWER:
        *lp++ = (GLboolean) gc->state.light.model.localViewer;
        break;
    case GL_LIGHT_MODEL_TWO_SIDE:
        *lp++ = (GLboolean) gc->state.light.model.twoSided;
        break;
    case GL_LIGHT_MODEL_AMBIENT:
        *cp++ = gc->state.light.model.ambient.r;
        *cp++ = gc->state.light.model.ambient.g;
        *cp++ = gc->state.light.model.ambient.b;
        *cp++ = gc->state.light.model.ambient.a;
        break;
    case GL_COLOR_MATERIAL_FACE:
        *ip++ = gc->state.light.colorMaterialFace;
        break;
    case GL_COLOR_MATERIAL_PARAMETER:
        *ip++ = gc->state.light.colorMaterialParam;
        break;
    case GL_SHADE_MODEL:
        *ip++ = gc->state.light.shadingModel;
        break;
    case GL_FOG_INDEX:
        *fp++ = gc->state.fog.index;
        break;
    case GL_FOG_DENSITY:
        *fp++ = gc->state.fog.density;
        break;
    case GL_FOG_START:
        *fp++ = gc->state.fog.start;
        break;
    case GL_FOG_END:
        *fp++ = gc->state.fog.end;
        break;
    case GL_FOG_MODE:
        *ip++ = gc->state.fog.mode;
        break;
    case GL_FOG_COLOR:
        {
            __GLcolor fogColor;
            __CHECK_GET_COLOR_CLAMP(&fogColor, &gc->state.fog.color);
            *scp++ = fogColor.r;
            *scp++ = fogColor.g;
            *scp++ = fogColor.b;
            *scp++ = fogColor.a;
        }
        break;
    case GL_DEPTH_RANGE:
        /* These get scaled like colors, to [0, 2^31-1] */
        *cp++ = gc->state.viewport.zNear;
        *cp++ = gc->state.viewport.zFar;
        break;
    case GL_DEPTH_WRITEMASK:
        *lp++ = gc->state.depth.writeEnable;
        break;
    case GL_DEPTH_CLEAR_VALUE:
        /* This gets scaled like colors, to [0, 2^31-1] */
        *cp++ = (GLdouble)gc->state.depth.clear;
        break;
    case GL_DEPTH_FUNC:
        *ip++ = gc->state.depth.testFunc;
        break;
    case GL_ACCUM_CLEAR_VALUE:
        *cp++ = gc->state.accum.clear.r;
        *cp++ = gc->state.accum.clear.g;
        *cp++ = gc->state.accum.clear.b;
        *cp++ = gc->state.accum.clear.a;
        break;
    case GL_STENCIL_CLEAR_VALUE:
        *ip++ = gc->state.stencil.clear;
        break;
    case GL_STENCIL_FUNC:
        *ip++ = gc->state.stencil.current.front.testFunc;
        break;
    case GL_STENCIL_BACK_FUNC:
        *ip++ = gc->state.stencil.current.back.testFunc;
        break;
    case GL_STENCIL_VALUE_MASK:
        *ip++ = gc->state.stencil.current.front.mask;
        break;
    case GL_STENCIL_BACK_VALUE_MASK:
        *ip++ = gc->state.stencil.current.back.mask;
        break;
    case GL_STENCIL_FAIL:
        *ip++ = gc->state.stencil.current.front.fail;
        break;
    case GL_STENCIL_BACK_FAIL:
        *ip++ = gc->state.stencil.current.back.fail;
        break;
    case GL_STENCIL_PASS_DEPTH_FAIL:
        *ip++ = gc->state.stencil.current.front.depthFail;
        break;
    case GL_STENCIL_BACK_PASS_DEPTH_FAIL:
        *ip++ = gc->state.stencil.current.back.depthFail;
        break;
    case GL_STENCIL_PASS_DEPTH_PASS:
        *ip++ = gc->state.stencil.current.front.depthPass;
        break;
    case GL_STENCIL_BACK_PASS_DEPTH_PASS:
        *ip++ = gc->state.stencil.current.back.depthPass;
        break;
    case GL_STENCIL_REF:
        *ip++ = gc->state.stencil.current.front.reference;
        break;
    case GL_STENCIL_BACK_REF:
        *ip++ = gc->state.stencil.current.back.reference;
        break;
    case GL_STENCIL_WRITEMASK:
        *ip++ = gc->state.stencil.current.front.writeMask;
        break;
    case GL_STENCIL_BACK_WRITEMASK:
        *ip++ = gc->state.stencil.current.back.writeMask;
        break;
    case GL_MATRIX_MODE:
        *ip++ = gc->state.transform.matrixMode;
        break;
    case GL_VIEWPORT:
        *ip++ = gc->state.viewport.x;
        *ip++ = gc->state.viewport.y;
        *ip++ = gc->state.viewport.width;
        *ip++ = gc->state.viewport.height;
        break;
    case GL_ATTRIB_STACK_DEPTH:
        *ip++ = (GLint)(gc->attribute.stackPointer - gc->attribute.stack);
        break;
    case GL_MODELVIEW_STACK_DEPTH:
        *ip++ = 1 + (GLint)(gc->transform.modelView - gc->transform.modelViewStack);
        break;
    case GL_PROJECTION_STACK_DEPTH:
        *ip++ = 1 + (GLint)(gc->transform.projection - gc->transform.projectionStack);
        break;
    case GL_TEXTURE_STACK_DEPTH:
        index = gc->state.texture.activeTexIndex;
        *ip++ = 1 + (GLint)(gc->transform.texture[index] - gc->transform.textureStack[index]);
        break;
    case GL_MODELVIEW_MATRIX:
        mp = &gc->transform.modelView->matrix.matrix[0][0];
        *fp++ = *mp++; *fp++ = *mp++; *fp++ = *mp++; *fp++ = *mp++;
        *fp++ = *mp++; *fp++ = *mp++; *fp++ = *mp++; *fp++ = *mp++;
        *fp++ = *mp++; *fp++ = *mp++; *fp++ = *mp++; *fp++ = *mp++;
        *fp++ = *mp++; *fp++ = *mp++; *fp++ = *mp++; *fp++ = *mp++;
        break;
    case GL_TRANSPOSE_MODELVIEW_MATRIX:
        mp = &gc->transform.modelView->matrix.matrix[0][0];
        *fp++ = *mp;        *fp++ = *(mp+4);    *fp++ = *(mp+8);    *fp++ = *(mp+12);
        *fp++ = *(mp+1);    *fp++ = *(mp+5);    *fp++ = *(mp+9);    *fp++ = *(mp+13);
        *fp++ = *(mp+2);    *fp++ = *(mp+6);    *fp++ = *(mp+10);   *fp++ = *(mp+14);
        *fp++ = *(mp+3);    *fp++ = *(mp+7);    *fp++ = *(mp+11);   *fp++ = *(mp+15);
        break;
    case GL_PROJECTION_MATRIX:
        mp = &gc->transform.projection->matrix.matrix[0][0];
        *fp++ = *mp++; *fp++ = *mp++; *fp++ = *mp++; *fp++ = *mp++;
        *fp++ = *mp++; *fp++ = *mp++; *fp++ = *mp++; *fp++ = *mp++;
        *fp++ = *mp++; *fp++ = *mp++; *fp++ = *mp++; *fp++ = *mp++;
        *fp++ = *mp++; *fp++ = *mp++; *fp++ = *mp++; *fp++ = *mp++;
        break;
    case GL_TRANSPOSE_PROJECTION_MATRIX:
        mp = &gc->transform.projection->matrix.matrix[0][0];
        *fp++ = *mp;        *fp++ = *(mp+4);    *fp++ = *(mp+8);    *fp++ = *(mp+12);
        *fp++ = *(mp+1);    *fp++ = *(mp+5);    *fp++ = *(mp+9);    *fp++ = *(mp+13);
        *fp++ = *(mp+2);    *fp++ = *(mp+6);    *fp++ = *(mp+10);   *fp++ = *(mp+14);
        *fp++ = *(mp+3);    *fp++ = *(mp+7);    *fp++ = *(mp+11);   *fp++ = *(mp+15);
        break;
    case GL_TEXTURE_MATRIX:
        index = gc->state.texture.activeTexIndex;
        mp = &(gc->transform.texture[index]->matrix.matrix[0][0]);
        *fp++ = *mp++; *fp++ = *mp++; *fp++ = *mp++; *fp++ = *mp++;
        *fp++ = *mp++; *fp++ = *mp++; *fp++ = *mp++; *fp++ = *mp++;
        *fp++ = *mp++; *fp++ = *mp++; *fp++ = *mp++; *fp++ = *mp++;
        *fp++ = *mp++; *fp++ = *mp++; *fp++ = *mp++; *fp++ = *mp++;
        break;
    case GL_TRANSPOSE_TEXTURE_MATRIX_ARB:
        index = gc->state.texture.activeTexIndex;
        mp = &(gc->transform.texture[index]->matrix.matrix[0][0]);
        *fp++ = *mp;        *fp++ = *(mp+4);    *fp++ = *(mp+8);    *fp++ = *(mp+12);
        *fp++ = *(mp+1);    *fp++ = *(mp+5);    *fp++ = *(mp+9);    *fp++ = *(mp+13);
        *fp++ = *(mp+2);    *fp++ = *(mp+6);    *fp++ = *(mp+10);   *fp++ = *(mp+14);
        *fp++ = *(mp+3);    *fp++ = *(mp+7);    *fp++ = *(mp+11);   *fp++ = *(mp+15);
        break;
    case GL_ALPHA_TEST_FUNC:
        *ip++ = (GLint)gc->state.raster.alphaFunction;
        break;
    case GL_ALPHA_TEST_REF:
        *fp++ = gc->state.raster.alphaReference;
        break;
    case GL_BLEND_DST:
        *ip++ = gc->state.raster.blendDstRGB;
        break;
    case GL_BLEND_COLOR:
        {
            __GLcolor blendColor;
            __CHECK_GET_COLOR_CLAMP(&blendColor, &gc->state.raster.blendColor);

            *cp++ = blendColor.r;
            *cp++ = blendColor.g;
            *cp++ = blendColor.b;
            *cp++ = blendColor.a;
        }
        break;
    case GL_BLEND_SRC:
        *ip++ = gc->state.raster.blendSrcRGB;
        break;
    case GL_LOGIC_OP_MODE:
        *ip++ = gc->state.raster.logicOp;
        break;
    case GL_DRAW_BUFFER:
        *ip++ = gc->state.raster.drawBufferReturn;
        break;
    case GL_READ_BUFFER:
        *ip++ = gc->state.pixel.readBufferReturn;
        break;
    case GL_SCISSOR_BOX:
        *ip++ = gc->state.scissor.scissorX;
        *ip++ = gc->state.scissor.scissorY;
        *ip++ = gc->state.scissor.scissorWidth;
        *ip++ = gc->state.scissor.scissorHeight;
        break;
    case GL_INDEX_CLEAR_VALUE:
        *fp++ = gc->state.raster.clearIndex;
        break;
    case GL_INDEX_MODE:
        *lp++ = GL_FALSE;
        break;
    case GL_INDEX_WRITEMASK:
        *ip++ = gc->state.raster.writeMask;
        break;
    case GL_COLOR_CLEAR_VALUE:
        {
            __GLcolor clearColor;
            __CHECK_GET_COLOR_CLAMP(&clearColor, &gc->state.raster.clear);
            *cp++ = clearColor.r;
            *cp++ = clearColor.g;
            *cp++ = clearColor.b;
            *cp++ = clearColor.a;
        }
        break;
    case GL_RGBA_MODE:
        *lp++ = GL_TRUE;
        break;
    case GL_COLOR_WRITEMASK:
        *lp++ = gc->state.raster.colorMask[0].redMask;
        *lp++ = gc->state.raster.colorMask[0].greenMask;
        *lp++ = gc->state.raster.colorMask[0].blueMask;
        *lp++ = gc->state.raster.colorMask[0].alphaMask;
        break;
    case GL_RENDER_MODE:
        *ip++ = gc->renderMode;
        break;
    case GL_PERSPECTIVE_CORRECTION_HINT:
        *ip++ = gc->state.hints.perspectiveCorrection;
        break;
    case GL_POINT_SMOOTH_HINT:
        *ip++ = gc->state.hints.pointSmooth;
        break;
    case GL_LINE_SMOOTH_HINT:
        *ip++ = gc->state.hints.lineSmooth;
        break;
    case GL_POLYGON_SMOOTH_HINT:
        *ip++ = gc->state.hints.polygonSmooth;
        break;
    case GL_FOG_HINT:
        *ip++ = gc->state.hints.fog;
        break;
    case GL_LIST_BASE:
        *ip++ = gc->state.list.listBase;
        break;
    case GL_LIST_INDEX:
        *ip++ = gc->dlist.currentList;
        break;
    case GL_LIST_MODE:
        *ip++ = gc->dlist.mode;
        break;
    case GL_PACK_SWAP_BYTES:
        *lp++ = (GLboolean)gc->clientState.pixel.packModes.swapEndian;
        break;
    case GL_PACK_LSB_FIRST:
        *lp++ = (GLboolean)gc->clientState.pixel.packModes.lsbFirst;
        break;
    case GL_PACK_ROW_LENGTH:
        *ip++ = gc->clientState.pixel.packModes.lineLength;
        break;
    case GL_PACK_SKIP_ROWS:
        *ip++ = gc->clientState.pixel.packModes.skipLines;
        break;
    case GL_PACK_SKIP_PIXELS:
        *ip++ = gc->clientState.pixel.packModes.skipPixels;
        break;
    case GL_PACK_ALIGNMENT:
        *ip++ = gc->clientState.pixel.packModes.alignment;
        break;
    case GL_PACK_SKIP_IMAGES:
        *ip++ = gc->clientState.pixel.packModes.skipImages;
        break;
    case GL_PACK_IMAGE_HEIGHT:
        *ip++ = gc->clientState.pixel.packModes.imageHeight;
        break;
    case GL_UNPACK_SWAP_BYTES:
        *lp++ = (GLboolean)gc->clientState.pixel.unpackModes.swapEndian;
        break;
    case GL_UNPACK_LSB_FIRST:
        *lp++ = (GLboolean)gc->clientState.pixel.unpackModes.lsbFirst;
        break;
    case GL_UNPACK_ROW_LENGTH:
        *ip++ = gc->clientState.pixel.unpackModes.lineLength;
        break;
    case GL_UNPACK_SKIP_ROWS:
        *ip++ = gc->clientState.pixel.unpackModes.skipLines;
        break;
    case GL_UNPACK_SKIP_PIXELS:
        *ip++ = gc->clientState.pixel.unpackModes.skipPixels;
        break;
    case GL_UNPACK_ALIGNMENT:
        *ip++ = gc->clientState.pixel.unpackModes.alignment;
        break;
    case GL_UNPACK_SKIP_IMAGES:
        *ip++ = gc->clientState.pixel.unpackModes.skipImages;
        break;
    case GL_UNPACK_IMAGE_HEIGHT:
        *ip++ = gc->clientState.pixel.unpackModes.imageHeight;
        break;
    case GL_MAP_COLOR:
        *lp++ = (GLboolean)gc->state.pixel.transferMode.mapColor;
        break;
    case GL_MAP_STENCIL:
        *lp++ = (GLboolean)gc->state.pixel.transferMode.mapStencil;
        break;
    case GL_INDEX_SHIFT:
        *ip++ = gc->state.pixel.transferMode.indexShift;
        break;
    case GL_INDEX_OFFSET:
        *ip++ = gc->state.pixel.transferMode.indexOffset;
        break;
    case GL_RED_SCALE:
        *fp++ = gc->state.pixel.transferMode.r_scale;
        break;
    case GL_GREEN_SCALE:
        *fp++ = gc->state.pixel.transferMode.g_scale;
        break;
    case GL_BLUE_SCALE:
        *fp++ = gc->state.pixel.transferMode.b_scale;
        break;
    case GL_ALPHA_SCALE:
        *fp++ = gc->state.pixel.transferMode.a_scale;
        break;
    case GL_DEPTH_SCALE:
        *fp++ = gc->state.pixel.transferMode.d_scale;
        break;
    case GL_RED_BIAS:
        *fp++ = gc->state.pixel.transferMode.r_bias;
        break;
    case GL_GREEN_BIAS:
        *fp++ = gc->state.pixel.transferMode.g_bias;
        break;
    case GL_BLUE_BIAS:
        *fp++ = gc->state.pixel.transferMode.b_bias;
        break;
    case GL_ALPHA_BIAS:
        *fp++ = gc->state.pixel.transferMode.a_bias;
        break;
    case GL_DEPTH_BIAS:
        *fp++ = gc->state.pixel.transferMode.d_bias;
        break;
    case GL_POST_CONVOLUTION_RED_SCALE:
        *fp++ = gc->state.pixel.transferMode.postConvolutionScale.r;
        break;
    case GL_POST_CONVOLUTION_GREEN_SCALE:
        *fp++ = gc->state.pixel.transferMode.postConvolutionScale.g;
        break;
    case GL_POST_CONVOLUTION_BLUE_SCALE:
        *fp++ = gc->state.pixel.transferMode.postConvolutionScale.b;
        break;
    case GL_POST_CONVOLUTION_ALPHA_SCALE:
        *fp++ = gc->state.pixel.transferMode.postConvolutionScale.a;
        break;
    case GL_POST_CONVOLUTION_RED_BIAS:
        *fp++ = gc->state.pixel.transferMode.postConvolutionBias.r;
        break;
    case GL_POST_CONVOLUTION_GREEN_BIAS:
        *fp++ = gc->state.pixel.transferMode.postConvolutionBias.g;
        break;
    case GL_POST_CONVOLUTION_BLUE_BIAS:
        *fp++ = gc->state.pixel.transferMode.postConvolutionBias.b;
        break;
    case GL_POST_CONVOLUTION_ALPHA_BIAS:
        *fp++ = gc->state.pixel.transferMode.postConvolutionBias.a;
        break;
    case GL_ZOOM_X:
        *fp++ = gc->state.pixel.transferMode.zoomX;
        break;
    case GL_ZOOM_Y:
        *fp++ = gc->state.pixel.transferMode.zoomY;
        break;
    case GL_PIXEL_MAP_I_TO_I_SIZE:
    case GL_PIXEL_MAP_S_TO_S_SIZE:
    case GL_PIXEL_MAP_I_TO_R_SIZE:
    case GL_PIXEL_MAP_I_TO_G_SIZE:
    case GL_PIXEL_MAP_I_TO_B_SIZE:
    case GL_PIXEL_MAP_I_TO_A_SIZE:
    case GL_PIXEL_MAP_R_TO_R_SIZE:
    case GL_PIXEL_MAP_G_TO_G_SIZE:
    case GL_PIXEL_MAP_B_TO_B_SIZE:
    case GL_PIXEL_MAP_A_TO_A_SIZE:
        index = sq - GL_PIXEL_MAP_I_TO_I_SIZE;
        *ip++ = gc->state.pixel.pixelMap[index].size;
        break;
    case GL_MAX_EVAL_ORDER:
        *ip++ = gc->constants.maxEvalOrder;
        break;
    case GL_MAX_LIGHTS:
        *ip++ = gc->constants.numberOfLights;
        break;
    case GL_MAX_CLIP_PLANES:
        *ip++ = gc->constants.numberOfClipPlanes;
        break;
    case GL_MAX_PIXEL_MAP_TABLE:
        *ip++ = gc->constants.maxPixelMapTable;
        break;
    case GL_MAX_ATTRIB_STACK_DEPTH:
        *ip++ = gc->constants.maxAttribStackDepth;
        break;
    case GL_MAX_CLIENT_ATTRIB_STACK_DEPTH:
        *ip++ = gc->constants.maxClientAttribStackDepth;
        break;
    case GL_MAX_MODELVIEW_STACK_DEPTH:
        *ip++ = gc->constants.maxModelViewStackDepth;
        break;
    case GL_MAX_NAME_STACK_DEPTH:
        *ip++ = gc->constants.maxNameStackDepth;
        break;
    case GL_MAX_PROJECTION_STACK_DEPTH:
        *ip++ = gc->constants.maxProjectionStackDepth;
        break;
    case GL_MAX_TEXTURE_STACK_DEPTH:
        *ip++ = gc->constants.maxTextureStackDepth;
        break;
    case GL_INDEX_BITS:
        *ip++ = gc->modes.rgbaBits;
        break;

    case GL_RED_BITS:
        if (DRAW_FRAMEBUFFER_BINDING_NAME != 0)
        {
            *ip++ = __glGetFboColorBits(gc,GL_RED_BITS);
        }
        else
        {
            *ip++ = gc->modes.redBits;
        }
        break;
    case GL_GREEN_BITS:
        if (DRAW_FRAMEBUFFER_BINDING_NAME != 0)
        {
            *ip++ = __glGetFboColorBits(gc,GL_GREEN_BITS);
        }
        else
        {
            *ip++ = gc->modes.greenBits;
        }
        break;
    case GL_BLUE_BITS:
        if (DRAW_FRAMEBUFFER_BINDING_NAME != 0)
        {
            *ip++ = __glGetFboColorBits(gc,GL_BLUE_BITS);
        }
        else
        {
            *ip++ = gc->modes.blueBits;
        }
        break;
    case GL_ALPHA_BITS:
        if (DRAW_FRAMEBUFFER_BINDING_NAME != 0)
        {
            *ip++ = __glGetFboColorBits(gc,GL_ALPHA_BITS);
        }
        else
        {
            *ip++ = gc->modes.alphaBits;
        }
        break;
    case GL_DEPTH_BITS:
        if (DRAW_FRAMEBUFFER_BINDING_NAME != 0)
        {
            *ip++ = __glGetFboColorBits(gc,GL_DEPTH_BITS);
        }
        else
        {
            *ip++ = gc->modes.depthBits;
        }
        break;
    case GL_STENCIL_BITS:
        if (DRAW_FRAMEBUFFER_BINDING_NAME != 0)
        {
            *ip++ = __glGetFboColorBits(gc,GL_STENCIL_BITS);
        }
        else
        {
             *ip++ = gc->modes.stencilBits;
        }
        break;

    case GL_ACCUM_RED_BITS:
        *ip++ = gc->modes.accumRedBits;
        break;
    case GL_ACCUM_GREEN_BITS:
        *ip++ = gc->modes.accumGreenBits;
        break;
    case GL_ACCUM_BLUE_BITS:
        *ip++ = gc->modes.accumBlueBits;
        break;
    case GL_ACCUM_ALPHA_BITS:
        *ip++ = gc->modes.accumAlphaBits;
        break;
    case GL_MAP1_GRID_DOMAIN:
        *fp++ = gc->state.evaluator.u1.start;
        *fp++ = gc->state.evaluator.u1.finish;
        break;
    case GL_MAP1_GRID_SEGMENTS:
        *ip++ = gc->state.evaluator.u1.n;
        break;
    case GL_MAP2_GRID_DOMAIN:
        *fp++ = gc->state.evaluator.u2.start;
        *fp++ = gc->state.evaluator.u2.finish;
        *fp++ = gc->state.evaluator.v2.start;
        *fp++ = gc->state.evaluator.v2.finish;
        break;
    case GL_MAP2_GRID_SEGMENTS:
        *ip++ = gc->state.evaluator.u2.n;
        *ip++ = gc->state.evaluator.v2.n;
        break;
    case GL_NAME_STACK_DEPTH:
        *ip++ = (GLint)(gc->select.sp - gc->select.stack);
        break;
    case GL_MAX_VIEWPORT_DIMS:
        *ip++ = gc->constants.maxViewportWidth;
        *ip++ = gc->constants.maxViewportHeight;
        break;
    case GL_DOUBLEBUFFER:
        *lp++ = gc->modes.doubleBufferMode ? GL_TRUE : GL_FALSE;
        break;
    case GL_AUX_BUFFERS:
        *ip++ = gc->modes.numAuxBuffers;
        break;
    case GL_STEREO:
        *lp++ = GL_FALSE;
        break;
    case GL_POLYGON_OFFSET_FACTOR:
        *fp++ = gc->state.polygon.factor;
        break;
    case GL_POLYGON_OFFSET_UNITS:
        *fp++ = gc->state.polygon.units;
        break;
    case GL_TEXTURE_BINDING_1D:
        index = gc->state.texture.activeTexIndex;
        *ip++ = gc->texture.units[index].boundTextures[__GL_TEXTURE_1D_INDEX]->name;
        break;
    case GL_TEXTURE_BINDING_1D_ARRAY_EXT:
        index = gc->state.texture.activeTexIndex;
        *ip++ = gc->texture.units[index].boundTextures[__GL_TEXTURE_1D_ARRAY_INDEX]->name;
        break;
    case GL_TEXTURE_BINDING_2D:
        index = gc->state.texture.activeTexIndex;
        *ip++ = gc->texture.units[index].boundTextures[__GL_TEXTURE_2D_INDEX]->name;
        break;
    case GL_TEXTURE_BINDING_2D_ARRAY_EXT:
        index = gc->state.texture.activeTexIndex;
        *ip++ = gc->texture.units[index].boundTextures[__GL_TEXTURE_2D_ARRAY_INDEX]->name;
        break;
    case GL_MAX_ARRAY_TEXTURE_LAYERS_EXT:
        *ip++ = gc->constants.maxTextureArraySize;
        break;
    case GL_TEXTURE_BINDING_3D:
        index = gc->state.texture.activeTexIndex;
        *ip++ = gc->texture.units[index].boundTextures[__GL_TEXTURE_3D_INDEX]->name;
        break;
    case GL_TEXTURE_BINDING_CUBE_MAP:
        index = gc->state.texture.activeTexIndex;
        *ip++ = gc->texture.units[index].boundTextures[__GL_TEXTURE_CUBEMAP_INDEX]->name;
        break;
    case GL_TEXTURE_BINDING_BUFFER_EXT:
        index = gc->state.texture.activeTexIndex;
        *ip++ = gc->texture.units[index].boundTextures[__GL_TEXTURE_BUFFER_INDEX]->name;
        break;
    case GL_TEXTURE_BUFFER_FORMAT_EXT:
        index = gc->state.texture.activeTexIndex;
        *ip++ = gc->texture.units[index].boundTextures[__GL_TEXTURE_BUFFER_INDEX]->faceMipmap[0][0].requestedFormat;
        break;
    case GL_TEXTURE_BUFFER_DATA_STORE_BINDING_EXT:
        index = gc->state.texture.activeTexIndex;
        if(gc->texture.units[index].boundTextures[__GL_TEXTURE_BUFFER_INDEX]->bufferObj)
            *ip++ = gc->texture.units[index].boundTextures[__GL_TEXTURE_BUFFER_INDEX]->bufferObj->name;
        else
            *ip++ = 0;
        break;
    case GL_TEXTURE_BUFFER_EXT:
        *ip++ = gc->bufferObject.boundBuffer[__GL_TEXTURE_BUFFER_EXT_INDEX];
        break;
    case GL_MAX_TEXTURE_BUFFER_SIZE_EXT:
        *ip++ = gc->constants.maxTextureBufferSize;
        break;
    case GL_VERTEX_ARRAY_SIZE:
        *ip++ = gc->clientState.vertexArray.vertex.size;
        break;
    case GL_VERTEX_ARRAY_TYPE:
        *ip++ = gc->clientState.vertexArray.vertex.type;
        break;
    case GL_VERTEX_ARRAY_STRIDE:
        *ip++ = gc->clientState.vertexArray.vertex.usr_stride;
        break;
    case GL_NORMAL_ARRAY_TYPE:
        *ip++ = gc->clientState.vertexArray.normal.type;
        break;
    case GL_NORMAL_ARRAY_STRIDE:
        *ip++ = gc->clientState.vertexArray.normal.usr_stride;
        break;
    case GL_COLOR_ARRAY_SIZE:
        *ip++ = gc->clientState.vertexArray.color.size;
        break;
    case GL_COLOR_ARRAY_TYPE:
        *ip++ = gc->clientState.vertexArray.color.type;
        break;
    case GL_COLOR_ARRAY_STRIDE:
        *ip++ = gc->clientState.vertexArray.color.usr_stride;
        break;
    case GL_INDEX_ARRAY_TYPE:
        *ip++ = gc->clientState.vertexArray.colorindex.type;
        break;
    case GL_INDEX_ARRAY_STRIDE:
        *ip++ = gc->clientState.vertexArray.colorindex.usr_stride;
        break;
    case GL_TEXTURE_COORD_ARRAY_SIZE:
        index = gc->clientState.vertexArray.clientActiveUnit;
        *ip++ = gc->clientState.vertexArray.texture[index].size;
        break;
    case GL_TEXTURE_COORD_ARRAY_TYPE:
        index = gc->clientState.vertexArray.clientActiveUnit;
        *ip++ = gc->clientState.vertexArray.texture[index].type;
        break;
    case GL_TEXTURE_COORD_ARRAY_STRIDE:
        index = gc->clientState.vertexArray.clientActiveUnit;
        *ip++ = gc->clientState.vertexArray.texture[index].usr_stride;
        break;
    case GL_EDGE_FLAG_ARRAY_STRIDE:
        *ip++ = gc->clientState.vertexArray.edgeflag.usr_stride;
        break;
    case GL_FEEDBACK_BUFFER_SIZE:
        *ip++ = gc->feedback.resultLength;
        break;
    case GL_FEEDBACK_BUFFER_TYPE:
        *ip++ = gc->feedback.type;
        break;
    case GL_SELECTION_BUFFER_SIZE:
        *ip++ = gc->select.bufferMaxCount;
        break;
    case GL_MAX_ELEMENTS_VERTICES:
        *ip++  = gc->constants.maxElementsVertices;
        break;
    case GL_MAX_ELEMENTS_INDICES:
        *ip++  = gc->constants.maxElementsIndices;
        break;
    case GL_CLIENT_ACTIVE_TEXTURE:
        *ip++ = GL_TEXTURE0 + gc->clientState.vertexArray.clientActiveUnit;
        break;
    case GL_ACTIVE_TEXTURE:
        *ip++ = GL_TEXTURE0 +  gc->state.texture.activeTexIndex;
        break;
    case GL_MAX_TEXTURE_UNITS:
        *ip++ = gc->constants.numberOfTextureUnits;
        break;
    case GL_LIGHT_MODEL_COLOR_CONTROL:
        *ip++ = gc->state.light.model.colorControl;
        break;
    case GL_CURRENT_SECONDARY_COLOR:
        *cp++ = gc->state.current.color2.r;
        *cp++ = gc->state.current.color2.g;
        *cp++ = gc->state.current.color2.b;
        *cp++ = 0;
        break;
    case GL_SECONDARY_COLOR_ARRAY_SIZE:
        *ip++ = gc->clientState.vertexArray.color2.size;
        break;
    case GL_SECONDARY_COLOR_ARRAY_TYPE:
        *ip++ = gc->clientState.vertexArray.color2.type;
        break;
    case GL_SECONDARY_COLOR_ARRAY_STRIDE:
        *ip++ = gc->clientState.vertexArray.color2.usr_stride;
        break;
    case GL_FOG_COORDINATE_SOURCE:
        *ip++ = gc->state.fog.coordSource;
        break;
    case GL_CURRENT_FOG_COORDINATE:
        *fp++ = gc->state.current.fog;
        break;
    case GL_FOG_COORDINATE_ARRAY_TYPE:
        *ip++ = gc->clientState.vertexArray.fogcoord.type;
        break;
    case GL_FOG_COORDINATE_ARRAY_STRIDE:
        *ip++ = gc->clientState.vertexArray.fogcoord.usr_stride;
        break;
    case GL_BLEND_EQUATION:
        *ip++ = gc->state.raster.blendEquationRGB;
        break;
    case GL_MAX_TEXTURE_LOD_BIAS:
        *fp++ = gc->constants.maxTextureLodBias;
        break;


    case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
        {
            GLint count  = (gc->dp.querySupportedCompressedTextureFormats)(gc, NULL);
            *ip++ = count;
        }
        break;
    case GL_COMPRESSED_TEXTURE_FORMATS:
        {
            GLint count = (gc->dp.querySupportedCompressedTextureFormats)(gc, ip);
            ip += count;
        }
        break;


#if GL_EXT_depth_bounds_test
    case GL_DEPTH_BOUNDS_EXT:
        *fp++ = gc->state.depthBoundTest.zMin;
        *fp++ = gc->state.depthBoundTest.zMax;
        break;
#endif

#if GL_ARB_fragment_program
    case GL_MAX_TEXTURE_IMAGE_UNITS_ARB:
        *ip++ = gc->constants.maxCombinedTextureImageUnits - gc->constants.maxVSTextureImageUnits;
        break;
    case GL_MAX_TEXTURE_COORDS_ARB:
        *ip++ =  gc->constants.numberOfTextureUnits;
        break;
#endif

    case GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT:
        *ip++ = gc->constants.maxTextureMaxAnisotropy;
        break;

    case GL_SAMPLE_BUFFERS:
        *ip++ = gc->modes.sampleBuffers;
        break;
    case GL_SAMPLES:
        *ip++ = gc->modes.samples;
        break;
    case GL_SAMPLE_COVERAGE_VALUE:
        *fp++ = gc->state.multisample.coverageValue;
        break;
    case GL_SAMPLE_COVERAGE_INVERT:
        *lp++ = gc->state.multisample.coverageInvert;
        break;
    case GL_MULTISAMPLE:
        *lp++ = gc->state.enables.multisample.multisampleOn;
        break;
    case GL_SAMPLE_ALPHA_TO_COVERAGE:
        *lp++ = gc->state.multisample.alphaToCoverage;
        break;
    case GL_SAMPLE_ALPHA_TO_ONE:
        *lp++ = gc->state.multisample.alphaToOne;
        break;
    case GL_SAMPLE_COVERAGE:
        *lp++ = gc->state.multisample.coverage;
        break;
    case GL_ARRAY_BUFFER_BINDING:
        *ip++ = gc->bufferObject.boundBuffer[__GL_ARRAY_BUFFER_INDEX];
        break;
    case GL_ELEMENT_ARRAY_BUFFER_BINDING:
        *ip++ = gc->bufferObject.boundBuffer[__GL_ELEMENT_ARRAY_BUFFER_INDEX];
        break;
    case GL_VERTEX_ARRAY_BUFFER_BINDING:
        *ip++ = gc->clientState.vertexArray.vertex.bufBinding;
        break;
    case GL_NORMAL_ARRAY_BUFFER_BINDING:
        *ip++ = gc->clientState.vertexArray.normal.bufBinding;
        break;
    case GL_COLOR_ARRAY_BUFFER_BINDING:
        *ip++ = gc->clientState.vertexArray.color.bufBinding;
        break;
    case GL_INDEX_ARRAY_BUFFER_BINDING:
        *ip++ = 0;
        break;
    case GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING:
        index = gc->clientState.vertexArray.clientActiveUnit;
        *ip++ = gc->clientState.vertexArray.texture[index].bufBinding;
        break;
    case GL_EDGE_FLAG_ARRAY_BUFFER_BINDING:
        *ip++ = 0;
        break;
    case GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING:
        *ip++ = gc->clientState.vertexArray.color2.bufBinding;
        break;
    case GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING:
        *ip++ = gc->clientState.vertexArray.fogcoord.bufBinding;
        break;
#if GL_ATI_element_array
    case GL_ELEMENT_ARRAY_TYPE_ATI:
        *ip++ = gc->clientState.vertexArray.elementType;
        break;
#endif
    case GL_CURRENT_PROGRAM:
        if (gc->shaderProgram.currentShaderProgram)
        {
            *ip++ = gc->shaderProgram.currentShaderProgram->objectInfo.id;
        }
        else
        {
            *ip++ = 0;
        }
        break;
        /*#if GL_EXT_framebuffer_object        */
    case GL_MAX_COLOR_ATTACHMENTS_EXT:
        *ip++ = __GL_MAX_COLOR_ATTACHMENTS;
        break;
    case GL_MAX_RENDERBUFFER_SIZE_EXT:
        *ip++ = gc->constants.maxRenderBufferWidth;
        break;
    case GL_MAX_SAMPLES_EXT:
        *ip++ = gc->constants.maxSamples;
        break;
    case GL_DRAW_FRAMEBUFFER_BINDING_EXT /* alias GL_FRAMEBUFFER_BINDING_EXT */:
        *ip++ = gc->frameBuffer.drawFramebufObj->name;
        break;
    case GL_READ_FRAMEBUFFER_BINDING_EXT:
        *ip++ = gc->frameBuffer.readFramebufObj->name;
        break;
    case GL_RENDERBUFFER_BINDING_EXT:
        *ip++ = gc->frameBuffer.boundRenderbufObj->name;
        break;
        /*#endif        */

#if GL_EXT_bindable_uniform
    case GL_MAX_VERTEX_BINDABLE_UNIFORMS_EXT:
        *ip++ = gc->constants.maxVertexBindableUniform;
        break;
    case GL_MAX_FRAGMENT_BINDABLE_UNIFORMS_EXT:
        *ip++ = gc->constants.maxFragmentBindableUniform;
        break;
    case GL_MAX_GEOMETRY_BINDABLE_UNIFORMS_EXT:
        *ip++ = gc->constants.maxGeometryBindableUniform;
        break;
    case GL_MAX_BINDABLE_UNIFORM_SIZE_EXT:
        *ip++ = gc->constants.maxBindableUniformSize;
        break;
    case GL_UNIFORM_BUFFER_BINDING_EXT:
        *ip++ = gc->bufferObject.boundBuffer[__GL_UNIFORM_BUFFER_INDEX];
        break;
#endif

#if GL_EXT_gpu_shader4
    case GL_MIN_PROGRAM_TEXEL_OFFSET_EXT:
        *ip++ = gc->constants.minProgramTexelOffset;
        break;
    case GL_MAX_PROGRAM_TEXEL_OFFSET_EXT:
        *ip++ = gc->constants.maxProgramTexelOffset;
        break;
#endif

    case GL_MAX_DRAW_BUFFERS:
        *ip++ = gc->constants.maxDrawBuffers;
        break;
    case GL_DRAW_BUFFER0:
    case GL_DRAW_BUFFER1:
    case GL_DRAW_BUFFER2:
    case GL_DRAW_BUFFER3:
    case GL_DRAW_BUFFER4:
    case GL_DRAW_BUFFER5:
    case GL_DRAW_BUFFER6:
    case GL_DRAW_BUFFER7:
    case GL_DRAW_BUFFER8:
    case GL_DRAW_BUFFER9:
    case GL_DRAW_BUFFER10:
    case GL_DRAW_BUFFER11:
    case GL_DRAW_BUFFER12:
    case GL_DRAW_BUFFER13:
    case GL_DRAW_BUFFER14:
    case GL_DRAW_BUFFER15:
        index = sq - GL_DRAW_BUFFER0;
        if(index < (GLint)gc->constants.maxDrawBuffers)
            *ip++ = gc->state.raster.drawBuffer[index];
        else
            *ip++ = GL_NONE;
        break;

    case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:
        *ip++ = gc->constants.maxVSTextureImageUnits;
        break;

    case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:
        *ip++ = gc->constants.maxCombinedTextureImageUnits;
        break;

    case GL_MAX_VERTEX_UNIFORM_COMPONENTS:
        *ip++ = gc->constants.maxVertexUniformComponents;
        break;

    case GL_MAX_FRAGMENT_UNIFORM_COMPONENTS:
        *ip++ = gc->constants.maxFragmentUniformComponents;
        break;

#if GL_ARB_texture_rectangle
    case GL_TEXTURE_BINDING_RECTANGLE_ARB:
        index = gc->state.texture.activeTexIndex;
        *ip++ = gc->texture.units[index].boundTextures[__GL_TEXTURE_RECTANGLE_INDEX]->name;
        break;
    case GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB:
        *ip++ = gc->constants.maxTextureSize;
        break;
#endif

#if GL_EXT_texture_integer
    case GL_RGBA_INTEGER_MODE_EXT:
        *lp++ = gc->frameBuffer.drawFramebufObj->fbInteger; /*maybe not evaluate*/
        break;
#endif

#if GL_EXT_geometry_shader4
    case GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT:
        *ip++ = gc->constants.maxGeometryTextureImageUnits;
        break;
    case GL_MAX_GEOMETRY_VARYING_COMPONENTS_EXT:
        *ip++ = gc->constants.maxGeometryVaryingComponents;
        break;

    case GL_MAX_VERTEX_VARYING_COMPONENTS_EXT:
        *ip++ = gc->constants.maxVertexVaryingComponents;
        break;
    case GL_MAX_VARYING_COMPONENTS_EXT:
        *ip++ = gc->constants.maxVaryingComponents;
        break;
    case GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT:
        *ip++ = gc->constants.maxGeometryUniformComponents;
        break;
    case GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT:
        *ip++ = gc->constants.maxGeometryOutputVertices;
        break;
    case GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_EXT:
        *ip++ = gc->constants.maxGeometryTotalOuputComponents;
        break;
#endif

#if GL_ARB_color_buffer_float
    case GL_RGBA_FLOAT_MODE_ARB:
        *ip++ = gc->modes.rgbFloatMode;
        break;
    case GL_CLAMP_VERTEX_COLOR_ARB:
        *ip++ = gc->state.light.clampVertexColor;
        break;
    case GL_CLAMP_FRAGMENT_COLOR_ARB:
        *ip++ = gc->state.raster.clampFragColor;
        break;
    case GL_CLAMP_READ_COLOR_ARB:
        *ip++ = gc->state.raster.clampReadColor;
        break;
#endif

    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    /* Use the motion of the pointers to type convert the result */
    if (ip != itemp) {
        __glConvertResult(gc, __GL_INT32, itemp, type, result, (GLint)(ip - itemp));
    }
    else if (fp != ftemp) {
        __glConvertResult(gc, __GL_FLOAT, ftemp, type, result, (GLint)(fp - ftemp));
    }
    else if (lp != ltemp) {
        __glConvertResult(gc, __GL_BOOLEAN, ltemp, type, result, (GLint)(lp - ltemp));
    }
    else if (cp != ctemp) {
        __glConvertResult(gc, __GL_COLOR, ctemp, type, result, (GLint)(cp - ctemp));
    }
    else if (scp != sctemp) {
        __glConvertResult(gc, __GL_SCOLOR, sctemp, type, result, (GLint)(scp - sctemp));
    }
}

GLvoid APIENTRY __glim_GetDoublev(GLenum sq, GLdouble result[])
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetDoublev", DT_GLenum, sq, DT_GLdouble_ptr, result, DT_GLnull);
#endif

    __glDoGet(sq, (GLvoid *)result, (GLint)__GL_FLOAT64, (const GLbyte *)"glGetDoublev");
}

GLvoid APIENTRY __glim_GetFloatv(GLenum sq, GLfloat result[])
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetFloatv", DT_GLenum, sq, DT_GLfloat_ptr, result, DT_GLnull);
#endif

    __glDoGet(sq, (GLvoid *)result, (GLint)__GL_FLOAT32, (const GLbyte *)"glGetFloatv");
}

GLvoid APIENTRY __glim_GetIntegerv(GLenum sq, GLint result[])
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetIntegerv", DT_GLenum, sq, DT_GLint_ptr, result, DT_GLnull);
#endif

    __glDoGet(sq, (GLvoid *)result, (GLint)__GL_INT32, (const GLbyte *)"glGetIntegerv");
}

GLvoid APIENTRY __glim_GetBooleanv(GLenum sq, GLboolean result[])
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetBooleanv", DT_GLenum, sq, DT_GLboolean_ptr, result, DT_GLnull);
#endif

    __glDoGet(sq, (GLvoid *)result, (GLint)__GL_BOOLEAN, (const GLbyte *)"glGetBooleanv");
}

/*
** Return the current error code.
*/
GLenum APIENTRY __glim_GetError(GLvoid)
{
    GLint error;
    __GL_SETUP_NOT_IN_BEGIN_RET(0);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetError", DT_GLnull);
#endif

    error = gc->error;
    gc->error = 0;

    return(error);
}

/*
** Return a pointer to the requested string
*/
const GLubyte * APIENTRY __glim_GetString(GLenum name)
{
    __GL_SETUP_NOT_IN_BEGIN_RET(0);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetString", DT_GLenum, name, DT_GLnull);
#endif

    switch (name)
    {
    case GL_VENDOR:
        return(GLubyte*) gc->constants.vendor;
    case GL_RENDERER:
        return(GLubyte*) gc->constants.renderer;
    case GL_VERSION:
        return(GLubyte*) gc->constants.version;
    case GL_EXTENSIONS:
        {
            return(GLubyte*) gc->constants.extensions;
        }
#if GL_ARB_vertex_program || GL_ARB_fragment_program
    case GL_PROGRAM_ERROR_STRING_ARB:
        return (GLubyte*) gc->program.errStr;
#endif
    case GL_SHADING_LANGUAGE_VERSION:
        return (GLubyte*) gc->constants.GLSLVersion;
        break;

    default:
        __glSetError(GL_INVALID_ENUM);
        return(0);
    }
}


GLvoid APIENTRY __glim_GetPointerv(GLenum pname, GLvoid **params)
{
    GLint index;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetPointerv", DT_GLenum, pname, DT_GLvoid_ptr, params, DT_GLnull);
#endif

    switch (pname)
    {
    case GL_VERTEX_ARRAY_POINTER:
        *params = (GLvoid *)gc->clientState.vertexArray.vertex.pointer;
        return;
    case GL_NORMAL_ARRAY_POINTER:
        *params = (GLvoid *)gc->clientState.vertexArray.normal.pointer;
        return;
    case GL_COLOR_ARRAY_POINTER:
        *params = (GLvoid *)gc->clientState.vertexArray.color.pointer;
        return;
    case GL_INDEX_ARRAY_POINTER:
        *params = (GLvoid *)gc->clientState.vertexArray.colorindex.pointer;
        return;
    case GL_TEXTURE_COORD_ARRAY_POINTER:
        index = gc->clientState.vertexArray.clientActiveUnit;
        *params = (GLvoid *)gc->clientState.vertexArray.texture[index].pointer;
        return;
    case GL_EDGE_FLAG_ARRAY_POINTER:
        *params = (GLvoid *)gc->clientState.vertexArray.edgeflag.pointer;
        return;
    case GL_SECONDARY_COLOR_ARRAY_POINTER_EXT:
        *params = (GLvoid *)gc->clientState.vertexArray.color2.pointer;
        return;
    case GL_FOG_COORDINATE_ARRAY_POINTER_EXT:
        *params = (GLvoid *)gc->clientState.vertexArray.fogcoord.pointer;
        return;
    case GL_SELECTION_BUFFER_POINTER:
        *params = (GLvoid *)gc->select.buffer;
        return;
    case GL_FEEDBACK_BUFFER_POINTER:
        *params = (GLvoid *)gc->feedback.resultBase;
        return;
#if GL_ATI_element_array
    case GL_ELEMENT_ARRAY_POINTER_ATI:
        *params = (GLvoid *)gc->clientState.vertexArray.elementPointer;
        break;
#endif
    default:
        __glSetError(GL_INVALID_OPERATION);
        return;
    }
}

GLvoid APIENTRY __glim_GetPointervEXT(GLenum pname, GLvoid **params)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetPointervEXT", DT_GLenum, pname, DT_GLvoid_ptr, params, DT_GLnull);
#endif

    __glim_GetPointerv(pname, params);
}

GLvoid APIENTRY __glim_GetClipPlane(GLenum plane, GLdouble *eqn)
{
    GLint index;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetClipPlane", DT_GLenum, plane, DT_GLdouble_ptr, eqn, DT_GLnull);
#endif

    index = plane - GL_CLIP_PLANE0;
    if ((index < 0) || (index >= (GLint)gc->constants.numberOfClipPlanes)) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    eqn[0] = gc->state.transform.eyeClipPlanes[index].x;
    eqn[1] = gc->state.transform.eyeClipPlanes[index].y;
    eqn[2] = gc->state.transform.eyeClipPlanes[index].z;
    eqn[3] = gc->state.transform.eyeClipPlanes[index].w;
}
#if GL_EXT_draw_buffers2

GLvoid APIENTRY __glim_GetBooleanIndexedvEXT(GLenum value, GLuint index, GLboolean *data)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetBooleanIndexedvEXT", DT_GLenum, value, DT_GLuint, index, DT_GLboolean_ptr, data);
#endif

    if(index >= gc->constants.maxDrawBuffers)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    switch(value)
    {
    case GL_COLOR_WRITEMASK:
        *data++ = gc->state.raster.colorMask[index].redMask;
        *data++ = gc->state.raster.colorMask[index].greenMask;
        *data++ = gc->state.raster.colorMask[index].blueMask;
        *data++ = gc->state.raster.colorMask[index].alphaMask;
        break;
    case GL_BLEND:
        *data = gc->state.enables.colorBuffer.blend[index];
        break;
    default:
        __glDoGet(value, (GLvoid *)data, (GLint)__GL_BOOLEAN, (const GLbyte *)"glGetBooleanIndexedvEXT");
        break;
    }

}

GLvoid APIENTRY __glim_GetIntegerIndexedvEXT(GLenum value, GLuint index, GLint* data)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetIntegerIndexedvEXT", DT_GLenum, value, DT_GLuint, index, DT_GLint_ptr, data);
#endif

    if(index >= gc->constants.maxDrawBuffers)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    switch(value)
    {
    case GL_COLOR_WRITEMASK:
        *data++ = gc->state.raster.colorMask[index].redMask;
        *data++ = gc->state.raster.colorMask[index].greenMask;
        *data++ = gc->state.raster.colorMask[index].blueMask;
        *data++ = gc->state.raster.colorMask[index].alphaMask;
        break;
    case GL_BLEND:
        *data = gc->state.enables.colorBuffer.blend[index];
        break;
    default:
        __glDoGet(value, (GLvoid *)data, (GLint)__GL_INT32, (const GLbyte *)"glGetIntegerIndexedvEXT");
        break;
    }

}
#endif
#if defined(_WIN32)
#pragma warning(default: 4244)
#endif

