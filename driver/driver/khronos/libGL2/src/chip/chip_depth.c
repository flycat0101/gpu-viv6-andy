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
#include "chip_context.h"

#define _GC_OBJ_ZONE    gcvZONE_API_GL

GLenum testNames[] =
{
    GL_NEVER,           /* glvNEVER */
    GL_LESS,            /* glvLESS */
    GL_EQUAL,           /* glvEQUAL */
    GL_LEQUAL,          /* glvLESSOREQUAL */
    GL_GREATER,         /* glvGREATER */
    GL_NOTEQUAL,        /* glvNOTEQUAL */
    GL_GEQUAL,          /* glvGREATEROREQUAL */
    GL_ALWAYS,          /* glvALWAYS */
};

GLenum stencilOperationNames[] =
{
    GL_ZERO,            /* glvSTENCILZERO */
    GL_KEEP,            /* glvSTENCILKEEP */
    GL_REPLACE,         /* glvSTENCILREPLACE */
    GL_INCR,            /* glvSTENCILINC */
    GL_DECR,            /* glvSTENCILDEC */
    GL_INVERT,          /* glvSTENCILINVERT */
    GL_INCR_WRAP,       /* missing */
    GL_INCR_WRAP,       /* missing */
};

gceSTENCIL_OPERATION stencilOperationValues[] =
{
    gcvSTENCIL_ZERO,                /* glvSTENCILZERO */
    gcvSTENCIL_KEEP,                /* glvSTENCILKEEP */
    gcvSTENCIL_REPLACE,             /* glvSTENCILREPLACE */
    gcvSTENCIL_INCREMENT_SATURATE,  /* glvSTENCILINC */
    gcvSTENCIL_DECREMENT_SATURATE,  /* glvSTENCILDEC */
    gcvSTENCIL_INVERT,              /* glvSTENCILINVERT */
};

gceCOMPARE stencilTestValues[] =
{
    gcvCOMPARE_NEVER,               /* glvNEVER */
    gcvCOMPARE_LESS,                /* glvLESS */
    gcvCOMPARE_EQUAL,               /* glvEQUAL */
    gcvCOMPARE_LESS_OR_EQUAL,       /* glvLESSOREQUAL */
    gcvCOMPARE_GREATER,             /* glvGREATER */
    gcvCOMPARE_NOT_EQUAL,           /* glvNOTEQUAL */
    gcvCOMPARE_GREATER_OR_EQUAL,    /* glvGREATEROREQUAL */
    gcvCOMPARE_ALWAYS,              /* glvALWAYS */
};

GLenum updateDepthFunction(
    glsCHIPCONTEXT_PTR chipCtx,
    GLuint function,
    GLboolean testEnabled
    )
{
    GLenum result;

    static gceCOMPARE depthTestValues[] =
    {
        gcvCOMPARE_NEVER,               /* glvNEVER */
        gcvCOMPARE_LESS,                /* glvLESS */
        gcvCOMPARE_EQUAL,               /* glvEQUAL */
        gcvCOMPARE_LESS_OR_EQUAL,       /* glvLESSOREQUAL */
        gcvCOMPARE_GREATER,             /* glvGREATER */
        gcvCOMPARE_NOT_EQUAL,           /* glvNOTEQUAL */
        gcvCOMPARE_GREATER_OR_EQUAL,    /* glvGREATEROREQUAL */
        gcvCOMPARE_ALWAYS,              /* glvALWAYS */
    };

    /* Determine the depth function. */
    gceCOMPARE testFunction
        = testEnabled
            ? depthTestValues[function]
            : gcvCOMPARE_ALWAYS;

    gcmHEADER_ARG("Context=0x%x function=0x%x testEnabled=0x%x bFill", chipCtx, function, testEnabled);

    /* Set the depth function. */
    result = glmTRANSLATEHALSTATUS(gco3D_SetDepthCompare(
        chipCtx->hw,
        testFunction
        ));

    gcmFOOTER_ARG("return=0x%04x", result);

    return result;
}


GLenum updateDepthEnableAndRange(__GLcontext *gc)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    GLenum result = gcvSTATUS_OK;
    gcmHEADER_ARG("Context=0x%x", gc);

    do
    {
        gceDEPTH_MODE depthMode;
        /* Determine the depth mode. */
        depthMode = ((gc->state.enables.depthBuffer.test||
            gc->state.enables.stencilTest) &&
            chipCtx->drawDepth) ? gcvDEPTH_Z : gcvDEPTH_NONE;

        /* Set the depth mode. */
        glmERR_BREAK(glmTRANSLATEHALSTATUS(gco3D_SetDepthMode(
            chipCtx->hw,
            depthMode
            )));

        /* Set the depth range. */
        result = glmTRANSLATEHALSTATUS(gco3D_SetDepthRangeF(
            chipCtx->hw,
            depthMode,
            gc->state.viewport.zNear,
            gc->state.viewport.zFar
            ));

    }
    while (gcvFALSE);

    gcmFOOTER_ARG("return=0x%04x", result);

    return result;
}

GLenum updatePolygonOffset(
     glsCHIPCONTEXT_PTR chipCtx,
    GLfloat factor,
    GLfloat units,
    GLboolean bFill
    )
{
    GLenum result;

    gcmHEADER_ARG("Context=0x%x factor=0x%x units=0x%x bFill=0x%0x", chipCtx, factor, units, bFill);

    if (bFill)
    {
        gceSURF_FORMAT format;
        if (gcvSTATUS_OK == gcoSURF_GetFormat(chipCtx->drawDepth, gcvNULL, &format))
        {
            switch (format)
            {
                case gcvSURF_D16:
                    units /= 65535.0f;  /* 2 ^ 16 */
                    break;

                case gcvSURF_D24S8:
                case gcvSURF_D24X8:
                    units /= 16777215.0f;   /* 2 ^ 24 */
                    break;

                case gcvSURF_D32:
                    units /= 4294967295.0f; /* 2.0 ^ 32 */
                    break;

                default:
                    break;
            }
        }

        result = glmTRANSLATEHALSTATUS(gco3D_SetDepthScaleBiasF(
            chipCtx->hw,
            factor,
            units
            ));
    }
    else
    {
        result = glmTRANSLATEHALSTATUS(gco3D_SetDepthScaleBiasF(
            chipCtx->hw,
            0.0F,
            0.0F
            ));
    }

    gcmFOOTER_ARG("return=0x%04x", result);

    return result;
}


GLenum setDepthCompareFunction(glsCHIPCONTEXT_PTR chipCtx, GLenum testFunction, GLboolean testEnable)
{
    GLenum result;

    gcmHEADER_ARG("Context=0x%x testFunction=0x%x testEnable=0x%x", chipCtx, testFunction, testEnable);

    do
    {
        GLuint function;

        if (!glfConvertGLEnum(
                testNames,
                gcmCOUNTOF(testNames),
                &testFunction, glvINT,
                &function
                ))
        {
            result = GL_INVALID_ENUM;
            break;
        }

        /* Set the depth function. */
        glmERR_BREAK(updateDepthFunction(chipCtx, function, testEnable));
    }
    while (gcvFALSE);

    gcmFOOTER_ARG("return=0x%04x", result);

    return result;
}

GLenum setDepthMask(
    glsCHIPCONTEXT_PTR  chipCtx,
    GLboolean depthMask
    )
{
    GLenum result;

    gcmHEADER_ARG("Context=0x%x depthMask=0x%x", chipCtx, depthMask);

    result = glmTRANSLATEHALSTATUS(gco3D_EnableDepthWrite(
        chipCtx->hw,
        depthMask
        ));

    gcmFOOTER_ARG("return=0x%04x", result);

    return result;
}

GLenum setClearDepth(glsCHIPCONTEXT_PTR  chipCtx, GLfloat clearValue)
{
    GLenum result;

    gcmHEADER_ARG("Context=0x%x clearValue=0x%x", chipCtx, clearValue);

    /* Set the clear value. */
    result = glmTRANSLATEHALSTATUS(gco3D_SetClearDepthF(chipCtx->hw, clearValue));

    gcmFOOTER_ARG("return=0x%04x", result);
    return result;
}

GLenum validatePolygonOffset(__GLcontext *gc)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    GLenum result = 0;
    gcmHEADER_ARG("Context=0x%x ", gc);

    if (chipCtx->drawDepth)
    {
        result = updatePolygonOffset(chipCtx, gc->state.polygon.factor, gc->state.polygon.units,
            gc->state.enables.polygon.polygonOffsetFill);
    }

    gcmFOOTER_ARG("return=0x%04x", result);
    return result;
}

GLuint getStencilBits(
    glsCHIPCONTEXT_PTR chipCtx)
{
    gceSURF_FORMAT format = gcvSURF_UNKNOWN;
    GLuint result = 0;

    gcmHEADER_ARG("Context=0x%x", chipCtx);

    if (chipCtx->drawDepth == gcvNULL)
    {
        gcmFOOTER_ARG("return=0x%04x", result);
        return result;
    }

    gcmVERIFY_OK(gcoSURF_GetFormat(chipCtx->drawDepth, gcvNULL, &format));

    result = (format == gcvSURF_D24S8)
        ? 8
        : 0;

    gcmFOOTER_ARG("return=0x%04x", result);
    return result;
}

GLenum setStencilCompareFunction(
    glsCHIPCONTEXT_PTR chipCtx,
    GLenum Function,
    GLint Reference,
    GLuint Mask,
    GLenum face
    )
{
    GLenum result;

    static gceCOMPARE stencilTestValues[] =
    {
        gcvCOMPARE_NEVER,               /* glvNEVER */
        gcvCOMPARE_LESS,                /* glvLESS */
        gcvCOMPARE_EQUAL,               /* glvEQUAL */
        gcvCOMPARE_LESS_OR_EQUAL,       /* glvLESSOREQUAL */
        gcvCOMPARE_GREATER,             /* glvGREATER */
        gcvCOMPARE_NOT_EQUAL,           /* glvNOTEQUAL */
        gcvCOMPARE_GREATER_OR_EQUAL,    /* glvGREATEROREQUAL */
        gcvCOMPARE_ALWAYS,              /* glvALWAYS */
    };

    gcmHEADER_ARG("Context=0x%x Function=0x%x Reference=0x%x Mask=0x%x face=0x%x",
        chipCtx, Function, Reference, Mask, face);

    do
    {
        GLuint function;

        if (!glfConvertGLEnum(
                testNames,
                gcmCOUNTOF(testNames),
                &Function, glvINT,
                &function
                ))
        {
            result = GL_INVALID_ENUM;
            break;
        }

        if (getStencilBits(chipCtx) == 0)
        {
            function = glvALWAYS;
        }

        gco3D_SetStencilMask(chipCtx->hw,Mask);
        if (face == GL_FRONT) {
            /* Set front */
            gco3D_SetStencilReference(chipCtx->hw, Reference, 1);
            /* Set back */
            gco3D_SetStencilReference(chipCtx->hw, Reference, 0);
            gco3D_SetStencilCompare(chipCtx->hw, gcvSTENCIL_FRONT, stencilTestValues[function] );
            gco3D_SetStencilCompare(chipCtx->hw, gcvSTENCIL_BACK, stencilTestValues[function]);
        } else {
            gco3D_SetStencilReference(chipCtx->hw, Reference, 0);
            gco3D_SetStencilCompare(chipCtx->hw, gcvSTENCIL_BACK, stencilTestValues[function]);
        }
        result = GL_NO_ERROR;
    }
    while (gcvFALSE);

    gcmFOOTER_ARG("return=0x%04x", result);
    return result;
}

GLvoid setWriteMask(
    glsCHIPCONTEXT_PTR chipCtx,
    GLuint Mask
    )
{
#if defined(DEBUG) || defined(_DEBUG)
    gceSTATUS result;
    gcmHEADER_ARG("Context=0x%x Mask=0x%x", chipCtx, Mask);
    result = gco3D_SetStencilWriteMask(chipCtx->hw, (gctUINT8)(Mask & 0x00FF));
    gcmFOOTER_ARG("return=0x%04x", result);
#else
    gcmHEADER_ARG("Context=0x%x Mask=0x%x", chipCtx, Mask);
    gcmVERIFY_OK(gco3D_SetStencilWriteMask(chipCtx->hw, (gctUINT8)(Mask & 0x00FF)));
    gcmFOOTER_ARG("return=%s,%d", __FUNCTION__,__LINE__);
#endif
}

GLenum setStencilOperations(
    glsCHIPCONTEXT_PTR chipCtx,
    GLenum Fail,
    GLenum ZFail,
    GLenum ZPass,
    GLenum face
    )
{
    GLenum result;

    static gceSTENCIL_OPERATION stencilOperationValues[] =
    {
        gcvSTENCIL_ZERO,                /* glvSTENCILZERO */
        gcvSTENCIL_KEEP,                /* glvSTENCILKEEP */
        gcvSTENCIL_REPLACE,             /* glvSTENCILREPLACE */
        gcvSTENCIL_INCREMENT_SATURATE,  /* glvSTENCILINC */
        gcvSTENCIL_DECREMENT_SATURATE,  /* glvSTENCILDEC */
        gcvSTENCIL_INVERT,              /* glvSTENCILINVERT */
    };

    gcmHEADER_ARG("Context=0x%x Fail=0x%x ZFail=0x%x ZPass=0x%x face=0x%x", chipCtx, Fail, ZFail, ZPass, face);

    do
    {
        GLuint fail;
        GLuint zFail;
        GLuint zPass;

        if (!glfConvertGLEnum(
                stencilOperationNames,
                gcmCOUNTOF(stencilOperationNames),
                &Fail, glvINT,
                &fail
                ))
        {
            result = GL_INVALID_ENUM;
            break;
        }

        if (!glfConvertGLEnum(
                stencilOperationNames,
                gcmCOUNTOF(stencilOperationNames),
                &ZFail, glvINT,
                &zFail
                ))
        {
            result = GL_INVALID_ENUM;
            break;
        }

        if (!glfConvertGLEnum(
                stencilOperationNames,
                gcmCOUNTOF(stencilOperationNames),
                &ZPass, glvINT,
                &zPass
                ))
        {
            result = GL_INVALID_ENUM;
            break;
        }

        if (face == GL_FRONT) {
            gco3D_SetStencilFail(chipCtx->hw, gcvSTENCIL_FRONT, stencilOperationValues[fail]);
            gco3D_SetStencilFail(chipCtx->hw, gcvSTENCIL_BACK, stencilOperationValues[fail]);
            gco3D_SetStencilDepthFail(chipCtx->hw, gcvSTENCIL_FRONT, stencilOperationValues[zFail]);
            gco3D_SetStencilDepthFail(chipCtx->hw, gcvSTENCIL_BACK, stencilOperationValues[zFail]);
            gco3D_SetStencilPass(chipCtx->hw, gcvSTENCIL_FRONT, stencilOperationValues[zPass]);
            gco3D_SetStencilPass(chipCtx->hw, gcvSTENCIL_BACK, stencilOperationValues[zPass]);
        } else {
            gco3D_SetStencilFail(chipCtx->hw, gcvSTENCIL_BACK, stencilOperationValues[fail]);
            gco3D_SetStencilDepthFail(chipCtx->hw, gcvSTENCIL_BACK, stencilOperationValues[zFail]);
            gco3D_SetStencilPass(chipCtx->hw, gcvSTENCIL_BACK, stencilOperationValues[zPass]);
        }

        result = GL_NO_ERROR;
    }
    while (gcvFALSE);

    gcmFOOTER_ARG("return=0x%04x", result);
    return result;
}

GLenum setClearStencil(
    glsCHIPCONTEXT_PTR chipCtx,
    GLint clearValue
    )
{
    GLenum result;
    gcmHEADER_ARG("Context=0x%x clearValue=0x%x", chipCtx, clearValue);
    result = glmTRANSLATEHALSTATUS(gco3D_SetClearStencil(
        chipCtx->hw,
        clearValue
        ));
    gcmFOOTER_ARG("return=0x%04x", result);
    return result;
}

