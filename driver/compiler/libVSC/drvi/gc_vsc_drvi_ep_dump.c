/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_vsc.h"

void _PrintMachineCode(VSC_MC_RAW_INST* pMcCode,
                       gctUINT countOfMCInst,
                       VSC_HW_CONFIG* pHwCfg,
                       gctBOOL bExecuteOnDual16,
                       VSC_DUMPER* pDumper)
{
    if (pMcCode && countOfMCInst > 0)
    {
        vscDumper_PrintStrSafe(pDumper, "[code]");
        vscDumper_DumpBuffer(pDumper);

        vscMC_DumpInsts(pMcCode, countOfMCInst, pHwCfg, bExecuteOnDual16, pDumper);
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

        if (pCTC->hwConstantLocation.hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER)
        {
            vscDumper_PrintStrSafe(pDumper, "c%d = {", pCTC->hwConstantLocation.hwLoc.hwRegNo);
        }
        else
        {
            vscDumper_PrintStrSafe(pDumper, "m@%d = {", pCTC->hwConstantLocation.hwLoc.memAddr.offsetInConstantArray);
        }

        for (i = 0; i < CHANNEL_NUM; i ++)
        {
            if ((1 << i) & pCTC->hwConstantLocation.validHWChannelMask)
            {
                vscDumper_PrintStrSafe(pDumper, "%f(0x%08x)", *(float*)&pCTC->constantValue[i], pCTC->constantValue[i]);
            }
            else
            {
                vscDumper_PrintStrSafe(pDumper, "user-defined");
            }

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
    gctCHAR*   strClientType[] = {"UNKNOWN", "dx", "gl", "gles", "cl", "vk"};
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
    vscDumper_PrintStrSafe(pDumper, "crSpilled: %s\n", strYesNo[pExeHints->derivedHints.globalStates.bCrSpilled]);

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

void vscPrintSEP(VSC_SYS_CONTEXT* pSysCtx, SHADER_EXECUTABLE_PROFILE* pSEP, SHADER_HANDLE hShader)
{
    VSC_DUMPER    sepDumper;
    gctUINT       dumpBufferSize = 1024;
    gctCHAR*      pDumpBuffer;
    VIR_Shader*   pShader = (VIR_Shader*)hShader;
    gctCHAR*      strShaderType[] = {"UNKNOWN", "VS", "PS", "GS", "HS", "DS", "GPS"};

    /* We support cross-HW SEP dump. So ideally, we should get vscHWCFG based on chip model
       and its revision which is stored in SEP. However, we have no such framework yet, so
       we just get it from VSC_SYS_CONTEXT now, which means we can not support cross-HW SEP
       dump so far.

       Actually, although VSC compile interfaces have supported cross-compilation, driver can
       not get vsc-HWCFG for not-current HW, we also dont support cross-compilation in fact.
       So we need consider it later. */

    if (gcoOS_Allocate(gcvNULL, dumpBufferSize, (gctPOINTER*)&pDumpBuffer) != gcvSTATUS_OK)
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
                      &pSysCtx->pCoreSysCtx->hwCfg,
                      pSEP->exeHints.derivedHints.globalStates.bExecuteOnDual16,
                      &sepDumper);

    if (gcmOPT_EnableDebugDump() &&
        pShader->debugInfo)
    {
        vscDIDumpDIETree(pShader->debugInfo, 0, 0xffffffff);
        vscDIDumpLineTable(pShader->debugInfo);
    }

    /* Release dumper buffer */
    gcoOS_Free(gcvNULL, pDumpBuffer);
}


