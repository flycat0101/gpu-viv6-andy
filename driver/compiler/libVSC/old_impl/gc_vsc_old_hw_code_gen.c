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


/*
**  Hardware dependent gcSL code generator.
*/

#include "gc_vsc.h"

#if gcdENABLE_3D
/******************************************************************************\
|***** Version Signature ******************************************************|
\******************************************************************************/
#define _gcmTXT2STR(t) #t
#define gcmTXT2STR(t) _gcmTXT2STR(t)
const char * _VSC_VERSION = "\n\0$VERSION$"
                           gcmTXT2STR(gcvVERSION_MAJOR) "."
                           gcmTXT2STR(gcvVERSION_MINOR) "."
                           gcmTXT2STR(gcvVERSION_PATCH) ":"
                           gcmTXT2STR(gcvVERSION_BUILD)
                           "$\n";

#define _HAS_GETEXPT_GETMANT_       0
#define _CLAMP_PCF_REFERENCE_       1

/* Zone used for header/footer. */
#define _GC_OBJ_ZONE    gcvZONE_HARDWARE

#define _SUPPORT_INTEGER_NEGATIVE_MODIFIER_    0

#define _DO_PACKED_MODE_LONG_ULONG     0

#define _USE_HARDWARE_NORMALIZE_MACRO_OPCODES       1
#define _USE_CONV_FOR_EXPLICIT_CONVERT_FUNCTION     1

#ifndef M_PI
#   define M_PI 3.14159265358979323846f
#endif

#define USE_VIR 1

#if USE_VIR
#define DEFINE_WITH_VIR(name) static gctBOOL name##_with_vir(\
                                            IN gcLINKTREE Tree, \
                                            IN gcsCODE_GENERATOR_PTR CodeGen, \
                                            IN gcSL_INSTRUCTION Instruction, \
                                            IN OUT gctUINT32 * States \
                                            ) \
                                            { \
                                                if(_usingVIR_Lower(Tree)) { return gcvFALSE; } \
                                                return name(Tree, CodeGen, Instruction, States); \
                                            }

#define USE_WITH_VIR(name) name##_with_vir

static gctBOOL
_usingVIR_Lower(gcLINKTREE Tree)
{
    /* NOTE!! Can not invoke VSC_OPTN_LowerM2LOptions_GetSwitchOn(lowerM2L_options). */
    return (gcmOPT_UseVIRCodeGen() != VIRCG_None) &&
        ((Tree->shader->type == gcSHADER_TYPE_VERTEX) || (Tree->shader->type == gcSHADER_TYPE_FRAGMENT));
}
#else
#define DEFINE_WITH_VIR(name)
#define USE_WITH_VIR(name) name
#endif

const gctUINT type_conv[] =
{
    0x0, /* gcSL_FLOAT     */
    0x2, /* gcSL_INTEGER   */
    0x2, /* gcSL_BOOLEAN   */
    0x5, /* gcSL_UINT32    */
    0x4, /* gcSL_INT8      */
    0x7, /* gcSL_UINT8     */
    0x3, /* gcSL_INT16     */
    0x6, /* gcSL_UINT16    */
    0xA, /* gcSL_INT64     */
    0xD, /* gcSL_UINT64    */
    0xC, /* gcSL_SNORM8    */
    0xF, /* gcSL_UNORM8    */
    0x1, /* gcSL_FLOAT16   */
    0x0, /* gcSL_FLOAT64   */
    0xB, /* gcSL_SNORM16   */
    0xE, /* gcSL_UNORM16   */
    0x0           /* gcSL_INVALID   */
};

extern gctBOOL
_GetPreviousCode(
    IN gcsCODE_GENERATOR_PTR CodeGen,
    OUT gctUINT32_PTR * Code
    );

extern gceSTATUS
_AddConstantVec1(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctFLOAT Constant,
    OUT gctINT_PTR Index,
    OUT gctUINT8_PTR Swizzle,
    OUT gcSL_TYPE *Type
    );

extern gceSTATUS
_AddConstantVec2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctFLOAT Constant1,
    IN gctFLOAT Constant2,
    OUT gctINT_PTR Index,
    OUT gctUINT8_PTR Swizzle,
    OUT gcSL_TYPE *Type
    );

extern gceSTATUS
_AddConstantVec3(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctFLOAT Constant1,
    IN gctFLOAT Constant2,
    IN gctFLOAT Constant3,
    OUT gctINT_PTR Index,
    OUT gctUINT8_PTR Swizzle,
    OUT gcSL_TYPE *Type
    );

extern gceSTATUS
_AddConstantVec4(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctFLOAT Constant1,
    IN gctFLOAT Constant2,
    IN gctFLOAT Constant3,
    IN gctFLOAT Constant4,
    OUT gctINT * Index,
    OUT gctUINT8 * Swizzle,
    OUT gcSL_TYPE *Type
    );

extern gceSTATUS
_AddConstantIVec1(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctINT Constant,
    OUT gctINT_PTR Index,
    OUT gctUINT8_PTR Swizzle,
    OUT gcSL_TYPE *Type
    );

extern gceSTATUS
_AddConstantIVec2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctUINT Constant1,
    IN gctUINT Constant2,
    OUT gctINT * Index,
    OUT gctUINT8 * Swizzle,
    OUT gcSL_TYPE *Type
    );

extern gceSTATUS
_AddConstantIVec3(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctUINT Constant1,
    IN gctUINT Constant2,
    IN gctUINT Constant3,
    OUT gctINT * Index,
    OUT gctUINT8 * Swizzle,
    OUT gcSL_TYPE *Type
    );

extern gceSTATUS
_AddConstantIVec4(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctUINT Constant1,
    IN gctUINT Constant2,
    IN gctUINT Constant3,
    IN gctUINT Constant4,
    OUT gctINT * Index,
    OUT gctUINT8 * Swizzle,
    OUT gcSL_TYPE *Type
    );

extern gceSTATUS
_UsingConstUniform(
    IN gcLINKTREE            Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctINT SourceIndex,
    IN gctINT Index,
    IN gctUINT8 Swizzle,
    IN gcSL_TYPE ConstType,
    IN OUT gctUINT32 * States
    );

extern gctUINT8
_ReplicateSwizzle(
    IN gctUINT8 Swizzle,
    IN gctUINT Index
    );

extern gctUINT8
_Enable2Swizzle(
    IN gctUINT32 Enable
    );

extern gctINT
_getEnableComponentCount(
    IN gctUINT32 Enable
    );

extern gcSL_SWIZZLE
_ExtractSwizzle(
    IN gctUINT8 Swizzle,
    IN gctUINT Index
    );

static gctBOOL
mov(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    );

static gceSTATUS
deleteCaller(
    IN gcLINKTREE    Tree,
    IN gctUINT16     CodeIndex,
    IN gctINT        CallerIndex
    )
{
    gcsCODE_CALLER_PTR caller, prevCaller;
    gcSL_INSTRUCTION code = Tree->shader->code + CodeIndex;

    /* If the target of this caller is outside the shader, skip it. */
    if (CodeIndex >= (gctUINT16)Tree->shader->codeCount)
    {
        return gcvSTATUS_OK;
    }

    if (gcmSL_OPCODE_GET(code->opcode, Opcode) != gcSL_JMP &&
        gcmSL_OPCODE_GET(code->opcode, Opcode) != gcSL_CALL)
    {
        return gcvSTATUS_OK;
    }

    caller = Tree->hints[CodeIndex].callers;
    prevCaller = caller;

    while (caller)
    {
        if (caller->caller == CallerIndex)
        {
            if (caller == Tree->hints[CodeIndex].callers)
            {
                Tree->hints[CodeIndex].callers = caller->next;
            }
            else
            {
                prevCaller->next = caller->next;
            }
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, caller));
            break;
        }
        prevCaller = caller;
        caller = caller->next;
    }
    return gcvSTATUS_OK;
}

static gctBOOL
_hasSIGN_FLOOR_CEIL(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    return CodeGen->hasSIGN_FLOOR_CEIL;
}

static gctBOOL
_hasSQRT_TRIG(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    return CodeGen->hasSQRT_TRIG;
}

static gctBOOL
_hasNEW_SIN_COS_LOG_DIV(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    return CodeGen->hasNEW_SIN_COS_LOG_DIV;
}

static gctBOOL
_hasNEW_TEXLD(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    return CodeGen->hasNEW_TEXLD;
}

static gctBOOL
_hasHalti3(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    return CodeGen->hasHalti3;
}

static gctBOOL
_hasHalti4(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    return CodeGen->hasHalti4;
}

static gctBOOL
_hasHalti4_orPerf(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    gcePATCH_ID patchID = gcvPATCH_INVALID;
    gcoHAL_GetPatchID(gcvNULL, &patchID);

    return CodeGen->hasHalti4 || !(patchID == gcvPATCH_DEQP || patchID == gcvPATCH_OESCTS);
}

static gctBOOL
_hasHalti4_and_const0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    return CodeGen->hasHalti4 && (gcmSL_SOURCE_GET(Instruction->source0, Type) == gcSL_CONSTANT);
}

static gctBOOL
_hasHalti1(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    return gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI1);
}

static gctBOOL
_hasHalti2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    return gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI2);
}

static gctBOOL
_hasNEW_TEXLD_isVertex(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    return (CodeGen->hasNEW_TEXLD && (Tree->shader->type == gcSHADER_TYPE_VERTEX));
}

static gctBOOL
_isVertex(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    return (Tree->shader->type == gcSHADER_TYPE_VERTEX);
}

static gctBOOL
_isI2I(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (CodeGen->hasSHEnhancements2)
    {
        gcSL_FORMAT format0 =
            (gcSL_FORMAT)gcmSL_TARGET_GET(Instruction->temp, Format);
        gcSL_FORMAT format1 =
            (gcSL_FORMAT)gcmSL_SOURCE_GET(Instruction->source0, Format);

        if (format0 == gcSL_FLOAT || format1 == gcSL_FLOAT ||
            format0 == gcSL_INT64 || format1 == gcSL_INT64 ||
            format0 == gcSL_UINT64 || format1 == gcSL_UINT64)
        {
            return gcvFALSE;
        }
        if(gcmSL_OPCODE_GET(Instruction->opcode, Opcode) == gcSL_CONV)
        {
            /* get the convert from type */
            gcmASSERT(gcmSL_SOURCE_GET(Instruction->source1, Type) == gcSL_CONSTANT);
            /* Assemble the 32-bit value. */
            gcmASSERT(gcmSL_SOURCE_GET(Instruction->source1, Format) == gcSL_UINT32);
            format1 = (gcSL_FORMAT)(Instruction->source1Index |
                                    Instruction->source1Indexed << 16);
        }

        if (format0 != format1)
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

void
_SetValueType0(
    IN gctUINT ValueType,
    IN OUT gctUINT32 * States
    );

gctBOOL
value_type0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    );

#if _SUPPORT_LONG_ULONG_DATA_TYPE && _DO_PACKED_MODE_LONG_ULONG
#define _gcdLongUlongComponentByteOffset(Component) ((Component) << 3)

#define _gcdLongUlongSingleEnableByteOffset(Enable)  \
   (((Enable) & gcSL_ENABLE_W) ? _gcdLongUlongComponentByteOffset(gcSL_COMPONENT_W) \
                               : _gcdLongUlongComponentByteOffset((Enable) >> 1))

#define _gcdLongUlongSingleEnableTempRegOffset(Enable)  \
   (((Enable) & gcSL_ENABLE_ZW ) ? 1 : 0)

static gctINT
_getCL_Long_ulong_store_count(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction
    )
{
    gctINT storeCount = 0;

    if (CodeGen->clShader && !CodeGen->hasLongUlong)
    {
        gcSL_SWIZZLE swizzleX;
        gcSL_SWIZZLE swizzleY;
        gcSL_SWIZZLE swizzleZ;
        gcSL_SWIZZLE swizzleW;
        gctINT componentSpan = 0;

        gcSL_FORMAT format = gcmSL_TARGET_GET(Instruction->temp, Format);

        if(format != gcSL_INT64 && format != gcSL_UINT64) return 0;

        switch(gcmSL_TARGET_GET(Instruction->temp, Enable))
        {
        case gcSL_ENABLE_X:
        case gcSL_ENABLE_Y:
        case gcSL_ENABLE_Z:
        case gcSL_ENABLE_W:
            storeCount = 1;
            break;

        case gcSL_ENABLE_XY:
            swizzleX = gcmSL_SOURCE_GET(Instruction->source1, SwizzleX);
            swizzleY = gcmSL_SOURCE_GET(Instruction->source1, SwizzleY);
            switch(swizzleX)
            {
            case gcSL_SWIZZLE_X:
            case gcSL_SWIZZLE_Y:
                if(swizzleY == gcSL_SWIZZLE_X ||
                   swizzleY == gcSL_SWIZZLE_Y)
                {
                    storeCount = 1;
                }
                else
                {
                    storeCount = 2;
                }
                break;

           case gcSL_SWIZZLE_Z:
           case gcSL_SWIZZLE_W:
                if(swizzleY == gcSL_SWIZZLE_Z ||
                   swizzleY == gcSL_SWIZZLE_W)
                {
                    storeCount = 1;
                }
                else
                {
                    storeCount = 2;
                }
                break;
            default:
                gcmASSERT(0);
                break;
            }
            break;

        case gcSL_ENABLE_XZ:
        case gcSL_ENABLE_XW:
        case gcSL_ENABLE_YZ:
        case gcSL_ENABLE_YW:
            storeCount = 2;
            break;

        case gcSL_ENABLE_XYZ:
            swizzleX = gcmSL_SOURCE_GET(Instruction->source1, SwizzleX);
            swizzleY = gcmSL_SOURCE_GET(Instruction->source1, SwizzleY);
            swizzleZ = gcmSL_SOURCE_GET(Instruction->source1, SwizzleZ);
            storeCount = 2;
            switch(swizzleX)
            {
            case gcSL_SWIZZLE_X:
            case gcSL_SWIZZLE_Y:
                if(swizzleY == gcSL_SWIZZLE_X ||
                   swizzleY == gcSL_SWIZZLE_Y)
                {
                    break;
                }
                else
                {
                    if(swizzleY == gcSL_SWIZZLE_Y) swizzleY--;
                    componentSpan = swizzleY - swizzleZ;
                    componentSpan = (componentSpan < 0) ? -componentSpan : componentSpan;
                    if(componentSpan > 1)
                    {
                        storeCount++;
                    }
                }
                break;

           case gcSL_SWIZZLE_Z:
           case gcSL_SWIZZLE_W:
                if(swizzleY == gcSL_SWIZZLE_Z ||
                   swizzleY == gcSL_SWIZZLE_W)
                {
                    break;
                }
                else
                {
                    if(swizzleY == gcSL_SWIZZLE_Y) swizzleY--;
                    componentSpan = swizzleY - swizzleZ;
                    componentSpan = (componentSpan < 0) ? -componentSpan : componentSpan;
                    if(componentSpan > 1)
                    {
                        storeCount++;
                    }
                }
                break;

            default:
                gcmASSERT(0);
                break;
            }
            break;

        case gcSL_ENABLE_XYZW:
            swizzleX = gcmSL_SOURCE_GET(Instruction->source1, SwizzleX);
            swizzleY = gcmSL_SOURCE_GET(Instruction->source1, SwizzleY);
            swizzleZ = gcmSL_SOURCE_GET(Instruction->source1, SwizzleZ);
            swizzleW = gcmSL_SOURCE_GET(Instruction->source1, SwizzleW);
            storeCount = 2;
            switch(swizzleX)
            {
            case gcSL_SWIZZLE_X:
            case gcSL_SWIZZLE_Y:
                if(swizzleY == gcSL_SWIZZLE_X ||
                   swizzleY == gcSL_SWIZZLE_Y)
                {
                    componentSpan = swizzleZ - swizzleW;
                    componentSpan = (componentSpan < 0) ? -componentSpan : componentSpan;
                    if(componentSpan > 1)
                    {
                        storeCount++;
                    }
                }
                else
                {
                    storeCount++;
                    if(swizzleY == gcSL_SWIZZLE_Y) swizzleY--;
                    componentSpan = swizzleY - swizzleZ;
                    componentSpan = (componentSpan < 0) ? -componentSpan : componentSpan;
                    if(componentSpan > 1)
                    {
                        componentSpan = swizzleZ - swizzleW;
                        componentSpan = (componentSpan < 0) ? -componentSpan : componentSpan;
                        if(componentSpan > 1)
                        {
                            storeCount++;
                        }
                    }
                }
                break;

           case gcSL_SWIZZLE_Z:
           case gcSL_SWIZZLE_W:
                if(swizzleY == gcSL_SWIZZLE_Z ||
                   swizzleY == gcSL_SWIZZLE_W)
                {
                    componentSpan = swizzleZ - swizzleW;
                    componentSpan = (componentSpan < 0) ? -componentSpan : componentSpan;
                    if(componentSpan > 1)
                    {
                        storeCount++;
                    }
                }
                else
                {
                    storeCount++;
                    if(swizzleY == gcSL_SWIZZLE_Y) swizzleY--;
                    componentSpan = swizzleY - swizzleZ;
                    componentSpan = (componentSpan < 0) ? -componentSpan : componentSpan;
                    if(componentSpan > 1)
                    {
                        componentSpan = swizzleZ - swizzleW;
                        componentSpan = (componentSpan < 0) ? -componentSpan : componentSpan;
                        if(componentSpan > 1)
                        {
                            storeCount++;
                        }
                    }
                }
                break;

            default:
                gcmASSERT(0);
                break;
            }
            break;

        case gcSL_ENABLE_YZW:
            swizzleX = gcmSL_SOURCE_GET(Instruction->source1, SwizzleX);
            swizzleY = gcmSL_SOURCE_GET(Instruction->source1, SwizzleY);
            swizzleZ = gcmSL_SOURCE_GET(Instruction->source1, SwizzleZ);
            storeCount = 2;
            switch(swizzleX)
            {
            case gcSL_SWIZZLE_X:
            case gcSL_SWIZZLE_Y:
                if(swizzleY == gcSL_SWIZZLE_X ||
                   swizzleY == gcSL_SWIZZLE_Y)
                {
                    break;
                }
                else {
                    if(swizzleY == gcSL_SWIZZLE_Y) swizzleY--;
                    componentSpan = swizzleY - swizzleZ;
                    componentSpan = (componentSpan < 0) ? -componentSpan : componentSpan;
                    if(componentSpan > 1)
                    {
                        storeCount++;
                    }
                }
                break;

           case gcSL_SWIZZLE_Z:
           case gcSL_SWIZZLE_W:
                if(swizzleY == gcSL_SWIZZLE_Z ||
                   swizzleY == gcSL_SWIZZLE_W)
                {
                    break;
                }
                else {
                    if(swizzleY == gcSL_SWIZZLE_Y) swizzleY--;
                    componentSpan = swizzleY - swizzleZ;
                    componentSpan = (componentSpan < 0) ? -componentSpan : componentSpan;
                    if(componentSpan > 1)
                    {
                        storeCount++;
                    }
                }
                break;

            default:
                gcmASSERT(0);
                break;
            }
            break;

        case gcSL_ENABLE_XYW:
            swizzleX = gcmSL_SOURCE_GET(Instruction->source1, SwizzleX);
            swizzleY = gcmSL_SOURCE_GET(Instruction->source1, SwizzleY);
            swizzleZ = gcmSL_SOURCE_GET(Instruction->source1, SwizzleZ);
            storeCount = 2;
            switch(swizzleX)
            {
            case gcSL_SWIZZLE_X:
            case gcSL_SWIZZLE_Y:
                if(swizzleY == gcSL_SWIZZLE_X ||
                   swizzleY == gcSL_SWIZZLE_Y)
                {
                    break;
                }
                else
                {
                    storeCount++;
                }
                break;

           case gcSL_SWIZZLE_Z:
           case gcSL_SWIZZLE_W:
                if(swizzleY == gcSL_SWIZZLE_Z ||
                   swizzleY == gcSL_SWIZZLE_W)
                {
                    break;
                }
                else
                {
                    storeCount++;
                }
                break;

            default:
                gcmASSERT(0);
                break;
            }
            break;

        case gcSL_ENABLE_XZW:
            swizzleY = gcmSL_SOURCE_GET(Instruction->source1, SwizzleY);
            swizzleZ = gcmSL_SOURCE_GET(Instruction->source1, SwizzleZ);
            storeCount = 2;
            switch(swizzleY)
            {
            case gcSL_SWIZZLE_X:
            case gcSL_SWIZZLE_Y:
                if(swizzleZ == gcSL_SWIZZLE_X ||
                   swizzleZ == gcSL_SWIZZLE_Y)
                {
                    break;
                }
                else
                {
                    storeCount++;
                }
                break;

           case gcSL_SWIZZLE_Z:
           case gcSL_SWIZZLE_W:
                if(swizzleZ == gcSL_SWIZZLE_Z ||
                   swizzleZ == gcSL_SWIZZLE_W)
                {
                    break;
                }
                else
                {
                    storeCount++;
                }
                break;

            default:
                gcmASSERT(0);
                break;
            }
            break;

        default:
            gcmASSERT(0);
            break;
        }
    }

    return storeCount;
}

static gctBOOL
_isCL_Long_ulong_1_store(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    return (_getCL_Long_ulong_store_count(Tree, CodeGen, Instruction) == 1);
}

static gctBOOL
_isCL_Long_ulong_3_store(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    return (_getCL_Long_ulong_store_count(Tree, CodeGen, Instruction) == 3);
}

static gctBOOL
_isCL_Long_ulong_4_store(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    return (_getCL_Long_ulong_store_count(Tree, CodeGen, Instruction) == 4);
}

static gcSL_SWIZZLE
_longUlongOneComponentSwizzleMap[4] =
{
    gcSL_SWIZZLE_XYXY, /* x */
    gcSL_SWIZZLE_ZWZW, /* y */
    gcSL_SWIZZLE_XYXY, /* z */
    gcSL_SWIZZLE_ZWZW    /* w */
};

static gcSL_SWIZZLE
_longUlongTwoComponentSwizzleMap[16] =
{
    gcmSWIZZLE(X, Y, X, Y), /* xx */
    gcmSWIZZLE(X, Y, Z, W), /* xy */
    gcmSWIZZLE(X, Y, X, Y), /* xz - invalid combination: behave like x */
    gcmSWIZZLE(X, Y, X, Y), /* xw - invalid combination: behave like x */
    gcmSWIZZLE(Z, W, X, Y), /* yx */
    gcmSWIZZLE(Z, W, Z, W), /* yy */
    gcmSWIZZLE(Z, W, Z, W), /* yz - invalid combination: behave like y */
    gcmSWIZZLE(Z, W, Z, W), /* yw - invalid combination: behave like y */
    gcmSWIZZLE(X, Y, X, Y), /* zx - invalid combination: behave like z */
    gcmSWIZZLE(X, Y, X, Y), /* zy - invalid combination: behave like z */
    gcmSWIZZLE(X, Y, X, Y), /* zz */
    gcmSWIZZLE(X, Y, Z, W), /* zw */
    gcmSWIZZLE(Z, W, Z, W), /* wx - invalid combination: behave like w */
    gcmSWIZZLE(Z, W, Z, W), /* wy - invalid combination: behave like w */
    gcmSWIZZLE(Z, W, X, Y), /* wz */
    gcmSWIZZLE(Z, W, Z, W)    /* ww */
};

#define _gcdLongUlongStoreOneComponentEnable   gcSL_ENABLE_XY
#define _gcdLongUlongStoreTwoComponentEnable   gcSL_ENABLE_XYZW

static gctBOOL
long_ulong_first_store(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_SWIZZLE swizzleX;
    gcSL_SWIZZLE swizzleY;
    gctINT index = 0;
    gctINT tempRegOffset = 0;
    gctUINT8 swizzle = gcSL_SWIZZLE_XYXY;
    gcSL_TYPE constType;
    gcSL_FORMAT format;
    gcSL_ENABLE orgEnable;
    gcSL_ENABLE enable = _gcdLongUlongStoreOneComponentEnable;

    format = gcmSL_TARGET_GET(Instruction->temp, Format);
    if(format == gcSL_UINT64) format = gcSL_UINT32;
    else format = gcSL_INT32;

    _SetValueType0(type_conv[format], States);

    orgEnable = gcmSL_TARGET_GET(Instruction->temp, Enable);
    switch(orgEnable)
    {
    case gcSL_ENABLE_X:
    case gcSL_ENABLE_Y:
    case gcSL_ENABLE_Z:
    case gcSL_ENABLE_W:
        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                       CodeGen,
                                       _gcdLongUlongSingleEnableByteOffset(enable),
                                       &index,
                                       &swizzle,
                                       &constType));
        _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);
        enable = _gcdLongUlongStoreOneComponentEnable;
        tempRegOffset = _gcdLongUlongSingleEnableTempRegOffset(orgEnable);

        /* swizzleX */
        swizzleX = gcmSL_SOURCE_GET(Instruction->source1, SwizzleX);
        swizzle = _longUlongOneComponentSwizzleMap[swizzleX];
        break;

    case gcSL_ENABLE_XY:
    case gcSL_ENABLE_XYZ:
    case gcSL_ENABLE_XYW:
    case gcSL_ENABLE_XYZW:
    case gcSL_ENABLE_YZ:
    case gcSL_ENABLE_YZW:
    case gcSL_ENABLE_ZW:
        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                       CodeGen,
                                       ((orgEnable & gcSL_ENABLE_X)
                                           ?  _gcdLongUlongSingleEnableByteOffset(gcSL_ENABLE_X)
                                           : ((orgEnable == gcSL_ENABLE_ZW)
                                                ? _gcdLongUlongSingleEnableByteOffset(gcSL_ENABLE_Z)
                                                : _gcdLongUlongSingleEnableByteOffset(gcSL_ENABLE_Y))),
                                       &index,
                                       &swizzle,
                                       &constType));
        _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);
        tempRegOffset = (orgEnable & gcSL_ENABLE_X)
                            ? _gcdLongUlongSingleEnableTempRegOffset(gcSL_ENABLE_X)
                            : ((orgEnable == gcSL_ENABLE_ZW)
                                ? _gcdLongUlongSingleEnableByteOffset(gcSL_ENABLE_Z)
                                : _gcdLongUlongSingleEnableByteOffset(gcSL_ENABLE_Y));

        swizzleX = gcmSL_SOURCE_GET(Instruction->source1, SwizzleX);
        swizzleY = gcmSL_SOURCE_GET(Instruction->source1, SwizzleY);
        switch(swizzleX)
        {
        case gcSL_SWIZZLE_X:
        case gcSL_SWIZZLE_Y:
            if(swizzleY == gcSL_SWIZZLE_X ||
               swizzleY == gcSL_SWIZZLE_Y)
            {
                enable = _gcdLongUlongStoreTwoComponentEnable;
                /* swizzleX and swizzleY */
                swizzle = (gctUINT8) gcmComposeSwizzle(swizzleX, swizzleY, 0, 0);
                swizzle = _longUlongTwoComponentSwizzleMap[swizzle];
            }
            else
            {
                enable = _gcdLongUlongStoreOneComponentEnable;
                /* swizzleX */
                swizzle = _longUlongOneComponentSwizzleMap[swizzleX];
            }
            break;

       case gcSL_SWIZZLE_Z:
       case gcSL_SWIZZLE_W:
            if(swizzleY == gcSL_SWIZZLE_Z ||
               swizzleY == gcSL_SWIZZLE_W)
            {
                enable = _gcdLongUlongStoreTwoComponentEnable;
                /* swizzleX and swizzleY */
                swizzle = (gctUINT8)gcmComposeSwizzle(swizzleX, swizzleY, 0, 0);
                swizzle = _longUlongTwoComponentSwizzleMap[swizzle];
            }
            else
            {
                enable = _gcdLongUlongStoreOneComponentEnable;
                /* swizzleX */
                swizzle = _longUlongOneComponentSwizzleMap[swizzleX];
            }
            break;

        default:
            gcmASSERT(0);
            break;
        }
        break;

    case gcSL_ENABLE_XZ:
    case gcSL_ENABLE_XW:
    case gcSL_ENABLE_XZW:
    case gcSL_ENABLE_YW:
        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                       CodeGen,
                                       _gcdLongUlongSingleEnableByteOffset(orgEnable & gcSL_ENABLE_XY),
                                       &index,
                                       &swizzle,
                                       &constType));
        _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);
        enable = _gcdLongUlongStoreOneComponentEnable;
        tempRegOffset = _gcdLongUlongSingleEnableByteOffset(orgEnable & gcSL_ENABLE_XY),

        /* first component; swizzleX */
        swizzleX = gcmSL_SOURCE_GET(Instruction->source1, SwizzleX);
        swizzle = _longUlongOneComponentSwizzleMap[swizzleX];
        break;

    default:
        gcmASSERT(0);
        break;
    }

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable) & ((gctUINT32) ((((1 ? 26:23) - (0 ?
 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));
    if (CodeGen->hasSHEnhancements2)
    {
        swizzle = (((((gctUINT32) (States[1])) >> (0 ? 10:3)) & ((gctUINT32) ((((1 ? 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1)))))) );
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3))) | (((gctUINT32) ((gctUINT32) (swizzle | 0x80) & ((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3)));
    }
    if(tempRegOffset) {
        gctUINT address;

        switch(gcmSL_SOURCE_GET(Instruction->source1, Type))
        {
        case gcSL_TEMP:
            address = (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ) + 1;
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ? 12:4) - (0 ?
 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4)));
            break;

        case gcSL_UNIFORM:
            address = (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ) + 1;
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ? 12:4) - (0 ?
 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4)));
            break;

        case gcSL_CONSTANT:
            break;

        default:
            break;
        }
    }

    return gcvTRUE;
}

static gctBOOL
long_ulong_second_store(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_SWIZZLE swizzleX;
    gcSL_SWIZZLE swizzleY;
    gcSL_SWIZZLE swizzleZ;
    gcSL_SWIZZLE swizzleW;
    gctINT componentSpan = 0;
    gctUINT8 swizzle = 0;
    gcSL_FORMAT format;
    gcSL_ENABLE enable;

    format = gcmSL_TARGET_GET(Instruction->temp, Format);
    if(format == gcSL_UINT64) format = gcSL_UINT32;
    else format = gcSL_INT32;

    _SetValueType0(type_conv[format], States);

    enable = gcmSL_TARGET_GET(Instruction->temp, Enable);
    switch(enable)
    {
    case gcSL_ENABLE_XY:
    case gcSL_ENABLE_XZ:
    case gcSL_ENABLE_XW:
        swizzleY = gcmSL_SOURCE_GET(Instruction->source1, SwizzleY);
        /* swizzleY */
        break;

    case gcSL_ENABLE_YZ:
    case gcSL_ENABLE_YW:
        swizzleY = gcmSL_SOURCE_GET(Instruction->source1, SwizzleY);
        /* swizzleY */
        break;

    case gcSL_ENABLE_XYZ:
        swizzleX = gcmSL_SOURCE_GET(Instruction->source1, SwizzleX);
        swizzleY = gcmSL_SOURCE_GET(Instruction->source1, SwizzleY);
        swizzleZ = gcmSL_SOURCE_GET(Instruction->source1, SwizzleZ);
        switch(swizzleX)
        {
        case gcSL_SWIZZLE_X:
        case gcSL_SWIZZLE_Y:
            if(swizzleY == gcSL_SWIZZLE_X ||
               swizzleY == gcSL_SWIZZLE_Y)
            {
                /* swizzleZ */
            }
            else
            {
                swizzle = swizzleY;
                if(swizzle == gcSL_SWIZZLE_Y) swizzle--;
                componentSpan = swizzle - swizzleZ;
                componentSpan = (componentSpan < 0) ? -componentSpan : componentSpan;
                if(componentSpan > 1)
                {
                    /* swizzleY */
                }
                else
                {
                    /* swizzleY and swizzleZ */
                }
            }
            break;

       case gcSL_SWIZZLE_Z:
       case gcSL_SWIZZLE_W:
            if(swizzleY == gcSL_SWIZZLE_Z ||
               swizzleY == gcSL_SWIZZLE_W)
            {
                /* swizzleZ */
            }
            else
            {
                swizzle = swizzleY;
                if(swizzle == gcSL_SWIZZLE_Y) swizzle--;
                componentSpan = swizzle - swizzleZ;
                componentSpan = (componentSpan < 0) ? -componentSpan : componentSpan;
                if(componentSpan > 1)
                {
                    /* swizzleY */
                }
                else
                {
                    /* swizzleY and swizzleZ */
                }
            }
            break;

        default:
            gcmASSERT(0);
            break;
        }
        break;

    case gcSL_ENABLE_XYZW:
        swizzleX = gcmSL_SOURCE_GET(Instruction->source1, SwizzleX);
        swizzleY = gcmSL_SOURCE_GET(Instruction->source1, SwizzleY);
        swizzleZ = gcmSL_SOURCE_GET(Instruction->source1, SwizzleZ);
        swizzleW = gcmSL_SOURCE_GET(Instruction->source1, SwizzleW);
        switch(swizzleX)
        {
        case gcSL_SWIZZLE_X:
        case gcSL_SWIZZLE_Y:
            if(swizzleY == gcSL_SWIZZLE_X ||
               swizzleY == gcSL_SWIZZLE_Y)
            {
                componentSpan = swizzleZ - swizzleW;
                componentSpan = (componentSpan < 0) ? -componentSpan : componentSpan;
                if(componentSpan > 1)
                {
                    /* swizzleZ */
                }
                else
                {
                    /* swizzleZ and swizzleW */
                }
            }
            else
            {
                swizzle = swizzleY;
                if(swizzle == gcSL_SWIZZLE_Y) swizzle--;
                componentSpan = swizzle - swizzleZ;
                componentSpan = (componentSpan < 0) ? -componentSpan : componentSpan;
                if(componentSpan > 1)
                {
                    /* swizzleY */
                }
                else
                {
                    /* swizzleY and swizzleZ */
                }
            }
            break;

       case gcSL_SWIZZLE_Z:
       case gcSL_SWIZZLE_W:
            if(swizzleY == gcSL_SWIZZLE_Z ||
               swizzleY == gcSL_SWIZZLE_W)
            {
                componentSpan = swizzleZ - swizzleW;
                componentSpan = (componentSpan < 0) ? -componentSpan : componentSpan;
                if(componentSpan > 1)
                {
                    /* swizzleZ */
                }
                else
                {
                    /* swizzleZ and swizzleW */
                }
            }
            else
            {
                swizzle = swizzleY;
                if(swizzle == gcSL_SWIZZLE_Y) swizzle--;
                componentSpan = swizzle - swizzleZ;
                componentSpan = (componentSpan < 0) ? -componentSpan : componentSpan;
                if(componentSpan > 1)
                {
                    /* swizzleY */
                }
                else
                {
                    /* swizzleY and swizzleZ */
                }
            }
            break;

        default:
            gcmASSERT(0);
            break;
        }
        break;

    case gcSL_ENABLE_YZW:
        swizzleX = gcmSL_SOURCE_GET(Instruction->source1, SwizzleX);
        swizzleY = gcmSL_SOURCE_GET(Instruction->source1, SwizzleY);
        swizzleZ = gcmSL_SOURCE_GET(Instruction->source1, SwizzleZ);
        switch(swizzleX)
        {
        case gcSL_SWIZZLE_X:
        case gcSL_SWIZZLE_Y:
            if(swizzleY == gcSL_SWIZZLE_X ||
               swizzleY == gcSL_SWIZZLE_Y)
            {
                /* swizzleZ */
            }
            else {
                swizzle = swizzleY;
                if(swizzle == gcSL_SWIZZLE_Y) swizzle--;
                componentSpan = swizzle - swizzleZ;
                componentSpan = (componentSpan < 0) ? -componentSpan : componentSpan;
                if(componentSpan > 1)
                {
                    /* swizzleY */
                }
                else
                {
                    /* swizzleY and swizzleZ */
                }
            }
            break;

       case gcSL_SWIZZLE_Z:
       case gcSL_SWIZZLE_W:
            if(swizzleY == gcSL_SWIZZLE_Z ||
               swizzleY == gcSL_SWIZZLE_W)
            {
                /* swizzleZ */
            }
            else {
                swizzle = swizzleY;
                if(swizzle == gcSL_SWIZZLE_Y) swizzle--;
                componentSpan = swizzle - swizzleZ;
                componentSpan = (componentSpan < 0) ? -componentSpan : componentSpan;
                if(componentSpan > 1)
                {
                    /* swizzleY */
                }
                else
                {
                    /* swizzleY and swizzleZ */
                }
            }
            break;

        default:
            gcmASSERT(0);
            break;
        }
        break;

    case gcSL_ENABLE_XYW:
        swizzleX = gcmSL_SOURCE_GET(Instruction->source1, SwizzleX);
        swizzleY = gcmSL_SOURCE_GET(Instruction->source1, SwizzleY);
        swizzleZ = gcmSL_SOURCE_GET(Instruction->source1, SwizzleZ);
        switch(swizzleX)
        {
        case gcSL_SWIZZLE_X:
        case gcSL_SWIZZLE_Y:
            if(swizzleY == gcSL_SWIZZLE_X ||
               swizzleY == gcSL_SWIZZLE_Y)
            {
                /* swizzleZ */
            }
            else
            {
                /* swizzleY */
            }
            break;

       case gcSL_SWIZZLE_Z:
       case gcSL_SWIZZLE_W:
            if(swizzleY == gcSL_SWIZZLE_Z ||
               swizzleY == gcSL_SWIZZLE_W)
            {
                /* swizzleZ */
            }
            else
            {
                /* swizzleY */
            }
            break;

        default:
            gcmASSERT(0);
            break;
        }
        break;

    case gcSL_ENABLE_XZW:
        swizzleY = gcmSL_SOURCE_GET(Instruction->source1, SwizzleY);
        swizzleZ = gcmSL_SOURCE_GET(Instruction->source1, SwizzleZ);
        switch(swizzleY)
        {
        case gcSL_SWIZZLE_X:
        case gcSL_SWIZZLE_Y:
            if(swizzleZ == gcSL_SWIZZLE_X ||
               swizzleZ == gcSL_SWIZZLE_Y)
            {
                /* swizzleY and swizzleZ */
            }
            else
            {
                /* swizzleY */
            }
            break;

       case gcSL_SWIZZLE_Z:
       case gcSL_SWIZZLE_W:
            if(swizzleZ == gcSL_SWIZZLE_Z ||
               swizzleZ == gcSL_SWIZZLE_W)
            {
                /* swizzleY and swizzleZ */
            }
            else
            {
                /* swizzleY */
            }
            break;

        default:
            gcmASSERT(0);
            break;
        }
        break;

    default:
        gcmASSERT(0);
        break;
    }

    return gcvTRUE;
}

static gctBOOL
long_ulong_third_store(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_SWIZZLE swizzleX;
    gcSL_SWIZZLE swizzleY;
    gcSL_SWIZZLE swizzleZ;
    gcSL_SWIZZLE swizzleW;
    gctINT componentSpan = 0;
    gctUINT8 swizzle = 0;
    gcSL_FORMAT format;
    gcSL_ENABLE enable;

    format = gcmSL_TARGET_GET(Instruction->temp, Format);
    if(format == gcSL_UINT64) format = gcSL_UINT32;
    else format = gcSL_INT32;

    _SetValueType0(type_conv[format], States);

    enable = gcmSL_TARGET_GET(Instruction->temp, Enable);
    switch(enable)
    {
    case gcSL_ENABLE_XYZ:
        swizzleZ = gcmSL_SOURCE_GET(Instruction->source1, SwizzleZ);
        /* swizzleZ */
        break;

    case gcSL_ENABLE_XYZW:
        swizzleX = gcmSL_SOURCE_GET(Instruction->source1, SwizzleX);
        swizzleY = gcmSL_SOURCE_GET(Instruction->source1, SwizzleY);
        swizzleZ = gcmSL_SOURCE_GET(Instruction->source1, SwizzleZ);
        swizzleW = gcmSL_SOURCE_GET(Instruction->source1, SwizzleW);
        switch(swizzleX)
        {
        case gcSL_SWIZZLE_X:
        case gcSL_SWIZZLE_Y:
            if(swizzleY == gcSL_SWIZZLE_X ||
               swizzleY == gcSL_SWIZZLE_Y)
            {
                componentSpan = swizzleZ - swizzleW;
                componentSpan = (componentSpan < 0) ? -componentSpan : componentSpan;
                if(componentSpan > 1)
                {
                    /* swizzleZ */
                }
                else
                {
                    /* swizzleZ and swizzleW */
                }
            }
            else
            {
                swizzle = swizzleY;
                if(swizzle == gcSL_SWIZZLE_Y) swizzle--;
                componentSpan = swizzle - swizzleZ;
                componentSpan = (componentSpan < 0) ? -componentSpan : componentSpan;
                if(componentSpan > 1)
                {
                    componentSpan = swizzleZ - swizzleW;
                    componentSpan = (componentSpan < 0) ? -componentSpan : componentSpan;
                    if(componentSpan > 1)
                    {
                        /* swizzleZ */
                    }
                    else
                    {
                        /* swizzleZ and swizzleW */
                    }
                }
                else
                {
                    /* swizzleW */
                }
            }
            break;

       case gcSL_SWIZZLE_Z:
       case gcSL_SWIZZLE_W:
            if(swizzleY == gcSL_SWIZZLE_Z ||
               swizzleY == gcSL_SWIZZLE_W)
            {
                componentSpan = swizzleZ - swizzleW;
                componentSpan = (componentSpan < 0) ? -componentSpan : componentSpan;
                if(componentSpan > 1)
                {
                    /* swizzleZ */
                }
                else
                {
                    /* swizzleZ and swizzleW */
                }
            }
            else
            {
                swizzle = swizzleY;
                if(swizzle == gcSL_SWIZZLE_Y) swizzle--;
                componentSpan = swizzle - swizzleZ;
                componentSpan = (componentSpan < 0) ? -componentSpan : componentSpan;
                if(componentSpan > 1)
                {
                    componentSpan = swizzleZ - swizzleW;
                    componentSpan = (componentSpan < 0) ? -componentSpan : componentSpan;
                    if(componentSpan > 1)
                    {
                        /* swizzleZ */
                    }
                    else
                    {
                        /* swizzleZ and swizzleW */
                    }
                }
                else
                {
                    /* swizzleW */
                }
            }
            break;

        default:
            gcmASSERT(0);
            break;
        }
        break;

    case gcSL_ENABLE_YZW:
        swizzleZ = gcmSL_SOURCE_GET(Instruction->source1, SwizzleZ);
        /* swizzleZ */
        break;

    case gcSL_ENABLE_XYW:
        swizzleX = gcmSL_SOURCE_GET(Instruction->source1, SwizzleX);
        swizzleY = gcmSL_SOURCE_GET(Instruction->source1, SwizzleY);
        swizzleZ = gcmSL_SOURCE_GET(Instruction->source1, SwizzleZ);
        switch(swizzleX)
        {
        case gcSL_SWIZZLE_X:
        case gcSL_SWIZZLE_Y:
            if(swizzleY == gcSL_SWIZZLE_X ||
               swizzleY == gcSL_SWIZZLE_Y)
            {
                gcmASSERT(0);
            }
            else
            {
                /* swizzleZ */
            }
            break;

       case gcSL_SWIZZLE_Z:
       case gcSL_SWIZZLE_W:
            if(swizzleY == gcSL_SWIZZLE_Z ||
               swizzleY == gcSL_SWIZZLE_W)
            {
                gcmASSERT(0);
            }
            else
            {
                /* swizzleZ */
            }
            break;

        default:
            gcmASSERT(0);
            break;
        }
        break;

    case gcSL_ENABLE_XZW:
        swizzleY = gcmSL_SOURCE_GET(Instruction->source1, SwizzleY);
        swizzleZ = gcmSL_SOURCE_GET(Instruction->source1, SwizzleZ);
        switch(swizzleY)
        {
        case gcSL_SWIZZLE_X:
        case gcSL_SWIZZLE_Y:
            if(swizzleZ == gcSL_SWIZZLE_X ||
               swizzleZ == gcSL_SWIZZLE_Y)
            {
                gcmASSERT(0);
            }
            else
            {
                /* swizzleZ */
            }
            break;

       case gcSL_SWIZZLE_Z:
       case gcSL_SWIZZLE_W:
            if(swizzleZ == gcSL_SWIZZLE_Z ||
               swizzleZ == gcSL_SWIZZLE_W)
            {

                gcmASSERT(0);
            }
            else
            {
                /* swizzleZ */
            }
            break;

        default:
            gcmASSERT(0);
            break;
        }
        break;

    default:
        gcmASSERT(0);
        break;
    }

    return gcvTRUE;
}

static gctBOOL
long_ulong_fourth_store(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_SWIZZLE swizzleW;
    gctUINT8 swizzle = 0;
    gcSL_FORMAT format;

    gcmASSERT(gcmSL_TARGET_GET(Instruction->temp, Enable) == gcSL_ENABLE_XYZW);
    format = gcmSL_TARGET_GET(Instruction->temp, Format);
    if(format == gcSL_UINT64) format = gcSL_UINT32;
    else format = gcSL_INT32;

    _SetValueType0(type_conv[format], States);

    swizzleW = gcmSL_SOURCE_GET(Instruction->source1, SwizzleW);
    /* swizzleW */

    if (CodeGen->hasSHEnhancements2)
    {
        swizzle = (((((gctUINT32) (States[1])) >> (0 ? 10:3)) & ((gctUINT32) ((((1 ? 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1)))))) );
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3))) | (((gctUINT32) ((gctUINT32) (swizzle | 0x80) & ((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3)));
    }
    return gcvTRUE;
}
#else
#define _gcdLongUlongStoreOneComponentEnable   gcSL_ENABLE_X
#define _gcdLongUlongStoreTwoComponentEnable   gcSL_ENABLE_XZ

#define _gcdLongUlongComponentByteOffset(Component) ((Component) << 3)

#define _gcdLongUlongSingleEnableByteOffset(Enable)  \
   (((Enable) & gcSL_ENABLE_W) ? _gcdLongUlongComponentByteOffset(gcSL_COMPONENT_W) \
                               : _gcdLongUlongComponentByteOffset((Enable) >> 1))


static gcSL_SWIZZLE
_longUlongOneComponentSwizzleMap[4] =
{
    gcSL_SWIZZLE_XXXX, /* x */
    gcSL_SWIZZLE_YYYY, /* y */
    gcSL_SWIZZLE_ZZZZ, /* z */
    gcSL_SWIZZLE_WWWW    /* w */
};

static gcSL_SWIZZLE
_longUlongTwoComponentSwizzleMap[16] =
{
    gcmSWIZZLE(X, X, X, X), /* xx */
    gcmSWIZZLE(Y, Y, X, X), /* yx */
    gcmSWIZZLE(Z, Z, X, X), /* zx */
    gcmSWIZZLE(W, W, X, X), /* wx */
    gcmSWIZZLE(X, X, Y, Y), /* xy */
    gcmSWIZZLE(Y, Y, Y, Y), /* yy */
    gcmSWIZZLE(Z, Z, Y, Y), /* zy */
    gcmSWIZZLE(W, W, Y, Y), /* wy */
    gcmSWIZZLE(X, X, Z, Z), /* xz */
    gcmSWIZZLE(Y, Y, Z, Z), /* yz */
    gcmSWIZZLE(Z, Z, Z, Z), /* zz */
    gcmSWIZZLE(W, W, Z, Z), /* wz */
    gcmSWIZZLE(X, X, W, W), /* xw */
    gcmSWIZZLE(Y, Y, W, W), /* yw */
    gcmSWIZZLE(Z, Z, W, W), /* zw */
    gcmSWIZZLE(W, W, W, W)    /* ww */
};

static gctINT
_getCL_Long_ulong_store_count(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction
    )
{
    gctINT storeCount = 0;

    if (CodeGen->clShader && !CodeGen->hasLongUlong)
    {
        gcSL_ENABLE enable;
        gcSL_FORMAT format = gcmSL_TARGET_GET(Instruction->temp, Format);

        if(format != gcSL_INT64 && format != gcSL_UINT64) return 0;

        enable = gcmSL_TARGET_GET(Instruction->temp, Enable);
        if((enable & gcSL_ENABLE_XY) && (enable & gcSL_ENABLE_ZW))
        {
            storeCount = 4;
        }
        else
        {
            storeCount = 2;
        }
    }

    return storeCount;
}

static gctBOOL
_isCL_Long_ulong_2_store(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    return (_getCL_Long_ulong_store_count(Tree, CodeGen, Instruction) == 2);
}

static gctBOOL
_NoLabel_isCL_Long_ulong_2_store(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    gctINT pc = (gctINT )(Instruction - Tree->shader->code);

    if (Tree->hints[pc].callers != gcvNULL)
    {
        return gcvFALSE;
    }

    return (_getCL_Long_ulong_store_count(Tree, CodeGen, Instruction) == 2);
}

static gctBOOL
long_ulong_lower_offset(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gcSL_ENABLE enable;
    gctUINT8 swizzle = gcSL_SWIZZLE_XXXX;
    gcSL_TYPE constType = gcSL_INT32;

    enable = gcmSL_TARGET_GET((Instruction + 1)->temp, Enable);
    switch(enable)
    {
    case gcSL_ENABLE_X:
    case gcSL_ENABLE_Y:
    case gcSL_ENABLE_Z:
    case gcSL_ENABLE_W:
        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                       CodeGen,
                                       _gcdLongUlongSingleEnableByteOffset(enable),
                                       &index,
                                       &swizzle,
                                       &constType));
        break;

    case gcSL_ENABLE_XY:
    case gcSL_ENABLE_XYZ:
    case gcSL_ENABLE_XYW:
    case gcSL_ENABLE_XYZW:
    case gcSL_ENABLE_YZ:
    case gcSL_ENABLE_ZW:
        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                       CodeGen,
                                       ((enable & gcSL_ENABLE_X)
                                           ?  _gcdLongUlongSingleEnableByteOffset(gcSL_ENABLE_X)
                                           : ((enable & gcSL_ENABLE_Y)
                                                ? _gcdLongUlongSingleEnableByteOffset(gcSL_ENABLE_Y)
                                                : _gcdLongUlongSingleEnableByteOffset(gcSL_ENABLE_Z))),
                                       &index,
                                       &swizzle,
                                       &constType));
        break;

    case gcSL_ENABLE_XZ:
    case gcSL_ENABLE_XW:
    case gcSL_ENABLE_XZW:
    case gcSL_ENABLE_YW:
        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                       CodeGen,
                                       _gcdLongUlongSingleEnableByteOffset(enable & gcSL_ENABLE_XY),
                                       &index,
                                       &swizzle,
                                       &constType));
        break;

    default:
        gcmASSERT(0);
        break;
    }

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);
    value_type0(Tree, CodeGen, Instruction, States);

    return gcvTRUE;
}

static gctBOOL
long_ulong_first_add_store(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_INSTRUCTION instruction = Instruction + 1;
    gctUINT8 orgSwizzle;
    gctUINT8 swizzle = gcSL_SWIZZLE_XYYY;
    gcSL_FORMAT format;
    gcSL_ENABLE enable = _gcdLongUlongStoreOneComponentEnable;

    format = gcmSL_TARGET_GET(instruction->temp, Format);
    if(format == gcSL_UINT64) format = gcSL_UINT32;
    else format = gcSL_INT32;

    _SetValueType0(type_conv[format], States);

    orgSwizzle = (((((gctUINT32) (States[3])) >> (0 ? 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1)))))) ) & 0xF;
    switch(gcmSL_TARGET_GET(instruction->temp, Enable))
    {
    case gcSL_ENABLE_X:
    case gcSL_ENABLE_Y:
    case gcSL_ENABLE_Z:
    case gcSL_ENABLE_W:
        swizzle = _longUlongOneComponentSwizzleMap[orgSwizzle & 0x3];
        enable = _gcdLongUlongStoreOneComponentEnable;
        break;

    case gcSL_ENABLE_XY:
    case gcSL_ENABLE_XYZ:
    case gcSL_ENABLE_XYW:
    case gcSL_ENABLE_XYZW:
    case gcSL_ENABLE_YZ:
    case gcSL_ENABLE_ZW:
        swizzle = _longUlongTwoComponentSwizzleMap[orgSwizzle];
        enable = _gcdLongUlongStoreTwoComponentEnable;
        break;

    case gcSL_ENABLE_XZ:
    case gcSL_ENABLE_XW:
    case gcSL_ENABLE_XZW:
    case gcSL_ENABLE_YW:
        swizzle = _longUlongOneComponentSwizzleMap[orgSwizzle & 0x3];
        enable = _gcdLongUlongStoreOneComponentEnable;
        break;

    default:
        gcmASSERT(0);
        break;
    }

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable) & ((gctUINT32) ((((1 ? 26:23) - (0 ?
 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));

    if (CodeGen->hasSHEnhancements2)
    {
        swizzle = (((((gctUINT32) (States[1])) >> (0 ? 10:3)) & ((gctUINT32) ((((1 ? 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1)))))) );
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3))) | (((gctUINT32) ((gctUINT32) (swizzle | 0x80) & ((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3)));
    }

    return gcvTRUE;
}

static gctBOOL
long_ulong_second_store_zero_2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle;
    gcSL_TYPE constType;

    long_ulong_first_add_store(Tree,
                               CodeGen,
                               Instruction,
                               States);

    if (Generate20BitsImmediate(CodeGen, Instruction, 0))
    {
        gcsConstantValue constValue;
        constValue.ty = gcSL_INTEGER;
        constValue.value.u = 0;

        gcmVERIFY_OK(gcEncodeSourceImmediate20(States,
                                               2,
                                               &constValue));
    }
    else
    {
        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                       CodeGen,
                                       0,
                                       &index,
                                       &swizzle,
                                       &constType));

        _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);
    }
    return gcvTRUE;
}

static gctBOOL
int_value_type0_const_4(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                   CodeGen,
                                   4,
                                   &index,
                                   &swizzle,
                                   &constType));
    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x2, States);

    return gcvTRUE;
}

#if _SUPPORT_LONG_ULONG_DATA_TYPE
static gctBOOL
_hasInteger_long_ulong(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    gctBOOL ok = gcvFALSE;

    if (CodeGen->clShader && !CodeGen->hasLongUlong)
    {
        gcSL_FORMAT format = gcmSL_TARGET_GET(Instruction->temp, Format);

        if(format == gcSL_INT64 || format == gcSL_UINT64) ok = gcvTRUE;
    }
    return ok && CodeGen->hasInteger;
}

static gctBOOL
_isCL_Long_ulong_4_store(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    return (_getCL_Long_ulong_store_count(Tree, CodeGen, Instruction) == 4);
}

#define _gcdLongUlongStoreOneComponentEnable   gcSL_ENABLE_X
#define _gcdLongUlongStoreTwoComponentEnable   gcSL_ENABLE_XZ

static gctBOOL
long_ulong_first_store(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 orgSwizzle;
    gctUINT8 swizzle = gcSL_SWIZZLE_XYYY;
    gcSL_TYPE constType;
    gcSL_FORMAT format;
    gcSL_ENABLE orgEnable;
    gcSL_ENABLE enable = _gcdLongUlongStoreOneComponentEnable;

    format = gcmSL_TARGET_GET(Instruction->temp, Format);
    if(format == gcSL_UINT64) format = gcSL_UINT32;
    else format = gcSL_INT32;

    _SetValueType0(type_conv[format], States);

    orgSwizzle = (((((gctUINT32) (States[3])) >> (0 ? 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1)))))) ) & 0xF;
    orgEnable = gcmSL_TARGET_GET(Instruction->temp, Enable);
    switch(orgEnable)
    {
    case gcSL_ENABLE_X:
    case gcSL_ENABLE_Y:
    case gcSL_ENABLE_Z:
    case gcSL_ENABLE_W:
        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                       CodeGen,
                                       _gcdLongUlongSingleEnableByteOffset(enable),
                                       &index,
                                       &swizzle,
                                       &constType));
        _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);
        swizzle = _longUlongOneComponentSwizzleMap[orgSwizzle & 0x3];
        enable = _gcdLongUlongStoreOneComponentEnable;
        break;

    case gcSL_ENABLE_XY:
    case gcSL_ENABLE_XYZ:
    case gcSL_ENABLE_XYW:
    case gcSL_ENABLE_XYZW:
    case gcSL_ENABLE_YZ:
    case gcSL_ENABLE_ZW:
        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                       CodeGen,
                                       ((orgEnable & gcSL_ENABLE_X)
                                           ?  _gcdLongUlongSingleEnableByteOffset(gcSL_ENABLE_X)
                                           : ((orgEnable & gcSL_ENABLE_Y)
                                                ? _gcdLongUlongSingleEnableByteOffset(gcSL_ENABLE_Y)
                                                : _gcdLongUlongSingleEnableByteOffset(gcSL_ENABLE_Z))),
                                       &index,
                                       &swizzle,
                                       &constType));
        _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);
        swizzle = _longUlongTwoComponentSwizzleMap[orgSwizzle];
        enable = _gcdLongUlongStoreTwoComponentEnable;
        break;

    case gcSL_ENABLE_XZ:
    case gcSL_ENABLE_XW:
    case gcSL_ENABLE_XZW:
    case gcSL_ENABLE_YW:
        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                       CodeGen,
                                       _gcdLongUlongSingleEnableByteOffset(orgEnable & gcSL_ENABLE_XY),
                                       &index,
                                       &swizzle,
                                       &constType));
        _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);
        swizzle = _longUlongOneComponentSwizzleMap[orgSwizzle & 0x3];
        enable = _gcdLongUlongStoreOneComponentEnable;
        break;

    default:
        gcmASSERT(0);
        break;
    }

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable) & ((gctUINT32) ((((1 ? 26:23) - (0 ?
 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));

    if (CodeGen->hasSHEnhancements2)
    {
        swizzle = (((((gctUINT32) (States[1])) >> (0 ? 10:3)) & ((gctUINT32) ((((1 ? 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1)))))) );
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3))) | (((gctUINT32) ((gctUINT32) (swizzle | 0x80) & ((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3)));
    }

    return gcvTRUE;
}

static gctBOOL
long_ulong_second_store(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 orgSwizzle;
    gctUINT8 swizzle = gcSL_SWIZZLE_XYYY;
    gcSL_TYPE constType;
    gcSL_FORMAT format;
    gcSL_ENABLE orgEnable;
    gcSL_ENABLE enable = _gcdLongUlongStoreOneComponentEnable;
    gctUINT address;
    gcsConstantValue constValue;

    format = gcmSL_TARGET_GET(Instruction->temp, Format);
    if(format == gcSL_UINT64) format = gcSL_UINT32;
    else format = gcSL_INT32;

    _SetValueType0(type_conv[format], States);

    orgSwizzle = (((((gctUINT32) (States[3])) >> (0 ? 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1)))))) ) & 0xF;
    orgEnable = gcmSL_TARGET_GET(Instruction->temp, Enable);
    switch(orgEnable)
    {
    case gcSL_ENABLE_X:
    case gcSL_ENABLE_Y:
    case gcSL_ENABLE_Z:
    case gcSL_ENABLE_W:
        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                       CodeGen,
                                       _gcdLongUlongSingleEnableByteOffset(enable) + 4,
                                       &index,
                                       &swizzle,
                                       &constType));
        _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);
        swizzle = _longUlongOneComponentSwizzleMap[orgSwizzle & 0x3];
        enable = _gcdLongUlongStoreOneComponentEnable;
        break;

    case gcSL_ENABLE_XY:
    case gcSL_ENABLE_XYZ:
    case gcSL_ENABLE_XYW:
    case gcSL_ENABLE_XYZW:
    case gcSL_ENABLE_YZ:
    case gcSL_ENABLE_ZW:
        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                       CodeGen,
                                       ((orgEnable & gcSL_ENABLE_X)
                                           ?  _gcdLongUlongSingleEnableByteOffset(gcSL_ENABLE_X)
                                           : ((orgEnable & gcSL_ENABLE_Y)
                                                ? _gcdLongUlongSingleEnableByteOffset(gcSL_ENABLE_Y)
                                                : _gcdLongUlongSingleEnableByteOffset(gcSL_ENABLE_Z))) + 4,
                                       &index,
                                       &swizzle,
                                       &constType));
        _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);
        swizzle = _longUlongTwoComponentSwizzleMap[orgSwizzle];
        enable = _gcdLongUlongStoreTwoComponentEnable;
        break;

    case gcSL_ENABLE_XZ:
    case gcSL_ENABLE_XW:
    case gcSL_ENABLE_XZW:
    case gcSL_ENABLE_YW:
        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                       CodeGen,
                                       _gcdLongUlongSingleEnableByteOffset(orgEnable & gcSL_ENABLE_XY) + 4,
                                       &index,
                                       &swizzle,
                                       &constType));
        _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);
        swizzle = _longUlongOneComponentSwizzleMap[orgSwizzle & 0x3];
        enable = _gcdLongUlongStoreOneComponentEnable;
        break;

    default:
        gcmASSERT(0);
        break;
    }

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable) & ((gctUINT32) ((((1 ? 26:23) - (0 ?
 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));

    if (CodeGen->hasSHEnhancements2)
    {
        swizzle = (((((gctUINT32) (States[1])) >> (0 ? 10:3)) & ((gctUINT32) ((((1 ? 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1)))))) );
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3))) | (((gctUINT32) ((gctUINT32) (swizzle | 0x80) & ((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3)));
    }

    switch(gcmSL_SOURCE_GET(Instruction->source1, Type))
    {
    case gcSL_TEMP:
        address = (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ) + 1;
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ? 12:4) - (0 ?
 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4)));
        break;

    case gcSL_UNIFORM:
        address = (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ) + 1;
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ? 12:4) - (0 ?
 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4)));
        break;

    case gcSL_CONSTANT:
        constValue.value.u = 0;
        format = gcmSL_SOURCE_GET(Instruction->source1, Format);
        if(format == gcSL_INTEGER  ||
           format == gcSL_INT16  ||
           format == gcSL_INT8 ||
           format == gcSL_INT64)
        {
            gctINT val;
            /* Assemble the 32-bit value. */
            val = (Instruction->source1Index & 0xFFFF) | (Instruction->source1Indexed << 16);
            if(val < 0)
            {
                constValue.value.u = 0xFFFFFFFF;
            }
            constValue.ty = gcSL_INTEGER;
        }
        else
        {
            constValue.ty = gcSL_UINT32;
        }

        if (Generate20BitsImmediate(CodeGen, Instruction, 1))
        {
            gcmVERIFY_OK(gcEncodeSourceImmediate20(States,
                                                   2,
                                                   &constValue));
        }
        else
        {
            gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                           CodeGen,
                                           constValue.value.u,
                                           &index,
                                           &swizzle,
                                           &constType));
            _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ? 27:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 27:25) - (0 ?
 27:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ?
 27:25)));
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 22:22) - (0 ?
 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
 22:22)));
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ? 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 23:23) - (0 ?
 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23)));
        }
        break;

    default:
        break;
    }
    return gcvTRUE;
}

static gctBOOL
long_ulong_third_store(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = gcSL_SWIZZLE_XYYY;
    gcSL_TYPE constType;
    gcSL_FORMAT format;
    gcSL_ENABLE orgEnable;
    gcSL_ENABLE enable = _gcdLongUlongStoreOneComponentEnable;

    format = gcmSL_TARGET_GET(Instruction->temp, Format);
    if(format == gcSL_UINT64) format = gcSL_UINT32;
    else format = gcSL_INT32;

    _SetValueType0(type_conv[format], States);

    orgEnable = gcmSL_TARGET_GET(Instruction->temp, Enable);
    switch(orgEnable)
    {
    case gcSL_ENABLE_XYZ:
    case gcSL_ENABLE_XYW:
    case gcSL_ENABLE_XYZW:
    case gcSL_ENABLE_YZW:
    case gcSL_ENABLE_XZ:
    case gcSL_ENABLE_XW:
    case gcSL_ENABLE_XZW:
    case gcSL_ENABLE_YW:
        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                       CodeGen,
                                       (orgEnable & gcSL_ENABLE_Z)
                                           ? _gcdLongUlongSingleEnableByteOffset(gcSL_ENABLE_Z)
                                           : _gcdLongUlongSingleEnableByteOffset(gcSL_ENABLE_W),
                                       &index,
                                       &swizzle,
                                       &constType));
        _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);

        swizzle = (((((gctUINT32) (States[3])) >> (0 ? 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1)))))) ) >> 4;
        if((orgEnable & gcSL_ENABLE_Z) && (orgEnable & gcSL_ENABLE_W))
        {
            swizzle = _longUlongTwoComponentSwizzleMap[swizzle];
            enable = _gcdLongUlongStoreTwoComponentEnable;
        }
        else
        {
            swizzle = _longUlongOneComponentSwizzleMap[swizzle & 0x3];
            enable = _gcdLongUlongStoreOneComponentEnable;
        }
        break;

    default:
        gcmASSERT(0);
        break;
    }

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable) & ((gctUINT32) ((((1 ? 26:23) - (0 ?
 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));
    if (CodeGen->hasSHEnhancements2)
    {
        swizzle = (((((gctUINT32) (States[1])) >> (0 ? 10:3)) & ((gctUINT32) ((((1 ? 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1)))))) );
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3))) | (((gctUINT32) ((gctUINT32) (swizzle | 0x80) & ((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3)));
    }
    return gcvTRUE;
}

static gctBOOL
long_ulong_fourth_store(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = gcSL_SWIZZLE_XYYY;
    gcSL_TYPE constType;
    gcSL_FORMAT format;
    gcSL_ENABLE orgEnable;
    gcSL_ENABLE enable = _gcdLongUlongStoreOneComponentEnable;
    gctUINT address;
    gcsConstantValue constValue;

    format = gcmSL_TARGET_GET(Instruction->temp, Format);
    if(format == gcSL_UINT64) format = gcSL_UINT32;
    else format = gcSL_INT32;

    _SetValueType0(type_conv[format], States);

    orgEnable = gcmSL_TARGET_GET(Instruction->temp, Enable);
    switch(orgEnable)
    {
    case gcSL_ENABLE_XYZ:
    case gcSL_ENABLE_XYW:
    case gcSL_ENABLE_XYZW:
    case gcSL_ENABLE_YZW:
    case gcSL_ENABLE_XZ:
    case gcSL_ENABLE_XW:
    case gcSL_ENABLE_XZW:
    case gcSL_ENABLE_YW:
        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                       CodeGen,
                                       ((orgEnable & gcSL_ENABLE_Z)
                                           ? _gcdLongUlongSingleEnableByteOffset(gcSL_ENABLE_Z)
                                           : _gcdLongUlongSingleEnableByteOffset(gcSL_ENABLE_W)) + 4,
                                       &index,
                                       &swizzle,
                                       &constType));
        _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);

        swizzle = (((((gctUINT32) (States[3])) >> (0 ? 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1)))))) ) >> 4;
        if((orgEnable & gcSL_ENABLE_Z) && (orgEnable & gcSL_ENABLE_W))
        {
            swizzle = _longUlongTwoComponentSwizzleMap[swizzle];
            enable = _gcdLongUlongStoreTwoComponentEnable;
        }
        else
        {
            swizzle = _longUlongOneComponentSwizzleMap[swizzle & 0x3];
            enable = _gcdLongUlongStoreOneComponentEnable;
        }
        break;

    default:
        gcmASSERT(0);
        break;
    }

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable) & ((gctUINT32) ((((1 ? 26:23) - (0 ?
 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));

    if (CodeGen->hasSHEnhancements2)
    {
        swizzle = (((((gctUINT32) (States[1])) >> (0 ? 10:3)) & ((gctUINT32) ((((1 ? 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1)))))) );
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3))) | (((gctUINT32) ((gctUINT32) (swizzle | 0x80) & ((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3)));
    }

    switch(gcmSL_SOURCE_GET(Instruction->source1, Type))
    {
    case gcSL_TEMP:
        address = (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ) + 1;
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ? 12:4) - (0 ?
 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4)));
        break;

    case gcSL_UNIFORM:
        address = (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ) + 1;
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ? 12:4) - (0 ?
 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4)));
        break;

    case gcSL_CONSTANT:
        constValue.value.u = 0;
        format = gcmSL_SOURCE_GET(Instruction->source1, Format);
        if(format == gcSL_INTEGER  ||
           format == gcSL_INT16  ||
           format == gcSL_INT8 ||
           format == gcSL_INT64)
        {
            gctINT val;
            /* Assemble the 32-bit value. */
            val = (Instruction->source1Index & 0xFFFF) | (Instruction->source1Indexed << 16);
            if(val < 0)
            {
                constValue.value.u = 0xFFFFFFFF;
            }
            constValue.ty = gcSL_INTEGER;
        }
        else
        {
            constValue.ty = gcSL_UINT32;
        }

        if (Generate20BitsImmediate(CodeGen, Instruction, 1))
        {
            gcmVERIFY_OK(gcEncodeSourceImmediate20(States,
                                                   2,
                                                   &constValue));
        }
        else
        {
            gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                           CodeGen,
                                           constValue.value.u,
                                           &index,
                                           &swizzle,
                                           &constType));
            _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ? 27:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 27:25) - (0 ?
 27:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ?
 27:25)));
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 22:22) - (0 ?
 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
 22:22)));
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ? 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 23:23) - (0 ?
 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23)));
        }
        break;

    default:
        break;
    }
    return gcvTRUE;
}

static gctBOOL
long_ulong_upper_offset(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = gcSL_SWIZZLE_XYYY;
    gcSL_TYPE constType = gcSL_INT32;
    gcSL_ENABLE enable;

    enable = gcmSL_TARGET_GET((Instruction + 1)->temp, Enable);
    switch(enable)
    {
    case gcSL_ENABLE_XYZ:
    case gcSL_ENABLE_XYW:
    case gcSL_ENABLE_XYZW:
    case gcSL_ENABLE_YZW:
    case gcSL_ENABLE_XZ:
    case gcSL_ENABLE_XW:
    case gcSL_ENABLE_XZW:
    case gcSL_ENABLE_YW:
        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                       CodeGen,
                                       (enable & gcSL_ENABLE_Z)
                                           ? _gcdLongUlongSingleEnableByteOffset(gcSL_ENABLE_Z)
                                           : _gcdLongUlongSingleEnableByteOffset(gcSL_ENABLE_W),
                                       &index,
                                       &swizzle,
                                       &constType));
        _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);
        value_type0(Tree, CodeGen, Instruction, States);
        break;

    default:
        gcmASSERT(0);
        break;
    }

    return gcvTRUE;
}

static gctBOOL
_NoLabel_isCL_Long_ulong_4_store(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    gctINT pc = (gctINT )(Instruction - Tree->shader->code);

    if (Tree->hints[pc].callers != gcvNULL)
    {
        return gcvFALSE;
    }

    return (_getCL_Long_ulong_store_count(Tree, CodeGen, Instruction) == 4);
}

static gctBOOL
long_ulong_second_add_store(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT address;
    gcSL_FORMAT format;
    gctINT index = 0;
    gctUINT8 swizzle;
    gcSL_TYPE constType;
    gcsConstantValue constValue;

    long_ulong_first_add_store(Tree,
                               CodeGen,
                               Instruction,
                               States);

    switch(gcmSL_SOURCE_GET((Instruction + 1)->source1, Type))
    {
    case gcSL_TEMP:
        address = (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ) + 1;
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ? 12:4) - (0 ?
 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4)));
        break;

    case gcSL_UNIFORM:
        address = (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ) + 1;
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ? 12:4) - (0 ?
 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4)));
        break;

    case gcSL_CONSTANT:
        constValue.value.u = 0;
        format = gcmSL_SOURCE_GET((Instruction + 1)->source1, Format);
        if(format == gcSL_INTEGER  ||
           format == gcSL_INT16  ||
           format == gcSL_INT8 ||
           format == gcSL_INT64)
        {
            gctINT val;
            /* Assemble the 32-bit value. */
            val = ((Instruction + 1)->source1Index & 0xFFFF) | ((Instruction + 1)->source1Indexed << 16);
            if(val < 0)
            {
                constValue.value.u = 0xFFFFFFFF;
            }
            constValue.ty = gcSL_INTEGER;
        }
        else
        {
            constValue.ty = gcSL_UINT32;
        }

        if (Generate20BitsImmediate(CodeGen, Instruction + 1, 1))
        {
            gcmVERIFY_OK(gcEncodeSourceImmediate20(States,
                                                   2,
                                                   &constValue));
        }
        else
        {
            gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                           CodeGen,
                                           constValue.value.u,
                                           &index,
                                           &swizzle,
                                           &constType));
            _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ? 27:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 27:25) - (0 ?
 27:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ?
 27:25)));
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 22:22) - (0 ?
 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
 22:22)));
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ? 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 23:23) - (0 ?
 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23)));
        }
        break;

    default:
        break;
    }
    return gcvTRUE;
}

static gctBOOL
long_ulong_third_add_store(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT8 swizzle = gcSL_SWIZZLE_XYYY;
    gcSL_INSTRUCTION instruction = Instruction + 1;
    gcSL_FORMAT format;
    gcSL_ENABLE orgEnable;
    gcSL_ENABLE enable = _gcdLongUlongStoreOneComponentEnable;

    format = gcmSL_TARGET_GET(instruction->temp, Format);
    if(format == gcSL_UINT64) format = gcSL_UINT32;
    else format = gcSL_INT32;

    _SetValueType0(type_conv[format], States);

    orgEnable = gcmSL_TARGET_GET(instruction->temp, Enable);
    switch(orgEnable)
    {
    case gcSL_ENABLE_XYZ:
    case gcSL_ENABLE_XYW:
    case gcSL_ENABLE_XYZW:
    case gcSL_ENABLE_YZW:
    case gcSL_ENABLE_XZ:
    case gcSL_ENABLE_XW:
    case gcSL_ENABLE_XZW:
    case gcSL_ENABLE_YW:
        swizzle = (((((gctUINT32) (States[3])) >> (0 ? 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1)))))) ) >> 4;
        if((orgEnable & gcSL_ENABLE_Z) && (orgEnable & gcSL_ENABLE_W))
        {
            swizzle = _longUlongTwoComponentSwizzleMap[swizzle];
            enable = _gcdLongUlongStoreTwoComponentEnable;
        }
        else
        {
            swizzle = _longUlongOneComponentSwizzleMap[swizzle & 0x3];
            enable = _gcdLongUlongStoreOneComponentEnable;
        }
        break;

    default:
        gcmASSERT(0);
        break;
    }

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable) & ((gctUINT32) ((((1 ? 26:23) - (0 ?
 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));
    if (CodeGen->hasSHEnhancements2)
    {
        swizzle = (((((gctUINT32) (States[1])) >> (0 ? 10:3)) & ((gctUINT32) ((((1 ? 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1)))))) );
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3))) | (((gctUINT32) ((gctUINT32) (swizzle | 0x80) & ((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3)));
    }
    return gcvTRUE;
}

static gctBOOL
long_ulong_fourth_add_store(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT address;
    gcSL_FORMAT format;
    gctINT index = 0;
    gctUINT8 swizzle;
    gcSL_TYPE constType;
    gcsConstantValue constValue;

    long_ulong_third_add_store(Tree,
                               CodeGen,
                               Instruction,
                               States);

    switch(gcmSL_SOURCE_GET((Instruction + 1)->source1, Type))
    {
    case gcSL_TEMP:
        address = (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ) + 1;
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ? 12:4) - (0 ?
 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4)));
        break;

    case gcSL_UNIFORM:
        address = (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ) + 1;
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ? 12:4) - (0 ?
 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4)));
        break;

    case gcSL_CONSTANT:
        constValue.value.u = 0;
        format = gcmSL_SOURCE_GET((Instruction + 1)->source1, Format);
        if(format == gcSL_INTEGER  ||
           format == gcSL_INT16  ||
           format == gcSL_INT8 ||
           format == gcSL_INT64)
        {
            gctINT val;
            /* Assemble the 32-bit value. */
            val = ((Instruction + 1)->source1Index & 0xFFFF) | ((Instruction + 1)->source1Indexed << 16);
            if(val < 0)
            {
                constValue.value.u = 0xFFFFFFFF;
            }
            constValue.ty = gcSL_INTEGER;
        }
        else
        {
            constValue.ty = gcSL_UINT32;
        }

        if (Generate20BitsImmediate(CodeGen, Instruction + 1, 1))
        {
            gcmVERIFY_OK(gcEncodeSourceImmediate20(States,
                                                   2,
                                                   &constValue));
        }
        else
        {
            gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                           CodeGen,
                                           constValue.value.u,
                                           &index,
                                           &swizzle,
                                           &constType));
            _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ? 27:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 27:25) - (0 ?
 27:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ?
 27:25)));
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 22:22) - (0 ?
 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
 22:22)));
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ? 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 23:23) - (0 ?
 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23)));
        }
        break;

    default:
        break;
    }
    return gcvTRUE;
}

static gctBOOL
long_ulong_first_logical_op(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_FORMAT format;

    format = gcmSL_TARGET_GET(Instruction->temp, Format);
    if(format == gcSL_UINT64) format = gcSL_UINT32;
    else format = gcSL_INT32;

    _SetValueType0(type_conv[format], States);

    return gcvTRUE;
}

static gctBOOL
long_ulong_second_logical_op(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle;
    gcSL_TYPE constType;
    gcSL_FORMAT format;
    gctUINT address;
    gcsConstantValue constValue;

    long_ulong_first_logical_op(Tree,
                                CodeGen,
                                Instruction,
                                States);

    address = (((((gctUINT32) (States[0])) >> (0 ? 22:16)) & ((gctUINT32) ((((1 ? 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1)))))) ) + 1;
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ?
 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ?
 22:16)));

    switch(gcmSL_SOURCE_GET(Instruction->source0, Type))
    {
    case gcSL_TEMP:
        address = (((((gctUINT32) (States[1])) >> (0 ? 20:12)) & ((gctUINT32) ((((1 ? 20:12) - (0 ? 20:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:12) - (0 ? 20:12) + 1)))))) ) + 1;
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ? 20:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ?
 20:12) - (0 ? 20:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ?
 20:12)));
        break;

    case gcSL_UNIFORM:
        address = (((((gctUINT32) (States[1])) >> (0 ? 20:12)) & ((gctUINT32) ((((1 ? 20:12) - (0 ? 20:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:12) - (0 ? 20:12) + 1)))))) ) + 1;
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ? 20:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ?
 20:12) - (0 ? 20:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ?
 20:12)));
        break;

    case gcSL_CONSTANT:
        constValue.value.u = 0;
        format = gcmSL_SOURCE_GET(Instruction->source0, Format);
        if(format == gcSL_INTEGER  ||
           format == gcSL_INT16  ||
           format == gcSL_INT8 ||
           format == gcSL_INT64)
        {
            gctINT val;
            /* Assemble the 32-bit value. */
            val = (Instruction->source0Index & 0xFFFF) | (Instruction->source0Indexed << 16);
            if(val < 0)
            {
                constValue.value.u = 0xFFFFFFFF;
            }
            constValue.ty = gcSL_INTEGER;
        }
        else
        {
            constValue.ty = gcSL_UINT32;
        }

        if (Generate20BitsImmediate(CodeGen, Instruction, 0))
        {
            gcmVERIFY_OK(gcEncodeSourceImmediate20(States,
                                                   0,
                                                   &constValue));
        }
        else
        {
            gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                           CodeGen,
                                           constValue.value.u,
                                           &index,
                                           &swizzle,
                                           &constType));
            _UsingConstUniform(Tree, CodeGen, 0, index, swizzle, constType, States);
            States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));
            States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ? 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 30:30) - (0 ?
 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ?
 30:30)));
            States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)));
        }
        break;

    default:
        break;
    }

    switch(gcmSL_SOURCE_GET(Instruction->source1, Type))
    {
    case gcSL_TEMP:
        address = (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ) + 1;
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ? 12:4) - (0 ?
 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4)));
        break;

    case gcSL_UNIFORM:
        address = (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ) + 1;
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ? 12:4) - (0 ?
 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4)));
        break;

    case gcSL_CONSTANT:
        constValue.value.u = 0;
        format = gcmSL_SOURCE_GET(Instruction->source1, Format);
        if(format == gcSL_INTEGER  ||
           format == gcSL_INT16  ||
           format == gcSL_INT8 ||
           format == gcSL_INT64)
        {
            gctINT val;
            /* Assemble the 32-bit value. */
            val = (Instruction->source1Index & 0xFFFF) | (Instruction->source1Indexed << 16);
            if(val < 0)
            {
                constValue.value.u = 0xFFFFFFFF;
            }
            constValue.ty = gcSL_INTEGER;
        }
        else
        {
            constValue.ty = gcSL_UINT32;
        }

        if (Generate20BitsImmediate(CodeGen, Instruction, 1))
        {
            gcmVERIFY_OK(gcEncodeSourceImmediate20(States,
                                                   2,
                                                   &constValue));
        }
        else
        {
            gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                           CodeGen,
                                           constValue.value.u,
                                           &index,
                                           &swizzle,
                                           &constType));
            _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ? 27:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 27:25) - (0 ?
 27:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ?
 27:25)));
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 22:22) - (0 ?
 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
 22:22)));
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ? 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 23:23) - (0 ?
 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23)));
        }
        break;

    default:
        break;
    }
    return gcvTRUE;
}

static gctBOOL
long_ulong_first_logical_not_op(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    return long_ulong_first_logical_op(Tree,
                                       CodeGen,
                                       Instruction,
                                       States);
}

static gctBOOL
long_ulong_second_logical_not_op(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle;
    gcSL_TYPE constType;
    gcSL_FORMAT format;
    gctUINT address;
    gcsConstantValue constValue;

    long_ulong_first_logical_not_op(Tree,
                                CodeGen,
                                Instruction,
                                States);

    address = (((((gctUINT32) (States[0])) >> (0 ? 22:16)) & ((gctUINT32) ((((1 ? 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1)))))) ) + 1;
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ?
 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ?
 22:16)));


    switch(gcmSL_SOURCE_GET(Instruction->source0, Type))
    {
    case gcSL_TEMP:
        address = (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ) + 1;
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ? 12:4) - (0 ?
 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4)));
        break;

    case gcSL_UNIFORM:
        address = (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ) + 1;
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ? 12:4) - (0 ?
 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4)));
        break;

    case gcSL_CONSTANT:
        constValue.value.u = 0;
        format = gcmSL_SOURCE_GET(Instruction->source0, Format);
        if(format == gcSL_INTEGER  ||
           format == gcSL_INT16  ||
           format == gcSL_INT8 ||
           format == gcSL_INT64)
        {
            gctINT val;
            /* Assemble the 32-bit value. */
            val = (Instruction->source0Index & 0xFFFF) | (Instruction->source0Indexed << 16);
            if(val < 0)
            {
                constValue.value.u = 0xFFFFFFFF;
            }
            constValue.ty = gcSL_INTEGER;
        }
        else
        {
            constValue.ty = gcSL_UINT32;
        }

        if (Generate20BitsImmediate(CodeGen, Instruction, 0))
        {
            gcmVERIFY_OK(gcEncodeSourceImmediate20(States,
                                                   2,
                                                   &constValue));
        }
        else
        {
            gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                           CodeGen,
                                           constValue.value.u,
                                           &index,
                                           &swizzle,
                                           &constType));
            _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ? 27:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 27:25) - (0 ?
 27:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ?
 27:25)));
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 22:22) - (0 ?
 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
 22:22)));
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ? 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 23:23) - (0 ?
 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23)));
        }
        break;

    default:
        break;
    }

    return gcvTRUE;
}

static gctBOOL
long_ulong_first_mov(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_FORMAT format;

    format = gcmSL_TARGET_GET(Instruction->temp, Format);
    if(format == gcSL_UINT64) format = gcSL_UINT32;
    else format = gcSL_INT32;

    _SetValueType0(type_conv[format], States);

    return mov(Tree,
              CodeGen,
              Instruction,
              States);
}

static gctBOOL
long_ulong_second_mov(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_FORMAT format;
    gctUINT     address;
    gctBOOL     res = gcvTRUE;
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;
    gcsConstantValue constValue;

    format = gcmSL_TARGET_GET(Instruction->temp, Format);
    if(format == gcSL_UINT64) format = gcSL_UINT32;
    else format = gcSL_INT32;

    _SetValueType0(type_conv[format], States);

    res = mov(Tree,
              CodeGen,
              Instruction,
              States);
    if (!res)
    {
        return res;
    }

    address = (((((gctUINT32) (States[0])) >> (0 ? 22:16)) & ((gctUINT32) ((((1 ? 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1)))))) ) + 1;
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ?
 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ?
 22:16)));


    switch(gcmSL_SOURCE_GET(Instruction->source0, Type))
    {
    case gcSL_TEMP:
        address = (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ) + 1;
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ? 12:4) - (0 ?
 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4)));
        break;

    case gcSL_UNIFORM:
        address = (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ) + 1;
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ? 12:4) - (0 ?
 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4)));
        break;

    case gcSL_CONSTANT:
        constValue.value.u = 0;
        format = gcmSL_SOURCE_GET(Instruction->source0, Format);
        if(format == gcSL_INTEGER  ||
           format == gcSL_INT16  ||
           format == gcSL_INT8 ||
           format == gcSL_INT64)
        {
            gctINT val;
            /* Assemble the 32-bit value. */
            val = (Instruction->source0Index & 0xFFFF) | (Instruction->source0Indexed << 16);
            if(val < 0)
            {
                constValue.value.u = 0xFFFFFFFF;
            }
            constValue.ty = gcSL_INTEGER;
        }
        else
        {
            constValue.ty = gcSL_UINT32;
        }

        if (Generate20BitsImmediate(CodeGen, Instruction, 0))
        {
            gcmVERIFY_OK(gcEncodeSourceImmediate20(States,
                                                   2,
                                                   &constValue));
        }
        else
        {
            gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                           CodeGen,
                                           constValue.value.u,
                                           &index,
                                           &swizzle,
                                           &constType));
            _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ? 27:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 27:25) - (0 ?
 27:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ?
 27:25)));
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 22:22) - (0 ?
 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
 22:22)));
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ? 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 23:23) - (0 ?
 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23)));
        }
        break;

    default:
        break;
    }

    return gcvTRUE;
}

static gctUINT
_CountEnables(
    IN gcSL_ENABLE Enable
    )
{
    gcSL_ENABLE enable = Enable;
    gctUINT count = 0;

    while(enable)
    {
        if(enable & 0x1) count++;
        enable >>= 1;
    }
    return count;
}

static gctBOOL
_isCL_Long_ulong_one_load_two_moves(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    if(_hasInteger_long_ulong(Tree, CodeGen, Instruction, States))
    {
        return gcmSL_TARGET_GET(Instruction->temp, Enable) == gcSL_ENABLE_XY ||
               _CountEnables(gcmSL_TARGET_GET(Instruction->temp, Enable)) == 1;
    }
    else return gcvFALSE;
}

static gctBOOL
_isCL_Long_ulong_two_load_four_moves(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    if(_hasInteger_long_ulong(Tree, CodeGen, Instruction, States))
    {
        return (gcmSL_TARGET_GET(Instruction->temp, Enable) & 0x3) &&
               (gcmSL_TARGET_GET(Instruction->temp, Enable) & 0xc);
    }
    else return gcvFALSE;
}

static gctBOOL
long_ulong_first_load_to_temp(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT8 swizzle = gcSL_SWIZZLE_XYYY;
    gcSL_FORMAT format;
    gcSL_ENABLE enable;

    format = gcmSL_TARGET_GET(Instruction->temp, Format);
    if(format == gcSL_UINT64) format = gcSL_UINT32;
    else format = gcSL_INT32;

    _SetValueType0(type_conv[format], States);

    enable = gcmSL_TARGET_GET(Instruction->temp, Enable) & 0x3;
    if(!enable)
    {
        enable = gcmSL_TARGET_GET(Instruction->temp, Enable) & 0xc;
    }
    switch(enable)
    {
    case gcSL_ENABLE_X:
    case gcSL_ENABLE_Y:
    case gcSL_ENABLE_Z:
    case gcSL_ENABLE_W:
        enable = gcSL_ENABLE_XY;
        break;

    case gcSL_ENABLE_XY:
        enable = gcSL_ENABLE_XYZW;
        break;

    default:
        break;
    }
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable) & ((gctUINT32) ((((1 ? 26:23) - (0 ?
 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));

    if (CodeGen->hasSHEnhancements2)
    {
        swizzle = (((((gctUINT32) (States[1])) >> (0 ? 10:3)) & ((gctUINT32) ((((1 ? 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1)))))) );
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3))) | (((gctUINT32) ((gctUINT32) (swizzle | 0x80) & ((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3)));
    }

    return gcvTRUE;
}

static gctBOOL
long_ulong_second_load_to_temp(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT8 swizzle = gcSL_SWIZZLE_XYYY;
    gcSL_FORMAT format;
    gcSL_ENABLE enable;

    format = gcmSL_TARGET_GET(Instruction->temp, Format);
    if(format == gcSL_UINT64) format = gcSL_UINT32;
    else format = gcSL_INT32;

    _SetValueType0(type_conv[format], States);

    gcmASSERT(gcmSL_TARGET_GET(Instruction->temp, Enable) & 0x3);
    enable = gcmSL_TARGET_GET(Instruction->temp, Enable) & ~0x3;
    switch(enable)
    {
    case gcSL_ENABLE_Z:
        enable = gcSL_ENABLE_XY;
        break;

    case gcSL_ENABLE_W:
    case gcSL_ENABLE_ZW:
        enable = gcSL_ENABLE_XYZW;
        break;

    default:
        break;
    }

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable) & ((gctUINT32) ((((1 ? 26:23) - (0 ?
 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));

    if (CodeGen->hasSHEnhancements2)
    {
        swizzle = (((((gctUINT32) (States[1])) >> (0 ? 10:3)) & ((gctUINT32) ((((1 ? 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1)))))) );
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3))) | (((gctUINT32) ((gctUINT32) (swizzle | 0x80) & ((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3)));
    }

    return gcvTRUE;
}

static gctBOOL
long_ulong_first_load_mov(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_FORMAT format;
    gcSL_ENABLE enable;
    gcSL_SWIZZLE swizzle = gcmSWIZZLE(X, Z, Z, Z);

    format = gcmSL_TARGET_GET(Instruction->temp, Format);
    if(format == gcSL_UINT64) format = gcSL_UINT32;
    else format = gcSL_INT32;

    _SetValueType0(type_conv[format], States);

    enable = (((((gctUINT32) (States[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) );
    switch(enable)
    {
    case gcSL_ENABLE_XYZ:
    case gcSL_ENABLE_XYW:
    case gcSL_ENABLE_XYZW:
        enable &= 0x3;
        break;

    case gcSL_ENABLE_Y:
    case gcSL_ENABLE_YW:
    case gcSL_ENABLE_YZ:
        swizzle = gcmSWIZZLE(X, X, Z, Z);
        break;

    case gcSL_ENABLE_YZW:
        enable = gcSL_ENABLE_YZ;
        swizzle = gcmSWIZZLE(X, X, Z, Z);
        break;

    case gcSL_ENABLE_Z:
    case gcSL_ENABLE_ZW:
        swizzle = gcmSWIZZLE(X, X, X, Z);
        break;

    case gcSL_ENABLE_W:
        swizzle = gcmSWIZZLE(X, X, X, X);
        break;

    default:
        break;
    }
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable) & ((gctUINT32) ((((1 ? 26:23) - (0 ?
 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));

    return gcvTRUE;
}

static gctBOOL
long_ulong_second_load_mov(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_FORMAT format;
    gcSL_ENABLE enable = gcSL_ENABLE_XY;
    gcSL_SWIZZLE swizzle = gcmSWIZZLE(Y, W, W, W);
    gctUINT address;

    format = gcmSL_TARGET_GET(Instruction->temp, Format);
    if(format == gcSL_UINT64) format = gcSL_UINT32;
    else format = gcSL_INT32;

    _SetValueType0(type_conv[format], States);

    address = (((((gctUINT32) (States[0])) >> (0 ? 22:16)) & ((gctUINT32) ((((1 ? 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1)))))) ) + 1;
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ?
 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ?
 22:16)));
    enable = (((((gctUINT32) (States[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) );
    switch(enable)
    {
    case gcSL_ENABLE_XYZ:
    case gcSL_ENABLE_XYW:
    case gcSL_ENABLE_XYZW:
        enable &= 0x3;
        break;

    case gcSL_ENABLE_Y:
    case gcSL_ENABLE_YW:
    case gcSL_ENABLE_YZ:
        swizzle = gcmSWIZZLE(Y, Y, W, W);
        break;

    case gcSL_ENABLE_YZW:
        enable = gcSL_ENABLE_YZ;
        swizzle = gcmSWIZZLE(Y, Y, W, W);
        break;

    case gcSL_ENABLE_Z:
    case gcSL_ENABLE_ZW:
        swizzle = gcmSWIZZLE(Y, Y, Y, W);
        break;

    case gcSL_ENABLE_W:
        swizzle = gcmSWIZZLE(Y, Y, Y, Y);
        break;

    default:
        break;
    }
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable) & ((gctUINT32) ((((1 ? 26:23) - (0 ?
 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));

    return gcvTRUE;
}

static gctBOOL
long_ulong_third_load_mov(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_FORMAT format;
    gcSL_ENABLE enable;
    gcSL_SWIZZLE swizzle = gcmSWIZZLE(X, X, X, X);

    format = gcmSL_TARGET_GET(Instruction->temp, Format);
    if(format == gcSL_UINT64) format = gcSL_UINT32;
    else format = gcSL_INT32;

    _SetValueType0(type_conv[format], States);

    enable = (((((gctUINT32) (States[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) );
    switch(enable)
    {
    case gcSL_ENABLE_XYZ:
    case gcSL_ENABLE_XYZW:
        enable &= ~0x3;
        swizzle = gcmSWIZZLE(X, X, X, Z);
        break;

    case gcSL_ENABLE_XYW:
    case gcSL_ENABLE_XZW:
    case gcSL_ENABLE_YZW:
        enable = gcSL_ENABLE_W;
        break;

    default:
        gcmASSERT(0);
        enable = gcSL_ENABLE_NONE;
        break;
    }
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable) & ((gctUINT32) ((((1 ? 26:23) - (0 ?
 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));

    return gcvTRUE;
}

static gctBOOL
long_ulong_fourth_load_mov(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_FORMAT format;
    gcSL_ENABLE enable;
    gcSL_SWIZZLE swizzle = gcmSWIZZLE(Y, Y, Y, Y);
    gctUINT address;

    format = gcmSL_TARGET_GET(Instruction->temp, Format);
    if(format == gcSL_UINT64) format = gcSL_UINT32;
    else format = gcSL_INT32;

    _SetValueType0(type_conv[format], States);

    address = (((((gctUINT32) (States[0])) >> (0 ? 22:16)) & ((gctUINT32) ((((1 ? 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1)))))) ) + 1;
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ?
 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ?
 22:16)));

    enable = (((((gctUINT32) (States[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) );
    switch(enable)
    {
    case gcSL_ENABLE_XYZ:
    case gcSL_ENABLE_XYZW:
        enable &= ~0x3;
        swizzle = gcmSWIZZLE(Y, Y, Y, W);
        break;

    case gcSL_ENABLE_XYW:
    case gcSL_ENABLE_XZW:
    case gcSL_ENABLE_YZW:
        enable = gcSL_ENABLE_W;
        break;

    default:
        gcmASSERT(0);
        enable = gcSL_ENABLE_NONE;
        break;
    }
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable) & ((gctUINT32) ((((1 ? 26:23) - (0 ?
 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));

    return gcvTRUE;
}

static gctBOOL
long_ulong_lower(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_FORMAT format;

    format = gcmSL_TARGET_GET(Instruction->temp, Format);

    _SetValueType0(type_conv[format], States);

    return gcvTRUE;
}

static gctBOOL
long_ulong_upper(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle;
    gcSL_TYPE constType;
    gcSL_FORMAT format;
    gctUINT address;
    gcsConstantValue constValue;

    format = gcmSL_TARGET_GET(Instruction->temp, Format);

    _SetValueType0(type_conv[format], States);

    switch(gcmSL_SOURCE_GET(Instruction->source0, Type))
    {
    case gcSL_TEMP:
        address = (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ) + 1;
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ? 12:4) - (0 ?
 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4)));
        break;

    case gcSL_UNIFORM:
        address = (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ) + 1;
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ? 12:4) - (0 ?
 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4)));
        break;

    case gcSL_CONSTANT:
        constValue.value.u = 0;
        format = gcmSL_SOURCE_GET(Instruction->source0, Format);
        if(format == gcSL_INTEGER  ||
           format == gcSL_INT16  ||
           format == gcSL_INT8 ||
           format == gcSL_INT64)
        {
            gctINT val;
            /* Assemble the 32-bit value. */
            val = (Instruction->source0Index & 0xFFFF) | (Instruction->source0Indexed << 16);
            if(val < 0)
            {
                constValue.value.u = 0xFFFFFFFF;
            }
            constValue.ty = gcSL_INTEGER;
        }
        else
        {
            constValue.ty = gcSL_UINT32;
        }

        if (Generate20BitsImmediate(CodeGen, Instruction, 0))
        {
            gcmVERIFY_OK(gcEncodeSourceImmediate20(States,
                                                   2,
                                                   &constValue));
        }
        else
        {
            gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                           CodeGen,
                                           constValue.value.u,
                                           &index,
                                           &swizzle,
                                           &constType));
            _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ? 27:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 27:25) - (0 ?
 27:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ?
 27:25)));
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 22:22) - (0 ?
 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
 22:22)));
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ? 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 23:23) - (0 ?
 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23)));
        }
        break;

    default:
        break;
    }
    return gcvTRUE;
}

static gctBOOL
long_ulong_set_lower(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_FORMAT format;
    format = gcmSL_TARGET_GET(Instruction->temp, Format);
    if(format == gcSL_UINT64) format = gcSL_UINT32;
    else format = gcSL_INT32;

    _SetValueType0(type_conv[format], States);

    return gcvTRUE;
}

static gctBOOL
long_ulong_set_upper(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_FORMAT format;
    gctUINT address;

    format = gcmSL_TARGET_GET(Instruction->temp, Format);
    if(format == gcSL_UINT64) format = gcSL_UINT32;
    else format = gcSL_INT32;

    _SetValueType0(type_conv[format], States);

    address = (((((gctUINT32) (States[0])) >> (0 ? 22:16)) & ((gctUINT32) ((((1 ? 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1)))))) ) + 1;
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ?
 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ?
 22:16)));

    return gcvTRUE;
}
#else
static gctBOOL
denorm_value_type0_zero_2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT    index      = 0;
    gctUINT8  swizzle    = 0;
    gcSL_TYPE constType;

    if (Generate20BitsImmediate(CodeGen, Instruction, 0))
    {
        gcsConstantValue constValue;
        constValue.ty = gcmSL_TARGET_GET(Instruction->temp, Format);
        constValue.value.u = 0;

        gcmVERIFY_OK(gcEncodeSourceImmediate20(States,
                                               2,
                                               &constValue));
    }
    else
    {
        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                       CodeGen,
                                       0,
                                       &index,
                                       &swizzle,
                                       &constType));

        _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);
    }

    value_type0(Tree, CodeGen, Instruction, States);

    if (CodeGen->hasSHEnhancements2)
    {
        swizzle = (((((gctUINT32) (States[1])) >> (0 ? 10:3)) & ((gctUINT32) ((((1 ? 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1)))))) );
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3))) | (((gctUINT32) ((gctUINT32) (swizzle | 0x80) & ((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3)));
    }
    return gcvTRUE;
}

#endif
#endif

static gctBOOL
_isCL_X_Signed_8_16(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    if (CodeGen->isCL_X && !CodeGen->hasBugFixes11)
    {
        gcSL_FORMAT format = gcmSL_TARGET_GET(Instruction->temp, Format);

        switch (format)
        {
        case gcSL_INT8:
        case gcSL_INT16:
            return gcvTRUE;

        default:
            return gcvFALSE;
        }
    }
    else
    {
        return gcvFALSE;
    }
}

DEFINE_WITH_VIR(_isCL_X_Signed_8_16)

static gctBOOL
_isCL_X_Unsigned_8_16(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    if (CodeGen->isCL_X && !CodeGen->hasBugFixes11)
    {
        gcSL_FORMAT format = gcmSL_TARGET_GET(Instruction->temp, Format);

        switch (format)
        {
        case gcSL_UINT8:
        case gcSL_UINT16:
            return gcvTRUE;

        default:
            return gcvFALSE;
        }
    }
    else
    {
        return gcvFALSE;
    }
}
DEFINE_WITH_VIR(_isCL_X_Unsigned_8_16)

static gctBOOL
_isCL_X_Signed_8_16_store(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    if (CodeGen->isCL_X && !CodeGen->hasBugFixes11)
    {
        gcSL_FORMAT format = gcmSL_TARGET_GET(Instruction->temp, Format);
        gctINT index = gcmSL_INDEX_GET(Instruction->tempIndex, Index);

        switch (format)
        {
        case gcSL_INT8:
            if (Tree->tempArray[index].format != gcSL_INT8)
            {
                return gcvTRUE;
            }
            return gcvFALSE;

        case gcSL_INT16:
            if (Tree->tempArray[index].format != gcSL_INT8
            &&  Tree->tempArray[index].format != gcSL_INT16)
            {
                return gcvTRUE;
            }
            return gcvFALSE;

        default:
            return gcvFALSE;
        }
    }
    else
    {
        return gcvFALSE;
    }
}

static gctBOOL
_isCL_X_Unsigned_8_16_store(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    if (CodeGen->isCL_X && !CodeGen->hasBugFixes11)
    {
        gcSL_FORMAT format = gcmSL_TARGET_GET(Instruction->temp, Format);
        gctINT index = gcmSL_INDEX_GET(Instruction->tempIndex, Index);

        switch (format)
        {
        case gcSL_UINT8:
            if (Tree->tempArray[index].format != gcSL_UINT8)
            {
                return gcvTRUE;
            }
            return gcvFALSE;

        case gcSL_UINT16:
            if (Tree->tempArray[index].format != gcSL_UINT8
            &&  Tree->tempArray[index].format != gcSL_UINT16)
            {
                return gcvTRUE;
            }
            return gcvFALSE;

        default:
            return gcvFALSE;
        }
    }
    else
    {
        return gcvFALSE;
    }
}

static gctBOOL
_NoLabel_isCL_X_Signed_8_16_store1(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctPTRDIFF_T pc = Instruction - Tree->shader->code;

    if (Tree->hints[pc].callers != gcvNULL)
    {
        return gcvFALSE;
    }

    if (CodeGen->isCL_X && !CodeGen->hasBugFixes11)
    {
        gcSL_FORMAT format = gcmSL_TARGET_GET(Instruction->temp, Format);
        gctINT index = gcmSL_INDEX_GET(Instruction->source1Index, Index);

        if (gcmSL_SOURCE_GET(Instruction->source1, Type) != gcSL_TEMP &&
            gcmSL_SOURCE_GET(Instruction->source1, Type) != gcSL_UNIFORM)
            return gcvFALSE;

        switch (format)
        {
        case gcSL_INT8:
            if (Tree->tempArray[index].format != gcSL_INT8)
            {
                return gcvTRUE;
            }
            return gcvFALSE;

        case gcSL_INT16:
            if (Tree->tempArray[index].format != gcSL_INT8
            &&  Tree->tempArray[index].format != gcSL_INT16)
            {
                return gcvTRUE;
            }
            return gcvFALSE;

        default:
            return gcvFALSE;
        }
    }
    else
    {
        return gcvFALSE;
    }
}

static gctBOOL
_NoLabel_isCL_X_Unsigned_8_16_store1(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctPTRDIFF_T pc = Instruction - Tree->shader->code;

    if (Tree->hints[pc].callers != gcvNULL)
    {
        return gcvFALSE;
    }

    if (CodeGen->isCL_X && !CodeGen->hasBugFixes11)
    {
        gcSL_FORMAT format = gcmSL_TARGET_GET(Instruction->temp, Format);
        gctINT index = gcmSL_INDEX_GET(Instruction->source1Index, Index);

        if (gcmSL_SOURCE_GET(Instruction->source1, Type) != gcSL_TEMP &&
            gcmSL_SOURCE_GET(Instruction->source1, Type) != gcSL_UNIFORM)
            return gcvFALSE;

        switch (format)
        {
        case gcSL_UINT8:
            if (Tree->tempArray[index].format != gcSL_UINT8)
            {
                return gcvTRUE;
            }
            return gcvFALSE;

        case gcSL_UINT16:
            if (Tree->tempArray[index].format != gcSL_UINT8
            &&  Tree->tempArray[index].format != gcSL_UINT16)
            {
                return gcvTRUE;
            }
            return gcvFALSE;

        default:
            return gcvFALSE;
        }
    }
    else
    {
        return gcvFALSE;
    }
}

static gctBOOL
_NoLabel_isnotCL_X(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctPTRDIFF_T pc = Instruction - Tree->shader->code;

    if (Tree->hints[pc].callers != gcvNULL)
    {
        return gcvFALSE;
    }

    if (CodeGen->isCL_X)
    {
        return gcvFALSE;
    }
    else
    {
        return gcvTRUE;
    }
}

static gctBOOL
_is_dest_16bit_src_int8(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (!CodeGen->isCL_X || CodeGen->hasBugFixes11)
    {
        gctUINT format = gcmSL_TARGET_GET(Instruction->temp, Format);

        if (gcmSL_SOURCE_GET(Instruction->source0, Type) != gcSL_TEMP &&
            gcmSL_SOURCE_GET(Instruction->source0, Type) != gcSL_UNIFORM)
        {
            return gcvFALSE;
        }

        if (format == gcSL_INT16 || format == gcSL_UINT16)
        {
            gctUINT32 srcFormat = Instruction->source1Index | (Instruction->source1Indexed << 16);

            if (srcFormat == gcSL_INT8)
            {
                return gcvTRUE;
            }
        }
    }

    return gcvFALSE;
}

DEFINE_WITH_VIR(_is_dest_16bit_src_int8)

static gctBOOL
_is_dest_32bit_src_int8(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (!CodeGen->isCL_X || CodeGen->hasBugFixes11)
    {
        gctUINT format = gcmSL_TARGET_GET(Instruction->temp, Format);

        if (gcmSL_SOURCE_GET(Instruction->source0, Type) != gcSL_TEMP &&
            gcmSL_SOURCE_GET(Instruction->source0, Type) != gcSL_UNIFORM)
        {
            return gcvFALSE;
        }

        if (format == gcSL_INT32 || format == gcSL_UINT32)
        {
            gctUINT32 srcFormat = Instruction->source1Index | (Instruction->source1Indexed << 16);

            if (srcFormat == gcSL_INT8)
            {
                return gcvTRUE;
            }
        }
    }

    return gcvFALSE;
}

DEFINE_WITH_VIR(_is_dest_32bit_src_int8)

static gctBOOL
_is_dest_32bit_src_int16(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (!CodeGen->isCL_X || CodeGen->hasBugFixes11)
    {
        gctUINT format = gcmSL_TARGET_GET(Instruction->temp, Format);

        if (gcmSL_SOURCE_GET(Instruction->source0, Type) != gcSL_TEMP &&
            gcmSL_SOURCE_GET(Instruction->source0, Type) != gcSL_UNIFORM)
        {
            return gcvFALSE;
        }

        if (format == gcSL_INT32 || format == gcSL_UINT32)
        {
            gctUINT32 srcFormat = Instruction->source1Index | (Instruction->source1Indexed << 16);

            if (srcFormat == gcSL_INT16)
            {
                return gcvTRUE;
            }
        }
    }

    return gcvFALSE;
}

DEFINE_WITH_VIR(_is_dest_32bit_src_int16)

static gctBOOL
_is_dest_8bit(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (!CodeGen->isCL_X || CodeGen->hasBugFixes11)
    {
        gctUINT format = gcmSL_TARGET_GET(Instruction->temp, Format);

        if (format == gcSL_INT8 || format == gcSL_UINT8)
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

DEFINE_WITH_VIR(_is_dest_8bit)

static gctBOOL
_is_dest_16bit(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (!CodeGen->isCL_X || CodeGen->hasBugFixes11)
    {
        gctUINT format = gcmSL_TARGET_GET(Instruction->temp, Format);

        if (format == gcSL_INT16 || format == gcSL_UINT16)
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

DEFINE_WITH_VIR(_is_dest_16bit)

static gctBOOL
_codeHasCaller(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen
    )
{
    return (Tree->hints[CodeGen->nextSource - 1].callers != gcvNULL);
}

static gctBOOL
_shaderNeedIntSupport(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    return (CodeGen->clShader || CodeGen->computeShader || CodeGen->haltiShader || CodeGen->dxShader);
}

static gctBOOL
_isCLShader_hasNEW_SIN_COS_LOG_DIV(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    return CodeGen->clShader && CodeGen->hasNEW_SIN_COS_LOG_DIV;
}

static gctBOOL
_isNotCLShader(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    return ! CodeGen->clShader;
}
static gctBOOL
_hasInteger(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    return CodeGen->hasInteger;
}

static gctBOOL
_hasRounding_mode(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32_PTR States
    )
{
    if (CodeGen->hasSHEnhancements2)
    {
        gcSL_ROUND round = gcmSL_OPCODE_GET(Instruction->opcode, Round);
        return (round != gcSL_ROUND_DEFAULT);
    }

    return gcvFALSE;
}

gctBOOL
value_type0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT format = gcmSL_TARGET_GET(Instruction->temp, Format);
    gctUINT value_type0 = type_conv[format];
    gctUINT inst_type0 = value_type0 & 0x1;
    gctUINT inst_type1 = value_type0 >> 1;

    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) ((gctUINT32) (inst_type0) & ((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21)));
    States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (inst_type1) & ((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30)));

    return gcvTRUE;
}

void
_SetValueType0(
    IN gctUINT ValueType,
    IN OUT gctUINT32 * States
    )
{
    gctUINT instType0 = ValueType & 0x1;
    gctUINT instType1 = ValueType >> 1;

    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) ((gctUINT32) (instType0) & ((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21)));
    States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (instType1) & ((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30)));
}


static gctBOOL
value_types_I2I(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_FORMAT format;
    gctUINT value_type;
    gctUINT inst_type0;
    gctUINT inst_type1;
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    if(gcmSL_OPCODE_GET(Instruction->opcode, Opcode) == gcSL_CONV)
    {
        /* get the convert from type */
        gcmASSERT(gcmSL_SOURCE_GET(Instruction->source1, Type) == gcSL_CONSTANT);
        /* Assemble the 32-bit value. */
        gcmASSERT(gcmSL_SOURCE_GET(Instruction->source1, Format) == gcSL_UINT32);
        format = (gcSL_FORMAT)(Instruction->source1Index |
                                Instruction->source1Indexed << 16);
    }
    else
    {
        format = (gcSL_FORMAT)gcmSL_SOURCE_GET(Instruction->source0, Format);
    }
    value_type = type_conv[format];
    inst_type0 = value_type & 0x1;
    inst_type1 = value_type >> 1;

    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) ((gctUINT32) (inst_type0) & ((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21)));
    States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (inst_type1) & ((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30)));

    format = gcmSL_TARGET_GET(Instruction->temp, Format);
    value_type = type_conv[format] << 4;
    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                   CodeGen,
                                   value_type,
                                   &index,
                                   &swizzle,
                                   &constType));

    _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);
    return gcvTRUE;
}

static gctBOOL
float16_sign(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                  CodeGen,
                                  0x8000u,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x6, States);

    return gcvTRUE;
}

static gctBOOL
float16_exp_bits(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                  CodeGen,
                                  0x10u,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x5, States);

    return gcvTRUE;
}

static gctBOOL
float16_exp(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                  CodeGen,
                                  0x7C00u,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x6, States);

    return gcvTRUE;
}

static gctBOOL
float16_man_bits(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                  CodeGen,
                                  0xDu,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x5, States);

    return gcvTRUE;
}

static gctBOOL
float16_man(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                  CodeGen,
                                  0x3FFu,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x6, States);

    return gcvTRUE;
}

static gctBOOL
float16_exp_iszero(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x05 & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                  CodeGen,
                                  0,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                  CodeGen,
                                  0x38000000u,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x5, States);

    return gcvTRUE;
}

static gctBOOL
float16_exp_isnan(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                  CodeGen,
                                  0x0F800000u,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                  CodeGen,
                                  1,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x5, States);

    return gcvTRUE;
}

static gctBOOL
float16_exp_isaddnanNZ(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x0B & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                  CodeGen,
                                  0x7000000u,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x5, States);

    return gcvTRUE;
}

static gctBOOL
value_types_u32(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    _SetValueType0(0x5, States);

    return gcvTRUE;
}

static gctBOOL
_isF16_2_F32_hasCMP(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_FORMAT formatDest;
    gcSL_FORMAT formatSource;

    /* get the convert from type */
    gcmASSERT(gcmSL_SOURCE_GET(Instruction->source1, Type) == gcSL_CONSTANT);
    /* Assemble the 32-bit value. */
    gcmASSERT(gcmSL_SOURCE_GET(Instruction->source1, Format) == gcSL_UINT32);

    if (!CodeGen->hasCL)
    {
        return gcvFALSE;
    }

    formatDest = (gcSL_FORMAT)gcmSL_TARGET_GET(Instruction->temp, Format);
    formatSource = (gcSL_FORMAT)(Instruction->source1Index |
                            Instruction->source1Indexed << 16);

    if (formatDest == gcSL_FLOAT && formatSource == gcSL_FLOAT16)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

DEFINE_WITH_VIR(_isF16_2_F32_hasCMP)

static gctBOOL
float32_sign(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                  CodeGen,
                                  0x80000000u,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x5, States);

    return gcvTRUE;
}

static gctBOOL
float32_exp_isnan(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                  CodeGen,
                                  0x7F800000u,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                  CodeGen,
                                  1,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x5, States);

    return gcvTRUE;
}

static gctBOOL
float32_exp_bits(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                  CodeGen,
                                  0x10u,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x5, States);

    return gcvTRUE;
}

static gctBOOL
float32_exp(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                  CodeGen,
                                  0x7F800000u,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x5, States);

    return gcvTRUE;
}

static gctBOOL
float32_man_bits(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                  CodeGen,
                                  0xDu,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x5, States);

    return gcvTRUE;
}

static gctBOOL
float32_man(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                  CodeGen,
                                  0x7FFFFFu,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x5, States);

    return gcvTRUE;
}

static gctBOOL
value_types_16(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    _SetValueType0(0x6, States);

    return gcvTRUE;
}

static gctBOOL
_isF32_2_F16_hasCMP(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_FORMAT formatDest;
    gcSL_FORMAT formatSource;

    /* get the convert from type */
    gcmASSERT(gcmSL_SOURCE_GET(Instruction->source1, Type) == gcSL_CONSTANT);
    /* Assemble the 32-bit value. */
    gcmASSERT(gcmSL_SOURCE_GET(Instruction->source1, Format) == gcSL_UINT32);

    formatDest = (gcSL_FORMAT)gcmSL_TARGET_GET(Instruction->temp, Format);
    formatSource = (gcSL_FORMAT)(Instruction->source1Index |
                            Instruction->source1Indexed << 16);

    if (!CodeGen->hasCL)
    {
        return gcvFALSE;
    }

    if (formatDest == gcSL_FLOAT16 && formatSource == gcSL_FLOAT)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

DEFINE_WITH_VIR(_isF32_2_F16_hasCMP)

static gctBOOL
_isF2F(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (CodeGen->hasSHEnhancements2)
    {
        gcSL_FORMAT format0 =
            (gcSL_FORMAT)gcmSL_TARGET_GET(Instruction->temp, Format);
        gcSL_FORMAT format1 =
            (gcSL_FORMAT)gcmSL_SOURCE_GET(Instruction->source0, Format);

        if ((format0 != gcSL_FLOAT && format0 != gcSL_FLOAT16) || (format1 != gcSL_FLOAT && format1 != gcSL_FLOAT16))
        {
            return gcvFALSE;
        }

        /* get the convert from type */
        gcmASSERT(gcmSL_SOURCE_GET(Instruction->source1, Type) == gcSL_CONSTANT);
        /* Assemble the 32-bit value. */
        gcmASSERT(gcmSL_SOURCE_GET(Instruction->source1, Format) == gcSL_UINT32);
        format1 = (gcSL_FORMAT)(Instruction->source1Index |
                                Instruction->source1Indexed << 16);

        if (format0 != format1)
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

static gctBOOL
value_types_F2F(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_FORMAT format;
    gctUINT value_type;
    gctUINT inst_type0;
    gctUINT inst_type1;
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmASSERT(gcmSL_SOURCE_GET(Instruction->source1, Type) == gcSL_CONSTANT);
    /* Assemble the 32-bit value. */
    gcmASSERT(gcmSL_SOURCE_GET(Instruction->source1, Format) == gcSL_UINT32);

    /* Set the dest format. */
    format = (gcSL_FORMAT)gcmSL_TARGET_GET(Instruction->temp, Format);

    value_type = type_conv[format];
    inst_type0 = value_type & 0x1;
    inst_type1 = value_type >> 1;

    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) ((gctUINT32) (inst_type0) & ((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21)));
    States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (inst_type1) & ((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30)));

    /* Set the source format. */
    format = (gcSL_FORMAT)(Instruction->source1Index |
                           Instruction->source1Indexed << 16);
    value_type = type_conv[format];
    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                   CodeGen,
                                   value_type,
                                   &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);

    return gcvTRUE;
}

static gctBOOL
value_type0_32bit_from_src0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT value_type0;
    gctUINT inst_type0;
    gctUINT inst_type1;
    gctUINT format = gcmSL_SOURCE_GET(Instruction->source0, Format);

    /* Select does not support 8/16-bit integer type. */
    /* Convert it to 32-bit for select. */
    if (format == gcSL_INT8 || format == gcSL_INT16)
    {
        format = gcSL_INT32;
    }
    else if (format == gcSL_UINT8 || format == gcSL_UINT16)
    {
        format = gcSL_UINT32;
    }
    value_type0 = type_conv[format];
    inst_type0 = value_type0 & 0x1;
    inst_type1 = value_type0 >> 1;

    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) ((gctUINT32) (inst_type0) & ((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21)));
    States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (inst_type1) & ((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30)));

    return gcvTRUE;
}

static gctBOOL
value_type0_32bit_from_src0_and_delete_second_caller(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_INSTRUCTION code = Instruction + 2;
    gctINT codeIndex = (gctINT)(code - Tree->shader->code);

    value_type0_32bit_from_src0(Tree, CodeGen, Instruction, States);

    deleteCaller(Tree, code->tempIndex, codeIndex);
    return gcvTRUE;
}

static gctBOOL
value_type0_from_src0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT format;
    gctUINT value_type0;
    gctUINT inst_type0;
    gctUINT inst_type1;

    /* Inst type of img_load is regareded as result type which is converted from src img fmt. */
    if (Instruction->opcode == gcSL_IMAGE_RD || Instruction->opcode == gcSL_IMAGE_RD_3D)
    {
        gcUNIFORM uniform = gcvNULL;

        gcmASSERT(gcmSL_SOURCE_GET(Instruction->source0, Type) == gcSL_UNIFORM);

        uniform = Tree->shader->uniforms[gcmSL_INDEX_GET(Instruction->source0Index, Index)];

        switch (uniform->imageFormat)
        {
        /* float-image-format-qualifier. */
        case gcIMAGE_FORMAT_RGBA32F:
        case gcIMAGE_FORMAT_R32F:
        case gcIMAGE_FORMAT_RGBA16F:
        case gcIMAGE_FORMAT_RGBA8:
        case gcIMAGE_FORMAT_RGBA8_SNORM:
            format =gcSL_FLOAT;
            break;

        /* int-image-format-qualifier. */
        case gcIMAGE_FORMAT_RGBA32I:
        case gcIMAGE_FORMAT_R32I:
        case gcIMAGE_FORMAT_RGBA16I:
        case gcIMAGE_FORMAT_RGBA8I:
            format =gcSL_INTEGER;
            break;

        /* uint-image-format-qualifier. */
        case gcIMAGE_FORMAT_RGBA32UI:
        case gcIMAGE_FORMAT_R32UI:
        case gcIMAGE_FORMAT_RGBA16UI:
        case gcIMAGE_FORMAT_RGBA8UI:
            format =gcSL_UINT32;
            break;

        default:
            gcmASSERT(gcvFALSE);
            format = gcSL_INTEGER;
            break;
        }
    }
    else
    {
        format = gcmSL_SOURCE_GET(Instruction->source0, Format);
    }

    value_type0 = type_conv[format];
    inst_type0 = value_type0 & 0x1;
    inst_type1 = value_type0 >> 1;

    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) ((gctUINT32) (inst_type0) & ((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21)));
    States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (inst_type1) & ((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30)));

    return gcvTRUE;
}

static gctBOOL
canUseSelectCmpSetInst(
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction
    )
{
    gcSL_INSTRUCTION prevInstruction = Instruction - 1;
    gctUINT format = gcmSL_TARGET_GET(Instruction->temp, Format);
    gctUINT prevFormat = gcmSL_SOURCE_GET(prevInstruction->source0, Format);

    if (format != prevFormat)
    {
        return gcvFALSE;
    }

    if (format == gcSL_FLOAT)
    {
        if (CodeGen->clShader)
        {
            if (!gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_SUPPORT_INTEGER_BRANCH))
            {
                return gcvTRUE;
            }
            else
            {
                return gcvFALSE;
            }
        }
        return gcvTRUE;
    }
    else if (CodeGen->isCL_X)
    {
        return gcvFALSE;
    }
    else if (format == gcSL_INT32 || format == gcSL_UINT32)
    {
        return gcvTRUE;
    }
    else
    {
        return gcvFALSE;
    }
}


gctBOOL
rtz(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT value = (((((gctUINT32) (States[1])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) );

    value = (value & 0x4) | 0x1;
    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (value) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));

    return gcvTRUE;
}

/*
    FRAC 1, 2
        frc 1, 0, 0, 2, 0
    gc3000/gc5000 has bug which return 1.0 for FRAC(-2^-20) when RTNE
    need to set RTZ explicitly
*/
gctBOOL
rtz_for_HW_bug(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (CodeGen->hasSHEnhancements2)
    {
        gctUINT value = (((((gctUINT32) (States[1])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) );
        gcePATCH_ID patchID = gcvPATCH_INVALID;
        gcoHAL_GetPatchID(gcvNULL, &patchID);

        if (!(patchID == gcvPATCH_DEQP || patchID == gcvPATCH_OESCTS))
        {
            value = (value & 0x4) | 0x1;
            States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (value) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));
        }
    }
    return gcvTRUE;
}

gctBOOL
_GFX27Patch(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcePATCH_ID patchID = gcvPATCH_INVALID;
    gcoHAL_GetPatchID(gcvNULL, &patchID);

    if (patchID == gcvPATCH_GLBM27)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

gctBOOL
rtne(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT value = (((((gctUINT32) (States[1])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) );

    value = (value & 0x4) | 0x2;
    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (value) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));

    return gcvTRUE;
}

gctBOOL
rounding_mode(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (CodeGen->hasSHEnhancements2)
    {
        gctUINT round = gcmSL_OPCODE_GET(Instruction->opcode, Round);
        gctUINT value = (((((gctUINT32) (States[1])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) );

        value = (value & 0x4) | round;
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (value) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));
    }

    return gcvTRUE;
}

gctBOOL
rounding_mode_value_type0_from_src0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    rounding_mode(Tree, CodeGen, Instruction, States);

    value_type0_from_src0(Tree, CodeGen, Instruction, States);

    return gcvTRUE;
}

gctBOOL
isSource1_RCP_OF_LOG2_E(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (gcmSL_SOURCE_GET(Instruction->source1, Type) == gcSL_CONSTANT)
    {
        if (Instruction->source1Index == 0x7218 && Instruction->source1Indexed == 0x3f31)
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}


static gctUINT32
_sl2gcCondition(
    IN gcSL_CONDITION Condition
    )
{
    switch (Condition)
    {
    case gcSL_ALWAYS:
        return 0x00;

    case gcSL_NOT_EQUAL:
        return 0x06;

    case gcSL_LESS_OR_EQUAL:
        return 0x04;

    case gcSL_LESS:
        return 0x02;

    case gcSL_EQUAL:
        return 0x05;

    case gcSL_GREATER:
        return 0x01;

    case gcSL_GREATER_OR_EQUAL:
        return 0x03;

    case gcSL_AND:
        return 0x07;

    case gcSL_OR:
        return 0x08;

    case gcSL_XOR:
        return 0x09;

    case gcSL_NOT_ZERO:
        return 0x0B;

    case gcSL_ZERO:
        return 0x0A;

    case gcSL_GREATER_OR_EQUAL_ZERO:
        return 0x0C;

    case gcSL_GREATER_ZERO:
        return 0x0D;

    case gcSL_LESS_OREQUAL_ZERO:
        return 0x0E;

    case gcSL_LESS_ZERO:
        return 0x0F;

    case gcSL_ALLMSB:
        return 0x15;

    case gcSL_ANYMSB:
        return 0x14;

    case gcSL_SELMSB:
        return 0x16;

    default:
        gcmFATAL("ERROR: Invalid condition 0x%04X", Condition);
        return 0x00;
    }

}

static gctUINT32
_reverseEqualCondition(
    IN gctUINT32 Condition
    )
{
    switch (Condition)
    {
    case 0x04:
        return 0x02;

    case 0x02:
        return 0x04;

    case 0x01:
        return 0x03;

    case 0x03:
        return 0x01;

    case 0x05:
        return 0x06;

    case 0x06:
        return 0x05;

    default:
        return Condition;
    }
}

/* return TRUE if the condition has coresponding reverse condition,
 *   the reversed condition is returned in ReversedCondition if it
 *   is reversible
 */
gctBOOL
isConditionReversible(
    IN  gcSL_CONDITION   Condition,
    OUT gcSL_CONDITION * ReversedCondition
    )
{
    switch (Condition)
    {
    case gcSL_ALWAYS:
    case gcSL_AND:
    case gcSL_OR:
    case gcSL_XOR:
    case gcSL_ALLMSB:
    case gcSL_ANYMSB:
    case gcSL_SELMSB:
        return gcvFALSE;

    case gcSL_NOT_EQUAL:
        * ReversedCondition = gcSL_EQUAL;
        return gcvTRUE;

    case gcSL_EQUAL:
        * ReversedCondition = gcSL_NOT_EQUAL;
        return gcvTRUE;

    case gcSL_LESS_OR_EQUAL:
        * ReversedCondition = gcSL_GREATER;
        return gcvTRUE;

    case gcSL_GREATER:
        * ReversedCondition = gcSL_LESS_OR_EQUAL;
        return gcvTRUE;

    case gcSL_LESS:
        * ReversedCondition = gcSL_GREATER_OR_EQUAL;
        return gcvTRUE;

    case gcSL_GREATER_OR_EQUAL:
        * ReversedCondition = gcSL_LESS;
        return gcvTRUE;

    case gcSL_NOT_ZERO:
        * ReversedCondition = gcSL_ZERO;
        return gcvTRUE;

    case gcSL_ZERO:
        * ReversedCondition = gcSL_NOT_ZERO;
        return gcvTRUE;

    case gcSL_GREATER_OR_EQUAL_ZERO:
        * ReversedCondition = gcSL_LESS_ZERO;
        return gcvTRUE;

    case gcSL_GREATER_ZERO:
        * ReversedCondition = gcSL_LESS_OREQUAL_ZERO;
        return gcvTRUE;

    case gcSL_LESS_OREQUAL_ZERO:
        * ReversedCondition = gcSL_GREATER_ZERO;
        return gcvTRUE;

    case gcSL_LESS_ZERO:
        * ReversedCondition = gcSL_GREATER_OR_EQUAL_ZERO;
        return gcvTRUE;

    default:
        gcmFATAL("ERROR: Invalid condition 0x%04X", Condition);
        return gcvFALSE;
    }
}

/* return true if Instruction source SrcNo has floating point constant Value */
gctBOOL
isSourceConstantf(
    IN gcSL_INSTRUCTION Instruction,
    IN gctINT           SrcNo,
    IN gctFLOAT         Value
    )
{
    gctSOURCE_t src = SrcNo == 0 ? Instruction->source0 : Instruction->source1;

    if (gcmSL_TARGET_GET(Instruction->temp, Format) == gcSL_FLOAT)
    {
        if (gcmSL_SOURCE_GET(src, Type) == gcSL_CONSTANT)
        {
            gctINT val = 0;
            /* Assemble the 32-bit value. */
            switch (SrcNo) {
            case 0:
                val = (Instruction->source0Index & 0xFFFF) | (Instruction->source0Indexed << 16);
                break;
            case 1:
                val = (Instruction->source1Index & 0xFFFF) | (Instruction->source1Indexed << 16);
                break;
            default:
                gcmASSERT(0);
                return gcvFALSE;
            }

            return gcoMATH_UIntAsFloat(val) == Value;
        }

    }
    return gcvFALSE;
}

/* return true if Instruction source SrcNo has integer constant Value */
gctBOOL
isSourceConstanti(
    IN gcSL_INSTRUCTION Instruction,
    IN gctINT           SrcNo,
    IN gctINT           Value
    )
{
    gctSOURCE_t src = SrcNo == 0 ? Instruction->source0 : Instruction->source1;

    if (gcmSL_TARGET_GET(Instruction->temp, Format) == gcSL_INTEGER ||
        gcmSL_TARGET_GET(Instruction->temp, Format) == gcSL_UINT32)
    {
        if (gcmSL_SOURCE_GET(src, Type) == gcSL_CONSTANT)
        {
            gctINT val = 0;
            /* Assemble the 32-bit value. */
            switch (SrcNo) {
            case 0:
                val = (Instruction->source0Index & 0xFFFF) | (Instruction->source0Indexed << 16);
                break;
            case 1:
                val = (Instruction->source1Index & 0xFFFF) | (Instruction->source1Indexed << 16);
                break;
            default:
                gcmASSERT(0);
                return gcvFALSE;
            }

            return val == Value;
        }

    }
    return gcvFALSE;
}

/* return TRUE if the condition can only use one operand, the gl_Condition
 * is converted to gcCondition and returned in *GcCondition
 */
gctBOOL
isConditionCanBeOneOperand(
    IN gcSL_INSTRUCTION Instruction,
    OUT gctUINT32 *     GcCondition,
    OUT gctUINT32 *     UseSrc1
    )
{
    gcSL_CONDITION Condition = (gcSL_CONDITION)gcmSL_TARGET_GET(Instruction->temp,
                                                                  Condition);
    *UseSrc1  = 0;
    switch (Condition)
    {
    case gcSL_ALWAYS:
        *GcCondition = 0x00;
        return gcvTRUE;
    case gcSL_AND:
    case gcSL_OR:
    case gcSL_XOR:
        return gcvFALSE;

    /* check if the two operand condition uses constant zero as one operand
       and convert the condition to compare to zero gcCondition
    */
    case gcSL_EQUAL:
    case gcSL_NOT_EQUAL:
    case gcSL_LESS_OR_EQUAL:
    case gcSL_GREATER:
    case gcSL_LESS:
    case gcSL_GREATER_OR_EQUAL:
        {
            if (isSourceConstantf(Instruction, 0, 0.0) ||
                isSourceConstanti(Instruction, 0, 0)     )
            {
                switch (Condition)
                {
                case gcSL_EQUAL:
                   /* 0.0 == src1  ==>  !src1  */
                    *GcCondition = 0x0A;
                    break;
                case gcSL_NOT_EQUAL:
                   /* 0.0 != src1  ==>  src1 != 0.0  */
                   *GcCondition = 0x0B;
                    break;
                /* the first operand is 0.0, reverse the following condition */
                case gcSL_LESS_OR_EQUAL:
                    /* 0.0 <= src1  ==>  src1 >= 0.0  */
                    *GcCondition = 0x0C;
                    break;
                case gcSL_GREATER:
                    /* 0.0 > src1  ==>  src1 < 0.0  */
                    *GcCondition = 0x0F;
                    break;
                case gcSL_LESS:
                    /* 0.0 < src1  ==>  src1 > 0.0  */
                    *GcCondition = 0x0D;
                    break;
                case gcSL_GREATER_OR_EQUAL:
                    /* 0.0 >= src1  ==>  src1 <= 0.0  */
                    *GcCondition = 0x0E;
                    break;
                default:
                    return gcvFALSE;
                }
                *UseSrc1  = 1;
                return gcvTRUE;
            }
            else if (isSourceConstantf(Instruction, 1, 0.0) ||
                     isSourceConstanti(Instruction, 1, 0)     )
            {
                switch (Condition)
                {
                case gcSL_EQUAL:
                   /* src0 == 0.0  ==>  !src0  */
                    *GcCondition = 0x0A;
                    break;
                case gcSL_NOT_EQUAL:
                   /* src0 == 0.0  */
                   *GcCondition = 0x0B;
                    break;
                case gcSL_LESS_OR_EQUAL:
                    /* src0 <= 0.0  */
                    *GcCondition = 0x0E;
                    break;
                case gcSL_GREATER:
                    /* src0 > 0.0  */
                    *GcCondition = 0x0D;
                    break;
                case gcSL_LESS:
                    /* src0 < 0.0  */
                    *GcCondition = 0x0F;
                    break;
                case gcSL_GREATER_OR_EQUAL:
                    /* src0 >= 0.0  */
                    *GcCondition = 0x0C;
                    break;
                default:
                    return gcvFALSE;
                }
                return gcvTRUE;
             }
            return gcvFALSE;
        }

    case gcSL_NOT_ZERO:
        * GcCondition = 0x0B;
        return gcvTRUE;

    case gcSL_ZERO:
        * GcCondition = 0x0A;
        return gcvTRUE;

    case gcSL_ALLMSB:
        * GcCondition = 0x15;
        return gcvTRUE;

    case gcSL_ANYMSB:
        * GcCondition = 0x14;
        return gcvTRUE;

    case gcSL_SELMSB:
        * GcCondition = 0x16;
        return gcvTRUE;

    default:
        return gcvFALSE;
    }
}
/* only apply on reversible condition */
static gctUINT32
_Reverse_sl2gcCondition(
    IN gcSL_CONDITION Condition
    )
{
    gcSL_CONDITION reversedCondition;
    gctBOOL reversible = isConditionReversible(Condition, &reversedCondition);
    if (!reversible)
    {
        gcmFATAL("ERROR: Invalid condition 0x%04X", Condition);
    }
    return _sl2gcCondition(reversedCondition);
}

static gctBOOL
saturate(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ? 11:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));

    return gcvTRUE;
}

static gctBOOL
conditionLT(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x02 & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));
    if (_shaderNeedIntSupport(Tree, CodeGen, Instruction, States))
    {
        value_type0_32bit_from_src0(Tree, CodeGen, Instruction, States);
    }
    return gcvTRUE;
}

static gctBOOL
conditionLTAbs1(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x02 & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));
    gcSetSrcABS(States, 1);
    if (_shaderNeedIntSupport(Tree, CodeGen, Instruction, States))
    {
        value_type0_32bit_from_src0(Tree, CodeGen, Instruction, States);
    }
    return gcvTRUE;
}

static gctBOOL
conditionNZ(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x0B & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));
    if (_shaderNeedIntSupport(Tree, CodeGen, Instruction, States))
    {
        value_type0(Tree, CodeGen, Instruction, States);
    }
    return gcvTRUE;
}

static gctBOOL
conditionNZ_from_next_inst(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_INSTRUCTION nextInstruction = Instruction + 1;

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x0B & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));
    if (_shaderNeedIntSupport(Tree, CodeGen, nextInstruction, States))
    {
        value_type0(Tree, CodeGen, nextInstruction, States);
    }
    return gcvTRUE;
}

static gctBOOL
conditionNZ_value_type0_from_src0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT format = gcmSL_SOURCE_GET(Instruction->source0, Format);
    gctUINT value_type_0 = type_conv[format];
    gctUINT inst_type0 = value_type_0 & 0x1;
    gctUINT inst_type1 = value_type_0 >> 1;

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x0B & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));
    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) ((gctUINT32) (inst_type0) & ((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21)));
    States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (inst_type1) & ((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30)));
    if (_shaderNeedIntSupport(Tree, CodeGen, Instruction, States))
    {
        value_type0(Tree, CodeGen, Instruction, States);
    }

    return gcvTRUE;
}

static gctBOOL
first_condition_value_type0_from_src0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_CONDITION condition = gcmSL_TARGET_GET(Instruction->temp, Condition);
    gctUINT format = gcmSL_SOURCE_GET(Instruction->source0, Format);
    gctUINT value_type_0 = type_conv[format];
    gctUINT inst_type0 = value_type_0 & 0x1;
    gctUINT inst_type1 = value_type_0 >> 1;

    switch (condition)
    {
        case gcSL_ZERO:
            States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x0A & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));
            break;
        case gcSL_NOT_ZERO:
            States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x0B & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));
            break;
        case gcSL_LESS_ZERO:
            States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));
            break;
        case gcSL_LESS_OREQUAL_ZERO:
            States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x0E & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));
            break;
        case gcSL_GREATER_ZERO:
            States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));
            break;
        case gcSL_GREATER_OR_EQUAL_ZERO:
            States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x0C & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));
            break;
        case gcSL_ALLMSB:
            States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x15 & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));
            break;
        case gcSL_ANYMSB:
            States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x14 & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));
            break;
        case gcSL_SELMSB:
            States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x16 & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));
            break;
        default:
            gcmASSERT(0);
    }

    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) ((gctUINT32) (inst_type0) & ((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21)));
    States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (inst_type1) & ((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30)));
    if (CodeGen->hasSHEnhancements2)
    {
        gctUINT8 swizzle = (((((gctUINT32) (States[1])) >> (0 ? 10:3)) & ((gctUINT32) ((((1 ? 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1)))))) );
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3))) | (((gctUINT32) ((gctUINT32) (swizzle | 0x80) & ((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3)));
    }
    return gcvTRUE;
}

static gctBOOL
conditionGE(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x03 & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));
    if (_shaderNeedIntSupport(Tree, CodeGen, Instruction, States))
    {
        value_type0(Tree, CodeGen, Instruction, States);
    }
    return gcvTRUE;
}

static gctBOOL
_HasModInAllUses(
    IN gcLINKTREE Tree,
    IN gctINT defTemp
    )
{
    gcsLINKTREE_LIST_PTR users;
    gcSL_INSTRUCTION code;
    gctBOOL hasSrcMod = gcvFALSE;

    users = Tree->tempArray[defTemp].users;

    while (users)
    {
        code = &Tree->shader->code[users->index];

        gcmASSERT(code);

        if (gcmSL_SOURCE_GET(code->source0, Type) == gcSL_TEMP &&
            gcmSL_INDEX_GET(code->source0Index, Index) == defTemp)
        {
            if (gcmSL_SOURCE_GET(code->source0, Neg) || gcmSL_SOURCE_GET(code->source0, Abs))
            {
                hasSrcMod = gcvTRUE;
                break;
            }
        }

        if (gcmSL_SOURCE_GET(code->source1, Type) == gcSL_TEMP &&
            gcmSL_INDEX_GET(code->source1Index, Index) == defTemp)
        {
            if (gcmSL_SOURCE_GET(code->source1, Neg) || gcmSL_SOURCE_GET(code->source1, Abs))
            {
                hasSrcMod = gcvTRUE;
                break;
            }
        }

        users = users->next;
    }

    return hasSrcMod;
}

static gctBOOL
_UseDestInNextOnly(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT temp = Instruction->tempIndex;
    if ((Tree->tempArray[temp].users == gcvNULL)
    ||  (Tree->tempArray[temp].users->next != gcvNULL)
    )
    {
        return gcvFALSE;
    }

    if (_HasModInAllUses(Tree, temp))
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

static gctBOOL
_UseDestInNextOnly_Dual16OnMediumpSrc0Src1(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT temp = Instruction->tempIndex;
    if ((Tree->tempArray[temp].users == gcvNULL)
    ||  (Tree->tempArray[temp].users->next != gcvNULL)
    )
    {
        return gcvFALSE;
    }

    if (_HasModInAllUses(Tree, temp))
    {
        return gcvFALSE;
    }

    if (CodeGen->isDual16Shader &&
        gcmSL_SOURCE_GET(Instruction->source0, Precision) == gcSL_PRECISION_MEDIUM &&
        gcmSL_SOURCE_GET(Instruction->source1, Precision) == gcSL_PRECISION_MEDIUM) {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_UseDestInNextOnly_AbsOnSrc0_SameSrc0AsSrc0InPrev(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT temp = Instruction->tempIndex;
    gcSL_INSTRUCTION prev_inst = Instruction - 1;
    gctSOURCE_t src0 = GetInstSource0(Instruction);
    gctSOURCE_t prev_src0 = GetInstSource0(prev_inst);

    if(!gcmSL_SOURCE_GET(src0, Abs))
    {
        return gcvFALSE;
    }
    src0 = gcmSL_SOURCE_SET(src0, Abs, 0);
    if(src0 != prev_src0)
    {
        return gcvFALSE;
    }

    if ((Tree->tempArray[temp].users == gcvNULL)
    ||  (Tree->tempArray[temp].users->next != gcvNULL)
    )
    {
        return gcvFALSE;
    }

    if (_HasModInAllUses(Tree, temp))
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

#if (GC_ENABLE_DUAL_FP16 > 0)
/*
    { 2, gcSL_MUL, 1, 2, 3, 0, 0, _UseDestInNextOnly_ConstSrc1AndDual16On },
    { 1, gcSL_FRAC, 4, 1 },
            { -4, 0x03, 1, 2, 3, 0, 0, _t0_destHP },
            { -3, 0x13, 4, 0, 0, 1, 0, _t0_src2HP},
            { -2, 0x03, 1, 2, 3, 0, 0, _t1_destHP },
            { -1, 0x13, 4, 0, 0, 1, 0, _t1_src2HP},
*/

static gctBOOL
_UseDestInNextOnly_ConstSrc1AndDual16On(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (!CodeGen->isDual16Shader ||
        !_UseDestInNextOnly(Tree, CodeGen, Instruction, States) ||
        (gcmSL_SOURCE_GET(Instruction->source0, Type) != gcSL_CONSTANT &&
         gcmSL_SOURCE_GET(Instruction->source1, Type) != gcSL_CONSTANT    ) )
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

static gctBOOL
_Dual16OnMediumpDstMediumpSrc0HighpSrc1(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (CodeGen->isDual16Shader &&
        gcmSL_TARGET_GET(Instruction->temp, Precision) == gcSL_PRECISION_MEDIUM &&
        gcmSL_SOURCE_GET(Instruction->source0, Precision) == gcSL_PRECISION_MEDIUM &&
        gcmSL_SOURCE_GET(Instruction->source1, Precision) == gcSL_PRECISION_HIGH &&
        gcmSL_SOURCE_GET(Instruction->source1, Type) == gcSL_UNIFORM)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_Dual16OnHighpDstMediumpSrc0HighpSrc1(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (CodeGen->isDual16Shader &&
        gcmSL_TARGET_GET(Instruction->temp, Precision) == gcSL_PRECISION_HIGH &&
        gcmSL_SOURCE_GET(Instruction->source0, Precision) == gcSL_PRECISION_MEDIUM &&
        gcmSL_SOURCE_GET(Instruction->source1, Precision) == gcSL_PRECISION_HIGH &&
        gcmSL_SOURCE_GET(Instruction->source1, Type) == gcSL_UNIFORM)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_Dual16OnMediumpDstHighpSrc0HighpSrc1(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (CodeGen->isDual16Shader &&
        gcmSL_TARGET_GET(Instruction->temp, Precision) == gcSL_PRECISION_MEDIUM &&
        gcmSL_SOURCE_GET(Instruction->source0, Precision) == gcSL_PRECISION_HIGH &&
        gcmSL_SOURCE_GET(Instruction->source1, Precision) == gcSL_PRECISION_HIGH &&
        (gcmSL_SOURCE_GET(Instruction->source0, Type) == gcSL_UNIFORM ||
         gcmSL_SOURCE_GET(Instruction->source1, Type) == gcSL_UNIFORM))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_Dual16OnMediumpDstHighpSrc0MediumpSrc1(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (CodeGen->isDual16Shader &&
        gcmSL_TARGET_GET(Instruction->temp, Precision) == gcSL_PRECISION_HIGH &&
        gcmSL_SOURCE_GET(Instruction->source0, Precision) == gcSL_PRECISION_HIGH &&
        gcmSL_SOURCE_GET(Instruction->source1, Precision) == gcSL_PRECISION_MEDIUM &&
        gcmSL_SOURCE_GET(Instruction->source0, Type) == gcSL_UNIFORM)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

/* set dest address to thread T0 */
static gctBOOL
destAddrT0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    /* Set to 0x1 */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:13) - (0 ? 13:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:13) - (0 ? 13:13) + 1))))))) << (0 ?
 13:13))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ? 13:13) - (0 ?
 13:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:13) - (0 ? 13:13) + 1))))))) << (0 ?
 13:13)));
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 24:24) - (0 ?
 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24)));

    return gcvTRUE;
}

/* set dest address to thread T1 */
static gctBOOL
destAddrT1(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    /* Set to 0x2 */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:13) - (0 ? 13:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:13) - (0 ? 13:13) + 1))))))) << (0 ?
 13:13))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 13:13) - (0 ?
 13:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:13) - (0 ? 13:13) + 1))))))) << (0 ?
 13:13)));
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ? 24:24) - (0 ?
 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24)));

    return gcvTRUE;
}

/* set dest address to thread T0 - Highp dest, Mediump/highp Src0, Mediump/highp Src1 */
static gctBOOL
_t0_destHP(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    /* set dest type to highp */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)));

    if(gcmSL_SOURCE_GET(Instruction->source0, Type) == gcSL_ATTRIBUTE && /* check for sources being gl_FragCoord */
       CodeGen->usePosition && Instruction->source0Index == CodeGen->positionIndex)
    {
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 5:3) - (0 ?
 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3)));
    }
    else if(gcmSL_SOURCE_GET(Instruction->source0, Type) != gcSL_CONSTANT &&
            gcmSL_SOURCE_GET(Instruction->source0, Type) != gcSL_UNIFORM)
    {
        if(gcmSL_SOURCE_GET(Instruction->source0, Precision) == gcSL_PRECISION_MEDIUM)
        {
            States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 5:3) - (0 ?
 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3)));
        }
        else
        {
            States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 5:3) - (0 ?
 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3)));

        }
    }

    if(gcmSL_SOURCE_GET(Instruction->source1, Type) == gcSL_ATTRIBUTE && /* check for sources being gl_FragCoord */
       CodeGen->usePosition && Instruction->source1Index == CodeGen->positionIndex)
    {
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));
    }
    else if(gcmSL_SOURCE_GET(Instruction->source1, Type) != gcSL_CONSTANT &&
            gcmSL_SOURCE_GET(Instruction->source1, Type) != gcSL_UNIFORM)
    {
        if(gcmSL_SOURCE_GET(Instruction->source1, Precision) == gcSL_PRECISION_MEDIUM)
        {
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));
        }
        else
        {
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));
        }
    }

    /* Set to 0x1 */
    return destAddrT0(Tree, CodeGen, Instruction, States);
}

static gctBOOL
_t0_destMP(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    /* set dest type to highp */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)));

    if(gcmSL_SOURCE_GET(Instruction->source0, Type) == gcSL_ATTRIBUTE && /* check for sources being gl_FragCoord */
       CodeGen->usePosition && Instruction->source0Index == CodeGen->positionIndex)
    {
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 5:3) - (0 ?
 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3)));
    }
    else if(gcmSL_SOURCE_GET(Instruction->source0, Type) != gcSL_CONSTANT &&
            gcmSL_SOURCE_GET(Instruction->source0, Type) != gcSL_UNIFORM)
    {
        if(gcmSL_SOURCE_GET(Instruction->source0, Precision) == gcSL_PRECISION_MEDIUM)
        {
            States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 5:3) - (0 ?
 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3)));
        }
        else
        {
            States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 5:3) - (0 ?
 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3)));

        }
    }

    if(gcmSL_SOURCE_GET(Instruction->source1, Type) == gcSL_ATTRIBUTE && /* check for sources being gl_FragCoord */
       CodeGen->usePosition && Instruction->source1Index == CodeGen->positionIndex)
    {
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));
    }
    else if(gcmSL_SOURCE_GET(Instruction->source1, Type) != gcSL_CONSTANT &&
            gcmSL_SOURCE_GET(Instruction->source1, Type) != gcSL_UNIFORM)
    {
        if(gcmSL_SOURCE_GET(Instruction->source1, Precision) == gcSL_PRECISION_MEDIUM)
        {
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));
        }
        else
        {
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));
        }
    }

    /* Set to 0x1 */
    return destAddrT0(Tree, CodeGen, Instruction, States);
}

/* set dest address to thread T1 - Highp dest, Mediump/highp Src0, Mediump/highp Src1 */
static gctBOOL
_t1_destHP(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT addr;

    /* set dest type to highp */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)));

    if(gcmSL_SOURCE_GET(Instruction->source0, Type) == gcSL_ATTRIBUTE && /* check for sources being gl_FragCoord */
       CodeGen->usePosition && Instruction->source0Index == CodeGen->positionIndex)
    {
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 5:3) - (0 ?
 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3)));
        addr = (((((gctUINT32) (States[1])) >> (0 ? 20:12)) & ((gctUINT32) ((((1 ? 20:12) - (0 ? 20:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:12) - (0 ? 20:12) + 1)))))) );
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ? 20:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (addr + 1) & ((gctUINT32) ((((1 ?
 20:12) - (0 ? 20:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ?
 20:12)));
    }
    else if(gcmSL_SOURCE_GET(Instruction->source0, Type) != gcSL_CONSTANT &&
            gcmSL_SOURCE_GET(Instruction->source0, Type) != gcSL_UNIFORM)
    {
        if(gcmSL_SOURCE_GET(Instruction->source0, Precision) == gcSL_PRECISION_MEDIUM)
        {
            States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 5:3) - (0 ?
 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3)));
        }
        else
        {
            States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 5:3) - (0 ?
 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3)));
            addr = (((((gctUINT32) (States[1])) >> (0 ? 20:12)) & ((gctUINT32) ((((1 ? 20:12) - (0 ? 20:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:12) - (0 ? 20:12) + 1)))))) );
            States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ? 20:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (addr + 1) & ((gctUINT32) ((((1 ?
 20:12) - (0 ? 20:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ?
 20:12)));
        }
    }

    if(gcmSL_SOURCE_GET(Instruction->source1, Type) == gcSL_ATTRIBUTE && /* check for sources being gl_FragCoord */
       CodeGen->usePosition && Instruction->source1Index == CodeGen->positionIndex)
    {
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));
        addr = (((((gctUINT32) (States[2])) >> (0 ? 15:7)) & ((gctUINT32) ((((1 ? 15:7) - (0 ? 15:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:7) - (0 ? 15:7) + 1)))))) );
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:7) - (0 ? 15:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:7) - (0 ? 15:7) + 1))))))) << (0 ?
 15:7))) | (((gctUINT32) ((gctUINT32) (addr + 1) & ((gctUINT32) ((((1 ?
 15:7) - (0 ? 15:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:7) - (0 ? 15:7) + 1))))))) << (0 ?
 15:7)));
    }
    else if(gcmSL_SOURCE_GET(Instruction->source1, Type) != gcSL_CONSTANT &&
            gcmSL_SOURCE_GET(Instruction->source1, Type) != gcSL_UNIFORM)
    {
        if(gcmSL_SOURCE_GET(Instruction->source1, Precision) == gcSL_PRECISION_MEDIUM)
        {
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));
        }
        else
        {
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));
            addr = (((((gctUINT32) (States[2])) >> (0 ? 15:7)) & ((gctUINT32) ((((1 ? 15:7) - (0 ? 15:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:7) - (0 ? 15:7) + 1)))))) );
            States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:7) - (0 ? 15:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:7) - (0 ? 15:7) + 1))))))) << (0 ?
 15:7))) | (((gctUINT32) ((gctUINT32) (addr + 1) & ((gctUINT32) ((((1 ?
 15:7) - (0 ? 15:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:7) - (0 ? 15:7) + 1))))))) << (0 ?
 15:7)));
        }
    }
    /* Set to 0x2 */
    return destAddrT1(Tree, CodeGen, Instruction, States);
}

static gctBOOL
_t1_destMP(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT addr;

    /* set dest type to highp */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)));

    if(gcmSL_SOURCE_GET(Instruction->source0, Type) == gcSL_ATTRIBUTE && /* check for sources being gl_FragCoord */
       CodeGen->usePosition && Instruction->source0Index == CodeGen->positionIndex)
    {
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 5:3) - (0 ?
 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3)));
        addr = (((((gctUINT32) (States[1])) >> (0 ? 20:12)) & ((gctUINT32) ((((1 ? 20:12) - (0 ? 20:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:12) - (0 ? 20:12) + 1)))))) );
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ? 20:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (addr + 1) & ((gctUINT32) ((((1 ?
 20:12) - (0 ? 20:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ?
 20:12)));
    }
    else if(gcmSL_SOURCE_GET(Instruction->source0, Type) != gcSL_CONSTANT &&
            gcmSL_SOURCE_GET(Instruction->source0, Type) != gcSL_UNIFORM)
    {
        if(gcmSL_SOURCE_GET(Instruction->source0, Precision) == gcSL_PRECISION_MEDIUM)
        {
            States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 5:3) - (0 ?
 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3)));
        }
        else
        {
            States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 5:3) - (0 ?
 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3)));
            addr = (((((gctUINT32) (States[1])) >> (0 ? 20:12)) & ((gctUINT32) ((((1 ? 20:12) - (0 ? 20:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:12) - (0 ? 20:12) + 1)))))) );
            States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ? 20:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (addr + 1) & ((gctUINT32) ((((1 ?
 20:12) - (0 ? 20:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ?
 20:12)));
        }
    }

    if(gcmSL_SOURCE_GET(Instruction->source1, Type) == gcSL_ATTRIBUTE && /* check for sources being gl_FragCoord */
       CodeGen->usePosition && Instruction->source1Index == CodeGen->positionIndex)
    {
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));
        addr = (((((gctUINT32) (States[2])) >> (0 ? 15:7)) & ((gctUINT32) ((((1 ? 15:7) - (0 ? 15:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:7) - (0 ? 15:7) + 1)))))) );
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:7) - (0 ? 15:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:7) - (0 ? 15:7) + 1))))))) << (0 ?
 15:7))) | (((gctUINT32) ((gctUINT32) (addr + 1) & ((gctUINT32) ((((1 ?
 15:7) - (0 ? 15:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:7) - (0 ? 15:7) + 1))))))) << (0 ?
 15:7)));
    }
    else if(gcmSL_SOURCE_GET(Instruction->source1, Type) != gcSL_CONSTANT &&
            gcmSL_SOURCE_GET(Instruction->source1, Type) != gcSL_UNIFORM)
    {
        if(gcmSL_SOURCE_GET(Instruction->source1, Precision) == gcSL_PRECISION_MEDIUM)
        {
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));
        }
        else
        {
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));
            addr = (((((gctUINT32) (States[2])) >> (0 ? 15:7)) & ((gctUINT32) ((((1 ? 15:7) - (0 ? 15:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:7) - (0 ? 15:7) + 1)))))) );
            States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:7) - (0 ? 15:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:7) - (0 ? 15:7) + 1))))))) << (0 ?
 15:7))) | (((gctUINT32) ((gctUINT32) (addr + 1) & ((gctUINT32) ((((1 ?
 15:7) - (0 ? 15:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:7) - (0 ? 15:7) + 1))))))) << (0 ?
 15:7)));
        }
    }
    /* Set to 0x2 */
    return destAddrT1(Tree, CodeGen, Instruction, States);
}

/* set dest address to thread T0 - dest: Mediump, states Src2: Highp   */
static gctBOOL
_t0_src2HP(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    /* set dest type to mediump */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)));

    /* Set Source1 type to mediump */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 30:28) - (0 ?
 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ?
 30:28)));

    /* Set to 0x1 */
    return destAddrT0(Tree, CodeGen, Instruction, States);
}

/* set dest address to thread T1 - dest: Mediump, states Src2: Highp
 * reuse the same temp register as t0
 */
static gctBOOL
_t1_src2HP(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    /* set dest type to mediump */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)));

    /* Set Source1 type to mediump */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 30:28) - (0 ?
 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ?
 30:28)));

    /* Set to 0x2 */
    return destAddrT1(Tree, CodeGen, Instruction, States);
}

/* set dest address to thread T0 - dest: Mediump, states Src2: Highp   */
static gctBOOL
_t0_src2HP_dstHP(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    /* set dest type to mediump */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)));

    /* Set Source1 type to mediump */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 30:28) - (0 ?
 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ?
 30:28)));

    /* Set to 0x1 */
    return destAddrT0(Tree, CodeGen, Instruction, States);
}

/* set dest address to thread T1 - dest: Mediump, states Src2: Highp
 * reuse the same temp register as t0
 */
static gctBOOL
_t1_src2HP_dstHP(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    /* set dest type to mediump */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)));

    /* Set Source1 type to mediump */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 30:28) - (0 ?
 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ?
 30:28)));

    /* Set to 0x2 */
    return destAddrT1(Tree, CodeGen, Instruction, States);
}

/* swizzle s0.x and s2.y with thread t0 */
static gctBOOL
_t0_src0HP_dstMP(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    /* set dest type to highp */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)));

    /* Set Src2 type to highp */
    States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 5:3) - (0 ?
 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3)));

    return destAddrT0(Tree, CodeGen, Instruction, States);
}

static gctBOOL
_t1_src0HP_dstMP(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    /* set dest type to highp */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)));

    /* Set Src2 type to highp */
    States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 5:3) - (0 ?
 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3)));

    return destAddrT1(Tree, CodeGen, Instruction, States);
}

/* swizzle s0.x and s2.y with thread t0 */
static gctBOOL
_t0_src1HP_dstMP(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    /* set dest type to highp */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)));

    /* Set Src2 type to highp */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));

    return destAddrT0(Tree, CodeGen, Instruction, States);
}

static gctBOOL
_t1_src1HP_dstMP(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    /* set dest type to highp */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)));

    /* Set Src2 type to highp */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));

    return destAddrT1(Tree, CodeGen, Instruction, States);
}

/* swizzle s0.x and s2.y with thread t0 */
static gctBOOL
swizzleS0xS2yWithT0HpHpHp(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (0x00) & ((gctUINT32) ((((1 ? 29:22) - (0 ?
 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22)));
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (0x55) & ((gctUINT32) ((((1 ? 21:14) - (0 ?
 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));

    /* set dest type to highp */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)));

    /* Set Src0 type to highp */
    States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 5:3) - (0 ?
 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3)));

    /* Set Src2 type to highp */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 30:28) - (0 ?
 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ?
 30:28)));
    return destAddrT0(Tree, CodeGen, Instruction, States);
}

/* swizzle s0.x and s2.y with thread t1 */
static gctBOOL
swizzleS0xS2yWithT1HpHpHp(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (0x00) & ((gctUINT32) ((((1 ? 29:22) - (0 ?
 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22)));
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (0x55) & ((gctUINT32) ((((1 ? 21:14) - (0 ?
 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));

    /* set dest type to highp */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)));

    /* Set Src0 type to highp */
    States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 5:3) - (0 ?
 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3)));

    /* Set Src2 type to highp */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 30:28) - (0 ?
 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ?
 30:28)));
    return destAddrT1(Tree, CodeGen, Instruction, States);
}
#endif /* GC_ENABLE_DUAL_FP16 */

static gctBOOL
_HasPreNormInst(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (CodeGen->hasDual16) {
        /* pre normal instructions is a sub-set of dual 16 feature */
        return gcvTRUE;
    }
    return gcvFALSE;
}

#if (GC_ENABLE_DUAL_FP16 > 0) && _USE_HARDWARE_NORMALIZE_MACRO_OPCODES
static gctBOOL
_OnlyXYOrXEnabledHasDual16(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (CodeGen->hasDual16) {
        gcSL_ENABLE  enable = gcmSL_TARGET_GET(Instruction->temp, Enable);
        return enable == (gcSL_ENABLE_X | gcSL_ENABLE_Y) ||
               enable == gcSL_ENABLE_X;
    }
    return gcvFALSE;
}

static gctBOOL
_OnlyXYZEnabledHasDual16(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (CodeGen->hasDual16) {
        gcSL_ENABLE  enable = gcmSL_TARGET_GET(Instruction->temp, Enable);
        return enable == (gcSL_ENABLE_X | gcSL_ENABLE_Y | gcSL_ENABLE_Z);
    }
    return gcvFALSE;
}

static gctBOOL
_XYZWEnabledHasDual16(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (CodeGen->hasDual16) {
        gcSL_ENABLE  enable = gcmSL_TARGET_GET(Instruction->temp, Enable);
        return enable == (gcSL_ENABLE_X | gcSL_ENABLE_Y | gcSL_ENABLE_Z | gcSL_ENABLE_W);
    }
    return gcvFALSE;
}

/* set Mediump dest, Mediump Src0, Mediump Src1 */
static gctBOOL
destMpSrc0MpSrc1Mp(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if(CodeGen->isDual16Shader) {
        /* set dest type to mediump */
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)));

        if (gcmSL_SOURCE_GET(Instruction->source0, Type) != gcSL_CONSTANT &&
            gcmSL_SOURCE_GET(Instruction->source0, Type) != gcSL_UNIFORM) {
                /* Set Source0, Source1 type to mediump */
            States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 5:3) - (0 ?
 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3)));
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));
        }
    }
    return gcvTRUE;
}
#else
#if (GC_ENABLE_DUAL_FP16 > 0)
static gctBOOL
_OnlyXYOrXEnabledMediumpSrc0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (CodeGen->isDual16Shader &&
        gcmSL_SOURCE_GET(Instruction->source0, Precision) == gcSL_PRECISION_MEDIUM) {
        gcSL_ENABLE  enable = gcmSL_TARGET_GET(Instruction->temp, Enable);
        return enable == (gcSL_ENABLE_X | gcSL_ENABLE_Y) ||
               enable == gcSL_ENABLE_X;
    }
    return gcvFALSE;
}

static gctBOOL
_XYZEnabledMediumpSrc0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (CodeGen->isDual16Shader &&
        gcmSL_SOURCE_GET(Instruction->source0, Precision) == gcSL_PRECISION_MEDIUM) {
        gcSL_ENABLE  enable = gcmSL_TARGET_GET(Instruction->temp, Enable);
        return enable == (gcSL_ENABLE_X | gcSL_ENABLE_Y | gcSL_ENABLE_Z);
    }
    return gcvFALSE;
}

static gctBOOL
_XYZWEnabledMediumpSrc0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (CodeGen->isDual16Shader &&
        gcmSL_SOURCE_GET(Instruction->source0, Precision) == gcSL_PRECISION_MEDIUM) {
        gcSL_ENABLE  enable = gcmSL_TARGET_GET(Instruction->temp, Enable);
        return enable == (gcSL_ENABLE_X | gcSL_ENABLE_Y | gcSL_ENABLE_Z | gcSL_ENABLE_W);
    }
    return gcvFALSE;
}

/* set dest address to thread T0 - Highp dest, Mediump Src0, Mediump Src1 */
static gctBOOL
destAddrT0HpMpMp(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    /* set dest type to highp */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)));

    if (gcmSL_SOURCE_GET(Instruction->source0, Type) == gcSL_TEMP) {
        /* Set Source0 type to mediump */
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 5:3) - (0 ?
 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3)));
    }

    if (gcmSL_SOURCE_GET(Instruction->source1, Type) == gcSL_TEMP) {
        /* Set Source1 type to mediump */
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));
    }

    /* Set to 0x1 */
    return destAddrT0(Tree, CodeGen, Instruction, States);
}

/* set dest address to thread T1 - Highp dest, Mediump Src0, Mediump Src1 */
static gctBOOL
destAddrT1HpMpMp(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT address;

    /* set dest type to highp */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)));

    if (gcmSL_SOURCE_GET(Instruction->source0, Type) == gcSL_TEMP) {
        /* set r + 1, for T1 */
        address = (((((gctUINT32) (States[1])) >> (0 ? 20:12)) & ((gctUINT32) ((((1 ? 20:12) - (0 ? 20:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:12) - (0 ? 20:12) + 1)))))) ) + 1;
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ? 20:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ?
 20:12) - (0 ? 20:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ?
 20:12)));

        /* Set Source0 type to mediump */
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 5:3) - (0 ?
 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3)));
    }

    if (gcmSL_SOURCE_GET(Instruction->source1, Type) == gcSL_TEMP) {
        /* set r + 1, for T1 */
        address = (((((gctUINT32) (States[2])) >> (0 ? 15:7)) & ((gctUINT32) ((((1 ? 15:7) - (0 ? 15:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:7) - (0 ? 15:7) + 1)))))) ) + 1;
        States[1] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:7) - (0 ? 15:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:7) - (0 ? 15:7) + 1))))))) << (0 ?
 15:7))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ? 15:7) - (0 ?
 15:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:7) - (0 ? 15:7) + 1))))))) << (0 ?
 15:7)));

        /* Set Source1 type to mediump */
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));
    }

    /* Set to 0x2 */
    return destAddrT1(Tree, CodeGen, Instruction, States);
}

/* set dest address to thread T0 - Mediump dest, Highp Src0, Mediump Src1 */
static gctBOOL
destAddrT0MpHpMp(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    /* set dest type to mediump */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)));

    if (gcmSL_SOURCE_GET(Instruction->source0, Type) == gcSL_TEMP) {
        /* Set Source0 type to highp */
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 5:3) - (0 ?
 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3)));
    }

    if (gcmSL_SOURCE_GET(Instruction->source1, Type) == gcSL_TEMP) {
        /* Set Source1 type to mediump */
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));
    }

    /* Set to 0x1 */
    return destAddrT0(Tree, CodeGen, Instruction, States);
}

/* set dest address to thread T1 - Mediump dest, Highp Src0, Mediump Src1 */
static gctBOOL
destAddrT1MpHpMp(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT address;

    /* set r + 1, for T1 */
    address = (((((gctUINT32) (States[0])) >> (0 ? 22:16)) & ((gctUINT32) ((((1 ? 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1)))))) ) + 1;
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ?
 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ?
 22:16)));

    /* set dest type to mediump */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)));

    if (gcmSL_SOURCE_GET(Instruction->source0, Type) == gcSL_TEMP) {

        /* Set Source0 type to highp */
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 5:3) - (0 ?
 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3)));
    }

    if (gcmSL_SOURCE_GET(Instruction->source1, Type) == gcSL_TEMP) {
        /* set r + 1, for T1 */
        address = (((((gctUINT32) (States[2])) >> (0 ? 15:7)) & ((gctUINT32) ((((1 ? 15:7) - (0 ? 15:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:7) - (0 ? 15:7) + 1)))))) ) + 1;
        States[1] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:7) - (0 ? 15:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:7) - (0 ? 15:7) + 1))))))) << (0 ?
 15:7))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ? 15:7) - (0 ?
 15:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:7) - (0 ? 15:7) + 1))))))) << (0 ?
 15:7)));

        /* Set Source1 type to mediump */
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));
    }

    /* Set to 0x2 */
    return destAddrT1(Tree, CodeGen, Instruction, States);
}

/* set dest address to thread T0 - Highp dest, Highp Src0 */
static gctBOOL
destAddrT0HpHp(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    /* set dest type to highp */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)));

    if (gcmSL_SOURCE_GET(Instruction->source0, Type) == gcSL_TEMP) {
        /* Set Source0 type to highp */
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 5:3) - (0 ?
 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3)));
    }

    /* Set to 0x1 */
    return destAddrT0(Tree, CodeGen, Instruction, States);
}

/* set dest address to thread T1 - Highp dest, Highp Src0 */
static gctBOOL
destAddrT1HpHp(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    /* set dest type to highp */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)));

    if (gcmSL_SOURCE_GET(Instruction->source0, Type) == gcSL_TEMP) {
        /* Set Source0 type to highp */
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 5:3) - (0 ?
 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3)));
    }

    /* Set to 0x2 */
    return destAddrT1(Tree, CodeGen, Instruction, States);
}

static gctBOOL
swizzle2XWithT0HpHp(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle((((((gctUINT32) (States[3])) >> (0 ?
 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 21:14) - (0 ? 21:14) + 1)))))) ), 0)) & ((gctUINT32) ((((1 ? 21:14) - (0 ?
 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));
    /* set dest type to highp */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)));

    if (gcmSL_SOURCE_GET(Instruction->source0, Type) == gcSL_TEMP) {
        /* Set Source0 type to highp */
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 5:3) - (0 ?
 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3)));
    }

    return destAddrT0(Tree, CodeGen, Instruction, States);
}

static gctBOOL
swizzle2XWithT1HpHp(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle((((((gctUINT32) (States[3])) >> (0 ?
 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 21:14) - (0 ? 21:14) + 1)))))) ), 0)) & ((gctUINT32) ((((1 ? 21:14) - (0 ?
 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));
    /* set dest type to highp */
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)));

    if (gcmSL_SOURCE_GET(Instruction->source0, Type) == gcSL_TEMP) {
        /* Set Source0 type to highp */
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 5:3) - (0 ?
 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3)));
    }

    return destAddrT1(Tree, CodeGen, Instruction, States);
}
#endif
#endif

/*
 * For the normalize macro, the normalize of a vector(0.0) is handled
 * not by the RSQ, but by the subsequent norm_mul.  It needs to have the
 * "mul0zero" bit in the instruction set.  This will make 0*INF=0, thus
 * handling the special case properly.  This is instruction_sampler_num[0],
 * so this bit should be set to 1 in the instruction.  The norm_mul
 * instruction should always assemble with that bit set.
 */
static gctBOOL
set_norm_mul0zero(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT sampler_num = (((((gctUINT32) (States[0])) >> (0 ? 31:27)) & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1)))))) );

    /* set bit 0 to 1 */
    sampler_num |= 0x01;
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) ((gctUINT32) (sampler_num) & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27)));

    if (Instruction->opcode == gcSL_NORM)
    {
        /* NORM_MUL may be used for dual16, so try to do it if we enable the dual16 */
        return destMpSrc0MpSrc1Mp(Tree, CodeGen, Instruction, States);
    }

    return gcvTRUE;
}

static gctBOOL
swizzle2X(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle((((((gctUINT32) (States[3])) >> (0 ?
 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 21:14) - (0 ? 21:14) + 1)))))) ), 0)) & ((gctUINT32) ((((1 ? 21:14) - (0 ?
 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));

    return gcvTRUE;
}

/* swizzle s0.x and s2.y */
static gctBOOL
swizzleS0xS2y(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (0x00) & ((gctUINT32) ((((1 ? 29:22) - (0 ?
 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22)));
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (0x55) & ((gctUINT32) ((((1 ? 21:14) - (0 ?
 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));

    return gcvTRUE;
}

static gctBOOL
getSwizzleForShadowTexture(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_TYPE type = gcmSL_SOURCE_GET(Instruction->source0, Type);
    gcUNIFORM uniform = gcvNULL;
    gctUINT16 index = gcmSL_INDEX_GET(Instruction->source0Index, Index);
    gctUINT8 sourceSwizzle;
    gctUINT8 newSourceSwizzle;

    if (type == gcSL_UNIFORM)
    {
        uniform = Tree->shader->uniforms[index];
    }
    else
    {
        gctUINT32 i;

        gcmASSERT(type == gcSL_SAMPLER);
        for (i = 0; i < Tree->shader->uniformCount; i++)
        {
            uniform = Tree->shader->uniforms[i];

            if (!isSamplerType(uniform->u.type))
                continue;

            if (index >= uniform->physical &&
                index <= (uniform->physical + uniform->arraySize - 1))
            {
                break;
            }
        }

        gcmASSERT(i < Tree->shader->uniformCount);
    }

    switch (uniform->u.type)
    {
    /* Use two components as coord. */
    case gcSHADER_SAMPLER_2D_SHADOW:
    case gcSHADER_SAMPLER_1D_ARRAY_SHADOW:
        sourceSwizzle  = (((((gctUINT32) (States[1])) >> (0 ? 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1)))))) );

        newSourceSwizzle = gcmComposeSwizzle(gcmExtractSwizzle(sourceSwizzle, 0),
                                             gcmExtractSwizzle(sourceSwizzle, 1),
                                             gcmExtractSwizzle(sourceSwizzle, 1),
                                             gcmExtractSwizzle(sourceSwizzle, 1));

        /* adjust swizzle to use .w */
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (newSourceSwizzle) & ((gctUINT32) ((((1 ?
 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22)));
        break;

    /* Use three components as coord. */
    case gcSHADER_SAMPLER_CUBE_SHADOW:
    case gcSHADER_SAMPLER_2D_ARRAY_SHADOW:
        sourceSwizzle  = (((((gctUINT32) (States[1])) >> (0 ? 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1)))))) );

        newSourceSwizzle = gcmComposeSwizzle(gcmExtractSwizzle(sourceSwizzle, 0),
                                             gcmExtractSwizzle(sourceSwizzle, 1),
                                             gcmExtractSwizzle(sourceSwizzle, 2),
                                             gcmExtractSwizzle(sourceSwizzle, 2));

        /* adjust swizzle to use .w */
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (newSourceSwizzle) & ((gctUINT32) ((((1 ?
 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22)));
        break;
    /*
    ** When compare is present, it is used as D_ref and the
    ** array layer comes from the last component of P. When compare is not
    ** present, the last component of P is used as D_ref and the array layer
    ** comes from the second to last component of P.
    ** HW always uses the last component of P as array layer, so we need to change the swizzle.
    */
    case gcSHADER_SAMPLER_CUBEMAP_ARRAY_SHADOW:
        {
            if (gcmSL_OPCODE_GET(Instruction->opcode, Opcode) != gcSL_TEXBIAS)
            {
                sourceSwizzle  = (((((gctUINT32) (States[1])) >> (0 ? 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1)))))) );

                newSourceSwizzle = gcmComposeSwizzle(gcmExtractSwizzle(sourceSwizzle, 0),
                                                     gcmExtractSwizzle(sourceSwizzle, 1),
                                                     gcmExtractSwizzle(sourceSwizzle, 2),
                                                     gcmExtractSwizzle(sourceSwizzle, 2));

                /* adjust swizzle to use .w */
                States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (newSourceSwizzle) & ((gctUINT32) ((((1 ?
 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22)));
            }
            break;
        }

    default:
        break;
    }

    return gcvTRUE;
}

static gctUINT32
getSource1Usage(
    IN gcLINKTREE Tree,
    IN gcSL_INSTRUCTION Instruction
    )
{
    gctSOURCE_t source = Instruction->source1;
    gctUINT16 index  = Instruction->source1Index;

    gctUINT32 usage = 0;
    if (gcmSL_SOURCE_GET(source, Type) == gcSL_TEMP)
    {
        usage = Tree->tempArray[gcmSL_INDEX_GET(index, Index)].usage;
    }
    else if (gcmSL_SOURCE_GET(source, Type) == gcSL_ATTRIBUTE)
    {
        switch (Tree->shader->attributes[gcmSL_INDEX_GET(index, Index)]->type)
        {
        case gcSHADER_FLOAT_X1:
            usage = 0x1;
            break;

        case gcSHADER_FLOAT_X2:
            usage = 0x3;
            break;

        case gcSHADER_FLOAT_X3:
            usage = 0x7;
            break;

        case gcSHADER_FLOAT_X4:
            usage = 0xF;
            break;

        default:
            break;
        }
    }
    else if (gcmSL_SOURCE_GET(source, Type) == gcSL_UNIFORM)
    {
        switch (Tree->shader->uniforms[gcmSL_INDEX_GET(index, Index)]->u.type)
        {
        case gcSHADER_FLOAT_X1:
            usage = 0x1;
            break;

        case gcSHADER_FLOAT_X2:
            usage = 0x3;
            break;

        case gcSHADER_FLOAT_X3:
            usage = 0x7;
            break;

        case gcSHADER_FLOAT_X4:
            usage = 0xF;
            break;

        default:
            break;
        }
    }

    return usage;
}

static gctBOOL
swizzle2ZorW(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT32 usage = getSource1Usage(Tree, Instruction);

    if (usage & 0x8)
    {
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle((((((gctUINT32) (States[3])) >> (0 ?
 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 21:14) - (0 ? 21:14) + 1)))))) ), 3)) & ((gctUINT32) ((((1 ? 21:14) - (0 ?
 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));
    }
    else if (usage & 0x4)
    {
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle((((((gctUINT32) (States[3])) >> (0 ?
 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 21:14) - (0 ? 21:14) + 1)))))) ), 2)) & ((gctUINT32) ((((1 ? 21:14) - (0 ?
 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    return gcvTRUE;
}

#if !_CLAMP_PCF_REFERENCE_
static gctBOOL
swizzle2ZorW_sample_swizzleX(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT32 usage = getSource1Usage(Tree, Instruction);

    if (usage == 0x7)
    {
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle((((((gctUINT32) (States[3])) >> (0 ?
 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 21:14) - (0 ? 21:14) + 1)))))) ), 2)) & ((gctUINT32) ((((1 ? 21:14) - (0 ?
 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));
    }
    else if (usage == 0xF)
    {
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle((((((gctUINT32) (States[3])) >> (0 ?
 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 21:14) - (0 ? 21:14) + 1)))))) ), 3)) & ((gctUINT32) ((((1 ? 21:14) - (0 ?
 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));
    }

    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3))) | (((gctUINT32) ((gctUINT32) (gcSL_SWIZZLE_XXXX) & ((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3)));

    return gcvTRUE;
}

static gctBOOL
swizzle2ZorW_sample_swizzleX_fix_shadow_coord_swizzle(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    return swizzle2ZorW_sample_swizzleX(Tree, CodeGen, Instruction, States) &&
        getSwizzleForShadowTexture(Tree, CodeGen, Instruction, States);
}

static gctBOOL
zero_1_swizzle2ZorW_sample_swizzleX(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT32 usage = getSource1Usage(Tree, Instruction);
    gctINT index = 0;
    gctUINT8 swizzle = 0;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  0.0f,
                                  &index,
                                  &swizzle));

    _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);

    gcmASSERT(usage != 0);

    if (usage == 0x7)
    {
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle((((((gctUINT32) (States[3])) >> (0 ?
 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 21:14) - (0 ? 21:14) + 1)))))) ), 2)) & ((gctUINT32) ((((1 ? 21:14) - (0 ?
 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));
    }
    else if (usage == 0xF)
    {
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle((((((gctUINT32) (States[3])) >> (0 ?
 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 21:14) - (0 ? 21:14) + 1)))))) ), 3)) & ((gctUINT32) ((((1 ? 21:14) - (0 ?
 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));
    }

    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3))) | (((gctUINT32) ((gctUINT32) (gcSL_SWIZZLE_XXXX) & ((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3)));

    return gcvTRUE;
}

static gctBOOL
zero_1_swizzle2ZorW_sample_swizzleX_fix_shadow_coord_swizzle(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    return zero_1_swizzle2ZorW_sample_swizzleX(Tree, CodeGen, Instruction, States) &&
        getSwizzleForShadowTexture(Tree, CodeGen, Instruction, States);
}
#endif

static gctBOOL
swizzle2ZorW_from_next_inst(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_INSTRUCTION nextInstruction = Instruction + 1;
    gctUINT32 usage = getSource1Usage(Tree, nextInstruction);

    if (usage & 0x8)
    {
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle((((((gctUINT32) (States[3])) >> (0 ?
 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 21:14) - (0 ? 21:14) + 1)))))) ), 3)) & ((gctUINT32) ((((1 ? 21:14) - (0 ?
 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));
    }
    else if (usage & 0x4)
    {
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle((((((gctUINT32) (States[3])) >> (0 ?
 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 21:14) - (0 ? 21:14) + 1)))))) ), 2)) & ((gctUINT32) ((((1 ? 21:14) - (0 ?
 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    return gcvTRUE;
}

static gctBOOL
swizzle2Z_sample_swizzleX(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle((((((gctUINT32) (States[3])) >> (0 ?
 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 21:14) - (0 ? 21:14) + 1)))))) ), 2)) & ((gctUINT32) ((((1 ? 21:14) - (0 ?
 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));

    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3))) | (((gctUINT32) ((gctUINT32) (gcSL_SWIZZLE_XXXX) & ((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3)));

    return gcvTRUE;
}

static gctBOOL
swizzle0XY_sample_swizzleX(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_SWIZZLE swizzle = (((((gctUINT32) (States[1])) >> (0 ? 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1)))))) );

    if (swizzle == gcSL_SWIZZLE_X)
    {
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (gcSL_SWIZZLE_XYYY) & ((gctUINT32) ((((1 ?
 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22)));
    }
    else
    {
        gcSL_SWIZZLE xSwizzle, ySwizzle, newSwizzle;

        xSwizzle = swizzle & 0x3;
        ySwizzle = (swizzle >> 2) & 0x3;

        newSwizzle = ((xSwizzle << 0) |
                      (ySwizzle << 2) |
                      (ySwizzle << 4) |
                      (ySwizzle << 6));

        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (newSwizzle) & ((gctUINT32) ((((1 ?
 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22)));
    }

    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3))) | (((gctUINT32) ((gctUINT32) (gcSL_SWIZZLE_XXXX) & ((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3)));

    return gcvTRUE;
}

static gctBOOL
zero_1_swizzle2Z_sample_swizzleX(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  0.0f,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);

    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle((((((gctUINT32) (States[3])) >> (0 ?
 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 21:14) - (0 ? 21:14) + 1)))))) ), 2)) & ((gctUINT32) ((((1 ? 21:14) - (0 ?
 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));

    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3))) | (((gctUINT32) ((gctUINT32) (gcSL_SWIZZLE_XXXX) & ((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3)));

    return gcvTRUE;
}

static gctBOOL
enable_w_swizzle2W(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (gcSL_ENABLE_W) & ((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));

    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle((((((gctUINT32) (States[3])) >> (0 ?
 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 21:14) - (0 ? 21:14) + 1)))))) ), 3)) & ((gctUINT32) ((((1 ? 21:14) - (0 ?
 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));

    return gcvTRUE;
}

static gctBOOL
swizzle1W(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:17) - (0 ? 24:17) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:17) - (0 ? 24:17) + 1))))))) << (0 ?
 24:17))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle((((((gctUINT32) (States[2])) >> (0 ?
 24:17)) & ((gctUINT32) ((((1 ? 24:17) - (0 ? 24:17) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 24:17) - (0 ? 24:17) + 1)))))) ), 3)) & ((gctUINT32) ((((1 ? 24:17) - (0 ?
 24:17) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:17) - (0 ? 24:17) + 1))))))) << (0 ?
 24:17)));

    return gcvTRUE;
}

static gctBOOL
conditionLE_swizzle0Z(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_SWIZZLE swizzle = (((((gctUINT32) (States[1])) >> (0 ? 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1)))))) );

    if (swizzle == 0)
    {
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (gcSL_SWIZZLE_ZZZZ) & ((gctUINT32) ((((1 ?
 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22)));
    }
    else
    {
        gcSL_SWIZZLE zSwizzle, newSwizzle;

        zSwizzle = (swizzle >> 4) & 0x3;

        newSwizzle = ((zSwizzle << 0) |
                      (zSwizzle << 2) |
                      (zSwizzle << 4) |
                      (zSwizzle << 6));

        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (newSwizzle) & ((gctUINT32) ((((1 ?
 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22)));
    }

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x04 & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));

    return gcvTRUE;
}

static gctBOOL
sample_swizzleX(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3))) | (((gctUINT32) ((gctUINT32) (gcSL_SWIZZLE_XXXX) & ((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3)));

    return gcvTRUE;
}

static gctBOOL
sample_swizzleX_fix_shadow_coord_swizzle(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    return sample_swizzleX(Tree, CodeGen, Instruction, States) &&
        getSwizzleForShadowTexture(Tree, CodeGen, Instruction, States);
}


static gctBOOL
zero_1_sample_swizzleX(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  0.0f,
                                  &index,
                                  &swizzle,
                                  &constType));

    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3))) | (((gctUINT32) ((gctUINT32) (gcSL_SWIZZLE_XXXX) & ((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3)));

    _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);

    return gcvTRUE;
}

static gctBOOL
zero_1_sample_swizzleX_fix_shadow_coord_swizzle(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    return zero_1_sample_swizzleX(Tree, CodeGen, Instruction, States) &&
        getSwizzleForShadowTexture(Tree, CodeGen, Instruction, States);
}

#if _CLAMP_PCF_REFERENCE_
static gctBOOL
enable_z_saturate_swizzle2Z(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (gcSL_ENABLE_Z) & ((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ? 11:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));

    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle((((((gctUINT32) (States[3])) >> (0 ?
 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 21:14) - (0 ? 21:14) + 1)))))) ), 2)) & ((gctUINT32) ((((1 ? 21:14) - (0 ?
 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));

    return gcvTRUE;
}

static gctBOOL
enable_w_saturate_swizzle2Z(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (gcSL_ENABLE_W) & ((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ? 11:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));

    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle((((((gctUINT32) (States[3])) >> (0 ?
 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 21:14) - (0 ? 21:14) + 1)))))) ), 2)) & ((gctUINT32) ((((1 ? 21:14) - (0 ?
 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));

    return gcvTRUE;
}

static gctBOOL
saturate_swizzle2ZorW(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT32 usage = getSource1Usage(Tree, Instruction);

    if (usage == 0x7)
    {
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle((((((gctUINT32) (States[3])) >> (0 ?
 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 21:14) - (0 ? 21:14) + 1)))))) ), 2)) & ((gctUINT32) ((((1 ? 21:14) - (0 ?
 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));
    }
    else if (usage == 0xF)
    {
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle((((((gctUINT32) (States[3])) >> (0 ?
 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 21:14) - (0 ? 21:14) + 1)))))) ), 3)) & ((gctUINT32) ((((1 ? 21:14) - (0 ?
 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));
    }

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ? 11:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));

    return gcvTRUE;
}

static gctBOOL
saturate_swizzle2ZorW_from_next_inst(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_INSTRUCTION nextInstruction = Instruction + 1;
    gctUINT32 usage = getSource1Usage(Tree, nextInstruction);

    if (usage & 0x8)
    {
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle((((((gctUINT32) (States[3])) >> (0 ?
 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 21:14) - (0 ? 21:14) + 1)))))) ), 3)) & ((gctUINT32) ((((1 ? 21:14) - (0 ?
 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));
    }
    else if (usage & 0x4)
    {
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle((((((gctUINT32) (States[3])) >> (0 ?
 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 21:14) - (0 ? 21:14) + 1)))))) ), 2)) & ((gctUINT32) ((((1 ? 21:14) - (0 ?
 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ? 11:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));

    return gcvTRUE;
}

static gctBOOL
enable_w_saturate_swizzle2ZorW_from_next_inst(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_INSTRUCTION nextInstruction = Instruction + 1;
    gctUINT32 usage = getSource1Usage(Tree, nextInstruction);

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (gcSL_ENABLE_W) & ((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));

    if (usage & 0x8)
    {
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle((((((gctUINT32) (States[3])) >> (0 ?
 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 21:14) - (0 ? 21:14) + 1)))))) ), 3)) & ((gctUINT32) ((((1 ? 21:14) - (0 ?
 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));
    }
    else if (usage & 0x4)
    {
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle((((((gctUINT32) (States[3])) >> (0 ?
 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 21:14) - (0 ? 21:14) + 1)))))) ), 2)) & ((gctUINT32) ((((1 ? 21:14) - (0 ?
 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ? 11:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));

    return gcvTRUE;
}

static gctBOOL
conditionLE(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x04 & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));
    if (_shaderNeedIntSupport(Tree, CodeGen, Instruction, States))
    {
        value_type0_32bit_from_src0(Tree, CodeGen, Instruction, States);
    }
    return gcvTRUE;
}
#else
static gctBOOL
conditionLE_swizzle0ZorW(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT32 usage = getSource1Usage(Tree, Instruction);

    if (usage == 0x7)
    {
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle((((((gctUINT32) (States[1])) >> (0 ?
 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 29:22) - (0 ? 29:22) + 1)))))) ), 2)) & ((gctUINT32) ((((1 ? 29:22) - (0 ?
 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22)));
    }
    else if (usage == 0xF)
    {
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle((((((gctUINT32) (States[1])) >> (0 ?
 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 29:22) - (0 ? 29:22) + 1)))))) ), 3)) & ((gctUINT32) ((((1 ? 29:22) - (0 ?
 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22)));
    }

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x04 & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));

    return gcvTRUE;
}

static gctBOOL
enable_w_swizzle2Z(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (gcSL_ENABLE_W) & ((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));

    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle((((((gctUINT32) (States[3])) >> (0 ?
 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 21:14) - (0 ? 21:14) + 1)))))) ), 2)) & ((gctUINT32) ((((1 ? 21:14) - (0 ?
 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));

    return gcvTRUE;
}
#endif

static gctBOOL
conditionGZ(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));
    if (_shaderNeedIntSupport(Tree, CodeGen, Instruction, States))
    {
        value_type0(Tree, CodeGen, Instruction, States);
    }
    return gcvTRUE;
}

static gctBOOL
conditionGT(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));
    if (_shaderNeedIntSupport(Tree, CodeGen, Instruction, States))
    {
        value_type0_32bit_from_src0(Tree, CodeGen, Instruction, States);
    }
    return gcvTRUE;
}

static gctBOOL
conditionGTAbs1(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));
    gcSetSrcABS(States, 1);
    if (_shaderNeedIntSupport(Tree, CodeGen, Instruction, States))
    {
        value_type0_32bit_from_src0(Tree, CodeGen, Instruction, States);
    }
    return gcvTRUE;
}

static gctBOOL
conditionUCARRY(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x17 & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));
    if (_shaderNeedIntSupport(Tree, CodeGen, Instruction, States))
    {
        value_type0(Tree, CodeGen, Instruction, States);
    }
    return gcvTRUE;
}

static gctBOOL
crossSwizzle(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT32 swizzle0 = (((((gctUINT32) (States[1])) >> (0 ? 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1)))))) );
    gctUINT32 swizzle1 = (((((gctUINT32) (States[2])) >> (0 ? 24:17)) & ((gctUINT32) ((((1 ? 24:17) - (0 ? 24:17) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:17) - (0 ? 24:17) + 1)))))) );

    swizzle0 = (((swizzle0 >> 4) & 3) << 0)
             | (((swizzle0 >> 0) & 3) << 2)
             | (((swizzle0 >> 2) & 3) << 4)
             | (((swizzle0 >> 2) & 3) << 6);
    swizzle1 = (((swizzle1 >> 2) & 3) << 0)
             | (((swizzle1 >> 4) & 3) << 2)
             | (((swizzle1 >> 0) & 3) << 4)
             | (((swizzle1 >> 0) & 3) << 6);

    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (swizzle0) & ((gctUINT32) ((((1 ?
 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22)));
    States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:17) - (0 ? 24:17) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:17) - (0 ? 24:17) + 1))))))) << (0 ?
 24:17))) | (((gctUINT32) ((gctUINT32) (swizzle1) & ((gctUINT32) ((((1 ?
 24:17) - (0 ? 24:17) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:17) - (0 ? 24:17) + 1))))))) << (0 ?
 24:17)));

    return gcvTRUE;
}

static gctBOOL
copyCondition(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) ((gctUINT32) (_sl2gcCondition((gcSL_CONDITION) gcmSL_TARGET_GET(Instruction->temp, Condition))) & ((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6)));

    if (_shaderNeedIntSupport(Tree, CodeGen, Instruction, States))
    {
        /* set the inst type base on instruction source 0*/
        value_type0_32bit_from_src0(Tree, CodeGen, Instruction, States);
    }

    return gcvTRUE;
}

static gctBOOL
copyConditionAndDeleteSecondCaller(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_INSTRUCTION code = Instruction + 2;
    gctINT codeIndex = (gctINT)(code - Tree->shader->code);

    copyCondition(Tree, CodeGen, Instruction, States);

    deleteCaller(Tree, code->tempIndex, codeIndex);

    return gcvTRUE;
}

static gctBOOL
reverseCondition(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) ((gctUINT32) (_Reverse_sl2gcCondition((gcSL_CONDITION) gcmSL_TARGET_GET(Instruction->temp, Condition))) & ((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6)));

    if (_shaderNeedIntSupport(Tree, CodeGen, Instruction, States))
    {
        /* set the inst type base on instruction source 0*/
        value_type0_32bit_from_src0(Tree, CodeGen, Instruction, States);
    }
    return gcvTRUE;
}

static gctBOOL
reverseConditionAndDeleteFirstCaller(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT codeIndex = (gctINT)(Instruction - Tree->shader->code);

    reverseCondition(Tree, CodeGen, Instruction, States);

    deleteCaller(Tree, Instruction->tempIndex, codeIndex);

    return gcvTRUE;
}

static gctBOOL
reverseConditionAndDeleteSecondCaller(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_INSTRUCTION code = Instruction + 2;
    gctINT codeIndex = (gctINT)(code - Tree->shader->code);

    reverseCondition(Tree, CodeGen, Instruction, States);

    deleteCaller(Tree, code->tempIndex, codeIndex);

    return gcvTRUE;
}

static gctBOOL
reverseConditionAndReverseEqual(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT32 condition = _Reverse_sl2gcCondition((gcSL_CONDITION)
                                                     gcmSL_TARGET_GET(Instruction->temp,
                                                                      Condition));

    condition = _reverseEqualCondition(condition);

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) ((gctUINT32) (condition) & ((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6)));

    if (_shaderNeedIntSupport(Tree, CodeGen, Instruction, States))
    {
        /* set the inst type base on instruction source 0*/
        value_type0_32bit_from_src0(Tree, CodeGen, Instruction, States);
    }
    return gcvTRUE;
}

static gctBOOL
reverseConditionAndReverseEqualAndDeleteCall(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT codeIndex = (gctINT)(Instruction - Tree->shader->code);

    reverseConditionAndReverseEqual(Tree, CodeGen, Instruction, States);

    deleteCaller(Tree, Instruction->tempIndex, codeIndex);
    return gcvTRUE;
}

static gctBOOL
oneOperandCondition(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT32 gcCondition;
    gctUINT32 useSrc1 = 0;

    if (!isConditionCanBeOneOperand(Instruction, &gcCondition, &useSrc1))
    {
        return gcvFALSE;
    }

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) ((gctUINT32) (gcCondition) & ((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6)));
    if (_shaderNeedIntSupport(Tree, CodeGen, Instruction, States))
    {
        /* set the inst type base on instruction source 0*/
        value_type0_32bit_from_src0(Tree, CodeGen, Instruction, States);
    }
    return gcvTRUE;
}

static gctBOOL
oneOperandConditionAndDeleteSecondCaller(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_INSTRUCTION code = Instruction + 2;
    gctINT codeIndex = (gctINT)(code - Tree->shader->code);

    oneOperandCondition(Tree, CodeGen, Instruction, States);

    deleteCaller(Tree, code->tempIndex, codeIndex);
    return gcvTRUE;
}

static gctBOOL
branch(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_BRANCH_LIST entry;
    gctPOINTER pointer;

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) ((gctUINT32) (_sl2gcCondition((gcSL_CONDITION) gcmSL_TARGET_GET(Instruction->temp, Condition))) & ((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6)));

    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL,
                                   gcmSIZEOF(struct _gcSL_BRANCH_LIST),
                                   &pointer)))
    {
        return gcvFALSE;
    }
    entry = pointer;

    entry->next   = Tree->branch;
    entry->ip     = gcsCODE_GENERATOR_GetIP(CodeGen);
    entry->target = Instruction->tempIndex;
    entry->call   = (gcmSL_OPCODE_GET(Instruction->opcode, Opcode) == gcSL_CALL);
    entry->duplicatedT0T1 = gcvFALSE;
    if (CodeGen->isDual16Shader)
    {
        /* check if the branch need to be duplicated */

        /* phase 1 doesn't support CALL */
        if (!entry->call)
        {
            /* check if any source is high precision (except for unifrom,
             * which can be handled no matter high or medium precision
             */
            if (((gcmSL_SOURCE_GET(Instruction->source0, Type) == gcSL_TEMP )       &&
                  gcmSL_SOURCE_GET(Instruction->source0, Precision) == gcSL_PRECISION_HIGH) ||
                ((gcmSL_SOURCE_GET(Instruction->source1, Type) == gcSL_TEMP )       &&
                  gcmSL_SOURCE_GET(Instruction->source1, Precision) == gcSL_PRECISION_HIGH) ||
                ((gcmSL_SOURCE_GET(Instruction->source0, Type) == gcSL_ATTRIBUTE && /* check for sources being gl_FragCoord */
                  CodeGen->usePosition && Instruction->source0Index == CodeGen->positionIndex)) ||
                ((gcmSL_SOURCE_GET(Instruction->source1, Type) == gcSL_ATTRIBUTE &&
                  CodeGen->usePosition && Instruction->source1Index == CodeGen->positionIndex)))
            {
                entry->duplicatedT0T1 = gcvTRUE;
            }
        }
    }

    Tree->branch  = entry;
    if (_shaderNeedIntSupport(Tree, CodeGen, Instruction, States))
    {
        value_type0_from_src0(Tree, CodeGen, Instruction, States);
    }
    return gcvTRUE;
}

static gctBOOL
branchAndDeleteCaller(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT codeIndex = (gctINT)(Instruction - Tree->shader->code);

    branch(Tree, CodeGen, Instruction, States);

    deleteCaller(Tree, Instruction->tempIndex, codeIndex);

    return gcvTRUE;
}

static gctBOOL
_IsSourceSingleComponent(
    IN gctSOURCE_t           Source
    )
{
    gcSL_SWIZZLE swizzle = (gcSL_SWIZZLE) gcmSL_SOURCE_GET(Source, SwizzleX);

    /* temp.xxxx or temp.yyyy, ... */
    if (swizzle == (gcSL_SWIZZLE) gcmSL_SOURCE_GET(Source, SwizzleY) &&
        swizzle == (gcSL_SWIZZLE) gcmSL_SOURCE_GET(Source, SwizzleZ) &&
        swizzle == (gcSL_SWIZZLE) gcmSL_SOURCE_GET(Source, SwizzleW))
    {
        return gcvTRUE;
    }
    return gcvFALSE;
}

static gctBOOL
_NoLabel_CanUseSelectCmpSet(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT pc = (gctINT)(Instruction - Tree->shader->code);

    if (!canUseSelectCmpSetInst(CodeGen, Instruction))
    {
        return gcvFALSE;
    }

    if (Tree->hints[pc].callers != gcvNULL)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

static gctBOOL
_NoLabel(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT pc = (gctINT)(Instruction - Tree->shader->code);

    if (Tree->hints[pc].callers != gcvNULL)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

static gctBOOL
_NoLabel_Sat(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if(!_NoLabel(Tree, CodeGen, Instruction, States))
    {
        return gcvFALSE;
    }

    if(!gcmSL_OPCODE_GET(Instruction->opcode, Sat))
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

static gctBOOL
_NoLabel_Neg1(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if(!_NoLabel(Tree, CodeGen, Instruction, States))
    {
        return gcvFALSE;
    }

    if(!gcmSL_SOURCE_GET(Instruction->source1, Neg))
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

static gctBOOL
_NoLabel_Neg0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if(!_NoLabel(Tree, CodeGen, Instruction, States))
    {
        return gcvFALSE;
    }

    if(!gcmSL_SOURCE_GET(Instruction->source0, Neg))
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

static gctBOOL
_NoLabelAndConditionAlways(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT pc = (gctINT )(Instruction - Tree->shader->code);
    gcSL_CONDITION Condition;

    if (Tree->hints[pc].callers != gcvNULL)
    {
        return gcvFALSE;
    }

    Condition = (gcSL_CONDITION)gcmSL_TARGET_GET(Instruction->temp, Condition);

    return Condition == gcSL_ALWAYS;
}

static gctBOOL
_Const1_NoLabel(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT pc = (gctINT )(Instruction - Tree->shader->code);

    if (Tree->hints[pc].callers != gcvNULL)
    {
        return gcvFALSE;
    }

    /* check if the Src0 is constant one */
    return isSourceConstantf(Instruction, 0, 1.0) ;
}

static gctBOOL
_Const0_NoLabel(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT pc = (gctINT )(Instruction - Tree->shader->code);

    if (Tree->hints[pc].callers != gcvNULL)
    {
        return gcvFALSE;
    }

    /* check if the Src0 is constant one */
    return isSourceConstantf(Instruction, 0, 0.0) ;
}

static gctBOOL
_Src0Const0_UseDestInTwoOnly(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT temp = Instruction->tempIndex;
    if ((Tree->tempArray[temp].users == gcvNULL)
    ||  (Tree->tempArray[temp].users->next == gcvNULL)
    ||  (Tree->tempArray[temp].users->next->next != gcvNULL)
    )
    {
        return gcvFALSE;
    }

    if (_HasModInAllUses(Tree, temp))
    {
        return gcvFALSE;
    }

    /* check if the Src0 is constant one */
    return isSourceConstantf(Instruction, 0, 0.0);
}

static gctBOOL
_HasOneLabel(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT pc = (gctINT )(Instruction - Tree->shader->code);

    if (Tree->hints[pc].callers != gcvNULL &&
        Tree->hints[pc].callers->next == gcvNULL)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_Const0_HasOneLabel(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT pc = (gctINT )(Instruction - Tree->shader->code);

    /* has one label? */
    if (Tree->hints[pc].callers != gcvNULL &&
        Tree->hints[pc].callers->next == gcvNULL)
    {
        /* check if the Src0 is constant 0 */
        return isSourceConstantf(Instruction, 0, 0.0) ;
    }

    return gcvFALSE;
}

static gctBOOL
_Const1_HasOneLabel(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT pc = (gctINT )(Instruction - Tree->shader->code);

    /* has one label? */
    if (Tree->hints[pc].callers != gcvNULL &&
        Tree->hints[pc].callers->next == gcvNULL)
    {
        /* check if the Src0 is constant 1 */
        return isSourceConstantf(Instruction, 0, 1.0) ;
    }

    return gcvFALSE;
}

/* check to see if the jmp target is the one after next instruction */
static gctBOOL
_jmpToNextPlusOne(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT   jmpTarget     = Instruction->tempIndex;
    gctINT   curCodeIndex  = (gctINT )(Instruction - Tree->shader->code);

    return (jmpTarget == curCodeIndex+2) && _NoLabel(Tree, CodeGen, Instruction, States) ;
}

static gctBOOL
_hasFloatCompare_jmpToNextPlusOne_halti4(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (!_jmpToNextPlusOne(Tree, CodeGen, Instruction, States))
        return gcvFALSE;

    if (!(gcmSL_SOURCE_GET(Instruction->source0, Format) == gcSL_FLOAT || CodeGen->hasMediumPrecision))
        return gcvFALSE;

    return gcvTRUE;
}

/* check to see if the jmp target is the one after next instruction */
static gctBOOL
_jmpToNextPlusTwo(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT   jmpTarget     = Instruction->tempIndex;
    gctINT   curCodeIndex  = (gctINT )(Instruction - Tree->shader->code);

    return (jmpTarget == curCodeIndex+3) && _NoLabel(Tree, CodeGen, Instruction, States) ;
}

static gctBOOL
_hasFloatCompare_jmpToNextPlusTwo_halti4(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (!_jmpToNextPlusTwo(Tree, CodeGen, Instruction, States))
        return gcvFALSE;

    if (!(gcmSL_SOURCE_GET(Instruction->source0, Format) == gcSL_FLOAT || CodeGen->hasMediumPrecision))
        return gcvFALSE;

    return gcvTRUE;
}

extern gctUINT8
_Enable2SwizzleWShift(
    IN gctUINT32 Enable
    );

static gctBOOL
_jmpToNextPlusOneWithSameSourceAndFloatOperand(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT   jmpTarget     = Instruction->tempIndex;
    gctINT   curCodeIndex  = (gctINT )(Instruction - Tree->shader->code);
    gcSL_INSTRUCTION nextInst = Instruction + 1;
    gcSL_CONDITION reversedCondition;
    gcSL_FORMAT format     = gcmSL_SOURCE_GET(Instruction->source0, Format);

    if ((format != gcSL_FLOAT) ||
        (gcmSL_SOURCE_GET(Instruction->source0, Type) != gcSL_TEMP) ||
        (jmpTarget != curCodeIndex + 2) ||
        (nextInst->opcode != gcSL_MOV))
        return gcvFALSE;

    if ((gcmSL_SOURCE_GET(Instruction->source0, Swizzle) != _Enable2SwizzleWShift(gcmSL_TARGET_GET(nextInst->temp, Enable)) ||
                Instruction->source0Index != nextInst->tempIndex ||
                Instruction->source0Indexed != nextInst->tempIndexed))
    {
        return gcvFALSE;
    }

    if (!isConditionReversible((gcSL_CONDITION)gcmSL_TARGET_GET(Instruction->temp, Condition), &reversedCondition))
        return gcvFALSE;

    return _NoLabel(Tree, CodeGen, Instruction, States);
}

/* check if the jmp target is the one after next two instruction and float type target */
static gctBOOL
_jmpToNextPlusTwoAndFloatOperand(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT   jmpTarget     = Instruction->tempIndex;
    gctINT   curCodeIndex  = (gctINT )(Instruction - Tree->shader->code);
    gcSL_FORMAT format     = gcmSL_SOURCE_GET(Instruction->source0, Format);

    if (format != gcSL_FLOAT)
        return gcvFALSE;

    if (!_IsSourceSingleComponent(Instruction->source0))
        return gcvFALSE;

    return (jmpTarget == curCodeIndex+3)  && _NoLabel(Tree, CodeGen, Instruction, States);
}

/* check if the jmp target is the one after next two instruction and has CMP instruction */
static gctBOOL
_jmpToNextPlusTwo_hasCMP_NoFloatOperand(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT   jmpTarget     = Instruction->tempIndex;
    gctINT   curCodeIndex  = (gctINT )(Instruction - Tree->shader->code);
    gcSL_FORMAT format     = gcmSL_SOURCE_GET(Instruction->source0, Format);

    if (format == gcSL_FLOAT)
        return gcvFALSE;

    if (!CodeGen->hasCL || !_IsSourceSingleComponent(Instruction->source0))
        return gcvFALSE;

    return (jmpTarget == curCodeIndex+3)  && _NoLabel(Tree, CodeGen, Instruction, States);
}

/* check to see if the jmp target is the one after next two instruction
   and the comparison can only use one operand (src0)
 */
static gctBOOL
_jmpToNextPlusTwo_OneOperandCmp_0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT    jmpTarget     = Instruction->tempIndex;
    gctINT    curCodeIndex  = (gctINT)(Instruction - Tree->shader->code);
    gctUINT32 gcCondition;
    gctUINT32 useSrc1 = 0;

    if (!isConditionCanBeOneOperand(Instruction, &gcCondition, &useSrc1) || useSrc1)
        return gcvFALSE;

    if (!_IsSourceSingleComponent(Instruction->source0))
        return gcvFALSE;

    return (jmpTarget == curCodeIndex+3)  && _NoLabel(Tree, CodeGen, Instruction, States);
}

/* check to see if the jmp target is the one after next two instruction
   and the comparison can only use one operand (src1)
 */
static gctBOOL
_jmpToNextPlusTwo_OneOperandCmp_1(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT    jmpTarget     = Instruction->tempIndex;
    gctINT    curCodeIndex  = (gctINT)(Instruction - Tree->shader->code);
    gctUINT32 gcCondition;
    gctUINT32 useSrc1 = 0;

    if (!isConditionCanBeOneOperand(Instruction, &gcCondition, &useSrc1) && !useSrc1)
        return gcvFALSE;

    if (!_IsSourceSingleComponent(Instruction->source0))
        return gcvFALSE;

    return (jmpTarget == curCodeIndex+3)  && _NoLabel(Tree, CodeGen, Instruction, States);
}

static gctBOOL
_jmpToNextPlusTwoAndFloatOperand_reversibleCondition(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT   jmpTarget     = Instruction->tempIndex;
    gctINT   curCodeIndex  = (gctINT )(Instruction - Tree->shader->code);
    gcSL_CONDITION reversedCondition;
    gcSL_FORMAT format     = gcmSL_SOURCE_GET(Instruction->source0, Format);

    if (format != gcSL_FLOAT)
        return gcvFALSE;

    if (!_IsSourceSingleComponent(Instruction->source0))
        return gcvFALSE;

    return (jmpTarget == curCodeIndex+3)  && _NoLabel(Tree, CodeGen, Instruction, States) &&
           isConditionReversible((gcSL_CONDITION)gcmSL_TARGET_GET(Instruction->temp,
                                                                  Condition),
                                 &reversedCondition);
}

static gctBOOL
texkill(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) ((gctUINT32) (_sl2gcCondition((gcSL_CONDITION) gcmSL_TARGET_GET(Instruction->temp, Condition))) & ((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6)));

    return gcvTRUE;
}

static gctBOOL
add2mad(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT32_PTR code;
    gctINT32 srcAddress[3];
    gctINT32 srcType[3];

    /* Get previous instruction. */
    if (!_GetPreviousCode(CodeGen, &code))
    {
        /* No previous instruction. */
        return gcvTRUE;
    }

    /* Get source 0 and 1 types and addressses. */
    srcType[0] = (((((gctUINT32) (code[2])) >> (0 ? 5:3)) & ((gctUINT32) ((((1 ? 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1)))))) );
    srcType[1] = (((((gctUINT32) (code[3])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) );
    if (srcType[0] == 0x2)
    {
        srcAddress[0] = (((((gctUINT32) (code[1])) >> (0 ? 20:12)) & ((gctUINT32) ((((1 ? 20:12) - (0 ? 20:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:12) - (0 ? 20:12) + 1)))))) );
    }
    else
    {
        srcAddress[0] = -1;
    }
    if (srcType[1] == 0x2)
    {
        srcAddress[1] = (((((gctUINT32) (code[2])) >> (0 ? 15:7)) & ((gctUINT32) ((((1 ? 15:7) - (0 ? 15:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:7) - (0 ? 15:7) + 1)))))) );
    }
    else
    {
        srcAddress[1] = -1;
    }

    /* Check that the previous instruction is a MUL. */
    if (((((gctUINT32) (code[0])) >> (0 ? 5:0) & ((gctUINT32) ((((1 ? 5:0) - (0 ?
 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1)))))) == (0x03 & ((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1)))))))

    /* Both instructions must have no conditional code. */
    &&  ((((gctUINT32) (States[0])) >> (0 ? 10:6) & ((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1)))))) == (0x00 & ((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1)))))))
    &&  ((((gctUINT32) (code[0])) >> (0 ? 10:6) & ((gctUINT32) ((((1 ? 10:6) - (0 ?
 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1)))))) == (0x00 & ((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1)))))))

    /* Both instructions must have the same destination. */
    &&  ((((((gctUINT32) (States[0])) >> (0 ? 22:16)) & ((gctUINT32) ((((1 ? 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1)))))) ) ==
         (((((gctUINT32) (code[0])) >> (0 ? 22:16)) & ((gctUINT32) ((((1 ? 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1)))))) ))
    &&  ((((((gctUINT32) (States[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) ) ==
         (((((gctUINT32) (code[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) ))

    /* Add sources are not the same. */
    &&  (((((((gctUINT32) (States[2])) >> (0 ? 5:3)) & ((gctUINT32) ((((1 ? 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1)))))) ) !=
            (((((gctUINT32) (States[3])) >> (0 ? 30:28)) & ((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1)))))) ))
        || ((((((gctUINT32) (States[1])) >> (0 ? 20:12)) & ((gctUINT32) ((((1 ? 20:12) - (0 ? 20:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:12) - (0 ? 20:12) + 1)))))) ) !=
            (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ))
        || ((((((gctUINT32) (States[2])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) ) !=
            (((((gctUINT32) (States[3])) >> (0 ? 27:25)) & ((gctUINT32) ((((1 ? 27:25) - (0 ? 27:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1)))))) ))
        || ((((((gctUINT32) (States[1])) >> (0 ? 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1)))))) ) !=
            (((((gctUINT32) (States[3])) >> (0 ? 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1)))))) ))
        || ((((((gctUINT32) (States[1])) >> (0 ? 30:30)) & ((gctUINT32) ((((1 ? 30:30) - (0 ? 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1)))))) ) !=
            (((((gctUINT32) (States[3])) >> (0 ? 22:22)) & ((gctUINT32) ((((1 ? 22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1)))))) ))
        || ((((((gctUINT32) (States[1])) >> (0 ? 31:31)) & ((gctUINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1)))))) ) !=
            (((((gctUINT32) (States[3])) >> (0 ? 23:23)) & ((gctUINT32) ((((1 ? 23:23) - (0 ? 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1)))))) ))
        )
    /* MUL should not have SAT modifier */
    && (((((gctUINT32) (code[0])) >> (0 ? 11:11) & ((gctUINT32) ((((1 ?
 11:11) - (0 ? 11:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ?
 11:11) - (0 ? 11:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:
11) + 1))))))))
    )
    {
        gctBOOL diffConst;

        /* Get source 2 type and address. */
        srcType[2] = (((((gctUINT32) (States[3])) >> (0 ? 30:28)) & ((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1)))))) );
        if (srcType[2] == 0x2)
        {
            srcAddress[2] = (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) );
        }
        else
        {
            srcAddress[2] = -1;
        }

        /* Determine the usage of constants. */
        diffConst = gcvFALSE;
        if ((srcAddress[0] >= 0)
        &&  (srcAddress[2] >= 0)
        &&  (srcAddress[0] != srcAddress[2])
        )
        {
            /* Source 0 and 2 are using different constants. */
            diffConst = gcvTRUE;
        }

        if ((srcAddress[1] >= 0)
        &&  (srcAddress[2] >= 0)
        &&  (srcAddress[1] != srcAddress[2])
        )
        {
            /* Source 1 and 2 are using different constants. */
            diffConst = gcvTRUE;
        }

        /* First, see if the previous destination matches the current source0:
        **
        **  MUL rd, s0(0), s1(1)
        **  ADD rd, rd(0), s2(2)
        */
        if (((((((gctUINT32) (States[2])) >> (0 ? 5:3)) & ((gctUINT32) ((((1 ? 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1)))))) ) ==
             0x0)
        &&  ((((((gctUINT32) (States[1])) >> (0 ? 20:12)) & ((gctUINT32) ((((1 ? 20:12) - (0 ? 20:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:12) - (0 ? 20:12) + 1)))))) ) ==
             (((((gctUINT32) (code[0])) >> (0 ? 22:16)) & ((gctUINT32) ((((1 ? 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1)))))) ))
        &&  ((((((gctUINT32) (States[2])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) ) ==
             (((((gctUINT32) (code[0])) >> (0 ? 15:13)) & ((gctUINT32) ((((1 ? 15:13) - (0 ? 15:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:13) - (0 ? 15:13) + 1)))))) ))
        &&  ((((((gctUINT32) (States[1])) >> (0 ? 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1)))))) ) ==
             _Enable2Swizzle((((((gctUINT32) (code[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) )))
        &&  !diffConst
        )
        {
            /* Replace previous source2 with current source2 and change opcode
            ** to MAD:
            **
            **  MAD rd, s0(0), s1(1), s2(2)
            */
            code[0] = ((((gctUINT32) (code[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) (0x02 & ((gctUINT32) ((((1 ? 5:0) - (0 ? 5:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));
            code[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) ((((((gctUINT32) (code[3])) >> (0 ?
 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 2:0) - (0 ? 2:0) + 1)))))) )) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)));

            /* Expand ABS modifier for rd to MUL sources. */
            if ((((((gctUINT32) (States[1])) >> (0 ? 31:31)) & ((gctUINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1)))))) ))
            {
                gcSetSrcABS(code, 0);
                gcSetSrcABS(code, 1);
            }

            /* Copy NEG modifier for rd to MUL source0. */
            if ((((((gctUINT32) (States[1])) >> (0 ? 30:30)) & ((gctUINT32) ((((1 ? 30:30) - (0 ? 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1)))))) ))
            {
                gcSetSrcNEG(code, 0);
            }

            /* Disgard current instruction. */
            return gcvFALSE;
        }

        /* Get source 2 type and address. */
        srcType[2] = (((((gctUINT32) (States[2])) >> (0 ? 5:3)) & ((gctUINT32) ((((1 ? 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1)))))) );
        if (srcType[2] == 0x2)
        {
            srcAddress[2] = (((((gctUINT32) (States[1])) >> (0 ? 20:12)) & ((gctUINT32) ((((1 ? 20:12) - (0 ? 20:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:12) - (0 ? 20:12) + 1)))))) );
        }
        else
        {
            srcAddress[2] = -1;
        }

        /* Determine the usage of constants. */
        diffConst = gcvFALSE;
        if ((srcAddress[0] >= 0)
        &&  (srcAddress[2] >= 0)
        &&  (srcAddress[0] != srcAddress[2])
        )
        {
            /* Source 0 and 2 are using different constants. */
            diffConst = gcvTRUE;
        }

        if ((srcAddress[1] >= 0)
        &&  (srcAddress[2] >= 0)
        &&  (srcAddress[1] != srcAddress[2])
        )
        {
            /* Source 0 and 2 are using different constants. */
            /* Source 1 and 2 have different constants. */
            diffConst = gcvTRUE;
        }

        /* Otherwise, see if the previous destination matches the current
        **  source2:
        **
        **  MUL rd, s0(0), s1(1)
        **  ADD rd, s2(0), rd(2)
        */
        if (((((((gctUINT32) (States[3])) >> (0 ? 30:28)) & ((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1)))))) ) ==
             0x0)
        &&  ((((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ) ==
             (((((gctUINT32) (code[0])) >> (0 ? 22:16)) & ((gctUINT32) ((((1 ? 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1)))))) ))
        &&  ((((((gctUINT32) (States[3])) >> (0 ? 27:25)) & ((gctUINT32) ((((1 ? 27:25) - (0 ? 27:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1)))))) ) ==
             (((((gctUINT32) (code[0])) >> (0 ? 15:13)) & ((gctUINT32) ((((1 ? 15:13) - (0 ? 15:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:13) - (0 ? 15:13) + 1)))))) ))
        &&  ((((((gctUINT32) (States[3])) >> (0 ? 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1)))))) ) ==
             _Enable2Swizzle((((((gctUINT32) (code[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) )))
        &&  !diffConst
        )
        {

            /* Replace previous source2 with current source0 and change opcode
            ** to MAD:
            **
            **  MAD rd, s0(0), s1(1), s2(0)
            */
            code[0] = ((((gctUINT32) (code[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) (0x02 & ((gctUINT32) ((((1 ? 5:0) - (0 ? 5:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));
            code[3] = ((((gctUINT32) (code[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 3:3) - (0 ?
 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ?
 3:3)));
            code[3] = ((((gctUINT32) (code[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) ((((((gctUINT32) (States[1])) >> (0 ?
 20:12)) & ((gctUINT32) ((((1 ? 20:12) - (0 ? 20:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 20:12) - (0 ? 20:12) + 1)))))) )) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)));
            code[3] = ((((gctUINT32) (code[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) ((((((gctUINT32) (States[1])) >> (0 ?
 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 29:22) - (0 ? 29:22) + 1)))))) )) & ((gctUINT32) ((((1 ? 21:14) - (0 ?
 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));
            code[3] = ((((gctUINT32) (code[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) ((gctUINT32) ((((((gctUINT32) (States[1])) >> (0 ?
 30:30)) & ((gctUINT32) ((((1 ? 30:30) - (0 ? 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 30:30) - (0 ? 30:30) + 1)))))) )) & ((gctUINT32) ((((1 ? 22:22) - (0 ?
 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
 22:22)));
            code[3] = ((((gctUINT32) (code[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ? 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) ((((((gctUINT32) (States[1])) >> (0 ?
 31:31)) & ((gctUINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 31:31) - (0 ? 31:31) + 1)))))) )) & ((gctUINT32) ((((1 ? 23:23) - (0 ?
 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23)));
            code[3] = ((((gctUINT32) (code[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ? 27:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) ((((((gctUINT32) (States[2])) >> (0 ?
 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 2:0) - (0 ? 2:0) + 1)))))) )) & ((gctUINT32) ((((1 ? 27:25) - (0 ? 27:25) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)));
            code[3] = ((((gctUINT32) (code[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) ((((((gctUINT32) (States[2])) >> (0 ?
 5:3)) & ((gctUINT32) ((((1 ? 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 5:3) - (0 ? 5:3) + 1)))))) )) & ((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));

            /* Expand ABS modifier for rd to MUL sources. */
            if ((((((gctUINT32) (States[3])) >> (0 ? 23:23)) & ((gctUINT32) ((((1 ? 23:23) - (0 ? 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1)))))) ))
            {
                gcSetSrcABS(code, 0);
                gcSetSrcABS(code, 1);
            }

            /* Copy NEG modifier for rd to MUL source0. */
            if ((((((gctUINT32) (States[3])) >> (0 ? 22:22)) & ((gctUINT32) ((((1 ? 22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1)))))) ))
            {
                gcSetSrcNEG(code, 0);
            }

            /* Disgard current instruction. */
            return gcvFALSE;
        }
    }

    /* Nothing can be merged. */
    return gcvTRUE;
}

static gctBOOL
mov(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT32_PTR code;

    if (((((gctUINT32) (States[0])) >> (0 ? 10:6) & ((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1)))))) == (0x00 & ((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1)))))))
    &&  ((((((gctUINT32) (States[3])) >> (0 ? 30:28)) & ((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1)))))) ) ==
         0x0 )
    &&  ((((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ) ==
         (((((gctUINT32) (States[0])) >> (0 ? 22:16)) & ((gctUINT32) ((((1 ? 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1)))))) ) )
    &&  ((((((gctUINT32) (States[3])) >> (0 ? 27:25)) & ((gctUINT32) ((((1 ? 27:25) - (0 ? 27:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1)))))) ) ==
         (((((gctUINT32) (States[0])) >> (0 ? 15:13)) & ((gctUINT32) ((((1 ? 15:13) - (0 ? 15:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:13) - (0 ? 15:13) + 1)))))) ) )
    &&  ((((((gctUINT32) (States[3])) >> (0 ? 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1)))))) ) ==
         _Enable2Swizzle((((((gctUINT32) (States[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) )))
    &&  !(((((gctUINT32) (States[3])) >> (0 ? 22:22)) & ((gctUINT32) ((((1 ? 22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1)))))) )
    &&  !(((((gctUINT32) (States[3])) >> (0 ? 23:23)) & ((gctUINT32) ((((1 ? 23:23) - (0 ? 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1)))))) )
    &&   (((((gctUINT32) (States[0])) >> (0 ? 11:11)) & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1)))))) ) == 0
    )
    {
        /* Source and destination are the same. */
        return gcvFALSE;
    }

    if (! _codeHasCaller(Tree, CodeGen)
    &&  _GetPreviousCode(CodeGen, &code)
    &&  ((((gctUINT32) (code[0])) >> (0 ? 5:0) & ((gctUINT32) ((((1 ? 5:0) - (0 ?
 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1)))))) == (0x09 & ((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1)))))))
    &&  ((((((gctUINT32) (code[0])) >> (0 ? 22:16)) & ((gctUINT32) ((((1 ? 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1)))))) ) ==
         (((((gctUINT32) (States[0])) >> (0 ? 22:16)) & ((gctUINT32) ((((1 ? 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1)))))) ))
    &&  ((((((gctUINT32) (code[0])) >> (0 ? 15:13)) & ((gctUINT32) ((((1 ? 15:13) - (0 ? 15:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:13) - (0 ? 15:13) + 1)))))) ) ==
         (((((gctUINT32) (States[0])) >> (0 ? 15:13)) & ((gctUINT32) ((((1 ? 15:13) - (0 ? 15:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:13) - (0 ? 15:13) + 1)))))) ))
    &&  (!((((((gctUINT32) (code[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) ) &
           (((((gctUINT32) (States[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) )))
    &&  ((((((gctUINT32) (code[0])) >> (0 ? 11:11)) & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1)))))) ) ==
         (((((gctUINT32) (States[0])) >> (0 ? 11:11)) & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1)))))) ))
    &&  ((((((gctUINT32) (code[3])) >> (0 ? 30:28)) & ((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1)))))) ) ==
         (((((gctUINT32) (States[3])) >> (0 ? 30:28)) & ((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1)))))) ))
    &&  ((((((gctUINT32) (code[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ) ==
         (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ))
    &&  ((((((gctUINT32) (code[3])) >> (0 ? 27:25)) & ((gctUINT32) ((((1 ? 27:25) - (0 ? 27:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1)))))) ) ==
         (((((gctUINT32) (States[3])) >> (0 ? 27:25)) & ((gctUINT32) ((((1 ? 27:25) - (0 ? 27:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1)))))) ))
    &&  ((((((gctUINT32) (code[3])) >> (0 ? 22:22)) & ((gctUINT32) ((((1 ? 22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1)))))) ) ==
         (((((gctUINT32) (States[3])) >> (0 ? 22:22)) & ((gctUINT32) ((((1 ? 22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1)))))) ))
    &&  ((((((gctUINT32) (code[3])) >> (0 ? 23:23)) & ((gctUINT32) ((((1 ? 23:23) - (0 ? 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1)))))) ) ==
         (((((gctUINT32) (States[3])) >> (0 ? 23:23)) & ((gctUINT32) ((((1 ? 23:23) - (0 ? 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1)))))) ))
    )
    {
        gctSOURCE_t srcType = (((((gctUINT32) (code[3])) >> (0 ? 30:28)) & ((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1)))))) );
        gctUINT32 enable        = (((((gctUINT32) (States[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) );
        gctUINT32 codeSwizzle   = (((((gctUINT32) (code[3])) >> (0 ? 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1)))))) );
        gctUINT32 sourceSwizzle = (((((gctUINT32) (States[3])) >> (0 ? 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1)))))) );

        /* check if immediate values are the same */
        if (srcType == 0x7)
        {
            gctUINT32 immediate1;
            gctUINT32 immType1;
            gctUINT32 immediate2;
            gctUINT32 immType2;
            if (!(gcExtractSource20BitImmediate(States, 2, &immediate1, &immType1 ) &&
                  gcExtractSource20BitImmediate(code, 2, &immediate2, &immType2 )   &&
                  immediate1 == immediate2 && immType1 == immType2) )
            {
                return gcvTRUE;
            }
        }
        else if ((((((gctUINT32) (code[0])) >> (0 ? 22:16)) & ((gctUINT32) ((((1 ? 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1)))))) ) ==
            (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ))
        {
        /* Check if current instruction depends on previous instruction. */
            gctUINT32 codeEnable    = (((((gctUINT32) (code[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) );
            if ((codeEnable & gcSL_ENABLE_X) &&
                (((sourceSwizzle & 0x03) == 0x00) ||
                 ((sourceSwizzle & 0x0C) == 0x00) ||
                 ((sourceSwizzle & 0x30) == 0x00) ||
                 ((sourceSwizzle & 0xC0) == 0x00)))
            {
                return gcvTRUE;
            }

            if ((codeEnable & gcSL_ENABLE_Y) &&
                (((sourceSwizzle & 0x03) == 0x01) ||
                 ((sourceSwizzle & 0x0C) == 0x04) ||
                 ((sourceSwizzle & 0x30) == 0x10) ||
                 ((sourceSwizzle & 0xC0) == 0x40)))
            {
                return gcvTRUE;
            }

            if ((codeEnable & gcSL_ENABLE_Z) &&
                (((sourceSwizzle & 0x03) == 0x02) ||
                 ((sourceSwizzle & 0x0C) == 0x08) ||
                 ((sourceSwizzle & 0x30) == 0x20) ||
                 ((sourceSwizzle & 0xC0) == 0x80)))
            {
                return gcvTRUE;
            }

            if ((codeEnable & gcSL_ENABLE_W) &&
                (((sourceSwizzle & 0x03) == 0x03) ||
                 ((sourceSwizzle & 0x0C) == 0x0C) ||
                 ((sourceSwizzle & 0x30) == 0x30) ||
                 ((sourceSwizzle & 0xC0) == 0xC0)))
            {
                return gcvTRUE;
            }
        }

        if (enable & gcSL_ENABLE_X)
        {
            codeSwizzle = (codeSwizzle & ~0x03) | (sourceSwizzle & 0x03);
        }

        if (enable & gcSL_ENABLE_Y)
        {
            codeSwizzle = (codeSwizzle & ~0x0C) | (sourceSwizzle & 0x0C);
        }

        if (enable & gcSL_ENABLE_Z)
        {
            codeSwizzle = (codeSwizzle & ~0x30) | (sourceSwizzle & 0x30);
        }

        if (enable & gcSL_ENABLE_W)
        {
            codeSwizzle = (codeSwizzle & ~0xC0) | (sourceSwizzle & 0xC0);
        }

        code[0] = ((((gctUINT32) (code[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) ((((((gctUINT32) (code[0])) >> (0 ?
 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 26:23) - (0 ? 26:23) + 1)))))) ) | (((((gctUINT32) (States[0])) >> (0 ?
 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 26:23) - (0 ? 26:23) + 1)))))) )) & ((gctUINT32) ((((1 ? 26:23) - (0 ?
 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));
        if (srcType != 0x7)
            code[3] = ((((gctUINT32) (code[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (codeSwizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ?
 21:14)));

        return gcvFALSE;
    }

    return gcvTRUE;
}

static gctBOOL
one_0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  1.0f,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 0, index, swizzle, constType, States);

    return gcvTRUE;
}

static gctBOOL
one_2_rtz(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gctUINT value = (((((gctUINT32) (States[1])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) );
    gcSL_TYPE constType;

    /* set rtz rounding mode: steal sampler bits */
    value = (value & 0x4) | 0x1;
    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (value) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));

    /* create constant 1.0 uniform */
    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  1.0f,
                                  &index,
                                  &swizzle,
                                  &constType));

    /* set source2 to use the uniform */
    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    return gcvTRUE;
}

static gctBOOL
eight_2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                  CodeGen,
                                  8,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);
    value_type0_from_src0(Tree, CodeGen, Instruction, States);

    return gcvTRUE;
}

static gctBOOL
rcppi2_1_dot5_2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec2(Tree,
                                  CodeGen,
                                  1.0f / (2.0f * (float) M_PI),
                                  0.5f,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 1, index, _ReplicateSwizzle(swizzle, 0), constType, States);
    _UsingConstUniform(Tree, CodeGen, 2, index, _ReplicateSwizzle(swizzle, 1), constType, States);

    return gcvTRUE;
}

static gctBOOL
pi2_1_pi_2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec2(Tree,
                                  CodeGen,
                                  2.0f * (float) M_PI,
                                  (float) M_PI,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 1, index, _ReplicateSwizzle(swizzle, 0), constType, States);
    _UsingConstUniform(Tree, CodeGen, 2, index, _ReplicateSwizzle(swizzle, 1), constType, States);

    return gcvTRUE;
}

static gctBOOL
abs_0_zero_2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcSetSrcABS(States, 0);

    /* create constant 0.0 uniform */
    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  0.0f,
                                  &index,
                                  &swizzle,
                                  &constType));

    /* set source2 to use the uniform */
    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    return gcvTRUE;
}

static gctBOOL
zero_2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    /* create constant 0.0 uniform */
    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  0.0f,
                                  &index,
                                  &swizzle,
                                  &constType));

    /* set source2 to use the uniform */
    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    return gcvTRUE;
}

static gctBOOL
abs_0_abs_2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSetSrcABS(States, 0);
    gcSetSrcABS(States, 2);
    return gcvTRUE;
}

static gctBOOL
halfpi_0_abs_2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  (float) M_PI / 2.0f,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 0, index, swizzle, constType, States);
    gcSetSrcABS(States, 2);

    return gcvTRUE;
}

static gctBOOL
gt_abs_0_one_1(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  1.0f,
                                  &index,
                                  &swizzle,
                                  &constType));

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));
    gcSetSrcABS(States, 0);
    _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);

    return gcvTRUE;
}

static gctBOOL
rcppi2_1(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  2.0f / (float) M_PI,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);

    return gcvTRUE;
}

static gctBOOL
rcppi(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  1.0f / (float) M_PI,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);

    return gcvTRUE;
}

static gctBOOL
set_new_sin_cos_log_div(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (CodeGen->hasNEW_SIN_COS_LOG_DIV)
    {
        gctUINT value = (((((gctUINT32) (States[1])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) );

        value = (value & 0x4) | 0x1;
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (value) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));
    }

    return gcvTRUE;
}

static gctBOOL
set_src2_abs_set_new_sin_cos_log_div(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSetSrcABS(States, 2);

    if (CodeGen->hasNEW_SIN_COS_LOG_DIV)
    {
        gctUINT value = (((((gctUINT32) (States[1])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) );

        value = (value & 0x4) | 0x1;
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (value) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));
    }

    return gcvTRUE;
}

static gctBOOL
half_pi_2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  (float) M_PI / 2.0f,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    return gcvTRUE;
}

static gctBOOL
tan9_1_tan7_2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec2(Tree,
                                  CodeGen,
                                  5237760.0f / 239500800.0f,
                                  65280.0f / 1209600.0f,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 1, index, _ReplicateSwizzle(swizzle, 0), constType, States);
    _UsingConstUniform(Tree, CodeGen, 2, index, _ReplicateSwizzle(swizzle, 1), constType, States);

    return gcvTRUE;
}

static gctBOOL
tan5_2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  4032.0f / 30240.0f,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    return gcvTRUE;
}

static gctBOOL
tan3_2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  240.0f / 720.0f,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    return gcvTRUE;
}

static gctBOOL
asin9_1_asin7_2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec2(Tree,
                                  CodeGen,
                                  35.0f / 1152.0f,
                                  5.0f / 112.0f,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 1, index, _ReplicateSwizzle(swizzle, 0), constType, States);
    _UsingConstUniform(Tree, CodeGen, 2, index, _ReplicateSwizzle(swizzle, 1), constType, States);

    return gcvTRUE;
}

static gctBOOL
asin5_2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  3.0f / 40.0f,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    return gcvTRUE;
}

static gctBOOL
asin3_2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  1.0f / 6.0f,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    return gcvTRUE;
}

static gctBOOL
atan9_1_atan7_2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec2(Tree,
                                  CodeGen,
                                  0.023060280510707944f,
                                  0.09045060332177933f,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 1, index, _ReplicateSwizzle(swizzle, 0), constType, States);
    _UsingConstUniform(Tree, CodeGen, 2, index, _ReplicateSwizzle(swizzle, 1), constType, States);

    return gcvTRUE;
}

static gctBOOL
atan5_2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  0.18449097954748866f,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    return gcvTRUE;
}

static gctBOOL
atan3_2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  0.33168528523552876f,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    return gcvTRUE;
}

static gctBOOL
sin_factor9_1_factor7_2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec2(Tree,
                                  CodeGen,
                                  0.000002147873374269693200f, /*1.0f / 362880.0f,*/
                                  0.000192650026292540130000f, /*1.0f / 5040.0f,*/
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 1, index, _ReplicateSwizzle(swizzle, 0), constType, States);
    _UsingConstUniform(Tree, CodeGen, 2, index, _ReplicateSwizzle(swizzle, 1), constType, States);

    return gcvTRUE;
}

static gctBOOL
cos_factor8_1_factor6_2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec2(Tree,
                                  CodeGen,
                                  0.000018929871657746844000f, /*1.0f / 40320.0f,*/
                                  0.001342294854111969500000f, /*1.0f / 720.0f,*/
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 1, index, _ReplicateSwizzle(swizzle, 0), constType, States);
    _UsingConstUniform(Tree, CodeGen, 2, index, _ReplicateSwizzle(swizzle, 1), constType, States);

    return gcvTRUE;
}

static gctBOOL
sin_factor5_2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  0.008308985270559787800000f, /*1.0f / 120.0f,*/
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    return gcvTRUE;
}

static gctBOOL
cos_factor4_2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  0.041518036276102066000000f, /*1.0f / 24.0f,*/
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    return gcvTRUE;
}

static gctBOOL
sin_factor3_2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  0.166624382138252260000000f, /*1.0f / 6.0f,*/
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    return gcvTRUE;
}

static gctBOOL
cos_factor2_2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  0.499851584434509280000000f, /*1.0f / 2.0f,*/
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    return gcvTRUE;
}


static gctBOOL
one_2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  1.0f,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    return gcvTRUE;
}

static gctBOOL
sin_one_2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  0.999979376792907710000000f, /*1.0f,*/
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    return gcvTRUE;
}



static gctBOOL
one_2_value_type0_from_src0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                  CodeGen,
                                  1,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    value_type0_from_src0(Tree, CodeGen, Instruction, States);

    return gcvTRUE;
}

static gctBOOL
one_2_value_type0_from_src0_and_delete_second_caller(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_INSTRUCTION code = Instruction + 2;
    gctINT codeIndex = (gctINT)(code - Tree->shader->code);

    one_2_value_type0_from_src0(Tree, CodeGen, Instruction, States);

    deleteCaller(Tree, code->tempIndex, codeIndex);
    return gcvTRUE;
}

static gctBOOL
minusOne_2_value_type0_from_src0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    if (gcmSL_TARGET_GET(Instruction->temp, Format) == gcSL_FLOAT)
    {
        gcmVERIFY_OK(_AddConstantVec1(Tree,
                                      CodeGen,
                                      -1.0,
                                      &index,
                                      &swizzle,
                                      &constType));
    }
    else
    {
        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                       CodeGen,
                                       -1,
                                       &index,
                                       &swizzle,
                                       &constType));
    }

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    value_type0_from_src0(Tree, CodeGen, Instruction, States);

    return gcvTRUE;
}

static gctBOOL
One_2_value_type0_from_src0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  1,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    value_type0_from_src0(Tree, CodeGen, Instruction, States);

    return gcvTRUE;
}

static gctBOOL
_is_value_type_float(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT format = gcmSL_TARGET_GET(Instruction->temp, Format);

    if (format == gcSL_FLOAT)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_is_value_type_integer(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT format = gcmSL_TARGET_GET(Instruction->temp, Format);

    if (format == gcSL_INT32)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_hasSIGN_FLOOR_CEIL_and_float_type(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (! _hasSIGN_FLOOR_CEIL(Tree, CodeGen, Instruction, States))
    {
        return gcvFALSE;
    }
    else
    {
        return _is_value_type_float(Tree, CodeGen, Instruction, States);
    }
}

static gctBOOL
one_1_conditionGZ(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  1.0f,
                                  &index,
                                  &swizzle,
                                  &constType));

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));

    _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);

    return gcvTRUE;
}

static gctBOOL
one_1_conditionLZ(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  1.0f,
                                  &index,
                                  &swizzle,
                                  &constType));

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));

    _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);

    return gcvTRUE;
}

#if _USE_CONV_FOR_EXPLICIT_CONVERT_FUNCTION
static gctBOOL
value_type0_32bit(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT value_type0;
    gctUINT inst_type0;
    gctUINT inst_type1;
    gctUINT format = gcmSL_TARGET_GET(Instruction->temp, Format);

    if (CodeGen->isCL_X && !CodeGen->hasBugFixes11)
    {
        /* Convert it to 32-bit. */
        switch (format)
        {
        case gcSL_INT8:
        case gcSL_INT16:
        case gcSL_INT32:
        case gcSL_BOOLEAN:
            format = gcSL_INT32;
            break;

        case gcSL_UINT8:
        case gcSL_UINT16:
        case gcSL_UINT32:
            format = gcSL_UINT32;
            break;

        default:
            return gcvFALSE;
        }
    }

    value_type0 = type_conv[format];
    inst_type0 = value_type0 & 0x1;
    inst_type1 = value_type0 >> 1;

    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) ((gctUINT32) (inst_type0) & ((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21)));
    States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (inst_type1) & ((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30)));

    return gcvTRUE;
}

static gctBOOL
_value_type0_32bit_from_src0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT format = gcmSL_SOURCE_GET(Instruction->source0, Format);
    gctUINT value_type0;
    gctUINT inst_type0;
    gctUINT inst_type1;

    if (CodeGen->isCL_X && !CodeGen->hasBugFixes11)
    {
        /* Convert it to 32-bit. */
        switch (format)
        {
        case gcSL_INT8:
        case gcSL_INT16:
        case gcSL_INT32:
        case gcSL_BOOLEAN:
            format = gcSL_INT32;
            break;

        case gcSL_UINT8:
        case gcSL_UINT16:
        case gcSL_UINT32:
            format = gcSL_UINT32;
            break;

        default:
            return gcvFALSE;
        }
    }

    value_type0 = type_conv[format];
    inst_type0 = value_type0 & 0x1;
    inst_type1 = value_type0 >> 1;

    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) ((gctUINT32) (inst_type0) & ((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21)));
    States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (inst_type1) & ((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30)));

    return gcvTRUE;
}

static gctBOOL
rounding_mode_value_type0_32bit_from_src0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    return _value_type0_32bit_from_src0(Tree, CodeGen, Instruction,States) &&
        rounding_mode(Tree, CodeGen, Instruction, States);

}

static gctBOOL
max_type0_const_conditionGT(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gctUINT format = gcmSL_TARGET_GET(Instruction->temp, Format);
    union
    {
        gctFLOAT f;
        gctUINT32 u32;
    } value;
    gcSL_TYPE constType;

    switch (format)
    {
    case gcSL_UINT8:
        value.u32 = 255;
        break;

    case gcSL_INT8:
        value.u32 = 127;
        break;

    case gcSL_UINT16:
        value.u32 = 65535;
        break;

    case gcSL_INT16:
        value.u32 = 32767;
        break;

    case gcSL_UINT32:
        value.u32 = 0xFFFFFFFF;
        break;

    case gcSL_INT32:
        value.u32 = 2147483647;
        break;

    default:
        return gcvFALSE;
    }

    format = (gcSL_FORMAT)gcmSL_SOURCE_GET(Instruction->source0, Format);
    if (format == gcSL_FLOAT)
    {
        value.f = (gctFLOAT) value.u32;
    }
    else
    {
        gctUINT value_type0;

        /* get the convert from type */
        gcmASSERT(gcmSL_SOURCE_GET(Instruction->source1, Type) == gcSL_CONSTANT);
        /* Assemble the 32-bit value. */
        gcmASSERT(gcmSL_SOURCE_GET(Instruction->source1, Format) == gcSL_UINT32);

        format = (gcSL_FORMAT)(Instruction->source1Index |
                                Instruction->source1Indexed << 16);

        if (CodeGen->isCL_X && !CodeGen->hasBugFixes11)
        {
            /* Convert 8-bit&16-bit to 32-bit. */
            switch (format)
            {
            case gcSL_INT8:
            case gcSL_INT16:
            case gcSL_INT32:
                format = gcSL_INT32;
                break;

            case gcSL_UINT8:
            case gcSL_UINT16:
            case gcSL_UINT32:
                format = gcSL_UINT32;
                break;

            default:
                return gcvFALSE;
            }
        }

        value_type0 = type_conv[format];
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) ((gctUINT32) (value_type0 & 0x1) & ((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21)));
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (value_type0 >> 1) & ((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30)));
    }

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  value.f,
                                  &index,
                                  &swizzle,
                                  &constType));

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));

    _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);

    return gcvTRUE;
}

static gctBOOL
min_type0_const_conditionLT(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gctUINT format = gcmSL_TARGET_GET(Instruction->temp, Format);
    union
    {
        gctFLOAT f;
        gctINT32 i32;
    } value;
    gcSL_TYPE constType;

    switch (format)
    {
    case gcSL_UINT8:
    case gcSL_UINT16:
    case gcSL_UINT32:
        value.i32 = 0;
        break;

    case gcSL_INT8:
        value.i32 = -128;
        break;

    case gcSL_INT16:
        value.i32 = -32768;
        break;

    case gcSL_INT32:
        value.i32 = (-2147483647 - 1);
        break;

    default:
        return gcvFALSE;
    }

    format = (gcSL_FORMAT)gcmSL_SOURCE_GET(Instruction->source0, Format);
    if (format == gcSL_FLOAT)
    {
        value.f = (gctFLOAT) value.i32;
    }
    else
    {
        gctUINT value_type0;

        /* get the convert from type */
        gcmASSERT(gcmSL_SOURCE_GET(Instruction->source1, Type) == gcSL_CONSTANT);
        /* Assemble the 32-bit value. */
        gcmASSERT(gcmSL_SOURCE_GET(Instruction->source1, Format) == gcSL_UINT32);

        format = (gcSL_FORMAT)(Instruction->source1Index |
                                Instruction->source1Indexed << 16);

        if (CodeGen->isCL_X && !CodeGen->hasBugFixes11)
        {
            /* Convert 8-bit&16-bit to 32-bit. */
            switch (format)
            {
            case gcSL_INT8:
            case gcSL_INT16:
            case gcSL_INT32:
                format = gcSL_INT32;
                break;

            case gcSL_UINT8:
            case gcSL_UINT16:
            case gcSL_UINT32:
                format = gcSL_UINT32;
                break;

            default:
                return gcvFALSE;
            }
        }

        value_type0 = type_conv[format];
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) ((gctUINT32) (value_type0 & 0x1) & ((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21)));
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (value_type0 >> 1) & ((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30)));
    }

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  value.f,
                                  &index,
                                  &swizzle,
                                  &constType));

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x02 & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));

    _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);

    return gcvTRUE;
}
#endif

static gctBOOL
zero_1(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  0.0f,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);

    return gcvTRUE;
}

static gctBOOL
zero_1_conditionEQ(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  0.0f,
                                  &index,
                                  &swizzle,
                                  &constType));

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x05 & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));

    _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);

    return gcvTRUE;
}

static gctBOOL
zero_2_enable_w(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  0.0f,
                                  &index,
                                  &swizzle,
                                  &constType));

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (gcSL_ENABLE_W) & ((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    return gcvTRUE;
}

static gctBOOL
int_one_1_conditionGZ(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                   CodeGen,
                                   1,
                                   &index,
                                   &swizzle,
                                   &constType));

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));

    _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);

    _SetValueType0(0x2, States);

    return gcvTRUE;
}

static gctBOOL
int_minus_one_1_conditionLZ(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                   CodeGen,
                                   -1,
                                   &index,
                                   &swizzle,
                                   &constType));

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));

    _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);

    _SetValueType0(0x2, States);

    return gcvTRUE;
}

static gctBOOL
int_zero_1_conditionEQ(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                   CodeGen,
                                   0,
                                   &index,
                                   &swizzle,
                                   &constType));

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x05 & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));

    _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);

    _SetValueType0(0x2, States);

    return gcvTRUE;
}

static gctBOOL
smallest0_2_GZ(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantVec1(Tree,
                                  CodeGen,
                                  (CodeGen->hasHalti4 || Tree->shader->type == gcSHADER_TYPE_CL) ? 0.0f : 1.175494351e-038f,
                                  &index,
                                  &swizzle,
                                  &constType));

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    return gcvTRUE;
}
/*
static gctBOOL
enable_x(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (gcSL_ENABLE_X) & ((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));

    return gcvTRUE;
}

static gctBOOL
enable_y(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (gcSL_ENABLE_Y) & ((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));

    return gcvTRUE;
}
*/
static gctBOOL
enable_w(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (gcSL_ENABLE_W) & ((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));

    return gcvTRUE;
}

static gctBOOL
source0_add_swizzle_w(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT8 sourceSwizzle;
    gctUINT8 newSourceSwizzle;

    sourceSwizzle  = (((((gctUINT32) (States[1])) >> (0 ? 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1)))))) );

    newSourceSwizzle = gcmComposeSwizzle(gcmExtractSwizzle(sourceSwizzle, 0),
                                         gcmExtractSwizzle(sourceSwizzle, 1),
                                         gcmExtractSwizzle(sourceSwizzle, 2),
                                         gcSL_SWIZZLE_W );

    /* adjust swizzle to use .w */
    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (newSourceSwizzle) & ((gctUINT32) ((((1 ?
 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ?
 29:22)));

    return gcvTRUE;
}

static gctBOOL
_InsertNop(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcOPTIMIZER_OPTION * opt = gcGetOptimizerOption();
    /* insert a nop after the instruction if insertNOP is on */
    if ((Instruction->opcode == gcSL_MUL && opt->insertNOPAfterMUL) ||
        (Instruction->opcode == gcSL_MULLO && opt->insertNOPAfterMULLO) ||
        (Instruction->opcode == gcSL_DP3 && opt->insertNOPAfterDP3) ||
        (Instruction->opcode == gcSL_DP4 && opt->insertNOPAfterDP4 ))
        return gcvTRUE;

    return gcvFALSE;
}

#define IS_SATURATE(Opcode) \
    (gcmSL_OPCODE_GET((Opcode), Sat) == gcSL_SATURATE)

static gctBOOL
_UseDestInNextOnlyAndMADOn(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT temp;
    gcOPTIMIZER_OPTION * opt = gcGetOptimizerOption();

    /* do not generate MAD if split mad is on */
    if (opt->splitMAD)
        return gcvFALSE;

    /* do not generate MAD if mul has sat modifier */
    if (IS_SATURATE(Instruction->opcode))
    {
        return gcvFALSE;
    }

    temp = Instruction->tempIndex;
    if ((Tree->tempArray[temp].users == gcvNULL)
    ||  (Tree->tempArray[temp].users->next != gcvNULL)
    )
    {
        return gcvFALSE;
    }

    if (_HasModInAllUses(Tree, temp))
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

#if (GC_ENABLE_DUAL_FP16 > 0)
static gctBOOL
_Dual16On_UseDestInNextOnlyAndMADOn(
                                    IN gcLINKTREE Tree,
                                    IN gcsCODE_GENERATOR_PTR CodeGen,
                                    IN gcSL_INSTRUCTION Instruction,
                                    IN OUT gctUINT32 * States
                                    )
{
    if (CodeGen->isDual16Shader &&
        gcmSL_TARGET_GET(Instruction->temp, Precision) == gcSL_PRECISION_HIGH &&
        gcmSL_SOURCE_GET(Instruction->source0, Precision) == gcSL_PRECISION_MEDIUM &&
        gcmSL_SOURCE_GET(Instruction->source1, Precision) == gcSL_PRECISION_HIGH &&
        gcmSL_SOURCE_GET(Instruction->source1, Type) == gcSL_UNIFORM)
    {
        return _UseDestInNextOnlyAndMADOn(Tree, CodeGen, Instruction, States);
    }

    return gcvFALSE;
}
#endif

static gctBOOL
_UseDestInNextOnly_hasMULLO_ICLimit(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT temp = Instruction->tempIndex;
    /* MULLO is part of OpenCL support, PXA988 (GC1000) doesn't have it */
    if (!CodeGen->hasCL)
    {
        return gcvFALSE;
    }
    if ((Tree->tempArray[temp].users == gcvNULL)
    ||  (Tree->tempArray[temp].users->next != gcvNULL)
    )
    {
        return gcvFALSE;
    }

    if (_HasModInAllUses(Tree, temp))
    {
        return gcvFALSE;
    }

    /* A temp WAR for HW inst limitation */
    if (Tree->shader->codeCount >= 200 &&
        !CodeGen->useICache &&
        CodeGen->hardware->config->instructionCount < 1024)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

static gctBOOL
_UseDestInNextTwoOnly(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT temp = Instruction->tempIndex;

    if (_UseDestInNextOnly(Tree, CodeGen, Instruction, States))
        return gcvTRUE;

    if ((Tree->tempArray[temp].users == gcvNULL)
    ||  (Tree->tempArray[temp].users->next == gcvNULL)
    ||  (Tree->tempArray[temp].users->next->next != gcvNULL)
    )
    {
        return gcvFALSE;
    }

    if (_HasModInAllUses(Tree, temp))
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

static gctBOOL
_UsedAsIndexingOnly(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT temp = Instruction->tempIndex;

    if (Tree->tempArray[temp].isIndex)
    {
        return gcvTRUE;
    }
    /* its user is used as index */
    if (Tree->tempArray[temp].users != gcvNULL)
    {
        gcsLINKTREE_LIST_PTR users = Tree->tempArray[temp].users;
        gcSL_INSTRUCTION code = &Tree->shader->code[users->index];
        if (gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_MUL)
        {
            gctUINT index = code->tempIndex;
            if (Tree->tempArray[index].isIndex)
                return gcvTRUE;
        }
    }
    return gcvFALSE;
}

static gctBOOL
_set_helper_or_not(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_CONDITION Condition = (gcSL_CONDITION)gcmSL_TARGET_GET(Instruction->temp, Condition);
    gcSL_TYPE      src0Type = (gcSL_TYPE)gcmSL_SOURCE_GET(Instruction->source0, Type);
    gcSL_FORMAT    src0Fmt = (gcSL_FORMAT)gcmSL_SOURCE_GET(Instruction->source0, Format);
    gctUINT16      src0Idx = gcmSL_INDEX_GET(Instruction->source0Index, Index);
    gcSL_TYPE      src1Type = (gcSL_TYPE)gcmSL_SOURCE_GET(Instruction->source1, Type);
    gcSL_FORMAT    src1Fmt = (gcSL_FORMAT)gcmSL_SOURCE_GET(Instruction->source1, Format);

    value_type0_32bit_from_src0(Tree, CodeGen, Instruction, States);

    if (Condition == gcSL_EQUAL &&
        src0Type == gcSL_ATTRIBUTE &&
        src0Fmt == gcSL_BOOLEAN &&
        Tree->shader->attributes[src0Idx]->nameLength == gcSL_HELPER_INVOCATION &&
        src1Type == gcSL_CONSTANT &&
        src1Fmt == gcSL_BOOLEAN &&
        Instruction->source1Index == 1)
    {
        States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x18 & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));
    }

    return gcvTRUE;
}

static gcSL_ENABLE
_GetUsedComponents(
    IN gcSL_INSTRUCTION Instruction,
    IN gctINT           SourceNo
    )
{
    gctUINT16    enable = gcmSL_TARGET_GET(Instruction->temp, Enable);
    gctSOURCE_t  source = SourceNo == 0 ? Instruction->source0 : Instruction->source1;
    gcSL_ENABLE  usedComponents = 0;
    gctINT       i;

    for (i=0; i < gcSL_COMPONENT_COUNT; i++)
    {
        if (gcmIsComponentEnabled(enable, i))
        {
            gcSL_SWIZZLE swizzle = 0;
            switch (i)
            {
            case 0:
                swizzle = (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleX);
                break;
            case 1:
                swizzle = (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleY);
                break;
            case 2:
                swizzle = (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleZ);
                break;
            case 3:
                swizzle = (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleW);
                break;
            default:
                gcmASSERT(0);
                break;
            } /* switch */
            /* the component in swizzle is used, convert swizzle to enable */
            usedComponents |= 1 << swizzle;
        } /* if */
    } /* for */
    return usedComponents;
}

static gctBOOL
_HandleBiasedTextureLoad(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT temp = gcmSL_INDEX_GET(Instruction->source1Index, Index);
    gctUINT32 components = 0, rows = 0;

    /* source is Attribute ot Temp ?*/
    if (gcmSL_SOURCE_GET(Instruction->source1, Type) == gcSL_TEMP)
    {
        /* don't know if the temp is allocated in proper register which
         * can extend to use w component, so skip it */
        return gcvFALSE;

    }
    else if (gcmSL_SOURCE_GET(Instruction->source1, Type) == gcSL_ATTRIBUTE)
    {
        gcsLINKTREE_LIST_PTR users;

        gcSHADER_TYPE type = Tree->shader->attributes[temp]->type;
        gcTYPE_GetTypeInfo(Tree->shader->attributes[temp]->type,
                           &components,
                           &rows,
                           0);
        rows *= Tree->shader->attributes[temp]->arraySize;
        if (rows > 1)
        {
            /* cannot handle multiple rows */
            return gcvFALSE;
        }

        if (type == gcSHADER_FLOAT_X4)
        {
            /* check if the w component is used in any users */
            users = Tree->attributeArray[temp].users;

            for (; users != gcvNULL; users = users->next)
            {
                gcSL_INSTRUCTION code = &Tree->shader->code[users->index];
                gcSL_ENABLE      usedComponents = 0;
                switch (gcmSL_OPCODE_GET(code->opcode, Opcode)) {
                case gcSL_TEXLD:
                case gcSL_TEXLDPROJ:
                case gcSL_TEXLDPCF:
                case gcSL_TEXLDPCFPROJ:
                case gcSL_TEXBIAS:
                case gcSL_TEXGRAD:
                case gcSL_TEXGATHER:
                case gcSL_TEXFETCH_MS:
                case gcSL_TEXLOD:
                case gcSL_JMP:
                case gcSL_CALL:
                case gcSL_RET:
                case gcSL_KILL:
                case gcSL_TEXU:
                case gcSL_TEXU_LOD:
                    continue;
                    /* break; */
                default :
                    /* check source 0 */
                    if (gcmSL_SOURCE_GET(code->source0, Type) == gcSL_ATTRIBUTE &&
                        gcmSL_INDEX_GET(code->source0Index, Index) == temp)
                    {
                        usedComponents = _GetUsedComponents(code, 0);
                        if ((usedComponents & gcSL_ENABLE_W) != 0)
                            return gcvFALSE;
                    }
                    /* check source 1 */
                    if (gcmSL_SOURCE_GET(code->source1, Type) == gcSL_ATTRIBUTE &&
                        gcmSL_INDEX_GET(code->source1Index, Index) == temp)
                    {
                        usedComponents = _GetUsedComponents(code, 1);
                        if ((usedComponents & gcSL_ENABLE_W) != 0)
                            return gcvFALSE;
                    }
                    break;
                }
            }
        }
        else  /* not gcSHADER_FLOAT_X4 */
        {
            /* change the type to gcSHADER_FLOAT_X4 so we can use the w component */
            gcmASSERT(components == 2 || components == 3);
            Tree->shader->attributes[temp]->type = gcSHADER_FLOAT_X4;
        }

        return gcvTRUE;
    }
    return gcvFALSE;
}

static gctBOOL
_OnlyXYOrXEnabled(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_ENABLE  enable = gcmSL_TARGET_GET(Instruction->temp, Enable);
    return enable == (gcSL_ENABLE_X | gcSL_ENABLE_Y) ||
           enable == gcSL_ENABLE_X;
}

static gctBOOL
_OnlyXYZEnabled(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_ENABLE  enable = gcmSL_TARGET_GET(Instruction->temp, Enable);
    return enable == (gcSL_ENABLE_X | gcSL_ENABLE_Y | gcSL_ENABLE_Z);
}

static gctBOOL
_XYZWEnabled(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_ENABLE  enable = gcmSL_TARGET_GET(Instruction->temp, Enable);
    return enable == (gcSL_ENABLE_X | gcSL_ENABLE_Y | gcSL_ENABLE_Z | gcSL_ENABLE_W);
}

static gctBOOL
int_value_type0_const_0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                   CodeGen,
                                   0x0,
                                   &index,
                                   &swizzle,
                                   &constType));

    _UsingConstUniform(Tree, CodeGen, 0, index, swizzle, constType, States);

    _SetValueType0(0x2, States);

    return gcvTRUE;
}

static gctBOOL
int_value_type0_const_7F800000(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                   CodeGen,
                                   0x7F800000,
                                   &index,
                                   &swizzle,
                                   &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x2, States);

    return gcvTRUE;
}

static gctBOOL
int_value_type0_const_23(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                   CodeGen,
                                   23,
                                   &index,
                                   &swizzle,
                                   &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x2, States);

    return gcvTRUE;
}

static gctBOOL
int_value_type0_const_Minus127(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                   CodeGen,
                                   0xFFFFFF81,
                                   &index,
                                   &swizzle,
                                   &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x2, States);

    return gcvTRUE;
}

static gctBOOL
uint_value_type0_const_7FFFFF(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                   CodeGen,
                                   0x007FFFFF,
                                   &index,
                                   &swizzle,
                                   &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x5, States);

    return gcvTRUE;
}

static gctBOOL
uint_value_type0_const_800000(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                   CodeGen,
                                   0x00800000,
                                   &index,
                                   &swizzle,
                                   &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x5, States);

    return gcvTRUE;
}

static gctBOOL
int_value_type0_const_24_16(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_FORMAT format = gcmSL_TARGET_GET(Instruction->temp, Format);
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    if (format == gcSL_INT8)
    {
        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                       CodeGen,
                                       24,
                                       &index,
                                       &swizzle,
                                       &constType));
    }
    else
    {
        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                       CodeGen,
                                       16,
                                       &index,
                                       &swizzle,
                                       &constType));
    }

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x2, States);

    return gcvTRUE;
}

static gctBOOL
uint_value_type0_const_FF_FFFF(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_FORMAT format = gcmSL_TARGET_GET(Instruction->temp, Format);
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    if (format == gcSL_UINT8)
    {
        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                       CodeGen,
                                       0x000000FF,
                                       &index,
                                       &swizzle,
                                       &constType));
    }
    else
    {
        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                       CodeGen,
                                       0x0000FFFF,
                                       &index,
                                       &swizzle,
                                       &constType));
    }

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x5, States);

    return gcvTRUE;
}

static gctBOOL
value_type0_const_FF(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                   CodeGen,
                                   0x000000FF,
                                   &index,
                                   &swizzle,
                                   &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    value_type0(Tree, CodeGen, Instruction, States);

    return gcvTRUE;
}

static gctBOOL
value_type0_const_FFFF(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                   CodeGen,
                                   0x0000FFFF,
                                   &index,
                                   &swizzle,
                                   &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    value_type0(Tree, CodeGen, Instruction, States);

    return gcvTRUE;
}

static gctBOOL
short_value_type0_const_8(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                   CodeGen,
                                   8,
                                   &index,
                                   &swizzle,
                                   &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x3, States);

    return gcvTRUE;
}

static gctBOOL
int_value_type0_const_16(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                   CodeGen,
                                   16,
                                   &index,
                                   &swizzle,
                                   &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x2, States);

    return gcvTRUE;
}

static gctBOOL
int_value_type0_const_24(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                   CodeGen,
                                   24,
                                   &index,
                                   &swizzle,
                                   &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x2, States);

    return gcvTRUE;
}

static gctBOOL
int_value_type0_const_24_16_from_next_inst(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_INSTRUCTION nextInstruction = Instruction + 1;
    gcSL_FORMAT format = gcmSL_TARGET_GET(nextInstruction->temp, Format);
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    if (format == gcSL_INT8)
    {
        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                       CodeGen,
                                       24,
                                       &index,
                                       &swizzle,
                                       &constType));
    }
    else
    {
        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                       CodeGen,
                                       16,
                                       &index,
                                       &swizzle,
                                       &constType));
    }

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x2, States);

    return gcvTRUE;
}

static gctBOOL
uint_value_type0_const_FF_FFFF_from_next_inst(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_INSTRUCTION nextInstruction = Instruction + 1;
    gcSL_FORMAT format = gcmSL_TARGET_GET(nextInstruction->temp, Format);
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    if (format == gcSL_UINT8)
    {
        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                       CodeGen,
                                       0x000000FF,
                                       &index,
                                       &swizzle,
                                       &constType));
    }
    else
    {
        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                       CodeGen,
                                       0x0000FFFF,
                                       &index,
                                       &swizzle,
                                       &constType));
    }

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x5, States);

    return gcvTRUE;
}


static gctBOOL
value_type0_const_0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT    index      = 0;
    gctUINT8  swizzle    = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                   CodeGen,
                                   0,
                                   &index,
                                   &swizzle,
                                   &constType));

    _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);

    value_type0(Tree, CodeGen, Instruction, States);

    return gcvTRUE;
}

static gctBOOL
value_type0_immediate_or_const_0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT    index      = 0;
    gctUINT8  swizzle    = 0;
    gcSL_TYPE constType;

    if (Generate20BitsImmediate(CodeGen, Instruction, 0))
    {
        gcsConstantValue constValue;
        constValue.ty = gcmSL_TARGET_GET(Instruction->temp, Format);
        constValue.value.u = 0;

        gcmVERIFY_OK(gcEncodeSourceImmediate20(States,
                                               0,
                                               &constValue));
    }
    else
    {
        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                       CodeGen,
                                       0,
                                       &index,
                                       &swizzle,
                                       &constType));

        _UsingConstUniform(Tree, CodeGen, 0, index, swizzle, constType, States);
    }

    value_type0(Tree, CodeGen, Instruction, States);

    return gcvTRUE;
}

static gctBOOL
denorm_value_type0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT format = gcmSL_TARGET_GET(Instruction->temp, Format);
    gctUINT value_type0 = type_conv[format];
    gctUINT inst_type0 = value_type0 & 0x1;
    gctUINT inst_type1 = value_type0 >> 1;

    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) ((gctUINT32) (inst_type0) & ((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21)));
    States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (inst_type1) & ((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30)));

    if (CodeGen->hasSHEnhancements2)
    {
        gctUINT8 swizzle = (((((gctUINT32) (States[1])) >> (0 ? 10:3)) & ((gctUINT32) ((((1 ? 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1)))))) );
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3))) | (((gctUINT32) ((gctUINT32) (swizzle | 0x80) & ((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3)));
    }

    return gcvTRUE;
}

static gctBOOL
denorm_value_type0_from_next_inst(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_INSTRUCTION nextInstruction = Instruction + 1;
    gctUINT format = gcmSL_TARGET_GET(nextInstruction->temp, Format);
    gctUINT value_type0 = type_conv[format];
    gctUINT inst_type0 = value_type0 & 0x1;
    gctUINT inst_type1 = value_type0 >> 1;

    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) ((gctUINT32) (inst_type0) & ((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21)));
    States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (inst_type1) & ((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30)));

    if (CodeGen->hasSHEnhancements2)
    {
        gctUINT8 swizzle = (((((gctUINT32) (States[1])) >> (0 ? 10:3)) & ((gctUINT32) ((((1 ? 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1)))))) );
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3))) | (((gctUINT32) ((gctUINT32) (swizzle | 0x80) & ((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3)));
    }

    return gcvTRUE;
}

static gctBOOL
denorm_value_type0_const_0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                   CodeGen,
                                   0x0,
                                   &index,
                                   &swizzle,
                                   &constType));

    _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);

    value_type0(Tree, CodeGen, Instruction, States);

    if (CodeGen->hasSHEnhancements2)
    {
        swizzle = (((((gctUINT32) (States[1])) >> (0 ? 10:3)) & ((gctUINT32) ((((1 ? 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1)))))) );
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3))) | (((gctUINT32) ((gctUINT32) (swizzle | 0x80) & ((gctUINT32) ((((1 ?
 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ?
 10:3)));
    }

    return gcvTRUE;
}

static gctBOOL
_IntUseDestInNextOnly(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (gcmSL_TARGET_GET(Instruction->temp, Format) == gcSL_FLOAT)
    {
        return gcvFALSE;
    }
    else
    {
        gctINT temp = Instruction->tempIndex;
        if ((Tree->tempArray[temp].users == gcvNULL)
        ||  (Tree->tempArray[temp].users->next != gcvNULL)
        )
        {
            return gcvFALSE;
        }

        if (_HasModInAllUses(Tree, temp))
        {
            return gcvFALSE;
        }

        return gcvTRUE;
    }
}

static gctBOOL
_IntOpcodeAndHasIABS(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (!CodeGen->hasCL ||
        gcmSL_TARGET_GET(Instruction->temp, Format) == gcSL_FLOAT)
    {
        return gcvFALSE;
    }
    else
    {
        return gcvTRUE;
    }
}

static gctBOOL
_IntOpcode(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (gcmSL_TARGET_GET(Instruction->temp, Format) == gcSL_FLOAT)
    {
        return gcvFALSE;
    }
    else
    {
        return gcvTRUE;
    }
}

static gctBOOL
_IsNZ(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (gcmSL_TARGET_GET(Instruction->temp, Condition) == gcSL_NOT_ZERO)
    {
        return gcvTRUE;
    }
    else
    {
        return gcvFALSE;
    }
}

static gctBOOL
_source0_is_constant(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (gctOPT_hasFeature(FB_DISABLE_MERGE_CONST))
    {
        return gcvFALSE;
    }

    if (gcmSL_SOURCE_GET(Instruction->source0, Type) != gcSL_CONSTANT)
    {
        /* source0 is not a constant */
        return gcvFALSE;
    }
    if (!(gcmSL_TARGET_GET(Instruction->temp, Format) == gcSL_FLOAT ||
          gcmSL_TARGET_GET(Instruction->temp, Format) == gcSL_INTEGER ||
          gcmSL_TARGET_GET(Instruction->temp, Format) == gcSL_UINT32))
    {
        return gcvFALSE;
    }
    return gcvTRUE;
}

static gctBOOL
_source0_is_constant_dest_as_prev(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_INSTRUCTION prevInst = Instruction - 1;
    gcLINKTREE_TEMP curTemp  = Tree->tempArray + Instruction->tempIndex;
    gcLINKTREE_TEMP prevTemp = Tree->tempArray + prevInst->tempIndex;
    gctINT pc = (gctINT )(Instruction - Tree->shader->code);

    /* make sure the instruction has no label on it */
    if (Tree->hints[pc].callers != gcvNULL)
    {
        return gcvFALSE;
    }

    if (gcmSL_SOURCE_GET(Instruction->source0, Type) != gcSL_CONSTANT)
    {
        /* source0 is not a constant */
        return gcvFALSE;
    }
    if (!(gcmSL_TARGET_GET(Instruction->temp, Format) == gcSL_FLOAT ||
          gcmSL_TARGET_GET(Instruction->temp, Format) == gcSL_INTEGER ||
          gcmSL_TARGET_GET(Instruction->temp, Format) == gcSL_UINT32))
    {
        return gcvFALSE;
    }
    gcmASSERT(gcmSL_OPCODE_GET(prevInst->opcode, Opcode) ==
              gcmSL_OPCODE_GET(Instruction->opcode, Opcode));

    /* should not be indexed, precision condition should be the same */
    if (gcmSL_TARGET_GET(Instruction->temp, Precision) != gcmSL_TARGET_GET(prevInst->temp, Precision)
    ||  gcmSL_TARGET_GET(Instruction->temp, Condition) != gcmSL_TARGET_GET(prevInst->temp, Condition)
    ||  gcmSL_TARGET_GET(Instruction->temp, Indexed) != gcmSL_TARGET_GET(prevInst->temp, Indexed)
    ||  gcmSL_TARGET_GET(Instruction->temp, Indexed) != gcSL_NOT_INDEXED)
    {
        return gcvFALSE;
    }

    if (prevTemp->assigned != curTemp->assigned || prevTemp->index != curTemp->index)
    {
        /* not assigned yet or not assigned to same register */
        return gcvFALSE;
    }

    return gcvTRUE;
}

static gcSL_ENABLE
_shiftEnable(
    IN gcSL_ENABLE Enable,
    IN gctUINT     Shift
    )
{
    gctINT shiftedEnable = ((gctINT)Enable) << Shift;
    return (gcSL_ENABLE) shiftedEnable;
}

static gctBOOL
merge_4_constants(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_INSTRUCTION nextInst = Instruction + 1;
    gcSL_INSTRUCTION next2Inst = Instruction + 2;
    gcSL_INSTRUCTION next3Inst = Instruction + 3;
    gcSL_ENABLE curEnable;
    gcSL_ENABLE nextEnable;
    gcSL_ENABLE next2Enable;
    gcSL_ENABLE next3Enable;
    gcSL_ENABLE mergedEnable;
    gcLINKTREE_TEMP curTemp  = Tree->tempArray + Instruction->tempIndex;
    gcLINKTREE_TEMP nextTemp = Tree->tempArray + nextInst->tempIndex;
    gcLINKTREE_TEMP next2Temp = Tree->tempArray + next2Inst->tempIndex;
    gcLINKTREE_TEMP next3Temp = Tree->tempArray + next3Inst->tempIndex;
    gcSL_TYPE constType;

    gctINT   index   = 0;
    gctUINT8 swizzle = 0;
    gctUINT8 val0_swizzle, val1_swizzle, val2_swizzle, val3_swizzle;
    gctUINT8 compSwizzle[4];

    union
    {
        gctFLOAT f;
        gctUINT32 u;
    } value0, value1, value2, value3;

    gctUINT32 val0 = (Instruction->source0Index & 0xFFFF) | (Instruction->source0Indexed << 16);
    gctUINT32 val1 = (nextInst->source0Index & 0xFFFF) | (nextInst->source0Indexed << 16);
    gctUINT32 val2 = (next2Inst->source0Index & 0xFFFF) | (next2Inst->source0Indexed << 16);
    gctUINT32 val3 = (next3Inst->source0Index & 0xFFFF) | (next3Inst->source0Indexed << 16);


    gcmASSERT(gcmSL_OPCODE_GET(nextInst->opcode, Opcode) ==
                gcmSL_OPCODE_GET(Instruction->opcode, Opcode) &&
              gcmSL_OPCODE_GET(next2Inst->opcode, Opcode) ==
                gcmSL_OPCODE_GET(Instruction->opcode, Opcode) &&
              gcmSL_OPCODE_GET(next3Inst->opcode, Opcode) ==
                gcmSL_OPCODE_GET(Instruction->opcode, Opcode));
    gcmASSERT(gcmSL_SOURCE_GET(Instruction->source0, Type) == gcSL_CONSTANT);
    gcmASSERT(gcmSL_SOURCE_GET(nextInst->source0, Type) == gcSL_CONSTANT);
    gcmASSERT(gcmSL_SOURCE_GET(next2Inst->source0, Type) == gcSL_CONSTANT);
    gcmASSERT(gcmSL_SOURCE_GET(next3Inst->source0, Type) == gcSL_CONSTANT);
    gcmASSERT(curTemp->assigned == nextTemp->assigned &&
              curTemp->assigned == next2Temp->assigned &&
              curTemp->assigned == next3Temp->assigned);

    value0.u = val0;
    value1.u = val1;
    value2.u = val2;
    value3.u = val3;
    if (gcmSL_TARGET_GET(Instruction->temp, Format) == gcSL_FLOAT)
    {
        gcmVERIFY_OK(_AddConstantVec4(Tree,
                                        CodeGen,
                                        value0.f,
                                        value1.f,
                                        value2.f,
                                        value3.f,
                                        &index,
                                        &swizzle,
                                        &constType));
    }
    else if (gcmSL_TARGET_GET(Instruction->temp, Format) == gcSL_INTEGER ||
             gcmSL_TARGET_GET(Instruction->temp, Format) == gcSL_UINT32)
    {
        gcmVERIFY_OK(_AddConstantIVec4(Tree,
                                        CodeGen,
                                        value0.u,
                                        value1.u,
                                        value2.u,
                                        value3.u,
                                        &index,
                                        &swizzle,
                                        &constType));

    }
    else
    {
        return gcvFALSE;
    }
    val0_swizzle = _ExtractSwizzle(swizzle, 0);
    val1_swizzle = _ExtractSwizzle(swizzle, 1);
    val2_swizzle = _ExtractSwizzle(swizzle, 2);
    val3_swizzle = _ExtractSwizzle(swizzle, 3);

    /* get final machine code enable for each instruction (after shift) */
    curEnable = _shiftEnable(gcmSL_TARGET_GET(Instruction->temp, Enable), curTemp->shift);
    nextEnable = _shiftEnable(gcmSL_TARGET_GET(nextInst->temp, Enable), nextTemp->shift);
    next2Enable = _shiftEnable(gcmSL_TARGET_GET(next2Inst->temp, Enable), next2Temp->shift);
    next3Enable = _shiftEnable(gcmSL_TARGET_GET(next3Inst->temp, Enable), next3Temp->shift);
    mergedEnable = (gcSL_ENABLE)(curEnable | nextEnable | next2Enable | next3Enable);

    compSwizzle[0] = compSwizzle[1] = compSwizzle[2] = compSwizzle[3] = val0_swizzle;

    /* figure out the source swizzle and destination enable */
    {
        /* get swizzle based on nextInst and current instruciton's enable,
         * it may overwrite existing swizzle:
         *
         *   MOV  temp(1).zw, 0.1
         *   MOV  temp(1).yz, 2.0
         *   MOV  temp(1).w, 3.0
         *
         *   ==>
         *
         *   MOV temp(1).yzw, <0.1, 2.0, 3.0>.yyz
         */
        /* shift the enables if there is shift in the allocated register:
         *
         *    allocate temp(2).xyz ==> r3.yzw
         *
         *    MOV temp(2).xz, 0.2
         *    MOV temp(2).x, 1.0
         *
         *  ==>
         *    <0.2, 1.0> allocated to c2.zw
         *
         *    MOV temp(2.)xz, <0.2, 1.0>.wz
         *
         *    mov r3.yw, c2.wz
         *
         *  Another case is different temp registers are allocated to
         *  one register:
         *
         *    MOV temp(3).xy, 0.0    # temp(3).xy ==> r4.xy
         *    MOV temp(4).x, 1.0    # temp(4).x ==>  r4.z
         *    MOV temp.int(5).x, 1     # temp(5).x ==>  r4.w
         *
         *  ==>
         *    <0.0, 1.0, 1> allocated to c3.yzw
         *
         *    mov r4.xyzw, c3.yyzw
         *
         */
        /* next instruction */
        if (nextEnable & gcSL_ENABLE_X)
        {
            compSwizzle[0] = val1_swizzle;
        }
        if (nextEnable & gcSL_ENABLE_Y)
        {
            compSwizzle[1] = val1_swizzle;
        }
        if (nextEnable & gcSL_ENABLE_Z)
        {
            compSwizzle[2] = val1_swizzle;
        }
        if (nextEnable & gcSL_ENABLE_W)
        {
            compSwizzle[3] = val1_swizzle;
        }
        /* next 2 instruction */
        if (next2Enable & gcSL_ENABLE_X)
        {
            compSwizzle[0] = val2_swizzle;
        }
        if (next2Enable & gcSL_ENABLE_Y)
        {
            compSwizzle[1] = val2_swizzle;
        }
        if (next2Enable & gcSL_ENABLE_Z)
        {
            compSwizzle[2] = val2_swizzle;
        }
        if (next2Enable & gcSL_ENABLE_W)
        {
            compSwizzle[3] = val2_swizzle;
        }
        /* next 3 instruction */
        if (next3Enable & gcSL_ENABLE_X)
        {
            compSwizzle[0] = val3_swizzle;
        }
        if (next3Enable & gcSL_ENABLE_Y)
        {
            compSwizzle[1] = val3_swizzle;
        }
        if (next3Enable & gcSL_ENABLE_Z)
        {
            compSwizzle[2] = val3_swizzle;
        }
        if (next3Enable & gcSL_ENABLE_W)
        {
            compSwizzle[3] = val3_swizzle;
        }
    }

    swizzle = compSwizzle[0]
            | compSwizzle[1] << 2
            | compSwizzle[2] << 4
            | compSwizzle[3] << 6;

    States[0] =  ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (mergedEnable) & ((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    return gcvTRUE;
}

static gctBOOL
merge_3_constants(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_INSTRUCTION nextInst = Instruction + 1;
    gcSL_INSTRUCTION next2Inst = Instruction + 2;
    gcSL_ENABLE curEnable;
    gcSL_ENABLE nextEnable;
    gcSL_ENABLE next2Enable;
    gcSL_ENABLE mergedEnable;
    gcLINKTREE_TEMP curTemp  = Tree->tempArray + Instruction->tempIndex;
    gcLINKTREE_TEMP nextTemp = Tree->tempArray + nextInst->tempIndex;
    gcLINKTREE_TEMP next2Temp = Tree->tempArray + next2Inst->tempIndex;
    gcSL_TYPE constType;

    gctINT   index   = 0;
    gctUINT8 swizzle = 0;
    gctUINT8 val0_swizzle, val1_swizzle, val2_swizzle;
    gctUINT8 compSwizzle[4];

    union
    {
        gctFLOAT f;
        gctUINT32 u;
    } value0, value1, value2;

    gctUINT32 val0 = (Instruction->source0Index & 0xFFFF) | (Instruction->source0Indexed << 16);
    gctUINT32 val1 = (nextInst->source0Index & 0xFFFF) | (nextInst->source0Indexed << 16);
    gctUINT32 val2 = (next2Inst->source0Index & 0xFFFF) | (next2Inst->source0Indexed << 16);


    gcmASSERT(gcmSL_OPCODE_GET(nextInst->opcode, Opcode) ==
                gcmSL_OPCODE_GET(Instruction->opcode, Opcode) &&
              gcmSL_OPCODE_GET(next2Inst->opcode, Opcode) ==
                gcmSL_OPCODE_GET(Instruction->opcode, Opcode)   );
    gcmASSERT(gcmSL_SOURCE_GET(Instruction->source0, Type) == gcSL_CONSTANT);
    gcmASSERT(gcmSL_SOURCE_GET(nextInst->source0, Type) == gcSL_CONSTANT);
    gcmASSERT(gcmSL_SOURCE_GET(next2Inst->source0, Type) == gcSL_CONSTANT);
    gcmASSERT(curTemp->assigned == nextTemp->assigned &&
              curTemp->assigned == next2Temp->assigned   );

    value0.u = val0;
    value1.u = val1;
    value2.u = val2;
    if (gcmSL_TARGET_GET(Instruction->temp, Format) == gcSL_FLOAT)
    {
        gcmVERIFY_OK(_AddConstantVec3(Tree,
                                        CodeGen,
                                        value0.f,
                                        value1.f,
                                        value2.f,
                                        &index,
                                        &swizzle,
                                        &constType));
    }
    else if (gcmSL_TARGET_GET(Instruction->temp, Format) == gcSL_INTEGER ||
             gcmSL_TARGET_GET(Instruction->temp, Format) == gcSL_UINT32)
    {
        gcmVERIFY_OK(_AddConstantIVec3(Tree,
                                        CodeGen,
                                        value0.u,
                                        value1.u,
                                        value2.u,
                                        &index,
                                        &swizzle,
                                        &constType));

    }
    else
    {
        return gcvFALSE;
    }
    val0_swizzle = _ExtractSwizzle(swizzle, 0);
    val1_swizzle = _ExtractSwizzle(swizzle, 1);
    val2_swizzle = _ExtractSwizzle(swizzle, 2);

    /* get final machine code enable for each instruction (after shift) */
    curEnable = _shiftEnable(gcmSL_TARGET_GET(Instruction->temp, Enable), curTemp->shift);
    nextEnable = _shiftEnable(gcmSL_TARGET_GET(nextInst->temp, Enable), nextTemp->shift);
    next2Enable = _shiftEnable(gcmSL_TARGET_GET(next2Inst->temp, Enable), next2Temp->shift);
    mergedEnable = (gcSL_ENABLE)(curEnable | nextEnable | next2Enable);

    compSwizzle[0] = compSwizzle[1] = compSwizzle[2] = compSwizzle[3] = val0_swizzle;

    /* figure out the source swizzle and destination enable */
    {
        /* get swizzle based on nextInst and current instruciton's enable,
         * it may overwrite existing swizzle:
         *
         *   MOV  temp(1).zw, 0.1
         *   MOV  temp(1).yz, 2.0
         *   MOV  temp(1).w, 3.0
         *
         *   ==>
         *
         *   MOV temp(1).yzw, <0.1, 2.0, 3.0>.yyz
         */
        /* shift the enables if there is shift in the allocated register:
         *
         *    allocate temp(2).xyz ==> r3.yzw
         *
         *    MOV temp(2).xz, 0.2
         *    MOV temp(2).x, 1.0
         *
         *  ==>
         *    <0.2, 1.0> allocated to c2.zw
         *
         *    MOV temp(2.)xz, <0.2, 1.0>.wz
         *
         *    mov r3.yw, c2.wz
         *
         *  Another case is different temp registers are allocated to
         *  one register:
         *
         *    MOV temp(3).xy, 0.0    # temp(3).xy ==> r4.xy
         *    MOV temp(4).x, 1.0    # temp(4).x ==>  r4.z
         *    MOV temp.int(5).x, 1     # temp(5).x ==>  r4.w
         *
         *  ==>
         *    <0.0, 1.0, 1> allocated to c3.yzw
         *
         *    mov r4.xyzw, c3.yyzw
         *
         */
        /* next instruction */
        if (nextEnable & gcSL_ENABLE_X)
        {
            compSwizzle[0] = val1_swizzle;
        }
        if (nextEnable & gcSL_ENABLE_Y)
        {
            compSwizzle[1] = val1_swizzle;
        }
        if (nextEnable & gcSL_ENABLE_Z)
        {
            compSwizzle[2] = val1_swizzle;
        }
        if (nextEnable & gcSL_ENABLE_W)
        {
            compSwizzle[3] = val1_swizzle;
        }
        /* next 2 instruction */
        if (next2Enable & gcSL_ENABLE_X)
        {
            compSwizzle[0] = val2_swizzle;
        }
        if (next2Enable & gcSL_ENABLE_Y)
        {
            compSwizzle[1] = val2_swizzle;
        }
        if (next2Enable & gcSL_ENABLE_Z)
        {
            compSwizzle[2] = val2_swizzle;
        }
        if (next2Enable & gcSL_ENABLE_W)
        {
            compSwizzle[3] = val2_swizzle;
        }
    }

    swizzle = compSwizzle[0]
            | compSwizzle[1] << 2
            | compSwizzle[2] << 4
            | compSwizzle[3] << 6;

    States[0] =  ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (mergedEnable) & ((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    return gcvTRUE;
}

static gctBOOL
merge_2_constants(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_INSTRUCTION nextInst = Instruction + 1;
    gcSL_ENABLE curEnable  = (gcSL_ENABLE)gcmSL_TARGET_GET(Instruction->temp, Enable);
    gcSL_ENABLE nextEnable = (gcSL_ENABLE)gcmSL_TARGET_GET(nextInst->temp, Enable);
    gcSL_ENABLE mergedEnable;
    gcSL_TYPE constType;

    gcLINKTREE_TEMP curTemp  = Tree->tempArray + Instruction->tempIndex;
    gcLINKTREE_TEMP nextTemp = Tree->tempArray + nextInst->tempIndex;
    gctINT   index   = 0;
    gcSL_SWIZZLE swizzle;
    gcSL_SWIZZLE val0_swizzle, val1_swizzle;
    gctUINT8 compSwizzle[4];

    union
    {
        gctFLOAT f;
        gctUINT32 u;
    } value0, value1;
    gctUINT32 val0, val1;

    gcmASSERT(gcmSL_OPCODE_GET(nextInst->opcode, Opcode) ==
              gcmSL_OPCODE_GET(Instruction->opcode, Opcode));
    gcmASSERT(gcmSL_SOURCE_GET(Instruction->source0, Type) == gcSL_CONSTANT);
    gcmASSERT(curTemp->assigned == nextTemp->assigned);

    /* get the constant values from current and next instructions */
    val0 = (Instruction->source0Index & 0xFFFF) | (Instruction->source0Indexed << 16);
    val1 = (nextInst->source0Index & 0xFFFF) | (nextInst->source0Indexed << 16);
    value0.u = val0;
    value1.u = val1;

    /* create constant vector */
    if (gcmSL_TARGET_GET(Instruction->temp, Format) == gcSL_FLOAT)
    {
        gcmVERIFY_OK(_AddConstantVec2(Tree,
                                        CodeGen,
                                        value0.f,
                                        value1.f,
                                        &index,
                                        (gctUINT8_PTR)&swizzle,
                                        &constType));
    }
    else if (gcmSL_TARGET_GET(Instruction->temp, Format) == gcSL_INTEGER ||
             gcmSL_TARGET_GET(Instruction->temp, Format) == gcSL_UINT32)
    {
        gcmVERIFY_OK(_AddConstantIVec2(Tree,
                                        CodeGen,
                                        value0.u,
                                        value1.u,
                                        &index,
                                        (gctUINT8_PTR)&swizzle,
                                        &constType));

    }
    else
    {
        gcmASSERT(gcvFALSE);
        return gcvFALSE;
    }
    val0_swizzle = _ExtractSwizzle(swizzle, 0);
    val1_swizzle = _ExtractSwizzle(swizzle, 1);

    /* get final machine code enable for each instruction (after shift) */
    curEnable = _shiftEnable(gcmSL_TARGET_GET(Instruction->temp, Enable), curTemp->shift);
    nextEnable = _shiftEnable(gcmSL_TARGET_GET(nextInst->temp, Enable), nextTemp->shift);
    mergedEnable = (gcSL_ENABLE)(curEnable | nextEnable);

    compSwizzle[0] = compSwizzle[1] = compSwizzle[2] = compSwizzle[3] = val0_swizzle;


    /* figure out the source swizzle and destination enable */

    {

        /* get swizzle based on nextInst and current instruciton's enable,
         * it may overwrite existing swizzle:
         *
         *   MOV  temp(1).zw, 0.1
         *   MOV  temp(1).yz, 2.0
         *
         *   ==>
         *
         *   MOV temp(1).yzw, <0.1, 2.0>.yxy
         */
        if (nextEnable & gcSL_ENABLE_X)
        {
            compSwizzle[0] = val1_swizzle;
        }
        if (nextEnable & gcSL_ENABLE_Y)
        {
            compSwizzle[1] = val1_swizzle;
        }
        if (nextEnable & gcSL_ENABLE_Z)
        {
            compSwizzle[2] = val1_swizzle;
        }
        if (nextEnable & gcSL_ENABLE_W)
        {
            compSwizzle[3] = val1_swizzle;
        }
    }

    swizzle = compSwizzle[0]
            | compSwizzle[1] << 2
            | compSwizzle[2] << 4
            | compSwizzle[3] << 6;

    States[0] =  ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (mergedEnable) & ((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);
    return gcvTRUE;
}

static gctBOOL
_IsZ(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (gcmSL_TARGET_GET(Instruction->temp, Condition) == gcSL_ZERO)
    {
        return gcvTRUE;
    }
    else
    {
        return gcvFALSE;
    }
}

static gctBOOL
_ComparingWithZ(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_CONDITION condition = gcmSL_TARGET_GET(Instruction->temp, Condition);
    switch (condition)
    {
        case gcSL_ZERO:
        case gcSL_NOT_ZERO:
        case gcSL_LESS_ZERO:
        case gcSL_LESS_OREQUAL_ZERO:
        case gcSL_GREATER_ZERO:
        case gcSL_GREATER_OR_EQUAL_ZERO:
            return gcvTRUE;
        default:
            return gcvFALSE;
    }
}

static gctBOOL
_MSBCompare(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_CONDITION condition = gcmSL_TARGET_GET(Instruction->temp, Condition);
    switch (condition)
    {
        case gcSL_ALLMSB:
        case gcSL_ANYMSB:
        case gcSL_SELMSB:
            return gcvTRUE;
        default:
            return gcvFALSE;
    }
}

static gctBOOL
_ConditionSameAsPrev(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_CONDITION condition = gcmSL_TARGET_GET(Instruction->temp, Condition);
    gcSL_INSTRUCTION prev = Instruction - 1;
    gcSL_CONDITION prevCondition = gcmSL_TARGET_GET(prev->temp, Condition);
    if(condition == prevCondition)
    {
        return gcvTRUE;
    }
    return gcvFALSE;
}

extern gctBOOL
isConditionReversible(
    IN  gcSL_CONDITION   Condition,
    OUT gcSL_CONDITION * ReversedCondition
    );

static gctBOOL
_ConditionReversedWithPrev(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_CONDITION condition = gcmSL_TARGET_GET(Instruction->temp, Condition);
    gcSL_INSTRUCTION prev = Instruction - 1;
    gcSL_CONDITION prevCondition = gcmSL_TARGET_GET(prev->temp, Condition);
    gcSL_CONDITION reversed = gcSL_ALWAYS;
    if(isConditionReversible(condition, &reversed))
    {
        if(reversed == prevCondition)
        {
            return gcvTRUE;
        }
    }
    return gcvFALSE;
}

#if _USE_CONV_FOR_EXPLICIT_CONVERT_FUNCTION

#define IS_ROUND_MODE(Opcode, RoundMode) \
    (gcmSL_OPCODE_GET((Opcode), Round) == (RoundMode))

static gctBOOL
_isI2F(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if ((gcmSL_TARGET_GET(Instruction->temp, Format) == gcSL_FLOAT)
    &&  (gcmSL_SOURCE_GET(Instruction->source0, Format) != gcSL_FLOAT))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_isI2F_Rounding_mode(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (_isI2F(Tree, CodeGen, Instruction, States)
    &&  _hasRounding_mode(Tree, CodeGen, Instruction, States))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_isF2I(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if ((gcmSL_TARGET_GET(Instruction->temp, Format) != gcSL_FLOAT)
    &&  (gcmSL_TARGET_GET(Instruction->temp, Format) != gcSL_FLOAT16)
    &&  (gcmSL_TARGET_GET(Instruction->temp, Format) != gcSL_FLOAT64)
    &&  (gcmSL_SOURCE_GET(Instruction->source0, Format) == gcSL_FLOAT)
       )
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_isF2I_Sat(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (IS_SATURATE(Instruction->opcode) && _isF2I(Tree, CodeGen, Instruction, States))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

DEFINE_WITH_VIR(_isF2I_Sat)

static gctBOOL
_isF2I_Sat_Rtp(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (IS_SATURATE(Instruction->opcode) && IS_ROUND_MODE(Instruction->opcode, gcSL_ROUND_RTP)
    && _isF2I(Tree, CodeGen, Instruction, States))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

DEFINE_WITH_VIR(_isF2I_Sat_Rtp)

static gctBOOL
_isF2I_Sat_Rtn(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (IS_SATURATE(Instruction->opcode) && IS_ROUND_MODE(Instruction->opcode, gcSL_ROUND_RTN)
    && _isF2I(Tree, CodeGen, Instruction, States))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

DEFINE_WITH_VIR(_isF2I_Sat_Rtn)

static gctBOOL
_isF2I_Rtp(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (IS_ROUND_MODE(Instruction->opcode, gcSL_ROUND_RTP) && _isF2I(Tree, CodeGen, Instruction, States))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

DEFINE_WITH_VIR(_isF2I_Rtp)

static gctBOOL
_isF2I_Rtn(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (IS_ROUND_MODE(Instruction->opcode, gcSL_ROUND_RTN) && _isF2I(Tree, CodeGen, Instruction, States))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

DEFINE_WITH_VIR(_isF2I_Rtn)

static gctBOOL
_isI2I_Sat_s2us(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT bits, bits0;
    gcSL_FORMAT format, format0;

    if (!IS_SATURATE(Instruction->opcode))
    {
        return gcvFALSE;
    }

    format = (gcSL_FORMAT)gcmSL_TARGET_GET(Instruction->temp, Format);
    format0 = (gcSL_FORMAT)(Instruction->source1Index |
                            Instruction->source1Indexed << 16);

    if ((gcmSL_SOURCE_GET(Instruction->source1, Type) != gcSL_CONSTANT)
    || (gcmSL_SOURCE_GET(Instruction->source1, Format) != gcSL_UINT32))
    {
        return gcvFALSE;
    }

    if (format0 == format)
    {
        return gcvFALSE;
    }

    switch (format)
    {
    case gcSL_INT8:
    case gcSL_UINT8:
        bits = 8;
        break;

    case gcSL_INT16:
    case gcSL_UINT16:
        bits = 16;
        break;

    default:
        return gcvFALSE;
    }

    switch (format0)
    {
    case gcSL_INT8:
        bits0 = 8;
        break;

    case gcSL_INT16:
        bits0 = 16;
        break;

    case gcSL_INT32:
        bits0 = 32;
        break;

    default:
        return gcvFALSE;
    }

    if (bits0 > bits)
    {
        return gcvTRUE;
    }
    else
    {
        return gcvFALSE;
    }
}

DEFINE_WITH_VIR(_isI2I_Sat_s2us)

static gctBOOL
_isI2I_Sat_u2us(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT bits, bits0;
    gcSL_FORMAT format, format0;

    if (!IS_SATURATE(Instruction->opcode))
    {
        return gcvFALSE;
    }

    format = (gcSL_FORMAT)gcmSL_TARGET_GET(Instruction->temp, Format);
    format0 = (gcSL_FORMAT)(Instruction->source1Index |
                            Instruction->source1Indexed << 16);

    if ((gcmSL_SOURCE_GET(Instruction->source1, Type) != gcSL_CONSTANT)
    || (gcmSL_SOURCE_GET(Instruction->source1, Format) != gcSL_UINT32))
    {
        return gcvFALSE;
    }

    if (format0 == format)
    {
        return gcvFALSE;
    }

    switch (format)
    {
    case gcSL_INT8:
    case gcSL_UINT8:
        bits = 8;
        break;

    case gcSL_INT16:
    case gcSL_UINT16:
        bits = 16;
        break;

    case gcSL_INT32:
    case gcSL_UINT32:
        bits = 32;
        break;

    default:
        return gcvFALSE;
    }

    switch (format0)
    {
    case gcSL_UINT8:
        bits0 = 8;
        break;

    case gcSL_UINT16:
        bits0 = 16;
        break;

    case gcSL_UINT32:
        bits0 = 32;
        break;

    default:
        return gcvFALSE;
    }

    if (bits0 >= bits)
    {
        return gcvTRUE;
    }
    else
    {
        return gcvFALSE;
    }
}

DEFINE_WITH_VIR(_isI2I_Sat_u2us);

static gctBOOL
_isI2I_Sat_s2u(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctUINT bits, bits0;
    gcSL_FORMAT format, format0;

    if (!IS_SATURATE(Instruction->opcode))
    {
        return gcvFALSE;
    }

    format = (gcSL_FORMAT)gcmSL_TARGET_GET(Instruction->temp, Format);
    format0 = (gcSL_FORMAT)(Instruction->source1Index |
                            Instruction->source1Indexed << 16);

    if ((gcmSL_SOURCE_GET(Instruction->source1, Type) != gcSL_CONSTANT)
    || (gcmSL_SOURCE_GET(Instruction->source1, Format) != gcSL_UINT32))
    {
        return gcvFALSE;
    }

    if (format0 == format)
    {
        return gcvFALSE;
    }

    switch (format)
    {
    case gcSL_UINT8:
        bits = 8;
        break;

    case gcSL_UINT16:
        bits = 16;
        break;

    case gcSL_UINT32:
        bits = 32;
        break;

    default:
        return gcvFALSE;
    }

    switch (format0)
    {
    case gcSL_INT8:
        bits0 = 8;
        break;

    case gcSL_INT16:
        bits0 = 16;
        break;

    case gcSL_INT32:
        bits0 = 32;
        break;

    default:
        return gcvFALSE;
    }

    if (bits0 <= bits)
    {
        return gcvTRUE;
    }
    else
    {
        return gcvFALSE;
    }
}

DEFINE_WITH_VIR(_isI2I_Sat_s2u);
#endif

static gctBOOL
_SatAbs0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    /* Enable saturation. */
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ? 11:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));

    /* Set source 0 to absolute modifier. */
    gcSetSrcABS(States, 0);
    return gcvTRUE;
}

static gctBOOL
_Sat0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    /* Enable saturation. */
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ? 11:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));
    return gcvTRUE;
}

static gctBOOL
_Neg2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) ((gctUINT32) (gcmSL_SOURCE_GET((Instruction + 1)->source0, Neg) ?
 0 : 1) & ((gctUINT32) ((((1 ? 22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 22:22) - (0 ? 22:22) + 1))))))) << (0 ? 22:22)));
    return gcvTRUE;
}

static gctBOOL
_Neg0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ? 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) ((gctUINT32) (gcmSL_SOURCE_GET((Instruction + 1)->source0, Neg) ?
 0 : 1) & ((gctUINT32) ((((1 ? 30:30) - (0 ? 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)));
    return gcvTRUE;
}

static gctBOOL
set_mova(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_ENABLE  enable  = (gcSL_ENABLE) gcmSL_TARGET_GET(Instruction->temp, Enable);

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable) & ((gctUINT32) ((((1 ? 26:23) - (0 ?
 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));

    return gcvTRUE;
}

/* return true if Instruction dest is float type */
gctBOOL
_isDstFloat(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (gcmSL_TARGET_GET(Instruction->temp, Format) == gcSL_FLOAT ||
        gcmSL_TARGET_GET(Instruction->temp, Format) == gcSL_FLOAT16)
    {
        return gcvTRUE;
    }
    return gcvFALSE;
}

static gctBOOL
set_extended_opcode_lodq(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcsConstantValue constValue;

    constValue.ty = gcSL_UINT32;
    constValue.value.u = 0x04;
    gcEncodeSourceImmediate20(States, 2, &constValue);

    return gcvTRUE;
}

static gctBOOL
set_extended_opcode_flush(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcsConstantValue constValue;

    constValue.ty = gcSL_UINT32;
    constValue.value.u = 0x03;
    gcEncodeSourceImmediate20(States, 2, &constValue);

    return gcvTRUE;
}

static gctBOOL
set_extended_opcode_findlsb_src0_type(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcsConstantValue constValue;
    gctUINT format = gcmSL_SOURCE_GET(Instruction->source0, Format);
    gctUINT value_type0 = type_conv[format];
    gctUINT inst_type0 = value_type0 & 0x1;
    gctUINT inst_type1 = value_type0 >> 1;

    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) ((gctUINT32) (inst_type0) & ((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21)));
    States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (inst_type1) & ((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30)));

    constValue.ty = gcSL_UINT32;
    constValue.value.u = 0x0B;
    gcEncodeSourceImmediate20(States, 2, &constValue);

    return gcvTRUE;
}

static gctBOOL
set_extended_opcode_findmsb_src0_type(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcsConstantValue constValue;
    gctUINT format = gcmSL_SOURCE_GET(Instruction->source0, Format);
    gctUINT value_type0 = type_conv[format];
    gctUINT inst_type0 = value_type0 & 0x1;
    gctUINT inst_type1 = value_type0 >> 1;

    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) ((gctUINT32) (inst_type0) & ((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21)));
    States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (inst_type1) & ((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30)));

    constValue.ty = gcSL_UINT32;
    constValue.value.u = 0x0C;
    gcEncodeSourceImmediate20(States, 2, &constValue);

    return gcvTRUE;
}

static gctBOOL
set_extended_opcode_fetchms(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcsConstantValue constValue;

    constValue.ty = gcSL_UINT32;
    constValue.value.u = 0x0D;
    gcEncodeSourceImmediate20(States, 2, &constValue);

    return gcvTRUE;
}

static gctBOOL
_isImage2D(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    if (gcmSL_SOURCE_GET(Instruction->source0, Type) == gcSL_UNIFORM)
    {
        gcUNIFORM uniform = Tree->shader->uniforms[gcmSL_INDEX_GET(Instruction->source0Index, Index)];

        if (isUniformImage2D(uniform))
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

static gctBOOL
_hasHalti4_image2D(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    return _hasHalti4(Tree, CodeGen, Instruction, States) && _isImage2D(Tree, CodeGen, Instruction, States);
}

static gctBOOL
setAllComponentsEnable(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (gcSL_ENABLE_XYZW) & ((gctUINT32) ((((1 ?
 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ?
 26:23)));
    return gcvTRUE;
}

/* 0x00 gcSL_NOP */
const gcsSL_PATTERN patterns_NOP[] =
{
    { 0 }
};

/* 0x01 gcSL_MOV */
const gcsSL_PATTERN patterns_MOV[] =
{
    /*
        MOV     1, 2
        MOV     3, 4
        MOV     5, 6
        MOV     7, 8
            mov 5, 0, 0, 2, 0
    */
    { 4, gcSL_MOV, 1, 2, 0, 0, 0, _source0_is_constant },
    { 3, gcSL_MOV, 3, 4, 0, 0, 0, _source0_is_constant_dest_as_prev },
    { 2, gcSL_MOV, 5, 6, 0, 0, 0, _source0_is_constant_dest_as_prev },
    { 1, gcSL_MOV, 7, 8, 0, 0, 0, _source0_is_constant_dest_as_prev },
        { -1, 0x09, 7, 0, 0, gcSL_CG_CONSTANT, 0, merge_4_constants },

    /*
        MOV     1, 2
        MOV     3, 4
        MOV     5, 6
            mov 5, 0, 0, 2, 0
    */
    { 3, gcSL_MOV, 1, 2, 0, 0, 0, _source0_is_constant },
    { 2, gcSL_MOV, 3, 4, 0, 0, 0, _source0_is_constant_dest_as_prev },
    { 1, gcSL_MOV, 5, 6, 0, 0, 0, _source0_is_constant_dest_as_prev },
        { -1, 0x09, 5, 0, 0, gcSL_CG_CONSTANT, 0, merge_3_constants },

    /*
        MOV     1, 2
        MOV     3, 4
            mov 3, 0, 0, 2, 0
    */
    { 2, gcSL_MOV, 1, 2, 0, 0, 0, _source0_is_constant },
    { 1, gcSL_MOV, 3, 4, 0, 0, 0, _source0_is_constant_dest_as_prev },
        { -1, 0x09, 3, 0, 0, gcSL_CG_CONSTANT, 0, merge_2_constants },

    /*
        MOV     1, 2
        TEXBIAS 0, 3, 1
        TEXLD   4, 3, 5
            mov 5.w, 0, 0, 2, 0
            texldb 4, 5, 0, 0, 3
    */
    { 3, gcSL_MOV, 1, 2, 0, 0, 0, _UseDestInNextTwoOnly }, /* TEXLD has a special dependcy on this MOV */
    { 2, gcSL_TEXBIAS, 0, 3, 1 },
    { 1, gcSL_TEXLD, 4, 3, 5, 0, 0, _HandleBiasedTextureLoad },
        { -2, 0x09, 5, 0, 0, 2, 0, enable_w },
        { -1, 0x19, 4, 5, 0, 0, 3, source0_add_swizzle_w },

    /*
        MOV     4, 5
        TEXBIAS 0, 2, 4
        TEXLD   1, 2, 3
            mov TEMP1_XYZW, 0, 0, 3, 0
            mov TEMP1_XYZW.w, 0, 0, 5, 0
            texldb 1, TEMP1_XYZW, 0, 0, 2
    */
    { 3, gcSL_MOV, 4, 5, 0, 0, 0, _UseDestInNextTwoOnly }, /* TEXLD has a special dependcy on this MOV */
    { 2, gcSL_TEXBIAS, 0, 2, 4 },
    { 1, gcSL_TEXLD, 1, 2, 3 },
        { -3, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0 },
        { -2, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 5, 0, enable_w },
        { -1, 0x19, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2 },

    /*
        MOV 1, 2
        IMAGE_ADDR    3, 1, 4
            img_addr   3, 2, 4
    */
    { 2, gcSL_MOV, 1, 2, 0, 0, 0, _hasHalti4_image2D },
    { 1, gcSL_IMAGE_ADDR, 3, 1, 4, 0, 0, 0 },
        { -1, 0x37, 3, 2, 4, 0, 0, value_types_u32},

    /*
        IMAGE_ADDR    1, 2, 3
            img_addr_3d   1, 2, 3
    */
    { 2, gcSL_MOV, 1, 2, 0, 0, 0, _hasHalti4 },
    { 1, gcSL_IMAGE_ADDR, 3, 1, 4, 0, 0, 0 },
        { -1, 0x38, 3, 2, 4, 0, 0, value_types_u32},
#if _SUPPORT_LONG_ULONG_DATA_TYPE

    /*
        MOV 1, 2
            mov 1, 0, 0, 2, 0
    */
    { 1, gcSL_MOV, 1, 2, 0, 0, 0, _hasInteger_long_ulong},
        { -2, 0x09, 1, 0, 0, 2, 0, long_ulong_first_mov },
        { -1, 0x09, 1, 0, 0, 2, 0, long_ulong_second_mov },
#endif
    /*
        MOV 1, 2
            mov 1, 0, 0, 2, 0
    */
    { 1, gcSL_MOV, 1, 2 },
        { -1, 0x09, 1, 0, 0, 2, 0, mov },

    { 0 }
};

/* 0x02 gcSL_SAT */
const gcsSL_PATTERN patterns_SAT[] =
{
    /*
        SAT 1, 2
            mov.sat 1, 2, 0, 0, 0
    */
    { 1, gcSL_SAT, 1, 2 },
        { -1, 0x09, 1, 0, 0, 2, 0, saturate },

    { 0 }
};

/* 0x03 gcSL_DP3 */
const gcsSL_PATTERN patterns_DP3[] =
{
    /*
        DP3 1, 2, 3
            dp3 1, 2, 3, 0, 0
            nop
    */
    { 1, gcSL_DP3, 1, 2, 3, 0, 0, _InsertNop },
        { -2, 0x05, 1, 2, 3, 0, 0 },
        { -1, 0x00, 0, 0, 0, 0, 0 },

    /*
        DP3 1, 2, 3
        SAT 4, 1
            dp3.sat 4, 2, 3, 0, 0
    */
    { 2, gcSL_DP3, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_SAT, 4, 1 },
        { -1, 0x05, 4, 2, 3, 0, 0, _Sat0 },

#if (GC_ENABLE_DUAL_FP16 > 0)
    /*
        DP3  1, 2, 3
        SQRT 4, 1
            dp3.t0 1.hp, 2, 3, 0, 0
            sqrt.t0 4, 0, 0, 1.hp
            dp3.t1 1.hp, 2, 3, 0, 0
            sqrt.t1 4, 0, 0, 1.hp
    */
    { 2, gcSL_DP3, 1, 2, 3, 0, 0, _UseDestInNextOnly_Dual16OnMediumpSrc0Src1 },
    { 1, gcSL_SQRT, 4, 1 },
        { -4, 0x05, gcSL_CG_TEMP1, 2, 3, 0, 0, _t0_destHP },
        { -3, 0x21, 4, 0, 0, gcSL_CG_TEMP1, 0, _t0_src2HP },
        { -2, 0x05, gcSL_CG_TEMP1, 2, 3, 0, 0, _t1_destHP },
        { -1, 0x21, 4, 0, 0, gcSL_CG_TEMP1, 0, _t1_src2HP },

    /*
        DP3 1, 2, 3
            dp3.t0 1, 2, 3.hp, 0, 0
            dp3.t1 1, 2, 3.hp, 0, 0
    */
    { 1, gcSL_DP3, 1, 2, 3, 0, 0, _Dual16OnMediumpDstMediumpSrc0HighpSrc1},
        { -2, 0x05, 1, 2, 3, 0, 0, _t0_destMP},
        { -1, 0x05, 1, 2, 3, 0, 0, _t1_destMP},
#endif

    /*
        DP3 1, 2, 3
            dp3 1, 2, 3, 0, 0
    */
    { 1, gcSL_DP3, 1, 2, 3, 0, 0, _GFX27Patch },
        { -1, 0x05, 1, 2, 3, 0, 0, rtz },

    /*
        DP3 1, 2, 3
            dp3 1, 2, 3, 0, 0
    */
    { 1, gcSL_DP3, 1, 2, 3 },
        { -1, 0x05, 1, 2, 3, 0, 0, rtne },

    { 0 }
};

/* 0x04 gcSL_DP4 */
const gcsSL_PATTERN patterns_DP4[] =
{
    /*
        DP4 1, 2, 3
            dp4 1, 2, 3, 0, 0
            nop
    */
    { 1, gcSL_DP4, 1, 2, 3, 0, 0, _InsertNop },
        { -2, 0x06, 1, 2, 3, 0, 0 },
        { -1, 0x00, 0, 0, 0, 0, 0 },

    /*
        DP4 1, 2, 3
        SAT 4, 1
            dp4.sat 4, 2, 3, 0, 0
    */
    { 2, gcSL_DP4, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_SAT, 4, 1 },
        { -1, 0x06, 4, 2, 3, 0, 0, _Sat0 },

#if (GC_ENABLE_DUAL_FP16 > 0)
    /*
        DP4  1, 2, 3
        SQRT 4, 1
            dp3.t0 1.hp, 2, 3, 0, 0
            sqrt.t0 4, 0, 0, 1.hp
            dp3.t1 1.hp, 2, 3, 0, 0
            sqrt.t1 4, 0, 0, 1.hp
    */
    { 2, gcSL_DP4, 1, 2, 3, 0, 0, _UseDestInNextOnly_Dual16OnMediumpSrc0Src1 },
    { 1, gcSL_SQRT, 4, 1 },
        { -4, 0x06, gcSL_CG_TEMP1, 2, 3, 0, 0, _t0_destHP },
        { -3, 0x21, 4, 0, 0, gcSL_CG_TEMP1, 0, _t0_src2HP },
        { -2, 0x06, gcSL_CG_TEMP1, 2, 3, 0, 0, _t1_destHP },
        { -1, 0x21, 4, 0, 0, gcSL_CG_TEMP1, 0, _t1_src2HP },
#endif
    /*
        DP4 1, 2, 3
            dp4 1, 2, 3, 0, 0
    */
    { 1, gcSL_DP4, 1, 2, 3 },
        { -1, 0x06, 1, 2, 3, 0, 0, rtne },

    { 0 }
};

/* 0x05 gcSL_ABS */
const gcsSL_PATTERN patterns_ABS[] =
{
    /*
        This is a special pattern generated by the ES 1.1 driver for fog.

        ABS 1, 2
        MUL 3, 1, 4
        ADD 5, 3, 6
        SAT 7, 5
            mad.sat 7, |2|, 4, 6
    */
    { 4, gcSL_ABS, 1, 2, 0, 0, 0, _UseDestInNextOnly },
    { 3, gcSL_MUL, 3, 1, 4, 0, 0, _UseDestInNextOnly },
    { 2, gcSL_ADD, 5, 3, 6, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_SAT, 7, 5 },
        { -1, 0x02, 7, 2, 4, 6, 0, _SatAbs0 },

    /*
        ABS 1, 2
        MUL 3, 1, 4
        SAT 5, 3
            mul.sat 5, |2|, 4
    */
    { 3, gcSL_ABS, 1, 2, 0, 0, 0, _UseDestInNextOnly },
    { 2, gcSL_MUL, 3, 1, 4, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_SAT, 5, 3 },
        { -1, 0x03, 5, 2, 4, 0, 0, _SatAbs0 },

    /*
        ABS 1, 2
        MUL 3, 4, 1
        SAT 5, 3
            mul.sat 5, |2|, 4
    */
    { 3, gcSL_ABS, 1, 2, 0, 0, 0, _UseDestInNextOnly },
    { 2, gcSL_MUL, 3, 1, 4, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_SAT, 5, 3 },
        { -1, 0x03, 5, 4, 2, 0, 0, _SatAbs0 },

    /*
        ABS 1, 2
            select.lt 1, 2, -2, 2, 0
    */
    { 1, gcSL_ABS, 1, 2, 0, 0, 0, _IntOpcodeAndHasIABS },
        { -1, 0x57, 1, 0, 0, 2, 0, value_type0_from_src0 },

    /*
        ABS 1, 2
            select.lt 1, 2, -2, 2, 0
    */
    { 1, gcSL_ABS, 1, 2, 0, 0, 0, _IntOpcode },
        { -2, 0x01, gcSL_CG_TEMP1, gcSL_CG_CONSTANT, 0, -2, 0, int_value_type0_const_0 },
        { -1, 0x0F, 1, 2, gcSL_CG_TEMP1, 2, 0, conditionLT },

    /*
        ABS 1, 2
        MIN 3, 1, 4
            select.gt 3, 4, |2|, 4, 0
    */
    { 2, gcSL_ABS, 1, 2, 0, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_MIN, 3, 1, 4, 0, 0, _NoLabel },
        { -1, 0x0F, 3, 4, 2, 4, 0, conditionGTAbs1 },

    /*
        ABS 1, 2
        MIN 3, 4, 1
            select.gt 3, 4, |2|, 4, 0
    */
    { 2, gcSL_ABS, 1, 2, 0, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_MIN, 3, 4, 1, 0, 0, _NoLabel },
        { -1, 0x0F, 3, 4, 2, 4, 0, conditionGTAbs1 },

    /*
        ABS 1, 2
        MAX 3, 1, 4
            select.lt 3, 4, |2|, 4, 0
    */
    { 2, gcSL_ABS, 1, 2, 0, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_MAX, 3, 1, 4, 0, 0, _NoLabel },
        { -1, 0x0F, 3, 4, 2, 4, 0, conditionLTAbs1 },

    /*
        ABS 1, 2
        MAX 3, 4, 1
            select.lt 3, 4, |2|, 4, 0
    */
    { 2, gcSL_ABS, 1, 2, 0, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_MAX, 3, 4, 1, 0, 0, _NoLabel },
        { -1, 0x0F, 3, 4, 2, 4, 0, conditionLTAbs1 },

    /*
        ABS 1, 2
            select.lt 1, 2, -2, 2, 0
    */
    { 1, gcSL_ABS, 1, 2, 0, 0, 0, _IntOpcode },
        { -1, 0x0F, 1, 2, -2, 2, 0, conditionLT },

    /*
        ABS 1, 2
            select.lt 1, 2, -2, 2, 0
    */
    { 1, gcSL_ABS, 1, 2 },
        /*{ -1, 0x0F, 1, 2, -2, 2, 0, conditionLT },*/
        /*new chip's select doesn't flush denorm to zero, add zero to flush to zero*/
        { -1, 0x01, 1, 2, 0, gcSL_CG_CONSTANT, 0, abs_0_zero_2 },

    { 0 }
};

/* 0x06 gcSL_JMP */
const gcsSL_PATTERN patterns_JMP[] =
{
    /*
        JMP.cond 1, 2, 3
        MOV      4, ONE
        JMP      6
        MOV      4, ZERO
            set.reverse_cond   4, 2, 3
    */
    { 4, gcSL_JMP, 1, 2, 3, 0, 0, _jmpToNextPlusTwoAndFloatOperand_reversibleCondition },
    { 3, gcSL_MOV, 4, 5, 0, 0, 0, _Const1_NoLabel },
    { 2, gcSL_JMP, 6, 0, 0, 0, 0, _jmpToNextPlusOne },
    { 1, gcSL_MOV, 4, 7, 0 ,0, 0, _Const0_HasOneLabel },
        { -1, 0x10, 4, 2, 3, 0, 0, reverseConditionAndDeleteSecondCaller },

    /*
        JMP.cond 1, 2, 3
        MOV      4, ZERO
        JMP      6
        MOV      4, ONE
            set.cond   4, 2, 3
    */
    { 4, gcSL_JMP, 1, 2, 3, 0, 0, _jmpToNextPlusTwoAndFloatOperand },
    { 3, gcSL_MOV, 4, 5, 0, 0, 0, _Const0_NoLabel },
    { 2, gcSL_JMP, 6, 0, 0, 0, 0, _jmpToNextPlusOne },
    { 1, gcSL_MOV, 4, 7, 0 ,0, 0, _Const1_HasOneLabel },
        { -1, 0x10, 4, 2, 3, 0, 0, copyConditionAndDeleteSecondCaller },

    /*
        JMP.cond 1, 2, 3
        MOV      4, 5
        JMP      6
        MOV      4, 7
            select.cond  4, 2, 7, 5, 0
    */
    { 4, gcSL_JMP, 1, 2, 3, 0, 0, _jmpToNextPlusTwo_OneOperandCmp_0 },
    { 3, gcSL_MOV, 4, 5, 0, 0, 0, _NoLabel_CanUseSelectCmpSet },
    { 2, gcSL_JMP, 6, 0, 0, 0, 0, _jmpToNextPlusOne },
    { 1, gcSL_MOV, 4, 7, 0 ,0, 0, _HasOneLabel },
        { -1, 0x0F, 4, 2, 7, 5, 0, oneOperandConditionAndDeleteSecondCaller },

    /*
        JMP.cond 1, 2, 3
        MOV      4, 5
        JMP      6
        MOV      4, 7
            select.cond  4, 3, 7, 5, 0
    */
    { 4, gcSL_JMP, 1, 2, 3, 0, 0, _jmpToNextPlusTwo_OneOperandCmp_1 },
    { 3, gcSL_MOV, 4, 5, 0, 0, 0, _NoLabel_CanUseSelectCmpSet },
    { 2, gcSL_JMP, 6, 0, 0, 0, 0, _jmpToNextPlusOne },
    { 1, gcSL_MOV, 4, 7, 0 ,0, 0, _HasOneLabel },
        { -1, 0x0F, 4, 3, 7, 5, 0, oneOperandConditionAndDeleteSecondCaller },

    /*
        JMP.cond 1, 2, 3   // float type
        MOV      4, 5
        JMP      6
        MOV      4, 7
            set.cond   temp 2, 3
            select.NZ  4, temp, 7, 5, 0
    */
    { 4, gcSL_JMP, 1, 2, 3, 0, 0, _jmpToNextPlusTwoAndFloatOperand },
    { 3, gcSL_MOV, 4, 5, 0, 0, 0, _NoLabel_CanUseSelectCmpSet },
    { 2, gcSL_JMP, 6, 0, 0, 0, 0, _jmpToNextPlusOne },
    { 1, gcSL_MOV, 4, 7, 0 ,0, 0, _HasOneLabel },
        { -2, 0x10, gcSL_CG_TEMP1, 2, 3, 0, 0, value_type0_32bit_from_src0_and_delete_second_caller },
        { -1, 0x0F, 4, gcSL_CG_TEMP1, 7, 5, 0, conditionNZ_from_next_inst },

    /*
        JMP.cond 1, 2, 3
        MOV      4, 5
        JMP      6
        MOV      4, 7
            set.cond   temp 2, 3
            select.NZ  4, temp, 7, 5, 0
    */
    { 4, gcSL_JMP, 1, 2, 3, 0, 0, _jmpToNextPlusTwo_hasCMP_NoFloatOperand },
    { 3, gcSL_MOV, 4, 5, 0, 0, 0, _NoLabel_CanUseSelectCmpSet },
    { 2, gcSL_JMP, 6, 0, 0, 0, 0, _jmpToNextPlusOne },
    { 1, gcSL_MOV, 4, 7, 0 ,0, 0, _HasOneLabel },
        { -2, 0x31, gcSL_CG_TEMP1, 2, 3, gcSL_CG_CONSTANT, 0, one_2_value_type0_from_src0_and_delete_second_caller },
        { -1, 0x0F, 4, gcSL_CG_TEMP1, 7, 5, 0, conditionNZ_from_next_inst },

    /*
        JMP.cond 1, 2, 3
        MOV 2, 4
            select.reverse_cond 2, 3, 2, 4
    */
    { 2, gcSL_JMP, 1, 2, 3, 0, 0, _jmpToNextPlusOneWithSameSourceAndFloatOperand},
    { 1, gcSL_MOV, 2, 4, 0, 0, 0 },
        { -1, 0x0F, 2, 3, 2, 4, 0, reverseConditionAndReverseEqualAndDeleteCall},

    /*
        JMP.cond 1, 2, 3
        TEXKILL
            texkill.cond 2, 3
    */
    { 2, gcSL_JMP, 1, 2, 3, 0, 0, _hasFloatCompare_jmpToNextPlusOne_halti4 },
    { 1, gcSL_KILL, 4, 0, 0, 0, 0, _NoLabelAndConditionAlways },
        { -1, 0x17, 4, 2, 3, 0, 0, reverseConditionAndDeleteFirstCaller },

    /*
        JMP.cond 1, 2, 3
        TEXKILL
        JMP
            texkill.cond 2, 3
    */
    { 3, gcSL_JMP, 1, 2, 3, 0, 0, _hasFloatCompare_jmpToNextPlusTwo_halti4 },
    { 2, gcSL_KILL, 4, 0, 0, 0, 0, _NoLabelAndConditionAlways },
    { 1, gcSL_JMP, 5, 0, 0, 0, 0, _NoLabelAndConditionAlways },
        { -1, 0x17, 4, 2, 3, 0, 0, reverseConditionAndDeleteFirstCaller },

#if _SUPPORT_LONG_ULONG_DATA_TYPE
    /*
        JMP.cond 1, 2, 3
            branch.cond 0, 1, 2, 0, 0
    */
    { 1, gcSL_JMP, 0, 1, 2, 0, 0, _hasInteger_long_ulong },
        { -1, 0x16, 0, 1, 2, 0, 0, branchAndDeleteCaller },
#endif

    /*
        JMP.cond 1, 2, 3
            branch.cond 0, 1, 2, 0, 0
    */
    { 1, gcSL_JMP, 0, 1, 2 },
        { -1, 0x16, 0, 1, 2, 0, 0, branchAndDeleteCaller },

    { 0 }
};

/* 0x07 gcSL_ADD */
const gcsSL_PATTERN patterns_ADD[] =
{
    /*
        ADD 1, 2, 3
            add 1, 2, 0, 3, 0
    */
    { 1, gcSL_ADD, 1, 2, 3, 0, 0, _hasRounding_mode },
        { -1, 0x01, 1, 2, 0, 3, 0, rounding_mode },

    /*
        ADD 1, 2, 3
        SAT 4, 1
            add.sat 4, 2, 0, 3, 0
    */
    { 2, gcSL_ADD, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_SAT, 4, 1 },
        { -1, 0x01, 4, 2, 0, 3, 0, _Sat0 },

#if _SUPPORT_LONG_ULONG_DATA_TYPE
    /*
        ADD    1, 2, 3
        STORE1 4, 1, 5
            store 0, 2, 3, 4, 0
    */
    { 2, gcSL_ADD, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_STORE1, 4, 1, 5, 0, 0, _NoLabel_isCL_Long_ulong_2_store },
        { -4, 0x01, gcSL_CG_TEMP1, 3, 0, gcSL_CG_CONSTANT, 0, long_ulong_lower_offset },
        { -3, 0x33, 4, 2, gcSL_CG_TEMP1, 5, 0, long_ulong_first_add_store },
        { -2, 0x01, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_4 },
        { -1, 0x33, 4, 2, gcSL_CG_TEMP1, 5, 0, long_ulong_second_add_store },

    /*
        ADD    1, 2, 3
        STORE1 4, 1, 5
            store 0, 2, 3, 4, 0
    */
    { 2, gcSL_ADD, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_STORE1, 4, 1, 5, 0, 0, _NoLabel_isCL_Long_ulong_4_store },
        { -8, 0x01, gcSL_CG_TEMP1, 3, 0, gcSL_CG_CONSTANT, 0, long_ulong_lower_offset },
        { -7, 0x33, 4, 2, gcSL_CG_TEMP1, 5, 0, long_ulong_first_add_store },
        { -6, 0x01, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_4 },
        { -5, 0x33, 4, 2, gcSL_CG_TEMP1, 5, 0, long_ulong_second_add_store },
        { -4, 0x01, gcSL_CG_TEMP1, 3, 0, gcSL_CG_CONSTANT, 0, long_ulong_upper_offset },
        { -3, 0x33, 4, 2, gcSL_CG_TEMP1, 5, 0, long_ulong_third_add_store },
        { -2, 0x01, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_4 },
        { -1, 0x33, 4, 2, gcSL_CG_TEMP1, 5, 0, long_ulong_fourth_add_store },
#else
    /* Handle OCL 1.2 conformance case without full support of long/ulong */
    /*
        ADD    1, 2, 3
        STORE1 4, 1, 5
            store 0, 2, 3, 4, 0
    */
    { 2, gcSL_ADD, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_STORE1, 4, 1, 5, 0, 0, _NoLabel_isCL_Long_ulong_2_store },
        { -4, 0x01, gcSL_CG_TEMP1, 3, 0, gcSL_CG_CONSTANT, 0, long_ulong_lower_offset },
        { -3, 0x33, 4, 2, gcSL_CG_TEMP1, 5, 0, long_ulong_first_add_store },
        { -2, 0x01, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_4 },
        { -1, 0x33, 4, 2, gcSL_CG_TEMP1, gcSL_CG_CONSTANT, 0, long_ulong_second_store_zero_2 },
#endif

    /*
        ADD    1, 2, 3
        STORE1 4, 1, 5
            store 0, 2, 3, 1, 0
    */
    { 2, gcSL_ADD, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_STORE1, 4, 1, 5, 0, 0, _NoLabel_isCL_X_Signed_8_16_store1 },
        { -3, 0x59, gcSL_CG_TEMP1, 5, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_24_16_from_next_inst },
        { -2, 0x5A, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_24_16_from_next_inst },
        { -1, 0x33, 4, 2, 3, gcSL_CG_TEMP1, 0, denorm_value_type0_from_next_inst },

    /*
        ADD    1, 2, 3
        STORE1 4, 1, 5
            store 0, 2, 3, 1, 0
    */
    { 2, gcSL_ADD, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_STORE1, 4, 1, 5, 0, 0, _NoLabel_isCL_X_Unsigned_8_16_store1 },
        { -2, 0x5D, gcSL_CG_TEMP1, 5, 0, gcSL_CG_CONSTANT, 0, uint_value_type0_const_FF_FFFF_from_next_inst },
        { -1, 0x33, 4, 2, 3, gcSL_CG_TEMP1, 0, denorm_value_type0_from_next_inst },

    /*
        ADD    1, 2, 3
        STORE1 4, 1, 5
            store 0, 2, 3, 4, 0
    */
    { 2, gcSL_ADD, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_STORE1, 4, 1, 5, 0, 0, _NoLabel },
        { -1, 0x33, 4, 2, 3, 5, 0, denorm_value_type0_from_next_inst },

    /*
        ADD 1, 2, 3
        ATOMADD 4, 1, 5
            atom_add 4, 2, 3, 5, 0
    */
    { 2, gcSL_ADD, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_ATOMADD, 4, 1, 5, 0, 0, _NoLabel },
        { -1, 0x65, 4, 2, 3, 5, 0, value_type0 },

    /*
        ADD 1, 2, 3
        ATOMSUB 4, 1, 5
            atom_add 4, 2, 3, -5, 0
    */
    { 2, gcSL_ADD, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_ATOMSUB, 4, 1, 5, 0, 0, _NoLabel },
        { -1, 0x65, 4, 2, 3, -5, 0, value_type0 },

    /*
        ADD 1, 2, 3
        ATOMXCHG 4, 1, 5
            atom_xchg 4, 2, 3, 5, 0
    */
    { 2, gcSL_ADD, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_ATOMXCHG, 4, 1, 5, 0, 0, _NoLabel },
        { -1, 0x66, 4, 2, 3, 5, 0, value_type0 },

    /*
        ADD 1, 2, 3
        ATOMCMPXCHG 4, 1, 5
            atom_cmpxchg 4, 2, 3, 5, 0
    */
    { 2, gcSL_ADD, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_ATOMCMPXCHG, 4, 1, 5, 0, 0, _NoLabel },
        { -1, 0x67, 4, 2, 3, 5, 0, value_type0 },

    /*
        ADD 1, 2, 3
        ATOMMIN 4, 1, 5
            atom_min 4, 2, 3, 5, 0
    */
    { 2, gcSL_ADD, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_ATOMMIN, 4, 1, 5, 0, 0, _NoLabel },
        { -1, 0x68, 4, 2, 3, 5, 0, value_type0 },

    /*
        ADD 1, 2, 3
        ATOMMAX 4, 1, 5
            atom_max 4, 2, 3, 5, 0
    */
    { 2, gcSL_ADD, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_ATOMMAX, 4, 1, 5, 0, 0, _NoLabel },
        { -1, 0x69, 4, 2, 3, 5, 0, value_type0 },

    /*
        ADD 1, 2, 3
        ATOMOR 4, 1, 5
            atom_or 4, 2, 3, 5, 0
    */
    { 2, gcSL_ADD, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_ATOMOR, 4, 1, 5, 0, 0, _NoLabel },
        { -1, 0x6A, 4, 2, 3, 5, 0, value_type0 },

    /*
        ADD 1, 2, 3
        ATOMAND 4, 1, 5
            atom_and 4, 2, 3, 5, 0
    */
    { 2, gcSL_ADD, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_ATOMAND, 4, 1, 5, 0, 0, _NoLabel },
        { -1, 0x6B, 4, 2, 3, 5, 0, value_type0 },

    /*
        ADD 1, 2, 3
        ATOMXOR 4, 1, 5
            atom_xor 4, 2, 3, 5, 0
    */
    { 2, gcSL_ADD, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_ATOMXOR, 4, 1, 5, 0, 0, _NoLabel },
        { -1, 0x6C, 4, 2, 3, 5, 0, value_type0 },

    /*
        ADD 1, 2, 3
            add 1, 2, 0, 3, 0
    */
    { 1, gcSL_ADD, 1, 2, 3, 0, 0, _IntOpcode },
        { -1, 0x01, 1, 2, 0, 3, 0, value_type0 },

    /*
        ADD 1, 2, 3
            add 1, 2, 0, 3, 0
    */
    { 1, gcSL_ADD, 1, 2, 3 },
        { -1, 0x01, 1, 2, 0, 3, 0, add2mad },

    { 0 }
};

/* 0x08 gcSL_MUL */
const gcsSL_PATTERN patterns_MUL[] =
{
    /*
        MUL 1, 2, 3
            mul 1, 2, 3, 0, 0
    */
    { 1, gcSL_MUL, 1, 2, 3, 0, 0, _hasRounding_mode  },
        { -1, 0x03, 1, 2, 3, 0, 0, rounding_mode },

#if (GC_ENABLE_DUAL_FP16 > 0)
    /*
        MUL  1, 2, 3
        FRAC 4, 1
            mul.t0 1.hp, 2, 3, 0, 0
            frc.t0 4, 0, 0, 1.hp
            mul.t1 1.hp, 2, 3, 0, 0
            frc.t1 4, 0, 0, 1.hp
    */
    { 2, gcSL_MUL, 1, 2, 3, 0, 0, _UseDestInNextOnly_ConstSrc1AndDual16On },
    { 1, gcSL_FRAC, 4, 1 },
            { -4, 0x03, gcSL_CG_TEMP1, 2, 3, 0, 0, _t0_destHP },
            { -3, 0x13, 4, 0, 0, gcSL_CG_TEMP1, 0, _t0_src2HP},
            { -2, 0x03, gcSL_CG_TEMP1, 2, 3, 0, 0, _t1_destHP },
            { -1, 0x13, 4, 0, 0, gcSL_CG_TEMP1, 0, _t1_src2HP},


    /*
        MUL  1, 2, 3
        SUB  4, 1, 5
            mad.t0 4, 2, 3.hp, 5.hp
            mad.t1 4, 2, 3.hp, 5.hp
    */
    { 2, gcSL_MUL, 1, 2, 3, 0, 0, _Dual16On_UseDestInNextOnlyAndMADOn },
    { 1, gcSL_SUB, 4, 1, 5, 0, 0, _Dual16OnMediumpDstHighpSrc0HighpSrc1 },
            { -2, 0x02, 4, 2, 3, -5, 0, _t0_destMP},
            { -1, 0x02, 4, 2, 3, -5, 0, _t1_destMP},

    /*
        MUL  1, 2, 3
        MUL  4, 5, 6
        ADD  7, 1, 4
        MUL  8, 9, 10
        ADD  11,7, 8
        ADD  12, 11, 13
            mul.t0 1.hp, 2.hp, 3
            mad.t0 7.hp, 5.hp, 6, 1.hp
            mad.t0 11.hp, 9.hp, 10, 7.hp
            add.t0, 12, 11.hp, 13.hp
    */
    { 6, gcSL_MUL, 1, 2, 3, 0, 0, _Dual16OnMediumpDstHighpSrc0MediumpSrc1 },
    { 5, gcSL_MUL, 4, 5, 6, 0, 0, _UseDestInNextOnlyAndMADOn},
    { 4, gcSL_ADD, 7, 1, 4},
    { 3, gcSL_MUL, 8, 9, 10, 0, 0, _UseDestInNextOnlyAndMADOn},
    { 2, gcSL_ADD, 11, 7, 8},
    { 1, gcSL_ADD, 12, 11, 13},
        { -8, 0x03, gcSL_CG_TEMP1, 2, 3, 0, 0, _t0_destHP },
        { -7, 0x02, gcSL_CG_TEMP2, 5, 6, gcSL_CG_TEMP1, 0, _t0_src2HP_dstHP },
        { -6, 0x02, gcSL_CG_TEMP3, 9, 10, gcSL_CG_TEMP2, 0, _t0_src2HP_dstHP },
        { -5, 0x01, 12, gcSL_CG_TEMP3, 0, 13, 0, _t0_src0HP_dstMP },
        { -4, 0x03, gcSL_CG_TEMP1, 2, 3, 0, 0, _t1_destHP },
        { -3, 0x02, gcSL_CG_TEMP2, 5, 6, gcSL_CG_TEMP1, 0, _t1_src2HP_dstHP },
        { -2, 0x02, gcSL_CG_TEMP3, 9, 10, gcSL_CG_TEMP2, 0, _t1_src2HP_dstHP },
        { -1, 0x01, 12, gcSL_CG_TEMP3, 0, 13, 0, _t1_src0HP_dstMP },

#endif /* GC_ENABLE_DUAL_FP16 */

    /*
        MUL 1, 2, 3
        SAT 4, 1
            mul.sat 4, 2, 3, 0, 0
    */
    { 2, gcSL_MUL, 1, 2, 3, 0, 0, _UseDestInNextOnlyAndMADOn },
    { 1, gcSL_SAT, 4, 1 },
            { -1, 0x03, 4, 2, 3, 0, 0, _Sat0 },

    /*
        MUL 1, 2, 3
        ADD 4, 5, 1
        SAT 6, 4
            mad.sat 6, 2, 3, 5
    */
    { 3, gcSL_MUL, 1, 2, 3, 0, 0, _UseDestInNextOnlyAndMADOn },
    { 2, gcSL_ADD, 4, 5, 1, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_SAT, 6, 4 },
        { -1, 0x02, 6, 2, 3, 5, 0, _Sat0 },

    /*
        MUL 1, 2, 3
        ADD 4, 1, 5
        SAT 6, 4
            mad.sat 6, 2, 3, 5
    */
    { 3, gcSL_MUL, 1, 2, 3, 0, 0, _UseDestInNextOnlyAndMADOn },
    { 2, gcSL_ADD, 4, 1, 5, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_SAT, 6, 4 },
        { -1, 0x02, 6, 2, 3, 5, 0, _Sat0 },

    /*
        MUL 1, 2, 3
        ADD.sat 4, 1, 5
            mad.sat 4, 2, 3, 5, 0
    */
    { 2, gcSL_MUL, 1, 2, 3, 0, 0, _UseDestInNextOnlyAndMADOn },
    { 1, gcSL_ADD, 4, 1, 5, 0, 0, _NoLabel_Sat },
        { -1, 0x02, 4, 2, 3, 5, 0, _Sat0 },

    /*
        MUL 1, 2, 3
        ADD.sat 4, 5, 1
            mad.sat 4, 2, 3, 5, 0
    */
    { 2, gcSL_MUL, 1, 2, 3, 0, 0, _UseDestInNextOnlyAndMADOn },
    { 1, gcSL_ADD, 4, 5, 1, 0, 0, _NoLabel_Sat },
        { -1, 0x02, 4, 2, 3, 5, 0, _Sat0 },

    /*
        MUL 1, 2, 3
        ADD 4, 5, 1
            imadlo0 4, 2, 3, 5, 0
    */
    { 2, gcSL_MUL, 1, 2, 3, 0, 0, _IntUseDestInNextOnly },
    { 1, gcSL_ADD, 4, 5, 1, 0, 0, _NoLabel },
        { -1, 0x4C, 4, 2, 3, 5, 0, value_type0 },

    /*
        MUL 1, 2, 3
        ADD 4, 5, 1
            mad 4, 2, 3, 5, 0
    */
    { 2, gcSL_MUL, 1, 2, 3, 0, 0, _UseDestInNextOnlyAndMADOn },
    { 1, gcSL_ADD, 4, 5, 1, 0, 0, _NoLabel },
        { -1, 0x02, 4, 2, 3, 5, 0 },

    /*
        MUL 1, 2, 3
        ADD 4, 1, 5
            imadlo0 4, 2, 3, 5, 0
    */
    { 2, gcSL_MUL, 1, 2, 3, 0, 0, _IntUseDestInNextOnly },
    { 1, gcSL_ADD, 4, 1, 5, 0, 0, _NoLabel },
        { -1, 0x4C, 4, 2, 3, 5, 0, value_type0 },

    /*
        MUL 1, 2, 3
        ADD 4, 1, 5
            mad 4, 2, 3, 5, 0
    */
    { 2, gcSL_MUL, 1, 2, 3, 0, 0, _UseDestInNextOnlyAndMADOn },
    { 1, gcSL_ADD, 4, 1, 5, 0, 0, _NoLabel },
        { -1, 0x02, 4, 2, 3, 5, 0 },

#if _SUPPORT_INTEGER_NEGATIVE_MODIFIER_
    /*
        MUL 1, 2, 3
        SUB 4, 1, 5
            imadlo0 4, 2, 3, -5, 0
    */
    { 2, gcSL_MUL, 1, 2, 3, 0, 0, _IntUseDestInNextOnly },
    { 1, gcSL_SUB, 4, 1, 5, 0, 0, _NoLabel },
        { -1, 0x4C, 4, 2, 3, -5, 0, value_type0 },
#else
    /* Negative modifier is not allowed in imadlo0. */
    /*
        MUL 1, 2, 3
        SUB 4, 1, 5
            imullo0 1, 2, 3, 0, 0
            add     4, 1, 0, -5, 0
    */
    { 2, gcSL_MUL, 1, 2, 3, 0, 0, _IntUseDestInNextOnly },
    { 1, gcSL_SUB, 4, 1, 5, 0, 0, _NoLabel },
        { -2, 0x3C, gcSL_CG_TEMP1, 2, 3, 0, 0, value_type0 },
        { -1, 0x01, 4, gcSL_CG_TEMP1, 0, -5, 0, value_type0 },
#endif

    /*
        MUL 1, 2, 3
        SUB 4, 1, -5
            mad 4, 2, 3, 5, 0
    */
    { 2, gcSL_MUL, 1, 2, 3, 0, 0, _UseDestInNextOnlyAndMADOn },
    { 1, gcSL_SUB, 4, 1, 5, 0, 0, _NoLabel_Neg1 },
        { -1, 0x02, 4, 2, 3, 5, 0, _Neg2 },

    /*
        MUL 1, 2, 3
        SUB 4, -1, 5
            mad 4, 2, 3, 5, 0
    */
    { 2, gcSL_MUL, 1, 2, 3, 0, 0, _UseDestInNextOnlyAndMADOn },
    { 1, gcSL_SUB, 4, 1, 5, 0, 0, _NoLabel_Neg0 },
        { -1, 0x02, 4, 2, 3, 5, 0, _Neg0 },

    /*
        MUL 1, 2, 3
        SUB.sat 4, 1, 5
            mad.sat 4, 2, 3, -5, 0
    */
    { 2, gcSL_MUL, 1, 2, 3, 0, 0, _UseDestInNextOnlyAndMADOn },
    { 1, gcSL_SUB, 4, 1, 5, 0, 0, _NoLabel_Sat },
        { -1, 0x02, 4, 2, 3, -5, 0, _Sat0 },

    /*
        MUL 1, 2, 3
        SUB.sat 4, 5, 1
            mad.sat 4, 2, 3, -5, 0
    */
    { 2, gcSL_MUL, 1, 2, 3, 0, 0, _UseDestInNextOnlyAndMADOn },
    { 1, gcSL_SUB, 4, 5, 1, 0, 0, _NoLabel_Sat },
        { -1, 0x02, 4, 2, 3, -5, 0, _Sat0 },

    /*
        MUL 1, 2, 3
        SUB 4, 1, 5
            mad 4, 2, 3, -5, 0
    */
    { 2, gcSL_MUL, 1, 2, 3, 0, 0, _UseDestInNextOnlyAndMADOn },
    { 1, gcSL_SUB, 4, 1, 5, 0, 0, _NoLabel },
        { -1, 0x02, 4, 2, 3, -5, 0 },

#if _SUPPORT_INTEGER_NEGATIVE_MODIFIER_
    /*
        MUL 1, 2, 3
        SUB 4, 5, 1
            imadlo0 4, -2, 3, 5, 0
    */
    { 2, gcSL_MUL, 1, 2, 3, 0, 0, _IntUseDestInNextOnly },
    { 1, gcSL_SUB, 4, 5, 1, 0, 0, _NoLabel },
        { -1, 0x4C, 4, -2, 3, 5, 0, value_type0 },
#else
    /* Negative modifier is not allowed in imadlo0. */
    /*
        MUL 1, 2, 3
        SUB 4, 5, 1
            imullo0 1, 2, 3, 0, 0
            add     4, 5, 0, -1, 0
    */
    { 2, gcSL_MUL, 1, 2, 3, 0, 0, _IntUseDestInNextOnly },
    { 1, gcSL_SUB, 4, 5, 1, 0, 0, _NoLabel },
        { -2, 0x3C, gcSL_CG_TEMP1, 2, 3, 0, 0, value_type0 },
        { -1, 0x01, 4, 5, 0, -gcSL_CG_TEMP1, 0, value_type0 },
#endif

    /*
        MUL 1, 2, 3
        SUB 4, 5, 1
            mad 4, -2, 3, 5, 0
    */
    { 2, gcSL_MUL, 1, 2, 3, 0, 0, _UseDestInNextOnlyAndMADOn },
    { 1, gcSL_SUB, 4, 5, 1, 0, 0, _NoLabel },
        { -1, 0x02, 4, -2, 3, 5, 0 },

    /*
        MUL 1, 2, 3
            imul0 1, 2, 3, 0, 0
    */
    { 1, gcSL_MUL, 1, 2, 3, 0, 0, _IntOpcode },
        { -1, 0x3C, 1, 2, 3, 0, 0, value_type0 },

    /*
        MUL 1, 2, 3
            mul 1, 2, 3, 0, 0
    */
    { 1, gcSL_MUL, 1, 2, 3, 0, 0, _InsertNop},
        { -2, 0x03, 1, 2, 3, 0, 0 },
        { -1, 0x00, 0, 0, 0, 0, 0 },

    /*
        MUL 1, 2, 3
            mul 1, 2, 3, 0, 0
    */
    { 1, gcSL_MUL, 1, 2, 3 },
        { -1, 0x03, 1, 2, 3, 0, 0 },

    { 0 }
};

/* 0x09 gcSL_RCP */
const gcsSL_PATTERN patterns_RCP[] =
{
    /*
        RCP   1, 2
        SQRT  3, 1
            rsq 3, 0, 0, 2, 0
    */
    { 2, gcSL_RCP, 1, 2, 0, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_SQRT, 3, 1 },
        { -1, 0x0D, 3, 0, 0, 2, 0 },

    /*  do high precision divsion if it is used in mod
         RCP      1, 2
         MUL      3, 4, 1
         FLOOR    5, 3
            r1 = Rcp(r2)           //Our HW function AQFloatRcpSqrtrcpExpLog32_OCL
            t1 = -r1*r2 + 1.0      //Round to zero (RTZ) MAD
            t2 = MULLO(r1,r2)      //RTZ, our HW MULLOW
            t1 = t1 ?t2           //RTZ
            r3 = r4*r1             //RTZ
            t2 = MULLO(r4, r1)     //RTZ
            t1 = r3*t1 + t2        //MAD, RTNE or RTZ, doesnt matter
            r3 = r3 + t1           //RTNE, Final division output
            r5 = floor(r3)
    */
    { 3, gcSL_RCP, 1, 2, 0, 0, 0, _UseDestInNextOnly_hasMULLO_ICLimit },
    { 2, gcSL_MUL, 3, 4, 1, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_FLOOR, 5, 3, 0, 0, 0, _hasSIGN_FLOOR_CEIL },
        { -10, 0x0C, 1, 0, 0, 2, 0 },
        { -9, 0x02, gcSL_CG_TEMP1, -1, 2, gcSL_CG_CONSTANT, 0, one_2_rtz},
        { -8, 0x29, gcSL_CG_TEMP2, 1, 2, 0, 0, rtz },
        { -7, 0x01, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, -gcSL_CG_TEMP2, 0, rtz },
        { -6, 0x03, 3, 4, 1, 0, 0, rtz },
        { -5, 0x29, gcSL_CG_TEMP2, 4, 1, 0, 0, rtz },
        { -4, 0x02, gcSL_CG_TEMP1, 3, gcSL_CG_TEMP1, gcSL_CG_TEMP2, 0 },
        { -3, 0x01, 3, 3, 0, gcSL_CG_TEMP1, 0, rtne },
        { -2, 0x0F, 3, 4, gcSL_CG_CONSTANT, 3, 0, zero_1_conditionEQ },
        { -1, 0x25, 5, 0, 0, 3 },

    /*
        RCP 1, 2
            rcp 1, 0, 0, 2, 0
    */
    { 1, gcSL_RCP, 1, 2 },
        { -1, 0x0C, 1, 0, 0, 2, 0 },

    { 0 }
};

/* 0x0A gcSL_SUB */
const gcsSL_PATTERN patterns_SUB[] =
{
    /*
        SUB 1, 2, 3
            add 1, 2, 0, -3, 0
    */
    { 1, gcSL_SUB, 1, 2, 3, 0, 0, _hasRounding_mode },
        { -1, 0x01, 1, 2, 0, -3, 0, rounding_mode },

    /* reflect(-a, b) = 2*dot(b, a)*b - a
     *
     *   SUB    1, const_0, 2
     *   DP3    3, 4, 1
     *   ADD    5, 3, 3
     *   MUL    6, 5, 4
     *   SUB    7, 1, 6
     *       dp3 3, 4, 2
     *       add 5, 3, 3
     *       mad 7, 5, 4, -2
     */
    { 5, gcSL_SUB, 1, 0, 2, 0, 0, _Src0Const0_UseDestInTwoOnly },
    { 4, gcSL_DP3, 3, 4, 1, 0, 0, _UseDestInNextOnly },
    { 3, gcSL_ADD, 5, 3, 3, 0, 0, _UseDestInNextOnly },
    { 2, gcSL_MUL, 6, 5, 4, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_SUB, 7, 1, 6, 0, 0, _NoLabel },
        { -3, 0x05, 3, 4, 2, 0, 0 },
        { -2, 0x01, 5, 3, 0, 3, 0 },
        { -1, 0x02, 7, 5, 4, -2, 0 },

    /* reflect(-a, b) = -a -2*dot(b, -a)*b = 2*dot(b, a)*b - a
     *
     *   SUB    1, const_0, 2
     *   DP4    3, 4, 1
     *   ADD    5, 3, 3
     *   MUL    6, 5, 4
     *   SUB    7, 1, 6
     *       dp4 3, 4, 2
     *       add 5, 3, 3
     *       mad 7, 5, 4, -2
     */
    { 5, gcSL_SUB, 1, 0, 2, 0, 0, _Src0Const0_UseDestInTwoOnly },
    { 4, gcSL_DP4, 3, 4, 1, 0, 0, _UseDestInNextOnly },
    { 3, gcSL_ADD, 5, 3, 3, 0, 0, _UseDestInNextOnly },
    { 2, gcSL_MUL, 6, 5, 4, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_SUB, 7, 1, 6, 0, 0, _NoLabel },
        { -3, 0x06, 3, 4, 2, 0, 0 },
        { -2, 0x01, 5, 3, 0, 3, 0 },
        { -1, 0x02, 7, 5, 4, -2, 0 },

    /*
        SUB 1, 2, 3
        SAT 4, 1
            add.sat 4, 2, 0, -3, 0
    */
    { 2, gcSL_SUB, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_SAT, 4, 1 },
        { -1, 0x01, 4, 2, 0, -3, 0, _Sat0 },

#if (GC_ENABLE_DUAL_FP16 > 0)
    /*
     *   SUB    1, 2, 3
     *   RCP    4, 1
     *   MUL    6, 5, 4
     *       add.t0 1.hp, 2, -3.hp
     *       rcp.t0 4.hp, 1.hp
     *       mul.t0 6, 5.hp, 4.hp
     *       add.t1 1.hp, 2, 3.hp
     *       rcp.t1 4.hp, 1.hp
     *       mul.t1 6, 5.hp, 4.hp
     */
     { 3, gcSL_SUB, 1, 2, 3, 0, 0, _Dual16OnHighpDstMediumpSrc0HighpSrc1 },
     { 2, gcSL_RCP, 4, 1, 0, 0, 0, _UseDestInNextOnly },
     { 1, gcSL_MUL, 6, 5, 4, 0, 0, _Dual16OnMediumpDstHighpSrc0HighpSrc1 },
        { -6, 0x01, gcSL_CG_TEMP1, 2, 0, -3, 0, _t0_destHP },
        { -5, 0x0C, gcSL_CG_TEMP2, 0, 0, gcSL_CG_TEMP1, 0, _t0_src2HP_dstHP },
        { -4, 0x03, 6, 5, gcSL_CG_TEMP2, 0, 0, _t0_src1HP_dstMP },
        { -3, 0x01, gcSL_CG_TEMP1, 2, 0, -3, 0, _t1_destHP },
        { -2, 0x0C, gcSL_CG_TEMP2, 0, 0, gcSL_CG_TEMP1, 0, _t1_src2HP_dstHP },
        { -1, 0x03, 6, 5, gcSL_CG_TEMP2, 0, 0, _t1_src1HP_dstMP },
#endif

    /*
        SUB 1, 2, 3
            add 1, 2, 0, -3, 0
    */
    { 1, gcSL_SUB, 1, 2, 3, 0, 0, _IntOpcode },
        { -1, 0x01, 1, 2, 0, -3, 0, value_type0 },


#if (GC_ENABLE_DUAL_FP16 > 0)
    /*
        SUB 1, 2, 3
            add.t0 1, 2, 0, -3.hp, 0
            add.t1 1, 2, 0, -3.hp, 0
    */
    { 1, gcSL_SUB, 1, 2, 3, 0, 0, _Dual16OnMediumpDstMediumpSrc0HighpSrc1 },
        { -2, 0x01, 1, 2, 0, -3, 0, _t0_destMP},
        { -1, 0x01, 1, 2, 0, -3, 0, _t1_destMP},
#endif

    /*
        SUB 1, 2, 3
            add 1, 2, 0, -3, 0
    */
    { 1, gcSL_SUB, 1, 2, 3 },
        { -1, 0x01, 1, 2, 0, -3, 0 },

    { 0 }
};

/* 0x0B gcSL_KILL */
const gcsSL_PATTERN patterns_KILL[] =
{
    /*
        KILL.cond 1, 2
            texkill.cond 0, 1, 2, 0, 0
    */
    { 1, gcSL_KILL, 0, 1, 2 },
        { -1, 0x17, 0, 1, 2, 0, 0, texkill },

    { 0 }
};

/* 0x0C gcSL_TEXLD */
const gcsSL_PATTERN patterns_TEXLD[] =
{
    /*
        TEXLD  1, 2, 3
            texld_l_pcf 1, 3, CONSTANT_0, 0, 2
    */
    { 1, gcSL_TEXLD, 1, 2, 3, 0, 0, _hasNEW_TEXLD_isVertex },
        { -1, 0x6F, 1, 3, gcSL_CG_CONSTANT, 0, 2, zero_1 },

    /*
        TEXLD  1, 2, 3
            mov TEMP1_XYZW, 0, 0, 3, 0
            mov TEMP1_XYZW.w, 0, 0, CONSTANT_0, 0
            texldl 1, TEMP1_XYZW, 0, 0, 2
    */
    { 1, gcSL_TEXLD, 1, 2, 3, 0, 0, _isVertex },
        { -3, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0 },
        { -2, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_CONSTANT, 0, zero_2_enable_w },
        { -1, 0x1B, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2 },

    /*
        TEXLD 1, 2, 3
            texld 1, 3, 0, 0, 2
    */
    { 1, gcSL_TEXLD, 1, 2, 3 },
        { -1, 0x18, 1, 3, 0, 0, 2 },

    { 0 }
};

/* 0x0D gcSL_CALL */
const gcsSL_PATTERN patterns_CALL[] =
{
    /*
        CALL.cond 1, 2, 3
            call.cond 0, 1, 2, 0, 0
    */
    { 1, gcSL_CALL, 0, 1, 2 },
        { -1, 0x14, 0, 1, 2, 0, 0, branchAndDeleteCaller },

    { 0 }
};

/* 0x0E gcSL_RET */
const gcsSL_PATTERN patterns_RET[] =
{
    /*
        RET
            ret 0, 0, 0, 0, 0
    */
    { 1, gcSL_RET },
        { -1, 0x15, 0, 0, 0, 0, 0 },

    { 0 }
};

/* 0x0F gcSL_NORM */
const gcsSL_PATTERN patterns_NORM[] =
{
/* USE hardware NORMALIZE macro */
#if (GC_ENABLE_DUAL_FP16 > 0) && _USE_HARDWARE_NORMALIZE_MACRO_OPCODES
#if DX_SHADER
    /*
DX Spec:

This instruction works conceptually as shown here.

squareRootOfTheSum = (src0.x*src0.x + src0.y*src0.y + src0.z*src0.z)1/2;
dest.x = src0.x * (1 / squareRootOfTheSum);
dest.y = src0.y * (1 / squareRootOfTheSum);
dest.z = src0.z * (1 / squareRootOfTheSum);
dest.w = src0.w * (1 / squareRootOfTheSum);
    */

    /*
        NORM 1, 2
            dp3 TEMP1.x, 2, 2, 0, 0
            rsq TEMP1.x, 0, 0, TEMP1.x, 0
            mul 1, 2, TEMP1.x, 0, 0
    */
    { 1, gcSL_NORM, 1, 2, 0, 0, 0, _HasPreNormInst },
        { -3, 0x75, gcSL_CG_TEMP1_X, 2, 2, 0, 0, destMpSrc0MpSrc1Mp },
        { -2, 0x0D, gcSL_CG_TEMP1_X, 0, 0, gcSL_CG_TEMP1_X, 0 /*, swizzle2X */},
        { -1, 0x77, 1, 2, gcSL_CG_TEMP1_X, 0, 0, destMpSrc0MpSrc1Mp },
    /*
        NORM 1, 2
            dp3 TEMP1.x, 2, 2, 0, 0
            rsq TEMP1.x, 0, 0, TEMP1.x, 0
            mul 1, 2, TEMP1.x, 0, 0
    */
    { 1, gcSL_NORM, 1, 2, 0, 0, 0, 0 },
        { -3, 0x05, gcSL_CG_TEMP1_X, 2, 2, 0, 0 },
        { -2, 0x0D, gcSL_CG_TEMP1_X, 0, 0, gcSL_CG_TEMP1_X, 0/*, swizzle2X */},
        { -1, 0x03, 1, 2, gcSL_CG_TEMP1_X, 0, 0 },
#else
    /*
        NORM 1, 2
            mul TEMP1, 2, 2, 0, 0
            add TEMP.x, TEMP.x, TEMP.y
            rsq TEMP1, 0, 0, TEMP1.x, 0
            mul 1, 2, TEMP1, 0, 0
    */
    { 1, gcSL_NORM, 1, 2, 0, 0, 0, _OnlyXYOrXEnabledHasDual16 },
        { -3, 0x74, gcSL_CG_TEMP1_X, 2, 2, 0, 0, destMpSrc0MpSrc1Mp },
        { -2, 0x0D, gcSL_CG_TEMP1_X, 0, 0, gcSL_CG_TEMP1_X, 0 /*, swizzle2X */},
        { -1, 0x77, 1, 2, gcSL_CG_TEMP1_X, 0, 0, set_norm_mul0zero },

    /*
        NORM 1, 2
            mul TEMP1, 2, 2, 0, 0
            add TEMP.x, TEMP.x, TEMP.y
            rsq TEMP1, 0, 0, TEMP1.x, 0
            mul 1, 2, TEMP1, 0, 0
    */
    { 1, gcSL_NORM, 1, 2, 0, 0, 0, _OnlyXYOrXEnabled },
        { -4, 0x03, gcSL_CG_TEMP1, 2, 2, 0, 0 },
        { -3, 0x01, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, gcSL_CG_TEMP1, 0, swizzleS0xS2y},
        { -2, 0x0D, gcSL_CG_TEMP1, 0, 0, gcSL_CG_TEMP1, 0, swizzle2X },
        { -1, 0x03, 1, 2, gcSL_CG_TEMP1, 0, 0 },

    /*
        NORM 1, 2
            dp3 TEMP1.x, 2, 2, 0, 0
            rsq TEMP1.x, 0, 0, TEMP1.x, 0
            mul 1, 2, TEMP1.x, 0, 0
    */
    { 1, gcSL_NORM, 1, 2, 0, 0, 0, _OnlyXYZEnabledHasDual16 },
        { -3, 0x75, gcSL_CG_TEMP1_X, 2, 2, 0, 0, destMpSrc0MpSrc1Mp },
        { -2, 0x0D, gcSL_CG_TEMP1_X, 0, 0, gcSL_CG_TEMP1_X, 0 /*, swizzle2X */},
        { -1, 0x77, 1, 2, gcSL_CG_TEMP1_X, 0, 0, set_norm_mul0zero },

    /*
        NORM 1, 2
            dp3 TEMP1.x, 2, 2, 0, 0
            rsq TEMP1.x, 0, 0, TEMP1.x, 0
            mul 1, 2, TEMP1.x, 0, 0
    */
    { 1, gcSL_NORM, 1, 2, 0, 0, 0, _OnlyXYZEnabled },
        { -3, 0x05, gcSL_CG_TEMP1_X, 2, 2, 0, 0 },
        { -2, 0x0D, gcSL_CG_TEMP1_X, 0, 0, gcSL_CG_TEMP1_X, 0/*, swizzle2X */},
        { -1, 0x03, 1, 2, gcSL_CG_TEMP1_X, 0, 0 },

    /*
        NORM 1, 2
            dp4 TEMP1.x, 2, 2, 0, 0
            rsq TEMP1.x, 0, 0, TEMP1.x, 0
            mul 1, 2, TEMP1.x, 0, 0
    */
    { 1, gcSL_NORM, 1, 2, 0, 0, 0, _XYZWEnabledHasDual16 },
        { -3, 0x76, gcSL_CG_TEMP1_X, 2, 2, 0, 0, destMpSrc0MpSrc1Mp },
        { -2, 0x0D, gcSL_CG_TEMP1_X, 0, 0, gcSL_CG_TEMP1_X, 0 /*, swizzle2X */},
        { -1, 0x77, 1, 2, gcSL_CG_TEMP1_X, 0, 0, set_norm_mul0zero },

    /*
        NORM 1, 2
            dp4 TEMP1.x, 2, 2, 0, 0
            rsq TEMP1.x, 0, 0, TEMP1.x, 0
            mul 1, 2, TEMP1.x, 0, 0
    */
    { 1, gcSL_NORM, 1, 2, 0, 0, 0, _XYZWEnabled },
        { -3, 0x06, gcSL_CG_TEMP1_X, 2, 2, 0, 0 },
        { -2, 0x0D, gcSL_CG_TEMP1_X, 0, 0, gcSL_CG_TEMP1_X, 0/*, swizzle2X */},
        { -1, 0x03, 1, 2, gcSL_CG_TEMP1_X, 0, 0 },
#endif
#else
#if (GC_ENABLE_DUAL_FP16 > 0)
    /*
        NORM 1, 2
            mul TEMP1, 2, 2, 0, 0
            add TEMP.x, TEMP.x, TEMP.y
            rsq TEMP1, 0, 0, TEMP1.x, 0
            mul 1, 2, TEMP1, 0, 0
    */
    { 1, gcSL_NORM, 1, 2, 0, 0, 0, _OnlyXYOrXEnabledMediumpSrc0 },
        { -8, 0x03, gcSL_CG_TEMP1, 2, 2, 0, 0, destAddrT0HpMpMp },
        { -7, 0x01, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, gcSL_CG_TEMP1, 0, swizzleS0xS2yWithT0HpHpHp},
        { -6, 0x0D, gcSL_CG_TEMP1, 0, 0, gcSL_CG_TEMP1, 0, swizzle2XWithT0HpHp },
        { -5, 0x03, 1, 2, gcSL_CG_TEMP1, 0, 0, destAddrT0MpHpMp },
        { -4, 0x03, gcSL_CG_TEMP1, 2, 2, 0, 0, destAddrT1HpMpMp },
        { -3, 0x01, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, gcSL_CG_TEMP1, 0, swizzleS0xS2yWithT1HpHpHp},
        { -2, 0x0D, gcSL_CG_TEMP1, 0, 0, gcSL_CG_TEMP1, 0, swizzle2XWithT1HpHp },
        { -1, 0x03, 1, 2, gcSL_CG_TEMP1, 0, 0, destAddrT1MpHpMp },

    /*
        NORM 1, 2
            mul TEMP1, 2, 2, 0, 0
            add TEMP.x, TEMP.x, TEMP.y
            rsq TEMP1, 0, 0, TEMP1.x, 0
            mul 1, 2, TEMP1, 0, 0
    */
    { 1, gcSL_NORM, 1, 2, 0, 0, 0, _OnlyXYOrXEnabled },
        { -4, 0x03, gcSL_CG_TEMP1, 2, 2, 0, 0 },
        { -3, 0x01, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, gcSL_CG_TEMP1, 0, swizzleS0xS2y},
        { -2, 0x0D, gcSL_CG_TEMP1, 0, 0, gcSL_CG_TEMP1, 0, swizzle2X },
        { -1, 0x03, 1, 2, gcSL_CG_TEMP1, 0, 0 },

    /*
        NORM 1, 2
            dp3 TEMP1.x, 2, 2, 0, 0
            rsq TEMP1.x, 0, 0, TEMP1.x, 0
            mul 1, 2, TEMP1.x, 0, 0
    */
    { 1, gcSL_NORM, 1, 2, 0, 0, 0, _OnlyXYZEnabledMediumpSrc0 },
        { -6, 0x05, gcSL_CG_TEMP1_X, 2, 2, 0, 0, destAddrT0HpMpMp },
        { -5, 0x0D, gcSL_CG_TEMP1_X, 0, 0, gcSL_CG_TEMP1_X, 0, destAddrT0HpHp/*, swizzle2X */},
        { -4, 0x03, 1, 2, gcSL_CG_TEMP1_X, 0, 0, destAddrT0MpHpMp },
        { -3, 0x05, gcSL_CG_TEMP1_X, 2, 2, 0, 0, destAddrT1HpMpMp },
        { -2, 0x0D, gcSL_CG_TEMP1_X, 0, 0, gcSL_CG_TEMP1_X, 0, destAddrT1HpHp/*, swizzle2X */},
        { -1, 0x03, 1, 2, gcSL_CG_TEMP1_X, 0, 0, destAddrT1MpHpMp },


    /*
        NORM 1, 2
            dp3 TEMP1.x, 2, 2, 0, 0
            rsq TEMP1.x, 0, 0, TEMP1.x, 0
            mul 1, 2, TEMP1.x, 0, 0
    */
    { 1, gcSL_NORM, 1, 2, 0, 0, 0, _OnlyXYZEnabled },
        { -3, 0x05, gcSL_CG_TEMP1_X, 2, 2, 0, 0 },
        { -2, 0x0D, gcSL_CG_TEMP1_X, 0, 0, gcSL_CG_TEMP1_X, 0/*, swizzle2X */},
        { -1, 0x03, 1, 2, gcSL_CG_TEMP1_X, 0, 0 },

    /*
        NORM 1, 2
            dp4 TEMP1.x, 2, 2, 0, 0
            rsq TEMP1.x, 0, 0, TEMP1.x, 0
            mul 1, 2, TEMP1.x, 0, 0
    */
    { 1, gcSL_NORM, 1, 2, 0, 0, 0, _XYZWEnabledMediumpSrc0 },
        { -6, 0x06, gcSL_CG_TEMP1_X, 2, 2, 0, 0, destAddrT0HpMpMp },
        { -5, 0x0D, gcSL_CG_TEMP1_X, 0, 0, gcSL_CG_TEMP1_X, 0, destAddrT0HpHp/*, swizzle2X */},
        { -4, 0x03, 1, 2, gcSL_CG_TEMP1_X, 0, 0, destAddrT0MpHpMp },
        { -3, 0x06, gcSL_CG_TEMP1_X, 2, 2, 0, 0, destAddrT1HpMpMp },
        { -2, 0x0D, gcSL_CG_TEMP1_X, 0, 0, gcSL_CG_TEMP1_X, 0, destAddrT1HpHp/*, swizzle2X */},
        { -1, 0x03, 1, 2, gcSL_CG_TEMP1_X, 0, 0, destAddrT1MpHpMp },

    /*
        NORM 1, 2
            dp4 TEMP1.x, 2, 2, 0, 0
            rsq TEMP1.x, 0, 0, TEMP1.x, 0
            mul 1, 2, TEMP1.x, 0, 0
    */
    { 1, gcSL_NORM, 1, 2, 0, 0, 0, _XYZWEnabled },
        { -3, 0x06, gcSL_CG_TEMP1_X, 2, 2, 0, 0 },
        { -2, 0x0D, gcSL_CG_TEMP1_X, 0, 0, gcSL_CG_TEMP1_X, 0/*, swizzle2X */},
        { -1, 0x03, 1, 2, gcSL_CG_TEMP1_X, 0, 0 },

#else
    /*
        NORM 1, 2
            mul TEMP1, 2, 2, 0, 0
            add TEMP.x, TEMP.x, TEMP.y
            rsq TEMP1, 0, 0, TEMP1.x, 0
            mul 1, 2, TEMP1, 0, 0
    */
    { 1, gcSL_NORM, 1, 2, 0, 0, 0, _OnlyXYOrXEnabled },
        { -4, 0x03, gcSL_CG_TEMP1, 2, 2, 0, 0 },
        { -3, 0x01, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, gcSL_CG_TEMP1, 0, swizzleS0xS2y},
        { -2, 0x0D, gcSL_CG_TEMP1, 0, 0, gcSL_CG_TEMP1, 0, swizzle2X },
        { -1, 0x03, 1, 2, gcSL_CG_TEMP1, 0, 0 },

    /*
        NORM 1, 2
            dp3 TEMP1.x, 2, 2, 0, 0
            rsq TEMP1.x, 0, 0, TEMP1.x, 0
            mul 1, 2, TEMP1.x, 0, 0
    */
    { 1, gcSL_NORM, 1, 2, 0, 0, 0, _OnlyXYZEnabled },
        { -3, 0x05, gcSL_CG_TEMP1_X, 2, 2, 0, 0 },
        { -2, 0x0D, gcSL_CG_TEMP1_X, 0, 0, gcSL_CG_TEMP1_X, 0/*, swizzle2X */},
        { -1, 0x03, 1, 2, gcSL_CG_TEMP1_X, 0, 0 },


    /*
        NORM 1, 2
            dp4 TEMP1.x, 2, 2, 0, 0
            rsq TEMP1.x, 0, 0, TEMP1.x, 0
            mul 1, 2, TEMP1.x, 0, 0
    */
    { 1, gcSL_NORM, 1, 2, 0, 0, 0, _XYZWEnabled },
        { -3, 0x06, gcSL_CG_TEMP1_X, 2, 2, 0, 0 },
        { -2, 0x0D, gcSL_CG_TEMP1_X, 0, 0, gcSL_CG_TEMP1_X, 0/*, swizzle2X */},
        { -1, 0x03, 1, 2, gcSL_CG_TEMP1_X, 0, 0 },

#endif
#endif

    { 0 }
};

/* 0x10 gcSL_MAX */
const gcsSL_PATTERN patterns_MAX[] =
{
    /*
        MAX 1, 2, 3
            select.lt 1, 2, 3, 2, 0
    */
    { 1, gcSL_MAX, 1, 2, 3, 0, 0, _hasHalti4_orPerf },
        { -1, 0x0F, 1, 2, 3, 2, 0, conditionLT },

    /*
        MAX 1, 2, 3
            select.lt 1, 2, 3, 2, 0
    */
    { 1, gcSL_MAX, 1, 2, 3, 0, 0, _IntOpcode },
        { -1, 0x0F, 1, 2, 3, 2, 0, conditionLT },

    /*
        MAX 1, 2, 3
            select.lt 1, 2, 3, 2, 0
            add       1, 1, 0, zero

        new chip's select doesn't flush denorm to zero, add zero to flush to zero
    */
    { 1, gcSL_MAX, 1, 2, 3 },
        { -2, 0x0F, gcSL_CG_TEMP1, 2, 3, 2, 0, conditionLT },
        { -1, 0x01, 1, gcSL_CG_TEMP1, 0, gcSL_CG_CONSTANT, 0, zero_2 },

    { 0 }
};

/* 0x11 gcSL_MIN */
const gcsSL_PATTERN patterns_MIN[] =
{
    /*
        MIN 1, 2, 3
            select.gt 1, 2, 3, 2, 0
    */
    { 1, gcSL_MIN, 1, 2, 3, 0, 0, _hasHalti4_orPerf },
        { -1, 0x0F, 1, 2, 3, 2, 0, conditionGT },

    /*
        MIN 1, 2, 3
            select.gt 1, 2, 3, 2, 0
    */
    { 1, gcSL_MIN, 1, 2, 3, 0, 0, _IntOpcode },
        { -1, 0x0F, 1, 2, 3, 2, 0, conditionGT },

    /*
        MIN 1, 2, 3
            select.gt 1, 2, 3, 2, 0
            add       1, 1, 0, zero

        new chip's select doesn't flush denorm to zero, add zero to flush to zero
    */
    { 1, gcSL_MIN, 1, 2, 3 },
        { -2, 0x0F, gcSL_CG_TEMP1, 2, 3, 2, 0, conditionGT },
        { -1, 0x01, 1, gcSL_CG_TEMP1, 0, gcSL_CG_CONSTANT, 0, zero_2 },

    { 0 }
};

/* 0x12 gcSL_POW */
const gcsSL_PATTERN patterns_POW[] =
{
    /*
        POW 1, 2, 3
            log t1, 0, 0, 2, 0
            mul t1, t1, 3, 0, 0
            exp 1, 0, 0, t1, 0
    */
    { 1, gcSL_POW, 1, 2, 3, 0, 0, _isCLShader_hasNEW_SIN_COS_LOG_DIV },
        { -3, 0x12, gcSL_CG_TEMP1, 0, 0, 2, 0, set_new_sin_cos_log_div },
        { -2, 0x03, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 3, 0, 0 },
        { -1, 0x11, 1, 0, 0, gcSL_CG_TEMP1, 0 },

    /*
        POW 1, 2, 3
            log t1, 0, 0, |2|, 0
            mul t1, t1, 3, 0, 0
            exp 1, 0, 0, t1, 0
    */
    { 1, gcSL_POW, 1, 2, 3 },
        { -3, 0x12, gcSL_CG_TEMP1, 0, 0, 2, 0, set_src2_abs_set_new_sin_cos_log_div },
        { -2, 0x03, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 3, 0, 0 },
        { -1, 0x11, 1, 0, 0, gcSL_CG_TEMP1, 0 },

    { 0 }
};

/* 0x13 gcSL_RSQ */
const gcsSL_PATTERN patterns_RSQ[] =
{
    /*
        RSQ 1, 2
            rsq 1, 0, 0, 2, 0
    */
    { 1, gcSL_RSQ, 1, 2 },
        { -1, 0x0D, 1, 0, 0, 2, 0 },

    { 0 }
};

/* 0x14 gcSL_LOG */
const gcsSL_PATTERN patterns_LOG[] =
{
    /*
        LOG 1, 2
            log 1, 0, 0, 2, 0
    */
    { 1, gcSL_LOG, 1, 2, 0, 0, 0, _isCLShader_hasNEW_SIN_COS_LOG_DIV },
        { -1, 0x12, 1, 0, 0, 2, 0, set_new_sin_cos_log_div },

    /*
        LOG 1, 2
            select.gz 1, 2, 2, smallest0, 0
            log 1, 0, 0, 1, 0
    */
    { 1, gcSL_LOG, 1, 2, 0, 0, 0, 0 },
        { -2, 0x0F, gcSL_CG_TEMP1, 2, 2, gcSL_CG_CONSTANT, 0, smallest0_2_GZ },
        { -1, 0x12, 1, 0, 0, gcSL_CG_TEMP1, 0, set_new_sin_cos_log_div },

    { 0 }
};

/* 0x15 gcSL_FRAC */
const gcsSL_PATTERN patterns_FRAC[] =
{
    /*
        FRAC 1, 2
            frc 1, 0, 0, 2, 0
    */
    { 1, gcSL_FRAC, 1, 2, 0, 0, 0, _hasRounding_mode },
        { -1, 0x13, 1, 0, 0, 2, 0, rounding_mode },

    /*
        FRAC 1, 2
            frc 1, 0, 0, 2, 0
        gc3000/gc5000 has bug which return 1.0 for FRAC(-2^-20) when RTNE
        need to set RTZ explicitly
    */
    { 1, gcSL_FRAC, 1, 2, 0, 0, 0, _isNotCLShader },
        { -1, 0x13, 1, 0, 0, 2, 0, rtz_for_HW_bug },

    /*
        FRAC 1, 2
            floor 1, 0, 0, 2, 0
            add 1, 2, 0, -1, 0
    */
    { 1, gcSL_FRAC, 1, 2, 0, 0, 0, _hasSIGN_FLOOR_CEIL },
        { -2, 0x25, gcSL_CG_TEMP1, 0, 0, 2, 0 },
        { -1, 0x01, 1, 2, 0, -gcSL_CG_TEMP1, 0 },

    /*
        FRAC 1, 2
            frc 1, 0, 0, 2, 0
    */
    { 1, gcSL_FRAC, 1, 2 },
        { -1, 0x13, 1, 0, 0, 2, 0 },

    { 0 }
};

/* 0x16 gcSL_FLOOR */
const gcsSL_PATTERN patterns_FLOOR[] =
{
    /*
        FLOOR 1, 2
            floor 1, 0, 0, 2
    */
    { 1, gcSL_FLOOR, 1, 2, 0, 0, 0, _hasSIGN_FLOOR_CEIL },
        { -1, 0x25, 1, 0, 0, 2 },

    /*
        FLOOR 1, 2
            frc 1, 0, 0, 2, 0
            add 1, 2, 0, -1, 0
    */
    { 1, gcSL_FLOOR, 1, 2 },
        { -2, 0x13, gcSL_CG_TEMP1, 0, 0, 2, 0 },
        { -1, 0x01, 1, 2, 0, -gcSL_CG_TEMP1, 0 },

    { 0 }
};

/* 0x17 gcSL_CEIL */
const gcsSL_PATTERN patterns_CEIL[] =
{
    /*
        CEIL 1, 2
            ceil 1, 0, 0, 2
    */
    { 1, gcSL_CEIL, 1, 2, 0, 0, 0, _hasSIGN_FLOOR_CEIL },
        { -1, 0x26, 1, 0, 0, 2 },

    /*
        CEIL 1, 2
            frc 1, 0, 0, 2, 0
            add TEMP1_XYZW, one, 0, -1, 0
            select.gz 1, 1, TEMP1_XYZW, 1, 0
            add 1, 2, 0, 1, 0
    */
    { 1, gcSL_CEIL, 1, 2 },
        { -4, 0x13, gcSL_CG_TEMP2, 0, 0, 2, 0 },
        { -3, 0x01, gcSL_CG_TEMP1_XYZW, gcSL_CG_CONSTANT, 0, -1, 0, one_0 },
        { -2, 0x0F, gcSL_CG_TEMP2, gcSL_CG_TEMP2, gcSL_CG_TEMP1_XYZW, gcSL_CG_TEMP2, 0, conditionGZ },
        { -1, 0x01, 1, 2, 0, gcSL_CG_TEMP2, 0 },

    { 0 }
};

/* 0x18 gcSL_CROSS */
const gcsSL_PATTERN patterns_CROSS[] =
{
    /*
        CROSS 1, 2, 3
            mul 1, 2.zxy, 3.yzx, 0, 0
            mad 1, 3.zxy, 2.yzx, -1, 0
    */
    { 1, gcSL_CROSS, 1, 2, 3 },
        { -3, 0x03, gcSL_CG_TEMP1_XYZW, 2, 3, 0, 0, crossSwizzle },
        { -2, 0x02, gcSL_CG_TEMP1_XYZW, 3, 2, -gcSL_CG_TEMP1_XYZW, 0, crossSwizzle },
        { -1, 0x09, 1, 0, 0, gcSL_CG_TEMP1_XYZW, 0, 0 },

    { 0 }
};

/* 0x19 gcSL_TEXLDPROJ */
const gcsSL_PATTERN patterns_TEXLDPROJ[] =
{
    /*
        TEXLDPROJ 1, 2, 3
            rcp TEMP1, 0, 0, 3.<last>
            mul TEMP1, 3, TEMP1, 0
            texld_l_pcf 1, TEMP1, 4, 0, 2
    */
    { 1, gcSL_TEXLDPROJ, 1, 2, 3, 0, 0, _hasNEW_TEXLD_isVertex },
        { -3, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, swizzle2ZorW },
        { -2, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0 },
        { -1, 0x6F, 1, gcSL_CG_TEMP1_XYZW, gcSL_CG_CONSTANT, 0, 2, zero_1 },

    /*
        TEXLDPROJ 1, 2, 3
            rcp TEMP1, 0, 0, 3.<last>
            mul TEMP1, 3, TEMP1, 0
            mov TEMP1, 0, 0, 4
            texldl 1, TEMP1, 0, 0, 2
    */
    { 1, gcSL_TEXLDPROJ, 1, 2, 3, 0, 0, _isVertex },
        { -4, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, swizzle2ZorW },
        { -3, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0 },
        { -2, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_CONSTANT, 0, zero_2_enable_w },
        { -1, 0x1B, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2 },

    /*
        TEXLDPROJ 1, 2, 3
            rcp TEMP1, 0, 0, 3.<last>
            mul TEMP1, 3, TEMP1, 0
            texld 1, TEMP1, 0, 0, 2
    */
    { 1, gcSL_TEXLDPROJ, 1, 2, 3 },
        { -3, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, swizzle2ZorW },
        { -2, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0 },
        { -1, 0x18, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2 },

    { 0 }
};

/* 0x1A gcSL_TEXBIAS */
const gcsSL_PATTERN patterns_TEXBIAS[] =
{
    /*
        TEXBIAS 0, 2, 4
        TEXLD   1, 2, 3
            texld_b_pcf 1, 3, 4, 0, 2
    */
    { 2, gcSL_TEXBIAS, 0, 2, 4 },
    { 1, gcSL_TEXLD, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -1, 0x18, 1, 3, 4, 0, 2 },

    /*
        TEXBIAS 0, 2, 4
        TEXLD   1, 2, 3
            mov TEMP1_XYZW, 0, 0, 3, 0
            mov TEMP1_XYZW.w, 0, 0, 4, 0
            texldb 1, TEMP1_XYZW, 0, 0, 2
    */
    { 2, gcSL_TEXBIAS, 0, 2, 4 },
    { 1, gcSL_TEXLD, 1, 2, 3 },
        { -3, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0 },
        { -2, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 4, 0, enable_w },
        { -1, 0x19, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2 },

    /*
        TEXBIAS   0, 2, 4
        TEXLDPROJ 1, 2, 3
            rcp TEMP1, 0, 0, 3.<last>
            mul TEMP1, 3, TEMP1, 0
            texld_b_pcf 1, TEMP1, 4, 0, 2
    */
    { 2, gcSL_TEXBIAS, 0, 2, 4 },
    { 1, gcSL_TEXLDPROJ, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -3, 0x0C, gcSL_CG_TEMP1, 0, 0, 3, 0, swizzle2ZorW_from_next_inst },
        { -2, 0x03, gcSL_CG_TEMP1, 3, gcSL_CG_TEMP1, 0, 0 },
        { -1, 0x18, 1, gcSL_CG_TEMP1, 4, 0, 2 },

    /*
        TEXBIAS   0, 2, 4
        TEXLDPROJ 1, 2, 3
            rcp TEMP1, 0, 0, 3.<last>
            mul TEMP1, 3, TEMP1, 0
            mov TEMP1, 0, 0, 4
            texldb 1, TEMP1, 0, 0, 2
    */
    { 2, gcSL_TEXBIAS, 0, 2, 4 },
    { 1, gcSL_TEXLDPROJ, 1, 2, 3 },
        { -4, 0x0C, gcSL_CG_TEMP1, 0, 0, 3, 0, swizzle2ZorW_from_next_inst },
        { -3, 0x03, gcSL_CG_TEMP1, 3, gcSL_CG_TEMP1, 0, 0 },
        { -2, 0x09, gcSL_CG_TEMP1, 0, 0, 4, 0, enable_w },
        { -1, 0x19, 1, gcSL_CG_TEMP1, 0, 0, 2 },

#if _CLAMP_PCF_REFERENCE_
    /*
        TEXBIAS 0, 2, 4
        TEXLDPCF 1, 2, 3
            texld_b_pcf 1, 3, 4, 0, 2
    */
    { 2, gcSL_TEXBIAS, 0, 2, 4 },
    { 1, gcSL_TEXLDPCF, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -2, 0x09, gcSL_CG_TEMP1_X_NO_SRC_SHIFT, 0, 0, 3, 0, saturate_swizzle2ZorW_from_next_inst },
        { -1, 0x18, 1, 3, 4, gcSL_CG_TEMP1_X_NO_SRC_SHIFT, 2, sample_swizzleX_fix_shadow_coord_swizzle },

    /*
        TEXBIAS 0, 2, 4
        TEXLDPCF 1, 2, 3
            mov TEMP1_XYZW, 0, 0, 3, 0
            mov TEMP1_XYZW.w, 0, 0, 4, 0
            texldb  1, TEMP1_XYZW, 0, 0, 2
            set.le 1, 1, 3.w
    */
    { 2, gcSL_TEXBIAS, 0, 2, 4 },
    { 1, gcSL_TEXLDPCF, 1, 2, 3 },
        { -5, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0 },
        { -4, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 4, 0, enable_w },
        { -3, 0x09, gcSL_CG_TEMP2_X_NO_SRC_SHIFT, 0, 0, 3, 0, saturate_swizzle2ZorW_from_next_inst },
        { -2, 0x19, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2, sample_swizzleX_fix_shadow_coord_swizzle },
        { -1, 0x10, 1, gcSL_CG_TEMP2_X_NO_SRC_SHIFT, 1, 0, 0, conditionLE },
    /*
        TEXBIAS 0, 2, 4
        TEXLDPCFPROJ 1, 2, 3
            rcp TEMP1, 0, 0, 3.<last>
            mul TEMP1, 3, TEMP1, 0
            texld_b_pcf 1, TEMP1, 0, 0, 2
    */
    { 2, gcSL_TEXBIAS, 0, 2, 4 },
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -4, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -3, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -2, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_TEMP1_XYZW, 0, enable_z_saturate_swizzle2Z },
        { -1, 0x18, 1, gcSL_CG_TEMP1_XYZW, 4, gcSL_CG_TEMP1_XYZW, 2, swizzle2Z_sample_swizzleX },

    /*
        TEXBIAS 0, 2, 4
        TEXLDPCFPCFPROJ 1, 2, 3
            rcp TEMP1, 0, 0, 3.<last>
            mul TEMP1, 3, TEMP1, 0
            texldb  1, TEMP1, 0, 0, 2
            set.le 1, 1, 3.w
    */
    { 2, gcSL_TEXBIAS, 0, 2, 4 },
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3 },
        { -6, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -5, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -4, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_TEMP1_XYZW, 0, enable_z_saturate_swizzle2Z },
        { -3, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 4, 0, enable_w },
        { -2, 0x19, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2, sample_swizzleX },
        { -1, 0x10, 1, gcSL_CG_TEMP1_XYZW, 1, 0, 0, conditionLE_swizzle0Z },
#else
    /*
        TEXBIAS 0, 2, 4
        TEXLDPCF 1, 2, 3
            texld_b_pcf 1, 3, 0, 0, 2
    */
    { 2, gcSL_TEXBIAS, 0, 2, 4 },
    { 1, gcSL_TEXLDPCF, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -1, 0x18, 1, 3, 4, 3, 2, swizzle2ZorW_sample_swizzleX_fix_shadow_coord_swizzle },

    /*
        TEXBIAS 0, 2, 4
        TEXLDPCF 1, 2, 3
            texldb  1, 3, 0, 0, 2
            set.le 1, 1, 3.w
    */
    { 2, gcSL_TEXBIAS, 0, 2, 4 },
    { 1, gcSL_TEXLDPCF, 1, 2, 3 },
        { -4, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0 },
        { -3, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 4, 0, enable_w },
        { -2, 0x19, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2, sample_swizzleX_fix_shadow_coord_swizzle },
        { -1, 0x10, 1, 3, 1, 0, 0, conditionLE_swizzle0ZorW },
    /*
        TEXBIAS 0, 2, 4
        TEXLDPCFPROJ 1, 2, 3
            rcp TEMP1, 0, 0, 3.<last>
            mul TEMP1, 3, TEMP1, 0
            texld_l_pcf 1, TEMP1, 0, 0, 2
    */
    { 2, gcSL_TEXBIAS, 0, 2, 4 },
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -4, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -3, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -2, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 4, 0, enable_w },
        { -1, 0x18, 1, gcSL_CG_TEMP1_XYZW, 4, gcSL_CG_TEMP1_XYZW, 2, swizzle2Z_sample_swizzleX },

    /*
        TEXBIAS 0, 2, 4
        TEXLDPCFPROJ 1, 2, 3
            rcp TEMP1, 0, 0, 3.<last>
            mul TEMP1, 3, TEMP1, 0
            texldb  1, TEMP1, 0, 0, 2
            set.le 1, 1, 3.w
    */
    { 2, gcSL_TEXBIAS, 0, 2, 4 },
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3 },
        { -5, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -4, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -3, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 4, 0, enable_w },
        { -2, 0x19, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2, sample_swizzleX },
        { -1, 0x10, 1, gcSL_CG_TEMP1_XYZW, 1, 0, 0, conditionLE_swizzle0Z },
#endif
    { 0 }
};

/* 0x1B gcSL_TEXGRAD */
const gcsSL_PATTERN patterns_TEXGRAD[] =
{
    /*
        TEXGRAD  0, 4, 5
        TEXLD    1, 2, 3
            add TEMP1, 3, 0, 4, 0
            add TEMP2, 3, 0, 5, 0
            texld_g 1, 3, TEMP1, TEMP2, 2
    */
    { 2, gcSL_TEXGRAD, 0, 4, 5 },
    { 1, gcSL_TEXLD, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -3, 0x01, gcSL_CG_TEMP1, 3, 0, 4, 0 },
        { -2, 0x01, gcSL_CG_TEMP2, 3, 0, 5, 0 },
        { -1, 0x1A, 1, 3, gcSL_CG_TEMP1, gcSL_CG_TEMP2, 2 },

    /*
        TEXGRAD  0, 4, 5
        TEXLDPROJ 1, 2, 3
            rcp TEMP1, 0, 0, 3.<last>
            mul TEMP1, 3, TEMP1, 0
            add TEMP2, 3, 0, 4
            add TEMP3, 3, 0, 5
            texld_g 1, TEMP1, TEMP2, TEMP3, 2
    */
    { 2, gcSL_TEXGRAD, 0, 4, 5 },
    { 1, gcSL_TEXLDPROJ, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -5, 0x0C, gcSL_CG_TEMP1, 0, 0, 3, 0, swizzle2ZorW_from_next_inst },
        { -4, 0x03, gcSL_CG_TEMP1, 3, gcSL_CG_TEMP1, 0, 0 },
        { -3, 0x01, gcSL_CG_TEMP2_XYZW, gcSL_CG_TEMP1, 0, 4, 0 },
        { -2, 0x01, gcSL_CG_TEMP3_XYZW, gcSL_CG_TEMP1, 0, 5, 0 },
        { -1, 0x1A, 1, gcSL_CG_TEMP1, gcSL_CG_TEMP2_XYZW, gcSL_CG_TEMP3_XYZW, 2 },

#if _CLAMP_PCF_REFERENCE_
    /*
        TEXGRAD  0, 4, 5
        TEXLDPCF 1, 2, 3
            mov.sat TEMP1, 3
            add TEMP2, 3, 0, 4, 0
            add TEMP3, 3, 0, 5, 0
            texld_g_pcf 1, TEMP1, TEMP2, TEMP3, 2
    */
    { 2, gcSL_TEXGRAD, 0, 4, 5 },
    { 1, gcSL_TEXLDPCF, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -5, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0 },
        { -4, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_TEMP1_XYZW, 0, enable_w_saturate_swizzle2ZorW_from_next_inst },
        { -3, 0x01, gcSL_CG_TEMP2_XYZW, 3, 0, 4, 0 },
        { -2, 0x01, gcSL_CG_TEMP3_XYZW, 3, 0, 5, 0 },
        { -1, 0x70, 1, gcSL_CG_TEMP1_XYZW, gcSL_CG_TEMP2_XYZW, gcSL_CG_TEMP3_XYZW, 2, sample_swizzleX },
    /*
        TEXGRAD  0, 4, 5
        TEXLDPCFPROJ 1, 2, 3
            rcp TEMP1, 0, 0, 3.<last>
            mul TEMP1, 3, TEMP1, 0
            add TEMP2, 3, 0, 4, 0
            add TEMP3, 3, 0, 5, 0
            texld_g_pcf 1, TEMP1, 0, 0, 2
    */
    { 2, gcSL_TEXGRAD, 0, 4, 5 },
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -6, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -5, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -4, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_TEMP1_XYZW, 0, enable_w_saturate_swizzle2Z },
        { -3, 0x01, gcSL_CG_TEMP2_XYZW, gcSL_CG_TEMP1_XYZW, 0, 4, 0 },
        { -2, 0x01, gcSL_CG_TEMP3_XYZW, gcSL_CG_TEMP1_XYZW, 0, 5, 0 },
        { -1, 0x70, 1, gcSL_CG_TEMP1_XYZW, gcSL_CG_TEMP2_XYZW, gcSL_CG_TEMP3_XYZW, 2, sample_swizzleX },

#else
    /*
        TEXGRAD  0, 4, 5
        TEXLDPCF 1, 2, 3
            add TEMP1, 3, 0, 4, 0
            add TEMP2, 3, 0, 5, 0
            texld_g_pcf 1, 3, 0, 0, 2
    */
    { 2, gcSL_TEXGRAD, 0, 4, 5 },
    { 1, gcSL_TEXLDPCF, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -3, 0x01, gcSL_CG_TEMP1, 3, 0, 4, 0 },
        { -2, 0x01, gcSL_CG_TEMP2, 3, 0, 5, 0 },
        { -1, 0x70, 1, 3, gcSL_CG_TEMP1, gcSL_CG_TEMP2, 2, sample_swizzleX },
    /*
        TEXGRAD  0, 4, 5
        TEXLDPCFPROJ 1, 2, 3
            rcp TEMP1, 0, 0, 3.<last>
            mul TEMP1, 3, TEMP1, 0
            add TEMP2, 3, 0, 4, 0
            add TEMP3, 3, 0, 5, 0
            texld_g_pcf 1, 3, 0, 0, 2
    */
    { 2, gcSL_TEXGRAD, 0, 4, 5 },
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -6, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -5, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -4, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_TEMP1_XYZW, 0, enable_w_swizzle2Z },
        { -3, 0x01, gcSL_CG_TEMP2_XYZW, 3, 0, 4, 0 },
        { -2, 0x01, gcSL_CG_TEMP3_XYZW, 3, 0, 5, 0 },
        { -1, 0x70, 1, gcSL_CG_TEMP1_XYZW, gcSL_CG_TEMP2_XYZW, gcSL_CG_TEMP3_XYZW, 2, sample_swizzleX },
#endif

    /*
        TEXGRAD  0, 4, 5
        TEXLODQ  1, 2, 3
            add TEMP1, 3, 0, 4, 0
            add TEMP2, 3, 0, 5, 0
            lodq_g 1, 3, TEMP1, TEMP2, 2
    */
    { 2, gcSL_TEXGRAD, 0, 4, 5, 0, 0, _hasHalti3 },
    { 1, gcSL_TEXLODQ, 1, 2, 3, 0, 0 },
        { -3, 0x01, gcSL_CG_TEMP1, 3, 0, 4, 0 },
        { -2, 0x01, gcSL_CG_TEMP2, 3, 0, 5, 0 },
        { -1, 0x7C, 1, 3, gcSL_CG_TEMP1, gcSL_CG_TEMP2, 2 },

    { 0 }
};

/* 0x1C gcSL_TEXLOD */
const gcsSL_PATTERN patterns_TEXLOD[] =
{
    /*
        TEXLOD 0, 2, 4
        TEXLD  1, 2, 3
            texld_l_pcf 1, 3, 4, 0, 2
    */
    { 2, gcSL_TEXLOD, 0, 2, 4 },
    { 1, gcSL_TEXLD, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -1, 0x6F, 1, 3, 4, 0, 2 },

    /*
        TEXLOD 0, 2, 4
        TEXLD  1, 2, 3
            mov TEMP1_XYZW, 0, 0, 3, 0
            mov TEMP1_XYZW.w, 0, 0, 4, 0
            texldl 1, TEMP1_XYZW, 0, 0, 2
    */
    { 2, gcSL_TEXLOD, 0, 2, 4 },
    { 1, gcSL_TEXLD, 1, 2, 3 },
        { -3, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0 },
        { -2, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 4, 0, enable_w },
        { -1, 0x1B, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2 },

    /*
        TEXLOD    0, 2, 4
        TEXLDPROJ 1, 2, 3
            rcp TEMP1, 0, 0, 3.<last>
            mul TEMP1, 3, TEMP1, 0
            texld_l_pcf 1, TEMP1, 4, 0, 2
    */
    { 2, gcSL_TEXLOD, 0, 2, 4 },
    { 1, gcSL_TEXLDPROJ, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -3, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, swizzle2ZorW_from_next_inst },
        { -2, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0 },
        { -1, 0x6F, 1, gcSL_CG_TEMP1_XYZW, 4, 0, 2 },

    /*
        TEXLOD    0, 2, 4
        TEXLDPROJ 1, 2, 3
            rcp TEMP1, 0, 0, 3.<last>
            mul TEMP1, 3, TEMP1, 0
            mov TEMP1, 0, 0, 4
            texldl 1, TEMP1, 0, 0, 2
    */
    { 2, gcSL_TEXLOD, 0, 2, 4 },
    { 1, gcSL_TEXLDPROJ, 1, 2, 3 },
        { -4, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, swizzle2ZorW_from_next_inst },
        { -3, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0 },
        { -2, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 4, 0, enable_w },
        { -1, 0x1B, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2 },

#if _CLAMP_PCF_REFERENCE_
    /*
        TEXLOD 0, 2, 4
        TEXLDPCF 1, 2, 3
            texld_l_pcf 1, 3, 4, 0, 2
    */
    { 2, gcSL_TEXLOD, 0, 2, 4 },
    { 1, gcSL_TEXLDPCF, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -2, 0x09, gcSL_CG_TEMP1_X_NO_SRC_SHIFT, 0, 0, 3, 0, saturate_swizzle2ZorW_from_next_inst },
        { -1, 0x6F, 1, 3, 4, gcSL_CG_TEMP1_X_NO_SRC_SHIFT, 2, sample_swizzleX },

    /*
        TEXLOD 0, 2, 4
        TEXLDPCF 1, 2, 3
            mov TEMP1_XYZW, 0, 0, 3, 0
            mov TEMP1_XYZW.w, 0, 0, 4, 0
            texldl  1, TEMP1_XYZW, 0, 0, 2
            set.le 1, 1, 3.w
    */
    { 2, gcSL_TEXLOD, 0, 2, 4 },
    { 1, gcSL_TEXLDPCF, 1, 2, 3 },
        { -5, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0 },
        { -4, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 4, 0, enable_w },
        { -3, 0x09, gcSL_CG_TEMP2_X_NO_SRC_SHIFT, 0, 0, 3, 0, saturate_swizzle2ZorW_from_next_inst },
        { -2, 0x1B, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2, sample_swizzleX },
        { -1, 0x10, 1, gcSL_CG_TEMP2_X_NO_SRC_SHIFT, 1, 0, 0, conditionLE },

    /*
        TEXLOD 0, 2, 4
        TEXLDPCFPROJ 1, 2, 3
            rcp TEMP1, 0, 0, 3.<last>
            mul TEMP1, 3, TEMP1, 0
            texld_l_pcf 1, TEMP1, 0, 0, 2
    */
    { 2, gcSL_TEXLOD, 0, 2, 4 },
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -4, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -3, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -2, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_TEMP1_XYZW, 0, enable_z_saturate_swizzle2Z },
        { -1, 0x6F, 1, gcSL_CG_TEMP1_XYZW, 4, gcSL_CG_TEMP1_XYZW, 2, swizzle2Z_sample_swizzleX },

    /*
        TEXLOD 0, 2, 4
        TEXLDPCFPCFPROJ 1, 2, 3
            rcp TEMP1, 0, 0, 3.<last>
            mul TEMP1, 3, TEMP1, 0
            texldl  1, TEMP1, 0, 0, 2
            set.le 1, 1, 3.w
    */
    { 2, gcSL_TEXLOD, 0, 2, 4 },
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3 },
        { -6, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -5, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -4, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_TEMP1_XYZW, 0, enable_z_saturate_swizzle2Z },
        { -3, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 4, 0, enable_w },
        { -2, 0x1B, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2, sample_swizzleX },
        { -1, 0x10, 1, gcSL_CG_TEMP1_XYZW, 1, 0, 0, conditionLE_swizzle0Z },
#else
    /*
        TEXLOD 0, 2, 4
        TEXLDPCF 1, 2, 3
            texld_l_pcf 1, 3, 0, 0, 2
    */
    { 2, gcSL_TEXLOD, 0, 2, 4 },
    { 1, gcSL_TEXLDPCF, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -1, 0x6F, 1, 3, 4, 3, 2, swizzle2ZorW_sample_swizzleX },

    /*
        TEXLOD 0, 2, 4
        TEXLDPCF 1, 2, 3
            texldl  1, 3, 0, 0, 2
            set.le 1, 1, 3.w
    */
    { 2, gcSL_TEXLOD, 0, 2, 4 },
    { 1, gcSL_TEXLDPCF, 1, 2, 3 },
        { -4, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0 },
        { -3, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 4, 0, enable_w },
        { -2, 0x1B, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2, sample_swizzleX },
        { -1, 0x10, 1, 3, 1, 0, 0, conditionLE_swizzle0ZorW },

    /*
        TEXLOD 0, 2, 4
        TEXLDPCFPROJ 1, 2, 3
            rcp TEMP1, 0, 0, 3.<last>
            mul TEMP1, 3, TEMP1, 0
            texld_l_pcf 1, TEMP1, 0, 0, 2
    */
    { 2, gcSL_TEXLOD, 0, 2, 4 },
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -4, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -3, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -2, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 4, 0, enable_w },
        { -1, 0x6F, 1, gcSL_CG_TEMP1_XYZW, 4, gcSL_CG_TEMP1_XYZW, 2, swizzle2Z_sample_swizzleX },

    /*
        TEXLOD 0, 2, 4
        TEXLDPCFPROJ 1, 2, 3
            rcp TEMP1, 0, 0, 3.<last>
            mul TEMP1, 3, TEMP1, 0
            texldl  1, TEMP1, 0, 0, 2
            set.le 1, 1, 3.w
    */
    { 2, gcSL_TEXLOD, 0, 2, 4 },
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3 },
        { -5, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -4, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -3, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 4, 0, enable_w },
        { -2, 0x1B, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2, sample_swizzleX },
        { -1, 0x10, 1, gcSL_CG_TEMP1_XYZW, 1, 0, 0, conditionLE_swizzle0Z },
#endif

    { 0 }
};

/* 0x1D gcSL_SIN */
const gcsSL_PATTERN patterns_SIN[] =
{
    /*
        SIN 1, 2
            mul 1, 2, rcppi, 0
            sinpi 1, 0, 0, 1
    */
    { 1, gcSL_SIN, 1, 2, 0, 0, 0, _hasNEW_SIN_COS_LOG_DIV },
        { -2, 0x03, gcSL_CG_TEMP1, 2, gcSL_CG_CONSTANT, 0, 0, rcppi },
        { -1, 0x22, 1, 0, 0, gcSL_CG_TEMP1, 0, set_new_sin_cos_log_div },

    /*
        SIN 1, 2
            mul 1, 2, rcppi2, 0
            sinpi 1, 0, 0, 1
    */
    { 1, gcSL_SIN, 1, 2, 0, 0, 0, _hasSQRT_TRIG },
        { -2, 0x03, gcSL_CG_TEMP1, 2, gcSL_CG_CONSTANT, 0, 0, rcppi2_1 },
        { -1, 0x22, 1, 0, 0, gcSL_CG_TEMP1 },

    /*
        SIN 1, 2
            mad TEMP1, 2, rcppi2, dot5
            frc TEMP1, TEMP1
            mad TEMP1, TEMP1, pi2, -pi
            mul TEMP2, TEMP1, TEMP1
            mad TEMP3, TEMP2, factor9, -factor7
            mad TEMP3, TEMP2, TEMP3, factor5
            mad TEMP3, TEMP2, TEMP3, -factor3
            mad TEMP3, TEMP2, TEMP3, one
            mul 1, TEMP3, TEMP1
    */
    { 1, gcSL_SIN, 1, 2 },
        { -9, 0x02, gcSL_CG_TEMP1, 2, gcSL_CG_CONSTANT, gcSL_CG_CONSTANT, 0, rcppi2_1_dot5_2 },
        { -8, 0x13, gcSL_CG_TEMP1, 0, 0, gcSL_CG_TEMP1, 0 },
        { -7, 0x02, gcSL_CG_TEMP1, gcSL_CG_TEMP1, gcSL_CG_CONSTANT, -gcSL_CG_CONSTANT, 0, pi2_1_pi_2 },
        { -6, 0x03, gcSL_CG_TEMP2, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, 0 },
        { -5, 0x02, gcSL_CG_TEMP3, gcSL_CG_TEMP2, gcSL_CG_CONSTANT, -gcSL_CG_CONSTANT, 0, sin_factor9_1_factor7_2 },
        { -4, 0x02, gcSL_CG_TEMP3, gcSL_CG_TEMP2, gcSL_CG_TEMP3, gcSL_CG_CONSTANT, 0, sin_factor5_2 },
        { -3, 0x02, gcSL_CG_TEMP3, gcSL_CG_TEMP2, gcSL_CG_TEMP3, -gcSL_CG_CONSTANT, 0, sin_factor3_2 },
        { -2, 0x02, gcSL_CG_TEMP3, gcSL_CG_TEMP2, gcSL_CG_TEMP3, gcSL_CG_CONSTANT, 0, sin_one_2 },
        { -1, 0x03, 1, gcSL_CG_TEMP3, gcSL_CG_TEMP1, 0, 0 },

    { 0 }
};

/* 0x1E gcSL_COS */
const gcsSL_PATTERN patterns_COS[] =
{
    /*
        COS 1, 2
            mul 1, 2, rcppi, 0
            cospi 1, 0, 0, 1
    */
    { 1, gcSL_COS, 1, 2, 0, 0, 0, _hasNEW_SIN_COS_LOG_DIV },
        { -2, 0x03, gcSL_CG_TEMP1, 2, gcSL_CG_CONSTANT, 0, 0, rcppi },
        { -1, 0x23, 1, 0, 0, gcSL_CG_TEMP1, 0, set_new_sin_cos_log_div },

    /*
        COS 1, 2
            mul 1, 2, rcppi2, 0
            cospi 1, 0, 0, 1
    */
    { 1, gcSL_COS, 1, 2, 0, 0, 0, _hasSQRT_TRIG },
        { -2, 0x03, gcSL_CG_TEMP1, 2, gcSL_CG_CONSTANT, 0, 0, rcppi2_1 },
        { -1, 0x23, 1, 0, 0, gcSL_CG_TEMP1 },

    /*
        COS 1, 2
            mad TEMP1, 2, rcppi2, dot5
            frc TEMP1, TEMP1
            mad TEMP1, TEMP1, pi2, -pi
            mul TEMP1, TEMP1, TEMP1
            mad 1, TEMP1, factor8, -factor6
            mad 1, TEMP1, 1, factor4
            mad 1, TEMP1, 1, -factor2
            mad 1, TEMP1, 1, one
    */
    { 1, gcSL_COS, 1, 2 },
        { -8, 0x02, gcSL_CG_TEMP1, 2, gcSL_CG_CONSTANT, gcSL_CG_CONSTANT, 0, rcppi2_1_dot5_2 },
        { -7, 0x13, gcSL_CG_TEMP1, 0, 0, gcSL_CG_TEMP1, 0 },
        { -6, 0x02, gcSL_CG_TEMP1, gcSL_CG_TEMP1, gcSL_CG_CONSTANT, -gcSL_CG_CONSTANT, 0, pi2_1_pi_2 },
        { -5, 0x03, gcSL_CG_TEMP1, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, 0 },
        { -4, 0x02, 1, gcSL_CG_TEMP1, gcSL_CG_CONSTANT, -gcSL_CG_CONSTANT, 0, cos_factor8_1_factor6_2 },
        { -3, 0x02, 1, gcSL_CG_TEMP1, 1, gcSL_CG_CONSTANT, 0, cos_factor4_2 },
        { -2, 0x02, 1, gcSL_CG_TEMP1, 1, -gcSL_CG_CONSTANT, 0, cos_factor2_2 },
        { -1, 0x02, 1, gcSL_CG_TEMP1, 1, gcSL_CG_CONSTANT, 0, one_2 },

    { 0 }
};

/* 0x1F gcSL_TAN */
const gcsSL_PATTERN patterns_TAN[] =
{
    /*
        TAN 1, 2
            mul temp1, 2, rcppi, 0
            sinpi temp2, 0, 0, temp1
            cospi temp3, 0, 0, temp1
            div 1, temp2, temp3 // tan(x) = sin(x)/cos(x)
    */
    { 1, gcSL_TAN, 1, 2, 0, 0, 0, _hasNEW_SIN_COS_LOG_DIV },
        { -4, 0x03, gcSL_CG_TEMP1, 2, gcSL_CG_CONSTANT, 0, 0, rcppi },
        { -3, 0x22, gcSL_CG_TEMP2, 0, 0, gcSL_CG_TEMP1, 0, set_new_sin_cos_log_div },
        { -2, 0x23, gcSL_CG_TEMP3, 0, 0, gcSL_CG_TEMP1, 0, set_new_sin_cos_log_div },
        { -1, 0x64, 1, 0, gcSL_CG_TEMP2, gcSL_CG_TEMP3, 0, set_new_sin_cos_log_div },

    /*
        TAN 1, 2
            mul temp1, 2, rcppi2, 0
            cospi temp2, 0, 0, temp1
            sinpi temp1, 0, 0, temp1
            rcp temp2, temp2
            mul 1, temp2, temp1 // tan(x) = sin(x)/cos(x)
    */
    { 1, gcSL_TAN, 1, 2, 0, 0, 0, _hasSQRT_TRIG },
        { -5, 0x03, gcSL_CG_TEMP1, 2, gcSL_CG_CONSTANT, 0, 0, rcppi2_1 },
        { -4, 0x23, gcSL_CG_TEMP2, 0, 0, gcSL_CG_TEMP1},
        { -3, 0x22, gcSL_CG_TEMP1, 0, 0, gcSL_CG_TEMP1},
        { -2, 0x0C, gcSL_CG_TEMP2, 0, 0, gcSL_CG_TEMP2, 0, },
        { -1, 0x03, 1, gcSL_CG_TEMP1, gcSL_CG_TEMP2, 0, 0, },

    /*
        TAN 1, 2
            mad TEMP1, 2, rcppi2, dot5
            frc TEMP1, TEMP1
            mad TEMP1, TEMP1, pi2, -pi

            tan(x) = x + 1/3 x^3 + 2/15 x^5 + 17/315 x^7 + 62/2835 x^9
                   = x (1 + x^2 (1/3 + x^2 (2/15 + x^2 (17/315 + 62/2835 x^2 ) ) ) )

            mul TEMP2, TEMP1, TEMP1
            mad 1, TEMP2, tan9, tan7
            mad 1, TEMP2, 1, tan5
            mad 1, TEMP2, 1, tan3
            mad 1, TEMP2, 1, one
            mul 1, TEMP1, 1
    */
    { 1, gcSL_TAN, 1, 2 },
        { -9, 0x02, gcSL_CG_TEMP1, 2, gcSL_CG_CONSTANT, gcSL_CG_CONSTANT, 0, rcppi2_1_dot5_2 },
        { -8, 0x13, gcSL_CG_TEMP1, 0, 0, gcSL_CG_TEMP1, 0 },
        { -7, 0x02, gcSL_CG_TEMP1, gcSL_CG_TEMP1, gcSL_CG_CONSTANT, -gcSL_CG_CONSTANT, 0, pi2_1_pi_2 },
        { -6, 0x03, gcSL_CG_TEMP2, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, 0 },
        { -5, 0x02, gcSL_CG_TEMP3, gcSL_CG_TEMP2, gcSL_CG_CONSTANT, gcSL_CG_CONSTANT, 0, tan9_1_tan7_2 },
        { -4, 0x02, gcSL_CG_TEMP3, gcSL_CG_TEMP2, gcSL_CG_TEMP3, gcSL_CG_CONSTANT, 0, tan5_2 },
        { -3, 0x02, gcSL_CG_TEMP3, gcSL_CG_TEMP2, gcSL_CG_TEMP3, gcSL_CG_CONSTANT, 0, tan3_2 },
        { -2, 0x02, gcSL_CG_TEMP3, gcSL_CG_TEMP2, gcSL_CG_TEMP3, gcSL_CG_CONSTANT, 0, one_2 },
        { -1, 0x03, 1, gcSL_CG_TEMP1, gcSL_CG_TEMP3, 0, 0, },

    { 0 }
};

/* 0x20 gcSL_EXP */
const gcsSL_PATTERN patterns_EXP[] =
{
    /*
        EXP 1, 2
            exp 1, 0, 0, 2, 0
    */
    { 1, gcSL_EXP, 1, 2 },
        { -1, 0x11, 1, 0, 0, 2, 0 },

    { 0 }
};

/* 0x21 gcSL_SIGN */
const gcsSL_PATTERN patterns_SIGN[] =
{
    /*  if the int(v) pattern is only used in indexing, we can
        safely assume v is alway positive, soit can be simplified
        as FLOOR(v):

        SIGN   1, 2
        ABS    3, 2
        FLOOR  4, 3
        MUL    5, 1, 4
            floor   5, 2
    */
    { 4, gcSL_SIGN, 1, 2, 0, 0, 0, _hasSIGN_FLOOR_CEIL },
    { 3, gcSL_ABS, 3, 2, 0, 0, 0, _UseDestInNextOnly },
    { 2, gcSL_FLOOR, 4, 3, 0, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_MUL, 5, 1, 4, 0, 0, _UsedAsIndexingOnly },
        { -1, 0x25, 5, 0, 0, 2, 0 },

    /*
        SIGN   1, 2
        FLOOR  3, 4         //4 is Abs(2)
        MUL    5, 1, 3
            floor   5, 2
    */
    { 3, gcSL_SIGN, 1, 2, 0, 0, 0, _hasSIGN_FLOOR_CEIL },
    { 2, gcSL_FLOOR, 3, 4, 0, 0, 0, _UseDestInNextOnly_AbsOnSrc0_SameSrc0AsSrc0InPrev },
    { 1, gcSL_MUL, 5, 1, 3, 0, 0, _UsedAsIndexingOnly },
        { -1, 0x25, 5, 0, 0, 2, 0 },

    /*
        SIGN 1, 2
            sign 1, 0, 0, 2
    */
    { 1, gcSL_SIGN, 1, 2, 0, 0, 0, _hasSIGN_FLOOR_CEIL_and_float_type },
       /* { -1, 0x27, 1, 0, 0, 2 },*/
        { -1, 0x27, 1, 0, 0, 2, 0, value_type0 },

    /*
        SIGN 1, 2
            select.gz 1, 2, one, 2
            select.eq 1, 1, zero, 1
            select.lz 1, 1, -one, 2
    */
    { 1, gcSL_SIGN, 1, 2, 0, 0, 0, _is_value_type_float },
        { -3, 0x0F, 1, 2, gcSL_CG_CONSTANT, 2, 0, one_1_conditionGZ },
        { -2, 0x0F, 1, 2, gcSL_CG_CONSTANT, 1, 0, zero_1_conditionEQ },
        { -1, 0x0F, 1, 2, -gcSL_CG_CONSTANT, 1, 0, one_1_conditionLZ },

    /*
        SIGN 1, 2
            select.gz 1, 2, one, 2
            select.eq 1, 1, zero, 1
            select.lz 1, 1, -one, 2
    */
    { 1, gcSL_SIGN, 1, 2 },
        { -3, 0x0F, 1, 2, gcSL_CG_CONSTANT, 2, 0, int_one_1_conditionGZ },
        { -2, 0x0F, 1, 2, gcSL_CG_CONSTANT, 1, 0, int_zero_1_conditionEQ },
        { -1, 0x0F, 1, 2, gcSL_CG_CONSTANT, 1, 0, int_minus_one_1_conditionLZ },

    { 0 }
};

/* 0x22 gcSL_STEP */
const gcsSL_PATTERN patterns_STEP[] =
{
    /*
        STEP 1, 2, 3
            set.ge 1, 3, 2, 0, 0
    */
    { 1, gcSL_STEP, 1, 2, 3 },
        { -1, 0x10, 1, 3, 2, 0, 0, conditionGE },

    { 0 }
};

/* 0x23 gcSL_SQRT */
const gcsSL_PATTERN patterns_SQRT[] =
{
    /*
        SQRT 1, 2
        RCP  3, 1
            rsq 3, 0, 0, 2, 0
    */
    { 2, gcSL_SQRT, 1, 2, 0, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_RCP, 3, 1 },
            { -1, 0x0D, 3, 0, 0, 2, 0 },

    /*
        SQRT 1, 2
            sqrt 1, 0, 0, 2, 0
    */
    { 1, gcSL_SQRT, 1, 2, 0, 0, 0, _hasSQRT_TRIG },
        { -1, 0x21, 1, 0, 0, 2, 0 },

    /*
        SQRT 1, 2
            rsq 1, 0, 0, 2, 0
            rcp 1, 0, 0, 1, 0
    */
    { 1, gcSL_SQRT, 1, 2 },
        { -2, 0x0D, 1, 0, 0, 2, 0 },
        { -1, 0x0C, 1, 0, 0, 1, 0 },

    { 0 }
};

/* 0x24 gcSL_ACOS */
const gcsSL_PATTERN patterns_ACOS[] =
{
    /*
        ACOS 1, 2
            ACOS(x) = 1/2 pi - (x + 1/6 x^3 + 3/40 x^5 + 5/112 x^7 + 35/1152 x^9)
                    = 1/2 pi - (x (1 + x^2 (1/6 + x^2 (3/40 + x^2 (5/112 + 35/1152 x^2)))))

            mul TEMP1, 2, 2
            mad 1, TEMP1, asin9, asin7
            mad 1, TEMP1, 1, asin5
            mad 1, TEMP1, 1, asin3
            mad 1, TEMP1, 1, one
            mad 1, 2, -1, half_pi
    */
    { 1, gcSL_ACOS, 1, 2 },
        { -6, 0x03, gcSL_CG_TEMP1, 2, 2, 0, 0 },
        { -5, 0x02, gcSL_CG_TEMP2, gcSL_CG_TEMP1, gcSL_CG_CONSTANT, gcSL_CG_CONSTANT, 0, asin9_1_asin7_2 },
        { -4, 0x02, gcSL_CG_TEMP2, gcSL_CG_TEMP1, gcSL_CG_TEMP2, gcSL_CG_CONSTANT, 0, asin5_2 },
        { -3, 0x02, gcSL_CG_TEMP2, gcSL_CG_TEMP1, gcSL_CG_TEMP2, gcSL_CG_CONSTANT, 0, asin3_2 },
        { -2, 0x02, gcSL_CG_TEMP2, gcSL_CG_TEMP1, gcSL_CG_TEMP2, gcSL_CG_CONSTANT, 0, one_2 },
        { -1, 0x02, 1, 2, -gcSL_CG_TEMP2, gcSL_CG_CONSTANT, 0, half_pi_2 },

    { 0 }
};

/* 0x25 gcSL_ASIN */
const gcsSL_PATTERN patterns_ASIN[] =
{
    /*
        ASIN 1, 2
            ASIN(x) = x + 1/6 x^3 + 3/40 x^5 + 5/112 x^7 + 35/1152 x^9
                    = x (1 + x^2 (1/6 + x^2 (3/40 + x^2 (5/112 + 35/1152 x^2))))

            mul TEMP1, 2, 2
            mad 1, TEMP1, asin9, asin7
            mad 1, TEMP1, 1, asin5
            mad 1, TEMP1, 1, asin3
            mad 1, TEMP1, 1, one
            mul 1, 2, 1
    */
    { 1, gcSL_ASIN, 1, 2 },
        { -6, 0x03, gcSL_CG_TEMP1, 2, 2, 0, 0 },
        { -5, 0x02, gcSL_CG_TEMP2, gcSL_CG_TEMP1, gcSL_CG_CONSTANT, gcSL_CG_CONSTANT, 0, asin9_1_asin7_2 },
        { -4, 0x02, gcSL_CG_TEMP2, gcSL_CG_TEMP1, gcSL_CG_TEMP2, gcSL_CG_CONSTANT, 0, asin5_2 },
        { -3, 0x02, gcSL_CG_TEMP2, gcSL_CG_TEMP1, gcSL_CG_TEMP2, gcSL_CG_CONSTANT, 0, asin3_2 },
        { -2, 0x02, gcSL_CG_TEMP2, gcSL_CG_TEMP1, gcSL_CG_TEMP2, gcSL_CG_CONSTANT, 0, one_2 },
        { -1, 0x03, 1, 2, gcSL_CG_TEMP2, 0, 0 },

    { 0 }
};

/* 0x26 gcSL_ATAN */
const gcsSL_PATTERN patterns_ATAN[] =
{
    /*
        ATAN 1, 2

            if (|x| > 1) flag = 1; x = 1 / x; else flag = 0;

                set.gt TEMP1, |x|, 1
                rcp TEMP2, x
                select.nz TEMP2, TEMP1, TEMP2, x

            atan(x) = x - 1/3 x^3 + 1/5 x^5 - 1/7 x^7 + 1/9 x^9
                    = x (1 + x^2 (-1/3 + x^2 (1/5 + x^2 (-1/7 + 1/9 x^2 ) ) ) )

                mul TEMP3, TEMP2, TEMP2
                mad 1, TEMP3, atan9, -atan7
                mad 1, TEMP3, 1, atan5
                mad 1, TEMP3, 1, -atan3
                mad 1, TEMP3, 1, one
                mul 1, TEMP2, 1, 0

            if (x < 0) t2 = -pi/2 - abs(atan); else t2 = pi/2 - abs(atan);

                add TEMP2, PI/2, 0, |atan|
                select.lt TEMP2, x, -TEMP2, TEMP2

            return flag ? t2 : atan;

                select.nz 1, TEMP1, TEMP2, 1

    */
    { 1, gcSL_ATAN, 1, 2 },
        { -12, 0x10, gcSL_CG_TEMP1, 2, gcSL_CG_CONSTANT, 0, 0, gt_abs_0_one_1 },
        { -11, 0x0C, gcSL_CG_TEMP2, 0, 0, 2, 0, },
        { -10, 0x0F, gcSL_CG_TEMP2, gcSL_CG_TEMP1, gcSL_CG_TEMP2, 2, 0, conditionNZ },
        { -9, 0x03, gcSL_CG_TEMP3, gcSL_CG_TEMP2, gcSL_CG_TEMP2, 0, 0, },
        { -8, 0x02, 1, gcSL_CG_TEMP3, gcSL_CG_CONSTANT, -gcSL_CG_CONSTANT, 0, atan9_1_atan7_2 },
        { -7, 0x02, 1, gcSL_CG_TEMP3, 1, gcSL_CG_CONSTANT, 0, atan5_2 },
        { -6, 0x02, 1, gcSL_CG_TEMP3, 1, -gcSL_CG_CONSTANT, 0, atan3_2 },
        { -5, 0x02, 1, gcSL_CG_TEMP3, 1, gcSL_CG_CONSTANT, 0, one_2 },
        { -4, 0x03, 1, gcSL_CG_TEMP2, 1, 0, 0, },
        { -3, 0x01, gcSL_CG_TEMP2, gcSL_CG_CONSTANT, 0, -1, 0, halfpi_0_abs_2 },
        { -2, 0x0F, gcSL_CG_TEMP2, 2, -gcSL_CG_TEMP2, gcSL_CG_TEMP2, 0, conditionLT },
        { -1, 0x0F, 1, gcSL_CG_TEMP1, gcSL_CG_TEMP2, 1, 0, conditionNZ },

    { 0 }
};

/* 0x27 gcSL_SET */
const gcsSL_PATTERN patterns_SET[] =
{
     /*
        SET 1, 2, 3
        CMP.z  4, 1, 4
        CMP.nz 4, 1, 3
            select 4, 2, 3, 4
    */
    { 3, gcSL_SET, 1, 2, 3, 0, 0, _UseDestInNextTwoOnly },
    { 2, gcSL_CMP, 4, 1, 4, 0, 0, _IsZ },
    { 1, gcSL_CMP, 4, 1, 3, 0, 0, _IsNZ },
        { -1, 0x0F, 4, 2, 3, 4, 0, value_type0_from_src0 },

     /*
        SET 1, 2, 3
        CMP.z  4, 1, 2
        CMP.nz 4, 1, 3
            select 4, 2, 3, 2
    */
    { 3, gcSL_SET, 1, 2, 3, 0, 0, _UseDestInNextTwoOnly },
    { 2, gcSL_CMP, 4, 1, 2, 0, 0, _IsZ },
    { 1, gcSL_CMP, 4, 1, 3, 0, 0, _IsNZ },
        { -1, 0x0F, 4, 2, 3, 2, 0, value_type0_from_src0 },

     /*
        SET 1, 2, 3
        CMP.z  4, 1, 5
        CMP.nz 4, 1, 3
            select 4, 2, 3, 5
    */
    { 3, gcSL_SET, 1, 2, 3, 0, 0, _UseDestInNextTwoOnly },
    { 2, gcSL_CMP, 4, 1, 5, 0, 0, _IsZ },
    { 1, gcSL_CMP, 4, 1, 3, 0, 0, _IsNZ },
        { -1, 0x0F, 4, 2, 3, 5, 0, value_type0_from_src0 },

    /*
        SET.z  1, 2, 4
        SET.nz 1, 2, 3
            select.nz 1, 2, 3, 4, 0
    */
    { 2, gcSL_SET, 0, 2, 4, 0, 0, _IsZ },
    { 1, gcSL_SET, 1, 2, 3, 0, 0, _IsNZ },
        { -1, 0x0F, 1, 2, 3, 4, 0, conditionNZ },

    /*
        SET.cond 1, 2, 3
            set.cond 1, 2, 3, 0, 0
    */
    { 1, gcSL_SET, 1, 2, 3 },
        { -1, 0x10, 1, 2, 3, 0, 0, _set_helper_or_not},

    { 0 }
};

/* 0x28 gcSL_DSX */
const gcsSL_PATTERN patterns_DSX[] =
{
    /*
        DSX 1, 2
            dsx 1, 2, 0, 2, 0
    */
    { 1, gcSL_DSX, 1, 2 },
        { -1, 0x07, 1, 2, 0, 2, 0 },

    { 0 }
};

/* 0x29 gcSL_DSY */
const gcsSL_PATTERN patterns_DSY[] =
{
    /*
        DSY 1, 2
            dsy 1, 2, 0, 2, 0
    */
    { 1, gcSL_DSY, 1, 2 },
        { -1, 0x08, 1, 2, 0, 2, 0 },

    { 0 }
};

/* 0x2A gcSL_FWIDTH */
const gcsSL_PATTERN patterns_FWIDTH[] =
{
    /*
        FWIDTH 1, 2
            dsx TEMP1, 2, 0, 2, 0
            dsy TEMP2, 2, 0, 2, 0
            add 1, |TEMP1|, 0, |TEMP2|, 0
    */
    { 1, gcSL_FWIDTH, 1, 2 },
        { -3, 0x07, gcSL_CG_TEMP1, 2, 0, 2, 0 },
        { -2, 0x08, gcSL_CG_TEMP2, 2, 0, 2, 0 },
        { -1, 0x01, 1, gcSL_CG_TEMP1, 0, gcSL_CG_TEMP2, 0, abs_0_abs_2 },

    { 0 }
};

/* 0x2B gcSL_DIV */
const gcsSL_PATTERN patterns_DIV[] =
{
    /*
        DIV 1, 2, 3
            idiv 1, 2, 3, 0, 0
    */
    { 1, gcSL_DIV, 1, 2, 3, 0, 0, _IntOpcode },
        { -1, 0x44, 1, 2, 3, 0, 0, value_type0 },

    /*
        DIV 1, 2, 3
            div 3, 0, 0, 2, 0
    */
    { 1, gcSL_DIV, 1, 2, 3, 0, 0, _hasNEW_SIN_COS_LOG_DIV },
            { -1, 0x64, 1, 0, 2, 3, 0, set_new_sin_cos_log_div },

    /*
        DIV 1, 2, 3
            rcp TEMP1, 0, 0, 3
            mul 1, 2, TEMP1, 0
    */
    { 1, gcSL_DIV, 1, 2, 3 },
        { -2, 0x0C, gcSL_CG_TEMP1, 0, 0, 3, 0, value_type0_from_src0 },
        { -1, 0x03, 1, 2, gcSL_CG_TEMP1, 0, 0, value_type0 },

    { 0 }
};

/* 0x2C gcSL_MOD */
const gcsSL_PATTERN patterns_MOD[] =
{
    /*
        MOD 1, 2, 3
            mod 1, 2, 3, 0, 0
    */
    { 1, gcSL_MOD, 1, 2, 3 },
        { -1, 0x48, 1, 2, 3, 0, 0, value_type0 },

    { 0 }
};

/* 0x2D gcSL_AND_BITWISE */
const gcsSL_PATTERN patterns_AND_BITWISE[] =
{
#if _SUPPORT_LONG_ULONG_DATA_TYPE
    /*
        gcSL_AND_BITWISE 1, 2, 3
            and 1, 2, 0, 3, 0
    */
    { 1, gcSL_AND_BITWISE, 1, 2, 3, 0, 0, _hasInteger_long_ulong },
        { -2, 0x5D, 1, 2, 0, 3, 0, long_ulong_first_logical_op },
        { -1, 0x5D, 1, 2, 0, 3, 0, long_ulong_second_logical_op },
#endif
    /*
        gcSL_AND_BITWISE 1, 2, 3
            and 1, 2, 0, 3, 0
    */
    { 1, gcSL_AND_BITWISE, 1, 2, 3, 0, 0, _hasInteger },
        { -1, 0x5D, 1, 2, 0, 3, 0, value_type0 },

    { 0 }
};

/* 0x2E gcSL_OR_BITWISE */
const gcsSL_PATTERN patterns_OR_BITWISE[] =
{
#if _SUPPORT_LONG_ULONG_DATA_TYPE
    /*
        gcSL_OR_BITWISE 1, 2, 3
            or 1, 2, 0, 3, 0
    */
    { 1, gcSL_OR_BITWISE, 1, 2, 3, 0, 0, _hasInteger_long_ulong },
        { -2, 0x5C, 1, 2, 0, 3, 0, long_ulong_first_logical_op },
        { -1, 0x5C, 1, 2, 0, 3, 0, long_ulong_second_logical_op },
#endif
    /*
        gcSL_OR_BITWISE 1, 2, 3
            or 1, 2, 0, 3, 0
    */
    { 1, gcSL_OR_BITWISE, 1, 2, 3, 0, 0, _hasInteger },
        { -1, 0x5C, 1, 2, 0, 3, 0, value_type0 },

    { 0 }
};

/* 0x2F gcSL_XOR_BITWISE */
const gcsSL_PATTERN patterns_XOR_BITWISE[] =
{
#if _SUPPORT_LONG_ULONG_DATA_TYPE
    /*
        gcSL_XOR_BITWISE 1, 2, 3
            xor 1, 2, 0, 3, 0
    */
    { 1, gcSL_XOR_BITWISE, 1, 2, 3, 0, 0, _hasInteger_long_ulong },
        { -2, 0x5E, 1, 2, 0, 3, 0, long_ulong_first_logical_op },
        { -1, 0x5E, 1, 2, 0, 3, 0, long_ulong_second_logical_op },
#endif
    /*
        gcSL_XOR_BITWISE 1, 2, 3
            xor 1, 2, 0, 3, 0
    */
    { 1, gcSL_XOR_BITWISE, 1, 2, 3, 0, 0, _hasInteger },
        { -1, 0x5E, 1, 2, 0, 3, 0, value_type0 },

    { 0 }
};

/* 0x30 gcSL_NOT_BITWISE */
const gcsSL_PATTERN patterns_NOT_BITWISE[] =
{
#if _SUPPORT_LONG_ULONG_DATA_TYPE
    /*
        gcSL_NOT_BITWISE 1, 2
            not 1, 0, 0, 2, 0
    */
    { 1, gcSL_NOT_BITWISE, 1, 2, 0, 0, 0, _hasInteger_long_ulong },
        { -2, 0x5F, 1, 0, 0, 2, 0, long_ulong_first_logical_not_op },
        { -1, 0x5F, 1, 0, 0, 2, 0, long_ulong_second_logical_not_op },
#endif
    /*
        gcSL_NOT_BITWISE 1, 2
            not 1, 0, 0, 2, 0
    */
    { 1, gcSL_NOT_BITWISE, 1, 2, 0, 0, 0, _hasInteger },
        { -1, 0x5F, 1, 0, 0, 2, 0, value_type0 },

    { 0 }
};

/* 0x31 gcSL_LSHIFT */
const gcsSL_PATTERN patterns_LSHIFT[] =
{
    /*
        LSHIFT 1, 2, 3
            lshift 1, 2, 0, 3, 0
    */
    { 1, gcSL_LSHIFT, 1, 2, 3, 0, 0, _hasInteger },
        { -1, 0x59, 1, 2, 0, 3, 0, value_type0 },

    { 0 }
};

/* 0x32 gcSL_RSHIFT */
const gcsSL_PATTERN patterns_RSHIFT[] =
{
    /*
        RSHIFT 1, 2, 3
            rshift 1, 2, 0, 3, 0
    */
    { 1, gcSL_RSHIFT, 1, 2, 3, 0, 0, _hasInteger },
        { -1, 0x5A, 1, 2, 0, 3, 0, value_type0 },

    { 0 }
};

/* 0x33 gcSL_ROTATE */
const gcsSL_PATTERN patterns_ROTATE[] =
{
    /*
        ROTATE 1, 2, 3
            rotate 1, 2, 0, 3, 0
    */
    { 1, gcSL_ROTATE, 1, 2, 3, 0, 0, _hasInteger },
        { -1, 0x5B, 1, 2, 0, 3, 0, value_type0 },

    { 0 }
};

/* 0x34 gcSL_BITSEL */
const gcsSL_PATTERN patterns_BITSEL[] =
{
    { 0 }
};

/* 0x35 gcSL_LEADZERO */
const gcsSL_PATTERN patterns_LEADZERO[] =
{
    /*
        LEADZERO 1, 2, 3
            leadzero 1, 2, 3, 0, 0
    */
    { 1, gcSL_LEADZERO, 1, 2, 0, 0, 0, _IntOpcode },
        { -1, 0x58, 1, 0, 0, 2, 0, value_type0 },

    { 0 }
};

/* 0x36 gcSL_LOAD */
const gcsSL_PATTERN patterns_LOAD[] =
{
    /*
        LOAD 1, 2, 3
        ATOMADD 4, 1, 5
            atom_add 4, 2, 3, 5, 0
    */
    { 2, gcSL_LOAD, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_ATOMADD, 4, 1, 5, 0, 0, _NoLabel },
        { -1, 0x65, 4, 2, 3, 5, 0, value_type0 },

    /*
        LOAD 1, 2, 3
        ATOMSUB 4, 1, 5
            atom_add 4, 2, 3, -5, 0
    */
    { 2, gcSL_LOAD, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_ATOMSUB, 4, 1, 5, 0, 0, _NoLabel },
        { -1, 0x65, 4, 2, 3, -5, 0, value_type0 },

    /*
        LOAD 1, 2, 3
        ATOMXCHG 4, 1, 5
            atom_xchg 4, 2, 3, 5, 0
    */
    { 2, gcSL_LOAD, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_ATOMXCHG, 4, 1, 5, 0, 0, _NoLabel },
        { -1, 0x66, 4, 2, 3, 5, 0, value_type0 },

    /*
        LOAD 1, 2, 3
        ATOMMIN 4, 1, 5
            atom_min 4, 2, 3, 5, 0
    */
    { 2, gcSL_LOAD, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_ATOMMIN, 4, 1, 5, 0, 0, _NoLabel },
        { -1, 0x68, 4, 2, 3, 5, 0, value_type0 },

    /*
        LOAD 1, 2, 3
        ATOMMAX 4, 1, 5
            atom_max 4, 2, 3, 5, 0
    */
    { 2, gcSL_LOAD, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_ATOMMAX, 4, 1, 5, 0, 0, _NoLabel },
        { -1, 0x69, 4, 2, 3, 5, 0, value_type0 },

    /*
        LOAD 1, 2, 3
        ATOMOR 4, 1, 5
            atom_or 4, 2, 3, 5, 0
    */
    { 2, gcSL_LOAD, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_ATOMOR, 4, 1, 5, 0, 0, _NoLabel },
        { -1, 0x6A, 4, 2, 3, 5, 0, value_type0 },

    /*
        LOAD 1, 2, 3
        ATOMAND 4, 1, 5
            atom_and 4, 2, 3, 5, 0
    */
    { 2, gcSL_LOAD, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_ATOMAND, 4, 1, 5, 0, 0, _NoLabel },
        { -1, 0x6B, 4, 2, 3, 5, 0, value_type0 },

    /*
        LOAD 1, 2, 3
        ATOMXOR 4, 1, 5
            atom_xor 4, 2, 3, 5, 0
    */
    { 2, gcSL_LOAD, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_ATOMXOR, 4, 1, 5, 0, 0, _NoLabel },
        { -1, 0x6C, 4, 2, 3, 5, 0, value_type0 },

    /*
        LOAD 1, 2, 3
        ATOMCMPXCHG 4, 1, 5
            atom_cmp_xchg 4, 2, 3, 5, 0
    */
    { 2, gcSL_LOAD, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_ATOMCMPXCHG, 4, 1, 5, 0, 0, _NoLabel },
        { -1, 0x67, 4, 2, 3, 5, 0, value_type0 },
#if _SUPPORT_LONG_ULONG_DATA_TYPE
    /*
        LOAD 1, 2, 3
            load TEMP1, 2, 3, 0, 0
            mov 1, 0, 0, TEMP1.xyzw, 0
            mov 1, 0, 0, TEMP1.xyzw, 0
    */
    { 1, gcSL_LOAD, 1, 2, 3, 0, 0, _isCL_Long_ulong_one_load_two_moves},
        { -3, 0x32, gcSL_CG_TEMP1_XYZW, 2, 3, 0, 0, long_ulong_first_load_to_temp },
        { -2, 0x09, 1, 0, 0, gcSL_CG_TEMP1_XYZW, 0, long_ulong_first_load_mov },
        { -1, 0x09, 1, 0, 0, gcSL_CG_TEMP1_XYZW, 0, long_ulong_second_load_mov },

    /*
        LOAD 1, 2, 3
            load TEMP1, 2, 3, 0, 0
            mov 1, 0, 0, TEMP1.xyzw, 0
            mov 1, 0, 0, TEMP1.xyzw, 0
            add gcSL_CG_TEMP2, 3, constant(16)
            load TEMP1, 2, TEMP2, 0, 0
            mov 1, 0, 0, TEMP1.xyzw, 0
            mov 1, 0, 0, TEMP1.xyzw, 0
    */
    { 1, gcSL_LOAD, 1, 2, 3, 0, 0, _isCL_Long_ulong_two_load_four_moves},
        { -7, 0x32, gcSL_CG_TEMP1_XYZW, 2, 3, 0, 0, long_ulong_first_load_to_temp },
        { -6, 0x09, 1, 0, 0, gcSL_CG_TEMP1_XYZW, 0, long_ulong_first_load_mov },
        { -5, 0x09, 1, 0, 0, gcSL_CG_TEMP1_XYZW, 0, long_ulong_second_load_mov },
        { -4, 0x01, gcSL_CG_TEMP2_X, 3, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_16 },
        { -3, 0x32, gcSL_CG_TEMP1_XYZW, 2, gcSL_CG_TEMP2_X, 0, 0, long_ulong_second_load_to_temp },
        { -2, 0x09, 1, 0, 0, gcSL_CG_TEMP1_XYZW, 0, long_ulong_third_load_mov },
        { -1, 0x09, 1, 0, 0, gcSL_CG_TEMP1_XYZW, 0, long_ulong_fourth_load_mov },
#endif
    /*
        LOAD 1, 2, 3
            load 1, 2, 3, 0, 0
    */
    { 1, gcSL_LOAD, 1, 2, 3, 0, 0, _hasInteger },
        { -1, 0x32, 1, 2, 3, 0, 0, denorm_value_type0 },

    { 0 }
};

/* 0x37 gcSL_STORE */
const gcsSL_PATTERN patterns_STORE[] =
{
    /*
        STORE 1, 2, 3
            store 0, 2, 3, 1, 0
    */
    { 1, gcSL_STORE, 1, 2, 3, 0, 0, _isCL_X_Signed_8_16_store },
        { -3, 0x59, gcSL_CG_TEMP1, 1, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_24_16 },
        { -2, 0x5A, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_24_16 },
        { -1, 0x33, gcSL_CG_TEMP1, 2, 3, gcSL_CG_TEMP1, 0, denorm_value_type0 },

    /*
        STORE 1, 2, 3
            store 0, 2, 3, 1, 0
    */
    { 1, gcSL_STORE, 1, 2, 3, 0, 0, _isCL_X_Unsigned_8_16_store },
        { -2, 0x5D, gcSL_CG_TEMP1, 1, 0, gcSL_CG_CONSTANT, 0, uint_value_type0_const_FF_FFFF },
        { -1, 0x33, gcSL_CG_TEMP1, 2, 3, gcSL_CG_TEMP1, 0, denorm_value_type0 },

    /*
        STORE 1, 2, 3
            store 0, 2, 3, 1, 0
    */
    { 1, gcSL_STORE, 1, 2, 3, 0, 0, _hasInteger },
        { -1, 0x33, 1, 2, 3, 1, 0, denorm_value_type0 },

    { 0 }
};

/* 0x38 gcSL_BARRIER */
const gcsSL_PATTERN patterns_BARRIER[] =
{
    /*
        BARRIER 1
            barrier 0, 0, 0, 0, 0
    */
    { 1, gcSL_BARRIER },
        { -1, 0x2A, 0, 0, 0, 0, 0 },

    { 0 }
};

/* 0x39 gcSL_STORE1 */
const gcsSL_PATTERN patterns_STORE1[] =
{
#if _SUPPORT_LONG_ULONG_DATA_TYPE
#if _DO_PACKED_MODE_LONG_ULONG
    /*
        STORE1 1, 2, 3
            store 0, 2, constant_0, 3, 0
    */
    { 1, gcSL_STORE1, 1, 2, 3, 0, 0, _isCL_Long_ulong_1_store},
        { -1, 0x33, 1, 2, gcSL_CG_CONSTANT, 3, 0, long_ulong_first_store },
    { 1, gcSL_STORE1, 1, 2, 3, 0, 0, _isCL_Long_ulong_2_store},
        { -2, 0x33, 1, 2, gcSL_CG_CONSTANT, 3, 0, long_ulong_first_store },
        { -1, 0x33, 1, 2, gcSL_CG_CONSTANT, 3, 0, long_ulong_second_store },
    { 1, gcSL_STORE1, 1, 2, 3, 0, 0, _isCL_Long_ulong_3_store},
        { -3, 0x33, 1, 2, gcSL_CG_CONSTANT, 3, 0, long_ulong_first_store },
        { -2, 0x33, 1, 2, gcSL_CG_CONSTANT, 3, 0, long_ulong_second_store },
        { -1, 0x33, 1, 2, gcSL_CG_CONSTANT, 3, 0, long_ulong_third_store },
    { 1, gcSL_STORE1, 1, 2, 3, 0, 0, _isCL_Long_ulong_4_store},
        { -4, 0x33, 1, 2, gcSL_CG_CONSTANT, 3, 0, long_ulong_first_store },
        { -3, 0x33, 1, 2, gcSL_CG_CONSTANT, 3, 0, long_ulong_second_store },
        { -2, 0x33, 1, 2, gcSL_CG_CONSTANT, 3, 0, long_ulong_third_store },
        { -1, 0x33, 1, 2, gcSL_CG_CONSTANT, 3, 0, long_ulong_fourth_store },
#else
    /*
        STORE1 1, 2, 3
            store 0, 2, constant_0, 3, 0
    */
    { 1, gcSL_STORE1, 1, 2, 3, 0, 0, _isCL_Long_ulong_2_store},
        { -2, 0x33, 1, 2, gcSL_CG_CONSTANT, 3, 0, long_ulong_first_store },
        { -1, 0x33, 1, 2, gcSL_CG_CONSTANT, 3, 0, long_ulong_second_store },

    /*
        STORE1 1, 2, 3
            store 0, 2, constant_0, 3, 0
    */
    { 1, gcSL_STORE1, 1, 2, 3, 0, 0, _isCL_Long_ulong_4_store},
        { -4, 0x33, 1, 2, gcSL_CG_CONSTANT, 3, 0, long_ulong_first_store },
        { -3, 0x33, 1, 2, gcSL_CG_CONSTANT, 3, 0, long_ulong_second_store },
        { -2, 0x33, 1, 2, gcSL_CG_CONSTANT, 3, 0, long_ulong_third_store },
        { -1, 0x33, 1, 2, gcSL_CG_CONSTANT, 3, 0, long_ulong_fourth_store },
#endif
#else
    /* Handle OCL 1.2 conformance case without full support of long/ulong */
    /*
        STORE1 1, 2, 3
            store 0, 2, constant_0, 3, 0
            store 0, 2, constant_4, constant_0, 0
    */
    { 1, gcSL_STORE1, 1, 2, 3, 0, 0, _isCL_Long_ulong_2_store},
        { -3, 0x33, 1, 2, gcSL_CG_CONSTANT, 3, 0, denorm_value_type0_const_0 },
        { -2, 0x09, gcSL_CG_TEMP1, 0, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_4 },
        { -1, 0x33, 1, 2, gcSL_CG_TEMP1, gcSL_CG_CONSTANT, 0, denorm_value_type0_zero_2 },
#endif

    /*
        STORE1 1, 2, 3
            store 0, 2, constant_0, 3, 0
    */
    { 1, gcSL_STORE1, 1, 2, 3, 0, 0, _isCL_X_Signed_8_16_store },
        { -3, 0x59, gcSL_CG_TEMP1, 3, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_24_16 },
        { -2, 0x5A, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_24_16 },
        { -1, 0x33, 1, 2, gcSL_CG_CONSTANT, gcSL_CG_TEMP1, 0, denorm_value_type0_const_0 },

    /*
        STORE1 1, 2, 3
            store 0, 2, constant_0, 3, 0
    */
    { 1, gcSL_STORE1, 1, 2, 3, 0, 0, _isCL_X_Unsigned_8_16_store },
        { -2, 0x5D, gcSL_CG_TEMP1, 3, 0, gcSL_CG_CONSTANT, 0, uint_value_type0_const_FF_FFFF },
        { -1, 0x33, 1, 2, gcSL_CG_CONSTANT, gcSL_CG_TEMP1, 0, denorm_value_type0_const_0 },

    /*
        STORE1 1, 2, 3
            store 0, 2, constant_0, 3, 0
    */
    { 1, gcSL_STORE1, 1, 2, 3 },
        { -1, 0x33, 1, 2, gcSL_CG_CONSTANT, 3, 0, denorm_value_type0_const_0 },

    { 0 }
};

/* 0x3A gcSL_ATOMADD */
const gcsSL_PATTERN patterns_ATOMADD[] =
{
    /*
        ATOMADD 1, 2, 3
            atom_add 1, 2, constant_0, 3, 0
    */
    { 1, gcSL_ATOMADD, 1, 2, 3, 0, 0, 0 },
        { -1, 0x65, 1, 2, gcSL_CG_CONSTANT, 3, 0, value_type0_const_0 },

    { 0 }
};

/* 0x3B gcSL_ATOMSUB */
const gcsSL_PATTERN patterns_ATOMSUB[] =
{
    /*
        ATOMSUB 1, 2, 3
            atom_add 1, 2, constant_0, -3, 0
    */
    { 1, gcSL_ATOMSUB, 1, 2, 3, 0, 0, 0 },
        { -1, 0x65, 1, 2, gcSL_CG_CONSTANT, -3, 0, value_type0_const_0 },

    { 0 }
};

/* 0x3C gcSL_ATOMXCHG */
const gcsSL_PATTERN patterns_ATOMXCHG[] =
{
    /*
        ATOMXCHG 1, 2, 3
            atom_xchg 1, 2, constant_0, 3, 0
    */
    { 1, gcSL_ATOMXCHG, 1, 2, 3, 0, 0 },
        { -1, 0x66, 1, 2, gcSL_CG_CONSTANT, 3, 0, value_type0_const_0 },

    { 0 }
};

/* 0x3D gcSL_ATOMCMPXCHG */
const gcsSL_PATTERN patterns_ATOMCMPXCHG[] =
{
    /*
        ATOMCMPXCHG 1, 2, 3
            atom_cmpxchg 1, 2, constant_0, 3, 0
    */
    { 1, gcSL_ATOMCMPXCHG, 1, 2, 3, 0, 0 },
        { -1, 0x67, 1, 2, gcSL_CG_CONSTANT, 3, 0, value_type0_const_0 },

    { 0 }
};

/* 0x3E gcSL_ATOMMIN */
const gcsSL_PATTERN patterns_ATOMMIN[] =
{
    /*
        ATOMMIN 1, 2, 3
            atom_min 1, 2, constant_0, 3, 0
    */
    { 1, gcSL_ATOMMIN, 1, 2, 3, 0, 0 },
        { -1, 0x68, 1, 2, gcSL_CG_CONSTANT, 3, 0, value_type0_const_0 },

    { 0 }
};

/* 0x3F gcSL_ATOMMAX */
const gcsSL_PATTERN patterns_ATOMMAX[] =
{
    /*
        ATOMMAX 1, 2, 3
            atom_max 1, 2, constant_0, 3, 0
    */
    { 1, gcSL_ATOMMAX, 1, 2, 3, 0, 0 },
        { -1, 0x69, 1, 2, gcSL_CG_CONSTANT, 3, 0, value_type0_const_0 },

    { 0 }
};

/* 0x40 gcSL_ATOMOR */
const gcsSL_PATTERN patterns_ATOMOR[] =
{
    /*
        ATOMOR 1, 2, 3
            atom_or 1, 2, constant_0, 3, 0
    */
    { 1, gcSL_ATOMOR, 1, 2, 3, 0, 0 },
        { -1, 0x6A, 1, 2, gcSL_CG_CONSTANT, 3, 0, value_type0_const_0 },

    { 0 }
};

/* 0x41 gcSL_ATOMAND */
const gcsSL_PATTERN patterns_ATOMAND[] =
{
    /*
        ATOMAND 1, 2, 3
            atom_and 1, 2, constant_0, 3, 0
    */
    { 1, gcSL_ATOMAND, 1, 2, 3, 0, 0 },
        { -1, 0x6B, 1, 2, gcSL_CG_CONSTANT, 3, 0, value_type0_const_0 },

    { 0 }
};

/* 0x42 gcSL_ATOMXOR */
const gcsSL_PATTERN patterns_ATOMXOR[] =
{
    /*
        ATOMXOR 1, 2, 3
            atom_xor 1, 2, constant_0, 3, 0
    */
    { 1, gcSL_ATOMXOR, 1, 2, 3, 0, 0 },
        { -1, 0x6C, 1, 2, gcSL_CG_CONSTANT, 3, 0, value_type0_const_0 },

    { 0 }
};

/* 0x43 gcSL_TEXLDPCF */
const gcsSL_PATTERN patterns_TEXLDPCF[] =
{
#if _CLAMP_PCF_REFERENCE_
    /*
        TEXLDPCF 1, 2, 3
            texld_l_pcf 1, 3, 4, 0, 2
    */
    { 1, gcSL_TEXLDPCF, 1, 2, 3, 0, 0, _hasNEW_TEXLD_isVertex },
        { -2, 0x09, gcSL_CG_TEMP1_X_NO_SRC_SHIFT, 0, 0, 3, 0, saturate_swizzle2ZorW },
        { -1, 0x6F, 1, 3, gcSL_CG_CONSTANT, gcSL_CG_TEMP1_X_NO_SRC_SHIFT, 2, zero_1_sample_swizzleX_fix_shadow_coord_swizzle },

    /*
        TEXLDPCF 1, 2, 3
            mov TEMP1_XYZW, 0, 0, 3, 0
            mov TEMP1_XYZW.w, 0, 0, 4, 0
            texldl  1, TEMP1_XYZW, 0, 0, 2
            set.le 1, 1, 3.w
    */
    { 1, gcSL_TEXLDPCF, 1, 2, 3, 0, 0, _isVertex },
        { -5, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0 },
        { -4, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_CONSTANT, 0, zero_2_enable_w },
        { -3, 0x09, gcSL_CG_TEMP2_X_NO_SRC_SHIFT, 0, 0, 3, 0, saturate_swizzle2ZorW },
        { -2, 0x1B, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2, sample_swizzleX_fix_shadow_coord_swizzle },
        { -1, 0x10, 1, gcSL_CG_TEMP2_X_NO_SRC_SHIFT, 1, 0, 0, conditionLE },

        /*
        TEXLDPCF 1, 2, 3
            texld_b_pcf 1, 3, 0, 0, 2
    */
    { 1, gcSL_TEXLDPCF, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -2, 0x09, gcSL_CG_TEMP1_X_NO_SRC_SHIFT, 0, 0, 3, 0, saturate_swizzle2ZorW },
        { -1, 0x18, 1, 3, 0, gcSL_CG_TEMP1_X_NO_SRC_SHIFT, 2, sample_swizzleX_fix_shadow_coord_swizzle },

    /*
        TEXLDPCF 1, 2, 3
            texld  1, 3, 0, 0, 2
            set.le 1, 1, 3.w
    */
    { 1, gcSL_TEXLDPCF, 1, 2, 3 },
        { -3, 0x09, gcSL_CG_TEMP1_X_NO_SRC_SHIFT, 0, 0, 3, 0, saturate_swizzle2ZorW },
        { -2, 0x18, 1, 3, 0, 0, 2, sample_swizzleX_fix_shadow_coord_swizzle },
        { -1, 0x10, 1, gcSL_CG_TEMP1_X_NO_SRC_SHIFT, 1, 0, 0, conditionLE },
#else
    /*
        TEXLDPCF 1, 2, 3
            texld_l_pcf 1, 3, 0, 0, 2
    */
    { 1, gcSL_TEXLDPCF, 1, 2, 3, 0, 0, _hasNEW_TEXLD_isVertex },
        { -1, 0x6F, 1, 3, gcSL_CG_CONSTANT, 3, 2, zero_1_swizzle2ZorW_sample_swizzleX_fix_shadow_coord_swizzle },

    /*
        TEXLDPCF 1, 2, 3
            texldl  1, 3, 0, 0, 2
            set.le 1, 1, 3.w
    */
    { 1, gcSL_TEXLDPCF, 1, 2, 3 },
        { -4, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0 },
        { -3, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_CONSTANT, 0, zero_2_enable_w },
        { -2, 0x1B, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2, sample_swizzleX_fix_shadow_coord_swizzle },
        { -1, 0x10, 1, 3, 1, 0, 0, conditionLE_swizzle0ZorW },

        /*
        TEXLDPCF 1, 2, 3
            texld_b_pcf 1, 3, 0, 0, 2
    */
    { 1, gcSL_TEXLDPCF, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -1, 0x18, 1, 3, 0, 3, 2, swizzle2ZorW_sample_swizzleX_fix_shadow_coord_swizzle },

    /*
        TEXLDPCF 1, 2, 3
            texld  1, 3, 0, 0, 2
            set.le 1, 1, 3.w
    */
    { 1, gcSL_TEXLDPCF, 1, 2, 3 },
        { -2, 0x18, 1, 3, 0, 0, 2, sample_swizzleX_fix_shadow_coord_swizzle },
        { -1, 0x10, 1, 3, 1, 0, 0, conditionLE_swizzle0ZorW },
#endif

    { 0 }
};

/* 0x44 gcSL_TEXLDPCFPROJ */
const gcsSL_PATTERN patterns_TEXLDPCFPROJ[] =
{
#if _CLAMP_PCF_REFERENCE_
    /*
        TEXLDPCFPROJ 1, 2, 3
            rcp TEMP1, 0, 0, 3.<last>
            mul TEMP1, 3, TEMP1, 0
            texld_l_pcf 1, TEMP1, 0, 0, 2
    */
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3, 0, 0, _hasNEW_TEXLD_isVertex },
        { -4, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -3, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -2, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_TEMP1_XYZW, 0, enable_z_saturate_swizzle2Z },
        { -1, 0x6F, 1, gcSL_CG_TEMP1_XYZW, gcSL_CG_CONSTANT, gcSL_CG_TEMP1_XYZW, 2, zero_1_swizzle2Z_sample_swizzleX },

    /*
        TEXLDPCFPCFPROJ 1, 2, 3
            rcp TEMP1, 0, 0, 3.<last>
            mul TEMP1, 3, TEMP1, 0
            texldl  1, TEMP1, 0, 0, 2
            set.le 1, 1, 3.w
    */
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3, 0, 0, _isVertex },
        { -6, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -5, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -4, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_TEMP1_XYZW, 0, enable_z_saturate_swizzle2Z },
        { -3, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_CONSTANT, 0, zero_2_enable_w },
        { -2, 0x1B, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2, sample_swizzleX },
        { -1, 0x10, 1, gcSL_CG_TEMP1_XYZW, 1, 0, 0, conditionLE_swizzle0Z },

        /*
        TEXLDPCFPROJ 1, 2, 3
            rcp TEMP1, 0, 0, 3.<last>
            mul TEMP1, 3, TEMP1, 0
            texld_b_pcf 1, TEMP1, 0, 0, 2
    */
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -4, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -3, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -2, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_TEMP1_XYZW, 0, enable_z_saturate_swizzle2Z },
        { -1, 0x18, 1, gcSL_CG_TEMP1_XYZW, 0, gcSL_CG_TEMP1_XYZW, 2, swizzle2Z_sample_swizzleX },

    /*
        TEXLDPCFPCFPROJ 1, 2, 3
            rcp TEMP1, 0, 0, 3.<last>
            mul TEMP1, 3, TEMP1, 0
            texld  1, TEMP1, 0, 0, 2
            set.le 1, 1, 3.w
    */
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3 },
        { -5, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -4, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -3, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_TEMP1_XYZW, 0, enable_z_saturate_swizzle2Z },
        { -2, 0x18, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2, swizzle0XY_sample_swizzleX },
        { -1, 0x10, 1, gcSL_CG_TEMP1_XYZW, 1, 0, 0, conditionLE_swizzle0Z },
#else
    /*
        TEXLDPCFPROJ 1, 2, 3
            rcp TEMP1, 0, 0, 3.<last>
            mul TEMP1, 3, TEMP1, 0
            texld_l_pcf 1, TEMP1, 0, 0, 2
    */
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -4, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -3, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -2, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 4, 0, enable_w },
        { -1, 0x6F, 1, gcSL_CG_TEMP1_XYZW, gcSL_CG_CONSTANT, gcSL_CG_TEMP1_XYZW, 2, zero_1_swizzle2Z_sample_swizzleX },

    /*
        TEXLDPCFPROJ 1, 2, 3
            rcp TEMP1, 0, 0, 3.<last>
            mul TEMP1, 3, TEMP1, 0
            texldl  1, TEMP1, 0, 0, 2
            set.le 1, 1, 3.w
    */
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3, 0, 0, _isVertex },
        { -5, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -4, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -3, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_CONSTANT, 0, zero_2_enable_w },
        { -2, 0x1B, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2, sample_swizzleX },
        { -1, 0x10, 1, gcSL_CG_TEMP1_XYZW, 1, 0, 0, conditionLE_swizzle0Z },

        /*
        TEXLDPCFPROJ 1, 2, 3
            rcp TEMP1, 0, 0, 3.<last>
            mul TEMP1, 3, TEMP1, 0
            texld_b_pcf 1, TEMP1, 0, 0, 2
    */
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -3, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -2, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -1, 0x18, 1, gcSL_CG_TEMP1_XYZW, 0, gcSL_CG_TEMP1_XYZW, 2, swizzle2Z_sample_swizzleX },

    /*
        TEXLDPCFPROJ 1, 2, 3
            rcp TEMP1, 0, 0, 3.<last>
            mul TEMP1, 3, TEMP1, 0
            texld  1, TEMP1, 0, 0, 2
            set.le 1, 1, 3.w
    */
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3 },
        { -4, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -3, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -2, 0x18, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2, sample_swizzleX },
        { -1, 0x10, 1, gcSL_CG_TEMP1_XYZW, 1, 0, 0, conditionLE_swizzle0Z },
#endif

    { 0 }
};

/* 0x45 gcSL_TEXLODQ */
const gcsSL_PATTERN patterns_TEXLODQ[] =
{
    /*
        TEXLDDQ  1, 2, 3
            lodq 1, 3, 0, 0, 2
    */
    { 1, gcSL_TEXLODQ, 1, 2, 3, 0, 0, _hasHalti3 },
        { -1, 0x7F, 1, 3, 0, gcSL_CG_CONSTANT, 2, set_extended_opcode_lodq },

    { 0 }
};

/* 0x46 gcSL_FLUSH */
const gcsSL_PATTERN patterns_FLUSH[] =
{
    /*
        FLUSH  0, 1
            flush 0, 1, 0, opcode
    */
    { 1, gcSL_FLUSH, 0, 1, 0, 0, 0, _hasHalti3 },
        { -1, 0x7F, 0, 1, 0, gcSL_CG_CONSTANT, 0, set_extended_opcode_flush },

    { 0 }
};

/* 0x47 gcSL_JMP_ANY */
const gcsSL_PATTERN patterns_JMP_ANY[] =
{
    /*
        JMP_ANY.cond 1, 2, 3
            branch_any.cond 0, 1, 2, 0, 0
    */
    { 1, gcSL_JMP_ANY, 0, 1, 2, 0, 0, _hasHalti4 },
        { -1, 0x24, 0, 1, 2, 0, 0, branch },

    { 0 }
};

/* 0x48 gcSL_BITRANGE */
const gcsSL_PATTERN patterns_BITRANGE[] =
{
    /*
        BITRANGE   0, 3, 4
        BITEXTRACT 1, 2
            bit_extract  1, 2, 3, 4
    */
    { 2, gcSL_BITRANGE, 0, 3, 4, 0, 0, _hasHalti4 },
    { 1, gcSL_BITEXTRACT, 1, 2 },
        { -1, 0x60, 1, 2, 3, 4, 0, value_type0},

    /*
        BITRANGE   0, 4, 5
        BITINSERT  1, 2, 3
            bit_insert1  1, 2, 3, 4
    */
    { 2, gcSL_BITRANGE, 0, 4, 5, 0, 0, _hasHalti4 },
    { 1, gcSL_BITINSERT, 1, 2, 3 },
        { -3, 0x59, gcSL_CG_TEMP1_X, 5, 0, gcSL_CG_CONSTANT, 0, eight_2 },
        { -2, 0x5C, gcSL_CG_TEMP1_X, gcSL_CG_TEMP1_X, 0, 4, 0, value_type0 },
        { -1, 0x54, 1, 2, 3, gcSL_CG_TEMP1_X, 0, value_type0},

    { 0 }
};

/* 0x49 gcSL_BITRANGE1 */
const gcsSL_PATTERN patterns_BITRANGE1[] =
{
    /*
        BITRANGE1  0, 4
        BITINSERT  1, 2, 3
            bit_insert1  1, 2, 3, 4
    */
    { 2, gcSL_BITRANGE1, 0, 4, 0, 0, 0, _hasHalti4_and_const0 },
    { 1, gcSL_BITINSERT, 1, 2, 3 },
        { -1, 0x54, 1, 2, 3, 4, 0, value_type0},

    /*
        BITRANGE1  0, 4
        BITINSERT  1, 2, 3
            bit_insert2  1, 2, 3, 4
    */
    { 2, gcSL_BITRANGE1, 0, 4, 0, 0, 0, _hasHalti4 },
    { 1, gcSL_BITINSERT, 1, 2, 3 },
        { -1, 0x55, 1, 2, 3, 4, 0, value_type0},

    { 0 }
};

/* 0x4A gcSL_BITEXTRACT */
const gcsSL_PATTERN patterns_BITEXTRACT[] =
{
    { 0 }
};

/* 0x4B gcSL_BITINSERT */
const gcsSL_PATTERN patterns_BITINSERT[] =
{
    { 0 }
};

/* 0x4C gcSL_FINDLSB */
const gcsSL_PATTERN patterns_FINDLSB[] =
{
    /*
        FINDLSB 1, 2
            findlsb 1, 2, 0, opcode
    */
    { 1, gcSL_FINDLSB, 1, 2, 0, 0, 0, _hasHalti4 },
        { -1, 0x7F, 1, 2, 0, gcSL_CG_CONSTANT, 0, set_extended_opcode_findlsb_src0_type},

    { 0 }
};

/* 0x4D gcSL_FINDMSB */
const gcsSL_PATTERN patterns_FINDMSB[] =
{
    /*
        FINDMSB 1, 2
            findmsb 1, 2, 0, opcode
    */
    { 1, gcSL_FINDMSB, 1, 2, 0, 0, 0, _hasHalti4 },
        { -1, 0x7F, 1, 2, 0, gcSL_CG_CONSTANT, 0, set_extended_opcode_findmsb_src0_type},

    { 0 }
};

/* 0x4E gcSL_IMAGE_OFFSET */
const gcsSL_PATTERN patterns_IMAGE_OFFSET[] =
{
    /*
        IMAGE_OFFSET  0, 2, 4
        IMAGE_RD      1, 2, 3
            img_load  1, 2, 3, 4
    */
    { 2, gcSL_IMAGE_OFFSET, 0, 2, 4, 0, 0, _hasHalti3 },
    { 1, gcSL_IMAGE_RD, 1, 2, 3 },
        { -1, 0x79, 1, 2, 3, 4},

    /*
        IMAGE_OFFSET  0, 2, 4
        IMAGE_ADDR    1, 2, 3
            img_addr   1, 2, 3, 4
    */
    { 2, gcSL_IMAGE_OFFSET, 0, 2, 4, 0, 0, _hasHalti4 },
    { 1, gcSL_IMAGE_ADDR, 1, 2, 3 },
        { -1, 0x37, 1, 2, 3, 4},

    { 0 }
};

/* 0x4F gcSL_IMAGE_ADDR */
const gcsSL_PATTERN patterns_IMAGE_ADDR[] =
{
    /*
        IMAGE_ADDR    1, 2, 3
            img_addr   1, 2, 3
    */
    { 1, gcSL_IMAGE_ADDR, 1, 2, 3, 0, 0, _hasHalti4_image2D },
        { -1, 0x37, 1, 2, 3, 0, 0, value_types_u32},

    /*
        IMAGE_ADDR    1, 2, 3
            img_addr_3d   1, 2, 3
    */
    { 1, gcSL_IMAGE_ADDR, 1, 2, 3, 0, 0, _hasHalti4 },
        { -1, 0x38, 1, 2, 3, 0, 0, value_types_u32},

    { 0 }
};

/* 0x50 gcSL_SINPI */
const gcsSL_PATTERN patterns_SINPI[] =
{
    /*
        SINPI 1, 2
            sin 1, 0, 0, 2
    */
    { 1, gcSL_SINPI, 1, 2, 0, 0, 0, _hasNEW_SIN_COS_LOG_DIV },
        { -1, 0x22, 1, 0, 0, 2, 0, set_new_sin_cos_log_div },

    /*
        SINPI 1, 2
            sin 1, 0, 0, 2
    */
    { 1, gcSL_SINPI, 1, 2, 0, 0, 0, _hasSQRT_TRIG },
        { -1, 0x22, 1, 0, 0, 2, 0, 0 },

    { 0 }
};

/* 0x51 gcSL_COSPI */
const gcsSL_PATTERN patterns_COSPI[] =
{
    /*
        COSPI 1, 2
            cos 1, 0, 0, 2
    */
    { 1, gcSL_COSPI, 1, 2, 0, 0, 0, _hasNEW_SIN_COS_LOG_DIV },
        { -1, 0x23, 1, 0, 0, 2, 0, set_new_sin_cos_log_div },

    /*
        COSPI 1, 2
            cos 1, 0, 0, 2
    */
    { 1, gcSL_COSPI, 1, 2, 0, 0, 0, _hasSQRT_TRIG },
        { -1, 0x23, 1, 0, 0, 2, 0, 0 },
    { 0 }
};

/* 0x52 gcSL_TANPI */
const gcsSL_PATTERN patterns_TANPI[] =
{
    /*
        TANPI 1, 2
            sin temp1, 0, 0, 2
            cos temp2, 0, 0, 2
            div 1, temp1, temp2 // tan(x) = sin(x)/cos(x)
    */
    { 1, gcSL_TANPI, 1, 2, 0, 0, 0, _hasNEW_SIN_COS_LOG_DIV },
        { -3, 0x22, gcSL_CG_TEMP1, 0, 0, 2, 0, set_new_sin_cos_log_div },
        { -4, 0x23, gcSL_CG_TEMP2, 0, 0, 2, 0, set_new_sin_cos_log_div },
        { -1, 0x64, 1, 0, gcSL_CG_TEMP1, gcSL_CG_TEMP2, 0, set_new_sin_cos_log_div },

    { 0 }
};

/* 0x53 gcSL_ADDLO */
const gcsSL_PATTERN patterns_ADDLO[] =
{
    /*
        ADDLO 1, 2, 3
            add 1, 2, 0, 3, 0
    */
    { 1, gcSL_ADDLO, 1, 2, 3, 0, 0, _hasRounding_mode },
        { -1, 0x28, 1, 2, 0, 3, 0, rounding_mode },

    /*
        ADDLO 1, 2, 3
            add 1, 2, 0, 3, 0
    */
    { 1, gcSL_ADDLO, 1, 2, 3 },
        { -1, 0x28, 1, 2, 0, 3, 0, add2mad },

    { 0 }
};

/* 0x54 gcSL_MULLO */
const gcsSL_PATTERN patterns_MULLO[] =
{
    /*
        MULLO 1, 2, 3
            mul 1, 2, 3, 0, 0
    */
    { 1, gcSL_MULLO, 1, 2, 3, 0, 0, _hasRounding_mode },
        { -1, 0x29, 1, 2, 3, 0, 0, rounding_mode },

    /*
        MULLO 1, 2, 3
            mul 1, 2, 3, 0, 0
    */
    { 1, gcSL_MULLO, 1, 2, 3 },
        { -1, 0x29, 1, 2, 3, 0, 0 },

    { 0 }
};

#if _SUPPORT_LONG_ULONG_DATA_TYPE

/* singned to ulong convert with _sat mode enable */
static gctBOOL
_isI2I_int2ulong_sat(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_FORMAT formatDst, srcFormat;

    if (!IS_SATURATE(Instruction->opcode))
    {
        return gcvFALSE;
    }

    srcFormat = gcmSL_SOURCE_GET(Instruction->source0, Format);
    formatDst = gcmSL_TARGET_GET(Instruction->temp, Format);
    if((srcFormat == gcSL_INT32) && (formatDst == gcSL_UINT64)) return gcvTRUE;
    else return gcvFALSE;
}

static gctBOOL
int2ulong_sat_cmp(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));
    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                  CodeGen,
                                  0,
                                  &index,
                                  &swizzle,
                                  &constType));

    _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, constType, States);

    _SetValueType0(type_conv[gcSL_INT32], States);

    return gcvTRUE;
}

/* singend 2 long or ulong */
static gctBOOL
_isI2I_int2longulong(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_FORMAT formatDst, srcFormat;

    srcFormat = gcmSL_SOURCE_GET(Instruction->source0, Format);
    formatDst = gcmSL_TARGET_GET(Instruction->temp, Format);

    if (IS_SATURATE(Instruction->opcode) && (formatDst == gcSL_UINT64))
    {
        return gcvFALSE;
    }

    if((srcFormat == gcSL_INT32) && (formatDst == gcSL_INT64 || formatDst == gcSL_UINT64)) return gcvTRUE;
    else return gcvFALSE;
}

static gctBOOL
int2longulong_mov(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_FORMAT format;

    format = gcmSL_TARGET_GET(Instruction->temp, Format);
    if(format == gcSL_INT64) format = gcSL_INT32;
    else if(format == gcSL_UINT64) format = gcSL_UINT32;

    _SetValueType0(type_conv[format], States);
    return gcvTRUE;
}

static gctBOOL
int2longulong_rshift(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;
    gcSL_FORMAT format;

    format = gcmSL_TARGET_GET(Instruction->temp, Format);

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                    CodeGen,
                                    31,
                                    &index,
                                    &swizzle,
                                    &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(0x2, States);

    return gcvTRUE;
}

static gctBOOL
int2longulong_sign_bit_set(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_FORMAT format;
    gctUINT address;

    format = gcmSL_TARGET_GET(Instruction->temp, Format);
    if(format == gcSL_INT64) format = gcSL_INT32;
    else if(format == gcSL_UINT64) format = gcSL_UINT32;

    _SetValueType0(type_conv[format], States);

    address = (((((gctUINT32) (States[0])) >> (0 ? 22:16)) & ((gctUINT32) ((((1 ? 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1)))))) ) + 1;
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ?
 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ?
 22:16)));

    return gcvTRUE;
}

/* uint 2 long or ulong */
static gctBOOL
_isI2I_uint2longulong(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_FORMAT formatDst, srcFormat;

    srcFormat = gcmSL_SOURCE_GET(Instruction->source0, Format);
    formatDst = gcmSL_TARGET_GET(Instruction->temp, Format);
    if((srcFormat == gcSL_UINT32) && (formatDst == gcSL_INT64 || formatDst == gcSL_UINT64)) return gcvTRUE;
    else return gcvFALSE;
}

static gctBOOL
uint2longulong_first_mov(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    _SetValueType0(type_conv[gcSL_UINT32], States);

    return gcvTRUE;
}

static gctBOOL
uint2longulong_second_mov(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gctINT index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;
    gctUINT address;

    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                    CodeGen,
                                    0u,
                                    &index,
                                    &swizzle,
                                    &constType));

    _UsingConstUniform(Tree, CodeGen, 2, index, swizzle, constType, States);

    _SetValueType0(type_conv[gcSL_UINT32], States);

    address = (((((gctUINT32) (States[0])) >> (0 ? 22:16)) & ((gctUINT32) ((((1 ? 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1)))))) ) + 1;
    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ?
 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ?
 22:16)));

    return gcvTRUE;
}

/* longulong to sus convert */
static gctBOOL
_isI2I_longulong2sus(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_FORMAT formatDst, srcFormat;

    if (IS_SATURATE(Instruction->opcode))
    {
        return gcvFALSE;
    }

    srcFormat = gcmSL_SOURCE_GET(Instruction->source0, Format);
    formatDst = gcmSL_TARGET_GET(Instruction->temp, Format);
    if((srcFormat == gcSL_UINT64 || srcFormat == gcSL_INT64) &&
        (formatDst == gcSL_INT32 || formatDst == gcSL_UINT32 ||
         formatDst == gcSL_INT16 || formatDst == gcSL_UINT16 ||
         formatDst == gcSL_INT8 || formatDst == gcSL_UINT8))
        return gcvTRUE;
    else return gcvFALSE;
}

static gctBOOL
longulong2usu_mov(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_FORMAT format;
    gctUINT value_type;
    gctUINT inst_type0;
    gctUINT inst_type1;

    format = gcmSL_TARGET_GET(Instruction->temp, Format);
    value_type = type_conv[format];
    inst_type0 = value_type & 0x1;
    inst_type1 = value_type >> 1;

    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) ((gctUINT32) (inst_type0) & ((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21)));
    States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (inst_type1) & ((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30)));

    return gcvTRUE;
}

/* long ulong convert while _sat mode disable */
static gctBOOL
_isI2I_longulongConvert(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    )
{
    gcSL_FORMAT formatDst, srcFormat;

    if (IS_SATURATE(Instruction->opcode))
    {
        return gcvFALSE;
    }

    srcFormat = gcmSL_SOURCE_GET(Instruction->source0, Format);
    formatDst = gcmSL_TARGET_GET(Instruction->temp, Format);
    if((srcFormat == gcSL_UINT64 || srcFormat == gcSL_INT64) &&
        (formatDst == gcSL_INT64 || formatDst == gcSL_UINT64))
        return gcvTRUE;
    else return gcvFALSE;
}

#endif

/* 0x55 gcSL_CONV */
const gcsSL_PATTERN patterns_CONV[] =
{
#if _USE_CONV_FOR_EXPLICIT_CONVERT_FUNCTION
    /*
        CONV 1, 2
            SELECT, 1, 2, gcSL_CG_CONSTANT, 2, 0
            SELECT, 1, 1, gcSL_CG_CONSTANT, 1, 0
            CEIL, 1, 0, 0, 1
            F2I, 1, 1, 0, 0, 0
    */
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isF2I_Sat_Rtp) },
        { -4, 0x0F, 1, 2, gcSL_CG_CONSTANT, 2, 0, max_type0_const_conditionGT },
        { -3, 0x0F, 1, 1, gcSL_CG_CONSTANT, 1, 0, min_type0_const_conditionLT },
        { -2, 0x26, 1, 0, 0, 1},
        { -1, 0x2E, 1, 1, 0, 0, 0, value_type0_32bit },

    /*
        CONV 1, 2
            SELECT, 1, 2, gcSL_CG_CONSTANT, 2, 0
            SELECT, 1, 1, gcSL_CG_CONSTANT, 1, 0
            FLOOR, 1, 0, 0, 1
            F2I, 1, 1, 0, 0, 0
    */
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isF2I_Sat_Rtn) },
        { -4, 0x0F, 1, 2, gcSL_CG_CONSTANT, 2, 0, max_type0_const_conditionGT },
        { -3, 0x0F, 1, 1, gcSL_CG_CONSTANT, 1, 0, min_type0_const_conditionLT },
        { -2, 0x25, 1, 0, 0, 1},
        { -1, 0x2E, 1, 1, 0, 0, 0, value_type0_32bit },

    /*
        CONV 1, 2
            CEIL, 1, 0, 0, 1
            F2I, 1, 1, 0, 0, 0
    */
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isF2I_Rtp) },
        { -2, 0x26, 1, 0, 0, 2},
        { -1, 0x2E, 1, 1, 0, 0, 0, value_type0_32bit },

    /*
        CONV 1, 2
            CEIL, 1, 0, 0, 1
            F2I, 1, 1, 0, 0, 0
    */
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isF2I_Rtn) },
        { -2, 0x25, 1, 0, 0, 2},
        { -1, 0x2E, 1, 1, 0, 0, 0, value_type0_32bit },

    /*
        CONV 1, 2
            SELECT, 1, 2, gcSL_CG_CONSTANT, 2, 0
            SELECT, 1, 1, gcSL_CG_CONSTANT, 1, 0
            F2I, 1, 1, 0, 0, 0
    */
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isF2I_Sat) },
        { -3, 0x0F, 1, 2, gcSL_CG_CONSTANT, 2, 0, max_type0_const_conditionGT },
        { -2, 0x0F, 1, 1, gcSL_CG_CONSTANT, 1, 0, min_type0_const_conditionLT },
        { -1, 0x2E, 1, 1, 0, 0, 0, value_type0_32bit },

    /*
        CONV 1, 2
            F2I 1, 0, 0, 2, 0
    */
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, _isF2I },
        { -1, 0x2E, 1, 2, 0, 0, 0, value_type0_32bit },

    /*
        CONV 1, 2
            I2F 1, 0, 0, 2, 0
    */
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, _isI2F_Rounding_mode },
        { -1, 0x2D, 1, 2, 0, 0, 0, rounding_mode_value_type0_32bit_from_src0 },

    /*
        CONV 1, 2
            I2F 1, 0, 0, 2, 0
    */
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, _isI2F },
        { -1, 0x2D, 1, 2, 0, 0, 0, _value_type0_32bit_from_src0 },

    /*
        CONV 1, 2
            I2I 1, 0, 0, 2, 0
    */
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isI2I_Sat_s2us) },
        { -2, 0x0F, 1, 2, gcSL_CG_CONSTANT, 2, 0, max_type0_const_conditionGT },
        { -1, 0x0F, 1, 1, gcSL_CG_CONSTANT, 1, 0, min_type0_const_conditionLT },

    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isI2I_Sat_u2us) },
        { -1, 0x0F, 1, 2, gcSL_CG_CONSTANT, 2, 0, max_type0_const_conditionGT },

    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isI2I_Sat_s2u) },
        { -1, 0x0F, 1, 2, gcSL_CG_CONSTANT, 2, 0, min_type0_const_conditionLT },
#endif

#if _SUPPORT_LONG_ULONG_DATA_TYPE
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, _isI2I_int2ulong_sat },
        { -3, 0x31, gcSL_CG_TEMP1, 2, gcSL_CG_CONSTANT, 2, 0, int2ulong_sat_cmp },
        { -2, 0x09, 1, 0, 0, gcSL_CG_TEMP1, 0, uint2longulong_first_mov },
        { -1, 0x09, 1, 0, 0, gcSL_CG_CONSTANT, 0, uint2longulong_second_mov },

    { 1, gcSL_CONV, 1, 2, 0, 0, 0, _isI2I_int2longulong },
        { -3, 0x09, 1, 0, 0, 2, 0, int2longulong_mov },
        { -2, 0x5A, gcSL_CG_TEMP1, 2, 0, gcSL_CG_CONSTANT, 0, int2longulong_rshift },
        { -1, 0x09, 1, 0, 0, gcSL_CG_TEMP1, 0, int2longulong_sign_bit_set },

    { 1, gcSL_CONV, 1, 2, 0, 0, 0, _isI2I_uint2longulong },
        { -2, 0x09, 1, 0, 0, 2, 0, uint2longulong_first_mov },
        { -1, 0x09, 1, 0, 0, gcSL_CG_CONSTANT, 0, uint2longulong_second_mov },

    { 1, gcSL_CONV, 1, 2, 0, 0, 0, _isI2I_longulong2sus },
        { -1, 0x09, 1, 0, 0, 2, 0, longulong2usu_mov },

    { 1, gcSL_CONV, 1, 2, 0, 0, 0, _isI2I_longulongConvert },
        { -2, 0x09, 1, 0, 0, 2, 0, long_ulong_first_mov },
        { -1, 0x09, 1, 0, 0, 2, 0, long_ulong_second_mov },


#endif

    /*
        CONV 1, 2
            I2I 1, 0, 0, 2, 0
    */
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, _isI2I },
        { -1, 0x2C, 1, 2, gcSL_CG_CONSTANT, 0, 0, value_types_I2I },

    /*
        CONV 1, 2
            lshift 1, 2, 0, constant_24_or_16, 0
            rshift 1, 1, 0, constant_24_or_16, 0
    */
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isCL_X_Signed_8_16) },
        { -2, 0x59, 1, 2, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_24_16 },
        { -1, 0x5A, 1, 1, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_24_16 },

    /*
        CONV 1, 2
            and 1, 2, 0, constant_FF_or_FFFF, 0
    */
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isCL_X_Unsigned_8_16) },
        { -1, 0x5D, 1, 2, 0, gcSL_CG_CONSTANT, 0, uint_value_type0_const_FF_FFFF },

    /*
        CONV 1, 2, 3
            and 1, 2, 0, constant_FF, 0
    */
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_is_dest_8bit) },
        { -1, 0x5D, 1, 2, 0, gcSL_CG_CONSTANT, 0, value_type0_const_FF },

    /*
        CONV 1, 2, 3
            lshift 1, 2, 0, constant_8, 0
            rshift 1, 1, 0, constant_8, 0
    */
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_is_dest_16bit_src_int8) },
        { -2, 0x59, 1, 2, 0, gcSL_CG_CONSTANT, 0, short_value_type0_const_8 },
        { -1, 0x5A, 1, 1, 0, gcSL_CG_CONSTANT, 0, short_value_type0_const_8 },

    /*
        CONV 1, 2
            and 1, 2, 0, constant_FFFF, 0
    */
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_is_dest_16bit) },
        { -1, 0x5D, 1, 2, 0, gcSL_CG_CONSTANT, 0, value_type0_const_FFFF },

    /*
        CONV 1, 2
            lshift 1, 2, 0, constant_24, 0
            rshift 1, 1, 0, constant_24, 0
    */
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_is_dest_32bit_src_int8) },
        { -2, 0x59, 1, 2, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_24 },
        { -1, 0x5A, 1, 1, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_24 },

    /*
        CONV 1, 2
            lshift 1, 2, 0, constant_16, 0
            rshift 1, 1, 0, constant_16, 0
    */
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_is_dest_32bit_src_int16) },
        { -2, 0x59, 1, 2, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_16 },
        { -1, 0x5A, 1, 1, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_16 },

    /*
        CONV 1, 2
            conv 1, 0, 0, 2, 0
    */
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, _isF2F },
        { -1, 0x72, 1, 2, gcSL_CG_CONSTANT, 0, 0, value_types_F2F },

    /* conv float16 to float32
            and temp1, 2, 0, float16_exp, 0
            lshift temp1, temp1, 0, float16_man_bits, 0
            select.eq temp2, temp1, 0, float16_bias
            cmp temp3, temp1, float16_exp_isnan
            select.nz, temp2, temp3, temp2, float16_exp_isaddnanNZ
            add temp1, temp1, 0, temp2, 0
            and temp2, 2, 0, float16_sign, 0
            lshift temp2, temp2, 0, float16_exp_bits, 0
            or temp1, temp1, 0, temp2
            and temp2, 2, 0, float16_man
            lshift temp2, temp2, 0, float16_man_bits, 0
            or temp1, temp1, 0, temp2
    */
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isF16_2_F32_hasCMP) },
        { -12, 0x5D, gcSL_CG_TEMP1, 2, 0, gcSL_CG_CONSTANT, 0, float16_exp },
        { -11, 0x59, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, gcSL_CG_CONSTANT, 0, float16_man_bits },
        { -10, 0x0F, gcSL_CG_TEMP2, gcSL_CG_TEMP1, gcSL_CG_CONSTANT, gcSL_CG_CONSTANT, 0, float16_exp_iszero },
        { -9, 0x31, gcSL_CG_TEMP3, gcSL_CG_TEMP1, gcSL_CG_CONSTANT, gcSL_CG_CONSTANT, 0, float16_exp_isnan },
        { -8, 0x0F, gcSL_CG_TEMP2, gcSL_CG_TEMP3, gcSL_CG_TEMP2, gcSL_CG_CONSTANT, 0, float16_exp_isaddnanNZ },
        { -7, 0x01, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, gcSL_CG_TEMP2, 0, value_types_u32 },
        { -6, 0x5D, gcSL_CG_TEMP2, 2, 0, gcSL_CG_CONSTANT, 0, float16_sign },
        { -5, 0x59, gcSL_CG_TEMP2, gcSL_CG_TEMP2, 0, gcSL_CG_CONSTANT, 0, float16_exp_bits },
        { -4, 0x5C, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, gcSL_CG_TEMP2, 0, value_types_u32 },
        { -3, 0x5D, gcSL_CG_TEMP2, 2, 0, gcSL_CG_CONSTANT, 0, float16_man },
        { -2, 0x59, gcSL_CG_TEMP2, gcSL_CG_TEMP2, 0, gcSL_CG_CONSTANT, 0, float16_man_bits },
        { -1, 0x5C, 1, gcSL_CG_TEMP1, 0, gcSL_CG_TEMP2, 0, value_types_u32 },

    /* conv float32 to float16
            and temp1, 2, 0, float32_exp, 0
            select.eq temp2, temp1, 0, float16_exp_iszero
            cmp temp3, temp1, float32_exp_isnan
            select.nz, temp2, temp3, temp2, float16_exp_isaddnanNZ
            add temp1, temp1, 0, -temp2, 0
            rightshift temp1, temp1, 0, float32_man_bits, 0
            and temp2, 2, 0, float32_sign, 0
            rightshift temp2, temp1, 0, float32_exp_bits, 0
            or temp1, temp1, 0, temp2
            and temp2, 2, 0, float32_man
            rightshift temp2, temp2, 0, float32_man_bits, 0
            or temp1, temp1, 0, temp2
    */
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isF32_2_F16_hasCMP) },
        { -12, 0x5D, gcSL_CG_TEMP1, 2, 0, gcSL_CG_CONSTANT, 0, float32_exp },
        { -11, 0x0F, gcSL_CG_TEMP2, gcSL_CG_TEMP1, gcSL_CG_CONSTANT, gcSL_CG_CONSTANT, 0, float16_exp_iszero },
        { -10, 0x31, gcSL_CG_TEMP3, gcSL_CG_TEMP1, gcSL_CG_CONSTANT, gcSL_CG_CONSTANT, 0, float32_exp_isnan },
        { -9, 0x0F, gcSL_CG_TEMP2, gcSL_CG_TEMP3, gcSL_CG_TEMP2, gcSL_CG_CONSTANT, 0, float16_exp_isaddnanNZ },
        { -8, 0x01, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, -gcSL_CG_TEMP2, 0, value_types_u32 },
        { -7, 0x5A, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, gcSL_CG_CONSTANT, 0, float32_man_bits },
        { -6, 0x5D, gcSL_CG_TEMP2, 2, 0, gcSL_CG_CONSTANT, 0, float32_sign },
        { -5, 0x5A, gcSL_CG_TEMP2, gcSL_CG_TEMP2, 0, gcSL_CG_CONSTANT, 0, float32_exp_bits },
        { -4, 0x5C, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, gcSL_CG_TEMP2, 0, value_types_16 },
        { -3, 0x5D, gcSL_CG_TEMP2, 2, 0, gcSL_CG_CONSTANT, 0, float32_man },
        { -2, 0x5A, gcSL_CG_TEMP2, gcSL_CG_TEMP2, 0, gcSL_CG_CONSTANT, 0, float32_man_bits },
        { -1, 0x5C, 1, gcSL_CG_TEMP1, 0, gcSL_CG_TEMP2, 0, value_types_16 },

    /*
        CONV 1, 2
            mov 1, 0, 0, 2, 0
    */
    { 1, gcSL_CONV, 1, 2, 0, 0, 0 },
        { -1, 0x09, 1, 0, 0, 2, 0 },

    { 0 }
};

/* 0x56 gcSL_GETEXP */
const gcsSL_PATTERN patterns_GETEXP[] =
{
    /*
    GETEXP 1, 2
        getexp 1, 0, 0, 2, 0
    */
#if _HAS_GETEXPT_GETMANT_
    { 1, gcSL_GETEXP, 1, 2 },
        { -1, AQ_INST_OP_CODE_GETEXP, 1, 0, 0, 2, 0 },
#else
    /* GC2100 and GC4000 do not have GETEXP. */
    { 1, gcSL_GETEXP, 1, 2 },
        { -3, 0x5D, gcSL_CG_TEMP1, 2, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_7F800000 },
        { -2, 0x5A, gcSL_CG_TEMP2, gcSL_CG_TEMP1, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_23 },
        { -1, 0x01, 1, gcSL_CG_TEMP2, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_Minus127 },
#endif

    { 0 }
};

/* 0x57 gcSL_GETMANT */
const gcsSL_PATTERN patterns_GETMANT[] =
{
    /*
    GETMANT 1, 2
        getmant 1, 0, 0, 2, 0
    */
#if _HAS_GETEXPT_GETMANT_
    { 1, gcSL_GETMANT, 1, 2 },
        { -1, AQ_INST_OP_CODE_GETMANT, 1, 0, 0, 2, 0 },
#else
    /* GC2100 and GC4000 do not have GETMANT. */
    { 1, gcSL_GETMANT, 1, 2 },
        { -2, 0x5D, gcSL_CG_TEMP1, 2, 0, gcSL_CG_CONSTANT, 0, uint_value_type0_const_7FFFFF },
        { -1, 0x5C, 1, gcSL_CG_TEMP1, 0, gcSL_CG_CONSTANT, 0, uint_value_type0_const_800000 },
#endif

    { 0 }
};

/* 0x58 gcSL_MULHI */
const gcsSL_PATTERN patterns_MULHI[] =
{
    /*
        MULHI 1, 2, 3
        ADD 4, 1, 5
            imadhi 4, 2, 3, 5, 0
    */
    { 2, gcSL_MULHI, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_ADD, 4, 1, 5, 0, 0, _NoLabel_isnotCL_X },
        { -1, 0x50, 4, 2, 3, 5, 0, value_type0 },

    /*
        MULHI 1, 2, 3
            imulhi0 1, 2, 3, 0, 0
    */
    { 1, gcSL_MULHI, 1, 2, 3, 0, 0, _IntOpcode },
        { -1, 0x40, 1, 2, 3, 0, 0, value_type0 },

    { 0 }
};

/* 0x59 gcSL_CMP */
const gcsSL_PATTERN patterns_CMP[] =
{
    /*
        CMP 1, 2, 3
        SET.z  4, 1, 5
        SET.nz 4, 1, 3
            select.nz 4, 2, 3, 5, 0
    */
    { 3, gcSL_CMP, 1, 2, 3, 0, 0, _UseDestInNextTwoOnly },
    { 2, gcSL_SET, 4, 1, 5, 0, 0, _IsZ },
    { 1, gcSL_SET, 4, 1, 3, 0, 0, _IsNZ },
        { -1, 0x0F, 4, 2, 3, 5, 0, conditionNZ_value_type0_from_src0 },

    /*
        CMP 1, 2, 3
        SET.z  4, 1, 1
        SET.nz 4, 1, 5
            cmp 4, 2, 3, 5, 0
    */
    { 3, gcSL_CMP, 1, 2, 3, 0, 0, _UseDestInNextTwoOnly },
    { 2, gcSL_SET, 4, 1, 1, 0, 0, _IsZ },
    { 1, gcSL_SET, 4, 1, 5, 0, 0, _IsNZ },
        { -1, 0x31, 4, 2, 3, 5, 0, value_type0_from_src0 },

    /*
        CMP 1, 2, 3
        CMP.z  4, 1, 1
        CMP.nz 4, 1, 5
            cmp 4, 2, 3, 5, 0
    */
    { 3, gcSL_CMP, 1, 2, 3, 0, 0, _UseDestInNextTwoOnly },
    { 2, gcSL_CMP, 4, 1, 1, 0, 0, _IsZ },
    { 1, gcSL_CMP, 4, 1, 5, 0, 0, _IsNZ },
        { -1, 0x31, 4, 2, 3, 5, 0, value_type0_from_src0 },

    /*
        CMP 1, 2, 3
        CMP.nz 4, 1, 5
        CMP.z  4, 1, 1
            cmp 4, 2, 3, 5, 0
    */
    { 3, gcSL_CMP, 1, 2, 3, 0, 0, _UseDestInNextTwoOnly },
    { 2, gcSL_CMP, 4, 1, 5, 0, 0, _IsNZ },
    { 1, gcSL_CMP, 4, 1, 1, 0, 0, _IsZ },
        { -1, 0x31, 4, 2, 3, 5, 0, value_type0_from_src0 },

    /*
        CMP 1, 2, 3
        CMP.z  4, 1, 5
        CMP.nz 4, 1, 1
            cmp 4, 2, 5, 3, 0
    */
    { 3, gcSL_CMP, 1, 2, 3, 0, 0, _UseDestInNextTwoOnly },
    { 2, gcSL_CMP, 4, 1, 5, 0, 0, _IsZ },
    { 1, gcSL_CMP, 4, 1, 1, 0, 0, _IsNZ },
        { -1, 0x31, 4, 2, 5, 3, 0, value_type0_from_src0 },

    /*
        CMP 1, 2, 3
        CMP.nz 4, 1, 1
        CMP.z  4, 1, 5
            cmp 4, 2, 5, 3, 0
    */
    { 3, gcSL_CMP, 1, 2, 3, 0, 0, _UseDestInNextTwoOnly },
    { 2, gcSL_CMP, 4, 1, 1, 0, 0, _IsNZ },
    { 1, gcSL_CMP, 4, 1, 5, 0, 0, _IsZ },
        { -1, 0x31, 4, 2, 5, 3, 0, value_type0_from_src0 },

     /*
        CMP 1, 2, 3
        CMP.z  4, 1, 4
        CMP.nz 4, 1, 3
            select 4, 2, 3, 4
    */
    { 3, gcSL_CMP, 1, 2, 3, 0, 0, _UseDestInNextTwoOnly },
    { 2, gcSL_CMP, 4, 1, 4, 0, 0, _IsZ },
    { 1, gcSL_CMP, 4, 1, 3, 0, 0, _IsNZ },
        { -1, 0x0F, 4, 2, 3, 4, 0, value_type0_from_src0 },

    /*
        CMP 1, 2, 3
        CMP.z  4, 1, 4
        CMP.nz 4, 1, 5
            cmp temp1, 2, 3,
            select.nz 4, temp1, 5, 4
    */
    { 3, gcSL_CMP, 1, 2, 3, 0, 0, _UseDestInNextTwoOnly },
    { 2, gcSL_CMP, 4, 1, 4, 0, 0, _IsZ },
    { 1, gcSL_CMP, 4, 1, 5, 0, 0, _IsNZ },
        { -2, 0x31, gcSL_CG_TEMP1, 2, 3, gcSL_CG_CONSTANT, 0, minusOne_2_value_type0_from_src0 },
        { -1, 0x0F, 4, gcSL_CG_TEMP1, 5, 4, 0, conditionNZ },

    /*
        CMP.sz  1, 2, 3
        CMP.nsz 1, 2, 4
            select.z 1, 2, 3, 4, 0
    */
    { 2, gcSL_CMP, 1, 2, 3, 0, 0, _ComparingWithZ },
    { 1, gcSL_CMP, 1, 2, 4, 0, 0, _ConditionReversedWithPrev },
        { -1, 0x0F, 1, 2, 3, 4, 0, first_condition_value_type0_from_src0 },

    /*
        CMP.sz  1, 2, 2
        CMP.nsz 1, 2, 4
            select.z 1, 2, 2, 4, 0
    */
    { 2, gcSL_CMP, 1, 2, 2, 0, 0, _ComparingWithZ },
    { 1, gcSL_CMP, 1, 2, 4, 0, 0, _ConditionReversedWithPrev },
        { -1, 0x0F, 1, 2, 2, 4, 0, first_condition_value_type0_from_src0 },

    /*
        CMP.sz  1, 2, 3
        CMP.nsz 1, 2, 2
            select.z 1, 2, 3, 2, 0
    */
    { 2, gcSL_CMP, 1, 2, 3, 0, 0, _ComparingWithZ },
    { 1, gcSL_CMP, 1, 2, 2, 0, 0, _ConditionReversedWithPrev },
        { -1, 0x0F, 1, 2, 3, 2, 0, first_condition_value_type0_from_src0 },

    /*
        CMP.allmsb  1, 2, 3
        CMP.allmsb  1, 2, 4
            select.allmsb 1, 2, 3, 4, 0
    */
    { 2, gcSL_CMP, 1, 2, 3, 0, 0, _MSBCompare },
    { 1, gcSL_CMP, 1, 2, 4, 0, 0, _ConditionSameAsPrev },
        { -1, 0x0F, 1, 2, 3, 4, 0, first_condition_value_type0_from_src0 },
    /*
        CMP 1, 2, 3  is_float(dest.type)
        cmp 1, 2, 3, one, 0
    */
    { 1, gcSL_CMP, 1, 2, 3, 0, 0, _isDstFloat},
        { -1, 0x31, 1, 2, 3, gcSL_CG_CONSTANT, 0, One_2_value_type0_from_src0 },

    /*
        CMP 1, 2, 3
            cmp 1, 2, 3, minus_one, 0
    */
    { 1, gcSL_CMP, 1, 2, 3 },
        { -1, 0x31, 1, 2, 3, gcSL_CG_CONSTANT, 0, minusOne_2_value_type0_from_src0 },

    { 0 }
};

/* 0x5A gcSL_I2F */
const gcsSL_PATTERN patterns_I2F[] =
{
    /*
        I2F 1, 2
            I2F 1, 0, 0, 2, 0
    */
    { 1, gcSL_I2F, 1, 2, 0, 0, 0, _hasRounding_mode },
        { -1, 0x2D, 1, 2, 0, 0, 0, rounding_mode_value_type0_from_src0 },

    /*
        I2F 1, 2
            I2F 1, 0, 0, 2, 0
    */
    { 1, gcSL_I2F, 1, 2, 0, 0, 0, _hasInteger },
        { -1, 0x2D, 1, 2, 0, 0, 0, value_type0_from_src0 },

    { 0 }
};

/* 0x5B gcSL_F2I */
const gcsSL_PATTERN patterns_F2I[] =
{
    /*
        F2I 1, 2, 3
            F2I 1, 2, 3, 0, 0
    */
    { 1, gcSL_F2I, 1, 2, 3, 0, 0, _hasInteger },
        { -1, 0x2E, 1, 2, 3, 0, 0, value_type0 },

    /*
        F2I 1, 2
            SIGN            temp1, 2
            ABS             temp2, 2
            FLOOR           temp2, temp2
            MUL             1, temp1, temp2
    */
    { 1, gcSL_F2I, 1, 2, 0, 0, 0 },
        { -4, 0x27, gcSL_CG_TEMP1, 0, 0, 2, 0, value_type0 },
        { -3, 0x0F, gcSL_CG_TEMP2, 2, -2, 2, 0, conditionLT },
        { -2, 0x25, gcSL_CG_TEMP2, 0, 0, gcSL_CG_TEMP2, 0 },
        { -1, 0x03, 1, gcSL_CG_TEMP1, gcSL_CG_TEMP2, 0, 0, 0 },


    { 0 }
};

/* 0x5C gcSL_ADDSAT */
const gcsSL_PATTERN patterns_ADDSAT[] =
{
    { 1, gcSL_ADDSAT, 1, 2, 3, 0, 0, _IntOpcode },
        { -1, 0x3B, 1, 2, 0, 3, 0, value_type0 },
    /*
        ADDSAT 1, 2, 3
            iaddsat 1, 2, 0, 3, 0
    */
    { 1, gcSL_ADDSAT, 1, 2, 3 },
        { -1, 0x3B, 1, 2, 0, 3, 0, add2mad },

    { 0 }
};

/* 0x5D gcSL_SUBSAT */
const gcsSL_PATTERN patterns_SUBSAT[] =
{
    { 1, gcSL_SUBSAT, 1, 2, 3, 0, 0, _IntOpcode },
        { -1, 0x3B, 1, 2, 0, -3, 0, value_type0 },

    /*
        SUBSAT 1, 2, 3
            iaddsat 1, 2, 0, -3, 0
    */
    { 1, gcSL_SUBSAT, 1, 2, 3 },
        { -1, 0x3B, 1, 2, 0, -3, 0 },

    { 0 }
};

/* 0x5E gcSL_MULSAT */
const gcsSL_PATTERN patterns_MULSAT[] =
{
    /*
        MULSAT 1, 2, 3
        ADDSAT 4, 5, 1
            imadsat 4, 2, 3, 5, 0
    */
    { 2, gcSL_MULSAT, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_ADDSAT, 4, 5, 1, 0, 0, _NoLabel },
        { -1, 0x4E, 4, 2, 3, 5, 0, value_type0 },

    /*
        MULSAT 1, 2, 3
        ADDSAT 4, 1, 5
            imadsat 4, 2, 3, 5, 0
    */
    { 2, gcSL_MULSAT, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_ADDSAT, 4, 1, 5, 0, 0, _NoLabel },
        { -1, 0x4E, 4, 2, 3, 5, 0, value_type0 },

    /*
    temporary use, MULSAD+MULSAD = MAD0 | MAD1, for short-integer
        MULSAT 1, 2, 3
        MULSAT 4, 1, 5
            imadsat0 temp1, 2, 3, 5, 0
            imadsat1 4, 2, 3, 5, 0
            add 4, temp1, 4

    */
    { 2, gcSL_MULSAT, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_MULSAT, 4, 1, 5, 0, 0, _NoLabel },
        { -3, 0x4E, gcSL_CG_TEMP1, 2, 3, 5, 0, value_type0 },
        { -2, 0x4F, 4, 2, 3, 5, 0, value_type0 },
        { -1, 0x5C, 4, 4, 0, gcSL_CG_TEMP1, 0, value_type0 },

#if _SUPPORT_INTEGER_NEGATIVE_MODIFIER_
    /*
        MULSAT 1, 2, 3
        SUBSAT 4, 1, 5
            imadsat 4, 2, 3, -5, 0
    */
    { 2, gcSL_MULSAT, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_SUBSAT, 4, 1, 5, 0, 0, _NoLabel },
        { -1, 0x4E, 4, 2, 3, -5, 0, value_type0},
#else
    /* Negative modifier is not allowed in imadlo0. */
    /*
        MULSAT 1, 2, 3
        SUBSAT 4, 1, 5
            imulsat 1, 2, 3, 0, 0
            iaddsat 4, 1, 0, -5, 0
    */
    { 2, gcSL_MULSAT, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_SUBSAT, 4, 1, 5, 0, 0, _NoLabel },
        { -2, 0x3E, 1, 2, 3, 0, 0, value_type0 },
        { -1, 0x3B, 4, 1, 0, -5, 0, value_type0 },
#endif

#if _SUPPORT_INTEGER_NEGATIVE_MODIFIER_
    /*
        MULSAT 1, 2, 3
        SUBSAT 4, 5, 1
            imadsat 4, -2, 3, 5, 0
    */
    { 2, gcSL_MULSAT, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_SUBSAT, 4, 5, 1, 0, 0, _NoLabel },
        { -1, 0x4E, 4, -2, 3, 5, 0, value_type0 },
#else
    /* Negative modifier is not allowed in imadlo0. */
    /*
        MULSAT 1, 2, 3
        SUBSAT 4, 5, 1
            imulsat 1, 2, 3, 0, 0
            iaddsat 4, 5, 0, -1, 0
    */
    { 2, gcSL_MULSAT, 1, 2, 3, 0, 0, _UseDestInNextOnly },
    { 1, gcSL_SUBSAT, 4, 5, 1, 0, 0, _NoLabel },
        { -2, 0x3E, 1, 2, 3, 0, 0, value_type0 },
        { -1, 0x3B, 4, 5, 0, -1, 0, value_type0 },
#endif

    /*
        MULSAT 1, 2, 3
            imulsat 1, 2, 3, 0, 0
    */
    { 1, gcSL_MULSAT, 1, 2, 3 },
        { -1, 0x3E, 1, 2, 3, 0, 0, value_type0 },

    { 0 }
};

/* 0x5F gcSL_DP2 */
const gcsSL_PATTERN patterns_DP2[] =
{
#if (GC_ENABLE_DUAL_FP16 > 0)
    /*
        DP2  1, 2, 3
        SQRT 4, 1
            mul.t0 1.hp, 2, 3, 0, 0
            add.t0 1.hp, 1.hp.x, 0, 1.hp.y, 0
            sqrt.t0 4, 0, 0, 1.hp.x
            mul.t1 1.hp, 2, 3, 0, 0
            add.t1 1.hp, 1.hp.x, 0, 1.hp.y, 0
            sqrt.t1 4, 0, 0, 1.hp.x
    */
    { 2, gcSL_DP2, 1, 2, 3, 0, 0, _UseDestInNextOnly_Dual16OnMediumpSrc0Src1 },
    { 1, gcSL_SQRT, 4, 1 },
        { -6, 0x03, gcSL_CG_TEMP1_XYZW, 2, 3, 0, 0, _t0_destHP },
        { -5, 0x01, gcSL_CG_TEMP2_X, gcSL_CG_TEMP1_XYZW, 0, gcSL_CG_TEMP1_XYZW, 0, swizzleS0xS2yWithT0HpHpHp },
        { -4, 0x21, 4, 0, 0, gcSL_CG_TEMP2, 0, _t0_src2HP },
        { -3, 0x03, gcSL_CG_TEMP1_XYZW, 2, 3, 0, 0, _t1_destHP },
        { -2, 0x01, gcSL_CG_TEMP2_X, gcSL_CG_TEMP1_XYZW, 0, gcSL_CG_TEMP1_XYZW, 0, swizzleS0xS2yWithT1HpHpHp },
        { -1, 0x21, 4, 0, 0, gcSL_CG_TEMP2, 0, _t1_src2HP },
#endif
    /*
        DP2  1, 2, 3
            mul  TEMP, 2, 3, 0, 0
            add  1, TEMP.x, TEMP.y
    */
    { 1, gcSL_DP2, 1, 2, 3 },
        { -2, 0x03, gcSL_CG_TEMP1_XYZW, 2, 3, 0, 0, 0 },
        { -1, 0x01, 1, gcSL_CG_TEMP1_XYZW, 0, gcSL_CG_TEMP1_XYZW, 0, swizzleS0xS2y },

    { 0 }
};

/* 0x60 gcSL_UNPACK */
const gcsSL_PATTERN patterns_UNPACK[] =
{
    /*  UNPACK 1, 2, 3
     *  AND_BITWISE 4, 1, 5
     *      swizzle 4, 2, 3, 5, 0
     */
    { 2, gcSL_UNPACK, 1, 2, 3 },
    { 1, gcSL_AND_BITWISE, 4, 1, 5 },
    {   -1, 0x2B, 4, 2, 3, 5, 0, value_type0 },

    { 0 }
};

/* 0x61 gcSL_IMAGE_WR */
const gcsSL_PATTERN    patterns_IMAGE_WR[] =
{
    /*
        IMAGE_WR    1, 2, 3
            img_store   1, 2, 3
    */
    { 1, gcSL_IMAGE_WR, 1, 2, 3, 0, 0 },
        { -1, 0x7A, 0, 2, 3, 1, 0, setAllComponentsEnable},

    {0}
};

/* 0x62 gcSL_SAMPLER_ADD */
const gcsSL_PATTERN    patterns_SAMPLER_ADD[] =
{
    {0}
};

/* 0x63 gcSL_MOVA */
const gcsSL_PATTERN    patterns_MOVA[] =
{
    /*
        MOVA 1, 2
            MOVAF 1, 2, 0, 0, 0
    */
    { 1, gcSL_MOVA, 1, 2, 0, 0, 0, _is_value_type_float },
        { -1, 0x0B, 0, 0, 0, 2, 0, set_mova },

    /*
        MOVA 1, 2
            MOVAI 1, 2, 0, 0, 0
    */
    { 1, gcSL_MOVA, 1, 2, 0, 0, 0, _is_value_type_integer },
        { -1, 0x56, 0, 0, 0, 2, 0, set_mova },
    {0}
};

/* 0x64 gcSL_IMAGE_RD */
const gcsSL_PATTERN    patterns_IMAGE_RD[] =
{
    /*
        IMAGE_RD    1, 2, 3
            img_load   1, 2, 3
    */
    { 1, gcSL_IMAGE_RD, 1, 2, 3, 0, 0 },
        { -1, 0x79, 1, 2, 3, 0, 0, value_type0_from_src0},

    {0}
};

/* 0x65 gcSL_IMAGE_SAMPLER */
const gcsSL_PATTERN    patterns_IMAGE_SAMPLER[] =
{
    /*
        IMAGE_SAMPLER  x, x, x
        IMAGE_RD       1, 2, 3
            img_load      1, 2, 3
    */
    { 2, gcSL_IMAGE_SAMPLER, 1, 2, 3 },
    { 1, gcSL_IMAGE_RD, 4, 5, 6, 0, 0 },
        { -1, 0x79, 4, 5, 6, 0, 0, value_type0_from_src0},

    /*
        IMAGE_SAMPLER  x, x, x
        IMAGE_WR       1, 2, 3
            img_store     1, 2, 3
    */
    { 2, gcSL_IMAGE_SAMPLER, 1, 2, 3 },
    { 1, gcSL_IMAGE_WR, 4, 5, 6, 0, 0 },
        { -1, 0x7A, 0, 5, 6, 4, 0, setAllComponentsEnable},

    {0}
};

/* 0x66 gcSL_NORM_MUL */
const gcsSL_PATTERN    patterns_NORM_MUL[] =
{
    /*
        NORM_MUL 1, 2, 3
            norm_mul 1, 2, 3, 0, 0
    */
    { 1, gcSL_NORM_MUL, 1, 2, 3, 0, 0, _HasPreNormInst },
        { -1, 0x77, 1, 2, 3, 0, 0, set_norm_mul0zero },
    {0}
};

/* 0x67 gcSL_NORM_DP2 */
const gcsSL_PATTERN    patterns_NORM_DP2[] =
{
    /*
        NORM_DP2 1, 2, 0
            norm_dp2 1, 2, 0, 0, 0
    */
    { 1, gcSL_NORM_DP2, 1, 2, 0, 0, 0, _HasPreNormInst },
        { -1, 0x74, 1, 2, 0, 0, 0, 0 },
    {0}
};

/* 0x68 gcSL_NORM_DP3 */
const gcsSL_PATTERN    patterns_NORM_DP3[] =
{
    /*
        NORM_DP3 1, 2, 0
            norm_dp3 1, 2, 0, 0, 0
    */
    { 1, gcSL_NORM_DP3, 1, 2, 0, 0, 0, _HasPreNormInst },
        { -1, 0x75, 1, 2, 0, 0, 0, 0 },
    {0}
};

/* 0x69 gcSL_NORM_DP4 */
const gcsSL_PATTERN    patterns_NORM_DP4[] =
{
    /*
        NORM_DP4 1, 2, 0
            norm_dp4 1, 2, 0, 0, 0
    */
    { 1, gcSL_NORM_DP4, 1, 2, 0, 0, 0, _HasPreNormInst },
        { -1, 0x76, 1, 2, 0, 0, 0, 0 },
    {0}
};

/* 0x6A gcSL_PRE_DIV */
const gcsSL_PATTERN    patterns_PRE_DIV[] =
{
    /*
        DIV 1, 2, 3
            div 3, 0, 0, 2, 0
    */
    { 1, gcSL_PRE_DIV, 1, 2, 3, 0, 0, _hasNEW_SIN_COS_LOG_DIV },
            { -1, 0x64, 1, 0, 2, 3, 0, set_new_sin_cos_log_div },

    {0}
};

/* 0x6B gcSL_PRE_LOG2 */
const gcsSL_PATTERN    patterns_PRE_LOG2[] =
{
    /*
        PRE_LOG2 1, 2
        MUL      4, 1, 3
            log 1, 0, 0, 2, 0
            mul 4, 1, 3
    */
    { 2, gcSL_PRE_LOG2, 1, 2, 0, 0, 0, 0 },
    { 1, gcSL_MUL, 4, 1, 3, 0, 0, isSource1_RCP_OF_LOG2_E  },
        { -2, 0x12, 1, 0, 0, 2, 0, set_new_sin_cos_log_div },
        { -1, 0x03, 4, 1, 3, 0, 0, rtne },

    /*
        PRE_LOG2 1, 2
            log 1, 0, 0, 2, 0
    */
    { 1, gcSL_PRE_LOG2, 1, 2, 0, 0, 0, 0 },
        { -1, 0x12, 1, 0, 0, 2, 0, set_new_sin_cos_log_div },
    {0}
};

/* 0x6C gcSL_TEXGATHER */
const gcsSL_PATTERN patterns_TEXGATHER[] =
{
    /*
        TEXGATHER  0, 4, 5
        TEXLD      1, 2, 3
            texld_gather 1, 3, 4, 0, 2
    */
    { 2, gcSL_TEXGATHER, 0, 4, 5, 0, 0, _hasHalti4 },
    { 1, gcSL_TEXLD, 1, 2, 3 },
        { -1, 0x7D, 1, 3, 4, 0, 2 },

#if _CLAMP_PCF_REFERENCE_
    /*
        TEXGATHER  0, 4, 5
        TEXLDPCF   1, 2, 3
            mov.sat TEMP1, 5
            texld_gather 1, 3, 4, TEMP1, 2
    */
    { 2, gcSL_TEXGATHER, 0, 4, 5, 0, 0, _hasHalti4 },
    { 1, gcSL_TEXLDPCF, 1, 2, 3 },
        { -2, 0x09, gcSL_CG_TEMP1, 0, 0, 5, 0, saturate },
        { -1, 0x7D, 1, 3, 4, gcSL_CG_TEMP1_X, 2 },
#else
    /*
        TEXGATHER  0, 4, 5
        TEXLDPCF   1, 2, 3
            texld_gather 1, 3, 4, 5, 2
    */
    { 2, gcSL_TEXGATHER, 0, 4, 5, 0, 0, _hasHalti4 },
    { 1, gcSL_TEXLDPCF, 1, 2, 3 },
        { -1, 0x7D, 1, 3, 4, 5, 2 },
#endif

    /*
    ** If there is a gather exist on a chip that can't support halti4,
    ** it must be introduced by recompiler and not optimized due to instruction code limitation,
    ** it would never be executed, so we can just convert it to NOP.
    */
    { 2, gcSL_TEXGATHER, 0, 4, 5 },
    { 1, gcSL_TEXLDPCF, 1, 2, 3 },
        { -1, 0x00, 0, 0, 0, 0, 0 },

    { 2, gcSL_TEXGATHER, 0, 4, 5 },
    { 1, gcSL_TEXLD, 1, 2, 3 },
        { -1, 0x00, 0, 0, 0, 0, 0 },

    { 0 }
};

/* 0x6D gcSL_TEXFETCH_MS */
const gcsSL_PATTERN patterns_TEXFETCH_MS[] =
{
    /*
        TEXFETCH_MS  0, 2, 4
        TEXLD    1, 2, 3
            texld_fetchMS 1, 3, 4, 0, 2
    */
    { 2, gcSL_TEXFETCH_MS, 0, 2, 4, 0, 0, _hasHalti4 },
    { 1, gcSL_TEXLD, 1, 2, 3, 0, 0 },
        { -1, 0x7F, 1, 3, 4, 0, 2, set_extended_opcode_fetchms },

    { 0 }
};

/* 0x6E gcSL_POPCOUNT */
const gcsSL_PATTERN patterns_POPCOUNT[] =
{
    /*
        POPCOUNT    1, 2
            popcount 1, 0, 0, 2
    */
    { 1, gcSL_POPCOUNT, 1, 2, 0, 0, 0, _hasHalti1 },
        { -1, 0x61, 1, 0, 0, 2, 0, value_type0 },

    { 0 }
};

/* 0x6F gcSL_BIT_REVERSAL */
const gcsSL_PATTERN patterns_BIT_REVERSAL[] =
{
    /*
        BIT_REVERSAL    1, 2
            bit_reversal 1, 2
    */
    { 1, gcSL_BIT_REVERSAL, 1, 2, 0, 0, 0, _hasHalti2 },
        { -1, 0x6D, 1, 2, 0, 0, 0, value_type0 },

    { 0 }
};

/* 0x70 gcSL_BYTE_REVERSAL */
const gcsSL_PATTERN patterns_BYTE_REVERSAL[] =
{
    /*
        BYTE_REVERSAL    1, 2
            byte_reversal 1, 2
    */
    { 1, gcSL_BYTE_REVERSAL, 1, 2, 0, 0, 0, _hasHalti2 },
        { -1, 0x6E, 1, 2, 0, 0, 0, value_type0 },

    { 0 }
};

/* 0x71 gcSL_TEXPCF */
const gcsSL_PATTERN patterns_TEXPCF[] =
{
#if _CLAMP_PCF_REFERENCE_
    /*
        TEXPCF  0, 2, 4
        TEXLD   1, 2, 3
            mov.sat TEMP1, 4
            texld_gather 1, 3, 4, TEMP1, 2
    */
    { 2, gcSL_TEXPCF, 0, 2, 4, 0, 0, _hasHalti4 },
    { 1, gcSL_TEXLD, 1, 2, 3 },
        { -2, 0x09, gcSL_CG_TEMP1, 0, 0, 4, 0, saturate },
        { -1, 0x7D, 1, 3, 0, gcSL_CG_TEMP1_X, 2 },
#else
    /*
        TEXPCF  0, 2, 4
        TEXLD   1, 2, 3
            texld_gather 1, 3, const_0, 4, 2
    */
    { 2, gcSL_TEXPCF, 0, 2, 4, 0, 0, _hasHalti4 },
    { 1, gcSL_TEXLD, 1, 2, 3 },
        { -1, 0x7D, 1, 3, 0, 4, 2 },
#endif

    { 0 }
};

/* 0x72 gcSL_UCARRY */
const gcsSL_PATTERN patterns_UCARRY[] =
{
    /*
        UCARRY  1, 2, 3
            set.ucarry 1, 2, 3
    */
    { 1, gcSL_UCARRY, 1, 2, 3, 0, 0, _hasHalti4 },
        { -1, 0x10, 1, 2, 3, 0, 0, conditionUCARRY},

    { 0 }
};

/* 0x73 gcSL_TEXU */
const gcsSL_PATTERN patterns_TEXU[] =
{
    { 2, gcSL_TEXU, 1, 2, 0, 0, 0, _hasHalti4 },
    { 1, gcSL_TEXLD, 1, 3, 4, 0, 0 },
        { -1, 0x7B, 1, 4, 0, 2, 3, 0},

    { 0 }
};

/* 0x74 gcSL_TEXU_LOD */
const gcsSL_PATTERN patterns_TEXU_LOD[] =
{
    { 2, gcSL_TEXU_LOD, 1, 2, 3, 0, 0, _hasHalti4 },
    { 1, gcSL_TEXLD, 1, 4, 5, 0, 0 },
        { -1, 0x7B, 1, 5, 2, 3, 4, 0},

    { 0 }
};

/* 0x75 gcSL_MEM_BARRIER*/
const gcsSL_PATTERN patterns_MEM_BARRIER[] =
{
    /*
        BARRIER 1
            barrier 0, 0, 0, 0, 0
    */
    { 1, gcSL_MEM_BARRIER },
        { -1, 0x00, 0, 0, 0, 0, 0 },

    { 0 }
};

/* 0x76 gcSL_SAMPLER_ASSIGN*/
const gcsSL_PATTERN patterns_SAMPLER_ASSIGN[] =
{
    /*
        SAMPLER_ASSIGN 1, 2
            mov 1, 0, 0, 2, 0
    */
    { 1, gcSL_SAMPLER_ASSIGN, 1, 2 },
        { -1, 0x09, 1, 0, 0, 2, 0 },

    { 0 }
};

/* 0x77  Get Image/Sampler index */
const gcsSL_PATTERN patterns_GET_SAMPLER_IDX[] =
{
    { 0 }
};

/* 0x78 gcSL_IMAGE_RD_3D */
const gcsSL_PATTERN    patterns_IMAGE_RD_3D[] =
{
    /*
        IMAGE_RD    1, 2, 3
            img_load_3d   1, 2, 3
    */
    { 1, gcSL_IMAGE_RD_3D, 1, 2, 3 },
        { -1, 0x34, 1, 2, 3, 0, 0, value_type0_from_src0},

    {0}
};


/* 0x79 gcSL_IMAGE_WR_3D */
const gcsSL_PATTERN    patterns_IMAGE_WR_3D[] =
{
    /*
        IMAGE_WR    1, 2, 3
            img_store_3D   1, 2, 3
    */
    { 1, gcSL_IMAGE_WR_3D, 1, 2, 3 },
        { -1, 0x35, 0, 2, 3, 1, 0, setAllComponentsEnable},

    {0}

};

/* 0x7A gcSL_CLAMP0MAX */
const gcsSL_PATTERN    patterns_CLAMP0MAX[] =
{
    /*
        CLAMP0MAX    1, 2, 3
            CLAMP0MAX   1, 2, 3, 0
    */
    { 1, gcSL_CLAMP0MAX, 1, 2, 3 },
        { -1, 0x36, 1, 2, 3, gcSL_CG_CONSTANT, 0, zero_2},

    {0}
};

/* 0x7B gcSL_FMA_MUL first part: MUL */
const gcsSL_PATTERN patterns_FMA_MUL[] =
{
    { 0 }
};

/* 0x7C gcSL_FMA_ADD, FMA second part: ADD */
const gcsSL_PATTERN patterns_FMA_ADD[] =
{
    { 0 }
};

/* 0x7D gcSL_ATTR_ST */
const gcsSL_PATTERN patterns_ATTR_ST[] =
{
    { 0 }
};

/* 0x7E gcSL_ATTR_LD */
const gcsSL_PATTERN patterns_ATTR_LD[] =
{
    { 0 }
};

/* 0x7F For function "EmitVertex" */
const gcsSL_PATTERN patterns_EMIT_VERTEX[] =
{
    { 0 }
};

/* 0x80 For function "EndPrimitive" */
const gcsSL_PATTERN patterns_END_PRIMITIVE[] =
{
    { 0 }
};

/* 0x81 gcSL_ARCTRIG0 */
const gcsSL_PATTERN patterns_ARCTRIG0[] =
{
    { 0 }
};

/* 0x82 gcSL_ARCTRIG1 */
const gcsSL_PATTERN patterns_ARCTRIG1[] =
{
    { 0 }
};

/* 0x83 gcSL_MUL_Z */
const gcsSL_PATTERN patterns_MUL_Z[] =
{
    { 0 }
};

/* 0x84 gcSL_NEG */
const gcsSL_PATTERN patterns_NEG[] =
{
    /*
        NEG 1, 2
            mov 1, 0, 0, -2, 0
    */
    { 1, gcSL_NEG, 1, 2, 0, 0, 0, _is_value_type_float },
        { -1, 0x09, 1, 0, 0, -2, 0 },

    /*
        NEG 1, 2
            add 1, 0, constant_0, -2, 0
    */
    { 1, gcSL_NEG, 1, 2 },
        { -1, 0x01, 1, 0, gcSL_CG_CONSTANT, -2, 0, value_type0_immediate_or_const_0 },

    { 0 }
};

/* 0x85 gcSL_LONGLO */
const gcsSL_PATTERN patterns_LONGLO[] =
{
#if _SUPPORT_LONG_ULONG_DATA_TYPE
    /*
        LONGLO 1, 2
            mov 1, 0, 0, 2, 0
    */
    { 1, gcSL_LONGLO, 1, 2, 0, 0, 0 },
        { -1, 0x09, 1, 0, 0, 2, 0, long_ulong_lower },
#endif
    { 0 }
};

/* 0x86 gcSL_LONGHI */
const gcsSL_PATTERN patterns_LONGHI[] =
{
#if _SUPPORT_LONG_ULONG_DATA_TYPE
    /*
        LONGHI 1, 2
            mov 1, 0, 0, 2, 0
    */
    { 1, gcSL_LONGHI, 1, 2, 0, 0, 0 },
        { -1, 0x09, 1, 0, 0, 2, 0, long_ulong_upper },
#endif

    { 0 }
};

/* 0x87 gcSL_MOV_LONG */
const gcsSL_PATTERN patterns_MOV_LONG[] =
{
#if _SUPPORT_LONG_ULONG_DATA_TYPE
    /*
        MOV_LONG 1, 2, 3
            mov 1, 0, 0, 2, 0
    */
    { 1, gcSL_MOV_LONG, 1, 2, 3, 0, 0 },
        { -2, 0x09, 1, 0, 0, 2, 0, long_ulong_set_lower },
        { -1, 0x09, 1, 0, 0, 3, 0, long_ulong_set_upper },
#endif

    { 0 }
};

/* 0x88 gcSL_MADSAT */
const gcsSL_PATTERN patterns_MADSAT[] =
{
    { 0 }
};

/* 0x89 gcSL_COPY */
const gcsSL_PATTERN patterns_COPY[] =
{
    { 0 }
};

/* 0x8A gcSL_IMAGE_ADDR_3D */
const gcsSL_PATTERN patterns_IMAGE_ADDR_3D[] =
{
    /*
        IMAGE_ADDR    1, 2, 3
            img_addr_3d   1, 2, 3
    */
    { 1, gcSL_IMAGE_ADDR_3D, 1, 2, 3, 0, 0, _hasHalti4 },
        { -1, 0x38, 1, 2, 3, 0, 0, value_types_u32},

    { 0 }
};

/* 0x8B  Get sampler's lodminmax */
const gcsSL_PATTERN patterns_GET_SAMPLER_LMM[] =
{
    { 0 }
};

/* 0x8C  Get sampler's levelbasesize */
const gcsSL_PATTERN patterns_GET_SAMPLER_LBS[] =
{
    { 0 }
};

const gcsSL_PATTERN_PTR  patterns[] =
{
    patterns_NOP, /* 0x00 gcSL_NOP */
    patterns_MOV, /* 0x01 gcSL_MOV */
    patterns_SAT, /* 0x02 gcSL_SAT */
    patterns_DP3, /* 0x03 gcSL_DP3 */
    patterns_DP4, /* 0x04 gcSL_DP4 */
    patterns_ABS, /* 0x05 gcSL_ABS */
    patterns_JMP, /* 0x06 gcSL_JMP */
    patterns_ADD, /* 0x07 gcSL_ADD */
    patterns_MUL, /* 0x08 gcSL_MUL */
    patterns_RCP, /* 0x09 gcSL_RCP */
    patterns_SUB, /* 0x0A gcSL_SUB */
    patterns_KILL, /* 0x0B gcSL_KILL */
    patterns_TEXLD, /* 0x0C gcSL_TEXLD */
    patterns_CALL, /* 0x0D gcSL_CALL */
    patterns_RET, /* 0x0E gcSL_RET */
    patterns_NORM, /* 0x0F gcSL_NORM */
    patterns_MAX, /* 0x10 gcSL_MAX */
    patterns_MIN, /* 0x11 gcSL_MIN */
    patterns_POW, /* 0x12 gcSL_POW */
    patterns_RSQ, /* 0x13 gcSL_RSQ */
    patterns_LOG, /* 0x14 gcSL_LOG */
    patterns_FRAC, /* 0x15 gcSL_FRAC */
    patterns_FLOOR, /* 0x16 gcSL_FLOOR */
    patterns_CEIL, /* 0x17 gcSL_CEIL */
    patterns_CROSS, /* 0x18 gcSL_CROSS */
    patterns_TEXLDPROJ, /* 0x19 gcSL_TEXLDPROJ */
    patterns_TEXBIAS, /* 0x1A gcSL_TEXBIAS */
    patterns_TEXGRAD, /* 0x1B gcSL_TEXGRAD */
    patterns_TEXLOD, /* 0x1C gcSL_TEXLOD */
    patterns_SIN, /* 0x1D gcSL_SIN */
    patterns_COS, /* 0x1E gcSL_COS */
    patterns_TAN, /* 0x1F gcSL_TAN */
    patterns_EXP, /* 0x20 gcSL_EXP */
    patterns_SIGN, /* 0x21 gcSL_SIGN */
    patterns_STEP, /* 0x22 gcSL_STEP */
    patterns_SQRT, /* 0x23 gcSL_SQRT */
    patterns_ACOS, /* 0x24 gcSL_ACOS */
    patterns_ASIN, /* 0x25 gcSL_ASIN */
    patterns_ATAN, /* 0x26 gcSL_ATAN */
    patterns_SET, /* 0x27 gcSL_SET */
    patterns_DSX, /* 0x28 gcSL_DSX */
    patterns_DSY, /* 0x29 gcSL_DSY */
    patterns_FWIDTH, /* 0x2A gcSL_FWIDTH */
    patterns_DIV, /* 0x2B gcSL_DIV */
    patterns_MOD, /* 0x2C gcSL_MOD */
    patterns_AND_BITWISE, /* 0x2D gcSL_AND_BITWISE */
    patterns_OR_BITWISE, /* 0x2E gcSL_OR_BITWISE */
    patterns_XOR_BITWISE, /* 0x2F gcSL_XOR_BITWISE */
    patterns_NOT_BITWISE, /* 0x30 gcSL_NOT_BITWISE */
    patterns_LSHIFT, /* 0x31 gcSL_LSHIFT */
    patterns_RSHIFT, /* 0x32 gcSL_RSHIFT */
    patterns_ROTATE, /* 0x33 gcSL_ROTATE */
    patterns_BITSEL, /* 0x34 gcSL_BITSEL */
    patterns_LEADZERO, /* 0x35 gcSL_LEADZERO */
    patterns_LOAD, /* 0x36 gcSL_LOAD */
    patterns_STORE, /* 0x37 gcSL_STORE */
    patterns_BARRIER, /* 0x38 gcSL_BARRIER */
    patterns_STORE1, /* 0x39 gcSL_STORE1 */
    patterns_ATOMADD, /* 0x3A gcSL_ATOMADD */
    patterns_ATOMSUB, /* 0x3B gcSL_ATOMSUB */
    patterns_ATOMXCHG, /* 0x3C gcSL_ATOMXCHG */
    patterns_ATOMCMPXCHG, /* 0x3D gcSL_ATOMCMPXCHG */
    patterns_ATOMMIN, /* 0x3E gcSL_ATOMMIN */
    patterns_ATOMMAX, /* 0x3F gcSL_ATOMMAX */
    patterns_ATOMOR, /* 0x40 gcSL_ATOMOR */
    patterns_ATOMAND, /* 0x41 gcSL_ATOMAND */
    patterns_ATOMXOR, /* 0x42 gcSL_ATOMXOR */
    patterns_TEXLDPCF, /* 0x43 gcSL_TEXLDPCF */
    patterns_TEXLDPCFPROJ, /* 0x44 gcSL_TEXLDPCFPROJ */
    patterns_TEXLODQ, /* 0x45 gcSL_TEXLODQ */
    patterns_FLUSH, /* 0x46 gcSL_FLUSH */
    patterns_JMP_ANY, /* 0x47 gcSL_JMP_ANY */
    patterns_BITRANGE, /* 0x48 gcSL_BITRANGE */
    patterns_BITRANGE1, /* 0x49 gcSL_BITRANGE1 */
    patterns_BITEXTRACT, /* 0x4A gcSL_BITEXTRACT */
    patterns_BITINSERT, /* 0x4B gcSL_BITINSERT */
    patterns_FINDLSB, /* 0x4C gcSL_FINDLSB */
    patterns_FINDMSB, /* 0x4D gcSL_FINDMSB */
    patterns_IMAGE_OFFSET, /* 0x4E gcSL_IMAGE_OFFSET */
    patterns_IMAGE_ADDR, /* 0x4F gcSL_IMAGE_ADDR */
    patterns_SINPI, /* 0x50 gcSL_SINPI */
    patterns_COSPI, /* 0x51 gcSL_COSPI */
    patterns_TANPI, /* 0x52 gcSL_TANPI */
    patterns_ADDLO, /* 0x53 gcSL_ADDLO */
    patterns_MULLO, /* 0x54 gcSL_MULLO */
    patterns_CONV, /* 0x55 gcSL_CONV */
    patterns_GETEXP, /* 0x56 gcSL_GETEXP */
    patterns_GETMANT, /* 0x57 gcSL_GETMANT */
    patterns_MULHI, /* 0x58 gcSL_MULHI */
    patterns_CMP, /* 0x59 gcSL_CMP */
    patterns_I2F, /* 0x5A gcSL_I2F */
    patterns_F2I, /* 0x5B gcSL_F2I */
    patterns_ADDSAT, /* 0x5C gcSL_ADDSAT */
    patterns_SUBSAT, /* 0x5D gcSL_SUBSAT */
    patterns_MULSAT, /* 0x5E gcSL_MULSAT */
    patterns_DP2, /* 0x5F gcSL_DP2 */
    patterns_UNPACK, /* 0x60 gcSL_UNPACK */
    patterns_IMAGE_WR, /* 0x61 gcSL_IMAGE_WR */
    patterns_SAMPLER_ADD, /* 0x62 gcSL_SAMPLER_ADD */
    patterns_MOVA, /* 0x63 gcSL_MOVA */
    patterns_IMAGE_RD, /* 0x64 gcSL_IMAGE_RD */
    patterns_IMAGE_SAMPLER, /* 0x65 gcSL_IMAGE_SAMPLER */
    patterns_NORM_MUL, /* 0x66 gcSL_NORM_MUL */
    patterns_NORM_DP2, /* 0x67 gcSL_NORM_DP2 */
    patterns_NORM_DP3, /* 0x68 gcSL_NORM_DP3 */
    patterns_NORM_DP4, /* 0x69 gcSL_NORM_DP4 */
    patterns_PRE_DIV, /* 0x6A gcSL_PRE_DIV */
    patterns_PRE_LOG2, /* 0x6B gcSL_PRE_LOG2 */
    patterns_TEXGATHER, /* 0x6C gcSL_TEXGATHER */
    patterns_TEXFETCH_MS, /* 0x6D gcSL_TEXFETCH_MS */
    patterns_POPCOUNT, /* 0x6E gcSL_POPCOUNT */
    patterns_BIT_REVERSAL, /* 0x6F gcSL_BIT_REVERSAL */
    patterns_BYTE_REVERSAL, /* 0x70 gcSL_BYTE_REVERSAL */
    patterns_TEXPCF, /* 0x71 gcSL_TEXPCF */
    patterns_UCARRY, /* 0x72 gcSL_UCARRY */
    patterns_TEXU, /* 0x73 gcSL_TEXU */
    patterns_TEXU_LOD, /* 0x74 gcSL_TEXU_LOD */
    patterns_MEM_BARRIER, /* 0x75 gcSL_MEM_BARRIER */
    patterns_SAMPLER_ASSIGN,/* 0x76 gcSL_SAMPLER_ASSIGN */
    patterns_GET_SAMPLER_IDX, /* 0x77  Get Image/Sampler index */
    patterns_IMAGE_RD_3D, /* 0x78 gcSL_IMAGE_RD_3D */
    patterns_IMAGE_WR_3D, /* 0x79 gcSL_IMAGE_WR_3D */
    patterns_CLAMP0MAX, /* 0x7A gcSL_CLAMP0MAX */
    patterns_FMA_MUL, /* 0x7B gcSL_FMA_MUL first part: MUL */
    patterns_FMA_ADD, /* 0x7C gcSL_FMA_ADD, FMA second part: ADD */
    patterns_ATTR_ST, /* 0x7D ATTR_ST attribute(0+temp(1).x), InvocationIndex, val */
    patterns_ATTR_LD, /* 0x7E ATTR_LD dest, attribute(0+temp(1).x), InvocationIndex */
    patterns_EMIT_VERTEX, /* 0x7F For function "EmitVertex" */
    patterns_END_PRIMITIVE, /* 0x80 For function "EndPrimitive" */
    patterns_ARCTRIG0, /* 0x81 gcSL_ARCTRIG0 */
    patterns_ARCTRIG1, /* 0x82 gcSL_ARCTRIG1 */
    patterns_MUL_Z, /* 0x83 gcSL_MUL_Z */
    patterns_NEG, /* 0x84 gcSL_NEG */
    patterns_LONGLO, /* 0x85 gcSL_LONGLO */
    patterns_LONGHI, /* 0x86 gcSL_LONGHI */
    patterns_MOV_LONG, /* 0x87 gcSL_MOV_LONG */
    patterns_MADSAT, /* 0x88 gcSL_MADSAT */
    patterns_COPY, /* 0x89 gcSL_COPY */
    patterns_IMAGE_ADDR_3D, /* 0x8A gcSL_IMAGE_ADDR_3D */
    patterns_GET_SAMPLER_LMM,/* 0x8B gcSL_GET_SAMPLER_LMM */
    patterns_GET_SAMPLER_LBS,/* 0x8C gcSL_GET_SAMPLER_LBS */
};

#ifdef WIN32
char _check_patterns_size[sizeof(patterns)/sizeof(patterns[0]) == gcSL_MAXOPCODE];
#endif

#endif


