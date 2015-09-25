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


#include "gc_vsc.h"
#include "vir/codegen/gc_vsc_vir_ep_gen.h"
#include "chip/gc_vsc_chip_mc_codec.h"

/* Temporarily, use old MC dump function */
extern void
gcDumpMCStates(
    IN gctUINT32 Address,
    IN gctUINT32 * States,
    IN gctBOOL OutputFormat,
    IN gctBOOL OutputHexStates,
    IN gctBOOL OutputDual16Modifiers,
    IN gctSIZE_T BufSize,
    OUT gctSTRING Buffer
    );

/* Defined in drvi_link.c */
extern void _ConvertVirPerVtxPxlAndPerPrimIoList(
    VIR_Shader* pShader,
    VSC_MM* pMM,
    gctBOOL bInput,
    VIR_IdList* pPerVtxPxlList,
    VIR_IdList* pPerPrimList);

typedef struct _VSC_BASE_EP_GEN_HELPER
{
    VSC_PRIMARY_MEM_POOL              pmp;
    PVSC_HW_CONFIG                    pHwCfg;
}VSC_BASE_EP_GEN_HELPER;

typedef struct _VSC_SEP_GEN_HELPER
{
    VSC_BASE_EP_GEN_HELPER            baseEpGen;
    VIR_Shader*                       pShader;
}VSC_SEP_GEN_HELPER;

static gctUINT _GenerateShaderVersionAndType(VIR_Shader* pShader)
{
    SHADER_CLIENT shClient = SHADER_CLIENT_UNKNOWN;
    SHADER_TYPE   shType = SHADER_TYPE_UNKNOWN;
    gctUINT8      shMajorVer = 0;
    gctUINT8      shMinorVer = 0;

    switch (pShader->clientApiVersion)
    {
    case gcvAPI_D3D:
        gcmASSERT((pShader->compilerVersion[0] & 0xFFFF) == _SHADER_DX_LANGUAGE_TYPE);
        shClient = SHADER_CLIENT_DX;
        break;
    case gcvAPI_OPENGL_ES11:
    case gcvAPI_OPENGL_ES20:
    case gcvAPI_OPENGL_ES30:
    case gcvAPI_OPENGL_ES31:
        gcmASSERT((pShader->compilerVersion[0] & 0xFFFF) == _SHADER_GL_LANGUAGE_TYPE);
        shClient = SHADER_CLIENT_GLES;
        break;
    case gcvAPI_OPENGL:
        gcmASSERT((pShader->compilerVersion[0] & 0xFFFF) == _SHADER_GL_LANGUAGE_TYPE);
        shClient = SHADER_CLIENT_GL;
        break;
    case gcvAPI_OPENCL:
        gcmASSERT((pShader->compilerVersion[0] & 0xFFFF) == _cldLanguageType);
        shClient = SHADER_CLIENT_CL;
        break;
    default:
        shClient = SHADER_CLIENT_UNKNOWN;
        break;
    }

    switch (pShader->shaderKind)
    {
    case VIR_SHADER_VERTEX:
        shType = SHADER_TYPE_VERTEX;
        break;
    case VIR_SHADER_FRAGMENT:
        shType = SHADER_TYPE_PIXEL;
        break;
    case VIR_SHADER_GEOMETRY:
        shType = SHADER_TYPE_GEOMETRY;
        break;
    case VIR_SHADER_TESSELLATION_CONTROL:
        shType = SHADER_TYPE_HULL;
        break;
    case VIR_SHADER_TESSELLATION_EVALUATION:
        shType = SHADER_TYPE_DOMAIN;
        break;
    case VIR_SHADER_CL:
    case VIR_SHADER_COMPUTE:
        shType = SHADER_TYPE_GENERAL;
        break;
    default:
        shType = SHADER_TYPE_UNKNOWN;
        break;
    }

    switch (pShader->compilerVersion[1])
    {
    case _SHADER_HALTI_VERSION:
    case _SHADER_DX_VERSION_30:
        shMajorVer = 3;
        shMinorVer = 0;
        break;
    case _SHADER_ES31_VERSION:
        shMajorVer = 3;
        shMinorVer = 1;
        break;
    case _SHADER_ES11_VERSION:
    case _cldCL1Dot1:
        shMajorVer = 1;
        shMinorVer = 1;
        break;
    case _cldCL1Dot2:
        shMajorVer = 1;
        shMinorVer = 2;
        break;
    default:
        break;
    }

    return ENCODE_SHADER_VER_TYPE(shClient, shType, shMajorVer, shMinorVer);
}

static VSC_ErrCode _CollectMachineCodeToSEP(VSC_SEP_GEN_HELPER* pSepGenHelper, SHADER_EXECUTABLE_PROFILE* pOutSEP)
{
    VIR_Shader*                    pShader = pSepGenHelper->pShader;
    VIR_FunctionNode*              pFuncNode  = gcvNULL;
    VIR_FuncIterator               funcIter;
    VIR_Instruction*               pInst;
    VIR_InstIterator               instIter;
    gctUINT                        curPC = 0, instCountForThisRoutine;
    gctBOOL                        bHasValidSubRoutine = gcvFALSE, bAppendNopEOM = gcvFALSE;
    VSC_MC_RAW_INST                nopInst;
    VSC_MC_RAW_INST*               pInstOfEOM = gcvNULL;

    pOutSEP->countOfMCInst = 0;

    /* NOP has all zero'ed data */
    nopInst.word[0] = 0;
    nopInst.word[1] = 0;
    nopInst.word[2] = 0;
    nopInst.word[3] = 0;

    /* Calculate HW inst count */
    VIR_FuncIterator_Init(&funcIter, &pShader->functions);
    pFuncNode = VIR_FuncIterator_First(&funcIter);

    for(; pFuncNode != gcvNULL; pFuncNode = VIR_FuncIterator_Next(&funcIter))
    {
        VIR_Function *pFunc = pFuncNode->function;

        VIR_InstIterator_Init(&instIter, &pFunc->instList);
        pInst = VIR_InstIterator_First(&instIter);

        instCountForThisRoutine = 0;
        for (; pInst != gcvNULL; pInst = VIR_InstIterator_Next(&instIter))
        {
            pOutSEP->countOfMCInst += pInst->mcInstCount;

            instCountForThisRoutine += pInst->mcInstCount;
        }

        if (instCountForThisRoutine && !(pFunc->flags & VIR_FUNCFLAG_MAIN))
        {
            bHasValidSubRoutine = gcvTRUE;
        }
    }

    /* We need append a NOP for main function to indicate end of shader execution
       if there is no inst for main function. For shaders that have subroutines,
       NOP should be already added by MC gen pass because it will affect branch
       target */
    if (pOutSEP->countOfMCInst == 0)
    {
        bAppendNopEOM = gcvTRUE;
        pOutSEP->countOfMCInst ++;
    }

    /* Fill HW inst to SEP */
    if (pOutSEP->countOfMCInst)
    {
        /* Allocate HW inst memory of SEP */
        if (gcoOS_Allocate(gcvNULL, pOutSEP->countOfMCInst*sizeof(VSC_MC_RAW_INST),
            (gctPOINTER*)&pOutSEP->pMachineCode) != gcvSTATUS_OK)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }

        /* Main routine firstly */
        VIR_InstIterator_Init(&instIter, &pShader->mainFunction->instList);
        pInst = VIR_InstIterator_First(&instIter);

        for (; pInst != gcvNULL; pInst = VIR_InstIterator_Next(&instIter))
        {
            if (pInst->mcInstCount)
            {
                gcmASSERT(pInst->mcInst);

                gcoOS_MemCopy((gctUINT8*)pOutSEP->pMachineCode + curPC*sizeof(VSC_MC_RAW_INST),
                              pInst->mcInst,
                              pInst->mcInstCount*sizeof(VSC_MC_RAW_INST));

                curPC += pInst->mcInstCount;
            }
        }

        if (bHasValidSubRoutine)
        {
            gcmASSERT(curPC > 1);

            pInstOfEOM = (VSC_MC_RAW_INST*)pOutSEP->pMachineCode + curPC - 1;

            /* Last of main routine must be NOP when there are other subroutines */
            gcmASSERT(pInstOfEOM->word[0] == 0 &&
                      pInstOfEOM->word[1] == 0 &&
                      pInstOfEOM->word[2] == 0 &&
                      pInstOfEOM->word[3] == 0);

            pInstOfEOM = pInstOfEOM;

            /* End PC of execution, last NOP must be excluded */
            pOutSEP->endPCOfMainRoutine = curPC - 2;
        }
        else
        {
            /* Append a NOP now */
            if (bAppendNopEOM)
            {
                gcoOS_MemCopy((gctUINT8*)pOutSEP->pMachineCode + curPC*sizeof(VSC_MC_RAW_INST),
                              &nopInst,
                              sizeof(VSC_MC_RAW_INST));

                curPC ++;
            }

            /* End PC of execution */
            pOutSEP->endPCOfMainRoutine = curPC - 1;
        }

        /* Then sub-routines */
        VIR_FuncIterator_Init(&funcIter, &pShader->functions);
        pFuncNode = VIR_FuncIterator_First(&funcIter);

        for(; pFuncNode != gcvNULL; pFuncNode = VIR_FuncIterator_Next(&funcIter))
        {
            VIR_Function *pFunc = pFuncNode->function;

            if (!(pFunc->flags & VIR_FUNCFLAG_MAIN))
            {
                VIR_InstIterator_Init(&instIter, &pFunc->instList);
                pInst = VIR_InstIterator_First(&instIter);

                for (; pInst != gcvNULL; pInst = VIR_InstIterator_Next(&instIter))
                {
                    if (pInst->mcInstCount)
                    {
                        gcmASSERT(pInst->mcInst);

                        gcoOS_MemCopy((gctUINT8*)pOutSEP->pMachineCode + curPC*sizeof(VSC_MC_RAW_INST),
                                      pInst->mcInst,
                                      pInst->mcInstCount*sizeof(VSC_MC_RAW_INST));

                        curPC += pInst->mcInstCount;
                    }
                }
            }
        }

        gcmASSERT(curPC == pOutSEP->countOfMCInst);
    }

    return VSC_ERR_NONE;
}

static SHADER_IO_USAGE _MapBuiltInNameIdToIoUsage(VIR_Symbol* pVirIoSym, gctBOOL bInputMapping, VIR_ShaderKind shKind)
{
    SHADER_IO_USAGE ioUsage;
    VIR_NameId virName = VIR_Symbol_GetName(pVirIoSym);

    /* Set default usage */
    if (shKind == VIR_SHADER_FRAGMENT && !bInputMapping)
    {
        /* Output of PS is color by default */
        ioUsage = SHADER_IO_USAGE_COLOR;
    }
    else
    {
        ioUsage = SHADER_IO_USAGE_GENERAL;
    }

    /* Get real usage */
    if (virName == VIR_NAME_POSITION ||
        virName == VIR_NAME_POSITION_W ||
        virName == VIR_NAME_IN_POSITION)
    {
        return SHADER_IO_USAGE_POSITION;
    }
    else if (virName == VIR_NAME_POINT_SIZE ||
             virName == VIR_NAME_IN_POINT_SIZE)
    {
        return SHADER_IO_USAGE_POINTSIZE;
    }
    else if (virName == VIR_NAME_COLOR)
    {
        return SHADER_IO_USAGE_COLOR;
    }
    else if (virName == VIR_NAME_FRONT_FACING)
    {
        return SHADER_IO_USAGE_ISFRONTFACE;
    }
    else if (virName == VIR_NAME_POINT_COORD ||
             (VIR_Symbol_GetFlag(pVirIoSym) & VIR_SYMFLAG_POINTSPRITE_TC))
    {
        return SHADER_IO_USAGE_POINT_COORD;
    }
    else if (virName == VIR_NAME_DEPTH)
    {
        return SHADER_IO_USAGE_DEPTH;
    }
    else if (virName == VIR_NAME_FOG_COORD)
    {
        return SHADER_IO_USAGE_FOG_COORD;
    }
    else if (virName == VIR_NAME_VERTEX_ID)
    {
        return SHADER_IO_USAGE_VERTEXID;
    }
    else if (virName == VIR_NAME_INSTANCE_ID)
    {
        return SHADER_IO_USAGE_INSTANCEID;
    }
    else if (virName == VIR_NAME_WORK_GROUP_ID)
    {
        return SHADER_IO_USAGE_THREADGROUPID;
    }
    else if (virName == VIR_NAME_LOCAL_INVOCATION_ID ||
             virName == VIR_NAME_LOCALINVOCATIONINDEX)
    {
        return SHADER_IO_USAGE_THREADIDINGROUP;
    }
    else if (virName == VIR_NAME_GLOBAL_INVOCATION_ID)
    {
        return SHADER_IO_USAGE_THREADID;
    }
    else if (virName == VIR_NAME_HELPER_INVOCATION)
    {
        return SHADER_IO_USAGE_HELPER_PIXEL;
    }
    else if (virName == VIR_NAME_SUBSAMPLE_DEPTH)
    {
        return SHADER_IO_USAGE_SAMPLE_DEPTH;
    }
    else if (virName == VIR_NAME_INVOCATION_ID)
    {
        if (shKind == VIR_SHADER_TESSELLATION_CONTROL)
        {
            return SHADER_IO_USAGE_OUTPUTCONTROLPOINTID;
        }
        else
        {
            gcmASSERT(shKind == VIR_SHADER_GEOMETRY);

            return SHADER_IO_USAGE_INSTANCING_ID;
        }
    }
    else if (virName == VIR_NAME_PRIMITIVE_ID ||
             virName == VIR_NAME_PRIMITIVE_ID_IN)
    {
        return SHADER_IO_USAGE_PRIMITIVEID;
    }
    else if (virName == VIR_NAME_TESS_LEVEL_OUTER)
    {
        return SHADER_IO_USAGE_TESSFACTOR;
    }
    else if (virName == VIR_NAME_TESS_LEVEL_INNER)
    {
        return SHADER_IO_USAGE_INSIDETESSFACTOR;
    }
    else if (virName == VIR_NAME_LAYER ||
             virName == VIR_NAME_PS_OUT_LAYER)
    {
        return SHADER_IO_USAGE_RENDERTARGETARRAYINDEX;
    }
    else if (virName == VIR_NAME_TESS_COORD)
    {
        return SHADER_IO_USAGE_DOMAIN_LOCATION;
    }
    else if (virName == VIR_NAME_PATCH_VERTICES_IN)
    {
        /* HW does not provide SGV, let's use constant reg instead, so driver
           can flush the constant when drawing */
        gcmASSERT(gcvFALSE);

        return SHADER_IO_USAGE_INPUT_VTX_CP_COUNT;
    }
    else if (virName == VIR_NAME_SAMPLE_ID)
    {
        return SHADER_IO_USAGE_SAMPLE_INDEX;
    }
    else if (virName == VIR_NAME_SAMPLE_POSITION)
    {
        return SHADER_IO_USAGE_SAMPLE_POSITION;
    }
    else if (virName == VIR_NAME_SAMPLE_MASK_IN ||
             virName == VIR_NAME_SAMPLE_MASK)
    {
        return SHADER_IO_USAGE_SAMPLE_MASK;
    }

    return ioUsage;
}

static void _FillIoRegChannel(VIR_Shader* pShader,
                              VSC_HW_CONFIG* pHwCfg,
                              VIR_Symbol* pVirIoSym,
                              gctBOOL bInputMapping,
                              SHADER_IO_MODE ioRegMode,
                              SHADER_IO_USAGE ioUsage,
                              gctUINT usageIndex,
                              gctUINT hwLoc,
                              gctUINT hwRegChannel,
                              gctUINT hwRegNoT1,
                              gctUINT hwRegChannelT1,
                              gctBOOL bStreamOut,
                              SHADER_IO_CHANNEL_MAPPING* pThisIoChannelMapping)
{
    gctBOOL bHighPrecisionOnDual16 = (VIR_Symbol_GetPrecision(pVirIoSym) == VIR_PRECISION_HIGH &&
                                      pShader->__IsDual16Shader);
    gctBOOL bInteger = ((VIR_TYFLAG_ISINTEGER & VIR_GetTypeFlag(VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(pVirIoSym)))) != 0);
    gctBOOL bNeedSpecialHwRegNo = (((ioUsage == SHADER_IO_USAGE_INSTANCEID || ioUsage == SHADER_IO_USAGE_VERTEXID) &&
                                   !pHwCfg->hwFeatureFlags.vtxInstanceIdAsAttr) ||
                                   ioUsage == SHADER_IO_USAGE_ISFRONTFACE ||
                                   ioUsage == SHADER_IO_USAGE_HELPER_PIXEL ||
                                   ioUsage == SHADER_IO_USAGE_SAMPLE_INDEX ||
                                   ioUsage == SHADER_IO_USAGE_SAMPLE_POSITION ||
                                   (ioUsage == SHADER_IO_USAGE_SAMPLE_MASK && bInputMapping));

    gcmASSERT(hwRegChannel <= CHANNEL_W);
    gcmASSERT(ioUsage < SHADER_IO_USAGE_TOTAL_COUNT);

    if (bHighPrecisionOnDual16 && !bNeedSpecialHwRegNo && (ioUsage != SHADER_IO_USAGE_SAMPLE_DEPTH))
    {
        if (hwLoc != hwRegNoT1)
        {
            gcmASSERT(hwRegChannel == hwRegChannelT1);
        }
        else
        {
            gcmASSERT(hwRegChannel != hwRegChannelT1);
        }
    }

    pThisIoChannelMapping->ioUsage = ioUsage;
    pThisIoChannelMapping->usageIndex = usageIndex;

    if (ioRegMode == SHADER_IO_MODE_ACTIVE)
    {
        pThisIoChannelMapping->hwLoc.cmnHwLoc.u.hwChannelLoc = hwLoc;
    }
    else
    {
        if (bNeedSpecialHwRegNo)
        {
            pThisIoChannelMapping->hwLoc.cmnHwLoc.u.hwRegNo = SPECIAL_HW_IO_REG_NO;
        }
        else if (ioUsage == SHADER_IO_USAGE_SAMPLE_DEPTH)
        {
            pThisIoChannelMapping->hwLoc.cmnHwLoc.u.hwRegNo = VIR_Shader_GetRegWatermark(pShader) -
                                                              ((pShader->__IsDual16Shader) ? 2 : 1);
        }
        else
        {
            pThisIoChannelMapping->hwLoc.cmnHwLoc.u.hwRegNo = hwLoc;
        }
    }

    pThisIoChannelMapping->hwLoc.cmnHwLoc.hwRegChannel = (gctUINT8)hwRegChannel;

    if (bHighPrecisionOnDual16)
    {
        gcmASSERT(ioRegMode == SHADER_IO_MODE_PASSIVE);

        if (bNeedSpecialHwRegNo)
        {
            pThisIoChannelMapping->hwLoc.t1HwLoc.hwRegNo = SPECIAL_HW_IO_REG_NO;
        }
        else if (ioUsage == SHADER_IO_USAGE_SAMPLE_DEPTH)
        {
            pThisIoChannelMapping->hwLoc.t1HwLoc.hwRegNo = VIR_Shader_GetRegWatermark(pShader) - 1;
        }
        else
        {
            pThisIoChannelMapping->hwLoc.t1HwLoc.hwRegNo = hwRegNoT1;
        }

        pThisIoChannelMapping->hwLoc.t1HwLoc.hwRegChannel = (gctUINT8)hwRegChannelT1;
    }

    pThisIoChannelMapping->flag.bActiveWithinShader = gcvTRUE;
    pThisIoChannelMapping->flag.bDeclared = gcvFALSE;
    pThisIoChannelMapping->flag.bConstInterpolate = ((VIR_SYMFLAG_FLAT & VIR_Symbol_GetFlag(pVirIoSym)) != 0);
    pThisIoChannelMapping->flag.bStreamOutToBuffer = bStreamOut;
    pThisIoChannelMapping->flag.bPerspectiveCorrection = !pThisIoChannelMapping->flag.bConstInterpolate; /* Need to refine */
    pThisIoChannelMapping->flag.bSampleFrequency = isSymSample(pVirIoSym);
    pThisIoChannelMapping->flag.bCentroidInterpolate = isSymCentroid(pVirIoSym);
    pThisIoChannelMapping->flag.bHighPrecisionOnDual16 = bHighPrecisionOnDual16;
    pThisIoChannelMapping->flag.bInteger = bInteger;
}

static gctBOOL _CheckOutputAsStreamOutput(VIR_Shader* pShader,
                                          VIR_Symbol* pVirIoSym,
                                          gctBOOL bGross,
                                          gctUINT* pVirSoSeqIndex,
                                          gctUINT* pGlobalExpandedSeqIndex,
                                          gctUINT* pLocalExpandedSeqIndex)
{
    gctUINT                    outputIdx, soOutputCount, thisSoIoRegCount;
    gctUINT                    localExpandedSeqIdx, globalExpandedSeqIndex = 0;
    VIR_Symbol*                pSoSym;
    gctBOOL                    bIsStreamOutput = gcvFALSE;

    if (pGlobalExpandedSeqIndex)
    {
        *pGlobalExpandedSeqIndex = NOT_ASSIGNED;
    }

    if (pLocalExpandedSeqIndex)
    {
        *pLocalExpandedSeqIndex = NOT_ASSIGNED;
    }

    if (pVirSoSeqIndex)
    {
        *pVirSoSeqIndex = NOT_ASSIGNED;
    }

    if (pShader->transformFeedback.varyings)
    {
        soOutputCount = VIR_IdList_Count(pShader->transformFeedback.varyings);

        if (soOutputCount > 0)
        {
            for (outputIdx = 0; outputIdx < soOutputCount; outputIdx ++)
            {
                pSoSym = VIR_Shader_GetSymFromId(pShader,
                                    VIR_IdList_GetId(pShader->transformFeedback.varyings, outputIdx));

                if (bGross)
                {
                    if (VIR_Symbol_GetIndexingInfo(pShader, pSoSym).underlyingSym == pVirIoSym)
                    {
                        bIsStreamOutput = gcvTRUE;
                        break;
                    }
                }
                else
                {
                    thisSoIoRegCount = VIR_Shader_GetXFBVaryingTempRegInfo(pShader, outputIdx)->tempRegCount;

                    for (localExpandedSeqIdx = 0; localExpandedSeqIdx < thisSoIoRegCount; localExpandedSeqIdx ++)
                    {
                        if (VIR_Symbol_GetVregIndex(pVirIoSym) == (VIR_Symbol_GetVregIndex(pSoSym) + localExpandedSeqIdx))
                        {
                            break;
                        }
                    }

                    if (localExpandedSeqIdx < thisSoIoRegCount)
                    {
                        if (pVirSoSeqIndex)
                        {
                            *pVirSoSeqIndex = outputIdx;
                        }

                        if (pGlobalExpandedSeqIndex)
                        {
                            *pGlobalExpandedSeqIndex = globalExpandedSeqIndex + localExpandedSeqIdx;
                        }

                        if (pLocalExpandedSeqIndex)
                        {
                            *pLocalExpandedSeqIndex = localExpandedSeqIdx;
                        }

                        bIsStreamOutput = gcvTRUE;
                        break;
                    }
                    else
                    {
                        globalExpandedSeqIndex += thisSoIoRegCount;
                    }
                }
            }
        }
    }

    return bIsStreamOutput;
}

static VSC_ErrCode _CollectPerExeObjIoMappingToSEP(VIR_Shader* pShader,
                                                   gctBOOL bInputMapping,
                                                   VSC_HW_CONFIG* pHwCfg,
                                                   VIR_IdList* pIoIdLsts,
                                                   SHADER_IO_MAPPING_PER_EXE_OBJ* pOutExeObjIoMapping)
{
    gctUINT                    virIoIdx, virIoCount;
    gctUINT                    compCount, channelIdx, thisUsageIdx = 0, hwRegLoc, hwLoc, hiHwLoc, hiHwRegLoc;
    gctUINT                    hwRegFirstChannel, hiHwRegFirstChannel, hwRegChannel, hiHwRegChannel;
    gctUINT                    virSoSeqIndex = 0, firstVirRegNo, globalExpandedSoSeqIdx = 0, localExpandedSoSeqIdx = 0;
    gctUINT                    maxIoIdx = NOT_ASSIGNED, thisMaxIoIdx, ioIdx, thisIoRegCount;
    gctUINT                    nonShiftIoIdx, hwRegLocShift = 0;
    VIR_Symbol*                pVirIoSym, *pVregSym;
    VIR_Type*                  pVirIoType;
    SHADER_IO_REG_MAPPING*     pThisIoRegMapping;
    SHADER_IO_CHANNEL_MAPPING* pThisIoChannelMapping;
    USAGE_2_IO*                pThisUsage2Io;
    SHADER_IO_USAGE            thisIoUsage;
    SHADER_IO_MODE             thisRegIoMode;
    SHADER_IO_MEM_ALIGN        ioMemAlign;
    gctBOOL                    bHasActiveModeIo = gcvFALSE, bVirIoStreamOutput, bIoStreamOutput;

    virIoCount = VIR_IdList_Count(pIoIdLsts);

    /* Calculate valid io reg count */
    for (virIoIdx = 0; virIoIdx < virIoCount; virIoIdx ++)
    {
        pVirIoSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pIoIdLsts, virIoIdx));

        if (!isSymUnused(pVirIoSym) && !isSymVectorizedOut(pVirIoSym))
        {
            gcmASSERT(VIR_Symbol_GetFirstSlot(pVirIoSym) != NOT_ASSIGNED);

            thisIoRegCount = VIR_Symbol_GetVirIoRegCount(pShader, pVirIoSym);
            thisMaxIoIdx = VIR_Symbol_GetFirstSlot(pVirIoSym) + thisIoRegCount;

            if (maxIoIdx == NOT_ASSIGNED)
            {
                maxIoIdx = thisMaxIoIdx;
            }
            else if (thisMaxIoIdx > maxIoIdx)
            {
                maxIoIdx = thisMaxIoIdx;
            }
        }
    }

    pOutExeObjIoMapping->countOfIoRegMapping =  (maxIoIdx == NOT_ASSIGNED) ? 0 : maxIoIdx;

    gcmASSERT(pOutExeObjIoMapping->countOfIoRegMapping <= MAX_SHADER_IO_NUM);

    /* Currently, we only support non-mempacked mode, it is an opt to support mempacked mode. We will
       do it later. See _NeedIoHwMemPackedBetweenTwoShaderStagesPerExeObj, _NeedInputHwMemPacked and
       _NeedOutputHwMemPacked in linker.*/
    ioMemAlign = SHADER_IO_MEM_ALIGN_4_CHANNELS;
    pOutExeObjIoMapping->ioMemAlign = ioMemAlign;

    /* Fill now. Note that currently we assume VIR attribute/output can not mix different usages together!!! */
    if (pOutExeObjIoMapping->countOfIoRegMapping)
    {
        /* Allocate io reg mapping of SEP */
        if (gcoOS_Allocate(gcvNULL, pOutExeObjIoMapping->countOfIoRegMapping*sizeof(SHADER_IO_REG_MAPPING),
                           (gctPOINTER*)&pOutExeObjIoMapping->pIoRegMapping) != gcvSTATUS_OK)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }

        for (ioIdx = 0; ioIdx < pOutExeObjIoMapping->countOfIoRegMapping; ioIdx ++)
        {
            vscInitializeIoRegMapping(&pOutExeObjIoMapping->pIoRegMapping[ioIdx]);
        }

        for (virIoIdx = 0; virIoIdx < virIoCount; virIoIdx ++)
        {
            pVirIoSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pIoIdLsts, virIoIdx));
            pVirIoType = VIR_Symbol_GetType(pVirIoSym);
            thisIoRegCount = VIR_Symbol_GetVirIoRegCount(pShader, pVirIoSym);
            firstVirRegNo = VIR_Symbol_GetVregIndex(pVirIoSym);
            compCount = VIR_GetTypeComponents(pVirIoType->_base);
            bVirIoStreamOutput = bInputMapping ? gcvFALSE : _CheckOutputAsStreamOutput(pShader, pVirIoSym, gcvTRUE, gcvNULL, gcvNULL, gcvNULL);

            gcmASSERT(compCount <= CHANNEL_NUM && compCount > 0);

            if (!isSymUnused(pVirIoSym) && !isSymVectorizedOut(pVirIoSym))
            {
                thisMaxIoIdx = VIR_Symbol_GetFirstSlot(pVirIoSym) + thisIoRegCount;

                for (ioIdx = VIR_Symbol_GetFirstSlot(pVirIoSym); ioIdx < thisMaxIoIdx; ioIdx ++)
                {
                    nonShiftIoIdx = ioIdx - VIR_Symbol_GetFirstSlot(pVirIoSym);

                    /* out_layer has no vregSym */
                    if (VIR_Symbol_GetName(pVirIoSym) == VIR_NAME_PS_OUT_LAYER)
                    {
                        pVregSym = pVirIoSym;
                    }
                    else
                    {
                        pVregSym = VIR_Shader_FindSymbolByTempIndex(pShader, firstVirRegNo + nonShiftIoIdx);
                    }

                    pThisIoRegMapping = &pOutExeObjIoMapping->pIoRegMapping[ioIdx];
                    thisIoUsage = _MapBuiltInNameIdToIoUsage(pVirIoSym, bInputMapping, pShader->shaderKind);
                    pThisUsage2Io = &pOutExeObjIoMapping->usage2IO[thisIoUsage];

                    thisUsageIdx = ((thisIoUsage == SHADER_IO_USAGE_INSIDETESSFACTOR) || (thisIoUsage == SHADER_IO_USAGE_TESSFACTOR)) ?
                                   nonShiftIoIdx : 0;

                    thisRegIoMode = isSymLoadStoreAttr(pVirIoSym) ? SHADER_IO_MODE_ACTIVE : SHADER_IO_MODE_PASSIVE;

                    pOutExeObjIoMapping->ioIndexMask |= (1LL << ioIdx);

                    bIoStreamOutput = gcvFALSE;
                    if (bVirIoStreamOutput)
                    {
                        bIoStreamOutput = _CheckOutputAsStreamOutput(pShader,
                                                                     pVregSym,
                                                                     gcvFALSE,
                                                                     &virSoSeqIndex,
                                                                     &globalExpandedSoSeqIdx,
                                                                     &localExpandedSoSeqIdx);

                        if (bIoStreamOutput)
                        {
                            pOutExeObjIoMapping->soIoIndexMask |= (1LL << ioIdx);

                            if (pShader->clientApiVersion != gcvAPI_D3D)
                            {
                                if (pShader->transformFeedback.bufferMode == VIR_FEEDBACK_INTERLEAVED)
                                {
                                    pThisIoRegMapping->soStreamBufferSlot = 0;
                                    pThisIoRegMapping->soSeqInStreamBuffer = globalExpandedSoSeqIdx;
                                }
                                else
                                {
                                    if (pShader->clientApiVersion == gcvAPI_OPENGL_ES11 ||
                                        pShader->clientApiVersion == gcvAPI_OPENGL_ES20 ||
                                        pShader->clientApiVersion == gcvAPI_OPENGL_ES30 ||
                                        pShader->clientApiVersion == gcvAPI_OPENGL_ES31)
                                    {
                                        gcmASSERT(virSoSeqIndex < MAX_SHADER_STREAM_OUT_BUFFER_NUM);
                                    }

                                    pThisIoRegMapping->soStreamBufferSlot = virSoSeqIndex % MAX_SHADER_STREAM_OUT_BUFFER_NUM;
                                    pThisIoRegMapping->soSeqInStreamBuffer = localExpandedSoSeqIdx;
                                }
                            }
                            else
                            {
                                /* NOT IMPLEMENTED */
                                gcmASSERT(gcvFALSE);
                            }
                        }
                    }

                    /* Fill io reg mapping */
                    pThisIoRegMapping->ioIndex = ioIdx;
                    pThisIoRegMapping->packedIoIndexMask = 0;
                    pThisIoRegMapping->regIoMode = thisRegIoMode;

                    if (thisRegIoMode == SHADER_IO_MODE_ACTIVE)
                    {
                        gcmASSERT(VIR_Symbol_GetHwFirstCompIndex(pVirIoSym) != NOT_ASSIGNED);

                        bHasActiveModeIo = gcvTRUE;
                        hwRegFirstChannel = hiHwRegFirstChannel = NOT_ASSIGNED;

                        hwRegLocShift = nonShiftIoIdx;
                        if (thisIoUsage == SHADER_IO_USAGE_TESSFACTOR ||
                            thisIoUsage == SHADER_IO_USAGE_INSIDETESSFACTOR)
                        {
                            hwRegLoc = VIR_Symbol_GetHwFirstCompIndex(pVirIoSym) + hwRegLocShift;
                        }
                        else
                        {
                            hwRegLoc = VIR_Symbol_GetHwFirstCompIndex(pVirIoSym) + hwRegLocShift * CHANNEL_NUM;
                        }

                        hiHwRegLoc = NOT_ASSIGNED;
                        pThisIoRegMapping->firstValidIoChannel = CHANNEL_X;
                    }
                    else
                    {
                        if (bInputMapping)
                        {
                            /* For dual16 shader, the input could be in 2 registers */
                            hwRegLocShift = nonShiftIoIdx * VIR_Symbol_GetRegSize(pShader, pHwCfg, pVirIoSym);

                            hwRegFirstChannel = VIR_Symbol_GetHwShift(pVirIoSym);
                            hiHwRegFirstChannel = VIR_Symbol_GetHIHwShift(pVirIoSym);
                            hwRegLoc = VIR_Symbol_GetHwRegId(pVirIoSym) + hwRegLocShift;
                            hiHwRegLoc = VIR_Symbol_GetHIHwRegId(pVirIoSym) + hwRegLocShift;
                        }
                        else
                        {
                            /* Output is coming from vreg symbol, since the array could be not consecutive */
                            gcmASSERT(pVregSym != gcvNULL);
                            hwRegFirstChannel = VIR_Symbol_GetHwShift(pVregSym);
                            hiHwRegFirstChannel = VIR_Symbol_GetHIHwShift(pVregSym);
                            hwRegLoc = VIR_Symbol_GetHwRegId(pVregSym);
                            hiHwRegLoc = VIR_Symbol_GetHIHwRegId(pVregSym);
                        }
#if IO_HW_CHANNEL_IDENTICAL_TO_IO_LL_CHANNEL
                        pThisIoRegMapping->firstValidIoChannel = hwRegFirstChannel;
#else
                        pThisIoRegMapping->firstValidIoChannel = CHANNEL_X;
#endif
                    }

                    /* Fill io reg channel mapping */
                    for (channelIdx = pThisIoRegMapping->firstValidIoChannel;
                         channelIdx < (pThisIoRegMapping->firstValidIoChannel + compCount);
                         channelIdx ++)
                    {
                        pThisIoChannelMapping = &pThisIoRegMapping->ioChannelMapping[channelIdx];

                        if (hwRegFirstChannel == NOT_ASSIGNED)
                        {
                            hwLoc = hwRegLoc + (channelIdx - pThisIoRegMapping->firstValidIoChannel);
                            hwRegChannel =  (ioMemAlign == SHADER_IO_MEM_ALIGN_4_CHANNELS) ? (hwLoc % CHANNEL_NUM) : NOT_ASSIGNED;
                            hiHwLoc = NOT_ASSIGNED; /* Active mode can not support dual16 */
                            hiHwRegChannel = hwRegChannel;
                        }
                        else
                        {
                            hwLoc = hwRegLoc;
                            hiHwLoc = hiHwRegLoc;
                            hwRegChannel = channelIdx +
#if IO_HW_CHANNEL_IDENTICAL_TO_IO_LL_CHANNEL
                                           0;
#else
                                           hwRegFirstChannel;
#endif
                            hiHwRegChannel = hiHwRegFirstChannel + (channelIdx - pThisIoRegMapping->firstValidIoChannel);
                        }

                        _FillIoRegChannel(pShader,
                                          pHwCfg,
                                          pVirIoSym,
                                          bInputMapping,
                                          thisRegIoMode,
                                          thisIoUsage,
                                          thisUsageIdx,
                                          hwLoc,
                                          hwRegChannel,
                                          hiHwLoc,
                                          hiHwRegChannel,
                                          bIoStreamOutput,
                                          pThisIoChannelMapping);

                        pThisIoRegMapping->ioChannelMask |= (1 << channelIdx);
                    }

                    /* Fill usage-2-io mapping */
                    pThisUsage2Io->ioIndexMask |= (1LL << ioIdx);
                    pThisUsage2Io->usageIndexMask = (1 << thisUsageIdx);
                    if (thisUsageIdx == 0)
                    {
                        pThisUsage2Io->mainIoChannelMask = pThisIoRegMapping->ioChannelMask;
                        pThisUsage2Io->mainFirstValidIoChannel = pThisIoRegMapping->firstValidIoChannel;
                        pThisUsage2Io->mainIoIndex = ioIdx;
                    }
                }
            }
        }

        pOutExeObjIoMapping->ioMode = bHasActiveModeIo ? SHADER_IO_MODE_ACTIVE : SHADER_IO_MODE_PASSIVE;
    }

    return VSC_ERR_NONE;
}

static VSC_ErrCode _CollectIoMappingToSEP(VSC_SEP_GEN_HELPER* pSepGenHelper,
                                          gctBOOL bInputMapping,
                                          SHADER_EXECUTABLE_PROFILE* pOutSEP)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_Shader*                pShader = pSepGenHelper->pShader;
    VIR_IdList                 workingPerVtxPxlIdLst, workingPerPrimIdLst;

    _ConvertVirPerVtxPxlAndPerPrimIoList(pShader, &pSepGenHelper->baseEpGen.pmp.mmWrapper,
                                        bInputMapping, &workingPerVtxPxlIdLst,
                                        &workingPerPrimIdLst);

    if (bInputMapping)
    {
        errCode= _CollectPerExeObjIoMappingToSEP(pShader, bInputMapping,
                                                 pSepGenHelper->baseEpGen.pHwCfg,
                                                 &workingPerVtxPxlIdLst,
                                                 &pOutSEP->inputMapping.ioVtxPxl);
        ON_ERROR(errCode, "Collect per vtx/pxl input mapping to SEP");

        errCode= _CollectPerExeObjIoMappingToSEP(pShader, bInputMapping,
                                                 pSepGenHelper->baseEpGen.pHwCfg,
                                                 &workingPerPrimIdLst,
                                                 &pOutSEP->inputMapping.ioPrim);
        ON_ERROR(errCode, "Collect per prim input mapping to SEP");
    }
    else
    {
        errCode= _CollectPerExeObjIoMappingToSEP(pShader, bInputMapping,
                                                 pSepGenHelper->baseEpGen.pHwCfg,
                                                 &workingPerVtxPxlIdLst,
                                                 &pOutSEP->outputMapping.ioVtxPxl);
        ON_ERROR(errCode, "Collect per vtx/pxl output mapping to SEP");

        errCode= _CollectPerExeObjIoMappingToSEP(pShader, bInputMapping,
                                                 pSepGenHelper->baseEpGen.pHwCfg,
                                                 &workingPerPrimIdLst,
                                                 &pOutSEP->outputMapping.ioPrim);
        ON_ERROR(errCode, "Collect per prim output mapping to SEP");
    }

OnError:
    return errCode;
}

static gctBOOL _IsUniformAllocatedOnTargetHwLocation(VIR_Uniform*                         pVirUniform,
                                                     SHADER_CONSTANT_HW_LOCATION_MAPPING* pTargetHwLocation)
{
    if (pVirUniform->physical != -1 &&
        pTargetHwLocation->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER)
    {
        return (pTargetHwLocation->hwLoc.hwRegNo == (gctUINT)pVirUniform->physical);
    }

    /* Do not support mapping to mem */
    gcmASSERT(gcvFALSE);

    return gcvFALSE;
}

static void _SetUniformTargetHwLocation(VIR_Uniform*                         pVirUniform,
                                        SHADER_CONSTANT_HW_LOCATION_MAPPING* pTargetHwLocation)
{
    if (pVirUniform->physical != -1)
    {
        pTargetHwLocation->hwLoc.hwRegNo = pVirUniform->physical;
        pTargetHwLocation->hwAccessMode = SHADER_HW_ACCESS_MODE_REGISTER;

        return;
    }

    /* Do not support mapping to mem */
    gcmASSERT(gcvFALSE);
}

SHADER_COMPILE_TIME_CONSTANT* _EnlargeCTCRoom(SHADER_CONSTANT_MAPPING* pCnstMapping, gctUINT enlargeCTCCount)
{
    void*                         pOldCTCSet;
    gctUINT                       oldCTCCount, newCTCCount;

    pOldCTCSet = pCnstMapping->pCompileTimeConstant;
    oldCTCCount = pCnstMapping->countOfCompileTimeConstant;
    newCTCCount = oldCTCCount + enlargeCTCCount;
    gcoOS_Allocate(gcvNULL,
                   sizeof(SHADER_COMPILE_TIME_CONSTANT) * newCTCCount,
                   (gctPOINTER*)&pCnstMapping->pCompileTimeConstant);

    if (pOldCTCSet)
    {
        memcpy(pCnstMapping->pCompileTimeConstant, pOldCTCSet,
               oldCTCCount*sizeof(SHADER_COMPILE_TIME_CONSTANT));

        gcoOS_Free(gcvNULL, pOldCTCSet);
    }

    pCnstMapping->countOfCompileTimeConstant = newCTCCount;

    /* Return first enlarged CTC */
    return &pCnstMapping->pCompileTimeConstant[oldCTCCount];
}

static VSC_ErrCode _CollectConstantMappingToSEP(VSC_SEP_GEN_HELPER* pSepGenHelper, SHADER_EXECUTABLE_PROFILE* pOutSEP)
{
    VIR_Shader*                   pShader = pSepGenHelper->pShader;
    VIR_UniformIdList*            pVirUniformLsts = VIR_Shader_GetUniforms(pShader);
    VIR_Symbol*                   pVirUniformSym;
    VIR_Uniform*                  pVirUniform;
    VIR_Type*                     pVirUniformType;
    gctUINT                       virUniformIdx, ctcSlot;
    gctUINT                       channel, hwChannel;
    gctUINT                       thisUniformRegCount;
    VIR_Const*                    pThisVirCTCVal;
    SHADER_CONSTANT_MAPPING*      pCnstMapping = &pOutSEP->constantMapping;
    SHADER_COMPILE_TIME_CONSTANT* pThisCTC;
    gctUINT                       thisChannelValue;

    for (virUniformIdx = 0; virUniformIdx < VIR_IdList_Count(pVirUniformLsts); virUniformIdx ++)
    {
        pVirUniformSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pVirUniformLsts, virUniformIdx));

        if (!isSymUniformUsedInShader(pVirUniformSym))
        {
            continue;
        }

        if (isSymUniformMovedToAUBO(pVirUniformSym))
        {
            continue;
        }

        pVirUniform = gcvNULL;
        if (VIR_Symbol_isUniform(pVirUniformSym))
        {
            pVirUniform = VIR_Symbol_GetUniform(pVirUniformSym);
        }
        else if (VIR_Symbol_isImage(pVirUniformSym))
        {
            pVirUniform = VIR_Symbol_GetImage(pVirUniformSym);
        }

        /* Don't collect it for some circumstances */
        if (pVirUniform && pVirUniform->physical == -1 &&
             /* Only affiliated with LTC */
            (isSymUniformUsedInLTC(pVirUniformSym) ||
             /* We use same uniform to dedicate ubo and ubo address, but it is possible that ubo is used but ubo address is not used */
             VIR_Symbol_GetUniformKind(pVirUniformSym) == VIR_UNIFORM_UNIFORM_BLOCK_ADDRESS))
        {
            continue;
        }

        if (pVirUniform)
        {
            gcmASSERT(pVirUniform->physical != -1);

            pVirUniformType = VIR_Symbol_GetType(pVirUniformSym);
            thisUniformRegCount = VIR_Type_GetVirRegCount(pShader, pVirUniformType);

            /* HW constant register count */
            pCnstMapping->hwConstRegCount = vscMAX(pCnstMapping->hwConstRegCount,
                                                   (pVirUniform->physical + thisUniformRegCount));

            /* Collect CTC */
            if (isSymUniformCompiletimeInitialized(pVirUniformSym))
            {
                gcmASSERT(thisUniformRegCount == 1);

                pThisVirCTCVal = (VIR_Const*)VIR_GetSymFromId(&pShader->constTable, pVirUniform->u.initializer);

                pThisCTC = gcvNULL;

                /* Check whether there is already a CTC allocated on same HW location */
                for (ctcSlot = 0; ctcSlot < pCnstMapping->countOfCompileTimeConstant; ctcSlot ++)
                {
                    if (_IsUniformAllocatedOnTargetHwLocation(pVirUniform,
                                             &pCnstMapping->pCompileTimeConstant[ctcSlot].hwConstantLocation))
                    {
                        pThisCTC = &pCnstMapping->pCompileTimeConstant[ctcSlot];
                        break;
                    }
                }

                /* Enlarge CTC set to make room for current CTC */
                if (pThisCTC == gcvNULL)
                {
                    pThisCTC = _EnlargeCTCRoom(pCnstMapping, 1);
                    vscInitializeCTC(pThisCTC);
                    _SetUniformTargetHwLocation(pVirUniform, &pThisCTC->hwConstantLocation);
                }

                /* Copy CTC now */
                for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
                {
                    if (channel >= (gctUINT)(VIR_GetTypeComponents(pThisVirCTCVal->type)))
                    {
                        break;
                    }

                    hwChannel = (((pVirUniform->swizzle) >> ((channel) * 2)) & 0x3);
                    thisChannelValue = *(gctUINT*)&pThisVirCTCVal->value.vecVal.u32Value[channel];

                    if (pThisCTC->hwConstantLocation.validHWChannelMask & (1 << hwChannel))
                    {
                        gcmASSERT(thisChannelValue == pThisCTC->constantValue[hwChannel]);
                    }
                    else
                    {
                        pThisCTC->hwConstantLocation.validHWChannelMask |= (1 << hwChannel);
                        pThisCTC->constantValue[hwChannel] = thisChannelValue;
                    }
                }
            }
        }
    }

    return VSC_ERR_NONE;
}

static VSC_ErrCode _CollectSamplerMappingToSEP(VSC_SEP_GEN_HELPER* pSepGenHelper, SHADER_EXECUTABLE_PROFILE* pOutSEP)
{
    VSC_ErrCode                   errCode = VSC_ERR_NONE;
    VIR_Shader*                   pShader = pSepGenHelper->pShader;
    SHADER_SAMPLER_MAPPING*       pSamplerMapping = &pOutSEP->samplerMapping;
    gctINT                        samplerCount = 0;

    errCode = VIR_Shader_CalcSamplerCount(pShader, &samplerCount);
    ON_ERROR(errCode, "Calc sampler error");

    pSamplerMapping->hwSamplerRegCount = (gctUINT)samplerCount;

OnError:
    return errCode;
}

static void _MapTesLayoutToIoNativeTessHints(VIR_TESLayout* pVirTesLayout,
                                             SHADER_TESSELLATOR_DOMAIN_TYPE* pTessDomainType,
                                             SHADER_TESSELLATOR_PARTITION_TYPE* pTessPartitionType,
                                             SHADER_TESSELLATOR_OUTPUT_PRIMITIVE_TOPOLOGY* pTessOutputPrim)
{
    VIR_TessOutputPrimitive virTessOutputPrim;

    /* Domain type */
    if (pVirTesLayout->tessPrimitiveMode == VIR_TESS_PMODE_TRIANGLE)
    {
        *pTessDomainType = SHADER_TESSELLATOR_DOMAIN_TRIANGLE;
    }
    else if (pVirTesLayout->tessPrimitiveMode == VIR_TESS_PMODE_QUAD)
    {
        *pTessDomainType = SHADER_TESSELLATOR_DOMAIN_QUAD;
    }
    else if (pVirTesLayout->tessPrimitiveMode == VIR_TESS_PMODE_ISOLINE)
    {
        *pTessDomainType = SHADER_TESSELLATOR_DOMAIN_ISOLINE;
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    /* Partition type */
    if (pVirTesLayout->tessVertexSpacing == VIR_TESS_SPACING_EQUAL)
    {
        *pTessPartitionType = SHADER_TESSELLATOR_PARTITION_INTEGER;
    }
    else if (pVirTesLayout->tessVertexSpacing == VIR_TESS_SPACING_EVEN)
    {
        *pTessPartitionType = SHADER_TESSELLATOR_PARTITION_FRACTIONAL_EVEN;
    }
    else if (pVirTesLayout->tessVertexSpacing == VIR_TESS_SPACING_ODD)
    {
        *pTessPartitionType = SHADER_TESSELLATOR_PARTITION_FRACTIONAL_ODD;
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    /* Output prim type */
    virTessOutputPrim = VIR_ConvertTESLayoutToOutputPrimitive(pVirTesLayout);
    if (virTessOutputPrim == VIR_TESS_OUTPUT_PRIM_POINT)
    {
        *pTessOutputPrim = SHADER_TESSELLATOR_OUTPUT_PRIM_POINT;
    }
    else if (virTessOutputPrim == VIR_TESS_OUTPUT_PRIM_LINE)
    {
        *pTessOutputPrim = SHADER_TESSELLATOR_OUTPUT_PRIM_LINE;
    }
    else if (virTessOutputPrim == VIR_TESS_OUTPUT_PRIM_TRIANGLE_CW)
    {
        *pTessOutputPrim = SHADER_TESSELLATOR_OUTPUT_PRIM_TRIANGLE_CW;
    }
    else if (virTessOutputPrim == VIR_TESS_OUTPUT_PRIM_TRIANGLE_CCW)
    {
        *pTessOutputPrim = SHADER_TESSELLATOR_OUTPUT_PRIM_TRIANGLE_CCW;
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }
}

static void _MapGeoLayoutToIoNativeGsHints(VIR_GEOLayout* pVirGeoLayout,
                                           SHADER_GS_INPUT_PRIMITIVE_TOPOLOGY* pGsInputPrim,
                                           gctUINT* pInputVtxCount,
                                           SHADER_GS_OUTPUT_PRIMITIVE_TOPOLOGY* pGsOutputPrim)
{
    /* Input prim type */
    if (pVirGeoLayout->geoInPrimitive == VIR_GEO_POINTS)
    {
        *pGsInputPrim = SHADER_GS_INPUT_PRIMITIVE_POINT;
        *pInputVtxCount = 1;
    }
    else if (pVirGeoLayout->geoInPrimitive == VIR_GEO_LINES)
    {
        *pGsInputPrim = SHADER_GS_INPUT_PRIMITIVE_LINE;
        *pInputVtxCount = 2;
    }
    else if (pVirGeoLayout->geoInPrimitive == VIR_GEO_LINES_ADJACENCY)
    {
        *pGsInputPrim = SHADER_GS_INPUT_PRIMITIVE_LINE_ADJ;
        *pInputVtxCount = 4;
    }
    else if (pVirGeoLayout->geoInPrimitive == VIR_GEO_TRIANGLES)
    {
        *pGsInputPrim = SHADER_GS_INPUT_PRIMITIVE_TRIANGLE;
        *pInputVtxCount = 3;
    }
    else if (pVirGeoLayout->geoInPrimitive == VIR_GEO_TRIANGLES_ADJACENCY)
    {
        *pGsInputPrim = SHADER_GS_INPUT_PRIMITIVE_TRIANGLE_ADJ;
        *pInputVtxCount = 6;
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    /* Output prim type */
    if (pVirGeoLayout->geoOutPrimitive == VIR_GEO_POINTS)
    {
        *pGsOutputPrim = SHADER_GS_OUTPUT_PRIMITIVE_POINTLIST;
    }
    else if (pVirGeoLayout->geoOutPrimitive == VIR_GEO_LINE_STRIP)
    {
        *pGsOutputPrim = SHADER_GS_OUTPUT_PRIMITIVE_LINESTRIP;
    }
    else if (pVirGeoLayout->geoOutPrimitive == VIR_GEO_TRIANGLE_STRIP)
    {
        *pGsOutputPrim = SHADER_GS_OUTPUT_PRIMITIVE_TRIANGLESTRIP;
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }
}

static void _CollectExeHints(VSC_SEP_GEN_HELPER* pSepGenHelper, SHADER_EXECUTABLE_PROFILE* pOutSEP)
{
    VIR_Shader*                 pShader = pSepGenHelper->pShader;
    UNIFIED_RF_ALLOC_STRATEGY   samplerAllocStrategy = UNIFIED_RF_ALLOC_STRATEGY_UNIFIED;
    gctUINT                     i;

    /* 1. Native hints */

    pOutSEP->exeHints.nativeHints.globalStates.bSeparatedShader = (pShader->clientApiVersion == gcvAPI_D3D) ?
                                                                           gcvTRUE : VIR_Shader_IsSeparated(pShader);

    if (pShader->shaderKind == VIR_SHADER_FRAGMENT)
    {
        pOutSEP->exeHints.nativeHints.prvStates.ps.bEarlyPixelTestInRa = pShader->useEarlyFragTest;
    }

    if (pShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL)
    {
        /* Currently, for OGL driver (DX has no this issue), it will always trigger recompiling
           when compiler internally analyzed input count (a) is different with count (b) set by
           GL API. This is BAD because recompiling is very time-consuming and a perf killer. We
           should only trigger recompiling when (a > b). But if so, we need remove USC and other
           throttle registers programmming out of VSC */
        pOutSEP->exeHints.nativeHints.prvStates.ts.inputCtrlPointCount =
                                                  pShader->shaderLayout.tcs.tcsPatchInputVertices;

        pOutSEP->exeHints.nativeHints.prvStates.ts.outputCtrlPointCount =
                                                  pShader->shaderLayout.tcs.tcsPatchOutputVertices;
    }

    if (pShader->shaderKind == VIR_SHADER_TESSELLATION_EVALUATION)
    {
        pOutSEP->exeHints.nativeHints.prvStates.ts.inputCtrlPointCount =
                                                  pShader->shaderLayout.tes.tessPatchInputVertices;
    }

    if (pShader->clientApiVersion != gcvAPI_D3D)
    {
        if (pShader->shaderKind == VIR_SHADER_TESSELLATION_EVALUATION)
        {
            _MapTesLayoutToIoNativeTessHints(&pShader->shaderLayout.tes,
                                             &pOutSEP->exeHints.nativeHints.prvStates.ts.tessDomainType,
                                             &pOutSEP->exeHints.nativeHints.prvStates.ts.tessPartitionType,
                                             &pOutSEP->exeHints.nativeHints.prvStates.ts.tessOutputPrim);

            pOutSEP->exeHints.nativeHints.prvStates.ts.maxTessFactor = 64;
        }
    }
    else
    {
        if (pShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL)
        {
            /* NOT IMPLEMENTED */
            gcmASSERT(gcvFALSE);
        }
    }

    if (pShader->shaderKind == VIR_SHADER_GEOMETRY)
    {
        pOutSEP->exeHints.nativeHints.prvStates.gs.instanceCount = pShader->shaderLayout.geo.geoInvocations;
        pOutSEP->exeHints.nativeHints.prvStates.gs.maxOutputVtxCount = pShader->shaderLayout.geo.geoMaxVertices;

        _MapGeoLayoutToIoNativeGsHints(&pShader->shaderLayout.geo,
                                       &pOutSEP->exeHints.nativeHints.prvStates.gs.inputPrim,
                                       &pOutSEP->exeHints.nativeHints.prvStates.gs.inputVtxCount,
                                       &pOutSEP->exeHints.nativeHints.prvStates.gs.outputPrim);
    }

    /* 2. Derived hints */

    if (VIR_Shader_isPackUnifiedSampler(pShader))
    {
        samplerAllocStrategy = UNIFIED_RF_ALLOC_STRATEGY_FLOAT_ADDR_OFFSET;
    }
    else
    {
        samplerAllocStrategy = UNIFIED_RF_ALLOC_STRATEGY_FIXED_ADDR_OFFSET;
    }

    pOutSEP->exeHints.derivedHints.globalStates.bExecuteOnDual16 = pShader->__IsDual16Shader;
    pOutSEP->exeHints.derivedHints.globalStates.unifiedConstRegAllocStrategy = UNIFIED_RF_ALLOC_STRATEGY_FLOAT_ADDR_OFFSET;
    pOutSEP->exeHints.derivedHints.globalStates.unifiedSamplerRegAllocStrategy = samplerAllocStrategy;
    pOutSEP->exeHints.derivedHints.globalStates.bGprSpilled = pShader->hasRegisterSpill;
    pOutSEP->exeHints.derivedHints.globalStates.bLoadStore = pShader->hasLoadStore;
    pOutSEP->exeHints.derivedHints.globalStates.bImgReadWrite = pShader->hasImgReadWrite;
    pOutSEP->exeHints.derivedHints.globalStates.bAtomicOp = pShader->hasAtomicOp;
    pOutSEP->exeHints.derivedHints.globalStates.bMemAccess = pShader->hasMemoryAccess;
    pOutSEP->exeHints.derivedHints.globalStates.bBarrierOp = pShader->hasBarrier;
    pOutSEP->exeHints.derivedHints.globalStates.hwStartRegNoForUSCAddrs = pShader->remapRegStart;
    pOutSEP->exeHints.derivedHints.globalStates.hwStartRegChannelForUSCAddrs = pShader->remapChannelStart;
    pOutSEP->exeHints.derivedHints.globalStates.bIoUSCAddrsPackedToOneReg = (pShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL) ?
                                                                             VIR_Shader_TCS_UsePackedRemap(pShader) : gcvFALSE;

    pOutSEP->exeHints.derivedHints.prePaStates.bOutPosZDepW = pShader->vsPositionZDependsOnW;

    if (pShader->shaderKind == VIR_SHADER_GEOMETRY)
    {
        pOutSEP->exeHints.derivedHints.prvStates.gs.bHasPrimRestartOp = VIR_Shader_GS_HasRestartOp(pShader);
    }

    if (pShader->shaderKind == VIR_SHADER_FRAGMENT)
    {
        pOutSEP->exeHints.derivedHints.prvStates.ps.bPxlDiscard = pShader->psHasDiscard;
        pOutSEP->exeHints.derivedHints.prvStates.ps.bDerivRTy = pShader->hasDsy;
        pOutSEP->exeHints.derivedHints.prvStates.ps.hwRegNoForSampleMaskId = pShader->sampleMaskIdRegStart;
        pOutSEP->exeHints.derivedHints.prvStates.ps.hwRegChannelForSampleMaskId = pShader->sampleMaskIdChannelStart;
        pOutSEP->exeHints.derivedHints.prvStates.ps.bExecuteOnSampleFreq = VIR_Shader_PS_RunOnSampleShading(pShader);

        for (i = CHANNEL_X; i < CHANNEL_NUM; i ++)
        {
            if (pShader->psInputPosCompValid[i])
            {
                pOutSEP->exeHints.derivedHints.prvStates.ps.inputPosChannelValid |= (1 << i);
            }

            if (i < CHANNEL_Z && pShader->psInputPCCompValid[i])
            {
                pOutSEP->exeHints.derivedHints.prvStates.ps.inputPntCoordChannelValid |= (1 << i);
            }
        }
    }

    if (pShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL)
    {
        pOutSEP->exeHints.derivedHints.prvStates.hs.bPerCpOutputsUsedByOtherThreads = pShader->shaderLayout.tcs.hasOutputVertexAccess;
    }
}

static VSC_ErrCode _CollectStaticPatchesInternal(VSC_SEP_GEN_HELPER* pSepGenHelper,
                                                 SHADER_EXECUTABLE_PROFILE* pOutSEP,
                                                 gctUINT* pExtraMemPatchCount,
                                                 gctUINT* pConstantPatchCount)
{
    VIR_Shader*                   pShader = pSepGenHelper->pShader;
    gctINT                        extraMemPatchIdx = -1, constantPatchIdx = -1;
    SSP_GPR_SPILL_PRIV*           pGprSpillPriv;
    VIR_UniformIdList*            pVirUniformLsts = VIR_Shader_GetUniforms(pShader);
    VIR_Symbol*                   pVirUniformSym;
    VIR_Uniform*                  pVirUniform;
    gctUINT                       virUniformIdx;

    if (pShader->hasRegisterSpill)
    {
        gcmASSERT(pShader->vidmemSizeOfSpill > 0);

        /* Move to next patch index */
        extraMemPatchIdx ++;
        constantPatchIdx ++;

        /* TODO: Need move spillBuffer and subCBMapping filling into to uav and constant mapping collection, and only
                 refer pointer from those mapping tables, see commented out pointer member in SSP_GPR_SPILL_PRIV and
                 SHADER_PATCH_CONSTANT_ENTRY */
        if (pOutSEP)
        {
            /* Spill memory description */
            pOutSEP->staticPatchExtraMems.pPatchCommonEntries[extraMemPatchIdx].patchFlag = SSP_COMMON_FLAG_GPR_SPILL_MEMORY;
            pOutSEP->staticPatchExtraMems.pPatchCommonEntries[extraMemPatchIdx].patchFlagIndex = 0;

            if (gcoOS_Allocate(gcvNULL, sizeof(SSP_GPR_SPILL_PRIV),
                               (gctPOINTER*)&pOutSEP->staticPatchExtraMems.
                                             pPatchCommonEntries[extraMemPatchIdx].pPrivateData) != gcvSTATUS_OK)
            {
                return VSC_ERR_OUT_OF_MEMORY;
            }

            pGprSpillPriv = (SSP_GPR_SPILL_PRIV*)pOutSEP->staticPatchExtraMems.
                                                 pPatchCommonEntries[extraMemPatchIdx].pPrivateData;

            pGprSpillPriv->spillBuffer.accessMode = SHADER_UAV_ACCESS_MODE_RAW;
            pGprSpillPriv->spillBuffer.flattenedUavSize = pShader->vidmemSizeOfSpill;
            pGprSpillPriv->spillBuffer.u.s.uavDimension = SHADER_UAV_DIMENSION_BUFFER;

            /* Spill memory address */
            pOutSEP->staticPatchConstants.pPatchConstantEntries[constantPatchIdx].patchFlag = SSP_CONSTANT_FLAG_GPR_SPILL_MEM_ADDR;
            pOutSEP->staticPatchConstants.pPatchConstantEntries[constantPatchIdx].patchFlagIndex = 0;
            pOutSEP->staticPatchConstants.pPatchConstantEntries[constantPatchIdx].mode = SHADER_PATCH_CONSTANT_MODE_VAL_2_MEMORREG;
            pOutSEP->staticPatchConstants.pPatchConstantEntries[constantPatchIdx].u.subCBMapping.hwFirstConstantLocation.
                                                                                      hwAccessMode = SHADER_HW_ACCESS_MODE_REGISTER;

            for (virUniformIdx = 0; virUniformIdx < VIR_IdList_Count(pVirUniformLsts); virUniformIdx ++)
            {
                pVirUniformSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pVirUniformLsts, virUniformIdx));
                if (VIR_Symbol_GetUniformKind(pVirUniformSym) == VIR_UNIFORM_TEMP_REG_SPILL_MEM_ADDRESS)
                {
                    gcmASSERT(VIR_Symbol_isUniform(pVirUniformSym));

                    pVirUniform = VIR_Symbol_GetUniform(pVirUniformSym);
                    gcmASSERT(pVirUniform->physical != -1);

                    pOutSEP->staticPatchConstants.pPatchConstantEntries[constantPatchIdx].u.subCBMapping.hwFirstConstantLocation.
                                                                                      hwLoc.hwRegNo = pVirUniform->physical;
                    pOutSEP->staticPatchConstants.pPatchConstantEntries[constantPatchIdx].u.subCBMapping.hwFirstConstantLocation.
                                                                                      validHWChannelMask = (1 << (pVirUniform->swizzle & 0x3));

                    break;
                }
            }
        }
    }

    if (pExtraMemPatchCount)
    {
        *pExtraMemPatchCount = extraMemPatchIdx + 1;
    }

    if (pConstantPatchCount)
    {
        *pConstantPatchCount = constantPatchIdx + 1;
    }

    return VSC_ERR_NONE;
}

static VSC_ErrCode _CollectStaticPatches(VSC_SEP_GEN_HELPER* pSepGenHelper, SHADER_EXECUTABLE_PROFILE* pOutSEP)
{
    VSC_ErrCode      errCode = VSC_ERR_NONE;
    gctUINT          extraMemPatchCount = 0, constantPatchCount = 0;

    /* Determine how many static patches we collect */
    errCode = _CollectStaticPatchesInternal(pSepGenHelper, gcvNULL, &extraMemPatchCount, &constantPatchCount);
    ON_ERROR(errCode, "Determine patch count");

    pOutSEP->staticPatchExtraMems.countOfEntries = extraMemPatchCount;
    pOutSEP->staticPatchConstants.countOfEntries = constantPatchCount;

    /* Allocate patch entries */
    if (extraMemPatchCount)
    {
        if (gcoOS_Allocate(gcvNULL, extraMemPatchCount * sizeof(SHADER_PATCH_COMMON_ENTRY),
                           (gctPOINTER*)&pOutSEP->staticPatchExtraMems.pPatchCommonEntries) != gcvSTATUS_OK)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }

        gcoOS_ZeroMemory(pOutSEP->staticPatchExtraMems.pPatchCommonEntries,
                         extraMemPatchCount * sizeof(SHADER_PATCH_COMMON_ENTRY));
    }

    if (constantPatchCount)
    {
        if (gcoOS_Allocate(gcvNULL, constantPatchCount * sizeof(SHADER_PATCH_CONSTANT_ENTRY),
                           (gctPOINTER*)&pOutSEP->staticPatchConstants.pPatchConstantEntries) != gcvSTATUS_OK)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }

        gcoOS_ZeroMemory(pOutSEP->staticPatchConstants.pPatchConstantEntries,
                         constantPatchCount * sizeof(SHADER_PATCH_CONSTANT_ENTRY));
    }

    /* 3. Now do real patch collection */
    if (extraMemPatchCount + constantPatchCount > 0)
    {
        errCode = _CollectStaticPatchesInternal(pSepGenHelper, pOutSEP, gcvNULL, gcvNULL);
        ON_ERROR(errCode, "Collect real patch collection");
    }

OnError:
    return errCode;
}

void _PrintMachineCode(VSC_MC_RAW_INST* pMcCode, gctUINT countOfMCInst, gctBOOL bExecuteOnDual16, VSC_DUMPER* pDumper)
{
    gctUINT  instIdx;
    gctCHAR  buffer[256];

    if (pMcCode && countOfMCInst > 0)
    {
        vscDumper_PrintStrSafe(pDumper, "[code]");
        vscDumper_DumpBuffer(pDumper);

        for (instIdx = 0; instIdx < countOfMCInst; instIdx ++)
        {
            gcDumpMCStates(instIdx,
                           (gctUINT32*)(pMcCode + instIdx),
                           gcvTRUE,
                           gcvTRUE,
                           bExecuteOnDual16,
                           sizeof(buffer),
                           buffer
                           );

            vscDumper_PrintStrSafe(pDumper, "%s", buffer);
            vscDumper_DumpBuffer(pDumper);
        }
    }
}

void _PrintIoMappingPerExeObj(SHADER_IO_MAPPING_PER_EXE_OBJ* pIoMappingPerExeObj, gctBOOL bInput,
                              gctBOOL bPerPrim, VIR_Shader* pShader, VSC_DUMPER* pDumper)
{
    gctINT          ioIndex, ui, channel, i;
    gctUINT         hwLoc, hwLocT1;
    gctUINT         usageIndex;
    SHADER_IO_USAGE thisUsage;
    gctUINT         writeMask, writeMaskT1 = 0;
    gctBOOL         bHpRegOnDual16;
    gctUINT         usageIndexMask[SHADER_IO_USAGE_TOTAL_COUNT];

    gctCONST_STRING strUsageName[] =
    {
        "position",
        "blendweight",
        "blendindices",
        "normal",
        "psize",
        "texcoord",
        "tagent",
        "binormal",
        "tessfactor",
        "positiont",
        "color",
        "fog",
        "depth",
        "sample",
        "vertexid",
        "primitiveid",
        "instanceid",
        "inputcoverage",
        "isfrontface",
        "sampleindex",
        "outputcontrolpointid",
        "forkinstanceid",
        "joininstanceid",
        "domain",
        "threadid",
        "threadgroupid",
        "threadidingroup",
        "threadidingroupflattened",
        "clipdistance",
        "culldistance",
        "rendertargetarrayindex",
        "viewportarrayindex",
        "depthgreaterequal",
        "depthlessequal",
        "insidetessfactor",
        "samplemask",
        "pointcoord",
        "fogcoord",
        "helperpixel",
        "sampledepth",
        "samplepos",
        "inputvtxcpcount",
        "instancingid"
        "",
        "?*&%#"
    };

    gctCONST_STRING strWriteMask[] =
    {
        ".", ".x", ".y", ".xy",
        ".z", ".xz", ".yz", ".xyz",
        ".w", ".xw", ".yw", ".xyw",
        ".zw", ".xzw", ".yzw", ".xyzw"
    };

#define IO_DUMP_ALIGNEMENT  11

    for (ioIndex = 0; ioIndex < MAX_SHADER_IO_NUM; ioIndex ++)
    {
        if (!(pIoMappingPerExeObj->ioIndexMask & (1LL << ioIndex)))
        {
            continue;
        }

        for (ui = 0; ui < SHADER_IO_USAGE_TOTAL_COUNT; ui ++)
        {
            if (pIoMappingPerExeObj->usage2IO[ui].ioIndexMask & (1LL << ioIndex))
            {
                memset(usageIndexMask, 0, sizeof(gctUINT)*SHADER_IO_USAGE_TOTAL_COUNT);
                thisUsage = (SHADER_IO_USAGE)ui;

                for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
                {
                    if (pIoMappingPerExeObj->pIoRegMapping[ioIndex].ioChannelMapping[channel].ioUsage == thisUsage &&
                        pIoMappingPerExeObj->pIoRegMapping[ioIndex].ioChannelMapping[channel].flag.bActiveWithinShader)
                    {
                        usageIndex = pIoMappingPerExeObj->pIoRegMapping[ioIndex].ioChannelMapping[channel].usageIndex;
                        if (usageIndexMask[thisUsage] & (1 << usageIndex))
                        {
                            continue;
                        }

                        bHpRegOnDual16 = gcvFALSE;
                        hwLoc = pIoMappingPerExeObj->pIoRegMapping[ioIndex].ioChannelMapping[channel].hwLoc.cmnHwLoc.u.hwRegNo;
                        hwLocT1 = pIoMappingPerExeObj->pIoRegMapping[ioIndex].ioChannelMapping[channel].hwLoc.t1HwLoc.hwRegNo;
                        usageIndexMask[thisUsage] |= (1 << usageIndex);
                        writeMask = (1 << channel);
                        if (pIoMappingPerExeObj->pIoRegMapping[ioIndex].ioChannelMapping[channel].flag.bHighPrecisionOnDual16)
                        {
                            bHpRegOnDual16 = gcvTRUE;

                            if (hwLoc == hwLocT1)
                            {
                                writeMaskT1 = (1 << pIoMappingPerExeObj->pIoRegMapping[ioIndex].
                                                    ioChannelMapping[channel].hwLoc.t1HwLoc.hwRegChannel);
                            }
                            else
                            {
                                writeMaskT1 = writeMask;
                            }
                        }

                        for (i = channel + 1; i < CHANNEL_NUM; i ++)
                        {
                            if (pIoMappingPerExeObj->pIoRegMapping[ioIndex].ioChannelMapping[i].flag.bActiveWithinShader)
                            {
                                if ((pIoMappingPerExeObj->pIoRegMapping[ioIndex].ioChannelMapping[i].ioUsage == thisUsage) &&
                                    (usageIndex == pIoMappingPerExeObj->pIoRegMapping[ioIndex].ioChannelMapping[i].usageIndex))
                                {
                                    writeMask |= (1 << i);

                                    if (bHpRegOnDual16)
                                    {
                                        gcmASSERT(pIoMappingPerExeObj->pIoRegMapping[ioIndex].ioChannelMapping[i].flag.bHighPrecisionOnDual16);

                                        if (hwLoc == hwLocT1)
                                        {
                                            writeMaskT1 |= (1 << pIoMappingPerExeObj->pIoRegMapping[ioIndex].
                                                                 ioChannelMapping[i].hwLoc.t1HwLoc.hwRegChannel);
                                        }
                                        else
                                        {
                                            writeMaskT1 = writeMask;
                                        }
                                    }
                                }
                            }
                        }

                        if (thisUsage == SHADER_IO_USAGE_ISFRONTFACE)
                        {
                            gcmASSERT(bInput);

                            vscDumper_PrintStrSafe(pDumper, "i%d%s", ioIndex, strWriteMask[writeMask]);

                            for (i = pDumper->curOffset; i < IO_DUMP_ALIGNEMENT; i ++)
                            {
                                vscDumper_PrintStrSafe(pDumper, " ");
                            }

                            vscDumper_PrintStrSafe(pDumper, "------>    vface\n");
                        }
                        else
                        {
                            /* pi --- per-prim input
                               po --- per-prim output */
                            vscDumper_PrintStrSafe(pDumper,
                                                   bInput ? (bPerPrim ? "pi%d%s" : "i%d%s") : (bPerPrim ? "po%d%s" : "o%d%s"),
                                                   ioIndex, strWriteMask[writeMask]);

                            for (i = pDumper->curOffset; i < IO_DUMP_ALIGNEMENT; i ++)
                            {
                                vscDumper_PrintStrSafe(pDumper, " ");
                            }

                            if (hwLoc == SPECIAL_HW_IO_REG_NO)
                            {
                                vscDumper_PrintStrSafe(pDumper, "------>    specialHwReg");
                            }
                            else
                            {
                                /* pci --- per-prim channel index
                                   vci --- per-vertex channel index */
                                if (bHpRegOnDual16)
                                {
                                    vscDumper_PrintStrSafe(pDumper, "------>    r%d%s/r%d%s",
                                                           hwLoc, strWriteMask[writeMask],
                                                           hwLocT1, strWriteMask[writeMaskT1]);
                                }
                                else
                                {
                                    vscDumper_PrintStrSafe(pDumper,
                                                           pIoMappingPerExeObj->pIoRegMapping[ioIndex].regIoMode == SHADER_IO_MODE_ACTIVE ?
                                                           (bPerPrim ? "------>    pci%d" : "------>    vci%d") : "------>    r%d%s",
                                                           hwLoc, strWriteMask[writeMask]);
                                }
                            }

                            if (thisUsage != SHADER_IO_USAGE_GENERAL)
                            {
                                if (usageIndex == 0)
                                {
                                    vscDumper_PrintStrSafe(pDumper, " (%s", strUsageName[thisUsage]);
                                }
                                else
                                {
                                    vscDumper_PrintStrSafe(pDumper, " (%s%d", strUsageName[thisUsage], usageIndex);
                                }

                                if (pIoMappingPerExeObj->soIoIndexMask & (1LL << ioIndex))
                                {
                                    vscDumper_PrintStrSafe(pDumper, ", streamout)\n");
                                }
                                else
                                {
                                    vscDumper_PrintStrSafe(pDumper, ")\n");
                                }
                            }
                            else
                            {
                                if (pIoMappingPerExeObj->soIoIndexMask & (1LL << ioIndex))
                                {
                                    vscDumper_PrintStrSafe(pDumper, " (streamout)\n");
                                }
                                else
                                {
                                    vscDumper_PrintStrSafe(pDumper, "\n");
                                }
                            }
                        }

                        vscDumper_DumpBuffer(pDumper);
                    }
                }
            }
        }
    }
}

void _PrintIoMapping(SHADER_IO_MAPPING* pIoMapping, gctBOOL bInput, VIR_Shader* pShader, VSC_DUMPER* pDumper)
{
    _PrintIoMappingPerExeObj(&pIoMapping->ioVtxPxl, bInput, gcvFALSE, pShader, pDumper);
    _PrintIoMappingPerExeObj(&pIoMapping->ioPrim, bInput, gcvTRUE, pShader, pDumper);
}

void _PrintConstantMapping(SHADER_CONSTANT_MAPPING* pConstantMapping, VIR_Shader* pShader, VSC_DUMPER* pDumper)
{
    gctUINT ctcIdx, i;

    /* Print CTC */
    for (ctcIdx = 0; ctcIdx < pConstantMapping->countOfCompileTimeConstant; ctcIdx ++)
    {
        SHADER_COMPILE_TIME_CONSTANT* pCTC = &pConstantMapping->pCompileTimeConstant[ctcIdx];

        gcmASSERT(pCTC->hwConstantLocation.hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER);

        vscDumper_PrintStrSafe(pDumper, "c%d = {", pCTC->hwConstantLocation.hwLoc.hwRegNo);

        for (i = 0; i < CHANNEL_NUM; i ++)
        {
            vscDumper_PrintStrSafe(pDumper, "%f(0x%08x)", *(float*)&pCTC->constantValue[i], pCTC->constantValue[i]);

            if (i < CHANNEL_NUM - 1)
            {
                vscDumper_PrintStrSafe(pDumper, ",");
            }
        }

        vscDumper_PrintStrSafe(pDumper, "}\n");
        vscDumper_DumpBuffer(pDumper);
    }
}

void _PrintMappingTables(SHADER_EXECUTABLE_PROFILE* pSEP, VIR_Shader* pShader, VSC_DUMPER* pDumper)
{
    vscDumper_PrintStrSafe(pDumper, "[mapping tables]");
    vscDumper_DumpBuffer(pDumper);

    /* TODO:
            If pShader is not NULL, we need dump vir-symbol->#->hw resource, not only #->hw resource.
    */

    /* Print inputs */
    _PrintIoMapping(&pSEP->inputMapping, gcvTRUE, pShader, pDumper);

    /* Print outputs */
    _PrintIoMapping(&pSEP->outputMapping, gcvFALSE, pShader, pDumper);

    /* Print constant mapping */
    _PrintConstantMapping(&pSEP->constantMapping, pShader, pDumper);
}

void _PrintSEPMisc(SHADER_EXECUTABLE_PROFILE* pSEP, VSC_DUMPER* pDumper)
{
    gctCHAR*   strClientType[] = {"UNKNOWN", "dx", "gl", "gles", "cl"};
    gctCHAR*   strShaderType[] = {"UNKNOWN", "vs", "ps", "gs", "hs", "ds", "gps"};

    /* Print shader type */
    vscDumper_PrintStrSafe(pDumper, "%s_%s_%d_%d\n",
                                    strClientType[DECODE_SHADER_CLIENT(pSEP->shVersionType)],
                                    strShaderType[DECODE_SHADER_TYPE(pSEP->shVersionType)],
                                    DECODE_SHADER_MAJOR_VER(pSEP->shVersionType),
                                    DECODE_SHADER_MINOR_VER(pSEP->shVersionType));

    /* Print chip version */
    vscDumper_PrintStrSafe(pDumper, "chip = 0x%x\n", pSEP->chipModel);
    vscDumper_PrintStrSafe(pDumper, "chipRevision = 0x%x\n", pSEP->chipRevision);

    /* Print total inst count */
    vscDumper_PrintStrSafe(pDumper, "instCount = %d\n", pSEP->countOfMCInst);

    /* Print end pc */
    vscDumper_PrintStrSafe(pDumper, "endPC = %d\n", pSEP->endPCOfMainRoutine);

    /* Print GPR count */
    vscDumper_PrintStrSafe(pDumper, "tempRegCount = %d\n", pSEP->gprCount);

    /* Flush now */
    vscDumper_DumpBuffer(pDumper);
}

void _PrintExeHints(SHADER_EXECUTABLE_PROFILE* pSEP, VSC_DUMPER* pDumper)
{
    SHADER_EXECUTABLE_HINTS* pExeHints = &pSEP->exeHints;

    gctCONST_STRING strOnOff[] =
    {
        "off",
        "on"
    };

    gctCONST_STRING strYesNo[] =
    {
        "no",
        "yes"
    };

    gctCONST_STRING strURFAllocStrategy[] =
    {
        "unified",
        "fixed",
        "float"
    };

    gctCONST_STRING strGsInputPrimName[] =
    {
        "point",
        "line",
        "triangle"
        "line_adj",
        "triangle_adj",
        "patch_1",
        "patch_2",
        "patch_3",
        "patch_4",
        "patch_5",
        "patch_6",
        "patch_7",
        "patch_8",
        "patch_9",
        "patch_10",
        "patch_11",
        "patch_12",
        "patch_13",
        "patch_14",
        "patch_15",
        "patch_16",
        "patch_17",
        "patch_18",
        "patch_19",
        "patch_20",
        "patch_21",
        "patch_22",
        "patch_23",
        "patch_24",
        "patch_25",
        "patch_26",
        "patch_27",
        "patch_28",
        "patch_29",
        "patch_30",
        "patch_31",
        "patch_32",
    };

    gctCONST_STRING strGsOutputPrimName[] =
    {
        "pointlist",
        "linestrip",
        "triaglestrip"
    };

    gctCONST_STRING strTessDomainTypeName[] =
    {
        "isoline",
        "triangle",
        "quad"
    };

    gctCONST_STRING strTessPartitionTypeName[] =
    {
        "integer",
        "pow2",
        "fractional_odd",
        "fractional_even"
    };

    gctCONST_STRING strTessOutputPrimName[] =
    {
        "point",
        "line",
        "triangle_cw",
        "triangle_ccw"
    };

    vscDumper_PrintStrSafe(pDumper, "[exe-hints]");
    vscDumper_DumpBuffer(pDumper);

    vscDumper_PrintStrSafe(pDumper, "executeOnDual16: %s\n", strOnOff[pExeHints->derivedHints.globalStates.bExecuteOnDual16]);
    vscDumper_PrintStrSafe(pDumper, "allocCrByUnifiedMode: %s\n", strURFAllocStrategy[pExeHints->derivedHints.globalStates.unifiedConstRegAllocStrategy]);
    vscDumper_PrintStrSafe(pDumper, "allocSrByUnifiedMode: %s\n", strURFAllocStrategy[pExeHints->derivedHints.globalStates.unifiedSamplerRegAllocStrategy]);
    vscDumper_PrintStrSafe(pDumper, "gprSpilled: %s\n", strYesNo[pExeHints->derivedHints.globalStates.bGprSpilled]);

    if (DECODE_SHADER_TYPE(pSEP->shVersionType) == SHADER_TYPE_HULL ||
        DECODE_SHADER_TYPE(pSEP->shVersionType) == SHADER_TYPE_DOMAIN)
    {
        vscDumper_PrintStrSafe(pDumper, "inputCtrlPointCount: %d\n", pExeHints->nativeHints.prvStates.ts.inputCtrlPointCount);

        if (DECODE_SHADER_TYPE(pSEP->shVersionType) == SHADER_TYPE_HULL)
        {
            vscDumper_PrintStrSafe(pDumper, "outputCtrlPointCount: %d\n", pExeHints->nativeHints.prvStates.ts.outputCtrlPointCount);
        }

        if ((DECODE_SHADER_CLIENT(pSEP->shVersionType) == SHADER_CLIENT_DX && DECODE_SHADER_TYPE(pSEP->shVersionType) == SHADER_TYPE_HULL) ||
            (DECODE_SHADER_CLIENT(pSEP->shVersionType) != SHADER_CLIENT_DX && DECODE_SHADER_TYPE(pSEP->shVersionType) == SHADER_TYPE_DOMAIN))
        {
            vscDumper_PrintStrSafe(pDumper, "tessDomainType: %s\n", strTessDomainTypeName[pExeHints->nativeHints.prvStates.ts.tessDomainType]);
            vscDumper_PrintStrSafe(pDumper, "tessPartitionType: %s\n", strTessPartitionTypeName[pExeHints->nativeHints.prvStates.ts.tessPartitionType]);
            vscDumper_PrintStrSafe(pDumper, "tessOutputPrim: %s\n", strTessOutputPrimName[pExeHints->nativeHints.prvStates.ts.tessOutputPrim]);
            vscDumper_PrintStrSafe(pDumper, "maxTessFactor: %d\n", pExeHints->nativeHints.prvStates.ts.maxTessFactor);
        }
    }
    else if (DECODE_SHADER_TYPE(pSEP->shVersionType) == SHADER_TYPE_GEOMETRY)
    {
        vscDumper_PrintStrSafe(pDumper, "inputPrim: %s\n", strGsInputPrimName[pExeHints->nativeHints.prvStates.gs.inputPrim]);
        vscDumper_PrintStrSafe(pDumper, "outputPrim: %s\n", strGsOutputPrimName[pExeHints->nativeHints.prvStates.gs.outputPrim]);
        vscDumper_PrintStrSafe(pDumper, "maxOutputVtxCount: %d\n", pExeHints->nativeHints.prvStates.gs.maxOutputVtxCount);
        vscDumper_PrintStrSafe(pDumper, "instanceCount: %d\n", pExeHints->nativeHints.prvStates.gs.instanceCount);
    }
    else if (DECODE_SHADER_TYPE(pSEP->shVersionType) == SHADER_TYPE_PIXEL)
    {
        vscDumper_PrintStrSafe(pDumper, "executeOnSampleFreq: %s\n", strOnOff[pExeHints->derivedHints.prvStates.ps.bExecuteOnSampleFreq]);
        vscDumper_PrintStrSafe(pDumper, "earlyPixelTestInRa: %s\n", strYesNo[pExeHints->nativeHints.prvStates.ps.bEarlyPixelTestInRa]);
    }

    /* Flush now */
    vscDumper_DumpBuffer(pDumper);
}

void vscPrintSEP(SHADER_EXECUTABLE_PROFILE* pSEP, void* pVirShader)
{
    VSC_DUMPER  sepDumper;
    gctUINT     dumpBufferSize = 1024;
    gctCHAR*    pDumpBuffer;
    VIR_Shader* pShader = (VIR_Shader*)pVirShader;
    gctCHAR*    strShaderType[] = {"UNKNOWN", "VS", "PS", "GS", "HS", "DS", "GPS"};

    if (gcoOS_Allocate(gcvNULL, 1024, (gctPOINTER*)&pDumpBuffer) != gcvSTATUS_OK)
    {
        return;
    }

    vscDumper_Initialize(&sepDumper,
                         gcvNULL,
                         gcvNULL,
                         pDumpBuffer,
                         dumpBufferSize);

    if (pShader)
    {
        vscDumper_PrintStrSafe(&sepDumper,
                               "\n************ [ Generated Shader Executable Profile <%s> (id:%u)] ************",
                               strShaderType[DECODE_SHADER_TYPE(pSEP->shVersionType)],
                               VIR_Shader_GetId(pShader));
    }
    else
    {
        vscDumper_PrintStrSafe(&sepDumper,
                               "\n************ [ Generated Shader Executable Profile <%s>] ************",
                               strShaderType[DECODE_SHADER_TYPE(pSEP->shVersionType)]);
    }

    vscDumper_DumpBuffer(&sepDumper);

    /* Print SEP version */
    vscDumper_PrintStrSafe(&sepDumper, "SEP_%d_%d\n", DECODE_SEP_MAJOR_VER(pSEP->profileVersion),
                                                      DECODE_SEP_MINOR_VER(pSEP->profileVersion));
    vscDumper_DumpBuffer(&sepDumper);

    /* Print misc info */
    _PrintSEPMisc(pSEP, &sepDumper);

    /* Print executable hints */
    _PrintExeHints(pSEP, &sepDumper);

    /* Print mapping tables */
    _PrintMappingTables(pSEP, pShader, &sepDumper);

    /* Print code */
    _PrintMachineCode((VSC_MC_RAW_INST*)pSEP->pMachineCode,
                      pSEP->countOfMCInst,
                      pSEP->exeHints.derivedHints.globalStates.bExecuteOnDual16,
                      &sepDumper);

    /* Release dumper buffer */
    gcoOS_Free(gcvNULL, pDumpBuffer);
}

VSC_ErrCode
vscVIR_GenerateSEP(
    IN  VIR_Shader*                pShader,
    IN  VSC_HW_CONFIG*             pHwCfg,
    IN  gctBOOL                    bDumpSEP,
    OUT SHADER_EXECUTABLE_PROFILE* pOutSEP
    )
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VSC_SEP_GEN_HELPER         sepGenHelper;

    sepGenHelper.pShader = pShader;
    sepGenHelper.baseEpGen.pHwCfg = pHwCfg;
    vscPMP_Intialize(&sepGenHelper.baseEpGen.pmp, gcvNULL, 512, sizeof(void*), gcvTRUE);

    vscInitializeSEP(pOutSEP);

    /* SEP profile version */
    pOutSEP->profileVersion = ENCODE_SEP_VER_TYPE(1, 0);

    /* HW target that this SEP works on */
    pOutSEP->chipModel = pHwCfg->chipModel;
    pOutSEP->chipRevision = pHwCfg->chipRevision;

    /* Shader type and version */
    pOutSEP->shVersionType = _GenerateShaderVersionAndType(pShader);

    /* Collect machine code */
    errCode = _CollectMachineCodeToSEP(&sepGenHelper, pOutSEP);
    ON_ERROR(errCode, "Collect machine code to SEP");

    /* Calc temp register count */
    pOutSEP->gprCount = VIR_Shader_GetRegWatermark(pShader);

    /* Collect all executable hints */
    _CollectExeHints(&sepGenHelper, pOutSEP);

    /* Input-mapping collection */
    errCode = _CollectIoMappingToSEP(&sepGenHelper, gcvTRUE, pOutSEP);
    ON_ERROR(errCode, "Collect input mapping to SEP");

    /* Output-mapping collection */
    errCode = _CollectIoMappingToSEP(&sepGenHelper, gcvFALSE, pOutSEP);
    ON_ERROR(errCode, "Collect output mapping to SEP");

    /* Constant-mapping collection */
    errCode = _CollectConstantMappingToSEP(&sepGenHelper, pOutSEP);
    ON_ERROR(errCode, "Collect constant mapping to SEP");

    /* Sampler-mapping collection */
    errCode = _CollectSamplerMappingToSEP(&sepGenHelper, pOutSEP);
    ON_ERROR(errCode, "Collect sampler mapping to SEP");

    /* Static patches collection */
    errCode = _CollectStaticPatches(&sepGenHelper, pOutSEP);
    ON_ERROR(errCode, "Collect static patches to SEP");

    if (bDumpSEP)
    {
        vscPrintSEP(pOutSEP, pShader);
    }

OnError:
    if (errCode != VSC_ERR_NONE)
    {
        vscFinalizeSEP(pOutSEP);
    }

    vscPMP_Finalize(&sepGenHelper.baseEpGen.pmp);

    return errCode;
}


