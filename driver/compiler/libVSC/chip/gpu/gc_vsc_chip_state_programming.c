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
#include "chip/gpu/gc_vsc_chip_state_programming.h"

#define VSC_CHIP_STATES_ALLOC_GRANULARITY  512

/* Belows for old chips */
#define MAX_BASIC_VS_HW_INPUTS             16
#define MAX_EXT_VS_HW_INPUTS               1
#define MAX_VS_HW_INPUTS                   (MAX_BASIC_VS_HW_INPUTS + MAX_EXT_VS_HW_INPUTS)

#define MAX_BASIC_VS_HW_OUTPUTS            16
#define MAX_EXT_VS_HW_OUTPUTS              3
#define MAX_VS_HW_OUTPUTS                  (MAX_BASIC_VS_HW_OUTPUTS + MAX_EXT_VS_HW_OUTPUTS)

#define MAX_HW_PS_INPUTS                   MAX_BASIC_VS_HW_OUTPUTS
#define MAX_HW_PS_COLOR_OUTPUTS            8

/* Belows for new chips */
#define MAX_HW_IO_COUNT                    32

#define DMA_BATCH_SIZE_LIMIT               256

#define SHADER_STATE_PADDING               0xdeadbeef

#define ENABLE_USC_DYNAMICALLY_ALLOC       0

typedef enum GPGPU_THD_GRP_ID_ORDER
{
    /* G: global thread id
       W: work thread group id
       L: thread id in group (local)
    */

    GPGPU_THD_GRP_ID_ORDER_LWG2GLW  = 0,
    GPGPU_THD_GRP_ID_ORDER_WLG2GWL,
    GPGPU_THD_GRP_ID_ORDER_GWL2LGW,
    GPGPU_THD_GRP_ID_ORDER_GLW2WGL,
    GPGPU_THD_GRP_ID_ORDER_WGL2LWG,
    GPGPU_THD_GRP_ID_ORDER_LGW2WLG,
}
GPGPU_THREAD_GROUP_ID_ORDER;

#define BEGIN_STATE_BATCH(pStateBuffer, address, count)                                             \
    *(pStateBuffer) ++  = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE, LOAD_STATE) | \
                          gcmSETFIELD     (0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, (address))  | \
                          gcmSETFIELD     (0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT, (count))    | \
                          gcmSETFIELD     (0, AQ_COMMAND_LOAD_STATE_COMMAND, FLOAT, 0);

VSC_ErrCode vscInitializeChipStatesProgrammer(VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer,
                                              PVSC_SYS_CONTEXT pSysCtx,
                                              struct _gcsHINT* pHints)
{
    gcoOS_ZeroMemory(pStatesPgmer, sizeof(VSC_CHIP_STATES_PROGRAMMER));

    vscPMP_Intialize(&pStatesPgmer->pmp, gcvNULL, VSC_CHIP_STATES_ALLOC_GRANULARITY*4, sizeof(void*), gcvTRUE);

    pStatesPgmer->pSysCtx = pSysCtx;

    pStatesPgmer->pStartStateBuffer = (gctUINT*)vscMM_Alloc(&pStatesPgmer->pmp.mmWrapper,
                                                            VSC_CHIP_STATES_ALLOC_GRANULARITY*sizeof(gctUINT));
    pStatesPgmer->allocSize = VSC_CHIP_STATES_ALLOC_GRANULARITY;
    pStatesPgmer->nextStateAddr = 0;

    pStatesPgmer->pHints = pHints;

    pStatesPgmer->pStateDelta = (gctUINT32*)vscMM_Alloc(&pStatesPgmer->pmp.mmWrapper,
        VSC_CHIP_STATES_ALLOC_GRANULARITY * sizeof(gctUINT));
    pStatesPgmer->stateDeltaAllocSize  = VSC_CHIP_STATES_ALLOC_GRANULARITY;
    pStatesPgmer->nextStateDeltaAddr = 0;

    return (pStatesPgmer->pStartStateBuffer) ? VSC_ERR_NONE : VSC_ERR_OUT_OF_MEMORY;
}

VSC_ErrCode vscFinalizeChipStatesProgrammer(VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    vscPMP_Finalize(&pStatesPgmer->pmp);
    pStatesPgmer->pStartStateBuffer = gcvNULL;
    pStatesPgmer->allocSize = 0;
    pStatesPgmer->nextStateAddr = 0;
    pStatesPgmer->pHints = gcvNULL;

    pStatesPgmer->pStateDelta = gcvNULL;
    pStatesPgmer->stateDeltaAllocSize = 0;
    pStatesPgmer->nextStateDeltaAddr = 0;

    gcoOS_ZeroMemory(&pStatesPgmer->patchOffsetsInDW, sizeof(gcsPROGRAM_VidMemPatchOffset));

    return VSC_ERR_NONE;
}

enum
{
    _VSC_PATCH_OFFSET_INSTR    = 1,
    _VSC_PATCH_OFFSET_GPRSPILL = 2,
    _VSC_PATCH_OFFSET_CRSPILL  = 3,
    _VSC_PATCH_OFFSET_SHAREMEM = 4,
    _VSC_PATCH_OFFSET_THREADID = 5,
};

/*
** !!!PLEASE be sure this function is called RIGHT before the VSC_LOAD_HW_STATE for buffer address.
*/
static VSC_ErrCode _SetPatchOffsets(VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer,
                                    gctUINT patchOffsetType,
                                    gcsSHADER_GROUP_SHADER_KIND shaderType
                                    )
{
    switch (patchOffsetType)
    {
    case _VSC_PATCH_OFFSET_INSTR:
        pStatesPgmer->patchOffsetsInDW.instVidMemInStateBuffer[shaderType] = pStatesPgmer->nextStateAddr + 1;
        pStatesPgmer->patchOffsetsInDW.instVidMemInStateDelta[shaderType] = pStatesPgmer->nextStateDeltaAddr + 2;
        break;
    case _VSC_PATCH_OFFSET_GPRSPILL:
        pStatesPgmer->patchOffsetsInDW.gprSpillVidMemInStateBuffer[shaderType] = pStatesPgmer->nextStateAddr + 1;
        pStatesPgmer->patchOffsetsInDW.gprSpillVidMemInStateDelta[shaderType] = pStatesPgmer->nextStateDeltaAddr + 2;
        break;
    case _VSC_PATCH_OFFSET_CRSPILL:
        pStatesPgmer->patchOffsetsInDW.crSpillVidMemInStateBuffer[shaderType] = pStatesPgmer->nextStateAddr + 1;
        pStatesPgmer->patchOffsetsInDW.crSpillVidMemInStateDelta[shaderType] = pStatesPgmer->nextStateDeltaAddr + 2;
        break;
    case _VSC_PATCH_OFFSET_SHAREMEM:
        pStatesPgmer->patchOffsetsInDW.sharedMemVidMemInStateBuffer = pStatesPgmer->nextStateAddr + 1;
        pStatesPgmer->patchOffsetsInDW.sharedMemVidMemInStateDelta = pStatesPgmer->nextStateDeltaAddr + 2;
        break;
    case _VSC_PATCH_OFFSET_THREADID:
        pStatesPgmer->patchOffsetsInDW.threadIdVidMemInStateBuffer = pStatesPgmer->nextStateAddr + 1;
        pStatesPgmer->patchOffsetsInDW.threadIdVidMemInStateDelta = pStatesPgmer->nextStateDeltaAddr + 2;
        break;
    }
    return VSC_ERR_NONE;
}

static VSC_ErrCode _LoadContinuousAddressStates(VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer,
                                                gctUINT startAddress,
                                                gctUINT* pStateData,
                                                gctUINT dataCountInDW)
{
    gctUINT      requestSize, i;
    gctUINT*     pStateBuffer;
    gctUINT*     pStateDelta;
    gctUINT      requestStateDelta;

    /* Align States request to a multiple of 2 dwords for command alignment. */
    requestSize = ((dataCountInDW + 2) / 2) * 2;

    requestStateDelta = dataCountInDW + VSC_STATE_DELTA_DESC_SIZE_IN_UINT32;

    /* Get where to load states */
    if (pStatesPgmer->nextStateAddr + requestSize > pStatesPgmer->allocSize)
    {
        pStatesPgmer->allocSize = VSC_UTILS_ALIGN(pStatesPgmer->nextStateAddr + requestSize,
                                                  VSC_CHIP_STATES_ALLOC_GRANULARITY);

        pStatesPgmer->pStartStateBuffer = (gctUINT*)vscMM_Realloc(&pStatesPgmer->pmp.mmWrapper,
                                                                  pStatesPgmer->pStartStateBuffer,
                                                                  pStatesPgmer->allocSize*sizeof(gctUINT));

        if (pStatesPgmer->pStartStateBuffer == gcvNULL)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
    }

    pStateBuffer = pStatesPgmer->pStartStateBuffer + pStatesPgmer->nextStateAddr;
    /* Begin to load */
        *(pStateBuffer) ++ =  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27)))  |  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) ((startAddress)) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0)))  |  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) ((dataCountInDW)) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16)))  |  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26))) ;

    /* Load real states */
    memcpy(pStateBuffer, pStateData, sizeof(gctUINT)*dataCountInDW);
    pStateBuffer += dataCountInDW;

    /* Add filler for command alignment if needed. */
    for (i = dataCountInDW + 1; i < requestSize; i ++)
    {
        *(pStateBuffer) ++ = SHADER_STATE_PADDING;
    }

    /* Move on to next available state buffer address */
    pStatesPgmer->nextStateAddr += requestSize;

    /* Get where to load states */
    if (pStatesPgmer->nextStateDeltaAddr + requestStateDelta > pStatesPgmer->stateDeltaAllocSize)
    {
        pStatesPgmer->stateDeltaAllocSize = VSC_UTILS_ALIGN(pStatesPgmer->nextStateDeltaAddr + requestStateDelta,
                VSC_CHIP_STATES_ALLOC_GRANULARITY);

        pStatesPgmer->pStateDelta = (gctUINT32 *)vscMM_Realloc(&pStatesPgmer->pmp.mmWrapper,
                                                                  pStatesPgmer->pStateDelta,
                                                                  pStatesPgmer->stateDeltaAllocSize*sizeof(gctUINT32));

        if (pStatesPgmer->pStateDelta == gcvNULL)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
    }

    pStateDelta = pStatesPgmer->pStateDelta + pStatesPgmer->nextStateDeltaAddr;

    *pStateDelta++ = startAddress;
    *pStateDelta++ = dataCountInDW;
    gcoOS_MemCopy(pStateDelta, pStateData, sizeof(gctUINT)*dataCountInDW);
    pStateDelta += dataCountInDW;
    *pStateDelta = VSC_STATE_DELTA_END;

    pStatesPgmer->nextStateDeltaAddr += requestStateDelta;


    return VSC_ERR_NONE;
}

/* Load states for multiple continuous address */
#define VSC_LOAD_HW_STATES(startAddress, pStateData, count)                                \
    errCode = _LoadContinuousAddressStates(pStatesPgmer, startAddress, pStateData, count); \
    ON_ERROR(errCode, "Load states");

/* Load one state for one address */
#define VSC_LOAD_HW_STATE(startAddress, stateData)                                         \
    VSC_LOAD_HW_STATES((startAddress), &(stateData), 1);

VSC_ErrCode vscVerifyShaderStates(VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    gctUINT  offset, statesCount;
    gctUINT* statesBuffer = pStatesPgmer->pStartStateBuffer;
    gctUINT* stateDeltaBuffer = pStatesPgmer->pStateDelta;

    gcmASSERT(statesBuffer && pStatesPgmer->nextStateAddr);

    for (offset = 0; offset < pStatesPgmer->nextStateAddr;)
    {
        /* Extract fields. */
        statesCount = (((((gctUINT32) (*statesBuffer)) >> (0 ? 25:16)) & ((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1)))))) );
        statesCount = (statesCount == 0) ? 1024 : statesCount;

        /* command */
        statesBuffer += 1;
        offset += 1;

        /* states */
        statesBuffer += statesCount;
        offset += statesCount;

        /* Verify padding*/
        if ((statesCount & 1) == 0)
        {
            gcmASSERT(*statesBuffer == SHADER_STATE_PADDING);

            statesBuffer += 1;
            offset += 1;
        }
    }

    gcmASSERT(offset == pStatesPgmer->nextStateAddr);

    for (offset = 0; offset < pStatesPgmer->nextStateDeltaAddr;)
    {
        statesCount = *(stateDeltaBuffer + 1);
        gcmASSERT(*(stateDeltaBuffer + 2 + statesCount) == VSC_STATE_DELTA_END);
        stateDeltaBuffer += statesCount + VSC_STATE_DELTA_DESC_SIZE_IN_UINT32;
        offset += statesCount + VSC_STATE_DELTA_DESC_SIZE_IN_UINT32;
    }

    gcmASSERT(offset == pStatesPgmer->nextStateDeltaAddr);

    return VSC_ERR_NONE;
}

static gctUINT _GetValidHwRegChannelMaskForChannelPervertedIoReg(SHADER_IO_REG_MAPPING* pIoRegMapping)
{
    gctUINT channelIdx;
    gctUINT hwChannelMask = 0;

    for (channelIdx = 0; channelIdx < CHANNEL_NUM; channelIdx ++)
    {
        if (pIoRegMapping->ioChannelMask & (1 << channelIdx))
        {
            hwChannelMask |= (1 << pIoRegMapping->ioChannelMapping[channelIdx].hwLoc.cmnHwLoc.hwRegChannel);
        }
    }

    return hwChannelMask;
}

extern gctUINT _GetValidHwRegChannelCount(SHADER_IO_REG_MAPPING* pIoRegMapping, SHADER_IO_MEM_ALIGN ioMemAlign)
{
    gctUINT writeMask = _GetValidHwRegChannelMaskForChannelPervertedIoReg(pIoRegMapping);

#define WRITEMASK_2_VALID_CHANNEL_COUNT(writeMask)   \
    ((writeMask & WRITEMASK_W) ? (CHANNEL_NUM)   : (\
     (writeMask & WRITEMASK_Z) ? (CHANNEL_NUM-1) : (\
     (writeMask & WRITEMASK_Y) ? (CHANNEL_NUM-2) : (\
     (writeMask & WRITEMASK_X) ? (CHANNEL_NUM-3) :   \
     (CHANNEL_NUM)))))

    if (pIoRegMapping->regIoMode == SHADER_IO_MODE_ACTIVE)
    {
        if (ioMemAlign == SHADER_IO_MEM_ALIGN_4_CHANNELS)
        {
            return WRITEMASK_2_VALID_CHANNEL_COUNT(writeMask);
        }
        else
        {
            /* An IO may spans 2 corresponding HW regs in memory */
            gcmASSERT(gcvFALSE);
            return CHANNEL_NUM;
        }
    }
    else
    {
#if IO_HW_CHANNEL_IDENTICAL_TO_IO_LL_CHANNEL
        return WRITEMASK_2_VALID_CHANNEL_COUNT(pIoRegMapping->ioChannelMask);
#else
        return WRITEMASK_2_VALID_CHANNEL_COUNT(writeMask);
#endif
    }
}

static void _ProgramConstCountInfo(SHADER_HW_INFO* pShHwInfo,
                                   VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer,
                                   gctBOOL bGPipe)
{
    if (pShHwInfo->pSEP->exeHints.derivedHints.globalStates.unifiedConstRegAllocStrategy == UNIFIED_RF_ALLOC_STRATEGY_UNIFIED)
    {
        pStatesPgmer->pHints->unifiedStatus.constCount = vscMAX(pShHwInfo->pSEP->constantMapping.maxHwConstRegIndex + 1,
                                                                pStatesPgmer->pHints->unifiedStatus.constCount);
    }
    else if (pShHwInfo->pSEP->exeHints.derivedHints.globalStates.unifiedConstRegAllocStrategy == UNIFIED_RF_ALLOC_STRATEGY_PACK_FLOAT_ADDR_OFFSET)
    {
        if (pStatesPgmer->pHints->unifiedStatus.constCount >= 0)
        {
            pStatesPgmer->pHints->unifiedStatus.constCount += pShHwInfo->pSEP->constantMapping.maxHwConstRegIndex + 1;
        }
        else
        {
            pStatesPgmer->pHints->unifiedStatus.constCount = pShHwInfo->pSEP->constantMapping.maxHwConstRegIndex + 1;
        }
    }
    else if (pShHwInfo->pSEP->exeHints.derivedHints.globalStates.unifiedConstRegAllocStrategy == UNIFIED_RF_ALLOC_STRATEGY_GPIPE_TOP_PS_BOTTOM_FLOAT_ADDR_OFFSET)
    {
        if (bGPipe)
        {
            pStatesPgmer->pHints->unifiedStatus.constGPipeEnd += pShHwInfo->pSEP->constantMapping.hwConstRegCount;
        }
        else
        {
            pStatesPgmer->pHints->unifiedStatus.constPSStart = pShHwInfo->hwProgrammingHints.hwConstantRegAddrOffset;
        }
    }
}

static void _ProgramSamplerCountInfo(SHADER_HW_INFO* pShHwInfo,
                                     VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer,
                                     gctBOOL bGPipe)
{
    if (pShHwInfo->pSEP->exeHints.derivedHints.globalStates.unifiedSamplerRegAllocStrategy == UNIFIED_RF_ALLOC_STRATEGY_UNIFIED)
    {
        pStatesPgmer->pHints->unifiedStatus.samplerCount = vscMAX(pShHwInfo->pSEP->samplerMapping.maxHwSamplerRegIndex + 1,
                                                                    pStatesPgmer->pHints->unifiedStatus.samplerCount);
    }
    else if (pShHwInfo->pSEP->exeHints.derivedHints.globalStates.unifiedSamplerRegAllocStrategy == UNIFIED_RF_ALLOC_STRATEGY_PACK_FLOAT_ADDR_OFFSET)
    {
        if (pStatesPgmer->pHints->unifiedStatus.samplerCount >= 0)
        {
            pStatesPgmer->pHints->unifiedStatus.samplerCount += pShHwInfo->pSEP->samplerMapping.maxHwSamplerRegIndex + 1;
        }
        else
        {
            pStatesPgmer->pHints->unifiedStatus.samplerCount = pShHwInfo->pSEP->samplerMapping.maxHwSamplerRegIndex + 1;
        }
    }
    else if (pShHwInfo->pSEP->exeHints.derivedHints.globalStates.unifiedSamplerRegAllocStrategy == UNIFIED_RF_ALLOC_STRATEGY_PS_TOP_GPIPE_BOTTOM_FLOAT_ADDR_OFFSET)
    {
        if (bGPipe)
        {
            pStatesPgmer->pHints->unifiedStatus.samplerGPipeStart -= pShHwInfo->pSEP->samplerMapping.hwSamplerRegCount;
        }
        else
        {
            if (pShHwInfo->pSEP->samplerMapping.hwSamplerRegCount != 0)
            {
                pStatesPgmer->pHints->unifiedStatus.samplerPSEnd = pShHwInfo->pSEP->samplerMapping.hwSamplerRegCount - 1;
            }
        }
    }
}

static VSC_ErrCode _ProgramRegedCTC(SHADER_HW_INFO* pShHwInfo,
                                    gctUINT startConstRegAddr,
                                    VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    gctUINT                    ctcIdx, channel, ctcAddr;
    gctBOOL                    useRegedCTC = gcvFALSE;

    for (ctcIdx = 0; ctcIdx < pShHwInfo->pSEP->constantMapping.countOfCompileTimeConstant; ctcIdx ++)
    {
        SHADER_COMPILE_TIME_CONSTANT *pCTC = pShHwInfo->pSEP->constantMapping.pCompileTimeConstant + ctcIdx;

        if (pCTC->hwConstantLocation.hwAccessMode != SHADER_HW_ACCESS_MODE_REGISTER)
        {
            continue;
        }

        useRegedCTC = gcvTRUE;

        if (pCTC->hwConstantLocation.validHWChannelMask == WRITEMASK_ALL)
        {
            ctcAddr = startConstRegAddr + (pCTC->hwConstantLocation.hwLoc.constReg.hwRegNo * CHANNEL_NUM);
            VSC_LOAD_HW_STATES(ctcAddr, pCTC->constantValue, CHANNEL_NUM);
        }
        else
        {
            for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
            {
                if (pCTC->hwConstantLocation.validHWChannelMask & (1 << channel))
                {
                    ctcAddr = startConstRegAddr + (pCTC->hwConstantLocation.hwLoc.constReg.hwRegNo * CHANNEL_NUM) + channel;
                    VSC_LOAD_HW_STATE(ctcAddr, pCTC->constantValue[channel]);
                }
            }
        }
    }

    switch ((SHADER_TYPE)DECODE_SHADER_TYPE(pShHwInfo->pSEP->shVersionType))
    {
    case SHADER_TYPE_VERTEX:
        pStatesPgmer->pHints->useRegedCTC[gceSGSK_VERTEX_SHADER] = (gctCHAR)useRegedCTC;
        break;

    case SHADER_TYPE_HULL:
        pStatesPgmer->pHints->useRegedCTC[gceSGSK_TC_SHADER] = (gctCHAR)useRegedCTC;
        break;

    case SHADER_TYPE_DOMAIN:
        pStatesPgmer->pHints->useRegedCTC[gceSGSK_TE_SHADER] = (gctCHAR)useRegedCTC;
        break;

    case SHADER_TYPE_GEOMETRY:
        pStatesPgmer->pHints->useRegedCTC[gceSGSK_GEOMETRY_SHADER] = (gctCHAR)useRegedCTC;
        break;

    case SHADER_TYPE_PIXEL:
        pStatesPgmer->pHints->useRegedCTC[gceSGSK_FRAGMENT_SHADER] = (gctCHAR)useRegedCTC;
        break;

    case SHADER_TYPE_GENERAL:
        pStatesPgmer->pHints->useRegedCTC[gceSGSK_CL_SHADER] = (gctCHAR)useRegedCTC;
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

OnError:
    return errCode;
}

static VSC_ErrCode _ProgramStreamOut(SHADER_HW_INFO* pShHwInfo,
                                     SHADER_IO_MAPPING_PER_EXE_OBJ* pOutputMappingPerVtx,
                                     SHADER_IO_LINKAGE_INFO_PER_EXE_OBJ* pOutputLinkagePerVtx,
                                     VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    SHADER_IO_REG_MAPPING*     pSoOutputRegMapping;
    SHADER_IO_REG_LINKAGE*     pSoOutputLinkage;
    gctUINT                    state, ioIdx, channelIdx, descriptorAddrOffset = 0, descriptorCountPerStream = 0;
    gctUINT                    soOutputArray[MAX_SHADER_STREAM_OUT_BUFFER_NUM][MAX_SHADER_IO_NUM];
    gctUINT                    soHwStartChannel, soHwChannelCount, soStreamIdx, soOutputIdx, activeStreamIdx;
    gctBOOL                    bFinishSoCompCalc;
    SHADER_TYPE                shType = (SHADER_TYPE)DECODE_SHADER_TYPE(pShHwInfo->pSEP->shVersionType);
    gctBOOL                    bHasStreamOut = gcvFALSE;

    gcmASSERT(shType == SHADER_TYPE_VERTEX   ||
              shType == SHADER_TYPE_GEOMETRY ||
              shType == SHADER_TYPE_DOMAIN);

    if (shType == SHADER_TYPE_GEOMETRY && pShHwInfo->pSEP->exeHints.nativeHints.prvStates.gs.bHasStreamOut)
    {
        bHasStreamOut = gcvTRUE;
    }

    for (soStreamIdx = 0; soStreamIdx < MAX_SHADER_STREAM_OUT_BUFFER_NUM; soStreamIdx ++)
    {
        for (soOutputIdx = 0; soOutputIdx < MAX_SHADER_IO_NUM; soOutputIdx ++)
        {
            soOutputArray[soStreamIdx][soOutputIdx] = NOT_ASSIGNED;
        }
    }

    for (ioIdx = 0; ioIdx < pOutputMappingPerVtx->countOfIoRegMapping; ioIdx ++)
    {
        if (pOutputMappingPerVtx->ioIndexMask & (1LL << ioIdx))
        {
            /* Only consider linked one */
            if (pOutputLinkagePerVtx->ioRegLinkage[ioIdx].linkNo != NOT_ASSIGNED)
            {
                /* Consider streamed one */
                if (pOutputMappingPerVtx->soIoIndexMask & (1LL << ioIdx))
                {
                    pSoOutputRegMapping = &pOutputMappingPerVtx->pIoRegMapping[ioIdx];

                    gcmASSERT(pSoOutputRegMapping->soStreamBufferSlot < MAX_SHADER_STREAM_OUT_BUFFER_NUM);
                    gcmASSERT(pSoOutputRegMapping->soSeqInStreamBuffer < MAX_SHADER_IO_NUM);

                    soOutputArray[pSoOutputRegMapping->soStreamBufferSlot]
                                 [pSoOutputRegMapping->soSeqInStreamBuffer] = ioIdx;
                }
            }
        }
    }

    for (soStreamIdx = 0; soStreamIdx < MAX_SHADER_STREAM_OUT_BUFFER_NUM; soStreamIdx ++)
    {
        /* If this shader have no multi-stream, then we always use stream 0. */
        if (bHasStreamOut)
        {
            /* Each stream has 128 descirptors, so the descriptorAddrOffset starts with StreamIndex * 128. */
            descriptorAddrOffset = soStreamIdx * 128;
            descriptorCountPerStream = 0;
            activeStreamIdx = soStreamIdx;
        }
        else
        {
            activeStreamIdx = 0;
        }

        for (soOutputIdx = 0; soOutputIdx < MAX_SHADER_IO_NUM; soOutputIdx ++)
        {
            ioIdx = soOutputArray[soStreamIdx][soOutputIdx];

            if (ioIdx != NOT_ASSIGNED)
            {
                pSoOutputLinkage = &pOutputLinkagePerVtx->ioRegLinkage[ioIdx];

                gcmASSERT(pOutputMappingPerVtx->ioIndexMask & (1LL << ioIdx) &&
                          pSoOutputLinkage->linkNo != NOT_ASSIGNED &&
                          pOutputMappingPerVtx->soIoIndexMask & (1LL << ioIdx));

                pSoOutputRegMapping = &pOutputMappingPerVtx->pIoRegMapping[ioIdx];

                /* Cacl start hw channel and hw channel count to stream */
                soHwStartChannel = NOT_ASSIGNED;
                soHwChannelCount = 0;
                bFinishSoCompCalc = gcvFALSE;
                for (channelIdx = pSoOutputRegMapping->firstValidIoChannel;
                     channelIdx < CHANNEL_NUM;
                     channelIdx ++)
                {
                    if (pSoOutputRegMapping->ioChannelMask & (1 << channelIdx))
                    {
                        if (pSoOutputRegMapping->ioChannelMapping[channelIdx].flag.bStreamOutToBuffer)
                        {
                            if (soHwStartChannel == NOT_ASSIGNED)
                            {
                                soHwStartChannel = pSoOutputRegMapping->ioChannelMapping[channelIdx].hwLoc.cmnHwLoc.hwRegChannel;
                            }
                            else
                            {
                                gcmASSERT(pSoOutputRegMapping->ioChannelMapping[channelIdx].hwLoc.cmnHwLoc.hwRegChannel ==
                                          (soHwStartChannel + soHwChannelCount));
                            }

                            soHwChannelCount ++;
                        }
                        else
                        {
                            if (soHwStartChannel != NOT_ASSIGNED)
                            {
                                if (!bFinishSoCompCalc)
                                {
                                    bFinishSoCompCalc = gcvTRUE;
                                }
                                else
                                {
                                    gcmASSERT(gcvFALSE);
                                }
                            }
                        }
                    }
                }

                state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (pSoOutputRegMapping->soStreamBufferSlot) & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (soHwChannelCount > 0 ?
 0 : 1) & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) ((gctUINT32) (pSoOutputLinkage->linkNo) & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:16) - (0 ?
 17:16) + 1))))))) << (0 ?
 17:16))) | (((gctUINT32) ((gctUINT32) (soHwStartChannel) & ((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ?
 21:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:20) - (0 ?
 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) ((gctUINT32) (((soHwChannelCount == 4) ?
 0 : soHwChannelCount)) & ((gctUINT32) ((((1 ?
 21:20) - (0 ?
 21:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));

                VSC_LOAD_HW_STATE(0x7200 + descriptorAddrOffset, state);
                descriptorAddrOffset++;
                descriptorCountPerStream++;
            }
        }

        /* Tell HW how many TFBDescriptor we have programed */
        if (bHasStreamOut || soStreamIdx == MAX_SHADER_STREAM_OUT_BUFFER_NUM - 1)
        {
            VSC_LOAD_HW_STATE(0x7040 + activeStreamIdx, descriptorCountPerStream);
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _AllocVidMemForGprSpill(VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer,
                                           SHADER_EXECUTABLE_PROFILE* pSEP,
                                           gcsSURF_NODE_PTR* pGprSpillVidmemNode,
                                           gctUINT32* pVidMemAddrOfSpillMem,
                                           gctUINT *  pGprSpillSize)

{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    gctUINT                    gprSpillSize = 0, i;
    gctUINT32                  physical = NOT_ASSIGNED;

    for (i = 0; i < pSEP->staticPrivMapping.privUavMapping.countOfEntries; i ++)
    {
        if (pSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].commonPrivm.privmKind == SHS_PRIV_MEM_KIND_GPR_SPILLED_MEMORY)
        {
            gcmASSERT(pSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].memData.ppCnstSubArray == gcvNULL);
            gcmASSERT(pSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].memData.ppCTC == gcvNULL);

            gprSpillSize = pSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].pBuffer->sizeInByte;

            break;
        }
    }

    gcmASSERT(gprSpillSize);
    *pGprSpillSize = gprSpillSize;
    (*pStatesPgmer->pSysCtx->drvCBs.pfnAllocVidMemCb)(pStatesPgmer->pSysCtx->hDrv,
                                                      gcvSURF_VERTEX,
                                                      "temp register spill memory",
                                                      gprSpillSize,
                                                      64,
                                                      (gctPOINTER *)pGprSpillVidmemNode,
                                                      gcvNULL,
                                                      &physical,
                                                      gcvNULL,
                                                      gcvTRUE);

    if (NOT_ASSIGNED == physical)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        ON_ERROR(errCode, "Create gpr spill vid-mem");
    }

    *pVidMemAddrOfSpillMem = physical;

OnError:
    return errCode;
}

static VSC_ErrCode _ProgramGprSpillMemAddr(SHADER_EXECUTABLE_PROFILE* pSEP,
                                           gctUINT startConstRegAddr,
                                           gctUINT spillMemAddr,
                                           gctUINT gprSpillSize,
                                           VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                           errCode = VSC_ERR_NONE;
    gctUINT                               channel, regAddr, i;
    SHADER_CONSTANT_HW_LOCATION_MAPPING*  pConstHwLocMapping = gcvNULL;

    for (i = 0; i < pSEP->staticPrivMapping.privUavMapping.countOfEntries; i ++)
    {
        if (pSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].commonPrivm.privmKind == SHS_PRIV_MEM_KIND_GPR_SPILLED_MEMORY)
        {
            gcmASSERT(pSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].memData.ppCnstSubArray == gcvNULL);
            gcmASSERT(pSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].memData.ppCTC == gcvNULL);

            gcmASSERT(pSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].pBuffer->hwMemAccessMode ==
                      SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR);

            pConstHwLocMapping = pSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].pBuffer->hwLoc.pHwDirectAddrBase;

            break;
        }
    }

    gcmASSERT(pConstHwLocMapping);
    gcmASSERT(pConstHwLocMapping->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER);
    if (pSEP->exeHints.derivedHints.globalStates.bEnableRobustCheck)
    {
        gctUINT upperLimit;
        /* must at start with x and at least .x.y.z channels are allocated */
        gcmASSERT((pConstHwLocMapping->validHWChannelMask & 0x07) == 0x07);
        regAddr = startConstRegAddr + (pConstHwLocMapping->hwLoc.constReg.hwRegNo * CHANNEL_NUM);
        VSC_LOAD_HW_STATE(regAddr, spillMemAddr);

        regAddr += 1;  /* .y lower limit */
        VSC_LOAD_HW_STATE(regAddr, spillMemAddr);

        regAddr += 1;  /* .z upper limit */
        upperLimit = spillMemAddr + gprSpillSize - 1;
        VSC_LOAD_HW_STATE(regAddr, upperLimit);
    }
    else
    {
        for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
        {
            if (pConstHwLocMapping->validHWChannelMask & (1 << channel))
            {
                regAddr = startConstRegAddr + (pConstHwLocMapping->hwLoc.constReg.hwRegNo * CHANNEL_NUM) + channel;
                VSC_LOAD_HW_STATE(regAddr, spillMemAddr);
            }
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _AllocVidMemForCrSpill(VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer,
                                          SHADER_EXECUTABLE_PROFILE* pSEP,
                                          gcsSURF_NODE_PTR* pCrSpillVidmemNode,
                                          gctUINT32* pVidMemAddrOfSpillMem,
                                          gctUINT * pCrSpillMemSize)

{
    VSC_ErrCode                   errCode = VSC_ERR_NONE;
    gctUINT                       crSpillSize = 0, i, channel;
    gctUINT*                      pSpillData = gcvNULL;
    SHADER_PRIV_UAV_ENTRY*        pPrivUavEntry = gcvNULL;
    SHADER_COMPILE_TIME_CONSTANT* pCTC;

    for (i = 0; i < pSEP->staticPrivMapping.privUavMapping.countOfEntries; i ++)
    {
        pPrivUavEntry = &pSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i];

        if (pPrivUavEntry->commonPrivm.privmKind == SHS_PRIV_MEM_KIND_CONSTANT_SPILLED_MEMORY)
        {
            /* Only all data of memory can be determined by vsc, we can go on */
            if (pPrivUavEntry->memData.ppCnstSubArray == gcvNULL && pPrivUavEntry->memData.ppCTC)
            {
                crSpillSize = pPrivUavEntry->pBuffer->sizeInByte;
            }

            break;
        }
    }

    if (crSpillSize > 0)
    {
        gctUINT32 physical = NOT_ASSIGNED;

        pSpillData = (gctUINT*)vscMM_Alloc(&pStatesPgmer->pmp.mmWrapper, crSpillSize);
        memset(pSpillData, 0, crSpillSize);

        *pCrSpillMemSize = crSpillSize;

        for (i = 0; i < pPrivUavEntry->memData.ctcCount; i ++)
        {
            pCTC = pPrivUavEntry->memData.ppCTC[i];

            gcmASSERT(pCTC->hwConstantLocation.hwAccessMode == SHADER_HW_ACCESS_MODE_MEMORY);
            gcmASSERT(pCTC->hwConstantLocation.hwLoc.memAddr.hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_UAV);

            if (pCTC->hwConstantLocation.hwLoc.memAddr.constantOffsetKind == SHADER_CONSTANT_OFFSET_IN_ARRAY)
            {
                if (pCTC->hwConstantLocation.validHWChannelMask == WRITEMASK_ALL)
                {
                    memcpy(pSpillData + pCTC->hwConstantLocation.hwLoc.memAddr.constantOffset,
                           pCTC->constantValue,
                           4 * CHANNEL_NUM);
                }
                else
                {
                    for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
                    {
                        if (pCTC->hwConstantLocation.validHWChannelMask & (1 << channel))
                        {
                            *(pSpillData + pCTC->hwConstantLocation.hwLoc.memAddr.constantOffset + channel) = pCTC->constantValue[channel];
                        }
                    }
                }
            }
            else if (pCTC->hwConstantLocation.hwLoc.memAddr.constantOffsetKind == SHADER_CONSTANT_OFFSET_IN_BYTE)
            {
                gctUINT8*   pData = (gctUINT8 *)pSpillData + pCTC->hwConstantLocation.hwLoc.memAddr.constantOffset;

                if (pCTC->hwConstantLocation.validHWChannelMask == WRITEMASK_ALL &&
                    pCTC->hwConstantLocation.hwLoc.memAddr.componentSizeInByte == 4)
                {
                    memcpy(pData,
                           pCTC->constantValue,
                           pCTC->hwConstantLocation.hwLoc.memAddr.componentSizeInByte * CHANNEL_NUM);
                }
                else
                {
                    for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
                    {
                        if (pCTC->hwConstantLocation.validHWChannelMask & (1 << channel))
                        {
                            memcpy((void *)(pData + channel * pCTC->hwConstantLocation.hwLoc.memAddr.componentSizeInByte),
                                   (void *)&pCTC->constantValue[channel],
                                   pCTC->hwConstantLocation.hwLoc.memAddr.componentSizeInByte);
                        }
                    }
                }
            }
        }

        (*pStatesPgmer->pSysCtx->drvCBs.pfnAllocVidMemCb)(pStatesPgmer->pSysCtx->hDrv,
                                                          gcvSURF_VERTEX,
                                                          "immediate constant spill memory",
                                                          crSpillSize,
                                                          256,
                                                          (gctPOINTER *)pCrSpillVidmemNode,
                                                          gcvNULL,
                                                          &physical,
                                                          (gctPOINTER)pSpillData,
                                                          gcvFALSE);

        if (NOT_ASSIGNED == physical)
        {
            errCode = VSC_ERR_OUT_OF_MEMORY;
            ON_ERROR(errCode, "Create cr spill vid-mem");
        }

        *pVidMemAddrOfSpillMem = physical;
    }
    else
    {
        *pVidMemAddrOfSpillMem = NOT_ASSIGNED;
    }

OnError:
    return errCode;
}

static VSC_ErrCode _ProgramCrSpillMemAddr(SHADER_EXECUTABLE_PROFILE* pSEP,
                                          gctUINT startConstRegAddr,
                                          gctUINT spillMemAddr,
                                          gctUINT spillMemSize,
                                          VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                           errCode = VSC_ERR_NONE;
    gctUINT                               channel, regAddr, i;
    SHADER_CONSTANT_HW_LOCATION_MAPPING*  pConstHwLocMapping = gcvNULL;

    for (i = 0; i < pSEP->staticPrivMapping.privUavMapping.countOfEntries; i ++)
    {
        if (pSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].commonPrivm.privmKind == SHS_PRIV_MEM_KIND_CONSTANT_SPILLED_MEMORY)
        {
            gcmASSERT(pSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].memData.ppCnstSubArray == gcvNULL);
            gcmASSERT(pSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].memData.ppCTC != gcvNULL);

            gcmASSERT(pSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].pBuffer->hwMemAccessMode ==
                      SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR);

            pConstHwLocMapping = pSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].pBuffer->hwLoc.pHwDirectAddrBase;

            break;
        }
    }

    gcmASSERT(pConstHwLocMapping);
    gcmASSERT(pConstHwLocMapping->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER);

    if (pSEP->exeHints.derivedHints.globalStates.bEnableRobustCheck)
    {
        gctUINT upperLimit;
        /* must at start with x and at least .x.y.z channels are allocated */
        gcmASSERT((pConstHwLocMapping->validHWChannelMask & 0x07) == 0x07);
        regAddr = startConstRegAddr + (pConstHwLocMapping->hwLoc.constReg.hwRegNo * CHANNEL_NUM);
        VSC_LOAD_HW_STATE(regAddr, spillMemAddr);

        regAddr += 1;  /* .y lower limit */
        VSC_LOAD_HW_STATE(regAddr, spillMemAddr);

        regAddr += 1;  /* .z upper limit */
        upperLimit = spillMemAddr + spillMemSize - 1;
        VSC_LOAD_HW_STATE(regAddr, upperLimit);
    }
    else
    {
        for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
        {
            if (pConstHwLocMapping->validHWChannelMask & (1 << channel))
            {
                regAddr = startConstRegAddr + (pConstHwLocMapping->hwLoc.constReg.hwRegNo * CHANNEL_NUM) + channel;
                VSC_LOAD_HW_STATE(regAddr, spillMemAddr);
            }
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _ProgramVsInsts(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    SHADER_EXECUTABLE_PROFILE* pVsSEP = pShHwInfo->pSEP;
    gctUINT                    pushedInstSize, thisPushInstSize;
    gctUINT                    state, hwInstBaseAddr, ftEntryIdx;
    gctUINT                    vidMemAddrOfCode = NOT_ASSIGNED;
    gctPOINTER                 instVidmemNode = gcvNULL;
    gctUINT                    startPC, endPC, shaderConfigData = 0;

    if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti5)
    {
        shaderConfigData = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) ((gctUINT32) ((pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.rtneRoundingEnabled ?
 0x1 : 0x0)) & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)));
    }
    else
    {
        shaderConfigData = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ? 28:28))) |
                           ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) ((pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.rtneRoundingEnabled ?
 0x1 : 0x0)) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)));
    }

    if (!pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.newSteeringICacheFlush)
    {
        shaderConfigData |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) |
                            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)));
    }

    if ((pShHwInfo->pSEP->exeHints.derivedHints.globalStates.memoryAccessHint & SHADER_EDH_MEM_ACCESS_HINT_ATOMIC) &&
        pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.robustAtomic)
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

    pStatesPgmer->pHints->shaderConfigData = shaderConfigData;

    if (pShHwInfo->hwProgrammingHints.hwInstFetchMode == HW_INST_FETCH_MODE_CACHE)
    {
        /* ----- I$ case ----- */

        gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasInstCache);

        (*pStatesPgmer->pSysCtx->drvCBs.pfnAllocVidMemCb)(pStatesPgmer->pSysCtx->hDrv,
                                                          gcvSURF_ICACHE,
                                                          "instruction memory for VS",
                                                          pVsSEP->countOfMCInst * sizeof(VSC_MC_RAW_INST),
                                                          256,
                                                          &instVidmemNode,
                                                          gcvNULL,
                                                          &vidMemAddrOfCode,
                                                          pVsSEP->pMachineCode,
                                                          gcvFALSE);

        if (NOT_ASSIGNED == vidMemAddrOfCode)
        {
            errCode = VSC_ERR_OUT_OF_MEMORY;
            ON_ERROR(errCode, "Create inst vid-mem");
        }

        pStatesPgmer->pHints->shaderVidNodes.instVidmemNode[gceSGSK_VERTEX_SHADER] = instVidmemNode;

        /* Program start and end PC. */
        startPC = 0;
        VSC_LOAD_HW_STATE(0x021D, startPC);
        if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti5)
        {
            endPC = pVsSEP->endPCOfMainRoutine + 1;
            VSC_LOAD_HW_STATE(0x022F, endPC);
        }
        else
        {
            endPC = pVsSEP->endPCOfMainRoutine;
            VSC_LOAD_HW_STATE(0x021E, endPC);
        }

        /* !!! Must be RIGHT before instruction state programming */
        _SetPatchOffsets(pStatesPgmer, _VSC_PATCH_OFFSET_INSTR, gceSGSK_VERTEX_SHADER);

        /* Program where to fetch from vid-mem */
        VSC_LOAD_HW_STATE(0x021B, vidMemAddrOfCode);

        /* Prepare to program cache mode */
        if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti5)
        {
            VSC_LOAD_HW_STATE(0x5580, shaderConfigData);
        }
        else
        {
            VSC_LOAD_HW_STATE(0x0218, shaderConfigData);
        }

        /* Invalidate cache lines ownered by VS */
        state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

        if (!pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.newSteeringICacheFlush)
        {
            state |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)));
        }
        VSC_LOAD_HW_STATE(0x021A, state);

        /* Prefetch inst */
        if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasInstCachePrefetch)
        {
            state = (pVsSEP->countOfMCInst - 1);
            if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti5)
            {
                VSC_LOAD_HW_STATE(0x5581, state);
            }
            else
            {
                VSC_LOAD_HW_STATE(0x0224, state);
            }

            pStatesPgmer->pHints->vsICachePrefetch[0] = 0;
            for (ftEntryIdx = 1; ftEntryIdx < GC_ICACHE_PREFETCH_TABLE_SIZE; ftEntryIdx++)
            {
                pStatesPgmer->pHints->vsICachePrefetch[ftEntryIdx] = -1;
            }
        }
    }
    else
    {
        /* ----- Register case ----- */

        gcmASSERT(!pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti5);

        /* Program start and end PC. */
        if (pShHwInfo->hwProgrammingHints.hwInstFetchMode == HW_INST_FETCH_MODE_UNUNIFIED_BUFFER)
        {
            gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxHwNativeTotalInstCount <= 256);
            gcmASSERT(pShHwInfo->hwProgrammingHints.hwInstBufferAddrOffset == 0);
            gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.vsInstBufferAddrBase == 0x1000);

            startPC = pShHwInfo->hwProgrammingHints.hwInstBufferAddrOffset;
            endPC = pShHwInfo->hwProgrammingHints.hwInstBufferAddrOffset + pVsSEP->endPCOfMainRoutine + 1;

            state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:0) - (0 ?
 11:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:0) - (0 ?
 11:0) + 1))))))) << (0 ?
 11:0))) | (((gctUINT32) ((gctUINT32) (startPC) & ((gctUINT32) ((((1 ?
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
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
            VSC_LOAD_HW_STATE(0x020E, state);

            state = endPC;
            VSC_LOAD_HW_STATE(0x0200, state);
        }
        else if (pShHwInfo->hwProgrammingHints.hwInstFetchMode == HW_INST_FETCH_MODE_UNIFIED_BUFFER_0)
        {
            gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxHwNativeTotalInstCount > 256);
            gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.vsInstBufferAddrBase == 0x3000);

            startPC = pShHwInfo->hwProgrammingHints.hwInstBufferAddrOffset;
            endPC = pShHwInfo->hwProgrammingHints.hwInstBufferAddrOffset + pVsSEP->endPCOfMainRoutine;

            /* If HW has I$, we then need use register set of I$ to program */
            if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasInstCache)
            {
                VSC_LOAD_HW_STATE(0x021D, startPC);
                VSC_LOAD_HW_STATE(0x021E, endPC);
            }
            else
            {
                state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (startPC) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (endPC) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));
                VSC_LOAD_HW_STATE(0x0217, state);
            }
        }
        else if (pShHwInfo->hwProgrammingHints.hwInstFetchMode == HW_INST_FETCH_MODE_UNIFIED_BUFFER_1)
        {
            gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxHwNativeTotalInstCount > 1024);
            gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.vsInstBufferAddrBase == 0x8000);

            startPC = pShHwInfo->hwProgrammingHints.hwInstBufferAddrOffset;
            endPC = pShHwInfo->hwProgrammingHints.hwInstBufferAddrOffset + pVsSEP->endPCOfMainRoutine;

            /* If HW has I$, we then need use register set of I$ to program */
            if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasInstCache)
            {
                VSC_LOAD_HW_STATE(0x021D, startPC);
                VSC_LOAD_HW_STATE(0x021E, endPC);
            }
            else
            {
                state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (startPC) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (endPC) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));
                VSC_LOAD_HW_STATE(0x0217, state);
            }
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }

        /* Get from where to push insts */
        hwInstBaseAddr = pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.vsInstBufferAddrBase +
                         (pShHwInfo->hwProgrammingHints.hwInstBufferAddrOffset << 2);

        if (pShHwInfo->hwProgrammingHints.hwInstFetchMode == HW_INST_FETCH_MODE_UNIFIED_BUFFER_0 ||
            pShHwInfo->hwProgrammingHints.hwInstFetchMode == HW_INST_FETCH_MODE_UNIFIED_BUFFER_1)
        {
            pStatesPgmer->pHints->unifiedStatus.instVSEnd = pVsSEP->endPCOfMainRoutine;
        }

        /* Prepare to load insts */
        VSC_LOAD_HW_STATE(0x0218, shaderConfigData);

        /* We need assure inst won't be fetched from I$ */
        if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasInstCache)
        {
            state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

            if (!pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.newSteeringICacheFlush)
            {
                state |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)));
            }

            VSC_LOAD_HW_STATE(0x021A, state)

            state = 0;
            VSC_LOAD_HW_STATE(0x021B, state);
        }

        /* Program insts now. Note that a single DMA buffer have limits to 256 instructions per buffer. */
        for (pushedInstSize = 0; pushedInstSize < pVsSEP->countOfMCInst;)
        {
            thisPushInstSize = ((pVsSEP->countOfMCInst - pushedInstSize) > DMA_BATCH_SIZE_LIMIT) ?
                                                                           DMA_BATCH_SIZE_LIMIT  :
                                                          (pVsSEP->countOfMCInst - pushedInstSize);

            VSC_LOAD_HW_STATES((hwInstBaseAddr + pushedInstSize*VSC_MC_INST_DWORD_SIZE),
                               (pVsSEP->pMachineCode + pushedInstSize*VSC_MC_INST_DWORD_SIZE),
                               thisPushInstSize*VSC_MC_INST_DWORD_SIZE);

            pushedInstSize += thisPushInstSize;
        }
    }

OnError:
    return errCode;
}

static gctUINT _GetVsStartConstRegAddr(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    return pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.vsConstRegAddrBase +
           pShHwInfo->hwProgrammingHints.hwConstantRegAddrOffset * CHANNEL_NUM;
}

static VSC_ErrCode _ProgramVsCompileTimeConsts(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    gctUINT                    state, startConstRegAddr;
    VSC_ErrCode                errCode = VSC_ERR_NONE;

    if (pShHwInfo->hwProgrammingHints.hwConstantFetchMode == HW_CONSTANT_FETCH_MODE_UNIFIED_REG_FILE)
    {
        gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxHwNativeTotalConstRegCount > 256);
        gcmASSERT((pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.vsConstRegAddrBase == 0xC000) ||
                  (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.vsConstRegAddrBase == 0xD000));

        state = pShHwInfo->hwProgrammingHints.hwConstantRegAddrOffset;
        VSC_LOAD_HW_STATE(0x0219, state);

        if (!pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.newSteeringICacheFlush)
        {
            state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)));
            VSC_LOAD_HW_STATE(0x0218, state);
        }
    }

    startConstRegAddr = _GetVsStartConstRegAddr(pShHwInfo, pStatesPgmer);

    _ProgramConstCountInfo(pShHwInfo, pStatesPgmer, gcvTRUE);

    pStatesPgmer->pHints->hwConstRegBases[gcvPROGRAM_STAGE_VERTEX] = startConstRegAddr * 4;

    pStatesPgmer->pHints->constRegNoBase[gcvPROGRAM_STAGE_COMPUTE] =
    pStatesPgmer->pHints->constRegNoBase[gcvPROGRAM_STAGE_OPENCL] =
    pStatesPgmer->pHints->constRegNoBase[gcvPROGRAM_STAGE_VERTEX] = pShHwInfo->hwProgrammingHints.hwConstantRegAddrOffset;

    errCode = _ProgramRegedCTC(pShHwInfo,
                               startConstRegAddr,
                               pStatesPgmer);
    ON_ERROR(errCode, "Program VS Reged CTC");

OnError:
    return errCode;
}

static VSC_ErrCode _ProgramVsGprSpill(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    SHADER_EXECUTABLE_PROFILE* pVsSEP = pShHwInfo->pSEP;
    gcsSURF_NODE_PTR           gprSpillVidmemNode = gcvNULL;
    gctUINT                    vidMemAddrOfSpillMem = NOT_ASSIGNED;
    gctUINT                    gprSpillSize = 0;

    errCode = _AllocVidMemForGprSpill(pStatesPgmer, pVsSEP, &gprSpillVidmemNode, &vidMemAddrOfSpillMem, &gprSpillSize);
    ON_ERROR(errCode, "Alloc vid-mem for gpr spill");

    pStatesPgmer->pHints->shaderVidNodes.gprSpillVidmemNode[gceSGSK_VERTEX_SHADER] = gprSpillVidmemNode;

    _SetPatchOffsets(pStatesPgmer, _VSC_PATCH_OFFSET_GPRSPILL, gceSGSK_VERTEX_SHADER);

    errCode = _ProgramGprSpillMemAddr(pVsSEP,
                                      _GetVsStartConstRegAddr(pShHwInfo, pStatesPgmer),
                                      vidMemAddrOfSpillMem,
                                      gprSpillSize,
                                      pStatesPgmer);
    ON_ERROR(errCode, "Program GPR spill mem address ");

OnError:
    return errCode;
}

static VSC_ErrCode _ProgramVsCrSpill(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    SHADER_EXECUTABLE_PROFILE* pVsSEP = pShHwInfo->pSEP;
    gcsSURF_NODE_PTR           crSpillVidmemNode = gcvNULL;
    gctUINT                    vidMemAddrOfSpillMem = NOT_ASSIGNED;
    gctUINT                    crSpillMemSize = 0;

    errCode = _AllocVidMemForCrSpill(pStatesPgmer, pVsSEP, &crSpillVidmemNode, &vidMemAddrOfSpillMem, &crSpillMemSize);
    ON_ERROR(errCode, "Alloc vid-mem for cr spill");

    if (vidMemAddrOfSpillMem != NOT_ASSIGNED)
    {
        pStatesPgmer->pHints->shaderVidNodes.crSpillVidmemNode[gceSGSK_VERTEX_SHADER] = crSpillVidmemNode;

        _SetPatchOffsets(pStatesPgmer, _VSC_PATCH_OFFSET_CRSPILL, gceSGSK_VERTEX_SHADER);

        errCode = _ProgramCrSpillMemAddr(pVsSEP,
                                         _GetVsStartConstRegAddr(pShHwInfo, pStatesPgmer),
                                         vidMemAddrOfSpillMem,
                                         crSpillMemSize,
                                         pStatesPgmer);
        ON_ERROR(errCode, "Program CR spill mem address ");
    }

OnError:
    return errCode;
}

static gctUINT _GetCalibratedGprCount(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    gctUINT   calibratedGprCount = pShHwInfo->pSEP->gprCount;

    if (pShHwInfo->hwProgrammingHints.hwInstFetchMode == HW_INST_FETCH_MODE_CACHE &&
        !pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasICacheAllocCountFix)
    {
        calibratedGprCount = vscMAX(calibratedGprCount, 3);
    }

    return calibratedGprCount;
}

static void _GetMinMaxUscSize(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer,
                              gctUINT* pMinUscSize, gctUINT* pMaxUscSize, gctUINT* pExtraUscSize)
{
    gcmASSERT(pShHwInfo->hwProgrammingHints.minUscSizeInKbyte <= pShHwInfo->hwProgrammingHints.maxUscSizeInKbyte);

    if (pMinUscSize)
    {
#if ENABLE_USC_DYNAMICALLY_ALLOC
        *pMinUscSize = pShHwInfo->hwProgrammingHints.minUscSizeInKbyte + 1;
#else
        *pMinUscSize = pShHwInfo->hwProgrammingHints.maxUscSizeInKbyte + 1;
#endif
    }

    if (pMaxUscSize)
    {
#if ENABLE_USC_DYNAMICALLY_ALLOC
        *pMaxUscSize = pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxUSCAttribBufInKbyte;
#else
        *pMaxUscSize = pShHwInfo->hwProgrammingHints.maxUscSizeInKbyte + 1;
#endif
    }

    if (pExtraUscSize)
    {
#if ENABLE_USC_DYNAMICALLY_ALLOC
        *pExtraUscSize = pShHwInfo->hwProgrammingHints.maxUscSizeInKbyte - pShHwInfo->hwProgrammingHints.minUscSizeInKbyte;
#else
        *pExtraUscSize = 0;
#endif
    }
}

static VSC_ErrCode _ProgramVS(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    SHADER_EXECUTABLE_PROFILE* pVsSEP = pShHwInfo->pSEP;
    SHADER_IO_LINKAGE_INFO*    pVSInputLinkageInfo = &pShHwInfo->inputLinkageInfo;
    SHADER_IO_LINKAGE_INFO*    pVSOutputLinkageInfo = &pShHwInfo->outputLinkageInfo;
    gctUINT                    state, ioIdx, index, shift, hwRegNo;
    gctUINT                    timeoutofFE2VS, effectiveOutputCount, validChannelCount;
    gctUINT                    vsInputCount = 0, vsOutputCount = 0, vsOutputCompCount = 0;
    gctUINT                    soOnlyOutputCount = 0, dummyOutputCount = 0;
    gctUINT                    baseOutputRegMapping[MAX_HW_IO_COUNT/4] = {0, 0, 0, 0, 0, 0, 0, 0};
    gctUINT                    vsInput16RegNo = 0, ptSzLinkNo = NOT_ASSIGNED;
    gctUINT                    hwRegNoForVtxId = NOT_ASSIGNED, hwRegNoForInstanceId = NOT_ASSIGNED;
    gctUINT                    firstValidIoChannel, hwThreadGroupSize, resultCacheWinSize;
    gctUINT                    calibratedGprCount = _GetCalibratedGprCount(pShHwInfo, pStatesPgmer);
    gctBOOL                    bHasStreamOutedOutput = gcvFALSE;
    gctUINT                    minUscSize, maxUscSize, extraUscSize;

    gcmASSERT(pVsSEP->inputMapping.ioVtxPxl.ioMode == SHADER_IO_MODE_PASSIVE);
    gcmASSERT(pVsSEP->outputMapping.ioVtxPxl.ioMode == SHADER_IO_MODE_PASSIVE);
    gcmASSERT(!pVsSEP->exeHints.derivedHints.globalStates.bIoUSCAddrsPackedToOneReg);

    /* Input */
    for (ioIdx = 0; ioIdx < pVsSEP->inputMapping.ioVtxPxl.countOfIoRegMapping; ioIdx ++)
    {
        if (pVsSEP->inputMapping.ioVtxPxl.ioIndexMask & (1LL << ioIdx))
        {
            firstValidIoChannel = pVsSEP->inputMapping.ioVtxPxl.pIoRegMapping[ioIdx].firstValidIoChannel;

            if (pVsSEP->inputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_VERTEXID].ioIndexMask & (1LL << ioIdx))
            {
                hwRegNoForVtxId = pVsSEP->inputMapping.ioVtxPxl.pIoRegMapping[ioIdx].ioChannelMapping[firstValidIoChannel].
                                  hwLoc.cmnHwLoc.u.hwRegNo;
            }

            if (pVsSEP->inputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_INSTANCEID].ioIndexMask & (1LL << ioIdx))
            {
                hwRegNoForInstanceId = pVsSEP->inputMapping.ioVtxPxl.pIoRegMapping[ioIdx].ioChannelMapping[firstValidIoChannel].
                                       hwLoc.cmnHwLoc.u.hwRegNo;
            }

            if (pVSInputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].linkNo == MAX_BASIC_VS_HW_INPUTS)
            {
                vsInput16RegNo = pVsSEP->inputMapping.ioVtxPxl.pIoRegMapping[ioIdx].ioChannelMapping[firstValidIoChannel].
                                 hwLoc.cmnHwLoc.u.hwRegNo;

                gcmASSERT(vsInput16RegNo == pVSInputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].linkNo);
            }

            vsInputCount ++;
        }
    }

    if (!pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.supportZeroAttrsInFE &&
        (hwRegNoForVtxId < SPECIAL_HW_IO_REG_NO || hwRegNoForInstanceId < SPECIAL_HW_IO_REG_NO) &&
        (vsInputCount == 1))
    {
        /* For the HW that does not support zero attributes and vs has only vertexid/instanceid input, we
           need add a dummy input just before this vertexid/instanceid input (RA and linker has considered
           it), that means input count will be 2 */
        gcmASSERT(pVSInputLinkageInfo->vtxPxlLinkage.totalLinkNoCount == 2);
        vsInputCount = pVSInputLinkageInfo->vtxPxlLinkage.totalLinkNoCount;
    }
    else
    {
        /* Need consider input link count is different with IO count because currently we only
           support IOs without usages mixed */
        vsInputCount = vscMIN(vsInputCount, pVSInputLinkageInfo->vtxPxlLinkage.totalLinkNoCount);
    }

    /* Following code needs match logic of FE stream programmming */
    if (vsInputCount == 0)
    {
        gcmASSERT((hwRegNoForVtxId == NOT_ASSIGNED) && (hwRegNoForInstanceId == NOT_ASSIGNED));

        vsInputCount ++;

        if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.supportZeroAttrsInFE)
        {
            hwRegNoForVtxId = 0;
        }
    }

    if (vsInputCount >  ((hwRegNoForVtxId < SPECIAL_HW_IO_REG_NO || hwRegNoForInstanceId < SPECIAL_HW_IO_REG_NO) ?
                         pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxAttributeCount + MAX_EXT_VS_HW_INPUTS :
                         pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxAttributeCount))
    {
        return VSC_ERR_TOO_MANY_ATTRIBUTES;
    }

    /* Output */
    for (ioIdx = 0; ioIdx < pVsSEP->outputMapping.ioVtxPxl.countOfIoRegMapping; ioIdx ++)
    {
        if (pVsSEP->outputMapping.ioVtxPxl.ioIndexMask & (1LL << ioIdx))
        {
            /* Only consider linked one */
            if (pVSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].linkNo != NOT_ASSIGNED)
            {
                if (vsOutputCount >= (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxVaryingCount + 1) ||
                    pVSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].linkNo >=
                    (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxVaryingCount + 1))
                {
                    return VSC_ERR_TOO_MANY_VARYINGS;
                }

                bHasStreamOutedOutput |= (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.supportStreamOut &&
                                          (((1LL << ioIdx) & pVsSEP->outputMapping.ioVtxPxl.soIoIndexMask) != 0));

                validChannelCount = _GetValidHwRegChannelCount(&pVsSEP->outputMapping.ioVtxPxl.pIoRegMapping[ioIdx],
                                                               pVsSEP->outputMapping.ioVtxPxl.ioMemAlign);
                gcmASSERT(validChannelCount <= CHANNEL_NUM);

                vsOutputCompCount += validChannelCount;

                /* Get HW register number, NOTE in current implementation all channels
                   must be in same HW register number */
                firstValidIoChannel = pVsSEP->outputMapping.ioVtxPxl.pIoRegMapping[ioIdx].firstValidIoChannel;
                hwRegNo = pVsSEP->outputMapping.ioVtxPxl.pIoRegMapping[ioIdx].ioChannelMapping[firstValidIoChannel].
                          hwLoc.cmnHwLoc.u.hwRegNo;
                gcmASSERT(hwRegNo != NOT_ASSIGNED);

                if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.newGPIPE &&
                    pVsSEP->outputMapping.ioVtxPxl.ioMode == SHADER_IO_MODE_PASSIVE)
                {
                    /* Convert link into output reg mapping array index and shift */
                    index = pVSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].linkNo >> 2;
                    shift = (pVSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].linkNo & 3) * 8;

                    /* Copy register into output reg mapping array. */
                    baseOutputRegMapping[index] |= (hwRegNo << shift);
                }
                else
                {
                    if (pVSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].linkNo < MAX_BASIC_VS_HW_OUTPUTS)
                    {
                        /* Convert link into output reg mapping array index and shift */
                        index = pVSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].linkNo >> 2;
                        shift = (pVSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].linkNo & 3) * 8;

                        /* Copy register into output reg mapping array. */
                        baseOutputRegMapping[index] |= (hwRegNo << shift);
                    }
                    else
                    {
                        if (pVSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].linkNo == 16)
                        {
                            pStatesPgmer->pHints->vsOutput16RegNo = hwRegNo;
                        }
                        else if (pVSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].linkNo == 17)
                        {
                            pStatesPgmer->pHints->vsOutput17RegNo = hwRegNo;
                        }
                        else if (pVSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].linkNo == 18)
                        {
                            pStatesPgmer->pHints->vsOutput18RegNo = hwRegNo;
                        }
                        else
                        {
                            gcmASSERT(gcvFALSE);
                        }
                    }
                }

                if (pVSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].bOnlyLinkToSO)
                {
                    soOnlyOutputCount ++;
                }

                if (pVSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].bIsDummyLink)
                {
                    gcmASSERT(pVsSEP->outputMapping.ioVtxPxl.pIoRegMapping[ioIdx].regIoMode == SHADER_IO_MODE_ACTIVE);

                    dummyOutputCount ++;
                }

                 if (pVsSEP->outputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_POINTSIZE].ioIndexMask & (1LL << ioIdx))
                 {
                     ptSzLinkNo = pVSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].linkNo;
                 }

                /* Ok, increase count */
                vsOutputCount ++;
            }
        }
    }

    /* For the cases that need dummy outputs for VS (generally, these outputs should be generated by HW,
       but our HW does not), so outputs of shader and outputs to PA are different, we need pick the right
       one here */
    vsOutputCount = vscMAX(vsOutputCount, pVSOutputLinkageInfo->vtxPxlLinkage.totalLinkNoCount);

    /* Output must include position (explicit or implicit) and pointSize if present, just following the driver bypass mode. */
    if (vsOutputCount == 0)
    {
        vsOutputCount++;
        if (pVsSEP->outputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_POINTSIZE].ioIndexMask != 0)
        {
            vsOutputCount++;
        }
    }

    _GetMinMaxUscSize(pShHwInfo, pStatesPgmer, &minUscSize, &maxUscSize, &extraUscSize);

    /* Fill hints */
    pStatesPgmer->pHints->vsHasPointSize = (pVsSEP->outputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_POINTSIZE].ioIndexMask != 0);
    pStatesPgmer->pHints->isPtSizeStreamedOut = (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.supportStreamOut &&
                                                 ((pVsSEP->outputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_POINTSIZE].ioIndexMask &
                                                   pVsSEP->outputMapping.ioVtxPxl.soIoIndexMask) != 0));
    pStatesPgmer->pHints->hasAttrStreamOuted = bHasStreamOutedOutput;
    pStatesPgmer->pHints->vsPtSizeAtLastLinkLoc = (ptSzLinkNo == (pVSOutputLinkageInfo->vtxPxlLinkage.totalLinkNoCount - 1));
    pStatesPgmer->pHints->vsInstCount = pVsSEP->countOfMCInst;
    pStatesPgmer->pHints->vsOutputCount  = vsOutputCount;
#if gcdUSE_WCLIP_PATCH
    pStatesPgmer->pHints->vsPositionZDependsOnW = pVsSEP->exeHints.derivedHints.prePaStates.bOutPosZDepW;
#endif
    pStatesPgmer->pHints->vsMaxTemp = calibratedGprCount;
    pStatesPgmer->pHints->extraUscPages += extraUscSize;
    pStatesPgmer->pHints->memoryAccessFlags[gcvSHADER_HIGH_LEVEL][gcvPROGRAM_STAGE_VERTEX] = (gceMEMORY_ACCESS_FLAG)pVsSEP->exeHints.nativeHints.globalStates.memoryAccessHint;
    pStatesPgmer->pHints->memoryAccessFlags[gcvSHADER_MACHINE_LEVEL][gcvPROGRAM_STAGE_VERTEX] = (gceMEMORY_ACCESS_FLAG)pVsSEP->exeHints.derivedHints.globalStates.memoryAccessHint;
    pStatesPgmer->pHints->flowControlFlags[gcvSHADER_HIGH_LEVEL][gcvPROGRAM_STAGE_VERTEX] = (gceFLOW_CONTROL_FLAG)pVsSEP->exeHints.nativeHints.globalStates.flowControlHint;
    pStatesPgmer->pHints->flowControlFlags[gcvSHADER_MACHINE_LEVEL][gcvPROGRAM_STAGE_VERTEX] = (gceFLOW_CONTROL_FLAG)pVsSEP->exeHints.derivedHints.globalStates.flowControlHint;
    pStatesPgmer->pHints->texldFlags[gcvSHADER_HIGH_LEVEL][gcvPROGRAM_STAGE_VERTEX] = (gceTEXLD_FLAG)pVsSEP->exeHints.nativeHints.globalStates.texldHint;
    pStatesPgmer->pHints->texldFlags[gcvSHADER_MACHINE_LEVEL][gcvPROGRAM_STAGE_VERTEX] = (gceTEXLD_FLAG)pVsSEP->exeHints.derivedHints.globalStates.texldHint;
    pStatesPgmer->pHints->samplerBaseOffset[gcvPROGRAM_STAGE_VERTEX] = pShHwInfo->hwProgrammingHints.hwSamplerRegAddrOffset;

    _ProgramSamplerCountInfo(pShHwInfo, pStatesPgmer, gcvTRUE);

    pStatesPgmer->pHints->vsUseStoreAttr = (pVsSEP->outputMapping.ioVtxPxl.ioMode == SHADER_IO_MODE_ACTIVE);
    pStatesPgmer->pHints->prePaShaderHasPointSize = pStatesPgmer->pHints->vsHasPointSize;
    pStatesPgmer->pHints->prePaShaderHasPrimitiveId = (pVsSEP->outputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_PRIMITIVEID].ioIndexMask != 0);
    pStatesPgmer->pHints->shader2PaOutputCount = vsOutputCount - soOnlyOutputCount - dummyOutputCount;

    if (pStatesPgmer->pHints->prePaShaderHasPointSize)
    {
        pStatesPgmer->pHints->ptSzAttrIndex = pVSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[pVsSEP->outputMapping.ioVtxPxl.
                                                                            usage2IO[SHADER_IO_USAGE_POINTSIZE].mainIoIndex].linkNo;

    }

    /* Why no chip model check????????? */
    effectiveOutputCount = (!pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.outputCountFix) ?
                            gcmALIGN(vsOutputCount, 2)                            :
                            (((vsOutputCompCount + 7) >> 2) +
                             ((pStatesPgmer->pHints->vsHasPointSize && ((vsOutputCompCount % 4 )) == 0) ? 1 : 0)
                            );

    pStatesPgmer->pHints->balanceMin = ((256 *
                                            10  *
                                            8   / /* Pipe line depth. */
                                            (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.vertexOutputBufferSize -
                                             effectiveOutputCount * pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.vertexCacheSize
                                            )
                                         )
                                            + 9
                                       ) / 10;
    pStatesPgmer->pHints->balanceMax = gcmMIN(255,
                                              2048 / (pShHwInfo->hwProgrammingHints.maxThreadsPerHwTG *
                                                      effectiveOutputCount));

    /* Program input */
    timeoutofFE2VS = gcmALIGN(vsInputCount * 4 + 4, 16) / 16;
    if (hwRegNoForVtxId < SPECIAL_HW_IO_REG_NO || hwRegNoForInstanceId < SPECIAL_HW_IO_REG_NO)
    {
        gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.vtxInstanceIdAsAttr);

        state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
    else
    {
        state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)));
    }
    state |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (vsInputCount) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));
    state |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) ((gctUINT32) (timeoutofFE2VS) & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)));
    state |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:16) - (0 ?
 21:16) + 1))))))) << (0 ?
 21:16))) | (((gctUINT32) ((gctUINT32) (vsInput16RegNo) & ((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ? 21:16)));
    VSC_LOAD_HW_STATE(0x0202, state);

    if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.newGPIPE)
    {
        /* Program output */
        VSC_LOAD_HW_STATES(0x0238, baseOutputRegMapping, ((vsOutputCount + 3) / 4));

        /* Program attribute layout registers */
        hwThreadGroupSize = pShHwInfo->hwProgrammingHints.maxThreadsPerHwTG * vsOutputCount;
        state  = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (vsOutputCount) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0))) |
                 ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:8) - (0 ?
 18:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:8) - (0 ?
 18:8) + 1))))))) << (0 ?
 18:8))) | (((gctUINT32) ((gctUINT32) (hwThreadGroupSize) & ((gctUINT32) ((((1 ?
 18:8) - (0 ?
 18:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:8) - (0 ? 18:8) + 1))))))) << (0 ? 18:8)));
        VSC_LOAD_HW_STATE(0x021C, state);

        /* Program USC registers */
        resultCacheWinSize = pShHwInfo->hwProgrammingHints.resultCacheWindowSize;
        state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (maxUscSize) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0))) |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:12) - (0 ?
 19:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:12) - (0 ?
 19:12) + 1))))))) << (0 ?
 19:12))) | (((gctUINT32) ((gctUINT32) (pShHwInfo->hwProgrammingHints.maxThreadsPerHwTG) & ((gctUINT32) ((((1 ?
 19:12) - (0 ?
 19:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:12) - (0 ? 19:12) + 1))))))) << (0 ? 19:12)))|
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:20) - (0 ?
 28:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:20) - (0 ?
 28:20) + 1))))))) << (0 ?
 28:20))) | (((gctUINT32) ((gctUINT32) (resultCacheWinSize) & ((gctUINT32) ((((1 ?
 28:20) - (0 ?
 28:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:20) - (0 ? 28:20) + 1))))))) << (0 ? 28:20)));
        VSC_LOAD_HW_STATE(0x0228, state);

        if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.multiCluster)
        {
            state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (minUscSize) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));
            VSC_LOAD_HW_STATE(0x5582, state);
        }
    }
    else
    {
        /* Program output */
        VSC_LOAD_HW_STATES(0x0204, baseOutputRegMapping,
            (((vsOutputCount > MAX_BASIC_VS_HW_OUTPUTS ? MAX_BASIC_VS_HW_OUTPUTS : vsOutputCount) + 3) / 4));
    }

    /* Program used GPR count */
    state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:0) - (0 ?
 6:0) + 1))))))) << (0 ?
 6:0))) | (((gctUINT32) ((gctUINT32) (calibratedGprCount) & ((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:0) - (0 ? 6:0) + 1))))))) << (0 ? 6:0)));
    VSC_LOAD_HW_STATE(0x0203, state);

    /* Program SO */
    if (pVsSEP->outputMapping.ioVtxPxl.soIoIndexMask != 0 && pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.supportStreamOut)
    {
        errCode = _ProgramStreamOut(pShHwInfo, &pVsSEP->outputMapping.ioVtxPxl,
                                    &pVSOutputLinkageInfo->vtxPxlLinkage, pStatesPgmer);
        ON_ERROR(errCode, "Program SO");
    }

    /* Program sampler-related. */
    if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasSamplerBaseOffset)
    {
        state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:0) - (0 ?
 6:0) + 1))))))) << (0 ?
 6:0))) | (((gctUINT32) ((gctUINT32) (pShHwInfo->hwProgrammingHints.hwSamplerRegAddrOffset) & ((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:0) - (0 ? 6:0) + 1))))))) << (0 ? 6:0)));
        VSC_LOAD_HW_STATE(0x022A, state);
    }

    /* Program CTCs */
    errCode = _ProgramVsCompileTimeConsts(pShHwInfo, pStatesPgmer);
    ON_ERROR(errCode, "Program VS CTC");

    /* Program insts */
    errCode = _ProgramVsInsts(pShHwInfo, pStatesPgmer);
    ON_ERROR(errCode, "Program VS inst");

    /* Program gpr spill */
    if (pVsSEP->exeHints.derivedHints.globalStates.bGprSpilled)
    {
        pStatesPgmer->pHints->useGPRSpill[gcvPROGRAM_STAGE_VERTEX] = gcvTRUE;
        errCode = _ProgramVsGprSpill(pShHwInfo, pStatesPgmer);
        ON_ERROR(errCode, "Program VS grp spill");
    }

    /* Program cr spill */
    if (pVsSEP->exeHints.derivedHints.globalStates.bCrSpilled)
    {
        if (DECODE_SHADER_CLIENT(pVsSEP->shVersionType) == SHADER_CLIENT_VK)
        {
            errCode = _ProgramVsCrSpill(pShHwInfo, pStatesPgmer);
            ON_ERROR(errCode, "Program VS cr spill");
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _ProgramHsInsts(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    SHADER_EXECUTABLE_PROFILE* pHsSEP = pShHwInfo->pSEP;
    gctUINT                    state, ftEntryIdx;
    gctUINT                    vidMemAddrOfCode = NOT_ASSIGNED;
    gctPOINTER                 instVidmemNode = gcvNULL;
    gctUINT                    startPC, endPC;

    gcmASSERT(pShHwInfo->hwProgrammingHints.hwInstFetchMode == HW_INST_FETCH_MODE_CACHE);
    gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasInstCache);

    (*pStatesPgmer->pSysCtx->drvCBs.pfnAllocVidMemCb)(pStatesPgmer->pSysCtx->hDrv,
                                                      gcvSURF_ICACHE,
                                                      "instruction Memory for HS",
                                                      pHsSEP->countOfMCInst * sizeof(VSC_MC_RAW_INST),
                                                      256,
                                                      &instVidmemNode,
                                                      gcvNULL,
                                                      &vidMemAddrOfCode,
                                                      pHsSEP->pMachineCode,
                                                      gcvFALSE);

    if (NOT_ASSIGNED == vidMemAddrOfCode)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        ON_ERROR(errCode, "Create inst vid-mem");
    }

    pStatesPgmer->pHints->shaderVidNodes.instVidmemNode[gceSGSK_TC_SHADER] = instVidmemNode;

    /* Program start and end PC. */
    startPC = 0;
    endPC = pHsSEP->endPCOfMainRoutine + 1;
    VSC_LOAD_HW_STATE(0x5280, startPC);
    VSC_LOAD_HW_STATE(0x5281, endPC);

    /* !!! Must be RIGHT before instruction state programming */
    _SetPatchOffsets(pStatesPgmer, _VSC_PATCH_OFFSET_INSTR, gceSGSK_TC_SHADER);

    /* Program where to fetch from vid-mem */
    VSC_LOAD_HW_STATE(0x5282, vidMemAddrOfCode);

    state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

    VSC_LOAD_HW_STATE(0x021A, state);

    /* Invalidate cache lines ownered by HS */
    gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.newSteeringICacheFlush);

    /* Prefetch inst */
    if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasInstCachePrefetch)
    {
        state = (pHsSEP->countOfMCInst - 1);

        VSC_LOAD_HW_STATE(0x5284, state);

        pStatesPgmer->pHints->tcsICachePrefetch[0] = 0;
        for (ftEntryIdx = 1; ftEntryIdx < GC_ICACHE_PREFETCH_TABLE_SIZE; ftEntryIdx++)
        {
            pStatesPgmer->pHints->tcsICachePrefetch[ftEntryIdx] = -1;
        }
    }

OnError:
    return errCode;
}

static gctUINT _GetHsStartConstRegAddr(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    gctUINT constRegBaseAddr;

    if (pShHwInfo->pSEP->exeHints.derivedHints.globalStates.unifiedConstRegAllocStrategy == UNIFIED_RF_ALLOC_STRATEGY_UNIFIED)
    {
        constRegBaseAddr = pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.vsConstRegAddrBase;
    }
    else
    {
        constRegBaseAddr = pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.tcsConstRegAddrBase;
    }

    constRegBaseAddr += pShHwInfo->hwProgrammingHints.hwConstantRegAddrOffset * CHANNEL_NUM;

    return constRegBaseAddr;
}

static VSC_ErrCode _ProgramHsCompileTimeConsts(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    gctUINT                    state, startConstRegAddr;
    VSC_ErrCode                errCode = VSC_ERR_NONE;

    if (pShHwInfo->hwProgrammingHints.hwConstantFetchMode == HW_CONSTANT_FETCH_MODE_UNIFIED_REG_FILE)
    {
        gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxHwNativeTotalConstRegCount > 256);
        gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.tcsConstRegAddrBase == 0xD000);

        state = pShHwInfo->hwProgrammingHints.hwConstantRegAddrOffset;
        VSC_LOAD_HW_STATE(0x5291, state);
    }

    startConstRegAddr = _GetHsStartConstRegAddr(pShHwInfo, pStatesPgmer);

    _ProgramConstCountInfo(pShHwInfo, pStatesPgmer, gcvTRUE);

    pStatesPgmer->pHints->hwConstRegBases[gcvPROGRAM_STAGE_TCS] = startConstRegAddr * 4;
    pStatesPgmer->pHints->constRegNoBase[gcvPROGRAM_STAGE_TCS] = pShHwInfo->hwProgrammingHints.hwConstantRegAddrOffset;

    errCode = _ProgramRegedCTC(pShHwInfo,
                               startConstRegAddr,
                               pStatesPgmer);
    ON_ERROR(errCode, "Program HS Reged CTC");

OnError:
    return errCode;
}

static VSC_ErrCode _ProgramHsGprSpill(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    SHADER_EXECUTABLE_PROFILE* pHsSEP = pShHwInfo->pSEP;
    gcsSURF_NODE_PTR           gprSpillVidmemNode = gcvNULL;
    gctUINT                    vidMemAddrOfSpillMem = NOT_ASSIGNED;
    gctUINT                    gprSpillSize = 0;

    errCode = _AllocVidMemForGprSpill(pStatesPgmer, pHsSEP, &gprSpillVidmemNode, &vidMemAddrOfSpillMem, &gprSpillSize);
    ON_ERROR(errCode, "Alloc vid-mem for gpr spill");

    pStatesPgmer->pHints->shaderVidNodes.gprSpillVidmemNode[gceSGSK_TC_SHADER] = gprSpillVidmemNode;

    _SetPatchOffsets(pStatesPgmer, _VSC_PATCH_OFFSET_GPRSPILL, gceSGSK_TC_SHADER);

    errCode = _ProgramGprSpillMemAddr(pHsSEP,
                                      _GetHsStartConstRegAddr(pShHwInfo, pStatesPgmer),
                                      vidMemAddrOfSpillMem,
                                      gprSpillSize,
                                      pStatesPgmer);
    ON_ERROR(errCode, "Program GPR spill mem address ");

OnError:
    return errCode;
}

static VSC_ErrCode _ProgramHsCrSpill(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    SHADER_EXECUTABLE_PROFILE* pHsSEP = pShHwInfo->pSEP;
    gcsSURF_NODE_PTR           crSpillVidmemNode = gcvNULL;
    gctUINT                    vidMemAddrOfSpillMem = NOT_ASSIGNED;
    gctUINT                    crSpillMemSize = 0;

    errCode = _AllocVidMemForCrSpill(pStatesPgmer, pHsSEP, &crSpillVidmemNode, &vidMemAddrOfSpillMem, &crSpillMemSize);
    ON_ERROR(errCode, "Alloc vid-mem for cr spill");

    if (vidMemAddrOfSpillMem != NOT_ASSIGNED)
    {
        pStatesPgmer->pHints->shaderVidNodes.crSpillVidmemNode[gceSGSK_TC_SHADER] = crSpillVidmemNode;

        _SetPatchOffsets(pStatesPgmer, _VSC_PATCH_OFFSET_CRSPILL, gceSGSK_TC_SHADER);

        errCode = _ProgramCrSpillMemAddr(pHsSEP,
                                         _GetHsStartConstRegAddr(pShHwInfo, pStatesPgmer),
                                         vidMemAddrOfSpillMem,
                                         crSpillMemSize,
                                         pStatesPgmer);
        ON_ERROR(errCode, "Program CR spill mem address ");
    }

OnError:
    return errCode;
}

static VSC_ErrCode _ProgramTPG(SHADER_EXECUTABLE_PROFILE* pTsSEP, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    gctUINT                    state;
    gctUINT                    outputPrim;

    static gctUINT const tsDomainMap[] =
    {
        0x2,
        0x0,
        0x1
    };

    static gctUINT const tsPartitionMap[] =
    {
        0x0,
        0x1,
        0x2,
        0x3
    };

    static gctUINT const tsOutputPrimMapOGL[] =
    {
        0x0,
        0x1,
        0x2,
        0x3
    };

    static gctUINT const tsOutputPrimMapVulkan[] =
    {
        0x0,
        0x1,
        0x3,
        0x2
    };

    gcmASSERT(pTsSEP->exeHints.nativeHints.prvStates.ts.tessDomainType <= SHADER_TESSELLATOR_DOMAIN_QUAD);
    gcmASSERT(pTsSEP->exeHints.nativeHints.prvStates.ts.tessPartitionType <= SHADER_TESSELLATOR_PARTITION_FRACTIONAL_EVEN);
    gcmASSERT(pTsSEP->exeHints.nativeHints.prvStates.ts.tessOutputPrim <= SHADER_TESSELLATOR_OUTPUT_PRIM_TRIANGLE_CCW);

    if (DECODE_SHADER_CLIENT(pTsSEP->shVersionType) == SHADER_CLIENT_VK)
    {
        outputPrim = tsOutputPrimMapVulkan[pTsSEP->exeHints.nativeHints.prvStates.ts.tessOutputPrim];
    }
    else
    {
        outputPrim = tsOutputPrimMapOGL[pTsSEP->exeHints.nativeHints.prvStates.ts.tessOutputPrim];
    }

    state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (tsDomainMap[pTsSEP->exeHints.nativeHints.prvStates.ts.tessDomainType]) & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))          |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (tsPartitionMap[pTsSEP->exeHints.nativeHints.prvStates.ts.tessPartitionType]) & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:8) - (0 ?
 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (outputPrim) & ((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:8) - (0 ?
 9:8) + 1))))))) << (0 ?
 9:8)))                                                                 |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:12) - (0 ?
 18:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:12) - (0 ?
 18:12) + 1))))))) << (0 ?
 18:12))) | (((gctUINT32) ((gctUINT32) (pTsSEP->exeHints.nativeHints.prvStates.ts.maxTessFactor) & ((gctUINT32) ((((1 ?
 18:12) - (0 ?
 18:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:12) - (0 ? 18:12) + 1))))))) << (0 ? 18:12)))                  |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:20) - (0 ?
 26:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:20) - (0 ?
 26:20) + 1))))))) << (0 ?
 26:20))) | (((gctUINT32) ((gctUINT32) (32) & ((gctUINT32) ((((1 ?
 26:20) - (0 ?
 26:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:20) - (0 ? 26:20) + 1))))))) << (0 ? 26:20)));

    if (gcoHAL_GetOption(gcvNULL, gcvOPTION_PREFER_TPG_TRIVIALMODEL))
    {
        state |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ? 28:28)));
    }

    VSC_LOAD_HW_STATE(0x52C0, state);

OnError:
    return errCode;
}

extern gctUINT _GetHsRemapMode(SHADER_EXECUTABLE_PROFILE* pHsSEP,
                               gctUINT hsPerCPInputCount,
                               gctUINT hsPerCPOutputCount,
                               gctBOOL* pIsInputRemapMode)
{
    gctUINT remapMode;

    if (hsPerCPInputCount == 0 && hsPerCPOutputCount == 0)
    {
        /* Just set REMAP_INPUT as HS has no vertex input and output */
        remapMode = 0x0;
    }
    else if (pHsSEP->exeHints.derivedHints.globalStates.bIoUSCAddrsPackedToOneReg)
    {
        gcmASSERT(pHsSEP->inputMapping.ioVtxPxl.ioMode == SHADER_IO_MODE_ACTIVE ||
                  pHsSEP->inputMapping.ioVtxPxl.countOfIoRegMapping == 0);
        gcmASSERT(pHsSEP->outputMapping.ioVtxPxl.ioMode == SHADER_IO_MODE_ACTIVE ||
                  pHsSEP->outputMapping.ioVtxPxl.countOfIoRegMapping == 0);

        /*gcmASSERT(pHsSEP->exeHints.nativeHints.prvStates.ts.inputCtrlPointCount <= 4);*/
        gcmASSERT(pHsSEP->exeHints.nativeHints.prvStates.ts.outputCtrlPointCount <= 4);

        remapMode = 0x2;
    }
    else if (pHsSEP->outputMapping.ioVtxPxl.ioMode == SHADER_IO_MODE_ACTIVE)
    {
        if (pHsSEP->exeHints.derivedHints.prvStates.hs.bPerCpOutputsUsedByOtherThreads)
        {
            remapMode = 0x1;
        }
        else
        {
            remapMode = 0x0;
        }
    }
    else
    {
        remapMode = 0x0;
    }

    if (pIsInputRemapMode)
    {
        *pIsInputRemapMode = (remapMode == 0x0);
    }

    return remapMode;
}

extern gctUINT _GetHsValidMaxPatchesPerHwTG(SHADER_EXECUTABLE_PROFILE* pHsSEP,
                                            gctUINT maxHwTGThreadCount,
                                            gctBOOL bIsInputRemap,
                                            gctUINT maxParallelFactor)
{
    gctUINT maxPatchesPerHwTG;

    maxPatchesPerHwTG = vscMIN(maxHwTGThreadCount / pHsSEP->exeHints.nativeHints.prvStates.ts.outputCtrlPointCount,
                               (maxHwTGThreadCount * 8)/(bIsInputRemap ?
                               pHsSEP->exeHints.nativeHints.prvStates.ts.inputCtrlPointCount :
                               (pHsSEP->exeHints.nativeHints.prvStates.ts.inputCtrlPointCount +
                                pHsSEP->exeHints.nativeHints.prvStates.ts.outputCtrlPointCount)));
    maxPatchesPerHwTG = vscMIN(maxPatchesPerHwTG, maxParallelFactor);

    return (maxPatchesPerHwTG == 0) ? 1 : maxPatchesPerHwTG;
}

static VSC_ErrCode _ProgramHS(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    SHADER_EXECUTABLE_PROFILE* pHsSEP = pShHwInfo->pSEP;
    SHADER_IO_LINKAGE_INFO*    pHSOutputLinkageInfo = &pShHwInfo->outputLinkageInfo;
    gctUINT                    state, ioIdx, index, hwRegNo, i;
    gctUINT                    hsPerCPInputCount = 0, hsPerCPOutputCount = 0, hsPerPatchOutputCount = 0;
    gctUINT                    perCPOutputRegMapping[MAX_HW_IO_COUNT];
    gctUINT                    firstValidIoChannel, totalCPOutputCount, hsPerCPOutputAttrCount;
    gctUINT                    remapMode, maxPatchesPerHwTG, totalOutputCountPerhwTG;
    gctINT                     maxPerPatchOutputLinkNo = -1;
    gctUINT                    minUscSize, maxUscSize, extraUscSize;

    gcmASSERT(pHsSEP->inputMapping.ioVtxPxl.ioMode == SHADER_IO_MODE_ACTIVE ||
              pHsSEP->inputMapping.ioVtxPxl.countOfIoRegMapping == 0);
    gcmASSERT(pHsSEP->outputMapping.ioPrim.ioMode == SHADER_IO_MODE_ACTIVE ||
              pHsSEP->outputMapping.ioPrim.countOfIoRegMapping == 0);
#if !PROGRAMING_STATES_FOR_SEPERATED_PROGRAM
    gcmASSERT(pHSOutputLinkageInfo->linkedShaderStage == SHADER_TYPE_DOMAIN);
#endif

    memset(&perCPOutputRegMapping[0], 0, sizeof(gctUINT)*MAX_HW_IO_COUNT);

    /* Input */
    for (ioIdx = 0; ioIdx < pHsSEP->inputMapping.ioVtxPxl.countOfIoRegMapping; ioIdx ++)
    {
        if (pHsSEP->inputMapping.ioVtxPxl.ioIndexMask & (1LL << ioIdx))
        {
            hsPerCPInputCount ++;
        }
    }

    if (hsPerCPInputCount > pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxAttributeCount)
    {
        return VSC_ERR_TOO_MANY_ATTRIBUTES;
    }

    /* Output */
    for (ioIdx = 0; ioIdx < pHsSEP->outputMapping.ioVtxPxl.countOfIoRegMapping; ioIdx ++)
    {
        if (pHsSEP->outputMapping.ioVtxPxl.ioIndexMask & (1LL << ioIdx))
        {
            /* Only consider linked one */
            if (pHSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].linkNo != NOT_ASSIGNED)
            {
                if (hsPerCPOutputCount >= (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxVaryingCount + 1) ||
                    pHSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].linkNo >= (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxVaryingCount + 1))
                {
                    return VSC_ERR_TOO_MANY_VARYINGS;
                }

                if (pHsSEP->outputMapping.ioVtxPxl.ioMode == SHADER_IO_MODE_PASSIVE)
                {
                    /* Get HW register number, NOTE in current implementation all channels
                       must be in same HW register number */
                    firstValidIoChannel = pHsSEP->outputMapping.ioVtxPxl.pIoRegMapping[ioIdx].firstValidIoChannel;
                    hwRegNo = pHsSEP->outputMapping.ioVtxPxl.pIoRegMapping[ioIdx].ioChannelMapping[firstValidIoChannel].
                              hwLoc.cmnHwLoc.u.hwRegNo;
                    gcmASSERT(hwRegNo != NOT_ASSIGNED);

                    /* Convert link into output reg mapping array index */
                    index = pHSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].linkNo;

                    /* Copy register into output reg mapping array. */
                    perCPOutputRegMapping[index] = hwRegNo;
                }

                /* Ok, increase count */
                hsPerCPOutputCount ++;
            }
        }
    }

    for (ioIdx = 0; ioIdx < pHsSEP->outputMapping.ioPrim.countOfIoRegMapping; ioIdx ++)
    {
        if (pHsSEP->outputMapping.ioPrim.ioIndexMask & (1LL << ioIdx))
        {
            /* Only consider linked one */
            if (pHSOutputLinkageInfo->primLinkage.ioRegLinkage[ioIdx].linkNo != NOT_ASSIGNED)
            {
                if (maxPerPatchOutputLinkNo < (gctINT)pHSOutputLinkageInfo->primLinkage.ioRegLinkage[ioIdx].linkNo)
                {
                    maxPerPatchOutputLinkNo = pHSOutputLinkageInfo->primLinkage.ioRegLinkage[ioIdx].linkNo;
                }

                /* Ok, increase count */
                hsPerPatchOutputCount ++;
            }
        }
    }

    _GetMinMaxUscSize(pShHwInfo, pStatesPgmer, &minUscSize, &maxUscSize, &extraUscSize);

    pStatesPgmer->pHints->tcsPerVertexAttributeCount = hsPerCPInputCount;
    pStatesPgmer->pHints->extraUscPages += extraUscSize;
    pStatesPgmer->pHints->memoryAccessFlags[gcvSHADER_HIGH_LEVEL][gcvPROGRAM_STAGE_TCS] = (gceMEMORY_ACCESS_FLAG)pHsSEP->exeHints.nativeHints.globalStates.memoryAccessHint;
    pStatesPgmer->pHints->memoryAccessFlags[gcvSHADER_MACHINE_LEVEL][gcvPROGRAM_STAGE_TCS] = (gceMEMORY_ACCESS_FLAG)pHsSEP->exeHints.derivedHints.globalStates.memoryAccessHint;
    pStatesPgmer->pHints->flowControlFlags[gcvSHADER_HIGH_LEVEL][gcvPROGRAM_STAGE_TCS] = (gceFLOW_CONTROL_FLAG)pHsSEP->exeHints.nativeHints.globalStates.flowControlHint;
    pStatesPgmer->pHints->flowControlFlags[gcvSHADER_MACHINE_LEVEL][gcvPROGRAM_STAGE_TCS] = (gceFLOW_CONTROL_FLAG)pHsSEP->exeHints.derivedHints.globalStates.flowControlHint;
    pStatesPgmer->pHints->texldFlags[gcvSHADER_HIGH_LEVEL][gcvPROGRAM_STAGE_TCS] = (gceTEXLD_FLAG)pHsSEP->exeHints.nativeHints.globalStates.texldHint;
    pStatesPgmer->pHints->texldFlags[gcvSHADER_MACHINE_LEVEL][gcvPROGRAM_STAGE_TCS] = (gceTEXLD_FLAG)pHsSEP->exeHints.derivedHints.globalStates.texldHint;
    pStatesPgmer->pHints->samplerBaseOffset[gcvPROGRAM_STAGE_TCS] = pShHwInfo->hwProgrammingHints.hwSamplerRegAddrOffset;

    _ProgramSamplerCountInfo(pShHwInfo, pStatesPgmer, gcvTRUE);

    /* Need consider output link count is different with IO count because currently we only
       support IOs without usages mixed */
    hsPerPatchOutputCount = (hsPerPatchOutputCount == (gctUINT)(maxPerPatchOutputLinkNo + 1)) ?
                            hsPerPatchOutputCount : (gctUINT)(maxPerPatchOutputLinkNo + 1);
    hsPerPatchOutputCount = (hsPerPatchOutputCount < 2) ? 2 : hsPerPatchOutputCount;

    gcmASSERT(pHsSEP->exeHints.derivedHints.globalStates.hwStartRegNoForUSCAddrs == 1);

    remapMode = _GetHsRemapMode(pHsSEP, hsPerCPInputCount, hsPerCPOutputCount, gcvNULL);

    maxPatchesPerHwTG = _GetHsValidMaxPatchesPerHwTG(pHsSEP, pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxCoreCount * 4,
                                                     (remapMode == 0x0),
                                                     pShHwInfo->hwProgrammingHints.maxParallelFactor);
    totalCPOutputCount = hsPerCPOutputCount * pHsSEP->exeHints.nativeHints.prvStates.ts.outputCtrlPointCount;
    totalOutputCountPerhwTG = (totalCPOutputCount + hsPerPatchOutputCount) * maxPatchesPerHwTG;

    /* Program TPG control */
    if (DECODE_SHADER_CLIENT(pHsSEP->shVersionType) == SHADER_CLIENT_DX)
    {
        errCode = _ProgramTPG(pHsSEP, pStatesPgmer);
        ON_ERROR(errCode, "Program TPG");
    }

    /* Program HS control */
    hsPerCPOutputAttrCount = (pHsSEP->outputMapping.ioVtxPxl.ioMode == SHADER_IO_MODE_ACTIVE) ? 0 : hsPerCPOutputCount;
    state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:12) - (0 ?
 18:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:12) - (0 ?
 18:12) + 1))))))) << (0 ?
 18:12))) | (((gctUINT32) ((gctUINT32) (_GetCalibratedGprCount(pShHwInfo, pStatesPgmer)) & ((gctUINT32) ((((1 ?
 18:12) - (0 ?
 18:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:12) - (0 ? 18:12) + 1))))))) << (0 ? 18:12))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:20) - (0 ?
 25:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:20) - (0 ?
 25:20) + 1))))))) << (0 ?
 25:20))) | (((gctUINT32) ((gctUINT32) (hsPerCPOutputAttrCount) & ((gctUINT32) ((((1 ?
 25:20) - (0 ?
 25:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:20) - (0 ? 25:20) + 1))))))) << (0 ? 25:20)))                   |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:8) - (0 ?
 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (remapMode) & ((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:8) - (0 ?
 9:8) + 1))))))) << (0 ? 9:8)))                                            |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (pHsSEP->exeHints.nativeHints.prvStates.ts.outputCtrlPointCount) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));
    VSC_LOAD_HW_STATE(0x5285, state);

    /* Program output */
    for (i = 0; i < MAX_HW_IO_COUNT/4; i ++)
    {
        state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (perCPOutputRegMapping[i * 4 + 0]) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0))) |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:8) - (0 ?
 13:8) + 1))))))) << (0 ?
 13:8))) | (((gctUINT32) ((gctUINT32) (perCPOutputRegMapping[i * 4 + 1]) & ((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ? 13:8))) |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:16) - (0 ?
 21:16) + 1))))))) << (0 ?
 21:16))) | (((gctUINT32) ((gctUINT32) (perCPOutputRegMapping[i * 4 + 2]) & ((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ? 21:16))) |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:24) - (0 ?
 29:24) + 1))))))) << (0 ?
 29:24))) | (((gctUINT32) ((gctUINT32) (perCPOutputRegMapping[i * 4 + 3]) & ((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ? 29:24)));

        VSC_LOAD_HW_STATE(0x5288 + i, state);
    }

    /* Program attribute layout registers */
    state  = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (hsPerCPOutputCount) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)))                 |
             ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:8) - (0 ?
 18:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:8) - (0 ?
 18:8) + 1))))))) << (0 ?
 18:8))) | (((gctUINT32) ((gctUINT32) (totalCPOutputCount) & ((gctUINT32) ((((1 ?
 18:8) - (0 ?
 18:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:8) - (0 ? 18:8) + 1))))))) << (0 ? 18:8)))                   |
             ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:20) - (0 ?
 30:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:20) - (0 ?
 30:20) + 1))))))) << (0 ?
 30:20))) | (((gctUINT32) ((gctUINT32) (totalCPOutputCount + hsPerPatchOutputCount) & ((gctUINT32) ((((1 ?
 30:20) - (0 ?
 30:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:20) - (0 ? 30:20) + 1))))))) << (0 ? 30:20))) |
             ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)));
    VSC_LOAD_HW_STATE(0x5287, state);

    /* Program attribute-ex layout registers */
    state  = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:0) - (0 ?
 10:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:0) - (0 ?
 10:0) + 1))))))) << (0 ?
 10:0))) | (((gctUINT32) ((gctUINT32) (totalOutputCountPerhwTG) & ((gctUINT32) ((((1 ?
 10:0) - (0 ?
 10:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:0) - (0 ? 10:0) + 1))))))) << (0 ? 10:0)));
    VSC_LOAD_HW_STATE(0x5290, state);

    /* Program USC registers */
    state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (maxUscSize) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:20) - (0 ?
 25:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:20) - (0 ?
 25:20) + 1))))))) << (0 ?
 25:20))) | (((gctUINT32) ((gctUINT32) (minUscSize) & ((gctUINT32) ((((1 ?
 25:20) - (0 ?
 25:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:20) - (0 ? 25:20) + 1))))))) << (0 ? 25:20))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:12) - (0 ?
 19:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:12) - (0 ?
 19:12) + 1))))))) << (0 ?
 19:12))) | (((gctUINT32) ((gctUINT32) (maxPatchesPerHwTG) & ((gctUINT32) ((((1 ?
 19:12) - (0 ?
 19:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:12) - (0 ? 19:12) + 1))))))) << (0 ? 19:12)));
    VSC_LOAD_HW_STATE(0x5286, state);

    /* Program sampler-related. */
    if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasSamplerBaseOffset)
    {
        state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:0) - (0 ?
 6:0) + 1))))))) << (0 ?
 6:0))) | (((gctUINT32) ((gctUINT32) (pShHwInfo->hwProgrammingHints.hwSamplerRegAddrOffset) & ((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:0) - (0 ? 6:0) + 1))))))) << (0 ? 6:0)));
        VSC_LOAD_HW_STATE(0x5293, state);
    }

    /* Program CTCs */
    errCode = _ProgramHsCompileTimeConsts(pShHwInfo, pStatesPgmer);
    ON_ERROR(errCode, "Program HS CTC");

    /* Program insts */
    errCode = _ProgramHsInsts(pShHwInfo, pStatesPgmer);
    ON_ERROR(errCode, "Program HS inst");

    /* Program gpr spill */
    if (pHsSEP->exeHints.derivedHints.globalStates.bGprSpilled)
    {
        pStatesPgmer->pHints->useGPRSpill[gcvPROGRAM_STAGE_TCS] = gcvTRUE;
        errCode = _ProgramHsGprSpill(pShHwInfo, pStatesPgmer);
        ON_ERROR(errCode, "Program HS grp spill");
    }

    /* Program cr spill */
    if (pHsSEP->exeHints.derivedHints.globalStates.bCrSpilled)
    {
        if (DECODE_SHADER_CLIENT(pHsSEP->shVersionType) == SHADER_CLIENT_VK)
        {
            errCode = _ProgramHsCrSpill(pShHwInfo, pStatesPgmer);
            ON_ERROR(errCode, "Program HS cr spill");
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _ProgramDsInsts(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    SHADER_EXECUTABLE_PROFILE* pDsSEP = pShHwInfo->pSEP;
    gctUINT                    state, ftEntryIdx;
    gctUINT                    vidMemAddrOfCode = NOT_ASSIGNED;
    gctPOINTER                 instVidmemNode = gcvNULL;
    gctUINT                    startPC, endPC;

    gcmASSERT(pShHwInfo->hwProgrammingHints.hwInstFetchMode == HW_INST_FETCH_MODE_CACHE);
    gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasInstCache);

    (*pStatesPgmer->pSysCtx->drvCBs.pfnAllocVidMemCb)(pStatesPgmer->pSysCtx->hDrv,
                                                      gcvSURF_ICACHE,
                                                      "instruction Memory for DS",
                                                      pDsSEP->countOfMCInst * sizeof(VSC_MC_RAW_INST),
                                                      256,
                                                      &instVidmemNode,
                                                      gcvNULL,
                                                      &vidMemAddrOfCode,
                                                      pDsSEP->pMachineCode,
                                                      gcvFALSE);

    if (NOT_ASSIGNED == vidMemAddrOfCode)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        ON_ERROR(errCode, "Create inst vid-mem");
    }

    pStatesPgmer->pHints->shaderVidNodes.instVidmemNode[gceSGSK_TE_SHADER] = instVidmemNode;

    /* Program start and end PC. */
    startPC = 0;
    endPC = pDsSEP->endPCOfMainRoutine + 1;
    VSC_LOAD_HW_STATE(0x52C1, startPC);
    VSC_LOAD_HW_STATE(0x52C2, endPC);

    /* !!! Must be RIGHT before instruction state programming */
    _SetPatchOffsets(pStatesPgmer, _VSC_PATCH_OFFSET_INSTR, gceSGSK_TE_SHADER);

    /* Program where to fetch from vid-mem */
    VSC_LOAD_HW_STATE(0x52C3, vidMemAddrOfCode);

    state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

    VSC_LOAD_HW_STATE(0x021A, state);

    /* Invalidate cache lines ownered by DS */
    gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.newSteeringICacheFlush);

    /* Prefetch inst */
    if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasInstCachePrefetch)
    {
        state = (pDsSEP->countOfMCInst - 1);
        VSC_LOAD_HW_STATE(0x52C5, state);

        pStatesPgmer->pHints->tesICachePrefetch[0] = 0;
        for (ftEntryIdx = 1; ftEntryIdx < GC_ICACHE_PREFETCH_TABLE_SIZE; ftEntryIdx++)
        {
            pStatesPgmer->pHints->tesICachePrefetch[ftEntryIdx] = -1;
        }
    }

OnError:
    return errCode;
}

static gctUINT _GetDsStartConstRegAddr(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    gctUINT constRegBaseAddr;

    if (pShHwInfo->pSEP->exeHints.derivedHints.globalStates.unifiedConstRegAllocStrategy == UNIFIED_RF_ALLOC_STRATEGY_UNIFIED)
    {
        constRegBaseAddr = pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.vsConstRegAddrBase;
    }
    else
    {
        constRegBaseAddr = pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.tesConstRegAddrBase;
    }

    constRegBaseAddr += pShHwInfo->hwProgrammingHints.hwConstantRegAddrOffset * CHANNEL_NUM;

    return constRegBaseAddr;
}

static VSC_ErrCode _ProgramDsCompileTimeConsts(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    gctUINT                    state, startConstRegAddr;
    VSC_ErrCode                errCode = VSC_ERR_NONE;

    if (pShHwInfo->hwProgrammingHints.hwConstantFetchMode == HW_CONSTANT_FETCH_MODE_UNIFIED_REG_FILE)
    {
        gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxHwNativeTotalConstRegCount > 256);
        gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.tesConstRegAddrBase == 0xD000);

        state = pShHwInfo->hwProgrammingHints.hwConstantRegAddrOffset;
        VSC_LOAD_HW_STATE(0x52C9, state);
    }

    startConstRegAddr = _GetDsStartConstRegAddr(pShHwInfo, pStatesPgmer);

    _ProgramConstCountInfo(pShHwInfo, pStatesPgmer, gcvTRUE);

    pStatesPgmer->pHints->hwConstRegBases[gcvPROGRAM_STAGE_TES] = startConstRegAddr * 4;
    pStatesPgmer->pHints->constRegNoBase[gcvPROGRAM_STAGE_TES] = pShHwInfo->hwProgrammingHints.hwConstantRegAddrOffset;

    errCode = _ProgramRegedCTC(pShHwInfo,
                               startConstRegAddr,
                               pStatesPgmer);
    ON_ERROR(errCode, "Program DS Reged CTC");

OnError:
    return errCode;
}

static VSC_ErrCode _ProgramDsGprSpill(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    SHADER_EXECUTABLE_PROFILE* pDsSEP = pShHwInfo->pSEP;
    gcsSURF_NODE_PTR           gprSpillVidmemNode = gcvNULL;
    gctUINT                    vidMemAddrOfSpillMem = NOT_ASSIGNED;
    gctUINT                    gprSpillSize = 0;

    errCode = _AllocVidMemForGprSpill(pStatesPgmer, pDsSEP, &gprSpillVidmemNode, &vidMemAddrOfSpillMem, &gprSpillSize);
    ON_ERROR(errCode, "Alloc vid-mem for gpr spill");

    pStatesPgmer->pHints->shaderVidNodes.gprSpillVidmemNode[gceSGSK_TE_SHADER] = gprSpillVidmemNode;

    _SetPatchOffsets(pStatesPgmer, _VSC_PATCH_OFFSET_GPRSPILL, gceSGSK_TE_SHADER);

    errCode = _ProgramGprSpillMemAddr(pDsSEP,
                                      _GetDsStartConstRegAddr(pShHwInfo, pStatesPgmer),
                                      vidMemAddrOfSpillMem,
                                      gprSpillSize,
                                      pStatesPgmer);
    ON_ERROR(errCode, "Program GPR spill mem address ");

OnError:
    return errCode;
}

static VSC_ErrCode _ProgramDsCrSpill(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    SHADER_EXECUTABLE_PROFILE* pDsSEP = pShHwInfo->pSEP;
    gcsSURF_NODE_PTR           crSpillVidmemNode = gcvNULL;
    gctUINT                    vidMemAddrOfSpillMem = NOT_ASSIGNED;
    gctUINT                    crSpillMemSize = 0;

    errCode = _AllocVidMemForCrSpill(pStatesPgmer, pDsSEP, &crSpillVidmemNode, &vidMemAddrOfSpillMem, &crSpillMemSize);
    ON_ERROR(errCode, "Alloc vid-mem for cr spill");

    if (vidMemAddrOfSpillMem != NOT_ASSIGNED)
    {
        pStatesPgmer->pHints->shaderVidNodes.crSpillVidmemNode[gceSGSK_TE_SHADER] = crSpillVidmemNode;

        _SetPatchOffsets(pStatesPgmer, _VSC_PATCH_OFFSET_CRSPILL, gceSGSK_TE_SHADER);

        errCode = _ProgramCrSpillMemAddr(pDsSEP,
                                         _GetDsStartConstRegAddr(pShHwInfo, pStatesPgmer),
                                         vidMemAddrOfSpillMem,
                                         crSpillMemSize,
                                         pStatesPgmer);
        ON_ERROR(errCode, "Program CR spill mem address ");
    }

OnError:
    return errCode;
}

static VSC_ErrCode _ProgramDS(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    SHADER_EXECUTABLE_PROFILE* pDsSEP = pShHwInfo->pSEP;
    SHADER_IO_LINKAGE_INFO*    pDSOutputLinkageInfo = &pShHwInfo->outputLinkageInfo;
    gctUINT                    state, ioIdx, index, hwRegNo, i;
    gctUINT                    dsPerCPInputCount = 0, dsOutputCount = 0, dsOutputAttrCount;
    gctUINT                    soOnlyOutputCount = 0, dummyOutputCount = 0;
    gctUINT                    outputRegMapping[MAX_HW_IO_COUNT];
    gctUINT                    firstValidIoChannel, hwThreadGroupSize, resultCacheWinSize;
    gctUINT                    minUscSize, maxUscSize, extraUscSize;

    gcmASSERT(pDsSEP->inputMapping.ioVtxPxl.ioMode == SHADER_IO_MODE_ACTIVE ||
              pDsSEP->inputMapping.ioVtxPxl.countOfIoRegMapping == 0);
    gcmASSERT(pDsSEP->inputMapping.ioPrim.ioMode == SHADER_IO_MODE_ACTIVE ||
              pDsSEP->inputMapping.ioPrim.countOfIoRegMapping <= 1);
    gcmASSERT(pDsSEP->outputMapping.ioVtxPxl.ioMode == SHADER_IO_MODE_PASSIVE);
    gcmASSERT(!pDsSEP->exeHints.derivedHints.globalStates.bIoUSCAddrsPackedToOneReg);
#if !PROGRAMING_STATES_FOR_SEPERATED_PROGRAM
    gcmASSERT(pShHwInfo->inputLinkageInfo.linkedShaderStage == SHADER_TYPE_HULL);
#endif

    memset(&outputRegMapping[0], 0, sizeof(gctUINT)*MAX_HW_IO_COUNT);

    /* Input */
    for (ioIdx = 0; ioIdx < pDsSEP->inputMapping.ioVtxPxl.countOfIoRegMapping; ioIdx ++)
    {
        if (pDsSEP->inputMapping.ioVtxPxl.ioIndexMask & (1LL << ioIdx))
        {
            dsPerCPInputCount ++;
        }
    }

    if (dsPerCPInputCount > pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxAttributeCount)
    {
        return VSC_ERR_TOO_MANY_ATTRIBUTES;
    }

    /* Output */
    for (ioIdx = 0; ioIdx < pDsSEP->outputMapping.ioVtxPxl.countOfIoRegMapping; ioIdx ++)
    {
        if (pDsSEP->outputMapping.ioVtxPxl.ioIndexMask & (1LL << ioIdx))
        {
            /* Only consider linked one */
            if (pDSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].linkNo != NOT_ASSIGNED)
            {
                if (dsOutputCount >= (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxVaryingCount + 1) ||
                    pDSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].linkNo >=
                    (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxVaryingCount + 1))
                {
                    return VSC_ERR_TOO_MANY_VARYINGS;
                }

                if (pDsSEP->outputMapping.ioVtxPxl.ioMode == SHADER_IO_MODE_PASSIVE)
                {
                    /* Get HW register number, NOTE in current implementation all channels
                       must be in same HW register number */
                    firstValidIoChannel = pDsSEP->outputMapping.ioVtxPxl.pIoRegMapping[ioIdx].firstValidIoChannel;
                    hwRegNo = pDsSEP->outputMapping.ioVtxPxl.pIoRegMapping[ioIdx].ioChannelMapping[firstValidIoChannel].
                              hwLoc.cmnHwLoc.u.hwRegNo;
                    gcmASSERT(hwRegNo != NOT_ASSIGNED);

                    /* Convert link into output reg mapping array index */
                    index = pDSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].linkNo;

                    /* Copy register into output reg mapping array. */
                    outputRegMapping[index] = hwRegNo;
                }

                if (pDSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].bOnlyLinkToSO)
                {
                    soOnlyOutputCount ++;
                }

                if (pDSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].bIsDummyLink)
                {
                    gcmASSERT(pDsSEP->outputMapping.ioVtxPxl.pIoRegMapping[ioIdx].regIoMode == SHADER_IO_MODE_ACTIVE);

                    dummyOutputCount ++;
                }

                /* Ok, increase count */
                dsOutputCount ++;
            }
        }
    }

    /* For the cases that need dummy outputs for DS (generally, these outputs should be generated by HW,
       but our HW does not), so outputs of shader and outputs to PA are different, we need pick the right
       one here */
    dsOutputCount = vscMAX(dsOutputCount, pDSOutputLinkageInfo->vtxPxlLinkage.totalLinkNoCount);

    /* Output must include position (explicit or implicit) */
    if (dsOutputCount == 0)
    {
        dsOutputCount ++;
    }

    _GetMinMaxUscSize(pShHwInfo, pStatesPgmer, &minUscSize, &maxUscSize, &extraUscSize);

    pStatesPgmer->pHints->extraUscPages += extraUscSize;
    pStatesPgmer->pHints->prePaShaderHasPointSize = (pDsSEP->outputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_POINTSIZE].ioIndexMask != 0);
    pStatesPgmer->pHints->prePaShaderHasPrimitiveId = (pDsSEP->outputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_PRIMITIVEID].ioIndexMask != 0);
    pStatesPgmer->pHints->shader2PaOutputCount = dsOutputCount - soOnlyOutputCount - dummyOutputCount;
    pStatesPgmer->pHints->memoryAccessFlags[gcvSHADER_HIGH_LEVEL][gcvPROGRAM_STAGE_TES] = (gceMEMORY_ACCESS_FLAG)pDsSEP->exeHints.nativeHints.globalStates.memoryAccessHint;
    pStatesPgmer->pHints->memoryAccessFlags[gcvSHADER_MACHINE_LEVEL][gcvPROGRAM_STAGE_TES] = (gceMEMORY_ACCESS_FLAG)pDsSEP->exeHints.derivedHints.globalStates.memoryAccessHint;
    pStatesPgmer->pHints->flowControlFlags[gcvSHADER_HIGH_LEVEL][gcvPROGRAM_STAGE_TES] = (gceFLOW_CONTROL_FLAG)pDsSEP->exeHints.nativeHints.globalStates.flowControlHint;
    pStatesPgmer->pHints->flowControlFlags[gcvSHADER_MACHINE_LEVEL][gcvPROGRAM_STAGE_TES] = (gceFLOW_CONTROL_FLAG)pDsSEP->exeHints.derivedHints.globalStates.flowControlHint;
    pStatesPgmer->pHints->texldFlags[gcvSHADER_HIGH_LEVEL][gcvPROGRAM_STAGE_TES] = (gceTEXLD_FLAG)pDsSEP->exeHints.nativeHints.globalStates.texldHint;
    pStatesPgmer->pHints->texldFlags[gcvSHADER_MACHINE_LEVEL][gcvPROGRAM_STAGE_TES] = (gceTEXLD_FLAG)pDsSEP->exeHints.derivedHints.globalStates.texldHint;
    pStatesPgmer->pHints->samplerBaseOffset[gcvPROGRAM_STAGE_TES] = pShHwInfo->hwProgrammingHints.hwSamplerRegAddrOffset;

    _ProgramSamplerCountInfo(pShHwInfo, pStatesPgmer, gcvTRUE);

    if (pStatesPgmer->pHints->prePaShaderHasPointSize)
    {
        pStatesPgmer->pHints->ptSzAttrIndex = pDSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[pDsSEP->outputMapping.ioVtxPxl.
                                                                            usage2IO[SHADER_IO_USAGE_POINTSIZE].mainIoIndex].linkNo;
    }

    /* Program TPG control */
    if (DECODE_SHADER_CLIENT(pDsSEP->shVersionType) != SHADER_CLIENT_DX)
    {
        errCode = _ProgramTPG(pDsSEP, pStatesPgmer);
        ON_ERROR(errCode, "Program TPG");
    }

    /* Program DS control */
    dsOutputAttrCount = (pDsSEP->outputMapping.ioVtxPxl.ioMode == SHADER_IO_MODE_ACTIVE) ? 0 : dsOutputCount;
    state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:12) - (0 ?
 18:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:12) - (0 ?
 18:12) + 1))))))) << (0 ?
 18:12))) | (((gctUINT32) ((gctUINT32) (_GetCalibratedGprCount(pShHwInfo, pStatesPgmer)) & ((gctUINT32) ((((1 ?
 18:12) - (0 ?
 18:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:12) - (0 ? 18:12) + 1))))))) << (0 ? 18:12))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:4) - (0 ?
 9:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:4) - (0 ?
 9:4) + 1))))))) << (0 ?
 9:4))) | (((gctUINT32) ((gctUINT32) (dsOutputAttrCount) & ((gctUINT32) ((((1 ?
 9:4) - (0 ?
 9:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:4) - (0 ? 9:4) + 1))))))) << (0 ? 9:4)))                        |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));
    VSC_LOAD_HW_STATE(0x52C6, state);

    /* Program output */
    for (i = 0; i < MAX_HW_IO_COUNT/4; i ++)
    {
        state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (outputRegMapping[i * 4 + 0]) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0))) |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:8) - (0 ?
 13:8) + 1))))))) << (0 ?
 13:8))) | (((gctUINT32) ((gctUINT32) (outputRegMapping[i * 4 + 1]) & ((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ? 13:8))) |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:16) - (0 ?
 21:16) + 1))))))) << (0 ?
 21:16))) | (((gctUINT32) ((gctUINT32) (outputRegMapping[i * 4 + 2]) & ((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ? 21:16))) |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:24) - (0 ?
 29:24) + 1))))))) << (0 ?
 29:24))) | (((gctUINT32) ((gctUINT32) (outputRegMapping[i * 4 + 3]) & ((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ? 29:24)));

        VSC_LOAD_HW_STATE(0x52D0 + i, state);
    }

    /* Program attribute layout registers */
    hwThreadGroupSize = pShHwInfo->hwProgrammingHints.maxThreadsPerHwTG * dsOutputCount;
    state  = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (dsOutputCount) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0))) |
             ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:8) - (0 ?
 18:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:8) - (0 ?
 18:8) + 1))))))) << (0 ?
 18:8))) | (((gctUINT32) ((gctUINT32) (hwThreadGroupSize) & ((gctUINT32) ((((1 ?
 18:8) - (0 ?
 18:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:8) - (0 ? 18:8) + 1))))))) << (0 ? 18:8)));
    VSC_LOAD_HW_STATE(0x52C8, state);

    /* Program USC registers */
    resultCacheWinSize = pShHwInfo->hwProgrammingHints.resultCacheWindowSize;
    state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (maxUscSize) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:12) - (0 ?
 19:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:12) - (0 ?
 19:12) + 1))))))) << (0 ?
 19:12))) | (((gctUINT32) ((gctUINT32) (pShHwInfo->hwProgrammingHints.maxThreadsPerHwTG) & ((gctUINT32) ((((1 ?
 19:12) - (0 ?
 19:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:12) - (0 ? 19:12) + 1))))))) << (0 ? 19:12)))|
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:20) - (0 ?
 28:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:20) - (0 ?
 28:20) + 1))))))) << (0 ?
 28:20))) | (((gctUINT32) ((gctUINT32) (resultCacheWinSize) & ((gctUINT32) ((((1 ?
 28:20) - (0 ?
 28:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:20) - (0 ? 28:20) + 1))))))) << (0 ? 28:20)));
    VSC_LOAD_HW_STATE(0x52C7, state);

    /* Program USC extra registers */
    state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:8) - (0 ?
 13:8) + 1))))))) << (0 ?
 13:8))) | (((gctUINT32) ((gctUINT32) (minUscSize) & ((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ? 13:8))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:0) - (0 ?
 7:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:0) - (0 ?
 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 7:0) - (0 ?
 7:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ? 7:0)));
    VSC_LOAD_HW_STATE(0x52CD, state);

    /* Program SO */
    if (pDsSEP->outputMapping.ioVtxPxl.soIoIndexMask != 0 && pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.supportStreamOut)
    {
        errCode = _ProgramStreamOut(pShHwInfo, &pDsSEP->outputMapping.ioVtxPxl,
                                    &pDSOutputLinkageInfo->vtxPxlLinkage, pStatesPgmer);
        ON_ERROR(errCode, "Program SO");
    }

    /* Program sampler-related. */
    if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasSamplerBaseOffset)
    {
        state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:0) - (0 ?
 6:0) + 1))))))) << (0 ?
 6:0))) | (((gctUINT32) ((gctUINT32) (pShHwInfo->hwProgrammingHints.hwSamplerRegAddrOffset) & ((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:0) - (0 ? 6:0) + 1))))))) << (0 ? 6:0)));
        VSC_LOAD_HW_STATE(0x52CB, state);
    }

    /* Program CTCs */
    errCode = _ProgramDsCompileTimeConsts(pShHwInfo, pStatesPgmer);
    ON_ERROR(errCode, "Program DS CTC");

    /* Program insts */
    errCode = _ProgramDsInsts(pShHwInfo, pStatesPgmer);
    ON_ERROR(errCode, "Program DS inst");

    /* Program gpr spill */
    if (pDsSEP->exeHints.derivedHints.globalStates.bGprSpilled)
    {
        pStatesPgmer->pHints->useGPRSpill[gcvPROGRAM_STAGE_TES] = gcvTRUE;
        errCode = _ProgramDsGprSpill(pShHwInfo, pStatesPgmer);
        ON_ERROR(errCode, "Program DS grp spill");
    }

    /* Program cr spill */
    if (pDsSEP->exeHints.derivedHints.globalStates.bCrSpilled)
    {
        if (DECODE_SHADER_CLIENT(pDsSEP->shVersionType) == SHADER_CLIENT_VK)
        {
            errCode = _ProgramDsCrSpill(pShHwInfo, pStatesPgmer);
            ON_ERROR(errCode, "Program DS cr spill");
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _ProgramGsInsts(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    SHADER_EXECUTABLE_PROFILE* pGsSEP = pShHwInfo->pSEP;
    gctUINT                    state, ftEntryIdx;
    gctUINT                    vidMemAddrOfCode = NOT_ASSIGNED;
    gctPOINTER                 instVidmemNode = gcvNULL;
    gctUINT                    startPC, endPC;

    gcmASSERT(pShHwInfo->hwProgrammingHints.hwInstFetchMode == HW_INST_FETCH_MODE_CACHE);
    gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasInstCache);

    (*pStatesPgmer->pSysCtx->drvCBs.pfnAllocVidMemCb)(pStatesPgmer->pSysCtx->hDrv,
                                                      gcvSURF_ICACHE,
                                                      "instruction Memory for GS",
                                                      pGsSEP->countOfMCInst * sizeof(VSC_MC_RAW_INST),
                                                      256,
                                                      &instVidmemNode,
                                                      gcvNULL,
                                                      &vidMemAddrOfCode,
                                                      pGsSEP->pMachineCode,
                                                      gcvFALSE);

    if (NOT_ASSIGNED == vidMemAddrOfCode)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        ON_ERROR(errCode, "Create inst vid-mem");
    }

    pStatesPgmer->pHints->shaderVidNodes.instVidmemNode[gceSGSK_GEOMETRY_SHADER] = instVidmemNode;

    /* Program start and end PC. */
    startPC = 0;
    endPC = pGsSEP->endPCOfMainRoutine + 1;
    VSC_LOAD_HW_STATE(0x0443, startPC);
    VSC_LOAD_HW_STATE(0x0444, endPC);

    /*!!! Must be RIGHT before instruction state programming */
    _SetPatchOffsets(pStatesPgmer, _VSC_PATCH_OFFSET_INSTR, gceSGSK_GEOMETRY_SHADER);

    /* Program where to fetch from vid-mem */
    VSC_LOAD_HW_STATE(0x0445, vidMemAddrOfCode);

    state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

    VSC_LOAD_HW_STATE(0x021A, state);

    gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.newSteeringICacheFlush);

    /* Prefetch inst */
    if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasInstCachePrefetch)
    {
        state = (pGsSEP->countOfMCInst - 1);

        VSC_LOAD_HW_STATE(0x0447, state);

        pStatesPgmer->pHints->gsICachePrefetch[0] = 0;
        for (ftEntryIdx = 1; ftEntryIdx < GC_ICACHE_PREFETCH_TABLE_SIZE; ftEntryIdx++)
        {
            pStatesPgmer->pHints->gsICachePrefetch[ftEntryIdx] = -1;
        }
    }

OnError:
    return errCode;
}

static gctUINT _GetGsStartConstRegAddr(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    gctUINT constRegBaseAddr;

    if (pShHwInfo->pSEP->exeHints.derivedHints.globalStates.unifiedConstRegAllocStrategy == UNIFIED_RF_ALLOC_STRATEGY_UNIFIED)
    {
        constRegBaseAddr = pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.vsConstRegAddrBase;
    }
    else
    {
        constRegBaseAddr = pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.gsConstRegAddrBase;
    }

    constRegBaseAddr += pShHwInfo->hwProgrammingHints.hwConstantRegAddrOffset * CHANNEL_NUM;

    return constRegBaseAddr;
}

static VSC_ErrCode _ProgramGsCompileTimeConsts(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    gctUINT                    state, startConstRegAddr;
    VSC_ErrCode                errCode = VSC_ERR_NONE;

    if (pShHwInfo->hwProgrammingHints.hwConstantFetchMode == HW_CONSTANT_FETCH_MODE_UNIFIED_REG_FILE)
    {
        gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxHwNativeTotalConstRegCount > 256);
        gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.gsConstRegAddrBase == 0xD000);

        state = pShHwInfo->hwProgrammingHints.hwConstantRegAddrOffset;
        VSC_LOAD_HW_STATE(0x0453, state);
    }

    startConstRegAddr = _GetGsStartConstRegAddr(pShHwInfo, pStatesPgmer);

    _ProgramConstCountInfo(pShHwInfo, pStatesPgmer, gcvTRUE);

    pStatesPgmer->pHints->hwConstRegBases[gcvPROGRAM_STAGE_GEOMETRY] = startConstRegAddr * 4;
    pStatesPgmer->pHints->constRegNoBase[gcvPROGRAM_STAGE_GEOMETRY] = pShHwInfo->hwProgrammingHints.hwConstantRegAddrOffset;

    errCode = _ProgramRegedCTC(pShHwInfo,
                               startConstRegAddr,
                               pStatesPgmer);
    ON_ERROR(errCode, "Program GS Reged CTC");

OnError:
    return errCode;
}

static VSC_ErrCode _ProgramGsGprSpill(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    SHADER_EXECUTABLE_PROFILE* pGsSEP = pShHwInfo->pSEP;
    gcsSURF_NODE_PTR           gprSpillVidmemNode = gcvNULL;
    gctUINT                    vidMemAddrOfSpillMem = NOT_ASSIGNED;
    gctUINT                    gprSpillSize = 0;

    errCode = _AllocVidMemForGprSpill(pStatesPgmer, pGsSEP, &gprSpillVidmemNode, &vidMemAddrOfSpillMem, &gprSpillSize);
    ON_ERROR(errCode, "Alloc vid-mem for gpr spill");

    pStatesPgmer->pHints->shaderVidNodes.gprSpillVidmemNode[gceSGSK_GEOMETRY_SHADER] = gprSpillVidmemNode;

    _SetPatchOffsets(pStatesPgmer, _VSC_PATCH_OFFSET_GPRSPILL, gceSGSK_GEOMETRY_SHADER);

    errCode = _ProgramGprSpillMemAddr(pGsSEP,
                                      _GetGsStartConstRegAddr(pShHwInfo, pStatesPgmer),
                                      vidMemAddrOfSpillMem,
                                      gprSpillSize,
                                      pStatesPgmer);
    ON_ERROR(errCode, "Program GPR spill mem address ");

OnError:
    return errCode;
}

static VSC_ErrCode _ProgramGsCrSpill(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    SHADER_EXECUTABLE_PROFILE* pGsSEP = pShHwInfo->pSEP;
    gcsSURF_NODE_PTR           crSpillVidmemNode = gcvNULL;
    gctUINT                    vidMemAddrOfSpillMem = NOT_ASSIGNED;
    gctUINT                    crSpillMemSize = 0;

    errCode = _AllocVidMemForCrSpill(pStatesPgmer, pGsSEP, &crSpillVidmemNode, &vidMemAddrOfSpillMem, &crSpillMemSize);
    ON_ERROR(errCode, "Alloc vid-mem for cr spill");

    if (vidMemAddrOfSpillMem != NOT_ASSIGNED)
    {
        pStatesPgmer->pHints->shaderVidNodes.crSpillVidmemNode[gceSGSK_GEOMETRY_SHADER] = crSpillVidmemNode;

        _SetPatchOffsets(pStatesPgmer, _VSC_PATCH_OFFSET_CRSPILL, gceSGSK_GEOMETRY_SHADER);

        errCode = _ProgramCrSpillMemAddr(pGsSEP,
                                         _GetGsStartConstRegAddr(pShHwInfo, pStatesPgmer),
                                         vidMemAddrOfSpillMem,
                                         crSpillMemSize,
                                         pStatesPgmer);
        ON_ERROR(errCode, "Program CR spill mem address ");
    }

OnError:
    return errCode;
}

static gctUINT _GetGSRemapInputStartHwReg(SHADER_EXECUTABLE_PROFILE* pGsSEP)
{
    gctUINT retValue = NOT_ASSIGNED;

    if (pGsSEP->exeHints.derivedHints.globalStates.hwStartRegNoForUSCAddrs == 0)
    {
        if (pGsSEP->exeHints.derivedHints.globalStates.hwStartRegChannelForUSCAddrs == CHANNEL_Z)
        {
            retValue = 0x0;
        }
    }
    else if (pGsSEP->exeHints.derivedHints.globalStates.hwStartRegNoForUSCAddrs == 1)
    {
        if (pGsSEP->exeHints.derivedHints.globalStates.hwStartRegChannelForUSCAddrs == CHANNEL_X)
        {
            retValue = 0x1;
        }
        else if (pGsSEP->exeHints.derivedHints.globalStates.hwStartRegChannelForUSCAddrs == CHANNEL_Z)
        {
            retValue = 0x2;
        }
    }
    else if (pGsSEP->exeHints.derivedHints.globalStates.hwStartRegNoForUSCAddrs == 2)
    {
        if (pGsSEP->exeHints.derivedHints.globalStates.hwStartRegChannelForUSCAddrs == CHANNEL_X)
        {
            retValue = 0x3;
        }
    }

    gcmASSERT(retValue != NOT_ASSIGNED);

    return retValue;
}

extern gctUINT _GetGsValidMaxThreadsPerHwTG(SHADER_EXECUTABLE_PROFILE* pGsSEP, gctUINT maxThreadsPerHwTG, gctUINT maxHwTGThreadCount)
{
    return vscMIN((maxHwTGThreadCount * 6)/pGsSEP->exeHints.nativeHints.prvStates.gs.inputVtxCount, maxThreadsPerHwTG);
}

static VSC_ErrCode _ProgramGS(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    SHADER_EXECUTABLE_PROFILE* pGsSEP = pShHwInfo->pSEP;
    SHADER_IO_LINKAGE_INFO*    pGSOutputLinkageInfo = &pShHwInfo->outputLinkageInfo;
    gctUINT                    state, ioIdx, index, hwRegNo, i;
    gctUINT                    gsPerVtxInputCount = 0, gsOutputCount = 0;
    gctUINT                    soOnlyOutputCount = 0, dummyOutputCount = 0;
    gctUINT                    compCountOfLastOutput = 0x0;
    gctUINT                    outputRegMapping[MAX_HW_IO_COUNT];
    gctUINT                    firstValidIoChannel;
    gctINT                     lastOutputOfPerVtx = -1;
    gctBOOL                    bGsPrimitiveId = gcvFALSE, bGsInstancingId = gcvFALSE;
    gctUINT                    startRemapHwReg, gsOutputSizePerThread;
    gctUINT                    maxThreadsPerHwTG, hwTGSize, gsOutputAttrCount;
    gctUINT                    minUscSize, maxUscSize, extraUscSize;
#if PROGRAMING_OUTPUTS_ON_COMPONENT_GRANULARITY
    gctUINT                    gsOutputCompCount = 0;
#endif

    static gctUINT const gsOutputPrimMap[] =
    {
        0x1,
        0x3,
        0x5
    };

    gcmASSERT(pGsSEP->exeHints.nativeHints.prvStates.gs.outputPrim <= SHADER_GS_OUTPUT_PRIMITIVE_TRIANGLESTRIP);
    gcmASSERT(pGsSEP->inputMapping.ioVtxPxl.ioMode == SHADER_IO_MODE_ACTIVE ||
              pGsSEP->inputMapping.ioVtxPxl.countOfIoRegMapping == 0);
    gcmASSERT(pGsSEP->inputMapping.ioPrim.ioMode == SHADER_IO_MODE_PASSIVE);
    gcmASSERT(pGsSEP->outputMapping.ioVtxPxl.ioMode == SHADER_IO_MODE_PASSIVE ||
              !pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.gsSupportEmit);
    gcmASSERT(!pGsSEP->exeHints.derivedHints.globalStates.bIoUSCAddrsPackedToOneReg);

    memset(&outputRegMapping[0], 0, sizeof(gctUINT)*MAX_HW_IO_COUNT);

    /* Input */
    for (ioIdx = 0; ioIdx < pGsSEP->inputMapping.ioVtxPxl.countOfIoRegMapping; ioIdx ++)
    {
        if (pGsSEP->inputMapping.ioVtxPxl.ioIndexMask & (1LL << ioIdx))
        {
            gsPerVtxInputCount ++;
        }
    }

    if (gsPerVtxInputCount > pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxAttributeCount)
    {
        return VSC_ERR_TOO_MANY_ATTRIBUTES;
    }

    /* Output */
    for (ioIdx = 0; ioIdx < pGsSEP->outputMapping.ioVtxPxl.countOfIoRegMapping; ioIdx ++)
    {
        if (pGsSEP->outputMapping.ioVtxPxl.ioIndexMask & (1LL << ioIdx))
        {
            /* Only consider linked one */
            if (pGSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].linkNo != NOT_ASSIGNED)
            {
                if (gsOutputCount >= (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxVaryingCount + 1) ||
                    pGSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].linkNo >=
                    (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxVaryingCount + 1))
                {
                    return VSC_ERR_TOO_MANY_VARYINGS;
                }

                if ((gctINT)ioIdx > lastOutputOfPerVtx)
                {
                    lastOutputOfPerVtx = ioIdx;
                }

                if (pGsSEP->outputMapping.ioVtxPxl.ioMode == SHADER_IO_MODE_PASSIVE)
                {
                    /* Get HW register number, NOTE in current implementation all channels
                       must be in same HW register number */
                    firstValidIoChannel = pGsSEP->outputMapping.ioVtxPxl.pIoRegMapping[ioIdx].firstValidIoChannel;
                    hwRegNo = pGsSEP->outputMapping.ioVtxPxl.pIoRegMapping[ioIdx].ioChannelMapping[firstValidIoChannel].
                              hwLoc.cmnHwLoc.u.hwRegNo;
                    gcmASSERT(hwRegNo != NOT_ASSIGNED);

                    /* Convert link into output reg mapping array index */
                    index = pGSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].linkNo;

                    /* Copy register into output reg mapping array. */
                    outputRegMapping[index] = hwRegNo;
                }

                if (pGSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].bOnlyLinkToSO)
                {
                    soOnlyOutputCount ++;
                }

                if (pGSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[ioIdx].bIsDummyLink)
                {
                    gcmASSERT(pGsSEP->outputMapping.ioVtxPxl.pIoRegMapping[ioIdx].regIoMode == SHADER_IO_MODE_ACTIVE);

                    dummyOutputCount ++;
                }

#if PROGRAMING_OUTPUTS_ON_COMPONENT_GRANULARITY
                /* Ok, increase channel count */
                gsOutputCompCount += _GetValidHwRegChannelCount(&pGsSEP->outputMapping.ioVtxPxl.pIoRegMapping[ioIdx],
                                                                pGsSEP->outputMapping.ioVtxPxl.ioMemAlign);
#endif

                /* Ok, increase count */
                gsOutputCount ++;
            }
        }
    }

    /* For the cases that need dummy outputs for GS (generally, these outputs should be generated by HW,
       but our HW does not), so outputs of shader and outputs to PA are different, we need pick the right
       one here */
#if PROGRAMING_OUTPUTS_ON_COMPONENT_GRANULARITY
    if (pGSOutputLinkageInfo->vtxPxlLinkage.totalLinkNoCount > gsOutputCount)
    {
        gsOutputCompCount += CHANNEL_NUM * (pGSOutputLinkageInfo->vtxPxlLinkage.totalLinkNoCount - gsOutputCount);
    }
#endif

    gsOutputCount = vscMAX(gsOutputCount, pGSOutputLinkageInfo->vtxPxlLinkage.totalLinkNoCount);

    /* Output must include position (explicit or implicit) */
#if PROGRAMING_OUTPUTS_ON_COMPONENT_GRANULARITY
    if (gsOutputCompCount < CHANNEL_NUM)
    {
        gsOutputCompCount = CHANNEL_NUM;
    }
#endif

    if (gsOutputCount == 0)
    {
        gsOutputCount ++;
    }

    if (lastOutputOfPerVtx >= 0 && pGsSEP->outputMapping.ioVtxPxl.ioMode == SHADER_IO_MODE_PASSIVE)
    {
        compCountOfLastOutput = _GetValidHwRegChannelCount(
                                    &pGsSEP->outputMapping.ioVtxPxl.pIoRegMapping[lastOutputOfPerVtx],
                                    pGsSEP->outputMapping.ioVtxPxl.ioMemAlign);
    }

    bGsPrimitiveId = (pGsSEP->inputMapping.ioPrim.usage2IO[SHADER_IO_USAGE_PRIMITIVEID].ioIndexMask != 0);
    bGsInstancingId = (pGsSEP->inputMapping.ioPrim.usage2IO[SHADER_IO_USAGE_INSTANCING_ID].ioIndexMask != 0);
    startRemapHwReg = _GetGSRemapInputStartHwReg(pGsSEP);

#if PROGRAMING_OUTPUTS_ON_COMPONENT_GRANULARITY
    gsOutputSizePerThread = VSC_UTILS_ALIGN((gsOutputCompCount << 2), 16) * pGsSEP->exeHints.nativeHints.prvStates.gs.maxOutputVtxCount;
#else
    gsOutputSizePerThread = (gsOutputCount << 4) * pGsSEP->exeHints.nativeHints.prvStates.gs.maxOutputVtxCount;
#endif

    maxThreadsPerHwTG = _GetGsValidMaxThreadsPerHwTG(pGsSEP, pShHwInfo->hwProgrammingHints.maxThreadsPerHwTG,
                                                     pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxCoreCount * 4);
    hwTGSize = maxThreadsPerHwTG * gsOutputSizePerThread + pShHwInfo->hwProgrammingHints.gsMetaDataSizePerHwTGInBtye;
    hwTGSize = VSC_UTILS_ALIGN(hwTGSize, 16);

    _GetMinMaxUscSize(pShHwInfo, pStatesPgmer, &minUscSize, &maxUscSize, &extraUscSize);

    pStatesPgmer->pHints->extraUscPages += extraUscSize;
    pStatesPgmer->pHints->prePaShaderHasPointSize = (pGsSEP->outputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_POINTSIZE].ioIndexMask != 0);
    pStatesPgmer->pHints->prePaShaderHasPrimitiveId = (pGsSEP->outputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_PRIMITIVEID].ioIndexMask != 0);
    pStatesPgmer->pHints->shader2PaOutputCount = gsOutputCount - soOnlyOutputCount - dummyOutputCount;
    pStatesPgmer->pHints->memoryAccessFlags[gcvSHADER_HIGH_LEVEL][gcvPROGRAM_STAGE_GEOMETRY] = (gceMEMORY_ACCESS_FLAG)pGsSEP->exeHints.nativeHints.globalStates.memoryAccessHint;
    pStatesPgmer->pHints->memoryAccessFlags[gcvSHADER_MACHINE_LEVEL][gcvPROGRAM_STAGE_GEOMETRY] = (gceMEMORY_ACCESS_FLAG)pGsSEP->exeHints.derivedHints.globalStates.memoryAccessHint;
    pStatesPgmer->pHints->flowControlFlags[gcvSHADER_HIGH_LEVEL][gcvPROGRAM_STAGE_GEOMETRY] = (gceFLOW_CONTROL_FLAG)pGsSEP->exeHints.nativeHints.globalStates.flowControlHint;
    pStatesPgmer->pHints->flowControlFlags[gcvSHADER_MACHINE_LEVEL][gcvPROGRAM_STAGE_GEOMETRY] = (gceFLOW_CONTROL_FLAG)pGsSEP->exeHints.derivedHints.globalStates.flowControlHint;
    pStatesPgmer->pHints->texldFlags[gcvSHADER_HIGH_LEVEL][gcvPROGRAM_STAGE_GEOMETRY] = (gceTEXLD_FLAG)pGsSEP->exeHints.nativeHints.globalStates.texldHint;
    pStatesPgmer->pHints->texldFlags[gcvSHADER_MACHINE_LEVEL][gcvPROGRAM_STAGE_GEOMETRY] = (gceTEXLD_FLAG)pGsSEP->exeHints.derivedHints.globalStates.texldHint;
    pStatesPgmer->pHints->samplerBaseOffset[gcvPROGRAM_STAGE_GEOMETRY] = pShHwInfo->hwProgrammingHints.hwSamplerRegAddrOffset;

    _ProgramSamplerCountInfo(pShHwInfo, pStatesPgmer, gcvTRUE);

    if (pStatesPgmer->pHints->prePaShaderHasPointSize)
    {
        pStatesPgmer->pHints->ptSzAttrIndex = pGSOutputLinkageInfo->vtxPxlLinkage.ioRegLinkage[pGsSEP->outputMapping.ioVtxPxl.
                                                                            usage2IO[SHADER_IO_USAGE_POINTSIZE].mainIoIndex].linkNo;
    }

    /* Program GS control */
    gsOutputAttrCount = (pGsSEP->outputMapping.ioVtxPxl.ioMode == SHADER_IO_MODE_ACTIVE) ? 0 : gsOutputCount;
    state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0)))                                                     |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ?
 21:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:20) - (0 ?
 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) ((gctUINT32) (startRemapHwReg) & ((gctUINT32) ((((1 ?
 21:20) - (0 ?
 21:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:20) - (0 ?
 21:20) + 1))))))) << (0 ? 21:20)))                                |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:4) - (0 ?
 9:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:4) - (0 ?
 9:4) + 1))))))) << (0 ?
 9:4))) | (((gctUINT32) ((gctUINT32) (gsOutputAttrCount) & ((gctUINT32) ((((1 ?
 9:4) - (0 ?
 9:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:4) - (0 ? 9:4) + 1))))))) << (0 ? 9:4)))                             |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:12) - (0 ?
 18:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:12) - (0 ?
 18:12) + 1))))))) << (0 ?
 18:12))) | (((gctUINT32) ((gctUINT32) (_GetCalibratedGprCount(pShHwInfo, pStatesPgmer)) & ((gctUINT32) ((((1 ?
 18:12) - (0 ?
 18:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:12) - (0 ? 18:12) + 1))))))) << (0 ? 18:12))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (bGsPrimitiveId) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26)))                            |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:27) - (0 ?
 27:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:27) - (0 ?
 27:27) + 1))))))) << (0 ?
 27:27))) | (((gctUINT32) ((gctUINT32) (bGsInstancingId) & ((gctUINT32) ((((1 ?
 27:27) - (0 ?
 27:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:27) - (0 ? 27:27) + 1))))))) << (0 ? 27:27)))                            |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:25) - (0 ?
 25:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:25) - (0 ?
 25:25) + 1))))))) << (0 ?
 25:25))) | (((gctUINT32) ((gctUINT32) (pGsSEP->exeHints.nativeHints.prvStates.gs.bHasStreamOut) & ((gctUINT32) ((((1 ?
 25:25) - (0 ?
 25:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ? 25:25))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:28) - (0 ?
 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) ((gctUINT32) (compCountOfLastOutput) & ((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:28) - (0 ? 29:28) + 1))))))) << (0 ? 29:28)))                       |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:24) - (0 ?
 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (pGsSEP->exeHints.derivedHints.prvStates.gs.bHasPrimRestartOp) & ((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24)));
    VSC_LOAD_HW_STATE(0x0440, state);

    /* GS output primitive type */
    state = gsOutputPrimMap[pGsSEP->exeHints.nativeHints.prvStates.gs.outputPrim];
    VSC_LOAD_HW_STATE(0x0441, state);

    /* Program GS instance/max-output-vertex count */
    state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:0) - (0 ?
 6:0) + 1))))))) << (0 ?
 6:0))) | (((gctUINT32) ((gctUINT32) (pGsSEP->exeHints.nativeHints.prvStates.gs.instanceCount) & ((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:0) - (0 ? 6:0) + 1))))))) << (0 ? 6:0))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:16) - (0 ?
 24:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:16) - (0 ?
 24:16) + 1))))))) << (0 ?
 24:16))) | (((gctUINT32) ((gctUINT32) (pGsSEP->exeHints.nativeHints.prvStates.gs.maxOutputVtxCount) & ((gctUINT32) ((((1 ?
 24:16) - (0 ?
 24:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:16) - (0 ? 24:16) + 1))))))) << (0 ? 24:16)));
    VSC_LOAD_HW_STATE(0x0442, state);

    /* Program output */
    for (i = 0; i < MAX_HW_IO_COUNT/4; i ++)
    {
        state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (outputRegMapping[i * 4 + 0]) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0))) |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:8) - (0 ?
 13:8) + 1))))))) << (0 ?
 13:8))) | (((gctUINT32) ((gctUINT32) (outputRegMapping[i * 4 + 1]) & ((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ? 13:8))) |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:16) - (0 ?
 21:16) + 1))))))) << (0 ?
 21:16))) | (((gctUINT32) ((gctUINT32) (outputRegMapping[i * 4 + 2]) & ((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ? 21:16))) |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:24) - (0 ?
 29:24) + 1))))))) << (0 ?
 29:24))) | (((gctUINT32) ((gctUINT32) (outputRegMapping[i * 4 + 3]) & ((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ? 29:24)));

        VSC_LOAD_HW_STATE(0x0448 + i, state);
    }

    /* Program attribute layout registers */
    state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:8) - (0 ?
 16:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:8) - (0 ?
 16:8) + 1))))))) << (0 ?
 16:8))) | (((gctUINT32) ((gctUINT32) (gsOutputSizePerThread >> 4) & ((gctUINT32) ((((1 ?
 16:8) - (0 ?
 16:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:8) - (0 ? 16:8) + 1))))))) << (0 ? 16:8)))            |
#if PROGRAMING_OUTPUTS_ON_COMPONENT_GRANULARITY
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:0) - (0 ?
 7:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:0) - (0 ?
 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) (VSC_UTILS_ALIGN(gsOutputCompCount, 4)) & ((gctUINT32) ((((1 ?
 7:0) - (0 ?
 7:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ? 7:0))) |
#else
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:0) - (0 ?
 7:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:0) - (0 ?
 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) (gsOutputCount << 2) & ((gctUINT32) ((((1 ?
 7:0) - (0 ?
 7:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:0) - (0 ? 7:0) + 1))))))) << (0 ? 7:0)))                    |
#endif
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:20) - (0 ?
 31:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:20) - (0 ?
 31:20) + 1))))))) << (0 ?
 31:20))) | (((gctUINT32) ((gctUINT32) ((maxThreadsPerHwTG * gsOutputSizePerThread) >> 4) & ((gctUINT32) ((((1 ?
 31:20) - (0 ?
 31:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:20) - (0 ? 31:20) + 1))))))) << (0 ? 31:20)));
    VSC_LOAD_HW_STATE(0x0451, state);

    /* Program attribute-ex layout registers */
    state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:0) - (0 ?
 11:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:0) - (0 ?
 11:0) + 1))))))) << (0 ?
 11:0))) | (((gctUINT32) ((gctUINT32) (hwTGSize >> 4) & ((gctUINT32) ((((1 ?
 11:0) - (0 ?
 11:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:0) - (0 ? 11:0) + 1))))))) << (0 ? 11:0)));
    VSC_LOAD_HW_STATE(0x0452, state);

    /* Program USC registers */
    state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (maxUscSize) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:21) - (0 ?
 26:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:21) - (0 ?
 26:21) + 1))))))) << (0 ?
 26:21))) | (((gctUINT32) ((gctUINT32) (minUscSize) & ((gctUINT32) ((((1 ?
 26:21) - (0 ?
 26:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:21) - (0 ? 26:21) + 1))))))) << (0 ? 26:21))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:8) - (0 ?
 15:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:8) - (0 ?
 15:8) + 1))))))) << (0 ?
 15:8))) | (((gctUINT32) ((gctUINT32) (maxThreadsPerHwTG) & ((gctUINT32) ((((1 ?
 15:8) - (0 ?
 15:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8)));
    VSC_LOAD_HW_STATE(0x0450, state);

    /* Program SO */
    if (pGsSEP->outputMapping.ioVtxPxl.soIoIndexMask != 0 && pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.supportStreamOut)
    {
        errCode = _ProgramStreamOut(pShHwInfo, &pGsSEP->outputMapping.ioVtxPxl,
                                    &pGSOutputLinkageInfo->vtxPxlLinkage, pStatesPgmer);
        ON_ERROR(errCode, "Program SO");
    }

    /* Program sampler-related. */
    if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasSamplerBaseOffset)
    {
        state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:0) - (0 ?
 6:0) + 1))))))) << (0 ?
 6:0))) | (((gctUINT32) ((gctUINT32) (pShHwInfo->hwProgrammingHints.hwSamplerRegAddrOffset) & ((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:0) - (0 ? 6:0) + 1))))))) << (0 ? 6:0)));
        VSC_LOAD_HW_STATE(0x0455, state);
    }

    /* Program CTCs */
    errCode = _ProgramGsCompileTimeConsts(pShHwInfo, pStatesPgmer);
    ON_ERROR(errCode, "Program GS CTC");

    /* Program insts */
    errCode = _ProgramGsInsts(pShHwInfo, pStatesPgmer);
    ON_ERROR(errCode, "Program GS inst");

    /* Program gpr spill */
    if (pGsSEP->exeHints.derivedHints.globalStates.bGprSpilled)
    {
        pStatesPgmer->pHints->useGPRSpill[gcvPROGRAM_STAGE_GEOMETRY] = gcvTRUE;
        errCode = _ProgramGsGprSpill(pShHwInfo, pStatesPgmer);
        ON_ERROR(errCode, "Program GS grp spill");
    }

    /* Program cr spill */
    if (pGsSEP->exeHints.derivedHints.globalStates.bCrSpilled)
    {
        if (DECODE_SHADER_CLIENT(pGsSEP->shVersionType) == SHADER_CLIENT_VK)
        {
            errCode = _ProgramGsCrSpill(pShHwInfo, pStatesPgmer);
            ON_ERROR(errCode, "Program GS cr spill");
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _ProgramPsInsts(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    SHADER_EXECUTABLE_PROFILE* pPsSEP = pShHwInfo->pSEP;
    gctUINT                    pushedInstSize, thisPushInstSize;
    gctUINT                    state, hwInstBaseAddr, ftEntryIdx;
    gctUINT                    vidMemAddrOfCode = NOT_ASSIGNED, shaderConfigData;
    gctPOINTER                 instVidmemNode = gcvNULL;
    gctUINT                    startPC, endPC;

    if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti5)
    {
        shaderConfigData = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:2) - (0 ?
 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) ((gctUINT32) ((pPsSEP->exeHints.derivedHints.globalStates.bExecuteOnDual16 ?
 0x1 : 0x0)) & ((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2)))
                         | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) ((gctUINT32) ((pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.rtneRoundingEnabled ?
 0x1 : 0x0)) & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)));

        if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasPSIOInterlock &&
            pShHwInfo->pSEP->exeHints.derivedHints.prvStates.ps.bNeedRtRead)
        {
            shaderConfigData |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:10) - (0 ?
 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ? 10:10)));
        }
    }
    else
    {
        shaderConfigData = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:29) - (0 ?
 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) ((gctUINT32) ((pPsSEP->exeHints.derivedHints.globalStates.bExecuteOnDual16 ?
 0x1 : 0x0)) & ((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29))) |
                           ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) ((pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.rtneRoundingEnabled ?
 0x1 : 0x0)) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)));
    }

    if (!pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.newSteeringICacheFlush)
    {
        shaderConfigData |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) |
                            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)));
    }

    if ((pShHwInfo->pSEP->exeHints.derivedHints.globalStates.memoryAccessHint & SHADER_EDH_MEM_ACCESS_HINT_ATOMIC) &&
        pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.robustAtomic)
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

    shaderConfigData |= pStatesPgmer->pHints->shaderConfigData;
    pStatesPgmer->pHints->shaderConfigData = shaderConfigData;

    if (pShHwInfo->hwProgrammingHints.hwInstFetchMode == HW_INST_FETCH_MODE_CACHE)
    {
        /* ----- I$ case ----- */

        gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasInstCache);

        (*pStatesPgmer->pSysCtx->drvCBs.pfnAllocVidMemCb)(pStatesPgmer->pSysCtx->hDrv,
                                                          gcvSURF_ICACHE,
                                                          "instruction Memory for PS/GPS",
                                                          pPsSEP->countOfMCInst * sizeof(VSC_MC_RAW_INST),
                                                          256,
                                                          &instVidmemNode,
                                                          gcvNULL,
                                                          &vidMemAddrOfCode,
                                                          pPsSEP->pMachineCode,
                                                          gcvFALSE);

        if (NOT_ASSIGNED == vidMemAddrOfCode)
        {
            errCode = VSC_ERR_OUT_OF_MEMORY;
            ON_ERROR(errCode, "Create inst vid-mem");
        }

        pStatesPgmer->pHints->shaderVidNodes.instVidmemNode[gceSGSK_FRAGMENT_SHADER] = instVidmemNode;

        /* Program start and end PC. */
        startPC = 0;
        VSC_LOAD_HW_STATE(0x021F, startPC);
        if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti5)
        {
            endPC = pPsSEP->endPCOfMainRoutine + 1;
            VSC_LOAD_HW_STATE(0x0424, endPC);
        }
        else
        {
            endPC = pPsSEP->endPCOfMainRoutine;
            VSC_LOAD_HW_STATE(0x0220, endPC);
        }
        /* !!! Must be RIGHT before instruction state programming */
        _SetPatchOffsets(pStatesPgmer, _VSC_PATCH_OFFSET_INSTR, gceSGSK_FRAGMENT_SHADER);

        /* Program where to fetch from vid-mem */
        VSC_LOAD_HW_STATE(0x040A, vidMemAddrOfCode);

        /* Prepare to program cache mode */
        if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti5)
        {
            VSC_LOAD_HW_STATE(0x5580, shaderConfigData);
        }
        else
        {
            VSC_LOAD_HW_STATE(0x0218, shaderConfigData);
        }

        /* Invalidate cache lines ownered by PS */
        state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

        if (!pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.newSteeringICacheFlush)
        {
            state |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)));
        }

        VSC_LOAD_HW_STATE(0x021A, state);

        /* Prefetch inst */
        if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasInstCachePrefetch)
        {
            state = (pPsSEP->countOfMCInst - 1);
            if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti5)
            {
                VSC_LOAD_HW_STATE(0x0425, state);
            }
            else
            {
                VSC_LOAD_HW_STATE(0x0413, state);
            }

            pStatesPgmer->pHints->fsICachePrefetch[0] = 0;
            for (ftEntryIdx = 1; ftEntryIdx < GC_ICACHE_PREFETCH_TABLE_SIZE; ftEntryIdx++)
            {
                pStatesPgmer->pHints->fsICachePrefetch[ftEntryIdx] = -1;
            }
        }
    }
    else
    {
        /* ----- Register case ----- */

        gcmASSERT(!pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti5);

        /* Program start and end PC. */
        if (pShHwInfo->hwProgrammingHints.hwInstFetchMode == HW_INST_FETCH_MODE_UNUNIFIED_BUFFER)
        {
            gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxHwNativeTotalInstCount <= 256);
            gcmASSERT(pShHwInfo->hwProgrammingHints.hwInstBufferAddrOffset == 0);
            gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.psInstBufferAddrBase == 0x1800);

            startPC = pShHwInfo->hwProgrammingHints.hwInstBufferAddrOffset;
            endPC = pShHwInfo->hwProgrammingHints.hwInstBufferAddrOffset + pPsSEP->endPCOfMainRoutine + 1;

            state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:0) - (0 ?
 11:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:0) - (0 ?
 11:0) + 1))))))) << (0 ?
 11:0))) | (((gctUINT32) ((gctUINT32) (startPC) & ((gctUINT32) ((((1 ?
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
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
            VSC_LOAD_HW_STATE(0x0406, state);

            state = endPC;
            VSC_LOAD_HW_STATE(0x0400, state);

#if gcdALPHA_KILL_IN_SHADER
            if (pPsSEP->exeHints.derivedHints.prvStates.ps.alphaClrKillInstsGened)
            {
                pStatesPgmer->pHints->killStateAddress    = 0x0400;
                pStatesPgmer->pHints->alphaKillStateValue = endPC + 1;
                pStatesPgmer->pHints->colorKillStateValue = endPC + 2;
            }
#endif
        }
        else if (pShHwInfo->hwProgrammingHints.hwInstFetchMode == HW_INST_FETCH_MODE_UNIFIED_BUFFER_0)
        {
            gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxHwNativeTotalInstCount > 256);
            gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.psInstBufferAddrBase == 0x2000);

            startPC = pShHwInfo->hwProgrammingHints.hwInstBufferAddrOffset;
            endPC = pShHwInfo->hwProgrammingHints.hwInstBufferAddrOffset + pPsSEP->endPCOfMainRoutine;

            /* If HW has I$, we then need use register set of I$ to program */
            if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasInstCache)
            {
                VSC_LOAD_HW_STATE(0x021F, startPC);
                VSC_LOAD_HW_STATE(0x0220, endPC);

#if gcdALPHA_KILL_IN_SHADER
                if (pPsSEP->exeHints.derivedHints.prvStates.ps.alphaClrKillInstsGened)
                {
                    pStatesPgmer->pHints->killStateAddress    = 0x0220;
                    pStatesPgmer->pHints->alphaKillStateValue = endPC + 1;
                    pStatesPgmer->pHints->colorKillStateValue = endPC + 2;
                }
#endif
            }
            else
            {
                state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (startPC) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (endPC) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));
                VSC_LOAD_HW_STATE(0x0407, state);

#if gcdALPHA_KILL_IN_SHADER
                if (pPsSEP->exeHints.derivedHints.prvStates.ps.alphaClrKillInstsGened)
                {
                    pStatesPgmer->pHints->killStateAddress    = 0x0407;
                    pStatesPgmer->pHints->alphaKillStateValue = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (startPC) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) |
                                                                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (endPC + 1) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));
                    pStatesPgmer->pHints->colorKillStateValue = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (startPC) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) |
                                                                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (endPC + 2) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));
                }
#endif
            }
        }
        else if (pShHwInfo->hwProgrammingHints.hwInstFetchMode == HW_INST_FETCH_MODE_UNIFIED_BUFFER_1)
        {
            gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxHwNativeTotalInstCount > 1024);
            gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.psInstBufferAddrBase == 0x8000);

            startPC = pShHwInfo->hwProgrammingHints.hwInstBufferAddrOffset;
            endPC = pShHwInfo->hwProgrammingHints.hwInstBufferAddrOffset + pPsSEP->endPCOfMainRoutine;

            /* If HW has I$, we then need use register set of I$ to program */
            if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasInstCache)
            {
                VSC_LOAD_HW_STATE(0x021F, startPC);
                VSC_LOAD_HW_STATE(0x0220, endPC);

#if gcdALPHA_KILL_IN_SHADER
                if (pPsSEP->exeHints.derivedHints.prvStates.ps.alphaClrKillInstsGened)
                {
                    pStatesPgmer->pHints->killStateAddress    = 0x0220;
                    pStatesPgmer->pHints->alphaKillStateValue = endPC + 1;
                    pStatesPgmer->pHints->colorKillStateValue = endPC + 2;
                }
#endif
            }
            else
            {
                state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (startPC) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (endPC) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));
                VSC_LOAD_HW_STATE(0x0407, state);

#if gcdALPHA_KILL_IN_SHADER
                if (pPsSEP->exeHints.derivedHints.prvStates.ps.alphaClrKillInstsGened)
                {
                    pStatesPgmer->pHints->killStateAddress    = 0x0407;
                    pStatesPgmer->pHints->alphaKillStateValue = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (startPC) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) |
                                                                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (endPC + 1) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));
                    pStatesPgmer->pHints->colorKillStateValue = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (startPC) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) |
                                                                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (endPC + 2) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));
                }
#endif
            }
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }

        /* Get from where to push insts */
        hwInstBaseAddr = pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.psInstBufferAddrBase +
                         (pShHwInfo->hwProgrammingHints.hwInstBufferAddrOffset << 2);

        if (pShHwInfo->hwProgrammingHints.hwInstFetchMode == HW_INST_FETCH_MODE_UNIFIED_BUFFER_0 ||
            pShHwInfo->hwProgrammingHints.hwInstFetchMode == HW_INST_FETCH_MODE_UNIFIED_BUFFER_1)
        {
            pStatesPgmer->pHints->unifiedStatus.instPSStart = pShHwInfo->hwProgrammingHints.hwInstBufferAddrOffset;
        }

        /* Prepare to load insts */
        VSC_LOAD_HW_STATE(0x0218, shaderConfigData);

        /* We need assure inst won't be fetched from I$ */
        if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasInstCache)
        {
            state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

            if (!pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.newSteeringICacheFlush)
            {
                state |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)));
            }

            VSC_LOAD_HW_STATE(0x021A, state);

            state = 0;
            VSC_LOAD_HW_STATE(0x040A, state);
        }

        /* Program insts now. Note that a single DMA buffer have limits to 256 instructions per buffer. */
        for (pushedInstSize = 0; pushedInstSize < pPsSEP->countOfMCInst;)
        {
            thisPushInstSize = ((pPsSEP->countOfMCInst - pushedInstSize) > DMA_BATCH_SIZE_LIMIT) ?
                                                                           DMA_BATCH_SIZE_LIMIT  :
                                                          (pPsSEP->countOfMCInst - pushedInstSize);

            VSC_LOAD_HW_STATES((hwInstBaseAddr + pushedInstSize*VSC_MC_INST_DWORD_SIZE),
                               (pPsSEP->pMachineCode + pushedInstSize*VSC_MC_INST_DWORD_SIZE),
                               thisPushInstSize*VSC_MC_INST_DWORD_SIZE);

            pushedInstSize += thisPushInstSize;
        }

#if gcdALPHA_KILL_IN_SHADER
        if (pPsSEP->exeHints.derivedHints.prvStates.ps.alphaClrKillInstsGened)
        {
            pStatesPgmer->pHints->killInstructionAddress =
                (hwInstBaseAddr + (pPsSEP->endPCOfMainRoutine + 1) * VSC_MC_INST_DWORD_SIZE);

            gcoOS_MemCopy(pStatesPgmer->pHints->alphaKillInstruction,
                          pPsSEP->pMachineCode + (pPsSEP->endPCOfMainRoutine + 2) * VSC_MC_INST_DWORD_SIZE,
                          gcmSIZEOF(pStatesPgmer->pHints->alphaKillInstruction));

            gcoOS_MemCopy(pStatesPgmer->pHints->colorKillInstruction,
                          pPsSEP->pMachineCode + (pPsSEP->endPCOfMainRoutine + 1) * VSC_MC_INST_DWORD_SIZE,
                          gcmSIZEOF(pStatesPgmer->pHints->colorKillInstruction));
        }
#endif
    }

OnError:
    return errCode;
}

static gctUINT _GetPsStartConstRegAddr(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    gctUINT constRegBaseAddr;

    if (pShHwInfo->pSEP->exeHints.derivedHints.globalStates.unifiedConstRegAllocStrategy == UNIFIED_RF_ALLOC_STRATEGY_UNIFIED)
    {
        constRegBaseAddr = pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.vsConstRegAddrBase;
    }
    else
    {
        constRegBaseAddr = pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.psConstRegAddrBase;
    }

    constRegBaseAddr += pShHwInfo->hwProgrammingHints.hwConstantRegAddrOffset * CHANNEL_NUM;

    return constRegBaseAddr;
}

static VSC_ErrCode _ProgramPsCompileTimeConsts(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    gctUINT                    state, startConstRegAddr;
    VSC_ErrCode                errCode = VSC_ERR_NONE;

    if (pShHwInfo->hwProgrammingHints.hwConstantFetchMode == HW_CONSTANT_FETCH_MODE_UNIFIED_REG_FILE)
    {
        gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxHwNativeTotalConstRegCount > 256);

        if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.smallBatch)
        {
            gcmASSERT(pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.psConstRegAddrBase == 0xD000);
        }
        else
        {
        gcmASSERT((pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.psConstRegAddrBase == 0xC000) ||
                  (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.psConstRegAddrBase == 0xD800));
        }

        state = pShHwInfo->hwProgrammingHints.hwConstantRegAddrOffset;
        VSC_LOAD_HW_STATE(0x0409, state);

        if (!pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.newSteeringICacheFlush)
        {
            state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)));
            VSC_LOAD_HW_STATE(0x0218, state);
        }
    }

    startConstRegAddr = _GetPsStartConstRegAddr(pShHwInfo, pStatesPgmer);

    _ProgramConstCountInfo(pShHwInfo, pStatesPgmer, gcvFALSE);

    pStatesPgmer->pHints->hwConstRegBases[gcvPROGRAM_STAGE_COMPUTE] =
    pStatesPgmer->pHints->hwConstRegBases[gcvPROGRAM_STAGE_OPENCL] =
    pStatesPgmer->pHints->hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT] = startConstRegAddr * 4;

    pStatesPgmer->pHints->constRegNoBase[gcvPROGRAM_STAGE_COMPUTE] =
    pStatesPgmer->pHints->constRegNoBase[gcvPROGRAM_STAGE_OPENCL] =
    pStatesPgmer->pHints->constRegNoBase[gcvPROGRAM_STAGE_FRAGMENT] = pShHwInfo->hwProgrammingHints.hwConstantRegAddrOffset;

    errCode = _ProgramRegedCTC(pShHwInfo,
                               startConstRegAddr,
                               pStatesPgmer);
    ON_ERROR(errCode, "Program PS Reged CTC");

OnError:
    return errCode;
}

static VSC_ErrCode _ProgramPsGprSpill(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    SHADER_EXECUTABLE_PROFILE* pPsSEP = pShHwInfo->pSEP;
    gcsSURF_NODE_PTR           gprSpillVidmemNode = gcvNULL;
    gctUINT                    vidMemAddrOfSpillMem = NOT_ASSIGNED;
    gctUINT                    gprSpillSize = 0;

    errCode = _AllocVidMemForGprSpill(pStatesPgmer, pPsSEP, &gprSpillVidmemNode, &vidMemAddrOfSpillMem, &gprSpillSize);
    ON_ERROR(errCode, "Alloc vid-mem for gpr spill");

    pStatesPgmer->pHints->shaderVidNodes.gprSpillVidmemNode[gceSGSK_FRAGMENT_SHADER] = gprSpillVidmemNode;

    _SetPatchOffsets(pStatesPgmer, _VSC_PATCH_OFFSET_GPRSPILL, gceSGSK_FRAGMENT_SHADER);

    errCode = _ProgramGprSpillMemAddr(pPsSEP,
                                      _GetPsStartConstRegAddr(pShHwInfo, pStatesPgmer),
                                      vidMemAddrOfSpillMem,
                                      gprSpillSize,
                                      pStatesPgmer);
    ON_ERROR(errCode, "Program GPR spill mem address ");

OnError:
    return errCode;
}

static VSC_ErrCode _ProgramPsCrSpill(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    SHADER_EXECUTABLE_PROFILE* pPsSEP = pShHwInfo->pSEP;
    gcsSURF_NODE_PTR           crSpillVidmemNode = gcvNULL;
    gctUINT                    vidMemAddrOfSpillMem = NOT_ASSIGNED;
    gctUINT                    crSpillMemSize = 0;

    errCode = _AllocVidMemForCrSpill(pStatesPgmer, pPsSEP, &crSpillVidmemNode, &vidMemAddrOfSpillMem, &crSpillMemSize);
    ON_ERROR(errCode, "Alloc vid-mem for cr spill");

    if (vidMemAddrOfSpillMem != NOT_ASSIGNED)
    {
        pStatesPgmer->pHints->shaderVidNodes.crSpillVidmemNode[gceSGSK_FRAGMENT_SHADER] = crSpillVidmemNode;

        _SetPatchOffsets(pStatesPgmer, _VSC_PATCH_OFFSET_CRSPILL, gceSGSK_FRAGMENT_SHADER);

        errCode = _ProgramCrSpillMemAddr(pPsSEP,
                                         _GetPsStartConstRegAddr(pShHwInfo, pStatesPgmer),
                                         vidMemAddrOfSpillMem,
                                         crSpillMemSize,
                                         pStatesPgmer);
        ON_ERROR(errCode, "Program CR spill mem address ");
    }

OnError:
    return errCode;
}

static gctUINT _GetPSSampleMaskLocation(SHADER_EXECUTABLE_PROFILE* pPsSEP)
{
    if (pPsSEP->exeHints.derivedHints.prvStates.ps.hwRegNoForSampleMaskId == 0)
    {
        if (pPsSEP->exeHints.derivedHints.prvStates.ps.hwRegChannelForSampleMaskId == CHANNEL_Z)
        {
            return 0x1;
        }
        else if (pPsSEP->exeHints.derivedHints.prvStates.ps.hwRegChannelForSampleMaskId == CHANNEL_W)
        {
            return 0x2;
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }
    }
    else if (pPsSEP->exeHints.derivedHints.prvStates.ps.hwRegNoForSampleMaskId == pPsSEP->gprCount - 1)
    {
        /* Sample-depth must be in last register number */
        gcmASSERT(pPsSEP->exeHints.derivedHints.prvStates.ps.hwRegChannelForSampleMaskId == CHANNEL_X);
        return 0x0;
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    return NOT_ASSIGNED;
}

static gctUINT _GetPSInputChannelInterpolationType(SHADER_EXECUTABLE_PROFILE* pPsSEP,
                                                   SHADER_IO_CHANNEL_MAPPING* pIoChannelMapping)
{
    if (pIoChannelMapping->flag.bInteger)
    {
        if (pPsSEP->exeHints.derivedHints.globalStates.bExecuteOnDual16 &&
            !pIoChannelMapping->flag.bHighPrecisionOnDual16)
        {
            return 0x3;
        }
        else
        {
            return 0x2;
        }
    }
    else
    {
        return pIoChannelMapping->flag.bConstInterpolate ?
               0x2 : (pIoChannelMapping->flag.bPerspectiveCorrection ?
                   0x0 : 0x1);
    }
}

static gctUINT _GetPSInputChannelInterpolationLoc(SHADER_EXECUTABLE_PROFILE* pPsSEP,
                                                  SHADER_IO_CHANNEL_MAPPING* pIoChannelMapping,
                                                  gctBOOL bForceSampleFreq)
{
    return (pIoChannelMapping->flag.bSampleFrequency || bForceSampleFreq) ?
            0x2 : (pIoChannelMapping->flag.bCentroidInterpolate ?
                0x1 : 0x0);
}

static VSC_ErrCode _ProgramPS(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    SHADER_EXECUTABLE_PROFILE* pPsSEP = pShHwInfo->pSEP;
    gctUINT                    state, ioIdx, texcoord, channelIdx, sortedIoIdx, hwRegNo, hwRegNoT1, index, i;
    gctUINT                    psInputCount = 0, psOutputCount = 0;
    gctUINT                    psColorOutputState = 0, psColorOutputCtrlState = 0;
    gctUINT                    psColorOutputPrecisionState = 0, psExtColorOutputPrecisionState = 0;
    gctUINT                    validChannelCount, psInputComponnentCount = 0;
    gctCHAR                    componentType[MAX_HW_IO_COUNT * CHANNEL_NUM];
    gctCHAR                    psInputType[MAX_HW_IO_COUNT * CHANNEL_NUM];
    gctCHAR                    interpolationType[MAX_HW_IO_COUNT * CHANNEL_NUM];
    gctCHAR                    interpolationLoc[MAX_HW_IO_COUNT * CHANNEL_NUM];
    gctUINT                    varyingPacking[MAX_HW_IO_COUNT];
    gctUINT                    lastValidInputIndex = NOT_ASSIGNED;
    gctBOOL                    bHalfAttribute = gcvFALSE, bForceSampleFreq = gcvFALSE;
    gctBOOL                    bHasSampleFreqInput = gcvFALSE, bHighPOnDual16 = gcvFALSE;
    gctBOOL                    bHasCentroidInput = gcvFALSE;
    gctINT                     output2RTIndex[gcdMAX_DRAW_BUFFERS];
    gctUINT                    firstValidIoChannel, hpInputCount = 0, startHwRegForNonPosInputs = NOT_ASSIGNED;
    SHADER_IO_CHANNEL_MAPPING* pThisIoChannelMapping;
    gctINT                     ptCoordStartComponent = -1, rtArrayStartComponent = -1, primIdStartComponent = -1;
    gctUINT                    calibratedGprCount = _GetCalibratedGprCount(pShHwInfo, pStatesPgmer);
#if !IO_HW_LOC_NUMBER_IN_ORDER
    gctUINT                    sortedIoIdxArray[MAX_SHADER_IO_NUM];
#endif

    gcmASSERT(pPsSEP->inputMapping.ioVtxPxl.ioMode == SHADER_IO_MODE_PASSIVE);
    gcmASSERT(pPsSEP->outputMapping.ioVtxPxl.ioMode == SHADER_IO_MODE_PASSIVE);

#if !IO_HW_LOC_NUMBER_IN_ORDER
    vscSortIOsByHwLoc(&pPsSEP->inputMapping.ioVtxPxl, sortedIoIdxArray);
#endif

    memset(&componentType[0], 0 /*0x0*/,
           sizeof(gctCHAR)*MAX_HW_IO_COUNT * CHANNEL_NUM);
    memset(&interpolationLoc[0], 0 /*0x0*/,
           sizeof(gctCHAR)*MAX_HW_IO_COUNT * CHANNEL_NUM);
    memset(&interpolationType[0], 0 /*0x0*/,
           sizeof(gctCHAR)*MAX_HW_IO_COUNT * CHANNEL_NUM);
    memset(&varyingPacking[0], 0, sizeof(gctUINT)*MAX_HW_IO_COUNT);
    memset(&psInputType[0], 0 /*0x0*/,
           sizeof(gctCHAR)*MAX_HW_IO_COUNT * CHANNEL_NUM);

    for (i = 0; i < gcdMAX_DRAW_BUFFERS; ++i)
    {
        output2RTIndex[i] = -1;
    }

    bForceSampleFreq = pPsSEP->exeHints.derivedHints.prvStates.ps.bExecuteOnSampleFreq;

#if gcdDUMP
    if (!pPsSEP->inputMapping.ioVtxPxl.countOfIoRegMapping &&
        !pPsSEP->outputMapping.ioVtxPxl.countOfIoRegMapping)
    {
        pPsSEP->exeHints.derivedHints.globalStates.bExecuteOnDual16 = 0;
    }

#endif

    /* Input */
    for (ioIdx = 0; ioIdx < pPsSEP->inputMapping.ioVtxPxl.countOfIoRegMapping; ioIdx ++)
    {
#if !IO_HW_LOC_NUMBER_IN_ORDER
        sortedIoIdx = sortedIoIdxArray[ioIdx];
#else
        sortedIoIdx = ioIdx;
#endif

        if (pPsSEP->inputMapping.ioVtxPxl.ioIndexMask & (1LL << sortedIoIdx))
        {
            /* Position and frontface need to be considered for half-attribute */
            lastValidInputIndex = sortedIoIdx;

            firstValidIoChannel = pPsSEP->inputMapping.ioVtxPxl.pIoRegMapping[sortedIoIdx].firstValidIoChannel;

            bHighPOnDual16 = pPsSEP->inputMapping.ioVtxPxl.pIoRegMapping[sortedIoIdx].ioChannelMapping[firstValidIoChannel].
                             flag.bHighPrecisionOnDual16;

            /* No need to count position since it have special location */
            if (pPsSEP->inputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_POSITION].ioIndexMask & (1LL << sortedIoIdx))
            {
                continue;
            }

            /* No need to count frontface/helper/sample_id/sample_pos/sample_mask pixel since they use special hw regs */
            if (pPsSEP->inputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_ISFRONTFACE].ioIndexMask & (1LL << sortedIoIdx) ||
                pPsSEP->inputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_HELPER_PIXEL].ioIndexMask & (1LL << sortedIoIdx) ||
                pPsSEP->inputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_SAMPLE_INDEX].ioIndexMask & (1LL << sortedIoIdx) ||
                pPsSEP->inputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_SAMPLE_MASK].ioIndexMask & (1LL << sortedIoIdx) ||
                pPsSEP->inputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_SAMPLE_POSITION].ioIndexMask & (1LL << sortedIoIdx))
            {
                gcmASSERT(pPsSEP->inputMapping.ioVtxPxl.pIoRegMapping[sortedIoIdx].ioChannelMapping[firstValidIoChannel].
                          hwLoc.cmnHwLoc.u.hwRegNo == SPECIAL_HW_IO_REG_NO);

                continue;
            }

            /* No need to count sample depth because it is always passed into shader by the last temp register */
            if (pPsSEP->inputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_SAMPLE_DEPTH].ioIndexMask & (1LL << sortedIoIdx))
            {
                continue;
            }

            if (bHighPOnDual16)
            {
                hpInputCount ++;
            }

            if (pPsSEP->inputMapping.ioVtxPxl.pIoRegMapping[sortedIoIdx].ioChannelMapping[firstValidIoChannel].
                hwLoc.cmnHwLoc.u.hwRegNo < startHwRegForNonPosInputs)
            {
                startHwRegForNonPosInputs = pPsSEP->inputMapping.ioVtxPxl.pIoRegMapping[sortedIoIdx].
                                            ioChannelMapping[firstValidIoChannel].hwLoc.cmnHwLoc.u.hwRegNo;
            }

            if (pPsSEP->inputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_POINT_COORD].ioIndexMask & (1LL << sortedIoIdx))
            {
                ptCoordStartComponent = psInputComponnentCount;
            }

            if (pPsSEP->inputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_RENDERTARGETARRAYINDEX].ioIndexMask & (1LL << sortedIoIdx))
            {
                rtArrayStartComponent = psInputComponnentCount;
            }

            if (pPsSEP->inputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_PRIMITIVEID].ioIndexMask & (1LL << sortedIoIdx))
            {
                primIdStartComponent = psInputComponnentCount;
            }

            if (psInputCount >= pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxVaryingCount)
            {
                return VSC_ERR_TOO_MANY_VARYINGS;
            }

            /* Per HW, 0x0290 is only valid for under GC1000. For GC1000 and above,
               it is obsolete and HW instead uses 0x0E08 */
            if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.varyingPackingLimited)
            {
                texcoord = (pPsSEP->inputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_TEXCOORD].ioIndexMask &
                            (1LL << sortedIoIdx)) ? 0xF : 0x0;

                state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (texcoord) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:4) - (0 ?
 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (texcoord) & ((gctUINT32) ((((1 ?
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
 ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ? 11:8)));
                VSC_LOAD_HW_STATE(0x0290 + psInputCount, state);
            }

            /* Collect info for varying pack and component type. Note that HW has a limitation that each varying
               pack must be start with X channel, so if x channel is not enabled, we still must take it into account */
            if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.varyingPackingLimited)
            {
                validChannelCount = CHANNEL_NUM;
            }
            else
            {
                validChannelCount = _GetValidHwRegChannelCount(
                                          &pPsSEP->inputMapping.ioVtxPxl.pIoRegMapping[sortedIoIdx],
                                          pPsSEP->inputMapping.ioVtxPxl.ioMemAlign);
            }

            gcmASSERT(validChannelCount <= CHANNEL_NUM);

            varyingPacking[psInputCount] = validChannelCount;

            for (channelIdx = 0; channelIdx < validChannelCount; channelIdx ++)
            {
                pThisIoChannelMapping =
                    &pPsSEP->inputMapping.ioVtxPxl.pIoRegMapping[sortedIoIdx].ioChannelMapping[channelIdx];

                if (pThisIoChannelMapping->flag.bConstInterpolate)
                {
                    if (DECODE_SHADER_CLIENT(pPsSEP->shVersionType) != SHADER_CLIENT_DX)
                    {
                        pStatesPgmer->pHints->shaderMode = gcvSHADING_FLAT_OPENGL;
                    }
                    else
                    {
                        pStatesPgmer->pHints->shaderMode = gcvSHADING_FLAT_D3D;
                    }
                }

                interpolationType[psInputComponnentCount] = (gctCHAR)_GetPSInputChannelInterpolationType(pPsSEP, pThisIoChannelMapping);
                interpolationLoc[psInputComponnentCount] = (gctCHAR)_GetPSInputChannelInterpolationLoc(pPsSEP, pThisIoChannelMapping,
                                                                                              bForceSampleFreq);
                bHasCentroidInput = pThisIoChannelMapping->flag.bCentroidInterpolate;

                if (pThisIoChannelMapping->flag.bSampleFrequency)
                {
                    bHasSampleFreqInput = gcvTRUE;
                }

                /* Component type programming tips:
                   1. TEXTURE_U and TEXTURE_V are only valid for point-sprite to get normalized TC inside of point.
                   2. COLOR must be for constant interpolate (flat).
                   3. For linear point interpolate, OTHER and TEXTURE_* are same.
                */
                if (channelIdx == CHANNEL_X)
                {
                    componentType[psInputComponnentCount] =
                        pThisIoChannelMapping->flag.bConstInterpolate ? 0x1 :
                        ((pThisIoChannelMapping->ioUsage == SHADER_IO_USAGE_POINT_COORD) ?
                         0x2 :
                         0x0);
                    psInputType[psInputComponnentCount] = pThisIoChannelMapping->flag.bInteger ?
                            0x1 :
                            0x0;
                    psInputComponnentCount ++;
                }
                else if (channelIdx == CHANNEL_Y)
                {
                    componentType[psInputComponnentCount] =
                        pThisIoChannelMapping->flag.bConstInterpolate ? 0x1 :
                        ((pThisIoChannelMapping->ioUsage == SHADER_IO_USAGE_POINT_COORD) ?
                         0x3 :
                         0x0);
                    psInputType[psInputComponnentCount] = pThisIoChannelMapping->flag.bInteger ?
                            0x1 :
                            0x0;
                    psInputComponnentCount ++;
                }
                else if (channelIdx == CHANNEL_Z)
                {
                    componentType[psInputComponnentCount] =
                        pThisIoChannelMapping->flag.bConstInterpolate ? 0x1 :
                        ((pThisIoChannelMapping->ioUsage == SHADER_IO_USAGE_POINT_COORD) ?
                         0x2 :
                         0x0);
                    psInputType[psInputComponnentCount] = pThisIoChannelMapping->flag.bInteger ?
                            0x1 :
                            0x0;
                    psInputComponnentCount ++;
                }
                else
                {
                    componentType[psInputComponnentCount] =
                        pThisIoChannelMapping->flag.bConstInterpolate ? 0x1 :
                        ((pThisIoChannelMapping->ioUsage == SHADER_IO_USAGE_POINT_COORD) ?
                         0x3 :
                         0x0);
                    psInputType[psInputComponnentCount] = pThisIoChannelMapping->flag.bInteger ?
                            0x1 :
                            0x0;
                    psInputComponnentCount ++;
                }
            }

            /* Ok, increase count */
            psInputCount ++;
        }
    }

    if (!pPsSEP->outputMapping.ioVtxPxl.countOfIoRegMapping)
    {
        psColorOutputPrecisionState = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));
    }

    /* Output */
    for (ioIdx = 0; ioIdx < pPsSEP->outputMapping.ioVtxPxl.countOfIoRegMapping; ioIdx ++)
    {
        if (pPsSEP->outputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_COLOR].ioIndexMask & (1LL << ioIdx))
        {
            firstValidIoChannel = pPsSEP->outputMapping.ioVtxPxl.pIoRegMapping[ioIdx].firstValidIoChannel;
            hwRegNo = pPsSEP->outputMapping.ioVtxPxl.pIoRegMapping[ioIdx].ioChannelMapping[firstValidIoChannel].
                      hwLoc.cmnHwLoc.u.hwRegNo;
            hwRegNoT1 = pPsSEP->outputMapping.ioVtxPxl.pIoRegMapping[ioIdx].ioChannelMapping[firstValidIoChannel].
                        hwLoc.t1HwLoc.hwRegNo;
            bHighPOnDual16 = pPsSEP->outputMapping.ioVtxPxl.pIoRegMapping[ioIdx].ioChannelMapping[firstValidIoChannel].
                             flag.bHighPrecisionOnDual16;

            index = psOutputCount;

            switch (index)
            {
            case 0:
                psColorOutputState = ((((gctUINT32) (psColorOutputState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (hwRegNo) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));

                psColorOutputPrecisionState = ((((gctUINT32) (psColorOutputPrecisionState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (hwRegNoT1) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));
                psColorOutputPrecisionState = ((((gctUINT32) (psColorOutputPrecisionState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (bHighPOnDual16) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));

                break;

            case 1:
                psColorOutputState = ((((gctUINT32) (psColorOutputState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:8) - (0 ?
 13:8) + 1))))))) << (0 ?
 13:8))) | (((gctUINT32) ((gctUINT32) (hwRegNo) & ((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ? 13:8)));

                psColorOutputPrecisionState = ((((gctUINT32) (psColorOutputPrecisionState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:8) - (0 ?
 13:8) + 1))))))) << (0 ?
 13:8))) | (((gctUINT32) ((gctUINT32) (hwRegNoT1) & ((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ? 13:8)));
                psColorOutputPrecisionState = ((((gctUINT32) (psColorOutputPrecisionState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:14) - (0 ?
 14:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:14) - (0 ?
 14:14) + 1))))))) << (0 ?
 14:14))) | (((gctUINT32) ((gctUINT32) (bHighPOnDual16) & ((gctUINT32) ((((1 ?
 14:14) - (0 ?
 14:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:14) - (0 ? 14:14) + 1))))))) << (0 ? 14:14)));

                break;

            case 2:
                psColorOutputState = ((((gctUINT32) (psColorOutputState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:16) - (0 ?
 21:16) + 1))))))) << (0 ?
 21:16))) | (((gctUINT32) ((gctUINT32) (hwRegNo) & ((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ? 21:16)));

                psColorOutputPrecisionState = ((((gctUINT32) (psColorOutputPrecisionState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:16) - (0 ?
 21:16) + 1))))))) << (0 ?
 21:16))) | (((gctUINT32) ((gctUINT32) (hwRegNoT1) & ((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ? 21:16)));
                psColorOutputPrecisionState = ((((gctUINT32) (psColorOutputPrecisionState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:22) - (0 ?
 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) ((gctUINT32) (bHighPOnDual16) & ((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ? 22:22)));

                break;

            case 3:
                psColorOutputState = ((((gctUINT32) (psColorOutputState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:24) - (0 ?
 29:24) + 1))))))) << (0 ?
 29:24))) | (((gctUINT32) ((gctUINT32) (hwRegNo) & ((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ? 29:24)));

                psColorOutputPrecisionState = ((((gctUINT32) (psColorOutputPrecisionState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:24) - (0 ?
 29:24) + 1))))))) << (0 ?
 29:24))) | (((gctUINT32) ((gctUINT32) (hwRegNoT1) & ((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ? 29:24)));
                psColorOutputPrecisionState = ((((gctUINT32) (psColorOutputPrecisionState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:30) - (0 ?
 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) ((gctUINT32) (bHighPOnDual16) & ((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)));

                break;

            case 4:
                psColorOutputCtrlState = ((((gctUINT32) (psColorOutputCtrlState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (hwRegNo) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));

                psExtColorOutputPrecisionState = ((((gctUINT32) (psExtColorOutputPrecisionState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (hwRegNoT1) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));
                psExtColorOutputPrecisionState = ((((gctUINT32) (psExtColorOutputPrecisionState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (bHighPOnDual16) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));

                break;

            case 5:
                psColorOutputCtrlState = ((((gctUINT32) (psColorOutputCtrlState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:8) - (0 ?
 13:8) + 1))))))) << (0 ?
 13:8))) | (((gctUINT32) ((gctUINT32) (hwRegNo) & ((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ? 13:8)));

                psExtColorOutputPrecisionState = ((((gctUINT32) (psExtColorOutputPrecisionState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:8) - (0 ?
 13:8) + 1))))))) << (0 ?
 13:8))) | (((gctUINT32) ((gctUINT32) (hwRegNoT1) & ((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ? 13:8)));
                psExtColorOutputPrecisionState = ((((gctUINT32) (psExtColorOutputPrecisionState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:14) - (0 ?
 14:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:14) - (0 ?
 14:14) + 1))))))) << (0 ?
 14:14))) | (((gctUINT32) ((gctUINT32) (bHighPOnDual16) & ((gctUINT32) ((((1 ?
 14:14) - (0 ?
 14:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:14) - (0 ? 14:14) + 1))))))) << (0 ? 14:14)));

                break;

            case 6:
                psColorOutputCtrlState = ((((gctUINT32) (psColorOutputCtrlState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:16) - (0 ?
 21:16) + 1))))))) << (0 ?
 21:16))) | (((gctUINT32) ((gctUINT32) (hwRegNo) & ((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ? 21:16)));

                psExtColorOutputPrecisionState = ((((gctUINT32) (psExtColorOutputPrecisionState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:16) - (0 ?
 21:16) + 1))))))) << (0 ?
 21:16))) | (((gctUINT32) ((gctUINT32) (hwRegNoT1) & ((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ? 21:16)));
                psExtColorOutputPrecisionState = ((((gctUINT32) (psExtColorOutputPrecisionState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:22) - (0 ?
 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) ((gctUINT32) (bHighPOnDual16) & ((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ? 22:22)));

                break;

            case 7:
                psColorOutputCtrlState = ((((gctUINT32) (psColorOutputCtrlState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:24) - (0 ?
 29:24) + 1))))))) << (0 ?
 29:24))) | (((gctUINT32) ((gctUINT32) (hwRegNo) & ((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ? 29:24)));

                psExtColorOutputPrecisionState = ((((gctUINT32) (psExtColorOutputPrecisionState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:24) - (0 ?
 29:24) + 1))))))) << (0 ?
 29:24))) | (((gctUINT32) ((gctUINT32) (hwRegNoT1) & ((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ? 29:24)));
                psExtColorOutputPrecisionState = ((((gctUINT32) (psExtColorOutputPrecisionState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:30) - (0 ?
 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) ((gctUINT32) (bHighPOnDual16) & ((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)));

                break;

            default:
                gcmASSERT(gcvFALSE);
                return VSC_ERR_INVALID_DATA;
            }

            output2RTIndex[index] = ioIdx;

            psOutputCount ++;
        }
    }

    pStatesPgmer->pHints->psHasDiscard = pStatesPgmer->pHints->hasKill = pPsSEP->exeHints.derivedHints.prvStates.ps.bPxlDiscard;
    pStatesPgmer->pHints->memoryAccessFlags[gcvSHADER_HIGH_LEVEL][gcvPROGRAM_STAGE_FRAGMENT] = (gceMEMORY_ACCESS_FLAG)pPsSEP->exeHints.nativeHints.globalStates.memoryAccessHint;
    pStatesPgmer->pHints->memoryAccessFlags[gcvSHADER_MACHINE_LEVEL][gcvPROGRAM_STAGE_FRAGMENT] = (gceMEMORY_ACCESS_FLAG)pPsSEP->exeHints.derivedHints.globalStates.memoryAccessHint;
    pStatesPgmer->pHints->flowControlFlags[gcvSHADER_HIGH_LEVEL][gcvPROGRAM_STAGE_FRAGMENT] = (gceFLOW_CONTROL_FLAG)pPsSEP->exeHints.nativeHints.globalStates.flowControlHint;
    pStatesPgmer->pHints->flowControlFlags[gcvSHADER_MACHINE_LEVEL][gcvPROGRAM_STAGE_FRAGMENT] = (gceFLOW_CONTROL_FLAG)pPsSEP->exeHints.derivedHints.globalStates.flowControlHint;
    pStatesPgmer->pHints->texldFlags[gcvSHADER_HIGH_LEVEL][gcvPROGRAM_STAGE_FRAGMENT] = (gceTEXLD_FLAG)pPsSEP->exeHints.nativeHints.globalStates.texldHint;
    pStatesPgmer->pHints->texldFlags[gcvSHADER_MACHINE_LEVEL][gcvPROGRAM_STAGE_FRAGMENT] = (gceTEXLD_FLAG)pPsSEP->exeHints.derivedHints.globalStates.texldHint;
    pStatesPgmer->pHints->samplerBaseOffset[gcvPROGRAM_STAGE_FRAGMENT] = pShHwInfo->hwProgrammingHints.hwSamplerRegAddrOffset;

    _ProgramSamplerCountInfo(pShHwInfo, pStatesPgmer, gcvFALSE);

    pStatesPgmer->pHints->elementCount = psInputCount;
    pStatesPgmer->pHints->componentCount = gcmALIGN(psInputComponnentCount, 2);
    pStatesPgmer->pHints->fsInstCount = pPsSEP->countOfMCInst;
    pStatesPgmer->pHints->fsIsDual16 = pPsSEP->exeHints.derivedHints.globalStates.bExecuteOnDual16;
    pStatesPgmer->pHints->psHighPVaryingCount = hpInputCount;
    pStatesPgmer->pHints->removeAlphaAssignment = (pPsSEP->exeHints.derivedHints.prvStates.ps.alphaWriteOutputIndexMask != 0);
    pStatesPgmer->pHints->psHasFragDepthOut = (pPsSEP->outputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_DEPTH].ioIndexMask != 0);
    pStatesPgmer->pHints->useDSX = pPsSEP->exeHints.derivedHints.prvStates.ps.bDerivRTx;
    pStatesPgmer->pHints->useDSY = pPsSEP->exeHints.derivedHints.prvStates.ps.bDerivRTy;
    pStatesPgmer->pHints->yInvertAware = ((pPsSEP->inputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_ISFRONTFACE].ioIndexMask != 0) ||
                                          (pPsSEP->inputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_SAMPLE_POSITION].ioIndexMask != 0) ||
                                          (pPsSEP->exeHints.derivedHints.prvStates.ps.inputPosChannelValid & (1 << CHANNEL_Y)) ||
                                          (pPsSEP->exeHints.derivedHints.prvStates.ps.inputPntCoordChannelValid & (1 << CHANNEL_Y)) ||
                                          (pPsSEP->exeHints.derivedHints.prvStates.ps.bDsyBeforeLowering));

    pStatesPgmer->pHints->hasCentroidInput = bHasCentroidInput;
    pStatesPgmer->pHints->useEarlyFragmentTest = pPsSEP->exeHints.nativeHints.prvStates.ps.bEarlyPixelTestInRa;

    pStatesPgmer->pHints->fsMaxTemp = pPsSEP->inputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_SAMPLE_DEPTH].ioIndexMask ?
                                      /* FlushShader will add 1 back under msaa case */
                                      (calibratedGprCount - (pPsSEP->exeHints.derivedHints.globalStates.bExecuteOnDual16 ? 2 : 1)) :
                                      calibratedGprCount;

    pStatesPgmer->pHints->fragColorUsage = pPsSEP->exeHints.derivedHints.prvStates.ps.fragColorUsage;

    gcoOS_MemCopy((gctPOINTER)pStatesPgmer->pHints->psOutput2RtIndex, (gctPOINTER)output2RTIndex, sizeof(output2RTIndex));
    gcoOS_MemCopy((gctPOINTER)pStatesPgmer->pHints->interpolationType, (gctPOINTER)interpolationType, sizeof(interpolationType));

    for (channelIdx = 0; channelIdx < CHANNEL_NUM; channelIdx ++)
    {
        if (pPsSEP->exeHints.derivedHints.prvStates.ps.inputPosChannelValid & (1 << channelIdx))
        {
            pStatesPgmer->pHints->useFragCoord[channelIdx] = gcvTRUE;
        }

        if (channelIdx < CHANNEL_Z)
        {
            if (pPsSEP->exeHints.derivedHints.prvStates.ps.inputPntCoordChannelValid & (1 << channelIdx))
            {
                pStatesPgmer->pHints->usePointCoord[channelIdx] = gcvTRUE;
            }
        }
    }

    /* Adding 1 means for implicit position, it must be at first location */
    pStatesPgmer->pHints->fsInputCount = psInputCount + 1;

    pStatesPgmer->pHints->pointCoordComponent = ptCoordStartComponent;
    pStatesPgmer->pHints->rtArrayComponent = rtArrayStartComponent;
    pStatesPgmer->pHints->primIdComponent = primIdStartComponent;

    /* Sample-mask (in & out), sample_index and sample_pos (which is actually got by sample_index) needs a
       special register channel (location) to hold these info. */
    if (pPsSEP->inputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_SAMPLE_MASK].ioIndexMask != 0 ||
        pPsSEP->outputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_SAMPLE_MASK].ioIndexMask != 0 ||
        pPsSEP->inputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_SAMPLE_INDEX].ioIndexMask != 0 ||
        pPsSEP->inputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_SAMPLE_POSITION].ioIndexMask != 0 ||
        bForceSampleFreq)
    {
        pStatesPgmer->pHints->sampleMaskLoc = _GetPSSampleMaskLocation(pPsSEP);

        /* It looks not make sense, right??? Need double check!! */
        if (rtArrayStartComponent != -1)
        {
            gcmASSERT(pStatesPgmer->pHints->sampleMaskLoc != 0x2);
        }
    }

    pStatesPgmer->pHints->usedSampleIdOrSamplePosition = bForceSampleFreq;
    pStatesPgmer->pHints->psUsedSampleInput = bHasSampleFreqInput;
    pStatesPgmer->pHints->sampleMaskOutWritten =
        (pPsSEP->outputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_SAMPLE_MASK].ioIndexMask != 0);

    if (pPsSEP->exeHints.derivedHints.globalStates.bExecuteOnDual16)
    {
        if (pPsSEP->inputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_POSITION].ioIndexMask != 0 ||
            pStatesPgmer->pHints->sampleMaskLoc == 0x2 ||
            startHwRegForNonPosInputs == 2 ||
            /* Following 2 conditions look not make sense, right??? Need double check!! */
            pStatesPgmer->pHints->psHasFragDepthOut ||
            rtArrayStartComponent != -1
            )
        {
            pStatesPgmer->pHints->psInputControlHighpPosition = 0x1;
        }
    }

    /* Program ra-2-PS control */
    if (lastValidInputIndex != NOT_ASSIGNED &&
        (pPsSEP->inputMapping.ioVtxPxl.pIoRegMapping[lastValidInputIndex].ioChannelMask == WRITEMASK_X ||
         pPsSEP->inputMapping.ioVtxPxl.pIoRegMapping[lastValidInputIndex].ioChannelMask == WRITEMASK_XY))
    {
        bHalfAttribute = gcvTRUE;
    }
    state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) ((gctUINT32) (bHalfAttribute) & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:4) - (0 ?
 9:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:4) - (0 ?
 9:4) + 1))))))) << (0 ?
 9:4))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 9:4) - (0 ?
 9:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:4) - (0 ? 9:4) + 1))))))) << (0 ? 9:4)));
    VSC_LOAD_HW_STATE(0x0380, state);

    /* Program color output */
    VSC_LOAD_HW_STATE(0x0401, psColorOutputState);
    pStatesPgmer->pHints->psOutCntl0to3 = psColorOutputState;
    if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxRenderTargetCount > 4)
    {
        VSC_LOAD_HW_STATE(0x040B, psColorOutputCtrlState);
        pStatesPgmer->pHints->psOutCntl4to7 = psColorOutputCtrlState;
    }

    /* Program color output precision */
    VSC_LOAD_HW_STATE(0x040D, psColorOutputPrecisionState);
    if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxRenderTargetCount > 4)
    {
        VSC_LOAD_HW_STATE(0x040E, psExtColorOutputPrecisionState);
    }

    if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.newGPIPE)
    {
        /* Program varying pack */
        for (i = 0; i < MAX_HW_IO_COUNT/8; i ++)
        {
            state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (varyingPacking[i * 8 + 0]) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:4) - (0 ?
 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) ((gctUINT32) (varyingPacking[i * 8 + 1]) & ((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ? 6:4))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:8) - (0 ?
 10:8) + 1))))))) << (0 ?
 10:8))) | (((gctUINT32) ((gctUINT32) (varyingPacking[i * 8 + 2]) & ((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ? 10:8))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:12) - (0 ?
 14:12) + 1))))))) << (0 ?
 14:12))) | (((gctUINT32) ((gctUINT32) (varyingPacking[i * 8 + 3]) & ((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ? 14:12))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) ((gctUINT32) (varyingPacking[i * 8 + 4]) & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) ((gctUINT32) (varyingPacking[i * 8 + 5]) & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) ((gctUINT32) (varyingPacking[i * 8 + 6]) & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (varyingPacking[i * 8 + 7]) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));

            VSC_LOAD_HW_STATE(0x02A4 + i, state);
            VSC_LOAD_HW_STATE(0x0420 + i, state);
        }

        /* Program component semantics */
        for (i = 0; i < MAX_HW_IO_COUNT/2; i ++)
        {
            state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (interpolationType[i * 8 + 0]) & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (interpolationType[i * 8 + 1]) & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:8) - (0 ?
 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (interpolationType[i * 8 + 2]) & ((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:12) - (0 ?
 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) ((gctUINT32) (interpolationType[i * 8 + 3]) & ((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ? 13:12))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:16) - (0 ?
 17:16) + 1))))))) << (0 ?
 17:16))) | (((gctUINT32) ((gctUINT32) (interpolationType[i * 8 + 4]) & ((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ?
 21:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:20) - (0 ?
 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) ((gctUINT32) (interpolationType[i * 8 + 5]) & ((gctUINT32) ((((1 ?
 21:20) - (0 ?
 21:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:24) - (0 ?
 25:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:24) - (0 ?
 25:24) + 1))))))) << (0 ?
 25:24))) | (((gctUINT32) ((gctUINT32) (interpolationType[i * 8 + 6]) & ((gctUINT32) ((((1 ?
 25:24) - (0 ?
 25:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:24) - (0 ? 25:24) + 1))))))) << (0 ? 25:24))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:28) - (0 ?
 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) ((gctUINT32) (interpolationType[i * 8 + 7]) & ((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ? 29:28))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:2) - (0 ?
 3:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:2) - (0 ?
 3:2) + 1))))))) << (0 ?
 3:2))) | (((gctUINT32) ((gctUINT32) (interpolationLoc[i * 8 + 0]) & ((gctUINT32) ((((1 ?
 3:2) - (0 ?
 3:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:2) - (0 ? 3:2) + 1))))))) << (0 ? 3:2)))       |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:6) - (0 ?
 7:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:6) - (0 ?
 7:6) + 1))))))) << (0 ?
 7:6))) | (((gctUINT32) ((gctUINT32) (interpolationLoc[i * 8 + 1]) & ((gctUINT32) ((((1 ?
 7:6) - (0 ?
 7:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ? 7:6)))       |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:10) - (0 ?
 11:10) + 1))))))) << (0 ?
 11:10))) | (((gctUINT32) ((gctUINT32) (interpolationLoc[i * 8 + 2]) & ((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:10) - (0 ? 11:10) + 1))))))) << (0 ? 11:10)))       |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) ((gctUINT32) (interpolationLoc[i * 8 + 3]) & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))       |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:18) - (0 ?
 19:18) + 1))))))) << (0 ?
 19:18))) | (((gctUINT32) ((gctUINT32) (interpolationLoc[i * 8 + 4]) & ((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:18) - (0 ? 19:18) + 1))))))) << (0 ? 19:18)))       |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:22) - (0 ?
 23:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:22) - (0 ?
 23:22) + 1))))))) << (0 ?
 23:22))) | (((gctUINT32) ((gctUINT32) (interpolationLoc[i * 8 + 5]) & ((gctUINT32) ((((1 ?
 23:22) - (0 ?
 23:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:22) - (0 ? 23:22) + 1))))))) << (0 ? 23:22)))       |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:26) - (0 ?
 27:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:26) - (0 ?
 27:26) + 1))))))) << (0 ?
 27:26))) | (((gctUINT32) ((gctUINT32) (interpolationLoc[i * 8 + 6]) & ((gctUINT32) ((((1 ?
 27:26) - (0 ?
 27:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:26) - (0 ? 27:26) + 1))))))) << (0 ? 27:26)))       |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:30) - (0 ?
 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (interpolationLoc[i * 8 + 7]) & ((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));

           VSC_LOAD_HW_STATE(0x0E30 + i, state);
        }
    }
    else
    {
        /* Program varying pack */
        state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));
        VSC_LOAD_HW_STATE(0x0E08, state);

        if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxVaryingCount > 8)
        {
            state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (varyingPacking[8]) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))   |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:4) - (0 ?
 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) ((gctUINT32) (varyingPacking[9]) & ((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ? 6:4)))   |
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
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));
            VSC_LOAD_HW_STATE(0x0E0D, state);
        }

        /* Program component type */
        state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (componentType[0]) & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))  |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:2) - (0 ?
 3:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:2) - (0 ?
 3:2) + 1))))))) << (0 ?
 3:2))) | (((gctUINT32) ((gctUINT32) (componentType[1]) & ((gctUINT32) ((((1 ?
 3:2) - (0 ?
 3:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:2) - (0 ? 3:2) + 1))))))) << (0 ? 3:2)))  |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (componentType[2]) & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)))  |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:6) - (0 ?
 7:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:6) - (0 ?
 7:6) + 1))))))) << (0 ?
 7:6))) | (((gctUINT32) ((gctUINT32) (componentType[3]) & ((gctUINT32) ((((1 ?
 7:6) - (0 ?
 7:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ? 7:6)))  |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:8) - (0 ?
 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (componentType[4]) & ((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8)))  |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:10) - (0 ?
 11:10) + 1))))))) << (0 ?
 11:10))) | (((gctUINT32) ((gctUINT32) (componentType[5]) & ((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:10) - (0 ? 11:10) + 1))))))) << (0 ? 11:10)))  |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:12) - (0 ?
 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) ((gctUINT32) (componentType[6]) & ((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ? 13:12)))  |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) ((gctUINT32) (componentType[7]) & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))  |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:16) - (0 ?
 17:16) + 1))))))) << (0 ?
 17:16))) | (((gctUINT32) ((gctUINT32) (componentType[8]) & ((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16)))  |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:18) - (0 ?
 19:18) + 1))))))) << (0 ?
 19:18))) | (((gctUINT32) ((gctUINT32) (componentType[9]) & ((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:18) - (0 ? 19:18) + 1))))))) << (0 ? 19:18)))  |
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
 ~0U : (~(~0U << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));
        VSC_LOAD_HW_STATE(0x0E0A, state);

        state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 ~0U : (~(~0U << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));
        VSC_LOAD_HW_STATE(0x0E0B, state);

        if (pPsSEP->exeHints.derivedHints.globalStates.bExecuteOnDual16)
        {
            /* Program 0x0410 only in dual16*/
            state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (psInputType[0]) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))  |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) ((gctUINT32) (psInputType[1]) & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))  |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:2) - (0 ?
 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) ((gctUINT32) (psInputType[2]) & ((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2)))  |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (psInputType[3]) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))  |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (psInputType[4]) & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)))  |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) ((gctUINT32) (psInputType[5]) & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)))  |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (psInputType[6]) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)))  |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:7) - (0 ?
 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) ((gctUINT32) (psInputType[7]) & ((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7)))  |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:8) - (0 ?
 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) ((gctUINT32) (psInputType[8]) & ((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8)))  |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:9) - (0 ?
 9:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:9) - (0 ?
 9:9) + 1))))))) << (0 ?
 9:9))) | (((gctUINT32) ((gctUINT32) (psInputType[9]) & ((gctUINT32) ((((1 ?
 9:9) - (0 ?
 9:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9)))  |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:10) - (0 ?
 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) ((gctUINT32) (psInputType[10]) & ((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ? 10:10))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (psInputType[11]) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (psInputType[12]) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:13) - (0 ?
 13:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:13) - (0 ?
 13:13) + 1))))))) << (0 ?
 13:13))) | (((gctUINT32) ((gctUINT32) (psInputType[13]) & ((gctUINT32) ((((1 ?
 13:13) - (0 ?
 13:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:13) - (0 ? 13:13) + 1))))))) << (0 ? 13:13))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:14) - (0 ?
 14:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:14) - (0 ?
 14:14) + 1))))))) << (0 ?
 14:14))) | (((gctUINT32) ((gctUINT32) (psInputType[14]) & ((gctUINT32) ((((1 ?
 14:14) - (0 ?
 14:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:14) - (0 ? 14:14) + 1))))))) << (0 ? 14:14))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:15) - (0 ?
 15:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:15) - (0 ?
 15:15) + 1))))))) << (0 ?
 15:15))) | (((gctUINT32) ((gctUINT32) (psInputType[15]) & ((gctUINT32) ((((1 ?
 15:15) - (0 ?
 15:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ? 15:15))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (psInputType[16]) & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:17) - (0 ?
 17:17) + 1))))))) << (0 ?
 17:17))) | (((gctUINT32) ((gctUINT32) (psInputType[17]) & ((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:17) - (0 ? 17:17) + 1))))))) << (0 ? 17:17))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:18) - (0 ?
 18:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:18) - (0 ?
 18:18) + 1))))))) << (0 ?
 18:18))) | (((gctUINT32) ((gctUINT32) (psInputType[18]) & ((gctUINT32) ((((1 ?
 18:18) - (0 ?
 18:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:18) - (0 ? 18:18) + 1))))))) << (0 ? 18:18))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:19) - (0 ?
 19:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:19) - (0 ?
 19:19) + 1))))))) << (0 ?
 19:19))) | (((gctUINT32) ((gctUINT32) (psInputType[19]) & ((gctUINT32) ((((1 ?
 19:19) - (0 ?
 19:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:19) - (0 ? 19:19) + 1))))))) << (0 ? 19:19))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:20) - (0 ?
 20:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:20) - (0 ?
 20:20) + 1))))))) << (0 ?
 20:20))) | (((gctUINT32) ((gctUINT32) (psInputType[20]) & ((gctUINT32) ((((1 ?
 20:20) - (0 ?
 20:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ? 20:20))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ?
 21:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:21) - (0 ?
 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) ((gctUINT32) (psInputType[21]) & ((gctUINT32) ((((1 ?
 21:21) - (0 ?
 21:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ? 21:21))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:22) - (0 ?
 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) ((gctUINT32) (psInputType[22]) & ((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ? 22:22))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:23) - (0 ?
 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) (psInputType[23]) & ((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ? 23:23))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:24) - (0 ?
 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (psInputType[24]) & ((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:25) - (0 ?
 25:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:25) - (0 ?
 25:25) + 1))))))) << (0 ?
 25:25))) | (((gctUINT32) ((gctUINT32) (psInputType[25]) & ((gctUINT32) ((((1 ?
 25:25) - (0 ?
 25:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ? 25:25))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (psInputType[26]) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:27) - (0 ?
 27:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:27) - (0 ?
 27:27) + 1))))))) << (0 ?
 27:27))) | (((gctUINT32) ((gctUINT32) (psInputType[27]) & ((gctUINT32) ((((1 ?
 27:27) - (0 ?
 27:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:27) - (0 ? 27:27) + 1))))))) << (0 ? 27:27))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) ((gctUINT32) (psInputType[28]) & ((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ? 28:28))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:29) - (0 ?
 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) ((gctUINT32) (psInputType[29]) & ((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:30) - (0 ?
 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) ((gctUINT32) (psInputType[30]) & ((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (psInputType[31]) & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31))) ;
            VSC_LOAD_HW_STATE(0x0410, state);

            if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxVaryingCount > 8)
            {
                state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (psInputType[32]) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) ((gctUINT32) (psInputType[33]) & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:2) - (0 ?
 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) ((gctUINT32) (psInputType[34]) & ((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (psInputType[35]) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (psInputType[36]) & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) ((gctUINT32) (psInputType[37]) & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (psInputType[38]) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:7) - (0 ?
 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) ((gctUINT32) (psInputType[39]) & ((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:8) - (0 ?
 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) ((gctUINT32) (psInputType[40]) & ((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:9) - (0 ?
 9:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:9) - (0 ?
 9:9) + 1))))))) << (0 ?
 9:9))) | (((gctUINT32) ((gctUINT32) (psInputType[41]) & ((gctUINT32) ((((1 ?
 9:9) - (0 ?
 9:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:10) - (0 ?
 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) ((gctUINT32) (psInputType[42]) & ((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ? 10:10))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (psInputType[43]) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (psInputType[44]) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:13) - (0 ?
 13:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:13) - (0 ?
 13:13) + 1))))))) << (0 ?
 13:13))) | (((gctUINT32) ((gctUINT32) (psInputType[45]) & ((gctUINT32) ((((1 ?
 13:13) - (0 ?
 13:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:13) - (0 ? 13:13) + 1))))))) << (0 ? 13:13))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:14) - (0 ?
 14:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:14) - (0 ?
 14:14) + 1))))))) << (0 ?
 14:14))) | (((gctUINT32) ((gctUINT32) (psInputType[46]) & ((gctUINT32) ((((1 ?
 14:14) - (0 ?
 14:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:14) - (0 ? 14:14) + 1))))))) << (0 ? 14:14))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:15) - (0 ?
 15:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:15) - (0 ?
 15:15) + 1))))))) << (0 ?
 15:15))) | (((gctUINT32) ((gctUINT32) (psInputType[47]) & ((gctUINT32) ((((1 ?
 15:15) - (0 ?
 15:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ? 15:15))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (psInputType[48]) & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:17) - (0 ?
 17:17) + 1))))))) << (0 ?
 17:17))) | (((gctUINT32) ((gctUINT32) (psInputType[49]) & ((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:17) - (0 ? 17:17) + 1))))))) << (0 ? 17:17))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:18) - (0 ?
 18:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:18) - (0 ?
 18:18) + 1))))))) << (0 ?
 18:18))) | (((gctUINT32) ((gctUINT32) (psInputType[50]) & ((gctUINT32) ((((1 ?
 18:18) - (0 ?
 18:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:18) - (0 ? 18:18) + 1))))))) << (0 ? 18:18))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:19) - (0 ?
 19:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:19) - (0 ?
 19:19) + 1))))))) << (0 ?
 19:19))) | (((gctUINT32) ((gctUINT32) (psInputType[51]) & ((gctUINT32) ((((1 ?
 19:19) - (0 ?
 19:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:19) - (0 ? 19:19) + 1))))))) << (0 ? 19:19))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:20) - (0 ?
 20:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:20) - (0 ?
 20:20) + 1))))))) << (0 ?
 20:20))) | (((gctUINT32) ((gctUINT32) (psInputType[52]) & ((gctUINT32) ((((1 ?
 20:20) - (0 ?
 20:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ? 20:20))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ?
 21:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:21) - (0 ?
 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) ((gctUINT32) (psInputType[53]) & ((gctUINT32) ((((1 ?
 21:21) - (0 ?
 21:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ? 21:21))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:22) - (0 ?
 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) ((gctUINT32) (psInputType[54]) & ((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ? 22:22))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:23) - (0 ?
 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) (psInputType[55]) & ((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ? 23:23))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:24) - (0 ?
 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (psInputType[56]) & ((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:25) - (0 ?
 25:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:25) - (0 ?
 25:25) + 1))))))) << (0 ?
 25:25))) | (((gctUINT32) ((gctUINT32) (psInputType[57]) & ((gctUINT32) ((((1 ?
 25:25) - (0 ?
 25:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ? 25:25))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (psInputType[58]) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:27) - (0 ?
 27:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:27) - (0 ?
 27:27) + 1))))))) << (0 ?
 27:27))) | (((gctUINT32) ((gctUINT32) (psInputType[59]) & ((gctUINT32) ((((1 ?
 27:27) - (0 ?
 27:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:27) - (0 ? 27:27) + 1))))))) << (0 ? 27:27))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) ((gctUINT32) (psInputType[60]) & ((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ? 28:28))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:29) - (0 ?
 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) ((gctUINT32) (psInputType[61]) & ((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:30) - (0 ?
 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) ((gctUINT32) (psInputType[62]) & ((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30))) |
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (psInputType[63]) & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31))) ;
                VSC_LOAD_HW_STATE(0x0410 + 1, state);
            }
        }

        if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxVaryingCount > 8)
        {
            state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (componentType[32]) & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))  |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:2) - (0 ?
 3:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:2) - (0 ?
 3:2) + 1))))))) << (0 ?
 3:2))) | (((gctUINT32) ((gctUINT32) (componentType[33]) & ((gctUINT32) ((((1 ?
 3:2) - (0 ?
 3:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:2) - (0 ? 3:2) + 1))))))) << (0 ? 3:2)))  |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (componentType[34]) & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)))  |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:6) - (0 ?
 7:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:6) - (0 ?
 7:6) + 1))))))) << (0 ?
 7:6))) | (((gctUINT32) ((gctUINT32) (componentType[35]) & ((gctUINT32) ((((1 ?
 7:6) - (0 ?
 7:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ? 7:6)))  |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:8) - (0 ?
 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (componentType[36]) & ((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8)))  |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:10) - (0 ?
 11:10) + 1))))))) << (0 ?
 11:10))) | (((gctUINT32) ((gctUINT32) (componentType[37]) & ((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:10) - (0 ? 11:10) + 1))))))) << (0 ? 11:10)))  |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:12) - (0 ?
 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) ((gctUINT32) (componentType[38]) & ((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ? 13:12)))  |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) ((gctUINT32) (componentType[39]) & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))  |
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
 ~0U : (~(~0U << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));
            VSC_LOAD_HW_STATE(0x0E0E, state);
        }

        if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxVaryingCount > 12)
        {
            state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 ~0U : (~(~0U << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));
            VSC_LOAD_HW_STATE(0x0E15, state);
        }
    }

    /* Program sampler-related. */
    if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasSamplerBaseOffset)
    {
        state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:0) - (0 ?
 6:0) + 1))))))) << (0 ?
 6:0))) | (((gctUINT32) ((gctUINT32) (pShHwInfo->hwProgrammingHints.hwSamplerRegAddrOffset) & ((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:0) - (0 ? 6:0) + 1))))))) << (0 ? 6:0)));
        VSC_LOAD_HW_STATE(0x0416, state);
    }

    /* Program CTCs */
    errCode = _ProgramPsCompileTimeConsts(pShHwInfo, pStatesPgmer);
    ON_ERROR(errCode, "Program PS CTC");

    /* Program insts */
    errCode = _ProgramPsInsts(pShHwInfo, pStatesPgmer);
    ON_ERROR(errCode, "Program PS inst");

    /* Program gpr spill */
    if (pPsSEP->exeHints.derivedHints.globalStates.bGprSpilled)
    {
        pStatesPgmer->pHints->useGPRSpill[gcvPROGRAM_STAGE_FRAGMENT] = gcvTRUE;
        errCode = _ProgramPsGprSpill(pShHwInfo, pStatesPgmer);
        ON_ERROR(errCode, "Program PS grp spill");
    }

    /* Program cr spill */
    if (pPsSEP->exeHints.derivedHints.globalStates.bCrSpilled)
    {
        if (DECODE_SHADER_CLIENT(pPsSEP->shVersionType) == SHADER_CLIENT_VK)
        {
            errCode = _ProgramPsCrSpill(pShHwInfo, pStatesPgmer);
            ON_ERROR(errCode, "Program PS cr spill");
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _AllocVidMemForSharedMemory(VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer,
                                               SHADER_EXECUTABLE_PROFILE* pGpsSEP,
                                               gcsSURF_NODE_PTR* pSharedMemVidmemNode,
                                               gctUINT32* pVidMemAddrOfSharedMem,
                                               gctUINT* pSharedMemSize)

{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    gctUINT                    sharedMemVidMemSizePerGrp = 0, totalSharedMemVidMemSize = 0, i;
    gctUINT32                  physical = NOT_ASSIGNED;

    for (i = 0; i < pGpsSEP->staticPrivMapping.privUavMapping.countOfEntries; i ++)
    {
        if (pGpsSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].commonPrivm.privmKind == SHS_PRIV_MEM_KIND_SHARED_MEMORY)
        {
            gcmASSERT(pGpsSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].memData.ppCnstSubArray == gcvNULL);
            gcmASSERT(pGpsSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].memData.ppCTC == gcvNULL);

            gcmASSERT(pGpsSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].pBuffer->sizeInByte ==
                      pGpsSEP->exeHints.nativeHints.prvStates.gps.shareMemSizePerThreadGrpInByte);

            sharedMemVidMemSizePerGrp = pGpsSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].pBuffer->sizeInByte;

            break;
        }
    }

    totalSharedMemVidMemSize = pGpsSEP->exeHints.nativeHints.prvStates.gps.currWorkGrpNum * sharedMemVidMemSizePerGrp;

    gcmASSERT(totalSharedMemVidMemSize);

    (*pStatesPgmer->pSysCtx->drvCBs.pfnAllocVidMemCb)(pStatesPgmer->pSysCtx->hDrv,
                                                      gcvSURF_VERTEX,
                                                      "shared memory",
                                                      totalSharedMemVidMemSize,
                                                      64,
                                                      (gctPOINTER *)pSharedMemVidmemNode,
                                                      gcvNULL,
                                                      &physical,
                                                      gcvNULL,
                                                      gcvTRUE);

    if (NOT_ASSIGNED == physical)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        ON_ERROR(errCode, "Create shared-mem vid-mem");
    }

    *pVidMemAddrOfSharedMem = physical;
    *pSharedMemSize = totalSharedMemVidMemSize;

OnError:
    return errCode;
}

static VSC_ErrCode _ProgramSharedMemAddr(SHADER_EXECUTABLE_PROFILE* pGpsSEP,
                                           gctUINT startConstRegAddr,
                                           gctUINT sharedMemAddr,
                                           gctUINT sharedMemSize,
                                           VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                           errCode = VSC_ERR_NONE;
    gctUINT                               channel, regAddr, i;
    SHADER_CONSTANT_HW_LOCATION_MAPPING*  pConstHwLocMapping = gcvNULL;

    for (i = 0; i < pGpsSEP->staticPrivMapping.privUavMapping.countOfEntries; i ++)
    {
        if (pGpsSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].commonPrivm.privmKind == SHS_PRIV_MEM_KIND_SHARED_MEMORY)
        {
            gcmASSERT(pGpsSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].memData.ppCnstSubArray == gcvNULL);
            gcmASSERT(pGpsSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].memData.ppCTC == gcvNULL);

            gcmASSERT(pGpsSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].pBuffer->hwMemAccessMode ==
                      SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR);

            pConstHwLocMapping = pGpsSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].pBuffer->hwLoc.pHwDirectAddrBase;

            break;
        }
    }

    gcmASSERT(pConstHwLocMapping);
    gcmASSERT(pConstHwLocMapping->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER);

    if (pGpsSEP->exeHints.derivedHints.globalStates.bEnableRobustCheck)
    {
        gctUINT upperLimit;
        /* must at start with x and at least .x.y.z channels are allocated */
        gcmASSERT((pConstHwLocMapping->validHWChannelMask & 0x07) == 0x07);
        regAddr = startConstRegAddr + (pConstHwLocMapping->hwLoc.constReg.hwRegNo * CHANNEL_NUM);
        VSC_LOAD_HW_STATE(regAddr, sharedMemAddr);

        regAddr += 1;  /* .y lower limit */
        VSC_LOAD_HW_STATE(regAddr, sharedMemAddr);

        regAddr += 1;  /* .z upper limit */
        upperLimit = sharedMemAddr + sharedMemSize - 1;
        VSC_LOAD_HW_STATE(regAddr, upperLimit);
    }
    else
    {
        for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
        {
            if (pConstHwLocMapping->validHWChannelMask & (1 << channel))
            {
                regAddr = startConstRegAddr + (pConstHwLocMapping->hwLoc.constReg.hwRegNo * CHANNEL_NUM) + channel;
                VSC_LOAD_HW_STATE(regAddr, sharedMemAddr);
            }
        }
    }
OnError:
    return errCode;
}

static VSC_ErrCode _ProgramGpsSharedMemory(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    SHADER_EXECUTABLE_PROFILE* pGpsSEP = pShHwInfo->pSEP;
    gcsSURF_NODE_PTR           sharedVidmemNode = gcvNULL;
    gctUINT                    vidMemAddrOfSharedMem = NOT_ASSIGNED;
    gctBOOL                    threadWalkInPs = pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasThreadWalkerInPS;
    gctUINT                    sharedMemSize = 0;

    errCode = _AllocVidMemForSharedMemory(pStatesPgmer, pGpsSEP, &sharedVidmemNode, &vidMemAddrOfSharedMem, &sharedMemSize);
    ON_ERROR(errCode, "Alloc vid-mem for shared memory");

    pStatesPgmer->pHints->shaderVidNodes.sharedMemVidMemNode = sharedVidmemNode;

    _SetPatchOffsets(pStatesPgmer, _VSC_PATCH_OFFSET_SHAREMEM, 0);

    errCode = _ProgramSharedMemAddr(pGpsSEP,
                                    threadWalkInPs ?
                                    _GetPsStartConstRegAddr(pShHwInfo, pStatesPgmer)
                                  : _GetVsStartConstRegAddr(pShHwInfo, pStatesPgmer),
                                    vidMemAddrOfSharedMem,
                                    sharedMemSize,
                                    pStatesPgmer);
    ON_ERROR(errCode, "Program shared mem address ");

OnError:
    return errCode;
}

/* Alloate the video memory to save the thread ID. */
static VSC_ErrCode _AllocVidMemForThreadID(VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer,
                                           SHADER_EXECUTABLE_PROFILE* pGpsSEP,
                                           gcsSURF_NODE_PTR* pThreadIDVidmemNode,
                                           gctUINT32* pVidMemAddrOfThreadID,
                                           gctUINT* pThreadIDSize)

{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    gctUINT                    ThreadIdVidMemSize = 0, i;
    gctUINT32                  physical = NOT_ASSIGNED;

    for (i = 0; i < pGpsSEP->staticPrivMapping.privUavMapping.countOfEntries; i ++)
    {
        if (pGpsSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].commonPrivm.privmKind == SHS_PRIV_MEM_KIND_THREAD_ID_MEM_ADDR)
        {
            gcmASSERT(pGpsSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].memData.ppCnstSubArray == gcvNULL);
            gcmASSERT(pGpsSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].memData.ppCTC == gcvNULL);

            ThreadIdVidMemSize = pGpsSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].pBuffer->sizeInByte;
            gcmASSERT(ThreadIdVidMemSize == 4);

            break;
        }
    }

    if (ThreadIdVidMemSize == 0)
    {
        return errCode;
    }

    (*pStatesPgmer->pSysCtx->drvCBs.pfnAllocVidMemCb)(pStatesPgmer->pSysCtx->hDrv,
                                                      gcvSURF_VERTEX,
                                                      "shared memory",
                                                      ThreadIdVidMemSize,
                                                      64,
                                                      (gctPOINTER *)pThreadIDVidmemNode,
                                                      gcvNULL,
                                                      &physical,
                                                      gcvNULL,
                                                      gcvTRUE);

    if (NOT_ASSIGNED == physical)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        ON_ERROR(errCode, "Create shared-mem vid-mem");
    }

    *pVidMemAddrOfThreadID = physical;
    *pThreadIDSize = ThreadIdVidMemSize;

OnError:
    return errCode;
}

/* Program the thread ID memory address. */
static VSC_ErrCode _ProgramThreadIdMemAddr(SHADER_EXECUTABLE_PROFILE* pGpsSEP,
                                           gctUINT startConstRegAddr,
                                           gctUINT threadIdMemAddr,
                                           gctUINT threadIdMemSize,
                                           VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                           errCode = VSC_ERR_NONE;
    gctUINT                               channel, regAddr, i;
    SHADER_CONSTANT_HW_LOCATION_MAPPING*  pConstHwLocMapping = gcvNULL;

    for (i = 0; i < pGpsSEP->staticPrivMapping.privUavMapping.countOfEntries; i ++)
    {
        if (pGpsSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].commonPrivm.privmKind == SHS_PRIV_MEM_KIND_THREAD_ID_MEM_ADDR)
        {
            gcmASSERT(pGpsSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].memData.ppCnstSubArray == gcvNULL);
            gcmASSERT(pGpsSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].memData.ppCTC == gcvNULL);

            gcmASSERT(pGpsSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].pBuffer->hwMemAccessMode ==
                      SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR);

            pConstHwLocMapping = pGpsSEP->staticPrivMapping.privUavMapping.pPrivUavEntries[i].pBuffer->hwLoc.pHwDirectAddrBase;

            break;
        }
    }

    gcmASSERT(pConstHwLocMapping);
    gcmASSERT(pConstHwLocMapping->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER);

    if (pGpsSEP->exeHints.derivedHints.globalStates.bEnableRobustCheck)
    {
        gctUINT upperLimit;
        /* must at start with x and at least .x.y.z channels are allocated */
        gcmASSERT((pConstHwLocMapping->validHWChannelMask & 0x07) == 0x07);
        regAddr = startConstRegAddr + (pConstHwLocMapping->hwLoc.constReg.hwRegNo * CHANNEL_NUM);
        VSC_LOAD_HW_STATE(regAddr, threadIdMemAddr);

        regAddr += 1;  /* .y lower limit */
        VSC_LOAD_HW_STATE(regAddr, threadIdMemAddr);

        regAddr += 1;  /* .z upper limit */
        upperLimit = threadIdMemAddr + threadIdMemSize - 1;
        VSC_LOAD_HW_STATE(regAddr, upperLimit);
    }
    else
    {
        for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
        {
            if (pConstHwLocMapping->validHWChannelMask & (1 << channel))
            {
                regAddr = startConstRegAddr + (pConstHwLocMapping->hwLoc.constReg.hwRegNo * CHANNEL_NUM) + channel;
                VSC_LOAD_HW_STATE(regAddr, threadIdMemAddr);
            }
        }
    }
OnError:
    return errCode;
}

/* Allocate the program the thread ID memory. */
static VSC_ErrCode _ProgramGpsThreadIDMemory(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    SHADER_EXECUTABLE_PROFILE* pGpsSEP = pShHwInfo->pSEP;
    gcsSURF_NODE_PTR           threadIdVidMemNode = gcvNULL;
    gctUINT                    threadIdMemAddrOfSharedMem = NOT_ASSIGNED;
    gctBOOL                    threadWalkInPs = pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasThreadWalkerInPS;
    gctUINT                    threadIdMemSize = 0;

    errCode = _AllocVidMemForThreadID(pStatesPgmer, pGpsSEP, &threadIdVidMemNode, &threadIdMemAddrOfSharedMem, &threadIdMemSize);
    ON_ERROR(errCode, "Alloc vid-mem for thread ID");

    if (threadIdMemSize == 0)
    {
        return errCode;
    }

    pStatesPgmer->pHints->shaderVidNodes.threadIdVidMemNode = threadIdVidMemNode;

    _SetPatchOffsets(pStatesPgmer, _VSC_PATCH_OFFSET_THREADID, 0);

    errCode = _ProgramThreadIdMemAddr(pGpsSEP,
                                      threadWalkInPs ? _GetPsStartConstRegAddr(pShHwInfo, pStatesPgmer) : _GetVsStartConstRegAddr(pShHwInfo, pStatesPgmer),
                                      threadIdMemAddrOfSharedMem,
                                      threadIdMemSize,
                                      pStatesPgmer);
    ON_ERROR(errCode, "Program thread ID memory address ");

OnError:
    return errCode;
}

static VSC_ErrCode _ProgramGPS(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    SHADER_EXECUTABLE_PROFILE* pGpsSEP = pShHwInfo->pSEP;
    gctUINT                    i, state, ioIdx, sortedIoIdx;
    gctUINT                    gpsInputCount = 0, theadIdIdx = 0;
    gctUINT                    firstValidIoChannel;
    gctUINT                    varyingPacking[2] = {0, 0};
    gctUINT                    calibratedGprCount = _GetCalibratedGprCount(pShHwInfo, pStatesPgmer);
    SHADER_IO_USAGE            threadIdSeq[3] = {SHADER_IO_USAGE_THREADID,
                                                 SHADER_IO_USAGE_THREADID,
                                                 SHADER_IO_USAGE_THREADID};
#if !IO_HW_LOC_NUMBER_IN_ORDER
    gctUINT                    sortedIoIdxArray[MAX_SHADER_IO_NUM];
#endif
    gctBOOL                    threadWalkInPs = pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasThreadWalkerInPS;

    gcmASSERT(pGpsSEP->inputMapping.ioVtxPxl.ioMode == SHADER_IO_MODE_PASSIVE);
    gcmASSERT(pGpsSEP->outputMapping.ioVtxPxl.ioMode == SHADER_IO_MODE_PASSIVE);

#if !IO_HW_LOC_NUMBER_IN_ORDER
    vscSortIOsByHwLoc(&pGpsSEP->inputMapping.ioVtxPxl, sortedIoIdxArray);
#endif

    /* Input */
    for (ioIdx = 0; ioIdx < pGpsSEP->inputMapping.ioVtxPxl.countOfIoRegMapping; ioIdx ++)
    {
#if !IO_HW_LOC_NUMBER_IN_ORDER
        sortedIoIdx = sortedIoIdxArray[ioIdx];
#else
        sortedIoIdx = ioIdx;
#endif

        if (pGpsSEP->inputMapping.ioVtxPxl.ioIndexMask & (1LL << sortedIoIdx))
        {
            if (gpsInputCount >= pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxVaryingCount)
            {
                return VSC_ERR_TOO_MANY_VARYINGS;
            }

            if (pGpsSEP->inputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_THREADID].ioIndexMask & (1LL << sortedIoIdx) ||
                pGpsSEP->inputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_THREADGROUPID].ioIndexMask & (1LL << sortedIoIdx) ||
                pGpsSEP->inputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_THREADIDINGROUP].ioIndexMask & (1LL << sortedIoIdx))
            {
                gcmASSERT(theadIdIdx < 3);

                firstValidIoChannel = pGpsSEP->inputMapping.ioVtxPxl.pIoRegMapping[sortedIoIdx].firstValidIoChannel;
                threadIdSeq[theadIdIdx ++] = pGpsSEP->inputMapping.ioVtxPxl.pIoRegMapping[sortedIoIdx].
                                             ioChannelMapping[firstValidIoChannel].ioUsage;
            }

            /* Ok, increase count */
            gpsInputCount ++;
        }
    }

    switch (threadIdSeq[0])
    {
    case SHADER_IO_USAGE_THREADID:
        if (threadIdSeq[1] == SHADER_IO_USAGE_THREADGROUPID)
        {
            pStatesPgmer->pHints->valueOrder = GPGPU_THD_GRP_ID_ORDER_GWL2LGW;
        }
        else
        {
            pStatesPgmer->pHints->valueOrder = GPGPU_THD_GRP_ID_ORDER_GLW2WGL;
        }
        break;

    case SHADER_IO_USAGE_THREADGROUPID:
        if (threadIdSeq[1] == SHADER_IO_USAGE_THREADID)
        {
            pStatesPgmer->pHints->valueOrder = GPGPU_THD_GRP_ID_ORDER_WGL2LWG;
        }
        else
        {
            pStatesPgmer->pHints->valueOrder = GPGPU_THD_GRP_ID_ORDER_WLG2GWL;
        }
        break;

    case SHADER_IO_USAGE_THREADIDINGROUP:
        if (threadIdSeq[1] == SHADER_IO_USAGE_THREADID)
        {
            pStatesPgmer->pHints->valueOrder = GPGPU_THD_GRP_ID_ORDER_LGW2WLG;
        }
        else
        {
            pStatesPgmer->pHints->valueOrder = GPGPU_THD_GRP_ID_ORDER_LWG2GLW;
        }
        break;

    default:
        pStatesPgmer->pHints->valueOrder = GPGPU_THD_GRP_ID_ORDER_LWG2GLW;
        break;
    }

    pStatesPgmer->pHints->threadWalkerInPS = threadWalkInPs;
    pStatesPgmer->pHints->elementCount = gpsInputCount;
    pStatesPgmer->pHints->fsMaxTemp = calibratedGprCount;
    pStatesPgmer->pHints->vsMaxTemp = threadWalkInPs ? 0 : calibratedGprCount;
    pStatesPgmer->pHints->fsInstCount = pGpsSEP->countOfMCInst;
    pStatesPgmer->pHints->fsIsDual16 = pGpsSEP->exeHints.derivedHints.globalStates.bExecuteOnDual16;
    pStatesPgmer->pHints->fsInputCount = (gpsInputCount > 0) ? gpsInputCount : 1;
    pStatesPgmer->pHints->workGrpSize.x = pGpsSEP->exeHints.nativeHints.prvStates.gps.threadGrpDimX;
    pStatesPgmer->pHints->workGrpSize.y = pGpsSEP->exeHints.nativeHints.prvStates.gps.threadGrpDimY;
    pStatesPgmer->pHints->workGrpSize.z = pGpsSEP->exeHints.nativeHints.prvStates.gps.threadGrpDimZ;
    pStatesPgmer->pHints->threadGroupSync = pGpsSEP->exeHints.derivedHints.prvStates.gps.bThreadGroupSync;
    pStatesPgmer->pHints->useGroupId = (pGpsSEP->inputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_THREADGROUPID].ioIndexMask != 0);
    pStatesPgmer->pHints->useLocalId = (pGpsSEP->inputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_THREADIDINGROUP].ioIndexMask != 0);
    pStatesPgmer->pHints->useEvisInst = pGpsSEP->exeHints.derivedHints.prvStates.gps.bUseEvisInst;
    for (i = 0; i < 3; i++)
    {
        pStatesPgmer->pHints->workGroupSizeFactor[i] = pGpsSEP->exeHints.derivedHints.prvStates.gps.workGroupSizeFactor[i];
    }

    _ProgramSamplerCountInfo(pShHwInfo, pStatesPgmer, gcvFALSE);

    if (DECODE_SHADER_CLIENT(pGpsSEP->shVersionType) == SHADER_CLIENT_CL)
    {
        pStatesPgmer->pHints->memoryAccessFlags[gcvSHADER_HIGH_LEVEL][gcvPROGRAM_STAGE_OPENCL] =
                               (gceMEMORY_ACCESS_FLAG)pGpsSEP->exeHints.nativeHints.globalStates.memoryAccessHint;
        pStatesPgmer->pHints->memoryAccessFlags[gcvSHADER_MACHINE_LEVEL][gcvPROGRAM_STAGE_OPENCL] =
                               (gceMEMORY_ACCESS_FLAG)pGpsSEP->exeHints.derivedHints.globalStates.memoryAccessHint;

        pStatesPgmer->pHints->flowControlFlags[gcvSHADER_HIGH_LEVEL][gcvPROGRAM_STAGE_COMPUTE] =
                               (gceFLOW_CONTROL_FLAG)pGpsSEP->exeHints.nativeHints.globalStates.flowControlHint;
        pStatesPgmer->pHints->flowControlFlags[gcvSHADER_MACHINE_LEVEL][gcvPROGRAM_STAGE_COMPUTE] =
                               (gceFLOW_CONTROL_FLAG)pGpsSEP->exeHints.derivedHints.globalStates.flowControlHint;

        pStatesPgmer->pHints->texldFlags[gcvSHADER_HIGH_LEVEL][gcvPROGRAM_STAGE_COMPUTE] =
                               (gceTEXLD_FLAG)pGpsSEP->exeHints.nativeHints.globalStates.texldHint;
        pStatesPgmer->pHints->texldFlags[gcvSHADER_MACHINE_LEVEL][gcvPROGRAM_STAGE_COMPUTE] =
                               (gceTEXLD_FLAG)pGpsSEP->exeHints.derivedHints.globalStates.texldHint;

        pStatesPgmer->pHints->samplerBaseOffset[gcvPROGRAM_STAGE_OPENCL] =
                               pShHwInfo->hwProgrammingHints.hwSamplerRegAddrOffset;
    }
    else
    {
        pStatesPgmer->pHints->memoryAccessFlags[gcvSHADER_HIGH_LEVEL][gcvPROGRAM_STAGE_COMPUTE] =
                               (gceMEMORY_ACCESS_FLAG)pGpsSEP->exeHints.nativeHints.globalStates.memoryAccessHint;
        pStatesPgmer->pHints->memoryAccessFlags[gcvSHADER_MACHINE_LEVEL][gcvPROGRAM_STAGE_COMPUTE] =
                               (gceMEMORY_ACCESS_FLAG)pGpsSEP->exeHints.derivedHints.globalStates.memoryAccessHint;

        pStatesPgmer->pHints->flowControlFlags[gcvSHADER_HIGH_LEVEL][gcvPROGRAM_STAGE_COMPUTE] =
                               (gceFLOW_CONTROL_FLAG)pGpsSEP->exeHints.nativeHints.globalStates.flowControlHint;
        pStatesPgmer->pHints->flowControlFlags[gcvSHADER_MACHINE_LEVEL][gcvPROGRAM_STAGE_COMPUTE] =
                               (gceFLOW_CONTROL_FLAG)pGpsSEP->exeHints.derivedHints.globalStates.flowControlHint;
        pStatesPgmer->pHints->texldFlags[gcvSHADER_HIGH_LEVEL][gcvPROGRAM_STAGE_COMPUTE] =
                               (gceTEXLD_FLAG)pGpsSEP->exeHints.nativeHints.globalStates.texldHint;
        pStatesPgmer->pHints->texldFlags[gcvSHADER_MACHINE_LEVEL][gcvPROGRAM_STAGE_COMPUTE] =
                               (gceTEXLD_FLAG)pGpsSEP->exeHints.derivedHints.globalStates.texldHint;
        pStatesPgmer->pHints->samplerBaseOffset[gcvPROGRAM_STAGE_COMPUTE] =
                               pShHwInfo->hwProgrammingHints.hwSamplerRegAddrOffset;
    }

    /* Save the concurrent workThreadCount. */
    pStatesPgmer->pHints->workThreadCount = (gctUINT16)pGpsSEP->exeHints.nativeHints.prvStates.gps.currWorkThreadNum;

    /* Save the concurrent workGroupCount. */
    pStatesPgmer->pHints->workGroupCount = (gctUINT16)pGpsSEP->exeHints.nativeHints.prvStates.gps.currWorkGrpNum;

    switch (gpsInputCount)
    {
    case 3:
        varyingPacking[1] = 2;
        /*fall through*/
    case 2:
        varyingPacking[0] = 2;
    }

    pStatesPgmer->pHints->componentCount = gcmALIGN(varyingPacking[0] + varyingPacking[1], 2);

    if (threadWalkInPs)
    {
        /* Program varying pack */
        state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ? 6:4)));

        if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.newGPIPE)
        {
            VSC_LOAD_HW_STATE(0x02A4, state);
            VSC_LOAD_HW_STATE(0x0420, state);
        }
        else
        {
            VSC_LOAD_HW_STATE(0x0E08, state);

            state = 0;
            VSC_LOAD_HW_STATE(0x0E0D, state);
        }

        /* Program PSCS_THROTTLE reg. */
        if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.supportPSCSThrottle)
        {
            state = 0;
            state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:0) - (0 ?
 6:0) + 1))))))) << (0 ?
 6:0))) | (((gctUINT32) ((gctUINT32) ((pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxLocalMemSizeInByte >> 10)) & ((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:0) - (0 ? 6:0) + 1))))))) << (0 ? 6:0)));
            if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.supportHWManagedLS)
            {
                state |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:8) - (0 ?
 15:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:8) - (0 ?
 15:8) + 1))))))) << (0 ?
 15:8))) | (((gctUINT32) ((gctUINT32) (pGpsSEP->exeHints.nativeHints.prvStates.gps.workGroupNumPerShaderGroup) & ((gctUINT32) ((((1 ?
 15:8) - (0 ?
 15:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8)));
            }
            VSC_LOAD_HW_STATE(0x0427, state);
        }

        /* Program sampler-related. */
        if (pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasSamplerBaseOffset)
        {
            state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:0) - (0 ?
 6:0) + 1))))))) << (0 ?
 6:0))) | (((gctUINT32) ((gctUINT32) (pShHwInfo->hwProgrammingHints.hwSamplerRegAddrOffset) & ((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:0) - (0 ? 6:0) + 1))))))) << (0 ? 6:0)));
            VSC_LOAD_HW_STATE(0x0416, state);
        }

        /* Program CTCs */
        errCode = _ProgramPsCompileTimeConsts(pShHwInfo, pStatesPgmer);
        ON_ERROR(errCode, "Program GP CTC");

        /* Program insts */
        errCode = _ProgramPsInsts(pShHwInfo, pStatesPgmer);
        ON_ERROR(errCode, "Program GP inst");

        /* Program gpr spill */
        if (pGpsSEP->exeHints.derivedHints.globalStates.bGprSpilled)
        {
            if (DECODE_SHADER_CLIENT(pGpsSEP->shVersionType) == SHADER_CLIENT_CL)
            {
                pStatesPgmer->pHints->useGPRSpill[gcvPROGRAM_STAGE_OPENCL] = gcvTRUE;
            }
            else
            {
                pStatesPgmer->pHints->useGPRSpill[gcvPROGRAM_STAGE_COMPUTE] = gcvTRUE;
            }
            /* Do not program the reg spill memory when multi-GPU is enabled, let driver programs it. */
            if (!pGpsSEP->exeHints.derivedHints.globalStates.bEnableMultiGPU)
            {
                errCode = _ProgramPsGprSpill(pShHwInfo, pStatesPgmer);
                ON_ERROR(errCode, "Program GP grp spill");
            }
        }

        /* Program cr spill */
        if (pGpsSEP->exeHints.derivedHints.globalStates.bCrSpilled)
        {
            if (DECODE_SHADER_CLIENT(pGpsSEP->shVersionType) == SHADER_CLIENT_VK ||
                DECODE_SHADER_CLIENT(pGpsSEP->shVersionType) == SHADER_CLIENT_CL)
            {
                errCode = _ProgramPsCrSpill(pShHwInfo, pStatesPgmer);
                ON_ERROR(errCode, "Program GP cr spill");
            }
        }
    }
    else
    {
        gctUINT timeoutofFE2VS;
        gctUINT vsInputCount = (gpsInputCount > 0) ? gpsInputCount : 1;
        timeoutofFE2VS = gcmALIGN(vsInputCount * 4 + 4, 16) / 16;
        state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (vsInputCount) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) ((gctUINT32) (timeoutofFE2VS) & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)));

        VSC_LOAD_HW_STATE(0x0202, state);

        /* Program used GPR count */
        state = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:0) - (0 ?
 6:0) + 1))))))) << (0 ?
 6:0))) | (((gctUINT32) ((gctUINT32) (calibratedGprCount) & ((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:0) - (0 ? 6:0) + 1))))))) << (0 ? 6:0)));
        VSC_LOAD_HW_STATE(0x0203, state);

        /* Program CTCs */
        errCode = _ProgramVsCompileTimeConsts(pShHwInfo, pStatesPgmer);
        ON_ERROR(errCode, "Program GP CTC");

        /* Program insts */
        errCode = _ProgramVsInsts(pShHwInfo, pStatesPgmer);
        ON_ERROR(errCode, "Program GP inst");

        /* Program gpr spill */
        if (pGpsSEP->exeHints.derivedHints.globalStates.bGprSpilled)
        {
            if (DECODE_SHADER_CLIENT(pGpsSEP->shVersionType) == SHADER_CLIENT_CL)
            {
                pStatesPgmer->pHints->useGPRSpill[gcvPROGRAM_STAGE_OPENCL] = gcvTRUE;
            }
            else
            {
                pStatesPgmer->pHints->useGPRSpill[gcvPROGRAM_STAGE_COMPUTE] = gcvTRUE;
            }
            /* Do not program the reg spill memory when multi-GPU is enabled, let driver programs it. */
            if (!pGpsSEP->exeHints.derivedHints.globalStates.bEnableMultiGPU)
            {
                errCode = _ProgramVsGprSpill(pShHwInfo, pStatesPgmer);
                ON_ERROR(errCode, "Program GP grp spill");
            }
        }
    }

    if (pGpsSEP->exeHints.derivedHints.prvStates.gps.bUseLocalMemory)
    {
        /* Save local memory size. */
        pStatesPgmer->pHints->localMemSizeInByte = pGpsSEP->exeHints.nativeHints.prvStates.gps.shareMemSizePerThreadGrpInByte;
    }
    /* Program simulated shared (local) memory */
    else if (pGpsSEP->exeHints.nativeHints.prvStates.gps.shareMemSizePerThreadGrpInByte > 0)
    {
        if (DECODE_SHADER_CLIENT(pGpsSEP->shVersionType) == SHADER_CLIENT_VK ||
            DECODE_SHADER_CLIENT(pGpsSEP->shVersionType) == SHADER_CLIENT_CL)
        {
             /* Do not program the shared memory when multi-GPU is enabled, let driver programs it. */
            if (pGpsSEP->exeHints.derivedHints.globalStates.bEnableMultiGPU)
            {
                gcmASSERT(DECODE_SHADER_CLIENT(pGpsSEP->shVersionType) == SHADER_CLIENT_CL);
            }
            else
            {
                errCode = _ProgramGpsSharedMemory(pShHwInfo, pStatesPgmer);
                ON_ERROR(errCode, "Program GP shared memory");

                pStatesPgmer->pHints->sharedMemAllocByCompiler = gcvTRUE;
            }
        }
    }

    /* Allocate the global memory to save the thread ID. */
    if (pGpsSEP->exeHints.derivedHints.prvStates.gps.bUsePrivateMemory)
    {
        errCode = _ProgramGpsThreadIDMemory(pShHwInfo, pStatesPgmer);
        ON_ERROR(errCode, "Program thread ID memory");
    }

OnError:
    return errCode;
}

VSC_ErrCode vscProgramShaderStates(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer)
{
    VSC_ErrCode   errCode = VSC_ERR_NONE;
    SHADER_TYPE   shType = (SHADER_TYPE)DECODE_SHADER_TYPE(pShHwInfo->pSEP->shVersionType);
    SHADER_CLIENT shClient = (SHADER_CLIENT)DECODE_SHADER_CLIENT(pShHwInfo->pSEP->shVersionType);

    gcmASSERT(pStatesPgmer->pSysCtx && pStatesPgmer->pHints);

    switch (shType)
    {
    case SHADER_TYPE_VERTEX:
        gcsHINT_SetProgramStageBit(pStatesPgmer->pHints, gcvPROGRAM_STAGE_VERTEX);

        errCode = _ProgramVS(pShHwInfo, pStatesPgmer);
        ON_ERROR(errCode, "Program VS states");
        break;

    case SHADER_TYPE_HULL:
        gcsHINT_SetProgramStageBit(pStatesPgmer->pHints, gcvPROGRAM_STAGE_TCS);

        errCode = _ProgramHS(pShHwInfo, pStatesPgmer);
        ON_ERROR(errCode, "Program HS states");
        break;

    case SHADER_TYPE_DOMAIN:
        gcsHINT_SetProgramStageBit(pStatesPgmer->pHints, gcvPROGRAM_STAGE_TES);

        errCode = _ProgramDS(pShHwInfo, pStatesPgmer);
        ON_ERROR(errCode, "Program DS states");
        break;

    case SHADER_TYPE_GEOMETRY:
        gcsHINT_SetProgramStageBit(pStatesPgmer->pHints, gcvPROGRAM_STAGE_GEOMETRY);

        errCode = _ProgramGS(pShHwInfo, pStatesPgmer);
        ON_ERROR(errCode, "Program GS states");
        break;

    case SHADER_TYPE_PIXEL:
        gcsHINT_SetProgramStageBit(pStatesPgmer->pHints, gcvPROGRAM_STAGE_FRAGMENT);

        errCode = _ProgramPS(pShHwInfo, pStatesPgmer);
        ON_ERROR(errCode, "Program PS states");
        break;

    case SHADER_TYPE_GENERAL:
        gcsHINT_SetProgramStageBit(pStatesPgmer->pHints, ((shClient == SHADER_CLIENT_CL) ?
                                   gcvPROGRAM_STAGE_OPENCL : gcvPROGRAM_STAGE_COMPUTE));

        errCode = _ProgramGPS(pShHwInfo, pStatesPgmer);
        ON_ERROR(errCode, "Program GP states");
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    /* Set instruction fetch mode. */
    if (pShHwInfo->hwProgrammingHints.hwInstFetchMode == HW_INST_FETCH_MODE_UNIFIED_BUFFER_0 ||
        pShHwInfo->hwProgrammingHints.hwInstFetchMode == HW_INST_FETCH_MODE_UNIFIED_BUFFER_1)
    {
        pStatesPgmer->pHints->unifiedStatus.instruction = gcvTRUE;
        pStatesPgmer->pHints->maxInstCount = pStatesPgmer->pSysCtx->pCoreSysCtx->hwCfg.maxHwNativeTotalInstCount;
    }
    else if (pShHwInfo->hwProgrammingHints.hwInstFetchMode == HW_INST_FETCH_MODE_CACHE)
    {
        pStatesPgmer->pHints->unifiedStatus.useIcache = gcvTRUE;
    }

    /* Set constant reg fetch mode. */
    pStatesPgmer->pHints->unifiedStatus.constantUnifiedMode = (gceUNIFOEM_ALLOC_MODE)
        pShHwInfo->pSEP->exeHints.derivedHints.globalStates.unifiedConstRegAllocStrategy;

    /* Set sampler fetch mode. */
    pStatesPgmer->pHints->unifiedStatus.samplerUnifiedMode = (gceUNIFOEM_ALLOC_MODE)
        pShHwInfo->pSEP->exeHints.derivedHints.globalStates.unifiedSamplerRegAllocStrategy;

OnError:
    return errCode;
}


