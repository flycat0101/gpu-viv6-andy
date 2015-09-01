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
#include "wgl_icd_ver.h"
#include "chip_context.h"

#define _GC_OBJ_ZONE    gcvZONE_API_GL

GLenum fogModeNames[] =
{
    GL_LINEAR,          /* glvLINEARFOG */
    GL_EXP,             /* glvEXPFOG */
    GL_EXP2,            /* glvEXP2FOG */
};


/******************************************************************************\
********************** Individual State Setting Functions **********************
\******************************************************************************/

GLenum setFogMode(
    glsCHIPCONTEXT_PTR chipCtx,
    const GLvoid* FogMode,
    gleTYPE Type
    )
{
    GLuint fogMode;

    if (!glfConvertGLEnum(
            fogModeNames,
            gcmCOUNTOF(fogModeNames),
            FogMode, Type,
            &fogMode
            ))
    {
        return GL_INVALID_ENUM;
    }

    chipCtx->hashKey.hashFogMode = fogMode;
    chipCtx->fogStates.mode = (gleFOGMODES) fogMode;

    return GL_NO_ERROR;
}

GLenum setFogDensity(
    glsCHIPCONTEXT_PTR chipCtx,
    const GLvoid* FogDensity,
    gleTYPE Type
    )
{
    /* Density cannot be negative. */
    if (glfFracFromRaw(FogDensity, Type) < glvFRACZERO)
    {
        return GL_INVALID_VALUE;
    }

    glfSetMutant(&chipCtx->fogStates.density, FogDensity, Type);
    chipCtx->fogStates.expDirty  = GL_TRUE;
    chipCtx->fogStates.exp2Dirty = GL_TRUE;
    return GL_NO_ERROR;
}

GLenum setFogStart(
    glsCHIPCONTEXT_PTR chipCtx,
    const GLvoid* FogStart,
    gleTYPE Type
    )
{
    glfSetMutant(&chipCtx->fogStates.start, FogStart, Type);
    chipCtx->fogStates.linearDirty = GL_TRUE;
    return GL_NO_ERROR;
}

GLenum setFogEnd(
    glsCHIPCONTEXT_PTR chipCtx,
    const GLvoid* FogEnd,
    gleTYPE Type
    )
{
    glfSetMutant(&chipCtx->fogStates.end, FogEnd, Type);
    chipCtx->fogStates.linearDirty = GL_TRUE;
    return GL_NO_ERROR;
}

GLenum setFogColor(
    glsCHIPCONTEXT_PTR chipCtx,
    const GLvoid* Value,
    gleTYPE Type
    )
{
    glfSetVector4(&chipCtx->fogStates.color, Value, Type);
    return GL_NO_ERROR;
}


GLenum setFog(
    glsCHIPCONTEXT_PTR chipCtx,
    GLenum Name,
    const GLvoid* Value,
    gleTYPE Type,
    gctUINT32 ValueArraySize
    )
{
    GLenum result;

    gcmHEADER_ARG("Context=0x%x Name=0x%x Value=0x%x Type=0x%x ValueArraySize=0x%x",
        chipCtx, Name, Value, Type, ValueArraySize);

    if (ValueArraySize > 1)
    {
        switch (Name)
        {
        case GL_FOG_COLOR:
            result = setFogColor(chipCtx, Value, Type);
            gcmFOOTER_ARG("return=0x%04x", result);
            return result;
        }
    }

    switch (Name)
    {
    case GL_FOG_MODE:
        result = setFogMode(chipCtx, Value, Type);
        break;

    case GL_FOG_DENSITY:
        result = setFogDensity(chipCtx, Value, Type);
        break;

    case GL_FOG_START:
        result = setFogStart(chipCtx, Value, Type);
        break;

    case GL_FOG_END:
        result = setFogEnd(chipCtx, Value, Type);
        break;
    default:
        result = GL_INVALID_ENUM;
        break;
    }

    gcmFOOTER_ARG("return=0x%04x", result);

    return result;
}





