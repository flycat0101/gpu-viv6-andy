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
#include "vir/transform/gc_vsc_vir_param_opts.h"
#include "vir/transform/gc_vsc_vir_uniform.h"
#include "vir/transform/gc_vsc_vir_static_patch.h"
#include "vir/transform/gc_vsc_vir_vectorization.h"
#include "vir/codegen/gc_vsc_vir_inst_scheduler.h"
#include "vir/codegen/gc_vsc_vir_reg_alloc.h"
#include "vir/codegen/gc_vsc_vir_mc_gen.h"
#include "vir/codegen/gc_vsc_vir_ep_gen.h"
#include "vir/codegen/gc_vsc_vir_ep_back_patch.h"
#include "vir/linker/gc_vsc_vir_linker.h"


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
            gcoOS_Free(gcvNULL, pSEP->staticPrivMapping.privConstantMapping.pPrivmConstantEntries[i].commonPrivm.pPrivateData);
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
            gcoOS_Free(gcvNULL, pSEP->dynamicPrivMapping.privSamplerMapping.pPrivSamplerEntries[i].commonPrivm.pPrivateData);
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
            gcoOS_Free(gcvNULL, pSEP->dynamicPrivMapping.privOutputMapping.pPrivOutputEntries[i].commonPrivm.pPrivateData);
            pSEP->dynamicPrivMapping.privOutputMapping.pPrivOutputEntries[i].commonPrivm.pPrivateData = gcvNULL;
        }
    }

    if (pSEP->dynamicPrivMapping.privOutputMapping.pPrivOutputEntries)
    {
        gcoOS_Free(gcvNULL, pSEP->dynamicPrivMapping.privOutputMapping.pPrivOutputEntries);
        pSEP->dynamicPrivMapping.privOutputMapping.pPrivOutputEntries = gcvNULL;
        pSEP->dynamicPrivMapping.privOutputMapping.countOfEntries = 0;
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
    pCnstHwLocMapping->hwLoc.hwRegNo = NOT_ASSIGNED;
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

gceSTATUS vscReferenceShader(SHADER_HANDLE hShader)
{
    VIR_Shader *pVirShader = (VIR_Shader*)hShader;

    ++pVirShader->refCount;

    return gcvSTATUS_OK;
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

    vscReferenceShader(pVirShader);

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

    if (--pVirShader->refCount == 0)
    {
        errCode = VIR_Shader_Destroy(pVirShader);
        ON_ERROR(errCode, "Shader Destroy");

        gcmONERROR(gcoOS_Free(gcvNULL, pVirShader));
    }

OnError:
    return (status == gcvSTATUS_OK) ? vscERR_CastErrCode2GcStatus(errCode) : status;
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

    vscReferenceShader(pToVirShader);

OnError:
    return (status == gcvSTATUS_OK) ? vscERR_CastErrCode2GcStatus(errCode) : status;
}

gceSTATUS vscPrintShader(SHADER_HANDLE hShader,
                         gctFILE hFile,
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
    VIR_Shader_IOBuffer buf;

    if (*pBinary == gcvNULL)
    {
        errCode = VIR_Shader_Save((VIR_Shader *)hShader, &buf);
        if (errCode == VSC_ERR_NONE)
        {
            *pBinary = (void*)(buf.buffer);
            *pSizeInByte = buf.curPos;
        }
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
    VIR_Shader_IOBuffer buf;
    VIR_Shader *        readShader = gcvNULL;

    buf.allocatedBytes = sizeInByte;
    buf.buffer = (gctCHAR *)pBinary;
    buf.curPos = 0;
    gcmONERROR(gcoOS_Allocate(gcvNULL,
                                sizeof(VIR_Shader),
                                (gctPOINTER*)&readShader));

    errCode = VIR_Shader_Construct(gcvNULL, VIR_SHADER_UNKNOWN, readShader);
    ON_ERROR(errCode, "Shader Load");

    buf.shader = readShader;

    errCode = VIR_Shader_Read(readShader, &buf);
    ON_ERROR(errCode, "Shader Load");
    if (bFreeBinary)
    {
        VIR_IO_Finalize(&buf);  /* reclaim allocated buffer */
    }
    vscReferenceShader(readShader);

    *pShaderHandle = readShader;
    return gcvSTATUS_OK;

OnError:
    VIR_IO_Finalize(&buf);  /* reclaim allocated buffer */
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

    /*
    ** Generate combined sampler for separated samper and separated texture.
    ** We need to do this before patch lib linking.
    */
    CALL_SH_PASS(vscVIR_GenCombinedSampler, 0, gcvNULL);

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
    gctBOOL             bGlobalCPP = gcvTRUE;
    gctBOOL             bCheckAlwaysInlineOnly = gcvFALSE;

    gcmASSERT(VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Pre_Low ||
              VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Post_Medium);

    /* Lower ML to LL firstly */
    CALL_SH_PASS(VIR_Lower_MiddleLevel_To_LowLevel, 0, &bRAEnabled);

    /* It must be called after lower */
    vscPM_SetCurPassLevel(&pShPassMnger->basePM, VSC_PASS_LEVEL_LL);

    /* Call (schedule) passes of LL by ourselves */
    CALL_SH_PASS(vscVIR_PreprocessLLShader, 0, gcvNULL);
    CALL_SH_PASS(VSC_IL_PerformOnShader, 0, &bCheckAlwaysInlineOnly);
    CALL_SH_PASS(VSC_SCPP_PerformOnShader, 0, gcvNULL);
    CALL_SH_PASS(VIR_LoopOpts_PerformOnShader, 0, gcvNULL);
    CALL_SH_PASS(VIR_CFO_PerformOnShader, 0, gcvNULL);
    CALL_SH_PASS(vscVIR_ConvertVirtualInstructions, 0, gcvNULL);
    CALL_SH_PASS(VSC_CPF_PerformOnShader, 0, gcvNULL);
    CALL_SH_PASS(vscVIR_InitializeVariables, 0, gcvNULL);
    CALL_SH_PASS(VSC_CPP_PerformOnShader, 0, &bGlobalCPP);
    CALL_SH_PASS(VSC_PARAM_Optimization_PerformOnShader, 0, gcvNULL);
    CALL_SH_PASS(VSC_SIMP_Simplification_PerformOnShader, 0, gcvNULL);
    CALL_SH_PASS(VSC_SCL_Scalarization_PerformOnShader, 0, gcvNULL);
    CALL_SH_PASS(VSC_PH_Peephole_PerformOnShader, 0, gcvNULL);
    /* peep hole may delete no used jmp, cfo could help to delete no-used label */
    CALL_SH_PASS(VIR_CFO_PerformOnShader, 0, gcvNULL);
    CALL_SH_PASS(VSC_LCSE_PerformOnShader, 0, gcvNULL);
    CALL_SH_PASS(VSC_DCE_Perform, 0, gcvNULL);
    CALL_SH_PASS(vscVIR_AdjustPrecision, 0, gcvNULL);
    CALL_SH_PASS(vscVIR_DoLocalVectorization, 0, gcvNULL);
    CALL_SH_PASS(vscVIR_AddOutOfBoundCheckSupport, 0, gcvNULL);

    /* We are at the end of LL of VIR, so set this correct level */
    VIR_Shader_SetLevel(pShader, VIR_SHLEVEL_Post_Low);

OnError:
    return errCode;
}

static VSC_ErrCode _CompileShaderAtMCLevel(VSC_SHADER_PASS_MANAGER* pShPassMnger)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Shader*         pShader = (VIR_Shader*)pShPassMnger->pCompilerParam->hShader;
    gctBOOL             bGlobalCPP = gcvFALSE;

    gcmASSERT(VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Pre_Machine ||
              VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Post_Low);

    /* Lower LL to MC firstly */
    CALL_SH_PASS(VIR_Lower_LowLevel_To_MachineCodeLevel, 0, gcvNULL);

    /* It must be called after lower */
    vscPM_SetCurPassLevel(&pShPassMnger->basePM, VSC_PASS_LEVEL_MC);

    /* Call (schedule) passes of MC by ourselves. Note we can only call these passes when new-CG is on */
    if (gcUseFullNewLinker(pShPassMnger->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti2))
    {
        CALL_SH_PASS(vscVIR_PerformSpecialHwPatches, 0, gcvNULL);
        CALL_SH_PASS(VIR_Shader_CheckDual16able, 0, gcvNULL);
        CALL_SH_PASS(vscVIR_PutScalarConstToImm, 0, gcvNULL);
        CALL_SH_PASS(vscVIR_PutImmValueToUniform, 0, gcvNULL);
        CALL_SH_PASS(vscVIR_CheckPosAndDepthConflict, 0, gcvNULL);
        CALL_SH_PASS(VSC_CPP_PerformOnShader, 1, &bGlobalCPP);
        CALL_SH_PASS(VSC_DCE_Perform, 1, gcvNULL);
        CALL_SH_PASS(vscVIR_FixDynamicIdxDep, 0, gcvNULL);
    }

    /* We are at end of MC level of VIR, so set this correct level */
    VIR_Shader_SetLevel(pShader, VIR_SHLEVEL_Post_Machine);

OnError:
    return errCode;
}

static VSC_ErrCode _PerformCodegen(VSC_SHADER_PASS_MANAGER*   pShPassMnger,
                                   SHADER_EXECUTABLE_PROFILE* pOutSEP)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    gctBOOL             bRAEnabled = VSC_OPTN_RAOptions_GetSwitchOn(VSC_OPTN_Options_GetRAOptions(pShPassMnger->basePM.pOptions, 0));

    /* Set PM level firstly */
    vscPM_SetCurPassLevel(&pShPassMnger->basePM, VSC_PASS_LEVEL_CG);

    /* Call (schedule) passes of CG by ourselves */
    CALL_SH_PASS(vscVIR_PreCleanup, 0, &bRAEnabled);
    CALL_SH_PASS(vscVIR_CheckEvisInstSwizzleRestriction, 0, gcvNULL);
    CALL_SH_PASS(VIR_RA_PerformUniformAlloc, 0, gcvNULL);
    CALL_SH_PASS(vscVIR_CheckCstRegFileReadPortLimitation, 0, gcvNULL);

    CALL_SH_PASS(VSC_IS_InstSched_PerformOnShader, 0, gcvNULL);

    CALL_SH_PASS(VIR_RA_LS_PerformTempRegAlloc, 0, gcvNULL);

    CALL_SH_PASS(VSC_IS_InstSched_PerformOnShader, 1, gcvNULL);

    CALL_SH_PASS(vscVIR_PostCleanup, 0, gcvNULL);
    CALL_SH_PASS(VSC_MC_GEN_MachineCodeGen, 0, gcvNULL);

    if (pOutSEP)
    {
        if (!(VSC_OPTN_MCGenOptions_GetSwitchOn(VSC_OPTN_Options_GetMCGenOptions(pShPassMnger->basePM.pOptions, 0)) &&
              VSC_OPTN_RAOptions_GetSwitchOn(VSC_OPTN_Options_GetRAOptions(pShPassMnger->basePM.pOptions, 0))))
        {
            /* Oops, why you ask me to generate SEP but you did not set mc-gen and RA?? */
            gcmASSERT(gcvFALSE);
            errCode = VSC_ERR_INVALID_ARGUMENT;
            ON_ERROR(errCode, "SEP specified, but RA or mc-gen is disabled");
        }

        CALL_SH_PASS(vscVIR_GenerateSEP, 0, pOutSEP);
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
        libShLevel = VIR_Shader_GetLevel((VIR_Shader*)pShPassMnger->pCompilerParam->pShLibLinkTable->pShLibLinkEntries[0].hShaderLib);

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
        libShLevel = VIR_Shader_GetLevel((VIR_Shader*)pShPassMnger->pCompilerParam->pShLibLinkTable->pShLibLinkEntries[0].hShaderLib);

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
        libShLevel = VIR_Shader_GetLevel((VIR_Shader*)pShPassMnger->pCompilerParam->pShLibLinkTable->pShLibLinkEntries[0].hShaderLib);

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
        libShLevel = VIR_Shader_GetLevel((VIR_Shader*)pShPassMnger->pCompilerParam->pShLibLinkTable->pShLibLinkEntries[0].hShaderLib);

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
        libShLevel = VIR_Shader_GetLevel((VIR_Shader*)pShPassMnger->pCompilerParam->pShLibLinkTable->pShLibLinkEntries[0].hShaderLib);

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
    gctBOOL             bCheckAlwaysInlineOnly = gcvTRUE;

    vscPM_SetCurPassLevel(&pShPassMnger->basePM, VSC_PASS_LEVEL_ML);

    /* If this is from recompiler, we need to do these processes before linking external lib functions. */
    if (bIsRecompiler)
    {
        /* Link intrinsic functions. */
        CALL_SH_PASS(VIR_LinkInternalLibFunc, 0, gcvNULL);

        /* Do some preprocess works for inline. */
        CALL_SH_PASS(vscVIR_PreprocessMLPostShader, 0, gcvNULL);

        /* Inline some always inline functions. */
        CALL_SH_PASS(VSC_IL_PerformOnShader, 0, &bCheckAlwaysInlineOnly);

        /* Do some postprocess works for inline. */
        CALL_SH_PASS(vscVIR_PostprocessMLPostShader, 0, gcvNULL);
    }

    /* Do lib link */
    if (pShPassMnger->pCompilerParam->pShLibLinkTable)
    {
        libShLevel = VIR_Shader_GetLevel((VIR_Shader*)pShPassMnger->pCompilerParam->pShLibLinkTable->pShLibLinkEntries[0].hShaderLib);

        if (libShLevel == VIR_SHLEVEL_Post_Medium)
        {
            CALL_SH_PASS(VIR_LinkExternalLibFunc, 3, gcvNULL);
            ON_ERROR(errCode, "Lib link");
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
        libShLevel = VIR_Shader_GetLevel((VIR_Shader*)pShPassMnger->pCompilerParam->pShLibLinkTable->pShLibLinkEntries[0].hShaderLib);

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
        libShLevel = VIR_Shader_GetLevel((VIR_Shader*)pShPassMnger->pCompilerParam->pShLibLinkTable->pShLibLinkEntries[0].hShaderLib);

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
                                   SHADER_EXECUTABLE_PROFILE* pOutSEP)
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
        errCode = _PerformCodegen(pShPassMnger, pOutSEP);
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
        thisShLevel = VIR_Shader_GetLevel((((VIR_Shader*)pShLibLinkTable->pShLibLinkEntries[i].hShaderLib)));

        if (thisShLevel > maxShLevel)
        {
            maxShLevel = thisShLevel;
        }
    }

    /* Compile libs to expected shader level */
    for (i = 0; i < pShLibLinkTable->shLinkEntryCount; i ++)
    {
        thisShLevel = VIR_Shader_GetLevel((((VIR_Shader*)pShLibLinkTable->pShLibLinkEntries[i].hShaderLib)));

        expectedShLevel = vscMAX(thisShLevel, maxShLevel);

        /* We need make sure shader level of lib is not smaller than start shader level of main shader */
        expectedShLevel = vscMAX(expectedShLevel, VIR_Shader_GetLevel((VIR_Shader*)pCompilerParam->hShader));

        if (thisShLevel < expectedShLevel)
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
    errCode = _CompileShaderInternal(&shPassMnger, pOutSEP);
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

