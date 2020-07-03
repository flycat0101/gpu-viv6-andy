/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
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

/* Defined in drvi_link.c */
extern void _ConvertVirPerVtxPxlAndPerPrimIoList(VIR_Shader* pShader,
                                                 VSC_MM* pMM,
                                                 gctBOOL bInput,
                                                 VIR_IdList* pPerVtxPxlList,
                                                 VIR_IdList* pPerPrimList);

typedef struct _VSC_BASE_EP_GEN_HELPER
{
    VSC_MM*                           pMM;
    PVSC_HW_CONFIG                    pHwCfg;
}VSC_BASE_EP_GEN_HELPER;

typedef struct _VSC_SEP_GEN_HELPER
{
    VSC_BASE_EP_GEN_HELPER            baseEpGen;
    VIR_Shader*                       pShader;
    gctBOOL                           bSkipPepGen;
}VSC_SEP_GEN_HELPER;

typedef struct _VSC_PEP_GEN_HELPER
{
    VSC_BASE_EP_GEN_HELPER            baseEpGen;
    VIR_Shader*                       pShader;
    SHADER_EXECUTABLE_PROFILE*        pSep;
}VSC_PEP_GEN_HELPER;

static gctUINT _GenerateShaderVersionAndType(VIR_Shader* pShader)
{
    SHADER_CLIENT shClient = SHADER_CLIENT_UNKNOWN;
    SHADER_TYPE   shType = SHADER_TYPE_UNKNOWN;
    gctUINT       shMajorVer = 0;
    gctUINT       shMinorVer = 0;

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
    case gcvAPI_OPENGL_ES32:
        gcmASSERT((pShader->compilerVersion[0] & 0xFFFF) == _SHADER_GL_LANGUAGE_TYPE ||
                  /* to be removed after VG shader has its only client api set */
                  (pShader->compilerVersion[0] & 0xFFFF) == _SHADER_VG_TYPE);
        shClient = SHADER_CLIENT_GLES;
        break;
    case gcvAPI_OPENGL:
        gcmASSERT((pShader->compilerVersion[0] & 0xFFFF) == _SHADER_OGL_LANGUAGE_TYPE);
        shClient = SHADER_CLIENT_GL;
        break;
    case gcvAPI_OPENCL:
        gcmASSERT((pShader->compilerVersion[0] & 0xFFFF) == _cldLanguageType);
        shClient = SHADER_CLIENT_CL;
        break;
    case gcvAPI_OPENVK:
        shClient = SHADER_CLIENT_VK;
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
    case VIR_SHADER_COMPUTE:
        shType = SHADER_TYPE_GENERAL;
        break;
    default:
        shType = SHADER_TYPE_UNKNOWN;
        break;
    }

    VIR_Shader_DecodeCompilerVersionToShVersion(pShader,
                                                pShader->compilerVersion[1],
                                                &shMajorVer,
                                                &shMinorVer);

    return ENCODE_SHADER_VER_TYPE(shClient, shType, shMajorVer, shMinorVer);
}

#define _DUMP_REG 0

static VSC_ErrCode _UpdateHWLocPcRange(VIR_Shader *pShader, VIR_Instruction* pInst, gctUINT curPC)
{
    VIR_OpCode virOpcode    = VIR_Inst_GetOpcode(pInst);
    gctSIZE_T  i = 0;
    VIR_Operand  *operand;
    VIR_Symbol *sym;
#if _DUMP_REG
    VIR_Type *type;
    gctUINT   regCount = 0;
#endif
    VSC_DI_SW_LOC *sl = gcvNULL;
    VSC_DI_HW_LOC * hl = gcvNULL;
    VSC_DIContext * diContext = (VSC_DIContext *) (pShader->debugInfo);


    if (virOpcode == VIR_OP_JMP ||
        virOpcode == VIR_OP_JMPC ||
        virOpcode == VIR_OP_JMP_ANY ||
        virOpcode == VIR_OP_CALL)
        return VSC_ERR_NONE;


    for(i = 0; i < VIR_Inst_GetSrcNum(pInst); ++i)
    {
        operand = VIR_Inst_GetSource(pInst, i);

        if (VIR_Operand_GetOpKind(operand) != VIR_OPND_SYMBOL)
            continue;

        sym  = VIR_Operand_GetSymbol(operand);

        if (VIR_Symbol_isUniform(sym))
            continue;

        sl = vscDIFindSWLoc(diContext, VIR_Symbol_GetVregIndex(sym));

        if (sl && (sl->u.reg.type == VSC_DIE_REG_TYPE_TMP))
        {
            hl = vscDIGetHWLoc(diContext, sl->hwLoc);
            if (hl)
            {
                gctUINT16 highPC = (gctUINT16) curPC - 1;
                gctUINT16 lowPC = (gctUINT16) (curPC - pInst->mcInstCount);
                if (hl->beginPC == 0)
                {
                    hl->beginPC = lowPC;
                }
                else
                {
                    if (highPC > hl->endPC)
                        hl->endPC = highPC;
                    if (lowPC < hl->beginPC)
                        hl->beginPC = lowPC;
                }

                if (hl->endPC == 0)
                {
                    hl->endPC = highPC;
                }
                else
                {
                    if (highPC > hl->endPC)
                        hl->endPC = highPC;
                    if (lowPC < hl->beginPC)
                        hl->beginPC = lowPC;
                }
#if _DUMP_REG
                type = VIR_Symbol_GetType(sym);
                regCount = VIR_Type_GetVirRegCount(pShader, type, -1);
                gcmPRINT("Src curPC: %d reg: %d - %d\n", curPC, VIR_Symbol_GetVregIndex(sym), VIR_Symbol_GetVregIndex(sym) + regCount -1);
#endif
            }
        }
    }


    do
    {
        if (VIR_Inst_GetDest(pInst) != gcvNULL)
        {
            operand = VIR_Inst_GetDest(pInst);
            if (VIR_Operand_GetOpKind(operand) != VIR_OPND_SYMBOL)
                break;

            sym  = VIR_Operand_GetSymbol(operand);

            if (VIR_Symbol_isUniform(sym))
                break;

            sl = vscDIFindSWLoc(diContext, VIR_Symbol_GetVregIndex(sym));

            if (sl && (sl->u.reg.type == VSC_DIE_REG_TYPE_TMP))
            {
                hl = vscDIGetHWLoc(diContext, sl->hwLoc);
                if (hl)
                {
                    gctUINT16 highPC = (gctUINT16) curPC - 1;
                    gctUINT16 lowPC = (gctUINT16) (curPC - pInst->mcInstCount);
                    if (hl->beginPC == 0)
                    {
                        hl->beginPC = lowPC;
                    }
                    else
                    {
                        if (highPC > hl->endPC)
                            hl->endPC = highPC;
                        if (lowPC < hl->beginPC)
                            hl->beginPC = lowPC;
                    }

                    if (hl->endPC == 0)
                    {
                        hl->endPC = highPC;
                    }
                    else
                    {
                        if (highPC > hl->endPC)
                            hl->endPC = highPC;
                        if (lowPC < hl->beginPC)
                            hl->beginPC = lowPC;
                    }

#if _DUMP_REG
                    type = VIR_Symbol_GetType(sym);
                    regCount = VIR_Type_GetVirRegCount(pShader, type, -1);
                    gcmPRINT("Dst curPC: %d reg: %d - %d\n", curPC, VIR_Symbol_GetVregIndex(sym), VIR_Symbol_GetVregIndex(sym) + regCount -1);
#endif
                }
            }
        }
    } while (gcvFALSE);

    return VSC_ERR_NONE;
}

static VSC_ErrCode _UpdateDIEPcRange(VIR_Shader *pShader, gctUINT16 endPC)
{
    VSC_DIContext *diContext;
    gctSIZE_T      i = 0;
    VSC_DI_SW_LOC  SWLoc;
    VSC_DI_HW_LOC  HWLoc;
    VIR_Symbol     *sym;

    if (!pShader)
        return VSC_ERR_NONE;

    diContext = (VSC_DIContext *)pShader->debugInfo;
    if (!diContext)
        return VSC_ERR_NONE;

    for (i = 0; i < (gctINT) VIR_IdList_Count(&pShader->uniforms); ++i)
    {
        VIR_Id      id  = VIR_IdList_GetId(&pShader->uniforms, i);
        VIR_Uniform *symUniform;
        sym = VIR_Shader_GetSymFromId(pShader, id);
        symUniform = VIR_Symbol_GetUniformPointer(pShader, sym);

        if (symUniform == gcvNULL)
        {
            continue;
        }

        if (!isSymCompilerGen(sym) && symUniform->physical != -1)
        {
            /* find the temp reg for the uniform */
            SWLoc.reg = gcvTRUE;
            SWLoc.u.reg.type = VSC_DIE_REG_TYPE_CONST;
            SWLoc.u.reg.start = (gctUINT16) symUniform->index;
            SWLoc.u.reg.end = (gctUINT16)(symUniform->index + symUniform->realUseArraySize - 1);

            HWLoc.beginPC = 0;
            HWLoc.endPC = endPC;
            HWLoc.next = VSC_DI_INVALID_HW_LOC;

            HWLoc.reg = gcvTRUE;
            HWLoc.u.reg.type = VSC_DIE_HW_REG_CONST;
            HWLoc.u.reg.start = (gctUINT16) symUniform->physical;
            HWLoc.u.reg.end = (gctUINT16) symUniform->physical;
            HWLoc.u1.swizzle = (gctUINT) symUniform->swizzle;

            vscDISetHwLocToSWLoc((VSC_DIContext *)(pShader->debugInfo), &SWLoc, &HWLoc);
        }
    }

    for (i = 0; i < diContext->dieTable.usedCount; i++)
    {
        VSC_DIE die = diContext->dieTable.die[i];
        VSC_DIE parent;
        VSC_DI_SW_LOC * sl;
        VSC_DI_HW_LOC * hl;
        gctBOOL isUniform = gcvFALSE;

        if (die.tag != VSC_DI_TAG_VARIABE &&
            die.tag != VSC_DI_TAG_PARAMETER)
            continue;

        /* if parent is block, find the function parent */
        parent = diContext->dieTable.die[die.parent];
        while (parent.tag != VSC_DI_TAG_SUBPROGRAM)
        {
            if (parent.id == VSC_DI_INVALIDE_DIE || parent.parent == VSC_DI_INVALIDE_DIE)
                break;
            parent = diContext->dieTable.die[parent.parent];
        }

        /* check if the variable is uniform or not */
        sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(diContext, die.u.variable.swLoc);
        while (sl)
        {
            if (sl->u.reg.type == VSC_DIE_REG_TYPE_CONST)
            {
                diContext->dieTable.die[i].u.variable.pcLine[0] = 0;
                diContext->dieTable.die[i].u.variable.pcLine[1] = endPC;
                isUniform = gcvTRUE;
                break;
            }
            sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(diContext, sl->next);
        }

        if (isUniform)
            continue;

        if (die.tag == VSC_DI_TAG_VARIABE)
        {
            sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(diContext, die.u.variable.swLoc);

            while (sl)
            {
                hl = (VSC_DI_HW_LOC *) vscDIGetHWLoc(diContext, sl->hwLoc);

                while(hl)
                {
                    if (diContext->dieTable.die[i].u.variable.pcLine[0] == 0 ||
                        diContext->dieTable.die[i].u.variable.pcLine[0] > hl->beginPC)
                    {
                        diContext->dieTable.die[i].u.variable.pcLine[0] = hl->beginPC;
                    }
                    hl = (VSC_DI_HW_LOC *) vscDIGetHWLoc(diContext, hl->next);
                }
                sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(diContext, sl->next);
            }
            diContext->dieTable.die[i].u.variable.pcLine[1] = parent.u.func.pcLine[1];
        }

        if (die.tag == VSC_DI_TAG_PARAMETER)
        {
            diContext->dieTable.die[i].u.variable.pcLine[0] = parent.u.func.pcLine[0];
            diContext->dieTable.die[i].u.variable.pcLine[1] = parent.u.func.pcLine[1];
        }
    }

    return VSC_ERR_NONE;
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
    VSC_DIContext *                diContext = gcvNULL;
    gctUINT                        diLineMapCount = 0;
    gctUINT                        startPC;
    VSC_DIE *                      die;

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

            if (pInst->mcInstCount > 0)
                diLineMapCount++;
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

        if (pShader->debugInfo != gcvNULL)
        {
            diContext = (VSC_DIContext *) (pShader->debugInfo);

            if (vscDIAddLineTable(diContext, diLineMapCount) != gcvSTATUS_OK)
            {
                return VSC_ERR_OUT_OF_MEMORY;
            }

            diLineMapCount = 0;
        }

        /* Main rountine firstly */
        VIR_FuncIterator_Init(&funcIter, &pShader->functions);
        pFuncNode = VIR_FuncIterator_First(&funcIter);

        startPC = curPC;

        for(; pFuncNode != gcvNULL; pFuncNode = VIR_FuncIterator_Next(&funcIter))
        {
            VIR_Function *pFunc = pFuncNode->function;

            if (pFunc->flags & VIR_FUNCFLAG_MAIN)
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

                        if (diContext != gcvNULL)
                        {
                            vscDIAddLineMap(diContext, diLineMapCount, pInst->sourceLoc,curPC - pInst->mcInstCount,curPC - 1);
                            diLineMapCount++;
                        }
                    }
                }
            }
        }

        if (bHasValidSubRoutine)
        {
            gcmASSERT(curPC > 1);

            pInstOfEOM = (VSC_MC_RAW_INST*)pOutSEP->pMachineCode + curPC - 1;

            /* Last of main routine must be NOP when there are other subroutines */
            if (!(pInstOfEOM->word[0] == 0 &&
                  pInstOfEOM->word[1] == 0 &&
                  pInstOfEOM->word[2] == 0 &&
                  pInstOfEOM->word[3] == 0))
            {
                gcmASSERT(0);
            }

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

                if (diContext != gcvNULL)
                {
                    /* vscDIAddLineMap(diContext, diLineMapCount, nopInst->sourceLoc, curPC - 1, curPC - 1);*/
                    diLineMapCount++;
                }
            }

            /* End PC of execution */
            pOutSEP->endPCOfMainRoutine = curPC - 1;
        }

        if (diContext != gcvNULL)
        {
            die = vscDIGetDIE(diContext, pShader->mainFunction->die);

            if (die != gcvNULL)
            {
                die->u.func.pcLine[0] = (gctUINT16)startPC;
                die->u.func.pcLine[1] = (gctUINT16)pOutSEP->endPCOfMainRoutine;
            }
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
                startPC = curPC;

                for (; pInst != gcvNULL; pInst = VIR_InstIterator_Next(&instIter))
                {
                    if (pInst->mcInstCount)
                    {
                        gcmASSERT(pInst->mcInst);

                        gcoOS_MemCopy((gctUINT8*)pOutSEP->pMachineCode + curPC*sizeof(VSC_MC_RAW_INST),
                                      pInst->mcInst,
                                      pInst->mcInstCount*sizeof(VSC_MC_RAW_INST));

                        curPC += pInst->mcInstCount;

                        /* set hw loc's pc range in debug info */
                        if (diContext != gcvNULL)
                        {
                            _UpdateHWLocPcRange(pShader, pInst, curPC);
                            vscDIAddLineMap(diContext, diLineMapCount, pInst->sourceLoc,curPC - pInst->mcInstCount, curPC - 1);
                            diLineMapCount++;
                        }
                    }
                }

                if (diContext != gcvNULL)
                {
                    die = vscDIGetDIE(diContext, pFunc->die);

                    if (die != gcvNULL)
                    {
                        die->u.func.pcLine[0] = (gctUINT16)startPC;
                        die->u.func.pcLine[1] = (gctUINT16)curPC -1;
                    }
                }
            }
        }

        gcmASSERT(curPC == pOutSEP->countOfMCInst);
    }

    if (pShader->debugInfo)
    {
        _UpdateDIEPcRange(pShader, (gctUINT16)curPC -1);
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
             (VIR_Symbol_GetFlags(pVirIoSym) & VIR_SYMFLAG_POINTSPRITE_TC))
    {
        return SHADER_IO_USAGE_POINT_COORD;
    }
    else if (virName == VIR_NAME_DEPTH)
    {
        return SHADER_IO_USAGE_DEPTH;
    }
    else if (virName == VIR_NAME_FOG_FRAG_COORD)
    {
        return SHADER_IO_USAGE_FOG_FRAG_COORD;
    }
    else if (virName == VIR_NAME_VERTEX_ID ||
             virName == VIR_NAME_VERTEX_INDEX)
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
    else if (virName == VIR_NAME_LOCAL_INVOCATION_ID)
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

static gctBOOL _IsIoConstInterpolate(VIR_Shader* pShader,
                                     VSC_HW_CONFIG* pHwCfg,
                                     VIR_Symbol* pVirIoSym)
{
    VIR_Type*       pSymType = VIR_Symbol_GetType(pVirIoSym);

    if (isSymFlat(pVirIoSym))
    {
        return gcvTRUE;
    }

    while (VIR_Type_isArray(pSymType))
    {
        pSymType = VIR_Shader_GetTypeFromId(pShader, VIR_Type_GetBaseTypeId(pSymType));
    }

    if (VIR_TypeId_isFloat16(VIR_Type_GetIndex(pSymType)))
    {
        WARNING_REPORT(VSC_ERR_NOT_SUPPORTED, "HW can't support smooth for FP16!!!");
        return gcvTRUE;
    }

    return gcvFALSE;
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
    pThisIoChannelMapping->flag.bConstInterpolate = _IsIoConstInterpolate(pShader, pHwCfg, pVirIoSym);
    pThisIoChannelMapping->flag.bStreamOutToBuffer = bStreamOut;
    pThisIoChannelMapping->flag.bPerspectiveCorrection = !pThisIoChannelMapping->flag.bConstInterpolate && !isSymNoperspective(pVirIoSym);
    pThisIoChannelMapping->flag.bSampleFrequency = isSymSample(pVirIoSym);
    pThisIoChannelMapping->flag.bCentroidInterpolate = isSymCentroid(pVirIoSym);
    pThisIoChannelMapping->flag.bHighPrecisionOnDual16 = bHighPrecisionOnDual16;
    pThisIoChannelMapping->flag.bInteger = bInteger;
}

static gctBOOL _CheckOutputAsStreamOutput(VIR_Shader* pShader,
                                          VIR_Symbol* pVirIoSym,
                                          gctBOOL bGross,
                                          gctUINT* thisArrayCount,
                                          gctUINT* thisArrayFirstPos,
                                          gctUINT* pVirSoSeqIndex,
                                          gctUINT* pGlobalExpandedSeqIndex,
                                          gctUINT* pLocalExpandedSeqIndex)
{
    gctUINT                    outputIdx, thisSoIoRegCount;
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
        if (*thisArrayCount > 0)
        {
            for (outputIdx = *thisArrayFirstPos; outputIdx < *thisArrayFirstPos + *thisArrayCount; outputIdx ++)
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

static void _FindStreamOutputArrayInfo(VIR_Shader* pShader,
                                       VIR_Symbol* pVirIoSym,
                                       gctUINT* thisArrayCount,
                                       gctUINT* thisArrayPos
                                       )
{
    gctUINT symIdx = 0, ioIdx = 0, soOutputCount = 0;
    gctUINT aArrayCount = 0, bArrayCount = 0, arrayPos = 0;
    VIR_Symbol * sym;

    soOutputCount = VIR_IdList_Count(pShader->transformFeedback.varyings);
    /* Search the pVirIoSym position in pShader->transformFeedback.varyings */
    for (symIdx = 0; symIdx < soOutputCount; symIdx ++)
    {
        gctUINT thisIoRegCount = 0, i;
        sym = VIR_Shader_GetSymFromId(pShader,VIR_IdList_GetId(pShader->transformFeedback.varyings, symIdx));
        thisIoRegCount = VIR_Shader_GetXFBVaryingTempRegInfo(pShader, symIdx)->tempRegCount;
        for (i = 0; i < thisIoRegCount; i++)
        {
            if (VIR_Symbol_GetVregIndex(pVirIoSym) == (VIR_Symbol_GetVregIndex(sym) + i))
            {
                break;
            }
        }
        if (i < thisIoRegCount)
        {
            break;
        }
    }

    for (ioIdx = 0; ioIdx < symIdx; ioIdx ++)
    {
        sym = VIR_Shader_GetSymFromId(pShader,VIR_IdList_GetId(pShader->transformFeedback.varyings, ioIdx));
        if (isSymbeBeEndOfInterleavedBuffer(sym))
        {
            aArrayCount = 0;
            arrayPos ++;
        }
        else
        {
            aArrayCount ++;
        }
    }

    for (ioIdx = symIdx; ioIdx < soOutputCount; ioIdx ++)
    {
        sym = VIR_Shader_GetSymFromId(pShader,VIR_IdList_GetId(pShader->transformFeedback.varyings, ioIdx));
        bArrayCount ++;
        if (isSymbeBeEndOfInterleavedBuffer(sym))
        {
            break;
        }
    }

    *thisArrayCount = aArrayCount + bArrayCount;
    *thisArrayPos = arrayPos;
}

static gctBOOL _NeedToCollectThisIO(VIR_Shader* pShader,
                                    VIR_Symbol* pSymbol
                                    )
{
    gctBOOL needToCollect = gcvTRUE;

    /* Some IO are compile-generated faked IO, we don't need to collect them. */
    if (VIR_Symbol_GetName(pSymbol) == VIR_NAME_CLUSTER_ID) {
        /* #cluster_id is not counted to input count */
        return gcvFALSE;
    }

    return needToCollect;
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
    VIR_Symbol*                pVirIoSym, *pVregSym, *sym;
    VIR_Type*                  pVirIoType;
    SHADER_IO_REG_MAPPING*     pThisIoRegMapping;
    SHADER_IO_CHANNEL_MAPPING* pThisIoChannelMapping;
    USAGE_2_IO*                pThisUsage2Io;
    SHADER_IO_USAGE            thisIoUsage;
    SHADER_IO_MODE             thisRegIoMode;
    SHADER_IO_MEM_ALIGN        ioMemAlign;
    gctBOOL                    bHasActiveModeIo = gcvFALSE, bVirIoStreamOutput, bIoStreamOutput;
    gctUINT                    thisArrayPos = 0, thisArrayCount = 0, soOutputCount = 0, symIdx = 0 ;
    gctUINT                    thisArrayFirstPos = 0, *arrayFirstPos = gcvNULL, idx = 0, pos = 0;

    virIoCount = VIR_IdList_Count(pIoIdLsts);

    /* Calculate valid io reg count */
    for (virIoIdx = 0; virIoIdx < virIoCount; virIoIdx ++)
    {
        pVirIoSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pIoIdLsts, virIoIdx));

        if (!isSymUnused(pVirIoSym) && !isSymVectorizedOut(pVirIoSym) && _NeedToCollectThisIO(pShader, pVirIoSym))
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

        /* For xfb interleaved mode (has N * "gl_NextBuffer" in varying array),
           For example:
                xfb_varyings[] = {variable_1, variable_2, gl_NextBuffer,
                                  variable_3, variable_4, variable_5, variable_6, gl_NextBuffer,
                                  variable_7};
                xfb_mode = "Interleaved Mode";

            variable_1 and variable_2 in the first array (buffer), arrayFirstPos[0] is 0.
            variable_3, variable_4, variable_5 and variable_6 in the second array (buffer), arrayFirstPos[1] is 2.
            variable_7 in a array (buffer), arrayFirstPos[2] is 6.
        */

        if (pShader->transformFeedback.varyings)
        {
            soOutputCount = VIR_IdList_Count(pShader->transformFeedback.varyings);

            if (gcoOS_Allocate(gcvNULL, gcmSIZEOF(gctUINT) * soOutputCount,
                               (gctPOINTER*)&arrayFirstPos) != gcvSTATUS_OK)
            {
                return VSC_ERR_OUT_OF_MEMORY;
            }
            gcoOS_ZeroMemory(arrayFirstPos, gcmSIZEOF(gctUINT) * soOutputCount);

            for (symIdx = 0; symIdx < soOutputCount; symIdx ++)
            {
                sym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pShader->transformFeedback.varyings, symIdx));
                if (isSymbeBeEndOfInterleavedBuffer(sym) == gcvTRUE && symIdx < (soOutputCount - 1))
                {
                    idx ++;
                    arrayFirstPos[idx] = pos + 1;
                }
                pos ++;
            }
        }

        for (virIoIdx = 0; virIoIdx < virIoCount; virIoIdx ++)
        {
            pVirIoSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pIoIdLsts, virIoIdx));
            pVirIoType = VIR_Symbol_GetType(pVirIoSym);
            thisIoRegCount = VIR_Symbol_GetVirIoRegCount(pShader, pVirIoSym);
            firstVirRegNo = VIR_Symbol_GetVregIndex(pVirIoSym);
            compCount = VIR_GetTypeComponents(pVirIoType->_base);
            bVirIoStreamOutput = bInputMapping ? gcvFALSE : _CheckOutputAsStreamOutput(pShader, pVirIoSym, gcvTRUE, &soOutputCount, &arrayFirstPos[0], gcvNULL, gcvNULL, gcvNULL);

            gcmASSERT(compCount <= CHANNEL_NUM && compCount > 0);

            if (!isSymUnused(pVirIoSym) && !isSymVectorizedOut(pVirIoSym) && _NeedToCollectThisIO(pShader, pVirIoSym))
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
                        /* For xfb interleaved mode (has N * "gl_NextBuffer" in varying array),
                           For example:
                               xfb_varyings[] = {variable_1, variable_2, gl_NextBuffer,
                                                 variable_3, variable_4, variable_5, variable_6, gl_NextBuffer,
                                                 variable_7};
                               xfb_mode = "Interleaved Mode";

                               variable_1 and variable_2 in the same array (buffer), thisArrayCount is 2.
                               variable_3, variable_4, variable_5 and variable_6 in the same array (buffer), thisArrayCount is 4.
                               variable_7 in a array (buffer), thisArrayCount is 1.

                            for "variable_5", thisArrayCount is 4, thisArrayPos is 1, thisArrayPos is point to variable_3.
                            so, in function "_CheckOutputAsStreamOutput", to find globalExpandedSoSeqIdx in the corresponding array.
                            for "variable_5", globalExpandedSoSeqIdx is 2.
                        */
                        _FindStreamOutputArrayInfo(pShader, pVregSym, &thisArrayCount, &thisArrayPos);
                        thisArrayFirstPos = arrayFirstPos[thisArrayPos];
                        bIoStreamOutput = _CheckOutputAsStreamOutput(pShader,
                                                                     pVregSym,
                                                                     gcvFALSE,
                                                                     &thisArrayCount,
                                                                     &thisArrayFirstPos,
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
                                    pThisIoRegMapping->soStreamBufferSlot = thisArrayPos;
                                    pThisIoRegMapping->soSeqInStreamBuffer = globalExpandedSoSeqIdx;
                                }
                                else
                                {
                                    if (pShader->clientApiVersion == gcvAPI_OPENGL_ES11 ||
                                        pShader->clientApiVersion == gcvAPI_OPENGL_ES20 ||
                                        pShader->clientApiVersion == gcvAPI_OPENGL_ES30 ||
                                        pShader->clientApiVersion == gcvAPI_OPENGL_ES31 ||
                                        pShader->clientApiVersion == gcvAPI_OPENGL_ES32)
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

    if (pShader->transformFeedback.varyings && arrayFirstPos)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, arrayFirstPos));
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

    _ConvertVirPerVtxPxlAndPerPrimIoList(pShader, pSepGenHelper->baseEpGen.pMM,
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

static gctBOOL _IsUniformAllocatedOnHwCnstReg(gctINT                               uniformPhysicalAddr,
                                              SHADER_CONSTANT_HW_LOCATION_MAPPING* pTargetHwLocation)
{
    if (uniformPhysicalAddr != -1 &&
        pTargetHwLocation->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER)
    {
        return (pTargetHwLocation->hwLoc.constReg.hwRegNo == (gctUINT)uniformPhysicalAddr);
    }

    return gcvFALSE;
}

static void _SetUniformHwCnstReg(gctINT                               uniformPhysicalAddr,
                                 gctUINT                              uniformPhysicalSize,
                                 SHADER_CONSTANT_HW_LOCATION_MAPPING* pTargetHwLocation)
{
    if (uniformPhysicalAddr != -1)
    {
        gcmASSERT(uniformPhysicalSize);

        pTargetHwLocation->hwLoc.constReg.hwRegNo = uniformPhysicalAddr;
        pTargetHwLocation->hwLoc.constReg.hwRegRange = uniformPhysicalSize;
        pTargetHwLocation->hwAccessMode = SHADER_HW_ACCESS_MODE_REGISTER;

        return;
    }
}

void _SetValidChannelForHwConstantLoc(SHADER_CONSTANT_HW_LOCATION_MAPPING* pCnstHwLoc, gctUINT hwChannel)
{
    pCnstHwLoc->validHWChannelMask |= (1 << hwChannel);

    if (hwChannel < pCnstHwLoc->firstValidHwChannel)
    {
        pCnstHwLoc->firstValidHwChannel = hwChannel;
    }
}

SHADER_COMPILE_TIME_CONSTANT* _EnlargeCTCRoom(SHADER_CONSTANT_MAPPING* pCnstMapping,
                                              gctUINT enlargeCTCCount,
                                              gctUINT* pStartCtcSlot)
{
    void*                         pOldCTCSet;
    gctUINT                       oldCTCCount, newCTCCount;

    pOldCTCSet = pCnstMapping->pCompileTimeConstant;
    oldCTCCount = pCnstMapping->countOfCompileTimeConstant;
    newCTCCount = oldCTCCount + enlargeCTCCount;
    if (gcoOS_Allocate(gcvNULL,
                   sizeof(SHADER_COMPILE_TIME_CONSTANT) * newCTCCount,
                   (gctPOINTER*)&pCnstMapping->pCompileTimeConstant) != gcvSTATUS_OK)
    {
        return gcvNULL;
    }

    if (pOldCTCSet)
    {
        memcpy(pCnstMapping->pCompileTimeConstant, pOldCTCSet,
               oldCTCCount*sizeof(SHADER_COMPILE_TIME_CONSTANT));

        gcoOS_Free(gcvNULL, pOldCTCSet);
    }

    pCnstMapping->countOfCompileTimeConstant = newCTCCount;

    if (pStartCtcSlot)
    {
        *pStartCtcSlot = oldCTCCount;
    }

    /* Return first enlarged CTC */
    return &pCnstMapping->pCompileTimeConstant[oldCTCCount];
}

static SHADER_CONSTANT_ARRAY_MAPPING* _GetConstantArrayMappingByArrayIdx(SHADER_CONSTANT_MAPPING* pCnstMapping, gctUINT constantArrayIndex)
{
    void*                          pOldCnstArrayMapping;
    gctUINT                        i, oldCountOfConstantArrayMapping, newCountOfConstantArrayMapping;

    if (constantArrayIndex >= pCnstMapping->countOfConstantArrayMapping)
    {
        pOldCnstArrayMapping = pCnstMapping->pConstantArrayMapping;
        oldCountOfConstantArrayMapping = pCnstMapping->countOfConstantArrayMapping;

        newCountOfConstantArrayMapping = constantArrayIndex + 1;
        if (gcoOS_Allocate(gcvNULL, newCountOfConstantArrayMapping * sizeof(SHADER_CONSTANT_ARRAY_MAPPING),
                       (gctPOINTER*)&pCnstMapping->pConstantArrayMapping) != gcvSTATUS_OK)
        {
            return gcvNULL;
        }
        pCnstMapping->countOfConstantArrayMapping = newCountOfConstantArrayMapping;

        if (pOldCnstArrayMapping)
        {
            memcpy(pCnstMapping->pConstantArrayMapping, pOldCnstArrayMapping,
                   oldCountOfConstantArrayMapping * sizeof(SHADER_CONSTANT_ARRAY_MAPPING));
            gcoOS_Free(gcvNULL, pOldCnstArrayMapping);
        }

        for (i = 0; i < newCountOfConstantArrayMapping - oldCountOfConstantArrayMapping; i ++)
        {
            vscInitializeCnstArrayMapping(pCnstMapping->pConstantArrayMapping + oldCountOfConstantArrayMapping + i);
        }
    }

    return &pCnstMapping->pConstantArrayMapping[constantArrayIndex];
}

static SHADER_CONSTANT_SUB_ARRAY_MAPPING* _enlargeCnstSubArrayMappingRoom(SHADER_CONSTANT_ARRAY_MAPPING* pConstantArrayMapping,
                                                                          gctUINT enlargeCSAMCount,
                                                                          gctUINT* pStartCSAMIdx)
{
    void*                         pOldCSAM;
    gctUINT                       oldCSAMCount, newCSAMCount;

    pOldCSAM = pConstantArrayMapping->pSubConstantArrays;
    oldCSAMCount = pConstantArrayMapping->countOfSubConstantArray;
    newCSAMCount = oldCSAMCount + enlargeCSAMCount;
    if (gcoOS_Allocate(gcvNULL,
                   sizeof(SHADER_CONSTANT_SUB_ARRAY_MAPPING) * newCSAMCount,
                   (gctPOINTER*)&pConstantArrayMapping->pSubConstantArrays) != gcvSTATUS_OK)
    {
        return gcvNULL;
    }

    if (pOldCSAM)
    {
        memcpy(pConstantArrayMapping->pSubConstantArrays, pOldCSAM,
               oldCSAMCount*sizeof(SHADER_CONSTANT_SUB_ARRAY_MAPPING));

        gcoOS_Free(gcvNULL, pOldCSAM);
    }

    pConstantArrayMapping->countOfSubConstantArray = newCSAMCount;

    if (pStartCSAMIdx)
    {
        *pStartCSAMIdx = oldCSAMCount;
    }

    /* Return first enlarged const-sub-array-mapping entry */
    return &pConstantArrayMapping->pSubConstantArrays[oldCSAMCount];
}

static VSC_ErrCode _CollectConstantMappingToSEP(VSC_SEP_GEN_HELPER* pSepGenHelper,
                                                SHADER_EXECUTABLE_PROFILE* pOutSEP,
                                                gctBOOL bPostCollection)
{
    VIR_Shader*                        pShader = pSepGenHelper->pShader;
    VIR_UniformIdList*                 pVirUniformLsts = VIR_Shader_GetUniforms(pShader);
    VIR_UBOIdList*                     pVirUboLsts = &pShader->uniformBlocks;
    VIR_Symbol*                        pVirUniformSym;
    VIR_Uniform*                       pVirUniform;
    VIR_Symbol*                        pVirUniformBlockSym;
    VIR_UniformBlock*                  pVirUniformBlock;
    VIR_Type*                          pVirUniformType;
    gctUINT                            virUniformIdx, virUboIdx, ctcSlot, subUniformIdx, llSlot;
    gctUINT                            channel, hwChannel, thisConstantArrayIndex;
    gctUINT                            thisUniformRegCount, compCount;
    gctUINT                            tartCSAMIdx;
    VIR_Const*                         pThisVirCTCVal;
    SHADER_CONSTANT_MAPPING*           pCnstMapping = &pOutSEP->constantMapping;
    SHADER_COMPILE_TIME_CONSTANT*      pThisCTC;
    gctUINT                            thisChannelValue, i, k, subConstArrayRange;
    gctINT                             thisSubUniformPhysicalAddr;
    gctINT                             uniformPhysical;
    gctUINT*                           pThisSubUniformConstValue;
    SHADER_CONSTANT_ARRAY_MAPPING*     pThisConstantArrayMapping;
    SHADER_CONSTANT_SUB_ARRAY_MAPPING* pThisCnstSubArrayMapping;
    SHADER_CONSTANT_SUB_ARRAY_MAPPING* pPrevCnstSubArrayMapping;

    /* Uniforms to HW const-reg, it is must be collected at first collection time */
    if (!bPostCollection)
    {
        for (virUniformIdx = 0; virUniformIdx < VIR_IdList_Count(pVirUniformLsts); virUniformIdx ++)
        {
            pVirUniformSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pVirUniformLsts, virUniformIdx));

            if (!isSymUniformUsedInShader(pVirUniformSym) &&
                !isSymUniformImplicitlyUsed(pVirUniformSym) &&
                !VIR_Uniform_AlwaysAlloc(pShader, pVirUniformSym))
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
            else if (VIR_Symbol_isImageT(pVirUniformSym))
            {
                pVirUniform = VIR_Symbol_GetImageT(pVirUniformSym);
            }
            else if (VIR_Symbol_isSampler(pVirUniformSym))
            {
                if (isSymUniformTreatSamplerAsConst(pVirUniformSym))
                {
                    pVirUniform = VIR_Symbol_GetSampler(pVirUniformSym);
                }
            }
            if (pVirUniform == gcvNULL)
            {
                continue;
            }

            uniformPhysical = VIR_Uniform_GetPhysical(pVirUniform);
            if (uniformPhysical == -1)
            {
                continue;
            }

            pVirUniformType = VIR_Symbol_GetType(pVirUniformSym);
            thisUniformRegCount = VIR_Type_GetVirRegCount(pShader, pVirUniformType, pVirUniform->realUseArraySize);
            compCount = VIR_GetTypeComponents(pVirUniformType->_base);

            /* HW constant register count */
            pCnstMapping->hwConstRegCount = vscMAX(pCnstMapping->hwConstRegCount, (uniformPhysical + thisUniformRegCount));
            pCnstMapping->maxHwConstRegIndex = vscMAX(pCnstMapping->maxHwConstRegIndex, (gctINT)((uniformPhysical + thisUniformRegCount - 1)));

            /* Collect CTC */
            if (isSymUniformCompiletimeInitialized(pVirUniformSym))
            {
                VIR_Type *symType = VIR_Symbol_GetType(pVirUniformSym);
                VIR_ConstId *initializerPtr;
                gctUINT  uniformIdx = 0;
                gctUINT arraySize = 1;
                gctUINT subRegCount, arrElemIdx;
                gctBOOL bLongConst;
                VIR_Type *pBaseType;
                gctUINT subUniformConstValue[CHANNEL_NUM];

                pBaseType = symType;
                if (VIR_Type_isArray(symType))
                {
                    /* To be handled for arrays */
                    arraySize = pVirUniform->realUseArraySize;
                    initializerPtr = pVirUniform->u.initializerPtr;                        ;
                    while (VIR_Type_isArray(pBaseType))
                    {
                        pBaseType = VIR_Shader_GetTypeFromId(pShader, VIR_Type_GetBaseTypeId(pBaseType));
                    }
                }
                else
                {
                    initializerPtr = &pVirUniform->u.initializer;
                }

                subRegCount = VIR_Type_GetVirRegCount(pShader, pVirUniformType, 1);
                /* Maxium is 512-bits uniform (4 HW constants) */
                gcmASSERT((subRegCount * arraySize) == thisUniformRegCount);

                bLongConst = (VIR_GetTypeSize(VIR_GetTypeComponentType(VIR_Type_GetIndex(pBaseType))) >= 8);
                for (arrElemIdx = 0; arrElemIdx < arraySize; arrElemIdx++)
                {
                    pThisVirCTCVal = (VIR_Const*)VIR_GetSymFromId(&pShader->constTable, initializerPtr[arrElemIdx]);

                    for (subUniformIdx = 0; subUniformIdx < subRegCount; subUniformIdx++, uniformIdx++)
                    {
                        pThisCTC = gcvNULL;

                        thisSubUniformPhysicalAddr = uniformPhysical + uniformIdx;
                        if (bLongConst)
                        {
                            gctUINT64 *pU64Value;

                            pU64Value = &pThisVirCTCVal->value.vecVal.u64Value[CHANNEL_NUM * subUniformIdx >> 1];
                            if (subUniformIdx & 0x1)
                            {
                                for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
                                {
                                    subUniformConstValue[channel] = (gctUINT)((pU64Value[channel] >> 32) & 0xFFFFFFFF);
                                }
                            }
                            else
                            {
                                for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
                                {
                                    subUniformConstValue[channel] = (gctUINT)(pU64Value[channel] & 0xFFFFFFFF);
                                }
                            }
                            pThisSubUniformConstValue = subUniformConstValue;
                        }
                        else
                        {
                            pThisSubUniformConstValue = &pThisVirCTCVal->value.vecVal.u32Value[CHANNEL_NUM * subUniformIdx];
                        }

                        /* Check whether there is already a CTC allocated on same HW location */
                        for (ctcSlot = 0; ctcSlot < pCnstMapping->countOfCompileTimeConstant; ctcSlot ++)
                        {
                            if (_IsUniformAllocatedOnHwCnstReg(thisSubUniformPhysicalAddr,
                                                               &pCnstMapping->pCompileTimeConstant[ctcSlot].hwConstantLocation))
                            {
                                pThisCTC = &pCnstMapping->pCompileTimeConstant[ctcSlot];
                                break;
                            }
                        }

                        /* Enlarge CTC set to make room for current CTC */
                        if (pThisCTC == gcvNULL)
                        {
                            pThisCTC = _EnlargeCTCRoom(pCnstMapping, 1, &ctcSlot);
                            if (pThisCTC == gcvNULL)
                            {
                                return VSC_ERR_OUT_OF_MEMORY;
                            }
                            vscInitializeCTC(pThisCTC);
                            _SetUniformHwCnstReg(thisSubUniformPhysicalAddr, 1, &pThisCTC->hwConstantLocation);
                        }

                        /* Copy CTC now */
                        for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
                        {
                            if (channel >= (gctUINT)(VIR_GetTypeComponents(pThisVirCTCVal->type)))
                            {
                                break;
                            }

                            hwChannel = (((pVirUniform->swizzle) >> ((channel) * 2)) & 0x3);
                            thisChannelValue = *(gctUINT*)&pThisSubUniformConstValue[channel];

                            if (pThisCTC->hwConstantLocation.validHWChannelMask & (1 << hwChannel))
                            {
                                gcmASSERT(thisChannelValue == pThisCTC->constantValue[hwChannel]);
                            }
                            else
                            {
                                _SetValidChannelForHwConstantLoc(&pThisCTC->hwConstantLocation, hwChannel);
                                pThisCTC->constantValue[hwChannel] = thisChannelValue;
                            }
                        }

                        if (subUniformIdx == 0)
                        {
                            VIR_Symbol_SetFirstSlot(pVirUniformSym, ctcSlot);
                        }
                    }
                }
            }
            else
            {
                subConstArrayRange = (VIR_Symbol_isImage(pVirUniformSym)) ? 1 : thisUniformRegCount;
                for (k = 0; k < thisUniformRegCount; k += subConstArrayRange)
                {
                    thisConstantArrayIndex = 0;
                    pThisConstantArrayMapping = _GetConstantArrayMappingByArrayIdx(pCnstMapping, thisConstantArrayIndex);
                    if (pThisConstantArrayMapping == gcvNULL)
                    {
                        return VSC_ERR_OUT_OF_MEMORY;
                    }
                    pCnstMapping->arrayIndexMask |= (1 << thisConstantArrayIndex);
                    pThisConstantArrayMapping->constantArrayIndex = thisConstantArrayIndex;

                    pThisConstantArrayMapping->arrayRange += subConstArrayRange;

                    if (!(pShader->clientApiVersion == gcvAPI_D3D && pShader->compilerVersion[1] == _SHADER_DX_VERSION_30))
                    {
                        pThisConstantArrayMapping->constantUsage = SHADER_CONSTANT_USAGE_MIXED;
                    }

                    pThisCnstSubArrayMapping = _enlargeCnstSubArrayMappingRoom(pThisConstantArrayMapping, 1, &tartCSAMIdx);
                    if (pThisCnstSubArrayMapping == gcvNULL)
                    {
                        return VSC_ERR_OUT_OF_MEMORY;
                    }
                    vscInitializeCnstSubArrayMapping(pThisCnstSubArrayMapping);
                    if (pShader->clientApiVersion == gcvAPI_D3D)
                    {
                    }
                    else
                    {
                        pThisCnstSubArrayMapping->firstMSCSharpRegNo = NOT_ASSIGNED;
                    }
                    if (pThisConstantArrayMapping->pSubConstantArrays == pThisCnstSubArrayMapping)
                    {
                        pThisCnstSubArrayMapping->startIdx = 0;
                    }
                    else
                    {
                        pPrevCnstSubArrayMapping = pThisCnstSubArrayMapping - 1;
                        pThisCnstSubArrayMapping->startIdx = pPrevCnstSubArrayMapping->startIdx + pPrevCnstSubArrayMapping->subArrayRange;
                    }
                    pThisCnstSubArrayMapping->subArrayRange = subConstArrayRange;
                    for (i = 0; i < compCount; i ++)
                    {
                        pThisCnstSubArrayMapping->validChannelMask |= (1 << i);
                    }

                    _SetUniformHwCnstReg((uniformPhysical + k), subConstArrayRange, &pThisCnstSubArrayMapping->hwFirstConstantLocation);
                    for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
                    {
                        hwChannel = (((pVirUniform->swizzle) >> ((channel) * 2)) & 0x3);
                        _SetValidChannelForHwConstantLoc(&pThisCnstSubArrayMapping->hwFirstConstantLocation, hwChannel);
                    }

                    if (k == 0)
                    {
                        VIR_Symbol_SetArraySlot(pVirUniformSym, thisConstantArrayIndex);
                        VIR_Symbol_SetFirstSlot(pVirUniformSym, tartCSAMIdx);
                    }
                }
            }
        }
    }

    /* Uniforms to spilled mem */
    if (pShader->hasCRegSpill)
    {
        for (virUboIdx = 0; virUboIdx < VIR_IdList_Count(pVirUboLsts); virUboIdx ++)
        {
            pVirUniformBlockSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pVirUboLsts, virUboIdx));
            pVirUniformBlock = VIR_Symbol_GetUBO(pVirUniformBlockSym);

            if (pVirUniformBlock->flags & VIR_IB_WITH_CUBO)
            {
                for (virUniformIdx = 0; virUniformIdx < pVirUniformBlock->uniformCount; virUniformIdx ++)
                {
                    pVirUniform = pVirUniformBlock->uniforms[virUniformIdx];
                    pVirUniformSym = VIR_Shader_GetSymFromId(pShader, pVirUniform->sym);
                    pVirUniformType = VIR_Symbol_GetType(pVirUniformSym);
                    thisUniformRegCount = VIR_Type_GetVirRegCount(pShader, pVirUniformType, pVirUniform->realUseArraySize);

                    if (isSymUniformCompiletimeInitialized(pVirUniformSym))
                    {
                        if (!bPostCollection)
                        {
                            /* deal array in uniform block */
                            VIR_ConstId *initializerPtr;
                            gctUINT arraySize = 1, arrElemIdx, baseTypeSize;
                            VIR_Type *pBaseType = pVirUniformType;
                            if(VIR_Type_isArray(pBaseType)) {
                                /* To be handled for arrays */
                                arraySize = pVirUniform->realUseArraySize;
                                initializerPtr = pVirUniform->u.initializerPtr;
                                while (VIR_Type_isArray(pBaseType))
                                {
                                    pBaseType = VIR_Shader_GetTypeFromId(pShader, VIR_Type_GetBaseTypeId(pBaseType));
                                }
                            }
                            else
                            {
                                initializerPtr = &pVirUniform->u.initializer;
                            }
                            baseTypeSize = VIR_GetTypeSize(VIR_GetTypeComponentType(VIR_Type_GetIndex(pBaseType)));
                            for (arrElemIdx = 0; arrElemIdx < arraySize; arrElemIdx++)
                            {
                                pThisVirCTCVal = (VIR_Const*)VIR_GetSymFromId(&pShader->constTable, initializerPtr[arrElemIdx]);

                                /* Enlarge CTC set to make room for current CTC */
                                pThisCTC = _EnlargeCTCRoom(pCnstMapping, 1, &ctcSlot);
                                if (pThisCTC == gcvNULL)
                                {
                                    return VSC_ERR_OUT_OF_MEMORY;
                                }
                                vscInitializeCTC(pThisCTC);
                                pThisCTC->hwConstantLocation.hwAccessMode = SHADER_HW_ACCESS_MODE_MEMORY;
                                pThisCTC->hwConstantLocation.hwLoc.memAddr.hwMemAccessMode = SHADER_HW_MEM_ACCESS_MODE_UAV;
                                pThisCTC->hwConstantLocation.hwLoc.memAddr.constantOffsetKind = SHADER_CONSTANT_OFFSET_IN_BYTE;
                                pThisCTC->hwConstantLocation.hwLoc.memAddr.constantOffset = VIR_Uniform_GetOffset(pVirUniform) + arrElemIdx * baseTypeSize;
                                pThisCTC->hwConstantLocation.hwLoc.memAddr.componentSizeInByte = baseTypeSize;

                                /* Copy CTC now */
                                for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
                                {
                                    if (channel >= (gctUINT)(VIR_GetTypeComponents(pThisVirCTCVal->type)))
                                    {
                                        break;
                                    }
                                    hwChannel = channel;
                                    _SetValidChannelForHwConstantLoc(&pThisCTC->hwConstantLocation, hwChannel);
                                    thisChannelValue = *(gctUINT*)&pThisVirCTCVal->value.vecVal.u32Value[channel];
                                    pThisCTC->constantValue[hwChannel] = thisChannelValue;
                                }
                                if (arrElemIdx == 0)
                                {
                                    VIR_Symbol_SetFirstSlot(pVirUniformSym, ctcSlot);
                                }
                            }
                        }
                        else
                        {
                            ctcSlot = VIR_Symbol_GetFirstSlot(pVirUniformSym);
                            llSlot = VIR_Symbol_GetFirstSlot(pVirUniformBlockSym);
                            pThisCTC = &pCnstMapping->pCompileTimeConstant[ctcSlot];
                            pThisCTC->hwConstantLocation.hwLoc.memAddr.memBase.pUav = &pOutSEP->uavMapping.pUAV[llSlot];
                        }
                    }
                    else
                    {
                        gcmASSERT(gcvFALSE);
                    }
                }

                break;
            }
        }
    }

    return VSC_ERR_NONE;
}

static SHADER_SAMPLER_SLOT_MAPPING* _enlargeSamplerSlotMappingRoom(SHADER_SAMPLER_MAPPING* pSamplerMapping,
                                                                   gctUINT enlargeSamplerSlotCount,
                                                                   gctUINT* pStartSamplerSlotIdx)
{
    void*                         pOldSamplerSlot;
    gctUINT                       oldSamplerSlotCount, newSamplerSlotCount;

    pOldSamplerSlot = pSamplerMapping->pSampler;
    oldSamplerSlotCount = pSamplerMapping->countOfSamplers;
    newSamplerSlotCount = oldSamplerSlotCount + enlargeSamplerSlotCount;
    if (gcoOS_Allocate(gcvNULL,
                   sizeof(SHADER_SAMPLER_SLOT_MAPPING) * newSamplerSlotCount,
                   (gctPOINTER*)&pSamplerMapping->pSampler) != gcvSTATUS_OK)
    {
        return gcvNULL;
    }

    if (pOldSamplerSlot)
    {
        memcpy(pSamplerMapping->pSampler, pOldSamplerSlot,
               oldSamplerSlotCount*sizeof(SHADER_SAMPLER_SLOT_MAPPING));

        gcoOS_Free(gcvNULL, pOldSamplerSlot);
    }

    pSamplerMapping->countOfSamplers = newSamplerSlotCount;

    if (pStartSamplerSlotIdx)
    {
        *pStartSamplerSlotIdx = oldSamplerSlotCount;
    }

    /* Return first enlarged sampler-slot-mapping entry */
    return &pSamplerMapping->pSampler[oldSamplerSlotCount];
}

static VSC_ErrCode _CollectSamplerMappingToSEP(VSC_SEP_GEN_HELPER* pSepGenHelper,
                                               SHADER_EXECUTABLE_PROFILE* pOutSEP)
{
    VSC_ErrCode                        errCode = VSC_ERR_NONE;
    VIR_Shader*                        pShader = pSepGenHelper->pShader;
    VIR_UniformIdList*                 pVirUniformLsts = VIR_Shader_GetUniforms(pShader);
    VIR_Symbol*                        pVirUniformSym;
    VIR_Uniform*                       pVirUniformSampler;
    VIR_Type*                          pVirUniformSymType;
    gctUINT                            samplerRegCount;
    gctUINT                            virUniformIdx, subUniformIdx, startSamplerSlotIdx;
    gctUINT                            thisSamplerSlotIndex;
    SHADER_SAMPLER_MAPPING*            pSamplerMapping = &pOutSEP->samplerMapping;
    SHADER_SAMPLER_SLOT_MAPPING*       pThisSamplerSlotMapping;
    SHADER_SAMPLER_SLOT_MAPPING*       pBaseSamplerSlotMapping;

    for (virUniformIdx = 0; virUniformIdx < VIR_IdList_Count(pVirUniformLsts); virUniformIdx ++)
    {
        pVirUniformSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pVirUniformLsts, virUniformIdx));

        if (!VIR_Symbol_isSampler(pVirUniformSym) ||
            (VIR_Symbol_GetIndex(pVirUniformSym) == VIR_Shader_GetBaseSamplerId(pShader)))
        {
            continue;
        }

        /* If this texture is not used on shader, we can skip it. */
        if (!isSymUniformUsedInShader(pVirUniformSym) &&
            !isSymUniformUsedInTextureSize(pVirUniformSym) &&
            !isSymUniformUsedInLTC(pVirUniformSym) &&
            !VIR_Uniform_AlwaysAlloc(pShader, pVirUniformSym))
        {
            continue;
        }

        pVirUniformSampler = VIR_Symbol_GetSampler(pVirUniformSym);
        if (pVirUniformSampler == gcvNULL)
        {
            continue;
        }

        pBaseSamplerSlotMapping = _enlargeSamplerSlotMappingRoom(pSamplerMapping,
                                                                 pVirUniformSampler->realUseArraySize,
                                                                 &startSamplerSlotIdx);
        if(pBaseSamplerSlotMapping == gcvNULL)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
        for (subUniformIdx = 0; subUniformIdx < (gctUINT)pVirUniformSampler->realUseArraySize; subUniformIdx ++)
        {
            thisSamplerSlotIndex = startSamplerSlotIdx + subUniformIdx;
            pSamplerMapping->samplerSlotMask |= (1 << thisSamplerSlotIndex);

            pThisSamplerSlotMapping = pBaseSamplerSlotMapping + subUniformIdx;
            vscInitializeSamplerSlotMapping(pThisSamplerSlotMapping);

            pThisSamplerSlotMapping->samplerSlotIndex = thisSamplerSlotIndex;
            pThisSamplerSlotMapping->hwSamplerSlot = pVirUniformSampler->physical + subUniformIdx;
        }
        pVirUniformSymType = VIR_Symbol_GetType(pVirUniformSym);
        if (VIR_Type_GetKind(pVirUniformSymType) == VIR_TY_ARRAY)
        {
            samplerRegCount = VIR_Type_GetArrayLength(pVirUniformSymType);
        }
        else
        {
            samplerRegCount = 1;
        }
        pSamplerMapping->hwSamplerRegCount += samplerRegCount;
        pSamplerMapping->maxHwSamplerRegIndex = vscMAX(pSamplerMapping->maxHwSamplerRegIndex,
                                                       (gctINT)((VIR_Uniform_GetPhysical(pVirUniformSampler) + samplerRegCount - 1)));

        VIR_Symbol_SetFirstSlot(pVirUniformSym, startSamplerSlotIdx);
    }

    return errCode;
}

static VSC_ErrCode _CollectSrvMappingToSEP(VSC_SEP_GEN_HELPER* pSepGenHelper,
                                           SHADER_EXECUTABLE_PROFILE* pOutSEP)
{
    VSC_ErrCode      errCode = VSC_ERR_NONE;

    return errCode;
}

static SHADER_UAV_SLOT_MAPPING* _enlargeUavSlotMappingRoom(SHADER_UAV_MAPPING* pUavMapping,
                                                           gctUINT enlargeUavSlotCount,
                                                           gctUINT* pStartUavSlotIdx)
{
    void*                         pOldUavSlot;
    gctUINT                       oldUavSlotCount, newUavSlotCount;

    pOldUavSlot = pUavMapping->pUAV;
    oldUavSlotCount = pUavMapping->countOfUAVs;
    newUavSlotCount = oldUavSlotCount + enlargeUavSlotCount;
    if (gcoOS_Allocate(gcvNULL,
                   sizeof(SHADER_UAV_SLOT_MAPPING) * newUavSlotCount,
                   (gctPOINTER*)&pUavMapping->pUAV) != gcvSTATUS_OK)
    {
        return gcvNULL;
    }

    if (pOldUavSlot)
    {
        memcpy(pUavMapping->pUAV, pOldUavSlot,
               oldUavSlotCount*sizeof(SHADER_UAV_SLOT_MAPPING));

        gcoOS_Free(gcvNULL, pOldUavSlot);
    }

    pUavMapping->countOfUAVs = newUavSlotCount;

    if (pStartUavSlotIdx)
    {
        *pStartUavSlotIdx = oldUavSlotCount;
    }

    /* Return first enlarged uav-slot-mapping entry */
    return &pUavMapping->pUAV[oldUavSlotCount];
}

static VSC_ErrCode _CollectUavMappingToSEP(VSC_SEP_GEN_HELPER* pSepGenHelper,
                                           SHADER_EXECUTABLE_PROFILE* pOutSEP,
                                           gctBOOL bPostCollection)
{
    VSC_ErrCode                   errCode = VSC_ERR_NONE;
    VIR_Shader*                   pShader = pSepGenHelper->pShader;
    gctUINT                       startUavSlotIdx;
    SHADER_UAV_MAPPING*           pUavMapping = &pOutSEP->uavMapping;
    SHADER_UAV_SLOT_MAPPING*      pThisUavSlotMapping;
    VIR_UniformIdList*            pVirUniformLsts = VIR_Shader_GetUniforms(pShader);
    VIR_Symbol*                   pVirUniformSym;
    VIR_Uniform*                  pVirUniform;
    VIR_UBOIdList*                pVirUboLsts = &pShader->uniformBlocks;
    VIR_SBOIdList*                pVirSboLsts = &pShader->storageBlocks;
    VIR_Symbol*                   pVirUniformBlockSym;
    VIR_UniformBlock*             pVirUniformBlock;
    VIR_Symbol*                   pVirStorageBlockSym;
    VIR_StorageBlock*             pVirStorageBlock;
    gctUINT                       virUboIdx, virSboIdx, virUniformIdx, llArraySlot;
    gctUINT                       llSlot, thisUniformRegCount, i;

    /* Normal UAVs */
    for (virUniformIdx = 0; virUniformIdx < VIR_IdList_Count(pVirUniformLsts); virUniformIdx ++)
    {
        pVirUniformSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pVirUniformLsts, virUniformIdx));

        if (VIR_Symbol_isImage(pVirUniformSym) || VIR_Symbol_isImageT(pVirUniformSym))
        {
            pVirUniform = VIR_Symbol_GetUniformPointer(pShader, pVirUniformSym);

            if (pVirUniform && pVirUniform->physical == -1)
            {
                continue;
            }

            thisUniformRegCount = VIR_Type_GetVirRegCount(pShader, VIR_Symbol_GetType(pVirUniformSym), pVirUniform->realUseArraySize);
            for (i = 0; i < thisUniformRegCount; i ++)
            {
                if (!bPostCollection)
                {
                    pThisUavSlotMapping = _enlargeUavSlotMappingRoom(pUavMapping, 1, &startUavSlotIdx);
                    if (pThisUavSlotMapping == gcvNULL)
                    {
                        return VSC_ERR_OUT_OF_MEMORY;
                    }
                    vscInitializeUavSlotMapping(pThisUavSlotMapping);

                    pUavMapping->uavSlotMask |= (1 << startUavSlotIdx);

                    pThisUavSlotMapping->uavSlotIndex = startUavSlotIdx;
                    pThisUavSlotMapping->accessMode = SHADER_UAV_ACCESS_MODE_TYPE;
                    pThisUavSlotMapping->u.s.uavDimension = SHADER_UAV_DIMENSION_UNKNOWN;
                    if (VIR_Symbol_GetLlResSlot(pVirUniformSym) == NOT_ASSIGNED)
                    {
                        VIR_Symbol_SetLlResSlot(pVirUniformSym, startUavSlotIdx);
                    }
                }
                else
                {
                    pThisUavSlotMapping = &pOutSEP->uavMapping.pUAV[VIR_Symbol_GetLlResSlot(pVirUniformSym) + i];

                    llArraySlot = VIR_Symbol_GetArraySlot(pVirUniformSym);
                    llSlot = VIR_Symbol_GetFirstSlot(pVirUniformSym);

                    gcmASSERT(llArraySlot != NOT_ASSIGNED && llSlot != NOT_ASSIGNED);

                    pThisUavSlotMapping->hwMemAccessMode = SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR;
                    pThisUavSlotMapping->hwLoc.pHwDirectAddrBase = &pOutSEP->constantMapping.pConstantArrayMapping[llArraySlot].
                                                                    pSubConstantArrays[llSlot + i].hwFirstConstantLocation;
                }
            }
        }
    }

    /* Gpr spillage memory */
    if (pShader->hasRegisterSpill)
    {
        gcmASSERT(pShader->vidmemSizeOfSpill > 0);

        if (!bPostCollection)
        {
            pThisUavSlotMapping = _enlargeUavSlotMappingRoom(pUavMapping, 1, &startUavSlotIdx);
            if (pThisUavSlotMapping == gcvNULL)
            {
                return VSC_ERR_OUT_OF_MEMORY;
            }
            vscInitializeUavSlotMapping(pThisUavSlotMapping);

            pUavMapping->uavSlotMask |= (1 << startUavSlotIdx);

            pThisUavSlotMapping->uavSlotIndex = startUavSlotIdx;
            pThisUavSlotMapping->accessMode = SHADER_UAV_ACCESS_MODE_RAW;
            pThisUavSlotMapping->sizeInByte = pShader->vidmemSizeOfSpill;
            pThisUavSlotMapping->u.s.uavDimension = SHADER_UAV_DIMENSION_BUFFER;

            pShader->llSlotForSpillVidmem = startUavSlotIdx;
        }
        else
        {
            pThisUavSlotMapping = &pOutSEP->uavMapping.pUAV[pShader->llSlotForSpillVidmem];

            for (virUniformIdx = 0; virUniformIdx < VIR_IdList_Count(pVirUniformLsts); virUniformIdx ++)
            {
                pVirUniformSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pVirUniformLsts, virUniformIdx));
                if (VIR_Symbol_GetUniformKind(pVirUniformSym) == VIR_UNIFORM_TEMP_REG_SPILL_MEM_ADDRESS)
                {
                    gcmASSERT(VIR_Symbol_isUniform(pVirUniformSym));

                    llArraySlot = VIR_Symbol_GetArraySlot(pVirUniformSym);
                    llSlot = VIR_Symbol_GetFirstSlot(pVirUniformSym);

                    gcmASSERT(llArraySlot != NOT_ASSIGNED && llSlot != NOT_ASSIGNED);

                    pThisUavSlotMapping->hwMemAccessMode = SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR;
                    pThisUavSlotMapping->hwLoc.pHwDirectAddrBase = &pOutSEP->constantMapping.pConstantArrayMapping[llArraySlot].
                                                                   pSubConstantArrays[llSlot].hwFirstConstantLocation;

                    break;
                }
            }
        }
    }

    /* Cr spillage memory */
    if (pShader->hasCRegSpill)
    {
        for (virUboIdx = 0; virUboIdx < VIR_IdList_Count(pVirUboLsts); virUboIdx ++)
        {
            pVirUniformBlockSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pVirUboLsts, virUboIdx));
            pVirUniformBlock = VIR_Symbol_GetUBO(pVirUniformBlockSym);

            if (pVirUniformBlock->flags & VIR_IB_WITH_CUBO)
            {
                if (!bPostCollection)
                {
                    pThisUavSlotMapping = _enlargeUavSlotMappingRoom(pUavMapping, 1, &startUavSlotIdx);
                    if (pThisUavSlotMapping == gcvNULL)
                    {
                        return VSC_ERR_OUT_OF_MEMORY;
                    }
                    vscInitializeUavSlotMapping(pThisUavSlotMapping);

                    pUavMapping->uavSlotMask |= (1 << startUavSlotIdx);

                    pThisUavSlotMapping->uavSlotIndex = startUavSlotIdx;
                    pThisUavSlotMapping->accessMode = SHADER_UAV_ACCESS_MODE_RAW;
                    pThisUavSlotMapping->u.s.uavDimension = SHADER_UAV_DIMENSION_BUFFER;

                    pThisUavSlotMapping->sizeInByte = pVirUniformBlock->blockSize;

                    VIR_Symbol_SetFirstSlot(pVirUniformBlockSym, startUavSlotIdx);
                }
                else
                {
                    llSlot = VIR_Symbol_GetFirstSlot(pVirUniformBlockSym);
                    gcmASSERT(llSlot != NOT_ASSIGNED);
                    pThisUavSlotMapping = &pOutSEP->uavMapping.pUAV[llSlot];

                    pVirUniformSym = VIR_Shader_GetSymFromId(pShader, pVirUniformBlock->baseAddr);
                    gcmASSERT(VIR_Symbol_isUniform(pVirUniformSym));
                    gcmASSERT(VIR_Symbol_GetUniformKind(pVirUniformSym) == VIR_UNIFORM_UNIFORM_BLOCK_ADDRESS);

                    llArraySlot = VIR_Symbol_GetArraySlot(pVirUniformSym);
                    llSlot = VIR_Symbol_GetFirstSlot(pVirUniformSym);

                    gcmASSERT(llArraySlot != NOT_ASSIGNED && llSlot != NOT_ASSIGNED);

                    pThisUavSlotMapping->hwMemAccessMode = SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR;
                    pThisUavSlotMapping->hwLoc.pHwDirectAddrBase = &pOutSEP->constantMapping.pConstantArrayMapping[llArraySlot].
                                                                   pSubConstantArrays[llSlot].hwFirstConstantLocation;
                }

                break;
            }
        }
    }

    /* Share memory simulated by global memory */
    for (virSboIdx = 0; virSboIdx < VIR_IdList_Count(pVirSboLsts); virSboIdx ++)
    {
        pVirStorageBlockSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pVirSboLsts, virSboIdx));
        pVirStorageBlock = VIR_Symbol_GetSBO(pVirStorageBlockSym);

        if (pVirStorageBlock->flags & VIR_IB_FOR_SHARED_VARIABLE)
        {
            /* Skip if this SBO is not used in shader. */
            pVirUniformSym = VIR_Shader_GetSymFromId(pShader, pVirStorageBlock->baseAddr);
            gcmASSERT(VIR_Symbol_isUniform(pVirUniformSym));
            gcmASSERT(VIR_Symbol_GetUniformKind(pVirUniformSym) == VIR_UNIFORM_STORAGE_BLOCK_ADDRESS);

            if (!isSymUniformUsedInShader(pVirUniformSym) &&
                !isSymUniformImplicitlyUsed(pVirUniformSym))
            {
                break;
            }

            if (!bPostCollection)
            {
                pThisUavSlotMapping = _enlargeUavSlotMappingRoom(pUavMapping, 1, &startUavSlotIdx);
                if (pThisUavSlotMapping == gcvNULL)
                {
                    return VSC_ERR_OUT_OF_MEMORY;
                }
                vscInitializeUavSlotMapping(pThisUavSlotMapping);

                pUavMapping->uavSlotMask |= (1 << startUavSlotIdx);

                pThisUavSlotMapping->uavSlotIndex = startUavSlotIdx;
                pThisUavSlotMapping->accessMode = SHADER_UAV_ACCESS_MODE_RAW;
                pThisUavSlotMapping->u.s.uavDimension = SHADER_UAV_DIMENSION_BUFFER;

                pThisUavSlotMapping->sizeInByte = pVirStorageBlock->blockSize;

                VIR_Symbol_SetFirstSlot(pVirStorageBlockSym, startUavSlotIdx);
            }
            else
            {
                llSlot = VIR_Symbol_GetFirstSlot(pVirStorageBlockSym);
                gcmASSERT(llSlot != NOT_ASSIGNED);
                pThisUavSlotMapping = &pOutSEP->uavMapping.pUAV[llSlot];

                llArraySlot = VIR_Symbol_GetArraySlot(pVirUniformSym);
                llSlot = VIR_Symbol_GetFirstSlot(pVirUniformSym);

                gcmASSERT(llArraySlot != NOT_ASSIGNED && llSlot != NOT_ASSIGNED);

                pThisUavSlotMapping->hwMemAccessMode = SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR;
                pThisUavSlotMapping->hwLoc.pHwDirectAddrBase = &pOutSEP->constantMapping.pConstantArrayMapping[llArraySlot].
                                                                pSubConstantArrays[llSlot].hwFirstConstantLocation;
            }

            break;
        }
    }

    /*
    ** 1) Share memory simulated by global memory for OCL.
    ** 2) The global memory to save the thread ID.
    */
    for (virUniformIdx = 0; virUniformIdx < VIR_IdList_Count(pVirUniformLsts); virUniformIdx ++)
    {
        pVirUniformSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pVirUniformLsts, virUniformIdx));
        pVirUniform = VIR_Symbol_GetUniform(pVirUniformSym);

        if (pVirUniform == gcvNULL ||
            pVirUniform->physical == -1 ||
            !isSymUniformUsedInShader(pVirUniformSym))
        {
            continue;
        }

        if (strcmp(VIR_Shader_GetSymNameString(pShader, pVirUniformSym), _sldLocalStorageAddressName) == 0)
        {
            if (!bPostCollection)
            {
                pThisUavSlotMapping = _enlargeUavSlotMappingRoom(pUavMapping, 1, &startUavSlotIdx);
                if (pThisUavSlotMapping == gcvNULL)
                {
                    return VSC_ERR_OUT_OF_MEMORY;
                }
                vscInitializeUavSlotMapping(pThisUavSlotMapping);

                pUavMapping->uavSlotMask |= (1 << startUavSlotIdx);

                pThisUavSlotMapping->uavSlotIndex = startUavSlotIdx;
                pThisUavSlotMapping->accessMode = SHADER_UAV_ACCESS_MODE_TYPE;
                pThisUavSlotMapping->u.s.uavDimension = SHADER_UAV_DIMENSION_BUFFER;
                pThisUavSlotMapping->sizeInByte = pOutSEP->exeHints.nativeHints.prvStates.gps.shareMemSizePerThreadGrpInByte;

                if (VIR_Symbol_GetLlResSlot(pVirUniformSym) == NOT_ASSIGNED)
                {
                    VIR_Symbol_SetLlResSlot(pVirUniformSym, startUavSlotIdx);
                }
            }
            else
            {
                pThisUavSlotMapping = &pOutSEP->uavMapping.pUAV[VIR_Symbol_GetLlResSlot(pVirUniformSym)];

                llArraySlot = VIR_Symbol_GetArraySlot(pVirUniformSym);
                llSlot = VIR_Symbol_GetFirstSlot(pVirUniformSym);

                gcmASSERT(llArraySlot != NOT_ASSIGNED && llSlot != NOT_ASSIGNED);

                pThisUavSlotMapping->hwMemAccessMode = SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR;
                pThisUavSlotMapping->hwLoc.pHwDirectAddrBase = &pOutSEP->constantMapping.pConstantArrayMapping[llArraySlot].
                                                                pSubConstantArrays[llSlot].hwFirstConstantLocation;
            }
        }
        else if (VIR_Symbol_GetUniformKind(pVirUniformSym) == VIR_UNIFORM_THREAD_ID_MEM_ADDR)
        {
            if (!bPostCollection)
            {
                pThisUavSlotMapping = _enlargeUavSlotMappingRoom(pUavMapping, 1, &startUavSlotIdx);
                if (pThisUavSlotMapping == gcvNULL)
                {
                    return VSC_ERR_OUT_OF_MEMORY;
                }
                vscInitializeUavSlotMapping(pThisUavSlotMapping);

                pUavMapping->uavSlotMask |= (1 << startUavSlotIdx);

                pThisUavSlotMapping->uavSlotIndex = startUavSlotIdx;
                pThisUavSlotMapping->accessMode = SHADER_UAV_ACCESS_MODE_TYPE;
                pThisUavSlotMapping->u.s.uavDimension = SHADER_UAV_DIMENSION_BUFFER;
                /* We only need 4 bytes to save the threadID. */
                pThisUavSlotMapping->sizeInByte = 4;

                if (VIR_Symbol_GetLlResSlot(pVirUniformSym) == NOT_ASSIGNED)
                {
                    VIR_Symbol_SetLlResSlot(pVirUniformSym, startUavSlotIdx);
                }
            }
            else
            {
                pThisUavSlotMapping = &pOutSEP->uavMapping.pUAV[VIR_Symbol_GetLlResSlot(pVirUniformSym)];

                llArraySlot = VIR_Symbol_GetArraySlot(pVirUniformSym);
                llSlot = VIR_Symbol_GetFirstSlot(pVirUniformSym);

                gcmASSERT(llArraySlot != NOT_ASSIGNED && llSlot != NOT_ASSIGNED);

                pThisUavSlotMapping->hwMemAccessMode = SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR;
                pThisUavSlotMapping->hwLoc.pHwDirectAddrBase = &pOutSEP->constantMapping.pConstantArrayMapping[llArraySlot].
                                                                pSubConstantArrays[llSlot].hwFirstConstantLocation;
            }
        }
    }

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

static void _CollectExeHints(VSC_SHADER_COMPILER_PARAM* pCompilerParam, VSC_SEP_GEN_HELPER* pSepGenHelper, SHADER_EXECUTABLE_PROFILE* pOutSEP)
{
    VIR_Shader*                 pShader = pSepGenHelper->pShader;
    gctUINT                     i;

    /* 1. Native hints */

    pOutSEP->exeHints.nativeHints.globalStates.bSeparatedShader = (pShader->clientApiVersion == gcvAPI_D3D) ?
                                                                           gcvTRUE : VIR_Shader_IsSeparated(pShader);
    pOutSEP->exeHints.nativeHints.globalStates.bLinkProgramPipeline = (pCompilerParam->cfg.cFlags & VSC_COMPILER_FLAG_LINK_PROGRAM_PIPELINE_OBJ);
    pOutSEP->exeHints.nativeHints.globalStates.memoryAccessHint = (SHADER_EDH_MEM_ACCESS_HINT)pShader->memoryAccessFlag[VIR_SHLEVEL_Pre_Low];
    pOutSEP->exeHints.nativeHints.globalStates.flowControlHint = (SHADER_EDH_FLOW_CONTROL_HINT)pShader->flowControlFlag[VIR_SHLEVEL_Pre_Low];
    pOutSEP->exeHints.nativeHints.globalStates.texldHint = (SHADER_EDH_TEXLD_HINT)pShader->texldFlag[VIR_SHLEVEL_Pre_Low];


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
        pOutSEP->exeHints.nativeHints.prvStates.ts.hasNoPerVertexInput = VIR_Shader_HasNoPerVertexInput(pShader);

        pOutSEP->exeHints.nativeHints.prvStates.ts.inputCtrlPointCount =
                                                  pShader->shaderLayout.tcs.tcsPatchInputVertices;

        pOutSEP->exeHints.nativeHints.prvStates.ts.outputCtrlPointCount =
                                                  pShader->shaderLayout.tcs.tcsOutputVertexCount == 0 ? 1 : pShader->shaderLayout.tcs.tcsOutputVertexCount;
    }

    if (pShader->shaderKind == VIR_SHADER_TESSELLATION_EVALUATION)
    {
        /* So far this variable is not used, we only use the inputCtrlPointCount of TCS to program. */
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

        pOutSEP->exeHints.nativeHints.prvStates.gs.bHasStreamOut = VIR_Shader_GS_HasStreamOut(pShader);
    }

    if (pShader->shaderKind == VIR_SHADER_COMPUTE)
    {
        /* Private memory set, only support OCL now. */
        if (VIR_Shader_IsCLFromLanguage(pShader))
        {
            pOutSEP->exeHints.nativeHints.prvStates.gps.privMemSizePerThreadInByte = VIR_Shader_GetPrivateMemorySize(pShader);
            pOutSEP->exeHints.nativeHints.prvStates.gps.currWorkThreadNum = VIR_Shader_GetCurrWorkThreadNum(pShader);
            if (pOutSEP->exeHints.nativeHints.prvStates.gps.privMemSizePerThreadInByte > 0)
            {
                gcmASSERT(pOutSEP->exeHints.nativeHints.prvStates.gps.currWorkThreadNum > 0);
            }
        }

        pOutSEP->exeHints.nativeHints.prvStates.gps.shareMemSizePerThreadGrpInByte = VIR_Shader_GetShareMemorySize(pShader);
        pOutSEP->exeHints.nativeHints.prvStates.gps.currWorkGrpNum = VIR_Shader_GetCurrWorkGrpNum(pShader);
        if (pOutSEP->exeHints.nativeHints.prvStates.gps.shareMemSizePerThreadGrpInByte > 0)
        {
            gcmASSERT(pOutSEP->exeHints.nativeHints.prvStates.gps.currWorkGrpNum > 0);
        }

        pOutSEP->exeHints.nativeHints.prvStates.gps.workGroupNumPerShaderGroup = VIR_Shader_GetWorkGroupNumPerShaderGroup(pShader);

        pOutSEP->exeHints.nativeHints.prvStates.gps.threadGrpDimX = pShader->shaderLayout.compute.workGroupSize[0];
        pOutSEP->exeHints.nativeHints.prvStates.gps.threadGrpDimY = pShader->shaderLayout.compute.workGroupSize[1];
        pOutSEP->exeHints.nativeHints.prvStates.gps.threadGrpDimZ = pShader->shaderLayout.compute.workGroupSize[2];

        pOutSEP->exeHints.nativeHints.prvStates.gps.calculatedWorkGroupSize = VIR_Shader_GetWorkGroupSize(pShader);
    }

    /* 2. Derived hints */
    /* Check sampler uniform allocate strategy. */
    if (VIR_Shader_isFullUnifiedUniforms(pShader))
    {
        pOutSEP->exeHints.derivedHints.globalStates.unifiedSamplerRegAllocStrategy = UNIFIED_RF_ALLOC_STRATEGY_UNIFIED;
    }
    /* We need to use this macro to check because we can disable unified-allocation by using flag VSC_COMPILER_FLAG_UNI_SAMPLER_UNIFIED_ALLOC, */
    else if (VIR_Shader_isPackUnifiedSampler(pShader))
    {
        if (pSepGenHelper->baseEpGen.pHwCfg->hwFeatureFlags.supportUnifiedSampler)
        {
            pOutSEP->exeHints.derivedHints.globalStates.unifiedSamplerRegAllocStrategy = UNIFIED_RF_ALLOC_STRATEGY_PACK_FLOAT_ADDR_OFFSET;
        }
        else if (pSepGenHelper->baseEpGen.pHwCfg->hwFeatureFlags.samplerRegFileUnified)
        {
            pOutSEP->exeHints.derivedHints.globalStates.unifiedSamplerRegAllocStrategy = UNIFIED_RF_ALLOC_STRATEGY_PS_TOP_GPIPE_BOTTOM_FLOAT_ADDR_OFFSET;
        }
        else
        {
            pOutSEP->exeHints.derivedHints.globalStates.unifiedSamplerRegAllocStrategy = UNIFIED_RF_ALLOC_STRATEGY_FIXED_ADDR_OFFSET;
        }
    }
    else
    {
        pOutSEP->exeHints.derivedHints.globalStates.unifiedSamplerRegAllocStrategy = UNIFIED_RF_ALLOC_STRATEGY_FIXED_ADDR_OFFSET;
    }
    /* Check constant uniform allocate strategy. */
    if (VIR_Shader_isFullUnifiedUniforms(pShader))
    {
        pOutSEP->exeHints.derivedHints.globalStates.unifiedConstRegAllocStrategy = UNIFIED_RF_ALLOC_STRATEGY_UNIFIED;
    }
    else if (!pSepGenHelper->baseEpGen.pHwCfg->hwFeatureFlags.supportUnifiedConstant &&
             VIR_Shader_IsVulkan(pShader))
    {
        /* For Vulkan, there is a pipeline layout compability requirement, so we need make sure
           each shader has fixed address offset */
        pOutSEP->exeHints.derivedHints.globalStates.unifiedConstRegAllocStrategy = UNIFIED_RF_ALLOC_STRATEGY_FIXED_ADDR_OFFSET;
    }
    else
    {
        if (pSepGenHelper->baseEpGen.pHwCfg->hwFeatureFlags.supportUnifiedConstant)
        {
            pOutSEP->exeHints.derivedHints.globalStates.unifiedConstRegAllocStrategy = UNIFIED_RF_ALLOC_STRATEGY_PACK_FLOAT_ADDR_OFFSET;
        }
        else if (pSepGenHelper->baseEpGen.pHwCfg->hwFeatureFlags.constRegFileUnified)
        {
            pOutSEP->exeHints.derivedHints.globalStates.unifiedConstRegAllocStrategy = UNIFIED_RF_ALLOC_STRATEGY_GPIPE_TOP_PS_BOTTOM_FLOAT_ADDR_OFFSET;
        }
        else
        {
            pOutSEP->exeHints.derivedHints.globalStates.unifiedConstRegAllocStrategy = UNIFIED_RF_ALLOC_STRATEGY_FIXED_ADDR_OFFSET;
        }
    }

    pOutSEP->exeHints.derivedHints.globalStates.bExecuteOnDual16 = pShader->__IsDual16Shader;
    pOutSEP->exeHints.derivedHints.globalStates.bGprSpilled  = pShader->hasRegisterSpill;
    pOutSEP->exeHints.derivedHints.globalStates.gprSpillSize = pShader->vidmemSizeOfSpill;
    pOutSEP->exeHints.derivedHints.globalStates.bCrSpilled   = pShader->hasCRegSpill;
    pOutSEP->exeHints.derivedHints.globalStates.memoryAccessHint = (SHADER_EDH_MEM_ACCESS_HINT)pShader->memoryAccessFlag[VIR_SHLEVEL_Post_Machine];
    pOutSEP->exeHints.derivedHints.globalStates.flowControlHint = (SHADER_EDH_FLOW_CONTROL_HINT)pShader->flowControlFlag[VIR_SHLEVEL_Post_Machine];
    pOutSEP->exeHints.derivedHints.globalStates.texldHint = (SHADER_EDH_TEXLD_HINT)pShader->texldFlag[VIR_SHLEVEL_Post_Machine];
    pOutSEP->exeHints.derivedHints.globalStates.hwStartRegNoForUSCAddrs = pShader->remapRegStart;
    pOutSEP->exeHints.derivedHints.globalStates.hwStartRegChannelForUSCAddrs = pShader->remapChannelStart;
    pOutSEP->exeHints.derivedHints.globalStates.bIoUSCAddrsPackedToOneReg = (pShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL) ?
                                                                             VIR_Shader_TCS_UsePackedRemap(pShader) : gcvFALSE;
    pOutSEP->exeHints.derivedHints.globalStates.bEnableMultiGPU = VIR_Shader_IsEnableMultiGPU(pShader);
    pOutSEP->exeHints.derivedHints.globalStates.bEnableRobustCheck = VIR_Shader_IsEnableRobustCheck(pShader);

    pOutSEP->exeHints.derivedHints.prePaStates.bOutPosZDepW = pShader->vsPositionZDependsOnW;

    if (pShader->shaderKind == VIR_SHADER_GEOMETRY)
    {
        pOutSEP->exeHints.derivedHints.prvStates.gs.bHasPrimRestartOp = VIR_Shader_GS_HasRestartOp(pShader);
    }

    if (pShader->shaderKind == VIR_SHADER_FRAGMENT)
    {
        pOutSEP->exeHints.derivedHints.prvStates.ps.bPxlDiscard = pShader->psHasDiscard;
        pOutSEP->exeHints.derivedHints.prvStates.ps.bDerivRTx = pShader->hasDsx;
        pOutSEP->exeHints.derivedHints.prvStates.ps.bDerivRTy = pShader->hasDsy;
        pOutSEP->exeHints.derivedHints.prvStates.ps.bDsyBeforeLowering = VIR_Shader_hasDsyBeforeLowering(pShader);
        pOutSEP->exeHints.derivedHints.prvStates.ps.hwRegNoForSampleMaskId = pShader->sampleMaskIdRegStart;
        pOutSEP->exeHints.derivedHints.prvStates.ps.hwRegChannelForSampleMaskId = pShader->sampleMaskIdChannelStart;
        pOutSEP->exeHints.derivedHints.prvStates.ps.psStartRegIndex = pShader->psRegStartIndex;
        pOutSEP->exeHints.derivedHints.prvStates.ps.bExecuteOnSampleFreq = VIR_Shader_PS_RunOnSampleShading(pShader);
        pOutSEP->exeHints.derivedHints.prvStates.ps.bNeedRtRead = pShader->useLastFragData;
        pOutSEP->exeHints.derivedHints.prvStates.ps.fragColorUsage = pShader->fragColorUsage;

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

    if (pShader->shaderKind == VIR_SHADER_COMPUTE)
    {
        pOutSEP->exeHints.derivedHints.prvStates.gps.bThreadGroupSync = pShader->hasThreadGroupSync;
        pOutSEP->exeHints.derivedHints.prvStates.gps.bUseLocalMemory = VIR_Shader_UseLocalMem(pShader);
        pOutSEP->exeHints.derivedHints.prvStates.gps.bUsePrivateMemory = VIR_Shader_UsePrivateMem(pShader);
        pOutSEP->exeHints.derivedHints.prvStates.gps.bUseEvisInst = VIR_Shader_UseEvisInst(pShader);
        pOutSEP->exeHints.derivedHints.prvStates.gps.bDependOnWorkGroupSize = VIR_Shader_DependOnWorkGroupSize(pShader);

        for (i = 0; i < 3; i++)
        {
            pOutSEP->exeHints.derivedHints.prvStates.gps.workGroupSizeFactor[i] = VIR_Shader_GetWorkGroupSizeFactor(pShader, i);
        }
    }
}

static SHADER_PRIV_UAV_ENTRY* _enlargePrivUavMappingRoom(SHADER_PRIV_UAV_MAPPING* pPrivUavMapping,
                                                         gctUINT enlargePrivUavEntryCount)
{
    void*                         pOldPrivUavEntry;
    gctUINT                       oldPrivUavEntryCount, newPrivUavEntryCount;
    SHADER_PRIV_UAV_ENTRY*        pNewPrivUavEntry;

    pOldPrivUavEntry = pPrivUavMapping->pPrivUavEntries;
    oldPrivUavEntryCount = pPrivUavMapping->countOfEntries;
    newPrivUavEntryCount = oldPrivUavEntryCount + enlargePrivUavEntryCount;
    if (gcoOS_Allocate(gcvNULL,
                   sizeof(SHADER_PRIV_UAV_ENTRY) * newPrivUavEntryCount,
                   (gctPOINTER*)&pPrivUavMapping->pPrivUavEntries) != gcvSTATUS_OK)
    {
        return gcvNULL;
    }

    if (pOldPrivUavEntry)
    {
        memcpy(pPrivUavMapping->pPrivUavEntries, pOldPrivUavEntry,
               oldPrivUavEntryCount*sizeof(SHADER_PRIV_UAV_ENTRY));

        gcoOS_Free(gcvNULL, pOldPrivUavEntry);
    }

    pPrivUavMapping->countOfEntries = newPrivUavEntryCount;

    pNewPrivUavEntry = &pPrivUavMapping->pPrivUavEntries[oldPrivUavEntryCount];
    memset(pNewPrivUavEntry, 0, sizeof(SHADER_PRIV_UAV_ENTRY));
    pNewPrivUavEntry->uavEntryIndex = oldPrivUavEntryCount;

    /* Return first enlarged priv-Uav-mapping entry */
    return pNewPrivUavEntry;
}

static SHADER_PRIV_CONSTANT_ENTRY* _enlargePrivCnstMappingRoom(SHADER_PRIV_CONSTANT_MAPPING* pPrivCnstMapping,
                                                               gctUINT enlargePrivCnstEntryCount,
                                                               gctUINT* pStartPrivCnstEntryIdx)
{
    void*                         pOldPrivCnstEntry;
    gctUINT                       oldPrivCnstEntryCount, newPrivCnstEntryCount;

    pOldPrivCnstEntry = pPrivCnstMapping->pPrivmConstantEntries;
    oldPrivCnstEntryCount = pPrivCnstMapping->countOfEntries;
    newPrivCnstEntryCount = oldPrivCnstEntryCount + enlargePrivCnstEntryCount;
    if (gcoOS_Allocate(gcvNULL,
                   sizeof(SHADER_PRIV_CONSTANT_ENTRY) * newPrivCnstEntryCount,
                   (gctPOINTER*)&pPrivCnstMapping->pPrivmConstantEntries) != gcvSTATUS_OK)
    {
        return gcvNULL;
    }

    memset(pPrivCnstMapping->pPrivmConstantEntries, 0, sizeof(SHADER_PRIV_CONSTANT_ENTRY) * newPrivCnstEntryCount);

    if (pOldPrivCnstEntry)
    {
        memcpy(pPrivCnstMapping->pPrivmConstantEntries, pOldPrivCnstEntry,
               oldPrivCnstEntryCount*sizeof(SHADER_PRIV_CONSTANT_ENTRY));

        gcoOS_Free(gcvNULL, pOldPrivCnstEntry);
    }

    pPrivCnstMapping->countOfEntries = newPrivCnstEntryCount;

    if (pStartPrivCnstEntryIdx)
    {
        *pStartPrivCnstEntryIdx = oldPrivCnstEntryCount;
    }

    /* Return first enlarged priv-constant-mapping entry */
    return &pPrivCnstMapping->pPrivmConstantEntries[oldPrivCnstEntryCount];
}

static SHADER_COMPILE_TIME_CONSTANT** _enlargePrivCTCMemDataMappingRoom(SHADER_PRIV_MEM_DATA_MAPPING* pPrivCTCMemDataMapping,
                                                                        gctUINT enlargePrivCTCMemDataEntryCount,
                                                                        gctUINT* pStartPrivCTCMemDataEntryIdx)
{
    void*                         pOldPrivCTCMemDataEntry;
    gctUINT                       oldPrivCTCMemDataEntryCount, newPrivCTCMemDataEntryCount;

    pOldPrivCTCMemDataEntry = pPrivCTCMemDataMapping->ppCTC;
    oldPrivCTCMemDataEntryCount = pPrivCTCMemDataMapping->ctcCount;
    newPrivCTCMemDataEntryCount = oldPrivCTCMemDataEntryCount + enlargePrivCTCMemDataEntryCount;
    if (gcoOS_Allocate(gcvNULL,
                   sizeof(SHADER_COMPILE_TIME_CONSTANT*) * newPrivCTCMemDataEntryCount,
                   (gctPOINTER*)&pPrivCTCMemDataMapping->ppCTC) != gcvSTATUS_OK)
    {
        return gcvNULL;
    }

    if (pOldPrivCTCMemDataEntry)
    {
        memcpy(pPrivCTCMemDataMapping->ppCTC, pOldPrivCTCMemDataEntry,
               oldPrivCTCMemDataEntryCount*sizeof(SHADER_COMPILE_TIME_CONSTANT*));

        gcoOS_Free(gcvNULL, pOldPrivCTCMemDataEntry);
    }

    pPrivCTCMemDataMapping->ctcCount = newPrivCTCMemDataEntryCount;

    if (pStartPrivCTCMemDataEntryIdx)
    {
        *pStartPrivCTCMemDataEntryIdx = oldPrivCTCMemDataEntryCount;
    }

    /* Return first enlarged priv-constant-mapping entry */
    return &pPrivCTCMemDataMapping->ppCTC[oldPrivCTCMemDataEntryCount];
}

static SHS_PRIV_CONSTANT_KIND _MapUniformKindToPrivConstKind(VIR_Shader* pShader, VIR_Symbol* pUniformSym, gctBOOL *pOCLOnly)
{
    SHS_PRIV_CONSTANT_KIND      privConstKind = SHS_PRIV_CONSTANT_KIND_COUNT;
    VIR_UniformKind             uniformKind = VIR_Symbol_GetUniformKind(pUniformSym);
    gctBOOL                     bOCLOnly = gcvFALSE;

    switch (uniformKind)
    {
    /* General uniform kind. */
    case VIR_UNIFORM_NUM_GROUPS:
        privConstKind = SHS_PRIV_CONSTANT_KIND_COMPUTE_GROUP_NUM;
        break;
    case VIR_UNIFORM_NUM_GROUPS_FOR_SINGLE_GPU:
        privConstKind = SHS_PRIV_CONSTANT_KIND_COMPUTE_GROUP_NUM_FOR_SINGLE_GPU;
        break;

    case VIR_UNIFORM_BASE_INSTANCE:
        privConstKind = SHS_PRIV_CONSTANT_KIND_BASE_INSTANCE;
        break;

    case VIR_UNIFORM_LEVEL_BASE_SIZE:
        {
            VIR_Uniform*    pVirUniform = VIR_Symbol_GetUniform(pUniformSym);
            VIR_Symbol*     pParentSamplerSym = VIR_Shader_GetSymFromId(pShader, pVirUniform->u.samplerOrImageAttr.parentSamplerSymId);

            if (VIR_Symbol_isImage(pParentSamplerSym) || VIR_Symbol_isImageT(pParentSamplerSym))
            {
                privConstKind = SHS_PRIV_CONSTANT_KIND_IMAGE_SIZE;
            }
            else
            {
                privConstKind = SHS_PRIV_CONSTANT_KIND_TEXTURE_SIZE;
            }
        }
        break;

    case VIR_UNIFORM_LOD_MIN_MAX:
        privConstKind = SHS_PRIV_CONSTANT_KIND_LOD_MIN_MAX;
        break;

    case VIR_UNIFORM_LEVELS_SAMPLES:
        privConstKind = SHS_PRIV_CONSTANT_KIND_LEVELS_SAMPLES;
        break;

    case VIR_UNIFORM_SAMPLE_LOCATION:
        privConstKind = SHS_PRIV_CONSTANT_KIND_SAMPLE_LOCATION;
        break;

    case VIR_UNIFORM_ENABLE_MULTISAMPLE_BUFFERS:
        privConstKind = SHS_PRIV_CONSTANT_KIND_ENABLE_MULTISAMPLE_BUFFERS;
        break;

    /* OCL only uniform kind. */
    case VIR_UNIFORM_LOCAL_ADDRESS_SPACE:
        privConstKind = SHS_PRIV_CONSTANT_KIND_LOCAL_ADDRESS_SPACE;
        bOCLOnly = gcvTRUE;
        break;

    case VIR_UNIFORM_PRIVATE_ADDRESS_SPACE:
        privConstKind = SHS_PRIV_CONSTANT_KIND_PRIVATE_ADDRESS_SPACE;
        bOCLOnly = gcvTRUE;
        break;

    case VIR_UNIFORM_CONSTANT_ADDRESS_SPACE:
        privConstKind = SHS_PRIV_CONSTANT_KIND_CONSTANT_ADDRESS_SPACE;
        bOCLOnly = gcvTRUE;
        break;

    case VIR_UNIFORM_GLOBAL_SIZE:
        privConstKind = SHS_PRIV_CONSTANT_KIND_GLOBAL_SIZE;
        bOCLOnly = gcvTRUE;
        break;

    case VIR_UNIFORM_LOCAL_SIZE:
        privConstKind = SHS_PRIV_CONSTANT_KIND_LOCAL_SIZE;
        bOCLOnly = gcvTRUE;
        break;

    case VIR_UNIFORM_GLOBAL_OFFSET:
        privConstKind = SHS_PRIV_CONSTANT_KIND_GLOBAL_OFFSET;
        bOCLOnly = gcvTRUE;
        break;

    case VIR_UNIFORM_WORK_DIM:
        privConstKind = SHS_PRIV_CONSTANT_KIND_WORK_DIM;
        bOCLOnly = gcvTRUE;
        break;

    case VIR_UNIFORM_PRINTF_ADDRESS:
        privConstKind = SHS_PRIV_CONSTANT_KIND_PRINTF_ADDRESS;
        bOCLOnly = gcvTRUE;
        break;

    case VIR_UNIFORM_WORKITEM_PRINTF_BUFFER_SIZE:
        privConstKind = SHS_PRIV_CONSTANT_KIND_WORKITEM_PRINTF_BUFFER_SIZE;
        bOCLOnly = gcvTRUE;
        break;

    case VIR_UNIFORM_WORK_THREAD_COUNT:
        privConstKind = SHS_PRIV_CONSTANT_KIND_WORK_THREAD_COUNT;
        bOCLOnly = gcvTRUE;
        break;

    case VIR_UNIFORM_WORK_GROUP_COUNT:
        privConstKind = SHS_PRIV_CONSTANT_KIND_WORK_GROUP_COUNT;
        bOCLOnly = gcvTRUE;
        break;

    case VIR_UNIFORM_KERNEL_ARG_LOCAL_MEM_SIZE:
        privConstKind = SHS_PRIV_CONSTANT_KIND_LOCAL_MEM_SIZE;
        bOCLOnly = gcvTRUE;
        break;

    case VIR_UNIFORM_WORK_GROUP_ID_OFFSET:
        privConstKind = SHS_PRIV_CONSTANT_KIND_WORK_GROUP_ID_OFFSET;
        bOCLOnly = gcvTRUE;
        break;

    case VIR_UNIFORM_GLOBAL_INVOCATION_ID_OFFSET:
        privConstKind = SHS_PRIV_CONSTANT_KIND_GLOBAL_INVOCATION_ID_OFFSET;
        bOCLOnly = gcvTRUE;
        break;

    case VIR_UNIFORM_TEMP_REG_SPILL_MEM_ADDRESS:
        if (VIR_Shader_IsEnableMultiGPU(pShader))
        {
            privConstKind = SHS_PRIV_CONSTANT_KIND_TEMP_REG_SPILL_MEM_ADDRESS;
        }
        break;

    case VIR_UNIFORM_GLOBAL_WORK_SCALE:
        privConstKind = SHS_PRIV_CONSTANT_KIND_GLOBAL_WORK_SCALE;
        bOCLOnly = gcvTRUE;
        break;

    case VIR_UNIFORM_VIEW_INDEX:
        privConstKind = SHS_PRIV_CONSTANT_KIND_VIEW_INDEX;
        break;

    default:
        break;
    }

    if (pOCLOnly)
    {
        *pOCLOnly = bOCLOnly;
    }

    return privConstKind;
}

static SHADER_DEFAULT_UBO_MEMBER_ENTRY* _enlargeDefaultUboMappingRoom(SHADER_DEFAULT_UBO_MAPPING* pDuboMapping,
                                                                      gctUINT enlargeDuboEntryCount,
                                                                      gctUINT* pStartDuboEntryIdx)
{
    void*                         pOldDuboEntry;
    gctUINT                       oldDuboEntryCount, newDuboEntryCount;

    pOldDuboEntry = pDuboMapping->pDefaultUboMemberEntries;
    oldDuboEntryCount = pDuboMapping->countOfEntries;
    newDuboEntryCount = oldDuboEntryCount + enlargeDuboEntryCount;
    if (gcoOS_Allocate(gcvNULL,
                   sizeof(SHADER_DEFAULT_UBO_MEMBER_ENTRY) * newDuboEntryCount,
                   (gctPOINTER*)&pDuboMapping->pDefaultUboMemberEntries) != gcvSTATUS_OK)
    {
        return gcvNULL;
    }

    if (pOldDuboEntry)
    {
        memcpy(pDuboMapping->pDefaultUboMemberEntries, pOldDuboEntry,
               oldDuboEntryCount*sizeof(SHADER_DEFAULT_UBO_MEMBER_ENTRY));

        gcoOS_Free(gcvNULL, pOldDuboEntry);
    }

    pDuboMapping->countOfEntries = newDuboEntryCount;

    if (pStartDuboEntryIdx)
    {
        *pStartDuboEntryIdx = oldDuboEntryCount;
    }

    /* Return first enlarged default UBO member entry */
    return &pDuboMapping->pDefaultUboMemberEntries[oldDuboEntryCount];
}

static VSC_ErrCode _AllocateDefaultUboToSEP(VSC_SEP_GEN_HELPER* pSepGenHelper,
                                            SHADER_EXECUTABLE_PROFILE* pOutSEP)
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    SHADER_DEFAULT_UBO_MAPPING* pDuboMapping = &pOutSEP->defaultUboMapping;
    VIR_Shader*                 pShader = pSepGenHelper->pShader;
    VIR_Symbol*                 pDuboSym = gcvNULL;
    VIR_Symbol*                 pDuboBaseAddrSym = gcvNULL;
    VIR_UniformBlock*           pDubo = gcvNULL;
    VIR_Uniform*                pDuboBaseAddrUniform = gcvNULL;
    SHADER_PRIV_CONSTANT_ENTRY* pPrivCnstEntry = gcvNULL;
    gctUINT                     llSlot, llArraySlot;

    if (VIR_Shader_GetDefaultUBOIndex(pShader) == -1)
    {
        return errCode;
    }

    /* Get the default UBO symbol. */
    pDuboSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(VIR_Shader_GetUniformBlocks(pShader), VIR_Shader_GetDefaultUBOIndex(pShader)));
    if (!(pDuboSym && isSymUBODUBO(pDuboSym)))
    {
        gcmASSERT(gcvFALSE);
        return errCode;
    }

    /* Get the default UBO. */
    pDubo = VIR_Symbol_GetUBO(pDuboSym);

    /* Get the base address symbol. */
    pDuboBaseAddrSym = VIR_Shader_GetSymFromId(pShader, VIR_UBO_GetBaseAddress(pDubo));
    if (pDuboBaseAddrSym == gcvNULL)
    {
        gcmASSERT(gcvFALSE);
        return errCode;
    }

    /* Get the uniform. */
    pDuboBaseAddrUniform = VIR_Symbol_GetUniform(pDuboBaseAddrSym);

    /* Check if we need to allocate this UBO. */
    if (pDuboBaseAddrUniform == gcvNULL || VIR_Uniform_GetPhysical(pDuboBaseAddrUniform) == -1)
    {
        return errCode;
    }

    if (!isSymUniformUsedInShader(pDuboBaseAddrSym)   &&
        !isSymUniformImplicitlyUsed(pDuboBaseAddrSym) &&
        !isSymUniformUsedInLTC(pDuboBaseAddrSym))
    {
        return errCode;
    }

    /* Allocate the default UBO mapping. */
    pDuboMapping->sizeInByte = VIR_UBO_GetBlockSize(pDubo);

    /* Creata a new private constant entry. */
    pPrivCnstEntry = _enlargePrivCnstMappingRoom(&pOutSEP->staticPrivMapping.privConstantMapping, 1, &pDuboMapping->baseAddressIndexInPrivConstTable);
    if (pPrivCnstEntry == gcvNULL)
    {
        return VSC_ERR_OUT_OF_MEMORY;
    }
    pPrivCnstEntry->commonPrivm.privmKind = SHS_PRIV_CONSTANT_KIND_DEFAULT_UBO_ADDRESS;
    pPrivCnstEntry->commonPrivm.privmKindIndex = 0;
    pPrivCnstEntry->commonPrivm.pPrivateData = gcvNULL;

    llArraySlot = VIR_Symbol_GetArraySlot(pDuboBaseAddrSym);
    gcmASSERT(llArraySlot != NOT_ASSIGNED);
    llSlot = VIR_Symbol_GetFirstSlot(pDuboBaseAddrSym);

    pPrivCnstEntry->mode = SHADER_PRIV_CONSTANT_MODE_VAL_2_MEMORREG;
    pPrivCnstEntry->u.pSubCBMapping = &pOutSEP->constantMapping.pConstantArrayMapping[llArraySlot].pSubConstantArrays[llSlot];

    return errCode;
}

static VSC_ErrCode _CollectStaticPrivateMappingToSEP(VSC_SEP_GEN_HELPER* pSepGenHelper, SHADER_EXECUTABLE_PROFILE* pOutSEP)
{
    VSC_ErrCode                    errCode = VSC_ERR_NONE;
    VIR_Shader*                    pShader = pSepGenHelper->pShader;
    VIR_UniformIdList*             pVirUniformLsts = VIR_Shader_GetUniforms(pShader);
    VIR_Symbol*                    pVirUniformSym;
    VIR_Uniform*                   pVirUniform;
    VIR_UBOIdList*                 pVirUboLsts = &pShader->uniformBlocks;
    VIR_SBOIdList*                 pVirSboLsts = &pShader->storageBlocks;
    VIR_Symbol*                    pVirUniformBlockSym;
    VIR_UniformBlock*              pVirUniformBlock;
    VIR_Symbol*                    pVirStorageBlockSym;
    VIR_StorageBlock*              pVirStorageBlock;
    gctUINT                        virSboIdx, virUniformIdx, llArraySlot, llSlot, virUboIdx, ctcSlot, thisUniformRegCount, i;
    SHADER_PRIV_UAV_ENTRY*         pPrivUavEntry;
    SHADER_PRIV_CONSTANT_ENTRY*    pPrivCnstEntry;
    SHADER_COMPILE_TIME_CONSTANT** ppCTC;
    gctBOOL                        bCreateDuboInSep = (pOutSEP->defaultUboMapping.sizeInByte != 0);

    /* Gpr spilled memory */
    if (pShader->hasRegisterSpill)
    {
        gcmASSERT(pShader->vidmemSizeOfSpill > 0);

        pPrivUavEntry = _enlargePrivUavMappingRoom(&pOutSEP->staticPrivMapping.privUavMapping, 1);
        if (pPrivUavEntry == gcvNULL)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }

        pPrivUavEntry->commonPrivm.privmKind = SHS_PRIV_MEM_KIND_GPR_SPILLED_MEMORY;
        pPrivUavEntry->commonPrivm.privmKindIndex = 0;
        pPrivUavEntry->commonPrivm.pPrivateData = gcvNULL;
        pPrivUavEntry->pBuffer = &pOutSEP->uavMapping.pUAV[pShader->llSlotForSpillVidmem];
    }

    /* Const reg spilled memory */
    if (pShader->hasCRegSpill)
    {
        for (virUboIdx = 0; virUboIdx < VIR_IdList_Count(pVirUboLsts); virUboIdx ++)
        {
            pVirUniformBlockSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pVirUboLsts, virUboIdx));
            pVirUniformBlock = VIR_Symbol_GetUBO(pVirUniformBlockSym);

            if (pVirUniformBlock->flags & VIR_IB_WITH_CUBO)
            {
                pPrivUavEntry = _enlargePrivUavMappingRoom(&pOutSEP->staticPrivMapping.privUavMapping, 1);
                if (pPrivUavEntry == gcvNULL)
                {
                    return VSC_ERR_OUT_OF_MEMORY;
                }
                llSlot = VIR_Symbol_GetFirstSlot(pVirUniformBlockSym);
                gcmASSERT(llSlot != NOT_ASSIGNED);

                pPrivUavEntry->commonPrivm.privmKind = SHS_PRIV_MEM_KIND_CONSTANT_SPILLED_MEMORY;
                pPrivUavEntry->commonPrivm.privmKindIndex = 0;
                pPrivUavEntry->commonPrivm.pPrivateData = gcvNULL;
                pPrivUavEntry->pBuffer = &pOutSEP->uavMapping.pUAV[llSlot];

                for (virUniformIdx = 0; virUniformIdx < pVirUniformBlock->uniformCount; virUniformIdx ++)
                {
                    pVirUniform = pVirUniformBlock->uniforms[virUniformIdx];
                    pVirUniformSym = VIR_Shader_GetSymFromId(pShader, pVirUniform->sym);

                    if (isSymUniformCompiletimeInitialized(pVirUniformSym))
                    {
                        VIR_Type* pVirUniformType = VIR_Symbol_GetType(pVirUniformSym);

                        ctcSlot = VIR_Symbol_GetFirstSlot(pVirUniformSym);

                        ppCTC = _enlargePrivCTCMemDataMappingRoom(&pPrivUavEntry->memData, 1, gcvNULL);
                        if (ppCTC == gcvNULL)
                        {
                            return VSC_ERR_OUT_OF_MEMORY;
                        }
                        *ppCTC = &pOutSEP->constantMapping.pCompileTimeConstant[ctcSlot];
                        if (VIR_Type_isArray(pVirUniformType))
                        {
                            /*copy left data to memData if uniform is array*/
                            gctINT32 i;
                            for (i = 1; i < pVirUniform->realUseArraySize; i++)
                            {
                                 ppCTC = _enlargePrivCTCMemDataMappingRoom(&pPrivUavEntry->memData, 1, gcvNULL);
                                 if (ppCTC == gcvNULL)
                                 {
                                     return VSC_ERR_OUT_OF_MEMORY;
                                 }
                                 *ppCTC = &pOutSEP->constantMapping.pCompileTimeConstant[ctcSlot+i];
                            }
                        }
                    }
                    else
                    {
                        gcmASSERT(gcvFALSE);
                    }
                }

                break;
            }
        }
    }

    /* Share memory simulated by global memory */
    for (virSboIdx = 0; virSboIdx < VIR_IdList_Count(pVirSboLsts); virSboIdx ++)
    {
        pVirStorageBlockSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pVirSboLsts, virSboIdx));
        pVirStorageBlock = VIR_Symbol_GetSBO(pVirStorageBlockSym);

        if (pVirStorageBlock->flags & VIR_IB_FOR_SHARED_VARIABLE)
        {
            /* Skip if this SBO is not used in shader. */
            pVirUniformSym = VIR_Shader_GetSymFromId(pShader, pVirStorageBlock->baseAddr);
            gcmASSERT(VIR_Symbol_isUniform(pVirUniformSym));
            gcmASSERT(VIR_Symbol_GetUniformKind(pVirUniformSym) == VIR_UNIFORM_STORAGE_BLOCK_ADDRESS);

            if (!isSymUniformUsedInShader(pVirUniformSym) &&
                !isSymUniformImplicitlyUsed(pVirUniformSym))
            {
                break;
            }

            pPrivUavEntry = _enlargePrivUavMappingRoom(&pOutSEP->staticPrivMapping.privUavMapping, 1);
            if (pPrivUavEntry == gcvNULL)
            {
                return VSC_ERR_OUT_OF_MEMORY;
            }
            llSlot = VIR_Symbol_GetFirstSlot(pVirStorageBlockSym);
            gcmASSERT(llSlot != NOT_ASSIGNED);

            pPrivUavEntry->commonPrivm.privmKind = SHS_PRIV_MEM_KIND_SHARED_MEMORY;
            pPrivUavEntry->commonPrivm.privmKindIndex = 0;
            pPrivUavEntry->commonPrivm.pPrivateData = gcvNULL;
            pPrivUavEntry->pBuffer = &pOutSEP->uavMapping.pUAV[llSlot];

            break;
        }
    }

    /*
    ** 1) Share memory simulated by global memory for OCL.
    ** 2) The global memory to save the thread ID.
    */
    for (virUniformIdx = 0; virUniformIdx < VIR_IdList_Count(pVirUniformLsts); virUniformIdx ++)
    {
        pVirUniformSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pVirUniformLsts, virUniformIdx));
        pVirUniform = VIR_Symbol_GetUniform(pVirUniformSym);

        if (pVirUniform == gcvNULL ||
            pVirUniform->physical == -1 ||
            !isSymUniformUsedInShader(pVirUniformSym))
        {
            continue;
        }

        if (strcmp(VIR_Shader_GetSymNameString(pShader, pVirUniformSym), _sldLocalStorageAddressName) == 0)
        {
            pPrivUavEntry = _enlargePrivUavMappingRoom(&pOutSEP->staticPrivMapping.privUavMapping, 1);
            if (pPrivUavEntry == gcvNULL)
            {
                return VSC_ERR_OUT_OF_MEMORY;
            }
            llSlot = VIR_Symbol_GetLlResSlot(pVirUniformSym);
            gcmASSERT(llSlot != NOT_ASSIGNED);

            pPrivUavEntry->commonPrivm.privmKind = SHS_PRIV_MEM_KIND_SHARED_MEMORY;
            pPrivUavEntry->commonPrivm.privmKindIndex = 0;
            pPrivUavEntry->commonPrivm.pPrivateData = gcvNULL;
            pPrivUavEntry->pBuffer = &pOutSEP->uavMapping.pUAV[llSlot];
        }
        else if (VIR_Symbol_GetUniformKind(pVirUniformSym) == VIR_UNIFORM_THREAD_ID_MEM_ADDR)
        {
            pPrivUavEntry = _enlargePrivUavMappingRoom(&pOutSEP->staticPrivMapping.privUavMapping, 1);
            if (pPrivUavEntry == gcvNULL)
            {
                return VSC_ERR_OUT_OF_MEMORY;
            }
            llSlot = VIR_Symbol_GetLlResSlot(pVirUniformSym);
            gcmASSERT(llSlot != NOT_ASSIGNED);

            pPrivUavEntry->commonPrivm.privmKind = SHS_PRIV_MEM_KIND_THREAD_ID_MEM_ADDR;
            pPrivUavEntry->commonPrivm.privmKindIndex = 0;
            pPrivUavEntry->commonPrivm.pPrivateData = gcvNULL;
            pPrivUavEntry->pBuffer = &pOutSEP->uavMapping.pUAV[llSlot];
        }
    }

    /* Other private UAVs(image-related). */
    for (virUniformIdx = 0; virUniformIdx < VIR_IdList_Count(pVirUniformLsts); virUniformIdx ++)
    {
        pVirUniformSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pVirUniformLsts, virUniformIdx));

        if ((VIR_Symbol_isImage(pVirUniformSym) || VIR_Symbol_isImageT(pVirUniformSym)) && isSymCompilerGen(pVirUniformSym))
        {
            SHS_PRIV_MEM_KIND   privMemKind = SHS_PRIV_MEM_KIND_NONE;
            VIR_UniformKind     uniformKind;
            gctBOOL             bAddLlResSlot = gcvTRUE;
            gctBOOL             bSpecifiedPrivmKindIndex = gcvFALSE;
            gctUINT             specifiedPrivmKindIndex = 0;

            pVirUniform = VIR_Symbol_GetUniformPointer(pShader, pVirUniformSym);
            if (pVirUniform && pVirUniform->physical == -1)
            {
                continue;
            }

            uniformKind = VIR_Symbol_GetUniformKind(pVirUniformSym);
            switch (uniformKind)
            {
            case VIR_UNIFORM_EXTRA_LAYER:
                privMemKind = SHS_PRIV_MEM_KIND_EXTRA_UAV_LAYER;
                break;

            case VIR_UNIFORM_YCBCR_PLANE:
                {
                    VIR_Symbol* pParentSym = VIR_Shader_GetSymFromId(pShader, pVirUniform->u.samplerOrImageAttr.parentSamplerSymId);
                    VIR_Uniform *pParentUniform = VIR_Symbol_GetUniformPointer(pShader, pParentSym);
                    gctUINT     i;
                    VIR_SymId   *pPlaneSymId;

                    privMemKind = SHS_PRIV_MEM_KIND_YCBCR_PLANE;
                    bAddLlResSlot = gcvFALSE;
                    bSpecifiedPrivmKindIndex = gcvTRUE;

                    gcmASSERT(pParentUniform->u.samplerOrImageAttr.pYcbcrPlaneSymId);
                    pPlaneSymId = pParentUniform->u.samplerOrImageAttr.pYcbcrPlaneSymId +
                                      (pVirUniform->u.samplerOrImageAttr.arrayIdxInParent * __YCBCR_PLANE_COUNT__);
                    for (i = 0; i < __YCBCR_PLANE_COUNT__; i++)
                    {
                        if (pPlaneSymId[i] == VIR_Symbol_GetIndex(pVirUniformSym))
                        {
                            break;
                        }
                    }

                    gcmASSERT(i != __YCBCR_PLANE_COUNT__);
                    specifiedPrivmKindIndex = pVirUniform->u.samplerOrImageAttr.arrayIdxInParent * __YCBCR_PLANE_COUNT__ + i;
                }
                break;

            default:
                break;
            }

            if (privMemKind == SHS_PRIV_MEM_KIND_NONE)
            {
                continue;
            }

            thisUniformRegCount = VIR_Type_GetVirRegCount(pShader, VIR_Symbol_GetType(pVirUniformSym), pVirUniform->realUseArraySize);
            for (i = 0; i < thisUniformRegCount; i ++)
            {
                pPrivUavEntry = _enlargePrivUavMappingRoom(&pOutSEP->staticPrivMapping.privUavMapping, 1);
                if (pPrivUavEntry == gcvNULL)
                {
                    return VSC_ERR_OUT_OF_MEMORY;
                }
                pPrivUavEntry->commonPrivm.privmKind = privMemKind;

                if (bSpecifiedPrivmKindIndex)
                {
                    pPrivUavEntry->commonPrivm.privmKindIndex = specifiedPrivmKindIndex;
                }
                else
                {
                    pPrivUavEntry->commonPrivm.privmKindIndex = i;
                }

                if (bAddLlResSlot)
                {
                    pPrivUavEntry->commonPrivm.privmKindIndex +=
                        VIR_Symbol_GetLlResSlot(VIR_Shader_GetSymFromId(pShader, pVirUniform->u.samplerOrImageAttr.parentSamplerSymId));
                }

                /* For vulkan, it will be processed when collecting info for resource layouts in pep */
                if (VIR_Shader_IsVulkan(pShader) && !pSepGenHelper->bSkipPepGen)
                {
                    pPrivUavEntry->commonPrivm.notAllocated = gcvTRUE;
                    pPrivUavEntry->commonPrivm.pPrivateData = VIR_Shader_GetSymFromId(pShader, pVirUniform->u.samplerOrImageAttr.parentSamplerSymId);
                }
                else
                {
                    pPrivUavEntry->commonPrivm.pPrivateData = gcvNULL;
                }

                pPrivUavEntry->pBuffer = &pOutSEP->uavMapping.pUAV[VIR_Symbol_GetLlResSlot(pVirUniformSym) + i];
            }
        }
    }

    /* Private constants */
    for (virUniformIdx = 0; virUniformIdx < VIR_IdList_Count(pVirUniformLsts); virUniformIdx ++)
    {
        SHS_PRIV_CONSTANT_KIND  privConstKind;
        gctUINT                 privConstIndex = 0;
        gctBOOL                 bOCLOnly = gcvFALSE;
        gctBOOL                 bUseInDubo = gcvFALSE, bMapToDubo = gcvFALSE;

        pVirUniformSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pVirUniformLsts, virUniformIdx));
        pVirUniform = VIR_Symbol_GetUniformPointer(pShader, pVirUniformSym);
        bUseInDubo = isSymUniformMovedToAUBO(pVirUniformSym) && bCreateDuboInSep;

        if (pVirUniform == gcvNULL ||
            isSymUniformCompiletimeInitialized(pVirUniformSym))
        {
            continue;
        }

        if (VIR_Uniform_GetPhysical(pVirUniform) == -1)
        {
            if (bUseInDubo)
            {
                bMapToDubo = gcvTRUE;
            }
            else
            {
                continue;
            }
        }

        if (!isSymUniformUsedInShader(pVirUniformSym)   &&
            !isSymUniformImplicitlyUsed(pVirUniformSym) &&
            !isSymUniformUsedInLTC(pVirUniformSym)      &&
            !VIR_Uniform_AlwaysAlloc(pShader, pVirUniformSym))
        {
            continue;
        }

        privConstKind = _MapUniformKindToPrivConstKind(pShader, pVirUniformSym, &bOCLOnly);
        if (privConstKind == SHS_PRIV_CONSTANT_KIND_COUNT)
        {
            continue;
        }

        /* Collect OCL only private uniforms  */

        /* Creata a new private constant entry. */
        pPrivCnstEntry = _enlargePrivCnstMappingRoom(&pOutSEP->staticPrivMapping.privConstantMapping, 1, &privConstIndex);
        if (pPrivCnstEntry == gcvNULL)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
        pPrivCnstEntry->commonPrivm.privmKind = privConstKind;
        pPrivCnstEntry->commonPrivm.privmKindIndex = 0;
        pPrivCnstEntry->commonPrivm.pPrivateData = gcvNULL;

        if (privConstKind == SHS_PRIV_CONSTANT_KIND_IMAGE_SIZE      ||
            privConstKind == SHS_PRIV_CONSTANT_KIND_TEXTURE_SIZE    ||
            privConstKind == SHS_PRIV_CONSTANT_KIND_LOD_MIN_MAX     ||
            privConstKind == SHS_PRIV_CONSTANT_KIND_LEVELS_SAMPLES)
        {
            pPrivCnstEntry->commonPrivm.privmKindIndex = VIR_Symbol_GetFirstSlot(
                VIR_Shader_GetSymFromId(pShader, pVirUniform->u.samplerOrImageAttr.parentSamplerSymId));

            /* For vulkan, it will be processed when collecting info for resource layouts in pep */
            if (VIR_Shader_IsVulkan(pShader) && !pSepGenHelper->bSkipPepGen)
            {
                pPrivCnstEntry->commonPrivm.notAllocated = gcvTRUE;
                pPrivCnstEntry->commonPrivm.pPrivateData = VIR_Shader_GetSymFromId(pShader, pVirUniform->u.samplerOrImageAttr.parentSamplerSymId);
            }
        }

        if (bMapToDubo)
        {
            SHADER_DEFAULT_UBO_MEMBER_ENTRY*    pDuboMemberEntry = gcvNULL;
            gctUINT                             duboEntryIndex = 0;

            pDuboMemberEntry = _enlargeDefaultUboMappingRoom(&pOutSEP->defaultUboMapping, 1, &duboEntryIndex);
            if (pDuboMemberEntry == gcvNULL)
            {
                return VSC_ERR_OUT_OF_MEMORY;
            }
            pDuboMemberEntry->memberIndexInOtherEntryTable = privConstIndex;
            pDuboMemberEntry->memberKind = SHS_DEFAULT_UBO_MEMBER_PRIV_CONST;
            pDuboMemberEntry->offsetInByte = VIR_Uniform_GetOffset(pVirUniform);

            pPrivCnstEntry->mode = SHADER_PRIV_CONSTANT_MODE_VAL_2_DUBO;
            pPrivCnstEntry->u.duboEntryIndex = duboEntryIndex;
        }
        else
        {
            llArraySlot = VIR_Symbol_GetArraySlot(pVirUniformSym);
            gcmASSERT(llArraySlot != NOT_ASSIGNED);
            llSlot = VIR_Symbol_GetFirstSlot(pVirUniformSym);

            pPrivCnstEntry->mode = SHADER_PRIV_CONSTANT_MODE_VAL_2_MEMORREG;
            pPrivCnstEntry->u.pSubCBMapping = &pOutSEP->constantMapping.pConstantArrayMapping[llArraySlot].pSubConstantArrays[llSlot];
        }
    }

    return errCode;
}

static SHADER_PRIV_OUTPUT_ENTRY* _enlargePrivOutputMappingRoom(SHADER_PRIV_OUTPUT_MAPPING* pPrivOutputMapping,
                                                               gctUINT enlargePrivOutputEntryCount,
                                                               gctUINT* pStartPrivOutputEntryIdx)
{
    void*                         pOldPrivOutputEntry;
    gctUINT                       oldPrivOutputEntryCount, newPrivOutputEntryCount;

    pOldPrivOutputEntry = pPrivOutputMapping->pPrivOutputEntries;
    oldPrivOutputEntryCount = pPrivOutputMapping->countOfEntries;
    newPrivOutputEntryCount = oldPrivOutputEntryCount + enlargePrivOutputEntryCount;
    if (gcoOS_Allocate(gcvNULL,
                   sizeof(SHADER_PRIV_OUTPUT_ENTRY) * newPrivOutputEntryCount,
                   (gctPOINTER*)&pPrivOutputMapping->pPrivOutputEntries) != gcvSTATUS_OK)
    {
        return gcvNULL;
    }

    if (pOldPrivOutputEntry)
    {
        memcpy(pPrivOutputMapping->pPrivOutputEntries, pOldPrivOutputEntry,
               oldPrivOutputEntryCount*sizeof(SHADER_PRIV_OUTPUT_ENTRY));

        gcoOS_Free(gcvNULL, pOldPrivOutputEntry);
    }

    pPrivOutputMapping->countOfEntries = newPrivOutputEntryCount;

    if (pStartPrivOutputEntryIdx)
    {
        *pStartPrivOutputEntryIdx = oldPrivOutputEntryCount;
    }

    /* Return first enlarged priv-output-mapping entry */
    return &pPrivOutputMapping->pPrivOutputEntries[oldPrivOutputEntryCount];
}

static SHADER_PRIV_SAMPLER_ENTRY* _enlargePrivSamplerMappingRoom(SHADER_PRIV_SAMPLER_MAPPING* pPrivSamplerMapping,
                                                                 gctUINT enlargePrivSamplerEntryCount,
                                                                 gctUINT* pStartPrivSamplerEntryIdx)
{
    void*                         pOldPrivSamplerEntry;
    gctUINT                       oldPrivSamplerEntryCount, newPrivSamplerEntryCount;

    pOldPrivSamplerEntry = pPrivSamplerMapping->pPrivSamplerEntries;
    oldPrivSamplerEntryCount = pPrivSamplerMapping->countOfEntries;
    newPrivSamplerEntryCount = oldPrivSamplerEntryCount + enlargePrivSamplerEntryCount;
    if (gcoOS_Allocate(gcvNULL,
                   sizeof(SHADER_PRIV_SAMPLER_ENTRY) * newPrivSamplerEntryCount,
                   (gctPOINTER*)&pPrivSamplerMapping->pPrivSamplerEntries) != gcvSTATUS_OK)
    {
        return gcvNULL;
    }

    if (pOldPrivSamplerEntry)
    {
        memcpy(pPrivSamplerMapping->pPrivSamplerEntries, pOldPrivSamplerEntry,
               oldPrivSamplerEntryCount*sizeof(SHADER_PRIV_SAMPLER_ENTRY));

        gcoOS_Free(gcvNULL, pOldPrivSamplerEntry);
    }

    pPrivSamplerMapping->countOfEntries = newPrivSamplerEntryCount;

    if (pStartPrivSamplerEntryIdx)
    {
        *pStartPrivSamplerEntryIdx = oldPrivSamplerEntryCount;
    }

    /* Return first enlarged priv-Sampler-mapping entry */
    return &pPrivSamplerMapping->pPrivSamplerEntries[oldPrivSamplerEntryCount];
}

static VSC_ErrCode _CollectDynamicPrivateMappingToSEP(VSC_SEP_GEN_HELPER* pSepGenHelper, SHADER_EXECUTABLE_PROFILE* pOutSEP)
{
    VSC_ErrCode                    errCode = VSC_ERR_NONE;
    VIR_Shader*                    pShader = pSepGenHelper->pShader;
    VIR_IdList*                    pFragoutList = VIR_Shader_GetOutputs(pShader);
    gctUINT                        ioIdx, virIoIdx, virIoCount = VIR_IdList_Count(pFragoutList);
    gctUINT                        virUniformIdx, subUniformIdx;
    VIR_Symbol*                    pVirIoSym;
    VIR_Symbol*                    pVirUniformSym;
    VIR_Uniform*                   pVirUniform;
    VIR_UniformIdList*             pVirUniformLsts = VIR_Shader_GetUniforms(pShader);
    SHADER_PRIV_OUTPUT_ENTRY*      pPrivOutputEntry;
    SHADER_PRIV_SAMPLER_ENTRY*     pPrivSamplerEntry;

    /* Private outputs for VSC_LIB_LINK_TYPE_COLOR_OUTPUT */
    if (pShader->shaderKind == VIR_SHADER_FRAGMENT)
    {
        for (virIoIdx = 0; virIoIdx < virIoCount; virIoIdx ++)
        {
            pVirIoSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pFragoutList, virIoIdx));

            if (!isSymUnused(pVirIoSym) && !isSymVectorizedOut(pVirIoSym) && isSymCompilerGen(pVirIoSym))
            {
                pPrivOutputEntry = _enlargePrivOutputMappingRoom(&pOutSEP->dynamicPrivMapping.privOutputMapping, 1, gcvNULL);
                if (pPrivOutputEntry == gcvNULL)
                {
                    return VSC_ERR_OUT_OF_MEMORY;
                }
                pPrivOutputEntry->commonPrivm.privmKind = VSC_LIB_LINK_TYPE_COLOR_OUTPUT;
                pPrivOutputEntry->commonPrivm.privmKindIndex = VIR_Symbol_GetFirstSlot(pVirIoSym);

                /* For vulkan, set private data to master output's location */
                if (VIR_Shader_IsVulkan(pShader) && !pSepGenHelper->bSkipPepGen)
                {
                    if (gcoOS_Allocate(gcvNULL, sizeof(gctUINT), (gctPOINTER*)&pPrivOutputEntry->commonPrivm.pPrivateData) != gcvSTATUS_OK)
                    {
                        return VSC_ERR_OUT_OF_MEMORY;
                    }
                    *(gctUINT*)pPrivOutputEntry->commonPrivm.pPrivateData = VIR_Symbol_GetMasterLocation(pVirIoSym);
                }
                else
                {
                    pPrivOutputEntry->commonPrivm.pPrivateData = gcvNULL;
                }

                ioIdx = VIR_Symbol_GetFirstSlot(pVirIoSym);
                pPrivOutputEntry->pOutput = &pOutSEP->outputMapping.ioVtxPxl.pIoRegMapping[ioIdx];
            }
        }
    }

    /* Private samplers for VSC_LIB_LINK_TYPE_RESOURCE */
    for (virUniformIdx = 0; virUniformIdx < VIR_IdList_Count(pVirUniformLsts); virUniformIdx ++)
    {
        pVirUniformSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pVirUniformLsts, virUniformIdx));

        if (VIR_Symbol_isSampler(pVirUniformSym) &&
            !isSymInactive(pVirUniformSym) &&
            isSymCompilerGen(pVirUniformSym) &&
            (VIR_Symbol_GetUniformKind(pVirUniformSym) == VIR_UNIFORM_EXTRA_LAYER))
        {
            pVirUniform = VIR_Symbol_GetSampler(pVirUniformSym);
            gcmASSERT(pVirUniform);
            gcmASSERT(pVirUniform->physical != -1);

            for (subUniformIdx = 0; subUniformIdx < (gctUINT)pVirUniform->realUseArraySize; subUniformIdx ++)
            {
                pPrivSamplerEntry = _enlargePrivSamplerMappingRoom(&pOutSEP->dynamicPrivMapping.privSamplerMapping, 1, gcvNULL);
                if (pPrivSamplerEntry == gcvNULL)
                {
                    return VSC_ERR_OUT_OF_MEMORY;
                }
                pPrivSamplerEntry->commonPrivm.privmKind = VSC_LIB_LINK_TYPE_RESOURCE;
                pPrivSamplerEntry->commonPrivm.privmKindIndex = VIR_Symbol_GetFirstSlot(VIR_Shader_GetSymFromId(pShader,
                                                                                pVirUniform->u.samplerOrImageAttr.parentSamplerSymId)) + subUniformIdx;
                pPrivSamplerEntry->commonPrivm.pPrivateData = gcvNULL;

                /* For vulkan, it will be processed when collecting info for resource layouts in pep */
                if (VIR_Shader_IsVulkan(pShader) && !pSepGenHelper->bSkipPepGen)
                {
                    pPrivSamplerEntry->commonPrivm.notAllocated = gcvTRUE;
                    pPrivSamplerEntry->commonPrivm.pPrivateData = pVirUniformSym;
                }
                else
                {
                    pPrivSamplerEntry->commonPrivm.pPrivateData = gcvNULL;
                }

                ioIdx = VIR_Symbol_GetFirstSlot(pVirUniformSym);
                pPrivSamplerEntry->pSampler = &pOutSEP->samplerMapping.pSampler[ioIdx + subUniformIdx];
            }
        }
    }

    return errCode;
}

static VSC_ErrCode _CollectAttrTableToPEP(VSC_PEP_GEN_HELPER* pPepGenHelper,
                                          PROGRAM_EXECUTABLE_PROFILE* pOutPEP)
{
    VSC_ErrCode                  errCode = VSC_ERR_NONE;
    VIR_Shader*                  pShader = pPepGenHelper->pShader;
    SHADER_EXECUTABLE_PROFILE*   pSEP = pPepGenHelper->pSep;
    VIR_IdList*                  pAttrList = VIR_Shader_GetAttributes(pShader);
    gctUINT                      i, virIoIdx, virIoCount = VIR_IdList_Count(pAttrList);
    VIR_Symbol*                  pVirIoSym;
    VIR_Type*                    pVirIoType;
    VIR_Type*                    pElementType;
    gctUINT                      ioIdx, locationCount, lengthOfName, maxLengthOfName = 0;
    gctUINT                      attrTableEntryCount = 0, attrTableEntryIdx = 0;
    PROG_ATTRIBUTE_TABLE*        pAttrTable = &pOutPEP->attribTable;
    PROG_ATTRIBUTE_TABLE_ENTRY*  pAttrEntry;
    gctSTRING                    name;

    /* Calc how many entries we need */
    for (virIoIdx = 0; virIoIdx < virIoCount; virIoIdx ++)
    {
        pVirIoSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pAttrList, virIoIdx));

        if (!isSymUnused(pVirIoSym) && !isSymVectorizedOut(pVirIoSym))
        {
            attrTableEntryCount ++;
        }
    }

    /* Allocate entries if needed */
    if (attrTableEntryCount > 0)
    {
        if (gcoOS_Allocate(gcvNULL,
                       sizeof(PROG_ATTRIBUTE_TABLE_ENTRY) * attrTableEntryCount,
                       (gctPOINTER*)&pAttrTable->pAttribEntries) != gcvSTATUS_OK)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }

        memset(pAttrTable->pAttribEntries, 0, sizeof(PROG_ATTRIBUTE_TABLE_ENTRY) * attrTableEntryCount);

        pAttrTable->countOfEntries = attrTableEntryCount;
    }

    /* Now fill the each entry */
    for (virIoIdx = 0; virIoIdx < virIoCount; virIoIdx ++)
    {
        pVirIoSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pAttrList, virIoIdx));

        if (!isSymUnused(pVirIoSym) && !isSymVectorizedOut(pVirIoSym))
        {
            pAttrEntry = &pAttrTable->pAttribEntries[attrTableEntryIdx];
            pAttrEntry->attribEntryIndex = attrTableEntryIdx;

            /* Type */
            pVirIoType = VIR_Symbol_GetType(pVirIoSym);
            pElementType = pVirIoType;
            if (VIR_Type_GetKind(pVirIoType) == VIR_TY_ARRAY)
            {
                pAttrEntry->arraySize = VIR_Type_GetArrayLength(pVirIoType);
            }
            else
            {
                pAttrEntry->arraySize = 1;
            }

            while (VIR_Type_isArray(pElementType))
            {
                pElementType = VIR_Shader_GetTypeFromId(pShader, VIR_Type_GetBaseTypeId(pElementType));
            }
            gcmASSERT(VIR_Type_isPrimitive(pElementType));
            pAttrEntry->type = (VSC_SHADER_DATA_TYPE)VIR_Type_GetIndex(pElementType);

            switch (VIR_GetTypeSize(VIR_GetTypeComponentType(VIR_Type_GetIndex(pElementType))))
            {
            case 4:
                pAttrEntry->resEntryBit |= VSC_RES_ENTRY_BIT_32BIT;
                break;

            case 2:
                pAttrEntry->resEntryBit |= VSC_RES_ENTRY_BIT_16BIT;
                break;

            case 1:
                pAttrEntry->resEntryBit |= VSC_RES_ENTRY_BIT_8BIT;
                break;

            default:
                break;
            }

            if (pOutPEP->pepClient == PEP_CLIENT_VK)
            {
                /* Type for GL is vec1~vec4 */
            }
            else
            {
                /* Type for GL is element of array */
            }

            /* Name */
            name = VIR_Shader_GetSymNameString(pShader, pVirIoSym);
            lengthOfName = gcoOS_StrLen(name, gcvNULL);
            if (lengthOfName > maxLengthOfName)
            {
                maxLengthOfName = lengthOfName;
            }
            if (gcoOS_Allocate(gcvNULL, lengthOfName + 1, (gctPOINTER*)&pAttrEntry->name) != gcvSTATUS_OK)
            {
                return VSC_ERR_OUT_OF_MEMORY;
            }
            memcpy((gctPOINTER)pAttrEntry->name, name, lengthOfName + 1);
            pAttrEntry->nameLength = lengthOfName;

            /* LL IO-Mapping */
            ioIdx = VIR_Symbol_GetFirstSlot(pVirIoSym);
            pAttrEntry->pIoRegMapping = &pSEP->inputMapping.ioVtxPxl.pIoRegMapping[ioIdx];

            /* Location */
            locationCount = VIR_Symbol_GetVirIoRegCount(pShader, pVirIoSym);
            pAttrEntry->locationCount = locationCount;
            if (gcoOS_Allocate(gcvNULL, locationCount * sizeof(gctUINT), (gctPOINTER*)&pAttrEntry->pLocation)!= gcvSTATUS_OK)
            {
                return VSC_ERR_OUT_OF_MEMORY;
            }
            for (i = 0; i < locationCount; i ++)
            {
                pAttrEntry->pLocation[i] = VIR_Symbol_GetLocation(pVirIoSym) + i;
            }

            /* Vec4 mask */
            pAttrEntry->vec4BasedCount = locationCount;
            for (i = 0; i < pAttrEntry->vec4BasedCount; i ++)
            {
                pAttrEntry->activeVec4Mask |= (1 << i);
            }

            /* Go on to next entry */
            attrTableEntryIdx ++;
        }
    }

    gcmASSERT(attrTableEntryIdx == attrTableEntryCount);

    pAttrTable->maxLengthOfName = maxLengthOfName;

    return errCode;
}

static VSC_ErrCode _CollectFragOutTableToPEP(VSC_PEP_GEN_HELPER* pPepGenHelper,
                                             PROGRAM_EXECUTABLE_PROFILE* pOutPEP)
{
    VSC_ErrCode                  errCode = VSC_ERR_NONE;
    VIR_Shader*                  pShader = pPepGenHelper->pShader;
    SHADER_EXECUTABLE_PROFILE*   pSEP = pPepGenHelper->pSep;
    VIR_IdList*                  pFragoutList = VIR_Shader_GetOutputs(pShader);
    gctUINT                      i, virIoIdx, virIoCount = VIR_IdList_Count(pFragoutList);
    VIR_Symbol*                  pVirIoSym;
    VIR_Type*                    pVirIoType;
    VIR_Type*                    pElementType;
    gctUINT                      ioIdx, locationCount, lengthOfName;
    gctUINT                      foTableEntryCount = 0, foTableEntryIdx = 0;
    PROG_FRAGOUT_TABLE*          pFoTable = &pOutPEP->fragOutTable;
    PROG_FRAGOUT_TABLE_ENTRY*    pFoEntry;
    gctSTRING                    name;

    /* Calc how many entries we need */
    for (virIoIdx = 0; virIoIdx < virIoCount; virIoIdx ++)
    {
        pVirIoSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pFragoutList, virIoIdx));

        if (!isSymUnused(pVirIoSym) && !isSymVectorizedOut(pVirIoSym))
        {
            foTableEntryCount ++;
        }
    }

    /* Allocate entries if needed */
    if (foTableEntryCount > 0)
    {
        if (gcoOS_Allocate(gcvNULL,
                       sizeof(PROG_FRAGOUT_TABLE_ENTRY) * foTableEntryCount,
                       (gctPOINTER*)&pFoTable->pFragOutEntries) != gcvSTATUS_OK)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }

        memset(pFoTable->pFragOutEntries, 0, sizeof(PROG_FRAGOUT_TABLE_ENTRY) * foTableEntryCount);

        pFoTable->countOfEntries = foTableEntryCount;
    }

    /* Now fill the each entry */
    for (virIoIdx = 0; virIoIdx < virIoCount; virIoIdx ++)
    {
        pVirIoSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pFragoutList, virIoIdx));

        if (!isSymUnused(pVirIoSym) && !isSymVectorizedOut(pVirIoSym))
        {
            pFoEntry = &pFoTable->pFragOutEntries[foTableEntryIdx];
            pFoEntry->fragOutEntryIndex = foTableEntryIdx;

            /* Type */
            pVirIoType = VIR_Symbol_GetType(pVirIoSym);
            pElementType = pVirIoType;

            while (VIR_Type_isArray(pElementType))
            {
                pElementType = VIR_Shader_GetTypeFromId(pShader, VIR_Type_GetBaseTypeId(pElementType));
            }
            gcmASSERT(VIR_Type_isPrimitive(pElementType));
            pFoEntry->type = (VSC_SHADER_DATA_TYPE)VIR_Type_GetIndex(pElementType);

            switch (VIR_GetTypeSize(VIR_GetTypeComponentType(VIR_Type_GetIndex(pElementType))))
            {
            case 4:
                pFoEntry->resEntryBit |= VSC_RES_ENTRY_BIT_32BIT;
                break;

            case 2:
                pFoEntry->resEntryBit |= VSC_RES_ENTRY_BIT_16BIT;
                break;

            case 1:
                pFoEntry->resEntryBit |= VSC_RES_ENTRY_BIT_8BIT;
                break;

            default:
                break;
            }

            /* Name */
            name = VIR_Shader_GetSymNameString(pShader, pVirIoSym);
            lengthOfName = gcoOS_StrLen(name, gcvNULL);
            if (gcoOS_Allocate(gcvNULL, lengthOfName + 1, (gctPOINTER*)&pFoEntry->name) != gcvSTATUS_OK)
            {
                return VSC_ERR_OUT_OF_MEMORY;
            }
            memcpy((gctPOINTER)pFoEntry->name, name, lengthOfName + 1);
            pFoEntry->nameLength = lengthOfName;

            /* LL IO-Mapping */
            ioIdx = VIR_Symbol_GetFirstSlot(pVirIoSym);
            pFoEntry->pIoRegMapping = &pSEP->inputMapping.ioVtxPxl.pIoRegMapping[ioIdx];

            /* Location */
            locationCount = VIR_Symbol_GetVirIoRegCount(pShader, pVirIoSym);
            pFoEntry->locationCount = locationCount;
            if (gcoOS_Allocate(gcvNULL, locationCount * sizeof(gctUINT), (gctPOINTER*)&pFoEntry->pLocation) != gcvSTATUS_OK)
            {
                return VSC_ERR_OUT_OF_MEMORY;
            }
            for (i = 0; i < locationCount; i ++)
            {
                pFoEntry->pLocation[i] = VIR_Symbol_GetLocation(pVirIoSym) + i;
            }

            /* Vec4 mask */
            pFoEntry->vec4BasedCount = locationCount;
            for (i = 0; i < pFoEntry->vec4BasedCount; i ++)
            {
                pFoEntry->activeVec4Mask |= (1 << i);
            }

            /* Go on to next entry */
            foTableEntryIdx ++;
        }
    }

    gcmASSERT(foTableEntryIdx == foTableEntryCount);

    return errCode;
}

VSC_RES_OP_BIT _VirResOpType2DrviResOpBit(gctUINT resOpType)
{
    switch (resOpType)
    {
    case VIR_RES_OP_TYPE_TEXLD:
        return VSC_RES_OP_BIT_TEXLD;

    case VIR_RES_OP_TYPE_TEXLD_BIAS:
        return VSC_RES_OP_BIT_TEXLD_BIAS;

    case VIR_RES_OP_TYPE_TEXLD_PCF:
        return VSC_RES_OP_BIT_TEXLD_PCF | VSC_RES_OP_BIT_TEXLD;

    case VIR_RES_OP_TYPE_TEXLD_LOD:
        return VSC_RES_OP_BIT_TEXLD_LOD;

    case VIR_RES_OP_TYPE_TEXLD_BIAS_PCF:
        return VSC_RES_OP_BIT_TEXLD_BIAS_PCF | VSC_RES_OP_BIT_TEXLD_BIAS;

    case VIR_RES_OP_TYPE_TEXLD_LOD_PCF:
        return VSC_RES_OP_BIT_TEXLD_LOD_PCF | VSC_RES_OP_BIT_TEXLD_LOD;

    case VIR_RES_OP_TYPE_TEXLD_GRAD:
        return VSC_RES_OP_BIT_TEXLD_GRAD;

    case VIR_RES_OP_TYPE_TEXLDP:
        return VSC_RES_OP_BIT_TEXLDP;

    case VIR_RES_OP_TYPE_TEXLDP_GRAD:
        return VSC_RES_OP_BIT_TEXLDP_GRAD;

    case VIR_RES_OP_TYPE_TEXLDP_BIAS:
        return VSC_RES_OP_BIT_TEXLDP_BIAS;

    case VIR_RES_OP_TYPE_TEXLDP_LOD:
        return VSC_RES_OP_BIT_TEXLDP_LOD;

    case VIR_RES_OP_TYPE_FETCH:
        return VSC_RES_OP_BIT_FETCH;

    case VIR_RES_OP_TYPE_FETCH_MS:
        return VSC_RES_OP_BIT_FETCH_MS;

    case VIR_RES_OP_TYPE_GATHER:
        return VSC_RES_OP_BIT_GATHER;

    case VIR_RES_OP_TYPE_GATHER_PCF:
        return VSC_RES_OP_BIT_GATHER_PCF;

    case VIR_RES_OP_TYPE_LODQ:
        return VSC_RES_OP_BIT_LODQ;

    case VIR_RES_OP_TYPE_LOAD_STORE:
        return VSC_RES_OP_BIT_LOAD_STORE;

    case VIR_RES_OP_TYPE_IMAGE_OP:
        return VSC_RES_OP_BIT_IMAGE_OP;

    case VIR_RES_OP_TYPE_ATOMIC:
        return VSC_RES_OP_BIT_ATOMIC;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    return (VSC_RES_OP_BIT)0;
}

static VSC_RES_OP_BIT _TranslateResOpBits(gctUINT32 resOpBitsInUniform)
{
    gctUINT         resOpType;
    VSC_RES_OP_BIT  resultResOpBits = 0;

    for (resOpType = 0; resOpType < VIR_RES_OP_TYPE_COUNT; resOpType ++)
    {
        if (resOpBitsInUniform & (1 << resOpType))
        {
            resultResOpBits |= _VirResOpType2DrviResOpBit(resOpType);
        }
    }

    return resultResOpBits;
}

static gctBOOL _SetResOpBits(VIR_Shader* pShader,
                          VSC_SHADER_RESOURCE_BINDING* pResBinding,
                          VSC_RES_OP_BIT** ppResOpBits)
{
    gctUINT                                   virUniformIdx, subUniformIdx;
    gctUINT                                   resArraySize;
    VIR_Symbol*                               pVirUniformSym;
    VIR_Uniform*                              pVirUniform;
    VIR_UniformIdList*                        pVirUniformLsts = VIR_Shader_GetUniforms(pShader);
    VIR_Type*                                 pVirUniformSymType;
    VSC_RES_OP_BIT*                           pResOpBits = *ppResOpBits;

    if (pResOpBits == gcvNULL)
    {
        if (gcoOS_Allocate(gcvNULL, sizeof(VSC_RES_OP_BIT) * pResBinding->arraySize, (gctPOINTER*)&pResOpBits) != gcvSTATUS_OK)
        {
            return gcvFALSE;
        }
        memset(pResOpBits, 0, sizeof(VSC_RES_OP_BIT) * pResBinding->arraySize);
    }

    for (virUniformIdx = 0; virUniformIdx < VIR_IdList_Count(pVirUniformLsts); virUniformIdx ++)
    {
        pVirUniformSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pVirUniformLsts, virUniformIdx));
        pVirUniform = VIR_Symbol_GetUniformPointer(pShader, pVirUniformSym);

        if (pVirUniform == gcvNULL)
        {
            continue;
        }

        if (VIR_Uniform_GetResOpBitsArray(pVirUniform) == gcvNULL)
        {
            continue;
        }

        resArraySize = 1;
        pVirUniformSymType = VIR_Symbol_GetType(pVirUniformSym);
        while (VIR_Type_isArray(pVirUniformSymType))
        {
            resArraySize *= VIR_Type_GetArrayLength(pVirUniformSymType);
            pVirUniformSymType = VIR_Shader_GetTypeFromId(pShader, VIR_Type_GetBaseTypeId(pVirUniformSymType));
        }

        if (VIR_Symbol_GetDescriptorSet(pVirUniformSym) == pResBinding->set &&
            VIR_Symbol_GetBinding(pVirUniformSym) == pResBinding->binding &&
            resArraySize == pResBinding->arraySize)
        {
            for (subUniformIdx = 0; subUniformIdx < (gctUINT)pVirUniform->realUseArraySize; subUniformIdx ++)
            {
                pResOpBits[subUniformIdx] |= _TranslateResOpBits(VIR_Uniform_GetResOpBits(pVirUniform, subUniformIdx));
            }
        }
    }

    if (*ppResOpBits == gcvNULL)
    {
        *ppResOpBits = pResOpBits;
    }
    return gcvTRUE;
}

static void _GetImageFormat(VIR_Shader* pShader,
                            VSC_SHADER_RESOURCE_BINDING* pResBinding,
                            PROG_VK_IMAGE_FORMAT_INFO* pImageFormatInfo)
{
    gctUINT                                   virUniformIdx;
    gctUINT                                   resArraySize;
    VIR_Symbol*                               pVirUniformSym;
    VIR_Uniform*                              pVirUniform;
    VIR_UniformIdList*                        pVirUniformLsts = VIR_Shader_GetUniforms(pShader);
    VIR_Type*                                 pVirUniformSymType;
    VSC_IMAGE_FORMAT                          imageFormat = VSC_IMAGE_FORMAT_NONE;
    gctBOOL                                   bSetInSpirv = gcvTRUE;

    for (virUniformIdx = 0; virUniformIdx < VIR_IdList_Count(pVirUniformLsts); virUniformIdx ++)
    {
        pVirUniformSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pVirUniformLsts, virUniformIdx));
        pVirUniform = VIR_Symbol_GetUniformPointer(pShader, pVirUniformSym);

        if (pVirUniform == gcvNULL)
        {
            continue;
        }

        resArraySize = 1;
        pVirUniformSymType = VIR_Symbol_GetType(pVirUniformSym);
        while (VIR_Type_isArray(pVirUniformSymType))
        {
            resArraySize *= VIR_Type_GetArrayLength(pVirUniformSymType);
            pVirUniformSymType = VIR_Shader_GetTypeFromId(pShader, VIR_Type_GetBaseTypeId(pVirUniformSymType));
        }

        if (VIR_Symbol_GetDescriptorSet(pVirUniformSym) == pResBinding->set &&
            VIR_Symbol_GetBinding(pVirUniformSym) == pResBinding->binding &&
            resArraySize == pResBinding->arraySize)
        {
            imageFormat = (VSC_IMAGE_FORMAT)VIR_Symbol_GetImageFormat(pVirUniformSym);

            /* If there is no image format information in shader, compiler uses 16bit as the default value. */
            if (imageFormat == VSC_IMAGE_FORMAT_NONE)
            {
                bSetInSpirv = gcvFALSE;

                if (VIR_TypeId_isImageDataUnSignedInteger(VIR_Type_GetIndex(pVirUniformSymType)))
                {
                    imageFormat = VSC_IMAGE_FORMAT_RGBA16UI;
                }
                else if (VIR_TypeId_isImageDataSignedInteger(VIR_Type_GetIndex(pVirUniformSymType)))
                {
                    imageFormat = VSC_IMAGE_FORMAT_RGBA16I;
                }
                else
                {
                    imageFormat = VSC_IMAGE_FORMAT_RGBA16F;
                }
            }

            if (pImageFormatInfo)
            {
                pImageFormatInfo->imageFormat = imageFormat;
                pImageFormatInfo->bSetInSpriv = bSetInSpirv;
            }
            break;
        }
    }

    return;
}

static PROG_VK_RESOURCE_SET* _GetVkResourceSetBySetIdx(PROGRAM_EXECUTABLE_PROFILE* pPEP, gctUINT setIndex)
{
    void*                          pOldResSet;
    gctUINT                        oldCountOfSets, newCountOfSet;

    if (setIndex >= pPEP->u.vk.resourceSetCount)
    {
        pOldResSet = pPEP->u.vk.pResourceSets;
        oldCountOfSets = pPEP->u.vk.resourceSetCount;

        newCountOfSet = setIndex + 1;
        if (gcoOS_Allocate(gcvNULL, newCountOfSet * sizeof(PROG_VK_RESOURCE_SET),
            (gctPOINTER*)&pPEP->u.vk.pResourceSets) != gcvSTATUS_OK)
        {
            gcmPRINT("Failed to allocate memory in GetVkResourceSetBySetIdx.");
            return gcvNULL;
        }
        pPEP->u.vk.resourceSetCount = newCountOfSet;

        if (pOldResSet)
        {
            memcpy(pPEP->u.vk.pResourceSets, pOldResSet, oldCountOfSets * sizeof(PROG_VK_RESOURCE_SET));
            gcoOS_Free(gcvNULL, pOldResSet);
        }

        memset(pPEP->u.vk.pResourceSets + oldCountOfSets, 0,
               sizeof(PROG_VK_RESOURCE_SET) * (newCountOfSet - oldCountOfSets));
    }

    return &pPEP->u.vk.pResourceSets[setIndex];
}

static gctBOOL _CheckTwoResBindingsAreSame(VSC_SHADER_RESOURCE_BINDING* pResBinding1, VSC_SHADER_RESOURCE_BINDING* pResBinding2)
{
    return (pResBinding1->arraySize == pResBinding2->arraySize &&
            pResBinding1->type == pResBinding2->type &&
            pResBinding1->set == pResBinding2->set &&
            pResBinding1->binding == pResBinding2->binding);

}

static PROG_VK_COMBINED_TEX_SAMPLER_TABLE_ENTRY* _enlargeVkCombTsEntryRoom(PROG_VK_COMBINED_TEXTURE_SAMPLER_TABLE* pCombinedSampTexTable,
                                                                           gctUINT enlargeCount,
                                                                           gctUINT* pStartEntryIdx)
{
    void*                         pOldCombTsEntry;
    gctUINT                       oldCombTsEntryCount, newCombTsEntryCount;

    pOldCombTsEntry = pCombinedSampTexTable->pCombTsEntries;
    oldCombTsEntryCount = pCombinedSampTexTable->countOfEntries;
    newCombTsEntryCount = oldCombTsEntryCount + enlargeCount;
    if (gcoOS_Allocate(gcvNULL,
                   sizeof(PROG_VK_COMBINED_TEX_SAMPLER_TABLE_ENTRY) * newCombTsEntryCount,
                   (gctPOINTER*)&pCombinedSampTexTable->pCombTsEntries) != gcvSTATUS_OK)
    {
        return gcvNULL;
    }

    if (pOldCombTsEntry)
    {
        memcpy(pCombinedSampTexTable->pCombTsEntries, pOldCombTsEntry,
               oldCombTsEntryCount*sizeof(PROG_VK_COMBINED_TEX_SAMPLER_TABLE_ENTRY));

        gcoOS_Free(gcvNULL, pOldCombTsEntry);
    }

    pCombinedSampTexTable->countOfEntries = newCombTsEntryCount;

    if (pStartEntryIdx)
    {
        *pStartEntryIdx = oldCombTsEntryCount;
    }

    /* Return first enlarged combined sampler texture entry */
    return &pCombinedSampTexTable->pCombTsEntries[oldCombTsEntryCount];
}

static VSC_ErrCode _AddExtraSamplerArray(SHADER_PRIV_SAMPLER_ENTRY*** pppExtraSamplerArray,
                                         VSC_SHADER_RESOURCE_BINDING* pResBinding,
                                         VIR_Shader* pShader,
                                         SHADER_EXECUTABLE_PROFILE* pSep,
                                         gctBOOL bCheckSeparateImage,
                                         gctBOOL bCheckSeparateSampler,
                                         gctINT arraySize,
                                         gctINT arrayStride,
                                         gctINT offset)
{
    VSC_ErrCode                               errCode = VSC_ERR_NONE;
    gctUINT                                   i;
    gctUINT                                   resArraySize, resArrayIndex;
    VIR_Symbol*                               pVirUniformSym;
    VIR_Symbol*                               pVirUniformSymParent;
    VIR_Uniform*                              pVirUniform;
    VIR_Type*                                 pVirUniformSymType;
    gctBOOL                                   matched = gcvFALSE;
    SHADER_PRIV_SAMPLER_ENTRY*                pPrivSamplerEntry;
    SHADER_PRIV_SAMPLER_ENTRY**               ppExtraSamplerArray = *pppExtraSamplerArray;

    for (i = 0; i < pSep->dynamicPrivMapping.privSamplerMapping.countOfEntries; i ++)
    {
        matched = gcvFALSE;
        pPrivSamplerEntry = &pSep->dynamicPrivMapping.privSamplerMapping.pPrivSamplerEntries[i];

        if (pPrivSamplerEntry->commonPrivm.privmKind == VSC_LIB_LINK_TYPE_RESOURCE)
        {
            pVirUniformSym = (VIR_Symbol*)pPrivSamplerEntry->commonPrivm.pPrivateData;

            gcmASSERT(pVirUniformSym);
            gcmASSERT(VIR_Symbol_isSampler(pVirUniformSym));

            pVirUniform = VIR_Symbol_GetSampler(pVirUniformSym);

            pVirUniformSymParent = VIR_Shader_GetSymFromId(pShader, pVirUniform->u.samplerOrImageAttr.parentSamplerSymId);

            pVirUniformSymType = VIR_Symbol_GetType(pVirUniformSymParent);
            if (VIR_Type_GetKind(pVirUniformSymType) == VIR_TY_ARRAY)
            {
                resArraySize = VIR_Type_GetArrayLength(pVirUniformSymType);
            }
            else
            {
                resArraySize = 1;
            }

            if (VIR_Symbol_GetDescriptorSet(pVirUniformSymParent) == pResBinding->set &&
                VIR_Symbol_GetBinding(pVirUniformSymParent) == pResBinding->binding &&
                resArraySize == pResBinding->arraySize)
            {
                matched = gcvTRUE;
            }
            else if ((bCheckSeparateImage || bCheckSeparateSampler)
                     &&
                     (VIR_Symbol_GetUniformKind(pVirUniformSymParent) == VIR_UNIFORM_SAMPLED_IMAGE))
            {
                VIR_Symbol*   pSparateImageSym = gcvNULL;
                VIR_Symbol*   pSeparateSamplerSym = gcvNULL;

                if (bCheckSeparateImage)
                {
                    pSparateImageSym = VIR_Symbol_GetSeparateImage(pShader, pVirUniformSymParent);
                    if (pSparateImageSym != gcvNULL)
                    {
                        if (VIR_Symbol_GetDescriptorSet(pSparateImageSym) == pResBinding->set &&
                            VIR_Symbol_GetBinding(pSparateImageSym) == pResBinding->binding &&
                            resArraySize == pResBinding->arraySize)
                        {
                            matched = gcvTRUE;
                        }
                    }
                }
                else if (!matched && bCheckSeparateSampler)
                {
                    pSeparateSamplerSym = VIR_Symbol_GetHwMappingSeparateSampler(pShader, pVirUniformSymParent);
                    if (pSeparateSamplerSym != gcvNULL)
                    {
                        if (VIR_Symbol_GetDescriptorSet(pSeparateSamplerSym) == pResBinding->set &&
                            VIR_Symbol_GetBinding(pSeparateSamplerSym) == pResBinding->binding &&
                            resArraySize == pResBinding->arraySize)
                        {
                            matched = gcvTRUE;
                        }
                    }
                }
            }

            if (matched)
            {
                if (arraySize != -1)
                {
                    resArraySize = arraySize;
                }

                if (ppExtraSamplerArray == gcvNULL)
                {
                    if (gcoOS_Allocate(gcvNULL, sizeof(SHADER_PRIV_SAMPLER_ENTRY*) * resArraySize,
                        (gctPOINTER*)&ppExtraSamplerArray) != gcvSTATUS_OK)
                    {
                        return VSC_ERR_OUT_OF_MEMORY;
                    }
                    memset(ppExtraSamplerArray, 0,
                            sizeof(SHADER_PRIV_SAMPLER_ENTRY*) * resArraySize);
                }
                /*
                ** Since we don't check the array index right now, so arrayIdxInParent is invalid.
                */
                /* resArrayIndex = pVirUniform->u.samplerOrImageAttr.arrayIdxInParent; */
                resArrayIndex = pPrivSamplerEntry->commonPrivm.privmKindIndex -
                                VIR_Symbol_GetFirstSlot(VIR_Shader_GetSymFromId(pShader, pVirUniform->u.samplerOrImageAttr.parentSamplerSymId));

                gcmASSERT(resArrayIndex < resArraySize);
                gcmASSERT(ppExtraSamplerArray[resArrayIndex] == gcvNULL);

                ppExtraSamplerArray[resArrayIndex * arrayStride + offset] = pPrivSamplerEntry;
            }
        }
    }

    *pppExtraSamplerArray = ppExtraSamplerArray;

    return errCode;
}

static VSC_ErrCode _AddTextureSizeAndLodMinMax(SHADER_EXECUTABLE_PROFILE*       pSep,
                                               VIR_Shader*                      pShader,
                                               gctBOOL                          bCheckSeparateImage,
                                               gctBOOL                          bCheckSeparateSampler,
                                               SHADER_PRIV_CONSTANT_ENTRY**     ppTextureSize,
                                               SHADER_PRIV_CONSTANT_ENTRY**     ppLodMinMax,
                                               SHADER_PRIV_CONSTANT_ENTRY**     ppLevelsSamples,
                                               VSC_SHADER_RESOURCE_BINDING*     pBinding)
{
    VSC_ErrCode                     errCode = VSC_ERR_NONE;
    SHADER_PRIV_CONSTANT_ENTRY*     pPrivCnstEntry;
    gctUINT                         resArrayLength;
    VIR_Symbol*                     pTextureSym;
    VIR_Type*                       pSymType;
    gctUINT                         i;
    gctBOOL                         bMatch = gcvFALSE;
    VIR_Symbol*                     pSparateImageSym = gcvNULL;
    VIR_Symbol*                     pSeparateSamplerSym = gcvNULL;

    if (ppTextureSize == gcvNULL && ppLodMinMax == gcvNULL && ppLevelsSamples == gcvNULL)
    {
        return errCode;
    }

    for (i = 0; i < pSep->staticPrivMapping.privConstantMapping.countOfEntries; i ++)
    {
        pPrivCnstEntry = &pSep->staticPrivMapping.privConstantMapping.pPrivmConstantEntries[i];

        /* Check TextureSize/LodMinMax/LevelsSamples*/
        if (pPrivCnstEntry->commonPrivm.privmKind != SHS_PRIV_CONSTANT_KIND_TEXTURE_SIZE    &&
            pPrivCnstEntry->commonPrivm.privmKind != SHS_PRIV_CONSTANT_KIND_LOD_MIN_MAX     &&
            pPrivCnstEntry->commonPrivm.privmKind != SHS_PRIV_CONSTANT_KIND_LEVELS_SAMPLES)
        {
            continue;
        }

        /* Get the corresponding texture symbol. */
        pTextureSym = (VIR_Symbol*)pPrivCnstEntry->commonPrivm.pPrivateData;
        if (pTextureSym == gcvNULL)
        {
            gcmASSERT(gcvFALSE);
            continue;
        }

        /* Get the array length. */
        pSymType = VIR_Symbol_GetType(pTextureSym);
        if (VIR_Type_GetKind(pSymType) == VIR_TY_ARRAY)
        {
            resArrayLength = VIR_Type_GetArrayLength(pSymType);
        }
        else
        {
            resArrayLength = 1;
        }

        /* Check the binding information. */
        if (VIR_Symbol_GetDescriptorSet(pTextureSym) == pBinding->set &&
            VIR_Symbol_GetBinding(pTextureSym) == pBinding->binding &&
            resArrayLength == pBinding->arraySize)
        {
            bMatch = gcvTRUE;
        }
        /* For a sampled image, we need to check the separate sampler. */
        else if ((bCheckSeparateImage || bCheckSeparateSampler)
                 &&
                 (VIR_Symbol_GetUniformKind(pTextureSym) == VIR_UNIFORM_SAMPLED_IMAGE))
        {
            if (bCheckSeparateImage)
            {
                pSparateImageSym = VIR_Symbol_GetSeparateImage(pShader, pTextureSym);
                if (pSparateImageSym != gcvNULL)
                {
                    if (VIR_Symbol_GetDescriptorSet(pSparateImageSym) == pBinding->set &&
                        VIR_Symbol_GetBinding(pSparateImageSym) == pBinding->binding &&
                        resArrayLength == pBinding->arraySize)
                    {
                        bMatch = gcvTRUE;
                    }
                }
            }
            else if (!bMatch && bCheckSeparateSampler)
            {
                pSeparateSamplerSym = VIR_Symbol_GetHwMappingSeparateSampler(pShader, pTextureSym);
                if (pSeparateSamplerSym != gcvNULL)
                {
                    if (VIR_Symbol_GetDescriptorSet(pSeparateSamplerSym) == pBinding->set &&
                        VIR_Symbol_GetBinding(pSeparateSamplerSym) == pBinding->binding &&
                        resArrayLength == pBinding->arraySize)
                    {
                        bMatch = gcvTRUE;
                    }
                }
            }
        }

        /* No found. */
        if (!bMatch)
        {
            continue;
        }

        switch (pPrivCnstEntry->commonPrivm.privmKind)
        {
        case SHS_PRIV_CONSTANT_KIND_TEXTURE_SIZE:
            if (ppTextureSize)
            {
                if ((ppTextureSize)[0] == gcvNULL)
                {
                    (ppTextureSize)[0] = pPrivCnstEntry;
                }
                else
                {
                    gcmASSERT((ppTextureSize)[1] == gcvNULL);
                    (ppTextureSize)[1] = pPrivCnstEntry;
                }
            }
            break;

        case SHS_PRIV_CONSTANT_KIND_LOD_MIN_MAX:
            if (ppLodMinMax)
            {
                if ((ppLodMinMax)[0] == gcvNULL)
                {
                    (ppLodMinMax)[0] = pPrivCnstEntry;
                }
                else
                {
                    gcmASSERT((ppLodMinMax)[1] == gcvNULL);
                    (ppLodMinMax)[1] = pPrivCnstEntry;
                }
            }
            break;

        case SHS_PRIV_CONSTANT_KIND_LEVELS_SAMPLES:
            if (ppLevelsSamples)
            {
                if ((ppLevelsSamples)[0] == gcvNULL)
                {
                    (ppLevelsSamples)[0] = pPrivCnstEntry;
                }
                else
                {
                    gcmASSERT((ppLevelsSamples)[1] == gcvNULL);
                    (ppLevelsSamples)[1] = pPrivCnstEntry;
                }
            }
            break;

        default:
            gcmASSERT(gcvFALSE);
            break;
        }
    }

    return errCode;
}

static VSC_ErrCode _AddPrivateImageUniform(SHADER_PRIV_UAV_ENTRY**          ppPrivateImageUniform,
                                           VSC_SHADER_RESOURCE_BINDING*     pBinding,
                                           SHADER_EXECUTABLE_PROFILE*       pSep,
                                           SHS_PRIV_MEM_KIND                memKind,
                                           gctBOOL                          bCheckPrivmKindIndex,
                                           gctUINT                          privmKindIndex)
{
    VSC_ErrCode                     errCode = VSC_ERR_NONE;
    SHADER_PRIV_UAV_ENTRY *         pPrivUavEntry;
    gctUINT                         resArraySize;
    VIR_Symbol*                     pImageSym;
    VIR_Type*                       pSymType;
    gctUINT                         i, firstPrivUavIdxForExtraImgLayer = NOT_ASSIGNED;

    if (ppPrivateImageUniform == gcvNULL)
    {
        return errCode;
    }

    for (i = 0; i < pSep->staticPrivMapping.privUavMapping.countOfEntries; i ++)
    {
        pPrivUavEntry = &pSep->staticPrivMapping.privUavMapping.pPrivUavEntries[i];

        if ((SHS_PRIV_MEM_KIND)pPrivUavEntry->commonPrivm.privmKind == memKind
            &&
            (!bCheckPrivmKindIndex || (pPrivUavEntry->commonPrivm.privmKindIndex == privmKindIndex)))
        {
            pImageSym = (VIR_Symbol*)pPrivUavEntry->commonPrivm.pPrivateData;

            gcmASSERT(pImageSym);

            pSymType = VIR_Symbol_GetType(pImageSym);
            if (VIR_Type_GetKind(pSymType) == VIR_TY_ARRAY)
            {
                resArraySize = VIR_Type_GetArrayLength(pSymType);
            }
            else
            {
                resArraySize = 1;
            }

            if (VIR_Symbol_GetDescriptorSet(pImageSym) == pBinding->set &&
                VIR_Symbol_GetBinding(pImageSym) == pBinding->binding &&
                resArraySize == pBinding->arraySize)
            {
                if (ppPrivateImageUniform)
                {
                    if (*ppPrivateImageUniform == gcvNULL)
                    {
                        *ppPrivateImageUniform= pPrivUavEntry;
                        firstPrivUavIdxForExtraImgLayer = i;
                    }
                    else
                    {
                        if (pPrivUavEntry->pBuffer->uavSlotIndex -
                            (*ppPrivateImageUniform)->pBuffer->uavSlotIndex !=
                            (i - firstPrivUavIdxForExtraImgLayer))
                        {
                            gcmASSERT(gcvFALSE);
                        }
                    }
                }

                break;
            }
        }
    }

    return errCode;
}

static VSC_ErrCode _AddImageSizeAndMipmapLevel(SHADER_PRIV_CONSTANT_ENTRY**   ppImageSize,
                                               SHADER_PRIV_CONSTANT_ENTRY**   ppMipLevel,
                                               VSC_SHADER_RESOURCE_BINDING*   pBinding,
                                               SHADER_EXECUTABLE_PROFILE*     pSep)
{
    VSC_ErrCode                     errCode = VSC_ERR_NONE;
    SHADER_PRIV_CONSTANT_ENTRY*     pPrivCnstEntry;
    gctUINT                         resArraySize;
    VIR_Symbol*                     pImageSym;
    VIR_Type*                       pSymType;
    gctUINT                         i, findCount = 0;

    if (ppImageSize == gcvNULL && ppMipLevel == gcvNULL)
    {
        return errCode;
    }

    for (i = 0; i < pSep->staticPrivMapping.privConstantMapping.countOfEntries; i ++)
    {
        pPrivCnstEntry = &pSep->staticPrivMapping.privConstantMapping.pPrivmConstantEntries[i];

        if (pPrivCnstEntry->commonPrivm.privmKind == SHS_PRIV_CONSTANT_KIND_IMAGE_SIZE
            ||
            pPrivCnstEntry->commonPrivm.privmKind == SHS_PRIV_CONSTANT_KIND_LEVELS_SAMPLES)
        {
            pImageSym = (VIR_Symbol*)pPrivCnstEntry->commonPrivm.pPrivateData;

            gcmASSERT(pImageSym);

            pSymType = VIR_Symbol_GetType(pImageSym);
            if (VIR_Type_GetKind(pSymType) == VIR_TY_ARRAY)
            {
                resArraySize = VIR_Type_GetArrayLength(pSymType);
            }
            else
            {
                resArraySize = 1;
            }

            if (VIR_Symbol_GetDescriptorSet(pImageSym) == pBinding->set &&
                VIR_Symbol_GetBinding(pImageSym) == pBinding->binding &&
                resArraySize == pBinding->arraySize)
            {
                if (pPrivCnstEntry->commonPrivm.privmKind == SHS_PRIV_CONSTANT_KIND_IMAGE_SIZE)
                {
                    if (ppImageSize)
                    {
                        *ppImageSize = pPrivCnstEntry;
                    }
                }
                else
                {
                    if (ppMipLevel)
                    {
                        *ppMipLevel = pPrivCnstEntry;
                    }
                }

                findCount++;
                if (findCount == 2)
                {
                    break;
                }
            }
        }
    }

    return errCode;
}

static VSC_ErrCode _FillImageDerivedInfo(VIR_Shader* pShader,
                                         VSC_SHADER_RESOURCE_BINDING* pBinding,
                                         SHADER_EXECUTABLE_PROFILE* pSep,
                                         PROG_VK_IMAGE_DERIVED_INFO* pImageDerivedInfo)
{
    /* Set image size  and mip level. */
    _AddImageSizeAndMipmapLevel(&pImageDerivedInfo->pImageSize,
                                &pImageDerivedInfo->pMipLevel,
                                pBinding,
                                pSep);

    /* Set image format. */
    _GetImageFormat(pShader, pBinding, &pImageDerivedInfo->imageFormatInfo);

    /* Extra image layer */
    _AddPrivateImageUniform(&pImageDerivedInfo->pExtraLayer,
                            pBinding,
                            pSep,
                            SHS_PRIV_MEM_KIND_EXTRA_UAV_LAYER,
                            gcvFALSE,
                            0);

    return VSC_ERR_NONE;
}

static VSC_ErrCode _FillSamplerDerivedInfo(VIR_Shader* pShader,
                                           VSC_SHADER_RESOURCE_BINDING* pBinding,
                                           SHADER_EXECUTABLE_PROFILE* pSep,
                                           PROG_VK_SAMPLER_DERIVED_INFO* pSamplerDerivedInfo,
                                           gctBOOL bCheckSeparateImage,
                                           gctBOOL bCheckSeparateSampler)
{
    /* Set textureSize/lodMinMax/levelsSamples */
    _AddTextureSizeAndLodMinMax(pSep,
                                pShader,
                                bCheckSeparateImage,
                                bCheckSeparateSampler,
                                pSamplerDerivedInfo->pTextureSize,
                                pSamplerDerivedInfo->pLodMinMax,
                                pSamplerDerivedInfo->pLevelsSamples,
                                pBinding);

    return VSC_ERR_NONE;
}

static PROG_VK_SEPARATED_SAMPLER_TABLE_ENTRY* _enlargeVkSeparatedSamplerEntryRoom(PROG_VK_SEPARATED_SAMPLER_TABLE* pSeparatedSamplerTable,
                                                                                  gctUINT enlargeCount,
                                                                                  gctUINT* pStartEntryIdx)
{
    void*                         pOldSamplerEntry;
    gctUINT                       oldSamplerEntryCount, newSamplerEntryCount;

    pOldSamplerEntry = pSeparatedSamplerTable->pSamplerEntries;
    oldSamplerEntryCount = pSeparatedSamplerTable->countOfEntries;
    newSamplerEntryCount = oldSamplerEntryCount + enlargeCount;
    if (gcoOS_Allocate(gcvNULL,
                   sizeof(PROG_VK_SEPARATED_SAMPLER_TABLE_ENTRY) * newSamplerEntryCount,
                   (gctPOINTER*)&pSeparatedSamplerTable->pSamplerEntries) != gcvSTATUS_OK)
    {
        return gcvNULL;
    }

    if (pOldSamplerEntry)
    {
        memcpy(pSeparatedSamplerTable->pSamplerEntries, pOldSamplerEntry,
               oldSamplerEntryCount*sizeof(PROG_VK_SEPARATED_SAMPLER_TABLE_ENTRY));

        gcoOS_Free(gcvNULL, pOldSamplerEntry);
    }

    pSeparatedSamplerTable->countOfEntries = newSamplerEntryCount;

    if (pStartEntryIdx)
    {
        *pStartEntryIdx = oldSamplerEntryCount;
    }

    /* Return first enlarged Sampler entry */
    return &pSeparatedSamplerTable->pSamplerEntries[oldSamplerEntryCount];
}

static VSC_ErrCode _AddVkSeparatedSamplerEntryToSeparatedSamplerTableOfPEP(VSC_PEP_GEN_HELPER* pPepGenHelper,
                                                                           PROG_VK_RESOURCE_SET* pResSet,
                                                                           PROG_VK_SEPARATED_SAMPLER_TABLE* pSeparatedSamplerTable,
                                                                           VIR_SHADER_RESOURCE_ALLOC_ENTRY* pResAllocEntry,
                                                                           VIR_Shader* pShader,
                                                                           gctUINT stageIdx,
                                                                           SHADER_EXECUTABLE_PROFILE* pSep)
{
    PROG_VK_SEPARATED_SAMPLER_TABLE_ENTRY* pSamplerEntry = gcvNULL;
    gctUINT                                i, SamplerEntryIndex;

    for (i = 0; i < pSeparatedSamplerTable->countOfEntries; i ++)
    {
        if (_CheckTwoResBindingsAreSame(&pSeparatedSamplerTable->pSamplerEntries[i].samplerBinding, &pResAllocEntry->resBinding))
        {
            pSamplerEntry = &pSeparatedSamplerTable->pSamplerEntries[i];
            break;
        }
    }

    if (pSamplerEntry == gcvNULL)
    {
        pSamplerEntry = _enlargeVkSeparatedSamplerEntryRoom(pSeparatedSamplerTable, 1, &SamplerEntryIndex);
        if (pSamplerEntry == gcvNULL)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
        memset(pSamplerEntry, 0, sizeof(PROG_VK_SEPARATED_SAMPLER_TABLE_ENTRY));
        pSamplerEntry->samplerEntryIndex = SamplerEntryIndex;
        memcpy(&pSamplerEntry->samplerBinding, &pResAllocEntry->resBinding, sizeof(VSC_SHADER_RESOURCE_BINDING));
    }

    if (pResAllocEntry->hwRegNo != NOT_ASSIGNED)
    {
        pSamplerEntry->activeStageMask |= pResAllocEntry->bUse ? (1 << stageIdx) : 0;
        pSamplerEntry->stageBits |= VSC_SHADER_STAGE_2_STAGE_BIT(stageIdx);
        pSamplerEntry->hwMappings[stageIdx].samplerMapping.hwSamplerSlot = pResAllocEntry->hwRegNo;
    }
    else
    {
        pSamplerEntry->hwMappings[stageIdx].samplerMapping.hwSamplerSlot = NOT_ASSIGNED;
    }

    pSamplerEntry->bUsingHwMppingList = gcvFALSE;

    if (_SetResOpBits(pShader, &pSamplerEntry->samplerBinding, &pSamplerEntry->pResOpBits) == gcvFALSE)
    {
        return VSC_ERR_OUT_OF_MEMORY;
    }

    return VSC_ERR_NONE;
}

static PROG_VK_SEPARATED_TEXTURE_TABLE_ENTRY* _enlargeVkSeparatedTexEntryRoom(PROG_VK_SEPARATED_TEXTURE_TABLE* pSeparatedTexTable,
                                                                              gctUINT enlargeCount,
                                                                              gctUINT* pStartEntryIdx)
{
    void*                         pOldTextureEntry;
    gctUINT                       oldTextureEntryCount, newTextureEntryCount;

    pOldTextureEntry = pSeparatedTexTable->pTextureEntries;
    oldTextureEntryCount = pSeparatedTexTable->countOfEntries;
    newTextureEntryCount = oldTextureEntryCount + enlargeCount;
    if (gcoOS_Allocate(gcvNULL,
                   sizeof(PROG_VK_SEPARATED_TEXTURE_TABLE_ENTRY) * newTextureEntryCount,
                   (gctPOINTER*)&pSeparatedTexTable->pTextureEntries) != gcvSTATUS_OK)
    {
        return gcvNULL;
    }

    if (pOldTextureEntry)
    {
        memcpy(pSeparatedTexTable->pTextureEntries, pOldTextureEntry,
               oldTextureEntryCount*sizeof(PROG_VK_SEPARATED_TEXTURE_TABLE_ENTRY));

        gcoOS_Free(gcvNULL, pOldTextureEntry);
    }

    pSeparatedTexTable->countOfEntries = newTextureEntryCount;

    if (pStartEntryIdx)
    {
        *pStartEntryIdx = oldTextureEntryCount;
    }

    /* Return first enlarged texture entry */
    return &pSeparatedTexTable->pTextureEntries[oldTextureEntryCount];
}

static VSC_ErrCode _AddVkSeparatedTexEntryToSeparatedTexTableOfPEP(VSC_PEP_GEN_HELPER* pPepGenHelper,
                                                                   PROG_VK_RESOURCE_SET* pResSet,
                                                                   PROG_VK_SEPARATED_TEXTURE_TABLE* pSeparatedTexTable,
                                                                   VIR_SHADER_RESOURCE_ALLOC_ENTRY* pResAllocEntry,
                                                                   VIR_Shader* pShader,
                                                                   gctUINT stageIdx,
                                                                   SHADER_EXECUTABLE_PROFILE* pSep)
{
    PROG_VK_SEPARATED_TEXTURE_TABLE_ENTRY* pTextureEntry = gcvNULL;
    gctUINT                                    i, textureEntryIndex;

    for (i = 0; i < pSeparatedTexTable->countOfEntries; i ++)
    {
        if (_CheckTwoResBindingsAreSame(&pSeparatedTexTable->pTextureEntries[i].texBinding, &pResAllocEntry->resBinding))
        {
            pTextureEntry = &pSeparatedTexTable->pTextureEntries[i];
            break;
        }
    }

    if (pTextureEntry == gcvNULL)
    {
        pTextureEntry = _enlargeVkSeparatedTexEntryRoom(pSeparatedTexTable, 1, &textureEntryIndex);
        if (pTextureEntry == gcvNULL)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
        memset(pTextureEntry, 0, sizeof(PROG_VK_SEPARATED_TEXTURE_TABLE_ENTRY));
        pTextureEntry->textureEntryIndex = textureEntryIndex;
        memcpy(&pTextureEntry->texBinding, &pResAllocEntry->resBinding, sizeof(VSC_SHADER_RESOURCE_BINDING));
    }

    if (pPepGenHelper->baseEpGen.pHwCfg->hwFeatureFlags.supportSeparatedTex)
    {
        gcmASSERT(gcvFALSE);

        if (pResAllocEntry->hwRegNo != NOT_ASSIGNED)
        {
            pTextureEntry->activeStageMask |= pResAllocEntry->bUse ? (1 << stageIdx) : 0;
            pTextureEntry->stageBits |= VSC_SHADER_STAGE_2_STAGE_BIT(stageIdx);
            pTextureEntry->hwMappings[stageIdx].texMapping.hwResourceSlot = pResAllocEntry->hwRegNo;
        }
        else
        {
            pTextureEntry->hwMappings[stageIdx].texMapping.hwResourceSlot = NOT_ASSIGNED;
        }

        pTextureEntry->bUsingHwMppingList = gcvFALSE;
    }
    /* When HW can't natively support this, we have allocated a image uniform for it. */
    else
    {
        SHADER_CONSTANT_HW_LOCATION_MAPPING*  pHwDirectAddrBase;
        gctUINT                               channel, hwChannel;

        gcmASSERT(pResAllocEntry->hwRegNo != NOT_ASSIGNED);

        pTextureEntry->activeStageMask |= pResAllocEntry->bUse ? (1 << stageIdx) : 0;
        pTextureEntry->stageBits |= VSC_SHADER_STAGE_2_STAGE_BIT(stageIdx);
        if (pResAllocEntry->resFlag & VIR_SRE_FLAG_TREAT_IA_AS_SAMPLER)
        {
            pTextureEntry->hwMappings[stageIdx].s.hwMapping.hwMemAccessMode = SHADER_HW_MEM_ACCESS_MODE_DIRECT_SAMPLER;
        }
        else
        {
            pTextureEntry->hwMappings[stageIdx].s.hwMapping.hwMemAccessMode = SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR;
        }
        pTextureEntry->hwMappings[stageIdx].s.hwMapping.accessMode = SHADER_UAV_ACCESS_MODE_TYPE;

        /* Alloc direct mem addr constant reg location mapping */
        if (gcoOS_Allocate(gcvNULL, sizeof(SHADER_CONSTANT_HW_LOCATION_MAPPING),
                           (gctPOINTER*)&pTextureEntry->hwMappings[stageIdx].s.hwMapping.hwLoc.pHwDirectAddrBase) != gcvSTATUS_OK)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
        pHwDirectAddrBase = pTextureEntry->hwMappings[stageIdx].s.hwMapping.hwLoc.pHwDirectAddrBase;
        vscInitializeCnstHwLocMapping(pHwDirectAddrBase);

        /* Fill direct mem base addr constant reg */
        pHwDirectAddrBase->hwAccessMode = SHADER_HW_ACCESS_MODE_REGISTER;
        pHwDirectAddrBase->hwLoc.constReg.hwRegRange = pResAllocEntry->hwRegRange;
        gcmASSERT(pHwDirectAddrBase->hwLoc.constReg.hwRegRange);

        if (pResAllocEntry->resFlag & VIR_SRE_FLAG_TREAT_IA_AS_SAMPLER)
        {
            pTextureEntry->hwMappings[stageIdx].s.hwMapping.hwSamplerSlot = pResAllocEntry->hwRegNo;
        }
        else
        {
            pHwDirectAddrBase->hwLoc.constReg.hwRegNo = pResAllocEntry->hwRegNo;

            for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
            {
                hwChannel = (((pResAllocEntry->swizzle) >> ((channel) * 2)) & 0x3);
                _SetValidChannelForHwConstantLoc(pHwDirectAddrBase, hwChannel);
            }
        }

        /* Fill the image derived information. */
        _FillImageDerivedInfo(pShader,
                              &pTextureEntry->texBinding,
                              pSep,
                              &pTextureEntry->hwMappings[stageIdx].s.imageDerivedInfo);
    }

    if (_SetResOpBits(pShader, &pTextureEntry->texBinding, &pTextureEntry->pResOpBits) == gcvFALSE)
    {
        return VSC_ERR_OUT_OF_MEMORY;
    }

    return VSC_ERR_NONE;
}

static PROG_VK_UNIFORM_TEXEL_BUFFER_TABLE_ENTRY* _enlargeVkUniformTexBufferEntryRoom(PROG_VK_UNIFORM_TEXEL_BUFFER_TABLE* pUtbTable,
                                                                                     gctUINT enlargeCount,
                                                                                     gctUINT* pStartEntryIdx)
{
    void*                         pOldUtbEntry;
    gctUINT                       oldUtbEntryCount, newUtbEntryCount;

    pOldUtbEntry = pUtbTable->pUtbEntries;
    oldUtbEntryCount = pUtbTable->countOfEntries;
    newUtbEntryCount = oldUtbEntryCount + enlargeCount;
    if (gcoOS_Allocate(gcvNULL,
                   sizeof(PROG_VK_UNIFORM_TEXEL_BUFFER_TABLE_ENTRY) * newUtbEntryCount,
                   (gctPOINTER*)&pUtbTable->pUtbEntries) != gcvSTATUS_OK)
    {
        return gcvNULL;
    }

    if (pOldUtbEntry)
    {
        memcpy(pUtbTable->pUtbEntries, pOldUtbEntry,
               oldUtbEntryCount*sizeof(PROG_VK_UNIFORM_TEXEL_BUFFER_TABLE_ENTRY));

        gcoOS_Free(gcvNULL, pOldUtbEntry);
    }

    pUtbTable->countOfEntries = newUtbEntryCount;

    if (pStartEntryIdx)
    {
        *pStartEntryIdx = oldUtbEntryCount;
    }

    /* Return first enlarged uniform texel buffer entry */
    return &pUtbTable->pUtbEntries[oldUtbEntryCount];
}

static VSC_ErrCode _AddVkUtbEntryToUniformTexBufTableOfPEP(VSC_PEP_GEN_HELPER* pPepGenHelper,
                                                           PROG_VK_RESOURCE_SET* pResSet,
                                                           PROG_VK_UNIFORM_TEXEL_BUFFER_TABLE* pUtbTable,
                                                           VIR_SHADER_RESOURCE_ALLOC_ENTRY* pResAllocEntry,
                                                           VIR_Shader* pShader,
                                                           gctUINT stageIdx,
                                                           SHADER_EXECUTABLE_PROFILE* pSep)
{
    PROG_VK_UNIFORM_TEXEL_BUFFER_TABLE_ENTRY* pUtbEntry = gcvNULL;
    gctUINT                                   i, utbEntryIndex, channel, hwChannel;
    gctBOOL                                   bIsImage = gcvFALSE;
    SHADER_CONSTANT_HW_LOCATION_MAPPING*      pHwDirectAddrBase;

    for (i = 0; i < pUtbTable->countOfEntries; i ++)
    {
        if (_CheckTwoResBindingsAreSame(&pUtbTable->pUtbEntries[i].utbBinding, &pResAllocEntry->resBinding))
        {
            pUtbEntry = &pUtbTable->pUtbEntries[i];
            break;
        }
    }

    if (pUtbEntry == gcvNULL)
    {
        pUtbEntry = _enlargeVkUniformTexBufferEntryRoom(pUtbTable, 1, &utbEntryIndex);
        if (pUtbEntry == gcvNULL)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
        memset(pUtbEntry, 0, sizeof(PROG_VK_UNIFORM_TEXEL_BUFFER_TABLE_ENTRY));
        pUtbEntry->utbEntryIndex = utbEntryIndex;
        memcpy(&pUtbEntry->utbBinding, &pResAllocEntry->resBinding, sizeof(VSC_SHADER_RESOURCE_BINDING));
    }

    pUtbEntry->hwMappings[stageIdx].hwMappingMode = VK_UNIFORM_TEXEL_BUFFER_HW_MAPPING_MODE_NOT_NATIVELY_SUPPORT;
    if ((pResAllocEntry->resFlag & VIR_SRE_FLAG_TREAT_TEXELBUFFER_AS_IMAGE)
        ||
        (pResAllocEntry->resFlag & VIR_SRE_FLAG_TEXELBUFFER_AN_IMAGE_NATIVELY))
    {
        pUtbEntry->hwMappings[stageIdx].u.s.hwMemAccessMode = SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR;
        bIsImage = gcvTRUE;

        if (pResAllocEntry->resFlag & VIR_SRE_FLAG_TREAT_TEXELBUFFER_AS_IMAGE)
        {
            pUtbEntry->utbEntryFlag |= PROG_VK_UTB_ENTRY_FLAG_TREAT_TEXELBUFFER_AS_IMAGE;
        }
    }
    else
    {
        pUtbEntry->hwMappings[stageIdx].u.s.hwMemAccessMode = SHADER_HW_MEM_ACCESS_MODE_DIRECT_SAMPLER;
    }

    /* Allocate the resource. */
    if (bIsImage)
    {
        /* Alloc direct mem addr constant reg location mapping */
        if (gcoOS_Allocate(gcvNULL, sizeof(SHADER_CONSTANT_HW_LOCATION_MAPPING),
                           (gctPOINTER*)&pUtbEntry->hwMappings[stageIdx].u.s.hwLoc.pHwDirectAddrBase) != gcvSTATUS_OK)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }

        pHwDirectAddrBase = pUtbEntry->hwMappings[stageIdx].u.s.hwLoc.pHwDirectAddrBase;
        vscInitializeCnstHwLocMapping(pHwDirectAddrBase);

        /* Fill direct mem base addr constant reg */
        pHwDirectAddrBase->hwAccessMode = SHADER_HW_ACCESS_MODE_REGISTER;

        pHwDirectAddrBase->hwLoc.constReg.hwRegNo = pResAllocEntry->hwRegNo;
        pHwDirectAddrBase->hwLoc.constReg.hwRegRange = pResAllocEntry->hwRegRange;

        for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
        {
            hwChannel = (((pResAllocEntry->swizzle) >> ((channel) * 2)) & 0x3);
            _SetValidChannelForHwConstantLoc(pHwDirectAddrBase, hwChannel);
        }

        _GetImageFormat(pShader, &pUtbEntry->utbBinding, &pUtbEntry->imageDerivedInfo[stageIdx].imageFormatInfo);
    }
    else
    {
        pUtbEntry->hwMappings[stageIdx].u.s.hwLoc.samplerMapping.hwSamplerSlot = pResAllocEntry->hwRegNo;
    }

    /* Set texture size */
    if (!(pResAllocEntry->resFlag & VIR_SRE_FLAG_TEXELBUFFER_AN_IMAGE_NATIVELY))
    {
        /* Fill the sampler derived information. */
        _FillSamplerDerivedInfo(pShader,
                                &pUtbEntry->utbBinding,
                                pSep,
                                &pUtbEntry->samplerDerivedInfo[stageIdx],
                                gcvFALSE,
                                gcvFALSE);
    }
    else
    {
        /* Fill the image derived information. */
        _FillImageDerivedInfo(pShader,
                              &pUtbEntry->utbBinding,
                              pSep,
                              &pUtbEntry->imageDerivedInfo[stageIdx]);
    }

    pUtbEntry->activeStageMask |= pResAllocEntry->bUse ? (1 << stageIdx) : 0;
    pUtbEntry->stageBits |= VSC_SHADER_STAGE_2_STAGE_BIT(stageIdx);

    if (_SetResOpBits(pShader, &pUtbEntry->utbBinding, &pUtbEntry->pResOpBits) == gcvFALSE)
    {
        return VSC_ERR_OUT_OF_MEMORY;
    }

    return VSC_ERR_NONE;
}

static PROG_VK_INPUT_ATTACHMENT_TABLE_ENTRY* _enlargeInputAttachmentEntryRoom(PROG_VK_INPUT_ATTACHMENT_TABLE* pIaTable,
                                                                              gctUINT enlargeCount,
                                                                              gctUINT* pStartEntryIdx)
{
    void*                         pOldIaEntry;
    gctUINT                       oldIaEntryCount, newIaEntryCount;

    pOldIaEntry = pIaTable->pIaEntries;
    oldIaEntryCount = pIaTable->countOfEntries;
    newIaEntryCount = oldIaEntryCount + enlargeCount;
    if (gcoOS_Allocate(gcvNULL,
                   sizeof(PROG_VK_INPUT_ATTACHMENT_TABLE_ENTRY) * newIaEntryCount,
                   (gctPOINTER*)&pIaTable->pIaEntries) != gcvSTATUS_OK)
    {
        return gcvNULL;
    }

    if (pOldIaEntry)
    {
        memcpy(pIaTable->pIaEntries, pOldIaEntry,
               oldIaEntryCount*sizeof(PROG_VK_INPUT_ATTACHMENT_TABLE_ENTRY));

        gcoOS_Free(gcvNULL, pOldIaEntry);
    }

    pIaTable->countOfEntries = newIaEntryCount;

    if (pStartEntryIdx)
    {
        *pStartEntryIdx = oldIaEntryCount;
    }

    /* Return first enlarged input attachment entry */
    return &pIaTable->pIaEntries[oldIaEntryCount];
}

static VSC_ErrCode _AddVkInputAttachmentTableOfPEP(VSC_PEP_GEN_HELPER* pPepGenHelper,
                                                   PROG_VK_RESOURCE_SET* pResSet,
                                                   PROG_VK_INPUT_ATTACHMENT_TABLE* pIaTable,
                                                   VIR_SHADER_RESOURCE_ALLOC_ENTRY* pResAllocEntry,
                                                   VIR_Shader* pShader,
                                                   gctUINT stageIdx,
                                                   SHADER_EXECUTABLE_PROFILE* pSep)
{
    PROG_VK_INPUT_ATTACHMENT_TABLE_ENTRY* pIaEntry = gcvNULL;
    gctUINT                               i, iaEntryIndex, channel, hwChannel;
    SHADER_CONSTANT_HW_LOCATION_MAPPING*  pHwDirectAddrBase;
    gctBOOL                               bIsSampler = gcvFALSE;

    for (i = 0; i < pIaTable->countOfEntries; i ++)
    {
        if (_CheckTwoResBindingsAreSame(&pIaTable->pIaEntries[i].iaBinding, &pResAllocEntry->resBinding))
        {
            pIaEntry = &pIaTable->pIaEntries[i];
            break;
        }
    }

    if (pIaEntry == gcvNULL)
    {
        pIaEntry = _enlargeInputAttachmentEntryRoom(pIaTable, 1, &iaEntryIndex);
        if (pIaEntry == gcvNULL)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
        memset(pIaEntry, 0, sizeof(PROG_VK_INPUT_ATTACHMENT_TABLE_ENTRY));
        pIaEntry->iaEntryIndex = iaEntryIndex;
        memcpy(&pIaEntry->iaBinding, &pResAllocEntry->resBinding, sizeof(VSC_SHADER_RESOURCE_BINDING));
    }
    pIaEntry->activeStageMask |= pResAllocEntry->bUse ? (1 << stageIdx) : 0;
    pIaEntry->stageBits |= VSC_SHADER_STAGE_2_STAGE_BIT(stageIdx);
    if (pResAllocEntry->resFlag & VIR_SRE_FLAG_TREAT_IA_AS_SAMPLER)
    {
        pIaEntry->hwMappings[stageIdx].uavMapping.hwMemAccessMode = SHADER_HW_MEM_ACCESS_MODE_DIRECT_SAMPLER;
        bIsSampler = gcvTRUE;
    }
    else
    {
        pIaEntry->hwMappings[stageIdx].uavMapping.hwMemAccessMode = SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR;
    }
    pIaEntry->hwMappings[stageIdx].uavMapping.accessMode = SHADER_UAV_ACCESS_MODE_TYPE;

    /* Alloc direct mem addr constant reg location mapping */
    if (gcoOS_Allocate(gcvNULL, sizeof(SHADER_CONSTANT_HW_LOCATION_MAPPING),
                       (gctPOINTER*)&pIaEntry->hwMappings[stageIdx].uavMapping.hwLoc.pHwDirectAddrBase) != gcvSTATUS_OK)
    {
        return VSC_ERR_OUT_OF_MEMORY;
    }
    pHwDirectAddrBase = pIaEntry->hwMappings[stageIdx].uavMapping.hwLoc.pHwDirectAddrBase;
    vscInitializeCnstHwLocMapping(pHwDirectAddrBase);

    /* Fill direct mem base addr constant reg */
    pHwDirectAddrBase->hwAccessMode = SHADER_HW_ACCESS_MODE_REGISTER;
    pHwDirectAddrBase->hwLoc.constReg.hwRegRange = pResAllocEntry->hwRegRange;
    gcmASSERT(pHwDirectAddrBase->hwLoc.constReg.hwRegRange);

    if (bIsSampler)
    {
        pIaEntry->hwMappings[stageIdx].uavMapping.hwSamplerSlot = pResAllocEntry->hwRegNo;
    }
    else
    {
        pHwDirectAddrBase->hwLoc.constReg.hwRegNo = pResAllocEntry->hwRegNo;

        for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
        {
            hwChannel = (((pResAllocEntry->swizzle) >> ((channel) * 2)) & 0x3);
            _SetValidChannelForHwConstantLoc(pHwDirectAddrBase, hwChannel);
        }
    }

    if (bIsSampler)
    {
        _AddExtraSamplerArray(&pIaEntry->hwMappings[stageIdx].ppExtraSamplerArray,
                              &pIaEntry->iaBinding,
                              pShader,
                              pSep,
                              gcvFALSE,
                              gcvFALSE,
                              -1,
                              1,
                              0);

        /* Fill the sampler derived information. */
        _FillSamplerDerivedInfo(pShader,
                                &pIaEntry->iaBinding,
                                pSep,
                                &pIaEntry->samplerDerivedInfo[stageIdx],
                                gcvFALSE,
                                gcvFALSE);
    }
    else
    {
        /* Fill the image derived information. */
        _FillImageDerivedInfo(pShader,
                              &pIaEntry->iaBinding,
                              pSep,
                              &pIaEntry->imageDerivedInfo[stageIdx]);
    }

    if (_SetResOpBits(pShader, &pIaEntry->iaBinding, &pIaEntry->pResOpBits) == gcvFALSE)
    {
        return VSC_ERR_OUT_OF_MEMORY;
    }

    return VSC_ERR_NONE;
}

static PROG_VK_STORAGE_TABLE_ENTRY* _enlargeVkStorageEntryRoom(PROG_VK_STORAGE_TABLE* pStorageTable,
                                                               gctUINT enlargeCount,
                                                               gctUINT* pStartEntryIdx)
{
    void*                         pOldStorageEntry;
    gctUINT                       oldStorageEntryCount, newStorageEntryCount;

    pOldStorageEntry = pStorageTable->pStorageEntries;
    oldStorageEntryCount = pStorageTable->countOfEntries;
    newStorageEntryCount = oldStorageEntryCount + enlargeCount;
    if (gcoOS_Allocate(gcvNULL,
                   sizeof(PROG_VK_STORAGE_TABLE_ENTRY) * newStorageEntryCount,
                   (gctPOINTER*)&pStorageTable->pStorageEntries) != gcvSTATUS_OK)
    {
        return gcvNULL;
    }

    if (pOldStorageEntry)
    {
        memcpy(pStorageTable->pStorageEntries, pOldStorageEntry,
               oldStorageEntryCount*sizeof(PROG_VK_STORAGE_TABLE_ENTRY));

        gcoOS_Free(gcvNULL, pOldStorageEntry);
    }

    pStorageTable->countOfEntries = newStorageEntryCount;

    if (pStartEntryIdx)
    {
        *pStartEntryIdx = oldStorageEntryCount;
    }

    /* Return first enlarged storage entry */
    return &pStorageTable->pStorageEntries[oldStorageEntryCount];
}

static VSC_ErrCode _AddVkStorageEntryToStorageTableOfPEP(VSC_PEP_GEN_HELPER* pPepGenHelper,
                                                         PROG_VK_RESOURCE_SET* pResSet,
                                                         PROG_VK_STORAGE_TABLE* pStorageTable,
                                                         VIR_SHADER_RESOURCE_ALLOC_ENTRY* pResAllocEntry,
                                                         VIR_Shader* pShader,
                                                         gctUINT stageIdx,
                                                         SHADER_EXECUTABLE_PROFILE* pSep)
{
    PROG_VK_STORAGE_TABLE_ENTRY*         pStorageEntry = gcvNULL;
    gctUINT                              i, StorageEntryIndex, channel, hwChannel;
    SHADER_CONSTANT_HW_LOCATION_MAPPING* pHwDirectAddrBase;

    for (i = 0; i < pStorageTable->countOfEntries; i ++)
    {
        if (_CheckTwoResBindingsAreSame(&pStorageTable->pStorageEntries[i].storageBinding, &pResAllocEntry->resBinding))
        {
            pStorageEntry = &pStorageTable->pStorageEntries[i];
            break;
        }
    }

    if (pStorageEntry == gcvNULL)
    {
        pStorageEntry = _enlargeVkStorageEntryRoom(pStorageTable, 1, &StorageEntryIndex);
        if (pStorageEntry == gcvNULL)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
        memset(pStorageEntry, 0, sizeof(PROG_VK_STORAGE_TABLE_ENTRY));
        pStorageEntry->storageEntryIndex = StorageEntryIndex;
        memcpy(&pStorageEntry->storageBinding, &pResAllocEntry->resBinding, sizeof(VSC_SHADER_RESOURCE_BINDING));
    }
    pStorageEntry->activeStageMask |= pResAllocEntry->bUse ? (1 << stageIdx) : 0;
    pStorageEntry->stageBits |= VSC_SHADER_STAGE_2_STAGE_BIT(stageIdx);
    pStorageEntry->hwMappings[stageIdx].hwMemAccessMode = SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR;

    if (pResAllocEntry->resBinding.type == VSC_SHADER_RESOURCE_TYPE_STORAGE_BUFFER ||
        pResAllocEntry->resBinding.type == VSC_SHADER_RESOURCE_TYPE_STORAGE_BUFFER_DYNAMIC)
    {
        if (pResAllocEntry->lastElementSize != 0)
        {
            pStorageEntry->hwMappings[stageIdx].accessMode = SHADER_UAV_ACCESS_MODE_RESIZABLE;
            pStorageEntry->hwMappings[stageIdx].sizeInByte = pResAllocEntry->fixedSize;
            pStorageEntry->hwMappings[stageIdx].u.sizableEleSize = pResAllocEntry->lastElementSize;
        }
        else
        {
            pStorageEntry->hwMappings[stageIdx].accessMode = SHADER_UAV_ACCESS_MODE_STRUCTURED;
        }
    }
    else if (pResAllocEntry->resBinding.type == VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE ||
             pResAllocEntry->resBinding.type == VSC_SHADER_RESOURCE_TYPE_STORAGE_TEXEL_BUFFER)
    {
        pStorageEntry->hwMappings[stageIdx].accessMode = SHADER_UAV_ACCESS_MODE_TYPE;
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    /* Alloc direct mem addr constant reg location mapping */
    if (gcoOS_Allocate(gcvNULL, sizeof(SHADER_CONSTANT_HW_LOCATION_MAPPING),
                       (gctPOINTER*)&pStorageEntry->hwMappings[stageIdx].hwLoc.pHwDirectAddrBase) != gcvSTATUS_OK)
    {
        return VSC_ERR_OUT_OF_MEMORY;
    }
    pHwDirectAddrBase = pStorageEntry->hwMappings[stageIdx].hwLoc.pHwDirectAddrBase;
    vscInitializeCnstHwLocMapping(pHwDirectAddrBase);

    /* Fill direct mem base addr constant reg */
    pHwDirectAddrBase->hwAccessMode = SHADER_HW_ACCESS_MODE_REGISTER;
    pHwDirectAddrBase->hwLoc.constReg.hwRegNo = pResAllocEntry->hwRegNo;
    pHwDirectAddrBase->hwLoc.constReg.hwRegRange = pResAllocEntry->hwRegRange;
    gcmASSERT(pHwDirectAddrBase->hwLoc.constReg.hwRegRange);
    for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
    {
        hwChannel = (((pResAllocEntry->swizzle) >> ((channel) * 2)) & 0x3);
        _SetValidChannelForHwConstantLoc(pHwDirectAddrBase, hwChannel);
    }

    /* Fill the image derived information. */
    _FillImageDerivedInfo(pShader,
                          &pStorageEntry->storageBinding,
                          pSep,
                          &pStorageEntry->imageDerivedInfo[stageIdx]);

    if (_SetResOpBits(pShader, &pStorageEntry->storageBinding, &pStorageEntry->pResOpBits) == gcvFALSE)
    {
        return VSC_ERR_OUT_OF_MEMORY;
    }

    return VSC_ERR_NONE;
}

static PROG_VK_UNIFORM_BUFFER_TABLE_ENTRY* _enlargeVkUbEntryRoom(PROG_VK_UNIFORM_BUFFER_TABLE* pUbTable,
                                                                 gctUINT enlargeCount,
                                                                 gctUINT* pStartEntryIdx)
{
    void*                         pOldUbEntry;
    gctUINT                       oldUbEntryCount, newUbEntryCount;

    pOldUbEntry = pUbTable->pUniformBufferEntries;
    oldUbEntryCount = pUbTable->countOfEntries;
    newUbEntryCount = oldUbEntryCount + enlargeCount;
    if (gcoOS_Allocate(gcvNULL,
                   sizeof(PROG_VK_UNIFORM_BUFFER_TABLE_ENTRY) * newUbEntryCount,
                   (gctPOINTER*)&pUbTable->pUniformBufferEntries) != gcvSTATUS_OK)
    {
        return gcvNULL;
    }

    if (pOldUbEntry)
    {
        memcpy(pUbTable->pUniformBufferEntries, pOldUbEntry,
               oldUbEntryCount*sizeof(PROG_VK_UNIFORM_BUFFER_TABLE_ENTRY));

        gcoOS_Free(gcvNULL, pOldUbEntry);
    }

    pUbTable->countOfEntries = newUbEntryCount;

    if (pStartEntryIdx)
    {
        *pStartEntryIdx = oldUbEntryCount;
    }

    /* Return first enlarged ub entry */
    return &pUbTable->pUniformBufferEntries[oldUbEntryCount];
}

static VSC_ErrCode _AddVkUbEntryToUbTableOfPEP(VSC_PEP_GEN_HELPER* pPepGenHelper,
                                               PROG_VK_RESOURCE_SET* pResSet,
                                               PROG_VK_UNIFORM_BUFFER_TABLE* pUbTable,
                                               VIR_SHADER_RESOURCE_ALLOC_ENTRY* pResAllocEntry,
                                               VIR_Shader* pShader,
                                               gctUINT stageIdx,
                                               SHADER_EXECUTABLE_PROFILE* pSep)
{
    PROG_VK_UNIFORM_BUFFER_TABLE_ENTRY*  pUbEntry = gcvNULL;
    gctUINT                              i, ubEntryIndex, channel, hwChannel;
    SHADER_CONSTANT_HW_LOCATION_MAPPING* pHwDirectAddrBase;
    VIR_SHADER_RESOURCE_ALLOC_ENTRY*     pConstReg = pResAllocEntry->pConstRegForUbo;

    for (i = 0; i < pUbTable->countOfEntries; i ++)
    {
        if (_CheckTwoResBindingsAreSame(&pUbTable->pUniformBufferEntries[i].ubBinding, &pResAllocEntry->resBinding))
        {
            pUbEntry = &pUbTable->pUniformBufferEntries[i];
            break;
        }
    }

    if (pUbEntry == gcvNULL)
    {
        pUbEntry = _enlargeVkUbEntryRoom(pUbTable, 1, &ubEntryIndex);
        if (pUbEntry == gcvNULL)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
        memset(pUbEntry, 0, sizeof(PROG_VK_UNIFORM_BUFFER_TABLE_ENTRY));
        pUbEntry->ubEntryIndex = ubEntryIndex;
        memcpy(&pUbEntry->ubBinding, &pResAllocEntry->resBinding, sizeof(VSC_SHADER_RESOURCE_BINDING));
    }
    pUbEntry->activeStageMask |= pResAllocEntry->bUse ? (1 << stageIdx) : 0;
    pUbEntry->stageBits |= VSC_SHADER_STAGE_2_STAGE_BIT(stageIdx);
    if (pConstReg != gcvNULL)
    {
        pUbEntry->hwMappings[stageIdx].hwAccessMode = SHADER_HW_ACCESS_MODE_BOTH_REG_AND_MEM;
    }
    else
    {
        pUbEntry->hwMappings[stageIdx].hwAccessMode = SHADER_HW_ACCESS_MODE_MEMORY;
    }

    /* I: Program the direct memory address. */
    pUbEntry->hwMappings[stageIdx].hwLoc.memAddr.hwMemAccessMode = SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR;
    pUbEntry->hwMappings[stageIdx].validHWChannelMask = WRITEMASK_ALL;

    /* Alloc direct mem addr constant reg location mapping */
    if (gcoOS_Allocate(gcvNULL, sizeof(SHADER_CONSTANT_HW_LOCATION_MAPPING),
                       (gctPOINTER*)&pUbEntry->hwMappings[stageIdx].hwLoc.memAddr.memBase.pHwDirectAddrBase) != gcvSTATUS_OK)
    {
        return VSC_ERR_OUT_OF_MEMORY;
    }
    pHwDirectAddrBase = pUbEntry->hwMappings[stageIdx].hwLoc.memAddr.memBase.pHwDirectAddrBase;
    vscInitializeCnstHwLocMapping(pHwDirectAddrBase);

    /* Fill direct mem base addr constant reg */
    pHwDirectAddrBase->hwAccessMode = SHADER_HW_ACCESS_MODE_REGISTER;
    pHwDirectAddrBase->hwLoc.constReg.hwRegNo = pResAllocEntry->hwRegNo;
    pHwDirectAddrBase->hwLoc.constReg.hwRegRange = pResAllocEntry->hwRegRange;
    gcmASSERT(pHwDirectAddrBase->hwLoc.constReg.hwRegRange);
    for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
    {
        hwChannel = (((pResAllocEntry->swizzle) >> ((channel) * 2)) & 0x3);
        _SetValidChannelForHwConstantLoc(pHwDirectAddrBase, hwChannel);
    }

    /* II: Program the constant register if needed. */
    if (pConstReg != gcvNULL)
    {
        pUbEntry->hwMappings[stageIdx].hwLoc.constReg.hwRegNo = pConstReg->hwRegNo;
        pUbEntry->hwMappings[stageIdx].hwLoc.constReg.hwRegRange = pConstReg->hwRegRange;

        for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
        {
            hwChannel = (((pConstReg->swizzle) >> ((channel) * 2)) & 0x3);
            _SetValidChannelForHwConstantLoc(&pUbEntry->hwMappings[stageIdx], hwChannel);
        }
    }

    return VSC_ERR_NONE;
}

static VSC_ErrCode _AddVkCombStEntryToCombStTableOfPEP(VSC_PEP_GEN_HELPER* pPepGenHelper,
                                                       PROG_VK_RESOURCE_SET* pResSet,
                                                       PROG_VK_COMBINED_TEXTURE_SAMPLER_TABLE* pCombinedSampTexTable,
                                                       VIR_SHADER_RESOURCE_ALLOC_ENTRY* pResAllocEntry,
                                                       VIR_Shader* pShader,
                                                       gctUINT stageIdx,
                                                       SHADER_EXECUTABLE_PROFILE* pSep)
{
    PROG_VK_COMBINED_TEX_SAMPLER_TABLE_ENTRY* pCombTsEntry = gcvNULL;
    VIR_SHADER_RESOURCE_ALLOC_ENTRY*          pSampledImage = pResAllocEntry->pCombinedSampledImage;
    gctUINT                                   i, j, combTsEntryIndex;

    for (i = 0; i < pCombinedSampTexTable->countOfEntries; i ++)
    {
        if (_CheckTwoResBindingsAreSame(&pCombinedSampTexTable->pCombTsEntries[i].combTsBinding, &pResAllocEntry->resBinding))
        {
            pCombTsEntry = &pCombinedSampTexTable->pCombTsEntries[i];
            break;
        }
    }

    if (pCombTsEntry == gcvNULL)
    {
        pCombTsEntry = _enlargeVkCombTsEntryRoom(pCombinedSampTexTable, 1, &combTsEntryIndex);
        if (pCombTsEntry == gcvNULL)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
        memset(pCombTsEntry, 0, sizeof(PROG_VK_COMBINED_TEX_SAMPLER_TABLE_ENTRY));
        pCombTsEntry->combTsEntryIndex = combTsEntryIndex;
        memcpy(&pCombTsEntry->combTsBinding, &pResAllocEntry->resBinding, sizeof(VSC_SHADER_RESOURCE_BINDING));
    }

    if (pResAllocEntry->hwRegNo != NOT_ASSIGNED)
    {
        pCombTsEntry->activeStageMask |= pResAllocEntry->bUse ? (1 << stageIdx) : 0;
        pCombTsEntry->stageBits |= VSC_SHADER_STAGE_2_STAGE_BIT(stageIdx);
        pCombTsEntry->hwMappings[stageIdx].samplerMapping.hwSamplerSlot = pResAllocEntry->hwRegNo;
    }
    else
    {
        pCombTsEntry->hwMappings[stageIdx].samplerMapping.hwSamplerSlot = NOT_ASSIGNED;
    }

    if (_SetResOpBits(pShader, &pCombTsEntry->combTsBinding, &pCombTsEntry->pResOpBits) == gcvFALSE)
    {
        return VSC_ERR_OUT_OF_MEMORY;
    }

    /* Extra samplers for VSC_LIB_LINK_TYPE_RESOURCE */
    if (pResAllocEntry->hwRegNo != NOT_ASSIGNED)
    {
        gctUINT planes;

        _AddExtraSamplerArray(&pCombTsEntry->hwMappings[stageIdx].ppExtraSamplerArray,
                              &pCombTsEntry->combTsBinding,
                              pShader,
                              pSep,
                              gcvFALSE,
                              gcvTRUE,
                              -1,
                              1,
                              0);

        planes = __YCBCR_PLANE_COUNT__ * pCombTsEntry->combTsBinding.arraySize;
        if (pCombTsEntry->hwMappings[stageIdx].ppYcbcrPlanes == gcvNULL)
        {
            gctPOINTER ptr;

            if (gcoOS_Allocate(gcvNULL, sizeof(SHADER_PRIV_UAV_ENTRY*) * planes,
                (gctPOINTER *) &ptr) != gcvSTATUS_OK)
            {
                return VSC_ERR_OUT_OF_MEMORY;
            }

            pCombTsEntry->hwMappings[stageIdx].ppYcbcrPlanes = ptr;
            memset(ptr, 0, sizeof(SHADER_PRIV_UAV_ENTRY*) * planes);
        }
        for (j = 0; j < planes; j++)
        {
            _AddPrivateImageUniform(pCombTsEntry->hwMappings[stageIdx].ppYcbcrPlanes + j,
                                    &pCombTsEntry->combTsBinding,
                                    pSep,
                                    SHS_PRIV_MEM_KIND_YCBCR_PLANE,
                                    gcvTRUE,
                                    j);
        }
    }

    /* Fill the sampler derived information. */
    _FillSamplerDerivedInfo(pShader,
                            &pCombTsEntry->combTsBinding,
                            pSep,
                            &pCombTsEntry->samplerDerivedInfo[stageIdx],
                            gcvFALSE,
                            gcvFALSE);

    /*
    ** pSampledImage is not empty means that this combined image sampler is accessed via separate sampler and sampled image shader variables,
    ** so we add this image entry to the storage table.
    */
    if (pSampledImage != gcvNULL)
    {
        PROG_VK_SEPARATED_TEXTURE_TABLE_ENTRY*  pTextureEntry = gcvNULL;
        PROG_VK_SEPARATED_TEXTURE_TABLE*        pSeparatedTexTable = &pResSet->separatedTexTable;
        gctUINT                                 i;

        _AddVkSeparatedTexEntryToSeparatedTexTableOfPEP(pPepGenHelper,
                                                        pResSet,
                                                        pSeparatedTexTable,
                                                        pSampledImage,
                                                        pShader,
                                                        stageIdx,
                                                        pSep);

        for (i = 0; i < pSeparatedTexTable->countOfEntries; i ++)
        {
            if (_CheckTwoResBindingsAreSame(&pSeparatedTexTable->pTextureEntries[i].texBinding, &pSampledImage->resBinding))
            {
                pTextureEntry = &pSeparatedTexTable->pTextureEntries[i];
                break;
            }
        }

        if (pTextureEntry == gcvNULL)
        {
            gcmASSERT(gcvFALSE);
        }

        pCombTsEntry->sampledImageIndexInStorageTable[stageIdx] = i;
    }
    else
    {
        pCombTsEntry->sampledImageIndexInStorageTable[stageIdx] = NOT_ASSIGNED;
    }

    return VSC_ERR_NONE;
}

static gctBOOL _CheckTwoPushCnstEntriesAreSame(VSC_SHADER_PUSH_CONSTANT_RANGE* pPushCnstEntry1, VSC_SHADER_PUSH_CONSTANT_RANGE* pPushCnstEntry2)
{
    return (pPushCnstEntry1->offset == pPushCnstEntry2->offset && pPushCnstEntry1->size == pPushCnstEntry2->size);
}

static PROG_VK_PUSH_CONSTANT_TABLE_ENTRY* _enlargeVkPushCnstEntryRoom(PROG_VK_PUSH_CONSTANT_TABLE* pPushCnstTable,
                                                                      gctUINT enlargeCount,
                                                                      gctUINT* pStartEntryIdx)
{
    void*                         pOldPushCnstEntry;
    gctUINT                       oldPushCnstEntryCount, newPushCnstEntryCount;

    pOldPushCnstEntry = pPushCnstTable->pPushConstantEntries;
    oldPushCnstEntryCount = pPushCnstTable->countOfEntries;
    newPushCnstEntryCount = oldPushCnstEntryCount + enlargeCount;
    if (gcoOS_Allocate(gcvNULL,
                   sizeof(PROG_VK_PUSH_CONSTANT_TABLE_ENTRY) * newPushCnstEntryCount,
                   (gctPOINTER*)&pPushCnstTable->pPushConstantEntries) != gcvSTATUS_OK)
    {
        return gcvNULL;
    }

    if (pOldPushCnstEntry)
    {
        memcpy(pPushCnstTable->pPushConstantEntries, pOldPushCnstEntry,
               oldPushCnstEntryCount*sizeof(PROG_VK_PUSH_CONSTANT_TABLE_ENTRY));

        gcoOS_Free(gcvNULL, pOldPushCnstEntry);
    }

    pPushCnstTable->countOfEntries = newPushCnstEntryCount;

    if (pStartEntryIdx)
    {
        *pStartEntryIdx = oldPushCnstEntryCount;
    }

    /* Return first enlarged push-constant entry */
    return &pPushCnstTable->pPushConstantEntries[oldPushCnstEntryCount];
}

static VSC_ErrCode _AddVkPushCnstEntryToPushCnstTableOfPEP(VSC_PEP_GEN_HELPER* pPepGenHelper,
                                                           PROG_VK_PUSH_CONSTANT_TABLE* pPushCnstTable,
                                                           VIR_SHADER_PUSH_CONSTANT_ALLOC_ENTRY* pPushCnstAllocEntry,
                                                           gctUINT stageIdx)
{
    PROG_VK_PUSH_CONSTANT_TABLE_ENTRY*      pPushCnstEntry = gcvNULL;
    gctUINT                                 i, pushCnstEntryIndex, channel, hwChannel;
    SHADER_CONSTANT_HW_LOCATION_MAPPING*    pHwDirectAddrBase;

    for (i = 0; i < pPushCnstTable->countOfEntries; i ++)
    {
        if (_CheckTwoPushCnstEntriesAreSame(&pPushCnstTable->pPushConstantEntries[i].pushConstRange, &pPushCnstAllocEntry->pushCnstRange))
        {
            pPushCnstEntry = &pPushCnstTable->pPushConstantEntries[i];
            break;
        }
    }

    if (pPushCnstEntry == gcvNULL)
    {
        pPushCnstEntry = _enlargeVkPushCnstEntryRoom(pPushCnstTable, 1, &pushCnstEntryIndex);
        if (pPushCnstEntry == gcvNULL)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
        memset(pPushCnstEntry, 0, sizeof(PROG_VK_PUSH_CONSTANT_TABLE_ENTRY));
        memcpy(&pPushCnstEntry->pushConstRange, &pPushCnstAllocEntry->pushCnstRange, sizeof(VSC_SHADER_PUSH_CONSTANT_RANGE));
    }
    vscInitializeCnstHwLocMapping(&pPushCnstEntry->hwMappings[stageIdx]);

    pPushCnstEntry->activeStageMask |= pPushCnstAllocEntry->bUse ? (1 << stageIdx) : 0;
    pPushCnstEntry->stageBits |= VSC_SHADER_STAGE_2_STAGE_BIT(stageIdx);

    if (pPushCnstAllocEntry->bBaseAddr)
    {
        pPushCnstEntry->hwMappings[stageIdx].hwAccessMode = SHADER_HW_ACCESS_MODE_MEMORY;
        pPushCnstEntry->hwMappings[stageIdx].hwLoc.memAddr.hwMemAccessMode = SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR;
        pPushCnstEntry->hwMappings[stageIdx].validHWChannelMask = WRITEMASK_ALL;

        /* Alloc direct mem addr constant reg location mapping */
        if (gcoOS_Allocate(gcvNULL, sizeof(SHADER_CONSTANT_HW_LOCATION_MAPPING),
                           (gctPOINTER*)&pPushCnstEntry->hwMappings[stageIdx].hwLoc.memAddr.memBase.pHwDirectAddrBase) != gcvSTATUS_OK)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
        pHwDirectAddrBase = pPushCnstEntry->hwMappings[stageIdx].hwLoc.memAddr.memBase.pHwDirectAddrBase;
        vscInitializeCnstHwLocMapping(pHwDirectAddrBase);

        /* Fill direct mem base addr constant reg */
        pHwDirectAddrBase->hwAccessMode = SHADER_HW_ACCESS_MODE_REGISTER;
        pHwDirectAddrBase->hwLoc.constReg.hwRegNo = pPushCnstAllocEntry->hwRegNo;
        pHwDirectAddrBase->hwLoc.constReg.hwRegRange = 1;
        gcmASSERT(pHwDirectAddrBase->hwLoc.constReg.hwRegRange);
        for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
        {
            hwChannel = (((pPushCnstAllocEntry->swizzle) >> ((channel) * 2)) & 0x3);
            _SetValidChannelForHwConstantLoc(pHwDirectAddrBase, hwChannel);
        }
    }
    else
    {
        pPushCnstEntry->hwMappings[stageIdx].hwAccessMode = SHADER_HW_ACCESS_MODE_REGISTER;

        pPushCnstEntry->hwMappings[stageIdx].hwLoc.constReg.hwRegNo = pPushCnstAllocEntry->hwRegNo;
        pPushCnstEntry->hwMappings[stageIdx].hwLoc.constReg.hwRegRange = 1;
        gcmASSERT(pPushCnstEntry->hwMappings[stageIdx].hwLoc.constReg.hwRegRange);
        for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
        {
            hwChannel = (((pPushCnstAllocEntry->swizzle) >> ((channel) * 2)) & 0x3);
            _SetValidChannelForHwConstantLoc(&pPushCnstEntry->hwMappings[stageIdx], hwChannel);
        }
    }

    return VSC_ERR_NONE;
}

static PROG_VK_PRIV_COMB_TEX_SAMP_HW_MAPPING* _enlargeVkPctsHwMappingEntryRoom(PROG_VK_PRIV_COMB_TEX_SAMP_HW_MAPPING_POOL* pPctsHwMappingTable,
                                                                               gctUINT enlargeCount,
                                                                               gctUINT* pStartEntryIdx)
{
    void*                         pOldPctsHwMappingEntry;
    gctUINT                       oldPctsHwMappingEntryCount, newPctsHwMappingEntryCount;

    pOldPctsHwMappingEntry = pPctsHwMappingTable->pPrivCombTsHwMappingArray;
    oldPctsHwMappingEntryCount = pPctsHwMappingTable->countOfArray;
    newPctsHwMappingEntryCount = oldPctsHwMappingEntryCount + enlargeCount;
    if (gcoOS_Allocate(gcvNULL,
                   sizeof(PROG_VK_PRIV_COMB_TEX_SAMP_HW_MAPPING) * newPctsHwMappingEntryCount,
                   (gctPOINTER*)&pPctsHwMappingTable->pPrivCombTsHwMappingArray) != gcvSTATUS_OK)
    {
        return gcvNULL;
    }

    if (pOldPctsHwMappingEntry)
    {
        memcpy(pPctsHwMappingTable->pPrivCombTsHwMappingArray, pOldPctsHwMappingEntry,
               oldPctsHwMappingEntryCount*sizeof(PROG_VK_PRIV_COMB_TEX_SAMP_HW_MAPPING));

        gcoOS_Free(gcvNULL, pOldPctsHwMappingEntry);
    }

    pPctsHwMappingTable->countOfArray = newPctsHwMappingEntryCount;

    if (pStartEntryIdx)
    {
        *pStartEntryIdx = oldPctsHwMappingEntryCount;
    }

    /* Return first enlarged private-combined-texture-sampler entry */
    return &pPctsHwMappingTable->pPrivCombTsHwMappingArray[oldPctsHwMappingEntryCount];
}

static gctUINT* _enlargeVkPctsHwMappingEntryIdxListRoom(PROG_VK_PRIV_COMB_TEX_SAMP_HW_MAPPING_LIST* pPctsHwMappingList,
                                                        gctUINT enlargeCount,
                                                        gctUINT* pStartEntryIdx)
{
    void*                         pOldPctsHwMappingEntryIdx;
    gctUINT                       oldPctsHwMappingEntryIdxCount, newPctsHwMappingEntryIdxCount;

    pOldPctsHwMappingEntryIdx = pPctsHwMappingList->pPctsHmEntryIdxArray;
    oldPctsHwMappingEntryIdxCount = pPctsHwMappingList->arraySize;
    newPctsHwMappingEntryIdxCount = oldPctsHwMappingEntryIdxCount + enlargeCount;
    if (gcoOS_Allocate(gcvNULL,
                   sizeof(gctUINT) * newPctsHwMappingEntryIdxCount,
                   (gctPOINTER*)&pPctsHwMappingList->pPctsHmEntryIdxArray) != gcvSTATUS_OK)
    {
        return gcvNULL;
    }

    if (pOldPctsHwMappingEntryIdx)
    {
        memcpy(pPctsHwMappingList->pPctsHmEntryIdxArray, pOldPctsHwMappingEntryIdx,
               oldPctsHwMappingEntryIdxCount*sizeof(gctUINT));

        gcoOS_Free(gcvNULL, pOldPctsHwMappingEntryIdx);
    }

    pPctsHwMappingList->arraySize = newPctsHwMappingEntryIdxCount;

    if (pStartEntryIdx)
    {
        *pStartEntryIdx = oldPctsHwMappingEntryIdxCount;
    }

    /* Return first enlarged private-combined-texture-sampler-idx entry */
    return &pPctsHwMappingList->pPctsHmEntryIdxArray[oldPctsHwMappingEntryIdxCount];
}

static VSC_ErrCode _CollectCompilerGeneatedCombinedSampler(VSC_PEP_GEN_HELPER* pPepGenHelper,
                                                           gctUINT stageIdx,
                                                           PROGRAM_EXECUTABLE_PROFILE* pOutPEP)
{
    VSC_ErrCode                                 errCode = VSC_ERR_NONE;
    VIR_Shader*                                 pShader = pPepGenHelper->pShader;
    gctUINT                                     i, j, resArraySize, pctsHmEntryIndex = NOT_ASSIGNED;
    VIR_UniformIdList*                          pVirUniformLsts = VIR_Shader_GetUniforms(pShader);
    VIR_Symbol*                                 pVirUniformSym;
    VIR_Symbol*                                 pTexSym;
    VIR_Symbol*                                 pSamplerSym;
    PROG_VK_RESOURCE_SET*                       pResSet;
    PROG_VK_SEPARATED_SAMPLER_TABLE_ENTRY*      pSeparatedSamplerEntry;
    PROG_VK_SEPARATED_TEXTURE_TABLE_ENTRY*      pSeparatedTexEntry;
    VIR_Type*                                   pSymType;
    PROG_VK_PRIV_COMB_TEX_SAMP_HW_MAPPING*      pPctsHwMapping = gcvNULL;
    gctUINT*                                    pPctsHwMappingListIdx = gcvNULL;
    gctINT16                                    texStartIdx, texSubRange, samplerStartIdx, samplerSubRange;

    for (i = 0; i < VIR_IdList_Count(pVirUniformLsts); ++ i)
    {
        pVirUniformSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pVirUniformLsts, i));

        if (isSymCompilerGen(pVirUniformSym) &&
            VIR_Symbol_isSampler(pVirUniformSym) &&
            VIR_Symbol_GetUniformKind(pVirUniformSym) == VIR_UNIFORM_SAMPLED_IMAGE)
        {
            pSamplerSym = VIR_Symbol_GetSeparateSampler(pShader, pVirUniformSym);
            pTexSym = VIR_Symbol_GetSeparateImage(pShader, pVirUniformSym);

            texStartIdx = samplerStartIdx = 0;
            texSubRange = (gctINT16)VIR_Symbol_GetImgIdxRange(pVirUniformSym);
            samplerSubRange = (gctINT16)VIR_Symbol_GetSamplerIdxRange(pVirUniformSym);

            if (pSamplerSym != gcvNULL && pTexSym != gcvNULL)
            {
                gcmASSERT(VIR_Symbol_GetSampler(pVirUniformSym)->physical != -1);

                /* 1. Separated tex part */
                pSymType = VIR_Symbol_GetType(pTexSym);
                if (VIR_Type_GetKind(pSymType) == VIR_TY_ARRAY)
                {
                    resArraySize = VIR_Type_GetArrayLength(pSymType);
                }
                else
                {
                    resArraySize = 1;
                }

                if (texSubRange == -1)
                {
                    /* We dont support dynamic indexing now */
                    gcmASSERT(gcvFALSE);

                    texSubRange= (gctINT16)resArraySize;
                }
                else
                {
                    texStartIdx = texSubRange;
                    texSubRange = 1;
                }

                pResSet = _GetVkResourceSetBySetIdx(pOutPEP, VIR_Symbol_GetDescriptorSet(pTexSym));
                if (pResSet == gcvNULL)
                {
                    return VSC_ERR_OUT_OF_MEMORY;
                }
                for (j = 0; j < pResSet->separatedTexTable.countOfEntries; j ++)
                {
                    pSeparatedTexEntry = &pResSet->separatedTexTable.pTextureEntries[j];

                    if (VIR_Symbol_GetDescriptorSet(pTexSym) == pSeparatedTexEntry->texBinding.set &&
                        VIR_Symbol_GetBinding(pTexSym) == pSeparatedTexEntry->texBinding.binding &&
                        resArraySize == pSeparatedTexEntry->texBinding.arraySize)
                    {
                        pPctsHwMapping = _enlargeVkPctsHwMappingEntryRoom(&pOutPEP->u.vk.privateCombTsHwMappingPool,
                                                                          1,
                                                                          &pctsHmEntryIndex);
                        if (pPctsHwMapping == gcvNULL)
                        {
                            return VSC_ERR_OUT_OF_MEMORY;
                        }
                        memset(pPctsHwMapping, 0, sizeof(PROG_VK_PRIV_COMB_TEX_SAMP_HW_MAPPING));

                        pPctsHwMapping->pctsHmEntryIndex = pctsHmEntryIndex;
                        pPctsHwMapping->bSamplerMajor = gcvTRUE;

                        pPctsHwMapping->texSubBinding.pResBinding = &pSeparatedTexEntry->texBinding;
                        pPctsHwMapping->texSubBinding.startIdxOfSubArray = texStartIdx;
                        pPctsHwMapping->texSubBinding.subArraySize = texSubRange;

                        pPctsHwMapping->pSamplerSlotMapping =
                                       &pOutPEP->seps[stageIdx].samplerMapping.pSampler[VIR_Symbol_GetFirstSlot(pVirUniformSym)];
                        pSeparatedTexEntry->activeStageMask |= 1 << stageIdx;
                        pSeparatedTexEntry->bUsingHwMppingList = gcvTRUE;
                        pSeparatedTexEntry->stageBits |= VSC_SHADER_STAGE_2_STAGE_BIT(stageIdx);
                        pPctsHwMappingListIdx = _enlargeVkPctsHwMappingEntryIdxListRoom(&pSeparatedTexEntry->hwMappings[stageIdx].s.texHwMappingList,
                                                                                       1,
                                                                                       gcvNULL);
                        if (pPctsHwMappingListIdx == gcvNULL)
                        {
                            return VSC_ERR_OUT_OF_MEMORY;
                        }
                        *pPctsHwMappingListIdx = pctsHmEntryIndex;

                        break;
                    }
                }

                /* 2. Separated sampler part */
                pSymType = VIR_Symbol_GetType(pSamplerSym);
                if (VIR_Type_GetKind(pSymType) == VIR_TY_ARRAY)
                {
                    resArraySize = VIR_Type_GetArrayLength(pSymType);
                }
                else
                {
                    resArraySize = 1;
                }

                if (samplerSubRange == -1)
                {
                    /* We dont support dynamic indexing now */
                    gcmASSERT(gcvFALSE);

                    samplerSubRange= (gctINT16)resArraySize;
                }
                else
                {
                    samplerStartIdx = samplerSubRange;
                    samplerSubRange = 1;
                }

                pResSet = _GetVkResourceSetBySetIdx(pOutPEP, VIR_Symbol_GetDescriptorSet(pSamplerSym));
                if (pResSet == gcvNULL)
                {
                    return VSC_ERR_OUT_OF_MEMORY;
                }
                for (j = 0; j < pResSet->separatedSamplerTable.countOfEntries; j ++)
                {
                    pSeparatedSamplerEntry = &pResSet->separatedSamplerTable.pSamplerEntries[j];

                    if (VIR_Symbol_GetDescriptorSet(pSamplerSym) == pSeparatedSamplerEntry->samplerBinding.set &&
                        VIR_Symbol_GetBinding(pSamplerSym) == pSeparatedSamplerEntry->samplerBinding.binding &&
                        resArraySize == pSeparatedSamplerEntry->samplerBinding.arraySize)
                    {
                        gcmASSERT(pPctsHwMapping);

                        pPctsHwMapping->samplerSubBinding.pResBinding = &pSeparatedSamplerEntry->samplerBinding;
                        pPctsHwMapping->samplerSubBinding.startIdxOfSubArray = samplerStartIdx;
                        pPctsHwMapping->samplerSubBinding.subArraySize = samplerSubRange;
                        _AddExtraSamplerArray(&pPctsHwMapping->ppExtraSamplerArray,
                                              &pSeparatedSamplerEntry->samplerBinding,
                                              pShader,
                                              &pOutPEP->seps[stageIdx],
                                              gcvFALSE,
                                              gcvTRUE,
                                              pPctsHwMapping->samplerSubBinding.subArraySize * pPctsHwMapping->texSubBinding.subArraySize,
                                              pPctsHwMapping->texSubBinding.subArraySize,
                                              0
                                              );

                        _FillSamplerDerivedInfo(pShader,
                                                &pSeparatedSamplerEntry->samplerBinding,
                                                &pOutPEP->seps[stageIdx],
                                                &pPctsHwMapping->samplerDerivedInfo,
                                                gcvFALSE,
                                                gcvTRUE);

                        pSeparatedSamplerEntry->activeStageMask |= 1 << stageIdx;
                        pSeparatedSamplerEntry->bUsingHwMppingList = gcvTRUE;
                        pSeparatedSamplerEntry->stageBits |= VSC_SHADER_STAGE_2_STAGE_BIT(stageIdx);
                        pPctsHwMappingListIdx = _enlargeVkPctsHwMappingEntryIdxListRoom(&pSeparatedSamplerEntry->hwMappings[stageIdx].samplerHwMappingList,
                                                                                        1,
                                                                                        gcvNULL);
                        if (pPctsHwMappingListIdx == gcvNULL)
                        {
                            return VSC_ERR_OUT_OF_MEMORY;
                        }
                        *pPctsHwMappingListIdx = pctsHmEntryIndex;

                        break;
                    }
                }
            }
        }
    }

    return errCode;
}

static VSC_ErrCode _PostProcessImageDerivedInfo(PROG_VK_IMAGE_DERIVED_INFO* pImageDerivedInfo,
                                                VSC_SHADER_RESOURCE_BINDING* pResBinding,
                                                gctUINT entryIndex)
{
    VSC_ErrCode                                 errCode = VSC_ERR_NONE;
    SHADER_PRIV_CONSTANT_ENTRY*                 pPrivCnstEntry;
    SHADER_PRIV_UAV_ENTRY*                      pPrivUavEntry;
    gctUINT                                     i;

    /* Image size */
    pPrivCnstEntry = pImageDerivedInfo->pImageSize;
    if (pPrivCnstEntry)
    {
        if (gcoOS_Allocate(gcvNULL, sizeof(gctUINT), (gctPOINTER*)&pPrivCnstEntry->commonPrivm.pPrivateData) != gcvSTATUS_OK)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
        *(gctUINT*)pPrivCnstEntry->commonPrivm.pPrivateData = entryIndex;
    }

    /* Mipmap level */
    pPrivCnstEntry = pImageDerivedInfo->pMipLevel;
    if (pPrivCnstEntry)
    {
        if (gcoOS_Allocate(gcvNULL, sizeof(gctUINT), (gctPOINTER*)&pPrivCnstEntry->commonPrivm.pPrivateData) != gcvSTATUS_OK)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
        *(gctUINT*)pPrivCnstEntry->commonPrivm.pPrivateData = entryIndex;
    }

    /* Extra image layer */
    pPrivUavEntry = pImageDerivedInfo->pExtraLayer;
    if (pPrivUavEntry)
    {
        for (i = 0; i < pResBinding->arraySize; i++)
        {
            if (gcoOS_Allocate(gcvNULL, sizeof(gctUINT), (gctPOINTER*)&pPrivUavEntry->commonPrivm.pPrivateData) != gcvSTATUS_OK)
            {
                return VSC_ERR_OUT_OF_MEMORY;
            }
            *(gctUINT*)pPrivUavEntry->commonPrivm.pPrivateData = entryIndex;

            pPrivUavEntry++;
        }
    }

    return errCode;
}

static VSC_ErrCode _PostProcessSamplerDerivedInfo(PROG_VK_SAMPLER_DERIVED_INFO* pSamplerDerivedInfo,
                                                  VSC_SHADER_RESOURCE_BINDING* pResBinding,
                                                  gctUINT entryIndex,
                                                  gctBOOL bAllocate)
{
    VSC_ErrCode                                   errCode = VSC_ERR_NONE;
    gctUINT                                       layerIdx;
    SHADER_PRIV_CONSTANT_ENTRY*                   pPrivCnstEntry;

    for (layerIdx = 0; layerIdx < 2; layerIdx++)
    {
        /* Texture size */
        pPrivCnstEntry = pSamplerDerivedInfo->pTextureSize[layerIdx];
        if (pPrivCnstEntry)
        {
            if (bAllocate)
            {
                if (gcoOS_Allocate(gcvNULL, sizeof(gctUINT), (gctPOINTER*)&pPrivCnstEntry->commonPrivm.pPrivateData) != gcvSTATUS_OK)
                {
                    return VSC_ERR_OUT_OF_MEMORY;
                }
                *(gctUINT*)pPrivCnstEntry->commonPrivm.pPrivateData = entryIndex;
            }
            else
            {
                pPrivCnstEntry->commonPrivm.pPrivateData = gcvNULL;
            }
        }

        /* lodMinMax */
        pPrivCnstEntry = pSamplerDerivedInfo->pLodMinMax[layerIdx];
        if (pPrivCnstEntry)
        {
            if (bAllocate)
            {
                if (gcoOS_Allocate(gcvNULL, sizeof(gctUINT), (gctPOINTER*)&pPrivCnstEntry->commonPrivm.pPrivateData) != gcvSTATUS_OK)
                {
                    return VSC_ERR_OUT_OF_MEMORY;
                }
                *(gctUINT*)pPrivCnstEntry->commonPrivm.pPrivateData = entryIndex;
            }
            else
            {
                pPrivCnstEntry->commonPrivm.pPrivateData = gcvNULL;
            }
        }

        /* levelsSamples */
        pPrivCnstEntry = pSamplerDerivedInfo->pLevelsSamples[layerIdx];
        if (pPrivCnstEntry)
        {
            if (bAllocate)
            {
                if (gcoOS_Allocate(gcvNULL, sizeof(gctUINT), (gctPOINTER*)&pPrivCnstEntry->commonPrivm.pPrivateData) != gcvSTATUS_OK)
                {
                    return VSC_ERR_OUT_OF_MEMORY;
                }
                *(gctUINT*)pPrivCnstEntry->commonPrivm.pPrivateData = entryIndex;
            }
            else
            {
                pPrivCnstEntry->commonPrivm.pPrivateData = gcvNULL;
            }
        }
    }

    return errCode;
}

static VSC_ErrCode _PostProcessVkCombStTable(VSC_PEP_GEN_HELPER* pPepGenHelper, PROG_VK_COMBINED_TEXTURE_SAMPLER_TABLE* pCombinedSampTexTable, gctUINT stageIdx)
{
    VSC_ErrCode                                errCode = VSC_ERR_NONE;
    PROG_VK_COMBINED_TEX_SAMPLER_TABLE_ENTRY*  pComTsEntry = gcvNULL;
    gctUINT                                    i, j, k;
    SHADER_PRIV_SAMPLER_ENTRY*                 pPrivSamplerEntry;
    SHADER_PRIV_UAV_ENTRY*                     pPrivUavEntry;

    for (i = 0; i < pCombinedSampTexTable->countOfEntries; i ++)
    {
        pComTsEntry = &pCombinedSampTexTable->pCombTsEntries[i];

        /* Extra samplers for VSC_LIB_LINK_TYPE_RESOURCE */
        if (pComTsEntry->hwMappings[stageIdx].ppExtraSamplerArray)
        {
            for (j = 0; j < pComTsEntry->combTsBinding.arraySize; j ++)
            {
                pPrivSamplerEntry = pComTsEntry->hwMappings[stageIdx].ppExtraSamplerArray[j];
                if (pPrivSamplerEntry)
                {
                    if (gcoOS_Allocate(gcvNULL, sizeof(gctUINT), (gctPOINTER*)&pPrivSamplerEntry->commonPrivm.pPrivateData) != gcvSTATUS_OK)
                    {
                        return VSC_ERR_OUT_OF_MEMORY;
                    }
                    *(gctUINT*)pPrivSamplerEntry->commonPrivm.pPrivateData = pComTsEntry->combTsEntryIndex;
                }
            }
        }

        /* Ycbcr planes layer */
        if (pComTsEntry->hwMappings[stageIdx].ppYcbcrPlanes)
        {
            for (k = 0; k < (__YCBCR_PLANE_COUNT__ * pComTsEntry->combTsBinding.arraySize); k++)
            {
                pPrivUavEntry = pComTsEntry->hwMappings[stageIdx].ppYcbcrPlanes[k];
                if (pPrivUavEntry)
                {
                    if (gcoOS_Allocate(gcvNULL, sizeof(gctUINT), (gctPOINTER*)&pPrivUavEntry->commonPrivm.pPrivateData) != gcvSTATUS_OK)
                    {
                        return VSC_ERR_OUT_OF_MEMORY;
                    }
                    *(gctUINT*)pPrivUavEntry->commonPrivm.pPrivateData = pComTsEntry->combTsEntryIndex;
                }
            }
        }

        /* Process the sampler derived information. */
        _PostProcessSamplerDerivedInfo(&pComTsEntry->samplerDerivedInfo[stageIdx],
                                       &pComTsEntry->combTsBinding,
                                       pComTsEntry->combTsEntryIndex,
                                       gcvFALSE);
    }

    return errCode;
}

static VSC_ErrCode _PostProcessVkSeparatedTexEntryTable(VSC_PEP_GEN_HELPER* pPepGenHelper, PROG_VK_SEPARATED_TEXTURE_TABLE* pSeparatedTexTable, gctUINT stageIdx)
{
    VSC_ErrCode                             errCode = VSC_ERR_NONE;
    PROG_VK_SEPARATED_TEXTURE_TABLE_ENTRY*  pTexEntry = gcvNULL;
    gctUINT                                 i;

    for (i = 0; i < pSeparatedTexTable->countOfEntries; i ++)
    {
        pTexEntry = &pSeparatedTexTable->pTextureEntries[i];

        if (!pPepGenHelper->baseEpGen.pHwCfg->hwFeatureFlags.supportSeparatedTex)
        {
            /* Process the image derived information. */
            _PostProcessImageDerivedInfo(&pTexEntry->hwMappings[stageIdx].s.imageDerivedInfo,
                                         &pTexEntry->texBinding,
                                         pTexEntry->textureEntryIndex);
        }
    }
    return errCode;
}

static VSC_ErrCode _PostProcessVkUtbEntryTable(VSC_PEP_GEN_HELPER* pPepGenHelper, PROG_VK_UNIFORM_TEXEL_BUFFER_TABLE* pUniformTexBufTable, gctUINT stageIdx)
{
    VSC_ErrCode                               errCode = VSC_ERR_NONE;
    PROG_VK_UNIFORM_TEXEL_BUFFER_TABLE_ENTRY* pUtbEntry = gcvNULL;
    gctUINT                                   i;

    for (i = 0; i < pUniformTexBufTable->countOfEntries; i ++)
    {
        pUtbEntry = &pUniformTexBufTable->pUtbEntries[i];

        /* Process the image derived information. */
        _PostProcessImageDerivedInfo(&pUtbEntry->imageDerivedInfo[stageIdx],
                                     &pUtbEntry->utbBinding,
                                     pUtbEntry->utbEntryIndex);

        /* Process the sampler derived information. */
        _PostProcessSamplerDerivedInfo(&pUtbEntry->samplerDerivedInfo[stageIdx],
                                       &pUtbEntry->utbBinding,
                                       pUtbEntry->utbEntryIndex,
                                       gcvFALSE);
    }

    return errCode;
}

static VSC_ErrCode _PostProcessVkStorageTable(VSC_PEP_GEN_HELPER* pPepGenHelper, PROG_VK_STORAGE_TABLE* pStorageTable, gctUINT stageIdx)
{
    VSC_ErrCode                          errCode = VSC_ERR_NONE;
    PROG_VK_STORAGE_TABLE_ENTRY*         pStorageEntry = gcvNULL;
    gctUINT                              i;

    for (i = 0; i < pStorageTable->countOfEntries; i ++)
    {
        pStorageEntry = &pStorageTable->pStorageEntries[i];

        /* Process the image derived information. */
        _PostProcessImageDerivedInfo(&pStorageEntry->imageDerivedInfo[stageIdx],
                                     &pStorageEntry->storageBinding,
                                     pStorageEntry->storageEntryIndex);
    }

    return errCode;
}

static VSC_ErrCode _PostProcessVkInputAttachmentTable(VSC_PEP_GEN_HELPER* pPepGenHelper, PROG_VK_INPUT_ATTACHMENT_TABLE* pInputAttachmentTable, gctUINT stageIdx)
{
    VSC_ErrCode                          errCode = VSC_ERR_NONE;
    PROG_VK_INPUT_ATTACHMENT_TABLE_ENTRY*pIaEntries;
    gctUINT                              i, j;
    SHADER_PRIV_SAMPLER_ENTRY*           pPrivSamplerEntry;
    gctBOOL                              bIsSampler = gcvFALSE;

    for (i = 0; i < pInputAttachmentTable->countOfEntries; i++)
    {
        pIaEntries = &pInputAttachmentTable->pIaEntries[i];

        bIsSampler = (pIaEntries->hwMappings[stageIdx].uavMapping.hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_SAMPLER);

        if (bIsSampler)
        {
            /* Extra samplers for VSC_LIB_LINK_TYPE_RESOURCE */
            if (pIaEntries->hwMappings[stageIdx].ppExtraSamplerArray)
            {
                for (j = 0; j < pIaEntries->iaBinding.arraySize; j ++)
                {
                    pPrivSamplerEntry = pIaEntries->hwMappings[stageIdx].ppExtraSamplerArray[j];
                    if (pPrivSamplerEntry)
                    {
                        if (gcoOS_Allocate(gcvNULL, sizeof(gctUINT), (gctPOINTER*)&pPrivSamplerEntry->commonPrivm.pPrivateData) != gcvSTATUS_OK)
                        {
                            return VSC_ERR_OUT_OF_MEMORY;
                        }
                        *(gctUINT*)pPrivSamplerEntry->commonPrivm.pPrivateData = pIaEntries->iaEntryIndex;
                    }
                }
            }

            /* Process the sampler derived information. */
            _PostProcessSamplerDerivedInfo(&pIaEntries->samplerDerivedInfo[stageIdx],
                                           &pIaEntries->iaBinding,
                                           pIaEntries->iaEntryIndex,
                                           gcvFALSE);
        }
        else
        {
            /* Process the image derived information. */
            _PostProcessImageDerivedInfo(&pIaEntries->imageDerivedInfo[stageIdx],
                                         &pIaEntries->iaBinding,
                                         pIaEntries->iaEntryIndex);
        }
    }

    return errCode;
}

static VSC_ErrCode _PostProcessResourceSetTables(VSC_PEP_GEN_HELPER* pPepGenHelper, PROGRAM_EXECUTABLE_PROFILE* pOutPEP, gctUINT stageIdx)
{
    VSC_ErrCode                          errCode = VSC_ERR_NONE;
    gctUINT                              set;

    for (set = 0; set < pOutPEP->u.vk.resourceSetCount; set ++)
    {
        errCode = _PostProcessVkCombStTable(pPepGenHelper, &pOutPEP->u.vk.pResourceSets[set].combinedSampTexTable, stageIdx);
        ON_ERROR(errCode, "Post process vk combined sampler/texture table");

        errCode = _PostProcessVkSeparatedTexEntryTable(pPepGenHelper, &pOutPEP->u.vk.pResourceSets[set].separatedTexTable, stageIdx);
        ON_ERROR(errCode, "Post process vk separated texture table");

        errCode = _PostProcessVkUtbEntryTable(pPepGenHelper, &pOutPEP->u.vk.pResourceSets[set].uniformTexBufTable, stageIdx);
        ON_ERROR(errCode, "Post process vk uniform texel buffer table");

        errCode = _PostProcessVkStorageTable(pPepGenHelper, &pOutPEP->u.vk.pResourceSets[set].storageTable, stageIdx);
        ON_ERROR(errCode, "Post process vk storage table");

        errCode = _PostProcessVkInputAttachmentTable(pPepGenHelper, &pOutPEP->u.vk.pResourceSets[set].inputAttachmentTable, stageIdx);
        ON_ERROR(errCode, "Post process vk input attchment table");
    }

OnError:
    return errCode;
}

static VSC_ErrCode _PostProcessPrivateCombTsHwMapping(PROGRAM_EXECUTABLE_PROFILE* pOutPEP, gctUINT stageIdx)
{
    VSC_ErrCode                          errCode = VSC_ERR_NONE;
    gctUINT                              i, j, k;
    PROG_VK_PRIV_COMB_TEX_SAMP_HW_MAPPING* pPrivCombTsHwMappingEntry;
    SHADER_PRIV_SAMPLER_ENTRY*           pPrivSamplerEntry;

    for (i = 0; i < pOutPEP->u.vk.privateCombTsHwMappingPool.countOfArray; i++)
    {
        pPrivCombTsHwMappingEntry = &pOutPEP->u.vk.privateCombTsHwMappingPool.pPrivCombTsHwMappingArray[i];

        if (pPrivCombTsHwMappingEntry->ppExtraSamplerArray)
        {
            for (j = 0; j < pPrivCombTsHwMappingEntry->samplerSubBinding.subArraySize; j++)
            {
                for (k = 0; k < pPrivCombTsHwMappingEntry->texSubBinding.subArraySize; k++)
                {
                    pPrivSamplerEntry = pPrivCombTsHwMappingEntry->ppExtraSamplerArray[j * pPrivCombTsHwMappingEntry->texSubBinding.subArraySize + k];
                    if (pPrivSamplerEntry)
                    {
                        pPrivSamplerEntry->commonPrivm.pPrivateData = gcvNULL;
                    }
                }
            }
        }

        _PostProcessSamplerDerivedInfo(&pPrivCombTsHwMappingEntry->samplerDerivedInfo,
                                       pPrivCombTsHwMappingEntry->samplerSubBinding.pResBinding,
                                       0,
                                       gcvTRUE);
    }

    return errCode;
}

static VSC_ErrCode _CollectResourceLayoutTablesToPEP(VSC_PEP_GEN_HELPER* pPepGenHelper,
                                                     gctUINT stageIdx,
                                                     PROGRAM_EXECUTABLE_PROFILE* pOutPEP)
{
    VSC_ErrCode                                 errCode = VSC_ERR_NONE;
    VIR_Shader*                                 pShader = pPepGenHelper->pShader;
    VIR_SHADER_RESOURCE_ALLOC_LAYOUT*           pResAllocLayout = &pShader->shaderResAllocLayout;
    gctUINT                                     i;
    VIR_SHADER_RESOURCE_ALLOC_ENTRY*            pResAllocEntry;
    PROG_VK_RESOURCE_SET*                       pResSet;
    VIR_SHADER_PUSH_CONSTANT_ALLOC_ENTRY*       pPushCnstAllocEntry;

    /* Resource set */
    for (i = 0; i < pResAllocLayout->resAllocEntryCount; i++)
    {
        pResAllocEntry = &pResAllocLayout->pResAllocEntries[i];

        /* Get corresponding set */
        pResSet = _GetVkResourceSetBySetIdx(pOutPEP, pResAllocEntry->resBinding.set);
        if (pResSet == gcvNULL)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
        if (pResAllocEntry->resBinding.arraySize == 0)
        {
            continue;
        }

        switch (pResAllocEntry->resBinding.type)
        {
        case VSC_SHADER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER:
            errCode = _AddVkCombStEntryToCombStTableOfPEP(pPepGenHelper,
                                                          pResSet,
                                                          &pResSet->combinedSampTexTable,
                                                          pResAllocEntry,
                                                          pShader,
                                                          stageIdx,
                                                          &pOutPEP->seps[stageIdx]);
            ON_ERROR(errCode, "Add vk combined sampler/texture entry to combined sampler/texture table of pep");

            break;

        case VSC_SHADER_RESOURCE_TYPE_SAMPLER:
            errCode = _AddVkSeparatedSamplerEntryToSeparatedSamplerTableOfPEP(pPepGenHelper,
                                                                              pResSet,
                                                                              &pResSet->separatedSamplerTable,
                                                                              pResAllocEntry,
                                                                              pShader,
                                                                              stageIdx,
                                                                              &pOutPEP->seps[stageIdx]);
            ON_ERROR(errCode, "Add vk separated sampler entry to separated sampler table of pep");

            break;

        case VSC_SHADER_RESOURCE_TYPE_SAMPLED_IMAGE:
            errCode = _AddVkSeparatedTexEntryToSeparatedTexTableOfPEP(pPepGenHelper,
                                                                      pResSet,
                                                                      &pResSet->separatedTexTable,
                                                                      pResAllocEntry,
                                                                      pShader,
                                                                      stageIdx,
                                                                      &pOutPEP->seps[stageIdx]);
            ON_ERROR(errCode, "Add vk separated texture entry to separated texture table of pep");

            break;

        case VSC_SHADER_RESOURCE_TYPE_UNIFORM_TEXEL_BUFFER:
            gcmASSERT(pResAllocEntry->hwRegNo != NOT_ASSIGNED);

            errCode = _AddVkUtbEntryToUniformTexBufTableOfPEP(pPepGenHelper,
                                                              pResSet,
                                                              &pResSet->uniformTexBufTable,
                                                              pResAllocEntry,
                                                              pShader,
                                                              stageIdx,
                                                              &pOutPEP->seps[stageIdx]);
            ON_ERROR(errCode, "Add vk uniform tex buffer entry to uniform tex buffer table of pep");

            break;

        case VSC_SHADER_RESOURCE_TYPE_INPUT_ATTACHMENT:
            gcmASSERT(pResAllocEntry->hwRegNo != NOT_ASSIGNED);

            errCode = _AddVkInputAttachmentTableOfPEP(pPepGenHelper,
                                                      pResSet,
                                                      &pResSet->inputAttachmentTable,
                                                      pResAllocEntry,
                                                      pShader,
                                                      stageIdx,
                                                      &pOutPEP->seps[stageIdx]);
            ON_ERROR(errCode, "Add vk input-attachment entry to input-attachment table of pep");

            break;

        case VSC_SHADER_RESOURCE_TYPE_STORAGE_BUFFER:
        case VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE:
        case VSC_SHADER_RESOURCE_TYPE_STORAGE_TEXEL_BUFFER:
        case VSC_SHADER_RESOURCE_TYPE_STORAGE_BUFFER_DYNAMIC:
            gcmASSERT(pResAllocEntry->hwRegNo != NOT_ASSIGNED);

            errCode = _AddVkStorageEntryToStorageTableOfPEP(pPepGenHelper,
                                                            pResSet,
                                                            &pResSet->storageTable,
                                                            pResAllocEntry,
                                                            pShader,
                                                            stageIdx,
                                                            &pOutPEP->seps[stageIdx]);
            ON_ERROR(errCode, "Add vk storage entry to storage table of pep");

            break;

        case VSC_SHADER_RESOURCE_TYPE_UNIFORM_BUFFER:
        case VSC_SHADER_RESOURCE_TYPE_UNIFORM_BUFFER_DYNAMIC:
            gcmASSERT(pResAllocEntry->hwRegNo != NOT_ASSIGNED);

            errCode = _AddVkUbEntryToUbTableOfPEP(pPepGenHelper,
                                                  pResSet,
                                                  &pResSet->uniformBufferTable,
                                                  pResAllocEntry,
                                                  pShader,
                                                  stageIdx,
                                                  &pOutPEP->seps[stageIdx]);
            ON_ERROR(errCode, "Add vk ub entry to ub table of pep");

            break;

        default:
            break;
        }
    }

    /* Push constants */
    for (i = 0; i < pResAllocLayout->pushCnstAllocEntryCount; i ++)
    {
        pPushCnstAllocEntry = &pResAllocLayout->pPushCnstAllocEntries[i];

        errCode = _AddVkPushCnstEntryToPushCnstTableOfPEP(pPepGenHelper,
                                                          &pOutPEP->u.vk.pushConstantTable,
                                                          pPushCnstAllocEntry,
                                                          stageIdx);
        ON_ERROR(errCode, "Add vk push constant entry to push-constant table of pep");
    }

OnError:
    return errCode;
}

static VSC_ErrCode _CollectCompileGenResourceToPEP(VSC_PEP_GEN_HELPER* pPepGenHelper,
                                                   gctUINT stageIdx,
                                                   PROGRAM_EXECUTABLE_PROFILE* pOutPEP)
{
    VSC_ErrCode                                 errCode = VSC_ERR_NONE;

    /* Take care of compiler generated combined sampler for resource set */
    errCode = _CollectCompilerGeneatedCombinedSampler(pPepGenHelper, stageIdx, pOutPEP);
    ON_ERROR(errCode, "Collect compiled generated combined sampler");

OnError:
    return errCode;
}

static VSC_ErrCode _PostProcessCollectResource(VSC_PEP_GEN_HELPER* pPepGenHelper,
                                               gctUINT stageIdx,
                                               PROGRAM_EXECUTABLE_PROFILE* pOutPEP)
{
    VSC_ErrCode                                 errCode = VSC_ERR_NONE;

    /* Post process resource set */
    errCode = _PostProcessResourceSetTables(pPepGenHelper, pOutPEP, stageIdx);
    ON_ERROR(errCode, "Post process resource set tables");

    /* Post privateCombTsHwMappingPool. */
    errCode = _PostProcessPrivateCombTsHwMapping(pOutPEP, stageIdx);
    ON_ERROR(errCode, "Post process resource set tables");

OnError:
    return errCode;
}

DEF_QUERY_PASS_PROP(vscVIR_GenerateSEP)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_CG;
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_SEP_GEN;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;
}

DEF_SH_NECESSITY_CHECK(vscVIR_GenerateSEP)
{
    return gcvTRUE;
}

VSC_ErrCode
vscVIR_GenerateSEP(
    VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_Shader*                pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VSC_HW_CONFIG*             pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    VSC_SEP_GEN_PRIV_DATA*     pPrvData = (VSC_SEP_GEN_PRIV_DATA*)pPassWorker->basePassWorker.pPassSpecificData;
    SHADER_EXECUTABLE_PROFILE* pOutSEP = pPrvData ? pPrvData->pOutSEP : gcvNULL;
    VSC_SEP_GEN_HELPER         sepGenHelper;

    sepGenHelper.pShader = pShader;
    sepGenHelper.baseEpGen.pHwCfg = pHwCfg;
    sepGenHelper.baseEpGen.pMM = pPassWorker->basePassWorker.pMM;
    sepGenHelper.bSkipPepGen = pPrvData ? pPrvData->bSkipPepGen : gcvTRUE;

    vscInitializeSEP(pOutSEP);

    /* SEP profile version */
    pOutSEP->profileVersion = ENCODE_SEP_VER_TYPE(1, 0);

    /* HW target that this SEP works on */
    pOutSEP->chipModel = pHwCfg->chipModel;
    pOutSEP->chipRevision = pHwCfg->chipRevision;
    pOutSEP->productID = pHwCfg->productID;
    pOutSEP->customerID = pHwCfg->customerID;

    /* Shader type and version */
    pOutSEP->shVersionType = _GenerateShaderVersionAndType(pShader);

    /* Collect machine code */
    errCode = _CollectMachineCodeToSEP(&sepGenHelper, pOutSEP);
    ON_ERROR(errCode, "Collect machine code to SEP");

    /* Calc temp register count */
    pOutSEP->gprCount = VIR_Shader_GetRegWatermark(pShader);

    /* Collect all executable hints */
    _CollectExeHints(pPassWorker->pCompilerParam, &sepGenHelper, pOutSEP);

    /* Input-mapping collection */
    errCode = _CollectIoMappingToSEP(&sepGenHelper, gcvTRUE, pOutSEP);
    ON_ERROR(errCode, "Collect input mapping to SEP");

    /* Output-mapping collection */
    errCode = _CollectIoMappingToSEP(&sepGenHelper, gcvFALSE, pOutSEP);
    ON_ERROR(errCode, "Collect output mapping to SEP");

    /* Sampler-mapping collection */
    errCode = _CollectSamplerMappingToSEP(&sepGenHelper, pOutSEP);
    ON_ERROR(errCode, "Collect sampler mapping to SEP");

    /* SRV-mapping collection */
    errCode = _CollectSrvMappingToSEP(&sepGenHelper, pOutSEP);
    ON_ERROR(errCode, "Collect srv mapping to SEP");

    /* Constant-mapping collection */
    errCode = _CollectConstantMappingToSEP(&sepGenHelper, pOutSEP, gcvFALSE);
    ON_ERROR(errCode, "Collect constant mapping to SEP");

    /* UAV-mapping collection */
    errCode = _CollectUavMappingToSEP(&sepGenHelper, pOutSEP, gcvFALSE);
    ON_ERROR(errCode, "Collect uav mapping to SEP");

    /* Constant-mapping post-collection */
    errCode = _CollectConstantMappingToSEP(&sepGenHelper, pOutSEP, gcvTRUE);
    ON_ERROR(errCode, "Collect constant mapping to SEP");

    /* Default UBO allocation. */
    errCode = _AllocateDefaultUboToSEP(&sepGenHelper, pOutSEP);
    ON_ERROR(errCode, "Allocate default UBO.");

    /* UAV-mapping post-collection */
    errCode = _CollectUavMappingToSEP(&sepGenHelper, pOutSEP, gcvTRUE);
    ON_ERROR(errCode, "Collect uav mapping to SEP");

    /* Static-private-mapping collection */
    errCode = _CollectStaticPrivateMappingToSEP(&sepGenHelper, pOutSEP);
    ON_ERROR(errCode, "Collect static private mapping to SEP");

    /* Dynamic-private-mapping collection */
    errCode = _CollectDynamicPrivateMappingToSEP(&sepGenHelper, pOutSEP);
    ON_ERROR(errCode, "Collect dynamic private mapping to SEP");

OnError:
    if (errCode != VSC_ERR_NONE)
    {
        vscFinalizeSEP(pOutSEP);
    }

    return errCode;
}

DEF_QUERY_PASS_PROP(vscVIR_GeneratePEP)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_CG;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;
}

DEF_GPG_NECESSITY_CHECK(vscVIR_GeneratePEP)
{
    return gcvTRUE;
}

VSC_ErrCode
vscVIR_GeneratePEP(
    VSC_GPG_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VSC_HW_CONFIG*              pHwCfg = &pPassWorker->pPgmLinkerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    VSC_PEP_GEN_PRIV_DATA*      pPrvData = (VSC_PEP_GEN_PRIV_DATA*)pPassWorker->basePassWorker.pPassSpecificData;
    PROGRAM_EXECUTABLE_PROFILE* pOutPEP = pPrvData->pOutPEP;
    VSC_PEP_GEN_HELPER          pepGenHelper;
    gctUINT                     stageIdx;

    pepGenHelper.baseEpGen.pHwCfg = pHwCfg;
    pepGenHelper.baseEpGen.pMM = pPassWorker->basePassWorker.pMM;

    /* Set pep client, different client may have different tables for spec requirement */
    gcmASSERT(pPrvData->client != PEP_CLIENT_UNKNOWN);
    pOutPEP->pepClient = pPrvData->client;

    /* SEPs maintained by PEP should have been generated by per-shader compilation kicked by vscLinkProgram */
    gcmASSERT(vscIsValidSEP(&pOutPEP->seps[VSC_SHADER_STAGE_VS]) ||
              vscIsValidSEP(&pOutPEP->seps[VSC_SHADER_STAGE_HS]) ||
              vscIsValidSEP(&pOutPEP->seps[VSC_SHADER_STAGE_DS]) ||
              vscIsValidSEP(&pOutPEP->seps[VSC_SHADER_STAGE_GS]) ||
              vscIsValidSEP(&pOutPEP->seps[VSC_SHADER_STAGE_PS]) ||
              vscIsValidSEP(&pOutPEP->seps[VSC_SHADER_STAGE_CS]));

    /* Attribute table for PEP is only retrieved from VS */
    if (pPassWorker->pPgmLinkerParam->hShaderArray[VSC_SHADER_STAGE_VS])
    {
        gcmASSERT(vscIsValidSEP(&pOutPEP->seps[VSC_SHADER_STAGE_VS]));

        pepGenHelper.pShader = (VIR_Shader*)pPassWorker->pPgmLinkerParam->hShaderArray[VSC_SHADER_STAGE_VS];
        pepGenHelper.pSep = &pOutPEP->seps[VSC_SHADER_STAGE_VS];
        errCode = _CollectAttrTableToPEP(&pepGenHelper, pOutPEP);
        ON_ERROR(errCode, "Collect attribute table to PEP");
    }

    /* Frag-out table for PEP is only retrieved from PS */
    if (pPassWorker->pPgmLinkerParam->hShaderArray[VSC_SHADER_STAGE_PS])
    {
        gcmASSERT(vscIsValidSEP(&pOutPEP->seps[VSC_SHADER_STAGE_PS]));

        pepGenHelper.pShader = (VIR_Shader*)pPassWorker->pPgmLinkerParam->hShaderArray[VSC_SHADER_STAGE_PS];
        pepGenHelper.pSep = &pOutPEP->seps[VSC_SHADER_STAGE_PS];
        errCode = _CollectFragOutTableToPEP(&pepGenHelper, pOutPEP);
        ON_ERROR(errCode, "Collect frag-out table to PEP");
    }

    if (pPrvData->client == PEP_CLIENT_GL)
    {
    }
    else
    {
        /* I: collect all resources that from the resource layout. */
        for (stageIdx = 0; stageIdx < VSC_MAX_SHADER_STAGE_COUNT; stageIdx ++)
        {
            if (pPassWorker->pPgmLinkerParam->hShaderArray[stageIdx])
            {
                pepGenHelper.pShader = (VIR_Shader*)pPassWorker->pPgmLinkerParam->hShaderArray[stageIdx];
                pepGenHelper.pSep = &pOutPEP->seps[stageIdx];
                errCode = _CollectResourceLayoutTablesToPEP(&pepGenHelper, stageIdx, pOutPEP);
                ON_ERROR(errCode, "Collect resource layout table to PEP");
            }
        }

        /* II: collect all resources that generated by compiler. */
        for (stageIdx = 0; stageIdx < VSC_MAX_SHADER_STAGE_COUNT; stageIdx ++)
        {
            if (pPassWorker->pPgmLinkerParam->hShaderArray[stageIdx])
            {
                pepGenHelper.pShader = (VIR_Shader*)pPassWorker->pPgmLinkerParam->hShaderArray[stageIdx];
                pepGenHelper.pSep = &pOutPEP->seps[stageIdx];
                errCode = _CollectCompileGenResourceToPEP(&pepGenHelper, stageIdx, pOutPEP);
                ON_ERROR(errCode, "Collect compiler-generated resources to PEP");
            }
        }

        /* III: do some post work. */
        for (stageIdx = 0; stageIdx < VSC_MAX_SHADER_STAGE_COUNT; stageIdx ++)
        {
            if (pPassWorker->pPgmLinkerParam->hShaderArray[stageIdx])
            {
                pepGenHelper.pShader = (VIR_Shader*)pPassWorker->pPgmLinkerParam->hShaderArray[stageIdx];
                pepGenHelper.pSep = &pOutPEP->seps[stageIdx];
                errCode = _PostProcessCollectResource(&pepGenHelper, stageIdx, pOutPEP);
                ON_ERROR(errCode, "Post process collect resource.");
            }
        }
    }

OnError:

    return errCode;
}

static VSC_ErrCode _CollectCombStTableToKEP(VIR_Shader* pShader,
                                                     KERNEL_EXECUTABLE_PROFILE * pOutKEP)
{
    return VSC_ERR_NONE;
}

static PROG_CL_IMAGE_TABLE_ENTRY* _enlargeCLImageTEntryRoom(PROG_CL_IMAGE_TABLE* pImageTable,
                                                            gctUINT enlargeCount,
                                                            gctUINT* pStartEntryIdx)
{
    void*                         pOldImageTEntry;
    gctUINT                       oldImageTEntryCount, newImageTEntryCount;

    pOldImageTEntry = pImageTable->pImageEntries;
    oldImageTEntryCount = pImageTable->countOfEntries;
    newImageTEntryCount = oldImageTEntryCount + enlargeCount;

    if (gcoOS_Allocate(gcvNULL,
                   sizeof(PROG_CL_IMAGE_TABLE_ENTRY) * newImageTEntryCount,
                   (gctPOINTER*)&pImageTable->pImageEntries) != gcvSTATUS_OK)
    {
        return gcvNULL;
    }

    if (pOldImageTEntry)
    {
        memcpy(pImageTable->pImageEntries, pOldImageTEntry,
               oldImageTEntryCount*sizeof(PROG_CL_IMAGE_TABLE_ENTRY));

        gcoOS_Free(gcvNULL, pOldImageTEntry);
    }

    pImageTable->countOfEntries = newImageTEntryCount;

    if (pStartEntryIdx)
    {
        *pStartEntryIdx = oldImageTEntryCount;
    }

    /* Return first enlarged combined sampler texture entry */
    return &pImageTable->pImageEntries[oldImageTEntryCount];
}

static PROG_CL_UNIFORM_TABLE_ENTRY* _enlargeCLUniformTEntryRoom(PROG_CL_UNIFORM_TABLE* pUniformTable,
                                                                gctUINT enlargeCount,
                                                                gctUINT* pStartEntryIdx)
{
    void*                         pOldImageTEntry;
    gctUINT                       oldImageTEntryCount, newImageTEntryCount;

    pOldImageTEntry = pUniformTable->pUniformEntries;
    oldImageTEntryCount = pUniformTable->countOfEntries;
    newImageTEntryCount = oldImageTEntryCount + enlargeCount;

    if (gcoOS_Allocate(gcvNULL,
                   sizeof(PROG_CL_UNIFORM_TABLE_ENTRY) * newImageTEntryCount,
                   (gctPOINTER*)&pUniformTable->pUniformEntries) != gcvSTATUS_OK)
    {
        return gcvNULL;
    }

    if (pOldImageTEntry)
    {
        memcpy(pUniformTable->pUniformEntries, pOldImageTEntry,
               oldImageTEntryCount*sizeof(PROG_CL_UNIFORM_TABLE_ENTRY));

        gcoOS_Free(gcvNULL, pOldImageTEntry);
    }

    pUniformTable->countOfEntries = newImageTEntryCount;

    if (pStartEntryIdx)
    {
        *pStartEntryIdx = oldImageTEntryCount;
    }

    /* Return first enlarged combined sampler texture entry */
    return &pUniformTable->pUniformEntries[oldImageTEntryCount];
}

static PROG_CL_ARG_ENTRY* _enlargeCLArgEntryRoom(PROG_CL_ARG_TABLE* pArgTable,
                                                 gctUINT enlargeCount,
                                                 gctUINT* pStartEntryIdx)
{
    void*                         pOldEntry;
    gctUINT                       oldEntryCount, newEntryCount;

    pOldEntry = pArgTable->pArgsEntries;
    oldEntryCount = pArgTable->countOfEntries;
    newEntryCount = oldEntryCount + enlargeCount;

    if (gcoOS_Allocate(gcvNULL,
                   sizeof(PROG_CL_ARG_ENTRY) * newEntryCount,
                   (gctPOINTER*)&pArgTable->pArgsEntries) != gcvSTATUS_OK)
    {
        gcmPRINT("Failed to allocate memory for enlargeCLArgEntryRoom.");
        return gcvNULL;
    }

    if (pOldEntry)
    {
        memcpy(pArgTable->pArgsEntries, pOldEntry,
               oldEntryCount*sizeof(PROG_CL_ARG_ENTRY));

        gcoOS_Free(gcvNULL, pOldEntry);
    }

    pArgTable->countOfEntries = newEntryCount;

    if (pStartEntryIdx)
    {
        *pStartEntryIdx = oldEntryCount;
    }

    /* Return first enlarged combined sampler texture entry */
    return &pArgTable->pArgsEntries[oldEntryCount];
}

static VSC_ErrCode _CollectKernelArg(VIR_Symbol*  symbol,
                                     VIR_Shader*  pShader,
                                     KERNEL_EXECUTABLE_PROFILE* pOutKEP,
                                     PROG_CL_ARG_ENTRY ** pArgsEntry)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctUINT argIndex;
    VIR_Uniform * uniform;
    gctBOOL bIsSampler = gcvFALSE;
    gctBOOL bIsImage = gcvFALSE;

    uniform = VIR_Symbol_GetUniformPointer(pShader, symbol);
    bIsSampler = VIR_Symbol_isSampler(symbol) || VIR_Symbol_isSamplerT(symbol);
    bIsImage = VIR_Symbol_isImage(symbol) || VIR_Symbol_isImageT(symbol);

    if (VIR_Uniform_isKernelArg(uniform))
    {
        gctSTRING name;
        gctCHAR   buffer[1000];
        VIR_TypeId typeId;
        VIR_Type * type;
        argIndex = VIR_Uniform_GetKernelArgIndex(uniform);

        if (argIndex >= pOutKEP->argTable.countOfEntries)
        {
            if (_enlargeCLArgEntryRoom(&pOutKEP->argTable,
                argIndex - pOutKEP->argTable.countOfEntries + 1, gcvNULL) == gcvNULL)
            {
                return VSC_ERR_OUT_OF_MEMORY;
            }
        }

        *pArgsEntry = &pOutKEP->argTable.pArgsEntries[argIndex];

        (*pArgsEntry)->argIndex = argIndex;
        (*pArgsEntry)->isSampler = bIsSampler;
        (*pArgsEntry)->isImage = bIsImage;
        (*pArgsEntry)->isPointer = VIR_Uniform_isPointer(uniform);
        (*pArgsEntry)->typeQualifier = VIR_Symbol_GetTyQualifier(symbol);
        (*pArgsEntry)->accessQualifier = VIR_Symbol_GetTyQualifier(symbol);
        (*pArgsEntry)->addressQualifier= VIR_Symbol_GetAddrSpace(symbol);
        gcmASSERT(VIR_TypeId_isPrimitive(VIR_Symbol_GetTypeId(symbol)));
        (*pArgsEntry)->type = (VSC_SHADER_DATA_TYPE)VIR_Symbol_GetTypeId(symbol);
        name = VIR_Shader_GetStringFromId(VIR_Symbol_GetShader(symbol), VIR_Symbol_GetName(symbol));
        gcmVERIFY_OK(gcoOS_StrDup(gcvNULL, name, &(*pArgsEntry)->argName));

        typeId = (VIR_Uniform_GetDerivedType(uniform) != VIR_TYPE_UNKNOWN) ?
                      VIR_Uniform_GetDerivedType(uniform) : VIR_Symbol_GetTypeId(symbol);
        type = VIR_Shader_GetTypeFromId(pShader, typeId);
        if(VIR_Type_GetKind(type) == VIR_TY_POINTER && !VIR_Uniform_isTrulyDefinedPointer(uniform))
        {
            typeId = VIR_Type_GetBaseTypeId(type);
        }
        ON_ERROR(VIR_Dump_OCLTypeName(pShader,
                                      typeId,
                                      buffer,
                                      sizeof(buffer)),
                                      "dump type name");
        gcmVERIFY_OK(gcoOS_StrDup(gcvNULL, buffer, &(*pArgsEntry)->argTypeName));

        if (bIsSampler)
        {
            pOutKEP->kernelHints.samplerCount++;
        }

        if (bIsImage)
        {
            pOutKEP->kernelHints.imageCount ++;
        }

        return errCode;
    }

OnError:
    (*pArgsEntry) = gcvNULL;
    return errCode;
}

static VSC_ErrCode _GetImageSamplerPairs(VIR_Shader* pShader,
                                               VIR_Symbol*  symbol,
                                               PROG_CL_IMAGE_TABLE_ENTRY * entry)
{
    gctUINT imageArgIndex = 0xffffffff;
    gctUINT samplerArgIndex = 0xffffffff;
    gctUINT samplerValue = 0;
    VIR_Uniform  * uniform;
    VIR_Symbol* pVirSamplerSym;
    VIR_SymId id;

    uniform = VIR_Symbol_GetImage(symbol);
    imageArgIndex = VIR_Uniform_GetKernelArgIndex(uniform);

    if (imageArgIndex == -1)
    {
        VIR_Symbol* pVirImageSym;

        id = VIR_Uniform_GetImageSymId(uniform);
        pVirImageSym = VIR_Shader_GetSymFromId(pShader, id);
        imageArgIndex = VIR_Uniform_GetKernelArgIndex(VIR_Symbol_GetImage(pVirImageSym));
        gcmASSERT(imageArgIndex != -1);
    }

    if (VIR_Uniform_GetImageSamplerSymId(uniform) != VIR_INVALID_ID)
    {
        id = VIR_Uniform_GetImageSamplerSymId(uniform);
        pVirSamplerSym = VIR_Shader_GetSymFromId(pShader, id);
        samplerArgIndex = VIR_Uniform_GetKernelArgIndex(VIR_Symbol_GetSampler(pVirSamplerSym));
        entry->kernelHardcodeSampler = gcvFALSE;
        entry->assumedSamplerValue = VIR_Uniform_GetImageSamplerValue(uniform);
    }
    else
    {
        samplerValue = VIR_Uniform_GetImageSamplerValue(uniform);
        entry->kernelHardcodeSampler = gcvTRUE;
    }

    entry->imageArgIndex = imageArgIndex;
    entry->samplerArgIndex = samplerArgIndex;
    entry->constSamplerValue = samplerValue;
    gcoOS_MemCopy(entry->imageDesc.rawbits, uniform->u0.imageDesc.rawbits, 8*sizeof(gctUINT));

    return VSC_ERR_NONE;
}

static VSC_ErrCode _CollectConstantMappingToKEP(VIR_Shader* pShader,
                                                KERNEL_EXECUTABLE_PROFILE* pOutKEP)
{
    VSC_ErrCode                        errCode = VSC_ERR_NONE;
    VIR_UniformIdList*                 pVirUniformLsts = VIR_Shader_GetUniforms(pShader);
    VIR_Symbol*                        pVirUniformSym;
    SHADER_CONSTANT_MAPPING*           pCnstMapping = &pOutKEP->sep.constantMapping;
    gctUINT                            arraySlot;
    gctUINT                            firstSlot;
    PROG_CL_IMAGE_TABLE_ENTRY *        imageEntry;
    PROG_CL_UNIFORM_TABLE_ENTRY*       uniformEntry;
    gctUINT                            virUniformIdx;
    gctUINT                            entryIdx;
    PROG_CL_ARG_ENTRY *                pArgEntry = gcvNULL;

    for (virUniformIdx = 0; virUniformIdx < VIR_IdList_Count(pVirUniformLsts); virUniformIdx ++)
    {
        pVirUniformSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pVirUniformLsts, virUniformIdx));

        errCode = _CollectKernelArg(pVirUniformSym, pShader, pOutKEP, &pArgEntry);
        ON_ERROR(errCode, "Failed to collect kernel argument.");

        arraySlot = VIR_Symbol_GetArraySlot(pVirUniformSym);
        firstSlot = VIR_Symbol_GetFirstSlot(pVirUniformSym);

        /* We have physica addres */
        if (arraySlot != NOT_ASSIGNED && firstSlot != NOT_ASSIGNED)
        {
            if (VIR_Symbol_isImageT(pVirUniformSym) ||
                VIR_Symbol_isImage(pVirUniformSym))
            {
                imageEntry = _enlargeCLImageTEntryRoom(&pOutKEP->resourceTable.imageTable, 1, &entryIdx);
                if (imageEntry == gcvNULL)
                {
                    return VSC_ERR_OUT_OF_MEMORY;
                }

                _GetImageSamplerPairs(pShader, pVirUniformSym, imageEntry);

                imageEntry->hwMapping = &pCnstMapping->pConstantArrayMapping[arraySlot].pSubConstantArrays[firstSlot];
            }
            else if (pArgEntry)
            {
                uniformEntry = _enlargeCLUniformTEntryRoom(&pOutKEP->resourceTable.uniformTable, 1, &entryIdx);
                if (uniformEntry == gcvNULL)
                {
                    return VSC_ERR_OUT_OF_MEMORY;
                }
                uniformEntry->argIndex = pArgEntry->argIndex;
                uniformEntry->common.hwMapping = &pCnstMapping->pConstantArrayMapping[arraySlot].pSubConstantArrays[firstSlot];
            }
            else if (VIR_Symbol_GetUniformKind(pVirUniformSym) == VIR_UNIFORM_PRINTF_ADDRESS)
            {
                pOutKEP->kernelHints.hasPrintf = gcvTRUE;
            }
            else
            {
                if (VirSHADER_DumpCodeGenVerbose(pShader))
                {
                    gcmPRINT("skip symbol with hw location, name id = %d",VIR_Symbol_GetName(pVirUniformSym));
                }
            }
        }
        else
        {
            if (VirSHADER_DumpCodeGenVerbose(pShader))
            {
                gcmPRINT("skip name id = %d",VIR_Symbol_GetName(pVirUniformSym));
            }
        }
    }

    /* loop private table to collect skipped uniforms */
OnError:
    return errCode;
}

static VSC_ErrCode _CollectResourceTablesToKEP(VIR_Shader* pShader,
                                                        VIR_Function * pVirFunction,
                                                        KERNEL_EXECUTABLE_PROFILE* pOutKEP)
{
    VSC_ErrCode                                 errCode = VSC_ERR_NONE;

    errCode = _CollectCombStTableToKEP(pShader, pOutKEP);

    ON_ERROR(errCode, "Add cl combined sampler/texture entry to resource combined sampler/texture table of KEP");

    errCode = _CollectConstantMappingToKEP(pShader, pOutKEP);

    ON_ERROR(errCode, "Add CL constant entry entry to reousrce uniform table of pep");

OnError:
    return errCode;
}

DECLARE_QUERY_PASS_PROP(vscVIR_GenerateKEP)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_PST;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;
}

DEF_SH_NECESSITY_CHECK(vscVIR_GenerateKEP)
{
    return gcvTRUE;
}

VSC_ErrCode
vscVIR_GenerateKEP(
    VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_Shader*                pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    KERNEL_EXECUTABLE_PROFILE* pOutKEP = (KERNEL_EXECUTABLE_PROFILE*)pPassWorker->basePassWorker.pPassSpecificData;
    VIR_Function *             pVirFunction = VIR_Shader_GetCurrentKernelFunction(pShader);
    gctUINT                    i,j;

    if (pOutKEP && vscIsValidSEP(&pOutKEP->sep))
    {
        if (pVirFunction)
        {
            /* collect function level hints */
            VIR_KernelInfo * kernelInfo = pVirFunction->kernelInfo;
            VIR_KernelProperty * property;

            if (kernelInfo)
            {
                for (i = 0; i < VIR_ValueList_Count(&kernelInfo->properties); i++)
                {
                    property = (VIR_KernelProperty *)(VIR_ValueList_GetValue(&kernelInfo->properties, i));

                    pOutKEP->kernelHints.property[i].type  = property->propertyType;
                    pOutKEP->kernelHints.property[i].size  = property->propertySize;

                    for (j = 0; j < property->propertySize; j++)
                    {
                        pOutKEP->kernelHints.property[property->propertyType].value[j] = property->propertyValue[j];
                    }
                }

                pOutKEP->kernelHints.localMemorySize = kernelInfo->localMemorySize;
            }

            pOutKEP->kernelHints.privateMemorySize = pShader->privateMemorySize;
            /* Don't see this get or send from gcSL, is this really useful in VIR? Need put in SEP hints later */
            pOutKEP->kernelHints.constantMemorySize = pShader->constantMemorySize;
            if(pOutKEP->kernelHints.constantMemorySize)
            {
               gctPOINTER pointer;

               if(pOutKEP->kernelHints.constantMemBuffer)
               {
                  gcmOS_SAFE_FREE(gcvNULL, pOutKEP->kernelHints.constantMemBuffer);
               }
               /* Allocate a new buffer to store the constants */
               if (gcoOS_Allocate(gcvNULL,
                            gcmSIZEOF(gctCHAR) * pShader->constantMemorySize,
                            &pointer) != gcvSTATUS_OK)
               {
                   return VSC_ERR_OUT_OF_MEMORY;
               }

               pOutKEP->kernelHints.constantMemBuffer = (gctCHAR *)pointer;
               gcoOS_MemCopy(pOutKEP->kernelHints.constantMemBuffer,
                             pShader->constantMemoryBuffer,
                             pShader->constantMemorySize);
            }

            _CollectResourceTablesToKEP(pShader, pVirFunction, pOutKEP);
        }
    }
    return errCode;
}


