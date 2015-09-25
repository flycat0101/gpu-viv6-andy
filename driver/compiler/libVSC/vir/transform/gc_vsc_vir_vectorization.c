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
#include "vir/transform/gc_vsc_vir_vectorization.h"

typedef struct VIR_VECTORIZED_INFO
{
    VIR_Symbol*                  pVectorizedSym;
    VIR_Symbol**                 ppVectorizedVirRegArray;
    gctUINT                      virRegRange;
}VIR_VECTORIZED_INFO;

typedef struct VIR_IO_VECTORIZED_INFO
{
    VIR_IO_VECTORIZABLE_PACKET*  pIoPacket;
    VIR_VECTORIZED_INFO          vectorizedInfo;
}VIR_IO_VECTORIZED_INFO;

typedef enum VIR_OPND_VECTORIZE_TYPE
{
    VIR_OPND_VECTORIZE_TYPE_UNKNOW     = 0,
    VIR_OPND_VECTORIZE_TYPE_SYM_2_SYM,
    VIR_OPND_VECTORIZE_TYPE_VIRREG_2_SYM,
    VIR_OPND_VECTORIZE_TYPE_SYM_2_VIRREG,
    VIR_OPND_VECTORIZE_TYPE_VIRREG_2_VIRREG,
    VIR_OPND_VECTORIZE_TYPE_SIMM_2_SIMM,
    VIR_OPND_VECTORIZE_TYPE_VIMM_2_SIMM,
    VIR_OPND_VECTORIZE_TYPE_SIMM_2_VIMM,
    VIR_OPND_VECTORIZE_TYPE_VIMM_2_VIMM,
}VIR_OPND_VECTORIZE_TYPE;

typedef enum VIR_OPND_VECTORIZABILITY_CHK_RES
{
    VIR_OPND_VECTORIZABILITY_CHK_RES_DIRECT_VECTORIZE = 0,
    VIR_OPND_VECTORIZABILITY_CHK_RES_NEED_SYM_OR_VIRREG_VECTORIZE,
    VIR_OPND_VECTORIZABILITY_CHK_RES_DISCARD,
}VIR_OPND_VECTORIZABILITY_CHK_RES;

typedef enum VIR_OPND_VECTORIZE_MODE
{
    VIR_OPND_VECTORIZE_MODE_TO_SEED_INST = 0,
    VIR_OPND_VECTORIZE_MODE_FROM_SEED_INST,
}VIR_OPND_VECTORIZE_MODE;

typedef struct VIR_OPERAND_SYM_PAIR
{
    /* For sym or virreg operand only */

    VIR_Operand*                 pSeedOpnd;
    VIR_Operand*                 pOpnd;
}VIR_OPERAND_PAIR;

typedef struct VIR_INST_PAIR
{
    VIR_Instruction*             pSeedInst;
    VIR_Instruction*             pInst;
}VIR_INST_PAIR;

typedef struct VIR_OPND_VECTORIZED_INFO
{
    /* Info for current vectorization round */
    VIR_OPND_VECTORIZE_TYPE                 ovType;
    VIR_OPND_VECTORIZABILITY_CHK_RES        ovcResult;
    VIR_INST_PAIR                           instPair;
    VIR_OPERAND_PAIR                        opndPair;

    /* Info shared by all vectorization rounds for same seed inst */
    VIR_VECTORIZED_INFO                     vectorizedInfo;
}VIR_OPND_VECTORIZED_INFO;

typedef VIR_OPND_VECTORIZABILITY_CHK_RES
(*PFN_OPND_VECTORIZABILITY_CHECK)(VIR_Shader*,
                                  VIR_OPND_VECTORIZED_INFO*,
                                  gctBOOL);

typedef VSC_ErrCode
(*PFN_VECTORIZE_OPND)(VIR_Shader*,
                      VIR_BASIC_BLOCK*,
                      VIR_DEF_USAGE_INFO*,
                      VIR_OPND_VECTORIZED_INFO*,
                      gctUINT8,
                      gctUINT8,
                      gctBOOL);

typedef VSC_ErrCode
(*PFN_VECTORIZE_SYMORVIRREG)(VIR_Shader*,
                             VIR_DEF_USAGE_INFO*,
                             VIR_OPND_VECTORIZED_INFO*,
                             gctUINT8,
                             gctUINT*,
                             gctBOOL,
                             gctBOOL*);

typedef struct VIR_OPND_VECTORIZE_CALLBACKS
{
    VIR_OPND_VECTORIZE_TYPE          ovType;
    PFN_OPND_VECTORIZABILITY_CHECK   pfnOvCheck;
    PFN_VECTORIZE_SYMORVIRREG        pfnSymOrVirregVectorize;
    PFN_VECTORIZE_OPND               pfnOvVectorize;
}VIR_OPND_VECTORIZE_CALLBACKS;

static gctBOOL _CheckSymsVectorizability(VIR_Shader* pShader, VIR_Symbol** ppSymArray, gctUINT symCount)
{
    gctUINT           i, compCount = 0;

    for (i = 0; i < symCount; i ++)
    {
        if (i > 0)
        {
            if (!vscVIR_CheckTwoSymsVectorizability(pShader, ppSymArray[i], ppSymArray[i-1]))
            {
                return gcvFALSE;
            }
        }

        compCount += VIR_GetTypeComponents(VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(ppSymArray[i])));
    }

    if (compCount > VIR_CHANNEL_COUNT)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

static gctSTRING _GetVectorizedSymStrName(VIR_Shader* pShader, VIR_Symbol** ppSymArray, gctUINT symCount)
{
    gctUINT        i, newNameLength = 0;
    gctSTRING      strOrgName[16];
    gctSTRING      strNewName, strNameAfterDot;
    gctSTRING      strConnect = "+";

    for (i = 0; i < symCount; i ++)
    {
        strOrgName[i] = VIR_Shader_GetSymNameString(pShader, ppSymArray[i]);

        strNameAfterDot = gcvNULL;
        if (isSymIOBlockMember(ppSymArray[i]) &&
            isSymInstanceMember(ppSymArray[i]))
        {
            gcoOS_StrStr(strOrgName[i], ".", &strNameAfterDot);

            /* Instance name */
            if (i == 0 && strNameAfterDot)
            {
                strNameAfterDot[0] = '\0';
                newNameLength += strlen(strOrgName[i]);
                strNameAfterDot[0] = '.';
            }
        }

        if (strNameAfterDot)
        {
            newNameLength += strlen(&strNameAfterDot[1]);
        }
        else
        {
            newNameLength += strlen(strOrgName[i]);
        }
    }
    newNameLength += 8;

    strNewName = (gctSTRING)vscMM_Alloc(&pShader->mempool, newNameLength);
    strNewName[0] = '\0';

    /* Instance name */
    if (isSymIOBlockMember(ppSymArray[0]) &&
        isSymInstanceMember(ppSymArray[0]))
    {
        strNameAfterDot = gcvNULL;
        gcoOS_StrStr(strOrgName[0], ".", &strNameAfterDot);

        if (strNameAfterDot)
        {
            strNameAfterDot[0] = '\0';
            gcoOS_StrCatSafe(strNewName, newNameLength, strOrgName[0]);
            strNameAfterDot[0] = '.';
        }

        gcoOS_StrCatSafe(strNewName, newNameLength, ".");
    }

    for (i = 0; i < symCount; i ++)
    {
        strNameAfterDot = gcvNULL;
        if (isSymIOBlockMember(ppSymArray[i]) &&
            isSymInstanceMember(ppSymArray[i]))
        {
            gcoOS_StrStr(strOrgName[i], ".", &strNameAfterDot);
        }

        if (strNameAfterDot)
        {
            gcoOS_StrCatSafe(strNewName, newNameLength, &strNameAfterDot[1]);
        }
        else
        {
            gcoOS_StrCatSafe(strNewName, newNameLength, strOrgName[i]);
        }

        if (i < symCount - 1)
        {
            gcoOS_StrCatSafe(strNewName, newNameLength, strConnect);
        }
    }

    return strNewName;
}

static VIR_Type* _GetVectorizedSymType(VIR_Shader* pShader, VIR_Symbol** ppSymArray, gctUINT symCount)
{
    gctUINT           i, compCount = 0, rowsCount;
    VIR_TypeId        compType, newTypeId = VIR_TYPE_UNKNOWN;

    for (i = 0; i < symCount; i ++)
    {
        compCount += VIR_GetTypeComponents(VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(ppSymArray[i])));
    }

    compType = VIR_GetTypeComponentType(VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(ppSymArray[0])));
    rowsCount = VIR_GetTypeRows(VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(ppSymArray[0])));

    newTypeId = VIR_TypeId_ComposeNonOpaqueArrayedType(pShader,
                                                       compType,
                                                       compCount,
                                                       rowsCount,
                                                       VIR_Type_isArray(VIR_Symbol_GetType(ppSymArray[0])) ?
                                                       VIR_Type_GetArrayLength(VIR_Symbol_GetType(ppSymArray[0])) :
                                                       -1);

    return VIR_Shader_GetTypeFromId(pShader, newTypeId);
}

static gctBOOL _CheckIoPacketVectorizability(VIR_Shader* pShader, VIR_IO_VECTORIZABLE_PACKET* pIoPacket)
{
    return _CheckSymsVectorizability(pShader, pIoPacket->pSymIo, pIoPacket->realCount);
}

static gctSTRING _GetVectorizedIoSymStrName(VIR_Shader* pShader, VIR_IO_VECTORIZABLE_PACKET* pIoPacket)
{
    return _GetVectorizedSymStrName(pShader, pIoPacket->pSymIo, pIoPacket->realCount);
}

static VIR_Type* _GetVectorizedIoSymType(VIR_Shader* pShader, VIR_IO_VECTORIZABLE_PACKET* pIoPacket)
{
    return _GetVectorizedSymType(pShader, pIoPacket->pSymIo, pIoPacket->realCount);
}

static VSC_ErrCode _CreateIoVectorizedInfoFromIoPacket(VIR_Shader* pShader,
                                                       VIR_IO_VECTORIZABLE_PACKET* pIoPacket,
                                                       VIR_IO_VECTORIZED_INFO* pIoVectorizedInfo)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    gctSTRING           strNewName = gcvNULL;
    VIR_NameId          newNameId;
    VIR_SymId           newSymId = VIR_INVALID_ID;
    VIR_Type*           pNewSymType = gcvNULL;
    VIR_VirRegId        virRegId;
    VIR_Symbol*         pNewSym = gcvNULL;
    VIR_Symbol*         pVirRegSym = gcvNULL;
    VIR_Symbol*         pSoSym = gcvNULL;
    gctUINT             i, firstVirRegNo, virRegRange, firstMatchedXFBVaryingIdx;
    VIR_SymIdList*      pOrgTFBVaryingList = gcvNULL;
    VIR_ValueList*      pOrgTFBVaryingTempRegInfoList = gcvNULL;
    VIR_VarTempRegInfo  tfbVaryingTempRegInfo;

    /* Initialize vectorized info */
    pIoVectorizedInfo->pIoPacket = pIoPacket;
    pIoVectorizedInfo->vectorizedInfo.pVectorizedSym = gcvNULL;
    pIoVectorizedInfo->vectorizedInfo.ppVectorizedVirRegArray = gcvNULL;
    pIoVectorizedInfo->vectorizedInfo.virRegRange = 0;

    if (!_CheckIoPacketVectorizability(pShader, pIoPacket))
    {
        return VSC_ERR_NONE;
    }

    /* Add new vectorized sym name */
    strNewName = _GetVectorizedIoSymStrName(pShader, pIoPacket);
    errCode = VIR_Shader_AddString(pShader,strNewName, &newNameId);
    ON_ERROR(errCode, "Add vectorized IO string");

    /* Get type of new vectorized sym */
    pNewSymType = _GetVectorizedIoSymType(pShader, pIoPacket);
    if (pNewSymType == gcvNULL)
    {
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Get vectorized IO sym type");
    }

    /* Add new sym to table */
    errCode = VIR_Shader_AddSymbol(pShader,
                                   VIR_SYM_VARIABLE,
                                   newNameId,
                                   pNewSymType,
                                   (VIR_StorageClass)pIoPacket->pSymIo[0]->_storageClass,
                                   &newSymId);
    ON_ERROR(errCode, "Add vectorized IO sym");

    pNewSym = VIR_Shader_GetSymFromId(pShader, newSymId);
    virRegRange = VIR_Symbol_GetVirIoRegCount(pShader, pNewSym);
    firstVirRegNo = VIR_Shader_NewVirRegId(pShader, virRegRange);

    pIoVectorizedInfo->vectorizedInfo.ppVectorizedVirRegArray =
                                (VIR_Symbol**)vscMM_Alloc(&pShader->mempool, virRegRange*sizeof(VIR_Symbol*));
    pIoVectorizedInfo->vectorizedInfo.pVectorizedSym = pNewSym;
    pIoVectorizedInfo->vectorizedInfo.virRegRange = virRegRange;

    /* Set correct attributes for this new sym */
    VIR_Symbol_SetTyQualifier(pNewSym, pIoPacket->bOutput ? VIR_TYQUAL_NONE : VIR_TYQUAL_CONST);
    VIR_Symbol_SetLayoutQualifier(pNewSym, VIR_LAYQUAL_NONE);
    VIR_Symbol_SetLocation(pNewSym, pIoPacket->vectorizedLocation);
    VIR_Symbol_SetFirstSlot(pNewSym, NOT_ASSIGNED);
    VIR_Symbol_SetArraySlot(pNewSym, NOT_ASSIGNED);
    VIR_Symbol_SetHwFirstCompIndex(pNewSym, NOT_ASSIGNED);
    VIR_Symbol_SetPrecision(pNewSym, VIR_Symbol_GetPrecision(pIoPacket->pSymIo[0]));
    pNewSym->u2.tempIndex = firstVirRegNo;

    for (i = 0; i < pIoPacket->realCount; i ++)
    {
        pNewSym->flags |= pIoPacket->pSymIo[i]->flags;

        /* Mark original sym has been vectorized out */
        VIR_Symbol_SetFlag(pIoPacket->pSymIo[i], VIR_SYMFLAG_VECTORIZED_OUT);
    }

    /* Create virReg symbols for this new vectorized sym for outputs */
    for (i = 0; i < virRegRange; i ++)
    {
        errCode = VIR_Shader_AddSymbol(pShader,
                                       VIR_SYM_VIRREG,
                                       firstVirRegNo + i,
                                       VIR_Shader_GetTypeFromId(pShader, VIR_Type_GetBaseTypeId(pNewSymType)),
                                       VIR_STORAGE_UNKNOWN,
                                       &virRegId);
        ON_ERROR(errCode, "Add vir reg for vectorized io sym");

        pVirRegSym= VIR_Shader_GetSymFromId(pShader, virRegId);
        pVirRegSym->u2.variable = pNewSym;

        pIoVectorizedInfo->vectorizedInfo.ppVectorizedVirRegArray[i] = pVirRegSym;

        if (pIoPacket->bOutput)
        {
            if (VIR_Symbol_isOutput(pIoPacket->pSymIo[0]))
            {
                VIR_IdList_Add(&pShader->outputVregs, virRegId);
            }
            else if (VIR_Symbol_isPerPatchOutput(pIoPacket->pSymIo[0]))
            {
                VIR_IdList_Add(&pShader->perpatchOutputVregs, virRegId);
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }
        }
    }

    /* For tfb case, also update tfb variables */
    if (pIoPacket->bTFB)
    {
        gcmASSERT(pShader->transformFeedback.varyings);

        VIR_IdList_Init(&pShader->mempool,
                        MAX_SHADER_IO_NUM,
                        &pOrgTFBVaryingList);

        VIR_ValueList_Init(&pShader->mempool,
                           MAX_SHADER_IO_NUM,
                           sizeof(VIR_VarTempRegInfo),
                           &pOrgTFBVaryingTempRegInfoList);

        for (i = 0; i < VIR_IdList_Count(pShader->transformFeedback.varyings); i ++)
        {
            VIR_IdList_Add(pOrgTFBVaryingList,
                           VIR_IdList_GetId(pShader->transformFeedback.varyings, i));

            VIR_ValueList_Add(pOrgTFBVaryingTempRegInfoList,
                              VIR_ValueList_GetValue(pShader->transformFeedback.varRegInfos, i));
        }

        /* Reset */
        pShader->transformFeedback.varyings->count = 0;
        pShader->transformFeedback.varRegInfos->count = 0;

        firstMatchedXFBVaryingIdx = VIR_IdList_Count(pOrgTFBVaryingList);
        for (i = 0; i < VIR_IdList_Count(pOrgTFBVaryingList); i ++)
        {
            pSoSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pOrgTFBVaryingList, i));

            if (pSoSym == pIoPacket->pSymIo[0])
            {
                /* Only hit here once */
                gcmASSERT(firstMatchedXFBVaryingIdx == VIR_IdList_Count(pOrgTFBVaryingList));

                firstMatchedXFBVaryingIdx = i;
            }

            if (i - firstMatchedXFBVaryingIdx < pIoPacket->realCount)
            {
                /* Outputs for TFB related packed must be togeter and same seq in TFB varying list */

                gcmASSERT(VIR_Symbol_GetIndexingInfo(pShader, pSoSym).underlyingSym == pSoSym);
                gcmASSERT(pSoSym == pIoPacket->pSymIo[i - firstMatchedXFBVaryingIdx]);

                /* Use new vectorized sym now */
                if (i - firstMatchedXFBVaryingIdx == 0)
                {
                    tfbVaryingTempRegInfo.streamoutSize = 0;
                    tfbVaryingTempRegInfo.tempRegCount = virRegRange;
                    tfbVaryingTempRegInfo.tempRegTypes = gcvNULL;
                    tfbVaryingTempRegInfo.variable = newSymId;

                    VIR_IdList_Add(pShader->transformFeedback.varyings, newSymId);
                    VIR_ValueList_Add(pShader->transformFeedback.varRegInfos, (gctCHAR *)&tfbVaryingTempRegInfo);
                }
            }
            else
            {
                /* Add others */
                VIR_IdList_Add(pShader->transformFeedback.varyings,
                               VIR_IdList_GetId(pOrgTFBVaryingList, i));

                VIR_ValueList_Add(pShader->transformFeedback.varRegInfos,
                                  VIR_ValueList_GetValue(pOrgTFBVaryingTempRegInfoList, i));
            }
        }
    }

    /* For member of IOB, need update IOB */
    if (isSymIOBlockMember(pIoPacket->pSymIo[0]))
    {
        /* TODO */
    }

OnError:
    if (strNewName)
    {
        vscMM_Free(&pShader->mempool, strNewName);
    }

    if (errCode != VSC_ERR_NONE)
    {
        if (pIoVectorizedInfo->vectorizedInfo.ppVectorizedVirRegArray)
        {
            vscMM_Free(&pShader->mempool, pIoVectorizedInfo->vectorizedInfo.ppVectorizedVirRegArray);
            pIoVectorizedInfo->vectorizedInfo.ppVectorizedVirRegArray = gcvNULL;
        }
    }

    return errCode;
}

static gctBOOL _CheckSymOfOpndIsInIoVectorizedInfos(VIR_Shader* pShader,
                                                    VIR_IO_VECTORIZED_INFO* pIoVectorizedInfo,
                                                    gctUINT numOfVectorizedInfos,
                                                    VIR_Operand* pOpnd,
                                                    gctUINT* pChannelShift,
                                                    VIR_VirRegId* pNewSymId)
{
    gctUINT                   i, j, channelShift = 0;
    VIR_VirRegId              newSymId = VIR_INVALID_ID;
    gctBOOL                   bFoundSym = gcvFALSE;
    VIR_IO_VECTORIZED_INFO*   pVectorizedInfo;
    VIR_Symbol*               pOpndSym;

    pOpndSym = VIR_Operand_GetSymbol(pOpnd);

    for (i = 0; i < numOfVectorizedInfos; i ++)
    {
        pVectorizedInfo = &pIoVectorizedInfo[i];

        if (pVectorizedInfo->vectorizedInfo.pVectorizedSym == gcvNULL ||
            pVectorizedInfo->vectorizedInfo.ppVectorizedVirRegArray == gcvNULL)
        {
            continue;
        }

        channelShift = 0;
        for (j = 0; j < pVectorizedInfo->pIoPacket->realCount; j ++)
        {
            if (VIR_Symbol_isVreg(pOpndSym))
            {
                if (pOpndSym->u2.variable == pVectorizedInfo->pIoPacket->pSymIo[j])
                {
                    gcmASSERT(pVectorizedInfo->pIoPacket->bOutput);

                    newSymId = pVectorizedInfo->vectorizedInfo.ppVectorizedVirRegArray[pOpndSym->u1.vregIndex - pOpndSym->u2.variable->u2.tempIndex]->index;

                    bFoundSym = gcvTRUE;
                    break;
                }
            }
            else if (VIR_Symbol_isVariable(pOpndSym))
            {
                if (pOpndSym == pVectorizedInfo->pIoPacket->pSymIo[j])
                {
                    gcmASSERT(!pVectorizedInfo->pIoPacket->bOutput);

                    newSymId = pVectorizedInfo->vectorizedInfo.pVectorizedSym->index;

                    bFoundSym = gcvTRUE;
                    break;
                }
            }

            channelShift += VIR_GetTypeComponents(VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(pVectorizedInfo->pIoPacket->pSymIo[j])));
        }

        if (bFoundSym)
        {
            break;
        }
    }

    if (pChannelShift)
    {
        *pChannelShift = bFoundSym ? channelShift : 0;
    }

    if (pNewSymId)
    {
        *pNewSymId = newSymId;
    }

    return bFoundSym;
}

static VSC_ErrCode _InsertMOVForUnComponentWisedInst(VIR_Shader*       pShader,
                                                     VIR_Function*     Func,
                                                     VIR_Instruction*  Inst,
                                                     gctUINT           DstChannelShift,
                                                     VIR_VirRegId      DstNewSymId)
{
    VIR_Instruction*    newInst = gcvNULL;
    VIR_Operand*        origDest = VIR_Inst_GetDest(Inst);
    VIR_Enable          origEnable = VIR_Inst_GetEnable(Inst);
    VIR_Swizzle         prevSwizzle = VIR_SWIZZLE_INVALID;
    VIR_Swizzle         newSwizzle = VIR_SWIZZLE_X;
    gctINT              i, j;
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_Operand*        opnd;

    /* Insert a MOV after current instruction. */
    retValue = VIR_Function_AddInstructionAfter(Func,
                                                VIR_OP_MOV,
                                                VIR_Operand_GetType(VIR_Inst_GetDest(Inst)),
                                                Inst,
                                                &newInst);
    if (retValue != VSC_ERR_NONE) return retValue;

    /* Set the dest. */
    retValue = VIR_Inst_CopyDest(newInst,
                                 origDest,
                                 gcvFALSE);
    if (retValue != VSC_ERR_NONE) return retValue;

    opnd = VIR_Inst_GetDest(newInst);

    VIR_Operand_SetSymbol(opnd, Func, DstNewSymId);
    VIR_Operand_SetEnable(opnd, VIR_Operand_GetEnable(origDest) << DstChannelShift);

    /* Set the source. */
    retValue = VIR_Inst_CopySource(newInst,
                                   0,
                                   origDest,
                                   gcvFALSE);
    if (retValue != VSC_ERR_NONE) return retValue;

    /* Mapping the swizzle. */
    for (i = 0; i < VIR_CHANNEL_COUNT; i++)
    {
        if (origEnable & (1 << i))
        {
            VIR_Swizzle_SetChannel(newSwizzle, i, i);

            if (prevSwizzle == VIR_SWIZZLE_INVALID)
            {
                for (j = 0; j < i; j++)
                {
                    VIR_Swizzle_SetChannel(newSwizzle, j, i);
                }
            }
            prevSwizzle = (VIR_Swizzle)i;
        }
        else if (prevSwizzle != VIR_SWIZZLE_INVALID)
        {
            VIR_Swizzle_SetChannel(newSwizzle, i, prevSwizzle);
        }
    }

    if (i != VIR_CHANNEL_COUNT)
    {
        for (j = i; j < VIR_CHANNEL_COUNT; j++)
        {
            VIR_Swizzle_SetChannel(newSwizzle, j, prevSwizzle);
        }
    }

    /* Shift swizzle. */
    for (i = VIR_CHANNEL_COUNT - 1; i >= 0; i--)
    {
        if (i - DstChannelShift < 0)
        {
            break;
        }

        prevSwizzle = VIR_Swizzle_GetChannel(newSwizzle, i - DstChannelShift);
        VIR_Swizzle_SetChannel(newSwizzle, i, prevSwizzle);
    }

    /* Set the new swizzle.*/
    VIR_Operand_SetSwizzle(VIR_Inst_GetSource(newInst, 0), newSwizzle);

    return retValue;
}

static VSC_ErrCode _ChangeInstsByIoVectorizedInfos(VIR_Shader* pShader,
                                                   VIR_IO_VECTORIZED_INFO* pIoVectorizedInfo,
                                                   gctUINT numOfVectorizedInfos)
{
    VSC_ErrCode       errCode = VSC_ERR_NONE;
    VIR_FuncIterator  func_iter;
    VIR_FunctionNode* func_node;
    VIR_Operand*      opnd;
    VIR_Function*     func;
    VIR_InstIterator  inst_iter;
    VIR_Instruction*  inst;
    gctUINT           i, j, k, dstChannelShift, srcChannelShift;
    gctUINT           channelSwizzle, swizzle;
    VIR_VirRegId      newSymId;
    gctBOOL           bMovInserted;

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        func = func_node->function;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
        for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             inst != gcvNULL; inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            bMovInserted = gcvFALSE;

            /* Dst */
            opnd = VIR_Inst_GetDest(inst);
            dstChannelShift = 0;
            if (opnd != gcvNULL && VIR_Operand_GetOpKind(opnd) == VIR_OPND_SYMBOL)
            {
                if (_CheckSymOfOpndIsInIoVectorizedInfos(pShader,
                                                         pIoVectorizedInfo,
                                                         numOfVectorizedInfos,
                                                         opnd,
                                                         &dstChannelShift,
                                                         &newSymId))
                {
                    if (!VIR_OPCODE_isComponentwise(VIR_Inst_GetOpcode(inst)) &&
                        dstChannelShift > 0 &&
                        /* RA will handle ATTR_LD/ATTR_ST*/
                        VIR_Inst_GetOpcode(inst) != VIR_OP_ATTR_LD &&
                        VIR_Inst_GetOpcode(inst) != VIR_OP_ATTR_ST &&
                        /*
                        ** For STORE, dest is used as buffer for USC constrain,
                        ** so we don't need to insert a MOV for it.
                        */
                        VIR_Inst_GetOpcode(inst) != VIR_OP_STORE)
                    {
                        errCode = _InsertMOVForUnComponentWisedInst(pShader,
                                                                    func,
                                                                    inst,
                                                                    dstChannelShift,
                                                                    newSymId);
                        ON_ERROR(errCode, "Insert mov for uncomponentwised inst");

                        bMovInserted = gcvTRUE;
                    }
                    else
                    {
                        VIR_Operand_SetSymbol(opnd, func, newSymId);
                        VIR_Operand_SetEnable(opnd, VIR_Operand_GetEnable(opnd) << dstChannelShift);
                    }
                }
            }

            /* Srcs */
            for (i = 0; i < VIR_Inst_GetSrcNum(inst); ++i)
            {
                opnd = VIR_Inst_GetSource(inst, i);
                if (VIR_Operand_GetOpKind(opnd) == VIR_OPND_SYMBOL)
                {
                    if (_CheckSymOfOpndIsInIoVectorizedInfos(pShader,
                                                             pIoVectorizedInfo,
                                                             numOfVectorizedInfos,
                                                             opnd,
                                                             &srcChannelShift,
                                                             &newSymId))
                    {
                        VIR_Operand_SetSymbol(opnd, func, newSymId);

                        swizzle = VIR_Operand_GetSwizzle(opnd);

                        for (j = 0; j < VIR_CHANNEL_COUNT; j ++)
                        {
                            channelSwizzle = (swizzle >> j*2) & 0x3;
                            channelSwizzle += srcChannelShift;
                            swizzle &= ~(0x3 << j*2);
                            swizzle |= (channelSwizzle << j*2);
                        }

                        VIR_Operand_SetSwizzle(opnd, swizzle);
                    }
                }
                else if (VIR_Operand_GetOpKind(opnd) == VIR_OPND_TEXLDPARM)
                {
                    VIR_Operand_TexldModifier  *texldOperand = (VIR_Operand_TexldModifier *)opnd;

                    for (j = 0; j < VIR_TEXLDMODIFIER_COUNT; ++j)
                    {
                        opnd = texldOperand->tmodifier[j];

                        if (opnd != gcvNULL)
                        {
                            if (VIR_Operand_GetOpKind(opnd) == VIR_OPND_SYMBOL)
                            {
                                if (_CheckSymOfOpndIsInIoVectorizedInfos(pShader,
                                                                         pIoVectorizedInfo,
                                                                         numOfVectorizedInfos,
                                                                         opnd,
                                                                         &srcChannelShift,
                                                                         &newSymId))
                                {
                                    VIR_Operand_SetSymbol(opnd, func, newSymId);

                                    swizzle = VIR_Operand_GetSwizzle(opnd);

                                    for (k = 0; k < VIR_CHANNEL_COUNT; k ++)
                                    {
                                        channelSwizzle = (swizzle >> k*2) & 0x3;
                                        channelSwizzle += srcChannelShift;
                                        swizzle &= ~(0x3 << k*2);
                                        swizzle |= (channelSwizzle << k*2);
                                    }

                                    VIR_Operand_SetSwizzle(opnd, swizzle);
                                }
                            }
                        }
                    }

                    opnd = VIR_Inst_GetSource(inst, i);
                }

                if (VIR_Operand_GetOpKind(opnd) == VIR_OPND_SYMBOL ||
                    VIR_Operand_GetOpKind(opnd) == VIR_OPND_VIRREG)
                {
                    if ((VIR_OPCODE_isComponentwise(VIR_Inst_GetOpcode(inst)) ||
                         VIR_OPCODE_isSourceComponentwise(VIR_Inst_GetOpcode(inst), i)) &&
                         dstChannelShift > 0)
                    {
                        swizzle = VIR_Operand_GetSwizzle(opnd);
                        swizzle = (swizzle << dstChannelShift * 2) & 0xff;
                        VIR_Operand_SetSwizzle(opnd, swizzle);
                    }
                }
            }

            /* Point to the next instruction. */
            if (bMovInserted)
            {
                inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter);
            }
        }
    }

OnError:
    return errCode;
}

static VIR_OPND_VECTORIZE_TYPE _GetOpndVectorizeType(VIR_Shader* pShader,
                                                     VIR_Operand* pSeedOpnd,
                                                     VIR_Operand* pOpnd,
                                                     VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfo)
{
    /* Only consider sym/virreg/imm/const operands */
    if (VIR_Operand_GetOpKind(pSeedOpnd) != VIR_OPND_SYMBOL &&
        VIR_Operand_GetOpKind(pSeedOpnd) != VIR_OPND_VIRREG &&
        VIR_Operand_GetOpKind(pSeedOpnd) != VIR_OPND_IMMEDIATE &&
        VIR_Operand_GetOpKind(pSeedOpnd) != VIR_OPND_CONST)
    {
        return VIR_OPND_VECTORIZE_TYPE_UNKNOW;
    }

    /* Sym/virreg can only be vectorized with same operand kind */
    if ((VIR_Operand_GetOpKind(pSeedOpnd) == VIR_OPND_SYMBOL ||
         VIR_Operand_GetOpKind(pSeedOpnd) == VIR_OPND_VIRREG) &&
        !(VIR_Operand_GetOpKind(pOpnd) == VIR_OPND_SYMBOL ||
          VIR_Operand_GetOpKind(pOpnd) == VIR_OPND_VIRREG))
    {
        return VIR_OPND_VECTORIZE_TYPE_UNKNOW;
    }

    /* Imm/const can only be vectorized with same operand kind */
    if ((VIR_Operand_GetOpKind(pSeedOpnd) == VIR_OPND_IMMEDIATE ||
         VIR_Operand_GetOpKind(pSeedOpnd) == VIR_OPND_CONST) &&
        !(VIR_Operand_GetOpKind(pOpnd) == VIR_OPND_IMMEDIATE ||
          VIR_Operand_GetOpKind(pOpnd) == VIR_OPND_CONST))
    {
        return VIR_OPND_VECTORIZE_TYPE_UNKNOW;
    }

    pOpndVectorizedInfo->opndPair.pSeedOpnd = pSeedOpnd;
    pOpndVectorizedInfo->opndPair.pOpnd = pOpnd;

    if (VIR_Operand_GetOpKind(pSeedOpnd) == VIR_OPND_SYMBOL)
    {
        if (VIR_Operand_GetOpKind(pOpnd) == VIR_OPND_SYMBOL)
        {
            return VIR_OPND_VECTORIZE_TYPE_SYM_2_SYM;
        }
        else
        {
            return VIR_OPND_VECTORIZE_TYPE_VIRREG_2_SYM;
        }
    }
    else if (VIR_Operand_GetOpKind(pSeedOpnd) == VIR_OPND_VIRREG)
    {
        if (VIR_Operand_GetOpKind(pOpnd) == VIR_OPND_SYMBOL)
        {
            return VIR_OPND_VECTORIZE_TYPE_SYM_2_VIRREG;
        }
        else
        {
            return VIR_OPND_VECTORIZE_TYPE_VIRREG_2_VIRREG;
        }
    }
    else if (VIR_Operand_GetOpKind(pSeedOpnd) == VIR_OPND_IMMEDIATE)
    {
        if (VIR_Operand_GetOpKind(pOpnd) == VIR_OPND_CONST)
        {
            return VIR_OPND_VECTORIZE_TYPE_VIMM_2_SIMM;
        }
        else
        {
            return VIR_OPND_VECTORIZE_TYPE_SIMM_2_SIMM;
        }
    }
    else if (VIR_Operand_GetOpKind(pSeedOpnd) == VIR_OPND_CONST)
    {
        if (VIR_Operand_GetOpKind(pOpnd) == VIR_OPND_CONST)
        {
            return VIR_OPND_VECTORIZE_TYPE_VIMM_2_VIMM;
        }
        else
        {
            return VIR_OPND_VECTORIZE_TYPE_SIMM_2_VIMM;
        }
    }

    return VIR_OPND_VECTORIZE_TYPE_UNKNOW;
}

static gctUINT _GetVectorizedCompCount(gctUINT8 seedEnable, gctUINT8 enable)
{
    gctUINT8 vectorizedEnable = seedEnable | enable;

    if (vectorizedEnable & VIR_ENABLE_W)
    {
        return VIR_CHANNEL_COUNT;
    }
    else if (vectorizedEnable & VIR_ENABLE_Z)
    {
        return VIR_CHANNEL_COUNT - 1;
    }
    else if (vectorizedEnable & VIR_ENABLE_Y)
    {
        return VIR_CHANNEL_COUNT - 2;
    }
    else if (vectorizedEnable & VIR_ENABLE_X)
    {
        return VIR_CHANNEL_COUNT - 3;
    }

    return VIR_CHANNEL_COUNT;
}

static VIR_Swizzle _GetImmOperandSwizzleByCompCount(gctUINT compCount)
{
    switch (compCount)
    {
    case 1:
        return VIR_SWIZZLE_XXXX;
    case 2:
        return VIR_SWIZZLE_XYYY;
    case 3:
        return VIR_SWIZZLE_XYZZ;
    case 4:
    default:
        return VIR_SWIZZLE_XYZW;
    }
}

static VIR_OPND_VECTORIZABILITY_CHK_RES _Sym2SymOpndsVectorizabilityCheck(VIR_Shader* pShader,
                                                                          VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfo,
                                                                          gctBOOL bDst)
{
    VIR_Operand*    pSeedOpnd = pOpndVectorizedInfo->opndPair.pSeedOpnd;
    VIR_Operand*    pOpnd = pOpndVectorizedInfo->opndPair.pOpnd;
    VIR_OperandInfo seedOpInfo, opInfo;

    if (VIR_Operand_GetRelAddrMode(pSeedOpnd) == VIR_INDEXED_NONE &&
        VIR_Operand_GetRelAddrMode(pOpnd) == VIR_INDEXED_NONE)
    {
        if (VIR_Operand_GetSymbol(pSeedOpnd) == VIR_Operand_GetSymbol(pOpnd))
        {
            if (VIR_Operand_GetMatrixConstIndex(pSeedOpnd) + VIR_Operand_GetRelIndexing(pSeedOpnd) ==
                VIR_Operand_GetMatrixConstIndex(pOpnd) + VIR_Operand_GetRelIndexing(pOpnd))
            {
                return VIR_OPND_VECTORIZABILITY_CHK_RES_DIRECT_VECTORIZE;
            }
        }
        else if (vscVIR_CheckTwoSymsVectorizability(pShader, VIR_Operand_GetSymbol(pSeedOpnd), VIR_Operand_GetSymbol(pOpnd)))
        {
            VIR_Operand_GetOperandInfo(pOpndVectorizedInfo->instPair.pSeedInst, pSeedOpnd, &seedOpInfo);
            VIR_Operand_GetOperandInfo(pOpndVectorizedInfo->instPair.pInst, pOpnd, &opInfo);

            /* Skip io-vectorization because it will be done at linkage time */
            if (!seedOpInfo.isInput && !seedOpInfo.isOutput && !opInfo.isInput && !opInfo.isOutput)
            {
                if ((VIR_Operand_GetMatrixConstIndex(pSeedOpnd) + VIR_Operand_GetRelIndexing(pSeedOpnd)) == 0 &&
                    (VIR_Operand_GetMatrixConstIndex(pOpnd) + VIR_Operand_GetRelIndexing(pOpnd)) == 0)
                {
                    /* Currently, only consider dst sym2sym vectorization */
                    if (bDst && VIR_Inst_GetOpcode(pOpndVectorizedInfo->instPair.pInst) == VIR_OP_MOVA)
                    {
                        return VIR_OPND_VECTORIZABILITY_CHK_RES_NEED_SYM_OR_VIRREG_VECTORIZE;
                    }
                }
            }
        }
    }

    return VIR_OPND_VECTORIZABILITY_CHK_RES_DISCARD;
}

static VIR_OPND_VECTORIZABILITY_CHK_RES _Virreg2SymOpndsVectorizabilityCheck(VIR_Shader* pShader,
                                                                             VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfo,
                                                                             gctBOOL bDst)
{
    return VIR_OPND_VECTORIZABILITY_CHK_RES_DISCARD;
}

static VIR_OPND_VECTORIZABILITY_CHK_RES _Sym2VirregOpndsVectorizabilityCheck(VIR_Shader* pShader,
                                                                             VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfo,
                                                                             gctBOOL bDst)
{
    return VIR_OPND_VECTORIZABILITY_CHK_RES_DISCARD;
}

static VIR_OPND_VECTORIZABILITY_CHK_RES _Virreg2VirregOpndsVectorizabilityCheck(VIR_Shader* pShader,
                                                                                VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfo,
                                                                                gctBOOL bDst)
{
    VIR_Operand* pSeedOpnd = pOpndVectorizedInfo->opndPair.pSeedOpnd;
    VIR_Operand* pOpnd = pOpndVectorizedInfo->opndPair.pOpnd;

    if (VIR_Operand_GetSymbol(pSeedOpnd) == VIR_Operand_GetSymbol(pOpnd))
    {
        return VIR_OPND_VECTORIZABILITY_CHK_RES_DIRECT_VECTORIZE;
    }

    return VIR_OPND_VECTORIZABILITY_CHK_RES_DISCARD;
}

static VIR_OPND_VECTORIZABILITY_CHK_RES _Simm2SimmOpndsVectorizabilityCheck(VIR_Shader* pShader,
                                                                            VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfo,
                                                                            gctBOOL bDst)
{
    /* Scalar imm must be able to vectorized into another scalar imm */
    return VIR_OPND_VECTORIZABILITY_CHK_RES_DIRECT_VECTORIZE;
}

static VIR_OPND_VECTORIZABILITY_CHK_RES _Vimm2SimmOpndsVectorizabilityCheck(VIR_Shader* pShader,
                                                                            VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfo,
                                                                            gctBOOL bDst)
{
    /* Vector imm must be able to vectorized into another scalar imm */
    return VIR_OPND_VECTORIZABILITY_CHK_RES_DIRECT_VECTORIZE;
}

static VIR_OPND_VECTORIZABILITY_CHK_RES _Simm2VimmOpndsVectorizabilityCheck(VIR_Shader* pShader,
                                                                            VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfo,
                                                                            gctBOOL bDst)
{
    /* Scalar imm must be able to vectorized into another vector imm */
    return VIR_OPND_VECTORIZABILITY_CHK_RES_DIRECT_VECTORIZE;
}

static VIR_OPND_VECTORIZABILITY_CHK_RES _Vimm2VimmOpndsVectorizabilityCheck(VIR_Shader* pShader,
                                                                            VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfo,
                                                                            gctBOOL bDst)
{
    /* Vector imm must be able to vectorized into another vector imm */
    return VIR_OPND_VECTORIZABILITY_CHK_RES_DIRECT_VECTORIZE;
}

static gctBOOL _CommonOpndsVectorizabilityCheck(VIR_Shader* pShader,
                                                VIR_Operand* pSeedOpnd,
                                                VIR_Operand* pOpnd,
                                                gctBOOL bDst)
{
    if (VIR_Operand_GetModifier(pSeedOpnd) != VIR_Operand_GetModifier(pOpnd))
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

static VIR_OPND_VECTORIZABILITY_CHK_RES _CanOpndVectorizedToSeedOpnd(VIR_Shader* pShader,
                                                                     VIR_OPND_VECTORIZE_CALLBACKS* pOvCallbacks,
                                                                     VIR_Operand* pSeedOpnd,
                                                                     VIR_Operand* pOpnd,
                                                                     gctBOOL bDst,
                                                                     VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfo)
{
    VIR_OPND_VECTORIZABILITY_CHK_RES       ovcResult;
    VIR_OPND_VECTORIZE_TYPE                ovType = _GetOpndVectorizeType(pShader, pSeedOpnd, pOpnd, pOpndVectorizedInfo);

    if (_CommonOpndsVectorizabilityCheck(pShader, pSeedOpnd, pOpnd, bDst))
    {
        if (pOvCallbacks[ovType].pfnOvCheck == gcvNULL)
        {
            /* If corresponding operand vectorizability check function is not implemented, just skip this as discard */
            ovcResult = VIR_OPND_VECTORIZABILITY_CHK_RES_DISCARD;
        }
        else
        {
            /* Do the real operand vectorizability check */
            ovcResult = pOvCallbacks[ovType].pfnOvCheck(pShader, pOpndVectorizedInfo, bDst);
        }
    }
    else
    {
        ovcResult = VIR_OPND_VECTORIZABILITY_CHK_RES_DISCARD;
    }

    pOpndVectorizedInfo->ovcResult = ovcResult;
    pOpndVectorizedInfo->ovType = ovType;

    return ovcResult;
}

static gctBOOL _CanMoveInstToSeedInst(VIR_Shader* pShader,
                                      VIR_DEF_USAGE_INFO* pDuInfo,
                                      VIR_OPND_VECTORIZE_CALLBACKS* pOvCallbacks,
                                      VIR_Instruction* pSeedInst,
                                      VIR_Instruction* pInst)
{
    gctUINT                 i;
    VIR_GENERAL_DU_ITERATOR duIter;
    VIR_GENERAL_UD_ITERATOR udIter;
    VIR_DEF*                pDef;
    VIR_USAGE*              pUsage;
    VIR_DEF_KEY             defKey;
    VIR_Enable              defEnableMask;
    gctUINT                 firstRegNo, regNoRange;
    gctUINT8                channel;
    gctUINT                 defIdx;

    vscVIR_QueryRealWriteVirRegInfo(pDuInfo->baseTsDFA.baseDFA.pOwnerCG->pOwnerShader,
                                    pInst,
                                    &defEnableMask,
                                    gcvNULL,
                                    &firstRegNo,
                                    &regNoRange,
                                    gcvNULL,
                                    gcvNULL);

    gcmASSERT(regNoRange == 1);

    /* Check WAW and WAR */
    for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
    {
        if (!VSC_UTILS_TST_BIT(defEnableMask, channel))
        {
            continue;
        }

        defKey.pDefInst = VIR_ANY_DEF_INST;
        defKey.regNo = firstRegNo;
        defKey.channel = channel;
        defIdx = vscBT_HashSearch(&pDuInfo->defTable, &defKey);

        while (VIR_INVALID_DEF_INDEX != defIdx)
        {
            pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, defIdx);

            if (pDef->defKey.channel == channel)
            {
                /* WAW */
                if ((VIR_Inst_GetId(pDef->defKey.pDefInst) > VIR_Inst_GetId(pSeedInst)) &&
                    (VIR_Inst_GetId(pDef->defKey.pDefInst) < VIR_Inst_GetId(pInst)))
                {
                    return gcvFALSE;
                }

                vscVIR_InitGeneralDuIterator(&duIter,
                                             pDuInfo,
                                             pDef->defKey.pDefInst,
                                             firstRegNo,
                                             channel,
                                             gcvFALSE);

                /* WAR */
                for (pUsage = vscVIR_GeneralDuIterator_First(&duIter); pUsage != gcvNULL;
                     pUsage = vscVIR_GeneralDuIterator_Next(&duIter))
                {
                    if (pUsage->usageKey.pUsageInst == VIR_OUTPUT_USAGE_INST)
                    {
                        continue;
                    }

                    if ((VIR_Inst_GetId(pUsage->usageKey.pUsageInst) > VIR_Inst_GetId(pSeedInst)) &&
                        (VIR_Inst_GetId(pUsage->usageKey.pUsageInst) < VIR_Inst_GetId(pInst)))
                    {
                        return gcvFALSE;
                    }
                }
            }

            /* Get next def with same regNo */
            defIdx = pDef->nextDefIdxOfSameRegNo;
        }
    }

    /* Check RAW */
    for (i = 0; i < VIR_Inst_GetSrcNum(pInst); ++ i)
    {
        vscVIR_InitGeneralUdIterator(&udIter, pDuInfo, pInst, VIR_Inst_GetSource(pInst, i), gcvTRUE);

        for (pDef = vscVIR_GeneralUdIterator_First(&udIter); pDef != gcvNULL;
             pDef = vscVIR_GeneralUdIterator_Next(&udIter))
        {
            if ((VIR_Inst_GetId(pDef->defKey.pDefInst) >= VIR_Inst_GetId(pSeedInst)) &&
                (VIR_Inst_GetId(pDef->defKey.pDefInst) < VIR_Inst_GetId(pInst)))
            {
                return gcvFALSE;
            }
        }
    }

    return gcvTRUE;
}

static gctBOOL _CanMoveSeedInstToInst(VIR_Shader* pShader,
                                      VIR_DEF_USAGE_INFO* pDuInfo,
                                      VIR_OPND_VECTORIZE_CALLBACKS* pOvCallbacks,
                                      VIR_Instruction* pSeedInst,
                                      VIR_Instruction* pInst)
{
    gctUINT                 i;
    VIR_GENERAL_DU_ITERATOR duIter;
    VIR_DEF*                pDef;
    VIR_USAGE*              pUsage;
    VIR_DEF_KEY             defKey;
    VIR_Enable              defEnableMask;
    gctUINT                 firstRegNo, regNoRange;
    gctUINT8                channel;
    gctUINT                 defIdx;
    VIR_OperandInfo         operandInfo;

    vscVIR_QueryRealWriteVirRegInfo(pDuInfo->baseTsDFA.baseDFA.pOwnerCG->pOwnerShader,
                                    pSeedInst,
                                    &defEnableMask,
                                    gcvNULL,
                                    &firstRegNo,
                                    &regNoRange,
                                    gcvNULL,
                                    gcvNULL);

    gcmASSERT(regNoRange == 1);

    /* Check RAW */
    for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
    {
        if (!VSC_UTILS_TST_BIT(defEnableMask, channel))
        {
            continue;
        }

        vscVIR_InitGeneralDuIterator(&duIter,
                                     pDuInfo,
                                     pSeedInst,
                                     firstRegNo,
                                     channel,
                                     gcvTRUE);

        for (pUsage = vscVIR_GeneralDuIterator_First(&duIter); pUsage != gcvNULL;
             pUsage = vscVIR_GeneralDuIterator_Next(&duIter))
        {
            if (pUsage->usageKey.pUsageInst == VIR_OUTPUT_USAGE_INST)
            {
                continue;
            }

            if ((VIR_Inst_GetId(pUsage->usageKey.pUsageInst) > VIR_Inst_GetId(pSeedInst)) &&
                (VIR_Inst_GetId(pUsage->usageKey.pUsageInst) <= VIR_Inst_GetId(pInst)))
            {
                return gcvFALSE;
            }
        }
    }

    /* Check WAW */
    for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
    {
        if (!VSC_UTILS_TST_BIT(defEnableMask, channel))
        {
            continue;
        }

        defKey.pDefInst = VIR_ANY_DEF_INST;
        defKey.regNo = firstRegNo;
        defKey.channel = channel;
        defIdx = vscBT_HashSearch(&pDuInfo->defTable, &defKey);

        while (VIR_INVALID_DEF_INDEX != defIdx)
        {
            pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, defIdx);

            if (pDef->defKey.channel == channel)
            {
                if ((VIR_Inst_GetId(pDef->defKey.pDefInst) > VIR_Inst_GetId(pSeedInst)) &&
                    (VIR_Inst_GetId(pDef->defKey.pDefInst) < VIR_Inst_GetId(pInst)))
                {
                    return gcvFALSE;
                }
            }

            /* Get next def with same regNo */
            defIdx = pDef->nextDefIdxOfSameRegNo;
        }
    }

    /* Check WAR */
    for (i = 0; i < VIR_Inst_GetSrcNum(pSeedInst); ++ i)
    {
        VIR_Operand_GetOperandInfo(pSeedInst,
                                   VIR_Inst_GetSource(pSeedInst, i),
                                   &operandInfo);

        if (VIR_OpndInfo_Is_Virtual_Reg(&operandInfo))
        {
            firstRegNo = operandInfo.u1.virRegInfo.virReg;
            defEnableMask = VIR_Operand_GetRealUsedChannels(VIR_Inst_GetSource(pSeedInst, i), pSeedInst, gcvNULL);

            for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
            {
                if (!VSC_UTILS_TST_BIT(defEnableMask, channel))
                {
                    continue;
                }

                defKey.pDefInst = VIR_ANY_DEF_INST;
                defKey.regNo = firstRegNo;
                defKey.channel = channel;
                defIdx = vscBT_HashSearch(&pDuInfo->defTable, &defKey);

                while (VIR_INVALID_DEF_INDEX != defIdx)
                {
                    pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, defIdx);

                    if (pDef->defKey.channel == channel)
                    {
                        if ((VIR_Inst_GetId(pDef->defKey.pDefInst) > VIR_Inst_GetId(pSeedInst)) &&
                            (VIR_Inst_GetId(pDef->defKey.pDefInst) < VIR_Inst_GetId(pInst)))
                        {
                            return gcvFALSE;
                        }
                    }

                    /* Get next def with same regNo */
                    defIdx = pDef->nextDefIdxOfSameRegNo;
                }
            }
        }
    }

    return gcvTRUE;
}

static gctBOOL _CanInstVectorizeToSeedInst(VIR_Shader* pShader,
                                           VIR_DEF_USAGE_INFO* pDuInfo,
                                           VIR_OPND_VECTORIZE_CALLBACKS* pOvCallbacks,
                                           VIR_Instruction* pSeedInst,
                                           VIR_Instruction* pInst,
                                           VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfoArray,
                                           VIR_OPND_VECTORIZE_MODE* pOvMode)
{
    gctUINT                                i;

    /* Check opcode */
    if (VIR_Inst_GetOpcode(pSeedInst) != VIR_Inst_GetOpcode(pInst))
    {
        return gcvFALSE;
    }

    /* Check condOpcode */
    if (VIR_Inst_GetConditionOp(pSeedInst) != VIR_Inst_GetConditionOp(pInst))
    {
        return gcvFALSE;
    }

    /* Even base opcode is component-wised, condOpcode also may block component-wise */
    if (VIR_Inst_GetConditionOp(pSeedInst) == VIR_COP_ANYMSB ||
        VIR_Inst_GetConditionOp(pSeedInst) == VIR_COP_ALLMSB)
    {
        return gcvFALSE;
    }

    /* Conditional-write inst can not have enable overlap */
    if (VIR_OPCODE_CONDITIONAL_WRITE(VIR_Inst_GetOpcode(pSeedInst)))
    {
        if (VIR_Operand_GetEnable(VIR_Inst_GetDest(pSeedInst)) &
            VIR_Operand_GetEnable(VIR_Inst_GetDest(pInst)))
        {
            return gcvFALSE;
        }
    }

    /* Following insts must keep scalar */
    if (VIR_Inst_GetOpcode(pSeedInst) == VIR_OP_RCP ||
        VIR_Inst_GetOpcode(pSeedInst) == VIR_OP_RSQ ||
        VIR_Inst_GetOpcode(pSeedInst) == VIR_OP_EXP2 ||
        VIR_Inst_GetOpcode(pSeedInst) == VIR_OP_PRE_LOG2 ||
        VIR_Inst_GetOpcode(pSeedInst) == VIR_OP_SQRT ||
        VIR_Inst_GetOpcode(pSeedInst) == VIR_OP_SINPI ||
        VIR_Inst_GetOpcode(pSeedInst) == VIR_OP_COSPI ||
        VIR_Inst_GetOpcode(pSeedInst) == VIR_OP_DIV ||
        VIR_Inst_GetOpcode(pSeedInst) == VIR_OP_PRE_DIV)
    {
        return gcvFALSE;
    }

    /* Check dst */
    pOpndVectorizedInfoArray[0].instPair.pSeedInst = pSeedInst;
    pOpndVectorizedInfoArray[0].instPair.pInst = pInst;
    if (_CanOpndVectorizedToSeedOpnd(pShader,
                                     pOvCallbacks,
                                     VIR_Inst_GetDest(pSeedInst),
                                     VIR_Inst_GetDest(pInst),
                                     gcvTRUE,
                                     &pOpndVectorizedInfoArray[0]) == VIR_OPND_VECTORIZABILITY_CHK_RES_DISCARD)
    {
        return gcvFALSE;
    }

    /* Check srcs */
    for (i = 0; i < VIR_Inst_GetSrcNum(pSeedInst); ++ i)
    {
        pOpndVectorizedInfoArray[i + 1].instPair.pSeedInst = pSeedInst;
        pOpndVectorizedInfoArray[i + 1].instPair.pInst = pInst;

        if (_CanOpndVectorizedToSeedOpnd(pShader,
                                         pOvCallbacks,
                                         VIR_Inst_GetSource(pSeedInst, i),
                                         VIR_Inst_GetSource(pInst, i),
                                         gcvFALSE,
                                         &pOpndVectorizedInfoArray[i + 1]) == VIR_OPND_VECTORIZABILITY_CHK_RES_DISCARD)
        {
            return gcvFALSE;
        }
    }

    /* Check data dependencies */
    if (_CanMoveInstToSeedInst(pShader,
                               pDuInfo,
                               pOvCallbacks,
                               pSeedInst,
                               pInst))
    {
        *pOvMode = VIR_OPND_VECTORIZE_MODE_TO_SEED_INST;
    }
    else if (_CanMoveSeedInstToInst(pShader,
                                    pDuInfo,
                                    pOvCallbacks,
                                    pSeedInst,
                                    pInst))
    {
        *pOvMode = VIR_OPND_VECTORIZE_MODE_FROM_SEED_INST;
    }
    else
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

static void _VectorizeOpndsBasedOnOrgSymOrVirreg(VIR_Shader* pShader,
                                                 VIR_BASIC_BLOCK* pBasicBlock,
                                                 VIR_DEF_USAGE_INFO* pDuInfo,
                                                 VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfo,
                                                 gctUINT8 orgSeedEnable,
                                                 gctUINT8 orgEnable,
                                                 VIR_Operand* pSeedOpnd,
                                                 VIR_Operand* pOpnd,
                                                 gctBOOL bDst)
{
    gctUINT                 i, channelSwizzle, seedSwizzle, swizzle, resultSwizzle = 0;
    gctUINT8                seedEnable, enable, deltaEnable, channel;
    gctUINT                 firstRegNo, regNoRange;
    VIR_Enable              defEnableMask;
    VIR_DEF_FLAGS           defFlags;
    VIR_GENERAL_DU_ITERATOR duIter;
    VIR_GENERAL_UD_ITERATOR udIter;
    VIR_USAGE*              pUsage;
    VIR_DEF*                pDef;
    VIR_OperandInfo         operandInfo;
    VIR_TypeId              typeId, componentTypeId;

    if (bDst)
    {
        /* Set new enable for dst of seed inst */
        seedEnable = VIR_Operand_GetEnable(pSeedOpnd);
        enable = VIR_Operand_GetEnable(pOpnd);
        VIR_Operand_SetEnable(pSeedOpnd, (seedEnable | enable));

        /* Set new operand type. */
        componentTypeId = VIR_GetTypeComponentType(VIR_Operand_GetType(pSeedOpnd));
        typeId = VIR_TypeId_ComposeNonOpaqueType(componentTypeId,
                                                 VIR_Enable_Channel_Count(seedEnable | enable),
                                                 1);
        VIR_Operand_SetType(pSeedOpnd, typeId);

        vscVIR_QueryRealWriteVirRegInfo(pDuInfo->baseTsDFA.baseDFA.pOwnerCG->pOwnerShader,
                                        pOpndVectorizedInfo->instPair.pSeedInst,
                                        &defEnableMask,
                                        gcvNULL,
                                        &firstRegNo,
                                        &regNoRange,
                                        &defFlags,
                                        gcvNULL);

        gcmASSERT(regNoRange == 1);

        /* Add defs for seed inst */
        deltaEnable = (~seedEnable & enable);
        if (deltaEnable != VIR_ENABLE_NONE)
        {
            vscVIR_AddNewDef(pDuInfo,
                             pOpndVectorizedInfo->instPair.pSeedInst,
                             firstRegNo,
                             regNoRange,
                             deltaEnable,
                             VIR_HALF_CHANNEL_MASK_FULL,
                             defFlags.bIsInput,
                             defFlags.bIsOutput,
                             gcvNULL
                             );
        }

        /* Copy all usages of def of inst to def of seed inst, and delete corresponding usages for def of inst */
        for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
        {
            if (!VSC_UTILS_TST_BIT(enable, channel))
            {
                continue;
            }

            vscVIR_InitGeneralDuIterator(&duIter,
                                         pDuInfo,
                                         pOpndVectorizedInfo->instPair.pInst,
                                         firstRegNo,
                                         channel,
                                         gcvFALSE);

            for (pUsage = vscVIR_GeneralDuIterator_First(&duIter); pUsage != gcvNULL;
                 pUsage = vscVIR_GeneralDuIterator_Next(&duIter))
            {
                vscVIR_AddNewUsageToDef(pDuInfo,
                                        pOpndVectorizedInfo->instPair.pSeedInst,
                                        pUsage->usageKey.pUsageInst,
                                        pUsage->usageKey.pOperand,
                                        firstRegNo,
                                        regNoRange,
                                        (1 << channel),
                                        VIR_HALF_CHANNEL_MASK_FULL,
                                        gcvNULL);

                vscVIR_DeleteUsage(pDuInfo,
                                   pOpndVectorizedInfo->instPair.pInst,
                                   pUsage->usageKey.pUsageInst,
                                   pUsage->usageKey.pOperand,
                                   firstRegNo,
                                   regNoRange,
                                   (1 << channel),
                                   VIR_HALF_CHANNEL_MASK_FULL,
                                   gcvNULL);
            }
        }

        /* Delete defs that have been derived to seed inst */
        vscVIR_DeleteDef(pDuInfo,
                         pOpndVectorizedInfo->instPair.pInst,
                         firstRegNo,
                         regNoRange,
                         enable,
                         VIR_HALF_CHANNEL_MASK_FULL,
                         gcvNULL);
    }
    else
    {
        seedSwizzle = VIR_Operand_GetSwizzle(pSeedOpnd);
        swizzle = VIR_Operand_GetSwizzle(pOpnd);

        for (i = 0; i < VIR_CHANNEL_COUNT; i ++)
        {
            if (orgEnable & (1 << i))
            {
                channelSwizzle = (swizzle >> i*2) & 0x3;
            }
            else
            {
                channelSwizzle = (seedSwizzle >> i*2) & 0x3;
            }

            resultSwizzle |= (channelSwizzle << i*2);
        }

        /* Set new swizzle for opnd of seed inst */
        VIR_Operand_SetSwizzle(pSeedOpnd, resultSwizzle);

        vscVIR_InitGeneralUdIterator(&udIter, pDuInfo, pOpndVectorizedInfo->instPair.pInst, pOpnd, gcvFALSE);

        VIR_Operand_GetOperandInfo(pOpndVectorizedInfo->instPair.pInst,
                                   pOpnd,
                                   &operandInfo);

        /* Copy src operand usage from inst to seed inst, also delete original usage for inst */
        for (pDef = vscVIR_GeneralUdIterator_First(&udIter); pDef != gcvNULL;
             pDef = vscVIR_GeneralUdIterator_Next(&udIter))
        {
            vscVIR_AddNewUsageToDef(pDuInfo,
                                    pDef->defKey.pDefInst,
                                    pOpndVectorizedInfo->instPair.pSeedInst,
                                    pSeedOpnd,
                                    operandInfo.u1.virRegInfo.virReg,
                                    1,
                                    (1 << pDef->defKey.channel),
                                    VIR_HALF_CHANNEL_MASK_FULL,
                                    gcvNULL);
        }

        vscVIR_DeleteUsage(pDuInfo,
                           VIR_ANY_DEF_INST,
                           pOpndVectorizedInfo->instPair.pInst,
                           pOpnd,
                           operandInfo.u1.virRegInfo.virReg,
                           1,
                           VIR_Swizzle_2_Enable(swizzle),
                           VIR_HALF_CHANNEL_MASK_FULL,
                           gcvNULL);
    }
}

static VSC_ErrCode _VectorizeSym2SymOpnds(VIR_Shader* pShader,
                                          VIR_BASIC_BLOCK* pBasicBlock,
                                          VIR_DEF_USAGE_INFO* pDuInfo,
                                          VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfo,
                                          gctUINT8 orgSeedEnable,
                                          gctUINT8 orgEnable,
                                          gctBOOL bDst)
{
    VIR_Operand* pSeedOpnd = pOpndVectorizedInfo->opndPair.pSeedOpnd;
    VIR_Operand* pOpnd = pOpndVectorizedInfo->opndPair.pOpnd;

    if (pOpndVectorizedInfo->vectorizedInfo.pVectorizedSym)
    {
        /* Use new vectorized sym or virreg */
    }
    else
    {
        /* Use original sym or virreg */
        _VectorizeOpndsBasedOnOrgSymOrVirreg(pShader, pBasicBlock, pDuInfo, pOpndVectorizedInfo,
                                             orgSeedEnable, orgEnable, pSeedOpnd, pOpnd,
                                             bDst);
    }

    return VSC_ERR_NONE;
}

static VSC_ErrCode _VectorizeVirreg2SymOpnds(VIR_Shader* pShader,
                                             VIR_BASIC_BLOCK* pBasicBlock,
                                             VIR_DEF_USAGE_INFO* pDuInfo,
                                             VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfo,
                                             gctUINT8 orgSeedEnable,
                                             gctUINT8 orgEnable,
                                             gctBOOL bDst)
{
    gcmASSERT(pOpndVectorizedInfo->vectorizedInfo.pVectorizedSym);

    return VSC_ERR_NONE;
}

static VSC_ErrCode _VectorizeSym2VirregOpnds(VIR_Shader* pShader,
                                             VIR_BASIC_BLOCK* pBasicBlock,
                                             VIR_DEF_USAGE_INFO* pDuInfo,
                                             VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfo,
                                             gctUINT8 orgSeedEnable,
                                             gctUINT8 orgEnable,
                                             gctBOOL bDst)
{
    gcmASSERT(pOpndVectorizedInfo->vectorizedInfo.pVectorizedSym);

    return VSC_ERR_NONE;
}

static VSC_ErrCode _VectorizeVirreg2VirregOpnds(VIR_Shader* pShader,
                                                VIR_BASIC_BLOCK* pBasicBlock,
                                                VIR_DEF_USAGE_INFO* pDuInfo,
                                                VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfo,
                                                gctUINT8 orgSeedEnable,
                                                gctUINT8 orgEnable,
                                                gctBOOL bDst)
{
    VIR_Operand* pSeedOpnd = pOpndVectorizedInfo->opndPair.pSeedOpnd;
    VIR_Operand* pOpnd = pOpndVectorizedInfo->opndPair.pOpnd;

    if (pOpndVectorizedInfo->vectorizedInfo.pVectorizedSym)
    {
        /* Use new vectorized sym or virreg */
    }
    else
    {
        /* Use original sym or virreg */
        _VectorizeOpndsBasedOnOrgSymOrVirreg(pShader, pBasicBlock, pDuInfo, pOpndVectorizedInfo,
                                             orgSeedEnable, orgEnable, pSeedOpnd, pOpnd,
                                             bDst);
    }

    return VSC_ERR_NONE;
}

static VSC_ErrCode _VectorizeSimm2SimmOpnds(VIR_Shader* pShader,
                                            VIR_BASIC_BLOCK* pBasicBlock,
                                            VIR_DEF_USAGE_INFO* pDuInfo,
                                            VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfo,
                                            gctUINT8 orgSeedEnable,
                                            gctUINT8 orgEnable,
                                            gctBOOL bDst)
{
    VIR_Operand*  pSeedOpnd = pOpndVectorizedInfo->opndPair.pSeedOpnd;
    VIR_Operand*  pOpnd = pOpndVectorizedInfo->opndPair.pOpnd;
    VIR_TypeId    vConstType = VIR_TYPE_UNKNOWN;
    VIR_ConstVal  vConstValue;
    VIR_ConstId   vConstId;
    gctUINT       i, compCount = _GetVectorizedCompCount(orgSeedEnable, orgEnable);

    gcmASSERT(!bDst);

    memset(&vConstValue, 0, sizeof(VIR_ConstVal));

    vConstType = VIR_TypeId_ComposeNonOpaqueArrayedType(pShader,
                                                        VIR_Operand_GetType(pSeedOpnd),
                                                        compCount,
                                                        1,
                                                        -1);

    for (i = 0; i < VIR_CHANNEL_COUNT; i ++)
    {
        vConstValue.vecVal.u32Value[i] = pSeedOpnd->u1.uConst;

        if (orgEnable & (1 << i))
        {
            vConstValue.vecVal.u32Value[i] = pOpnd->u1.uConst;
        }
    }

    VIR_Shader_AddConstant(pShader, vConstType, &vConstValue, &vConstId);
    VIR_Operand_SetConstId(pSeedOpnd, vConstId);
    VIR_Operand_SetOpKind(pSeedOpnd, VIR_OPND_CONST);
    VIR_Operand_SetType(pSeedOpnd, vConstType);
    VIR_Operand_SetSwizzle(pSeedOpnd, _GetImmOperandSwizzleByCompCount(compCount));

    return VSC_ERR_NONE;
}

static VSC_ErrCode _VectorizeVimm2SimmOpnds(VIR_Shader* pShader,
                                            VIR_BASIC_BLOCK* pBasicBlock,
                                            VIR_DEF_USAGE_INFO* pDuInfo,
                                            VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfo,
                                            gctUINT8 orgSeedEnable,
                                            gctUINT8 orgEnable,
                                            gctBOOL bDst)
{
    VIR_Operand*  pSeedOpnd = pOpndVectorizedInfo->opndPair.pSeedOpnd;
    VIR_Operand*  pOpnd = pOpndVectorizedInfo->opndPair.pOpnd;
    VIR_TypeId    vConstType = VIR_TYPE_UNKNOWN;
    VIR_ConstVal  vConstValue;
    VIR_Const*    pOrgConstValue;
    VIR_ConstId   vConstId;
    gctUINT       i, compCount = _GetVectorizedCompCount(orgSeedEnable, orgEnable);

    gcmASSERT(!bDst);

    memset(&vConstValue, 0, sizeof(VIR_ConstVal));

    vConstType = VIR_TypeId_ComposeNonOpaqueArrayedType(pShader,
                                                        VIR_Operand_GetType(pSeedOpnd),
                                                        compCount,
                                                        1,
                                                        -1);

    pOrgConstValue = (VIR_Const*)VIR_GetSymFromId(&pShader->constTable, pOpnd->u1.constId);

    for (i = 0; i < VIR_CHANNEL_COUNT; i ++)
    {
        vConstValue.vecVal.u32Value[i] = pSeedOpnd->u1.uConst;

        if (orgEnable & (1 << i))
        {
            vConstValue.vecVal.u32Value[i] = pOrgConstValue->value.vecVal.u32Value[i];
        }
    }

    VIR_Shader_AddConstant(pShader, vConstType, &vConstValue, &vConstId);
    VIR_Operand_SetConstId(pSeedOpnd, vConstId);
    VIR_Operand_SetOpKind(pSeedOpnd, VIR_OPND_CONST);
    VIR_Operand_SetType(pSeedOpnd, vConstType);
    VIR_Operand_SetSwizzle(pSeedOpnd, _GetImmOperandSwizzleByCompCount(compCount));

    return VSC_ERR_NONE;
}

static VSC_ErrCode _VectorizeSimm2VimmOpnds(VIR_Shader* pShader,
                                            VIR_BASIC_BLOCK* pBasicBlock,
                                            VIR_DEF_USAGE_INFO* pDuInfo,
                                            VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfo,
                                            gctUINT8 orgSeedEnable,
                                            gctUINT8 orgEnable,
                                            gctBOOL bDst)
{
    VIR_Operand*  pSeedOpnd = pOpndVectorizedInfo->opndPair.pSeedOpnd;
    VIR_Operand*  pOpnd = pOpndVectorizedInfo->opndPair.pOpnd;
    VIR_TypeId    vConstType = VIR_TYPE_UNKNOWN;
    VIR_ConstVal  vConstValue;
    VIR_Const*    pOrgSeedConstValue;
    VIR_ConstId   vConstId;
    gctUINT       i, compCount = _GetVectorizedCompCount(orgSeedEnable, orgEnable);

    gcmASSERT(!bDst);

    memset(&vConstValue, 0, sizeof(VIR_ConstVal));

    vConstType = VIR_TypeId_ComposeNonOpaqueArrayedType(pShader,
                                                        VIR_Operand_GetType(pOpnd),
                                                        compCount,
                                                        1,
                                                        -1);

    pOrgSeedConstValue = (VIR_Const*)VIR_GetSymFromId(&pShader->constTable, pSeedOpnd->u1.constId);

    for (i = 0; i < VIR_CHANNEL_COUNT; i ++)
    {
        vConstValue.vecVal.u32Value[i] = pOrgSeedConstValue->value.vecVal.u32Value[i];

        if (orgEnable & (1 << i))
        {
            vConstValue.vecVal.u32Value[i] = pOpnd->u1.uConst;
        }
    }

    VIR_Shader_AddConstant(pShader, vConstType, &vConstValue, &vConstId);
    VIR_Operand_SetConstId(pSeedOpnd, vConstId);
    VIR_Operand_SetOpKind(pSeedOpnd, VIR_OPND_CONST);
    VIR_Operand_SetType(pSeedOpnd, vConstType);
    VIR_Operand_SetSwizzle(pSeedOpnd, _GetImmOperandSwizzleByCompCount(compCount));

    return VSC_ERR_NONE;
}

static VSC_ErrCode _VectorizeVimm2VimmOpnds(VIR_Shader* pShader,
                                            VIR_BASIC_BLOCK* pBasicBlock,
                                            VIR_DEF_USAGE_INFO* pDuInfo,
                                            VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfo,
                                            gctUINT8 orgSeedEnable,
                                            gctUINT8 orgEnable,
                                            gctBOOL bDst)
{
    VIR_Operand*  pSeedOpnd = pOpndVectorizedInfo->opndPair.pSeedOpnd;
    VIR_Operand*  pOpnd = pOpndVectorizedInfo->opndPair.pOpnd;
    VIR_TypeId    vConstType = VIR_TYPE_UNKNOWN;
    VIR_ConstVal  vConstValue;
    VIR_Const*    pOrgConstValue;
    VIR_Const*    pOrgSeedConstValue;
    VIR_ConstId   vConstId;
    gctUINT       i, compCount = _GetVectorizedCompCount(orgSeedEnable, orgEnable);

    gcmASSERT(!bDst);

    memset(&vConstValue, 0, sizeof(VIR_ConstVal));

    vConstType = VIR_TypeId_ComposeNonOpaqueArrayedType(pShader,
                                                        VIR_GetTypeComponentType(VIR_Operand_GetType(pOpnd)),
                                                        compCount,
                                                        1,
                                                        -1);

    pOrgSeedConstValue = (VIR_Const*)VIR_GetSymFromId(&pShader->constTable, pSeedOpnd->u1.constId);
    pOrgConstValue = (VIR_Const*)VIR_GetSymFromId(&pShader->constTable, pOpnd->u1.constId);

    for (i = 0; i < VIR_CHANNEL_COUNT; i ++)
    {
        vConstValue.vecVal.u32Value[i] = pOrgSeedConstValue->value.vecVal.u32Value[i];

        if (orgEnable & (1 << i))
        {
            vConstValue.vecVal.u32Value[i] = pOrgConstValue->value.vecVal.u32Value[i];
        }
    }

    VIR_Shader_AddConstant(pShader, vConstType, &vConstValue, &vConstId);
    VIR_Operand_SetConstId(pSeedOpnd, vConstId);
    VIR_Operand_SetOpKind(pSeedOpnd, VIR_OPND_CONST);
    VIR_Operand_SetType(pSeedOpnd, vConstType);
    VIR_Operand_SetSwizzle(pSeedOpnd, _GetImmOperandSwizzleByCompCount(compCount));

    return VSC_ERR_NONE;
}

static VSC_ErrCode _VectorizeOpndToSeedOpnd(VIR_Shader* pShader,
                                            VIR_BASIC_BLOCK* pBasicBlock,
                                            VIR_DEF_USAGE_INFO* pDuInfo,
                                            VIR_OPND_VECTORIZE_CALLBACKS* pOvCallbacks,
                                            VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfo,
                                            gctUINT8 orgSeedEnable,
                                            gctUINT8 orgEnable,
                                            gctBOOL bDst)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;

    if (pOvCallbacks[pOpndVectorizedInfo->ovType].pfnOvVectorize != gcvNULL)
    {
        gcmASSERT(pOpndVectorizedInfo->ovcResult == VIR_OPND_VECTORIZABILITY_CHK_RES_DIRECT_VECTORIZE);

        errCode = pOvCallbacks[pOpndVectorizedInfo->ovType].pfnOvVectorize(pShader, pBasicBlock, pDuInfo,
                                                                           pOpndVectorizedInfo, orgSeedEnable,
                                                                           orgEnable, bDst);
    }

    return errCode;
}

static VSC_ErrCode _VectorizeSym2Sym(VIR_Shader* pShader,
                                     VIR_DEF_USAGE_INFO* pDuInfo,
                                     VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfo,
                                     gctUINT8 redundantEnable,
                                     gctUINT* pSeedChlMappingArray,
                                     gctBOOL bDst,
                                     gctBOOL* pVectorizeSucc)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_Instruction*           pInst = pOpndVectorizedInfo->instPair.pInst;
    VIR_Instruction*           pSeedInst = pOpndVectorizedInfo->instPair.pSeedInst;
    VIR_Operand*               pInstDst = VIR_Inst_GetDest(pInst);
    VIR_Operand*               pSeedInstDst = VIR_Inst_GetDest(pSeedInst);
    VIR_Enable                 defEnableMask, orgInstEnable = VIR_Operand_GetEnable(pInstDst);
    VIR_DEF_FLAGS              instDefFlags, seedInstDefFlags;
    gctUINT                    firstRegNo, firstSeedRegNo, regNoRange, seedRegNoRange;
    gctUINT                    deltaEnable, freeSeedInstEnable = 0, newInstEnable = 0, unRedundantEnable = 0;
    gctUINT                    finalChannelCount = 0, i;
    gctUINT                    channelSwizzle, swizzle, newSwizzle;
    gctUINT8                   channel, channel1;
    VIR_GENERAL_DU_ITERATOR    duIter;
    VIR_USAGE*                 pUsage;
    VIR_DEF_KEY                defKey;
    VIR_DEF*                   pDef;
    gctUINT                    defIdx;
    VIR_Instruction*           pUsageInst;
    VIR_Operand*               pUsageOpnd;
    VIR_Operand*               pSrcOpnd;
    VIR_TypeId                 typeId, componentTypeId;
    gctUINT                    urSeedChlMappingArray[VIR_CHANNEL_NUM] = {NOT_ASSIGNED, NOT_ASSIGNED,
                                                                         NOT_ASSIGNED, NOT_ASSIGNED};

    *pVectorizeSucc = gcvFALSE;

    /* Currently, we dont consider src sym-vectorization */
    if (!bDst)
    {
        return VSC_ERR_NONE;
    }

    /* Don't consider the case of multi-defs usage */
    if (!vscVIR_IsUniqueDefInstOfUsagesInItsDUChain(pDuInfo,
                                                    pInst,
                                                    gcvNULL, gcvNULL))
    {
        return VSC_ERR_NONE;
    }

    /* Check whether 2 syms can be merged without channel overflowed, if overflowed,
       just skip this vectorization */
    deltaEnable = (VIR_Operand_GetEnable(pInstDst) & ~redundantEnable);
    for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
    {
        if (VSC_UTILS_TST_BIT(deltaEnable, channel))
        {
            finalChannelCount ++;
        }

        if (VSC_UTILS_TST_BIT(VIR_Operand_GetEnable(pSeedInstDst), channel))
        {
            finalChannelCount ++;
        }
        else
        {
            VSC_UTILS_SET_BIT(freeSeedInstEnable, channel);
        }
    }
    if (finalChannelCount > VIR_CHANNEL_COUNT)
    {
        return VSC_ERR_NONE;
    }

    vscVIR_QueryRealWriteVirRegInfo(pDuInfo->baseTsDFA.baseDFA.pOwnerCG->pOwnerShader,
                                    pInst,
                                    &defEnableMask,
                                    gcvNULL,
                                    &firstRegNo,
                                    &regNoRange,
                                    &instDefFlags,
                                    gcvNULL);

    gcmASSERT(regNoRange == 1);

    vscVIR_QueryRealWriteVirRegInfo(pDuInfo->baseTsDFA.baseDFA.pOwnerCG->pOwnerShader,
                                    pSeedInst,
                                    &defEnableMask,
                                    gcvNULL,
                                    &firstSeedRegNo,
                                    &seedRegNoRange,
                                    &seedInstDefFlags,
                                    gcvNULL);

    gcmASSERT(seedRegNoRange == 1);

    gcmASSERT(firstSeedRegNo != firstRegNo);

    /* Currently, don't consider the case that mapped seed-inst channel of redundant channel
       has def other than seed-inst.
       TODO: fully rename */
    for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
    {
        if (!VSC_UTILS_TST_BIT(redundantEnable, channel))
        {
            continue;
        }

        defKey.pDefInst = VIR_ANY_DEF_INST;
        defKey.regNo = firstSeedRegNo;
        defKey.channel = (gctUINT8)pSeedChlMappingArray[channel];

        defIdx = vscBT_HashSearch(&pDuInfo->defTable, &defKey);
        while (VIR_INVALID_DEF_INDEX != defIdx)
        {
            pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, defIdx);

            if (pDef->defKey.channel == (gctUINT8)pSeedChlMappingArray[channel] &&
                pDef->defKey.pDefInst != pSeedInst)
            {
                return VSC_ERR_NONE;
            }

            /* Get next def with same regNo */
            defIdx = pDef->nextDefIdxOfSameRegNo;
        }
    }

    /* Get new-enable of new dst of inst */
    for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
    {
        if (!VSC_UTILS_TST_BIT(freeSeedInstEnable, channel))
        {
            continue;
        }

        defKey.pDefInst = VIR_ANY_DEF_INST;
        defKey.regNo = firstSeedRegNo;
        defKey.channel = channel;

        /* Currently, don't consider the case that one of free-channel has been def'ed by
           other inst.
           TODO: fully rename */
        defIdx = vscBT_HashSearch(&pDuInfo->defTable, &defKey);
        if (VIR_INVALID_DEF_INDEX != defIdx)
        {
            continue;
        }

        for (channel1 = VIR_CHANNEL_X; channel1 < VIR_CHANNEL_NUM; channel1 ++)
        {
            if (!VSC_UTILS_TST_BIT(deltaEnable, channel1) ||
                urSeedChlMappingArray[channel1] != NOT_ASSIGNED)
            {
                continue;
            }

            urSeedChlMappingArray[channel1] = channel;
            VSC_UTILS_SET_BIT(newInstEnable, channel);
            VSC_UTILS_SET_BIT(unRedundantEnable, channel1);

            break;
        }
    }

    /* No new-enable can be used, just return */
    if (newInstEnable == 0)
    {
        return VSC_ERR_NONE;
    }

    /* Change sym of dst of inst to sym of dst of seed-inst */
    VIR_Operand_SetOpKind(pInstDst, VIR_Operand_GetOpKind(pSeedInstDst));
    VIR_Operand_SetSym(pInstDst, VIR_Operand_GetSymbol(pSeedInstDst));

    /* Set new-enable to new dst of inst */
    VIR_Operand_SetEnable(pInstDst, newInstEnable);

    /* Set new operand type. */
    componentTypeId = VIR_GetTypeComponentType(VIR_Operand_GetType(pSeedInstDst));
    typeId = VIR_TypeId_ComposeNonOpaqueType(componentTypeId,
                                             VIR_Enable_Channel_Count(VIR_Operand_GetEnable(pSeedInstDst) | newInstEnable),
                                             1);
    VIR_Operand_SetType(pSeedInstDst, typeId);
    VIR_Operand_SetType(pInstDst, typeId);

    /* Add new defs related to new-enable */
    vscVIR_AddNewDef(pDuInfo,
                     pInst,
                     firstSeedRegNo,
                     seedRegNoRange,
                     newInstEnable,
                     VIR_HALF_CHANNEL_MASK_FULL,
                     seedInstDefFlags.bIsInput,
                     seedInstDefFlags.bIsOutput,
                     gcvNULL
                     );

    /* Change swizzle of srcs of inst due to inst's enable has been changed */
    for (i = 0; i < VIR_Inst_GetSrcNum(pInst); i ++)
    {
        pSrcOpnd= VIR_Inst_GetSource(pInst, i);

        swizzle = VIR_Operand_GetSwizzle(pSrcOpnd);

        for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
        {
            if (!VSC_UTILS_TST_BIT(unRedundantEnable, channel))
            {
                continue;
            }

            channelSwizzle = (swizzle >> channel*2) & 0x3;
            swizzle &= ~(0x3 << urSeedChlMappingArray[channel]*2);
            swizzle |= (channelSwizzle << urSeedChlMappingArray[channel]*2);
        }

        VIR_Operand_SetSwizzle(pSrcOpnd, swizzle);
    }

    for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
    {
        if (!VSC_UTILS_TST_BIT(orgInstEnable, channel))
        {
            continue;
        }

        vscVIR_InitGeneralDuIterator(&duIter,
                                     pDuInfo,
                                     pInst,
                                     firstRegNo,
                                     channel,
                                     gcvFALSE);

        for (pUsage = vscVIR_GeneralDuIterator_First(&duIter); pUsage != gcvNULL;
             pUsage = vscVIR_GeneralDuIterator_Next(&duIter))
        {
            pUsageInst = pUsage->usageKey.pUsageInst;
            pUsageOpnd = pUsage->usageKey.pOperand;

            vscVIR_DeleteUsage(pDuInfo,
                               pInst,
                               pUsageInst,
                               pUsageOpnd,
                               firstRegNo,
                               regNoRange,
                               (1 << channel),
                               VIR_HALF_CHANNEL_MASK_FULL,
                               gcvNULL);

            swizzle = VIR_Operand_GetSwizzle(pUsageOpnd);
            newSwizzle = 0;

            for (i = 0; i < VIR_CHANNEL_COUNT; i ++)
            {
                channelSwizzle = (swizzle >> i*2) & 0x3;

                if (channelSwizzle == channel)
                {
                    if (VSC_UTILS_TST_BIT(redundantEnable, channelSwizzle))
                    {
                        newSwizzle |= (pSeedChlMappingArray[channelSwizzle] << i*2);
                    }
                    else if (VSC_UTILS_TST_BIT(unRedundantEnable, channelSwizzle))
                    {
                        newSwizzle |= (urSeedChlMappingArray[channelSwizzle] << i*2);
                    }
                    else
                    {
                        /* Dummy swizzle */
                        newSwizzle |= (channelSwizzle << i*2);
                    }
                }
                else
                {
                    newSwizzle |= (channelSwizzle << i*2);
                }
            }

            VIR_Operand_SetSwizzle(pUsageOpnd, newSwizzle);

            if (VIR_Operand_GetSymbol(pUsageOpnd) != VIR_Operand_GetSymbol(pSeedInstDst))
            {
                VIR_Operand_SetOpKind(pUsageOpnd, VIR_Operand_GetOpKind(pSeedInstDst));
                VIR_Operand_SetType(pUsageOpnd, VIR_Operand_GetType(pSeedInstDst));
                VIR_Operand_SetSym(pUsageOpnd, VIR_Operand_GetSymbol(pSeedInstDst));
            }

            if (VSC_UTILS_TST_BIT(redundantEnable, channel))
            {
                vscVIR_AddNewUsageToDef(pDuInfo,
                                        pSeedInst,
                                        pUsageInst,
                                        pUsageOpnd,
                                        firstSeedRegNo,
                                        seedRegNoRange,
                                        (1 << pSeedChlMappingArray[channel]),
                                        VIR_HALF_CHANNEL_MASK_FULL,
                                        gcvNULL);
            }
            else if (VSC_UTILS_TST_BIT(unRedundantEnable, channel))
            {
                vscVIR_AddNewUsageToDef(pDuInfo,
                                        pInst,
                                        pUsageInst,
                                        pUsageOpnd,
                                        firstSeedRegNo,
                                        seedRegNoRange,
                                        (1 << urSeedChlMappingArray[channel]),
                                        VIR_HALF_CHANNEL_MASK_FULL,
                                        gcvNULL);
            }
        }
    }

    vscVIR_DeleteDef(pDuInfo,
                     pInst,
                     firstRegNo,
                     regNoRange,
                     orgInstEnable,
                     VIR_HALF_CHANNEL_MASK_FULL,
                     gcvNULL);

    /* OK, we have succefully vectorized sym */
    *pVectorizeSucc = gcvTRUE;

    return errCode;
}

static VSC_ErrCode _VectorizeVirreg2Sym(VIR_Shader* pShader,
                                        VIR_DEF_USAGE_INFO* pDuInfo,
                                        VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfo,
                                        gctUINT8 redundantEnable,
                                        gctUINT* pSeedChlMappingArray,
                                        gctBOOL bDst,
                                        gctBOOL* pVectorizeSucc)
{
    return VSC_ERR_NONE;
}

static VSC_ErrCode _VectorizeSym2Virreg(VIR_Shader* pShader,
                                        VIR_DEF_USAGE_INFO* pDuInfo,
                                        VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfo,
                                        gctUINT8 redundantEnable,
                                        gctUINT* pSeedChlMappingArray,
                                        gctBOOL bDst,
                                        gctBOOL* pVectorizeSucc)
{
    return VSC_ERR_NONE;
}

static VSC_ErrCode _VectorizeVirreg2Virreg(VIR_Shader* pShader,
                                           VIR_DEF_USAGE_INFO* pDuInfo,
                                           VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfo,
                                           gctUINT8 redundantEnable,
                                           gctUINT* pSeedChlMappingArray,
                                           gctBOOL bDst,
                                           gctBOOL* pVectorizeSucc)
{
    return VSC_ERR_NONE;
}

static gctUINT8 _CheckRedundantExpressions(VIR_Shader* pShader,
                                           VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfoArray,
                                           gctUINT opndCount,
                                           VIR_Instruction* pSeedInst,
                                           VIR_Instruction* pInst,
                                           gctUINT* pSeedChlMappingArray)
{
    gctUINT                    i, value, seedValue, virRegNo, seedVirRegNo;
    gctUINT8                   chnl, seedChnl, redundantEnable = 0;
    VIR_Enable                 seedInstEnable = VIR_Operand_GetEnable(VIR_Inst_GetDest(pSeedInst));
    VIR_Enable                 instEnable = VIR_Operand_GetEnable(VIR_Inst_GetDest(pInst));

    /* Not-direct-vectorize check result always can not do redundant expression removal. Note
       we only need consider src operands, so starting with 1 */
    for (i = 1; i < opndCount; i ++)
    {
        if (pOpndVectorizedInfoArray[i].ovcResult != VIR_OPND_VECTORIZABILITY_CHK_RES_DIRECT_VECTORIZE)
        {
            return 0;
        }
    }

    /* For each channel in inst's enable to find channel in seed-inst's enable which has
       the same expression as inst */
    for (chnl = VIR_CHANNEL_X; chnl < VIR_CHANNEL_NUM; chnl ++)
    {
        if (!VSC_UTILS_TST_BIT(instEnable, chnl))
        {
            continue;
        }

        for (seedChnl = VIR_CHANNEL_X; seedChnl < VIR_CHANNEL_NUM; seedChnl ++)
        {
            if (!VSC_UTILS_TST_BIT(seedInstEnable, seedChnl))
            {
                continue;
            }

            /* Check src by src */
            for (i = 0; i < VIR_Inst_GetSrcNum(pSeedInst); i ++)
            {
                seedVirRegNo = VIR_Opnd_GetCompWiseSrcChannelValue(pShader,
                                                                   pSeedInst,
                                                                   VIR_Inst_GetSource(pSeedInst, i),
                                                                   seedChnl,
                                                                   &seedValue);

                virRegNo = VIR_Opnd_GetCompWiseSrcChannelValue(pShader,
                                                               pInst,
                                                               VIR_Inst_GetSource(pInst, i),
                                                               chnl,
                                                               &value);

                if (seedVirRegNo != virRegNo)
                {
                    gcmASSERT(gcvFALSE);
                }

                if (seedValue != value)
                {
                    break;
                }
            }

            /* All srcs are same, which means the expression is same */
            if (i == VIR_Inst_GetSrcNum(pSeedInst))
            {
                break;
            }
        }

        /* We have found a same expression on a seed channel, so mark it to redundant enable */
        if (seedChnl < VIR_CHANNEL_NUM)
        {
            VSC_UTILS_SET_BIT(redundantEnable, chnl);
            pSeedChlMappingArray[chnl] = seedChnl;
        }
    }

    return redundantEnable;
}

static VSC_ErrCode _RemoveRedundantExpressions(VIR_Shader* pShader,
                                               VIR_DEF_USAGE_INFO* pDuInfo,
                                               VIR_Instruction* pSeedInst,
                                               VIR_Instruction* pInst,
                                               gctUINT8 redundantEnable,
                                               gctUINT* pSeedChlMappingArray)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    gctUINT8                   channel;
    VIR_Enable                 defEnableMask;
    VIR_DEF_FLAGS              defFlags;
    gctUINT                    firstRegNo, firstSeedRegNo, regNoRange, seedRegNoRange;
    gctUINT                    i, channelSwizzle, swizzle, newSwizzle;
    VIR_GENERAL_DU_ITERATOR    duIter;
    VIR_USAGE*                 pUsage;
    VIR_Instruction*           pUsageInst;
    VIR_Operand*               pUsageOpnd;
    VIR_Enable                 deltaEnable, skipEnable = 0;

    vscVIR_QueryRealWriteVirRegInfo(pDuInfo->baseTsDFA.baseDFA.pOwnerCG->pOwnerShader,
                                    pInst,
                                    &defEnableMask,
                                    gcvNULL,
                                    &firstRegNo,
                                    &regNoRange,
                                    &defFlags,
                                    gcvNULL);

    gcmASSERT(regNoRange == 1);

    vscVIR_QueryRealWriteVirRegInfo(pDuInfo->baseTsDFA.baseDFA.pOwnerCG->pOwnerShader,
                                    pSeedInst,
                                    &defEnableMask,
                                    gcvNULL,
                                    &firstSeedRegNo,
                                    &seedRegNoRange,
                                    &defFlags,
                                    gcvNULL);

    gcmASSERT(seedRegNoRange == 1);

    for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
    {
        if (!VSC_UTILS_TST_BIT(redundantEnable, channel))
        {
            continue;
        }

        vscVIR_InitGeneralDuIterator(&duIter,
                                     pDuInfo,
                                     pInst,
                                     firstRegNo,
                                     channel,
                                     gcvFALSE);

        for (pUsage = vscVIR_GeneralDuIterator_First(&duIter); pUsage != gcvNULL;
             pUsage = vscVIR_GeneralDuIterator_Next(&duIter))
        {
            if (VIR_IS_OUTPUT_USAGE_INST(pUsage->usageKey.pUsageInst))
            {
                skipEnable |= (1 << channel);
                continue;
            }

            pUsageInst = pUsage->usageKey.pUsageInst;
            pUsageOpnd = pUsage->usageKey.pOperand;

            vscVIR_DeleteUsage(pDuInfo,
                               pInst,
                               pUsageInst,
                               pUsageOpnd,
                               firstRegNo,
                               regNoRange,
                               (1 << channel),
                               VIR_HALF_CHANNEL_MASK_FULL,
                               gcvNULL);

            swizzle = VIR_Operand_GetSwizzle(pUsageOpnd);
            newSwizzle = 0;

            for (i = 0; i < VIR_CHANNEL_COUNT; i ++)
            {
                channelSwizzle = (swizzle >> i*2) & 0x3;

                if (channelSwizzle == channel)
                {
                    newSwizzle |= (pSeedChlMappingArray[channel] << i*2);
                }
                else
                {
                    newSwizzle |= (channelSwizzle << i*2);
                }
            }

            VIR_Operand_SetSwizzle(pUsageOpnd, newSwizzle);

            if (firstSeedRegNo != firstRegNo &&
                (VIR_Operand_GetSymbol(pUsageOpnd) != VIR_Operand_GetSymbol(VIR_Inst_GetDest(pSeedInst))))
            {
                VIR_Operand_SetOpKind(pUsageOpnd, VIR_Operand_GetOpKind(VIR_Inst_GetDest(pSeedInst)));
                VIR_Operand_SetType(pUsageOpnd, VIR_Operand_GetType(VIR_Inst_GetDest(pSeedInst)));
                VIR_Operand_SetSym(pUsageOpnd, VIR_Operand_GetSymbol(VIR_Inst_GetDest(pSeedInst)));
            }

            vscVIR_AddNewUsageToDef(pDuInfo,
                                    pSeedInst,
                                    pUsageInst,
                                    pUsageOpnd,
                                    firstSeedRegNo,
                                    seedRegNoRange,
                                    (1 << pSeedChlMappingArray[channel]),
                                    VIR_HALF_CHANNEL_MASK_FULL,
                                    gcvNULL);
        }
    }

    redundantEnable &= (~skipEnable);
    deltaEnable = (VIR_Operand_GetEnable(VIR_Inst_GetDest(pInst)) & ~redundantEnable);

    /* Delete defs for redundant enable */
    if (redundantEnable != 0)
    {
        vscVIR_DeleteDef(pDuInfo,
                         pInst,
                         firstRegNo,
                         regNoRange,
                         redundantEnable,
                         VIR_HALF_CHANNEL_MASK_FULL,
                         gcvNULL);
    }

    /* Ok, redundant expressions have been removed */
    VIR_Operand_SetEnable(VIR_Inst_GetDest(pInst), deltaEnable);

    return errCode;
}

static VSC_ErrCode _TryToRemoveRedundantExpressionsWithoutSymOrVirregVec(VIR_Shader* pShader,
                                                                         VIR_DEF_USAGE_INFO* pDuInfo,
                                                                         VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfoArray,
                                                                         gctUINT opndCount,
                                                                         VIR_Instruction* pSeedInst,
                                                                         VIR_Instruction* pInst,
                                                                         gctUINT8* pRedundantEnable,
                                                                         gctUINT* pSeedChlMappingArray)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_Enable                 defEnableMask;
    gctUINT                    firstRegNo, regNoRange;
    gctUINT8                   channel, srcEnable;
    VIR_GENERAL_DU_ITERATOR    duIter;
    VIR_USAGE*                 pUsage;
    VIR_DEF_KEY                defKey;
    VIR_DEF*                   pDef;
    gctUINT                    defIdx;
    gctBOOL                    bCanRemoveRedundant = gcvFALSE;

    /* Check whether we can simplily remove expressions without sym/vir vectorization
       even if such vectorization is needed */
    if (pOpndVectorizedInfoArray[0].ovcResult == VIR_OPND_VECTORIZABILITY_CHK_RES_NEED_SYM_OR_VIRREG_VECTORIZE)
    {
        /* Don't consider the case of multi-defs usage */
        bCanRemoveRedundant = vscVIR_IsUniqueDefInstOfUsagesInItsDUChain(pDuInfo, pInst, gcvNULL, gcvNULL);

        /* Now check channels of each usages are belongs to redundant enable */
        if (bCanRemoveRedundant)
        {
            if (vscVIR_QueryRealWriteVirRegInfo(pDuInfo->baseTsDFA.baseDFA.pOwnerCG->pOwnerShader,
                                                pInst,
                                                &defEnableMask,
                                                gcvNULL,
                                                &firstRegNo,
                                                &regNoRange,
                                                gcvNULL,
                                                gcvNULL))
            {
                gcmASSERT(regNoRange == 1);

                for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
                {
                    if (!VSC_UTILS_TST_BIT(defEnableMask, channel))
                    {
                        continue;
                    }

                    vscVIR_InitGeneralDuIterator(&duIter,
                                                 pDuInfo,
                                                 pInst,
                                                 firstRegNo,
                                                 channel,
                                                 gcvFALSE);

                    for (pUsage = vscVIR_GeneralDuIterator_First(&duIter); pUsage != gcvNULL;
                         pUsage = vscVIR_GeneralDuIterator_Next(&duIter))
                    {
                        if (VIR_IS_OUTPUT_USAGE_INST(pUsage->usageKey.pUsageInst))
                        {
                            continue;
                        }

                        srcEnable = VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(pUsage->usageKey.pOperand));
                        if (VSC_UTILS_TST(srcEnable, (*pRedundantEnable)) != srcEnable)
                        {
                            bCanRemoveRedundant = gcvFALSE;
                            break;
                        }
                    }

                    if (!bCanRemoveRedundant)
                    {
                        break;
                    }
                }
            }
        }
    }
    else if (pOpndVectorizedInfoArray[0].ovcResult == VIR_OPND_VECTORIZABILITY_CHK_RES_DIRECT_VECTORIZE)
    {
        /* Only consider the case that redundant channel is not equal to seed mapping channel. For that
           case, final operand vectorization will auto remove it  */
        for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
        {
            if (!VSC_UTILS_TST_BIT(*pRedundantEnable, channel))
            {
                continue;
            }

            if (channel != pSeedChlMappingArray[channel])
            {
                bCanRemoveRedundant = gcvTRUE;
                break;
            }
        }
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    /* For the case that mapped seed-inst channel of redundant channel has def other than seed-inst,
       it may need rename, so don't consider this case at this time. In sym/virreg vectorization stage,
       we will take care of it. */
    if (bCanRemoveRedundant)
    {
        vscVIR_QueryRealWriteVirRegInfo(pDuInfo->baseTsDFA.baseDFA.pOwnerCG->pOwnerShader,
                                        pSeedInst,
                                        &defEnableMask,
                                        gcvNULL,
                                        &firstRegNo,
                                        &regNoRange,
                                        gcvNULL,
                                        gcvNULL);

        for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
        {
            if (!VSC_UTILS_TST_BIT(*pRedundantEnable, channel))
            {
                continue;
            }

            defKey.pDefInst = VIR_ANY_DEF_INST;
            defKey.regNo = firstRegNo;
            defKey.channel = (gctUINT8)pSeedChlMappingArray[channel];

            defIdx = vscBT_HashSearch(&pDuInfo->defTable, &defKey);
            while (VIR_INVALID_DEF_INDEX != defIdx)
            {
                pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, defIdx);

                if (pDef->defKey.channel == (gctUINT8)pSeedChlMappingArray[channel] &&
                    pDef->defKey.pDefInst != pSeedInst)
                {
                    bCanRemoveRedundant = gcvFALSE;
                    break;
                }

                /* Get next def with same regNo */
                defIdx = pDef->nextDefIdxOfSameRegNo;
            }
        }
    }

    /* If yes, just remove them now */
    if (bCanRemoveRedundant)
    {
        errCode = _RemoveRedundantExpressions(pShader,
                                              pDuInfo,
                                              pSeedInst,
                                              pInst,
                                              *pRedundantEnable,
                                              pSeedChlMappingArray);
        ON_ERROR(errCode, "Remove redundant expressions");

        /* Mark redundant expressions have been removed */
        *pRedundantEnable = 0;
    }

OnError:
    return errCode;
}

static VSC_ErrCode _VectorizeSymsOrVirregs(VIR_Shader* pShader,
                                           VIR_DEF_USAGE_INFO* pDuInfo,
                                           VIR_OPND_VECTORIZE_CALLBACKS* pOvCallbacks,
                                           VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfoArray,
                                           gctUINT opndCount,
                                           VIR_Instruction* pSeedInst,
                                           VIR_Instruction* pInst,
                                           gctUINT8 redundantEnable,
                                           gctUINT* pSeedChlMappingArray,
                                           gctBOOL* pVectorizeSVSucc)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    gctUINT                    i;
    gctBOOL                    bVectorizeSucc;

    *pVectorizeSVSucc = gcvTRUE;

    /* Don't need go on if no any channel will be written */
    if (VIR_Operand_GetEnable(VIR_Inst_GetDest(pInst)) == VIR_ENABLE_NONE)
    {
        return VSC_ERR_NONE;
    }

    for (i = 0; i < opndCount; ++ i)
    {
        if (pOpndVectorizedInfoArray[i].ovcResult == VIR_OPND_VECTORIZABILITY_CHK_RES_NEED_SYM_OR_VIRREG_VECTORIZE)
        {
            if (pOvCallbacks[pOpndVectorizedInfoArray[i].ovType].pfnSymOrVirregVectorize != gcvNULL)
            {
                errCode = pOvCallbacks[pOpndVectorizedInfoArray[i].ovType].
                                    pfnSymOrVirregVectorize(pShader, pDuInfo, &pOpndVectorizedInfoArray[i],
                                                            redundantEnable, pSeedChlMappingArray, (i == 0),
                                                            &bVectorizeSucc);
                ON_ERROR(errCode, "Callbacks on calling indivdiual syms or virregs vectorization");

                *pVectorizeSVSucc &= bVectorizeSucc;

                if (*pVectorizeSVSucc == gcvFALSE)
                {
                    return VSC_ERR_NONE;
                }

                /* Now operand can be vectorized directly */
                pOpndVectorizedInfoArray[i].ovcResult = VIR_OPND_VECTORIZABILITY_CHK_RES_DIRECT_VECTORIZE;
            }
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _VectorizeInstToSeedInst(VIR_Shader* pShader,
                                            VIR_BASIC_BLOCK* pBasicBlock,
                                            VIR_DEF_USAGE_INFO* pDuInfo,
                                            VIR_OPND_VECTORIZE_CALLBACKS* pOvCallbacks,
                                            VIR_OPND_VECTORIZED_INFO* pOpndVectorizedInfoArray,
                                            VIR_OPND_VECTORIZE_MODE ovMode,
                                            VIR_Instruction* pSeedInst,
                                            VIR_Instruction* pInst,
                                            gctBOOL* pVectorizeSucc)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    gctUINT                    i, opndCount = (1 + VIR_Inst_GetSrcNum(pSeedInst));
    gctUINT8                   orgSeedEnable;
    gctUINT8                   orgEnable;
    gctUINT8                   redundantEnable;
    gctUINT                    seedChlMappingArray[VIR_CHANNEL_NUM];
    gctBOOL                    bVectorizeSVSucc = gcvFALSE;

    *pVectorizeSucc = gcvFALSE;

    /* We need firstly check whether inst has same expression (fully or partially) as seed-inst. */
    redundantEnable = _CheckRedundantExpressions(pShader,
                                                 pOpndVectorizedInfoArray,
                                                 opndCount,
                                                 pSeedInst,
                                                 pInst,
                                                 &seedChlMappingArray[0]);

    /* If there are redundant expressions, we will check whether these redundant expressions can be
       removed without sym/virreg vectorization (something like local CSE, but dont consider complex
       cases which need special DFA). Note that we'll make another trial to remove these redundant
       expressions when doing sym/virreg vectorization if we can not remove them at this time. */
    if (redundantEnable)
    {
        errCode = _TryToRemoveRedundantExpressionsWithoutSymOrVirregVec(pShader,
                                                                        pDuInfo,
                                                                        pOpndVectorizedInfoArray,
                                                                        opndCount,
                                                                        pSeedInst,
                                                                        pInst,
                                                                        &redundantEnable,
                                                                        &seedChlMappingArray[0]);
        ON_ERROR(errCode, "Try to remove redundant experessions without sym/virreg vectorization");
    }

    /* Before doing operands vectorization, do sym/virreg vectorization by which, new syms or virregs
       will be used in operand vectorization. Note that after it is done, NEED_SYM_OR_VIRREG_VECTORIZE
       will be changed to DIRECT_VECTORIZE */
    errCode = _VectorizeSymsOrVirregs(pShader,
                                      pDuInfo,
                                      pOvCallbacks,
                                      pOpndVectorizedInfoArray,
                                      opndCount,
                                      pSeedInst,
                                      pInst,
                                      redundantEnable,
                                      &seedChlMappingArray[0],
                                      &bVectorizeSVSucc);
    ON_ERROR(errCode, "Vectorize Syms or virregs");

    /* Due to some reasons, failed to vectorize sym/vir, then we don't need go on */
    if (!bVectorizeSVSucc)
    {
        return VSC_ERR_NONE;
    }

    orgSeedEnable = VIR_Operand_GetEnable(VIR_Inst_GetDest(pSeedInst));
    orgEnable = VIR_Operand_GetEnable(VIR_Inst_GetDest(pInst));

    if (orgEnable)
    {
        for (i = 0; i < opndCount; ++ i)
        {
            errCode = _VectorizeOpndToSeedOpnd(pShader,
                                               pBasicBlock,
                                               pDuInfo,
                                               pOvCallbacks,
                                               &pOpndVectorizedInfoArray[i],
                                               orgSeedEnable,
                                               orgEnable,
                                               (i == 0));
            ON_ERROR(errCode, "Vectorize perand to seed operand");
        }
    }

    /* As all changes have been made into seed inst, for FROM_SEED_INST mode, we need
       move seed inst to the place of inst and delete the inst later. */
    if (ovMode == VIR_OPND_VECTORIZE_MODE_FROM_SEED_INST)
    {
        VIR_Function_MoveInstructionBefore(pBasicBlock->pOwnerCFG->pOwnerFuncBlk->pVIRFunc,
                                           pInst, pSeedInst);

        /* Keep inst Id ordered */
        VIR_Inst_SetId(pSeedInst, VIR_Inst_GetId(pInst));
    }

    /* Delete inst */
    VIR_Function_RemoveInstruction(pBasicBlock->pOwnerCFG->pOwnerFuncBlk->pVIRFunc, pInst);

    *pVectorizeSucc = gcvTRUE;

OnError:
    return errCode;
}

static VSC_ErrCode _FindInstsToVectorizeToSeedInst(VIR_Shader* pShader,
                                                   VIR_BASIC_BLOCK* pBasicBlock,
                                                   VIR_DEF_USAGE_INFO* pDuInfo,
                                                   VIR_OPND_VECTORIZE_CALLBACKS* pOvCallbacks,
                                                   VIR_Instruction* pSeedInst,
                                                   gctBOOL* pbSeedInstRelocFwdly)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_OpCode                 seedOpcode = VIR_Inst_GetOpcode(pSeedInst);
    VIR_Instruction*           pInst = VIR_Inst_GetNext(pSeedInst);
    VIR_Instruction*           pNextInst;
    gctBOOL                    bTerminate, bVectorizeSucc;
    gctUINT                    i;
    VIR_OPND_VECTORIZE_MODE    ovMode;
    VIR_OPND_VECTORIZED_INFO   opndVectorizedInfoArray[VIR_MAX_SRC_NUM + 1]; /* dst + srcs, index 0 is dst, and other indices are srcs */

    *pbSeedInstRelocFwdly = gcvFALSE;

    if (!VIR_OPCODE_isComponentwise(seedOpcode) || !VIR_OPCODE_hasDest(seedOpcode))
    {
        return VSC_ERR_NONE;
    }

    /* If seed is already at end of bb, just return */
    if (pSeedInst == BB_GET_END_INST(pBasicBlock))
    {
        return VSC_ERR_NONE;
    }

    memset(opndVectorizedInfoArray, 0, sizeof(VIR_OPND_VECTORIZED_INFO)*(VIR_MAX_SRC_NUM + 1));

    while (pInst)
    {
        bTerminate = (pInst == BB_GET_END_INST(pBasicBlock));

        /* Get next inst */
        pNextInst = VIR_Inst_GetNext(pInst);

        /* Try to check whether current inst can be vectorized into seed inst, if yes, merge it */
        if (_CanInstVectorizeToSeedInst(pShader, pDuInfo, pOvCallbacks,
                                        pSeedInst, pInst, opndVectorizedInfoArray,
                                        &ovMode))
        {
            errCode = _VectorizeInstToSeedInst(pShader, pBasicBlock, pDuInfo,
                                               pOvCallbacks, opndVectorizedInfoArray,
                                               ovMode, pSeedInst, pInst, &bVectorizeSucc);
            ON_ERROR(errCode, "Vectorize insts to seed inst");

            if (bVectorizeSucc)
            {
                *pbSeedInstRelocFwdly |= (ovMode == VIR_OPND_VECTORIZE_MODE_FROM_SEED_INST);
            }
        }

        /* If current inst is the last inst of block, just bail out */
        if (bTerminate)
        {
            break;
        }

        /* Move to next inst */
        pInst = pNextInst;
    }

OnError:
    for (i = 0; i < VIR_MAX_SRC_NUM + 1; i ++)
    {
        if (opndVectorizedInfoArray[i].vectorizedInfo.ppVectorizedVirRegArray)
        {
            vscMM_Free(&pShader->mempool, opndVectorizedInfoArray[i].vectorizedInfo.ppVectorizedVirRegArray);
        }
    }

    return errCode;
}

static VSC_ErrCode _DoVectorizationOnBasicBlock(VIR_Shader* pShader,
                                                VIR_BASIC_BLOCK* pBasicBlock,
                                                VIR_DEF_USAGE_INFO* pDuInfo,
                                                VIR_OPND_VECTORIZE_CALLBACKS* pOvCallbacks)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_Instruction*           pSeedInst = BB_GET_START_INST(pBasicBlock);
    VIR_Instruction*           pOrgNextInst;
    gctBOOL                    bSeedInstRelocFwdly;

    while (pSeedInst)
    {
        /* Get original next inst */
        pOrgNextInst = VIR_Inst_GetNext(pSeedInst);

        errCode = _FindInstsToVectorizeToSeedInst(pShader, pBasicBlock, pDuInfo,
                                                  pOvCallbacks, pSeedInst, &bSeedInstRelocFwdly);
        ON_ERROR(errCode, "Find Insts to vectorize to seed inst");

        /* If current inst is the last inst of block, just bail out */
        if (!bSeedInstRelocFwdly && (pSeedInst == BB_GET_END_INST(pBasicBlock)))
        {
            break;
        }

        /* Move to next inst */
        pSeedInst = bSeedInstRelocFwdly ? pOrgNextInst : VIR_Inst_GetNext(pSeedInst);
    }

OnError:
    return errCode;
}

static VSC_ErrCode _DoVectorizationOnFunc(VIR_Shader* pShader,
                                          VIR_Function* pFunc,
                                          VIR_DEF_USAGE_INFO* pDuInfo,
                                          VIR_OPND_VECTORIZE_CALLBACKS* pOvCallbacks)
{
    VSC_ErrCode            errCode = VSC_ERR_NONE;
    VIR_FUNC_BLOCK*        pFuncBlk = pFunc->pFuncBlock;
    CFG_ITERATOR           basicBlkIter;
    VIR_BASIC_BLOCK*       pThisBlock;

    /* Go through all basic blocks of each func */
    CFG_ITERATOR_INIT(&basicBlkIter, &pFuncBlk->cfg);
    pThisBlock = (VIR_BASIC_BLOCK *)CFG_ITERATOR_FIRST(&basicBlkIter);
    for (; pThisBlock != gcvNULL; pThisBlock = (VIR_BASIC_BLOCK *)CFG_ITERATOR_NEXT(&basicBlkIter))
    {
        errCode = _DoVectorizationOnBasicBlock(pShader, pThisBlock, pDuInfo, pOvCallbacks);
        ON_ERROR(errCode, "Do vectorization on basic block");
    }

OnError:
    return errCode;
}

gctBOOL vscVIR_CheckTwoSymsVectorizability(VIR_Shader* pShader, VIR_Symbol* pSym1, VIR_Symbol* pSym2)
{
    gctSTRING strSymName1, strSymName2, strTemp1, strTemp2;

    /* Storage-class must be same */
    if (VIR_Symbol_GetStorageClass(pSym1) != VIR_Symbol_GetStorageClass(pSym2))
    {
        return gcvFALSE;
    }

    /* Sym kind must be same */
    if (VIR_Symbol_GetKind(pSym1) != VIR_Symbol_GetKind(pSym2))
    {
        return gcvFALSE;
    }

    /* Precision must be equal */
    if (VIR_Symbol_GetPrecision(pSym1) != VIR_Symbol_GetPrecision(pSym2))
    {
        return gcvFALSE;
    }

    /* Scalar sym can not be matched with arrayed one */
    if (VIR_Type_isArray(VIR_Symbol_GetType(pSym1)) !=
        VIR_Type_isArray(VIR_Symbol_GetType(pSym2)))
    {
        return gcvFALSE;
    }

    /* If arrayed, array size must be equal */
    if (VIR_Type_isArray(VIR_Symbol_GetType(pSym1)))
    {
        if (VIR_Type_GetArrayLength(VIR_Symbol_GetType(pSym1)) !=
            VIR_Type_GetArrayLength(VIR_Symbol_GetType(pSym2)))
        {
            return gcvFALSE;
        }
    }

    /* Primitive type must have same row and component type */
    if (VIR_GetTypeRows(VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(pSym1))) !=
        VIR_GetTypeRows(VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(pSym2))) ||
        VIR_GetTypeComponentType(VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(pSym1))) !=
        VIR_GetTypeComponentType(VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(pSym2))))
    {
        return gcvFALSE;
    }

    /* Check struct field (a temp solution) */
    if (!VIR_Symbol_isConst(pSym1) && !VIR_Symbol_isVreg(pSym1))
    {
        gcoOS_StrDup(gcvNULL, VIR_Shader_GetSymNameString(pShader, pSym1), &strSymName1);
        gcoOS_StrDup(gcvNULL, VIR_Shader_GetSymNameString(pShader, pSym2), &strSymName2);
        gcoOS_StrFindReverse(strSymName1, '.', &strTemp1);
        if (strTemp1) strTemp1[0] = '\0';
        gcoOS_StrFindReverse(strSymName2, '.', &strTemp2);
        if (strTemp2) strTemp2[0] = '\0';
        if (strTemp1 && strTemp2)
        {
            if (!gcmIS_SUCCESS(gcoOS_StrCmp(strSymName1, strSymName2)))
            {
                return gcvFALSE;
            }
        }
        else if ((strTemp1 && !strTemp2) || (!strTemp1 && strTemp2))
        {
            return gcvFALSE;
        }
        gcoOS_Free(gcvNULL, strSymName1);
        gcoOS_Free(gcvNULL, strSymName2);
    }

    /* IO syms special checkings */
    if (VIR_Symbol_isInputOrOutput(pSym1))
    {
        gcmASSERT(VIR_Symbol_isInputOrOutput(pSym2));

        /* For IO syms, interpolation mode must be equal */
        if ((isSymCentroid(pSym1) != isSymCentroid(pSym2)) ||
            (isSymSample(pSym1) != isSymSample(pSym2)) ||
            (isSymFlat(pSym1) != isSymFlat(pSym2)))
        {
            return gcvFALSE;
        }

        /* For IOB's member sym, they must be in same IOB.
           TODO: We need loose it to get better vectorizability later */
        if (isSymIOBlockMember(pSym1) != isSymIOBlockMember(pSym2) ||
            (isSymIOBlockMember(pSym1) && VIR_Symbol_GetIOBlockIndex(pSym1) != VIR_Symbol_GetIOBlockIndex(pSym2)))
        {
            return gcvFALSE;
        }
    }

    return gcvTRUE;
}

VSC_ErrCode vscVIR_VectorizeIoPackets(VIR_Shader* pShader,
                                      VIR_IO_VECTORIZABLE_PACKET* pIoVectorizablePackets,
                                      gctUINT numOfPackets)
{
    VSC_ErrCode              errCode = VSC_ERR_NONE;
    gctUINT                  packetIdx;
    VIR_IO_VECTORIZED_INFO*  pIoVectorizedInfoArray = gcvNULL;

    pIoVectorizedInfoArray = (VIR_IO_VECTORIZED_INFO*)vscMM_Alloc(&pShader->mempool,
                                                             sizeof(VIR_IO_VECTORIZED_INFO)*numOfPackets);

    /* Create io-vectorized-info based on each io-packet */
    for (packetIdx = 0; packetIdx < numOfPackets; packetIdx ++)
    {
        errCode = _CreateIoVectorizedInfoFromIoPacket(pShader,
                                                      &pIoVectorizablePackets[packetIdx],
                                                      &pIoVectorizedInfoArray[packetIdx]);
        ON_ERROR(errCode, "Create vectorized info from one IO packet");
    }

    /* Now change reference of old sym to new sym in each inst */
    _ChangeInstsByIoVectorizedInfos(pShader, pIoVectorizedInfoArray, numOfPackets);

    if (gcSHADER_DumpCodeGenVerbose(pShader))
    {
        VIR_Shader_Dump(gcvNULL, "After IO vectorization.", pShader, gcvTRUE);
    }

OnError:
    if (pIoVectorizedInfoArray)
    {
        for (packetIdx = 0; packetIdx < numOfPackets; packetIdx ++)
        {
            if (pIoVectorizedInfoArray[packetIdx].vectorizedInfo.ppVectorizedVirRegArray)
            {
                vscMM_Free(&pShader->mempool, pIoVectorizedInfoArray[packetIdx].vectorizedInfo.ppVectorizedVirRegArray);
            }
        }

        vscMM_Free(&pShader->mempool, pIoVectorizedInfoArray);
    }

    return errCode;
}

VSC_ErrCode vscVIR_DoLocalVectorization(VIR_Shader* pShader, VIR_DEF_USAGE_INFO* pDuInfo)
{
    VSC_ErrCode                    errCode = VSC_ERR_NONE;
    VIR_FuncIterator               func_iter;
    VIR_FunctionNode*              func_node;
    VIR_Function*                  func;
    VIR_OPND_VECTORIZE_CALLBACKS   ovCallbacks[] =
    {
        /*              type                                 check-cb, sym-virreg-vectorize-cb, opnd-vectorize-cb */
        {VIR_OPND_VECTORIZE_TYPE_UNKNOW, gcvNULL, gcvNULL, gcvNULL             },
        {VIR_OPND_VECTORIZE_TYPE_SYM_2_SYM, _Sym2SymOpndsVectorizabilityCheck, _VectorizeSym2Sym, _VectorizeSym2SymOpnds      },
        {VIR_OPND_VECTORIZE_TYPE_VIRREG_2_SYM, _Virreg2SymOpndsVectorizabilityCheck, _VectorizeVirreg2Sym, _VectorizeVirreg2SymOpnds   },
        {VIR_OPND_VECTORIZE_TYPE_SYM_2_VIRREG, _Sym2VirregOpndsVectorizabilityCheck, _VectorizeSym2Virreg, _VectorizeSym2VirregOpnds   },
        {VIR_OPND_VECTORIZE_TYPE_VIRREG_2_VIRREG, _Virreg2VirregOpndsVectorizabilityCheck, _VectorizeVirreg2Virreg, _VectorizeVirreg2VirregOpnds},
        {VIR_OPND_VECTORIZE_TYPE_SIMM_2_SIMM, _Simm2SimmOpndsVectorizabilityCheck, gcvNULL, _VectorizeSimm2SimmOpnds    },
        {VIR_OPND_VECTORIZE_TYPE_VIMM_2_SIMM, _Vimm2SimmOpndsVectorizabilityCheck, gcvNULL, _VectorizeVimm2SimmOpnds    },
        {VIR_OPND_VECTORIZE_TYPE_SIMM_2_VIMM, _Simm2VimmOpndsVectorizabilityCheck, gcvNULL, _VectorizeSimm2VimmOpnds    },
        {VIR_OPND_VECTORIZE_TYPE_VIMM_2_VIMM, _Vimm2VimmOpndsVectorizabilityCheck, gcvNULL, _VectorizeVimm2VimmOpnds    },
    };

    VIR_Shader_RenumberInstId(pShader);

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        func = func_node->function;

        errCode = _DoVectorizationOnFunc(pShader, func, pDuInfo, ovCallbacks);
        ON_ERROR(errCode, "Do vectorization on func");
    }

    if (gcSHADER_DumpCodeGenVerbose(pShader))
    {
        VIR_Shader_Dump(gcvNULL, "After local vectorization", pShader, gcvTRUE);
    }

OnError:
    return errCode;
}


