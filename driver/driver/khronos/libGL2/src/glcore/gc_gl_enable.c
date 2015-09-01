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

GLvoid APIENTRY __glim_EnableClientState(GLenum array);

GLvoid __glSetTexEnableDimension(__GLcontext *gc, GLuint unit)
{
    __GLTextureEnableState *textureEnable = &gc->state.enables.texUnits[unit];
    GLuint origEnabledDim = textureEnable->enabledDimension;

    /* set enabledMask and enabledDimension */
    gc->texture.enabledMask |= (1 << unit);
    if(textureEnable->textureCubeMap)
        textureEnable->enabledDimension = __GL_TEXTURE_CUBEMAP_INDEX + 1;
    else if(textureEnable->texture3D)
        textureEnable->enabledDimension = __GL_TEXTURE_3D_INDEX + 1;
    else if(textureEnable->textureRec)
        textureEnable->enabledDimension = __GL_TEXTURE_RECTANGLE_INDEX + 1;
    else if(textureEnable->texture2D)
        textureEnable->enabledDimension = __GL_TEXTURE_2D_INDEX + 1;
    else if(textureEnable->texture1D)
        textureEnable->enabledDimension = __GL_TEXTURE_1D_INDEX + 1;
    else
    {
        textureEnable->enabledDimension = 0;
        gc->texture.enabledMask &= ~(1 <<  unit);
    }

    if (origEnabledDim != textureEnable->enabledDimension) {
        __GL_SET_TEX_UNIT_BIT(gc, unit, __GL_TEX_ENABLE_DIM_CHANGED_BIT);
    }
}

#if (defined(_DEBUG) || defined(DEBUG))
/*
**  Sometimes we want to disable some states(especially depth test) on the fly.
*/
GLvoid __glDisableCertainStates(__GLcontext *gc)
{
    static GLuint disableMask = 0x0;

    if (disableMask)
    {
        if (disableMask & 0x1)  /* depth test */
        {
            gc->state.enables.depthBuffer.test = 0;
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_DEPTHTEST_ENDISABLE_BIT);
        }
        if (disableMask & 0x2)  /* Alpha test */
        {
            gc->state.enables.colorBuffer.alphaTest = 0;
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_ALPHATEST_ENDISABLE_BIT);
        }
        if (disableMask & 0x4)  /* Alpha test */
        {
            gc->state.enables.stencilTest = 0;
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_STENCILTEST_ENDISABLE_BIT);
        }
        if (disableMask & 0x8)  /* Alpha blend */
        {
            GLint i;
            for(i = 0; i < __GL_MAX_DRAW_BUFFERS; i++)
            {
                gc->state.enables.colorBuffer.blend[i] = 0;
            }
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_BLEND_ENDISABLE_BIT);
        }
        if (disableMask & 0x10) /* Scissor test */
        {
            gc->state.enables.scissor = 0;
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_SCISSORTEST_ENDISABLE_BIT);
        }
        if (disableMask & 0x20) /* fog */
        {
            gc->state.enables.fog = 0;
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_FOG_ENDISABLE_BIT);
        }
        if (disableMask & 0x40) /* lineStipple */
        {
            gc->state.enables.line.stipple = 0;
            gc->state.enables.line.stippleRequested = 0;
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_LINESTIPPLE_ENDISABLE_BIT | __GL_LINESTIPPLE_BIT);
        }
        if (disableMask & 0x80) /* lighting  */
        {
            gc->state.enables.lighting.lighting= 0;
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_LIGHTING_ATTRS, __GL_LIGHTING_ENDISABLE_BIT);
        }

        if (disableMask & 0x100) /* cullFace */
        {
            gc->state.enables.polygon.cullFace = 0;
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_CULLFACE_ENDISABLE_BIT);
        }


    }
}
#endif

__GL_INLINE GLvoid __glEnableDisable(__GLcontext *gc, GLenum cap, GLboolean val)
{
    __GLenableState *es = &gc->state.enables;
    GLint capOffset;
    GLuint unit;

    switch (cap) {
      case GL_ALPHA_TEST:
          if (es->colorBuffer.alphaTest ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->colorBuffer.alphaTest = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_ALPHATEST_ENDISABLE_BIT);
          }
          break;

      case GL_BLEND:
          {
              GLint i;
              for(i = 0; i < __GL_MAX_DRAW_BUFFERS; i++)
              {
                  if(es->colorBuffer.blend[i] ^ val)
                  {
                      break;
                  }
              }
              if(i < __GL_MAX_DRAW_BUFFERS)
              {
                  __GL_VERTEX_BUFFER_FLUSH(gc);

                  for(i = 0; i < __GL_MAX_DRAW_BUFFERS; i++)
                  {
                      es->colorBuffer.blend[i] = val;
                  }

                  __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_BLEND_ENDISABLE_BIT);
              }
          }
          break;

      case GL_COLOR_MATERIAL:
          if (es->lighting.colorMaterial ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);

              es->lighting.colorMaterial = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_LIGHTING_ATTRS, __GL_COLORMATERIAL_ENDISABLE_BIT);

              /* Also notify DP immediately to start tracking the Material changes */
              (*gc->dp.colorMaterialEndisable)(gc);

              __GL_INPUTMASK_CHANGED(gc);
          }
          break;

      case GL_CULL_FACE:
          if (es->polygon.cullFace ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->polygon.cullFace = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_CULLFACE_ENDISABLE_BIT);
          }
          break;

      case GL_DEPTH_TEST:
          if (es->depthBuffer.test ^ val) {
              __GL_DLIST_BUFFER_FLUSH(gc);
              es->depthBuffer.test = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1,  __GL_DEPTHTEST_ENDISABLE_BIT );
          }
          break;

      case GL_DITHER:
          if (es->colorBuffer.dither ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->colorBuffer.dither = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_DITHER_ENDISABLE_BIT);
          }
          break;

      case GL_FOG:
          if (es->fog ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->fog = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_FOG_ENDISABLE_BIT);

              __GL_INPUTMASK_CHANGED(gc);
          }
          break;

      case GL_LIGHTING:
          if (es->lighting.lighting ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->lighting.lighting = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_LIGHTING_ATTRS, __GL_LIGHTING_ENDISABLE_BIT);

              __GL_INPUTMASK_CHANGED(gc);
          }
          break;

      case GL_LINE_SMOOTH:
          if (es->line.smooth ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->line.smooth = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_LINESMOOTH_ENDISABLE_BIT);
          }
          break;

      case GL_LINE_STIPPLE:
          if (es->line.stippleRequested ^ val) {
              __GL_DLIST_BUFFER_FLUSH(gc);
              es->line.stippleRequested = val;
              es->line.stipple = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_LINESTIPPLE_ENDISABLE_BIT);
          }
          break;

      case GL_INDEX_LOGIC_OP:
          /* Index logicOp is handled for index mode only
          */
          if (es->colorBuffer.logicOp ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->colorBuffer.indexLogicOp = val;
              if(!gc->modes.rgbMode)
              {
                  es->colorBuffer.logicOp = val;
                  __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_LOGICOP_ENDISABLE_BIT);
              }
          }
          break;

      case GL_COLOR_LOGIC_OP:
          /* Color logicOp is handled for rgba mode only
          */
          if (es->colorBuffer.logicOp ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->colorBuffer.colorLogicOp = val;
              if(gc->modes.rgbMode)
              {
                  es->colorBuffer.logicOp = val;
                  __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_LOGICOP_ENDISABLE_BIT);
              }
          }
          break;

      case GL_NORMALIZE:
          if (es->transform.normalize ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->transform.normalize = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_NORMALIZE_ENDISABLE_BIT);
          }
          break;

      case GL_RESCALE_NORMAL:
          if (es->transform.rescaleNormal ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->transform.rescaleNormal = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_RESCALENORMAL_ENDISABLE_BIT);
          }
          break;

      case GL_POINT_SMOOTH:
          if (es->pointSmooth ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->pointSmooth = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POINTSMOOTH_ENDISABLE_BIT);
          }
          break;

      case GL_POLYGON_SMOOTH:
          if (es->polygon.smooth ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->polygon.smooth = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POLYGONSMOOTH_ENDISABLE_BIT);
          }
          break;

      case GL_POLYGON_STIPPLE:
          if (es->polygon.stipple ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->polygon.stipple = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POLYGONSTIPPLE_ENDISABLE_BIT);
          }
          break;

      case GL_SCISSOR_TEST:
          if (es->scissor ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->scissor = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_SCISSORTEST_ENDISABLE_BIT);

            /* Re-compute the core clip rectangle */
            __glComputeClipBox(gc);
          }
          break;

      case GL_STENCIL_TEST:
          if (es->stencilTest ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->stencilTest = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_STENCILTEST_ENDISABLE_BIT);
          }
          break;

      case GL_TEXTURE_1D:
          if (gc->modes.rgbMode) {
              unit = gc->state.texture.activeTexIndex;
              if (es->texUnits[unit].texture1D ^ val) {
                  __GL_VERTEX_BUFFER_FLUSH(gc);
                  es->texUnits[unit].texture1D = val;

                  __glSetTexEnableDimension(gc, unit);
                  __GL_INPUTMASK_CHANGED(gc);
              }
          }
          break;

      case GL_TEXTURE_2D:
          if (gc->modes.rgbMode) {
              unit = gc->state.texture.activeTexIndex;
              if (es->texUnits[unit].texture2D ^ val) {
                  __GL_VERTEX_BUFFER_FLUSH(gc);
                  es->texUnits[unit].texture2D = val;

                  __glSetTexEnableDimension(gc, unit);
                  __GL_INPUTMASK_CHANGED(gc);
              }
          }
          break;

      case GL_TEXTURE_3D:
          if (gc->modes.rgbMode) {
              unit = gc->state.texture.activeTexIndex;
              if (es->texUnits[unit].texture3D ^ val) {
                  __GL_VERTEX_BUFFER_FLUSH(gc);
                  es->texUnits[unit].texture3D = val;

                  __glSetTexEnableDimension(gc, unit);
                  __GL_INPUTMASK_CHANGED(gc);
              }
          }
          break;

      case GL_TEXTURE_CUBE_MAP:
          if (gc->modes.rgbMode) {
              unit = gc->state.texture.activeTexIndex;
              if (es->texUnits[unit].textureCubeMap ^ val) {
                  __GL_VERTEX_BUFFER_FLUSH(gc);
                  es->texUnits[unit].textureCubeMap = val;

                  __glSetTexEnableDimension(gc, unit);
                  __GL_INPUTMASK_CHANGED(gc);
              }
          }
          break;

#if GL_ARB_texture_rectangle
      case GL_TEXTURE_RECTANGLE_ARB:
          if (gc->modes.rgbMode) {
              unit = gc->state.texture.activeTexIndex;
              if (es->texUnits[unit].textureRec ^ val) {
                  __GL_VERTEX_BUFFER_FLUSH(gc);
                  es->texUnits[unit].textureRec = val;

                  __glSetTexEnableDimension(gc, unit);
                  __GL_INPUTMASK_CHANGED(gc);
              }
          }
          break;
#endif
      case GL_CLIP_PLANE0:
      case GL_CLIP_PLANE1:
      case GL_CLIP_PLANE2:
      case GL_CLIP_PLANE3:
      case GL_CLIP_PLANE4:
      case GL_CLIP_PLANE5:
          capOffset = cap - GL_CLIP_PLANE0;
          __GL_VERTEX_BUFFER_FLUSH(gc);
          if (val == GL_TRUE) {
              es->transform.clipPlanesMask |= (1 << capOffset);
          }
          else {
              es->transform.clipPlanesMask &= ~(1 << capOffset);
          }

          /* The upper 16 bits of globalDirtyState[__GL_CLIP_ATTRS] are for
          ** clipPlane enable/disable.
          */
          gc->globalDirtyState[__GL_CLIP_ATTRS] |= (1 << (capOffset + 16));
          gc->globalDirtyState[__GL_ALL_ATTRS] |= (1 << __GL_CLIP_ATTRS);
          break;

      case GL_LIGHT0:
      case GL_LIGHT1:
      case GL_LIGHT2:
      case GL_LIGHT3:
      case GL_LIGHT4:
      case GL_LIGHT5:
      case GL_LIGHT6:
      case GL_LIGHT7:
          capOffset = cap - GL_LIGHT0;
          if (es->lighting.light[capOffset] ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->lighting.light[capOffset] = val;
              __GL_SET_LIGHT_SRC_BIT(gc, capOffset, __GL_LIGHT_ENDISABLE_BIT);
          }
          break;

      case GL_MAP1_COLOR_4:
      case GL_MAP1_NORMAL:
      case GL_MAP1_INDEX:
      case GL_MAP1_TEXTURE_COORD_1:
      case GL_MAP1_TEXTURE_COORD_2:
      case GL_MAP1_TEXTURE_COORD_3:
      case GL_MAP1_TEXTURE_COORD_4:
      case GL_MAP1_VERTEX_3:
      case GL_MAP1_VERTEX_4:
          cap = __GL_EVAL1D_INDEX(cap);
          if (es->eval.map1[cap] ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->eval.map1[cap] = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_MAP1_ENDISABLE_BIT);
          }
          break;

      case GL_MAP2_COLOR_4:
      case GL_MAP2_NORMAL:
      case GL_MAP2_INDEX:
      case GL_MAP2_TEXTURE_COORD_1:
      case GL_MAP2_TEXTURE_COORD_2:
      case GL_MAP2_TEXTURE_COORD_3:
      case GL_MAP2_TEXTURE_COORD_4:
      case GL_MAP2_VERTEX_3:
      case GL_MAP2_VERTEX_4:
          cap = __GL_EVAL2D_INDEX(cap);
          if (es->eval.map2[cap] ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->eval.map2[cap] = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_MAP2_ENDISABLE_BIT);
          }
          break;

      case GL_AUTO_NORMAL:
          if (es->eval.autonormal ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->eval.autonormal = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_AUTONORMAL_ENDISABLE_BIT);
          }
          break;

      case GL_TEXTURE_GEN_S:
          unit = gc->state.texture.activeTexIndex;
          if (es->texUnits[unit].texGen[0] ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->texUnits[unit].texGen[0] = val;
              __GL_SET_TEX_UNIT_BIT(gc, unit, __GL_TEXGEN_S_ENDISABLE_BIT);

              __GL_INPUTMASK_CHANGED(gc);
          }
          break;

      case GL_TEXTURE_GEN_T:
          unit = gc->state.texture.activeTexIndex;
          if (es->texUnits[unit].texGen[1] ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->texUnits[unit].texGen[1] = val;
              __GL_SET_TEX_UNIT_BIT(gc, unit, __GL_TEXGEN_T_ENDISABLE_BIT);

              __GL_INPUTMASK_CHANGED(gc);
          }
          break;

      case GL_TEXTURE_GEN_R:
          unit = gc->state.texture.activeTexIndex;
          if (es->texUnits[unit].texGen[2] ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->texUnits[unit].texGen[2] = val;
              __GL_SET_TEX_UNIT_BIT(gc, unit, __GL_TEXGEN_R_ENDISABLE_BIT);

              __GL_INPUTMASK_CHANGED(gc);
          }
          break;

      case GL_TEXTURE_GEN_Q:
          unit = gc->state.texture.activeTexIndex;
          if (es->texUnits[unit].texGen[3] ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->texUnits[unit].texGen[3] = val;
              __GL_SET_TEX_UNIT_BIT(gc, unit, __GL_TEXGEN_Q_ENDISABLE_BIT);
          }
          break;

      case GL_POLYGON_OFFSET_POINT:
          if (es->polygon.polygonOffsetPoint ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->polygon.polygonOffsetPoint = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POLYGONOFFSET_POINT_ENDISABLE_BIT);
          }
          break;

      case GL_POLYGON_OFFSET_LINE:
          if (es->polygon.polygonOffsetLine ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->polygon.polygonOffsetLine = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POLYGONOFFSET_LINE_ENDISABLE_BIT);
          }
          break;

      case GL_POLYGON_OFFSET_FILL:
          if (es->polygon.polygonOffsetFill ^ val) {
              __GL_DLIST_BUFFER_FLUSH(gc);
              es->polygon.polygonOffsetFill = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POLYGONOFFSET_FILL_ENDISABLE_BIT);
          }
          break;

      case GL_MULTISAMPLE:
          if (es->multisample.multisampleOn ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->multisample.multisampleOn = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_MULTISAMPLE_ENDISABLE_BIT |
                  __GL_SAMPLE_ALPHA_TO_COVERAGE_ENDISABLE_BIT |
                  __GL_SAMPLE_ALPHA_TO_ONE_ENDISABLE_BIT |
                  __GL_SAMPLE_COVERAGE_ENDISABLE_BIT);
          }
          break;

      case GL_SAMPLE_ALPHA_TO_COVERAGE:
          if (es->multisample.alphaToCoverage ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->multisample.alphaToCoverage = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_SAMPLE_ALPHA_TO_COVERAGE_ENDISABLE_BIT);
          }
          break;

      case GL_SAMPLE_ALPHA_TO_ONE:
          if (es->multisample.alphaToOne ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->multisample.alphaToOne = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_SAMPLE_ALPHA_TO_ONE_ENDISABLE_BIT);
          }
          break;

      case GL_SAMPLE_COVERAGE:
          if (es->multisample.coverage ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->multisample.coverage = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_SAMPLE_COVERAGE_ENDISABLE_BIT);
          }
          break;

      case GL_COLOR_SUM:
          if (es->colorSum ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->colorSum = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_COLORSUM_ENDISABLE_BIT);

              __GL_INPUTMASK_CHANGED(gc);
          }
          break;

#if GL_EXT_vertex_array
          /* Legacy (extension) Vertex Array support */

      case GL_VERTEX_ARRAY_EXT:
          if (val) {
              __glim_EnableClientState(GL_VERTEX_ARRAY);
          } else {
              __glim_DisableClientState(GL_VERTEX_ARRAY);
          }
          return;
      case GL_NORMAL_ARRAY_EXT:
          if (val) {
              __glim_EnableClientState(GL_NORMAL_ARRAY);
          } else {
              __glim_DisableClientState(GL_NORMAL_ARRAY);
          }
          return;
      case GL_COLOR_ARRAY_EXT:
          if (val) {
              __glim_EnableClientState(GL_COLOR_ARRAY);
          } else {
              __glim_DisableClientState(GL_COLOR_ARRAY);
          }
          return;
      case GL_INDEX_ARRAY_EXT:
          if (val) {
              __glim_EnableClientState(GL_INDEX_ARRAY);
          } else {
              __glim_DisableClientState(GL_INDEX_ARRAY);
          }
          return;
      case GL_TEXTURE_COORD_ARRAY_EXT:
          if (val) {
              __glim_EnableClientState(GL_TEXTURE_COORD_ARRAY);
          } else {
              __glim_DisableClientState(GL_TEXTURE_COORD_ARRAY);
          }
          return;
      case GL_EDGE_FLAG_ARRAY_EXT:
          if (val) {
              __glim_EnableClientState(GL_EDGE_FLAG_ARRAY);
          } else {
              __glim_DisableClientState(GL_EDGE_FLAG_ARRAY);
          }
          return;
#endif

#if GL_ARB_vertex_program
      case GL_VERTEX_PROGRAM_ARB:
          /* If app enables the vertex program, we should modify some pointers.
          */
          if (es->program.vertexProgram ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->program.vertexProgram = val;

              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_VERTEX_PROGRAM_ENABLE);
              __GL_INPUTMASK_CHANGED(gc);
          }
          break;
      case GL_VERTEX_PROGRAM_POINT_SIZE_ARB:
          if (es->program.vpPointSize ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->program.vpPointSize = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_VP_POINT_SIZE_ENABLE);
          }
          break;
      case GL_VERTEX_PROGRAM_TWO_SIDE_ARB:
          if (es->program.vpTwoSize ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->program.vpTwoSize = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_VP_TWO_SIDE_ENABLE);
          }
          break;
#endif

#if GL_ARB_fragment_program
      case GL_FRAGMENT_PROGRAM_ARB:
          if (es->program.fragmentProgram ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->program.fragmentProgram = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_FRAGMENT_PROGRAM_ENABLE);
          }
          break;
#endif

#if GL_ARB_imaging
      case GL_COLOR_TABLE:
          if (es->colorTable ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->colorTable = val;

              (*gc->dp.colorTableEndisable)(gc);

              __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_COLORTABLE_BIT);
          }
          break;

      case GL_POST_CONVOLUTION_COLOR_TABLE:
          if (es->postConvColorTable ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->postConvColorTable = val;

              (*gc->dp.postConvColorTableEndisable)(gc);

              __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_POST_CONV_COLORTABLE_BIT);
          }
          break;

      case GL_POST_COLOR_MATRIX_COLOR_TABLE:
          if (es->postColorMatrixColorTable ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);

              es->postColorMatrixColorTable = val;

              (*gc->dp.postColorMatrixColorTableEndisable)(gc);

              __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_POST_MATRIX_COLORTABLE_BIT);
          }
          break;

      case GL_CONVOLUTION_1D:
          if (es->convolution.convolution1D ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->convolution.convolution1D = val;

              (*gc->dp.convolution1DEndisable)(gc);

              __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_CONVOLUTION_1D_BIT);
          }
          break;

      case GL_CONVOLUTION_2D:
          if (es->convolution.convolution2D ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->convolution.convolution2D = val;

              (*gc->dp.convolution2DEndisable)(gc);

              __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_CONVOLUTION_2D_BIT);
          }
          break;

      case GL_SEPARABLE_2D:
          if (es->convolution.separable2D ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->convolution.separable2D = val;

              (*gc->dp.separable2DEndisable)(gc);

              __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_SEPARABLE_2D_BIT);
          }
          break;

      case GL_HISTOGRAM:
          if (es->histogram ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->histogram = val;

              (*gc->dp.histogramEndisable)(gc);

              __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_HISTOGRAM_BIT);
          }
          break;

      case GL_MINMAX:
          if (es->histogram ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->minmax = val;

              (*gc->dp.minmaxEndisable)(gc);

              __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_MINMAX_BIT);
          }
          break;
#endif

#if GL_EXT_stencil_two_side
      case GL_STENCIL_TEST_TWO_SIDE_EXT:
          if (es->stencilTestTwoSideExt ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->stencilTestTwoSideExt = val;

              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_STENCIL_ATTR_BITS);
          }
          break;
#endif

#if GL_EXT_depth_bounds_test
      case GL_DEPTH_BOUNDS_TEST_EXT:
          if(es->depthBoundTest ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->depthBoundTest = val;

              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_DEPTHBOUNDTESTENABLE_BIT);
          }
          break;
#endif

      case GL_POINT_SPRITE:
          if(es->pointSprite ^ val) {
              __GL_VERTEX_BUFFER_FLUSH(gc);
              es->pointSprite = val;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POINTSPRITE_ENDISABLE_BIT);
          }
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }

#if (defined(_DEBUG) || defined(DEBUG))
    __glDisableCertainStates(gc);
#endif

}


GLvoid APIENTRY __glim_Enable(GLenum cap)
{
    __GL_SETUP();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Enable", DT_GLenum, cap, DT_GLnull);
#endif

    __glEnableDisable(gc, cap, GL_TRUE);
}

GLvoid APIENTRY __glim_Disable(GLenum cap)
{
    __GL_SETUP();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Disable", DT_GLenum, cap, DT_GLnull);
#endif

    __glEnableDisable(gc, cap, GL_FALSE);
}


GLboolean APIENTRY __glim_IsEnabled(GLenum cap)
{
    __GL_SETUP();
    __GLenableState *es = &gc->state.enables;
    GLint capOffset;

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_IsEnabled", DT_GLenum, cap, DT_GLnull);
#endif

    __GL_VERTEX_BUFFER_FLUSH(gc);

    switch (cap) {
      case GL_ALPHA_TEST:
          return es->colorBuffer.alphaTest;

      case GL_BLEND:
          return es->colorBuffer.blend[0];
          break;

      case GL_COLOR_MATERIAL:
          return es->lighting.colorMaterial;

      case GL_CULL_FACE:
          return es->polygon.cullFace;

      case GL_DEPTH_TEST:
          return es->depthBuffer.test;

      case GL_DITHER:
          return es->colorBuffer.dither;

      case GL_FOG:
          return es->fog;

      case GL_LIGHTING:
          return es->lighting.lighting;

      case GL_LINE_SMOOTH:
          return es->line.smooth;

      case GL_LINE_STIPPLE:
          return es->line.stippleRequested;

      case GL_INDEX_LOGIC_OP:
          return es->colorBuffer.indexLogicOp;

      case GL_COLOR_LOGIC_OP:
          return es->colorBuffer.colorLogicOp;

      case GL_NORMALIZE:
          return es->transform.normalize;

      case GL_RESCALE_NORMAL:
          return es->transform.rescaleNormal;

      case GL_POINT_SMOOTH:
          return es->pointSmooth;

      case GL_POLYGON_SMOOTH:
          return es->polygon.smooth;

      case GL_POLYGON_STIPPLE:
          return es->polygon.stipple;

      case GL_SCISSOR_TEST:
          return es->scissor;

      case GL_STENCIL_TEST:
          return es->stencilTest;

      case GL_TEXTURE_1D:
          return es->texUnits[gc->state.texture.activeTexIndex].texture1D;

      case GL_TEXTURE_2D:
          return es->texUnits[gc->state.texture.activeTexIndex].texture2D;
          break;

#if GL_ARB_texture_rectangle
      case GL_TEXTURE_RECTANGLE_ARB:
      return es->texUnits[gc->state.texture.activeTexIndex].textureRec;
      break;
#endif

      case GL_TEXTURE_3D:
          return es->texUnits[gc->state.texture.activeTexIndex].texture3D;

      case GL_TEXTURE_CUBE_MAP:
          return es->texUnits[gc->state.texture.activeTexIndex].textureCubeMap;

      case GL_AUTO_NORMAL:
          return es->eval.autonormal;

      case GL_CLIP_PLANE0:
      case GL_CLIP_PLANE1:
      case GL_CLIP_PLANE2:
      case GL_CLIP_PLANE3:
      case GL_CLIP_PLANE4:
      case GL_CLIP_PLANE5:
          capOffset = cap - GL_CLIP_PLANE0;
          return (es->transform.clipPlanesMask & (1 << capOffset)) != 0;

      case GL_LIGHT0:
      case GL_LIGHT1:
      case GL_LIGHT2:
      case GL_LIGHT3:
      case GL_LIGHT4:
      case GL_LIGHT5:
      case GL_LIGHT6:
      case GL_LIGHT7:
          capOffset = cap - GL_LIGHT0;
          return es->lighting.light[capOffset];

      case GL_MAP1_COLOR_4:
      case GL_MAP1_NORMAL:
      case GL_MAP1_INDEX:
      case GL_MAP1_TEXTURE_COORD_1:
      case GL_MAP1_TEXTURE_COORD_2:
      case GL_MAP1_TEXTURE_COORD_3:
      case GL_MAP1_TEXTURE_COORD_4:
      case GL_MAP1_VERTEX_3:
      case GL_MAP1_VERTEX_4:
          cap = __GL_EVAL1D_INDEX(cap);
          return es->eval.map1[cap];

      case GL_MAP2_COLOR_4:
      case GL_MAP2_NORMAL:
      case GL_MAP2_INDEX:
      case GL_MAP2_TEXTURE_COORD_1:
      case GL_MAP2_TEXTURE_COORD_2:
      case GL_MAP2_TEXTURE_COORD_3:
      case GL_MAP2_TEXTURE_COORD_4:
      case GL_MAP2_VERTEX_3:
      case GL_MAP2_VERTEX_4:
          cap = __GL_EVAL2D_INDEX(cap);
          return es->eval.map2[cap];

      case GL_TEXTURE_GEN_S:
      case GL_TEXTURE_GEN_T:
      case GL_TEXTURE_GEN_R:
      case GL_TEXTURE_GEN_Q:
          cap -= GL_TEXTURE_GEN_S;
          return es->texUnits[gc->state.texture.activeTexIndex].texGen[cap];

      case GL_VERTEX_ARRAY:
          return (gc->clientState.vertexArray.arrayEnabled & __GL_VARRAY_VERTEX) != 0;

      case GL_NORMAL_ARRAY:
          return (gc->clientState.vertexArray.arrayEnabled & __GL_VARRAY_NORMAL) != 0;

      case GL_COLOR_ARRAY:
          return (gc->clientState.vertexArray.arrayEnabled & __GL_VARRAY_DIFFUSE) != 0;

      case GL_SECONDARY_COLOR_ARRAY:
          return (gc->clientState.vertexArray.arrayEnabled & __GL_VARRAY_SPECULAR) != 0;

      case GL_FOG_COORD_ARRAY:
          return (gc->clientState.vertexArray.arrayEnabled & __GL_VARRAY_FOGCOORD) != 0;

      case GL_INDEX_ARRAY:
          return (gc->clientState.vertexArray.arrayEnabled & __GL_VARRAY_COLORINDEX) != 0;

      case GL_TEXTURE_COORD_ARRAY:
          capOffset = gc->clientState.vertexArray.clientActiveUnit;
          return (gc->clientState.vertexArray.arrayEnabled & (__GL_VARRAY_TEX0<<capOffset)) != 0;

      case GL_EDGE_FLAG_ARRAY:
          return (gc->clientState.vertexArray.arrayEnabled & __GL_VARRAY_EDGEFLAG) != 0;

      case GL_WEIGHT_ARRAY_ARB:
          return (gc->clientState.vertexArray.arrayEnabled & __GL_VARRAY_WEIGHT) != 0;

#if GL_ATI_element_array
      case GL_ELEMENT_ARRAY_ATI:
          return gc->clientState.vertexArray.elementArrayATI;
#endif

      case GL_POLYGON_OFFSET_POINT:
          return es->polygon.polygonOffsetPoint;

      case GL_POLYGON_OFFSET_LINE:
          return es->polygon.polygonOffsetLine;

      case GL_POLYGON_OFFSET_FILL:
          return es->polygon.polygonOffsetFill;

      case GL_MULTISAMPLE:
          return es->multisample.multisampleOn;

      case GL_SAMPLE_ALPHA_TO_COVERAGE:
          return es->multisample.alphaToCoverage;

      case GL_SAMPLE_ALPHA_TO_ONE:
          return es->multisample.alphaToOne;

      case GL_SAMPLE_COVERAGE:
          return es->multisample.coverage;

      case GL_COLOR_SUM:
          return es->colorSum;

#if GL_ARB_vertex_program
      case GL_VERTEX_PROGRAM_ARB:
          return es->program.vertexProgram;
      case GL_VERTEX_PROGRAM_POINT_SIZE_ARB:
          return es->program.vpPointSize;
      case GL_VERTEX_PROGRAM_TWO_SIDE_ARB:
          return es->program.vpTwoSize;
#endif

#if GL_ARB_fragment_program
      case GL_FRAGMENT_PROGRAM_ARB:
          return es->program.fragmentProgram;
#endif

#if GL_ARB_imaging
      case GL_COLOR_TABLE:
          return es->colorTable;

      case GL_CONVOLUTION_1D:
          return es->convolution.convolution1D;

      case GL_CONVOLUTION_2D:
          return es->convolution.convolution2D;

      case GL_SEPARABLE_2D:
          return es->convolution.separable2D;

      case GL_POST_CONVOLUTION_COLOR_TABLE:
          return es->postConvColorTable;

      case GL_POST_COLOR_MATRIX_COLOR_TABLE:
          return es->postColorMatrixColorTable;

      case GL_HISTOGRAM:
          return es->histogram;

      case GL_MINMAX:
          return es->minmax;
#endif

#if GL_EXT_depth_bounds_test
      case GL_DEPTH_BOUNDS_TEST_EXT:
          return es->depthBoundTest;
#endif

      case GL_POINT_SPRITE:
          return es->pointSprite;

      default:
          __glSetError(GL_INVALID_ENUM);
          return GL_FALSE;
    }
}

GLvoid APIENTRY __glim_EnableClientState(GLenum array)
{
    GLuint arrayIdx;
    GLbitfield bit;
    __GLvertexArrayState * pVertexArrayState;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_EnableClientState", DT_GLenum, array, DT_GLnull);
#endif

    pVertexArrayState = &gc->clientState.vertexArray;

    /* Note: Do not add __GL_VERTEX_BUFFER_FLUSH(gc) in this function */

    switch (array)
    {
    case GL_VERTEX_ARRAY:
        arrayIdx = __GL_VARRAY_VERTEX_INDEX;
        break;

    case GL_WEIGHT_ARRAY_ARB:
        arrayIdx = __GL_VARRAY_WEIGHT_INDEX;
        break;

    case GL_NORMAL_ARRAY:
        arrayIdx = __GL_VARRAY_NORMAL_INDEX;
        break;

    case GL_COLOR_ARRAY:
        arrayIdx = __GL_VARRAY_DIFFUSE_INDEX;
        break;

    case GL_SECONDARY_COLOR_ARRAY:
        arrayIdx = __GL_VARRAY_SPECULAR_INDEX;
        break;

    case GL_FOG_COORDINATE_ARRAY:
        arrayIdx = __GL_VARRAY_FOGCOORD_INDEX;
        break;

    case GL_EDGE_FLAG_ARRAY:
        arrayIdx = __GL_VARRAY_EDGEFLAG_INDEX;
        break;

    case GL_INDEX_ARRAY:
        arrayIdx = __GL_VARRAY_COLORINDEX_INDEX;
        break;

    case GL_TEXTURE_COORD_ARRAY:
        arrayIdx = __GL_VARRAY_TEX0_INDEX + pVertexArrayState->clientActiveUnit;
        break;

#if GL_ATI_element_array
    case GL_ELEMENT_ARRAY_ATI:
        /* It does not affact dirty bit, just return for this case */
        pVertexArrayState->elementArrayATI = GL_TRUE;
        return;
#endif

    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    bit = __GL_VARRAY_VERTEX << arrayIdx;
    if ((pVertexArrayState->arrayEnabled & bit) == 0)
    {
        pVertexArrayState->arrayEnabled |= bit;
        __GL_VALIDATE_VERTEX_ARRAYS(gc);
    }
}

GLvoid APIENTRY __glim_DisableClientState(GLenum array)
{
    GLuint arrayIdx;
    GLbitfield bit;
    __GLvertexArrayState * pVertexArrayState;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DisableClientState", DT_GLenum, array, DT_GLnull);
#endif

    pVertexArrayState = &gc->clientState.vertexArray;

    /* Note: Do not add __GL_VERTEX_BUFFER_FLUSH(gc) in this function */

    switch (array)
    {
    case GL_VERTEX_ARRAY:
        arrayIdx = __GL_VARRAY_VERTEX_INDEX;
        break;

    case GL_WEIGHT_ARRAY_ARB:
        arrayIdx = __GL_VARRAY_WEIGHT_INDEX;
        break;

    case GL_NORMAL_ARRAY:
        arrayIdx = __GL_VARRAY_NORMAL_INDEX;
        break;

    case GL_COLOR_ARRAY:
        arrayIdx = __GL_VARRAY_DIFFUSE_INDEX;
        break;

    case GL_SECONDARY_COLOR_ARRAY:
        arrayIdx = __GL_VARRAY_SPECULAR_INDEX;
        break;

    case GL_FOG_COORDINATE_ARRAY:
        arrayIdx = __GL_VARRAY_FOGCOORD_INDEX;
        break;

    case GL_EDGE_FLAG_ARRAY:
        arrayIdx = __GL_VARRAY_EDGEFLAG_INDEX;
        break;

    case GL_INDEX_ARRAY:
        arrayIdx = __GL_VARRAY_COLORINDEX_INDEX;
        break;

    case GL_TEXTURE_COORD_ARRAY:
        arrayIdx = __GL_VARRAY_TEX0_INDEX + pVertexArrayState->clientActiveUnit;
        break;

#if GL_ATI_element_array
    case GL_ELEMENT_ARRAY_ATI:
        pVertexArrayState->elementArrayATI = GL_FALSE;
        return;
#endif

    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    bit = __GL_VARRAY_VERTEX << arrayIdx;
    if (pVertexArrayState->arrayEnabled & bit)
    {
        pVertexArrayState->arrayEnabled &= ~bit;
        __GL_VALIDATE_VERTEX_ARRAYS(gc);
    }
}

GLvoid APIENTRY __glim_EnableVertexAttribArray(GLuint index)
{
    GLbitfield bit = __GL_VARRAY_ATT0 << index;
    __GLvertexArrayState * pVertexArrayState;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_EnableVertexAttribArray", DT_GLuint, index, DT_GLnull);
#endif

    /* Note: Do not add __GL_VERTEX_BUFFER_FLUSH(gc) in this function */

    if(index >= __GL_MAX_PROGRAM_VERTEX_ATTRIBUTES )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    pVertexArrayState = &gc->clientState.vertexArray;
    if ((pVertexArrayState->arrayEnabled & bit) == 0)
    {
        pVertexArrayState->arrayEnabled |= bit;
        __GL_VALIDATE_VERTEX_ARRAYS(gc);
    }
}

GLvoid APIENTRY __glim_DisableVertexAttribArray(GLuint index)
{
    GLbitfield bit = __GL_VARRAY_ATT0 << index;
    __GLvertexArrayState * pVertexArrayState;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DisableVertexAttribArray", DT_GLuint, index, DT_GLnull);
#endif

    /* Note: Do not add __GL_VERTEX_BUFFER_FLUSH(gc) in this function */

    if(index >= __GL_MAX_PROGRAM_VERTEX_ATTRIBUTES )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    pVertexArrayState = &gc->clientState.vertexArray;
    if (pVertexArrayState->arrayEnabled & bit)
    {
        pVertexArrayState->arrayEnabled &= ~bit;
        __GL_VALIDATE_VERTEX_ARRAYS(gc);
    }
}

#if GL_EXT_draw_buffers2

__GL_INLINE GLvoid __glEnableDisableIndexedEXT(__GLcontext* gc, GLenum target, GLuint index, GLboolean val)
{
    __GLenableState *es;

    es = &gc->state.enables;

    if(index >= __GL_MAX_DRAW_BUFFERS)
    {
        __glSetError(GL_INVALID_ENUM);

        return;
    }

    switch(target)
    {
    case GL_BLEND:
        {
            if(es->colorBuffer.blend[index] ^ val)
            {
                __GL_VERTEX_BUFFER_FLUSH(gc);

                es->colorBuffer.blend[index] = val;

                __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_BLEND_ENDISABLE_BIT);
            }
        }
        break;
    default:
        __glEnableDisable(gc, target, val);
    }
}

GLvoid APIENTRY __glim_EnableIndexedEXT(GLenum target, GLuint index)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_EnableIndexedEXT", DT_GLenum, target, DT_GLuint, index);
#endif

    __glEnableDisableIndexedEXT(gc, target, index, GL_TRUE);
}

GLvoid APIENTRY __glim_DisableIndexedEXT(GLenum target, GLuint index)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DisableIndexedEXT", DT_GLenum, target, DT_GLuint, index);
#endif

    __glEnableDisableIndexedEXT(gc, target, index, GL_FALSE);
}

GLboolean APIENTRY __glim_IsEnabledIndexedEXT(GLenum target, GLuint index)
{
    __GLenableState *es;

    __GL_SETUP_NOT_IN_BEGIN_RET(GL_FALSE);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_IsEnabledIndexedEXT", DT_GLenum, target, DT_GLuint, index);
#endif

    GL_ASSERT(index < __GL_MAX_DRAW_BUFFERS);

    es = &gc->state.enables;

    switch(target)
    {
    case GL_BLEND:
        return es->colorBuffer.blend[index];
        break;
    default:
        return __glim_IsEnabled(target);
    }
}
#endif
