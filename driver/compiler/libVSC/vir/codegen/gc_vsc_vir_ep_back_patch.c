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
#include "vir/codegen/gc_vsc_vir_ep_back_patch.h"

extern SHADER_COMPILE_TIME_CONSTANT* _EnlargeCTCRoom(SHADER_CONSTANT_MAPPING* pCnstMapping, gctUINT enlargeCTCCount, gctUINT* pStartCtcSlot);
extern void _SetValidChannelForHwConstantLoc(SHADER_CONSTANT_HW_LOCATION_MAPPING* pCnstHwLoc, gctUINT hwChannel);

#if gcdALPHA_KILL_IN_SHADER
static VSC_ErrCode _AddAlphaKillPatch(VIR_Shader* pShader, VSC_HW_CONFIG* pHwCfg, SHADER_EXECUTABLE_PROFILE* pOutSEP)
{
    VSC_MC_RAW_INST               instAlphaKill, instColorKill;
    gctUINT*                      pOrgMachineCode = pOutSEP->pMachineCode;
    gctUINT                       hwCnstRegNoFor1f = NOT_ASSIGNED, hwCnstRegChannelFor1f = NOT_ASSIGNED;
    gctUINT                       hwCnstRegNoFor1fDiv256f = NOT_ASSIGNED, hwCnstRegChannelFor1fDiv256f  = NOT_ASSIGNED;
    gctUINT                       ioIdx, firstValidIoChannel, hwRegNoForClrOutput = NOT_ASSIGNED, clrOutputCount = 0;
    SHADER_COMPILE_TIME_CONSTANT* pCTC;
    gctFLOAT                      imm1f = 1.0f, imm1fDiv256f = 1.0f / 256.0f;
    VSC_MC_CODEC                  mcCodec;
    VSC_MC_CODEC_INST             codecHelperInst;
    gctBOOL                       bNeedConstReg = (!pHwCfg->hwFeatureFlags.hasSHEnhance2 ||
                                                   pOutSEP->exeHints.derivedHints.globalStates.bExecuteOnDual16);

    if (!VIR_Shader_PS_NeedAlphaKillPatch(pShader))
    {
        return VSC_ERR_NONE;
    }

    gcmASSERT((SHADER_TYPE)DECODE_SHADER_TYPE(pOutSEP->shVersionType) == SHADER_TYPE_PIXEL);

    /* Get color output HW reg no */
    for (ioIdx = 0; ioIdx < pOutSEP->outputMapping.ioVtxPxl.countOfIoRegMapping; ioIdx ++)
    {
        if (pOutSEP->outputMapping.ioVtxPxl.usage2IO[SHADER_IO_USAGE_COLOR].ioIndexMask & (1LL << ioIdx))
        {
            firstValidIoChannel = pOutSEP->outputMapping.ioVtxPxl.pIoRegMapping[ioIdx].firstValidIoChannel;
            hwRegNoForClrOutput = pOutSEP->outputMapping.ioVtxPxl.pIoRegMapping[ioIdx].ioChannelMapping[firstValidIoChannel].
                                  hwLoc.cmnHwLoc.u.hwRegNo;

            /* We only generate 2 insts now, so don't consider HP output case which need single-t mode */
            if (pOutSEP->outputMapping.ioVtxPxl.pIoRegMapping[ioIdx].ioChannelMapping[firstValidIoChannel].
                flag.bHighPrecisionOnDual16)
            {
                return VSC_ERR_NONE;
            }

            clrOutputCount ++;
        }
    }

    /* Don't apply for MRT */
    if (clrOutputCount != 1)
    {
        return VSC_ERR_NONE;
    }

    /* If HW dost not support imm, we need find 2 free hw const reg channels to put 2 imms */
    if (bNeedConstReg)
    {
        if (pOutSEP->constantMapping.hwConstRegCount >= pHwCfg->maxPSConstRegCount)
        {
            return VSC_ERR_NONE;
        }

        /* Now just use a very simple way to get next freed hw const reg */
        hwCnstRegNoFor1fDiv256f = hwCnstRegNoFor1f = pOutSEP->constantMapping.hwConstRegCount ++;
        hwCnstRegChannelFor1f = CHANNEL_X;
        hwCnstRegChannelFor1fDiv256f = CHANNEL_Y;

        /* Add 2 imms to CTC */
        pCTC = _EnlargeCTCRoom(&pOutSEP->constantMapping, 1, gcvNULL);
        vscInitializeCTC(pCTC);
        pCTC->hwConstantLocation.hwLoc.constReg.hwRegNo = hwCnstRegNoFor1fDiv256f;
        pCTC->hwConstantLocation.hwLoc.constReg.hwRegRange = 1;
        pCTC->hwConstantLocation.hwAccessMode = SHADER_HW_ACCESS_MODE_REGISTER;
        _SetValidChannelForHwConstantLoc(&pCTC->hwConstantLocation, hwCnstRegChannelFor1f);
        pCTC->constantValue[hwCnstRegChannelFor1f] = *(gctUINT*)&imm1f;
        _SetValidChannelForHwConstantLoc(&pCTC->hwConstantLocation, hwCnstRegChannelFor1fDiv256f);
        pCTC->constantValue[hwCnstRegChannelFor1fDiv256f] = *(gctUINT*)&imm1fDiv256f;
    }

    /* Begin to generate alpha-kill and color-kill insts */
    vscMC_BeginCodec(&mcCodec,
                     pHwCfg,
                     pOutSEP->exeHints.derivedHints.globalStates.bExecuteOnDual16,
                     gcvTRUE);

    /* dp4 color.w, color.xyzw, 1.0 */
    memset(&codecHelperInst, 0, sizeof(codecHelperInst));
    codecHelperInst.baseOpcode = 0x06;
    codecHelperInst.bDstValid = gcvTRUE;
    codecHelperInst.dst.regNo = hwRegNoForClrOutput;
    codecHelperInst.dst.regType = pOutSEP->exeHints.derivedHints.globalStates.bExecuteOnDual16 ?
                                  0x0 : 0x0;
    codecHelperInst.dst.u.nmlDst.writeMask = WRITEMASK_W;
    codecHelperInst.instCtrl.instType = 0x0;
    codecHelperInst.instCtrl.threadType = 0x0;
    codecHelperInst.srcCount = 2;
    codecHelperInst.src[0].regType = pOutSEP->exeHints.derivedHints.globalStates.bExecuteOnDual16 ?
                                     0x0 : 0x0;
    codecHelperInst.src[0].u.reg.regNo = hwRegNoForClrOutput;
    codecHelperInst.src[0].u.reg.swizzle = VIR_SWIZZLE_XYZW;
    if (bNeedConstReg)
    {
        codecHelperInst.src[1].regType = 0x2;
        codecHelperInst.src[1].u.reg.regNo = hwCnstRegNoFor1f;
        codecHelperInst.src[1].u.reg.swizzle = VIR_SWIZZLE_XXXX;
    }
    else
    {
        codecHelperInst.src[1].regType = 0x7;
        codecHelperInst.src[1].u.imm.immData.ui = *(gctUINT*)&imm1f;
        codecHelperInst.src[1].u.imm.immType = 0x0;
    }
    vscMC_EncodeInst(&mcCodec, &codecHelperInst, &instColorKill);

    /* texkill.lt color.w, 1/256 */
    memset(&codecHelperInst, 0, sizeof(codecHelperInst));
    codecHelperInst.baseOpcode = 0x17;
    codecHelperInst.instCtrl.condOpCode = 0x02;
    codecHelperInst.instCtrl.instType = 0x0;
    codecHelperInst.instCtrl.threadType = 0x0;
    codecHelperInst.srcCount = 2;
    codecHelperInst.src[0].regType = pOutSEP->exeHints.derivedHints.globalStates.bExecuteOnDual16 ?
                                     0x0 : 0x0;
    codecHelperInst.src[0].u.reg.regNo = hwRegNoForClrOutput;
    codecHelperInst.src[0].u.reg.swizzle = VIR_SWIZZLE_WWWW;
    if (bNeedConstReg)
    {
        codecHelperInst.src[1].regType = 0x2;
        codecHelperInst.src[1].u.reg.regNo = hwCnstRegNoFor1fDiv256f;
        codecHelperInst.src[1].u.reg.swizzle = VIR_SWIZZLE_YYYY;
    }
    else
    {
        codecHelperInst.src[1].regType = 0x7;
        codecHelperInst.src[1].u.imm.immData.ui = *(gctUINT*)&imm1fDiv256f;
        codecHelperInst.src[1].u.imm.immType = 0x0;
    }
    vscMC_EncodeInst(&mcCodec, &codecHelperInst, &instAlphaKill);

    /* Ok, we finish the inst generation */
    vscMC_EndCodec(&mcCodec);

    /* Allocate new HW inst memory of SEP by inscreasing 2 inst count */
    if (gcoOS_Allocate(gcvNULL, (pOutSEP->countOfMCInst + 2)*sizeof(VSC_MC_RAW_INST),
        (gctPOINTER*)&pOutSEP->pMachineCode) != gcvSTATUS_OK)
    {
        return VSC_ERR_OUT_OF_MEMORY;
    }

    /* Copy main routine */
    gcoOS_MemCopy((gctUINT8*)pOutSEP->pMachineCode,
                  pOrgMachineCode,
                  (pOutSEP->endPCOfMainRoutine + 1)*sizeof(VSC_MC_RAW_INST));

    /* Copy color-kill inst */
    gcoOS_MemCopy((gctUINT8*)pOutSEP->pMachineCode + (pOutSEP->endPCOfMainRoutine + 1)*sizeof(VSC_MC_RAW_INST),
                  &instColorKill,
                  sizeof(VSC_MC_RAW_INST));

    /* Copy alpha-kill inst */
    gcoOS_MemCopy((gctUINT8*)pOutSEP->pMachineCode + (pOutSEP->endPCOfMainRoutine + 2)*sizeof(VSC_MC_RAW_INST),
                  &instAlphaKill,
                  sizeof(VSC_MC_RAW_INST));

    /* Copy sub-routines */
    if ((pOutSEP->countOfMCInst - pOutSEP->endPCOfMainRoutine - 1) > 0)
    {
        gcoOS_MemCopy((gctUINT8*)pOutSEP->pMachineCode + (pOutSEP->endPCOfMainRoutine + 3)*sizeof(VSC_MC_RAW_INST),
                      (gctUINT8*)pOrgMachineCode + (pOutSEP->endPCOfMainRoutine + 1)*sizeof(VSC_MC_RAW_INST),
                      (pOutSEP->countOfMCInst - pOutSEP->endPCOfMainRoutine - 1) * sizeof(VSC_MC_RAW_INST));
    }

    /* Set new inst count. Note that DON'T change the endPCOfMainRoutine because whether change it will be dependent
       on driver decision */
    pOutSEP->countOfMCInst += 2;

    /* Free original mahcine code buffer */
    gcoOS_Free(gcvNULL, pOrgMachineCode);

    /* Mark alpha-kill has been added */
    pOutSEP->exeHints.derivedHints.prvStates.ps.alphaClrKillInstsGened = gcvTRUE;

    return VSC_ERR_NONE;
}
#endif

DEF_QUERY_PASS_PROP(vscVIR_PerformSEPBackPatch)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_PST;
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_SEP_GEN;
}

DEF_SH_NECESSITY_CHECK(vscVIR_PerformSEPBackPatch)
{
    SHADER_EXECUTABLE_PROFILE* pOutSEP = (SHADER_EXECUTABLE_PROFILE*)pPassWorker->basePassWorker.pPassSpecificData;

    if (!(pOutSEP && vscIsValidSEP(pOutSEP)))
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

VSC_ErrCode
vscVIR_PerformSEPBackPatch(
    VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_Shader*                pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VSC_HW_CONFIG*             pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    SHADER_EXECUTABLE_PROFILE* pOutSEP = (SHADER_EXECUTABLE_PROFILE*)pPassWorker->basePassWorker.pPassSpecificData;
    VSC_OPTN_SEPGenOptions*    sepgen_options = (VSC_OPTN_SEPGenOptions*)pPassWorker->basePassWorker.pBaseOption;

    /* This code is temp put here because such patch is actually a dynamic (recompile)
       patch, and if we put it to general static patch framework, it will complex other
       stages unless we can design multiple-version SEP (which means we can prepare more
       than one SEP for driver's selection to flush) later. */
#if gcdALPHA_KILL_IN_SHADER
    errCode = _AddAlphaKillPatch(pShader, pHwCfg, pOutSEP);
    ON_ERROR(errCode, "Add alphakill patch to SEP");
#endif

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader),
                                           VSC_OPTN_DumpOptions_DUMP_FINALIR))
    {
        VIR_Dumper *pDumper = pPassWorker->basePassWorker.pDumper;
        VIR_Shader_Dump(gcvNULL, "Shader IR", pShader, gcvTRUE);
        VIR_LOG_FLUSH(pDumper);
    }

    if (VSC_OPTN_SEPGenOptions_GetTrace(sepgen_options) ||
        VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_CG))
    {
        vscPrintSEP(pPassWorker->pCompilerParam->cfg.ctx.pSysCtx, pOutSEP, pShader);
    }

    /* check shader instruction in dual16 mode, same check as AQSHADER30::Execute */
    gcmASSERT(!VIR_Shader_isDual16Mode(pShader) || (pOutSEP->endPCOfMainRoutine < 1024));

OnError:
    return errCode;
}


