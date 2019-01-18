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


#ifndef __gc_vsc_gl_common_lib_h_
#define __gc_vsc_gl_common_lib_h_

#define _GNU_SOURCE 1

#include "gc_vsc.h"

#if _WIN32
#include <Windows.h>
#if !defined(UNDER_CE)
# include <io.h>
#endif
#else
#ifdef __VXWORKS__
#else
#include <sys/file.h>
#endif
#include <stdio.h>
#endif

extern gctGLSLCompiler gcGLSLCompiler;


/* Builtin library for HW that can't support IMG instructions.*/
extern gcSHADER gcBuiltinLibrary0 ;
/* Builtin library for HW taht can support IMG instructions. */
extern gcSHADER gcBuiltinLibrary1 ;
extern gcSHADER gcBlendEquationLibrary ;


gceSTATUS
    gcSHADER_InitClBuiltinLibrary(
    IN gcSHADER     Shader,
    IN gctINT       ShaderType,
    IN gcLibType    LibType,
    OUT gcSHADER    *Binary,
    OUT gctSTRING   *builtinSource);

gceSTATUS
    gcSHADER_InitBuiltinLibrary(
    IN gcSHADER     Shader,
    IN gctINT       ShaderType,
    IN gcLibType    LibType,
    OUT gcSHADER    *Binary,
    OUT gctSTRING   *sloBuiltinSource
    );

gceSTATUS
    gcSHADER_WriteBuiltinLibToFile(
    IN gcSHADER    Binary,
    IN gctBOOL     isSupportImgInst,
    IN gcLibType    LibType
    );

gceSTATUS
    gcSHADER_ReadBuiltinLibFromFile(
    IN gctBOOL     isSupportImgInst,
    IN  gcLibType LibType,
    OUT gcSHADER    *Binary
    );

gceSTATUS
gcSHADER_WritePatchLibToFile(
    IN gcSHADER    Binary,
    IN gctBOOL     isSupportImgInst,
    IN gcLibType    LibType
    );

gceSTATUS
gcSHADER_ReadPatchLibFromFile(
    IN gctBOOL     isSupportImgInst,
    IN gcLibType    LibType,
    OUT gcSHADER    *Binary
    );

gceSTATUS
gcSHADER_ReadVirLibFromFile(
    IN gctSTRING virLibName,
    OUT VIR_Shader    **VirShader
    );

gceSTATUS
gcSHADER_WriteVirLibToFile(
    IN gctSTRING virLibName,
    IN VIR_Shader    *VirShader
    );



#endif

