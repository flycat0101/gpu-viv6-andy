/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_es_context.h"

extern GLvoid __glSetTexEnableDimension(__GLcontext *gc, GLuint unit);
extern GLvoid __glUpdateProgramEnableDimension(__GLcontext * gc);

GLvoid APIENTRY __glim_PushAttrib(__GLcontext *gc, GLuint mask)
{
    __GLattributeStack **spp;
    __GLattributeStack *sp;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    spp = gc->attribute.stackPointer;
    if (spp < &gc->attribute.stack[gc->constants.maxAttribStackDepth])
    {
        sp = *spp;
        if (!sp)
        {
            sp = (__GLattributeStack*)(*gc->imports.calloc)(gc, 1, sizeof(__GLattributeStack));
            *spp = sp;
        }
        gc->attribute.stackPointer = spp + 1;

        sp->mask = mask;

        /* Always save enables and deferred attribute masks.
        */
        sp->state.enables = gc->state.enables;

        if (mask & GL_ACCUM_BUFFER_BIT)
        {
            sp->state.accum = gc->state.accum;
        }
        if (mask & GL_COLOR_BUFFER_BIT)
        {
            sp->state.raster = gc->state.raster;
#if GL_EXT_framebuffer_object
            /* Per spec.: Save the drawbuffer state of current FBO */
            if(DRAW_FRAMEBUFFER_BINDING_NAME != 0)
            {
                sp->state.raster.drawBuffers[0] = gc->frameBuffer.drawFramebufObj->drawBuffers[0];
            }
#endif
        }
        if (mask & GL_CURRENT_BIT)
        {
            sp->state.current = gc->state.current;
            sp->state.rasterPos = gc->state.rasterPos;
        }
        if (mask & GL_DEPTH_BUFFER_BIT)
        {
            sp->state.depth = gc->state.depth;
#if GL_EXT_depth_bounds_test
            sp->state.depthBoundTest = gc->state.depthBoundTest;
#endif
        }
        if (mask & GL_EVAL_BIT)
        {
            sp->state.evaluator = gc->state.evaluator;
        }
        if (mask & GL_FOG_BIT)
        {
            sp->state.fog = gc->state.fog;
        }
        if (mask & GL_HINT_BIT)
        {
            sp->state.hints = gc->state.hints;
        }
        if (mask & GL_LIGHTING_BIT)
        {
            sp->state.light = gc->state.light;
        }
        if (mask & GL_LINE_BIT)
        {
            sp->state.line = gc->state.line;
        }
        if (mask & GL_LIST_BIT)
        {
            sp->state.list = gc->state.list;
        }
        if (mask & GL_PIXEL_MODE_BIT)
        {
            sp->state.pixel.readBuffer = gc->state.pixel.readBuffer;
            sp->state.pixel.readBufferReturn = gc->state.pixel.readBufferReturn;
            sp->state.pixel.transferMode = gc->state.pixel.transferMode;
            sp->state.pixel.colorTable[__GL_COLOR_TABLE_INDEX].state =
                gc->state.pixel.colorTable[__GL_COLOR_TABLE_INDEX].state;
            sp->state.pixel.colorTable[__GL_POST_CONVOLUTION_COLOR_TABLE_INDEX].state =
                gc->state.pixel.colorTable[__GL_POST_CONVOLUTION_COLOR_TABLE_INDEX].state;
            sp->state.pixel.colorTable[__GL_POST_COLOR_MATRIX_COLOR_TABLE_INDEX].state =
                gc->state.pixel.colorTable[__GL_POST_COLOR_MATRIX_COLOR_TABLE_INDEX].state;
            sp->state.pixel.convolutionFilter[__GL_CONVOLUTION_1D_INDEX].state =
                gc->state.pixel.convolutionFilter[__GL_CONVOLUTION_1D_INDEX].state;
            sp->state.pixel.convolutionFilter[__GL_CONVOLUTION_2D_INDEX].state =
                gc->state.pixel.convolutionFilter[__GL_CONVOLUTION_2D_INDEX].state;
            sp->state.pixel.convolutionFilter[__GL_SEPARABLE_2D_INDEX].state =
                gc->state.pixel.convolutionFilter[__GL_SEPARABLE_2D_INDEX].state;
        }
        if (mask & GL_POINT_BIT)
        {
            sp->state.point = gc->state.point;
        }
        if (mask & GL_POLYGON_BIT)
        {
            sp->state.polygon = gc->state.polygon;
        }
        if (mask & GL_POLYGON_STIPPLE_BIT)
        {
            sp->state.polygonStipple = gc->state.polygonStipple;
        }
        if (mask & GL_SCISSOR_BIT)
        {
            sp->state.scissor = gc->state.scissor;
        }
        if (mask & GL_STENCIL_BUFFER_BIT)
        {
            sp->state.stencil = gc->state.stencil;
        }
        if (mask & GL_TEXTURE_BIT)
        {
            sp->state.texture = gc->state.texture;
        }
        if (mask & GL_TRANSFORM_BIT)
        {
            sp->state.transform = gc->state.transform;
        }
        if (mask & GL_VIEWPORT_BIT)
        {
            sp->state.viewport = gc->state.viewport;
        }
        if (mask & GL_MULTISAMPLE_BIT)
        {
            sp->state.multisample = gc->state.multisample;
        }
        if (mask & GL_ENABLE_BIT)
        {
            sp->state.enables = gc->state.enables;
        }
    }
    else
    {
        __glSetError(gc, GL_STACK_OVERFLOW);
        return;
    }
}


GLvoid APIENTRY __glim_PopAttrib(__GLcontext *gc)
{
    __GLattributeStack **spp;
    __GLattributeStack *sp;
    GLuint mask, attribMask, i;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    spp = gc->attribute.stackPointer;
    if (spp > &gc->attribute.stack[0]) {
        --spp;
        sp = *spp;
        GL_ASSERT(sp != 0);

        mask = sp->mask;
        gc->attribute.stackPointer = spp;

        if (mask & GL_ACCUM_BUFFER_BIT) {
            gc->state.accum = sp->state.accum;

            /* Delayed notify attribute changes to DP.
            */
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_CLEARACCUM_BIT);
        }

        if (mask & GL_COLOR_BUFFER_BIT) {

#if GL_EXT_framebuffer_object
            /* Per spec.: Restore  the drawbuffer state of current FBO. gc's drawbuffer state not changed. */
            if(DRAW_FRAMEBUFFER_BINDING_NAME != 0){
                GLenum drawbuffer = gc->state.raster.drawBuffers[0];
                gc->frameBuffer.drawFramebufObj->drawBuffers[0] = sp->state.raster.drawBuffers[0];
                gc->state.raster = sp->state.raster;
                gc->state.raster.drawBuffers[0] = drawbuffer;
            }
            else
#endif
            {
                gc->state.raster = sp->state.raster;
            }

            gc->state.enables.colorBuffer = sp->state.enables.colorBuffer;

            /* Notify attribute changes to SWP.
            */
            __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_DRAWBUFFER_BIT);

            /* Immediately notify attribute changes to DP.
            */
            (*gc->dp.changeDrawBuffers)(gc);

            /* Delayed notify attribute changes to DP.
            */
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_COLORBUF_ATTR_BITS);
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_COLORBUF_ATTR2_BITS);

        }

        if (mask & GL_CURRENT_BIT) {
            gc->state.current = sp->state.current;
            gc->state.rasterPos = sp->state.rasterPos;

        }

        if (mask & GL_DEPTH_BUFFER_BIT) {
            gc->state.depth = sp->state.depth;
            gc->state.enables.depthBuffer = sp->state.enables.depthBuffer;

#if GL_EXT_depth_bounds_test
            gc->state.depthBoundTest = sp->state.depthBoundTest;
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_DEPTHBOUNDTEST_BIT);
#endif

            /* Delayed notify attribute changes to DP.
            */
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_DEPTHBUF_ATTR_BITS);
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_DEPTHBUF_ATTR2_BITS);
        }

        if (mask & GL_ENABLE_BIT) {
            gc->state.enables = sp->state.enables;

            /* Immediately notify attribute changes to DP.
            */
  //          (*gc->dp.colorMaterialEndisable)(gc);

            /* Delayed notify attribute changes to DP.
            */
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_ATTR1_ENABLE_BITS);
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_ATTR2_ENABLE_BITS);
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_ATTR3_ENABLE_BITS);
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_LIGHTING_ATTRS, __GL_LIGHTING_ENABLE_BITS);

            __GL_INPUTMASK_CHANGED(gc);

            /* The upper 16 bits of globalDirtyState[OGL_CLIP_ATTRS] are for
             * clipPlane enable/disable.
             */
            attribMask = (1 << gc->constants.numberOfClipPlanes) - 1;
            gc->globalDirtyState[__GL_CLIP_ATTRS] = (attribMask << 16);
            gc->globalDirtyState[__GL_ALL_ATTRS] |= (1 << __GL_CLIP_ATTRS);

            for (i = 0; i < gc->constants.numberOfLights; i++) {
                __GL_SET_LIGHT_SRC_BIT(gc, i, __GL_LIGHT_ENDISABLE_BIT);
            }

            for (i = 0; i < __GL_MAX_TEXTURE_COORDS; i++) {
                __glSetTexEnableDimension(gc, i);
                __GL_SET_TEX_UNIT_BIT(gc, i, __GL_TEXTURE_ENABLE_BITS);
            }

            /* Validate enables.texUnits[i].programVS/PSEnabledDimension
            */
            /*lan disable it */
            //__glUpdateProgramEnableDimension(gc);

            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_VERTEX_PROGRAM_ENABLE);
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_FRAGMENT_PROGRAM_ENABLE);
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_VP_POINT_SIZE_ENABLE);
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_VP_TWO_SIDE_ENABLE);
        }

        if (mask & GL_EVAL_BIT) {
            gc->state.evaluator = sp->state.evaluator;
            gc->state.enables.eval = sp->state.enables.eval;

            /* Delayed notify attribute changes to DP.
            */
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_MAP1_ENDISABLE_BIT |
                                                            __GL_MAP2_ENDISABLE_BIT |
                                                            __GL_AUTONORMAL_ENDISABLE_BIT);
        }

        if (mask & GL_FOG_BIT) {
            gc->state.fog = sp->state.fog;
            gc->state.enables.fog = sp->state.enables.fog;

            /* Delayed notify attribute changes to DP.
            */
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_FOG_ATTR_BITS);
        }

        if (mask & GL_HINT_BIT) {
            gc->state.hints = sp->state.hints;

            /* Delayed notify attribute changes to DP.
            */
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_HINT_BIT);
        }

        if (mask & GL_LIGHTING_BIT) {
            gc->state.light = sp->state.light;
            gc->state.enables.lighting = sp->state.enables.lighting;

            /* Immediately notify attribute changes to DP.
            */
//            (*gc->dp.colorMaterialEndisable)(gc);

            /* Delayed notify attribute changes to DP.
            */
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_LIGHTING_ATTRS, __GL_LIGHTING_ATTR_BITS);
            for (i = 0; i < gc->constants.numberOfLights; i++) {
                __GL_SET_LIGHT_SRC_BIT(gc, i, __GL_LIGHT_SRC_BITS);
            }
        }

        if (mask & GL_LINE_BIT) {
            gc->state.line = sp->state.line;
            gc->state.enables.line = sp->state.enables.line;

            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_LINE_ATTR_BITS);
        }

        if (mask & GL_LIST_BIT) {
            gc->state.list = sp->state.list;
        }

        if (mask & GL_PIXEL_MODE_BIT) {
            gc->state.pixel.transferMode = sp->state.pixel.transferMode;
            gc->state.pixel.readBufferReturn = sp->state.pixel.readBufferReturn;
            gc->state.pixel.readBuffer = sp->state.pixel.readBuffer;
            gc->state.pixel.colorTable[__GL_COLOR_TABLE_INDEX].state =
                sp->state.pixel.colorTable[__GL_COLOR_TABLE_INDEX].state;
            gc->state.pixel.colorTable[__GL_POST_CONVOLUTION_COLOR_TABLE_INDEX].state =
                sp->state.pixel.colorTable[__GL_POST_CONVOLUTION_COLOR_TABLE_INDEX].state;
            gc->state.pixel.colorTable[__GL_POST_COLOR_MATRIX_COLOR_TABLE_INDEX].state =
                sp->state.pixel.colorTable[__GL_POST_COLOR_MATRIX_COLOR_TABLE_INDEX].state;
            gc->state.pixel.convolutionFilter[__GL_CONVOLUTION_1D_INDEX].state =
                sp->state.pixel.convolutionFilter[__GL_CONVOLUTION_1D_INDEX].state;
            gc->state.pixel.convolutionFilter[__GL_CONVOLUTION_2D_INDEX].state =
                sp->state.pixel.convolutionFilter[__GL_CONVOLUTION_2D_INDEX].state;
            gc->state.pixel.convolutionFilter[__GL_SEPARABLE_2D_INDEX].state =
                sp->state.pixel.convolutionFilter[__GL_SEPARABLE_2D_INDEX].state;

            /* Notify attribute changes to SWP.
            */
            __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_READBUFFER_BIT |
                                        __GL_SWP_PIXELTRANSFER_BIT |
                                        __GL_SWP_PIXELZOOM_BIT);


            /* Immediately notify attribute changes to DP.
            */
            /* lan: disable it, wait till pixel code is ready
            (*gc->dp.readBuffer)(gc);
            (*gc->dp.pixelTransfer)(gc);
            (*gc->dp.pixelZoom)(gc);
            */
        }

        if (mask & GL_POINT_BIT) {
            gc->state.point = sp->state.point;
            gc->state.enables.pointSmooth = sp->state.enables.pointSmooth;

            /* Delayed notify attribute changes to DP.
            */
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_POINT_ATTR_BITS);
        }

        if (mask & GL_POLYGON_BIT) {
            gc->state.polygon = sp->state.polygon;
            gc->state.enables.polygon = sp->state.enables.polygon;

            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1,
                            __GL_POLYGON_ATTR_BITS & (~__GL_POLYGONSTIPPLE_BIT));
        }

        if (mask & GL_POLYGON_STIPPLE_BIT) {
            gc->state.polygonStipple = sp->state.polygonStipple;
            gc->state.enables.polygon.stipple = sp->state.enables.polygon.stipple;

            /* Delayed notify attribute changes to DP.
            */
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_POLYGONSTIPPLE_ENDISABLE_BIT |
                                                            __GL_POLYGONSTIPPLE_BIT);
        }

        if (mask & GL_SCISSOR_BIT) {
            gc->state.scissor = sp->state.scissor;
            gc->state.enables.scissor = sp->state.enables.scissor;

            /* Delayed notify attribute changes to DP.
            */
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_SCISSORTEST_ENDISABLE_BIT | __GL_SCISSOR_BIT);
        }

        if (mask & GL_STENCIL_BUFFER_BIT) {
            gc->state.stencil = sp->state.stencil;
            gc->state.enables.stencilTest = sp->state.enables.stencilTest;

            /* Delayed notify attribute changes to DP.
            */
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_STENCIL_ATTR_BITS);
        }

        if (mask & GL_TEXTURE_BIT) {
            GLuint unit, targetIdx;
            /*
            ** If the texture name is different, a new binding is
            ** called for.  Deferring the binding is dangerous, because
            ** the state before the pop has to be saved with the
            ** texture that is being unbound.  If we defer the binding,
            ** we need to watch out for cases like two pops in a row
            ** or a pop followed by a bind.
            */
            for (unit = 0; unit < __GL_MAX_TEXTURE_COORDS; unit++) {
                __GLtextureUnitState *gcts = &gc->state.texture.texUnits[unit];
                __GLtextureUnitState *spts = &sp->state.texture.texUnits[unit];

                GLuint maxbinding;

                if(__glExtension[__GL_EXTID_ARB_texture_array].bEnabled)
                    maxbinding = __GL_MAX_TEXTURE_BINDINGS;
                else
                    maxbinding = __GL_TEXTURE_RECTANGLE_INDEX + 1;

                for (targetIdx = 0; targetIdx < maxbinding; targetIdx++) {
                    __GLtextureObject *boundTexObj;
                    if (gcts->texObj[targetIdx].name != spts->texObj[targetIdx].name) {
                        __glBindTexture(gc, unit, targetIdx, spts->texObj[targetIdx].name);
                    }

                    /*
                    ** There are two reasons that we need to copy the pushed texParameter
                    ** states to the bound texture object:
                    ** 1. the parameters have been changed after being pushed (but the binding does not change)
                    ** 2. the texture object has been deleted after being pushed
                    */
                    boundTexObj = gc->texture.units[unit].boundTextures[targetIdx];
                    boundTexObj->params = spts->texObj[targetIdx].params;
                }
            }

            /* Copy all texture states including the texParameter states back to gc->state.texture */
            gc->state.texture = sp->state.texture;
            /* Note: Only copy the first 8 texUnits which are used by fixed function */
            __GL_MEMCOPY(gc->state.enables.texUnits, sp->state.enables.texUnits,
                        sizeof(__GLTextureEnableState) * __GL_MAX_TEXTURE_COORDS);

            for (unit = 0; unit < __GL_MAX_TEXTURE_COORDS; unit++) {
                __glSetTexEnableDimension(gc, unit);
                /* Set all texture state dirty bits */
                __GL_SET_TEX_UNIT_BIT(gc, unit, __GL_TEX_UNIT_ALL_BITS);
            }

            /* Validate enables.texUnits[i].programVS/PSEnabledDimension
            */
            /*lan: disable it*/
//            __glUpdateProgramEnableDimension(gc);

            /* Notify DP the current active texture unit */
  //          (*gc->dp.activeTexture)(gc, gc->state.texture.activeTexIndex);

            /* Restore GL_TEXTURE_BUFFER_EXT binding point. */
            /*lan:disable it
            if (gc->state.texture.textureBufBinding !=
                gc->bufferObject.boundBuffer[__GL_TEXTURE_BUFFER_EXT_INDEX])
            {
                __glBindBuffer(gc, __GL_TEXTURE_BUFFER_EXT_INDEX,
                    gc->state.texture.textureBufBinding);
            }
            */
        }

        if (mask & GL_TRANSFORM_BIT) {
            gc->state.enables.transform = sp->state.enables.transform;
            gc->state.transform = sp->state.transform;
            __glim_MatrixMode(gc, gc->state.transform.matrixMode);

            /* Delayed notify attribute changes to DP.
            */
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3,
                                        __GL_NORMALIZE_ENDISABLE_BIT |
                                        __GL_RESCALENORMAL_ENDISABLE_BIT);

            /* The upper 16 bits are for clipPlane enable/disable.
            ** The lower 16 bits are for clipPlane parameters.
            */
            attribMask = (1 << gc->constants.numberOfClipPlanes) - 1;
            gc->globalDirtyState[__GL_CLIP_ATTRS] = attribMask | (attribMask << 16);
            gc->globalDirtyState[__GL_ALL_ATTRS] |= (1 << __GL_CLIP_ATTRS);
        }

        if (mask & GL_VIEWPORT_BIT) {
            gc->state.viewport = sp->state.viewport;

            /* Delayed notify attribute changes to DP.
            */
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2,
                                        __GL_VIEWPORT_BIT);
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1,
                                        __GL_DEPTHRANGE_BIT);
        }

        if (mask & GL_MULTISAMPLE_BIT) {
            gc->state.multisample = sp->state.multisample;
            gc->state.enables.multisample = sp->state.enables.multisample;
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_MULTISAMPLE_ATTR_BITS);
        }

        /* Re-compute the core clip rectangle.
        */

        if (mask & (GL_SCISSOR_BIT | GL_ENABLE_BIT)) {
            __glComputeClipBox(gc);
        }

        /*
        ** Clear out mask so that any memory frees done above won't get
        ** re-done when the context is destroyed
        */
        sp->mask = 0;
    }
    else {
        __glSetError(gc, GL_STACK_UNDERFLOW);
        return;
    }
}

GLuint __glCopyContext(__GLcontext *dst, __GLcontext *src, GLuint mask)
{
    GLuint attribMask, i;

    __GL_VERTEX_BUFFER_FLUSH(src);
    __GL_VERTEX_BUFFER_FLUSH(dst);

    if (mask & GL_ACCUM_BUFFER_BIT) {
        dst->state.accum = src->state.accum;

        /* Delayed notify attribute changes to DP.
        */
        __GL_SET_ATTR_DIRTY_BIT(dst, __GL_DIRTY_ATTRS_2, __GL_CLEARACCUM_BIT);
    }

    if (mask & GL_COLOR_BUFFER_BIT) {
        dst->state.raster = src->state.raster;
        dst->state.enables.colorBuffer = src->state.enables.colorBuffer;

        /* Notify attribute changes to SWP.
        */
        __GL_SET_SWP_DIRTY_ATTR(dst, __GL_SWP_DRAWBUFFER_BIT);

        /* Delayed notify attribute changes to DP.
        */
        __GL_SET_ATTR_DIRTY_BIT(dst, __GL_DIRTY_ATTRS_1, __GL_COLORBUF_ATTR_BITS);
        __GL_SET_ATTR_DIRTY_BIT(dst, __GL_DIRTY_ATTRS_2, __GL_COLORBUF_ATTR2_BITS);
    }

    if (mask & GL_CURRENT_BIT) {
        dst->state.current = src->state.current;
        dst->state.rasterPos = src->state.rasterPos;
    }

    if (mask & GL_DEPTH_BUFFER_BIT) {
        dst->state.depth = src->state.depth;
        dst->state.enables.depthBuffer = src->state.enables.depthBuffer;

        __GL_SET_ATTR_DIRTY_BIT(dst, __GL_DIRTY_ATTRS_1, __GL_DEPTHBUF_ATTR_BITS);
        __GL_SET_ATTR_DIRTY_BIT(dst, __GL_DIRTY_ATTRS_2, __GL_DEPTHBUF_ATTR2_BITS);
    }

    if (mask & GL_ENABLE_BIT) {
        dst->state.enables = src->state.enables;

        /* Immediately notify attribute changes to DP.
        */
//        (*dst->dp.colorMaterialEndisable)(dst);

        /* Delayed notify attribute changes to DP.
        */
        __GL_SET_ATTR_DIRTY_BIT(dst, __GL_DIRTY_ATTRS_1, __GL_ATTR1_ENABLE_BITS);
        __GL_SET_ATTR_DIRTY_BIT(dst, __GL_DIRTY_ATTRS_2, __GL_ATTR2_ENABLE_BITS);
        __GL_SET_ATTR_DIRTY_BIT(dst, __GL_DIRTY_ATTRS_3, __GL_ATTR3_ENABLE_BITS);
        __GL_SET_ATTR_DIRTY_BIT(dst, __GL_LIGHTING_ATTRS, __GL_LIGHTING_ENABLE_BITS);

        /* The upper 16 bits of globalDirtyState[OGL_CLIP_ATTRS] are for
            * clipPlane enable/disable.
            */
        attribMask = (1 << dst->constants.numberOfClipPlanes) - 1;
        dst->globalDirtyState[__GL_CLIP_ATTRS] = (attribMask << 16);
        dst->globalDirtyState[__GL_ALL_ATTRS] |= (1 << __GL_CLIP_ATTRS);

        for (i = 0; i < dst->constants.numberOfLights; i++) {
            __GL_SET_LIGHT_SRC_BIT(dst, i, __GL_LIGHT_ENDISABLE_BIT);
        }

        for (i = 0; i < __GL_MAX_TEXTURE_COORDS; i++) {
            __glSetTexEnableDimension(dst, i);
            __GL_SET_TEX_UNIT_BIT(dst, i, __GL_TEXTURE_ENABLE_BITS);
        }

        /* Validate enables.texUnits[i].programEnabledDimension
        */
        /*lan:disable it*/
        //__glUpdateProgramEnableDimension(dst);

        __GL_SET_ATTR_DIRTY_BIT(dst, __GL_PROGRAM_ATTRS, __GL_DIRTY_VERTEX_PROGRAM_ENABLE);
        __GL_SET_ATTR_DIRTY_BIT(dst, __GL_PROGRAM_ATTRS, __GL_DIRTY_FRAGMENT_PROGRAM_ENABLE);
        __GL_SET_ATTR_DIRTY_BIT(dst, __GL_PROGRAM_ATTRS, __GL_DIRTY_VP_POINT_SIZE_ENABLE);
        __GL_SET_ATTR_DIRTY_BIT(dst, __GL_PROGRAM_ATTRS, __GL_DIRTY_VP_TWO_SIDE_ENABLE);
    }

    if (mask & GL_EVAL_BIT) {
        dst->state.evaluator = src->state.evaluator;
        dst->state.enables.eval = src->state.enables.eval;

        /* Delayed notify attribute changes to DP.
        */
        __GL_SET_ATTR_DIRTY_BIT(dst, __GL_DIRTY_ATTRS_3, __GL_MAP1_ENDISABLE_BIT |
                                                        __GL_MAP2_ENDISABLE_BIT |
                                                        __GL_AUTONORMAL_ENDISABLE_BIT);

    }

    if (mask & GL_FOG_BIT) {
        dst->state.fog = src->state.fog;
        dst->state.enables.fog = src->state.enables.fog;

        /* Delayed notify attribute changes to DP.
        */
        __GL_SET_ATTR_DIRTY_BIT(dst, __GL_DIRTY_ATTRS_2, __GL_FOG_ATTR_BITS);
    }

    if (mask & GL_HINT_BIT) {
        dst->state.hints = src->state.hints;

        /* Delayed notify attribute changes to DP.
        */
        __GL_SET_ATTR_DIRTY_BIT(dst, __GL_DIRTY_ATTRS_2, __GL_HINT_BIT);
    }

    if (mask & GL_LIGHTING_BIT) {
        dst->state.light = src->state.light;
        dst->state.enables.lighting = src->state.enables.lighting;

        /* Immediately notify attribute changes to DP.
        */
//        (*dst->dp.colorMaterialEndisable)(dst);

        /* Delayed notify attribute changes to DP.
        */
        __GL_SET_ATTR_DIRTY_BIT(dst, __GL_LIGHTING_ATTRS, __GL_LIGHTING_ATTR_BITS);
        for (i = 0; i < dst->constants.numberOfLights; i++) {
            __GL_SET_LIGHT_SRC_BIT(dst, i, __GL_LIGHT_ENDISABLE_BIT);
        }
    }

    if (mask & GL_LINE_BIT) {
        dst->state.line = src->state.line;
        dst->state.enables.line = src->state.enables.line;

        __GL_SET_ATTR_DIRTY_BIT(dst, __GL_DIRTY_ATTRS_2, __GL_LINE_ATTR_BITS);
    }

    if (mask & GL_LIST_BIT) {
        dst->state.list = src->state.list;
    }

    if (mask & GL_PIXEL_MODE_BIT) {
        dst->state.pixel.transferMode = src->state.pixel.transferMode;
        dst->state.pixel.readBufferReturn = src->state.pixel.readBufferReturn;
        dst->state.pixel.readBuffer = src->state.pixel.readBuffer;
        dst->state.pixel.colorTable[__GL_COLOR_TABLE_INDEX].state =
            src->state.pixel.colorTable[__GL_COLOR_TABLE_INDEX].state;
        dst->state.pixel.colorTable[__GL_POST_CONVOLUTION_COLOR_TABLE_INDEX].state =
            src->state.pixel.colorTable[__GL_POST_CONVOLUTION_COLOR_TABLE_INDEX].state;
        dst->state.pixel.colorTable[__GL_POST_COLOR_MATRIX_COLOR_TABLE_INDEX].state =
            src->state.pixel.colorTable[__GL_POST_COLOR_MATRIX_COLOR_TABLE_INDEX].state;
        dst->state.pixel.convolutionFilter[__GL_CONVOLUTION_1D_INDEX].state =
            src->state.pixel.convolutionFilter[__GL_CONVOLUTION_1D_INDEX].state;
        dst->state.pixel.convolutionFilter[__GL_CONVOLUTION_2D_INDEX].state =
            src->state.pixel.convolutionFilter[__GL_CONVOLUTION_2D_INDEX].state;
        dst->state.pixel.convolutionFilter[__GL_SEPARABLE_2D_INDEX].state =
            src->state.pixel.convolutionFilter[__GL_SEPARABLE_2D_INDEX].state;

        /* Notify attribute changes to SWP.
        */
        __GL_SET_SWP_DIRTY_ATTR(dst, __GL_SWP_READBUFFER_BIT |
                                    __GL_SWP_PIXELTRANSFER_BIT |
                                    __GL_SWP_PIXELZOOM_BIT);

        /* Immediately notify attribute changes to DP.
        */
        /*lan: wait till pixel code is ready
        (*dst->dp.pixelTransfer)(dst);
        (*dst->dp.pixelZoom)(dst);
        */
    }

    if (mask & GL_POINT_BIT) {
        dst->state.point = src->state.point;
        dst->state.enables.pointSmooth = src->state.enables.pointSmooth;

        /* Delayed notify attribute changes to DP.
        */
        __GL_SET_ATTR_DIRTY_BIT(dst, __GL_DIRTY_ATTRS_2, __GL_POINT_ATTR_BITS);
    }

    if (mask & GL_POLYGON_BIT) {
        dst->state.polygon = src->state.polygon;
        dst->state.enables.polygon = src->state.enables.polygon;

        __GL_SET_ATTR_DIRTY_BIT(dst, __GL_DIRTY_ATTRS_1, __GL_POLYGON_ATTR_BITS);
    }

    if (mask & GL_POLYGON_STIPPLE_BIT) {
        dst->state.polygonStipple = src->state.polygonStipple;

        /* Delayed notify attribute changes to DP.
        */
        __GL_SET_ATTR_DIRTY_BIT(dst, __GL_DIRTY_ATTRS_1, __GL_POLYGONSTIPPLE_BIT);
    }

    if (mask & GL_SCISSOR_BIT) {
        dst->state.scissor = src->state.scissor;

        /* Delayed notify attribute changes to DP.
        */
        __GL_SET_ATTR_DIRTY_BIT(dst, __GL_DIRTY_ATTRS_2, __GL_SCISSOR_BIT);
    }

    if (mask & GL_STENCIL_BUFFER_BIT) {
        dst->state.stencil = src->state.stencil;
        dst->state.enables.stencilTest = src->state.enables.stencilTest;

        /* Delayed notify attribute changes to DP.
        */
        __GL_SET_ATTR_DIRTY_BIT(dst, __GL_DIRTY_ATTRS_1, __GL_STENCIL_ATTR_BITS);
    }

    if (mask & GL_TEXTURE_BIT) {
        GLuint unit, targetIdx;
        /*
        ** If the texture name is different, a new binding is
        ** called for.  Deferring the binding is dangerous, because
        ** the state before the pop has to be saved with the
        ** texture that is being unbound.  If we defer the binding,
        ** we need to watch out for cases like two pops in a row
        ** or a pop followed by a bind.
        */
        for (unit = 0; unit < __GL_MAX_TEXTURE_COORDS; unit++) {
            __GLtextureUnitState *dstts = &dst->state.texture.texUnits[unit];
            const __GLtextureUnitState *srcts = &src->state.texture.texUnits[unit];

            for (targetIdx = 0; targetIdx < __GL_MAX_TEXTURE_BINDINGS; targetIdx++) {
                if (dstts->texObj[targetIdx].name != srcts->texObj[targetIdx].name) {
                    __glBindTexture(dst, unit, targetIdx, srcts->texObj[targetIdx].name);
                }
            }
        }

        /* Copy all texture states */
        dst->state.texture = src->state.texture;
        /* Note: Only copy the first 8 texUnits which are used by fixed function */
        __GL_MEMCOPY(dst->state.enables.texUnits, src->state.enables.texUnits,
                    sizeof(__GLTextureEnableState) * __GL_MAX_TEXTURE_COORDS);

        for (unit = 0; unit < __GL_MAX_TEXTURE_COORDS; unit++) {
            /* Set all texture state dirty bits */
            __glSetTexEnableDimension(dst, unit);
            __GL_SET_TEX_UNIT_BIT(dst, unit, __GL_TEX_UNIT_ALL_BITS);
        }

        /* Validate enables.texUnits[i].programEnabledDimension
        */
        /*lan:disable it*/
        //__glUpdateProgramEnableDimension(dst);

        /* Notify DP the current active texture unit */
//        (*dst->dp.activeTexture)(dst, dst->state.texture.activeTexIndex);
    }

    if (mask & GL_TRANSFORM_BIT) {
        dst->state.enables.transform = src->state.enables.transform;
        dst->state.transform = src->state.transform;
        __glim_MatrixMode(dst, dst->state.transform.matrixMode);

        /* Delayed notify attribute changes to DP.
        */
        __GL_SET_ATTR_DIRTY_BIT(dst, __GL_DIRTY_ATTRS_3,
                                    __GL_NORMALIZE_ENDISABLE_BIT |
                                    __GL_RESCALENORMAL_ENDISABLE_BIT);

        /* The upper 16 bits are for clipPlane enable/disable.
        ** The lower 16 bits are for clipPlane parameters.
        */
        attribMask = (1 << dst->constants.numberOfClipPlanes) - 1;
        dst->globalDirtyState[__GL_CLIP_ATTRS] = attribMask | (attribMask << 16);
        dst->globalDirtyState[__GL_ALL_ATTRS] |= (1 << __GL_CLIP_ATTRS);
    }

    if (mask & GL_VIEWPORT_BIT) {
        dst->state.viewport = src->state.viewport;

        /* Delayed notify attribute changes to DP.
        */
        __GL_SET_ATTR_DIRTY_BIT(dst, __GL_DIRTY_ATTRS_2,
                                        __GL_VIEWPORT_BIT);
        __GL_SET_ATTR_DIRTY_BIT(dst, __GL_DIRTY_ATTRS_1,
                                        __GL_DEPTHRANGE_BIT);
    }

    /* InputMask could be changed by several states.
    */
    __GL_INPUTMASK_CHANGED(dst);

    return GL_TRUE;
}

GLvoid APIENTRY __glim_PushClientAttrib(__GLcontext *gc, GLbitfield mask)
{
    __GLclientAttribStack **spp;
    __GLclientAttribStack *sp;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    spp = gc->attribute.clientStackPointer;
    if (spp < &gc->attribute.clientStack[gc->constants.maxClientAttribStackDepth])
    {
        sp = *spp;
        if (!sp)
        {
            sp = (__GLclientAttribStack*)(*gc->imports.calloc)(gc, 1, sizeof(__GLclientAttribStack));
            *spp = sp;
        }
        sp->mask = mask;
        gc->attribute.clientStackPointer = spp + 1;

        if (mask & GL_CLIENT_PIXEL_STORE_BIT)
        {
            sp->clientState.pixel.packModes = gc->clientState.pixel.packModes;
            sp->clientState.pixel.unpackModes = gc->clientState.pixel.unpackModes;
        }

        if (mask & GL_CLIENT_VERTEX_ARRAY_BIT)
        {
            sp->clientState.vertexArray = gc->vertexArray.boundVAO->vertexArray;
        }
    }
    else
    {
        __glSetError(gc, GL_STACK_OVERFLOW);
        return;
    }
}

GLvoid APIENTRY __glim_PopClientAttrib(__GLcontext *gc)
{
    __GLclientAttribStack **spp;
    __GLclientAttribStack *sp;
    GLuint mask;
    GLuint i;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    spp = gc->attribute.clientStackPointer;
    if (spp > &gc->attribute.clientStack[0]) {
        --spp;
        sp = *spp;
        GL_ASSERT(sp != 0);

        mask = sp->mask;
        gc->attribute.clientStackPointer = spp;

        if (mask & GL_CLIENT_PIXEL_STORE_BIT) {
            /* Pop the pixel client state */
            gc->clientState.pixel = sp->clientState.pixel;

            /* Immediately notify attribute changes to DP.
            */
            /* lan:wait until pixel code is ready"
            (*gc->dp.pixelStore)(gc);
            */
            /* Restore PIXEL_PACK_BUFFER binding point. */
            if (gc->clientState.pixel.packBufBinding !=
//                gc->bufferObject.boundBuffer[__GL_PIXEL_PACK_BUFFER_INDEX])
                gc->bufferObject.generalBindingPoint[__GL_PIXEL_PACK_BUFFER_INDEX].boundBufName)
            {
                __glBindBufferToGeneralPoint(gc, __GL_PIXEL_PACK_BUFFER_INDEX,
                    gc->clientState.pixel.packBufBinding);
            }

            /* Restore PIXEL_UNPACK_BUFFER binding point. */
            if (gc->clientState.pixel.unpackBufBinding !=
                gc->bufferObject.generalBindingPoint[__GL_PIXEL_PACK_BUFFER_INDEX].boundBufName)
            {
                __glBindBufferToGeneralPoint(gc, __GL_PIXEL_UNPACK_BUFFER_INDEX,
                    gc->clientState.pixel.unpackBufBinding);
            }
        }

        if (mask & GL_CLIENT_VERTEX_ARRAY_BIT) {
            GLuint buffer, arrayBufBinding;
            __GLvertexArrayState *vertexArrayState = &gc->vertexArray.boundVAO->vertexArray;
            __GLvertexAttrib *pAttrib;
            __GLvertexAttribBinding *pAttribBinding;

            /* Pop the vertex array state */
            *vertexArrayState = sp->clientState.vertexArray;

            /* Save the current ARRAY_BUFFER binding because the following
            ** __glBindBuffer() will change it.
            */
//            arrayBufBinding = gc->clientState.vertexArray.arrayBufBinding;
            arrayBufBinding = vertexArrayState->arrayBufBinding;

            /* Restore the per-array binding points according to just poped
            ** gc->clientState.vertexArray.currentArrays[i].bufBinding.
            */
            for (i = 0; i < __GL_TOTAL_VERTEX_ATTRIBUTES; i++)
            {
//                buffer = gc->clientState.vertexArray.currentArrays[i].bufBinding;
                pAttrib = &vertexArrayState->attribute[i];
                pAttribBinding = &vertexArrayState->attributeBinding[pAttrib->attribBinding];
                buffer = pAttribBinding->boundArrayName;
                __glBindBufferToGeneralPoint(gc, __GL_ARRAY_BUFFER_INDEX, buffer);
//                gc->bufferObject.boundArrays[i] = gc->bufferObject.boundTarget[__GL_ARRAY_BUFFER_INDEX];


                /* Set all the vertex array dirty bits */
                __GL_SET_VARRAY_ENABLE_BIT(gc);
                __GL_SET_VARRAY_FORMAT_BIT(gc);
                __GL_SET_VARRAY_BINDING_BIT(gc);
            }

            /* Restore the ARRAY_BUFFER binding point according to the saved
            ** gc->clientState.vertexArray.arrayBufBinding.
            */
            if (arrayBufBinding != gc->bufferObject.generalBindingPoint[__GL_ARRAY_BUFFER_INDEX].boundBufName)
            {
                __glBindBufferToGeneralPoint(gc, __GL_ARRAY_BUFFER_INDEX, arrayBufBinding);
            }

            /* Restore the ELEMENT_ARRAY_BUFFER binding point according to the saved
            ** gc->clientState.vertexArray.elementBufBinding.
            */
            if (vertexArrayState->boundIdxName !=
                gc->bufferObject.generalBindingPoint[__GL_ELEMENT_ARRAY_BUFFER_INDEX].boundBufName)
            {
                __glBindBufferToGeneralPoint(gc, __GL_ELEMENT_ARRAY_BUFFER_INDEX,
                    vertexArrayState->boundIdxName);
            }

            /* Notify the new clientActiveUnit to DP.
            */
//            (*gc->dp.clientActiveTexture)(gc, gc->clientState.vertexArray.clientActiveUnit);
        }

        /*
        ** Clear out mask so that any memory frees done above won't get
        ** re-done when the context is destroyed
        */
        sp->mask = 0;
    }
    else {
        __glSetError(gc, GL_STACK_UNDERFLOW);
        return;
    }
}


/*
** Free any attribute state left on the stack.  Stop at the first
** zero in the array.
*/
GLvoid __glFreeAttribStackState(__GLcontext *gc)
{
    GLvoid *sp, **spp;

    /*
    ** Need to pop all pushed attributes to free storage.
    ** Then it will be safe to delete stack entries
    */
    for (spp = (GLvoid **)&gc->attribute.stack[0];
         spp < (GLvoid **)&gc->attribute.stack[gc->constants.maxAttribStackDepth];
         spp++)
    {
        sp = *spp;
        if (sp)
        {
            (*gc->imports.free)(gc, sp);
        }
        else
        {
            break;
        }
    }
    (*gc->imports.free)(gc, gc->attribute.stack);
    gc->attribute.stack = gc->attribute.stackPointer = NULL;

    /* Free clientStack.
    */
    for (spp = (GLvoid **)&gc->attribute.clientStack[0];
        spp < (GLvoid **)&gc->attribute.clientStack[gc->constants.maxClientAttribStackDepth];
        spp++)
    {
        sp = *spp;
        if (sp)
        {
            (*gc->imports.free)(gc, sp);
        }
        else
        {
            break;
        }
    }
    (*gc->imports.free)(gc, gc->attribute.clientStack);
    gc->attribute.clientStack = gc->attribute.clientStackPointer = NULL;
}

GLvoid __glInitAttribStackState(__GLcontext *gc)
{
    gc->attribute.stack = (__GLattributeStack **)
        (*gc->imports.calloc)(gc, gc->constants.maxAttribStackDepth,
                              sizeof(__GLattributeStack *) );
    gc->attribute.stackPointer = gc->attribute.stack;
    if (gc->attribute.stack == NULL) {
        __glSetError(gc, GL_OUT_OF_MEMORY);
        return;
    }

    gc->attribute.clientStack = (__GLclientAttribStack **)
        (*gc->imports.calloc)(gc, gc->constants.maxClientAttribStackDepth,
                              sizeof(__GLclientAttribStack *) );
    gc->attribute.clientStackPointer = gc->attribute.clientStack;
    if (gc->attribute.stack == NULL) {
        __glSetError(gc, GL_OUT_OF_MEMORY);
        return;
    }
}
