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


#include "vir/codegen/gc_vsc_vir_reg_alloc.h"
#include "vir/transform/gc_vsc_vir_uniform.h"

/* ===========================================================================
   VIR_CG_MapUniforms related functions:
   This is to allocation uniforms.
   ===========================================================================
*/
void _VIR_CG_UniformColorMap_Init(
    IN VIR_Shader           *pShader,
    IN VSC_HW_CONFIG        *pHwConfig,
    IN VSC_MM               *pMM,
    IN OUT VIR_RA_ColorMap  *uniformCM,
    OUT gctUINT             *CodeGenUniformBase
    )
{
    uniformCM->maxAllocReg = 0;
    uniformCM->availReg = 0;

    /* get from the hardware configuration */
    switch (pShader->shaderKind) {
    case VIR_SHADER_FRAGMENT:
        *CodeGenUniformBase = pHwConfig->psConstRegAddrBase;
        break;
    case VIR_SHADER_VERTEX:
        *CodeGenUniformBase = pHwConfig->vsConstRegAddrBase;
        break;
    case VIR_SHADER_TESSELLATION_CONTROL:
        *CodeGenUniformBase = pHwConfig->tcsConstRegAddrBase;
        break;
    case VIR_SHADER_TESSELLATION_EVALUATION:
        *CodeGenUniformBase = pHwConfig->tesConstRegAddrBase;
        break;
    case VIR_SHADER_GEOMETRY:
        *CodeGenUniformBase = pHwConfig->gsConstRegAddrBase;
        break;
    case VIR_SHADER_COMPUTE:
        *CodeGenUniformBase = pHwConfig->hwFeatureFlags.hasThreadWalkerInPS ?
            pHwConfig->psConstRegAddrBase : pHwConfig->vsConstRegAddrBase;
        break;
    default:
        gcmASSERT(0);
        break;
    }

    /* set the max of uniform to be the HW max const, since APILevelResource check is already done.
       uniform allocation is after dubo. dubo guarantees uniform should be able to fit HW requirements.
       thus here we need to assign uniform based on HW requirements.
       (api uniform + compiler generated uniform could be larger than spec max) */
    uniformCM->maxReg = pHwConfig->maxTotalConstRegCount;

    /* each const registers needs 4 channels (w, z, y, x) */
    vscBV_Initialize(&uniformCM->usedColor,
        pMM,
        uniformCM->maxReg * VIR_CHANNEL_NUM);
}

static void _VIR_CG_ConfigSamplers(
    IN VIR_Shader       *pShader,
    IN VSC_HW_CONFIG    *pHwConfig,
    OUT gctINT          *maxSampler,
    OUT gctINT          *sampler,
    OUT gctBOOL         *allocateSamplerReverse
    )
{
    VIR_ShaderKind      shaderKind = VIR_Shader_GetKind(pShader);
    gctUINT32           vsSamplers = 0, psSamplers = 0;
    gctINT              samplerCount = 0, samplerRegNoBase = 0;

     /* Config sampler setting. */
    if (VIR_Shader_isPackUnifiedSampler(pShader))
    {
        samplerRegNoBase = VIR_Shader_GetSamplerBaseOffset(pShader);
        VIR_Shader_CalcSamplerCount(pShader, &samplerCount);

        gcmASSERT(samplerRegNoBase != -1);

        if (pHwConfig->hwFeatureFlags.supportUnifiedSampler)
        {
            if (pHwConfig->hwFeatureFlags.hasSamplerBaseOffset)
            {
                *maxSampler = samplerCount;
                *sampler = 0;
            }
            else
            {
                *maxSampler = samplerRegNoBase + samplerCount;
                *sampler = samplerRegNoBase;
            }
        }
        else
        {
            /*
            ** Set the max sampler count and the start index.
            ** If this chip can support sampler base offset, all sampler index begin with 0,
            ** and we need to program samplerRegNoBase, otherwise, we set samplerRegNoBase to 0.
            */
            if (VIR_Shader_IsGPipe(pShader))
            {
                *allocateSamplerReverse = gcvTRUE;

                if (pHwConfig->hwFeatureFlags.hasSamplerBaseOffset)
                {
                    *maxSampler = 0;
                    *sampler = samplerCount - 1;
                }
                else
                {
                    *maxSampler = samplerRegNoBase;
                    *sampler = samplerRegNoBase + samplerCount - 1;
                }
            }
            else
            {
                *maxSampler = samplerCount;
                *sampler = 0;
            }
        }
    }
    else if (pHwConfig->hwFeatureFlags.hasSamplerBaseOffset)
    {
        *sampler = 0;
        switch (shaderKind)
        {
        case VIR_SHADER_FRAGMENT:
            *maxSampler = pHwConfig->maxPSSamplerCount;
            break;

        case VIR_SHADER_COMPUTE:
            *maxSampler = pHwConfig->maxCSSamplerCount;
            break;

        case VIR_SHADER_VERTEX:
            *maxSampler = pHwConfig->maxVSSamplerCount;
            break;

        case VIR_SHADER_TESSELLATION_CONTROL:
            *maxSampler = pHwConfig->maxTCSSamplerCount;
            break;

        case VIR_SHADER_TESSELLATION_EVALUATION:
            *maxSampler = pHwConfig->maxTESSamplerCount;
            break;

        case VIR_SHADER_GEOMETRY:
            *maxSampler = pHwConfig->maxGSSamplerCount;
            break;

        default:
            gcmASSERT(0);
            *maxSampler = pHwConfig->maxPSSamplerCount;
            break;
        }
    }
    else
    {
        /* map Sampler */
        vsSamplers = pHwConfig->maxVSSamplerCount;
        psSamplers = pHwConfig->maxPSSamplerCount;

        /* Determine starting sampler index. */
        *sampler = (shaderKind == VIR_SHADER_VERTEX)
                    ? psSamplers
                    : 0;

        /* Determine maximum sampler index. */
        /* Note that CL kernel can use all samplers if unified. */
        *maxSampler = (shaderKind == VIR_SHADER_FRAGMENT)
                        ? psSamplers
                        : psSamplers + vsSamplers;
    }
}

static void _VIR_CG_isUBOSupported(
    IN VIR_Shader           *pShader,
    IN VSC_HW_CONFIG        *pHwConfig,
    OUT gctBOOL             *handleDefaultUBO,
    OUT gctBOOL             *unblockUniformBlock)
{
    if (VIR_IdList_Count(&pShader->uniformBlocks) > 0)
    {
        if (pShader->_enableDefaultUBO  &&
            pHwConfig->hwFeatureFlags.hasHalti1)
        {
            *handleDefaultUBO = gcvTRUE;
            *unblockUniformBlock = gcvFALSE;
        }
        else if (!pHwConfig->hwFeatureFlags.hasHalti1)
        {
            *handleDefaultUBO = gcvFALSE;
            *unblockUniformBlock = gcvTRUE;
        }
        else
        {
            *handleDefaultUBO = gcvFALSE;
            *unblockUniformBlock = gcvFALSE;
        }
    }
}

/* mark uniform slot used */
void VIR_CG_SetUniformUsed(
    IN VIR_RA_ColorMap  *uniformColorMap,
    IN gctINT           startIdx,
    IN gctINT           rows,
    IN gctUINT8         enable
    )
{
    VSC_BIT_VECTOR  *usedColor = &uniformColorMap->usedColor;

    while (rows-- > 0)
    {
         /* Test if x-component is available. */
        if (enable & 0x1)
        {
             vscBV_SetBit(usedColor, startIdx * 4 + 0);
        }

        /* Test if y-component is available. */
        if (enable & 0x2)
        {
            vscBV_SetBit(usedColor, startIdx * 4 + 1);
        }

        /* Test if z-component is available. */
        if (enable & 0x4)
        {
            vscBV_SetBit(usedColor, startIdx * 4 + 2);
        }

        /* Test if w-component is available. */
        if (enable & 0x8)
        {
            vscBV_SetBit(usedColor, startIdx * 4 + 3);
        }

        startIdx++;
    }
}

gctBOOL VIR_CG_UniformAvailable(
    IN VIR_RA_ColorMap      *uniformColorMap,
    IN gctINT               startIdx,
    IN gctINT               rows,
    IN gctUINT8             enable
    )
{
    VSC_BIT_VECTOR  *usedColor = &uniformColorMap->usedColor;

    while (rows-- > 0)
    {
         /* Test if x-component is available. */
        if ((enable & 0x1) && vscBV_TestBit(usedColor, startIdx * 4 + 0))
        {
            return gcvFALSE;
        }

        /* Test if y-component is available. */
        if ((enable & 0x2) && vscBV_TestBit(usedColor, startIdx * 4 + 1))
        {
            return gcvFALSE;
        }

        /* Test if z-component is available. */
        if ((enable & 0x4) && vscBV_TestBit(usedColor, startIdx * 4 + 2))
        {
            return gcvFALSE;
        }

        /* Test if w-component is available. */
        if ((enable & 0x8) && vscBV_TestBit(usedColor, startIdx * 4 + 3))
        {
            return gcvFALSE;
        }

        startIdx++;
    }

    return gcvTRUE;
}

/* find next available slot and mark it used */
VSC_ErrCode
VIR_CG_FindUniformUse(
    IN VIR_RA_ColorMap  *uniformColorMap,
    IN VIR_TypeId       type,
    IN gctINT           arraySize,
    IN gctBOOL          restricted,
    OUT gctINT          *Physical,
    OUT gctUINT8        *Swizzle,
    OUT gctINT          *Shift)
{
    gctINT i;
    gctUINT32 components = 0;
    gctINT shift;
    gctUINT8 swizzle = 0, enable = 0;

    components = VIR_GetTypeComponents(type);

    if ((gctINT) uniformColorMap->maxReg < arraySize)
    {
        return VSC_ERR_OUT_OF_RESOURCE;
    }

    for (i = 0; i <= ((gctINT) uniformColorMap->maxReg) - arraySize; ++i)
    {
        shift = -1;

        switch (components)
        {
        case 1:
            /* See if x-component is available. */
            if (VIR_CG_UniformAvailable(uniformColorMap, i, arraySize, 0x1 << 0))
            {
                shift   = 0;
                enable  = VIR_ENABLE_X;
                swizzle = VIR_SWIZZLE_XXXX;
            }

            /* See if y-component is available. */
            else if (!restricted && VIR_CG_UniformAvailable(uniformColorMap, i, arraySize, 0x1 << 1))
            {
                shift   = 1;
                enable  = VIR_ENABLE_Y;
                swizzle = VIR_SWIZZLE_YYYY;
            }

            /* See if z-component is available. */
            else if (!restricted && VIR_CG_UniformAvailable(uniformColorMap, i, arraySize, 0x1 << 2))
            {
                shift   = 2;
                enable  = VIR_ENABLE_Z;
                swizzle = VIR_SWIZZLE_ZZZZ;
            }

            /* See if w-component is available. */
            else if (!restricted && VIR_CG_UniformAvailable(uniformColorMap, i, arraySize, 0x1 << 3))
            {
                shift   = 3;
                enable  = VIR_ENABLE_W;
                swizzle = VIR_SWIZZLE_WWWW;
            }

            break;

        case 2:
            /* See if x- and y-components are available. */
            if (VIR_CG_UniformAvailable(uniformColorMap, i, arraySize, 0x3 << 0))
            {
                shift   = 0;
                enable  = VIR_ENABLE_XY;
                swizzle = VIR_SWIZZLE_XYYY;
            }

            /* See if y- and z-components are available. */
            else if (!restricted && VIR_CG_UniformAvailable(uniformColorMap, i, arraySize, 0x3 << 1))
            {
                shift   = 1;
                enable  = VIR_ENABLE_YZ;
                swizzle = VIR_SWIZZLE_YZZZ;
            }

            /* See if z- and w-components are available. */
            else if (!restricted && VIR_CG_UniformAvailable(uniformColorMap, i, arraySize, 0x3 << 2))
            {
                shift   = 2;
                enable  = VIR_ENABLE_ZW;
                swizzle = VIR_SWIZZLE_ZWWW;
            }

            break;

        case 3:
            /* See if x-, y- and z-components are available. */
            if (VIR_CG_UniformAvailable(uniformColorMap, i, arraySize, 0x7 << 0))
            {
                shift   = 0;
                enable  = VIR_ENABLE_XYZ;
                swizzle = VIR_SWIZZLE_XYZZ;
            }

            /* See if y-, z- and w-components are available. */
            else if (!restricted && VIR_CG_UniformAvailable(uniformColorMap, i, arraySize, 0x7 << 1))
            {
                shift   = 1;
                enable  = VIR_ENABLE_YZW;
                swizzle = VIR_SWIZZLE_YZWW;
            }

            break;

        case 4:
            /* See if x-, y-, z- and w-components are available. */
            if (VIR_CG_UniformAvailable(uniformColorMap, i, arraySize, 0xF << 0))
            {
                shift   = 0;
                enable  = VIR_ENABLE_XYZW;
                swizzle = VIR_SWIZZLE_XYZW;
            }

            break;

        default:
            gcmASSERT(gcvFALSE);
            break;
        }

        if (shift >=0)
        {
            *Physical = i;
            *Shift = shift;
            *Swizzle = swizzle;

            /* Set the uniform used */
            VIR_CG_SetUniformUsed(uniformColorMap, i, arraySize, enable);

            return VSC_ERR_NONE;
        }
    }

    return VSC_ERR_OUT_OF_RESOURCE;
}

static gctUINT8 _VIR_CG_EnableShiftWrap(
    gctUINT8    origEnable,
    gctUINT     count,
    gctUINT     step)
{
    gctUINT8 retEanble = VIR_ENABLE_NONE;

    switch (step)
    {
    case 1:
        retEanble = origEnable;
        break;
    case 2:
        if ((count % 2) == 0)
        {
            retEanble = origEnable;
        }
        else
        {
            switch (origEnable)
            {
            case VIR_ENABLE_XY:
                retEanble = VIR_ENABLE_ZW;
                break;
            case VIR_ENABLE_ZW:
                retEanble = VIR_ENABLE_XY;
                break;
            default:
                gcmASSERT(gcvFALSE);
                break;
            }
        }
        break;
    case 4:
        if ((count % 4) == 0)
        {
            retEanble = origEnable;
        }
        else if ((count % 4) == 1)
        {
            switch (origEnable)
            {
            case VIR_ENABLE_X:
                retEanble = VIR_ENABLE_Y;
                break;
            case VIR_ENABLE_Y:
                retEanble = VIR_ENABLE_Z;
                break;
            case VIR_ENABLE_Z:
                retEanble = VIR_ENABLE_W;
                break;
            case VIR_ENABLE_W:
                retEanble = VIR_ENABLE_X;
                break;
            default:
                gcmASSERT(gcvFALSE);
                break;
            }
        }
        else if ((count % 4) == 2)
        {
            switch (origEnable)
            {
            case VIR_ENABLE_X:
                retEanble = VIR_ENABLE_Z;
                break;
            case VIR_ENABLE_Y:
                retEanble = VIR_ENABLE_W;
                break;
            case VIR_ENABLE_Z:
                retEanble = VIR_ENABLE_X;
                break;
            case VIR_ENABLE_W:
                retEanble = VIR_ENABLE_Y;
                break;
            default:
                gcmASSERT(gcvFALSE);
                break;
            }
        }
        else
        {
            switch (origEnable)
            {
            case VIR_ENABLE_X:
                retEanble = VIR_ENABLE_W;
                break;
            case VIR_ENABLE_Y:
                retEanble = VIR_ENABLE_X;
                break;
            case VIR_ENABLE_Z:
                retEanble = VIR_ENABLE_Y;
                break;
            case VIR_ENABLE_W:
                retEanble = VIR_ENABLE_Z;
                break;
            default:
                gcmASSERT(gcvFALSE);
                break;
            }
        }
        break;
    default:
        break;
    }

    return retEanble;
}

gctBOOL VIR_CG_UniformAvailablePacked(
    IN VIR_RA_ColorMap      *uniformColorMap,
    IN gctINT               origStartIdx,
    IN gctINT               rows,
    IN gctUINT8             enable,
    IN gctUINT              step
    )
{
    VSC_BIT_VECTOR  *usedColor = &uniformColorMap->usedColor;
    gctUINT         curIdx = origStartIdx;
    gctUINT         count = 0;

    while (rows-- > 0)
    {
         /* Test if x-component is available. */
        if ((enable & 0x1) && vscBV_TestBit(usedColor, curIdx * 4 + 0))
        {
            return gcvFALSE;
        }

        /* Test if y-component is available. */
        if ((enable & 0x2) && vscBV_TestBit(usedColor, curIdx * 4 + 1))
        {
            return gcvFALSE;
        }

        /* Test if z-component is available. */
        if ((enable & 0x4) && vscBV_TestBit(usedColor, curIdx * 4 + 2))
        {
            return gcvFALSE;
        }

        /* Test if w-component is available. */
        if ((enable & 0x8) && vscBV_TestBit(usedColor, curIdx * 4 + 3))
        {
            return gcvFALSE;
        }

        count++;

        curIdx = origStartIdx + count / step;

        enable = _VIR_CG_EnableShiftWrap(enable, count, step);
    }

    return gcvTRUE;
}

static gctBOOL
_isTypeIdLongUlong(
    IN VIR_TypeId   TypeId
)
{
    VIR_PrimitiveTypeId format;

    format = VIR_GetTypeComponentType(TypeId);
    return format == VIR_TYPE_INT64 || format == VIR_TYPE_UINT64;
}

/* whether we could find an existing const */
gctBOOL VIR_CG_ConstUniformExistBefore(
    IN VIR_Shader       *pShader,
    IN VIR_Symbol       *pSym,
    IN OUT VIR_Uniform  *pSymUniform
    )
{
    gctBOOL isConstExistBefore = gcvFALSE, match = gcvFALSE;
    gctINT  i, index = 0, count;
    gctUINT8 swizzle = 0;
    gctBOOL valid[4] = {gcvFALSE, gcvFALSE, gcvFALSE, gcvFALSE};
    VIR_Uniform *uniform = gcvNULL;

    VIR_Const *pConstVal;
    VIR_Type *symType;
    gctBOOL isLongUlong = gcvFALSE;

    /* skip check for initialized uniform array */
    symType = VIR_Symbol_GetType(pSym);
    if(VIR_Type_isArray(symType)) return gcvFALSE;

    pConstVal = (VIR_Const *) VIR_GetSymFromId(&pShader->constTable,
                                               pSymUniform->u.initializer);

    switch (pConstVal->type)
    {
    case VIR_TYPE_FLOAT_X32:
    case VIR_TYPE_FLOAT_X16:
    case VIR_TYPE_FLOAT_X8:
    case VIR_TYPE_INT8_X32:
    case VIR_TYPE_INT8_X16:
    case VIR_TYPE_INT8_X8:
    case VIR_TYPE_INT16_X32:
    case VIR_TYPE_INT16_X16:
    case VIR_TYPE_INT16_X8:
    case VIR_TYPE_INTEGER_X32:
    case VIR_TYPE_INTEGER_X16:
    case VIR_TYPE_INTEGER_X8:
    case VIR_TYPE_UINT8_X32:
    case VIR_TYPE_UINT8_X16:
    case VIR_TYPE_UINT8_X8:
    case VIR_TYPE_UINT16_X32:
    case VIR_TYPE_UINT16_X16:
    case VIR_TYPE_UINT16_X8:
    case VIR_TYPE_UINT_X32:
    case VIR_TYPE_UINT_X16:
    case VIR_TYPE_UINT_X8:
    case VIR_TYPE_INT16_P32:
    case VIR_TYPE_INT16_P16:
    case VIR_TYPE_UINT16_P32:
    case VIR_TYPE_UINT16_P16:
    case VIR_TYPE_INT8_P32:
    case VIR_TYPE_UINT8_P32:
    case VIR_TYPE_INT64_X32:
    case VIR_TYPE_INT64_X16:
    case VIR_TYPE_INT64_X8:
    case VIR_TYPE_UINT64_X32:
    case VIR_TYPE_UINT64_X16:
    case VIR_TYPE_UINT64_X8:
        return gcvFALSE;
    case VIR_TYPE_FLOAT_X4:
        valid[3] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_FLOAT_X3:
        valid[2] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_FLOAT_X2:
        valid[1] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_FLOAT32:
        valid[0] = gcvTRUE;
        break;
    case VIR_TYPE_UINT_X4:
        valid[3] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_UINT_X3:
        valid[2] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_UINT_X2:
        valid[1] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_UINT32:
        valid[0] = gcvTRUE;
        break;
    case VIR_TYPE_INTEGER_X4:
        valid[3] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INTEGER_X3:
        valid[2] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INTEGER_X2:
        valid[1] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INT32:
        valid[0] = gcvTRUE;
        break;
    case VIR_TYPE_UINT64_X4:
        valid[3] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_UINT64_X3:
        valid[2] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_UINT64_X2:
        valid[1] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_UINT64:
        valid[0] = gcvTRUE;
        isLongUlong = gcvTRUE;
        break;
    case VIR_TYPE_INT64_X4:
        valid[3] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INT64_X3:
        valid[2] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INT64_X2:
        valid[1] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INT64:
        valid[0] = gcvTRUE;
        isLongUlong = gcvTRUE;
        break;
    case VIR_TYPE_INT16_X4:
    case VIR_TYPE_UINT16_X4:
        valid[3] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INT16_X3:
    case VIR_TYPE_UINT16_X3:
        valid[2] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INT16_X2:
    case VIR_TYPE_UINT16_X2:
        valid[1] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INT16:
    case VIR_TYPE_UINT16:
        valid[0] = gcvTRUE;
        break;
    case VIR_TYPE_INT16_P8:
    case VIR_TYPE_UINT16_P8:
        valid[3] = gcvTRUE;
        valid[2] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INT16_P4:
    case VIR_TYPE_UINT16_P4:
    case VIR_TYPE_INT16_P3:
    case VIR_TYPE_UINT16_P3:
        valid[1] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INT16_P2:
    case VIR_TYPE_UINT16_P2:
        valid[0] = gcvTRUE;
        break;
    case VIR_TYPE_UINT8_X4:
        valid[3] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_UINT8_X3:
        valid[2] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_UINT8_X2:
        valid[1] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_UINT8:
        valid[0] = gcvTRUE;
        break;
    case VIR_TYPE_INT8_X4:
        valid[3] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INT8_X3:
        valid[2] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INT8_X2:
        valid[1] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INT8:
        valid[0] = gcvTRUE;
        break;
    case VIR_TYPE_INT8_P16:
    case VIR_TYPE_UINT8_P16:
        valid[3] = gcvTRUE;
        valid[2] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INT8_P8:
    case VIR_TYPE_UINT8_P8:
        valid[1] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INT8_P4:
    case VIR_TYPE_UINT8_P4:
    case VIR_TYPE_INT8_P3:
    case VIR_TYPE_UINT8_P3:
    case VIR_TYPE_INT8_P2:
    case VIR_TYPE_UINT8_P2:
        valid[0] = gcvTRUE;
        break;
    case VIR_TYPE_BOOLEAN_X4:
        valid[3] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_BOOLEAN_X3:
        valid[2] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_BOOLEAN_X2:
        valid[1] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_BOOLEAN:
        valid[0] = gcvTRUE;
        break;

    case VIR_TYPE_FLOAT16_X4:
        valid[3] = gcvTRUE;
    case VIR_TYPE_FLOAT16_X3:
        valid[2] = gcvTRUE;
    case VIR_TYPE_FLOAT16_X2:
        valid[1] = gcvTRUE;
    case VIR_TYPE_FLOAT16:
        valid[0] = gcvTRUE;
        break;

    default:
        gcmASSERT(gcvFALSE);
        return gcvFALSE;
    }

    count = VIR_GetTypeComponents(pConstVal->type);

    for (i = 0; i < (gctINT) VIR_IdList_Count(&pShader->uniforms); ++i)
    {
        VIR_Id      id  = VIR_IdList_GetId(&pShader->uniforms, i);
        VIR_Symbol *sym = VIR_Shader_GetSymFromId(pShader, id);
        uniform = VIR_Symbol_GetUniform(sym);

        if (uniform == gcvNULL || (pSymUniform == uniform))
        {
            continue;
        }

        if (isSymUniformCompiletimeInitialized(sym) &&
            (uniform->physical != -1))
        {
            VIR_Const *constVal;
            gctINT constCount;
            symType = VIR_Symbol_GetType(sym);

            if(VIR_Type_isArray(symType)) continue;

            constVal = (VIR_Const *) VIR_GetSymFromId(&pShader->constTable, uniform->u.initializer);
            constCount = VIR_GetTypeComponents(constVal->type);

            gcmASSERT(constCount > 0);

            if(isLongUlong && _isTypeIdLongUlong(constVal->type))
            {
                switch (count)
                {
                case 1:
                    for (index = 0; index < constCount; ++index)
                    {
                        if (pConstVal->value.vecVal.u64Value[0] == constVal->value.vecVal.u64Value[index])
                        {
                            match = gcvTRUE;
                            break;
                        }
                    }
                    break;

                case 2:
                    for (index = 0; index < constCount - 1; ++index)
                    {
                        if ((!valid[0] || pConstVal->value.vecVal.u64Value[0] == constVal->value.vecVal.u64Value[index])
                        &&  (!valid[1] || pConstVal->value.vecVal.u64Value[1] == constVal->value.vecVal.u64Value[index + 1])
                        )
                        {
                            match = gcvTRUE;
                            break;
                        }
                    }
                    break;

                case 3:
                    for (index = 0; index < constCount - 2; ++index)
                    {
                        if ((!valid[0] || pConstVal->value.vecVal.u64Value[0] == constVal->value.vecVal.u64Value[index])
                        &&  (!valid[1] || pConstVal->value.vecVal.u64Value[1] == constVal->value.vecVal.u64Value[index + 1])
                        &&  (!valid[2] || pConstVal->value.vecVal.u64Value[2] == constVal->value.vecVal.u64Value[index + 2])
                        )
                        {
                            match = gcvTRUE;
                            break;
                        }
                    }
                    break;

                case 4:
                    index = 0;

                    if (constCount == count
                    &&  (!valid[0] || pConstVal->value.vecVal.u64Value[0] == constVal->value.vecVal.u64Value[index])
                    &&  (!valid[1] || pConstVal->value.vecVal.u64Value[1] == constVal->value.vecVal.u64Value[index + 1])
                    &&  (!valid[2] || pConstVal->value.vecVal.u64Value[2] == constVal->value.vecVal.u64Value[index + 2])
                    &&  (!valid[3] || pConstVal->value.vecVal.u64Value[3] == constVal->value.vecVal.u64Value[index + 3])
                    )
                    {
                        match = gcvTRUE;
                    }
                    break;
                }
            }
            else
            {
                switch (count)
                {
                case 1:
                    for (index = 0; index < constCount; ++index)
                    {
                        if (pConstVal->value.vecVal.u32Value[0] == constVal->value.vecVal.u32Value[index])
                        {
                            match = gcvTRUE;
                            break;
                        }
                    }
                    break;

                case 2:
                    for (index = 0; index < constCount - 1; ++index)
                    {
                        if ((!valid[0] || pConstVal->value.vecVal.u32Value[0] == constVal->value.vecVal.u32Value[index])
                        &&  (!valid[1] || pConstVal->value.vecVal.u32Value[1] == constVal->value.vecVal.u32Value[index + 1])
                        )
                        {
                            match = gcvTRUE;
                            break;
                        }
                    }
                    break;

                case 3:
                    for (index = 0; index < constCount - 2; ++index)
                    {
                        if ((!valid[0] || pConstVal->value.vecVal.u32Value[0] == constVal->value.vecVal.u32Value[index])
                        &&  (!valid[1] || pConstVal->value.vecVal.u32Value[1] == constVal->value.vecVal.u32Value[index + 1])
                        &&  (!valid[2] || pConstVal->value.vecVal.u32Value[2] == constVal->value.vecVal.u32Value[index + 2])
                        )
                        {
                            match = gcvTRUE;
                            break;
                        }
                    }
                    break;

                case 4:
                    index = 0;

                    if (constCount == count
                    &&  (!valid[0] || pConstVal->value.vecVal.u32Value[0] == constVal->value.vecVal.u32Value[index])
                    &&  (!valid[1] || pConstVal->value.vecVal.u32Value[1] == constVal->value.vecVal.u32Value[index + 1])
                    &&  (!valid[2] || pConstVal->value.vecVal.u32Value[2] == constVal->value.vecVal.u32Value[index + 2])
                    &&  (!valid[3] || pConstVal->value.vecVal.u32Value[3] == constVal->value.vecVal.u32Value[index + 3])
                    )
                    {
                        match = gcvTRUE;
                    }
                    break;
                }
            }

            if (match)
            {
                isConstExistBefore = gcvTRUE;
                break;
            }
        }
    }

    if (isConstExistBefore)
    {
        /* if there is a const uniform already exit, use the existed uniform */
        pSymUniform->physical = uniform->physical;
        pSymUniform->address = uniform->address;
        swizzle = uniform->swizzle >> (index * 2);
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
        pSymUniform->swizzle = swizzle;
    }

    return isConstExistBefore;
}

/* whether we need to allocate this uniform or not */
static gctBOOL _VIR_CG_isUniformAllocable(
    VIR_Symbol  *pSym,
    gctBOOL     handleDefaultUBO,
    gctBOOL     unblockUniformBlock,
    VIR_Uniform **symUniform)
{
    gctBOOL     retValue = gcvTRUE;
    VIR_Uniform *pUniform = VIR_Symbol_GetUniformPointer(VIR_Symbol_GetShader(pSym), pSym);

    if (pUniform == gcvNULL)
    {
        retValue = gcvFALSE;
    }
    else
    {
        switch (VIR_Symbol_GetUniformKind(pSym))
        {
        case VIR_UNIFORM_NORMAL:
        case VIR_UNIFORM_LOD_MIN_MAX:
        case VIR_UNIFORM_LEVEL_BASE_SIZE:
        case VIR_UNIFORM_LEVELS_SAMPLES:
        case VIR_UNIFORM_STORAGE_BLOCK_ADDRESS:
        case VIR_UNIFORM_GLOBAL_SIZE:
        case VIR_UNIFORM_LOCAL_SIZE:
        case VIR_UNIFORM_NUM_GROUPS:
        case VIR_UNIFORM_NUM_GROUPS_FOR_SINGLE_GPU:
        case VIR_UNIFORM_GLOBAL_OFFSET:
        case VIR_UNIFORM_WORK_DIM:
        case VIR_UNIFORM_KERNEL_ARG:
        case VIR_UNIFORM_KERNEL_ARG_LOCAL:
        case VIR_UNIFORM_KERNEL_ARG_SAMPLER:
        case VIR_UNIFORM_KERNEL_ARG_CONSTANT:
        case VIR_UNIFORM_KERNEL_ARG_LOCAL_MEM_SIZE:
        case VIR_UNIFORM_KERNEL_ARG_PRIVATE:
        case VIR_UNIFORM_SAMPLE_LOCATION:
        case VIR_UNIFORM_ENABLE_MULTISAMPLE_BUFFERS:
        case VIR_UNIFORM_TEMP_REG_SPILL_MEM_ADDRESS:
        case VIR_UNIFORM_LOCAL_ADDRESS_SPACE:
        case VIR_UNIFORM_PRIVATE_ADDRESS_SPACE:
        case VIR_UNIFORM_CONSTANT_ADDRESS_SPACE:
        case VIR_UNIFORM_CONST_BORDER_VALUE:
        case VIR_UNIFORM_SAMPLED_IMAGE:
        case VIR_UNIFORM_EXTRA_LAYER:
        case VIR_UNIFORM_PUSH_CONSTANT:
        case VIR_UNIFORM_BASE_INSTANCE:
        case VIR_UNIFORM_GL_IMAGE_FOR_IMAGE_T:
        case VIR_UNIFORM_GL_SAMPLER_FOR_IMAGE_T:
        case VIR_UNIFORM_WORK_THREAD_COUNT:
        case VIR_UNIFORM_WORK_GROUP_COUNT:
        case VIR_UNIFORM_WORK_GROUP_ID_OFFSET:
        case VIR_UNIFORM_PRINTF_ADDRESS:
        case VIR_UNIFORM_WORKITEM_PRINTF_BUFFER_SIZE:
        case VIR_UNIFORM_GENERAL_PATCH:
        case VIR_UNIFORM_TRANSFORM_FEEDBACK_BUFFER:
        case VIR_UNIFORM_TRANSFORM_FEEDBACK_STATE:
        case VIR_UNIFORM_TEXELBUFFER_TO_IMAGE:
        case VIR_UNIFORM_GLOBAL_WORK_SCALE:

            if (isSymUniformMovedToAUBO(pSym))
            {
                retValue = gcvFALSE;
            }
            break;

        case VIR_UNIFORM_UNIFORM_BLOCK_ADDRESS:
            if (handleDefaultUBO)
            {
                if (!isSymUniformUsedInShader(pSym))
                {
                    retValue = gcvFALSE;
                }
            }
            else if (unblockUniformBlock)
            {
                retValue = gcvFALSE;
            }
            break;

        case VIR_UNIFORM_BLOCK_MEMBER:
            if(handleDefaultUBO)
            {
                if(!isSymUniformMovedToDUB(pSym))
                {
                    retValue = gcvFALSE;
                }
            }
            else if(!unblockUniformBlock)
            {
                retValue = gcvFALSE;
            }
            break;

        default:
            retValue = gcvFALSE;
            break;
        }
    }

    if (retValue)
    {
        *symUniform = pUniform;
    }

    return retValue;
}

static gctBOOL _VIR_CG_isSamplerType(
    VIR_Symbol *pSym)
{
    gctBOOL     retValue = gcvFALSE;

    VIR_PrimitiveTypeId baseType;

    baseType = VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(pSym));

    switch(baseType)
    {
    case VIR_TYPE_SAMPLER:
    case VIR_TYPE_SAMPLER_1D:
    case VIR_TYPE_SAMPLER_2D:
    case VIR_TYPE_SAMPLER_3D:
    case VIR_TYPE_SAMPLER_EXTERNAL_OES:
    case VIR_TYPE_SAMPLER_2D_SHADOW:
    case VIR_TYPE_SAMPLER_CUBE_SHADOW:
    case VIR_TYPE_SAMPLER_CUBE_ARRAY_SHADOW:
    case VIR_TYPE_SAMPLER_CUBIC:
    case VIR_TYPE_SAMPLER_CUBE_ARRAY:
    case VIR_TYPE_SAMPLER_1D_ARRAY:
    case VIR_TYPE_SAMPLER_1D_ARRAY_SHADOW:
    case VIR_TYPE_SAMPLER_2D_ARRAY:
    case VIR_TYPE_SAMPLER_2D_ARRAY_SHADOW:
    case VIR_TYPE_ISAMPLER_2D:
    case VIR_TYPE_ISAMPLER_2D_ARRAY:
    case VIR_TYPE_ISAMPLER_3D:
    case VIR_TYPE_ISAMPLER_CUBIC:
    case VIR_TYPE_ISAMPLER_CUBE_ARRAY:
    case VIR_TYPE_USAMPLER_2D:
    case VIR_TYPE_USAMPLER_2D_ARRAY:
    case VIR_TYPE_USAMPLER_3D:
    case VIR_TYPE_USAMPLER_CUBIC:
    case VIR_TYPE_USAMPLER_CUBE_ARRAY:
    case VIR_TYPE_SAMPLER_2D_MS:
    case VIR_TYPE_ISAMPLER_2D_MS:
    case VIR_TYPE_USAMPLER_2D_MS:
    case VIR_TYPE_SAMPLER_2D_MS_ARRAY:
    case VIR_TYPE_ISAMPLER_2D_MS_ARRAY:
    case VIR_TYPE_USAMPLER_2D_MS_ARRAY:
    case VIR_TYPE_SAMPLER_BUFFER:
    case VIR_TYPE_ISAMPLER_BUFFER:
    case VIR_TYPE_USAMPLER_BUFFER:
    case VIR_TYPE_VIV_GENERIC_GL_SAMPLER:
    case VIR_TYPE_SAMPLER_2D_RECT:
    case VIR_TYPE_ISAMPLER_2D_RECT:
    case VIR_TYPE_USAMPLER_2D_RECT:
    case VIR_TYPE_SAMPLER_2D_RECT_SHADOW:
    case VIR_TYPE_ISAMPLER_1D_ARRAY:
    case VIR_TYPE_USAMPLER_1D_ARRAY:
    case VIR_TYPE_ISAMPLER_1D:
    case VIR_TYPE_USAMPLER_1D:
    case VIR_TYPE_SAMPLER_1D_SHADOW:
        retValue = gcvTRUE;
        break;
    default:
        break;
    }

    return retValue;
}

void
_VIR_CG_UniformListQueue(
    IN VSC_MM                   *pMM,
    IN VSC_SIMPLE_QUEUE         *WorkList,
    IN VIR_Uniform              *pUniform
    )
{
    VSC_UNI_LIST_NODE_EXT *worklistNode = (VSC_UNI_LIST_NODE_EXT *)vscMM_Alloc(pMM,
        sizeof(VSC_UNI_LIST_NODE_EXT));

    vscULNDEXT_Initialize(worklistNode, pUniform);
    QUEUE_PUT_ENTRY(WorkList, worklistNode);
}

void
_VIR_CG_UniformListDequeue(
    IN VSC_MM                   *pMM,
    IN VSC_SIMPLE_QUEUE         *WorkList,
    OUT VIR_Uniform            **pUniform
    )
{
    VSC_UNI_LIST_NODE_EXT *worklistNode = (VSC_UNI_LIST_NODE_EXT *)QUEUE_GET_ENTRY(WorkList);

    *pUniform = (VIR_Uniform *)vscULNDEXT_GetContainedUserData(worklistNode);

    vscMM_Free(pMM, worklistNode);
}

static gctBOOL
_VIR_CG_IsUniformRestricted(
    IN VIR_Symbol       *pSymbol
    )
{
    gctBOOL restricted = gcvFALSE;
    VIR_UniformKind uniformKind = VIR_Symbol_GetUniformKind(pSymbol);

    /*
    ** 1) for #num_group, it must be c.xyz
    ** 2) for #base_instance, it must be c.x
    */
    if (uniformKind == VIR_UNIFORM_NUM_GROUPS    ||
        uniformKind == VIR_UNIFORM_NUM_GROUPS_FOR_SINGLE_GPU ||
        uniformKind == VIR_UNIFORM_BASE_INSTANCE)
    {
        restricted = gcvTRUE;
    }
    else if (VIR_Symbol_GetCannotShift(pSymbol))
    {
        restricted = gcvTRUE;
    }

    return restricted;
}

VSC_ErrCode _VIR_CG_MapNonSamplerUniforms(
    IN VIR_Shader       *pShader,
    IN VSC_HW_CONFIG    *pHwConfig,
    IN VIR_Uniform      *pUniform,
    IN gctBOOL          Initialized,
    IN VIR_RA_ColorMap  *uniformColorMap,
    IN gctINT           codeGenUniformBase,
    IN gctBOOL          handleDefaultUBO,
    IN gctBOOL          unblockUniformBlock,
    IN gctBOOL          TreatSamplerAsConst,
    IN gctBOOL          singleUniform,
    IN gctBOOL          alwaysAllocate,
    IN VSC_MM           *pMM,
    OUT gctUINT         *pUniformSize,
    OUT gctINT          *NextUniformIndex
    )
{
    VSC_ErrCode retValue = VSC_ERR_NONE;

    VIR_Symbol  *pSym = VIR_Shader_GetSymFromId(pShader, VIR_Uniform_GetSymID(pUniform));
    gctINT      UniformIndex = VIR_Uniform_GetID(pUniform);
    gctINT      lastUniformIndex = UniformIndex;

    gctINT      shift = 0, arraySize = 0, physical = 0;
    VIR_TypeId  type = VIR_TYPE_FLOAT_X4;
    gctSIZE_T   maxComp = 0;
    gctUINT8    swizzle = 0;
    gctBOOL     restricted = gcvFALSE;
    VSC_SIMPLE_QUEUE    uniformList;
    gctINT     i;

    /* Determine base address for uniforms. */
    const gctUINT32 uniformBaseAddress = codeGenUniformBase * 4;

    if (!singleUniform && !TreatSamplerAsConst)
    {
        VSC_GetUniformIndexingRange(pShader,
                                UniformIndex,
                                &lastUniformIndex);
    }

    QUEUE_INITIALIZE(&uniformList);

    for (i = UniformIndex; i <= lastUniformIndex; i ++)
    {
        /* Get uniform. */
        VIR_Id      id  = VIR_IdList_GetId(&pShader->uniforms, i);
        VIR_Symbol  *sym = VIR_Shader_GetSymFromId(pShader, id);
        VIR_Type    *symType = VIR_Symbol_GetType(sym);
        VIR_Type    *baseType = gcvNULL;
        VIR_Uniform *symUniform = gcvNULL;
        gctUINT32 components = 0, rows = 0;

        if (!_VIR_CG_isUniformAllocable(sym, handleDefaultUBO, unblockUniformBlock, &symUniform))
        {
            continue;
        }

        if (!alwaysAllocate &&
            !isSymUniformUsedInShader(sym) &&
            !isSymUniformImplicitlyUsed(sym) &&
            !VIR_Uniform_AlwaysAlloc(pShader, sym))
        {
            continue;
        }

        restricted = _VIR_CG_IsUniformRestricted(sym);

        /* skip samplers */
        if(VIR_Symbol_isSampler(sym) && !TreatSamplerAsConst)
        {
            continue;
        }

        baseType = VIR_Shader_GetTypeFromId(pShader, VIR_Type_GetBaseTypeId(symType));
        components = VIR_Symbol_GetComponents(sym);
        rows = VIR_Uniform_GetRealUseArraySize(symUniform) * VIR_Type_GetVirRegCount(pShader, baseType, -1);

        if (maxComp < components)
            maxComp = components;

        arraySize += rows;

        /* put this uniform to a queue */
        _VIR_CG_UniformListQueue(pMM, &uniformList, symUniform);
    }

    switch (maxComp)
    {
    case 1:
        type = VIR_TYPE_FLOAT32;
        break;
    case 2:
        type = VIR_TYPE_FLOAT_X2;
        break;
    case 3:
        type = VIR_TYPE_FLOAT_X3;
        break;
    case 4:
        type = VIR_TYPE_FLOAT_X4;
        break;
    case 8:
    case 16:
    case 32:
        type = VIR_TYPE_FLOAT_X4;
        break;
    default:
        gcmASSERT(0);
        if (NextUniformIndex)
        {
            *NextUniformIndex = lastUniformIndex + 1;
        }
        return retValue;
    }

    if (arraySize > 0)
    {
        if (Initialized)
        {
            gcmASSERT(lastUniformIndex == UniformIndex);

            if (!VIR_CG_ConstUniformExistBefore(pShader, pSym, pUniform))
            {
                retValue = VIR_CG_FindUniformUse(uniformColorMap,
                                        type,
                                        arraySize,
                                        restricted,
                                        &physical,
                                        &swizzle,
                                        &shift);

                ON_ERROR(retValue, "Failed to Allocate Uniform");

                pUniform->swizzle  = swizzle;
                pUniform->physical = physical;
                pUniform->address  = uniformBaseAddress +
                                       pUniform->physical * 16 + shift * 4;
            }
        }
        else
        {
            retValue = VIR_CG_FindUniformUse(uniformColorMap,
                                        type,
                                        arraySize,
                                        restricted,
                                        &physical,
                                        &swizzle,
                                        &shift);
            ON_ERROR(retValue, "Failed to Allocate Uniform");

            /* Set physical address for each uniform in the queue. */
            while(!QUEUE_CHECK_EMPTY(&uniformList))
            {
                VIR_Uniform *symUniform = gcvNULL;
                VIR_Symbol  *sym;
                VIR_Type    *baseType;

                _VIR_CG_UniformListDequeue(pMM, &uniformList, &symUniform);

                sym = VIR_Shader_GetSymFromId(pShader, VIR_Uniform_GetSymID(symUniform));

                if (VIR_Symbol_HasFlag(sym, VIR_SYMUNIFORMFLAG_ATOMIC_COUNTER))
                {
                    VIR_Symbol  *baseUniformSym = VIR_Shader_GetSymFromId(pShader, symUniform->baseBindingUniform);
                    VIR_Uniform *baseUniform    = VIR_Symbol_GetUniform(baseUniformSym);
                    gcmASSERT(symUniform->baseBindingUniform != VIR_INVALID_ID);

                    if(baseUniform->physical == -1)
                    {
                        gctUINT32    rows           = 0;
                        VIR_Type    *symType        = VIR_Symbol_GetType(baseUniformSym);

                        baseType = VIR_Shader_GetTypeFromId(pShader,
                                                            VIR_Type_GetBaseTypeId(symType));

                        baseUniform->swizzle  = swizzle;
                        baseUniform->physical = physical;
                        baseUniform->address  = uniformBaseAddress + baseUniform->physical * 16 + shift * 4;

                        rows = VIR_Uniform_GetRealUseArraySize(baseUniform) * VIR_Type_GetVirRegCount(pShader, baseType, -1);
                        physical += rows;
                    }

                    symUniform->swizzle  = baseUniform->swizzle;
                    symUniform->physical = baseUniform->physical;
                    symUniform->address  = baseUniform->address;
                }
                else
                {
                    gctUINT32 rows = 0;
                    VIR_Type  *symType = VIR_Symbol_GetType(sym);

                    baseType = VIR_Shader_GetTypeFromId(pShader,
                                                        VIR_Type_GetBaseTypeId(symType));

                    gcmASSERT(!VIR_Symbol_isSampler(sym) || TreatSamplerAsConst);

                    symUniform->swizzle = swizzle;
                    symUniform->physical = physical;
                    symUniform->address = uniformBaseAddress + symUniform->physical * 16 + shift * 4;

                    rows = VIR_Uniform_GetRealUseArraySize(symUniform) * VIR_Type_GetVirRegCount(pShader, baseType, -1);
                    physical += rows;
                }
            }
        }
    }

OnError:
    QUEUE_FINALIZE(&uniformList);

    if (NextUniformIndex)
        *NextUniformIndex = lastUniformIndex + 1;

    if (pUniformSize)
    {
        *pUniformSize = (gctUINT)arraySize;
    }

    return retValue;
}

VSC_ErrCode _VIR_CG_MapSamplerUniforms(
    IN VIR_Shader       *pShader,
    IN VSC_HW_CONFIG    *pHwConfig,
    IN VIR_Uniform      *pUniform,
    IN VIR_RA_ColorMap  *uniformColorMap,
    IN gctINT           codeGenUniformBase,
    IN gctBOOL          handleDefaultUBO,
    IN gctBOOL          unblockUniformBlock,
    IN gctBOOL          allocateSamplerReverse,
    IN gctBOOL          alwaysAllocate,
    IN gctINT           maxSampler,
    IN VSC_MM           *pMM,
    OUT gctUINT         *pSamplerSize,
    OUT gctINT          *sampler
    )
{
    VSC_ErrCode retValue = VSC_ERR_NONE;
    gctINT arrayLength = 1;
    VIR_Symbol  *pSym = VIR_Shader_GetSymFromId(pShader, VIR_Uniform_GetSymID(pUniform));
    gctBOOL   treatSamplerAsConst = gcvFALSE;

    if (isSymUniformTreatSamplerAsConst(pSym) &&
        isSymUniformUsedInShader(pSym))
    {
        treatSamplerAsConst = gcvTRUE;

        retValue = _VIR_CG_MapNonSamplerUniforms(pShader,
            pHwConfig,
            pUniform,
            gcvFALSE,
            uniformColorMap,
            codeGenUniformBase,
            handleDefaultUBO,
            unblockUniformBlock,
            gcvTRUE,
            gcvFALSE,
            alwaysAllocate,
            pMM,
            pSamplerSize,
            gcvNULL);
        ON_ERROR(retValue, "Failed to Allocate Uniform");
    }
    else
    {
        VIR_Type    *symType = VIR_Symbol_GetType(pSym);

        if (VIR_Type_GetKind(symType) == VIR_TY_ARRAY)
        {
            arrayLength = (gctINT)VIR_Type_GetArrayLength(symType);
        }

        /* Test if sampler is in range */
        if (allocateSamplerReverse)
        {
            if (*sampler < maxSampler)
            {
                retValue = VSC_ERR_OUT_OF_SAMPLER;
                goto OnError;
            }
            else
            {
                if (treatSamplerAsConst)
                {
                    VIR_Uniform_SetSamplerPhysical(pUniform, *sampler + 1 - arrayLength);
                }
                else
                {
                    VIR_Uniform_SetPhysical(pUniform, *sampler + 1 - arrayLength);
                }
                *sampler -= arrayLength;
            }
        }
        else
        {
            if (*sampler >= maxSampler)
            {
                retValue = VSC_ERR_OUT_OF_SAMPLER;
                goto OnError;
            }
            else
            {
                if (treatSamplerAsConst)
                {
                    VIR_Uniform_SetSamplerPhysical(pUniform, *sampler);
                }
                else
                {
                    VIR_Uniform_SetPhysical(pUniform, *sampler);
                }

                if (VIR_Type_GetKind(symType) == VIR_TY_ARRAY)
                {
                    *sampler += arrayLength;
                }
                else
                {
                    *sampler += 1;
                }
            }
        }

        if (treatSamplerAsConst)
        {
            if (VIR_Uniform_GetSamplerPhysical(pUniform) != VIR_Uniform_GetOrigPhysical(pUniform))
            {
                VIR_Shader_SetNeedToAjustSamplerPhysical(pShader, gcvTRUE);
            }
        }
        else
        {
            if (VIR_Uniform_GetPhysical(pUniform) != VIR_Uniform_GetOrigPhysical(pUniform))
            {
                VIR_Shader_SetNeedToAjustSamplerPhysical(pShader, gcvTRUE);
            }
        }


        if (pSamplerSize)
        {
            *pSamplerSize = (gctUINT)arrayLength;
        }
    }

OnError:
    return retValue;
}

/* allocate uniforms without layout table */
VSC_ErrCode VIR_CG_MapUniforms(
    IN VIR_Shader       *pShader,
    IN VSC_HW_CONFIG    *pHwConfig,
    IN VSC_MM           *pMM
    )
{
    VSC_ErrCode         retValue = VSC_ERR_NONE;
    gctINT              i, nextUniformIndex = 0;
    gctBOOL             unblockUniformBlock = gcvFALSE;
    gctBOOL             handleDefaultUBO = gcvFALSE;
    gctBOOL             allocateSamplerReverse = gcvFALSE;
    gctINT              maxSampler = 0, sampler = 0;
    VIR_RA_ColorMap     uniformColorMap;
    gctUINT             codeGenUniformBase;

    /* initialize the colorMap */
    _VIR_CG_UniformColorMap_Init(
        pShader,
        pHwConfig,
        pMM,
        &uniformColorMap,
        &codeGenUniformBase);

    /* Config sampler setting. */
    _VIR_CG_ConfigSamplers(pShader, pHwConfig, &maxSampler, &sampler, &allocateSamplerReverse);

    /* set the handleDefaultUBO flag */
    _VIR_CG_isUBOSupported(pShader, pHwConfig, &handleDefaultUBO, &unblockUniformBlock);

    /* check uniform usage: if a uniform is used in shader or LTC expression */
    VSC_CheckUniformUsage(pShader);

    /* Map all uniforms. */
    for (i = 0; i < (gctINT) VIR_IdList_Count(&pShader->uniforms); ++i)
    {
        VIR_Id      id  = VIR_IdList_GetId(&pShader->uniforms, i);
        VIR_Symbol  *sym = VIR_Shader_GetSymFromId(pShader, id);
        VIR_Uniform *symUniform = gcvNULL;

        if (!_VIR_CG_isUniformAllocable(sym, handleDefaultUBO, unblockUniformBlock, &symUniform))
        {
            continue;
        }

        if (_VIR_CG_isSamplerType(sym))
        {
            /* This physical address of base sampler symbol is always 0. */
            if (VIR_Symbol_GetIndex(sym) == VIR_Shader_GetBaseSamplerId(pShader))
            {
                VIR_Uniform_SetPhysical(symUniform, 0);
                continue;
            }

            /* If this texture is not used on shader, we can skip it. */
            if (!isSymUniformUsedInShader(sym) &&
                !isSymUniformUsedInTextureSize(sym) &&
                /* If this texture is treated as a const, it can be used in LTC. */
                !isSymUniformUsedInLTC(sym) &&
                !VIR_Uniform_AlwaysAlloc(pShader, sym))
            {
                VIR_Uniform_SetPhysical(symUniform, -1);
                VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_INACTIVE);
                continue;
            }

            VIR_Symbol_ClrFlag(sym, VIR_SYMFLAG_INACTIVE);

            retValue = _VIR_CG_MapSamplerUniforms(pShader,
                    pHwConfig,
                    symUniform,
                    &uniformColorMap,
                    codeGenUniformBase,
                    handleDefaultUBO,
                    unblockUniformBlock,
                    allocateSamplerReverse,
                    gcvFALSE, /* always allocate */
                    maxSampler,
                    pMM,
                    gcvNULL,
                    &sampler);
            ON_ERROR(retValue, "Failed to Allocate Uniform");
        }
        else
        {
            if (nextUniformIndex > i)
            {
                continue;
            }

            VIR_Symbol_ClrFlag(sym, VIR_SYMFLAG_INACTIVE);

            /* skip the uniform not used in the shader */
            if (!isSymUniformUsedInShader(sym) &&
                !isSymUniformImplicitlyUsed(sym) &&
                !VIR_Uniform_AlwaysAlloc(pShader, sym))
            {
                if (!isSymUniformForcedToActive(sym) &&
                    !isSymUniformUsedInLTC(sym))
                {
                    VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_INACTIVE);
                }

                continue;
            }

            retValue = _VIR_CG_MapNonSamplerUniforms(pShader,
                pHwConfig,
                symUniform,
                isSymUniformCompiletimeInitialized(sym),
                &uniformColorMap,
                codeGenUniformBase,
                handleDefaultUBO,
                unblockUniformBlock,
                gcvFALSE, /* treat sampler as const */
                gcvFALSE, /* single uniform */
                gcvFALSE, /* always allocate */
                pMM,
                gcvNULL,
                &nextUniformIndex);
            ON_ERROR(retValue, "Failed to Allocate Uniform");

        }
    }

OnError:
    vscBV_Finalize(&uniformColorMap.usedColor);

    return retValue;
}

static gctBOOL _VIG_CG_IsUniformPushConst(
    IN VIR_Shader           *pShader,
    IN VIR_Symbol           *pSym,
    IN VIR_Type             *pType,
    OUT gctBOOL             *pIsBaseAddr
    )
{
    gctBOOL                 bIsPushConst = gcvFALSE;
    gctBOOL                 bIsBaseAddr = gcvFALSE;
    VIR_Uniform             *pUniform = gcvNULL;

    if (VIR_Symbol_GetUniformKind(pSym) == VIR_UNIFORM_PUSH_CONSTANT &&
        VIR_Type_GetKind(pType) != VIR_TY_STRUCT)
    {
        bIsPushConst = gcvTRUE;
    }
    else if (VIR_Symbol_isUniform(pSym))
    {
        pUniform = VIR_Symbol_GetUniform(pSym);
        gcmASSERT(pUniform);

        if (VIR_Uniform_IsPushConstantBaseAddr(pUniform))
        {
            bIsPushConst = gcvTRUE;
            bIsBaseAddr = gcvTRUE;
        }
    }

    if (pIsBaseAddr)
    {
        *pIsBaseAddr = bIsBaseAddr;
    }

    return bIsPushConst;
}

static void _VIR_CG_FindPushConstUniform(
    IN VIR_Shader           *pShader,
    IN VSC_MM               *pMM,
    IN VSC_SHADER_PUSH_CONSTANT_RANGE *pConstRange,
    OUT gctINT              *maxAlignment,
    OUT VSC_SIMPLE_QUEUE    *pushConstList,
    OUT gctBOOL             *pushConstUBO
    )
{
    VIR_Uniform     *pushConstUniform = gcvNULL;
    gctUINT         i;

    for (i = 0; i < (gctINT) VIR_IdList_Count(&pShader->uniforms); ++i)
    {
        VIR_Id      id  = VIR_IdList_GetId(&pShader->uniforms, i);
        VIR_Symbol  *sym = VIR_Shader_GetSymFromId(pShader, id);
        VIR_Type    *symType = VIR_Symbol_GetType(sym);
        VIR_Uniform *uniform = gcvNULL;
        gctBOOL     bIsBaseAddr = gcvFALSE;

        /* Skip non-push-const uniform. */
        if (!_VIG_CG_IsUniformPushConst(pShader, sym, symType, &bIsBaseAddr))
        {
            continue;
        }

        /* Check the range. */
        if (bIsBaseAddr)
        {
            VIR_Symbol          *pUboSym;
            VIR_UniformBlock    *pUbo;

            uniform = VIR_Symbol_GetUniform(sym);
            pUboSym = VIR_Shader_GetSymFromId(pShader, uniform->u.parentSSBOOrUBO);
            pUbo = VIR_Symbol_GetUBO(pUboSym);

            if (VIR_Symbol_GetLayoutOffset(sym) == pConstRange->offset)
            {
                if (VIR_UBO_GetBlockSize(pUbo) != (pConstRange->size + pConstRange->offset))
                {
                    gcmASSERT(gcvFALSE);
                }

                pushConstUniform = uniform;
                _VIR_CG_UniformListQueue(pMM, pushConstList, pushConstUniform);

                if (pushConstUBO)
                {
                    *pushConstUBO = gcvTRUE;
                }

                break;
            }
        }
        else
        {
            if (VIR_Symbol_GetLayoutOffset(sym) >= pConstRange->offset &&
                VIR_Symbol_GetLayoutOffset(sym) + VIR_Type_GetTypeByteSize(pShader, symType) <= pConstRange->offset + pConstRange->size)
            {
                pushConstUniform = VIR_Symbol_GetUniform(sym);
                _VIR_CG_UniformListQueue(pMM, pushConstList, pushConstUniform);
                /* the alignment of struct is the largest base alignment */
                if (VIR_Type_GetAlignement(symType) > *maxAlignment)
                {
                    *maxAlignment = VIR_Type_GetAlignement(symType);
                }
            }
        }
    }
}

static gctUINT
_VIR_CG_CalculateSkipOffset(
    IN gctUINT startOffsetInByte
    )
{
    return (startOffsetInByte / 16) * 16;
}

static VSC_ErrCode
_VIR_CG_AllocatePushConst(
    IN VIR_RA_ColorMap *uniformColorMap,
    IN gctUINT maxAlignment,
    IN gctUINT fixedStartOffsetInByte,
    IN gctUINT pushConstSizeInByte,
    OUT gctINT *physical,
    OUT gctUINT8 *swizzle)
{
    VSC_ErrCode retValue = VSC_ERR_NONE;
    gctUINT     totalSizeInByte = fixedStartOffsetInByte + pushConstSizeInByte;
    gctUINT     allocateRegCount = (totalSizeInByte / 16) + (totalSizeInByte % 16 == 0 ? 0 : 1);
    VIR_TypeId  allocTyId = VIR_TYPE_FLOAT32;
    gctINT      shift = 0;

    if (allocateRegCount > 1)
    {
        allocTyId = VIR_TYPE_FLOAT_X4;
    }
    else
    {
        switch (totalSizeInByte / 4)
        {
        case 1:
            allocTyId = VIR_TYPE_FLOAT32;
            break;

        case 2:
            allocTyId = VIR_TYPE_FLOAT_X2;
            break;

        case 3:
            allocTyId = VIR_TYPE_FLOAT_X3;
            break;

        case 4:
            allocTyId = VIR_TYPE_FLOAT_X4;
            break;

        default:
            gcmASSERT(gcvFALSE);
            allocTyId = VIR_TYPE_FLOAT_X4;
            break;
        }
    }

    retValue = VIR_CG_FindUniformUse(uniformColorMap, allocTyId, allocateRegCount, gcvFALSE, physical, swizzle, &shift);

    return retValue;
}

static VIR_Swizzle _VIR_CG_SwizzleShiftWrap(
    IN VIR_Swizzle          origSwizzle,
    IN gctUINT              componentOffset,
    IN gctUINT              componentCount
    )
{
    VIR_Swizzle             newSwizzle = origSwizzle;
    VIR_Swizzle             channelSwizzle[4];
    gctUINT                 i;

    for (i = 0; i < 4; i++)
    {
        channelSwizzle[i] = VIR_Swizzle_GetChannel(origSwizzle, i);
    }

    switch (componentOffset)
    {
        case 0:
            if (componentCount == 1)
            {
                newSwizzle = VIR_Swizzle_ComposeSwizzle(channelSwizzle[0], channelSwizzle[0], channelSwizzle[0], channelSwizzle[0]);
            }
            else if (componentCount == 2)
            {
                newSwizzle = VIR_Swizzle_ComposeSwizzle(channelSwizzle[0], channelSwizzle[1], channelSwizzle[1], channelSwizzle[1]);
            }
            else if (componentCount == 3)
            {
                newSwizzle = VIR_Swizzle_ComposeSwizzle(channelSwizzle[0], channelSwizzle[1], channelSwizzle[2], channelSwizzle[2]);
            }
            else
            {
                newSwizzle = VIR_Swizzle_ComposeSwizzle(channelSwizzle[0], channelSwizzle[1], channelSwizzle[2], channelSwizzle[3]);
            }
            break;

        case 1:
            if (componentCount == 1)
            {
                newSwizzle = VIR_Swizzle_ComposeSwizzle(channelSwizzle[1], channelSwizzle[1], channelSwizzle[1], channelSwizzle[1]);
            }
            else if (componentCount == 2)
            {
                newSwizzle = VIR_Swizzle_ComposeSwizzle(channelSwizzle[1], channelSwizzle[2], channelSwizzle[2], channelSwizzle[2]);
            }
            else if (componentCount == 3)
            {
                newSwizzle = VIR_Swizzle_ComposeSwizzle(channelSwizzle[1], channelSwizzle[2], channelSwizzle[3], channelSwizzle[3]);
            }
            else
            {
                gcmASSERT(gcvFALSE);
                newSwizzle = VIR_SWIZZLE_XYZW;
            }
            break;

        case 2:
            if (componentCount == 1)
            {
                newSwizzle = VIR_Swizzle_ComposeSwizzle(channelSwizzle[2], channelSwizzle[2], channelSwizzle[2], channelSwizzle[2]);
            }
            else if (componentCount == 2)
            {
                newSwizzle = VIR_Swizzle_ComposeSwizzle(channelSwizzle[2], channelSwizzle[3], channelSwizzle[3], channelSwizzle[3]);
            }
            else
            {
                gcmASSERT(gcvFALSE);
                newSwizzle = VIR_SWIZZLE_XYZW;
            }
            break;

        case 3:
            if (componentCount == 1)
            {
                newSwizzle = VIR_Swizzle_ComposeSwizzle(channelSwizzle[3], channelSwizzle[3], channelSwizzle[3], channelSwizzle[3]);
            }
            else
            {
                gcmASSERT(gcvFALSE);
                newSwizzle = VIR_SWIZZLE_XYZW;
            }
            break;

        default:
            gcmASSERT(gcvFALSE);
            newSwizzle = VIR_SWIZZLE_XYZW;
            break;
    }

    return newSwizzle;
}

static void _VIR_CG_AssignPushConstUniform(
    IN VIR_Shader           *pShader,
    IN VSC_MM               *pMM,
    IN VSC_SHADER_PUSH_CONSTANT_RANGE   *pConstRange,
    IN gctUINT              skipOffsetInByte,
    IN gctUINT              maxAlignment,
    IN VSC_SIMPLE_QUEUE     *pushConstList,
    IN gctINT               physical,
    IN gctUINT8             origSwizzle)
{
    while(!QUEUE_CHECK_EMPTY(pushConstList))
    {
        VIR_Symbol          *sym;
        VIR_Type            *type;
        VIR_TypeId          typeId;
        gctUINT             offset, componentOffset, componentCount;
        VIR_Swizzle         swizzle = VIR_SWIZZLE_XXXX;
        VIR_Uniform         *pushConstUniform = gcvNULL;

        _VIR_CG_UniformListDequeue(pMM, pushConstList, &pushConstUniform);

        sym = VIR_Shader_GetSymFromId(pShader, VIR_Uniform_GetSymID(pushConstUniform));

        /* Get offset. */
        offset = VIR_Symbol_GetLayoutOffset(sym) - skipOffsetInByte;

        /* Get basic type. */
        type = VIR_Symbol_GetType(sym);
        while (VIR_Type_isArray(type))
        {
            type = VIR_Shader_GetTypeFromId(pShader, VIR_Type_GetBaseTypeId(type));
        }
        typeId = VIR_Type_GetIndex(type);

        /* Get component offset. */
        componentOffset = (offset % 16) / 4;
        componentCount = VIR_GetTypeComponents(VIR_GetTypeRowType(typeId));

        /* Get swizzle. */
        swizzle = _VIR_CG_SwizzleShiftWrap((VIR_Swizzle)origSwizzle, componentOffset, componentCount);

        /* Set physical/swizzle. */
        pushConstUniform->physical = physical + offset / 16;
        pushConstUniform->swizzle = (gctUINT8)swizzle;
    }
}

static void
_VIR_CG_AllocateSampledImage(
    IN VIR_Shader*                  pShader,
    IN VSC_SHADER_RESOURCE_LAYOUT*  pResLayout,
    IN VIR_Symbol*                  pSampledImage,
    IN VIR_Uniform*                 pUniform
    )
{
    VIR_Symbol*                     pSeparateSamplerSym;
    VIR_Uniform*                    pSeparateSamplerUniform;
    gctINT                          index = VIR_Symbol_GetSamplerIdxRange(pSampledImage);

    if (index < 0)
    {
        /* We meet a sampled image with a non-constant sampler(dynamic indexing), we can't support it yet. */
        gcmASSERT(index == -1);
        gcmASSERT(gcvFALSE);
        index = 0;
    }

    pSeparateSamplerUniform = VIR_Symbol_GetHwMappingSeparateSamplerUniform(pResLayout, pShader, pSampledImage);

    if (pSeparateSamplerUniform == gcvNULL)
    {
        pSeparateSamplerSym = VIR_Symbol_GetSeparateSampler(pShader, pSampledImage);
        gcmASSERT(pSeparateSamplerSym != gcvNULL);
        pSeparateSamplerUniform = VIR_Symbol_GetUniformPointer(pShader, pSeparateSamplerSym);
    }

    /* Get the address from the separate sampler. */
    pUniform->swizzle  = pSeparateSamplerUniform->swizzle;
    pUniform->physical = pSeparateSamplerUniform->physical + index;
    pUniform->address  = pSeparateSamplerUniform->address;

    return;
}

/* allocate the uniform based on the shader resource layout */
VSC_ErrCode VIR_CG_MapUniformsWithLayout(
    IN VIR_Shader                   *pShader,
    IN VSC_HW_CONFIG                *pHwConfig,
    IN VSC_SHADER_RESOURCE_LAYOUT   *pResLayout,
    IN VSC_HASH_TABLE               *pUnbindUniformHash,
    IN VSC_MM                       *pMM
    )
{
    VSC_ErrCode                       retValue = VSC_ERR_NONE;
    gctBOOL                           unblockUniformBlock = gcvFALSE;
    gctBOOL                           handleDefaultUBO = gcvFALSE;
    gctBOOL                           allocateSamplerReverse = gcvFALSE;
    gctINT                            maxSampler = 0, sampler = 0;
    VIR_RA_ColorMap                   uniformColorMap;
    gctUINT                           codeGenUniformBase, i;
    VIR_SHADER_RESOURCE_ALLOC_LAYOUT* pResAllocLayout = &pShader->shaderResAllocLayout;

    /* initialize the colorMap */
    _VIR_CG_UniformColorMap_Init(
        pShader,
        pHwConfig,
        pMM,
        &uniformColorMap,
        &codeGenUniformBase);

    /* config the sampler setting */
    _VIR_CG_ConfigSamplers(pShader, pHwConfig, &maxSampler, &sampler, &allocateSamplerReverse);

    /* set the handleDefaultUBO flag */
    _VIR_CG_isUBOSupported(pShader, pHwConfig, &handleDefaultUBO, &unblockUniformBlock);

    /* check uniform usage: if a uniform is used in shader or LTC expression */
    VSC_CheckUniformUsage(pShader);

    /* Intialize res-alloc-layout */
    memset(pResAllocLayout, 0, sizeof(VIR_SHADER_RESOURCE_ALLOC_LAYOUT));
    if (pResLayout->resourceBindingCount)
    {
        pResAllocLayout->pResAllocEntries = (VIR_SHADER_RESOURCE_ALLOC_ENTRY*)vscMM_Alloc(&pShader->pmp.mmWrapper,
                                            sizeof(VIR_SHADER_RESOURCE_ALLOC_ENTRY) * pResLayout->resourceBindingCount);
        pResAllocLayout->resAllocEntryCount = pResLayout->resourceBindingCount;

        for (i = 0; i < pResLayout->resourceBindingCount; i ++)
        {
            memset(&pResAllocLayout->pResAllocEntries[i], 0, sizeof(VIR_SHADER_RESOURCE_ALLOC_ENTRY));
            pResAllocLayout->pResAllocEntries[i].hwRegNo = NOT_ASSIGNED;
        }
    }
    if (pResLayout->pushConstantRangeCount)
    {
        pResAllocLayout->pPushCnstAllocEntries = (VIR_SHADER_PUSH_CONSTANT_ALLOC_ENTRY*)vscMM_Alloc(&pShader->pmp.mmWrapper,
                                     sizeof(VIR_SHADER_PUSH_CONSTANT_ALLOC_ENTRY) * pResLayout->pushConstantRangeCount);
        pResAllocLayout->pushCnstAllocEntryCount = pResLayout->pushConstantRangeCount;

        for (i = 0; i < pResLayout->pushConstantRangeCount; i ++)
        {
            memset(&pResAllocLayout->pPushCnstAllocEntries[i], 0, sizeof(VIR_SHADER_PUSH_CONSTANT_ALLOC_ENTRY));
            pResAllocLayout->pPushCnstAllocEntries[i].hwRegNo = NOT_ASSIGNED;
        }
    }

    /* allocate the resources inside pResLayout*/
    for (i = 0; i < pResLayout->resourceBindingCount; i++)
    {
        VSC_SHADER_RESOURCE_BINDING resBinding = pResLayout->pResBindings[i];
        VIR_Uniform*    pUniformArray[2] = { gcvNULL, gcvNULL };
        VIR_UniformKind uniformKind = VIR_Resouce_ResType2UniformKind(resBinding.type);
        VIR_Symbol      *pSym = gcvNULL;
        gctUINT         uniformSize = 0;
        gctUINT         j, resCount = 0;

        memcpy(&pResAllocLayout->pResAllocEntries[i].resBinding, &resBinding, sizeof(VSC_SHADER_RESOURCE_BINDING));

        /*
        ** According to vulkan spec:
        **      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER descriptor set entries can also be accessed via
        **      separate sampler and sampled image shader variables.
        ** So we may get two uniforms for a resource descriptorSet/binding pair.
        */
        resCount = VIR_Resouce_FindResUniform(pShader, uniformKind, &resBinding, VIR_FIND_RES_MODE_RES_ONLY, pUniformArray);

        /* allocate even if it does not appear in the shader */
        if (resCount == 0)
        {
            gctINT physical, shift;
            gctUINT8 swizzle;

            /* not find in the shader */
            retValue = VIR_CG_FindUniformUse(&uniformColorMap,
                                  VIR_TYPE_FLOAT_X4,
                                  resBinding.arraySize,
                                  gcvTRUE,
                                  &physical,
                                  &swizzle,
                                  &shift);
            ON_ERROR(retValue, "Failed to Allocate Uniform");

            pResAllocLayout->pResAllocEntries[i].bUse = gcvFALSE;
            pResAllocLayout->pResAllocEntries[i].hwRegNo = physical;
            pResAllocLayout->pResAllocEntries[i].hwRegRange = resBinding.arraySize;
            pResAllocLayout->pResAllocEntries[i].swizzle = swizzle;

            continue;
        }

        /* So far only COMBINED_IMAGE_SAMPLER can map to two uniforms. */
        if (resCount == 2)
        {
            gcmASSERT(resBinding.type == VSC_SHADER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER);
        }

        /* Found, allocate all uniforms. */
        for (j = 0; j < resCount; j++)
        {
            VIR_Uniform*    pUniform = pUniformArray[j];

            gcmASSERT(pUniform);

            pSym = VIR_Shader_GetSymFromId(pShader, VIR_Uniform_GetSymID(pUniform));
            VIR_Symbol_ClrFlag(pSym, VIR_SYMFLAG_INACTIVE);

            switch (resBinding.type)
            {
            case VSC_SHADER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER:
            {
                gctBOOL bIsSeparateImage = (j == 1);

                if (bIsSeparateImage)
                {
                    /* For a separate image, we need to allocate a image uniform for it, and we may also need to allocate a sampler for it. */
                    gcmASSERT(VIR_Uniform_IsImageCanBeSampled(pUniform));

                    if (!pHwConfig->hwFeatureFlags.supportSeparatedTex)
                    {
                        retValue = _VIR_CG_MapNonSamplerUniforms(pShader,
                            pHwConfig,
                            pUniform,
                            gcvFALSE,
                            &uniformColorMap,
                            codeGenUniformBase,
                            handleDefaultUBO,
                            unblockUniformBlock,
                            gcvFALSE, /* treat sampler as const */
                            gcvTRUE, /* single uniform */
                            gcvTRUE, /* always allocate */
                            pMM,
                            &uniformSize,
                            gcvNULL);
                        ON_ERROR(retValue, "Failed to Allocate Uniform");

                        gcmASSERT(uniformSize <= resBinding.arraySize);
                    }
                    else
                    {
                        gcmASSERT(gcvFALSE);
                    }
                }
                else if (VIR_Uniform_IsTreatTexelBufferAsImg(pUniform))
                {
                    retValue = _VIR_CG_MapNonSamplerUniforms(pShader,
                        pHwConfig,
                        pUniform,
                        gcvFALSE,
                        &uniformColorMap,
                        codeGenUniformBase,
                        handleDefaultUBO,
                        unblockUniformBlock,
                        gcvTRUE, /* treat sampler as const */
                        gcvTRUE, /* single uniform */
                        gcvTRUE, /* always allocate */
                        pMM,
                        &uniformSize,
                        gcvNULL);

                    if (VIR_Uniform_IsTreatTexelBufferAsImg(pUniform))
                    {
                        pResAllocLayout->pResAllocEntries[i].resFlag |= VIR_SRE_FLAG_TREAT_TEXELBUFFER_AS_IMAGE;
                    }
                }
                else
                {
                    /* sampler allocation */
                    retValue = _VIR_CG_MapSamplerUniforms(pShader,
                        pHwConfig,
                        pUniform,
                        &uniformColorMap,
                        codeGenUniformBase,
                        handleDefaultUBO,
                        unblockUniformBlock,
                        allocateSamplerReverse,
                        gcvTRUE, /* always allocate */
                        maxSampler,
                        pMM,
                        &uniformSize,
                        &sampler);
                    ON_ERROR(retValue, "Failed to Allocate Uniform");
                }
                gcmASSERT(uniformSize <= resBinding.arraySize);

                if (!bIsSeparateImage)
                {
                    pResAllocLayout->pResAllocEntries[i].bUse = gcvTRUE;
                    pResAllocLayout->pResAllocEntries[i].hwRegNo = pUniform->physical;
                    pResAllocLayout->pResAllocEntries[i].hwRegRange = uniformSize;
                    pResAllocLayout->pResAllocEntries[i].swizzle = pUniform->swizzle;
                }
                else
                {
                    pResAllocLayout->pResAllocEntries[i].pCombinedSampledImage =
                        (VIR_SHADER_RESOURCE_ALLOC_ENTRY*)vscMM_Alloc(&pShader->pmp.mmWrapper, sizeof(VIR_SHADER_RESOURCE_ALLOC_ENTRY));
                    memset(pResAllocLayout->pResAllocEntries[i].pCombinedSampledImage, 0, sizeof(VIR_SHADER_RESOURCE_ALLOC_ENTRY));

                    memcpy(&pResAllocLayout->pResAllocEntries[i].pCombinedSampledImage->resBinding, &resBinding, sizeof(VSC_SHADER_RESOURCE_BINDING));

                    pResAllocLayout->pResAllocEntries[i].pCombinedSampledImage->resBinding.type = VSC_SHADER_RESOURCE_TYPE_SAMPLED_IMAGE;
                    pResAllocLayout->pResAllocEntries[i].pCombinedSampledImage->bUse = gcvTRUE;
                    pResAllocLayout->pResAllocEntries[i].pCombinedSampledImage->hwRegNo = pUniform->physical;
                    pResAllocLayout->pResAllocEntries[i].pCombinedSampledImage->hwRegRange = uniformSize;
                    pResAllocLayout->pResAllocEntries[i].pCombinedSampledImage->swizzle = pUniform->swizzle;
                }

                break;
            }

            case VSC_SHADER_RESOURCE_TYPE_UNIFORM_TEXEL_BUFFER:
            {
                if (VIR_Uniform_IsTreatTexelBufferAsImg(pUniform))
                {
                    retValue = _VIR_CG_MapNonSamplerUniforms(pShader,
                        pHwConfig,
                        pUniform,
                        gcvFALSE,
                        &uniformColorMap,
                        codeGenUniformBase,
                        handleDefaultUBO,
                        unblockUniformBlock,
                        gcvTRUE, /* treat sampler as const */
                        gcvTRUE, /* single uniform */
                        gcvTRUE, /* always allocate */
                        pMM,
                        &uniformSize,
                        gcvNULL);

                    pResAllocLayout->pResAllocEntries[i].resFlag |= VIR_SRE_FLAG_TREAT_TEXELBUFFER_AS_IMAGE;
                }
                else
                {
                    /* sampler allocation */
                    retValue = _VIR_CG_MapSamplerUniforms(pShader,
                        pHwConfig,
                        pUniform,
                        &uniformColorMap,
                        codeGenUniformBase,
                        handleDefaultUBO,
                        unblockUniformBlock,
                        allocateSamplerReverse,
                        gcvTRUE, /* always allocate */
                        maxSampler,
                        pMM,
                        &uniformSize,
                        &sampler);
                    ON_ERROR(retValue, "Failed to Allocate Uniform");
                }
                gcmASSERT(uniformSize <= resBinding.arraySize);

                pResAllocLayout->pResAllocEntries[i].bUse = gcvTRUE;
                pResAllocLayout->pResAllocEntries[i].hwRegNo = pUniform->physical;
                pResAllocLayout->pResAllocEntries[i].hwRegRange = uniformSize;
                pResAllocLayout->pResAllocEntries[i].swizzle = pUniform->swizzle;

                break;
            }

            case VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE:
            case VSC_SHADER_RESOURCE_TYPE_STORAGE_TEXEL_BUFFER:
            case VSC_SHADER_RESOURCE_TYPE_STORAGE_BUFFER:
            case VSC_SHADER_RESOURCE_TYPE_STORAGE_BUFFER_DYNAMIC:
            case VSC_SHADER_RESOURCE_TYPE_UNIFORM_BUFFER:
            case VSC_SHADER_RESOURCE_TYPE_UNIFORM_BUFFER_DYNAMIC:
            /* Treat input attachment as a image. */
            case VSC_SHADER_RESOURCE_TYPE_INPUT_ATTACHMENT:
            {
                if (isSymUniformTreatImageAsSampler(pSym))
                {
                    retValue = _VIR_CG_MapSamplerUniforms(pShader,
                        pHwConfig,
                        pUniform,
                        &uniformColorMap,
                        codeGenUniformBase,
                        handleDefaultUBO,
                        unblockUniformBlock,
                        allocateSamplerReverse,
                        gcvTRUE, /* always allocate */
                        maxSampler,
                        pMM,
                        &uniformSize,
                        &sampler);
                }
                else
                {
                    retValue = _VIR_CG_MapNonSamplerUniforms(pShader,
                        pHwConfig,
                        pUniform,
                        gcvFALSE,
                        &uniformColorMap,
                        codeGenUniformBase,
                        handleDefaultUBO,
                        unblockUniformBlock,
                        gcvFALSE, /* treat sampler as const */
                        gcvTRUE, /* single uniform */
                        gcvTRUE, /* always allocate */
                        pMM,
                        &uniformSize,
                        gcvNULL);
                }
                ON_ERROR(retValue, "Failed to Allocate Uniform");

                gcmASSERT(uniformSize <= resBinding.arraySize);

                pResAllocLayout->pResAllocEntries[i].bUse = gcvTRUE;
                pResAllocLayout->pResAllocEntries[i].hwRegNo = pUniform->physical;
                pResAllocLayout->pResAllocEntries[i].hwRegRange = uniformSize;
                pResAllocLayout->pResAllocEntries[i].swizzle = pUniform->swizzle;

                if (isSymUniformTreatImageAsSampler(pSym))
                {
                    pResAllocLayout->pResAllocEntries[i].resFlag |= VIR_SRE_FLAG_TREAT_IA_AS_SAMPLER;
                }

                if (resBinding.type == VSC_SHADER_RESOURCE_TYPE_STORAGE_BUFFER ||
                    resBinding.type == VSC_SHADER_RESOURCE_TYPE_STORAGE_BUFFER_DYNAMIC)
                {
                    VIR_Symbol* sbSym = VIR_Shader_GetSymFromId(pShader, pUniform->u.parentSSBOOrUBO);
                    VIR_StorageBlock* sb = VIR_Symbol_GetSBO(sbSym);

                    gcmASSERT(sb);

                    if (VIR_IB_GetFlags(sb) & VIR_IB_UNSIZED)
                    {
                        /* The last element may be a struct, so we can't get it from variable list.
                        ** Just get it from the SB struct type.
                        */
                        VIR_Type* sbType = VIR_Symbol_GetType(sbSym);
                        VIR_SymIdList* fields;
                        VIR_Id lastElementId;
                        VIR_Symbol* lastElementSym = gcvNULL;
                        VIR_FieldInfo* lastElementFieldInfo = gcvNULL;

                        while (VIR_Type_isArray(sbType))
                        {
                            sbType = VIR_Shader_GetTypeFromId(pShader, VIR_Type_GetBaseTypeId(sbType));
                        }

                        fields = VIR_Type_GetFields(sbType);
                        lastElementId = VIR_IdList_GetId(fields, VIR_IdList_Count(fields) - 1);
                        lastElementSym = VIR_Shader_GetSymFromId(pShader, lastElementId);
                        lastElementFieldInfo = VIR_Symbol_GetFieldInfo(lastElementSym);

                        pResAllocLayout->pResAllocEntries[i].fixedSize = VIR_FieldInfo_GetOffset(lastElementFieldInfo);
                        pResAllocLayout->pResAllocEntries[i].lastElementSize = VIR_FieldInfo_GetArrayStride(lastElementFieldInfo);
                    }
                }

                break;
            }

            case VSC_SHADER_RESOURCE_TYPE_SAMPLER:
                /* no need to assign hw reg */
                if (pUniform->u.samplerOrImageAttr.sampledImageSymId != VIR_INVALID_ID)
                {
                    /* sampler allocation */
                    retValue = _VIR_CG_MapSamplerUniforms(pShader,
                                                          pHwConfig,
                                                          pUniform,
                                                          &uniformColorMap,
                                                          codeGenUniformBase,
                                                          handleDefaultUBO,
                                                          unblockUniformBlock,
                                                          allocateSamplerReverse,
                                                          gcvTRUE, /* always allocate */
                                                          maxSampler,
                                                          pMM,
                                                          &uniformSize,
                                                          &sampler);
                    ON_ERROR(retValue, "Failed to Allocate Uniform");

                    gcmASSERT(uniformSize <= resBinding.arraySize);

                    pResAllocLayout->pResAllocEntries[i].bUse = gcvTRUE;
                    pResAllocLayout->pResAllocEntries[i].hwRegNo = pUniform->physical;
                    pResAllocLayout->pResAllocEntries[i].hwRegRange = uniformSize;
                    pResAllocLayout->pResAllocEntries[i].swizzle = pUniform->swizzle;
                }
                else
                {
                    pResAllocLayout->pResAllocEntries[i].hwRegNo = NOT_ASSIGNED;
                }
                break;

            case VSC_SHADER_RESOURCE_TYPE_SAMPLED_IMAGE:
                /* For a sampled image, we need to allocate a image uniform for it, and we may also need to allocate a sampler for it. */
                gcmASSERT(VIR_Uniform_IsImageCanBeSampled(pUniform));

                if (!pHwConfig->hwFeatureFlags.supportSeparatedTex)
                {
                    retValue = _VIR_CG_MapNonSamplerUniforms(pShader,
                        pHwConfig,
                        pUniform,
                        gcvFALSE,
                        &uniformColorMap,
                        codeGenUniformBase,
                        handleDefaultUBO,
                        unblockUniformBlock,
                        gcvFALSE, /* treat sampler as const */
                        gcvTRUE, /* single uniform */
                        gcvTRUE, /* always allocate */
                        pMM,
                        &uniformSize,
                        gcvNULL);
                    ON_ERROR(retValue, "Failed to Allocate Uniform");

                    gcmASSERT(uniformSize <= resBinding.arraySize);

                    pResAllocLayout->pResAllocEntries[i].bUse = gcvTRUE;
                    pResAllocLayout->pResAllocEntries[i].hwRegNo = pUniform->physical;
                    pResAllocLayout->pResAllocEntries[i].hwRegRange = uniformSize;
                    pResAllocLayout->pResAllocEntries[i].swizzle = pUniform->swizzle;
                }
                else
                {
                    gcmASSERT(gcvFALSE);
                }
                break;

            default:
                break;
            }
        }
    }

    /* allocate push const inside pResLayout */
    for (i = 0; i < pResLayout->pushConstantRangeCount; i++)
    {
        VSC_SHADER_PUSH_CONSTANT_RANGE pushConst = pResLayout->pPushConstantRanges[i];
        gctINT         firstPhysical = NOT_ASSIGNED;
        gctUINT8       firstSwizzle = 0;
        gctINT         maxAlignment = 4;
        gctBOOL        bPushConstUBO = gcvFALSE;

        VSC_SIMPLE_QUEUE    pushConstList;
        gctUINT             pushConstCount = 0;

        QUEUE_INITIALIZE(&pushConstList);

        memcpy(&pResAllocLayout->pPushCnstAllocEntries[i].pushCnstRange, &pushConst, sizeof(VSC_SHADER_PUSH_CONSTANT_RANGE));

        _VIR_CG_FindPushConstUniform(pShader, pMM, &pushConst, &maxAlignment, &pushConstList, &bPushConstUBO);
        pushConstCount = vscUNILST_GetNodeCount(&pushConstList);

        if (bPushConstUBO)
        {
            VIR_Uniform*    pUniform;
            gctUINT         uniformSize = 0;

            gcmASSERT(pushConstCount == 1);

            _VIR_CG_UniformListDequeue(pMM, &pushConstList, &pUniform);
            retValue = _VIR_CG_MapNonSamplerUniforms(pShader,
                                                     pHwConfig,
                                                     pUniform,
                                                     gcvFALSE,
                                                     &uniformColorMap,
                                                     codeGenUniformBase,
                                                     handleDefaultUBO,
                                                     unblockUniformBlock,
                                                     gcvFALSE, /* treat sampler as const */
                                                     gcvTRUE, /* single uniform */
                                                     gcvTRUE, /* always allocate */
                                                     pMM,
                                                     &uniformSize,
                                                     gcvNULL);
            if (retValue != VSC_ERR_NONE)
            {
                QUEUE_FINALIZE(&pushConstList);
                goto OnError;
            }
            gcmASSERT(uniformSize == 1);

            pResAllocLayout->pPushCnstAllocEntries[i].bUse = gcvTRUE;
            pResAllocLayout->pPushCnstAllocEntries[i].bBaseAddr = gcvTRUE;
            pResAllocLayout->pPushCnstAllocEntries[i].hwRegNo = pUniform->physical;
            pResAllocLayout->pPushCnstAllocEntries[i].swizzle = pUniform->swizzle;
        }
        else
        {
            if (pushConstCount != 0)
            {
                /*
                ** Allocate policy:
                **  1) The constant register always begins with channel x, if the offset of the first element is 4, then channel x is not used.
                **  2) Skip the 16X offset, for exampler, if the offset is 20, then we skip the first 16bytes, and change the offset to 4, change the size = size + 4.
                **  2) Contain any variable that need multiple-regs(matrix and array) and whose basic aligment is N/2N,
                **       including array of float/vec2/mat2/mat3x2/mat4x2, and mat2/mat3x2/mat4x2, have been converted to memory in spirv converter.
                */
                gctUINT skipOffsetInByte = _VIR_CG_CalculateSkipOffset(pushConst.offset);

                retValue = _VIR_CG_AllocatePushConst(&uniformColorMap,
                                                     maxAlignment,
                                                     pushConst.offset - skipOffsetInByte,
                                                     pushConst.size,
                                                     &firstPhysical,
                                                     &firstSwizzle);
                if (retValue != VSC_ERR_NONE)
                {
                    QUEUE_FINALIZE(&pushConstList);
                    goto OnError;
                }

                gcmASSERT(firstPhysical != NOT_ASSIGNED);

                pResAllocLayout->pPushCnstAllocEntries[i].bUse = gcvTRUE;
                pResAllocLayout->pPushCnstAllocEntries[i].hwRegNo = firstPhysical;
                pResAllocLayout->pPushCnstAllocEntries[i].swizzle = firstSwizzle;

                _VIR_CG_AssignPushConstUniform(pShader,
                                               pMM,
                                               &pushConst,
                                               skipOffsetInByte,
                                               maxAlignment,
                                               &pushConstList,
                                               firstPhysical,
                                               firstSwizzle);
            }
            else
            {
                pResAllocLayout->pPushCnstAllocEntries[i].bUse = gcvFALSE;
                pResAllocLayout->pPushCnstAllocEntries[i].hwRegNo = firstPhysical;
                pResAllocLayout->pPushCnstAllocEntries[i].swizzle = firstSwizzle;
            }
        }

        QUEUE_FINALIZE(&pushConstList);
    }

    /* allocate the compiler generated uniform and
       check whether all used uniforms are allocated */
    for (i = 0; i < (gctINT) VIR_IdList_Count(&pShader->uniforms); ++i)
    {
        VIR_Id      id  = VIR_IdList_GetId(&pShader->uniforms, i);
        VIR_Symbol  *sym = VIR_Shader_GetSymFromId(pShader, id);
        VIR_Uniform *symUniform = gcvNULL;
        gctUINT     uniformSize = 0;

        if (!_VIR_CG_isUniformAllocable(sym, handleDefaultUBO, unblockUniformBlock, &symUniform))
        {
            continue;
        }

        if (isSymUniformWithResLayout(sym))
        {
            continue;
        }

        if (!isSymUniformUsedInShader(sym) &&
            !isSymUniformUsedInLTC(sym) &&
            !isSymUniformImplicitlyUsed(sym) &&
            !VIR_Uniform_AlwaysAlloc(pShader, sym))
        {
            VIR_Uniform_SetPhysical(symUniform, -1);
            VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_INACTIVE);
            continue;
        }

        /* This physical address of base sampler symbol is always 0. */
        if (VIR_Symbol_GetIndex(sym) == VIR_Shader_GetBaseSamplerId(pShader))
        {
            VIR_Uniform_SetPhysical(symUniform, 0);
            continue;
        }

        VIR_Symbol_ClrFlag(sym, VIR_SYMFLAG_INACTIVE);

        /* If this is an image that related to a texelBuffer, just use copy the texelBuffer. */
        if (VIR_Symbol_GetUniformKind(sym) == VIR_UNIFORM_TEXELBUFFER_TO_IMAGE)
        {
            VIR_Symbol*     pParentSamplerSym;
            VIR_Uniform*    pParentUniform;

            pParentSamplerSym = VIR_Shader_GetSymFromId(pShader, symUniform->u.samplerOrImageAttr.parentSamplerSymId);
            pParentUniform = VIR_Symbol_GetSampler(pParentSamplerSym);;

            symUniform->swizzle  = pParentUniform->swizzle;
            symUniform->physical = pParentUniform->physical;
            symUniform->address  = pParentUniform->address;

            continue;
        }
        else if (VIR_Symbol_GetUniformKind(sym) == VIR_UNIFORM_SAMPLED_IMAGE)
        {
            _VIR_CG_AllocateSampledImage(pShader, pResLayout, sym, symUniform);

            continue;
        }

        if (isSymCompilerGen(sym))
        {
            gcmASSERT(symUniform->physical == -1);

            if (VIR_Symbol_isSampler(sym))
            {
                retValue = _VIR_CG_MapSamplerUniforms(pShader,
                    pHwConfig,
                    symUniform,
                    &uniformColorMap,
                    codeGenUniformBase,
                    handleDefaultUBO,
                    unblockUniformBlock,
                    allocateSamplerReverse,
                    gcvTRUE, /* always allocate */
                    maxSampler,
                    pMM,
                    &uniformSize,
                    &sampler);
                ON_ERROR(retValue, "Failed to Allocate Uniform");
            }
            else
            {
                retValue = _VIR_CG_MapNonSamplerUniforms(pShader,
                    pHwConfig,
                    symUniform,
                    gcvFALSE,
                    &uniformColorMap,
                    codeGenUniformBase,
                    handleDefaultUBO,
                    unblockUniformBlock,
                    gcvFALSE, /* treat sampler as const */
                    gcvTRUE, /* single uniform */
                    gcvFALSE, /* always allocate */
                    pMM,
                    &uniformSize,
                    gcvNULL);
                ON_ERROR(retValue, "Failed to Allocate Uniform");
            }
        }
        else
        {
            /*
            ** If a user-defined ubo/sbo is used in shader but not within the resource layout,
            ** we need to mark it as unused and remove all its usages in the shader.
            */
            if (VIR_Symbol_GetBinding(sym) == NOT_ASSIGNED          &&
                VIR_Symbol_GetDescriptorSet(sym) == NOT_ASSIGNED    &&
                (!_VIG_CG_IsUniformPushConst(pShader, sym, VIR_Symbol_GetType(sym), gcvNULL)) &&
                (VIR_Uniform_GetPhysical(symUniform) == -1) &&
                (VIR_Symbol_GetUniformKind(sym) == VIR_UNIFORM_UNIFORM_BLOCK_ADDRESS ||
                 VIR_Symbol_GetUniformKind(sym) == VIR_UNIFORM_STORAGE_BLOCK_ADDRESS))
            {
                vscHTBL_DirectSet(pUnbindUniformHash, (void *)sym, gcvNULL);

                VIR_Symbol_ClrFlag(sym, VIR_SYMUNIFORMFLAG_USED_IN_SHADER);
                VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_INACTIVE);
            }
        }
    }

OnError:
    vscBV_Finalize(&uniformColorMap.usedColor);

    return retValue;
}

static gctBOOL
VIR_CG_CheckUnBindUniformRelated(
    IN VIR_DEF_USAGE_INFO*          pDuInfo,
    IN VIR_Shader*                  pShader,
    IN VSC_HASH_TABLE*              pUnbindUniformHash,
    IN VIR_Instruction*             pInst,
    IN VIR_Operand*                 pOpnd,
    IN VIR_Symbol*                  pSymbol
    )
{
    gctBOOL                         bFound = gcvFALSE;

    /*
    ** Check if this symbol is in the hash table first, if it is not in the table,
    ** Check if any its DEF is in the hash table.
    */
    if (vscHTBL_TestAndGet(pUnbindUniformHash, (void *)pSymbol, gcvNULL))
    {
        bFound = gcvTRUE;
    }
    else
    {
        VIR_GENERAL_UD_ITERATOR     udIter;
        VIR_DEF*                    pDef;

        vscVIR_InitGeneralUdIterator(&udIter, pDuInfo, pInst, pOpnd, gcvFALSE, gcvFALSE);
        for (pDef = vscVIR_GeneralUdIterator_First(&udIter);
             pDef != gcvNULL;
             pDef = vscVIR_GeneralUdIterator_Next(&udIter))
        {
            VIR_Instruction*        pDefInst = pDef->defKey.pDefInst;
            gctUINT                 i;
            VIR_Operand*            pSrcOpnd = gcvNULL;
            VIR_Symbol*             pSrcOpndSym = gcvNULL;

            gcmASSERT(pDefInst);

            if (VIR_IS_IMPLICIT_DEF_INST(pDefInst))
            {
                continue;
            }

            for (i = 0; i < VIR_Inst_GetSrcNum(pDefInst); i++)
            {
                pSrcOpnd = VIR_Inst_GetSource(pDefInst, i);
                if (!VIR_Operand_isSymbol(pSrcOpnd))
                {
                    continue;
                }
                pSrcOpndSym = VIR_Operand_GetSymbol(pSrcOpnd);

                bFound = VIR_CG_CheckUnBindUniformRelated(pDuInfo,
                                                          pShader,
                                                          pUnbindUniformHash,
                                                          pDefInst,
                                                          pSrcOpnd,
                                                          pSrcOpndSym);
                if (bFound)
                {
                    vscHTBL_DirectSet(pUnbindUniformHash, (void *)pSrcOpndSym, gcvNULL);
                    break;
                }
            }

            if (bFound)
            {
                vscHTBL_DirectSet(pUnbindUniformHash, (void *)pSymbol, gcvNULL);
                break;
            }
        }
    }

    return bFound;
}

/* Remove all instructions that related to a unbind sbo/ubo. */
static VSC_ErrCode
VIR_CG_RemoveInstRelatedToUnbindRes(
    IN VIR_DEF_USAGE_INFO*          pDuInfo,
    IN VIR_Shader*                  pShader,
    IN VSC_HASH_TABLE*              pUnbindUniformHash,
    INOUT gctBOOL*                  pChanged
    )
{
    VSC_ErrCode                     retValue = VSC_ERR_NONE;
    gctBOOL                         bChanged = gcvFALSE;
    VIR_FuncIterator                func_iter;
    VIR_FunctionNode*               pFuncNode;

    /* No unbind resource, just return. */
    if (HTBL_GET_ITEM_COUNT(pUnbindUniformHash) == 0)
    {
        return retValue;
    }

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (pFuncNode = VIR_FuncIterator_First(&func_iter);
         pFuncNode != gcvNULL;
         pFuncNode = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function*               pFunc = pFuncNode->function;
        VIR_InstIterator            inst_iter;
        VIR_Instruction*            pInst;
        VIR_Operand*                pAddrOpnd = gcvNULL;
        VIR_Symbol*                 pAddrSym = gcvNULL;
        VIR_OpCode                  opCode;
        gctBOOL                     bFound = gcvFALSE;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(pFunc));
        for (pInst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             pInst != gcvNULL;
             pInst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            opCode = VIR_Inst_GetOpcode(pInst);
            bFound = gcvFALSE;

            /* Check LOAD/STORE only. */
            if (!VIR_OPCODE_isGlobalMemLd(opCode) && !VIR_OPCODE_isGlobalMemSt(opCode))
            {
                continue;
            }

            pAddrOpnd = VIR_Inst_GetSource(pInst, 0);
            if (!VIR_Operand_isSymbol(pAddrOpnd))
            {
                continue;
            }
            pAddrSym = VIR_Operand_GetSymbol(pAddrOpnd);

            /* Check if the address symbol is within the hash table. */
            bFound = VIR_CG_CheckUnBindUniformRelated(pDuInfo,
                                                      pShader,
                                                      pUnbindUniformHash,
                                                      pInst,
                                                      pAddrOpnd,
                                                      pAddrSym);
            if (!bFound)
            {
                continue;
            }

            /* Change the instruction:
            **      1. Change STORE to NOP.
            **      2. Change LOAD to MOV 0.
            */
            if (VIR_OPCODE_isGlobalMemLd(opCode))
            {
                VIR_Inst_SetOpcode(pInst, VIR_OP_MOV);
                VIR_Inst_SetSrcNum(pInst, 1);
                if (VIR_TypeId_isFloat(VIR_Operand_GetTypeId(VIR_Inst_GetDest(pInst))))
                {
                    VIR_Operand_SetImmediateFloat(pAddrOpnd, 0.0f);
                }
                else if (VIR_TypeId_isUnSignedInteger(VIR_Operand_GetTypeId(VIR_Inst_GetDest(pInst))))
                {
                    VIR_Operand_SetImmediateUint(pAddrOpnd, 0);
                }
                else
                {
                    VIR_Operand_SetImmediateInt(pAddrOpnd, 0);
                }
            }
            else
            {
                gcmASSERT(VIR_OPCODE_isGlobalMemSt(opCode));

                VIR_Function_ChangeInstToNop(pFunc, pInst);

            }

            bChanged = gcvTRUE;
        }
    }

    if (pChanged)
    {
        *pChanged = bChanged;
    }

    return retValue;
}

DEF_QUERY_PASS_PROP(VIR_RA_PerformUniformAlloc)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_CG;
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_RA;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;

    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;
}

DEF_SH_NECESSITY_CHECK(VIR_RA_PerformUniformAlloc)
{
    return gcvTRUE;
}

VSC_ErrCode VIR_RA_PerformUniformAlloc(
    VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode                       retValue  = VSC_ERR_NONE;
    VIR_Shader                        *pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VSC_HW_CONFIG                     *pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    VSC_OPTN_RAOptions                *pOption = (VSC_OPTN_RAOptions*)pPassWorker->basePassWorker.pBaseOption;
    VSC_MM                            *pMM = pPassWorker->basePassWorker.pMM;
    VSC_SHADER_RESOURCE_LAYOUT        *pShResourceLayout = pPassWorker->pCompilerParam->pShResourceLayout;
    VSC_HASH_TABLE                    *pUnbindUniformHash = gcvNULL;
    gctBOOL                           allocUniform = gcvFALSE;

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetOPTS(pOption),
        VSC_OPTN_RAOptions_ALLOC_UNIFORM))
    {
        if (!VIR_Shader_isConstRegAllocated(pShader))
        {
            allocUniform = gcvTRUE;

            if (pShResourceLayout)
            {
                gctBOOL                 bChanged = gcvFALSE;
                VIR_DEF_USAGE_INFO*     pDuInfo = pPassWorker->pDuInfo;

                pUnbindUniformHash = vscHTBL_Create(pMM, vscHFUNC_Default, vscHKCMP_Default, 8);
                retValue = VIR_CG_MapUniformsWithLayout(pShader, pHwCfg, pShResourceLayout, pUnbindUniformHash, pMM);
                ON_ERROR(retValue, "VIR_CG_MapUniformsWithLayout");

                /*
                ** If a user-defined ubo/sbo is used in shader but not within the resource layout,
                ** we need to mark it as unused and remove all its usages in the shader.
                */
                retValue = VIR_CG_RemoveInstRelatedToUnbindRes(pDuInfo, pShader, pUnbindUniformHash, &bChanged);
                ON_ERROR(retValue, "VIR_CG_RemoveInstRelatedToUnbindRes");

                /* We have removed some instructions without updating DU, so we need to mark DU as invalidated. */
                if (bChanged)
                {
                    pPassWorker->pResDestroyReq->s.bInvalidateDu = gcvTRUE;
                }
            }
            else
            {
                retValue = VIR_CG_MapUniforms(pShader, pHwCfg, pMM);
                ON_ERROR(retValue, "VIR_CG_MapUniforms");
            }

            /* set const register allocated to shader */
            VIR_Shader_SetConstRegAllocated(pShader, gcvTRUE);
        }
    }

    if (allocUniform)
    {
        if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
        {
            VIR_Shader_Dump(gcvNULL, "After Uniform allocation", pShader, gcvTRUE);
        }
    }

OnError:
    if (pUnbindUniformHash)
    {
        vscHTBL_Destroy(pUnbindUniformHash);
    }

    return retValue;
}

void _VIR_CG_Unified_UniformColorMap_Init(
    IN VSC_HW_CONFIG        *pHwConfig,
    IN VSC_MM               *pMM,
    IN OUT VIR_RA_ColorMap  *uniformCM,
    OUT gctUINT             *CodeGenUniformBase
    )
{
    uniformCM->maxAllocReg = 0;
    uniformCM->availReg = 0;
    *CodeGenUniformBase = pHwConfig->vsConstRegAddrBase;

    uniformCM->maxReg = pHwConfig->maxTotalConstRegCount;

    /* each const registers needs 4 channels (w, z, y, x) */
    vscBV_Initialize(&uniformCM->usedColor,
        pMM,
        uniformCM->maxReg * VIR_CHANNEL_NUM);
}

static void _VIR_CG_Unified_ConfigSamplers(
    IN VSC_AllShaders   *all_shaders,
    IN VSC_HW_CONFIG    *pHwConfig,
    IN gctINT           *samplerBaseOffset,
    OUT gctINT          *maxSampler,
    OUT gctINT          *sampler,
    OUT gctBOOL         *allocateSamplerReverse
    )
{
    gctUINT i;

    /* Determine starting sampler index. */
    if (sampler)
    {
        *sampler = 0;
    }

    if (maxSampler)
    {
        *maxSampler = vscMIN(pHwConfig->maxSamplerCountPerShader,
                             pHwConfig->maxHwNativeTotalSamplerCount);
    }

    for (i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        VIR_Shader* pShader = VSC_AllShaders_GetShader(all_shaders, i);

        if (pShader)
        {
            /*
            ** If it is a recompiler link, the sampler base offset may be set before,
            ** so we need to save the original value in case unified allocation fail.
            */
            samplerBaseOffset[i] = VIR_Shader_GetSamplerBaseOffset(pShader);
            VIR_Shader_SetSamplerBaseOffset(pShader, 0);
        }
    }
}

static void _VIR_CG_RollBack_MapUniforms(
    IN OUT VSC_AllShaders   *all_shaders,
    IN gctBOOL              handleDefaultUBO,
    IN gctBOOL              unblockUniformBlock,
    IN gctINT               *samplerBaseOffset
    )
{
    gctINT              i, j;

    for (i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        VIR_Shader* pShader = all_shaders->shaders[i];

        if (pShader == gcvNULL)
        {
            continue;
        }

        VIR_Shader_SetSamplerBaseOffset(pShader, samplerBaseOffset[i]);
        VIR_Shader_SetNeedToAjustSamplerPhysical(pShader, gcvFALSE);

        for (j = 0; j < (gctINT) VIR_IdList_Count(&pShader->uniforms); ++j)
        {
            VIR_Id      id  = VIR_IdList_GetId(&pShader->uniforms, j);
            VIR_Symbol  *sym = VIR_Shader_GetSymFromId(pShader, id);
            VIR_Uniform *symUniform = gcvNULL;

            if (!_VIR_CG_isUniformAllocable(sym, handleDefaultUBO, unblockUniformBlock, &symUniform))
            {
                continue;
            }

            VIR_Uniform_SetPhysical(symUniform, -1);
            VIR_Uniform_SetSamplerPhysical(symUniform, -1);
            symUniform->address = (gctUINT32)-1;
            symUniform->swizzle = 0;
        }
    }
}

VSC_ErrCode VIR_CG_Unified_MapUniforms(
    IN OUT VSC_AllShaders   *all_shaders,
    IN VSC_HW_CONFIG        *pHwConfig
    )
{
    VSC_ErrCode         retValue = VSC_ERR_NONE;
    VSC_GlobalUniformTable   *pGlobalUniformTable = &all_shaders->global_uniform_table;
    VSC_MM              *pMM = &all_shaders->mem_pool;
    gctINT              i, j;
    gctBOOL             unblockUniformBlock = gcvFALSE;
    gctBOOL             handleDefaultUBO = gcvFALSE;
    gctBOOL             allocateSamplerReverse = gcvFALSE;
    gctINT              samplerBaseOffset[VSC_MAX_LINKABLE_SHADER_STAGE_COUNT] = {0};
    gctINT              maxSampler = 0, sampler = 0;
    VIR_RA_ColorMap     uniformColorMap;
    gctUINT             codeGenUniformBase;

    VSC_GlobalUniformTable_Iterator iter;
    VSC_GlobalUniformItem* item;

    /* initialize the colorMap */
    _VIR_CG_Unified_UniformColorMap_Init(
        pHwConfig,
        pMM,
        &uniformColorMap,
        &codeGenUniformBase);

    /* Config sampler setting. */
    _VIR_CG_Unified_ConfigSamplers(all_shaders,
        pHwConfig, samplerBaseOffset, &maxSampler, &sampler, &allocateSamplerReverse);

    /* set the handleDefaultUBO flag */
    handleDefaultUBO = gcvTRUE;
    unblockUniformBlock = gcvFALSE;

    /* Map all uniforms. */
    VSC_GlobalUniformTable_Iterator_Init(&iter, pGlobalUniformTable);
    for (item = VSC_GlobalUniformTable_Iterator_First(&iter), i = 0; item != gcvNULL;
         item = VSC_GlobalUniformTable_Iterator_Next(&iter), i++)
    {
        gctBOOL     vAllocated = gcvFALSE;
        VIR_Uniform *preSymUnifom = gcvNULL;

        for (i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
        {
            VIR_SymId uniform_symid = item->uniforms[i];
            if (VIR_Id_isValid(uniform_symid))
            {
                VIR_Shader* pShader = item->all_shaders->shaders[i];
                VIR_Symbol* sym = VIR_Shader_GetSymFromId(pShader, uniform_symid);
                VIR_Uniform* symUniform = gcvNULL;

                if (!_VIR_CG_isUniformAllocable(sym, handleDefaultUBO, unblockUniformBlock, &symUniform))
                {
                    continue;
                }

                if (_VIR_CG_isSamplerType(sym))
                {
                    /* This physical address of base sampler symbol is always 0. */
                    if (VIR_Symbol_GetIndex(sym) == VIR_Shader_GetBaseSamplerId(pShader))
                    {
                        VIR_Uniform_SetPhysical(symUniform, 0);
                        continue;
                    }

                    /* If this texture is not used on shader, we can skip it. */
                    if (!isSymUniformUsedInShader(sym) &&
                        !isSymUniformUsedInTextureSize(sym) &&
                        /* If this texture is treated as a const, it can be used in LTC. */
                        !isSymUniformUsedInLTC(sym) &&
                        !VIR_Uniform_AlwaysAlloc(pShader, sym))
                    {
                        VIR_Uniform_SetPhysical(symUniform, -1);
                        VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_INACTIVE);
                        continue;
                    }

                    VIR_Symbol_ClrFlag(sym, VIR_SYMFLAG_INACTIVE);

                    if (vAllocated)
                    {
                        symUniform->physical = preSymUnifom->physical;

                        VIR_Uniform_SetRealUseArraySize(symUniform,
                            VIR_Uniform_GetRealUseArraySize(preSymUnifom));
                    }
                    else
                    {
                        retValue = _VIR_CG_MapSamplerUniforms(pShader,
                            pHwConfig,
                            symUniform,
                            &uniformColorMap,
                            codeGenUniformBase,
                            handleDefaultUBO,
                            unblockUniformBlock,
                            allocateSamplerReverse,
                            gcvFALSE, /* always allocate */
                            maxSampler,
                            pMM,
                            gcvNULL,
                            &sampler);
                        ON_ERROR(retValue, "Failed to Allocate Uniform");

                        preSymUnifom = symUniform;
                        vAllocated = gcvTRUE;
                    }
                }
                else
                {
                    if (symUniform->physical != -1)
                    {
                        continue;
                    }

                    VIR_Symbol_ClrFlag(sym, VIR_SYMFLAG_INACTIVE);

                    /* skip the uniform not used in the shader */
                    if (!isSymUniformUsedInShader(sym) &&
                        !isSymUniformImplicitlyUsed(sym) &&
                        !VIR_Uniform_AlwaysAlloc(pShader, sym))
                    {
                        if (!isSymUniformForcedToActive(sym) &&
                            !isSymUniformUsedInLTC(sym))
                        {
                            VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_INACTIVE);
                        }

                        continue;
                    }

                    if (vAllocated)
                    {
                        symUniform->address = preSymUnifom->address;
                        symUniform->swizzle = preSymUnifom->swizzle;
                        symUniform->physical = preSymUnifom->physical;

                        /* set the array length */
                        VIR_Uniform_SetRealUseArraySize(symUniform,
                            VIR_Uniform_GetRealUseArraySize(preSymUnifom));
                    }
                    else
                    {
                        retValue = _VIR_CG_MapNonSamplerUniforms(pShader,
                            pHwConfig,
                            symUniform,
                            isSymUniformCompiletimeInitialized(sym),
                            &uniformColorMap,
                            codeGenUniformBase,
                            handleDefaultUBO,
                            unblockUniformBlock,
                            gcvFALSE, /* treat sampler as const */
                            gcvFALSE, /* single uniform */
                            gcvFALSE, /* always allocate */
                            pMM,
                            gcvNULL,
                            gcvNULL);
                        ON_ERROR(retValue, "Failed to Allocate Uniform");

                        preSymUnifom = symUniform;
                        vAllocated = gcvTRUE;
                    }
                }
            }
        }
    }

    /* Allocate the private uniforms. */
    for (i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        VIR_Shader* pShader = all_shaders->shaders[i];

        if (pShader == gcvNULL)
        {
            continue;
        }

        /* check uniform usage: if a uniform is used in shader or LTC expression */
        VSC_CheckUniformUsage(pShader);

        /* Map all the other uniforms. */
        for (j = 0; j < (gctINT) VIR_IdList_Count(&pShader->uniforms); ++j)
        {
            VIR_Id      id  = VIR_IdList_GetId(&pShader->uniforms, j);
            VIR_Symbol  *sym = VIR_Shader_GetSymFromId(pShader, id);
            VIR_Uniform *symUniform = gcvNULL;

            if (!_VIR_CG_isUniformAllocable(sym, handleDefaultUBO, unblockUniformBlock, &symUniform))
            {
                continue;
            }

            /* Skip those uniforms that been allocated. */
            if (VIR_Uniform_GetPhysical(symUniform) != -1)
            {
                continue;
            }

            if (_VIR_CG_isSamplerType(sym))
            {
                /* This physical address of base sampler symbol is always 0. */
                if (VIR_Symbol_GetIndex(sym) == VIR_Shader_GetBaseSamplerId(pShader))
                {
                    VIR_Uniform_SetPhysical(symUniform, 0);
                    continue;
                }

                /* If this texture is not used on shader, we can skip it. */
                if (!isSymUniformUsedInShader(sym) &&
                    !isSymUniformUsedInTextureSize(sym) &&
                    /* If this texture is treated as a const, it can be used in LTC. */
                    !isSymUniformUsedInLTC(sym) &&
                    !VIR_Uniform_AlwaysAlloc(pShader, sym))
                {
                    VIR_Uniform_SetPhysical(symUniform, -1);
                    VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_INACTIVE);
                    continue;
                }

                VIR_Symbol_ClrFlag(sym, VIR_SYMFLAG_INACTIVE);

                retValue = _VIR_CG_MapSamplerUniforms(pShader,
                        pHwConfig,
                        symUniform,
                        &uniformColorMap,
                        codeGenUniformBase,
                        handleDefaultUBO,
                        unblockUniformBlock,
                        allocateSamplerReverse,
                        gcvFALSE, /* always allocate */
                        maxSampler,
                        pMM,
                        gcvNULL,
                        &sampler);
                ON_ERROR(retValue, "Failed to Allocate Uniform");
            }
            else
            {
                VIR_Symbol_ClrFlag(sym, VIR_SYMFLAG_INACTIVE);

                /* skip the uniform not used in the shader */
                if (!isSymUniformUsedInShader(sym) &&
                    !isSymUniformImplicitlyUsed(sym) &&
                    !VIR_Uniform_AlwaysAlloc(pShader, sym))
                {
                    if (!isSymUniformForcedToActive(sym) &&
                        !isSymUniformUsedInLTC(sym))
                    {
                        VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_INACTIVE);
                    }

                    continue;
                }

                retValue = _VIR_CG_MapNonSamplerUniforms(pShader,
                    pHwConfig,
                    symUniform,
                    isSymUniformCompiletimeInitialized(sym),
                    &uniformColorMap,
                    codeGenUniformBase,
                    handleDefaultUBO,
                    unblockUniformBlock,
                    gcvFALSE, /* treat sampler as const */
                    gcvFALSE, /* single uniform */
                    gcvFALSE, /* always allocate */
                    pMM,
                    gcvNULL,
                    gcvNULL);
                ON_ERROR(retValue, "Failed to Allocate Uniform");
            }
        }
    }

    /* Allocate unified uniforms successfully, now we can save the states.*/
    for (i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        VIR_Shader* pShader = all_shaders->shaders[i];

        if (pShader)
        {
            VIR_Shader_SetConstRegAllocated(pShader, gcvTRUE);
            VIR_Shader_SetFullUnifiedUniforms(pShader, gcvTRUE);
        }
    }

OnError:
    /* Meet any error, roll back all changes. */
    if (retValue != VSC_ERR_NONE)
    {
        _VIR_CG_RollBack_MapUniforms(all_shaders,
                                     handleDefaultUBO,
                                     unblockUniformBlock,
                                     samplerBaseOffset);
    }

    vscBV_Finalize(&uniformColorMap.usedColor);

    return retValue;
}

