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

VSC_ErrCode
VIR_IO_ReallocateMem(
    VIR_Shader_IOBuffer * Buf,
    gctUINT               Sz
    )
{
    return VSC_IO_ReallocateMem(Buf->ioBuffer, Sz);
    }

VSC_ErrCode
VIR_IO_Init(VIR_Shader_IOBuffer *Buf, VSC_IO_BUFFER *IOBuf, VIR_Shader *Shader, gctUINT Size, gctBOOL QueryOnly)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    Buf->ioBuffer = IOBuf;
    Buf->shader = Shader;
    if (!QueryOnly)
    {
        errCode = VSC_IO_Init(Buf->ioBuffer, Size);
    }
    else
    {
        Buf->ioBuffer->buffer = gcvNULL;
        Buf->ioBuffer->allocatedBytes = 1024;
        Buf->ioBuffer->curPos = 0;;
    }

    return errCode;
}

void
VIR_IO_Finalize(VIR_Shader_IOBuffer *Buf, gctBOOL bFreeBuffer)
{
#if _DEBUG_VIR_IO_COPY
    {
        if (VirSHADER_DumpCodeGenVerbose(Buf->shader))
        {
            VIR_Dumper       *Dumper = Buf->shader->dumper;
            VERIFY_OK(
                VIR_LOG(Dumper, "VIR_IO buffer allocated %d KB for shader [id:%d]\n",
                        (int)(Buf->allocatedBytes/1024), Buf->shader->_id));
            VIR_LOG_FLUSH(Dumper);
        }
    }
#endif
    if (bFreeBuffer && Buf->ioBuffer)
    {
        VSC_IO_Finalize(Buf->ioBuffer);
    }
}

static void
_VIR_IO_SymbolListQueue(
    IN VSC_MM                   *pMM,
    IN VSC_SIMPLE_QUEUE         *pWorkList,
    IN VIR_Symbol               *pSymbol
    )
{
    VSC_UNI_LIST_NODE_EXT *worklistNode = (VSC_UNI_LIST_NODE_EXT *)vscMM_Alloc(pMM,
        sizeof(VSC_UNI_LIST_NODE_EXT));

    vscULNDEXT_Initialize(worklistNode, pSymbol);
    QUEUE_PUT_ENTRY(pWorkList, worklistNode);
}

static void
_VIR_IO_SymbolListDequeue(
    IN VSC_MM                   *pMM,
    IN VSC_SIMPLE_QUEUE         *pWorkList,
    OUT VIR_Symbol             **pSymbol
    )
{
    VSC_UNI_LIST_NODE_EXT *worklistNode = (VSC_UNI_LIST_NODE_EXT *)QUEUE_GET_ENTRY(pWorkList);

    *pSymbol = (VIR_Symbol *)vscULNDEXT_GetContainedUserData(worklistNode);

    vscMM_Free(pMM, worklistNode);
}

VSC_ErrCode
VIR_IO_UpdateHostFunction(VIR_Shader* pShader, VSC_UNI_LIST* pSymList)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_MM*             pMM = &pShader->pmp.mmWrapper;
    VSC_SIMPLE_QUEUE*   pWorkList = (VSC_SIMPLE_QUEUE*)pSymList;
    VIR_Symbol*         pSym = gcvNULL;
    VIR_SymId           funcSymId = VIR_INVALID_ID;
    VIR_Symbol*         pFuncSym = gcvNULL;

    while(!QUEUE_CHECK_EMPTY(pWorkList))
    {
        _VIR_IO_SymbolListDequeue(pMM, pWorkList, &pSym);
        gcmASSERT(isSymLocal(pSym) && VIR_Id_isFunctionScope(VIR_Symbol_GetIndex(pSym)));

        funcSymId = (VIR_SymId)(gcmPTR2INT32(VIR_Symbol_GetHostFunction(pSym)));

        pFuncSym = VIR_Shader_GetSymFromId(pShader, funcSymId);
        gcmASSERT(VIR_Symbol_isFunction(pFuncSym));

        VIR_Symbol_SetHostFunction(pSym, VIR_Symbol_GetFunction(pFuncSym));
        gcmASSERT(VIR_Symbol_GetHostFunction(pSym));
    }

    return errCode;
    }

#define VIR_IO_ReserveBytes(Buf, Size) ((((Buf)->ioBuffer)->curPos + Size > ((Buf)->ioBuffer)->allocatedBytes) ?  \
                                        VIR_IO_ReallocateMem(Buf, ((Buf)->ioBuffer)->curPos + Size) : VSC_ERR_NONE)

VSC_ErrCode
VIR_IO_CheckBounds(VIR_Shader_IOBuffer *Buf, gctUINT Size)
{
    gctUINT64 tempCount;
    tempCount = (gctUINT64)(Buf->ioBuffer->curPos) + (gctUINT64)Size;
    if ((tempCount > gcvMAXUINT32) ||
        ((Buf->ioBuffer)->curPos + Size > ((Buf)->ioBuffer)->allocatedBytes))
    {
        return VSC_ERR_OUT_OF_BOUNDS;
    }
    else
    {
        return VSC_ERR_NONE;
    }
}

VSC_ErrCode
VIR_IO_writeInt(VIR_Shader_IOBuffer *Buf, gctINT Val)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_writeInt(Buf->ioBuffer, Val);
    return errCode;
}

VSC_ErrCode
VIR_IO_writeUint(VIR_Shader_IOBuffer *Buf, gctUINT Val)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_writeUint(Buf->ioBuffer, Val);
    return errCode;
}

VSC_ErrCode
VIR_IO_writeShort(VIR_Shader_IOBuffer *Buf, gctINT16 Val)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_writeShort(Buf->ioBuffer, Val);
    return errCode;
}

VSC_ErrCode
VIR_IO_writeUshort(VIR_Shader_IOBuffer *Buf, gctUINT16 Val)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_writeUshort(Buf->ioBuffer, Val);
    return errCode;
}

VSC_ErrCode
VIR_IO_writeFloat(VIR_Shader_IOBuffer *Buf, gctFLOAT Val)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_writeFloat(Buf->ioBuffer, Val);
    return errCode;
}

VSC_ErrCode
VIR_IO_writeChar(VIR_Shader_IOBuffer *Buf, gctCHAR Val)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_writeChar(Buf->ioBuffer, Val);
    return errCode;
}

VSC_ErrCode
VIR_IO_writeBlock(VIR_Shader_IOBuffer *Buf, gctCHAR *Val, gctUINT Sz)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_writeBlock(Buf->ioBuffer, Val, Sz);
    return errCode;
}

VSC_ErrCode
VIR_IO_writeBlockTable(VIR_Shader_IOBuffer * Buf,
                       VSC_BLOCK_TABLE *     pBlockTbl,
                       WRITE_NODE_FP         fp,
                       VIR_Id                StartIdToWrite)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER   *ioBuffer = Buf->ioBuffer;
    gctUINT usedBytes;
    errCode = VIR_IO_ReserveBytes(Buf, 5*sizeof(gctUINT));
    ON_ERROR(errCode, "Failure on writeBlockTable");

    VIR_IO_writeUint(Buf, StartIdToWrite);

    VIR_IO_writeUint(Buf, pBlockTbl->curBlockIdx);
    VIR_IO_writeUint(Buf, pBlockTbl->nextOffsetInCurBlock);
    VIR_IO_writeUint(Buf, pBlockTbl->flag);
    VIR_IO_writeUint(Buf, pBlockTbl->entrySize);
    VIR_IO_writeUint(Buf, pBlockTbl->blockSize);

    /* get used bolck table size */
    usedBytes = vscBT_GetUsedSize(pBlockTbl);
    errCode = VIR_IO_ReserveBytes(Buf, usedBytes);
    if (errCode == VSC_ERR_NONE)
    {
        gctUINT i;
        gctUINT j;
        gctUINT startToWriteBlockIndex = BT_GET_BLOCK_INDEX(pBlockTbl, StartIdToWrite);
        gctUINT startToWriteBlockOffset = BT_GET_BLOCK_OFFSET(pBlockTbl, StartIdToWrite);

        /* write whole blocks */
        for (i=startToWriteBlockIndex; i < pBlockTbl->curBlockIdx; i++)
        {
            if (fp)
            {
                for (j=0; j < pBlockTbl->entryCountPerBlock; j++)
                {
                    gctUINT offset =  j * pBlockTbl->entrySize;
                    if (i == startToWriteBlockIndex && offset < startToWriteBlockOffset)
                    {
                        /* skip the data before the start to write point */
                        continue;
                    }
                    ON_ERROR(fp(Buf, pBlockTbl->ppBlockArray[i] + offset), "write block elem");
                }
            }
            else
            {
                if (i == startToWriteBlockIndex)
                {
                    if (ioBuffer->buffer)
                    {
                        gcoOS_MemCopy(ioBuffer->buffer+ioBuffer->curPos,
                                      pBlockTbl->ppBlockArray[i] + startToWriteBlockOffset,
                                      pBlockTbl->blockSize - startToWriteBlockOffset);
                    }
                    ioBuffer->curPos += pBlockTbl->blockSize  - startToWriteBlockOffset;
                }
                else
                {
                    if (ioBuffer->buffer)
                    {
                        gcoOS_MemCopy(ioBuffer->buffer+ioBuffer->curPos,
                                      pBlockTbl->ppBlockArray[i],
                                      pBlockTbl->blockSize);
                    }
                    ioBuffer->curPos += pBlockTbl->blockSize;
                }
            }
        }

        /* write data in last block */
        if (pBlockTbl->nextOffsetInCurBlock > 0)
        {
            if (fp)
            {
                for (j=0; j < pBlockTbl->nextOffsetInCurBlock/pBlockTbl->entrySize; j++)
                {
                    gctUINT offset =  j * pBlockTbl->entrySize;
                    if (i == startToWriteBlockIndex && offset < startToWriteBlockOffset)
                    {
                        /* skip the data before the start to write point */
                        continue;
                    }
                    ON_ERROR(fp(Buf, pBlockTbl->ppBlockArray[i] + offset), "write block elem");
                }
            }
            else
            {
                if (i == startToWriteBlockIndex)
                {
                    if (ioBuffer->buffer)
                    {
                        gcoOS_MemCopy(ioBuffer->buffer+ioBuffer->curPos,
                                      pBlockTbl->ppBlockArray[i] + startToWriteBlockOffset,
                                      pBlockTbl->nextOffsetInCurBlock - startToWriteBlockOffset);
                    }
                    ioBuffer->curPos += pBlockTbl->nextOffsetInCurBlock  - startToWriteBlockOffset;
                }
                else
                {
                    if (ioBuffer->buffer)
                    {
                        gcoOS_MemCopy(ioBuffer->buffer+ioBuffer->curPos,
                                      pBlockTbl->ppBlockArray[pBlockTbl->curBlockIdx],
                                      pBlockTbl->nextOffsetInCurBlock);
                    }
                    ioBuffer->curPos += pBlockTbl->nextOffsetInCurBlock;
               }
            }
        }
    }
OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_writeStringTable(VIR_Shader_IOBuffer *Buf, VIR_StringTable* pStringTbl)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_HASH_ITERATOR     iter;
    VSC_DIRECT_HNODE_PAIR pair;
    /* write bolcktable */
    errCode = VIR_IO_writeBlockTable(Buf, pStringTbl, gcvNULL, VIR_NAME_BUILTIN_LAST);
    ON_ERROR(errCode, "Failed to write string table");

    /* write hash table, only need to write nameId, hashTable can be
     * re-constructed from NameId at read-time */
    vscHTBLIterator_Init(&iter, pStringTbl->pHashTable);
    for (pair = vscHTBLIterator_DirectFirst(&iter);
         IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
    {
        VIR_NameId id = (VIR_NameId)(size_t)VSC_DIRECT_HNODE_PAIR_SECOND(&pair);
        gcmASSERT(!VIR_Id_isInvalid(id));
        errCode = VIR_IO_writeUint(Buf, id);
        ON_ERROR(errCode, "Failed to write nameId");
    }

    /* write invalidId as last item */
    VIR_IO_writeUint(Buf, VIR_INVALID_ID);

OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_writeTypeTable(VIR_Shader_IOBuffer *Buf, VIR_TypeTable* pTypeTbl)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_HASH_ITERATOR     iter;
    VSC_DIRECT_HNODE_PAIR pair;

    ON_ERROR0(VIR_IO_writeBlockTable(Buf, pTypeTbl, (WRITE_NODE_FP)VIR_IO_writeType,
                                    VIR_TYPE_LAST_PRIMITIVETYPE+1));

    vscHTBLIterator_Init(&iter, pTypeTbl->pHashTable);
    for (pair = vscHTBLIterator_DirectFirst(&iter);
         IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
    {
        VIR_TypeId tyId = (VIR_TypeId)(size_t) VSC_DIRECT_HNODE_PAIR_SECOND(&pair);
        if (VIR_TypeId_isPrimitive(tyId))
        {
            /* don't write primitive type, it is initialized for every type table */
            continue;
        }
        VIR_IO_writeUint(Buf, tyId);
    }
    /* write invalidId as last item */
    VIR_IO_writeUint(Buf, VIR_INVALID_ID);

OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_writeLabelTable(VIR_Shader_IOBuffer *Buf, VIR_LabelTable* pLabelTbl)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_HASH_ITERATOR     iter;
    VSC_DIRECT_HNODE_PAIR pair;

    ON_ERROR0(VIR_IO_writeBlockTable(Buf, pLabelTbl, (WRITE_NODE_FP)VIR_IO_writeLabel, 0));

    vscHTBLIterator_Init(&iter, pLabelTbl->pHashTable);
    for (pair = vscHTBLIterator_DirectFirst(&iter);
         IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
    {
        VIR_LabelId labelId = (VIR_LabelId)(size_t) VSC_DIRECT_HNODE_PAIR_SECOND(&pair);
        ON_ERROR0(VIR_IO_writeUint(Buf, labelId));
    }
    /* write invalidId as last item */
    VIR_IO_writeUint(Buf, VIR_INVALID_ID);

OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_writeConstTable(VIR_Shader_IOBuffer *Buf, VIR_ConstTable* pConstTbl)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_HASH_ITERATOR     iter;
    VSC_DIRECT_HNODE_PAIR pair;

    ON_ERROR0(VIR_IO_writeBlockTable(Buf, pConstTbl, (WRITE_NODE_FP)VIR_IO_writeConst, 0));

    vscHTBLIterator_Init(&iter, pConstTbl->pHashTable);
    for (pair = vscHTBLIterator_DirectFirst(&iter);
         IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
    {
        VIR_ConstId constId = (VIR_ConstId)(size_t) VSC_DIRECT_HNODE_PAIR_SECOND(&pair);
        ON_ERROR0(VIR_IO_writeUint(Buf, constId));
    }
    /* write invalidId as last item */
    VIR_IO_writeUint(Buf, VIR_INVALID_ID);

OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_writeOperandTable(VIR_Shader_IOBuffer *Buf, VIR_OperandTable* pOperandTbl)
{
    VSC_ErrCode errCode = VIR_IO_writeBlockTable(Buf, pOperandTbl, (WRITE_NODE_FP)VIR_IO_writeOperand, 0);
    return errCode;
}

VSC_ErrCode
VIR_IO_writeSymTable(VIR_Shader_IOBuffer *Buf, VIR_SymTable* pSymTbl)
{
    VSC_ErrCode errCode =  VIR_IO_writeBlockTable(Buf, pSymTbl, (WRITE_NODE_FP)VIR_IO_writeSymbol, 0);
    VSC_HASH_ITERATOR     iter;
    VSC_DIRECT_HNODE_PAIR pair;

    vscHTBLIterator_Init(&iter, pSymTbl->pHashTable);
    for (pair = vscHTBLIterator_DirectFirst(&iter);
         IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
    {
        VIR_SymId symId = (VIR_SymId)(size_t) VSC_DIRECT_HNODE_PAIR_SECOND(&pair);
        VIR_IO_writeUint(Buf, symId);
    }
    /* write invalidId as last item */
    VIR_IO_writeUint(Buf, VIR_INVALID_ID);

    return errCode;
}

VSC_ErrCode
VIR_IO_writeVirRegTable(VIR_Shader_IOBuffer *Buf, VIR_VirRegTable* pVirRegTable)
{
    VSC_ErrCode errCode =  VSC_ERR_NONE;
    VSC_HASH_ITERATOR     iter;
    VSC_DIRECT_HNODE_PAIR pair;

    vscHTBLIterator_Init(&iter, pVirRegTable);
    for (pair = vscHTBLIterator_DirectFirst(&iter);
         IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
    {
        VIR_SymId virRegId = (VIR_VirRegId)(size_t) VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
        VIR_SymId symId = (VIR_SymId)(size_t) VSC_DIRECT_HNODE_PAIR_SECOND(&pair);
        VIR_IO_writeUint(Buf, virRegId);
        VIR_IO_writeUint(Buf, symId);
    }
    /* write invalidId as last item */
    VIR_IO_writeUint(Buf, VIR_INVALID_ID);

    return errCode;
}

VSC_ErrCode
VIR_IO_writeIdList(VIR_Shader_IOBuffer *Buf, VIR_IdList* pIdList)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    if (pIdList)
    {
        ON_ERROR0(VIR_IO_writeUint(Buf, pIdList->count));
        if (pIdList->count > 0)
        {
            ON_ERROR0(VIR_IO_writeBlock(Buf, (gctCHAR *)pIdList->ids, pIdList->count * sizeof(VIR_Id)));
        }
    }
    else
    {
        ON_ERROR0(VIR_IO_writeUint(Buf, VIR_INVALID_ID));
    }
OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_writeUniform(VIR_Shader_IOBuffer *Buf, VIR_Uniform* pUniform)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    ON_ERROR0(VIR_IO_writeBlock(Buf, (gctCHAR *)pUniform, sizeof(VIR_Uniform)));
    if (VIR_Uniform_isSampler(pUniform) && VIR_Uniform_GetResOpBitsArraySize(pUniform) > 0)
    {
        gcmASSERT(pUniform->u0.samplerRes.resOpBitsArray != gcvNULL);
        ON_ERROR0(VIR_IO_writeBlock(Buf, (gctCHAR *)VIR_Uniform_GetResOpBitsArray(pUniform),
                                   sizeof(gctUINT32) * VIR_Uniform_GetResOpBitsArraySize(pUniform)));
    }

OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_writeImageSampler(VIR_Shader_IOBuffer *Buf, VIR_ImageSampler* pImageSampler)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    ON_ERROR0(VIR_IO_writeChar(Buf, pImageSampler->imageNum));
    ON_ERROR0(VIR_IO_writeChar(Buf, (gctCHAR)pImageSampler->isConstantSamplerType));
    ON_ERROR0(VIR_IO_writeUint(Buf, pImageSampler->samplerType));

OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_writeKernelInfo(VIR_Shader_IOBuffer *Buf, VIR_KernelInfo* pKernelInfo)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    if (pKernelInfo != gcvNULL)
    {
        ON_ERROR0(VIR_IO_writeUint(Buf, 0)); /* marker */
        ON_ERROR0(VIR_IO_writeUint(Buf, pKernelInfo->kernelName));
        ON_ERROR0(VIR_IO_writeUint(Buf, pKernelInfo->localMemorySize));
        ON_ERROR0(VIR_IO_writeIdList(Buf, &pKernelInfo->uniformArguments));
        ON_ERROR0(VIR_IO_writeInt(Buf, pKernelInfo->samplerIndex));
        ON_ERROR0(VIR_IO_writeValueList(Buf, &pKernelInfo->imageSamplers, (WRITE_NODE_FP)0));
        ON_ERROR0(VIR_IO_writeValueList(Buf, &pKernelInfo->properties, (WRITE_NODE_FP)0));
        ON_ERROR0(VIR_IO_writeInt(Buf, pKernelInfo->isMain));
    }
    else
    {
        VIR_IO_writeUint(Buf, VIR_INVALID_ID);
    }
OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_writeInstList(VIR_Shader_IOBuffer *Buf, VIR_InstList* pInstList)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_InstIterator iter;
    VIR_Instruction* inst;
    gctUINT idx = 0;

    VIR_InstIterator_Init(&iter, pInstList);
    for(inst = VIR_InstIterator_First(&iter);
        inst != gcvNULL; inst = VIR_InstIterator_Next(&iter))
    {
        ON_ERROR0(VIR_IO_writeUint(Buf, idx));
        ON_ERROR0(VIR_IO_writeInst(Buf, inst));
    }
    /* write end of inst mark */
    VIR_IO_writeUint(Buf, VIR_INVALID_ID);
OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_writeFunction(VIR_Shader_IOBuffer *Buf, VIR_Function* pFunction)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    ON_ERROR0(VIR_IO_writeInt(Buf, pFunction->_lastInstId));
    ON_ERROR0(VIR_IO_writeUint(Buf, pFunction->_labelId));
    ON_ERROR0(VIR_IO_writeInt(Buf, pFunction->funcSym));
    ON_ERROR0(VIR_IO_writeUint(Buf, pFunction->flags));
    ON_ERROR0(VIR_IO_writeUint(Buf, pFunction->maxCallDepth));

    ON_ERROR0(VIR_IO_writeSymTable(Buf, &pFunction->symTable));
    /* debug help */
    VIR_IO_writeUint(Buf, SYMTBL_SIG);

    ON_ERROR0(VIR_IO_writeLabelTable(Buf, &pFunction->labelTable));
    ON_ERROR0(VIR_IO_writeOperandTable(Buf, &pFunction->operandTable));

    ON_ERROR0(VIR_IO_writeIdList(Buf, &pFunction->localVariables));
    ON_ERROR0(VIR_IO_writeIdList(Buf, &pFunction->paramters));
    ON_ERROR0(VIR_IO_writeIdList(Buf, &pFunction->temps));

    ON_ERROR0(VIR_IO_writeKernelInfo(Buf, pFunction->kernelInfo));

    ON_ERROR0(VIR_IO_writeInt(Buf, pFunction->tempIndexStart));
    ON_ERROR0(VIR_IO_writeInt(Buf, pFunction->tempIndexCount));

    /* write instructions */
    ON_ERROR0(VIR_IO_writeInstList(Buf, &pFunction->instList));

    /* debug help */
    ON_ERROR0(VIR_IO_writeUint(Buf, DBUG_SIG));

OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_writeUniformBlock(VIR_Shader_IOBuffer *Buf, VIR_UniformBlock* pUniformBlock)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctUINT i;

    ON_ERROR0(VIR_IO_writeUint(Buf, pUniformBlock->sym));

    ON_ERROR0(VIR_IO_writeUint(Buf, pUniformBlock->flags));
    ON_ERROR0(VIR_IO_writeShort(Buf, pUniformBlock->blockIndex));
    ON_ERROR0(VIR_IO_writeUint(Buf, pUniformBlock->baseAddr));
    ON_ERROR0(VIR_IO_writeUint(Buf, pUniformBlock->blockSize));
    ON_ERROR0(VIR_IO_writeUint(Buf, pUniformBlock->uniformCount));

    for (i=0; i<pUniformBlock->uniformCount; i++)
    {
        gcmASSERT(pUniformBlock->uniforms);
        ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Uniform_GetSymID(pUniformBlock->uniforms[i])));
    }
OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_writeStorageBlock(VIR_Shader_IOBuffer *Buf, VIR_StorageBlock* pStorageBlock)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctUINT i;

    ON_ERROR0(VIR_IO_writeUint(Buf, pStorageBlock->sym));

    ON_ERROR0(VIR_IO_writeUint(Buf, pStorageBlock->flags));
    ON_ERROR0(VIR_IO_writeShort(Buf, pStorageBlock->blockIndex));
    ON_ERROR0(VIR_IO_writeUint(Buf, pStorageBlock->baseAddr));
    ON_ERROR0(VIR_IO_writeUint(Buf, pStorageBlock->blockSize));
    ON_ERROR0(VIR_IO_writeUint(Buf, pStorageBlock->variableCount));

    for (i=0; i<pStorageBlock->variableCount; i++)
    {
        gcmASSERT(pStorageBlock->variables);
        ON_ERROR0(VIR_IO_writeUint(Buf, pStorageBlock->variables[i]));
    }
OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_writeIOBlock(VIR_Shader_IOBuffer *Buf, VIR_IOBlock* pIOBlock)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    ON_ERROR0(VIR_IO_writeUint(Buf, pIOBlock->sym));
    ON_ERROR0(VIR_IO_writeUint(Buf, pIOBlock->flags));
    ON_ERROR0(VIR_IO_writeShort(Buf, pIOBlock->blockIndex));
    ON_ERROR0(VIR_IO_writeInt(Buf, pIOBlock->blockNameLength));
    ON_ERROR0(VIR_IO_writeInt(Buf, pIOBlock->instanceNameLength));
    ON_ERROR0(VIR_IO_writeUint(Buf, pIOBlock->Storage));

OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_writeLabel(VIR_Shader_IOBuffer *Buf, VIR_Label* pLabel)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    ON_ERROR0(VIR_IO_writeUint(Buf, pLabel->index));
    ON_ERROR0(VIR_IO_writeUint(Buf, pLabel->sym));
    ON_ERROR0(VIR_IO_writeUint(Buf, pLabel->defined ? VIR_Inst_GetId(pLabel->defined): VIR_INVALID_ID));
OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_writeConst(VIR_Shader_IOBuffer *Buf, VIR_Const* pConst)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctUINT components      = VIR_GetTypeComponents(pConst->type);
    gctUINT rows            = VIR_GetTypeRows(pConst->type);
    gctUINT i;

    ON_ERROR0(VIR_IO_writeUint(Buf, pConst->index));
    ON_ERROR0(VIR_IO_writeUint(Buf, pConst->type));
    gcmASSERT(VIR_TypeId_isPrimitive(pConst->type));
    gcmASSERT(components*rows > 0 && components*rows < 16);
    /* write constant data one by one */
    for (i=0; i < components*rows; i++)
    {
        switch (VIR_GetTypeComponentType(pConst->type)) {
        case VIR_TYPE_INT8:
        case VIR_TYPE_UINT8:
            ON_ERROR0(VIR_IO_writeChar(Buf, pConst->value.vecVal.u8Value[i]));
            break;
        case VIR_TYPE_INT16:
            ON_ERROR0(VIR_IO_writeShort(Buf, pConst->value.vecVal.i16Value[i]));
            break;
        case VIR_TYPE_UINT16:
            ON_ERROR0(VIR_IO_writeUshort(Buf, pConst->value.vecVal.u16Value[i]));
            break;
        case VIR_TYPE_INT32:
        case VIR_TYPE_BOOLEAN:
            ON_ERROR0(VIR_IO_writeInt(Buf, pConst->value.vecVal.i32Value[i]));
            break;
        case VIR_TYPE_UINT32:
            ON_ERROR0(VIR_IO_writeUint(Buf, pConst->value.vecVal.u32Value[i]));
            break;
        case VIR_TYPE_FLOAT32:
            ON_ERROR0(VIR_IO_writeFloat(Buf, pConst->value.vecVal.f32Value[i]));
            break;
        default:
            gcmASSERT(gcvFALSE);
            break;
        }
    }

OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_writeInst(VIR_Shader_IOBuffer *Buf, VIR_Instruction* pInst)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctUINT val;
    gctUINT i;

    /* Word 1. */
    val = VIR_Inst_GetOpcode(pInst) << 22   |
          VIR_Inst_GetId(pInst) << 2        |
          VIR_Inst_isPrecise(pInst) << 1    |
          VIR_Inst_GetPatched(pInst) ;
    ON_ERROR0(VIR_IO_writeUint(Buf, val));

    /* Word 2. */
    val = VIR_Inst_GetInstType(pInst);
    ON_ERROR0(VIR_IO_writeUint(Buf, val));

    /* Word 3. */
    val = VIR_Inst_GetConditionOp(pInst) << 27  |
          VIR_Inst_GetFlags(pInst) << 24        |
          VIR_Inst_GetSrcNum(pInst) << 21       |
          VIR_Inst_GetThreadMode(pInst) << 18   |
          VIR_Inst_GetParentUseBB(pInst) << 17  |
          VIR_Inst_GetResOpType(pInst) << 11    |
          VIR_Inst_IsPatternRep(pInst) << 10    |
          VIR_Inst_IsLoopInvariant(pInst) << 9 |
          VIR_Inst_IsEndOfBB(pInst) << 8        |
          VIR_Inst_IsUSCUnallocate(pInst) << 7  |
          VIR_Inst_HasSkHp(pInst)         << 6;
    ON_ERROR0(VIR_IO_writeUint(Buf, val));

    ON_ERROR0(VIR_IO_writeUint(Buf, *(gctUINT *)&pInst->sourceLoc));

    if (VIR_Inst_GetDest(pInst))
    {
        ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Operand_GetIndex(VIR_Inst_GetDest(pInst))));
    }
    else
    {
        ON_ERROR0(VIR_IO_writeUint(Buf, VIR_INVALID_ID));
    }

    for (i = 0; i < VIR_Inst_GetSrcNum(pInst); i++)
    {
        VIR_Operand * src = VIR_Inst_GetSource(pInst, i);
        if (src)
        {
            ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Operand_GetIndex(src)));
        }
        else
        {
            ON_ERROR0(VIR_IO_writeUint(Buf, VIR_INVALID_ID));
        }
    }
    /* debug help */
    ON_ERROR0(VIR_IO_writeUint(Buf, INST_SIG));


OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_writeParmPassing(VIR_Shader_IOBuffer *Buf, VIR_ParmPassing* pParmPassing)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctUINT i;

    if (pParmPassing)
    {
        ON_ERROR0(VIR_IO_writeUint(Buf, pParmPassing->argNum));
        for (i=0; i<pParmPassing->argNum; i++)
        {
            gcmASSERT(pParmPassing->args[i]);
            ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Operand_GetIndex(pParmPassing->args[i])));
        }
    }
    else
    {
        ON_ERROR0(VIR_IO_writeUint(Buf, VIR_INVALID_ID));
    }
OnError:
    return errCode;
}
VSC_ErrCode
VIR_IO_writeVarTempRegInfo(VIR_Shader_IOBuffer *Buf, VIR_VarTempRegInfo *pVarInfo)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    ON_ERROR0(VIR_IO_writeUint(Buf, pVarInfo->variable));
    ON_ERROR0(VIR_IO_writeUint(Buf, pVarInfo->streamoutSize));
    ON_ERROR0(VIR_IO_writeInt(Buf, pVarInfo->tempRegCount));
    gcmASSERT(pVarInfo->tempRegTypes == gcvNULL);
OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_writeTransformFeedback(VIR_Shader_IOBuffer *Buf, VIR_TransformFeedback *tfb)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    ON_ERROR0(VIR_IO_writeIdList(Buf, tfb->varyings));
    ON_ERROR0(VIR_IO_writeInt(Buf, tfb->bufferMode));
    ON_ERROR0(VIR_IO_writeUint(Buf, tfb->stateUniformId));
    if (tfb->varRegInfos)
    {
        ON_ERROR0(VIR_IO_writeUint(Buf, 0)); /* marker */
        ON_ERROR0(VIR_IO_writeValueList(Buf, tfb->varRegInfos,
                                    (WRITE_NODE_FP)VIR_IO_writeVarTempRegInfo));
    }
    else
    {
        ON_ERROR0(VIR_IO_writeUint(Buf, VIR_INVALID_ID));
    }
    ON_ERROR0(VIR_IO_writeUint(Buf, tfb->totalSize));
    ON_ERROR0(VIR_IO_writeInt(Buf, tfb->shaderTempCount));
    if (tfb->bufferMode == VIR_FEEDBACK_INTERLEAVED)
    {
        ON_ERROR0(VIR_IO_writeUint(Buf, tfb->feedbackBuffer.interleavedBufUniformId));
    }
    else
    {
        gctINT i;
        /* VIR_FEEDBACK_SEPARATE mode */
        for (i=0; i<tfb->shaderTempCount; i++)
        {
            ON_ERROR0(VIR_IO_writeUint(Buf, tfb->feedbackBuffer.separateBufUniformIds[i]));
        }
    }
OnError:
    return errCode;
}


VSC_ErrCode
VIR_IO_writeOperandList(
    VIR_Shader_IOBuffer *Buf,
    VIR_OperandList* pOperandList)
{
    VSC_ErrCode       errCode = VSC_ERR_NONE;
    VIR_OperandList * ptr     = pOperandList;

    while (ptr)
    {
        ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Operand_GetIndex(ptr->value)));
        ptr = ptr->next;
    }
    ON_ERROR0(VIR_IO_writeUint(Buf, VIR_INVALID_ID));
OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_writePhiOperandArray(
    VIR_Shader_IOBuffer *   Buf,
    VIR_PhiOperandArray *   pPhiOperandArray
    )
{
    VSC_ErrCode      errCode = VSC_ERR_NONE;
    gctUINT          i;
    gctUINT          sz;

    sz = VIR_PhiOperandArray_ComputeSize(pPhiOperandArray->count);

    ON_ERROR0(VIR_IO_ReserveBytes(Buf, sz));

    ON_ERROR0(VIR_IO_writeUint(Buf, pPhiOperandArray->count));
    if (pPhiOperandArray->count > 0)
    {
        for (i=0; i < pPhiOperandArray->count; i++)
        {
            VIR_PhiOperand* phiOperand = VIR_PhiOperandArray_GetNthOperand(pPhiOperandArray, i);
            ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Operand_GetIndex(phiOperand->value)));
            ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Label_GetId(phiOperand->label)));
            ON_ERROR0(VIR_IO_writeUint(Buf, phiOperand->flags));
        }
    }
OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_writeValueList(
    VIR_Shader_IOBuffer *Buf,
    VIR_ValueList* pValueList,
    WRITE_NODE_FP fp)
{
    VSC_ErrCode      errCode = VSC_ERR_NONE;
    gctUINT          i;
    gctUINT          sz;

    sz = 2*sizeof(gctUINT) + (pValueList->count * pValueList->elemSize);

    ON_ERROR0(VIR_IO_ReserveBytes(Buf, sz));

    ON_ERROR0(VIR_IO_writeUint(Buf, pValueList->elemSize));
    ON_ERROR0(VIR_IO_writeUint(Buf, pValueList->count));
    if (pValueList->count > 0)
    {
        gcmASSERT(pValueList->values != gcvNULL);

        if (fp)
        {
            for (i=0; i < pValueList->count; i++)
            {
                ON_ERROR0(fp(Buf, pValueList->values + (i * pValueList->elemSize)));
            }
        }
        else
        {
            ON_ERROR0(VIR_IO_writeBlock(Buf, pValueList->values,
                pValueList->count * pValueList->elemSize));
        }
    }
    /* debug help */
    ON_ERROR0(VIR_IO_writeUint(Buf, DBUG_SIG));


OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_writeOperand(VIR_Shader_IOBuffer *Buf, VIR_Operand* pOperand)
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    gctUINT         val;
    VIR_OperandKind opndKind = (VIR_OperandKind)VIR_Operand_GetOpKind(pOperand);

    /* operand header */
    /* debug help */
    ON_ERROR0(VIR_IO_writeUint(Buf, DBUG_SIG));
    val = *(gctUINT*)&pOperand->header;
    ON_ERROR0(VIR_IO_writeUint(Buf, val));

    if (opndKind != VIR_OPND_TEXLDPARM)
    {
        /* Word 1. */
        val = VIR_Operand_GetTypeId(pOperand);
        ON_ERROR0(VIR_IO_writeUint(Buf, val));

        /* Word 2. */
        val = VIR_Operand_GetSwizzle(pOperand) << 24    |
              VIR_Operand_GetPrecision(pOperand) << 21  |
              VIR_Operand_isBigEndian(pOperand) << 20   |
              VIR_Operand_GetModOrder(pOperand) << 18;
        ON_ERROR0(VIR_IO_writeUint(Buf, val));

        /* Word 3. */
        val = VIR_Operand_GetHwShift(pOperand) << 30    |
              VIR_Operand_GetHwRegId(pOperand) << 20    |
              VIR_Operand_GetHIHwRegId(pOperand) << 10  |
              VIR_Operand_GetHIHwShift(pOperand) << 8   |
              VIR_Operand_GetLShift(pOperand) << 5;
        ON_ERROR0(VIR_IO_writeUint(Buf, val));

        ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Operand_GetFlags(pOperand)));

        /* u1 */
        switch(VIR_Operand_GetOpKind(pOperand))
        {
        case VIR_OPND_NONE:
        case VIR_OPND_UNDEF:
        case VIR_OPND_UNUSED:
            break;
        case VIR_OPND_EVIS_MODIFIER:
            ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Operand_GetEvisModifier(pOperand)));
            break;
        case VIR_OPND_CONST:
            ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Operand_GetConstId(pOperand)));
            break;
        case VIR_OPND_PARAMETERS:
            {
                VIR_ParmPassing *parm = VIR_Operand_GetParameters(pOperand);
                ON_ERROR0(VIR_IO_writeParmPassing(Buf, parm));
            }
            break;
        case VIR_OPND_LABEL:
            ON_ERROR0(VIR_IO_writeUint(Buf,
                VIR_Label_GetId(VIR_Operand_GetLabel(pOperand))));
            break;
        case VIR_OPND_FUNCTION:
            ON_ERROR0(VIR_IO_writeUint(Buf,
                VIR_Function_GetSymId(VIR_Operand_GetFunction(pOperand))));
            break;
        case VIR_OPND_SAMPLER_INDEXING:
        case VIR_OPND_SYMBOL:
        case VIR_OPND_ADDRESS_OF:
        case VIR_OPND_VEC_INDEXING:
            ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Operand_GetSymbolId_(pOperand)));
            break;
        case VIR_OPND_NAME:
            ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Operand_GetNameId(pOperand)));
            break;
        case VIR_OPND_INTRINSIC:
            ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Operand_GetIntrinsicKind(pOperand)));
            break;
        case VIR_OPND_FIELD:
            ON_ERROR0(VIR_IO_writeUint(Buf,
                VIR_Operand_GetIndex(VIR_Operand_GetFieldBase(pOperand))));
    /*
            sym = VIR_Function_GetSymFromId(func,
                VIR_Operand_GetFieldId(Operand));
                */
            break;
        case VIR_OPND_ARRAY:
            ON_ERROR0(VIR_IO_writeUint(Buf,
                VIR_Operand_GetIndex(VIR_Operand_GetArrayBase(pOperand))));

            /*
            operandList = VIR_Operand_GetArrayIndex(Operand);
            */
            break;
        case VIR_OPND_PHI:
            ON_ERROR0(VIR_IO_writePhiOperandArray(Buf, VIR_Operand_GetPhiOperands(pOperand)));
            break;
        case VIR_OPND_SIZEOF:
        case VIR_OPND_OFFSETOF:
            gcmASSERT(gcvFALSE);  /* need to refine the definition of the operand */
            break;
        case VIR_OPND_IMMEDIATE:
        default:
            /* write bits as uint */
            ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Operand_GetImmediateUint(pOperand)));
            break;
        }
        /* u2 */
        switch(VIR_Operand_GetOpKind(pOperand))
        {
        case VIR_OPND_FIELD:
            ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Operand_GetFieldId(pOperand)));
            break;
        case VIR_OPND_ARRAY:
            ON_ERROR0(VIR_IO_writeOperandList(Buf, VIR_Operand_GetArrayIndex(pOperand)));
            break;
        case VIR_OPND_VEC_INDEXING:
            ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Operand_GetVecIndexSymId(pOperand)));
            break;
        default:
            ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Operand_GetFieldId(pOperand)));
            break;
        }
        /* u3 */
        {
            ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Operand_GetMatrixStride(pOperand)));
            ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Operand_GetLayoutQual(pOperand)));
        }
    }
    else
    {
        gctUINT i;
        VIR_Operand * opnd;
        for (i = 0; i <VIR_TEXLDMODIFIER_COUNT; i++)
        {
            opnd = VIR_Operand_GetTexldModifier(pOperand, i);
            if (opnd)
            {
                ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Operand_GetIndex(opnd)));
            }
            else
            {
                ON_ERROR0(VIR_IO_writeUint(Buf, VIR_INVALID_ID));
            }
        }
    }
OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_writeSymbol(VIR_Shader_IOBuffer *Buf, VIR_Symbol* pSymbol)
{
    VSC_ErrCode       errCode = VSC_ERR_NONE;
    VIR_SymbolKind    symKind = VIR_Symbol_GetKind(pSymbol);

    /* write first two words */
    ON_ERROR0(VIR_IO_writeUint(Buf, ((gctUINT*)pSymbol)[0]));
    ON_ERROR0(VIR_IO_writeUint(Buf, ((gctUINT*)pSymbol)[1]));

    ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Symbol_GetTypeId(pSymbol)));
    ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Symbol_GetFixedTypeId(pSymbol)));
    ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Symbol_GetFlags(pSymbol)));
    ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Symbol_GetFlagsExt(pSymbol)));
    ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Symbol_GetIndex(pSymbol)));
    ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Symbol_GetIOBlockIndex(pSymbol)));

    ON_ERROR0(VIR_IO_writeBlock(Buf, (gctCHAR*)&pSymbol->layout, sizeof(pSymbol->layout)));

    /* u0 */
    if (isSymLocal(pSymbol))
    {
        ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Function_GetSymId(VIR_Symbol_GetHostFunction(pSymbol))));
    }

    /* u1 */
    ON_ERROR0(VIR_IO_writeUint(Buf, pSymbol->u1.vregIndex));

    /* u2 */
    switch(symKind)
    {
    case VIR_SYM_UNIFORM:
        ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Uniform_GetID(VIR_Symbol_GetUniform(pSymbol))));
        ON_ERROR0(VIR_IO_writeUniform(Buf, VIR_Symbol_GetUniform(pSymbol)));
        break;
    case VIR_SYM_SAMPLER:
        ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Uniform_GetID(VIR_Symbol_GetSampler(pSymbol))));
        ON_ERROR0(VIR_IO_writeUniform(Buf, VIR_Symbol_GetSampler(pSymbol)));
        break;
    case VIR_SYM_SAMPLER_T:
        ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Uniform_GetID(VIR_Symbol_GetSamplerT(pSymbol))));
        ON_ERROR0(VIR_IO_writeUniform(Buf, VIR_Symbol_GetSamplerT(pSymbol)));
        break;
    case VIR_SYM_IMAGE:
        ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Uniform_GetID(VIR_Symbol_GetImage(pSymbol))));
        ON_ERROR0(VIR_IO_writeUniform(Buf, VIR_Symbol_GetImage(pSymbol)));
        break;
    case VIR_SYM_IMAGE_T:
        ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Uniform_GetID(VIR_Symbol_GetImage(pSymbol))));
        ON_ERROR0(VIR_IO_writeUniform(Buf, VIR_Symbol_GetImageT(pSymbol)));
        break;
    case VIR_SYM_VARIABLE:
    case VIR_SYM_TEXTURE:
        ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Symbol_GetVariableVregIndex(pSymbol)));
        break;
    case VIR_SYM_SBO:
        ON_ERROR0(VIR_IO_writeUint(Buf, VIR_SBO_GetBlockIndex(VIR_Symbol_GetSBO(pSymbol))));
        ON_ERROR0(VIR_IO_writeStorageBlock(Buf, VIR_Symbol_GetSBO(pSymbol)));
        break;
    case VIR_SYM_UBO:
        ON_ERROR0(VIR_IO_writeUint(Buf, VIR_UBO_GetBlockIndex(VIR_Symbol_GetUBO(pSymbol))));
        ON_ERROR0(VIR_IO_writeUniformBlock(Buf, VIR_Symbol_GetUBO(pSymbol)));
        break;
    case VIR_SYM_IOBLOCK:
        ON_ERROR0(VIR_IO_writeUint(Buf, VIR_IOBLOCK_GetBlockIndex(VIR_Symbol_GetIOB(pSymbol))));
        ON_ERROR0(VIR_IO_writeIOBlock(Buf, VIR_Symbol_GetIOB(pSymbol)));
        break;
    case VIR_SYM_FUNCTION:
        /* delay to end of shader to write function contents */
        break;
    case VIR_SYM_FIELD:
        if (VIR_Symbol_GetFieldInfo(pSymbol) != gcvNULL)
        {
            VIR_FieldInfo * fi = VIR_Symbol_GetFieldInfo(pSymbol);
            ON_ERROR0(VIR_IO_writeUint(Buf, 0)); /* marker */
            ON_ERROR0(VIR_IO_writeBlock(Buf, (gctCHAR *)fi, sizeof(VIR_FieldInfo)));
        }
        else
        {
            ON_ERROR0(VIR_IO_writeUint(Buf, VIR_INVALID_ID));
        }
        break;
    case VIR_SYM_TYPE:
    case VIR_SYM_LABEL:
    case VIR_SYM_VIRREG:
    case VIR_SYM_CONST:
    case VIR_SYM_UNKNOWN:
    default:
        ON_ERROR0(VIR_IO_writeUint(Buf, pSymbol->u2.varSymId));
        break;
    }
    /* u3 */
    switch(symKind)
    {
    case VIR_SYM_FUNCTION:
        ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Symbol_GetFuncMangleName(pSymbol)));
        break;
    case VIR_SYM_FIELD:
        ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Symbol_GetStructTypeId(pSymbol)));
        break;
    default:
        if (isSymCombinedSampler(pSymbol))
        {
            ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Symbol_GetSeparateSamplerId(pSymbol)));
            ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Symbol_GetSeparateSamplerFuncId(pSymbol)));
        }
        else
            ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Symbol_GetOffsetInVar(pSymbol)));
        break;
    }
    /* u4 */
    if (isSymCombinedSampler(pSymbol))
    {
        ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Symbol_GetSeparateImageId(pSymbol)));
        ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Symbol_GetSeparateImageFuncId(pSymbol)));
    }
    else if (VIR_Symbol_isParamVirReg(pSymbol))
         ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Symbol_GetParamFuncSymId(pSymbol)));

    /* u5 */
    ON_ERROR0(VIR_IO_writeInt(Buf, VIR_Symbol_GetIndexRange(pSymbol)));

    /* u6 */
    ON_ERROR0(VIR_IO_writeUint(Buf, VIR_Symbol_GetFirstElementId(pSymbol)));

    /* debug help */
    ON_ERROR0(VIR_IO_writeUint(Buf, SYMB_SIG));

OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_writeType(VIR_Shader_IOBuffer *Buf, VIR_Type* pType)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    ON_ERROR0(VIR_IO_writeBlock(Buf, (gctCHAR *)pType, sizeof(VIR_Type)));
    switch (pType->_kind) {
    case VIR_TY_STRUCT:
    case VIR_TY_ENUM:
        /* field id list */
        ON_ERROR0(VIR_IO_writeIdList(Buf, VIR_Type_GetFields(pType)));
        break;
    case VIR_TY_FUNCTION:
        /* parameter id list */
        ON_ERROR0(VIR_IO_writeIdList(Buf, VIR_Type_GetParameters(pType)));
        break;
    case VIR_TY_POINTER:
    case VIR_TY_ARRAY:
    default:
        break;
    }

OnError:
    return errCode;
}

gctUINT VIR_Shader_EstimateSize(VIR_Shader* pShader)
{
    gctUINT sz = 0;
    /* symbol table size */
    sz = 32*1024;
    return sz;
}

VSC_ErrCode
VIR_Shader_IOBuffer_Initialize(VIR_Shader_IOBuffer *Buf)
{
    Buf->ioBuffer = gcvNULL;
    Buf->shader = gcvNULL;
    QUEUE_INITIALIZE(&Buf->localSymbolList);

    return VSC_ERR_NONE;
}

VSC_ErrCode
VIR_Shader_IOBuffer_Finalize(VIR_Shader_IOBuffer *Buf)
{
    QUEUE_FINALIZE(&Buf->localSymbolList);

    return VSC_ERR_NONE;
}

/* save the shader binary to a buffer allocated by compiler */
VSC_ErrCode
VIR_Shader_Save(VIR_Shader* pShader, VIR_Shader_IOBuffer *Buf)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gcmASSERT (Buf != gcvNULL);

    {
        VSC_IO_BUFFER       ioBuf = {0, 0, 0};

        VIR_IO_Init(Buf, &ioBuf, pShader, VIR_Shader_EstimateSize(pShader), gcvFALSE);
        ON_ERROR0(VIR_IO_writeShader(Buf, pShader));
    }
OnError:
    return errCode;
}

/* save the shader binary to a buffer allocated by user, the BufSz must be acquired
 * by VIR_Shader_QueryBinarySize() */
VSC_ErrCode
VIR_Shader_Save2Buffer(VIR_Shader* pShader, gctCHAR *Buffer, gctUINT BufSz)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER       ioBuf = {0, 0, 0};
    VIR_Shader_IOBuffer buf;

    VIR_Shader_IOBuffer_Initialize(&buf);

    buf.ioBuffer    = &ioBuf;
    buf.shader      = gcvNULL;

    gcmASSERT (Buffer != gcvNULL);
    {
        ioBuf.allocatedBytes    = BufSz;
        ioBuf.buffer            = Buffer;
        ioBuf.curPos            = 0;
        buf.shader         = pShader;
        ON_ERROR0(VIR_IO_writeShader(&buf, pShader));
    }

OnError:
    VIR_Shader_IOBuffer_Finalize(&buf);
    return errCode;
}

VSC_ErrCode
VIR_Shader_QueryBinarySize(VIR_Shader* pShader, gctUINT *BinarySz)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_Shader_IOBuffer buf;

    VIR_Shader_IOBuffer_Initialize(&buf);

    gcmASSERT (BinarySz != gcvNULL);

    {
        VSC_IO_BUFFER       ioBuf = {0, 0, 0};

        VIR_IO_Init(&buf, &ioBuf, pShader, VIR_Shader_EstimateSize(pShader), gcvTRUE);
        ON_ERROR0(VIR_IO_writeShader(&buf, pShader));
        *BinarySz = buf.ioBuffer->allocatedBytes;
    }

OnError:
    VIR_Shader_IOBuffer_Finalize(&buf);
    return errCode;
}

VSC_ErrCode
VIR_Shader_Read(VIR_Shader* pShader, VIR_Shader_IOBuffer *Buf, gctUINT messageLevel)
{
    gcmASSERT (Buf != gcvNULL);

    return (VIR_IO_readShader(Buf, pShader, messageLevel));
}

gctUINT
VIR_IO_getMagicNum(void)
{
    gctUINT   magicNum;
    gctUINT   buf[16];
    buf[0]  = sizeof(VIR_Shader);
    buf[1]  = sizeof(VIR_Uniform);
    buf[2]  = sizeof(VIR_Function);
    buf[3]  = sizeof(VIR_UniformBlock);
    buf[4]  = sizeof(VIR_StorageBlock);
    buf[5]  = sizeof(VIR_IOBlock);
    buf[6]  = sizeof(VIR_Label);
    buf[7]  = sizeof(VIR_Const);
    buf[8]  = sizeof(VIR_Instruction);
    buf[9]  = sizeof(VIR_Operand);
    buf[10] = sizeof(VIR_ParmPassing);
    buf[11] = sizeof(VIR_OperandList);
    buf[12] = sizeof(VIR_SymbolList);
    buf[13] = sizeof(VIR_Symbol);
    buf[14] = sizeof(VIR_Dumper);
    buf[15] = sizeof(VIR_Type);

    magicNum = vscEvaluateCRC32(buf, 16 * sizeof(gctUINT));
    return magicNum;
}

VSC_ErrCode
VIR_IO_writeShader(VIR_Shader_IOBuffer *buf, VIR_Shader* pShader)
{
    VSC_ErrCode errCode;
    gctUINT i;
    gctUINT magicNum;
    magicNum = VIR_IO_getMagicNum();
    VIR_IO_writeInt(buf, SHDR_SIG);
    VIR_IO_writeInt(buf, gcdVIR_SHADER_BINARY_FILE_VERSION);
    VIR_IO_writeUint(buf, magicNum);
    VIR_IO_writeInt(buf, gcGetHWCaps()->chipModel);
    VIR_IO_writeInt(buf, gcGetHWCaps()->chipRevision);

    VIR_IO_writeInt(buf, pShader->clientApiVersion);
    VIR_IO_writeUint(buf, pShader->_id);
    VIR_IO_writeUint(buf, pShader->_constVectorId);
    VIR_IO_writeUint(buf, pShader->_dummyUniformCount);
    VIR_IO_writeUint(buf, pShader->_orgTempCount);
    VIR_IO_writeUint(buf, pShader->_tempRegCount);
    VIR_IO_writeUint(buf, pShader->_anonymousNameId);
    VIR_IO_writeInt(buf, pShader->shLevel);
    VIR_IO_writeInt(buf, pShader->shaderKind);
    VIR_IO_writeInt(buf, pShader->flags);
    VIR_IO_writeInt(buf, pShader->flagsExt1);
    VIR_IO_writeUint(buf, pShader->compilerVersion[0]);
    VIR_IO_writeUint(buf, pShader->compilerVersion[1]);
    VIR_IO_writeInt(buf, pShader->constUniformBlockIndex);
    VIR_IO_writeInt(buf, pShader->defaultUniformBlockIndex);
    VIR_IO_writeUint(buf, pShader->maxKernelFunctionArgs);
    VIR_IO_writeUint(buf, pShader->privateMemorySize);
    VIR_IO_writeUint(buf, pShader->localMemorySize);

    VIR_IO_writeInt(buf, pShader->constUBOSize);
    if (pShader->constUBOSize)
    {
        errCode = VIR_IO_writeBlock(buf, (gctCHAR *)pShader->constUBOData, pShader->constUBOSize*16);
        ON_ERROR(errCode, "Fail to write constUBOData, sz: %d.", pShader->constUBOSize*16);
    }

    VIR_IO_writeUint(buf, pShader->constantMemorySize);
    if (pShader->constantMemoryBuffer > 0)
    {
        errCode = VIR_IO_writeBlock(buf, (gctCHAR *)pShader->constantMemoryBuffer, pShader->constantMemorySize);
        ON_ERROR(errCode, "Fail to write constantMemoryBuffer, sz: %d.", pShader->constantMemorySize);
    }

    errCode = VIR_IO_writeIdList(buf, &pShader->attributes);
    ON_ERROR(errCode, "Fail to write attributes id list.");

    errCode = VIR_IO_writeIdList(buf, &pShader->outputs);
    ON_ERROR(errCode, "Fail to write outputs id list.");

    errCode = VIR_IO_writeIdList(buf, &pShader->outputVregs);
    ON_ERROR(errCode, "Fail to write outputVregs id list.");

    errCode = VIR_IO_writeIdList(buf, &pShader->perpatchInput);
    ON_ERROR(errCode, "Fail to write perpatchInput id list.");

    errCode = VIR_IO_writeIdList(buf, &pShader->perpatchOutput);
    ON_ERROR(errCode, "Fail to write perpatchOutput id list.");

    errCode = VIR_IO_writeIdList(buf, &pShader->perpatchOutputVregs);
    ON_ERROR(errCode, "Fail to write perpatchOutputVregs id list.");

    errCode = VIR_IO_writeIdList(buf, &pShader->buffers);
    ON_ERROR(errCode, "Fail to write buffers id list.");

    errCode = VIR_IO_writeIdList(buf, &pShader->uniforms);
    ON_ERROR(errCode, "Fail to write uniforms id list.");

    VIR_IO_writeUint(buf, pShader->uniformVectorCount);
    VIR_IO_writeInt(buf, pShader->samplerIndex);
    VIR_IO_writeUint(buf, pShader->baseSamplerId);
    VIR_IO_writeInt(buf, pShader->samplerBaseOffset);

    /* save layout info */
    switch (pShader->shaderKind)
    {
    case VIR_SHADER_COMPUTE:
        VIR_IO_writeBlock(buf, (gctCHAR *)&pShader->shaderLayout.compute, sizeof(VIR_ComputeLayout));
        break;
    case VIR_SHADER_TESSELLATION_CONTROL:
        VIR_IO_writeBlock(buf, (gctCHAR *)&pShader->shaderLayout.tcs, sizeof(VIR_TCSLayout));
        break;
    case VIR_SHADER_TESSELLATION_EVALUATION:
        VIR_IO_writeBlock(buf, (gctCHAR *)&pShader->shaderLayout.tes, sizeof(VIR_TESLayout));
        break;
    case VIR_SHADER_GEOMETRY:
        VIR_IO_writeBlock(buf, (gctCHAR *)&pShader->shaderLayout.geo, sizeof(VIR_GEOLayout));
        break;
    default:
        break;
    }

    errCode = VIR_IO_writeIdList(buf, &pShader->variables);
    ON_ERROR(errCode, "Fail to write variables id list.");

    errCode = VIR_IO_writeIdList(buf, &pShader->sharedVariables);
    ON_ERROR(errCode, "Fail to write shared variables id list.");

    errCode = VIR_IO_writeIdList(buf, &pShader->uniformBlocks);
    ON_ERROR(errCode, "Fail to write uniformBlocks id list.");

    errCode = VIR_IO_writeIdList(buf, &pShader->storageBlocks);
    ON_ERROR(errCode, "Fail to write storageBlocks id list.");

    errCode = VIR_IO_writeIdList(buf, &pShader->ioBlocks);
    ON_ERROR(errCode, "Fail to write ioBlocks id list.");

    errCode = VIR_IO_writeIdList(buf, &pShader->moduleProcesses);
    ON_ERROR(errCode, "Fail to write module process id list.");

    /* LTC info */
    VIR_IO_writeInt(buf, pShader->ltcUniformCount);
    if (pShader->ltcUniformCount)
    {
        VIR_IO_writeUint(buf, pShader->ltcUniformBegin);
        VIR_IO_writeUint(buf, pShader->ltcInstructionCount);
        for (i = 0; i < pShader->ltcInstructionCount; i++)
        {
            VIR_IO_writeInt(buf, pShader->ltcCodeUniformIndex[i]);
        }
        for (i = 0; i < pShader->ltcInstructionCount; i++)
        {
            errCode = VIR_IO_writeInst(buf, &pShader->ltcExpressions[i]);
            ON_ERROR(errCode, "Fail to write ltcExpressions[i].", i);
        }
    }

    VIR_IO_writeUint(buf, pShader->optimizationOption);

    /* Source code string */
    VIR_IO_writeUint(buf, pShader->sourceLength);
    if (pShader->sourceLength)
    {
        /* encode the source string */
        for (i = 0; i < pShader->sourceLength; i++)
        {
            pShader->source[i] ^= SOURCE_ENCODE_CHAR;
        }
        /* write the string */
        errCode = VIR_IO_writeBlock(buf, pShader->source, pShader->sourceLength);

        /* decode the string */
        for (i = 0; i < pShader->sourceLength; i++)
        {
            pShader->source[i] ^= SOURCE_ENCODE_CHAR;
        }
        ON_ERROR(errCode, "Fail to write source.");
    }

    VIR_IO_writeUint(buf, pShader->replaceIndex);
    VIR_IO_writeBlock(buf, (gctCHAR *)pShader->memoryAccessFlag, sizeof(pShader->memoryAccessFlag));
    VIR_IO_writeBlock(buf, (gctCHAR *)pShader->flowControlFlag, sizeof(pShader->flowControlFlag));
    VIR_IO_writeBlock(buf, (gctCHAR *)pShader->texldFlag, sizeof(pShader->texldFlag));
    VIR_IO_writeUint(buf, pShader->vsPositionZDependsOnW);
    VIR_IO_writeUint(buf, pShader->psHasDiscard);
    VIR_IO_writeUint(buf, pShader->useEarlyFragTest);
    VIR_IO_writeUint(buf, pShader->hasDsx);
    VIR_IO_writeUint(buf, pShader->hasDsy);
    VIR_IO_writeUint(buf, pShader->useLastFragData);
    VIR_IO_writeUint(buf, pShader->__IsDual16Shader);
    VIR_IO_writeUint(buf, pShader->__IsMasterDual16Shader);
    VIR_IO_writeUint(buf, pShader->packUnifiedSampler);
    VIR_IO_writeUint(buf, pShader->fullUnifiedUniforms);
    VIR_IO_writeUint(buf, pShader->needToAdjustSamplerPhysical);
    VIR_IO_writeUint(buf, pShader->_enableDefaultUBO);

    errCode = VIR_IO_writeStringTable(buf, &pShader->stringTable);
    ON_ERROR(errCode, "Fail to write string table.");
    /* debug help */
    VIR_IO_writeUint(buf, STRTBL_SIG);

    errCode = VIR_IO_writeTypeTable(buf, &pShader->typeTable);
    ON_ERROR(errCode, "Fail to write type table.");

    /* debug help */
    VIR_IO_writeUint(buf, TYTBL_SIG);

    errCode = VIR_IO_writeConstTable(buf, &pShader->constTable);
    ON_ERROR(errCode, "Fail to write const table.");

    errCode = VIR_IO_writeSymTable(buf, &pShader->symTable);
    ON_ERROR(errCode, "Fail to write sym table.");

    /* debug help */
    VIR_IO_writeUint(buf, SYMTBL_SIG);

    /* Transform feedback varyings */
    errCode = VIR_IO_writeTransformFeedback(buf, &pShader->transformFeedback);
    ON_ERROR(errCode, "Fail to write transformFeedback.");

    /* write functions */
    {
        VIR_FunctionNode*  pFuncNode  = gcvNULL;
        VIR_FuncIterator   funcIter;
        int                count = 0;

        /* write function symId first */
        VIR_FuncIterator_Init(&funcIter, &pShader->functions);
        pFuncNode = VIR_FuncIterator_First(&funcIter);

        for(; pFuncNode != gcvNULL; pFuncNode = VIR_FuncIterator_Next(&funcIter))
        {
            VIR_Function *pFunc = pFuncNode->function;

            VIR_IO_writeUint(buf, VIR_Function_GetSymId(pFunc));
        }
        VIR_IO_writeInt(buf, VIR_INVALID_ID);

        /* write function body now */
        VIR_FuncIterator_Init(&funcIter, &pShader->functions);
        pFuncNode = VIR_FuncIterator_First(&funcIter);

        for(; pFuncNode != gcvNULL; pFuncNode = VIR_FuncIterator_Next(&funcIter))
        {
            VIR_Function *pFunc = pFuncNode->function;

            VIR_IO_writeUint(buf, FUNC_SIG);
            VIR_IO_writeUint(buf, VIR_Function_GetSymId(pFunc));

            errCode = VIR_IO_writeFunction(buf, pFunc);
            ON_ERROR(errCode, "Fail to write function.");
            VIR_IO_writeUint(buf, ENDF_SIG);
            count++;
        }
        /* save function count */
        VIR_IO_writeInt(buf, count);
    }

    VIR_IO_writeUint(buf, pShader->RAEnabled);
    VIR_IO_writeUint(buf, pShader->hwRegAllocated);
    VIR_IO_writeUint(buf, pShader->hwRegWatermark);
    VIR_IO_writeUint(buf, pShader->constRegAllocated);
    VIR_IO_writeUint(buf, pShader->remapRegStart);
    VIR_IO_writeUint(buf, pShader->remapChannelStart);
    VIR_IO_writeUint(buf, pShader->sampleMaskIdRegStart);
    VIR_IO_writeUint(buf, pShader->sampleMaskIdChannelStart);
    VIR_IO_writeUint(buf, pShader->hasRegisterSpill);
    VIR_IO_writeUint(buf, pShader->llSlotForSpillVidmem);
    VIR_IO_writeUint(buf, pShader->hasCRegSpill);
    VIR_IO_writeUint(buf, pShader->useHwManagedLS);

    VIR_IO_writeBlock(buf, (gctCHAR *)pShader->psInputPosCompValid, sizeof(pShader->psInputPosCompValid));

    VIR_IO_writeBlock(buf, (gctCHAR *)pShader->psInputPCCompValid, sizeof(pShader->psInputPCCompValid));

    VIR_IO_writeUint(buf, pShader->inLinkedShaderStage);
    VIR_IO_writeUint(buf, pShader->outLinkedShaderStage);
    VIR_IO_writeUint(buf, VIR_Shader_GetKernelNameId(pShader));
    VIR_IO_writeInt(buf, ENDS_SIG);

OnError:
    return errCode;

}

/*************************************************************************
 *    V I R   R E A D
 *************************************************************************/
VSC_ErrCode
VIR_IO_readInt(VIR_Shader_IOBuffer *Buf, gctINT * Val)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_readInt(Buf->ioBuffer, Val);
    return errCode;
}

VSC_ErrCode
VIR_IO_readUint(VIR_Shader_IOBuffer *Buf, gctUINT * Val)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_readUint(Buf->ioBuffer, Val);
    return errCode;
}

VSC_ErrCode
VIR_IO_readShort(VIR_Shader_IOBuffer *Buf, gctINT16 * Val)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_readShort(Buf->ioBuffer, Val);
    return errCode;
}

VSC_ErrCode
VIR_IO_readUshort(VIR_Shader_IOBuffer *Buf, gctUINT16 * Val)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_readUshort(Buf->ioBuffer, Val);
    return errCode;
}

VSC_ErrCode
VIR_IO_readFloat(VIR_Shader_IOBuffer *Buf, gctFLOAT * Val)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_readFloat(Buf->ioBuffer, Val);
    return errCode;
}

VSC_ErrCode
VIR_IO_readChar(VIR_Shader_IOBuffer *Buf, gctCHAR * Val)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_readChar(Buf->ioBuffer, Val);
    return errCode;
}

VSC_ErrCode
VIR_IO_readBlock(VIR_Shader_IOBuffer *Buf, gctCHAR *Val, gctUINT Sz)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_readBlock(Buf->ioBuffer, Val, Sz);
    return errCode;
}

VSC_ErrCode
VIR_IO_readBlockTable(VIR_Shader_IOBuffer * Buf,
                      VSC_BLOCK_TABLE *     pBlockTbl,
                      READ_NODE_FP          fp)
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    VSC_IO_BUFFER   *ioBuffer = Buf->ioBuffer;
    VSC_BLOCK_TABLE blkTbl;
    gctUINT         usedBytes;
    VIR_Id          startIdToRead;
    gctUINT         startToReadBlockIndex;
    gctUINT         startToReadBlockOffset;
    gctUINT         curBlockIdx;
    gctUINT         nextOffsetInCurBlock;
    gctUINT64       tempCount;

    ON_ERROR0(VIR_IO_readUint(Buf, (gctUINT*)&startIdToRead));
    startToReadBlockIndex = BT_GET_BLOCK_INDEX(pBlockTbl, startIdToRead);
    startToReadBlockOffset = BT_GET_BLOCK_OFFSET(pBlockTbl, startIdToRead);

    ON_ERROR0(VIR_IO_readUint(Buf, &curBlockIdx));
    blkTbl.curBlockIdx = curBlockIdx;
    ON_ERROR0(VIR_IO_readUint(Buf, &nextOffsetInCurBlock));
    blkTbl.nextOffsetInCurBlock = nextOffsetInCurBlock;
    ON_ERROR0(VIR_IO_readUint(Buf, (gctUINT*)&blkTbl.flag));
    ON_ERROR0(VIR_IO_readUint(Buf, &blkTbl.entrySize));
    ON_ERROR0(VIR_IO_readUint(Buf, &blkTbl.blockSize));

    gcmASSERT(blkTbl.blockSize == pBlockTbl->blockSize &&
              blkTbl.entrySize == pBlockTbl->entrySize &&
              blkTbl.flag == pBlockTbl->flag);
    /* get used bolck table size */
    tempCount = (gctUINT64)blkTbl.curBlockIdx * (gctUINT64)blkTbl.blockSize +
                (gctUINT64)blkTbl.nextOffsetInCurBlock;
    if (tempCount < gcvMAXUINT32)
    {
        usedBytes = vscBT_GetUsedSize(&blkTbl);
    }
    else
    {
        return VSC_ERR_OUT_OF_BOUNDS;
    }
    if (!fp)
    {
        errCode = VIR_IO_CheckBounds(Buf, usedBytes);
    }
    if (errCode == VSC_ERR_NONE)
    {
        gctUINT i;
        gctUINT j;
        /* set the block table to start to write position */
        /* make sure all used blocks in blockArray are pre-allocated */
        if (((gctUINT64)curBlockIdx + 1) < gcvMAXUINT32)
        {
            ON_ERROR0(vscBT_ResizeBlockArray (pBlockTbl, curBlockIdx + 1, gcvTRUE));
        }
        else
        {
            return VSC_ERR_OUT_OF_BOUNDS;
        }

        /* read whole blocks */
        for (i=startToReadBlockIndex; i < curBlockIdx; i++)
        {
            pBlockTbl->curBlockIdx = i;
            pBlockTbl->nextOffsetInCurBlock = (i == startToReadBlockIndex) ? startToReadBlockOffset : 0;
            if (fp)
            {
                if (pBlockTbl->ppBlockArray[i] == gcvNULL)
                {
                    /* make sure block is allocated */
                    pBlockTbl->ppBlockArray[i] = (VSC_BT_BLOCK_PTR)vscMM_Alloc(pBlockTbl->pMM, pBlockTbl->blockSize);
                    gcmASSERT(pBlockTbl->ppBlockArray[i] != gcvNULL);
                }
                j = (i == startToReadBlockIndex) ? startToReadBlockOffset/pBlockTbl->entrySize : 0;
                for (; j < pBlockTbl->entryCountPerBlock; j++)
                {
                    gctUINT offset =  j * pBlockTbl->entrySize;
                    ON_ERROR0(fp(Buf, pBlockTbl->ppBlockArray[i] + offset));
                    pBlockTbl->nextOffsetInCurBlock += pBlockTbl->entrySize;
                }
            }
            else
            {
                if (i == startToReadBlockIndex)
                {
                    vscBT_AddContinuousEntries(pBlockTbl,
                                  ioBuffer->buffer+ioBuffer->curPos,
                                  (pBlockTbl->blockSize - startToReadBlockOffset)/pBlockTbl->entrySize);
                    ioBuffer->curPos += pBlockTbl->blockSize  - startToReadBlockOffset;
                }
                else
                {
                    vscBT_AddContinuousEntries(pBlockTbl,
                                  ioBuffer->buffer + ioBuffer->curPos,
                                  pBlockTbl->entryCountPerBlock);
                    ioBuffer->curPos += pBlockTbl->blockSize;
                }
            }
        }

        gcmASSERT(i == curBlockIdx);
        /* read data in last block */
        if (nextOffsetInCurBlock > 0)
        {
            pBlockTbl->curBlockIdx = curBlockIdx;
            pBlockTbl->nextOffsetInCurBlock = (curBlockIdx == startToReadBlockIndex) ? startToReadBlockOffset : 0;
            if (fp)
            {
                j = (i == startToReadBlockIndex) ? startToReadBlockOffset/pBlockTbl->entrySize : 0;
                if (pBlockTbl->ppBlockArray[i] == gcvNULL)
                {
                    /* make sure block is allocated */
                    pBlockTbl->ppBlockArray[i] = (VSC_BT_BLOCK_PTR)vscMM_Alloc(pBlockTbl->pMM, pBlockTbl->blockSize);
                    gcmASSERT(pBlockTbl->ppBlockArray[i] != gcvNULL);
                }
                for (; j < nextOffsetInCurBlock/pBlockTbl->entrySize; j++)
                {
                    gctUINT offset =  j * pBlockTbl->entrySize;
                    ON_ERROR0(fp(Buf, pBlockTbl->ppBlockArray[i] + offset));
                    pBlockTbl->nextOffsetInCurBlock += pBlockTbl->entrySize;
                }
            }
            else
            {
                if (i == startToReadBlockIndex)
                {
                    vscBT_AddContinuousEntries(pBlockTbl,
                                  ioBuffer->buffer+ioBuffer->curPos,
                                  (nextOffsetInCurBlock - startToReadBlockOffset)/pBlockTbl->entrySize);
                    ioBuffer->curPos += nextOffsetInCurBlock  - startToReadBlockOffset;
                }
                else
                {
                    vscBT_AddContinuousEntries(pBlockTbl,
                                  ioBuffer->buffer+ioBuffer->curPos,
                                  nextOffsetInCurBlock/pBlockTbl->entrySize);
                    ioBuffer->curPos += nextOffsetInCurBlock;
                }
                pBlockTbl->nextOffsetInCurBlock = nextOffsetInCurBlock;
            }
        }
    }
    gcmASSERT(blkTbl.nextOffsetInCurBlock == nextOffsetInCurBlock);

OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_readStringTable(VIR_Shader_IOBuffer *Buf, VIR_StringTable* pStringTbl)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    /* read bolcktable */
    errCode = VIR_IO_readBlockTable(Buf, pStringTbl, gcvNULL);
    ON_ERROR(errCode, "Failed to read string table");

    /* re-construct hash table, by reading the nameId*/
    do {
        VIR_Id  nameId;
        gctCHAR * str;
        ON_ERROR0(VIR_IO_readUint(Buf, &nameId));
        if (nameId == VIR_INVALID_ID)
            break;
        /* make sure the namId is in the string table id range */
        if ((nameId <= pStringTbl->curBlockIdx * pStringTbl->blockSize +
            pStringTbl->nextOffsetInCurBlock) == 0)
        {
            return VSC_ERR_OUT_OF_BOUNDS;
        }
        str = (gctCHAR *)BT_GET_ENTRY_DATA(pStringTbl, nameId);
        vscBT_AddToHash(pStringTbl, nameId, str);
    } while (1);

OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_readTypeTable(VIR_Shader_IOBuffer *Buf, VIR_TypeTable* pTypeTbl)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VIR_IO_readBlockTable(Buf, pTypeTbl, (READ_NODE_FP)VIR_IO_readType);
    ON_ERROR(errCode, "Failed to read type table");

    do {
        VIR_Id  tyId;
        VIR_Type * ty;
        ON_ERROR0(VIR_IO_readUint(Buf, &tyId));
        if (tyId == VIR_INVALID_ID)
            break;
        /* make sure the tyId is in the type table id range */
        gcmASSERT(tyId <= (pTypeTbl->curBlockIdx * pTypeTbl->blockSize +
                             pTypeTbl->nextOffsetInCurBlock)/pTypeTbl->entrySize);
        ty = (VIR_Type *)BT_GET_ENTRY_DATA(pTypeTbl, tyId);
        vscBT_AddToHash(pTypeTbl, tyId, ty);
    } while (1);

OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_readLabelTable(VIR_Shader_IOBuffer *Buf, VIR_LabelTable* pLabelTbl)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VIR_IO_readBlockTable(Buf, pLabelTbl, (READ_NODE_FP)VIR_IO_readLabel);
    ON_ERROR(errCode, "Failed to read label table");

    do {
        VIR_Id      labelId;
        VIR_Label * label;
        ON_ERROR0(VIR_IO_readUint(Buf, &labelId));
        if (labelId == VIR_INVALID_ID)
            break;
        /* make sure the tyId is in the type table id range */
        gcmASSERT(labelId <= (pLabelTbl->curBlockIdx * pLabelTbl->blockSize +
                             pLabelTbl->nextOffsetInCurBlock)/pLabelTbl->entrySize);
        label = (VIR_Label *)BT_GET_ENTRY_DATA(pLabelTbl, labelId);
        vscBT_AddToHash(pLabelTbl, labelId, label);
    } while (1);

OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_readConstTable(VIR_Shader_IOBuffer *Buf, VIR_ConstTable* pConstTbl)
{
   VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VIR_IO_readBlockTable(Buf, pConstTbl, (READ_NODE_FP)VIR_IO_readConst);
    ON_ERROR(errCode, "Failed to read label table");

    do {
        VIR_Id      constId;
        VIR_Const * cnst;
        ON_ERROR0(VIR_IO_readUint(Buf, &constId));
        if (constId == VIR_INVALID_ID)
            break;
        /* make sure the tyId is in the type table id range */
        gcmASSERT(constId <= (pConstTbl->curBlockIdx * pConstTbl->blockSize +
                             pConstTbl->nextOffsetInCurBlock)/pConstTbl->entrySize);
        cnst = (VIR_Const *)BT_GET_ENTRY_DATA(pConstTbl, constId);
        vscBT_AddToHash(pConstTbl, constId, cnst);
    } while (1);

OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_readOperandTable(VIR_Shader_IOBuffer *Buf, VIR_OperandTable* pOperandTbl)
{
    VSC_ErrCode errCode = VIR_IO_readBlockTable(Buf, pOperandTbl, (READ_NODE_FP)VIR_IO_readOperand);
    return errCode;
}

VSC_ErrCode
VIR_IO_readSymTable(VIR_Shader_IOBuffer *Buf, VIR_SymTable* pSymTbl)
{
    VSC_ErrCode errCode =  VIR_IO_readBlockTable(Buf, pSymTbl, (READ_NODE_FP)VIR_IO_readSymbol);
    ON_ERROR(errCode, "Failed to read sym table");

    do {
        VIR_Id       symId;
        VIR_Symbol * sym;
        ON_ERROR0(VIR_IO_readUint(Buf, &symId));

        if (symId == VIR_INVALID_ID)
            break;

        /* make sure the tyId is in the type table id range */
        gcmASSERT(symId <= (pSymTbl->curBlockIdx * pSymTbl->blockSize +
                             pSymTbl->nextOffsetInCurBlock)/pSymTbl->entrySize);
        sym = (VIR_Symbol *)BT_GET_ENTRY_DATA(pSymTbl, symId);
        vscBT_AddToHash(pSymTbl, symId, sym);
    } while (1);

OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_readVirRegTable(VIR_Shader_IOBuffer *Buf, VIR_VirRegTable* pVirRegTbl)
{
    VSC_ErrCode errCode =  VSC_ERR_NONE;

    do {
        VIR_Id       symId;
        VIR_Id       virRegId;
        ON_ERROR0(VIR_IO_readUint(Buf, &virRegId));

        if (virRegId == VIR_INVALID_ID)
            break;

        ON_ERROR0(VIR_IO_readUint(Buf, &symId));
        vscHTBL_DirectSet(pVirRegTbl, (void*)(gctUINTPTR_T)virRegId, (void*)(gctUINTPTR_T)symId);
    } while (1);

OnError:
    return errCode;
}
VSC_ErrCode
VIR_IO_readNewIdList(VIR_Shader_IOBuffer *Buf, VIR_IdList** pIdList, gctBOOL Create)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctUINT     count;
    VIR_IdList* list;
    ON_ERROR0(VIR_IO_readUint(Buf, &count));
    if (count == VIR_INVALID_ID)
    {
        if (Create) *pIdList = gcvNULL;
        else
        {
            gcmASSERT(*pIdList != gcvNULL);
            (*pIdList)->count = 0;
            (*pIdList)->ids = gcvNULL;
        }
    }
    else
    {
        if (Create)
        {
            list = gcvNULL;
            ON_ERROR0(VIR_IdList_Init(&Buf->shader->pmp.mmWrapper, count, &list));
            *pIdList = list;
        }
        else
        {
            list = *pIdList;
            ON_ERROR0(VIR_IdList_Init(&Buf->shader->pmp.mmWrapper, count, &list));
        }
        list->count = count;
        if (count > 0)
        {
            ON_ERROR0(VIR_IdList_Reserve(list, count));
            ON_ERROR0(VIR_IO_readBlock(Buf, (gctCHAR *)list->ids, count * sizeof(VIR_Id)));
        }
    }
OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_readIdList(VIR_Shader_IOBuffer *Buf, VIR_IdList* pIdList)
{
    return VIR_IO_readNewIdList(Buf, &pIdList, gcvFALSE);

}

VSC_ErrCode
VIR_IO_readUniform(VIR_Shader_IOBuffer *Buf, VIR_Uniform* pUniform, VIR_SymbolKind symKind)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    ON_ERROR0(VIR_IO_readBlock(Buf, (gctCHAR *)pUniform, sizeof(VIR_Uniform)));
    if (VIR_Uniform_isSampler(pUniform)&& VIR_Uniform_GetResOpBitsArraySize(pUniform) > 0)
    {
        gcmASSERT(VIR_Uniform_GetResOpBitsArray(pUniform) != gcvNULL);
        ON_ERROR0(VIR_IO_readBlock(Buf, (gctCHAR *)VIR_Uniform_GetResOpBitsArray(pUniform),
                                   sizeof(gctUINT32) * VIR_Uniform_GetResOpBitsArraySize(pUniform)));
    }
    else if (VIR_Uniform_isImage(pUniform))
    {
        VSC_ImageDesc desc = { { 0 } };
        /* set the image desc to NULL when read shader, it should be set by driver at
         * runtime if recompile is needed */
        VIR_Uniform_SetImageDesc(pUniform, desc);
        if (symKind == VIR_SYM_SAMPLER_T || symKind == VIR_SYM_IMAGE_T)
        {
            /* also set the lib func name for the image to NULL, it will be set to correct
             * value when go through compiling/recompiling */
            VIR_Uniform_SetImageLibFuncName(pUniform, gcvNULL);
        }
    }

OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_readImageSampler(VIR_Shader_IOBuffer *Buf, VIR_ImageSampler* pImageSampler)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctCHAR ch;
    ON_ERROR0(VIR_IO_readChar(Buf, (gctCHAR*)&pImageSampler->imageNum));
    ON_ERROR0(VIR_IO_readChar(Buf, &ch));
    pImageSampler->isConstantSamplerType = ch;
    ON_ERROR0(VIR_IO_readUint(Buf, &pImageSampler->samplerType));

OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_readKernelInfo(VIR_Shader_IOBuffer *Buf, VIR_KernelInfo** pKernelInfo)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctUINT uVal;

    ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
    if (uVal == VIR_INVALID_ID)
    {
        pKernelInfo = gcvNULL;
    }
    else
    {
        VIR_KernelInfo *kInfo;
        VIR_ValueList * list;
        VSC_MM *        memPool    = &Buf->shader->pmp.mmWrapper;
        kInfo = (VIR_KernelInfo *)vscMM_Alloc(memPool, sizeof(VIR_KernelInfo));
        if (kInfo == gcvNULL)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
        memset(kInfo, 0, sizeof(VIR_KernelInfo));

        ON_ERROR0(VIR_IO_readUint(Buf, &kInfo->kernelName));
        ON_ERROR0(VIR_IO_readUint(Buf, &kInfo->localMemorySize));
        ON_ERROR0(VIR_IO_readIdList(Buf, &kInfo->uniformArguments));
        ON_ERROR0(VIR_IO_readInt(Buf, &kInfo->samplerIndex));
        list = &kInfo->imageSamplers;
        ON_ERROR0(VIR_IO_readValueList(Buf, &list, (READ_NODE_FP)0));
        list  = &kInfo->properties;
        ON_ERROR0(VIR_IO_readValueList(Buf, &list, (READ_NODE_FP)0));
        ON_ERROR0(VIR_IO_readInt(Buf, &kInfo->isMain));
    }

OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_readInstList(
    VIR_Shader_IOBuffer *   Buf,
    VIR_Function *          Function,
    VIR_InstList *          pInstList
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Instruction *   inst;
    gctUINT             idx = 0;

    do {
        ON_ERROR0(VIR_IO_readUint(Buf, &idx));
        if (idx == VIR_INVALID_ID)
            break;
        ON_ERROR0(VIR_Function_AddInstruction(Function, VIR_OP_NOP, VIR_TYPE_UNKNOWN, &inst));
        ON_ERROR0(VIR_IO_readInst(Buf, inst));
    } while (1);

OnError:
    return errCode;
}

static VSC_ErrCode
_PatchLabelDefInst(
    VIR_Function *     pFunction
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Instruction *   inst;
    VIR_InstIterator    instIter;

    VIR_InstIterator_Init(&instIter, VIR_Function_GetInstList(pFunction));
    inst = (VIR_Instruction *)VIR_InstIterator_First(&instIter);
    for (; inst != gcvNULL; inst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter))
    {
        if (VIR_Inst_GetOpcode(inst) == VIR_OP_LABEL)
        {

            VIR_Label *label  = VIR_Operand_GetLabel(VIR_Inst_GetDest(inst));
            gcmASSERT(label->sym != VIR_INVALID_ID);
            gcmASSERT((gctUINTPTR_T)label->defined == (gctUINTPTR_T)VIR_Inst_GetId(inst));
            label->defined    = inst;
        }
    }
    return errCode;
}

VSC_ErrCode
VIR_IO_readFunction(VIR_Shader_IOBuffer *Buf, VIR_Function* pFunction)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctUINT     uVal;

    VIR_Shader_SetCurrentFunction(Buf->shader, pFunction);
    VIR_Function_SetShader(pFunction, Buf->shader);
    VIR_Function_SetFuncBlock(pFunction, gcvNULL);

    ON_ERROR0(VIR_IO_readInt(Buf, &pFunction->_lastInstId));
    ON_ERROR0(VIR_IO_readUint(Buf, &pFunction->_labelId));
    ON_ERROR0(VIR_IO_readUint(Buf, &pFunction->funcSym));
    ON_ERROR0(VIR_IO_readUint(Buf, (gctUINT *)&pFunction->flags));
    ON_ERROR0(VIR_IO_readUint(Buf, &pFunction->maxCallDepth));

    ON_ERROR0(VIR_IO_readSymTable(Buf, &pFunction->symTable));
    /* debug help */
    VIR_IO_readUint(Buf, &uVal);
    if (uVal != SYMTBL_SIG)
    {
        gcmASSERT(gcvFALSE);
    }

    ON_ERROR0(VIR_IO_readLabelTable(Buf, &pFunction->labelTable));
    ON_ERROR0(VIR_IO_readOperandTable(Buf, &pFunction->operandTable));

    ON_ERROR0(VIR_IO_readIdList(Buf, &pFunction->localVariables));
    ON_ERROR0(VIR_IO_readIdList(Buf, &pFunction->paramters));
    ON_ERROR0(VIR_IO_readIdList(Buf, &pFunction->temps));

    ON_ERROR0(VIR_IO_readKernelInfo(Buf, &pFunction->kernelInfo));

    ON_ERROR0(VIR_IO_readInt(Buf, &pFunction->tempIndexStart));
    ON_ERROR0(VIR_IO_readInt(Buf, &pFunction->tempIndexCount));

    /* read instructions */
    ON_ERROR0(VIR_IO_readInstList(Buf, pFunction, &pFunction->instList));
    /* debug help */
    VIR_IO_readUint(Buf, &uVal);
    if (uVal != DBUG_SIG)
    {
        gcmASSERT(gcvFALSE);
    }

    ON_ERROR0(_PatchLabelDefInst(pFunction));
    ON_ERROR0(VIR_Function_BuildLabelLinks(pFunction));

    VIR_Shader_SetCurrentFunction(Buf->shader, gcvNULL);

OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_readUniformBlock(VIR_Shader_IOBuffer *Buf, VIR_UniformBlock* pUniformBlock)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctUINT i, uVal;

    ON_ERROR0(VIR_IO_readUint(Buf, &pUniformBlock->sym));

    ON_ERROR0(VIR_IO_readUint(Buf, (gctUINT *)&pUniformBlock->flags));
    ON_ERROR0(VIR_IO_readShort(Buf, &pUniformBlock->blockIndex));

    ON_ERROR0(VIR_IO_readUint(Buf, &pUniformBlock->baseAddr));
    ON_ERROR0(VIR_IO_readUint(Buf, &pUniformBlock->blockSize));
    ON_ERROR0(VIR_IO_readUint(Buf, &pUniformBlock->uniformCount));

    if (pUniformBlock->uniformCount > 0)
    {
        pUniformBlock->uniforms = (VIR_Uniform **)vscMM_Alloc(&Buf->shader->pmp.mmWrapper,
                                          sizeof(VIR_Uniform*) * pUniformBlock->uniformCount);
        for (i=0; i<pUniformBlock->uniformCount; i++)
        {
            ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
            /* to patch */
            pUniformBlock->uniforms[i] = (VIR_Uniform *)(size_t)uVal;
        }
    }
    else
    {
        pUniformBlock->uniforms = gcvNULL;
    }
OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_readStorageBlock(VIR_Shader_IOBuffer *Buf, VIR_StorageBlock* pStorageBlock)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctUINT i, uVal;

    ON_ERROR0(VIR_IO_readUint(Buf, &pStorageBlock->sym));

    ON_ERROR0(VIR_IO_readUint(Buf, (gctUINT *)&pStorageBlock->flags));
    ON_ERROR0(VIR_IO_readShort(Buf, &pStorageBlock->blockIndex));
    ON_ERROR0(VIR_IO_readUint(Buf, &pStorageBlock->baseAddr));
    ON_ERROR0(VIR_IO_readUint(Buf, &pStorageBlock->blockSize));
    ON_ERROR0(VIR_IO_readUint(Buf, &pStorageBlock->variableCount));

    if (pStorageBlock->variableCount > 0)
    {
        pStorageBlock->variables = (VIR_SymId *)vscMM_Alloc(&Buf->shader->pmp.mmWrapper,
                                          sizeof(VIR_SymId) * pStorageBlock->variableCount);
        for (i=0; i<pStorageBlock->variableCount; i++)
        {
            ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
            pStorageBlock->variables[i] = (VIR_SymId)uVal;
        }
    }
    else
    {
        pStorageBlock->variables = gcvNULL;
    }
OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_readIOBlock(VIR_Shader_IOBuffer *Buf, VIR_IOBlock* pIOBlock)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    ON_ERROR0(VIR_IO_readUint(Buf, &pIOBlock->sym));
    ON_ERROR0(VIR_IO_readUint(Buf, (gctUINT *)&pIOBlock->flags));
    ON_ERROR0(VIR_IO_readShort(Buf, &pIOBlock->blockIndex));
    ON_ERROR0(VIR_IO_readInt(Buf, &pIOBlock->blockNameLength));
    ON_ERROR0(VIR_IO_readInt(Buf, &pIOBlock->instanceNameLength));
    ON_ERROR0(VIR_IO_readUint(Buf, (gctUINT *)&pIOBlock->Storage));

OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_readLabel(VIR_Shader_IOBuffer *Buf, VIR_Label* pLabel)
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;

    gctUINT         uVal;

    ON_ERROR0(VIR_IO_readUint(Buf, (gctUINT *)&pLabel->index));
    ON_ERROR0(VIR_IO_readUint(Buf, &pLabel->sym));
    ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
    /* to patch */
    pLabel->defined = (uVal == VIR_INVALID_ID) ? gcvNULL : (VIR_Instruction *)(size_t)uVal;
    pLabel->referenced = gcvNULL;

OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_readConst(VIR_Shader_IOBuffer *Buf, VIR_Const* pConst)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctUINT components;
    gctUINT rows;
    gctUINT i;

    ON_ERROR0(VIR_IO_readUint(Buf, &pConst->index));
    ON_ERROR0(VIR_IO_readUint(Buf, (gctUINT *)&pConst->type));
    components = VIR_GetTypeComponents(pConst->type);
    rows       = VIR_GetTypeRows(pConst->type);
    gcmASSERT(VIR_TypeId_isPrimitive(pConst->type));
    gcmASSERT(components*rows > 0 && components*rows < 16);
    /* read constant data one by one */
    memset(&pConst->value, 0, sizeof(pConst->value));
    for (i=0; i < components*rows; i++)
    {
        switch (VIR_GetTypeComponentType(pConst->type)) {
        case VIR_TYPE_INT8:
        case VIR_TYPE_UINT8:
            ON_ERROR0(VIR_IO_readChar(Buf, (gctCHAR *)&pConst->value.vecVal.i8Value[i]));
            break;
        case VIR_TYPE_INT16:
            ON_ERROR0(VIR_IO_readShort(Buf, &pConst->value.vecVal.i16Value[i]));
            break;
        case VIR_TYPE_UINT16:
            ON_ERROR0(VIR_IO_readUshort(Buf, &pConst->value.vecVal.u16Value[i]));
            break;
        case VIR_TYPE_INT32:
        case VIR_TYPE_BOOLEAN:
            ON_ERROR0(VIR_IO_readInt(Buf, &pConst->value.vecVal.i32Value[i]));
            break;
        case VIR_TYPE_UINT32:
            ON_ERROR0(VIR_IO_readUint(Buf, &pConst->value.vecVal.u32Value[i]));
            break;
        case VIR_TYPE_FLOAT32:
            ON_ERROR0(VIR_IO_readFloat(Buf, &pConst->value.vecVal.f32Value[i]));
            break;
        default:
            gcmASSERT(gcvFALSE);
            break;
        }
    }

OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_readInst(VIR_Shader_IOBuffer *Buf, VIR_Instruction* pInst)
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    gctUINT         uVal;
    gctUINT         i;
    VIR_Operand *   opnd;

    /* Word 1. */
    ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
    VIR_Inst_SetOpcode(pInst, (VIR_OpCode)((uVal >> 22) & 0x3FF));
    VIR_Inst_SetId(pInst, (uVal >> 2)&0xFFFFF);
    VIR_Inst_SetIsPrecise(pInst, (uVal >> 1) & 0x01);
    VIR_Inst_SetPatched(pInst, (uVal & 0x01)) ;

    /* Word 2. */
    ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
    VIR_Inst_SetInstType(pInst, (VIR_TypeId)(uVal));

    /* Word 3. */
    ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
    VIR_Inst_SetConditionOp(pInst, (uVal >> 27) & 0x1F);
    VIR_Inst_SetFlags(pInst, (uVal >> 24) & 0x07);

    VIR_Inst_SetSrcNum(pInst, (uVal >> 21) & 0x07);
    if (VIR_Inst_GetSrcNum(pInst) > VIR_MAX_SRC_NUM)
    {
        gcmASSERT(gcvFALSE);
        VIR_Inst_SetSrcNum(pInst, VIR_OPCODE_GetSrcOperandNum(VIR_Inst_GetOpcode(pInst)));
    }

    VIR_Inst_SetThreadMode(pInst, (uVal >> 18) & 0x07);
    VIR_Inst_SetParentUseBB(pInst, (uVal >> 17) & 0x1);
    VIR_Inst_SetResOpType(pInst, (VIR_RES_OP_TYPE)((uVal >> 11) & 0x3F));
    VIR_Inst_SetIsPatternRep(pInst, (uVal >> 10) & 0x1);
    VIR_Inst_SetLoopInvariant(pInst, (uVal >> 9) & 0x1);
    VIR_Inst_SetEndOfBB(pInst, (uVal >> 8) & 0x1);
    VIR_Inst_SetUSCUnallocate(pInst, (uVal >> 7) & 0x1);
    VIR_INST_SetSkHp(pInst, (uVal >> 6) & 0x1);

    ON_ERROR0(VIR_IO_readUint(Buf, (gctUINT *)&pInst->sourceLoc));

    ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
    if (uVal != VIR_INVALID_ID)
    {
        opnd = VIR_Function_GetOperandFromId(Buf->shader->currentFunction, uVal);
        VIR_Inst_SetDest(pInst, opnd);
    }
    else
    {
        VIR_Inst_SetDest(pInst, gcvNULL);
    }

    for (i = 0; i < VIR_Inst_GetSrcNum(pInst); i++)
    {
        ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
        if (uVal != VIR_INVALID_ID)
        {
            opnd = VIR_Function_GetOperandFromId(Buf->shader->currentFunction, uVal);
            VIR_Inst_SetSource(pInst, i, opnd);
        }
        else
        {
            VIR_Inst_SetSource(pInst, i, gcvNULL);
        }
    }
    ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
    /* debug help: check INST_SIG */
    gcmASSERT(uVal == INST_SIG);

OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_readParmPassing(VIR_Shader_IOBuffer *Buf, VIR_ParmPassing** pParmPassing)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctUINT uVal;
    gctUINT i;
    VIR_ParmPassing *parm;

    ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
    if (uVal == VIR_INVALID_ID)
    {
        *pParmPassing = gcvNULL;
    }
    else
    {
        /* create parameter passing array */
        gctUINT allocSize = sizeof(VIR_ParmPassing) + ((uVal > 0) ? (uVal - 1) * sizeof (VIR_Operand *) : 0);
        parm = (VIR_ParmPassing *)vscMM_Alloc(&Buf->shader->pmp.mmWrapper, allocSize);

        *pParmPassing = parm;
        if (parm == gcvNULL)
        {
            errCode = VSC_ERR_OUT_OF_MEMORY;
        }
        else
        {
            parm->argNum = uVal;
            for (i = 0; i<parm->argNum; i++)
            {
                ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
                parm->args[i] = VIR_Function_GetOperandFromId(Buf->shader->currentFunction, uVal);
            }
        }
    }
OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_readVarTempRegInfo(VIR_Shader_IOBuffer *Buf, VIR_VarTempRegInfo *pVarInfo)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    ON_ERROR0(VIR_IO_readUint(Buf, &pVarInfo->variable));
    ON_ERROR0(VIR_IO_readUint(Buf, &pVarInfo->streamoutSize));
    ON_ERROR0(VIR_IO_readInt(Buf, &pVarInfo->tempRegCount));
    pVarInfo->tempRegTypes = gcvNULL;
OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_readTransformFeedback(VIR_Shader_IOBuffer *Buf, VIR_TransformFeedback *tfb)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctUINT uVal;

    ON_ERROR0(VIR_IO_readNewIdList(Buf, &tfb->varyings, gcvTRUE));
    ON_ERROR0(VIR_IO_readInt(Buf, (gctINT*)&tfb->bufferMode));
    ON_ERROR0(VIR_IO_readUint(Buf, &(tfb->stateUniformId)));
    tfb->varRegInfos = gcvNULL;
    ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
    if (uVal != VIR_INVALID_ID)
    {
        ON_ERROR0(VIR_IO_readValueList(Buf, &tfb->varRegInfos,
                                    (WRITE_NODE_FP)VIR_IO_readVarTempRegInfo));
    }
    ON_ERROR0(VIR_IO_readUint(Buf, &tfb->totalSize));
    ON_ERROR0(VIR_IO_readInt(Buf, &tfb->shaderTempCount));
    if (tfb->bufferMode == VIR_FEEDBACK_INTERLEAVED)
    {
        ON_ERROR0(VIR_IO_readUint(Buf, &(tfb->feedbackBuffer.interleavedBufUniformId)));
    }
    else
    {
        gctINT i;
        gctUINT64 allocSize;
        VIR_UniformId * uniformIds;
        /* VIR_FEEDBACK_SEPARATE mode */
        allocSize = sizeof(VIR_UniformId) * (gctUINT64)(tfb->shaderTempCount);
        if (allocSize > gcvMAXUINT32)
        {
            return VSC_ERR_OUT_OF_BOUNDS;
        }
        else
        {
            allocSize = sizeof(VIR_UniformId) * tfb->shaderTempCount;
        }
        uniformIds = (VIR_UniformId *)vscMM_Alloc(&Buf->shader->pmp.mmWrapper, (gctUINT)allocSize);
        tfb->feedbackBuffer.separateBufUniformIds = uniformIds;
        if (uniformIds == gcvNULL)
        {
            errCode = VSC_ERR_OUT_OF_MEMORY;
        }
        else
        {
            for (i=0; i<tfb->shaderTempCount; i++)
            {
                ON_ERROR0(VIR_IO_readUint(Buf,
                    &(tfb->feedbackBuffer.separateBufUniformIds[i])));
            }
        }
    }
OnError:
    return errCode;
}


VSC_ErrCode
VIR_IO_readOperandList(
    VIR_Shader_IOBuffer *Buf,
    VIR_OperandList**    pOperandList)
{
    VSC_ErrCode       errCode = VSC_ERR_NONE;
    VIR_OperandList * ptr     = gcvNULL;
    gctUINT           uVal;
    gctUINT           allocSize;

    gcmASSERT(*pOperandList == gcvNULL);

    while (ptr)
    {
        VIR_OperandList * node;
        ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
        if (uVal == VIR_INVALID_ID)
            break;

        allocSize = sizeof(VIR_OperandList) * uVal;
        node = (VIR_OperandList *)vscMM_Alloc(&Buf->shader->pmp.mmWrapper, allocSize);
        node->value = VIR_Function_GetOperandFromId(Buf->shader->currentFunction, uVal);
        node->next = gcvNULL;
        if (*pOperandList == gcvNULL)
        {
            *pOperandList = node;
        }
        else
        {
            gcmASSERT(ptr != gcvNULL);
            ptr->next = node;
        }
        ptr = node;
    }
OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_readPhiOperandArray(
    VIR_Shader_IOBuffer *    Buf,
    VIR_PhiOperandArray **   pPhiOperandArray
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    gctUINT                 i;
    gctUINT                 cnt, uVal;
    VIR_PhiOperandArray *   newPhiOperands;
    VIR_Function *          curFunction = Buf->shader->currentFunction;

    ON_ERROR0(VIR_IO_readUint(Buf, &cnt));
    errCode = VIR_Function_NewPhiOperandArray(curFunction, cnt, &newPhiOperands);
    if (errCode != VSC_ERR_NONE)
    {
        return errCode;
    }

    *pPhiOperandArray = newPhiOperands;

    for(i = 0; i < cnt; i++)
    {
        VIR_PhiOperand* phiOperand = VIR_PhiOperandArray_GetNthOperand(newPhiOperands, i);

        ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
        VIR_PhiOperand_SetValue(phiOperand, VIR_Function_GetOperandFromId(curFunction, uVal));
        ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
        VIR_PhiOperand_SetLabel(phiOperand, VIR_Function_GetLabelFromId(curFunction, uVal));
        ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
        VIR_PhiOperand_SetFlags(phiOperand, uVal);
    }

OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_readValueList(VIR_Shader_IOBuffer *Buf, VIR_ValueList** pValueList, READ_NODE_FP fp)
{
    VSC_ErrCode      errCode = VSC_ERR_NONE;
    VIR_ValueList *  list;
    gctUINT          i;
    gctUINT          sz;
    gctUINT          cnt, elemSz;

    ON_ERROR0(VIR_IO_readUint(Buf, &elemSz));
    ON_ERROR0(VIR_IO_readUint(Buf, &cnt));

    sz = (cnt * elemSz);

    ON_ERROR0(VIR_IO_CheckBounds(Buf, sz));
    ON_ERROR0(VIR_ValueList_Init(&Buf->shader->pmp.mmWrapper, cnt, elemSz, pValueList));
    list = * pValueList;
    if (cnt > 0)
    {
        list->count = cnt;
        if (fp)
        {
            for (i=0; i < list->count; i++)
            {
                ON_ERROR0(fp(Buf, list->values + (i * list->elemSize)));
            }
        }
        else
        {
            ON_ERROR0(VIR_IO_readBlock(Buf, list->values, sz));
        }
    }

    ON_ERROR0(VIR_IO_readUint(Buf, &sz));
    /* debug help: check DBUG_SIG */
    gcmASSERT(sz == DBUG_SIG);

OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_readOperand(VIR_Shader_IOBuffer *Buf, VIR_Operand* pOperand)
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    gctUINT         val;
    VIR_Precision   precision;
    VIR_OperandKind opndKind;

    /* operand header */

    ON_ERROR0(VIR_IO_readUint(Buf, &val));
    /* debug help: check DBUG_SIG */
    gcmASSERT(val == DBUG_SIG);

    ON_ERROR0(VIR_IO_readUint(Buf, &val));
    pOperand->header = *(VIR_Operand_Header*)&val;
    opndKind = (VIR_OperandKind)VIR_Operand_GetOpKind(pOperand);

    if (opndKind != VIR_OPND_TEXLDPARM)
    {
        /* Word 1. */
        ON_ERROR0(VIR_IO_readUint(Buf, &val));
        VIR_Operand_SetTypeId(pOperand, (VIR_TypeId)(val));

        /* Word 2. */
        ON_ERROR0(VIR_IO_readUint(Buf, &val));
        if (VIR_Operand_isLvalue(pOperand))
        {
            VIR_Operand_SetEnable(pOperand, (VIR_Enable)((val >> 24) & 0xFF));
        }
        else
        {
            VIR_Operand_SetSwizzle(pOperand, (VIR_Swizzle)((val >> 24) & 0xFF));
        }
        precision = (VIR_Precision)((val >> 21) & 0x07);
        VIR_Operand_SetBigEndian(pOperand, ((val >> 20) & 0x1));
        VIR_Operand_SetModOrder(pOperand, ((val >> 18) & 0x2));

        /* Word 3. */
        ON_ERROR0(VIR_IO_readUint(Buf, &val));

        VIR_Operand_SetHwShift(pOperand, ((val >> 30) & 0x03));
        VIR_Operand_SetHwRegId(pOperand, ((val >> 20) & 0x3FF));
        VIR_Operand_SetHIHwRegId(pOperand, ((val >> 10) & 0x3FF));
        VIR_Operand_SetHIHwShift(pOperand, ((val >> 8) & 0x03));
        VIR_Operand_SetLShift(pOperand, ((val >> 5) & 0x07));

        ON_ERROR0(VIR_IO_readUint(Buf, &val));
        VIR_Operand_SetFlags(pOperand, (VIR_OPNDFLAG)val);
        /* u1 */
        switch(opndKind)
        {
        case VIR_OPND_NONE:
        case VIR_OPND_UNDEF:
        case VIR_OPND_UNUSED:
            break;
        case VIR_OPND_EVIS_MODIFIER:
            ON_ERROR0(VIR_IO_readUint(Buf, &val));
            VIR_Operand_SetEvisModifier(pOperand, val);
            break;
        case VIR_OPND_CONST:
            ON_ERROR0(VIR_IO_readUint(Buf, &val));
            VIR_Operand_SetConstId(pOperand, val);
            break;
        case VIR_OPND_PARAMETERS:
            ON_ERROR0(VIR_IO_readParmPassing(Buf, &VIR_Operand_GetParameters(pOperand)));
            break;
        case VIR_OPND_LABEL:
            ON_ERROR0(VIR_IO_readUint(Buf,&val));
            pOperand->u.n.u1.label = VIR_GetLabelFromId(Buf->shader->currentFunction, val);
            break;
        case VIR_OPND_FUNCTION:
            ON_ERROR0(VIR_IO_readUint(Buf, &val));
            VIR_Operand_SetFunc(pOperand,
                VIR_Symbol_GetFunction(VIR_Shader_GetSymFromId(Buf->shader, val)));
            break;
        case VIR_OPND_SAMPLER_INDEXING:
        case VIR_OPND_SYMBOL:
        case VIR_OPND_ADDRESS_OF:
        case VIR_OPND_VEC_INDEXING:
            {
                VIR_Symbol * sym;
                ON_ERROR0(VIR_IO_readUint(Buf, &val));
                sym = VIR_Function_GetSymFromId(Buf->shader->currentFunction, val);
                VIR_Operand_SetSym(pOperand, sym);
            }
            break;
        case VIR_OPND_NAME:
            ON_ERROR0(VIR_IO_readUint(Buf, &val));
            VIR_Operand_SetName(pOperand, (VIR_NameId)val);
            break;
        case VIR_OPND_INTRINSIC:
            ON_ERROR0(VIR_IO_readUint(Buf, &val));
            VIR_Operand_SetIntrinsicKind(pOperand, (VIR_IntrinsicsKind)val);
            break;
        case VIR_OPND_FIELD:
            ON_ERROR0(VIR_IO_readUint(Buf, &val));
            VIR_Operand_SetFieldBase(pOperand,
                VIR_Function_GetOperandFromId(Buf->shader->currentFunction, val));
            break;
        case VIR_OPND_ARRAY:
            ON_ERROR0(VIR_IO_readUint(Buf, &val));
            VIR_Operand_SetArrayBase(pOperand,
                VIR_Function_GetOperandFromId(Buf->shader->currentFunction, val));
            break;
        case VIR_OPND_PHI:
             ON_ERROR0(VIR_IO_readPhiOperandArray(Buf, &VIR_Operand_GetPhiOperands(pOperand)));
           break;
        case VIR_OPND_SIZEOF:
        case VIR_OPND_OFFSETOF:
            gcmASSERT(gcvFALSE);  /* need to refine the definition of the operand */
            break;
        case VIR_OPND_IMMEDIATE:
        default:
            /* write bits as uint */
            ON_ERROR0(VIR_IO_readUint(Buf, &VIR_Operand_GetImmediateUint(pOperand)));
            break;
        }
        /* u2 */
        switch(VIR_Operand_GetOpKind(pOperand))
        {
        case VIR_OPND_FIELD:
            ON_ERROR0(VIR_IO_readUint(Buf, &val));
            VIR_Operand_SetFieldId(pOperand, val);
            break;
        case VIR_OPND_ARRAY:
            VIR_Operand_SetArrayIndex(pOperand, gcvNULL);
            ON_ERROR0(VIR_IO_readOperandList(Buf, &VIR_Operand_GetArrayIndex(pOperand)));
            break;
        case VIR_OPND_VEC_INDEXING:
            ON_ERROR0(VIR_IO_readUint(Buf, &val));
            VIR_Operand_SetVecIndexSymId(pOperand, val);
            break;
        default:
            ON_ERROR0(VIR_IO_readUint(Buf, &val));
            VIR_Operand_SetFieldId(pOperand, val);
            break;
        }

        /* u3 */
        {
            ON_ERROR0(VIR_IO_readUint(Buf, &val));
            VIR_Operand_SetMatrixStride(pOperand, val);
            ON_ERROR0(VIR_IO_readUint(Buf, &val));
            VIR_Operand_SetLayoutQual(pOperand, (VIR_LayoutQual)val);
        }
        /* setting precision depends other part be read */
        VIR_Operand_SetPrecision(pOperand, precision);
    }
    else
    {
        gctUINT i;
        for (i = 0; i <VIR_TEXLDMODIFIER_COUNT; i++)
        {
            ON_ERROR0(VIR_IO_readUint(Buf, &val));
            if (val != VIR_INVALID_ID)
            {
                VIR_Operand_SetTexldModifier(pOperand, i,
                    VIR_Function_GetOperandFromId(Buf->shader->currentFunction, val));
            }
            else
            {
                VIR_Operand_SetTexldModifier(pOperand, i, gcvNULL);
            }
        }
    }
OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_readSymbol(VIR_Shader_IOBuffer *Buf, VIR_Symbol* pSymbol)
{
    VSC_ErrCode       errCode = VSC_ERR_NONE;
    VIR_SymbolKind    symKind;
    gctUINT           uVal;
    VIR_Uniform *     uniform;
    VIR_SymId         firstElementId;

    /* write first two words */
    ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
    ((gctUINT*)pSymbol)[0] = uVal;
    ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
    ((gctUINT*)pSymbol)[1] = uVal;

    ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
    VIR_Symbol_SetTypeId(pSymbol, (VIR_TypeId)uVal);
    ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
    VIR_Symbol_SetFixedTypeId(pSymbol, (VIR_TypeId)uVal);
    ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
    VIR_Symbol_SetFlags(pSymbol, (VIR_SymFlag)uVal);
    ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
    VIR_Symbol_SetFlagsExt(pSymbol, (VIR_SymFlagExt)uVal);
    ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
    VIR_Symbol_SetIndex(pSymbol, uVal);
    ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
    VIR_Symbol_SetIOBlockIndex(pSymbol, uVal);

    ON_ERROR0(VIR_IO_readBlock(Buf, (gctCHAR*)&pSymbol->layout, sizeof(pSymbol->layout)));
    /* u0 */
    if (isSymLocal(pSymbol))
    {
        ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
        VIR_Symbol_SetHostFunction(pSymbol, (VIR_Function *)gcmINT2PTR(uVal));
        _VIR_IO_SymbolListQueue(&Buf->shader->pmp.mmWrapper,
                                &Buf->localSymbolList,
                                pSymbol);
    }
    else
    {
        VIR_Symbol_SetHostShader(pSymbol, Buf->shader);
    }

    /* u1 */
    ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
    pSymbol->u1.vregIndex = uVal;

    symKind = VIR_Symbol_GetKind(pSymbol);
    /* u2 */
    switch(symKind)
    {
    case VIR_SYM_UNIFORM:
    case VIR_SYM_SAMPLER:
    case VIR_SYM_SAMPLER_T:
    case VIR_SYM_IMAGE:
    case VIR_SYM_IMAGE_T:
        ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
        VIR_Shader_AddSymbolContents(Buf->shader, pSymbol, uVal, gcvFALSE);
        uniform = (pSymbol)->u2.uniform;
        ON_ERROR0(VIR_IO_readUniform(Buf, uniform, symKind));
        break;
    case VIR_SYM_VARIABLE:
    case VIR_SYM_TEXTURE:
        ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
        VIR_Symbol_SetVariableVregIndex(pSymbol, uVal);
        break;
    case VIR_SYM_SBO:
        {
            VIR_StorageBlock* storageBlock;
            ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
            VIR_Shader_AddSymbolContents(Buf->shader, pSymbol, uVal, gcvFALSE);
            storageBlock = VIR_Symbol_GetSBO(pSymbol);
            ON_ERROR0(VIR_IO_readStorageBlock(Buf, storageBlock));
        }
        break;
    case VIR_SYM_UBO:
        {
            VIR_UniformBlock* ubo;
            ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
            VIR_Shader_AddSymbolContents(Buf->shader, pSymbol, uVal, gcvFALSE);
            ubo = VIR_Symbol_GetUBO(pSymbol);
            ON_ERROR0(VIR_IO_readUniformBlock(Buf, ubo));
        }
        break;
    case VIR_SYM_IOBLOCK:
        {
            VIR_IOBlock* iob;
            ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
            VIR_Shader_AddSymbolContents(Buf->shader, pSymbol, uVal, gcvFALSE);
            iob = VIR_Symbol_GetIOB(pSymbol);
            ON_ERROR0(VIR_IO_readIOBlock(Buf, iob));
        }
        break;
    case VIR_SYM_FUNCTION:
        VIR_Symbol_SetFunction(pSymbol, gcvNULL);
        break;
    case VIR_SYM_FIELD:
        ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
        if (uVal != VIR_INVALID_ID)
        {
            VIR_FieldInfo * fi;
            fi = (VIR_FieldInfo *)vscMM_Alloc(&Buf->shader->pmp.mmWrapper,
                                             sizeof(VIR_FieldInfo));
            if (fi == gcvNULL)
            {
                return VSC_ERR_OUT_OF_MEMORY;
            }
            VIR_Symbol_SetFieldInfo(pSymbol, fi);
            ON_ERROR0(VIR_IO_readBlock(Buf, (gctCHAR *)fi, sizeof(VIR_FieldInfo)));
        }
        else
        {
            VIR_Symbol_SetFieldInfo(pSymbol, gcvNULL);
        }
        break;
    case VIR_SYM_VIRREG:
        ON_ERROR0(VIR_IO_readUint(Buf, &pSymbol->u2.varSymId));
        VIR_Shader_AddSymbolContents(Buf->shader, pSymbol, VIR_Symbol_GetVregIndex(pSymbol), gcvFALSE);
        break;
    case VIR_SYM_TYPE:
    case VIR_SYM_LABEL:
    case VIR_SYM_CONST:
    case VIR_SYM_UNKNOWN:
    default:
        ON_ERROR0(VIR_IO_readUint(Buf, &pSymbol->u2.varSymId));
       break;
    }
    /* u3 */
    switch(symKind)
    {
    case VIR_SYM_FUNCTION:
        ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
        VIR_Symbol_SetFuncMangleName(pSymbol, uVal);
        break;
    case VIR_SYM_FIELD:
        ON_ERROR0(VIR_IO_readUint(Buf, &uVal));
        VIR_Symbol_SetStructTypeId(pSymbol, (VIR_TypeId)uVal);
        break;
    default:
        if (isSymCombinedSampler(pSymbol))
        {
            ON_ERROR0(VIR_IO_readUint(Buf, &VIR_Symbol_GetSeparateSamplerId(pSymbol)));
            ON_ERROR0(VIR_IO_readUint(Buf, &VIR_Symbol_GetSeparateSamplerFuncId(pSymbol)));
        }
        else
            ON_ERROR0(VIR_IO_readUint(Buf, &VIR_Symbol_GetOffsetInVar(pSymbol)));
        break;
    }
    /* u4 */
    if (isSymCombinedSampler(pSymbol))
    {
        ON_ERROR0(VIR_IO_readUint(Buf, &VIR_Symbol_GetSeparateImageId(pSymbol)));
        ON_ERROR0(VIR_IO_readUint(Buf, &VIR_Symbol_GetSeparateImageFuncId(pSymbol)));
    }
    else if (VIR_Symbol_isParamVirReg(pSymbol))
         ON_ERROR0(VIR_IO_readUint(Buf, &VIR_Symbol_GetParamFuncSymId(pSymbol)));

    /* u5 */
    ON_ERROR0(VIR_IO_readInt(Buf, &VIR_Symbol_GetIndexRange(pSymbol)));

    /* u6 */
    ON_ERROR0(VIR_IO_readUint(Buf, &firstElementId));
    VIR_Symbol_SetFirstElementId(pSymbol, firstElementId);

    /* debug help */
    VIR_IO_readUint(Buf, &uVal);
    if (uVal != SYMB_SIG)
    {
        gcmASSERT(gcvFALSE);
    }


OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_readType(VIR_Shader_IOBuffer *Buf, VIR_Type* pType)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    ON_ERROR0(VIR_IO_readBlock(Buf, (gctCHAR*)pType, sizeof(VIR_Type)));

    switch (pType->_kind) {
    case VIR_TY_STRUCT:
    case VIR_TY_ENUM:
        /* field id list */
        VIR_Type_SetFields(pType, gcvNULL);
        ON_ERROR0(VIR_IO_readNewIdList(Buf, &VIR_Type_GetFields(pType), gcvTRUE));
        break;
    case VIR_TY_FUNCTION:
        /* parameter id list */
        VIR_Type_SetParameters(pType, gcvNULL);
        ON_ERROR0(VIR_IO_readNewIdList(Buf, &VIR_Type_GetParameters(pType), gcvTRUE));
        break;
    case VIR_TY_POINTER:
    case VIR_TY_ARRAY:
    default:
        break;
    }
OnError:
    return errCode;
}

VSC_ErrCode
VIR_IO_readShader(VIR_Shader_IOBuffer *buf, VIR_Shader* pShader, gctUINT messageLevel)
{
    VSC_ErrCode errCode= VSC_ERR_NONE;
    gctUINT     i;
    gctUINT     uVal;
    gctUINT     binaryFileVersion, chipModel, chipRevision;
    VSC_MM *    memPool    = &pShader->pmp.mmWrapper;
    gctBOOL     needToPrint = gcvFALSE;
    gctSTRING   messagePrefix = "";
    gctUINT     magicNum;

    ON_ERROR0(VIR_IO_readUint(buf, &uVal));
    if (uVal != SHDR_SIG)
    {
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid shader signature 0x%x.", uVal);
    }
    ON_ERROR0(VIR_IO_readUint(buf, &binaryFileVersion));

    /* If the shader or chip version does not match with current compiler's,
     * messageLevel = 0: print information messages only when VC_OPTION is set to -DUMP:ALL;
     * messageLevel = 1: print warning messages;
     * messageLevel = 2: print error messages. */
    if (messageLevel > 0 || gcmOPT_DUMP_OPTIMIZER())
    {
        needToPrint = gcvTRUE;
        if (messageLevel == 0)
            messagePrefix = "INFO";
        else if (messageLevel == 1)
            messagePrefix = "WARN";
        else if (messageLevel == 2)
            messagePrefix = "ERROR";
    }
    if (binaryFileVersion != gcdVIR_SHADER_BINARY_FILE_VERSION)
    {
        errCode = VSC_ERR_VERSION_MISMATCH;
        if (needToPrint)
        {
            gcoOS_Print("%s: Shader file version 0x%x doesn't match current version 0x%x.",
                        messagePrefix, binaryFileVersion, gcdVIR_SHADER_BINARY_FILE_VERSION);
        }
        return errCode;
    }

    ON_ERROR0(VIR_IO_readUint(buf, &magicNum));
    if (magicNum != VIR_IO_getMagicNum())
    {
        errCode = VSC_ERR_VERSION_MISMATCH;
        if (needToPrint)
        {
            gcoOS_Print("%s: Shader file 0x%x doesn't match current file 0x%x.",
                        messagePrefix, binaryFileVersion, gcdVIR_SHADER_BINARY_FILE_VERSION);
        }
        return errCode;
    }

    ON_ERROR0(VIR_IO_readUint(buf, &chipModel));
    if (chipModel != gcGetHWCaps()->chipModel)
    {
        errCode = VSC_ERR_VERSION_MISMATCH;
        if (needToPrint)
        {
            gcoOS_Print("%s: Shader file chipModel 0x%x doesn't match current chipModel 0x%x.",
                        messagePrefix, chipModel, gcGetHWCaps()->chipModel);
        }
        return errCode;
    }
    ON_ERROR0(VIR_IO_readUint(buf, &chipRevision));
    if (chipRevision != gcGetHWCaps()->chipRevision)
    {
        errCode = VSC_ERR_VERSION_MISMATCH;
        if (needToPrint)
        {
            gcoOS_Print("%s: Shader file chipRevision 0x%x doesn't match current chipRevision 0x%x.",
                        messagePrefix, chipRevision, gcGetHWCaps()->chipRevision);
        }
        return errCode;
    }

    ON_ERROR0(VIR_IO_readInt(buf, (gctINT*)&pShader->clientApiVersion));
    ON_ERROR0(VIR_IO_readUint(buf, &pShader->_id));
    ON_ERROR0(VIR_IO_readUint(buf, &pShader->_constVectorId));
    ON_ERROR0(VIR_IO_readUint(buf, &pShader->_dummyUniformCount));
    ON_ERROR0(VIR_IO_readUint(buf, &pShader->_orgTempCount));
    ON_ERROR0(VIR_IO_readUint(buf, &pShader->_tempRegCount));
    ON_ERROR0(VIR_IO_readUint(buf, &pShader->_anonymousNameId));
    ON_ERROR0(VIR_IO_readInt(buf, (gctINT*)&pShader->shLevel));
    ON_ERROR0(VIR_IO_readInt(buf, (gctINT*)&pShader->shaderKind));
    ON_ERROR0(VIR_IO_readInt(buf, (gctINT*)&pShader->flags));
    ON_ERROR0(VIR_IO_readInt(buf, (gctINT*)&pShader->flagsExt1));
    ON_ERROR0(VIR_IO_readUint(buf, &pShader->compilerVersion[0]));
    ON_ERROR0(VIR_IO_readUint(buf, &pShader->compilerVersion[1]));
    ON_ERROR0(VIR_IO_readInt(buf, &pShader->constUniformBlockIndex));
    ON_ERROR0(VIR_IO_readInt(buf, &pShader->defaultUniformBlockIndex));
    ON_ERROR0(VIR_IO_readUint(buf, &pShader->maxKernelFunctionArgs));
    ON_ERROR0(VIR_IO_readUint(buf, &pShader->privateMemorySize));
    ON_ERROR0(VIR_IO_readUint(buf, &pShader->localMemorySize));

    ON_ERROR0(VIR_IO_readInt(buf, &pShader->constUBOSize));
    if (pShader->constUBOSize)
    {
        pShader->constUBOData = (gctUINT32 *)vscMM_Alloc(memPool, pShader->constUBOSize * 16);
        if (pShader->constUBOData == gcvNULL)
        {
            ON_ERROR0(VSC_ERR_OUT_OF_MEMORY);
        }

        errCode = VIR_IO_readBlock(buf, (gctCHAR *)pShader->constUBOData, pShader->constUBOSize * 16);
        ON_ERROR(errCode, "Fail to read constUBOData, sz: %d.", pShader->constUBOSize * 16);
    }

    ON_ERROR0(VIR_IO_readUint(buf, &pShader->constantMemorySize));
    if (pShader->constantMemorySize > 0)
    {
        pShader->constantMemoryBuffer = (gctCHAR *)vscMM_Alloc(memPool, pShader->constantMemorySize);
        if (pShader->constantMemoryBuffer == gcvNULL)
        {
            ON_ERROR0(VSC_ERR_OUT_OF_MEMORY);
        }
        errCode = VIR_IO_readBlock(buf, (gctCHAR *)pShader->constantMemoryBuffer, pShader->constantMemorySize);
        ON_ERROR(errCode, "Fail to read constantMemoryBuffer, sz: %d.", pShader->constantMemorySize);
    }

    errCode = VIR_IO_readIdList(buf, &pShader->attributes);
    ON_ERROR(errCode, "Fail to read attributes id list.");

    errCode = VIR_IO_readIdList(buf, &pShader->outputs);
    ON_ERROR(errCode, "Fail to read outputs id list.");

    errCode = VIR_IO_readIdList(buf, &pShader->outputVregs);
    ON_ERROR(errCode, "Fail to read outputVregs id list.");

    errCode = VIR_IO_readIdList(buf, &pShader->perpatchInput);
    ON_ERROR(errCode, "Fail to read perpatchInput id list.");

    errCode = VIR_IO_readIdList(buf, &pShader->perpatchOutput);
    ON_ERROR(errCode, "Fail to read perpatchOutput id list.");

    errCode = VIR_IO_readIdList(buf, &pShader->perpatchOutputVregs);
    ON_ERROR(errCode, "Fail to read perpatchOutputVregs id list.");

    errCode = VIR_IO_readIdList(buf, &pShader->buffers);
    ON_ERROR(errCode, "Fail to read buffers id list.");

    errCode = VIR_IO_readIdList(buf, &pShader->uniforms);
    ON_ERROR(errCode, "Fail to read uniforms id list.");

    ON_ERROR0(VIR_IO_readUint(buf, &pShader->uniformVectorCount));
    ON_ERROR0(VIR_IO_readInt(buf, &pShader->samplerIndex));
    ON_ERROR0(VIR_IO_readUint(buf, &pShader->baseSamplerId));
    ON_ERROR0(VIR_IO_readInt(buf, &pShader->samplerBaseOffset));

    /* save layout info */
    switch (pShader->shaderKind)
    {
    case VIR_SHADER_COMPUTE:
        VIR_IO_readBlock(buf, (gctCHAR *)&pShader->shaderLayout.compute, sizeof(VIR_ComputeLayout));
        break;
    case VIR_SHADER_TESSELLATION_CONTROL:
        VIR_IO_readBlock(buf, (gctCHAR *)&pShader->shaderLayout.tcs, sizeof(VIR_TCSLayout));
        break;
    case VIR_SHADER_TESSELLATION_EVALUATION:
        VIR_IO_readBlock(buf, (gctCHAR *)&pShader->shaderLayout.tes, sizeof(VIR_TESLayout));
        break;
    case VIR_SHADER_GEOMETRY:
        VIR_IO_readBlock(buf, (gctCHAR *)&pShader->shaderLayout.geo, sizeof(VIR_GEOLayout));
        break;
    default:
        break;
    }

    errCode = VIR_IO_readIdList(buf, &pShader->variables);
    ON_ERROR(errCode, "Fail to read variables id list.");

    errCode = VIR_IO_readIdList(buf, &pShader->sharedVariables);
    ON_ERROR(errCode, "Fail to read shared variables id list.");

    errCode = VIR_IO_readIdList(buf, &pShader->uniformBlocks);
    ON_ERROR(errCode, "Fail to read uniformBlocks id list.");

    errCode = VIR_IO_readIdList(buf, &pShader->storageBlocks);
    ON_ERROR(errCode, "Fail to read storageBlocks id list.");

    errCode = VIR_IO_readIdList(buf, &pShader->ioBlocks);
    ON_ERROR(errCode, "Fail to read ioBlocks id list.");

    errCode = VIR_IO_readIdList(buf, &pShader->moduleProcesses);
    ON_ERROR(errCode, "Fail to read module process id list.");

    /* LTC info */
    ON_ERROR0(VIR_IO_readInt(buf, &pShader->ltcUniformCount));
    if (pShader->ltcUniformCount)
    {
        ON_ERROR0(VIR_IO_readUint(buf, &pShader->ltcUniformBegin));
        ON_ERROR0(VIR_IO_readUint(buf, &pShader->ltcInstructionCount));

        if ((gctUINT64)pShader->ltcUniformCount * sizeof(gctINT) > gcvMAXUINT32)
        {
            ON_ERROR0(VSC_ERR_OUT_OF_MEMORY);
        }
        else
        {
            pShader->ltcCodeUniformIndex =
                (gctINT32 *)vscMM_Alloc(memPool, pShader->ltcUniformCount * sizeof(gctINT));
        }
        if (pShader->ltcCodeUniformIndex == gcvNULL)
        {
            ON_ERROR0(VSC_ERR_OUT_OF_MEMORY);
        }
        for (i = 0; i < pShader->ltcInstructionCount; i++)
        {
            ON_ERROR0(VIR_IO_readInt(buf, &pShader->ltcCodeUniformIndex[i]));
        }

        if ((gctUINT64)pShader->ltcInstructionCount * sizeof(VIR_Instruction) > gcvMAXUINT32)
        {
            ON_ERROR0(VSC_ERR_OUT_OF_MEMORY);
        }
        else
        {
            pShader->ltcExpressions =
                (VIR_Instruction *)vscMM_Alloc(memPool, pShader->ltcInstructionCount * sizeof(VIR_Instruction));
        }

        if (pShader->ltcCodeUniformIndex == gcvNULL)
        {
            ON_ERROR0(VSC_ERR_OUT_OF_MEMORY);
        }
        for (i = 0; i < pShader->ltcInstructionCount; i++)
        {
            errCode = VIR_IO_readInst(buf, &pShader->ltcExpressions[i]);
            ON_ERROR(errCode, "Fail to read ltcExpressions[i].", i);
        }
    }

    ON_ERROR0(VIR_IO_readUint(buf, &pShader->optimizationOption));

    /* Source code string */
    ON_ERROR0(VIR_IO_readUint(buf, &pShader->sourceLength));
    if (pShader->sourceLength)
    {
        /* write the string */
        errCode = VIR_IO_readBlock(buf, pShader->source, pShader->sourceLength);

        /* decode the string */
        for (i = 0; i < pShader->sourceLength; i++)
        {
            pShader->source[i] ^= SOURCE_ENCODE_CHAR;
        }
        ON_ERROR(errCode, "Fail to read source.");
    }

    ON_ERROR0(VIR_IO_readUint(buf, &pShader->replaceIndex));
    ON_ERROR0(VIR_IO_readBlock(buf, (gctCHAR *)pShader->memoryAccessFlag, sizeof(pShader->memoryAccessFlag)));
    ON_ERROR0(VIR_IO_readBlock(buf, (gctCHAR *)pShader->flowControlFlag, sizeof(pShader->flowControlFlag)));
    ON_ERROR0(VIR_IO_readBlock(buf, (gctCHAR *)pShader->texldFlag, sizeof(pShader->texldFlag)));
    ON_ERROR0(VIR_IO_readUint(buf, (gctUINT*)&pShader->vsPositionZDependsOnW));
    ON_ERROR0(VIR_IO_readUint(buf, (gctUINT*)&pShader->psHasDiscard));
    ON_ERROR0(VIR_IO_readUint(buf, (gctUINT*)&pShader->useEarlyFragTest));
    ON_ERROR0(VIR_IO_readUint(buf, (gctUINT*)&pShader->hasDsx));
    ON_ERROR0(VIR_IO_readUint(buf, (gctUINT*)&pShader->hasDsy));
    ON_ERROR0(VIR_IO_readUint(buf, (gctUINT*)&pShader->useLastFragData));
    ON_ERROR0(VIR_IO_readUint(buf, (gctUINT*)&pShader->__IsDual16Shader));
    ON_ERROR0(VIR_IO_readUint(buf, (gctUINT*)&pShader->__IsMasterDual16Shader));
    ON_ERROR0(VIR_IO_readUint(buf, (gctUINT*)&pShader->packUnifiedSampler));
    ON_ERROR0(VIR_IO_readUint(buf, (gctUINT*)&pShader->fullUnifiedUniforms));
    ON_ERROR0(VIR_IO_readUint(buf, (gctUINT*)&pShader->needToAdjustSamplerPhysical));
    ON_ERROR0(VIR_IO_readUint(buf, (gctUINT*)&pShader->_enableDefaultUBO));

    errCode = VIR_IO_readStringTable(buf, &pShader->stringTable);
    ON_ERROR(errCode, "Fail to read string table.");

    ON_ERROR0(VIR_IO_readUint(buf, &uVal));
    if (uVal != STRTBL_SIG)
    {
        gcmASSERT(gcvFALSE);
    }

    errCode = VIR_IO_readTypeTable(buf, &pShader->typeTable);
    ON_ERROR(errCode, "Fail to read type table.");

    ON_ERROR0(VIR_IO_readUint(buf, &uVal));
    if (uVal != TYTBL_SIG)
    {
        gcmASSERT(gcvFALSE);
    }

    errCode = VIR_IO_readConstTable(buf, &pShader->constTable);
    ON_ERROR(errCode, "Fail to read const table.");

    errCode = VIR_IO_readSymTable(buf, &pShader->symTable);
    ON_ERROR(errCode, "Fail to read sym table.");

    ON_ERROR0(VIR_IO_readUint(buf, &uVal));
    if (uVal != SYMTBL_SIG)
    {
        gcmASSERT(gcvFALSE);
    }

    /* Transform feedback varyings */
    errCode = VIR_IO_readTransformFeedback(buf, &pShader->transformFeedback);
    ON_ERROR(errCode, "Fail to read transformFeedback.");

    /* read functions */
    {
        gctUINT        count = 0;
        gctUINT        val;

        /* read function symId and create VIR_Function for the symbol */
        do
        {
            VIR_Function * func = gcvNULL;
            VIR_Symbol *   sym;

            ON_ERROR0(VIR_IO_readUint(buf, &val));
            if (val == VIR_INVALID_ID)
            {
                break;
            }
            sym = VIR_Shader_GetSymFromId(buf->shader, val);
            gcmASSERT(sym != gcvNULL);
            ON_ERROR0(VIR_Shader_AddFunctionContent(buf->shader, sym, &func, gcvTRUE));
        } while (1);

        /* read the function bodies */
        do
        {
            VIR_Function * func = gcvNULL;
            VIR_Symbol *   sym;

            ON_ERROR0(VIR_IO_readUint(buf, &val));
            if (val != FUNC_SIG)
            {
                break;
            }
            ON_ERROR0(VIR_IO_readUint(buf, &val));
            sym = VIR_Shader_GetSymFromId(buf->shader, val);
            gcmASSERT(sym != gcvNULL);
            func = VIR_Symbol_GetFunction(sym);
            gcmASSERT(func != gcvNULL);
            errCode = VIR_IO_readFunction(buf, func);
            ON_ERROR(errCode, "Fail to read function.");
            ON_ERROR0(VIR_IO_readUint(buf, &val));
            /* check ENDF_SIG and function count */
            gcmASSERT(val == ENDF_SIG);
            count++;
        } while (1);
        if (count != val)
        {
            gcmASSERT(gcvFALSE);
        }
    }

    ON_ERROR0(VIR_IO_readUint(buf, &uVal));
    pShader->RAEnabled = uVal;
    ON_ERROR0(VIR_IO_readUint(buf, &uVal));
    pShader->hwRegAllocated = uVal;
    ON_ERROR0(VIR_IO_readUint(buf, &uVal));
    pShader->hwRegWatermark = uVal;
    ON_ERROR0(VIR_IO_readUint(buf, &uVal));
    pShader->constRegAllocated = uVal;
    ON_ERROR0(VIR_IO_readUint(buf, &uVal));
    pShader->remapRegStart = uVal;
    ON_ERROR0(VIR_IO_readUint(buf, &uVal));
    pShader->remapChannelStart = (gctUINT8)uVal;
    ON_ERROR0(VIR_IO_readUint(buf, &uVal));
    pShader->sampleMaskIdRegStart = uVal;
    ON_ERROR0(VIR_IO_readUint(buf, &uVal));
    pShader->sampleMaskIdChannelStart = uVal;
    ON_ERROR0(VIR_IO_readUint(buf, &uVal));
    pShader->hasRegisterSpill = uVal;
    ON_ERROR0(VIR_IO_readUint(buf, &uVal));
    pShader->llSlotForSpillVidmem = uVal;
    ON_ERROR0(VIR_IO_readUint(buf, &uVal));
    pShader->hasCRegSpill = uVal;
    ON_ERROR0(VIR_IO_readUint(buf, &uVal));
    pShader->useHwManagedLS = uVal;

    ON_ERROR0(VIR_IO_readBlock(buf, (gctCHAR *)pShader->psInputPosCompValid, sizeof(pShader->psInputPosCompValid)));

    ON_ERROR0(VIR_IO_readBlock(buf, (gctCHAR *)pShader->psInputPCCompValid, sizeof(pShader->psInputPCCompValid)));

    ON_ERROR0(VIR_IO_readUint(buf, &uVal));
    pShader->inLinkedShaderStage = (VIR_ShaderKind)uVal;
    ON_ERROR0(VIR_IO_readUint(buf, &uVal));
    pShader->outLinkedShaderStage = (VIR_ShaderKind)uVal;
    ON_ERROR0(VIR_IO_readUint(buf, &uVal));
    VIR_Shader_SetKernelNameId(pShader, (VIR_NameId)uVal);
    ON_ERROR0(VIR_IO_readUint(buf, &uVal));
    if (uVal != ENDS_SIG)
    {
        gcmASSERT(gcvFALSE);
        errCode = VSC_ERR_INVALID_DATA;
        ON_ERROR(errCode, "Invalid end of shader signature 0x%x.", uVal);
    }

    /* Now we can update the hostFunction since we have all information we need. */
    VIR_IO_UpdateHostFunction(buf->shader, &buf->localSymbolList);

OnError:
    return errCode;
}

/*******************************************************************************
**                              VIR_Shader_Copy
********************************************************************************
**
**    Copy a VIR_Shader object.
**
**    INPUT:
**
**        VIR_Shader Shader
**            Pointer to a VIR_Shader object.
**
**      VIR_Shader Source
**          Pointer to a VIR_Shader object that will be copied.
**
**    OUTPUT:
**
**        Nothing.
*/

VSC_ErrCode
VIR_CopyBlock(gctCHAR *Dest, gctCHAR *Source, gctUINT Sz)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    if (Sz!=0)
    {
        gcoOS_MemCopy(Dest, Source, Sz);
    }
    return errCode;
}

void VIR_CopyHashTable(
    VIR_CopyContext *    Ctx,
    VSC_BLOCK_TABLE *    pToBlockTbl,
    VSC_HASH_TABLE *     pDstHT,
    VSC_HASH_TABLE *     pSrcHT,
    GET_KEY_FROM_VAL     fpGetKey
    )
{
#ifdef _SANITY_CHECK
    gctINT      count;
#endif
    gcmASSERT(pSrcHT && pDstHT);

    if (pDstHT->tableSize > 0)
    {
        vscHTBL_Reset(pDstHT);
    }

#ifdef _SANITY_CHECK
    count = vscHTBL_CountItems(pSrcHT);
    if (count != (gctINT)pSrcHT->itemCount)
    {
        ON_ERROR(VSC_ERR_INVALID_DATA, "HashTable damaged: items doesn't match itemCount");
    }
#endif
    if (pSrcHT->tableSize > 0)
    {
        VSC_HASH_NODE* pSrcHashNode;
        VSC_HASH_ITERATOR iter;
        void * newKey;
        vscHTBLIterator_Init(&iter, pSrcHT);
        for (pSrcHashNode = vscHTBLIterator_First(&iter); pSrcHashNode != gcvNULL; pSrcHashNode = vscHTBLIterator_Next(&iter))
        {
            void* pVal = vscHTBL_DirectGet(pSrcHT, pSrcHashNode->pHashKey);
            newKey = fpGetKey(pToBlockTbl, pVal);
            vscHTBL_DirectSet(pDstHT, newKey, pVal);
        }
    }
    return;
#ifdef _SANITY_CHECK
OnError:
    abort();
    return;
#endif
}

VSC_ErrCode
VIR_CopyBlockTable(VIR_CopyContext *    Ctx,
                   VSC_BLOCK_TABLE *    pToBlockTbl,
                   VSC_BLOCK_TABLE *    pFromBlockTbl,
                   COPY_NODE_FP         fp,
                   GET_KEY_FROM_VAL     fpGetKey
                   )
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    gctUINT         flag = pFromBlockTbl->flag;
    VSC_HASH_TABLE *hTbl = (flag & VSC_BLOCK_TABLE_FLAG_HASH_ENTRIES) ?
                             pFromBlockTbl->pHashTable : gcvNULL;
    gctUINT         i, j;

    gcmASSERT(pToBlockTbl != gcvNULL);

    vscBT_Initialize(pToBlockTbl, Ctx->memPool,
                     pFromBlockTbl->flag,
                     pFromBlockTbl->entrySize,
                     pFromBlockTbl->blockSize,
                     pFromBlockTbl->curBlockIdx+1,
                     pFromBlockTbl->pfnGetFreeEntry,
                     hTbl ? hTbl->pfnHashFunc : gcvNULL,
                     hTbl ? hTbl->pfnKeyCmp : gcvNULL,
                     hTbl ? hTbl->tableSize : 0);

    pToBlockTbl->curBlockIdx = pFromBlockTbl->curBlockIdx;
    pToBlockTbl->nextOffsetInCurBlock = pFromBlockTbl->nextOffsetInCurBlock;
    pToBlockTbl->firstFreeEntry = pFromBlockTbl->firstFreeEntry;

    if (pToBlockTbl->curBlockIdx == 0 && pToBlockTbl->nextOffsetInCurBlock == 0)
    {
        /* empty table */
        return errCode;
    }
    /* copy ppBlockArray */
    for (i = 0; i <= pFromBlockTbl->curBlockIdx; i++)
    {
        pToBlockTbl->ppBlockArray[i] =
            (VSC_BT_BLOCK_PTR)vscMM_Alloc(pToBlockTbl->pMM,
                                          pToBlockTbl->blockSize);
        if (pToBlockTbl->ppBlockArray[i] == gcvNULL)
        {
            goto OnError;
        }
        gcoOS_MemCopy(pToBlockTbl->ppBlockArray[i],
                      pFromBlockTbl->ppBlockArray[i],
                      pFromBlockTbl->blockSize);
        if (fp)
        {
            for (j=0; j < pToBlockTbl->entryCountPerBlock; j++)
            {
                gctUINT offset =  j * pToBlockTbl->entrySize;
                if (i == pToBlockTbl->curBlockIdx && offset >= pToBlockTbl->nextOffsetInCurBlock)
                {
                    /* skip unused part of last block  */
                    break;
                }
                ON_ERROR0(fp(Ctx, pToBlockTbl->ppBlockArray[i] + offset));
            }
        }
    }

    /* copy hash table */
    if (hTbl != gcvNULL)
    {
        VIR_CopyHashTable(Ctx, pToBlockTbl, pToBlockTbl->pHashTable, hTbl, fpGetKey);
    }
OnError:
    return errCode;
}

VSC_ErrCode
VIR_Copy_PatchBlockTable(VIR_CopyContext *   Ctx,
                    VSC_BLOCK_TABLE *        pBlockTbl,
                    PATCH_NODE_FP            fp)
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    gctUINT         i, j;

    gcmASSERT(pBlockTbl != gcvNULL);
    gcmASSERT(fp != gcvNULL);

    if (pBlockTbl->curBlockIdx == 0 && pBlockTbl->nextOffsetInCurBlock == 0)
    {
        /* empty table */
        return errCode;
    }
    /* patch ppBlockArray */
    for (i = 0; i <= pBlockTbl->curBlockIdx; i++)
    {
        if (pBlockTbl->ppBlockArray[i] == gcvNULL)
        {
            goto OnError;
        }
        for (j=0; j < pBlockTbl->entryCountPerBlock; j++)
        {
            gctUINT offset =  j * pBlockTbl->entrySize;
            if (i == pBlockTbl->curBlockIdx && offset >= pBlockTbl->nextOffsetInCurBlock)
            {
                /* skip unused part of last block  */
                break;
            }
            ON_ERROR0(fp(Ctx, pBlockTbl->ppBlockArray[i] + offset));
        }
    }

OnError:
    return errCode;
}

void * VIR_Copy_GetEntryFromId(VSC_BLOCK_TABLE *pTbl, VIR_Id Val)
{
    return (void *)VIR_GetEntryFromId(pTbl, Val);
}

VSC_ErrCode
VIR_CopyStringTable(VIR_CopyContext *Ctx, VIR_StringTable *pToStringTbl, VIR_StringTable* pFromStringTbl)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    /* read bolcktable */
    errCode = VIR_CopyBlockTable(Ctx, pToStringTbl, pFromStringTbl,
                                 gcvNULL, (GET_KEY_FROM_VAL)VIR_Copy_GetEntryFromId);
    ON_ERROR(errCode, "Failed to copy string table");

OnError:
    return errCode;
}

VSC_ErrCode
VIR_CopyTypeTable(VIR_CopyContext *Ctx, VIR_TypeTable* pToTypeTbl, VIR_TypeTable* pFromTypeTbl)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VIR_CopyBlockTable(Ctx, pToTypeTbl, pFromTypeTbl,
                                 (COPY_NODE_FP)VIR_Copy_FixType,
                                 (GET_KEY_FROM_VAL)VIR_Copy_GetEntryFromId);
    ON_ERROR(errCode, "Failed to copy type table");
OnError:
    return errCode;
}

VSC_ErrCode
VIR_CopyLabelTable(VIR_CopyContext *Ctx, VIR_LabelTable* pToLabelTbl, VIR_LabelTable* pFromLabelTbl)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VIR_CopyBlockTable(Ctx, pToLabelTbl, pFromLabelTbl,
                                 (COPY_NODE_FP)VIR_Copy_FixLabel,
                                 (GET_KEY_FROM_VAL)VIR_Copy_GetEntryFromId);
    ON_ERROR(errCode, "Failed to copy label table");

OnError:
    return errCode;
}

VSC_ErrCode
VIR_CopyConstTable(VIR_CopyContext *Ctx, VIR_ConstTable* pToConstTbl, VIR_ConstTable* pFromConstTbl)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VIR_CopyBlockTable(Ctx, pToConstTbl, pFromConstTbl,
                                 (COPY_NODE_FP)gcvNULL,
                                 (GET_KEY_FROM_VAL)VIR_Copy_GetEntryFromId);
    ON_ERROR(errCode, "Failed to copy const table");

OnError:
    return errCode;
}

VSC_ErrCode
VIR_CopyOperandTable(VIR_CopyContext *Ctx, VIR_OperandTable* pToOperandTbl, VIR_OperandTable* pFromOperandTbl)
{
    VSC_ErrCode errCode = VIR_CopyBlockTable(Ctx, pToOperandTbl, pFromOperandTbl,
                                             (COPY_NODE_FP)gcvNULL,
                                             (GET_KEY_FROM_VAL)VIR_Copy_GetEntryFromId);
    return errCode;
}

VSC_ErrCode
VIR_CopySymTable(VIR_CopyContext *Ctx, VIR_SymTable* pToSymTbl, VIR_SymTable* pFromSymTbl)
{
    VSC_ErrCode errCode =
        VIR_CopyBlockTable(Ctx, pToSymTbl, pFromSymTbl,
                           (COPY_NODE_FP)VIR_Copy_FixSymbol,
                           (GET_KEY_FROM_VAL)VIR_GetSymFromId);
    return errCode;
}


VSC_ErrCode
VIR_CopyNewIdList(VIR_CopyContext *Ctx, VIR_IdList** pToIdList, VIR_IdList* pFromIdList, gctBOOL Create)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_IdList* list;
    if (Create)
    {
        list = (VIR_IdList *)vscMM_Alloc(Ctx->memPool, sizeof(VIR_IdList));

        if (list == gcvNULL)
        {
            return  VSC_ERR_OUT_OF_MEMORY;
        }
        *pToIdList = list;
    }
    else
    {
        list = *pToIdList;
        gcmASSERT(list != gcvNULL);
    }

    ON_ERROR0(VIR_CopyIdList(Ctx, list, pFromIdList));

OnError:
    return errCode;
}

VSC_ErrCode
VIR_CopyIdList(VIR_CopyContext *Ctx, VIR_IdList* pToIdList, VIR_IdList* pFromIdList)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    gcmASSERT(pToIdList != gcvNULL);

    VIR_IdList_Init(Ctx->memPool, (pFromIdList->count) > 0 ? pFromIdList->count : 1, &pToIdList);

    /* copy ids */
    if (pFromIdList->count > 0)
    {
        gcoOS_MemCopy(pToIdList->ids, (gctCHAR *)pFromIdList->ids, pFromIdList->count *sizeof(VIR_Id));
    }
    pToIdList->count = pFromIdList->count;

    return errCode;
}

VSC_ErrCode
VIR_CopyUniform(VIR_CopyContext *Ctx, VIR_Uniform* pToUniform, VIR_Uniform* pFromUniform)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    gcoOS_MemCopy(pToUniform, (gctCHAR *)pFromUniform, sizeof(VIR_Uniform));

    return errCode;
}

VSC_ErrCode
VIR_CopyImageSampler(VIR_ImageSampler* pToImageSampler, VIR_ImageSampler* pFromImageSampler)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    Copy_Field(pToImageSampler, pFromImageSampler, imageNum);
    Copy_Field(pToImageSampler, pFromImageSampler, isConstantSamplerType);
    Copy_Field(pToImageSampler, pFromImageSampler, samplerType);

    return errCode;
}

VSC_ErrCode
VIR_CopyKernelInfo(VIR_CopyContext *Ctx, VIR_KernelInfo** pToKernelInfo, VIR_KernelInfo* pFromKernelInfo)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    if (pFromKernelInfo == gcvNULL)
    {
        *pToKernelInfo = gcvNULL;
    }
    else
    {
        VIR_KernelInfo *kInfo;
        VSC_MM *        memPool    = Ctx->memPool;
        kInfo = (VIR_KernelInfo *)vscMM_Alloc(memPool, sizeof(VIR_KernelInfo));
        if (kInfo == gcvNULL)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
        *pToKernelInfo = kInfo;

        Copy_Field(*pToKernelInfo, pFromKernelInfo, kernelName);
        Copy_Field(*pToKernelInfo, pFromKernelInfo, localMemorySize);
        Copy_Field(*pToKernelInfo, pFromKernelInfo, samplerIndex);
        Copy_Field(*pToKernelInfo, pFromKernelInfo, isMain);
        ON_ERROR0(VIR_CopyIdList(Ctx, &(*pToKernelInfo)->uniformArguments, &pFromKernelInfo->uniformArguments));
        ON_ERROR0(VIR_CopyValueList(Ctx, &(*pToKernelInfo)->imageSamplers, &pFromKernelInfo->imageSamplers, (COPY_NODE_FP)0));
        ON_ERROR0(VIR_CopyValueList(Ctx, &(*pToKernelInfo)->properties, &pFromKernelInfo->properties, (COPY_NODE_FP)0));
    }

OnError:
    return errCode;
}

VSC_ErrCode
VIR_CopyInstList(
    VIR_CopyContext *       Ctx,
    VIR_InstList *          pToInstList,
    VIR_InstList *          pFromInstList
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Instruction *   inst;
    VIR_InstIterator    instIter;

    VIR_InstIterator_Init(&instIter, pFromInstList);
    inst = (VIR_Instruction *)VIR_InstIterator_First(&instIter);

    for (; inst != gcvNULL; inst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter))
    {
        VIR_Instruction *   newInst;

        ON_ERROR0(VIR_Function_AddInstruction(Ctx->curToFunction, VIR_OP_NOP, VIR_TYPE_UNKNOWN, &newInst));
        ON_ERROR0(VIR_CopyInst(Ctx, newInst, inst));
    }

    ON_ERROR0(VIR_Function_BuildLabelLinks(Ctx->curToFunction));
OnError:
    return errCode;
}

VSC_ErrCode
VIR_CopyFunction(VIR_CopyContext *Ctx, VIR_Function* pToFunction, VIR_Function* pFromFunction)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    Ctx->curFromFunction = pFromFunction;
    Ctx->curToFunction   = pToFunction;

    VIR_Function_SetShader(pToFunction, Ctx->toShader);
    VIR_Function_SetFuncBlock(pToFunction, gcvNULL);

    Copy_Field(pToFunction, pFromFunction, _lastInstId);
    Copy_Field(pToFunction, pFromFunction, _labelId);
    Copy_Field(pToFunction, pFromFunction, funcSym);
    Copy_Field(pToFunction, pFromFunction, flags);
    Copy_Field(pToFunction, pFromFunction, maxCallDepth);
    Copy_Field(pToFunction, pFromFunction, tempIndexStart);
    Copy_Field(pToFunction, pFromFunction, tempIndexCount);
    Copy_Field(pToFunction, pFromFunction, die);

    ON_ERROR0(VIR_CopySymTable(Ctx, &pToFunction->symTable, &pFromFunction->symTable));
    ON_ERROR0(VIR_CopyLabelTable(Ctx, &pToFunction->labelTable, &pFromFunction->labelTable));
    ON_ERROR0(VIR_CopyOperandTable(Ctx, &pToFunction->operandTable, &pFromFunction->operandTable));

    ON_ERROR0(VIR_CopyIdList(Ctx, &pToFunction->localVariables, &pFromFunction->localVariables));
    ON_ERROR0(VIR_CopyIdList(Ctx, &pToFunction->paramters, &pFromFunction->paramters));
    ON_ERROR0(VIR_CopyIdList(Ctx, &pToFunction->temps, &pFromFunction->temps));

    ON_ERROR0(VIR_CopyKernelInfo(Ctx, &pToFunction->kernelInfo, pFromFunction->kernelInfo));

    /* copy instructions and label table*/
    ON_ERROR0(VIR_CopyInstList(Ctx, &pToFunction->instList, &pFromFunction->instList));

    pToFunction->pFuncBlock = gcvNULL;

    Ctx->curFromFunction = gcvNULL;
    Ctx->curToFunction   = gcvNULL;

OnError:
    return errCode;
}

VSC_ErrCode
VIR_CopyUniformBlock(VIR_CopyContext * Ctx, VIR_UniformBlock* pToUniformBlock, VIR_UniformBlock* pFromUniformBlock)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctUINT i;

    Copy_Field(pToUniformBlock, pFromUniformBlock, sym);

    Copy_Field(pToUniformBlock, pFromUniformBlock, flags);
    Copy_Field(pToUniformBlock, pFromUniformBlock, blockIndex);

    Copy_Field(pToUniformBlock, pFromUniformBlock, baseAddr);
    Copy_Field(pToUniformBlock, pFromUniformBlock, blockSize);
    Copy_Field(pToUniformBlock, pFromUniformBlock, uniformCount);

    if (pFromUniformBlock->uniformCount > 0)
    {
        pToUniformBlock->uniforms = (VIR_Uniform **)vscMM_Alloc(Ctx->memPool,
                                          sizeof(VIR_Uniform*) * pFromUniformBlock->uniformCount);
        if (pToUniformBlock->uniforms == gcvNULL)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
        for (i=0; i<pToUniformBlock->uniformCount; i++)
        {
            /* to patch */
            pToUniformBlock->uniforms[i] = pFromUniformBlock->uniforms[i];
        }
    }
    else
    {
        pToUniformBlock->uniforms = gcvNULL;
    }
    return errCode;
}

VSC_ErrCode
VIR_CopyStorageBlock(VIR_CopyContext * Ctx, VIR_StorageBlock* pToStorageBlock, VIR_StorageBlock* pFromStorageBlock)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctUINT i;

    Copy_Field(pToStorageBlock, pFromStorageBlock, sym);

    Copy_Field(pToStorageBlock, pFromStorageBlock, flags);
    Copy_Field(pToStorageBlock, pFromStorageBlock, blockIndex);
    Copy_Field(pToStorageBlock, pFromStorageBlock, baseAddr);
    Copy_Field(pToStorageBlock, pFromStorageBlock, blockSize);
    Copy_Field(pToStorageBlock, pFromStorageBlock, variableCount);

    if (pFromStorageBlock->variableCount > 0)
    {
        pToStorageBlock->variables = (VIR_SymId *)vscMM_Alloc(Ctx->memPool,
                                          sizeof(VIR_SymId) * pFromStorageBlock->variableCount);
        for (i=0; i<pFromStorageBlock->variableCount; i++)
        {
            pToStorageBlock->variables[i] = pFromStorageBlock->variables[i];
        }
    }
    else
    {
        pToStorageBlock->variables = gcvNULL;
    }
    return errCode;
}

VSC_ErrCode
VIR_CopyIOBlock(VIR_CopyContext * Ctx, VIR_IOBlock* pToIOBlock, VIR_IOBlock* pFromIOBlock)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    Copy_Field(pToIOBlock, pFromIOBlock, sym);
    Copy_Field(pToIOBlock, pFromIOBlock, flags);
    Copy_Field(pToIOBlock, pFromIOBlock, blockIndex);
    Copy_Field(pToIOBlock, pFromIOBlock, blockNameLength);
    Copy_Field(pToIOBlock, pFromIOBlock, instanceNameLength);
    Copy_Field(pToIOBlock, pFromIOBlock, Storage);

    return errCode;
}

VSC_ErrCode
VIR_Copy_FixLabel(VIR_CopyContext * Ctx, VIR_Label* pLabel)
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;

    /* to patch */
    pLabel->defined    = (pLabel->defined);
    pLabel->referenced = gcvNULL;
    return errCode;
}

VSC_ErrCode
VIR_CopyInst(
    VIR_CopyContext *    Ctx,
    VIR_Instruction *    pToInst,
    VIR_Instruction *    pFromInst)
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    VIR_Operand *   opnd;
    VIR_Operand *   newOpnd;
    gctUINT         i;
    VIR_InstHeader  biLstNode = pToInst->biLstNode;   /* save link list info */

    gcoOS_MemCopy(pToInst, (gctCHAR *)pFromInst, sizeof(VIR_Instruction));
    /* restore link list info */
    pToInst->biLstNode = biLstNode;

    VIR_Inst_SetFunction(pToInst, Ctx->curToFunction);

    /* copy dest */
    opnd = VIR_Inst_GetDest(pFromInst);
    if (opnd)
    {
        newOpnd = VIR_Function_GetOperandFromId(Ctx->curToFunction,
                                                VIR_Operand_GetIndex(opnd));
        VIR_Inst_SetDest(pToInst, newOpnd);
        VIR_Copy_FixOperand(Ctx, newOpnd);
    }

    /* copy sources */
    for (i = 0; i < VIR_Inst_GetSrcNum(pFromInst); i++)
    {
        opnd = VIR_Inst_GetSource(pFromInst, i);
        if (opnd)
        {
            newOpnd = VIR_Function_GetOperandFromId(Ctx->curToFunction,
                                                    VIR_Operand_GetIndex(opnd));
            VIR_Inst_SetSource(pToInst, i, newOpnd);
            VIR_Copy_FixOperand(Ctx, newOpnd);
        }
    }

    pToInst->mcInstCount = 0;
    pToInst->mcInst      = gcvNULL;

    if (VIR_Inst_GetOpcode(pToInst) == VIR_OP_LABEL)
    {
        /* patch label definition */
        VIR_Label *label  = VIR_Operand_GetLabel(VIR_Inst_GetDest(pToInst));
        label->defined = pToInst;
    }
    return errCode;
}

/* the old paramList pointer is in pParmPassing */
VSC_ErrCode
VIR_CopyParmPassing(VIR_CopyContext *Ctx, VIR_ParmPassing** pParmPassing)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctUINT uVal;
    gctUINT i;
    VIR_ParmPassing *parm = *pParmPassing;

    if (parm != gcvNULL)
    {
        gctUINT allocSize;
        VIR_ParmPassing *newParm;
        /* create parameter passing array */
        uVal = parm->argNum;
        allocSize = sizeof(VIR_ParmPassing) +
                    ((uVal > 0) ? (uVal - 1) * sizeof (VIR_Operand *) : 0);
        newParm = (VIR_ParmPassing *)vscMM_Alloc(Ctx->memPool, allocSize);

        *pParmPassing = newParm;
        if (newParm == gcvNULL)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
        else
        {
            newParm->argNum = uVal;
            for (i = 0; i<newParm->argNum; i++)
            {
                VIR_Operand * newArg = VIR_Function_GetOperandFromId(Ctx->curToFunction,
                                               VIR_Operand_GetIndex(parm->args[i]));
                newParm->args[i] = newArg;
                ON_ERROR0(VIR_Copy_FixOperand(Ctx, newArg));
            }
        }
    }
OnError:
    return errCode;
}

VSC_ErrCode
VIR_CopyPhiOperandArray(VIR_CopyContext *Ctx, VIR_PhiOperandArray** pPhiOperands)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_PhiOperandArray *phiOperands = *pPhiOperands;
    VIR_PhiOperandArray *newPhiOperands;
    gctUINT i;

    /* create parameter passing array */
    errCode = VIR_Function_NewPhiOperandArray(Ctx->curToFunction, VIR_PhiOperandArray_GetCount(phiOperands), &newPhiOperands);
    if (errCode != VSC_ERR_NONE)
    {
        return errCode;
    }

    for(i = 0; i < VIR_PhiOperandArray_GetCount(phiOperands); i++)
    {
        VIR_PhiOperand* phiOperand = VIR_PhiOperandArray_GetNthOperand(phiOperands, i);
        VIR_PhiOperand* newPhiOperand = VIR_PhiOperandArray_GetNthOperand(newPhiOperands, i);

        VIR_PhiOperand_SetValue(newPhiOperand, VIR_Function_GetOperandFromId(Ctx->curToFunction,
                                        VIR_Operand_GetIndex(VIR_PhiOperand_GetValue(phiOperand))));
        VIR_PhiOperand_SetLabel(newPhiOperand, VIR_Function_GetLabelFromId(Ctx->curToFunction,
                                        VIR_Label_GetId(VIR_PhiOperand_GetLabel(phiOperand))));
        VIR_PhiOperand_SetFlags(newPhiOperand, VIR_PhiOperand_GetFlags(phiOperand));

        ON_ERROR0(VIR_Copy_FixOperand(Ctx, newPhiOperand->value));
    }

    *pPhiOperands = newPhiOperands;
OnError:
    return errCode;
}

VSC_ErrCode
VIR_CopyVarTempRegInfo(VIR_CopyContext *Ctx, VIR_VarTempRegInfo *pVarInfo)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_TypeId * tempRegTypes = pVarInfo->tempRegTypes;

    if (tempRegTypes != gcvNULL)
    {
        gctUINT allocSize;

        /* create tempRegTypes array */
        allocSize = pVarInfo->tempRegCount * sizeof (VIR_TypeId *);
        pVarInfo->tempRegTypes = (VIR_TypeId *)vscMM_Alloc(Ctx->memPool, allocSize);

        if (pVarInfo->tempRegTypes == gcvNULL)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
        else
        {
            gcoOS_MemCopy(pVarInfo->tempRegTypes, (gctCHAR *)tempRegTypes, allocSize);
        }
    }

    return errCode;
}

VSC_ErrCode
VIR_CopyTransformFeedback(VIR_CopyContext *Ctx, VIR_TransformFeedback *toTfb, VIR_TransformFeedback *fromTfb)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    if (fromTfb->varyings)
    {
        ON_ERROR0(VIR_CopyNewIdList(Ctx, &toTfb->varyings, fromTfb->varyings, gcvTRUE));
    }
    else
    {
        toTfb->varyings = gcvNULL;
    }
    Copy_Field(toTfb, fromTfb, bufferMode);
    Copy_Field(toTfb, fromTfb, stateUniformId);
    if (fromTfb->varRegInfos)
    {
        VIR_ValueList_Init(Ctx->memPool,
                           fromTfb->varRegInfos->count,
                           sizeof(VIR_VarTempRegInfo),
                           &toTfb->varRegInfos);
        ON_ERROR0(VIR_CopyValueList(Ctx, toTfb->varRegInfos, fromTfb->varRegInfos,
                                        (COPY_NODE_FP)VIR_CopyVarTempRegInfo));
    }
    else
    {
        toTfb->varRegInfos = gcvNULL;
    }
    Copy_Field(toTfb, fromTfb, totalSize);
    Copy_Field(toTfb, fromTfb, shaderTempCount);
    if (fromTfb->bufferMode == VIR_FEEDBACK_INTERLEAVED)
    {
        Copy_Field(toTfb, fromTfb, feedbackBuffer.interleavedBufUniformId);
    }
    else
    {
        /* VIR_FEEDBACK_SEPARATE mode */
        if (fromTfb->feedbackBuffer.separateBufUniformIds != gcvNULL)
        {
            gctUINT allocSize = sizeof(VIR_UniformId) * fromTfb->shaderTempCount;
            VIR_UniformId * uniformIds = (VIR_UniformId *)vscMM_Alloc(Ctx->memPool, allocSize);
            toTfb->feedbackBuffer.separateBufUniformIds = uniformIds;
            if (uniformIds == gcvNULL)
            {
                errCode = VSC_ERR_OUT_OF_MEMORY;
            }
            else
            {
                gcoOS_MemCopy(uniformIds, (gctCHAR *)fromTfb->feedbackBuffer.separateBufUniformIds, allocSize);
            }
        }
        else
        {
            toTfb->feedbackBuffer.separateBufUniformIds = gcvNULL;
        }
    }
OnError:
    return errCode;
}


VSC_ErrCode
VIR_CopyOperandList(
    VIR_CopyContext *    Ctx,
    VIR_OperandList**    pToOperandList,
    VIR_OperandList*     pOperandList)
{
    VSC_ErrCode       errCode = VSC_ERR_NONE;
    VIR_OperandList * ptr     = pOperandList;
    VIR_OperandList * lastNode = gcvNULL;
    gctUINT           allocSize;

    gcmASSERT(*pToOperandList == gcvNULL);

    while (ptr)
    {
        VIR_OperandList * node;

        allocSize = sizeof(VIR_OperandList);
        node = (VIR_OperandList *)vscMM_Alloc(Ctx->memPool, allocSize);
        node->value = VIR_Function_GetOperandFromId(Ctx->curToFunction, VIR_Operand_GetIndex(ptr->value));
        ON_ERROR0(VIR_Copy_FixOperand(Ctx, node->value));
        node->next = gcvNULL;
        if (*pToOperandList == gcvNULL)
        {
            *pToOperandList = node;
        }
        else
        {
            gcmASSERT(lastNode != gcvNULL);
            lastNode->next = node;
        }
        lastNode = node;
        ptr = ptr->next;
    }
OnError:
    return errCode;
}

VSC_ErrCode
VIR_CopyFunctionList(
    VIR_CopyContext *    Ctx,
    VIR_FunctionList *   pToFuncList,
    VIR_FunctionList *   pFromFuncList
    )
{
    VSC_ErrCode       errCode   = VSC_ERR_NONE;
    VIR_FunctionNode *funcNode  = gcvNULL;
    VIR_FuncIterator  funcIter;

    vscBILST_Initialize(pToFuncList, gcvFALSE);

    VIR_FuncIterator_Init(&funcIter, pFromFuncList);
    /* iterate over functions */
    for(funcNode = VIR_FuncIterator_First(&funcIter); funcNode != gcvNULL;
        funcNode = VIR_FuncIterator_Next(&funcIter))
    {
        VIR_FunctionNode * newFuncNode;
        VIR_Function *     fromFunc = funcNode->function;
        VIR_Function *     toFunc;

        toFunc = VIR_Symbol_GetFunction(VIR_Shader_GetSymFromId(Ctx->toShader,
                                             VIR_Function_GetSymId(fromFunc)));
        gcmASSERT(toFunc != gcvNULL);

        ON_ERROR0(VIR_CopyFunction(Ctx, toFunc, fromFunc));

        newFuncNode = (VIR_FunctionNode *)vscMM_Alloc(Ctx->memPool, sizeof(VIR_FunctionNode));
        if (newFuncNode == gcvNULL)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
        newFuncNode->function = toFunc;
        vscBILST_Append(pToFuncList, (VSC_BI_LIST_NODE*)newFuncNode);

        /* add to kernel function list */
        if (fromFunc->flags & VIR_FUNCFLAG_KERNEL)
        {
            VIR_FunctionNode *kernelFuncNode =
                (VIR_FunctionNode *)vscMM_Alloc(Ctx->memPool, sizeof(VIR_FunctionNode));
            if (kernelFuncNode == gcvNULL)
            {
                return VSC_ERR_OUT_OF_MEMORY;
            }
            kernelFuncNode->function = toFunc;
            vscBILST_Append(&Ctx->toShader->kernelFunctions, (VSC_BI_LIST_NODE*)kernelFuncNode);
        }
    }

OnError:
    return errCode;
}

VSC_ErrCode
VIR_CopyValueList(VIR_CopyContext *Ctx, VIR_ValueList* pToValueList, VIR_ValueList* pFromValueList, COPY_NODE_FP fp)
{
    VSC_ErrCode      errCode = VSC_ERR_NONE;
    gctUINT          i;
    gctUINT          sz;

    pToValueList->memPool = Ctx->memPool;
    Copy_Field(pToValueList, pFromValueList, count);
    Copy_Field(pToValueList, pFromValueList, elemSize);
    Copy_Field(pToValueList, pFromValueList, count);
    pToValueList->allocated = pToValueList->count;

    sz = pToValueList->allocated * pToValueList->elemSize;
    pToValueList->values = (gctCHAR *)vscMM_Alloc(Ctx->memPool, sz);
    if (pToValueList->values == gcvNULL)
    {
        gcmASSERT(gcvFALSE);
        goto OnError;
    }
    ON_ERROR0(VIR_CopyBlock(pToValueList->values, pFromValueList->values, sz));
    if (fp)
    {
        for (i=0; i < pToValueList->count; i++)
        {
            gctCHAR * val = VIR_ValueList_GetValue(pToValueList, i);
            ON_ERROR0(fp(Ctx, val));
        }
    }

OnError:
    return errCode;
}

/* the contents of Operand is already copied, need to fixup the
 * pointers for the new shader */
VSC_ErrCode
VIR_Copy_FixOperand(VIR_CopyContext *Ctx, VIR_Operand* pOperand)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    /* fix u1 */
    switch(VIR_Operand_GetOpKind(pOperand))
    {
    case VIR_OPND_NONE:
    case VIR_OPND_UNDEF:
    case VIR_OPND_UNUSED:
        break;
    case VIR_OPND_IMMEDIATE:
    case VIR_OPND_EVIS_MODIFIER:
    case VIR_OPND_CONST:
    case VIR_OPND_NAME:
    case VIR_OPND_INTRINSIC:
        break;
    case VIR_OPND_PARAMETERS:
        ON_ERROR0(VIR_CopyParmPassing(Ctx, &VIR_Operand_GetParameters(pOperand)));
        break;
    case VIR_OPND_LABEL:
        VIR_Operand_SetLabel(pOperand,
            VIR_GetLabelFromId(Ctx->curToFunction,
                               VIR_Label_GetId(VIR_Operand_GetLabel(pOperand))));
        break;
    case VIR_OPND_FUNCTION:
        VIR_Operand_SetFunc(pOperand,
            VIR_Symbol_GetFunction(VIR_Shader_GetSymFromId(Ctx->toShader,
                                         VIR_Operand_GetFunctionId_(pOperand))));
        break;
    case VIR_OPND_SAMPLER_INDEXING:
    case VIR_OPND_SYMBOL:
    case VIR_OPND_ADDRESS_OF:
        {
            VIR_Symbol * sym = VIR_Function_GetSymFromId(Ctx->curToFunction,
                                      VIR_Operand_GetSymbolId_(pOperand));
            VIR_Operand_SetSym(pOperand, sym);
        }
        break;
    case VIR_OPND_FIELD:
        VIR_Operand_SetFieldBase(pOperand,
            VIR_Function_GetOperandFromId(Ctx->curToFunction,
                 VIR_Operand_GetIndex(VIR_Operand_GetFieldBase(pOperand))));
        break;
    case VIR_OPND_ARRAY:
        {
            VIR_Operand * newOpnd =
                VIR_Function_GetOperandFromId(Ctx->curToFunction,
                     VIR_Operand_GetIndex(VIR_Operand_GetArrayBase(pOperand)));
            VIR_Operand_SetFieldBase(pOperand, newOpnd);
            ON_ERROR0(VIR_Copy_FixOperand(Ctx, newOpnd));
        }
        break;
    case VIR_OPND_TEXLDPARM:
        {
            gctUINT i;
            for (i = 0; i <VIR_TEXLDMODIFIER_COUNT; i++)
            {
                VIR_Operand * opnd = VIR_Operand_GetTexldModifier(pOperand, i);
                if (opnd != gcvNULL)
                {
                    VIR_Operand * newOpnd =
                        VIR_Function_GetOperandFromId(Ctx->curToFunction,
                                                      VIR_Operand_GetIndex(opnd));
                    VIR_Operand_SetTexldModifier(pOperand, i, newOpnd);
                    ON_ERROR0(VIR_Copy_FixOperand(Ctx, newOpnd));
                }
                else
                {
                   VIR_Operand_SetTexldModifier(pOperand, i, gcvNULL);
                }
            }
        }
        break;

    case VIR_OPND_SIZEOF:
    case VIR_OPND_OFFSETOF:
        gcmASSERT(gcvFALSE);  /* need to refine the definition of the operand */
        break;
    case VIR_OPND_PHI:
        ON_ERROR0(VIR_CopyPhiOperandArray(Ctx, &VIR_Operand_GetPhiOperands(pOperand)));
        break;
    default:
        gcmASSERT(0);
        break;
    }
    /* copy u2 */
    switch(VIR_Operand_GetOpKind(pOperand))
    {
    case VIR_OPND_ARRAY:
        {
            VIR_OperandList * arrayIndex = VIR_Operand_GetArrayIndex(pOperand);
            VIR_Operand_SetArrayIndex(pOperand, gcvNULL);
            ON_ERROR0(VIR_CopyOperandList(Ctx, &VIR_Operand_GetArrayIndex(pOperand), arrayIndex));
        }
        break;
    case VIR_OPND_FIELD:
    default:
        break;
    }

OnError:
    return errCode;
}

/* fix symbol */
VSC_ErrCode
VIR_Copy_FixSymbol(VIR_CopyContext * Ctx, VIR_Symbol* pSymbol)
{
    VSC_ErrCode       errCode = VSC_ERR_NONE;
    VIR_SymbolKind    symKind = VIR_Symbol_GetKind(pSymbol);
    VIR_Uniform *     uniform;

    /* fix type */

    /* u0 */
    if (isSymLocal(pSymbol))
    {
        VIR_SymId funcSymId = VIR_Function_GetSymId(VIR_Symbol_GetHostFunction(pSymbol));

        VIR_Symbol_SetHostFunction(pSymbol, (VIR_Function *)gcmINT2PTR(funcSymId));
        _VIR_IO_SymbolListQueue(&Ctx->toShader->pmp.mmWrapper,
                                &Ctx->localSymbolList,
                                pSymbol);
    }
    else
    {
        VIR_Symbol_SetHostShader(pSymbol, Ctx->toShader);
    }
    /* u1 */

    /* u2 */
    switch(symKind)
    {
    case VIR_SYM_UNIFORM:
    case VIR_SYM_SAMPLER:
    case VIR_SYM_SAMPLER_T:
    case VIR_SYM_IMAGE:
    case VIR_SYM_IMAGE_T:
        uniform = (pSymbol)->u2.uniform;
        /* create a new uniform and add it to symbol */
        ON_ERROR0(VIR_Shader_AddSymbolContents(Ctx->toShader, pSymbol, VIR_Uniform_GetID(uniform), gcvFALSE));
        /* copy the contents of uniform */
        ON_ERROR0(VIR_CopyUniform(Ctx, (pSymbol)->u2.uniform, uniform));
        break;
    case VIR_SYM_VARIABLE:
    case VIR_SYM_TEXTURE:
        VIR_Shader_AddSymbolContents(Ctx->toShader, pSymbol, VIR_Symbol_GetVariableVregIndex(pSymbol), gcvFALSE);
        break;
    case VIR_SYM_SBO:
        {
            VIR_StorageBlock* storageBlock =  VIR_Symbol_GetSBO(pSymbol);
            if (storageBlock)
            {
                VIR_Shader_AddSymbolContents(Ctx->toShader, pSymbol, storageBlock->blockIndex, gcvFALSE);
                ON_ERROR0(VIR_CopyStorageBlock(Ctx, VIR_Symbol_GetSBO(pSymbol), storageBlock));
            }
        }
        break;
    case VIR_SYM_UBO:
        {
            VIR_UniformBlock* ubo = VIR_Symbol_GetUBO(pSymbol);
            if (ubo)
            {
                VIR_Shader_AddSymbolContents(Ctx->toShader, pSymbol, ubo->blockIndex, gcvFALSE);
                ON_ERROR0(VIR_CopyUniformBlock(Ctx, VIR_Symbol_GetUBO(pSymbol), ubo));
            }
        }
        break;
    case VIR_SYM_IOBLOCK:
        {
            VIR_IOBlock* iob = VIR_Symbol_GetIOB(pSymbol);
            if (iob)
            {
                VIR_Shader_AddSymbolContents(Ctx->toShader, pSymbol, iob->blockIndex, gcvFALSE);
                ON_ERROR0(VIR_CopyIOBlock(Ctx, VIR_Symbol_GetIOB(pSymbol), iob));
            }
        }
        break;
    case VIR_SYM_FUNCTION:
        {
            VIR_Function * func =  VIR_Symbol_GetFunction(pSymbol);

            if (func != gcvNULL)
            {
                VIR_Function * newFunc;
                ON_ERROR0(VIR_Shader_AddFunctionContent(Ctx->toShader, pSymbol, &newFunc, gcvFALSE));
                VIR_Symbol_SetFunction(pSymbol, newFunc);
            }
        }
        break;
    case VIR_SYM_FIELD:
        if (VIR_Symbol_GetFieldInfo(pSymbol) != gcvNULL)
        {
            VIR_FieldInfo * fi = VIR_Symbol_GetFieldInfo(pSymbol);
            VIR_FieldInfo * newFi;
            newFi = (VIR_FieldInfo *)vscMM_Alloc(Ctx->memPool, sizeof(VIR_FieldInfo));
            if (newFi == gcvNULL)
            {
                return VSC_ERR_OUT_OF_MEMORY;
            }
            VIR_Symbol_SetFieldInfo(pSymbol, newFi);
            ON_ERROR0(VIR_CopyBlock((gctCHAR *)newFi, (gctCHAR *)fi, sizeof(VIR_FieldInfo)));
        }
        else
        {
            VIR_Symbol_SetFieldInfo(pSymbol, gcvNULL);
        }
        break;
    case VIR_SYM_VIRREG:
        {
            VIR_VirRegId virRegId = VIR_Symbol_GetVregIndex(pSymbol);
            /* add <virregId, symId> to shader virreg hash table */
            vscHTBL_DirectSet(VIR_Shader_GetVirRegTable(Ctx->toShader),
                             (void *)(gctUINTPTR_T)virRegId,
                             (void *)(gctUINTPTR_T)VIR_Symbol_GetIndex(pSymbol));
            break;
        }
    case VIR_SYM_TYPE:
    case VIR_SYM_LABEL:
    case VIR_SYM_CONST:
    case VIR_SYM_UNKNOWN:
    default:
        break;
    }
    /* u3 */
    switch(symKind)
    {
    case VIR_SYM_FIELD:
    case VIR_SYM_FUNCTION:
    default:
        break;
    }

    /* u4 */

OnError:
    return errCode;
}

/* fix type */
VSC_ErrCode
VIR_Copy_FixType(VIR_CopyContext * Ctx, VIR_Type* pType)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    if (VIR_Type_isPrimitive(pType))
    {
        return errCode;
    }
    switch (pType->_kind) {
    case VIR_TY_STRUCT:
    case VIR_TY_ENUM:
        {
            VIR_SymIdList *   fields = VIR_Type_GetFields(pType);
            pType->u2.fields = gcvNULL;

            /* field id list */
            if (fields != gcvNULL)
            {
            ON_ERROR0(VIR_CopyNewIdList(Ctx, &pType->u2.fields, fields, gcvTRUE));
        }
        }
        break;
    case VIR_TY_FUNCTION:
        {
            VIR_TypeIdList *  params = VIR_Type_GetParameters(pType);
            pType->u2.params = gcvNULL;
            /* parameter id list */
            ON_ERROR0(VIR_CopyNewIdList(Ctx, &pType->u2.params, params, gcvTRUE));
        }
        break;
    case VIR_TY_POINTER:
    case VIR_TY_ARRAY:
    default:
        break;
    }
OnError:
    return errCode;
}

VSC_ErrCode
VIR_Copy_FixUBOs(
    VIR_CopyContext * Ctx,
    VIR_UBOIdList *   UniformBlocks
    )
{
    VSC_ErrCode          errCode     = VSC_ERR_NONE;
    gctUINT              i;

    for (i=0; i < VIR_IdList_Count(UniformBlocks); i++)
    {
        VIR_Id uboId = VIR_IdList_GetId(UniformBlocks, i);
        VIR_Symbol* virUBOSym = VIR_Shader_GetSymFromId(Ctx->toShader, uboId);
        VIR_UniformBlock* virUBO = VIR_Symbol_GetUBO(virUBOSym);
        gctUINT j;
        for (j=0; j<virUBO->uniformCount; j++)
        {
            VIR_Uniform *uniform = virUBO->uniforms[j];
            VIR_Symbol * sym = VIR_Shader_GetSymFromId(Ctx->toShader, VIR_Uniform_GetSymID(uniform));
            gcmASSERT(sym && VIR_Symbol_isUniform(sym));
            virUBO->uniforms[j] = VIR_Symbol_GetUniform(sym);
        }
    }
    return errCode;
};

VSC_ErrCode
VIR_Shader_Copy(
    IN OUT VIR_Shader  *Shader,
    IN  VIR_Shader     *Source)
{

    VSC_ErrCode          errCode     = VSC_ERR_NONE;
    gctUINT              i;
    VSC_MM *             memPool;
    VIR_CopyContext      context = {gcvNULL, Shader, Source, gcvNULL, gcvNULL };

    QUEUE_INITIALIZE(&context.localSymbolList);

#ifdef _SANITY_CHECK
    {
        VSC_HASH_TABLE  *   pHT;
        /* check if from shader is right object type */
        if (Source->object.type != gcvOBJ_VIR_SHADER)
        {
            ERR_REPORT(VSC_ERR_INVALID_DATA, "Shader damaged: object type is not VIR_SHADER");
            abort();
        }
        /* check sanity of shader's hash tables */
        pHT = Source->stringTable.pHashTable;
        vscHTBL_CountItems(pHT);
        if (pHT && vscHTBL_CountItems(pHT) != (gctINT)pHT->itemCount)
        {
            ERR_REPORT(VSC_ERR_INVALID_DATA, "StringHashTable damaged: items doesn't match itemCount");
            abort();
        }
        pHT = Source->typeTable.pHashTable;
        if (pHT && vscHTBL_CountItems(pHT) != (gctINT)pHT->itemCount)
        {
            ERR_REPORT(VSC_ERR_INVALID_DATA, "TypeHashTable damaged: items doesn't match itemCount");
            abort();
        }
        pHT = Source->constTable.pHashTable;
        if (pHT && vscHTBL_CountItems(pHT) != (gctINT)pHT->itemCount)
        {
            ERR_REPORT(VSC_ERR_INVALID_DATA, "ConstHashTable damaged: items doesn't match itemCount");
            abort();
        }
        pHT = Source->symTable.pHashTable;
        if (pHT && vscHTBL_CountItems(pHT) != (gctINT)pHT->itemCount)
        {
            ERR_REPORT(VSC_ERR_INVALID_DATA, "SymbolHashTable damaged: items doesn't match itemCount");
            abort();
        }
    }
#endif
    ON_ERROR0(VIR_Shader_Construct0(gcvNULL, VIR_Shader_GetKind(Source), Shader, gcvFALSE));
    memPool         = &Shader->pmp.mmWrapper;
    context.memPool = memPool;

    Copy_Field(Shader, Source, clientApiVersion);
    Copy_Field(Shader, Source, _id);
    Copy_Field(Shader, Source, _constVectorId);
    Copy_Field(Shader, Source, _dummyUniformCount);
    Copy_Field(Shader, Source, _orgTempCount);
    Copy_Field(Shader, Source, _tempRegCount);
    Copy_Field(Shader, Source, _anonymousNameId);
    Copy_Field(Shader, Source, shLevel);
    Copy_Field(Shader, Source, shaderKind);
    Copy_Field(Shader, Source, flags);
    Copy_Field(Shader, Source, flagsExt1);
    Copy_Field(Shader, Source, compilerVersion[0]);
    Copy_Field(Shader, Source, compilerVersion[1]);
    Copy_Field(Shader, Source, constUniformBlockIndex);
    Copy_Field(Shader, Source, defaultUniformBlockIndex);
    Copy_Field(Shader, Source, maxKernelFunctionArgs);
    Copy_Field(Shader, Source, privateMemorySize);
    Copy_Field(Shader, Source, localMemorySize);
    Copy_Field(Shader, Source, debugInfo);
    Copy_Field(Shader, Source, optionsLen);
    Copy_Field(Shader, Source, buildOptions);
    Copy_Field(Shader, Source, fragColorUsage);

    Copy_Field(Shader, Source, constUBOSize);
    if (Source->constUBOSize)
    {
        Shader->constUBOData =
            (gctUINT32 *)vscMM_Realloc(memPool, Shader->constUBOData,
                                       Source->constUBOSize * 16);
        if (Shader->constUBOData == gcvNULL)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
        gcoOS_MemCopy(Shader->constUBOData, (gctCHAR *)Source->constUBOData,
                      Source->constUBOSize*16);
    }

    Copy_Field(Shader, Source, constantMemorySize);
    if (Source->constantMemorySize > 0)
    {
        Shader->constantMemoryBuffer =
            (gctCHAR *)vscMM_Realloc(memPool, Shader->constantMemoryBuffer,
                                     Source->constantMemorySize);
        if (Shader->constantMemoryBuffer == gcvNULL)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
        gcoOS_MemCopy(Shader->constantMemoryBuffer,
                      (gctCHAR *)Source->constantMemoryBuffer,
                      Source->constantMemorySize);
    }
    else
        Shader->constantMemoryBuffer = gcvNULL;

    Copy_Field(Shader, Source, uniformVectorCount);
    Copy_Field(Shader, Source, samplerIndex);
    Copy_Field(Shader, Source, baseSamplerId);
    Copy_Field(Shader, Source, samplerBaseOffset);

    /* save layout info */
    switch (Source->shaderKind)
    {
    case VIR_SHADER_COMPUTE:
        VIR_CopyBlock((gctCHAR *)&Shader->shaderLayout.compute,
                      (gctCHAR *)&Source->shaderLayout.compute,
                      sizeof(VIR_ComputeLayout));
        break;
    case VIR_SHADER_TESSELLATION_CONTROL:
        VIR_CopyBlock((gctCHAR *)&Shader->shaderLayout.tcs,
                      (gctCHAR *)&Source->shaderLayout.tcs,
                      sizeof(VIR_TCSLayout));
        break;
    case VIR_SHADER_TESSELLATION_EVALUATION:
        VIR_CopyBlock((gctCHAR *)&Shader->shaderLayout.tes,
                      (gctCHAR *)&Source->shaderLayout.tes,
                      sizeof(VIR_TESLayout));
        break;
    case VIR_SHADER_GEOMETRY:
        VIR_CopyBlock((gctCHAR *)&Shader->shaderLayout.geo,
                      (gctCHAR *)&Source->shaderLayout.geo,
                      sizeof(VIR_GEOLayout));
        break;
    default:
        break;
    }

    /* Source code string */
    Copy_Field(Shader, Source, sourceLength);
    if (Source->sourceLength)
    {
        Shader->source = (gctCHAR *)vscMM_Alloc(memPool, Source->sourceLength * sizeof(gctCHAR));
        if (Shader->source == gcvNULL)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
        gcoOS_MemCopy(Shader->source, Source->source, Source->sourceLength);
    }

    Copy_Field(Shader, Source, replaceIndex);
    gcoOS_MemCopy(Shader->memoryAccessFlag,
                 (gctCHAR *)Source->memoryAccessFlag,
                 sizeof(Shader->memoryAccessFlag));
    gcoOS_MemCopy(Shader->flowControlFlag,
                 (gctCHAR *)Source->flowControlFlag,
                 sizeof(Shader->flowControlFlag));
    gcoOS_MemCopy(Shader->texldFlag,
                 (gctCHAR *)Source->texldFlag,
                 sizeof(Shader->texldFlag));
    Copy_Field(Shader, Source, vsPositionZDependsOnW);
    Copy_Field(Shader, Source, psHasDiscard);
    Copy_Field(Shader, Source, useEarlyFragTest);
    Copy_Field(Shader, Source, hasDsx);
    Copy_Field(Shader, Source, hasDsy);
    Copy_Field(Shader, Source, useLastFragData);
    Copy_Field(Shader, Source, __IsDual16Shader);
    Copy_Field(Shader, Source, __IsMasterDual16Shader);
    Copy_Field(Shader, Source, packUnifiedSampler);
    Copy_Field(Shader, Source, fullUnifiedUniforms);
    Copy_Field(Shader, Source, needToAdjustSamplerPhysical);
    Copy_Field(Shader, Source, _enableDefaultUBO);

    errCode = VIR_CopyIdList(&context, &Shader->attributes, &Source->attributes);
    ON_ERROR(errCode, "Fail to copy attributes id list.");

    errCode = VIR_CopyIdList(&context, &Shader->outputs, &Source->outputs);
    ON_ERROR(errCode, "Fail to copy outputs id list.");

    errCode = VIR_CopyIdList(&context, &Shader->outputVregs, &Source->outputVregs);
    ON_ERROR(errCode, "Fail to copy outputVregs id list.");

    errCode = VIR_CopyIdList(&context, &Shader->perpatchInput, &Source->perpatchInput);
    ON_ERROR(errCode, "Fail to copy perpatchInput id list.");

    errCode = VIR_CopyIdList(&context, &Shader->perpatchOutput, &Source->perpatchOutput);
    ON_ERROR(errCode, "Fail to copy perpatchOutput id list.");

    errCode = VIR_CopyIdList(&context, &Shader->perpatchOutputVregs, &Source->perpatchOutputVregs);
    ON_ERROR(errCode, "Fail to copy perpatchOutputVregs id list.");

    errCode = VIR_CopyIdList(&context, &Shader->buffers, &Source->buffers);
    ON_ERROR(errCode, "Fail to copy buffers id list.");

    errCode = VIR_CopyIdList(&context, &Shader->uniforms, &Source->uniforms);
    ON_ERROR(errCode, "Fail to copy uniforms id list.");

    errCode = VIR_CopyIdList(&context, &Shader->variables, &Source->variables);
    ON_ERROR(errCode, "Fail to copy variables id list.");

    errCode = VIR_CopyIdList(&context, &Shader->sharedVariables, &Source->sharedVariables);
    ON_ERROR(errCode, "Fail to copy shared variables id list.");

    errCode = VIR_CopyIdList(&context, &Shader->uniformBlocks, &Source->uniformBlocks);
    ON_ERROR(errCode, "Fail to copy uniformBlocks id list.");

    errCode = VIR_CopyIdList(&context, &Shader->storageBlocks, &Source->storageBlocks);
    ON_ERROR(errCode, "Fail to copy storageBlocks id list.");

    errCode = VIR_CopyIdList(&context, &Shader->ioBlocks, &Source->ioBlocks);
    ON_ERROR(errCode, "Fail to copy ioBlocks id list.");

    errCode = VIR_CopyIdList(&context, &Shader->moduleProcesses, &Source->moduleProcesses);
    ON_ERROR(errCode, "Fail to copy module process id list.");

    errCode = VIR_CopyStringTable(&context, &Shader->stringTable, &Source->stringTable);
    ON_ERROR(errCode, "Fail to copy string table.");

    errCode = VIR_CopyTypeTable(&context, &Shader->typeTable, &Source->typeTable);
    ON_ERROR(errCode, "Fail to copy type table.");

    errCode = VIR_CopyConstTable(&context, &Shader->constTable, &Source->constTable);
    ON_ERROR(errCode, "Fail to copy const table.");

    errCode = VIR_CopySymTable(&context, &Shader->symTable, &Source->symTable);
    ON_ERROR(errCode, "Fail to copy sym table.");

    /* Transform feedback varyings */
    errCode = VIR_CopyTransformFeedback(&context, &Shader->transformFeedback, &Source->transformFeedback);
    ON_ERROR(errCode, "Fail to copy transformFeedback.");

    /* LTC info */
    Copy_Field(Shader, Source, ltcUniformCount);
    if (Source->ltcUniformCount)
    {
        Shader->ltcCodeUniformIndex =
            (gctINT *)vscMM_Alloc(memPool, Source->ltcUniformCount * sizeof(gctINT));
        if (Shader->constantMemoryBuffer == gcvNULL)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
        gcoOS_MemCopy(Shader->ltcCodeUniformIndex,
                      (gctCHAR *)Source->ltcCodeUniformIndex,
                      Source->ltcUniformCount * sizeof(gctINT));
        for (i = 0; i < Source->ltcInstructionCount; i++)
        {
            Copy_Field(Shader, Source, ltcCodeUniformIndex[i]);
        }
        Shader->ltcExpressions =
            (VIR_Instruction *)vscMM_Alloc(memPool, Source->ltcInstructionCount * sizeof(VIR_Instruction));
        if (Shader->constantMemoryBuffer == gcvNULL)
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
        Copy_Field(Shader, Source, ltcInstructionCount);
        for (i = 0; i < Source->ltcInstructionCount; i++)
        {
            errCode = VIR_CopyInst(&context, &Shader->ltcExpressions[i], &Source->ltcExpressions[i]);
            ON_ERROR(errCode, "Fail to copy ltcExpressions[i].", i);
        }
    }

    /* copy function list */
    VIR_CopyFunctionList(&context, &Shader->functions, &Source->functions);

    if (Source->currentFunction)
    {
        Shader->currentFunction = VIR_Symbol_GetFunction(VIR_Shader_GetSymFromId(Shader,
                                         VIR_Function_GetSymId(Source->currentFunction)));
    }
    else
    {
        Shader->currentFunction = gcvNULL;
    }

    if (Source->mainFunction)
    {
        Shader->mainFunction = VIR_Symbol_GetFunction(VIR_Shader_GetSymFromId(Shader,
                                         VIR_Function_GetSymId(Source->mainFunction)));
    }
    else
    {
        Shader->mainFunction = gcvNULL;
    }

    if (Source->initFunction)
    {
        Shader->initFunction = VIR_Symbol_GetFunction(VIR_Shader_GetSymFromId(Shader,
                                         VIR_Function_GetSymId(Source->initFunction)));
    }
    else
    {
        Shader->initFunction = gcvNULL;
    }

    if (Source->currentKernelFunction)
    {
        Shader->currentKernelFunction = VIR_Symbol_GetFunction(VIR_Shader_GetSymFromId(Shader,
                                            VIR_Function_GetSymId(Source->currentKernelFunction)));
    }
    else
    {
        Shader->currentKernelFunction = gcvNULL;
    }
    Copy_Field(Shader, Source, kernelNameId);

    /* more fixes: some information are not available when doing the copy
     * now it is the time to do them */

    ON_ERROR0(VIR_Copy_FixUBOs(&context, &Shader->uniformBlocks));

    Copy_Field(Shader, Source, RAEnabled);
    Copy_Field(Shader, Source, hwRegAllocated);
    Copy_Field(Shader, Source, hwRegWatermark);
    Copy_Field(Shader, Source, constRegAllocated);
    Copy_Field(Shader, Source, remapRegStart);
    Copy_Field(Shader, Source, remapChannelStart);
    Copy_Field(Shader, Source, sampleMaskIdRegStart);
    Copy_Field(Shader, Source, sampleMaskIdChannelStart);
    Copy_Field(Shader, Source, hasRegisterSpill);
    Copy_Field(Shader, Source, llSlotForSpillVidmem);
    Copy_Field(Shader, Source, hasCRegSpill);
    Copy_Field(Shader, Source, useHwManagedLS);

    gcoOS_MemCopy(Shader->psInputPosCompValid,
                (gctCHAR *)Source->psInputPosCompValid,
                sizeof(Shader->psInputPosCompValid));

    gcoOS_MemCopy(Shader->psInputPCCompValid,
                (gctCHAR *)Source->psInputPCCompValid,
                sizeof(Shader->psInputPCCompValid));

    Copy_Field(Shader, Source, inLinkedShaderStage);
    Copy_Field(Shader, Source, outLinkedShaderStage);

    /* Now we can update the hostFunction since we have all information we need. */
    VIR_IO_UpdateHostFunction(Shader, &context.localSymbolList);

    QUEUE_FINALIZE(&context.localSymbolList);

OnError:
    return errCode;
}

