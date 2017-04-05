/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_vsc.h"

#if gcdENABLE_3D

static gctBOOL
_hasUniformType(
    IN gcUNIFORM Uniform
    )
{
    if (isUniformNormal(Uniform) ||
        isUniformBlockMember(Uniform) ||
        isUniformBlockAddress(Uniform) ||
        isUniformLodMinMax(Uniform) ||
        isUniformLevelBaseSize(Uniform) ||
        isUniformSampleLocation(Uniform) ||
        isUniformMultiSampleBuffers(Uniform))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gceSTATUS
_gcChangeAtomicCounterUniform2BaseAddrBindingUniform(
    IN gcSHADER      Shader
    )
{
    gctUINT     i      = 0;
    gceSTATUS   status = gcvSTATUS_OK;

    gcmHEADER_ARG("Shader=0x%x", Shader);

    gcmASSERT(Shader);

    for(i = 0; i < Shader->lastInstruction; ++i)
    {
        gctINT           j    = 0;

        for(j = 0; j < 2; ++j)
        {
            gctSOURCE_t              *source             = j == 0 ? &Shader->code[i].source0 : &Shader->code[i].source1;
            gctUINT16                *sourceIndex        = j == 0 ? &Shader->code[i].source0Index : &Shader->code[i].source1Index;
            gctUINT16                *sourceIndexed      = j == 0 ? &Shader->code[i].source0Indexed : &Shader->code[i].source1Indexed;
            gcSL_TYPE                 src_type;
            gcSL_FORMAT               src_format;
            gctINT                    index              = 0;
            gctUINT                   constIndex         = 0;
            gcUNIFORM                 uniform            = gcvNULL;
            gcUNIFORM                 bindingUniform     = gcvNULL;
            gctUINT16                 newRegNo[3];
            gctUINT                   offset             = 0;

            struct _gcSL_INSTRUCTION  loadInst, addInst, mulInst;

            src_type = gcmSL_SOURCE_GET(*source, Type);
            if(src_type != gcSL_UNIFORM)
            {
                continue;
            }

            index   = gcmSL_INDEX_GET(*sourceIndex, Index);
            uniform = Shader->uniforms[index];
            if (uniform == gcvNULL)
            {
                continue;
            }

            if(!_hasUniformType(uniform))
            {
                continue;
            }

            if (gcmType_Kind(uniform->u.type) != gceTK_ATOMIC)
            {
                continue;
            }

            if (Shader->code[i].opcode == gcSL_LOAD)
            {
                continue;
            }

            gcmASSERT(uniform->baseBindingIdx >= 0 && uniform->baseBindingIdx < (gctINT16)Shader->uniformCount);

            SetUniformFlag(uniform, gcvUNIFORM_FLAG_USED_IN_SHADER);
            bindingUniform = Shader->uniforms[uniform->baseBindingIdx];

            if (bindingUniform == gcvNULL)
            {
                gcmASSERT(gcvFALSE);
            }

            memset(&loadInst, 0, sizeof(loadInst));
            memset(&addInst, 0, sizeof(addInst));
            memset(&mulInst, 0, sizeof(mulInst));

            newRegNo[0] = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_UINT_X1);
            newRegNo[1] = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_UINT_X1);
            newRegNo[2] = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_UINT_X1);

            src_format = gcmSL_SOURCE_GET(*source, Format);
            if(gcmSL_SOURCE_GET(*source, Indexed) == gcSL_NOT_INDEXED)
            {
                /*
                     3: ATOMADD         temp.uint(4).x, uniform.uint(2 + 3).hp.y, 1

                     3: LOAD            temp.uint(61).x, uniform.uint(1).y, offset + 3
                     4: ATOMADD         temp.uint(4).x, temp.uint(61).x, 1
                */
                gcmSL_OPCODE_UPDATE(loadInst.opcode, Opcode, gcSL_LOAD);
                loadInst.temp        = gcmSL_TARGET_SET(0, Condition, gcSL_ALWAYS)
                                      | gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_X)
                                      | gcmSL_TARGET_SET(0, Precision, gcSHADER_PRECISION_HIGH)
                                      | gcmSL_TARGET_SET(0, Format, src_format);
                loadInst.tempIndex   = newRegNo[0];

                loadInst.source0     = *source;
                loadInst.source0     = gcmSL_SOURCE_SET(loadInst.source0, Indexed, gcSL_NOT_INDEXED);
                loadInst.source0     = gcmSL_SOURCE_SET(loadInst.source0, Precision, gcSHADER_PRECISION_HIGH);

                constIndex = gcmSL_INDEX_GET(*sourceIndex, ConstValue) + *sourceIndexed;
                offset     = uniform->offset + constIndex * 4;

                loadInst.source0Index = *sourceIndex;
                loadInst.source0Index = gcmSL_INDEX_SET(loadInst.source0Index, Index, uniform->baseBindingIdx);
                loadInst.source0Index = gcmSL_INDEX_SET(loadInst.source0Index, ConstValue, 0);

                loadInst.source1        = gcmSL_SOURCE_SET(0, Type, gcSL_CONSTANT)
                                        | gcmSL_SOURCE_SET(0, Format, gcSL_UINT32);
                loadInst.source1Index   = ((gctUINT16 *) &offset)[0];
                loadInst.source1Indexed = ((gctUINT16 *) &offset)[1];

                *source      = gcmSL_SOURCE_SET(*source, Type, gcSL_TEMP);
                *source      = gcmSL_SOURCE_SET(*source, Swizzle, gcSL_SWIZZLE_XXXX);
                *source      = gcmSL_SOURCE_SET(*source, Indexed, gcSL_NOT_INDEXED);
                *source      = gcmSL_SOURCE_SET(*source, Precision, gcSHADER_PRECISION_HIGH);

                *sourceIndex = gcmSL_INDEX_SET(*sourceIndex, Index, newRegNo[0]);
                *sourceIndex = gcmSL_INDEX_SET(*sourceIndex, ConstValue, 0);

                *sourceIndexed = 0;

                gcSHADER_InsertNOP2BeforeCode(Shader, i, 1, gcvTRUE, gcvTRUE);
                Shader->code[i] = loadInst;

                ++i;
            }
            else
            {
                /*
                     3: ATOMADD         temp.uint(4).x, uniform.uint(2 + temp(3).y + 3).hp.y, 1

                     2: MUL             temp.uint(60).x, temp(3).y, 4
                     3: ADD             temp.uint(61).x, temp(60).x, offset + 3
                     4: LOAD            temp.uint(62).x, uniform.uint(1).y, temp.uint(61).x
                     5: ATOMADD         temp.uint(4).x, temp.uint(62).x, 1
                */
                gcSL_SWIZZLE srcSwz = gcSL_SWIZZLE_XXXX;
                gctINT k;
                gcSHADER_PRECISION precision = gcSHADER_PRECISION_DEFAULT;

                for(k = (gctINT)(i - 1); k >= 0; k--)
                {
                    if(Shader->code[k].tempIndex == *sourceIndexed)
                    {
                        precision = gcmSL_TARGET_GET(Shader->code[k].temp, Precision);
                        break;
                    }
                }
                gcmASSERT(precision != gcSHADER_PRECISION_DEFAULT);

                constIndex = gcmSL_INDEX_GET(*sourceIndex, ConstValue);
                offset     = uniform->offset + constIndex * 4;

                gcmSL_OPCODE_UPDATE(mulInst.opcode, Opcode, gcSL_MUL);
                mulInst.temp        = gcmSL_TARGET_SET(0, Condition, gcSL_ALWAYS)
                                    | gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_X)
                                    | gcmSL_TARGET_SET(0, Format, gcSL_UINT32)
                                    | gcmSL_TARGET_SET(0, Precision, precision);
                mulInst.tempIndex   = newRegNo[2];

                switch (gcmSL_SOURCE_GET(*source, Indexed))
                {
                case gcSL_INDEXED_X:
                    srcSwz = gcSL_SWIZZLE_XXXX;
                    break;
                case gcSL_INDEXED_Y:
                    srcSwz = gcSL_SWIZZLE_YYYY;
                    break;
                case gcSL_INDEXED_Z:
                    srcSwz = gcSL_SWIZZLE_ZZZZ;
                    break;
                case gcSL_INDEXED_W:
                    srcSwz = gcSL_SWIZZLE_WWWW;
                    break;
                default:
                    gcmASSERT(0);
                    break;
                }

                mulInst.source0     = gcmSL_SOURCE_SET(mulInst.source0, Swizzle, srcSwz);
                mulInst.source0     = gcmSL_SOURCE_SET(mulInst.source0, Type, gcSL_TEMP);
                mulInst.source0     = gcmSL_SOURCE_SET(mulInst.source0, Indexed, gcSL_NOT_INDEXED);
                mulInst.source0     = gcmSL_SOURCE_SET(mulInst.source0, Precision, gcSHADER_PRECISION_DEFAULT);
                mulInst.source0     = gcmSL_SOURCE_SET(mulInst.source0, Format, gcSL_INTEGER);
                mulInst.source0     = gcmSL_SOURCE_SET(mulInst.source0, Precision, precision);

                mulInst.source0Index = *sourceIndexed;
                mulInst.source0Index = gcmSL_INDEX_SET(mulInst.source0Index, ConstValue, 0);

                mulInst.source1      = gcmSL_SOURCE_SET(0, Type, gcSL_CONSTANT)
                                     | gcmSL_SOURCE_SET(0, Format, gcSL_UINT32);
                mulInst.source1Index   = 4;
                mulInst.source1Indexed = 0;


                gcmSL_OPCODE_UPDATE(addInst.opcode, Opcode, gcSL_ADD);
                addInst.temp        = gcmSL_TARGET_SET(0, Condition, gcSL_ALWAYS)
                                    | gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_X)
                                    | gcmSL_TARGET_SET(0, Format, gcSL_UINT32)
                                    | gcmSL_TARGET_SET(0, Precision, precision);
                addInst.tempIndex   = newRegNo[0];


                addInst.source0     = gcmSL_SOURCE_SET(addInst.source0, Swizzle, gcSL_SWIZZLE_XXXX);
                addInst.source0     = gcmSL_SOURCE_SET(addInst.source0, Type, gcSL_TEMP);
                addInst.source0     = gcmSL_SOURCE_SET(addInst.source0, Indexed, gcSL_NOT_INDEXED);
                addInst.source0     = gcmSL_SOURCE_SET(addInst.source0, Precision, precision);
                addInst.source0     = gcmSL_SOURCE_SET(addInst.source0, Format, gcSL_INTEGER);

                addInst.source0Index = newRegNo[2];
                addInst.source0Index = gcmSL_INDEX_SET(addInst.source0Index, ConstValue, 0);

                addInst.source1      = gcmSL_SOURCE_SET(0, Type, gcSL_CONSTANT)
                                     | gcmSL_SOURCE_SET(0, Format, gcSL_UINT32);
                addInst.source1Index   = ((gctUINT16 *) &offset)[0];
                addInst.source1Indexed = ((gctUINT16 *) &offset)[1];


                gcmSL_OPCODE_UPDATE(loadInst.opcode, Opcode, gcSL_LOAD);
                loadInst.temp        = gcmSL_TARGET_SET(0, Condition, gcSL_ALWAYS)
                                     | gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_X)
                                     | gcmSL_TARGET_SET(0, Precision, gcSHADER_PRECISION_HIGH)
                                     | gcmSL_TARGET_SET(0, Format, src_format);
                loadInst.tempIndex   = newRegNo[1];

                loadInst.source0     = *source;
                loadInst.source0     = gcmSL_SOURCE_SET(loadInst.source0, Indexed, gcSL_NOT_INDEXED);
                loadInst.source0     = gcmSL_SOURCE_SET(loadInst.source0, Precision, gcSHADER_PRECISION_HIGH);

                loadInst.source0Index = *sourceIndex;
                loadInst.source0Index = gcmSL_INDEX_SET(loadInst.source0Index, Index, uniform->baseBindingIdx);
                loadInst.source0Index = gcmSL_INDEX_SET(loadInst.source0Index, ConstValue, 0);

                loadInst.source1     = gcmSL_SOURCE_SET(loadInst.source1, Swizzle, gcSL_SWIZZLE_XXXX);
                loadInst.source1     = gcmSL_SOURCE_SET(loadInst.source1, Type, gcSL_TEMP);
                loadInst.source1     = gcmSL_SOURCE_SET(loadInst.source1, Indexed, gcSL_NOT_INDEXED);
                loadInst.source1     = gcmSL_SOURCE_SET(loadInst.source1, Precision, precision);
                loadInst.source1     = gcmSL_SOURCE_SET(loadInst.source1, Format, gcSL_INTEGER);

                loadInst.source1Index   = newRegNo[0];

                *source      = gcmSL_SOURCE_SET(*source, Swizzle, gcSL_SWIZZLE_XXXX);
                *source     = gcmSL_SOURCE_SET(*source, Type, gcSL_TEMP);
                *source     = gcmSL_SOURCE_SET(*source, Indexed, gcSL_NOT_INDEXED);
                *source     = gcmSL_SOURCE_SET(*source, Precision, gcSHADER_PRECISION_HIGH);

                *sourceIndex = gcmSL_INDEX_SET(*sourceIndex, Index, newRegNo[1]);
                *sourceIndex = gcmSL_INDEX_SET(*sourceIndex, ConstValue, 0);

                *sourceIndexed = 0;

                gcSHADER_InsertNOP2BeforeCode(Shader, i, 3, gcvTRUE, gcvTRUE);

                Shader->code[i]     = mulInst;
                Shader->code[i + 1] = addInst;
                Shader->code[i + 2] = loadInst;

                i += 3;
            }
        }
    }

    gcmFOOTER();
    return status;
}

static gceSTATUS
_gcPreprocessHeplerInvocation(
    gcSHADER FragmentShader
    )
{
    gctUINT               i, j, f;
    gctINT                tempIdx;
    gceSTATUS             status = gcvSTATUS_OK;
    gctBOOL               hasHelperInvocation = gcvFALSE, bInMainRoutine;
    gcATTRIBUTE           attribHelperInvocation = gcvNULL, attrib;
    gctSOURCE_t           *source;
    gctUINT16             *sourceIndex;
    gctINT                attribIdx;
    gcFUNCTION            function;
    gcSL_INSTRUCTION      setCode;

    gcmHEADER_ARG("FragmentShader=0x%x", FragmentShader);

    gcmASSERT(FragmentShader);

    for (i = 0; i < FragmentShader->attributeCount; ++i)
    {
        /* PS varying may be deleted by optimization. */
        if (FragmentShader->attributes[i] &&
            FragmentShader->attributes[i]->nameLength == gcSL_HELPER_INVOCATION)
        {
            attribHelperInvocation = FragmentShader->attributes[i];
            break;
        }
    }

    if (attribHelperInvocation)
    {
        for (i = 0; i < FragmentShader->lastInstruction; ++i)
        {
            for (j = 0; j < 2; ++j)
            {
                source        = (j == 0) ? &FragmentShader->code[i].source0 : &FragmentShader->code[i].source1;
                sourceIndex   = (j == 0) ? &FragmentShader->code[i].source0Index : &FragmentShader->code[i].source1Index;

                if (gcmSL_SOURCE_GET(*source, Type) != gcSL_ATTRIBUTE)
                {
                    continue;
                }

                attribIdx = gcmSL_INDEX_GET(*sourceIndex, Index);

                if (attribHelperInvocation->index == attribIdx)
                {
                    hasHelperInvocation = gcvTRUE;
                    break;
                }
            }
        }
    }

    if (hasHelperInvocation)
    {
        for (i = 0; i < FragmentShader->lastInstruction; i ++)
        {
            bInMainRoutine = gcvTRUE;

            /* Determine ownership of the code for functions. */
            for (f = 0; f < FragmentShader->functionCount; ++f)
            {
                function = FragmentShader->functions[f];

                if ((i >= function->codeStart) &&
                    (i < function->codeStart + function->codeCount))
                {
                    bInMainRoutine = gcvFALSE;
                    break;
                }
            }

            if (bInMainRoutine)
            {
                break;
            }
        }

        gcmONERROR(gcSHADER_InsertNOP2BeforeCode(FragmentShader, i, 1, gcvTRUE, gcvTRUE));

        setCode = &FragmentShader->code[i];

        tempIdx = gcSHADER_NewTempRegs(FragmentShader, 1, gcSHADER_FLOAT_X1);

        if (gcHWCaps.hwFeatureFlags.supportHelperInv)
        {
            setCode->opcode = gcSL_SET;
            setCode->temp = gcmSL_TARGET_SET(0, Format, gcSL_BOOLEAN)  |
                            gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_X) |
                            gcmSL_TARGET_SET(0, Condition, gcSL_EQUAL);
            setCode->tempIndex = (gctUINT16)tempIdx;

            setCode->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_ATTRIBUTE)  |
                               gcmSL_SOURCE_SET(0, Format, gcSL_BOOLEAN)  |
                               gcmSL_SOURCE_SET(0, Precision, gcSHADER_PRECISION_MEDIUM)  |
                               gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XXXX);
            setCode->source0Index = gcmSL_INDEX_SET(0, Index, attribHelperInvocation->index);

            setCode->source1 = gcmSL_SOURCE_SET(0, Type, gcSL_CONSTANT) |
                               gcmSL_SOURCE_SET(0, Format, gcSL_BOOLEAN);
            setCode->source1Index = 1;
        }
        else
        {
            setCode->opcode = gcSL_MOV;
            setCode->temp = gcmSL_TARGET_SET(0, Format, gcSL_BOOLEAN)  |
                            gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_X);
            setCode->tempIndex = (gctUINT16)tempIdx;

            setCode->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_CONSTANT)  |
                               gcmSL_SOURCE_SET(0, Format, gcSL_BOOLEAN)  |
                               gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XXXX);
            setCode->source0Index = setCode->source0Indexed = 0;

            setCode->source1 = 0;
            setCode->source0Index = setCode->source0Indexed = 0;
        }

        for (i ++; i < FragmentShader->lastInstruction; ++i)
        {
            for (j = 0; j < 2; ++j)
            {
                source        = (j == 0) ? &FragmentShader->code[i].source0 : &FragmentShader->code[i].source1;
                sourceIndex   = (j == 0) ? &FragmentShader->code[i].source0Index : &FragmentShader->code[i].source1Index;

                if (gcmSL_SOURCE_GET(*source, Type) != gcSL_ATTRIBUTE)
                {
                    continue;
                }

                attribIdx = gcmSL_INDEX_GET(*sourceIndex, Index);
                attrib = FragmentShader->attributes[attribIdx];
                if (attrib == gcvNULL)
                {
                    continue;
                }

                gcmASSERT(attrib->index == attribIdx);

                if (attrib->nameLength != gcSL_HELPER_INVOCATION)
                {
                    continue;
                }

                *source = gcmSL_SOURCE_SET(*source, Type, gcSL_TEMP);
                *sourceIndex = (gctUINT16)tempIdx;
            }
        }
    }

OnError:
    gcmFOOTER();
    return status;
}

static gceSTATUS
_gcMovTexldModifier(
    IN gcSHADER Shader
    )
{
    gceSTATUS   status = gcvSTATUS_OK;
    gctUINT     i, j;

    for (i = 0; i < Shader->codeCount; i++)
    {
        gcSL_INSTRUCTION code = &Shader->code[i];
        gcSL_INSTRUCTION nextCode, newCode;
        gcSL_OPCODE      opcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(code->opcode, Opcode);

        if (!gcSL_isOpcodeTexldModifier(opcode))
        {
            continue;
        }

        for (j = i + 1; j < Shader->codeCount; j++)
        {
            nextCode = &Shader->code[j];
            opcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(nextCode->opcode, Opcode);

            if (!gcSL_isOpcodeTexld(opcode))
            {
                continue;
            }

            if (j == i + 1)
            {
                break;
            }

            gcmONERROR(gcSHADER_InsertNOP2BeforeCode(Shader, j, (gctUINT)1, gcvTRUE, gcvTRUE));
            newCode = &Shader->code[j];
            code = &Shader->code[i];
            gcoOS_MemCopy(newCode, code, gcmSIZEOF(struct _gcSL_INSTRUCTION));
            gcoOS_ZeroMemory(code, gcmSIZEOF(struct _gcSL_INSTRUCTION));
            break;
        }
    }

OnError:
    return status;
}

/* A special optimize for GLBM. */
static gceSTATUS
_gcConvertTEXLD2MOV(
    IN gcSHADER FragmentShader
    )
{
    gceSTATUS   status = gcvSTATUS_OK;
    gctINT      i, j;
    gcsValue    constValue;
    gcUNIFORM   constantTexel;

    /*
    ** Change
    **      MOV      r1, vec2(0, 0)
    **      TEXLD    r2, s1, r1
    ** -->
    **      MOV      r2, c1
    */
    for (i = 0; i < (gctINT)FragmentShader->codeCount; i++)
    {
        gcSL_INSTRUCTION code = &FragmentShader->code[i];
        gcSL_INSTRUCTION prevCode = gcvNULL, coordMovCode = gcvNULL;
        gctBOOL findDef = gcvFALSE;

        if (gcmSL_OPCODE_GET(code->opcode, Opcode) != gcSL_TEXLD ||
            gcmSL_SOURCE_GET(code->source1, Type) != gcSL_TEMP)
        {
            continue;
        }

        /* Find the coord MOV. */
        for (j = i - 1; j >= 0; j--)
        {
            prevCode = &FragmentShader->code[j];

            if (gcmSL_OPCODE_GET(prevCode->opcode, Opcode) != gcSL_MOV ||
                prevCode->tempIndex != code->source1Index)
            {
                continue;
            }

            /* Multi def, skip this TEXLD. */
            if (findDef)
            {
                coordMovCode = gcvNULL;
                break;
            }

            /* Find the match MOV. */
            if (gcmSL_SOURCE_GET(prevCode->source0, Type) == gcSL_CONSTANT &&
                prevCode->source0Index == 0 &&
                prevCode->source0Indexed == 0)
            {
                coordMovCode = prevCode;
            }
            findDef = gcvTRUE;
        }

        if (coordMovCode == gcvNULL)
        {
            continue;
        }

        /* Create the constant uniform. */
        constValue.f32_v4[0] = 0.3125f;
        constValue.f32_v4[1] = 0.546875f;
        constValue.f32_v4[2] = 0.96875f;
        constValue.f32_v4[3] = 0.0f;

        gcmONERROR(gcSHADER_CreateConstantUniform(FragmentShader,
                                                  gcSHADER_FLOAT_X4,
                                                  &constValue,
                                                  &constantTexel));

        /* Change TEXLD to MOV. */
        code->opcode = gcSL_MOV;

        /* Set src of MOV to this new uniform */
        code->source0 = gcmSL_SOURCE_SET(code->source0, Type, gcSL_UNIFORM);
        code->source0Index = constantTexel->index;

        /* 2nd src is now obsolete */
        code->source1 = gcmSL_SOURCE_SET(code->source1, Type, gcSL_NONE);
        code->source1Index = code->source1Indexed = 0;
    }

OnError:
    return status;
}

gceSTATUS
_gcConvSamplerAssignForParameter(
    IN gcSHADER Shader
    )
{
    gctUINT32   i;

    for (i = 0; i < Shader->codeCount; i++)
    {
        gcSL_INSTRUCTION code = &Shader->code[i];

        if (gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_SAMPLER_ASSIGN)
        {
            code->opcode = gcmSL_OPCODE_SET(code->opcode, Opcode, gcSL_MOV);
        }
        else if (gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_GET_SAMPLER_IDX)
        {
            if (gcmSL_SOURCE_GET(code->source0, Type) == gcSL_UNIFORM)
            {
                gcUNIFORM uniform = gcvNULL;

                gcSHADER_GetUniform(Shader, gcmSL_INDEX_GET(code->source0Index, Index), &uniform);

                if (isUniformBuffer(uniform))
                {
                    code->opcode = gcmSL_OPCODE_SET(code->opcode, Opcode, gcSL_MOV);
                }
            }
        }
        else
        {
            continue;
        }
    }

    return gcvSTATUS_OK;
}

static gctBOOL
_gcIsCodeMainFunction(
    IN gcSHADER             Shader,
    IN gctUINT              CodeIdx,
    OUT gcFUNCTION         *Function
    )
{
    gctUINT i;

    for (i = 0; i < Shader->functionCount; ++i)
    {
        if (CodeIdx >= Shader->functions[i]->codeStart &&
            CodeIdx < Shader->functions[i]->codeStart + Shader->functions[i]->codeCount)
        {
            if (Function)
            {
                *Function = Shader->functions[i];
            }
            return gcvFALSE;
        }
    }

    return gcvTRUE;
}

static gceSTATUS
_gcFixedJmpInMainFunc(
    IN gcSHADER             Shader
    )
{
    gceSTATUS               status = gcvSTATUS_OK;
    gctUINT                 i;

    for (i = 0; i < Shader->lastInstruction; i++)
    {
        gcSL_INSTRUCTION    code;
        gcFUNCTION          func = gcvNULL;

        code = &Shader->code[i];

        /* Skip non-jmp code. */
        if (gcmSL_OPCODE_GET(code->opcode, Opcode) != gcSL_JMP)
        {
            continue;
        }

        /* Skip not main function code. */
        if (!_gcIsCodeMainFunction(Shader, i, gcvNULL))
        {
            continue;
        }

        /* Skip if target is a main function code. */
        if (_gcIsCodeMainFunction(Shader, code->tempIndex, &func))
        {
            continue;
        }
        gcmASSERT(code->tempIndex == func->codeStart);

        /* Insert a NOP before the target code. */
        gcmONERROR(gcSHADER_InsertNOP2BeforeCode(Shader, code->tempIndex, 1, gcvTRUE, gcvFALSE));
    }

OnError:
    return status;
}

static gctBOOL
_gcFindMainFunctionCodeSegment(
    IN gcSHADER             Shader,
    IN gctUINT              StartIdx,
    IN gctUINT             *CodeSegmentStartIdx,
    IN gctUINT             *CodeSegmentEndIdx
    )
{
    gctBOOL                 findNewSegment = gcvFALSE;
    gctUINT                 startIdx = StartIdx;
    gctUINT                 endIdx = 0;
    gcFUNCTION              function = gcvNULL;

    while (startIdx < Shader->lastInstruction &&
           !_gcIsCodeMainFunction(Shader, startIdx, &function))
    {
        startIdx = function->codeStart + function->codeCount;
    }

    if (startIdx < Shader->lastInstruction)
    {
        findNewSegment = gcvTRUE;
        endIdx = startIdx;

        while (endIdx < Shader->lastInstruction && _gcIsCodeMainFunction(Shader, endIdx + 1, gcvNULL))
        {
            endIdx++;
        }

        if (CodeSegmentStartIdx)
        {
            *CodeSegmentStartIdx = startIdx;
        }

        if (CodeSegmentEndIdx)
        {
            *CodeSegmentEndIdx = endIdx;
        }
    }

    return findNewSegment;
}

/*
** The main function may be splited into several parts by sub-function, so make sure that the main function is in the bottom of a shader.
*/
static gceSTATUS
_gcPackMainFunction(
    IN gcSHADER             Shader
    )
{
    gceSTATUS               status = gcvSTATUS_OK;
    gctBOOL                 packMainFunction = gcvTRUE;
    gctUINT                 currentCodeSegmentStartIdx = 0, currentCodeSegmentEndIdx = 0;
    gctUINT                 newCodeSegmentStartIdx = 0, newCodeSegmentEndIdx = 0;

    /* Skip a shader if this shader has no function. */
    if (Shader->functionCount == 0 &&
        Shader->kernelFunctionCount == 0)
    {
        packMainFunction = gcvFALSE;
    }

    /* I: Pack main function to the bottom of this shader if needed. */
    if (packMainFunction)
    {
        /*
        ** The target of a JMP in main function may be the head of a normal function,
        ** if so, we need to insert a NOP before this normal function.
        */
        gcmONERROR(_gcFixedJmpInMainFunc(Shader));

        /* Find a main function code segment. */
        _gcFindMainFunctionCodeSegment(Shader, 0, &currentCodeSegmentStartIdx, &currentCodeSegmentEndIdx);

        while (((currentCodeSegmentEndIdx + 1) < Shader->lastInstruction)
               &&
               _gcFindMainFunctionCodeSegment(Shader,
                                              currentCodeSegmentEndIdx + 1,
                                              &newCodeSegmentStartIdx,
                                              &newCodeSegmentEndIdx))
        {
            gcmONERROR(gcSHADER_MoveCodeListBeforeCode(Shader,
                                                       newCodeSegmentStartIdx,
                                                       currentCodeSegmentStartIdx,
                                                       currentCodeSegmentEndIdx));

            currentCodeSegmentEndIdx = newCodeSegmentEndIdx + currentCodeSegmentEndIdx - currentCodeSegmentStartIdx + 1;
            currentCodeSegmentStartIdx = newCodeSegmentStartIdx;
        }

        /* Now main function should be a single code segment. */
        if (currentCodeSegmentEndIdx != Shader->lastInstruction)
        {
            gcmONERROR(gcSHADER_MoveCodeListBeforeCode(Shader,
                                                       Shader->lastInstruction,
                                                       currentCodeSegmentStartIdx,
                                                       currentCodeSegmentEndIdx));
        }
    }

    /* II: Add a RET after the last instruction of main function. */
    gcmONERROR(gcSHADER_AddOpcode(Shader, gcSL_RET, 0, 0, 0, 0, 0));
    Shader->instrIndex = gcSHADER_OPCODE;
    Shader->lastInstruction++;

OnError:
    return status;
}

static gceSTATUS
_gcRemoveNOPsInMainFunction(
    IN gcSHADER             Shader
    )
{
    gceSTATUS               status = gcvSTATUS_OK;
    gctUINT                 i, j;
    gctUINT                 firstNOPCodeIndex, lastNOPCodeIndex;
    gctUINT                 removeCodeCount;
    gctUINT                 leftCodeCount;
    gcSL_INSTRUCTION        code = gcvNULL;

    for (i = 0; i < Shader->lastInstruction; i++)
    {
        code = &Shader->code[i];

        /* Find the NOP code list. */
        if (gcmSL_OPCODE_GET(code->opcode, Opcode) != gcSL_NOP)
        {
            continue;
        }

        if (gcSHADER_GetFunctionByCodeId(Shader, i, gcvNULL) != (gctUINT)-1)
        {
            continue;
        }

        firstNOPCodeIndex = i;
        for (j = i + 1; j < Shader->lastInstruction; j++)
        {
            code = &Shader->code[j];
            if ((gcmSL_OPCODE_GET(code->opcode, Opcode) != gcSL_NOP) ||
                (gcSHADER_GetFunctionByCodeId(Shader, j, gcvNULL) != (gctUINT)-1))
            {
                break;
            }
        }
        j--;
        lastNOPCodeIndex = j;

        /* Remove the NOP code list. */
        removeCodeCount = lastNOPCodeIndex - firstNOPCodeIndex + 1;
        leftCodeCount = Shader->lastInstruction - lastNOPCodeIndex - 1;

        /* Copy the instructions. */
        if (lastNOPCodeIndex < (Shader->lastInstruction -1))
        {
            gcoOS_MemCopy(Shader->code + firstNOPCodeIndex,
                          Shader->code + lastNOPCodeIndex + 1,
                          leftCodeCount * gcmSIZEOF(struct _gcSL_INSTRUCTION));
        }

        /* Modify the last instruction. */
        Shader->lastInstruction -= removeCodeCount;
        gcoOS_ZeroMemory(Shader->code + Shader->lastInstruction,
                        (Shader->codeCount - Shader->lastInstruction) * gcmSIZEOF(struct _gcSL_INSTRUCTION));

        /* Modify the target of CALL/JMP. */
        for (i = 0; i < Shader->lastInstruction; i++)
        {
            gcSL_INSTRUCTION    code = &Shader->code[i];
            gcSL_OPCODE         opcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(code->opcode, Opcode);

            if (opcode != gcSL_CALL && opcode != gcSL_JMP)
            {
                continue;
            }

            /* Only if the target is below the NOP start, we need to update it. */
            if (code->tempIndex > lastNOPCodeIndex)
            {
                code->tempIndex -= (gctUINT16)removeCodeCount;
            }
            else if (code->tempIndex >= (gctUINT16)firstNOPCodeIndex && code->tempIndex <= (gctUINT16)lastNOPCodeIndex)
            {
                code->tempIndex = (gctUINT16)firstNOPCodeIndex;
            }
        }

        /* Modify the function code count. */
        for (i = 0; i < Shader->functionCount; i++)
        {
            gcFUNCTION          func = Shader->functions[i];

            if (func->codeStart > lastNOPCodeIndex)
            {
                func->codeStart -= removeCodeCount;
            }
        }

        /* Modify the kernel function code count. */
        for (i = 0; i < Shader->kernelFunctionCount; i++)
        {
            gcKERNEL_FUNCTION   func = Shader->kernelFunctions[i];

            if (func->codeStart > lastNOPCodeIndex)
            {
                func->codeStart -= removeCodeCount;
                func->codeEnd -= removeCodeCount;
            }
        }

        /* Move to next non-nop code. */
        i = firstNOPCodeIndex;
    }

    return status;
}

static gceSTATUS
_GetRtImage(
    IN gcSHADER Shader,
    IN gctINT RtCount,
    OUT gcUNIFORM *RtImage
    )
{
    gceSTATUS       status = gcvSTATUS_OK;
    gcUNIFORM       rtImage = gcvNULL;
    gcUNIFORM       extraLayerRtImage = gcvNULL;
    gctCHAR*        rtImageName = "#sh_rtImage";
    gceIMAGE_FORMAT rtImageFormat = gcIMAGE_FORMAT_RGBA8;

    gcmONERROR(gcSHADER_GetUniformByName(Shader,
                                         rtImageName,
                                         gcoOS_StrLen(rtImageName, gcvNULL),
                                         &rtImage));
    if (rtImage != gcvNULL)
    {
        if (RtImage)
        {
            *RtImage = rtImage;
        }
        return status;
    }

    gcmONERROR(gcSHADER_AddUniformEx1(Shader,
                                      rtImageName,
                                      gcSHADER_IMAGE_2D,
                                      gcSHADER_PRECISION_HIGH,
                                      -1,
                                      0,
                                      -1,
                                      RtCount > 1 ? 1 : 0,
                                      RtCount > 1 ? &RtCount : gcvNULL,
                                      gcSHADER_VAR_CATEGORY_NORMAL,
                                      0,
                                      -1,
                                      -1,
                                      rtImageFormat,
                                      gcvNULL,
                                      &rtImage));
    gcmASSERT(rtImage);
    SetUniformFlags(rtImage, gcvUNIFORM_FLAG_COMPILER_GEN);

    if (GetUniformImageFormat(rtImage) == gcIMAGE_FORMAT_RGBA32F)
    {
        gcmONERROR(gcSHADER_AddUniformEx1(Shader,
                                          "#sh_imageExtraLayer_rtImage",
                                          gcSHADER_IMAGE_2D,
                                          gcSHADER_PRECISION_HIGH,
                                          -1,
                                          0,
                                          -1,
                                          RtCount > 1 ? 1 : 0,
                                          RtCount > 1 ? &RtCount : gcvNULL,
                                          gcSHADER_VAR_CATEGORY_NORMAL,
                                          0,
                                          GetUniformIndex(rtImage),
                                          -1,
                                          gcIMAGE_FORMAT_RGBA32F,
                                          gcvNULL,
                                          &extraLayerRtImage));

        SetUniformKind(extraLayerRtImage, gcvUNIFORM_KIND_IMAGE_EXTRA_LAYER);
        SetUniformFlag(extraLayerRtImage, gcvUNIFORM_FLAG_COMPILER_GEN);
        gcUNIFORM_SetFormat(extraLayerRtImage, gcIMAGE_FORMAT_RGBA32F, gcvFALSE);
    }

    if (RtImage)
    {
        *RtImage = rtImage;
    }

OnError:
    return status;
}

static gceSTATUS
_GetImageLoadFunc(
    IN gcSHADER Shader,
    OUT gcFUNCTION * Function
    )
{
    gceSTATUS       status = gcvSTATUS_OK;
    gcFUNCTION      imageLoadFunc = gcvNULL;
    gctCHAR*        imageLoadFuncName = "_viv_image_load_image_2d";
    gcsSHADER_VAR_INFO varInfo;
    gctINT16        variableIndex;
    gctUINT16       newTempIndex;

    gcmONERROR(gcSHADER_GetFunctionByName(Shader,
                                          imageLoadFuncName,
                                          &imageLoadFunc));
    if (imageLoadFunc == gcvNULL)
    {
        /* Add function/ */
        gcmONERROR(gcSHADER_AddFunction(Shader,
                                        imageLoadFuncName,
                                        &imageLoadFunc));
        gcmASSERT(imageLoadFunc);

        SetShaderHasIntrinsicBuiltin(Shader, gcvTRUE);
        SetFunctionFlags(imageLoadFunc, gcvFUNC_INTRINSICS);
        SetFunctionIntrinsicsKind(imageLoadFunc, gceINTRIN_image_load);

        /* Add image input. */
        newTempIndex = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_UINT_X4);
        varInfo.varCategory             = gcSHADER_VAR_CATEGORY_FUNCTION_INPUT_ARGUMENT;
        varInfo.type                    = gcSHADER_IMAGE_2D;
        varInfo.firstChild              = -1;
        varInfo.nextSibling             = -1;
        varInfo.prevSibling             = -1;
        varInfo.parent                  = -1;
        varInfo.u.numBlockElement       = 0;
        varInfo.u.numStructureElement   = 0;
        varInfo.precision               = gcSHADER_PRECISION_HIGH;
        varInfo.location                = -1;
        varInfo.binding                 = -1;
        varInfo.offset                  = 0;
        varInfo.isArray                 = gcvFALSE;
        varInfo.isLocal                 = gcvTRUE;
        varInfo.isOutput                = gcvFALSE;
        varInfo.isPrecise               = gcvFALSE;
        varInfo.isPerVertex             = gcvFALSE;
        varInfo.isPointer               = gcvFALSE;
        varInfo.arraySize               = 1;
        varInfo.arrayCount              = 0;
        varInfo.arraySizeList           = gcvNULL;
        varInfo.format                  = gcSL_UINT32;
        varInfo.imageFormat             = gcIMAGE_FORMAT_RGBA32F;
        gcmONERROR(gcSHADER_AddVariableEx1(Shader, "_viv_image_load_image_2d_input", newTempIndex, &varInfo, &variableIndex));
        gcmONERROR(gcFUNCTION_AddArgument(imageLoadFunc,
                                          (gctUINT16)variableIndex,
                                          newTempIndex,
                                          gcSL_ENABLE_XYZW,
                                          gcvFUNCTION_INPUT,
                                          gcSHADER_PRECISION_HIGH,
                                          gcvFALSE));

        /* Add coord input. */
        newTempIndex = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_INTEGER_X2);
        varInfo.varCategory             = gcSHADER_VAR_CATEGORY_FUNCTION_INPUT_ARGUMENT;
        varInfo.type                    = gcSHADER_INTEGER_X2;
        varInfo.firstChild              = -1;
        varInfo.nextSibling             = -1;
        varInfo.prevSibling             = -1;
        varInfo.parent                  = -1;
        varInfo.u.numBlockElement       = 0;
        varInfo.u.numStructureElement   = 0;
        varInfo.precision               = gcSHADER_PRECISION_HIGH;
        varInfo.location                = -1;
        varInfo.binding                 = -1;
        varInfo.offset                  = 0;
        varInfo.isArray                 = gcvFALSE;
        varInfo.isLocal                 = gcvTRUE;
        varInfo.isOutput                = gcvFALSE;
        varInfo.isPrecise               = gcvFALSE;
        varInfo.isPerVertex             = gcvFALSE;
        varInfo.isPointer               = gcvFALSE;
        varInfo.arraySize               = 1;
        varInfo.arrayCount              = 0;
        varInfo.arraySizeList           = gcvNULL;
        varInfo.format                  = gcSL_INT32;
        varInfo.imageFormat             = gcIMAGE_FORMAT_DEFAULT;
        gcmONERROR(gcSHADER_AddVariableEx1(Shader, "_viv_image_load_image_2d_input", newTempIndex, &varInfo, &variableIndex));
        gcmONERROR(gcFUNCTION_AddArgument(imageLoadFunc,
                                          (gctUINT16)variableIndex,
                                          newTempIndex,
                                          gcSL_ENABLE_XY,
                                          gcvFUNCTION_INPUT,
                                          gcSHADER_PRECISION_HIGH,
                                          gcvFALSE));

        /* Add output. */
        newTempIndex = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_FLOAT_X4);
        varInfo.varCategory             = gcSHADER_VAR_CATEGORY_FUNCTION_OUTPUT_ARGUMENT;
        varInfo.type                    = gcSHADER_FLOAT_X4;
        varInfo.firstChild              = -1;
        varInfo.nextSibling             = -1;
        varInfo.prevSibling             = -1;
        varInfo.parent                  = -1;
        varInfo.u.numBlockElement       = 0;
        varInfo.u.numStructureElement   = 0;
        varInfo.precision               = gcSHADER_PRECISION_HIGH;
        varInfo.location                = -1;
        varInfo.binding                 = -1;
        varInfo.offset                  = 0;
        varInfo.isArray                 = gcvFALSE;
        varInfo.isLocal                 = gcvTRUE;
        varInfo.isOutput                = gcvFALSE;
        varInfo.isPrecise               = gcvFALSE;
        varInfo.isPerVertex             = gcvFALSE;
        varInfo.isPointer               = gcvFALSE;
        varInfo.arraySize               = 1;
        varInfo.arrayCount              = 0;
        varInfo.arraySizeList           = gcvNULL;
        varInfo.format                  = gcSL_FLOAT;
        varInfo.imageFormat             = gcIMAGE_FORMAT_DEFAULT;
        gcmONERROR(gcSHADER_AddVariableEx1(Shader, "_viv_image_load_image_2d_output", newTempIndex, &varInfo, &variableIndex));
        gcmONERROR(gcFUNCTION_AddArgument(imageLoadFunc,
                                          (gctUINT16)variableIndex,
                                          newTempIndex,
                                          gcSL_ENABLE_XYZW,
                                          gcvFUNCTION_OUTPUT,
                                          gcSHADER_PRECISION_HIGH,
                                          gcvFALSE));
    }

    if (Function)
    {
        *Function = imageLoadFunc;
    }

OnError:
    return status;
}

static gceSTATUS
_GetImageStoreFunc(
    IN gcSHADER Shader,
    OUT gcFUNCTION * Function
    )
{
    gceSTATUS       status = gcvSTATUS_OK;
    gcFUNCTION      imageStoreFunc = gcvNULL;
    gctCHAR*        imageStoreFuncName = "_viv_image_store_image_2d";
    gcsSHADER_VAR_INFO varInfo;
    gctINT16        variableIndex;
    gctUINT16       newTempIndex;

    gcmONERROR(gcSHADER_GetFunctionByName(Shader,
                                          imageStoreFuncName,
                                          &imageStoreFunc));
    if (imageStoreFunc == gcvNULL)
    {
        /* Add function/ */
        gcmONERROR(gcSHADER_AddFunction(Shader,
                                        imageStoreFuncName,
                                        &imageStoreFunc));
        gcmASSERT(imageStoreFunc);

        SetShaderHasIntrinsicBuiltin(Shader, gcvTRUE);
        SetFunctionFlags(imageStoreFunc, gcvFUNC_INTRINSICS);
        SetFunctionIntrinsicsKind(imageStoreFunc, gceINTRIN_image_store);

        /* Add image input. */
        newTempIndex = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_UINT_X4);
        varInfo.varCategory             = gcSHADER_VAR_CATEGORY_FUNCTION_INPUT_ARGUMENT;
        varInfo.type                    = gcSHADER_IMAGE_2D;
        varInfo.firstChild              = -1;
        varInfo.nextSibling             = -1;
        varInfo.prevSibling             = -1;
        varInfo.parent                  = -1;
        varInfo.u.numBlockElement       = 0;
        varInfo.u.numStructureElement   = 0;
        varInfo.precision               = gcSHADER_PRECISION_HIGH;
        varInfo.location                = -1;
        varInfo.binding                 = -1;
        varInfo.offset                  = 0;
        varInfo.isArray                 = gcvFALSE;
        varInfo.isLocal                 = gcvTRUE;
        varInfo.isOutput                = gcvFALSE;
        varInfo.isPrecise               = gcvFALSE;
        varInfo.isPerVertex             = gcvFALSE;
        varInfo.isPointer               = gcvFALSE;
        varInfo.arraySize               = 1;
        varInfo.arrayCount              = 0;
        varInfo.arraySizeList           = gcvNULL;
        varInfo.format                  = gcSL_UINT32;
        varInfo.imageFormat             = gcIMAGE_FORMAT_RGBA32F;
        gcmONERROR(gcSHADER_AddVariableEx1(Shader, "_viv_image_store_image_2d_input", newTempIndex, &varInfo, &variableIndex));
        gcmONERROR(gcFUNCTION_AddArgument(imageStoreFunc,
                                          (gctUINT16)variableIndex,
                                          newTempIndex,
                                          gcSL_ENABLE_XYZW,
                                          gcvFUNCTION_INPUT,
                                          gcSHADER_PRECISION_HIGH,
                                          gcvFALSE));

        /* Add coord input. */
        newTempIndex = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_INTEGER_X2);
        varInfo.varCategory             = gcSHADER_VAR_CATEGORY_FUNCTION_INPUT_ARGUMENT;
        varInfo.type                    = gcSHADER_INTEGER_X2;
        varInfo.firstChild              = -1;
        varInfo.nextSibling             = -1;
        varInfo.prevSibling             = -1;
        varInfo.parent                  = -1;
        varInfo.u.numBlockElement       = 0;
        varInfo.u.numStructureElement   = 0;
        varInfo.precision               = gcSHADER_PRECISION_HIGH;
        varInfo.location                = -1;
        varInfo.binding                 = -1;
        varInfo.offset                  = 0;
        varInfo.isArray                 = gcvFALSE;
        varInfo.isLocal                 = gcvTRUE;
        varInfo.isOutput                = gcvFALSE;
        varInfo.isPrecise               = gcvFALSE;
        varInfo.isPerVertex             = gcvFALSE;
        varInfo.isPointer               = gcvFALSE;
        varInfo.arraySize               = 1;
        varInfo.arrayCount              = 0;
        varInfo.arraySizeList           = gcvNULL;
        varInfo.format                  = gcSL_INT32;
        varInfo.imageFormat             = gcIMAGE_FORMAT_DEFAULT;
        gcmONERROR(gcSHADER_AddVariableEx1(Shader, "_viv_image_store_image_2d_input", newTempIndex, &varInfo, &variableIndex));
        gcmONERROR(gcFUNCTION_AddArgument(imageStoreFunc,
                                          (gctUINT16)variableIndex,
                                          newTempIndex,
                                          gcSL_ENABLE_XY,
                                          gcvFUNCTION_INPUT,
                                          gcSHADER_PRECISION_HIGH,
                                          gcvFALSE));

        /* Add data. */
        newTempIndex = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_FLOAT_X4);
        varInfo.varCategory             = gcSHADER_VAR_CATEGORY_FUNCTION_INPUT_ARGUMENT;
        varInfo.type                    = gcSHADER_FLOAT_X4;
        varInfo.firstChild              = -1;
        varInfo.nextSibling             = -1;
        varInfo.prevSibling             = -1;
        varInfo.parent                  = -1;
        varInfo.u.numBlockElement       = 0;
        varInfo.u.numStructureElement   = 0;
        varInfo.precision               = gcSHADER_PRECISION_HIGH;
        varInfo.location                = -1;
        varInfo.binding                 = -1;
        varInfo.offset                  = 0;
        varInfo.isArray                 = gcvFALSE;
        varInfo.isLocal                 = gcvTRUE;
        varInfo.isOutput                = gcvFALSE;
        varInfo.isPrecise               = gcvFALSE;
        varInfo.isPerVertex             = gcvFALSE;
        varInfo.isPointer               = gcvFALSE;
        varInfo.arraySize               = 1;
        varInfo.arrayCount              = 0;
        varInfo.arraySizeList           = gcvNULL;
        varInfo.format                  = gcSL_FLOAT;
        varInfo.imageFormat             = gcIMAGE_FORMAT_DEFAULT;
        gcmONERROR(gcSHADER_AddVariableEx1(Shader, "_viv_image_store_image_2d_input", newTempIndex, &varInfo, &variableIndex));
        gcmONERROR(gcFUNCTION_AddArgument(imageStoreFunc,
                                          (gctUINT16)variableIndex,
                                          newTempIndex,
                                          gcSL_ENABLE_XYZW,
                                          gcvFUNCTION_INPUT,
                                          gcSHADER_PRECISION_HIGH,
                                          gcvFALSE));
    }

    if (Function)
    {
        *Function = imageStoreFunc;
    }

OnError:
    return status;
}

/*
** Load gl_LastFragData:
** -->
** F2I, temp(1).int.xy, gl_FragCoord
** call temp(2)[MaxDrawBuffers].xy = _viv_image_load_image_2d(rtImage[MaxDrawBuffers], temp(1).int.xy)
** replace gl_LastFragData with temp(2)[MaxDrawBuffers]
*/
static gceSTATUS
_UpdateLastFragData(
    IN gcSHADER Shader
    )
{
    gceSTATUS       status = gcvSTATUS_OK;
    gcATTRIBUTE     lastFragData = gcvNULL;
    gcsSHADER_VAR_INFO varInfo;
    gctCHAR*        newLastFragDataName = "#sh_LastFragData";
    gctINT16        newLastFragDataIndex;
    gcVARIABLE      newLastFragDataVar = gcvNULL;
    gcATTRIBUTE     fragCoord = gcvNULL;
    gcOUTPUT        output = gcvNULL;
    gcUNIFORM       rtImage = gcvNULL;
    gcFUNCTION      imageLoadFunc = gcvNULL, imageStoreFunc = gcvNULL;
    gcSHADER_LABEL  shaderLabel;
    gctINT          rtUsedCount = 0, rtCount = 0, rtImageIndex = 0;
    gctINT          mainStartIndex = 0, mainEndIndex, i, j;
    gctUINT         origlastInst = Shader->lastInstruction;
    gctUINT         codeCount;
    gctUINT16       coordTempIndex, newLastFragDataTempIndex;
    gctINT          outputCodeCount = 0;
    gctINT          maxOutputLocation = 32;
    gctINT          minLocation = maxOutputLocation, maxLocation = 0;
    gctINT          outputLocationMapping[32];

    gcmASSERT(GetShaderType(Shader) == gcSHADER_TYPE_FRAGMENT);

    /* Find gl_LastFragData, if not found, just return. */
    gcmONERROR(gcSHADER_GetAttributeByName(Shader,
                                           gcvNULL,
                                           (gctUINT)gcSL_LAST_FRAG_DATA,
                                           &lastFragData));
    if (!lastFragData)
    {
        return status;
    }

    /* Set flag. */
    Shader->useLastFragData = gcvTRUE;

    /* Update gl_LastFragData by using a image if hardware can't support it. */
    if (!gcHWCaps.hwFeatureFlags.hasPSIOInterlock)
    {
        return status;
    }

    /* Get the rt size. */
    rtCount = GetATTRArraySize(lastFragData);

    /* Find gl_FragCoord, if not found, create it. */
    gcmONERROR(gcSHADER_GetAttributeByName(Shader,
                                           gcvNULL,
                                           (gctUINT)gcSL_POSITION,
                                           &fragCoord));
    if (!fragCoord)
    {
        gcmONERROR(gcSHADER_AddAttributeWithLocation(Shader,
                                                     "#Position",
                                                     gcSHADER_FLOAT_X4,
                                                     gcSHADER_PRECISION_HIGH,
                                                     1,
                                                     0,
                                                     gcvFALSE,
                                                     gcSHADER_SHADER_DEFAULT,
                                                     -1,
                                                     -1,
                                                     gcvFALSE,
                                                     gcvFALSE,
                                                     &fragCoord));
        gcmASSERT(fragCoord);
        gcmATTRIBUTE_SetIsPosition(fragCoord, gcvTRUE);
        gcmATTRIBUTE_SetEnabled(fragCoord, gcvTRUE);
    }

    /* Find #rtImage, if not found, create it. */
    gcmONERROR(_GetRtImage(Shader, rtCount, &rtImage));
    gcmASSERT(rtImage);

    /* Create a new temp register to save the coord. */
    coordTempIndex = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_INTEGER_X2);

    /* Create a global variable to save the gl_LastFragData.
    ** We can't just use a temp index to save this because it may be dynamic indexed.
    */
    newLastFragDataTempIndex = (gctUINT16)gcSHADER_NewTempRegs(Shader, rtCount, gcSHADER_FLOAT_X4);

    varInfo.varCategory             = gcSHADER_VAR_CATEGORY_NORMAL;
    varInfo.type                    = gcSHADER_FLOAT_X4;
    varInfo.firstChild              = -1;
    varInfo.nextSibling             = -1;
    varInfo.prevSibling             = -1;
    varInfo.parent                  = -1;
    varInfo.u.numBlockElement       = 0;
    varInfo.u.numStructureElement   = 0;
    varInfo.precision               = gcSHADER_PRECISION_HIGH;
    varInfo.location                = -1;
    varInfo.binding                 = -1;
    varInfo.offset                  = 0;
    varInfo.isArray                 = rtCount > 1;
    varInfo.isLocal                 = gcvFALSE;
    varInfo.isOutput                = gcvFALSE;
    varInfo.isPrecise               = gcvFALSE;
    varInfo.isPerVertex             = gcvFALSE;
    varInfo.isPointer               = gcvFALSE;
    varInfo.arraySize               = rtCount;
    varInfo.arrayCount              = rtCount > 1;
    varInfo.arraySizeList           = rtCount > 1 ? &rtCount : gcvNULL;
    varInfo.format                  = gcSL_FLOAT;
    varInfo.imageFormat             = gcIMAGE_FORMAT_DEFAULT;

    gcmONERROR(gcSHADER_AddVariableEx1(Shader,
                                       newLastFragDataName,
                                       newLastFragDataTempIndex,
                                       &varInfo,
                                       &newLastFragDataIndex));
    gcmONERROR(gcSHADER_GetVariable(Shader, (gctUINT)newLastFragDataIndex, &newLastFragDataVar));
    SetVariableIsLocal(newLastFragDataVar);

    /*--------------------------Begin to load the data---------------------*/
    /* Get the function "imageLoad", if not found, create one. */
    gcmONERROR(_GetImageLoadFunc(Shader,
                                 &imageLoadFunc));

    gcmASSERT(imageLoadFunc);

    /* Check how may elements of gl_LastFragData[] are used.. */
    for (i = 0; i < (gctINT)Shader->lastInstruction; i++)
    {
        gcSL_INSTRUCTION code = &Shader->code[i];

        if (!code) continue;

        if (gcmSL_SOURCE_GET(code->source0, Type) == gcSL_ATTRIBUTE &&
            gcmSL_INDEX_GET(code->source0Index, Index) == GetATTRIndex(lastFragData))
        {
            if (gcmSL_SOURCE_GET(code->source0, Indexed) == gcSL_NOT_INDEXED)
            {
                rtUsedCount = gcmMAX(rtUsedCount, gcmSL_INDEX_GET(code->source0Index, ConstValue) + 1);
            }
            else
            {
                rtUsedCount = rtCount;
                break;
            }
        }

        if (gcmSL_SOURCE_GET(code->source1, Type) == gcSL_ATTRIBUTE &&
            gcmSL_INDEX_GET(code->source1Index, Index) == GetATTRIndex(lastFragData))
        {
            if (gcmSL_SOURCE_GET(code->source1, Indexed) == gcSL_NOT_INDEXED)
            {
                rtUsedCount = gcmMAX(rtUsedCount, gcmSL_INDEX_GET(code->source1Index, ConstValue) + 1);
            }
            else
            {
                rtUsedCount = rtCount;
                break;
            }
        }
    }

    /* Insert NOPs. */
    codeCount = 1 + rtUsedCount * 4;
    gcmONERROR(gcSHADER_FindMainFunction(Shader, &mainStartIndex, gcvNULL));
    gcmONERROR(gcSHADER_InsertNOP2BeforeCode(Shader, mainStartIndex, (gctUINT)codeCount, gcvTRUE, gcvTRUE));
    Shader->lastInstruction = mainStartIndex;
    Shader->instrIndex = gcSHADER_OPCODE;

    /* Insert a F2I. */
    gcmONERROR(gcSHADER_AddOpcodeIndexedWithPrecision(Shader,
                                                      gcSL_F2I,
                                                      coordTempIndex,
                                                      gcSL_ENABLE_XY,
                                                      gcSL_NOT_INDEXED,
                                                      0,
                                                      gcSL_INT32,
                                                      gcSHADER_PRECISION_HIGH,
                                                      0));
    gcmONERROR(gcSHADER_AddSourceAttributeIndexedFormattedWithPrecision(Shader,
                                                                        fragCoord,
                                                                        gcSL_SWIZZLE_XYYY,
                                                                        0,
                                                                        gcSL_NOT_INDEXED,
                                                                        0,
                                                                        gcSL_FLOAT,
                                                                        gcSHADER_PRECISION_HIGH));

    for (i = 0; i < rtUsedCount; i++)
    {
        /* Mov parameter image. */
        gcmONERROR(gcSHADER_AddOpcodeIndexedWithPrecision(Shader,
                                                          gcSL_MOV,
                                                          imageLoadFunc->arguments[0].index,
                                                          gcSL_ENABLE_XYZW,
                                                          gcSL_NOT_INDEXED,
                                                          0,
                                                          gcSL_UINT32,
                                                          gcSHADER_PRECISION_HIGH,
                                                          0));
        gcmONERROR(gcSHADER_AddSourceUniformIndexedFormattedWithPrecision(Shader,
                                                                          rtImage,
                                                                          gcSL_SWIZZLE_XYZW,
                                                                          i,
                                                                          gcSL_NOT_INDEXED,
                                                                          gcSL_NONE_INDEXED,
                                                                          0,
                                                                          gcSL_UINT32,
                                                                          gcSHADER_PRECISION_HIGH));

        /* Mov parameter coord. */
        gcmONERROR(gcSHADER_AddOpcodeIndexedWithPrecision(Shader,
                                                          gcSL_MOV,
                                                          imageLoadFunc->arguments[1].index,
                                                          gcSL_ENABLE_XY,
                                                          gcSL_NOT_INDEXED,
                                                          0,
                                                          gcSL_INT32,
                                                          gcSHADER_PRECISION_HIGH,
                                                          0));
        gcmONERROR(gcSHADER_AddSourceIndexedWithPrecision(Shader,
                                                          gcSL_TEMP,
                                                          coordTempIndex,
                                                          gcSL_SWIZZLE_XYYY,
                                                          gcSL_NOT_INDEXED,
                                                          0,
                                                          gcSL_INT32,
                                                          gcSHADER_PRECISION_HIGH));
        /* Call function. */
        gcmONERROR(gcSHADER_AddOpcodeConditional(Shader,
                                                 gcSL_CALL,
                                                 gcSL_ALWAYS,
                                                 imageLoadFunc->label,
                                                 0));

        if (gcSHADER_FindLabel(Shader, imageLoadFunc->label, &shaderLabel))
        {
            shaderLabel->function = imageLoadFunc;
        }

        /* Mov output. */
        gcmONERROR(gcSHADER_AddOpcodeIndexedWithPrecision(Shader,
                                                          gcSL_MOV,
                                                          newLastFragDataTempIndex + (gctUINT16)i,
                                                          gcSL_ENABLE_XYZW,
                                                          gcSL_NOT_INDEXED,
                                                          0,
                                                          gcSL_FLOAT,
                                                          gcSHADER_PRECISION_HIGH,
                                                          0));
        gcmONERROR(gcSHADER_AddSourceIndexedWithPrecision(Shader,
                                                          gcSL_TEMP,
                                                          imageLoadFunc->arguments[2].index,
                                                          gcSL_SWIZZLE_XYZW,
                                                          gcSL_NOT_INDEXED,
                                                          0,
                                                          gcSL_FLOAT,
                                                          gcSHADER_PRECISION_HIGH));
    }
    Shader->lastInstruction = origlastInst + codeCount;

    /* Replace all users of gl_LastFragData. */
    for (i = 0; i < (gctINT)Shader->lastInstruction; i++)
    {
        gcSL_INSTRUCTION code = &Shader->code[i];

        if (!code) continue;

        if (gcmSL_SOURCE_GET(code->source0, Type) == gcSL_ATTRIBUTE &&
            gcmSL_INDEX_GET(code->source0Index, Index) == GetATTRIndex(lastFragData))
        {
            code->source0 = gcmSL_SOURCE_SET(code->source0, Type, gcSL_TEMP);
            code->source0Index = newLastFragDataTempIndex + gcmSL_INDEX_GET(code->source0Index, ConstValue);
        }

        if (gcmSL_SOURCE_GET(code->source1, Type) == gcSL_ATTRIBUTE &&
            gcmSL_INDEX_GET(code->source1Index, Index) == GetATTRIndex(lastFragData))
        {
            code->source1 = gcmSL_SOURCE_SET(code->source1, Type, gcSL_TEMP);
            code->source1Index = newLastFragDataTempIndex + gcmSL_INDEX_GET(code->source1Index, ConstValue);
        }
    }

    /* Pack shader. */
    gcmONERROR(gcSHADER_Pack(Shader));

    /*--------------------------Begin to store the data---------------------*/
    origlastInst = Shader->lastInstruction;

    /* Get the function "imageStore", if not found, create one. */
    gcmONERROR(_GetImageStoreFunc(Shader,
                                  &imageStoreFunc));

    gcmASSERT(imageStoreFunc);

    if (Shader->outputCount == 0)
    {
        return status;
    }

    gcoOS_MemFill(outputLocationMapping, 0xFF, maxOutputLocation * gcmSIZEOF(gctINT));

    /* Find the output location. */
    for (i = 0; i < (gctINT)Shader->outputCount; i++)
    {
        output = Shader->outputs[i];
        if (!output)
        {
            continue;
        }
        gcmASSERT(GetOutputLocation(output) >= 0 && GetOutputLocation(output) < maxOutputLocation);

        outputLocationMapping[GetOutputLocation(output)] = i;
        outputCodeCount += GetOutputArraySize(output);
        minLocation = gcmMIN(minLocation, GetOutputLocation(output));
        maxLocation = gcmMAX(maxLocation, GetOutputLocation(output));
    }

    if (outputCodeCount == 0)
    {
        return status;
    }

    /* Insert NOPs. */
    codeCount = outputCodeCount * 4;
    gcmONERROR(gcSHADER_FindMainFunction(Shader, gcvNULL, &mainEndIndex));
    gcmONERROR(gcSHADER_InsertNOP2BeforeCode(Shader, mainEndIndex, (gctUINT)codeCount, gcvTRUE, gcvTRUE));
    Shader->lastInstruction = mainEndIndex;
    Shader->instrIndex = gcSHADER_OPCODE;

    for (i = minLocation; i <= maxLocation; i++)
    {
        if (outputLocationMapping[i] == -1)
        {
            continue;
        }

        output = Shader->outputs[outputLocationMapping[i]];

        for (j = 0; j < GetOutputArraySize(output); j++)
        {
            /* Mov parameter image. */
            gcmONERROR(gcSHADER_AddOpcodeIndexedWithPrecision(Shader,
                                                              gcSL_MOV,
                                                              imageStoreFunc->arguments[0].index,
                                                              gcSL_ENABLE_XYZW,
                                                              gcSL_NOT_INDEXED,
                                                              0,
                                                              gcSL_UINT32,
                                                              gcSHADER_PRECISION_HIGH,
                                                              0));
            gcmONERROR(gcSHADER_AddSourceUniformIndexedFormattedWithPrecision(Shader,
                                                                              rtImage,
                                                                              gcSL_SWIZZLE_XYZW,
                                                                              rtImageIndex,
                                                                              gcSL_NOT_INDEXED,
                                                                              gcSL_NONE_INDEXED,
                                                                              0,
                                                                              gcSL_UINT32,
                                                                              gcSHADER_PRECISION_HIGH));

            /* Mov parameter coord. */
            gcmONERROR(gcSHADER_AddOpcodeIndexedWithPrecision(Shader,
                                                              gcSL_MOV,
                                                              imageStoreFunc->arguments[1].index,
                                                              gcSL_ENABLE_XY,
                                                              gcSL_NOT_INDEXED,
                                                              0,
                                                              gcSL_INT32,
                                                              gcSHADER_PRECISION_HIGH,
                                                              0));
            gcmONERROR(gcSHADER_AddSourceIndexedWithPrecision(Shader,
                                                              gcSL_TEMP,
                                                              coordTempIndex,
                                                              gcSL_SWIZZLE_XYYY,
                                                              gcSL_NOT_INDEXED,
                                                              0,
                                                              gcSL_INT32,
                                                              gcSHADER_PRECISION_HIGH));

            /* Mov output data. */
            gcmONERROR(gcSHADER_AddOpcodeIndexedWithPrecision(Shader,
                                                              gcSL_MOV,
                                                              imageStoreFunc->arguments[2].index,
                                                              gcSL_ENABLE_XYZW,
                                                              gcSL_NOT_INDEXED,
                                                              0,
                                                              gcSL_FLOAT,
                                                              gcSHADER_PRECISION_HIGH,
                                                              0));
            gcmONERROR(gcSHADER_AddSourceOutputIndexedFormattedWithPrecision(Shader,
                                                                             output,
                                                                             gcSL_SWIZZLE_XYZW,
                                                                             j,
                                                                             gcSL_NOT_INDEXED,
                                                                             0,
                                                                             gcSL_FLOAT,
                                                                             gcSHADER_PRECISION_HIGH));
            /* Call function. */
            gcmONERROR(gcSHADER_AddOpcodeConditional(Shader,
                                                     gcSL_CALL,
                                                     gcSL_ALWAYS,
                                                     imageStoreFunc->label,
                                                     0));

            if (gcSHADER_FindLabel(Shader, imageStoreFunc->label, &shaderLabel))
            {
                shaderLabel->function = imageStoreFunc;
            }
            rtImageIndex++;
        }
    }

    Shader->lastInstruction = origlastInst + codeCount;
    /* Pack shader. */
    gcmONERROR(gcSHADER_Pack(Shader));

OnError:
    return status;
}

static gceSTATUS
_PreprocessLinkBuiltinLibs(
    IN gcSHADER Shader
    )
{
    gceSTATUS       status = gcvSTATUS_OK;

    /* Update gl_LastFragData by using a image if hardware can't support it. */
    if (GetShaderType(Shader) == gcSHADER_TYPE_FRAGMENT)
    {
        gcmONERROR(_UpdateLastFragData(Shader));
    }

OnError:
    return status;
}

gceSTATUS
_gcLinkBuiltinLibs(
    IN gcSHADER* Shaders
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT   i;

    for (i = 0; i < gcMAX_SHADERS_IN_LINK_GOURP; i ++)
    {
        if (!Shaders[i])
        {
            continue;
        }
#if DX_SHADER
        if (GetShaderHasIntrinsicBuiltin(Shaders[i]))
        {
            gcSHADER libBinary = gcvNULL;

            gcmONERROR(gcSHADER_CompileBuiltinLibrary(
                                    Shaders[i],
                                    gcSHADER_TYPE_LIBRARY,
                                    gcLIB_DX_BUILTIN,
                                    &libBinary));

            gcmONERROR(gcSHADER_LinkBuiltinLibrary(
                                    Shaders[i],
                                    libBinary,
                                    gcLIB_DX_BUILTIN));

            /* after linking successfully, reset the flag */
            SetShaderHasIntrinsicBuiltin(Shaders[i], gcvFALSE);
        }
#else
        /* Do some works which use built-in functions. */
        gcmONERROR(_PreprocessLinkBuiltinLibs(Shaders[i]));

        if (GetShaderNeedPatchForCentroid(Shaders[i]))
        {
            status = gcSHADER_PatchCentroidVaryingAsCenter(Shaders[i]);

            if (gcmIS_ERROR(status))
            {
                return status;
            }

            /* after patch successfully, reset the flag */
            SetShaderNeedPatchForCentroid(Shaders[i], gcvFALSE);
        }

        if (GetShaderHasIntrinsicBuiltin(Shaders[i]))
        {
            gcSHADER libBinary = gcvNULL;
            gcLibType libType = gcLIB_BUILTIN;

            if (Shaders[i]->type == gcSHADER_TYPE_CL)
            {
                libType = gcLIB_CL_BUILTIN;
                status = gcSHADER_CompileCLBuiltinLibrary(Shaders[i],
                                                          gcSHADER_TYPE_LIBRARY,
                                                          libType,
                                                          &libBinary);
                if (gcmIS_ERROR(status))
                {
                    return status;
                }
            }
            else
            {
                status = gcSHADER_CompileBuiltinLibrary(Shaders[i],
                                                        gcSHADER_TYPE_LIBRARY,
                                                        libType,
                                                        &libBinary);
                if (gcmIS_ERROR(status))
                {
                    return status;
                }
            }

            status = gcSHADER_LinkBuiltinLibrary(Shaders[i],
                                                 libBinary,
                                                 libType);
            if (gcmIS_ERROR(status))
            {
                return status;
            }

            /* after linking successfully, reset the flag */
            SetShaderHasIntrinsicBuiltin(Shaders[i], gcvFALSE);
        }
#endif

        if (gceLAYOUT_QUALIFIER_HasHWNotSupportingBlendMode(GetShaderOutputBlends(Shaders[i])))
        {
            gcSHADER libBinary = gcvNULL;

            status = gcSHADER_CompileBuiltinLibrary(
                                    Shaders[i],
                                    GetShaderType(Shaders[i]),
                                    gcLIB_BLEND_EQUATION,
                                    &libBinary);

            if (gcmIS_ERROR(status))
            {
                return status;
            }

            status = gcSHADER_LinkBuiltinLibrary(
                                    Shaders[i],
                                    libBinary,
                                    gcLIB_BLEND_EQUATION);

            if (gcmIS_ERROR(status))
            {
                return status;
            }

            /* after linking successfully, reset the flag */
            ClearShaderOutputBlends(Shaders[i]);
        }

        /* After link all built-in functions, analyze them. */
        gcSHADER_AnalyzeFunctions(Shaders[i], gcvFALSE);
    }

OnError:
    return status;
}

gctBOOL
gceLAYOUT_QUALIFIER_HasHWNotSupportingBlendMode(
    IN gceLAYOUT_QUALIFIER Qualifier
    )
{
    if (gcHWCaps.hwFeatureFlags.supportAdvBlendPart0)
    {
        return Qualifier & gcvLAYOUT_QUALIFIER_BLEND_HW_UNSUPPORT_EQUATIONS_PART0;
    }
    else
    {
        return Qualifier & gcvLAYOUT_QUALIFIER_BLEND_HW_UNSUPPORT_EQUATIONS_ALL;
    }
}

static gceSTATUS
_CreateTransformFeedbackStateUniform(
    IN  gcSHADER       VertexShader,
    OUT gcUNIFORM *    UniformPtr
    )
{
    gceSTATUS             status = gcvSTATUS_OK;
    gcUNIFORM             uniform;
    gctCHAR               name[32];
    gctUINT               offset;

    /* construct TransformFeedbackStateUniform name */
    offset = 0;
    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(name,
                           gcmSIZEOF(name),
                           &offset,
                           "#__TransformFeedbackState%d", /* private (internal)
                                                           * uniform starts with '#' */
                           VertexShader->_id));

    gcmONERROR(gcSHADER_AddUniform(VertexShader, name,
                                   gcSHADER_INTEGER_X1,
                                   1 /* Length */,
                                   gcSHADER_PRECISION_HIGH,
                                   &uniform));
    SetUniformFlag(uniform, gcvUNIFORM_KIND_TRANSFORM_FEEDBACK_STATE);
    SetUniformFlag(uniform, gcvUNIFORM_FLAG_COMPILER_GEN);
    *UniformPtr = uniform;
OnError:
    return status;
}

static gctINT
_getFeedbackOutputSize(
    IN gcSHADER_TYPE      type
    )
{
    switch (type) {
    case gcSHADER_FLOAT_X1:
        return 4;
    case gcSHADER_BOOLEAN_X1:
    case gcSHADER_INTEGER_X1:
    case gcSHADER_UINT_X1:
        return 4;  /* size of int */
    default:
        gcmASSERT(gcvFALSE);
        break;
    }
    return 0;
}

gctSIZE_T
gcSHADER_GetVarTempRegInfo(
    IN gcSHADER              Shader,
    IN gcOUTPUT              Varying,
    IN gctBOOL               IsArray,
    OUT gcsVarTempRegInfo *  VarTempRegInfo,
    OUT gctSIZE_T *          Size
    )
{
    gceSTATUS          status = gcvSTATUS_OK;
    gctINT             sz = 0;
    gctSIZE_T          totalSize = 0;
    gctINT             components;
    gctINT             rows;
    gcSHADER_TYPE      componentType;
    gctINT             componentOutputSize;  /* the feedback output size is
                                                different than component size */
    gcSHADER_TYPE *    tempTypeArray;
    gctUINT            bytes;
    gcSHADER_TYPE      tempType    = Varying->type;

    /* only one element for each output */
    bytes = sizeof(gcSHADER_TYPE);
    /* allocate the temp register type array */
    gcmONERROR(gcoOS_Allocate(gcvNULL,
                              bytes,
                              (gctPOINTER *) &tempTypeArray));

    rows        = gcmType_Rows(tempType);
    components  = gcmType_Comonents(tempType);
    gcmASSERT(tempType != gcSHADER_UNKONWN_TYPE &&
                 components > 0 && components <= 4);
    componentType = gcmType_ComonentType(tempType);
    componentOutputSize = _getFeedbackOutputSize(componentType);
    sz = rows * components * componentOutputSize;
    if (IsArray)
    {
        sz *= Varying->arraySize;
    }
    gcmASSERT(sz > 0);

    VarTempRegInfo->varying         = Varying;
    VarTempRegInfo->tempRegTypes    = tempTypeArray;
    VarTempRegInfo->tempRegCount    = rows;
    VarTempRegInfo->isArray         = IsArray;
    VarTempRegInfo->streamoutSize   = 0;

    VarTempRegInfo->streamoutSize += sz;
    totalSize += sz;

    *Size = totalSize;

OnError:
    return status;
}


gceSTATUS
gcSHADER_ComputeTotalFeedbackVaryingsSize(
    IN gcSHADER Shader
    )
{
    gceSTATUS              status       = gcvSTATUS_OK;
    gctSIZE_T              totalSize    = 0;
    gcsTRANSFORM_FEEDBACK *xfbk         = &Shader->transformFeedback;
    gctINT                 i;
    const gctINT           varyingCount = xfbk->varyingCount;
    gcsVarTempRegInfo *    varRegInfos  = gcvNULL;

    if(varyingCount == 0 || xfbk->varRegInfos != gcvNULL)
    {
        return gcvSTATUS_OK;
    }

    gcmONERROR(gcoOS_Allocate(gcvNULL,
                              varyingCount * sizeof(gcsVarTempRegInfo),
                              (gctPOINTER *) &varRegInfos));
    gcoOS_ZeroMemory(varRegInfos, varyingCount * sizeof(gcsVarTempRegInfo));
    xfbk->varRegInfos = varRegInfos;
    for (i = 0; i < varyingCount; i++)
    {
        gctSIZE_T sz = 0;
        gcSHADER_GetVarTempRegInfo(Shader,
                                   xfbk->varyings[i].output,
                                   xfbk->varyings[i].isArray,
                                   &varRegInfos[i],
                                   &sz);
        totalSize += sz;
    }
    xfbk->totalSize = totalSize;

OnError:
    return status;
}

static gceSTATUS
_CreatePatchUniform(
    IN  gcSHADER         Shader,
    IN  gctCONST_STRING  BaseName,
    IN  gctINT           Index,
    IN  gcSHADER_TYPE    type,
    IN  gceUNIFORM_FLAGS uFlag,
    IN  gctBOOL          AppendId2Name,
    IN  gcSHADER_PRECISION Precision,
    OUT gcUNIFORM *      UniformPtr
    )
{
    gceSTATUS             status = gcvSTATUS_OK;
    gcUNIFORM             uniform = gcvNULL;
    gctCHAR               name[512];
    gctUINT               offset, i;
    gctUINT32             uniformNameLength;
    gctCONST_STRING       uniformName;

    /* construct name */
    offset = 0;

    if (AppendId2Name)
    {
        if (Index != -1)
        {
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(name,
                                   gcmSIZEOF(name),
                                   &offset,
                                   "#%s%d_%d", /* private (internal)
                                                  * uniform starts with '#' */
                                   BaseName,
                                   Shader->_id,
                                   Index));
        }
        else
        {
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(name,
                                   gcmSIZEOF(name),
                                   &offset,
                                   "#%s%d", /* private (internal)
                                               * uniform starts with '#' */
                                   BaseName,
                                   Shader->_id));
        }
    }
    else
    {
        gcmVERIFY_OK(
                gcoOS_PrintStrSafe(name,
                                   gcmSIZEOF(name),
                                   &offset,
                                   "#%s", /* private (internal)
                                             * uniform starts with '#' */
                                   BaseName));
    }

    for (i = 0; i < Shader->uniformCount; i ++)
    {
        if(!Shader->uniforms[i]) continue;

        gcmVERIFY_OK(gcUNIFORM_GetName(Shader->uniforms[i],
                                       &uniformNameLength, &uniformName));

        if (uniformNameLength == (gctUINT32) gcoOS_StrLen(name, gcvNULL))
        {
            if (gcoOS_MemCmp(name, uniformName, uniformNameLength) == 0)
            {
                uniform = Shader->uniforms[i];

                break;
            }
        }
    }

    if (uniform == gcvNULL)
    {
        gcmONERROR(gcSHADER_AddUniform(Shader, name,
                                       type,
                                       1 /* Length */,
                                       Precision,
                                       &uniform));
        SetUniformFlag(uniform, uFlag);
    }
    else
    {
        gcmASSERT(UniformHasFlag(uniform, uFlag));
        gcmASSERT(uniform->u.type == type);
    }

    *UniformPtr = uniform;

OnError:
    return status;
}

gcSL_FORMAT
gcGetFormatFromType(
    IN gcSHADER_TYPE Type
    )
{
    gcSL_FORMAT format = gcSL_FLOAT;

    switch (Type)
    {
    case gcSHADER_FLOAT_X1:
    case gcSHADER_FLOAT_X2:
    case gcSHADER_FLOAT_X3:
    case gcSHADER_FLOAT_X4:
    case gcSHADER_FLOAT_X8:
    case gcSHADER_FLOAT_X16:
    case gcSHADER_FLOAT_4X2:
    case gcSHADER_FLOAT_4X3:
    case gcSHADER_FLOAT_4X4:
    case gcSHADER_FLOAT_2X2:
    case gcSHADER_FLOAT_2X3:
    case gcSHADER_FLOAT_2X4:
    case gcSHADER_FLOAT_3X2:
    case gcSHADER_FLOAT_3X3:
    case gcSHADER_FLOAT_3X4:
        format = gcSL_FLOAT;
        break;

    case gcSHADER_INTEGER_X1:
    case gcSHADER_INTEGER_X2:
    case gcSHADER_INTEGER_X3:
    case gcSHADER_INTEGER_X4:
    case gcSHADER_INTEGER_X8:
    case gcSHADER_INTEGER_X16:
        format = gcSL_INTEGER;
        break;

    case gcSHADER_UINT_X1:
    case gcSHADER_UINT_X2:
    case gcSHADER_UINT_X3:
    case gcSHADER_UINT_X4:
    case gcSHADER_UINT_X8:
    case gcSHADER_UINT_X16:
        format = gcSL_UINT32;
        break;

    case gcSHADER_BOOLEAN_X1:
    case gcSHADER_BOOLEAN_X2:
    case gcSHADER_BOOLEAN_X3:
    case gcSHADER_BOOLEAN_X4:
        format = gcSL_BOOLEAN;
        break;

    case gcSHADER_INT64_X1:
    case gcSHADER_INT64_X2:
    case gcSHADER_INT64_X3:
    case gcSHADER_INT64_X4:
    case gcSHADER_INT64_X8:
    case gcSHADER_INT64_X16:
        format = gcSL_INT64;
        break;

    case gcSHADER_UINT64_X1:
    case gcSHADER_UINT64_X2:
    case gcSHADER_UINT64_X3:
    case gcSHADER_UINT64_X4:
    case gcSHADER_UINT64_X8:
    case gcSHADER_UINT64_X16:
        format = gcSL_UINT64;
        break;

    case gcSHADER_SAMPLER_1D:
    case gcSHADER_SAMPLER_2D:
    case gcSHADER_SAMPLER_3D:
    case gcSHADER_SAMPLER_BUFFER:
    case gcSHADER_SAMPLER_2D_SHADOW:
    case gcSHADER_SAMPLER_CUBE_SHADOW:
    case gcSHADER_SAMPLER_CUBIC:
    case gcSHADER_SAMPLER_CUBEMAP_ARRAY:
    case gcSHADER_SAMPLER_CUBEMAP_ARRAY_SHADOW:
    case gcSHADER_SAMPLER_EXTERNAL_OES:
    case gcSHADER_SAMPLER_1D_ARRAY:
    case gcSHADER_SAMPLER_1D_ARRAY_SHADOW:
    case gcSHADER_SAMPLER_2D_ARRAY:
    case gcSHADER_SAMPLER_2D_ARRAY_SHADOW:
        format = gcSL_FLOAT;
        break;

    case gcSHADER_ISAMPLER_2D:
    case gcSHADER_ISAMPLER_3D:
    case gcSHADER_ISAMPLER_BUFFER:
    case gcSHADER_ISAMPLER_CUBIC:
    case gcSHADER_ISAMPLER_CUBEMAP_ARRAY:
    case gcSHADER_ISAMPLER_2D_ARRAY:
        format = gcSL_INTEGER;
        break;

    case gcSHADER_USAMPLER_2D:
    case gcSHADER_USAMPLER_3D:
    case gcSHADER_USAMPLER_BUFFER:
    case gcSHADER_USAMPLER_CUBIC:
    case gcSHADER_USAMPLER_CUBEMAP_ARRAY:
    case gcSHADER_USAMPLER_2D_ARRAY:
        format = gcSL_UINT32;
        break;

    case gcSHADER_SAMPLER_2D_MS:
    case gcSHADER_SAMPLER_2D_MS_ARRAY:
        format = gcSL_FLOAT;
        break;

    case gcSHADER_ISAMPLER_2D_MS:
    case gcSHADER_ISAMPLER_2D_MS_ARRAY:
        format = gcSL_INTEGER;
        break;

    case gcSHADER_USAMPLER_2D_MS:
    case gcSHADER_USAMPLER_2D_MS_ARRAY:
        format = gcSL_UINT32;
        break;

    case gcSHADER_IIMAGE_2D:                     /* 0x39 */
    case gcSHADER_IIMAGE_3D:                     /* 0x3B */
    case gcSHADER_IIMAGE_CUBE:                   /* 0x3E */
    case gcSHADER_IIMAGE_2D_ARRAY:               /* 0x41 */
    case gcSHADER_IIMAGE_CUBEMAP_ARRAY:          /* 0x52 */
    case gcSHADER_IIMAGE_BUFFER:                 /* 0x5A */
        format = gcSL_INTEGER;
        break;


    case gcSHADER_UIMAGE_2D:                     /* 0x3A */
    case gcSHADER_UIMAGE_3D:                     /* 0x3C */
    case gcSHADER_UIMAGE_CUBE:                   /* 0x3F */
    case gcSHADER_UIMAGE_2D_ARRAY:               /* 0x42 */
    case gcSHADER_UIMAGE_CUBEMAP_ARRAY:          /* 0x53 */
    case gcSHADER_UIMAGE_BUFFER:                 /* 0x5B */
        format = gcSL_UINT32;
        break;

    case gcSHADER_IMAGE_2D:                      /* 0x17 */
    case gcSHADER_IMAGE_3D:                      /* 0x18 */
    case gcSHADER_IMAGE_CUBE:                    /* 0x3D */
    case gcSHADER_IMAGE_CUBEMAP_ARRAY:           /* 0x50 */
    case gcSHADER_IMAGE_2D_ARRAY:                /* 0x40 */
    case gcSHADER_IMAGE_1D:                      /* 0x44 */
    case gcSHADER_IMAGE_1D_ARRAY:                /* 0x45 */
    case gcSHADER_IMAGE_1D_BUFFER:               /* 0x46 */
    case gcSHADER_IMAGE_BUFFER:                  /* 0x59 */
        format = gcSL_FLOAT;
        break;

    case gcSHADER_SAMPLER:
        format = gcSL_UINT32;
        break;

    case gcSHADER_ATOMIC_UINT:
        format = gcSL_UINT32;
        break;

    /* float16 */
    case gcSHADER_FLOAT16_X1:      /* half2 */
    case gcSHADER_FLOAT16_X2:      /* half2 */
    case gcSHADER_FLOAT16_X3:      /* half3 */
    case gcSHADER_FLOAT16_X4:      /* half4 */
    case gcSHADER_FLOAT16_X8:      /* half8 */
    case gcSHADER_FLOAT16_X16:     /* half16 */
    case gcSHADER_FLOAT16_X32:     /* half32 */
        format = gcSL_FLOAT16;
        break;

    /*  boolean  */
    case gcSHADER_BOOLEAN_X8:
    case gcSHADER_BOOLEAN_X16:
    case gcSHADER_BOOLEAN_X32:
        format = gcSL_BOOLEAN;
        break;

    /* uchar vectors  */
    case gcSHADER_UINT8_X1:
    case gcSHADER_UINT8_X2:
    case gcSHADER_UINT8_X3:
    case gcSHADER_UINT8_X4:
    case gcSHADER_UINT8_X8:
    case gcSHADER_UINT8_X16:
    case gcSHADER_UINT8_X32:
        format = gcSL_UINT8;
        break;

    /* char vectors  */
    case gcSHADER_INT8_X1:
    case gcSHADER_INT8_X2:
    case gcSHADER_INT8_X3:
    case gcSHADER_INT8_X4:
    case gcSHADER_INT8_X8:
    case gcSHADER_INT8_X16:
    case gcSHADER_INT8_X32:
        format = gcSL_INT8;
        break;

    /* ushort vectors */
    case gcSHADER_UINT16_X1:
    case gcSHADER_UINT16_X2:
    case gcSHADER_UINT16_X3:
    case gcSHADER_UINT16_X4:
    case gcSHADER_UINT16_X8:
    case gcSHADER_UINT16_X16:
    case gcSHADER_UINT16_X32:
        format = gcSL_UINT16;
        break;

    /* short vectors */
    case gcSHADER_INT16_X1:
    case gcSHADER_INT16_X2:
    case gcSHADER_INT16_X3:
    case gcSHADER_INT16_X4:
    case gcSHADER_INT16_X8:
    case gcSHADER_INT16_X16:
    case gcSHADER_INT16_X32:
        format = gcSL_INT16;
        break;

    /* packed data type */
    /* packed float16 (2 bytes per element) */
    case gcSHADER_FLOAT16_P2:      /* half2 */
    case gcSHADER_FLOAT16_P3:      /* half3 */
    case gcSHADER_FLOAT16_P4:      /* half4 */
    case gcSHADER_FLOAT16_P8:      /* half8 */
    case gcSHADER_FLOAT16_P16:     /* half16 */
    case gcSHADER_FLOAT16_P32:     /* half32 */
        format = gcSL_FLOAT16;
        break;

    /* packed boolean (1 byte per element) */
    case gcSHADER_BOOLEAN_P2:    /* bool2, bvec2 */
    case gcSHADER_BOOLEAN_P3:
    case gcSHADER_BOOLEAN_P4:
    case gcSHADER_BOOLEAN_P8:
    case gcSHADER_BOOLEAN_P16:
    case gcSHADER_BOOLEAN_P32:
        format = gcSL_BOOLEAN;
        break;

    /* uchar vectors (1 byte per element) */
    case gcSHADER_UINT8_P2:
    case gcSHADER_UINT8_P3:
    case gcSHADER_UINT8_P4:
    case gcSHADER_UINT8_P8:
    case gcSHADER_UINT8_P16:
    case gcSHADER_UINT8_P32:
        format = gcSL_UINT8;
        break;

    /* char vectors (1 byte per element) */
    case gcSHADER_INT8_P2:
    case gcSHADER_INT8_P3:
    case gcSHADER_INT8_P4:
    case gcSHADER_INT8_P8:
    case gcSHADER_INT8_P16:
    case gcSHADER_INT8_P32:
        format = gcSL_INT8;
        break;

    /* ushort vectors (2 bytes per element) */
    case gcSHADER_UINT16_P2:
    case gcSHADER_UINT16_P3:
    case gcSHADER_UINT16_P4:
    case gcSHADER_UINT16_P8:
    case gcSHADER_UINT16_P16:
    case gcSHADER_UINT16_P32:
        format = gcSL_UINT16;
        break;

    /* short vectors (2 bytes per element) */
    case gcSHADER_INT16_P2:
    case gcSHADER_INT16_P3:
    case gcSHADER_INT16_P4:
    case gcSHADER_INT16_P8:
    case gcSHADER_INT16_P16:
    case gcSHADER_INT16_P32:
        format = gcSL_INT16;
        break;

    default:
        gcmASSERT(0);
    }
    return format;
}

/* generate transform feedback write to base address at Offset for the Varying,
   if BufferBaseAddress is not NULL, then the base address is in the uniform,
   otherwise the base address is in temp register BufferBaseAddressTempIndex,
   return wrote bytes in Size */
gceSTATUS
_generateFeedbackWrite(
    IN gcSHADER            VertexShader,
    IN gcsVarTempRegInfo * VaryingRegInfo,
    IN gcUNIFORM           BufferBaseAddress,
    IN gctINT              BufferBaseAddressTempIndex,
    IN gctINT              Offset,
    OUT gctSIZE_T *        Size)
{
    gceSTATUS     status = gcvSTATUS_OK;
    gctINT        currentOffset = Offset;
    gctUINT       startIndex;
    gctINT        i, j;
    gcSHADER      shader = VertexShader;
    gcOUTPUT      varying = gcvNULL;
    static const gcSL_ENABLE component2Enable[] = {
             gcSL_ENABLE_X,
             gcSL_ENABLE_XY,
             gcSL_ENABLE_XYZ,
             gcSL_ENABLE_XYZW
    };

    gcmASSERT(Offset >= 0 && VaryingRegInfo && VaryingRegInfo->varying != gcvNULL);

    /* Get the first element of an array. */
    gcmONERROR(gcSHADER_GetOutputByTempIndex(VertexShader,
                                             VaryingRegInfo->varying->tempIndex,
                                             &varying));

    for (j = 0; j < varying->arraySize; j++)
    {
        gcmONERROR(gcSHADER_GetOutputByTempIndex(VertexShader,
                                                 VaryingRegInfo->varying->tempIndex + (gctUINT16)(j * VaryingRegInfo->tempRegCount),
                                                 &varying));

        for (i = 0; i < VaryingRegInfo->tempRegCount; i ++)
        {
            gcSHADER_TYPE tempType    = varying->type;
            gctINT        components  = gcmType_Comonents(tempType);
            gcSL_FORMAT   format      = gcGetFormatFromType(tempType);
            gcSHADER_PRECISION precision = varying->precision;
            gcSL_ENABLE   enable;

            startIndex = varying->tempIndex;

            gcmASSERT(tempType != gcSHADER_UNKONWN_TYPE &&
                      components > 0 && components <= 4);
            enable = component2Enable[components - 1];
            /* write one temp register at a time

                 src0.x <= BufferBaseAddress
                 src1.x <= currentOffset
                 dest   <= temp[i]
                 store temp[i], src1.x, src2

                 currentOffset += byteSize(tempType);
             */
            gcSHADER_AddOpcode(shader, gcSL_STORE,
                               (gctINT16)(startIndex+i),
                               enable, format, precision, 0);
            if (BufferBaseAddress != gcvNULL)
            {
                gcSHADER_AddSourceUniformFormatted(shader, BufferBaseAddress,
                                                   gcSL_SWIZZLE_XXXX,
                                                   gcSL_NOT_INDEXED,
                                                   gcSL_INTEGER);
            }
            else
            {
                gcSHADER_AddSource(shader, gcSL_TEMP,
                                   (gctUINT16)BufferBaseAddressTempIndex,
                                   gcSL_SWIZZLE_XXXX, gcSL_INTEGER, gcSHADER_PRECISION_HIGH);
            }
            gcSHADER_AddSourceConstantFormatted(shader, (void *)&currentOffset,
                                                gcSL_INTEGER);

            /* adjust offset */
            currentOffset += components * gcmType_ComponentByteSize;
        }

        if (!VaryingRegInfo->isArray)
            break;
    }

    *Size = currentOffset - Offset;

OnError:
    return status;
}

static gctINT
_getUnusedTempIndex(
    IN gcSHADER Shader,
    IN gctINT NumTemp
    )
{
    return gcSHADER_NewTempRegs(Shader, NumTemp, gcSHADER_FLOAT_X1);
}

/* find the temp register assigned to gl_vertexID variable */
static gcVARIABLE
_findVexterInstIDTemp(
    IN gcSHADER Shader,
    IN gceBuiltinNameKind vtxOrInstIdName
    )
{
    gcVARIABLE    vertexInstIDVariable  = gcvNULL;
    gctUINT       i;

    gcmASSERT(Shader->type == gcSHADER_TYPE_VERTEX || Shader->type == gcSHADER_TYPE_TES);
    for (i = 0; i < Shader->variableCount; i++)
    {
        gcVARIABLE variable = Shader->variables[i];

        /* attributes could be null if it is not used */
        if (variable == gcvNULL)
            continue;
        if (variable->nameLength == (gctINT)vtxOrInstIdName)
        {
            vertexInstIDVariable = variable;
        }
    }

    if (vertexInstIDVariable == gcvNULL)
    {
        gctINT vertexInstIDTemp = _getUnusedTempIndex(Shader, 1);
        gctINT16 varIndex = -1;
        gctINT size = 1;
        gcSHADER_TYPE  vertexInstIDType = gcHWCaps.hwFeatureFlags.vtxInstanceIdAsInteger?
                                            gcSHADER_INTEGER_X1 : gcSHADER_FLOAT_X1;

        gcSHADER_AddVariableEx(Shader,
                               (vtxOrInstIdName == gcSL_VERTEX_ID) ? "#VertexID" : "#InstanceID",
                               vertexInstIDType,
                               1,
                               &size,
                               (gctUINT16)vertexInstIDTemp,
                               gcSHADER_VAR_CATEGORY_NORMAL,
                               gcSHADER_PRECISION_HIGH,
                               0,
                               -1, /* parent */
                               -1, /* prevSibling */
                               &varIndex);
        gcmASSERT(varIndex >= 0);
        vertexInstIDVariable = Shader->variables[varIndex];
        SetVariableIsCompilerGenerated(vertexInstIDVariable);
    }

    return vertexInstIDVariable;
}


static gctINT
_getVertexIDTemp(
    IN gcSHADER   VertexShader,
    IN gctBOOL    needVertexIDReversePatch
    )
{
    gcVARIABLE     vertexIDVariable = _findVexterInstIDTemp(VertexShader, gcSL_VERTEX_ID);
    gcVARIABLE     instIDVariable = _findVexterInstIDTemp(VertexShader, gcSL_INSTANCE_ID);
    gcUNIFORM      uStartVertex, uTotalVtxCountPerInstance;
    gctINT         vertexIDTemp1, vertexIDTemp2, vertexIDTemp3;
    gcSL_FORMAT    format;

    gcmASSERT(vertexIDVariable->precision == gcSHADER_PRECISION_HIGH &&
              instIDVariable->precision == gcSHADER_PRECISION_HIGH);
    vertexIDTemp1 = _getUnusedTempIndex(VertexShader, 1);

    if (needVertexIDReversePatch)
    {
        _CreatePatchUniform(VertexShader,
                            "sh_startVertex",
                            0,
                            gcSHADER_INTEGER_X1,
                            gcvUNIFORM_KIND_GENERAL_PATCH,
                            gcvFALSE,
                            gcSHADER_PRECISION_HIGH,
                            &uStartVertex);

        format = gcSL_INTEGER;
        SetUniformFlag(uStartVertex, gcvUNIFORM_FLAG_COMPILER_GEN);

        /* vertexIndex = gl_vertexID - startVertex */
        gcSHADER_AddOpcode(VertexShader, gcSL_SUB, (gctUINT16)vertexIDTemp1,
                           gcSL_ENABLE_X, format, gcSHADER_PRECISION_HIGH, 0);
        gcSHADER_AddSource(VertexShader, gcSL_TEMP, (gctUINT16)vertexIDVariable->tempIndex,
                           gcSL_SWIZZLE_XXXX, format, gcSHADER_PRECISION_HIGH);
        gcSHADER_AddSourceUniformFormatted(VertexShader,
                                           uStartVertex,
                                           gcSL_SWIZZLE_XXXX,
                                           gcSL_NOT_INDEXED,
                                           format);
    }
    else
    {
        vertexIDTemp1 = vertexIDVariable->tempIndex;
    }

    /*
    ** If FE don't reset vertexID when a new instance start,
    ** then we don't need to evaluate the vertexID.
    */
    if (!gcHWCaps.hwFeatureFlags.supportStartVertexFE)
    {
        return vertexIDTemp1;
    }

    vertexIDTemp2 = _getUnusedTempIndex(VertexShader, 1);
    vertexIDTemp3 = _getUnusedTempIndex(VertexShader, 1);

    _CreatePatchUniform(VertexShader,
                        "sh_totalVtxCountPerInstance",
                        0,
                        gcSHADER_INTEGER_X1,
                        gcvUNIFORM_KIND_GENERAL_PATCH,
                        gcvFALSE,
                        gcSHADER_PRECISION_HIGH,
                        &uTotalVtxCountPerInstance);
    format = gcSL_INTEGER;
    SetUniformFlag(uTotalVtxCountPerInstance, gcvUNIFORM_FLAG_COMPILER_GEN);

    /* vertexIndex = vertexIndex + totalVertexCount * gl_instanceID */
    gcSHADER_AddOpcode(VertexShader, gcSL_MUL, (gctUINT16)vertexIDTemp2,
                       gcSL_ENABLE_X, format, gcSHADER_PRECISION_HIGH, 0);
    gcSHADER_AddSource(VertexShader, gcSL_TEMP, (gctUINT16)instIDVariable->tempIndex,
                       gcSL_SWIZZLE_XXXX, format, gcSHADER_PRECISION_HIGH);
    gcSHADER_AddSourceUniformFormatted(VertexShader,
                                       uTotalVtxCountPerInstance,
                                       gcSL_SWIZZLE_XXXX,
                                       gcSL_NOT_INDEXED,
                                       format);

    gcSHADER_AddOpcode(VertexShader, gcSL_ADD, (gctUINT16)vertexIDTemp3,
                       gcSL_ENABLE_X, format, gcSHADER_PRECISION_HIGH, 0);
    gcSHADER_AddSource(VertexShader, gcSL_TEMP, (gctUINT16)vertexIDTemp1,
                       gcSL_SWIZZLE_XXXX, format, gcSHADER_PRECISION_HIGH);
    gcSHADER_AddSource(VertexShader, gcSL_TEMP, (gctUINT16)vertexIDTemp2,
                       gcSL_SWIZZLE_XXXX, format, gcSHADER_PRECISION_HIGH);

    /* no conversion is needed if the type is integer */
    return vertexIDTemp3;

}

static gceSTATUS
_gcAddTransformFeedback(
    IN gcSHADER   VertexShader,
    IN gctBOOL    needVertexIDReversePatch
    )
{
    gceSTATUS             status = gcvSTATUS_OK;
    gcsTRANSFORM_FEEDBACK *xfbk  = &VertexShader->transformFeedback;
    gcUNIFORM             xfbkState;
    gctUINT               label;
    gctINT                i;
    gctINT32              offset = 0;
    gctSIZE_T             sz;
    const gctINT          zeroConstant = 0;
    gctINT                temp1, temp2;
    gctINT                vertexIDTemp;
    gcUNIFORM *           separateBuffers = gcvNULL;
    gcUNIFORM             buffer = gcvNULL;

    /* create TransformFeedbackStateUniform */
    gcmONERROR(_CreateTransformFeedbackStateUniform(VertexShader,
                                                    &xfbkState));
    /* add conditonal jump code:
          if (xfbkState.x == 0) goto label;
     */
    label  = gcSHADER_FindNextUsedLabelId(VertexShader);
    gcmONERROR(gcSHADER_AddOpcodeConditionalFormatted(VertexShader,
                                                    gcSL_JMP,
                                                    gcSL_EQUAL,
                                                    gcSL_INTEGER,
                                                    label,
                                                    0));
    gcmONERROR(gcSHADER_AddSourceUniformFormatted(VertexShader,
                                                  xfbkState,
                                                  gcSL_SWIZZLE_XXXX,
                                                  0,
                                                  gcSL_INTEGER));
    gcmONERROR(gcSHADER_AddSourceConstantFormatted(VertexShader,
                                                   (void *)&zeroConstant,
                                                   gcSL_INTEGER));

    /* compute the total size of all feedback varyings */
    gcSHADER_ComputeTotalFeedbackVaryingsSize(VertexShader);

    vertexIDTemp = _getVertexIDTemp(VertexShader, needVertexIDReversePatch);

    if (xfbk->bufferMode == gcvFEEDBACK_INTERLEAVED)
    {
        gcUNIFORM     xfbkInterLeavedBuffer;
        gctINT        totalSize = xfbk->totalSize;
        /*   Interleaved Mode
         *
         * --- +--------------------+ <--- xfbkInterLeavedBuffer
         *  ^  |   varying 0        |
         *  |  |                    |
         *  |  +--------------------+      vertexID=0
         *  |  |   varying 1        |
         *  |  |                    |      varyingOffset[0] = 0;
         *  |  +--------------------+      varyingOffset[i] = varyingOffset[i-1]
         *  |  |   ...              |                         + varyingSize[i-1];
         *  |  +--------------------+
         *  |  |                    |      TotalBlockSz = Sum(varyingSize[0:n-1]);
         *  |  |   varying n-1      |
         *  v  |                    |
         * --- +--------------------+ <---  vertexID=1
         *     |                    |
         *     |   ...              |
         *     |                    |
         *     |                    |
         *     +--------------------+ <--- curBase = xfbkInterLeavedBuffer +
         *     |                    |                vertexID * TotalBlockSz;
         *     |                    |
         *     |                    |
         *     |                    |
         *     |                    |
         *     |                    |
         *     |                    |
         *     +--------------------+
         */

        /* create tranformFeedbackBuffer uniform */
        gcmONERROR(_CreatePatchUniform(VertexShader,
                              "InterleavedTransformFeedbackBuffer",
                              -1,
                              gcSHADER_INTEGER_X1,
                              gcvUNIFORM_KIND_TRANSFORM_FEEDBACK_BUFFER,
                              gcvTRUE,
                              gcSHADER_PRECISION_HIGH,
                              &xfbkInterLeavedBuffer));
        SetUniformFlag(xfbkInterLeavedBuffer, gcvUNIFORM_FLAG_COMPILER_GEN);
        xfbk->feedbackBuffer.interleavedBufUniform = xfbkInterLeavedBuffer;

        /* interleaved mode, add following code to VertexShader:

               if (xfbkState.x != 0)
               {
                   gctINT32 baseAddr = (char *)xfbkInterLeavedBuffer;
                   gctINT32 offset   = gl_VertexID * xfbk->totalSize;

                   for (int i = 0; i < xfbk->varyingCount; i++)
                   {
                       size_t outputSize = _getVaryingOutputSize(xfbk->varying[i]);
                       memcopy(baseAddr, &xfbk>varying[i], outputSize);
                       baseAddr += outputSize;
                   }
               }
         */

        /* base address = xfbkInterLeavedBuffer + vertexID * TotalBlockSz; */
        temp1 = _getUnusedTempIndex(VertexShader, 1);
        temp2 = _getUnusedTempIndex(VertexShader, 1);

        /* We don't have MAD operation in current IR, we need to do MUL and ADD */

        /* create MUL temp1.x, totalSize, temp[VertxID].x */
        gcSHADER_AddOpcode(VertexShader, gcSL_MUL, (gctUINT16)temp1,
                          gcSL_ENABLE_X, gcSL_INTEGER, gcSHADER_PRECISION_HIGH, 0);
        gcSHADER_AddSourceConstantFormatted(VertexShader, (void *)&totalSize,
                                            gcSL_INTEGER);
        gcSHADER_AddSource(VertexShader, gcSL_TEMP, (gctUINT16)vertexIDTemp,
                           gcSL_SWIZZLE_XXXX, gcSL_INTEGER, gcSHADER_PRECISION_HIGH);

        /* create ADD temp2.x, temp1.x, InterleavedBufferAddr */
        gcSHADER_AddOpcode(VertexShader, gcSL_ADD, (gctUINT16)temp2,
                           gcSL_ENABLE_X, gcSL_INTEGER, gcSHADER_PRECISION_HIGH, 0);
        gcSHADER_AddSource(VertexShader, gcSL_TEMP, (gctUINT16)temp1,
                           gcSL_SWIZZLE_XXXX, gcSL_INTEGER, gcSHADER_PRECISION_HIGH);
        gcSHADER_AddSourceUniformFormatted(VertexShader,
                                           xfbkInterLeavedBuffer,
                                           gcSL_SWIZZLE_XXXX,
                                           gcSL_NOT_INDEXED,
                                           gcSL_INTEGER);

        for (i=0; i < (gctINT)xfbk->varyingCount; i++)
        {
            gcmONERROR(_generateFeedbackWrite(VertexShader,
                                              &xfbk->varRegInfos[i],
                                              gcvNULL,
                                              temp2, /* base addr temp index */
                                              offset,
                                              &sz));
            offset += sz;
        }
    }
    else
    {
        /*  Separate Mode
         *
         *     +--------------+ <--- SeparateBufferAddr[0]
         *     |   varying 0  |
         *     |              |       vertexID=0
         *     |              |
         *     +--------------+ <---  vertexID=1
         *     |              |
         *     |   ...        |
         *     |              |
         *     |              |
         *     +--------------+ <--- curBase = SeparateBufferAddr[0] +
         *     |              |                vertexID * varyingSize[0];
         *     |              |
         *     |              |
         *     +--------------+
         *
         *     +--------------+ <--- SeparateBufferAddr[1]
         *     |   varying 1  |
         *     |              |       vertexID=0
         *     |              |
         *     +--------------+ <---  vertexID=1
         *     |              |
         *     |   ...        |
         *     |              |
         *     |              |
         *     +--------------+ <--- curBase = SeparateBufferAddr[1] +
         *     |              |                vertexID * varyingSize[1];
         *     |              |
         *     |              |
         *     +--------------+
         *
         */

        gcmASSERT(xfbk->bufferMode == gcvFEEDBACK_SEPARATE);
        gcmONERROR(gcoOS_Allocate(gcvNULL,
                                 xfbk->varyingCount * sizeof(gcUNIFORM),
                                 (gctPOINTER *) &separateBuffers));
        for (i=0; i < (gctINT)xfbk->varyingCount; i++)
        {
            /* create tranformFeedbackBuffer uniform */
            gcmONERROR(_CreatePatchUniform(VertexShader,
                                  "SeparateTransformFeedbackBuffer",
                                  i,
                                  gcSHADER_INTEGER_X1,
                                  gcvUNIFORM_KIND_TRANSFORM_FEEDBACK_BUFFER,
                                  gcvTRUE,
                                  gcSHADER_PRECISION_HIGH,
                                  &buffer));
            SetUniformFlag(buffer, gcvUNIFORM_FLAG_COMPILER_GEN);
            separateBuffers[i] = buffer;

            /* base address = separateBuffers[i] + vertexID * varyingSize[i]; */
            temp1 = _getUnusedTempIndex(VertexShader, 1);
            temp2 = _getUnusedTempIndex(VertexShader, 1);

            /* We don't have MAD operation in current IR, we need to do MUL and ADD */

            /* create MUL temp1.x, totalSize, temp[VertxID].x */
            gcSHADER_AddOpcode(VertexShader, gcSL_MUL, (gctUINT16)temp1,
                              gcSL_ENABLE_X, gcSL_INTEGER, gcSHADER_PRECISION_HIGH, 0);
            gcSHADER_AddSourceConstantFormatted(VertexShader,
                                               (void *)&xfbk->varRegInfos[i].streamoutSize,
                                                gcSL_INTEGER);
            gcSHADER_AddSource(VertexShader, gcSL_TEMP, (gctUINT16)vertexIDTemp,
                               gcSL_SWIZZLE_XXXX, gcSL_INTEGER, gcSHADER_PRECISION_HIGH);

            /* create ADD temp2.x, temp1.x, InterleavedBufferAddr */
            gcSHADER_AddOpcode(VertexShader, gcSL_ADD, (gctUINT16)temp2,
                               gcSL_ENABLE_X, gcSL_INTEGER, gcSHADER_PRECISION_HIGH, 0);
            gcSHADER_AddSource(VertexShader, gcSL_TEMP, (gctUINT16)temp1,
                               gcSL_SWIZZLE_XXXX, gcSL_INTEGER, gcSHADER_PRECISION_HIGH);
            gcSHADER_AddSourceUniformFormatted(VertexShader,
                                               separateBuffers[i],
                                               gcSL_SWIZZLE_XXXX,
                                               gcSL_NOT_INDEXED,
                                               gcSL_INTEGER);


            gcmONERROR(_generateFeedbackWrite(VertexShader,
                                              &xfbk->varRegInfos[i],
                                              gcvNULL,
                                              temp2,
                                              0,
                                              &sz));
        }
        xfbk->feedbackBuffer.separateBufUniforms = separateBuffers;
    }

    /* add label */
    gcSHADER_AddLabel(VertexShader, label);
    /* add nop for the label */
    gcSHADER_AddOpcode(VertexShader, gcSL_NOP, 0, 0, 0, 0, 0);
    gcSHADER_AddSource(VertexShader, gcSL_NONE, 0, 0, 0, 0);
    gcSHADER_AddSource(VertexShader, gcSL_NONE, 0, 0, 0, 0);

    gcSHADER_Pack(VertexShader);
    return status;

OnError:
    if (separateBuffers)
        gcoOS_Free(gcvNULL,separateBuffers);

    return status;
}

static gceSTATUS
_gcAddVertexAndInstanceIdPatch(
    IN gcSHADER             VertexShader,
    IN gctBOOL              XFBPatch,
    OUT gctBOOL*            NeedVertexIDReversePatch
    )
{
    gceSTATUS             status = gcvSTATUS_OK;
    gctUINT               i, f, mainEnter, addCodeCount;
    gctINT                tempRegNo;
    gcSL_INSTRUCTION      code, addCode, movCode, conCode;
    gctBOOL               bVertexIdUsed = gcvFALSE, bInstanceIdUsed = gcvFALSE, bInMainRoutine;
    gctBOOL               needXFB = XFBPatch;
    gcFUNCTION            function;
    gcUNIFORM             uStartVertex;
    gcSL_FORMAT           format;
    gcVARIABLE            vtxIdVariable = gcvNULL, intanceIdVariable = gcvNULL;

    /* 1. Check whether vertexID/instanceID is used within whole shader.*/
    for (i = 0; i < VertexShader->variableCount; ++ i)
    {
        gcVARIABLE variable = VertexShader->variables[i];

        if(!variable) continue;

        if(isVariableNormal(variable) && ((gctINT)variable->nameLength < 0))
        {
            if (variable->nameLength == gcSL_VERTEX_ID)
            {
                vtxIdVariable = variable;
            }
            else if (variable->nameLength == gcSL_INSTANCE_ID)
            {
                intanceIdVariable = variable;
            }
        }
    }

    if (!vtxIdVariable)
        bVertexIdUsed = gcvTRUE;
    if (!intanceIdVariable)
        bInstanceIdUsed = gcvTRUE;

    if (vtxIdVariable || intanceIdVariable)
    {
        for (i = 0; i < VertexShader->codeCount; i ++)
        {
            gcVARIABLE variable;
            code = &VertexShader->code[i];

            if (vtxIdVariable)
            {
                variable = vtxIdVariable;
                if ((gctINT)code->source0Index == variable->tempIndex ||
                    (gctINT)code->source1Index == variable->tempIndex ||
                    (gcmSL_SOURCE_GET(code->source0, Indexed) != gcSL_NOT_INDEXED &&
                    (gctINT)code->source0Indexed == variable->tempIndex) ||
                    (gcmSL_SOURCE_GET(code->source1, Indexed) != gcSL_NOT_INDEXED &&
                    (gctINT)code->source1Indexed == variable->tempIndex) ||
                    (gcmSL_TARGET_GET(code->temp, Indexed) != gcSL_NOT_INDEXED &&
                    (gctINT)code->tempIndexed == variable->tempIndex))
                {
                    bVertexIdUsed = gcvTRUE;
                }
            }

            if (intanceIdVariable)
            {
                variable = intanceIdVariable;
                if ((gctINT)code->source0Index == variable->tempIndex ||
                    (gctINT)code->source1Index == variable->tempIndex ||
                    (gcmSL_SOURCE_GET(code->source0, Indexed) != gcSL_NOT_INDEXED &&
                    (gctINT)code->source0Indexed == variable->tempIndex) ||
                    (gcmSL_SOURCE_GET(code->source1, Indexed) != gcSL_NOT_INDEXED &&
                    (gctINT)code->source1Indexed == variable->tempIndex) ||
                    (gcmSL_TARGET_GET(code->temp, Indexed) != gcSL_NOT_INDEXED &&
                    (gctINT)code->tempIndexed == variable->tempIndex))
                {
                    bInstanceIdUsed = gcvTRUE;
                }
            }

            if (bVertexIdUsed && bInstanceIdUsed)
                break;
        }
    }

    if (!vtxIdVariable)
        bVertexIdUsed = gcvFALSE;
    if (!intanceIdVariable)
        bInstanceIdUsed = gcvFALSE;

    if (!bVertexIdUsed && !bInstanceIdUsed && !needXFB)
        return gcvSTATUS_OK;

    if (needXFB)
    {
        vtxIdVariable = _findVexterInstIDTemp(VertexShader, gcSL_VERTEX_ID);
        intanceIdVariable = _findVexterInstIDTemp(VertexShader, gcSL_INSTANCE_ID);
    }

    /* Find the enter of main function. */
    for (mainEnter = 0; mainEnter < VertexShader->codeCount; mainEnter++)
    {
        bInMainRoutine = gcvTRUE;

        /* Determine ownership of the code for functions. */
        for (f = 0; f < VertexShader->functionCount; ++f)
        {
            function = VertexShader->functions[f];

            if ((mainEnter >= function->codeStart) &&
                (mainEnter < function->codeStart + function->codeCount))
            {
                bInMainRoutine = gcvFALSE;
                break;
            }
        }

        if (bInMainRoutine)
        {
            break;
        }
    }

    /* 2. Convert vertexID/instanceID if needed. */
    if (!gcHWCaps.hwFeatureFlags.vtxInstanceIdAsAttr)
    {
        addCodeCount = 0;
        if (needXFB)
        {
            addCodeCount = 4;
        }
        else
        {
            if (bVertexIdUsed)
                addCodeCount += 2;
            if (bInstanceIdUsed)
                addCodeCount += 2;
        }

        gcmONERROR(gcSHADER_InsertNOP2BeforeCode(VertexShader, mainEnter, addCodeCount, gcvTRUE, gcvTRUE));

        if (needXFB || bVertexIdUsed)
        {
            /* Convert vertexID. */
            conCode = &VertexShader->code[mainEnter];
            movCode = &VertexShader->code[mainEnter + 1];

            conCode->opcode = gcSL_F2I;
            conCode->temp = gcmSL_TARGET_SET(0, Format, gcSL_INTEGER)
                            | gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_XYZW)
                            | gcmSL_TARGET_SET(0, Indexed, gcSL_NOT_INDEXED)
                            | gcmSL_TARGET_SET(0, Precision, gcSL_PRECISION_HIGH);
            conCode->tempIndex = (gctUINT16)gcSHADER_NewTempRegs(VertexShader, 1, gcSHADER_FLOAT_X4);

            conCode->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_TEMP)
                                | gcmSL_SOURCE_SET(0, Format, gcSL_FLOAT)
                                | gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XYZW)
                                | gcmSL_SOURCE_SET(0, Indexed, gcSL_NOT_INDEXED)
                                | gcmSL_SOURCE_SET(0, Precision, gcSL_PRECISION_HIGH);
            conCode->source0Index = vtxIdVariable->tempIndex;

            movCode->opcode = gcSL_MOV;
            movCode->temp = conCode->temp;
            movCode->tempIndex = vtxIdVariable->tempIndex;

            movCode->source0 = gcmSL_SOURCE_SET(conCode->source0, Format, gcSL_INTEGER);
            movCode->source0Index = conCode->tempIndex;

            mainEnter += 2;
        }

        if (needXFB || bInstanceIdUsed)
        {
            /* Convert instanceID. */
            conCode = &VertexShader->code[mainEnter];
            movCode = &VertexShader->code[mainEnter + 1];

            conCode->opcode = gcSL_F2I;
            conCode->temp = gcmSL_TARGET_SET(0, Format, gcSL_INTEGER)
                            | gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_XYZW)
                            | gcmSL_TARGET_SET(0, Indexed, gcSL_NOT_INDEXED)
                            | gcmSL_TARGET_SET(0, Precision, gcSL_PRECISION_HIGH);
            conCode->tempIndex = (gctUINT16)gcSHADER_NewTempRegs(VertexShader, 1, gcSHADER_FLOAT_X4);

            conCode->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_TEMP)
                                | gcmSL_SOURCE_SET(0, Format, gcSL_FLOAT)
                                | gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XYZW)
                                | gcmSL_SOURCE_SET(0, Indexed, gcSL_NOT_INDEXED)
                                | gcmSL_SOURCE_SET(0, Precision, gcSL_PRECISION_HIGH);
            conCode->source0Index = intanceIdVariable->tempIndex;

            movCode->opcode = gcSL_MOV;
            movCode->temp = conCode->temp;
            movCode->tempIndex = intanceIdVariable->tempIndex;

            movCode->source0 = gcmSL_SOURCE_SET(conCode->source0, Format, gcSL_INTEGER);
            movCode->source0Index = conCode->tempIndex;
            mainEnter += 2;
        }

        gcmONERROR(gcSHADER_Pack(VertexShader));
    }

    /* 3. Add patch at the entry of shader */
    if (gcHWCaps.hwFeatureFlags.hasHalti0 &&
        !gcHWCaps.hwFeatureFlags.supportStartVertexFE &&
        bVertexIdUsed && vtxIdVariable)
    {
        gcmONERROR(gcSHADER_InsertNOP2BeforeCode(VertexShader, mainEnter, 2, gcvTRUE, gcvTRUE));

        addCode = &VertexShader->code[mainEnter];
        movCode = &VertexShader->code[mainEnter + 1];

        gcmONERROR(_CreatePatchUniform(VertexShader,
                                        "sh_startVertex",
                                        0,
                                        gcSHADER_INTEGER_X1,
                                        gcvUNIFORM_KIND_GENERAL_PATCH,
                                        gcvFALSE,
                                        gcSHADER_PRECISION_HIGH,
                                        &uStartVertex));

        format = gcSL_INTEGER;

        /* gl_vertexID = gl_vertexID + startVertex */

        tempRegNo = _getUnusedTempIndex(VertexShader, 1);

        addCode->opcode = gcSL_ADD;
        addCode->temp = gcmSL_TARGET_SET(0, Format, format) |
                          gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_X) |
                          gcmSL_TARGET_SET(0, Precision, gcSHADER_PRECISION_HIGH);
        addCode->tempIndex = (gctUINT16)tempRegNo;
        addCode->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_TEMP) |
                             gcmSL_SOURCE_SET(0, Format, format)  |
                             gcmSL_SOURCE_SET(0, Precision, gcSHADER_PRECISION_HIGH)  |
                             gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XXXX);
        addCode->source0Index = (gctUINT16)vtxIdVariable->tempIndex;
        addCode->source1 = gcmSL_SOURCE_SET(0, Type, gcSL_UNIFORM) |
                             gcmSL_SOURCE_SET(0, Format, format)  |
                             gcmSL_SOURCE_SET(0, Precision, gcSHADER_PRECISION_HIGH)  |
                             gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XXXX);
        addCode->source1Index = (gctUINT16)uStartVertex->index;
        SetUniformFlag(uStartVertex, gcvUNIFORM_FLAG_DIRECTLY_ADDRESSED);
        SetUniformFlag(uStartVertex, gcvUNIFORM_FLAG_COMPILER_GEN);

        movCode->opcode = gcSL_MOV;
        movCode->temp = gcmSL_TARGET_SET(0, Format, format) |
                        gcmSL_TARGET_SET(0, Precision, gcSHADER_PRECISION_HIGH) |
                          gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_X);
        movCode->tempIndex = (gctUINT16)vtxIdVariable->tempIndex;
        movCode->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_TEMP) |
                             gcmSL_SOURCE_SET(0, Format, format)  |
                             gcmSL_SOURCE_SET(0, Precision, gcSHADER_PRECISION_HIGH)  |
                             gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XXXX);
        movCode->source0Index = (gctUINT16)tempRegNo;
        gcmONERROR(gcSHADER_Pack(VertexShader));

        if (NeedVertexIDReversePatch)
        {
            *NeedVertexIDReversePatch = gcvTRUE;
        }
    }

    gcmONERROR(gcSHADER_Pack(VertexShader));

OnError:
    return status;
}

/* Do some preprocesses before optimize/link. */
gceSTATUS
gcDoPreprocess(
    IN gcSHADER *           Shaders,
    IN gceSHADER_FLAGS      Flags
    )
{
    gceSTATUS               status = gcvSTATUS_OK;
    gcePATCH_ID             patchId = gcPatchId;
    gcSHADER                shader;
    gcSHADER_KIND           shaderType;
    gctBOOL                 isRecompiler = Flags & gcvSHADER_RECOMPILER;
    gctBOOL                 packMainFunction;
    gctUINT                 i;

    /* if there is intrinsic builtin functions,
       compile the builtin libary and
       link the library functions into the shader*/
    _gcLinkBuiltinLibs(Shaders);

    for (i = 0; i < gcMAX_SHADERS_IN_LINK_GOURP; i ++)
    {
        shader = Shaders[i];
        if (shader == gcvNULL)
        {
            continue;
        }
        shaderType = GetShaderType(shader);

        /* Handle xfb. */
        if (shaderType == gcSHADER_TYPE_VERTEX)
        {
            gctBOOL needVertexIDReversePatch
                = gcHWCaps.hwFeatureFlags.supportStartVertexFE
                ? gcvTRUE
                : gcvFALSE;

            gctBOOL hwTFBsupport = gcHWCaps.hwFeatureFlags.supportStreamOut;

            /* Do vertexId patch */
            if (!isRecompiler)
            {
                gctBOOL xfbPatch = (shader->transformFeedback.varyingCount > 0) && !hwTFBsupport;
                /* After this, both vertexID/instanceID are integer. */
                gcmONERROR(_gcAddVertexAndInstanceIdPatch(shader, xfbPatch, &needVertexIDReversePatch));

                /* Do xfb patch */
                if (xfbPatch)
                {
                    gcmONERROR(_gcAddTransformFeedback(shader, needVertexIDReversePatch));
                }
            }
        }

        /* Need to refine for CL shader. */
        if (shaderType == gcSHADER_TYPE_CL)
        {
            packMainFunction = gcvFALSE;
        }
        else
        {
            packMainFunction = gcvTRUE;
        }

        /* Pack main function.*/
        if (packMainFunction)
        {
            gcmONERROR(_gcPackMainFunction(shader));
        }

        /* Make sure that the texld modifier instruction is just before texld instrution. */
        gcmONERROR(_gcMovTexldModifier(shader));

        /* Convert the sampler assign for function parameters. */
        gcmONERROR(_gcConvSamplerAssignForParameter(shader));

        /* Change the atomic counter uniform. */
        gcmONERROR(_gcChangeAtomicCounterUniform2BaseAddrBindingUniform(shader));

        /* Different shader stages only. */
        switch (shaderType)
        {
        case gcSHADER_TYPE_FRAGMENT:
#if TEMP_OPT_CONSTANT_TEXLD_COORD
            if (patchId == gcvPATCH_GLBM21 || patchId == gcvPATCH_GLBM25)
            {
                gcmONERROR(_gcConvertTEXLD2MOV(shader));
            }
#endif
            /* HW does not support explicit helper-invocation to shader, to reduce complex of
            next stages of BE, just put helper-invocation reference at one place */
            gcmONERROR(_gcPreprocessHeplerInvocation(shader));
            break;

        default:
            break;
        }

        /* Remove extra NOPs. */
        if (packMainFunction)
        {
            gcmONERROR(_gcRemoveNOPsInMainFunction(shader));
        }
        /* Pack shader */
        gcmONERROR(gcSHADER_Pack(shader));
    }

OnError:
    return status;
}

#endif

