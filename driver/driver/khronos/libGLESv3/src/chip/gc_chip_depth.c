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
#include "gc_chip_context.h"

#define _GC_OBJ_ZONE    gcdZONE_ES30_DEPTH

/**************************************************************************/
/* Implementation of internal functions                                   */
/**************************************************************************/
__GL_INLINE gceCOMPARE
gcChipUtilConvertDepthStencilTest2HAL(
    GLenum test
    )
{
    gceCOMPARE dsTest2HAL[] =
    {
        gcvCOMPARE_NEVER,
        gcvCOMPARE_LESS,
        gcvCOMPARE_EQUAL,
        gcvCOMPARE_LESS_OR_EQUAL,
        gcvCOMPARE_GREATER,
        gcvCOMPARE_NOT_EQUAL,
        gcvCOMPARE_GREATER_OR_EQUAL,
        gcvCOMPARE_ALWAYS,
    };

    if (test >= GL_NEVER && test <= GL_ALWAYS)
    {
        return dsTest2HAL[test - GL_NEVER];
    }

    return gcvCOMPARE_INVALID;
}

__GL_INLINE gceSTENCIL_OPERATION
gcChipUtilConvertStencilOp2HAL(
    GLenum op
    )
{
    gceSTENCIL_OPERATION ret = gcvSTENCIL_OPERATION_INVALID;

    gcmHEADER_ARG("op=0x%04x", op);
    switch (op)
    {
    case GL_ZERO:
        ret = gcvSTENCIL_ZERO;
        break;
    case GL_KEEP:
        ret =  gcvSTENCIL_KEEP;
        break;
    case GL_REPLACE:
        ret =  gcvSTENCIL_REPLACE;
        break;
    case GL_INCR:
        ret =  gcvSTENCIL_INCREMENT_SATURATE;
        break;
    case GL_DECR:
        ret =  gcvSTENCIL_DECREMENT_SATURATE;
        break;
    case GL_INVERT:
        ret =  gcvSTENCIL_INVERT;
        break;
    case GL_INCR_WRAP:
        ret =  gcvSTENCIL_INCREMENT;
        break;
    case GL_DECR_WRAP:
        ret =  gcvSTENCIL_DECREMENT;
        break;
    }

    gcmFOOTER_ARG("return=%d", ret);
    return ret;
}

__GL_INLINE GLuint
gcChipGetStencilBits(
    __GLchipContext *chipCtx
    )
{
    gceSURF_FORMAT format = gcvSURF_UNKNOWN;
    GLuint result = 0;

    gcmHEADER_ARG("chipCtx=0x%x", chipCtx);

    if (chipCtx->drawDepthView.surf)
    {
        gcmVERIFY_OK(gcoSURF_GetFormat(chipCtx->drawDepthView.surf, gcvNULL, &format));
    }
    else if (chipCtx->drawStencilView.surf)
    {
        gcmVERIFY_OK(gcoSURF_GetFormat(chipCtx->drawStencilView.surf, gcvNULL, &format));
    }

    result = (gcvSURF_D24S8  == format ||
              gcvSURF_S8D32F == format ||
              gcvSURF_S8     == format ||
              gcvSURF_X24S8  == format
              ) ? 8 : 0;

    gcmFOOTER_ARG("return=0x%04x", result);
    return result;
}


__GL_INLINE gceSTATUS
gcChipSetStencilCompareFunction(
    __GLchipContext *chipCtx,
    GLenum front,
    GLenum Function,
    GLuint Mask,
    GLenum face
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("chipCtx=0x%x front=0x%04x Function=0x%04x Mask=0x%x face=0x%04x",
                   chipCtx, front, Function, Mask, face);

    do
    {
        gceSTENCIL_WHERE halFace;
        gceCOMPARE compare = gcChipGetStencilBits(chipCtx)
                           ? gcChipUtilConvertDepthStencilTest2HAL(Function)
                           : gcvCOMPARE_ALWAYS;

        if (GL_FRONT == face)
        {
            halFace = (GL_CCW == front) ? gcvSTENCIL_FRONT : gcvSTENCIL_BACK;
        }
        else
        {
            halFace = (GL_CCW == front) ? gcvSTENCIL_BACK : gcvSTENCIL_FRONT;
        }

        if (chipCtx->drawYInverted)
        {
            halFace = (gcvSTENCIL_FRONT == halFace) ? gcvSTENCIL_BACK : gcvSTENCIL_FRONT;
        }

        if (gcvSTENCIL_FRONT == halFace)
        {
            gcmERR_BREAK(gco3D_SetStencilMask(chipCtx->engine, (gctUINT8)Mask));
        }
        else
        {
            gcmERR_BREAK(gco3D_SetStencilMaskBack(chipCtx->engine, (gctUINT8)Mask));
        }

        gcmERR_BREAK(gco3D_SetStencilCompare(chipCtx->engine, halFace, compare));

    } while (gcvFALSE);

    gcmFOOTER();
    return status;
}

__GL_INLINE gceSTATUS
gcChipSetStencilOperations(
    __GLchipContext *chipCtx,
    GLenum front,
    GLenum Fail,
    GLenum ZFail,
    GLenum ZPass,
    GLenum face
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("chipCtx=0x%x front=ox04x Fail=0x%04x ZFail=0x%04x ZPass=0x%04x face=0x%04x",
                  chipCtx, front, Fail, ZFail, ZPass, face);

    do
    {
        gceSTENCIL_OPERATION fail  = gcChipUtilConvertStencilOp2HAL(Fail);
        gceSTENCIL_OPERATION zFail = gcChipUtilConvertStencilOp2HAL(ZFail);
        gceSTENCIL_OPERATION zPass = gcChipUtilConvertStencilOp2HAL(ZPass);
        gceSTENCIL_WHERE halFace;

        if (GL_FRONT == face)
        {
            halFace = (GL_CCW == front) ? gcvSTENCIL_FRONT : gcvSTENCIL_BACK;
        }
        else
        {
            halFace = (GL_CCW == front) ? gcvSTENCIL_BACK : gcvSTENCIL_FRONT;
        }

        if (chipCtx->drawYInverted)
        {
            halFace = (gcvSTENCIL_FRONT == halFace) ? gcvSTENCIL_BACK : gcvSTENCIL_FRONT;
        }

        gcmERR_BREAK(gco3D_SetStencilFail(chipCtx->engine, halFace, fail));
        gcmERR_BREAK(gco3D_SetStencilDepthFail(chipCtx->engine, halFace, zFail));
        gcmERR_BREAK(gco3D_SetStencilPass(chipCtx->engine, halFace, zPass));

    } while (gcvFALSE);

    gcmFOOTER();
    return status;
}

__GL_INLINE gceSTATUS
gcChipSetStencilWriteMask(
    __GLchipContext *chipCtx,
    GLenum front,
    GLint mask,
    GLenum face
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("chipCtx=0x%x front=0x%04x mask=0x%x face=0x%04x", chipCtx, front, mask, face);

    do
    {
        gctBOOL halFront = gcvFALSE;
        if (GL_FRONT == face)
        {
            halFront = (GL_CCW == front) ? gcvTRUE : gcvFALSE;
        }
        else
        {
            halFront = (GL_CCW == front) ? gcvFALSE : gcvTRUE;
        }

        if (chipCtx->drawYInverted)
        {
            halFront = (gcvSTENCIL_FRONT == halFront) ? gcvSTENCIL_BACK : gcvSTENCIL_FRONT;
        }

        if (halFront)
        {
            gcmERR_BREAK(gco3D_SetStencilWriteMask(chipCtx->engine, (gctUINT8)(mask & 0x00FF)));
        }
        else
        {
            gcmERR_BREAK(gco3D_SetStencilWriteMaskBack(chipCtx->engine, (gctUINT8)(mask & 0x00FF)));
        }
    } while (gcvFALSE);

    gcmFOOTER();
    return status;
}



/**************************************************************************/
/* Implementation of EXPORTED FUNCTIONS                                   */
/**************************************************************************/

/*
** Stencil test state setting
*/
gceSTATUS
gcChipSetStencilStates(
    __GLcontext *gc,
    GLbitfield localMask
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x localMask=0x%x", gc, localMask);

    /* If stencil test was disabled, no need to send those test related parameters */
    if (gc->state.enables.stencilTest)
    {
        if (localMask & (__GL_STENCILMASK_FRONT_BIT | __GL_FRONTFACE_BIT))
        {
            gcmONERROR(gcChipSetStencilWriteMask(chipCtx,
                                                   gc->state.polygon.frontFace,
                                                   gc->state.stencil.front.writeMask,
                                                   GL_FRONT));
        }

        if (localMask & (__GL_STENCILMASK_BACK_BIT | __GL_FRONTFACE_BIT))
        {
            gcmONERROR(gcChipSetStencilWriteMask(chipCtx,
                                                   gc->state.polygon.frontFace,
                                                   gc->state.stencil.back.writeMask,
                                                   GL_BACK));
        }

        /* set front face stencil information */
        if (localMask & (__GL_STENCILOP_FRONT_BIT | __GL_FRONTFACE_BIT))
        {
            gcmONERROR(gcChipSetStencilOperations(chipCtx,
                                                    gc->state.polygon.frontFace,
                                                    gc->state.stencil.front.fail,
                                                    gc->state.stencil.front.depthFail,
                                                    gc->state.stencil.front.depthPass,
                                                    GL_FRONT));
        }

        /* set back face stencil information */
        if (localMask & (__GL_STENCILOP_BACK_BIT | __GL_FRONTFACE_BIT))
        {
            gcmONERROR(gcChipSetStencilOperations(chipCtx,
                                                    gc->state.polygon.frontFace,
                                                    gc->state.stencil.back.fail,
                                                    gc->state.stencil.back.depthFail,
                                                    gc->state.stencil.back.depthPass,
                                                    GL_BACK));
        }

        if (localMask & (__GL_STENCILFUNC_FRONT_BIT | __GL_FRONTFACE_BIT))
        {
            chipCtx->chipDirty.uDefer.sDefer.stencilRef = 1;

            gcmONERROR(gcChipSetStencilCompareFunction(chipCtx,
                                                         gc->state.polygon.frontFace,
                                                         gc->state.stencil.front.testFunc,
                                                         gc->state.stencil.front.mask,
                                                         GL_FRONT));
        }

        if (localMask & (__GL_STENCILFUNC_BACK_BIT | __GL_FRONTFACE_BIT))
        {
            chipCtx->chipDirty.uDefer.sDefer.stencilRef = 1;

            gcmONERROR(gcChipSetStencilCompareFunction(chipCtx,
                                                         gc->state.polygon.frontFace,
                                                         gc->state.stencil.back.testFunc,
                                                         gc->state.stencil.back.mask,
                                                         GL_BACK));
        }
    }

    if (localMask & __GL_STENCILTEST_ENDISABLE_BIT)
    {
        chipCtx->chipDirty.uDefer.sDefer.stencilMode = 1;
        chipCtx->chipDirty.uDefer.sDefer.stencilTest = 1;
        chipCtx->chipDirty.uDefer.sDefer.stencilRef  = 1;
    }

OnError:
    gcmFOOTER();
    return status;
}


gceSTATUS
gcChipSetStencilRef(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    )
{
    gceSTATUS status =gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%04x", gc, chipCtx);

    do
    {
        if (gc->state.enables.stencilTest && chipCtx->drawStencilView.surf)
        {
            gctUINT8 clampedRef;
            gctBOOL frontCCW = (gc->state.polygon.frontFace == GL_CCW);

            if (chipCtx->drawYInverted)
            {
                clampedRef = (gctUINT8)__glClampi(gc->state.stencil.back.reference, 0, chipCtx->drawStencilMask);
                gcmERR_BREAK(gco3D_SetStencilReference(chipCtx->engine, clampedRef, frontCCW));

                clampedRef = (gctUINT8)__glClampi(gc->state.stencil.front.reference,  0, chipCtx->drawStencilMask);
                gcmERR_BREAK(gco3D_SetStencilReference(chipCtx->engine, clampedRef, !frontCCW));
            }
            else
            {
                clampedRef = (gctUINT8)__glClampi(gc->state.stencil.front.reference, 0, chipCtx->drawStencilMask);
                gcmERR_BREAK(gco3D_SetStencilReference(chipCtx->engine, clampedRef, frontCCW));

                clampedRef = (gctUINT8)__glClampi(gc->state.stencil.back.reference,  0, chipCtx->drawStencilMask);
                gcmERR_BREAK(gco3D_SetStencilReference(chipCtx->engine, clampedRef, !frontCCW));
            }
        }
    } while (gcvFALSE);

    gcmFOOTER();
    return status;
}


gceSTATUS
gcChipSetStencilMode(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x", gc);

    do
    {
        gceSTENCIL_MODE stencilMode;

        /*
        ** Stencil will affect early Z programming, but will NOT affect HW stencil mode anymore.
        ** As long as stencil plane exist, we will program hardware stencil mode is TWO_SIDE.
        ** Detail can refer to gcoHARDWARE_FlushStencil
        */
        stencilMode = (gc->state.enables.stencilTest && chipCtx->drawStencilView.surf)
                      ? gcvSTENCIL_DOUBLE_SIDED
                      : gcvSTENCIL_NONE;

        /* Set the stencil mode. */
        gcmERR_BREAK(gco3D_SetStencilMode(chipCtx->engine, stencilMode));
    } while (GL_FALSE);

    gcmFOOTER();
    return status;
}


/*
** Stencil Test enable state switch
*/
gceSTATUS
gcChipSetStencilTest(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x", gc);

    /*
    ** stencil test enabled and we have stencil buffer, we can't rely on STENCIL mode
    ** as it could always be ON for MC work correctly for comp/decompression.
    */
    if (gc->state.enables.stencilTest && chipCtx->drawStencilView.surf)
    {
        /* If stencil test was switched back from disabled to enabled,
        ** need to reset all stencil test parameters even not dirtied.
        */
        gcmONERROR(gcChipSetStencilWriteMask(chipCtx, gc->state.polygon.frontFace,
                                               gc->state.stencil.front.writeMask, GL_FRONT));

        gcmONERROR(gcChipSetStencilWriteMask(chipCtx, gc->state.polygon.frontFace,
                                               gc->state.stencil.back.writeMask, GL_BACK));

        gcmONERROR(gcChipSetStencilOperations(chipCtx,
                                                gc->state.polygon.frontFace,
                                                gc->state.stencil.front.fail,
                                                gc->state.stencil.front.depthFail,
                                                gc->state.stencil.front.depthPass,
                                                GL_FRONT));
        gcmONERROR(gcChipSetStencilOperations(chipCtx,
                                                gc->state.polygon.frontFace,
                                                gc->state.stencil.back.fail,
                                                gc->state.stencil.back.depthFail,
                                                gc->state.stencil.back.depthPass,
                                                GL_BACK));
        gcmONERROR(gcChipSetStencilCompareFunction(chipCtx,
                                                     gc->state.polygon.frontFace,
                                                     gc->state.stencil.front.testFunc,
                                                     gc->state.stencil.front.mask,
                                                     GL_FRONT));
        gcmONERROR(gcChipSetStencilCompareFunction(chipCtx,
                                                     gc->state.polygon.frontFace,
                                                     gc->state.stencil.back.testFunc,
                                                     gc->state.stencil.back.mask,
                                                     GL_BACK));
    }
    else
    {
        /* If stencil test was disabled, set stencil test parameters to unaffected ones. */
        gcmONERROR(gcChipSetStencilWriteMask(chipCtx, GL_CCW, 0x00, GL_FRONT));
        gcmONERROR(gcChipSetStencilWriteMask(chipCtx, GL_CCW, 0x00, GL_BACK));
        gcmONERROR(gcChipSetStencilOperations(chipCtx, GL_CCW, GL_KEEP, GL_KEEP, GL_KEEP, GL_FRONT));
        gcmONERROR(gcChipSetStencilOperations(chipCtx, GL_CCW, GL_KEEP, GL_KEEP, GL_KEEP, GL_BACK));
        gcmONERROR(gcChipSetStencilCompareFunction(chipCtx, GL_CCW, GL_ALWAYS, 0, GL_FRONT));
        gcmONERROR(gcChipSetStencilCompareFunction(chipCtx, GL_CCW, GL_ALWAYS, 0, GL_BACK));
    }

OnError:
    gcmFOOTER();
    return status;
}


gceSTATUS
gcChipSetDepthMode(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x", gc);
    do
    {
        /* If depth or stencil test was enabled and the corresponding buffer exists.
        ** DepthMode will be really set at gcChipBegin and consider drawDepth dirty
        */
        chipCtx->depthMode = ((gc->state.enables.depthTest && chipCtx->drawDepthView.surf) ||
                              (gc->state.enables.stencilTest && chipCtx->drawStencilView.surf))
                           ? gcvDEPTH_Z
                           : gcvDEPTH_NONE;

        if (!gc->frameBuffer.drawFramebufObj->name &&
            (chipCtx->patchId == gcvPATCH_DEQP || chipCtx->patchId == gcvPATCH_GTFES30) &&
            chipCtx->drawDepthView.surf &&
            !chipCtx->chipFeature.hwFeature.hasBugFixes7)
        {
            chipCtx->depthMode = gcvDEPTH_Z;
        }

        /* Set the depth mode. */
        gcmERR_BREAK(gco3D_SetDepthMode(chipCtx->engine, chipCtx->depthMode));
    } while (GL_FALSE);


    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipSetDepthRange(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x", gc);

    /* Set the depth range. */
   gcmVERIFY_OK(gco3D_SetDepthRangeF(chipCtx->engine,
                                     chipCtx->depthMode,
                                     gc->state.depth.zNear,
                                     gc->state.depth.zFar));

    gcmFOOTER();
    return status;
}

/*
** Depth test enable state switch
*/
gceSTATUS
gcChipSetDepthTest(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x", gc);

    /* Restore depth state, if depth test enabled and depth buffer exist.
    ** We can't rely on depthMode, which can be not NONE if stencil test need it.
    */
    if (gc->state.enables.depthTest && chipCtx->drawDepthView.surf)
    {
        gcmONERROR(gcChipSetDepthCompareFunction(chipCtx, gc->state.depth.testFunc));
    }
    else
    {
        gcmONERROR(gcChipSetDepthCompareFunction(chipCtx, GL_ALWAYS));
    }

OnError:
    gcmFOOTER();
    return status;
}


/*
** Depth test state setting
*/
gceSTATUS
gcChipSetDepthStates(
    __GLcontext *gc,
    GLbitfield localMask
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x localMask=0x%x", gc, localMask);

    if (localMask & __GL_DEPTHTEST_ENDISABLE_BIT)
    {
        chipCtx->chipDirty.uDefer.sDefer.depthMode  = 1;
        chipCtx->chipDirty.uDefer.sDefer.depthTest  = 1;
        chipCtx->chipDirty.uDefer.sDefer.depthMask  = 1;
    }

    /* If depth test was disabled, no need to send those test related parameters */
    if (gc->state.enables.depthTest)
    {
        if (localMask & __GL_DEPTHFUNC_BIT)
        {
            gcmONERROR(gcChipSetDepthCompareFunction(chipCtx, gc->state.depth.testFunc));
        }
    }

OnError:
    gcmFOOTER();
    return status;
}


gceSTATUS
gcChipSetDepthCompareFunction(
    __GLchipContext *chipCtx,
    GLenum testFunction
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("chipCtx=0x%x testFunction=0x%x", chipCtx, testFunction);

    do
    {
        gceCOMPARE compare = gcChipUtilConvertDepthStencilTest2HAL(testFunction);
        gcmERR_BREAK(gco3D_SetDepthCompare(chipCtx->engine, compare));
    } while (gcvFALSE);


    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipSetDepthMask(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gctBOOL depthWrite;
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x", gc);

    depthWrite = (gc->state.enables.depthTest && chipCtx->drawDepthView.surf)
               ? gc->state.depth.writeEnable
               : gcvFALSE;

    status = gco3D_EnableDepthWrite(chipCtx->engine, depthWrite);

    gcmFOOTER();
    return status;
}


gceSTATUS
gcChipSetPolygonOffset(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x ", gc);
    if (chipCtx->drawDepthView.surf)
    {
        __GLpolygonState *polygonState = &gc->state.polygon;
        if (gc->state.enables.polygon.polygonOffsetFill)
        {
            gceSURF_FORMAT format;
            GLfloat units = polygonState->units;
            gcmONERROR(gcoSURF_GetFormat(chipCtx->drawDepthView.surf, gcvNULL, &format));

            switch (format)
            {
            case gcvSURF_D16:
                units = (units * 2) / 65535.0f; /* 2 / (2 ^ 16 - 1) */
                break;
            case gcvSURF_D24S8:
            case gcvSURF_D24X8:
                units = (units * 2) / 16777215.0f; /* 2 / (2 ^ 24 - 1) */
                break;
            case gcvSURF_D32:
                units = (units * 2) / 4294967295.0f; /* 2 / (2.0 ^ 32 - 1)*/
                break;
            default:
                break;
            }

            gcmONERROR(gco3D_SetDepthScaleBiasF(chipCtx->engine,
                                                polygonState->factor,
                                                units));
        }
        else
        {
            gcmONERROR(gco3D_SetDepthScaleBiasF(chipCtx->engine,
                                                0.0F,
                                                0.0F));
        }
    }

OnError:
    gcmFOOTER();
    return status;
}


