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
#include "gc_gl_buffers.h"
#include "api/gc_gl_api_inline.c"
#include "gc_gl_debug.h"
#include "gc_gl_names_inline.c"
#include "dri/viv_lock.h"


GLvoid __glInitDrawPixelsInfo(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                                   GLint width, GLint height, GLenum format,
                                   GLenum type, const GLvoid *pixels)
{
}

GLvoid __glInitReadPixelsInfo(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                                   GLint x, GLint y, GLint width, GLint height,
                                   GLenum format, GLenum type, const GLvoid *pixels)
{
}

GLvoid __glGenericPickReadPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, GLuint readSrcType)
{
}

GLvoid __glGenericPickDrawPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, GLuint drawDstType)
{
}

GLvoid __glFreeDefaultPixelMap(__GLcontext *gc, GLenum map)
{
    __GLpixelState *ps = &gc->state.pixel;
    __GLpixelMapHead *pMap = ps->pixelMap;
    GLint index = map - GL_PIXEL_MAP_I_TO_I;

    switch (map) {
      case GL_PIXEL_MAP_I_TO_I:
      case GL_PIXEL_MAP_S_TO_S:
          /*
          ** Allocate single-entry map for index type.
          */
          if(pMap[index].base.mapI)
              (*gc->imports.free)(gc, pMap[index].base.mapI);
          break;

      case GL_PIXEL_MAP_I_TO_R: case GL_PIXEL_MAP_I_TO_G:
      case GL_PIXEL_MAP_I_TO_B: case GL_PIXEL_MAP_I_TO_A:
      case GL_PIXEL_MAP_R_TO_R: case GL_PIXEL_MAP_G_TO_G:
      case GL_PIXEL_MAP_B_TO_B: case GL_PIXEL_MAP_A_TO_A:
          /*
          ** Allocate single-entry map for component type.
          */
          if (pMap[index].base.mapF)
              (*gc->imports.free)(gc, pMap[index].base.mapF);
          break;
    }
}

GLvoid __glInitDefaultPixelMap(__GLcontext *gc, GLenum map)
{
    __GLpixelState *ps = &gc->state.pixel;
    __GLpixelMapHead *pMap = ps->pixelMap;
    GLint index = map - GL_PIXEL_MAP_I_TO_I;

    switch (map) {
      case GL_PIXEL_MAP_I_TO_I:
      case GL_PIXEL_MAP_S_TO_S:
          /*
          ** Allocate single-entry map for index type.
          */
          if (!(pMap[index].base.mapI = (GLint*)
              (*gc->imports.malloc)(gc, sizeof(GLint) )) ) {
                  return;
          } else {
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
          if (!(pMap[index].base.mapF = (__GLfloat*)
              (*gc->imports.malloc)(gc, sizeof(__GLfloat) ))) {
                  return;
          } else {
              pMap[index].base.mapF[0] = __glZero;
              pMap[index].size = 1;
          }
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }
}

/*
** Specify modes that control the storage format of pixel arrays.
*/
GLvoid APIENTRY __glim_PixelStoref(GLenum mode, GLfloat value)
{

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_PixelStoref", DT_GLenum, mode, DT_GLfloat, value, DT_GLnull);
#endif

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
              __glim_PixelStorei(mode, (GLint) (value - (__GLfloat) 0.5));
          } else {
              __glim_PixelStorei(mode, (GLint) (value + (__GLfloat) 0.5));
          }
          break;
      case GL_PACK_SWAP_BYTES:
      case GL_PACK_LSB_FIRST:
      case GL_UNPACK_SWAP_BYTES:
      case GL_UNPACK_LSB_FIRST:
          if (value == __glZero) {
              __glim_PixelStorei(mode, GL_FALSE);
          } else {
              __glim_PixelStorei(mode, GL_TRUE);
          }
      default:
          __glim_PixelStorei(mode, (GLint) value);
          break;
    }
}

GLvoid APIENTRY __glim_PixelStorei(GLenum mode, GLint value)
{
    __GLclientPixelState *ps;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_PixelStorei", DT_GLenum, mode, DT_GLint, value, DT_GLnull);
#endif

    __GL_VERTEX_BUFFER_FLUSH(gc);

    ps = &gc->clientState.pixel;

    switch (mode) {
      case GL_PACK_ROW_LENGTH:
          if (value < 0) {
              __glSetError(GL_INVALID_VALUE);
              return;
          }
          ps->packModes.lineLength = value;
          break;
      case GL_PACK_SKIP_ROWS:
          if (value < 0) {
              __glSetError(GL_INVALID_VALUE);
              return;
          }
          ps->packModes.skipLines = value;
          break;
      case GL_PACK_SKIP_PIXELS:
          if (value < 0) {
              __glSetError(GL_INVALID_VALUE);
              return;
          }
          ps->packModes.skipPixels = value;
          break;
      case GL_PACK_ALIGNMENT:
          switch (value) {
              case 1: case 2: case 4: case 8:
                  ps->packModes.alignment = value;
                  break;
              default:
                  __glSetError(GL_INVALID_VALUE);
                  return;
          }
          break;
      case GL_PACK_SWAP_BYTES:
          ps->packModes.swapEndian = (value != 0);
          break;
      case GL_PACK_LSB_FIRST:
          ps->packModes.lsbFirst = (value != 0);
          break;
      case GL_PACK_SKIP_IMAGES:
          ps->packModes.skipImages = value;
          break;
      case GL_PACK_IMAGE_HEIGHT:
          ps->packModes.imageHeight = value;
          break;

      case GL_UNPACK_ROW_LENGTH:
          if (value < 0) {
              __glSetError(GL_INVALID_VALUE);
              return;
          }
          ps->unpackModes.lineLength = value;
          break;
      case GL_UNPACK_SKIP_ROWS:
          if (value < 0) {
              __glSetError(GL_INVALID_VALUE);
              return;
          }
          ps->unpackModes.skipLines = value;
          break;
      case GL_UNPACK_SKIP_PIXELS:
          if (value < 0) {
              __glSetError(GL_INVALID_VALUE);
              return;
          }
          ps->unpackModes.skipPixels = value;
          break;
      case GL_UNPACK_ALIGNMENT:
          switch (value) {
              case 1: case 2: case 4: case 8:
                  ps->unpackModes.alignment = value;
                  break;
              default:
                  __glSetError(GL_INVALID_VALUE);
                  return;
          }
          break;
      case GL_UNPACK_SWAP_BYTES:
          ps->unpackModes.swapEndian = (value != 0);
          break;
      case GL_UNPACK_LSB_FIRST:
          ps->unpackModes.lsbFirst = (value != 0);
          break;
      case GL_UNPACK_SKIP_IMAGES:
          ps->unpackModes.skipImages = value;
          break;
      case GL_UNPACK_IMAGE_HEIGHT:
          ps->unpackModes.imageHeight = value;
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    /*Immediate dp notification*/
    (*gc->dp.pixelStore)(gc);

    /*Notify swp*/
    __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_PIXELSTORE_BIT);
}

/*
** Specify zoom factor that affects drawing and copying of pixel arrays
*/
GLvoid APIENTRY __glim_PixelZoom(GLfloat xfactor, GLfloat yfactor)
{
    __GLpixelState *ps;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_PixelZoom",DT_GLfloat, xfactor, DT_GLfloat, yfactor, DT_GLnull);
#endif

    __GL_VERTEX_BUFFER_FLUSH(gc);

    ps = &gc->state.pixel;
    ps->transferMode.zoomX = xfactor;
    ps->transferMode.zoomY = yfactor;

    /*Immediate dp notification*/
    (*gc->dp.pixelZoom)(gc);

    /*Notify swp*/
    __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_PIXELZOOM_BIT);
}

/*
** Specify modes that control the transfer of pixel arrays.
*/
GLvoid APIENTRY __glim_PixelTransferf(GLenum mode, GLfloat value)
{
    __GLpixelState *ps;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_PixelTransferf",DT_GLenum, mode, DT_GLfloat, value, DT_GLnull);
#endif

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
#if GL_EXT_convolution
      case GL_POST_CONVOLUTION_RED_SCALE:
          ps->transferMode.postConvolutionScale.r = value;
          break;
      case GL_POST_CONVOLUTION_GREEN_SCALE:
          ps->transferMode.postConvolutionScale.g = value;
          break;
      case GL_POST_CONVOLUTION_BLUE_SCALE:
          ps->transferMode.postConvolutionScale.b = value;
          break;
      case GL_POST_CONVOLUTION_ALPHA_SCALE:
          ps->transferMode.postConvolutionScale.a = value;
          break;
      case GL_POST_CONVOLUTION_RED_BIAS:
          ps->transferMode.postConvolutionBias.r = value;
          break;
      case GL_POST_CONVOLUTION_GREEN_BIAS:
          ps->transferMode.postConvolutionBias.g = value;
          break;
      case GL_POST_CONVOLUTION_BLUE_BIAS:
          ps->transferMode.postConvolutionBias.b = value;
          break;
      case GL_POST_CONVOLUTION_ALPHA_BIAS:
          ps->transferMode.postConvolutionBias.a = value;
          break;
#endif
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
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    /*Immediate dp notification*/
    (*gc->dp.pixelTransfer)(gc);

    /*Notify swp*/
    __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_PIXELTRANSFER_BIT);
}

GLvoid APIENTRY __glim_PixelTransferi( GLenum mode, GLint value)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_PixelTransferi", DT_GLenum, mode, DT_GLint, value, DT_GLnull);
#endif

    __glim_PixelTransferf(mode, (GLfloat) value);
}

/************************************************************************/

/*
** Functions to specify mapping of pixel colors and stencil values.
*/
GLvoid APIENTRY __glim_PixelMapfv(GLenum map, GLint mapSize,
                                const GLfloat values[])
{
    __GLpixelState *ps;
    __GLpixelMapHead *pMap;
    GLint index = map - GL_PIXEL_MAP_I_TO_I;
    GLfloat value;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_PixelMapfv", DT_GLenum, map, DT_GLint, mapSize, DT_GLfloat_ptr, values, DT_GLnull);
#endif

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
              __glSetError(GL_INVALID_VALUE);
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
              __glSetError(GL_INVALID_VALUE);
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
              __glSetError(GL_INVALID_VALUE);
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
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    /*Immediate dp notification*/
    (*gc->dp.pixelMap)(gc, GL_FLOAT);

    /*Notify swp*/
    __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_PIXELTRANSFER_BIT);
}

GLvoid APIENTRY __glim_PixelMapuiv(GLenum map, GLint mapSize,
                                 const GLuint values[])
{
    __GLpixelState *ps;
    __GLpixelMapHead *pMap;
    GLint index = map - GL_PIXEL_MAP_I_TO_I;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_PixelMapuiv", DT_GLenum, map, DT_GLint, mapSize, DT_GLuint_ptr, values, DT_GLnull);
#endif

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
              __glSetError(GL_INVALID_VALUE);
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
              __glSetError(GL_INVALID_VALUE);
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
              __glSetError(GL_INVALID_VALUE);
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
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    /*Immediate dp notification*/
    (*gc->dp.pixelMap)(gc, GL_UNSIGNED_INT);

    /*Notify swp*/
    __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_PIXELTRANSFER_BIT);
}

GLvoid APIENTRY __glim_PixelMapusv(GLenum map, GLint mapSize,
                                 const GLushort values[])
{
    __GLpixelState *ps;
    __GLpixelMapHead *pMap;
    GLint index = map - GL_PIXEL_MAP_I_TO_I;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_PixelMapusv", DT_GLenum, map, DT_GLint, mapSize, DT_GLushort_ptr, values, DT_GLnull);
#endif

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
              __glSetError(GL_INVALID_VALUE);
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
              __glSetError(GL_INVALID_VALUE);
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
              __glSetError(GL_INVALID_VALUE);
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
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    /*Immediate dp notification*/
    (*gc->dp.pixelMap)(gc, GL_UNSIGNED_SHORT);

    /*Notify swp*/
    __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_PIXELTRANSFER_BIT);
}

GLboolean __glCheckUnpackArgs(__GLcontext *gc, GLenum format, GLenum type)
{
    /*if gl is in color index mode*/
    if( !gc->modes.rgbMode )
    {
        switch( format )
        {
        case GL_STENCIL_INDEX:
        case GL_COLOR_INDEX:
        case GL_DEPTH_COMPONENT:
            break;
        default:
            __glSetError(GL_INVALID_OPERATION);
            return GL_FALSE;
        }
    }

    /*check format*/
    switch (format)
    {
    case GL_STENCIL_INDEX:
    case GL_DEPTH_COMPONENT:
        break;

    case GL_COLOR_INDEX:
    case GL_RED:
    case GL_GREEN:
    case GL_BLUE:
    case GL_ALPHA:
    case GL_RGB:
    case GL_RGBA:
    case GL_BGR:
    case GL_BGRA:
    case GL_LUMINANCE:
    case GL_LUMINANCE_ALPHA:
        break;
    case GL_ABGR_EXT:
        if(!__glExtension[INDEX_EXT_abgr].bEnabled )
        {
            __glSetError(GL_INVALID_ENUM);
            return GL_FALSE;
        }
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
        if(!__glExtension[INDEX_EXT_texture_integer].bEnabled)
        {
            __glSetError(GL_INVALID_ENUM);
            return GL_FALSE;
        }
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return GL_FALSE;
    }

    /*check type*/
    switch (type)
    {
    case GL_BITMAP:
        switch( format )
        {
        case GL_COLOR_INDEX:
        case GL_STENCIL_INDEX:
            break;
        default:
            __glSetError(GL_INVALID_ENUM);
            return GL_FALSE;
        }
        break;

    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    case GL_INT:
    case GL_UNSIGNED_INT:
      break;
    case GL_FLOAT:
        switch(format)
        {
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
                __glSetError(GL_INVALID_ENUM);
                return GL_FALSE;
        }
        break;

    case GL_UNSIGNED_BYTE_3_3_2:
    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_BYTE_2_3_3_REV:
    case GL_UNSIGNED_SHORT_5_6_5_REV:
        switch (format)
        {
            case GL_RGB:
            case GL_BGR:
                break;
            default:
                __glSetError(GL_INVALID_OPERATION);
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
        switch (format)
        {
            case GL_RGBA:
            case GL_ABGR_EXT:
            case GL_BGRA:
                break;
            default:
                __glSetError(GL_INVALID_OPERATION);
                return GL_FALSE;
        }
        break;
    case GL_UNSIGNED_INT_5_9_9_9_REV_EXT:
        if(!__glExtension[INDEX_EXT_texture_shared_exponent].bEnabled)
        {
            __glSetError(GL_INVALID_ENUM);
            return GL_FALSE;
        }

        if(format != GL_RGB)
        {
            __glSetError(GL_INVALID_ENUM);
            return GL_FALSE;
        }
        break;
    case GL_HALF_FLOAT_ARB:
        if (!__glExtension[INDEX_ARB_half_float_pixel].bEnabled)
        {
            __glSetError(GL_INVALID_ENUM);
            return GL_FALSE;
        }
        break;
    case GL_UNSIGNED_INT_10F_11F_11F_REV_EXT:
        if(!__glExtension[INDEX_EXT_packed_float].bEnabled)
        {
            __glSetError(GL_INVALID_ENUM);
            return GL_FALSE;
        }

        if (format != GL_RGB)
        {
            __glSetError(GL_INVALID_OPERATION);
            return GL_FALSE;
        }
    default:
        __glSetError(GL_INVALID_ENUM);
        return GL_FALSE;
    }

    return GL_TRUE;
}

GLboolean __glCheckDrawPixelArgs(__GLcontext *gc, GLsizei width, GLsizei height,
                                 GLenum format, GLenum type)
{
    if ((width < 0) || (height < 0))
    {
        __glSetError(GL_INVALID_VALUE);
        return GL_FALSE;
    }

    switch (format)
    {
    case GL_STENCIL_INDEX:
        if (!gc->modes.haveStencilBuffer)
        {
            __glSetError(GL_INVALID_OPERATION);
            return GL_FALSE;
        }
    case GL_DEPTH_COMPONENT:
        if( !gc->modes.haveDepthBuffer )
        {
            __glSetError(GL_INVALID_OPERATION);
            return GL_FALSE;
        }
        break;
    }

    return __glCheckUnpackArgs(gc, format,type);
}

GLboolean __glCheckReadPixelArgs(__GLcontext *gc, GLsizei width, GLsizei height,
                                 GLenum format, GLenum type)
{
    if(READ_FRAMEBUFFER_BINDING_NAME != 0)
    {
        if(!gc->dp.isFramebufferComplete(gc, gc->frameBuffer.readFramebufObj))
        {
            __glSetError(GL_INVALID_FRAMEBUFFER_OPERATION_EXT);
            return GL_FALSE;
        }
        else
        {
            if(gc->frameBuffer.readFramebufObj->fbSamples > 0)
            {
                __glSetError(GL_INVALID_FRAMEBUFFER_OPERATION_EXT);
                return GL_FALSE;
            }

            if (gc->frameBuffer.readFramebufObj->fbInteger)
            {
                switch(format)
                {
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
                  break;
                default:
                    __glSetError(GL_INVALID_OPERATION);
                    return GL_FALSE;
                }
            }
        }
    }

    if ((width < 0) || (height < 0)) {
        __glSetError(GL_INVALID_VALUE);
        return GL_FALSE;
    }
    switch (format)
    {
      case GL_STENCIL_INDEX:
          if (!gc->modes.haveStencilBuffer) {
              __glSetError(GL_INVALID_OPERATION);
              return GL_FALSE;
          }
          break;
      case GL_COLOR_INDEX:
          //        if (gc->modes.rgbMode) {
          /* Can't convert RGB to color index */
          __glSetError(GL_INVALID_OPERATION);
          return GL_FALSE;
          //        }
          //        break;
      case GL_DEPTH_COMPONENT:
          if (!gc->modes.haveDepthBuffer) {
              __glSetError(GL_INVALID_OPERATION);
              return GL_FALSE;
          }
          break;
      case GL_DEPTH_STENCIL_EXT:
          if (!gc->modes.haveDepthBuffer || !gc->modes.haveStencilBuffer) {
              __glSetError(GL_INVALID_OPERATION);
              return GL_FALSE;
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
          if (!__glExtension[INDEX_EXT_texture_integer].bEnabled || (type == GL_FLOAT ))
          {
             __glSetError(GL_INVALID_ENUM);
             return GL_FALSE;
          }

          if (!gc->frameBuffer.readFramebufObj->fbInteger)
          {
             __glSetError(GL_INVALID_OPERATION);
             return GL_FALSE;
          }
          break;

      default:
          __glSetError(GL_INVALID_ENUM);
          return GL_FALSE;
    }

    switch (type)
    {
      case GL_BITMAP:
          if (format != GL_STENCIL_INDEX && format != GL_COLOR_INDEX) {
              __glSetError(GL_INVALID_OPERATION);
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
          /*currently only for GL_DEPTH_STENCIL_EXT , to do more if needed*/
          if ( format != GL_DEPTH_STENCIL_EXT)
          {
                  __glSetError(GL_INVALID_OPERATION);
                  return GL_FALSE;
          }
          break;
      case GL_UNSIGNED_BYTE_3_3_2:
      case GL_UNSIGNED_SHORT_5_6_5:
      case GL_UNSIGNED_BYTE_2_3_3_REV:
      case GL_UNSIGNED_SHORT_5_6_5_REV:
          switch (format) {
              case GL_RGB:
              case GL_BGR:
                  break;
              default:
                  __glSetError(GL_INVALID_OPERATION);
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
                  __glSetError(GL_INVALID_OPERATION);
                  return GL_FALSE;
          }
          break;
    case GL_UNSIGNED_INT_5_9_9_9_REV_EXT:
        if (!__glExtension[INDEX_EXT_texture_shared_exponent].bEnabled)
        {
            __glSetError(GL_INVALID_ENUM);
            return GL_FALSE;
        }

        if(format != GL_RGB)
        {
            __glSetError(GL_INVALID_ENUM);
            return GL_FALSE;
        }
        break;
    case GL_HALF_FLOAT_ARB:
        if (!__glExtension[INDEX_ARB_half_float_pixel].bEnabled)
        {
            __glSetError(GL_INVALID_ENUM);
            return GL_FALSE;
        }
        break;

    case GL_UNSIGNED_INT_10F_11F_11F_REV_EXT:
        if (!__glExtension[INDEX_EXT_packed_float].bEnabled)
        {
            __glSetError(GL_INVALID_ENUM);
            return GL_FALSE;
        }

        if(format != GL_RGB)
        {
            __glSetError(GL_INVALID_ENUM);
            return GL_FALSE;
        }
        break;

    default:
          __glSetError(GL_INVALID_ENUM);
          return GL_FALSE;
    }

    return GL_TRUE;
}
GLvoid APIENTRY __glim_DrawPixels(GLsizei width, GLsizei height, GLenum format,
                                  GLenum type, const GLvoid *pixels)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DrawPixels", DT_GLsizei, width, DT_GLsizei, height, DT_GLenum, format, DT_GLenum, type, DT_GLvoid_ptr, pixels, DT_GLnull);
#endif

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
         (format == GL_STENCIL_INDEX && !gc->state.stencil.StencilArb.front.writeMask) ||
         (format == GL_DEPTH_STENCIL_EXT && !gc->state.depth.writeEnable && !gc->state.stencil.StencilArb.front.writeMask))
    {
        return;
    }

    /* Get the latest drawable information */
    LINUX_LOCK_FRAMEBUFFER(gc);

    /* Check all GL attributes for redundancy before notify device pipeline.
    */
    __glEvaluateAttribDrawableChange(gc);

    if ((gc->flags & __GL_DISCARD_FOLLOWING_DRAWS_FRAMEBUFFER_NOT_COMPLETE) == 0)
    {
        (*gc->dp.rasterBegin)(gc, __GL_RASTERFUNC_DRAWPIX,format, width, height);
        (*gc->pipeline->drawPixels)(gc, width, height, format, type, (GLubyte *)pixels);
        (*gc->dp.rasterEnd)(gc, __GL_RASTERFUNC_DRAWPIX);
    }
    LINUX_UNLOCK_FRAMEBUFFER(gc);
}

GLvoid APIENTRY __glim_ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height,
                                  GLenum format, GLenum type, GLvoid *buf)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ReadPixels", DT_GLint, x, DT_GLint, y, DT_GLsizei, width, DT_GLsizei, height, DT_GLenum, format, DT_GLenum, type, DT_GLvoid_ptr, buf, DT_GLnull);
#endif

    if (!__glCheckReadPixelArgs(gc, width, height, format, type)) {
        return;
    }

    if (gc->readablePrivate->width * gc->readablePrivate->height == 0 )
    {
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Get the latest drawable information */
    LINUX_LOCK_FRAMEBUFFER(gc);

    /* Check all GL attributes for redundancy before notify device pipeline.
    */
    __glEvaluateAttribDrawableChange(gc);


    if ((gc->flags & __GL_DISCARD_FOLLOWING_DRAWS_FRAMEBUFFER_NOT_COMPLETE) == 0)
    {
        /* RasterBegin should confirm previous draw commited before read from FB. */
        (*gc->dp.rasterBegin)(gc, __GL_RASTERFUNC_READPIX,format, width, height);
        (*gc->pipeline->readPixels)(gc, x, y, width, height, format, type, (GLubyte*)buf);
        (*gc->dp.rasterEnd)(gc, __GL_RASTERFUNC_READPIX);
    }
    LINUX_UNLOCK_FRAMEBUFFER(gc);
}

GLvoid APIENTRY __glim_CopyPixels(GLint x, GLint y, GLsizei width, GLsizei height,
                                GLenum type)
{
    GLenum format;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_CopyPixels", DT_GLint, x, DT_GLint, y, DT_GLsizei, width, DT_GLsizei, height, DT_GLenum, type, DT_GLnull);
#endif

    if(READ_FRAMEBUFFER_BINDING_NAME != 0)
    {
        if(!gc->dp.isFramebufferComplete(gc, gc->frameBuffer.readFramebufObj))
        {
            __glSetError(GL_INVALID_FRAMEBUFFER_OPERATION_EXT);
            return;
        }
        else
        {
            if(gc->frameBuffer.readFramebufObj->fbSamples > 0)
            {
                __glSetError(GL_INVALID_FRAMEBUFFER_OPERATION_EXT);
                return;
            }

            if (gc->frameBuffer.readFramebufObj->fbInteger)
            {
                GL_ASSERT(0);
                return;
            }
        }

    }

    if ((width < 0) || (height < 0)) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    switch (type) {
      case GL_STENCIL:
          if (!gc->modes.haveStencilBuffer) {
              __glSetError(GL_INVALID_OPERATION);
              return;
          }
          format = GL_STENCIL_INDEX;
          break;
      case GL_COLOR:
          format = gc->modes.rgbMode ? GL_RGBA : GL_COLOR_INDEX;
          break;
      case GL_DEPTH:
          if (!gc->modes.haveDepthBuffer) {
              __glSetError(GL_INVALID_OPERATION);
              return;
          }
          format = GL_DEPTH_COMPONENT;
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
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
        (format == GL_STENCIL_INDEX && !gc->state.stencil.StencilArb.front.writeMask) ||
        (format == GL_DEPTH_STENCIL_EXT && !gc->state.depth.writeEnable && !gc->state.stencil.StencilArb.front.writeMask))
    {
        return;
    }

    /* Get the latest drawable information */
    LINUX_LOCK_FRAMEBUFFER(gc);

    /* Check all GL attributes for redundancy before notify device pipeline.
    */
    __glEvaluateAttribDrawableChange(gc);

    if ((gc->flags & __GL_DISCARD_FOLLOWING_DRAWS_FRAMEBUFFER_NOT_COMPLETE) == 0)
    {
        (*gc->dp.rasterBegin)(gc, __GL_RASTERFUNC_COPYPIX, format, width, height);
        (*gc->pipeline->copyPixels)(gc, x, y, width, height, format);
        (*gc->dp.rasterEnd)(gc, __GL_RASTERFUNC_COPYPIX);
    }
    LINUX_UNLOCK_FRAMEBUFFER(gc);
}

GLvoid APIENTRY __glim_Bitmap(GLsizei w, GLsizei h, GLfloat xOrig, GLfloat yOrig,
                            GLfloat xMove, GLfloat yMove, const GLubyte *bitmap)
{
    GLuint unpackBuffer;
    __GLbufferObject *bufObj = NULL;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Bitmap",DT_GLsizei, w, DT_GLsizei, h, DT_GLfloat, xOrig, DT_GLfloat, yOrig, DT_GLfloat, xMove, DT_GLfloat, yMove, DT_GLubyte_ptr, bitmap, DT_GLnull);
#endif

    if ((w < 0) || (h < 0)) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if (!gc->state.rasterPos.validRasterPos) {
        return;
    }

    if(!gc->drawablePrivate->width || !gc->drawablePrivate->height)
    {
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Get the latest drawable information */
    LINUX_LOCK_FRAMEBUFFER(gc);

    /* Check all GL attributes for redundancy before notify device pipeline.
    */
    __glEvaluateAttribDrawableChange(gc);

    if ((gc->flags & __GL_DISCARD_FOLLOWING_DRAWS_FRAMEBUFFER_NOT_COMPLETE) == 0)
    {
        (*gc->dp.rasterBegin)(gc, __GL_RASTERFUNC_BITMAP,GL_RGBA, 0, 0);

        /* The image is from unpack buffer object? */
        unpackBuffer = gc->bufferObject.boundBuffer[__GL_PIXEL_UNPACK_BUFFER_INDEX];

        /* If the source is from a buffer object and HW can't support this kind of blt, patch the address using system cache in buffer object. */
        if (unpackBuffer > 0)
        {
            GL_ASSERT(unpackBuffer > 0);

            bufObj = (__GLbufferObject *)__glGetObject(gc, gc->bufferObject.shared, unpackBuffer);
            if (bufObj == NULL)
            {
                GL_ASSERT(0);
                LINUX_UNLOCK_FRAMEBUFFER(gc);
                return;
            }
        }

        (*gc->pipeline->bitmaps)(gc, w, h, xOrig, yOrig, xMove, yMove, bitmap, bufObj);
        (*gc->dp.rasterEnd)(gc, __GL_RASTERFUNC_BITMAP);

        /* Advance current raster position. */
        {
            GLboolean yInvert;
            yInvert = (DRAW_FRAMEBUFFER_BINDING_NAME == 0)?gc->drawablePrivate->yInverted: GL_FALSE;
            gc->state.rasterPos.rPos.winPos.x += xMove;
            gc->state.rasterPos.rPos.winPos.y += (yInvert? -1 : 1 ) * yMove;
        }
    }
    LINUX_UNLOCK_FRAMEBUFFER(gc);
}

GLvoid APIENTRY __glim_Accum(GLenum op, GLfloat value)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Accum", DT_GLenum, op, DT_GLfloat, value, DT_GLnull);
#endif

    if (!gc->modes.haveAccumBuffer) {
        __glSetError(GL_INVALID_OPERATION);
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
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Get the latest drawable information */
    LINUX_LOCK_FRAMEBUFFER(gc);

    /* Check all GL attributes for redundancy before notify device pipeline.
    ** Must be call here because __GL_CLEAR_ATTR_BITS affect dp.clear.
    */
    __glEvaluateAttribDrawableChange(gc);


    if (gc->renderMode == GL_RENDER) {
        (*gc->dp.accum)(gc, op, value);
    }

    LINUX_UNLOCK_FRAMEBUFFER(gc);
}

/*****************************************************************************/

GLvoid APIENTRY __glim_GetPixelMapfv(GLenum map, GLfloat buf[])
{
    GLint index;
    GLint limit;
    GLfloat *rp;
    __GLpixelMapHead *pMap;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetPixelMapfv", DT_GLenum, map, DT_GLfloat, buf, DT_GLnull);
#endif

    pMap = gc->state.pixel.pixelMap;
    index = map - GL_PIXEL_MAP_I_TO_I;
    rp = buf;
    switch (map)
    {
    case GL_PIXEL_MAP_I_TO_I:
    case GL_PIXEL_MAP_S_TO_S:
        {
            GLint *fromp = pMap[index].base.mapI;
            limit = pMap[index].size;
            while (--limit >= 0) {
                *rp++ = (GLfloat)*fromp++;
            }
        }
        break;
    case GL_PIXEL_MAP_I_TO_R:
    case GL_PIXEL_MAP_I_TO_G:
    case GL_PIXEL_MAP_I_TO_B:
    case GL_PIXEL_MAP_I_TO_A:
    case GL_PIXEL_MAP_R_TO_R:
    case GL_PIXEL_MAP_G_TO_G:
    case GL_PIXEL_MAP_B_TO_B:
    case GL_PIXEL_MAP_A_TO_A:
        {
            __GLfloat *fromp = pMap[index].base.mapF;
            limit = pMap[index].size;
            while (--limit >= 0) {
                *rp++ = *fromp++;
            }
        }
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

GLvoid APIENTRY __glim_GetPixelMapuiv(GLenum map, GLuint buf[])
{
    GLint index;
    GLint limit;
    GLuint *rp;
    __GLpixelMapHead *pMap;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetPixelMapuiv", DT_GLenum, map, DT_GLuint_ptr, buf, DT_GLnull);
#endif

    pMap = gc->state.pixel.pixelMap;
    index = map - GL_PIXEL_MAP_I_TO_I;
    rp = buf;
    switch (map)
    {
    case GL_PIXEL_MAP_I_TO_I:
    case GL_PIXEL_MAP_S_TO_S:
        {
            GLint *fromp = pMap[index].base.mapI;
            limit = pMap[index].size;
            while (--limit >= 0) {
                *rp++ = *fromp++;
            }
        }
        break;
    case GL_PIXEL_MAP_I_TO_R:
    case GL_PIXEL_MAP_I_TO_G:
    case GL_PIXEL_MAP_I_TO_B:
    case GL_PIXEL_MAP_I_TO_A:
    case GL_PIXEL_MAP_R_TO_R:
    case GL_PIXEL_MAP_G_TO_G:
    case GL_PIXEL_MAP_B_TO_B:
    case GL_PIXEL_MAP_A_TO_A:
        {
            __GLfloat *fromp = pMap[index].base.mapF;
            limit = pMap[index].size;
            while (--limit >= 0) {
                *rp++ = (GLuint) *fromp++;
            }
        }
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

GLvoid APIENTRY __glim_GetPixelMapusv(GLenum map, GLushort buf[])
{
    GLint index;
    GLint limit;
    GLushort *rp;
    __GLpixelMapHead *pMap;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetPixelMapusv", DT_GLenum, map, DT_GLushort_ptr, buf, DT_GLnull);
#endif

    pMap = gc->state.pixel.pixelMap;
    index = map - GL_PIXEL_MAP_I_TO_I;
    rp = buf;
    switch (map)
    {
    case GL_PIXEL_MAP_I_TO_I:
    case GL_PIXEL_MAP_S_TO_S:
        {
            GLint *fromp = pMap[index].base.mapI;
            limit = pMap[index].size;
            while (--limit >= 0) {
                *rp++ = (GLushort) *fromp++;
            }
        }
        break;
    case GL_PIXEL_MAP_I_TO_R:
    case GL_PIXEL_MAP_I_TO_G:
    case GL_PIXEL_MAP_I_TO_B:
    case GL_PIXEL_MAP_I_TO_A:
    case GL_PIXEL_MAP_R_TO_R:
    case GL_PIXEL_MAP_G_TO_G:
    case GL_PIXEL_MAP_B_TO_B:
    case GL_PIXEL_MAP_A_TO_A:
        {
            __GLfloat *fromp = pMap[index].base.mapF;
            limit = pMap[index].size;
            while (--limit >= 0) {
                *rp++ = (GLushort) *fromp++;
            }
        }
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

/*********************************************************************************************/

static GLenum colorTableTarget[__GL_NUM_COLOR_TABLE_TARGETS] = {
    GL_COLOR_TABLE,
    GL_POST_CONVOLUTION_COLOR_TABLE,
    GL_POST_COLOR_MATRIX_COLOR_TABLE
};

static GLenum proxyColorTableTarget[__GL_NUM_COLOR_TABLE_TARGETS] = {
    GL_PROXY_COLOR_TABLE,
    GL_PROXY_POST_CONVOLUTION_COLOR_TABLE,
    GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE
};

static GLenum convolutionTarget[__GL_NUM_CONVOLUTION_TARGETS] = {
    GL_CONVOLUTION_1D,
    GL_CONVOLUTION_2D,
    GL_SEPARABLE_2D
};

__GL_INLINE GLvoid __glInitColorTable(__GLcolorTable *ct, GLenum target)
{
    /* Initialize color table */
    ct->target = target;

    ct->state.scale.r = 1.0;
    ct->state.scale.g = 1.0;
    ct->state.scale.b = 1.0;
    ct->state.scale.a = 1.0;
    ct->state.bias.r = 0.0;
    ct->state.bias.g = 0.0;
    ct->state.bias.b = 0.0;
    ct->state.bias.a = 0.0;

    ct->width = 0;
    ct->formatReturn = GL_RGBA;
    ct->redSize = 0;
    ct->greenSize = 0;
    ct->blueSize = 0;
    ct->alphaSize = 0;
    ct->luminanceSize = 0;
    ct->intensitySize = 0;
    ct->table = NULL;
}

__GL_INLINE GLvoid __glInitConvolutionFilter(__GLconvolutionFilter *cf, GLenum target)
{
    /* Initialize convolution filter */
    cf->target = target;
    cf->formatReturn = GL_RGBA;
    cf->width = 0;
    cf->height = 0;
    cf->filter = NULL;

    cf->state.scale.r = 1.0;
    cf->state.scale.g = 1.0;
    cf->state.scale.b = 1.0;
    cf->state.scale.a = 1.0;
    cf->state.bias.r = 0.0;
    cf->state.bias.g = 0.0;
    cf->state.bias.b = 0.0;
    cf->state.bias.a = 0.0;
    cf->state.borderMode = GL_REDUCE;
    cf->state.borderColor.r = 0.0;
    cf->state.borderColor.g = 0.0;
    cf->state.borderColor.b = 0.0;
    cf->state.borderColor.a = 0.0;
}

__GL_INLINE GLvoid __glInitHistogramState(__GLhistogram *hs)
{
    /* Initialize histogram */
    hs->width = 0;
    hs->formatReturn = GL_RGBA;
    hs->redSize = 0;
    hs->greenSize = 0;
    hs->blueSize = 0;
    hs->alphaSize = 0;
    hs->luminanceSize = 0;
    hs->sink = GL_FALSE;
}

__GL_INLINE GLvoid __glInitMinmaxState(__GLminmax *ms)
{
    /* Initialize Minmax */
    ms->sink = GL_FALSE;
    ms->format =
    ms->formatReturn =
    ms->baseFormat = GL_RGBA;
    ms->type = GL_FLOAT;
}

GLvoid __glInitPixelState(__GLcontext *gc)
{
    __GLpixelState *ps = &gc->state.pixel;
    __GLclientPixelState *cps = &gc->clientState.pixel;
    GLenum m;
    GLint i;

    /*client state:pack/unpack modes*/
    cps->packModes.alignment = 4;
    cps->packModes.imageHeight = 0;
    cps->packModes.lineLength = 0;
    cps->packModes.lsbFirst = GL_FALSE;
    cps->packModes.skipImages = 0;
    cps->packModes.skipLines = 0;
    cps->packModes.skipPixels = 0;
    cps->packModes.swapEndian = GL_FALSE;

    cps->unpackModes.alignment = 4;
    cps->unpackModes.imageHeight = 0;
    cps->unpackModes.lineLength = 0;
    cps->unpackModes.lsbFirst = GL_FALSE;
    cps->unpackModes.skipImages = 0;
    cps->unpackModes.skipLines = 0;
    cps->unpackModes.skipPixels = 0;
    cps->unpackModes.swapEndian = GL_FALSE;

    ps->transferMode.r_scale = 1.0;
    ps->transferMode.g_scale = 1.0;
    ps->transferMode.b_scale = 1.0;
    ps->transferMode.a_scale = 1.0;
    ps->transferMode.d_scale = 1.0;
    ps->transferMode.zoomX = 1.0;
    ps->transferMode.zoomY = 1.0;

    ps->transferMode.postConvolutionScale.r = 1.0;
    ps->transferMode.postConvolutionScale.g = 1.0;
    ps->transferMode.postConvolutionScale.b = 1.0;
    ps->transferMode.postConvolutionScale.a = 1.0;
    ps->transferMode.postConvolutionBias.r  = 0.0;
    ps->transferMode.postConvolutionBias.g  = 0.0;
    ps->transferMode.postConvolutionBias.b  = 0.0;
    ps->transferMode.postConvolutionBias.a  = 0.0;

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

    /* Initialize color tables.
    */
    for (i = 0; i < __GL_NUM_COLOR_TABLE_TARGETS; i++) {
        __glInitColorTable(&ps->colorTable[i], colorTableTarget[i]);
        __glInitColorTable(&ps->proxyColorTable[i], proxyColorTableTarget[i]);
    }

    /* Initialize convolution filters.
    */
    for (i = 0; i < __GL_NUM_CONVOLUTION_TARGETS; i++) {
        __glInitConvolutionFilter(&ps->convolutionFilter[i], convolutionTarget[i]);
    }

    /* Initialize histogram.
    */
    __glInitHistogramState(&ps->histogram);
    __glInitHistogramState(&ps->proxyHistogram);

    /* Initialize minmax.
    */
    __glInitMinmaxState(&ps->minmax);
}

GLvoid __glFreePixelSpanInfo( __GLcontext *gc )
{
    if ( gc->pixel.spanInfo )
    {
        (*gc->imports.free)(gc, gc->pixel.spanInfo);
        gc->pixel.spanInfo = NULL;

    }
}

GLuint __glInitPixelSpanInfo( __GLcontext *gc )
{
    if ( !gc->pixel.spanInfo )
    {
        gc->pixel.spanInfo =
            (__GLpixelSpanInfo*) (*gc->imports.malloc)( gc, sizeof(__GLpixelSpanInfo)  );
        if ( !gc->pixel.spanInfo )
        {
            __glSetError(GL_OUT_OF_MEMORY);
            return GL_FALSE;
        }
    }
    return GL_TRUE;
}

GLvoid __glFreePixelMapState(__GLcontext *gc)
{
    __GLpixelMachine*pm = &gc->pixel;
    GLenum m;
    GLuint i;

    for (m = GL_PIXEL_MAP_I_TO_I; m <= GL_PIXEL_MAP_A_TO_A; m++) {
        __glFreeDefaultPixelMap(gc, m);
    }

    if (pm->redMap) {
        (*gc->imports.free)(gc, pm->redMap);
        pm->redMap = NULL;
    }

    if (pm->greenMap) {
        (*gc->imports.free)(gc, pm->greenMap);
        pm->greenMap = NULL;
    }

    if (pm->blueMap) {
        (*gc->imports.free)(gc, pm->blueMap);
        pm->blueMap = NULL;
    }

    if (pm->alphaMap) {
        (*gc->imports.free)(gc, pm->alphaMap);
        pm->alphaMap = NULL;
    }

    if (pm->iMap) {
        (*gc->imports.free)(gc, pm->iMap);
        pm->iMap = NULL;
    }

    for (i = 0; i < __GL_NUM_CONVOLUTION_TARGETS; i++) {
        if(gc->state.pixel.convolutionFilter[i].filter)
        {
            (*gc->imports.free)(gc, gc->state.pixel.convolutionFilter[i].filter);
            gc->state.pixel.convolutionFilter[i].filter = NULL;
        }
    }
}

__GL_INLINE GLvoid __glInitPixelColorScales(__GLcontext *gc)
{
    __GLpixelMachine*pm = &gc->pixel;
    GLuint mask, redMax;
    GLfloat redScale, greenScale, blueScale, alphaScale;
    GLint i;

    if (pm->redMap == NULL) {
        /* First time allocation of these maps */

        /*
        ** These lookup tables are for type UNSIGNED_BYTE, so they are sized
        ** to 256 entries.  They map from UNSIGNED_BYTE to internal scaled
        ** floating point colors.
        */
        pm->redMap =
            (GLfloat*) (*gc->imports.malloc)(gc, 256 * sizeof(GLfloat) );
        pm->greenMap =
            (GLfloat*) (*gc->imports.malloc)(gc, 256 * sizeof(GLfloat) );
        pm->blueMap =
            (GLfloat*) (*gc->imports.malloc)(gc, 256 * sizeof(GLfloat) );
        pm->alphaMap =
            (GLfloat*) (*gc->imports.malloc)(gc, 256 * sizeof(GLfloat) );
        pm->iMap =
            (GLfloat*) (*gc->imports.malloc)(gc, 256 * sizeof(GLfloat) );
    }

    mask = gc->modes.redMask;
    for (i=0; mask; i++, mask>>=1) if (mask & 1) break;
    redScale = (GLfloat)mask / 255;
    redMax = mask;

    mask = gc->modes.greenMask;
    for (i=0; mask; i++, mask>>=1) if (mask & 1) break;
    greenScale = (GLfloat)mask / 255;

    mask = gc->modes.blueMask;
    for (i=0; mask; i++, mask>>=1) if (mask & 1) break;
    blueScale = (GLfloat)mask / 255;

    mask = gc->modes.alphaMask;
    for (i=0; mask; i++, mask>>=1) if (mask & 1) break;
    if(mask)
        alphaScale = (GLfloat)mask / 255;
    else
        alphaScale = 1.0;

    for (i=0; i<256; i++) {
        pm->redMap[i] = i * redScale;
        pm->greenMap[i] = i * greenScale;
        pm->blueMap[i] = i * blueScale;
        pm->alphaMap[i] = i * alphaScale;
        pm->iMap[i] = (GLfloat)(i & redMax);
    }

    /*
    ** Invalidate the RGBA modify tables so that they will be
    ** recomputed using the current color buffer scales.
    */
    pm->rgbaCurrent = GL_FALSE;
}

GLvoid __glInitPixelMachine(__GLcontext *gc)
{
    /*Initialize color scale table dase on color depth
    */
    __glInitPixelColorScales(gc);
}

