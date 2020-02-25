/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_es_context.h"
#include "gc_es_object_inline.c"

#define _GC_OBJ_ZONE gcdZONE_GL40_CORE


extern GLboolean __glCheckPBO(__GLcontext *gc,
                              __GLpixelPackMode *packMode,
                              __GLbufferObject *bufObj,
                              GLsizei width,
                              GLsizei height,
                              GLsizei depth,
                              GLenum format,
                              GLenum type,
                              const GLvoid *buf);

extern  GLuint __glPixelSize(__GLcontext *gc, GLenum format, GLenum type);

#ifdef OPENGL40
GLvoid __glInitDefaultPixelMap(__GLcontext *gc, GLenum map)
{
    __GLpixelState *ps = &gc->state.pixel;
    __GLpixelMapHead *pMap = ps->pixelMap;
    GLint index = map - GL_PIXEL_MAP_I_TO_I;

    switch (map)
    {
      case GL_PIXEL_MAP_I_TO_I:
      case GL_PIXEL_MAP_S_TO_S:
          /*
          ** Allocate single-entry map for index type.
          */
          pMap[index].base.mapI = (GLint*)(*gc->imports.malloc)(gc, sizeof(GLint));
          if (!pMap[index].base.mapI)
          {
            return;
          }
          else
          {
              pMap[index].base.mapI[0] = 0;
              pMap[index].size = 1;
          }
          break;
      case GL_PIXEL_MAP_I_TO_R: case GL_PIXEL_MAP_I_TO_G:
      case GL_PIXEL_MAP_I_TO_B: case GL_PIXEL_MAP_I_TO_A:
      case GL_PIXEL_MAP_R_TO_R: case GL_PIXEL_MAP_G_TO_G:
      case GL_PIXEL_MAP_B_TO_B: case GL_PIXEL_MAP_A_TO_A:
          /*
          ** Allocate single-entry map for component type.
          */
          pMap[index].base.mapF = (__GLfloat*)(*gc->imports.malloc)(gc, sizeof(__GLfloat));
          if (!pMap[index].base.mapF)
          {
                  return;
          }
          else
          {
              pMap[index].base.mapF[0] = __glZero;
              pMap[index].size = 1;
          }
          break;
      default:
          __glSetError(gc, GL_INVALID_ENUM);
          return;
    }
}

#endif

GLvoid __glInitPixelState(__GLcontext *gc)
{
    __GLclientPixelState *cps = &gc->clientState.pixel;
    __GLpixelState *ps = &gc->state.pixel;
    GLenum m;

    cps->packModes.alignment = 4;
    cps->packModes.lineLength = 0;
    cps->packModes.imageHeight = 0;
    cps->packModes.skipPixels = 0;
    cps->packModes.skipLines = 0;
    cps->packModes.skipImages = 0;

    cps->unpackModes.alignment = 4;
    cps->unpackModes.lineLength = 0;
    cps->unpackModes.imageHeight = 0;
    cps->unpackModes.skipPixels = 0;
    cps->unpackModes.skipLines = 0;
    cps->unpackModes.skipImages = 0;

#ifdef OPENGL40
    if (gc->imports.conformGLSpec)
    {
        cps->unpackModes.swapEndian = GL_FALSE;
        cps->unpackModes.lsbFirst = GL_FALSE;
        cps->packModes.swapEndian = GL_FALSE;
        cps->packModes.lsbFirst = GL_FALSE;
        ps->transferMode.r_scale = 1.0;
        ps->transferMode.g_scale = 1.0;
        ps->transferMode.b_scale = 1.0;
        ps->transferMode.a_scale = 1.0;
        ps->transferMode.d_scale = 1.0;
        ps->transferMode.zoomX = 1.0;
        ps->transferMode.zoomY = 1.0;

        ps->transferMode.postColorMatrixScale.r = 1.0;
        ps->transferMode.postColorMatrixScale.g = 1.0;
        ps->transferMode.postColorMatrixScale.b = 1.0;
        ps->transferMode.postColorMatrixScale.a = 1.0;
        ps->transferMode.postColorMatrixBias.r  = 0.0;
        ps->transferMode.postColorMatrixBias.g  = 0.0;
        ps->transferMode.postColorMatrixBias.b  = 0.0;
        ps->transferMode.postColorMatrixBias.a  = 0.0;

        /* Initialize pixel maps with default sizes and values.
        */
        for (m = GL_PIXEL_MAP_I_TO_I; m <= GL_PIXEL_MAP_A_TO_A; m++) {
            __glInitDefaultPixelMap(gc, m);
        }

        /* Setup to use the correct read buffer */
        if (gc->modes.doubleBufferMode) {
            ps->readBuffer = GL_BACK;
        }
        else {
            ps->readBuffer = GL_FRONT;
        }
        ps->readBufferReturn = ps->readBuffer;
    }
#endif
}

GLboolean __glCheckUnpackArgs(__GLcontext *gc, GLenum format, GLenum type)
{
    switch (format)
    {
    case GL_RGBA:
    case GL_ABGR_EXT:
        switch (type)
        {
        case GL_BYTE:
        case GL_UNSIGNED_BYTE:
        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
        case GL_HALF_FLOAT:
        case GL_FLOAT:
            break;
        default:
            __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
        }
        break;

    case GL_RGB:
        switch (type)
        {
        case GL_BYTE:
        case GL_UNSIGNED_BYTE:
        case GL_UNSIGNED_SHORT_5_6_5:
        case GL_UNSIGNED_INT_10F_11F_11F_REV:
        case GL_UNSIGNED_INT_5_9_9_9_REV:
        case GL_HALF_FLOAT:
        case GL_FLOAT:
            break;
        default:
            __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
        }
        break;

    case GL_RG:
    case GL_RED:
        switch (type)
        {
        case GL_BYTE:
        case GL_UNSIGNED_BYTE:
        case GL_HALF_FLOAT:
        case GL_FLOAT:
            break;
        default:
            __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
        }
        break;

    case GL_RGBA_INTEGER:
        switch (type)
        {
        case GL_BYTE:
        case GL_UNSIGNED_BYTE:
        case GL_SHORT:
        case GL_UNSIGNED_SHORT:
        case GL_INT:
        case GL_UNSIGNED_INT:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            break;
        default:
            __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
        }
        break;

    case GL_RGB_INTEGER:
    case GL_RG_INTEGER:
    case GL_RED_INTEGER:
        switch (type)
        {
        case GL_BYTE:
        case GL_UNSIGNED_BYTE:
        case GL_SHORT:
        case GL_UNSIGNED_SHORT:
        case GL_INT:
        case GL_UNSIGNED_INT:
            break;
        default:
            __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
        }
        break;

    case GL_DEPTH_COMPONENT:
        switch (type)
        {
        case GL_UNSIGNED_SHORT:
        case GL_UNSIGNED_INT:
        case GL_FLOAT:
            break;
        default:
            __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
        }
        break;

    case GL_DEPTH_STENCIL:
        switch (type)
        {
        case GL_UNSIGNED_INT_24_8:
        case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
            break;
        default:
            __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
        }
        break;

    case GL_ALPHA:
    case GL_LUMINANCE:
    case GL_LUMINANCE_ALPHA:
        switch (type)
        {
        case GL_UNSIGNED_BYTE:
            break;
        default:
            __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
        }
        break;

    default:
        __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
    }

    return GL_TRUE;
}

GLvoid GL_APIENTRY __glim_DrawBuffers(__GLcontext *gc, GLsizei n, const GLenum *bufs)
{
    GLsizei i;
    GLboolean changed = GL_FALSE;
    GLenum *pDrawBuffers = gcvNULL;
#ifdef OPENGL40
    GLuint AttachTime[8] = {0};
    GLsizei times;
#endif

    __GL_HEADER();

    if (DRAW_FRAMEBUFFER_BINDING_NAME)
    {
        __GLframebufferObject *drawFBO = gc->frameBuffer.drawFramebufObj;

        if ((gcvNULL == bufs) && !(gc->constants.majorVersion == 3 && gc->constants.minorVersion >= 1))
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }

        if (n > (GLsizei)gc->constants.shaderCaps.maxDrawBuffers || n < 0)
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }

        if (gc->imports.conformGLSpec)
        {
            /* An INVALID_ENUM error is generated if any value in bufs is not one of the
            ** values in tables 15.3, BACK, or NONE
            */
            for (i = 0; i < n; ++i)
            {
                if ((bufs[i] >= GL_COLOR_ATTACHMENT0 + gc->constants.shaderCaps.maxDrawBuffers) ||
                    (bufs[i] >= GL_FRONT_LEFT && bufs[i] <= GL_BACK_RIGHT) ||
                    ((bufs[i] >= GL_AUX0) && ((GLint)bufs[i] < gc->modes.numAuxBuffers + GL_AUX0)))
                {
                    __GL_ERROR_EXIT(GL_INVALID_OPERATION);
                }
                else
                {
                    if (bufs[i] != GL_NONE && !((bufs[i] >= GL_COLOR_ATTACHMENT0) && (bufs[i] < GL_COLOR_ATTACHMENT0 + gc->constants.shaderCaps.maxDrawBuffers)))
                    {
                        __GL_ERROR_EXIT(GL_INVALID_ENUM);
                    }
                    else
                    {
                        if (bufs[i] != GL_NONE)
                        {
                            GLuint j = bufs[i] - GL_COLOR_ATTACHMENT0;
                            if (AttachTime[j])
                            {
                                __GL_ERROR_EXIT(GL_INVALID_OPERATION);
                            }
                            else
                            {
                                AttachTime[j]++;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            for (i = 0; i < n; ++i)
            {
                if (bufs[i] != GL_NONE && bufs[i] != GL_BACK &&
                    ((bufs[i] < GL_COLOR_ATTACHMENT0) || (bufs[i] > GL_COLOR_ATTACHMENT0 + gc->constants.shaderCaps.maxDrawBuffers - 1)))
                {
                    __GL_ERROR_EXIT(GL_INVALID_ENUM);
                }
                if ((bufs[i] != GL_NONE) && (bufs[i] != (GLenum)(GL_COLOR_ATTACHMENT0 + i)))
                {
                    __GL_ERROR_EXIT(GL_INVALID_OPERATION);
                }
            }
        }

        drawFBO->drawBufferCount = n;
        pDrawBuffers = drawFBO->drawBuffers;
    }
    else
    {
        if (gcvNULL == bufs)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }

        /* Error check */
        if (gc->imports.conformGLSpec)
        {
            if ((n <= 0) || (n > (GLsizei)gc->constants.maxDrawBuffers ))
            {
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }

            for (i = 0; i < n; i++)
            {
                if (bufs[i] != GL_NONE && !((bufs[i] >= GL_COLOR_ATTACHMENT0) && (bufs[i] < GL_COLOR_ATTACHMENT0 + gc->constants.shaderCaps.maxDrawBuffers)) &&
                    !(bufs[i] >= GL_FRONT_LEFT && bufs[i] <= GL_BACK_RIGHT) && !(bufs[i] >= GL_AUX0 && gc->modes.numAuxBuffers > (GLint)(bufs[i] - GL_AUX0)))
                {
                    __GL_ERROR_EXIT(GL_INVALID_ENUM);
                }
                else
                {
                    if ((bufs[i] >= GL_COLOR_ATTACHMENT0) && (bufs[i] < GL_COLOR_ATTACHMENT0 + gc->constants.shaderCaps.maxDrawBuffers))
                    {
                        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
                    }
                }
                if (!gc->modes.stereoMode && ((bufs[i] == GL_FRONT_RIGHT)||(bufs[i] == GL_BACK_RIGHT)))
                {
                    __GL_ERROR_EXIT(GL_INVALID_OPERATION);
                }
                if (!gc->modes.doubleBufferMode && ((bufs[i] == GL_BACK_LEFT) ||(bufs[i] == GL_BACK_RIGHT)))
                {
                    __GL_ERROR_EXIT(GL_INVALID_OPERATION);
                }
            }

            for (i = GL_FRONT_LEFT; i <= GL_AUX3; i++)
            {
                GLsizei j;
                times = 0;
                if ((i <= GL_BACK_RIGHT) || (i >= GL_AUX0))
                {
                    for (j = 0; j < n; j++)
                    {
                        if (bufs[j] == (GLenum)i)
                        {
                            times++;
                            if (times > 1)
                            {
                                __GL_ERROR_EXIT(GL_INVALID_OPERATION);
                            }
                        }
                    }
                }
            }
        }
        else
        {
            if (n != 1 || !(GL_NONE == *bufs || GL_BACK == *bufs))
            {
                __GL_ERROR_EXIT(GL_INVALID_OPERATION);
            }
        }

        pDrawBuffers = gc->state.raster.drawBuffers;
    }

    /* Redundancy check */
    for (i = 0; i < (GLsizei)gc->constants.shaderCaps.maxDrawBuffers; ++i)
    {
        GLenum buf = (i < n) ? bufs[i] : GL_NONE;

        if (pDrawBuffers[i] != buf)
        {
            pDrawBuffers[i] = buf;
            changed = GL_TRUE;
        }
    }

    if (changed)
    {
        gc->drawableDirtyMask |= __GL_BUFFER_DRAW_BIT;

        if (DRAW_FRAMEBUFFER_BINDING_NAME)
        {
            /* __GL_FRAMEBUFFER_DRAWBUFFER_DIRTY */
            __GL_FRAMEBUFFER_COMPLETE_DIRTY(gc->frameBuffer.drawFramebufObj);
        }
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_ReadBuffer(__GLcontext *gc, GLenum mode)
{
    __GL_HEADER();

    /* Fullfill ES Spec requirements */
    if (!gc->imports.conformGLSpec)
    {
        if ((mode != GL_NONE) && (mode != GL_BACK) &&
            (mode < GL_COLOR_ATTACHMENT0 || mode > __GL_COLOR_ATTACHMENT31))
        {
            __GL_ERROR_EXIT(GL_INVALID_ENUM);
        }
    }

    if (READ_FRAMEBUFFER_BINDING_NAME)
    {
        if ((gc->imports.conformGLSpec) &&
            ((mode == GL_BACK) || (mode == GL_FRONT) ||
             (mode == GL_LEFT) || (mode == GL_RIGHT) ||
             (mode == GL_FRONT_LEFT) || (mode == GL_FRONT_RIGHT) ||
             (mode == GL_BACK_LEFT)  || (mode == GL_BACK_RIGHT)  ||
             (mode > __GL_COLOR_ATTACHMENTn)))
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        if ((!gc->imports.conformGLSpec) &&
            ((mode == GL_BACK) || (mode > __GL_COLOR_ATTACHMENTn)))
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }

        /* If a FBO is bound to gc, set the states in FBO instead of gc. */
        if (mode != gc->frameBuffer.readFramebufObj->readBuffer)
        {
            gc->frameBuffer.readFramebufObj->readBuffer = mode;
            gc->drawableDirtyMask |= __GL_BUFFER_READ_BIT;

            __GL_FRAMEBUFFER_COMPLETE_DIRTY(gc->frameBuffer.readFramebufObj);
        }
    }
    else
    {
        if (gc->imports.conformGLSpec)
        {
            GLenum modeOffset = 0;
            GLenum originalReadBuffer;

            /* gc binding to the window buffer */
            __GL_VERTEX_BUFFER_FLUSH(gc);

            if ((mode & GL_FRONT_LEFT) && (mode >= GL_AUX0)) {
                modeOffset = mode - GL_AUX0;
                mode = GL_AUX0;
            }

            originalReadBuffer = gc->state.pixel.readBuffer;
            gc->state.pixel.readBuffer = mode;

            switch (mode) {
              case GL_NONE:
                  gc->state.pixel.readBuffer = GL_NONE;
                  break;

              case GL_FRONT_RIGHT:
                  if (!gc->modes.stereoMode) {
                      __glSetError(gc, GL_INVALID_OPERATION);
                      gc->state.pixel.readBuffer = originalReadBuffer;
                      __GL_EXIT();
                  }
                  break;

              case GL_FRONT_LEFT:
                  break;

              case GL_BACK_RIGHT:
                  if (!(gc->modes.stereoMode && gc->modes.doubleBufferMode)) {
                      __glSetError(gc, GL_INVALID_OPERATION);
                      gc->state.pixel.readBuffer = originalReadBuffer;
                      __GL_EXIT();
                  }
                  break;

              case GL_BACK_LEFT:
                  if (!gc->modes.doubleBufferMode) {
                      __glSetError(gc, GL_INVALID_OPERATION);
                      gc->state.pixel.readBuffer = originalReadBuffer;
                      __GL_EXIT();
                  }
                  break;

              case GL_FRONT:
                  gc->state.pixel.readBuffer = GL_FRONT_LEFT;
                  break;

              case GL_BACK:
                  if (!gc->modes.doubleBufferMode) {
                      __glSetError(gc, GL_INVALID_OPERATION);
                      gc->state.pixel.readBuffer = originalReadBuffer;
                      __GL_EXIT();
                  }
                  gc->state.pixel.readBuffer = GL_BACK_LEFT;
                  break;

              case GL_LEFT:
                      gc->state.pixel.readBuffer = GL_FRONT_LEFT;
                  break;

              case GL_RIGHT:
                  if (!gc->modes.stereoMode) {
                      __glSetError(gc, GL_INVALID_OPERATION);
                      gc->state.pixel.readBuffer = originalReadBuffer;
                      __GL_EXIT();
                  }
                  gc->state.pixel.readBuffer = GL_FRONT_RIGHT;
                  break;

              case GL_AUX0:
                  if (modeOffset >= (GLenum)gc->modes.numAuxBuffers) {
                      __glSetError(gc, GL_INVALID_OPERATION);
                      gc->state.pixel.readBuffer = originalReadBuffer;
                      __GL_EXIT();
                  }
                  mode = modeOffset + GL_AUX0;
                  gc->state.pixel.readBuffer = mode;
                  break;

              default:
                  __GL_ERROR_EXIT(GL_INVALID_ENUM);
            }

            if (mode != gc->state.pixel.readBuffer)
            {
                gc->drawableDirtyMask |= __GL_BUFFER_READ_BIT;
            }

            /* Update GL state */
            gc->state.pixel.readBufferReturn = mode;

            /* flip attribute dirty bit */
            __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_READBUFFER_BIT);
        }
        else
        {
            if (mode != GL_NONE && mode != GL_BACK)
            {
                __GL_ERROR_EXIT(GL_INVALID_OPERATION);
            }

            /* If gc binds to default FBO. */
            if (mode != gc->state.raster.readBuffer)
            {
                gc->state.raster.readBuffer = mode;
                gc->drawableDirtyMask |= __GL_BUFFER_READ_BIT;
            }
        }
    }
OnExit:
OnError:
    __GL_FOOTER();
}


GLvoid GL_APIENTRY __glim_PixelStorei(__GLcontext *gc, GLenum pname, GLint param)
{
    __GLclientPixelState *ps = &gc->clientState.pixel;

    __GL_HEADER();

    /* All pname require non-negative param */
    if (param < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    switch (pname)
    {
    case GL_PACK_ALIGNMENT:
        switch (param)
        {
        case 1: case 2: case 4: case 8:
            ps->packModes.alignment = param;
            break;
        default:
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        break;
    case GL_UNPACK_ALIGNMENT:
        switch (param)
        {
        case 1: case 2: case 4: case 8:
            ps->unpackModes.alignment = param;
            break;
        default:
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        break;
    case GL_PACK_ROW_LENGTH:
        ps->packModes.lineLength = param;
        break;
    case GL_PACK_SKIP_ROWS:
        ps->packModes.skipLines = param;
        break;
    case GL_PACK_SKIP_PIXELS:
        ps->packModes.skipPixels = param;
        break;
    case GL_UNPACK_ROW_LENGTH:
        ps->unpackModes.lineLength = param;
        break;
    case GL_UNPACK_SKIP_ROWS:
        ps->unpackModes.skipLines = param;
        break;
    case GL_UNPACK_SKIP_PIXELS:
        ps->unpackModes.skipPixels = param;
        break;
    case GL_UNPACK_SKIP_IMAGES:
        ps->unpackModes.skipImages = param;
        break;
    case GL_UNPACK_IMAGE_HEIGHT:
        ps->unpackModes.imageHeight = param;
        break;
#ifdef OPENGL40
    case GL_PACK_LSB_FIRST:
        ps->packModes.lsbFirst = param;
        break;
    case GL_PACK_SWAP_BYTES:
        ps->packModes.swapEndian = param;
        break;
    case GL_UNPACK_LSB_FIRST:
        ps->unpackModes.lsbFirst = param;
        break;
    case GL_UNPACK_SWAP_BYTES:
        ps->unpackModes.swapEndian = param;
        break;
    case GL_PACK_IMAGE_HEIGHT:
        ps->packModes.imageHeight = param;
        break;
    case GL_PACK_SKIP_IMAGES:
        ps->packModes.skipImages = param;
        break;
#endif
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

OnError:
    __GL_FOOTER();
}

GLboolean __glCheckReadPixelArgs(__GLcontext *gc, GLsizei width, GLsizei height,
                                 GLenum format, GLenum type)
{
    __GLformatInfo * formatInfo = gcvNULL;
    __GLframebufferObject *readFBO = gc->frameBuffer.readFramebufObj;

    if (!gc->dp.isFramebufferComplete(gc, readFBO))
    {
        __GL_ERROR_RET_VAL(GL_INVALID_FRAMEBUFFER_OPERATION, GL_FALSE);
    }

    /*OPENGLES only read color buffer*/
    if (!((format == GL_DEPTH_COMPONENT) || (format == GL_STENCIL_INDEX) || (format == GL_DEPTH_STENCIL)))
    {
        /* Check if framebuffer is complete */
        if (READ_FRAMEBUFFER_BINDING_NAME == 0)
        {
            if (gc->state.raster.readBuffer == GL_NONE)
            {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }

            formatInfo = gc->drawablePrivate->rtFormatInfo;
        }
        else
        {
            __GLfboAttachPoint *attachPoint;
            GLint attachIndex;

            if (readFBO->readBuffer == GL_NONE)
            {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }

            attachIndex = __glMapAttachmentToIndex(readFBO->readBuffer);
            if (-1 == attachIndex)
            {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }
            attachPoint = &readFBO->attachPoint[attachIndex];

            if (0 == attachPoint->objName)
            {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }

            /* Disable multi-samples check for tex RT to support EXT_multisampled_render_to_texture */
            if (readFBO->fbSamples > 0 && !(attachPoint->isExtMode))
            {
                 __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }

            formatInfo = __glGetFramebufferFormatInfo(gc, readFBO, readFBO->readBuffer);
        }

        if (formatInfo == gcvNULL)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
        }
    }

    if ((width < 0) || (height < 0))
    {
        __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
    }

#ifdef OPENGL40
    if (gc->imports.conformGLSpec)
    {
        switch (format)
        {
          case GL_STENCIL_INDEX:
              if (READ_FRAMEBUFFER_BINDING_NAME ?
                 !gc->frameBuffer.readFramebufObj->attachPoint[__GL_STENCIL_ATTACHMENT_POINT_INDEX].objName : !gc->modes.haveStencilBuffer) {
                  __glSetError(gc, GL_INVALID_OPERATION);
                  return GL_FALSE;
              }
              break;
          case GL_COLOR_INDEX:
              //        if (gc->modes.rgbMode) {
              /* Can't convert RGB to color index */
              __glSetError(gc, GL_INVALID_OPERATION);
              return GL_FALSE;
              //        }
              //        break;
          case GL_DEPTH_COMPONENT:
              if (READ_FRAMEBUFFER_BINDING_NAME ?
                 !gc->frameBuffer.readFramebufObj->attachPoint[__GL_DEPTH_ATTACHMENT_POINT_INDEX].objName : !gc->modes.haveDepthBuffer) {
                  __glSetError(gc, GL_INVALID_OPERATION);
                  return GL_FALSE;
              }
              break;
          case GL_DEPTH_STENCIL_EXT:
              if (READ_FRAMEBUFFER_BINDING_NAME ?
                (!gc->frameBuffer.readFramebufObj->attachPoint[__GL_DEPTH_ATTACHMENT_POINT_INDEX].objName ||
                !gc->frameBuffer.readFramebufObj->attachPoint[__GL_STENCIL_ATTACHMENT_POINT_INDEX].objName) :
                (!gc->modes.haveDepthBuffer || !gc->modes.haveStencilBuffer)) {
                  __glSetError(gc, GL_INVALID_OPERATION);
                  return GL_FALSE;
              }
              if (!((type == GL_UNSIGNED_INT_24_8) || (type == GL_FLOAT_32_UNSIGNED_INT_24_8_REV)))
              {
                  __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
              }
              break;
          case GL_RED:
          case GL_GREEN:
          case GL_BLUE:
          case GL_ALPHA:
          case GL_RGB:
          case GL_RGBA:
          case GL_ABGR_EXT:
          case GL_BGR:
          case GL_BGRA:
          case GL_LUMINANCE:
          case GL_LUMINANCE_ALPHA:
              break;

          case GL_RED_INTEGER_EXT:
          case GL_GREEN_INTEGER_EXT:
          case GL_BLUE_INTEGER_EXT:
          case GL_ALPHA_INTEGER_EXT:
          case GL_RGB_INTEGER_EXT:
          case GL_RGBA_INTEGER_EXT:
          case GL_BGR_INTEGER_EXT:
          case GL_BGRA_INTEGER_EXT:
          case GL_LUMINANCE_INTEGER_EXT:
          case GL_LUMINANCE_ALPHA_INTEGER_EXT:
              if (!__glExtension[__GL_EXTID_EXT_texture_integer].bEnabled || (type == GL_FLOAT ))
              {
                 __glSetError(gc, GL_INVALID_ENUM);
                 return GL_FALSE;
              }

              if ((!gc->frameBuffer.readFramebufObj->fbIntMask)||(READ_FRAMEBUFFER_BINDING_NAME == 0))
              {
                 __glSetError(gc, GL_INVALID_OPERATION);
                 return GL_FALSE;
              }
              break;
        case GL_RG_INTEGER:
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            break;
        case GL_RG:
            if (((type == GL_UNSIGNED_INT_24_8) || (type == GL_FLOAT_32_UNSIGNED_INT_24_8_REV)))
            {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }
            break;
        default:
              __glSetError(gc, GL_INVALID_ENUM);
              return GL_FALSE;
        }

        switch (type)
        {
          case GL_BITMAP:
              if (format != GL_STENCIL_INDEX && format != GL_COLOR_INDEX) {
                  __glSetError(gc, GL_INVALID_OPERATION);
                  return GL_FALSE;
              }
              break;
          case GL_BYTE:
          case GL_UNSIGNED_BYTE:
          case GL_SHORT:
          case GL_UNSIGNED_SHORT:
          case GL_INT:
          case GL_UNSIGNED_INT:
          case GL_FLOAT:
              break;
          case GL_UNSIGNED_INT_24_8_EXT:
          case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
              /*currently only for GL_DEPTH_STENCIL_EXT , to do more if needed*/
              if ( format != GL_DEPTH_STENCIL_EXT)
              {
                      __glSetError(gc, GL_INVALID_OPERATION);
                      return GL_FALSE;
              }
              break;
          case GL_UNSIGNED_BYTE_3_3_2:
          case GL_UNSIGNED_SHORT_5_6_5:
          case GL_UNSIGNED_BYTE_2_3_3_REV:
          case GL_UNSIGNED_SHORT_5_6_5_REV:
              switch (format) {
                  case GL_RGB:
                      break;
                  default:
                      __glSetError(gc, GL_INVALID_OPERATION);
                      return GL_FALSE;
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
              switch (format) {
                  case GL_RGBA:
                  case GL_ABGR_EXT:
                  case GL_BGRA:
                      break;
                  default:
                      __glSetError(gc, GL_INVALID_OPERATION);
                      return GL_FALSE;
              }
              break;
        case GL_UNSIGNED_INT_5_9_9_9_REV_EXT:
            if (!__glExtension[__GL_EXTID_EXT_texture_shared_exponent].bEnabled)
            {
                __glSetError(gc, GL_INVALID_ENUM);
                return GL_FALSE;
            }

            if (format != GL_RGB)
            {
                __glSetError(gc, GL_INVALID_ENUM);
                return GL_FALSE;
            }
            break;
        case GL_HALF_FLOAT_ARB:
            if (!__glExtension[__GL_EXTID_ARB_half_float_pixel].bEnabled)
            {
                __glSetError(gc, GL_INVALID_ENUM);
                return GL_FALSE;
            }
            break;

        case GL_UNSIGNED_INT_10F_11F_11F_REV_EXT:
            if (!__glExtension[__GL_EXTID_EXT_packed_float].bEnabled)
            {
                __glSetError(gc, GL_INVALID_ENUM);
                return GL_FALSE;
            }

            if (format != GL_RGB)
            {
                __glSetError(gc, GL_INVALID_ENUM);
                return GL_FALSE;
            }
            break;

        default:
              __glSetError(gc, GL_INVALID_ENUM);
              return GL_FALSE;
        }
    }
    else  /* Running OES api */
#endif
    {
        /* Implementation-chosen format information */
        if ((formatInfo->dataType == type) && (formatInfo->dataFormat == format))
        {
            return GL_TRUE;
        }

        switch (formatInfo->category)
        {
        case GL_SIGNED_NORMALIZED:
        case GL_UNSIGNED_NORMALIZED:
            if (GL_RGBA != format && GL_BGRA_EXT != format)
            {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }
            if (__GL_FMT_RGB10_A2 == formatInfo->drvFormat)
            {
                if (GL_UNSIGNED_BYTE != type && GL_UNSIGNED_INT_2_10_10_10_REV != type)
                {
                    __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
                }
            }
            else if (GL_BGRA_EXT == format)
            {
                if (GL_UNSIGNED_BYTE != type &&
                    GL_UNSIGNED_SHORT_4_4_4_4_REV_EXT != type &&
                    GL_UNSIGNED_SHORT_1_5_5_5_REV_EXT != type)
                {
                    __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
                }
            }
            else
            {
                if (GL_UNSIGNED_BYTE != type)
                {
                    __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
                }
            }
            break;

        case GL_FLOAT:
            if (GL_RGBA != format || GL_FLOAT != type)
            {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }
            break;

        case GL_UNSIGNED_INT:
            if (GL_RGBA_INTEGER != format || GL_UNSIGNED_INT != type)
            {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }
            break;

        case GL_INT:
            if (GL_RGBA_INTEGER != format || GL_INT != type)
            {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }
            break;

        default:
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }
    }

    return GL_TRUE;
}

__GL_INLINE GLboolean __glReadPixelsBegin(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height,
                                          GLenum format, GLenum type, GLvoid* pixels)
{
    if (width == 0 || height == 0)
    {
        return GL_FALSE;
    }

    if (gc->flags & __GL_CONTEXT_SKIP_DRAW_INVALID_RENDERBUFFER)
    {
        return GL_FALSE;
    }
    else
    {
        return (*gc->dp.readPixelsBegin)(gc);
    }

}

__GL_INLINE GLvoid __glReadPixelsValidateState(__GLcontext *gc)
{
    (*gc->dp.readPixelsValidateState)(gc);
}

__GL_INLINE GLvoid __glReadPixelsEnd(__GLcontext *gc)
{
    GLboolean retVal;

    retVal = (*gc->dp.readPixelsEnd)(gc);

    if (!retVal)
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }
}


GLvoid GL_APIENTRY __glim_ReadPixels(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height,
                                     GLenum format, GLenum type, GLvoid* pixels)
{
    GLboolean retVal;
    __GLbufferObject *packBufObj = gc->bufferObject.generalBindingPoint[__GL_PIXEL_PACK_BUFFER_INDEX].boundBufObj;
    __GLpixelTransferInfo transferInfo;
    __GLformatInfo *formatInfo;

    __GL_HEADER();

    memset(&transferInfo, 0 ,sizeof(__GLpixelTransferInfo));

    if (!__glCheckReadPixelArgs(gc, width, height, format, type))
    {
        __GL_EXIT();
    }

    /* Check if framebuffer is complete */
    if (READ_FRAMEBUFFER_BINDING_NAME == 0)
    {
        formatInfo = gc->drawablePrivate->rtFormatInfo;
    }
    else
    {
        __GLframebufferObject *readFBO = gc->frameBuffer.readFramebufObj;
        formatInfo = __glGetFramebufferFormatInfo(gc, readFBO, readFBO->readBuffer);
    }

    if (formatInfo != gcvNULL)
    {
        __glGenericPixelTransfer(gc, width, height, 1, formatInfo, format, &type, pixels, &transferInfo, __GL_ReadPixelsPre);
    }
    else
    {
        transferInfo.srcImage = pixels;
        transferInfo.applyPixelTransfer = GL_FALSE;
    }

    /* The image is from pack buffer object? */
    if (packBufObj)
    {
        if (!__glCheckPBO(gc, &gc->clientState.pixel.packModes, packBufObj, width, height, 0, format, type, transferInfo.srcImage))
        {
            __GL_EXIT();
        }
    }

    __glEvaluateDrawableChange(gc, __GL_BUFFER_READ_BIT);

    if (GL_TRUE == __glReadPixelsBegin(gc, x, y, width, height, format, type, (GLvoid*)transferInfo.srcImage))
    {
        __glReadPixelsValidateState(gc);

        retVal = (*gc->dp.readPixels)(gc, x, y, width, height, format, type, (GLubyte*)transferInfo.srcImage);

        __glReadPixelsEnd(gc);

        if (!retVal)
        {
            __GL_ERROR((*gc->dp.getError)(gc));
        }
    }

    if ((GL_TRUE == transferInfo.applyPixelTransfer) && (formatInfo != gcvNULL))
    {
        __glGenericPixelTransfer(gc, width, height, 1, formatInfo, format, &type, pixels, &transferInfo, __GL_ReadPixels);
    }

OnExit:
    if ((GL_TRUE == transferInfo.srcNeedFree) && (gcvNULL != transferInfo.srcImage)){
        (*gc->imports.free)(gc, (void*)transferInfo.srcImage);
        transferInfo.srcImage = gcvNULL;
    }
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_ReadnPixels(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height,
                                         GLenum format, GLenum type, GLsizei bufSize, GLvoid *data)
{
    GLint requiredsize;
    __GLclientPixelState *ps = &gc->clientState.pixel;
    GLuint lineLength = ps->packModes.lineLength ? ps->packModes.lineLength : (GLuint)width;
    GLuint imageHeight = ps->packModes.imageHeight ? ps->packModes.imageHeight : (GLuint)height;

    __GL_HEADER();

    requiredsize = lineLength * __glPixelSize(gc, format, type);

    requiredsize = gcmALIGN(requiredsize, ps->packModes.alignment);

    requiredsize *= imageHeight;

    if (requiredsize <= bufSize)
    {
        __glim_ReadPixels(gc, x, y, width, height, format, type, data);
    }
    else
    {
        __GL_ERROR(GL_INVALID_OPERATION);
    }

    __GL_FOOTER();
}

#ifdef OPENGL40
GLvoid APIENTRY __glim_DrawBuffer(__GLcontext *gc, GLenum mode)
{
    GLenum modeOffset = 0;
    GLenum originalDrawBuffer;
    GLuint i;
    GLboolean changed = GL_FALSE;
    GLenum *pDrawBuffers = gcvNULL;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    /*
    ** Gc is binding to a FBO, set the state to fbo not gc. */
    if (DRAW_FRAMEBUFFER_BINDING_NAME != 0){
        __GLframebufferObject *drawFBO = gc->frameBuffer.drawFramebufObj;

        if (mode != GL_NONE && mode != GL_BACK &&
            ((mode < GL_COLOR_ATTACHMENT0) || (mode > GL_COLOR_ATTACHMENT0 + gc->constants.shaderCaps.maxDrawBuffers - 1)))
        {
            __glSetError(gc, GL_INVALID_ENUM);
            return;
        }

        drawFBO->drawBufferCount = 1;
        pDrawBuffers = drawFBO->drawBuffers;

        if (pDrawBuffers[0] != mode)
        {
            pDrawBuffers[0] = mode;
            changed = GL_TRUE;
        }
    }
    else
    {
        __GL_VERTEX_BUFFER_FLUSH(gc);

        if ((mode & GL_FRONT_LEFT) && (mode >= GL_AUX0)) {
            modeOffset = mode - GL_AUX0;
            mode = GL_AUX0;
        }

        originalDrawBuffer = gc->state.raster.drawBuffers[0];
        gc->state.raster.drawBuffers[0] = mode;

        switch (mode) {
          case GL_NONE:
              break;

          case GL_FRONT_RIGHT:
              if (!gc->modes.stereoMode) {
                  __glSetError(gc, GL_INVALID_OPERATION);
                  gc->state.raster.drawBuffers[0] = originalDrawBuffer;
                  return;
              }
              break;

          case GL_FRONT_LEFT:
              break;

          case GL_BACK_RIGHT:
              if (!(gc->modes.stereoMode && gc->modes.doubleBufferMode)) {
                  __glSetError(gc, GL_INVALID_OPERATION);
                  gc->state.raster.drawBuffers[0] = originalDrawBuffer;
                  return;
              }
              break;

          case GL_BACK_LEFT:
              if (!gc->modes.doubleBufferMode) {
                  __glSetError(gc, GL_INVALID_OPERATION);
                  gc->state.raster.drawBuffers[0] = originalDrawBuffer;
                  return;
              }
              break;

          case GL_FRONT:
              break;

          case GL_BACK:
              if (!gc->modes.doubleBufferMode) {
                  __glSetError(gc, GL_INVALID_OPERATION);
                  gc->state.raster.drawBuffers[0] = originalDrawBuffer;
                  return;
              }
              break;

          case GL_LEFT:
              break;

          case GL_RIGHT:
              if (!gc->modes.stereoMode) {
                  __glSetError(gc, GL_INVALID_OPERATION);
                  gc->state.raster.drawBuffers[0] = originalDrawBuffer;
                  return;
              }
              break;

          case GL_FRONT_AND_BACK:
              break;

          case GL_AUX0:
              if (modeOffset >= (GLenum)gc->modes.numAuxBuffers) {
                  __glSetError(gc, GL_INVALID_OPERATION);
                  gc->state.raster.drawBuffers[0] = originalDrawBuffer;
                  return;
              }
              mode = modeOffset + GL_AUX0;
              gc->state.raster.drawBuffers[0] = mode;
              break;

          default:
              __glSetError(gc, GL_INVALID_ENUM);
              return;
        }

        for(i = 1; i < gc->constants.maxDrawBuffers; i++)
            gc->state.raster.drawBuffers[i] = GL_NONE;

        if (gc->state.raster.drawBuffers[0] != originalDrawBuffer)
        {
            changed = GL_TRUE;
        }
    }

    if (changed)
    {
        gc->drawableDirtyMask |= __GL_BUFFER_DRAW_BIT;

        if (DRAW_FRAMEBUFFER_BINDING_NAME)
        {
            /* __GL_FRAMEBUFFER_DRAWBUFFER_DIRTY */
            __GL_FRAMEBUFFER_COMPLETE_DIRTY(gc->frameBuffer.drawFramebufObj);
        }
    }

    /* flip attribute dirty bit */
    __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_DRAWBUFFER_BIT);
}

GLvoid APIENTRY __glim_Accum(__GLcontext *gc, GLenum op, GLfloat value)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    if (gc->conditionalRenderDiscard){
        return;
    }

    if (!gc->modes.haveAccumBuffer) {
        __glSetError(gc, GL_INVALID_OPERATION);
        return;
    }

    switch (op) {
      case GL_ACCUM:
      case GL_LOAD:
      case GL_RETURN:
      case GL_MULT:
      case GL_ADD:
          break;
      default:
          __glSetError(gc, GL_INVALID_ENUM);
          return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Get the latest drawable information */
//    LINUX_LOCK_FRAMEBUFFER(gc);

    /* Check all GL attributes for redundancy before notify device pipeline.
    ** Must be call here because __GL_CLEAR_ATTR_BITS affect dp.clear.
    */
//    __glEvaluateAttribDrawableChange(gc);
    __glEvaluateDrawableChange(gc, __GL_BUFFER_DRAW_READ_BITS);

    if (gc->renderMode == GL_RENDER) {
        (*gc->dp.accum)(gc, op, value);
    }

//    LINUX_UNLOCK_FRAMEBUFFER(gc);
}



/*
** Specify modes that control the storage format of pixel arrays.
*/
GLvoid APIENTRY __glim_PixelStoref(__GLcontext *gc, GLenum mode, GLfloat value)
{
    switch (mode) {
      case GL_PACK_ROW_LENGTH:
      case GL_PACK_SKIP_ROWS:
      case GL_PACK_SKIP_PIXELS:
      case GL_PACK_ALIGNMENT:
      case GL_UNPACK_ROW_LENGTH:
      case GL_UNPACK_SKIP_ROWS:
      case GL_UNPACK_SKIP_PIXELS:
      case GL_UNPACK_ALIGNMENT:
          /* Round */
          if (value < 0) {
              __glim_PixelStorei(gc, mode, (GLint) (value - (__GLfloat) 0.5));
          } else {
              __glim_PixelStorei(gc, mode, (GLint) (value + (__GLfloat) 0.5));
          }
          break;
      case GL_PACK_SWAP_BYTES:
      case GL_PACK_LSB_FIRST:
      case GL_UNPACK_SWAP_BYTES:
      case GL_UNPACK_LSB_FIRST:
          if (value == __glZero) {
              __glim_PixelStorei(gc, mode, GL_FALSE);
          } else {
              __glim_PixelStorei(gc, mode, GL_TRUE);
          }
      default:
          __glim_PixelStorei(gc, mode, (GLint) value);
          break;
    }
}

/*
** Specify modes that control the transfer of pixel arrays.
*/
GLvoid APIENTRY __glim_PixelTransferf(__GLcontext *gc, GLenum mode, GLfloat value)
{
    __GLpixelState *ps;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    ps = &gc->state.pixel;

    switch (mode) {
      case GL_RED_SCALE:
          ps->transferMode.r_scale = value;
          break;
      case GL_GREEN_SCALE:
          ps->transferMode.g_scale = value;
          break;
      case GL_BLUE_SCALE:
          ps->transferMode.b_scale = value;
          break;
      case GL_ALPHA_SCALE:
          ps->transferMode.a_scale = value;
          break;
      case GL_DEPTH_SCALE:
          ps->transferMode.d_scale = value;
          break;
      case GL_RED_BIAS:
          ps->transferMode.r_bias = value;
          break;
      case GL_GREEN_BIAS:
          ps->transferMode.g_bias = value;
          break;
      case GL_BLUE_BIAS:
          ps->transferMode.b_bias = value;
          break;
      case GL_ALPHA_BIAS:
          ps->transferMode.a_bias = value;
          break;
      case GL_DEPTH_BIAS:
          ps->transferMode.d_bias = value;
          break;
      case GL_INDEX_SHIFT:
          /* Round */
          if (value > 0) {
              ps->transferMode.indexShift = (GLint) (value + __glHalf);
          } else {
              ps->transferMode.indexShift = (GLint) (value - __glHalf);
          }
          break;
      case GL_INDEX_OFFSET:
          /* Round */
          if (value > 0) {
              ps->transferMode.indexOffset = (GLint) (value + __glHalf);
          } else {
              ps->transferMode.indexOffset = (GLint) (value - __glHalf);
          }
          break;
      case GL_MAP_COLOR:
          ps->transferMode.mapColor = (value != __glZero);
          break;
      case GL_MAP_STENCIL:
          ps->transferMode.mapStencil = (value != __glZero);
          break;
      default:
          __glSetError(gc, GL_INVALID_ENUM);
          return;
    }

    /*Immediate dp notification*/
//    (*gc->dp.pixelTransfer)(gc);

    /*Notify swp*/
    __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_PIXELTRANSFER_BIT);
}

GLvoid APIENTRY __glim_PixelTransferi( __GLcontext *gc, GLenum mode, GLint value)
{
    __glim_PixelTransferf(gc, mode, (GLfloat) value);
}

/************************************************************************/

/*
** Functions to specify mapping of pixel colors and stencil values.
*/
GLvoid APIENTRY __glim_PixelMapfv(__GLcontext *gc, GLenum map, GLint mapSize,
                                const GLfloat values[])
{
    __GLpixelState *ps;
    __GLpixelMapHead *pMap;
    GLint index = map - GL_PIXEL_MAP_I_TO_I;
    GLfloat value;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    ps = &gc->state.pixel;
    pMap = ps->pixelMap;

    switch (map) {
      case GL_PIXEL_MAP_I_TO_I:
      case GL_PIXEL_MAP_S_TO_S:
          if (mapSize <= 0 || (mapSize & (mapSize - 1))) {
              /*
              ** Maps indexed by color or stencil index must be sized
              ** to a power of two.
              */
              __glSetError(gc, GL_INVALID_VALUE);
              return;
          }
          if (pMap[index].base.mapI) {
              (*gc->imports.free)(gc, pMap[index].base.mapI);
              pMap[index].base.mapI = 0;
          }
          pMap[index].base.mapI = (GLint*)
              (*gc->imports.malloc)(gc, (size_t) (mapSize * sizeof(GLint)) );
          if (!pMap[index].base.mapI) {
              pMap[index].size = 0;
              return;
          }
          pMap[index].size = mapSize;
          while (--mapSize >= 0) {
              value = values[mapSize];
              if (value > 0) {            /* round! */
                  pMap[index].base.mapI[mapSize] =
                      (GLint)(value + __glHalf);
              } else {
                  pMap[index].base.mapI[mapSize] =
                      (GLint)(value - __glHalf);
              }
          }
          break;
      case GL_PIXEL_MAP_I_TO_R:
      case GL_PIXEL_MAP_I_TO_G:
      case GL_PIXEL_MAP_I_TO_B:
      case GL_PIXEL_MAP_I_TO_A:
          if (mapSize <= 0 || (mapSize & (mapSize - 1))) {
              /*
              ** Maps indexed by color or stencil index must be sized
              ** to a power of two.
              */
              __glSetError(gc, GL_INVALID_VALUE);
              return;
          }
      case GL_PIXEL_MAP_R_TO_R:
      case GL_PIXEL_MAP_G_TO_G:
      case GL_PIXEL_MAP_B_TO_B:
      case GL_PIXEL_MAP_A_TO_A:
          if (mapSize < 0) {
              /*
              ** Maps indexed by color component must not have negative size.
              */
              __glSetError(gc, GL_INVALID_VALUE);
              return;
          }
          if (pMap[index].base.mapF) {
              (*gc->imports.free)(gc, pMap[index].base.mapF);
              pMap[index].base.mapF = 0;
          }
          if (mapSize == 0) {
              __glInitDefaultPixelMap(gc, map);
          } else {
              pMap[index].base.mapF = (__GLfloat*)
                  (*gc->imports.malloc)(gc,(size_t) (mapSize * sizeof(__GLfloat)) );
              if (!pMap[index].base.mapF) {
                  pMap[index].size = 0;
                  return;
              }
              pMap[index].size = mapSize;
              while (--mapSize >= 0) {
                  value = values[mapSize];
                  if (value < __glZero) value = __glZero;
                  else if (value > __glOne) value = __glOne;
                  pMap[index].base.mapF[mapSize] = value;
              }
          }
          break;
      default:
          __glSetError(gc, GL_INVALID_ENUM);
          return;
    }

    /*Immediate dp notification*/
//    (*gc->dp.pixelMap)(gc, GL_FLOAT);

    /*Notify swp*/
    __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_PIXELTRANSFER_BIT);
}

GLvoid APIENTRY __glim_PixelMapuiv(__GLcontext *gc, GLenum map, GLint mapSize,
                                 const GLuint values[])
{
    __GLpixelState *ps;
    __GLpixelMapHead *pMap;
    GLint index = map - GL_PIXEL_MAP_I_TO_I;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    ps = &gc->state.pixel;
    pMap = ps->pixelMap;

    switch (map) {
      case GL_PIXEL_MAP_I_TO_I:
      case GL_PIXEL_MAP_S_TO_S:
          if (mapSize <= 0 || (mapSize & (mapSize - 1))) {
              /*
              ** Maps indexed by color or stencil index must be sized
              ** to a power of two.
              */
              __glSetError(gc, GL_INVALID_VALUE);
              return;
          }
          if (pMap[index].base.mapI) {
              (*gc->imports.free)(gc, pMap[index].base.mapI);
              pMap[index].base.mapI = 0;
          }
          pMap[index].base.mapI = (GLint*)
              (*gc->imports.malloc)(gc, (size_t) (mapSize * sizeof(GLint)) );
          if (!pMap[index].base.mapI) {
              pMap[index].size = 0;
              return;
          }
          pMap[index].size = mapSize;
          while (--mapSize >= 0) {
              pMap[index].base.mapI[mapSize] = values[mapSize];
          }
          break;
      case GL_PIXEL_MAP_I_TO_R:
      case GL_PIXEL_MAP_I_TO_G:
      case GL_PIXEL_MAP_I_TO_B:
      case GL_PIXEL_MAP_I_TO_A:
          if (mapSize <= 0 || (mapSize & (mapSize - 1))) {
              /*
              ** Maps indexed by color or stencil index must be sized
              ** to a power of two.
              */
              __glSetError(gc, GL_INVALID_VALUE);
              return;
          }
      case GL_PIXEL_MAP_R_TO_R:
      case GL_PIXEL_MAP_G_TO_G:
      case GL_PIXEL_MAP_B_TO_B:
      case GL_PIXEL_MAP_A_TO_A:
          if (mapSize < 0) {
              /*
              ** Maps indexed by color component must not have negative size.
              */
              __glSetError(gc, GL_INVALID_VALUE);
              return;
          }
          if (pMap[index].base.mapF) {
              (*gc->imports.free)(gc, pMap[index].base.mapF);
              pMap[index].base.mapF = 0;
          }
          if (mapSize == 0) {
              __glInitDefaultPixelMap(gc, map);
          } else {
              pMap[index].base.mapF = (__GLfloat*)
                  (*gc->imports.malloc)(gc, (size_t) (mapSize * sizeof(GLfloat)) );
              if (!pMap[index].base.mapF) {
                  pMap[index].size = 0;
                  return;
              }
              pMap[index].size = mapSize;
              while (--mapSize >= 0) {
                  pMap[index].base.mapF[mapSize] =
                      __GL_UI_TO_FLOAT(values[mapSize]);
              }
          }
          break;
      default:
          __glSetError(gc, GL_INVALID_ENUM);
          return;
    }

    /*Immediate dp notification*/
//    (*gc->dp.pixelMap)(gc, GL_UNSIGNED_INT);

    /*Notify swp*/
    __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_PIXELTRANSFER_BIT);
}

GLvoid APIENTRY __glim_PixelMapusv(__GLcontext *gc, GLenum map, GLint mapSize,
                                 const GLushort values[])
{
    __GLpixelState *ps;
    __GLpixelMapHead *pMap;
    GLint index = map - GL_PIXEL_MAP_I_TO_I;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    ps = &gc->state.pixel;
    pMap = ps->pixelMap;

    switch (map) {
      case GL_PIXEL_MAP_I_TO_I:
      case GL_PIXEL_MAP_S_TO_S:
          if (mapSize <= 0 || (mapSize & (mapSize - 1))) {
              /*
              ** Maps indexed by color or stencil index must be sized
              ** to a power of two.
              */
              __glSetError(gc, GL_INVALID_VALUE);
              return;
          }
          if (pMap[index].base.mapI) {
              (*gc->imports.free)(gc, pMap[index].base.mapI);
              pMap[index].base.mapI = 0;
          }
          pMap[index].base.mapI = (GLint*)
              (*gc->imports.malloc)(gc, (size_t) (mapSize * sizeof(GLint)) );
          if (!pMap[index].base.mapI) {
              pMap[index].size = 0;
              return;
          }
          pMap[index].size = mapSize;
          while (--mapSize >= 0) {
              pMap[index].base.mapI[mapSize] = values[mapSize];
          }
          break;
      case GL_PIXEL_MAP_I_TO_R:
      case GL_PIXEL_MAP_I_TO_G:
      case GL_PIXEL_MAP_I_TO_B:
      case GL_PIXEL_MAP_I_TO_A:
          if (mapSize <= 0 || (mapSize & (mapSize - 1))) {
              /*
              ** Maps indexed by color or stencil index must be sized
              ** to a power of two.
              */
              __glSetError(gc, GL_INVALID_VALUE);
              return;
          }
      case GL_PIXEL_MAP_R_TO_R:
      case GL_PIXEL_MAP_G_TO_G:
      case GL_PIXEL_MAP_B_TO_B:
      case GL_PIXEL_MAP_A_TO_A:
          if (mapSize < 0) {
              /*
              ** Maps indexed by color component must not have negative size.
              */
              __glSetError(gc, GL_INVALID_VALUE);
              return;
          }
          if (pMap[index].base.mapF) {
              (*gc->imports.free)(gc, pMap[index].base.mapF);
              pMap[index].base.mapF = 0;
          }
          if (mapSize == 0) {
              __glInitDefaultPixelMap(gc, map);
          } else {
              pMap[index].base.mapF = (__GLfloat*)
                  (*gc->imports.malloc)(gc, (size_t) (mapSize * sizeof(GLfloat)) );
              if (!pMap[index].base.mapF) {
                  pMap[index].size = 0;
                  return;
              }
              pMap[index].size = mapSize;
              while (--mapSize >= 0) {
                  pMap[index].base.mapF[mapSize] =
                      __GL_US_TO_FLOAT(values[mapSize]);
              }
          }
          break;
      default:
          __glSetError(gc, GL_INVALID_ENUM);
          return;
    }

    /*Immediate dp notification*/
//    (*gc->dp.pixelMap)(gc, GL_UNSIGNED_SHORT);

    /*Notify swp*/
    __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_PIXELTRANSFER_BIT);
}

/*
** Specify zoom factor that affects drawing and copying of pixel arrays
*/
GLvoid APIENTRY __glim_PixelZoom(__GLcontext *gc, GLfloat xfactor, GLfloat yfactor)
{
    __GLpixelState *ps;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    ps = &gc->state.pixel;
    ps->transferMode.zoomX = xfactor;
    ps->transferMode.zoomY = yfactor;

    /*Immediate dp notification*/
//    (*gc->dp.pixelZoom)(gc);

    /*Notify swp*/
    __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_PIXELZOOM_BIT);
}


GLboolean __glCheckDrawPixelArgs(__GLcontext *gc, GLsizei width, GLsizei height,
                                 GLenum format, GLenum type)
{
    if ((width < 0) || (height < 0))
    {
        __glSetError(gc, GL_INVALID_VALUE);
        return GL_FALSE;
    }

    switch (format)
    {
    case GL_STENCIL_INDEX:
        if (!gc->modes.haveStencilBuffer)
        {
            __glSetError(gc, GL_INVALID_OPERATION);
            return GL_FALSE;
        }
    case GL_DEPTH_COMPONENT:
        if ( !gc->modes.haveDepthBuffer )
        {
            __glSetError(gc, GL_INVALID_OPERATION);
            return GL_FALSE;
        }
        break;
    }

    return __glCheckUnpackArgs(gc, format,type);
}

GLvoid APIENTRY __glim_DrawPixels(__GLcontext *gc, GLsizei width, GLsizei height, GLenum format,
                                  GLenum type, const GLvoid *pixels)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    if (gc->conditionalRenderDiscard){
        return;
    }

    if (!__glCheckDrawPixelArgs(gc, width, height, format, type)) {
        return;
    }

    if (!gc->state.rasterPos.validRasterPos) {
        return;
    }

    if (gc->drawablePrivate->width * gc->drawablePrivate->height == 0 )
    {
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    if ( (format == GL_DEPTH_COMPONENT &&  !gc->state.depth.writeEnable) ||
         (format == GL_STENCIL_INDEX && !gc->state.stencil.StencilArbFront.writeMask) ||
         (format == GL_DEPTH_STENCIL_EXT && !gc->state.depth.writeEnable && !gc->state.stencil.StencilArbFront.writeMask))
    {
        return;
    }

    /* Get the latest drawable information */
//    LINUX_LOCK_FRAMEBUFFER(gc);
    __glEvaluateDrawableChange(gc, __GL_BUFFER_DRAW_READ_BITS);

    if ((gc->flags & __GL_DISCARD_FOLLOWING_DRAWS_FRAMEBUFFER_NOT_COMPLETE) == 0)
    {
        (*gc->dp.rasterBegin)(gc, __GL_RASTERFUNC_DRAWPIX,format, width, height);
        (*gc->dp.drawPixels)(gc, width, height, format, type, (GLubyte *)pixels);
        (*gc->dp.rasterEnd)(gc, __GL_RASTERFUNC_DRAWPIX);
    }
 //   LINUX_UNLOCK_FRAMEBUFFER(gc);
}

GLvoid APIENTRY __glim_CopyPixels(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height,
                                GLenum type)
{
    GLenum format;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    if (gc->conditionalRenderDiscard){
        return;
    }

    if (READ_FRAMEBUFFER_BINDING_NAME != 0)
    {
        if (!gc->dp.isFramebufferComplete(gc, gc->frameBuffer.readFramebufObj))
        {
            __glSetError(gc, GL_INVALID_FRAMEBUFFER_OPERATION_EXT);
            return;
        }
        else
        {
            if (gc->frameBuffer.readFramebufObj->fbSamples > 0)
            {
                __glSetError(gc, GL_INVALID_FRAMEBUFFER_OPERATION_EXT);
                return;
            }

            /* Disable it temporarily
            if (gc->frameBuffer.readFramebufObj->fbInteger)
            {
                GL_ASSERT(0);
                return;
            }
            */
        }

    }

    if ((width < 0) || (height < 0)) {
        __glSetError(gc, GL_INVALID_VALUE);
        return;
    }

    switch (type) {
      case GL_STENCIL:
          if (!gc->modes.haveStencilBuffer) {
              __glSetError(gc, GL_INVALID_OPERATION);
              return;
          }
          format = GL_STENCIL_INDEX;
          break;
      case GL_COLOR:
          format = gc->modes.rgbMode ? GL_RGBA : GL_COLOR_INDEX;
          break;
      case GL_DEPTH:
          if (!gc->modes.haveDepthBuffer) {
              __glSetError(gc, GL_INVALID_OPERATION);
              return;
          }
          format = GL_DEPTH_COMPONENT;
          break;
      default:
          __glSetError(gc, GL_INVALID_ENUM);
          return;
    }

    if (!gc->state.rasterPos.validRasterPos) {
        return;
    }

    if (gc->drawablePrivate->width * gc->drawablePrivate->height == 0 ||
        gc->readablePrivate->width * gc->readablePrivate->height == 0)
    {
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    if ((format == GL_DEPTH_COMPONENT &&  !gc->state.depth.writeEnable) ||
        (format == GL_STENCIL_INDEX && !gc->state.stencil.StencilArbFront.writeMask) ||
        (format == GL_DEPTH_STENCIL_EXT && !gc->state.depth.writeEnable && !gc->state.stencil.StencilArbFront.writeMask))
    {
        return;
    }

    /* Get the latest drawable information */
//    LINUX_LOCK_FRAMEBUFFER(gc);
    __glEvaluateDrawableChange(gc, __GL_BUFFER_DRAW_READ_BITS);

    if ((gc->flags & __GL_DISCARD_FOLLOWING_DRAWS_FRAMEBUFFER_NOT_COMPLETE) == 0)
    {
        (*gc->dp.rasterBegin)(gc, __GL_RASTERFUNC_COPYPIX, format, width, height);
        (*gc->dp.copyPixels)(gc, x, y, width, height, format);
        (*gc->dp.rasterEnd)(gc, __GL_RASTERFUNC_COPYPIX);
    }
//    LINUX_UNLOCK_FRAMEBUFFER(gc);
}


GLvoid APIENTRY __glim_Bitmap(__GLcontext *gc, GLsizei w, GLsizei h, GLfloat xOrig, GLfloat yOrig,
                            GLfloat xMove, GLfloat yMove, const GLubyte *bitmap)
{
    GLuint unpackBuffer;
    __GLbufferObject *bufObj = NULL;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    if (gc->conditionalRenderDiscard){
        return;
    }

    if ((w < 0) || (h < 0)) {
        __glSetError(gc, GL_INVALID_VALUE);
        return;
    }

    if (!gc->state.rasterPos.validRasterPos) {
        return;
    }

    if (!gc->drawablePrivate->width || !gc->drawablePrivate->height)
    {
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Get the latest drawable information */
//    LINUX_LOCK_FRAMEBUFFER(gc);

    __glEvaluateDrawableChange(gc, __GL_BUFFER_DRAW_READ_BITS);

    if ((gc->flags & __GL_DISCARD_FOLLOWING_DRAWS_FRAMEBUFFER_NOT_COMPLETE) == 0)
    {
        (*gc->dp.rasterBegin)(gc, __GL_RASTERFUNC_BITMAP,GL_RGBA, 0, 0);

        /* The image is from unpack buffer object? */
        unpackBuffer = gc->bufferObject.generalBindingPoint[__GL_PIXEL_UNPACK_BUFFER_INDEX].boundBufName;

        /* If the source is from a buffer object and HW can't support this kind of blt, patch the address using system cache in buffer object. */
        if (unpackBuffer > 0)
        {
            GL_ASSERT(unpackBuffer > 0);

            bufObj = (__GLbufferObject *)__glGetObject(gc, gc->bufferObject.shared, unpackBuffer);
            if (bufObj == NULL)
            {
                GL_ASSERT(0);
//                LINUX_UNLOCK_FRAMEBUFFER(gc);
                return;
            }
        }

        (*gc->dp.bitmaps)(gc, w, h, xOrig, yOrig, xMove, yMove, bitmap, bufObj);
        (*gc->dp.rasterEnd)(gc, __GL_RASTERFUNC_BITMAP);

        /* Advance current raster position. */
        {
            GLboolean yInvert=GL_FALSE;
//            yInvert = (DRAW_FRAMEBUFFER_BINDING_NAME == 0)?gc->drawablePrivate->yInverted: GL_FALSE;
            gc->state.rasterPos.rPos.winPos.f.x += xMove;
            gc->state.rasterPos.rPos.winPos.f.y += (yInvert? -1 : 1 ) * yMove;
        }
    }
//    LINUX_UNLOCK_FRAMEBUFFER(gc);
}
#endif
