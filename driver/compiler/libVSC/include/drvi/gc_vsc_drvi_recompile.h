/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_vsc_drvi_recompile_h_
#define __gc_vsc_drvi_recompile_h_

#define VIR_RECOMPILE           0
#if VIR_RECOMPILE
#include "gc_vsc.h"

BEGIN_EXTERN_C()

enum scheduledRecompileKind
{
    VIR_RK_PATCH_NONE = 0,
    VIR_RK_PATCH_TEXLD_FORMAT_CONVERSION,
    VIR_RK_PATCH_OUTPUT_FORMAT_CONVERSION,
    VIR_RK_PATCH_DEPTH_COMPARISON,
    VIR_RK_PATCH_CONSTANT_CONDITION,
    VIR_RK_PATCH_CONSTANT_TEXLD,
    VIR_RK_PATCH_COLOR_FACTORING,
    VIR_RK_PATCH_ALPHA_BLENDING,
    VIR_RK_PATCH_DEPTH_BIAS,
    VIR_RK_PATCH_PRE_ROTATION,
    VIR_RK_PATCH_NP2TEXTURE,
    VIR_RK_PATCH_GLOBAL_WORK_SIZE,
    VIR_RK_PATCH_READ_IMAGE,
    VIR_RK_PATCH_WRITE_IMAGE,
    VIR_RK_PATCH_Y_FLIPPED_TEXTURE,
    VIR_RK_PATCH_REMOVE_ASSIGNMENT_FOR_ALPHA,
    VIR_RK_PATCH_Y_FLIPPED_SHADER
};

typedef struct _scheduledInputConversion
{
    gctINT                  layers;       /* numberof layers the input format
                                             represented internally (up to 4) */
    gcUNIFORM               samplers[4];
    gcsSURF_FORMAT_INFO     samplerInfo;  /* */
    gctCONST_STRING         sourceFormat;
    gceTEXTURE_SWIZZLE      swizzle[gcvTEXTURE_COMPONENT_NUM];
}
scheduledInputConversion;

typedef struct _scheduledRecompileDirective * scheduledPatchDirective_PTR;
typedef struct _scheduledRecompileDirective
{
    enum gceRecompileKind   kind;
    union {
        gcsInputConversion  *     formatConversion;
        /*gcsOutputConversion *     outputConversion;
        gcsConstantCondition *    constCondition;
        gcsDepthComparison  *     depthComparison;
        gcsConstantTexld    *     constTexld;
        gcsPatchColorFactoring *  colorFactoring;
        gcsPatchAlphaBlending *   alphaBlending;
        gcsPatchDepthBias *       depthBias;
        gcsPatchNP2Texture *      np2Texture;
        gcsPatchGlobalWorkSize *  globalWorkSize;
        gcsPatchReadImage *       readImage;
        gcsPatchWriteImage *      writeImage;
        gcsPatchYFilppedTexture * yFilppedTexture;
        gcsPatchRemoveAssignmentForAlphaChannel * removeOutputAlpha;
        gcsPatchYFilppedShader *  yFilppedShader;*/
    } patchValue;
    gcPatchDirective_PTR    next;  /* pointer to next patch directive */
}
gcPatchDirective;

END_EXTERN_C()

#endif
#endif

