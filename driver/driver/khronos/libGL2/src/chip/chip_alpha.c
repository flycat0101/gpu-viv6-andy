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

GLenum alphaTestNames[] =
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

GLenum blendModeNames[] =
{
    GL_FUNC_ADD,                    /* glvBLENDADD */
    GL_FUNC_SUBTRACT,               /* glvBLENDSUBTRACT */
    GL_FUNC_REVERSE_SUBTRACT,       /* glvBLENDREVERSESUBTRACT */
    GL_MIN,                         /* glvBLNEDMIN */
    GL_MAX                          /* glvBLENDMAX */
};

GLenum srcBlendFunctionNames[] =
{
    GL_ZERO,                    /* glvBLENDZERO */
    GL_ONE,                     /* glvBLENDONE */
    GL_SRC_COLOR,               /* glvBLENDSRCCOLOR */
    GL_ONE_MINUS_SRC_COLOR,     /* glvBLENDSRCCOLORINV */
    GL_SRC_ALPHA,               /* glvBLENDSRCALPHA */
    GL_ONE_MINUS_SRC_ALPHA,     /* glvBLENDSRCALPHAINV */
    GL_DST_COLOR,               /* glvBLENDDSTCOLOR */
    GL_ONE_MINUS_DST_COLOR,     /* glvBLENDDSTCOLORINV */
    GL_DST_ALPHA,               /* glvBLENDDSTALPHA */
    GL_ONE_MINUS_DST_ALPHA,     /* glvBLENDDSTALPHAINV */
    GL_SRC_ALPHA_SATURATE,      /* glvBLENDSRCALPHASATURATE */
};

GLenum destBlendFunctionNames[] =
{
    GL_ZERO,                    /* glvBLENDZERO */
    GL_ONE,                     /* glvBLENDONE */
    GL_SRC_COLOR,               /* glvBLENDSRCCOLOR */
    GL_ONE_MINUS_SRC_COLOR,     /* glvBLENDSRCCOLORINV */
    GL_SRC_ALPHA,               /* glvBLENDSRCALPHA */
    GL_ONE_MINUS_SRC_ALPHA,     /* glvBLENDSRCALPHAINV */
    GL_DST_COLOR,               /* glvBLENDDSTCOLOR */
    GL_ONE_MINUS_DST_COLOR,     /* glvBLENDDSTCOLORINV */
    GL_DST_ALPHA,               /* glvBLENDDSTALPHA */
    GL_ONE_MINUS_DST_ALPHA,     /* glvBLENDDSTALPHAINV */
};


/******************************************************************************\
***************************** HAL Translation Arrays ***************************
\******************************************************************************/

gceCOMPARE alphaTestValues[] =
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

gceBLEND_MODE blendModeValues[] =
{
    gcvBLEND_ADD,                   /* glvBLENDADD */
    gcvBLEND_SUBTRACT,              /* glvBLENDSUBTRACT */
    gcvBLEND_REVERSE_SUBTRACT,      /* glvBLENDREVERSESUBTRACT */
    gcvBLEND_MIN,                   /* glvBLENDMIN */
    gcvBLEND_MAX                    /* glvBLENDMAX */
};

gceBLEND_FUNCTION blendFunctionValues[] =
{
    gcvBLEND_ZERO,                  /* glvBLENDZERO */
    gcvBLEND_ONE,                   /* glvBLENDONE */
    gcvBLEND_SOURCE_COLOR,          /* glvBLENDSRCCOLOR */
    gcvBLEND_INV_SOURCE_COLOR,      /* glvBLENDSRCCOLORINV */
    gcvBLEND_SOURCE_ALPHA,          /* glvBLENDSRCALPHA */
    gcvBLEND_INV_SOURCE_ALPHA,      /* glvBLENDSRCALPHAINV */
    gcvBLEND_TARGET_COLOR,          /* glvBLENDDSTCOLOR */
    gcvBLEND_INV_TARGET_COLOR,      /* glvBLENDDSTCOLORINV */
    gcvBLEND_TARGET_ALPHA,          /* glvBLENDDSTALPHA */
    gcvBLEND_INV_TARGET_ALPHA,      /* glvBLENDDSTALPHAINV */
    gcvBLEND_SOURCE_ALPHA_SATURATE, /* glvBLENDSRCALPHASATURATE */
};


/******************************************************************************\
********************** Individual State Setting Functions **********************
\******************************************************************************/

GLenum setAlphaTestReference(
    glsCHIPCONTEXT_PTR chipCtx,
    GLenum Function,
    GLfloat Value
    )
{
    GLenum result;
    gcmHEADER_ARG("Context=0x%x Function=0x%04x Value=0x%x",
                    chipCtx, Function, Value);
    do
    {
        GLuint function;
        gceSTATUS status;

        if (!glfConvertGLEnum(
                alphaTestNames,
                gcmCOUNTOF(alphaTestNames),
                &Function, glvINT,
                &function
                ))
        {
            result = GL_INVALID_ENUM;
            break;
        }

        do
        {
            gcmERR_BREAK(gco3D_SetAlphaCompare(
                chipCtx->hw,
                alphaTestValues[function]
                ));

            gcmERR_BREAK(gco3D_SetAlphaReferenceF(
                chipCtx->hw,
                Value
                ));
        }
        while (gcvFALSE);

        result = glmTRANSLATEHALSTATUS(status);
    }
    while (gcvFALSE);

    gcmFOOTER_ARG("return=0x%04x", result);

    return result;
}

GLenum setBlendFuncSeparate(
    glsCHIPCONTEXT_PTR chipCtx,
    GLenum SrcRGB,
    GLenum DstRGB,
    GLenum SrcAlpha,
    GLenum DstAlpha
    )
{
    GLenum result;
    gcmHEADER_ARG("Context=0x%x SrcRGB=0x%04x DstRGB=0x%x SrcAlpha=0x%04x DstAlpha=0x%x",
                    chipCtx, SrcRGB, DstRGB, SrcAlpha, DstAlpha);
    do
    {
        GLuint blendSrcRGB, blendSrcAlpha;
        GLuint blendDstRGB, blendDstAlpha;
        gceSTATUS status;

        if (!glfConvertGLEnum(
                srcBlendFunctionNames,
                gcmCOUNTOF(srcBlendFunctionNames),
                &SrcRGB, glvINT,
                &blendSrcRGB
                ))
        {
            result = GL_INVALID_ENUM;
            break;
        }

        if (!glfConvertGLEnum(
                destBlendFunctionNames,
                gcmCOUNTOF(destBlendFunctionNames),
                &DstRGB, glvINT,
                &blendDstRGB
                ))
        {
            result = GL_INVALID_ENUM;
            break;
        }

        if (!glfConvertGLEnum(
                srcBlendFunctionNames,
                gcmCOUNTOF(srcBlendFunctionNames),
                &SrcAlpha, glvINT,
                &blendSrcAlpha
                ))
        {
            result = GL_INVALID_ENUM;
            break;
        }

        if (!glfConvertGLEnum(
                destBlendFunctionNames,
                gcmCOUNTOF(destBlendFunctionNames),
                &DstAlpha, glvINT,
                &blendDstAlpha
                ))
        {
            result = GL_INVALID_ENUM;
            break;
        }

        do
        {
            gceBLEND_FUNCTION srcFunctionRGB =
                blendFunctionValues[blendSrcRGB];

            gceBLEND_FUNCTION dstFunctionRGB =
                blendFunctionValues[blendDstRGB];

            gceBLEND_FUNCTION srcFunctionAlpha =
                blendFunctionValues[blendSrcAlpha];

            gceBLEND_FUNCTION dstFunctionAlpha =
                blendFunctionValues[blendDstAlpha];

            gcmERR_BREAK(
                gco3D_SetBlendFunction(chipCtx->hw,
                               gcvBLEND_SOURCE,
                               srcFunctionRGB, srcFunctionAlpha));

            gcmERR_BREAK(
                gco3D_SetBlendFunction(chipCtx->hw,
                               gcvBLEND_TARGET,
                               dstFunctionRGB, dstFunctionAlpha));
        }
        while (gcvFALSE);

        result = glmTRANSLATEHALSTATUS(status);
    }
    while (gcvFALSE);

    gcmFOOTER_ARG("return=0x%04x", result);

    return result;
}

GLenum setBlendEquationSeparate(
    glsCHIPCONTEXT_PTR chipCtx,
    GLenum modeRGB,
    GLenum modeAlpha
    )
{
    GLenum result;

    gcmHEADER_ARG("Context=0x%x modeRGB=0x%04x modeAlpha=0x%x",
                    chipCtx, modeRGB, modeAlpha);
    do
    {
        gceSTATUS status;
        GLuint blendModeRGB;
        GLuint blendModeAlpha;

        if (!glfConvertGLEnum(
                blendModeNames,
                gcmCOUNTOF(blendModeNames),
                &modeRGB, glvINT,
                &blendModeRGB
                ))
        {
            result = GL_INVALID_ENUM;
            break;
        }

        if (!glfConvertGLEnum(
                blendModeNames,
                gcmCOUNTOF(blendModeNames),
                &modeAlpha, glvINT,
                &blendModeAlpha
                ))
        {
            result = GL_INVALID_ENUM;
            break;
        }

        do
        {
            gceBLEND_MODE gceBlendModeRGB =
                blendModeValues[blendModeRGB];

            gceBLEND_MODE gceBlendModeAlpha =
                blendModeValues[blendModeAlpha];

            gcmERR_BREAK(gco3D_SetBlendMode(
                chipCtx->hw,
                gceBlendModeRGB,
                gceBlendModeAlpha
                ));
        }
        while (gcvFALSE);

        result = glmTRANSLATEHALSTATUS(status);
    }
    while (gcvFALSE);

    gcmFOOTER_ARG("return=0x%04x", result);

    return result;
}
