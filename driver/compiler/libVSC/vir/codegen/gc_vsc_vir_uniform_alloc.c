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
        *CodeGenUniformBase = pHwConfig->psConstRegAddrBase;
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
                samplerRegNoBase = 0;
            }
        }
        else
        {
            *maxSampler = samplerCount;
            *sampler = 0;
        }
    }
    else if (pHwConfig->hwFeatureFlags.hasSamplerBaseOffset)
    {
        *sampler = 0;
        switch (shaderKind)
        {
        case VIR_SHADER_FRAGMENT:
            *maxSampler = pHwConfig->maxPSSamplerCount;
            samplerRegNoBase = pHwConfig->psSamplerRegNoBase;
            break;

        case VIR_SHADER_COMPUTE:
            *maxSampler = pHwConfig->maxCSSamplerCount;
            samplerRegNoBase = pHwConfig->csSamplerRegNoBase;
            break;

        case VIR_SHADER_VERTEX:
            *maxSampler = pHwConfig->maxVSSamplerCount;
            samplerRegNoBase = pHwConfig->vsSamplerRegNoBase;
            break;

        case VIR_SHADER_TESSELLATION_CONTROL:
            *maxSampler = pHwConfig->maxTCSSamplerCount;
            samplerRegNoBase = pHwConfig->tcsSamplerRegNoBase;
            break;

        case VIR_SHADER_TESSELLATION_EVALUATION:
            *maxSampler = pHwConfig->maxTESSamplerCount;
            samplerRegNoBase = pHwConfig->tesSamplerRegNoBase;
            break;

        case VIR_SHADER_GEOMETRY:
            *maxSampler = pHwConfig->maxGSSamplerCount;
            samplerRegNoBase = pHwConfig->gsSamplerRegNoBase;
            break;

        default:
            gcmASSERT(0);
            *maxSampler = pHwConfig->maxPSSamplerCount;
            samplerRegNoBase = pHwConfig->psSamplerRegNoBase;
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

        samplerRegNoBase = 0;
    }

    /* Update the sampler base offset. */
    VIR_Shader_SetSamplerBaseOffset(pShader, samplerRegNoBase);
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


static gctUINT8 _VIR_CG_SwizzleShiftWrap(
    gctUINT8    origSwizzle,
    gctUINT     count,
    gctUINT     step,
    gctBOOL     *wrapAround
    )
{
    gctUINT8 retSwizzle = VIR_SWIZZLE_X;

    switch (step)
    {
    case 1:
        retSwizzle = origSwizzle;
        break;
    case 2:
        if ((count % 2) == 0)
        {
            retSwizzle = origSwizzle;
        }
        else
        {
            switch (origSwizzle)
            {
            case VIR_SWIZZLE_XYYY:
                retSwizzle = VIR_SWIZZLE_ZWWW;
                break;
            case VIR_SWIZZLE_ZWWW:
                retSwizzle = VIR_SWIZZLE_XYYY;
                *wrapAround = gcvTRUE;
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
            retSwizzle = origSwizzle;
        }
        else if ((count % 4) == 1)
        {
            switch (origSwizzle)
            {
            case VIR_SWIZZLE_XXXX:
                retSwizzle = VIR_SWIZZLE_YYYY;
                break;
            case VIR_SWIZZLE_YYYY:
                retSwizzle = VIR_SWIZZLE_ZZZZ;
                break;
            case VIR_SWIZZLE_ZZZZ:
                retSwizzle = VIR_SWIZZLE_WWWW;
                break;
            case VIR_SWIZZLE_WWWW:
                *wrapAround = gcvTRUE;
                retSwizzle = VIR_SWIZZLE_XXXX;
                break;
            default:
                gcmASSERT(gcvFALSE);
                break;
            }
        }
        else if ((count % 4) == 2)
        {
            switch (origSwizzle)
            {
            case VIR_SWIZZLE_XXXX:
                retSwizzle = VIR_SWIZZLE_ZZZZ;
                break;
            case VIR_SWIZZLE_YYYY:
                retSwizzle = VIR_SWIZZLE_WWWW;
                break;
            case VIR_SWIZZLE_ZZZZ:
                *wrapAround = gcvTRUE;
                retSwizzle = VIR_SWIZZLE_XXXX;
                break;
            case VIR_SWIZZLE_WWWW:
                *wrapAround = gcvTRUE;
                retSwizzle = VIR_SWIZZLE_YYYY;
                break;
            default:
                gcmASSERT(gcvFALSE);
                break;
            }
        }
        else
        {
            switch (origSwizzle)
            {
            case VIR_SWIZZLE_XXXX:
                retSwizzle = VIR_SWIZZLE_WWWW;
                break;
            case VIR_SWIZZLE_YYYY:
                *wrapAround = gcvTRUE;
                retSwizzle = VIR_SWIZZLE_XXXX;
                break;
            case VIR_SWIZZLE_ZZZZ:
                *wrapAround = gcvTRUE;
                retSwizzle = VIR_SWIZZLE_YYYY;
                break;
            case VIR_SWIZZLE_WWWW:
                *wrapAround = gcvTRUE;
                retSwizzle = VIR_SWIZZLE_ZZZZ;
                break;
            default:
                gcmASSERT(gcvFALSE);
                break;
            }
        }
        break;
    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    return retSwizzle;
}

/* mark uniform slot used */
void VIR_CG_SetUniformUsedPacked(
    IN VIR_RA_ColorMap  *uniformColorMap,
    IN gctINT           startIdx,
    IN gctINT           rows,
    IN gctUINT8         enable,
    IN gctUINT          step
    )
{
    VSC_BIT_VECTOR  *usedColor = &uniformColorMap->usedColor;
    gctUINT8        firstEnable = enable;
    gctUINT         count = 0;

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

        count ++;
        startIdx = startIdx + count / step;
        enable = _VIR_CG_EnableShiftWrap(firstEnable, count, step);;
    }
}

gctBOOL VIR_CG_UniformAvailablePacked(
    IN VIR_RA_ColorMap      *uniformColorMap,
    IN gctINT               startIdx,
    IN gctINT               rows,
    IN gctUINT8             enable,
    IN gctUINT              step
    )
{
    VSC_BIT_VECTOR  *usedColor = &uniformColorMap->usedColor;
    gctUINT         count = 0;

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

        count++;

        startIdx = startIdx + count / step;

        enable = _VIR_CG_EnableShiftWrap(enable, count, step);
    }

    return gcvTRUE;
}

/* find next available slot and mark it used,
   the packed layout is used */
VSC_ErrCode
VIR_CG_FindUniformUsePacked(
    IN VIR_RA_ColorMap  *uniformColorMap,
    IN VIR_TypeId       type,
    IN gctINT           arraySize,
    IN gctBOOL          restricted,
    OUT gctINT          *Physical,
    OUT gctUINT8        *Swizzle)
{
    gctINT i;
    gctUINT32 components = 0;
    gctINT shift;
    gctUINT8 swizzle = 0, enable = 0;
    gctUINT vec4Size = arraySize, step = 1;

    components = VIR_GetTypeComponents(type);

    switch (components)
    {
    case 1:
        vec4Size = arraySize / 4 + ((arraySize % 4 == 0) ? 0 : 1);
        break;
    case 2:
        vec4Size = arraySize / 2 + ((arraySize % 2 == 0) ? 0 : 1);
        break;
    case 4:
        vec4Size = arraySize;
        break;
    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    if (uniformColorMap->maxReg < vec4Size)
    {
        return VSC_ERR_OUT_OF_RESOURCE;
    }

    for (i = 0; i <= (gctINT) (uniformColorMap->maxReg - vec4Size); ++i)
    {
        shift = -1;

        switch (components)
        {
        case 1:
            step = 4;

            /* See if x-component is available. */
            if (VIR_CG_UniformAvailablePacked(uniformColorMap, i, arraySize, 0x1 << 0, step))
            {
                shift   = 0;
                enable  = VIR_ENABLE_X;
                swizzle = VIR_SWIZZLE_XXXX;
            }

            /* See if y-component is available. */
            else if (!restricted && VIR_CG_UniformAvailablePacked(uniformColorMap, i, arraySize, 0x1 << 1, step))
            {
                shift   = 1;
                enable  = VIR_ENABLE_Y;
                swizzle = VIR_SWIZZLE_YYYY;
            }

            /* See if z-component is available. */
            else if (!restricted && VIR_CG_UniformAvailablePacked(uniformColorMap, i, arraySize, 0x1 << 2, step))
            {
                shift   = 2;
                enable  = VIR_ENABLE_Z;
                swizzle = VIR_SWIZZLE_ZZZZ;
            }

            /* See if w-component is available. */
            else if (!restricted && VIR_CG_UniformAvailablePacked(uniformColorMap, i, arraySize, 0x1 << 3, step))
            {
                shift   = 3;
                enable  = VIR_ENABLE_W;
                swizzle = VIR_SWIZZLE_WWWW;
            }

            break;

        case 2:
            step = 2;
            /* See if x- and y-components are available. */
            if (VIR_CG_UniformAvailablePacked(uniformColorMap, i, arraySize, 0x3 << 0, step))
            {
                shift   = 0;
                enable  = VIR_ENABLE_XY;
                swizzle = VIR_SWIZZLE_XYYY;
            }

            /* See if y- and z-components are available. */
            else if (!restricted && VIR_CG_UniformAvailablePacked(uniformColorMap, i, arraySize, 0x3 << 1, step))
            {
                shift   = 1;
                enable  = VIR_ENABLE_YZ;
                swizzle = VIR_SWIZZLE_YZZZ;
            }

            /* See if z- and w-components are available. */
            else if (!restricted && VIR_CG_UniformAvailablePacked(uniformColorMap, i, arraySize, 0x3 << 2, step))
            {
                shift   = 2;
                enable  = VIR_ENABLE_ZW;
                swizzle = VIR_SWIZZLE_ZWWW;
            }

            break;

        case 3:
            step = 1;
            /* See if x-, y- and z-components are available. */
            if (VIR_CG_UniformAvailablePacked(uniformColorMap, i, arraySize, 0x7 << 0, step))
            {
                shift   = 0;
                enable  = VIR_ENABLE_XYZ;
                swizzle = VIR_SWIZZLE_XYZZ;
            }

            /* See if y-, z- and w-components are available. */
            else if (!restricted && VIR_CG_UniformAvailablePacked(uniformColorMap, i, arraySize, 0x7 << 1, step))
            {
                shift   = 1;
                enable  = VIR_ENABLE_YZW;
                swizzle = VIR_SWIZZLE_YZWW;
            }

            break;

        case 4:
            step = 1;
            /* See if x-, y-, z- and w-components are available. */
            if (VIR_CG_UniformAvailablePacked(uniformColorMap, i, arraySize, 0xF << 0, step))
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

        if (shift >=0 )
        {
            *Physical = i;
            *Swizzle = swizzle;

            /* Set the uniform used */
            VIR_CG_SetUniformUsedPacked(uniformColorMap, i, arraySize, enable, step);

            return VSC_ERR_NONE;
        }
    }

    return VSC_ERR_OUT_OF_RESOURCE;
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
    case VIR_TYPE_INTEGER_X32:
    case VIR_TYPE_INTEGER_X16:
    case VIR_TYPE_INTEGER_X8:
    case VIR_TYPE_UINT_X32:
    case VIR_TYPE_UINT_X16:
    case VIR_TYPE_UINT_X8:
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
    case VIR_TYPE_UINT16_X4:
        valid[3] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_UINT16_X3:
        valid[2] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_UINT16_X2:
        valid[1] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_UINT16:
        valid[0] = gcvTRUE;
        break;
    case VIR_TYPE_INT16_X4:
        valid[3] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INT16_X3:
        valid[2] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INT16_X2:
        valid[1] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INT16:
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
        return gcvFALSE;
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
    VIR_Uniform *pUniform = gcvNULL;

    if (VIR_Symbol_isUniform(pSym))
    {
        pUniform = VIR_Symbol_GetUniform(pSym);
    }
    else if (VIR_Symbol_isImage(pSym))
    {
        pUniform = VIR_Symbol_GetImage(pSym);
    }
    else if (VIR_Symbol_isSampler(pSym))
    {
        pUniform = VIR_Symbol_GetSampler(pSym);
    }

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
        case VIR_UNIFORM_GLOBAL_OFFSET:
        case VIR_UNIFORM_WORK_DIM:
        case VIR_UNIFORM_KERNEL_ARG_CONSTANT:
        case VIR_UNIFORM_KERNEL_ARG_LOCAL_MEM_SIZE:
        case VIR_UNIFORM_KERNEL_ARG_PRIVATE:
        case VIR_UNIFORM_SAMPLE_LOCATION:
        case VIR_UNIFORM_ENABLE_MULTISAMPLE_BUFFERS:
        case VIR_UNIFORM_TEMP_REG_SPILL_MEM_ADDRESS:
        case VIR_UNIFORM_CONST_BORDER_VALUE:
        case VIR_UNIFORM_SAMPLED_IMAGE:
        case VIR_UNIFORM_EXTRA_LAYER:
        case VIR_UNIFORM_PUSH_CONSTANT:
        case VIR_UNIFORM_BASE_INSTANCE:
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
    case VIR_TYPE_SAMPLER_GENERIC:
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
        VIR_Uniform *symUniform = gcvNULL;
        gctUINT32 components = 0, rows = 0;

        if (!_VIR_CG_isUniformAllocable(sym, handleDefaultUBO, unblockUniformBlock, &symUniform))
        {
            continue;
        }

        if (!alwaysAllocate &&
            !isSymUniformUsedInShader(sym) &&
            !isSymUniformImplicitlyUsed(sym) &&
            VIR_Symbol_GetUniformKind(sym) != VIR_UNIFORM_NUM_GROUPS)
        {
            continue;
        }

        /*
        ** When allocate constant register for #num_group, we should always allocate XYZ for it
        ** because HW always use XYZ for this uniform, so is base instance.
        */
        if (VIR_Symbol_GetUniformKind(sym) == VIR_UNIFORM_NUM_GROUPS    ||
            VIR_Symbol_GetUniformKind(sym) == VIR_UNIFORM_BASE_INSTANCE ||
            VIR_Symbol_GetCannotShift(sym))
            restricted = gcvTRUE;

        /* skip samplers */
        if(VIR_Symbol_isSampler(sym) && !TreatSamplerAsConst)
        {
            continue;
        }

        components = VIR_Symbol_GetComponents(sym);

        if (VIR_Uniform_GetRealUseArraySize(symUniform) == -1)
        {
            rows = VIR_Type_GetVirRegCount(pShader, symType, -1);

            if (VIR_Type_GetKind(symType) == VIR_TY_ARRAY)
            {
                VIR_Uniform_SetRealUseArraySize(symUniform, VIR_Type_GetArrayLength(symType));
            }
            else
            {
                VIR_Uniform_SetRealUseArraySize(symUniform, 1);
            }
        }
        else
        {
            VIR_Type * baseType = VIR_Shader_GetTypeFromId(pShader,
                                                            VIR_Type_GetBaseTypeId(symType));
            rows = VIR_Uniform_GetRealUseArraySize(symUniform) * VIR_Type_GetVirRegCount(pShader, baseType, -1);
        }

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

                        if (VIR_Uniform_GetRealUseArraySize(baseUniform) == -1)
                        {
                            rows = VIR_Type_GetVirRegCount(pShader, symType, -1);
                        }
                        else
                        {
                            rows = VIR_Uniform_GetRealUseArraySize(baseUniform) * VIR_Type_GetVirRegCount(pShader, baseType, -1);
                        }
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

                    if (VIR_Uniform_GetRealUseArraySize(symUniform) == -1)
                    {
                        rows = VIR_Type_GetVirRegCount(pShader, symType, -1);
                    }
                    else
                    {
                        rows = VIR_Uniform_GetRealUseArraySize(symUniform) * VIR_Type_GetVirRegCount(pShader, baseType, -1);
                    }
                    physical += rows;
                }
            }
        }
    }

OnError:
    QUEUE_FINALIZE(&uniformList);

    if (NextUniformIndex)
        *NextUniformIndex = lastUniformIndex + 1;

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
    OUT gctINT          *sampler
    )
{
    VSC_ErrCode retValue = VSC_ERR_NONE;

    VIR_Symbol  *pSym = VIR_Shader_GetSymFromId(pShader, VIR_Uniform_GetSymID(pUniform));
    gctBOOL   allocateSamplerPhysical = gcvFALSE;

    if (isSymUniformTreatSamplerAsConst(pSym) &&
        isSymUniformUsedInShader(pSym))
    {
        allocateSamplerPhysical = gcvTRUE;

        _VIR_CG_MapNonSamplerUniforms(pShader,
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
            gcvNULL);
    }
    else
    {
        VIR_Type    *symType = VIR_Symbol_GetType(pSym);

        /* Allocate sampler for a const-texture because driver need this. */
        if (VIR_Uniform_GetRealUseArraySize(pUniform) == -1)
        {
            if (VIR_Type_GetKind(symType) == VIR_TY_ARRAY)
            {
                VIR_Uniform_SetRealUseArraySize(pUniform, VIR_Type_GetArrayLength(symType));
            }
            else
            {
                VIR_Uniform_SetRealUseArraySize(pUniform, 1);
            }
        }

        /* Test if sampler is in range */
        if (allocateSamplerReverse)
        {
            if (*sampler < maxSampler)
            {
                retValue = VSC_ERR_OUT_OF_SAMPLER;
                goto OnRet;
            }
            else
            {
                gctINT arrayLength = 1;

                if (VIR_Type_GetKind(symType) == VIR_TY_ARRAY)
                {
                    arrayLength = (gctINT)VIR_Type_GetArrayLength(symType);
                }

                if (allocateSamplerPhysical)
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
                goto OnRet;
            }
            else
            {
                if (allocateSamplerPhysical)
                {
                    VIR_Uniform_SetSamplerPhysical(pUniform, *sampler);
                }
                else
                {
                    VIR_Uniform_SetPhysical(pUniform, *sampler);
                }

                if (VIR_Type_GetKind(symType) == VIR_TY_ARRAY)
                {
                    *sampler += VIR_Type_GetArrayLength(symType);
                }
                else
                {
                    *sampler += 1;
                }
            }
        }

        if (allocateSamplerPhysical)
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
    }

OnRet:
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
                !isSymUniformUsedInLTC(sym))
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
                VIR_Symbol_GetUniformKind(sym) != VIR_UNIFORM_NUM_GROUPS)
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
                &nextUniformIndex);
            ON_ERROR(retValue, "Failed to Allocate Uniform");

        }
    }

OnError:
    vscBV_Finalize(&uniformColorMap.usedColor);

    return retValue;
}

VIR_Uniform* _VIR_CG_FindResUniform(
    IN VIR_Shader           *pShader,
    IN VIR_UniformKind      uniformKind,
    IN gctUINT              setNo,
    IN gctUINT              binding,
    IN gctUINT              arraySize)
{
    VIR_Uniform     *retUniform = gcvNULL;
    gctUINT         i;

    for (i = 0; i < (gctINT) VIR_IdList_Count(&pShader->uniforms); ++i)
    {
        VIR_Id      id  = VIR_IdList_GetId(&pShader->uniforms, i);
        VIR_Symbol  *sym = VIR_Shader_GetSymFromId(pShader, id);
        VIR_Uniform *symUniform = gcvNULL;
        VIR_Type    *symType = VIR_Symbol_GetType(sym);
        gctUINT     thisArraySize;

        if (VIR_Type_GetKind(symType) == VIR_TY_STRUCT)
        {
            continue;
        }

        if (VIR_Symbol_isUniform(sym))
        {
           symUniform = VIR_Symbol_GetUniform(sym);
        }
        else if (VIR_Symbol_isSampler(sym))
        {
            symUniform = VIR_Symbol_GetSampler(sym);
        }
        else if (VIR_Symbol_isImage(sym))
        {
            symUniform = VIR_Symbol_GetImage(sym);
        }

        if (symUniform == gcvNULL ||
            VIR_Symbol_GetUniformKind(sym) != uniformKind)
        {
            continue;
        }

        if (VIR_Type_GetKind(symType) == VIR_TY_ARRAY)
        {
            thisArraySize = VIR_Type_GetArrayLength(symType);
        }
        else
        {
            thisArraySize = 1;
        }

        if (VIR_Symbol_GetDescriptorSet(sym) == setNo &&
            VIR_Symbol_GetBinding(sym) == binding &&
            thisArraySize == arraySize)
        {
            retUniform = symUniform;
            break;
        }
    }

    return retUniform;
}

static void _VIR_CG_FindPushConstUniform(
    IN VIR_Shader           *pShader,
    IN VSC_MM               *pMM,
    IN VSC_SHADER_PUSH_CONSTANT_RANGE *pConstRange,
    OUT gctINT              *maxAlignment,
    OUT VSC_SIMPLE_QUEUE    *pushConstList)
{
    VIR_Uniform     *pushCosntUniform = gcvNULL;
    gctUINT         i;

    for (i = 0; i < (gctINT) VIR_IdList_Count(&pShader->uniforms); ++i)
    {
        VIR_Id      id  = VIR_IdList_GetId(&pShader->uniforms, i);
        VIR_Symbol  *sym = VIR_Shader_GetSymFromId(pShader, id);
        VIR_Type    *symType = VIR_Symbol_GetType(sym);

        if (VIR_Type_GetKind(symType) == VIR_TY_STRUCT ||
            VIR_Symbol_GetUniformKind(sym) != VIR_UNIFORM_PUSH_CONSTANT)
        {
            continue;
        }

        if (VIR_Symbol_GetLayoutOffset(sym) >= pConstRange->offset &&
            VIR_Symbol_GetLayoutOffset(sym) + VIR_Type_GetTypeByteSize(pShader, symType) <= pConstRange->offset + pConstRange->size)
        {
            pushCosntUniform = sym->u2.uniform;
            _VIR_CG_UniformListQueue(pMM, pushConstList, pushCosntUniform);
            /* the alignment of struct is the largest base alignment */
            if (VIR_Type_GetAlignement(symType) > *maxAlignment)
            {
                *maxAlignment = VIR_Type_GetAlignement(symType);
            }
        }
    }
}

static VSC_ErrCode
_VIR_CG_AllocatePushConst(
    IN VIR_RA_ColorMap *uniformColorMap,
    IN gctUINT maxAlignment,
    IN gctUINT maxSize,
    OUT gctINT *physical,
    OUT gctUINT8 *swizzle)
{
    VSC_ErrCode retValue = VSC_ERR_NONE;
    gctUINT     allocateSize = (maxSize / maxAlignment) + (maxSize % maxAlignment == 0 ? 0 : 1);
    VIR_TypeId  allocTyId = VIR_TYPE_FLOAT32;

    switch (maxAlignment)
    {
    case 4:
        allocTyId = VIR_TYPE_FLOAT32;
        break;
    case 8:
        allocTyId = VIR_TYPE_FLOAT_X2;
        break;
    case 16:
        allocTyId = VIR_TYPE_FLOAT_X4;
        break;
    default:
        break;
    }

    retValue = VIR_CG_FindUniformUsePacked(uniformColorMap, allocTyId, allocateSize, gcvFALSE, physical, swizzle);

    return retValue;
}

static void _VIR_CG_AssignPushConstUniform(
    IN VIR_Shader           *pShader,
    IN VSC_MM               *pMM,
    IN VSC_SHADER_PUSH_CONSTANT_RANGE   *pConstRange,
    IN gctUINT              maxAlignment,
    IN VSC_SIMPLE_QUEUE     *pushConstList,
    IN gctINT               physical,
    IN gctUINT8             swizzle)
{
    VIR_Uniform     *pushCosntUniform = gcvNULL;
    gctUINT         step = 1;

    switch (maxAlignment)
    {
    case 16:
        step = 1;
        break;
    case 8:
        step = 2;
        break;
    case 4:
        step = 4;
        break;
    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    while(!QUEUE_CHECK_EMPTY(pushConstList))
    {
        VIR_Symbol  *sym;
        gctUINT     distance = 0;
        gctBOOL     wrapRound = gcvFALSE;

        _VIR_CG_UniformListDequeue(pMM, pushConstList, &pushCosntUniform);

        sym = VIR_Shader_GetSymFromId(pShader, VIR_Uniform_GetSymID(pushCosntUniform));
        distance = (VIR_Symbol_GetLayoutOffset(sym) - pConstRange->offset) / maxAlignment;
        gcmASSERT(((VIR_Symbol_GetLayoutOffset(sym) - pConstRange->offset) % maxAlignment) == 0);
        pushCosntUniform->swizzle = _VIR_CG_SwizzleShiftWrap(swizzle, distance, step, &wrapRound);
        pushCosntUniform->physical = physical +  (distance / step) + ((wrapRound) ? 1 : 0);
    }
}

/* resource type to uniform kind */
VIR_UniformKind _VIR_CG_ResType2UniformKind(
    VSC_SHADER_RESOURCE_TYPE    resType)
{
    VIR_UniformKind uniformKind = VIR_UNIFORM_NORMAL;

    switch(resType)
    {
    case VSC_SHADER_RESOURCE_TYPE_SAMPLER:
    case VSC_SHADER_RESOURCE_TYPE_UNIFORM_TEXEL_BUFFER:
    case VSC_SHADER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER:
    case VSC_SHADER_RESOURCE_TYPE_SAMPLED_IMAGE:
    case VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE:
    case VSC_SHADER_RESOURCE_TYPE_STORAGE_TEXEL_BUFFER:
        uniformKind = VIR_UNIFORM_NORMAL;
        break;

    case VSC_SHADER_RESOURCE_TYPE_UNIFORM_BUFFER:
    case VSC_SHADER_RESOURCE_TYPE_UNIFORM_BUFFER_DYNAMIC:
        uniformKind = VIR_UNIFORM_UNIFORM_BLOCK_ADDRESS;
        break;

    case VSC_SHADER_RESOURCE_TYPE_STORAGE_BUFFER:
    case VSC_SHADER_RESOURCE_TYPE_STORAGE_BUFFER_DYNAMIC:
        uniformKind = VIR_UNIFORM_STORAGE_BLOCK_ADDRESS;
        break;

    case VSC_SHADER_RESOURCE_TYPE_INPUT_ATTACHMENT:
        break;

    default:
        break;
    }

    return uniformKind;
}

/* allocate the uniform based on the shader resource layout */
VSC_ErrCode VIR_CG_MapUniformsWithLayout(
    IN VIR_Shader                   *pShader,
    IN VSC_HW_CONFIG                *pHwConfig,
    IN VSC_SHADER_RESOURCE_LAYOUT   *pResLayout,
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
        VIR_Uniform     *pUniform = gcvNULL;
        VIR_UniformKind uniformKind = _VIR_CG_ResType2UniformKind(resBinding.type);
        VIR_Symbol      *pSym = gcvNULL;

        memcpy(&pResAllocLayout->pResAllocEntries[i].resBinding, &resBinding, sizeof(VSC_SHADER_RESOURCE_BINDING));

        pUniform = _VIR_CG_FindResUniform(pShader, uniformKind, resBinding.set, resBinding.binding, resBinding.arraySize);

        if (pUniform != gcvNULL)
        {
            pSym = VIR_Shader_GetSymFromId(pShader, VIR_Uniform_GetSymID(pUniform));
            VIR_Symbol_ClrFlag(pSym, VIR_SYMFLAG_INACTIVE);

            switch (resBinding.type)
            {
            case VSC_SHADER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER:
            case VSC_SHADER_RESOURCE_TYPE_UNIFORM_TEXEL_BUFFER:
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
                    &sampler);
                ON_ERROR(retValue, "Failed to Allocate Uniform");

                pResAllocLayout->pResAllocEntries[i].bUse = gcvTRUE;
                pResAllocLayout->pResAllocEntries[i].hwRegNo = pUniform->physical;
                pResAllocLayout->pResAllocEntries[i].swizzle = pUniform->swizzle;

                break;
            }

            case VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE:
            case VSC_SHADER_RESOURCE_TYPE_STORAGE_TEXEL_BUFFER:
            case VSC_SHADER_RESOURCE_TYPE_STORAGE_BUFFER:
            case VSC_SHADER_RESOURCE_TYPE_STORAGE_BUFFER_DYNAMIC:
            case VSC_SHADER_RESOURCE_TYPE_UNIFORM_BUFFER:
            case VSC_SHADER_RESOURCE_TYPE_UNIFORM_BUFFER_DYNAMIC:
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
                    gcvNULL);
                ON_ERROR(retValue, "Failed to Allocate Uniform");

                pResAllocLayout->pResAllocEntries[i].bUse = gcvTRUE;
                pResAllocLayout->pResAllocEntries[i].hwRegNo = pUniform->physical;
                pResAllocLayout->pResAllocEntries[i].swizzle = pUniform->swizzle;

                if (resBinding.type == VSC_SHADER_RESOURCE_TYPE_STORAGE_BUFFER ||
                    resBinding.type == VSC_SHADER_RESOURCE_TYPE_STORAGE_BUFFER_DYNAMIC)
                {
                    VIR_Symbol* sbSym = VIR_Shader_GetSymFromId(pShader, pUniform->u.parentSSBO);
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

            case VSC_SHADER_RESOURCE_TYPE_INPUT_ATTACHMENT:
            case VSC_SHADER_RESOURCE_TYPE_SAMPLER:
            case VSC_SHADER_RESOURCE_TYPE_SAMPLED_IMAGE:
                /* no need to assign hw reg */
                pResAllocLayout->pResAllocEntries[i].hwRegNo = NOT_ASSIGNED;
                break;

            default:
                break;
            }
        }
        else
        {
            /* allocate even if it does not appear in the shader */
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
            pResAllocLayout->pResAllocEntries[i].swizzle = swizzle;
        }
    }

    /* allocate push const inside pResLayout */
    for (i = 0; i < pResLayout->pushConstantRangeCount; i++)
    {
        VSC_SHADER_PUSH_CONSTANT_RANGE pushConst = pResLayout->pPushConstantRanges[i];
        gctINT         firstPhysical = NOT_ASSIGNED;
        gctUINT8       firstSwizzle = 0;
        gctINT         maxAlignment = 4;

        VSC_SIMPLE_QUEUE    pushConstList;

        QUEUE_INITIALIZE(&pushConstList);

        memcpy(&pResAllocLayout->pPushCnstAllocEntries[i].pushCnstRange, &pushConst, sizeof(VSC_SHADER_PUSH_CONSTANT_RANGE));

        _VIR_CG_FindPushConstUniform(pShader, pMM, &pushConst, &maxAlignment, &pushConstList);

        retValue = _VIR_CG_AllocatePushConst(&uniformColorMap, maxAlignment, pushConst.size, &firstPhysical, &firstSwizzle);
        if (retValue != VSC_ERR_NONE)
        {
            QUEUE_FINALIZE(&pushConstList);
            goto OnError;
        }

        gcmASSERT(firstPhysical != NOT_ASSIGNED);

        pResAllocLayout->pPushCnstAllocEntries[i].bUse = gcvTRUE;
        pResAllocLayout->pPushCnstAllocEntries[i].hwRegNo = firstPhysical;
        pResAllocLayout->pPushCnstAllocEntries[i].swizzle = firstSwizzle;

        _VIR_CG_AssignPushConstUniform(pShader, pMM, &pushConst, maxAlignment, &pushConstList, firstPhysical, firstSwizzle);

        QUEUE_FINALIZE(&pushConstList);
    }

    /* allocate the compiler generated uniform and
       check whether all used uniforms are allocated */
    for (i = 0; i < (gctINT) VIR_IdList_Count(&pShader->uniforms); ++i)
    {
        VIR_Id      id  = VIR_IdList_GetId(&pShader->uniforms, i);
        VIR_Symbol  *sym = VIR_Shader_GetSymFromId(pShader, id);
        VIR_Uniform *symUniform = gcvNULL;

        if (!_VIR_CG_isUniformAllocable(sym, handleDefaultUBO, unblockUniformBlock, &symUniform))
        {
            continue;
        }

        if (!isSymUniformUsedInShader(sym) &&
            !isSymUniformUsedInLTC(sym) &&
            !isSymUniformImplicitlyUsed(sym) &&
            VIR_Symbol_GetUniformKind(sym) != VIR_UNIFORM_NUM_GROUPS)
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
                    gcvNULL);
                ON_ERROR(retValue, "Failed to Allocate Uniform");
            }
        }
    }

OnError:
    vscBV_Finalize(&uniformColorMap.usedColor);

    return retValue;
}

static VSC_ErrCode _AppendTempRegSpillMemAddrUniform(VIR_Shader *pShader)
{
    VSC_ErrCode  virErrCode;
    VIR_NameId   spillMemName;
    VIR_SymId    spillMemSymId;
    VIR_Symbol*  spillMemSym;
    VIR_Uniform *virUniform = gcvNULL;

    virErrCode = VIR_Shader_AddString(pShader, "#TempRegSpillMemAddr", &spillMemName);
    ON_ERROR(virErrCode, "Failed to VIR_Shader_AddString");

    /* default ubo symbol */
    virErrCode = VIR_Shader_AddSymbol(pShader,
                                      VIR_SYM_UNIFORM,
                                      spillMemName,
                                      VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT32),
                                      VIR_STORAGE_UNKNOWN,
                                      &spillMemSymId);

    spillMemSym = VIR_Shader_GetSymFromId(pShader, spillMemSymId);
    VIR_Symbol_SetUniformKind(spillMemSym, VIR_UNIFORM_TEMP_REG_SPILL_MEM_ADDRESS);
    VIR_Symbol_SetFlag(spillMemSym, VIR_SYMUNIFORMFLAG_USED_IN_SHADER);
    VIR_Symbol_SetFlag(spillMemSym, VIR_SYMFLAG_COMPILER_GEN);
    VIR_Symbol_SetLocation(spillMemSym, -1);

    virUniform = VIR_Symbol_GetUniform(spillMemSym);
    virUniform->index = VIR_IdList_Count(VIR_Shader_GetUniforms(pShader)) - 1;

OnError:
    return virErrCode;
}

DEF_QUERY_PASS_PROP(VIR_RA_PerformUniformAlloc)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_CG;
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_RA;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;
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

    /* Append an uniform (one channel) for temp register spill mem address.
       This code should be moved to an phase that determine whether we need
       do temp register spill before uniform allocation */
    retValue = _AppendTempRegSpillMemAddrUniform(pShader);
    CHECK_ERROR(retValue, "Append temp-reg spill mem address uniform");

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetOPTS(pOption),
        VSC_OPTN_RAOptions_ALLOC_UNIFORM))
    {
        if (pShResourceLayout)
        {
            retValue = VIR_CG_MapUniformsWithLayout(pShader, pHwCfg, pShResourceLayout, pMM);
            CHECK_ERROR(retValue, "VIR_CG_MapUniformsWithLayout");
        }
        else
        {
            retValue = VIR_CG_MapUniforms(pShader, pHwCfg, pMM);
            CHECK_ERROR(retValue, "VIR_CG_MapUniforms");
        }

        /* set const register allocated to shader */
        VIR_Shader_SetConstRegAllocated(pShader, gcvTRUE);
    }

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After Uniform allocation", pShader, gcvTRUE);
    }

    return retValue;
}

