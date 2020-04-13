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


#include "gc_vsc.h"

#define EPSTART_SIG         gcmCC('E', 'P', 'S', 'S')   /* EP start signature. */
#define EPEND_SIG           gcmCC('E', 'P', 'E', 'E')   /* EP end signature. */
#define SEPSTART_SIG        gcmCC('S', 'E', 'P', 'S')   /* SEP start signature. */
#define SEPEND_SIG          gcmCC('S', 'E', 'P', 'E')   /* SEP end signature. */
#define KEPSTART_SIG        gcmCC('K', 'E', 'P', 'S')   /* KEP start signature. */
#define KEPEND_SIG          gcmCC('K', 'E', 'P', 'E')   /* KEP end signature. */
#define PEPSTART_SIG        gcmCC('P', 'E', 'P', 'S')   /* PEP start signature. */
#define PEPEND_SIG          gcmCC('P', 'E', 'P', 'E')   /* PEP end signature. */

/* SEP-related signature. */
#define IOMSTART_SIG        gcmCC('I', 'O', 'M', 'S')   /* Io mapping start signature. */
#define IOMEND_SIG          gcmCC('I', 'O', 'M', 'E')   /* Io mapping end signature. */
#define COMSTART_SIG        gcmCC('C', 'O', 'M', 'S')   /* Constant mapping start signature. */
#define COMEND_SIG          gcmCC('C', 'O', 'M', 'E')   /* Constant mapping end signature. */
#define SMMSTART_SIG        gcmCC('S', 'M', 'M', 'S')   /* Sampler mapping start signature. */
#define SMMEND_SIG          gcmCC('S', 'M', 'M', 'E')   /* Sampler mapping end signature. */
#define REMSTART_SIG        gcmCC('R', 'E', 'M', 'S')   /* Resource mapping start signature. */
#define REMEND_SIG          gcmCC('R', 'E', 'M', 'E')   /* Resource mapping end signature. */
#define UAMSTART_SIG        gcmCC('U', 'A', 'M', 'S')   /* UAV mapping start signature. */
#define UAMEND_SIG          gcmCC('U', 'A', 'M', 'E')   /* UAV mapping end signature. */
#define SPMSTART_SIG        gcmCC('S', 'P', 'M', 'S')   /* Static priv mapping start signature. */
#define SPMEND_SIG          gcmCC('S', 'P', 'M', 'E')   /* Static priv mapping end signature. */
#define DPMSTART_SIG        gcmCC('D', 'P', 'M', 'S')   /* Dynamic priv mapping start signature. */
#define DPMEND_SIG          gcmCC('D', 'P', 'M', 'E')   /* Dynamic priv mapping end signature. */
#define DUBOSTART_SIG       gcmCC('D', 'U', 'B', 'S')   /* Dynamic priv mapping start signature. */
#define DUBOEND_SIG         gcmCC('D', 'U', 'B', 'E')   /* Dynamic priv mapping end signature. */

/* PEP-related signature. */
#define PATSTART_SIG        gcmCC('P', 'A', 'T', 'S')   /* Program attribute table start signature. */
#define PATEND_SIG          gcmCC('P', 'A', 'T', 'E')   /* Program attribute table end signature. */
#define PFOSTART_SIG        gcmCC('P', 'F', 'O', 'S')   /* Program fragment output start signature. */
#define PFOEND_SIG          gcmCC('P', 'F', 'O', 'E')   /* Program fragment output end signature. */
#define GLMSTART_SIG        gcmCC('G', 'L', 'M', 'S')   /* GL high level mapping start signature. */
#define GLMEND_SIG          gcmCC('G', 'L', 'M', 'E')   /* GL high level mapping end signature. */
#define VKMSTART_SIG        gcmCC('V', 'K', 'M', 'S')   /* Vulkan high level mapping start signature. */
#define VKMEND_SIG          gcmCC('V', 'K', 'M', 'E')   /* Vulkan high level mapping end signature. */

typedef struct _VSC_EP_IO_BUFFER
{
    VSC_IO_BUFFER*      pIoBuf;
    VSC_EP_KIND         epKind;
    union
    {
        SHADER_EXECUTABLE_PROFILE*          pSEP;
        KERNEL_EXECUTABLE_PROFILE*          pKEP;
        PROGRAM_EXECUTABLE_PROFILE*         pPEP;
    } epData;
} VSC_EP_IO_BUFFER;

#define VSC_EP_ALLOC_MEM(MEM, TYPE, SIZE)         do {VSC_IO_AllocateMem(SIZE, (void**)&(MEM));  memset((MEM), 0, (SIZE));} while (0);

static void
_vscEP_Buffer_Init(
    VSC_EP_IO_BUFFER*       pEPBuf,
    VSC_IO_BUFFER*          pIoBuf,
    VSC_EP_KIND             EPKind,
    void*                   pEPData,
    void*                   pBuf,
    gctUINT                 SizeInByte,
    gctBOOL                 QueryOnly
    )
{
    pEPBuf->pIoBuf      = pIoBuf;
    pEPBuf->epKind      = EPKind;

    switch (pEPBuf->epKind)
    {
    case VSC_EP_KIND_SHADER:
        pEPBuf->epData.pSEP = (SHADER_EXECUTABLE_PROFILE*)pEPData;
        break;

    case VSC_EP_KIND_KERNEL:
        pEPBuf->epData.pKEP = (KERNEL_EXECUTABLE_PROFILE*)pEPData;
        break;

    case VSC_EP_KIND_PROGRAM:
        pEPBuf->epData.pPEP = (PROGRAM_EXECUTABLE_PROFILE*)pEPData;
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    if (QueryOnly)
    {
        pEPBuf->pIoBuf->buffer = gcvNULL;
        pEPBuf->pIoBuf->allocatedBytes = 1024;
        pEPBuf->pIoBuf->curPos = 0;
    }
    else
    {
        if (pBuf == gcvNULL)
        {
            VSC_IO_Init(pEPBuf->pIoBuf, 1024);
        }
        else
        {
            pEPBuf->pIoBuf->buffer = (gctCHAR*)pBuf;
            pEPBuf->pIoBuf->allocatedBytes = SizeInByte;
            pEPBuf->pIoBuf->curPos = 0;
        }
    }
}

static void
_vscEP_Buffer_Finalize(
    VSC_EP_IO_BUFFER*       pEPBuf
    )
{
}

/****************************************Save SEP-related functions****************************************/
static void
_vscEP_Buffer_SaveConstHwLocMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_CONSTANT_HW_LOCATION_MAPPING*pConstHwLocMapping
    );

/* Save priv mapping common entry*/
static void
_vscEP_Buffer_SavePrivMappingCommonEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_PRIV_MAPPING_COMMON_ENTRY* pPrivMappingCommonEntry
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_writeUint(pIoBuf, pPrivMappingCommonEntry->privmKind);
    VSC_IO_writeUint(pIoBuf, pPrivMappingCommonEntry->privmKindIndex);
    /* All private data is a UINT, please check _PostProcessResourceSetTables. */
    if (pPrivMappingCommonEntry->pPrivateData != gcvNULL)
    {
        VSC_IO_writeUint(pIoBuf, 1);
        VSC_IO_writeUint(pIoBuf, *(gctUINT*)pPrivMappingCommonEntry->pPrivateData);
    }
    else
    {
        VSC_IO_writeUint(pIoBuf, 0);
    }
}

/* Save IO mapping. */
static void
_vscEP_Buffer_SaveIoRegMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_IO_REG_MAPPING*  pIoRegMapping
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_writeBlock(pIoBuf, (gctCHAR*)pIoRegMapping->ioChannelMapping, sizeof(SHADER_IO_CHANNEL_MAPPING) * CHANNEL_NUM);

    VSC_IO_writeUint(pIoBuf, pIoRegMapping->ioIndex);
    VSC_IO_writeUint(pIoBuf, pIoRegMapping->ioChannelMask);
    VSC_IO_writeUint(pIoBuf, pIoRegMapping->firstValidIoChannel);
    VSC_IO_writeLong(pIoBuf, pIoRegMapping->packedIoIndexMask);
    VSC_IO_writeUint(pIoBuf, pIoRegMapping->soStreamBufferSlot);
    VSC_IO_writeUint(pIoBuf, pIoRegMapping->soSeqInStreamBuffer);
    VSC_IO_writeUint(pIoBuf, pIoRegMapping->regIoMode);
}

static void
_vscEP_Buffer_SaveIoMappingPerExeObj(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_IO_MAPPING_PER_EXE_OBJ* pPerExeObj
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, pPerExeObj->countOfIoRegMapping);
    for (i = 0; i < pPerExeObj->countOfIoRegMapping; i++)
    {
        _vscEP_Buffer_SaveIoRegMapping(pEPBuf, &pPerExeObj->pIoRegMapping[i]);
    }
    VSC_IO_writeLong(pIoBuf, pPerExeObj->ioIndexMask);
    VSC_IO_writeBlock(pIoBuf, (gctCHAR*)pPerExeObj->usage2IO, sizeof(USAGE_2_IO) * SHADER_IO_USAGE_TOTAL_COUNT);
    VSC_IO_writeLong(pIoBuf, pPerExeObj->soIoIndexMask);
    VSC_IO_writeUint(pIoBuf, (gctUINT)pPerExeObj->ioMode);
    VSC_IO_writeUint(pIoBuf, (gctUINT)pPerExeObj->ioMemAlign);
    VSC_IO_writeUint(pIoBuf, (gctUINT)pPerExeObj->ioCategory);
}

static void
_vscEP_Buffer_SaveIoMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_IO_MAPPING*      pShaderIoMapping
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_writeUint(pIoBuf, IOMSTART_SIG);

    _vscEP_Buffer_SaveIoMappingPerExeObj(pEPBuf, &pShaderIoMapping->ioVtxPxl);
    _vscEP_Buffer_SaveIoMappingPerExeObj(pEPBuf, &pShaderIoMapping->ioPrim);

    VSC_IO_writeUint(pIoBuf, IOMEND_SIG);
}

/* Save UAV mapping. */
static void
_vscEP_Buffer_SaveUavSlotMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_UAV_SLOT_MAPPING*pUavSlotMapping
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_writeUint(pIoBuf, pUavSlotMapping->uavSlotIndex);
    VSC_IO_writeUint(pIoBuf, pUavSlotMapping->accessMode);
    VSC_IO_writeUint(pIoBuf, pUavSlotMapping->hwMemAccessMode);
    VSC_IO_writeUint(pIoBuf, pUavSlotMapping->sizeInByte);

    switch (pUavSlotMapping->accessMode)
    {
    case SHADER_UAV_ACCESS_MODE_TYPE:
        VSC_IO_writeUint(pIoBuf, pUavSlotMapping->u.s.uavDimension);
        VSC_IO_writeUint(pIoBuf, pUavSlotMapping->u.s.uavType);
        break;

    case SHADER_UAV_ACCESS_MODE_RESIZABLE:
        VSC_IO_writeUint(pIoBuf, pUavSlotMapping->u.sizableEleSize);
        break;

    case SHADER_UAV_ACCESS_MODE_STRUCTURED:
        VSC_IO_writeUint(pIoBuf, pUavSlotMapping->u.structureSize);
        break;

    default:
        break;
    }

    switch (pUavSlotMapping->hwMemAccessMode)
    {
    case SHADER_HW_MEM_ACCESS_MODE_PLACE_HOLDER:
        VSC_IO_writeUint(pIoBuf, pUavSlotMapping->hwLoc.hwUavSlot);
        break;

    case SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR:
        if (pUavSlotMapping->hwLoc.pHwDirectAddrBase != gcvNULL)
        {
            VSC_IO_writeUint(pIoBuf, 1);
            _vscEP_Buffer_SaveConstHwLocMapping(pEPBuf, pUavSlotMapping->hwLoc.pHwDirectAddrBase);
        }
        else
        {
            VSC_IO_writeUint(pIoBuf, 0);
        }
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }
}

static void
_vscEP_Buffer_SaveUavMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_UAV_MAPPING*     pShaderUavMapping
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, UAMSTART_SIG);

    VSC_IO_writeUint(pIoBuf, pShaderUavMapping->countOfUAVs);
    for (i = 0; i < pShaderUavMapping->countOfUAVs; i++)
    {
        _vscEP_Buffer_SaveUavSlotMapping(pEPBuf, &pShaderUavMapping->pUAV[i]);
    }
    VSC_IO_writeUint(pIoBuf, pShaderUavMapping->uavSlotMask);
    VSC_IO_writeBlock(pIoBuf,
                      (gctCHAR*)pShaderUavMapping->dim2UavSlotMask,
                      sizeof(gctUINT) * SHADER_UAV_DIMENSION_TOTAL_COUNT);

    VSC_IO_writeUint(pIoBuf, UAMEND_SIG);
}

/* Save resource mapping. */
static void
_vscEP_Buffer_SaveResourceSlotMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_RESOURCE_SLOT_MAPPING* pResourceSlotMapping
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_writeUint(pIoBuf, pResourceSlotMapping->resourceSlotIndex);
    VSC_IO_writeUint(pIoBuf, pResourceSlotMapping->accessMode);

    switch (pResourceSlotMapping->accessMode)
    {
    case SHADER_RESOURCE_ACCESS_MODE_TYPE:
        VSC_IO_writeUint(pIoBuf, pResourceSlotMapping->u.s.resourceDimension);
        VSC_IO_writeUint(pIoBuf, pResourceSlotMapping->u.s.resourceReturnType);
        break;

    case SHADER_RESOURCE_ACCESS_MODE_STRUCTURED:
        VSC_IO_writeUint(pIoBuf, pResourceSlotMapping->u.structureSize);
        break;

    case SHADER_RESOURCE_ACCESS_MODE_RAW:
    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    VSC_IO_writeUint(pIoBuf, pResourceSlotMapping->hwResourceSlot);
}

static void
_vscEP_Buffer_SaveResourceMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_RESOURCE_MAPPING* pShaderResourceMapping
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, REMSTART_SIG);

    VSC_IO_writeUint(pIoBuf, pShaderResourceMapping->countOfResources);
    for (i = 0; i < pShaderResourceMapping->countOfResources; i++)
    {
        _vscEP_Buffer_SaveResourceSlotMapping(pEPBuf, &pShaderResourceMapping->pResource[i]);
    }
    VSC_IO_writeBlock(pIoBuf,
                      (gctCHAR*)pShaderResourceMapping->resourceSlotMask,
                      sizeof(gctUINT) * 4);
    VSC_IO_writeBlock(pIoBuf,
                      (gctCHAR*)pShaderResourceMapping->dim2ResourceSlotMask,
                      sizeof(gctUINT) * SHADER_RESOURCE_DIMENSION_TOTAL_COUNT * 4);

    VSC_IO_writeUint(pIoBuf, REMEND_SIG);
}

/* Save sampler mapping. */
static void
_vscEP_Buffer_SaveSamplerSlotMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_SAMPLER_SLOT_MAPPING* pSamplerSlotMapping
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_writeUint(pIoBuf, pSamplerSlotMapping->samplerSlotIndex);
    VSC_IO_writeUint(pIoBuf, pSamplerSlotMapping->samplerDimension);
    VSC_IO_writeUint(pIoBuf, pSamplerSlotMapping->samplerReturnType);
    VSC_IO_writeUint(pIoBuf, pSamplerSlotMapping->samplerMode);
    VSC_IO_writeUint(pIoBuf, pSamplerSlotMapping->hwSamplerSlot);
}

static void
_vscEP_Buffer_SaveSamplerMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_SAMPLER_MAPPING* pShaderSamplerMapping
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, SMMSTART_SIG);

    VSC_IO_writeUint(pIoBuf, pShaderSamplerMapping->countOfSamplers);
    for (i = 0; i < pShaderSamplerMapping->countOfSamplers; i++)
    {
        _vscEP_Buffer_SaveSamplerSlotMapping(pEPBuf, &pShaderSamplerMapping->pSampler[i]);
    }

    VSC_IO_writeUint(pIoBuf, pShaderSamplerMapping->samplerSlotMask);
    VSC_IO_writeBlock(pIoBuf, (gctCHAR*)pShaderSamplerMapping->dim2SamplerSlotMask, sizeof(gctUINT) * SHADER_RESOURCE_DIMENSION_TOTAL_COUNT);
    VSC_IO_writeUint(pIoBuf, pShaderSamplerMapping->hwSamplerRegCount);
    VSC_IO_writeInt(pIoBuf, pShaderSamplerMapping->maxHwSamplerRegIndex);

    VSC_IO_writeUint(pIoBuf, SMMEND_SIG);
}

/* Save constant mapping. */
static void
_vscEP_Buffer_SaveConstSubArrayMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_CONSTANT_SUB_ARRAY_MAPPING* pConstSubArrayMapping
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_writeUint(pIoBuf, pConstSubArrayMapping->startIdx);
    VSC_IO_writeUint(pIoBuf, pConstSubArrayMapping->subArrayRange);
    VSC_IO_writeUint(pIoBuf, pConstSubArrayMapping->firstMSCSharpRegNo);
    VSC_IO_writeUint(pIoBuf, pConstSubArrayMapping->validChannelMask);
    _vscEP_Buffer_SaveConstHwLocMapping(pEPBuf, &pConstSubArrayMapping->hwFirstConstantLocation);
}

static void
_vscEP_Buffer_SaveConstHwLocMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_CONSTANT_HW_LOCATION_MAPPING*pConstHwLocMapping
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_writeUint(pIoBuf, (gctUINT)pConstHwLocMapping->hwAccessMode);

    switch (pConstHwLocMapping->hwAccessMode)
    {
    case SHADER_HW_ACCESS_MODE_REGISTER:
        VSC_IO_writeUint(pIoBuf, pConstHwLocMapping->hwLoc.constReg.hwRegNo);
        VSC_IO_writeUint(pIoBuf, pConstHwLocMapping->hwLoc.constReg.hwRegRange);
        break;

    case SHADER_HW_ACCESS_MODE_MEMORY:
        VSC_IO_writeUint(pIoBuf, pConstHwLocMapping->hwLoc.memAddr.hwMemAccessMode);
        switch (pConstHwLocMapping->hwLoc.memAddr.hwMemAccessMode)
        {
        case SHADER_HW_MEM_ACCESS_MODE_PLACE_HOLDER:
            VSC_IO_writeUint(pIoBuf, pConstHwLocMapping->hwLoc.memAddr.memBase.hwConstantArraySlot);
            break;

        case SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR:
            if (pConstHwLocMapping->hwLoc.memAddr.memBase.pHwDirectAddrBase != gcvNULL)
            {
                VSC_IO_writeUint(pIoBuf, 1);
                _vscEP_Buffer_SaveConstHwLocMapping(pEPBuf, pConstHwLocMapping->hwLoc.memAddr.memBase.pHwDirectAddrBase);

            }
            else
            {
                VSC_IO_writeUint(pIoBuf, 0);
            }
            break;

        case SHADER_HW_MEM_ACCESS_MODE_SRV:
            if (pConstHwLocMapping->hwLoc.memAddr.memBase.pSrv != gcvNULL)
            {
                VSC_IO_writeUint(pIoBuf, 1);
                _vscEP_Buffer_SaveResourceSlotMapping(pEPBuf, pConstHwLocMapping->hwLoc.memAddr.memBase.pSrv);
            }
            else
            {
                VSC_IO_writeUint(pIoBuf, 0);
            }
            break;

        case SHADER_HW_MEM_ACCESS_MODE_UAV:
            if (pConstHwLocMapping->hwLoc.memAddr.memBase.pUav != gcvNULL)
            {
                VSC_IO_writeUint(pIoBuf, 1);
                _vscEP_Buffer_SaveUavSlotMapping(pEPBuf, pConstHwLocMapping->hwLoc.memAddr.memBase.pUav);
            }
            else
            {
                VSC_IO_writeUint(pIoBuf, 0);
            }
            break;

        default:
            gcmASSERT(gcvFALSE);
            break;
        }
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    VSC_IO_writeUint(pIoBuf, pConstHwLocMapping->hwLoc.memAddr.constantOffsetKind);
    VSC_IO_writeUint(pIoBuf, pConstHwLocMapping->hwLoc.memAddr.constantOffset);
    VSC_IO_writeUint(pIoBuf, pConstHwLocMapping->hwLoc.memAddr.componentSizeInByte);

    VSC_IO_writeUint(pIoBuf, pConstHwLocMapping->validHWChannelMask);
    VSC_IO_writeUint(pIoBuf, pConstHwLocMapping->firstValidHwChannel);
}

static void
_vscEP_Buffer_SaveCTC(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_COMPILE_TIME_CONSTANT *pCTC
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_writeBlock(pIoBuf,
                     (gctCHAR*)pCTC->constantValue,
                     sizeof(gctUINT) * CHANNEL_NUM);
    _vscEP_Buffer_SaveConstHwLocMapping(pEPBuf, &pCTC->hwConstantLocation);
}

static void
_vscEP_Buffer_SaveConstArrayMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_CONSTANT_ARRAY_MAPPING* pConstArrayMapping
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT j;

    VSC_IO_writeUint(pIoBuf, pConstArrayMapping->constantArrayIndex);
    VSC_IO_writeUint(pIoBuf, pConstArrayMapping->constantUsage);
    VSC_IO_writeUint(pIoBuf, pConstArrayMapping->arrayRange);
    VSC_IO_writeUint(pIoBuf, pConstArrayMapping->countOfSubConstantArray);

    for (j = 0; j < pConstArrayMapping->countOfSubConstantArray; j++)
    {
        _vscEP_Buffer_SaveConstSubArrayMapping(pEPBuf, &pConstArrayMapping->pSubConstantArrays[j]);
    }
}

static void
_vscEP_Buffer_SaveConstMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_CONSTANT_MAPPING*pShaderConstMapping
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, COMSTART_SIG);

    /* Save constant array mapping. */
    VSC_IO_writeUint(pIoBuf, pShaderConstMapping->countOfConstantArrayMapping);
    for (i = 0; i < pShaderConstMapping->countOfConstantArrayMapping; i++)
    {
        _vscEP_Buffer_SaveConstArrayMapping(pEPBuf, &pShaderConstMapping->pConstantArrayMapping[i]);
    }

    /* Save array index. */
    VSC_IO_writeUint(pIoBuf, pShaderConstMapping->arrayIndexMask);
    VSC_IO_writeBlock(pIoBuf, (gctCHAR*)pShaderConstMapping->usage2ArrayIndex, sizeof(gctUINT) * SHADER_CONSTANT_USAGE_TOTAL_COUNT);

    /* Save compile time constant. */
    VSC_IO_writeUint(pIoBuf, pShaderConstMapping->countOfCompileTimeConstant);
    for (i = 0; i < pShaderConstMapping->countOfCompileTimeConstant; i++)
    {
        _vscEP_Buffer_SaveCTC(pEPBuf, &pShaderConstMapping->pCompileTimeConstant[i]);
    }

    /* Save hw reg index. */
    VSC_IO_writeUint(pIoBuf, pShaderConstMapping->hwConstRegCount);
    VSC_IO_writeInt(pIoBuf, pShaderConstMapping->maxHwConstRegIndex);

    VSC_IO_writeUint(pIoBuf, COMEND_SIG);
}

/* Save priv constant entry. */
static void
_vscEP_Buffer_SavePrivConstEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_PRIV_CONSTANT_ENTRY* pPrivConstEntry
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    /* Save SHADER_PRIV_MAPPING_COMMON_ENTRY. */
   _vscEP_Buffer_SavePrivMappingCommonEntry(pEPBuf, &pPrivConstEntry->commonPrivm);

    /* Save const node. */
    VSC_IO_writeUint(pIoBuf, (gctUINT)pPrivConstEntry->mode);
    switch (pPrivConstEntry->mode)
    {
    case SHADER_PRIV_CONSTANT_MODE_CTC:
        _vscEP_Buffer_SaveCTC(pEPBuf, pPrivConstEntry->u.ctcConstant.pCTC);
        VSC_IO_writeUint(pIoBuf, pPrivConstEntry->u.ctcConstant.hwChannelMask);
        break;

    case SHADER_PRIV_CONSTANT_MODE_VAL_2_INST:
        VSC_IO_writeUint(pIoBuf, pPrivConstEntry->u.instImm.patchedPC);
        VSC_IO_writeUint(pIoBuf, pPrivConstEntry->u.instImm.srcNo);
        break;

    case SHADER_PRIV_CONSTANT_MODE_VAL_2_MEMORREG:
        if (pPrivConstEntry->u.pSubCBMapping != gcvNULL)
        {
            VSC_IO_writeUint(pIoBuf, 1);
            _vscEP_Buffer_SaveConstSubArrayMapping(pEPBuf, pPrivConstEntry->u.pSubCBMapping);
        }
        else
        {
            VSC_IO_writeUint(pIoBuf, 0);
        }
        break;

    case SHADER_PRIV_CONSTANT_MODE_VAL_2_DUBO:
        VSC_IO_writeUint(pIoBuf, pPrivConstEntry->u.duboEntryIndex);
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }
}

/* Save priv constant mapping. */
static void
_vscEP_Buffer_SavePrivConstMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_PRIV_CONSTANT_MAPPING* pPrivConstMapping
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, pPrivConstMapping->countOfEntries);

    /* Save SHADER_PRIV_CONSTANT_ENTRY. */
    for (i = 0; i < pPrivConstMapping->countOfEntries; i++)
    {
        _vscEP_Buffer_SavePrivConstEntry(pEPBuf, &pPrivConstMapping->pPrivmConstantEntries[i]);
    }
}

/* Save priv mem data mapping. */
static void
_vscEP_Buffer_SavePrivMemDataMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_PRIV_MEM_DATA_MAPPING*   pPrivMemDataMapping
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, pPrivMemDataMapping->ctcCount);
    for (i = 0; i < pPrivMemDataMapping->ctcCount; i++)
    {
        _vscEP_Buffer_SaveCTC(pEPBuf, pPrivMemDataMapping->ppCTC[i]);
    }

    VSC_IO_writeUint(pIoBuf, pPrivMemDataMapping->cnstSubArrayCount);
    for (i = 0; i < pPrivMemDataMapping->cnstSubArrayCount; i++)
    {
        _vscEP_Buffer_SaveConstSubArrayMapping(pEPBuf, pPrivMemDataMapping->ppCnstSubArray[i]);
    }
}

/* Save priv UAV entry. */
static void
_vscEP_Buffer_SavePrivUavEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_PRIV_UAV_ENTRY*  pPrivUavEntry
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_writeUint(pIoBuf, pPrivUavEntry->uavEntryIndex);

    /* Save SHADER_PRIV_MAPPING_COMMON_ENTRY. */
    _vscEP_Buffer_SavePrivMappingCommonEntry(pEPBuf, &pPrivUavEntry->commonPrivm);

    /* Save SHADER_PRIV_MEM_DATA_MAPPING. */
    _vscEP_Buffer_SavePrivMemDataMapping(pEPBuf, &pPrivUavEntry->memData);

    /* Save SHADER_UAV_SLOT_MAPPING. */
    if (pPrivUavEntry->pBuffer != gcvNULL)
    {
        VSC_IO_writeUint(pIoBuf, 1);
        _vscEP_Buffer_SaveUavSlotMapping(pEPBuf, pPrivUavEntry->pBuffer);
    }
    else
    {
        VSC_IO_writeUint(pIoBuf, 0);
    }
}

/* Save priv UAV mapping. */
static void
_vscEP_Buffer_SavePrivUavMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_PRIV_UAV_MAPPING*pPrivUavMapping
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, pPrivUavMapping->countOfEntries);

    /* Save SHADER_PRIV_UAV_ENTRY. */
    for (i = 0; i < pPrivUavMapping->countOfEntries; i++)
    {
        _vscEP_Buffer_SavePrivUavEntry(pEPBuf, &pPrivUavMapping->pPrivUavEntries[i]);
    }
}

/* Save static priv mapping. */
static void
_vscEP_Buffer_SaveStaticPrivMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_STATIC_PRIV_MAPPING* pStaticPrivMapping
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_writeUint(pIoBuf, SPMSTART_SIG);

    /* Save private constant mapping. */
    _vscEP_Buffer_SavePrivConstMapping(pEPBuf, &pStaticPrivMapping->privConstantMapping);

    /* Save private uav mapping. */
    _vscEP_Buffer_SavePrivUavMapping(pEPBuf, &pStaticPrivMapping->privUavMapping);

    VSC_IO_writeUint(pIoBuf, SPMEND_SIG);
}

/* Save priv sampler entry. */
static void
_vscEP_Buffer_SavePrivSamplerEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_PRIV_SAMPLER_ENTRY*pPrivSamplerEntry
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    _vscEP_Buffer_SavePrivMappingCommonEntry(pEPBuf, &pPrivSamplerEntry->commonPrivm);

    if (pPrivSamplerEntry->pSampler != gcvNULL)
    {
        VSC_IO_writeUint(pIoBuf, 1);
        _vscEP_Buffer_SaveSamplerSlotMapping(pEPBuf, pPrivSamplerEntry->pSampler);
    }
    else
    {
        VSC_IO_writeUint(pIoBuf, 0);
    }
}

/* Save priv sampler mapping. */
static void
_vscEP_Buffer_SavePrivSamplerMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_PRIV_SAMPLER_MAPPING*pPrivSamplerMapping
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, pPrivSamplerMapping->countOfEntries);

    for (i = 0; i < pPrivSamplerMapping->countOfEntries; i++)
    {
        _vscEP_Buffer_SavePrivSamplerEntry(pEPBuf, &pPrivSamplerMapping->pPrivSamplerEntries[i]);
    }
}

/* Save priv output entry. */
static void
_vscEP_Buffer_SavePrivOutputEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_PRIV_OUTPUT_ENTRY*pPrivOutputEntry
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    _vscEP_Buffer_SavePrivMappingCommonEntry(pEPBuf, &pPrivOutputEntry->commonPrivm);

    if (pPrivOutputEntry->pOutput != gcvNULL)
    {
        VSC_IO_writeUint(pIoBuf, 1);
        _vscEP_Buffer_SaveIoRegMapping(pEPBuf, pPrivOutputEntry->pOutput);
    }
    else
    {
        VSC_IO_writeUint(pIoBuf, 0);
    }
}

/* Save priv output mapping. */
static void
_vscEP_Buffer_SavePrivOutputMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_PRIV_OUTPUT_MAPPING*pPrivOutputMapping
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, pPrivOutputMapping->countOfEntries);

    for (i = 0; i < pPrivOutputMapping->countOfEntries; i++)
    {
        _vscEP_Buffer_SavePrivOutputEntry(pEPBuf, &pPrivOutputMapping->pPrivOutputEntries[i]);
    }
}

/* Save dynamic priv mapping. */
static void
_vscEP_Buffer_SaveDynamicPrivMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_DYNAMIC_PRIV_MAPPING* pDynamicPrivMapping
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_writeUint(pIoBuf, DPMSTART_SIG);

    /* Save priv sampler mapping. */
    _vscEP_Buffer_SavePrivSamplerMapping(pEPBuf, &pDynamicPrivMapping->privSamplerMapping);

    /* Save priv output mapping. */
    _vscEP_Buffer_SavePrivOutputMapping(pEPBuf, &pDynamicPrivMapping->privOutputMapping);

    VSC_IO_writeUint(pIoBuf, DPMEND_SIG);
}

/* Save default UBO mapping. */
static void
_vscEP_Buffer_SaveDefaultUboMeberEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_DEFAULT_UBO_MEMBER_ENTRY* pDefaultUboMemberEntry
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_writeUint(pIoBuf, pDefaultUboMemberEntry->memberIndexInOtherEntryTable);
    VSC_IO_writeUint(pIoBuf, (gctUINT)pDefaultUboMemberEntry->memberKind);
    VSC_IO_writeUint(pIoBuf, pDefaultUboMemberEntry->offsetInByte);
}

static void
_vscEP_Buffer_SaveDefaultUboMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_DEFAULT_UBO_MAPPING* pDefaultUboMapping
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, DUBOSTART_SIG);

    VSC_IO_writeUint(pIoBuf, pDefaultUboMapping->baseAddressIndexInPrivConstTable);
    VSC_IO_writeUint(pIoBuf, pDefaultUboMapping->countOfEntries);
    VSC_IO_writeUint(pIoBuf, pDefaultUboMapping->sizeInByte);

    for (i = 0; i < pDefaultUboMapping->countOfEntries; i++)
    {
        _vscEP_Buffer_SaveDefaultUboMeberEntry(pEPBuf, &pDefaultUboMapping->pDefaultUboMemberEntries[i]);
    }

    VSC_IO_writeUint(pIoBuf, DUBOEND_SIG);
}

/* Save the image derived information. */
static void
_vscEP_Buffer_SaveImageDerivedInfo(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_IMAGE_DERIVED_INFO* pImageDerivedInfo
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    /* Save the image size. */
    if (pImageDerivedInfo->pImageSize)
    {
        VSC_IO_writeUint(pIoBuf, 1);
        _vscEP_Buffer_SavePrivConstEntry(pEPBuf, pImageDerivedInfo->pImageSize);
    }
    else
    {
        VSC_IO_writeUint(pIoBuf, 0);
    }

    /* Save the extra layer image. */
    if (pImageDerivedInfo->pExtraLayer)
    {
        VSC_IO_writeUint(pIoBuf, 1);
        _vscEP_Buffer_SavePrivUavEntry(pEPBuf, pImageDerivedInfo->pExtraLayer);
    }
    else
    {
        VSC_IO_writeUint(pIoBuf, 0);
    }

    /* Save the image format. */
    VSC_IO_writeUint(pIoBuf, (gctUINT)pImageDerivedInfo->imageFormatInfo.imageFormat);
    VSC_IO_writeUint(pIoBuf, (gctUINT)pImageDerivedInfo->imageFormatInfo.bSetInSpriv);
}

/****************************************Save PEP-related functions****************************************/
/****************************************GL-related functions****************************************/
/* Save attribute table entry. */
static void
_vscEP_Buffer_SaveProgAttrEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_ATTRIBUTE_TABLE_ENTRY* pProgAttrEntry
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_writeUint(pIoBuf, pProgAttrEntry->type);
    VSC_IO_writeUint(pIoBuf, pProgAttrEntry->resEntryBit);
    VSC_IO_writeUint(pIoBuf, pProgAttrEntry->nameLength);
    VSC_IO_writeBlock(pIoBuf, (gctCHAR*)pProgAttrEntry->name, pProgAttrEntry->nameLength + 1);
    VSC_IO_writeUint(pIoBuf, (gctUINT)pProgAttrEntry->arraySize);
    VSC_IO_writeUint(pIoBuf, pProgAttrEntry->attribEntryIndex);

    if (pProgAttrEntry->pIoRegMapping != gcvNULL)
    {
        VSC_IO_writeUint(pIoBuf, 1);
        _vscEP_Buffer_SaveIoRegMapping(pEPBuf, pProgAttrEntry->pIoRegMapping);
    }
    else
    {
        VSC_IO_writeUint(pIoBuf, 0);
    }

    VSC_IO_writeUint(pIoBuf, pProgAttrEntry->locationCount);
    if (pProgAttrEntry->locationCount != 0)
    {
        VSC_IO_writeBlock(pIoBuf, (gctCHAR*)pProgAttrEntry->pLocation, sizeof(gctUINT) * pProgAttrEntry->locationCount);
    }

    VSC_IO_writeUint(pIoBuf, pProgAttrEntry->vec4BasedCount);
    VSC_IO_writeUint(pIoBuf, pProgAttrEntry->activeVec4Mask);
}

/* Save attribute table. */
static void
_vscEP_Buffer_SaveProgAttrTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_ATTRIBUTE_TABLE*   pProgAttrTable
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, PATSTART_SIG);

    VSC_IO_writeUint(pIoBuf, pProgAttrTable->countOfEntries);
    for (i = 0; i < pProgAttrTable->countOfEntries; i++)
    {
        _vscEP_Buffer_SaveProgAttrEntry(pEPBuf, &pProgAttrTable->pAttribEntries[i]);
    }
    VSC_IO_writeUint(pIoBuf, pProgAttrTable->maxLengthOfName);

    VSC_IO_writeUint(pIoBuf, PATEND_SIG);
}

/* Save fragment output table entry. */
static void
_vscEP_Buffer_SaveFragOutEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_FRAGOUT_TABLE_ENTRY* pFragOutEntry
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_writeUint(pIoBuf, pFragOutEntry->type);
    VSC_IO_writeUint(pIoBuf, pFragOutEntry->resEntryBit);
    VSC_IO_writeUint(pIoBuf, pFragOutEntry->nameLength);
    VSC_IO_writeBlock(pIoBuf, (gctCHAR*)pFragOutEntry->name, pFragOutEntry->nameLength + 1);
    VSC_IO_writeUint(pIoBuf, pFragOutEntry->usage);
    VSC_IO_writeUint(pIoBuf, pFragOutEntry->fragOutEntryIndex);

    if (pFragOutEntry->pIoRegMapping != gcvNULL)
    {
        VSC_IO_writeUint(pIoBuf, 1);
        _vscEP_Buffer_SaveIoRegMapping(pEPBuf, pFragOutEntry->pIoRegMapping);
    }
    else
    {
        VSC_IO_writeUint(pIoBuf, 0);
    }

    VSC_IO_writeUint(pIoBuf, pFragOutEntry->locationCount);
    if (pFragOutEntry->locationCount != 0)
    {
        VSC_IO_writeBlock(pIoBuf, (gctCHAR*)pFragOutEntry->pLocation, sizeof(gctUINT) * pFragOutEntry->locationCount);
    }

    VSC_IO_writeUint(pIoBuf, pFragOutEntry->vec4BasedCount);
    VSC_IO_writeUint(pIoBuf, pFragOutEntry->activeVec4Mask);
}

/* Save fragment output table. */
static void
_vscEP_Buffer_SaveFragOutTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_FRAGOUT_TABLE*     pFragOutTable
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, PFOSTART_SIG);

    VSC_IO_writeUint(pIoBuf, pFragOutTable->countOfEntries);
    for (i = 0; i < pFragOutTable->countOfEntries; i++)
    {
        _vscEP_Buffer_SaveFragOutEntry(pEPBuf, &pFragOutTable->pFragOutEntries[i]);
    }

    VSC_IO_writeUint(pIoBuf, PFOEND_SIG);
}

/* Save GL uniform HW sub mapping. */
static void
_vscEP_Buffer_SaveGLUniformHwSubMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_GL_UNIFORM_HW_SUB_MAPPING* pGLUniformHwSubMapping
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_writeUint(pIoBuf, pGLUniformHwSubMapping->startIdx);
    VSC_IO_writeUint(pIoBuf, pGLUniformHwSubMapping->rangeOfSubArray);
    VSC_IO_writeUint(pIoBuf, pGLUniformHwSubMapping->validChannelMask);
    VSC_IO_writeUint(pIoBuf, pGLUniformHwSubMapping->hwSubMappingMode);

    switch (pGLUniformHwSubMapping->hwSubMappingMode)
    {
    case GL_UNIFORM_HW_SUB_MAPPING_MODE_CONSTANT:
        if (pGLUniformHwSubMapping->hwMapping.pSubCBMapping != gcvNULL)
        {
            VSC_IO_writeUint(pIoBuf, 1);
            _vscEP_Buffer_SaveConstSubArrayMapping(pEPBuf, pGLUniformHwSubMapping->hwMapping.pSubCBMapping);
        }
        else
        {
            VSC_IO_writeUint(pIoBuf, 0);
        }
        break;

    case GL_UNIFORM_HW_SUB_MAPPING_MODE_SAMPLER:
        if (pGLUniformHwSubMapping->hwMapping.pSamplerMapping != gcvNULL)
        {
            VSC_IO_writeUint(pIoBuf, 1);
            _vscEP_Buffer_SaveSamplerSlotMapping(pEPBuf, pGLUniformHwSubMapping->hwMapping.pSamplerMapping);
        }
        else
        {
            VSC_IO_writeUint(pIoBuf, 0);
        }
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }
}

/* Save GL uniform HW mapping. */
static void
_vscEP_Buffer_SaveGLUniformHwMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_GL_UNIFORM_HW_MAPPING* pGLUniformHwMapping
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, pGLUniformHwMapping->countOfHWSubMappings);

    for (i = 0; i < pGLUniformHwMapping->countOfHWSubMappings; i++)
    {
        _vscEP_Buffer_SaveGLUniformHwSubMapping(pEPBuf, &pGLUniformHwMapping->pHWSubMappings[i]);
    }
}

/* Save GL uniform common entry. */
static void
_vscEP_Buffer_SaveGLUniformCommonEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_GL_UNIFORM_COMMON_ENTRY* pGLUniformCommonEntry
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, pGLUniformCommonEntry->type);
    VSC_IO_writeUint(pIoBuf, pGLUniformCommonEntry->nameLength);
    VSC_IO_writeBlock(pIoBuf, (gctCHAR*)pGLUniformCommonEntry->name, pGLUniformCommonEntry->nameLength + 1);
    VSC_IO_writeUint(pIoBuf, pGLUniformCommonEntry->precision);
    VSC_IO_writeUint(pIoBuf, pGLUniformCommonEntry->arraySize);
    VSC_IO_writeUint(pIoBuf, pGLUniformCommonEntry->activeArraySize);
    VSC_IO_writeUint(pIoBuf, pGLUniformCommonEntry->uniformEntryIndex);
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        _vscEP_Buffer_SaveGLUniformHwMapping(pEPBuf, &pGLUniformCommonEntry->hwMappings[i]);
    }
}

/* Save GL uniform table entry. */
static void
_vscEP_Buffer_SaveGLUniformTableEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_GL_UNIFORM_TABLE_ENTRY* pGLUniformTableEntry
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    _vscEP_Buffer_SaveGLUniformCommonEntry(pEPBuf, &pGLUniformTableEntry->common);
    VSC_IO_writeUint(pIoBuf, pGLUniformTableEntry->usage);
    VSC_IO_writeUint(pIoBuf, pGLUniformTableEntry->locationBase);
}

/* Save GL uniform table entry. */
static void
_vscEP_Buffer_SaveGLUniformTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_GL_UNIFORM_TABLE*  pGLUniformTable
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, pGLUniformTable->countOfEntries);

    for (i = 0; i < pGLUniformTable->countOfEntries; i++)
    {
        _vscEP_Buffer_SaveGLUniformTableEntry(pEPBuf, &pGLUniformTable->pUniformEntries[i]);
    }

    VSC_IO_writeUint(pIoBuf, pGLUniformTable->firstBuiltinUniformEntryIndex);
    VSC_IO_writeUint(pIoBuf, pGLUniformTable->firstUserUniformEntryIndex);
    VSC_IO_writeUint(pIoBuf, pGLUniformTable->maxLengthOfName);
}

/* Save GL uniformblock uniform entry. */
static void
_vscEP_Buffer_SaveGLUniformBlockUniformEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_GL_UNIFORMBLOCK_UNIFORM_ENTRY* pGLUniformBlockUniformEntry
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    _vscEP_Buffer_SaveGLUniformCommonEntry(pEPBuf, &pGLUniformBlockUniformEntry->common);
    VSC_IO_writeUint(pIoBuf, (gctUINT)pGLUniformBlockUniformEntry->offset);
    VSC_IO_writeUint(pIoBuf, (gctUINT)pGLUniformBlockUniformEntry->arrayStride);
    VSC_IO_writeUint(pIoBuf, (gctUINT)pGLUniformBlockUniformEntry->matrixMajor);
    VSC_IO_writeUint(pIoBuf, (gctUINT)pGLUniformBlockUniformEntry->matrixMajor);
}

/* Save GL uniformblock entry. */
static void
_vscEP_Buffer_SaveGLUniformBlockEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_GL_UNIFORMBLOCK_TABLE_ENTRY* pGLUniformBlockEntry
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, pGLUniformBlockEntry->countOfEntries);

    for (i = 0; i < pGLUniformBlockEntry->countOfEntries; i++)
    {
        _vscEP_Buffer_SaveGLUniformBlockUniformEntry(pEPBuf, &pGLUniformBlockEntry->pUniformEntries[i]);
    }

    VSC_IO_writeBlock(pIoBuf, (gctCHAR*)pGLUniformBlockEntry->pUniformIndices, sizeof(gctUINT));
    VSC_IO_writeUint(pIoBuf, pGLUniformBlockEntry->ubEntryIndex);
    VSC_IO_writeUint(pIoBuf, pGLUniformBlockEntry->dataSizeInByte);
    VSC_IO_writeBlock(pIoBuf, (gctCHAR*)pGLUniformBlockEntry->bRefedByShader, sizeof(gctBOOL) * VSC_MAX_SHADER_STAGE_COUNT);
}

/* Save GL uniformblock table. */
static void
_vscEP_Buffer_SaveGLUniformBlockTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_GL_UNIFORMBLOCK_TABLE* pGLUniformBlockTable
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, pGLUniformBlockTable->countOfEntries);

    for (i = 0; i < pGLUniformBlockTable->countOfEntries; i++)
    {
        _vscEP_Buffer_SaveGLUniformBlockEntry(pEPBuf, &pGLUniformBlockTable->pUniformBlockEntries[i]);
    }

    VSC_IO_writeUint(pIoBuf, pGLUniformBlockTable->maxLengthOfName);
}

/* Save XFB out table entry. */
static void
_vscEP_Buffer_SaveXfbOutEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_GL_XFB_OUT_TABLE_ENTRY* pXfbOutEntry
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_writeUint(pIoBuf, pXfbOutEntry->type);
    VSC_IO_writeUint(pIoBuf, pXfbOutEntry->nameLength);
    VSC_IO_writeBlock(pIoBuf, (gctCHAR*)pXfbOutEntry->name, pXfbOutEntry->nameLength + 1);
    VSC_IO_writeUint(pIoBuf, pXfbOutEntry->arraySize);
    VSC_IO_writeUint(pIoBuf, pXfbOutEntry->fxOutEntryIndex);
    VSC_IO_writeUint(pIoBuf, pXfbOutEntry->fxStreamingMode);

    switch (pXfbOutEntry->fxStreamingMode)
    {
    case GL_FX_STREAMING_MODE_FFU:
        if (pXfbOutEntry->u.s.pIoRegMapping != gcvNULL)
        {
            VSC_IO_writeUint(pIoBuf, 1);
            _vscEP_Buffer_SaveIoRegMapping(pEPBuf, pXfbOutEntry->u.s.pIoRegMapping);
        }
        else
        {
            VSC_IO_writeUint(pIoBuf, 0);
        }
        VSC_IO_writeUint(pIoBuf, pXfbOutEntry->u.s.vec4BasedCount);
        VSC_IO_writeUint(pIoBuf, pXfbOutEntry->u.s.activeVec4Mask);
        break;

    case GL_FX_STREAMING_MODE_SHADER:
        if (pXfbOutEntry->u.pSoBufferMapping != gcvNULL)
        {
            VSC_IO_writeUint(pIoBuf, 1);
            _vscEP_Buffer_SaveUavSlotMapping(pEPBuf, pXfbOutEntry->u.pSoBufferMapping);
        }
        else
        {
            VSC_IO_writeUint(pIoBuf, 0);
        }
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }
}

/* Save XFB table. */
static void
_vscEP_Buffer_SaveXfbOutTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_GL_XFB_OUT_TABLE*  pXfbOutTable
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, pXfbOutTable->countOfEntries);

    for (i = 0; i < pXfbOutTable->countOfEntries; i++)
    {
        _vscEP_Buffer_SaveXfbOutEntry(pEPBuf, &pXfbOutTable->pFxOutEntries[i]);
    }

    VSC_IO_writeUint(pIoBuf, pXfbOutTable->mode);
    VSC_IO_writeUint(pIoBuf, pXfbOutTable->maxLengthOfName);
}

/* Save GL high level mapping table. */
static void
_vscEP_Buffer_SaveGLHighLevelMappingTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROGRAM_EXECUTABLE_PROFILE* pPEP
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    /* Start signature. */
    VSC_IO_writeUint(pIoBuf, GLMSTART_SIG);

    /* Save GL uniform table. */
    _vscEP_Buffer_SaveGLUniformTable(pEPBuf, &pPEP->u.gl.uniformTable);

    /* Save GL uniformblock table. */
    _vscEP_Buffer_SaveGLUniformBlockTable(pEPBuf, &pPEP->u.gl.uniformBlockTable);

    /* Save GL XFB out table. */
    _vscEP_Buffer_SaveXfbOutTable(pEPBuf, &pPEP->u.gl.fxOutTable);

    /* End signature. */
    VSC_IO_writeUint(pIoBuf, GLMEND_SIG);
}

/****************************************Vulkan-related functions****************************************/
/* Save resource binding. */
static void
_vscEP_Buffer_SaveShaderResourceBinding(
    VSC_EP_IO_BUFFER*       pEPBuf,
    VSC_SHADER_RESOURCE_BINDING* pShaderResourceBinding
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_writeUint(pIoBuf, pShaderResourceBinding->type);
    VSC_IO_writeUint(pIoBuf, pShaderResourceBinding->set);
    VSC_IO_writeUint(pIoBuf, pShaderResourceBinding->binding);
    VSC_IO_writeUint(pIoBuf, pShaderResourceBinding->arraySize);
}

/* Save private combined texture sampler hw mapping list. */
static void
_vscEP_Buffer_SaveVKPrivCombTexSampHwMappingList(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_PRIV_COMB_TEX_SAMP_HW_MAPPING_LIST* pPrivCombTexSampHwMappingList
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_writeUint(pIoBuf, pPrivCombTexSampHwMappingList->arraySize);

    if (pPrivCombTexSampHwMappingList->arraySize != 0)
    {
        VSC_IO_writeBlock(pIoBuf,
                          (gctCHAR*)pPrivCombTexSampHwMappingList->pPctsHmEntryIdxArray,
                          sizeof(gctUINT) * pPrivCombTexSampHwMappingList->arraySize);
    }
}

/* Save private combined texture sampler hw mapping list. */
static void
_vscEP_Buffer_SaveVkCombTexSampHwMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_COMBINED_TEXTURE_SAMPLER_HW_MAPPING* pCombTsHwMapping,
    gctUINT                 ArraySize
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    _vscEP_Buffer_SaveSamplerSlotMapping(pEPBuf, &pCombTsHwMapping->samplerMapping);

    if (pCombTsHwMapping->ppExtraSamplerArray != gcvNULL)
    {
        VSC_IO_writeUint(pIoBuf, 1);
        for (i = 0; i < ArraySize; i++)
        {
            _vscEP_Buffer_SavePrivSamplerEntry(pEPBuf, pCombTsHwMapping->ppExtraSamplerArray[i]);
        }
    }
    else
    {
        VSC_IO_writeUint(pIoBuf, 0);
    }

    for (i = 0; i < __YCBCR_PLANE_COUNT__; i++)
    {
        if (pCombTsHwMapping->pYcbcrPlanes[i] != gcvNULL)
        {
            VSC_IO_writeUint(pIoBuf, 1);
            VSC_IO_writeUint(pIoBuf, pCombTsHwMapping->pYcbcrPlanes[i]->uavEntryIndex);
        }
        else
        {
            VSC_IO_writeUint(pIoBuf, 0);
        }
    }

    _vscEP_Buffer_SaveResourceSlotMapping(pEPBuf, &pCombTsHwMapping->texMapping);
    _vscEP_Buffer_SaveVKPrivCombTexSampHwMappingList(pEPBuf, &pCombTsHwMapping->samplerHwMappingList);
    _vscEP_Buffer_SaveVKPrivCombTexSampHwMappingList(pEPBuf, &pCombTsHwMapping->texHwMappingList);
}

/* Save Vulkan combined texture-sampler entry. */
static void
_vscEP_Buffer_SaveVKCombinedTextureSamplerEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_COMBINED_TEX_SAMPLER_TABLE_ENTRY* pCombTsEntry
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i, j;
    gctUINT entryMask = 0;

    _vscEP_Buffer_SaveShaderResourceBinding(pEPBuf, &pCombTsEntry->combTsBinding);
    VSC_IO_writeUint(pIoBuf, pCombTsEntry->combTsEntryIndex);
    VSC_IO_writeUint(pIoBuf, pCombTsEntry->stageBits);
    VSC_IO_writeUint(pIoBuf, pCombTsEntry->activeStageMask);

    /* Save texture size. */
    entryMask = 0;
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (pCombTsEntry->pTextureSize[i][j])
            {
                entryMask |= (1 << (i * 2 + j));
            }
        }
    }
    VSC_IO_writeUint(pIoBuf, entryMask);
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (pCombTsEntry->pTextureSize[i][j])
            {
                _vscEP_Buffer_SavePrivConstEntry(pEPBuf, pCombTsEntry->pTextureSize[i][j]);
            }
        }
    }

    /* Save lodMinMax */
    entryMask = 0;
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (pCombTsEntry->pLodMinMax[i][j])
            {
                entryMask |= (1 << (i * 2 + j));
            }
        }
    }
    VSC_IO_writeUint(pIoBuf, entryMask);
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (pCombTsEntry->pLodMinMax[i][j])
            {
                _vscEP_Buffer_SavePrivConstEntry(pEPBuf, pCombTsEntry->pLodMinMax[i][j]);
            }
        }
    }

    /* Save levels samples. */
    entryMask = 0;
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (pCombTsEntry->pLevelsSamples[i][j])
            {
                entryMask |= (1 << (i * 2 + j));
            }
        }
    }
    VSC_IO_writeUint(pIoBuf, entryMask);
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (pCombTsEntry->pLevelsSamples[i][j])
            {
                _vscEP_Buffer_SavePrivConstEntry(pEPBuf, pCombTsEntry->pLevelsSamples[i][j]);
            }
        }
    }

    if (pCombTsEntry->combTsBinding.arraySize != 0)
    {
        VSC_IO_writeBlock(pIoBuf, (gctCHAR*)pCombTsEntry->pResOpBits, sizeof(gctUINT) * pCombTsEntry->combTsBinding.arraySize);
    }

    /* Save hw mapping. */
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        _vscEP_Buffer_SaveVkCombTexSampHwMapping(pEPBuf, &pCombTsEntry->hwMappings[i], pCombTsEntry->combTsBinding.arraySize);
    }

    /* Save the sampled image index. */
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        VSC_IO_writeUint(pIoBuf, pCombTsEntry->sampledImageIndexInStorageTable[i]);
    }
}

/* Save Vulkan combined texture-sampler table. */
static void
_vscEP_Buffer_SaveVKCombinedTextureSamplerTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_COMBINED_TEXTURE_SAMPLER_TABLE* pCombinedSampTexTable
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, pCombinedSampTexTable->countOfEntries);

    for (i = 0; i < pCombinedSampTexTable->countOfEntries; i++)
    {
        _vscEP_Buffer_SaveVKCombinedTextureSamplerEntry(pEPBuf, &pCombinedSampTexTable->pCombTsEntries[i]);
    }
}

/* Save Vulkan separated sampler HW mapping. */
static void
_vscEP_Buffer_SaveVKSeparatedSamplerHwMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_SEPARATED_SAMPLER_HW_MAPPING* pSeparatedSamplerHwMapping
    )
{
    _vscEP_Buffer_SaveVKPrivCombTexSampHwMappingList(pEPBuf, &pSeparatedSamplerHwMapping->samplerHwMappingList);
}

/* Save Vulkan separated sampler table entry. */
static void
_vscEP_Buffer_SaveVKSeparatedSamplerEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_SEPARATED_SAMPLER_TABLE_ENTRY* pSeparatedSamplerEntry
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    _vscEP_Buffer_SaveShaderResourceBinding(pEPBuf, &pSeparatedSamplerEntry->samplerBinding);

    VSC_IO_writeUint(pIoBuf, pSeparatedSamplerEntry->samplerEntryIndex);
    VSC_IO_writeUint(pIoBuf, pSeparatedSamplerEntry->stageBits);
    VSC_IO_writeUint(pIoBuf, pSeparatedSamplerEntry->activeStageMask);

    if (pSeparatedSamplerEntry->samplerBinding.arraySize != 0)
    {
        VSC_IO_writeBlock(pIoBuf,
                         (gctCHAR*)pSeparatedSamplerEntry->pResOpBits,
                         sizeof(gctUINT) * pSeparatedSamplerEntry->samplerBinding.arraySize);
    }

    VSC_IO_writeUint(pIoBuf, pSeparatedSamplerEntry->bUsingHwMppingList);

    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        _vscEP_Buffer_SaveVKSeparatedSamplerHwMapping(pEPBuf, &pSeparatedSamplerEntry->hwMappings[i]);
    }
}

/* Save Vulkan separated sampler table. */
static void
_vscEP_Buffer_SaveVKSeparatedSamplerTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_SEPARATED_SAMPLER_TABLE* pSeparatedSamplerTable
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, pSeparatedSamplerTable->countOfEntries);

    for (i = 0; i < pSeparatedSamplerTable->countOfEntries; i++)
    {
        _vscEP_Buffer_SaveVKSeparatedSamplerEntry(pEPBuf, &pSeparatedSamplerTable->pSamplerEntries[i]);
    }
}

/* Save Vulkan separated texture HW mapping. */
static void
_vscEP_Buffer_SaveVKSeparatedTextureHwMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_SEPARATED_TEXTURE_HW_MAPPING* pSeparatedTextureHwMapping
    )
{
    _vscEP_Buffer_SaveImageDerivedInfo(pEPBuf, &pSeparatedTextureHwMapping->s.imageDerivedInfo);
    _vscEP_Buffer_SaveUavSlotMapping(pEPBuf, &pSeparatedTextureHwMapping->s.hwMapping);
    _vscEP_Buffer_SaveVKPrivCombTexSampHwMappingList(pEPBuf, &pSeparatedTextureHwMapping->s.texHwMappingList);
}

/* Save Vulkan separated texture table entry. */
static void
_vscEP_Buffer_SaveVKSeparatedTextureEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_SEPARATED_TEXTURE_TABLE_ENTRY* pSeparatedTextureEntry
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    _vscEP_Buffer_SaveShaderResourceBinding(pEPBuf, &pSeparatedTextureEntry->texBinding);

    VSC_IO_writeUint(pIoBuf, pSeparatedTextureEntry->textureEntryIndex);
    VSC_IO_writeUint(pIoBuf, pSeparatedTextureEntry->stageBits);
    VSC_IO_writeUint(pIoBuf, pSeparatedTextureEntry->activeStageMask);

    if (pSeparatedTextureEntry->texBinding.arraySize != 0)
    {
        VSC_IO_writeBlock(pIoBuf,
                          (gctCHAR*)pSeparatedTextureEntry->pResOpBits,
                          sizeof(gctUINT) * pSeparatedTextureEntry->texBinding.arraySize);
    }

    VSC_IO_writeUint(pIoBuf, pSeparatedTextureEntry->bUsingHwMppingList);

    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        _vscEP_Buffer_SaveVKSeparatedTextureHwMapping(pEPBuf, &pSeparatedTextureEntry->hwMappings[i]);
    }
}

/* Save Vulkan separated texture table. */
static void
_vscEP_Buffer_SaveVKSeparatedTextureTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_SEPARATED_TEXTURE_TABLE* pSeparatedTextureTable
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, pSeparatedTextureTable->countOfEntries);

    for (i = 0; i < pSeparatedTextureTable->countOfEntries; i++)
    {
        _vscEP_Buffer_SaveVKSeparatedTextureEntry(pEPBuf, &pSeparatedTextureTable->pTextureEntries[i]);
    }
}

/* Save Vulkan uniform texel buffer hw mapping. */
static void
_vscEP_Buffer_SaveVKUniformTexelBufferHwMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_UNIFORM_TEXEL_BUFFER_HW_MAPPING* pUniformTexelBufferHwMapping,
    gctUINT                 ArraySize
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, pUniformTexelBufferHwMapping->hwMappingMode);

    switch (pUniformTexelBufferHwMapping->hwMappingMode)
    {
    case VK_UNIFORM_TEXEL_BUFFER_HW_MAPPING_MODE_NATIVELY_SUPPORT:
        _vscEP_Buffer_SaveResourceSlotMapping(pEPBuf, &pUniformTexelBufferHwMapping->u.texMapping);
        break;

    case VK_UNIFORM_TEXEL_BUFFER_HW_MAPPING_MODE_NOT_NATIVELY_SUPPORT:
        VSC_IO_writeUint(pIoBuf, pUniformTexelBufferHwMapping->u.s.hwMemAccessMode);

        if (pUniformTexelBufferHwMapping->u.s.hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_SAMPLER)
        {
            _vscEP_Buffer_SaveSamplerSlotMapping(pEPBuf, &pUniformTexelBufferHwMapping->u.s.hwLoc.samplerMapping);
        }
        else
        {
            gcmASSERT(pUniformTexelBufferHwMapping->u.s.hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR);

            _vscEP_Buffer_SaveConstHwLocMapping(pEPBuf, pUniformTexelBufferHwMapping->u.s.hwLoc.pHwDirectAddrBase);
        }

        if (pUniformTexelBufferHwMapping->u.s.ppExtraSamplerArray != gcvNULL)
        {
            VSC_IO_writeUint(pIoBuf, 1);
            for (i = 0; i < ArraySize; i++)
            {
                _vscEP_Buffer_SavePrivSamplerEntry(pEPBuf, pUniformTexelBufferHwMapping->u.s.ppExtraSamplerArray[i]);
            }
        }
        else
        {
            VSC_IO_writeUint(pIoBuf, 0);
        }
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }
}

/* Save Vulkan uniform texel buffer entry. */
static void
_vscEP_Buffer_SaveVKUniformTexelBufferEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_UNIFORM_TEXEL_BUFFER_TABLE_ENTRY* pUniformTexelBufferEntry
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i, j;
    gctUINT entryMask = 0;

    _vscEP_Buffer_SaveShaderResourceBinding(pEPBuf, &pUniformTexelBufferEntry->utbBinding);
    VSC_IO_writeUint(pIoBuf, pUniformTexelBufferEntry->utbEntryIndex);
    VSC_IO_writeUint(pIoBuf, pUniformTexelBufferEntry->stageBits);
    VSC_IO_writeUint(pIoBuf, pUniformTexelBufferEntry->activeStageMask);

    VSC_IO_writeUint(pIoBuf, (gctUINT)pUniformTexelBufferEntry->utbEntryFlag);

    /* Save texture size. */
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (pUniformTexelBufferEntry->pTextureSize[i][j])
            {
                entryMask |= (1 << (i * 2 + j));
            }
        }
    }
    VSC_IO_writeUint(pIoBuf, entryMask);
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (pUniformTexelBufferEntry->pTextureSize[i][j])
            {
                _vscEP_Buffer_SavePrivConstEntry(pEPBuf, pUniformTexelBufferEntry->pTextureSize[i][j]);
            }
        }
    }

    /* Save the image derived information. */
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        _vscEP_Buffer_SaveImageDerivedInfo(pEPBuf, &pUniformTexelBufferEntry->imageDerivedInfo[i]);
    }

    if (pUniformTexelBufferEntry->utbBinding.arraySize != 0)
    {
        VSC_IO_writeBlock(pIoBuf,
                          (gctCHAR*)pUniformTexelBufferEntry->pResOpBits,
                          sizeof(gctUINT) * pUniformTexelBufferEntry->utbBinding.arraySize);
    }

    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        _vscEP_Buffer_SaveVKUniformTexelBufferHwMapping(pEPBuf,
                                                        &pUniformTexelBufferEntry->hwMappings[i],
                                                        pUniformTexelBufferEntry->utbBinding.arraySize);
    }
}

/* Save Vulkan uniform texel buffer table. */
static void
_vscEP_Buffer_SaveVKUniformTexelBufferTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_UNIFORM_TEXEL_BUFFER_TABLE* pUniformTexelBufferTable
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, pUniformTexelBufferTable->countOfEntries);

    for (i = 0; i < pUniformTexelBufferTable->countOfEntries; i++)
    {
        _vscEP_Buffer_SaveVKUniformTexelBufferEntry(pEPBuf, &pUniformTexelBufferTable->pUtbEntries[i]);
    }
}

/* Save Vulkan input attachment hw mapping. */
static void
_vscEP_Buffer_SaveVKInputAttachmentHwMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_INPUT_ATTACHMENT_HW_MAPPING* pInputAttachmentHwMapping,
    gctUINT32               ArraySize
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    _vscEP_Buffer_SaveUavSlotMapping(pEPBuf, &pInputAttachmentHwMapping->uavMapping);

    if (pInputAttachmentHwMapping->ppExtraSamplerArray != gcvNULL)
    {
        VSC_IO_writeUint(pIoBuf, 1);
        for (i = 0; i < ArraySize; i++)
        {
            _vscEP_Buffer_SavePrivSamplerEntry(pEPBuf, pInputAttachmentHwMapping->ppExtraSamplerArray[i]);
        }
    }
    else
    {
        VSC_IO_writeUint(pIoBuf, 0);
    }
}

/* Save Vulkan input attachment table entry. */
static void
_vscEP_Buffer_SaveVKInutAttachmentEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_INPUT_ATTACHMENT_TABLE_ENTRY* pInputAttachmentEntry
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i, j;
    gctUINT entryMask = 0;

    _vscEP_Buffer_SaveShaderResourceBinding(pEPBuf, &pInputAttachmentEntry->iaBinding);

    VSC_IO_writeUint(pIoBuf, pInputAttachmentEntry->iaEntryIndex);
    VSC_IO_writeUint(pIoBuf, pInputAttachmentEntry->stageBits);
    VSC_IO_writeUint(pIoBuf, pInputAttachmentEntry->activeStageMask);

    /* Save the image derived information. */
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        _vscEP_Buffer_SaveImageDerivedInfo(pEPBuf, &pInputAttachmentEntry->imageDerivedInfo[i]);
    }

    /* Save texture size. */
    entryMask = 0;
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (pInputAttachmentEntry->pTextureSize[i][j])
            {
                entryMask |= (1 << (i * 2 + j));
            }
        }
    }
    VSC_IO_writeUint(pIoBuf, entryMask);
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (pInputAttachmentEntry->pTextureSize[i][j])
            {
                _vscEP_Buffer_SavePrivConstEntry(pEPBuf, pInputAttachmentEntry->pTextureSize[i][j]);
            }
        }
    }

    /* Save lodMinMax */
    entryMask = 0;
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (pInputAttachmentEntry->pLodMinMax[i][j])
            {
                entryMask |= (1 << (i * 2 + j));
            }
        }
    }
    VSC_IO_writeUint(pIoBuf, entryMask);
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (pInputAttachmentEntry->pLodMinMax[i][j])
            {
                _vscEP_Buffer_SavePrivConstEntry(pEPBuf, pInputAttachmentEntry->pLodMinMax[i][j]);
            }
        }
    }

    /* Save levels samples. */
    entryMask = 0;
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (pInputAttachmentEntry->pLevelsSamples[i][j])
            {
                entryMask |= (1 << (i * 2 + j));
            }
        }
    }
    VSC_IO_writeUint(pIoBuf, entryMask);
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (pInputAttachmentEntry->pLevelsSamples[i][j])
            {
                _vscEP_Buffer_SavePrivConstEntry(pEPBuf, pInputAttachmentEntry->pLevelsSamples[i][j]);
            }
        }
    }

    /* Save resOpBits. */
    if (pInputAttachmentEntry->iaBinding.arraySize != 0)
    {
        VSC_IO_writeBlock(pIoBuf,
                          (gctCHAR*)pInputAttachmentEntry->pResOpBits,
                          sizeof(gctUINT) * pInputAttachmentEntry->iaBinding.arraySize);
    }

    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        _vscEP_Buffer_SaveVKInputAttachmentHwMapping(pEPBuf,
                                                     &pInputAttachmentEntry->hwMappings[i],
                                                     pInputAttachmentEntry->iaBinding.arraySize);
    }
}

/* Save Vulkan input attachment table. */
static void
_vscEP_Buffer_SaveVKInputAttachmentTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_INPUT_ATTACHMENT_TABLE* pInputAttachmentTable
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, pInputAttachmentTable->countOfEntries);

    for (i = 0; i < pInputAttachmentTable->countOfEntries; i++)
    {
        _vscEP_Buffer_SaveVKInutAttachmentEntry(pEPBuf, &pInputAttachmentTable->pIaEntries[i]);
    }
}

/* Save Vulkan storage table entry. */
static void
_vscEP_Buffer_SaveVKStorageEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_STORAGE_TABLE_ENTRY* pStorageEntry
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    _vscEP_Buffer_SaveShaderResourceBinding(pEPBuf, &pStorageEntry->storageBinding);
    VSC_IO_writeUint(pIoBuf, pStorageEntry->storageEntryIndex);
    VSC_IO_writeUint(pIoBuf, pStorageEntry->stageBits);
    VSC_IO_writeUint(pIoBuf, pStorageEntry->activeStageMask);

    /* Save the image derived information. */
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        _vscEP_Buffer_SaveImageDerivedInfo(pEPBuf, &pStorageEntry->imageDerivedInfo[i]);
    }

    /* Save resOpBits. */
    if (pStorageEntry->storageBinding.arraySize != 0)
    {
        VSC_IO_writeBlock(pIoBuf,
                          (gctCHAR*)pStorageEntry->pResOpBits,
                          sizeof(gctUINT) * pStorageEntry->storageBinding.arraySize);
    }

    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        _vscEP_Buffer_SaveUavSlotMapping(pEPBuf, &pStorageEntry->hwMappings[i]);
    }
}

/* Save Vulkan storage table. */
static void
_vscEP_Buffer_SaveVKStorageTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_STORAGE_TABLE*  pStorageTable
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, pStorageTable->countOfEntries);

    for (i = 0; i < pStorageTable->countOfEntries; i++)
    {
        _vscEP_Buffer_SaveVKStorageEntry(pEPBuf, &pStorageTable->pStorageEntries[i]);
    }
}

/* Save Vulkan uniform buffer table entry. */
static void
_vscEP_Buffer_SaveVKUniformBufferEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_UNIFORM_BUFFER_TABLE_ENTRY* pUniformBufferEntry
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    _vscEP_Buffer_SaveShaderResourceBinding(pEPBuf, &pUniformBufferEntry->ubBinding);
    VSC_IO_writeUint(pIoBuf, pUniformBufferEntry->ubEntryIndex);
    VSC_IO_writeUint(pIoBuf, pUniformBufferEntry->stageBits);
    VSC_IO_writeUint(pIoBuf, pUniformBufferEntry->activeStageMask);

    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        _vscEP_Buffer_SaveConstHwLocMapping(pEPBuf, &pUniformBufferEntry->hwMappings[i]);
    }
}

/* Save Vulkan uniform buffer table. */
static void
_vscEP_Buffer_SaveVKUniformBuffer(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_UNIFORM_BUFFER_TABLE* pUniformBufferTable
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, pUniformBufferTable->countOfEntries);

    for (i = 0; i < pUniformBufferTable->countOfEntries; i++)
    {
        _vscEP_Buffer_SaveVKUniformBufferEntry(pEPBuf, &pUniformBufferTable->pUniformBufferEntries[i]);
    }
}

/* Save Vulkan resource set. */
static void
_vscEP_Buffer_SaveVKResourceSet(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_RESOURCE_SET*   pVKResourceSet
    )
{
    /* Save Vulkan combined texture-sampler table. */
    _vscEP_Buffer_SaveVKCombinedTextureSamplerTable(pEPBuf, &pVKResourceSet->combinedSampTexTable);

    /* Save Vulkan separated sampler table. */
    _vscEP_Buffer_SaveVKSeparatedSamplerTable(pEPBuf, &pVKResourceSet->separatedSamplerTable);

    /* Save Vulkan separated texture table. */
    _vscEP_Buffer_SaveVKSeparatedTextureTable(pEPBuf, &pVKResourceSet->separatedTexTable);

    /* Save Vulkan uniform texel buffer table. */
    _vscEP_Buffer_SaveVKUniformTexelBufferTable(pEPBuf, &pVKResourceSet->uniformTexBufTable);

    /* Save Vulkan input attachment table. */
    _vscEP_Buffer_SaveVKInputAttachmentTable(pEPBuf, &pVKResourceSet->inputAttachmentTable);

    /* Save Vulkan storage table. */
    _vscEP_Buffer_SaveVKStorageTable(pEPBuf, &pVKResourceSet->storageTable);

    /* Save Vulkan uniform buffer table. */
    _vscEP_Buffer_SaveVKUniformBuffer(pEPBuf, &pVKResourceSet->uniformBufferTable);
}

/* Save shader push constant range. */
static void
_vscEP_Buffer_SaveShaderPushConstRange(
    VSC_EP_IO_BUFFER*       pEPBuf,
    VSC_SHADER_PUSH_CONSTANT_RANGE* pShaderPushConstRange
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_writeUint(pIoBuf, pShaderPushConstRange->offset);
    VSC_IO_writeUint(pIoBuf, pShaderPushConstRange->size);
}

/* Save Vulkan push constant table entry. */
static void
_vscEP_Buffer_SaveVKPushConstEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_PUSH_CONSTANT_TABLE_ENTRY* pPushConstEntry
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    _vscEP_Buffer_SaveShaderPushConstRange(pEPBuf, &pPushConstEntry->pushConstRange);
    VSC_IO_writeUint(pIoBuf, pPushConstEntry->stageBits);
    VSC_IO_writeUint(pIoBuf, pPushConstEntry->activeStageMask);

    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        _vscEP_Buffer_SaveConstHwLocMapping(pEPBuf, &pPushConstEntry->hwMappings[i]);
    }
}

/* Save Vulkan push constant table. */
static void
_vscEP_Buffer_SaveVKPushConstTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_PUSH_CONSTANT_TABLE* pVKPushConstTable
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, pVKPushConstTable->countOfEntries);

    for (i = 0; i < pVKPushConstTable->countOfEntries; i++)
    {
        _vscEP_Buffer_SaveVKPushConstEntry(pEPBuf, &pVKPushConstTable->pPushConstantEntries[i]);
    }
}

/* Save Vulkan sub resource binding. */
static void
_vscEP_Buffer_SaveVKSubResourceBinding(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_SUB_RESOURCE_BINDING* pSubResourceBinding
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    if (pSubResourceBinding->pResBinding != gcvNULL)
    {
        VSC_IO_writeUint(pIoBuf, 1);
        _vscEP_Buffer_SaveShaderResourceBinding(pEPBuf, pSubResourceBinding->pResBinding);
    }
    else
    {
        VSC_IO_writeUint(pIoBuf, 0);
    }

    VSC_IO_writeUint(pIoBuf, pSubResourceBinding->startIdxOfSubArray);
    VSC_IO_writeUint(pIoBuf, pSubResourceBinding->subArraySize);
}

/* Save Vulkan private combined tex-sampl hw mapping. */
static void
_vscEP_Buffer_SaveVKPrivCombTexSampHwMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_PRIV_COMB_TEX_SAMP_HW_MAPPING* pPrivCombTexSampHwMapping
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i, j;

    VSC_IO_writeUint(pIoBuf, pPrivCombTexSampHwMapping->pctsHmEntryIndex);
    VSC_IO_writeUint(pIoBuf, pPrivCombTexSampHwMapping->bSamplerMajor);

    _vscEP_Buffer_SaveVKSubResourceBinding(pEPBuf, &pPrivCombTexSampHwMapping->texSubBinding);
    _vscEP_Buffer_SaveVKSubResourceBinding(pEPBuf, &pPrivCombTexSampHwMapping->samplerSubBinding);

    if (pPrivCombTexSampHwMapping->pSamplerSlotMapping != gcvNULL)
    {
        VSC_IO_writeUint(pIoBuf, 1);
        _vscEP_Buffer_SaveSamplerSlotMapping(pEPBuf, pPrivCombTexSampHwMapping->pSamplerSlotMapping);
    }
    else
    {
        VSC_IO_writeUint(pIoBuf, 0);
    }

    if (pPrivCombTexSampHwMapping->ppExtraSamplerArray != gcvNULL)
    {
        VSC_IO_writeUint(pIoBuf, 1);

        for (i = 0; i < pPrivCombTexSampHwMapping->samplerSubBinding.subArraySize; i++)
        {
            for (j = 0; j < pPrivCombTexSampHwMapping->texSubBinding.subArraySize; j++)
            {
                _vscEP_Buffer_SavePrivSamplerEntry(pEPBuf, &pPrivCombTexSampHwMapping->ppExtraSamplerArray[i][j]);
            }
        }
    }
    else
    {
        VSC_IO_writeUint(pIoBuf, 0);
    }
}

/* Save Vulkan private CTS hw mapping pool. */
static void
_vscEP_Buffer_SaveVKPrivCTSHwMappingPool(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_PRIV_CTS_HW_MAPPING_POOL* pVKPrivCTSHwMappingPool
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, pVKPrivCTSHwMappingPool->countOfArray);

    for (i = 0; i < pVKPrivCTSHwMappingPool->countOfArray; i++)
    {
        _vscEP_Buffer_SaveVKPrivCombTexSampHwMapping(pEPBuf, &pVKPrivCTSHwMappingPool->pPrivCombTsHwMappingArray[i]);
    }
}

/* Save Vulkan high level mapping table. */
static void
_vscEP_Buffer_SaveVKHighLevelMappingTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROGRAM_EXECUTABLE_PROFILE* pPEP
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    /* Start signature. */
    VSC_IO_writeUint(pIoBuf, VKMSTART_SIG);

    /* Save Vulkan resource sets. */
    VSC_IO_writeUint(pIoBuf, pPEP->u.vk.resourceSetCount);
    for (i = 0; i < pPEP->u.vk.resourceSetCount; i++)
    {
        _vscEP_Buffer_SaveVKResourceSet(pEPBuf, &pPEP->u.vk.pResourceSets[i]);
    }

    /* Save Vulkan push constant table. */
    _vscEP_Buffer_SaveVKPushConstTable(pEPBuf, &pPEP->u.vk.pushConstantTable);

    /* Save Vulkan private CTS hw mapping pool. */
    _vscEP_Buffer_SaveVKPrivCTSHwMappingPool(pEPBuf, &pPEP->u.vk.privateCombTsHwMappingPool);

    /* End signature. */
    VSC_IO_writeUint(pIoBuf, VKMEND_SIG);
}

/****************************************Save EP entry functions****************************************/
/* Save SEP to binary. */
static void
_vscEP_Buffer_SaveSEPToBinary(
    VSC_EP_IO_BUFFER*       pEPBuf
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    SHADER_EXECUTABLE_PROFILE*  pSEP = pEPBuf->epData.pSEP;

    /* Start signature. */
    VSC_IO_writeInt(pIoBuf, SEPSTART_SIG);

    VSC_IO_writeUint(pIoBuf, pSEP->profileVersion);
    VSC_IO_writeUint(pIoBuf, pSEP->chipModel);
    VSC_IO_writeUint(pIoBuf, pSEP->chipRevision);
    VSC_IO_writeUint(pIoBuf, pSEP->productID);
    VSC_IO_writeUint(pIoBuf, pSEP->customerID);
    VSC_IO_writeUint(pIoBuf, pSEP->shVersionType);

    VSC_IO_writeUint(pIoBuf, pSEP->countOfMCInst);
    if (pSEP->countOfMCInst != 0)
    {
        VSC_IO_writeBlock(pIoBuf, (gctCHAR*)pSEP->pMachineCode, sizeof(VSC_MC_RAW_INST) * pSEP->countOfMCInst);
    }

    VSC_IO_writeUint(pIoBuf, pSEP->endPCOfMainRoutine);
    VSC_IO_writeUint(pIoBuf, pSEP->gprCount);
    VSC_IO_writeBlock(pIoBuf, (gctCHAR*)&pSEP->exeHints, sizeof(SHADER_EXECUTABLE_HINTS));

    _vscEP_Buffer_SaveIoMapping(pEPBuf, &pSEP->inputMapping);
    _vscEP_Buffer_SaveIoMapping(pEPBuf, &pSEP->outputMapping);
    _vscEP_Buffer_SaveConstMapping(pEPBuf, &pSEP->constantMapping);
    _vscEP_Buffer_SaveSamplerMapping(pEPBuf, &pSEP->samplerMapping);
    _vscEP_Buffer_SaveResourceMapping(pEPBuf, &pSEP->resourceMapping);
    _vscEP_Buffer_SaveUavMapping(pEPBuf, &pSEP->uavMapping);
    _vscEP_Buffer_SaveStaticPrivMapping(pEPBuf, &pSEP->staticPrivMapping);
    _vscEP_Buffer_SaveDynamicPrivMapping(pEPBuf, &pSEP->dynamicPrivMapping);
    _vscEP_Buffer_SaveDefaultUboMapping(pEPBuf, &pSEP->defaultUboMapping);

    /* End signature. */
    VSC_IO_writeInt(pIoBuf, SEPEND_SIG);
}

/* Save KEP to binary. */
static void
_vscEP_Buffer_SaveKEPToBinary(
    VSC_EP_IO_BUFFER*       pEPBuf
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    KERNEL_EXECUTABLE_PROFILE*  pKEP = pEPBuf->epData.pKEP;

    /* Start signature. */
    VSC_IO_writeInt(pIoBuf, KEPSTART_SIG);

    /* Save the SEP. */
    pEPBuf->epKind = VSC_EP_KIND_SHADER;
    pEPBuf->epData.pSEP = &pKEP->sep;
    _vscEP_Buffer_SaveSEPToBinary(pEPBuf);
    pEPBuf->epKind = VSC_EP_KIND_KERNEL;
    pEPBuf->epData.pKEP = pKEP;

    /* End signature. */
    VSC_IO_writeInt(pIoBuf, KEPEND_SIG);
}

/* Save PEP to binary. */
static void
_vscEP_Buffer_SavePEPToBinary(
    VSC_EP_IO_BUFFER*       pEPBuf
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    PROGRAM_EXECUTABLE_PROFILE*  pPEP = pEPBuf->epData.pPEP;
    gctUINT i;

    /* Start signature. */
    VSC_IO_writeInt(pIoBuf, PEPSTART_SIG);

    /* Save pep client version. */
    VSC_IO_writeUint(pIoBuf, pPEP->pepClient);

    /* Save all SEP. */
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        pEPBuf->epKind = VSC_EP_KIND_SHADER;
        pEPBuf->epData.pSEP = &pPEP->seps[i];
        _vscEP_Buffer_SaveSEPToBinary(pEPBuf);
    }
    pEPBuf->epKind = VSC_EP_KIND_PROGRAM;
    pEPBuf->epData.pPEP = pPEP;

    /* Save attribute table. */
    _vscEP_Buffer_SaveProgAttrTable(pEPBuf, &pPEP->attribTable);

    /* Save fragment output table. */
    _vscEP_Buffer_SaveFragOutTable(pEPBuf, &pPEP->fragOutTable);

    /* Save other high level mapping table. */
    switch (pPEP->pepClient)
    {
    case PEP_CLIENT_GL:
        _vscEP_Buffer_SaveGLHighLevelMappingTable(pEPBuf, pPEP);
        break;

    case PEP_CLIENT_VK:
        _vscEP_Buffer_SaveVKHighLevelMappingTable(pEPBuf, pPEP);
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    /* End signature. */
    VSC_IO_writeInt(pIoBuf, PEPEND_SIG);
}

/* save EP data to binary. */
static void
_vscEP_Buffer_SaveToBinary(
    VSC_EP_IO_BUFFER*       pEPBuf
    )
{
    VSC_IO_writeUint(pEPBuf->pIoBuf, EPSTART_SIG);
    VSC_IO_writeUint(pEPBuf->pIoBuf, pEPBuf->epKind);

    switch (pEPBuf->epKind)
    {
    case VSC_EP_KIND_SHADER:
        _vscEP_Buffer_SaveSEPToBinary(pEPBuf);
        break;

    case VSC_EP_KIND_KERNEL:
        _vscEP_Buffer_SaveKEPToBinary(pEPBuf);
        break;

    case VSC_EP_KIND_PROGRAM:
        _vscEP_Buffer_SavePEPToBinary(pEPBuf);
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    VSC_IO_writeUint(pEPBuf->pIoBuf, EPEND_SIG);
}

/****************************************Load SEP-related functions****************************************/
static VSC_ErrCode
_vscEP_Buffer_LoadConstHwLocMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_CONSTANT_HW_LOCATION_MAPPING*pConstHwLocMapping
    );

static VSC_ErrCode
_vscEP_Buffer_LoadPrivMappingCommonEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_PRIV_MAPPING_COMMON_ENTRY* pPrivMappingCommonEntry
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT uVal = 0;

    VSC_IO_readUint(pIoBuf, &pPrivMappingCommonEntry->privmKind);
    VSC_IO_readUint(pIoBuf, &pPrivMappingCommonEntry->privmKindIndex);

    /* All private data is a UINT, please check _PostProcessResourceSetTables. */
    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != 0)
    {
        VSC_EP_ALLOC_MEM(pPrivMappingCommonEntry->pPrivateData,
                         gctUINT,
                         sizeof(gctUINT));
        VSC_IO_readUint(pIoBuf, (gctUINT*)pPrivMappingCommonEntry->pPrivateData);
    }
    else
    {
        pPrivMappingCommonEntry->pPrivateData = gcvNULL;
    }

    return errCode;
}

/* Load IO mapping. */
static VSC_ErrCode
_vscEP_Buffer_LoadIoRegMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_IO_REG_MAPPING*  pIoRegMapping
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_readBlock(pIoBuf, (gctCHAR*)pIoRegMapping->ioChannelMapping, sizeof(SHADER_IO_CHANNEL_MAPPING) * CHANNEL_NUM);

    VSC_IO_readUint(pIoBuf, &pIoRegMapping->ioIndex);
    VSC_IO_readUint(pIoBuf, &pIoRegMapping->ioChannelMask);
    VSC_IO_readUint(pIoBuf, &pIoRegMapping->firstValidIoChannel);
    VSC_IO_readLong(pIoBuf, &pIoRegMapping->packedIoIndexMask);
    VSC_IO_readUint(pIoBuf, &pIoRegMapping->soStreamBufferSlot);
    VSC_IO_readUint(pIoBuf, &pIoRegMapping->soSeqInStreamBuffer);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pIoRegMapping->regIoMode);

    return errCode;
}

static VSC_ErrCode
_vscEP_Buffer_LoadIoMappingPerExeObj(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_IO_MAPPING_PER_EXE_OBJ* pPerExeObj
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_readUint(pIoBuf, &pPerExeObj->countOfIoRegMapping);

    if (pPerExeObj->countOfIoRegMapping != 0)
    {
        VSC_EP_ALLOC_MEM(pPerExeObj->pIoRegMapping,
                         SHADER_IO_REG_MAPPING,
                         sizeof(SHADER_IO_REG_MAPPING) * pPerExeObj->countOfIoRegMapping);
        for (i = 0; i < pPerExeObj->countOfIoRegMapping; i++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadIoRegMapping(pEPBuf, &pPerExeObj->pIoRegMapping[i]));
        }
    }
    else
    {
        pPerExeObj->pIoRegMapping = gcvNULL;
    }

    VSC_IO_readLong(pIoBuf, &pPerExeObj->ioIndexMask);
    VSC_IO_readBlock(pIoBuf, (gctCHAR*)pPerExeObj->usage2IO, sizeof(USAGE_2_IO) * SHADER_IO_USAGE_TOTAL_COUNT);
    VSC_IO_readLong(pIoBuf, &pPerExeObj->soIoIndexMask);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pPerExeObj->ioMode);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pPerExeObj->ioMemAlign);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pPerExeObj->ioCategory);

OnError:
    return errCode;
}

static VSC_ErrCode
_vscEP_Buffer_LoadIoMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_IO_MAPPING*      pShaderIoMapping
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctUINT     uVal;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != IOMSTART_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid shader signature 0x%x.", uVal);
    }

    ON_ERROR0(_vscEP_Buffer_LoadIoMappingPerExeObj(pEPBuf, &pShaderIoMapping->ioVtxPxl));
    ON_ERROR0(_vscEP_Buffer_LoadIoMappingPerExeObj(pEPBuf, &pShaderIoMapping->ioPrim));

    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != IOMEND_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid shader signature 0x%x.", uVal);
    }

OnError:
    return errCode;
}

/* Load UAV mapping. */
static VSC_ErrCode
_vscEP_Buffer_LoadUavSlotMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_UAV_SLOT_MAPPING*pUavSlotMapping
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT uVal = 0;

    VSC_IO_readUint(pIoBuf, &pUavSlotMapping->uavSlotIndex);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pUavSlotMapping->accessMode);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pUavSlotMapping->hwMemAccessMode);
    VSC_IO_readUint(pIoBuf, &pUavSlotMapping->sizeInByte);

    switch (pUavSlotMapping->accessMode)
    {
    case SHADER_UAV_ACCESS_MODE_TYPE:
        VSC_IO_readUint(pIoBuf, (gctUINT*)&pUavSlotMapping->u.s.uavDimension);
        VSC_IO_readUint(pIoBuf, (gctUINT*)&pUavSlotMapping->u.s.uavType);
        break;

    case SHADER_UAV_ACCESS_MODE_RESIZABLE:
        VSC_IO_readUint(pIoBuf, &pUavSlotMapping->u.sizableEleSize);
        break;

    case SHADER_UAV_ACCESS_MODE_STRUCTURED:
        VSC_IO_readUint(pIoBuf, &pUavSlotMapping->u.structureSize);
        break;

    default:
        break;
    }

    switch (pUavSlotMapping->hwMemAccessMode)
    {
    case SHADER_HW_MEM_ACCESS_MODE_PLACE_HOLDER:
        VSC_IO_readUint(pIoBuf, &pUavSlotMapping->hwLoc.hwUavSlot);
        break;

    case SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR:
        VSC_IO_readUint(pIoBuf, &uVal);

        if (uVal != 0)
        {
            VSC_EP_ALLOC_MEM(pUavSlotMapping->hwLoc.pHwDirectAddrBase,
                             SHADER_CONSTANT_HW_LOCATION_MAPPING,
                             sizeof(SHADER_CONSTANT_HW_LOCATION_MAPPING));
            ON_ERROR0(_vscEP_Buffer_LoadConstHwLocMapping(pEPBuf, pUavSlotMapping->hwLoc.pHwDirectAddrBase));
        }
        else
        {
            pUavSlotMapping->hwLoc.pHwDirectAddrBase = gcvNULL;
        }
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

OnError:
    return errCode;
}

static VSC_ErrCode
_vscEP_Buffer_LoadUavMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_UAV_MAPPING*     pShaderUavMapping
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT uVal = 0;
    gctUINT i;

    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != UAMSTART_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid shader signature 0x%x.", uVal);
    }

    VSC_IO_readUint(pIoBuf, &pShaderUavMapping->countOfUAVs);

    if (pShaderUavMapping->countOfUAVs != 0)
    {
        VSC_EP_ALLOC_MEM(pShaderUavMapping->pUAV,
                         SHADER_UAV_SLOT_MAPPING,
                         sizeof(SHADER_UAV_SLOT_MAPPING) * pShaderUavMapping->countOfUAVs);
        for (i = 0; i < pShaderUavMapping->countOfUAVs; i++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadUavSlotMapping(pEPBuf, &pShaderUavMapping->pUAV[i]));
        }
    }
    else
    {
        pShaderUavMapping->pUAV = gcvNULL;
    }

    VSC_IO_readUint(pIoBuf, &pShaderUavMapping->uavSlotMask);
    VSC_IO_readBlock(pIoBuf,
                     (gctCHAR*)pShaderUavMapping->dim2UavSlotMask,
                     sizeof(gctUINT) * SHADER_UAV_DIMENSION_TOTAL_COUNT);

    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != UAMEND_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid shader signature 0x%x.", uVal);
    }

OnError:
    return errCode;
}

/* Load resource mapping. */
static VSC_ErrCode
_vscEP_Buffer_LoadResourceSlotMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_RESOURCE_SLOT_MAPPING* pResourceSlotMapping
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_readUint(pIoBuf, &pResourceSlotMapping->resourceSlotIndex);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pResourceSlotMapping->accessMode);

    switch (pResourceSlotMapping->accessMode)
    {
    case SHADER_RESOURCE_ACCESS_MODE_TYPE:
        VSC_IO_readUint(pIoBuf, (gctUINT*)&pResourceSlotMapping->u.s.resourceDimension);
        VSC_IO_readUint(pIoBuf, (gctUINT*)&pResourceSlotMapping->u.s.resourceReturnType);
        break;

    case SHADER_RESOURCE_ACCESS_MODE_STRUCTURED:
        VSC_IO_readUint(pIoBuf, &pResourceSlotMapping->u.structureSize);
        break;

    case SHADER_RESOURCE_ACCESS_MODE_RAW:
    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    VSC_IO_readUint(pIoBuf, &pResourceSlotMapping->hwResourceSlot);

    return errCode;
}

static VSC_ErrCode
_vscEP_Buffer_LoadResourceMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_RESOURCE_MAPPING* pShaderResourceMapping
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT uVal = 0;
    gctUINT i;

    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != REMSTART_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid shader signature 0x%x.", uVal);
    }

    VSC_IO_readUint(pIoBuf, &pShaderResourceMapping->countOfResources);

    if (pShaderResourceMapping->countOfResources != 0)
    {
        VSC_EP_ALLOC_MEM(pShaderResourceMapping->pResource,
                         SHADER_RESOURCE_SLOT_MAPPING,
                         sizeof(SHADER_RESOURCE_SLOT_MAPPING) * pShaderResourceMapping->countOfResources);
        for (i = 0; i < pShaderResourceMapping->countOfResources; i++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadResourceSlotMapping(pEPBuf, &pShaderResourceMapping->pResource[i]));
        }
    }
    else
    {
        pShaderResourceMapping->pResource = gcvNULL;
    }

    VSC_IO_readBlock(pIoBuf,
                     (gctCHAR*)pShaderResourceMapping->resourceSlotMask,
                     sizeof(gctUINT) * 4);
    VSC_IO_readBlock(pIoBuf,
                     (gctCHAR*)pShaderResourceMapping->dim2ResourceSlotMask,
                     sizeof(gctUINT) * SHADER_RESOURCE_DIMENSION_TOTAL_COUNT * 4);

    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != REMEND_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid shader signature 0x%x.", uVal);
    }

OnError:
    return errCode;
}

/* Load sampler mapping. */
static VSC_ErrCode
_vscEP_Buffer_LoadSamplerSlotMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_SAMPLER_SLOT_MAPPING* pSamplerSlotMapping
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_readUint(pIoBuf, &pSamplerSlotMapping->samplerSlotIndex);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pSamplerSlotMapping->samplerDimension);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pSamplerSlotMapping->samplerReturnType);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pSamplerSlotMapping->samplerMode);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pSamplerSlotMapping->hwSamplerSlot);

    return errCode;
}

static VSC_ErrCode
_vscEP_Buffer_LoadSamplerMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_SAMPLER_MAPPING* pShaderSamplerMapping
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT uVal = 0;
    gctUINT i;

    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != SMMSTART_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid shader signature 0x%x.", uVal);
    }

    VSC_IO_readUint(pIoBuf, &pShaderSamplerMapping->countOfSamplers);

    if (pShaderSamplerMapping->countOfSamplers != 0)
    {
        VSC_EP_ALLOC_MEM(pShaderSamplerMapping->pSampler,
                         SHADER_SAMPLER_SLOT_MAPPING,
                         sizeof(SHADER_SAMPLER_SLOT_MAPPING) * pShaderSamplerMapping->countOfSamplers);
        for (i = 0; i < pShaderSamplerMapping->countOfSamplers; i++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadSamplerSlotMapping(pEPBuf, &pShaderSamplerMapping->pSampler[i]));
        }
    }
    else
    {
        pShaderSamplerMapping->pSampler = gcvNULL;
    }

    VSC_IO_readUint(pIoBuf, &pShaderSamplerMapping->samplerSlotMask);
    VSC_IO_readBlock(pIoBuf, (gctCHAR*)pShaderSamplerMapping->dim2SamplerSlotMask, sizeof(gctUINT) * SHADER_RESOURCE_DIMENSION_TOTAL_COUNT);
    VSC_IO_readUint(pIoBuf, &pShaderSamplerMapping->hwSamplerRegCount);
    VSC_IO_readInt(pIoBuf, &pShaderSamplerMapping->maxHwSamplerRegIndex);

    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != SMMEND_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid shader signature 0x%x.", uVal);
    }

OnError:
    return errCode;
}

/* Load constant mapping. */
static VSC_ErrCode
_vscEP_Buffer_LoadConstSubArrayMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_CONSTANT_SUB_ARRAY_MAPPING* pConstSubArrayMapping
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_readUint(pIoBuf, &pConstSubArrayMapping->startIdx);
    VSC_IO_readUint(pIoBuf, &pConstSubArrayMapping->subArrayRange);
    VSC_IO_readUint(pIoBuf, &pConstSubArrayMapping->firstMSCSharpRegNo);
    VSC_IO_readUint(pIoBuf, &pConstSubArrayMapping->validChannelMask);
    ON_ERROR0(_vscEP_Buffer_LoadConstHwLocMapping(pEPBuf, &pConstSubArrayMapping->hwFirstConstantLocation));

OnError:
    return errCode;
}

static VSC_ErrCode
_vscEP_Buffer_LoadConstHwLocMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_CONSTANT_HW_LOCATION_MAPPING*pConstHwLocMapping
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT uVal = 0;

    VSC_IO_readUint(pIoBuf, (gctUINT*)&pConstHwLocMapping->hwAccessMode);

    switch (pConstHwLocMapping->hwAccessMode)
    {
    case SHADER_HW_ACCESS_MODE_REGISTER:
        VSC_IO_readUint(pIoBuf, &pConstHwLocMapping->hwLoc.constReg.hwRegNo);
        VSC_IO_readUint(pIoBuf, &pConstHwLocMapping->hwLoc.constReg.hwRegRange);
        break;

    case SHADER_HW_ACCESS_MODE_MEMORY:
        VSC_IO_readUint(pIoBuf, (gctUINT*)&pConstHwLocMapping->hwLoc.memAddr.hwMemAccessMode);
        switch (pConstHwLocMapping->hwLoc.memAddr.hwMemAccessMode)
        {
        case SHADER_HW_MEM_ACCESS_MODE_PLACE_HOLDER:
            VSC_IO_readUint(pIoBuf, &pConstHwLocMapping->hwLoc.memAddr.memBase.hwConstantArraySlot);
            break;

        case SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR:
            VSC_IO_readUint(pIoBuf, &uVal);

            if (uVal != 0)
            {
                VSC_EP_ALLOC_MEM(pConstHwLocMapping->hwLoc.memAddr.memBase.pHwDirectAddrBase,
                                 SHADER_CONSTANT_HW_LOCATION_MAPPING,
                                 sizeof(SHADER_CONSTANT_HW_LOCATION_MAPPING));
                ON_ERROR0(_vscEP_Buffer_LoadConstHwLocMapping(pEPBuf, pConstHwLocMapping->hwLoc.memAddr.memBase.pHwDirectAddrBase));
            }
            else
            {
                pConstHwLocMapping->hwLoc.memAddr.memBase.pHwDirectAddrBase = gcvNULL;
            }
            break;

        case SHADER_HW_MEM_ACCESS_MODE_SRV:
            VSC_IO_readUint(pIoBuf, &uVal);

            if (uVal != 0)
            {
                VSC_EP_ALLOC_MEM(pConstHwLocMapping->hwLoc.memAddr.memBase.pSrv,
                                 SHADER_RESOURCE_SLOT_MAPPING,
                                 sizeof(SHADER_RESOURCE_SLOT_MAPPING));
                ON_ERROR0(_vscEP_Buffer_LoadResourceSlotMapping(pEPBuf, pConstHwLocMapping->hwLoc.memAddr.memBase.pSrv));
            }
            else
            {
                pConstHwLocMapping->hwLoc.memAddr.memBase.pSrv = gcvNULL;
            }
            break;

        case SHADER_HW_MEM_ACCESS_MODE_UAV:
            VSC_IO_readUint(pIoBuf, &uVal);

            if (uVal != 0)
            {
                VSC_EP_ALLOC_MEM(pConstHwLocMapping->hwLoc.memAddr.memBase.pUav,
                                 SHADER_UAV_SLOT_MAPPING,
                                 sizeof(SHADER_UAV_SLOT_MAPPING));
                ON_ERROR0(_vscEP_Buffer_LoadUavSlotMapping(pEPBuf, pConstHwLocMapping->hwLoc.memAddr.memBase.pUav));
            }
            else
            {
                pConstHwLocMapping->hwLoc.memAddr.memBase.pUav = gcvNULL;
            }
            break;

        default:
            gcmASSERT(gcvFALSE);
            break;
        }
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    VSC_IO_readUint(pIoBuf, (gctUINT *)&pConstHwLocMapping->hwLoc.memAddr.constantOffsetKind);
    VSC_IO_readUint(pIoBuf, &pConstHwLocMapping->hwLoc.memAddr.constantOffset);
    VSC_IO_readUint(pIoBuf, &pConstHwLocMapping->hwLoc.memAddr.componentSizeInByte);

    VSC_IO_readUint(pIoBuf, &pConstHwLocMapping->validHWChannelMask);
    VSC_IO_readUint(pIoBuf, &pConstHwLocMapping->firstValidHwChannel);

OnError:
    return errCode;
}

static VSC_ErrCode
_vscEP_Buffer_LoadCTC(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_COMPILE_TIME_CONSTANT *pCTC
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_readBlock(pIoBuf,
                     (gctCHAR*)pCTC->constantValue,
                     sizeof(gctUINT) * CHANNEL_NUM);
    ON_ERROR0(_vscEP_Buffer_LoadConstHwLocMapping(pEPBuf, &pCTC->hwConstantLocation));

OnError:
    return errCode;
}

static VSC_ErrCode
_vscEP_Buffer_LoadConstArrayMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_CONSTANT_ARRAY_MAPPING* pConstArrayMapping
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT j;

    VSC_IO_readUint(pIoBuf, &pConstArrayMapping->constantArrayIndex);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pConstArrayMapping->constantUsage);
    VSC_IO_readUint(pIoBuf, &pConstArrayMapping->arrayRange);
    VSC_IO_readUint(pIoBuf, &pConstArrayMapping->countOfSubConstantArray);

    if (pConstArrayMapping->countOfSubConstantArray != 0)
    {
        VSC_EP_ALLOC_MEM(pConstArrayMapping->pSubConstantArrays,
                         SHADER_CONSTANT_SUB_ARRAY_MAPPING,
                         sizeof(SHADER_CONSTANT_SUB_ARRAY_MAPPING) * pConstArrayMapping->countOfSubConstantArray);
        for (j = 0; j < pConstArrayMapping->countOfSubConstantArray; j++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadConstSubArrayMapping(pEPBuf, &pConstArrayMapping->pSubConstantArrays[j]));
        }
    }
    else
    {
        pConstArrayMapping->pSubConstantArrays = gcvNULL;
    }

OnError:
    return errCode;
}

static VSC_ErrCode
_vscEP_Buffer_LoadConstMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_CONSTANT_MAPPING*pShaderConstMapping
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT uVal = 0;
    gctUINT i;

    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != COMSTART_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid shader signature 0x%x.", uVal);
    }

    /* Load constant array mapping. */
    VSC_IO_readUint(pIoBuf, &pShaderConstMapping->countOfConstantArrayMapping);
    if (pShaderConstMapping->countOfConstantArrayMapping != 0)
    {
        VSC_EP_ALLOC_MEM(pShaderConstMapping->pConstantArrayMapping,
                         SHADER_CONSTANT_ARRAY_MAPPING,
                         sizeof(SHADER_CONSTANT_ARRAY_MAPPING) * pShaderConstMapping->countOfConstantArrayMapping);
        for (i = 0; i < pShaderConstMapping->countOfConstantArrayMapping; i++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadConstArrayMapping(pEPBuf, &pShaderConstMapping->pConstantArrayMapping[i]));
        }
    }
    else
    {
        pShaderConstMapping->pConstantArrayMapping = gcvNULL;
    }

    /* Load array index. */
    VSC_IO_readUint(pIoBuf, &pShaderConstMapping->arrayIndexMask);
    VSC_IO_readBlock(pIoBuf, (gctCHAR*)pShaderConstMapping->usage2ArrayIndex, sizeof(gctUINT) * SHADER_CONSTANT_USAGE_TOTAL_COUNT);

    /* Load compile time constant. */
    VSC_IO_readUint(pIoBuf, &pShaderConstMapping->countOfCompileTimeConstant);
    if (pShaderConstMapping->countOfCompileTimeConstant != 0)
    {
        VSC_EP_ALLOC_MEM(pShaderConstMapping->pCompileTimeConstant,
                         SHADER_COMPILE_TIME_CONSTANT,
                         sizeof(SHADER_COMPILE_TIME_CONSTANT) * pShaderConstMapping->countOfCompileTimeConstant);
        for (i = 0; i < pShaderConstMapping->countOfCompileTimeConstant; i++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadCTC(pEPBuf, &pShaderConstMapping->pCompileTimeConstant[i]));
        }
    }
    else
    {
        pShaderConstMapping->pCompileTimeConstant = gcvNULL;
    }

    /* Load hw reg index. */
    VSC_IO_readUint(pIoBuf, &pShaderConstMapping->hwConstRegCount);
    VSC_IO_readInt(pIoBuf, &pShaderConstMapping->maxHwConstRegIndex);

    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != COMEND_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid shader signature 0x%x.", uVal);
    }

OnError:
    return errCode;
}

/* Load priv constant entry. */
static VSC_ErrCode
_vscEP_Buffer_LoadPrivConstEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_PRIV_CONSTANT_ENTRY* pPrivConstEntry
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT uVal = 0;

    /* Load SHADER_PRIV_MAPPING_COMMON_ENTRY. */
    ON_ERROR0(_vscEP_Buffer_LoadPrivMappingCommonEntry(pEPBuf, &pPrivConstEntry->commonPrivm));

    /* Load const node. */
    VSC_IO_readUint(pIoBuf, (gctUINT*)(&pPrivConstEntry->mode));
    switch (pPrivConstEntry->mode)
    {
    case SHADER_PRIV_CONSTANT_MODE_CTC:
        ON_ERROR0(_vscEP_Buffer_LoadCTC(pEPBuf, pPrivConstEntry->u.ctcConstant.pCTC));
        VSC_IO_readUint(pIoBuf, &pPrivConstEntry->u.ctcConstant.hwChannelMask);
        break;

    case SHADER_PRIV_CONSTANT_MODE_VAL_2_INST:
        VSC_IO_readUint(pIoBuf, &pPrivConstEntry->u.instImm.patchedPC);
        VSC_IO_readUint(pIoBuf, &pPrivConstEntry->u.instImm.srcNo);
        break;

    case SHADER_PRIV_CONSTANT_MODE_VAL_2_MEMORREG:
        VSC_IO_readUint(pIoBuf, &uVal);

        if (uVal != 0)
        {
            VSC_EP_ALLOC_MEM(pPrivConstEntry->u.pSubCBMapping,
                             SHADER_CONSTANT_SUB_ARRAY_MAPPING,
                             sizeof(SHADER_CONSTANT_SUB_ARRAY_MAPPING));
            ON_ERROR0(_vscEP_Buffer_LoadConstSubArrayMapping(pEPBuf, pPrivConstEntry->u.pSubCBMapping));
        }
        else
        {
            pPrivConstEntry->u.pSubCBMapping = gcvNULL;
        }
        break;

    case SHADER_PRIV_CONSTANT_MODE_VAL_2_DUBO:
        VSC_IO_readUint(pIoBuf, &pPrivConstEntry->u.duboEntryIndex);
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

OnError:
    return errCode;
}

/* Load priv constant mapping. */
static VSC_ErrCode
_vscEP_Buffer_LoadPrivConstMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_PRIV_CONSTANT_MAPPING* pPrivConstMapping
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_readUint(pIoBuf, &pPrivConstMapping->countOfEntries);

    if (pPrivConstMapping->countOfEntries != 0)
    {
        VSC_EP_ALLOC_MEM(pPrivConstMapping->pPrivmConstantEntries,
                         SHADER_PRIV_CONSTANT_ENTRY,
                         sizeof(SHADER_PRIV_CONSTANT_ENTRY) * pPrivConstMapping->countOfEntries);
        for (i = 0; i < pPrivConstMapping->countOfEntries; i++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadPrivConstEntry(pEPBuf, &pPrivConstMapping->pPrivmConstantEntries[i]));
        }
    }
    else
    {
        pPrivConstMapping->pPrivmConstantEntries = gcvNULL;
    }

OnError:
    return errCode;
}

/*Load priv mem data mapping. */
static VSC_ErrCode
_vscEP_Buffer_LoadPrivMemDataMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_PRIV_MEM_DATA_MAPPING* pPrivMemDataMapping
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_readUint(pIoBuf, &pPrivMemDataMapping->ctcCount);

    if (pPrivMemDataMapping->ctcCount != 0)
    {
        VSC_EP_ALLOC_MEM(pPrivMemDataMapping->ppCTC,
                         SHADER_COMPILE_TIME_CONSTANT*,
                         sizeof(SHADER_COMPILE_TIME_CONSTANT*) * pPrivMemDataMapping->ctcCount);
        for (i = 0; i < pPrivMemDataMapping->ctcCount; i++)
        {
            VSC_EP_ALLOC_MEM(pPrivMemDataMapping->ppCTC[i],
                             SHADER_COMPILE_TIME_CONSTANT,
                             sizeof(SHADER_COMPILE_TIME_CONSTANT));
            ON_ERROR0(_vscEP_Buffer_LoadCTC(pEPBuf, pPrivMemDataMapping->ppCTC[i]));
        }
    }
    else
    {
        pPrivMemDataMapping->ppCTC = gcvNULL;
    }

    VSC_IO_readUint(pIoBuf, &pPrivMemDataMapping->cnstSubArrayCount);

    if (pPrivMemDataMapping->cnstSubArrayCount != 0)
    {
        VSC_EP_ALLOC_MEM(pPrivMemDataMapping->ppCnstSubArray,
                         SHADER_CONSTANT_SUB_ARRAY_MAPPING*,
                         sizeof(SHADER_CONSTANT_SUB_ARRAY_MAPPING*) * pPrivMemDataMapping->cnstSubArrayCount);
        for (i = 0; i < pPrivMemDataMapping->cnstSubArrayCount; i++)
        {
            VSC_EP_ALLOC_MEM(pPrivMemDataMapping->ppCnstSubArray[i],
                             SHADER_CONSTANT_SUB_ARRAY_MAPPING,
                             sizeof(SHADER_CONSTANT_SUB_ARRAY_MAPPING));
            ON_ERROR0(_vscEP_Buffer_LoadConstSubArrayMapping(pEPBuf, pPrivMemDataMapping->ppCnstSubArray[i]));
        }
    }
    else
    {
        pPrivMemDataMapping->ppCnstSubArray = gcvNULL;
    }

OnError:
    return errCode;
}

/* Load priv UAV entry. */
static VSC_ErrCode
_vscEP_Buffer_LoadPrivUavEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_PRIV_UAV_ENTRY*  pPrivUavEntry
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT uVal = 0;

    VSC_IO_readUint(pIoBuf, &pPrivUavEntry->uavEntryIndex);

    /* Load SHADER_PRIV_MAPPING_COMMON_ENTRY. */
    ON_ERROR0(_vscEP_Buffer_LoadPrivMappingCommonEntry(pEPBuf, &pPrivUavEntry->commonPrivm));

    /* Load SHADER_PRIV_MEM_DATA_MAPPING. */
    ON_ERROR0(_vscEP_Buffer_LoadPrivMemDataMapping(pEPBuf, &pPrivUavEntry->memData));

    /* Load SHADER_UAV_SLOT_MAPPING. */
    VSC_IO_readUint(pIoBuf, &uVal);

    if (uVal != 0)
    {
        VSC_EP_ALLOC_MEM(pPrivUavEntry->pBuffer,
                         SHADER_UAV_SLOT_MAPPING,
                         sizeof(SHADER_UAV_SLOT_MAPPING));
        ON_ERROR0(_vscEP_Buffer_LoadUavSlotMapping(pEPBuf, pPrivUavEntry->pBuffer));
    }
    else
    {
        pPrivUavEntry->pBuffer = gcvNULL;
    }

OnError:
    return errCode;
}

/* Load priv UAV mapping. */
static VSC_ErrCode
_vscEP_Buffer_LoadPrivUavMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_PRIV_UAV_MAPPING*pPrivUavMapping
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_readUint(pIoBuf, &pPrivUavMapping->countOfEntries);
    if (pPrivUavMapping->countOfEntries != 0)
    {
        VSC_EP_ALLOC_MEM(pPrivUavMapping->pPrivUavEntries,
                         SHADER_PRIV_UAV_ENTRY,
                         sizeof(SHADER_PRIV_UAV_ENTRY) * pPrivUavMapping->countOfEntries);
        /* Load SHADER_PRIV_UAV_ENTRY. */
        for (i = 0; i < pPrivUavMapping->countOfEntries; i++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadPrivUavEntry(pEPBuf, &pPrivUavMapping->pPrivUavEntries[i]));
        }
    }
    else
    {
        pPrivUavMapping->pPrivUavEntries = gcvNULL;
    }

OnError:
    return errCode;
}

/* Load static priv mapping. */
static VSC_ErrCode
_vscEP_Buffer_LoadStaticPrivMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_STATIC_PRIV_MAPPING* pStaticPrivMapping
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT uVal = 0;

    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != SPMSTART_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid shader signature 0x%x.", uVal);
    }

    /* Load private constant mapping. */
    ON_ERROR0(_vscEP_Buffer_LoadPrivConstMapping(pEPBuf, &pStaticPrivMapping->privConstantMapping));

    /* Load private uav mapping. */
    ON_ERROR0(_vscEP_Buffer_LoadPrivUavMapping(pEPBuf, &pStaticPrivMapping->privUavMapping));

    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != SPMEND_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid shader signature 0x%x.", uVal);
    }

OnError:
    return errCode;
}

/* Load priv sampler entry. */
static VSC_ErrCode
_vscEP_Buffer_LoadPrivSamplerEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_PRIV_SAMPLER_ENTRY*pPrivSamplerEntry
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT uVal = 0;

    ON_ERROR0(_vscEP_Buffer_LoadPrivMappingCommonEntry(pEPBuf, &pPrivSamplerEntry->commonPrivm));

    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != 0)
    {
        VSC_EP_ALLOC_MEM(pPrivSamplerEntry->pSampler,
                         SHADER_SAMPLER_SLOT_MAPPING,
                         sizeof(SHADER_SAMPLER_SLOT_MAPPING));
        ON_ERROR0(_vscEP_Buffer_LoadSamplerSlotMapping(pEPBuf, pPrivSamplerEntry->pSampler));
    }
    else
    {
        pPrivSamplerEntry->pSampler = gcvNULL;
    }

OnError:
    return errCode;
}

/* Load priv sampler mapping. */
static VSC_ErrCode
_vscEP_Buffer_LoadPrivSamplerMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_PRIV_SAMPLER_MAPPING*pPrivSamplerMapping
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_readUint(pIoBuf, &pPrivSamplerMapping->countOfEntries);
    if (pPrivSamplerMapping->countOfEntries != 0)
    {
        VSC_EP_ALLOC_MEM(pPrivSamplerMapping->pPrivSamplerEntries,
                         SHADER_PRIV_SAMPLER_ENTRY,
                         sizeof(SHADER_PRIV_SAMPLER_ENTRY) * pPrivSamplerMapping->countOfEntries);
        for (i = 0; i < pPrivSamplerMapping->countOfEntries; i++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadPrivSamplerEntry(pEPBuf, &pPrivSamplerMapping->pPrivSamplerEntries[i]));
        }
    }
    else
    {
        pPrivSamplerMapping->pPrivSamplerEntries = gcvNULL;
    }

OnError:
    return errCode;
}

/* Load priv output entry. */
static VSC_ErrCode
_vscEP_Buffer_LoadPrivOutputEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_PRIV_OUTPUT_ENTRY*pPrivOutputEntry
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT uVal = 0;

    ON_ERROR0(_vscEP_Buffer_LoadPrivMappingCommonEntry(pEPBuf, &pPrivOutputEntry->commonPrivm));

    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != 0)
    {
        VSC_EP_ALLOC_MEM(pPrivOutputEntry->pOutput,
                         SHADER_IO_REG_MAPPING,
                         sizeof(SHADER_IO_REG_MAPPING));
        ON_ERROR0(_vscEP_Buffer_LoadIoRegMapping(pEPBuf, pPrivOutputEntry->pOutput));
    }
    else
    {
        pPrivOutputEntry->pOutput = gcvNULL;
    }

OnError:
    return errCode;
}

/* Load priv output mapping. */
static VSC_ErrCode
_vscEP_Buffer_LoadPrivOutputMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_PRIV_OUTPUT_MAPPING*pPrivOutputMapping
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_readUint(pIoBuf, &pPrivOutputMapping->countOfEntries);
    if (pPrivOutputMapping->countOfEntries != 0)
    {
        VSC_EP_ALLOC_MEM(pPrivOutputMapping->pPrivOutputEntries,
                         SHADER_PRIV_OUTPUT_ENTRY,
                         sizeof(SHADER_PRIV_OUTPUT_ENTRY) * pPrivOutputMapping->countOfEntries);
        for (i = 0; i < pPrivOutputMapping->countOfEntries; i++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadPrivOutputEntry(pEPBuf, &pPrivOutputMapping->pPrivOutputEntries[i]));
        }
    }
    else
    {
        pPrivOutputMapping->pPrivOutputEntries = gcvNULL;
    }

OnError:
    return errCode;
}

/* Load dynamic priv mapping. */
static VSC_ErrCode
_vscEP_Buffer_LoadDynamicPrivMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_DYNAMIC_PRIV_MAPPING* pDynamicPrivMapping
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT uVal = 0;

    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != DPMSTART_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid shader signature 0x%x.", uVal);
    }

    /* Load priv sampler mapping. */
    ON_ERROR0(_vscEP_Buffer_LoadPrivSamplerMapping(pEPBuf, &pDynamicPrivMapping->privSamplerMapping));

    /* Load priv output mapping. */
    ON_ERROR0(_vscEP_Buffer_LoadPrivOutputMapping(pEPBuf, &pDynamicPrivMapping->privOutputMapping));

    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != DPMEND_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid shader signature 0x%x.", uVal);
    }

OnError:
    return errCode;
}

/* Load default UBO mapping. */
static VSC_ErrCode
_vscEP_Buffer_LoadDefaultUboMemberEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_DEFAULT_UBO_MEMBER_ENTRY* pDefaultUboMemberEntry
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_readUint(pIoBuf, &pDefaultUboMemberEntry->memberIndexInOtherEntryTable);
    VSC_IO_readUint(pIoBuf, (gctUINT *)&pDefaultUboMemberEntry->memberKind);
    VSC_IO_readUint(pIoBuf, &pDefaultUboMemberEntry->offsetInByte);

    return errCode;
}

static VSC_ErrCode
_vscEP_Buffer_LoadDefaultUboMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    SHADER_DEFAULT_UBO_MAPPING* pDefaultUboMapping
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT uVal = 0;
    gctUINT i;

    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != DUBOSTART_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid shader signature 0x%x.", uVal);
    }

    VSC_IO_readUint(pIoBuf, &pDefaultUboMapping->baseAddressIndexInPrivConstTable);
    VSC_IO_readUint(pIoBuf, &pDefaultUboMapping->countOfEntries);
    VSC_IO_readUint(pIoBuf, &pDefaultUboMapping->sizeInByte);

    if (pDefaultUboMapping->countOfEntries > 0)
    {
        VSC_EP_ALLOC_MEM(pDefaultUboMapping->pDefaultUboMemberEntries,
                         SHADER_DEFAULT_UBO_MEMBER_ENTRY,
                         sizeof(SHADER_DEFAULT_UBO_MEMBER_ENTRY) * pDefaultUboMapping->countOfEntries);
        for (i = 0; i < pDefaultUboMapping->countOfEntries; i++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadDefaultUboMemberEntry(pEPBuf, &pDefaultUboMapping->pDefaultUboMemberEntries[i]));
        }
    }
    else
    {
        pDefaultUboMapping->pDefaultUboMemberEntries = gcvNULL;
    }

    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != DUBOEND_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid shader signature 0x%x.", uVal);
    }

OnError:
    return errCode;
}

/* Load the image derived information. */
static void
_vscEP_Buffer_LoadImageDerivedInfo(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_IMAGE_DERIVED_INFO* pImageDerivedInfo
    )
{
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT uVal = 0;

    /* Load the image size. */
    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal == 1)
    {
        VSC_EP_ALLOC_MEM(pImageDerivedInfo->pImageSize, SHADER_PRIV_CONSTANT_ENTRY, sizeof(SHADER_PRIV_CONSTANT_ENTRY));
        _vscEP_Buffer_LoadPrivConstEntry(pEPBuf, pImageDerivedInfo->pImageSize);
    }
    else
    {
        pImageDerivedInfo->pImageSize = gcvNULL;
    }

    /* Load the extra layer image. */
    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal == 1)
    {
        VSC_EP_ALLOC_MEM(pImageDerivedInfo->pExtraLayer, SHADER_PRIV_UAV_ENTRY, sizeof(SHADER_PRIV_UAV_ENTRY));
        _vscEP_Buffer_LoadPrivUavEntry(pEPBuf, pImageDerivedInfo->pExtraLayer);
    }
    else
    {
        pImageDerivedInfo->pExtraLayer = gcvNULL;
    }

    /* Load the image format. */
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pImageDerivedInfo->imageFormatInfo.imageFormat);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pImageDerivedInfo->imageFormatInfo.bSetInSpriv);
}

/****************************************Load PEP-related functions****************************************/
/****************************************GL-related functions****************************************/
/* Load attribute table entry. */
static VSC_ErrCode
_vscEP_Buffer_LoadProgAttrEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_ATTRIBUTE_TABLE_ENTRY* pProgAttrEntry
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctPOINTER* pData = (gctPOINTER*)&pProgAttrEntry->name;
    gctUINT uVal = 0;

    VSC_IO_readUint(pIoBuf, (gctUINT*)&pProgAttrEntry->type);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pProgAttrEntry->resEntryBit);
    VSC_IO_readUint(pIoBuf, &pProgAttrEntry->nameLength);
    VSC_EP_ALLOC_MEM(*pData, gctCHAR, pProgAttrEntry->nameLength + 1);
    VSC_IO_readBlock(pIoBuf, (gctCHAR*)pProgAttrEntry->name, pProgAttrEntry->nameLength + 1);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pProgAttrEntry->arraySize);
    VSC_IO_readUint(pIoBuf, &pProgAttrEntry->attribEntryIndex);

    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != 0)
    {
        VSC_EP_ALLOC_MEM(pProgAttrEntry->pIoRegMapping,
                         SHADER_IO_REG_MAPPING,
                         sizeof(SHADER_IO_REG_MAPPING));
        ON_ERROR0(_vscEP_Buffer_LoadIoRegMapping(pEPBuf, pProgAttrEntry->pIoRegMapping));
    }
    else
    {
        pProgAttrEntry->pIoRegMapping = gcvNULL;
    }

    VSC_IO_readUint(pIoBuf, &pProgAttrEntry->locationCount);
    if (pProgAttrEntry->locationCount != 0)
    {
        VSC_EP_ALLOC_MEM(pProgAttrEntry->pLocation,
                         gctUINT,
                         sizeof(gctUINT) * pProgAttrEntry->locationCount);
        VSC_IO_readBlock(pIoBuf, (gctCHAR*)pProgAttrEntry->pLocation, sizeof(gctUINT) * pProgAttrEntry->locationCount);
    }
    else
    {
        pProgAttrEntry->pLocation = gcvNULL;
    }

    VSC_IO_readUint(pIoBuf, &pProgAttrEntry->vec4BasedCount);
    VSC_IO_readUint(pIoBuf, &pProgAttrEntry->activeVec4Mask);

OnError:
    return errCode;
}

/* Load attribute table. */
static VSC_ErrCode
_vscEP_Buffer_LoadProgAttrTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_ATTRIBUTE_TABLE*   pProgAttrTable
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT     uVal;
    gctUINT i;

    /* Start signature. */
    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != PATSTART_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid program signature 0x%x.", uVal);
    }

    VSC_IO_readUint(pIoBuf, &pProgAttrTable->countOfEntries);
    if (pProgAttrTable->countOfEntries != 0)
    {
        VSC_EP_ALLOC_MEM(pProgAttrTable->pAttribEntries,
                         PROG_ATTRIBUTE_TABLE_ENTRY,
                         sizeof(PROG_ATTRIBUTE_TABLE_ENTRY) * pProgAttrTable->countOfEntries);
        for (i = 0; i < pProgAttrTable->countOfEntries; i++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadProgAttrEntry(pEPBuf, &pProgAttrTable->pAttribEntries[i]));
        }
    }
    else
    {
        pProgAttrTable->pAttribEntries = gcvNULL;
    }

    VSC_IO_readUint(pIoBuf, &pProgAttrTable->maxLengthOfName);

    /* End signature. */
    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != PATEND_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid program signature 0x%x.", uVal);
    }

OnError:
    return errCode;
}

/* Load fragment output table entry. */
static VSC_ErrCode
_vscEP_Buffer_LoadFragOutEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_FRAGOUT_TABLE_ENTRY* pFragOutEntry
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctPOINTER* pData = (gctPOINTER*)&pFragOutEntry->name;
    gctUINT uVal = 0;

    VSC_IO_readUint(pIoBuf, (gctUINT*)&pFragOutEntry->type);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pFragOutEntry->resEntryBit);
    VSC_IO_readUint(pIoBuf, &pFragOutEntry->nameLength);
    VSC_EP_ALLOC_MEM(*pData, gctCHAR, pFragOutEntry->nameLength + 1);
    VSC_IO_readBlock(pIoBuf, (gctCHAR*)pFragOutEntry->name, pFragOutEntry->nameLength + 1);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pFragOutEntry->usage);
    VSC_IO_readUint(pIoBuf, &pFragOutEntry->fragOutEntryIndex);

    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != 0)
    {
        VSC_EP_ALLOC_MEM(pFragOutEntry->pIoRegMapping,
                         SHADER_IO_REG_MAPPING,
                         sizeof(SHADER_IO_REG_MAPPING));
        ON_ERROR0(_vscEP_Buffer_LoadIoRegMapping(pEPBuf, pFragOutEntry->pIoRegMapping));
    }
    else
    {
        pFragOutEntry->pIoRegMapping = gcvNULL;
    }

    VSC_IO_readUint(pIoBuf, &pFragOutEntry->locationCount);
    if (pFragOutEntry->locationCount != 0)
    {
        VSC_EP_ALLOC_MEM(pFragOutEntry->pLocation,
                         gctUINT,
                         sizeof(gctUINT) * pFragOutEntry->locationCount);
        VSC_IO_readBlock(pIoBuf, (gctCHAR*)pFragOutEntry->pLocation, sizeof(gctUINT) * pFragOutEntry->locationCount);
    }
    VSC_IO_readUint(pIoBuf, &pFragOutEntry->vec4BasedCount);
    VSC_IO_readUint(pIoBuf, &pFragOutEntry->activeVec4Mask);

OnError:
    return errCode;
}

/* Load fragment output table. */
static VSC_ErrCode
_vscEP_Buffer_LoadFragOutTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_FRAGOUT_TABLE*     pFragOutTable
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT     uVal;
    gctUINT i;

    /* Start signature. */
    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != PFOSTART_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid program signature 0x%x.", uVal);
    }

    VSC_IO_readUint(pIoBuf, &pFragOutTable->countOfEntries);
    if (pFragOutTable->countOfEntries != 0)
    {
        VSC_EP_ALLOC_MEM(pFragOutTable->pFragOutEntries,
                         PROG_FRAGOUT_TABLE_ENTRY,
                         sizeof(PROG_FRAGOUT_TABLE_ENTRY) * pFragOutTable->countOfEntries);
        for (i = 0; i < pFragOutTable->countOfEntries; i++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadFragOutEntry(pEPBuf, &pFragOutTable->pFragOutEntries[i]));
        }
    }
    else
    {
        pFragOutTable->pFragOutEntries = gcvNULL;
    }

    /* End signature. */
    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != PFOEND_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid program signature 0x%x.", uVal);
    }

OnError:
    return errCode;
}

/* Load GL uniform HW sub mapping. */
static VSC_ErrCode
_vscEP_Buffer_LoadGLUniformHwSubMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_GL_UNIFORM_HW_SUB_MAPPING* pGLUniformHwSubMapping
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT uVal = 0;

    VSC_IO_readUint(pIoBuf, &pGLUniformHwSubMapping->startIdx);
    VSC_IO_readUint(pIoBuf, &pGLUniformHwSubMapping->rangeOfSubArray);
    VSC_IO_readUint(pIoBuf, &pGLUniformHwSubMapping->validChannelMask);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pGLUniformHwSubMapping->hwSubMappingMode);

    switch (pGLUniformHwSubMapping->hwSubMappingMode)
    {
    case GL_UNIFORM_HW_SUB_MAPPING_MODE_CONSTANT:
        VSC_IO_readUint(pIoBuf, &uVal);
        if (uVal != 0)
        {
            VSC_EP_ALLOC_MEM(pGLUniformHwSubMapping->hwMapping.pSubCBMapping,
                             SHADER_CONSTANT_SUB_ARRAY_MAPPING,
                             sizeof(SHADER_CONSTANT_SUB_ARRAY_MAPPING));
            ON_ERROR0(_vscEP_Buffer_LoadConstSubArrayMapping(pEPBuf, pGLUniformHwSubMapping->hwMapping.pSubCBMapping));
        }
        else
        {
            pGLUniformHwSubMapping->hwMapping.pSubCBMapping = gcvNULL;
        }
        break;

    case GL_UNIFORM_HW_SUB_MAPPING_MODE_SAMPLER:
        VSC_IO_readUint(pIoBuf, &uVal);
        if (uVal != 0)
        {
            VSC_EP_ALLOC_MEM(pGLUniformHwSubMapping->hwMapping.pSamplerMapping,
                             SHADER_SAMPLER_SLOT_MAPPING,
                             sizeof(SHADER_SAMPLER_SLOT_MAPPING));
            ON_ERROR0(_vscEP_Buffer_LoadSamplerSlotMapping(pEPBuf, pGLUniformHwSubMapping->hwMapping.pSamplerMapping));
        }
        else
        {
            pGLUniformHwSubMapping->hwMapping.pSamplerMapping = gcvNULL;
        }
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

OnError:
    return errCode;
}

/* Load GL uniform HW mapping. */
static VSC_ErrCode
_vscEP_Buffer_LoadGLUniformHwMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_GL_UNIFORM_HW_MAPPING* pGLUniformHwMapping
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_readUint(pIoBuf, &pGLUniformHwMapping->countOfHWSubMappings);

    if (pGLUniformHwMapping->countOfHWSubMappings != 0)
    {
        VSC_EP_ALLOC_MEM(pGLUniformHwMapping->pHWSubMappings,
                         PROG_GL_UNIFORM_HW_SUB_MAPPING,
                         sizeof(PROG_GL_UNIFORM_HW_SUB_MAPPING) * pGLUniformHwMapping->countOfHWSubMappings);
        for (i = 0; i < pGLUniformHwMapping->countOfHWSubMappings; i++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadGLUniformHwSubMapping(pEPBuf, &pGLUniformHwMapping->pHWSubMappings[i]));
        }
    }
    else
    {
        pGLUniformHwMapping->pHWSubMappings = gcvNULL;
    }

OnError:
    return errCode;
}

/* Load GL uniform common entry. */
static VSC_ErrCode
_vscEP_Buffer_LoadGLUniformCommonEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_GL_UNIFORM_COMMON_ENTRY* pGLUniformCommonEntry
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctPOINTER* pData = (gctPOINTER*)pGLUniformCommonEntry->name;
    gctUINT i;

    VSC_IO_readUint(pIoBuf, (gctUINT*)&pGLUniformCommonEntry->type);
    VSC_IO_readUint(pIoBuf, &pGLUniformCommonEntry->nameLength);
    VSC_EP_ALLOC_MEM(*pData, gctCHAR, pGLUniformCommonEntry->nameLength + 1);
    VSC_IO_readBlock(pIoBuf, (gctCHAR*)pGLUniformCommonEntry->name, pGLUniformCommonEntry->nameLength + 1);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pGLUniformCommonEntry->precision);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pGLUniformCommonEntry->arraySize);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pGLUniformCommonEntry->activeArraySize);
    VSC_IO_readUint(pIoBuf, &pGLUniformCommonEntry->uniformEntryIndex);
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        ON_ERROR0(_vscEP_Buffer_LoadGLUniformHwMapping(pEPBuf, &pGLUniformCommonEntry->hwMappings[i]));
    }

OnError:
    return errCode;
}

/* Load GL uniform table entry. */
static VSC_ErrCode
_vscEP_Buffer_LoadGLUniformTableEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_GL_UNIFORM_TABLE_ENTRY* pGLUniformTableEntry
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    ON_ERROR0(_vscEP_Buffer_LoadGLUniformCommonEntry(pEPBuf, &pGLUniformTableEntry->common));
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pGLUniformTableEntry->usage);
    VSC_IO_readUint(pIoBuf, &pGLUniformTableEntry->locationBase);

OnError:
    return errCode;
}

/* Load GL uniform table entry. */
static VSC_ErrCode
_vscEP_Buffer_LoadGLUniformTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_GL_UNIFORM_TABLE*  pGLUniformTable
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_readUint(pIoBuf, &pGLUniformTable->countOfEntries);

    if (pGLUniformTable->countOfEntries != 0)
    {
        VSC_EP_ALLOC_MEM(pGLUniformTable->pUniformEntries,
                         PROG_GL_UNIFORM_TABLE_ENTRY,
                         sizeof(PROG_GL_UNIFORM_TABLE_ENTRY) * pGLUniformTable->countOfEntries);
        for (i = 0; i < pGLUniformTable->countOfEntries; i++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadGLUniformTableEntry(pEPBuf, &pGLUniformTable->pUniformEntries[i]));
        }
    }
    else
    {
        pGLUniformTable->pUniformEntries = gcvNULL;
    }

    VSC_IO_readUint(pIoBuf, &pGLUniformTable->firstBuiltinUniformEntryIndex);
    VSC_IO_readUint(pIoBuf, &pGLUniformTable->firstUserUniformEntryIndex);
    VSC_IO_readUint(pIoBuf, &pGLUniformTable->maxLengthOfName);

OnError:
    return errCode;
}

/* Load GL uniformblock uniform entry. */
static VSC_ErrCode
_vscEP_Buffer_LoadGLUniformBlockUniformEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_GL_UNIFORMBLOCK_UNIFORM_ENTRY* pGLUniformBlockUniformEntry
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    ON_ERROR0(_vscEP_Buffer_LoadGLUniformCommonEntry(pEPBuf, &pGLUniformBlockUniformEntry->common));
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pGLUniformBlockUniformEntry->offset);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pGLUniformBlockUniformEntry->arrayStride);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pGLUniformBlockUniformEntry->matrixMajor);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pGLUniformBlockUniformEntry->matrixMajor);

OnError:
    return errCode;
}

/* Load GL uniformblock entry. */
static VSC_ErrCode
_vscEP_Buffer_LoadGLUniformBlockEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_GL_UNIFORMBLOCK_TABLE_ENTRY* pGLUniformBlockEntry
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_readUint(pIoBuf, &pGLUniformBlockEntry->countOfEntries);

    if (pGLUniformBlockEntry->countOfEntries != 0)
    {
        VSC_EP_ALLOC_MEM(pGLUniformBlockEntry->pUniformEntries,
                         PROG_GL_UNIFORMBLOCK_UNIFORM_ENTRY,
                         sizeof(PROG_GL_UNIFORMBLOCK_UNIFORM_ENTRY) * pGLUniformBlockEntry->countOfEntries);
        for (i = 0; i < pGLUniformBlockEntry->countOfEntries; i++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadGLUniformBlockUniformEntry(pEPBuf, &pGLUniformBlockEntry->pUniformEntries[i]));
        }
    }
    else
    {
        pGLUniformBlockEntry->pUniformEntries = gcvNULL;
    }

    VSC_EP_ALLOC_MEM(pGLUniformBlockEntry->pUniformIndices, gctUINT, sizeof(gctUINT));
    VSC_IO_readBlock(pIoBuf, (gctCHAR*)pGLUniformBlockEntry->pUniformIndices, sizeof(gctUINT));
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pGLUniformBlockEntry->ubEntryIndex);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pGLUniformBlockEntry->dataSizeInByte);
    VSC_IO_readBlock(pIoBuf, (gctCHAR*)pGLUniformBlockEntry->bRefedByShader, sizeof(gctBOOL) * VSC_MAX_SHADER_STAGE_COUNT);

OnError:
    return errCode;
}

/* Load GL uniformblock table. */
static VSC_ErrCode
_vscEP_Buffer_LoadGLUniformBlockTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_GL_UNIFORMBLOCK_TABLE* pGLUniformBlockTable
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_readUint(pIoBuf, &pGLUniformBlockTable->countOfEntries);

    if (pGLUniformBlockTable->countOfEntries != 0)
    {
        VSC_EP_ALLOC_MEM(pGLUniformBlockTable->pUniformBlockEntries,
                         PROG_GL_UNIFORMBLOCK_TABLE_ENTRY,
                         sizeof(PROG_GL_UNIFORMBLOCK_TABLE_ENTRY) * pGLUniformBlockTable->countOfEntries);
        for (i = 0; i < pGLUniformBlockTable->countOfEntries; i++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadGLUniformBlockEntry(pEPBuf, &pGLUniformBlockTable->pUniformBlockEntries[i]));
        }
    }
    else
    {
        pGLUniformBlockTable->pUniformBlockEntries = gcvNULL;
    }

    VSC_IO_readUint(pIoBuf, &pGLUniformBlockTable->maxLengthOfName);

OnError:
    return errCode;
}

/* Load XFB out table entry. */
static VSC_ErrCode
_vscEP_Buffer_LoadXfbOutEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_GL_XFB_OUT_TABLE_ENTRY* pXfbOutEntry
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctPOINTER* pData = (gctPOINTER*)&pXfbOutEntry->name;
    gctUINT uVal = 0;

    VSC_IO_readUint(pIoBuf, (gctUINT*)&pXfbOutEntry->type);
    VSC_IO_readUint(pIoBuf, &pXfbOutEntry->nameLength);
    VSC_EP_ALLOC_MEM(*pData,
                     gctCHAR,
                     pXfbOutEntry->nameLength + 1);
    VSC_IO_readBlock(pIoBuf, (gctCHAR*)pXfbOutEntry->name, pXfbOutEntry->nameLength + 1);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pXfbOutEntry->arraySize);
    VSC_IO_readUint(pIoBuf, &pXfbOutEntry->fxOutEntryIndex);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pXfbOutEntry->fxStreamingMode);

    switch (pXfbOutEntry->fxStreamingMode)
    {
    case GL_FX_STREAMING_MODE_FFU:
        VSC_IO_readUint(pIoBuf, &uVal);

        if (uVal != 0)
        {
            VSC_EP_ALLOC_MEM(pXfbOutEntry->u.s.pIoRegMapping,
                             SHADER_IO_REG_MAPPING,
                             sizeof(SHADER_IO_REG_MAPPING));
            ON_ERROR0(_vscEP_Buffer_LoadIoRegMapping(pEPBuf, pXfbOutEntry->u.s.pIoRegMapping));
        }
        else
        {
            pXfbOutEntry->u.s.pIoRegMapping = gcvNULL;
        }
        VSC_IO_readUint(pIoBuf, &pXfbOutEntry->u.s.vec4BasedCount);
        VSC_IO_readUint(pIoBuf, &pXfbOutEntry->u.s.activeVec4Mask);
        break;

    case GL_FX_STREAMING_MODE_SHADER:
        VSC_IO_readUint(pIoBuf, &uVal);

        if (uVal != 0)
        {
            VSC_EP_ALLOC_MEM(pXfbOutEntry->u.pSoBufferMapping,
                             SHADER_UAV_SLOT_MAPPING,
                             sizeof(SHADER_UAV_SLOT_MAPPING));
            ON_ERROR0(_vscEP_Buffer_LoadUavSlotMapping(pEPBuf, pXfbOutEntry->u.pSoBufferMapping));
        }
        else
        {
            pXfbOutEntry->u.pSoBufferMapping = gcvNULL;
        }
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

OnError:
    return errCode;
}

/* Load XFB table. */
static VSC_ErrCode
_vscEP_Buffer_LoadXfbOutTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_GL_XFB_OUT_TABLE*  pXfbOutTable
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, pXfbOutTable->countOfEntries);

    if (pXfbOutTable->countOfEntries != 0)
    {
        VSC_EP_ALLOC_MEM(pXfbOutTable->pFxOutEntries,
                         PROG_GL_XFB_OUT_TABLE_ENTRY,
                         sizeof(PROG_GL_XFB_OUT_TABLE_ENTRY) * pXfbOutTable->countOfEntries);
        for (i = 0; i < pXfbOutTable->countOfEntries; i++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadXfbOutEntry(pEPBuf, &pXfbOutTable->pFxOutEntries[i]));
        }
    }
    else
    {
        pXfbOutTable->pFxOutEntries = gcvNULL;
    }

    VSC_IO_writeUint(pIoBuf, pXfbOutTable->mode);
    VSC_IO_writeUint(pIoBuf, pXfbOutTable->maxLengthOfName);

OnError:
    return errCode;
}

/* Load GL high level mapping table. */
static VSC_ErrCode
_vscEP_Buffer_LoadGLHighLevelMappingTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROGRAM_EXECUTABLE_PROFILE* pPEP
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT     uVal;

    /* Start signature. */
    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != GLMSTART_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid program signature 0x%x.", uVal);
    }

    /* Load GL uniform table. */
    ON_ERROR0(_vscEP_Buffer_LoadGLUniformTable(pEPBuf, &pPEP->u.gl.uniformTable));

    /* Load GL uniformblock table. */
    ON_ERROR0(_vscEP_Buffer_LoadGLUniformBlockTable(pEPBuf, &pPEP->u.gl.uniformBlockTable));

    /* Load GL XFB out table. */
    ON_ERROR0(_vscEP_Buffer_LoadXfbOutTable(pEPBuf, &pPEP->u.gl.fxOutTable));

    /* End signature. */
    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != GLMEND_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid program signature 0x%x.", uVal);
    }

OnError:
    return errCode;
}

/****************************************Vulkan-related functions****************************************/
/* Load resource binding. */
static VSC_ErrCode
_vscEP_Buffer_LoadShaderResourceBinding(
    VSC_EP_IO_BUFFER*       pEPBuf,
    VSC_SHADER_RESOURCE_BINDING* pShaderResourceBinding
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_readUint(pIoBuf, (gctUINT*)&pShaderResourceBinding->type);
    VSC_IO_readUint(pIoBuf, &pShaderResourceBinding->set);
    VSC_IO_readUint(pIoBuf, &pShaderResourceBinding->binding);
    VSC_IO_readUint(pIoBuf, &pShaderResourceBinding->arraySize);

    return errCode;
}

/* Load private combined texture sampler hw mapping list. */
static VSC_ErrCode
_vscEP_Buffer_LoadVKPrivCombTexSampHwMappingList(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_PRIV_COMB_TEX_SAMP_HW_MAPPING_LIST* pPrivCombTexSampHwMappingList
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_readUint(pIoBuf, &pPrivCombTexSampHwMappingList->arraySize);

    if (pPrivCombTexSampHwMappingList->arraySize != 0)
    {
        VSC_EP_ALLOC_MEM(pPrivCombTexSampHwMappingList->pPctsHmEntryIdxArray,
                         gctUINT,
                         sizeof(gctUINT) * pPrivCombTexSampHwMappingList->arraySize);
        VSC_IO_readBlock(pIoBuf,
                         (gctCHAR*)pPrivCombTexSampHwMappingList->pPctsHmEntryIdxArray,
                         sizeof(gctUINT) * pPrivCombTexSampHwMappingList->arraySize);
    }
    else
    {
        pPrivCombTexSampHwMappingList->pPctsHmEntryIdxArray = gcvNULL;
    }

    return errCode;
}

/* Load private combined texture sampler hw mapping list. */
static VSC_ErrCode
_vscEP_Buffer_LoadVkCombTexSampHwMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    gctUINT                 shaderStageIndex,
    PROG_VK_COMBINED_TEXTURE_SAMPLER_HW_MAPPING* pCombTsHwMapping,
    gctUINT                 ArraySize
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;
    gctUINT uVal = 0;
    SHADER_EXECUTABLE_PROFILE* pSep = &pEPBuf->epData.pPEP->seps[shaderStageIndex];

    ON_ERROR0(_vscEP_Buffer_LoadSamplerSlotMapping(pEPBuf, &pCombTsHwMapping->samplerMapping));

    VSC_IO_readUint(pIoBuf, &uVal);

    if (uVal != 0 && ArraySize != 0)
    {
        VSC_EP_ALLOC_MEM(pCombTsHwMapping->ppExtraSamplerArray,
                         SHADER_PRIV_SAMPLER_ENTRY*,
                         sizeof(SHADER_PRIV_SAMPLER_ENTRY*) * ArraySize);
        for (i = 0; i < ArraySize; i++)
        {
            VSC_EP_ALLOC_MEM(pCombTsHwMapping->ppExtraSamplerArray[i],
                             SHADER_PRIV_SAMPLER_ENTRY,
                             sizeof(SHADER_PRIV_SAMPLER_ENTRY) * ArraySize);
            ON_ERROR0(_vscEP_Buffer_LoadPrivSamplerEntry(pEPBuf, pCombTsHwMapping->ppExtraSamplerArray[i]));
        }
    }
    else
    {
        pCombTsHwMapping->ppExtraSamplerArray = gcvNULL;
    }

    for (i = 0; i < __YCBCR_PLANE_COUNT__; i++)
    {
        VSC_IO_readUint(pIoBuf, &uVal);
        if (uVal != 0)
        {
            VSC_IO_readUint(pIoBuf, &uVal);
            pCombTsHwMapping->pYcbcrPlanes[i] = &pSep->staticPrivMapping.privUavMapping.pPrivUavEntries[uVal];
        }
        else
        {
            pCombTsHwMapping->pYcbcrPlanes[i] = gcvNULL;
        }
    }

    ON_ERROR0(_vscEP_Buffer_LoadResourceSlotMapping(pEPBuf, &pCombTsHwMapping->texMapping));
    ON_ERROR0(_vscEP_Buffer_LoadVKPrivCombTexSampHwMappingList(pEPBuf, &pCombTsHwMapping->samplerHwMappingList));
    ON_ERROR0(_vscEP_Buffer_LoadVKPrivCombTexSampHwMappingList(pEPBuf, &pCombTsHwMapping->texHwMappingList));

OnError:
    return errCode;
}

/* Load Vulkan combined texture-sampler entry. */
static VSC_ErrCode
_vscEP_Buffer_LoadVKCombinedTextureSamplerEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_COMBINED_TEX_SAMPLER_TABLE_ENTRY* pCombTsEntry
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i, j;
    gctUINT entryMask = 0;

    ON_ERROR0(_vscEP_Buffer_LoadShaderResourceBinding(pEPBuf, &pCombTsEntry->combTsBinding));
    VSC_IO_readUint(pIoBuf, &pCombTsEntry->combTsEntryIndex);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pCombTsEntry->stageBits);
    VSC_IO_readUint(pIoBuf, &pCombTsEntry->activeStageMask);

    /* Load texture size. */
    entryMask = 0;
    VSC_IO_readUint(pIoBuf, &entryMask);
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (entryMask & (1 << (i * 2 + j)))
            {
                VSC_EP_ALLOC_MEM(pCombTsEntry->pTextureSize[i][j],
                                 SHADER_PRIV_CONSTANT_ENTRY,
                                 sizeof(SHADER_PRIV_CONSTANT_ENTRY));
                ON_ERROR0(_vscEP_Buffer_LoadPrivConstEntry(pEPBuf, pCombTsEntry->pTextureSize[i][j]));
            }
            else
            {
                pCombTsEntry->pTextureSize[i][j] = gcvNULL;
            }
        }
    }

    /* Load lodMinMax */
    entryMask = 0;
    VSC_IO_readUint(pIoBuf, &entryMask);
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (entryMask & (1 << (i * 2 + j)))
            {
                VSC_EP_ALLOC_MEM(pCombTsEntry->pLodMinMax[i][j],
                                 SHADER_PRIV_CONSTANT_ENTRY,
                                 sizeof(SHADER_PRIV_CONSTANT_ENTRY));
                ON_ERROR0(_vscEP_Buffer_LoadPrivConstEntry(pEPBuf, pCombTsEntry->pLodMinMax[i][j]));
            }
            else
            {
                pCombTsEntry->pLodMinMax[i][j] = gcvNULL;
            }
        }
    }

    /* Load levels samples. */
    entryMask = 0;
    VSC_IO_readUint(pIoBuf, &entryMask);
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (entryMask & (1 << (i * 2 + j)))
            {
                VSC_EP_ALLOC_MEM(pCombTsEntry->pLevelsSamples[i][j],
                                 SHADER_PRIV_CONSTANT_ENTRY,
                                 sizeof(SHADER_PRIV_CONSTANT_ENTRY));
                ON_ERROR0(_vscEP_Buffer_LoadPrivConstEntry(pEPBuf, pCombTsEntry->pLevelsSamples[i][j]));
            }
            else
            {
                pCombTsEntry->pLevelsSamples[i][j] = gcvNULL;
            }
        }
    }

    if (pCombTsEntry->combTsBinding.arraySize != 0)
    {
        VSC_EP_ALLOC_MEM(pCombTsEntry->pResOpBits,
                         VSC_RES_OP_BIT,
                         sizeof(gctUINT) * pCombTsEntry->combTsBinding.arraySize);
        VSC_IO_readBlock(pIoBuf, (gctCHAR*)pCombTsEntry->pResOpBits, sizeof(gctUINT) * pCombTsEntry->combTsBinding.arraySize);
    }
    else
    {
        pCombTsEntry->pResOpBits = gcvNULL;
    }

    /* Load hw mapping. */
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        ON_ERROR0(_vscEP_Buffer_LoadVkCombTexSampHwMapping(pEPBuf, i, &pCombTsEntry->hwMappings[i], pCombTsEntry->combTsBinding.arraySize));
    }

    /* Load the sampled image index. */
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        VSC_IO_readUint(pIoBuf, &pCombTsEntry->sampledImageIndexInStorageTable[i]);
    }

OnError:
    return errCode;
}

/* Load Vulkan combined texture-sampler table. */
static VSC_ErrCode
_vscEP_Buffer_LoadVKCombinedTextureSamplerTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_COMBINED_TEXTURE_SAMPLER_TABLE* pCombinedSampTexTable
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_readUint(pIoBuf, &pCombinedSampTexTable->countOfEntries);

    if (pCombinedSampTexTable->countOfEntries != 0)
    {
        VSC_EP_ALLOC_MEM(pCombinedSampTexTable->pCombTsEntries,
                         PROG_VK_COMBINED_TEX_SAMPLER_TABLE_ENTRY,
                         sizeof(PROG_VK_COMBINED_TEX_SAMPLER_TABLE_ENTRY) * pCombinedSampTexTable->countOfEntries);
        for (i = 0; i < pCombinedSampTexTable->countOfEntries; i++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadVKCombinedTextureSamplerEntry(pEPBuf, &pCombinedSampTexTable->pCombTsEntries[i]));
        }
    }
    else
    {
        pCombinedSampTexTable->pCombTsEntries = gcvNULL;
    }

OnError:
    return errCode;
}

/* Load Vulkan separated sampler HW mapping. */
static VSC_ErrCode
_vscEP_Buffer_LoadVKSeparatedSamplerHwMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_SEPARATED_SAMPLER_HW_MAPPING* pSeparatedSamplerHwMapping
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    ON_ERROR0(_vscEP_Buffer_LoadVKPrivCombTexSampHwMappingList(pEPBuf, &pSeparatedSamplerHwMapping->samplerHwMappingList));

OnError:
    return errCode;
}

/* Load Vulkan separated sampler table entry. */
static VSC_ErrCode
_vscEP_Buffer_LoadVKSeparatedSamplerEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_SEPARATED_SAMPLER_TABLE_ENTRY* pSeparatedSamplerEntry
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    ON_ERROR0(_vscEP_Buffer_LoadShaderResourceBinding(pEPBuf, &pSeparatedSamplerEntry->samplerBinding));

    VSC_IO_readUint(pIoBuf, &pSeparatedSamplerEntry->samplerEntryIndex);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pSeparatedSamplerEntry->stageBits);
    VSC_IO_readUint(pIoBuf, &pSeparatedSamplerEntry->activeStageMask);

    if (pSeparatedSamplerEntry->samplerBinding.arraySize != 0)
    {
        VSC_EP_ALLOC_MEM(pSeparatedSamplerEntry->pResOpBits,
                         VSC_RES_OP_BIT,
                         sizeof(gctUINT) * pSeparatedSamplerEntry->samplerBinding.arraySize);
        VSC_IO_readBlock(pIoBuf,
                         (gctCHAR*)pSeparatedSamplerEntry->pResOpBits,
                         sizeof(gctUINT) * pSeparatedSamplerEntry->samplerBinding.arraySize);
    }
    else
    {
        pSeparatedSamplerEntry->pResOpBits = gcvNULL;
    }

    VSC_IO_readUint(pIoBuf, (gctUINT*)&pSeparatedSamplerEntry->bUsingHwMppingList);

    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        ON_ERROR0(_vscEP_Buffer_LoadVKSeparatedSamplerHwMapping(pEPBuf, &pSeparatedSamplerEntry->hwMappings[i]));
    }

OnError:
    return errCode;
}

/* Load Vulkan separated sampler table. */
static VSC_ErrCode
_vscEP_Buffer_LoadVKSeparatedSamplerTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_SEPARATED_SAMPLER_TABLE* pSeparatedSamplerTable
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, pSeparatedSamplerTable->countOfEntries);

    if (pSeparatedSamplerTable->countOfEntries != 0)
    {
        VSC_EP_ALLOC_MEM(pSeparatedSamplerTable->pSamplerEntries,
                         PROG_VK_SEPARATED_SAMPLER_TABLE_ENTRY,
                         sizeof(PROG_VK_SEPARATED_SAMPLER_TABLE_ENTRY) * pSeparatedSamplerTable->countOfEntries);
        for (i = 0; i < pSeparatedSamplerTable->countOfEntries; i++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadVKSeparatedSamplerEntry(pEPBuf, &pSeparatedSamplerTable->pSamplerEntries[i]));
        }
    }
    else
    {
        pSeparatedSamplerTable->pSamplerEntries = gcvNULL;
    }

OnError:
    return errCode;
}

/* Load Vulkan separated texture HW mapping. */
static VSC_ErrCode
_vscEP_Buffer_LoadVKSeparatedTextureHwMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_SEPARATED_TEXTURE_HW_MAPPING* pSeparatedTextureHwMapping
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    /* Load the image derived information. */
    _vscEP_Buffer_LoadImageDerivedInfo(pEPBuf, &pSeparatedTextureHwMapping->s.imageDerivedInfo);
    ON_ERROR0(_vscEP_Buffer_LoadUavSlotMapping(pEPBuf, &pSeparatedTextureHwMapping->s.hwMapping));
    ON_ERROR0(_vscEP_Buffer_LoadVKPrivCombTexSampHwMappingList(pEPBuf, &pSeparatedTextureHwMapping->s.texHwMappingList));

OnError:
    return errCode;
}

/* Load Vulkan separated texture table entry. */
static VSC_ErrCode
_vscEP_Buffer_LoadVKSeparatedTextureEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_SEPARATED_TEXTURE_TABLE_ENTRY* pSeparatedTextureEntry
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    ON_ERROR0(_vscEP_Buffer_LoadShaderResourceBinding(pEPBuf, &pSeparatedTextureEntry->texBinding));

    VSC_IO_readUint(pIoBuf, &pSeparatedTextureEntry->textureEntryIndex);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pSeparatedTextureEntry->stageBits);
    VSC_IO_readUint(pIoBuf, &pSeparatedTextureEntry->activeStageMask);

    if (pSeparatedTextureEntry->texBinding.arraySize != 0)
    {
        VSC_EP_ALLOC_MEM(pSeparatedTextureEntry->pResOpBits,
                         VSC_RES_OP_BIT,
                         sizeof(gctUINT) * pSeparatedTextureEntry->texBinding.arraySize);
        VSC_IO_readBlock(pIoBuf,
                         (gctCHAR*)pSeparatedTextureEntry->pResOpBits,
                         sizeof(gctUINT) * pSeparatedTextureEntry->texBinding.arraySize);
    }
    else
    {
        pSeparatedTextureEntry->pResOpBits = gcvNULL;
    }

    VSC_IO_readUint(pIoBuf, (gctUINT*)&pSeparatedTextureEntry->bUsingHwMppingList);

    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        ON_ERROR0(_vscEP_Buffer_LoadVKSeparatedTextureHwMapping(pEPBuf, &pSeparatedTextureEntry->hwMappings[i]));
    }

OnError:
    return errCode;
}

/* Load Vulkan separated texture table. */
static VSC_ErrCode
_vscEP_Buffer_LoadVKSeparatedTextureTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_SEPARATED_TEXTURE_TABLE* pSeparatedTextureTable
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_writeUint(pIoBuf, pSeparatedTextureTable->countOfEntries);

    if (pSeparatedTextureTable->countOfEntries != 0)
    {
        VSC_EP_ALLOC_MEM(pSeparatedTextureTable->pTextureEntries,
                         PROG_VK_SEPARATED_TEXTURE_TABLE_ENTRY,
                         sizeof(PROG_VK_SEPARATED_TEXTURE_TABLE_ENTRY) * pSeparatedTextureTable->countOfEntries);
        for (i = 0; i < pSeparatedTextureTable->countOfEntries; i++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadVKSeparatedTextureEntry(pEPBuf, &pSeparatedTextureTable->pTextureEntries[i]));
        }
    }
    else
    {
        pSeparatedTextureTable->pTextureEntries = gcvNULL;
    }

OnError:
    return errCode;
}

/* Load Vulkan uniform texel buffer hw mapping. */
static VSC_ErrCode
_vscEP_Buffer_LoadVKUniformTexelBufferHwMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_UNIFORM_TEXEL_BUFFER_HW_MAPPING* pUniformTexelBufferHwMapping,
    gctUINT                 ArraySize
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;
    gctUINT uVal = 0;

    VSC_IO_readUint(pIoBuf, (gctUINT*)&pUniformTexelBufferHwMapping->hwMappingMode);

    switch (pUniformTexelBufferHwMapping->hwMappingMode)
    {
    case VK_UNIFORM_TEXEL_BUFFER_HW_MAPPING_MODE_NATIVELY_SUPPORT:
        ON_ERROR0(_vscEP_Buffer_LoadResourceSlotMapping(pEPBuf, &pUniformTexelBufferHwMapping->u.texMapping));
        break;

    case VK_UNIFORM_TEXEL_BUFFER_HW_MAPPING_MODE_NOT_NATIVELY_SUPPORT:
        VSC_IO_readUint(pIoBuf, (gctUINT*)&pUniformTexelBufferHwMapping->u.s.hwMemAccessMode);

        if (pUniformTexelBufferHwMapping->u.s.hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_SAMPLER)
        {
            ON_ERROR0(_vscEP_Buffer_LoadSamplerSlotMapping(pEPBuf, &pUniformTexelBufferHwMapping->u.s.hwLoc.samplerMapping));
        }
        else
        {
            gcmASSERT(pUniformTexelBufferHwMapping->u.s.hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR);

            ON_ERROR0(_vscEP_Buffer_LoadConstHwLocMapping(pEPBuf, pUniformTexelBufferHwMapping->u.s.hwLoc.pHwDirectAddrBase));
        }

        VSC_IO_readUint(pIoBuf, &uVal);

        if (uVal != 0 && ArraySize != 0)
        {
            VSC_EP_ALLOC_MEM(pUniformTexelBufferHwMapping->u.s.ppExtraSamplerArray,
                             SHADER_PRIV_SAMPLER_ENTRY*,
                             sizeof(SHADER_PRIV_SAMPLER_ENTRY*) * ArraySize);
            for (i = 0; i < ArraySize; i++)
            {
                VSC_EP_ALLOC_MEM(pUniformTexelBufferHwMapping->u.s.ppExtraSamplerArray[i],
                                 SHADER_PRIV_SAMPLER_ENTRY,
                                 sizeof(SHADER_PRIV_SAMPLER_ENTRY));
                ON_ERROR0(_vscEP_Buffer_LoadPrivSamplerEntry(pEPBuf, pUniformTexelBufferHwMapping->u.s.ppExtraSamplerArray[i]));
            }
        }
        else
        {
            pUniformTexelBufferHwMapping->u.s.ppExtraSamplerArray = gcvNULL;
        }
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

OnError:
    return errCode;
}

/* Load Vulkan uniform texel buffer entry. */
static VSC_ErrCode
_vscEP_Buffer_LoadVKUniformTexelBufferEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_UNIFORM_TEXEL_BUFFER_TABLE_ENTRY* pUniformTexelBufferEntry
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i, j;
    gctUINT entryMask = 0;

    ON_ERROR0(_vscEP_Buffer_LoadShaderResourceBinding(pEPBuf, &pUniformTexelBufferEntry->utbBinding));
    VSC_IO_readUint(pIoBuf, &pUniformTexelBufferEntry->utbEntryIndex);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pUniformTexelBufferEntry->stageBits);
    VSC_IO_readUint(pIoBuf, &pUniformTexelBufferEntry->activeStageMask);

    VSC_IO_readUint(pIoBuf, (gctUINT*)&pUniformTexelBufferEntry->utbEntryFlag);

    VSC_IO_readUint(pIoBuf, &entryMask);
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (entryMask & (1 << (i * 2 + j)))
            {
                VSC_EP_ALLOC_MEM(pUniformTexelBufferEntry->pTextureSize[i][j],
                                 SHADER_PRIV_CONSTANT_ENTRY,
                                 sizeof(SHADER_PRIV_CONSTANT_ENTRY));
                ON_ERROR0(_vscEP_Buffer_LoadPrivConstEntry(pEPBuf, pUniformTexelBufferEntry->pTextureSize[i][j]));
            }
            else
            {
                pUniformTexelBufferEntry->pTextureSize[i][j] = gcvNULL;
            }
        }
    }

    /* Load the image derived information. */
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        _vscEP_Buffer_LoadImageDerivedInfo(pEPBuf, &pUniformTexelBufferEntry->imageDerivedInfo[i]);
    }

    if (pUniformTexelBufferEntry->utbBinding.arraySize != 0)
    {
        VSC_EP_ALLOC_MEM(pUniformTexelBufferEntry->pResOpBits,
                         VSC_RES_OP_BIT,
                         sizeof(gctUINT) * pUniformTexelBufferEntry->utbBinding.arraySize);
        VSC_IO_readBlock(pIoBuf,
                         (gctCHAR*)pUniformTexelBufferEntry->pResOpBits,
                         sizeof(gctUINT) * pUniformTexelBufferEntry->utbBinding.arraySize);
    }

    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        ON_ERROR0(_vscEP_Buffer_LoadVKUniformTexelBufferHwMapping(pEPBuf,
                                                                  &pUniformTexelBufferEntry->hwMappings[i],
                                                                  pUniformTexelBufferEntry->utbBinding.arraySize));
    }

OnError:
    return errCode;
}

/* Load Vulkan uniform texel buffer table. */
static VSC_ErrCode
_vscEP_Buffer_LoadVKUniformTexelBufferTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_UNIFORM_TEXEL_BUFFER_TABLE* pUniformTexelBufferTable
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_readUint(pIoBuf, &pUniformTexelBufferTable->countOfEntries);

    if (pUniformTexelBufferTable->countOfEntries != 0)
    {
        VSC_EP_ALLOC_MEM(pUniformTexelBufferTable->pUtbEntries,
                         PROG_VK_UNIFORM_TEXEL_BUFFER_TABLE_ENTRY,
                         sizeof(PROG_VK_UNIFORM_TEXEL_BUFFER_TABLE_ENTRY) * pUniformTexelBufferTable->countOfEntries);
        for (i = 0; i < pUniformTexelBufferTable->countOfEntries; i++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadVKUniformTexelBufferEntry(pEPBuf, &pUniformTexelBufferTable->pUtbEntries[i]));
        }
    }
    else
    {
        pUniformTexelBufferTable->pUtbEntries = gcvNULL;
    }

OnError:
    return errCode;
}

/* Load Vulkan input attachment hw mapping. */
static VSC_ErrCode
_vscEP_Buffer_LoadVKInputAttachmentHwMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_INPUT_ATTACHMENT_HW_MAPPING* pInputAttachmentHwMapping,
    gctUINT32               ArraySize
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctUINT i;
    gctUINT uVal = 0;

    ON_ERROR0(_vscEP_Buffer_LoadUavSlotMapping(pEPBuf, &pInputAttachmentHwMapping->uavMapping));

    VSC_IO_readUint(pEPBuf->pIoBuf, &uVal);

    if (uVal != 0 && ArraySize != 0)
    {
        VSC_EP_ALLOC_MEM(pInputAttachmentHwMapping->ppExtraSamplerArray,
                         SHADER_PRIV_SAMPLER_ENTRY*,
                         sizeof(SHADER_PRIV_SAMPLER_ENTRY*) * ArraySize);
        for (i = 0; i < ArraySize; i++)
        {
            VSC_EP_ALLOC_MEM(pInputAttachmentHwMapping->ppExtraSamplerArray[i],
                             SHADER_PRIV_SAMPLER_ENTRY,
                             sizeof(SHADER_PRIV_SAMPLER_ENTRY) * ArraySize);
            ON_ERROR0(_vscEP_Buffer_LoadPrivSamplerEntry(pEPBuf, pInputAttachmentHwMapping->ppExtraSamplerArray[i]));
        }
    }
    else
    {
        pInputAttachmentHwMapping->ppExtraSamplerArray = gcvNULL;
    }

OnError:
    return errCode;
}

/* Load Vulkan input attachment table entry. */
static VSC_ErrCode
_vscEP_Buffer_LoadVKInutAttachmentEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_INPUT_ATTACHMENT_TABLE_ENTRY* pInputAttachmentEntry
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i, j;
    gctUINT entryMask = 0;

    ON_ERROR0(_vscEP_Buffer_LoadShaderResourceBinding(pEPBuf, &pInputAttachmentEntry->iaBinding));

    VSC_IO_readUint(pIoBuf, &pInputAttachmentEntry->iaEntryIndex);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pInputAttachmentEntry->stageBits);
    VSC_IO_readUint(pIoBuf, &pInputAttachmentEntry->activeStageMask);

    /* Load the image derived information. */
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        _vscEP_Buffer_LoadImageDerivedInfo(pEPBuf, &pInputAttachmentEntry->imageDerivedInfo[i]);
    }

    /* Load texture size. */
    entryMask = 0;
    VSC_IO_readUint(pIoBuf, &entryMask);
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (entryMask & (1 << (i * 2 + j)))
            {
                VSC_EP_ALLOC_MEM(pInputAttachmentEntry->pTextureSize[i][j],
                                 SHADER_PRIV_CONSTANT_ENTRY,
                                 sizeof(SHADER_PRIV_CONSTANT_ENTRY));
                ON_ERROR0(_vscEP_Buffer_LoadPrivConstEntry(pEPBuf, pInputAttachmentEntry->pTextureSize[i][j]));
            }
            else
            {
                pInputAttachmentEntry->pTextureSize[i][j] = gcvNULL;
            }
        }
    }

    /* Load lodMinMax */
    entryMask = 0;
    VSC_IO_readUint(pIoBuf, &entryMask);
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (entryMask & (1 << (i * 2 + j)))
            {
                VSC_EP_ALLOC_MEM(pInputAttachmentEntry->pLodMinMax[i][j],
                                 SHADER_PRIV_CONSTANT_ENTRY,
                                 sizeof(SHADER_PRIV_CONSTANT_ENTRY));
                ON_ERROR0(_vscEP_Buffer_LoadPrivConstEntry(pEPBuf, pInputAttachmentEntry->pLodMinMax[i][j]));
            }
            else
            {
                pInputAttachmentEntry->pLodMinMax[i][j] = gcvNULL;
            }
        }
    }

    /* Load levels samples. */
    entryMask = 0;
    VSC_IO_readUint(pIoBuf, &entryMask);
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (entryMask & (1 << (i * 2 + j)))
            {
                VSC_EP_ALLOC_MEM(pInputAttachmentEntry->pLevelsSamples[i][j],
                                 SHADER_PRIV_CONSTANT_ENTRY,
                                 sizeof(SHADER_PRIV_CONSTANT_ENTRY));
                ON_ERROR0(_vscEP_Buffer_LoadPrivConstEntry(pEPBuf, pInputAttachmentEntry->pLevelsSamples[i][j]));
            }
            else
            {
                pInputAttachmentEntry->pLevelsSamples[i][j] = gcvNULL;
            }
        }
    }

    /* Load ResOpBits. */
    if (pInputAttachmentEntry->iaBinding.arraySize != 0)
    {
        VSC_EP_ALLOC_MEM(pInputAttachmentEntry->pResOpBits,
                         VSC_RES_OP_BIT,
                         sizeof(gctUINT) * pInputAttachmentEntry->iaBinding.arraySize);
        VSC_IO_readBlock(pIoBuf,
                         (gctCHAR*)pInputAttachmentEntry->pResOpBits,
                         sizeof(gctUINT) * pInputAttachmentEntry->iaBinding.arraySize);
    }
    else
    {
        pInputAttachmentEntry->pResOpBits = gcvNULL;
    }

    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        ON_ERROR0(_vscEP_Buffer_LoadVKInputAttachmentHwMapping(pEPBuf,
                                                               &pInputAttachmentEntry->hwMappings[i],
                                                               pInputAttachmentEntry->iaBinding.arraySize));
    }

OnError:
    return errCode;
}

/* Load Vulkan input attachment table. */
static VSC_ErrCode
_vscEP_Buffer_LoadVKInputAttachmentTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_INPUT_ATTACHMENT_TABLE* pInputAttachmentTable
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_readUint(pIoBuf, &pInputAttachmentTable->countOfEntries);

    if (pInputAttachmentTable->countOfEntries != 0)
    {
        VSC_EP_ALLOC_MEM(pInputAttachmentTable->pIaEntries,
                         PROG_VK_INPUT_ATTACHMENT_TABLE_ENTRY,
                         sizeof(PROG_VK_INPUT_ATTACHMENT_TABLE_ENTRY) * pInputAttachmentTable->countOfEntries);
        for (i = 0; i < pInputAttachmentTable->countOfEntries; i++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadVKInutAttachmentEntry(pEPBuf, &pInputAttachmentTable->pIaEntries[i]));
        }
    }
    else
    {
        pInputAttachmentTable->pIaEntries = gcvNULL;
    }

OnError:
    return errCode;
}

/* Load Vulkan storage table entry. */
static VSC_ErrCode
_vscEP_Buffer_LoadVKStorageEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_STORAGE_TABLE_ENTRY* pStorageEntry
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    ON_ERROR0(_vscEP_Buffer_LoadShaderResourceBinding(pEPBuf, &pStorageEntry->storageBinding));
    VSC_IO_readUint(pIoBuf, &pStorageEntry->storageEntryIndex);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pStorageEntry->stageBits);
    VSC_IO_readUint(pIoBuf, &pStorageEntry->activeStageMask);

    /* Load the image derived information. */
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        _vscEP_Buffer_LoadImageDerivedInfo(pEPBuf, &pStorageEntry->imageDerivedInfo[i]);
    }

    /* Load ResOpBits. */
    if (pStorageEntry->storageBinding.arraySize != 0)
    {
        VSC_EP_ALLOC_MEM(pStorageEntry->pResOpBits,
                         VSC_RES_OP_BIT,
                         sizeof(gctUINT) * pStorageEntry->storageBinding.arraySize);
        VSC_IO_readBlock(pIoBuf,
                         (gctCHAR*)pStorageEntry->pResOpBits,
                         sizeof(gctUINT) * pStorageEntry->storageBinding.arraySize);
    }
    else
    {
        pStorageEntry->pResOpBits = gcvNULL;
    }

    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        ON_ERROR0(_vscEP_Buffer_LoadUavSlotMapping(pEPBuf, &pStorageEntry->hwMappings[i]));
    }

OnError:
    return errCode;
}

/* Load Vulkan storage table. */
static VSC_ErrCode
_vscEP_Buffer_LoadVKStorageTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_STORAGE_TABLE*  pStorageTable
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_readUint(pIoBuf, &pStorageTable->countOfEntries);

    if (pStorageTable->countOfEntries != 0)
    {
        VSC_EP_ALLOC_MEM(pStorageTable->pStorageEntries,
                         PROG_VK_STORAGE_TABLE_ENTRY,
                         sizeof(PROG_VK_STORAGE_TABLE_ENTRY) * pStorageTable->countOfEntries);
        for (i = 0; i < pStorageTable->countOfEntries; i++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadVKStorageEntry(pEPBuf, &pStorageTable->pStorageEntries[i]));
        }
    }
    else
    {
        pStorageTable->pStorageEntries = gcvNULL;
    }

OnError:
    return errCode;
}

/* Load Vulkan uniform buffer table entry. */
static VSC_ErrCode
_vscEP_Buffer_LoadVKUniformBufferEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_UNIFORM_BUFFER_TABLE_ENTRY* pUniformBufferEntry
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    ON_ERROR0(_vscEP_Buffer_LoadShaderResourceBinding(pEPBuf, &pUniformBufferEntry->ubBinding));
    VSC_IO_readUint(pIoBuf, &pUniformBufferEntry->ubEntryIndex);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pUniformBufferEntry->stageBits);
    VSC_IO_readUint(pIoBuf, &pUniformBufferEntry->activeStageMask);

    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        ON_ERROR0(_vscEP_Buffer_LoadConstHwLocMapping(pEPBuf, &pUniformBufferEntry->hwMappings[i]));
    }

OnError:
    return errCode;
}

/* Load Vulkan uniform buffer table. */
static VSC_ErrCode
_vscEP_Buffer_LoadVKUniformBuffer(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_UNIFORM_BUFFER_TABLE* pUniformBufferTable
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_readUint(pIoBuf, &pUniformBufferTable->countOfEntries);

    if (pUniformBufferTable->countOfEntries != 0)
    {
        VSC_EP_ALLOC_MEM(pUniformBufferTable->pUniformBufferEntries,
                         PROG_VK_UNIFORM_BUFFER_TABLE_ENTRY,
                         sizeof(PROG_VK_UNIFORM_BUFFER_TABLE_ENTRY) * pUniformBufferTable->countOfEntries);
        for (i = 0; i < pUniformBufferTable->countOfEntries; i++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadVKUniformBufferEntry(pEPBuf, &pUniformBufferTable->pUniformBufferEntries[i]));
        }
    }
    else
    {
        pUniformBufferTable->pUniformBufferEntries = gcvNULL;
    }

OnError:
    return errCode;
}

/* Load Vulkan resource set. */
static VSC_ErrCode
_vscEP_Buffer_LoadVKResourceSet(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_RESOURCE_SET*   pVKResourceSet
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    /* Load Vulkan combined texture-sampler table. */
    ON_ERROR0(_vscEP_Buffer_LoadVKCombinedTextureSamplerTable(pEPBuf, &pVKResourceSet->combinedSampTexTable));

    /* Load Vulkan separated sampler table. */
    ON_ERROR0(_vscEP_Buffer_LoadVKSeparatedSamplerTable(pEPBuf, &pVKResourceSet->separatedSamplerTable));

    /* Load Vulkan separated texture table. */
    ON_ERROR0(_vscEP_Buffer_LoadVKSeparatedTextureTable(pEPBuf, &pVKResourceSet->separatedTexTable));

    /* Load Vulkan uniform texel buffer table. */
    ON_ERROR0(_vscEP_Buffer_LoadVKUniformTexelBufferTable(pEPBuf, &pVKResourceSet->uniformTexBufTable));

    /* Load Vulkan input attachment table. */
    ON_ERROR0(_vscEP_Buffer_LoadVKInputAttachmentTable(pEPBuf, &pVKResourceSet->inputAttachmentTable));

    /* Load Vulkan storage table. */
    ON_ERROR0(_vscEP_Buffer_LoadVKStorageTable(pEPBuf, &pVKResourceSet->storageTable));

    /* Load Vulkan uniform buffer table. */
    ON_ERROR0(_vscEP_Buffer_LoadVKUniformBuffer(pEPBuf, &pVKResourceSet->uniformBufferTable));

OnError:
    return errCode;
}

/* Load shader push constant range. */
static VSC_ErrCode
_vscEP_Buffer_LoadShaderPushConstRange(
    VSC_EP_IO_BUFFER*       pEPBuf,
    VSC_SHADER_PUSH_CONSTANT_RANGE* pShaderPushConstRange
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;

    VSC_IO_readUint(pIoBuf, &pShaderPushConstRange->offset);
    VSC_IO_readUint(pIoBuf, &pShaderPushConstRange->size);

    return errCode;
}

/* Load Vulkan push constant table entry. */
static VSC_ErrCode
_vscEP_Buffer_LoadVKPushConstEntry(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_PUSH_CONSTANT_TABLE_ENTRY* pPushConstEntry
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    ON_ERROR0(_vscEP_Buffer_LoadShaderPushConstRange(pEPBuf, &pPushConstEntry->pushConstRange));
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pPushConstEntry->stageBits);
    VSC_IO_readUint(pIoBuf, &pPushConstEntry->activeStageMask);

    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        ON_ERROR0(_vscEP_Buffer_LoadConstHwLocMapping(pEPBuf, &pPushConstEntry->hwMappings[i]));
    }

OnError:
    return errCode;
}

/* Load Vulkan push constant table. */
static VSC_ErrCode
_vscEP_Buffer_LoadVKPushConstTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_PUSH_CONSTANT_TABLE* pVKPushConstTable
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_readUint(pIoBuf, &pVKPushConstTable->countOfEntries);

    if (pVKPushConstTable->countOfEntries != 0)
    {
        VSC_EP_ALLOC_MEM(pVKPushConstTable->pPushConstantEntries,
                         PROG_VK_PUSH_CONSTANT_TABLE_ENTRY,
                         sizeof(PROG_VK_PUSH_CONSTANT_TABLE_ENTRY) * pVKPushConstTable->countOfEntries);
        for (i = 0; i < pVKPushConstTable->countOfEntries; i++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadVKPushConstEntry(pEPBuf, &pVKPushConstTable->pPushConstantEntries[i]));
        }
    }
    else
    {
        pVKPushConstTable->pPushConstantEntries = gcvNULL;
    }

OnError:
    return errCode;
}

/* Load Vulkan sub resource binding. */
static VSC_ErrCode
_vscEP_Buffer_LoadVKSubResourceBinding(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_SUB_RESOURCE_BINDING* pSubResourceBinding
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT uVal = 0;

    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != 0)
    {
        VSC_EP_ALLOC_MEM(pSubResourceBinding->pResBinding,
                         VSC_SHADER_RESOURCE_BINDING,
                         sizeof(VSC_SHADER_RESOURCE_BINDING));
        ON_ERROR0(_vscEP_Buffer_LoadShaderResourceBinding(pEPBuf, pSubResourceBinding->pResBinding));
    }
    else
    {
        pSubResourceBinding->pResBinding = gcvNULL;
    }

    VSC_IO_readUint(pIoBuf, &pSubResourceBinding->startIdxOfSubArray);
    VSC_IO_readUint(pIoBuf, &pSubResourceBinding->subArraySize);

OnError:
    return errCode;
}

/* Load Vulkan private combined tex-sampl hw mapping. */
static VSC_ErrCode
_vscEP_Buffer_LoadVKPrivCombTexSampHwMapping(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_PRIV_COMB_TEX_SAMP_HW_MAPPING* pPrivCombTexSampHwMapping
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i, j;
    gctUINT uVal = 0;

    VSC_IO_readUint(pIoBuf, &pPrivCombTexSampHwMapping->pctsHmEntryIndex);
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pPrivCombTexSampHwMapping->bSamplerMajor);

    ON_ERROR0(_vscEP_Buffer_LoadVKSubResourceBinding(pEPBuf, &pPrivCombTexSampHwMapping->texSubBinding));
    ON_ERROR0(_vscEP_Buffer_LoadVKSubResourceBinding(pEPBuf, &pPrivCombTexSampHwMapping->samplerSubBinding));

    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != 0)
    {
        VSC_EP_ALLOC_MEM(pPrivCombTexSampHwMapping->pSamplerSlotMapping,
                         SHADER_SAMPLER_SLOT_MAPPING,
                         sizeof(SHADER_SAMPLER_SLOT_MAPPING));
        ON_ERROR0(_vscEP_Buffer_LoadSamplerSlotMapping(pEPBuf, pPrivCombTexSampHwMapping->pSamplerSlotMapping));
    }
    else
    {
        pPrivCombTexSampHwMapping->pSamplerSlotMapping = gcvNULL;
    }

    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != 0 && pPrivCombTexSampHwMapping->samplerSubBinding.subArraySize != 0)
    {
        gcmASSERT(pPrivCombTexSampHwMapping->samplerSubBinding.subArraySize);

        VSC_EP_ALLOC_MEM(pPrivCombTexSampHwMapping->ppExtraSamplerArray,
                         SHADER_PRIV_SAMPLER_ENTRY*,
                         sizeof(SHADER_PRIV_SAMPLER_ENTRY*) * pPrivCombTexSampHwMapping->samplerSubBinding.subArraySize);
        for (i = 0; i < pPrivCombTexSampHwMapping->samplerSubBinding.subArraySize; i++)
        {
            VSC_EP_ALLOC_MEM(pPrivCombTexSampHwMapping->ppExtraSamplerArray[i],
                             SHADER_PRIV_SAMPLER_ENTRY,
                             sizeof(SHADER_PRIV_SAMPLER_ENTRY) * pPrivCombTexSampHwMapping->texSubBinding.subArraySize);
            for (j = 0; j < pPrivCombTexSampHwMapping->texSubBinding.subArraySize; j++)
            {
                ON_ERROR0(_vscEP_Buffer_LoadPrivSamplerEntry(pEPBuf, &pPrivCombTexSampHwMapping->ppExtraSamplerArray[i][j]));
            }
        }
    }
    else
    {
        pPrivCombTexSampHwMapping->ppExtraSamplerArray = gcvNULL;
    }

OnError:
    return errCode;
}

/* Load Vulkan private CTS hw mapping pool. */
static VSC_ErrCode
_vscEP_Buffer_LoadVKPrivCTSHwMappingPool(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROG_VK_PRIV_CTS_HW_MAPPING_POOL* pVKPrivCTSHwMappingPool
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT i;

    VSC_IO_readUint(pIoBuf, &pVKPrivCTSHwMappingPool->countOfArray);

    if (pVKPrivCTSHwMappingPool->countOfArray != 0)
    {
        VSC_EP_ALLOC_MEM(pVKPrivCTSHwMappingPool->pPrivCombTsHwMappingArray,
                         PROG_VK_PRIV_COMB_TEX_SAMP_HW_MAPPING,
                         sizeof(PROG_VK_PRIV_COMB_TEX_SAMP_HW_MAPPING) * pVKPrivCTSHwMappingPool->countOfArray);
        for (i = 0; i < pVKPrivCTSHwMappingPool->countOfArray; i++)
        {
            ON_ERROR0(_vscEP_Buffer_LoadVKPrivCombTexSampHwMapping(pEPBuf, &pVKPrivCTSHwMappingPool->pPrivCombTsHwMappingArray[i]));
        }
    }
    else
    {
        pVKPrivCTSHwMappingPool->pPrivCombTsHwMappingArray = gcvNULL;
    }

OnError:
    return errCode;
}

/* Load Vulkan high level mapping table. */
static VSC_ErrCode
_vscEP_Buffer_LoadVKHighLevelMappingTable(
    VSC_EP_IO_BUFFER*       pEPBuf,
    PROGRAM_EXECUTABLE_PROFILE* pPEP
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    gctUINT     uVal, i;

    /* Start signature. */
    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != VKMSTART_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid program signature 0x%x.", uVal);
    }

    /* Load Vulkan resource sets. */
    VSC_IO_readUint(pIoBuf, &pPEP->u.vk.resourceSetCount);
    if (pPEP->u.vk.resourceSetCount != 0)
    {
        VSC_EP_ALLOC_MEM(pPEP->u.vk.pResourceSets,
                         PROG_VK_RESOURCE_SET,
                         sizeof(PROG_VK_RESOURCE_SET) * pPEP->u.vk.resourceSetCount);
    }
    else
    {
        pPEP->u.vk.pResourceSets = gcvNULL;
    }

    for (i = 0; i < pPEP->u.vk.resourceSetCount; i++)
    {
        ON_ERROR0(_vscEP_Buffer_LoadVKResourceSet(pEPBuf, &pPEP->u.vk.pResourceSets[i]));
    }

    /* Load Vulkan push constant table. */
    ON_ERROR0(_vscEP_Buffer_LoadVKPushConstTable(pEPBuf, &pPEP->u.vk.pushConstantTable));

    /* Load Vulkan private CTS hw mapping pool. */
    ON_ERROR0(_vscEP_Buffer_LoadVKPrivCTSHwMappingPool(pEPBuf, &pPEP->u.vk.privateCombTsHwMappingPool));

    /* End signature. */
    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != VKMEND_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid program signature 0x%x.", uVal);
    }

OnError:
    return errCode;
}

/****************************************Load EP entry functions****************************************/
/* Load SEP from binary. */
static VSC_ErrCode
_vscEP_Buffer_LoadSEPFromBinary(
    VSC_EP_IO_BUFFER*       pEPBuf
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctUINT     uVal;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    SHADER_EXECUTABLE_PROFILE* pSEP = pEPBuf->epData.pSEP;

    vscInitializeSEP(pSEP);

    /* Start signature. */
    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != SEPSTART_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid shader signature 0x%x.", uVal);
    }

    VSC_IO_readUint(pIoBuf, &pSEP->profileVersion);
    VSC_IO_readUint(pIoBuf, &pSEP->chipModel);
    VSC_IO_readUint(pIoBuf, &pSEP->chipRevision);
    VSC_IO_readUint(pIoBuf, &pSEP->productID);
    VSC_IO_readUint(pIoBuf, &pSEP->customerID);
    VSC_IO_readUint(pIoBuf, &pSEP->shVersionType);

    VSC_IO_readUint(pIoBuf, &pSEP->countOfMCInst);
    if (pSEP->countOfMCInst != 0)
    {
        VSC_EP_ALLOC_MEM(pSEP->pMachineCode, gctUINT, sizeof(VSC_MC_RAW_INST) * pSEP->countOfMCInst);
        VSC_IO_readBlock(pIoBuf, (gctCHAR*)pSEP->pMachineCode, sizeof(VSC_MC_RAW_INST) * pSEP->countOfMCInst);
    }
    else
    {
        pSEP->pMachineCode = gcvNULL;
    }

    VSC_IO_readUint(pIoBuf, &pSEP->endPCOfMainRoutine);
    VSC_IO_readUint(pIoBuf, &pSEP->gprCount);
    VSC_IO_readBlock(pIoBuf, (gctCHAR*)&pSEP->exeHints, sizeof(SHADER_EXECUTABLE_HINTS));

    ON_ERROR0(_vscEP_Buffer_LoadIoMapping(pEPBuf, &pSEP->inputMapping));
    ON_ERROR0(_vscEP_Buffer_LoadIoMapping(pEPBuf, &pSEP->outputMapping));
    ON_ERROR0(_vscEP_Buffer_LoadConstMapping(pEPBuf, &pSEP->constantMapping));
    ON_ERROR0(_vscEP_Buffer_LoadSamplerMapping(pEPBuf, &pSEP->samplerMapping));
    ON_ERROR0(_vscEP_Buffer_LoadResourceMapping(pEPBuf, &pSEP->resourceMapping));
    ON_ERROR0(_vscEP_Buffer_LoadUavMapping(pEPBuf, &pSEP->uavMapping));
    ON_ERROR0(_vscEP_Buffer_LoadStaticPrivMapping(pEPBuf, &pSEP->staticPrivMapping));
    ON_ERROR0(_vscEP_Buffer_LoadDynamicPrivMapping(pEPBuf, &pSEP->dynamicPrivMapping));
    ON_ERROR0(_vscEP_Buffer_LoadDefaultUboMapping(pEPBuf, &pSEP->defaultUboMapping));

    /* End signature. */
    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != SEPEND_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid shader signature 0x%x.", uVal);
    }

OnError:
    return errCode;
}

/* Load KEP from binary. */
static VSC_ErrCode
_vscEP_Buffer_LoadKEPFromBinary(
    VSC_EP_IO_BUFFER*       pEPBuf
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    KERNEL_EXECUTABLE_PROFILE* pKEP = pEPBuf->epData.pKEP;
    gctUINT uVal = 0;

    vscInitializeKEP(pKEP);

    /* Start signature. */
    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != KEPSTART_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid kernel signature 0x%x.", uVal);
    }

    /* Load the SEP. */
    pEPBuf->epKind = VSC_EP_KIND_SHADER;
    pEPBuf->epData.pSEP = &pKEP->sep;
    ON_ERROR0(_vscEP_Buffer_LoadSEPFromBinary(pEPBuf));
    pEPBuf->epKind = VSC_EP_KIND_KERNEL;
    pEPBuf->epData.pKEP = pKEP;

    /* End signature. */
    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != KEPEND_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid kernel signature 0x%x.", uVal);
    }

OnError:
    return errCode;
}

/* Load PEP from binary. */
static VSC_ErrCode
_vscEP_Buffer_LoadPEPFromBinary(
    VSC_EP_IO_BUFFER*       pEPBuf
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER* pIoBuf = pEPBuf->pIoBuf;
    PROGRAM_EXECUTABLE_PROFILE* pPEP = pEPBuf->epData.pPEP;
    gctUINT uVal = 0;
    gctUINT i;

    vscInitializePEP(pPEP);

    /* Start signature. */
    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != PEPSTART_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid program signature 0x%x.", uVal);
    }

    /* Load pep client version. */
    VSC_IO_readUint(pIoBuf, (gctUINT*)&pPEP->pepClient);

    /* Load all SEP. */
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        pEPBuf->epKind = VSC_EP_KIND_SHADER;
        pEPBuf->epData.pSEP = &pPEP->seps[i];
        ON_ERROR0(_vscEP_Buffer_LoadSEPFromBinary(pEPBuf));
    }
    pEPBuf->epKind = VSC_EP_KIND_PROGRAM;
    pEPBuf->epData.pPEP = pPEP;

    /* Load program attribute table. */
    ON_ERROR0(_vscEP_Buffer_LoadProgAttrTable(pEPBuf, &pPEP->attribTable));

    /* Load fragment output table. */
    ON_ERROR0(_vscEP_Buffer_LoadFragOutTable(pEPBuf, &pPEP->fragOutTable));

    /* Load other high level mapping table. */
    switch (pPEP->pepClient)
    {
    case PEP_CLIENT_GL:
        ON_ERROR0(_vscEP_Buffer_LoadGLHighLevelMappingTable(pEPBuf, pPEP));
        break;

    case PEP_CLIENT_VK:
        ON_ERROR0(_vscEP_Buffer_LoadVKHighLevelMappingTable(pEPBuf, pPEP));
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    /* End signature. */
    VSC_IO_readUint(pIoBuf, &uVal);
    if (uVal != PEPEND_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid program signature 0x%x.", uVal);
    }

OnError:
    return errCode;
}

/* Load EP data from binary. */
static VSC_ErrCode
_vscEP_Buffer_LoadFromBinary(
    VSC_EP_IO_BUFFER*       pEPBuf
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctUINT     uVal;

    VSC_IO_readUint(pEPBuf->pIoBuf, &uVal);
    if (uVal != EPSTART_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid shader signature 0x%x.", uVal);
    }

    VSC_IO_readUint(pEPBuf->pIoBuf, &uVal);
    switch ((VSC_EP_KIND)uVal)
    {
    case VSC_EP_KIND_SHADER:
        ON_ERROR0(_vscEP_Buffer_LoadSEPFromBinary(pEPBuf));
        break;

    case VSC_EP_KIND_KERNEL:
        ON_ERROR0(_vscEP_Buffer_LoadKEPFromBinary(pEPBuf));
        break;

    case VSC_EP_KIND_PROGRAM:
        ON_ERROR0(_vscEP_Buffer_LoadPEPFromBinary(pEPBuf));
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    VSC_IO_readUint(pEPBuf->pIoBuf, &uVal);
    if (uVal != EPEND_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid shader signature 0x%x.", uVal);
    }

OnError:
    return errCode;
}

/* For saver, it always returns how many bytes it saves to binary buffer, and
              1). if ppOutBinary == NULL (szBinaryInByte will be igored), then not doing real saving, just return byte size
              2). if ppOutBinary != NULL and *ppOutBinary == NULL (szBinaryInByte will be igored), then saver will allocate a
                  binary for user.
              3). *ppOutBinary != NULL, then szBinaryInByte must be the size of the binary (generally, this size got by first usage
                  of this function).
*/
gctUINT vscSaveEPToBinary(VSC_EP_KIND epKind, void* pEP, void** ppOutBinary, gctUINT szBinaryInByte)
{
    VSC_EP_IO_BUFFER    epBuf;
    VSC_IO_BUFFER       ioBuf = {0, 0, gcvNULL};
    gctBOOL             allocBuf = gcvFALSE;

    /* I: Init Buffer.*/
    if (ppOutBinary == gcvNULL)
    {
        _vscEP_Buffer_Init(&epBuf, &ioBuf, epKind, pEP, gcvNULL, szBinaryInByte, gcvTRUE);
    }
    else if (ppOutBinary != gcvNULL && *ppOutBinary == gcvNULL)
    {
        _vscEP_Buffer_Init(&epBuf, &ioBuf, epKind, pEP, gcvNULL, szBinaryInByte, gcvFALSE);
        allocBuf = gcvTRUE;
    }
    else
    {
        gcmASSERT(*ppOutBinary != gcvNULL);

        _vscEP_Buffer_Init(&epBuf, &ioBuf, epKind, pEP, *ppOutBinary, szBinaryInByte, gcvFALSE);
    }

    /* II: Save data to binary. */
    _vscEP_Buffer_SaveToBinary(&epBuf);

    /* III: Finalize. */
    _vscEP_Buffer_Finalize(&epBuf);

    /* IV: return. */
    if (allocBuf)
    {
        gcmASSERT(ppOutBinary != gcvNULL && *ppOutBinary == gcvNULL);

        *ppOutBinary = (void*)(epBuf.pIoBuf->buffer);
    }
    return (epBuf.pIoBuf->allocatedBytes);
}

/* For loader, szBinaryInByte must be size of binary, and return value is the real sparsed size of binary. */
gctUINT vscLoadEPFromBinary(VSC_EP_KIND epKind, void* pInBinary, gctUINT szBinaryInByte, void* pEP)
{
    VSC_EP_IO_BUFFER    epBuf;
    VSC_IO_BUFFER       ioBuf = {0, 0, gcvNULL};

    /* I: Init buffer. */
    _vscEP_Buffer_Init(&epBuf, &ioBuf, epKind, pEP, pInBinary, szBinaryInByte, gcvFALSE);

    /* II: Load data from binary. */
    _vscEP_Buffer_LoadFromBinary(&epBuf);

    /* III: Finalize. */
    _vscEP_Buffer_Finalize(&epBuf);

    /* IV: return value. */
    return epBuf.pIoBuf->curPos;
}


