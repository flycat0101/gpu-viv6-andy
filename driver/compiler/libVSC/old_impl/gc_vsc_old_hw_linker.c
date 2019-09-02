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


/*
**  Hardware dependent gcSL linker module.
*/

#include "gc_vsc.h"

#if gcdENABLE_3D

typedef struct _gcsSourceConstandInfo
{
    gctINT          uniformIndex;    /* the allocated uniform index */
    gctUINT8        swizzle;         /* the swizzle for the fisrt constant */
    gctINT          constNo;
    gctFLOAT        srcValues[3];    /* up to three constant in one instruction */
    gctBOOL         fromUBO;         /* From the constant UBO. */
} gcsSourceConstandInfo;


/* Zone used for header/footer. */
#define _GC_OBJ_ZONE    gcvZONE_HARDWARE

#define _EXTRA_16BIT_CONVERSION_         0
#define _USE_TEMP_FORMAT_                1
#define _USE_ORIGINAL_DST_WRITE_MASK_    1

#define gcvSL_AVAILABLE                  -1
#define gcvSL_TEMPORARY                  -2
#define gcvSL_RESERVED                   0x7FFFFFFF

#define _TEMP_INSTRUCTION_COUNT_         15


#define _gcmSetSource(Tree, CodeGen, States, Source, Type, Index, ConstIndex, Relative, Swizzle, Negate, Absolute, Precision) \
    ((CodeGen)->isDual16Shader ? _SetSourceWithPrecision((Tree), (CodeGen), (States), (Source), (Type), (Index), (ConstIndex), (Relative), (Swizzle), (Negate), (Absolute), (Precision)) \
                    : _SetSource((Tree), (CodeGen), (States), (Source), (Type), (Index), (ConstIndex), (Relative), (Swizzle), (Negate), (Absolute)))

#define _gcmSetDest(Tree, CodeGen, States, Index, Relative, Enable, Precision, Shift) \
    ((CodeGen)->isDual16Shader ? _SetDestWithPrecision((Tree), (CodeGen), (States), (Index), (Relative), (Enable), (Precision), (Shift)) \
                    : _SetDest((Tree), (CodeGen), (States), (Index), (Relative), (Enable), (Shift)))

#define _gcmHasDual16(Shader)   (GC_ENABLE_DUAL_FP16 > 0 && gcHWCaps.hwFeatureFlags.supportDual16)

gceSTATUS
gcLINKTREE_Destroy(
    IN gcLINKTREE Tree
    );

gceSTATUS
gcLINKTREE_Build(
    IN gcLINKTREE Tree,
    IN gcSHADER Shader,
    IN gceSHADER_FLAGS Flags
    );

void
_DumpLinkTree(
    IN gctCONST_STRING Text,
    IN gcLINKTREE Tree,
    IN gctBOOL DumpShaderOnly
    );

gceSTATUS
gcLINKTREE_MarkAllAsUsed(
    IN gcLINKTREE Tree
    );

gceSTATUS
gcLINKTREE_Construct(
    IN gcoOS Os,
    OUT gcLINKTREE * Tree
    );

void
gcLINKTREE_FindModelViewProjection(
    gcLINKTREE VertexTree
    );

gceSTATUS
gcLINKTREE_RemoveDeadCode(
    IN gcLINKTREE Tree
    );

gceSTATUS
gcLINKTREE_RemoveUnusedAttributes(
    IN gcLINKTREE Tree
    );

static gceSTATUS
_FinalEmit(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctUINT32 States[4],
    IN gctINT loopCount
    );

gctBOOL
_isHWRegisterAllocated(
    IN gcSHADER     Shader
    )
{
    return gcShaderHwRegAllocated(Shader);
}

gctBOOL
_isHWConstRegisterAllocated(
    IN gcSHADER     Shader
    )
{
    return gcShaderConstHwRegAllocated(Shader);
}

/* dump function */
static void
_printLinktreeTempName(
    IN gcLINKTREE_TEMP Temp,
    IN OUT char *      Buffer,
    IN gctSIZE_T       BufferSize
    )
{
    gctUINT offset = 0;

    gcoOS_PrintStrSafe(Buffer, BufferSize, &offset,
                      "temp(%d).", Temp->index);

    if (Temp->usage & gcSL_ENABLE_X)
    {
        /* X is used. */
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(Buffer, BufferSize, &offset, "x"));
    }

    if (Temp->usage & gcSL_ENABLE_Y)
    {
        /* Y is used. */
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(Buffer, BufferSize, &offset, "y"));
    }

    if (Temp->usage & gcSL_ENABLE_Z)
    {
        /* Z is used. */
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(Buffer, BufferSize, &offset, "z"));
    }

    if (Temp->usage & gcSL_ENABLE_W)
    {
        /* W is used. */
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(Buffer, BufferSize, &offset, "w"));
    }
}

static void
_printRegisterName(
    IN gcLINKTREE_TEMP Temp,
    IN OUT char *      Buffer,
    IN gctSIZE_T       BufferSize
    )
{
    gctUINT offset = 0;
    static const char *swizzleStr[] = {"x", "y", "z", "w" };

    gcSL_SWIZZLE  xSwizzle = gcmExtractSwizzle(Temp->swizzle, 0),
                  ySwizzle = gcmExtractSwizzle(Temp->swizzle, 1),
                  zSwizzle = gcmExtractSwizzle(Temp->swizzle, 2),
                  wSwizzle = gcmExtractSwizzle(Temp->swizzle, 3);

    gcoOS_PrintStrSafe(Buffer, BufferSize, &offset,
                      "r%d.%s%s%s%s", Temp->assigned,
                                      swizzleStr[xSwizzle],
                                      swizzleStr[ySwizzle],
                                      swizzleStr[zSwizzle],
                                      swizzleStr[wSwizzle]);
}

void dumpRegisterAllocation(
    IN gcLINKTREE_TEMP Temp
    )
{
    char tempName[32];
    char registerName[32];
    _printLinktreeTempName(Temp, tempName, gcmSIZEOF(tempName));
    _printRegisterName(Temp, registerName, gcmSIZEOF(registerName));

    gcoOS_Print("%s assigned to register %s (last use %d)",
                tempName, registerName, Temp->lastUse);
}

static gctBOOL
_IsSwizzleMatch(
    IN gcSL_SWIZZLE MatchSwizzle[4],
    IN gcSL_SWIZZLE UseSwizzle[4]
)
{
    gctBOOL match = gcvFALSE;
    gctINT i,j;

    for (i = 0; i < 4 && !match; i++)
    {
        for (j = 0; j < 4 && !match; j++)
        {
            if (MatchSwizzle[i] == UseSwizzle[j])
            {
                match = gcvTRUE;
            }
        }
    }
    return match;
}

static gctBOOL
_IsChannelUsedForAttribute(
    IN gcLINKTREE Tree,
    IN gcLINKTREE_ATTRIBUTE Attribute,
    IN gctUINT32 Index,
    IN gcSL_SWIZZLE MatchSwizzle
    )
{
    gctBOOL isUsed = gcvFALSE;
    gcsLINKTREE_LIST_PTR user = Attribute->users;
    gcSL_SWIZZLE matchSwizzle[4];
    gctINT i;

    for (i = 0; i < 4; i++)
    {
        matchSwizzle[i] = gcmExtractSwizzle(MatchSwizzle, i);
    }

    while (user)
    {
        gcSL_INSTRUCTION code = gcvNULL;
        gcSL_SWIZZLE useSwizzle[4];
        gctSOURCE_t source;

        code = Tree->shader->code + user->index;

        /* Get the source. */
        if (gcmSL_SOURCE_GET(code->source0, Type) == gcSL_ATTRIBUTE &&
            code->source0Index == Index)
        {
            source = code->source0;
        }
        else
        {
            gcmASSERT(gcmSL_SOURCE_GET(code->source1, Type) == gcSL_ATTRIBUTE &&
                code->source1Index == Index);
            source = code->source1;
        }

        /* Get the source swizzle. */
        for (i = 0; i < 4; i++)
        {
            useSwizzle[i] = gcmExtractSwizzle(gcmSL_SOURCE_GET(source, Swizzle), i);
        }

        /* If any used channel match, return. */
        isUsed = _IsSwizzleMatch(matchSwizzle, useSwizzle);

        if (isUsed)
        {
            break;
        }
        user = user->next;
    }

    return isUsed;
}


/****************************************************************
 *                                                              *
 *        20bit Immediate related help functions                *
 *                                                              *
 ****************************************************************/

/* return true if CodeGen can generate for Instruction's OperandNo
 *   OperandNo 0: source0,
 *             1: source1,
 *            -1: destination
 */
gctBOOL
Generate20BitsImmediate(
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION      Instruction,
    IN gctINT                OperandNo
    )
{
    gcSL_OPCODE opcode = gcmSL_OPCODE_GET(Instruction->opcode, Opcode);
    /* special handling for dual16 mode:
          - In dual-16 mode, BRANCH targets must be specified as a source2
             U20 immediate.
          - The I2I instruction's destination data type (provided as
            source_1.x[6:4]) must be an immediate.

       Supported sources for single-t instructions which are not supported
       for dual-t instructions:
         - Immediate source formats other than packed V16, except for branch
           targets and CONV/I2I src1 which are always U20 regardless of
           single_t/dual_t.
     */
    return (CodeGen->generateImmediate || CodeGen->forceGenImmediate) &&
           (!CodeGen->isDual16Shader || (opcode == gcSL_JMP && OperandNo == -1) ||
                                        (opcode == gcSL_CALL && OperandNo == -1) ||
                                        opcode == gcSL_CONV );
}

gctINT
gcGetSrcType(
     IN gctUINT32 * States,
     IN gctUINT     Src
     )
{
    gctINT srcType = 0;

    gcmASSERT(Src < 3);
    switch (Src) {
    case 0:
        srcType = (((((gctUINT32) (States[2])) >> (0 ? 5:3)) & ((gctUINT32) ((((1 ? 5:3) - (0 ? 5:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1)))))) );
        break;
    case 1:
        srcType = (((((gctUINT32) (States[3])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) );
        break;
    case 2:
        srcType = (((((gctUINT32) (States[3])) >> (0 ? 30:28)) & ((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1)))))) );
        break;
    }
    return srcType;
}

gctBOOL
isSourceImmediateValue(
     IN gctUINT32 * States,
     IN gctUINT     Src
     )
{
    gctINT srcType = 0;

    gcmASSERT(Src < 3);
    switch (Src) {
    case 0:
        srcType = (((((gctUINT32) (States[2])) >> (0 ? 5:3)) & ((gctUINT32) ((((1 ? 5:3) - (0 ? 5:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1)))))) );
        break;
    case 1:
        srcType = (((((gctUINT32) (States[3])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) );
        break;
    case 2:
        srcType = (((((gctUINT32) (States[3])) >> (0 ? 30:28)) & ((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1)))))) );
        break;
    }
    return srcType == 0x7;
}

/* Convert 32bit fp to 20bit fp, result saved in f20
   return true if the converted value is exact */
gctBOOL
gcConvertF32ToF20(
    IN  gctFP32BINARY  f32, /* in binary32 format */
    OUT gcsFLOAT20 *   f20
    )
{
    gctBOOL exact;

    /* get biased Exponent.*/
    f20->Exponent = ((f32 >> 23) & FP32_ExponentBitMask);
    f20->Sign     = ((f32 >> 31) & FP32_SignBitMask);    /* get 1 bit of Sign */
    f20->Mantissa = (f32 & FP32_MantissaBitMask) >> 12;  /* get 23 - 12 Bits of Mantissa */
    exact         = ((f32 & 0xfff) == 0);                /* last 12 Bits are zero? */
    return exact;
}

/* convert the 20bit value Immediate20Bits of ImmType to 32bit Value */
void
gcConvert20BitImmediateTo32Bit(
    IN  gctUINT32          Immediate20Bits,
    IN  gctUINT32          ImmType,
    OUT gcsConstantValue * ConstValue)
{
    gcmASSERT((Immediate20Bits & 0xFFF00000) == 0);

    ConstValue->value.u = Immediate20Bits;
    switch(ImmType) {
    case 0x0:
        ConstValue->value.u = GetFP32BianryFromFP20Binary(Immediate20Bits);
        ConstValue->ty = gcSL_FLOAT;
        break;
    case 0x1:
        if (Immediate20Bits & 0x80000)
            ConstValue->value.u |= 0xFFF00000; /* sign extension */
        ConstValue->ty = gcSL_INT32;
        break;
    case 0x2:
        ConstValue->ty = gcSL_UINT32;
        break;
    case 0x3:
        ConstValue->ty = gcSL_UINT16;   /* is this the right type??? */
        break;
    default:
        gcmASSERT(gcvFALSE);
        break;
    }
}

/* convert the 20bit value Hex20 of gcSL_FORMAT type to 32bit Value */
void
gcConvert20BitTo32BitValue(
    IN  gcSL_FORMAT         Format,
    IN  gctUINT32           Hex20,
    OUT gcsConstantValue*   Value
    )
{
    gcmASSERT((Hex20 & 0xFFF00000) == 0);

    Value->ty = Format;
    Value->value.u = Hex20;
    switch (Format) {
    case gcSL_FLOAT:
        Value->value.u = GetFP32BianryFromFP20Binary(Hex20);
        break;
    case gcSL_INT32:
        if (Hex20 & 0x80000)
            Value->value.u = Hex20 | 0xFFF00000; /* sign extension */
        break;
    case gcSL_INT16:
        if (Hex20 & 0x8000)
            Value->value.u = Hex20 | 0xFFFF0000; /* sign extension */
        break;
    case gcSL_UINT32:
    case gcSL_UINT16:
    default:
        break;
    }
}

/* return true if the Value can be fit into 20bits,
   considering it may be negated or absoluted later
  */
gctBOOL
ValueFit20Bits(
    IN  gcSL_FORMAT         Format,
    IN  gctUINT32           Hex32
    )
{
    gctBOOL          fit20bits = gcvFALSE;
    gcsFLOAT20       f20;
    gcsConstantValue cv;

    cv.value.u = Hex32;

    switch (Format) {
    case gcSL_FLOAT:
        fit20bits = gcConvertF32ToF20(Hex32, &f20);
        break;
    case gcSL_INT32:
       /* -INT20_MIN is out of range of int20 if negated */
        fit20bits = (cv.value.i <= INT20_MAX) &&
                    (cv.value.i > INT20_MIN) ;
        break;
    case gcSL_UINT32:
        fit20bits = cv.value.u <= INT20_MAX;
        break;
    case gcSL_UINT16:
        fit20bits = (cv.value.i <= INT16_MAX);
        break;
    case gcSL_INT16:
        fit20bits = (cv.value.i <= INT16_MAX) &&
                    (cv.value.i > INT16_MIN) ;
    default:
        break;
    }
    return fit20bits;
}

/* absolute the constant value, the value should fit in 20 bits */
void
gcAbsoluteValueFit20Bit(
    IN OUT gcsConstantValue*   Value
    )
{
    gcSL_FORMAT  format = Value->ty;

    switch (format) {
    case gcSL_FLOAT:
        if (Value->value.f < 0)
            Value->value.f = -Value->value.f;
        break;
    case gcSL_INT32:
       /* -INT20_MIN is out of range of int20 */
        gcmASSERT(Value->value.i > INT20_MIN);
        if (Value->value.i < 0)
            Value->value.i = -Value->value.i;
        break;
    case gcSL_UINT32:
    case gcSL_UINT16:
        /* do nothing for unsigned int32/int16 */
        break;
    case gcSL_INT16:
        gcmASSERT(Value->value.i > INT16_MIN);
        if (Value->value.i < 0)
            Value->value.i = -Value->value.i;
        break;
    default:
        gcmASSERT(gcvFALSE);
        break;
    }
}

/* negate the constant value, the value should fit in 20 bits */
void
gcNegateValueFit20Bit(
    IN OUT gcsConstantValue*   Value
    )
{
    gcSL_FORMAT  format = Value->ty;

    switch (format) {
    case gcSL_FLOAT:
        Value->value.f = -Value->value.f;
        break;
    case gcSL_INT32:
       /* -INT20_MIN is out of range of int20 */
        gcmASSERT(Value->value.i > INT20_MIN);
        Value->value.i = -Value->value.i;
        break;
    case gcSL_UINT32:
        gcmASSERT (Value->value.u <= INT20_MAX);
        /* change the type to signed type */
        Value->value.i = -Value->value.i;
        Value->ty      = gcSL_INT32;
        break;
    case gcSL_UINT16:
        gcmASSERT(Value->value.i <= INT16_MAX);
        /* change the type to signed type */
        Value->value.i = -Value->value.i;
        Value->ty      = gcSL_INT16;
        break;
    case gcSL_INT16:
        gcmASSERT(Value->value.i > INT16_MIN);
        Value->value.i = -Value->value.i;
        break;
    default:
        gcmASSERT(gcvFALSE);
        break;
    }
}

/* return true if the Source is immediate type, and set the value
   to Immediate, the immediate value type to ImmType,
   otherwise return false */
gctBOOL
gcExtractSource20BitImmediate(
    IN  gctUINT32  States[4],
    IN  gctUINT    Source,
    OUT gctUINT32* Immediate,
    OUT gctUINT32* ImmType
    )
{
    gctUINT32 index;
    gctUINT32 relative;
    gctUINT32 swizzle;
    gctUINT32 negate;
    gctUINT32 absolute;
    gctINT srcType = gcGetSrcType(States, Source);
    gctUINT32 data;

    if (srcType != 0x7)
        return gcvFALSE;

    switch (Source)
    {
    case 0:
        /* Set SRC0 fields. */
        index    = (((((gctUINT32) (States[1])) >> (0 ? 20:12)) & ((gctUINT32) ((((1 ? 20:12) - (0 ? 20:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1)))))) );
        swizzle  = (((((gctUINT32) (States[1])) >> (0 ? 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1)))))) );
        negate   = (((((gctUINT32) (States[1])) >> (0 ? 30:30)) & ((gctUINT32) ((((1 ? 30:30) - (0 ? 30:30) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:30) - (0 ? 30:30) + 1)))))) );
        absolute = (((((gctUINT32) (States[1])) >> (0 ? 31:31)) & ((gctUINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1)))))) );
        relative = (((((gctUINT32) (States[2])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) );
        break;
    case 1:
        /* Set SRC1 fields. */
        index    = (((((gctUINT32) (States[2])) >> (0 ? 15:7)) & ((gctUINT32) ((((1 ? 15:7) - (0 ? 15:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:7) - (0 ? 15:7) + 1)))))) );
        swizzle  = (((((gctUINT32) (States[2])) >> (0 ? 24:17)) & ((gctUINT32) ((((1 ? 24:17) - (0 ? 24:17) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1)))))) );
        negate   = (((((gctUINT32) (States[2])) >> (0 ? 25:25)) & ((gctUINT32) ((((1 ? 25:25) - (0 ? 25:25) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:25) - (0 ? 25:25) + 1)))))) );
        absolute = (((((gctUINT32) (States[2])) >> (0 ? 26:26)) & ((gctUINT32) ((((1 ? 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ? 26:26) + 1)))))) );
        relative = (((((gctUINT32) (States[2])) >> (0 ? 29:27)) & ((gctUINT32) ((((1 ? 29:27) - (0 ? 29:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:27) - (0 ? 29:27) + 1)))))) );
        break;
    case 2:
        /* Set SRC2 fields. */
        index    = (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) );
        swizzle  = (((((gctUINT32) (States[3])) >> (0 ? 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1)))))) );
        negate   = (((((gctUINT32) (States[3])) >> (0 ? 22:22)) & ((gctUINT32) ((((1 ? 22:22) - (0 ? 22:22) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 22:22) - (0 ? 22:22) + 1)))))) );
        absolute = (((((gctUINT32) (States[3])) >> (0 ? 23:23)) & ((gctUINT32) ((((1 ? 23:23) - (0 ? 23:23) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:23) - (0 ? 23:23) + 1)))))) );
        relative = (((((gctUINT32) (States[3])) >> (0 ? 27:25)) & ((gctUINT32) ((((1 ? 27:25) - (0 ? 27:25) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1)))))) );
        break;

    default:
        return gcvFALSE;
    }
    data = ((index    << 0)
            | (swizzle  << 9)
            | (negate   << 17)
            | (absolute << 18)
            | (relative << 19)
            );

    /* Grab the fields.*/
    *ImmType    = (data >> 20) & 0x03;
    *Immediate  = data & 0xFFFFF;
    return gcvTRUE;
}

void
gcSetSrcABS(
     IN OUT gctUINT32 * States,
     IN gctUINT         Src
     )
{
    gctUINT32 immediate;
    gctUINT32 immType;

    gcmASSERT(Src < 3);
    if (gcExtractSource20BitImmediate(States, Src, &immediate, &immType )) {
        /* the source is immediate number */
        gcsConstantValue value;
        gcConvert20BitImmediateTo32Bit(immediate, immType, &value);
        gcAbsoluteValueFit20Bit(&value);
        gcEncodeSourceImmediate20(States, Src, &value);
        return;
    }

    switch (Src) {
    case 0:
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)));
        break;
    case 1:
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26)));
        break;
    case 2:
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:23) - (0 ?
 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ? 23:23)));
        break;
    }
    return;
}

void
gcSetSrcNEG(
     IN OUT gctUINT32 * States,
     IN gctUINT         Src
     )
{
    gctUINT32 immediate;
    gctUINT32 immType;

    gcmASSERT(Src < 3);
    if (gcExtractSource20BitImmediate(States, Src, &immediate, &immType)) {
        /* the source is immediate number */
        gcsConstantValue value;
        gcConvert20BitImmediateTo32Bit(immediate, immType, &value);
        gcNegateValueFit20Bit(&value);
        gcEncodeSourceImmediate20(States, Src, &value);
        return;
    }
    switch (Src) {
    case 0:
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:30) - (0 ?
 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) ((gctUINT32) ((((((gctUINT32) (States[1])) >> (0 ?
 30:30)) & ((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:30) - (0 ?
 30:30) + 1)))))) ) ^ 1) & ((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)));
        break;
    case 1:
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:25) - (0 ?
 25:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:25) - (0 ?
 25:25) + 1))))))) << (0 ?
 25:25))) | (((gctUINT32) ((gctUINT32) ((((((gctUINT32) (States[2])) >> (0 ?
 25:25)) & ((gctUINT32) ((((1 ?
 25:25) - (0 ?
 25:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:25) - (0 ?
 25:25) + 1)))))) ) ^ 1) & ((gctUINT32) ((((1 ?
 25:25) - (0 ?
 25:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ? 25:25)));
        break;
    case 2:
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:22) - (0 ?
 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) ((gctUINT32) ((((((gctUINT32) (States[3])) >> (0 ?
 22:22)) & ((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:22) - (0 ?
 22:22) + 1)))))) ) ^ 1) & ((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ? 22:22)));
        break;
    }
    return ;
}

extern void
_SetValueType0(
    IN gctUINT ValueType,
    IN OUT gctUINT32 * States
    );

#if _USE_TEMP_FORMAT_
extern gctUINT type_conv[];

static void
_SetValueType0FromFormat(
    IN gctUINT Format,
    IN OUT gctUINT32 * States
    )
{
    gctUINT valueType0 = type_conv[Format];
    gctUINT instType0 = valueType0 & 0x1;
    gctUINT instType1 = valueType0 >> 1;

    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ?
 21:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:21) - (0 ?
 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) ((gctUINT32) (instType0) & ((gctUINT32) ((((1 ?
 21:21) - (0 ?
 21:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ? 21:21)));
    States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:30) - (0 ?
 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (instType1) & ((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));
}
#else
extern gctBOOL
value_type0(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    );
#endif

void gcCGUpdateMaxRegister(
    IN void *                 pCodeGen,
    IN gctUINT                Regs,
    IN gcLINKTREE             Tree
    )
{
    gcsCODE_GENERATOR_PTR CodeGen = (gcsCODE_GENERATOR_PTR)pCodeGen;
    /* do not update max register if Regs is fake subsampleDepth register */
    if (CodeGen->subsampleDepthPhysical == Regs)
    {
        return;
    }
    /* Assuming all new temp registers generated by vir->gcsl will be merged at codegen when RA is on. */
    if(!_isHWRegisterAllocated(Tree->shader))
    {
        gcmASSERT(Regs <= CodeGen->registerCount);
    }
    else
    {
        gcmASSERT(Regs < Tree->shader->RARegWaterMark + Tree->shader->RATempReg);
    }

    if (Regs > CodeGen->maxRegister)
        CodeGen->maxRegister = Regs;
}

gctUINT
gcsCODE_GENERATOR_GetIP(
    gcsCODE_GENERATOR_PTR CodeGen
    )
{
    gcmHEADER_ARG("CodeGen=0x%x", CodeGen);

    gcmASSERT(CodeGen->current != gcvNULL);

    gcmFOOTER_ARG("return=0x%x", CodeGen->current->ip);
    return CodeGen->current->ip;
}

static void
_DumpShader(
            IN gctUINT32_PTR States,
            IN gctUINT32 StateBufferOffset,
            IN gctBOOL OutputFormat,
            IN gctUINT InstBase,
            IN gctUINT InstMax,
            IN gctBOOL IsDual16Shader
            )
{
    gctUINTPTR_T lastState;
    gctUINT32 address, count, nextAddress;
    gctUINT       dumpBufferSize = 1024;
    gctCHAR*      pDumpBuffer;
    VSC_DUMPER    vscDumper;
    VSC_MC_CODEC  mcCodec;

    gcoOS_Allocate(gcvNULL, dumpBufferSize, (gctPOINTER*)&pDumpBuffer);

    vscDumper_Initialize(&vscDumper,
                         gcvNULL,
                         gcvNULL,
                         pDumpBuffer,
                         dumpBufferSize);

    vscMC_BeginCodec(&mcCodec, &gcHWCaps, IsDual16Shader, gcvFALSE);

    lastState = (gctUINTPTR_T)((gctUINT8_PTR) States + StateBufferOffset);
    nextAddress = 0;

    while ((gctUINTPTR_T) States < lastState)
    {
        gctUINT32 state = *States++;

        gcmASSERT(((((gctUINT32) (state)) >> (0 ?
 31:27) & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1)))))) == (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))));

        address = (((((gctUINT32) (state)) >> (0 ? 15:0)) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1)))))) );
        count   = (((((gctUINT32) (state)) >> (0 ? 25:16)) & ((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1)))))) );

        if ((address >= InstBase) &&
            (address < InstBase + InstMax) )
        {
            if (nextAddress == 0) {
                /* Header. */
                gcoOS_Print("***** [ Generated Shader Code ] *****");
            }

            /* Dump all states. */
            for (address = 0; count >= 4; count -= 4)
            {
                vscMC_DumpInst(&mcCodec, (VSC_MC_RAW_INST*)States, address++ + nextAddress, &vscDumper);

                /* Next instruction. */
                States += 4;
            }
            nextAddress += address;
        }
        else
        {
            States += count;
        }

        if ((count & 1) == 0) ++States;
    }

    /* Release dumper buffer */
    gcoOS_Free(gcvNULL, pDumpBuffer);
    vscMC_EndCodec(&mcCodec);
}

static void
_DumpUniform(
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctUINT32 Address,
    IN gctFLOAT Value
    )
{
    gctUINT32 base = CodeGen->uniformBase;
    gctUINT32 index   = (Address - base) >> 2;
    gctUINT32 swizzle = (Address - base) &  3;

    const char * shader = gcSL_GetShaderKindString(CodeGen->shaderType);
    const char enable[] = "xyzw";

    gcoOS_Print("%s: c%u.%c = %f (0x%08X)",
                  shader,
                  index,
                  enable[swizzle],
                  Value,
                  *(gctUINT32_PTR) &Value);
}

static void
_DumpUniforms(
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctUINT32_PTR States,
    IN gctUINT32 StateBufferOffset
    )
{
    gctUINTPTR_T lastState;

    lastState = (gctUINTPTR_T)((gctUINT8_PTR) States + StateBufferOffset);

    while ((gctUINTPTR_T) States < lastState)
    {
        gctUINT32 state = *States++;
        gctUINT32 address, count, i;

        gcmASSERT(((((gctUINT32) (state)) >> (0 ?
 31:27) & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1)))))) == (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))));

        address = (((((gctUINT32) (state)) >> (0 ? 15:0)) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1)))))) );
        count   = (((((gctUINT32) (state)) >> (0 ? 25:16)) & ((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1)))))) );

        if ((address >= CodeGen->uniformBase) &&
             (address < CodeGen->uniformBase + CodeGen->maxUniform * 4) )
        {
            for (i = 0; i < count; ++i)
            {
                _DumpUniform(CodeGen, address++, *(gctFLOAT *) States);
                ++States;
            }

            if ((count & 1) == 0) ++States;
        }
        else
        {
            States += count;
            if ((count & 1) == 0) ++States;
        }
    }
}

extern gcsSL_PATTERN_PTR patterns[];

static gceSTATUS
_FindRegisterUsage(
    IN OUT gcsSL_USAGE_PTR Usage,
    IN gctSIZE_T Count,
    IN gcSHADER_TYPE Type,
    IN gctINT Length,
    IN gctINT LastUse,
    IN gctBOOL Restricted,
    OUT gctINT * Physical,
    OUT gctUINT8 * Swizzle,
    OUT gctINT * Shift,
    OUT gctUINT8 * Enable,
    IN gctSIZE_T   StartIndex
    );

/* When using a constant, it may from a uniform or a UBO.
** If it is from a uniform, the type is 0x2;
** otherwise, the type is 0x0.
*/
gceSTATUS
_UsingConstUniform(
    IN gcLINKTREE            Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctINT SourceIndex,
    IN gctINT Index,
    IN gctUINT8 Swizzle,
    IN gcSL_TYPE ConstType,
    IN OUT gctUINT32 * States
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT type;

    if (ConstType == gcSL_UNIFORM)
        type = 0x2;
    else
        type = 0x0;

    switch (SourceIndex)
    {
    case 0:
        /* Set SRC0 fields. */
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:12) - (0 ?
 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (Index) & ((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ? 20:12)));
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:22) - (0 ?
 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (Swizzle) & ((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ? 29:22)));
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (type) & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)));
        break;

    case 1:
        /* Set SRC1 fields. */
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:7) - (0 ?
 15:7) + 1))))))) << (0 ?
 15:7))) | (((gctUINT32) ((gctUINT32) (Index) & ((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:7) - (0 ? 15:7) + 1))))))) << (0 ? 15:7)));
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:17) - (0 ?
 24:17) + 1))))))) << (0 ?
 24:17))) | (((gctUINT32) ((gctUINT32) (Swizzle) & ((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1))))))) << (0 ? 24:17)));
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (type) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)));
        break;

    case 2:
        /* Set SRC2 fields. */
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (Index) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)));
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (Swizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)));
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (type) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));
        break;

    default:
        gcmASSERT(0);
        break;
    }
    return status;
}

static gceSTATUS
_AllocateConstForConstUBO(
    IN gcLINKTREE            Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctUINT8              Usage,
    IN gcuFLOAT_UINT32       Constants[4],
    IN gctINT                Count,
    OUT gctINT_PTR           Index,
    OUT gctUINT8_PTR         Swizzle,
    OUT gctINT_PTR           Shift
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcUNIFORM uniform;
    gcsUNIFORM_BLOCK constUBO;
    gcSHADER shader = Tree->shader;
    gctUINT32 states[4];
    gctINT physicalAddress = -1;
    gctUINT8 swizzle;
    gctINT shift;
    static const gcSL_ENABLE component2Enable[] = {
                 gcSL_ENABLE_NONE,
                 gcSL_ENABLE_X,
                 gcSL_ENABLE_XY,
                 gcSL_ENABLE_XYZ,
                 gcSL_ENABLE_XYZW
        };
    static const gcSL_SWIZZLE component2Swizzle[] = {
                 gcSL_SWIZZLE_X,
                 gcSL_SWIZZLE_X,
                 gcSL_SWIZZLE_XYYY,
                 gcSL_SWIZZLE_XYZZ,
                 gcSL_SWIZZLE_XYZW
        };
    gcsConstantValue constValue;

    /* If the index of constant UBO is -1,
    ** it means there is no enough uniform memory left for const UBO, just skip it.
    */
    if (shader->constUniformBlockIndex == -1)
    {
        return gcvSTATUS_TOO_MANY_UNIFORMS;
    }

    /* Get the constant UBO. */
    gcmONERROR(gcSHADER_GetUniformBlock(shader,
                                      shader->constUniformBlockIndex,
                                      &constUBO));
    gcmASSERT(GetUBIndex(constUBO) != -1);

    gcmONERROR(gcSHADER_GetUniform(shader,
                                   GetUBIndex(constUBO),
                                   &uniform));
    gcmASSERT(uniform->physical != -1);

    /* Allocate a temporary register. */
    gcmONERROR(_FindRegisterUsage(CodeGen->registerUsage,
                            CodeGen->registerCount,
                            gcSHADER_FLOAT_X4,
                            1,
                            gcvSL_TEMPORARY,
                            gcvFALSE,
                            (gctINT_PTR) &physicalAddress,
                            &swizzle,
                            &shift,
                            gcvNULL,
                            0));

    /* Emit a extra load instruction to load the constant from constant UBO. */
    /* Zero out the generated instruction. */
    gcoOS_ZeroMemory(states, sizeof(states));

    /* Fill dest state. */
    states[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) (0x32 & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ?
 10:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:6) - (0 ?
 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x00 & ((gctUINT32) ((((1 ?
 10:6) - (0 ?
 10:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (physicalAddress) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (component2Enable[Count]) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));

    /* Fill source0 state. */
    states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))
              |  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:12) - (0 ?
 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (uniform->physical) & ((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ? 20:12)))
              |  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:22) - (0 ?
 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (uniform->swizzle) & ((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ? 29:22)))
              |  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:30) - (0 ?
 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)))
              |  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)));
    states[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
              |  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)));

    /* Fill source1 state. */
    constValue.ty = gcSL_INTEGER;
    constValue.value.u = shader->constUBOSize * 16;

    gcmONERROR(gcEncodeSourceImmediate20(
                            states,
                            1,
                            &constValue));

    /* Emit this instruction. */
    gcmONERROR(_FinalEmit(Tree, CodeGen, states, 0));

    *Index = physicalAddress;
    *Swizzle = component2Swizzle[Count];

    /* Add the UBO size. */
    shader->constUBOSize++;

    SetUBBlockSize(constUBO, shader->constUBOSize * 16);

    return status;
OnError:
    return status;
}

static gceSTATUS
_VIR_AllocateConst(
    IN gcLINKTREE            Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctUINT8              Usage,
    IN gcuFLOAT_UINT32       Constants[4],
    IN gcUNIFORM             uniform
    )
{
    gcsSL_CONSTANT_TABLE_PTR c;
    gceSTATUS status;
    gctINT index = 0, count;
    gctBOOL valid[4];

    valid[0] = (Usage & gcSL_ENABLE_X) ? gcvTRUE : gcvFALSE;
    valid[1] = (Usage & gcSL_ENABLE_Y) ? gcvTRUE : gcvFALSE;
    valid[2] = (Usage & gcSL_ENABLE_Z) ? gcvTRUE : gcvFALSE;
    valid[3] = (Usage & gcSL_ENABLE_W) ? gcvTRUE : gcvFALSE;
    count    = (Usage & gcSL_ENABLE_W) ? 4
             : (Usage & gcSL_ENABLE_Z) ? 3
             : (Usage & gcSL_ENABLE_Y) ? 2
                                       : 1;
    do
    {
        /* Walk all costants. */
        for (c = CodeGen->constants; c != gcvNULL; c = c->next)
        {
            gctBOOL match = gcvFALSE;

            if (count > c->count)
            {
                continue;
            }

            switch (count)
            {
            case 1:
                for (index = 0; index < c->count; ++index)
                {
                   /* if (c->constant[index] == Constants[0]) */
                    if (*((unsigned int *) (&c->constant[index])) == Constants[0].u)
                    {
                        match = gcvTRUE;
                        break;
                    }
                }
                break;

            case 2:
                for (index = 0; index < c->count - 1; ++index)
                {
                    if ((!valid[0] || (*((unsigned int *) (&c->constant[index + 0])) == Constants[0].u))
                    &&  (!valid[1] || (*((unsigned int *) (&c->constant[index + 1])) == Constants[1].u))
                    )
                    {
                        match = gcvTRUE;
                        break;
                    }
                }
                break;

            case 3:
                for (index = 0; index < c->count - 2; ++index)
                {
                    if ((!valid[0] || (*((unsigned int *) (&c->constant[index + 0])) == Constants[0].u))
                    &&  (!valid[1] || (*((unsigned int *) (&c->constant[index + 1])) == Constants[1].u))
                    &&  (!valid[2] || (*((unsigned int *) (&c->constant[index + 2])) == Constants[2].u))
                    )
                    {
                        match = gcvTRUE;
                        break;
                    }
                }
                break;

            case 4:
                index = 0;

                if ((!valid[0] || (*((unsigned int *) (&c->constant[0])) == Constants[0].u))
                &&  (!valid[1] || (*((unsigned int *) (&c->constant[1])) == Constants[1].u))
                &&  (!valid[2] || (*((unsigned int *) (&c->constant[2])) == Constants[2].u))
                &&  (!valid[3] || (*((unsigned int *) (&c->constant[3])) == Constants[3].u))
                )
                {
                    match = gcvTRUE;
                }
                break;
            }

            if (match)
            {
                break;
            }
        }

        /* Right now we don't reuse the constant from UBO
        ** because we don't evaluate the lastUse for these constants
        ** and we don't have enough temp registers to hold such constants.
        */
        if (c != gcvNULL && c->fromUBO)
        {
            c = gcvNULL;
        }

        if (c == gcvNULL)
        {
            gctPOINTER pointer = gcvNULL;

            /* Allocate a new constant. */
            gcmERR_BREAK(gcoOS_Allocate(gcvNULL,
                                        sizeof(gcsSL_CONSTANT_TABLE),
                                        &pointer));

            c = pointer;

            /* Initialize the constant. */
            c->next        = CodeGen->constants;
            c->count       = count;
            c->fromUBO     = gcvFALSE;
            for (index = 0; index < count; ++index)
            {
                c->constant[index] = Constants[index].f;
            }

            /* Link constant to head of tree. */
            CodeGen->constants = c;

            c->index = uniform->physical;
            c->swizzle = uniform->swizzle;
        }

        /* Success. */
        status = gcvSTATUS_OK;
    }
    while (gcvFALSE);

    /* Return the status. */
    return status;
}

/* return original Swizzle if the Shift pointer is not NULL, also
 * return the shift to the pointer, if is used for initialized
 * constant uniform vector.
 */
static gceSTATUS
_AllocateConst(
    IN gcLINKTREE            Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctUINT8              Usage,
    IN gcuFLOAT_UINT32       Constants[4],
    IN gctBOOL               Restricted,
    IN gctBOOL               Reuse,
    OUT gctINT_PTR           Index,
    OUT gctUINT8_PTR         Swizzle,
    OUT gctINT_PTR           Shift,
    OUT gcSL_TYPE            *Type
    )
{
    gcsSL_CONSTANT_TABLE_PTR c = gcvNULL;
    gceSTATUS status;
    gctINT shift = 0, index = 0, count;
    gctBOOL valid[4];
    gctUINT8 swizzle = 0;
    gctBOOL isConstExistBefore = gcvFALSE;

    valid[0] = (Usage & gcSL_ENABLE_X) ? gcvTRUE : gcvFALSE;
    valid[1] = (Usage & gcSL_ENABLE_Y) ? gcvTRUE : gcvFALSE;
    valid[2] = (Usage & gcSL_ENABLE_Z) ? gcvTRUE : gcvFALSE;
    valid[3] = (Usage & gcSL_ENABLE_W) ? gcvTRUE : gcvFALSE;
    count    = (Usage & gcSL_ENABLE_W) ? 4
             : (Usage & gcSL_ENABLE_Z) ? 3
             : (Usage & gcSL_ENABLE_Y) ? 2
                                       : 1;

    *Type = gcSL_UNIFORM;

    do
    {
        /* Wall all constants if we can resue them. */
        if (Reuse)
        {
            for (c = CodeGen->constants; c != gcvNULL; c = c->next)
            {
                gctBOOL match = gcvFALSE;

                if (count > c->count)
                {
                    continue;
                }

                switch (count)
                {
                case 1:
                    for (index = 0; index < c->count; ++index)
                    {
                       /* if (c->constant[index] == Constants[0]) */
                        if (*((unsigned int *) (&c->constant[index])) == Constants[0].u)
                        {
                            match = gcvTRUE;
                            break;
                        }
                    }
                    break;

                case 2:
                    for (index = 0; index < c->count - 1; ++index)
                    {
                        if ((!valid[0] || (*((unsigned int *) (&c->constant[index + 0])) == Constants[0].u))
                        &&  (!valid[1] || (*((unsigned int *) (&c->constant[index + 1])) == Constants[1].u))
                        )
                        {
                            match = gcvTRUE;
                            break;
                        }
                    }
                    break;

                case 3:
                    for (index = 0; index < c->count - 2; ++index)
                    {
                        if ((!valid[0] || (*((unsigned int *) (&c->constant[index + 0])) == Constants[0].u))
                        &&  (!valid[1] || (*((unsigned int *) (&c->constant[index + 1])) == Constants[1].u))
                        &&  (!valid[2] || (*((unsigned int *) (&c->constant[index + 2])) == Constants[2].u))
                        )
                        {
                            match = gcvTRUE;
                            break;
                        }
                    }
                    break;

                case 4:
                    index = 0;

                    if ((!valid[0] || (*((unsigned int *) (&c->constant[0])) == Constants[0].u))
                    &&  (!valid[1] || (*((unsigned int *) (&c->constant[1])) == Constants[1].u))
                    &&  (!valid[2] || (*((unsigned int *) (&c->constant[2])) == Constants[2].u))
                    &&  (!valid[3] || (*((unsigned int *) (&c->constant[3])) == Constants[3].u))
                    )
                    {
                        match = gcvTRUE;
                    }
                    break;
                }

                if (match)
                {
                    if (Restricted && gcmExtractSwizzle(c->swizzle, 0) != gcSL_SWIZZLE_X)
                    {
                        continue;
                    }

                    isConstExistBefore = gcvTRUE;
                    break;
                }
            }
        }

        /* Right now we don't reuse the constant from UBO
        ** because we don't evaluate the lastUse for these constants
        ** and we don't have enough temp registers to hold such constants.
        */
        if (c != gcvNULL && c->fromUBO)
        {
            c = gcvNULL;
        }

        if (c == gcvNULL)
        {
            gctPOINTER pointer = gcvNULL;

            isConstExistBefore = gcvFALSE;

            /* Allocate a new constant. */
            gcmERR_BREAK(gcoOS_Allocate(gcvNULL,
                                        gcmSIZEOF(gcsSL_CONSTANT_TABLE),
                                        &pointer));
            gcoOS_ZeroMemory(pointer, gcmSIZEOF(gcsSL_CONSTANT_TABLE));
            c = (gcsSL_CONSTANT_TABLE_PTR)pointer;

            /* Initialize the constant. */
            c->next        = CodeGen->constants;
            c->count       = count;
            c->fromUBO     = gcvFALSE;
            for (index = 0; index < count; ++index)
            {
                c->constant[index] = Constants[index].f;
            }

            /* Link constant to head of tree. */
            CodeGen->constants = c;

            /* Allocate a physical spot for the uniform. */
            status = _FindRegisterUsage(CodeGen->uniformUsage,
                                        CodeGen->maxUniform,
                                        (count == 1)   ? gcSHADER_FLOAT_X1
                                        : (count == 2) ? gcSHADER_FLOAT_X2
                                        : (count == 3) ? gcSHADER_FLOAT_X3
                                                       : gcSHADER_FLOAT_X4,
                                        1,
                                        gcvSL_RESERVED,
                                        Restricted,
                                        &c->index,
                                        &c->swizzle,
                                        &shift,
                                        gcvNULL,
                                        0);

            if (status != gcvSTATUS_OK)
            {
                if (gcHWCaps.hwFeatureFlags.hasHalti1 &&
                    !(CodeGen->clShader))
                {
                    /* If we move this constant to the UBO, we need to emit a extra load instruction,
                    ** and the return index is a temp register index, not a uniform register index.
                    */
                    status = _AllocateConstForConstUBO(Tree, CodeGen, Usage, Constants, count, &c->index, &c->swizzle, Shift);

                    if (status == gcvSTATUS_OK)
                    {
                        *Type = gcSL_TEMP;
                        c->fromUBO = gcvTRUE;
                    }
                    else
                    {
                        CodeGen->isConstOutOfMemory = gcvTRUE;
                        return status;
                    }
                }
                else
                {
                    CodeGen->isConstOutOfMemory = gcvTRUE;
                    return status;
                }
            }

            index = 0;
        }

        /* Return constant index and swizzle. */
        *Index = c->index;

        if (c->fromUBO)
            *Type = gcSL_TEMP;

        /* when allocate a compile-time constant uniform, if this uniform use a existed constant uniform,
        *  we need to change the swizzle.
        */
        if (Shift != gcvNULL && !isConstExistBefore)
        {
            *Shift   = shift;
            *Swizzle = c->swizzle;
        }
        else
        {
            /* replicate last swizzle: .yz => .yzzz, etc */
            swizzle = c->swizzle >> (index * 2);
            switch (count)
            {
            case 1:
                swizzle = (swizzle & 0x03) | ((swizzle & 0x03) << 2);
                /* fall through */
            case 2:
                swizzle = (swizzle & 0x0F) | ((swizzle & 0x0C) << 2);
                /* fall through */
            case 3:
                swizzle = (swizzle & 0x3F) | ((swizzle & 0x30) << 2);
                /* fall through */
            default:
                break;
            }
            *Swizzle = swizzle;

            if (Shift != gcvNULL && isConstExistBefore)
                *Shift = 0;
       }

        /* Success. */
        status = gcvSTATUS_OK;
    }
    while (gcvFALSE);


    if (status != gcvSTATUS_OK)
    {
        CodeGen->isConstOutOfMemory = gcvTRUE;
    }

    /* Return the status. */
    return status;
}

gcSL_SWIZZLE
_ExtractSwizzle(
    IN gctUINT8 Swizzle,
    IN gctUINT Index
    )
{
    switch (Index)
    {
    case 0:
        return (gcSL_SWIZZLE) ((Swizzle >> 0) & 0x3);

    case 1:
        return (gcSL_SWIZZLE) ((Swizzle >> 2) & 0x3);

    case 2:
        return (gcSL_SWIZZLE) ((Swizzle >> 4) & 0x3);

    case 3:
        return (gcSL_SWIZZLE) ((Swizzle >> 6) & 0x3);

    default:
        break;
    }

    gcmFATAL("Invalid swizzle index %d.", Index);
    return gcSL_SWIZZLE_INVALID;
}

static gctUINT8
_Swizzle2Enable(
    IN gcSL_SWIZZLE X,
    IN gcSL_SWIZZLE Y,
    IN gcSL_SWIZZLE Z,
    IN gcSL_SWIZZLE W
    )
{
    static const gctUINT8 _enable[] =
    {
        gcSL_ENABLE_X,
        gcSL_ENABLE_Y,
        gcSL_ENABLE_Z,
        gcSL_ENABLE_W
    };

    /* Return combined enables for each swizzle. */
    return _enable[X] | _enable[Y] | _enable[Z] | _enable[W];
}

static gctUINT8
_AdjustSwizzle(
    IN gctUINT8 Swizzle,
    IN gctUINT32 AssignedSwizzle
    )
{
    /* Decode assigned swizzles. */
    gctUINT8 swizzle[4];
    swizzle[0] = (AssignedSwizzle >> 0) & 0x3;
    swizzle[1] = (AssignedSwizzle >> 2) & 0x3;
    swizzle[2] = (AssignedSwizzle >> 4) & 0x3;
    swizzle[3] = (AssignedSwizzle >> 6) & 0x3;

    /* Convert swizzles to their assigned values. */
    return (swizzle[(Swizzle >> 0) & 0x3] << 0) |
           (swizzle[(Swizzle >> 2) & 0x3] << 2) |
           (swizzle[(Swizzle >> 4) & 0x3] << 4) |
           (swizzle[(Swizzle >> 6) & 0x3] << 6);
}

static gcSL_SWIZZLE
_SingleEnable2Swizzle(
    IN gctUINT8 Enable
    )
{
    switch (Enable)
    {
    case gcSL_ENABLE_X:
        return gcSL_SWIZZLE_XXXX;

    case gcSL_ENABLE_Y:
        return gcSL_SWIZZLE_YYYY;

    case gcSL_ENABLE_Z:
        return gcSL_SWIZZLE_ZZZZ;

    case gcSL_ENABLE_W:
        return gcSL_SWIZZLE_WWWW;

    default:
        break;
    }

    gcmFATAL("Invalid single enable 0x%x.", Enable);
    return gcSL_SWIZZLE_INVALID;
}

gctBOOL
_GetPreviousCode(
    IN gcsCODE_GENERATOR_PTR CodeGen,
    OUT gctUINT32_PTR * Code
)
{
    if (CodeGen->previousCode != gcvNULL)
    {
        *Code = CodeGen->previousCode;
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_AddReference(
    IN gcsSL_FUNCTION_CODE_PTR Function,
    IN gctINT Reference,
    IN gcSL_INSTRUCTION Instruction,
    IN gctINT SourceIndex
    )
{
    gctSIZE_T r;

    /* Process all referecnes. */
    for (r = 0; r < gcmCOUNTOF(Function->references); ++r)
    {
        /* See if we have an empty slot. */
        if (Function->references[r].index == 0)
        {
            /* Save reference. */
            Function->references[r].index       = Reference;
            Function->references[r].instruction = Instruction;
            Function->references[r].sourceIndex = SourceIndex;

            /* Success. */
            return gcvTRUE;
        }
    }

    /* No more empty slots. */
    return gcvFALSE;
}

static gcSHADER_TYPE
_Usage2Type(
    IN gctUINT usage
    )
{
    gcSHADER_TYPE type;

    switch (usage)
    {
    case 0x1:
        /* 1-component temporary register. */
        type = gcSHADER_FLOAT_X1;
        break;

    case 0x2:
    /* fall through */
    case 0x3:
        /* 2-component temporary register. */
        type = gcSHADER_FLOAT_X2;
        break;

    case 0x4:
    /* fall through */
    case 0x5:
    /* fall through */
    case 0x6:
    /* fall through */
    case 0x7:
        /* 3-component temporary register. */
        type = gcSHADER_FLOAT_X3;
        break;

    case 0x8:
    /* fall through */
    case 0x9:
    /* fall through */
    case 0xA:
    /* fall through */
    case 0xB:
    /* fall through */
    case 0xC:
    /* fall through */
    case 0xD:
    /* fall through */
    case 0xE:
    /* fall through */
    case 0xF:
        /* 4-component temporary register. */
        type = gcSHADER_FLOAT_X4;
        break;

    default:
        /* Special case for uninitialized variables. */
        type = gcSHADER_FLOAT_X1;
        break;
    }

    return type;
}

static gcSHADER_TYPE
_Channel2Type(
    IN gctUINT usage
    )
{
    gcSHADER_TYPE type;
    gctUINT8 component = 0;
    gctUINT8 i = 0;

    for (i = 0; i < 4; i ++)
    {
        if ((usage >> i) & 0x1 )
        {
            component++;
        }
    }

    switch (component)
    {
    case 1:
        type = gcSHADER_FLOAT_X1;
        break;
    case 2:
        type = gcSHADER_FLOAT_X2;
        break;
    case 3:
        type = gcSHADER_FLOAT_X3;
        break;
    case 4:
        type = gcSHADER_FLOAT_X4;
        break;
    default:
        type = gcSHADER_FLOAT_X1;
    }

    return type;
}

static gctUINT
_Bits(
    IN gcsSL_FUNCTION_CODE_PTR Function,
    IN gctINT Reference
    )
{
    gcsSL_REFERENCE_PTR match = gcvNULL;
    gctSIZE_T r;
    gctUINT16 enable;
    gctUINT bits;

    if (Reference == 0)
    {
        return 0;
    }

    switch (Reference)
    {
    case gcSL_CG_TEMP1:
        /* fall through */
    case gcSL_CG_TEMP1_X:
        /* fall through */
    case gcSL_CG_TEMP1_XY:
        /* fall through */
    case gcSL_CG_TEMP1_XYZ:
        /* fall through */
    case gcSL_CG_TEMP1_XYZW:
        /* fall through */
    case gcSL_CG_TEMP1_X_NO_SRC_SHIFT:
        /* fall through */
    case gcSL_CG_TEMP1_XY_NO_SRC_SHIFT:
        match = &Function->tempReferences[0];
        break;

    case gcSL_CG_TEMP2:
        /* fall through */
    case gcSL_CG_TEMP2_X:
        /* fall through */
    case gcSL_CG_TEMP2_XY:
        /* fall through */
    case gcSL_CG_TEMP2_XYZ:
        /* fall through */
    case gcSL_CG_TEMP2_XYZW:
        /* fall through */
    case gcSL_CG_TEMP2_X_NO_SRC_SHIFT:
        match = &Function->tempReferences[1];
        break;

    case gcSL_CG_TEMP3:
        /* fall through */
    case gcSL_CG_TEMP3_X:
        /* fall through */
    case gcSL_CG_TEMP3_XY:
        /* fall through */
    case gcSL_CG_TEMP3_XYZ:
        /* fall through */
    case gcSL_CG_TEMP3_XYZW:
        /* fall through */
    case gcSL_CG_TEMP3_X_NO_SRC_SHIFT:
        match = &Function->tempReferences[2];
        break;

    default:
        if (Reference < 0)
        {
            Reference = -Reference;
        }
        for (r = 0; r < gcmCOUNTOF(Function->references); ++r)
        {
            if (Function->references[r].index == Reference)
            {
                match = &Function->references[r];
                break;
            }
        }
    }

    if (match == gcvNULL || match->instruction == gcvNULL)
    {
        return 0;
    }
#if !_USE_ORIGINAL_DST_WRITE_MASK_
    if (match->sourceIndex == -1)
    {
        /* Get the referenced destination. */
        enable = gcmSL_TARGET_GET(match->instruction->temp, Enable);
    }
    else
    {
        /* Get the referenced source. */
        gctSOURCE_t source = (match->sourceIndex == 0)
            ? match->instruction->source0
            : match->instruction->source1;

        enable = _Swizzle2Enable((gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleX),
                                 (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleY),
                                 (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleZ),
                                 (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleW));
    }

    bits = 0;

    if (enable & 0x1) ++bits;
    if (enable & 0x2) ++bits;
    if (enable & 0x4) ++bits;
    if (enable & 0x8) ++bits;
#else
    enable = gcmSL_TARGET_GET(match->instruction->temp, Enable);
    bits = enable;
#endif

    return bits;
}

static void
_UpdateRATempReg(
    IN gcLINKTREE              Tree,
    IN gctUINT32               TempCount
    )
{
    gcmASSERT(_isHWRegisterAllocated(Tree->shader));

    if (Tree->shader->RATempReg < TempCount)
    {
        Tree->shader->RATempReg = TempCount;
    }
}

static gctBOOL
_FindTempRefAfterRA(
    IN gcLINKTREE               Tree,
    IN gcsCODE_GENERATOR_PTR    CodeGen,
    IN gctINT                   Reference,
    IN gcsSL_PATTERN_PTR        Pattern,
    OUT gctINT                  *index,
    OUT gctINT_PTR              shift,
    OUT gctUINT8                *enable,
    OUT gctUINT8                *swizzle
    )
{
    gctUINT bits = 0;
    gctUINT bitsSrc1 = 0;
    gctUINT bitsSrc2 = 0;
    gctUINT32 components = 0, rows = 0;
    gcSHADER_TYPE type = (gcSHADER_TYPE) 0;
    gcsSL_FUNCTION_CODE_PTR function = CodeGen->current;

    gcmASSERT(_isHWRegisterAllocated(Tree->shader));

    switch (Reference)
    {
    case gcSL_CG_TEMP1:
    case gcSL_CG_TEMP1_X:
    case gcSL_CG_TEMP1_XY:
    case gcSL_CG_TEMP1_XYZ:
    case gcSL_CG_TEMP1_XYZW:
    case gcSL_CG_TEMP1_X_NO_SRC_SHIFT:
    case gcSL_CG_TEMP1_XY_NO_SRC_SHIFT:
        *index = Tree->shader->RARegWaterMark;
        _UpdateRATempReg(Tree, 1);
        break;

    case gcSL_CG_TEMP2:
    case gcSL_CG_TEMP2_X:
    case gcSL_CG_TEMP2_XY:
    case gcSL_CG_TEMP2_XYZ:
    case gcSL_CG_TEMP2_XYZW:
    case gcSL_CG_TEMP2_X_NO_SRC_SHIFT:
        *index = Tree->shader->RARegWaterMark + 1;
        _UpdateRATempReg(Tree, 2);
        break;

    case gcSL_CG_TEMP3:
    case gcSL_CG_TEMP3_X:
    case gcSL_CG_TEMP3_XY:
    case gcSL_CG_TEMP3_XYZ:
    case gcSL_CG_TEMP3_XYZW:
    case gcSL_CG_TEMP3_X_NO_SRC_SHIFT:
        *index = Tree->shader->RARegWaterMark + 2;
        _UpdateRATempReg(Tree, 3);
        break;
    default:
        return gcvFALSE;
    }
    *shift = 0;

    switch (Reference)
    {
    case gcSL_CG_TEMP1_X:
    case gcSL_CG_TEMP2_X:
    case gcSL_CG_TEMP3_X:
    case gcSL_CG_TEMP1_X_NO_SRC_SHIFT:
    case gcSL_CG_TEMP2_X_NO_SRC_SHIFT:
    case gcSL_CG_TEMP3_X_NO_SRC_SHIFT:
        *enable  = gcSL_ENABLE_X;
        *swizzle = gcSL_SWIZZLE_XXXX;
        break;

    case gcSL_CG_TEMP1_XY:
    case gcSL_CG_TEMP2_XY:
    case gcSL_CG_TEMP3_XY:
    case gcSL_CG_TEMP1_XY_NO_SRC_SHIFT:
        *enable  = gcSL_ENABLE_XY;
        *swizzle = gcSL_SWIZZLE_XYYY;
        break;

    case gcSL_CG_TEMP1_XYZ:
    case gcSL_CG_TEMP2_XYZ:
    case gcSL_CG_TEMP3_XYZ:
        *enable  = gcSL_ENABLE_XYZ;
        *swizzle = gcSL_SWIZZLE_XYZZ;
        break;

    case gcSL_CG_TEMP1_XYZW:
    case gcSL_CG_TEMP2_XYZW:
    case gcSL_CG_TEMP3_XYZW:
        *enable  = gcSL_ENABLE_XYZW;
        *swizzle = gcSL_SWIZZLE_XYZW;
        break;
    default:
        {
            if (Pattern == gcvNULL)
            {
                return gcvFALSE;
            }

            bits = _Bits(function, Pattern->source0);
            bitsSrc1 = _Bits(function, Pattern->source1);
            bitsSrc2 = _Bits(function, Pattern->source2);
            bits = gcmMAX(bits, bitsSrc1);
            bits = gcmMAX(bits, bitsSrc2);
            type = _Channel2Type(bits);
            gcTYPE_GetTypeInfo(type, &components, &rows, 0);
            switch(components)
            {
            case 1:
                *enable  = gcSL_ENABLE_X;
                *swizzle = gcSL_SWIZZLE_XXXX;
                break;
            case 2:
                *enable  = gcSL_ENABLE_XY;
                *swizzle = gcSL_SWIZZLE_XYYY;
                break;
            case 3:
                *enable  = gcSL_ENABLE_XYZ;
                *swizzle = gcSL_SWIZZLE_XYZZ;
                break;
            case 4:
                *enable  = gcSL_ENABLE_XYZW;
                *swizzle = gcSL_SWIZZLE_XYZW;
                break;
            }
        }
        break;
    }

    return gcvTRUE;
}

static gctBOOL
_FindDestReference(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctINT Reference,
    OUT gcsSL_REFERENCE_PTR * Match
    )
{
    gctUINT i;
    gcsSL_FUNCTION_CODE_PTR function = CodeGen->current;

    gcmASSERT(Reference < gcSL_CG_TEMP1_XY_NO_SRC_SHIFT);

    /* Look in all references. */
    for (i = 0; i < gcmCOUNTOF(function->references); i++)
    {
        /* See if reference matches, check DEST reference only. */
        if (function->references[i].sourceIndex == -1   &&
            function->references[i].index == Reference)
        {
            /* Return match. */
            *Match = &function->references[i];

            /* Success. */
            return gcvTRUE;
        }
    }

    /* Reference not found. */
    return gcvFALSE;
}

static gctBOOL
_FindReference(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctINT Reference,
    OUT gcsSL_REFERENCE_PTR * Match,
    IN gcsSL_PATTERN_PTR Pattern
    )
{
    gctSIZE_T r;
    gceSTATUS status;
    gcSHADER_TYPE type = (gcSHADER_TYPE) 0;
    gctINT index = 0;
    gctUINT8 swizzle = gcSL_SWIZZLE_XXXX;
    gctUINT8 enable = gcSL_ENABLE_NONE;
    gctUINT16 target;
    gcSL_INSTRUCTION instruction;
    gcsSL_REFERENCE_PTR reference;
    gctINT_PTR shift;
    gcsSL_FUNCTION_CODE_PTR function = CodeGen->current;

    switch (Reference)
    {
    case gcSL_CG_TEMP1:
        /* fall through */
    case gcSL_CG_TEMP1_X:
        /* fall through */
    case gcSL_CG_TEMP1_XY:
        /* fall through */
    case gcSL_CG_TEMP1_XYZ:
        /* fall through */
    case gcSL_CG_TEMP1_XYZW:
        /* fall through */
    case gcSL_CG_TEMP1_X_NO_SRC_SHIFT:
        /* fall through */
    case gcSL_CG_TEMP1_XY_NO_SRC_SHIFT:
        reference   = &function->tempReferences[0];
        instruction = &function->tempInstruction[0];
        shift       = &function->tempShift[0];
        break;

    case gcSL_CG_TEMP2:
        /* fall through */
    case gcSL_CG_TEMP2_X:
        /* fall through */
    case gcSL_CG_TEMP2_XY:
        /* fall through */
    case gcSL_CG_TEMP2_XYZ:
        /* fall through */
    case gcSL_CG_TEMP2_XYZW:
        /* fall through */
    case gcSL_CG_TEMP2_X_NO_SRC_SHIFT:
        reference   = &function->tempReferences[1];
        instruction = &function->tempInstruction[1];
        shift       = &function->tempShift[1];
        break;

    case gcSL_CG_TEMP3:
        /* fall through */
    case gcSL_CG_TEMP3_X:
        /* fall through */
    case gcSL_CG_TEMP3_XY:
        /* fall through */
    case gcSL_CG_TEMP3_XYZ:
        /* fall through */
    case gcSL_CG_TEMP3_XYZW:
        /* fall through */
    case gcSL_CG_TEMP3_X_NO_SRC_SHIFT:
        reference   = &function->tempReferences[2];
        instruction = &function->tempInstruction[2];
        shift       = &function->tempShift[2];
        break;

    default:
        /* Look in all references. */
        for (r = 0; r < gcmCOUNTOF(function->references); ++r)
        {
            /* See if reference matches. */
            if (function->references[r].index == Reference)
            {
                /* Return match. */
                *Match = &function->references[r];

                /* Success. */
                return gcvTRUE;
            }
        }

        /* Reference not found. */
        return gcvFALSE;
    }

    /* the following is for temporary gcSL_CG_TEMP only */
    /* See if the temporary register needs to be allocated. */
    if (reference->instruction == gcvNULL)
    {
        gctUINT bits = 0;
        gctUINT bitsSrc1 = 0;
        gctUINT bitsSrc2 = 0;
        gctBOOL noShift = gcvFALSE;
        gctINT lastUse;

         if (_isHWRegisterAllocated(Tree->shader))
        {
            if (!_FindTempRefAfterRA(Tree, CodeGen, Reference, Pattern,
                                &index, shift, &enable, &swizzle))
            {
                return gcvFALSE;
            }
        }
        else
        {
            /* Get number of components. */
            switch (Reference)
            {
            case gcSL_CG_TEMP1:
            /* fall through */
            case gcSL_CG_TEMP2:
            /* fall through */
            case gcSL_CG_TEMP3:
                if (Pattern == gcvNULL)
                {
                    return gcvFALSE;
                }

                bits = _Bits(function, Pattern->source0);
                bitsSrc1 = _Bits(function, Pattern->source1);
                bitsSrc2 = _Bits(function, Pattern->source2);
                bits = gcmMAX(bits, bitsSrc1);
                bits = gcmMAX(bits, bitsSrc2);

#if !_USE_ORIGINAL_DST_WRITE_MASK_
                switch (bits)
                {
                case 1:
                    type = gcSHADER_FLOAT_X1;
                    break;

                case 2:
                    type = gcSHADER_FLOAT_X2;
                    break;

                case 3:
                    type = gcSHADER_FLOAT_X3;
                    break;

                case 4:
                    type = gcSHADER_FLOAT_X4;
                    break;

                default:
                    break;
                }
#else
                type = _Usage2Type(bits);
#endif
                break;

            case gcSL_CG_TEMP1_X:
                /* fall through */
            case gcSL_CG_TEMP2_X:
                /* fall through */
            case gcSL_CG_TEMP3_X:
                /* One component. */
                type = gcSHADER_FLOAT_X1;
                break;

            case gcSL_CG_TEMP1_X_NO_SRC_SHIFT:
                /* fall through */
            case gcSL_CG_TEMP2_X_NO_SRC_SHIFT:
                /* fall through */
            case gcSL_CG_TEMP3_X_NO_SRC_SHIFT:
                /* One component. */
                type = gcSHADER_FLOAT_X1;
                noShift = gcvTRUE;
                break;

            case gcSL_CG_TEMP1_XY:
                /* fall through */
            case gcSL_CG_TEMP2_XY:
                /* fall through */
            case gcSL_CG_TEMP3_XY:
                /* Two components. */
                type = gcSHADER_FLOAT_X2;
                break;

            case gcSL_CG_TEMP1_XY_NO_SRC_SHIFT:
                /* Two components. */
                type = gcSHADER_FLOAT_X2;
                noShift = gcvTRUE;
                break;

            case gcSL_CG_TEMP1_XYZ:
                /* fall through */
            case gcSL_CG_TEMP2_XYZ:
                /* fall through */
            case gcSL_CG_TEMP3_XYZ:
                /* Three components. */
                type = gcSHADER_FLOAT_X3;
                break;

            case gcSL_CG_TEMP1_XYZW:
                /* fall through */
            case gcSL_CG_TEMP2_XYZW:
                /* fall through */
            case gcSL_CG_TEMP3_XYZW:
                /* Four components. */
                type = gcSHADER_FLOAT_X4;
                break;

            default:
                break;
            }

            /* Find an empty spot for the temporary register. */
            lastUse = (Tree->hints[CodeGen->nextSource - 1].lastUseForTemp == (gctINT) CodeGen->nextSource - 1) ?
                      (gctINT) CodeGen->nextSource - 1 : Tree->hints[CodeGen->nextSource - 1].lastUseForTemp;
            *shift = 0;
            status = _FindRegisterUsage(CodeGen->registerUsage,
                                CodeGen->registerCount,
                                type,
                                1,
                                lastUse /* CodeGen->nextSource - 1 */,
                                (bits > 1) || noShift,
                                &index,
                                &swizzle,
                                shift,
                                &enable,
                                0);

            if (gcmIS_ERROR(status))
            {
                /* Error. */
                return gcvFALSE;
            }
        }

        /* Setup instruction. */
        gcmASSERT(enable == _Swizzle2Enable(_ExtractSwizzle(swizzle, 0),
                                            _ExtractSwizzle(swizzle, 1),
                                            _ExtractSwizzle(swizzle, 2),
                                            _ExtractSwizzle(swizzle, 3)));

        if (_isHWRegisterAllocated(Tree->shader))
        {
            target = gcmSL_TARGET_SET(0, Enable, enable)
               | gcmSL_TARGET_SET(0, Indexed, gcSL_NOT_INDEXED)
               | gcmSL_TARGET_SET(0, Condition, gcSL_ALWAYS)
               | gcmSL_TARGET_SET(0, Format, gcSL_FLOAT);
        }
        else
        {
#if !_USE_ORIGINAL_DST_WRITE_MASK_
        target = gcmSL_TARGET_SET(0, Enable, enable)
#else
        target = gcmSL_TARGET_SET(0, Enable, (bits == 0) ? enable : (gctUINT8)(bits << *shift))
#endif
               | gcmSL_TARGET_SET(0, Indexed, gcSL_NOT_INDEXED)
               | gcmSL_TARGET_SET(0, Condition, gcSL_ALWAYS)
               | gcmSL_TARGET_SET(0, Format, gcSL_FLOAT);
        }

        /* Setup instruction. */
        instruction->temp        = target;
        instruction->tempIndex   = (-1 - index);
        instruction->tempIndexed = 0;

        /* Setup reference. */
        reference->instruction = instruction;
        reference->sourceIndex = -1;
    }

    /* Return reference. */
    *Match = reference;

    /* Success. */
    return gcvTRUE;
}

static gctUINT8
_ChangeSwizzleForInstCombine(
    IN gctUINT  defOpcode,
    IN gctUINT8 usageSwizzle,
    IN gctUINT8 defWriteMask,
    IN gctUINT8 defSwizzle)
{
    gctINT i;
    gctUINT enabledChannelIdx, newChannelSwizzle;
    gctUINT8 newSwizzle = 0, swizzle2Mask = 0;

    for (i = 0; i < 4; i ++)
    {
        /* Get enable channel index from usage swizzle */
        enabledChannelIdx = (usageSwizzle >> (i*2)) & 0x3;

        swizzle2Mask |= (1 << enabledChannelIdx);

        /* Get channel swizzle from source of def based on enabled channel */
        newChannelSwizzle = (defSwizzle >> (enabledChannelIdx*2)) & 0x3;

        /* Set this new channel swizzle */
        newSwizzle |= (newChannelSwizzle << (i*2));
    }

    /* Only valid for same enable */
    gcmASSERT(swizzle2Mask == defWriteMask);

    /* For un-component-wised operations, such as DPx, do not change swizzle
       since they must not be used to combine into other component-wised insts.
       For cases that other insts (MOVs in general) are used to backforward combine
       into un-component-wised operations, it also don't need change swizzle of them */
    if (defOpcode == gcSL_DP3 || defOpcode == gcSL_DP4 || defOpcode == gcSL_DP2 ||
        defOpcode == gcSL_CROSS || defOpcode == gcSL_NORM ||
        defOpcode == gcSL_IMAGE_ADDR || defOpcode == gcSL_IMAGE_RD || defOpcode == gcSL_IMAGE_WR ||
        defOpcode == gcSL_IMAGE_RD_3D || defOpcode == gcSL_IMAGE_WR_3D)
    {
        newSwizzle = defSwizzle;
    }

    return newSwizzle;
}

static gctBOOL
_CompareDestOfInstruction(
    IN gcSL_INSTRUCTION TargetSource,
    IN gcSL_INSTRUCTION Source
    )
{
    gctBOOL matched = gcvTRUE;

    if (/* No need to check enable/format/precision/condition. */
        /*TargetSource->temp != Source->temp              ||*/
        TargetSource->tempIndex != Source->tempIndex    ||
        TargetSource->tempIndexed != Source->tempIndexed)
    {
        matched = gcvFALSE;
    }

    return matched;
}

static gctBOOL
_CompareSourceOfInstruction(
    IN gcSL_INSTRUCTION TargetSource,
    IN gctINT TargetSourceIndex,
    IN gcSL_INSTRUCTION Source,
    IN gctINT SourceIndex,
    OUT gctBOOL * IsSourceChange
    )
{
    gctTARGET_t target = TargetSource->temp;

    /* Extract proper source values. */
    gctSOURCE_t source  = (SourceIndex == 0)
                             ? Source->source0
                             : Source->source1;
    gctUINT32 index   = (SourceIndex == 0)
                             ? Source->source0Index
                             : Source->source1Index;
    gctUINT16 indexed = (SourceIndex == 0)
                             ? Source->source0Indexed
                             : Source->source1Indexed;

    /* Dispatch on target index. */
    switch (TargetSourceIndex)
    {
    case -1:
        /* Compare with destination: source must be a temp */
        if ((gcmSL_SOURCE_GET(source, Type) == gcSL_TEMP)
        &&   /* With the same index. */
             (index == TargetSource->tempIndex)
        &&   /* Of the same format. */
             (((gcmSL_SOURCE_GET(source, Format) == gcSL_FLOAT) &&
               (gcmSL_TARGET_GET(target, Format) == gcSL_FLOAT)) ||
              ((gcmSL_SOURCE_GET(source, Format) != gcSL_FLOAT) &&
               (gcmSL_TARGET_GET(target, Format) != gcSL_FLOAT)))
        &&   /* The swizzle must match the enables. */
             (_Swizzle2Enable((gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleX),
                              (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleY),
                              (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleZ),
                              (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleW)) == gcmSL_TARGET_GET(target, Enable))
        &&   /* With the same index mode. */
             (gcmSL_SOURCE_GET(source, Indexed) == (gctSOURCE_t)gcmSL_TARGET_GET(target, Indexed))
        &&   /* And indexed register. */
             (indexed == TargetSource->tempIndexed)
        )
        {
            /* Only swizzle matches write-enable is not enough, for example,
               mul r0.xy, r1.xy, r2.xy
               add r3.zw, r0.xyxy, r3.zwzw
               although dest of mul and source1 of add has same swizzle/enable,
               but we can not directly say they can be merged to mad unless we
               do some changes for source of mul.
            */
            gctUINT8 newSwizzle;
            gctSOURCE_t source0, source1;
            gctUINT defOpcode = gcmSL_OPCODE_GET(TargetSource->opcode, Opcode);

            /* source 0 */
            newSwizzle = _ChangeSwizzleForInstCombine(
                                            defOpcode,
                                            gcmSL_SOURCE_GET(source, Swizzle),
                                            gcmSL_TARGET_GET(target, Enable),
                                            gcmSL_SOURCE_GET(TargetSource->source0, Swizzle));

            source0 = gcmSL_SOURCE_SET(TargetSource->source0, Swizzle, newSwizzle);

            if (TargetSource->source0 != source0)
                *IsSourceChange = gcvTRUE;

            TargetSource->source0 = source0;

            /* source 1 */
            newSwizzle = _ChangeSwizzleForInstCombine(
                                            defOpcode,
                                            gcmSL_SOURCE_GET(source, Swizzle),
                                            gcmSL_TARGET_GET(target, Enable),
                                            gcmSL_SOURCE_GET(TargetSource->source1, Swizzle));

            source1 = gcmSL_SOURCE_SET(TargetSource->source1, Swizzle, newSwizzle);

            if (TargetSource->source0 != source0)
                *IsSourceChange = gcvTRUE;

            TargetSource->source1 = source1;

            /* We have a match. */
            return gcvTRUE;
        }
        break;

    case 0:
        /* Compare with source 0. */
        if ((source  == TargetSource->source0)
        &&  (index   == TargetSource->source0Index)
        &&  (indexed == TargetSource->source0Indexed)
        )
        {
            /* We have a match. */
            return gcvTRUE;
        }
        break;

    case 1:
        /* Compare with source 1. */
        if ((source  == TargetSource->source1)
        &&  (index   == TargetSource->source1Index)
        &&  (indexed == TargetSource->source1Indexed)
        )
        {
            /* We have a match. */
            return gcvTRUE;
        }
        break;

    default:
        break;
    }

    /* No match. */
    return gcvFALSE;
}

static gcsSL_PATTERN_PTR
_FindPattern(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcsSL_PATTERN_PTR *Patterns,
    IN gcSL_INSTRUCTION Code,
    IN OUT gctINT * CodeCount
    )
{
    gcsSL_PATTERN_PTR p;
    gctSIZE_T index = 0, i = 0;
    gcsSL_REFERENCE_PTR match;
    gctINT instructions = 0;
    gcsSL_FUNCTION_CODE_PTR function = CodeGen->current;
    gctUINT loopCount, loopCountA = 0;
    gctUINT opcode = gcmSL_OPCODE_GET(Code->opcode, Opcode);
    gctBOOL IsSourceChange;
    struct _gcSL_INSTRUCTION code[_TEMP_INSTRUCTION_COUNT_];

    if (opcode >= gcSL_MAXOPCODE)
    {
        /* Invalid opcode. */
        return gcvNULL;
    }

    /* Process all patterns. */
    for (p = Patterns[opcode], loopCount = 0; p->count > 0 && loopCount < MAX_LOOP_COUNT; loopCount++)
    {
        /* Zero the references. */
        gcoOS_ZeroMemory(function->references, gcmSIZEOF(function->references));
        gcoOS_ZeroMemory(function->tempReferences, gcmSIZEOF(function->tempReferences));

        /* Zero the temp instruction. */
        gcoOS_ZeroMemory(code, _TEMP_INSTRUCTION_COUNT_ * sizeof(struct _gcSL_INSTRUCTION));

        IsSourceChange = gcvFALSE;

        /* Skip those search patterns that have more instructions than are
           available. */
        if (p->count <= (gctINT) *CodeCount)
        {
            /* Start a new search. */
            index        = 0;
            instructions = p->count;

            /* Process all search patterns. */
            while (p->count > 0 && loopCountA < MAX_LOOP_COUNT)
            {
                /* Make sure the code number of a pattern is smaller than the max temp code size. */
                gcmASSERT(index < _TEMP_INSTRUCTION_COUNT_);
                gcoOS_MemCopy(&code[index], Code + index, sizeof(struct _gcSL_INSTRUCTION));
                loopCountA++;
                /* Bail out if the opcode doesn't match. */
                if (p->opcode != (gctUINT)gcmSL_OPCODE_GET(Code[index].opcode, Opcode))
                {
                    break;
                }

                /* Bail out if function returns gcvFALSE. */
                if (p->function != gcvNULL)
                {
                    if (!(*p->function)(Tree, CodeGen, Code + index, gcvNULL))
                    {
                        break;
                    }
                }

                /* Process destination search pattern. */
                if (p->dest != 0)
                {
                    if (_FindDestReference(Tree,
                                           CodeGen,
                                           p->dest,
                                           &match))
                    {
                        /* Compare destination reference. */
                        if (!_CompareDestOfInstruction(match->instruction,
                                                       Code + index))
                        {
                            /* Bail out if destination doesn't match. */
                            break;
                        }
                    }
                    else
                    {
                        _AddReference(function,
                                      p->dest,
                                      Code + index,
                                      -1);
                    }
                }

                /* Process source 0 search pattern. */
                if (p->source0 != 0)
                {
                    /* Check is source 0 reference is already used. */
                    if (_FindReference(Tree, CodeGen,
                                       gcmABS(p->source0),
                                       &match,
                                       gcvNULL))
                    {
                        /* Compare source 0 reference. */
                        if (!_CompareSourceOfInstruction(match->instruction,
                                                         match->sourceIndex,
                                                         Code + index,
                                                         0,
                                                         &IsSourceChange))
                        {
                            /* Bail out if source 0 doesn't match. */
                            break;
                        }
                    }

                    else
                    {
                        /* Add source 0 reference. */
                        if (!_AddReference(function,
                                           p->source0,
                                           Code + index,
                                           0))
                        {
                            /* Bail out of on any error. */
                            break;
                        }
                    }
                }

                /* Process source 1 search pattern. */
                if (p->source1 != 0)
                {
                    /* Check is source 1 reference is already used. */
                    if (_FindReference(Tree, CodeGen,
                                       gcmABS(p->source1),
                                       &match,
                                       gcvNULL))
                    {
                        /* Compare source 1 reference. */
                        if (!_CompareSourceOfInstruction(match->instruction,
                                                         match->sourceIndex,
                                                         Code + index,
                                                         1,
                                                         &IsSourceChange))
                        {
                            /* Bail out if source 1 doesn't match. */
                            break;
                        }
                    }
                    else
                    {
                        /* Add source 1 reference. */
                        if (!_AddReference(function,
                                           p->source1,
                                           Code + index,
                                           1))
                        {
                            /* Bail out of on any error. */
                            break;
                        }
                    }
                }

                if ((index > 0)
                &&  (p->source0 != 0)
                &&  (p->source1 != 0)
                &&  (gcmABS(p->source0) != gcmABS(p->source1))
                )
                {
                    if (_FindReference(Tree, CodeGen,
                                       gcmABS(p->source0),
                                       &match,
                                       gcvNULL))
                    {
                        if (_CompareSourceOfInstruction(match->instruction,
                                                        match->sourceIndex,
                                                        Code + index,
                                                        1,
                                                        &IsSourceChange))
                        {
                            /* Bail out if the sources match. */
                            break;
                        }
                    }
                }

                /* Next pattern. */
                ++p;
                ++index;
            }
        }

        if (p->count < 0)
        {
            /* Bail if we have a matching pattern. */
            break;
        }

        /* If we change the swizzle for instruction combine on a mismatch pattern,
           we need to change the swizzle to the original value.
        */
        if (IsSourceChange)
        {
            for (i = 0; i < index; i++)
            {
                gcmASSERT(i < _TEMP_INSTRUCTION_COUNT_);
                gcoOS_MemCopy(Code + i, &code[i], sizeof(struct _gcSL_INSTRUCTION));
            }
        }

        /* Skip over pattern matches. */
        p += p->count;

        /* Skip over code generation. */
        p += -p->count;
    }

    /* Test for unsupported pattern. */
    if (p->count == 0)
    {
        dbg_dumpIR(Code, (gctINT)(Code - Tree->shader->code));
        gcmFATAL("!!TODO!! Code pattern not yet supported.");
        p = gcvNULL;
    }

    /* Return number of matched instructions. */
    *CodeCount = instructions;

    /* Return pattern pointer. */
    return p;
}

static gctBOOL
_RegisterIsAvailable(
    IN gcsSL_USAGE_PTR Usage,
    IN gctINT Rows,
    IN gctUINT8 Enable
    )
{
    /* Loop through all rows. */
    while (Rows-- > 0)
    {
        /* Test if x-component is available. */
        if ((Enable & 0x1) && (Usage->lastUse[0] != gcvSL_AVAILABLE) )
        {
            return gcvFALSE;
        }

        /* Test if y-component is available. */
        if ((Enable & 0x2) && (Usage->lastUse[1] != gcvSL_AVAILABLE) )
        {
            return gcvFALSE;
        }

        /* Test if z-component is available. */
        if ((Enable & 0x4) && (Usage->lastUse[2] != gcvSL_AVAILABLE) )
        {
            return gcvFALSE;
        }

        /* Test if w-component is available. */
        if ((Enable & 0x8) && (Usage->lastUse[3] != gcvSL_AVAILABLE) )
        {
            return gcvFALSE;
        }

        /* Test next row. */
        ++Usage;
    }

    /* All requested rows and components are available. */
    return gcvTRUE;
}

static void
_SetRegisterUsage(
    IN gcsSL_USAGE_PTR Usage,
    IN gctINT          Rows,
    IN gctUINT8        Enable,
    IN gctINT          LastUse
    )
{
    /* Process all rows. */
    while (Rows-- > 0)
    {
        /* Set last usage for x-component if enabled. */
        if (Enable & gcSL_ENABLE_X)
        {
            Usage->lastUse[0] = LastUse;
        }

        /* Set last usage for y-component if enabled. */
        if (Enable & gcSL_ENABLE_Y)
        {
            Usage->lastUse[1] = LastUse;
        }

        /* Set last usage for z-component if enabled. */
        if (Enable & gcSL_ENABLE_Z)
        {
            Usage->lastUse[2] = LastUse;
        }

        /* Set last usage for w-component if enabled. */
        if (Enable & gcSL_ENABLE_W)
        {
            Usage->lastUse[3] = LastUse;
        }

        /* Process next row. */
        ++Usage;
    }
}

static gceSTATUS
_FindRegisterUsage(
    IN OUT gcsSL_USAGE_PTR    Usage,
    IN gctSIZE_T              Count,
    IN gcSHADER_TYPE          Type,
    IN gctINT                 Length,
    IN gctINT                 LastUse,
    IN gctBOOL                Restricted,
    OUT gctINT_PTR            Physical,
    OUT gctUINT8_PTR          Swizzle,
    OUT gctINT_PTR            Shift,
    OUT gctUINT8_PTR          Enable,
    IN gctSIZE_T              StartIndex
    )
{
    gctSIZE_T i;
    gctUINT32 components = 0, rows = 0;
    gctINT shift;
    gctUINT8 swizzle = 0, enable = 0;

    /* Determine the number of required rows and components. */
    gcTYPE_GetTypeInfo(Type, &components, &rows, 0);
    rows *= Length;

    gcmASSERT(StartIndex < Count);

    if (Count < rows)
    {
        /* Not enough hardware resources. */
        return gcvSTATUS_OUT_OF_RESOURCES;
    }

    /* Walk through all possible usages. */
    for (i = StartIndex; i <= Count - rows; ++i)
    {
        /* Assume there is no room. */
        shift = -1;

        /* Test number of required components. */
        switch (components)
        {
        case 1:
            /* See if x-component is available. */
            if (_RegisterIsAvailable(Usage + i, rows, 0x1 << 0))
            {
                shift   = 0;
                enable  = gcSL_ENABLE_X;
                swizzle = gcSL_SWIZZLE_XXXX;
            }

            /* See if y-component is available. */
            else if (!Restricted && _RegisterIsAvailable(Usage + i, rows, 0x1 << 1))
            {
                shift   = 1;
                enable  = gcSL_ENABLE_Y;
                swizzle = gcSL_SWIZZLE_YYYY;
            }

            /* See if z-component is available. */
            else if (!Restricted && _RegisterIsAvailable(Usage + i, rows, 0x1 << 2))
            {
                shift   = 2;
                enable  = gcSL_ENABLE_Z;
                swizzle = gcSL_SWIZZLE_ZZZZ;
            }

            /* See if w-component is available. */
            else if (!Restricted && _RegisterIsAvailable(Usage + i, rows, 0x1 << 3))
            {
                shift   = 3;
                enable  = gcSL_ENABLE_W;
                swizzle = gcSL_SWIZZLE_WWWW;
            }

            break;

        case 2:
            /* See if x- and y-components are available. */
            if (_RegisterIsAvailable(Usage + i, rows, 0x3 << 0))
            {
                shift   = 0;
                enable  = gcSL_ENABLE_XY;
                swizzle = gcSL_SWIZZLE_XYYY;
            }

            /* See if y- and z-components are available. */
            else if (!Restricted && _RegisterIsAvailable(Usage + i, rows, 0x3 << 1))
            {
                shift   = 1;
                enable  = gcSL_ENABLE_YZ;
                swizzle = gcSL_SWIZZLE_YZZZ;
            }

            /* See if z- and w-components are available. */
            else if (!Restricted && _RegisterIsAvailable(Usage + i, rows, 0x3 << 2))
            {
                shift   = 2;
                enable  = gcSL_ENABLE_ZW;
                swizzle = gcSL_SWIZZLE_ZWWW;
            }

            break;

        case 3:
            /* See if x-, y- and z-components are available. */
            if (_RegisterIsAvailable(Usage + i, rows, 0x7 << 0))
            {
                shift   = 0;
                enable  = gcSL_ENABLE_XYZ;
                swizzle = gcSL_SWIZZLE_XYZZ;
            }

            /* See if y-, z- and w-components are available. */
            else if (!Restricted && _RegisterIsAvailable(Usage + i, rows, 0x7 << 1))
            {
                shift   = 1;
                enable  = gcSL_ENABLE_YZW;
                swizzle = gcSL_SWIZZLE_YZWW;
            }

            break;

        case 4:
            /* See if x-, y-, z- and w-components are available. */
            if (_RegisterIsAvailable(Usage + i, rows, 0xF << 0))
            {
                shift   = 0;
                enable  = gcSL_ENABLE_XYZW;
                swizzle = gcSL_SWIZZLE_XYZW;
            }

            break;

        default:
            break;
        }

        /* See if there is enough room. */
        if (shift >= 0)
        {
            /* Return allocation. */
            *Physical = i;
            *Shift    += shift;
            *Swizzle  = swizzle;
            if (Enable)
            {
                *Enable = enable;
            }

            /* Set the usage. */
            _SetRegisterUsage(Usage + i, rows, enable, LastUse);

            /* Success. */
            return gcvSTATUS_OK;
        }
    }

    /* Out of resources. */
    gcmUSER_DEBUG_ERROR_MSG("Not enough register memory");
    return gcvSTATUS_OUT_OF_RESOURCES;
}

static gctBOOL
_IsSampler(gcSHADER_TYPE Type)
{
    return (gcmType_Kind(Type) == gceTK_SAMPLER);
}

static gcSHADER_TYPE_KIND
_GetSamplerKind(gcSHADER_TYPE SamplerType)
{
    switch(SamplerType ) {
    case gcSHADER_SAMPLER_1D:
    case gcSHADER_SAMPLER_2D:
    case gcSHADER_SAMPLER_3D:
    case gcSHADER_SAMPLER_BUFFER:
    case gcSHADER_SAMPLER_EXTERNAL_OES:
    case gcSHADER_SAMPLER_2D_SHADOW:
    case gcSHADER_SAMPLER_CUBE_SHADOW:
    case gcSHADER_SAMPLER_CUBIC:
    case gcSHADER_SAMPLER_1D_ARRAY:
    case gcSHADER_SAMPLER_1D_ARRAY_SHADOW:
    case gcSHADER_SAMPLER_2D_ARRAY:
    case gcSHADER_SAMPLER_2D_ARRAY_SHADOW:
    case gcSHADER_SAMPLER_2D_RECT:
    case gcSHADER_SAMPLER_2D_RECT_SHADOW:
    case gcSHADER_SAMPLER_1D_SHADOW:
        return gceTK_FLOAT;

    case gcSHADER_ISAMPLER_2D:
    case gcSHADER_ISAMPLER_3D:
    case gcSHADER_ISAMPLER_BUFFER:
    case gcSHADER_ISAMPLER_CUBIC:
    case gcSHADER_ISAMPLER_2D_ARRAY:
    case gcSHADER_ISAMPLER_2D_RECT:
    case gcSHADER_ISAMPLER_1D_ARRAY:
    case gcSHADER_ISAMPLER_1D:
        return gceTK_INT;

    case gcSHADER_USAMPLER_2D:
    case gcSHADER_USAMPLER_3D:
    case gcSHADER_USAMPLER_BUFFER:
    case gcSHADER_USAMPLER_CUBIC:
    case gcSHADER_USAMPLER_2D_ARRAY:
    case gcSHADER_USAMPLER_2D_RECT:
    case gcSHADER_USAMPLER_1D_ARRAY:
    case gcSHADER_USAMPLER_1D:
        return gceTK_UINT;

    case gcSHADER_SAMPLER_2D_MS:
    case gcSHADER_SAMPLER_2D_MS_ARRAY:
        return gceTK_FLOAT;

    case gcSHADER_ISAMPLER_2D_MS:
    case gcSHADER_ISAMPLER_2D_MS_ARRAY:
        return gceTK_INT;

    case gcSHADER_USAMPLER_2D_MS:
    case gcSHADER_USAMPLER_2D_MS_ARRAY:
        return gceTK_UINT;

    case gcSHADER_IIMAGE_2D:                     /* 0x39 */
    case gcSHADER_IIMAGE_3D:                     /* 0x3B */
    case gcSHADER_IIMAGE_CUBE:                   /* 0x3E */
    case gcSHADER_IIMAGE_2D_ARRAY:               /* 0x41 */
        return gceTK_INT;

    case gcSHADER_UIMAGE_2D:                     /* 0x3A */
    case gcSHADER_UIMAGE_3D:                     /* 0x3C */
    case gcSHADER_UIMAGE_CUBE:                   /* 0x3F */
    case gcSHADER_UIMAGE_2D_ARRAY:               /* 0x42 */
        return gceTK_UINT;

    case gcSHADER_IMAGE_2D:                      /* 0x17 */
    case gcSHADER_IMAGE_3D:                      /* 0x18 */
    case gcSHADER_IMAGE_CUBE:                    /* 0x3D */
    case gcSHADER_IMAGE_2D_ARRAY:                /* 0x40 */
        return gceTK_FLOAT;

    case gcSHADER_IMAGE_BUFFER:
    case gcSHADER_IIMAGE_BUFFER:
    case gcSHADER_UIMAGE_BUFFER:
        return gceTK_FLOAT;

    default:
        return gcmType_Kind(SamplerType);
    }
}

static gceSTATUS
_MapNonSamplerUniforms(
    IN gcLINKTREE            Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN OUT gcsSL_USAGE_PTR   Usage,
    IN gctSIZE_T             UsageCount,
    IN gctINT                UniformIndex,
    OUT gctINT *             NextUniformIndex,
    gctBOOL                  Initialized,
    gctSIZE_T                StartIndex
    )
{
    gceSTATUS status;
    gctINT    shift = 0, lastUniformIndex, i;
    gcSHADER_TYPE type = gcSHADER_FLOAT_X4;
    gctINT arraySize = 0, physical;
    gctSIZE_T maxComp = 0;
    gctUINT8 swizzle = 0;
    gctBOOL unblockUniformBlock = gcvFALSE;
    gctBOOL handleDefaultUBO = gcvFALSE;
    gctBOOL restricted = gcvFALSE;

    /* Determine base address for uniforms. */
    const gctUINT32 uniformBaseAddress = CodeGen->uniformBase * 4;

    gcmONERROR(gcSHADER_GetUniformIndexingRange(Tree->shader,
                                                UniformIndex,
                                                -1,
                                                &lastUniformIndex,
                                                gcvNULL,
                                                gcvNULL));

    if (Tree->shader->uniformBlockCount)
    {
        if(Tree->shader->enableDefaultUBO  &&
           gcHWCaps.hwFeatureFlags.hasHalti1)
        {
            handleDefaultUBO = gcvTRUE;
        }
        else if (!gcHWCaps.hwFeatureFlags.hasHalti1 ||
               (Tree->hints && Tree->hints->uploadedUBO))
        {
            unblockUniformBlock = gcvTRUE;
        }
        else
        {
            unblockUniformBlock = gcvFALSE;
        }
    }

    for (i = UniformIndex; i <= lastUniformIndex; i ++)
    {
        /* Get uniform. */
        gcUNIFORM uniform = Tree->shader->uniforms[i];

        if(!uniform) continue;

        switch (GetUniformCategory(uniform))
        {
        case gcSHADER_VAR_CATEGORY_NORMAL:
        case gcSHADER_VAR_CATEGORY_LOD_MIN_MAX:
        case gcSHADER_VAR_CATEGORY_LEVEL_BASE_SIZE:
        case gcSHADER_VAR_CATEGORY_SAMPLE_LOCATION:
        case gcSHADER_VAR_CATEGORY_ENABLE_MULTISAMPLE_BUFFERS:
        case gcSHADER_VAR_CATEGORY_WORK_THREAD_COUNT:
        case gcSHADER_VAR_CATEGORY_WORK_GROUP_COUNT:
        case gcSHADER_VAR_CATEGORY_WORK_GROUP_ID_OFFSET:
        case gcSHADER_VAR_CATEGORY_GLOBAL_INVOCATION_ID_OFFSET:
            break;

        case gcSHADER_VAR_CATEGORY_BLOCK_ADDRESS:
            if(isUniformStorageBlockAddress(uniform)) break;
            if(isUniformConstantAddressSpace(uniform)) break;
            if(handleDefaultUBO)
            {
                if(!isUniformUsedInShader(uniform)) continue;
            }
            else if(unblockUniformBlock)
            {
                continue;
            }
            break;

        case gcSHADER_VAR_CATEGORY_BLOCK_MEMBER:
            if(handleDefaultUBO)
            {
                if(!isUniformMovedToDUB(uniform)) continue;
            }
            else if(!unblockUniformBlock) continue;
            break;

        default:
            continue;
        }

        /*
        ** When allocate constant register for #num_group, we should always allocate XYZ for it
        ** because HW always use XYZ for this uniform.
        */
        if (uniform->nameLength == 11 &&
            gcmIS_SUCCESS(gcoOS_StrCmp(uniform->name, "#num_groups")))
        {
            restricted = gcvTRUE;
        }

        if (!_IsSampler(uniform->u.type))
        {
            gctUINT32 components = 0, rows = 0;
            gcTYPE_GetTypeInfo(uniform->u.type, &components, &rows, 0);
            if(gcmType_Kind(uniform->u.type) == gceTK_ATOMIC ||
               isUniformMatrix(uniform) ||
               !isUniformArray(uniform) ||
               !isUniformNormal(uniform) ||
               !(Tree->flags & gcvSHADER_REMOVE_UNUSED_UNIFORMS))
            {
                SetUniformUsedArraySize(uniform, GetUniformArraySize(uniform));
            }

            rows *= GetUniformUsedArraySize(uniform);

            if (maxComp < components)
                maxComp = components;

            arraySize += rows;
        }
    }

    switch (maxComp)
    {
    case 1:
        type = gcSHADER_FLOAT_X1;
        break;
    case 2:
        type = gcSHADER_FLOAT_X2;
        break;
    case 3:
        type = gcSHADER_FLOAT_X3;
        break;
    case 4:
        type = gcSHADER_FLOAT_X4;
        break;

    case 0: /* handles case when entire uniform is skipped */
        type = gcSHADER_FLOAT_X1;
        break;

    default:
        gcmASSERT(0);
        break;
    }

    if (arraySize > 0)
    {
        if (Initialized)
        {
            gcUNIFORM uniform = Tree->shader->uniforms[UniformIndex];
            gcSL_ENABLE Usage;
            gcSL_TYPE constType;

            gcmASSERT(UniformIndex == lastUniformIndex);

            Usage =  (type == gcSHADER_FLOAT_X1) ? gcSL_ENABLE_X    :
                     (type == gcSHADER_FLOAT_X2) ? gcSL_ENABLE_XY   :
                     (type == gcSHADER_FLOAT_X3) ? gcSL_ENABLE_XYZ  : gcSL_ENABLE_XYZW;

            if (GetUniformOffset(uniform) >= 0)
            {
                gctUINT8 * data = (gctUINT8*)(GetShaderConstantMemoryBuffer(Tree->shader) + GetUniformOffset(uniform));
                gctINT32 arrayStride = GetUniformArrayStride(uniform);

                /* Since this uniform is an array, we can't reuse the existed constant and need to make sure it is allocated restrictly */
                for (i = 0; i < GetUniformUsedArraySize(uniform); i++)
                {
                    gcmONERROR(_AllocateConst(Tree,
                                              CodeGen,
                                              Usage,
                                              (gcuFLOAT_UINT32 *)(data + i * arrayStride),
                                              gcvTRUE,
                                              gcvFALSE,
                                              &physical,
                                              &swizzle,
                                              &shift,
                                              &constType));

                    if (i == 0)
                    {
                        uniform->swizzle  = swizzle;
                        uniform->physical = physical;
                        uniform->address  = uniformBaseAddress + uniform->physical * 16 + shift * 4;
                        uniform->RAPriority++;
                    }
                }
            }
            else
            {
                gcmONERROR(_AllocateConst(Tree,
                                          CodeGen,
                                          Usage,
                                          (gcuFLOAT_UINT32 *) uniform->initializer.f32_v4,
                                          gcvFALSE,
                                          gcvTRUE,
                                          &physical,
                                          &swizzle,
                                          &shift,
                                          &constType));

                /* We should have enough uniform now. */
                gcmASSERT(constType == gcSL_UNIFORM);

                uniform->swizzle  = swizzle;
                uniform->physical = physical;
                uniform->address  = uniformBaseAddress + uniform->physical * 16 + shift * 4;
                uniform->RAPriority++;
            }
        }
        else
        {
            /* Find physical location for uniform. */
            gcmONERROR(_FindRegisterUsage(Usage, UsageCount,
                                  type,
                                  arraySize,
                                  gcvSL_RESERVED,
                                  restricted,
                                  &physical,
                                  &swizzle,
                                  &shift,
                                  gcvNULL,
                                  StartIndex));

            /* Set physical address for uniform. */
            for (i = UniformIndex; i <= lastUniformIndex; i ++)
            {
                gcUNIFORM uniform = Tree->shader->uniforms[i];

                if (!uniform) continue;

                if (isUniformSampler(uniform)) continue;

                switch (GetUniformCategory(uniform)) {
                case gcSHADER_VAR_CATEGORY_NORMAL:
                case gcSHADER_VAR_CATEGORY_LOD_MIN_MAX:
                case gcSHADER_VAR_CATEGORY_LEVEL_BASE_SIZE:
                case gcSHADER_VAR_CATEGORY_SAMPLE_LOCATION:
                case gcSHADER_VAR_CATEGORY_ENABLE_MULTISAMPLE_BUFFERS:
                case gcSHADER_VAR_CATEGORY_WORK_THREAD_COUNT:
                case gcSHADER_VAR_CATEGORY_WORK_GROUP_COUNT:
                case gcSHADER_VAR_CATEGORY_WORK_GROUP_ID_OFFSET:
                case gcSHADER_VAR_CATEGORY_GLOBAL_INVOCATION_ID_OFFSET:
                    break;

                case gcSHADER_VAR_CATEGORY_BLOCK_ADDRESS:
                    if(isUniformStorageBlockAddress(uniform)) break;
                    if(isUniformConstantAddressSpace(uniform)) break;
                    if(handleDefaultUBO)
                    {
                        if(!isUniformUsedInShader(uniform)) continue;
                    }
                    else if(unblockUniformBlock)
                    {
                        continue;
                    }
                    break;

                case gcSHADER_VAR_CATEGORY_BLOCK_MEMBER:
                    if(handleDefaultUBO)
                    {
                        if(!isUniformMovedToDUB(uniform)) continue;
                    }
                    else if(!unblockUniformBlock) continue;
                    break;

                default:
                    continue;
                }

                if(gcmType_Kind(uniform->u.type) == gceTK_ATOMIC)
                {
                    gcmASSERT(uniform->baseBindingIdx >= 0);

                    if(Tree->shader->uniforms[uniform->baseBindingIdx]->physical == -1)
                    {
                        gctUINT32 components = 0, rows = 0;
                        gcUNIFORM baseUniform = Tree->shader->uniforms[uniform->baseBindingIdx];

                        baseUniform->swizzle = swizzle;
                        baseUniform->physical = physical;
                        baseUniform->address = uniformBaseAddress +
                            baseUniform->physical * 16 + shift * 4;

                        gcTYPE_GetTypeInfo(baseUniform->u.type, &components, &rows, 0);
                        rows *= baseUniform->arraySize;
                        physical += rows;
                    }

                    uniform->swizzle  = Tree->shader->uniforms[uniform->baseBindingIdx]->swizzle;
                    uniform->physical = Tree->shader->uniforms[uniform->baseBindingIdx]->physical;
                    uniform->address  = Tree->shader->uniforms[uniform->baseBindingIdx]->address;
                }
                else if (!_IsSampler(uniform->u.type))
                {
                    gctUINT32 components = 0, rows = 0;

                    uniform->swizzle = swizzle;
                    uniform->physical = physical;
                    uniform->address = uniformBaseAddress +
                                       uniform->physical * 16 + shift * 4;

                    gcTYPE_GetTypeInfo(uniform->u.type, &components, &rows, 0);
                    rows *= GetUniformUsedArraySize(uniform);
                    physical += rows;
                }
                uniform->RAPriority++;
            }
        }
    }

    if (NextUniformIndex)
        *NextUniformIndex = lastUniformIndex + 1;

    /* Success. */
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    return status;
}

gceSTATUS
_VIR_MapUniforms(
    IN gcLINKTREE            Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN OUT gcsSL_USAGE_PTR   Usage,
    IN gctSIZE_T             UsageCount,
    IN gctSIZE_T             StartIndex
    )
{
    gctINT i;
    gcSHADER shader = Tree->shader;
    gctUINT32 components = 0, rows = 0;
    gctUINT8  enable = 0;

    /* Mark Usage based on the uniform info allocated in VIR */
    for (i = 0; i < (gctINT)shader->uniformCount; ++i)
    {
        /* Get uniform. */
        gcUNIFORM uniform = shader->uniforms[i];

        if(!uniform) continue;

        gcTYPE_GetTypeInfo(uniform->u.type, &components, &rows, 0);
        rows *= uniform->arraySize;

        enable = _Swizzle2Enable(_ExtractSwizzle(uniform->swizzle, 0),
                                 _ExtractSwizzle(uniform->swizzle, 1),
                                 _ExtractSwizzle(uniform->swizzle, 2),
                                 _ExtractSwizzle(uniform->swizzle, 3));

        if (uniform->physical >=0 )
        {
            _SetRegisterUsage(Usage + uniform->physical,
                              rows,
                              enable,
                              gcvSL_RESERVED);
        }

        if (isUniformCompiletimeInitialized(uniform))
        {
            gctUINT8 Usage =  (uniform->u.type == gcSHADER_FLOAT_X1) ? gcSL_ENABLE_X    :
                              (uniform->u.type == gcSHADER_FLOAT_X2) ? gcSL_ENABLE_XY   :
                              (uniform->u.type == gcSHADER_FLOAT_X3) ? gcSL_ENABLE_XYZ  : gcSL_ENABLE_XYZW;

            _VIR_AllocateConst(Tree,
                            CodeGen,
                            Usage,
                            (gcuFLOAT_UINT32 *) uniform->initializer.f32_v4,
                            uniform);
        }
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_CalcUniformCount(
    IN gcSHADER             Shader,
    IN gctBOOL              unblockUniformBlock,
    IN gctBOOL              handleDefaultUBO,
    IN gctUINT32*           ConstantCount,
    IN gctUINT32*           SamplerCount
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 constantCount = 0, samplerCount = 0;
    gctUINT i;

    for (i = 0; i < (gctINT)Shader->uniformCount; ++i)
    {
        /* Get uniform. */
        gcUNIFORM uniform = Shader->uniforms[i];

        if(!uniform) continue;

        if (GetUniformUsedArraySize(uniform) == 0)
        {
            SetUniformUsedArraySize(uniform, GetUniformArraySize(uniform));
        }

        switch (GetUniformCategory(uniform)) {
        case gcSHADER_VAR_CATEGORY_NORMAL:
        case gcSHADER_VAR_CATEGORY_LOD_MIN_MAX:
        case gcSHADER_VAR_CATEGORY_LEVEL_BASE_SIZE:
        case gcSHADER_VAR_CATEGORY_SAMPLE_LOCATION:
        case gcSHADER_VAR_CATEGORY_ENABLE_MULTISAMPLE_BUFFERS:
        case gcSHADER_VAR_CATEGORY_WORK_THREAD_COUNT:
        case gcSHADER_VAR_CATEGORY_WORK_GROUP_COUNT:
        case gcSHADER_VAR_CATEGORY_WORK_GROUP_ID_OFFSET:
        case gcSHADER_VAR_CATEGORY_GLOBAL_INVOCATION_ID_OFFSET:
            break;

        case gcSHADER_VAR_CATEGORY_BLOCK_ADDRESS:
            if(isUniformStorageBlockAddress(uniform)) break;
            if(isUniformConstantAddressSpace(uniform)) break;
            if(handleDefaultUBO)
            {
                if(!isUniformUsedInShader(uniform)) continue;
            }
            else if(unblockUniformBlock)
            {
                continue;
            }
            break;

        case gcSHADER_VAR_CATEGORY_BLOCK_MEMBER:
            if(handleDefaultUBO)
            {
                if(!isUniformMovedToDUB(uniform)) continue;
            }
            else if(!unblockUniformBlock) continue;
            break;

        default:
            continue;
        }

        if(gcmType_Kind(uniform->u.type) == gceTK_SAMPLER)
        {
            /* If this texture is not used on shader, we can skip it. */
#if !DX_SHADER
            if (!isUniformUsedInShader(uniform) && !isUniformSamplerCalculateTexSize(uniform) && !isUniformForceActive(uniform))
            {
                continue;
            }
#endif
            samplerCount += GetUniformArraySize(uniform);
        }
    }

    if (ConstantCount)
    {
        *ConstantCount = constantCount;
    }
    if (SamplerCount)
    {
        *SamplerCount = samplerCount;
    }

    return status;
}

gceSTATUS
_MapUniforms(
    IN gcLINKTREE            Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN OUT gcsSL_USAGE_PTR   Usage,
    IN gctSIZE_T             UsageCount,
    IN gctUINT32             UniformBase,
    IN gctSIZE_T             StartIndex,
    IN gcsHINT_PTR           Hints
    )
{
    gceSTATUS status;
    gctINT i, j, nextUniformIndex;
    gctUINT32 vsSamplers = 0, psSamplers = 0;
    gctINT maxSampler = 0, currSampler = 0;
    gctUINT32 samplerCount = 0;
    gctBOOL unblockUniformBlock = gcvFALSE;
    gctBOOL handleDefaultUBO = gcvFALSE;
    /* If chip can support unified uniform, pack vs and ps. */
    gctBOOL packSampler = gcHWCaps.hwFeatureFlags.supportUnifiedSampler;
    gctBOOL useAllSampler = gcHWCaps.hwFeatureFlags.samplerRegFileUnified;
    gctBOOL samplerAllocReversed = gcvFALSE;

    /* Extract the gcSHADER object. */
    gcSHADER shader = Tree->shader;

    gcmASSERT(Hints);

    if (_isHWConstRegisterAllocated(Tree->shader))
    {
        return _VIR_MapUniforms(Tree,
            CodeGen,
            Usage,
            UsageCount,
            StartIndex
            );
    }

    /* Get the sampler size. */
    vsSamplers = gcHWCaps.maxVSSamplerCount;
    psSamplers = gcHWCaps.maxPSSamplerCount;

    if (packSampler)
    {
        currSampler = Hints->unifiedStatus.samplerCount;

        if (Tree->shader->type == gcSHADER_TYPE_FRAGMENT)
        {
            maxSampler = psSamplers;
        }
        else if (Tree->shader->type == gcSHADER_TYPE_VERTEX)
        {
            maxSampler = vsSamplers;
        }
        else
        {
            maxSampler = psSamplers + vsSamplers;
        }
    }
    else if (useAllSampler)
    {
        maxSampler = gcmMIN(gcHWCaps.maxHwNativeTotalSamplerCount, gcHWCaps.maxSamplerCountPerShader);

        /* If a shader stage can use all samplers, for vertex, we need to allocate them in the bottom. */
        if (Tree->shader->type == gcSHADER_TYPE_VERTEX)
        {
            _CalcUniformCount(Tree->shader,
                              unblockUniformBlock,
                              handleDefaultUBO,
                              gcvNULL,
                              &samplerCount);
            /* We need to allocate sampler reversely so in recompiler, all original samplers have the same index. */
            samplerAllocReversed = gcvTRUE;
            currSampler = maxSampler - 1;
        }
        else
        {
            currSampler = 0;
        }
    }
    else
    {
        /* Determine starting sampler index. */
        currSampler = (Tree->shader->type == gcSHADER_TYPE_VERTEX)
                ? psSamplers
                : 0;

        /* Determine maximum sampler index. */
        /* Note that CL kernel can use all samplers if unified. */
        maxSampler = (Tree->shader->type == gcSHADER_TYPE_FRAGMENT)
                   ? psSamplers
                   : psSamplers + vsSamplers;
    }

    if (shader->uniformBlockCount) {
        if(shader->_defaultUniformBlockIndex != -1 ||
           (shader->type == gcSHADER_TYPE_CL && shader->enableDefaultUBO))
        {
            handleDefaultUBO = gcvTRUE;
        }
        else if (!gcHWCaps.hwFeatureFlags.hasHalti1 ||
               (Tree->hints && Tree->hints->uploadedUBO))
        {
            unblockUniformBlock = gcvTRUE;
        }
        else
        {
            unblockUniformBlock = gcvFALSE;
        }
    }

    /* check uniform usage: if a uniform is used in shader or LTC expression */
    gcSHADER_CheckUniformUsage(shader, Tree->flags);

    /* Map all uniforms. */
    for (j = shader->RAHighestPriority; j >=0; j--)
    {
        nextUniformIndex = 0;
        for (i = 0; i < (gctINT)shader->uniformCount; ++i)
        {
            /* Get uniform. */
            gcUNIFORM uniform = shader->uniforms[i];

            if(!uniform) continue;

            if (GetUniformUsedArraySize(uniform) == 0)
            {
                SetUniformUsedArraySize(uniform, GetUniformArraySize(uniform));
            }

            if(uniform->RAPriority != (gctUINT)j) continue;

            switch (GetUniformCategory(uniform)) {
            case gcSHADER_VAR_CATEGORY_NORMAL:
            case gcSHADER_VAR_CATEGORY_LOD_MIN_MAX:
            case gcSHADER_VAR_CATEGORY_LEVEL_BASE_SIZE:
            case gcSHADER_VAR_CATEGORY_SAMPLE_LOCATION:
            case gcSHADER_VAR_CATEGORY_ENABLE_MULTISAMPLE_BUFFERS:
            case gcSHADER_VAR_CATEGORY_WORK_THREAD_COUNT:
            case gcSHADER_VAR_CATEGORY_WORK_GROUP_COUNT:
            case gcSHADER_VAR_CATEGORY_WORK_GROUP_ID_OFFSET:
            case gcSHADER_VAR_CATEGORY_GLOBAL_INVOCATION_ID_OFFSET:
                break;

            case gcSHADER_VAR_CATEGORY_BLOCK_ADDRESS:
                if(isUniformStorageBlockAddress(uniform)) break;
                if(isUniformConstantAddressSpace(uniform)) break;
                if(handleDefaultUBO)
                {
                    if(!isUniformUsedInShader(uniform)) continue;
                }
                else if(unblockUniformBlock)
                {
                    continue;
                }
                break;

            case gcSHADER_VAR_CATEGORY_BLOCK_MEMBER:
                if(handleDefaultUBO)
                {
                    if(!isUniformMovedToDUB(uniform)) continue;
                }
                else if(!unblockUniformBlock) continue;
                break;

            default:
                continue;
            }

            if(gcmType_Kind(uniform->u.type) == gceTK_SAMPLER)
            {
                /* If this texture is not used on shader, we can skip it. */
#if !DX_SHADER
                if (!isUniformUsedInShader(uniform) && !isUniformSamplerCalculateTexSize(uniform) && !isUniformForceActive(uniform))
                {
                    SetUniformPhysical(uniform, -1);
                    SetUniformFlag(uniform, gcvUNIFORM_FLAG_IS_INACTIVE);
                    continue;
                }
#endif
                /* Use next sampler. */
                /* sampler physical index should be the same as
                    the index assigned when adding sampler uniform
                    to Shader, the sampler index is passed as argument
                    to function for its sampler typed parameter
                    */
                if (samplerAllocReversed)
                {
                    uniform->physical = currSampler - GetUniformArraySize(uniform) + 1;
                    currSampler -= GetUniformArraySize(uniform);

                    if (currSampler + 1 < 0)
                    {
                        gcmONERROR(gcvSTATUS_TOO_MANY_UNIFORMS);
                    }
                }
                else
                {
                    uniform->physical = currSampler;
                    currSampler += GetUniformArraySize(uniform);

                    if (currSampler > maxSampler)
                    {
                        gcmONERROR(gcvSTATUS_TOO_MANY_UNIFORMS);
                    }
                }

                ResetUniformFlag(uniform, gcvUNIFORM_FLAG_IS_INACTIVE);
            }
            else
            {
                if (nextUniformIndex > i)
                    continue;
                /*
                ** We need to allocate a register for gl_WorkGroupSize
                ** because HW would read the work group size for a indirect dispatch CS.
                */
                if (uniform->nameLength == 11 &&
                    gcmIS_SUCCESS(gcoOS_StrCmp(uniform->name, "#num_groups")))
                {
                    SetUniformFlag(uniform, gcvUNIFORM_FLAG_USED_IN_SHADER);
                }

                /* set uniform inactive */
                if ((Tree->flags & gcvSHADER_REMOVE_UNUSED_UNIFORMS) &&
                    !isUniformUsedInShader(uniform) &&
                    !isUniformUsedInLTC(uniform) &&
                    !isUniformMovedToDUBO(uniform))
                {
                    if (isUniformSTD140OrShared(uniform))
                        continue;

                    /* If this uniform is used as imageSize. */
                    if (isUniformSamplerCalculateTexSize(uniform))
                    {
                        gcUNIFORM childUniform = gcvNULL;
                        gctBOOL   childUsed = gcvFALSE;

                        gcmASSERT(uniform->firstChild != -1 &&
                                  uniform->firstChild < (gctUINT16)shader->uniformCount);

                        childUniform = shader->uniforms[uniform->firstChild];

                        gcmASSERT(childUniform);

                        while (childUniform)
                        {
                            if (isUniformUsedInShader(childUniform) ||
                                isUniformUsedInLTC(childUniform))
                            {
                                childUsed = gcvTRUE;
                                break;
                            }

                            if (childUniform->nextSibling != -1)
                            {
                                childUniform = shader->uniforms[childUniform->nextSibling];
                            }
                            else
                            {
                                childUniform = gcvNULL;
                            }
                        }

                        if (!childUsed)
                        {
                            SetUniformFlag(uniform, gcvUNIFORM_FLAG_IS_INACTIVE);
                        }
                        else
                        {
                            ResetUniformFlag(uniform, gcvUNIFORM_FLAG_IS_INACTIVE);
                        }
                        continue;
                    }

                    if (Tree->patchID != gcvPATCH_NENAMARK)
                    {
                        /* If a uniform is not used on shader and LTC, we don't need to map it.
                        ** This situation would happen if there is a non-used active UBO on shader.
                        */
                        SetUniformFlag(uniform, gcvUNIFORM_FLAG_IS_INACTIVE);
                        continue;
                    }
                }

                if (!isUniformUsedInShader(uniform) && isUniformUsedInLTC(uniform))
                {
                    /* skip uniforms only used in LTC. */
                    continue;
                }

                /* Update the sampler mask. */
                if (Hints)
                {
                    gctINT j;

                    for (j = currSampler - 1; j >= currSampler - uniform->arraySize; --j)
                    {
                        Hints->usedSamplerMask |= (1 << j);
                    }
                }

                ResetUniformFlag(uniform, gcvUNIFORM_FLAG_IS_INACTIVE);

                gcmONERROR(_MapNonSamplerUniforms(Tree, CodeGen, Usage,
                              UsageCount, i, &nextUniformIndex,
                              isUniformCompiletimeInitialized(uniform),
                              StartIndex));
            }
        }
    }
    shader->RAHighestPriority++;

    if (gcmOPT_DUMP_UNIFORM())
    {
        gctCHAR   buffer[512];
        gctUINT   offset      = 0;

        gcoOS_Print("Uniform Mapping(id:%d): \n", shader->_id);
        for (i = 0; i < (gctINT)shader->uniformCount; ++i)
        {
            /* Get uniform. */
            gcUNIFORM uniform = shader->uniforms[i];

            if(!uniform) continue;

            gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                               "uniform %d name %s physical %d swizzle %d address %d\n", i,
                                uniform->name,
                                uniform->physical,
                                uniform->swizzle,
                                uniform->address);
            gcoOS_Print("%s", buffer);
            offset = 0;
        }
        gcoOS_Print("\n");
    }

    /* Update sampler unified mode. */
    if (packSampler)
    {
        Hints->unifiedStatus.samplerUnifiedMode = gcvUNIFORM_ALLOC_PACK_FLOAT_BASE_OFFSET;
        Hints->unifiedStatus.samplerCount += currSampler;
    }
    else if (useAllSampler)
    {
        Hints->unifiedStatus.samplerUnifiedMode = gcvUNIFORM_ALLOC_PS_TOP_GPIPE_BOTTOM_FLOAT_BASE_OFFSET;

        if (Tree->shader->type == gcSHADER_TYPE_VERTEX)
        {
            gcmASSERT(currSampler == (gctINT)(maxSampler - samplerCount - 1));
            Hints->unifiedStatus.samplerGPipeStart = (maxSampler - samplerCount);
        }
        else
        {
            if (currSampler != 0)
            {
                Hints->unifiedStatus.samplerPSEnd = currSampler - 1;
            }
        }
    }

    /* Success. */
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    return status;
}

void dumpAttributeRegisterAllocation(
    IN gcATTRIBUTE Attribute,
    IN gctINT      Rows,
    IN gctINT      LastUse
    )
{
    if (Rows > 1)
    {
        gcoOS_Print("Attribute(%d) assigned to register r%d - r%d (last use %d)",
            Attribute->index, Attribute->inputIndex,
            Attribute->inputIndex + Rows - 1, LastUse);
    }
    else
    {
        gcoOS_Print("Attribute(%d) assigned to register r%d (last use %d)",
                    Attribute->index, Attribute->inputIndex, LastUse);
    }
}

gceSTATUS
_MapAttributesRAEnabled(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN OUT gcsSL_USAGE_PTR Usage,
    IN OUT gcsHINT_PTR  Hints
    )
{
    gctSIZE_T i;
    gceSTATUS status;

    /* Extract the gcSHADER object. */
    gcSHADER shader = Tree->shader;

    do
    {
        /* Process all attributes. */
        for (i = 0; i < shader->attributeCount; ++i)
        {
            /* Only process enabled attributes. */
            if (Tree->attributeArray[i].inUse)
            {
                /* Get attribute. */
                gcATTRIBUTE attribute = shader->attributes[i];

                /* set the attribute always used so the recompilation wouldn't
                 * remove this attribute later if the dynamic patched code
                 * doesn't use the attribute after recompilation, since recompile
                 * should not change attribute mapping, otherwise wrong data
                 * would be mapped to attributes */
                gcmATTRIBUTE_SetAlwaysUsed(attribute, gcvTRUE);

                if (shader->type == gcSHADER_TYPE_FRAGMENT && Hints &&
                    attribute && gcmATTRIBUTE_isCentroid(attribute))
                {
                    Hints->hasCentroidInput = gcvTRUE;
                }

                /* Check for special POSITION attribute. */
                if (attribute->nameLength == gcSL_POSITION)
                {
                    gctINT j;

                    gcmASSERT(CodeGen->shaderType == gcSHADER_TYPE_FRAGMENT);

                    gcmASSERT(attribute->inputIndex == 0);
                    CodeGen->usePosition  =
                        (CodeGen->flags & gcvSHADER_USE_GL_POSITION);
                    CodeGen->positionIndex = i;

                    gcmASSERT(Hints != gcvNULL);
                    for (j = 0; j < 4; j++)
                    {
                        Hints->useFragCoord[j] = (gctCHAR)_IsChannelUsedForAttribute(Tree,
                                                                            &Tree->attributeArray[i],
                                                                            attribute->index,
                                                                            gcmComposeSwizzle(j, j, j, j));
                    }
                    continue;
                }

                /* Check for special FRONT_FACING attribute. */
                if (attribute->nameLength == gcSL_FRONT_FACING)
                {
                    gcmASSERT(CodeGen->shaderType == gcSHADER_TYPE_FRAGMENT);

                    gcmASSERT(attribute->inputIndex == 0);
                    CodeGen->useFace      =
                        (CodeGen->flags & gcvSHADER_USE_GL_FACE);

                    gcmASSERT(Hints != gcvNULL);
                    Hints->useFrontFacing = gcvTRUE;
                    continue;
                }

                /* Check for special SAMPLE_POSITION attribute. */
                if (attribute->nameLength == gcSL_SAMPLE_POSITION)
                {
                    gcmASSERT(CodeGen->shaderType == gcSHADER_TYPE_FRAGMENT);

                    gcmASSERT(Hints != gcvNULL);
                    Hints->useSamplePosition = gcvTRUE;
                    continue;
                }

                if (attribute->nameLength == gcSL_POINT_COORD)
                {
                    gctINT j;
                    CodeGen->usePointCoord      =
                        (CodeGen->flags & gcvSHADER_USE_GL_POINT_COORD);
                    CodeGen->pointCoordPhysical = attribute->inputIndex;

                    gcmASSERT(Hints != gcvNULL);
                    for (j = 0; j < 4; j++)
                    {
                        Hints->usePointCoord[j] = (gctCHAR)_IsChannelUsedForAttribute(Tree,
                                                                            &Tree->attributeArray[i],
                                                                            attribute->index,
                                                                            gcmComposeSwizzle(j, j, j, j));
                    }
                }
            }
        }

        /* Success. */
        status = gcvSTATUS_OK;
    }
    while (gcvFALSE);

    return status;
}

gctBOOL
_needAddDummyAttribute(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen
    )
{
    if (CodeGen->shaderType == gcSHADER_TYPE_VERTEX &&
        !gcHWCaps.hwFeatureFlags.supportZeroAttrsInFE &&
        Tree->shader->attributeCount == 0)
    {
        return gcvTRUE;
    }
    else
        return gcvFALSE;
}

gceSTATUS
_MapAttributes(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN OUT gcsSL_USAGE_PTR Usage,
    OUT gctINT *RegCount,
    IN OUT gcsHINT_PTR  Hints
    )
{
    gctINT reg;
    gctSIZE_T i;
    gceSTATUS status;

    /* Extract the gcSHADER object. */
    gcSHADER shader = Tree->shader;

    do
    {
        if (CodeGen->shaderType != gcSHADER_TYPE_FRAGMENT)
        {
            /* Start at register 0 for vertex shaders. */
            if (_needAddDummyAttribute(Tree, CodeGen))
            {
                /* some chip don't support 0 attribute, so driver needs to
                 * fake a dummy input */
                reg = 1;
            }
            else
            {
                reg = 0;
            }
        }
        else
        {
            /* Start at register 1 for fragment shaders. */
            reg = 1;

            /* Mark register 0 as used (position). */
            Usage[0].lastUse[0] =
            Usage[0].lastUse[1] =
            Usage[0].lastUse[2] =
            Usage[0].lastUse[3] = gcvSL_RESERVED;
        }

        /* Process all attributes. */
        for (i = 0; i < shader->attributeCount; ++i)
        {
            /* Only process enabled attributes. */
            if (Tree->attributeArray[i].inUse)
            {
                /* Get attribute. */
                gcATTRIBUTE attribute = shader->attributes[i];
                gctUINT32 components = 0, rows = 0;
                gctUINT8 enable = 0;

                /* set the attribute always used so the recompilation wouldn't
                 * remove this attribute later if the dynamic patched code
                 * doesn't use the attribute after recompilation, since recompile
                 * should not change attribute mapping, otherwise wrong data
                 * would be mapped to attributes */
                gcmATTRIBUTE_SetAlwaysUsed(attribute, gcvTRUE);

                if (shader->type == gcSHADER_TYPE_FRAGMENT && Hints &&
                    attribute && gcmATTRIBUTE_isCentroid(attribute))
                {
                    Hints->hasCentroidInput = gcvTRUE;
                }

                if (attribute->nameLength == gcSL_HELPER_INVOCATION)
                {
                    attribute->inputIndex = 0;
                    continue;
                }

                /* Check for special POSITION attribute. */
                if (attribute->nameLength == gcSL_POSITION)
                {
                    gctINT j;
                    gcmASSERT(CodeGen->shaderType == gcSHADER_TYPE_FRAGMENT);

                    attribute->inputIndex = 0;
                    CodeGen->usePosition  =
                        (CodeGen->flags & gcvSHADER_USE_GL_POSITION);
                    CodeGen->positionIndex = i;

                    gcmASSERT(Hints != gcvNULL);
                    for (j = 0; j < 4; j++)
                    {
                        Hints->useFragCoord[j] = (gctCHAR)_IsChannelUsedForAttribute(Tree,
                                                                            &Tree->attributeArray[i],
                                                                            attribute->index,
                                                                            gcmComposeSwizzle(j, j, j, j));
                    }
                    continue;
                }

                /* Check for special SAMPLE_POSITION attribute. */
                if (attribute->nameLength == gcSL_SAMPLE_POSITION)
                {
                    gcmASSERT(CodeGen->shaderType == gcSHADER_TYPE_FRAGMENT);

                    gcmASSERT(Hints != gcvNULL);
                    Hints->useSamplePosition = gcvTRUE;
                    continue;
                }

                /* Check for special FRONT_FACING attribute. */
                if (attribute->nameLength == gcSL_FRONT_FACING)
                {
                    gcmASSERT(CodeGen->shaderType == gcSHADER_TYPE_FRAGMENT);

                    attribute->inputIndex = 0;
                    CodeGen->useFace      =
                        (CodeGen->flags & gcvSHADER_USE_GL_FACE);
                    gcmASSERT(Hints != gcvNULL);
                    Hints->useFrontFacing = gcvTRUE;
                    continue;
                }

                /* Determine rows and components. */
                gcTYPE_GetTypeInfo(attribute->type, &components, &rows, 0);
                rows *= attribute->arraySize;
                if (CodeGen->shaderType == gcSHADER_TYPE_VERTEX)
                {
                    /* Reserve all components for vertex shaders. */
                    enable = 0xF;
                }
                else
                {
                    /* Get the proper component enable bits. */
                    switch (components)
                    {
                    case 1:
                        enable = gcSL_ENABLE_X;
                        break;

                    case 2:
                        enable = gcSL_ENABLE_XY;
                        break;

                    case 3:
                        enable = gcSL_ENABLE_XYZ;
                        break;

                    case 4:
                        enable = gcSL_ENABLE_XYZW;
                        break;

                    default:
                        break;
                    }
                }
                if (gcmATTRIBUTE_hasAlias(attribute))
                {
                    gctUINT k;
                    gcATTRIBUTE aliseAttribute = gcvNULL;

                    /* find the aliased to attribute and use the assigned register and enable */
                    for (k = 0; k < i; k++)
                    {
                        if (attribute->location == shader->attributes[k]->location &&
                            gcmATTRIBUTE_isRegAllocated(shader->attributes[k]))
                        {
                            aliseAttribute = shader->attributes[k];
                        }
                    }
                    if (aliseAttribute)
                    {
                        attribute->inputIndex = aliseAttribute->inputIndex;
                        /* Set register usage. */
                        _SetRegisterUsage(Usage + attribute->inputIndex,
                                  rows,
                                  enable,
                                  Tree->attributeArray[i].lastUse);

                        if (gcSHADER_DumpCodeGenVerbose(Tree->shader))
                            dumpAttributeRegisterAllocation(attribute, rows,
                                                            Tree->attributeArray[i].lastUse);
                        continue;
                    }
                }

                /* Assign input register. */
                attribute->inputIndex = reg;
                gcmATTRIBUTE_SetRegAllocated(attribute, gcvTRUE);

                if (CodeGen->shaderType != gcSHADER_TYPE_VERTEX)
                {
                    if (attribute->nameLength == gcSL_POINT_COORD)
                    {
                        gctINT j;
                        CodeGen->usePointCoord      =
                            (CodeGen->flags & gcvSHADER_USE_GL_POINT_COORD);
                        CodeGen->pointCoordPhysical = reg;

                        gcmASSERT(Hints != gcvNULL);
                        for (j = 0; j < 4; j++)
                        {
                            Hints->usePointCoord[j] = (gctCHAR)_IsChannelUsedForAttribute(Tree,
                                                                                &Tree->attributeArray[i],
                                                                                attribute->index,
                                                                                gcmComposeSwizzle(j, j, j, j));
                        }

                    }
                }

                /* Set register usage. */
                _SetRegisterUsage(Usage + reg,
                            rows,
                            enable,
                            Tree->attributeArray[i].lastUse);

                if (gcSHADER_DumpCodeGenVerbose(Tree->shader))
                    dumpAttributeRegisterAllocation(attribute, rows,
                                                    Tree->attributeArray[i].lastUse);

                /* Move to next register. */
                reg += rows;

            }
        }

        if (CodeGen->clShader && ! CodeGen->hasBugFixes10)
        {
            gcmASSERT(reg <= 3);
            CodeGen->reservedRegForLoad = reg;
            CodeGen->loadDestIndex = -1;
            CodeGen->origAssigned = -1;
            CodeGen->lastLoadUser = -1;

            /* Mark register 3 as used (position). */
            Usage[reg].lastUse[0] =
            Usage[reg].lastUse[1] =
            Usage[reg].lastUse[2] =
            Usage[reg].lastUse[3] = gcvSL_RESERVED;
        }
        else
        {
            CodeGen->reservedRegForLoad = ~0U;
            CodeGen->loadDestIndex = -1;
            CodeGen->origAssigned = -1;
            CodeGen->lastLoadUser = -1;
        }

        /* Success. */
        status = gcvSTATUS_OK;
    }
    while (gcvFALSE);

    *RegCount = reg;
    /* Return the status. */
    return status;
}

gceSTATUS
_MapAttributesDual16RAEnabled(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN OUT gcsSL_USAGE_PTR Usage,
    IN OUT gcsHINT_PTR  Hints
    )
{
    gctSIZE_T i;
    gceSTATUS status;
    gctBOOL skipHighp = gcvFALSE;

    /* Extract the gcSHADER object. */
    gcSHADER shader = Tree->shader;

    do
    {
        /* Process all attributes . */
        for (i = 0; i < shader->attributeCount; ++i)
        {
            /* Only process enabled attributes. */
            if (Tree->attributeArray[i].inUse)
            {
                /* Get attribute. */
                gcATTRIBUTE attribute = shader->attributes[i];

                if(skipHighp && attribute->precision == gcSHADER_PRECISION_HIGH) continue;

                /* set the attribute always used so the recompilation wouldn't
                 * remove this attribute later if the dynamic patched code
                 * doesn't use the attribute after recompilation, since recompile
                 * should not change attribute mapping, otherwise wrong data
                 * would be mapped to attributes */
                gcmATTRIBUTE_SetAlwaysUsed(attribute, gcvTRUE);

                if (shader->type == gcSHADER_TYPE_FRAGMENT && Hints &&
                    attribute && gcmATTRIBUTE_isCentroid(attribute))
                {
                    Hints->hasCentroidInput = gcvTRUE;
                }

                /* Check for special POSITION attribute. */
                if (attribute->nameLength == gcSL_POSITION)
                {
                    gctINT j;
                    gcmASSERT(CodeGen->shaderType == gcSHADER_TYPE_FRAGMENT);

                    gcmASSERT(attribute->inputIndex == 0);
                    CodeGen->usePosition  =
                        (CodeGen->flags & gcvSHADER_USE_GL_POSITION);
                    CodeGen->positionIndex = i;

                    gcmASSERT(Hints != gcvNULL);
                    for (j = 0; j < 4; j++)
                    {
                        Hints->useFragCoord[j] = (gctCHAR)_IsChannelUsedForAttribute(Tree,
                                                                            &Tree->attributeArray[i],
                                                                            attribute->index,
                                                                            gcmComposeSwizzle(j, j, j, j));
                    }
                    continue;
                }

                /* Check for special SAMPLE_POSITION attribute. */
                if (attribute->nameLength == gcSL_SAMPLE_POSITION)
                {
                    gcmASSERT(CodeGen->shaderType == gcSHADER_TYPE_FRAGMENT);

                    gcmASSERT(Hints != gcvNULL);
                    Hints->useSamplePosition = gcvTRUE;
                    continue;
                }

                /* Check for special FRONT_FACING attribute. */
                if (attribute->nameLength == gcSL_FRONT_FACING)
                {
                    gcmASSERT(CodeGen->shaderType == gcSHADER_TYPE_FRAGMENT);

                    gcmASSERT(attribute->inputIndex == 0);
                    CodeGen->useFace      =
                        (CodeGen->flags & gcvSHADER_USE_GL_FACE);
                    gcmASSERT(Hints != gcvNULL);
                    Hints->useFrontFacing = gcvTRUE;
                    continue;
                }

                if (attribute->nameLength == gcSL_POINT_COORD)
                {
                    gctINT j;
                    CodeGen->usePointCoord      =
                        (CodeGen->flags & gcvSHADER_USE_GL_POINT_COORD);
                    CodeGen->pointCoordPhysical = attribute->inputIndex;

                    gcmASSERT(Hints != gcvNULL);
                    for (j = 0; j < 4; j++)
                    {
                        Hints->usePointCoord[j] = (gctCHAR)_IsChannelUsedForAttribute(Tree,
                                                                            &Tree->attributeArray[i],
                                                                            attribute->index,
                                                                            gcmComposeSwizzle(j, j, j, j));
                    }

                }
            }
        }

        /* Success. */
        status = gcvSTATUS_OK;
    }
    while (gcvFALSE);

    /* Return the status. */
    return status;
}

gceSTATUS
_MapAttributesDual16(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN OUT gcsSL_USAGE_PTR Usage,
    OUT gctINT *RegCount,
    IN OUT gcsHINT_PTR  Hints
    )
{
    gctINT reg;
    gctSIZE_T i;
    gceSTATUS status;
    gctBOOL skipHighp = gcvFALSE;

    /* Extract the gcSHADER object. */
    gcSHADER shader = Tree->shader;

    do
    {
        if (CodeGen->shaderType != gcSHADER_TYPE_FRAGMENT)
        {
            /* Start at register 0 for vertex shaders. */
            reg = 0;
        }
        else
        {
            /* Start at register 2 for fragment shaders. */
            reg = 2;

            /* Mark register 0, 1 as used (position). */
            Usage[0].lastUse[0] =
            Usage[0].lastUse[1] =
            Usage[0].lastUse[2] =
            Usage[0].lastUse[3] = gcvSL_RESERVED;

            Usage[1].lastUse[0] =
            Usage[1].lastUse[1] =
            Usage[1].lastUse[2] =
            Usage[1].lastUse[3] = gcvSL_RESERVED;

#if (GC_ENABLE_DUAL_FP16_PHASE > 1)
            /* Process all highp attributes first as those need two registers each . */
            for (i = 0; i < shader->attributeCount; ++i)
            {
                /* Only process enabled attributes. */
                if (Tree->attributeArray[i].inUse)
                {
                    /* Get attribute. */
                    gcATTRIBUTE attribute = shader->attributes[i];
                    gctUINT32 components = 0, rows = 0;
                    gctUINT8 enable = 0;

                    if(attribute->precision != gcSHADER_PRECISION_HIGH) continue;

                    /* set the attribute always used so the recompilation wouldn't
                     * remove this attribute later if the dynamic patched code
                     * doesn't use the attribute after recompilation, since recompile
                     * should not change attribute mapping, otherwise wrong data
                     * would be mapped to attributes */
                    gcmATTRIBUTE_SetAlwaysUsed(attribute, gcvTRUE);

                    if (attribute->nameLength == gcSL_HELPER_INVOCATION)
                    {
                        attribute->inputIndex = 0;
                        continue;
                    }

                    /* Check for special POSITION attribute. */
                    if (attribute->nameLength == gcSL_POSITION)
                    {
                        gctINT j;
                        gcmASSERT(CodeGen->shaderType == gcSHADER_TYPE_FRAGMENT);

                        attribute->inputIndex = 0;
                        CodeGen->usePosition  =
                            (CodeGen->flags & gcvSHADER_USE_GL_POSITION);
                        CodeGen->positionIndex = i;

                        gcmASSERT(Hints != gcvNULL);
                        for (j = 0; j < 4; j++)
                        {
                            Hints->useFragCoord[j] = _IsChannelUsedForAttribute(Tree,
                                                                                &Tree->attributeArray[i],
                                                                                attribute->index,
                                                                                gcmComposeSwizzle(j, j, j, j));
                        }
                        continue;
                    }

                    /* Check for special SAMPLE_POSITION attribute. */
                    if (attribute->nameLength == gcSL_SAMPLE_POSITION)
                    {
                        gcmASSERT(CodeGen->shaderType == gcSHADER_TYPE_FRAGMENT);

                        gcmASSERT(Hints != gcvNULL);
                        Hints->useSamplePosition = gcvTRUE;
                        continue;
                    }

                    /* Check for special FRONT_FACING attribute. */
                    if (attribute->nameLength == gcSL_FRONT_FACING)
                    {
                        gcmASSERT(CodeGen->shaderType == gcSHADER_TYPE_FRAGMENT);

                        attribute->inputIndex = 0;
                        CodeGen->useFace      =
                            (CodeGen->flags & gcvSHADER_USE_GL_FACE);
                        gcmASSERT(Hints != gcvNULL);
                        Hints->useFrontFacing = gcvTRUE;
                        continue;
                    }

                    /* Assign input register. */
                    attribute->inputIndex = reg;

                    /* Determine rows and components. */
                    gcTYPE_GetTypeInfo(attribute->type, &components, &rows, 0);
                    rows *= attribute->arraySize;

                    if (CodeGen->shaderType == gcSHADER_TYPE_VERTEX)
                    {
                        /* Reserve all components for vertex shaders. */
                        enable = 0xF;
                    }
                    else
                    {
                        /* Get the proper component enable bits. */
                        switch (components)
                        {
                        case 1:
                            enable = gcSL_ENABLE_X;
                            break;

                        case 2:
                            enable = gcSL_ENABLE_XY;
                            break;

                        case 3:
                            enable = gcSL_ENABLE_XYZ;
                            break;

                        case 4:
                            enable = gcSL_ENABLE_XYZW;
                            break;

                        default:
                            break;
                        }

                        if (attribute->nameLength == gcSL_POINT_COORD)
                        {
                            gctINT j;
                            CodeGen->usePointCoord      =
                                (CodeGen->flags & gcvSHADER_USE_GL_POINT_COORD);
                            CodeGen->pointCoordPhysical = reg;

                            gcmASSERT(Hints != gcvNULL);
                            for (j = 0; j < 4; j++)
                            {
                                Hints->usePointCoord[j] = _IsChannelUsedForAttribute(Tree,
                                                                                    &Tree->attributeArray[i],
                                                                                    attribute->index,
                                                                                    gcmComposeSwizzle(j, j, j, j));
                            }
                        }
                    }

                    /* Set register usage. */
                    _SetRegisterUsage(Usage + reg,
                              rows,
                              enable,
                              Tree->attributeArray[i].lastUse);

                    reg += rows;
                    _SetRegisterUsage(Usage + reg,
                              rows,
                              enable,
                              Tree->attributeArray[i].lastUse);

                    if (gcSHADER_DumpCodeGenVerbose(Tree->shader->_id))
                        dumpAttributeRegisterAllocation(attribute, rows,
                                                        Tree->attributeArray[i].lastUse);

                    /* Move to next register. */
                    reg += rows;
                }
            }
            skipHighp = gcvTRUE;
#endif
        }

        /* Process all attributes . */
        for (i = 0; i < shader->attributeCount; ++i)
        {
            /* Only process enabled attributes. */
            if (Tree->attributeArray[i].inUse)
            {
                /* Get attribute. */
                gcATTRIBUTE attribute = shader->attributes[i];
                gctUINT32 components = 0, rows = 0;
                gctUINT8 enable = 0;

                if(skipHighp && attribute->precision == gcSHADER_PRECISION_HIGH) continue;

                /* set the attribute always used so the recompilation wouldn't
                 * remove this attribute later if the dynamic patched code
                 * doesn't use the attribute after recompilation, since recompile
                 * should not change attribute mapping, otherwise wrong data
                 * would be mapped to attributes */
                gcmATTRIBUTE_SetAlwaysUsed(attribute, gcvTRUE);

                if (shader->type == gcSHADER_TYPE_FRAGMENT && Hints &&
                    attribute && gcmATTRIBUTE_isCentroid(attribute))
                {
                    Hints->hasCentroidInput = gcvTRUE;
                }

                if (attribute->nameLength == gcSL_HELPER_INVOCATION)
                {
                    attribute->inputIndex = 0;
                    continue;
                }

                /* Check for special POSITION attribute. */
                if (attribute->nameLength == gcSL_POSITION)
                {
                    gctINT j;
                    gcmASSERT(CodeGen->shaderType == gcSHADER_TYPE_FRAGMENT);

                    attribute->inputIndex = 0;
                    CodeGen->usePosition  =
                        (CodeGen->flags & gcvSHADER_USE_GL_POSITION);
                    CodeGen->positionIndex = i;

                    gcmASSERT(Hints != gcvNULL);
                    for (j = 0; j < 4; j++)
                    {
                        Hints->useFragCoord[j] = (gctCHAR)_IsChannelUsedForAttribute(Tree,
                                                                            &Tree->attributeArray[i],
                                                                            attribute->index,
                                                                            gcmComposeSwizzle(j, j, j, j));
                    }
                    continue;
                }

                /* Check for special SAMPLE_POSITION attribute. */
                if (attribute->nameLength == gcSL_SAMPLE_POSITION)
                {
                    gcmASSERT(CodeGen->shaderType == gcSHADER_TYPE_FRAGMENT);

                    gcmASSERT(Hints != gcvNULL);
                    Hints->useSamplePosition = gcvTRUE;
                    continue;
                }

                /* Check for special FRONT_FACING attribute. */
                if (attribute->nameLength == gcSL_FRONT_FACING)
                {
                    gcmASSERT(CodeGen->shaderType == gcSHADER_TYPE_FRAGMENT);

                    attribute->inputIndex = 0;
                    CodeGen->useFace      =
                        (CodeGen->flags & gcvSHADER_USE_GL_FACE);
                    gcmASSERT(Hints != gcvNULL);
                    Hints->useFrontFacing = gcvTRUE;
                    continue;
                }

                /* Assign input register. */
                attribute->inputIndex = reg;

                /* Determine rows and components. */
                gcTYPE_GetTypeInfo(attribute->type, &components, &rows, 0);
                rows *= attribute->arraySize;

                if (CodeGen->shaderType == gcSHADER_TYPE_VERTEX)
                {
                    /* Reserve all components for vertex shaders. */
                    enable = 0xF;
                }
                else
                {
                    /* Get the proper component enable bits. */
                    switch (components)
                    {
                    case 1:
                        enable = gcSL_ENABLE_X;
                        break;

                    case 2:
                        enable = gcSL_ENABLE_XY;
                        break;

                    case 3:
                        enable = gcSL_ENABLE_XYZ;
                        break;

                    case 4:
                        enable = gcSL_ENABLE_XYZW;
                        break;

                    default:
                        break;
                    }

                    if (attribute->nameLength == gcSL_POINT_COORD)
                    {
                        gctINT j;
                        CodeGen->usePointCoord      =
                            (CodeGen->flags & gcvSHADER_USE_GL_POINT_COORD);
                        CodeGen->pointCoordPhysical = reg;

                        gcmASSERT(Hints != gcvNULL);
                        for (j = 0; j < 4; j++)
                        {
                            Hints->usePointCoord[j] = (gctCHAR)_IsChannelUsedForAttribute(Tree,
                                                                                &Tree->attributeArray[i],
                                                                                attribute->index,
                                                                                gcmComposeSwizzle(j, j, j, j));
                        }

                    }
                }

                /* Set register usage. */
                _SetRegisterUsage(Usage + reg,
                          rows,
                          enable,
                          Tree->attributeArray[i].lastUse);

                if (gcSHADER_DumpCodeGenVerbose(Tree->shader))
                    dumpAttributeRegisterAllocation(attribute, rows,
                                                    Tree->attributeArray[i].lastUse);

                /* Move to next register. */
                reg += rows;
            }
        }

        if (CodeGen->clShader && ! CodeGen->hasBugFixes10)
        {
            gcmASSERT(reg <= 3);
            CodeGen->reservedRegForLoad = reg;
            CodeGen->loadDestIndex = -1;
            CodeGen->origAssigned = -1;
            CodeGen->lastLoadUser = -1;

            /* Mark register 3 as used (position). */
            Usage[reg].lastUse[0] =
            Usage[reg].lastUse[1] =
            Usage[reg].lastUse[2] =
            Usage[reg].lastUse[3] = gcvSL_RESERVED;
        }
        else
        {
            CodeGen->reservedRegForLoad = ~0U;
            CodeGen->loadDestIndex = -1;
            CodeGen->origAssigned = -1;
            CodeGen->lastLoadUser = -1;
        }

        /* Success. */
        status = gcvSTATUS_OK;
    }
    while (gcvFALSE);

    *RegCount = reg;

    /* Return the status. */
    return status;
}

gceSTATUS
_MapFragmentOutputs(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN OUT gcsSL_USAGE_PTR Usage
    )
{
    gctSIZE_T i;
    gcOUTPUT output;
    gctUINT index;
    gcLINKTREE_TEMP temp;
    gceSTATUS status = gcvSTATUS_OK;

    /* Extract the gcSHADER object. */
    gcSHADER shader = Tree->shader;

    do
    {
        if (CodeGen->shaderType != gcSHADER_TYPE_FRAGMENT) break;

        /* Process all outputs. */
        for (i = 0; i < shader->outputCount; ++i)
        {
            /* Get output. */
            output = shader->outputs[i];
            if(!output) continue;
            if (output->nameLength == gcSL_DEPTH) /* Check for special DEPTH output. */
            {
                index = Tree->outputArray[i].tempHolding;
                temp = &Tree->tempArray[index];
                if(temp->assigned == -1) { /* not yet assigned */
                   temp->assigned = 0;
                   temp->shift   = 2;
                   temp->swizzle = gcSL_SWIZZLE_ZZZZ;

                    if (gcSHADER_DumpCodeGenVerbose(shader))
                        dumpRegisterAllocation(temp);
                }
            }
            if (output->nameLength == gcSL_SUBSAMPLE_DEPTH) /* Check for special subsampleDepth output. */
            {
                index = Tree->outputArray[i].tempHolding;
                temp = &Tree->tempArray[index];
                if (temp->assigned == -1)
                {
                    /* Use a special register index(-128) for subSampleDepth, replace it with the last register after RA. */
                    temp->assigned = -128;
                    temp->shift   = 0;
                    temp->swizzle = gcSL_SWIZZLE_XYZW;

                    CodeGen->subsampleDepthRegIncluded = gcvTRUE;
                    CodeGen->subsampleDepthIndex       = index;
                    CodeGen->subsampleDepthPhysical    = temp->assigned; /* temporary setting */
                    if (gcSHADER_DumpCodeGenVerbose(shader))
                    {
                        dumpRegisterAllocation(temp);
                    }
                }
             }
         }
    }
    while (gcvFALSE);

    /* Return the status. */
    return status;
}

extern gctUINT8
_Enable2Swizzle(
    IN gctUINT32 Enable
    );

extern gctUINT16
_SelectSwizzle(
    IN gctUINT16 Swizzle,
    IN gctSOURCE_t Source
    );

gceSTATUS
_AddConstantVec1(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctFLOAT Constant,
    OUT gctINT * Index,
    OUT gctUINT8 * Swizzle,
    OUT gcSL_TYPE *Type
    )
{
    gcuFLOAT_UINT32 constants[4];
    constants[0].f = Constant;

    return _AllocateConst(Tree,
                          CodeGen,
                          gcSL_ENABLE_X,
                          constants,
                          gcvFALSE,
                          gcvTRUE,
                          Index,
                          Swizzle,
                          gcvNULL,
                          Type);
}

gceSTATUS
_AddConstantVec2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctFLOAT Constant1,
    IN gctFLOAT Constant2,
    OUT gctINT * Index,
    OUT gctUINT8 * Swizzle,
    OUT gcSL_TYPE *Type
    )
{
    gcuFLOAT_UINT32 constants[4];
    constants[0].f = Constant1;
    constants[1].f = Constant2;

    return _AllocateConst(Tree,
                          CodeGen,
                          gcSL_ENABLE_X | gcSL_ENABLE_Y,
                          constants,
                          gcvFALSE,
                          gcvTRUE,
                          Index,
                          Swizzle,
                          gcvNULL,
                          Type);
}

gceSTATUS
_AddConstantVec3(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctFLOAT Constant1,
    IN gctFLOAT Constant2,
    IN gctFLOAT Constant3,
    OUT gctINT * Index,
    OUT gctUINT8 * Swizzle,
    OUT gcSL_TYPE *Type
    )
{
    gcuFLOAT_UINT32 constants[4];
    constants[0].f = Constant1;
    constants[1].f = Constant2;
    constants[2].f = Constant3;

    return _AllocateConst(Tree,
                          CodeGen,
                          gcSL_ENABLE_X | gcSL_ENABLE_Y | gcSL_ENABLE_Z,
                          constants,
                          gcvFALSE,
                          gcvTRUE,
                          Index,
                          Swizzle,
                          gcvNULL,
                          Type);
}

gceSTATUS
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
    )
{
    gcuFLOAT_UINT32 constants[4];
    constants[0].f = Constant1;
    constants[1].f = Constant2;
    constants[2].f = Constant3;
    constants[3].f = Constant4;

    return _AllocateConst(Tree,
                          CodeGen,
                          gcSL_ENABLE_XYZW,
                          constants,
                          gcvFALSE,
                          gcvTRUE,
                          Index,
                          Swizzle,
                          gcvNULL,
                          Type);
}

gceSTATUS
_AddConstantIVec1(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctINT Constant,
    OUT gctINT * Index,
    OUT gctUINT8 * Swizzle,
    OUT gcSL_TYPE *Type
    )
{
    gctINT constants[4];
    constants[0] = Constant;

    return _AllocateConst(Tree,
                          CodeGen,
                          gcSL_ENABLE_X,
                          (gcuFLOAT_UINT32 *) constants,
                          gcvFALSE,
                          gcvTRUE,
                          Index,
                          Swizzle,
                          gcvNULL,
                          Type);
}

gceSTATUS
_AddConstantIVec2(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctUINT Constant1,
    IN gctUINT Constant2,
    OUT gctINT * Index,
    OUT gctUINT8 * Swizzle,
    OUT gcSL_TYPE *Type
    )
{
    gcuFLOAT_UINT32 constants[4];
    constants[0].u = Constant1;
    constants[1].u = Constant2;

    return _AllocateConst(Tree,
                          CodeGen,
                          gcSL_ENABLE_XY,
                          constants,
                          gcvFALSE,
                          gcvTRUE,
                          Index,
                          Swizzle,
                          gcvNULL,
                          Type);
}

gceSTATUS
_AddConstantIVec3(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctUINT Constant1,
    IN gctUINT Constant2,
    IN gctUINT Constant3,
    OUT gctINT * Index,
    OUT gctUINT8 * Swizzle,
    OUT gcSL_TYPE *Type
    )
{
    gcuFLOAT_UINT32 constants[4];
    constants[0].u = Constant1;
    constants[1].u = Constant2;
    constants[2].u = Constant3;

    return _AllocateConst(Tree,
                          CodeGen,
                          gcSL_ENABLE_XYZ,
                          constants,
                          gcvFALSE,
                          gcvTRUE,
                          Index,
                          Swizzle,
                          gcvNULL,
                          Type);
}

gceSTATUS
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
    )
{
    gcuFLOAT_UINT32 constants[4];
    constants[0].u = Constant1;
    constants[1].u = Constant2;
    constants[2].u = Constant3;
    constants[3].u = Constant4;

    return _AllocateConst(Tree,
                          CodeGen,
                          gcSL_ENABLE_XYZW,
                          constants,
                          gcvFALSE,
                          gcvTRUE,
                          Index,
                          Swizzle,
                          gcvNULL,
                          Type);
}
static void
_SetOpcode(
    IN OUT gctUINT32 States[4],
    IN gctUINT32 Opcode,
    IN gctUINT32 Condition,
    IN gctBOOL Saturate
    )
{
    /* Set opcode. */
    States[0] |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (Opcode & 0x3F) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)))
              |  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ?
 10:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:6) - (0 ?
 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) ((gctUINT32) (Condition) & ((gctUINT32) ((((1 ?
 10:6) - (0 ?
 10:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)))
              |  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (Saturate) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));
    States[2] |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (Opcode >> 6) & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)));
}

static gceSTATUS
_AllocateRegisterForTemp(
    IN gcLINKTREE              Tree,
    IN gcsCODE_GENERATOR_PTR   CodeGen,
    IN gcLINKTREE_TEMP         Temp
    )
{
    gcSHADER_TYPE type;
    gctUINT count = 1, i;
    gceSTATUS status;

    /* Leave register allocation at the lower half as the upper half register is allocated
       at the same time.
    */
    if(Temp->isPaired64BitUpper) return gcvSTATUS_OK;

    /* Convert usage into type. */
    type = _Usage2Type(Temp->usage);

    /* We need refine it to make more flexible for RA for array or matrix that are not indexed access*/
    /* In that case, we can regard them as separated registers so to get better HW register usage*/
    if (Temp->variable != gcvNULL)
    {
        if (Temp->isIndexing && (Temp->variable->parent != -1))
        {
            gcLINKTREE_TEMP thisTemp;
            gctUINT8 maxUsage = 0;
            gctUINT startIndex, endIndex, adjustedStartIndex = 0xffffffff;
            gcSHADER_GetVariableIndexingRange(Tree->shader, Temp->variable, gcvTRUE,
                                              &startIndex, &endIndex);

            for (i = startIndex; i < endIndex; i ++)
            {
                thisTemp = Tree->tempArray + i;
                if (maxUsage < thisTemp->usage)
                    maxUsage = thisTemp->usage;

                if ((thisTemp->assigned == -1) && (adjustedStartIndex == 0xffffffff))
                    adjustedStartIndex = i;
            }

            if (adjustedStartIndex != 0xffffffff)
            {
                count = endIndex - adjustedStartIndex;

                /* Convert usage into type. */
                type = _Usage2Type(maxUsage);

                Temp = Tree->tempArray + adjustedStartIndex;
            }
            else
            {
                count = 0;
            }
#if _SUPPORT_LONG_ULONG_DATA_TYPE
            if (isFormat64bit(Temp->format))
            {
                count *= 2;
            }
#endif
        }
        else if (Temp->isIndexing && ((Temp->variable->arrayLengthCount > 0) || gcmType_isMatrix(Temp->variable->u.type)))
        {
            gcVARIABLE variable = Temp->variable;
            gctINT32 tempIndex = (gctINT32)(Temp - Tree->tempArray);
            gctUINT components, rows = 0;
            gctINT i, arraySize = 1;

            /* Determine the number of rows per element. */
            gcTYPE_GetTypeInfo(variable->u.type, &components, &rows, 0);

            gcmASSERT(count == 1 || count <= (gctUINT)variable->arraySize);

            for (i = 0; i < variable->arrayLengthCount; i++)
            {
                arraySize *= variable->arrayLengthList[i];
            }

            count = arraySize * rows;
            if (tempIndex != (gctINT)variable->tempIndex)
            {
                Temp = Tree->tempArray + variable->tempIndex;

                if (!Temp->isIndexing)
                {
                    ++Temp;
                }

                gcmASSERT(Temp->assigned == -1);
#if gcmIS_DEBUG(gcdDEBUG_ASSERT)
                {
                    gctINT index = tempIndex - variable->tempIndex;
                    gcmASSERT((index > 0) && (index < (gctINT) count));
                }
#endif
            }
        }
#if _SUPPORT_LONG_ULONG_DATA_TYPE
        else
        {
            if (isFormat64bit(Temp->format))
            {
                count *= 2;
            }
        }
#endif
    }
#if _SUPPORT_LONG_ULONG_DATA_TYPE
    else
    {
        if (isFormat64bit(Temp->format))
        {
            count *= 2;
        }
    }
#endif

    /* No need to assign */
    if (count == 0) return gcvSTATUS_OK;

    Temp->shift = 0;
    do
    {
        gctUINT8 enable, swizzle = (gctUINT8)Temp->swizzle;
        gctINT assigned = Temp->assigned, shift = Temp->shift;
        /* Allocate physical register. */
        gcmERR_BREAK(
            _FindRegisterUsage(CodeGen->registerUsage,
                       CodeGen->registerCount,
                       type,
                       count,
                       (Temp->lastUse == -1) ? gcvSL_RESERVED : Temp->lastUse,
                       (Temp->lastUse == -1),
                       &assigned,
                       &swizzle,
                       &shift,
                       &enable,
                       0));
        Temp->assigned = assigned;
        Temp->swizzle  = swizzle;
        Temp->shift    = shift;
        gcCGUpdateMaxRegister(CodeGen, Temp->assigned, Tree);

        if (gcSHADER_DumpCodeGenVerbose(Tree->shader))
            dumpRegisterAllocation(Temp);

        /* Assign all temps in the output array. */
        for (i = 1; i < count; ++i)
        {
            if (Temp[i].assigned != -1)
                continue;

            Temp[i].assigned = Temp->assigned + i;
            Temp[i].swizzle  = Temp->swizzle;
            Temp[i].shift    = Temp->shift;

            gcCGUpdateMaxRegister(CodeGen, Temp[i].assigned, Tree);

            if (gcSHADER_DumpCodeGenVerbose(Tree->shader))
                dumpRegisterAllocation(&Temp[i]);

            /* Update lastUse for array. */
            if (Temp[i].lastUse > Temp->lastUse)
            {
                _SetRegisterUsage(CodeGen->registerUsage + Temp->assigned + i,
                                  1,
                                  enable,
                                  Temp[i].lastUse);
            }
        }
    }
    while (gcvFALSE);

    if (status != gcvSTATUS_OK)
    {
        CodeGen->isRegOutOfResource = gcvTRUE;
    }

    /* Return the status. */
    return status;
}

static gceSTATUS
_GetTEMPDest(
    IN gcLINKTREE Tree,
    IN gctINT32 Index,
    IN gctUINT32 Enable,
    IN gctUINT32 *index,
    IN gctUINT32 *enable
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcLINKTREE_TEMP temp = (Index >= 0)
                ? &Tree->tempArray[Index]
                : gcvNULL;

    *index = (Index >= 0) ? Index : -Index - 1 ;
    *enable = Enable;

    if (temp != gcvNULL)
    {
        temp->shift = 0;
    }

    return status;
}

static gctINT
_Enable2Shift(
    IN gctUINT32    enable
    )
{
    gctINT shift = 0;

    while (!((enable >> shift) & 0x1))
    {
        shift ++;
    }

    return shift;
}

static gceSTATUS
_SetDest(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN OUT gctUINT32 * States,
    IN gctINT Index,
    IN gctUINT32 Relative,
    IN gctUINT32 Enable,
    OUT gctINT_PTR Shift
    )
{
    gceSTATUS status;
    gctUINT32 index, enable;

    do
    {
        if (_isHWRegisterAllocated(Tree->shader))
        {
            gcmVERIFY_OK(_GetTEMPDest(Tree, Index, Enable, &index, &enable));
            if (Shift != gcvNULL)
            {
                /* get shift from enable, shift is only used when the instruction is newly generated */
                *Shift = _Enable2Shift(enable);
            }
        }
        else
        {
            /* Extract temporary register information. */
            gcLINKTREE_TEMP temp = (Index >= 0)
                ? &Tree->tempArray[Index]
                : gcvNULL;

            /* See if temporary register needs to be assigned. */
            if ((temp != gcvNULL) && (temp->assigned == -1) )
            {
                gcmERR_BREAK(_AllocateRegisterForTemp(Tree, CodeGen, temp));
            }

            /* Load physical mapping. */
            index  = (temp == gcvNULL) ? -Index - 1 : temp->assigned;
            enable = (temp == gcvNULL) ? Enable : (Enable << temp->shift);
            gcmASSERT((enable & ~gcSL_ENABLE_XYZW) == 0);

            if (index == CodeGen->reservedRegForLoad)
            {
                gcmASSERT(Index == CodeGen->loadDestIndex);
                index = CodeGen->origAssigned;
            }

            if (Shift != gcvNULL)
            {
                *Shift = (temp != gcvNULL) ? temp->shift : -1;
            }
        }

        /* Set DEST fields. */
        States[0] |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                  |  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (index) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
                  |  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (Relative) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)))
                  |  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));

        /* Keep maximum physical register. */
        gcCGUpdateMaxRegister(CodeGen, index, Tree);

        /* Success. */
        status = gcvSTATUS_OK;
    }
    while (gcvFALSE);

    /* Return the status. */
    return status;
}

static gceSTATUS
_SetDestWithPrecision(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN OUT gctUINT32 * States,
    IN gctINT Index,
    IN gctUINT32 Relative,
    IN gctUINT32 Enable,
    IN gcSL_PRECISION Precision,
    OUT gctINT_PTR Shift
    )
{
    gceSTATUS status;
    gctUINT32 index, enable;
    gctUINT32 type;

    do
    {
        if (_isHWRegisterAllocated(Tree->shader))
        {
            gcmVERIFY_OK(_GetTEMPDest(Tree, Index, Enable, &index, &enable));
            if (Shift != gcvNULL)
            {
                *Shift = _Enable2Shift(enable);
            }
        }
        else
        {
            /* Extract temporary register information. */
            gcLINKTREE_TEMP temp = (Index >= 0)
                ? &Tree->tempArray[Index]
                : gcvNULL;

            /* See if temporary register needs to be assigned. */
            if ((temp != gcvNULL) && (temp->assigned == -1) )
            {
                gcmERR_BREAK(_AllocateRegisterForTemp(Tree, CodeGen, temp));
            }

            /* Load physical mapping. */
            index  = (temp == gcvNULL) ? -Index - 1 : temp->assigned;
            enable = (temp == gcvNULL) ? Enable : (Enable << temp->shift);
            gcmASSERT((enable & ~gcSL_ENABLE_XYZW) == 0);

            if (index == CodeGen->reservedRegForLoad)
            {
                gcmASSERT(Index == CodeGen->loadDestIndex);
                index = CodeGen->origAssigned;
            }

            if (Shift != gcvNULL)
            {
                *Shift = (temp != gcvNULL) ? temp->shift : -1;
            }
        }

        /* Set DEST fields. */
        type = Precision == gcSL_PRECISION_HIGH ? 0x1
                                                : 0x0;
        States[0] |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                  |  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (index) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
                  |  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (Relative) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)))
                  |  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)))
                  |  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (type) & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)));

        /* Keep maximum physical register. */
        gcCGUpdateMaxRegister(CodeGen, index, Tree);

        /* Success. */
        status = gcvSTATUS_OK;
    }
    while (gcvFALSE);

    /* Return the status. */
    return status;
}

static gceSTATUS
_SetSampler(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN OUT gctUINT32 * States,
    IN gctUINT32 Index,
    IN gctUINT32 Relative,
    IN gctUINT32 Swizzle,
    IN gctUINT32 Type,
    IN gctINT32 arrayLoc
    )
{
    gceSTATUS status;
    gctINT32 sampler;

    do
    {
        /* Load physical sampler number. */
        if (gcmIsDummySamplerId(Index))
        {
            /* dummy texld code (see -PATCH_TEXLD option)*/
            sampler = CodeGen->dummySamplerId;
        }
        else if (Type == gcSL_UNIFORM)
        {
            gcmASSERT(arrayLoc < Tree->shader->uniforms[Index]->arraySize);
            sampler = Tree->shader->uniforms[Index]->physical + arrayLoc;
            gcmASSERT(!gcmOPT_PATCH_TEXLD() || sampler != CodeGen->dummySamplerId);
        }
        else
        {
            sampler = Index + arrayLoc;
        }

        /* Set SAMPLER fields. */
        States[0] |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) ((gctUINT32) (sampler) & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));
        States[1] |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (Relative) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  |  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:3) - (0 ?
 10:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:3) - (0 ?
 10:3) + 1))))))) << (0 ?
 10:3))) | (((gctUINT32) ((gctUINT32) (Swizzle) & ((gctUINT32) ((((1 ?
 10:3) - (0 ?
 10:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ? 10:3)));

        /* Success. */
        status = gcvSTATUS_OK;
    }
    while (gcvFALSE);

    /* Return the status. */
    return status;
}

static void
_SetSrcValue(
    IN OUT gctUINT32 States[4],
    IN gctUINT   Source,
    IN gctUINT   Type,
    IN gctINT32  Index,
    IN gctUINT32 Relative,
    IN gctUINT32 Swizzle,
    IN gctBOOL   Negate,
    IN gctBOOL   Absolute
    )
{
    switch (Source)
    {
    case 0:
        /* Set SRC0 fields. */
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:12) - (0 ?
 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (Index) & ((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ? 20:12)));
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:22) - (0 ?
 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (Swizzle) & ((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ? 29:22)));
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:30) - (0 ?
 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) ((gctUINT32) (Negate) & ((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)));
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (Absolute) & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)));
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (Relative) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)));
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (Type) & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)));
        break;

    case 1:
        /* Set SRC1 fields. */
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:7) - (0 ?
 15:7) + 1))))))) << (0 ?
 15:7))) | (((gctUINT32) ((gctUINT32) (Index) & ((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:7) - (0 ? 15:7) + 1))))))) << (0 ? 15:7)));
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:27) - (0 ?
 29:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:27) - (0 ?
 29:27) + 1))))))) << (0 ?
 29:27))) | (((gctUINT32) ((gctUINT32) (Relative) & ((gctUINT32) ((((1 ?
 29:27) - (0 ?
 29:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:27) - (0 ? 29:27) + 1))))))) << (0 ? 29:27)));
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:17) - (0 ?
 24:17) + 1))))))) << (0 ?
 24:17))) | (((gctUINT32) ((gctUINT32) (Swizzle) & ((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1))))))) << (0 ? 24:17)));
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:25) - (0 ?
 25:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:25) - (0 ?
 25:25) + 1))))))) << (0 ?
 25:25))) | (((gctUINT32) ((gctUINT32) (Negate) & ((gctUINT32) ((((1 ?
 25:25) - (0 ?
 25:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ? 25:25)));
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (Absolute) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26)));
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (Type) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)));
        break;

    case 2:
        /* Set SRC2 fields. */
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)));
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (Type) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (Index) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)));
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:25) - (0 ?
 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (Relative) & ((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)));
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (Swizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)));
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:22) - (0 ?
 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) ((gctUINT32) (Negate) & ((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ? 22:22)));
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:23) - (0 ?
 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) (Absolute) & ((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ? 23:23)));
        break;

    default:
        break;
    }
}

/* immediate/non-immediate encode for Source 0, 1, 2:
    Bit Range  Bit Range  Bit Range  Description                  Description
    Source 0   Source 1   Source 2   Source_Type not Immediate    Source_Type is immediate
    ----------------------------------------------------------------------------------------
    [43]       [70]       [99]              Read valid.  1 for valid, 0 for invalid
    [52:44]    [79:71]    [108:100]  Address                      Immediate Value bit 8:0
    [61:54]    [88:81]    [117:110]  Swizzle                      Immediate Value bit 16:9
    [62]       [89]       [118]      Negate modifier.             Immediate Value bit 17
    [63]       [90]       [119]      Absolute value modifier.     Immediate Value bit 18
    [64]       [91]       [121]      Relative Addressing bit 0    Immediate Value bit 19
    [66:65]    [93:92]    [123:122]  Relative Addressing bit 2:1  Immediate_Type
                                                                    0: U20
                                                                    1: S20
                                                                    2: FP20
                                                                    3: PACKED16
    [69:67]    [98:96]    [126:124]                       Source Type


 */
gceSTATUS
gcEncodeSourceImmediate20(
    IN OUT gctUINT32         States[4],
    IN gctUINT               Source,
    IN gcsConstantValue *    ConstValue)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 type = 0x7;
    gctUINT32 address = 0;
    gctUINT8  swizzle = 0;
    gctUINT32 relative = 0;
    gctBOOL   negate = 0;
    gctBOOL   absolute = 0;
    gctUINT32 valueType = 0;
    gctUINT32 value20bits = 0;
    gcsFLOAT20 f20;

    switch (ConstValue->ty) {
    case gcSL_FLOAT:
        valueType = 0x0 ;
        gcConvertF32ToF20(ConstValue->value.u, &f20);
        value20bits = GetFP20Binary(&f20);
        break;
    case gcSL_INT32:
    case gcSL_INT64:
        valueType = 0x1 ;
        value20bits = ConstValue->value.i & BitsMask(20);
        break;
    case gcSL_UINT32:
    case gcSL_UINT64:
        valueType = 0x2 ;
        value20bits = ConstValue->value.u & BitsMask(20);
        break;
    case gcSL_INT16:
    case gcSL_UINT16:
        valueType = 0x1 /* 0x3 */;
        value20bits = ConstValue->value.u & BitsMask(20);
        break;
    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    /* encode the bits into register fields, currently we only have
       non-immediate field names */
    address  = (value20bits >> 0)  & BitsMask(9);
    swizzle  = (value20bits >> 9)  & BitsMask(8);
    negate   = (value20bits >> 17) & BitsMask(1);
    absolute = (value20bits >> 18) & BitsMask(1);
    relative = (value20bits >> 19) & BitsMask(1);
    relative |= valueType << 1;  /* Immediate_Type */

    _SetSrcValue(States,
                 Source,
                 type,
                 address,
                 relative,
                 swizzle,
                 negate,
                 absolute
                );

    return status;
}

static gceSTATUS
_GetTEMPSource(
    IN gcLINKTREE Tree,
    IN gctINT32 Index,
    IN gctINT32 Swizzle,
    IN gctUINT32 *type,
    IN gctUINT32 *index,
    IN gctUINT8 *swizzle
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcLINKTREE_TEMP temp = &Tree->tempArray[Index];

    if (!gcHWCaps.hwFeatureFlags.vtxInstanceIdAsAttr)
    {

        if (Index == VIR_SR_VERTEXID)
        {
            *type = 0x4;
            *index = 0;
            *swizzle = (gctUINT8) Swizzle;
        }
        else if (Index == VIR_SR_INSTATNCEID)
        {
            *type = 0x5;
            *index = 0;
            *swizzle = (gctUINT8) Swizzle;
        }
        else
        {
            *type    = 0x0;
            *index   = Index;
            *swizzle = (gctUINT8) Swizzle;
        }
    }
    else
    {
        *type    = 0x0;
        *index   = Index;
        *swizzle = (gctUINT8) Swizzle;
    }

    temp->shift = 0;

    return status;
}

static gceSTATUS
_GetTEMPSourceWithPrecision(
    IN gcLINKTREE Tree,
    IN gctINT32 Index,
    IN gctINT32 Swizzle,
    IN gcSL_PRECISION Precision,
    IN gctUINT32 *type,
    IN gctUINT32 *index,
    IN gctUINT8 *swizzle
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcLINKTREE_TEMP temp = &Tree->tempArray[Index];

    if (Precision == gcSL_PRECISION_HIGH)
    {
        *type = 0x4;
    }
    else
    {
        *type = 0x0;
    }

    if (!gcHWCaps.hwFeatureFlags.vtxInstanceIdAsAttr)
    {
        if (Index == VIR_SR_VERTEXID)
        {
            *type = 0x4;
            *index = 0;
            *swizzle = (gctUINT8) Swizzle;
        }
        else if (Index == VIR_SR_INSTATNCEID)
        {
            *type = 0x5;
            *index = 0;
            *swizzle = (gctUINT8) Swizzle;
        }
        else
        {
            *index   = Index;
            *swizzle = (gctUINT8) Swizzle;
        }
    }
    else
    {
        *index   = Index;
        *swizzle = (gctUINT8) Swizzle;
    }

    temp->shift = 0;

    return status;
}

static gceSTATUS
_SetSource(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN OUT gctUINT32 States[4],
    IN gctUINT Source,
    IN gcSL_TYPE Type,
    IN gctINT32 Index,
    IN gctUINT32 ConstIndex,
    IN gctUINT32 Relative,
    IN gctUINT32 Swizzle,
    IN gctBOOL Negate,
    IN gctBOOL Absolute
    )
{
    gctUINT32 type = 0;
    gctUINT32 index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    /* Dispatch on given source type. */
    switch (Type)
    {
    case gcSL_TEMP:
        /* Temporary register. */
        if (Index < 0)
        {
            type    = 0x0;
            index   = -Index - 1;
            swizzle = (gctUINT8) Swizzle;
        }
        else
        {
            gctBOOL useConst = gcvTRUE;
            gctUINT8 usage = _Swizzle2Enable((gcSL_SWIZZLE) ((Swizzle >> 0) & 3),
                                             (gcSL_SWIZZLE) ((Swizzle >> 2) & 3),
                                             (gcSL_SWIZZLE) ((Swizzle >> 4) & 3),
                                             (gcSL_SWIZZLE) ((Swizzle >> 6) & 3));

            /* Test for constant propagation in X component. */
            if ((usage & gcSL_ENABLE_X)
            &&  (Tree->tempArray[Index].constUsage[0] != 1)
            )
            {
                useConst = gcvFALSE;
            }

            /* Test for constant propagation in Y component. */
            if ((usage & gcSL_ENABLE_Y)
            &&  (Tree->tempArray[Index].constUsage[1] != 1)
            )
            {
                useConst = gcvFALSE;
            }

            /* Test for constant propagation in Z component. */
            if ((usage & gcSL_ENABLE_Z)
            &&  (Tree->tempArray[Index].constUsage[2] != 1)
            )
            {
                useConst = gcvFALSE;
            }

            /* Test for constant propagation in W component. */
            if ((usage & gcSL_ENABLE_W)
            &&  (Tree->tempArray[Index].constUsage[3] != 1)
            )
            {
                useConst = gcvFALSE;
            }

            /* Is this register used as constant propagation? */
            if (useConst)
            {
                gcmVERIFY_OK(_AllocateConst(Tree,
                                            CodeGen,
                                            usage,
                                            Tree->tempArray[Index].constValue,
                                            gcvFALSE,
                                            gcvTRUE,
                                            (gctINT32_PTR) &index,
                                            (gctUINT8_PTR) &swizzle,
                                            gcvNULL,
                                            &constType));

                if (constType == gcSL_UNIFORM)
                {
                    type = 0x2;
                }
                else
                {
                    type = 0x0;
                }

                swizzle = _AdjustSwizzle((gctUINT8) Swizzle, swizzle);
            }

            else
            {
                if (_isHWRegisterAllocated(Tree->shader))
                {
                    /* VIR RA is enabled, so use the hw reg from the index */
                    gcmVERIFY_OK(_GetTEMPSource(
                        Tree,
                        Index,
                        Swizzle,
                        &type,
                        &index,
                        &swizzle));
                }
                else
                {
                    gcVARIABLE variable = Tree->tempArray[Index].variable;

                    /* See if temporary register needs to be assigned. */

                    if(variable &&
                       (variable->nameLength == gcSL_VERTEX_ID ||
                        variable->nameLength == gcSL_INSTANCE_ID)) { /* check for instanceID and vertexID */
                       gcmASSERT((gctINT32)variable->tempIndex == Index);
                       if (gcHWCaps.hwFeatureFlags.vtxInstanceIdAsAttr) {
#if !DX_SHADER
                           gcmASSERT (Tree->tempArray[Index].assigned != -1);
                           if (Tree->tempArray[Index].assigned == -1) {
                               gcmFATAL("Register for instance or vertex id should have been assigned already");
                           }
#else
                           Tree->tempArray[Index].usage = 0xe;

                           gcmVERIFY_OK(
                               _AllocateRegisterForTemp(Tree, CodeGen, &Tree->tempArray[Index]));
#endif

                           type    = 0x0;
                           index   = Tree->tempArray[Index].assigned;
                           swizzle = _AdjustSwizzle((gctUINT8) Swizzle,
                                                    Tree->tempArray[Index].swizzle);
                       }
                       else {
                           if (Tree->tempArray[Index].assigned == -1) {
                               if(variable->nameLength == gcSL_VERTEX_ID) {
                                   type    = 0x4;
                               }
                               else
                               {
                                   type    = 0x5;
                               }
                               index = 0;
                               swizzle = (gctUINT8) Swizzle;

                               gcmVERIFY_OK(
                                   _AllocateRegisterForTemp(Tree, CodeGen, &Tree->tempArray[Index]));
                           }
                           else {
                               type    = 0x0;
                               index   = Tree->tempArray[Index].assigned;
                               swizzle = _AdjustSwizzle((gctUINT8) Swizzle,
                                                        Tree->tempArray[Index].swizzle);
                           }
                       }
                    }
                    else {
                       if (Tree->tempArray[Index].assigned == -1)
                       {
                           gcmVERIFY_OK(
                               _AllocateRegisterForTemp(Tree, CodeGen, &Tree->tempArray[Index]));
                       }

                       type    = 0x0;
                       index   = Tree->tempArray[Index].assigned;
                       swizzle = _AdjustSwizzle((gctUINT8) Swizzle,
                                                Tree->tempArray[Index].swizzle);
                    }
                }
            }
        }

        /* Keep maximum physical register. */
        if (type == 0x0)
        {
            gcCGUpdateMaxRegister(CodeGen, index, Tree);
        }
        break;

    case gcSL_ATTRIBUTE:
        /* Attribute. */
        gcmASSERT(Tree->shader->attributes[Index]->inputIndex != -1);

        if (Tree->shader->attributes[Index]->nameLength == gcSL_FRONT_FACING)
        {
            if (CodeGen->useFace)
            {
                type    = 0x0;
                index   = CodeGen->facePhysical;
                swizzle = CodeGen->faceSwizzle;
            }
            else
            {
                if ((Tree->shader->compilerVersion[0] & 0xFFFF) == _SHADER_DX_LANGUAGE_TYPE &&
                    Tree->shader->compilerVersion[1] == _SHADER_DX_VERSION_30)
                {
                    type    = 0x1;
                    index   = 0;
                    swizzle = gcSL_SWIZZLE_YYYY;
                }
                else
                {
                    type    = 0x1;
                    index   = 0;
                    swizzle = gcSL_SWIZZLE_XXXX;
                }
            }
        }

        else if (Tree->shader->attributes[Index]->nameLength == gcSL_POSITION)
        {
            type    = 0x0;
            index   = CodeGen->usePosition ? CodeGen->positionPhysical : 0;
            swizzle = (gctUINT8) Swizzle;
        }

        else
        {
            gctUINT rows = 1;

            gcTYPE_GetTypeInfo(Tree->shader->attributes[Index]->type, gcvNULL, &rows, 0);
            rows   *= GetATTRArraySize(Tree->shader->attributes[Index]);
            type    = 0x0;
            index   = Tree->shader->attributes[Index]->inputIndex + ConstIndex;
            swizzle = (gctUINT8) Swizzle;

            /* Keep maximum physical register. */
            gcCGUpdateMaxRegister(CodeGen,
                               Tree->shader->attributes[Index]->inputIndex + rows - 1,
                               Tree);
        }
        break;

    case gcSL_UNIFORM:
        /* Uniform. */
        gcmASSERT(Tree->shader->uniforms[Index]->physical != -1);

        type    = 0x2;
        index   = Tree->shader->uniforms[Index]->physical + ConstIndex;
        swizzle = _AdjustSwizzle((gctUINT8) Swizzle,
                                 Tree->shader->uniforms[Index]->swizzle);
        break;

    case gcSL_CONSTANT:
        /* Constant. */
        type    = 0x2;
        index   = Index;
        swizzle = (gctUINT8) Swizzle;
        break;

    case gcSL_NONE:
        return gcvSTATUS_OK;

    default:
        gcmFATAL("!!TODO!! Unknown Type %u", Type);
    }

    _SetSrcValue(States,
                 Source,
                 type,
                 index,
                 Relative,
                 swizzle,
                 Negate,
                 Absolute
                );

    /* Success. */
    return gcvSTATUS_OK;
}

static gceSTATUS
_SetSourceWithPrecision(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN OUT gctUINT32 States[4],
    IN gctUINT Source,
    IN gcSL_TYPE Type,
    IN gctINT32 Index,
    IN gctUINT32 ConstIndex,
    IN gctUINT32 Relative,
    IN gctUINT32 Swizzle,
    IN gctBOOL Negate,
    IN gctBOOL Absolute,
    IN gcSL_PRECISION Precision
    )
{
    gctUINT32 type = 0;
    gctUINT32 index = 0;
    gctUINT8 swizzle = 0;
    gcSL_TYPE constType;

    /* Dispatch on given source type. */
    switch (Type)
    {
    case gcSL_TEMP:
        /* Temporary register. */
        if (Precision == gcSL_PRECISION_HIGH)
        {
            type   = 0x4;
        }
        else
        {
            type   = 0x0;
        }

        if (Index < 0)
        {
            index   = -Index - 1;
            swizzle = (gctUINT8) Swizzle;
        }
        else
        {
            gctBOOL useConst = gcvTRUE;
            gctUINT8 usage = _Swizzle2Enable((gcSL_SWIZZLE) ((Swizzle >> 0) & 3),
                                             (gcSL_SWIZZLE) ((Swizzle >> 2) & 3),
                                             (gcSL_SWIZZLE) ((Swizzle >> 4) & 3),
                                             (gcSL_SWIZZLE) ((Swizzle >> 6) & 3));

            /* Test for constant propagation in X component. */
            if ((usage & gcSL_ENABLE_X)
            &&  (Tree->tempArray[Index].constUsage[0] != 1)
            )
            {
                useConst = gcvFALSE;
            }

            /* Test for constant propagation in Y component. */
            if ((usage & gcSL_ENABLE_Y)
            &&  (Tree->tempArray[Index].constUsage[1] != 1)
            )
            {
                useConst = gcvFALSE;
            }

            /* Test for constant propagation in Z component. */
            if ((usage & gcSL_ENABLE_Z)
            &&  (Tree->tempArray[Index].constUsage[2] != 1)
            )
            {
                useConst = gcvFALSE;
            }

            /* Test for constant propagation in W component. */
            if ((usage & gcSL_ENABLE_W)
            &&  (Tree->tempArray[Index].constUsage[3] != 1)
            )
            {
                useConst = gcvFALSE;
            }

            /* Is this register used as constant propagation? */
            if (useConst)
            {
                gcmVERIFY_OK(_AllocateConst(Tree,
                                            CodeGen,
                                            usage,
                                            Tree->tempArray[Index].constValue,
                                            gcvFALSE,
                                            gcvTRUE,
                                            (gctINT32_PTR) &index,
                                            (gctUINT8_PTR) &swizzle,
                                            gcvNULL,
                                            &constType));

                if (constType == gcSL_UNIFORM)
                {
                    type = 0x2;
                }
                else
                {
                    type = 0x0;
                }

                swizzle = _AdjustSwizzle((gctUINT8) Swizzle, swizzle);
            }

            else
            {
                if (_isHWRegisterAllocated(Tree->shader))
                {
                    /* VIR RA is enabled, so use the hw reg from the index */
                    gcmVERIFY_OK(_GetTEMPSourceWithPrecision(
                        Tree,
                        Index,
                        Swizzle,
                        Precision,
                        &type,
                        &index,
                        &swizzle));
                }
                else
                {
                    gcVARIABLE variable = Tree->tempArray[Index].variable;

                    /* See if temporary register needs to be assigned. */

                    if(variable &&
                       (variable->nameLength == gcSL_VERTEX_ID ||
                        variable->nameLength == gcSL_INSTANCE_ID)) { /* check for instanceID and vertexID */
                       gcmASSERT((gctINT32)variable->tempIndex == Index);
                       if (gcHWCaps.hwFeatureFlags.vtxInstanceIdAsAttr) {
                           gcmASSERT (Tree->tempArray[Index].assigned != -1);
                           if (Tree->tempArray[Index].assigned == -1) {
                               gcmFATAL("Register for instance or vertex id should have been assigned already");
                           }
                           type    = 0x0;
                           index   = Tree->tempArray[Index].assigned;
                           swizzle = _AdjustSwizzle((gctUINT8) Swizzle,
                                                    Tree->tempArray[Index].swizzle);
                       }
                       else {
                           if (Tree->tempArray[Index].assigned == -1) {
                               if(variable->nameLength == gcSL_VERTEX_ID) {
                                   type    = 0x4;
                               }
                               else
                               {
                                   type    = 0x5;
                               }
                               index = 0;
                               swizzle = (gctUINT8) Swizzle;

                               gcmVERIFY_OK(
                                   _AllocateRegisterForTemp(Tree, CodeGen, &Tree->tempArray[Index]));
                           }
                           else {
                               index   = Tree->tempArray[Index].assigned;
                               swizzle = _AdjustSwizzle((gctUINT8) Swizzle,
                                                        Tree->tempArray[Index].swizzle);
                           }
                       }
                    }
                    else {
                       if (Tree->tempArray[Index].assigned == -1)
                       {
                           gcmVERIFY_OK(
                               _AllocateRegisterForTemp(Tree, CodeGen, &Tree->tempArray[Index]));
                       }

                       index   = Tree->tempArray[Index].assigned;
                       swizzle = _AdjustSwizzle((gctUINT8) Swizzle,
                                                Tree->tempArray[Index].swizzle);
                    }
                }
            }
        }

        /* Keep maximum physical register. */
        if (type == 0x0 ||
            type == 0x4)
        {
            gcCGUpdateMaxRegister(CodeGen, index, Tree);
        }
        break;

    case gcSL_ATTRIBUTE:
        /* Attribute. */
        gcmASSERT(Tree->shader->attributes[Index]->inputIndex != -1);

        if (Tree->shader->attributes[Index]->nameLength == gcSL_FRONT_FACING)
        {
            if (CodeGen->useFace)
            {
                type    = 0x0;
                index   = CodeGen->facePhysical;
                swizzle = CodeGen->faceSwizzle;
            }
            else
            {
                if ((Tree->shader->compilerVersion[0] & 0xFFFF) == _SHADER_DX_LANGUAGE_TYPE &&
                    Tree->shader->compilerVersion[1] == _SHADER_DX_VERSION_30)
                {
                    type    = 0x1;
                    index   = 0;
                    swizzle = gcSL_SWIZZLE_YYYY;
                }
                else
                {
                    type    = 0x1;
                    index   = 0;
                    swizzle = gcSL_SWIZZLE_XXXX;
                }
            }
        }

        else if (Tree->shader->attributes[Index]->nameLength == gcSL_POSITION)
        {
            type    = 0x0;
            index   = CodeGen->usePosition ? CodeGen->positionPhysical : 0;
            swizzle = (gctUINT8) Swizzle;
        }

        else
        {
            type    = 0x0;
            index   = Tree->shader->attributes[Index]->inputIndex + ConstIndex;
            swizzle = (gctUINT8) Swizzle;

            /* Keep maximum physical register. */
            if(Relative > 0 && Tree->shader->attributes[Index]->arraySize > 1)
            {
                gcCGUpdateMaxRegister(CodeGen, Tree->shader->attributes[Index]->inputIndex +
                        Tree->shader->attributes[Index]->arraySize - 1, Tree);
            }
            else
            {
                gcCGUpdateMaxRegister(CodeGen, index, Tree);
            }
        }
        break;

    case gcSL_UNIFORM:
        /* Uniform. */
        gcmASSERT(Tree->shader->uniforms[Index]->physical != -1);

        type    = 0x2;
        index   = Tree->shader->uniforms[Index]->physical + ConstIndex;
        swizzle = _AdjustSwizzle((gctUINT8) Swizzle,
                                 Tree->shader->uniforms[Index]->swizzle);
        break;

    case gcSL_CONSTANT:
        /* Constant. */
        type    = 0x2;
        index   = Index;
        swizzle = (gctUINT8) Swizzle;
        break;

    case gcSL_NONE:
        return gcvSTATUS_OK;

    default:
        gcmFATAL("!!TODO!! Unknown Type %u", Type);
    }

    _SetSrcValue(States,
                 Source,
                 type,
                 index,
                 Relative,
                 swizzle,
                 Negate,
                 Absolute
                );

    /* Success. */
    return gcvSTATUS_OK;
}

static void
_UpdateRegisterUsage(
    IN OUT gcsSL_USAGE_PTR Usage,
    IN gctSIZE_T UsageCount,
    IN gctINT Index
    )
{
    gctSIZE_T u, i;

    /* Process all usages */
    for (u = 0; u < UsageCount; ++u)
    {
        /* Process each component. */
        for (i = 0; i < gcmCOUNTOF(Usage->lastUse); ++i)
        {
            /* See if the component can be made available. */
            if (Usage[u].lastUse[i] == Index)
            {
                Usage[u].lastUse[i] = gcvSL_AVAILABLE;
            }
        }
    }
}

gctUINT8
_ReplicateSwizzle(
    IN gctUINT8 Swizzle,
    IN gctUINT Index
    )
{
    gctUINT8 swizzle = _ExtractSwizzle(Swizzle, Index);
    swizzle |= swizzle << 2;
    return swizzle | (swizzle << 4);
}

gctUINT8
_ReplicateSwizzle2(
    IN gctUINT8 Swizzle,
    IN gctUINT Index
    )
{
    gctUINT8 swizzle;

    switch (Index)
    {
    case 0:
        swizzle = Swizzle & 0xF;
        return swizzle | (swizzle << 4);

    case 1:
        swizzle = Swizzle & 0xF0;
        return swizzle | (swizzle >> 4);

    default:
        break;
    }

    gcmFATAL("Invalid swizzle index %d.", Index);
    return 0;
}

static gceSTATUS
_TempEmit(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctUINT32 States[4],
    IN gctUINT Source,
    IN gctINT * Physical,
    IN gctINT * LastUse
    )
{
    gctUINT physical    = 0;
    gctINT shift        = 0;
    gctUINT8 swizzle    = 0;
    gctUINT32 states[4] = {0};
    gceSTATUS status    = gcvSTATUS_OK;
    gctUINT address = 0, relative = 0, type = 0;
    gctINT lastUse      = 0;

    do
    {
        if (_isHWRegisterAllocated(Tree->shader))
        {
            physical = Tree->shader->RARegWaterMark;
            _UpdateRATempReg(Tree, 1);
            shift = 0;
        }
        else
        {
        /* Allocate a temporary register. */
        lastUse = (Tree->hints[CodeGen->nextSource - 1].lastUseForTemp == (gctINT) CodeGen->nextSource - 1) ?
                  gcvSL_TEMPORARY : Tree->hints[CodeGen->nextSource - 1].lastUseForTemp;
        gcmERR_BREAK(_FindRegisterUsage(CodeGen->registerUsage,
                                CodeGen->registerCount,
                                gcSHADER_FLOAT_X4,
                                1,
                                lastUse /*gcvSL_TEMPORARY*/,
                                gcvFALSE,
                                (gctINT_PTR) &physical,
                                &swizzle,
                                &shift,
                                gcvNULL,
                                0));
        }
        if (((((((gctUINT32) (States[1])) >> (0 ? 11:11)) & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1)))))) )
            && ((((((gctUINT32) (States[2])) >> (0 ? 5:3)) & ((gctUINT32) ((((1 ? 5:3) - (0 ? 5:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1)))))) ) == 0x0)
            && ((((((gctUINT32) (States[1])) >> (0 ? 20:12)) & ((gctUINT32) ((((1 ? 20:12) - (0 ? 20:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1)))))) ) == physical)
            )
        ||     ((((((gctUINT32) (States[2])) >> (0 ? 6:6)) & ((gctUINT32) ((((1 ? 6:6) - (0 ? 6:6) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1)))))) )
            && ((((((gctUINT32) (States[3])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) ) == 0x0)
            && ((((((gctUINT32) (States[2])) >> (0 ? 15:7)) & ((gctUINT32) ((((1 ? 15:7) - (0 ? 15:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:7) - (0 ? 15:7) + 1)))))) ) == physical)
            )
        ||     ((((((gctUINT32) (States[3])) >> (0 ? 3:3)) & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1)))))) )
            && ((((((gctUINT32) (States[3])) >> (0 ? 30:28)) & ((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1)))))) ) == 0x0)
            && ((((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ) == physical)
            )
        )
        {
            if (_isHWRegisterAllocated(Tree->shader))
            {
                physical++;
                _UpdateRATempReg(Tree, 2);
                shift = 0;
            }
            else
            {
            gcmERR_BREAK(_FindRegisterUsage(CodeGen->registerUsage,
                                    CodeGen->registerCount,
                                    gcSHADER_FLOAT_X4,
                                    1,
                                    lastUse /*gcvSL_TEMPORARY*/,
                                    gcvFALSE,
                                    (gctINT_PTR) &physical,
                                    &swizzle,
                                    &shift,
                                    gcvNULL,
                                    0));
            }
        }

        gcCGUpdateMaxRegister(CodeGen, physical, Tree);

        /* MOV temp, sourceX */
        switch (Source)
        {
        case 0:
            address  = (((((gctUINT32) (States[1])) >> (0 ? 20:12)) & ((gctUINT32) ((((1 ? 20:12) - (0 ? 20:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1)))))) );
            relative = (((((gctUINT32) (States[2])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) );
            type     = (((((gctUINT32) (States[2])) >> (0 ? 5:3)) & ((gctUINT32) ((((1 ? 5:3) - (0 ? 5:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1)))))) );
            break;

        case 1:
            address  = (((((gctUINT32) (States[2])) >> (0 ? 15:7)) & ((gctUINT32) ((((1 ? 15:7) - (0 ? 15:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:7) - (0 ? 15:7) + 1)))))) );
            relative = (((((gctUINT32) (States[2])) >> (0 ? 29:27)) & ((gctUINT32) ((((1 ? 29:27) - (0 ? 29:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:27) - (0 ? 29:27) + 1)))))) );
            type     = (((((gctUINT32) (States[3])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) );
            break;

        case 2:
            address  = (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) );
            relative = (((((gctUINT32) (States[3])) >> (0 ? 27:25)) & ((gctUINT32) ((((1 ? 27:25) - (0 ? 27:25) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1)))))) );
            type     = (((((gctUINT32) (States[3])) >> (0 ? 30:28)) & ((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1)))))) );
            break;

        default:
            break;
        }
        gcmASSERT(type != 0x7);

        states[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ?
 10:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:6) - (0 ?
 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x00 & ((gctUINT32) ((((1 ?
 10:6) - (0 ?
 10:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (physical) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (0xF) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));
        states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));
        states[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));
        states[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (0xE4) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:25) - (0 ?
 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (relative) & ((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (type) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));

        gcmERR_BREAK(_FinalEmit(Tree, CodeGen, states, 0));

        /* Modify sourceX. */
        switch (Source)
        {
        case 0:
            States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:12) - (0 ?
 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (physical) & ((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ? 20:12)));
            States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)));
            States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)));
            break;

        case 1:
            States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:7) - (0 ?
 15:7) + 1))))))) << (0 ?
 15:7))) | (((gctUINT32) ((gctUINT32) (physical) & ((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:7) - (0 ? 15:7) + 1))))))) << (0 ? 15:7)));
            States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:27) - (0 ?
 29:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:27) - (0 ?
 29:27) + 1))))))) << (0 ?
 29:27))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 29:27) - (0 ?
 29:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:27) - (0 ? 29:27) + 1))))))) << (0 ? 29:27)));
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)));
            break;

        case 2:
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (physical) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)));
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:25) - (0 ?
 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)));
            States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));
            break;

        default:
            break;
        }

        /* Success. */
        status = gcvSTATUS_OK;
    }
    while (gcvFALSE);

    if (Physical)
    {
        * Physical = physical;
    }

    if (LastUse)
    {
        * LastUse = lastUse;
    }

    /* Return the status. */
    return status;
}


static gceSTATUS
_FinalEmit(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctUINT32 States[4],
    IN gctINT32 loopCount)
{
    gceSTATUS status;
    gctUINT32 opcode;
    gctUINT8 enable = 0, swizzle = 0, mask, newSwizzle = 0;
    gctUINT8 swizzle1 = 0, newSwizzle1 = 0;
    gctBOOL handleSwizzle= gcvFALSE, handleSwizzle1 = gcvFALSE;
    gctBOOL again;
    gctBOOL genExtraMul = gcvFALSE;
    gctBOOL genExtraMov = gcvFALSE;
    gctUINT8 movSwizzle = 0;
    gctUINT32 movState0 = 0;
    gctUINT physical = 0;
    gctINT shift = 0;
    gctUINT8 swizzleTemp = 0, startSwizzle, orgSwizzle;
    gctUINT32 states[4];
    gctINT lastUse;
    gctUINT16 channelIdx;

    gcmASSERT(loopCount <= 4);

    /* Check whether this is a TEXKILL instruction. */
    if ((((((gctUINT32) (States[0])) >> (0 ? 5:0)) & ((gctUINT32) ((((1 ? 5:0) - (0 ? 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1)))))) ) == 0x17)
    {
        CodeGen->kill = gcvTRUE;
    }

    do
    {
        gcsSL_PHYSICAL_CODE_PTR code;

        /* Test for invalid uniform usage. */
        /* HW can't handle the instruction which contain two indexed sources or two different uniform sources.*/
        gctUINT32 s0Type  = (((((gctUINT32) (States[1])) >> (0 ? 11:11)) & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1)))))) )
            ? (((((gctUINT32) (States[2])) >> (0 ? 5:3)) & ((gctUINT32) ((((1 ? 5:3) - (0 ? 5:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1)))))) )
            : (gctUINT32) 0xFFFFFFFF;
        gctUINT32 s0Index = (((((gctUINT32) (States[1])) >> (0 ? 20:12)) & ((gctUINT32) ((((1 ? 20:12) - (0 ? 20:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1)))))) );
        gctUINT32 s0Rela   = (((((gctUINT32) (States[2])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) );
        gctUINT32 s1Type  = (((((gctUINT32) (States[2])) >> (0 ? 6:6)) & ((gctUINT32) ((((1 ? 6:6) - (0 ? 6:6) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1)))))) )
            ? (((((gctUINT32) (States[3])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) )
            : (gctUINT32) 0xFFFFFFFF;
        gctUINT32 s1Index = (((((gctUINT32) (States[2])) >> (0 ? 15:7)) & ((gctUINT32) ((((1 ? 15:7) - (0 ? 15:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:7) - (0 ? 15:7) + 1)))))) );
        gctUINT32 s1Rela   = (((((gctUINT32) (States[2])) >> (0 ? 29:27)) & ((gctUINT32) ((((1 ? 29:27) - (0 ? 29:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:27) - (0 ? 29:27) + 1)))))) );
        gctUINT32 s2Type  = (((((gctUINT32) (States[3])) >> (0 ? 3:3)) & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1)))))) )
            ? (((((gctUINT32) (States[3])) >> (0 ? 30:28)) & ((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1)))))) )
            : (gctUINT32) 0xFFFFFFFF;
        gctUINT32 s2Index = (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) );
        gctUINT32 s2Rela   = (((((gctUINT32) (States[3])) >> (0 ? 27:25)) & ((gctUINT32) ((((1 ? 27:25) - (0 ? 27:25) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1)))))) );

        gctINT32 newPhysical[2] = {-1, -1};
        gctINT32 lastUseIndex[2] = {-1, -1};
        gctINT i, j;

        if ((s0Type == 0x2)
        &&  (s1Type == 0x2)
        &&  (s2Type != 0x2)
        &&  ((s0Index != s1Index) || (s0Rela != s1Rela))
        )
        {
            /* Source 0 and 1 collision: Copy source 0 to temp. */
            gcmERR_BREAK(_TempEmit(Tree, CodeGen, States, 0, &newPhysical[0], &lastUseIndex[0]));
        }

        if ((s0Type == 0x2)
        &&  (s1Type != 0x2)
        &&  (s2Type == 0x2)
        &&  ((s0Index != s2Index) || (s0Rela != s2Rela))
        )
        {
            /* Source 0 and 2 collision: copy source 0 to temp. */
            gcmERR_BREAK(_TempEmit(Tree, CodeGen, States, 0, &newPhysical[0], &lastUseIndex[0]));
        }

        if ((s0Type != 0x2)
        &&  (s1Type == 0x2)
        &&  (s2Type == 0x2)
        &&  ((s1Index != s2Index) || (s1Rela != s2Rela))
        )
        {
            /* Source 1 and 2 collision: copy source 1 to temp. */
            gcmERR_BREAK(_TempEmit(Tree, CodeGen, States, 1, &newPhysical[0], &lastUseIndex[0]));
        }

        if ((s0Type == 0x2)
        &&  (s1Type == 0x2)
        &&  (s2Type == 0x2)
        )
        {
            /* Uniforms have different index. */
            if ((s0Index != s1Index) || (s0Index != s2Index) || (s1Index != s2Index))
            {
                if (s0Index == s1Index)
                {
                    /* Source 0/1 and 2 collision: copy source 2 to temp. */
                    gcmERR_BREAK(_TempEmit(Tree, CodeGen, States, 2, &newPhysical[0], &lastUseIndex[0]));

                    if (s0Rela != s1Rela)
                    {
                        /* Source 0 and 1 collision: copy source 1 to temp. */
                        gcmERR_BREAK(_TempEmit(Tree, CodeGen, States, 1, &newPhysical[1], &lastUseIndex[1]));
                    }
                }
                else if (s0Index == s2Index)
                {
                    /* Source 0/2 and 1 collision: copy source 1 to temp. */
                    gcmERR_BREAK(_TempEmit(Tree, CodeGen, States, 1, &newPhysical[0], &lastUseIndex[0]));

                    if (s0Rela != s2Rela)
                    {
                        /* Source 0 and 2 collision: copy source 2 to temp. */
                        gcmERR_BREAK(_TempEmit(Tree, CodeGen, States, 2, &newPhysical[1], &lastUseIndex[1]));
                    }
                }
                else if (s1Index == s2Index)
                {
                    /* Source 0 and 1/2 collision: copy source 0 to temp. */
                    gcmERR_BREAK(_TempEmit(Tree, CodeGen, States, 0, &newPhysical[0], &lastUseIndex[0]));

                    if (s1Rela != s2Rela)
                    {
                        /* Source 1 and 2 collision: copy source 2 to temp. */
                        gcmERR_BREAK(_TempEmit(Tree, CodeGen, States, 2, &newPhysical[1], &lastUseIndex[1]));
                    }
                }
                else
                {
                    /* Source 0, 1, and 2 collision: copy source 0 and 1 to temp. */
                    gcmERR_BREAK(_TempEmit(Tree, CodeGen, States, 0, &newPhysical[0], &lastUseIndex[0]));
                    gcmERR_BREAK(_TempEmit(Tree, CodeGen, States, 1, &newPhysical[1], &lastUseIndex[1]));
                }
            }
            /* Uniforms have same index but different relative. */
            else if (!(s0Rela == s1Rela && s0Rela == s2Rela))
            {
                if (s0Rela != s1Rela && s0Rela != s2Rela)
                {
                    gcmERR_BREAK(_TempEmit(Tree, CodeGen, States, 0, &newPhysical[0], &lastUseIndex[0]));
                }
                if (s1Rela != s0Rela && s1Rela != s2Rela)
                {
                    gcmERR_BREAK(_TempEmit(Tree, CodeGen, States, 1,
                                        newPhysical[0] == -1 ? &newPhysical[0] : &newPhysical[1],
                                        newPhysical[0] == -1 ? &lastUseIndex[0] : &lastUseIndex[1]));
                }
                if (s2Rela != s0Rela && s2Rela != s1Rela && s0Rela == s1Rela)
                {
                    gcmERR_BREAK(_TempEmit(Tree, CodeGen, States, 2,
                                        newPhysical[0] == -1 ? &newPhysical[0] : &newPhysical[1],
                                        newPhysical[0] == -1 ? &lastUseIndex[0] : &lastUseIndex[1]));
                }
            }
        }

        /* If we generate new register on TempEmit, we can reuse these temp register. */
        for (i = 0; i < 2; i++)
        {
            if (newPhysical[i] != -1)
            {
                for (j = 0; j < 4; j++)
                {
                    if ((CodeGen->registerUsage + newPhysical[i])->lastUse[j] == lastUseIndex[i])
                    {
                        (CodeGen->registerUsage + newPhysical[i])->lastUse[j] = gcvSL_AVAILABLE;
                    }
                }
            }
        }

        /*_AdjustTwoOrMoreIndexedInOneInstruction(Tree, CodeGen, States);*/

        /* Extract the current code array. */
        code = CodeGen->current->code;

        /* If there is no code array yet or the current code array is full,
           create a new code array. */
        if ((CodeGen->current->root == gcvNULL)
        ||  (code->count == code->maxCount)
        )
        {
            /* Allocate a new code array. */
            gctSIZE_T bytes = gcmSIZEOF(gcsSL_PHYSICAL_CODE) +
                           31 * gcmSIZEOF(code->states);
            gctPOINTER pointer = gcvNULL;

            gcmERR_BREAK(gcoOS_Allocate(gcvNULL, bytes, &pointer));

            code = pointer;

            /* Initilaize the code array. */
            code->next     = gcvNULL;
            code->maxCount = 32;
            code->count    = 0;

            /* Link in the code array to the end of the list. */
            if (CodeGen->current->root == gcvNULL)
            {
                CodeGen->current->root = code;
            }
            else
            {
                CodeGen->current->code->next = code;
            }

            /* Save the new code array. */
            CodeGen->current->code = code;
        }

        /* Assume we don't need to split the instruction. */
        again = gcvFALSE;

        opcode = (((((gctUINT32) (States[0])) >> (0 ? 5:0)) & ((gctUINT32) ((((1 ? 5:0) - (0 ? 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1)))))) )
               | (((((gctUINT32) (States[2])) >> (0 ? 16:16)) & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1)))))) ) << 6;

        /*
            Need set 4:4 for
            all ld/st/img_ld/img_st/atom instructions of PS when halti >= 4
        */
        if(CodeGen->hasHalti4 && (Tree->shader->type == gcSHADER_TYPE_FRAGMENT))
        {
            /* Set skip_for_helpers */
            switch(opcode)
            {
            case 0x33:
            case 0x7A:
            case 0x79:
            case 0x34:
            case 0x65:
            case 0x66:
            case 0x67:
            case 0x68:
            case 0x69:
            case 0x6A:
            case 0x6B:
            case 0x6C:
                {
                    gctUINT8 swizzle = (((((gctUINT32) (States[1])) >> (0 ? 10:3)) & ((gctUINT32) ((((1 ? 10:3) - (0 ? 10:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 10:3) - (0 ? 10:3) + 1)))))) );
                    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:3) - (0 ?
 10:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:3) - (0 ?
 10:3) + 1))))))) << (0 ?
 10:3))) | (((gctUINT32) ((gctUINT32) (swizzle | 0x10) & ((gctUINT32) ((((1 ?
 10:3) - (0 ?
 10:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ? 10:3)));
                    break;
                }
            default:
                break;
            }
        }

        switch (opcode)
        {
            /* Set DEST_VALID bit to 0 for STORE. */
        case 0x33:
            States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)));
            break;

        case 0x56:
            _SetValueType0FromFormat(gcSL_UINT32, States);
            break;

        case 0x0C:
        /* fall through */
        case 0x0D:
        /* fall through */
        case 0x11:
        /* fall through */
        case 0x12:
        /* fall through */
        case 0x21:
        /* fall through */
        case 0x23:
        /* fall through */
        case 0x22:
        /* fall through */
            enable  = (((((gctUINT32) (States[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) );
            swizzle = (((((gctUINT32) (States[3])) >> (0 ? 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1)))))) );
            orgSwizzle = swizzle;

            channelIdx = 0;
            while (!((enable >> channelIdx) & 0x01))
            {
                channelIdx ++;
            }

            startSwizzle = (swizzle >> (channelIdx*2)) & 0x3;
            for (i = 0; i < channelIdx; i ++)
            {
                swizzle &= ~(0x3 << (i*2));
                swizzle |= (startSwizzle << (i*2));
            }

            if (s2Type != 0x7)
            {
                if ((swizzle & 0x03) != ((swizzle >> 2) & 0x03))
                {
                    mask       = 0x1;
                    newSwizzle = (swizzle & 0xFC)
                               | (_ReplicateSwizzle(swizzle, 1) & 0x03);
                }
                else if ((swizzle & 0x03) != ((swizzle >> 4) & 0x03))
                {
                    mask       = 0x3;
                    newSwizzle = (swizzle & 0xF0)
                               | (_ReplicateSwizzle(swizzle, 2) & 0x0F);
                }
                else if ((swizzle & 0x03) != ((swizzle >> 6) & 0x03))
                {
                    mask       = 0x7;
                    newSwizzle = _ReplicateSwizzle(swizzle, 3);
                }
                else
                {
                    mask = 0x0;
                }
            }
            else
            {
                mask = 0x0;
            }

            if (loopCount == 0 && mask == 0 &&
                (enable == gcSL_ENABLE_X ||
                 enable == gcSL_ENABLE_Y ||
                 enable == gcSL_ENABLE_Z ||
                 enable == gcSL_ENABLE_W) &&
                 orgSwizzle != swizzle)
            {

                if (s2Type != 0x7)
                    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle(swizzle, 0)) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)));
            }
            else if (enable & mask)
            {
                States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable & mask) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));

                if (s2Type != 0x7)
                    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle(swizzle, 0)) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)));

                again   = gcvTRUE;
                enable &= ~mask;
                swizzle = newSwizzle;
            }
            break;

        case 0x64:
            enable   = (((((gctUINT32) (States[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) );

            if (s2Type != 0x7)
            {
                swizzle  = (((((gctUINT32) (States[3])) >> (0 ? 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1)))))) );
                handleSwizzle = gcvTRUE;
            }
            else
            {
                handleSwizzle = gcvFALSE;
            }
            if (s1Type != 0x7)
            {
                swizzle1 = (((((gctUINT32) (States[2])) >> (0 ? 24:17)) & ((gctUINT32) ((((1 ? 24:17) - (0 ? 24:17) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1)))))) );
                handleSwizzle1 = gcvTRUE;
            }
            else
            {
                handleSwizzle1 = gcvFALSE;
            }
            gcmASSERT(handleSwizzle || handleSwizzle1);

            if ((handleSwizzle  && (swizzle  & 0x03) != ((swizzle  >> 2) & 0x03))
            ||  (handleSwizzle1 && (swizzle1 & 0x03) != ((swizzle1 >> 2) & 0x03)))
            {
                mask        = 0x1;
                if (handleSwizzle)
                {
                    newSwizzle  = (swizzle & 0xFC)
                                | (_ReplicateSwizzle(swizzle, 1) & 0x03);
                }
                if (handleSwizzle1)
                {
                    newSwizzle1 = (swizzle1 & 0xFC)
                                | (_ReplicateSwizzle(swizzle1, 1) & 0x03);
                }
            }
            else if ((handleSwizzle  && (swizzle  & 0x03) != ((swizzle  >> 4) & 0x03))
                 ||  (handleSwizzle1 && (swizzle1 & 0x03) != ((swizzle1 >> 4) & 0x03)))
            {
                mask        = 0x3;
                if (handleSwizzle)
                {
                    newSwizzle  = (swizzle & 0xF0)
                                | (_ReplicateSwizzle(swizzle, 2) & 0x0F);
                }
                if (handleSwizzle1)
                {
                    newSwizzle1 = (swizzle1 & 0xF0)
                                | (_ReplicateSwizzle(swizzle1, 2) & 0x0F);
                }
            }
            else if ((handleSwizzle  && (swizzle  & 0x03) != ((swizzle  >> 6) & 0x03))
                 ||  (handleSwizzle1 && (swizzle1 & 0x03) != ((swizzle1 >> 6) & 0x03)))
            {
                mask        = 0x7;
                if (handleSwizzle)
                {
                    newSwizzle  = _ReplicateSwizzle(swizzle, 3);
                }
                if (handleSwizzle1)
                {
                    newSwizzle1 = _ReplicateSwizzle(swizzle1, 3);
                }
            }
            else
            {
                mask = 0x0;
            }

            if (enable & mask)
            {
                States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable & mask) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));

                if (s2Type != 0x7)
                    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle(swizzle, 0)) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)));

                if (s1Type != 0x7)
                    States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:17) - (0 ?
 24:17) + 1))))))) << (0 ?
 24:17))) | (((gctUINT32) ((gctUINT32) (_ReplicateSwizzle(swizzle1, 0)) & ((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1))))))) << (0 ? 24:17)));

                again    = gcvTRUE;
                enable  &= ~mask;
                swizzle  = newSwizzle;
                swizzle1 = newSwizzle1;
            }
            break;
        case 0x16:
            /* For non-scalar conditional branch, HW's normal branch is actually branch_all, however
               when condition opcode is NE, the logic should be branch_any. */
            if (((((((gctUINT32) (States[0])) >> (0 ? 10:6)) & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 10:6) - (0 ? 10:6) + 1)))))) ) == 0x06) &&
                ((s0Type != 0x7 &&
                  (((((gctUINT32) (States[1])) >> (0 ? 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1)))))) ) != gcSL_SWIZZLE_XXXX  &&
                  (((((gctUINT32) (States[1])) >> (0 ? 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1)))))) ) != gcSL_SWIZZLE_YYYY  &&
                  (((((gctUINT32) (States[1])) >> (0 ? 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1)))))) ) != gcSL_SWIZZLE_ZZZZ  &&
                  (((((gctUINT32) (States[1])) >> (0 ? 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1)))))) ) != gcSL_SWIZZLE_WWWW) ||
                 (s1Type != 0x7 &&
                  (((((gctUINT32) (States[2])) >> (0 ? 24:17)) & ((gctUINT32) ((((1 ? 24:17) - (0 ? 24:17) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1)))))) ) != gcSL_SWIZZLE_XXXX  &&
                  (((((gctUINT32) (States[2])) >> (0 ? 24:17)) & ((gctUINT32) ((((1 ? 24:17) - (0 ? 24:17) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1)))))) ) != gcSL_SWIZZLE_YYYY  &&
                  (((((gctUINT32) (States[2])) >> (0 ? 24:17)) & ((gctUINT32) ((((1 ? 24:17) - (0 ? 24:17) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1)))))) ) != gcSL_SWIZZLE_ZZZZ  &&
                  (((((gctUINT32) (States[2])) >> (0 ? 24:17)) & ((gctUINT32) ((((1 ? 24:17) - (0 ? 24:17) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1)))))) ) != gcSL_SWIZZLE_WWWW)))
            {
                swizzle = (((((gctUINT32) (States[1])) >> (0 ? 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1)))))) );
                swizzle1 = (((((gctUINT32) (States[2])) >> (0 ? 24:17)) & ((gctUINT32) ((((1 ? 24:17) - (0 ? 24:17) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1)))))) );

                for (i = 0; i < 4; i ++)
                {
                    states[0] = States[0];
                    states[1] = States[1];
                    states[2] = States[2];
                    states[3] = States[3];

                    if (s0Type != 0x7)
                    {
                        newSwizzle = _ReplicateSwizzle(swizzle, i);
                        states[1] = ((((gctUINT32) (states[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:22) - (0 ?
 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (newSwizzle) & ((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ? 29:22)));
                    }

                    if (s1Type != 0x7)
                    {
                        newSwizzle1 = _ReplicateSwizzle(swizzle1, i);
                        states[2] = ((((gctUINT32) (states[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:17) - (0 ?
 24:17) + 1))))))) << (0 ?
 24:17))) | (((gctUINT32) ((gctUINT32) (newSwizzle1) & ((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1))))))) << (0 ? 24:17)));
                    }

                    if (i > 0)
                    {
                        gcSL_BRANCH_LIST entry;
                        gctPOINTER pointer;

                        gcmERR_BREAK(gcoOS_Allocate(gcvNULL,
                                                    gcmSIZEOF(struct _gcSL_BRANCH_LIST),
                                                    &pointer));
                        entry = pointer;

                        entry->next   = Tree->branch;
                        entry->ip     = gcsCODE_GENERATOR_GetIP(CodeGen);
                        entry->target = Tree->branch->target;
                        entry->call   = Tree->branch->call;
                        entry->duplicatedT0T1 = Tree->branch->duplicatedT0T1;

                        Tree->branch  = entry;
                    }

                    gcmERR_BREAK(_FinalEmit(Tree, CodeGen, states, loopCount));
                }

                return gcvSTATUS_OK;
            }

        default:
            break;
        }

        if(CodeGen->hasUSC)
        {
            gctBOOL needTempReg = gcvFALSE;

            switch(opcode)
            {
            case 0x33:
            case 0x7A:
            case 0x35:
                if((((((gctUINT32) (States[3])) >> (0 ? 30:28)) & ((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1)))))) ) != 0x0 ||
                   (((((gctUINT32) (States[3])) >> (0 ? 27:25)) & ((gctUINT32) ((((1 ? 27:25) - (0 ? 27:25) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1)))))) ) != 0x0)
                   needTempReg = gcvTRUE;
                break;

            case 0x67:
                needTempReg = gcvTRUE;
                if(CodeGen->isDual16Shader &&
                   (((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28))) ||
                    ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))))
                {  /* USC constraint: If source2 is highp, ATOMIC destination must be highp */
                    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)));
                }
                break;

            case 0x65:
            case 0x66:
            case 0x68:
            case 0x69:
            case 0x6A:
            case 0x6B:
            case 0x6C:
                gcmASSERT((((((gctUINT32) (States[3])) >> (0 ? 3:3)) & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1)))))) ));
                swizzle = (((((gctUINT32) (States[3])) >> (0 ? 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1)))))) );
                swizzle = _ReplicateSwizzle(swizzle, 0);
                States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)));
                if(CodeGen->isDual16Shader &&
                   (((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28))) ||
                    ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))))
                {  /* USC constraint: If source2 is highp, ATOMIC destination must be highp */
                    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)));
                }
                break;

            default:
                break;
            }

            if(needTempReg)
            {
                gctINT i;

                lastUse = 0;
                if (_isHWRegisterAllocated(Tree->shader))
                {
                    physical = Tree->shader->RARegWaterMark;
                    _UpdateRATempReg(Tree, 1);
                    shift = 0;
                }
                else
                {
                    /* Allocate a temporary register. */
                    lastUse = (Tree->hints[CodeGen->nextSource - 1].lastUseForTemp == (gctINT) CodeGen->nextSource - 1) ?
                              gcvSL_TEMPORARY : Tree->hints[CodeGen->nextSource - 1].lastUseForTemp;
                    gcmERR_BREAK(_FindRegisterUsage(CodeGen->registerUsage,
                                            CodeGen->registerCount,
                                            gcSHADER_FLOAT_X4,
                                            1,
                                            lastUse /*gcvSL_TEMPORARY*/,
                                            gcvFALSE,
                                            (gctINT_PTR) &physical,
                                            &swizzle,
                                            &shift,
                                            gcvNULL,
                                            0));
                }
                if (((((((gctUINT32) (States[1])) >> (0 ? 11:11)) & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1)))))) )
                    && ((((((gctUINT32) (States[2])) >> (0 ? 5:3)) & ((gctUINT32) ((((1 ? 5:3) - (0 ? 5:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1)))))) ) == 0x0)
                    && ((((((gctUINT32) (States[1])) >> (0 ? 20:12)) & ((gctUINT32) ((((1 ? 20:12) - (0 ? 20:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1)))))) ) == physical)
                    )
                ||     ((((((gctUINT32) (States[2])) >> (0 ? 6:6)) & ((gctUINT32) ((((1 ? 6:6) - (0 ? 6:6) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1)))))) )
                    && ((((((gctUINT32) (States[3])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) ) == 0x0)
                    && ((((((gctUINT32) (States[2])) >> (0 ? 15:7)) & ((gctUINT32) ((((1 ? 15:7) - (0 ? 15:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:7) - (0 ? 15:7) + 1)))))) ) == physical)
                    )
                ||     ((((((gctUINT32) (States[3])) >> (0 ? 3:3)) & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1)))))) )
                    && ((((((gctUINT32) (States[3])) >> (0 ? 30:28)) & ((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1)))))) ) == 0x0)
                    && ((((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ) == physical)
                    )
                )
                {
                    if (_isHWRegisterAllocated(Tree->shader))
                    {
                        physical++;
                        _UpdateRATempReg(Tree, 2);
                        shift = 0;
                    }
                    else
                    {
                        gcmERR_BREAK(_FindRegisterUsage(CodeGen->registerUsage,
                                                CodeGen->registerCount,
                                                gcSHADER_FLOAT_X4,
                                                1,
                                                lastUse /*gcvSL_TEMPORARY*/,
                                                gcvFALSE,
                                                (gctINT_PTR) &physical,
                                                &swizzle,
                                                &shift,
                                                gcvNULL,
                                                0));
                    }
                }

                gcCGUpdateMaxRegister(CodeGen, physical, Tree);

                if(opcode == 0x67)
                {
                    gctUINT8 enable1 = 0;

                    gcmASSERT((((((gctUINT32) (0)) >> (0 ? 27:25)) & ((gctUINT32) ((((1 ? 27:25) - (0 ? 27:25) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1)))))) ) == 0x0);

                    /* Save States[0]. */
                    movState0 = States[0];

                    /* Set enable and swizzle. */
                    enable = (((((gctUINT32) (States[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) );
                    switch(enable)
                    {
                    case gcSL_ENABLE_X:
                    case gcSL_ENABLE_Y:
                        enable1 = gcSL_ENABLE_XY;
                        movSwizzle = gcSL_SWIZZLE_XXXX;
                        break;

                    case gcSL_ENABLE_Z:
                    case gcSL_ENABLE_W:
                        enable1 = gcSL_ENABLE_ZW;
                        movSwizzle = gcSL_SWIZZLE_ZZZZ;
                        break;

                    default:
                        enable1 = gcSL_ENABLE_XYZW;
                        movSwizzle = gcSL_SWIZZLE_XXXX;
                        break;
                    }

                    swizzle = (((((gctUINT32) (States[3])) >> (0 ? 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1)))))) );
                    swizzle = _ReplicateSwizzle2(swizzle, 0);
                    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (physical) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)));
                    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable1) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));
                    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)));

                    genExtraMov = gcvTRUE;
                }
                else
                {
                    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)));
                    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (physical) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)));
                }
                /* reuse the allocated temp register. */
                for (i = 0; i < 4; i++)
                {
                    if ((CodeGen->registerUsage + physical)->lastUse[i] == lastUse)
                    {
                        (CodeGen->registerUsage + physical)->lastUse[i] = gcvSL_AVAILABLE;
                    }
                }
            }
        }

        if (CodeGen->hasNEW_SIN_COS_LOG_DIV &&
            (opcode == 0x12 ||
             opcode == 0x23 ||
             opcode == 0x22 ||
             opcode == 0x64))
        {
            if (_isHWRegisterAllocated(Tree->shader))
            {
                physical = Tree->shader->RARegWaterMark + loopCount;
                _UpdateRATempReg(Tree, loopCount + 1);
                shift = 0;

                if (opcode == 0x64)
                {
                    if ((((((((gctUINT32) (States[3])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) ) == 0x0)
                      &&((((((gctUINT32) (States[2])) >> (0 ? 15:7)) & ((gctUINT32) ((((1 ? 15:7) - (0 ? 15:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:7) - (0 ? 15:7) + 1)))))) ) == physical - loopCount))
                      || (((((((gctUINT32) (States[3])) >> (0 ? 30:28)) & ((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1)))))) ) == 0x0)
                      &&((((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ) == physical - loopCount)))
                    {
                        physical++;
                        _UpdateRATempReg(Tree, loopCount + 2);
                    }
                }
                else
                {
                    if (((((((gctUINT32) (States[3])) >> (0 ? 30:28)) & ((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1)))))) ) == 0x0)
                      &&((((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ) == physical - loopCount))
                    {
                        physical++;
                        _UpdateRATempReg(Tree, loopCount + 2);
                    }
                }
            }
            else
            {
                /* Allocate a temporary register. */
                lastUse = (Tree->hints[CodeGen->nextSource - 1].lastUseForTemp == (gctINT) CodeGen->nextSource - 1) ?
                          gcvSL_TEMPORARY : Tree->hints[CodeGen->nextSource - 1].lastUseForTemp;
                gcmERR_BREAK(_FindRegisterUsage(CodeGen->registerUsage,
                                        CodeGen->registerCount,
                                        gcSHADER_FLOAT_X4,
                                        1,
                                        lastUse /*gcvSL_TEMPORARY*/,
                                        gcvFALSE,
                                        (gctINT_PTR) &physical,
                                        &swizzleTemp,
                                        &shift,
                                        gcvNULL,
                                        0));

                if (opcode == 0x64)
                {
                    gcmASSERT((((((gctUINT32) (States[2])) >> (0 ? 6:6)) & ((gctUINT32) ((((1 ? 6:6) - (0 ? 6:6) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1)))))) ));
                    gcmASSERT((((((gctUINT32) (States[3])) >> (0 ? 3:3)) & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1)))))) ));
                    while ((((((((gctUINT32) (States[3])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) ) == 0x0)
                      && ((((((gctUINT32) (States[2])) >> (0 ? 15:7)) & ((gctUINT32) ((((1 ? 15:7) - (0 ? 15:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:7) - (0 ? 15:7) + 1)))))) ) == physical))
                    ||  (((((((gctUINT32) (States[3])) >> (0 ? 30:28)) & ((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1)))))) ) == 0x0)
                      && ((((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ) == physical)))
                    {
                        gcmERR_BREAK(_FindRegisterUsage(CodeGen->registerUsage,
                                                CodeGen->registerCount,
                                                gcSHADER_FLOAT_X4,
                                                1,
                                                lastUse /*gcvSL_TEMPORARY*/,
                                                gcvFALSE,
                                                (gctINT_PTR) &physical,
                                                &swizzleTemp,
                                                &shift,
                                                gcvNULL,
                                                0));
                    }
                }
                else
                {
                    gcmASSERT((((((gctUINT32) (States[3])) >> (0 ? 3:3)) & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1)))))) ));
                    if (((((((gctUINT32) (States[3])) >> (0 ? 30:28)) & ((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1)))))) ) == 0x0)
                    &&  ((((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) ) == physical))
                    {
                        gcmERR_BREAK(_FindRegisterUsage(CodeGen->registerUsage,
                                                CodeGen->registerCount,
                                                gcSHADER_FLOAT_X4,
                                                1,
                                                lastUse /*gcvSL_TEMPORARY*/,
                                                gcvFALSE,
                                                (gctINT_PTR) &physical,
                                                &swizzleTemp,
                                                &shift,
                                                gcvNULL,
                                                0));
                    }
                }
            }

            gcCGUpdateMaxRegister(CodeGen, physical, Tree);

            /* Save States[0]. */
            states[0] = States[0];

            /* Change dest to two components. */
            States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (physical) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)));
            States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (gcSL_ENABLE_XY) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));
            States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)));

            /* Set this instruction's sat to be none, since now it generates the intermediate
               result. The final result is generated by mul.
               For example,
               div.rtz        r1.xy, r0.w, r0.y
               mul.sat        r0.y, r1.x, r1.y
            */
            States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));

            genExtraMul = gcvTRUE;
        }

        /* Copy the states into the code array. */
        gcoOS_MemCopy(code->states + code->count * 4,
                      States,
                      4 * sizeof(gctUINT32));

        /* Store location of states for future optimization. */
        CodeGen->previousCode = code->states + code->count * 4;

        /* Incease code counter and current instruction pointer. */
        ++code->count;
        ++CodeGen->current->ip;

        if(genExtraMov)
        {
            gctUINT32 movStates[4];

            /* Restore back States. */
            movStates[0] = movState0;

            /* MOV instruction to copy old value. */
            movStates[0] = ((((gctUINT32) (movStates[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));
            movStates[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));
            movStates[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));
            movStates[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (physical) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (movSwizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:25) - (0 ?
 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));
            gcmERR_BREAK(_FinalEmit(Tree, CodeGen, movStates, loopCount));
        }

        if (genExtraMul)
        {
            /* Restore back States. */
            States[0] = states[0];

            /* add extra multiply for division:

                original inst:  div r2.y, r0.x, r1.x

                ==>
                    div0 temp.xy, r0.x, r1.x
                    mul  r2.y, temp.x, temp.y
             */

            /* Add multiply. */
            states[0] = ((((gctUINT32) (states[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) (0x03 & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));
            states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:12) - (0 ?
 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (physical) & ((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ? 20:12)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:22) - (0 ?
 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (gcSL_SWIZZLE_XXXX) & ((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ? 29:22)));
            states[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:7) - (0 ?
 15:7) + 1))))))) << (0 ?
 15:7))) | (((gctUINT32) ((gctUINT32) (physical) & ((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:7) - (0 ? 15:7) + 1))))))) << (0 ? 15:7)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:27) - (0 ?
 29:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:27) - (0 ?
 29:27) + 1))))))) << (0 ?
 29:27))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 29:27) - (0 ?
 29:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:27) - (0 ? 29:27) + 1))))))) << (0 ? 29:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:17) - (0 ?
 24:17) + 1))))))) << (0 ?
 24:17))) | (((gctUINT32) ((gctUINT32) (gcSL_SWIZZLE_YYYY) & ((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1))))))) << (0 ? 24:17)));
            states[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)));

            gcmERR_BREAK(_FinalEmit(Tree, CodeGen, states, loopCount));
        }

        if (again && enable)
        {
            States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));
            if (s2Type != 0x7)
            {
                States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)));
            }
            if (opcode == 0x64
            &&  s1Type != 0x7)
            {
                States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:17) - (0 ?
 24:17) + 1))))))) << (0 ?
 24:17))) | (((gctUINT32) ((gctUINT32) (swizzle1) & ((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1))))))) << (0 ? 24:17)));
            }

            gcmERR_BREAK(_FinalEmit(Tree, CodeGen, States, loopCount+1));
        }

        /* Success. */
        status = gcvSTATUS_OK;
    }
    while (gcvFALSE);

    /* Return the status. */
    return status;
}

typedef enum _gceCONVERT_TYPE {
    gcvCONVERT_NONE,
    gcvCONVERT_EXTRA,
    gcvCONVERT_2COMPONENTS,
    gcvCONVERT_COMPONENTXY,
    gcvCONVERT_COMPONENTZW,
    gcvCONVERT_ROTATE,
    gcvCONVERT_LEADZERO,
    gcvCONVERT_LSHIFT,
    gcvCONVERT_DIVMOD,
    gcvCONVERT_HI,
    gcvCONVERT_LOAD,
    gcvCONVERT_STORE,
    gcvCONVERT_ATOMIC
}
gceCONVERT_TYPE;

static gceSTATUS
_SourceConvertEmit(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctUINT32 States[4],
    IN gctUINT ValueType,
    IN gctUINT Source,
    IN gceCONVERT_TYPE ConvertType
    )
{
    gceSTATUS status;
    gctUINT32 states[4] = {0};
    gctUINT physical = 0, physical1 = 0;
    gctUINT8 swizzle = 0, swizzle1  = 0;
    gctINT shift = 0, shift1 = 0;
    gctUINT8 enable = 0, enable1 = 0;
    gctINT constPhysical   = 0, constPhysical1 = 0;
    gctUINT8 constSwizzle  = 0, constSwizzle1 = 0;
    gctUINT sourcePhysical = 0;
    gctUINT8 sourceSwizzle = 0;
    gctUINT sourceRelative = 0;
    gctUINT sourceType     = 0;
    gctINT lastUse         = 0;
    gcSL_TYPE type = gcSL_NONE, type1 = gcSL_NONE;

    /* Get source info. */
    switch (Source)
    {
    case 0:
        sourcePhysical = (((((gctUINT32) (States[1])) >> (0 ? 20:12)) & ((gctUINT32) ((((1 ? 20:12) - (0 ? 20:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1)))))) );
        sourceRelative = (((((gctUINT32) (States[2])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) );
        sourceSwizzle  = (((((gctUINT32) (States[1])) >> (0 ? 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1)))))) );
        sourceType     = (((((gctUINT32) (States[2])) >> (0 ? 5:3)) & ((gctUINT32) ((((1 ? 5:3) - (0 ? 5:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1)))))) );
        break;

    case 1:
        sourcePhysical = (((((gctUINT32) (States[2])) >> (0 ? 15:7)) & ((gctUINT32) ((((1 ? 15:7) - (0 ? 15:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:7) - (0 ? 15:7) + 1)))))) );
        sourceRelative = (((((gctUINT32) (States[2])) >> (0 ? 29:27)) & ((gctUINT32) ((((1 ? 29:27) - (0 ? 29:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:27) - (0 ? 29:27) + 1)))))) );
        sourceSwizzle  = (((((gctUINT32) (States[2])) >> (0 ? 24:17)) & ((gctUINT32) ((((1 ? 24:17) - (0 ? 24:17) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1)))))) );
        sourceType     = (((((gctUINT32) (States[3])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) );
        break;

    case 2:
        sourcePhysical = (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) );
        sourceRelative = (((((gctUINT32) (States[3])) >> (0 ? 27:25)) & ((gctUINT32) ((((1 ? 27:25) - (0 ? 27:25) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1)))))) );
        sourceSwizzle  = (((((gctUINT32) (States[3])) >> (0 ? 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1)))))) );
        sourceType     = (((((gctUINT32) (States[3])) >> (0 ? 30:28)) & ((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1)))))) );
        break;

    default:
        /* Add redundant assignments to avoid warnings. */
        sourcePhysical =
        sourceRelative =
        sourceType     = 0;
        sourceSwizzle  = 0;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    lastUse = (Tree->hints[CodeGen->nextSource - 1].lastUseForTemp == (gctINT) CodeGen->nextSource - 1) ?
              gcvSL_TEMPORARY : Tree->hints[CodeGen->nextSource - 1].lastUseForTemp;

    /* Add conversion instructions. */
    switch (ConvertType)
    {
    case gcvCONVERT_ROTATE:
        if (_isHWRegisterAllocated(Tree->shader))
        {
            physical = Tree->shader->RARegWaterMark;
            _UpdateRATempReg(Tree, 1);
            shift = 0;
            enable = gcSL_ENABLE_X;
            swizzle = gcSL_SWIZZLE_XXXX;

            physical1 = physical+1;
            shift1 = 0;
            enable1 = gcSL_ENABLE_X;
            swizzle1 = gcSL_SWIZZLE_XXXX;
        }
        else
        {
        /* Allocate a temporary register. */
        gcmONERROR(_FindRegisterUsage(CodeGen->registerUsage,
                              CodeGen->registerCount,
                              gcSHADER_INTEGER_X1,
                              1,
                              lastUse /*gcvSL_TEMPORARY*/,
                              gcvFALSE,
                              (gctINT_PTR) &physical,
                              &swizzle,
                              &shift,
                              &enable,
                              0));

        /* Allocate another temporary register. */
        gcmONERROR(_FindRegisterUsage(CodeGen->registerUsage,
                              CodeGen->registerCount,
                              gcSHADER_INTEGER_X1,
                              1,
                              lastUse /*gcvSL_TEMPORARY*/,
                              gcvFALSE,
                              (gctINT_PTR) &physical1,
                              &swizzle1,
                              &shift1,
                              &enable1,
                              0));
        }
        gcCGUpdateMaxRegister(CodeGen, physical1, Tree);

        if (ValueType == 0x7
        ||  ValueType == 0x4)
        {
            gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                          CodeGen,
                                          0x000000FF,
                                          &constPhysical,
                                          &constSwizzle,
                                          &type));
            gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                          CodeGen,
                                          0x01010101,
                                          &constPhysical1,
                                          &constSwizzle1,
                                          &type1));
        }
        else
        {
            gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                          CodeGen,
                                          0x0000FFFF,
                                          &constPhysical,
                                          &constSwizzle,
                                          &type));
            gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                          CodeGen,
                                          0x00010001,
                                          &constPhysical1,
                                          &constSwizzle1,
                                          &type1));
        }

        states[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (physical) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));
        states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:12) - (0 ?
 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (sourcePhysical) & ((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ? 20:12)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:22) - (0 ?
 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (sourceSwizzle) & ((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ? 29:22)));
        states[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (sourceRelative) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (sourceType) & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));
        states[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (constPhysical) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (constSwizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:25) - (0 ?
 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (type == gcSL_UNIFORM ?
 0x2 : 0x0) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));

       _SetOpcode(states,
                  0x5D,
                  0x00,
                  gcvFALSE);

       _SetValueType0(0x5, states);

        gcmONERROR(_FinalEmit(Tree, CodeGen, states, 0));

        states[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (physical1) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable1) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));
        states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:12) - (0 ?
 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (physical) & ((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ? 20:12)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:22) - (0 ?
 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ? 29:22)));
        states[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:7) - (0 ?
 15:7) + 1))))))) << (0 ?
 15:7))) | (((gctUINT32) ((gctUINT32) (constPhysical1) & ((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:7) - (0 ? 15:7) + 1))))))) << (0 ? 15:7)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:17) - (0 ?
 24:17) + 1))))))) << (0 ?
 24:17))) | (((gctUINT32) ((gctUINT32) (constSwizzle1) & ((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1))))))) << (0 ? 24:17)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:27) - (0 ?
 29:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:27) - (0 ?
 29:27) + 1))))))) << (0 ?
 29:27))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 29:27) - (0 ?
 29:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:27) - (0 ? 29:27) + 1))))))) << (0 ? 29:27)));
        states[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (type1 == gcSL_UNIFORM ?
 0x2 : 0x0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)));

       _SetOpcode(states,
                  0x3C,
                  0x00,
                  gcvFALSE);

       _SetValueType0(0x5, states);

        gcmONERROR(_FinalEmit(Tree, CodeGen, states, 0));
        break;

    case gcvCONVERT_LEADZERO:
        if (_isHWRegisterAllocated(Tree->shader))
        {
            physical = Tree->shader->RARegWaterMark;
            _UpdateRATempReg(Tree, 1);
            shift = 0;
            enable = gcSL_ENABLE_X;
            swizzle = gcSL_SWIZZLE_XXXX;

            physical1 = physical+1;
            shift1 = 0;
            enable1 = gcSL_ENABLE_X;
            swizzle1 = gcSL_SWIZZLE_XXXX;
        }
        else
        {
        /* Allocate a temporary register. */
        gcmONERROR(_FindRegisterUsage(CodeGen->registerUsage,
                              CodeGen->registerCount,
                              gcSHADER_INTEGER_X1,
                              1,
                              lastUse /*gcvSL_TEMPORARY*/,
                              gcvFALSE,
                              (gctINT_PTR) &physical,
                              &swizzle,
                              &shift,
                              &enable,
                              0));

        /* Allocate another temporary register. */
        gcmONERROR(_FindRegisterUsage(CodeGen->registerUsage,
                              CodeGen->registerCount,
                              gcSHADER_INTEGER_X1,
                              1,
                              lastUse /*gcvSL_TEMPORARY*/,
                              gcvFALSE,
                              (gctINT_PTR) &physical1,
                              &swizzle1,
                              &shift1,
                              &enable1,
                              0));
        }
        gcCGUpdateMaxRegister(CodeGen, physical1, Tree);

        if (ValueType == 0x7
        ||  ValueType == 0x4)
        {
            gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                          CodeGen,
                                          24,
                                          &constPhysical,
                                          &constSwizzle,
                                          &type));
            gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                          CodeGen,
                                          0x00FFFFFF,
                                          &constPhysical1,
                                          &constSwizzle1,
                                          &type1));
        }
        else
        {
            gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                          CodeGen,
                                          16,
                                          &constPhysical,
                                          &constSwizzle,
                                          &type));
            gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                          CodeGen,
                                          0x0000FFFF,
                                          &constPhysical1,
                                          &constSwizzle1,
                                          &type1));
        }

        states[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (physical) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));
        states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:12) - (0 ?
 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (sourcePhysical) & ((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ? 20:12)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:22) - (0 ?
 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (sourceSwizzle) & ((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ? 29:22)));
        states[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (sourceRelative) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (sourceType) & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));
        states[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (constPhysical) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (constSwizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:25) - (0 ?
 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (type == gcSL_UNIFORM ?
 0x2 : 0x0) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));

       _SetOpcode(states,
                  0x59,
                  0x00,
                  gcvFALSE);

       _SetValueType0(0x5, states);

        gcmONERROR(_FinalEmit(Tree, CodeGen, states, 0));

        states[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (physical1) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable1) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));
        states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:12) - (0 ?
 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (physical) & ((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ? 20:12)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:22) - (0 ?
 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ? 29:22)));
        states[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));
        states[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (constPhysical1) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (constSwizzle1) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:25) - (0 ?
 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (type1 == gcSL_UNIFORM ?
 0x2 : 0x0) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));

       _SetOpcode(states,
                  0x5C,
                  0x00,
                  gcvFALSE);

       _SetValueType0(0x5, states);

        gcmONERROR(_FinalEmit(Tree, CodeGen, states, 0));
        break;

    case gcvCONVERT_LSHIFT:
        if (sourceType == 0x2)
        {
            /* Get constant. */
            gctUINT8 swizzle = sourceSwizzle & 0x3;
            gctINT value = 0;
            gcsSL_CONSTANT_TABLE_PTR c;

            for (c = CodeGen->constants; c != gcvNULL; c = c->next)
            {
                if (c->index == (gctINT) sourcePhysical)
                {
                    gctINT i;

                    for (i = 0; i < c->count; i++)
                    {
                        if (((c->swizzle >> (i * 2)) & 0x3) == swizzle)
                        {
                            value = *((int *) (&c->constant[i]));
                            break;
                        }
                    }
                    if (i < c->count)
                    {
                        break;
                    }
                }
            }

            /* Check if constant in the range. */
            if (ValueType == 0x7
            ||  ValueType == 0x4)
            {
                if (value < 8)
                {
                    /* No action needed. */
                    return gcvSTATUS_OK;
                }
            }
            else
            {
                if (value < 16)
                {
                    /* No action needed. */
                    return gcvSTATUS_OK;
                }
            }
        }

        if (_isHWRegisterAllocated(Tree->shader))
        {
            physical1 = Tree->shader->RARegWaterMark;
            _UpdateRATempReg(Tree, 1);
            shift1 = 0;
            enable1 = gcSL_ENABLE_X;
            swizzle1 = gcSL_SWIZZLE_XXXX;
        }
        else
        {
        /* Allocate a temporary register. */
        gcmONERROR(_FindRegisterUsage(CodeGen->registerUsage,
                              CodeGen->registerCount,
                              gcSHADER_INTEGER_X1,
                              1,
                              lastUse /*gcvSL_TEMPORARY*/,
                              gcvFALSE,
                              (gctINT_PTR) &physical1,
                              &swizzle1,
                              &shift1,
                              &enable1,
                              0));
        }
        gcCGUpdateMaxRegister(CodeGen, physical1, Tree);

        if (ValueType == 0x7
        ||  ValueType == 0x4)
        {
            gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                          CodeGen,
                                          0x7,
                                          &constPhysical,
                                          &constSwizzle,
                                          &type));
        }
        else
        {
            gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                          CodeGen,
                                          0xF,
                                          &constPhysical,
                                          &constSwizzle,
                                          &type));
        }

        states[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (physical1) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable1) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));
        states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:12) - (0 ?
 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (sourcePhysical) & ((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ? 20:12)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:22) - (0 ?
 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (sourceSwizzle) & ((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ? 29:22)));
        states[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (sourceRelative) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (sourceType) & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));
        states[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (constPhysical) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (constSwizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:25) - (0 ?
 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (type == gcSL_UNIFORM ?
 0x2 : 0x0) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));

       _SetOpcode(states,
                  0x5D,
                  0x00,
                  gcvFALSE);

       _SetValueType0(0x5, states);

        gcmONERROR(_FinalEmit(Tree, CodeGen, states, 0));
        break;

    case gcvCONVERT_DIVMOD:
        if (sourceSwizzle == gcSL_SWIZZLE_XXXX)
        {
            /* No action needed. */
            return gcvSTATUS_OK;
        }

        if (_isHWRegisterAllocated(Tree->shader))
        {
            physical1 = Tree->shader->RARegWaterMark;
            _UpdateRATempReg(Tree, 1);
            shift1 = 0;
        }
        else
        {
        /* Allocate a temporary register. */
        gcmONERROR(_FindRegisterUsage(CodeGen->registerUsage,
                              CodeGen->registerCount,
                              gcSHADER_INTEGER_X4,
                              1,
                              lastUse /*gcvSL_TEMPORARY*/,
                              gcvFALSE,
                              (gctINT_PTR) &physical1,
                              &swizzle1,
                              &shift1,
                              &enable1,
                              0));
        }

        /* Use x component only. */
        enable1 = gcSL_ENABLE_X;
        swizzle1 = gcSL_SWIZZLE_XXXX;

        gcCGUpdateMaxRegister(CodeGen, physical1, Tree);

        states[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (physical1) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable1) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));
        states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));
        states[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));
        states[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (sourcePhysical) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (sourceSwizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:25) - (0 ?
 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (sourceRelative) & ((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (sourceType) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));

       _SetOpcode(states,
                  0x09,
                  0x00,
                  gcvFALSE);

       _SetValueType0(0x5, states);

        gcmONERROR(_FinalEmit(Tree, CodeGen, states, 0));
        break;

#if _EXTRA_16BIT_CONVERSION_
    case gcvCONVERT_COMPONENTXY:
        if (_isHWRegisterAllocated(Tree->shader))
        {
            physical1 = Tree->shader->RARegWaterMark;
            _UpdateRATempReg(Tree, 1);
            shift1 = 0;
        }
        else
        {
        /* Allocate a temporary register. */
        gcmONERROR(_FindRegisterUsage(CodeGen->registerUsage,
                              CodeGen->registerCount,
                              gcSHADER_INTEGER_X4,
                              1,
                              lastUse /*gcvSL_TEMPORARY*/,
                              gcvFALSE,
                              (gctINT_PTR) &physical1,
                              &swizzle1,
                              &shift1,
                              &enable1,
                              0));
        }
        gcCGUpdateMaxRegister(CodeGen, physical1);

        /* Set enable and swizzle. */
        enable1 = (((((gctUINT32) (States[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) );
        switch (enable1)
        {
        case gcSL_ENABLE_X:
            swizzle1 = gcSL_SWIZZLE_XXXX;
            sourceSwizzle = _ReplicateSwizzle(sourceSwizzle, 0);
            break;

        case gcSL_ENABLE_Y:
            swizzle1 = gcSL_SWIZZLE_YYYY;
            sourceSwizzle = _ReplicateSwizzle(sourceSwizzle, 1);
            break;

        case gcSL_ENABLE_XY:
            swizzle1 = gcSL_SWIZZLE_XYYY;
            sourceSwizzle = _ReplicateSwizzle2(sourceSwizzle, 0);
            break;

        default:
            /* Error. */
            break;
        }

        states[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (physical1) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable1) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));
        states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));
        states[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));
        states[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (sourcePhysical) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (sourceSwizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:25) - (0 ?
 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (sourceRelative) & ((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (sourceType) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));

       _SetOpcode(states,
                  0x09,
                  0x00,
                  gcvFALSE);

       _SetValueType0(0x5, states);

        gcmONERROR(_FinalEmit(Tree, CodeGen, states, 0));
        break;

    case gcvCONVERT_COMPONENTZW:
        if (_isHWRegisterAllocated(Tree->shader))
        {
            physical1 = Tree->shader->RARegWaterMark;
            _UpdateRATempReg(Tree, 1);
            shift1 = 0;
        }
        else
        {
        /* Allocate a temporary register. */
        gcmONERROR(_FindRegisterUsage(CodeGen->registerUsage,
                              CodeGen->registerCount,
                              gcSHADER_INTEGER_X4,
                              1,
                              lastUse /*gcvSL_TEMPORARY*/,
                              gcvFALSE,
                              (gctINT_PTR) &physical1,
                              &swizzle1,
                              &shift1,
                              &enable1,
                              0));
        }

        gcCGUpdateMaxRegister(CodeGen, physical1);

        /* Set enable and swizzle. */
        enable1 = (((((gctUINT32) (States[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) );
        switch (enable1)
        {
        case gcSL_ENABLE_Z:
            swizzle1 = gcSL_SWIZZLE_ZZZZ;
            sourceSwizzle = _ReplicateSwizzle(sourceSwizzle, 2);
            break;

        case gcSL_ENABLE_W:
            swizzle1 = gcSL_SWIZZLE_WWWW;
            sourceSwizzle = _ReplicateSwizzle(sourceSwizzle, 3);
            break;

        case gcSL_ENABLE_ZW:
            swizzle1 = gcSL_SWIZZLE_XYZW;
            sourceSwizzle = _ReplicateSwizzle2(sourceSwizzle, 1);
            break;

        default:
            /* Error. */
            break;
        }

        states[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (physical1) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable1) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));
        states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));
        states[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));
        states[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (sourcePhysical) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (sourceSwizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:25) - (0 ?
 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (sourceRelative) & ((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (sourceType) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));

       _SetOpcode(states,
                  0x09,
                  0x00,
                  gcvFALSE);

       _SetValueType0(0x5, states);

        gcmONERROR(_FinalEmit(Tree, CodeGen, states, 0));
        break;
#endif


    default:
        return gcvSTATUS_MISMATCH;
    }

    /* Modify sourceX. */
    switch (Source)
    {
    case 0:
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:12) - (0 ?
 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (physical1) & ((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ? 20:12)));
        States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:22) - (0 ?
 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (swizzle1) & ((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ? 29:22)));
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)));
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)));
        break;

    case 1:
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:7) - (0 ?
 15:7) + 1))))))) << (0 ?
 15:7))) | (((gctUINT32) ((gctUINT32) (physical1) & ((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:7) - (0 ? 15:7) + 1))))))) << (0 ? 15:7)));
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:17) - (0 ?
 24:17) + 1))))))) << (0 ?
 24:17))) | (((gctUINT32) ((gctUINT32) (swizzle1) & ((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1))))))) << (0 ? 24:17)));
        States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:27) - (0 ?
 29:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:27) - (0 ?
 29:27) + 1))))))) << (0 ?
 29:27))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 29:27) - (0 ?
 29:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:27) - (0 ? 29:27) + 1))))))) << (0 ? 29:27)));
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)));
        break;

    case 2:
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (physical1) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)));
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (swizzle1) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)));
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:25) - (0 ?
 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)));
        States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));
        break;

    default:
        break;
    }

    /* Success. */
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    return status;
}

static gctUINT8
_Enable2SwizzleWShift(
    IN gctUINT32 Enable
    )
{
    switch (Enable)
    {
    case gcSL_ENABLE_X:
        return gcSL_SWIZZLE_XXXX;

    case gcSL_ENABLE_Y:
        return gcSL_SWIZZLE_YYYY;

    case gcSL_ENABLE_Z:
        return gcSL_SWIZZLE_ZZZZ;

    case gcSL_ENABLE_W:
        return gcSL_SWIZZLE_WWWW;

    case gcSL_ENABLE_XY:
        return gcSL_SWIZZLE_XYYY;

    case gcSL_ENABLE_XZ:
        return gcSL_SWIZZLE_XZZZ;

    case gcSL_ENABLE_XW:
        return gcSL_SWIZZLE_XWWW;

    case gcSL_ENABLE_YZ:
        return gcSL_SWIZZLE_YYZZ;

    case gcSL_ENABLE_YW:
        return gcSL_SWIZZLE_YYWW;

    case gcSL_ENABLE_ZW:
        return gcSL_SWIZZLE_ZZZW;

    case gcSL_ENABLE_XYZ:
        return gcSL_SWIZZLE_XYZZ;

    case gcSL_ENABLE_XYW:
        return gcSL_SWIZZLE_XYWW;

    case gcSL_ENABLE_XZW:
        return gcSL_SWIZZLE_XZZW;

    case gcSL_ENABLE_YZW:
        return gcSL_SWIZZLE_YYZW;

    case gcSL_ENABLE_XYZW:
        return gcSL_SWIZZLE_XYZW;

    default:
        break;

    }

    gcmFATAL("ERROR: Invalid enable 0x%04X", Enable);
    return gcSL_SWIZZLE_XYZW;
}

static gceSTATUS
_TargetConvertEmit(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctUINT32 States[4],
    IN gctUINT ValueType,
    IN gceCONVERT_TYPE ConvertType,
    IN gctBOOL Saturate
    )
{
    gceSTATUS status;
    gctBOOL isSigned = gcvTRUE;
    gctUINT32 states[4];
    gctUINT physical;
    gctUINT8 swizzle;
    gctINT shift = 0;
    gctUINT8 enable;
    gctINT constPhysical, constPhysical1;
    gctUINT8 constSwizzle, constSwizzle1;
    gctUINT targetPhysical;
    gctUINT targetRelative;
    gctUINT8 targetEnable;
    gctINT lastUse;
    gcSL_TYPE type, type1;

    /* Get original target info. */
    targetPhysical = (((((gctUINT32) (States[0])) >> (0 ? 22:16)) & ((gctUINT32) ((((1 ? 22:16) - (0 ? 22:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1)))))) );
    targetRelative = (((((gctUINT32) (States[0])) >> (0 ? 15:13)) & ((gctUINT32) ((((1 ? 15:13) - (0 ? 15:13) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1)))))) );
    targetEnable   = (((((gctUINT32) (States[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) );

    if (ConvertType == gcvCONVERT_LOAD)
    {
        if(CodeGen->clShader && !CodeGen->hasBugFixes10)
        {
            /* Change target to reservedRegForLoad. */
            States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (CodeGen->reservedRegForLoad) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)));
            States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)));

            /* Output the modified original instruction. */
            gcmONERROR(_FinalEmit(Tree, CodeGen, States, 0));

            /* Check if this extra MOV can be eliminated. */
            if (Tree->hints[CodeGen->nextSource - 1].lastLoadUser < 0)
            {
                states[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (targetPhysical) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (targetRelative) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (targetEnable) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));
                states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));
                states[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));
                states[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (CodeGen->reservedRegForLoad) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_Enable2SwizzleWShift(targetEnable)) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:25) - (0 ?
 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));

                _SetOpcode(states,
                           0x09,
                           0x00,
                           gcvFALSE);

                /*_SetValueType0(0x5, states);*/

                gcmONERROR(_FinalEmit(Tree, CodeGen, states, 0));
            }
            else
            {
                /* Change temp register assign to reservedRegForLoad. */
                gctUINT index = Tree->hints[CodeGen->nextSource - 1].loadDestIndex;
                CodeGen->loadDestIndex = index;
                gcmASSERT(Tree->tempArray[index].assigned == (gctINT) targetPhysical);
                gcmASSERT(CodeGen->origAssigned < 0);
                gcmASSERT(CodeGen->lastLoadUser < 0);
                CodeGen->origAssigned = Tree->tempArray[index].assigned;
                Tree->tempArray[index].assigned = CodeGen->reservedRegForLoad;
                CodeGen->lastLoadUser = Tree->hints[CodeGen->nextSource - 1].lastLoadUser;
            }
        }
        else
        {
            /* Output the original instruction. */
            gcmONERROR(_FinalEmit(Tree, CodeGen, States, 0));
        }

        /* Success. */
        return gcvSTATUS_OK;
    }

    if (ConvertType == gcvCONVERT_ATOMIC)
    {
        /* An extra MOV instruction is added. */

        /* Output the original instruction. */
        gcmONERROR(_FinalEmit(Tree, CodeGen, States, 0));

        /* Add extra MOV instruction. */
        states[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (targetPhysical) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (targetRelative) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (targetEnable) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));
        states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));
        states[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));
        states[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (targetPhysical) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_Enable2SwizzleWShift(targetEnable)) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:25) - (0 ?
 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (targetRelative) & ((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));

        _SetOpcode(states,
                   0x09,
                   0x00,
                   gcvFALSE);

        /*_SetValueType0(0x5, states);*/

        gcmONERROR(_FinalEmit(Tree, CodeGen, states, 0));

        /* Success. */
        return gcvSTATUS_OK;
    }

    /* For GC2100/GC880 only. */
    gcmASSERT(CodeGen->isCL_X);

    /* Allocate a temporary register. */
    if (_isHWRegisterAllocated(Tree->shader))
    {
        physical = Tree->shader->RARegWaterMark;
        _UpdateRATempReg(Tree, 1);
        shift = 0;
        enable = gcSL_ENABLE_X;
        swizzle = gcSL_SWIZZLE_XXXX;
    }
    else
    {
    lastUse = (Tree->hints[CodeGen->nextSource - 1].lastUseForTemp == (gctINT) CodeGen->nextSource - 1) ?
              gcvSL_TEMPORARY : Tree->hints[CodeGen->nextSource - 1].lastUseForTemp;
    gcmONERROR(_FindRegisterUsage(CodeGen->registerUsage,
                          CodeGen->registerCount,
                          gcSHADER_INTEGER_X1,
                          1,
                          lastUse /*gcvSL_TEMPORARY*/,
                          gcvFALSE,
                          (gctINT_PTR) &physical,
                          &swizzle,
                          &shift,
                          &enable,
                          0));
    }
    gcCGUpdateMaxRegister(CodeGen, physical, Tree);

    /* Add conversion instruction(s). */
    if (Saturate)
    {
        gctUINT32 opcode = (((((gctUINT32) (States[0])) >> (0 ? 5:0)) & ((gctUINT32) ((((1 ? 5:0) - (0 ? 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1)))))) )
                         | (((((gctUINT32) (States[2])) >> (0 ? 16:16)) & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1)))))) ) << 6;

        /* GC2100 does not support IADDSAT, IMULLOSAT0, IMADLOSAT0, and IMADHISAT0. */
        /* Use ADD, IMULLO0, and IMADLO0 with post processing to implement these instructions. */

        /* Change opcodes to supported opcodes. */
        switch (opcode)
        {
        case 0x3B:
            States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (0x01) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));
            break;

        case 0x3E:
            States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (0x3C) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));
            break;

        case 0x4E:
            States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (0x4C & 0x3F) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));
            States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (0x4C >> 6) & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)));
            break;

        case 0x52:
            States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (0x50 & 0x3F) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));
            States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (0x50 >> 6) & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)));
            break;

        }

        if (ConvertType == gcvCONVERT_NONE)
        {
            if (ValueType == 0x7
            ||  ValueType == 0x6)
            {
                /* Need to add one instruction, so change target to temp. */
                States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (physical) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)));
                States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)));
                States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));
                isSigned = gcvFALSE;
            }

            /* Output the modified original instruction. */
            gcmONERROR(_FinalEmit(Tree, CodeGen, States, 0));

            if (isSigned)
            {
                if (ValueType == 0x2)
                {
                }
                else
                {
                    if (ValueType == 0x4)
                    {
                        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                                      CodeGen,
                                                      0x0000007F,
                                                      &constPhysical,
                                                      &constSwizzle,
                                                      &type));
                        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                                      CodeGen,
                                                      0xFFFFFF80,
                                                      &constPhysical1,
                                                      &constSwizzle1,
                                                      &type1));
                    }
                    else
                    {
                        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                                      CodeGen,
                                                      0x00007FFF,
                                                      &constPhysical,
                                                      &constSwizzle,
                                                      &type));
                        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                                      CodeGen,
                                                      0xFFFF8000,
                                                      &constPhysical1,
                                                      &constSwizzle1,
                                                      &type1));
                    }

                    states[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (physical) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));
                    states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:12) - (0 ?
 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (targetPhysical) & ((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ? 20:12)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:22) - (0 ?
 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (_SingleEnable2Swizzle(targetEnable)) & ((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ? 29:22)));
                    states[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (targetRelative) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:7) - (0 ?
 15:7) + 1))))))) << (0 ?
 15:7))) | (((gctUINT32) ((gctUINT32) (constPhysical) & ((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:7) - (0 ? 15:7) + 1))))))) << (0 ? 15:7)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:17) - (0 ?
 24:17) + 1))))))) << (0 ?
 24:17))) | (((gctUINT32) ((gctUINT32) (constSwizzle) & ((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1))))))) << (0 ? 24:17)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:27) - (0 ?
 29:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:27) - (0 ?
 29:27) + 1))))))) << (0 ?
 29:27))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 29:27) - (0 ?
 29:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:27) - (0 ? 29:27) + 1))))))) << (0 ? 29:27)));
                    states[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (type == gcSL_UNIFORM ?
 0x2 : 0x0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (targetPhysical) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_SingleEnable2Swizzle(targetEnable)) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:25) - (0 ?
 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (targetRelative) & ((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));

                    _SetOpcode(states,
                               0x0F,
                               0x01,
                               gcvFALSE);

                    _SetValueType0(0x2, states);

                    gcmONERROR(_FinalEmit(Tree, CodeGen, states, 0));

                    states[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (targetPhysical) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (targetRelative) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (targetEnable) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));
                    states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:12) - (0 ?
 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (physical) & ((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ? 20:12)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:22) - (0 ?
 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ? 29:22)));
                    states[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:7) - (0 ?
 15:7) + 1))))))) << (0 ?
 15:7))) | (((gctUINT32) ((gctUINT32) (constPhysical1) & ((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:7) - (0 ? 15:7) + 1))))))) << (0 ? 15:7)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:17) - (0 ?
 24:17) + 1))))))) << (0 ?
 24:17))) | (((gctUINT32) ((gctUINT32) (constSwizzle1) & ((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1))))))) << (0 ? 24:17)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:27) - (0 ?
 29:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:27) - (0 ?
 29:27) + 1))))))) << (0 ?
 29:27))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 29:27) - (0 ?
 29:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:27) - (0 ? 29:27) + 1))))))) << (0 ? 29:27)));
                    states[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (type1 == gcSL_UNIFORM ?
 0x2 : 0x0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (physical) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:25) - (0 ?
 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));

                    _SetOpcode(states,
                               0x0F,
                               0x02,
                               gcvFALSE);

                    _SetValueType0(0x2, states);

                    gcmONERROR(_FinalEmit(Tree, CodeGen, states, 0));
                }
            }
            else
            {
                gctUINT modifier = (((((gctUINT32) (States[3])) >> (0 ? 22:22)) & ((gctUINT32) ((((1 ? 22:22) - (0 ? 22:22) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 22:22) - (0 ? 22:22) + 1)))))) );

                if (ValueType == 0x5)
                {
                }
                else if (modifier == 0)
                {
                    if (ValueType == 0x7)
                    {

                        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                                      CodeGen,
                                                      0x000000FF,
                                                      &constPhysical,
                                                      &constSwizzle,
                                                      &type));
                    }
                    else
                    {
                        gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                                      CodeGen,
                                                      0x0000FFFF,
                                                      &constPhysical,
                                                      &constSwizzle,
                                                      &type));
                    }

                    states[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (targetPhysical) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (targetRelative) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (targetEnable) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));
                    states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:12) - (0 ?
 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (physical) & ((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ? 20:12)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:22) - (0 ?
 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ? 29:22)));
                    states[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:7) - (0 ?
 15:7) + 1))))))) << (0 ?
 15:7))) | (((gctUINT32) ((gctUINT32) (constPhysical) & ((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:7) - (0 ? 15:7) + 1))))))) << (0 ? 15:7)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:17) - (0 ?
 24:17) + 1))))))) << (0 ?
 24:17))) | (((gctUINT32) ((gctUINT32) (constSwizzle) & ((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1))))))) << (0 ? 24:17)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:27) - (0 ?
 29:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:27) - (0 ?
 29:27) + 1))))))) << (0 ?
 29:27))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 29:27) - (0 ?
 29:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:27) - (0 ? 29:27) + 1))))))) << (0 ? 29:27)));
                    states[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (type == gcSL_UNIFORM ?
 0x2 : 0x0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (physical) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:25) - (0 ?
 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));

                    _SetOpcode(states,
                               0x0F,
                               0x01,
                               gcvFALSE);

                    _SetValueType0(0x5, states);

                    gcmONERROR(_FinalEmit(Tree, CodeGen, states, 0));
                }
                else
                {
                    gctUINT sourcePhysical0 = (((((gctUINT32) (States[1])) >> (0 ? 20:12)) & ((gctUINT32) ((((1 ? 20:12) - (0 ? 20:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1)))))) );
                    gctUINT sourceSwizzle0  = (((((gctUINT32) (States[1])) >> (0 ? 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1)))))) );
                    gctUINT sourceRelative0 = (((((gctUINT32) (States[2])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) );
                    gctUINT sourceType0     = (((((gctUINT32) (States[2])) >> (0 ? 5:3)) & ((gctUINT32) ((((1 ? 5:3) - (0 ? 5:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1)))))) );
                    gctUINT sourcePhysical2 = (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) );
                    gctUINT sourceSwizzle2  = (((((gctUINT32) (States[3])) >> (0 ? 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1)))))) );
                    gctUINT sourceRelative2 = (((((gctUINT32) (States[3])) >> (0 ? 27:25)) & ((gctUINT32) ((((1 ? 27:25) - (0 ? 27:25) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1)))))) );
                    gctUINT sourceType2     = (((((gctUINT32) (States[3])) >> (0 ? 30:28)) & ((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1)))))) );

                    gcmASSERT(opcode == 0x3B);

                    states[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (targetPhysical) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (targetRelative) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (targetEnable) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));
                    states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:12) - (0 ?
 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (sourcePhysical0) & ((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ? 20:12)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:22) - (0 ?
 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (sourceSwizzle0) & ((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ? 29:22)));
                    states[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (sourceRelative0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (sourceType0) & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:7) - (0 ?
 15:7) + 1))))))) << (0 ?
 15:7))) | (((gctUINT32) ((gctUINT32) (sourcePhysical2) & ((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:7) - (0 ? 15:7) + 1))))))) << (0 ? 15:7)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:17) - (0 ?
 24:17) + 1))))))) << (0 ?
 24:17))) | (((gctUINT32) ((gctUINT32) (sourceSwizzle2) & ((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1))))))) << (0 ? 24:17)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:27) - (0 ?
 29:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:27) - (0 ?
 29:27) + 1))))))) << (0 ?
 29:27))) | (((gctUINT32) ((gctUINT32) (sourceRelative2) & ((gctUINT32) ((((1 ?
 29:27) - (0 ?
 29:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:27) - (0 ? 29:27) + 1))))))) << (0 ? 29:27)));
                    states[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (sourceType2) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (physical) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:25) - (0 ?
 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));

                    _SetOpcode(states,
                               0x31,
                               0x01,
                               gcvFALSE);

                    _SetValueType0(0x5, states);

                    gcmONERROR(_FinalEmit(Tree, CodeGen, states, 0));
                }
            }
        }
        else
        {
            gcmASSERT(ConvertType == gcvCONVERT_HI);
            gcmASSERT(opcode == 0x52);

            /* Change hi opcode to lo opcode. */
            States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (0x4C & 0x3F) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));
            States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (0x4C >> 6) & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)));

            /* Output the modified original instruction. */
            gcmONERROR(_FinalEmit(Tree, CodeGen, States, 0));

            gcmASSERT(ConvertType != gcvCONVERT_HI);
        }
    }
    else
    {
        if (ValueType == 0x7
        ||  ValueType == 0x6
        ||  ConvertType == gcvCONVERT_HI)
        {
            /* Need to add one instruction, so change target to temp. */
            States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (physical) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)));
            States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)));
            States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));
            isSigned = gcvFALSE;
        }

        if (ConvertType == gcvCONVERT_NONE)
        {
            /* Output the (maybe modified) original instruction. */
            gcmONERROR(_FinalEmit(Tree, CodeGen, States, 0));

            if (isSigned)
            {
                if (ValueType == 0x4)
                {
                    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                                  CodeGen,
                                                  24,
                                                  &constPhysical,
                                                  &constSwizzle,
                                                  &type));
                }
                else
                {
                    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                                  CodeGen,
                                                  16,
                                                  &constPhysical,
                                                  &constSwizzle,
                                                  &type));
                }

                states[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (physical) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));
                states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:12) - (0 ?
 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (targetPhysical) & ((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ? 20:12)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:22) - (0 ?
 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (_SingleEnable2Swizzle(targetEnable)) & ((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ? 29:22)));
                states[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (targetRelative) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));
                states[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (constPhysical) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (constSwizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:25) - (0 ?
 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (type == gcSL_UNIFORM ?
 0x2 : 0x0) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));

                _SetOpcode(states,
                           0x59,
                           0x00,
                           gcvFALSE);

                _SetValueType0(0x2, states);

                gcmONERROR(_FinalEmit(Tree, CodeGen, states, 0));

                states[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (targetPhysical) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (targetRelative) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (targetEnable) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));
                states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:12) - (0 ?
 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (physical) & ((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ? 20:12)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:22) - (0 ?
 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ? 29:22)));
                states[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));
                states[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (constPhysical) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (constSwizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:25) - (0 ?
 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));

                _SetOpcode(states,
                           0x5A,
                           0x00,
                           gcvFALSE);

                _SetValueType0(0x2, states);

                gcmONERROR(_FinalEmit(Tree, CodeGen, states, 0));
            }
            else
            {
                if (ValueType == 0x7)
                {
                    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                                  CodeGen,
                                                  0x000000FF,
                                                  &constPhysical,
                                                  &constSwizzle,
                                                  &type));
                }
                else
                {
                    gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                                  CodeGen,
                                                  0x0000FFFF,
                                                  &constPhysical,
                                                  &constSwizzle,
                                                  &type));
                }

                states[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (targetPhysical) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (targetRelative) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (targetEnable) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));
                states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:12) - (0 ?
 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (physical) & ((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ? 20:12)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:22) - (0 ?
 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ? 29:22)));
                states[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));
                states[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (constPhysical) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (constSwizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:25) - (0 ?
 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (type == gcSL_UNIFORM ?
 0x2 : 0x0) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));

                _SetOpcode(states,
                           0x5D,
                           0x00,
                           gcvFALSE);

                _SetValueType0(0x5, states);

                gcmONERROR(_FinalEmit(Tree, CodeGen, states, 0));
            }
        }
        else
        {
            gctUINT32 opcode = (((((gctUINT32) (States[0])) >> (0 ? 5:0)) & ((gctUINT32) ((((1 ? 5:0) - (0 ? 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1)))))) )
                             | (((((gctUINT32) (States[2])) >> (0 ? 16:16)) & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1)))))) ) << 6;

            gcmASSERT(ConvertType == gcvCONVERT_HI);

            /* Change hi opcode to lo opcode. */
            switch (opcode)
            {
            case 0x40:
                States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (0x3C & 0x3F) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));
                States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (0x3C >> 6) & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)));
                break;

            case 0x50:
                States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (0x4C & 0x3F) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));
                States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (0x4C >> 6) & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)));
                break;

            }

            /* Output the (maybe modified) original instruction. */
            gcmONERROR(_FinalEmit(Tree, CodeGen, States, 0));

            if (ValueType == 0x4
            ||  ValueType == 0x7)
            {
                gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                              CodeGen,
                                              8,
                                              &constPhysical,
                                              &constSwizzle,
                                              &type));
            }
            else
            {
                gcmVERIFY_OK(_AddConstantIVec1(Tree,
                                              CodeGen,
                                              16,
                                              &constPhysical,
                                              &constSwizzle,
                                              &type));
            }

            states[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (targetPhysical) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (targetRelative) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (targetEnable) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));
            states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:12) - (0 ?
 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (physical) & ((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ? 20:12)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:22) - (0 ?
 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ? 29:22)));
            states[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));
            states[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (constPhysical) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (constSwizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:25) - (0 ?
 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (type == gcSL_UNIFORM ?
 0x2 : 0x0) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));

            _SetOpcode(states,
                       0x5A,
                       0x00,
                       gcvFALSE);

            if (ValueType == 0x4
            ||  ValueType == 0x3)
            {
                _SetValueType0(0x2, states);
            }
            else
            {
                _SetValueType0(0x5, states);
            }

            gcmONERROR(_FinalEmit(Tree, CodeGen, states, 0));
        }
    }

    /* Success. */
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    return status;
}

static gceSTATUS
_ExtraEmit(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctUINT32 States[4]
    )
{
    gceSTATUS status;
    gctUINT instType0 = (((((gctUINT32) (States[1])) >> (0 ? 21:21)) & ((gctUINT32) ((((1 ? 21:21) - (0 ? 21:21) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 21:21) - (0 ? 21:21) + 1)))))) );
    gctUINT instType1 = (((((gctUINT32) (States[2])) >> (0 ? 31:30)) & ((gctUINT32) ((((1 ? 31:30) - (0 ? 31:30) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:30) - (0 ? 31:30) + 1)))))) );
    gctUINT valueType0 = instType0 | (instType1 << 1);
    gctUINT32 opcode = (((((gctUINT32) (States[0])) >> (0 ? 5:0)) & ((gctUINT32) ((((1 ? 5:0) - (0 ? 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1)))))) )
                     | (((((gctUINT32) (States[2])) >> (0 ? 16:16)) & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1)))))) ) << 6;

    if (valueType0 == 0x0
    ||  valueType0 == 0x2
    ||  valueType0 == 0x5)
    {
        /* GC2100 does not support IADDSAT, IMULLOSAT0, IMADLOSAT0, and IMADHISAT0. */
        /* Need to use implement them by other instructions. */
        switch (opcode)
        {
        case 0x32:
            break;

        case 0x3B:
        case 0x3E:
        case 0x4E:
        case 0x52:
            if (CodeGen->hasBugFixes11)
            {
                return _FinalEmit(Tree, CodeGen, States, 0);
            }
            break;

        case 0x65:
        case 0x66:
        case 0x67:
        case 0x68:
        case 0x69:
        case 0x6A:
        case 0x6B:
        case 0x6C:
            /* Need to add extra dependency to delay following STORE instructions. */
            return _TargetConvertEmit(Tree, CodeGen, States, valueType0, gcvCONVERT_ATOMIC, gcvFALSE);
            break;

        default:
            return _FinalEmit(Tree, CodeGen, States, 0);
        }
    }

    /* IDIV0 and IMOD0 should use 16-bit value types. */
    if (opcode == 0x44
    ||  opcode == 0x48)
    {
        if (CodeGen->isCL_X)
        {
            /* Change value type to 16-bit. */
            if (valueType0 == 0x4)
            {
                _SetValueType0(0x3, States);
            }
            else if (valueType0 == 0x7)
            {
                _SetValueType0(0x6, States);
            }
        }

        /* Sources must be x component for DIV/MOD. */
        gcmONERROR(_SourceConvertEmit(Tree, CodeGen, States, valueType0, 0, gcvCONVERT_DIVMOD));
        gcmONERROR(_SourceConvertEmit(Tree, CodeGen, States, valueType0, 1, gcvCONVERT_DIVMOD));

        if (! CodeGen->isCL_X
        ||  valueType0 == 0x7
        ||  valueType0 == 0x6)
        {
            return _FinalEmit(Tree, CodeGen, States, 0);
        }
        /* else fall through. */
    }
    else

    /* All other integer instructions should use 32-bit value types, except LOAD/STORE. */
    if (opcode != 0x32
    &&  opcode != 0x33)
    {
        /* Change value type to 32-bit. */
        if (valueType0 == 0x4
        ||  valueType0 == 0x3)
        {
            _SetValueType0(0x2, States);
        }
        else if (valueType0 == 0x7
             ||  valueType0 == 0x6)
        {
            _SetValueType0(0x5, States);
        }
    }

    /* Add source conversion. */
    switch (opcode)
    {
    case 0x5B:
        /*
         * Need to add conversion for SRC0 before execution:
         * For  8-bit data, AND 0x000000FF and then multiply 0x01010101.
         * For 16-bit data, AND 0x0000FFFF and then multiply 0x00010001.
         */

        gcmONERROR(_SourceConvertEmit(Tree, CodeGen, States, valueType0, 0, gcvCONVERT_ROTATE));
        break;

    case 0x58:
        /*
         * Need to add conversion for SRC2 before:
         * For  8 bit, shift left 24 bits and then OR 0x00FFFFFF.
         * For 16 bit, shift left 16 bits and then OR 0x0000FFFF.
         */

        gcmONERROR(_SourceConvertEmit(Tree, CodeGen, States, valueType0, 2, gcvCONVERT_LEADZERO));
        break;

    case 0x59:
    case 0x5A:
        /*
         * Need to add conversion for SRC2 before:
         * For  8 bit, AND 0x00000007.
         * For 16 bit, AND 0x0000000F.
         */

        gcmONERROR(_SourceConvertEmit(Tree, CodeGen, States, valueType0, 2, gcvCONVERT_LSHIFT));
        break;

    case 0x33:
        /*
         * Need to add conversion for SRC2 before execution to avoid unwanted saturate:
         * For unsigned data, add AND.
         * For   signed data, add LEFTSHIFT and RIGHTSHIFT.
         */

        gcmONERROR(_SourceConvertEmit(Tree, CodeGen, States, valueType0, 2, gcvCONVERT_STORE));
        break;

    default:
        break;
    }

    /* Add target conversion. */
    switch (opcode)
    {
    case 0x01:
    case 0x2E:
    case 0x3C:
    case 0x4C:
    case 0x59:
    case 0x5B:
    case 0x44:
    case 0x48:
    case 0x5C:
    case 0x5D:
    case 0x5E:
    case 0x5F:
        /*
         * Need to add data conversion for target after execution:
         * For unsigned data, add AND.
         * For   signed data, add LEFTSHIFT and RIGHTSHIFT.
         */

        return _TargetConvertEmit(Tree, CodeGen, States, valueType0, gcvCONVERT_NONE, gcvFALSE);

    case 0x40:
    case 0x50:
        /*
         * Need to add data conversion for target after execution:
         * For unsigned data, add AND.
         * For   signed data, add LEFTSHIFT and RIGHTSHIFT.
         */

        return _TargetConvertEmit(Tree, CodeGen, States, valueType0, gcvCONVERT_HI, gcvFALSE);

    case 0x3B:
    case 0x3E:
    case 0x4E:
        /*
         * Need to add data conversion for target after execution:
         * For unsigned data, add MIN.
         * For   signed data, add MIN and MAX.
         */

        return _TargetConvertEmit(Tree, CodeGen, States, valueType0, gcvCONVERT_NONE, gcvTRUE);

    case 0x52:
        /*
         * Need to add data conversion for target after execution:
         * For unsigned data, add MIN.
         * For   signed data, add MIN and MAX.
         */

        return _TargetConvertEmit(Tree, CodeGen, States, valueType0, gcvCONVERT_HI, gcvTRUE);

    case 0x32:
        return _TargetConvertEmit(Tree, CodeGen, States, valueType0, gcvCONVERT_LOAD, gcvTRUE);

    default:
        return _FinalEmit(Tree, CodeGen, States, 0);
    }

OnError:
    /* Return the status. */
    return status;
}

static gceSTATUS
_ComponentEmit(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctUINT32 States[4],
    IN gctUINT SourceMask,
    IN gctUINT Enable,
    IN gctUINT8 Swizzle0,
    IN gctUINT8 Swizzle1,
    IN gctUINT8 Swizzle2,
    IN gceCONVERT_TYPE ExtraHandling
    )
{
#if _EXTRA_16BIT_CONVERSION_
    gceSTATUS status = gcvSTATUS_OK;
#endif
    gctUINT32 states[4];
    gctBOOL  src0IsImmediate = isSourceImmediateValue(States, 0);
    gctBOOL  src1IsImmediate = isSourceImmediateValue(States, 1);
    gctBOOL  src2IsImmediate = isSourceImmediateValue(States, 2);

    states[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (Enable) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));

    if ((SourceMask & 0x1) && !src0IsImmediate)
    {
        states[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:22) - (0 ?
 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (Swizzle0) & ((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ? 29:22)));
    }
    else
    {
        states[1] = States[1];
    }

    if ((SourceMask & 0x2) && !src1IsImmediate)
    {
        states[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:17) - (0 ?
 24:17) + 1))))))) << (0 ?
 24:17))) | (((gctUINT32) ((gctUINT32) (Swizzle1) & ((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1))))))) << (0 ? 24:17)));
    }
    else
    {
        states[2] = States[2];
    }

    if ((SourceMask & 0x4) && !src2IsImmediate)
    {
        states[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (Swizzle2) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)));
    }
    else
    {
        states[3] = States[3];
    }

    switch (ExtraHandling)
    {
    case gcvCONVERT_NONE:
        return _FinalEmit(Tree, CodeGen, states, 0);

    case gcvCONVERT_EXTRA:
        return _ExtraEmit(Tree, CodeGen, states);

    case gcvCONVERT_COMPONENTXY:
#if _EXTRA_16BIT_CONVERSION_
        /* Sources must be x or y components. */
        if ((SourceMask & 0x1) &&
            (((Enable & gcSL_ENABLE_X) && (Swizzle0 & 0x3) != gcSL_SWIZZLE_X) ||
             ((Enable & gcSL_ENABLE_Y) && (Swizzle0 & 0xc) != (gcSL_SWIZZLE_Y << 2))))
        {
            gcmONERROR(_SourceConvertEmit(Tree,
                                          CodeGen,
                                          states,
                                          0x3,
                                          0,
                                          ExtraHandling));
        }
        if ((SourceMask & 0x2) &&
            (((Enable & gcSL_ENABLE_X) && (Swizzle1 & 0x3) != gcSL_SWIZZLE_X) ||
             ((Enable & gcSL_ENABLE_Y) && (Swizzle1 & 0xc) != (gcSL_SWIZZLE_Y << 2))))
        {
            gcmONERROR(_SourceConvertEmit(Tree,
                                          CodeGen,
                                          states,
                                          0x3,
                                          1,
                                          ExtraHandling));
        }
        if ((SourceMask & 0x4) &&
            (((Enable & gcSL_ENABLE_X) && (Swizzle2 & 0x3) != gcSL_SWIZZLE_X) ||
             ((Enable & gcSL_ENABLE_Y) && (Swizzle2 & 0xc) != (gcSL_SWIZZLE_Y << 2))))
        {
            gcmONERROR(_SourceConvertEmit(Tree,
                                          CodeGen,
                                          states,
                                          0x3,
                                          2,
                                          ExtraHandling));
        }
#endif
        return _FinalEmit(Tree, CodeGen, states, 0);
        break;

    case gcvCONVERT_COMPONENTZW:
#if _EXTRA_16BIT_CONVERSION_
        /* Sources must be z or w components. */
        if ((SourceMask & 0x1) &&
            (((Enable & gcSL_ENABLE_Z) && (Swizzle0 & 0x30) != (gcSL_SWIZZLE_Z << 4)) ||
             ((Enable & gcSL_ENABLE_W) && (Swizzle0 & 0xc0) != (gcSL_SWIZZLE_W << 6))))
        {
            gcmONERROR(_SourceConvertEmit(Tree,
                                          CodeGen,
                                          states,
                                          0x3,
                                          0,
                                          ExtraHandling));
        }
        if ((SourceMask & 0x2) &&
            (((Enable & gcSL_ENABLE_Z) && (Swizzle1 & 0x30) != (gcSL_SWIZZLE_Z << 4)) ||
             ((Enable & gcSL_ENABLE_W) && (Swizzle1 & 0xc0) != (gcSL_SWIZZLE_W << 6))))
        {
            gcmONERROR(_SourceConvertEmit(Tree,
                                          CodeGen,
                                          states,
                                          0x3,
                                          1,
                                          ExtraHandling));
        }
        if ((SourceMask & 0x4) &&
            (((Enable & gcSL_ENABLE_Z) && (Swizzle2 & 0x30) != (gcSL_SWIZZLE_Z << 4)) ||
             ((Enable & gcSL_ENABLE_W) && (Swizzle2 & 0xc0) != (gcSL_SWIZZLE_W << 6))))
        {
            gcmONERROR(_SourceConvertEmit(Tree,
                                          CodeGen,
                                          states,
                                          0x3,
                                          2,
                                          ExtraHandling));
        }
#endif
        return _FinalEmit(Tree, CodeGen, states, 0);
        break;

    default:
        return _FinalEmit(Tree, CodeGen, states, 0);
    }

#if _EXTRA_16BIT_CONVERSION_
OnError:
    /* Return the status. */
    return status;
#endif
}

static gceSTATUS
_DuplicateEmit(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctUINT32 States[4],
    IN gctUINT SourceMask,
    IN gctUINT Enable,
    IN gctUINT8 Swizzle0,
    IN gctUINT8 Swizzle1,
    IN gctUINT8 Swizzle2,
    IN gctBOOL DuplicateOneComponent,
    IN gceCONVERT_TYPE ExtraHandling
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT /*addr,*/ addr0 = 0, addr1 = 0, addr2 = 0;
    gctUINT /*swizzle,*/ swizzle0, swizzle1, swizzle2;
    /*gctUINT swizzleMask, destEnable;*/
    gctUINT comp0 = 0, comp1 = 0, comp2 = 0;
    gctUINT restore0 = 0, restore1 = 0, restore2 = 0;
    gctUINT currentSource = CodeGen->nextSource - 1;
    const gctUINT enable[] = { 1, 2, 4, 8 };

    /* Restore back the lastUse for sources. */
    if ((((((gctUINT32) (States[1])) >> (0 ? 11:11)) & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1)))))) ) &&
        (((((gctUINT32) (States[2])) >> (0 ? 5:3)) & ((gctUINT32) ((((1 ? 5:3) - (0 ? 5:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1)))))) ) == 0x0)
    {
        addr0 = (((((gctUINT32) (States[1])) >> (0 ? 20:12)) & ((gctUINT32) ((((1 ? 20:12) - (0 ? 20:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1)))))) );
        swizzle0 = (((((gctUINT32) (States[1])) >> (0 ? 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1)))))) );
        comp0 = enable[swizzle0 & 0x3] | enable[(swizzle0 >> 2) & 0x3] |
                enable[(swizzle0 >> 4) & 0x3] | enable[(swizzle0 >> 6) & 0x3];
        if ((comp0 & 0x1) && (CodeGen->registerUsage[addr0].lastUse[0] == -1))
        {
            CodeGen->registerUsage[addr0].lastUse[0] = currentSource;
            restore0 |= 0x1;
        }
        if ((comp0 & 0x2) && (CodeGen->registerUsage[addr0].lastUse[1] == -1))
        {
            CodeGen->registerUsage[addr0].lastUse[1] = currentSource;
            restore0 |= 0x2;
        }
        if ((comp0 & 0x4) && (CodeGen->registerUsage[addr0].lastUse[2] == -1))
        {
            CodeGen->registerUsage[addr0].lastUse[2] = currentSource;
            restore0 |= 0x4;
        }
        if ((comp0 & 0x8) && (CodeGen->registerUsage[addr0].lastUse[3] == -1))
        {
            CodeGen->registerUsage[addr0].lastUse[3] = currentSource;
            restore0 |= 0x8;
        }
    }
    if ((((((gctUINT32) (States[2])) >> (0 ? 6:6)) & ((gctUINT32) ((((1 ? 6:6) - (0 ? 6:6) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1)))))) ) &&
        (((((gctUINT32) (States[3])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) ) == 0x0)
    {
        addr1 = (((((gctUINT32) (States[2])) >> (0 ? 15:7)) & ((gctUINT32) ((((1 ? 15:7) - (0 ? 15:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:7) - (0 ? 15:7) + 1)))))) );
        swizzle1 = (((((gctUINT32) (States[2])) >> (0 ? 24:17)) & ((gctUINT32) ((((1 ? 24:17) - (0 ? 24:17) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1)))))) );
        comp1 = enable[swizzle1 & 0x3] | enable[(swizzle1 >> 2) & 0x3] |
                enable[(swizzle1 >> 4) & 0x3] | enable[(swizzle1 >> 6) & 0x3];
        if ((comp1 & 0x1) && (CodeGen->registerUsage[addr1].lastUse[0] == -1))
        {
            CodeGen->registerUsage[addr1].lastUse[0] = currentSource;
            restore1 |= 0x1;
        }
        if ((comp1 & 0x2) && (CodeGen->registerUsage[addr1].lastUse[1] == -1))
        {
            CodeGen->registerUsage[addr1].lastUse[1] = currentSource;
            restore1 |= 0x2;
        }
        if ((comp1 & 0x4) && (CodeGen->registerUsage[addr1].lastUse[2] == -1))
        {
            CodeGen->registerUsage[addr1].lastUse[2] = currentSource;
            restore1 |= 0x4;
        }
        if ((comp1 & 0x8) && (CodeGen->registerUsage[addr1].lastUse[3] == -1))
        {
            CodeGen->registerUsage[addr1].lastUse[3] = currentSource;
            restore1 |= 0x8;
        }
    }
    if ((((((gctUINT32) (States[3])) >> (0 ? 3:3)) & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1)))))) ) &&
        (((((gctUINT32) (States[3])) >> (0 ? 30:28)) & ((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1)))))) ) == 0x0)
    {
        addr2 = (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) );
        swizzle2 = (((((gctUINT32) (States[3])) >> (0 ? 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1)))))) );
        comp2 = enable[swizzle2 & 0x3] | enable[(swizzle2 >> 2) & 0x3] |
                enable[(swizzle2 >> 4) & 0x3] | enable[(swizzle2 >> 6) & 0x3];
        if ((comp2 & 0x1) && (CodeGen->registerUsage[addr2].lastUse[0] == -1))
        {
            CodeGen->registerUsage[addr2].lastUse[0] = currentSource;
            restore2 |= 0x1;
        }
        if ((comp2 & 0x2) && (CodeGen->registerUsage[addr2].lastUse[1] == -1))
        {
            CodeGen->registerUsage[addr2].lastUse[1] = currentSource;
            restore2 |= 0x2;
        }
        if ((comp2 & 0x4) && (CodeGen->registerUsage[addr2].lastUse[2] == -1))
        {
            CodeGen->registerUsage[addr2].lastUse[2] = currentSource;
            restore2 |= 0x4;
        }
        if ((comp2 & 0x8) && (CodeGen->registerUsage[addr2].lastUse[3] == -1))
        {
            CodeGen->registerUsage[addr2].lastUse[3] = currentSource;
            restore2 |= 0x8;
        }
    }


    if (DuplicateOneComponent)
    {
        if (Enable & gcSL_ENABLE_X)
        {
            gcmONERROR(_ComponentEmit(Tree,
                                      CodeGen,
                                      States,
                                      SourceMask,
                                      gcSL_ENABLE_X,
                                      _ReplicateSwizzle(Swizzle0, 0),
                                      _ReplicateSwizzle(Swizzle1, 0),
                                      _ReplicateSwizzle(Swizzle2, 0),
                                      ExtraHandling));
        }

        if (Enable & gcSL_ENABLE_Y)
        {
            gcmONERROR(_ComponentEmit(Tree,
                                      CodeGen,
                                      States,
                                      SourceMask,
                                      gcSL_ENABLE_Y,
                                      _ReplicateSwizzle(Swizzle0, 1),
                                      _ReplicateSwizzle(Swizzle1, 1),
                                      _ReplicateSwizzle(Swizzle2, 1),
                                      ExtraHandling));
        }

        if (Enable & gcSL_ENABLE_Z)
        {
            gcmONERROR(_ComponentEmit(Tree,
                                      CodeGen,
                                      States,
                                      SourceMask,
                                      gcSL_ENABLE_Z,
                                      _ReplicateSwizzle(Swizzle0, 2),
                                      _ReplicateSwizzle(Swizzle1, 2),
                                      _ReplicateSwizzle(Swizzle2, 2),
                                      ExtraHandling));
        }

        if (Enable & gcSL_ENABLE_W)
        {
            gcmONERROR(_ComponentEmit(Tree,
                                      CodeGen,
                                      States,
                                      SourceMask,
                                      gcSL_ENABLE_W,
                                      _ReplicateSwizzle(Swizzle0, 3),
                                      _ReplicateSwizzle(Swizzle1, 3),
                                      _ReplicateSwizzle(Swizzle2, 3),
                                      ExtraHandling));
        }
    }
    else
    {
        if (Enable & gcSL_ENABLE_XY)
        {
            gcmONERROR(_ComponentEmit(Tree,
                                      CodeGen,
                                      States,
                                      SourceMask,
                                      Enable & gcSL_ENABLE_XY,
                                      _ReplicateSwizzle2(Swizzle0, 0),
                                      _ReplicateSwizzle2(Swizzle1, 0),
                                      _ReplicateSwizzle2(Swizzle2, 0),
                                      gcvCONVERT_COMPONENTXY));
        }

        if (Enable & gcSL_ENABLE_ZW)
        {
            gctUINT32 opcode = (((((gctUINT32) (States[0])) >> (0 ? 5:0)) & ((gctUINT32) ((((1 ? 5:0) - (0 ? 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1)))))) )
                             | (((((gctUINT32) (States[2])) >> (0 ? 16:16)) & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1)))))) ) << 6;

            /* Change opcode from 0x3C to 0x3D, and so for others. */
            /* Assume each pair of opcodes are adjacent. */
            opcode++;
            States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (opcode & 0x3F) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));
            States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (opcode >> 6) & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)));

            gcmONERROR(_ComponentEmit(Tree,
                                      CodeGen,
                                      States,
                                      SourceMask,
                                      Enable & gcSL_ENABLE_ZW,
                                      _ReplicateSwizzle2(Swizzle0, 1),
                                      _ReplicateSwizzle2(Swizzle1, 1),
                                      _ReplicateSwizzle2(Swizzle2, 1),
                                      gcvCONVERT_COMPONENTZW));
        }
    }

OnError:
    /* Reset lastUse that was restored. */
    if (restore0)
    {
        if (restore0 & 0x1)
        {
            CodeGen->registerUsage[addr0].lastUse[0] = -1;
        }
        if (restore0 & 0x2)
        {
            CodeGen->registerUsage[addr0].lastUse[1] = -1;
        }
        if (restore0 & 0x4)
        {
            CodeGen->registerUsage[addr0].lastUse[2] = -1;
        }
        if (restore0 & 0x8)
        {
            CodeGen->registerUsage[addr0].lastUse[3] = -1;
        }
    }
    if (restore1)
    {
        if (restore1 & 0x1)
        {
            CodeGen->registerUsage[addr1].lastUse[0] = -1;
        }
        if (restore1 & 0x2)
        {
            CodeGen->registerUsage[addr1].lastUse[1] = -1;
        }
        if (restore1 & 0x4)
        {
            CodeGen->registerUsage[addr1].lastUse[2] = -1;
        }
        if (restore1 & 0x8)
        {
            CodeGen->registerUsage[addr1].lastUse[3] = -1;
        }
    }
    if (restore2)
    {
        if (restore2 & 0x1)
        {
            CodeGen->registerUsage[addr2].lastUse[0] = -1;
        }
        if (restore2 & 0x2)
        {
            CodeGen->registerUsage[addr2].lastUse[1] = -1;
        }
        if (restore2 & 0x4)
        {
            CodeGen->registerUsage[addr2].lastUse[2] = -1;
        }
        if (restore2 & 0x8)
        {
            CodeGen->registerUsage[addr2].lastUse[3] = -1;
        }
    }

    /* Return the status. */
    return status;
}

static gceSTATUS
_Emit(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctUINT32 States[4]
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT valueType0;
    gctUINT instType0;
    gctUINT instType1;
    gctUINT8 enable;
    gctUINT8 swizzle0, swizzle1, swizzle2;

    /* Handle special requests for integer instructions for gc4000. */
    if (CodeGen->isCL_XE)
    {
        /* Has only one 32-bit multiplier. */
        /* Has only one 8-/16-bit divider that cannot handle constant registers. */
        /* Need to duplicate instructions for each component. */
        /* Note that it takes only source component x only. */
        gctUINT32 opcode = (((((gctUINT32) (States[0])) >> (0 ? 5:0)) & ((gctUINT32) ((((1 ? 5:0) - (0 ? 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1)))))) )
                         | (((((gctUINT32) (States[2])) >> (0 ? 16:16)) & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1)))))) ) << 6;
        gctBOOL duplicateOneComponent = gcvTRUE;
        gctINT newPhysical[2] = {-1, -1};
        gctINT32 lastUse[2] = {-1, -1};
        gctINT i, j;

        switch (opcode)
        {
        case 0x3C:
            /* fall through */
        case 0x3E:
            /* fall through */
        case 0x40:
            /* Get value type. */
            instType0 = (((((gctUINT32) (States[1])) >> (0 ? 21:21)) & ((gctUINT32) ((((1 ? 21:21) - (0 ? 21:21) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 21:21) - (0 ? 21:21) + 1)))))) );
            instType1 = (((((gctUINT32) (States[2])) >> (0 ? 31:30)) & ((gctUINT32) ((((1 ? 31:30) - (0 ? 31:30) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:30) - (0 ? 31:30) + 1)))))) );
            valueType0 = instType0 | (instType1 << 1);

            if (valueType0 == 0x4
            ||  valueType0 == 0x7)
            {
                /* No extra handling needed. */
                break;
            }

            if (valueType0 == 0x3
            ||  valueType0 == 0x6)
            {
                duplicateOneComponent = gcvFALSE;
            }

            enable   = (((((gctUINT32) (States[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) );
            swizzle0 = (((((gctUINT32) (States[1])) >> (0 ? 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1)))))) );
            swizzle1 = (((((gctUINT32) (States[2])) >> (0 ? 24:17)) & ((gctUINT32) ((((1 ? 24:17) - (0 ? 24:17) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1)))))) );

            gcmONERROR(_DuplicateEmit(Tree,
                                      CodeGen,
                                      States,
                                      0x3, /* Use src0 and src1. */
                                      enable,
                                      swizzle0,
                                      swizzle1,
                                      0,
                                      duplicateOneComponent,
                                      gcvCONVERT_NONE));
            return gcvSTATUS_OK;

        case 0x4C:
            /* fall through */
        case 0x4E:
            /* fall through */
        case 0x50:
            /* fall through */
        case 0x52:
            instType0 = (((((gctUINT32) (States[1])) >> (0 ? 21:21)) & ((gctUINT32) ((((1 ? 21:21) - (0 ? 21:21) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 21:21) - (0 ? 21:21) + 1)))))) );
            instType1 = (((((gctUINT32) (States[2])) >> (0 ? 31:30)) & ((gctUINT32) ((((1 ? 31:30) - (0 ? 31:30) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:30) - (0 ? 31:30) + 1)))))) );
            valueType0 = instType0 | (instType1 << 1);

            if (valueType0 == 0x4
            ||  valueType0 == 0x7)
            {
                /* No extra handling needed. */
                break;
            }

            if (valueType0 == 0x3
            ||  valueType0 == 0x6)
            {
                duplicateOneComponent = gcvFALSE;
            }

            enable   = (((((gctUINT32) (States[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) );
            swizzle0 = (((((gctUINT32) (States[1])) >> (0 ? 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1)))))) );
            swizzle1 = (((((gctUINT32) (States[2])) >> (0 ? 24:17)) & ((gctUINT32) ((((1 ? 24:17) - (0 ? 24:17) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1)))))) );
            swizzle2 = (((((gctUINT32) (States[3])) >> (0 ? 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1)))))) );

            gcmONERROR(_DuplicateEmit(Tree,
                                      CodeGen,
                                      States,
                                      0x7, /* Use src0, src1, and src2. */
                                      enable,
                                      swizzle0,
                                      swizzle1,
                                      swizzle2,
                                      duplicateOneComponent,
                                      gcvCONVERT_NONE));
            return gcvSTATUS_OK;

            /* Special handling for IDIV/IMOD. */
            /* Sources have to be temp registers. */
        case 0x44:
            /* fall through */
        case 0x48:
            if ((((((gctUINT32) (States[2])) >> (0 ? 5:3)) & ((gctUINT32) ((((1 ? 5:3) - (0 ? 5:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1)))))) ) == 0x2)
            {
                /* Source 0 is constant: Copy source 0 to temp. */
                gcmERR_BREAK(_TempEmit(Tree, CodeGen, States, 0, &newPhysical[0], &lastUse[0]));
            }
            if ((((((gctUINT32) (States[3])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) ) == 0x2)
            {
                /* Source 1 is constant: Copy source 1 to temp. */
                gcmERR_BREAK(_TempEmit(Tree, CodeGen, States, 1,
                                        newPhysical[0] == -1 ? &newPhysical[0] : &newPhysical[1],
                                        newPhysical[0] == -1 ? &lastUse[0] : &lastUse[1]));
            }

            enable   = (((((gctUINT32) (States[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) );
            swizzle0 = (((((gctUINT32) (States[1])) >> (0 ? 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1)))))) );
            swizzle1 = (((((gctUINT32) (States[2])) >> (0 ? 24:17)) & ((gctUINT32) ((((1 ? 24:17) - (0 ? 24:17) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1)))))) );

            gcmONERROR(_DuplicateEmit(Tree,
                                      CodeGen,
                                      States,
                                      0x3, /* Use src0 and src1. */
                                      enable,
                                      swizzle0,
                                      swizzle1,
                                      0,
                                      gcvTRUE,
                                      gcvCONVERT_EXTRA));

            /* If we generate new register on TempEmit, we can reuse these temp register. */
            for (i = 0; i < 2; i++)
            {
                if (newPhysical[i] != -1)
                {
                    for (j = 0; j < 4; j++)
                    {
                        if ((CodeGen->registerUsage + newPhysical[i])->lastUse[j] == lastUse[i])
                        {
                            (CodeGen->registerUsage + newPhysical[i])->lastUse[j] = gcvSL_AVAILABLE;
                        }
                    }
                }
            }

            return gcvSTATUS_OK;

        case 0x32:
            if (! CodeGen->hasBugFixes10)
            {
                return _ExtraEmit(Tree, CodeGen, States);
            }
            break;

        case 0x65:
        case 0x66:
        case 0x68:
        case 0x69:
        case 0x6A:
        case 0x6B:
        case 0x6C:
            return _ExtraEmit(Tree, CodeGen, States);

        case 0x67:
            if(!CodeGen->hasUSC)
            {
                return _ExtraEmit(Tree, CodeGen, States);
            }
            break;

        default:
            break;
        }

        return _FinalEmit(Tree, CodeGen, States, 0);
    }

    /* Handle special requests for integer instructions for gc2100. */
    if (CodeGen->isCL_X)
    {
        /* Has only one 32-bit multiplier and one 32-bit adder. */
        /* Has only one 16-bit divider that cannot handle constant registers. */
        /* Need to duplicate instructions for each component. */
        /* Note that it takes only source component x only. */
        gctUINT32 opcode = (((((gctUINT32) (States[0])) >> (0 ? 5:0)) & ((gctUINT32) ((((1 ? 5:0) - (0 ? 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1)))))) )
                         | (((((gctUINT32) (States[2])) >> (0 ? 16:16)) & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1)))))) ) << 6;
        gctINT newPhysical[2] = {-1, -1};
        gctINT32 lastUse[2] = {-1, -1};
        gctINT i, j;

        switch (opcode)
        {
            /* One-operand opcodes that use SRC0. */
        case 0x2D:
            /* fall through */
        case 0x2E:

            enable   = (((((gctUINT32) (States[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) );
            swizzle0 = (((((gctUINT32) (States[1])) >> (0 ? 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1)))))) );

            gcmONERROR(_DuplicateEmit(Tree,
                                      CodeGen,
                                      States,
                                      0x1, /* Use src0. */
                                      enable,
                                      swizzle0,
                                      0,
                                      0,
                                      gcvTRUE,
                                      gcvCONVERT_EXTRA));
            return gcvSTATUS_OK;

            /* One-operand opcodes that use SRC2. */
        case 0x5F:
            /* fall through */
        case 0x57:
            /* fall through */
        case 0x58:

            enable   = (((((gctUINT32) (States[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) );
            swizzle2 = (((((gctUINT32) (States[3])) >> (0 ? 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1)))))) );

            gcmONERROR(_DuplicateEmit(Tree,
                                      CodeGen,
                                      States,
                                      0x4, /* Use src2. */
                                      enable,
                                      0,
                                      0,
                                      swizzle2,
                                      gcvTRUE,
                                      gcvCONVERT_EXTRA));
            return gcvSTATUS_OK;

            /* Two-operand opcodes that use SRC0 and SRC1. */
        case 0x3C:
            /* fall through */
        case 0x40:
            /* fall through */
        case 0x3E:

            enable   = (((((gctUINT32) (States[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) );
            swizzle0 = (((((gctUINT32) (States[1])) >> (0 ? 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1)))))) );
            swizzle1 = (((((gctUINT32) (States[2])) >> (0 ? 24:17)) & ((gctUINT32) ((((1 ? 24:17) - (0 ? 24:17) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1)))))) );

            gcmONERROR(_DuplicateEmit(Tree,
                                      CodeGen,
                                      States,
                                      0x3, /* Use src0 and src1. */
                                      enable,
                                      swizzle0,
                                      swizzle1,
                                      0,
                                      gcvTRUE,
                                      gcvCONVERT_EXTRA));
            return gcvSTATUS_OK;

            /* Special handling for IDIV/IMOD. */
            /* Sources have to be temp registers. */
        case 0x44:
            /* fall through */
        case 0x48:

            if ((((((gctUINT32) (States[2])) >> (0 ? 5:3)) & ((gctUINT32) ((((1 ? 5:3) - (0 ? 5:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1)))))) ) == 0x2)
            {
                /* Source 0 is constant: Copy source 0 to temp. */
                gcmERR_BREAK(_TempEmit(Tree, CodeGen, States, 0, &newPhysical[0], &lastUse[0]));
            }
            if ((((((gctUINT32) (States[3])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) ) == 0x2)
            {
                /* Source 1 is constant: Copy source 1 to temp. */
                gcmERR_BREAK(_TempEmit(Tree, CodeGen, States, 1,
                                            newPhysical[0] == -1 ? &newPhysical[0] : &newPhysical[1],
                                            newPhysical[0] == -1 ? &lastUse[0] : &lastUse[1]));
            }

            enable   = (((((gctUINT32) (States[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) );
            swizzle0 = (((((gctUINT32) (States[1])) >> (0 ? 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1)))))) );
            swizzle1 = (((((gctUINT32) (States[2])) >> (0 ? 24:17)) & ((gctUINT32) ((((1 ? 24:17) - (0 ? 24:17) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1)))))) );

            gcmONERROR(_DuplicateEmit(Tree,
                                      CodeGen,
                                      States,
                                      0x3, /* Use src0 and src1. */
                                      enable,
                                      swizzle0,
                                      swizzle1,
                                      0,
                                      gcvTRUE,
                                      gcvCONVERT_EXTRA));

            /* If we generate new register on TempEmit, we can reuse these temp register. */
            for (i = 0; i < 2; i++)
            {
                if (newPhysical[i] != -1)
                {
                    for (j = 0; j < 4; j++)
                    {
                        if ((CodeGen->registerUsage + newPhysical[i])->lastUse[j] == lastUse[i])
                        {
                            (CodeGen->registerUsage + newPhysical[i])->lastUse[j] = gcvSL_AVAILABLE;
                        }
                    }
                }
            }
            return gcvSTATUS_OK;

            /* Two-operand opcodes that use SRC0 and SRC2. */
        case 0x01:
            /* Get value type. */
            instType0 = (((((gctUINT32) (States[1])) >> (0 ? 21:21)) & ((gctUINT32) ((((1 ? 21:21) - (0 ? 21:21) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 21:21) - (0 ? 21:21) + 1)))))) );
            instType1 = (((((gctUINT32) (States[2])) >> (0 ? 31:30)) & ((gctUINT32) ((((1 ? 31:30) - (0 ? 31:30) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:30) - (0 ? 31:30) + 1)))))) );
            valueType0 = instType0 | (instType1 << 1);

            if (valueType0 == 0x0)
            {
                break;
            }
            /* fall through */
        case 0x3B:
            /* fall through */
        case 0x59:
            /* fall through */
        case 0x5A:
            /* fall through */
        case 0x5B:
            /* fall through */
        case 0x5C:
            /* fall through */
        case 0x5D:
            /* fall through */
        case 0x5E:

            enable   = (((((gctUINT32) (States[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) );
            swizzle0 = (((((gctUINT32) (States[1])) >> (0 ? 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1)))))) );
            swizzle2 = (((((gctUINT32) (States[3])) >> (0 ? 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1)))))) );

            gcmONERROR(_DuplicateEmit(Tree,
                                      CodeGen,
                                      States,
                                      0x5, /* Use src0 and src2. */
                                      enable,
                                      swizzle0,
                                      0,
                                      swizzle2,
                                      gcvTRUE,
                                      gcvCONVERT_EXTRA));
            return gcvSTATUS_OK;

            /* Three-operand opcodes. */
        case 0x4C:
            /* fall through */
        case 0x50:
            /* fall through */
        case 0x4E:
            /* fall through */
        case 0x52:

            enable   = (((((gctUINT32) (States[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) );
            swizzle0 = (((((gctUINT32) (States[1])) >> (0 ? 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1)))))) );
            swizzle1 = (((((gctUINT32) (States[2])) >> (0 ? 24:17)) & ((gctUINT32) ((((1 ? 24:17) - (0 ? 24:17) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1)))))) );
            swizzle2 = (((((gctUINT32) (States[3])) >> (0 ? 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1)))))) );

            gcmONERROR(_DuplicateEmit(Tree,
                                      CodeGen,
                                      States,
                                      0x7, /* Use src0, src1, and src2. */
                                      enable,
                                      swizzle0,
                                      swizzle1,
                                      swizzle2,
                                      gcvTRUE,
                                      gcvCONVERT_EXTRA));
            return gcvSTATUS_OK;


        case 0x32:
            if (! CodeGen->hasBugFixes10)
            {
                return _ExtraEmit(Tree, CodeGen, States);
            }
            break;

        default:
            break;
        }

        return _FinalEmit(Tree, CodeGen, States, 0);
    }

    return _FinalEmit(Tree, CodeGen, States, 0);

OnError:
    /* Return the status. */
    return status;
}

static gceSTATUS
_CodeGenMOVA(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctINT indexedReg,
    IN gctUINT formatOfIndexedReg,
    IN gctUINT32 addrEnable,
    IN gctUINT8 indexedRegSwizzle
    )
{
    gceSTATUS status;

    do
    {
        gctUINT states[4];

#if _USE_TEMP_FORMAT_
        gctUINT format = Tree->tempArray[indexedReg].format;
#else
        gctUINT format = formatOfIndexedReg;
#endif

        /* Zero the instruction. */
        gcoOS_ZeroMemory(states, gcmSIZEOF(states));

        if (format == gcSL_FLOAT)
        {
            /* MOVAF opcode. */
            _SetOpcode(states,
                       0x0B,
                       0x00,
                       gcvFALSE);
        }
        else
        {
            /* MOVAI opcode. */
            _SetOpcode(states,
                       0x56,
                       0x00,
                       gcvFALSE);
#if _USE_TEMP_FORMAT_
            /* MOVAI assumes non-32-bit data are packed. */
            /* Always use uint for now. */
            /*_SetValueType0FromFormat(format, states);*/
            _SetValueType0FromFormat(gcSL_UINT32, states);
#else
            value_type0(Tree, CodeGen, instruction, states);
#endif
        }

        /* A0 register. */
        gcmERR_BREAK(_gcmSetDest(Tree,
                                 CodeGen,
                                 states,
                                 -1,
                                 0x0,
                                 addrEnable,
                                 Tree->tempArray[indexedReg].precision,
                                 gcvNULL));

        /* Temporary register. */
        gcmERR_BREAK(_gcmSetSource(Tree,
                                   CodeGen,
                                   states,
                                   2,
                                   gcSL_TEMP,
                                   indexedReg,
                                   0,
                                   0x0,
                                   indexedRegSwizzle, /*0xE4,*/
                                   gcvFALSE,
                                   gcvFALSE,
                                   Tree->tempArray[indexedReg].precision));

        /* Reset destination valid bit for a0 register. */
        states[0] = ((((gctUINT32) (states[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)));

        /* Emit the instruction. */
        gcmERR_BREAK(_Emit(Tree, CodeGen, states));
    }
    while (gcvFALSE);

    return status;
}

/* patch state for finalizing subsampleDepth register */
static void
_PatchSubsampleDepthRegister(
    IN    gcLINKTREE            Tree,
    IN    gcsCODE_GENERATOR_PTR CodeGen
    )
{
    gcsSL_PHYSICAL_CODE_PTR code;
    gctSIZE_T               f, i;
    gctUINT                 fakeSubsampleDepthReg = CodeGen->subsampleDepthPhysical;
    gctUINT                 realSubsampleDepthReg = CodeGen->maxRegister + 1;
    gctUINT *               States;

    gcmASSERT(fakeSubsampleDepthReg > CodeGen->maxRegister && CodeGen->subsampleDepthRegIncluded);

    /* update register allocation in Tree */
    Tree->tempArray[CodeGen->subsampleDepthIndex].assigned = realSubsampleDepthReg;
    gcCGUpdateMaxRegister(CodeGen, realSubsampleDepthReg, Tree);

    for (f = 0; f <= Tree->shader->functionCount + Tree->shader->kernelFunctionCount; ++f)
    {
        for (code = CodeGen->functions[f].root;
             code != gcvNULL;
             code = code->next)
        {
            /* Process all states. */
            for (i = 0; i < code->count; ++i)
            {
                States = &code->states[i*4];

                /* Check target. */
                if ((((((gctUINT32) (States[0])) >> (0 ? 12:12)) & ((gctUINT32) ((((1 ? 12:12) - (0 ? 12:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1)))))) ) &&
                    (((((gctUINT32) (States[0])) >> (0 ? 22:16)) & ((gctUINT32) ((((1 ? 22:16) - (0 ? 22:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1)))))) ) == (fakeSubsampleDepthReg & 0x7f))
                {
                    States[0] = ((((gctUINT32) (States[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (realSubsampleDepthReg) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)));
                }

                /* check src0 */
                if ((((((gctUINT32) (States[1])) >> (0 ? 11:11)) & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1)))))) ) &&
                    (((((gctUINT32) (States[2])) >> (0 ? 5:3)) & ((gctUINT32) ((((1 ? 5:3) - (0 ? 5:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1)))))) ) == 0x0 &&
                    (((((gctUINT32) (States[1])) >> (0 ? 20:12)) & ((gctUINT32) ((((1 ? 20:12) - (0 ? 20:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1)))))) )  == (fakeSubsampleDepthReg & 0x1FF))
                {
                    States[1] = ((((gctUINT32) (States[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:12) - (0 ?
 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (realSubsampleDepthReg) & ((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ? 20:12)));
                }

                /* check src1 */
                if ((((((gctUINT32) (States[2])) >> (0 ? 6:6)) & ((gctUINT32) ((((1 ? 6:6) - (0 ? 6:6) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1)))))) ) &&
                    (((((gctUINT32) (States[3])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) ) == 0x0 &&
                    (((((gctUINT32) (States[2])) >> (0 ? 15:7)) & ((gctUINT32) ((((1 ? 15:7) - (0 ? 15:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:7) - (0 ? 15:7) + 1)))))) )  == (fakeSubsampleDepthReg & 0x1FF))
                {
                    States[2] = ((((gctUINT32) (States[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:7) - (0 ?
 15:7) + 1))))))) << (0 ?
 15:7))) | (((gctUINT32) ((gctUINT32) (realSubsampleDepthReg) & ((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:7) - (0 ? 15:7) + 1))))))) << (0 ? 15:7)));
                }

                /* check src2 */
                if ((((((gctUINT32) (States[3])) >> (0 ? 3:3)) & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1)))))) ) &&
                    (((((gctUINT32) (States[3])) >> (0 ? 30:28)) & ((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1)))))) ) == 0x0 &&
                    (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) )  == (fakeSubsampleDepthReg & 0x1FF))
                {
                    States[3] = ((((gctUINT32) (States[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (realSubsampleDepthReg) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)));
                }
            }
        }
    }

}

static void
_SplitTmpAddrMapTableEntry(
    gcsSL_ADDRESS_REG_COLORING_PTR addRegColoring,
    gctINT entryToSplit,
    gctINT newChannelCount)
{
    gctINT i;

    gcmASSERT(addRegColoring->countOfMap < 4);

    /* split */
    for (i = addRegColoring->countOfMap - 1; i >= entryToSplit; i --)
    {
        if (i != entryToSplit)
        {
            addRegColoring->tmp2addrMap[i+1].channelCount =
                                       addRegColoring->tmp2addrMap[i].channelCount;

            addRegColoring->tmp2addrMap[i+1].indexedReg =
                                       addRegColoring->tmp2addrMap[i].indexedReg;

            addRegColoring->tmp2addrMap[i+1].startChannelInAddressReg =
                                       addRegColoring->tmp2addrMap[i].startChannelInAddressReg;

            addRegColoring->tmp2addrMap[i+1].startChannelInIndexedReg =
                                       addRegColoring->tmp2addrMap[i].startChannelInIndexedReg;
        }
        else
        {
            addRegColoring->tmp2addrMap[i].channelCount = addRegColoring->tmp2addrMap[i].channelCount - newChannelCount;
            addRegColoring->tmp2addrMap[i+1].channelCount = newChannelCount;
            /* Split will make old and new entries are all invalid, i.e indexedReg == -1 */
            addRegColoring->tmp2addrMap[i].indexedReg = addRegColoring->tmp2addrMap[i+1].indexedReg = -1;
            addRegColoring->tmp2addrMap[i+1].startChannelInAddressReg =
                   (gctUINT8)(addRegColoring->tmp2addrMap[i].startChannelInAddressReg + addRegColoring->tmp2addrMap[i].channelCount);
            addRegColoring->tmp2addrMap[i+1].startChannelInIndexedReg = -1;
        }
    }

    addRegColoring->countOfMap ++;
}

static void
_MergeTmpAddrMapTableAdjacentEntries(
    gcsSL_ADDRESS_REG_COLORING_PTR addRegColoring,
    gctINT PivotEntryToMerge,
    gctBOOL bRightAdj)
{
    gctINT i, leftIdx, rightIdx;

    gcmASSERT(addRegColoring->countOfMap <= 4);

    if (bRightAdj)
    {
        leftIdx = PivotEntryToMerge;
        rightIdx = leftIdx + 1;
    }
    else
    {
        rightIdx = PivotEntryToMerge;
        leftIdx = rightIdx - 1;
    }

    /* Merge will make new entry is invalid, i.e indexedReg == -1 */
    addRegColoring->tmp2addrMap[leftIdx].channelCount += addRegColoring->tmp2addrMap[rightIdx].channelCount;
    addRegColoring->tmp2addrMap[leftIdx].indexedReg = -1;

    for (i = rightIdx+1; i < addRegColoring->countOfMap; i ++)
    {
        addRegColoring->tmp2addrMap[i-1].channelCount =
                                   addRegColoring->tmp2addrMap[i].channelCount;

        addRegColoring->tmp2addrMap[i-1].indexedReg =
                                   addRegColoring->tmp2addrMap[i].indexedReg;

        addRegColoring->tmp2addrMap[i-1].startChannelInAddressReg =
                                   addRegColoring->tmp2addrMap[i].startChannelInAddressReg;

        addRegColoring->tmp2addrMap[i-1].startChannelInIndexedReg =
                                   addRegColoring->tmp2addrMap[i].startChannelInIndexedReg;
    }

    addRegColoring->countOfMap --;
}

static gceSTATUS
_ResetAddressRegChannel(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcSHADER  shader = Tree->shader;
    gcsSL_ADDRESS_REG_COLORING_PTR addRegColoring = &CodeGen->current->addrRegColoring;
    gcSL_ENABLE targetEnable;
    gctINT mapIdx, j;
    gctUINT32 i, k;
    gcFUNCTION function;

    if (Instruction->opcode == gcSL_CALL)
    {
        for (i = 0; i < shader->functionCount; ++i)
        {
            if (shader->functions[i]->codeStart == Instruction->tempIndex)
            {
                break;
            }
        }

        /* Do not need consider kernel func because we only consider subroutines */
        if (i < shader->functionCount)
        {
            function = shader->functions[i];
            for (k = 0; k < function->argumentCount; k++)
            {
                if (function->arguments[k].qualifier == gcvFUNCTION_OUTPUT)
                {
                    targetEnable = function->arguments[k].enable;

                    for (mapIdx = 0; mapIdx < addRegColoring->countOfMap; mapIdx ++)
                    {
                        if ((gctUINT32)addRegColoring->tmp2addrMap[mapIdx].indexedReg == function->arguments[k].index)
                        {
                            gcSL_ENABLE enable = (gcSL_ENABLE_X >>
                                addRegColoring->tmp2addrMap[mapIdx].startChannelInIndexedReg);

                            for (j = 0; j < addRegColoring->tmp2addrMap[mapIdx].channelCount; j++)
                            {
                                if (targetEnable & (enable >> j))
                                {
                                    /* currently, we just simply clean up all existed index address. */
                                    addRegColoring->countOfMap = 0;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        targetEnable = gcmSL_TARGET_GET(Instruction->temp, Enable);

        for (mapIdx = 0; mapIdx < addRegColoring->countOfMap; mapIdx ++)
        {
            if ((gctUINT32)addRegColoring->tmp2addrMap[mapIdx].indexedReg == Instruction->tempIndex)
            {
                gcSL_ENABLE enable = (gcSL_ENABLE_X >>
                                            addRegColoring->tmp2addrMap[mapIdx].startChannelInIndexedReg);

                for (j = 0; j < addRegColoring->tmp2addrMap[mapIdx].channelCount; j++)
                {
                    if (targetEnable & (enable >> j))
                    {
                        /* currently, we just simply clean up all existed index address. */
                        addRegColoring->countOfMap = 0;
                        break;
                    }
                }
            }
        }
    }

    return status;
}

static gceSTATUS
_FindAddressRegChannel(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctINT indexedReg,
    IN gctUINT formatOfIndexedReg,
    IN gcSL_INDEXED IndexedChannelOfReg,
    OUT gctUINT8* singleAddrEnable
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT    startChannelInAddressReg = -1;
    gctUINT8  curAddrEnableMask = 0;
    gcsSL_ADDRESS_REG_COLORING_PTR addRegColoring = &CodeGen->current->addrRegColoring;

    /* if the shader is converted back from VIR shader and HW register allocated,
     * it should already handled all indexing operands be explicitly generating
     * MOVA instruction, so we don't need to generate it agian
     */
    if (_isHWRegisterAllocated(Tree->shader))
    {
#if defined(_WINDOWS) || (defined(_WIN32) || defined(WIN32)) && !defined(UNDER_CE)
        gcmASSERT(indexedReg == VIR_SR_A0 || indexedReg == VIR_SR_B0);
#endif
        *singleAddrEnable = (0x1) << (IndexedChannelOfReg -1);
        return status;
    }

    do
    {
        gctBOOL bNeedGenMova = gcvTRUE;
        gctINT mapIdx, i;

        gcmASSERT(indexedReg >= 0);

        /* Check whether there is a mova generated for this indexed channel already */
        for (mapIdx = 0; mapIdx < CodeGen->current->addrRegColoring.countOfMap; mapIdx ++)
        {
            /* This reg has been mapped */
            if (addRegColoring->tmp2addrMap[mapIdx].indexedReg == indexedReg)
            {
                /* Shift it to zero-based */
                gctINT adjustedIndexedChannelOfReg = IndexedChannelOfReg - 1;

                if ((adjustedIndexedChannelOfReg >= addRegColoring->tmp2addrMap[mapIdx].startChannelInIndexedReg) &&
                    (adjustedIndexedChannelOfReg <= (addRegColoring->tmp2addrMap[mapIdx].startChannelInIndexedReg +
                     addRegColoring->tmp2addrMap[mapIdx].channelCount - 1)))
                {
                    bNeedGenMova = gcvFALSE;
                    startChannelInAddressReg = addRegColoring->tmp2addrMap[mapIdx].startChannelInAddressReg;

                    for (i = startChannelInAddressReg; i < startChannelInAddressReg +
                                      addRegColoring->tmp2addrMap[mapIdx].channelCount; i ++)
                    {
                        curAddrEnableMask |= (1 << i);
                    }

                    break;
                }
            }
        }

        if (bNeedGenMova)
        {
            gctUINT8 indexedRegEnableMask = 0, indexedRegEnableMaskTemp = 0;
            gctUINT8 indexedRegSwizzle = 0;
            gctINT mapIdxToInsert = -1;
            gctINT startChannelInIndexedReg = -1, enabledChannelCount = 0;
            gcsLINKTREE_LIST_PTR def;
            gcsLINKTREE_LIST_PTR user;
            gctINT j;

            /* Figure out how many channel are used and which channel is the first channel in temp register */
            gcmASSERT(Tree->tempArray[indexedReg].constUsage[0] != 1 ||
                      Tree->tempArray[indexedReg].constUsage[1] != 1 ||
                      Tree->tempArray[indexedReg].constUsage[2] != 1 ||
                      Tree->tempArray[indexedReg].constUsage[3] != 1);

            def = Tree->tempArray[indexedReg].defined;
            while (def)
            {
                gcSL_INSTRUCTION inst = Tree->shader->code + def->index;
                indexedRegEnableMaskTemp |= gcmSL_TARGET_GET(inst->temp, Enable);
                def = def->next;
            }

            user = Tree->tempArray[indexedReg].users;
            while (user)
            {
                gcSL_INSTRUCTION inst = Tree->shader->code + user->index;

                /* Only check indexed user */
                if (inst->tempIndexed == indexedReg &&
                    gcmSL_TARGET_GET(inst->temp, Indexed) != gcSL_NOT_INDEXED)
                {
                    indexedRegEnableMask |= (1 << (gcmSL_TARGET_GET(inst->temp, Indexed) - 1));
                }

                if (inst->source0Indexed == indexedReg &&
                    gcmSL_SOURCE_GET(inst->source0, Indexed) != gcSL_NOT_INDEXED)
                {
                    indexedRegEnableMask |= (1 << (gcmSL_SOURCE_GET(inst->source0, Indexed) - 1));
                }

                if (inst->source1Indexed == indexedReg &&
                    gcmSL_SOURCE_GET(inst->source1, Indexed) != gcSL_NOT_INDEXED)
                {
                    indexedRegEnableMask |= (1 << (gcmSL_SOURCE_GET(inst->source1, Indexed) - 1));
                }

                user = user->next;
            }

            if (gcGetVIRCGKind(Tree->hwCfg.hwFeatureFlags.hasHalti2) == VIRCG_None)
            {
                gcmASSERT((indexedRegEnableMask & indexedRegEnableMaskTemp) == indexedRegEnableMask);
            }

            for (i = 0; i < 4; i ++)
            {
                if ((indexedRegEnableMask >> i) & 0x01)
                {
                    if (startChannelInIndexedReg == -1)
                        startChannelInIndexedReg = i;

                    /* Note we permit a hole among channels, otherwise we need */
                    /* more complicated algorithm to handle it, at least we need */
                    /* address register in glSL level. */
                    enabledChannelCount = i - startChannelInIndexedReg + 1;
                }
            }

            /* If new desired address reg channel count is 4 but other src has occupied some, we can
               use full channel count, just use required one at this time. */

            if (enabledChannelCount == 4 && addRegColoring->localAddrChannelUsageMask != 0)
            {
                enabledChannelCount = 1;
                startChannelInIndexedReg = IndexedChannelOfReg - 1;
            }

            gcmASSERT(enabledChannelCount > 0 && startChannelInIndexedReg >= 0);

            /* Check whether we have enough space to hold current address channels assignment */
            if (addRegColoring->countOfMap > 0)
            {
                while (startChannelInAddressReg == -1)
                {
                    gcmASSERT(addRegColoring->countOfMap <= 4);

                    /* Check empty, maybe needs map table split */
                    for (mapIdx = 0; mapIdx < CodeGen->current->addrRegColoring.countOfMap; mapIdx ++)
                    {
                        if (addRegColoring->tmp2addrMap[mapIdx].indexedReg == -1)
                        {
                            gctINT deltaChannelCount = addRegColoring->tmp2addrMap[mapIdx].channelCount - enabledChannelCount;

                            if (deltaChannelCount < 0)
                                continue;

                            if (deltaChannelCount > 0)
                            {
                                _SplitTmpAddrMapTableEntry(addRegColoring, mapIdx, deltaChannelCount);
                            }

                            mapIdxToInsert = mapIdx;
                            startChannelInAddressReg = addRegColoring->tmp2addrMap[mapIdx].startChannelInAddressReg;
                            break;
                        }
                    }

                    /* Check tail unused part */
                    if (startChannelInAddressReg == -1 &&
                        addRegColoring->tmp2addrMap[addRegColoring->countOfMap - 1].indexedReg != -1)
                    {
                        if (4 - (addRegColoring->tmp2addrMap[addRegColoring->countOfMap - 1].startChannelInAddressReg +
                            addRegColoring->tmp2addrMap[addRegColoring->countOfMap - 1].channelCount) >= enabledChannelCount)
                        {
                            startChannelInAddressReg = addRegColoring->tmp2addrMap[addRegColoring->countOfMap - 1].startChannelInAddressReg +
                                                   addRegColoring->tmp2addrMap[addRegColoring->countOfMap - 1].channelCount;
                        }
                    }

                    /* Register spill here and split/merge */
                    if (startChannelInAddressReg == -1)
                    {
                        gctUINT8  tempAddrEnableMask = 0, cstart;
                        gctINT    ccount;
                        gctBOOL   bFound = gcvFALSE;
                        gctINT    startIdx = -1, endIdx = -1, tcount = 0;

                        /* A very rough heuristic here since we have no live analysis */
                        /* and spill cost analysis for address register. At least, we */
                        /* should add last usage of temp register for address access */
                        /* purpose then we can mark this unused after last usage */
                        /*            ****NEED REFINE THIS SPILL****             */

                        /* Always select foremost entries that can accommodate it */
                        for (mapIdx = 0; mapIdx < CodeGen->current->addrRegColoring.countOfMap; mapIdx ++)
                        {
                            cstart = addRegColoring->tmp2addrMap[mapIdx].startChannelInAddressReg;
                            ccount = addRegColoring->tmp2addrMap[mapIdx].channelCount;
                            tcount += ccount;

                            for (i = cstart; i < cstart + ccount; i ++)
                            {
                                tempAddrEnableMask |= (1 << i);
                            }

                            /* Need consider local usage mask here */
                            if (!(tempAddrEnableMask & addRegColoring->localAddrChannelUsageMask))
                            {
                                endIdx = mapIdx;
                                if (startIdx == -1)
                                    startIdx = mapIdx;

                                if (tcount >= enabledChannelCount)
                                {
                                    bFound = gcvTRUE;
                                    break;
                                }
                            }
                            else
                            {
                                /* reset */
                                startIdx = -1;
                                endIdx = -1;
                                tcount = 0;
                                tempAddrEnableMask = 0;
                            }
                        }

                        if (bFound)
                        {
                            if (startIdx == endIdx)
                            {
                                addRegColoring->tmp2addrMap[startIdx].indexedReg = -1;
                            }
                            else
                            {
                                /* merge */
                                for (mapIdx = endIdx-1; mapIdx >= startIdx; mapIdx --)
                                {
                                    _MergeTmpAddrMapTableAdjacentEntries(addRegColoring, mapIdx, gcvTRUE);
                                }
                            }
                        }
                        else
                        {
                            /* Goto compile error */
                            break;
                        }
                    }
                }

                /* Oops, fail to allocate since no enough channel space */
                if (startChannelInAddressReg == -1)
                {
                    status = gcvSTATUS_OUT_OF_RESOURCES;

                    /* This 'break' must be at the outmost loop for status return */
                    break;
                }
            }
            else
            {
                startChannelInAddressReg = 0;
            }

            gcmASSERT(startChannelInAddressReg >= 0);

            /* Ok, we can assign space safely, so update tmp2add mapping table */
            if (mapIdxToInsert == -1)
            {
                addRegColoring->tmp2addrMap[addRegColoring->countOfMap].channelCount = enabledChannelCount;
                addRegColoring->tmp2addrMap[addRegColoring->countOfMap].indexedReg = indexedReg;
                addRegColoring->tmp2addrMap[addRegColoring->countOfMap].startChannelInAddressReg = (gctUINT8)startChannelInAddressReg;
                addRegColoring->tmp2addrMap[addRegColoring->countOfMap].startChannelInIndexedReg = startChannelInIndexedReg;
                CodeGen->current->addrRegColoring.countOfMap ++;
            }
            else
            {
                gcmASSERT(addRegColoring->tmp2addrMap[mapIdxToInsert].indexedReg == -1);

                addRegColoring->tmp2addrMap[mapIdxToInsert].channelCount = enabledChannelCount;
                addRegColoring->tmp2addrMap[mapIdxToInsert].indexedReg = indexedReg;
                addRegColoring->tmp2addrMap[mapIdxToInsert].startChannelInAddressReg = (gctUINT8)startChannelInAddressReg;
                addRegColoring->tmp2addrMap[mapIdxToInsert].startChannelInIndexedReg = startChannelInIndexedReg;
            }

            /* Now, add a "MOVAF a0, temp(x)" instruction. */
            for (i = startChannelInAddressReg, j = 0; i < startChannelInAddressReg + enabledChannelCount; i ++, j ++)
            {
                curAddrEnableMask |= (1 << i);
                indexedRegSwizzle |= ((startChannelInIndexedReg + j) << i*2);
            }

            gcmERR_BREAK(_CodeGenMOVA(Tree, CodeGen, indexedReg, formatOfIndexedReg, curAddrEnableMask, indexedRegSwizzle));
        }
    }
    while (gcvFALSE);

    if (!gcmIS_ERROR(status))
    {
        gctUINT channelInAddrReg =0;
        gctINT mapIdx;

        for (mapIdx = 0; mapIdx < CodeGen->current->addrRegColoring.countOfMap; mapIdx ++)
        {
            if (addRegColoring->tmp2addrMap[mapIdx].indexedReg == indexedReg)
            {
               /* Shift it to zero-based */
                gctINT adjustedIndexedChannelOfReg = IndexedChannelOfReg - 1;

                if ((adjustedIndexedChannelOfReg >= addRegColoring->tmp2addrMap[mapIdx].startChannelInIndexedReg) &&
                    (adjustedIndexedChannelOfReg <= (addRegColoring->tmp2addrMap[mapIdx].startChannelInIndexedReg +
                     addRegColoring->tmp2addrMap[mapIdx].channelCount - 1)))
                {
                    channelInAddrReg = (startChannelInAddressReg +
                        (IndexedChannelOfReg - addRegColoring->tmp2addrMap[mapIdx].startChannelInIndexedReg) - 1);
                    break;
                }
            }
        }
        gcmASSERT(channelInAddrReg <= 3);

        *singleAddrEnable = (1 << channelInAddrReg);

        /* Update local usage mask */
        addRegColoring->localAddrChannelUsageMask |= curAddrEnableMask;
    }

    return status;
}

static gceSTATUS
_ProcessDestination(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    gctINT Reference,
    IN OUT gctUINT32 * States,
    OUT gctINT_PTR Shift,
    IN gcsSL_PATTERN_PTR Pattern
    )
{
    gcsSL_REFERENCE_PTR match = gcvNULL;
    gcSL_INSTRUCTION instruction;
    gceSTATUS status = gcvSTATUS_OK;
    gcsSL_FUNCTION_CODE_PTR function = CodeGen->current;

    gctUINT8 addrEnable;
    gctUINT32 addrIndexed;

    addrEnable = 0;
    addrIndexed = gcSL_NOT_INDEXED;

    do
    {
        /* Find the reference. */
        if (!_FindReference(Tree, CodeGen, Reference, &match, Pattern))
        {
            gcmERR_BREAK(gcvSTATUS_NOT_FOUND);
        }

        /* Reference the instruction. */
        instruction = match->instruction;

        /* Check whether this is a destination. */
        if (match->sourceIndex == -1)
        {
            /* Get the referenced destination. */
            gctTARGET_t target  = instruction->temp;
            gctUINT opcode = gcmSL_OPCODE_GET(instruction->opcode, Opcode);
            gctBOOL skipConst = (opcode == gcSL_MOV &&
                Pattern->opcode != 0x09 &&
                gcmSL_OPCODE_GET(instruction->opcode, Sat) == gcSL_NO_SATURATE);
            gcSL_FORMAT targetFormat = gcmSL_TARGET_GET(target, Format);

            /* Check whether the target is not enabled . */
            if (gcmSL_TARGET_GET(target, Enable) ==  gcSL_ENABLE_NONE ||
                (opcode == gcSL_STORE1 &&
                 (targetFormat == gcSL_INT64 || targetFormat == gcSL_UINT64)))
            {
                /* do not allocate register for destination if not enabled */
                return gcvSTATUS_OK;
            }

            /* Check whether the target is indexed by a different register. */
            if (gcmSL_TARGET_GET(target, Indexed) != gcSL_NOT_INDEXED)
            {
                gcmERR_BREAK(_FindAddressRegChannel(Tree,
                                                    CodeGen,
                                                    instruction->tempIndexed,
                                                    gcmSL_TARGET_GET(target, Format),
                                                    gcmSL_TARGET_GET(target, Indexed),
                                                    &addrEnable
                                                    ));
            }

            /* Test if this is a MOV which could be a constant propagation. */
            if (skipConst
            &&  (gcmSL_TARGET_GET(target, Enable) & gcSL_ENABLE_X)
            &&  (instruction->tempIndex < Tree->tempCount)
            &&  (Tree->tempArray[instruction->tempIndex].constUsage[0] != 1)
            )
            {
                /* X is used but is not marked as a constant. */
                skipConst = gcvFALSE;
            }

            if (skipConst
            &&  (gcmSL_TARGET_GET(target, Enable) & gcSL_ENABLE_Y)
            &&  (instruction->tempIndex < Tree->tempCount)
            &&  (Tree->tempArray[instruction->tempIndex].constUsage[1] != 1)
            )
            {
                /* Y is used but is not marked as a constant. */
                skipConst = gcvFALSE;
            }

            if (skipConst
            &&  (gcmSL_TARGET_GET(target, Enable) & gcSL_ENABLE_Z)
            &&  (instruction->tempIndex < Tree->tempCount)
            &&  (Tree->tempArray[instruction->tempIndex].constUsage[2] != 1)
            )
            {
                /* Z is used but is not marked as a constant. */
                skipConst = gcvFALSE;
            }

            if (skipConst
            &&  (gcmSL_TARGET_GET(target, Enable) & gcSL_ENABLE_W)
            &&  (instruction->tempIndex < Tree->tempCount)
            &&  (Tree->tempArray[instruction->tempIndex].constUsage[3] != 1)
            )
            {
                /* W is used but is not marked as a constant. */
                skipConst = gcvFALSE;
            }

            if (skipConst)
            {
                /* Skip generation of MOV to constant propagation registers. */
                return gcvSTATUS_SKIP;
            }

            if (gcmSL_TARGET_GET(target, Indexed) != gcSL_NOT_INDEXED)
            {
                switch (addrEnable)
                {
                case 1:
                    addrIndexed = gcSL_INDEXED_X; break;
                case 2:
                    addrIndexed = gcSL_INDEXED_Y; break;
                case 4:
                    addrIndexed = gcSL_INDEXED_Z; break;
                case 8:
                    addrIndexed = gcSL_INDEXED_W; break;
                }
            }

            /* Program destination. */
            gcmERR_BREAK(_gcmSetDest(Tree,
                                     CodeGen,
                                     States,
                                     instruction->tempIndex,
                                     addrIndexed,
                                     gcmSL_TARGET_GET(target, Enable),
                                     gcmSL_TARGET_GET(target, Precision),
                                     Shift));
        }
        else
        {
            /* Get the referenced source. */
            gctSOURCE_t source = (match->sourceIndex == 0)
                ? instruction->source0
                : instruction->source1;

            gctUINT32 sourceIndex = (match->sourceIndex == 0)
                ? instruction->source0Index
                : instruction->source1Index;

            gctINT index = gcmSL_INDEX_GET(sourceIndex, Index);

            gcmASSERT(gcmSL_SOURCE_GET(source, Type)    == gcSL_TEMP ||
                      gcmSL_SOURCE_GET(source, Type)    == gcSL_ATTRIBUTE);
            gcmASSERT(gcmSL_SOURCE_GET(source, Indexed) == gcSL_NOT_INDEXED);

            if (gcmSL_SOURCE_GET(source, Type)    == gcSL_ATTRIBUTE)
            {
                /* set the index to be the attribute register location */
                 index  = - (Tree->shader->attributes[index]->inputIndex + 1);
            }

            /* Program destination. */
            gcmERR_BREAK(_gcmSetDest(Tree,
                                     CodeGen,
                                     States,
                                     index,
                                     0x0,
                                     _Swizzle2Enable((gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleX),
                                                     (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleY),
                                                     (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleZ),
                                                     (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleW)),
                                     gcmSL_SOURCE_GET(source, Precision),
                                     Shift));
        }

        /* Success. */
        status = gcvSTATUS_OK;
    }
    while (gcvFALSE);

    if (gcmNO_ERROR(status) && (*Shift == -1) )
    {
        switch (Reference)
        {
        case gcSL_CG_TEMP1_X:
        /* fall through */
        case gcSL_CG_TEMP1_XY:
        /* fall through */
        case gcSL_CG_TEMP1_XYZ:
        /* fall through */
        case gcSL_CG_TEMP1_XYZW:
            *Shift = function->tempShift[0];
            break;

        case gcSL_CG_TEMP2_X:
        /* fall through */
        case gcSL_CG_TEMP2_XY:
        /* fall through */
        case gcSL_CG_TEMP2_XYZ:
        /* fall through */
        case gcSL_CG_TEMP2_XYZW:
            *Shift = function->tempShift[1];
            break;

        case gcSL_CG_TEMP3_X:
        /* fall through */
        case gcSL_CG_TEMP3_XY:
        /* fall through */
        case gcSL_CG_TEMP3_XYZ:
        /* fall through */
        case gcSL_CG_TEMP3_XYZW:
            *Shift = function->tempShift[2];
            break;

        case gcSL_CG_TEMP1_X_NO_SRC_SHIFT:
        /* fall through */
        case gcSL_CG_TEMP2_X_NO_SRC_SHIFT:
        /* fall through */
        case gcSL_CG_TEMP3_X_NO_SRC_SHIFT:
        /* fall through */
        case gcSL_CG_TEMP1_XY_NO_SRC_SHIFT:
            *Shift = 0;
            break;

        default:
            break;
        }
    }

    /* Return the status. */
    return status;
}

/* return true if the value is in the SrcConstantInfo, and set the
 * Swizzle to the corresponding component's swizzle, otherwise
 * return false
 */
static gctBOOL
_valueInSrcConstantInfo(
    IN gcsSourceConstandInfo *  SrcConstantInfo,
    IN gctUINT                 Value,
    OUT gctUINT8 *              Swizzle
    )
{
    gctINT i;

    for (i = 0; i < SrcConstantInfo->constNo; i++)
    {
        if (*((gctUINT *)&SrcConstantInfo->srcValues[i]) == Value)
        {
            /* found the constant value, now need to get the swizzle */
            gctUINT8 compSwizzle = _ExtractSwizzle(SrcConstantInfo->swizzle, i);
            *Swizzle = gcmComposeSwizzle(compSwizzle, compSwizzle,
                                         compSwizzle, compSwizzle);
            return gcvTRUE;
        }
    }
    return gcvFALSE;
}

static gceSTATUS
_ProcessSource(
    IN gcLINKTREE               Tree,
    IN gcsCODE_GENERATOR_PTR    CodeGen,
    IN gctINT                   Reference,
    IN OUT gctUINT32 *          States,
    IN gctUINT                  Source,
    IN gctINT                   Shift,
    IN gcsSourceConstandInfo *  SrcConstantInfo
    )
{
    gcsSL_REFERENCE_PTR match = gcvNULL;
    gcSL_INSTRUCTION instruction;
    gceSTATUS status;
    gctUINT8 swizzle;
    gctUINT8 addrEnable;
    gctUINT32 addrIndexed;
    gcSL_PRECISION precision = gcSL_PRECISION_MEDIUM;

    addrEnable = 0;
    addrIndexed = gcSL_NOT_INDEXED;

    do
    {
        /* Special case constants. */
        if (gcmABS(Reference) == gcSL_CG_CONSTANT)
        {
            status = _gcmSetSource(Tree,
                                   CodeGen,
                                   States,
                                   Source,
                                   gcSL_CONSTANT,
                                   0,
                                   0,
                                   0x0,
                                   gcSL_SWIZZLE_XXXX,
                                   Reference < 0,
                                   gcvFALSE,
                                   precision);
            break;
        }

        /* Find the referenced instruction. */
        if (!_FindReference(Tree, CodeGen, gcmABS(Reference), &match, gcvNULL))
        {
            gcmERR_BREAK(gcvSTATUS_NOT_FOUND);
        }

        instruction = match->instruction;

        if (match->sourceIndex < 0)
        {
            /* Extract the referenced target. */
            gctTARGET_t target = instruction->temp;
            gctUINT8 targetEnable = gcmSL_TARGET_GET(target, Enable);

            /* For dest as source for some spliting case, if dest dost not
               start with x channel, we need shift it, for example
               sin r2.yz, r1.wwy =>

               mul r2.yz, r1.wwy, c0.x
               sin r2.yz, r2.yyz (can not be yz here)
            */
            gctINT shiftInternal = 0;

            gcmASSERT(targetEnable != 0);

            while (!((targetEnable >> shiftInternal) & 0x1))
            {
                shiftInternal ++;
            }

            if(_isHWRegisterAllocated(Tree->shader))
            {
                Shift =  (Shift < 0) ? shiftInternal : Shift;
            }
            else
            {
                Shift =  (Shift < 0) ? shiftInternal : (shiftInternal + Shift);
            }

            swizzle = _Enable2Swizzle(targetEnable);

            while (Shift-- > 0)
            {
                swizzle = (swizzle << 2) | (swizzle & 0x3);
            }

            /* Set the source opcode. */
            /* Check whether the target is indexed by a different register. */
            if (gcmSL_TARGET_GET(target, Indexed) != gcSL_NOT_INDEXED)
            {
                gcmERR_BREAK(_FindAddressRegChannel(Tree,
                                                    CodeGen,
                                                    instruction->tempIndexed,
                                                    gcmSL_TARGET_GET(target, Format),
                                                    gcmSL_TARGET_GET(target, Indexed),
                                                    &addrEnable
                                                    ));
            }

            if (gcmSL_TARGET_GET(target, Indexed) != gcSL_NOT_INDEXED)
            {
                switch (addrEnable)
                {
                case 1:
                    addrIndexed = gcSL_INDEXED_X; break;
                case 2:
                    addrIndexed = gcSL_INDEXED_Y; break;
                case 4:
                    addrIndexed = gcSL_INDEXED_Z; break;
                case 8:
                    addrIndexed = gcSL_INDEXED_W; break;
                }
            }

            precision = gcmSL_TARGET_GET(target, Precision);
            gcmERR_BREAK(_gcmSetSource(Tree,
                                       CodeGen,
                                       States,
                                       Source,
                                       gcSL_TEMP,
                                       instruction->tempIndex,
                                       0,
                                       addrIndexed,
                                       swizzle,
                                       Reference < 0,
                                       gcvFALSE,
                                       precision));
        }
        else
        {
            /* Extract the referenced source. */
            gctSOURCE_t source = (match->sourceIndex == 0)
                ? instruction->source0
                : instruction->source1;
            gctUINT32 index = (match->sourceIndex == 0)
                ? instruction->source0Index
                : instruction->source1Index;
            gctINT indexed = (match->sourceIndex == 0)
                ? instruction->source0Indexed
                : instruction->source1Indexed;
            gctBOOL neg = gcmSL_SOURCE_GET(source, Neg);
            gctBOOL abs = gcmSL_SOURCE_GET(source, Abs);

            precision = gcmSL_SOURCE_GET(source, Precision);

            /* not generated in the pattern, already register allocated,
               thus the shift is 0 */
            if (_isHWRegisterAllocated(Tree->shader))
            {
                Shift =  0;
            }

            /* See if the source is a constant. */
            if (gcmSL_SOURCE_GET(source, Type) == gcSL_CONSTANT)
            {
                gctINT        uniform;
                gcSL_FORMAT   format = gcmSL_SOURCE_GET(source, Format);

                /* Extract the float constant. */
                union
                {
                    gctFLOAT f;
                    gctUINT16 hex[2];
                    gctUINT32 hex32;
                }
                value;
                value.hex32 = (match->sourceIndex == 0)
                        ? (instruction->source0Index | (instruction->source0Indexed << 16))
                        : (instruction->source1Index | (instruction->source1Indexed << 16));

                if (Generate20BitsImmediate(CodeGen, instruction, Source) &&
                    ValueFit20Bits(format, value.hex32))
                {
                    gcsConstantValue constValue;
                    constValue.ty = format;
                    constValue.value.u = value.hex32;

                    /* the value of the constant fits into 20 bits, encode
                       it to immediate value in the instruction */
                    if (Reference < 0)
                        gcNegateValueFit20Bit(&constValue);

                    gcmERR_BREAK(gcEncodeSourceImmediate20(
                                            States,
                                            Source,
                                            &constValue));

                }
                else
                {
                    gcSL_TYPE type = gcSL_TEMP;
                    gctBOOL constFromUBO = gcvFALSE;

                    /* check if the constant is pre-allocated in SrcConstantInfo */
                    if (SrcConstantInfo &&
                        _valueInSrcConstantInfo(SrcConstantInfo, value.hex32, &swizzle))
                    {
                        uniform = SrcConstantInfo->uniformIndex;
                        constFromUBO = SrcConstantInfo->fromUBO;
                    }
                    else
                    {
                        /* Allocate the constant. */
                        gcmERR_BREAK(_AddConstantVec1(Tree,
                                                      CodeGen,
                                                      value.f,
                                                      &uniform,
                                                      &swizzle,
                                                      &type));
                        constFromUBO = gcvTRUE;
                    }

                    /* Set the source operand. */
                    gcmERR_BREAK(_gcmSetSource(Tree,
                                               CodeGen,
                                               States,
                                               Source,
                                               gcSL_CONSTANT,
                                               uniform,
                                               0,
                                               0x0,
                                               swizzle,
                                               Reference < 0,
                                               gcvFALSE,
                                               precision));

                    if (constFromUBO)
                    {
                        _UsingConstUniform(Tree, CodeGen, Source, uniform, swizzle, type, States);
                    }
                }
            }
            else
            {
                gctUINT constIndex;

                /* Check whether the source is indexed by a different register. */
                if (gcmSL_SOURCE_GET(source, Indexed) != gcSL_NOT_INDEXED)
                {
                    gcmERR_BREAK(_FindAddressRegChannel(Tree,
                                                        CodeGen,
                                                        indexed,
                                                        gcmSL_SOURCE_GET(source, Format),
                                                        gcmSL_SOURCE_GET(source, Indexed),
                                                        &addrEnable
                                                        ));
                }

                /* Get constant index. */
                constIndex = (gcmSL_SOURCE_GET(source, Indexed) == gcSL_NOT_INDEXED)
                           ? indexed + gcmSL_INDEX_GET(index, ConstValue)
                           : gcmSL_INDEX_GET(index, ConstValue);

                if (gcmSL_SOURCE_GET(source, Indexed) != gcSL_NOT_INDEXED)
                {
                    switch (addrEnable)
                    {
                    case 1:
                        addrIndexed = gcSL_INDEXED_X; break;
                    case 2:
                        addrIndexed = gcSL_INDEXED_Y; break;
                    case 4:
                        addrIndexed = gcSL_INDEXED_Z; break;
                    case 8:
                        addrIndexed = gcSL_INDEXED_W; break;
                    }
                }

                /* Compute shifted swizzle. */
                swizzle = gcmSL_SOURCE_GET(source, Swizzle);

                while (Shift-- > 0)
                {
                    swizzle = (swizzle << 2) | (swizzle & 0x3);
                }

                /* Set the source operand. */
                gcmERR_BREAK(_gcmSetSource(Tree,
                                           CodeGen,
                                           States,
                                           Source,
                                           (gcSL_TYPE) gcmSL_SOURCE_GET(source, Type),
                                           gcmSL_INDEX_GET(index, Index),
                                           constIndex,
                                           addrIndexed,
                                           swizzle,
                                           (Reference < 0) ^ neg,
                                           abs,
                                           precision));
            }
        }

        /* Success. */
        status = gcvSTATUS_OK;
    }
    while (gcvFALSE);

    /* Return the staus. */
    return status;
}

static gceSTATUS
_ProcessSampler(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    gctINT Reference,
    IN OUT gctUINT32 States[4]
    )
{
    gcsSL_REFERENCE_PTR match = gcvNULL;
    gcSL_INSTRUCTION instruction;
    gceSTATUS status;
    gctUINT32 index = 0;
    gctUINT32 indexed = 0, swizzle = 0, relative = 0;
    gctUINT8 addrEnable = 0;
    gctINT32 arrayLoc = 0;
    gctUINT srcFormat = gcSL_FLOAT;
    gctUINT srcType = gcSL_UNIFORM;

    do
    {
        /* Find the reference. */
        if (!_FindReference(Tree, CodeGen, Reference, &match, gcvNULL))
        {
            gcmERR_BREAK(gcvSTATUS_NOT_FOUND);
        }

        /* Reference the instruction. */
        instruction = match->instruction;

        /* Decode source. */
        switch (match->sourceIndex)
        {
        case 0:
            relative = gcmSL_SOURCE_GET(instruction->source0, Indexed);
            swizzle  = gcmSL_SOURCE_GET(instruction->source0, Swizzle);
            index    = instruction->source0Index;
            indexed  = instruction->source0Indexed;
            srcFormat = gcmSL_SOURCE_GET(instruction->source0, Format);
            srcType  = gcmSL_SOURCE_GET(instruction->source0, Type);

            arrayLoc = (gcmSL_SOURCE_GET(instruction->source0, Indexed) == gcSL_NOT_INDEXED)
                ? indexed + gcmSL_INDEX_GET(index, ConstValue)
                : gcmSL_INDEX_GET(index, ConstValue);
            break;

        case 1:
            gcmASSERT(match->sourceIndex == 0);
            relative = gcmSL_SOURCE_GET(instruction->source1, Indexed);
            swizzle  = gcmSL_SOURCE_GET(instruction->source1, Swizzle);
            index    = instruction->source1Index;
            indexed  = instruction->source1Indexed;
            srcFormat = gcmSL_SOURCE_GET(instruction->source1, Format);
            srcType  = gcmSL_SOURCE_GET(instruction->source1, Type);

            arrayLoc = (gcmSL_SOURCE_GET(instruction->source1, Indexed) == gcSL_NOT_INDEXED)
                ? indexed + gcmSL_INDEX_GET(index, ConstValue)
                : gcmSL_INDEX_GET(index, ConstValue);
            break;

        default:
            gcmFATAL("Sample cannot be a target???");
        }

        switch (relative)
        {
        case gcSL_NOT_INDEXED:
            relative = 0x0;
            break;

        case gcSL_INDEXED_X:
            if (Tree->tempArray[indexed].constUsage[0] == 1)
            {
                if (Tree->tempArray[indexed].format == gcSL_FLOAT)
                {
                    index = gcmSL_INDEX_SET(index,
                                               Index,
                                               gcmSL_INDEX_GET(index, Index)
                                               + (gctUINT32) Tree->tempArray[indexed].constValue[0].f);
                }
                else
                {
                    index = gcmSL_INDEX_SET(index,
                                               Index,
                                               gcmSL_INDEX_GET(index, Index)
                                               + Tree->tempArray[indexed].constValue[0].u);
                }
                relative = 0x0;
            }
            else
            {
                relative = 0x1;
            }
            break;

        case gcSL_INDEXED_Y:
            if (Tree->tempArray[indexed].constUsage[1] == 1)
            {
                if (Tree->tempArray[indexed].format == gcSL_FLOAT)
                {
                    index = gcmSL_INDEX_SET(index,
                                               Index,
                                               gcmSL_INDEX_GET(index, Index)
                                               + (gctUINT32) Tree->tempArray[indexed].constValue[1].f);
                }
                else
                {
                    index = gcmSL_INDEX_SET(index,
                                               Index,
                                               gcmSL_INDEX_GET(index, Index)
                                               + Tree->tempArray[indexed].constValue[1].u);
                }
                relative = 0x0;
            }
            else
            {
                relative = 0x2;
            }
            break;

        case gcSL_INDEXED_Z:
            if (Tree->tempArray[indexed].constUsage[2] == 1)
            {
                if (Tree->tempArray[indexed].format == gcSL_FLOAT)
                {
                    index = gcmSL_INDEX_SET(index,
                                               Index,
                                               gcmSL_INDEX_GET(index, Index)
                                               + (gctUINT32) Tree->tempArray[indexed].constValue[2].f);
                }
                else
                {
                    index = gcmSL_INDEX_SET(index,
                                               Index,
                                               gcmSL_INDEX_GET(index, Index)
                                               + Tree->tempArray[indexed].constValue[2].u);
                }
                relative = 0x0;
            }
            else
            {
                relative = 0x3;
            }
            break;

        case gcSL_INDEXED_W:
            if (Tree->tempArray[indexed].constUsage[3] == 1)
            {
                if (Tree->tempArray[indexed].format == gcSL_FLOAT)
                {
                    index = gcmSL_INDEX_SET(index,
                                               Index,
                                               gcmSL_INDEX_GET(index, Index)
                                               + (gctUINT32) Tree->tempArray[indexed].constValue[3].f);
                }
                else
                {
                    index = gcmSL_INDEX_SET(index,
                                               Index,
                                               gcmSL_INDEX_GET(index, Index)
                                               + Tree->tempArray[indexed].constValue[3].u);
                }
                relative = 0x0;
            }
            else
            {
                relative = 0x4;
            }
            break;

        default:
            break;
        }

        if (relative != 0x0)
        {
            gcmERR_BREAK(_FindAddressRegChannel(Tree,
                                                CodeGen,
                                                indexed,
                                                srcFormat,
                                                relative,
                                                &addrEnable
                                                ));

            switch (addrEnable)
            {
            case 1:
                relative = 0x1; break;
            case 2:
                relative = 0x2; break;
            case 4:
                relative = 0x3; break;
            case 8:
                relative = 0x4; break;
            }
        }

        /* Set sampler. */
        gcmERR_BREAK(_SetSampler(Tree,
                                 CodeGen,
                                 States,
                                 gcmSL_INDEX_GET(index, Index),
                                 relative,
                                 swizzle,
                                 srcType,
                                 arrayLoc));
    }
    while (gcvFALSE);

    /* Return the status. */
    return status;
}

static gceSTATUS
_SetTarget(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR   CodeGen,
    IN gcsSL_FUNCTION_CODE_PTR Function,
    IN gcSL_BRANCH_LIST        Branch,
    IN gctUINT                 Target
    )
{
    gcsSL_PHYSICAL_CODE_PTR code;
    gctUINT                 ip = Branch->ip;

    /* Find the code that needs to be patched. */
    for (code = Function->root; code != gcvNULL; code = code->next)
    {
        if (ip < code->count)
        {
            /* Get the opcode for the instruction. */
            gctUINT32 opcode = (((((gctUINT32) (code->states[ip * 4 + 0])) >> (0 ? 5:0)) & ((gctUINT32) ((((1 ? 5:0) - (0 ? 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1)))))) );

            if ((opcode != 0x14)
            &&  (opcode != 0x16)
            &&  (opcode != 0x24)
            )
            {
                /* Move to the next instruction - emit might have inserted
                   instructions! */
                ++ip;
            }
        }

        if (ip < code->count)
        {
            gctUINT32 *states     = &code->states[ip * 4];
            /* Patch the code. */
            if (!gcHWCaps.hwFeatureFlags.canBranchOnImm)
            {
                states[3] = ((((gctUINT32) (states[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:7) - (0 ?
 26:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:7) - (0 ?
 26:7) + 1))))))) << (0 ?
 26:7))) | (((gctUINT32) ((gctUINT32) (Target) & ((gctUINT32) ((((1 ?
 26:7) - (0 ?
 26:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:7) - (0 ? 26:7) + 1))))))) << (0 ? 26:7)));
                gcmASSERT(!Branch->duplicatedT0T1);
            }
            else
            {
                gcsConstantValue value;

                /* the branch and call target is an immediate number in Source2 */
                gcConvert20BitImmediateTo32Bit(Target,
                                               0x2,
                                               &value);
                gcEncodeSourceImmediate20(states, 2, &value);

                /* patch the duplicated branch states */
                if (Branch->duplicatedT0T1)
                {
                    gctUINT32  threadMode = ((((((gctUINT32) (states[3])) >> (0 ? 24:24)) & ((gctUINT32) ((((1 ? 24:24) - (0 ? 24:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 24:24) - (0 ? 24:24) + 1)))))) ) << 1) |
                                             (((((gctUINT32) (states[3])) >> (0 ? 13:13)) & ((gctUINT32) ((((1 ? 13:13) - (0 ? 13:13) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 13:13) - (0 ? 13:13) + 1)))))) );

                    if (threadMode != 0x0)
                    {
                        states += 4;
                        gcmASSERT((((((gctUINT32) (states[0])) >> (0 ? 5:0)) & ((gctUINT32) ((((1 ? 5:0) - (0 ? 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1)))))) ) == 0x16) ;

                        gcEncodeSourceImmediate20(states, 2, &value);
                    }
                }
            }
            /* Success. */
            return gcvSTATUS_OK;
        }

        /* Next list in the code. */
        ip -= code->count;
    }

    /* Index not found. */
    return gcvSTATUS_INVALID_INDEX;
}

static gctUINT32
_gc2shCondition(
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
    case gcSL_ZERO:
        return 0x0B;

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
        gcmFATAL("Unknown condition: %X", Condition);
        return 0x00;
    }
}

typedef enum _gceINSTRUCTION_TYPE
{
    gcvINSTRUCTION_NORMAL,
    gcvINSTRUCTION_TRANSCENDENTAL_ONE_SOURCE,
    gcvINSTRUCTION_TRANSCENDENTAL_TWO_SOURCES
}
gceINSTRUCTION_TYPE;

static gceINSTRUCTION_TYPE
_GetInstructionType(
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction
    )
{
    switch (gcmSL_OPCODE_GET(Instruction->opcode, Opcode))
    {
    case gcSL_RCP:
        /* fall through */
    case gcSL_RSQ:
        /* fall through */
    case gcSL_EXP:
        /* fall through */
    case gcSL_LOG:
        /* fall through */
    case gcSL_SQRT:
        return gcvINSTRUCTION_TRANSCENDENTAL_ONE_SOURCE;

    case gcSL_MUL:
        if (gcmSL_TARGET_GET(Instruction->temp, Format) == gcSL_FLOAT)
        {
            return gcvINSTRUCTION_NORMAL;
        }

        /* fall through */
    case gcSL_MULLO:
        /* fall through */
    case gcSL_MULHI:
        /* fall through */
    case gcSL_MULSAT:
        if (CodeGen->isCL_XE)
        {
            gcSL_FORMAT format = (gcSL_FORMAT) gcmSL_TARGET_GET(Instruction->temp, Format);

            if (format != gcSL_INT32 && format != gcSL_UINT32)
            {
                return gcvINSTRUCTION_NORMAL;
            }
        }
        return gcvINSTRUCTION_TRANSCENDENTAL_TWO_SOURCES;

    case gcSL_NOT_BITWISE:
        /* fall through */
    case gcSL_LEADZERO:
        /* fall through */
    case gcSL_CONV:
        /* fall through */
    case gcSL_I2F:
        /* fall through */
    case gcSL_F2I:
        if (CodeGen->isCL_X)
        {
            return gcvINSTRUCTION_TRANSCENDENTAL_ONE_SOURCE;
        }
        return gcvINSTRUCTION_NORMAL;

    case gcSL_DIV:
        /* fall through */
    case gcSL_MOD:
        return gcvINSTRUCTION_TRANSCENDENTAL_TWO_SOURCES;

    case gcSL_ADD:
        /* fall through */
    case gcSL_SUB:
        if (gcmSL_TARGET_GET(Instruction->temp, Format) == gcSL_FLOAT)
        {
            return gcvINSTRUCTION_NORMAL;
        }

        /* fall through */
    case gcSL_AND_BITWISE:
        /* fall through */
    case gcSL_OR_BITWISE:
        /* fall through */
    case gcSL_XOR_BITWISE:
        /* fall through */
    case gcSL_LSHIFT:
        /* fall through */
    case gcSL_RSHIFT:
        /* fall through */
    case gcSL_ROTATE:
        /* fall through */
    case gcSL_ADDLO:
        /* fall through */
    case gcSL_ADDSAT:
        /* fall through */
    case gcSL_SUBSAT:
        if (CodeGen->isCL_X)
        {
            return gcvINSTRUCTION_TRANSCENDENTAL_TWO_SOURCES;
        }

        /* fall through */
    default:
        return gcvINSTRUCTION_NORMAL;
    }
}


static void
_allocateRegForLiveVaraibles(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctINT     InstIdx)
{
    gcLINKTREE_TEMP_LIST temp = Tree->hints[InstIdx].liveTemps;;

    gcmASSERT(temp != gcvNULL &&
              Tree->hints[InstIdx].callers != gcvNULL);

    if (gcSHADER_DumpCodeGenVerbose(Tree->shader))
        gcoOS_Print("allocate registers at loop/func head %d", InstIdx);

    /* go through each caller to find if there is any backward jump */
    for (;
         temp != gcvNULL;
         temp = temp->next)
    {
        if (temp->temp->isPaired64BitUpper) continue;
        if (temp->temp->assigned == -1 && temp->temp->inUse)
        {
            _AllocateRegisterForTemp(Tree, CodeGen, temp->temp);
        }
    }

    if (gcSHADER_DumpCodeGenVerbose(Tree->shader))
        gcoOS_Print("end of allocate registers at loop/func head %d", InstIdx);

    return;
}

/* return 1 if the SourceNo is constant, and the constant value
 * is stored at ConstValue, otherwise return 0; */
static gctINT
_getSourceConstant(
    IN gcLINKTREE                  Tree,
    IN gcsCODE_GENERATOR_PTR       CodeGen,
    IN gcsSL_PATTERN_PTR           Pattern,
    IN gctINT                      SourceNo,
    OUT gctUINT *                 ConstValue
                   )
{
    gcsSL_REFERENCE_PTR match = gcvNULL;
    gcSL_INSTRUCTION    instruction;
    gctINT              reference;
    gctINT              retval = 0;

    reference = SourceNo == 0 ? Pattern->source0 :
                SourceNo == 1 ? Pattern->source1 : Pattern->source2 ;

    if (gcmABS(reference) == gcSL_CG_CONSTANT)
    {
        return 0;
    }

    /* Find the referenced instruction. */
    if (!_FindReference(Tree, CodeGen, gcmABS(reference), &match, gcvNULL))
    {
        return 0;
    }

    instruction = match->instruction;

    {
        /* Extract the referenced source. */
        gctSOURCE_t source = (match->sourceIndex == 0)
            ? instruction->source0
            : instruction->source1;
        gctUINT32 index = (match->sourceIndex == 0)
            ? instruction->source0Index
            : instruction->source1Index;
        gctINT16 indexed = (match->sourceIndex == 0)
            ? instruction->source0Indexed
            : instruction->source1Indexed;

        /* See if the source is a constant. */
        if (gcmSL_SOURCE_GET(source, Type) == gcSL_CONSTANT)
        {
            gcSL_FORMAT   format = gcmSL_SOURCE_GET(source, Format);

            /* Extract the float constant. */
            union
            {
                gctFLOAT f;
                gctUINT16 hex[2];
                gctUINT32 hex32;
            }
            value;
            value.hex32 = (index & 0xFFFF) | (indexed << 16);
            if (Generate20BitsImmediate(CodeGen, instruction, SourceNo) &&
                ValueFit20Bits(format, value.hex32))
            {
                /* the constant can fit in immediate field,
                   do not put into uniform */
                retval = 0;
            }
            else {
                /* return the constant. */
                *ConstValue = value.hex32;
                retval = 1;
            }
        }
    }
    return retval;
}

static gctBOOL
_instUses2Constant(
    IN gcLINKTREE                  Tree,
    IN gcsCODE_GENERATOR_PTR       CodeGen,
    IN gcsSL_PATTERN_PTR           Pattern,
    IN OUT gcsSourceConstandInfo * SrcConstantInfo
    )
{
    gctBOOL    retVal  = gcvFALSE;
    gctINT     constNo = 0;  /* number of constant */
    gctUINT * valPtr  = (gctUINT *)(&SrcConstantInfo->srcValues[0]);

    /* Process source 0. */
    if (Pattern->source0 != 0 )
    {
        constNo += _getSourceConstant(Tree,
                                      CodeGen,
                                      Pattern,
                                      0,
                                      &valPtr[constNo]);
    }

    /* Process source 1. */
    if (Pattern->source1 != 0)
    {
        constNo += _getSourceConstant(Tree,
                                      CodeGen,
                                      Pattern,
                                      1,
                                      &valPtr[constNo]);
        /* check if the values are duplicate */
        if (constNo == 2 && valPtr[0] == valPtr[1])
        {
            constNo--;
        }
    }
    /* Process source 2. */
    if (Pattern->source2 != 0)
    {
        constNo += _getSourceConstant(Tree,
                                      CodeGen,
                                      Pattern,
                                      2,
                                      &valPtr[constNo]);
        /* check if the values are duplicate */
        if (constNo == 2 && valPtr[0] == valPtr[1])
        {
            constNo--;
        }
        else if (constNo == 3 && (valPtr[0] == valPtr[2] ||
                                  valPtr[1] == valPtr[2]))
        {
            constNo--;
        }
    }

    if (constNo == 2)
    {
        gctINT   uniform;
        gctUINT8 swizzle;
        gcSL_TYPE type;
        /* Allocate the vec2 constant. */
        if (_AddConstantVec2(Tree, CodeGen,
                             SrcConstantInfo->srcValues[0],
                             SrcConstantInfo->srcValues[1],
                             &uniform, &swizzle, &type) == gcvSTATUS_OK)
        {
            if (type == gcSL_UNIFORM)
            {
                SrcConstantInfo->fromUBO = gcvFALSE;
            }
            else
            {
                SrcConstantInfo->fromUBO = gcvTRUE;
            }

            SrcConstantInfo->uniformIndex = uniform;
            SrcConstantInfo->swizzle      = swizzle;
            SrcConstantInfo->constNo      = 2;
            retVal = gcvTRUE;
        }
    }
    else if (constNo == 3)
    {
        gctINT   uniform;
        gctUINT8 swizzle;
        gcSL_TYPE type;
        /* Allocate the vec3 constant. */
        if (_AddConstantVec3(Tree, CodeGen,
                             SrcConstantInfo->srcValues[0],
                             SrcConstantInfo->srcValues[1],
                             SrcConstantInfo->srcValues[2],
                             &uniform, &swizzle, &type) == gcvSTATUS_OK)
        {
            if (type == gcSL_UNIFORM)
            {
                SrcConstantInfo->fromUBO = gcvFALSE;
            }
            else
            {
                SrcConstantInfo->fromUBO = gcvTRUE;
            }

            SrcConstantInfo->uniformIndex = uniform;
            SrcConstantInfo->swizzle      = swizzle;
            SrcConstantInfo->constNo      = 3;
            retVal = gcvTRUE;
        }
    }
    return retVal;
}

#define _IsNotMulFracPattern(Tree, CodeGen, Instruction, Count)  \
    (gcmSL_OPCODE_GET((Instruction)->opcode, Opcode) != gcSL_MUL || (Count) == 1 || \
     gcmSL_OPCODE_GET(((Instruction) + 1)->opcode, Opcode) != gcSL_FRAC  || \
     ((Tree)->tempArray[(Instruction)->tempIndex].users != gcvNULL && \
      (Tree)->tempArray[(Instruction)->tempIndex].users->next == gcvNULL && \
         (gcmSL_SOURCE_GET((Instruction)->source0, Type) != gcSL_CONSTANT && \
          gcmSL_SOURCE_GET((Instruction)->source1, Type) != gcSL_CONSTANT)))

static gceSTATUS
_GenerateFunction(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctPOINTER Function,
    IN gctBOOL isKernelFunction
    )
{
    gctUINT codeBase;
    gctSIZE_T codeCount, i, curInstIdx;
    gcSHADER shader = Tree->shader;
    gceSTATUS status = gcvSTATUS_OK;
    gcSL_PRECISION precision = gcSL_PRECISION_MEDIUM;
    gctINT duplicateEmitForT1SourceAddr;

    /* Find function. */
    if (Function == gcvNULL)
    {
        CodeGen->current = &CodeGen->functions[0];
    }
    else
    {
        if (isKernelFunction) {

            for (i = 0; i < shader->kernelFunctionCount; ++i)
            {
                if (Function == shader->kernelFunctions[i])
                {
                    CodeGen->current = &CodeGen->functions[i + 1 + shader->functionCount];
                    break;
                }
            }
            gcmASSERT(i < shader->kernelFunctionCount);

        } else {

            for (i = 0; i < shader->functionCount; ++i)
            {
                if (Function == shader->functions[i])
                {
                    CodeGen->current = &CodeGen->functions[i + 1];
                    break;
                }
            }
            gcmASSERT(i < shader->functionCount);
        }
    }

    /* Check if function has already been generated. */
    if (CodeGen->current->root != gcvNULL)
    {
        return gcvSTATUS_OK;
    }

    /* Reset instruction pointer for current function. */
    CodeGen->current->ip        = 0;
    CodeGen->current->addrRegColoring.countOfMap = 0;

    if (Function == gcvNULL)
    {
        codeBase  = 0;
        codeCount = shader->codeCount;

        /* Allocate register for position. */
        if (CodeGen->usePosition)
        {
            do
            {
                gctINT shift = 0;
                gctUINT8 swizzle;
                gctUINT32 states[4];
                gctINT positionW = -1;

                for (i = 0; i < Tree->attributeCount; ++i)
                {
                    gcATTRIBUTE attribute = Tree->shader->attributes[i];

                    if ((attribute != gcvNULL)
                    &&  (attribute->nameLength == gcSL_POSITION_W)
                    )
                    {
                        positionW = attribute->inputIndex;
                    }
                }

                if (positionW == -1)
                {
                    CodeGen->positionPhysical = 0;
                }
                else
                {
                    /* Allocate one vec4 register. */
                    gcmERR_BREAK(_FindRegisterUsage(CodeGen->registerUsage,
                                            CodeGen->registerCount,
                                            gcSHADER_FLOAT_X4,
                                            1,
                                            gcvSL_RESERVED,
                                            gcvFALSE,
                                            (gctINT_PTR)&CodeGen->positionPhysical,
                                            &swizzle,
                                            &shift,
                                            gcvNULL,
                                            0));

                    gcCGUpdateMaxRegister(CodeGen, CodeGen->positionPhysical, Tree);

                    /* MOV temp, r0 */
                    states[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ?
 10:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:6) - (0 ?
 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x00 & ((gctUINT32) ((((1 ?
 10:6) - (0 ?
 10:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (CodeGen->positionPhysical) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (gcSL_ENABLE_XYZW) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));

                    states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));

                    states[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));

                    states[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (gcSL_SWIZZLE_XYZW) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:25) - (0 ?
 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));

                    gcmERR_BREAK(_Emit(Tree, CodeGen, states));

                    /* RCP temp.w, attribute(#POSITION_W).x */
                    states[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) (0x0C & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ?
 10:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:6) - (0 ?
 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x00 & ((gctUINT32) ((((1 ?
 10:6) - (0 ?
 10:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (CodeGen->positionPhysical) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (gcSL_ENABLE_W) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));

                    states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));

                    states[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));

                    states[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (positionW) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (gcSL_SWIZZLE_XXXX) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:25) - (0 ?
 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));

                    gcmERR_BREAK(_Emit(Tree, CodeGen, states));
                }
            }
            while (gcvFALSE);

            if (gcmIS_ERROR(status))
            {
                return status;
            }
        }

        /* Allocate register for face. */
        if (CodeGen->useFace)
        {
            do
            {
                gctINT shift = 0;
                gctUINT8 enable;
                gctUINT32 states[4];

                /* Allocate one float register. */
                gcmERR_BREAK(_FindRegisterUsage(CodeGen->registerUsage,
                                        CodeGen->registerCount,
                                        gcSHADER_FLOAT_X1,
                                        1,
                                        gcvSL_RESERVED,
                                        gcvFALSE,
                                        (gctINT_PTR)&CodeGen->facePhysical,
                                        &CodeGen->faceSwizzle,
                                        &shift,
                                        &enable,
                                        0));

                gcCGUpdateMaxRegister(CodeGen, CodeGen->facePhysical, Tree);

                /* SET.NOT temp.enable, face.x */
                states[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) (0x10 & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ?
 10:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:6) - (0 ?
 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x0A & ((gctUINT32) ((((1 ?
 10:6) - (0 ?
 10:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (CodeGen->facePhysical) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (enable) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));

                states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:12) - (0 ?
 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ? 20:12)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:22) - (0 ?
 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (gcSL_SWIZZLE_XXXX) & ((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ? 29:22)));

                states[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));

                states[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)));

                gcmERR_BREAK(_Emit(Tree, CodeGen, states));
            }
            while (gcvFALSE);

            if (gcmIS_ERROR(status))
            {
                return status;
            }
        }

        /* Modify PointCoord. */
        if (CodeGen->usePointCoord)
        {
            do
            {
                gctUINT32 states[4];
                gctINT index;
                gctUINT8 swizzle;
                gcSL_TYPE type;

                gcmERR_BREAK(_AddConstantVec1(Tree,
                                              CodeGen,
                                              1.0f,
                                              &index,
                                              &swizzle,
                                              &type));

                /* ADD pointCoord.y, 1, -pointCoord.y */
                states[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ?
 10:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:6) - (0 ?
 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x00 & ((gctUINT32) ((((1 ?
 10:6) - (0 ?
 10:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (CodeGen->pointCoordPhysical) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (gcSL_ENABLE_Y) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));

                states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:12) - (0 ?
 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (index) & ((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ? 20:12)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:22) - (0 ?
 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ? 29:22)));

                states[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (type == gcSL_UNIFORM ?
 0x2 : 0x0) & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));

                states[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (CodeGen->pointCoordPhysical) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (gcSL_SWIZZLE_YYYY) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:25) - (0 ?
 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:22) - (0 ?
 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ? 22:22)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));

                gcmERR_BREAK(_Emit(Tree, CodeGen, states));
            }
            while (gcvFALSE);

            if (gcmIS_ERROR(status))
            {
                return status;
            }
        }
    }
    else
    {
        if (isKernelFunction) {
            codeBase  = ((gcKERNEL_FUNCTION)Function)->codeStart;
            codeCount = ((gcKERNEL_FUNCTION)Function)->codeEnd -
                        ((gcKERNEL_FUNCTION)Function)->codeStart;
        } else {
            codeBase  = ((gcFUNCTION)Function)->codeStart;
            codeCount = ((gcFUNCTION)Function)->codeCount;
        }
    }

    /* Process all instructions. */
    for (curInstIdx = 0; curInstIdx < codeCount;)
    {
        /* Extract the instruction. */
        gcSL_INSTRUCTION instruction = &shader->code[codeBase + curInstIdx];
        gctSIZE_T j;
        gcsSL_PATTERN_PTR pattern;
        gctINT count;
        gceINSTRUCTION_TYPE instructionType;
        gctBOOL bDeferredCleanupUsage = gcvFALSE;
        gctUINT opcode;

        /* If this is the main function, we have to ignore any instructions
           that live inside a function. */
        if ((Function == gcvNULL) && (Tree->hints[codeBase + curInstIdx].owner != gcvNULL))
        {
            /* skip the code in the function */
            if (Tree->hints[curInstIdx].isOwnerKernel) {
                curInstIdx += ((gcKERNEL_FUNCTION)Tree->hints[curInstIdx].owner)->codeEnd -
                     ((gcKERNEL_FUNCTION)Tree->hints[curInstIdx].owner)->codeStart;
            } else {
                curInstIdx += ((gcFUNCTION)Tree->hints[curInstIdx].owner)->codeCount;
            }
            continue;
        }

        /* Clean the previous code and previous index if this is an instruction reachable by
        ** jump. */
        if (Tree->hints[codeBase + curInstIdx].callers != gcvNULL)
        {
            CodeGen->previousCode = gcvNULL;
            CodeGen->current->addrRegColoring.countOfMap = 0;
        }

        /* if the instruction is the target of backward jump, or function
           head, we need to allocate registers for live variables entering
           the instruction */
        if (Tree->hints[codeBase + curInstIdx].liveTemps != gcvNULL)
        {
            if (!_isHWRegisterAllocated(Tree->shader))
            {
                _allocateRegForLiveVaraibles(Tree, CodeGen, codeBase + curInstIdx);

                if (CodeGen->isRegOutOfResource)
                {
                    status = gcvSTATUS_OUT_OF_RESOURCES;
                    break;
                }
            }
        }

        /* Skip NOP instructions. */
        if (gcmSL_OPCODE_GET(instruction->opcode, Opcode) == gcSL_NOP)
        {
            CodeGen->codeMap[codeBase + curInstIdx].function = CodeGen->current;
            CodeGen->codeMap[codeBase + curInstIdx].location = CodeGen->current->ip;
            ++curInstIdx;
            continue;
        }

        /* Find a code pattern. */
        count   = (gctINT) (codeCount - curInstIdx);
        pattern = _FindPattern(Tree,
                               CodeGen,
                               patterns,
                               instruction,
                               &count);
        if (pattern == gcvNULL)
            return gcvSTATUS_INVALID_ARGUMENT;

        for (j = 0; j < (gctSIZE_T) count; ++j)
        {
            /* Save physical IP for generated instruction. */
            CodeGen->codeMap[codeBase + curInstIdx + j].function = CodeGen->current;
            CodeGen->codeMap[codeBase + curInstIdx + j].location = CodeGen->current->ip;
            _ResetAddressRegChannel(Tree, CodeGen, &shader->code[codeBase + curInstIdx + j]);
        }

        if (gcmSL_OPCODE_GET(instruction->opcode, Opcode) == gcSL_CALL)
        {
            gcsSL_FUNCTION_CODE_PTR current = CodeGen->current;
            gctBOOL isKernelFunction = gcvFALSE;

            for (j = 0; j < shader->functionCount; ++j)
            {
                if (shader->functions[j]->codeStart == instruction->tempIndex)
                {
                    break;
                }
            }

            if (j == shader->functionCount) {

                for (j = 0; j < shader->kernelFunctionCount; ++j)
                {
                    if (shader->kernelFunctions[j]->codeStart == instruction->tempIndex)
                    {
                        isKernelFunction = gcvTRUE;
                        break;
                    }
                }
                gcmASSERT(j < shader->kernelFunctionCount);
            }

            gcmASSERT(isKernelFunction || j < shader->functionCount);

            current->branch = Tree->branch;
            Tree->branch    = gcvNULL;

            if (isKernelFunction) {
                gcmERR_BREAK(_GenerateFunction(Tree,
                                               CodeGen,
                                               shader->kernelFunctions[j],
                                               gcvTRUE));
            } else {
                gcmERR_BREAK(_GenerateFunction(Tree,
                                               CodeGen,
                                               shader->functions[j],
                                               gcvFALSE));
            }

            CodeGen->current = current;
            Tree->branch     = current->branch;
        }

        /* Transcendental instruction will be splited to several instructions based on enabled component */
        /* count, so if instruction like RCP r0.xy, r0.yx is generated, we can not assign same register */
        /* to dest and source. To do this, we dont clean up its source usage before dst register assigned */
        instructionType = _GetInstructionType(CodeGen, instruction);
        if (instructionType != gcvINSTRUCTION_NORMAL)
        {
            gctUINT8 enableDst = gcmSL_TARGET_GET(instruction->temp, Enable);
            gctUINT16 channelIdx = 0;

            /* Skip unused ones */
            while (!((enableDst >> channelIdx) & 0x01))
            {
                channelIdx ++;
            }

            /* Move next valid one */
            channelIdx ++;

            while (channelIdx < 4)
            {
                gctUINT16 channelIdxPre;
                gcSL_SWIZZLE swizzle;

                if (!((enableDst >> channelIdx) & 0x01))
                {
                    ++channelIdx;
                    continue;
                }

                if (gcmSL_SOURCE_GET(instruction->source0, Type) == gcSL_TEMP ||
                    gcmSL_SOURCE_GET(instruction->source0, Type) == gcSL_ATTRIBUTE)
                {
                    swizzle = _SelectSwizzle(channelIdx, instruction->source0);
                    for (channelIdxPre = 0; channelIdxPre < channelIdx; channelIdxPre ++)
                    {
                        if (((enableDst >> channelIdxPre) & 0x01) && channelIdxPre == swizzle)
                        {
                            bDeferredCleanupUsage = gcvTRUE;
                            break;
                        }
                    }
                    if (channelIdx == 3 || bDeferredCleanupUsage)
                    {
                        break;
                    }
                }

                if (instructionType == gcvINSTRUCTION_TRANSCENDENTAL_TWO_SOURCES
                &&  (gcmSL_SOURCE_GET(instruction->source1, Type) == gcSL_TEMP ||
                     gcmSL_SOURCE_GET(instruction->source1, Type) == gcSL_ATTRIBUTE))
                {
                    swizzle = _SelectSwizzle(channelIdx, instruction->source1);
                    for (channelIdxPre = 0; channelIdxPre < channelIdx; channelIdxPre ++)
                    {
                        if (((enableDst >> channelIdxPre) & 0x01) && channelIdxPre == swizzle)
                        {
                            bDeferredCleanupUsage = gcvTRUE;
                            break;
                        }
                    }

                    if (channelIdx == 3 || bDeferredCleanupUsage)
                    {
                        break;
                    }
                }

                channelIdx ++;
            }
        }

        /* Save index for next source. */
        CodeGen->nextSource = codeBase + curInstIdx + count;

        duplicateEmitForT1SourceAddr = -1;
        if(CodeGen->shaderType == gcSHADER_TYPE_FRAGMENT &&
           CodeGen->isDual16Shader) {
           if(CodeGen->usePosition) {
               if (gcmSL_OPCODE_GET(instruction->opcode, Opcode) != gcSL_NORM &&
                   ((gcmSL_SOURCE_GET(instruction->source0, Type) == gcSL_ATTRIBUTE && instruction->source0Index == (gctUINT32)CodeGen->positionIndex) ||
                   (gcmSL_SOURCE_GET(instruction->source1, Type) == gcSL_ATTRIBUTE && instruction->source1Index == (gctUINT32)CodeGen->positionIndex)) &&
                   _IsNotMulFracPattern(Tree, CodeGen, instruction, count)) { /* check for sources being gl_FragCoord */
                   gcATTRIBUTE attribute = Tree->shader->attributes[CodeGen->positionIndex];
                   gcmASSERT(attribute->inputIndex != -1);
                   duplicateEmitForT1SourceAddr = attribute->inputIndex;
               }
           }
           if(duplicateEmitForT1SourceAddr == -1) {
               switch(gcmSL_OPCODE_GET(instruction->opcode, Opcode)) {
               case gcSL_STORE:
                   duplicateEmitForT1SourceAddr = instruction->tempIndex;
                   break;

               case gcSL_LOAD:
                   duplicateEmitForT1SourceAddr = CodeGen->registerCount + 1;
                   break;

               default:
                   break;
               }
           }
        }

        while (pattern->count < 0)
        {
            gctINT shift = 0;
            gctUINT32 states[4];
            gctBOOL skip, emit;
            gcsSourceConstandInfo * srcConstantInfo = gcvNULL;
            gcsSourceConstandInfo   srcConstant;
            CodeGen->current->addrRegColoring.localAddrChannelUsageMask = 0;

            if (pattern->count == -1 && !bDeferredCleanupUsage)
            {
                while (count-- > 0)
                {
                    /* Clean up the register usage. */
                    _UpdateRegisterUsage(CodeGen->registerUsage,
                                 CodeGen->registerCount,
                                 codeBase + curInstIdx++);
                }
            }

            /* Zero out the generated instruction. */
            gcoOS_ZeroMemory(states, sizeof(states));

            /* Set opcode. */
            _SetOpcode(states,
                       pattern->opcode,
                       _gc2shCondition(
                           (gcSL_CONDITION)
                           gcmSL_TARGET_GET(instruction->temp, Condition)),
                       (((gcSL_FORMAT)gcmSL_TARGET_GET(instruction->temp, Format)) == gcSL_FLOAT) ?
                           gcmSL_OPCODE_GET(instruction->opcode, Sat) : 0);

            /* allocate vec2 constant if the instruction uses two constants */
            if (_instUses2Constant(Tree, CodeGen, pattern, &srcConstant) != gcvFALSE)
            {
                srcConstantInfo = &srcConstant;
            }

            /* Process destination. */
            if (pattern->dest != 0)
            {
                gcmERR_BREAK(_ProcessDestination(Tree,
                                                 CodeGen,
                                                 pattern->dest,
                                                 states,
                                                 &shift,
                                                 pattern));

                switch (pattern->opcode)
                {
                case 0x73:
                case 0x05:
                    /* fall through */
                case 0x06:
                case 0x74:
                case 0x75:
                    /* fall through */
                case 0x76:

                    /* fall through */
                case 0x67:

                case 0x37:

                case 0x38:

/* texld*/
                case 0x18:

                case 0x19:

                case 0x6F:

                case 0x1B:

                case 0x1A:

                case 0x70:

                case 0x7D:

                case 0x7B:
                    shift = 0;
                    break;

                default:
                    break;
                }

                switch (instruction->opcode)
                {
                /*
                ** The HW opcode for these instructions is saved by extended opcode,
                ** so we use gcSL opcode to detect.
                */
                case gcSL_TEXLODQ:
                case gcSL_TEXFETCH_MS:
                    shift = 0;
                    break;
                default:
                    break;
                }

                skip = (status == gcvSTATUS_SKIP);

                if (CodeGen->isRegOutOfResource)
                {
                    status = gcvSTATUS_OUT_OF_RESOURCES;
                    break;
                }

            }
            else
            {
                skip = gcvFALSE;
            }

            if (bDeferredCleanupUsage)
            {
                if (pattern->count == -1)
                {
                    while (count-- > 0)
                    {
                        /* Clean up the register usage. */
                        _UpdateRegisterUsage(CodeGen->registerUsage,
                                     CodeGen->registerCount,
                                     codeBase + curInstIdx++);
                    }
                }
            }

            /* Process source 0. */
            if (!skip && (pattern->source0 != 0))
            {
                gcmERR_BREAK(_ProcessSource(Tree,
                                            CodeGen,
                                            pattern->source0,
                                            states,
                                            0,
                                            shift,
                                            srcConstantInfo));
            }

            if (CodeGen->isConstOutOfMemory)
            {
                status = gcvSTATUS_TOO_MANY_UNIFORMS;
                break;
            }

            if (CodeGen->isRegOutOfResource)
            {
                status = gcvSTATUS_OUT_OF_RESOURCES;
                break;
            }

            /* Process source 1. */
            if (!skip && (pattern->source1 != 0))
            {
                gcmERR_BREAK(_ProcessSource(Tree,
                                            CodeGen,
                                            pattern->source1,
                                            states,
                                            1,
                                            shift,
                                            srcConstantInfo));
            }

            if (CodeGen->isConstOutOfMemory)
            {
                status = gcvSTATUS_TOO_MANY_UNIFORMS;
                break;
            }

            if (CodeGen->isRegOutOfResource)
            {
                status = gcvSTATUS_OUT_OF_RESOURCES;
                break;
            }

            /* Process source 2. */
            if (!skip && (pattern->source2 != 0))
            {
                gcmERR_BREAK(_ProcessSource(Tree,
                                            CodeGen,
                                            pattern->source2,
                                            states,
                                            2,
                                            shift,
                                            srcConstantInfo));
            }

            if (CodeGen->isConstOutOfMemory)
            {
                status = gcvSTATUS_TOO_MANY_UNIFORMS;
                break;
            }

            if (CodeGen->isRegOutOfResource)
            {
                status = gcvSTATUS_OUT_OF_RESOURCES;
                break;
            }

            /* Process sampler. */
            if (!skip && (pattern->sampler != 0))
            {
                gcmERR_BREAK(_ProcessSampler(Tree,
                                             CodeGen,
                                             pattern->sampler,
                                             states));
            }

            /* Process user function. */
            if (!skip && (pattern->function != gcvNULL))
            {
                emit = (*pattern->function)(Tree,
                                            CodeGen,
                                            instruction,
                                            states);
            }
            else
            {
                emit = gcvTRUE;
            }

            if (CodeGen->isConstOutOfMemory)
            {
                status = gcvSTATUS_TOO_MANY_UNIFORMS;
                break;
            }

            if (CodeGen->isRegOutOfResource)
            {
                status = gcvSTATUS_OUT_OF_RESOURCES;
                break;
            }

            /* Emit code. */
            if (!skip && emit)
            {
                if(duplicateEmitForT1SourceAddr != -1)
                {
                    gctUINT addr;

                    /* set dest address to thread t0 */
                    states[3] = ((((gctUINT32) (states[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:13) - (0 ?
 13:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:13) - (0 ?
 13:13) + 1))))))) << (0 ?
 13:13))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 13:13) - (0 ?
 13:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:13) - (0 ? 13:13) + 1))))))) << (0 ? 13:13)));
                    states[3] = ((((gctUINT32) (states[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:24) - (0 ?
 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24)));

                    /* check gl_FragCoord source type, change them to highp register:
                     */
                    /* change matching source addresses to (addr + 1) */
                    if ((((((gctUINT32) (states[1])) >> (0 ? 11:11)) & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1)))))) ) &&
                        (((((gctUINT32) (states[2])) >> (0 ? 5:3)) & ((gctUINT32) ((((1 ? 5:3) - (0 ? 5:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1)))))) ) == 0x0)
                    {
                        addr = (((((gctUINT32) (states[1])) >> (0 ? 20:12)) & ((gctUINT32) ((((1 ? 20:12) - (0 ? 20:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1)))))) );
                        if(addr == (gctUINT)duplicateEmitForT1SourceAddr)
                        {
                            states[2] = ((((gctUINT32) (states[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)));
                        }
                    }
                    if ((((((gctUINT32) (states[2])) >> (0 ? 6:6)) & ((gctUINT32) ((((1 ? 6:6) - (0 ? 6:6) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1)))))) ) &&
                        (((((gctUINT32) (states[3])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) ) == 0x0)
                    {
                        addr = (((((gctUINT32) (states[2])) >> (0 ? 15:7)) & ((gctUINT32) ((((1 ? 15:7) - (0 ? 15:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:7) - (0 ? 15:7) + 1)))))) );
                        if(addr == (gctUINT)duplicateEmitForT1SourceAddr)
                        {
                            states[3] = ((((gctUINT32) (states[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)));
                        }
                    }
                    if ((((((gctUINT32) (states[3])) >> (0 ? 3:3)) & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1)))))) ) &&
                        (((((gctUINT32) (states[3])) >> (0 ? 30:28)) & ((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1)))))) ) == 0x0)
                    {
                        addr = (((((gctUINT32) (states[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) );
                        if(addr == (gctUINT)duplicateEmitForT1SourceAddr)
                        {
                            states[3] = ((((gctUINT32) (states[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));
                        }
                    }

                    gcmERR_BREAK(_Emit(Tree, CodeGen, states));

                    /* set dest address to thread t1 */
                    states[3] = ((((gctUINT32) (states[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:13) - (0 ?
 13:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:13) - (0 ?
 13:13) + 1))))))) << (0 ?
 13:13))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 13:13) - (0 ?
 13:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:13) - (0 ? 13:13) + 1))))))) << (0 ? 13:13)));
                    states[3] = ((((gctUINT32) (states[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:24) - (0 ?
 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24)));

                    /* change matching source addresses to (addr + 1) */
                    if ((((((gctUINT32) (states[1])) >> (0 ? 11:11)) & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1)))))) ) &&
                        (((((gctUINT32) (states[2])) >> (0 ? 5:3)) & ((gctUINT32) ((((1 ? 5:3) - (0 ? 5:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1)))))) ) == 0x4)
                    {
                        addr = (((((gctUINT32) (states[1])) >> (0 ? 20:12)) & ((gctUINT32) ((((1 ? 20:12) - (0 ? 20:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1)))))) );
                        if(addr == (gctUINT)duplicateEmitForT1SourceAddr)
                        {
                            states[1] = ((((gctUINT32) (states[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:12) - (0 ?
 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (addr + 1) & ((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ? 20:12)));
                       }
                    }
                    if ((((((gctUINT32) (states[2])) >> (0 ? 6:6)) & ((gctUINT32) ((((1 ? 6:6) - (0 ? 6:6) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1)))))) ) &&
                        (((((gctUINT32) (states[3])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) ) == 0x4)
                    {
                        addr = (((((gctUINT32) (states[2])) >> (0 ? 15:7)) & ((gctUINT32) ((((1 ? 15:7) - (0 ? 15:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:7) - (0 ? 15:7) + 1)))))) );
                        if(addr == (gctUINT)duplicateEmitForT1SourceAddr)
                        {
                            states[2] = ((((gctUINT32) (states[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:7) - (0 ?
 15:7) + 1))))))) << (0 ?
 15:7))) | (((gctUINT32) ((gctUINT32) (addr + 1) & ((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:7) - (0 ? 15:7) + 1))))))) << (0 ? 15:7)));
                        }
                    }
                    if ((((((gctUINT32) (states[3])) >> (0 ? 3:3)) & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1)))))) ) &&
                        (((((gctUINT32) (states[3])) >> (0 ? 30:28)) & ((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1)))))) ) == 0x4)
                    {
                        addr = (((((gctUINT32) (states[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) );
                        if(addr == (gctUINT)duplicateEmitForT1SourceAddr)
                        {
                            states[3] = ((((gctUINT32) (states[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (addr + 1) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)));
                        }
                    }
                    gcmERR_BREAK(_Emit(Tree, CodeGen, states));
                }
                else
                {
                    gcmERR_BREAK(_Emit(Tree, CodeGen, states));
                }

                /* Clean up the temporary register usage. */
                _UpdateRegisterUsage(CodeGen->registerUsage,
                             CodeGen->registerCount,
                             gcvSL_TEMPORARY);
            }

            /* Next pattern. */
            ++pattern;
        }

        if (CodeGen->lastLoadUser > 0 && CodeGen->nextSource > (gctUINT) CodeGen->lastLoadUser)
        {
            /* Restore back assigned register for loadDestIndex. */
            Tree->tempArray[CodeGen->loadDestIndex].assigned = CodeGen->origAssigned;
            CodeGen->origAssigned = -1;
            CodeGen->lastLoadUser = -1;
        }

        /* Break on error. */
        gcmERR_BREAK(status);
        instruction = &shader->code[codeBase + (curInstIdx - 1)];
        opcode = gcmSL_OPCODE_GET(instruction->opcode, Opcode);
        if (opcode == gcSL_JMP || opcode == gcSL_RET) {
            CodeGen->current->addrRegColoring.countOfMap = 0;
        }
    }

    if (gcmIS_ERROR(status))
    {
        return status;
    }

    /* End of main function. */
    if (Function == gcvNULL)
    {
        /* Save end of main. */
        CodeGen->endMain = CodeGen->current->ip;

        if ((CodeGen->shaderType == gcSHADER_TYPE_VERTEX)
        &&  (CodeGen->flags & gcvSHADER_USE_GL_Z)
        )
        {
            /*
                The GC family of GPU cores require the Z to be from 0 <= z <= w.
                However, OpenGL specifies the Z to be from -w <= z <= w.  So we
                have to a conversion here:

                    z = (z + w) / 2.

                So here we append two instructions to the vertex shader.
            */
            do
            {
                gctSIZE_T o;
                gctINT index = -1;
                gctUINT8 swizzle;
                gctUINT32 states[4];
                gcSL_TYPE type;

                /* Walk all outputs to find the position. */
                for (o = 0; o < Tree->outputCount; ++o)
                {
                    if ((Tree->shader->outputs[o] != gcvNULL)
                    &&  (Tree->shader->outputs[o]->nameLength == gcSL_POSITION)
                    )
                    {
                        /* Save temporary register holding the position. */
                        index = Tree->outputArray[o].tempHolding;
                    }
                }

                /* As SSBO/XFB can skip post-VS stages in pipeline (RASTERIZER_DISCARD is on),
                   VS can have no position legally */
                if (index == -1)
                {
                    gcmASSERT(Tree->shader->transformFeedback.varyingCount > 0 ||
                              Tree->shader->storageBlockCount > 0);

                    break;
                }

                if(index >= 0)
                {
                    precision = Tree->tempArray[index].precision;
                }

                /* ADD pos.z, pos.z, pos.w */
                gcoOS_ZeroMemory(states, gcmSIZEOF(states));

                _SetOpcode(states,
                           0x01,
                           0x00,
                           gcvFALSE);

                gcmERR_BREAK(_gcmSetDest(Tree,
                                         CodeGen,
                                         states,
                                         index,
                                         0x0,
                                         gcSL_ENABLE_Z,
                                         precision,
                                         gcvNULL));

                gcmERR_BREAK(_gcmSetSource(Tree,
                                           CodeGen,
                                           states,
                                           0,
                                           gcSL_TEMP,
                                           index,
                                           0,
                                           0x0,
                                           gcSL_SWIZZLE_ZZZZ,
                                           gcvFALSE,
                                           gcvFALSE,
                                           precision));

                gcmERR_BREAK(_gcmSetSource(Tree,
                                           CodeGen,
                                           states,
                                           2,
                                           gcSL_TEMP,
                                           index,
                                           0,
                                           0x0,
                                           gcSL_SWIZZLE_WWWW,
                                           gcvFALSE,
                                           gcvFALSE,
                                           precision));

                gcmERR_BREAK(_Emit(Tree, CodeGen, states));

                /* MUL pos.z, pos.z, 0.5 */
                gcoOS_ZeroMemory(states, gcmSIZEOF(states));

                _SetOpcode(states,
                           0x03,
                           0x00,
                           gcvFALSE);

                gcmERR_BREAK(_gcmSetDest(Tree,
                                         CodeGen,
                                         states,
                                         index,
                                         0x0,
                                         gcSL_ENABLE_Z,
                                         precision,
                                         gcvNULL));

                gcmERR_BREAK(_gcmSetSource(Tree,
                                           CodeGen,
                                           states,
                                           0,
                                           gcSL_TEMP,
                                           index,
                                           0,
                                           0x0,
                                           gcSL_SWIZZLE_ZZZZ,
                                           gcvFALSE,
                                           gcvFALSE,
                                           precision));

                gcmERR_BREAK(_AddConstantVec1(Tree,
                                              CodeGen,
                                              0.5f,
                                              &index,
                                              &swizzle,
                                              &type));

                gcmERR_BREAK(_gcmSetSource(Tree,
                                           CodeGen,
                                           states,
                                           1,
                                           gcSL_CONSTANT,
                                           index,
                                           0,
                                           0x0,
                                           swizzle,
                                           gcvFALSE,
                                           gcvFALSE,
                                           precision));

                _UsingConstUniform(Tree, CodeGen, 1, index, swizzle, type, states);

                gcmERR_BREAK(_Emit(Tree, CodeGen, states));
            }
            while (gcvFALSE);

            if (gcmIS_ERROR(status))
            {
                return status;
            }
        }

        /* Save end of program. */
        CodeGen->endPC = gcmMAX(CodeGen->current->ip, 1);

#if gcdALPHA_KILL_IN_SHADER
        if ((CodeGen->shaderType == gcSHADER_TYPE_FRAGMENT)
            &&
            (CodeGen->flags & gcvSHADER_USE_ALPHA_KILL)
            )
        {
            /* Save current kill flag. */
            gctBOOL kill = CodeGen->kill;

            /* Adding an alpha kill adds a TEXKILL instruction if the alpha of
             * the output color is < 1 / 256. */
            do
            {
                gctSIZE_T   o;
                gctINT      index = -1;
                gctINT      const256Index;
                gctUINT8    const256Swizzle;
                gctINT      const1Index;
                gctUINT8    const1Swizzle;
                gctUINT32   states[4];
                gcSL_TYPE   type, type1;

                /* Walk all outputs to find the position. */
                for (o = 0; o < Tree->outputCount; ++o)
                {
                    if ((Tree->shader->outputs[o] != gcvNULL)
                        &&
                        (Tree->shader->outputs[o]->nameLength == gcSL_COLOR)
                        )
                    {
                        if (index == -1)
                        {
                            /* Save temporary register holding the position. */
                            index = Tree->outputArray[o].tempHolding;
                        }
                        else
                        {
                            /* There are more than one output, skip alpha kill opt. */
                            index = -1;
                            break;
                        }
                    }
                }

                /* Do nothing if we didn't add a TEXKILL instruction. */
                if (index == -1)
                {
                    break;
                }

                /* Allocate a uniform with the value 1 / 256. */
                gcmERR_BREAK(_AddConstantVec1(Tree,
                                              CodeGen,
                                              1.0f / 256.0f,
                                              &const256Index,
                                              &const256Swizzle,
                                              &type));

                /* Allocate a uniform with the value 1. */
                gcmERR_BREAK(_AddConstantVec1(Tree,
                                              CodeGen,
                                              1.0f,
                                              &const1Index,
                                              &const1Swizzle,
                                              &type1));

                if (CodeGen->current->ip == 0)
                {
                    /* Append a NOP. */
                    states[0] = (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) (0x00 & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)))
                                 |
                                 ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ?
 10:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:6) - (0 ?
 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x00 & ((gctUINT32) ((((1 ?
 10:6) - (0 ?
 10:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)))
                                 |
                                 ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                                 );
                    states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));
                    states[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));
                    states[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)));

                    gcmERR_BREAK(_Emit(Tree, CodeGen, states));
                }

                /* Create TEXKILL.lt color.w, 1/256 */
                gcoOS_ZeroMemory(CodeGen->alphaKillInstruction,
                                 gcmSIZEOF(CodeGen->alphaKillInstruction));

                if(index >= 0)
                {
                    precision = Tree->tempArray[index].precision;
                }
                else
                {
                    precision = gcSL_PRECISION_MEDIUM;
                }

                _SetOpcode(CodeGen->alphaKillInstruction,
                           0x17,
                           0x02,
                           gcvFALSE);

                gcmERR_BREAK(_gcmSetSource(Tree,
                                           CodeGen,
                                           CodeGen->alphaKillInstruction,
                                           0,
                                           gcSL_TEMP,
                                           index,
                                           0,
                                           0x0,
                                           gcSL_SWIZZLE_WWWW,
                                           gcvFALSE,
                                           gcvFALSE,
                                           precision));

                gcmERR_BREAK(_gcmSetSource(Tree,
                                           CodeGen,
                                           CodeGen->alphaKillInstruction,
                                           1,
                                           gcSL_CONSTANT,
                                           const256Index,
                                           0,
                                           0x0,
                                           const256Swizzle,
                                           gcvFALSE,
                                           gcvFALSE,
                                           precision));

                _UsingConstUniform(Tree, CodeGen, 1, const256Index, const256Swizzle, type, CodeGen->alphaKillInstruction);

                /* Create DP4 color.w, color.xyzw, 1 */
                gcoOS_ZeroMemory(CodeGen->colorKillInstruction,
                                 gcmSIZEOF(CodeGen->colorKillInstruction));

                _SetOpcode(CodeGen->colorKillInstruction,
                           0x06,
                           0x00,
                           gcvFALSE);

                gcmERR_BREAK(_gcmSetDest(Tree,
                                         CodeGen,
                                         CodeGen->colorKillInstruction,
                                         index,
                                         0x0,
                                         gcSL_ENABLE_W,
                                         precision,
                                         gcvNULL));

                gcmERR_BREAK(_gcmSetSource(Tree,
                                           CodeGen,
                                           CodeGen->colorKillInstruction,
                                           0,
                                           gcSL_TEMP,
                                           index,
                                           0,
                                           0x0,
                                           gcSL_SWIZZLE_XYZW,
                                           gcvFALSE,
                                           gcvFALSE,
                                           precision));

                gcmERR_BREAK(_gcmSetSource(Tree,
                                           CodeGen,
                                           CodeGen->colorKillInstruction,
                                           1,
                                           gcSL_CONSTANT,
                                           const1Index,
                                           0,
                                           0x0,
                                           const1Swizzle,
                                           gcvFALSE,
                                           gcvFALSE,
                                           precision));

                 _UsingConstUniform(Tree, CodeGen, 1, const1Index, const1Swizzle, type1, CodeGen->colorKillInstruction);

                gcmERR_BREAK(_Emit(Tree,
                                   CodeGen,
                                   CodeGen->colorKillInstruction));
                CodeGen->endPCAlphaKill = CodeGen->current->ip;
                gcmERR_BREAK(_Emit(Tree,
                                   CodeGen,
                                   CodeGen->alphaKillInstruction));
                CodeGen->endPCColorKill = CodeGen->current->ip;
            }
            while (gcvFALSE);

            /* Restore kill flag. */
            CodeGen->kill = kill;

            if (gcmIS_ERROR(status))
            {
                return status;
            }
        }
#endif

        if ((Tree->shader->functionCount +  Tree->shader->kernelFunctionCount > 0)
        ||  (CodeGen->current->ip == 0)
        )
        {
            do
            {
                gctUINT32 states[4];

                /* Append a NOP. */
                states[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) (0x00 & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ?
 10:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:6) - (0 ?
 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) (0x00 & ((gctUINT32) ((((1 ?
 10:6) - (0 ?
 10:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)));
                states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));
                states[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));
                states[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)));

                gcmERR_BREAK(_Emit(Tree, CodeGen, states));
            }
            while (gcvFALSE);

            if (gcmIS_ERROR(status))
            {
                return status;
            }
        }
    }

    CodeGen->instCount += CodeGen->current->ip;
    CodeGen->current->branch = Tree->branch;
    Tree->branch             = gcvNULL;

    return gcvSTATUS_OK;
}

gceSTATUS
_GenerateCode(
    IN gcLINKTREE Tree,
    IN OUT gcsCODE_GENERATOR_PTR CodeGen
    )
{
    gceSTATUS status;

    /* Reset register count. */
    CodeGen->maxRegister = 0;

    do
    {
        gctSIZE_T i;
        gctUINT base = 0, max = 0;
        gctUINT vsInstMax = gcHWCaps.maxVSInstCount;
        gctUINT psInstMax = gcHWCaps.maxPSInstCount;
        gcSHADER shader = Tree->shader;

        /* Determine the maximum number of instructions. */

        /* check if FB_UNLIMITED_INSTRUCTION is set */
        if (gcmOPT_hasFeature(FB_UNLIMITED_INSTRUCTION))
        {
            vsInstMax = psInstMax = (gctUINT)-1;
        }
        /* Generate the main function. */
        gcmERR_BREAK(_GenerateFunction(Tree, CodeGen, gcvNULL, gcvFALSE));

        /* Walk all functions. */
        for (i = 0, base = 0; i <= shader->functionCount + shader->kernelFunctionCount; ++i)
        {
            CodeGen->functions[i].ipBase = base;
            base += CodeGen->functions[i].ip;
        }

        switch (CodeGen->shaderType)
        {
        case gcSHADER_TYPE_VERTEX:
            max = vsInstMax;
            break;

        case gcSHADER_TYPE_FRAGMENT:
            max = psInstMax;
            break;

        default:
            max = base;
            break;
        }

        if (base > max && !CodeGen->hasICache)
        {
            gcoOS_Print("Shader has too many instructions: %d (maximum is %d)", base, max);
            status = gcvSTATUS_TOO_MANY_INSTRUCTION;
            break;
        }

        /* Walk all functions. */
        for (i = 0; i <= shader->functionCount + shader->kernelFunctionCount; ++i)
        {
            gcsSL_FUNCTION_CODE_PTR function = &CodeGen->functions[i];

            /* Walk all branches. */
            while (function->branch != gcvNULL)
            {
                gctUINT target;

                gcSL_BRANCH_LIST branch = function->branch;
                function->branch        = branch->next;

                if (branch->target >= shader->codeCount)
                {
                    target = CodeGen->endMain;
                }
                else
                if (!branch->call
                &&  (function != CodeGen->codeMap[branch->target].function)
                )
                {
                    gcmASSERT(function->ipBase == 0);
                    target = CodeGen->endMain;
                }
                else
                {
                    target = CodeGen->codeMap[branch->target].function->ipBase
                           + CodeGen->codeMap[branch->target].location;
                }

                /* Update the TARGET field. */
                gcmERR_BREAK(_SetTarget(Tree,
                                        CodeGen,
                                        function,
                                        branch,
                                        target));

                /* Free the branch structure. */
                gcmERR_BREAK(gcmOS_SAFE_FREE(gcvNULL, branch));
            }

            if (gcmIS_ERROR(status))
            {
                break;
            }

            /* patch subsampleDepth register if it is used in shader */
            if (CodeGen->subsampleDepthRegIncluded)
            {
                _PatchSubsampleDepthRegister(Tree, CodeGen);
            }
        }
    }
    while (gcvFALSE);

    /* Return the status. */
    return status;
}

#if !DX_SHADER
static gceSTATUS
_SetState(
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctUINT32 Address,
    IN gctUINT32 Data
    )
{
    gctUINT32 maxLoadStateCount = gcmALIGN((((((gctUINT32) (~0)) >> (0 ? 25:16)) & ((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1)))))) ), 4)-4;

    /* Check if this address is sequential to the last loaded state. */
    if ((Address == CodeGen->lastStateAddress + CodeGen->lastStateCount)
    &&  (CodeGen->lastStateCount < maxLoadStateCount)
    )
    {
        /* Make sure we have enough room in the state buffer. */
        if (CodeGen->stateBufferOffset + 4 > CodeGen->stateBufferSize)
        {
            return gcvSTATUS_BUFFER_TOO_SMALL;
        }

        /* Increment state count of last LoadState command. */
        ++ CodeGen->lastStateCount;

        if (CodeGen->lastStateCommand != gcvNULL) {

            /* Physical LoadState command. */
            *CodeGen->lastStateCommand = ((((gctUINT32) (*CodeGen->lastStateCommand)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (CodeGen->lastStateCount) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
        }

        CodeGen->stateDeltaBufferOffset++;
        if (CodeGen->lastStateDeltaBatchEnd && CodeGen->lastStateDeltaBatchHead)
        {
            *(CodeGen->lastStateDeltaBatchEnd++) = Data;
            *(CodeGen->lastStateDeltaBatchEnd) = VSC_STATE_DELTA_END;
            *(CodeGen->lastStateDeltaBatchHead + 1) =  *(CodeGen->lastStateDeltaBatchHead + 1) + 1;
        }
    }
    else
    {
        /* Align last load state to 64-bit. */
        CodeGen->stateBufferOffset = gcmALIGN(CodeGen->stateBufferOffset, 8);

        /* Make sure we have enough room in the state buffer. */
        if (CodeGen->stateBufferOffset + 8 > CodeGen->stateBufferSize)
        {
            return gcvSTATUS_BUFFER_TOO_SMALL;
        }

        /* New LoadState command. */
        CodeGen->lastStateAddress = Address;
        CodeGen->lastStateCount   = 1;

        if (CodeGen->stateBuffer != gcvNULL)
        {
            /* Save address of LoadState command. */
            CodeGen->lastStateCommand = (gctUINT32 *) CodeGen->stateBuffer +
                                        CodeGen->stateBufferOffset / 4;

            /* Physical LoadState command. */
            *CodeGen->lastStateCommand =
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (Address) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
        }

        if (CodeGen->stateDeltaBuffer)
        {
            CodeGen->lastStateDeltaBatchHead = CodeGen->stateDeltaBuffer + CodeGen->stateDeltaBufferOffset;
            CodeGen->lastStateDeltaBatchEnd = CodeGen->lastStateDeltaBatchHead + 3;
            *CodeGen->lastStateDeltaBatchHead = Address;
            *(CodeGen->lastStateDeltaBatchHead + 1) = 1;
            *(CodeGen->lastStateDeltaBatchHead + 2) = Data;
            *CodeGen->lastStateDeltaBatchEnd = VSC_STATE_DELTA_END;
        }

        CodeGen->stateDeltaBufferOffset += 1 + VSC_STATE_DELTA_DESC_SIZE_IN_UINT32;
        /* Increment state buffer offset. */
        CodeGen->stateBufferOffset += 4;
    }

    if (CodeGen->stateBuffer != gcvNULL)
    {
        /* Physical data. */
        ((gctUINT32 *) CodeGen->stateBuffer)[CodeGen->stateBufferOffset / 4] =
            Data;
    }

    /* Increment state buffer offset. */
    CodeGen->stateBufferOffset += 4;

    /* Success. */
    return gcvSTATUS_OK;
}

/* get the total temp register number for a GPU core */
static gctINT
_totalTempRegisters(
    IN gcsCODE_GENERATOR_PTR CodeGen
    )
{
    /* each Shader Core has 4 SIMD units, each unit has 128 registers */
    return CodeGen->shaderCoreCount * 128 * 4;
}

static void
_adjustMaxTemp(
    IN gcLINKTREE                Tree,
    IN OUT gcsCODE_GENERATOR_PTR CodeGen
    )
{
    gcShaderCodeInfo  codeInfo;
    gctUINT           maxRegToUse;
    gctUINT           texldCount = 0; /* this is very rough count, didn't
                                        consider loops, calls, conditions,
                                        etc... */

    /* init codeInfo */
    gcoOS_ZeroMemory(&codeInfo, gcmSIZEOF(gcShaderCodeInfo));
    gcSHADER_CountCode(Tree->shader, &codeInfo);

    texldCount += codeInfo.codeCounter[gcSL_TEXLD];
    texldCount += codeInfo.codeCounter[gcSL_TEXLDPROJ];

    if (texldCount == 0)
        return;

    /* If the compiled shader program has 1 texture load instruction
       (lod bias/user load) and allocates 2 temporary registers, then
       the following occurs. GC1000 has two shader cores. Which means
       a total of 1024 temporary registers are present. In the example,
       as 2 temporary registers are used by each pixel thread, there can
       be a maximum of 512 pixels executing in parallel. We need to
       reduce this to 256 in order to prevent the sideband from becoming
       full. Increasing the temporary registers used to 4, will reduce
       the maximum pixel threads to 256. Following equation will give
       the temporary register count for each pixel

            TemporaryRegisterCountPerPixel >=
                            (#TotalTemporaryRegistersInGC1000 *
                             #TexldInstructionsInShaderProgram)/256

       The equation above will work for shader program which dont have
       texture instructions inside a conditional branch loop (Which should
       be 99% of the applications that run on GC1000).
     */
    maxRegToUse = (_totalTempRegisters(CodeGen) * texldCount)/256;
    /* the register starts from 0 */
    gcCGUpdateMaxRegister(CodeGen, maxRegToUse - 1, Tree);

    return;
}

/* after varying packing, the attribute in fragment shader maybe become
   unused, the link between VS and FS will break if we don't exclude these
   attribute from output mapping link
 */
static gctINT
_getLinkedFragmentAttributeLinkIndex(
    IN gcLINKTREE Tree,
    IN gctINT     Index
    )
{
    gctINT link;

    gcmASSERT(Tree && (gctINT)Tree->outputCount > Index);

    /* attribute[0] starts from r1 (after r0) */
    link = Tree->outputArray[Index].fragmentIndex + 1;
    /* skip unused attribute rows */
    link -= Tree->outputArray[Index].skippedFragmentAttributeRows;
    return link;
}
#if TEMP_SHADER_PATCH

#define INVALID_SHADER_SIZE    0xFFFFFFFF

static gctUINT32 _ReplaceShader(
    IN gcLINKTREE Tree,
    IN OUT gctUINT32** pPatchedShaderCode,
    IN gctUINT32  shaderSizeInDW,
    IN gctBOOL isDual16Shader,
    OUT gctBOOL * IsMatch,
    OUT gcsHINT_PTR Hints
    )
{
    gctUINT32 newShaderSizeInDW = shaderSizeInDW;

#if gcdSHADER_SRC_BY_MACHINECODE
    static gctUINT32 antutu30_replaceShaderCode0[] =
    {
        0x07811018,0x15001F20,0x00000000,0x00000000, /*0x0000        texld  r1, r1.xyyy, s0 */
        0x00000097,0x3FC01800,0x00000040,0x00000002, /*0x0001        texkill  lt  r1.w, u0.x */
        0x03811003,0x29001800,0x01480140,0x00000000, /*0x0002        mul    r1.xyz, r1.xyzz, r2.xyzz */
    };

    static gctUINT32 glb27_release_replaceShaderCode0[] =      /* Use u2, u3 instead of u0, u1 since patch framework bug */
    {
        0x06011018,0x15001A00,0x00000000,0x00000000, /*0x0000    texld              r1.zw, r1.xy, s0.xxxy */
        0x0F821018,0x15001F20,0x00000000,0x00000000, /*0x0001    texld              r2, r1.xy, s1 */
        0x06011002,0x3A801800,0x00800140,0x207A802A, /*0x0002    mad                r1.zw, r1.zzzw, u2.xxxy, -u2.zzzw */
        0x06011003,0x3A801800,0x01FE0140,0x00000000, /*0x0003    mul                r1.zw, r1.zzzw, r2.w */
        0x01811001,0x15001800,0x00000000,0x007F8018, /*0x0004    add                r1.xy, r1.xy, -r1.zw */
        0x0B831018,0x15001F20,0x00000000,0x00000000, /*0x0005    texld              r3.xyz, r1.xy, s1 */
        0x01811001,0x15001800,0x00000000,0x007F8018, /*0x0006    add                r1.xy, r1.xy, -r1.zw */
        0x03821001,0x29002800,0x00000000,0x00290038, /*0x0007    add                r2.xyz, r2.xyz, r3.xyz */
        0x0B831018,0x15001F20,0x00000000,0x00000000, /*0x0008    texld              r3.xyz, r1.xy, s1 */
        0x01811001,0x15001800,0x00000000,0x007F8018, /*0x0009    add                r1.xy, r1.xy, -r1.zw */
        0x03821001,0x29002800,0x00000000,0x00290038, /*0x000a    add                r2.xyz, r2.xyz, r3.xyz */
        0x0B811018,0x15001F20,0x00000000,0x00000000, /*0x000b    texld              r1.xyz, r1.xy, s1 */
        0x03811001,0x29002800,0x00000000,0x00290018, /*0x000c    add                r1.xyz, r2.xyz, r1.xyz */
        0x03811003,0x29001800,0x000001C0,0x00000002, /*0x000d    mul                r1.xyz, r1.xyz, u3.x */
    };

    static gctUINT32 glb25_release_replaceShaderCode0[]=
    {
        0x1f851018, 0x15001f20, 0x00000000, 0x00000000,/*000: texld              r5, r1.xy, s3*/
        0x07861018, 0x15001f20, 0x00000000, 0x00000000,/*001: texld              r6, r1.xy, s0*/
        0x02011005, 0x29002800, 0x01480140, 0x00000000,/*002: dp3                r1.z, r2.xyz, r2.xyz*/
        0x0201100d, 0x00000000, 0x00000000, 0x002a8018,/*003: rsq                r1.z, r1.z*/
        0x03821003, 0x29002800, 0x015400c0, 0x00000000,/*004: mul                r2.xyz, r2.xyz, r1.z*/
        0x02011005, 0x29003800, 0x014801c0, 0x00000000,/*005: dp3                r1.z, r3.xyz, r3.xyz*/
        0x0201100d, 0x00000000, 0x00000000, 0x002a8018,/*006: rsq                r1.z, r1.z*/
        0x03831003, 0x29003800, 0x015400c0, 0x00000000,/*007: mul                r3.xyz, r3.xyz, r1.z*/
        0x03871002, 0x29006800, 0x00000240, 0x2055404a,/*008: mad                r7.xyz, r6.xyz, c4.x, -c4.y*/
        0x02011005, 0x29007800, 0x014803c0, 0x00000000,/*009: dp3                r1.z, r7.xyz, r7.xyz*/
        0x0201100d, 0x00000000, 0x00000000, 0x002a8018,/*010: rsq                r1.z, r1.z*/
        0x03861003, 0x29007800, 0x015400c0, 0x00000000,/*011: mul                r6.xyz, r7.xyz, r1.z*/
        0x03871003, 0x14803800, 0x00120140, 0x00000000,/*012: mul                r7.xyz, r3.zxy, r2.yzx*/
        0x03871002, 0x14802800, 0x001201c0, 0x00690078,/*013: mad                r7.xyz, r2.zxy, r3.yzx, -r7.xyz*/
        0x03821003, 0x29002800, 0x00000340, 0x00000000,/*014: mul                r2.xyz, r2.xyz, r6.x*/
        0x03821002, 0x29007800, 0x00aa0340, 0x00290028,/*015: mad                r2.xyz, r7.xyz, r6.y, r2.xyz*/
        0x03821002, 0x29003800, 0x01540340, 0x00290028,/*016: mad                r2.xyz, r3.xyz, r6.z, r2.xyz*/
        0x02011005, 0x29004800, 0x01480240, 0x00000000,/*017: dp3                r1.z, r4.xyz, r4.xyz*/
        0x0201100d, 0x00000000, 0x00000000, 0x002a8018,/*018: rsq                r1.z, r1.z*/
        0x03831003, 0x29004800, 0x015400c0, 0x00000000,/*019: mul                r3.xyz, r4.xyz, r1.z*/
        0x27811018, 0x15001f20, 0x00000000, 0x00000000,/*020: texld              r1, r1.xy, s4*/
        0x03861001, 0x29003800, 0x00000000, 0x20290018,/*021: add                r6.xyz, r3.xyz, c1.xyz*/
        0x04021005, 0x29006800, 0x01480340, 0x00000000,/*022: dp3                r2.w, r6.xyz, r6.xyz*/
        0x0402100d, 0x00000000, 0x00000000, 0x003fc028,/*023: rsq                r2.w, r2.w*/
        0x03861003, 0x29006800, 0x01fe0140, 0x00000000,/*024: mul                r6.xyz, r6.xyz, r2.w*/
        0x04021805, 0x29002800, 0x014800c0, 0x00000002,/*025: dp3.sat            r2.w, r2.xyz, c1.xyz*/
        0x04031805, 0x29002800, 0x01480340, 0x00000000,/*026: dp3.sat            r3.w, r2.xyz, r6.xyz*/
        0x00861012, 0x00000000, 0x00000000, 0x00bfc038,/*027: log                r6.x, |r3.w|*/
        0x00861003, 0x00006800, 0x01540040, 0x00000002,/*028: mul                r6.x, r6.x, c0.z*/
        0x00861011, 0x00000000, 0x00000000, 0x00000068,/*029: exp                r6.x, r6.x*/
        0x00861003, 0x15400800, 0x00000350, 0x00000000,/*030: mul                r6.x, c0.y, r6.x*/
        0x04031003, 0x00006800, 0x00aa02c0, 0x00000000,/*031: mul                r3.w, r6.x, r5.y*/
        0x07861009, 0x00000000, 0x00000000, 0x20390008,/*032: mov                r6, c0*/
        0x04021002, 0x3fc02800, 0x00000340, 0x203fc028,/*033: mad                r2.w, r2.w, r6.x, c2.w*/
        0x03811002, 0x29001800, 0x01fe0140, 0x003fc038,/*034: mad                r1.xyz, r1.xyz, r2.w, r3.w*/
        0x03811003, 0x29001800, 0x01480140, 0x00000002,/*035: mul                r1.xyz, r1.xyz, c2.xyz*/
        0x02051003, 0x2a805800, 0x01fe00c0, 0x00000002,/*036: mul                r5.z, r5.z, c1.w*/
        0x00000116, 0x2a805800, 0x01fe01c0, 0x00001802,/*037: branch.le          r5.z, c3.w, 48*/
        0x03861009, 0x00000000, 0x00000000, 0x00290018,/*038: mov                r6.xyz, r1.xyz*/
        0x04011005, 0x29002800, 0x014801c0, 0x00000000,/*039: dp3                r1.w, r2.xyz, r3.xyz*/
        0x04021001, 0x3fc01800, 0x00000000, 0x003fc018,/*040: add                r2.w, r1.w, r1.w*/
        0x03821002, 0x3fc02800, 0x01480140, 0x00690038,/*041: mad                r2.xyz, r2.w, r2.xyz, -r3.xyz*/
        0x0f831018, 0x29002f20, 0x00000000, 0x00000000,/*042: texld              r3, r2.xyz, s1*/
        0x17821018, 0x29002f20, 0x00000000, 0x00000000,/*043: texld              r2, r2.xyz, s2*/
        0x03831001, 0x29003800, 0x00000000, 0x00690028,/*044: add                r3.xyz, r3.xyz, -r2.xyz*/
        0x03821002, 0x3fc00800, 0x014801d0, 0x00290028,/*045: mad                r2.xyz, c0.w, r3.xyz, r2.xyz*/
        0x03821001, 0x29002800, 0x00000000, 0x00690068,/*046: add                r2.xyz, r2.xyz, -r6.xyz*/
        0x03811002, 0x2a805800, 0x01480140, 0x00290068,/*047: mad                r1.xyz, r5.z, r2.xyz, r6.xyz*/
        0x04011003, 0x3fc04800, 0x01fe0240, 0x00000000,/*048: mul                r1.w, r4.w, r4.w*/
        0x03821001, 0x29003800, 0x00000010, 0x00690018,/*049: add                r2.xyz, c3.xyz, -r1.xyz*/
        0x03811002, 0x3fc01800, 0x01480140, 0x00290018,/*050: mad                r1.xyz, r1.w, r2.xyz, r1.xyz*/
    };

    static gctUINT32 glb25_release_fp16_replaceShaderCode0[]=
    {
        0x1F851018, 0x15001F20, 0x00000000, 0x00000000, /*  0: texld   r5, r1.xyyy, s3 */
        0x07861018, 0x15001F20, 0x00000000, 0x00000000, /*  1: texld   r6, r1.xyyy, s0 */
        0x02011035, 0x29002800, 0x01490140, 0x00000000, /*  2: norm_dp3     r1.z, r2.xyzz, r2.xyzz, */
        0x0201100D, 0x00000000, 0x00000000, 0x002A8018, /*  3: rsq     r1.z, r1.z, */
        0x03821037, 0x29002800, 0x015500C0, 0x00000000, /*  4: norm_mul     r2.xyz, r2.xyzz, r1.z, */
        0x02011035, 0x29003800, 0x014901C0, 0x00000000, /*  5: norm_dp3     r1.z, r3.xyzz, r3.xyzz, */
        0x0201100D, 0x00000000, 0x00000000, 0x002A8018, /*  6: rsq     r1.z, r1.z, */
        0x03831037, 0x29003800, 0x015500C0, 0x00000000, /*  7: norm_mul     r3.xyz, r3.xyzz, r1.z, */
        0x03871002, 0x29006800, 0x00000240, 0x2055404A, /*  8: mad     r7.xyz, r6.xyzz, u4.x, -u4.y, */
        0x02011035, 0x29007800, 0x014903C0, 0x00000000, /*  9: norm_dp3     r1.z, r7.xyzz, r7.xyzz, */
        0x0201100D, 0x00000000, 0x00000000, 0x002A8018, /* 10: rsq     r1.z, r1.z, */
        0x03861037, 0x29007800, 0x015500C0, 0x00000000, /* 11: norm_mul     r6.xyz, r7.xyzz, r1.z, */
        0x03871003, 0x14803800, 0x00120140, 0x00000000, /* 12: mul     r7.xyz, r3.zxyy, r2.yzxx, */
        0x03871002, 0x14802800, 0x001201C0, 0x00690078, /* 13: mad     r7.xyz, r2.zxyy, r3.yzxx, -r7.xyzz, */
        0x03821003, 0x29002800, 0x00000340, 0x00000000, /* 14: mul     r2.xyz, r2.xyzz, r6.x, */
        0x03821002, 0x29007800, 0x00AA0340, 0x00290028, /* 15: mad     r2.xyz, r7.xyzz, r6.y, r2.xyzz, */
        0x03821002, 0x29003800, 0x01540340, 0x00290028, /* 16: mad     r2.xyz, r3.xyzz, r6.z, r2.xyzz, */
        0x02011035, 0x29004800, 0x01490240, 0x00000000, /* 17: norm_dp3     r1.z, r4.xyzz, r4.xyzz, */
        0x0201100D, 0x00000000, 0x00000000, 0x002A8018, /* 18: rsq     r1.z, r1.z, */
        0x03831037, 0x29004800, 0x015500C0, 0x00000000, /* 19: norm_mul     r3.xyz, r4.xyzz, r1.z, */
        0x27811018, 0x15001F20, 0x00000000, 0x00000000, /* 20: texld   r1, r1.xyyy, s4 */
        0x03861001, 0x29003800, 0x00000000, 0x20290018, /* 21: add     r6.xyz, r3.xyzz, u1.xyzz, */
        0x04021035, 0x29006800, 0x01490340, 0x00000000, /* 22: norm_dp3     r2.w, r6.xyzz, r6.xyzz, */
        0x0402100D, 0x00000000, 0x00000000, 0x003FC028, /* 23: rsq     r2.w, r2.w, */
        0x03861037, 0x29006800, 0x01FF0140, 0x00000000, /* 24: norm_mul     r6.xyz, r6.xyzz, r2.w, */
        0x04021805, 0x29002800, 0x014800C0, 0x00000002, /* 25: dp3_sat r2.w, r2.xyzz, u1.xyzz, */
        0x04031805, 0x29002800, 0x01480340, 0x00000000, /* 26: dp3_sat r3.w, r2.xyzz, r6.xyzz, */
        0x01861012, 0x00000001, 0x00000000, 0x00BFC038, /* 27: log.rtz     r6.xy, |r3.w|*/
        0x00861003, 0x00006800, 0x00AA0340, 0x00000000, /* 28:  mul     r6.x, r6.x, r6.y*/
        0x00861003, 0x00006800, 0x01540040, 0x00000002, /* 29: mul     r6.x, r6.x, u0.z, */
        0x00861011, 0x00000000, 0x00000000, 0x00000068, /* 30: exp     r6.x, r6.x, */
        0x00861003, 0x15400800, 0x00000350, 0x00000000, /* 31: mul     r6.x, u0.y, r6.x, */
        0x04031003, 0x00006800, 0x00AA02C0, 0x00000000, /* 32: mul     r3.w, r6.x, r5.y, */
        0x07861009, 0x00000000, 0x00000000, 0x20390008, /* 33: mov     r6, u0, */
        0x04021002, 0x3FC02800, 0x00000340, 0x203FC028, /* 34: mad     r2.w, r2.w, r6.x, u2.w, */
        0x03811002, 0x29001800, 0x01FE0140, 0x003FC038, /* 35: mad     r1.xyz, r1.xyzz, r2.w, r3.w, */
        0x03811003, 0x29001800, 0x01480140, 0x00000002, /* 36: mul     r1.xyz, r1.xyzz, u2.xyzz, */
        0x02051003, 0x2A805800, 0x01FE00C0, 0x00000002, /* 37: mul     r5.z, r5.z, u1.w, */
        0x00000116, 0x2A805800, 0x01FE01C0, 0x00001882, /* 38: bra le      49, r5.z, u3.w*/
        0x03861009, 0x00000000, 0x00000000, 0x00290018, /* 39: mov     r6.xyz, r1.xyzz, */
        0x04011005, 0x29002800, 0x014801C0, 0x00000000, /* 40: dp3     r1.w, r2.xyzz, r3.xyzz, */
        0x04021001, 0x3FC01800, 0x00000000, 0x003FC018, /* 41: add     r2.w, r1.w, r1.w, */
        0x03821002, 0x3FC02800, 0x01480140, 0x00690038, /* 42: mad     r2.xyz, r2.w, r2.xyzz, -r3.xyzz, */
        0x0F831018, 0x29002F20, 0x00000000, 0x00000000, /* 43: texld   r3, r2.xyzz, s1 */
        0x17821018, 0x29002F20, 0x00000000, 0x00000000, /* 44: texld   r2, r2.xyzz, s2 */
        0x03831001, 0x29003800, 0x00000000, 0x00690028, /* 45: add     r3.xyz, r3.xyzz, -r2.xyzz, */
        0x03821002, 0x3FC00800, 0x014801D0, 0x00290028, /* 46: mad     r2.xyz, u0.w, r3.xyzz, r2.xyzz, */
        0x03821001, 0x29002800, 0x00000000, 0x00690068, /* 47: add     r2.xyz, r2.xyzz, -r6.xyzz, */
        0x03811002, 0x2A805800, 0x01480140, 0x00290068, /* 48: mad     r1.xyz, r5.z, r2.xyzz, r6.xyzz, */
        0x04011003, 0x3FC04800, 0x01FE0240, 0x00000000, /* 49: mul     r1.w, r4.w, r4.w, */
        0x03821001, 0x29003800, 0x00000010, 0x00690018, /* 50: add     r2.xyz, u3.xyzz, -r1.xyzz, */
        0x03811002, 0x3FC01800, 0x01480140, 0x00290018, /* 51: mad     r1.xyz, r1.w, r2.xyzz, r1.xyzz, */
    };

    static gctUINT32 glb25_release_replaceShaderCode1[]=
    {
        0x1f851018, 0x15001f20, 0x00000000, 0x00000000,/*000: texld              r5, r1.xy, s3*/
        0x00000097, 0x00005800, 0x01fe0140, 0x00000002,/*001: texkill.lt         r5.x, c2.w*/
        0x07861018, 0x15001f20, 0x00000000, 0x00000000,/*002: texld              r6, r1.xy, s0*/
        0x02011005, 0x29002800, 0x01480140, 0x00000000,/*003: dp3                r1.z, r2.xyz, r2.xyz*/
        0x0201100d, 0x00000000, 0x00000000, 0x002a8018,/*004: rsq                r1.z, r1.z*/
        0x03821003, 0x29002800, 0x015400c0, 0x00000000,/*005: mul                r2.xyz, r2.xyz, r1.z*/
        0x02011005, 0x29003800, 0x014801c0, 0x00000000,/*006: dp3                r1.z, r3.xyz, r3.xyz*/
        0x0201100d, 0x00000000, 0x00000000, 0x002a8018,/*007: rsq                r1.z, r1.z*/
        0x03831003, 0x29003800, 0x015400c0, 0x00000000,/*008: mul                r3.xyz, r3.xyz, r1.z*/
        0x03871002, 0x29006800, 0x00000240, 0x2055404a,/*009: mad                r7.xyz, r6.xyz, c4.x, -c4.y*/
        0x02011005, 0x29007800, 0x014803c0, 0x00000000,/*010: dp3                r1.z, r7.xyz, r7.xyz*/
        0x0201100d, 0x00000000, 0x00000000, 0x002a8018,/*011: rsq                r1.z, r1.z*/
        0x03861003, 0x29007800, 0x015400c0, 0x00000000,/*012: mul                r6.xyz, r7.xyz, r1.z*/
        0x03871003, 0x14803800, 0x00120140, 0x00000000,/*013: mul                r7.xyz, r3.zxy, r2.yzx*/
        0x03871002, 0x14802800, 0x001201c0, 0x00690078,/*014: mad                r7.xyz, r2.zxy, r3.yzx, -r7.xyz*/
        0x03821003, 0x29002800, 0x00000340, 0x00000000,/*015: mul                r2.xyz, r2.xyz, r6.x*/
        0x03821002, 0x29007800, 0x00aa0340, 0x00290028,/*016: mad                r2.xyz, r7.xyz, r6.y, r2.xyz*/
        0x03821002, 0x29003800, 0x01540340, 0x00290028,/*017: mad                r2.xyz, r3.xyz, r6.z, r2.xyz*/
        0x02011005, 0x29004800, 0x01480240, 0x00000000,/*018: dp3                r1.z, r4.xyz, r4.xyz*/
        0x0201100d, 0x00000000, 0x00000000, 0x002a8018,/*019: rsq                r1.z, r1.z*/
        0x03831003, 0x29004800, 0x015400c0, 0x00000000,/*020: mul                r3.xyz, r4.xyz, r1.z*/
        0x27811018, 0x15001f20, 0x00000000, 0x00000000,/*021: texld              r1, r1.xy, s4*/
        0x03861001, 0x29003800, 0x00000000, 0x20290018,/*022: add                r6.xyz, r3.xyz, c1.xyz*/
        0x04021005, 0x29006800, 0x01480340, 0x00000000,/*023: dp3                r2.w, r6.xyz, r6.xyz*/
        0x0402100d, 0x00000000, 0x00000000, 0x003fc028,/*024: rsq                r2.w, r2.w*/
        0x03861003, 0x29006800, 0x01fe0140, 0x00000000,/*025: mul                r6.xyz, r6.xyz, r2.w*/
        0x04021005, 0x29002800, 0x014800c0, 0x00000002,/*026: dp3                r2.w, r2.xyz, c1.xyz*/
        0x04021002, 0x3fc02800, 0x01fe0140, 0x203fc02a,/*027: mad                r2.w, r2.w, c2.w, c2.w*/
        0x04031805, 0x29002800, 0x01480340, 0x00000000,/*028: dp3.sat            r3.w, r2.xyz, r6.xyz*/
        0x00861012, 0x00000000, 0x00000000, 0x00bfc038,/*029: log                r6.x, |r3.w|*/
        0x00861003, 0x00006800, 0x01540040, 0x00000002,/*030: mul                r6.x, r6.x, c0.z*/
        0x00861011, 0x00000000, 0x00000000, 0x00000068,/*031: exp                r6.x, r6.x*/
        0x00861003, 0x15400800, 0x00000350, 0x00000000,/*032: mul                r6.x, c0.y, r6.x*/
        0x04031003, 0x00006800, 0x00aa02c0, 0x00000000,/*033: mul                r3.w, r6.x, r5.y*/
        0x07861009, 0x00000000, 0x00000000, 0x20390008,/*034: mov                r6, c0*/
        0x04021002, 0x3fc02800, 0x00000340, 0x203fc038,/*035: mad                r2.w, r2.w, r6.x, c3.w*/
        0x03811002, 0x29001800, 0x01fe0140, 0x003fc038,/*036: mad                r1.xyz, r1.xyz, r2.w, r3.w*/
        0x03811003, 0x29001800, 0x01480140, 0x00000002,/*037: mul                r1.xyz, r1.xyz, c2.xyz*/
        0x02051003, 0x2a805800, 0x01fe00c0, 0x00000002,/*038: mul                r5.z, r5.z, c1.w*/
        0x00000116, 0x2a805800, 0x01540240, 0x00001902,/*039: branch.le          r5.z, c4.z, 50*/
        0x03861009, 0x00000000, 0x00000000, 0x00290018,/*040: mov                r6.xyz, r1.xyz*/
        0x04011005, 0x29002800, 0x014801c0, 0x00000000,/*041: dp3                r1.w, r2.xyz, r3.xyz*/
        0x04021001, 0x3fc01800, 0x00000000, 0x003fc018,/*042: add                r2.w, r1.w, r1.w*/
        0x03821002, 0x3fc02800, 0x01480140, 0x00690038,/*043: mad                r2.xyz, r2.w, r2.xyz, -r3.xyz*/
        0x0f831018, 0x29002f20, 0x00000000, 0x00000000,/*044: texld              r3, r2.xyz, s1*/
        0x17821018, 0x29002f20, 0x00000000, 0x00000000,/*045: texld              r2, r2.xyz, s2*/
        0x03831001, 0x29003800, 0x00000000, 0x00690028,/*046: add                r3.xyz, r3.xyz, -r2.xyz*/
        0x03821002, 0x3fc00800, 0x014801d0, 0x00290028,/*047: mad                r2.xyz, c0.w, r3.xyz, r2.xyz*/
        0x03821001, 0x29002800, 0x00000000, 0x00690068,/*048: add                r2.xyz, r2.xyz, -r6.xyz*/
        0x03811002, 0x2a805800, 0x01480140, 0x00290068,/*049: mad                r1.xyz, r5.z, r2.xyz, r6.xyz*/
        0x04011003, 0x3fc04800, 0x01fe0240, 0x00000000,/*050: mul                r1.w, r4.w, r4.w*/
        0x03821001, 0x29003800, 0x00000010, 0x00690018,/*051: add                r2.xyz, c3.xyz, -r1.xyz*/
        0x03811002, 0x3fc01800, 0x01480140, 0x00290018,/*052: mad                r1.xyz, r1.w, r2.xyz, r1.xyz*/
    };

    gctUINT32 replacedShaderSize[gcvMACHINECODE_COUNT] = {
        gcmCOUNTOF(antutu30_replaceShaderCode0),
        gcmCOUNTOF(glb27_release_replaceShaderCode0),
        gcmCOUNTOF(glb25_release_replaceShaderCode0),
        gcmCOUNTOF(glb25_release_replaceShaderCode1),
    };

    gctUINT32* shaderReplacePointer[gcvMACHINECODE_COUNT] = {
        antutu30_replaceShaderCode0,
        glb27_release_replaceShaderCode0,
        glb25_release_replaceShaderCode0,
        glb25_release_replaceShaderCode1,
    };

    if (isDual16Shader)
    {
        switch(Tree->shader->replaceIndex)
        {
            case gcvMACHINECODE_GLB25_RELEASE_0:
                replacedShaderSize  [gcvMACHINECODE_GLB25_RELEASE_0] = gcmCOUNTOF(glb25_release_fp16_replaceShaderCode0);
                shaderReplacePointer[gcvMACHINECODE_GLB25_RELEASE_0] = glb25_release_fp16_replaceShaderCode0;
            break;
        }

    }

    /* REPLACE YOUR SHADER CODE BY CHECKING INCOMING SHADERCODE, AND RETURN
       CORRECT SHADERCODE SIZE (BYTES). DONOT FORGET TO SET YOUR OWN DEFINED
       IDENTIFIER TO Hints->pachedShaderIdentifier, WHICH FLUSH UNIFOMR NEEDS
    */

    /* Do NOT detect process name here, detect when setting the replaceIndex please. */
    if (Tree && Tree->shader && Tree->shader->replaceIndex < gcvMACHINECODE_COUNT)
    {
        gctUINT32 index = Tree->shader->replaceIndex;
        gctUINT32 *patched = gcvNULL;

        newShaderSizeInDW = replacedShaderSize[index];
        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, 4 * newShaderSizeInDW, (gctPOINTER*)&patched)))
        {
            gcmFATAL("%s(%d): gcoOS_Allocate failed", __FUNCTION__, __LINE__);
            return INVALID_SHADER_SIZE;
        }

        gcoOS_MemCopy(patched, shaderReplacePointer[index], 4 * newShaderSizeInDW);
        gcoOS_Free(gcvNULL, *pPatchedShaderCode);
        *pPatchedShaderCode = patched;

        if (Hints)
        {
            Hints->pachedShaderIdentifier = index;
        }

        if (IsMatch)
        {
            *IsMatch = gcvTRUE;
        }
    }
#endif

    return newShaderSizeInDW;
}

static gceSTATUS _PatchShaderByReplaceWholeShaderCode(
    IN gcLINKTREE Tree,
    IN  gcsCODE_GENERATOR_PTR CodeGen,
    OUT gctUINT32** ppPatchedShaderCode,
    OUT gctUINT32* pSizeInDW,
    OUT gcsHINT_PTR Hints
    )
{
    gceSTATUS               status = gcvSTATUS_OK;
    gcsSL_PHYSICAL_CODE_PTR code;
    gctSIZE_T               f, i;
    gctUINT32               shaderSizeInDW = 0;
    gctUINT32*              pPatchedShaderCode = gcvNULL;
    gctUINT32*              pTempShaderCode;
    gctBOOL                 isMatch = gcvFALSE;
    gctBOOL                 isSupportDual16;

    isSupportDual16 = gcHWCaps.hwFeatureFlags.supportDual16;

    /* Assume we are not in patching */
    if (Hints)
        Hints->pachedShaderIdentifier = gcvMACHINECODE_COUNT;

    for (f = 0; f <= Tree->shader->functionCount + Tree->shader->kernelFunctionCount; ++f)
    {
        for (code = CodeGen->functions[f].root;
             code != gcvNULL;
             code = code->next)
        {
            shaderSizeInDW += code->count * 4;
        }
    }

    status = gcoOS_Allocate(gcvNULL, shaderSizeInDW*4, (gctPOINTER*)&pPatchedShaderCode);
    if (gcmIS_ERROR(status))
    {
        gcmFATAL("%s(%d): gcoOS_Allocate failed", __FUNCTION__, __LINE__);
        return status;
    }

    pTempShaderCode = pPatchedShaderCode;
    for (f = 0; f <= Tree->shader->functionCount + Tree->shader->kernelFunctionCount; ++f)
    {
        for (code = CodeGen->functions[f].root;
             code != gcvNULL;
             code = code->next)
        {
            /* Process all states. */
            for (i = 0; i < code->count * 4; ++i)
            {
                *pTempShaderCode++ = code->states[i];
            }
        }
    }

    *ppPatchedShaderCode = pPatchedShaderCode;
    *pSizeInDW = shaderSizeInDW;

    if (CodeGen->shaderType != gcSHADER_TYPE_FRAGMENT)
        return status;

    /* Replace shader code */
    shaderSizeInDW = _ReplaceShader(Tree, &pPatchedShaderCode, shaderSizeInDW,
                                    !isSupportDual16
                                    ? gcvFALSE : CodeGen->isDual16Shader,
                                    &isMatch,
                                    Hints);

    if (shaderSizeInDW == INVALID_SHADER_SIZE)
    {
        if (pPatchedShaderCode && pPatchedShaderCode != *ppPatchedShaderCode)
        {
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, pPatchedShaderCode));
        }
        return gcvSTATUS_OUT_OF_MEMORY;
    }

    CodeGen->instCount = shaderSizeInDW / 4;

    /* Adjust end PC */
    if (Hints && isMatch)
        CodeGen->endPC = shaderSizeInDW/4;

    *ppPatchedShaderCode = pPatchedShaderCode;
    *pSizeInDW = shaderSizeInDW;

    return status;
}
#endif


static gceSTATUS
_GenerateStates(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gctPOINTER StateBuffer,
    IN OUT gctSIZE_T * Size,
    IN gctPOINTER StateDeltaBuffer,
    IN OUT gctSIZE_T *StateDeltaBufferSize,
    OUT gcsHINT_PTR Hints
    )
{
    gceSTATUS status;
    gctSIZE_T i, attributeCount, outputCount;
    gcsSL_CONSTANT_TABLE_PTR c;
    gctUINT32 uniformBase, instrBase;
    gctUINT32 codeAddress;
    gctUINT32 timeout;
    gctUINT32 maxNumInstrStates;
    gctUINT32 numInstrStates = 0;
    gctBOOL usePositionInVSOutput = gcvFALSE;  /* if glPositoin is used in VS output */
    gctBOOL hasPrecisionSetting = gcvFALSE;
    gctBOOL useRegedCTC = gcvFALSE;
    gcSHADER_PRECISION precision = gcSHADER_PRECISION_MEDIUM;
    gctBOOL hasShaderMediumP = gcvFALSE;
    gctBOOL isMediumpVS = gcvFALSE;
    gctBOOL isMediumpPS = gcvFALSE;
    gctUINT32 components = 0, rows = 0;
#if TEMP_SHADER_PATCH
    gctUINT32 shaderSizeInDW = 0;
    gctUINT32* BinaryShaderCode = gcvNULL;
#endif
    gctBOOL dumpCodeGen        = gcSHADER_DumpCodeGen(Tree->shader) && (StateBuffer != gcvNULL);
    gctINT uboSize = 0;
    gctUINT32_PTR constAddr = gcvNULL;
    gctINT32  output2RTIndex[gcdMAX_DRAW_BUFFERS];
    gctUINT32 shaderConfigData = 0;
    gctPOINTER instPtr = gcvNULL;

#if TEMP_SHADER_PATCH
    gcmONERROR(_PatchShaderByReplaceWholeShaderCode(Tree, CodeGen, &BinaryShaderCode,
                                                    &shaderSizeInDW, Hints));
#endif

    for (i = 0; i < gcdMAX_DRAW_BUFFERS; ++i)
    {
        output2RTIndex[i] = -1;
    }

    /* Initialize state buffer management. */
    CodeGen->stateBuffer             = StateBuffer;
    CodeGen->stateBufferSize         = (StateBuffer == gcvNULL) ? ~0U : *Size;
    CodeGen->stateBufferOffset       = 0;
    CodeGen->lastStateCommand        = gcvNULL;
    CodeGen->stateDeltaBuffer        = StateDeltaBuffer;
    CodeGen->stateDeltaSize          = (StateBuffer == gcvNULL) ? ~0U : *StateDeltaBufferSize;
    CodeGen->stateDeltaBufferOffset  = 0;
    CodeGen->lastStateDeltaBatchEnd  = gcvNULL;
    CodeGen->lastStateDeltaBatchHead = gcvNULL;

    /*
    ** FIXME: we temporarily disabled ps medium presion optimization as GL3Tests/half_float/input.run's 3D texture test using medium presion ps,
    ** but the result has 1 bit deviation with reference. The reference should be more tolerant.
    */
    if (CodeGen->hasSHEnhancements3 &&
       CodeGen->shaderType == gcSHADER_TYPE_FRAGMENT)
    {
       hasShaderMediumP = gcvFALSE;
    }

    if (CodeGen->clShader && !CodeGen->hasBugFixes10)
    {
        if (CodeGen->isCL_XE)
        {
            gcCGUpdateMaxRegister(CodeGen, 3, Tree);
        }
        else
        {
            gcCGUpdateMaxRegister(CodeGen, 9, Tree);
        }
    }

    if (CodeGen->useICache &&
        !gcHWCaps.hwFeatureFlags.hasICacheAllocCountFix)
    {
        gcCGUpdateMaxRegister(CodeGen, 3, Tree);
    }

    /* Count number of active attributes. */
    for (i = attributeCount = 0; i < Tree->attributeCount; ++i)
    {
        if (Tree->attributeArray[i].inUse)
        {
            if (Tree->shader->attributes[i]->nameLength != gcSL_POSITION)
            {
                if (Tree->shader->attributes[i]->nameLength != gcSL_FRONT_FACING &&
                    Tree->shader->attributes[i]->nameLength != gcSL_HELPER_INVOCATION)
                {
                    components = rows = 0;
                    gcTYPE_GetTypeInfo(Tree->shader->attributes[i]->type,
                                       &components, &rows, 0);
                    rows *= Tree->shader->attributes[i]->arraySize;
                    if (gcmATTRIBUTE_isRegAllocated(Tree->shader->attributes[i]))
                    {
                        /* only count the attribute if it is allocated register */
                        attributeCount += rows;
                    }
                }

              /*Do medium precision checking */
                if (hasShaderMediumP &&
                    gcmType_Kind(Tree->shader->attributes[i]->type) == gceTK_FLOAT)
                {
                    hasPrecisionSetting = gcvTRUE;
                    if (Tree->shader->attributes[i]->precision <= gcSHADER_PRECISION_HIGH)
                    {
                        precision = Tree->shader->attributes[i]->precision;
                    }
                }
            }
        }
    }

    /* Count number of active outputs. */
    for (i = outputCount = 0; i < Tree->outputCount; ++i)
    {
        if (Tree->outputArray[i].inUse
             ||
             ((Tree->shader->outputs[i] != gcvNULL)
               &&
               ((Tree->shader->outputs[i]->nameLength == gcSL_POSITION)
                 ||
                 (Tree->shader->outputs[i]->nameLength == gcSL_POINT_SIZE)
                 ||
                 (Tree->shader->outputs[i]->nameLength == gcSL_COLOR)
                 ||
                 (Tree->shader->outputs[i]->nameLength == gcSL_DEPTH)
               )
             )
           )
        {
            components = rows = 0;
            gcTYPE_GetTypeInfo(Tree->shader->outputs[i]->type, &components, &rows, 0);
            outputCount += rows;

            /*Do medium precision checking */
            if (hasShaderMediumP &&
                Tree->shader->outputs[i] != gcvNULL &&
                (Tree->shader->outputs[i]->nameLength != gcSL_POSITION &&
                 Tree->shader->outputs[i]->nameLength != gcSL_DEPTH) &&
                gcmType_Kind(Tree->shader->outputs[i]->type) == gceTK_FLOAT)
            {
                hasPrecisionSetting = gcvTRUE;
                if (Tree->shader->outputs[i]->precision <= gcSHADER_PRECISION_HIGH)
                {
                    precision = Tree->shader->outputs[i]->precision;
                }
            }

            if (Tree->shader->outputs[i]->nameLength == gcSL_POSITION)
            {
                usePositionInVSOutput = gcvTRUE;
            }
        }
    }

    if (CodeGen->shaderType == gcSHADER_TYPE_VERTEX && !usePositionInVSOutput)
    {
        /* adjust vertex shader output count if glPosition is not in its output
         * due to we always assume glPosition is r0*/
        outputCount += 1;
    }

    if (hasShaderMediumP)
    {
        /* do medium precision checking */
        /* check for samplers */
        for (i = 0; i < Tree->shader->uniformCount; ++i)
        {
            gcUNIFORM uniform = Tree->shader->uniforms[i];

            if(!uniform) continue;

            if (!isUniformNormal(uniform) &&
                !isUniformBlockMember(uniform))
                continue;
            if (_IsSampler(uniform->u.type))
            {
                if (_GetSamplerKind(uniform->u.type) == gceTK_FLOAT)
                {
                    hasPrecisionSetting = gcvTRUE;
                    if (uniform->precision <= gcSHADER_PRECISION_HIGH)
                    {
                        precision = uniform->precision;
                    }
                }
                break;
            }
        }

        for (i = 0; i < Tree->shader->variableCount; ++i)
        {
            gcVARIABLE variable = Tree->shader->variables[i];
            if (isVariableNormal(variable))
            {
                gcLINKTREE_TEMP temp = gcvNULL;
                temp = Tree->tempArray + variable->tempIndex;
                if(temp && temp->inUse &&
                    variable->nameLength != gcSL_POSITION &&
                    gcmType_Kind(variable->u.type) == gceTK_FLOAT)
                {
                    hasPrecisionSetting = gcvTRUE;
                    if(variable->precision <= gcSHADER_PRECISION_HIGH)
                    {
                       precision = variable->precision;
                    }
                }
            }
        }
    }

    if (CodeGen->shaderType == gcSHADER_TYPE_VERTEX)
    {
#define _OneForth_MAX_VARYINGS (_MAX_VARYINGS+3)/4
        /* Vertex shader. */
        gctUINT32 VertexShaderOutputReg[_OneForth_MAX_VARYINGS];
        /* if no packing: components numbers -1 (0-3),
           if packing:  4 + packingMode */
        gctINT32  VertexShaderOutputComponentsEncode[_MAX_VARYINGS];
        gctBOOL hasPointSize = gcvFALSE;
        gctINT    outputComponentCount = 0;

        /* Zero the outputs. */
        gcoOS_ZeroMemory(VertexShaderOutputReg, sizeof(VertexShaderOutputReg));
        gcoOS_ZeroMemory(VertexShaderOutputComponentsEncode,
                         sizeof(VertexShaderOutputComponentsEncode));

        gcmASSERT(CodeGen->maxVaryingVectors <= _MAX_VARYINGS);
        /* Walk through all outputs. */
        for (i = 0; i < Tree->outputCount; ++i)
        {
            gctINT link = 0, index, shift = 0, temp, reg;
            gctBOOL inUse;
            gctINT32  componentsEncode = 0;

            if (Tree->shader->outputs[i] == gcvNULL)
            {
                continue;
            }

            /* Extract internal name for output. */
            switch (Tree->shader->outputs[i]->nameLength)
            {
            case gcSL_POSITION:
                /* Position is always at r0. */
                link  = 0;
                inUse = gcvTRUE;
                componentsEncode = 4 - 1;
                break;

            case gcSL_POINT_SIZE:
                /* Point size is always at the end. */
                link         = outputCount - 1;
                inUse        = gcvTRUE;
                hasPointSize = gcvTRUE;
                componentsEncode = 1 -1;
                break;

            case gcSL_FRONT_COLOR:
            case gcSL_BACK_COLOR:
            case gcSL_FRONT_SECONDARY_COLOR:
            case gcSL_BACK_SECONDARY_COLOR:
                if(Tree->tempArray[Tree->outputArray[i].tempHolding].assigned == -1)
                {
                    Tree->tempArray[Tree->outputArray[i].tempHolding].assigned = 0;
                }

                {
                    /* Determine linked fragment attribute register. */
                    inUse = Tree->outputArray[i].inUse;
                    if (inUse)
                    {
                        link = _getLinkedFragmentAttributeLinkIndex(Tree, i);
                        gcmASSERT(Tree->outputArray[i].components > 0 &&
                            Tree->outputArray[i].components <= 4);
                        gcmASSERT(link < (gctINT)outputCount);
                        componentsEncode =
                            Tree->outputArray[i].isPacked ? Tree->outputArray[i].packingMode + 4
                            : Tree->outputArray[i].components - 1;
                    }
                }
                break;

            default:
                /* Determine linked fragment attribute register. */
                inUse = Tree->outputArray[i].inUse;
                if (inUse)
                {
                    link = _getLinkedFragmentAttributeLinkIndex(Tree, i);
                    gcmASSERT(Tree->outputArray[i].components > 0 &&
                              Tree->outputArray[i].components <= 4);
                    gcmASSERT(link < (gctINT)outputCount);
                    componentsEncode =
                        Tree->outputArray[i].isPacked ? Tree->outputArray[i].packingMode + 4
                                                      : Tree->outputArray[i].components - 1;
                }
                break;
            }

            /* Only process enabled outputs. */
            if (inUse)
            {
                gctINT endLink, thisLink, thisTemp;
                components = rows = 0;
                gcTYPE_GetTypeInfo(Tree->shader->outputs[i]->type, &components, &rows, 0);
                gcmASSERT((gctINT)rows ==
                          (Tree->outputArray[i].fragmentIndexEnd - Tree->outputArray[i].fragmentIndex + 1));

                outputComponentCount += components * rows;

                temp = Tree->outputArray[i].tempHolding;
                endLink = link + rows;

                for (thisLink = link, thisTemp = temp; thisLink < endLink; thisLink ++, thisTemp ++)
                {
                    /* Convert link register into output array index. */
                    /* encode every 4 output to one 32bit AQVertexShaderOutput register:
                          address 0 (byte 0):  output[4*n]'s register
                          address 1 (byte 1):  output[4*n+1]'s register
                          address 2 (byte 2):  output[4*n+2]'s register
                          address 3 (byte 3):  output[4*n+3]'s register
                     */
                    index = thisLink >> 2;
                    shift = (thisLink & 3) * 8;

                    /* Copy register into output array. */
                    if (_isHWRegisterAllocated(Tree->shader))
                    {
                        reg  = thisTemp;
                    }
                    else
                    {
                        reg  = Tree->tempArray[thisTemp].assigned;
                    }
                    if (reg != -1)
                    {
                        gcmASSERT(reg != -1);

                        if (thisLink < 16)
                        {
                            gcmASSERT(index < _OneForth_MAX_VARYINGS);
                            VertexShaderOutputReg[index] |= reg << shift;
                        }
                        else
                        {
                            gcmASSERT(thisLink < (gctINT)(16 + CodeGen->maxExtraVaryingVectors));

                            if (Hints != gcvNULL)
                            {
                                if (thisLink == 16)
                                {
                                    Hints->vsOutput16RegNo = reg;
                                }
                                else if (thisLink == 17)
                                {
                                    Hints->vsOutput17RegNo = reg;
                                }
                                else if (thisLink == 18)
                                {
                                    Hints->vsOutput18RegNo = reg;
                                }
                                else
                                {
                                    gcmASSERT(gcvFALSE);
                                }
                            }
                        }

                        VertexShaderOutputComponentsEncode[thisLink] =
                                                           componentsEncode;

                        gcCGUpdateMaxRegister(CodeGen, (gctUINT)reg, Tree);
                    }
                }
            }
        }
        /* patch glPosition output if it is not in VS output */
        if (!usePositionInVSOutput)
        {
        }

        if (Hints != gcvNULL)
        {
            Hints->vsInstCount    = CodeGen->instCount;
            Hints->vertexShaderId = Tree->shader->_id;
        }

        if (dumpCodeGen)
        {
            gcoOS_Print("\n");
            gcoOS_Print("==============");
            gcoOS_Print("VS: (id:%d)", Tree->shader->_id);
            if(Tree->shader->_id % 16 - 1)
            {
                gcoOS_Print("VS: Patched");
            }
            gcoOS_Print("VS: EndPC=%u", CodeGen->endPC);
            gcoOS_Print("VS: InstCount=%u", CodeGen->instCount);
        }

        /* Hints */
        if (Hints != gcvNULL)
        {
            Hints->vsOutputCount  = outputCount;
            Hints->vsHasPointSize = hasPointSize;
            Hints->vsPtSizeAtLastLinkLoc = gcvTRUE;
            Hints->prePaShaderHasPointSize = hasPointSize;
#if gcdUSE_WCLIP_PATCH
            Hints->vsPositionZDependsOnW = Tree->shader->vsPositionZDependsOnW;
#endif

            if (CodeGen->clShader)
            {
                /* Kernel shader does not have extra attribute as fragment shader. */
                /* However, fsInputCount cannot be 0. */
                if (attributeCount == 0) attributeCount = 1;
                Hints->fsInputCount   = attributeCount;
                Hints->fsMaxTemp      = CodeGen->maxRegister + 1;
                Hints->elementCount   = 0;
                Hints->shader2PaOutputCount = 0;

            }
        }

        if (dumpCodeGen)
        {
            const char * str[] =
            { "1", "2", "3", "4", "2_2", "3_1", "2_1_1", "1_1_1_1"};

            /* print input attrubute register mapping */
            gcoOS_Print("VS: inputCount=%u, attribute mapping:",
                          attributeCount);
            for (i = 0; i < Tree->shader->attributeCount; ++i)
            {
                gcATTRIBUTE      attribute = Tree->shader->attributes[i];
                gctCONST_STRING  name;
                gctUINT32        components = 0, rows = 0;

                if (!Tree->attributeArray[i].inUse)
                    continue;   /* skip unused attributes */

                /* Determine rows and components. */
                gcTYPE_GetTypeInfo(attribute->type, &components, &rows, 0);
                rows *= attribute->arraySize;
                gcATTRIBUTE_GetName(Tree->shader, attribute, gcvTRUE, gcvNULL, &name);
                if (rows > 1)
                {
                    gcoOS_Print("VS: Attribute(%d) %s ==> r%d - r%d",
                        attribute->index, name,
                        attribute->inputIndex,
                        attribute->inputIndex + rows - 1);
                }
                else
                {
                    gcoOS_Print("VS: Attribute(%d) %s ==> r%d",
                        attribute->index, name, attribute->inputIndex);
                }
            }

            gcoOS_Print("VS: outputCount=%u %s",
                          outputCount,
                          hasPointSize ? "(pointSize)" : "");
            gcoOS_Print("VS: attributes mapping:");

            for (i = 0; i < ((outputCount > 16 ? 16 : outputCount) + 3) / 4; ++i)
            {
                if (i*4+3 < outputCount)
                {
                    /* 4 more output left */
                    gcoOS_Print("VS: o%d <- r%d (%s), o%d <- r%d (%s), o%d <- r%d (%s), o%d <- r%d (%s)",
                                i*4, VertexShaderOutputReg[i] & 0x3f,
                                    str[VertexShaderOutputComponentsEncode[i*4]],
                                i*4+1, (VertexShaderOutputReg[i] >> 8) & 0x3f,
                                    str[VertexShaderOutputComponentsEncode[i*4+1]],
                                i*4+2, (VertexShaderOutputReg[i] >> 16) & 0x3f,
                                    str[VertexShaderOutputComponentsEncode[i*4+2]],
                                i*4+3, (VertexShaderOutputReg[i] >> 24) & 0x3f,
                                    str[VertexShaderOutputComponentsEncode[i*4+3]]);
                }
                else if (i*4+2 < outputCount)
                {
                    /* 3 output left */
                    gcoOS_Print("VS: o%d <- r%d (%s), o%d <- r%d (%s), o%d <- r%d (%s)",
                                i*4, VertexShaderOutputReg[i] & 0x3f,
                                    str[VertexShaderOutputComponentsEncode[i*4]],
                                i*4+1, (VertexShaderOutputReg[i] >> 8) & 0x3f,
                                    str[VertexShaderOutputComponentsEncode[i*4+1]],
                                i*4+2, (VertexShaderOutputReg[i] >> 16) & 0x3f,
                                    str[VertexShaderOutputComponentsEncode[i*4+2]]);
                }
                else if (i*4+1 < outputCount)
                {
                    /* 2 output left */
                    gcoOS_Print("VS: o%d <- r%d (%s), o%d <- r%d (%s)",
                                i*4, VertexShaderOutputReg[i] & 0x3f,
                                    str[VertexShaderOutputComponentsEncode[i*4]],
                                i*4+1, (VertexShaderOutputReg[i] >> 8) & 0x3f,
                                    str[VertexShaderOutputComponentsEncode[i*4+1]]);
                }
                else
                {
                    /* 1 output left */
                    gcoOS_Print("VS: o%d <- r%d (%s)",
                                i*4, VertexShaderOutputReg[i] & 0x3f,
                                    str[VertexShaderOutputComponentsEncode[i*4]]);
                }
            }

            if (Hints)
            {
                for (i = 16; i < outputCount; i ++)
                {
                    if (i == 16)
                    {
                        gcoOS_Print("VS: o%d <- r%d (%s)", i,
                            Hints->vsOutput16RegNo, str[VertexShaderOutputComponentsEncode[i]]);
                    }
                    else if (i == 17)
                    {
                        gcoOS_Print("VS: o%d <- r%d (%s)", i,
                            Hints->vsOutput17RegNo, str[VertexShaderOutputComponentsEncode[i]]);
                    }
                    else if (i == 18)
                    {
                        gcoOS_Print("VS: o%d <- r%d (%s)", i,
                            Hints->vsOutput18RegNo, str[VertexShaderOutputComponentsEncode[i]]);
                    }
                    else
                    {
                        gcmASSERT(gcvFALSE);
                    }
                }
            }
        }

        /* AQVertexShaderInputControl */
        timeout = gcmALIGN(attributeCount * 4 + 4, 16) / 16;
        if (attributeCount > CodeGen->maxAttributes )
        {
            gcmONERROR(gcvSTATUS_TOO_MANY_ATTRIBUTES);
        }
        if(CodeGen->shaderType == gcSHADER_TYPE_VERTEX &&
           CodeGen->vsHasVertexInstId &&
           gcHWCaps.hwFeatureFlags.vtxInstanceIdAsAttr)
        {
           if (attributeCount < CodeGen->maxAttributes )
           {
                /* special handle for HW which doesn't support zero attribute,
                 * driver needs to add one dummy input, vertexID is allocated
                 * to r1 by regAllocator, here we need to tell driver that
                 * we have one dummy input */
               gctBOOL needDummyInput = _needAddDummyAttribute(Tree, CodeGen);
               gcmONERROR(_SetState(CodeGen,
                    0x0202,
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (needDummyInput ?
 attributeCount + 2 : attributeCount + 1) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) ((gctUINT32) (timeout) & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))));
           }
           else
           {
                /* Get vertex/instance attribute is used by the current shader */
                gctINT vertexInstIndex = gcSHADER_GetVertexInstIdInputIndex(Tree->shader);

                gcmONERROR(_SetState(CodeGen,
                    0x0202,
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (attributeCount + 1) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:16) - (0 ?
 21:16) + 1))))))) << (0 ?
 21:16))) | (((gctUINT32) ((gctUINT32) (vertexInstIndex) & ((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ? 21:16))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) ((gctUINT32) (timeout) & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))));
           }
        }
        else
        {
           gcmONERROR(_SetState(CodeGen,
                                0x0202,
                                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (attributeCount ?
 attributeCount : 1) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0))) |
                                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) ((gctUINT32) (timeout) & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))));
        }
        if (dumpCodeGen)
        {
            gcoOS_Print("VS: attributeCount=%u", attributeCount);
            gcoOS_Print("VS: timeout=%u", timeout);
        }

        if (CodeGen->flags & gcvSHADER_TEXLD_W)
        {
            _adjustMaxTemp(Tree, CodeGen);
        }

        /* AQVertexShaderTemporaryRegisterControl */
        gcmONERROR(
            _SetState(CodeGen,
                      0x0203,
                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:0) - (0 ?
 6:0) + 1))))))) << (0 ?
 6:0))) | (((gctUINT32) ((gctUINT32) (CodeGen->maxRegister + 1) & ((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:0) - (0 ? 6:0) + 1))))))) << (0 ? 6:0)))));

        if (dumpCodeGen)
        {
            gcoOS_Print("VS: numRegisters=%u", CodeGen->maxRegister + 1);
        }


        if (gcHWCaps.hwFeatureFlags.newGPIPE)
        {
            gctUINT groupSize;
            gctUINT maxThreads;
            gctUINT resultSize;
            gctUINT vsPages = 32; /* program 32KB for VS only case for now */
            gctUINT numberVertecis;
            /* 0x0238 */
            for (i = 0; i < ((outputCount > _MAX_VARYINGS ? _MAX_VARYINGS : outputCount) + 3) / 4; ++i)
            {
                gcmONERROR(
                    _SetState(CodeGen,
                              0x0238 + i,
                              VertexShaderOutputReg[i]));
            }
            /* SW WA for transform feedback, when VS has no output. */
            if (outputCount == 0)
            {
                gcmONERROR(
                    _SetState(CodeGen,
                              0x0238,
                              0));
            }

            numberVertecis = (vsPages * 1024) / (outputCount * 16);

            maxThreads = (gcHWCaps.maxCoreCount * 4);

            groupSize = maxThreads * outputCount;

            resultSize = (numberVertecis < 128) ? (numberVertecis / 4) : (numberVertecis / 2);

            resultSize = (resultSize > 256) ? 256 : resultSize;

            gcmONERROR(
                _SetState(CodeGen,
                          0x021C,
                          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (outputCount) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0))) |
                          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:8) - (0 ?
 18:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:8) - (0 ?
 18:8) + 1))))))) << (0 ?
 18:8))) | (((gctUINT32) ((gctUINT32) (groupSize) & ((gctUINT32) ((((1 ?
 18:8) - (0 ?
 18:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:8) - (0 ? 18:8) + 1))))))) << (0 ? 18:8)))));
            gcmONERROR(
                _SetState(CodeGen,
                          0x0228,
                          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (vsPages) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0))) |
                          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:12) - (0 ?
 19:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:12) - (0 ?
 19:12) + 1))))))) << (0 ?
 19:12))) | (((gctUINT32) ((gctUINT32) (maxThreads) & ((gctUINT32) ((((1 ?
 19:12) - (0 ?
 19:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:12) - (0 ? 19:12) + 1))))))) << (0 ? 19:12))) |
                          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:20) - (0 ?
 28:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:20) - (0 ?
 28:20) + 1))))))) << (0 ?
 28:20))) | (((gctUINT32) ((gctUINT32) (resultSize) & ((gctUINT32) ((((1 ?
 28:20) - (0 ?
 28:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:20) - (0 ? 28:20) + 1))))))) << (0 ? 28:20)))));

        }
        else
        {
            /* 0x0204 */
            for (i = 0; i < ((outputCount > _MAX_VARYINGS ? _MAX_VARYINGS : outputCount) + 3) / 4; ++i)
            {
                gcmONERROR(_SetState(CodeGen,
                                     0x0204 + i,
                                     VertexShaderOutputReg[i]));
            }

            /* SW WA for transform feedback, when VS has no output. */
            if (outputCount == 0)
            {
                gcmONERROR(_SetState(CodeGen,
                                     0x0204,
                                     0));
            }
        }


        /* Load balancing min and max. */
        if (Hints != gcvNULL)
        {
            if (outputCount > 0)
            {
                gctSIZE_T effectiveOutputCount;
                gctINT min, max;
                gctSIZE_T prePackingOutputCount = outputCount +
                                               Tree->packedAwayOutputCount;
                gctBOOL hasOutputCountFix = gcHWCaps.hwFeatureFlags.outputCountFix;

                effectiveOutputCount = !hasOutputCountFix ?
                    gcmALIGN(prePackingOutputCount, 2)
                    : (((outputComponentCount + 7) >> 2)
                      + ((Hints->vsHasPointSize && ((outputComponentCount % 4 )) == 0) ?
                          1 : 0));

                Hints->balanceMin = ((256 * 10
                                      * 8 /* Pipe line depth. */
                                      / (gcHWCaps.vertexOutputBufferSize-
                                          effectiveOutputCount * gcHWCaps.vertexCacheSize)
                                      ) + 9
                                    ) / 10;
                Hints->balanceMax = gcmMIN(255, 512 / (gcHWCaps.maxCoreCount * effectiveOutputCount));

                min = Hints->balanceMin;
                max = Hints->balanceMax;
                if (gcOPT_getLoadBalanceForShader(Tree->shader, &min, &max))
                {
                    gcmASSERT(min>0 && max >= min);
                    /* over write the caculated values */
                    Hints->balanceMin = min;
                    Hints->balanceMax = max;
                }
            }
            else
            {
                /* XFB case. Set balanceMin/Max per effectiveOutputCount as 1. */
                Hints->balanceMin = ((256 * 10
                                      * 8 /* Pipe line depth. */
                                      / (gcHWCaps.vertexOutputBufferSize -
                                          1 * gcHWCaps.vertexCacheSize)
                                      ) + 9
                                    ) / 10;
                Hints->balanceMax = gcmMIN(255, 512 / (gcHWCaps.maxCoreCount));
            }
            if (dumpCodeGen)
            {
                gcoOS_Print("VS: balanceMin=%u, balanceMax=%u", Hints->balanceMin, Hints->balanceMax);
            }
        }

        if(Hints != gcvNULL)
        {
            Hints->vsMaxTemp      = CodeGen->maxRegister + 1;
            Hints->threadWalkerInPS = gcvFALSE;
        }

        if (CodeGen->clShader || CodeGen->computeShader)
        {
            gctUINT32 varyingPacking[2] = {0, 0};

            if (Hints != gcvNULL) {

                switch (Hints->elementCount)
                {
                case 3:
                    varyingPacking[1] = 2;
                    /*fall through*/
                case 2:
                    varyingPacking[0] = 2;
                }

                Hints->componentCount = gcmALIGN(varyingPacking[0] + varyingPacking[1], 2);
            }

            gcmONERROR(
                _SetState(CodeGen,
                          0x0E08,
                          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (varyingPacking[0]) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))) |
                          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:4) - (0 ?
 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) ((gctUINT32) (varyingPacking[1]) & ((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ? 6:4)))));

            gcmONERROR(
                _SetState(CodeGen,
                          0x0E0D,
                          0));
        }

        if (CodeGen->isDual16Shader ||
            (hasShaderMediumP &&
             (precision == gcSHADER_PRECISION_MEDIUM) && hasPrecisionSetting)) {
            isMediumpVS = gcvTRUE;
        }
        else {
            isMediumpVS = gcvFALSE;
        }

        if (CodeGen->hasHalti5)
        {
            shaderConfigData =
                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) ((gctUINT32) ((gcHWCaps.hwFeatureFlags.rtneRoundingEnabled ?
 0x1 : 0x0)) & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)));
            gcmONERROR(
                _SetState(CodeGen,
                          0x5580,
                          shaderConfigData
                          ));

        }
        else
        {
            shaderConfigData =
                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) ((gctUINT32) ((isMediumpVS ?
 0x1 : 0x0)) & ((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ? 28:28)))
                    /* Need to keep existing RTNE setting. */
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) ((gcHWCaps.hwFeatureFlags.rtneRoundingEnabled ?
 0x1 : 0x0)) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)));
            gcmONERROR(
                _SetState(CodeGen,
                          0x0218,
                          shaderConfigData
                          ));

        }




        /* Save the vertex precision. */
        if (Hints != gcvNULL)
        {
            if ((Hints->memoryAccessFlags[gcvSHADER_MACHINE_LEVEL][VSC_SHADER_STAGE_VS] & gceMA_FLAG_ATOMIC) &&
                gcHWCaps.hwFeatureFlags.robustAtomic)
            {
                shaderConfigData |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)));
            }

            Hints->shaderConfigData = shaderConfigData;
            Hints->unifiedStatus.instVSEnd = CodeGen->endPC - 1;
        }

        if (CodeGen->useICache)
        {
            /* Set code address. */
            instrBase         = (gctUINT32)(gctUINTPTR_T) StateBuffer;
            maxNumInstrStates = gcHWCaps.maxHwNativeTotalInstCount << 2;
        }
        else
        {
            gcmASSERT(!CodeGen->hasHalti5);
            /* set start/end PC */
            if (CodeGen->hasICache)
            {
                gcmONERROR(_SetState(CodeGen,
                                     0x021D,
                                     0));

                gcmONERROR(_SetState(CodeGen,
                                     0x021E,
                                     CodeGen->endPC-1));

            }
            else
            {
                if (gcHWCaps.maxHwNativeTotalInstCount <= 256)
                {
                    /* AQVertexShaderStartPC */
                    gcmONERROR(_SetState(CodeGen,
                                         0x020E,
                                         ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:0) - (0 ?
 11:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:0) - (0 ?
 11:0) + 1))))))) << (0 ?
 11:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 11:0) - (0 ?
 11:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:0) - (0 ? 11:0) + 1))))))) << (0 ? 11:0))) |
                                         ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)))
                                         ));

                    /* AQVertexShaderEndPC */
                    gcmONERROR(_SetState(CodeGen,
                                         0x0200,
                                         CodeGen->endPC
                                         ));

                }
                else
                {
                    gcmONERROR(_SetState(CodeGen,
                                  0x0217,
                                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (CodeGen->endPC-1 ) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
                                  ));

                }
            }

            /* Set code address. */
            if (gcHWCaps.maxHwNativeTotalInstCount > 1024)
            {
                instrBase         = 0x8000;
                maxNumInstrStates = gcHWCaps.maxHwNativeTotalInstCount << 2;

            }
            else if (gcHWCaps.maxHwNativeTotalInstCount > 256)
            {
                instrBase         = 0x3000;
                maxNumInstrStates = gcHWCaps.maxHwNativeTotalInstCount << 2;

            }
            else
            {
                instrBase         = 0x1000;
                maxNumInstrStates = 1024;
            }

        }

        if (CodeGen->hasICache && !CodeGen->useICache)
        {
            gcmONERROR(_SetState(CodeGen,
                                 0x021A,
                                   ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
                                 | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)))


                                 ));

            gcmONERROR(_SetState(CodeGen,
                                 0x021B,
                                 0
                                 ));
        }

        /* Set uniform address. */
        uniformBase = CodeGen->uniformBase;

        if (CodeGen->unifiedUniform)
        {
            /* Set offset. */
            gcmONERROR(
                _SetState(CodeGen,
                          0x0219,
                          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:0) - (0 ?
 9:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:0) - (0 ?
 9:0) + 1))))))) << (0 ?
 9:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 9:0) - (0 ?
 9:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ? 9:0)))
                          ));

            if ((Hints != gcvNULL) && ! CodeGen->clShader)
            {
                gctUINT         count = 0;
                gcsSL_USAGE_PTR usage;

                /* Get constant count. */
                for (count = 0, usage = CodeGen->uniformUsage;
                     count < CodeGen->maxUniform;
                     count++, usage++)
                {
                    if (usage->lastUse[0] == gcvSL_AVAILABLE &&
                        usage->lastUse[1] == gcvSL_AVAILABLE &&
                        usage->lastUse[2] == gcvSL_AVAILABLE &&
                        usage->lastUse[3] == gcvSL_AVAILABLE)
                    {
                        break;
                    }
                }

                Hints->unifiedStatus.constantUnifiedMode = gcvUNIFORM_ALLOC_PACK_FLOAT_BASE_OFFSET;
                Hints->unifiedStatus.constGPipeEnd = count;
                Hints->maxConstCount = gcHWCaps.maxTotalConstRegCount;
                Hints->vsConstCount = count;
                Hints->unifiedStatus.constCount = Hints->vsConstCount;
            }
        }
    }
    else
    {
        /* Fragment shader and CL kernel. */
        gctUINT32 address;
        gctBOOL halfAttribute = gcvFALSE;
        gctBOOL hasFragDepth = gcvFALSE;
        gctINT index;
        gctSTRING shaderTypeStr;

        gcmASSERT(CodeGen->shaderType == gcSHADER_TYPE_FRAGMENT ||
                  CodeGen->shaderType == gcSHADER_TYPE_CL ||
                  CodeGen->shaderType == gcSHADER_TYPE_COMPUTE);
        shaderTypeStr = gcSL_GetShaderKindString(CodeGen->shaderType);
        /* Find the last used attribute. */
        for (index = Tree->attributeCount - 1; index >= 0; --index)
        {
            if (Tree->attributeArray[index].inUse)
            {
                /* Test type of attribute. */
                switch (Tree->shader->attributes[index]->type)
                {
                case gcSHADER_FLOAT_X1:
                case gcSHADER_FLOAT_X2:
                case gcSHADER_FLOAT_2X2:
                    /* Half attribute can be enabled for FLOAT_X1 and
                       FLOAT_X2 types. */
                    halfAttribute = gcvTRUE;
                    break;

                default:
                    break;
                }

                /* Bail out loop. */
                break;
            }
        }

        /* AQRasterControl */
        gcmONERROR(_SetState(CodeGen,
                             0x0380,
                             ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) |
                             ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) ((gctUINT32) (halfAttribute) & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))));

        if (Hints != gcvNULL)
        {
            Hints->fsInstCount    = CodeGen->instCount;
            Hints->fsIsDual16     = CodeGen->isDual16Shader;
            Hints->psInputControlHighpPosition = CodeGen->psInputControlHighpPosition;
            Hints->fragmentShaderId            = Tree->shader->_id;

        }

        if (dumpCodeGen)
        {
            gcoOS_Print("\n");
            gcoOS_Print("==============");
            gcoOS_Print("%s: (id:%d)", shaderTypeStr, Tree->shader->_id);
            if((Tree->shader->_id % 16) - 2)
            {
                gcoOS_Print("%s: Patched", shaderTypeStr);

            }
            if(CodeGen->isDual16Shader)
            {
                gcoOS_Print("%s: DUAL16 shader, default precision 'mediump'", shaderTypeStr);
                gcoOS_Print("%s: Highp position %s", shaderTypeStr, Hints->psInputControlHighpPosition == 0x1
                                                     ? "enabled"
                                                     : "disabled");
            }
            gcoOS_Print("%s: EndPC=%u", shaderTypeStr, CodeGen->endPC);
            gcoOS_Print("%s: InstCount=%u", shaderTypeStr, CodeGen->instCount);
        }

        /* Detect alpha assignment for output. */
        if (Tree->outputCount > 0 &&
            Tree->shader->codeCount > 0 &&
            Tree->shader->type == gcSHADER_TYPE_FRAGMENT &&
            Hints != gcvNULL &&
            gcGetVIRCGKind(Tree->hwCfg.hwFeatureFlags.hasHalti2) == VIRCG_None)
        {
            gctINT32 index[gcdMAX_DRAW_BUFFERS] = {-1, -1, -1, -1};
            gctINT outputCount = 0;

            if (!gcSHADER_IsHaltiCompiler(Tree->shader))
            {
                for (i = 0; i < Tree->shader->outputCount; i++)
                {
                    if (Tree->shader->outputs[i] != gcvNULL && Tree->shader->outputs[i]->nameLength == gcSL_COLOR)
                    {
                        index[0] = Tree->shader->outputs[i]->tempIndex;
                        outputCount++;
                        break;
                    }
                }
            }
            else
            {
                for (i = 0; i < Tree->shader->outputCount; i++)
                {
                    if (Tree->shader->outputs[i] != gcvNULL && Tree->shader->outputs[i]->nameLength !=  gcSL_DEPTH)
                    {
                        if (Tree->shader->outputs[i]->type != gcSHADER_FLOAT_X4 &&
                            Tree->shader->outputs[i]->type != gcSHADER_INTEGER_X4 &&
                            Tree->shader->outputs[i]->type != gcSHADER_UINT_X4)
                            continue;

                        gcmASSERT(Tree->shader->outputLocations[i] < gcdMAX_DRAW_BUFFERS);
                        index[Tree->shader->outputLocations[i]] = Tree->shader->outputs[i]->tempIndex;
                        outputCount++;
                    }
                }
            }

            for (i = 0; i < (gctSIZE_T)outputCount; i++)
            {
                gcLINKTREE_TEMP temp = gcvNULL;
                gcsLINKTREE_LIST_PTR define = gcvNULL;

                if (Hints->removeAlphaAssignment)
                    break;

                /* Skip unused location output. */
                if (index[i] == -1)
                    continue;

                temp = &Tree->tempArray[index[i]];
                gcmASSERT(temp != gcvNULL);

                define = temp->defined;

                while (define)
                {
                    gcSL_INSTRUCTION inst = gcvNULL;
                    inst = Tree->shader->code + define->index;

                    if (gcmSL_TARGET_GET(inst->temp, Enable) == gcSL_ENABLE_W)
                    {
                        gcmASSERT(Hints != gcvNULL);

                        Hints->removeAlphaAssignment = gcvTRUE;
                        break;
                    }

                    define = define->next;
                }
            }
        }

        /* Walk through all outputs. */
        if (CodeGen->haltiShader == gcvTRUE) {
          gctUINT32 regPsColorOut = 0;
          gctUINT32 regPsOutCntrl[3] = {0};

          gctINT curLocation;
          gctUINT curIndex = 0;
          for (curLocation = -1; curLocation < (gctINT)gcHWCaps.maxRenderTargetCount; ++curLocation)
          {
              gctINT temp, reg;
              /* get the output whose location is curLocation. if not found,
                 curLocation is not specified in shader (i.e., there is a hole) and
                 go to the next location. */
              for (i = 0; i < Tree->outputCount; ++i)
              {
                  gctBOOL isRecompileOut = Tree->shader->outputs[i]->nameLength > 0
                                        && Tree->shader->outputs[i]->name[0] == '#';
                  /* use driver passed caps to get the user visible RTs */
                  gctINT maxLoc = isRecompileOut ? (gctINT)gcHWCaps.maxRenderTargetCount : GetGLMaxDrawBuffers();

                  if (Tree->shader->outputs[i] == gcvNULL)
                  {
                      continue;
                  }

                  gcmASSERT(i < Tree->shader->outputLocationCount);

                  if (Tree->shader->outputLocations[i] >= maxLoc)
                  {
                     gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_COMPILER,
                                   "Fragment output %d mapped to location greater than %d, location = %d",
                                   gcHWCaps.maxRenderTargetCount, i,
                                   Tree->shader->outputLocations[i]);

                     status = gcvSTATUS_TOO_MANY_OUTPUT;
                     gcmONERROR(gcvSTATUS_TOO_MANY_OUTPUT);
                  }

                  if (Tree->shader->outputs[i]->location == curLocation)
                  {
                      break;
                  }
              }

              if (i < Tree->outputCount)
              {
                  /* Extract internal name for output. */
                  switch (Tree->shader->outputs[i]->nameLength)
                  {
                  case gcSL_COLOR:
                      gcmASSERT(0);
                      status = gcvSTATUS_TOO_MANY_OUTPUT;
                      gcmONERROR(gcvSTATUS_TOO_MANY_OUTPUT);

                  case gcSL_DEPTH:
                      hasFragDepth = gcvTRUE;
                      break;

                  default:
                      break;
                  }

                  temp = Tree->outputArray[i].tempHolding;
                  if (_isHWRegisterAllocated(Tree->shader))
                  {
                      reg  = temp;
                  }
                  else
                  {
                      reg  = Tree->tempArray[temp].assigned;
                  }

                  if (reg == -1)
                  {
                      if (dumpCodeGen)
                      {
                          gcoOS_Print("Fragment Shader has no output, using r0.");
                      }
                      reg = 0;
                  }

                  gcmASSERT(reg != -1);
                  if (Tree->shader->outputs[i]->nameLength !=  gcSL_DEPTH &&
                      Tree->shader->outputs[i]->nameLength !=  gcSL_SUBSAMPLE_DEPTH)
                  {
                      if (Hints)
                      {
                          Hints->usedRTMask |= (1 << curIndex);
                      }

                      switch (curIndex) {
                      case 0:
                         regPsColorOut = ((((gctUINT32) (regPsColorOut)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (reg) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));
                         break;

                      case 1:
                         regPsColorOut = ((((gctUINT32) (regPsColorOut)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:8) - (0 ?
 13:8) + 1))))))) << (0 ?
 13:8))) | (((gctUINT32) ((gctUINT32) (reg) & ((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ? 13:8)));
                         break;

                      case 2:
                         regPsColorOut = ((((gctUINT32) (regPsColorOut)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:16) - (0 ?
 21:16) + 1))))))) << (0 ?
 21:16))) | (((gctUINT32) ((gctUINT32) (reg) & ((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ? 21:16)));
                         break;

                      case 3:
                         regPsColorOut = ((((gctUINT32) (regPsColorOut)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:24) - (0 ?
 29:24) + 1))))))) << (0 ?
 29:24))) | (((gctUINT32) ((gctUINT32) (reg) & ((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ? 29:24)));
                         break;

                      case 4:
                          regPsOutCntrl[0] = ((((gctUINT32) (regPsOutCntrl[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (reg) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));
                         break;

                      case 5:
                          regPsOutCntrl[0] = ((((gctUINT32) (regPsOutCntrl[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:8) - (0 ?
 13:8) + 1))))))) << (0 ?
 13:8))) | (((gctUINT32) ((gctUINT32) (reg) & ((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ? 13:8)));
                         break;

                      case 6:
                          regPsOutCntrl[0] = ((((gctUINT32) (regPsOutCntrl[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:16) - (0 ?
 21:16) + 1))))))) << (0 ?
 21:16))) | (((gctUINT32) ((gctUINT32) (reg) & ((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ? 21:16)));
                         break;

                      case 7:
                          regPsOutCntrl[0] = ((((gctUINT32) (regPsOutCntrl[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:24) - (0 ?
 29:24) + 1))))))) << (0 ?
 29:24))) | (((gctUINT32) ((gctUINT32) (reg) & ((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ? 29:24)));
                         break;

                      case 8:
                          regPsOutCntrl[1] = ((((gctUINT32) (regPsOutCntrl[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (reg) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));
                         break;

                      case 9:
                          regPsOutCntrl[1] = ((((gctUINT32) (regPsOutCntrl[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:8) - (0 ?
 13:8) + 1))))))) << (0 ?
 13:8))) | (((gctUINT32) ((gctUINT32) (reg) & ((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ? 13:8)));
                         break;

                      case 10:
                          regPsOutCntrl[1] = ((((gctUINT32) (regPsOutCntrl[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:16) - (0 ?
 21:16) + 1))))))) << (0 ?
 21:16))) | (((gctUINT32) ((gctUINT32) (reg) & ((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ? 21:16)));
                         break;

                      case 11:
                          regPsOutCntrl[1] = ((((gctUINT32) (regPsOutCntrl[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:24) - (0 ?
 29:24) + 1))))))) << (0 ?
 29:24))) | (((gctUINT32) ((gctUINT32) (reg) & ((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ? 29:24)));
                         break;

                      case 12:
                          regPsOutCntrl[2] = ((((gctUINT32) (regPsOutCntrl[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (reg) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));
                         break;

                      case 13:
                          regPsOutCntrl[2] = ((((gctUINT32) (regPsOutCntrl[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:8) - (0 ?
 13:8) + 1))))))) << (0 ?
 13:8))) | (((gctUINT32) ((gctUINT32) (reg) & ((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ? 13:8)));
                         break;

                      case 14:
                          regPsOutCntrl[2] = ((((gctUINT32) (regPsOutCntrl[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:16) - (0 ?
 21:16) + 1))))))) << (0 ?
 21:16))) | (((gctUINT32) ((gctUINT32) (reg) & ((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ? 21:16)));
                         break;

                      case 15:
                          regPsOutCntrl[2] = ((((gctUINT32) (regPsOutCntrl[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:24) - (0 ?
 29:24) + 1))))))) << (0 ?
 29:24))) | (((gctUINT32) ((gctUINT32) (reg) & ((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ? 29:24)));
                         break;

                      default:
                         gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_COMPILER,
                                       "Fragment output number %d is greater than 3", i);

                         status = gcvSTATUS_TOO_MANY_OUTPUT;
                         gcmONERROR(gcvSTATUS_TOO_MANY_OUTPUT);
                         break;
                      }
                      output2RTIndex[curIndex] = curLocation;
                      Tree->shader->outputs[i]->output2RTIndex = curLocation;
                      curIndex++;
                  }

                  gcCGUpdateMaxRegister(CodeGen, (gctUINT)reg, Tree);
                  if (dumpCodeGen)
                  {
                        gctCHAR nameBuffer[256] = {'\0'};
                        gctINT  len =
                            gcSL_GetName(Tree->shader->outputs[i]->nameLength,
                                            Tree->shader->outputs[i]->name,
                                            nameBuffer,
                                            sizeof(nameBuffer));
                        if (len != 0)
                            gcoOS_Print("%s: output %s ==> r%d", shaderTypeStr, nameBuffer, reg);
                   }
              }
          }

          gcmONERROR(_SetState(CodeGen,
                               0x0401,
                               regPsColorOut));
          if (Hints)
          {
            Hints->psOutCntl0to3 = regPsColorOut;
          }
          if (regPsOutCntrl[0] != 0)
          {
              gcmONERROR(_SetState(CodeGen,
                                   0x040B,
                                   regPsOutCntrl[0]));
              if (Hints)
              {
                  Hints->psOutCntl4to7 = regPsOutCntrl[0];
              }
          }
          if (regPsOutCntrl[1] != 0)
          {
              gcmONERROR(_SetState(CodeGen,
                                   0x0432,
                                   regPsOutCntrl[1]));
              if (Hints)
              {
                  Hints->psOutCntl8to11 = regPsOutCntrl[1];
              }
          }
          if (regPsOutCntrl[2] != 0)
          {
              gcmONERROR(_SetState(CodeGen,
                                   0x0433,
                                   regPsOutCntrl[2]));
              if (Hints)
              {
                  Hints->psOutCntl12to15 = regPsOutCntrl[2];
              }
          }
        }
        else
        {
            gctUINT32 regPsColorOut = 0;

            for (i = 0; i < Tree->outputCount; ++i)
            {
                gctINT temp, reg;

                if (Tree->shader->outputs[i] == gcvNULL)
                {
                    continue;
                }

                /* Extract internal name for output. */
                switch (Tree->shader->outputs[i]->nameLength)
                {
                case gcSL_COLOR:
                    break;
                case gcSL_DEPTH:
                    hasFragDepth = gcvTRUE;
                    break;

                default:
                    continue;
                }

                temp = Tree->outputArray[i].tempHolding;
                if (_isHWRegisterAllocated(Tree->shader))
                {
                    reg = temp;
                }
                else
                {
                    reg  = Tree->tempArray[temp].assigned;
                }

                if (reg == -1)
                {
                    if (dumpCodeGen)
                    {
                        gcoOS_Print("Fragment Shader has no output, using r0.");
                    }
                    reg = 0;
                }

                output2RTIndex[0] = 0;

                gcmASSERT(reg != -1);
                if (Tree->shader->outputs[i]->nameLength == gcSL_COLOR)
                {
#if TEMP_SHADER_PATCH
                    if (Hints && Hints->pachedShaderIdentifier == gcvMACHINECODE_ANTUTU0)
                    {
                        reg = 1;
                    }
#endif
                    gcmASSERT(Tree->shader->outputs[i]->location < (gctINT)gcHWCaps.maxRenderTargetCount);

                    if (Hints)
                    {
                        Hints->usedRTMask |= (1 << Tree->shader->outputs[i]->location);
                    }

                    switch (Tree->shader->outputs[i]->location) {
                      case 0:
                          regPsColorOut = ((((gctUINT32) (regPsColorOut)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (reg) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));
                          break;

                      case 1:
                          regPsColorOut = ((((gctUINT32) (regPsColorOut)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:8) - (0 ?
 13:8) + 1))))))) << (0 ?
 13:8))) | (((gctUINT32) ((gctUINT32) (reg) & ((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ? 13:8)));
                          break;

                      case 2:
                          regPsColorOut = ((((gctUINT32) (regPsColorOut)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:16) - (0 ?
 21:16) + 1))))))) << (0 ?
 21:16))) | (((gctUINT32) ((gctUINT32) (reg) & ((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ? 21:16)));
                          break;

                      case 3:
                          regPsColorOut = ((((gctUINT32) (regPsColorOut)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:24) - (0 ?
 29:24) + 1))))))) << (0 ?
 29:24))) | (((gctUINT32) ((gctUINT32) (reg) & ((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ? 29:24)));
                          break;

                      default:
                          gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_COMPILER,
                              "Fragment output number %d is greater than 3", i);

                          status = gcvSTATUS_TOO_MANY_OUTPUT;
                          gcmONERROR(gcvSTATUS_TOO_MANY_OUTPUT);
                          break;
                    }

                    output2RTIndex[Tree->shader->outputs[i]->location] = Tree->shader->outputs[i]->location;
                    Tree->shader->outputs[i]->output2RTIndex = Tree->shader->outputs[i]->location;
                }

                gcCGUpdateMaxRegister(CodeGen, (gctUINT)reg, Tree);

                if (dumpCodeGen)
                {
                    gctCHAR nameBuffer[256] = {'\0'};
                    gctINT  len =
                        gcSL_GetName(Tree->shader->outputs[i]->nameLength,
                                     Tree->shader->outputs[i]->name,
                                     nameBuffer,
                                     sizeof(nameBuffer));
                    if (len != 0)
                        gcoOS_Print("%s: output %s ==> r%d", shaderTypeStr,nameBuffer, reg);
                }
            }

            gcmONERROR(_SetState(CodeGen,
                                 0x0401,
                                 regPsColorOut));
            if (Hints)
            {
                Hints->psOutCntl0to3 = regPsColorOut;
            }
        }

        for (i = 0; i < Tree->attributeCount; ++ i)
        {
            if (Tree->attributeArray[i].inUse)
            {
                gcATTRIBUTE attribute = Tree->shader->attributes[i];
                gctUINT32 components = 0, rows = 0;
                gctINT reg;

                gcTYPE_GetTypeInfo(attribute->type, &components, &rows, 0);
                rows *= attribute->arraySize;

                reg = attribute->inputIndex + rows - 1;
                gcCGUpdateMaxRegister(CodeGen, (gctUINT) reg, Tree);
            }
        }

        if (CodeGen->flags & gcvSHADER_TEXLD_W)
        {
            _adjustMaxTemp(Tree, CodeGen);
        }

        if (Hints != gcvNULL)
        {
            gcmASSERT(attributeCount <= CodeGen->maxVaryingVectors);
            Hints->elementCount   = attributeCount;
            Hints->shader2PaOutputCount = attributeCount + 1;

            Hints->psHasFragDepthOut = hasFragDepth;

            gcoOS_MemCopy((gctPOINTER)Hints->psOutput2RtIndex,
                          (gctPOINTER)output2RTIndex,
                          gcmSIZEOF(output2RTIndex));

            if (CodeGen->shaderType == gcSHADER_TYPE_FRAGMENT)
            {
                Hints->fsInputCount   = 1 + attributeCount;
                Hints->fsMaxTemp      = (CodeGen->subsampleDepthRegIncluded ? 0 : 1) + CodeGen->maxRegister;
                gcSHADER_GetEarlyFragTest(Tree->shader, &Hints->useEarlyFragmentTest);
            }
            else
            {
                /* Kernel shader does not have extra attribute as fragment shader. */
                /* However, fsInputCount cannot be 0. */
                Hints->fsInputCount   = attributeCount ? attributeCount : 1;
                Hints->fsMaxTemp      = CodeGen->maxRegister + 1;
                Hints->threadWalkerInPS = gcvTRUE;
            }


            if (dumpCodeGen)
            {
                gcoOS_Print("%s: elementCount=%u", shaderTypeStr, Hints->elementCount);
                gcoOS_Print("%s: fsInputCount=%u", shaderTypeStr, Hints->fsInputCount);
                if (attributeCount > 0)
                {
                    gcoOS_Print("%s: varying mapping:", shaderTypeStr);
                    /* print input attrubute register mapping */
                    for (i = 0; i < Tree->shader->attributeCount; ++i)
                    {
                        gcATTRIBUTE      attribute = Tree->shader->attributes[i];
                        gctCONST_STRING  name;
                        gctUINT32        components = 0, rows = 0;

                        if (!Tree->attributeArray[i].inUse)
                            continue;   /* skip unused attributes */

                        /* Determine rows and components. */
                        gcTYPE_GetTypeInfo(attribute->type, &components, &rows, 0);
                        rows *= attribute->arraySize;
                        gcATTRIBUTE_GetName(Tree->shader, attribute, gcvTRUE, gcvNULL, &name);
                        if (rows > 1)
                        {
                            gcoOS_Print("%s: Attribute(%d) %s ==> r%d - r%d", shaderTypeStr,
                                attribute->index, name,
                                attribute->inputIndex,
                                attribute->inputIndex + rows - 1);
                        }
                        else
                        {
                            gcoOS_Print("%s: Attribute(%d) %s ==> r%d", shaderTypeStr,
                                attribute->index, name, attribute->inputIndex);
                        }

                        i += rows - 1;
                    }
                }
                gcoOS_Print("%s: fsMaxTemp=%u", shaderTypeStr, Hints->fsMaxTemp);
            }
        }

        if (!gcHWCaps.hwFeatureFlags.newGPIPE)
        {
            /* Generate element type. */
            address = 0x0290;
            for (i = 0; i < Tree->attributeCount; ++i)
            {
                if (Tree->attributeArray[i].inUse
                &&  !gcmATTRIBUTE_packedAway(Tree->shader->attributes[i])
                )
                {
                    /* Determine texture usage. */
                    gctUINT32 texture = gcmATTRIBUTE_isTexture(Tree->shader->attributes[i])
                        ? 0xF
                        : 0x0;

                    /* AQPAClipFlatColorTex */
                    gcmONERROR(
                        _SetState(CodeGen,
                                  address++,
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (texture) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:4) - (0 ?
 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (texture) & ((gctUINT32) ((((1 ?
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ? 7:4))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:8) - (0 ?
 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ? 11:8)))));
                }
            }
        }
        if (CodeGen->shaderType == gcSHADER_TYPE_FRAGMENT)
        {
            gctUINT32 componentIndex = 0;
            gctUINT32 componentType[_MAX_VARYINGS * 4];
            gctUINT32 varyingIndex = 0;
            gctUINT32 varyingPacking[_MAX_VARYINGS] = {0};
            gctUINT32 interpolation[_MAX_VARYINGS * 4];
            gctUINT32 interpolationLoc[_MAX_VARYINGS * 4] = {0};

            gcmASSERT(CodeGen->maxVaryingVectors <= _MAX_VARYINGS);
            /* Generate the varying packing. */
            for (i = 0; i < Tree->attributeCount; ++i)
            {
                gcSHADER_TYPE type;
                gctBOOL isTexture, isZWTexture;
                gctBOOL isFog;
                gctBOOL isFlat[4];
                gctSIZE_T size;
                gctINT j;
                gctBOOL isPointCoord;

                if (!Tree->attributeArray[i].inUse
                ||  (Tree->shader->attributes[i]->nameLength == gcSL_POSITION)
                ||  (Tree->shader->attributes[i]->nameLength == gcSL_FRONT_FACING)
                ||  (Tree->shader->attributes[i]->nameLength == gcSL_HELPER_INVOCATION)
                )
                {
                    continue;
                }

                if (gcHWCaps.hwFeatureFlags.supportIntAttrib)
                {
                    type = Tree->shader->attributes[i]->type;
                }
                else
                {
                    type = gcSHADER_FLOAT_X4;
                }

                isTexture = gcmATTRIBUTE_isTexture(Tree->shader->attributes[i]);
                isZWTexture = gcmATTRIBUTE_isZWTexture(Tree->shader->attributes[i]);
                isFog = Tree->shader->attributes[i]->nameLength == gcSL_FOG_COORD;
                isPointCoord = Tree->shader->attributes[i]->nameLength == gcSL_POINT_COORD;

                for (j = 0; j < 4; j++)
                {
                    if (Tree->shader->attributes[i]->componentShadeMode[j] == gcSHADER_SHADER_FLAT)
                        isFlat[j] = gcvTRUE;
                    else
                        isFlat[j] = gcvFALSE;
                }

                components = rows = 0;
                gcTYPE_GetTypeInfo(Tree->shader->attributes[i]->type,
                                   &components, &rows, 0);
                rows *= Tree->shader->attributes[i]->arraySize;

                size      = gcmMAX(1, rows);

                if (isPointCoord)
                {
                    if (Hints)
                    {
                        Hints->pointCoordComponent = componentIndex;
                    }
                }

                while (size-- > 0)
                {
                    if(varyingIndex >= CodeGen->maxVaryingVectors)
                    {
                        gcmONERROR(gcvSTATUS_TOO_MANY_INPUT);
                    }

                    switch (type)
                    {
                    case gcSHADER_UINT_X1:
                    case gcSHADER_FLOAT_X1:
                        /* fall through */
                    case gcSHADER_BOOLEAN_X1:
                        /* fall through */
                    case gcSHADER_INTEGER_X1:
                        gcmASSERT(varyingIndex < CodeGen->maxVaryingVectors);
                        gcmASSERT(isZWTexture == gcvFALSE);
                        varyingPacking[varyingIndex++] = 1;

                        gcmASSERT(componentIndex < CodeGen->maxVaryingVectors * 4);
                        interpolation[componentIndex] = isFlat[0] ? 0x2 :
                                                                    0x0;
                        componentType[componentIndex++] = isTexture
                            ? 0x2
                            : (isFog
                                  ? 0x0
                                  : (isFlat[0]
                                        ? 0x1
                                        : 0x0));

                        break;

                    case gcSHADER_UINT_X2:
                    case gcSHADER_FLOAT_X2:
                        /* fall through */
                    case gcSHADER_BOOLEAN_X2:
                        /* fall through */
                    case gcSHADER_INTEGER_X2:
                        /* fall through */
                    case gcSHADER_FLOAT_2X2:
                    case gcSHADER_FLOAT_3X2:
                    case gcSHADER_FLOAT_4X2:
                        gcmASSERT(varyingIndex < CodeGen->maxVaryingVectors);
                        gcmASSERT(isZWTexture == gcvFALSE);
                        varyingPacking[varyingIndex++] = 2;

                        gcmASSERT(componentIndex < CodeGen->maxVaryingVectors * 4 - 1);

                        interpolation[componentIndex] = isFlat[0] ? 0x2 :
                                                                    0x0;
                        componentType[componentIndex++] = isTexture
                            ? 0x2
                            : (isFlat[0]
                                  ? 0x1
                                  : 0x0);


                        interpolation[componentIndex] = isFlat[1] ? 0x2 :
                                                                    0x0;
                        componentType[componentIndex++] = isTexture
                            ? 0x3
                            : (isFlat[1]
                                  ? 0x1
                                  : 0x0);
                        break;

                    case gcSHADER_UINT_X3:
                    case gcSHADER_FLOAT_X3:
                        /* fall through */
                    case gcSHADER_BOOLEAN_X3:
                        /* fall through */
                    case gcSHADER_INTEGER_X3:
                        /* fall through */
                    case gcSHADER_FLOAT_2X3:
                    case gcSHADER_FLOAT_3X3:
                    case gcSHADER_FLOAT_4X3:
                        gcmASSERT(varyingIndex < CodeGen->maxVaryingVectors);
                        gcmASSERT(isZWTexture == gcvFALSE);
                        varyingPacking[varyingIndex++] = 3;

                        gcmASSERT(componentIndex < CodeGen->maxVaryingVectors * 4 - 2);

                        interpolation[componentIndex] = isFlat[0] ? 0x2 :
                                                                    0x0;
                        componentType[componentIndex++] = isTexture
                            ? 0x2
                            : (isFlat[0]
                                  ? 0x1
                                  : 0x0);

                        interpolation[componentIndex] = isFlat[1] ? 0x2 :
                                                                    0x0;
                        componentType[componentIndex++] = isTexture
                            ? 0x3
                            : (isFlat[1]
                                  ? 0x1
                                  : 0x0);

                        interpolation[componentIndex] = isFlat[2] ? 0x2 :
                                                                    0x0;
                        componentType[componentIndex++] = isFlat[2]
                            ? 0x1
                            : 0x0;
                        break;

                    case gcSHADER_UINT_X4:
                    case gcSHADER_FLOAT_X4:
                        /* fall through */
                    case gcSHADER_BOOLEAN_X4:
                        /* fall through */
                    case gcSHADER_INTEGER_X4:
                        /* fall through */
                    case gcSHADER_FLOAT_2X4:
                    case gcSHADER_FLOAT_3X4:
                    case gcSHADER_FLOAT_4X4:
                        gcmASSERT(varyingIndex < CodeGen->maxVaryingVectors * 4);
                        varyingPacking[varyingIndex++] = 4;

                        gcmASSERT(componentIndex < CodeGen->maxVaryingVectors * 4 - 3);

                        interpolation[componentIndex] = isFlat[0] ? 0x2 :
                                                                    0x0;
                        componentType[componentIndex++] = isTexture
                            ? 0x2
                            : (isFlat[0]
                                  ? 0x1
                                  : 0x0);

                        interpolation[componentIndex] = isFlat[1] ? 0x2 :
                                                                    0x0;
                        componentType[componentIndex++] = isTexture
                            ? 0x3
                            : (isFlat[1]
                                  ? 0x1
                                  : 0x0);

                        interpolation[componentIndex] = isFlat[2] ? 0x2 :
                                                                    0x0;
                        componentType[componentIndex++] = isZWTexture
                            ? 0x2
                            : (isFlat[2]
                                  ? 0x1
                                  : 0x0);

                        interpolation[componentIndex] = isFlat[3] ? 0x2 :
                                                                    0x0;
                        componentType[componentIndex++] = isZWTexture
                            ? 0x3
                            : (isFlat[3]
                                  ? 0x1
                                  : 0x0);
                        break;

                    default:
                        gcmFATAL("Huh? Some kind of weird attribute (%d) here?",
                                 Tree->shader->attributes[i]->type);
                        gcmONERROR(gcvSTATUS_TOO_MANY_INPUT);
                    }
                }
            }

            gcmASSERT(varyingIndex == attributeCount);

            while (varyingIndex < CodeGen->maxVaryingVectors)
            {
                varyingPacking[varyingIndex++] = 0;
            }

            if (Hints != gcvNULL)
            {
                Hints->componentCount = gcmALIGN(componentIndex, 2);

                if (dumpCodeGen)
                {
                    gcoOS_Print("%s: componentCount=%u", shaderTypeStr,
                                  Hints->componentCount);
                }
            }

            while (componentIndex < CodeGen->maxVaryingVectors * 4)
            {
                interpolation[componentIndex] = 0x0;
                componentType[componentIndex++] =
                    0x0;
            }

            if (gcHWCaps.hwFeatureFlags.newGPIPE)
            {
                for (i = 0; i < 4; i++)
                {
                   gcmONERROR(_SetState(CodeGen,
                                        0x02A4 + i,
                                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (varyingPacking[i * 8]) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:4) - (0 ?
 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) ((gctUINT32) (varyingPacking[i * 8 + 1]) & ((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ? 6:4)))
                                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:8) - (0 ?
 10:8) + 1))))))) << (0 ?
 10:8))) | (((gctUINT32) ((gctUINT32) (varyingPacking[i * 8 + 2]) & ((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ? 10:8)))
                                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:12) - (0 ?
 14:12) + 1))))))) << (0 ?
 14:12))) | (((gctUINT32) ((gctUINT32) (varyingPacking[i * 8 + 3]) & ((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ? 14:12)))
                                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) ((gctUINT32) (varyingPacking[i * 8 + 4]) & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) ((gctUINT32) (varyingPacking[i * 8 + 5]) & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) ((gctUINT32) (varyingPacking[i * 8 + 6]) & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (varyingPacking[i * 8 + 7]) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))));
                   gcmONERROR(_SetState(CodeGen,
                                        0x0420 + i,
                                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (varyingPacking[i * 8]) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:4) - (0 ?
 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) ((gctUINT32) (varyingPacking[i * 8 + 1]) & ((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ? 6:4)))
                                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:8) - (0 ?
 10:8) + 1))))))) << (0 ?
 10:8))) | (((gctUINT32) ((gctUINT32) (varyingPacking[i * 8 + 2]) & ((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ? 10:8)))
                                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:12) - (0 ?
 14:12) + 1))))))) << (0 ?
 14:12))) | (((gctUINT32) ((gctUINT32) (varyingPacking[i * 8 + 3]) & ((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ? 14:12)))
                                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) ((gctUINT32) (varyingPacking[i * 8 + 4]) & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) ((gctUINT32) (varyingPacking[i * 8 + 5]) & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) ((gctUINT32) (varyingPacking[i * 8 + 6]) & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (varyingPacking[i * 8 + 7]) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))));
                }
            }
            else
            {

#if TEMP_SHADER_PATCH
                if (Hints && (Hints->pachedShaderIdentifier == gcvMACHINECODE_ANTUTU0))
                {
                    gcmONERROR(_SetState(CodeGen, 0x0E08, 0x00000042));
                }
                else
                {
#endif
                gcmONERROR(
                    _SetState(CodeGen,
                              0x0E08,
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (varyingPacking[0]) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:4) - (0 ?
 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) ((gctUINT32) (varyingPacking[1]) & ((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ? 6:4))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:8) - (0 ?
 10:8) + 1))))))) << (0 ?
 10:8))) | (((gctUINT32) ((gctUINT32) (varyingPacking[2]) & ((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ? 10:8))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:12) - (0 ?
 14:12) + 1))))))) << (0 ?
 14:12))) | (((gctUINT32) ((gctUINT32) (varyingPacking[3]) & ((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ? 14:12))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) ((gctUINT32) (varyingPacking[4]) & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) ((gctUINT32) (varyingPacking[5]) & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) ((gctUINT32) (varyingPacking[6]) & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (varyingPacking[7]) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))));
#if TEMP_SHADER_PATCH
                }
#endif
                if (CodeGen->maxVaryingVectors > 12)
                {
                    gcmONERROR(
                        _SetState(CodeGen,
                                  0x0E0D,
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (varyingPacking[8]) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:4) - (0 ?
 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) ((gctUINT32) (varyingPacking[9]) & ((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ? 6:4))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:8) - (0 ?
 10:8) + 1))))))) << (0 ?
 10:8))) | (((gctUINT32) ((gctUINT32) (varyingPacking[10]) & ((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ? 10:8))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:12) - (0 ?
 14:12) + 1))))))) << (0 ?
 14:12))) | (((gctUINT32) ((gctUINT32) (varyingPacking[11]) & ((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ? 14:12))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) ((gctUINT32) (varyingPacking[12]) & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) ((gctUINT32) (varyingPacking[13]) & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) ((gctUINT32) (varyingPacking[14]) & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (varyingPacking[15]) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))));
                }
                else if (CodeGen->maxVaryingVectors > 8)
                {
                    /* on some chips, it would consume two varying bufers for position,
                    ** so the max varying would be 11, and we need to modify the varying packing info.
                    */
                    gcmASSERT(CodeGen->maxVaryingVectors == 12 || CodeGen->maxVaryingVectors == 11);

                    if (CodeGen->maxVaryingVectors == 11)
                    {
                        gcmONERROR(
                            _SetState(CodeGen,
                                      0x0E0D,
                                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (varyingPacking[8]) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))) |
                                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:4) - (0 ?
 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) ((gctUINT32) (varyingPacking[9]) & ((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ? 6:4))) |
                                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:8) - (0 ?
 10:8) + 1))))))) << (0 ?
 10:8))) | (((gctUINT32) ((gctUINT32) (varyingPacking[10]) & ((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ? 10:8)))));
                    }
                    else
                    {
                        gcmONERROR(

                            _SetState(CodeGen,
                                      0x0E0D,
                                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (varyingPacking[8]) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))) |
                                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:4) - (0 ?
 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) ((gctUINT32) (varyingPacking[9]) & ((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ? 6:4))) |
                                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:8) - (0 ?
 10:8) + 1))))))) << (0 ?
 10:8))) | (((gctUINT32) ((gctUINT32) (varyingPacking[10]) & ((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ? 10:8))) |
                                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:12) - (0 ?
 14:12) + 1))))))) << (0 ?
 14:12))) | (((gctUINT32) ((gctUINT32) (varyingPacking[11]) & ((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ? 14:12)))));
                    }
                }
            }
            if (dumpCodeGen)
            {
                if (CodeGen->maxVaryingVectors > 12)
                {
                    gcoOS_Print("%s: packing=%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u", shaderTypeStr,
                                  varyingPacking[0],
                                  varyingPacking[1],
                                  varyingPacking[2],
                                  varyingPacking[3],
                                  varyingPacking[4],
                                  varyingPacking[5],
                                  varyingPacking[6],
                                  varyingPacking[7],
                                  varyingPacking[8],
                                  varyingPacking[9],
                                  varyingPacking[10],
                                  varyingPacking[11],
                                  varyingPacking[12],
                                  varyingPacking[13],
                                  varyingPacking[14],
                                  varyingPacking[15]);
                }
                else if (CodeGen->maxVaryingVectors > 8)
                {
                    gcoOS_Print("%s: packing=%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u", shaderTypeStr,
                                  varyingPacking[0],
                                  varyingPacking[1],
                                  varyingPacking[2],
                                  varyingPacking[3],
                                  varyingPacking[4],
                                  varyingPacking[5],
                                  varyingPacking[6],
                                  varyingPacking[7],
                                  varyingPacking[8],
                                  varyingPacking[9],
                                  varyingPacking[10],
                                  varyingPacking[11]);
                }
                else
                {
                    gcoOS_Print("%s: packing=%u,%u,%u,%u,%u,%u,%u,%u", shaderTypeStr,
                                  varyingPacking[0],
                                  varyingPacking[1],
                                  varyingPacking[2],
                                  varyingPacking[3],
                                  varyingPacking[4],
                                  varyingPacking[5],
                                  varyingPacking[6],
                                  varyingPacking[7]);
                }
            }

            if (gcHWCaps.hwFeatureFlags.newGPIPE)
            {
                for (i = 0; i < 16; i++)
                {
                    gcmONERROR(_SetState(CodeGen,
                                         0x0E30 + i,
                                         ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (interpolation[i * 8]) & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))
                                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:2) - (0 ?
 3:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:2) - (0 ?
 3:2) + 1))))))) << (0 ?
 3:2))) | (((gctUINT32) ((gctUINT32) (interpolationLoc[i * 8]) & ((gctUINT32) ((((1 ?
 3:2) - (0 ?
 3:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:2) - (0 ? 3:2) + 1))))))) << (0 ? 3:2)))
                                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (interpolation[i * 8 + 1]) & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)))
                                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:6) - (0 ?
 7:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:6) - (0 ?
 7:6) + 1))))))) << (0 ?
 7:6))) | (((gctUINT32) ((gctUINT32) (interpolationLoc[i * 8 + 1]) & ((gctUINT32) ((((1 ?
 7:6) - (0 ?
 7:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ? 7:6)))
                                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:8) - (0 ?
 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (interpolation[i * 8 + 2]) & ((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8)))
                                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:10) - (0 ?
 11:10) + 1))))))) << (0 ?
 11:10))) | (((gctUINT32) ((gctUINT32) (interpolationLoc[i * 8 + 2]) & ((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:10) - (0 ? 11:10) + 1))))))) << (0 ? 11:10)))

                                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:12) - (0 ?
 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) ((gctUINT32) (interpolation[i * 8 + 3]) & ((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ? 13:12)))
                                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) ((gctUINT32) (interpolationLoc[i * 8 + 3]) & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:16) - (0 ?
 17:16) + 1))))))) << (0 ?
 17:16))) | (((gctUINT32) ((gctUINT32) (interpolation[i * 8 + 4]) & ((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16)))
                                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:18) - (0 ?
 19:18) + 1))))))) << (0 ?
 19:18))) | (((gctUINT32) ((gctUINT32) (interpolationLoc[i * 8 + 4]) & ((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:18) - (0 ? 19:18) + 1))))))) << (0 ? 19:18)))
                                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ?
 21:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:20) - (0 ?
 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) ((gctUINT32) (interpolation[i * 8 + 5]) & ((gctUINT32) ((((1 ?
 21:20) - (0 ?
 21:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)))
                                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:22) - (0 ?
 23:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:22) - (0 ?
 23:22) + 1))))))) << (0 ?
 23:22))) | (((gctUINT32) ((gctUINT32) (interpolationLoc[i * 8 + 5]) & ((gctUINT32) ((((1 ?
 23:22) - (0 ?
 23:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:22) - (0 ? 23:22) + 1))))))) << (0 ? 23:22)))
                                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:24) - (0 ?
 25:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:24) - (0 ?
 25:24) + 1))))))) << (0 ?
 25:24))) | (((gctUINT32) ((gctUINT32) (interpolation[i * 8 + 6]) & ((gctUINT32) ((((1 ?
 25:24) - (0 ?
 25:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:24) - (0 ? 25:24) + 1))))))) << (0 ? 25:24)))
                                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:26) - (0 ?
 27:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:26) - (0 ?
 27:26) + 1))))))) << (0 ?
 27:26))) | (((gctUINT32) ((gctUINT32) (interpolationLoc[i * 8 + 6]) & ((gctUINT32) ((((1 ?
 27:26) - (0 ?
 27:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:26) - (0 ? 27:26) + 1))))))) << (0 ? 27:26)))
                                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:28) - (0 ?
 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) ((gctUINT32) (interpolation[i * 8 + 7]) & ((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ? 29:28)))
                                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:30) - (0 ?
 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (interpolationLoc[i * 8 + 7]) & ((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)))));
                }
            }
            else
            {
                gcmONERROR(
                    _SetState(CodeGen,
                              0x0E0A,
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (componentType[0]) & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:2) - (0 ?
 3:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:2) - (0 ?
 3:2) + 1))))))) << (0 ?
 3:2))) | (((gctUINT32) ((gctUINT32) (componentType[1]) & ((gctUINT32) ((((1 ?
 3:2) - (0 ?
 3:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:2) - (0 ? 3:2) + 1))))))) << (0 ? 3:2))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (componentType[2]) & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:6) - (0 ?
 7:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:6) - (0 ?
 7:6) + 1))))))) << (0 ?
 7:6))) | (((gctUINT32) ((gctUINT32) (componentType[3]) & ((gctUINT32) ((((1 ?
 7:6) - (0 ?
 7:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ? 7:6))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:8) - (0 ?
 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (componentType[4]) & ((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:10) - (0 ?
 11:10) + 1))))))) << (0 ?
 11:10))) | (((gctUINT32) ((gctUINT32) (componentType[5]) & ((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:10) - (0 ? 11:10) + 1))))))) << (0 ? 11:10))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:12) - (0 ?
 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) ((gctUINT32) (componentType[6]) & ((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ? 13:12))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) ((gctUINT32) (componentType[7]) & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:16) - (0 ?
 17:16) + 1))))))) << (0 ?
 17:16))) | (((gctUINT32) ((gctUINT32) (componentType[8]) & ((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:18) - (0 ?
 19:18) + 1))))))) << (0 ?
 19:18))) | (((gctUINT32) ((gctUINT32) (componentType[9]) & ((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:18) - (0 ? 19:18) + 1))))))) << (0 ? 19:18))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ?
 21:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:20) - (0 ?
 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) ((gctUINT32) (componentType[10]) & ((gctUINT32) ((((1 ?
 21:20) - (0 ?
 21:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:22) - (0 ?
 23:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:22) - (0 ?
 23:22) + 1))))))) << (0 ?
 23:22))) | (((gctUINT32) ((gctUINT32) (componentType[11]) & ((gctUINT32) ((((1 ?
 23:22) - (0 ?
 23:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:22) - (0 ? 23:22) + 1))))))) << (0 ? 23:22))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:24) - (0 ?
 25:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:24) - (0 ?
 25:24) + 1))))))) << (0 ?
 25:24))) | (((gctUINT32) ((gctUINT32) (componentType[12]) & ((gctUINT32) ((((1 ?
 25:24) - (0 ?
 25:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:24) - (0 ? 25:24) + 1))))))) << (0 ? 25:24))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:26) - (0 ?
 27:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:26) - (0 ?
 27:26) + 1))))))) << (0 ?
 27:26))) | (((gctUINT32) ((gctUINT32) (componentType[13]) & ((gctUINT32) ((((1 ?
 27:26) - (0 ?
 27:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:26) - (0 ? 27:26) + 1))))))) << (0 ? 27:26))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:28) - (0 ?
 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) ((gctUINT32) (componentType[14]) & ((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ? 29:28))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:30) - (0 ?
 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (componentType[15]) & ((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)))));

                gcmONERROR(
                    _SetState(CodeGen,
                              0x0E0B,
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (componentType[16]) & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:2) - (0 ?
 3:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:2) - (0 ?
 3:2) + 1))))))) << (0 ?
 3:2))) | (((gctUINT32) ((gctUINT32) (componentType[17]) & ((gctUINT32) ((((1 ?
 3:2) - (0 ?
 3:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:2) - (0 ? 3:2) + 1))))))) << (0 ? 3:2))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (componentType[18]) & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:6) - (0 ?
 7:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:6) - (0 ?
 7:6) + 1))))))) << (0 ?
 7:6))) | (((gctUINT32) ((gctUINT32) (componentType[19]) & ((gctUINT32) ((((1 ?
 7:6) - (0 ?
 7:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ? 7:6))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:8) - (0 ?
 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (componentType[20]) & ((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:10) - (0 ?
 11:10) + 1))))))) << (0 ?
 11:10))) | (((gctUINT32) ((gctUINT32) (componentType[21]) & ((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:10) - (0 ? 11:10) + 1))))))) << (0 ? 11:10))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:12) - (0 ?
 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) ((gctUINT32) (componentType[22]) & ((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ? 13:12))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) ((gctUINT32) (componentType[23]) & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:16) - (0 ?
 17:16) + 1))))))) << (0 ?
 17:16))) | (((gctUINT32) ((gctUINT32) (componentType[24]) & ((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:18) - (0 ?
 19:18) + 1))))))) << (0 ?
 19:18))) | (((gctUINT32) ((gctUINT32) (componentType[25]) & ((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:18) - (0 ? 19:18) + 1))))))) << (0 ? 19:18))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ?
 21:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:20) - (0 ?
 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) ((gctUINT32) (componentType[26]) & ((gctUINT32) ((((1 ?
 21:20) - (0 ?
 21:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:22) - (0 ?
 23:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:22) - (0 ?
 23:22) + 1))))))) << (0 ?
 23:22))) | (((gctUINT32) ((gctUINT32) (componentType[27]) & ((gctUINT32) ((((1 ?
 23:22) - (0 ?
 23:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:22) - (0 ? 23:22) + 1))))))) << (0 ? 23:22))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:24) - (0 ?
 25:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:24) - (0 ?
 25:24) + 1))))))) << (0 ?
 25:24))) | (((gctUINT32) ((gctUINT32) (componentType[28]) & ((gctUINT32) ((((1 ?
 25:24) - (0 ?
 25:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:24) - (0 ? 25:24) + 1))))))) << (0 ? 25:24))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:26) - (0 ?
 27:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:26) - (0 ?
 27:26) + 1))))))) << (0 ?
 27:26))) | (((gctUINT32) ((gctUINT32) (componentType[29]) & ((gctUINT32) ((((1 ?
 27:26) - (0 ?
 27:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:26) - (0 ? 27:26) + 1))))))) << (0 ? 27:26))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:28) - (0 ?
 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) ((gctUINT32) (componentType[30]) & ((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ? 29:28))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:30) - (0 ?
 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (componentType[31]) & ((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)))));

                if (CodeGen->maxVaryingVectors > 8)
                {
                    gcmONERROR(
                        _SetState(CodeGen,
                                  0x0E0E,
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (componentType[32]) & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:2) - (0 ?
 3:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:2) - (0 ?
 3:2) + 1))))))) << (0 ?
 3:2))) | (((gctUINT32) ((gctUINT32) (componentType[33]) & ((gctUINT32) ((((1 ?
 3:2) - (0 ?
 3:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:2) - (0 ? 3:2) + 1))))))) << (0 ? 3:2))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (componentType[34]) & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:6) - (0 ?
 7:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:6) - (0 ?
 7:6) + 1))))))) << (0 ?
 7:6))) | (((gctUINT32) ((gctUINT32) (componentType[35]) & ((gctUINT32) ((((1 ?
 7:6) - (0 ?
 7:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ? 7:6))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:8) - (0 ?
 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (componentType[36]) & ((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:10) - (0 ?
 11:10) + 1))))))) << (0 ?
 11:10))) | (((gctUINT32) ((gctUINT32) (componentType[37]) & ((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:10) - (0 ? 11:10) + 1))))))) << (0 ? 11:10))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:12) - (0 ?
 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) ((gctUINT32) (componentType[38]) & ((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ? 13:12))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) ((gctUINT32) (componentType[39]) & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:16) - (0 ?
 17:16) + 1))))))) << (0 ?
 17:16))) | (((gctUINT32) ((gctUINT32) (componentType[40]) & ((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:18) - (0 ?
 19:18) + 1))))))) << (0 ?
 19:18))) | (((gctUINT32) ((gctUINT32) (componentType[41]) & ((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:18) - (0 ? 19:18) + 1))))))) << (0 ? 19:18))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ?
 21:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:20) - (0 ?
 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) ((gctUINT32) (componentType[42]) & ((gctUINT32) ((((1 ?
 21:20) - (0 ?
 21:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:22) - (0 ?
 23:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:22) - (0 ?
 23:22) + 1))))))) << (0 ?
 23:22))) | (((gctUINT32) ((gctUINT32) (componentType[43]) & ((gctUINT32) ((((1 ?
 23:22) - (0 ?
 23:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:22) - (0 ? 23:22) + 1))))))) << (0 ? 23:22))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:24) - (0 ?
 25:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:24) - (0 ?
 25:24) + 1))))))) << (0 ?
 25:24))) | (((gctUINT32) ((gctUINT32) (componentType[44]) & ((gctUINT32) ((((1 ?
 25:24) - (0 ?
 25:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:24) - (0 ? 25:24) + 1))))))) << (0 ? 25:24))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:26) - (0 ?
 27:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:26) - (0 ?
 27:26) + 1))))))) << (0 ?
 27:26))) | (((gctUINT32) ((gctUINT32) (componentType[45]) & ((gctUINT32) ((((1 ?
 27:26) - (0 ?
 27:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:26) - (0 ? 27:26) + 1))))))) << (0 ? 27:26))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:28) - (0 ?
 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) ((gctUINT32) (componentType[46]) & ((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ? 29:28))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:30) - (0 ?
 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (componentType[47]) & ((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)))));
                }
                if (CodeGen->maxVaryingVectors > 12)
                {
                    gcmONERROR(
                        _SetState(CodeGen,
                                  0x0E15,
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (componentType[48]) & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:2) - (0 ?
 3:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:2) - (0 ?
 3:2) + 1))))))) << (0 ?
 3:2))) | (((gctUINT32) ((gctUINT32) (componentType[49]) & ((gctUINT32) ((((1 ?
 3:2) - (0 ?
 3:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:2) - (0 ? 3:2) + 1))))))) << (0 ? 3:2))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (componentType[50]) & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:6) - (0 ?
 7:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:6) - (0 ?
 7:6) + 1))))))) << (0 ?
 7:6))) | (((gctUINT32) ((gctUINT32) (componentType[51]) & ((gctUINT32) ((((1 ?
 7:6) - (0 ?
 7:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ? 7:6))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:8) - (0 ?
 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (componentType[52]) & ((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:10) - (0 ?
 11:10) + 1))))))) << (0 ?
 11:10))) | (((gctUINT32) ((gctUINT32) (componentType[53]) & ((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:10) - (0 ? 11:10) + 1))))))) << (0 ? 11:10))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:12) - (0 ?
 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) ((gctUINT32) (componentType[54]) & ((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ? 13:12))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) ((gctUINT32) (componentType[55]) & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:16) - (0 ?
 17:16) + 1))))))) << (0 ?
 17:16))) | (((gctUINT32) ((gctUINT32) (componentType[56]) & ((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:18) - (0 ?
 19:18) + 1))))))) << (0 ?
 19:18))) | (((gctUINT32) ((gctUINT32) (componentType[57]) & ((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:18) - (0 ? 19:18) + 1))))))) << (0 ? 19:18))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ?
 21:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:20) - (0 ?
 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) ((gctUINT32) (componentType[58]) & ((gctUINT32) ((((1 ?
 21:20) - (0 ?
 21:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:22) - (0 ?
 23:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:22) - (0 ?
 23:22) + 1))))))) << (0 ?
 23:22))) | (((gctUINT32) ((gctUINT32) (componentType[59]) & ((gctUINT32) ((((1 ?
 23:22) - (0 ?
 23:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:22) - (0 ? 23:22) + 1))))))) << (0 ? 23:22))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:24) - (0 ?
 25:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:24) - (0 ?
 25:24) + 1))))))) << (0 ?
 25:24))) | (((gctUINT32) ((gctUINT32) (componentType[60]) & ((gctUINT32) ((((1 ?
 25:24) - (0 ?
 25:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:24) - (0 ? 25:24) + 1))))))) << (0 ? 25:24))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:26) - (0 ?
 27:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:26) - (0 ?
 27:26) + 1))))))) << (0 ?
 27:26))) | (((gctUINT32) ((gctUINT32) (componentType[61]) & ((gctUINT32) ((((1 ?
 27:26) - (0 ?
 27:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:26) - (0 ? 27:26) + 1))))))) << (0 ? 27:26))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:28) - (0 ?
 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) ((gctUINT32) (componentType[62]) & ((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ? 29:28))) |
                                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:30) - (0 ?
 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (componentType[63]) & ((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)))));
                }
            }
        }
        else if (gcHWCaps.hwFeatureFlags.hasThreadWalkerInPS)
        {
            gctUINT32 varyingPacking[2] = {0, 0};

            if (Hints != gcvNULL) {

                switch (Hints->elementCount)
                {
                case 3:
                    varyingPacking[1] = 2;
                    /*fall through*/
                case 2:
                    varyingPacking[0] = 2;
                }

                Hints->componentCount = gcmALIGN(varyingPacking[0] + varyingPacking[1], 2);
            }

            if (gcHWCaps.hwFeatureFlags.newGPIPE)
            {
                gcmONERROR(
                    _SetState(CodeGen,
                              0x02A4,
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (varyingPacking[0]) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:4) - (0 ?
 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) ((gctUINT32) (varyingPacking[1]) & ((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ? 6:4)))));

                gcmONERROR(
                    _SetState(CodeGen,
                              0x0420,
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (varyingPacking[0]) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:4) - (0 ?
 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) ((gctUINT32) (varyingPacking[1]) & ((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ? 6:4)))));

            }
            else
            {
                gcmONERROR(
                    _SetState(CodeGen,
                              0x0E08,
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (varyingPacking[0]) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))) |
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:4) - (0 ?
 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) ((gctUINT32) (varyingPacking[1]) & ((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ? 6:4)))));

                gcmONERROR(
                    _SetState(CodeGen,
                              0x0E0D,
                              0));
            }
        }

        if (CodeGen->isDual16Shader ||
            (hasShaderMediumP &&
             (precision == gcSHADER_PRECISION_MEDIUM) && hasPrecisionSetting)) {
            isMediumpPS = gcvTRUE;
            CodeGen->psInputControlHighpPosition = (hasFragDepth || CodeGen->usePosition)
                                                        ? 0x1
                                                        : 0x0;
        }
        else {
            isMediumpPS = gcvFALSE;
            CodeGen->psInputControlHighpPosition = 0x0;
        }

        /* Disable dual16 for opencl */
        if (CodeGen->clShader)
        {
            isMediumpPS = gcvFALSE;
        }

        if (CodeGen->hasHalti5)
        {
            shaderConfigData =
                            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:2) - (0 ?
 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) ((gctUINT32) ((isMediumpPS ?
 0x1 : 0x0)) & ((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2)))
                          /* Need to keep existing RTNE setting. */
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) ((gctUINT32) ((gcHWCaps.hwFeatureFlags.rtneRoundingEnabled ?
 0x1 : 0x0)) & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)));

        }
        else
        {
            shaderConfigData =
                            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:29) - (0 ?
 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) ((gctUINT32) ((isMediumpPS ?
 0x1 : 0x0)) & ((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29)))
                          /* Need to keep existing RTNE setting. */
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) ((gcHWCaps.hwFeatureFlags.rtneRoundingEnabled ?
 0x1 : 0x0)) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)));

        }

        if (Hints != gcvNULL)
        {
            if ((Hints->memoryAccessFlags[gcvSHADER_MACHINE_LEVEL][VSC_SHADER_STAGE_PS] & gceMA_FLAG_ATOMIC) &&
                gcHWCaps.hwFeatureFlags.robustAtomic)
            {
                shaderConfigData |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)));
            }
            shaderConfigData |= Hints->shaderConfigData;
            Hints->shaderConfigData = shaderConfigData;
        }

        if (CodeGen->hasHalti5)
        {
            gcmONERROR(
                _SetState(CodeGen,
                          0x5580,
                          shaderConfigData
                          ));
        }
        else
        {
            gcmONERROR(
                _SetState(CodeGen,
                          0x0218,
                          shaderConfigData
                          ));
        }

        if (CodeGen->useICache)
        {
            /* Set code address. */
            instrBase         = (gctUINT32)(gctUINTPTR_T) StateBuffer;
            maxNumInstrStates = gcHWCaps.maxHwNativeTotalInstCount << 2;

            if (Hints != gcvNULL)
            {
                Hints->unifiedStatus.useIcache = gcvTRUE;
            }
        }
        else
        {
            gctUINT psOffset = 0;

            if (! (CodeGen->clShader || CodeGen->computeShader))
            {
                 /* Use bottom part of instruction memory for PS. */
                if (CodeGen->instCount > 0)
                {
                    psOffset = gcHWCaps.maxHwNativeTotalInstCount - CodeGen->instCount;
                }
                else
                {
                    psOffset = gcHWCaps.maxHwNativeTotalInstCount - 1;
                }
            }
            /* initialize unified instruction status */
            if (Hints != gcvNULL)
            {
                Hints->unifiedStatus.useIcache = gcvFALSE;
            }

            /* Set start/end PC */
            if (CodeGen->hasICache)
            {
                gcmONERROR(_SetState(CodeGen,
                                     0x021F,
                                     psOffset));

                gcmONERROR(_SetState(CodeGen,
                                     0x0220,
                                     psOffset + CodeGen->endPC-1));
                if (Hints != gcvNULL)
                {
                    Hints->unifiedStatus.instruction = gcvTRUE;
                    Hints->unifiedStatus.instPSStart = psOffset;
                    Hints->maxInstCount = gcHWCaps.maxHwNativeTotalInstCount;
#if gcdALPHA_KILL_IN_SHADER
                    if (CodeGen->endPCAlphaKill == 0)
                    {
                        Hints->killStateAddress = 0;
                    }
                    else
                    {
                        Hints->killStateAddress    = 0x0220;
                        Hints->alphaKillStateValue = CodeGen->endPCAlphaKill-1;
                        Hints->colorKillStateValue = CodeGen->endPCColorKill-1;
                    }
#endif
                }
            }
            else
            {
                if (gcHWCaps.maxHwNativeTotalInstCount <= 256)
                {
                     /* AQPixelShaderStartPC */
                     gcmONERROR(_SetState(CodeGen,
                                          0x0406,
                                          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:0) - (0 ?
 11:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:0) - (0 ?
 11:0) + 1))))))) << (0 ?
 11:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 11:0) - (0 ?
 11:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:0) - (0 ? 11:0) + 1))))))) << (0 ? 11:0))) |
                                          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)))));

                     /* AQPixelShaderEndPC */
                     gcmONERROR(_SetState(CodeGen,
                                          0x0400,
                                          CodeGen->endPC));

                     if (Hints != gcvNULL)
                     {
                         Hints->unifiedStatus.instruction = gcvFALSE;

#if gcdALPHA_KILL_IN_SHADER
                         if (CodeGen->endPCAlphaKill == 0)
                         {
                             Hints->killStateAddress = 0;
                         }
                         else
                         {
                             Hints->killStateAddress    = 0x0400;
                             Hints->alphaKillStateValue = CodeGen->endPCAlphaKill;
                             Hints->colorKillStateValue = CodeGen->endPCColorKill;
                         }
#endif
                     }

                }
                else
                {
                    gcmONERROR(
                        _SetState(CodeGen,
                                  0x0407,
                                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (psOffset) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (psOffset + CodeGen->endPC-1) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
                                  ));



                    if (Hints != gcvNULL)
                    {
                        Hints->unifiedStatus.instruction = gcvTRUE;
                        Hints->unifiedStatus.instPSStart = psOffset;
                        Hints->maxInstCount = gcHWCaps.maxHwNativeTotalInstCount;

#if gcdALPHA_KILL_IN_SHADER
                        if (CodeGen->endPCAlphaKill == 0)
                        {
                            Hints->killStateAddress = 0;
                        }
                        else
                        {
                            Hints->killStateAddress = 0x0407;
                            Hints->alphaKillStateValue = (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (psOffset) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                                                          |
                                                          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (psOffset + CodeGen->endPCAlphaKill - 1) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
                                                          );
                            Hints->colorKillStateValue = (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (psOffset) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                                                          |
                                                          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (psOffset + CodeGen->endPCColorKill - 1) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
                                                          );
                        }
#endif
                    }

                }
            }

            /* Set code address. */
            if (gcHWCaps.maxHwNativeTotalInstCount > 1024)
            {
                instrBase         = 0x8000 + (psOffset << 2);
                maxNumInstrStates = gcHWCaps.maxHwNativeTotalInstCount << 2;

            }
            else if (gcHWCaps.maxHwNativeTotalInstCount > 256)
            {

                instrBase         = (!CodeGen->hasBugFixes7 ? 0x3000 : 0x2000)
                                    + (psOffset << 2);
                maxNumInstrStates = gcHWCaps.maxHwNativeTotalInstCount << 2;
            }
            else
            {
                instrBase         = 0x1800;
                maxNumInstrStates = 1024;
            }
        }

        if (CodeGen->hasICache && !CodeGen->useICache)
        {
            gcmONERROR(_SetState(CodeGen,
                                 0x021A,
                                 ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
                               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)))
                                 ));

            gcmONERROR(_SetState(CodeGen,
                                 0x040A,
                                 0
                                 ));
        }

        /* Set uniform address. */
        uniformBase = CodeGen->uniformBase;

        if (CodeGen->unifiedUniform)
        {
            gctUINT         count = 0;
            gcsSL_USAGE_PTR usage;

            /* Get constant count. */
            for (count = 0, usage = CodeGen->uniformUsage;
                 count < CodeGen->maxUniform;
                 count++, usage++)
            {
                if (usage->lastUse[0] == gcvSL_AVAILABLE &&
                    usage->lastUse[1] == gcvSL_AVAILABLE &&
                    usage->lastUse[2] == gcvSL_AVAILABLE &&
                    usage->lastUse[3] == gcvSL_AVAILABLE)
                {
                    break;
                }
            }

            if (CodeGen->clShader || CodeGen->computeShader)
            {
                /* Set offset. */
                gcmONERROR(
                    _SetState(CodeGen,
                              0x0409,
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:0) - (0 ?
 9:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:0) - (0 ?
 9:0) + 1))))))) << (0 ?
 9:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 9:0) - (0 ?
 9:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ? 9:0)))
                              ));

                if (Hints != gcvNULL)
                {
                    Hints->constRegNoBase[gcvPROGRAM_STAGE_COMPUTE] =
                    Hints->constRegNoBase[gcvPROGRAM_STAGE_OPENCL]  = 0;
                    Hints->fsConstCount = count;
                    Hints->unifiedStatus.constantUnifiedMode = gcvUNIFORM_ALLOC_PACK_FLOAT_BASE_OFFSET;
                    Hints->unifiedStatus.constPSStart = 0;
                    Hints->unifiedStatus.constCount = Hints->vsConstCount + Hints->fsConstCount;
                }
            }
            else
            {
                gctUINT         offset = 0;
                gctBOOL         unblockUniformBlock = gcvFALSE;
                gctBOOL         handleDefaultUBO = gcvFALSE;
                gctINT          i, nextUniformIndex = 0;

                /* Adjust uniform address. */
                if (Hints)
                {
                    /* If chip can support unified uniform, pack vs and ps. */
                    if (gcHWCaps.hwFeatureFlags.supportUnifiedConstant)
                    {
                        offset = Hints->vsConstCount;
                    }
                    else
                    {
                        offset = gcHWCaps.maxHwNativeTotalConstRegCount - count;
                    }
                    uniformBase += offset * 4;
                }

                /* Set offset. */
                gcmONERROR(
                    _SetState(CodeGen,
                              0x0409,
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:0) - (0 ?
 9:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:0) - (0 ?
 9:0) + 1))))))) << (0 ?
 9:0))) | (((gctUINT32) ((gctUINT32) (offset) & ((gctUINT32) ((((1 ?
 9:0) - (0 ?
 9:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ? 9:0)))
                              ));

                if (Hints != gcvNULL)
                {
                    Hints->maxConstCount = gcHWCaps.maxTotalConstRegCount;
                    Hints->fsConstCount = count;
                    if (gcHWCaps.hwFeatureFlags.supportUnifiedConstant)
                    {
                        Hints->unifiedStatus.constantUnifiedMode = gcvUNIFORM_ALLOC_PACK_FLOAT_BASE_OFFSET;
                    }
                    else
                    {
                        Hints->unifiedStatus.constantUnifiedMode = gcvUNIFORM_ALLOC_GPIPE_TOP_PS_BOTTOM_FLOAT_BASE_OFFSET;
                    }
                    Hints->unifiedStatus.constPSStart = offset;
                    Hints->unifiedStatus.constCount = Hints->vsConstCount + Hints->fsConstCount;

                    /* Update the ps const base address. */
                    Hints->constRegNoBase[gcvPROGRAM_STAGE_FRAGMENT] = offset;
                    Hints->hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT] += offset * 16;

                    /* Remap uniforms with new uniformBase. */
                    CodeGen->uniformBase += offset * 4;
                    offset *= 16;

                    if (Tree->shader->uniformBlockCount) {
                        if(Tree->shader->enableDefaultUBO  &&
                           gcHWCaps.hwFeatureFlags.hasHalti1)
                        {
                            handleDefaultUBO = gcvTRUE;
                        }
                        else if (!gcHWCaps.hwFeatureFlags.hasHalti1 ||
                               (Tree->hints && Tree->hints->uploadedUBO))
                        {
                            unblockUniformBlock = gcvTRUE;
                        }
                        else
                        {
                            unblockUniformBlock = gcvFALSE;
                        }
                    }

                    for (i = 0; i < (gctINT) Tree->shader->uniformCount; ++i)
                    {
                        /* Get uniform. */
                        gcUNIFORM uniform = Tree->shader->uniforms[i];

                        if(!uniform) continue;

                        switch (GetUniformCategory(uniform)) {
                        case gcSHADER_VAR_CATEGORY_NORMAL:
                        case gcSHADER_VAR_CATEGORY_LOD_MIN_MAX:
                        case gcSHADER_VAR_CATEGORY_LEVEL_BASE_SIZE:
                        case gcSHADER_VAR_CATEGORY_SAMPLE_LOCATION:
                        case gcSHADER_VAR_CATEGORY_ENABLE_MULTISAMPLE_BUFFERS:
                        case gcSHADER_VAR_CATEGORY_WORK_THREAD_COUNT:
                        case gcSHADER_VAR_CATEGORY_WORK_GROUP_COUNT:
                        case gcSHADER_VAR_CATEGORY_WORK_GROUP_ID_OFFSET:
                        case gcSHADER_VAR_CATEGORY_GLOBAL_INVOCATION_ID_OFFSET:
                            break;

                        case gcSHADER_VAR_CATEGORY_BLOCK_ADDRESS:
                            if(isUniformStorageBlockAddress(uniform)) break;
                            if(isUniformConstantAddressSpace(uniform)) break;
                            if(handleDefaultUBO)
                            {
                                if(!isUniformUsedInShader(uniform)) continue;
                            }
                            else if(unblockUniformBlock)
                            {
                                continue;
                            }
                            break;

                        case gcSHADER_VAR_CATEGORY_BLOCK_MEMBER:
                            if(handleDefaultUBO)
                            {
                                if(!isUniformMovedToDUB(uniform)) continue;
                            }
                            else if(!unblockUniformBlock) continue;
                            break;

                        default:
                            continue;
                        }

                        if(gcmType_Kind(uniform->u.type) == gceTK_SAMPLER)
                        {
                            continue;
                        }
                        else
                        {
                            if (nextUniformIndex > i)
                                continue;

                            uniform->address += offset;
                        }
                    }
                }
            }
        }
    }

    if (Hints != gcvNULL)
    {
        Hints->hasKill = CodeGen->kill;
    }

    /* Process all constants. */
    if (CodeGen->stateBuffer != gcvNULL &&
        CodeGen->constants != gcvNULL &&
        Tree->shader->constUBOSize > 0 &&
        ! (CodeGen->clShader))
    {
        gctPOINTER pointer = gcvNULL;
        /* Allocate a memory to hold these constants. */
        gcmASSERT(Tree->shader->constUBOData == gcvNULL);
        gcmONERROR(gcoOS_Allocate(gcvNULL,
                                    gcmSIZEOF(gctUINT32) * 4 * Tree->shader->constUBOSize,
                                    &pointer));
        /* Zero this memory. */
        gcoOS_ZeroMemory(pointer, gcmSIZEOF(gctUINT32) * 4 * Tree->shader->constUBOSize);

        Tree->shader->constUBOData = pointer;
        uboSize = Tree->shader->constUBOSize;
        constAddr = Tree->shader->constUBOData + (uboSize - 1) * 4;
    }

    for (c = CodeGen->constants; c != gcvNULL; c = c->next)
    {
        /* Determine offset of uniform. */
        gctINT u;

        /* If a constant is save on a uniform, program it. */
        if (!c->fromUBO)
        {
            for (u = 0; u < c->count; ++u)
            {
                /* Program uniform constant. */
                gctUINT32 index = c->index * 4 + _ExtractSwizzle(c->swizzle, u);

                gcmONERROR(_SetState(CodeGen,
                                     uniformBase + index,
                                     *(gctUINT32_PTR) &c->constant[u]));

                useRegedCTC = gcvTRUE;
            }
        }
        /* If a constant is save on the UBO, save it. */
        else
        {
            if (CodeGen->stateBuffer == gcvNULL) continue;
            gcmASSERT(Tree->shader->constUBOData);

            for (u = 0; u < c->count; u++)
            {
                gcoOS_MemCopy(constAddr, &c->constant[u], gcmSIZEOF(gctUINT32));
                constAddr++;
            }
            constAddr -= c->count;
            uboSize--;
            if (uboSize > 0)
            {
                constAddr -= 4;
            }
        }
    }

    if (Hints != gcvNULL)
    {
        if (CodeGen->shaderType == gcSHADER_TYPE_VERTEX)
        {
            Hints->useRegedCTC[gceSGSK_VERTEX_SHADER] = (gctCHAR)useRegedCTC;
        }
        else if (CodeGen->shaderType == gcSHADER_TYPE_FRAGMENT)
        {
            Hints->useRegedCTC[gceSGSK_FRAGMENT_SHADER] = (gctCHAR)useRegedCTC;
        }
        else
        {
            gcmASSERT(CodeGen->shaderType == gcSHADER_TYPE_COMPUTE ||
                      CodeGen->shaderType == gcSHADER_TYPE_CL);
            Hints->useRegedCTC[gceSGSK_COMPUTE_SHADER] = (gctCHAR)useRegedCTC;
        }
    }

    /* Make sure all constants have been stored. */
    if (CodeGen->stateBuffer != gcvNULL &&
        ! (CodeGen->clShader))
    {
        gcmASSERT(uboSize == 0 && constAddr == Tree->shader->constUBOData);

        /* If there is no constant on UBO, make it as inactive. */
        if (Tree->shader->constUBOSize == 0 && Tree->shader->constUniformBlockIndex != -1)
        {
            gcUNIFORM uniform;
            gcsUNIFORM_BLOCK constUBO;

            /* Get the constant UBO. */
            gcmONERROR(gcSHADER_GetUniformBlock(Tree->shader,
                                              Tree->shader->constUniformBlockIndex,
                                              &constUBO));
            gcmONERROR(gcSHADER_GetUniform(Tree->shader,
                                           GetUBIndex(constUBO),
                                           &uniform));
            SetUniformFlag(uniform, gcvUNIFORM_FLAG_IS_INACTIVE);
        }
    }

#if TEMP_SHADER_PATCH
    gcmASSERT(CodeGen->instCount * 4 == shaderSizeInDW);
#endif

    /* Process all code. */
    /* for HW has 2-group-fast-reissue feature, old CG need to set all instruction as EndOfBB
     * to make sure it can run correctly */
    if (Tree->hwCfg.hwFeatureFlags.supportEndOfBBReissue)
    {
#if TEMP_SHADER_PATCH
        for (i = 0; i < shaderSizeInDW; i += 4)
        {
            gctUINT *inst = BinaryShaderCode + i;
            gctUINT opCode = (((((gctUINT32) (inst[0])) >> (0 ? 5:0)) & ((gctUINT32) ((((1 ? 5:0) - (0 ? 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1)))))) );
            if (opCode == 0x31   ||
                 opCode == 0x10   ||
                 opCode == 0x09   ||
                 opCode == 0x56 ||
                 opCode == 0x0A ||
                 opCode == 0x0B ||
                 opCode == 0x0F)
            {
                /* For instructions who support condition code, this bit occupies
                 * the bit 6 of instruction word1 */
                inst[1] |= 0x40;
            }
            else if (opCode != 0x14   &&
                     opCode != 0x15    &&
                     opCode != 0x16 &&
                     opCode != 0x17  )
            {
                /* For instructions who does not support condition code, this bit
                 * occupies the bit 8 of instruction word0, these instructions are
                 * all non-control-flow instructions except for above instructions. */
                inst[0] |= 0x100;
            }
        }
#else
        gcmASSERT(gcvFALSE);
#endif
    }
    if (! CodeGen->useICache)
    {
#if TEMP_SHADER_PATCH
        for (i = 0, codeAddress = instrBase; i < shaderSizeInDW; i ++)
        {
            /* Program instruction. */
            gcmONERROR(_SetState(CodeGen,
                                 codeAddress++,
                                 BinaryShaderCode[i]));

            numInstrStates ++;
        }
#else
        gcsSL_PHYSICAL_CODE_PTR code;
        gctSIZE_T  f, i;

        for (f = 0, codeAddress = instrBase; f <= Tree->shader->functionCount + Tree->shader->kernelFunctionCount; ++f)
        {

            for (code = CodeGen->functions[f].root;
                 code != gcvNULL;
                 code = code->next)
            {
                /* Process all states. */
                for (i = 0; i < code->count * 4; ++i)
                {
                    /* Program instruction. */
                    gcmONERROR(_SetState(CodeGen,
                                         codeAddress++,
                                         code->states[i]));
                    numInstrStates ++;
                }
            }
        }
#endif
    }
    else
    {
        gctSIZE_T           size = 0;
        gctUINT32           physical = (gctUINT32)~0;
        gctPOINTER          memory = gcvNULL;
        gctPOINTER          pointer = gcvNULL;

        /* Calculate instruction buffer size. */
#if TEMP_SHADER_PATCH
        size = shaderSizeInDW * 4;
#else
        gcsSL_PHYSICAL_CODE_PTR code;
        gctSIZE_T f;

        for (f = 0; f <= Tree->shader->functionCount + Tree->shader->kernelFunctionCount; ++f)
        {
            for (code = CodeGen->functions[f].root;
                 code != gcvNULL;
                 code = code->next)
            {
                size += code->count * 16;
            }
        }
#endif
        if (Hints != gcvNULL)
        {
            gctPOINTER curInstrPtr;
            gcmONERROR(gcoOS_Allocate(gcvNULL,
                       size,
                       &pointer));

            instPtr = pointer;
            curInstrPtr = instPtr;

#if TEMP_SHADER_PATCH
            gcoOS_MemCopy(curInstrPtr, (gctCONST_POINTER)BinaryShaderCode, size);
#else
            for (f = 0; f <= Tree->shader->functionCount + Tree->shader->kernelFunctionCount; ++f)
            {
                for (code = CodeGen->functions[f].root;
                     code != gcvNULL;
                     code = code->next)
                {
                    gcoOS_MemCopy(curInstrPtr, code->states, code->count * 16);
                    curInstrPtr += code->count * 16;
                }
            }
#endif


            gcoSHADER_AllocateVidMem(gcvNULL,
                                     gcvSURF_ICACHE,
                                     "instruction memory for old linker",
                                     size,
                                     256,
                                     &memory,
                                     gcvNULL,
                                     &physical,
                                     instPtr,
                                     gcvFALSE);

            if ((gctUINT32)~0 == physical)
            {
                gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
            }
            if (CodeGen->shaderType == gcSHADER_TYPE_VERTEX)
            {
                Hints->shaderVidNodes.instVidmemNode[gceSGSK_VERTEX_SHADER] = memory;
            }
            else if (CodeGen->shaderType == gcSHADER_TYPE_FRAGMENT)
            {
                Hints->shaderVidNodes.instVidmemNode[gceSGSK_FRAGMENT_SHADER] = memory;
            }
            else if (CodeGen->shaderType == gcSHADER_TYPE_COMPUTE ||
                     CodeGen->shaderType == gcSHADER_TYPE_CL)
            {
                Hints->shaderVidNodes.instVidmemNode[gceSGSK_COMPUTE_SHADER] = memory;
            }

            /* Debug shader. */
            if (dumpCodeGen && CodeGen->lastStateCommand != gcvNULL)
            {
                gctUINT32     address, count;
                gctUINT32_PTR states;
                gctUINT       dumpBufferSize = 1024;
                gctCHAR*      pDumpBuffer;
                VSC_DUMPER    vscDumper;
                VSC_MC_CODEC  mcCodec;

                gcmONERROR(gcoOS_Allocate(gcvNULL, dumpBufferSize, (gctPOINTER*)&pDumpBuffer));

                vscDumper_Initialize(&vscDumper,
                                     gcvNULL,
                                     gcvNULL,
                                     pDumpBuffer,
                                     dumpBufferSize);

                vscMC_BeginCodec(&mcCodec, &gcHWCaps, CodeGen->isDual16Shader, gcvFALSE);

                _DumpUniforms(CodeGen, StateBuffer, CodeGen->stateBufferOffset);

                gcoOS_Print("***** [ Generated Shader Code ] *****");

                for (address = 0, count = size/4, states = instPtr; count >= 4; count -= 4)
                {
                    vscMC_DumpInst(&mcCodec, (VSC_MC_RAW_INST*)states, address++, &vscDumper);
                    states += 4;
                }

                /* Release dumper buffer */
                gcoOS_Free(gcvNULL, pDumpBuffer);
                vscMC_EndCodec(&mcCodec);
            }

            gcoOS_Free(gcvNULL, instPtr);
            instPtr = gcvNULL;
        }

        /* Set I-Cache states. */
        if (CodeGen->shaderType == gcSHADER_TYPE_VERTEX)
        {
            if (CodeGen->hasHalti5)
            {
                gcmONERROR(_SetState(CodeGen,
                             0x021A,
                               ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
                          ));

                gcmONERROR(_SetState(CodeGen,
                             0x022C,
                                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
                          ));

            }
            else
            {
                gcmONERROR(_SetState(CodeGen,
                             0x021A,
                               ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
                             | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)))
                             ));
            }

            gcmONERROR(_SetState(CodeGen,
                                 0x021D,
                                 0));

            if (CodeGen->hasHalti5)
            {
                gcmONERROR(_SetState(CodeGen,
                                     0x022F,
                                     CodeGen->endPC));

            }
            else
            {
                gcmONERROR(_SetState(CodeGen,
                                     0x021E,
                                     CodeGen->endPC-1));

            }
            gcmONERROR(_SetState(CodeGen,
                                 0x021B,
                                 physical
                                 ));


            if (gcHWCaps.hwFeatureFlags.hasInstCachePrefetch)
            {
                if (CodeGen->hasHalti5)
                {
                    gcmONERROR(_SetState(CodeGen,
                                         0x5581,
                                         (CodeGen->instCount - 1)
                                         ));
                }
                else
                {
                    gcmONERROR(_SetState(CodeGen,
                                         0x0224,
                                         (CodeGen->instCount  - 1)
                                         ));
                }

                if (Hints != gcvNULL)
                {
                    int i;

                    Hints->vsICachePrefetch[0] = 0;
                    for (i = 1; i < GC_ICACHE_PREFETCH_TABLE_SIZE; i++)
                    {
                        Hints->vsICachePrefetch[i] = -1;
                    }
                }
            }
        }
        else
        {
            if (CodeGen->hasHalti5)
            {
                gcmONERROR(_SetState(CodeGen,
                                     0x021A,
                                       ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
                                     ));

                gcmONERROR(_SetState(CodeGen,
                                        0x022C,
                                            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)))
                                    ));
            }
            else
            {
                gcmONERROR(_SetState(CodeGen,
                                     0x021A,
                                       ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
                                     | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)))
                                     ));
            }

            gcmONERROR(_SetState(CodeGen,
                                 0x021F,
                                 0));
            if (CodeGen->hasHalti5)
            {
                gcmONERROR(_SetState(CodeGen,
                                     0x0424,
                                     CodeGen->endPC));
            }
            else
            {
                gcmONERROR(_SetState(CodeGen,
                                     0x0220,
                                     CodeGen->endPC-1));
            }

            gcmONERROR(_SetState(CodeGen,
                                 0x040A,
                                 physical
                                 ));


            if (gcHWCaps.hwFeatureFlags.hasInstCachePrefetch)
            {
                if (CodeGen->hasHalti5)
                {
                    gcmONERROR(_SetState(CodeGen,
                                         0x0425,
                                         (CodeGen->instCount - 1)
                                         ));
                }
                else
                {
                    gcmONERROR(_SetState(CodeGen,
                                         0x0413,
                                         (CodeGen->instCount - 1)
                                         ));
                }

                if (Hints != gcvNULL)
                {
                    int i;

                    Hints->fsICachePrefetch[0] = 0;
                    for (i = 1; i < GC_ICACHE_PREFETCH_TABLE_SIZE; i++)
                    {
                        Hints->fsICachePrefetch[i] = -1;
                    }
                }
            }

        }
    }

#if gcdALPHA_KILL_IN_SHADER
    if ((Hints != gcvNULL) && (CodeGen->endPCAlphaKill != 0))
    {
        Hints->killInstructionAddress = (instrBase +
                                         (CodeGen->endPCAlphaKill - 1) * 4
                                         );

        gcoOS_MemCopy(Hints->alphaKillInstruction,
                      CodeGen->alphaKillInstruction,
                      gcmSIZEOF(Hints->alphaKillInstruction));

        gcoOS_MemCopy(Hints->colorKillInstruction,
                      CodeGen->colorKillInstruction,
                      gcmSIZEOF(Hints->colorKillInstruction));
    }
#endif

    /* Calculate the workGroupSize, use the old path that OCL driver used. */
    if (Hints != gcvNULL &&
        (CodeGen->clShader || CodeGen->computeShader) &&
        !Tree->shader->shaderLayout.compute.isWorkGroupSizeFixed)
    {
        gctBOOL hasBarrier = gcvFALSE, hasImageWrite = gcvFALSE;
        gctUINT j;
        gctUINT32 maxRegCount;
        gcSHADER kernelBinary = Tree->shader;
        gctUINT shaderCoreCount = gcHWCaps.maxCoreCount;

        maxRegCount = vscGetHWMaxFreeRegCount(&gcHWCaps);

        for (j = 0; j < GetShaderCodeCount(kernelBinary); j++)
        {
            gcSL_INSTRUCTION inst   = GetShaderInstruction(kernelBinary, j);
            if (gcmSL_OPCODE_GET(inst->opcode, Opcode) == gcSL_BARRIER)
            {
                hasBarrier = gcvTRUE;
            }

            if (gcSL_isOpcodeImageWrite(gcmSL_OPCODE_GET(inst->opcode, Opcode)))
            {
                hasImageWrite = gcvTRUE;
            }
        }

        /* Calculate workGroupSize based on temp register count only when this shader uses BARRIER. */
        if (hasBarrier)
        {
            gctUINT32 maxTempCount = (Hints->threadWalkerInPS) ? Hints->fsMaxTemp : Hints->vsMaxTemp;

            /*
            **  A WAR for bug13179:
            **      relax the workGroupSize by increasing the max register count while shader uses barrier as a HW limitation.
            */
            if (hasImageWrite)
            {
                maxTempCount += 3;
            }

            kernelBinary->shaderLayout.compute.adjustedWorkGroupSize =
                (gctUINT32)(maxRegCount / gcmMAX(2, maxTempCount)) * 4 * shaderCoreCount;
        }
    }

    /* Debug shader. */
    if (dumpCodeGen && CodeGen->lastStateCommand != gcvNULL && !CodeGen->useICache)
    {
        _DumpUniforms(CodeGen, StateBuffer, CodeGen->stateBufferOffset);
        _DumpShader(StateBuffer, CodeGen->stateBufferOffset,
                    CodeGen->hasInteger, instrBase, maxNumInstrStates, CodeGen->isDual16Shader);
    }

    if ((CodeGen->lastStateCommand != gcvNULL) && (numInstrStates > maxNumInstrStates) && (!CodeGen->useICache))
    {
        gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_COMPILER,
            "Not enough instruction memory, need %d, have %d", numInstrStates, maxNumInstrStates);
        gcmUSER_DEBUG_ERROR_MSG(
            "Not enough instruction memory, need %d, have %d.\n", numInstrStates, maxNumInstrStates);

        gcmONERROR(gcvSTATUS_TOO_MANY_INSTRUCTION);
    }
OnError:

    /* Return the required size. */
    *Size = gcmALIGN(CodeGen->stateBufferOffset, 8);
    *StateDeltaBufferSize = CodeGen->stateDeltaBufferOffset * 4;
#if TEMP_SHADER_PATCH
   if (BinaryShaderCode)
       gcoOS_Free(gcvNULL, BinaryShaderCode);
#endif

    if (instPtr != gcvNULL)
    {
        gcoOS_Free(gcvNULL, instPtr);
    }

    /* Return the status. */
    return status;
}

static gctBOOL
_CheckForPhase0HighpPositionEnabled(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR *CodeGen
    )
{
    gctSIZE_T i;
    gctBOOL hasFragDepth = gcvFALSE;
    if (CodeGen->shaderType == gcSHADER_TYPE_VERTEX) return gcvTRUE;

    for (i = 0; i < Tree->shader->outputCount; ++i) {
        if (Tree->shader->outputs[i] == gcvNULL) continue;
          /* Extract internal name for output. */
        if (Tree->shader->outputs[i]->nameLength == gcSL_DEPTH) {
            gctINT temp;

            temp = Tree->outputArray[i].tempHolding;
            if(Tree->tempArray[temp].assigned != -1) hasFragDepth = gcvTRUE;
            break;
        }
    }

    /* Check for glFragCoord || glFragDepth */
    for (i = 0; i < Tree->shader->attributeCount; ++i) {
        if (Tree->attributeArray[i].inUse)  {
            /* Get attribute. */
            gcATTRIBUTE attribute = Tree->shader->attributes[i];

            /* Check for special POSITION attribute. */
            if (attribute->nameLength == gcSL_POSITION) {
                CodeGen->usePosition  = (CodeGen->flags & gcvSHADER_USE_GL_POSITION);
                CodeGen->positionIndex = i;
                return gcvTRUE;
            }
        }
    }

    return hasFragDepth;
}

static gctUINT
_GetDefaultSamplerBaseOffset(
    IN gcSHADER_KIND               ShaderKind
    )
{
    gctUINT samplerBaseOffset = 0;

    switch (ShaderKind)
    {
    case gcSHADER_TYPE_VERTEX:
    case gcSHADER_TYPE_VERTEX_DEFAULT_UBO:
        samplerBaseOffset = gcHWCaps.vsSamplerRegNoBase;
        break;

    case gcSHADER_TYPE_FRAGMENT:
    case gcSHADER_TYPE_FRAGMENT_DEFAULT_UBO:
        samplerBaseOffset = gcHWCaps.psSamplerRegNoBase;
        break;

    case gcSHADER_TYPE_TCS:
        samplerBaseOffset = gcHWCaps.tcsSamplerRegNoBase;
        break;

    case gcSHADER_TYPE_TES:
        samplerBaseOffset = gcHWCaps.tesSamplerRegNoBase;
        break;

    case gcSHADER_TYPE_GEOMETRY:
        samplerBaseOffset = gcHWCaps.gsSamplerRegNoBase;
        break;

    case gcSHADER_TYPE_CL:
    case gcSHADER_TYPE_COMPUTE:
        samplerBaseOffset = gcHWCaps.csSamplerRegNoBase;
        break;

    default:
        break;
    }

    return samplerBaseOffset;
}

/*******************************************************************************
**
**  gcLINKTREE_GenerateStates
**
**  Generate hardware states from the shader.
**
**  INPUT:
**
**      gcLINKTREE pTree
**          Pointer to a gcLINKTREE object.
**
**      gceSHADER_FLAGS Flags
**          Linker flags.
**
**      gctUINT32 * StateBufferSize
**          Size of state buffer on entry.
**
**      gctPOINTER * StateBuffer
**          Pointer to state buffer.
**
**  OUTPUT:
**
**      IN OUT gcsPROGRAM_STATE             *ProgramState
**          Pointer to a variable receiving the program states
*/
gceSTATUS
gcLINKTREE_GenerateStates(
    IN OUT gcLINKTREE                  *pTree,
    IN gceSHADER_FLAGS                  Flags,
    IN gcsSL_USAGE_PTR                  UniformUsage,
    IN gcsSL_CONSTANT_TABLE_PTR         ConstUsage,
    IN OUT gcsPROGRAM_STATE             *ProgramState
    )
{
    gceSTATUS           status;
    gctSIZE_T           size;
    gctSIZE_T           size2;
    gctPOINTER          pointer = gcvNULL;
    gctUINT32           i;
    gctUINT32           vsSamplers, psSamplers;
    gctINT              vsSamplersBase, psSamplersBase;
    gcShaderCodeInfo    codeInfo;
    gcLINKTREE          tree = *pTree;
    /* Extract the gcSHADER shader object. */
    gcSHADER            shader = tree->shader;
    gcsHINT_PTR         hints;
    gctUINT32           stateBufferSize;
    gctUINT32           stateDeltaSize;
    gctUINT8 *          stateBuffer = gcvNULL;
    gctUINT32 *         stateDeltaBuffer = gcvNULL;

    /* The common code generator structure. */
    gcsCODE_GENERATOR   codeGen = { (gceSHADER_FLAGS) 0 };

    gcmHEADER_ARG("pTree=0x%x Flags=%d ProgramState=0x%x",
                    pTree, Flags, ProgramState);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(ProgramState != gcvNULL);

    /* old hints and stateBufferSize */
    hints = ProgramState->hints;
    stateBufferSize = ProgramState->stateBufferSize;
    stateDeltaSize = ProgramState->stateDeltaSize;

    /* count the code in the shader */
    gcoOS_ZeroMemory(&codeInfo, gcmSIZEOF(codeInfo));
    gcSHADER_CountCode(shader, &codeInfo);

    /* Determine if OpenCL shader. */
    codeGen.clShader = (shader->type == gcSHADER_TYPE_CL);
    codeGen.computeShader = (shader->type == gcSHADER_TYPE_COMPUTE);

    /* Set shaderType. */
    codeGen.shaderType = shader->type;
    if (codeGen.clShader)
    {
        /* Treat as vertex shader for linking purposes from this point on */
        if (!gcHWCaps.hwFeatureFlags.hasThreadWalkerInPS)
        {
            codeGen.shaderType = gcSHADER_TYPE_VERTEX;
        }
    }

    /* Determine if HALTI shader. */
    codeGen.haltiShader = gcSHADER_IsHaltiCompiler(shader);

    /* Cache hardware flags. */
    codeGen.isCL_X  = gcHWCaps.hwFeatureFlags.needCLXFixes;
    codeGen.isCL_XE = gcHWCaps.hwFeatureFlags.needCLXEFixes;
    codeGen.hasCL   = codeGen.isCL_X || codeGen.isCL_XE;
    codeGen.hasInteger = gcHWCaps.hwFeatureFlags.supportInteger;

    /* Determine if HW has dual-16 support. */
    codeGen.hasDual16 = _gcmHasDual16(shader);

    /* Determine if shader is in dual-16 mode. */
    codeGen.isDual16Shader = gcvFALSE;
    if(!(codeGen.clShader || codeGen.computeShader) && codeGen.hasDual16) {
        codeGen.isDual16Shader = gcSHADER_IsDual16Shader(shader, &codeInfo);
    }

    codeGen.shaderCoreCount  = gcHWCaps.maxCoreCount;

    if (codeGen.clShader && ! codeGen.hasCL)
    {
        gcmUSER_DEBUG_ERROR_MSG("GPU does not support OpenCL");
        gcmONERROR(gcvSTATUS_NOT_SUPPORT_CL);
    }

    if (codeGen.haltiShader && ! codeGen.hasInteger)
    {
        gcmUSER_DEBUG_ERROR_MSG("GPU does not support GLES30");
        gcmONERROR(gcvSTATUS_NOT_SUPPORT_INTEGER);
    }

    if (gcHWCaps.hwFeatureFlags.hasInstCache)
    {
        codeGen.hasICache = gcvTRUE;
        if (codeGen.clShader || (tree && tree->useICache))
        {
            codeGen.useICache = gcvTRUE;
        }
        else
        {
            codeGen.useICache = gcvFALSE;
        }
    }
    else
    {
        codeGen.hasICache = gcvFALSE;
        codeGen.useICache = gcvFALSE;
    }

    if (gcHWCaps.hwFeatureFlags.hasSHEnhance2)
    {
        codeGen.hasSHEnhancements2 = gcvTRUE;
        if (gcmOPT_NOIMMEDIATE())
            codeGen.generateImmediate  = gcvFALSE;
        else
            codeGen.generateImmediate  = gcvTRUE;
    }
    else
    {
        codeGen.hasSHEnhancements2 = gcvFALSE;
        codeGen.generateImmediate  = gcvFALSE;
    }

    if (gcHWCaps.hwFeatureFlags.hasSHEnhance3)
    {
        codeGen.hasSHEnhancements3 = gcvTRUE;
    }
    else
    {
        codeGen.hasSHEnhancements3 = gcvFALSE;
    }

    if (gcmOPT_FORCEIMMEDIATE())
            codeGen.forceGenImmediate  = gcvTRUE;

    if (gcHWCaps.hwFeatureFlags.hasBugFix10)
    {
        codeGen.hasBugFixes10 = gcvTRUE;
    }
    else
    {
        codeGen.hasBugFixes10 = gcvFALSE;

        if (codeGen.clShader && !codeGen.useICache &&
            gcHWCaps.maxHwNativeTotalInstCount < 1024)
        {
            if (tree->shader->codeCount > 422 && tree->shader->codeCount < 434 &&
                tree->shader->attributeCount == 1 && tree->shader->uniformCount == 2)
            {
                /* No STORE1. */
                gctUINT i;
                gcSL_INSTRUCTION codes = tree->shader->code;
#define __NUM_INST__    26
                gctUINT16 opcodes[__NUM_INST__] = {
                    gcSL_MOV, gcSL_MOV, gcSL_JMP, gcSL_JMP,
                    gcSL_MOV, gcSL_MOV, gcSL_MOV, gcSL_MOV, gcSL_LOAD,
                    gcSL_STORE, gcSL_STORE, gcSL_STORE, gcSL_STORE,
                    gcSL_MOV, gcSL_LOAD,
                    gcSL_STORE, gcSL_STORE, gcSL_STORE, gcSL_STORE,
                    gcSL_MOV, gcSL_LOAD, gcSL_LOAD,
                    gcSL_STORE, gcSL_STORE, gcSL_STORE, gcSL_STORE,
                };

                for (i = 0; i < __NUM_INST__; i++)
                {
                    if (gcmSL_OPCODE_GET(codes[i].opcode, Opcode) != opcodes[i]) break;
                }
                if (i == __NUM_INST__)
                {
                    codeGen.hasBugFixes10 = gcvTRUE;
                }
            }
            else if (tree->shader->codeCount > 545 && tree->shader->codeCount < 561 &&
                tree->shader->attributeCount == 1 && tree->shader->uniformCount == 2)
            {
                /* No STORE1. */
                gctUINT i;
                gcSL_INSTRUCTION codes = tree->shader->code;
#define __NUM_INST1__    37
                gctUINT16 opcodes[__NUM_INST1__] = {
                    gcSL_MOV, gcSL_MOV, gcSL_JMP, gcSL_JMP,
                    gcSL_MOV, gcSL_MOV, gcSL_MOV, gcSL_MOV, gcSL_LOAD,
                    gcSL_STORE1, gcSL_ADD, gcSL_STORE1, gcSL_ADD, gcSL_STORE1, gcSL_ADD, gcSL_STORE1,
                    gcSL_MOV, gcSL_LOAD,
                    gcSL_ADD, gcSL_STORE1, gcSL_ADD, gcSL_STORE1, gcSL_ADD, gcSL_STORE1, gcSL_ADD, gcSL_STORE1,
                    gcSL_MOV, gcSL_LOAD, gcSL_LOAD,
                    gcSL_ADD, gcSL_STORE1, gcSL_ADD, gcSL_STORE1, gcSL_ADD, gcSL_STORE1, gcSL_ADD, gcSL_STORE1,
                };

                for (i = 0; i < __NUM_INST1__; i++)
                {
                    if (gcmSL_OPCODE_GET(codes[i].opcode, Opcode) != opcodes[i]) break;
                }
                if (i == __NUM_INST1__)
                {
                    codeGen.hasBugFixes10 = gcvTRUE;
                }
            }
        }
    }

    codeGen.hasBugFixes11 = gcHWCaps.hwFeatureFlags.hasBugFix11;

    codeGen.hasBugFixes7 = gcHWCaps.hwFeatureFlags.hasBugFix7;

    /* Determine if hardware is bigEndian */
    codeGen.isBigEndian = gcHWCaps.hwFeatureFlags.bigEndianMI;

    if (codeGen.shaderType == gcSHADER_TYPE_VERTEX)
    {
        codeGen.uniformBase    = gcHWCaps.vsConstRegAddrBase;
        codeGen.maxUniform     = gcHWCaps.maxVSConstRegCount;
        codeGen.unifiedUniform = gcHWCaps.unifiedConst;

        /* If this chip can support unified constant, then this shader can use all of the constant register. */
        if (codeGen.unifiedUniform)
        {
            codeGen.maxUniform = gcmMIN(512, gcHWCaps.maxTotalConstRegCount);
        }
    }
    else if (codeGen.clShader || codeGen.computeShader)
    {
        /* If unified constant registers, CL/CS shaders can use all of them. */
        if (gcHWCaps.hwFeatureFlags.constRegFileUnified)
        {
            codeGen.uniformBase    = gcHWCaps.psConstRegAddrBase;
            codeGen.maxUniform     = gcmMIN(512, gcHWCaps.maxTotalConstRegCount);
            codeGen.unifiedUniform = gcHWCaps.unifiedConst;
        }
#if !gcdENABLE_UNIFIED_CONSTANT
        else if ((gcHWCaps.maxTotalConstRegCount== 320 &&
                  gcHWCaps.maxPSConstRegCount == 64 &&
                  gcHWCaps.maxVSConstRegCount == 256) &&
                  !codeGen.computeShader)
        {
            codeGen.uniformBase    = 0xC000;
            codeGen.maxUniform     = gcHWCaps.maxTotalConstRegCount;
            codeGen.unifiedUniform = gcvTRUE;
        }
#endif
        else
        {
            codeGen.uniformBase    = gcHWCaps.psConstRegAddrBase;
            codeGen.maxUniform     = gcHWCaps.maxPSConstRegCount;
            codeGen.unifiedUniform = gcHWCaps.unifiedConst;
        }
    }
    else
    {
        codeGen.uniformBase    = gcHWCaps.psConstRegAddrBase;
        codeGen.maxUniform     = gcHWCaps.maxPSConstRegCount;
        codeGen.unifiedUniform = gcHWCaps.unifiedConst;

        /* If this chip can support unified constant, then this shader can use all of the constant register. */
        if (codeGen.unifiedUniform)
        {
            codeGen.maxUniform = gcmMIN(512, gcHWCaps.maxTotalConstRegCount);
        }
    }

    vsSamplers = gcHWCaps.maxVSSamplerCount;
    psSamplers = gcHWCaps.maxPSSamplerCount;
    vsSamplersBase = gcHWCaps.vsSamplerNoBaseInInstruction;
    psSamplersBase = gcHWCaps.psSamplerNoBaseInInstruction;

    codeGen.dummySamplerId = (codeGen.shaderType == gcSHADER_TYPE_VERTEX)
        ? vsSamplersBase + vsSamplers - 1
        : psSamplersBase + psSamplers - 1;

    codeGen.maxVaryingVectors = gcHWCaps.maxVaryingCount;

    codeGen.maxExtraVaryingVectors = 3;

    codeGen.maxAttributes = gcHWCaps.maxAttributeCount;

    /* Get the maximum number of registers. */
    codeGen.registerCount = gcHWCaps.maxGPRCountPerThread;
    codeGen.flags = Flags;

    codeGen.hasSIGN_FLOOR_CEIL = gcHWCaps.hwFeatureFlags.hasSignFloorCeil;

    codeGen.hasSQRT_TRIG = gcHWCaps.hwFeatureFlags.hasSqrtTrig;

    codeGen.hasNEW_SIN_COS_LOG_DIV = gcHWCaps.hwFeatureFlags.hasNewSinCosLogDiv;

    codeGen.hasMediumPrecision = gcHWCaps.hwFeatureFlags.hasMediumPrecision;

    codeGen.hasNEW_TEXLD = gcHWCaps.hwFeatureFlags.hasHalti2;
    codeGen.hasHalti3    = gcHWCaps.hwFeatureFlags.hasHalti3;
    codeGen.hasHalti4    = gcHWCaps.hwFeatureFlags.hasHalti4;
    codeGen.hasHalti5    = gcHWCaps.hwFeatureFlags.hasHalti5;
    codeGen.hasUSC       = gcHWCaps.hwFeatureFlags.supportUSC;

    if (UniformUsage == gcvNULL)
    {
        /* Create uniform usage table. */
        gcmONERROR(
            gcoOS_Allocate(gcvNULL,
                           gcmSIZEOF(gcsSL_USAGE) * codeGen.maxUniform,
                           &pointer));

        codeGen.uniformUsage = pointer;

        gcoOS_MemFill(codeGen.uniformUsage,
                      0xFF,
                      gcmSIZEOF(gcsSL_USAGE) * codeGen.maxUniform);
    }
    else
    {
        codeGen.uniformUsage = UniformUsage;
    }

    /* Create register usage table. */
    gcmONERROR(
        gcoOS_Allocate(gcvNULL,
                       gcmSIZEOF(gcsSL_USAGE) * codeGen.registerCount,
                       &pointer));

    codeGen.registerUsage = pointer;

    gcoOS_MemFill(codeGen.registerUsage,
                  0xFF,
                  gcmSIZEOF(gcsSL_USAGE) * codeGen.registerCount);

    codeGen.isConstOutOfMemory = gcvFALSE;
    codeGen.isRegOutOfResource = gcvFALSE;

    /* Allocate a new hint structure. */
    if (hints == gcvNULL)
    {
        gctUINT i;

        gcmONERROR(gcoOS_Allocate(gcvNULL,
                                    gcmSIZEOF(struct _gcsHINT),
                                    &pointer));
        gcoOS_ZeroMemory(pointer, gcmSIZEOF(struct _gcsHINT));

        hints = pointer;

        for (i = 0; i < GC_ICACHE_PREFETCH_TABLE_SIZE; i++)
        {
            hints->vsICachePrefetch[i] = -1;
            hints->fsICachePrefetch[i] = -1;
            hints->tcsICachePrefetch[i] = -1;
            hints->tesICachePrefetch[i] = -1;
            hints->gsICachePrefetch[i] = -1;
        }

        hints->unifiedStatus.instVSEnd           = -1;
        hints->unifiedStatus.instPSStart         = -1;
        hints->unifiedStatus.constGPipeEnd       = -1;
        hints->unifiedStatus.constPSStart        = -1;
        hints->unifiedStatus.samplerGPipeStart   = vsSamplersBase;
        hints->unifiedStatus.samplerPSEnd        = vsSamplersBase - 1;
        /* By default it is GPIPE_TOP_PS_BOTTOM unified. */
        hints->unifiedStatus.constantUnifiedMode = gcvUNIFORM_ALLOC_GPIPE_TOP_PS_BOTTOM_FLOAT_BASE_OFFSET;
        /* By default it is non-unified. */
        hints->unifiedStatus.samplerUnifiedMode  = gcvUNIFORM_ALLOC_NONE_UNIFIED;
        hints->psOutCntl0to3                     = -1;
        hints->psOutCntl4to7                     = -1;
        hints->psOutCntl8to11                    = -1;
        hints->psOutCntl12to15                   = -1;
    }

    if (codeGen.shaderType == gcSHADER_TYPE_VERTEX)
    {
        hints->hwConstRegBases[gcvPROGRAM_STAGE_VERTEX]  = codeGen.uniformBase * 4;
        hints->constRegNoBase[gcvPROGRAM_STAGE_VERTEX]   = 0;
    }
    else
    {
        hints->hwConstRegBases[gcvPROGRAM_STAGE_COMPUTE] =
        hints->hwConstRegBases[gcvPROGRAM_STAGE_OPENCL]  =
        hints->hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT]= codeGen.uniformBase * 4;

        hints->constRegNoBase[gcvPROGRAM_STAGE_COMPUTE]  =
        hints->constRegNoBase[gcvPROGRAM_STAGE_OPENCL]   =
        hints->constRegNoBase[gcvPROGRAM_STAGE_FRAGMENT] = 0;
    }

#if gcdUSE_WCLIP_PATCH
    if (hints != gcvNULL && tree->shader->type == gcSHADER_TYPE_VERTEX)
    {
        hints->strictWClipMatch = tree->strictWClipMatch;
        hints->MVPCount = tree->MVPCount;

        hints->WChannelEqualToZ = tree->WChannelEqualToZ;
    }
#endif

    gcmASSERT(hints != gcvNULL);

    /* If VS or PS disable EarlyZ, we disable it for this program. */
    if (shader->disableEarlyZ && hints)
    {
        hints->disableEarlyZ = gcvTRUE;
    }

    /* Map all attributes. */
    {
        gctINT attributeCount = 0;

        if (_isHWRegisterAllocated(tree->shader))
        {
            if(codeGen.isDual16Shader &&
               _CheckForPhase0HighpPositionEnabled(tree, &codeGen))
            {
                gcmONERROR(_MapAttributesDual16RAEnabled(tree, &codeGen, codeGen.registerUsage, hints));
            }
            else
            {
                gcmONERROR(_MapAttributesRAEnabled(tree, &codeGen, codeGen.registerUsage, hints));
            }
        }
        else
        {
            if(codeGen.isDual16Shader &&
               _CheckForPhase0HighpPositionEnabled(tree, &codeGen))
            {
                gcmONERROR(_MapAttributesDual16(tree, &codeGen, codeGen.registerUsage, &attributeCount, hints));
            }
            else
            {
                gcmONERROR(_MapAttributes(tree, &codeGen, codeGen.registerUsage, &attributeCount, hints));
            }
        }

        codeGen.vsHasVertexInstId = gcvFALSE;
        codeGen.vertexIdIndex = -1;
        codeGen.instanceIdIndex = -1;
        if(codeGen.shaderType == gcSHADER_TYPE_VERTEX &&
           gcHWCaps.hwFeatureFlags.vtxInstanceIdAsAttr)
        {
            gctUINT i;
            gcVARIABLE variable;

            for (i = 0; i < tree->tempCount; i++)
            {
                if (!tree->tempArray[i].inUse ||
                    tree->tempArray[i].variable == gcvNULL)
                {
                    continue;
                }

                if (tree->tempArray[i].defined != gcvNULL)
                {
                    if (tree->tempArray[i].variable)
                    {
                        if (tree->tempArray[i].variable->nameLength != gcSL_INSTANCE_ID &&
                            tree->tempArray[i].variable->nameLength != gcSL_VERTEX_ID)
                        {
                            continue;
                        }
                    }
                    else
                    {
                        continue;
                    }
                }

                variable = tree->tempArray[i].variable;

                if (!isVariableNormal(variable)) continue;

                if (variable->nameLength == gcSL_INSTANCE_ID)
                 {
                     codeGen.instanceIdIndex = variable->tempIndex;
                     codeGen.vsHasVertexInstId = gcvTRUE;
                     if(codeGen.vertexIdIndex != -1)
                     {
                         break;
                     }
                 }
                 else if (variable->nameLength == gcSL_VERTEX_ID)
                 {
                     codeGen.vertexIdIndex = variable->tempIndex;
                     codeGen.vsHasVertexInstId = gcvTRUE;
                     if(codeGen.instanceIdIndex != -1)
                     {
                         break;
                     }
                 }
            }

            if (!_isHWRegisterAllocated(tree->shader))
            {
            if(codeGen.vsHasVertexInstId && /* assign instance/vertex id registers */
               gcHWCaps.hwFeatureFlags.vtxInstanceIdAsAttr)
            {
                _SetRegisterUsage(codeGen.registerUsage + attributeCount,
                                  1,
                                  gcSL_ENABLE_XYZW,
                                  gcvSL_RESERVED);
                if(codeGen.vertexIdIndex != -1)
                {
                    gcLINKTREE_TEMP temp = &tree->tempArray[codeGen.vertexIdIndex];
                    temp->assigned = attributeCount;
                    temp->swizzle  = gcSL_SWIZZLE_XXXX;
                    temp->shift    = 0;
                    if (gcSHADER_DumpCodeGenVerbose(tree->shader))
                        dumpRegisterAllocation(temp);
               }

                if(codeGen.instanceIdIndex != -1)
                {
                    gcLINKTREE_TEMP temp = &tree->tempArray[codeGen.instanceIdIndex];
                    temp->assigned = attributeCount;
                    temp->swizzle  = gcSL_SWIZZLE_YYYY;
                    temp->shift    = 0;
                    if (gcSHADER_DumpCodeGenVerbose(tree->shader))
                        dumpRegisterAllocation(temp);
                }
            }
            }
        }
    }

    /* Make sure that the lastUse of all output is -1. */
    for (i = 0; i < shader->outputCount; i++)
    {
        gcOUTPUT output;
        gcLINKTREE_TEMP tempOutput;
        gctUINT size, regIdx, endRegIdx;

        /* Get gcOUTPUT pointer. */
        output = shader->outputs[i];

        if (output != gcvNULL)
        {
            gctUINT32 tempIndex = output->tempIndex;

            /* If the temp reg index is larger than the max temp reg,
            ** then this output is no used.
            */
            if (tempIndex >= tree->tempCount)
                continue;

            /* we expand output array to elements of VS, but for fragment shader, */
            /* we consider it as entity. We should refine it to match at two sides later! */
            size = 1; /*output->arraySize;*/

            if (gcmType_isMatrix(output->type)) size *= gcmType_Rows(output->type);
            endRegIdx = tempIndex + size;

            for (regIdx = tempIndex; regIdx < endRegIdx; regIdx ++)
            {
                /* Get temporary register defining this output. */
                tempOutput = &tree->tempArray[regIdx];

                /* Make sure the temp is marked as an output. */
                tempOutput->lastUse = -1;
            }
        }
    }

    hints->useDSX = (codeInfo.codeCounter[gcSL_DSX] != 0);
    hints->useDSY = (codeInfo.codeCounter[gcSL_DSY] != 0);
    hints->yInvertAware = hints->useFragCoord[1]   ||
                          hints->useSamplePosition ||
                          hints->useFrontFacing    ||
                          hints->usePointCoord[1]  ||
                          hints->useDSY;

    /* If uniforms are not mapped before generating states, map them here. */
    if (UniformUsage == gcvNULL)
    {
        /* Map shader used uniforms/samplers. */
        gcmONERROR(_MapUniforms(tree,
                                &codeGen,
                                codeGen.uniformUsage,
                                codeGen.maxUniform,
                                codeGen.uniformBase,
                                0, /* start index */
                                hints
                                ));
    }

    /* Map all outputs. */
    if (!_isHWRegisterAllocated(tree->shader))
    {
        gcmONERROR(_MapFragmentOutputs(tree, &codeGen, codeGen.registerUsage));
    }

    /* Create function structures. */
    gcmONERROR(gcoOS_Allocate(gcvNULL,
                                gcmSIZEOF(gcsSL_FUNCTION_CODE) *
                                    (shader->functionCount + shader->kernelFunctionCount + 1),
                                &pointer));

    codeGen.functions = pointer;

    gcoOS_ZeroMemory(codeGen.functions,
                     gcmSIZEOF(gcsSL_FUNCTION_CODE) *
                         (shader->functionCount + shader->kernelFunctionCount + 1));

    codeGen.instCount = 0; /* init the isntruction count */

    /* Create code mapping table. */
    if (shader->codeCount > 0)
    {
        gcmONERROR(gcoOS_Allocate(gcvNULL,
                                    gcmSIZEOF(gcsSL_CODE_MAP) *
                                        shader->codeCount,
                                    &pointer));

        codeGen.codeMap = pointer;

        gcoOS_ZeroMemory(codeGen.codeMap,
                         gcmSIZEOF(gctUINT) * shader->codeCount);

    }
    else
    {
        codeGen.codeMap = gcvNULL;
    }

    /* Generate code for each instruction. */
    gcmONERROR(_GenerateCode(tree, &codeGen));

    /* Compute the size of the state buffer. */
    gcmONERROR(_GenerateStates(tree, &codeGen, gcvNULL, &size, gcvNULL, &size2, gcvNULL));

    /* Allocate a new state buffer. */
    gcmONERROR(gcoOS_Allocate(gcvNULL,
                                stateBufferSize + size,
                                &pointer));
    stateBuffer = pointer;

    /* Copy the old state buffer if there is any. */
    if (stateBufferSize > 0)
    {
        gcmASSERT(ProgramState->stateBuffer != gcvNULL);

        gcoOS_MemCopy(stateBuffer, ProgramState->stateBuffer, stateBufferSize);
    }

    /* Allocate a new state delta buffer. */
    gcmONERROR(gcoOS_Allocate(gcvNULL,
                                stateDeltaSize + size2,
                                &pointer));
    stateDeltaBuffer = pointer;

    /* Copy the old state delta buffer if there is any. */
    if (stateDeltaSize > 0)
    {
        gcmASSERT(ProgramState->stateDelta != gcvNULL);

        gcoOS_MemCopy(stateDeltaBuffer, ProgramState->stateDelta, stateDeltaSize);
    }


    /* set program stage bits */
    if (codeGen.clShader)
    {
        gcsHINT_SetProgramStageBit(hints, gcvPROGRAM_STAGE_OPENCL);
    }
    else if (codeGen.shaderType == gcSHADER_TYPE_VERTEX)
    {
        gcsHINT_SetProgramStageBit(hints, gcvPROGRAM_STAGE_VERTEX);
    }
    else if (codeGen.shaderType == gcSHADER_TYPE_FRAGMENT)
    {
        gcsHINT_SetProgramStageBit(hints, gcvPROGRAM_STAGE_FRAGMENT);
    }
    else if (codeGen.shaderType == gcSHADER_TYPE_COMPUTE)
    {
        gcsHINT_SetProgramStageBit(hints, gcvPROGRAM_STAGE_COMPUTE);
    }

    {
        gceMEMORY_ACCESS_FLAG memoryAccessFlag = gceMA_FLAG_NONE;
        gceFLOW_CONTROL_FLAG flowControlFlag = gceFC_FLAG_NONE;
        gceTEXLD_FLAG texldFlag = gceTEXLD_FLAG_NONE;
        gctBOOL     threadGroupSync = gcvFALSE;
        gctINT      stageIndex = 0;

        /* set ld/st hint attribute for the program, any load/store in
         * vertex shader or fragment shader would flag the program as
         * using laod/store */
        if (codeInfo.codeCounter[gcSL_LOAD] != 0)
        {
            memoryAccessFlag |= gceMA_FLAG_LOAD;
        }

        if ((codeInfo.codeCounter[gcSL_STORE] != 0) ||
            (codeInfo.codeCounter[gcSL_STORE1] != 0))
        {
            memoryAccessFlag |= gceMA_FLAG_STORE;
        }

        if ((codeInfo.codeCounter[gcSL_IMAGE_RD] != 0) ||
            (codeInfo.codeCounter[gcSL_IMAGE_RD_3D] != 0))
        {
            memoryAccessFlag |= gceMA_FLAG_IMG_READ;
        }

        if ((codeInfo.codeCounter[gcSL_IMAGE_WR] != 0) ||
            (codeInfo.codeCounter[gcSL_IMAGE_WR_3D] != 0))
        {
            memoryAccessFlag |= gceMA_FLAG_IMG_WRITE;
        }

        if ((codeInfo.codeCounter[gcSL_ATOMADD] != 0) ||
            (codeInfo.codeCounter[gcSL_ATOMSUB] != 0) ||
            (codeInfo.codeCounter[gcSL_ATOMXCHG] != 0) ||
            (codeInfo.codeCounter[gcSL_ATOMCMPXCHG] != 0) ||
            (codeInfo.codeCounter[gcSL_ATOMMIN] != 0) ||
            (codeInfo.codeCounter[gcSL_ATOMMAX] != 0) ||
            (codeInfo.codeCounter[gcSL_ATOMOR] != 0) ||
            (codeInfo.codeCounter[gcSL_ATOMAND] != 0) ||
            (codeInfo.codeCounter[gcSL_ATOMXOR] != 0))
        {
            memoryAccessFlag |= gceMA_FLAG_ATOMIC;
        }

        if ((codeInfo.codeCounter[gcSL_JMP] != 0) ||
            (codeInfo.codeCounter[gcSL_JMP_ANY] != 0))
        {
            flowControlFlag |= gceFC_FLAG_JMP;
        }

        if (codeInfo.codeCounter[gcSL_CALL] != 0)
        {
            flowControlFlag |= gceFC_FLAG_CALL;
        }

        if (codeInfo.codeCounter[gcSL_KILL] != 0)
        {
            flowControlFlag |= gceFC_FLAG_KILL;
        }

        if (codeInfo.codeCounter[gcSL_TEXLD] != 0       ||
            codeInfo.codeCounter[gcSL_TEXLD_U] != 0     ||
            codeInfo.codeCounter[gcSL_TEXLDPROJ] != 0   ||
            codeInfo.codeCounter[gcSL_TEXLDPCF] != 0    ||
            codeInfo.codeCounter[gcSL_TEXLODQ] != 0     ||
            codeInfo.codeCounter[gcSL_TEXLDPCFPROJ] != 0)
        {
            texldFlag |= gceTEXLD_FLAG_TEXLD;
        }

        if (codeInfo.codeCounter[gcSL_BARRIER] != 0)
        {
            threadGroupSync = gcvTRUE;
        }

        if (codeGen.shaderType == gcSHADER_TYPE_VERTEX)
        {
            stageIndex = gcvPROGRAM_STAGE_VERTEX;
        }
        else if (codeGen.shaderType == gcSHADER_TYPE_FRAGMENT)
        {
            stageIndex = gcvPROGRAM_STAGE_FRAGMENT;
        }
        else if (codeGen.shaderType == gcSHADER_TYPE_COMPUTE)
        {
            stageIndex = gcvPROGRAM_STAGE_COMPUTE;
        }
        else
        {
            gcmASSERT(codeGen.shaderType == gcSHADER_TYPE_CL);
            stageIndex = gcvPROGRAM_STAGE_OPENCL;
        }
        /* So far we won't generate any memory-access-related instruction for old CG. */
        hints->memoryAccessFlags[gcvSHADER_HIGH_LEVEL][stageIndex] = memoryAccessFlag;
        hints->memoryAccessFlags[gcvSHADER_MACHINE_LEVEL][stageIndex] = memoryAccessFlag;

        hints->flowControlFlags[gcvSHADER_HIGH_LEVEL][stageIndex] = flowControlFlag;
        hints->flowControlFlags[gcvSHADER_MACHINE_LEVEL][stageIndex] = flowControlFlag;

        hints->texldFlags[gcvSHADER_HIGH_LEVEL][stageIndex] = texldFlag;
        hints->texldFlags[gcvSHADER_MACHINE_LEVEL][stageIndex] = texldFlag;

        hints->threadGroupSync = threadGroupSync;
        /* The shader invocation control function is only available in compute shaders for OGL,
            * or OCL */
        if (threadGroupSync)
        {
            gcmASSERT(stageIndex == gcvPROGRAM_STAGE_OPENCL ||
                      stageIndex == gcvPROGRAM_STAGE_COMPUTE ||
                      codeGen.clShader || codeGen.computeShader);
        }

        /* Set sample base offset. */
        hints->samplerBaseOffset[stageIndex] = _GetDefaultSamplerBaseOffset(codeGen.shaderType);
    }

    if (codeGen.shaderType == gcSHADER_TYPE_VERTEX)
    {
        hints->shaderMode = gcvSHADING_SMOOTH;
    }
    else
    {
        gctSIZE_T i;
        for (i = 0; i < tree->attributeCount; i++)
        {
            if (tree->shader->attributes[i] != gcvNULL && tree->attributeArray[i].inUse)
            {
                if (tree->shader->attributes[i]->componentShadeMode[0] == gcSHADER_SHADER_FLAT
                 || tree->shader->attributes[i]->componentShadeMode[1] == gcSHADER_SHADER_FLAT
                 || tree->shader->attributes[i]->componentShadeMode[2] == gcSHADER_SHADER_FLAT
                 || tree->shader->attributes[i]->componentShadeMode[3] == gcSHADER_SHADER_FLAT)
                {
                    hints->shaderMode = gcvSHADING_FLAT_OPENGL;
                    break;
                }
            }
        }
    }

    if (codeGen.clShader || codeGen.computeShader)
    {
        /* Get the order of attributes. */
        gcmONERROR(gcSHADER_QueryValueOrder(shader, &(hints->valueOrder)));
    }

    /* Fill the state buffer. */
    gcmONERROR(_GenerateStates(tree,
                               &codeGen,
                               stateBuffer + stateBufferSize,
                               &size,
                               (gctUINT8 *)stateDeltaBuffer + stateDeltaSize,
                               &size2,
                               hints));

    /* Free any old state buffer. */
    if (stateBufferSize > 0)
    {
        gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, ProgramState->stateBuffer));
    }

    if (stateDeltaSize)
    {
        gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, ProgramState->stateDelta));
    }

    /* Set new state buffer. */
    ProgramState->stateBuffer = stateBuffer;
    ProgramState->stateBufferSize = stateBufferSize + size;
    ProgramState->stateDelta = stateDeltaBuffer;
    ProgramState->stateDeltaSize = stateDeltaSize + size2;
    ProgramState->hints = hints;
    stateBuffer       = gcvNULL;

OnError:
    /* Free up uniform usage table. */
    if (codeGen.uniformUsage != gcvNULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, codeGen.uniformUsage));
    }

    /* Free up register usage table. */
    if (codeGen.registerUsage != gcvNULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, codeGen.registerUsage));
    }

    /* Free up the code tables. */
    if (codeGen.functions != gcvNULL)
    {
        gctSIZE_T i;

        for (i = 0; i <= shader->functionCount + shader->kernelFunctionCount; ++i)
        {
            while (codeGen.functions[i].root != gcvNULL)
            {
                gcsSL_PHYSICAL_CODE_PTR code = codeGen.functions[i].root;
                codeGen.functions[i].root = code->next;

                gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, code));
            }
        }

        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, codeGen.functions));
    }

    /* Free up the code mapping table. */
    if (codeGen.codeMap != gcvNULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, codeGen.codeMap));
    }

    /* Free up the constant table. */
    while (codeGen.constants != gcvNULL)
    {
        gcsSL_CONSTANT_TABLE_PTR constant = codeGen.constants;
        codeGen.constants = constant->next;

        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, constant));
    }

    /* Free up the state buffer. */
    if (stateBuffer != gcvNULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, stateBuffer));
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}
#endif

#if !DX_SHADER
gctBOOL
gcSHADER_CheckBugFixes10(
    void
    )
{
    gctBOOL hasBugFixes10;

    gcmHEADER();

    if (gcHWCaps.hwFeatureFlags.hasBugFix10)
    {
        hasBugFixes10 = gcvTRUE;
    }
    else
    {
        hasBugFixes10 = gcvFALSE;
    }

    gcmFOOTER_ARG("return=%d", hasBugFixes10);
    return hasBugFixes10;
}

#endif

#endif


