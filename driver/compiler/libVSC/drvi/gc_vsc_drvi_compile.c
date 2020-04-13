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


/* This file is triggered by CompileShader API call of any client. A SEP may be generated
   if cFlags tells it should generate that. */

#include "gc_vsc.h"
#include "vir/lower/gc_vsc_vir_hl_2_hl.h"
#include "vir/lower/gc_vsc_vir_hl_2_ml.h"
#include "vir/lower/gc_vsc_vir_ml_2_ll.h"
#include "vir/lower/gc_vsc_vir_ll_2_mc.h"
#include "vir/transform/gc_vsc_vir_scalarization.h"
#include "vir/transform/gc_vsc_vir_peephole.h"
#include "vir/transform/gc_vsc_vir_simplification.h"
#include "vir/transform/gc_vsc_vir_misc_opts.h"
#include "vir/transform/gc_vsc_vir_cpp.h"
#include "vir/transform/gc_vsc_vir_dce.h"
#include "vir/transform/gc_vsc_vir_cpf.h"
#include "vir/transform/gc_vsc_vir_cse.h"
#include "vir/transform/gc_vsc_vir_inline.h"
#include "vir/transform/gc_vsc_vir_fcp.h"
#include "vir/transform/gc_vsc_vir_loop.h"
#include "vir/transform/gc_vsc_vir_cfo.h"
#include "vir/transform/gc_vsc_vir_uniform.h"
#include "vir/transform/gc_vsc_vir_static_patch.h"
#include "vir/transform/gc_vsc_vir_vectorization.h"
#include "vir/transform/gc_vsc_vir_param_opts.h"
#include "vir/codegen/gc_vsc_vir_inst_scheduler.h"
#include "vir/codegen/gc_vsc_vir_reg_alloc.h"
#include "vir/codegen/gc_vsc_vir_mc_gen.h"
#include "vir/codegen/gc_vsc_vir_ep_gen.h"
#include "vir/codegen/gc_vsc_vir_ep_back_patch.h"
#include "vir/linker/gc_vsc_vir_linker.h"
#if defined(LINUX) && !defined(ANDROID)
#include <unistd.h>
#endif


gceSTATUS vscCreatePrivateData(VSC_CORE_SYS_CONTEXT* pCoreSysCtx, VSC_PRIV_DATA_HANDLE* phOutPrivData, gctBOOL bForOCL)
{
    VSC_PRIV_DATA*        pPrivData;
    VSC_PRIV_DATA_HANDLE  hPrivData;

    gcmASSERT(pCoreSysCtx);

    gcoOS_Allocate(gcvNULL, sizeof(VSC_PRIV_DATA), &hPrivData);

    pPrivData = (VSC_PRIV_DATA*)hPrivData;
    gcoOS_ZeroMemory(pPrivData, sizeof(VSC_PRIV_DATA));

    *phOutPrivData = hPrivData;

    return gcvSTATUS_OK;
}

gceSTATUS vscDestroyPrivateData(VSC_CORE_SYS_CONTEXT* pCoreSysCtx, VSC_PRIV_DATA_HANDLE hPrivData)
{
    VSC_PRIV_DATA* pPrivData = (VSC_PRIV_DATA*)hPrivData;

    gcmASSERT(pCoreSysCtx);

    if (pPrivData)
    {
        /* Destroy graphics intrinsic lib. */
        if (pPrivData->IntrinsicLib.GLLib.pGraphicsIntrinsicLib)
        {
            VIR_DestroyIntrinsicLib(pPrivData->IntrinsicLib.GLLib.pGraphicsIntrinsicLib);
            pPrivData->IntrinsicLib.GLLib.pGraphicsIntrinsicLib = gcvNULL;
        }

        /* Destroy compute intrinsic lib. */
        if (pPrivData->IntrinsicLib.GLLib.pComputeIntrinsicLib)
        {
            VIR_DestroyIntrinsicLib(pPrivData->IntrinsicLib.GLLib.pComputeIntrinsicLib);
            pPrivData->IntrinsicLib.GLLib.pComputeIntrinsicLib = gcvNULL;
        }

        /* Destroy CL intrinsic lib. */
        if (pPrivData->IntrinsicLib.pCLIntrinsicLib)
        {
            VIR_DestroyIntrinsicLib(pPrivData->IntrinsicLib.pCLIntrinsicLib);
            pPrivData->IntrinsicLib.pCLIntrinsicLib = gcvNULL;
        }

        vscPMP_Finalize(&pPrivData->pmp);

        gcoOS_Free(gcvNULL, pPrivData);
    }


    return gcvSTATUS_OK;
}

gceSTATUS vscInitializeSEP(SHADER_EXECUTABLE_PROFILE* pSEP)
{
    gctUINT i;

    gcoOS_ZeroMemory(pSEP, sizeof(SHADER_EXECUTABLE_PROFILE));

    for (i = 0; i < SHADER_IO_USAGE_TOTAL_COUNT; i ++)
    {
        pSEP->inputMapping.ioVtxPxl.usage2IO[i].mainFirstValidIoChannel =
        pSEP->outputMapping.ioVtxPxl.usage2IO[i].mainFirstValidIoChannel =
        pSEP->inputMapping.ioVtxPxl.usage2IO[i].mainIoIndex =
        pSEP->outputMapping.ioVtxPxl.usage2IO[i].mainIoIndex = NOT_ASSIGNED;

        pSEP->inputMapping.ioPrim.usage2IO[i].mainFirstValidIoChannel =
        pSEP->outputMapping.ioPrim.usage2IO[i].mainFirstValidIoChannel =
        pSEP->inputMapping.ioPrim.usage2IO[i].mainIoIndex =
        pSEP->outputMapping.ioPrim.usage2IO[i].mainIoIndex = NOT_ASSIGNED;
    }

    pSEP->inputMapping.ioVtxPxl.ioCategory = SHADER_IO_CATEGORY_PER_VTX_PXL;
    pSEP->inputMapping.ioPrim.ioCategory = SHADER_IO_CATEGORY_PER_PRIM;

    pSEP->outputMapping.ioVtxPxl.ioCategory = SHADER_IO_CATEGORY_PER_VTX_PXL;
    pSEP->outputMapping.ioPrim.ioCategory = SHADER_IO_CATEGORY_PER_PRIM;

    for (i = 0; i < SHADER_CONSTANT_USAGE_TOTAL_COUNT; i ++)
    {
        pSEP->constantMapping.usage2ArrayIndex[i] = NOT_ASSIGNED;
    }
    pSEP->constantMapping.maxHwConstRegIndex = NOT_ASSIGNED;

    pSEP->samplerMapping.countOfSamplers = 0;
    pSEP->samplerMapping.hwSamplerRegCount = 0;
    pSEP->samplerMapping.maxHwSamplerRegIndex = NOT_ASSIGNED;

    pSEP->defaultUboMapping.pDefaultUboMemberEntries = gcvNULL;
    pSEP->defaultUboMapping.countOfEntries = 0;
    pSEP->defaultUboMapping.sizeInByte = 0;
    pSEP->defaultUboMapping.baseAddressIndexInPrivConstTable = 0xFFFFFFFF;

    return gcvSTATUS_OK;
}

gceSTATUS vscFinalizeSEP(SHADER_EXECUTABLE_PROFILE* pSEP)
{
    gctUINT i;

    if (pSEP->pMachineCode)
    {
        gcoOS_Free(gcvNULL, pSEP->pMachineCode);
        pSEP->pMachineCode = gcvNULL;
    }

    if (pSEP->inputMapping.ioVtxPxl.pIoRegMapping)
    {
        gcoOS_Free(gcvNULL, pSEP->inputMapping.ioVtxPxl.pIoRegMapping);
        pSEP->inputMapping.ioVtxPxl.pIoRegMapping = gcvNULL;
    }

    if (pSEP->inputMapping.ioPrim.pIoRegMapping)
    {
        gcoOS_Free(gcvNULL, pSEP->inputMapping.ioPrim.pIoRegMapping);
        pSEP->inputMapping.ioPrim.pIoRegMapping = gcvNULL;
    }

    if (pSEP->outputMapping.ioVtxPxl.pIoRegMapping)
    {
        gcoOS_Free(gcvNULL, pSEP->outputMapping.ioVtxPxl.pIoRegMapping);
        pSEP->outputMapping.ioVtxPxl.pIoRegMapping = gcvNULL;
    }

    if (pSEP->outputMapping.ioPrim.pIoRegMapping)
    {
        gcoOS_Free(gcvNULL, pSEP->outputMapping.ioPrim.pIoRegMapping);
        pSEP->outputMapping.ioPrim.pIoRegMapping = gcvNULL;
    }

    if (pSEP->constantMapping.pConstantArrayMapping)
    {
        if (pSEP->constantMapping.pConstantArrayMapping->pSubConstantArrays)
        {
            gcoOS_Free(gcvNULL, pSEP->constantMapping.pConstantArrayMapping->pSubConstantArrays);
            pSEP->constantMapping.pConstantArrayMapping->pSubConstantArrays = gcvNULL;
        }

        gcoOS_Free(gcvNULL, pSEP->constantMapping.pConstantArrayMapping);
        pSEP->constantMapping.pConstantArrayMapping = gcvNULL;
    }

    if (pSEP->constantMapping.pCompileTimeConstant)
    {
        gcoOS_Free(gcvNULL, pSEP->constantMapping.pCompileTimeConstant);
        pSEP->constantMapping.pCompileTimeConstant = gcvNULL;
    }

    if (pSEP->samplerMapping.pSampler)
    {
        gcoOS_Free(gcvNULL, pSEP->samplerMapping.pSampler);
        pSEP->samplerMapping.pSampler = gcvNULL;
    }

    if (pSEP->resourceMapping.pResource)
    {
        gcoOS_Free(gcvNULL, pSEP->resourceMapping.pResource);
        pSEP->resourceMapping.pResource = gcvNULL;
    }

    if (pSEP->uavMapping.pUAV)
    {
        gcoOS_Free(gcvNULL, pSEP->uavMapping.pUAV);
        pSEP->uavMapping.pUAV = gcvNULL;
    }

    for (i = 0; i < pSEP->staticPrivMapping.privConstantMapping.countOfEntries; i ++)
    {
        if (pSEP->staticPrivMapping.privConstantMapping.pPrivmConstantEntries[i].commonPrivm.pPrivateData)
        {
            if(!pSEP->staticPrivMapping.privConstantMapping.pPrivmConstantEntries[i].commonPrivm.notAllocated) {
                gcoOS_Free(gcvNULL, pSEP->staticPrivMapping.privConstantMapping.pPrivmConstantEntries[i].commonPrivm.pPrivateData);
            }
            pSEP->staticPrivMapping.privConstantMapping.pPrivmConstantEntries[i].commonPrivm.notAllocated = gcvFALSE;
            pSEP->staticPrivMapping.privConstantMapping.pPrivmConstantEntries[i].commonPrivm.pPrivateData = gcvNULL;
        }
    }

    if (pSEP->staticPrivMapping.privConstantMapping.pPrivmConstantEntries)
    {
        gcoOS_Free(gcvNULL, pSEP->staticPrivMapping.privConstantMapping.pPrivmConstantEntries);
        pSEP->staticPrivMapping.privConstantMapping.pPrivmConstantEntries = gcvNULL;
        pSEP->staticPrivMapping.privConstantMapping.countOfEntries = 0;
    }

    for (i = 0; i < pSEP->staticPrivMapping.privUavMapping.countOfEntries; i ++)
    {
        if (pSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].commonPrivm.pPrivateData)
        {
            gcoOS_Free(gcvNULL, pSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].commonPrivm.pPrivateData);
            pSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].commonPrivm.pPrivateData = gcvNULL;
        }

        if (pSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].memData.ppCnstSubArray)
        {
            gcoOS_Free(gcvNULL, pSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].memData.ppCnstSubArray);
            pSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].memData.ppCnstSubArray = gcvNULL;
        }

        if (pSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].memData.ppCTC)
        {
            gcoOS_Free(gcvNULL, pSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].memData.ppCTC);
            pSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].memData.ppCTC = gcvNULL;
        }
    }

    if (pSEP->staticPrivMapping.privUavMapping.pPrivUavEntries)
    {
        gcoOS_Free(gcvNULL, pSEP->staticPrivMapping.privUavMapping.pPrivUavEntries);
        pSEP->staticPrivMapping.privUavMapping.pPrivUavEntries = gcvNULL;
        pSEP->staticPrivMapping.privUavMapping.countOfEntries = 0;
    }

    for (i = 0; i < pSEP->dynamicPrivMapping.privSamplerMapping.countOfEntries; i ++)
    {
        if (pSEP->dynamicPrivMapping.privSamplerMapping.pPrivSamplerEntries[i].commonPrivm.pPrivateData)
        {
            if(!pSEP->dynamicPrivMapping.privSamplerMapping.pPrivSamplerEntries[i].commonPrivm.notAllocated) {
                gcoOS_Free(gcvNULL, pSEP->dynamicPrivMapping.privSamplerMapping.pPrivSamplerEntries[i].commonPrivm.pPrivateData);
            }
            pSEP->dynamicPrivMapping.privSamplerMapping.pPrivSamplerEntries[i].commonPrivm.notAllocated = gcvFALSE;
            pSEP->dynamicPrivMapping.privSamplerMapping.pPrivSamplerEntries[i].commonPrivm.pPrivateData = gcvNULL;
        }
    }

    if (pSEP->dynamicPrivMapping.privSamplerMapping.pPrivSamplerEntries)
    {
        gcoOS_Free(gcvNULL, pSEP->dynamicPrivMapping.privSamplerMapping.pPrivSamplerEntries);
        pSEP->dynamicPrivMapping.privSamplerMapping.pPrivSamplerEntries = gcvNULL;
        pSEP->dynamicPrivMapping.privSamplerMapping.countOfEntries = 0;
    }

    for (i = 0; i < pSEP->dynamicPrivMapping.privOutputMapping.countOfEntries; i ++)
    {
        if (pSEP->dynamicPrivMapping.privOutputMapping.pPrivOutputEntries[i].commonPrivm.pPrivateData)
        {
            if(!pSEP->dynamicPrivMapping.privOutputMapping.pPrivOutputEntries[i].commonPrivm.notAllocated) {
                gcoOS_Free(gcvNULL, pSEP->dynamicPrivMapping.privOutputMapping.pPrivOutputEntries[i].commonPrivm.pPrivateData);
            }
            pSEP->dynamicPrivMapping.privOutputMapping.pPrivOutputEntries[i].commonPrivm.notAllocated = gcvFALSE;
            pSEP->dynamicPrivMapping.privOutputMapping.pPrivOutputEntries[i].commonPrivm.pPrivateData = gcvNULL;
        }
    }

    if (pSEP->dynamicPrivMapping.privOutputMapping.pPrivOutputEntries)
    {
        gcoOS_Free(gcvNULL, pSEP->dynamicPrivMapping.privOutputMapping.pPrivOutputEntries);
        pSEP->dynamicPrivMapping.privOutputMapping.pPrivOutputEntries = gcvNULL;
        pSEP->dynamicPrivMapping.privOutputMapping.countOfEntries = 0;
    }

    if (pSEP->defaultUboMapping.pDefaultUboMemberEntries)
    {
        gcoOS_Free(gcvNULL, pSEP->defaultUboMapping.pDefaultUboMemberEntries);
        pSEP->defaultUboMapping.pDefaultUboMemberEntries = gcvNULL;
        pSEP->defaultUboMapping.countOfEntries = 0;
        pSEP->defaultUboMapping.sizeInByte = 0;
        pSEP->defaultUboMapping.baseAddressIndexInPrivConstTable = 0xFFFFFFFF;
    }

    vscInitializeSEP(pSEP);

    return gcvSTATUS_OK;
}

gctBOOL vscIsValidSEP(SHADER_EXECUTABLE_PROFILE* pSEP)
{
    return (pSEP->pMachineCode != gcvNULL);
}

gceSTATUS vscInitializeIoRegMapping(SHADER_IO_REG_MAPPING* pIoRegMapping)
{
    gctUINT i;

    gcoOS_ZeroMemory(pIoRegMapping, sizeof(SHADER_IO_REG_MAPPING));

    pIoRegMapping->ioIndex =
    pIoRegMapping->firstValidIoChannel = NOT_ASSIGNED;

    for (i = 0; i < CHANNEL_NUM; i ++)
    {
        pIoRegMapping->ioChannelMapping[i].ioUsage = SHADER_IO_USAGE_GENERAL;
        pIoRegMapping->ioChannelMapping[i].hwLoc.cmnHwLoc.u.hwRegNo = NOT_ASSIGNED;
        pIoRegMapping->ioChannelMapping[i].hwLoc.t1HwLoc.hwRegNo = NOT_ASSIGNED;
    }

    return gcvSTATUS_OK;
}

gceSTATUS vscFinalizeIoRegMapping(SHADER_IO_REG_MAPPING* pIoRegMapping)
{
    return vscInitializeIoRegMapping(pIoRegMapping);
}

gceSTATUS vscInitializeCnstHwLocMapping(SHADER_CONSTANT_HW_LOCATION_MAPPING* pCnstHwLocMapping)
{
    gcoOS_ZeroMemory(pCnstHwLocMapping, sizeof(SHADER_CONSTANT_HW_LOCATION_MAPPING));

    pCnstHwLocMapping->hwAccessMode = SHADER_HW_ACCESS_MODE_REGISTER;
    pCnstHwLocMapping->hwLoc.constReg.hwRegNo = NOT_ASSIGNED;
    pCnstHwLocMapping->hwLoc.constReg.hwRegRange = 0;
    pCnstHwLocMapping->firstValidHwChannel = NOT_ASSIGNED;

    return gcvSTATUS_OK;
}

gceSTATUS vscFinalizeCnstHwLocMapping(SHADER_CONSTANT_HW_LOCATION_MAPPING* pCnstHwLocMapping)
{
    return vscInitializeCnstHwLocMapping(pCnstHwLocMapping);
}

gceSTATUS vscInitializeCTC(SHADER_COMPILE_TIME_CONSTANT* pCompileTimeConstant)
{
    gcoOS_ZeroMemory(pCompileTimeConstant, sizeof(SHADER_COMPILE_TIME_CONSTANT));

    return vscInitializeCnstHwLocMapping(&pCompileTimeConstant->hwConstantLocation);
}

gceSTATUS vscFinalizeCTC(SHADER_COMPILE_TIME_CONSTANT* pCompileTimeConstant)
{
    return vscInitializeCTC(pCompileTimeConstant);
}

gceSTATUS vscInitializeCnstArrayMapping(SHADER_CONSTANT_ARRAY_MAPPING* pCnstArrayMapping)
{
    gcoOS_ZeroMemory(pCnstArrayMapping, sizeof(SHADER_CONSTANT_ARRAY_MAPPING));

    pCnstArrayMapping->constantArrayIndex = NOT_ASSIGNED;

    return gcvSTATUS_OK;
}

gceSTATUS vscFinalizeCnstArrayMapping(SHADER_CONSTANT_ARRAY_MAPPING* pCnstArrayMapping)
{
    return vscInitializeCnstArrayMapping(pCnstArrayMapping);
}

gceSTATUS vscInitializeCnstSubArrayMapping(SHADER_CONSTANT_SUB_ARRAY_MAPPING* pCnstSubArrayMapping)
{
    gcoOS_ZeroMemory(pCnstSubArrayMapping, sizeof(SHADER_CONSTANT_SUB_ARRAY_MAPPING));

    pCnstSubArrayMapping->firstMSCSharpRegNo = NOT_ASSIGNED;

    return vscInitializeCnstHwLocMapping(&pCnstSubArrayMapping->hwFirstConstantLocation);
}

gceSTATUS vscFinalizeCnstSubArrayMapping(SHADER_CONSTANT_SUB_ARRAY_MAPPING* pCnstSubArrayMapping)
{
    return vscInitializeCnstSubArrayMapping(pCnstSubArrayMapping);
}

gceSTATUS vscInitializeSamplerSlotMapping(SHADER_SAMPLER_SLOT_MAPPING* pSamplerSlotMapping)
{
    gcoOS_ZeroMemory(pSamplerSlotMapping, sizeof(SHADER_SAMPLER_SLOT_MAPPING));

    pSamplerSlotMapping->hwSamplerSlot = NOT_ASSIGNED;
    pSamplerSlotMapping->samplerSlotIndex = NOT_ASSIGNED;

    return gcvSTATUS_OK;
}

gceSTATUS vscFinalizeSamplerSlotMapping(SHADER_SAMPLER_SLOT_MAPPING* pSamplerSlotMapping)
{
    return vscInitializeSamplerSlotMapping(pSamplerSlotMapping);
}

gceSTATUS vscInitializeUavSlotMapping(SHADER_UAV_SLOT_MAPPING* pUavSlotMapping)
{
    gcoOS_ZeroMemory(pUavSlotMapping, sizeof(SHADER_UAV_SLOT_MAPPING));

    pUavSlotMapping->uavSlotIndex = NOT_ASSIGNED;
    pUavSlotMapping->hwLoc.hwUavSlot = NOT_ASSIGNED;

    return gcvSTATUS_OK;
}

gceSTATUS vscFinalizeUavSlotMapping(SHADER_UAV_SLOT_MAPPING* pUavSlotMapping)
{
    return vscInitializeUavSlotMapping(pUavSlotMapping);
}

gceSTATUS vscCreateShader(SHADER_HANDLE* pShaderHandle, gctUINT shStage)
{
    gceSTATUS                     status = gcvSTATUS_OK;
    VSC_ErrCode                   errCode = VSC_ERR_NONE;
    VIR_Shader*                   pVirShader = gcvNULL;
    VIR_ShaderKind                virShaderKind = VIR_SHADER_UNKNOWN;

    *pShaderHandle = gcvNULL;

    switch (shStage)
    {
    case VSC_SHADER_STAGE_VS:
        virShaderKind = VIR_SHADER_VERTEX;
        break;
    case VSC_SHADER_STAGE_HS:
        virShaderKind = VIR_SHADER_TESSELLATION_CONTROL;
        break;
    case VSC_SHADER_STAGE_DS:
        virShaderKind = VIR_SHADER_TESSELLATION_EVALUATION;
        break;
    case VSC_SHADER_STAGE_GS:
        virShaderKind = VIR_SHADER_GEOMETRY;
        break;
    case VSC_SHADER_STAGE_PS:
        virShaderKind = VIR_SHADER_FRAGMENT;
        break;
    case VSC_SHADER_STAGE_CS:
        virShaderKind = VIR_SHADER_COMPUTE;
        break;
    case VSC_SHADER_STAGE_UNKNOWN:
        virShaderKind = VIR_SHADER_LIBRARY;
        break;
    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    /* Allocate memory to hold vir shader */
    gcmONERROR(gcoOS_Allocate(gcvNULL, sizeof(VIR_Shader), (gctPOINTER*)&pVirShader));

    /* Now construct vir shader in allocated mem */
    errCode = VIR_Shader_Construct(gcvNULL, virShaderKind, pVirShader);
    ON_ERROR(errCode, "Shader Construct");

    /* Yes, we've created our vir shader, now just return its handle */
    *pShaderHandle = pVirShader;

OnError:
    return (status == gcvSTATUS_OK) ? vscERR_CastErrCode2GcStatus(errCode) : status;
}

gceSTATUS vscDestroyShader(SHADER_HANDLE hShader)
{
    gceSTATUS                     status = gcvSTATUS_OK;
    VSC_ErrCode                   errCode = VSC_ERR_NONE;
    VIR_Shader*                   pVirShader = (VIR_Shader*)hShader;

    errCode = VIR_Shader_Destroy(pVirShader);
    ON_ERROR(errCode, "Shader Destroy");

    gcmONERROR(gcoOS_Free(gcvNULL, pVirShader));

OnError:
    return (status == gcvSTATUS_OK) ? vscERR_CastErrCode2GcStatus(errCode) : status;
}

gctPOINTER vscGetDebugInfo(IN SHADER_HANDLE    Shader)
{
    if (Shader != NULL)
        return (gctPOINTER)(((VIR_Shader*)Shader)->debugInfo);

    return NULL;
}

gctPOINTER vscDupDebugInfo(IN SHADER_HANDLE    Shader)
{
    VIR_Shader* pVirShader = (VIR_Shader*)Shader;
    gctPOINTER ret = NULL;
    if (Shader == NULL)
        return NULL;

    vscDICopyDebugInfo(pVirShader->debugInfo, &ret);

    return ret;
}

gceSTATUS  vscDestroyDebugInfo(IN gctPOINTER DebugInfo)
{
    if (DebugInfo)
        vscDIDestroyContext((VSC_DIContext *)DebugInfo);
    return gcvSTATUS_OK;
}

gceSTATUS vscCopyShader(SHADER_HANDLE * hToShader, SHADER_HANDLE hFromShader)
{
    gceSTATUS                     status = gcvSTATUS_OK;
    VSC_ErrCode                   errCode = VSC_ERR_NONE;
    VIR_Shader*                   pFromVirShader = (VIR_Shader*)hFromShader;
    VIR_Shader*                   pToVirShader;

    /* Allocate memory to hold vir shader */
    gcmONERROR(gcoOS_Allocate(gcvNULL, sizeof(VIR_Shader), (gctPOINTER*)&pToVirShader));

    *hToShader = (SHADER_HANDLE)pToVirShader;

    errCode = VIR_Shader_Copy(pToVirShader, pFromVirShader);
    ON_ERROR(errCode, "Shader Copy");

OnError:
    return (status == gcvSTATUS_OK) ? vscERR_CastErrCode2GcStatus(errCode) : status;
}

gceSTATUS vscPrintShader(SHADER_HANDLE hShader,gctFILE hFile,
                         gctCONST_STRING strHeaderMsg,
                         gctBOOL bPrintHeaderFooter)
{
    VSC_ErrCode                   errCode = VSC_ERR_NONE;

    errCode = VIR_Shader_Dump(hFile, strHeaderMsg, (VIR_Shader*)hShader, bPrintHeaderFooter);

    return vscERR_CastErrCode2GcStatus(errCode);
}

/* Two ways to save shader binary:
 * 1) compiler allocates memory and return the memory in *pBinary and size in pSizeInByte
 *    if (* pBinary) is NULL when calling the function
 * 2) user query the shader binary size with vscQueryShaderBinarySize() and allocate
 *    memory in (*pBinary), size in pSizeInByte
 */
gceSTATUS vscSaveShaderToBinary(SHADER_HANDLE hShader,
                                void ** pBinary,
                                gctUINT* pSizeInByte)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;

    if (*pBinary == gcvNULL)
    {
        VIR_Shader_IOBuffer buf;

        VIR_Shader_IOBuffer_Initialize(&buf);

        errCode = VIR_Shader_Save((VIR_Shader *)hShader, &buf);
        if (errCode == VSC_ERR_NONE)
        {
            *pBinary = (void*)(buf.ioBuffer->buffer);
            *pSizeInByte = buf.ioBuffer->curPos;
        }

        VIR_Shader_IOBuffer_Finalize(&buf);
    }
    else
    {
        errCode = VIR_Shader_Save2Buffer((VIR_Shader *)hShader,
                                         (gctCHAR *)*pBinary,
                                         *pSizeInByte);
    }
    return vscERR_CastErrCode2GcStatus(errCode);
}

gceSTATUS vscQueryShaderBinarySize(SHADER_HANDLE hShader, gctUINT* pSizeInByte)
{
    VSC_ErrCode         errCode;
    errCode = VIR_Shader_QueryBinarySize((VIR_Shader *)hShader, pSizeInByte);
    return vscERR_CastErrCode2GcStatus(errCode);
}

gceSTATUS vscLoadShaderFromBinary(void*          pBinary,
                                  gctUINT        sizeInByte,
                                  SHADER_HANDLE* pShaderHandle,
                                  gctBOOL        bFreeBinary)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    gceSTATUS           status = gcvSTATUS_OK;
    VSC_IO_BUFFER       ioBuf = {0, 0, 0};
    VIR_Shader_IOBuffer buf;
    VIR_Shader *        readShader = gcvNULL;

    VIR_Shader_IOBuffer_Initialize(&buf);

    ioBuf.allocatedBytes = sizeInByte;
    ioBuf.buffer = (gctCHAR *)pBinary;
    ioBuf.curPos = 0;

    buf.ioBuffer    = &ioBuf;
    buf.shader      = gcvNULL;

    gcmONERROR(gcoOS_Allocate(gcvNULL,
                                sizeof(VIR_Shader),
                                (gctPOINTER*)&readShader));

    errCode = VIR_Shader_Construct(gcvNULL, VIR_SHADER_UNKNOWN, readShader);
    ON_ERROR(errCode, "Shader Load");

    buf.shader = readShader;

    errCode = VIR_Shader_Read(readShader, &buf, 0);
    /* if reading library shader fails due to version mismatch, we will regenerate the file, instead of reporting errors */
    if (errCode == VSC_ERR_VERSION_MISMATCH)
    {
        VIR_Shader_IOBuffer_Finalize(&buf);

        if (bFreeBinary)
        {
            VIR_IO_Finalize(&buf, bFreeBinary);  /* reclaim allocated buffer */
        }
        if (readShader)
        {
            gcoOS_Free(gcvNULL, readShader);
        }
        return gcvSTATUS_OK;
    }
    ON_ERROR(errCode, "Shader Load");
    if (bFreeBinary)
    {
        VIR_IO_Finalize(&buf, bFreeBinary);  /* reclaim allocated buffer */
    }
    VIR_Shader_IOBuffer_Finalize(&buf);

    *pShaderHandle = readShader;
    return gcvSTATUS_OK;

OnError:
    VIR_Shader_IOBuffer_Finalize(&buf);

    if (bFreeBinary)
    {
        VIR_IO_Finalize(&buf, bFreeBinary);  /* reclaim allocated buffer */
    }
    if (readShader)
    {
        gcoOS_Free(gcvNULL, readShader);
    }
    return (status == gcvSTATUS_OK) ? vscERR_CastErrCode2GcStatus(errCode) : status;
}

gceSTATUS vscExtractSubShader(SHADER_HANDLE   hMainShader,
                              gctCONST_STRING pSubShaderEntryName,
                              SHADER_HANDLE   hSubShader)
{
    return gcvSTATUS_OK;
}

gceSTATUS vscLinkLibShaderToShader(SHADER_HANDLE              hMainShader,
                                   VSC_SHADER_LIB_LINK_TABLE* pShLibLinkTable)
{
    return gcvSTATUS_OK;
}

static VSC_ErrCode _PreprocessShader(VSC_SHADER_PASS_MANAGER* pShPassMnger)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;

    /* Set PM level firstly */
    vscPM_SetCurPassLevel(&pShPassMnger->basePM, VSC_PASS_LEVEL_PRE);

    CALL_SH_PASS(vscVIR_VX_ReplaceDest, 0, gcvNULL);
    CALL_SH_PASS(vscVIR_RemoveNop, 0, gcvNULL);

OnError:
    return errCode;
}

static VSC_ErrCode _PostprocessShader(VSC_SHADER_PASS_MANAGER*   pShPassMnger,
                                      SHADER_EXECUTABLE_PROFILE* pOutSEP)
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;

    /* Set PM level firstly */
    vscPM_SetCurPassLevel(&pShPassMnger->basePM, VSC_PASS_LEVEL_PST);

    CALL_SH_PASS(vscVIR_PerformSEPBackPatch, 0, pOutSEP);

OnError:
    return VSC_ERR_NONE;
}

static VSC_ErrCode _CompileShaderAtHighLevel(VSC_SHADER_PASS_MANAGER* pShPassMnger)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Shader*         pShader = (VIR_Shader*)pShPassMnger->pCompilerParam->hShader;

    gcmASSERT(VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Pre_High);

    /* Set PM level firstly */
    vscPM_SetCurPassLevel(&pShPassMnger->basePM, VSC_PASS_LEVEL_HL);

    /* Call (schedule) passes of HL by ourselves */
    CALL_SH_PASS(VIR_Lower_HighLevel_To_HighLevel_Expand, 0, gcvNULL);

    /* We are at end of HL of VIR, so set this correct level */
    VIR_Shader_SetLevel(pShader, VIR_SHLEVEL_Post_High);

OnError:
    return errCode;
}

static VSC_ErrCode _CompileShaderAtMedLevel(VSC_SHADER_PASS_MANAGER* pShPassMnger)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Shader*         pShader = (VIR_Shader*)pShPassMnger->pCompilerParam->hShader;
    VIR_CHECK_VAR_USAGE checkVarUsage = { gcvTRUE, gcvTRUE, gcvFALSE, gcvFALSE, gcvFALSE};
    VSC_IL_PASS_DATA    ilPassData = { 0, gcvTRUE };
    VSC_CPP_PASS_DATA   cppPassData = { VSC_CPP_NONE, gcvTRUE };

    gcmASSERT(VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Pre_Medium ||
              VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Post_High);

    /* Lower HL to ML firstly */
    CALL_SH_PASS(VIR_Lower_HighLevel_To_MiddleLevel, 0, gcvNULL);

    /* It must be called after lower */
    vscPM_SetCurPassLevel(&pShPassMnger->basePM, VSC_PASS_LEVEL_ML);

    /* Call (schedule) passes of ML by ourselves */
    CALL_SH_PASS(vscVIR_CheckVariableUsage, 0, &checkVarUsage);

    /* Fix the texld offset */
    CALL_SH_PASS(vscVIR_FixTexldOffset, 0, gcvNULL);

    /* Convert atom* instruction to extcall atom* if
     * pHwCfg->hwFeatureFlags.supportUSC && !pHwCfg->hwFeatureFlags.hasUSCAtomicFix2
     * call before VIR_LinkInternalLibFunc and use same pass control as VIR_LinkInternalLibFunc
     */
    CALL_SH_PASS(vscVIR_GenExternalAtomicCall, 0, gcvNULL);

    /* Don't need to inline a patch lib. */
    if (!VIR_Shader_IsPatchLib(pShader))
    {
        gctBOOL linkImageIntrinsic = gcvFALSE;  /* image intrinsic needs combined sampler to work */
        ilPassData.bCheckAlwaysInlineOnly = gcvFALSE;

        /* Mark the must-inline functions. */
        if (ilPassData.bCheckAlwaysInlineOnly)
        {
            CALL_SH_PASS(vscVIR_CheckMustInlineFuncForML, 0, gcvNULL);
        }

        /* Link intrinsic functions. */
        CALL_SH_PASS(VIR_LinkInternalLibFunc, 0, &linkImageIntrinsic);
        /* Inline functions after link in lib functions and before combine sampler and image. */
        CALL_SH_PASS(VSC_IL_PerformOnShader, 0, &ilPassData);

        /* Initialize variables here to make CPP work. */
        CALL_SH_PASS(vscVIR_InitializeVariables, 0, gcvNULL);
        CALL_SH_PASS(VSC_CPP_PerformOnShader, 0, &cppPassData);

        /* simplification pass on ML */
        CALL_SH_PASS(VSC_SIMP_Simplification_PerformOnShader, 0, gcvNULL);

        /* check common intrinsic expression in function level and delete redundant ones to reduce code size */
        CALL_SH_PASS(VSC_CIE_PerformOnShader, 0, gcvNULL);
    }

    /*
    ** Generate combined sampler for separated samper and separated texture.
    ** We need to do this before patch lib linking, which is in post ML compilation.
    */
    CALL_SH_PASS(vscVIR_GenCombinedSampler, 0, gcvNULL);

    /* If there is any image format/sampled type mismatch, we need to process here before linking. */
    if (VIR_Shader_HasImageFormatMismatch(pShader))
    {
        VSC_IL_PASS_DATA    ilPassData = { 2, gcvTRUE };

        CALL_SH_PASS(vscVIR_CheckMustInlineFuncForML, 0, gcvNULL);
        CALL_SH_PASS(VSC_IL_PerformOnShader, 0, &ilPassData);
        CALL_SH_PASS(vscVIR_ProcessImageFormatMismatch, 0, gcvNULL);
    }

    /* We are at end of ML of VIR, so set this correct level */
    VIR_Shader_SetLevel(pShader, VIR_SHLEVEL_Post_Medium);

OnError:
    return errCode;
}

static VSC_ErrCode _CompileShaderAtLowLevel(VSC_SHADER_PASS_MANAGER* pShPassMnger)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Shader*         pShader = (VIR_Shader*)pShPassMnger->pCompilerParam->hShader;
    gctBOOL             bRAEnabled = VSC_OPTN_RAOptions_GetSwitchOn(VSC_OPTN_Options_GetRAOptions(pShPassMnger->basePM.pOptions, 0));
    VSC_PRELL_PASS_DATA preLLPassData = { gcvFALSE };
    VSC_CPP_PASS_DATA   cppPassData = { VSC_CPP_NONE, gcvTRUE };
    VSC_IL_PASS_DATA    ilPassData = { 3, gcvFALSE };
    VSC_CPF_PASS_DATA   cpfPassData = { gcvFALSE };

    gcmASSERT(VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Pre_Low ||
              VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Post_Medium);

    /* Lower ML to LL pre firstly */
    CALL_SH_PASS(VIR_Lower_MiddleLevel_To_LowLevel_Pre, 0, &bRAEnabled);

    /* It must be called after lower */
    vscPM_SetCurPassLevel(&pShPassMnger->basePM, VSC_PASS_LEVEL_LL);

    /* Record the instruction status. */
    CALL_SH_PASS(vscVIR_RecordInstructionStatus, 0, gcvNULL);

    /* Call (schedule) passes of LL by ourselves */
    CALL_SH_PASS(vscVIR_PreprocessLLShader, 0, &preLLPassData);

    /* If some functions are marked as FORCE_INLINED after pre-LL, we need to enable IL. */
    if (preLLPassData.bHasFuncNeedToForceInline)
    {
        VSC_OPTN_ILOptions* pInlineOptions = VSC_OPTN_Options_GetInlinerOptions(pShPassMnger->basePM.pOptions, 0);

        if (!VSC_OPTN_ILOptions_GetSwitchOn(pInlineOptions))
        {
            VSC_OPTN_ILOptions_SetSwitchOn(pInlineOptions, gcvTRUE);
            ilPassData.bCheckAlwaysInlineOnly = gcvTRUE;
        }
    }
    CALL_SH_PASS(VSC_IL_PerformOnShader, 0, &ilPassData);

    CALL_SH_PASS(VSC_SIMP_Simplification_PerformOnShader, 0, gcvNULL);
    CALL_SH_PASS(VSC_SCPP_PerformOnShader, 0, gcvNULL);
    CALL_SH_PASS(VIR_LoopOpts_PerformOnShader, 0, gcvNULL);
    CALL_SH_PASS(VIR_CFO_PerformOnShader, 0, gcvNULL);
    CALL_SH_PASS(vscVIR_ConvertVirtualInstructions, 0, gcvNULL);
    CALL_SH_PASS(VSC_CPF_PerformOnShader, 0, gcvNULL);
    CALL_SH_PASS(vscVIR_InitializeVariables, 0, gcvNULL);

    cppPassData.cppFlag |= VSC_CPP_COPY_FROM_OUTPUT_PARAM;
    CALL_SH_PASS(VSC_CPP_PerformOnShader, 0, &cppPassData);

    CALL_SH_PASS(VSC_PARAM_Optimization_PerformOnShader, 0, gcvNULL);
    CALL_SH_PASS(VSC_SCL_Scalarization_PerformOnShader, 0, gcvNULL);
    CALL_SH_PASS(VSC_PH_Peephole_PerformOnShader, 0, gcvNULL);
    CALL_SH_PASS(VSC_LCSE_PerformOnShader, 0, gcvNULL);
    CALL_SH_PASS(VSC_DCE_Perform, 0, gcvNULL);
    CALL_SH_PASS(VIR_CFO_PerformOnShader, 1, gcvNULL);
    CALL_SH_PASS(vscVIR_AdjustPrecision, 0, gcvNULL);

    /* Call SIMP before the local vectorization because some code patterns are messed up after the vectorization. */
    CALL_SH_PASS(VSC_SIMP_Simplification_PerformOnShader, 0, gcvNULL);

    CALL_SH_PASS(vscVIR_DoLocalVectorization, 0, gcvNULL);
    CALL_SH_PASS(vscVIR_AddOutOfBoundCheckSupport, 0, gcvNULL);

    CALL_SH_PASS(vscVIR_ClampPointSize, 0, gcvNULL);

    /* Call CPF one more time because after CFO we can optimize more instructions. */
    CALL_SH_PASS(VSC_CPF_PerformOnShader, 0, &cpfPassData);

    /* Call SIMP one more time because we can optimize more instructions after CPF. */
    if (cpfPassData.bChanged)
    {
        CALL_SH_PASS(VSC_SIMP_Simplification_PerformOnShader, 0, gcvNULL);
    }

    /* Lower ML to LL post */
    CALL_SH_PASS(VIR_Lower_MiddleLevel_To_LowLevel_Post, 0, &bRAEnabled);

    /* We are at the end of LL of VIR, so set this correct level */
    VIR_Shader_SetLevel(pShader, VIR_SHLEVEL_Post_Low);

OnError:
    return errCode;
}

static VSC_ErrCode _CompileShaderAtMCLevel(VSC_SHADER_PASS_MANAGER* pShPassMnger)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Shader*         pShader = (VIR_Shader*)pShPassMnger->pCompilerParam->hShader;
    VSC_CPP_PASS_DATA   cppPassData = { VSC_CPP_NONE, gcvFALSE };
    gctBOOL             bRAEnabled = VSC_OPTN_RAOptions_GetSwitchOn(VSC_OPTN_Options_GetRAOptions(pShPassMnger->basePM.pOptions, 0));
    gctBOOL             bScalarOnly = gcvTRUE;

    gcmASSERT(VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Pre_Machine ||
              VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Post_Low);

    /* Lower LL to MC firstly */
    CALL_SH_PASS(VIR_Lower_LowLevel_To_MachineCodeLevel, 0, gcvNULL);

    /* It must be called after lower */
    vscPM_SetCurPassLevel(&pShPassMnger->basePM, VSC_PASS_LEVEL_MC);

    /* Call (schedule) passes of MC by ourselves. Note we can only call these passes when new-CG is on */
    if (gcUseFullNewLinker(pShPassMnger->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti2))
    {
        CALL_SH_PASS(vscVIR_PreprocessMCShader, 0, gcvNULL);
        CALL_SH_PASS(vscVIR_PerformSpecialHwPatches, 0, gcvNULL);
        CALL_SH_PASS(VIR_Shader_CheckDual16able, 0, gcvNULL);
        CALL_SH_PASS(vscVIR_PutScalarConstToImm, 0, gcvNULL);
        CALL_SH_PASS(vscVIR_PutImmValueToUniform, 0, gcvNULL);
        /* vscVIR_PutImmValueToUniform may generate uniform variable for constant vector
         * put aubo pass after this pass to move uniform varialbe to uniform block if needed
         */
        CALL_SH_PASS(VSC_UF_CreateAUBOForCLShader, 0, gcvNULL);  /*if incoming shader is openCL compute shader*/
        /* do NOT insert a pass which could INVALID cfg between vscVIR_CheckPosAndDepthConflict and VSC_CPP_PerformOnShader.
         * overlapped depth and pos in same MOV:
         * MOV.t0t1           hp out  gl_FragDepth.hp.x, hp  gl_Position.hp.z
         * CPP could remove the new generated MOV in vscVIR_CheckPosAndDepthConflict to avoid Pos and depthand overlap.
         * case ES3-CTS.shaders.fragdepth.write.fragcoord_z could be tried for details.
         */
        CALL_SH_PASS(vscVIR_CheckPosAndDepthConflict, 0, gcvNULL);
        CALL_SH_PASS(vscVIR_ConvFrontFacing, 0, gcvNULL);

        cppPassData.cppFlag |= VSC_CPP_COPY_FROM_OUTPUT_PARAM;
        cppPassData.cppFlag |= VSC_CPP_FIND_NEAREST_DEF_INST;
        CALL_SH_PASS(VSC_CPP_PerformOnShader, 1, &cppPassData);

        CALL_SH_PASS(VSC_DCE_Perform, 1, gcvNULL);
        CALL_SH_PASS(vscVIR_FixDynamicIdxDep, 0, gcvNULL);
    }

    CALL_SH_PASS(vscVIR_PostMCCleanup, 0, &bRAEnabled);

    /* Call scalar again because MUL may be generated in vscVIR_PostMCCleanup. */
    CALL_SH_PASS(VIR_Lower_LowLevel_To_MachineCodeLevel, 0, &bScalarOnly);

    /* We are at end of MC level of VIR, so set this correct level */
    VIR_Shader_SetLevel(pShader, VIR_SHLEVEL_Post_Machine);

OnError:
    return errCode;
}

static VSC_ErrCode _PerformCodegen(VSC_SHADER_PASS_MANAGER*   pShPassMnger,
                                   SHADER_EXECUTABLE_PROFILE* pOutSEP,
                                   gctBOOL                    bSkipPepGen)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    gctBOOL             bCutDownWorkGroupSize = gcvFALSE;

    /* Set PM level firstly */
    vscPM_SetCurPassLevel(&pShPassMnger->basePM, VSC_PASS_LEVEL_CG);

    /* Do some preprocess work. */
    CALL_SH_PASS(vscVIR_PreprocessCGShader, 0, gcvNULL);

    /* Call (schedule) passes of CG by ourselves */
    CALL_SH_PASS(vscVIR_CheckEvisInstSwizzleRestriction, 0, gcvNULL);
    CALL_SH_PASS(VIR_RA_PerformUniformAlloc, 0, gcvNULL);
    CALL_SH_PASS(vscVIR_CheckCstRegFileReadPortLimitation, 0, gcvNULL);

    CALL_SH_PASS(VSC_IS_InstSched_PerformOnShader, 0, gcvNULL);
    CALL_SH_PASS(vscVIR_GenRobustBoundCheck, 0, gcvNULL);

    /* Start RA. */
    CALL_SH_PASS_WITHOUT_ERRCHECK(VIR_RA_LS_PerformTempRegAlloc, 0, &bCutDownWorkGroupSize);
    if (bCutDownWorkGroupSize)
    {
        /* Cut down the workGroupSize, then run RA again. */
        CALL_SH_PASS(vscVIR_CutDownWorkGroupSize, 0, gcvNULL);
        CALL_SH_PASS(VIR_RA_LS_PerformTempRegAlloc, 0, gcvNULL);
    }
    else
    {
        ON_ERROR(errCode, "VIR_RA_LS_PerformTempRegAlloc");
    }

    CALL_SH_PASS(VSC_IS_InstSched_PerformOnShader, 1, gcvNULL);

    /* Record the instruction status here so we can use it for the following passes. */
    CALL_SH_PASS(vscVIR_RecordInstructionStatus, 0, gcvNULL);

    CALL_SH_PASS(vscVIR_PostCGCleanup, 0, gcvNULL);
    CALL_SH_PASS(VSC_MC_GEN_MachineCodeGen, 0, gcvNULL);

    /* Record the instruction status. */
    CALL_SH_PASS(vscVIR_RecordInstructionStatus, 0, gcvNULL);

    if (pOutSEP)
    {
        VSC_SEP_GEN_PRIV_DATA   sepGenPrvData = { pOutSEP, bSkipPepGen };

        if (!(VSC_OPTN_MCGenOptions_GetSwitchOn(VSC_OPTN_Options_GetMCGenOptions(pShPassMnger->basePM.pOptions, 0)) &&
              VSC_OPTN_RAOptions_GetSwitchOn(VSC_OPTN_Options_GetRAOptions(pShPassMnger->basePM.pOptions, 0))))
        {
            /* Oops, why you ask me to generate SEP but you did not set mc-gen and RA?? */
            gcmASSERT(gcvFALSE);
            errCode = VSC_ERR_INVALID_ARGUMENT;
            ON_ERROR(errCode, "SEP specified, but RA or mc-gen is disabled");
        }

        CALL_SH_PASS(vscVIR_GenerateSEP, 0, &sepGenPrvData);
    }

OnError:
    return errCode;
}

static VSC_ErrCode _DoHLPreCompilation(VSC_SHADER_PASS_MANAGER* pShPassMnger)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_ShLevel         libShLevel;

    vscPM_SetCurPassLevel(&pShPassMnger->basePM, VSC_PASS_LEVEL_HL);

    /* Do lib link */
    if (pShPassMnger->pCompilerParam->pShLibLinkTable)
    {
        libShLevel = (VIR_ShLevel)pShPassMnger->pCompilerParam->pShLibLinkTable->pShLibLinkEntries[0].applyLevel;

        if (libShLevel == VIR_SHLEVEL_Pre_High)
        {
            CALL_SH_PASS(VIR_LinkExternalLibFunc, 0, gcvNULL);
            ON_ERROR(errCode, "Lib link");
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _DoMLPreCompilation(VSC_SHADER_PASS_MANAGER* pShPassMnger)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_ShLevel         libShLevel;

    vscPM_SetCurPassLevel(&pShPassMnger->basePM, VSC_PASS_LEVEL_ML);

    /* Do lib link */
    if (pShPassMnger->pCompilerParam->pShLibLinkTable)
    {
        libShLevel = (VIR_ShLevel)pShPassMnger->pCompilerParam->pShLibLinkTable->pShLibLinkEntries[0].applyLevel;

        if (libShLevel == VIR_SHLEVEL_Pre_Medium)
        {
            CALL_SH_PASS(VIR_LinkExternalLibFunc, 2, gcvNULL);
            ON_ERROR(errCode, "Lib link");
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _DoLLPreCompilation(VSC_SHADER_PASS_MANAGER* pShPassMnger)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_ShLevel         libShLevel;

    vscPM_SetCurPassLevel(&pShPassMnger->basePM, VSC_PASS_LEVEL_LL);

    /* Do lib link */
    if (pShPassMnger->pCompilerParam->pShLibLinkTable)
    {
        libShLevel = (VIR_ShLevel)pShPassMnger->pCompilerParam->pShLibLinkTable->pShLibLinkEntries[0].applyLevel;

        if (libShLevel == VIR_SHLEVEL_Pre_Low)
        {
            CALL_SH_PASS(VIR_LinkExternalLibFunc, 4, gcvNULL);
            ON_ERROR(errCode, "Lib link");
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _DoMCPreCompilation(VSC_SHADER_PASS_MANAGER* pShPassMnger)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_ShLevel         libShLevel;

    vscPM_SetCurPassLevel(&pShPassMnger->basePM, VSC_PASS_LEVEL_MC);

    /* Do lib link */
    if (pShPassMnger->pCompilerParam->pShLibLinkTable)
    {
        libShLevel = (VIR_ShLevel)pShPassMnger->pCompilerParam->pShLibLinkTable->pShLibLinkEntries[0].applyLevel;

        if (libShLevel == VIR_SHLEVEL_Pre_Machine)
        {
            CALL_SH_PASS(VIR_LinkExternalLibFunc, 6, gcvNULL);
            ON_ERROR(errCode, "Lib link");
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _DoHLPostCompilation(VSC_SHADER_PASS_MANAGER* pShPassMnger)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_ShLevel         libShLevel;

    vscPM_SetCurPassLevel(&pShPassMnger->basePM, VSC_PASS_LEVEL_HL);

    /* Do lib link */
    if (pShPassMnger->pCompilerParam->pShLibLinkTable)
    {
        libShLevel = (VIR_ShLevel)pShPassMnger->pCompilerParam->pShLibLinkTable->pShLibLinkEntries[0].applyLevel;

        if (libShLevel == VIR_SHLEVEL_Post_High)
        {
            CALL_SH_PASS(VIR_LinkExternalLibFunc, 1, gcvNULL);
            ON_ERROR(errCode, "Lib link");
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _DoMLPostCompilation(VSC_SHADER_PASS_MANAGER* pShPassMnger)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_ShLevel         libShLevel;
    gctBOOL             bIsRecompiler = (pShPassMnger->pCompilerParam->cfg.cFlags & VSC_COMPILER_FLAG_RECOMPILER);
    VSC_IL_PASS_DATA    ilPassData = { 1, gcvTRUE };

    vscPM_SetCurPassLevel(&pShPassMnger->basePM, VSC_PASS_LEVEL_ML);

    /* Do lib link */
    if (pShPassMnger->pCompilerParam->pShLibLinkTable)
    {
        libShLevel = (VIR_ShLevel)pShPassMnger->pCompilerParam->pShLibLinkTable->pShLibLinkEntries[0].applyLevel;

        if (libShLevel == VIR_SHLEVEL_Post_Medium)
        {
            VSC_EXTERNAL_LINK_PASS_DATA externalLinkPassData = { gcvFALSE };
            VSC_CPP_PASS_DATA           cppPassData = { VSC_CPP_NONE, gcvTRUE };

            /* If this is from recompiler, we need to do these processes before linking external lib functions. */
            if (bIsRecompiler)
            {
                /* Link intrinsic functions. */
                CALL_SH_PASS(VIR_LinkInternalLibFunc, 0, gcvNULL);

                /* Do some preprocess works for inline. */
                CALL_SH_PASS(vscVIR_CheckMustInlineFuncForML, 0, gcvNULL);

                /* Inline some always inline functions. */
                CALL_SH_PASS(VSC_IL_PerformOnShader, 0, &ilPassData);

                CALL_SH_PASS(vscVIR_ConvertVirtualInstructions, 0, gcvNULL);
                CALL_SH_PASS(VSC_CPP_PerformOnShader, 0, &cppPassData);
            }

            CALL_SH_PASS(VIR_LinkExternalLibFunc, 3, &externalLinkPassData);
            ON_ERROR(errCode, "Lib link");

            if (externalLinkPassData.bChanged)
            {
                /* Do some preprocess works for inline. */
                CALL_SH_PASS(vscVIR_CheckMustInlineFuncForML, 0, gcvNULL);

                /* Inline some always inline functions. */
                ilPassData.passIndex = 2;
                CALL_SH_PASS(VSC_IL_PerformOnShader, 0, &ilPassData);
                CALL_SH_PASS(vscVIR_ConvertVirtualInstructions, 0, gcvNULL);

                /* Initialize variables here to make CPP work. */
                CALL_SH_PASS(vscVIR_InitializeVariables, 0, gcvNULL);

                cppPassData.cppFlag |= VSC_CPP_USE_SRC_TYPE_FROM_MOVE;
                CALL_SH_PASS(VSC_CPP_PerformOnShader, 0, &cppPassData);
            }
        }
    }

    /* Link intrinsic functions. */
    CALL_SH_PASS(VIR_LinkInternalLibFunc, 0, gcvNULL);

OnError:
    return errCode;
}

static VSC_ErrCode _DoLLPostCompilation(VSC_SHADER_PASS_MANAGER* pShPassMnger)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_ShLevel         libShLevel;

    vscPM_SetCurPassLevel(&pShPassMnger->basePM, VSC_PASS_LEVEL_LL);

    /* Do lib link */
    if (pShPassMnger->pCompilerParam->pShLibLinkTable)
    {
        libShLevel = (VIR_ShLevel)pShPassMnger->pCompilerParam->pShLibLinkTable->pShLibLinkEntries[0].applyLevel;

        if (libShLevel == VIR_SHLEVEL_Post_Low)
        {
            CALL_SH_PASS(VIR_LinkExternalLibFunc, 5, gcvNULL);
            ON_ERROR(errCode, "Lib link");
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _DoMCPostCompilation(VSC_SHADER_PASS_MANAGER* pShPassMnger)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_ShLevel         libShLevel;

    vscPM_SetCurPassLevel(&pShPassMnger->basePM, VSC_PASS_LEVEL_MC);

    /* Do lib link */
    if (pShPassMnger->pCompilerParam->pShLibLinkTable)
    {
        libShLevel = (VIR_ShLevel)pShPassMnger->pCompilerParam->pShLibLinkTable->pShLibLinkEntries[0].applyLevel;

        if (libShLevel == VIR_SHLEVEL_Post_Machine)
        {
            CALL_SH_PASS(VIR_LinkExternalLibFunc, 7, gcvNULL);
            ON_ERROR(errCode, "Lib link");
        }
    }

OnError:
    return errCode;
}

gctUINT _GetCompLevelFromExpectedShaderLevel(VIR_ShLevel expectedShLevel)
{
    gctUINT    compilationLevel = 0;

    if (expectedShLevel == VIR_SHLEVEL_Pre_High ||
        expectedShLevel == VIR_SHLEVEL_Post_High)
    {
        compilationLevel = VSC_COMPILER_FLAG_COMPILE_TO_HL;
    }

    if (expectedShLevel == VIR_SHLEVEL_Pre_Medium ||
        expectedShLevel == VIR_SHLEVEL_Post_Medium)
    {
        compilationLevel = VSC_COMPILER_FLAG_COMPILE_TO_HL |
                           VSC_COMPILER_FLAG_COMPILE_TO_ML;
    }

    if (expectedShLevel == VIR_SHLEVEL_Pre_Low ||
        expectedShLevel == VIR_SHLEVEL_Post_Low)
    {
        compilationLevel = VSC_COMPILER_FLAG_COMPILE_TO_HL |
                           VSC_COMPILER_FLAG_COMPILE_TO_ML |
                           VSC_COMPILER_FLAG_COMPILE_TO_LL;
    }

    if (expectedShLevel == VIR_SHLEVEL_Pre_Machine ||
        expectedShLevel == VIR_SHLEVEL_Post_Machine)
    {
        compilationLevel = VSC_COMPILER_FLAG_COMPILE_TO_HL |
                           VSC_COMPILER_FLAG_COMPILE_TO_ML |
                           VSC_COMPILER_FLAG_COMPILE_TO_LL |
                           VSC_COMPILER_FLAG_COMPILE_TO_MC;
    }

    return compilationLevel;
}

VIR_ShLevel _GetExpectedLastLevel(VSC_SHADER_COMPILER_PARAM* pCompilerParam)
{
    if (pCompilerParam->cfg.cFlags & VSC_COMPILER_FLAG_COMPILE_TO_MC)
    {
        return VIR_SHLEVEL_Post_Machine;
    }
    else if (pCompilerParam->cfg.cFlags & VSC_COMPILER_FLAG_COMPILE_TO_LL)
    {
        return VIR_SHLEVEL_Post_Low;
    }
    else if (pCompilerParam->cfg.cFlags & VSC_COMPILER_FLAG_COMPILE_TO_ML)
    {
        return VIR_SHLEVEL_Post_Medium;
    }
    else if (pCompilerParam->cfg.cFlags & VSC_COMPILER_FLAG_COMPILE_TO_HL)
    {
        return VIR_SHLEVEL_Post_High;
    }
    else
    {
        return VIR_SHLEVEL_Unknown;
    }
}

#define NEED_HL_PRETRANSFORM(pShader)                                              \
    (VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Pre_High)

#define NEED_ML_PRETRANSFORM(pShader)                                              \
    (VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Pre_Medium)

#define NEED_LL_PRETRANSFORM(pShader)                                              \
    (VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Pre_Low)

#define NEED_MC_PRETRANSFORM(pShader)                                              \
    (VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Pre_Machine)

#define NEED_HL_TRANSFORMS(pShader, expectedLastLevel)                             \
    ((expectedLastLevel) >= VIR_SHLEVEL_Post_High &&                               \
     VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Pre_High)

#define NEED_ML_TRANSFORMS(pShader, expectedLastLevel)                             \
    ((expectedLastLevel) >= VIR_SHLEVEL_Post_Medium &&                             \
     (VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Pre_Medium ||                  \
      VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Post_High))

#define NEED_LL_TRANSFORMS(pShader, expectedLastLevel)                             \
    ((expectedLastLevel) >= VIR_SHLEVEL_Post_Low &&                                \
     (VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Pre_Low ||                     \
      VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Post_Medium))

#define NEED_MC_TRANSFORMS(pShader, expectedLastLevel)                             \
    ((expectedLastLevel) >= VIR_SHLEVEL_Post_Machine &&                            \
     (VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Pre_Machine ||                 \
      VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Post_Low))

#define NEED_HL_POSTTRANSFORM(pShader)                                             \
    (VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Post_High)

#define NEED_ML_POSTTRANSFORM(pShader)                                             \
    (VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Post_Medium)

#define NEED_LL_POSTTRANSFORM(pShader)                                             \
    (VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Post_Low)

#define NEED_MC_POSTTRANSFORM(pShader)                                             \
    (VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Post_Machine)

#define CAN_PERFORM_CODE_GEN(pCompilerParam)                                       \
    (VIR_Shader_GetLevel((VIR_Shader*)((pCompilerParam)->hShader)) == VIR_SHLEVEL_Post_Machine && \
     ((pCompilerParam)->cfg.cFlags & VSC_COMPILER_FLAG_COMPILE_CODE_GEN))

VSC_ErrCode _CompileShaderInternal(VSC_SHADER_PASS_MANAGER*   pShPassMnger,
                                   SHADER_EXECUTABLE_PROFILE* pOutSEP,
                                   gctBOOL                    bSkipPepGen)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Shader*         pShader = (VIR_Shader*)pShPassMnger->pCompilerParam->hShader;
    VIR_ShLevel         expectedLastLevel = _GetExpectedLastLevel(pShPassMnger->pCompilerParam);

    /* Make sure target client api of shader is same as invoking driver's client api. Core compiler
       can not support client-cross compilation */
    gcmASSERT(pShader->clientApiVersion == pShPassMnger->pCompilerParam->cfg.ctx.clientAPI);

    errCode = _PreprocessShader(pShPassMnger);
    ON_ERROR(errCode, "Preprocess shader");

    /* HL pre-compiling */
    if (NEED_HL_PRETRANSFORM(pShader))
    {
        errCode = _DoHLPreCompilation(pShPassMnger);
        ON_ERROR(errCode, "Do HL pre-compiling");
    }

    /* HL compiling */
    if (NEED_HL_TRANSFORMS(pShader, expectedLastLevel))
    {
        errCode = _CompileShaderAtHighLevel(pShPassMnger);
        ON_ERROR(errCode, "HL compiling");
    }

    /* HL post-compiling */
    if (NEED_HL_POSTTRANSFORM(pShader))
    {
        errCode = _DoHLPostCompilation(pShPassMnger);
        ON_ERROR(errCode, "Do HL post-compiling");
    }

    /* ML pre-compiling */
    if (NEED_ML_PRETRANSFORM(pShader))
    {
        errCode = _DoMLPreCompilation(pShPassMnger);
        ON_ERROR(errCode, "Do ML pre-compiling");
    }

    /* ML compiling */
    if (NEED_ML_TRANSFORMS(pShader, expectedLastLevel))
    {
        errCode = _CompileShaderAtMedLevel(pShPassMnger);
        ON_ERROR(errCode, "ML compiling");
    }

    /* ML post-compiling */
    if (NEED_ML_POSTTRANSFORM(pShader))
    {
        errCode = _DoMLPostCompilation(pShPassMnger);
        ON_ERROR(errCode, "Do ML post-compiling");
    }

    /* LL pre-compiling */
    if (NEED_LL_PRETRANSFORM(pShader))
    {
        errCode = _DoLLPreCompilation(pShPassMnger);
        ON_ERROR(errCode, "Do LL pre-compiling");
    }

    /* LL compiling */
    if (NEED_LL_TRANSFORMS(pShader, expectedLastLevel))
    {
        errCode = _CompileShaderAtLowLevel(pShPassMnger);
        ON_ERROR(errCode, "LL compiling");
    }

    /* LL post-compiling */
    if (NEED_LL_POSTTRANSFORM(pShader))
    {
        errCode = _DoLLPostCompilation(pShPassMnger);
        ON_ERROR(errCode, "Do LL post-compiling");
    }

    /* MC pre-compiling */
    if (NEED_MC_PRETRANSFORM(pShader))
    {
        errCode = _DoMCPreCompilation(pShPassMnger);
        ON_ERROR(errCode, "Do MC pre-compiling");
    }

    /* MC compiling */
    if (NEED_MC_TRANSFORMS(pShader, expectedLastLevel))
    {
        errCode = _CompileShaderAtMCLevel(pShPassMnger);
        ON_ERROR(errCode, "MC compiling");
    }

    /* MC post-compiling */
    if (NEED_MC_POSTTRANSFORM(pShader))
    {
        errCode = _DoMCPostCompilation(pShPassMnger);
        ON_ERROR(errCode, "Do MC post-compiling");
    }

    /* Perform codegen */
    if (CAN_PERFORM_CODE_GEN(pShPassMnger->pCompilerParam))
    {
        errCode = _PerformCodegen(pShPassMnger, pOutSEP, bSkipPepGen);
        ON_ERROR(errCode, "Perform codegen");
    }

    errCode = _PostprocessShader(pShPassMnger, pOutSEP);
    ON_ERROR(errCode, "Post-process shader");

OnError:
    return errCode;
}

static VSC_ErrCode _PreprocessLinkLibs(VSC_SHADER_COMPILER_PARAM* pCompilerParam,
                                       VSC_SHADER_LIB_LINK_TABLE* pShLibLinkTable)
{
    gctUINT                     i;
    VIR_ShLevel                 thisShLevel, maxShLevel = VIR_SHLEVEL_Unknown, expectedShLevel;
    VSC_SHADER_COMPILER_PARAM   compParam;

    memset(&compParam, 0, sizeof(VSC_SHADER_COMPILER_PARAM));

    /* Get maximum shader level among these libs */
    for (i = 0; i < pShLibLinkTable->shLinkEntryCount; i ++)
    {
        thisShLevel = (VIR_ShLevel)pShLibLinkTable->pShLibLinkEntries[i].applyLevel;

        if (thisShLevel > maxShLevel)
        {
            maxShLevel = thisShLevel;
        }
    }

    /* Compile libs to expected shader level */
    for (i = 0; i < pShLibLinkTable->shLinkEntryCount; i ++)
    {
        thisShLevel = (VIR_ShLevel)pShLibLinkTable->pShLibLinkEntries[i].applyLevel;

        expectedShLevel = vscMAX(thisShLevel, maxShLevel);

        /* We need make sure shader level of lib is not smaller than start shader level of main shader */
        expectedShLevel = vscMAX(expectedShLevel, VIR_Shader_GetLevel((VIR_Shader*)pCompilerParam->hShader));

        if (thisShLevel < expectedShLevel
            &&
            pShLibLinkTable->pShLibLinkEntries[i].hShaderLib)
        {
            compParam.hShader = pShLibLinkTable->pShLibLinkEntries[i].hShaderLib;
            memcpy(&compParam.cfg, &pCompilerParam->cfg, sizeof(VSC_COMPILER_CONFIG));
            compParam.cfg.cFlags &= ~(VSC_COMPILER_FLAG_COMPILE_TO_HL |
                                      VSC_COMPILER_FLAG_COMPILE_TO_ML |
                                      VSC_COMPILER_FLAG_COMPILE_TO_LL |
                                      VSC_COMPILER_FLAG_COMPILE_TO_MC);
            compParam.cfg.cFlags |= _GetCompLevelFromExpectedShaderLevel(expectedShLevel);

            if (vscCompileShader(&compParam, gcvNULL) != gcvSTATUS_OK)
            {
                return VSC_ERR_INVALID_ARGUMENT;
            }
        }
    }

    return VSC_ERR_NONE;
}

gceSTATUS vscCompileShader(VSC_SHADER_COMPILER_PARAM* pCompilerParam,
                           SHADER_EXECUTABLE_PROFILE* pOutSEP)
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_Shader*             pShader = (VIR_Shader*)pCompilerParam->hShader;
    VSC_SHADER_PASS_MANAGER shPassMnger;
    VSC_PASS_MM_POOL        passMemPool;
    VSC_OPTN_Options        options;

    vscInitializePassMMPool(&passMemPool);
    vscInitializeOptions(&options,
                         &pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg,
                         pCompilerParam->cfg.cFlags,
                         pCompilerParam->cfg.optFlags);

    /* Initialize our pass manager who will take over all passes
       whether to trigger or not */
    vscSPM_Initialize(&shPassMnger,
                      pCompilerParam,
                      &passMemPool,
                      gcvFALSE,
                      pShader->dumper,
                      &options,
                      VSC_PM_MODE_SEMI_AUTO);

    if (pCompilerParam->pShLibLinkTable)
    {
        errCode = _PreprocessLinkLibs(pCompilerParam, pCompilerParam->pShLibLinkTable);
        ON_ERROR(errCode, "Pre-process link libs");
    }

    /* Do real compiling jobs */
    errCode = _CompileShaderInternal(&shPassMnger, pOutSEP, gcvFALSE);
    ON_ERROR(errCode, "Compiler internal");

OnError:
    vscFinalizeOptions(&options);
    vscSPM_Finalize(&shPassMnger, gcvFALSE);
    vscFinalizePassMMPool(&passMemPool);

    return vscERR_CastErrCode2GcStatus(errCode);
}

gcSHADER_KIND vscGetShaderKindFromShaderHandle(
    SHADER_HANDLE hShader)
{
    VIR_Shader  *vShader = (VIR_Shader*)hShader;
    gcSHADER_KIND shaderKind = gcSHADER_TYPE_UNKNOWN;

    switch (VIR_Shader_GetKind(vShader))
    {
    case VIR_SHADER_VERTEX:
        shaderKind = gcSHADER_TYPE_VERTEX;
        break;
    case VIR_SHADER_FRAGMENT:
        shaderKind = gcSHADER_TYPE_FRAGMENT;
        break;
    case VIR_SHADER_COMPUTE:
        shaderKind = gcSHADER_TYPE_COMPUTE;
        break;
    case VIR_SHADER_TESSELLATION_CONTROL:
        shaderKind = gcSHADER_TYPE_TCS;
        break;
    case VIR_SHADER_TESSELLATION_EVALUATION:
        shaderKind = gcSHADER_TYPE_TES;
        break;
    case VIR_SHADER_GEOMETRY:
        shaderKind = gcSHADER_TYPE_GEOMETRY;
        break;
    case VIR_SHADER_PRECOMPILED:
        shaderKind = gcSHADER_TYPE_PRECOMPILED;
        break;
    case VIR_SHADER_LIBRARY:
        shaderKind = gcSHADER_TYPE_LIBRARY;
        break;
    default:
        break;
    }

    return shaderKind;
}

void vscSetDriverVIRPath(gctBOOL bUseVIRPath)
{
    gcOPTIMIZER_OPTION *option = gcGetOptimizerOption();
    if (!bUseVIRPath)
    {
        option->DriverVIRPath       = gcvFALSE;
        option->oclInt64InVir         = gcvFALSE;
        option->oclUniformForConstant = gcvFALSE;
    }
    else
    {
        option->DriverVIRPath       = gcvTRUE;
        option->oclInt64InVir         = gcvTRUE;
        option->oclUniformForConstant = gcvTRUE;
    }
}

void vscSetIsLibraryShader(
        SHADER_HANDLE hShader,
        gctBOOL bIsLibraryShader)
{
    VIR_Shader  *vShader = (VIR_Shader*)hShader;
    if (vShader)
        VIR_Shader_SetLibraryShader(vShader, bIsLibraryShader);
}

gctUINT
vscGetHWMaxFreeRegCount(
    IN VSC_HW_CONFIG   *pHwCfg
    )
{
    gctUINT maxFreeReg = 0;

    if (pHwCfg->hwFeatureFlags.computeOnly)
    {
        /* 128: total temp registers per shader.
        **   4: 4 registers for compute shader.
        **   3: partial page (up to 3 registers) used by current shader previously.
        ** free registers: 128 - 4 - 3 = 121
        */
        maxFreeReg = 121;  /* we can use more registers for compute only chip */
    }
    else if (pHwCfg->hwFeatureFlags.hasHalti5   /* Halti5 HW with TS/GS supports */
             && pHwCfg->hwFeatureFlags.supportGS
             && pHwCfg->hwFeatureFlags.supportTS
        )
    {
        /*  128: total temp registers per workgroup (128 = 512/4: total temp register per core/4)
            16: reserved 1 page (16 registers) each for other shaders.
            3: partial page (up to 3 registers) used by current shader previously.
            free registers: 128 - 16 - 3 = 109 */
        maxFreeReg = 109;
    }
    else
    {
        /* 128: total temp registers per workgroup.
            8: reserved 1 page (8 registers) for VS or PS.
            7: partial page (up to 7 registers) used by PS or VS.
            free registers: 128 - 8 - 7 = 113 */
        maxFreeReg = 113;
    }

    return maxFreeReg;
}

#define _cldFILENAME_MAX 1024
gceSTATUS vscGetTemporaryDir(
    OUT gctSTRING gcTmpDir
    )
{
    gctSTRING TmpDir = gcvNULL;
    gceSTATUS   status          = gcvSTATUS_OK;

#if defined(UNDER_CE)
    char cwdpath[MAX_PATH];
#elif defined(ANDROID)
    char path[_cldFILENAME_MAX];
#endif

    gcoOS_GetEnv(gcvNULL,
        "TMPDIR",
        &TmpDir);
    if (!TmpDir) {
        gcoOS_GetEnv(gcvNULL,
            "TEMP",
            &TmpDir);
    }
    if (!TmpDir) {
        gcoOS_GetEnv(gcvNULL,
            "TMP",
            &TmpDir);
    }
    if (!TmpDir) {
        gcoOS_GetEnv(gcvNULL,
                    "TEMPDIR",
                    &TmpDir);
    }
#if defined(LINUX) && !defined(ANDROID)
    if (!TmpDir)
    {
        int ret = access("/tmp", R_OK | W_OK);
        if (ret == 0)
        {
            /* /tmp is exist, readable and writable, use it as temp directory */
            TmpDir = "/tmp";
        }
    }
#endif
    if (!TmpDir)
    {
#if defined(UNDER_CE)
        /* Wince has no relative path */
        gcoOS_GetCwd(gcvNULL, MAX_PATH, cwdpath);
        TmpDir = cwdpath;
#elif defined(ANDROID)
        gctSIZE_T len=0;
        gctFILE filp=gcvNULL;
        gctINT i=0;
        static const char prefix[] = "/data/data/";

        /* Could these fail? */
        gcoOS_Open(gcvNULL, "/proc/self/cmdline", gcvFILE_READ, &filp);
        gcoOS_Read(gcvNULL, filp, _cldFILENAME_MAX - 1, path, &len);
        gcoOS_Close(gcvNULL, filp);

        /* Add terminator. */
        path[len] = '\0';

        if (strchr(path, '/')) {
            /* Like a relative path or abs path. */
            TmpDir = ".";
        }
        else if (strchr(path, '.') && len < _cldFILENAME_MAX - sizeof (prefix)) {
            /* Like an android apk. */
            for (i = len; i >= 0; i--) {
                path[i + sizeof (prefix) - 1] = path[i];
            }

            gcoOS_MemCopy(path, prefix, sizeof (prefix) - 1);
            gcoOS_StrCatSafe(path, _cldFILENAME_MAX, "/cache/");

            TmpDir = path;
        }
        else {
            TmpDir = ".";
        }
#else
        TmpDir = (gctSTRING) ".";
#endif
    }

    gcmONERROR(gcoOS_StrCopySafe(gcTmpDir, _cldFILENAME_MAX,TmpDir));

OnError:
    return status;
}

