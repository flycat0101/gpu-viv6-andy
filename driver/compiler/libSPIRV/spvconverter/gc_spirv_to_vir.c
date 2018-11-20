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


#include "gc_spirv_cvter.h"
#include "gc_spirv.h"

#define SPV_NEW_FUNCPARAM       1

#define _GC_OBJ_ZONE    gcvZONE_COMPILER

#define SPV_HAS_RESULT (spv->resultId != 0)

#define SPV_NEXT_WORD (spv->numOperands--, spv->src[spv->word++])

#define SPV_NEXT_WORD_WO_OPERAND (spv->src[spv->word++])

#define SPV_NEXT_WORD_TO_STRING (spv->numOperands--, (gctCHAR*)(gcmINT2PTR(&spv->src[spv->word])))

#define SPV_HAS_MORE_WORD (spv->word < spv->nextInst)

#define SPV_NEXT_INST (spv->word = spv->nextInst)

#define SPV_OPCODE_2_VIR_OPCODE(opCode) (InstructionDesc[opCode].virOpCode)

#define SPV_OPCODE_2_VIR_EXTERN_OPCODE(opCode) (InstructionDesc[opCode].externOpcode)

/* IN memPool, IN ptr, IN ptrtype, INOUT size, IN testsize, IN pagesize */
#define SPV_CHECK_DYNAMIC_SIZE(memPool, ptr, ptrtype, size, testsize, pagesize) \
    if ((size) <= 0) \
    { \
        gctUINT pages = 1 + (testsize) / (pagesize); \
        (size) = (pages * (pagesize)); \
        spvAllocate((memPool), (size) * gcmSIZEOF(ptrtype), (gctPOINTER *)(&(ptr))); \
        gcoOS_ZeroMemory((ptr), (size) * gcmSIZEOF(ptrtype)); \
    } \
    else if ((testsize) >= (size)) \
    { \
        gctUINT pages = 1 + (((testsize) - (size))) / (pagesize); \
        ptrtype* oldPtr = (ptr); \
        (size) += (pages * (pagesize)); \
        spvAllocate((memPool), (size) * gcmSIZEOF(ptrtype), (gctPOINTER *)(&(ptr))); \
        gcoOS_ZeroMemory((ptr), (size) * gcmSIZEOF(ptrtype)); \
        gcoOS_MemCopy((ptr), oldPtr, ((size) - pages * (pagesize)) * gcmSIZEOF(ptrtype)); \
        spvFree(gcvNULL, oldPtr); \
    }

#define SPV_CHECK_FUNC_CALLER(spv, id) \
    { \
        SPV_CHECK_DYNAMIC_SIZE( spv->spvMemPool, \
                                spv->idDescriptor[id].u.func.caller, \
                                SpvFunctionCallerDescriptor, \
                                spv->idDescriptor[id].u.func.callerSize, \
                                spv->idDescriptor[id].u.func.callerNum + 1, \
                                SPV_GENERAL_PAGESIZE); \
    }

#define SPV_CHECK_EXEMODE(spv, id) \
    { \
        SPV_CHECK_DYNAMIC_SIZE( spv->spvMemPool, \
                                spv->exeModeDescriptor, \
                                SpvExeModeDescriptor, \
                                spv->exeModeSize, \
                                id, \
                                SPV_GENERAL_PAGESIZE); \
    }

#define SPV_SET_VALID_VALUE(Val, OrigVal, InvalidVal)           (((Val) != (InvalidVal)) ? (Val) : (OrigVal))

#define SPV_ID_INTERFACE_FLAG(id) (spv->idDescriptor[id].interfaceFlag)

#define SPV_ID_TYPE_INT_WIDTH(id) (spv->idDescriptor[id].u.type.u.integer.width)
#define SPV_ID_TYPE_INT_SIGN(id) (spv->idDescriptor[id].u.type.u.integer.sign)
#define SPV_ID_TYPE_FLOAT_WIDTH(id) (spv->idDescriptor[id].u.type.u.floatf.width)
#define SPV_ID_TYPE_VEC_COMP_TYPE(id) (spv->idDescriptor[id].u.type.u.vector.compType)
#define SPV_ID_TYPE_VEC_COMP_NUM(id) (spv->idDescriptor[id].u.type.u.vector.num)
#define SPV_ID_TYPE_MAT_COL_TYPE(id) (spv->idDescriptor[id].u.type.u.matrix.colType)
#define SPV_ID_TYPE_MAT_COL_COUNT(id) (spv->idDescriptor[id].u.type.u.matrix.colCount)
#define SPV_ID_TYPE_POINTER_STORAGE_CLASS(id) (spv->idDescriptor[id].u.type.u.pointer.storageClass)
#define SPV_ID_TYPE_POINTER_OBJECT_SPV_TYPE(id) (spv->idDescriptor[id].u.type.u.pointer.objType)
#define SPV_ID_TYPE_FUNC_RET_SPV_TYPE(id) (spv->idDescriptor[id].u.type.u.func.returnType)
#define SPV_ID_TYPE_FUNC_ARG_SPV_TYPE(id) (spv->idDescriptor[id].u.type.u.func.argType)
#define SPV_ID_TYPE_FUNC_ARG_NUM(id) (spv->idDescriptor[id].u.type.u.func.argNum)
#define SPV_ID_TYPE_VIR_TYPE_ID(id) (spv->idDescriptor[id].virTypeId)
#define SPV_ID_TYPE_VIR_TYPE(id) (VIR_Shader_GetTypeFromId(virShader, SPV_ID_TYPE_VIR_TYPE_ID(id)))
#define SPV_ID_TYPE_STRUCT_MEMBER(id, mem) (spv->idDescriptor[id].u.type.u.st.member[mem])
#define SPV_ID_TYPE_IMAGE_SAMPLED_TYPE(id) (spv->idDescriptor[id].u.type.u.image.sampledType)
#define SPV_ID_TYPE_IMAGE_DIM(id) (spv->idDescriptor[id].u.type.u.image.dimension)
#define SPV_ID_TYPE_IMAGE_DEPTH(id) (spv->idDescriptor[id].u.type.u.image.depth)
#define SPV_ID_TYPE_IMAGE_ARRAY(id) (spv->idDescriptor[id].u.type.u.image.arrayed)
#define SPV_ID_TYPE_IMAGE_MSAA(id) (spv->idDescriptor[id].u.type.u.image.msaa)
#define SPV_ID_TYPE_IMAGE_SAMPLED(id) (spv->idDescriptor[id].u.type.u.image.sampled)
#define SPV_ID_TYPE_IMAGE_FORMAT(id) (spv->idDescriptor[id].u.type.u.image.format)
#define SPV_ID_TYPE_IMAGE_SAMPLER_IMAGE_TYPE(id) (spv->idDescriptor[id].u.type.u.image.sampledImageType)
#define SPV_ID_TYPE_IMAGE_ACCESS_QULIFIER(id) (spv->idDescriptor[id].u.type.u.image.qualifier)
#define SPV_ID_TYPE_ARRAY_BASE_TYPE_ID(id) (spv->idDescriptor[id].u.type.u.array.baseTypeId)
#define SPV_ID_TYPE_ARRAY_LENGTH(id) (spv->idDescriptor[id].u.type.u.array.length)
#define SPV_ID_TYPE_ARRAY_SAMPLER_IMAGE_TYPE(id) (spv->idDescriptor[id].u.type.u.array.sampledImageType)

#define SPV_ID_TYPE_IS_FLOAT(id) (spv->idDescriptor[id].u.type.typeFlags.isFloat)
#define SPV_ID_TYPE_IS_INTEGER(id) (spv->idDescriptor[id].u.type.typeFlags.isInteger)
#define SPV_ID_TYPE_IS_SIGNEDINTEGER(id) (spv->idDescriptor[id].u.type.typeFlags.isSignedInteger)
#define SPV_ID_TYPE_IS_UNSIGNEDINTEGER(id) (spv->idDescriptor[id].u.type.typeFlags.isUnsignedInteger)
#define SPV_ID_TYPE_IS_BOOLEAN(id) (spv->idDescriptor[id].u.type.typeFlags.isBoolean)
#define SPV_ID_TYPE_IS_SCALAR(id) (spv->idDescriptor[id].u.type.typeFlags.isScalar)
#define SPV_ID_TYPE_IS_VECTOR(id) (spv->idDescriptor[id].u.type.typeFlags.isVector)
#define SPV_ID_TYPE_IS_MATRIX(id) (spv->idDescriptor[id].u.type.typeFlags.isMatrix)
#define SPV_ID_TYPE_IS_SAMPLER(id) (spv->idDescriptor[id].u.type.typeFlags.isSampler)
#define SPV_ID_TYPE_IS_IMAGE(id) (spv->idDescriptor[id].u.type.typeFlags.isImage)
#define SPV_ID_TYPE_IS_VOID(id) (spv->idDescriptor[id].u.type.typeFlags.isVoid)
#define SPV_ID_TYPE_IS_POINTER(id) (spv->idDescriptor[id].u.type.typeFlags.isPointer)
#define SPV_ID_TYPE_IS_ARRAY(id) (spv->idDescriptor[id].u.type.typeFlags.isArray)
#define SPV_ID_TYPE_IS_STRUCT(id) (spv->idDescriptor[id].u.type.typeFlags.isStruct)
#define SPV_ID_TYPE_HAS_UNSIZEDARRAY(id) (spv->idDescriptor[id].u.type.typeFlags.hasUnSizedArray)
#define SPV_ID_TYPE_IS_FUNCTION(id) (spv->idDescriptor[id].u.type.typeFlags.isFunction)
#define SPV_ID_TYPE_IS_META(id) (spv->idDescriptor[id].u.type.typeFlags.isMeta)
#define SPV_ID_TYPE_IS_BLOCK(id) (spv->idDescriptor[id].u.type.typeFlags.isBlock)

#define SPV_ID_VIR_SYM_ID(id) (spv->idDescriptor[id].u.sym.descriptorHeader.virSymId)
#define SPV_ID_SYM_SPV_TYPE(id) (spv->idDescriptor[id].u.sym.spvTypeId)
#define SPV_ID_SYM_SRC_SPV_TYPE(id) (spv->idDescriptor[id].u.sym.srcSpvTypeId)
#define SPV_ID_SYM_BLOCK_OFFSET_TYPE(id) (spv->idDescriptor[id].u.sym.offsetInfo.virAcOffsetInfo.blockIndexType)
#define SPV_ID_SYM_BLOCK_OFFSET_VALUE(id) (spv->idDescriptor[id].u.sym.offsetInfo.virAcOffsetInfo.blockIndex)
#define SPV_ID_SYM_OFFSET_TYPE(id) (spv->idDescriptor[id].u.sym.offsetInfo.virAcOffsetInfo.baseOffsetType)
#define SPV_ID_SYM_OFFSET_VALUE(id) (spv->idDescriptor[id].u.sym.offsetInfo.virAcOffsetInfo.baseOffset)
#define SPV_ID_SYM_OFFSET_ACCESSVECCOMPBYVARIABLE(id) (spv->idDescriptor[id].u.sym.offsetInfo.virAcOffsetInfo.accessVecCompByVariable)
#define SPV_ID_SYM_OFFSET_VECTYPE(id) (spv->idDescriptor[id].u.sym.offsetInfo.virAcOffsetInfo.accessVecType)
#define SPV_ID_SYM_VECTOR_OFFSET_TYPE(id) (spv->idDescriptor[id].u.sym.offsetInfo.virAcOffsetInfo.vectorIndexType)
#define SPV_ID_SYM_VECTOR_OFFSET_VALUE(id) (spv->idDescriptor[id].u.sym.offsetInfo.virAcOffsetInfo.vectorIndex)
#define SPV_ID_SYM_NO_NEED_WSHIFT(id) (spv->idDescriptor[id].u.sym.offsetInfo.virAcOffsetInfo.noNeedWShift)
#define SPV_ID_SYM_MAPPING_ARRAY_SYM(id) (spv->idDescriptor[id].u.sym.offsetInfo.virAcOffsetInfo.arraySym)
#define SPV_ID_SYM_ARRAY_STRIDE(id) (spv->idDescriptor[id].u.sym.offsetInfo.virAcOffsetInfo.arrayStride)
#define SPV_ID_SYM_MATRIX_STRIDE(id) (spv->idDescriptor[id].u.sym.offsetInfo.virAcOffsetInfo.matrixStride)
#define SPV_ID_SYM_LAYOUT_QUAL(id) (spv->idDescriptor[id].u.sym.offsetInfo.virAcOffsetInfo.layoutQual)
#define SPV_ID_SYM_IS_ROW_MAJOR_MATRIX_COLUMN_INDEX(id) (spv->idDescriptor[id].u.sym.offsetInfo.virAcOffsetInfo.isRowMajorMatrixColumnIndexing)
#define SPV_ID_SYM_UBOARRAY_BASE_SYM(id) (spv->idDescriptor[id].u.sym.offsetInfo.uboArrayBaseVirSymId)
#define SPV_ID_SYM_UBOARRAY(id, index) (spv->idDescriptor[id].u.sym.offsetInfo.uboArrayVirSymId[index])
#define SPV_ID_SYM_AC_DYNAMIC_INDEXING(id) (spv->idDescriptor[id].u.sym.offsetInfo.acDynamicIndexing)
#define SPV_ID_SYM_IS_FUNC_PARAM(id) (spv->idDescriptor[id].u.sym.isFuncParam)
#define SPV_ID_SYM_VIR_FUNC(id) (spv->idDescriptor[id].u.sym.virFunc)
#define SPV_ID_SYM_IS_WORKGROUP(id) (spv->idDescriptor[id].u.sym.isWorkGroup)
#define SPV_ID_SYM_USED_STORE_AS_DEST(id) (spv->idDescriptor[id].u.sym.usedStoreAsDest)
#define SPV_ID_SYM_USED_LOAD_AS_DEST(id) (spv->idDescriptor[id].u.sym.usedLoadAsDest)
#define SPV_ID_SYM_SAMPLEDIMAGE_IMAGE(id) (spv->idDescriptor[id].u.sym.image)
#define SPV_ID_SYM_SAMPLEDIMAGE_SAMPLER(id) (spv->idDescriptor[id].u.sym.sampler)
#define SPV_ID_SYM_SAMPLED_IMAGE(id) (spv->idDescriptor[id].u.sym.isSampledImage)
#define SPV_ID_SYM_PARAM_TO_FUNC(id) (spv->idDescriptor[id].u.sym.paramToFunc)
#define SPV_ID_SYM_PER_PATCH(id) (spv->idDescriptor[id].u.sym.isPerPatch)
#define SPV_ID_SYM_PER_VERTEX(id) (spv->idDescriptor[id].u.sym.isPerVertex)
#define SPV_ID_SYM_ATTACHMENT_FLAG(id) (spv->idDescriptor[id].u.sym.attachmentFlag)
#define SPV_ID_SYM_STORAGE_CLASS(id) (spv->idDescriptor[id].u.sym.storageClass)

#define SPV_ID_VIR_TYPE_ID(id) (spv->idDescriptor[id].virTypeId)
#define SPV_ID_VIR_NAME_ID(id) (spv->idDescriptor[id].virNameId)
#define SPV_ID_INITIALIZED(id) (spv->idDescriptor[id].initialized)
#define SPV_ID_VIR_STD_SYM(id) ((VIR_Symbol *)VIR_Shader_GetSymFromId(virShader, SPV_ID_VIR_SYM_ID(id)))
#define SPV_ID_VIR_CURRENT_FUNC_SYM(id) ((VIR_Symbol *)VIR_Function_GetSymFromId(spv->virFunction, SPV_ID_VIR_SYM_ID(id)))
#define SPV_ID_VIR_FUNC_SYM(id) ((VIR_Symbol *)VIR_Function_GetSymFromId(SPV_ID_SYM_VIR_FUNC(id), SPV_ID_VIR_SYM_ID(id)))
#define SPV_ID_VIR_SYM(id) (SPV_ID_SYM_IS_FUNC_PARAM(id) ? SPV_ID_VIR_FUNC_SYM(id) : SPV_ID_VIR_STD_SYM(id))
#define SPV_ID_VIR_TYPE(id) (VIR_Shader_GetTypeFromId(virShader,SPV_ID_VIR_TYPE_ID(id)))
#define SPV_ID_CST_SPV_TYPE(id) (spv->idDescriptor[id].u.cst.spvTypeId)
#define SPV_ID_VIR_CONST_ID(id) (spv->idDescriptor[id].u.cst.virConstId)
#define SPV_ID_VIR_CONST(id) (spv->idDescriptor[id].u.cst.virConst)
#define SPV_ID_CST_VEC_SPVID(id, index) (spv->idDescriptor[id].u.cst.vecSpvId[index])
#define SPV_ID_TYPE(id) (spv->idDescriptor[id].idType)
#define SPV_ID_FIELD_HAS_NAME(id, mem) (spv->idDescriptor[id].u.type.u.st.hasName[mem])
#define SPV_ID_FIELD_VIR_NAME_ID(id, mem) (spv->idDescriptor[id].u.type.u.st.field[mem])
#define SPV_ID_COND(id) (spv->idDescriptor[id].u.cond.virCond)
#define SPV_ID_COND_OP(id, index) (spv->idDescriptor[id].u.cond.op[index])
#define SPV_ID_FUNC_CONTROL(id) (spv->idDescriptor[id].u.func.funcControl)
#define SPV_ID_FUNC_TYPE_ID(id) (spv->idDescriptor[id].u.func.typeId)
#define SPV_ID_FUNC_LABEL(id) (spv->idDescriptor[id].u.func.label)
#define SPV_ID_FUNC_VIR_LABEL(id) (spv->idDescriptor[id].u.func.virLabel)
#define SPV_ID_FUNC_VIR_FUNCION(id) (spv->idDescriptor[id].u.func.virFunction)
#define SPV_ID_FUNC_ARG_NUM(id) (spv->idDescriptor[id].u.func.argNum)
#define SPV_ID_FUNC_SPV_ARG(id) (spv->idDescriptor[id].u.func.spvArg)
#define SPV_ID_FUNC_CALLER_NUM(id) (spv->idDescriptor[id].u.func.callerNum)
#define SPV_ID_FUNC_VIR_CALLER_INST(id, index) (spv->idDescriptor[id].u.func.caller[index].virCallerInst)
#define SPV_ID_FUNC_PARAM_INST(id, index) (spv->idDescriptor[id].u.func.caller[index].paramInst)
#define SPV_ID_FUNC_CALLER_SPV_ARG(id, index) (spv->idDescriptor[id].u.func.caller[index].spvCallerArg)
#define SPV_ID_FUNC_VIR_CALLER_ARG_NUM(id, index) (spv->idDescriptor[id].u.func.caller[index].callerArgNum)
#define SPV_ID_FUNC_VIR_CALLER_VIR_FUNC(id, index) (spv->idDescriptor[id].u.func.caller[index].virFunction)
#define SPV_ID_FUNC_CALL_SPV_RET_ID(id, index) (spv->idDescriptor[id].u.func.caller[index].spvRet)
#define SPV_ID_FUNC_RETURN_SPV_TYPE(id) (SPV_ID_TYPE_FUNC_RET_SPV_TYPE(SPV_ID_FUNC_TYPE_ID(id)))
#define SPV_ID_FUNC_VIR_RET_SYM(id) (spv->idDescriptor[id].u.func.virRetSymbol)
#define SPV_ID_FUNC_ARG_STORAGE(id) (spv->idDescriptor[id].u.func.argStorage)

#define SPV_CACHED_INST(index) (spv->cachedInst[index])
#define SPV_CACHED_INST_COUNT() (spv->cachedInstCount)

#define SPV_CHECK_CACHED_INST(spv) \
        SPV_CHECK_DYNAMIC_SIZE(spv->spvMemPool, spv->cachedInst, SpvInternalInstruction, spv->cachedInstSize, spv->cachedInstCount, SPV_MAX_OPERAND_NUM);

#define SPV_SET_CACHE_INST() { spv->isCacheInst = gcvTRUE; }
#define SPV_SET_UN_CACHE_INST() { spv->isCacheInst = gcvFALSE; }

#define SPV_ID_CLONE(from, to) (spv->idDescriptor[to] = spv->idDescriptor[from])

#define SPV_CONST_SCALAR_INT(id) (VIR_Shader_GetConstFromId(virShader,spv->idDescriptor[id].u.cst.virConstId)->value.scalarVal.iValue)
#define SPV_CONST_SCALAR_UINT(id) (VIR_Shader_GetConstFromId(virShader,spv->idDescriptor[id].u.cst.virConstId)->value.scalarVal.uValue)
#define SPV_CONST_SCALAR_FLOAT(id) (VIR_Shader_GetConstFromId(virShader,spv->idDescriptor[id].u.cst.virConstId)->value.scalarVal.fValue)

#define SPV_POINTER_VAR_OBJ_SPV_TYPE(ptr) (spv->idDescriptor[spv->idDescriptor[ptr].u.sym.spvTypeId].u.type.u.pointer.objType)
#define SPV_POINTER_VAR_OBJ_SPV_NAME(ptr) (ptr)

#define SPV_POINTER_VAR_OBJ_VIR_TYPE_ID(ptr) (spv->idDescriptor[ptr].virTypeId)
#define SPV_POINTER_VAR_OBJ_VIR_NAME_ID(ptr) (spv->idDescriptor[ptr].virNameId)

#define SPV_VIR_OP_FORMAT(opCode) (InstructionDesc[opCode].virOpFormat)

#define SPV_WORKGROUP_INFO() (spv->workgroupInfo)
#define SPV_HAS_WORKGROUP() (spv->hasWorkGroup)
#define SPV_ADD_WORKGROUP_MEMBER(spv, id, offset) \
    { \
        gctUINT32 index = SPV_WORKGROUP_INFO()->memberCount; \
        SPV_WORKGROUP_INFO()->sboMembers[index] = id; \
        SPV_WORKGROUP_INFO()->sboMemberOffset[index] = offset; \
        SPV_WORKGROUP_INFO()->memberCount++; \
    }

#define SPV_INVALID_ID_DESC     0
#define SPV_DEFAULT_LOCATION    0xFFFFFFFF

#define SPV_INSTPARAM_SET_RESUTLTYPE(inst, r, t) \
    { \
        inst.resultPresent = r; \
        inst.typePresent = t; \
    }

#define SPV_INSTPARAM_SET_OPCLASS(inst, c) \
    { \
        inst.opClass = c; \
    }

#define SPV_INSTPARAM_SET_DESC(inst, c, d) \
    { \
        inst.operandClass[inst.oprandSize] = c; \
        inst.operandDesc[inst.oprandSize] = d; \
        inst.oprandSize++; \
    }

#define SPV_INSTPARAM_SET_VIR(inst, opCode, opFormat, mod) \
    {\
        inst.virOpCode = opCode; \
        inst.virOpFormat = opFormat; \
        inst.modifier = mod; \
    }

#define SPV_INSTPARAM_SET_FUNC(inst, func) \
    {\
        inst.opFunc = func; \
    }

#define SPV_CHECK_IDDESCRIPTOR(spv, id) \
        if (id >= spv->idDescSize) \
        { \
            gctUINT i = 0; \
            gctUINT oldSize = spv->idDescSize; \
            SPV_CHECK_DYNAMIC_SIZE(spv->spvMemPool, spv->idDescriptor, SpvCovIDDescriptor, spv->idDescSize, id, SPV_DESCRIPTOR_PAGESIZE); \
            for (i = oldSize; i < spv->idDescSize; i++) \
            { \
                spv->idDescriptor[i].idType = SPV_ID_TYPE_UNKNOW;\
                spv->idDescriptor[i].virNameId = VIR_INVALID_ID; \
                spv->idDescriptor[i].virTypeId = VIR_TYPE_UNKNOWN; \
                gcoOS_MemFill(&spv->idDescriptor[i].interfaceFlag, 0xFF, gcmSIZEOF(SpvInterfaceFlag)); \
            } \
        }

#define SPV_SET_IDDESCRIPTOR_NAME(spv, id, sid) \
    { \
            spv->idDescriptor[id].virNameId = sid; \
    }

#define SPV_SET_IDDESCRIPTOR_FIELD_NAME(spv, id, fieldId, sid) \
    { \
            spv->idDescriptor[id].u.type.u.st.field[fieldId] = sid; \
            spv->idDescriptor[id].u.type.u.st.hasName[fieldId] = gcvTRUE; \
    }

#define SPV_SET_IDDESCRIPTOR_SYM(spv, id, sid) \
    { \
        spv->idDescriptor[id].u.sym.descriptorHeader.virSymId = sid; \
    }

#define SPV_SET_IDDESCRIPTOR_UBOARRAY_SYM(spv, id, index, sid) \
    { \
        spv->idDescriptor[id].u.sym.offsetInfo.uboArrayVirSymId[index] = sid; \
    }


#define SPV_SET_IDDESCRIPTOR_TYPE(spv, id, sid) \
    { \
        spv->idDescriptor[id].virTypeId = sid; \
    }

#define SPV_SET_IDDESCRIPTOR_SPV_TYPE(spv, id, tid) \
    { \
        spv->idDescriptor[id].u.sym.spvTypeId = tid; \
    }

#define SPV_SET_IDDESCRIPTOR_SPV_OFFSET(spv, id, Info) \
    { \
        spv->idDescriptor[id].u.sym.offsetInfo.virAcOffsetInfo = (Info);\
    }

#define SPV_RESET_IDDESCRIPTOR_SPV_OFFSET(spv, id) \
    { \
        gcoOS_ZeroMemory(&(spv->idDescriptor[id].u.sym.offsetInfo.virAcOffsetInfo), gcmSIZEOF(VIR_AC_OFFSET_INFO)); \
    }

#define SPV_SET_IDDESCRIPTOR_SPV_COND_OP(spv, id ,index, operand) \
    { \
        spv->idDescriptor[id].u.cond.op[index] = operand; \
    }

#define SPV_SET_IDDESCRIPTOR_SPV_COND(spv, id, op) \
    { \
        spv->idDescriptor[id].u.cond.virCond = op; \
    }

#define SPV_IDDESCRIPTOR_VAR(spv, id) (spv->idDescriptor[id].u.sym.descriptorHeader.unHandledOperands)

#define SPV_CHECK_VAR_OPERAND(spv, id) \
        { \
        SPV_CHECK_DYNAMIC_SIZE(spv->spvMemPool, \
                               SPV_IDDESCRIPTOR_VAR(spv, id).infoList, \
                               SpvUnhandleInfo, \
                               SPV_IDDESCRIPTOR_VAR(spv, id).listSize, \
                               SPV_IDDESCRIPTOR_VAR(spv, id).listCount + 1, \
                               SPV_GENERAL_PAGESIZE); \
        }

#define SPV_SET_UNHANDLE_VAR_OPERAND(spv, id, Inst, Operand) \
        { \
        gctUINT unhandled = SPV_IDDESCRIPTOR_VAR(spv, id).listCount; \
        SPV_CHECK_VAR_OPERAND(spv, id); \
        SPV_IDDESCRIPTOR_VAR(spv, id).infoList[unhandled].inst = Inst; \
        SPV_IDDESCRIPTOR_VAR(spv, id).infoList[unhandled].operand = Operand; \
        SPV_IDDESCRIPTOR_VAR(spv, id).listCount++; \
        }

#define SPV_IDDESCRIPTOR_LABEL(spv, id) spv->idDescriptor[id].u.label

#define SPV_CHECK_LABEL_VIRINST(spv, id) \
    { \
        SPV_CHECK_DYNAMIC_SIZE(spv->spvMemPool, \
                               SPV_IDDESCRIPTOR_LABEL(spv, id).virInst, \
                               VIR_Instruction *, \
                               SPV_IDDESCRIPTOR_LABEL(spv, id).instSize, \
                               SPV_IDDESCRIPTOR_LABEL(spv, id).destCount + 1, \
                               SPV_GENERAL_PAGESIZE); \
    }

#define SPV_CHECK_LABEL_VIRDEST(spv, id) \
    { \
        SPV_CHECK_DYNAMIC_SIZE(spv->spvMemPool, \
                               SPV_IDDESCRIPTOR_LABEL(spv, id).virDest, \
                               VIR_Operand *, \
                               SPV_IDDESCRIPTOR_LABEL(spv, id).destSize, \
                               SPV_IDDESCRIPTOR_LABEL(spv, id).destCount + 1, \
                               SPV_GENERAL_PAGESIZE); \
    }

#define SPV_SET_UNHANDLE_LABEL_VIRDEST(spv, id, inst, dest) \
    { \
        gctUINT unhandled = SPV_IDDESCRIPTOR_LABEL(spv, id).destCount; \
        SPV_CHECK_LABEL_VIRINST(spv, id); \
        SPV_CHECK_LABEL_VIRDEST(spv, id); \
        SPV_IDDESCRIPTOR_LABEL(spv, id).virInst[unhandled] = inst; \
        SPV_IDDESCRIPTOR_LABEL(spv, id).virDest[unhandled] = dest; \
        SPV_IDDESCRIPTOR_LABEL(spv, id).destCount++; \
        }

#define SPV_CHECK_LABEL_PHI(spv, id) \
        { \
        SPV_CHECK_DYNAMIC_SIZE(spv->spvMemPool, \
                               SPV_IDDESCRIPTOR_LABEL(spv, id).phiOperands, \
                               VIR_PhiOperand*, \
                               SPV_IDDESCRIPTOR_LABEL(spv, id).phiSize, \
                               SPV_IDDESCRIPTOR_LABEL(spv, id).phiCount + 1, \
                               SPV_GENERAL_PAGESIZE); \
        }

#define SPV_SET_UNHANDLE_LABEL_PHI(spv, id, dest) \
        { \
        gctUINT unhandled = SPV_IDDESCRIPTOR_LABEL(spv, id).phiCount; \
        SPV_CHECK_LABEL_PHI(spv, id); \
        SPV_IDDESCRIPTOR_LABEL(spv, id).phiOperands[unhandled] = dest; \
        SPV_IDDESCRIPTOR_LABEL(spv, id).phiCount++; \
        }

#define SPV_GET_DECORATOR(dec, targetId, MemberIndex) \
    { \
        while (dec) \
        { \
            if ((dec)->target == (targetId) && (dec)->memberIndex == (MemberIndex)) \
            { \
                break; \
            } \
            dec = (dec)->next; \
        } \
    }

#define SPV_IS_SPECCONST_OP(opcode) (((opcode) == SpvOpSpecConstant) || \
                                     ((opcode) == SpvOpSpecConstantComposite) || \
                                     ((opcode) == SpvOpSpecConstantFalse) || \
                                     ((opcode) == SpvOpSpecConstantTrue) || \
                                     ((opcode) == SpvOpSpecConstantOp))

#define SPV_IS_EMPTY_STRING(str) ((str == gcvNULL) || (gcoOS_MemCmp(str, "", 1) == gcvSTATUS_OK))

#define VIR_OPINFO(OPCODE, OPNDNUM, FLAGS, WRITE2DEST, LEVEL)    {VIR_OP_##OPCODE, OPNDNUM, WRITE2DEST, LEVEL, FLAGS}

const VIR_Opcode_Info VIR_OpcodeInfo[] =
{
#include "vir/ir/gc_vsc_vir_opcode.def.h"
};
#undef VIR_OPINFO

typedef enum{
    SpvTypeUnknow,
    SpvTypeVoid,
    SpvTypeFloat16,
    SpvTypeFloat32,
    SpvTypeFloat64,
    SpvTypeInt16,
    SpvTypeInt32,
    SpvTypeInt64,
    SpvTypeUINT16,
    SpvTypeUint32,
}SpvType;

typedef struct
{
    gctBOOL builtIn;
    gctCHAR * name;
    gctBOOL noUse;
    VIR_SymFlag virSymFlag;

    VIR_Precision virPrecision;
    gctINT location;
    gctINT binding;
    gctINT descriptorSet;
    gctINT inputAttachmentIndex;
    gctINT arrayStride;
    gctINT matrixStride;
    gctINT offset;
    gctINT alignment;
    gctBOOL compilerGen;
    gctBOOL perPatch;

    SpvStorageClass spvStorage;

    /* Parameter for VIR_Shader_AddSymbol */
    VIR_SymbolKind virSymbolKind;
    VIR_StorageClass virStorageClass;
    VIR_LayoutQual virLayoutQual;
    union
    {
        VIR_UniformKind uniformKind;
    } u1;
}SpvVIRSymbolInternal;

#define SYMSPV_Initialize(SymSpv) \
    do \
    { \
        gcoOS_ZeroMemory(SymSpv, gcmSIZEOF(SpvVIRSymbolInternal));      \
        (SymSpv)->virPrecision = VIR_PRECISION_HIGH;                    \
        (SymSpv)->noUse = gcvFALSE;                                     \
        (SymSpv)->location = SPV_DEFAULT_LOCATION;                      \
        (SymSpv)->binding = -1;                                         \
        (SymSpv)->descriptorSet = -1;                                   \
        (SymSpv)->arrayStride = -1;                                     \
        (SymSpv)->matrixStride = -1;                                    \
        (SymSpv)->offset = -1;                                          \
        (SymSpv)->alignment = -1;                                       \
        (SymSpv)->virSymbolKind = VIR_SYM_UNKNOWN;                      \
        (SymSpv)->virStorageClass = VIR_STORAGE_UNKNOWN;                \
        (SymSpv)->virLayoutQual = VIR_LAYQUAL_NONE;                     \
        (SymSpv)->builtIn = gcvFALSE;                                   \
    } \
    while (gcvFALSE)

typedef enum
{
    SpvOffsetType_None = 0,
    SpvOffsetType_Normal,
    SpvOffsetType_UBO,
    SpvOffsetType_UBO_Array,
    SpvOffsetType_PER_VERTEX,
}SpvOffsetType;

static VSC_ErrCode __SpvAddLabel(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvAddType(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvAddConstant(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvAddUndef(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvAddVariable(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvAddFunction(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvAddFuncCall(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvAddFunctionEnd(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvEmitAccessChain(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvEmitVertexPrimitive(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvEmitVectorShuffle(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvEmitPhi(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvEmitCompositeExtract(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvEmitCompositeInsert(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvEmitVectorExtractDynamic(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvEmitVectorInsertDynamic(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvEmitCompositeConstruct(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvEmitIntrisicCall(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvEmitImageSample(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvEmitSampledImage(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvEmitSpecConstantOp(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvAddOpImage(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvEmitInstructions(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvEmitCopyMemory(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvEmitSwitch(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvEmitLoad(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvEmitStore(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvEmitArrayLength(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvEmitAtomic(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvAddDecorator(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvAddName(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvAddBranch(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvAddBranchConditional(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvAddReturn(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvAddExtInst(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvAddFunctionParameter(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvAddReturnValue(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvNop(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvAddIntrisicFunction(gcSPV spv, VIR_Shader * virShader);
static VSC_ErrCode __SpvFoldingSpecConstantOp(gcSPV spv, VIR_Shader * virShader);

static VSC_ErrCode __SpvDecodeImageOperand(
    gcSPV spv,
    VIR_Shader * virShader,
    VIR_Operand * operand,
    SpvImageOperandsMask operandMask,
    SpvId * operands,
    gctUINT operandCount);

#define SPV_MISSING_OP \
    { \
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassMissing, \
        gcvNULL, \
        0, \
        {OperandNone}, \
        {gcvNULL}, \
        {gcvNULL}, \
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE \
    } \


static struct SpvInstructionParameters InstructionDesc[SPV_MAX_OPCODE_NUM] =
{
#include "gc_spirv_instparam_def.h"
};

VIR_TypeId __SpvIntToVirType(gctUINT width, gctBOOL isSigned)
{
    VIR_TypeId typeId = VIR_TYPE_UNKNOWN;

    switch (width)
    {
    case 8:     typeId = isSigned ? VIR_TYPE_INT8 : VIR_TYPE_UINT8; break;
    case 16:    typeId = isSigned ? VIR_TYPE_INT16 : VIR_TYPE_UINT16; break;
    case 32:    typeId = isSigned ? VIR_TYPE_INT32 : VIR_TYPE_UINT32; break;
    case 64:    typeId = isSigned ? VIR_TYPE_INT64 : VIR_TYPE_UINT64; break;
    default: break;
    }

    return typeId;
}

static gctCHAR * SpvBuiltInName [] =
{
    "gl_Position", /* SpvBuiltInPosition = 0,*/
    "gl_PointSize", /* SpvBuildInPointSize = 1, */
    "gl_ClipVertex", /* SpvBuildInClipVertex = 2, */
    "gl_ClipDistance", /* SpvBuiltInClipDistance = 3, */
    "gl_CullDistance", /* SpvBuiltInCullDistance = 4, */
    "gl_VertexID", /* SpvBuiltInVertexId = 5, */
    "gl_InstanceID", /* SpvBuiltInInstanceId = 6, */
    "gl_PrimitiveID", /* SpvBuiltInPrimitiveId = 7, */
    "gl_InvocationID", /* SpvBuiltInInvocationId = 8, */
    "gl_Layer", /* SpvBuiltInLayer = 9, */
    "gl_ViewportIndex", /* SpvBuiltInViewportIndex = 10, */
    "gl_TessLevelOuter", /* SpvBuiltInTessLevelOuter = 11, */
    "gl_TessLevelInner", /* SpvBuiltInTessLevelInner = 12, */
    "gl_TessCoord", /* SpvBuiltInTessCoord = 13, */
    "gl_PatchVerticesIn", /* SpvBuiltInPatchVertices = 14, */
    "gl_Position", /* SpvBuiltInFragCoord = 15, */
    "gl_PointCoord", /* SpvBuiltInPointCoord = 16, */
    "gl_FrontFacing", /* SpvBuiltInFrontFacing = 17, */
    "gl_SampleID", /* SpvBuiltInSampleId = 18, */
    "gl_SamplePosition", /* SpvBuiltInSamplePosition = 19, */
    "gl_SampleMask", /* SpvBuiltInSampleMask = 20, */
    "gl_FragColor", /* SpvBuiltInFragColor = 21, */
    "gl_FragDepth", /* SpvBuiltInFragDepth = 22, */
    "gl_HelperInvocation", /* SpvBuiltInHelperInvocation = 23, */
    "gl_NumWorkGroups", /* SpvBuiltInNumWorkgroups = 24, */
    "gl_WorkGroupSize", /* SpvBuiltInWorkgroupSize = 25, */
    "gl_WorkGroupID", /* SpvBuiltInWorkgroupId = 26, */
    "gl_LocalInvocationID", /* SpvBuiltInLocalInvocationId = 27, */
    "gl_GlobalInvocationID", /* SpvBuiltInGlobalInvocationId = 28, */
    "gl_LocalInvocationIndex", /* SpvBuiltInLocalInvocationIndex = 29, */
    "gl_WorkDim", /* SpvBuiltInWorkDim = 30, */
    "gl_GlobalSize", /* SpvBuiltInGlobalSize = 31, */
    "", /* SpvBuiltInEnqueuedWorkgroupSize = 32, */
    "", /* SpvBuiltInGlobalOffset = 33, */
    "", /* SpvBuiltInGlobalLinearId = 34, */
    "", /* SpvBuiltInWorkgroupLinearId = 35, */
    "", /* SpvBuiltInSubgroupSize = 36, */
    "", /* SpvBuiltInSubgroupMaxSize = 37, */
    "", /* SpvBuiltInNumSubgroups = 38, */
    "", /* SpvBuiltInNumEnqueuedSubgroups = 39, */
    "", /* SpvBuiltInSubgroupId = 40, */
    "", /* SpvBuiltInSubgroupLocalInvocationId = 41, */
    "gl_VertexID", /* SpvBuiltInVertexIndex = 42, */
    "gl_InstanceIndex", /* SpvBuiltInInstanceIndex = 43, */
};

SpvOperandClass __SpvGetOperandClassFromOpCode(SpvOp opCode, gctUINT opndIndex)
{
    return InstructionDesc[opCode].operandClass[opndIndex];
}

gctUINT __SpvGetOperandNumFromOpCode(SpvOp opCode)
{
    return InstructionDesc[opCode].oprandSize;
}
gctBOOL __SpvOpCodeHasResult(SpvOp opCode)
{
    return InstructionDesc[opCode].resultPresent;
}

gctBOOL __SpvOpCodeHasType(SpvOp opCode)
{
    return InstructionDesc[opCode].typePresent;
}

VIR_Symbol * __SpvGetVirSymFromId(gcSPV spv, VIR_Shader * virShader, VIR_SymId symId)
{
    if (VIR_Id_isFunctionScope(symId))
    {
        return VIR_Function_GetSymFromId(spv->virFunction, symId);
    }
    else
    {
        return VIR_Shader_GetSymFromId(virShader, symId);
    }
}

static void __SpvCheckFlag(gcSPV spv, VIR_Shader * virShader, SpvId * spvIds, gctUINT idCount)
{
    gctUINT         i;
    SpvId           spvId;
    VIR_SymFlag     symFlag;
    VIR_Symbol     *sym;

    if (spvIds == gcvNULL || idCount < 1)
    {
        return;
    }

    for (i = 0; i < idCount; i++)
    {
        spvId = spvIds[i];
        if ((spvId >= spv->bound) ||
            (spvId == SPV_INVALID_ID) ||
            (SPV_ID_TYPE(spvId) != SPV_ID_TYPE_SYMBOL))
        {
            break;
        }

        sym = SPV_ID_VIR_SYM(spvId);

        symFlag = VIR_Symbol_GetFlag(sym);
        symFlag |= (VIR_SYMFLAG_ENABLED | VIR_SYMFLAG_STATICALLY_USED);
        VIR_Symbol_SetFlag(sym, symFlag);
    }
}

static gctUINT __GetMagicOffsetByTypeId(VIR_TypeId TypeId)
{
    gctUINT     magicOffset = 0;
    switch (TypeId)
    {
        case VIR_TYPE_FLOAT32:      magicOffset = 0;    break;
        case VIR_TYPE_INT32:        magicOffset = 7;    break;
        case VIR_TYPE_INT8:         magicOffset = 14;   break;
        case VIR_TYPE_INT16:        magicOffset = 21;   break;
        case VIR_TYPE_UINT32:       magicOffset = 28;   break;
        case VIR_TYPE_UINT8:        magicOffset = 35;   break;
        case VIR_TYPE_UINT16:       magicOffset = 42;   break;
        case VIR_TYPE_BOOLEAN:      magicOffset = 49;   break;
        default:
            gcmASSERT(gcvFALSE);
            break;
    }

    return magicOffset;
}

static void __SpvDecodeLiteralString(gcSPV spv, gctUINT * Length, gctCHAR * String, gctBOOL UpdateWord)
{
    gctUINT beginIndex = spv->word;
    gctUINT offset = 0;
    gctUINT length = 0;
    gctUINT source = spv->src[beginIndex];
    gctBOOL save = (String != gcvNULL);
    char c = source & 0xFF;

    while (c)
    {
        if (save)
        {
            String[length] = c;
        }
        length++;

        /* Get next word. */
        if ((length % 4) == 0)
        {
            offset++;
            source = spv->src[beginIndex + offset];
        }
        else
        {
            source = source >> 8;

        }
        c = source & 0xFF;
    }

    /* Calculate the offset. */
    offset = (length / 4) + 1;

    /* Save the '\0'. */
    if (save)
    {
        String[length] = '\0';
        gcmASSERT(strlen(String) == length);
    }
    length++;

    /* Save the result. */
    if (Length)
    {
        *Length = length;
    }

    if (UpdateWord)
    {
        spv->numOperands -= offset;
        spv->word += offset;
    }
}

static VIR_ConditionOp
__SpvOpCode2VIRCop(
    IN SpvOp opCode
    )
{
    VIR_ConditionOp virCop = VIR_COP_ALWAYS;

    switch (opCode)
    {
    case SpvOpIEqual:
    case SpvOpFOrdEqual:
        virCop = VIR_COP_EQUAL;
        break;

    case SpvOpFUnordEqual:
        virCop = VIR_COP_EQUAL_UQ;
        break;

    case SpvOpINotEqual:
    case SpvOpFOrdNotEqual:
        virCop = VIR_COP_NOT_EQUAL;
        break;

    case SpvOpFUnordNotEqual:
        virCop = VIR_COP_NOT_EQUAL_UQ;
        break;

    case SpvOpULessThan:
    case SpvOpSLessThan:
    case SpvOpFOrdLessThan:
        virCop = VIR_COP_LESS;
        break;

    case SpvOpFUnordLessThan:
        virCop = VIR_COP_LESS_UQ;
        break;

    case SpvOpUGreaterThan:
    case SpvOpSGreaterThan:
    case SpvOpFOrdGreaterThan:
        virCop = VIR_COP_GREATER;
        break;

    case SpvOpFUnordGreaterThan:
        virCop = VIR_COP_GREATER_UQ;
        break;

    case SpvOpULessThanEqual:
    case SpvOpSLessThanEqual:
    case SpvOpFOrdLessThanEqual:
        virCop = VIR_COP_LESS_OR_EQUAL;
        break;

    case SpvOpFUnordLessThanEqual:
        virCop = VIR_COP_LESS_OR_EQUAL_UQ;
        break;

    case SpvOpUGreaterThanEqual:
    case SpvOpSGreaterThanEqual:
    case SpvOpFOrdGreaterThanEqual:
        virCop = VIR_COP_GREATER_OR_EQUAL;
        break;

    case SpvOpFUnordGreaterThanEqual:
        virCop = VIR_COP_GREATER_OR_EQUAL_UQ;
        break;

    case SpvOpSelect:
        virCop = VIR_COP_NOT_ZERO;
        break;

    /* Logical condition. */
    case SpvOpLogicalEqual:
        virCop = VIR_COP_EQUAL;
        break;

    case SpvOpLogicalNotEqual:
        virCop = VIR_COP_NOT_EQUAL;
        break;

    case SpvOpLogicalNot:
        virCop = VIR_COP_NOT;
        break;

    case SpvOpLogicalOr:
        virCop = VIR_COP_OR;
        break;

    case SpvOpLogicalAnd:
        virCop = VIR_COP_AND;
        break;

    case SpvOpIsInf:
        virCop = VIR_COP_INFINITE;
        break;

    case SpvOpIsNan:
        virCop = VIR_COP_NAN;
        break;

    default:
        break;
    }

    return virCop;
}

static VIR_Swizzle
__SpvConstIndexToVIRSwizzle(
    IN gctUINT index
    )
{
    VIR_Swizzle result = VIR_SWIZZLE_XYZW;

    switch (index)
    {
    case 0:
        result = VIR_SWIZZLE_XXXX;
        break;

    case 1:
        result = VIR_SWIZZLE_YYYY;
        break;

    case 2:
        result = VIR_SWIZZLE_ZZZZ;
        break;

    case 3:
        result = VIR_SWIZZLE_WWWW;
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    return result;
}

static VIR_Enable virEnable [] =
{
    VIR_ENABLE_X,
    VIR_ENABLE_Y,
    VIR_ENABLE_Z,
    VIR_ENABLE_W
};

static VIR_Enable virEnableCompact [] =
{
    VIR_ENABLE_XYZW,
    VIR_ENABLE_X,
    VIR_ENABLE_XY,
    VIR_ENABLE_XYZ,
    VIR_ENABLE_XYZW,
};

static VIR_Swizzle virSwizzleCompact [] =
{
    VIR_SWIZZLE_XYZW,
    VIR_SWIZZLE_X,
    VIR_SWIZZLE_XYYY,
    VIR_SWIZZLE_XYZZ,
    VIR_SWIZZLE_XYZW,
};

static VIR_Enable
__SpvGenEnable(
    IN gcSPV spv,
    IN VIR_Type * DestType,
    IN SpvId ResultTypeId
    )
{
    VIR_Enable enable;

    if (DestType == gcvNULL)
    {
        enable = VIR_ENABLE_X;
        return enable;
    }

    if (VIR_Type_isScalar(DestType))
    {
        enable = VIR_ENABLE_X;
    }
    else if (VIR_Type_isVector(DestType))
    {
        enable = VIR_TypeId_Conv2Enable(VIR_Type_GetIndex(DestType));
    }
    else if (VIR_Type_isMatrix(DestType) || VIR_Type_isStruct(DestType))
    {
        enable = VIR_ENABLE_NONE;
    }
    else
    {
        enable = VIR_ENABLE_XYZW;
    }

    return enable;
}

static SpvId
__SpvGetResultTypeId (
    IN gcSPV spv,
    IN gctUINT id
    )
{
    SpvId spvTypeId = SPV_INVALID_ID;
    SpvIDType idType = SPV_ID_TYPE(id);

    switch (idType)
    {
    case SPV_ID_TYPE_SYMBOL:
        spvTypeId = SPV_ID_SYM_SPV_TYPE(id);
        break;

    case SPV_ID_TYPE_CONST:
        spvTypeId = SPV_ID_CST_SPV_TYPE(id);
        break;

    case SPV_ID_TYPE_FUNC_DEFINE:
        spvTypeId = SPV_ID_FUNC_TYPE_ID(id);
        break;

    case SPV_ID_TYPE_TYPE:
        if (SPV_ID_TYPE_IS_ARRAY(id))
        {
            spvTypeId = SPV_ID_TYPE_ARRAY_BASE_TYPE_ID(id);
        }
        else if (SPV_ID_TYPE_IS_POINTER(id))
        {
            spvTypeId = SPV_ID_TYPE_POINTER_OBJECT_SPV_TYPE(id);
        }
        break;

    default:
        break;
    }

    return spvTypeId;
}

static VIR_Swizzle
__ConvVectorIndexToSwizzle(
    IN  VIR_TypeId          typeId,
    IN  gctUINT             vectorIndex,
    IN  gctBOOL             needWShift
    )
{
    VIR_Swizzle             swizzle = VIR_SWIZZLE_XXXX;
    gctUINT                 compCount = VIR_GetTypeComponents(typeId);

    if (needWShift)
    {
        swizzle = __SpvConstIndexToVIRSwizzle(vectorIndex);
    }
    else
    {
        switch (vectorIndex)
        {
        case 0:
            switch (compCount)
            {
            case 1:
                swizzle = VIR_SWIZZLE_XXXX;
                break;
            case 2:
                swizzle = VIR_SWIZZLE_XYYY;
                break;
            case 3:
                swizzle = VIR_SWIZZLE_XYZZ;
                break;
            default:
                swizzle = VIR_SWIZZLE_XYZW;
                break;
            }
            break;

        case 1:
            switch (compCount)
            {
            case 1:
                swizzle = VIR_SWIZZLE_YYYY;
                break;
            case 2:
                swizzle = VIR_SWIZZLE_YZZZ;
                break;
            default:
                swizzle = VIR_SWIZZLE_YZWW;
                break;
            }
            break;

        case 2:
            switch (compCount)
            {
            case 1:
                swizzle = VIR_SWIZZLE_ZZZZ;
                break;
            default:
                swizzle = VIR_SWIZZLE_ZWWW;
                break;
            break;
            }

        case 3:
            swizzle = VIR_SWIZZLE_WWWW;
            break;

        default:
            gcmASSERT(gcvFALSE);
            break;
        }
    }

    return swizzle;
}


static VIR_Swizzle
__SpvID2Swizzle (
    IN gcSPV spv,
    IN gctUINT id
    )
{
    VIR_Swizzle virSwizzle = VIR_SWIZZLE_XYZW;
    SpvIDType idType = SPV_ID_TYPE(id);
    SpvId     sampler = 0;
    if (id >= spv->idDescSize)
    {
        return virSwizzle;
    }

    switch (idType)
    {
    case SPV_ID_TYPE_SYMBOL:        idType = SPV_ID_SYM_SPV_TYPE(id);
                                    sampler = SPV_ID_SYM_SAMPLEDIMAGE_SAMPLER(id);
                                    break;
    case SPV_ID_TYPE_CONST:         idType = SPV_ID_CST_SPV_TYPE(id); break;
    case SPV_ID_TYPE_FUNC_DEFINE:   idType = SPV_ID_FUNC_TYPE_ID(id); break;
    case SPV_ID_TYPE_TYPE:          idType = id; break;
    default: gcmASSERT(gcvFALSE); break;
    }

    if (SPV_ID_TYPE_IS_POINTER(idType))
    {
        if (SPV_ID_TYPE_IS_VECTOR(idType))
        {
            virSwizzle = virSwizzleCompact[SPV_ID_TYPE_VEC_COMP_NUM(SPV_ID_TYPE_POINTER_OBJECT_SPV_TYPE(idType))];
        }
        else if (sampler != 0)
        {
            /* the accessed symbol is sampler, use VIR_SWIZZLE_XYZW */
            virSwizzle = VIR_SWIZZLE_XYZW;
        }
        else
        {
            /* vec4 in_te_attr[] and get value of in_te_attr[0].z,
             * resultId.enable is set .z, set virSwizzle is of src0 .zzzz instead of .x
             *   ATTR_LD            hp global  #spv_id61.hp.z, hp  #spv_id22.hp.z,  uint 0,   uint 0
             */
            if (spv->resultId &&
                (SPV_ID_SYM_VECTOR_OFFSET_VALUE(spv->resultId) != VIR_INVALID_ID) &&
                (SPV_ID_SYM_VECTOR_OFFSET_TYPE(spv->resultId) == VIR_SYM_CONST))
            {
                virSwizzle = __ConvVectorIndexToSwizzle(SPV_ID_VIR_TYPE_ID(spv->resultId),
                                                        SPV_ID_SYM_VECTOR_OFFSET_VALUE(spv->resultId),
                                                        !SPV_ID_SYM_NO_NEED_WSHIFT(spv->resultId));
            }
            else
            {
                virSwizzle = VIR_SWIZZLE_XXXX;
            }
        }
    }
    else if (SPV_ID_TYPE_IS_VECTOR(idType))
    {
        virSwizzle = VIR_Swizzle_GenSwizzleByComponentCount(SPV_ID_TYPE_VEC_COMP_NUM(idType));
    }
    else if (SPV_ID_TYPE_IS_SCALAR(idType) || SPV_ID_TYPE_IS_BOOLEAN(idType))
    {
        virSwizzle = VIR_SWIZZLE_XXXX;
    }
    else
    {
        /* TO_DO, if not vector, like struct.xxx, the type should come from field type */
        virSwizzle = VIR_SWIZZLE_XYZW;
    }

    return virSwizzle;
}

static gceSTATUS __SpvDecodeString(
    IN gcSPV spv,
    OUT gctSTRING *str
    )
{
    gctUINT * stream = spv->src;
    gctSTRING wordString = gcvNULL;
    gctBOOL done = gcvFALSE;
    gctUINT i = 0;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER();

    gcmVERIFY_ARGUMENT(str != gcvNULL);

    *str = (gctSTRING)gcmINT2PTR(&stream[spv->word]);

    /* to jump to right word */
    do {
        gctUINT content = stream[spv->word];
        wordString = (gctSTRING)(gcmINT2PTR(&content));
        for (i = 0; i < 4; i++)
        {
            if (*wordString == 0)
            {
                done = gcvTRUE;
                break;
            }
            wordString++;
        }

        spv->word++;
    } while (!done);

    /* TO_DO, make random name */
    if (*str == gcvNULL)
    {
        gctUINT offset = 0;
        gctSTRING randName = gcvNULL;
        gcmONERROR(spvAllocate(spv->spvMemPool, 64, (gctPOINTER *)&randName));
        gcoOS_ZeroMemory(randName, 64);
        gcoOS_PrintStrSafe(randName, SPV_VIR_NAME_SIZE, &offset, "#sh_spv_unknow_id_%d", spv->unknowId++);
        *str = randName;
    }

OnError:
    gcmFOOTER();
    return status;
}

static void __SpvSetCapability(gcSPV spv, SpvCapability cap)
{
    switch (cap)
    {
    case SpvCapabilityMatrix:
        spv->capability.SpvCapabilityMatrix = 1;
        break;

    case SpvCapabilityShader:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        break;

    case SpvCapabilityGeometry:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityGeometry = 1;
        break;

    case SpvCapabilityTessellation:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityTessellation = 1;
        break;

    case SpvCapabilityAddresses:
        spv->capability.SpvCapabilityAddresses = 1;
        break;

    case SpvCapabilityLinkage:
        spv->capability.SpvCapabilityLinkage = 1;
        break;

    case SpvCapabilityKernel:
        spv->capability.SpvCapabilityKernel = 1;
        break;

    case SpvCapabilityVector16:
        spv->capability.SpvCapabilityKernel = 1;
        spv->capability.SpvCapabilityVector16 = 1;
        break;

    case SpvCapabilityFloat16Buffer:
        spv->capability.SpvCapabilityKernel = 1;
        spv->capability.SpvCapabilityFloat16Buffer = 1;
        break;

    case SpvCapabilityFloat16:
        spv->capability.SpvCapabilityFloat16 = 1;
        break;

    case SpvCapabilityFloat64:
        spv->capability.SpvCapabilityFloat64 = 1;
        break;

    case SpvCapabilityInt64:
        spv->capability.SpvCapabilityInt64 = 1;
        break;

    case SpvCapabilityInt64Atomics:
        spv->capability.SpvCapabilityInt64 = 1;
        spv->capability.SpvCapabilityInt64Atomics = 1;
        break;

    case SpvCapabilityImageBasic:
        spv->capability.SpvCapabilityKernel = 1;
        spv->capability.SpvCapabilityImageBasic = 1;
        break;

    case SpvCapabilityImageReadWrite:
        spv->capability.SpvCapabilityKernel = 1;
        spv->capability.SpvCapabilityImageBasic = 1;
        spv->capability.SpvCapabilityImageReadWrite = 1;
        break;

    case SpvCapabilityImageMipmap:
        spv->capability.SpvCapabilityKernel = 1;
        spv->capability.SpvCapabilityImageBasic = 1;
        spv->capability.SpvCapabilityImageMipmap = 1;
        break;

    case SpvCapabilityPipes:
        spv->capability.SpvCapabilityKernel = 1;
        spv->capability.SpvCapabilityPipes = 1;
        break;

    case SpvCapabilityGroups:
        spv->capability.SpvCapabilityGroups = 1;
        break;

    case SpvCapabilityDeviceEnqueue:
        spv->capability.SpvCapabilityKernel = 1;
        spv->capability.SpvCapabilityDeviceEnqueue = 1;
        break;

    case SpvCapabilityLiteralSampler:
        spv->capability.SpvCapabilityKernel = 1;
        spv->capability.SpvCapabilityLiteralSampler = 1;
        break;

    case SpvCapabilityAtomicStorage:
        spv->capability.SpvCapabilityKernel = 1;
        spv->capability.SpvCapabilityAtomicStorage = 1;
        break;

    case SpvCapabilityInt16:
        spv->capability.SpvCapabilityInt16 = 1;
        break;

    case SpvCapabilityTessellationPointSize:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityTessellation = 1;
        spv->capability.SpvCapabilityTessellationPointSize = 1;
        break;

    case SpvCapabilityGeometryPointSize:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityGeometry = 1;
        spv->capability.SpvCapabilityGeometryPointSize = 1;
        break;

    case SpvCapabilityImageGatherExtended:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilityImageGatherExtended = 1;
        break;

    case SpvCapabilityStorageImageMultisample:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilityStorageImageMultisample = 1;
        break;

    case SpvCapabilityUniformBufferArrayDynamicIndexing:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilityUniformBufferArrayDynamicIndexing = 1;
        break;

    case SpvCapabilitySampledImageArrayDynamicIndexing:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilitySampledImageArrayDynamicIndexing = 1;
        break;

    case SpvCapabilityStorageBufferArrayDynamicIndexing:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilityStorageBufferArrayDynamicIndexing = 1;
        break;

    case SpvCapabilityStorageImageArrayDynamicIndexing:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilityStorageImageArrayDynamicIndexing = 1;
        break;

    case SpvCapabilityClipDistance:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilityClipDistance = 1;
        break;

    case SpvCapabilityCullDistance:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilityCullDistance = 1;
        break;

    case SpvCapabilityImageCubeArray:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilitySampledCubeArray = 1;
        spv->capability.SpvCapabilityImageCubeArray = 1;
        break;

    case SpvCapabilitySampleRateShading:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilitySampleRateShading = 1;
        break;

    case SpvCapabilityImageRect:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilitySampledRect = 1;
        spv->capability.SpvCapabilityImageRect = 1;
        break;

    case SpvCapabilitySampledRect:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilitySampledRect = 1;
        break;

    case SpvCapabilityGenericPointer:
        spv->capability.SpvCapabilityAddresses = 1;
        spv->capability.SpvCapabilityGenericPointer = 1;
        break;

    case SpvCapabilityInt8:
        spv->capability.SpvCapabilityKernel = 1;
        spv->capability.SpvCapabilityInt8 = 1;
        break;

    case SpvCapabilityInputAttachment:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilityInputAttachment = 1;
        break;

    case SpvCapabilitySparseResidency:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilitySparseResidency = 1;
        break;

    case SpvCapabilityMinLod:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilityMinLod = 1;
        break;

    case SpvCapabilitySampled1D:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilitySampled1D = 1;
        break;

    case SpvCapabilityImage1D:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilitySampled1D = 1;
        spv->capability.SpvCapabilityImage1D = 1;
        break;

    case SpvCapabilitySampledCubeArray:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilitySampledCubeArray = 1;
        break;

    case SpvCapabilitySampledBuffer:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilitySampledBuffer = 1;
        break;

    case SpvCapabilityImageBuffer:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilitySampledBuffer = 1;
        spv->capability.SpvCapabilityImageBuffer = 1;
        break;

    case SpvCapabilityImageMSArray:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilityImageMSArray = 1;
        break;

    case SpvCapabilityStorageImageExtendedFormats:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilityStorageImageExtendedFormats = 1;
        break;

    case SpvCapabilityImageQuery:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilityImageQuery = 1;
        break;

    case SpvCapabilityDerivativeControl:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilityDerivativeControl = 1;
        break;

    case SpvCapabilityInterpolationFunction:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilityInterpolationFunction = 1;
        break;

    case SpvCapabilityTransformFeedback:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilityTransformFeedback = 1;
        break;

    case SpvCapabilityGeometryStreams:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityGeometry = 1;
        spv->capability.SpvCapabilityGeometryStreams = 1;
        break;

    case SpvCapabilityStorageImageReadWithoutFormat:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilityStorageImageReadWithoutFormat = 1;
        break;

    case SpvCapabilityStorageImageWriteWithoutFormat:
        spv->capability.SpvCapabilityMatrix = 1;
        spv->capability.SpvCapabilityShader = 1;
        spv->capability.SpvCapabilityStorageImageWriteWithoutFormat = 1;
        break;

    default:
        break;
    }
}

static void __SpvGenerateVIRName(gcSPV spv, gctUINT id)
{
    gctUINT offset = 0;

    gcoOS_PrintStrSafe(spv->virName, SPV_VIR_NAME_SIZE, &offset, "#sh_spv_id%d", id);
}

static void __SpvGenerateStructFiledName(gcSPV spv, gctSTRING structName, gctUINT id)
{
    gctUINT offset = 0;

    gcoOS_PrintStrSafe(spv->virName, SPV_VIR_NAME_SIZE, &offset, "%s_field_%d", structName, id);
}

static void __SpvGenerateCopyMemoryName(gcSPV spv, gctUINT id)
{
    gctUINT offset = 0;

    gcoOS_PrintStrSafe(spv->virName, SPV_VIR_NAME_SIZE, &offset, "#sh_copy_to_%d", id);
}

static void __SpvGenerateFuncName(gcSPV spv, gctUINT id)
{
    gctUINT offset = 0;

    gcoOS_PrintStrSafe(spv->virName, SPV_VIR_NAME_SIZE, &offset, "#sh_func_id%d", id);
}

/* Generate the function return variable name */
static void __SpvGenerateFuncReturnName(gcSPV spv, gctCHAR * funcName)
{
    gctUINT offset = 0;

    gcoOS_PrintStrSafe(spv->virName, SPV_VIR_NAME_SIZE, &offset,
        "#sh_%s_retValue", funcName);

    spv->nameId++;
}

static VIR_SymId __SpvGenerateVectorDynamicIndexSym(gcSPV spv, VIR_Shader *virShader, gctUINT id, VIR_TypeId TypeId, gctUINT ArraySize)
{
    gctUINT offset = 0;
    VIR_SymId symId;
    VIR_Symbol *sym = gcvNULL;
    VIR_NameId nameId;
    VIR_TypeId typeId;

    gcoOS_PrintStrSafe(spv->virName, SPV_VIR_NAME_SIZE, &offset, "#sh_spv_id%d_VectorDynamicIndex", id);

    VIR_Shader_AddString(virShader, spv->virName, &nameId);
    VIR_Shader_AddArrayType(virShader,
                            TypeId,
                            ArraySize,
                            0,
                            &typeId);
    VIR_Shader_AddSymbol(virShader,
                         VIR_SYM_VARIABLE,
                         nameId,
                         VIR_Shader_GetTypeFromId(virShader, typeId),
                         VIR_STORAGE_GLOBAL,
                         &symId);

    sym = VIR_Shader_GetSymFromId(virShader, symId);

    VIR_Symbol_SetFlag(sym,VIR_SYMFLAG_WITHOUT_REG);

    return symId;
}

static VSC_ErrCode __SpvReplaceBuiltInName(gcSPV spv, VIR_Shader * virShader, VIR_StorageClass virStorageClass, SpvId resultId, gctINT memberIndex)
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    SpvCovDecorator *dec = spv->decorationList;
    SpvBuiltIn builtIn;
    gctCHAR * name;
    VIR_NameId nameId = VIR_INVALID_ID;
    VIR_NameId orgNameId = spv->idDescriptor[resultId].virNameId;
    gctCHAR * orgName;

    if (memberIndex != -1 && SPV_ID_FIELD_HAS_NAME(resultId, memberIndex))
    {
        orgNameId = SPV_ID_FIELD_VIR_NAME_ID(resultId, memberIndex);
    }

    if (orgNameId == VIR_INVALID_ID)
    {
        orgName = "";
    }
    else
    {
        orgName = VIR_Shader_GetStringFromId(virShader, orgNameId);
    }

    /* Find the decoration by target and member index. */
    SPV_GET_DECORATOR(dec, resultId, memberIndex);

    if (dec == gcvNULL)
    {
        return virErrCode;
    }

    builtIn = dec->decorationData.builtIn;

    if ((builtIn < 0) || (builtIn >= gcmSIZEOF(SpvBuiltInName) / gcmSIZEOF(gctCHAR *)))
    {
        return virErrCode;
    }

    name = SpvBuiltInName[builtIn];

    if (spv->shaderStage == VSC_SHADER_STAGE_GS &&
        builtIn == SpvBuiltInPrimitiveId &&
        virStorageClass == VIR_STORAGE_INPUT)
    {
        name = "gl_PrimitiveIDIn";
    }

    if (!SPV_IS_EMPTY_STRING(name) && gcoOS_StrCmp(name, orgName) != gcvSTATUS_OK)
    {
        VIR_Shader_AddString(virShader, name, &nameId);
    }

    if (nameId != VIR_INVALID_ID)
    {
        if (memberIndex == -1)
        {
            SPV_SET_IDDESCRIPTOR_NAME(spv, resultId, nameId);
        }
        else
        {
            SPV_SET_IDDESCRIPTOR_FIELD_NAME(spv, resultId, memberIndex, nameId);
        }
    }

    return virErrCode;
}

static VSC_ErrCode __SpvFillVirSymWithSymSpv(gcSPV spv, VIR_Symbol * sym, VIR_Shader *virShader, SpvVIRSymbolInternal * symSpv)
{
    VIR_Uniform * virUniform;
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VIR_SymFlag symFlag = symSpv->virSymFlag;

    switch(symSpv->spvStorage)
    {
    case SpvStorageClassPushConstant:
        /* For a push-constant, if it is an array, we need to set the arraystride. */
        if (VIR_Type_isArray(VIR_Symbol_GetType(sym)))
        {
            VIR_Type        *pType = VIR_Symbol_GetType(sym);
            VIR_Type        *pBaseType = pType;
            VIR_SymIdList   *pFields = gcvNULL;
            VIR_Symbol      *pLastFieldSym = gcvNULL;
            VIR_Type        *pLastFieldType = gcvNULL;
            gctUINT         arrayLength = 1, offset = 0;
            gctINT          arrayStride = 0;

            while (VIR_Type_isArray(pBaseType))
            {
                pBaseType = VIR_Shader_GetTypeFromId(virShader, VIR_Type_GetBaseTypeId(pBaseType));
            }

            pFields = VIR_Type_GetFields(pBaseType);
            gcmASSERT(pFields);

            pLastFieldSym = VIR_Shader_GetSymFromId(virShader, pFields->ids[pFields->count - 1]);
            pLastFieldType = VIR_Symbol_GetType(pLastFieldSym);

            pBaseType = pLastFieldType;
            while (VIR_Type_isArray(pBaseType))
            {
                arrayLength *= VIR_Type_GetArrayLength(pBaseType);
                pBaseType = VIR_Shader_GetTypeFromId(virShader, VIR_Type_GetBaseTypeId(pBaseType));
            }

            /* Calculate the offset of the last field. */
            offset = VIR_FieldInfo_GetOffset(VIR_Symbol_GetFieldInfo(pLastFieldSym));

            /* Calculate the size of the last field. */
            VIR_Type_CalcByteOffset(virShader,
                                    pBaseType,
                                    VIR_Type_isArray(pLastFieldType),
                                    VIR_Symbol_GetLayoutQualifier(pLastFieldSym),
                                    offset,
                                    0,
                                    &arrayStride,
                                    gcvNULL,
                                    gcvNULL);

            VIR_Type_SetArrayStride(pType, offset + arrayStride * arrayLength);
        }
        /* According to spec, a push-constant uses a std430 layout. */
        VIR_Symbol_SetOneLayoutQualifier(sym, VIR_LAYQUAL_STD430);
    case SpvStorageClassUniformConstant:
    case SpvStorageClassUniform:
    case SpvStorageClassAtomicCounter:
    case SpvStorageClassImage:
        if (symSpv->spvStorage == SpvStorageClassPushConstant)
        {
            symSpv->u1.uniformKind = VIR_UNIFORM_PUSH_CONSTANT;
        }

        symFlag |= (VIR_SYMFLAG_ENABLED | VIR_SYMUNIFORMFLAG_USED_IN_SHADER);

        VIR_Symbol_SetAddrSpace(sym, VIR_AS_CONSTANT);
        VIR_Symbol_SetTyQualifier(sym, VIR_TYQUAL_CONST);
        /* GLSL uniform and OCL constant memory */
        /* TO_DO, more uniform flag in _ConvShaderUniformIdx2Vir */
        VIR_Symbol_SetPrecision(sym, symSpv->virPrecision);
        VIR_Symbol_SetUniformKind(sym, symSpv->u1.uniformKind);

        if (VIR_Symbol_GetKind(sym) == VIR_SYM_UNIFORM)
        {
            virUniform = sym->u2.uniform;
            virUniform->sym = VIR_Symbol_GetIndex(sym);
            /* TO_DO, what are these? */
            virUniform->gcslIndex = 0;
            virUniform->lastIndexingIndex = -1;
            virUniform->glUniformIndex = 0;
        }
        else if (VIR_Symbol_GetKind(sym) == VIR_SYM_UBO)
        {
            /* Set ubo information */
        }
        /*
           ............... a lot other thing, mess!
        */
        VIR_Symbol_SetLocation(sym,  symSpv->location);
        VIR_Symbol_SetBinding(sym,  symSpv->binding);
        VIR_Symbol_SetDescriptorSet(sym, symSpv->descriptorSet);
        VIR_Symbol_SetInputAttIndex(sym, symSpv->inputAttachmentIndex);
        VIR_Symbol_SetFlag(sym, symFlag);
        if (VIR_Symbol_isUniform(sym) ||
            VIR_Symbol_isSampler(sym) ||
            VIR_Symbol_isTexure(sym)  ||
            VIR_Symbol_isImage(sym))
        {
            VIR_Symbol_ClrFlag(sym, VIR_SYMFLAG_WITHOUT_REG);
        }
        break;

    case SpvStorageClassInput:
        VIR_Symbol_SetPrecision(sym, symSpv->virPrecision);
        VIR_Symbol_SetTyQualifier(sym, VIR_TYQUAL_CONST);
        VIR_Symbol_SetLayoutQualifier(sym, VIR_LAYQUAL_NONE);
        VIR_Symbol_SetLocation(sym,  symSpv->location);
        VIR_Symbol_SetFlag(sym, symFlag);
        break;

    case SpvStorageClassOutput:
        VIR_Symbol_SetTyQualifier(sym, VIR_TYQUAL_NONE);
        VIR_Symbol_SetPrecision(sym, symSpv->virPrecision);
        VIR_Symbol_SetLayoutQualifier(sym, VIR_LAYQUAL_NONE);
        VIR_Symbol_SetLocation(sym,  symSpv->location);
        VIR_Symbol_SetFlag(sym, symFlag);
        break;

    case SpvStorageClassFunction:
        VIR_Symbol_SetPrecision(sym, symSpv->virPrecision);
        VIR_Symbol_SetTyQualifier(sym, VIR_TYQUAL_NONE);
        VIR_Symbol_SetLayoutQualifier(sym, VIR_LAYQUAL_NONE);
        break;

    case SpvStorageClassPrivate:
        VIR_Symbol_SetPrecision(sym, symSpv->virPrecision);
        VIR_Symbol_SetTyQualifier(sym, VIR_TYQUAL_NONE);
        VIR_Symbol_SetLayoutQualifier(sym, VIR_LAYQUAL_NONE);
        break;

    case SpvStorageClassWorkgroup:
        VIR_Symbol_SetPrecision(sym, symSpv->virPrecision);
        VIR_Symbol_SetTyQualifier(sym, VIR_TYQUAL_NONE);
        VIR_Symbol_SetLayoutQualifier(sym, VIR_LAYQUAL_SHARED);
        break;

    default:
        gcmASSERT(0);
        break;
    }

    if (symSpv->builtIn)
    {
        VIR_Symbol_ClrFlag(sym, VIR_SYMFLAG_SKIP_NAME_CHECK);
    }

    return virErrCode;
}

static void
__SetAccessChainOffsetToOperand(
    IN  gcSPV               spv,
    IN  SpvId               ResultId,
    IN  VIR_Operand        *Operand,
    IN  SpvOffsetType       spvOffsetType
    )
{
    VIR_SymbolKind          baseOffsetType = SPV_ID_SYM_OFFSET_TYPE(ResultId);
    gctUINT                 baseOffset = SPV_ID_SYM_OFFSET_VALUE(ResultId);
    VIR_SymbolKind          vecOffsetType = SPV_ID_SYM_VECTOR_OFFSET_TYPE(ResultId);
    gctUINT                 vecOffset = SPV_ID_SYM_VECTOR_OFFSET_VALUE(ResultId);
    VIR_SymbolKind          blockOffsetType = SPV_ID_SYM_BLOCK_OFFSET_TYPE(ResultId);
    gctUINT                 blockOffset = SPV_ID_SYM_BLOCK_OFFSET_VALUE(ResultId);
    VIR_Swizzle             swizzle;

    if (SPV_ID_TYPE(ResultId) != SPV_ID_TYPE_SYMBOL)
    {
        return;
    }

    VIR_Operand_SetMatrixStride(Operand, SPV_ID_SYM_MATRIX_STRIDE(ResultId));
    VIR_Operand_SetLayoutQual(Operand, SPV_ID_SYM_LAYOUT_QUAL(ResultId));

    if (spvOffsetType == SpvOffsetType_None)
    {
        return;
    }
    else if (spvOffsetType == SpvOffsetType_UBO_Array)
    {
        /* need reset block offset and return if we meet block array, in case
           it always set block offset */
        switch (blockOffsetType)
        {
            /* Not offset. */
        case VIR_SYM_UNKNOWN:
            break;

            /* Symbol id used in indexing. */
        case VIR_SYM_VARIABLE:
            VIR_Operand_SetIsConstIndexing(Operand, gcvFALSE);
            VIR_Operand_SetRelIndex(Operand, blockOffset);
            VIR_Operand_SetRelAddrMode(Operand, VIR_INDEXED_X);
            break;

            /* Constant indexed. */
        case VIR_SYM_CONST:
            VIR_Operand_SetIsConstIndexing(Operand, gcvTRUE);
            VIR_Operand_SetRelIndex(Operand, blockOffset);
            break;

        default:
            gcmASSERT(gcvFALSE);
            break;
        }
    }
    else
    {
        if (vecOffset != VIR_INVALID_ID)
        {
            switch (vecOffsetType)
            {
            case VIR_SYM_UNKNOWN:
                break;

            case VIR_SYM_VARIABLE:
                VIR_Operand_SetRelIndexing(Operand, vecOffset);
                VIR_Operand_SetRelAddrMode(Operand, VIR_INDEXED_X);
                break;

            case VIR_SYM_CONST:
                swizzle = __ConvVectorIndexToSwizzle(SPV_ID_VIR_TYPE_ID(ResultId), vecOffset, !SPV_ID_SYM_NO_NEED_WSHIFT(ResultId));

                if (VIR_Operand_isLvalue(Operand))
                {
                    VIR_Operand_SetEnable(Operand, VIR_Swizzle_2_Enable(swizzle));
                }
                else
                {
                    VIR_Operand_SetSwizzle(Operand, swizzle);
                }
                break;

            default:
                gcmASSERT(gcvFALSE);
                break;
            }
        }

        switch (baseOffsetType)
        {
            /* Not offset. */
        case VIR_SYM_UNKNOWN:
            break;

            /* Symbol id used in indexing. */
        case VIR_SYM_VARIABLE:
            VIR_Operand_SetIsConstIndexing(Operand, gcvFALSE);
            VIR_Operand_SetRelIndex(Operand, baseOffset);
            VIR_Operand_SetRelAddrMode(Operand, VIR_INDEXED_X);
            break;

            /* Constant indexed. */
        case VIR_SYM_CONST:
            VIR_Operand_SetIsConstIndexing(Operand, gcvTRUE);
            VIR_Operand_SetRelIndex(Operand, baseOffset);
            break;

        default:
            gcmASSERT(gcvFALSE);
            break;
        }
    }

    return;
}

static VIR_IB_FLAG __SpvGetIBFlag(
    IN gcSPV spv,
    IN VIR_Shader *virShader,
    IN SpvId targetId,
    IN SpvId targetTypeId,
    IN SpvVIRSymbolInternal *spvSym)
{
    VIR_IB_FLAG virIBFlag = VIR_IB_NONE;
    VIR_NameId nameId = SPV_ID_VIR_NAME_ID(targetId);
    gctSTRING name = gcvNULL;

    if (nameId != VIR_INVALID_ID)
    {
        name = VIR_Shader_GetStringFromId(virShader, nameId);

        if (!SPV_IS_EMPTY_STRING(name))
        {
            virIBFlag |= VIR_IB_WITH_INSTANCE_NAME;
        }
    }

    if (SPV_ID_TYPE_HAS_UNSIZEDARRAY(targetTypeId))
    {
        virIBFlag |= VIR_IB_UNSIZED;
    }

    return virIBFlag;
}

static gctBOOL __SpvGetBuiltInBlockName(
    VIR_Shader * virShader,
    gctSTRING TypeName,
    gctSTRING InstanceName,
    VIR_NameId *NameId
    )
{
    gctBOOL result = gcvFALSE;
    VIR_NameId nameId = VIR_INVALID_ID;
    VIR_ShaderKind shaderKind = VIR_Shader_GetKind(virShader);

    if (gcmIS_SUCCESS(gcoOS_StrNCmp(TypeName, "gl_PerVertex", 12)))
    {
        switch (shaderKind)
        {
        case VIR_SHADER_TESSELLATION_CONTROL:
            if (gcmIS_SUCCESS(gcoOS_StrNCmp(InstanceName, "gl_in", 5)))
            {
                result = gcvTRUE;
                VIR_Shader_AddString(virShader, InstanceName, &nameId);
            }
            else if (gcmIS_SUCCESS(gcoOS_StrNCmp(InstanceName, "gl_out", 6)))
            {
                result = gcvTRUE;
                VIR_Shader_AddString(virShader, TypeName, &nameId);
            }
            break;

        case VIR_SHADER_TESSELLATION_EVALUATION:
        case VIR_SHADER_GEOMETRY:
            if (gcmIS_SUCCESS(gcoOS_StrNCmp(InstanceName, "gl_in", 5)))
            {
                result = gcvTRUE;
                VIR_Shader_AddString(virShader, InstanceName, &nameId);
            }
            else if (SPV_IS_EMPTY_STRING(InstanceName))
            {
                result = gcvTRUE;
                VIR_Shader_AddString(virShader, TypeName, &nameId);
            }
            break;

        default:
            break;
        }
    }

    if (result && NameId)
    {
        *NameId = nameId;
    }

    return result;
}

static VSC_ErrCode __SpvAddInterfaceBlockSymbol(
    gcSPV spv,
    VIR_Shader * virShader,
    SpvId id,
    SpvId type,
    gctUINT arraySize,
    SpvVIRSymbolInternal *spvSym
    )
{
    VIR_SymId symId = VIR_INVALID_ID;
    VIR_Symbol * sym;
    VIR_Type * virType;
    VIR_Type * baseVirType;
    VIR_SymIdList * fields;
    VIR_Id fieldSymId;
    VIR_Symbol * fieldSym;
    VIR_FieldInfo * fieldInfo;
    SpvId spvBaseTypeID;
    VIR_NameId typeNameId = 0;
    SpvId baseType = type;
    gctBOOL isArray = arraySize != SPV_INVALID_LABEL;
    gctBOOL setSkipNameCheckFlag = gcvFALSE;
    gctSTRING instanceName = gcvNULL;
    gctSTRING typeName = gcvNULL;
    gctUINT blockNameLength = 0, instanceNameLength = 0;

    if (SPV_ID_TYPE_IS_POINTER(baseType))
    {
        baseType = SPV_ID_TYPE_POINTER_OBJECT_SPV_TYPE(baseType);
    }

    if (SPV_ID_TYPE_IS_POINTER(baseType))
    {
        virType = SPV_ID_TYPE_VIR_TYPE(SPV_ID_TYPE_POINTER_OBJECT_SPV_TYPE(baseType));
    }
    else
    {
        virType = SPV_ID_TYPE_VIR_TYPE(baseType);
    }

    /*---------------I: Create interface block symbol---------------*/
    spvBaseTypeID = baseType;
    if (isArray)
    {
        spvBaseTypeID = SPV_ID_TYPE_ARRAY_BASE_TYPE_ID(baseType);
    }

    typeNameId = SPV_ID_VIR_NAME_ID(spvBaseTypeID);
    typeName = VIR_Shader_GetStringFromId(virShader, typeNameId);

    /*
    ** If this type is from spriv assembly directly, then more than one IBs may use it as name,
    ** we need to use the resultId as the IB name.
    ** TODO: we should use a flag to check if a type name is created by ourself.
    */
    if (gcmIS_SUCCESS(gcoOS_StrNCmp(typeName, "#sh_", 4)))
    {
        __SpvGenerateVIRName(spv, spv->resultId);
        VIR_Shader_AddString(virShader, spv->virName, &typeNameId);
        typeName = spv->virName;
        setSkipNameCheckFlag = gcvTRUE;
    }

    blockNameLength = (gctUINT)gcoOS_StrLen(typeName, gcvNULL);

    /* If this IO block has instance name. */
    if (spvSym->virSymbolKind == VIR_SYM_IOBLOCK &&
        SPV_ID_VIR_NAME_ID(id) != VIR_INVALID_ID)
    {
        VIR_NameId nameId;

        typeName = VIR_Shader_GetStringFromId(virShader, typeNameId);
        instanceName = VIR_Shader_GetStringFromId(virShader, SPV_ID_VIR_NAME_ID(id));
        instanceNameLength = (gctUINT)gcoOS_StrLen(instanceName, gcvNULL);

        if (__SpvGetBuiltInBlockName(virShader, typeName, instanceName, &nameId))
        {
            typeNameId = nameId;
        }
        else
        {
            if (!SPV_IS_EMPTY_STRING(instanceName) && !SPV_IS_EMPTY_STRING(typeName))
            {
                gctCHAR iobName[SPV_VIR_NAME_SIZE * 2 + 1];
                gctUINT iobNameSize = SPV_VIR_NAME_SIZE * 2 + 1;

                gcoOS_ZeroMemory(iobName, iobNameSize);
                gcoOS_StrCatSafe(iobName, iobNameSize, typeName);
                gcoOS_StrCatSafe(iobName, iobNameSize, ".");
                gcoOS_StrCatSafe(iobName, iobNameSize, instanceName);

                VIR_Shader_AddString(virShader, iobName, &nameId);

                typeNameId = nameId;
            }
        }
    }

    VIR_Shader_AddSymbol(virShader,
        spvSym->virSymbolKind,
        typeNameId,
        virType,
        spvSym->virStorageClass,
        &symId);

    sym = VIR_Shader_GetSymFromId(virShader, symId);

    VIR_Symbol_SetLayoutQualifier(sym, spvSym->virLayoutQual);
    VIR_Symbol_SetFlag(sym,VIR_SYMFLAG_WITHOUT_REG);
    if (setSkipNameCheckFlag || SPV_SKIP_NAME_CHECK)
    {
        VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_SKIP_NAME_CHECK);
    }
    VIR_Symbol_SetPrecision(sym, VIR_PRECISION_HIGH);
    VIR_Symbol_SetTyQualifier(sym, VIR_TYQUAL_CONST);
    VIR_Symbol_SetLocation(sym, -1);

    /* Check if we need to calculate offset/stride for this block. */
    baseVirType = virType;
    while (VIR_Type_isArray(baseVirType))
    {
        baseVirType = VIR_Shader_GetTypeFromId(virShader, VIR_Type_GetBaseTypeId(baseVirType));
    }
    gcmASSERT(VIR_Type_isStruct(baseVirType));

    fields = VIR_Type_GetFields(baseVirType);
    fieldSymId = ((VIR_Id)(fields)->ids[(0)]);
    fieldSym = VIR_Shader_GetSymFromId(virShader, fieldSymId);
    fieldInfo = VIR_Symbol_GetFieldInfo(fieldSym);

    if ((spvSym->virSymbolKind == VIR_SYM_UBO || spvSym->virSymbolKind == VIR_SYM_SBO) &&
        (VIR_FieldInfo_GetOffset(fieldInfo) == (gctUINT)-1))
    {
        VIR_InterfaceBlock_CalcDataByteSize(virShader,
                                            sym);
    }

    if (spvSym->virSymbolKind == VIR_SYM_UBO)
    {
        VIR_UniformBlock *ubo;

        /* set UBO data. */
        ubo = sym->u2.ubo;
        gcmASSERT(ubo);
        ubo->sym = symId;
        VIR_UBO_SetBlockIndex(ubo, (gctINT16)VIR_IdList_Count(VIR_Shader_GetUniformBlocks(virShader)) - 1);
        VIR_UBO_SetFlag(ubo, __SpvGetIBFlag(spv, virShader, id, type, spvSym));
    }
    else if (spvSym->virSymbolKind == VIR_SYM_SBO)
    {
        VIR_StorageBlock *sbo;

        /* set SBO data. */
        sbo = sym->u2.sbo;
        gcmASSERT(sbo);
        sbo->sym = symId;
        VIR_SBO_SetBlockIndex(sbo, (gctINT16)VIR_IdList_Count(VIR_Shader_GetSSBlocks(virShader)) - 1);
        VIR_SBO_SetFlag(sbo, __SpvGetIBFlag(spv, virShader, id, type, spvSym));

    }
    else if (spvSym->virSymbolKind == VIR_SYM_IOBLOCK)
    {
        VIR_IOBlock *iob;

        iob = sym->u2.ioBlock;
        gcmASSERT(iob);
        iob->sym = symId;
        VIR_IOBLOCK_SetBlockIndex(iob, (gctINT16)VIR_IdList_Count(VIR_Shader_GetIOBlocks(virShader)) - 1);
        VIR_IOBLOCK_SetStorage(iob, spvSym->virStorageClass);
        VIR_IOBLOCK_SetFlag(iob, __SpvGetIBFlag(spv, virShader, id, type, spvSym));
        VIR_IOBLOCK_SetBlockNameLength(iob, blockNameLength);
        VIR_IOBLOCK_SetInstanceNameLength(iob, instanceNameLength);
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    /* record symID, so we could get sym from id */
    SPV_SET_IDDESCRIPTOR_SYM(spv, id, symId);
    SPV_SET_IDDESCRIPTOR_TYPE(spv, id, SPV_ID_TYPE_VIR_TYPE_ID(baseType));
    SPV_SET_IDDESCRIPTOR_SPV_TYPE(spv, id, baseType);
    SPV_ID_TYPE(id) = SPV_ID_TYPE_SYMBOL;
    SPV_ID_SYM_IS_FUNC_PARAM(id) = gcvFALSE;

    return VSC_ERR_NONE;
}

static VSC_ErrCode __SpvCalStrideOffsetForSharedVariable(
    VIR_Shader * virShader,
    VIR_Type *  BlockType,
    VIR_SymId   VariableSymId,
    VIR_Type *  Type,
    gctINT * ArrayStride,
    gctINT * MatrixStride,
    gctINT * Offset
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_TypeKind            tyKind = VIR_Type_GetKind(Type);
    VIR_TypeId              tyId = VIR_Type_GetIndex(Type);
    VIR_Type               *baseType = gcvNULL;
    gctINT                  arrayStride = -1, matrixStride = -1;

    /* For a aggregate(array/strut), we need to check its elements. */
    if (tyKind == VIR_TY_ARRAY)
    {
        baseType = VIR_Shader_GetTypeFromId(virShader, VIR_Type_GetBaseTypeId(Type));
        arrayStride = VIR_Type_GetTypeByteSize(virShader, baseType);

        __SpvCalStrideOffsetForSharedVariable(virShader,
                                              BlockType,
                                              VariableSymId,
                                              baseType,
                                              gcvNULL,
                                              &matrixStride,
                                              gcvNULL);
    }
    else if (tyKind == VIR_TY_STRUCT)
    {
        VIR_SymIdList      *fields = VIR_Type_GetFields(Type);
        gctUINT             offset = 0;
        gctUINT             i;

        for (i = 0; i < VIR_IdList_Count(fields); i++)
        {
            VIR_Symbol     *field_sym = VIR_Shader_GetSymFromId(virShader, fields->ids[i]);
            VIR_Type       *field_type = VIR_Symbol_GetType(field_sym);
            VIR_FieldInfo  *fieldInfo = VIR_Symbol_GetFieldInfo(field_sym);
            gctINT          fieldArrayStride = -1, fieldMatrixStride = -1;

            /* Calculate stride/offset for this field member. */
            __SpvCalStrideOffsetForSharedVariable(virShader,
                                                  BlockType,
                                                  VariableSymId,
                                                  field_type,
                                                  &fieldArrayStride,
                                                  &fieldMatrixStride,
                                                  gcvNULL);
            VIR_FieldInfo_SetOffset(fieldInfo, offset);
            VIR_FieldInfo_SetArrayStride(fieldInfo, fieldArrayStride);
            VIR_FieldInfo_SetMatrixStride(fieldInfo, fieldMatrixStride);
            VIR_Type_SetArrayStride(field_type, fieldArrayStride);

            /* Update the offset. */
            offset += VIR_Type_GetTypeByteSize(virShader, field_type);
        }
    }
    else if (tyKind == VIR_TY_MATRIX)
    {
        matrixStride = (gctINT) (VIR_GetTypeSize(tyId) / VIR_GetTypeRows(tyId));
    }

    if (Offset)
    {
        VIR_SymIdList      *fields = VIR_Type_GetFields(BlockType);
        VIR_SymId           prevFieldSymId = VIR_INVALID_ID;
        VIR_Symbol         *prevFieldSym = gcvNULL;
        VIR_Type           *prevFieldType = gcvNULL;
        VIR_FieldInfo      *prevFieldInfo = gcvNULL;
        gctUINT             offset = 0;
        gctUINT             i;

        for (i = 0; i < VIR_IdList_Count(fields); i++)
        {
            if ((VIR_SymId)fields->ids[i] == VariableSymId)
            {
                break;
            }
            prevFieldSymId = (VIR_SymId)fields->ids[i];
        }

        if (prevFieldSymId != VIR_INVALID_ID)
        {
            prevFieldSym = VIR_Shader_GetSymFromId(virShader, prevFieldSymId);
            prevFieldType = VIR_Symbol_GetType(prevFieldSym);
            prevFieldInfo = VIR_Symbol_GetFieldInfo(prevFieldSym);

            offset = VIR_FieldInfo_GetOffset(prevFieldInfo) + VIR_Type_GetTypeByteSize(virShader, prevFieldType);
        }

        *Offset = offset;
    }

    /* Save the result. */
    if (ArrayStride)
    {
        *ArrayStride = arrayStride;
    }

    if (MatrixStride)
    {
        *MatrixStride = matrixStride;
    }

    return errCode;
}

static VSC_ErrCode __SpvAddSharedSymbol(
    gcSPV spv,
    VIR_Shader * virShader,
    SpvId id,
    SpvId type)
{
    VIR_SymId               symId;
    VIR_NameId              nameId;
    gctUINT                 arrayLength = 1;
    gctUINT                 size = 1;
    SpvId                   elementType;
    VIR_Type               *virType;
    VIR_Type               *virBlockType;
    VIR_Symbol             *sym;
    VIR_FieldInfo          *fieldInfo = gcvNULL;
    gctINT                  arrayStride = -1, matrixStride = -1, offset = 0;

    if (!SPV_HAS_WORKGROUP())
    {
        return VSC_ERR_INVALID_DATA;
    }

    while (SPV_ID_TYPE_IS_POINTER(type))
    {
        type = SPV_ID_TYPE_POINTER_OBJECT_SPV_TYPE(type);
    }
    elementType = type;

    /* get the size of this variable */
    if (SPV_ID_TYPE_IS_ARRAY(type))
    {
        arrayLength = SPV_ID_TYPE_ARRAY_LENGTH(type);
        elementType = SPV_ID_TYPE_ARRAY_BASE_TYPE_ID(type);
    }

    if (SPV_ID_TYPE_IS_SCALAR(elementType) || SPV_ID_TYPE_IS_BOOLEAN(elementType))
    {
        size = 4 * 1;
    }
    else if (SPV_ID_TYPE_IS_VECTOR(elementType))
    {
        size = 4 * SPV_ID_TYPE_VEC_COMP_NUM(elementType);
    }
    else if (SPV_ID_TYPE_IS_MATRIX(elementType))
    {
        size = 4 * SPV_ID_TYPE_MAT_COL_COUNT(elementType) * SPV_ID_TYPE_VEC_COMP_NUM(SPV_ID_TYPE_MAT_COL_TYPE(elementType));
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }
    size *= arrayLength;

    if (SPV_ID_VIR_NAME_ID(spv->resultId) == VIR_INVALID_ID)
    {
        /* Generate a name and add name to VIR */
        __SpvGenerateVIRName(spv, id);
        VIR_Shader_AddString(virShader, spv->virName, &nameId);
    }
    else
    {
        nameId = SPV_ID_VIR_NAME_ID(spv->resultId);
    }

    if (SPV_ID_TYPE_IS_POINTER(type))
    {
        virType = SPV_ID_TYPE_VIR_TYPE(SPV_ID_TYPE_POINTER_OBJECT_SPV_TYPE(type));
    }
    else
    {
        virType = SPV_ID_TYPE_VIR_TYPE(type);
    }

    SPV_ADD_WORKGROUP_MEMBER(spv, spv->resultId, SPV_WORKGROUP_INFO()->groupSize);
    SPV_WORKGROUP_INFO()->groupSize += size;

    virBlockType = VIR_Shader_GetTypeFromId(virShader, SPV_WORKGROUP_INFO()->sharedSboTypeId);

    VIR_Shader_AddFieldSymbol(virShader,
        nameId,
        virType,
        virBlockType,
        VIR_STORAGE_SHARED_VAR,
        &symId);

    VIR_Type_AddField(virShader,
        virBlockType,
        symId);

    sym = VIR_Shader_GetSymFromId(virShader, symId);

    VIR_Symbol_SetPrecision(sym, VIR_PRECISION_HIGH);
    VIR_Symbol_SetTyQualifier(sym, VIR_TYQUAL_NONE);
    VIR_Symbol_SetLayoutQualifier(sym, VIR_LAYQUAL_NONE);
    VIR_Symbol_SetLocation(sym, -1);

    /*
    ** Since we put shared variables into a sbo,
    ** we need to evaluate the arrayStride/matrixStride/offset for its fields and itself.
    */
    __SpvCalStrideOffsetForSharedVariable(virShader,
                                          virBlockType,
                                          symId,
                                          VIR_Symbol_GetType(sym),
                                          &arrayStride,
                                          &matrixStride,
                                          &offset);
    fieldInfo = VIR_Symbol_GetFieldInfo(sym);
    VIR_Type_SetArrayStride(virType, arrayStride);
    VIR_FieldInfo_SetOffset(fieldInfo, offset);
    VIR_FieldInfo_SetArrayStride(fieldInfo, arrayStride);
    VIR_FieldInfo_SetMatrixStride(fieldInfo, matrixStride);

    SPV_SET_IDDESCRIPTOR_NAME(spv, spv->resultId, nameId);

    /* record symID, so we could get sym from id */
    SPV_SET_IDDESCRIPTOR_SYM(spv, spv->resultId, symId);
    SPV_SET_IDDESCRIPTOR_TYPE(spv, spv->resultId, SPV_ID_TYPE_VIR_TYPE_ID(spv->resultTypeId));
    SPV_SET_IDDESCRIPTOR_SPV_TYPE(spv, spv->resultId, spv->resultTypeId);
    SPV_ID_TYPE(spv->resultId) = SPV_ID_TYPE_SYMBOL;
    SPV_ID_SYM_IS_FUNC_PARAM(spv->resultId) = gcvFALSE;
    SPV_ID_SYM_IS_WORKGROUP(spv->resultId) = gcvTRUE;

    return VSC_ERR_NONE;
}

static VIR_ImageFormat __SpvVkFormat2VirImageFormat(VkFormat vkformat)
{
    VIR_ImageFormat imageFormat = VIR_IMAGE_FORMAT_NONE;

    switch (vkformat)
    {
    /* FLOAT */
    case VK_FORMAT_R32G32B32A32_SFLOAT:
        imageFormat = VIR_IMAGE_FORMAT_RGBA32F;
        break;

    case VK_FORMAT_R32G32_SFLOAT:
        imageFormat = VIR_IMAGE_FORMAT_RG32F;
        break;

    case VK_FORMAT_R32_SFLOAT:
        imageFormat = VIR_IMAGE_FORMAT_R32F;
        break;

    case VK_FORMAT_R16G16B16A16_SFLOAT:
        imageFormat = VIR_IMAGE_FORMAT_RGBA16F;
        break;

    case VK_FORMAT_R16G16_SFLOAT:
        imageFormat = VIR_IMAGE_FORMAT_RG16F;
        break;

    case VK_FORMAT_R16_SFLOAT:
        imageFormat = VIR_IMAGE_FORMAT_R16F;
        break;

    /* Vulkan driver uses the same HW format for these image formats. */
    case VK_FORMAT_B8G8R8A8_UNORM:
    case VK_FORMAT_R8G8B8A8_UNORM:
        imageFormat = VIR_IMAGE_FORMAT_RGBA8;
        break;

    case VK_FORMAT_R8G8_UNORM:
        imageFormat = VIR_IMAGE_FORMAT_RG8;
        break;

    case VK_FORMAT_R8_UNORM:
        imageFormat = VIR_IMAGE_FORMAT_RG8;
        break;

    case VK_FORMAT_R8G8B8A8_SNORM:
        imageFormat = VIR_IMAGE_FORMAT_RGBA8_SNORM;
        break;

    /* SINT */
    case VK_FORMAT_R32G32B32A32_SINT:
        imageFormat = VIR_IMAGE_FORMAT_RGBA32I;
        break;

    case VK_FORMAT_R32G32_SINT:
        imageFormat = VIR_IMAGE_FORMAT_RG32I;
        break;

    case VK_FORMAT_R32_SINT:
        imageFormat = VIR_IMAGE_FORMAT_R32I;
        break;

    case VK_FORMAT_R16G16B16A16_SINT:
        imageFormat = VIR_IMAGE_FORMAT_RGBA16I;
        break;

    case VK_FORMAT_R16G16_SINT:
        imageFormat = VIR_IMAGE_FORMAT_RG16I;
        break;

    case VK_FORMAT_R16_SINT:
        imageFormat = VIR_IMAGE_FORMAT_R16I;
        break;

    case VK_FORMAT_R8G8B8A8_SINT:
        imageFormat = VIR_IMAGE_FORMAT_RGBA8I;
        break;

    case VK_FORMAT_R8G8_SINT:
        imageFormat = VIR_IMAGE_FORMAT_RG8I;
        break;

    case VK_FORMAT_R8_SINT:
        imageFormat = VIR_IMAGE_FORMAT_R8I;
        break;

    /* UINT */
    case VK_FORMAT_R32G32B32A32_UINT:
        imageFormat = VIR_IMAGE_FORMAT_RGBA32UI;
        break;

    case VK_FORMAT_R32G32_UINT:
        imageFormat = VIR_IMAGE_FORMAT_RG32UI;
        break;

    case VK_FORMAT_R32_UINT:
        imageFormat = VIR_IMAGE_FORMAT_R32UI;
        break;

    case VK_FORMAT_R16G16B16A16_UINT:
        imageFormat = VIR_IMAGE_FORMAT_RGBA16UI;
        break;

    case VK_FORMAT_R16G16_UINT:
        imageFormat = VIR_IMAGE_FORMAT_RG16UI;
        break;

    case VK_FORMAT_R16_UINT:
        imageFormat = VIR_IMAGE_FORMAT_R16UI;
        break;

    case VK_FORMAT_R8G8B8A8_UINT:
        imageFormat = VIR_IMAGE_FORMAT_RGBA8UI;
        break;

    case VK_FORMAT_R8G8_UINT:
        imageFormat = VIR_IMAGE_FORMAT_RG8UI;
        break;

    case VK_FORMAT_R8_UINT:
        imageFormat = VIR_IMAGE_FORMAT_R8UI;
        break;

    /* PACK */
    case VK_FORMAT_R5G6B5_UNORM_PACK16:
        imageFormat = VIR_IMAGE_FORMAT_R5G6B5_UNORM_PACK16;
        break;

    case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
        imageFormat = VIR_IMAGE_FORMAT_ABGR8_UNORM_PACK32;
        break;

    case VK_FORMAT_A8B8G8R8_UINT_PACK32:
        imageFormat = VIR_IMAGE_FORMAT_ABGR8UI_PACK32;
        break;

    case VK_FORMAT_A8B8G8R8_SINT_PACK32:
        imageFormat = VIR_IMAGE_FORMAT_ABGR8I_PACK32;
        break;

    /* Vulkan driver uses the same HW format for these image formats. */
    case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        imageFormat = VIR_IMAGE_FORMAT_A2R10G10B10_UNORM_PACK32;
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    return imageFormat;
}

static VIR_ImageFormat __SpvImageFormatToVirImageFormat(SpvImageFormat format)
{
    VIR_ImageFormat imageFormat = VIR_IMAGE_FORMAT_NONE;

    switch (format)
    {
    case SpvImageFormatUnknown:         imageFormat = VIR_IMAGE_FORMAT_NONE; break;
    case SpvImageFormatRgba32f:         imageFormat = VIR_IMAGE_FORMAT_RGBA32F; break;
    case SpvImageFormatRgba16f:         imageFormat = VIR_IMAGE_FORMAT_RGBA16F; break;
    case SpvImageFormatRg16f:           imageFormat = VIR_IMAGE_FORMAT_RG16F; break;
    case SpvImageFormatR32f:            imageFormat = VIR_IMAGE_FORMAT_R32F; break;
    case SpvImageFormatRgba8:           imageFormat = VIR_IMAGE_FORMAT_RGBA8; break;
    case SpvImageFormatRgba8Snorm:      imageFormat = VIR_IMAGE_FORMAT_RGBA8_SNORM; break;
    case SpvImageFormatRgba32i:         imageFormat = VIR_IMAGE_FORMAT_RGBA32I; break;
    case SpvImageFormatRgba16i:         imageFormat = VIR_IMAGE_FORMAT_RGBA16I; break;
    case SpvImageFormatRgba8i:          imageFormat = VIR_IMAGE_FORMAT_RGBA8I; break;
    case SpvImageFormatR32i:            imageFormat = VIR_IMAGE_FORMAT_R32I; break;
    case SpvImageFormatRgba32ui:        imageFormat = VIR_IMAGE_FORMAT_RGBA32UI; break;
    case SpvImageFormatRgba16ui:        imageFormat = VIR_IMAGE_FORMAT_RGBA16UI; break;
    case SpvImageFormatRgba8ui:         imageFormat = VIR_IMAGE_FORMAT_RGBA8UI; break;
    case SpvImageFormatR32ui:           imageFormat = VIR_IMAGE_FORMAT_R32UI; break;
    case SpvImageFormatRg32i:           imageFormat = VIR_IMAGE_FORMAT_RG32I; break;
    case SpvImageFormatRg16ui:          imageFormat = VIR_IMAGE_FORMAT_RG16UI; break;
    case SpvImageFormatRg16i:           imageFormat = VIR_IMAGE_FORMAT_RG16I; break;
    case SpvImageFormatRg32f:           imageFormat = VIR_IMAGE_FORMAT_RG32F; break;
    case SpvImageFormatR16f:            imageFormat = VIR_IMAGE_FORMAT_R16F; break;
    case SpvImageFormatRg8i:            imageFormat = VIR_IMAGE_FORMAT_RG8I; break;
    case SpvImageFormatR16i:            imageFormat = VIR_IMAGE_FORMAT_R16I; break;
    case SpvImageFormatR8i:             imageFormat = VIR_IMAGE_FORMAT_R8I; break;
    case SpvImageFormatRgba16:          imageFormat = VIR_IMAGE_FORMAT_RGBA16F; break;
    case SpvImageFormatRg16:            imageFormat = VIR_IMAGE_FORMAT_RG16F; break;
    case SpvImageFormatRg8:             imageFormat = VIR_IMAGE_FORMAT_RG8; break;
    case SpvImageFormatR16:             imageFormat = VIR_IMAGE_FORMAT_R16F; break;
    case SpvImageFormatR8:              imageFormat = VIR_IMAGE_FORMAT_R8; break;
    case SpvImageFormatRg32ui:          imageFormat = VIR_IMAGE_FORMAT_RG32UI; break;
    case SpvImageFormatRg8ui:           imageFormat = VIR_IMAGE_FORMAT_RG8UI; break;
    case SpvImageFormatR16ui:           imageFormat = VIR_IMAGE_FORMAT_RG16UI; break;
    case SpvImageFormatR8ui:            imageFormat = VIR_IMAGE_FORMAT_R8UI; break;

    case SpvImageFormatR11fG11fB10f:
    case SpvImageFormatRgb10A2:
    case SpvImageFormatRgba16Snorm:
    case SpvImageFormatRg16Snorm:
    case SpvImageFormatRg8Snorm:
    case SpvImageFormatR16Snorm:
    case SpvImageFormatR8Snorm:
    case SpvImageFormatRgb10a2ui:
        gcmASSERT(gcvFALSE);
        imageFormat = VIR_IMAGE_FORMAT_NONE;
        break;

    default: break;
    }

    return imageFormat;
}

static VIR_SymId __SpvAddIdSymbol(
    gcSPV spv,
    VIR_Shader * virShader,
    char * name,
    SpvId id,
    SpvId type,
    VIR_SymbolKind virSymbolKind,
    VIR_StorageClass virStorageClass,
    gctBOOL     compilerGen
    )
{
    SpvId baseTypeId;
    VIR_NameId nameId;
    VIR_SymId symId;
    VIR_Symbol * sym;
    VIR_Type * virType;
    VIR_TypeId virTypeId = VIR_INVALID_ID;
    VSC_ErrCode errCode;
    gctBOOL setLocation = gcvTRUE;
    gctBOOL setSkipNameCheckFlag = gcvFALSE;
    gctBOOL treatSubPassAsSampler = gcvFALSE;
    SpvAttachmentDesc* attachmentDesc = gcvNULL;
    Spv_AttachmentFlag attachmentFlag = SPV_ATTACHMENTFLAG_NONE;

    /* Get the type id. */
    while (SPV_ID_TYPE_IS_POINTER(type))
    {
        type = SPV_ID_TYPE_POINTER_OBJECT_SPV_TYPE(type);
    }
    baseTypeId = type;

    /* Get the non-array base type id. */
    if (SPV_ID_TYPE_IS_ARRAY(baseTypeId))
    {
        baseTypeId = SPV_ID_TYPE_ARRAY_BASE_TYPE_ID(baseTypeId);
    }

    /* Get attachmentDesc. */
    if (SPV_ID_TYPE_IS_IMAGE(baseTypeId))
    {
        if ((SPV_ID_TYPE_IMAGE_DIM(baseTypeId) == SpvDimSubpassData) &&
            (spv->renderpassInfo != gcvNULL))
        {
            SpvCovDecorator        *dec = spv->decorationList;
            gctUINT                 target = id;
            gctINT                  memberIndex = ~0U;
            gctINT                  subpassAttachmentIndex = -1;
            gctINT                  inputAttachmentIndex = -1;
            SpvRenderPassInfo      *renderPass = spv->renderpassInfo;

            /* Find the match decoration by using target and member index.*/
            SPV_GET_DECORATOR(dec, target, memberIndex);

            if (dec != gcvNULL)
            {
                subpassAttachmentIndex = dec->decorationData.inputAttachmentIndex;
            }

            if ((renderPass->subPassInfoCount > 0) &&
                (subpassAttachmentIndex >= 0) &&
                (subpassAttachmentIndex <= (gctINT)renderPass->subPassInfoCount))
            {
                inputAttachmentIndex = renderPass->subPassInfo[spv->subPass].input_attachment_index[subpassAttachmentIndex];
            }

            if (inputAttachmentIndex >= 0 && inputAttachmentIndex < (gctINT)renderPass->attachmentCount)
            {
                attachmentDesc = &renderPass->attachments[inputAttachmentIndex];
                treatSubPassAsSampler = (attachmentDesc->attachmentFlag & SPV_ATTACHMENTFLAG_TREAT_AS_SAMPLER) ? gcvTRUE : gcvFALSE;
                attachmentFlag = attachmentDesc->attachmentFlag;
            }
        }
    }

    /* Create the name string. */
    if (name != gcvNULL)
    {
        VIR_Shader_AddString(virShader, name, &nameId);
    }
    else if (SPV_ID_VIR_NAME_ID(id) == VIR_INVALID_ID)
    {
        /* Generate a name and add name to VIR */
        __SpvGenerateVIRName(spv, id);
        VIR_Shader_AddString(virShader, spv->virName, &nameId);
        setSkipNameCheckFlag = gcvTRUE;
    }
    else
    {
        nameId = SPV_ID_VIR_NAME_ID(id);
    }

    /* If we treat this input attachment as a sampler, get the sampler type. */
    if (treatSubPassAsSampler)
    {
        if (SPV_ID_TYPE_IS_ARRAY(type))
        {
            virTypeId = SPV_ID_TYPE_ARRAY_SAMPLER_IMAGE_TYPE(type);
        }
        else
        {
            virTypeId = SPV_ID_TYPE_IMAGE_SAMPLER_IMAGE_TYPE(type);
        }
        virType = VIR_Shader_GetTypeFromId(virShader, virTypeId);

        virSymbolKind = VIR_SYM_SAMPLER;
    }
    else
    {
        virTypeId = SPV_ID_TYPE_VIR_TYPE_ID(type);
        virType = SPV_ID_TYPE_VIR_TYPE(type);
    }

    /*Create VIR sym, we don't know type or other attribute*/
    errCode = VIR_Shader_AddSymbol(virShader,
        virSymbolKind,
        nameId,
        virType,
        virStorageClass,
        &symId);

    if(errCode == VSC_ERR_REDEFINITION && virSymbolKind == VIR_SYM_VARIABLE && virStorageClass == VIR_STORAGE_LOCAL)
    {
        VIR_Shader_DuplicateVariablelFromSymId(virShader, symId, &symId);
        setLocation = gcvFALSE;
    }

    sym = VIR_Shader_GetSymFromId(virShader, symId);

    VIR_Symbol_SetPrecision(sym, VIR_PRECISION_HIGH);
    VIR_Symbol_SetTyQualifier(sym, VIR_TYQUAL_NONE);
    VIR_Symbol_SetLayoutQualifier(sym, VIR_LAYQUAL_NONE);
    if (!(VIR_Symbol_isUniform(sym) ||
          VIR_Symbol_isSampler(sym) ||
          VIR_Symbol_isTexure(sym)  ||
          VIR_Symbol_isImage(sym)))
    {
        VIR_Symbol_SetFlag(sym,VIR_SYMFLAG_WITHOUT_REG);
    }
    if (setSkipNameCheckFlag || SPV_SKIP_NAME_CHECK)
    {
        VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_SKIP_NAME_CHECK);
    }
    if (treatSubPassAsSampler)
    {
        VIR_Symbol_SetFlag(sym, VIR_SYMUNIFORMFLAG_TREAT_IMAGE_AS_SAMPLER);
    }

    /* duplicated variable already has location information */
    if (setLocation)
    {
        VIR_Symbol_SetLocation(sym, -1);
    }

    /* The image format is saved in the base type id. */
    if (SPV_ID_TYPE_IS_IMAGE(baseTypeId) & !treatSubPassAsSampler)
    {
        if (attachmentDesc != gcvNULL)
        {
            VIR_Symbol_SetImageFormat(sym, __SpvVkFormat2VirImageFormat(attachmentDesc->format));
        }
        else
        {
            VIR_Symbol_SetImageFormat(sym, __SpvImageFormatToVirImageFormat(SPV_ID_TYPE_IMAGE_FORMAT(baseTypeId)));
        }
    }

    SPV_SET_IDDESCRIPTOR_NAME(spv, id, nameId);

    /* record symID, so we could get sym from id */
    SPV_SET_IDDESCRIPTOR_SYM(spv, id, symId);
    SPV_SET_IDDESCRIPTOR_TYPE(spv, id, virTypeId);
    SPV_SET_IDDESCRIPTOR_SPV_TYPE(spv, id, type);
    SPV_ID_TYPE(id) = SPV_ID_TYPE_SYMBOL;
    SPV_ID_SYM_IS_FUNC_PARAM(id) = gcvFALSE;

    /* Set attachment flag. */
    SPV_ID_SYM_ATTACHMENT_FLAG(id) = attachmentFlag;

    if (compilerGen)
    {
        VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_COMPILER_GEN);
    }

    return symId;
}

static VIR_SymFlag __SpvConvDecoratorToSymFlag(SpvConvDecorationData *DecorationData)
{
    VIR_SymFlag symFlag = VIR_SYMFLAG_NONE;

    /* Check interpolation qualifier. */
    if (DecorationData->interpolationQualifier == SPV_INTERPOLATION_FLAT)
    {
        symFlag |= VIR_SYMFLAG_FLAT;
    }

    /* Check variance qualifier. */
    if (DecorationData->varianceQualifier == SPV_VARIANCE_INVARIANT)
    {
        symFlag |= VIR_SYMFLAG_INVARIANT;
    }

    /* Check auxiliary qualifier. */
    if (DecorationData->auxiliaryQualifier == SPV_AUXILIARY_CENTROID)
    {
        symFlag |= VIR_SYMFLAG_ISCENTROID;
    }
    if (DecorationData->auxiliaryQualifier == SPV_AUXILIARY_SAMPLE)
    {
        symFlag |= VIR_SYMFLAG_ISSAMPLE;
    }

    if (DecorationData->preciseQualifier == SPV_PRECISE_ENABLE)
    {
        symFlag |= VIR_SYMFLAG_PRECISE;
    }

    return symFlag;
}

static VIR_LayoutQual __SpvConvDecoratorToSymLayout(SpvConvDecorationData *DecorationData)
{
    VIR_LayoutQual layout = VIR_LAYQUAL_NONE;

    if (DecorationData->layoutQualifier & SPV_LAYOUT_ROW_MAJOR)
    {
        layout |= VIR_LAYQUAL_ROW_MAJOR;
    }
    if (DecorationData->layoutQualifier & SPV_LAYOUT_COL_MAJOR)
    {
        layout |= VIR_LAYQUAL_COLUMN_MAJOR;
    }
    if (DecorationData->layoutQualifier & SPV_LAYOUT_GLSL_SHARED)
    {
        layout |= VIR_LAYQUAL_SHARED;
    }
    if (DecorationData->layoutQualifier & SPV_LAYOUT_GLSL_PACKED)
    {
        layout |= VIR_LAYQUAL_PACKED;
    }

    return layout;
}

static VSC_ErrCode __SpvIDCopy(gcSPV spv, VIR_Shader * virShader, SpvId from, SpvId to)
{
    VSC_ErrCode         virErrCode = VSC_ERR_NONE;
    VIR_Instruction    *virInst;
    VIR_Operand        *operand;
    SpvId               fromTypeId = 0;
    SpvIDType           fromIdType = SPV_ID_TYPE(from);
    VIR_NameId          nameId = SPV_ID_VIR_NAME_ID(to);
    VIR_Symbol         *toVirSym;
    VIR_Symbol         *fromVirSym;
    VIR_TypeId          virTypeId;
    VIR_Type           *virType = gcvNULL;

    /* Can we just do const and variable copy? */
    gcmASSERT((fromIdType == SPV_ID_TYPE_CONST) || (fromIdType == SPV_ID_TYPE_SYMBOL));

    if (fromIdType == SPV_ID_TYPE_CONST)
    {
        fromTypeId = SPV_ID_CST_SPV_TYPE(from);
    }
    else if (fromIdType == SPV_ID_TYPE_SYMBOL)
    {
        fromTypeId = SPV_ID_SYM_SPV_TYPE(from);
    }

    /* Create the result variable. */
    if (nameId == VIR_INVALID_ID)
    {
        __SpvAddIdSymbol(spv, virShader, gcvNULL, to, fromTypeId, VIR_SYM_VARIABLE, VIR_STORAGE_GLOBAL, gcvFALSE);
    }

    toVirSym = SPV_ID_VIR_SYM(to);
    virTypeId = SPV_ID_VIR_TYPE_ID(to);
    virType = VIR_Shader_GetTypeFromId(virShader, virTypeId);

    /* Copy the data from vector source first. */
    VIR_Function_AddInstruction(spv->virFunction,
        VIR_OP_MOV,
        virTypeId,
        &virInst);

    VIR_Inst_SetConditionOp(virInst, VIR_COP_ALWAYS);

    /* Set DEST. */
    operand = VIR_Inst_GetDest(virInst);
    VIR_Operand_SetSym(operand, toVirSym);
    VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
    VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
    VIR_Operand_SetEnable(operand, __SpvGenEnable(spv, virType, SPV_ID_SYM_SPV_TYPE(to)));
    VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
    VIR_Operand_SetTypeId(operand, virTypeId);

    /* Set SOURCE0 */
    operand = VIR_Inst_GetSource(virInst, 0);
    /*VIR_Operand_SetSwizzle(operand, VIR_Swizzle_GenSwizzleByComponentCount(componentCount));*/
    VIR_Operand_SetSwizzle(operand, __SpvID2Swizzle(spv, from));
    VIR_Operand_SetTypeId(operand, virTypeId);
    VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);

    switch (fromIdType)
    {
    case SPV_ID_TYPE_SYMBOL:
        fromVirSym = SPV_ID_VIR_SYM(from);
        VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
        VIR_Operand_SetSym(operand, fromVirSym);
        break;

    case SPV_ID_TYPE_CONST:
        VIR_Operand_SetOpKind(operand, VIR_OPND_CONST);
        VIR_Operand_SetConstId(operand, SPV_ID_VIR_CONST_ID(from));
        break;

    default:
        break;
    }

    return virErrCode;
}

static VSC_ErrCode __SpvConvToBuiltInConst(
    IN  gcSPV spv,
    IN  VIR_Shader * virShader,
    IN  SpvCovDecorator * dec,
    IN  SpvId targetId
    )
{
    VSC_ErrCode             virErrCode = VSC_ERR_NONE;

    gcmASSERT(dec);

    if (dec->decorationData.builtIn == -1)
    {
        return virErrCode;
    }

    switch (dec->decorationData.builtIn)
    {
    case SpvBuiltInWorkgroupSize:
        virShader->shaderLayout.compute.workGroupSize[0] = SPV_ID_VIR_CONST(targetId).vecVal.u32Value[0];
        virShader->shaderLayout.compute.workGroupSize[1] = SPV_ID_VIR_CONST(targetId).vecVal.u32Value[1];
        virShader->shaderLayout.compute.workGroupSize[2] = SPV_ID_VIR_CONST(targetId).vecVal.u32Value[2];
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    return virErrCode;
}

static VSC_ErrCode __SpvConvDecoratorToVIR(
    IN  gcSPV spv,
    IN  VIR_Shader * virShader,
    IN  SpvVIRSymbolInternal * symSpv,
    IN  gctUINT targetId,
    IN  gctINT memberIndex,
    IN  VIR_Symbol * symbol
    )
{
    VSC_ErrCode             virErrCode = VSC_ERR_NONE;
    SpvCovDecorator        *dec = spv->decorationList;
    SpvConvDecorationData  *decorationData = gcvNULL;
    VIR_FieldInfo          *fieldInfo = gcvNULL;
    gctINT                  location;

    /* Find the decoration by target and member index. */
    SPV_GET_DECORATOR(dec, targetId, memberIndex);

    if (dec)
    {
        decorationData = &dec->decorationData;

        /* For a variable save the info into symSpv. */
        if (spv->opCode == SpvOpVariable)
        {
            /* Convert the decoration to symbol flag. */
            symSpv->virSymFlag |= __SpvConvDecoratorToSymFlag(decorationData);

            /* Convert the decoration to symbol layout. */
            symSpv->virLayoutQual  |= __SpvConvDecoratorToSymLayout(decorationData);

            /* handle special build in decoration */
            switch(decorationData->builtIn)
            {
            case SpvBuiltInInvocationId:
                if (spv->shaderStage == VSC_SHADER_STAGE_GS)
                {
                    VIR_Shader_SetFlag(virShader, VIR_SHFLAG_HAS_INSTANCEID);
                }
                symSpv->builtIn = gcvTRUE;
                break;
            case SpvBuiltInPrimitiveId:
                if (spv->shaderStage == VSC_SHADER_STAGE_GS && symSpv->virStorageClass == VIR_STORAGE_INPUT)
                {
                    VIR_Shader_SetFlag(virShader, VIR_SHFLAG_HAS_PRIMITIVEID);
                }
                symSpv->builtIn = gcvTRUE;
                break;

            case SpvBuiltInSampleId:
            case SpvBuiltInSamplePosition:
                symSpv->builtIn = gcvTRUE;
                VIR_Shader_SetFlag(virShader, VIR_SHFLAG_PS_SAMPLE_SHADING);
                break;

            case SpvBuiltInPosition:
            case SpvBuiltInPointSize:
            case SpvBuiltInClipDistance:
            case SpvBuiltInCullDistance:
            case SpvBuiltInVertexId:
            case SpvBuiltInInstanceId:
            case SpvBuiltInLayer:
            case SpvBuiltInViewportIndex:
            case SpvBuiltInTessLevelOuter:
            case SpvBuiltInTessLevelInner:
            case SpvBuiltInTessCoord :
            case SpvBuiltInPatchVertices:
            case SpvBuiltInFragCoord:
            case SpvBuiltInPointCoord :
            case SpvBuiltInFrontFacing:
            case SpvBuiltInSampleMask:
            case SpvBuiltInFragDepth:
            case SpvBuiltInHelperInvocation:
                symSpv->builtIn = gcvTRUE;
                break;

            case SpvBuiltInNumWorkgroups:
            /* special handling for numWorkGroups - should be an uniform in our HW */
                symSpv->spvStorage = SpvStorageClassUniform;
                symSpv->virSymbolKind = VIR_SYM_UNIFORM;
                symSpv->u1.uniformKind = VIR_UNIFORM_NUM_GROUPS;
                symSpv->compilerGen = gcvTRUE;
                symSpv->builtIn = gcvTRUE;
                break;

            case SpvBuiltInWorkgroupSize:
            case SpvBuiltInWorkgroupId:
            case SpvBuiltInLocalInvocationId:
            case SpvBuiltInGlobalInvocationId:
            case SpvBuiltInLocalInvocationIndex:
            case SpvBuiltInWorkDim:
            case SpvBuiltInGlobalSize:
            case SpvBuiltInEnqueuedWorkgroupSize:
            case SpvBuiltInGlobalOffset:
            case SpvBuiltInGlobalLinearId:
            case SpvBuiltInSubgroupSize:
            case SpvBuiltInSubgroupMaxSize:
            case SpvBuiltInNumSubgroups:
            case SpvBuiltInNumEnqueuedSubgroups:
            case SpvBuiltInSubgroupId:
            case SpvBuiltInSubgroupLocalInvocationId:
            case SpvBuiltInVertexIndex:
            case SpvBuiltInInstanceIndex:
                symSpv->builtIn = gcvTRUE;
                break;

            default:
                break;
            }

            /* Set the arrayStride/matrixStride/location/binding. */
            symSpv->arrayStride = SPV_SET_VALID_VALUE(decorationData->arrayStride, symSpv->arrayStride, -1);
            symSpv->matrixStride = SPV_SET_VALID_VALUE(decorationData->matrixStride, symSpv->matrixStride, -1);
            symSpv->location = SPV_SET_VALID_VALUE(decorationData->location, symSpv->location, -1);
            symSpv->binding = SPV_SET_VALID_VALUE(decorationData->binding, symSpv->binding, -1);
            symSpv->descriptorSet = SPV_SET_VALID_VALUE(decorationData->descriptorSet, symSpv->descriptorSet, -1);

            /* Set the offset/alignment. */
            symSpv->offset = SPV_SET_VALID_VALUE(decorationData->offset, symSpv->offset, -1);
            symSpv->alignment = SPV_SET_VALID_VALUE(decorationData->alignment, symSpv->alignment, -1);

            /* Check for interface block. */
            if (decorationData->isBlock)
            {
                if (symSpv->spvStorage == SpvStorageClassOutput ||
                    symSpv->spvStorage == SpvStorageClassInput)
                {
                    symSpv->virSymbolKind = VIR_SYM_IOBLOCK;
                }
                else
                {
                    if (symSpv->spvStorage == SpvStorageClassUniform         ||
                        symSpv->spvStorage == SpvStorageClassUniformConstant)
                    {
                        symSpv->virSymbolKind = VIR_SYM_UBO;
                    }
                }
            }

            if (decorationData->isBufferBlock)
            {
                if (symSpv->spvStorage == SpvStorageClassUniform         ||
                    symSpv->spvStorage == SpvStorageClassUniformConstant)
                {
                    symSpv->virSymbolKind = VIR_SYM_SBO;
                }
            }

            /* Check if it is RelaxedPrecision. */
            if (decorationData->isRelaxedPrecision)
            {
                symSpv->virPrecision = VIR_PRECISION_MEDIUM;
            }

            /* if patch is used for gl_TessCoord, just skip the decoration */
            if (decorationData->auxiliaryQualifier == SPV_AUXILIARY_PATCH && decorationData->builtIn != SpvBuiltInTessCoord)
            {
                symSpv->perPatch = gcvTRUE;

                if (symSpv->spvStorage == SpvStorageClassInput)
                {
                    symSpv->virStorageClass = VIR_STORAGE_PERPATCH_INPUT;
                }
                else if (symSpv->spvStorage == SpvStorageClassOutput)
                {
                    symSpv->virStorageClass = VIR_STORAGE_PERPATCH_OUTPUT;
                }
            }
        }
        /* For a struct type, save the info into the field symbol. */
        else if (spv->opCode == SpvOpTypeStruct)
        {
            location = VIR_Symbol_GetLocation(symbol);
            gcmASSERT(VIR_Symbol_isField(symbol));

            /* Save the flag/layout qualifier/location into symbol. */
            VIR_Symbol_SetFlag(symbol, VIR_Symbol_GetFlag(symbol) | __SpvConvDecoratorToSymFlag(decorationData));
            VIR_Symbol_SetLayoutQualifier(symbol,
                VIR_Symbol_GetLayoutQualifier(symbol) | __SpvConvDecoratorToSymLayout(decorationData));
            VIR_Symbol_SetLocation(symbol, SPV_SET_VALID_VALUE(decorationData->location, location, -1));

            /* Save the offset/arrayStride/matrixStride into symbol field info. */
            fieldInfo = VIR_Symbol_GetFieldInfo(symbol);
            gcmASSERT(fieldInfo);

            VIR_FieldInfo_SetOffset(fieldInfo, SPV_SET_VALID_VALUE(decorationData->offset, VIR_FieldInfo_GetOffset(fieldInfo), -1));
            VIR_FieldInfo_SetArrayStride(fieldInfo, SPV_SET_VALID_VALUE(decorationData->arrayStride, VIR_FieldInfo_GetArrayStride(fieldInfo), -1));
            VIR_FieldInfo_SetMatrixStride(fieldInfo, SPV_SET_VALID_VALUE(decorationData->matrixStride, VIR_FieldInfo_GetMatrixStride(fieldInfo), -1));
        }
        else
        {
            gcmASSERT(0);
        }
    }

    return virErrCode;
}

typedef enum{
    SPV_INSERT_CURRENT,
    SPV_INSERT_AFTER,
    SPV_INSERT_BEFORE
}SpvInsertLocation;

static VSC_ErrCode __SpvInsertInstruction3(gcSPV spv, VIR_Shader * virShader, VIR_Function * virFunction,
                                           SpvInsertLocation loc, VIR_Instruction * inst,
                                           VIR_OpCode op, SpvId src, SpvId srcType,
                                           VIR_Symbol * dst, SpvId dstType)
{
    VIR_Swizzle virSwizzle;
    VIR_Enable virEnableMask;
    VIR_Instruction * virInst;
    VIR_Type *  dstVirType = gcvNULL;
    VIR_TypeId dstVirTypeId = VIR_TYPE_UNKNOWN;
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VIR_Function * func;
    VIR_ScalarConstVal constVal;

    dstVirType = SPV_ID_TYPE_VIR_TYPE(dstType);
    dstVirTypeId = SPV_ID_TYPE_VIR_TYPE_ID(dstType);

    virEnableMask = __SpvGenEnable(spv, dstVirType, dstType);

    if (virFunction != gcvNULL)
    {
        func = virFunction;
    }
    else
    {
        func = spv->virFunction;
    }

    if (loc == SPV_INSERT_BEFORE)
    {
        VIR_Function_AddInstructionBefore(func,
            op,
            dstVirTypeId,
            inst,
            gcvTRUE,
            &virInst);
    }
    else if (loc == SPV_INSERT_AFTER)
    {
        VIR_Function_AddInstructionAfter(func,
            op,
            dstVirTypeId,
            inst,
            gcvTRUE,
            &virInst);
    }
    else
    {
        VIR_Function_AddInstruction(func,
            op,
            dstVirTypeId,
            &virInst);
    }

    VIR_Inst_SetConditionOp(virInst, VIR_COP_ALWAYS);

    VIR_Operand_SetRoundMode(VIR_Inst_GetDest(virInst), VIR_ROUND_DEFAULT);
    VIR_Operand_SetModifier(VIR_Inst_GetDest(virInst), VIR_MOD_NONE);
    VIR_Operand_SetEnable(VIR_Inst_GetDest(virInst), virEnableMask);
    VIR_Operand_SetOpKind(VIR_Inst_GetDest(virInst), VIR_OPND_SYMBOL);
    VIR_Operand_SetTypeId(VIR_Inst_GetDest(virInst), dstVirTypeId);
    VIR_Operand_SetSym(VIR_Inst_GetDest(virInst), dst);

    virSwizzle = __SpvID2Swizzle(spv, src);

    constVal.uValue = SPV_CONST_SCALAR_UINT(src);
    VIR_Operand_SetImmediate(VIR_Inst_GetSource(virInst, 0), SPV_ID_VIR_TYPE_ID(srcType), constVal);
    VIR_Operand_SetSwizzle(VIR_Inst_GetSource(virInst, 0), virSwizzle);
    VIR_Operand_SetOpKind(VIR_Inst_GetSource(virInst, 0), VIR_OPND_IMMEDIATE);
    VIR_Operand_SetTypeId(VIR_Inst_GetSource(virInst, 0), SPV_ID_VIR_TYPE_ID(srcType));
    VIR_Operand_SetPrecision(VIR_Inst_GetSource(virInst, 0), VIR_PRECISION_HIGH);
    VIR_Operand_SetRoundMode(VIR_Inst_GetSource(virInst, 0), VIR_ROUND_DEFAULT);
    VIR_Operand_SetModifier(VIR_Inst_GetSource(virInst, 0), VIR_MOD_NONE);

    return virErrCode;
}

static VSC_ErrCode __SpvInsertInstruction2(gcSPV spv, VIR_Shader * virShader, VIR_Function * virFunction,
                                           SpvInsertLocation loc, VIR_Instruction * inst,
                                           VIR_OpCode op, VIR_Symbol * src, SpvId srcType,
                                           VIR_Symbol * dst, SpvId dstType)
{
    VIR_Swizzle virSwizzle;
    VIR_Enable virEnableMask;
    VIR_Instruction * virInst;
    VIR_Type *  dstVirType = gcvNULL;
    VIR_TypeId dstVirTypeId = VIR_TYPE_UNKNOWN;
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VIR_Function * func;

    dstVirType = SPV_ID_TYPE_VIR_TYPE(dstType);
    dstVirTypeId = SPV_ID_TYPE_VIR_TYPE_ID(dstType);

    virEnableMask = __SpvGenEnable(spv, dstVirType, dstType);

    if (virFunction != gcvNULL)
    {
        func = virFunction;
    }
    else
    {
        func = spv->virFunction;
    }

    if (loc == SPV_INSERT_BEFORE)
    {
        VIR_Function_AddInstructionBefore(func,
            op,
            dstVirTypeId,
            inst,
            gcvTRUE,
            &virInst);
    }
    else if (loc == SPV_INSERT_AFTER)
    {
        VIR_Function_AddInstructionAfter(func,
            op,
            dstVirTypeId,
            inst,
            gcvTRUE,
            &virInst);
    }
    else
    {
        VIR_Function_AddInstruction(func,
            op,
            dstVirTypeId,
            &virInst);
    }

    VIR_Inst_SetConditionOp(virInst, VIR_COP_ALWAYS);

    VIR_Operand_SetRoundMode(VIR_Inst_GetDest(virInst), VIR_ROUND_DEFAULT);
    VIR_Operand_SetModifier(VIR_Inst_GetDest(virInst), VIR_MOD_NONE);
    VIR_Operand_SetEnable(VIR_Inst_GetDest(virInst), virEnableMask);
    VIR_Operand_SetOpKind(VIR_Inst_GetDest(virInst), VIR_OPND_SYMBOL);
    VIR_Operand_SetTypeId(VIR_Inst_GetDest(virInst), dstVirTypeId);
    VIR_Operand_SetSym(VIR_Inst_GetDest(virInst), dst);

    virSwizzle = __SpvID2Swizzle(spv, srcType);

    VIR_Operand_SetSwizzle(VIR_Inst_GetSource(virInst, 0), virSwizzle);
    VIR_Operand_SetSym(VIR_Inst_GetSource(virInst, 0), src);
    VIR_Operand_SetOpKind(VIR_Inst_GetSource(virInst, 0), VIR_OPND_SYMBOL);
    VIR_Operand_SetTypeId(VIR_Inst_GetSource(virInst, 0), SPV_ID_TYPE_VIR_TYPE_ID(srcType));
    VIR_Operand_SetPrecision(VIR_Inst_GetSource(virInst, 0), VIR_PRECISION_HIGH);
    VIR_Operand_SetRoundMode(VIR_Inst_GetSource(virInst, 0), VIR_ROUND_DEFAULT);
    VIR_Operand_SetModifier(VIR_Inst_GetSource(virInst, 0), VIR_MOD_NONE);

    return virErrCode;
}


#if !SPV_NEW_FUNCPARAM
static VSC_ErrCode __SpvInsertInstruction(gcSPV spv, VIR_Shader * virShader, VIR_Function * virFunction,
                                          SpvInsertLocation loc, VIR_Instruction * inst,
                                          VIR_OpCode op, SpvId src, SpvId dst)
{
    VIR_Swizzle virSwizzle;
    VIR_Enable virEnableMask;
    VIR_Instruction * virInst;
    VIR_Symbol * dstVirSym = gcvNULL;
    VIR_Type *  dstVirType = gcvNULL;
    VIR_TypeId dstVirTypeId = VIR_TYPE_UNKNOWN;
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VIR_Function * virFunc;

    dstVirType =SPV_ID_VIR_TYPE(dst);
    dstVirTypeId = SPV_ID_VIR_TYPE_ID(dst);

    virEnableMask = __SpvGenEnable(spv, dstVirType, SPV_ID_SYM_SPV_TYPE(dst));

    if (virFunction != gcvNULL)
    {
        virFunc = virFunction;
    }
    else
    {
        virFunc = spv->virFunction;
    }

    if (loc == SPV_INSERT_BEFORE)
    {
        VIR_Function_AddInstructionBefore(virFunc,
            op,
            dstVirTypeId,
            inst,
            gcvTRUE,
            &virInst);
    }
    else if (loc == SPV_INSERT_AFTER)
    {
        VIR_Function_AddInstructionAfter(virFunc,
            op,
            dstVirTypeId,
            inst,
            gcvTRUE,
            &virInst);
    }
    else
    {
        VIR_Function_AddInstruction(virFunc,
            op,
            dstVirTypeId,
            &virInst);
    }

    dstVirSym = SPV_ID_VIR_CURRENT_FUNC_SYM(dst);

    VIR_Inst_SetConditionOp(virInst, VIR_COP_ALWAYS);

    VIR_Operand_SetRoundMode(VIR_Inst_GetDest(virInst), VIR_ROUND_DEFAULT);
    VIR_Operand_SetModifier(VIR_Inst_GetDest(virInst), VIR_MOD_NONE);
    VIR_Operand_SetEnable(VIR_Inst_GetDest(virInst), virEnableMask);
    VIR_Operand_SetOpKind(VIR_Inst_GetDest(virInst), VIR_OPND_SYMBOL);
    VIR_Operand_SetTypeId(VIR_Inst_GetDest(virInst), dstVirTypeId);
    VIR_Operand_SetSym(VIR_Inst_GetDest(virInst), dstVirSym);

    virSwizzle = __SpvID2Swizzle(spv, src);

    VIR_Operand_SetSwizzle(VIR_Inst_GetSource(virInst, 0), virSwizzle);
    VIR_Operand_SetSym(VIR_Inst_GetSource(virInst, 0), SPV_ID_VIR_SYM(src));
    VIR_Operand_SetOpKind(VIR_Inst_GetSource(virInst, 0), VIR_OPND_SYMBOL);
    VIR_Operand_SetTypeId(VIR_Inst_GetSource(virInst, 0), SPV_ID_VIR_TYPE_ID(src));
    VIR_Operand_SetPrecision(VIR_Inst_GetSource(virInst, 0), VIR_PRECISION_HIGH);
    VIR_Operand_SetRoundMode(VIR_Inst_GetSource(virInst, 0), VIR_ROUND_DEFAULT);
    VIR_Operand_SetModifier(VIR_Inst_GetSource(virInst, 0), VIR_MOD_NONE);

    return virErrCode;
}
#endif

static const gctUINT spvVecMagicBase[] =
{
    /* 0 - 15 */
    0, 1, 2, 3, 4, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0,
    /* 16 - 31 */
    6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 32 */
    7,
};

static const VIR_TypeId virVector[] =
{
    VIR_TYPE_UNKNOWN,
    VIR_TYPE_FLOAT32,
    VIR_TYPE_FLOAT_X2,
    VIR_TYPE_FLOAT_X3,
    VIR_TYPE_FLOAT_X4,
    VIR_TYPE_FLOAT_X8,
    VIR_TYPE_FLOAT_X16,
    VIR_TYPE_FLOAT_X32,
    VIR_TYPE_INT32,
    VIR_TYPE_INTEGER_X2,
    VIR_TYPE_INTEGER_X3,
    VIR_TYPE_INTEGER_X4,
    VIR_TYPE_INTEGER_X8,
    VIR_TYPE_INTEGER_X16,
    VIR_TYPE_INTEGER_X32,
    VIR_TYPE_INT8,
    VIR_TYPE_INT8_X2,
    VIR_TYPE_INT8_X3,
    VIR_TYPE_INT8_X4,
    VIR_TYPE_INT8_X8,
    VIR_TYPE_INT8_X16,
    VIR_TYPE_INT8_X32,
    VIR_TYPE_INT16,
    VIR_TYPE_INT16_X2,
    VIR_TYPE_INT16_X3,
    VIR_TYPE_INT16_X4,
    VIR_TYPE_INT16_X8,
    VIR_TYPE_INT16_X16,
    VIR_TYPE_INT16_X32,
    VIR_TYPE_UINT32,
    VIR_TYPE_UINT_X2,
    VIR_TYPE_UINT_X3,
    VIR_TYPE_UINT_X4,
    VIR_TYPE_UINT_X8,
    VIR_TYPE_UINT_X16,
    VIR_TYPE_UINT_X32,
    VIR_TYPE_UINT8,
    VIR_TYPE_UINT8_X2,
    VIR_TYPE_UINT8_X3,
    VIR_TYPE_UINT8_X4,
    VIR_TYPE_UINT8_X8,
    VIR_TYPE_UINT8_X16,
    VIR_TYPE_UINT8_X32,
    VIR_TYPE_UINT16,
    VIR_TYPE_UINT16_X2,
    VIR_TYPE_UINT16_X3,
    VIR_TYPE_UINT16_X4,
    VIR_TYPE_UINT16_X8,
    VIR_TYPE_UINT16_X16,
    VIR_TYPE_UINT16_X32,
    VIR_TYPE_BOOLEAN,
    VIR_TYPE_BOOLEAN_X2,
    VIR_TYPE_BOOLEAN_X3,
    VIR_TYPE_BOOLEAN_X4,
    VIR_TYPE_BOOLEAN_X8,
    VIR_TYPE_BOOLEAN_X16,
    VIR_TYPE_BOOLEAN_X32,
};

static const VIR_TypeId virMat[] =
{
    /* col = 2 */
    VIR_TYPE_FLOAT_2X2,
    VIR_TYPE_FLOAT_2X3,
    VIR_TYPE_FLOAT_2X4,
    /* col = 3 */
    VIR_TYPE_FLOAT_3X2,
    VIR_TYPE_FLOAT_3X3,
    VIR_TYPE_FLOAT_3X4,
    /* col = 4 */
    VIR_TYPE_FLOAT_4X2,
    VIR_TYPE_FLOAT_4X3,
    VIR_TYPE_FLOAT_4X4,
};

#define SPV_GEN_IMAGE_TYPE_MAGIC(dim, type, depth, arrayed, ms, sampled) \
    (dim | (type << 4) | (depth << 6) | (arrayed << 8) | (ms << 10) | (sampled << 12))

static VIR_TypeId __SpvImage2VirImageType(gcSPV spv, SpvId targetId, VIR_TypeId * SampledImageType)
{
    VIR_TypeId virType = VIR_TYPE_UNKNOWN;
    VIR_TypeId sampledImageType = VIR_TYPE_UNKNOWN;
    SpvImageFormat format;
    SpvDim dimension;
    gctUINT depth;
    gctUINT arrayed;
    gctUINT msaa;
    gctUINT sampled;
    gctUINT type = 0;
    gctUINT magic;
    gctUINT sampledType;

    if (!SPV_ID_TYPE_IS_IMAGE(targetId))
    {
        return virType;
    }

    format = SPV_ID_TYPE_IMAGE_FORMAT(targetId);
    dimension = SPV_ID_TYPE_IMAGE_DIM(targetId);
    depth = SPV_ID_TYPE_IMAGE_DEPTH(targetId);
    arrayed = SPV_ID_TYPE_IMAGE_ARRAY(targetId);
    msaa = SPV_ID_TYPE_IMAGE_MSAA(targetId);
    sampled = SPV_ID_TYPE_IMAGE_SAMPLED(targetId);

    sampledType = SPV_ID_TYPE_IMAGE_SAMPLED_TYPE(targetId);

    if (format != SpvImageFormatUnknown)
    {
        if (format <= SpvImageFormatR8Snorm)
        {
            type = 0;
        }
        else if (format <= SpvImageFormatR8i)
        {
            type = 1;
        }
        else if (format <= SpvImageFormatR8ui)
        {
            type = 2;
        }
    }
    else
    {
        if (SPV_ID_TYPE_IS_FLOAT(sampledType))
        {
            type = 0;
        }
        else if (SPV_ID_TYPE_IS_SIGNEDINTEGER(sampledType))
        {
            type = 1;
        }
        else if (SPV_ID_TYPE_IS_UNSIGNEDINTEGER(sampledType))
        {
            type = 2;
        }
    }

    /* magic = dimension | (type << 4) | (depth << 6) | (arrayed << 8) | (msaa << 10) | (sampled << 12); */
    magic = SPV_GEN_IMAGE_TYPE_MAGIC(dimension, type, depth, arrayed, msaa, sampled);

    switch (magic)
    {
        /* TODO: 1D has no difference between Integer and UInteger ? */
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim1D, 0, 0, 0, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim1D, 0, 0, 0, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim1D, 0, 0, 0, 0, 2):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim1D, 1, 0, 0, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim1D, 1, 0, 0, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim1D, 1, 0, 0, 0, 2):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim1D, 2, 0, 0, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim1D, 2, 0, 0, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim1D, 2, 0, 0, 0, 2):
        virType = VIR_TYPE_IMAGE_1D;
        sampledImageType = VIR_TYPE_SAMPLER_1D;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 0, 0, 0, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 0, 0, 0, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 0, 0, 0, 0, 2):
        virType = VIR_TYPE_IMAGE_2D;
        sampledImageType = VIR_TYPE_SAMPLER_2D;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim3D, 0, 0, 0, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim3D, 0, 0, 0, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim3D, 0, 0, 0, 0, 2):
        virType = VIR_TYPE_IMAGE_3D;
        sampledImageType = VIR_TYPE_SAMPLER_3D;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimCube, 0, 0, 0, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimCube, 0, 0, 0, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimCube, 0, 0, 0, 0, 2):
        virType = VIR_TYPE_IMAGE_CUBE;
        sampledImageType = VIR_TYPE_SAMPLER_CUBIC;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimCube, 0, 1, 0, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimCube, 0, 1, 0, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimCube, 0, 1, 0, 0, 2):
        virType = VIR_TYPE_IMAGE_CUBE_DEPTH;
        sampledImageType = VIR_TYPE_SAMPLER_CUBE_SHADOW;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimCube, 0, 1, 1, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimCube, 0, 1, 1, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimCube, 0, 1, 1, 0, 2):
        virType = VIR_TYPE_IMAGE_CUBE_DEPTH_ARRAY;
        sampledImageType = VIR_TYPE_SAMPLER_CUBE_ARRAY_SHADOW;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim1D, 0, 0, 1, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim1D, 0, 0, 1, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim1D, 0, 0, 1, 0, 2):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim1D, 1, 0, 1, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim1D, 1, 0, 1, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim1D, 1, 0, 1, 0, 2):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim1D, 2, 0, 1, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim1D, 2, 0, 1, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim1D, 2, 0, 1, 0, 2):
        virType = VIR_TYPE_IMAGE_1D_ARRAY;
        sampledImageType = VIR_TYPE_SAMPLER_1D_ARRAY;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim1D, 0, 1, 0, 0, 1):
        virType = VIR_TYPE_IMAGE_1D_DEPTH;
        sampledImageType = VIR_TYPE_SAMPLER_1D_SHADOW;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim1D, 0, 1, 1, 0, 1):
        virType = VIR_TYPE_IMAGE_1D_ARRAY_DEPTH;
        sampledImageType = VIR_TYPE_SAMPLER_1D_ARRAY_SHADOW;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 0, 0, 1, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 0, 0, 1, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 0, 0, 1, 0, 2):
        virType = VIR_TYPE_IMAGE_2D_ARRAY;
        sampledImageType = VIR_TYPE_SAMPLER_2D_ARRAY;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimCube, 0, 0, 1, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimCube, 0, 0, 1, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimCube, 0, 0, 1, 0, 2):
        virType = VIR_TYPE_IMAGE_CUBE_ARRAY;
        sampledImageType = VIR_TYPE_SAMPLER_CUBE_ARRAY;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 0, 0, 0, 1, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 0, 0, 0, 1, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 0, 0, 0, 1, 2):
        virType = VIR_TYPE_IMAGE_2D_MSSA;
        sampledImageType = VIR_TYPE_SAMPLER_2D_MS;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 1, 0, 0, 1, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 1, 0, 0, 1, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 1, 0, 0, 1, 2):
        virType = VIR_TYPE_IIMAGE_2D_MSSA;
        sampledImageType = VIR_TYPE_ISAMPLER_2D_MS;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 2, 0, 0, 1, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 2, 0, 0, 1, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 2, 0, 0, 1, 2):
        virType = VIR_TYPE_UIMAGE_2D_MSSA;
        sampledImageType = VIR_TYPE_USAMPLER_2D_MS;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 0, 0, 1, 1, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 0, 0, 1, 1, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 0, 0, 1, 1, 2):
        virType = VIR_TYPE_IMAGE_2D_ARRAY_MSSA;
        sampledImageType = VIR_TYPE_SAMPLER_2D_MS_ARRAY;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 1, 0, 1, 1, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 1, 0, 1, 1, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 1, 0, 1, 1, 2):
        virType = VIR_TYPE_IIMAGE_2D_ARRAY_MSSA;
        sampledImageType = VIR_TYPE_ISAMPLER_2D_MS_ARRAY;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 2, 0, 1, 1, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 2, 0, 1, 1, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 2, 0, 1, 1, 2):
        virType = VIR_TYPE_UIMAGE_2D_ARRAY_MSSA;
        sampledImageType = VIR_TYPE_USAMPLER_2D_MS_ARRAY;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 0, 1, 0, 1, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 0, 1, 0, 1, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 0, 1, 0, 1, 2):
        virType = VIR_TYPE_IMAGE_2D_MSSA_DEPTH;
        sampledImageType = VIR_TYPE_SAMPLER_2D_MS;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 0, 1, 1, 1, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 0, 1, 1, 1, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 0, 1, 1, 1, 2):
        virType = VIR_TYPE_IMAGE_2D_ARRAY_MSSA_DEPTH;
        sampledImageType = VIR_TYPE_SAMPLER_2D_MS_ARRAY;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 0, 1, 0, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 0, 1, 0, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 0, 1, 0, 0, 2):
        virType = VIR_TYPE_IMAGE_2D_DEPTH;
        sampledImageType = VIR_TYPE_SAMPLER_2D_SHADOW;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 0, 1, 1, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 0, 1, 1, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 0, 1, 1, 0, 2):
        virType = VIR_TYPE_IMAGE_2D_ARRAY_DEPTH;
        sampledImageType = VIR_TYPE_SAMPLER_2D_ARRAY_SHADOW;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 1, 0, 0, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 1, 0, 0, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 1, 0, 0, 0, 2):
        virType = VIR_TYPE_IIMAGE_2D;
        sampledImageType = VIR_TYPE_ISAMPLER_2D;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 2, 0, 0, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 2, 0, 0, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 2, 0, 0, 0, 2):
        virType = VIR_TYPE_UIMAGE_2D;
        sampledImageType = VIR_TYPE_USAMPLER_2D;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim3D, 1, 0, 0, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim3D, 1, 0, 0, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim3D, 1, 0, 0, 0, 2):
        virType = VIR_TYPE_IIMAGE_3D;
        sampledImageType = VIR_TYPE_ISAMPLER_3D;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim3D, 2, 0, 0, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim3D, 2, 0, 0, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim3D, 2, 0, 0, 0, 2):
        virType = VIR_TYPE_UIMAGE_3D;
        sampledImageType = VIR_TYPE_USAMPLER_3D;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 1, 0, 1, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 1, 0, 1, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 1, 0, 1, 0, 2):
        virType = VIR_TYPE_IIMAGE_2D_ARRAY;
        sampledImageType = VIR_TYPE_ISAMPLER_2D_ARRAY;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 2, 0, 1, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 2, 0, 1, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDim2D, 2, 0, 1, 0, 2):
        virType = VIR_TYPE_UIMAGE_2D_ARRAY;
        sampledImageType = VIR_TYPE_USAMPLER_2D_ARRAY;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimCube, 1, 0, 0, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimCube, 1, 0, 0, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimCube, 1, 0, 0, 0, 2):
        virType = VIR_TYPE_IIMAGE_CUBE;
        sampledImageType = VIR_TYPE_ISAMPLER_CUBIC;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimCube, 1, 0, 1, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimCube, 1, 0, 1, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimCube, 1, 0, 1, 0, 2):
        virType = VIR_TYPE_IIMAGE_CUBE_ARRAY;
        sampledImageType = VIR_TYPE_ISAMPLER_CUBE_ARRAY;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimCube, 2, 0, 0, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimCube, 2, 0, 0, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimCube, 2, 0, 0, 0, 2):
        virType = VIR_TYPE_UIMAGE_CUBE;
        sampledImageType = VIR_TYPE_USAMPLER_CUBIC;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimCube, 2, 0, 1, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimCube, 2, 0, 1, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimCube, 2, 0, 1, 0, 2):
        virType = VIR_TYPE_UIMAGE_CUBE_ARRAY;
        sampledImageType = VIR_TYPE_USAMPLER_CUBE_ARRAY;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimBuffer, 0, 0, 0, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimBuffer, 0, 0, 0, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimBuffer, 0, 0, 0, 0, 2):
        virType = VIR_TYPE_IMAGE_BUFFER;
        sampledImageType = VIR_TYPE_SAMPLER_BUFFER;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimBuffer, 1, 0, 0, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimBuffer, 1, 0, 0, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimBuffer, 1, 0, 0, 0, 2):
        virType = VIR_TYPE_IIMAGE_BUFFER;
        sampledImageType = VIR_TYPE_ISAMPLER_BUFFER;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimBuffer, 2, 0, 0, 0, 0):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimBuffer, 2, 0, 0, 0, 1):
    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimBuffer, 2, 0, 0, 0, 2):
        virType = VIR_TYPE_UIMAGE_BUFFER;
        sampledImageType = VIR_TYPE_USAMPLER_BUFFER;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimSubpassData, 0, 0, 0, 0, 2):
        virType = VIR_TYPE_SUBPASSINPUT;
        sampledImageType = VIR_TYPE_SAMPLER_2D;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimSubpassData, 0, 0, 0, 1, 2):
        virType = VIR_TYPE_SUBPASSINPUTMS;
        sampledImageType = VIR_TYPE_SAMPLER_2D_MS;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimSubpassData, 1, 0, 0, 0, 2):
        virType = VIR_TYPE_ISUBPASSINPUT;
        sampledImageType = VIR_TYPE_ISAMPLER_2D;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimSubpassData, 1, 0, 0, 1, 2):
        virType = VIR_TYPE_ISUBPASSINPUTMS;
        sampledImageType = VIR_TYPE_ISAMPLER_2D_MS;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimSubpassData, 2, 0, 0, 0, 2):
        virType = VIR_TYPE_USUBPASSINPUT;
        sampledImageType = VIR_TYPE_USAMPLER_2D;
        break;

    case SPV_GEN_IMAGE_TYPE_MAGIC(SpvDimSubpassData, 2, 0, 0, 1, 2):
        virType = VIR_TYPE_USUBPASSINPUTMS;
        sampledImageType = VIR_TYPE_USAMPLER_2D_MS;
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    if (SampledImageType)
    {
        *SampledImageType = sampledImageType;
    }

    return virType;
}

static VSC_ErrCode __SpvAddType(gcSPV spv, VIR_Shader * virShader)
{
    SpvCovDecorator *dec = spv->decorationList;
    VIR_TypeId virTypeId = VIR_TYPE_UNKNOWN;
    VIR_Type *virType;
    gctUINT nameId;
    gctUINT i;
    VIR_TypeId typeId;
    VIR_SymId symId;
    VIR_Symbol* sym;
    gctINT arrayStride = 0;
    gctUINT arrayLength = 0;
    gctUINT magicOffset = 0;
    VSC_ErrCode virErrCode = VSC_ERR_NONE;

    switch (spv->opCode)
    {
    case SpvOpTypeVoid:
        virTypeId = VIR_TYPE_VOID;
        SPV_ID_TYPE_IS_VOID(spv->resultId) = gcvTRUE;
        break;

    case SpvOpTypeBool:
        virTypeId = VIR_TYPE_BOOLEAN;
        SPV_ID_TYPE_IS_BOOLEAN(spv->resultId) = gcvTRUE;
        break;

    case SpvOpTypeInt:
        SPV_ID_TYPE_INT_WIDTH(spv->resultId) = spv->operands[0];
        SPV_ID_TYPE_INT_SIGN(spv->resultId) = spv->operands[1];

        if (SPV_ID_TYPE_INT_SIGN(spv->resultId) > 0)
        {
            virTypeId = __SpvIntToVirType(SPV_ID_TYPE_INT_WIDTH(spv->resultId), gcvTRUE);
            SPV_ID_TYPE_IS_SIGNEDINTEGER(spv->resultId) = gcvTRUE;
        }
        else
        {
            virTypeId = __SpvIntToVirType(SPV_ID_TYPE_INT_WIDTH(spv->resultId), gcvFALSE);
            SPV_ID_TYPE_IS_UNSIGNEDINTEGER(spv->resultId) = gcvTRUE;
        }

        SPV_ID_TYPE_IS_INTEGER(spv->resultId) = gcvTRUE;
        SPV_ID_TYPE_IS_SCALAR(spv->resultId) = gcvTRUE;
        break;

    case SpvOpTypeFloat:
        SPV_ID_TYPE_FLOAT_WIDTH(spv->resultId) = spv->operands[0];
        virTypeId = VIR_TYPE_FLOAT32;
        SPV_ID_TYPE_IS_FLOAT(spv->resultId) = gcvTRUE;
        SPV_ID_TYPE_IS_SCALAR(spv->resultId) = gcvTRUE;
        break;

    case SpvOpTypeVector:
        SPV_ID_TYPE_VEC_COMP_TYPE(spv->resultId) = spv->operands[0];
        SPV_ID_TYPE_VEC_COMP_NUM(spv->resultId) = spv->operands[1];

        magicOffset = __GetMagicOffsetByTypeId(SPV_ID_TYPE_VIR_TYPE_ID(SPV_ID_TYPE_VEC_COMP_TYPE(spv->resultId)));

        virTypeId = virVector[spvVecMagicBase[SPV_ID_TYPE_VEC_COMP_NUM(spv->resultId)] + magicOffset];

        SPV_ID_TYPE_IS_VECTOR(spv->resultId) = gcvTRUE;
        break;

    case SpvOpTypeMatrix:
        SPV_ID_TYPE_MAT_COL_TYPE(spv->resultId) = spv->operands[0];
        SPV_ID_TYPE_MAT_COL_COUNT(spv->resultId) = spv->operands[1];

        virTypeId = virMat[(SPV_ID_TYPE_MAT_COL_COUNT(spv->resultId) - 2 )* 3
                        + SPV_ID_TYPE_VEC_COMP_NUM(SPV_ID_TYPE_MAT_COL_TYPE(spv->resultId)) - 2];

        SPV_ID_TYPE_IS_MATRIX(spv->resultId) = gcvTRUE;
        break;

    case SpvOpTypeStruct:
        {
            VIR_SymIdList *fields;
            gctBOOL existed = gcvFALSE;
            gctUINT regOffset = 0;
            VIR_Type *structType = gcvFALSE;

            if (spv->resultId >= spv->idDescSize ||
                SPV_ID_VIR_NAME_ID(spv->resultId) == VIR_INVALID_ID)
            {
                __SpvGenerateVIRName(spv, spv->resultId);
                VIR_Shader_AddString(virShader, spv->virName, &nameId);
                SPV_SET_IDDESCRIPTOR_NAME(spv, spv->resultId, nameId);
                gcoOS_StrCopySafe(spv->tempName,
                                  gcoOS_StrLen(spv->virName, gcvNULL) + 1,
                                  spv->virName);
            }
            else
            {
                nameId = SPV_ID_VIR_NAME_ID(spv->resultId);
                gcoOS_StrCopySafe(spv->tempName,
                                  gcoOS_StrLen(VIR_Shader_GetStringFromId(virShader, nameId), gcvNULL) + 1,
                                  VIR_Shader_GetStringFromId(virShader, nameId));
            }

            VIR_Shader_AddStructType(virShader, gcvFALSE, nameId, gcvTRUE, &virTypeId);
            structType = VIR_Shader_GetTypeFromId(virShader, virTypeId);

            fields = VIR_Type_GetFields(structType);
            if (fields)
            {
                existed = gcvTRUE;
            }

            for (i = 0; i < spv->operandSize; i++)
            {
                VIR_Type * type;
                VIR_NameId fieldNameId;
                gctBOOL    isFieldUnSize = SPV_ID_TYPE_HAS_UNSIZEDARRAY(spv->operands[i]);
                gctBOOL    setSkipNameCheckFlag = gcvFALSE;

                __SpvReplaceBuiltInName(spv, virShader, VIR_STORAGE_UNKNOWN, spv->resultId, i);

                fieldNameId = SPV_ID_FIELD_VIR_NAME_ID(spv->resultId, i);
                SPV_ID_TYPE_STRUCT_MEMBER(spv->resultId, i) = spv->operands[i];
                type = SPV_ID_TYPE_VIR_TYPE(spv->operands[i]);
                typeId = VIR_Type_GetNameId(type);

                if (!existed)
                {
                    if (!SPV_ID_FIELD_HAS_NAME(spv->resultId, i))
                    {
                        __SpvGenerateStructFiledName(spv, spv->tempName, i);
                        VIR_Shader_AddString(virShader, spv->virName, &fieldNameId);
                        SPV_SET_IDDESCRIPTOR_FIELD_NAME(spv, spv->resultId, i, fieldNameId);
                        setSkipNameCheckFlag = gcvTRUE;
                    }

                    /* TO_DO, handle storage */
                    VIR_Shader_AddFieldSymbol(virShader,
                        fieldNameId,
                        SPV_ID_VIR_TYPE(spv->operands[i]),
                        structType,
                        VIR_STORAGE_UNKNOWN,
                        &symId);
                    sym = VIR_Shader_GetSymFromId(virShader, symId);

                    VIR_Type_AddField(virShader,
                        structType,
                        symId);

                    if (setSkipNameCheckFlag)
                    {
                        VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_SKIP_NAME_CHECK);
                    }

                    /* Offset/RowMajor/ColMajor/MatrixStride are saved by opMemberDecorate,
                    ** so the target ID should be the result ID.
                    */
                    __SpvConvDecoratorToVIR(spv,
                                            virShader,
                                            gcvNULL,
                                            spv->resultId,
                                            (gctINT)i,
                                            VIR_Shader_GetSymFromId(virShader, symId));

                    /* ArrayStride is saved by opDecorate,
                    ** so the target ID should be the operand ID.
                    */
                    __SpvConvDecoratorToVIR(spv,
                                            virShader,
                                            gcvNULL,
                                            spv->operands[i],
                                            -1,
                                            VIR_Shader_GetSymFromId(virShader, symId));
                    VIR_Symbol_GetFieldInfo(sym)->tempRegOrUniformOffset = regOffset;
                    regOffset += VIR_Type_GetVirRegCount(virShader, SPV_ID_VIR_TYPE(spv->operands[i]), -1);
                }

                if (isFieldUnSize)
                {
                    SPV_ID_TYPE_HAS_UNSIZEDARRAY(spv->resultId) = gcvTRUE;
                }
            }

            SPV_ID_TYPE_IS_STRUCT(spv->resultId) = gcvTRUE;
            break;
        }

    case SpvOpTypeSampler:
        virTypeId = VIR_TYPE_SAMPLER;
        SPV_ID_TYPE_IS_SAMPLER(spv->resultId) = gcvTRUE;
        /* TO_DO, handle the parameters of sampler */
        SPV_NEXT_INST;
        break;

    case SpvOpTypeArray:
    case SpvOpTypeRuntimeArray:
        {
            typeId = SPV_ID_VIR_TYPE_ID(spv->operands[0]);

            /* Get the array stride. */
            SPV_GET_DECORATOR(dec, (spv->resultId), -1);
            if (dec != gcvNULL)
            {
                arrayStride = dec->decorationData.arrayStride;
            }

            if (spv->opCode == SpvOpTypeRuntimeArray)
            {
                arrayLength = (gctUINT)-1;
            }
            else
            {
                arrayLength = SPV_ID_VIR_CONST(spv->operands[1]).scalarVal.uValue;
            }

            VIR_Shader_AddArrayType(virShader,
                typeId,
                arrayLength,
                arrayStride,
                &virTypeId);

            SPV_ID_TYPE_ARRAY_BASE_TYPE_ID(spv->resultId) = spv->operands[0];
            SPV_ID_TYPE_IS_ARRAY(spv->resultId) = gcvTRUE;

            if (spv->opCode == SpvOpTypeRuntimeArray)
            {
                virType = VIR_Shader_GetTypeFromId(virShader, virTypeId);
                VIR_Type_SetFlag(virType, VIR_TYFLAG_UNSIZED);
                SPV_ID_TYPE_HAS_UNSIZEDARRAY(spv->resultId) = gcvTRUE;
            }
            else
            {
                SPV_ID_TYPE_ARRAY_LENGTH(spv->resultId) = arrayLength;
            }

            if (SPV_ID_TYPE_IS_IMAGE(spv->operands[0]) &&
                SPV_ID_TYPE_IMAGE_SAMPLER_IMAGE_TYPE(spv->operands[0]) != VIR_TYPE_UNKNOWN)
            {
                VIR_TypeId samplerTypeId = SPV_ID_TYPE_IMAGE_SAMPLER_IMAGE_TYPE(spv->operands[0]);

                VIR_Shader_AddArrayType(virShader,
                                        samplerTypeId,
                                        arrayLength,
                                        arrayStride,
                                        &samplerTypeId);

                SPV_ID_TYPE_ARRAY_SAMPLER_IMAGE_TYPE(spv->resultId) = samplerTypeId;
            }
            else if (SPV_ID_TYPE_IS_ARRAY(spv->operands[0]) &&
                     SPV_ID_TYPE_ARRAY_SAMPLER_IMAGE_TYPE(spv->operands[0]) != VIR_TYPE_UNKNOWN)
            {
                VIR_TypeId samplerTypeId = SPV_ID_TYPE_ARRAY_SAMPLER_IMAGE_TYPE(spv->operands[0]);

                VIR_Shader_AddArrayType(virShader,
                                        samplerTypeId,
                                        arrayLength,
                                        arrayStride,
                                        &samplerTypeId);

                SPV_ID_TYPE_ARRAY_SAMPLER_IMAGE_TYPE(spv->resultId) = samplerTypeId;
            }

            break;
        }

    case SpvOpTypeOpaque:
        /*gcmASSERT(0);*/
        break;

    case SpvOpTypePointer:
        /* GLSL have no pointer, but SPV always pointer even GLSL, so record to VIR_Shader type too,
           but when recognize this type variable, we get pointer objType to set variable
        */
        virTypeId = VIR_TYPE_UINT32;
        SPV_ID_TYPE_POINTER_STORAGE_CLASS(spv->resultId) = (SpvStorageClass)spv->operands[0];
        SPV_ID_TYPE_POINTER_OBJECT_SPV_TYPE(spv->resultId) = spv->operands[1];
        SPV_ID_TYPE_IS_VECTOR(spv->resultId) = SPV_ID_TYPE_IS_VECTOR(spv->operands[1]);
        SPV_ID_TYPE_HAS_UNSIZEDARRAY(spv->resultId) = SPV_ID_TYPE_HAS_UNSIZEDARRAY(spv->operands[1]);
        SPV_ID_TYPE_IS_POINTER(spv->resultId) = gcvTRUE;
        break;

    case SpvOpTypeFunction:
        SPV_ID_TYPE_FUNC_RET_SPV_TYPE(spv->resultId) = spv->operands[0];
        SPV_ID_TYPE_IS_FUNCTION(spv->resultId) = gcvTRUE;

        if (spv->operandSize > 1)
        {
            spvAllocate(spv->spvMemPool, sizeof(gctUINT) * (spv->operandSize - 1), (gctPOINTER *)&SPV_ID_TYPE_FUNC_ARG_SPV_TYPE(spv->resultId));
            for(i = 1; i < spv->operandSize; i++)
            {
                if (SPV_ID_TYPE_IS_POINTER(spv->operands[i]))
                {
                    SPV_ID_TYPE_FUNC_ARG_SPV_TYPE(spv->resultId)[i - 1] = SPV_ID_TYPE_POINTER_OBJECT_SPV_TYPE(spv->operands[i]);
                }
                else
                {
                    SPV_ID_TYPE_FUNC_ARG_SPV_TYPE(spv->resultId)[i - 1] = spv->operands[i];
                }
            }
        }
        SPV_ID_TYPE_FUNC_ARG_NUM(spv->resultId) = spv->operandSize - 1;
        break;

    case SpvOpTypeImage:
        {
            VIR_TypeId sampledImageTypeId = VIR_TYPE_UNKNOWN;
            SPV_ID_TYPE_IMAGE_SAMPLED_TYPE(spv->resultId) = spv->operands[0];
            SPV_ID_TYPE_IMAGE_DIM(spv->resultId) = (SpvDim)spv->operands[1];
            SPV_ID_TYPE_IMAGE_DEPTH(spv->resultId) = spv->operands[2];
            SPV_ID_TYPE_IMAGE_ARRAY(spv->resultId) = spv->operands[3];
            SPV_ID_TYPE_IMAGE_MSAA(spv->resultId) = spv->operands[4];
            SPV_ID_TYPE_IMAGE_SAMPLED(spv->resultId) = spv->operands[5];
            SPV_ID_TYPE_IMAGE_FORMAT(spv->resultId) = (SpvImageFormat)spv->operands[6];
            if (spv->operandSize == 8)
            {
                SPV_ID_TYPE_IMAGE_ACCESS_QULIFIER(spv->resultId) = (SpvAccessQualifier)spv->operands[7];
            }
            SPV_ID_TYPE_IS_IMAGE(spv->resultId) = gcvTRUE;

            virTypeId = __SpvImage2VirImageType(spv, spv->resultId, &sampledImageTypeId);
            gcmASSERT(VIR_TypeId_isImage(virTypeId));
            gcmASSERT(VIR_TypeId_isSampler(sampledImageTypeId));

            SPV_ID_TYPE_IMAGE_SAMPLER_IMAGE_TYPE(spv->resultId) = sampledImageTypeId;

            /* for subpassData, sampled must be 2, image format must be unknown, model must be fragment */
            if (SPV_ID_TYPE_IMAGE_DIM(spv->resultId) == SpvDimSubpassData)
            {
                if (!((SPV_ID_TYPE_IMAGE_SAMPLED(spv->resultId) == 2) &&
                    (SPV_ID_TYPE_IMAGE_FORMAT(spv->resultId) == SpvImageFormatUnknown) &&
                    (spv->entryExeMode == SpvExecutionModelFragment)))
                {
                    virErrCode = VSC_ERR_INVALID_DATA;
                    ON_ERROR(virErrCode, "Invalid OpTypeImage operand");
                }

                /* set a default format for subpass data */
                SPV_ID_TYPE_IMAGE_FORMAT(spv->resultId) = SpvImageFormatRgba8;
            }

            break;
        }

    case SpvOpTypeSampledImage:
        /* This typs is an image, and also a sampler. */
        SPV_ID_TYPE_IS_IMAGE(spv->resultId) = gcvTRUE;
        SPV_ID_TYPE_IS_SAMPLER(spv->resultId) = gcvTRUE;
        /* Get the sampler type from type image. */
        virTypeId = SPV_ID_TYPE_IMAGE_SAMPLER_IMAGE_TYPE(spv->operands[0]);
        gcmASSERT(VIR_TypeId_isSampler(virTypeId));
        break;

    case SpvOpTypeEvent:
    case SpvOpTypeDeviceEvent:
    case SpvOpTypeReserveId:
    case SpvOpTypeQueue:
    case SpvOpTypePipe:
    case SpvOpTypeForwardPointer:
        gcmASSERT(0);

    default:
        gcmASSERT(0);
    };

    /* VIR add type */

    /* if it's composite type, we need add new type, if it's builtIn type, we need map to it's type ID */
    SPV_ID_TYPE_VIR_TYPE_ID(spv->resultId) = virTypeId;

    SPV_ID_TYPE(spv->resultId) = SPV_ID_TYPE_TYPE;

OnError:
    return virErrCode;
}

static VSC_ErrCode __SpvConstructWorkgroup(
    IN gcSPV spv,
    IN VIR_Shader * virShader
    )
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;

    /* pre-construction shader related things */
    if (spv->shaderStage == VSC_SHADER_STAGE_CS && SPV_HAS_WORKGROUP())
    {
        VIR_NameId          nameId;
        VIR_TypeId          virTypeId;
        VIR_SymId           symId;
        VIR_Symbol         *sym;
        VIR_StorageBlock   *sbo;

        spvAllocate(spv->spvMemPool, gcmSIZEOF(SpvWorkGroupInfo), (gctPOINTER *)(&spv->workgroupInfo));
        gcoOS_MemFill(spv->workgroupInfo, 0, gcmSIZEOF(SpvWorkGroupInfo));
        SPV_WORKGROUP_INFO()->curOffsetSymId = VIR_INVALID_ID;
        SPV_WORKGROUP_INFO()->groupOffsetSymId = VIR_INVALID_ID;

        VIR_Shader_AddString(virShader, "gl_WorkGroupIndex", &nameId);

        /* Create invocation_id */
        virErrCode = VIR_Shader_AddSymbol(
            virShader,
            VIR_SYM_VARIABLE,
            nameId,
            VIR_Shader_GetTypeFromId(virShader, VIR_TYPE_UINT32),
            VIR_STORAGE_INPUT,
            &symId);
        SPV_WORKGROUP_INFO()->invocationIdSymId = symId;
        sym = VIR_Shader_GetSymFromId(virShader, symId);
        VIR_Symbol_SetPrecision(sym, VIR_PRECISION_HIGH);
        VIR_Symbol_SetTyQualifier(sym, VIR_TYQUAL_NONE);
        VIR_Symbol_SetLayoutQualifier(sym, VIR_LAYQUAL_NONE);
        VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_WITHOUT_REG);
        VIR_Symbol_SetLocation(sym, -1);

        /* create a shared ssb for compute shader */
        virErrCode = VIR_Shader_AddString(virShader, "#sh_spv_shared_ssbo", &nameId);
        if (virErrCode != VSC_ERR_NONE) return virErrCode;

        virErrCode = VIR_Shader_AddStructType(virShader, gcvFALSE, nameId, gcvFALSE, &virTypeId);
        if (virErrCode != VSC_ERR_NONE) return virErrCode;

        virErrCode = VIR_Shader_AddSymbol(
            virShader,
            VIR_SYM_SBO,
            nameId,
            VIR_Shader_GetTypeFromId(virShader, virTypeId),
            VIR_STORAGE_GLOBAL,
            &symId);
        if (virErrCode != VSC_ERR_NONE) return virErrCode;

        sym = VIR_Shader_GetSymFromId(virShader, symId);
        VIR_Symbol_SetPrecision(sym, VIR_PRECISION_HIGH);
        VIR_Symbol_SetTyQualifier(sym, VIR_TYQUAL_NONE);
        VIR_Symbol_SetLayoutQualifier(sym, VIR_LAYQUAL_NONE);
        VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_WITHOUT_REG | VIR_SYMFLAG_COMPILER_GEN);
        VIR_Symbol_SetLocation(sym, -1);

        SPV_WORKGROUP_INFO()->sharedSboTypeId = virTypeId;
        SPV_WORKGROUP_INFO()->sharedSboSymId = symId;
        SPV_WORKGROUP_INFO()->sharedSboNameId = nameId;

        /* set SBO data. */
        sbo = sym->u2.sbo;
        gcmASSERT(sbo);
        sbo->sym = symId;
        VIR_SBO_SetFlag(sbo, VIR_IB_FOR_SHARED_VARIABLE);
    }

    return virErrCode;
}

static gctBOOL __SpvIsVariableInModel(gcSPV spv, VIR_Shader * virShader, SpvStorageClass storage)
{
    /* default we assume it is in current model */
    gctBOOL inModel = gcvTRUE;
    SpvId resultId = spv->resultId;

    if (spv->isMultiEntry &&
        (storage == SpvStorageClassInput || storage == SpvStorageClassOutput))
    {
        switch (spv->entryExeMode)
        {
        case SpvExecutionModelVertex:                   inModel = SPV_ID_INTERFACE_FLAG(resultId).inVert; break;
        case SpvExecutionModelTessellationControl:      inModel = SPV_ID_INTERFACE_FLAG(resultId).inTesc; break;
        case SpvExecutionModelTessellationEvaluation:   inModel = SPV_ID_INTERFACE_FLAG(resultId).inTese; break;
        case SpvExecutionModelGeometry:                 inModel = SPV_ID_INTERFACE_FLAG(resultId).inGeom; break;
        case SpvExecutionModelFragment:                 inModel = SPV_ID_INTERFACE_FLAG(resultId).inFrag; break;
        case SpvExecutionModelGLCompute:                inModel = SPV_ID_INTERFACE_FLAG(resultId).inComp; break;
        case SpvExecutionModelKernel:                   inModel = SPV_ID_INTERFACE_FLAG(resultId).inKernel; break;
        default: break;
        }
    }

    return inModel;
}

/* The OpName/OpDecorate */
static VSC_ErrCode __SpvAddVariable(gcSPV spv, VIR_Shader * virShader)
{
    /* All source level variable add here, we need collect enough info about,
       symbolKind, nameId, TypeId, storageClass */
    SpvVIRSymbolInternal symSpv;
    VIR_Symbol * sym = gcvNULL;
    SpvStorageClass storage = (SpvStorageClass)spv->operands[0];
    SpvId baseTypeId = spv->resultTypeId;
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    gctBOOL isArray = gcvFALSE;
    gctBOOL isBlock = gcvFALSE;

    if (!__SpvIsVariableInModel(spv, virShader, storage))
    {
        return VSC_ERR_NONE;
    }

    /* Get the non-pointer type ID. */
    while (SPV_ID_TYPE_IS_POINTER(baseTypeId))
    {
        baseTypeId = SPV_ID_TYPE_POINTER_OBJECT_SPV_TYPE(baseTypeId);
    }

    /* Get base type if it is array */
    if (SPV_ID_TYPE_IS_ARRAY(baseTypeId))
    {
        isArray = gcvTRUE;
        baseTypeId = SPV_ID_TYPE_ARRAY_BASE_TYPE_ID(baseTypeId);
    }

    if ((storage == SpvStorageClassUniform ||
        storage == SpvStorageClassUniformConstant ||
        storage == SpvStorageClassInput ||
        storage == SpvStorageClassOutput) &&
        SPV_ID_TYPE_IS_STRUCT(baseTypeId) &&
        SPV_ID_TYPE_IS_BLOCK(baseTypeId))
    {
        isBlock = gcvTRUE;
    }

    /* Initialize SpvVIRSymbolInternal */
    SYMSPV_Initialize(&symSpv);
    symSpv.spvStorage = storage;
    SPV_ID_SYM_STORAGE_CLASS(spv->resultId) = storage;

    switch(storage)
    {
    case SpvStorageClassUniformConstant:
    case SpvStorageClassPushConstant:
        SPV_ID_INITIALIZED(spv->resultId) = gcvTRUE;
        if (SPV_ID_TYPE_IS_SAMPLER(baseTypeId))
        {
            symSpv.virSymbolKind = VIR_SYM_SAMPLER;
        }
        else if (SPV_ID_TYPE_IS_IMAGE(baseTypeId))
        {
            symSpv.virSymbolKind = VIR_SYM_IMAGE;
        }
        else
        {
            symSpv.virSymbolKind = VIR_SYM_UNIFORM;
        }
        symSpv.virStorageClass = VIR_STORAGE_GLOBAL;
        break;

    case SpvStorageClassUniform:
        SPV_ID_INITIALIZED(spv->resultId) = gcvTRUE;
        symSpv.virSymbolKind = VIR_SYM_UBO;
        symSpv.virStorageClass = VIR_STORAGE_GLOBAL;
        break;

    case SpvStorageClassInput:
        SPV_ID_INITIALIZED(spv->resultId) = gcvTRUE;
        symSpv.virSymbolKind = VIR_SYM_VARIABLE;
        symSpv.virStorageClass = VIR_STORAGE_INPUT;
        break;

    case SpvStorageClassOutput:
        symSpv.virSymbolKind = VIR_SYM_VARIABLE;
        symSpv.virStorageClass = VIR_STORAGE_OUTPUT;
        break;

    case SpvStorageClassFunction:
        symSpv.virSymbolKind = VIR_SYM_VARIABLE;
        symSpv.virStorageClass = VIR_STORAGE_LOCAL;
        break;

    case SpvStorageClassAtomicCounter:
        SPV_ID_INITIALIZED(spv->resultId) = gcvTRUE;
        symSpv.virSymbolKind = VIR_SYM_UNIFORM;
        symSpv.virStorageClass = VIR_STORAGE_GLOBAL;
        symSpv.virSymFlag |= VIR_SYMUNIFORMFLAG_ATOMIC_COUNTER;
        break;

    case SpvStorageClassImage:
        SPV_ID_INITIALIZED(spv->resultId) = gcvTRUE;
        symSpv.virSymbolKind = VIR_SYM_IMAGE;
        symSpv.virStorageClass = VIR_STORAGE_GLOBAL;
        break;

    default:
        symSpv.virSymbolKind = VIR_SYM_VARIABLE;
        symSpv.virStorageClass = VIR_STORAGE_GLOBAL;
    }

    /* decorator and storage need these symSpv info, so put at last.
        if the base type of current variable has decoration, we
        need handle it too.
    */
    __SpvConvDecoratorToVIR(spv, virShader, &symSpv, spv->resultId, -1, gcvNULL);
    __SpvConvDecoratorToVIR(spv, virShader, &symSpv, baseTypeId, -1, gcvNULL);

    /* we only check non-member decoration here, member decoration already checked in type struct */
    __SpvReplaceBuiltInName(spv, virShader, symSpv.virStorageClass, spv->resultId, -1);

    if ((spv->shaderStage == VSC_SHADER_STAGE_HS) ||
        (spv->shaderStage == VSC_SHADER_STAGE_DS) ||
        (spv->shaderStage == VSC_SHADER_STAGE_GS))
    {
        if(symSpv.spvStorage == SpvStorageClassInput)
        {
            if (isArray && !symSpv.perPatch)
            {
                symSpv.virSymFlag |= VIR_SYMFLAG_ARRAYED_PER_VERTEX;
                SPV_ID_SYM_PER_VERTEX(spv->resultId) = gcvTRUE;;
            }
        }

        if (symSpv.perPatch)
        {
            SPV_ID_SYM_PER_PATCH(spv->resultId) = symSpv.perPatch;

            /* in/out/inout???? which one. FE not clear!!!! */
            if (symSpv.spvStorage == SpvStorageClassInput)
            {
                symSpv.virStorageClass = VIR_STORAGE_PERPATCH_INPUT;
            }
            else if (symSpv.spvStorage == SpvStorageClassOutput)
            {
                symSpv.virStorageClass = VIR_STORAGE_PERPATCH_OUTPUT;
            }
        }
    }

    if (spv->shaderStage == VSC_SHADER_STAGE_HS)
    {
        if (symSpv.spvStorage == SpvStorageClassOutput)
        {
            if (isArray && !symSpv.perPatch)
            {
                symSpv.virSymFlag |= VIR_SYMFLAG_ARRAYED_PER_VERTEX;
                SPV_ID_SYM_PER_VERTEX(spv->resultId) = gcvTRUE;;
            }
        }
    }

    if (!symSpv.noUse)
    {
        if ((storage == SpvStorageClassUniform &&
            SPV_ID_TYPE_IS_POINTER(spv->resultTypeId)) ||
            (isBlock))
        {
            /* uniform block array, we need split it into seperate uniform block */
            VIR_Type * virType = SPV_ID_VIR_TYPE(SPV_ID_TYPE_POINTER_OBJECT_SPV_TYPE(spv->resultTypeId));
            gctUINT uboArraySize = SPV_INVALID_LABEL;

            if (SPV_ID_TYPE_IS_ARRAY(SPV_ID_TYPE_POINTER_OBJECT_SPV_TYPE(spv->resultTypeId)))
            {
                uboArraySize = virType->u2.arrayLength;
            }

            __SpvAddInterfaceBlockSymbol(
                spv,
                virShader,
                spv->resultId,
                spv->resultTypeId,
                uboArraySize,
                &symSpv);
        }
        else if(spv->shaderStage == VSC_SHADER_STAGE_CS &&
            storage == SpvStorageClassWorkgroup)
        {
            /* met storage work group, set work group activate */
            SPV_HAS_WORKGROUP() = gcvTRUE;

            if (SPV_WORKGROUP_INFO() == gcvNULL)
            {
                __SpvConstructWorkgroup(spv, virShader);
            }

            __SpvAddSharedSymbol(spv, virShader, spv->resultId, spv->resultTypeId);
        }
        else
        {
            __SpvAddIdSymbol(spv, virShader, gcvNULL, spv->resultId, spv->resultTypeId, symSpv.virSymbolKind, symSpv.virStorageClass, symSpv.compilerGen);
        }

        sym = SPV_ID_VIR_SYM(spv->resultId);

        __SpvFillVirSymWithSymSpv(spv, sym, virShader, &symSpv);
    }
    else
    {
        sym = SPV_ID_VIR_SYM(spv->resultId);
    }

    /* There is initializer existed, generate instructions to initialize this variable. */
    if (spv->operandSize == 2)
    {
        SpvId               initId = spv->operands[1];
        SpvId               baseTypeId = spv->resultTypeId;
        VIR_ConstId         constId = VIR_INVALID_ID;
        VIR_Instruction    *virInst = gcvNULL;
        VIR_Symbol         *initSymbol = gcvNULL;
        VIR_Operand        *operand = gcvNULL;
        VIR_Enable          virEnable;
        VIR_Swizzle         virSwizzle;
        VIR_OpCode          virOpcode = VIR_OP_MOV;
        VIR_TypeId          virTypeId = SPV_ID_VIR_TYPE_ID(spv->resultId);

        while (SPV_ID_TYPE_IS_POINTER(baseTypeId))
        {
            baseTypeId = SPV_ID_TYPE_POINTER_OBJECT_SPV_TYPE(baseTypeId);
        }

        virEnable = __SpvGenEnable(spv, VIR_Shader_GetTypeFromId(virShader, virTypeId), baseTypeId);
        virSwizzle = __SpvID2Swizzle(spv, initId);

        /* If this variable is a scalar/vector, then the initializer is a constant. */
        if (SPV_ID_TYPE_IS_SCALAR(baseTypeId)    ||
            SPV_ID_TYPE_IS_BOOLEAN(baseTypeId)   ||
            SPV_ID_TYPE_IS_VECTOR(baseTypeId))
        {
            gcmASSERT(SPV_ID_TYPE(initId) == SPV_ID_TYPE_CONST);
            constId = SPV_ID_VIR_CONST_ID(initId);
        }
        /* The initialize is a variable. */
        else
        {
            initSymbol = SPV_ID_VIR_STD_SYM(initId);
        }

        /* Insert a MOV instruction. */
        virErrCode = VIR_Function_AddInstruction(spv->virFunction,
                                                 virOpcode,
                                                 virTypeId,
                                                 &virInst);
        ON_ERROR(virErrCode, "VIR_Function_AddInstruction");

        /* Set DEST. */
        operand = VIR_Inst_GetDest(virInst);
        VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
        VIR_Operand_SetEnable(operand, virEnable);
        VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
        VIR_Operand_SetTypeId(operand, virTypeId);
        VIR_Operand_SetSym(operand, sym);

        /* Set SOURCE0. */
        operand = VIR_Inst_GetSource(virInst, 0);
        if (constId != VIR_INVALID_ID)
        {
            VIR_Operand_SetSwizzle(operand, virSwizzle);
            VIR_Operand_SetConstId(operand, constId);
            VIR_Operand_SetOpKind(operand, VIR_OPND_CONST);
            VIR_Operand_SetTypeId(operand, virTypeId);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
        }
        else
        {
            VIR_Operand_SetSwizzle(operand, virSwizzle);
            VIR_Operand_SetSym(operand, initSymbol);
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetTypeId(operand, virTypeId);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
        }

        /* if this is parameter, record */
        SPV_ID_SYM_USED_STORE_AS_DEST(spv->resultId) = gcvTRUE;
    }

OnError:
    return virErrCode;
}

static VSC_ErrCode __SpvAddComplexTypeConstant(gcSPV spv, VIR_Shader * virShader, gctBOOL IsConstantNull)
{
    VSC_ErrCode         virErrCode = VSC_ERR_NONE;
    VIR_SymId           virSymId;
    VIR_SymId          *compositeSymId = gcvNULL;
    VIR_SymbolKind     *compositeSymKind = gcvNULL;
    gctUINT             i, constantCount = spv->operandSize;

    /* Add a symbol for this resultID. */
    __SpvAddIdSymbol(spv,
                     virShader,
                     gcvNULL,
                     spv->resultId,
                     spv->resultTypeId,
                     VIR_SYM_VARIABLE,
                     VIR_STORAGE_GLOBAL,
                     gcvFALSE);

    virSymId = SPV_ID_VIR_SYM_ID(spv->resultId);

    gcmASSERT(constantCount > 0 || IsConstantNull);

    if (constantCount > 0)
    {
        /* Initialize sym list. */
        spvAllocate(spv->spvMemPool, constantCount * gcmSIZEOF(VIR_SymId), (gctPOINTER *)&compositeSymId);
        gcoOS_ZeroMemory(compositeSymId, constantCount * gcmSIZEOF(VIR_SymId));

        spvAllocate(spv->spvMemPool, constantCount * gcmSIZEOF(VIR_SymbolKind), (gctPOINTER *)&compositeSymKind);
        gcoOS_ZeroMemory(compositeSymKind, constantCount * gcmSIZEOF(VIR_SymbolKind));

        /* Get composite source sym id. */
        for (i = 0; i < constantCount; i++)
        {
            if (IsConstantNull)
            {
                compositeSymKind[i] = VIR_SYM_CONST;
                compositeSymId[i] = SPV_ID_VIR_CONST_ID(spv->operands[i]);
                break;
            }

            if (SPV_ID_TYPE(spv->operands[i]) == SPV_ID_TYPE_CONST)
            {
                compositeSymKind[i] = VIR_SYM_CONST;
                compositeSymId[i] = SPV_ID_VIR_CONST_ID(spv->operands[i]);
            }
            else
            {
                compositeSymKind[i] = VIR_SYM_VARIABLE;
                compositeSymId[i] = SPV_ID_VIR_SYM_ID(spv->operands[i]);
            }
        }
    }

    VIR_Shader_CompositeConstruct(virShader,
                                  spv->virFunction,
                                  gcvNULL,
                                  virSymId,
                                  SPV_ID_TYPE_VIR_TYPE_ID(spv->resultTypeId),
                                  IsConstantNull,
                                  compositeSymId,
                                  compositeSymKind,
                                  spv->operandSize);

    if (constantCount > 0)
    {
        spvFree(gcvNULL, compositeSymId);
        spvFree(gcvNULL, compositeSymKind);
    }

    return virErrCode;
}

static gctBOOL __SpvFindSpecConstantValue(IN gcSPV spv, IN gctUINT specId, OUT gctUINT *entryIndex)
{
    gctUINT i;
    VkSpecializationMapEntry * mapEntry;
    VkSpecializationInfo * specInfo;

    if (spv->specInfo == gcvNULL)
    {
        return gcvFALSE;
    }

    specInfo = spv->specInfo;

    for (i = 0; i < specInfo->mapEntryCount; i++)
    {
        mapEntry = (VkSpecializationMapEntry *)(&specInfo->pMapEntries[i]);
        if (specId == mapEntry->constantID)
        {
            if (entryIndex)
            {
                *entryIndex = i;
            }
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

static VSC_ErrCode __SpvAddUndef(gcSPV spv, VIR_Shader * virShader)
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;

    /* Use constant null to initialize this variable. */
    __SpvAddComplexTypeConstant(spv, virShader, gcvTRUE);

    return virErrCode;
}

static VSC_ErrCode __SpvFoldingSpecConstantOp(gcSPV spv, VIR_Shader * virShader)
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    SpvId resultTypeId = spv->resultTypeId;
    SpvOp opCode = spv->opCode;

    gcmASSERT(SPV_ID_TYPE(resultTypeId) == SPV_ID_TYPE_TYPE);

    if (SPV_ID_TYPE_IS_INTEGER(resultTypeId) && opCode != SpvOpCompositeExtract)
    {
        int leftOperand = SPV_ID_VIR_CONST(spv->operands[0]).scalarVal.iValue;
        int rightOperand = SPV_ID_VIR_CONST(spv->operands[1]).scalarVal.iValue;
        int retValue = 0;

        switch (opCode)
        {
        case SpvOpIAdd:     retValue = leftOperand + rightOperand; break;
        case SpvOpISub:     retValue = leftOperand - rightOperand; break;
        case SpvOpIMul:     retValue = leftOperand * rightOperand; break;
        default: gcmASSERT(gcvFALSE); break;
        }

        spv->operands[0] = retValue;
        spv->operandSize = 1;
    }
    else if (SPV_ID_TYPE_IS_FLOAT(resultTypeId) && opCode != SpvOpCompositeExtract)
    {
        float leftOperand = SPV_ID_VIR_CONST(spv->operands[0]).scalarVal.fValue;
        float rightOperand = SPV_ID_VIR_CONST(spv->operands[1]).scalarVal.fValue;
        float retValue = 0.0f;

        switch (opCode)
        {
        case SpvOpFAdd:     retValue = leftOperand + rightOperand; break;
        case SpvOpFSub:     retValue = leftOperand - rightOperand; break;
        case SpvOpFMul:     retValue = leftOperand * rightOperand; break;
        default: gcmASSERT(gcvFALSE); break;
        }

        spv->operands[0] = *((int *)(&retValue));
        spv->operandSize = 1;
    }
    else if ((SPV_ID_TYPE_IS_VECTOR(resultTypeId)) ||
        (SPV_ID_TYPE_IS_VECTOR(SPV_ID_CST_SPV_TYPE(spv->operands[0])) && opCode == SpvOpCompositeExtract))
    {
        gctUINT compNum = SPV_ID_TYPE_VEC_COMP_NUM(resultTypeId);
        gctUINT i, j;
        gctUINT maxVecNum = 4;
        int leftOperand[4] = { 0 };
        int rightOperand[4] = { 0 };
        int retValue[4] = { 0 };

        gcmASSERT(compNum <= maxVecNum);

        for (i = 0; i < maxVecNum; i++)
        {
            /* vector should use the id descriptor as component, rather than immediate number */
            leftOperand[i] = SPV_ID_CST_VEC_SPVID(spv->operands[0], i);
            rightOperand[i] = SPV_ID_CST_VEC_SPVID(spv->operands[1], i);
        }

#define SPV_ITERATOR_COMPONENT for (i = 0; i < compNum; i++)

        switch (opCode)
        {
        case SpvOpIAdd:
        case SpvOpISub:
        case SpvOpIMul:
            gcmASSERT(gcvFALSE); /* TODO: handle vector, need get value from spv id, and get result, map to spvid */
            break;

        case SpvOpCompositeInsert:
            SPV_ITERATOR_COMPONENT retValue[i] = rightOperand[i];
            retValue[spv->operands[2]] = spv->operands[0];
            break;

        case SpvOpCompositeExtract:
            retValue[0] = SPV_ID_VIR_CONST(leftOperand[spv->operands[1]]).scalarVal.iValue;
            compNum = 1;
            break;

        case SpvOpVectorShuffle:
            for (i = 0, j = 0; i < spv->operandSize - 2; i++)
            {
                gctUINT compNumLeft = SPV_ID_TYPE_VEC_COMP_NUM(SPV_ID_CST_SPV_TYPE(spv->operands[0]));
                gctUINT curIndex = spv->operands[i + 2];
                retValue[j++] = (curIndex < compNumLeft) ? (leftOperand[curIndex]) : (rightOperand[curIndex - compNumLeft]);
            }
            break;

        default: gcmASSERT(gcvFALSE); break;
        }

        SPV_ITERATOR_COMPONENT spv->operands[i] = retValue[i];
        spv->operandSize = compNum;
    }
    else
    {
        /* the constant op only have scalar or vector now */
        gcmASSERT(gcvFALSE);
    }

    spv->opCode = SpvOpConstant;
    __SpvAddConstant(spv, virShader);

    return virErrCode;
}

static VSC_ErrCode __SpvEmitSpecConstantOp(gcSPV spv, VIR_Shader * virShader)
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    gctUINT i;

    gcmASSERT(spv->opCode == SpvOpSpecConstantOp);

    /* Get real constant op */
    spv->opCode = spv->operands[0];

    /* copy back the operands */
    for (i = 0; i < spv->operandSize - 1; i++)
    {
        spv->operands[i] = spv->operands[i + 1];
    }

    spv->operandSize -= 1;

    /* TODO: need to unfold all the operations,
             instead of treating them as non-constant operations. */
    switch (spv->opCode)
    {
    case SpvOpIAdd:
        __SpvFoldingSpecConstantOp(spv, virShader);
        break;
    case SpvOpISub:
    case SpvOpIMul:
    case SpvOpFAdd:
    case SpvOpFSub:
    case SpvOpFMul:
    case SpvOpSNegate:
    case SpvOpNot:
    case SpvOpUDiv:
    case SpvOpSDiv:
    case SpvOpUMod:
    case SpvOpSMod:
    case SpvOpSRem:
    case SpvOpShiftRightLogical:
    case SpvOpShiftRightArithmetic:
    case SpvOpShiftLeftLogical:
    case SpvOpBitwiseOr:
    case SpvOpBitwiseXor:
    case SpvOpBitwiseAnd:
    case SpvOpLogicalOr:
    case SpvOpLogicalAnd:
    case SpvOpLogicalNot:
    case SpvOpLogicalEqual:
    case SpvOpLogicalNotEqual:
    case SpvOpSelect:
    case SpvOpIEqual:
    case SpvOpULessThan:
    case SpvOpSLessThan:
    case SpvOpUGreaterThan:
    case SpvOpSGreaterThan:
    case SpvOpULessThanEqual:
    case SpvOpSLessThanEqual:
    case SpvOpUGreaterThanEqual:
    case SpvOpSGreaterThanEqual:
    case SpvOpConvertFToS:
    case SpvOpConvertSToF:
    case SpvOpConvertFToU:
    case SpvOpConvertUToF:
    case SpvOpBitcast:
    case SpvOpFNegate:
    case SpvOpFDiv:
    case SpvOpFRem:
    case SpvOpFMod:
        __SpvEmitInstructions(spv, virShader);
        break;

    case SpvOpVectorShuffle:
        __SpvEmitVectorShuffle(spv, virShader);
        break;

    case SpvOpCompositeExtract:
        __SpvEmitCompositeExtract(spv, virShader);
        break;

    case SpvOpCompositeInsert:
        __SpvEmitCompositeInsert(spv, virShader);
        break;

    case SpvOpQuantizeToF16:
        __SpvEmitIntrisicCall(spv, virShader);
        break;

    case SpvOpAccessChain:
    case SpvOpInBoundsAccessChain:
        __SpvEmitAccessChain(spv, virShader);
        break;

    case SpvOpSConvert:
    case SpvOpFConvert:
    case SpvOpUConvert:
    case SpvOpConvertPtrToU:
    case SpvOpConvertUToPtr:
    case SpvOpGenericCastToPtr:
    case SpvOpPtrCastToGeneric:
    case SpvOpPtrAccessChain:
    case SpvOpInBoundsPtrAccessChain:
    default:
        gcmASSERT(gcvFALSE); /* not handle */
        break;
    }

    return virErrCode;
}

static VSC_ErrCode __SpvAddConstant(gcSPV spv, VIR_Shader * virShader)
{
    VIR_Type * type = gcvNULL;
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VIR_SymId cstSymId = VIR_INVALID_ID;
    gctUINT cstId = VIR_INVALID_ID;
    gctINT memberIndex = -1; /* assume constant is not a member */
    SpvCovDecorator * dec = spv->decorationList;
    SpvConvDecorationData * decData;
    SpvId targetId = spv->resultId;
    gctUINT specId;
    gctUINT constValue = spv->operands[0];
    gctUINT specEntryIndex = 0;
    gctBOOL findSpecData = gcvFALSE;
    VkSpecializationMapEntry * mapEntry = gcvNULL;
    gctUINT8 *specData = spv->specInfo ? (gctUINT8 *)spv->specInfo->pData : gcvNULL;
    gctBOOL addSym = gcvFALSE;

    SPV_GET_DECORATOR(dec, targetId, memberIndex);

    if (dec && SPV_IS_SPECCONST_OP(spv->opCode))
    {
        decData = &dec->decorationData;
        specId = decData->specId;

        /* Find the value of specialization id */
        findSpecData = __SpvFindSpecConstantValue(spv, specId, &specEntryIndex);
        if (findSpecData)
        {
            mapEntry = (VkSpecializationMapEntry *)(&spv->specInfo->pMapEntries[specEntryIndex]);
            specData = specData + mapEntry->offset;
        }
    }

    if (SPV_ID_TYPE_IS_POINTER(spv->resultTypeId))
    {
        spv->resultTypeId = SPV_ID_TYPE_POINTER_OBJECT_SPV_TYPE(spv->resultTypeId);
    }

    /* Since we can only support scalar and vector in a operand,
    ** so put the rest constant into a variable.
    */

    /* If this is a scalar constant, just mark this result as a constant. */
    if (SPV_ID_TYPE_IS_SCALAR(spv->resultTypeId) || SPV_ID_TYPE_IS_BOOLEAN(spv->resultTypeId))
    {
        /* Get the vir type. */
        VIR_TypeId typeID = SPV_ID_TYPE_VIR_TYPE_ID(spv->resultTypeId);
        type = VIR_Shader_GetTypeFromId(virShader, typeID);

        /* Set the ID as a constant. */
        SPV_ID_TYPE(targetId) = SPV_ID_TYPE_CONST;

        if (findSpecData)
        {
            /*gcmASSERT(mapEntry->size == 4);*/
            gcoOS_MemCopy(&constValue, specData, mapEntry->size);
            SPV_ID_VIR_CONST(targetId).scalarVal.uValue = constValue;
        }
        else if (spv->opCode == SpvOpConstantNull)
        {
            SPV_ID_VIR_CONST(targetId).scalarVal.fValue = 0.0f;
            SPV_ID_VIR_CONST(targetId).scalarVal.iValue = 0;
            SPV_ID_VIR_CONST(targetId).scalarVal.uValue = 0;
        }
        else if (SPV_ID_TYPE_IS_FLOAT(spv->resultTypeId))
        {
            SPV_ID_VIR_CONST(targetId).scalarVal.fValue = (*(gctFLOAT *)(&constValue));
        }
        else if (SPV_ID_TYPE_IS_SIGNEDINTEGER(spv->resultTypeId))
        {
            SPV_ID_VIR_CONST(targetId).scalarVal.iValue = (gctINT)constValue;
        }
        else if (SPV_ID_TYPE_IS_UNSIGNEDINTEGER(spv->resultTypeId))
        {
            SPV_ID_VIR_CONST(targetId).scalarVal.uValue = constValue;
        }
        else if (SPV_ID_TYPE_IS_BOOLEAN(spv->resultTypeId))
        {
            /* Spec constant is just default value, if driver re-write, we dont set this.*/
            if (spv->opCode == SpvOpConstantTrue ||
                spv->opCode == SpvOpSpecConstantTrue
                )
            {
                SPV_ID_VIR_CONST(targetId).scalarVal.iValue = 1;
            }
            else if (spv->opCode == SpvOpConstantFalse ||
                     spv->opCode == SpvOpConstantNull ||
                     spv->opCode == SpvOpSpecConstantFalse
                     )
            {
                SPV_ID_VIR_CONST(targetId).scalarVal.iValue = 0;
            }
        }
        else
        {
            gcmASSERT(0);
        }

        VIR_Shader_AddConstant(virShader, typeID, &SPV_ID_VIR_CONST(targetId), &cstId);
        addSym = gcvTRUE;

        SPV_ID_VIR_CONST_ID(targetId) = cstId;
        SPV_ID_VIR_TYPE_ID(targetId) = typeID;
        SPV_ID_CST_SPV_TYPE(targetId) = spv->resultTypeId;
    }
    /* TODO: add sampler support. */
    else if (SPV_ID_TYPE_IS_SAMPLER(spv->resultTypeId))
    {
        gcmASSERT(0);
    }
    /* If this is a vector constant, just mark this result as a constant. */
    else if (SPV_ID_TYPE_IS_VECTOR(spv->resultTypeId))
    {
        VIR_TypeId typeID = SPV_ID_TYPE_VIR_TYPE_ID(spv->resultTypeId);
        gctUINT compType = SPV_ID_TYPE_VEC_COMP_TYPE(spv->resultTypeId);
        gctUINT i, compCount = spv->operandSize;
        gctBOOL isConstantNull = (spv->opCode == SpvOpConstantNull);

        /* Get the vir type. */
        type = VIR_Shader_GetTypeFromId(virShader, typeID);

        /* Set the ID as a constant. */
        SPV_ID_TYPE(targetId) = SPV_ID_TYPE_CONST;

        if (isConstantNull)
        {
            compCount = SPV_ID_TYPE_VEC_COMP_NUM(spv->resultTypeId);
        }

        if (SPV_ID_TYPE_IS_FLOAT(compType))
        {
            for (i = 0; i < compCount; i++)
            {
                SPV_ID_VIR_CONST(targetId).vecVal.f32Value[i] =
                    isConstantNull ? 0.0f : SPV_ID_VIR_CONST(spv->operands[i]).scalarVal.fValue;
            }
        }
        else if (SPV_ID_TYPE_IS_UNSIGNEDINTEGER(SPV_ID_TYPE_VEC_COMP_TYPE(spv->resultTypeId)))
        {
            for (i = 0; i < compCount; i++)
            {
                SPV_ID_VIR_CONST(targetId).vecVal.u32Value[i] =
                    isConstantNull ? 0 : SPV_ID_VIR_CONST(spv->operands[i]).scalarVal.uValue;
            }
        }
        else if (SPV_ID_TYPE_IS_SIGNEDINTEGER(SPV_ID_TYPE_VEC_COMP_TYPE(spv->resultTypeId)))
        {
            for (i = 0; i < compCount; i++)
            {
                SPV_ID_VIR_CONST(targetId).vecVal.i32Value[i] =
                    isConstantNull ? 0 : SPV_ID_VIR_CONST(spv->operands[i]).scalarVal.iValue;
            }
        }
        else if (SPV_ID_TYPE_IS_BOOLEAN(SPV_ID_TYPE_VEC_COMP_TYPE(spv->resultTypeId)))
        {
            for (i = 0; i < compCount; i++)
            {
                if (isConstantNull)
                {
                    SPV_ID_VIR_CONST(targetId).vecVal.u32Value[i] = 0;
                }
                else
                {
                    SPV_ID_VIR_CONST(targetId).vecVal.u32Value[i] =
                        (SPV_ID_VIR_CONST(spv->operands[i]).scalarVal.iValue == 0) ? 0 : 1;
                }
            }
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }

        VIR_Shader_AddConstant(virShader, typeID, &SPV_ID_VIR_CONST(targetId), &cstId);
        addSym = gcvTRUE;

        SPV_ID_VIR_CONST_ID(targetId) = cstId;
        SPV_ID_VIR_TYPE_ID(targetId) = typeID;
        SPV_ID_CST_SPV_TYPE(targetId) = spv->resultTypeId;

        /* save the spv id of components */
        for (i = 0; i < compCount; i++)
        {
            SPV_ID_CST_VEC_SPVID(targetId, i) = spv->operands[i];
        }
    }
    /* If this is a matrix/array/struct constant,
    ** then the operands could be constant or variable, call VIR_Shader_CompositeConstruct to construct it.
    */
    else if (SPV_ID_TYPE_IS_MATRIX(spv->resultTypeId) ||
             SPV_ID_TYPE_IS_STRUCT(spv->resultTypeId) ||
             SPV_ID_TYPE_IS_ARRAY(spv->resultTypeId))
    {
        virErrCode = __SpvAddComplexTypeConstant(spv,
                                                 virShader,
                                                 spv->opCode == SpvOpConstantNull);
        if (virErrCode != VSC_ERR_NONE) return virErrCode;
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    /* Check if the target of this constant is a built-in constant variable. */
    if (dec)
    {
        __SpvConvToBuiltInConst(spv,
                                virShader,
                                dec,
                                targetId);
    }

    if (addSym)
    {
        virErrCode = VIR_Shader_AddSymbol(virShader,
                                          VIR_SYM_CONST,
                                          cstId,
                                          type,
                                          VIR_STORAGE_UNKNOWN,
                                          &cstSymId);
        if (virErrCode == VSC_ERR_REDEFINITION ||
            virErrCode == VSC_ERR_NONE)
        {
            SPV_SET_IDDESCRIPTOR_SYM(spv, targetId, cstSymId);
            virErrCode = VSC_ERR_NONE;
        }
    }

    return virErrCode;
}

static VSC_ErrCode __SpvAddLabel(gcSPV spv, VIR_Shader * virShader)
{
    VIR_LabelId labelId;
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    gctCHAR labelName[20];
    VIR_Label *virLabel;
    gctUINT offset = 0;
    VIR_Instruction *   virInst = gcvNULL;
    gctBOOL functionLabel = gcvFALSE;
    gctBOOL needSetCaller = gcvFALSE;
    gctUINT i;
#if SPV_NEW_FUNCPARAM
    VIR_Operand        *operand;
#endif

    gcmVERIFY_OK(gcoOS_PrintStrSafe(labelName,
                                    20,
                                    &offset,
                                    "#sh_%u",
                                    spv->resultId));

    virErrCode = VIR_Function_AddLabel(spv->virFunction,
                                        labelName,
                                        &labelId);

    virErrCode =  VIR_Function_AddInstruction(spv->virFunction,
                                              SPV_OPCODE_2_VIR_OPCODE(SpvOpLabel),
                                              SPV_VIR_OP_FORMAT(spv->opCode),
                                              &virInst);

    if(virErrCode != VSC_ERR_NONE) return virErrCode;

    /* do dynamic size check if needed */
    SPV_CHECK_FUNC_CALLER(spv, spv->resultId);

    virLabel = VIR_GetLabelFromId(spv->virFunction, labelId);
    virLabel->defined = virInst;
    VIR_Operand_SetLabel(VIR_Inst_GetDest(virInst), virLabel);

    SPV_SET_IDDESCRIPTOR_NAME(spv, spv->resultId, labelId);
    SPV_ID_TYPE(spv->resultId) = SPV_ID_TYPE_LABEL;

    functionLabel = (SPV_ID_FUNC_LABEL(spv->func) == SPV_INVALID_ID);
    needSetCaller = (functionLabel && (SPV_ID_FUNC_CALLER_NUM(spv->func) != 0));

    if ((SPV_IDDESCRIPTOR_LABEL(spv, spv->resultId).destCount > 0) ||
        (SPV_IDDESCRIPTOR_LABEL(spv, spv->resultId).phiCount > 0) ||
        needSetCaller)
    {
        VIR_Instruction *virCallInst;
        VIR_Operand *virDest;
        VIR_Link* link;
        VIR_PhiOperand * phiOperand;

        if (needSetCaller)
        {
            for (i = 0; i < SPV_ID_FUNC_CALLER_NUM(spv->func); i++)
            {
                virDest = SPV_ID_FUNC_VIR_CALLER_INST(spv->func,i)->dest;
                VIR_Operand_SetFunction(virDest, spv->virFunction);

#if SPV_NEW_FUNCPARAM
                virInst = SPV_ID_FUNC_PARAM_INST(spv->func, i);
                operand = VIR_Inst_GetDest(virInst);
                VIR_Operand_SetFunction(operand, spv->virFunction);
#endif
            }

            /* clear unhandled label */
            SPV_IDDESCRIPTOR_LABEL(spv, spv->resultId).destCount = 0;
        }
        else
        {
            /* Check JMP users. */
            for (i = 0; i < SPV_IDDESCRIPTOR_LABEL(spv, spv->resultId).destCount; i++)
            {
                virCallInst = SPV_IDDESCRIPTOR_LABEL(spv, spv->resultId).virInst[i];
                virDest = SPV_IDDESCRIPTOR_LABEL(spv, spv->resultId).virDest[i];
                virLabel = VIR_Function_GetLabelFromId(spv->virFunction, labelId);
                VIR_Operand_SetLabel(virDest, virLabel);
                VIR_Function_NewLink(spv->virFunction, &link);
                VIR_Link_SetReference(link, (gctUINTPTR_T)virCallInst);
                VIR_Link_AddLink(&(virLabel->referenced), link);
            }
            SPV_IDDESCRIPTOR_LABEL(spv, spv->resultId).destCount = 0;

            /* Check PHI users. */
            for (i = 0; i < SPV_IDDESCRIPTOR_LABEL(spv, spv->resultId).phiCount; i++)
            {
                phiOperand = SPV_IDDESCRIPTOR_LABEL(spv, spv->resultId).phiOperands[i];
                VIR_PhiOperand_SetLabel(phiOperand, virLabel);
            }
            SPV_IDDESCRIPTOR_LABEL(spv, spv->resultId).phiCount = 0;
        }
    }

    if (functionLabel)
    {
        SPV_ID_FUNC_LABEL(spv->func) = spv->resultId;
        SPV_ID_FUNC_VIR_LABEL(spv->func) = VIR_Function_GetLabelFromId(spv->virFunction, labelId);
    }

    return virErrCode;
}

static VSC_ErrCode __SpvAddExtInst(gcSPV spv, VIR_Shader * virShader)
{

    __SpvAddIdSymbol(spv, virShader, gcvNULL, spv->resultId, spv->resultTypeId, VIR_SYM_VARIABLE, VIR_STORAGE_GLOBAL, gcvFALSE);

    /* we need plan a table which include argument variable, return variable, out parameter info to generate instructions */
    gcmASSERT(0);

    return VSC_ERR_NONE;
}

static VSC_ErrCode __SpvAddFuncCall(gcSPV spv, VIR_Shader * virShader)
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;

    VIR_Instruction * virInst;
    gctUINT func = spv->operands[0];
    gctUINT i;
    gctBOOL funcDef;
    gctBOOL hasReturn;
    gctUINT index;
    gctUINT callerListIndex;
#if SPV_NEW_FUNCPARAM
    VIR_Operand        *operand;
    VIR_ParmPassing    *parmOpnd;
    SpvId               spvArgs;
#endif

    /* do dynamic size check if needed */
    SPV_CHECK_FUNC_CALLER(spv, func);

    funcDef = (SPV_ID_FUNC_VIR_LABEL(func) != gcvNULL);

    index = SPV_ID_FUNC_CALLER_NUM(func);

    /* create symbol for function call result */
    __SpvAddIdSymbol(spv, virShader, gcvNULL, spv->resultId, spv->resultTypeId, VIR_SYM_VARIABLE, VIR_STORAGE_GLOBAL, gcvFALSE);

    SPV_ID_FUNC_VIR_CALLER_ARG_NUM(func, index) = spv->operandSize - 1;

    hasReturn = !SPV_ID_TYPE_IS_VOID(spv->resultTypeId);

    /* Record this caller. */
    callerListIndex = SPV_ID_FUNC_CALLER_NUM(func);

#if SPV_NEW_FUNCPARAM
    /* add a func parameter inst */
    virErrCode = VIR_Function_AddInstruction(
        spv->virFunction,
        VIR_OP_PARM,
        SPV_ID_TYPE_VIR_TYPE_ID(spv->resultTypeId), /* TODO: type should be what? */
        &virInst);

    VIR_Inst_SetConditionOp(virInst, VIR_COP_ALWAYS);

    if (funcDef)
    {
        operand = VIR_Inst_GetDest(virInst);
        VIR_Operand_SetFunction(operand, SPV_ID_FUNC_VIR_FUNCION(func));
    }

    VIR_Function_NewParameters(spv->virFunction, (hasReturn ? (spv->operandSize) : (spv->operandSize - 1)), &parmOpnd);
    operand = VIR_Inst_GetSource(virInst, 0);
    VIR_Operand_SetParameters(operand, parmOpnd);

    for (i = 1; i < spv->operandSize; i++)
    {
        spvArgs = spv->operands[i];
        operand = parmOpnd->args[i - 1];

        VIR_Operand_SetSwizzle(operand, __SpvID2Swizzle(spv, spvArgs));
        VIR_Operand_SetSym(operand, SPV_ID_VIR_SYM(spvArgs));
        VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
        VIR_Operand_SetTypeId(operand, SPV_ID_VIR_TYPE_ID(spvArgs));
        VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
        VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
    }

    if (hasReturn)
    {
        operand = parmOpnd->args[i - 1];

        VIR_Operand_SetSwizzle(operand, __SpvID2Swizzle(spv, spv->resultId));
        VIR_Operand_SetSym(operand, SPV_ID_VIR_SYM(spv->resultId));
        VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
        VIR_Operand_SetTypeId(operand, SPV_ID_VIR_TYPE_ID(spv->resultId));
        VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
        VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
    }

    SPV_ID_FUNC_PARAM_INST(func, callerListIndex) = virInst;
#endif

    virErrCode = VIR_Function_AddInstruction(spv->virFunction,
        VIR_OP_CALL,
        SPV_ID_TYPE_VIR_TYPE_ID(spv->resultTypeId),
        &virInst);

    VIR_Inst_SetConditionOp(virInst, VIR_COP_ALWAYS);
    VIR_Operand_SetRoundMode(VIR_Inst_GetDest(virInst), VIR_ROUND_DEFAULT);
    VIR_Operand_SetModifier(VIR_Inst_GetDest(virInst), VIR_MOD_NONE);

    for (i = 1; i < spv->operandSize; i++)
    {
        SPV_ID_SYM_PARAM_TO_FUNC(spv->operands[i]) = func;
    }

    /* Check the qualifier of this function. */
    if (!funcDef)
    {
        /* Allocate paramters and set qualifier for them. */
        SPV_ID_FUNC_ARG_NUM(func) = spv->operandSize - 1;

        if (SPV_ID_FUNC_ARG_NUM(func) > 0)
        {
            spvAllocate(spv->spvMemPool,sizeof (SpvId) * SPV_ID_FUNC_ARG_NUM(func), (gctPOINTER *)(&SPV_ID_FUNC_SPV_ARG(func)));
            spvAllocate(spv->spvMemPool, sizeof(SpvParameterStorage) * SPV_ID_FUNC_ARG_NUM(func), (gctPOINTER *)(&SPV_ID_FUNC_ARG_STORAGE(func)));
        }
    }

    for (i = 0; i < SPV_ID_FUNC_ARG_NUM(func); i++)
    {
        SPV_ID_FUNC_ARG_STORAGE(func)[i] = SPV_PARA_NONE;

        if (SPV_ID_SYM_USED_STORE_AS_DEST(spv->operands[i + 1]) ||
            SPV_ID_INITIALIZED(spv->operands[i + 1]))
        {
            if (SPV_ID_FUNC_ARG_STORAGE(func)[i] == SPV_PARA_OUT)
            {
                SPV_ID_FUNC_ARG_STORAGE(func)[i] = SPV_PARA_INOUT;
            }
            else
            {
                SPV_ID_FUNC_ARG_STORAGE(func)[i] = SPV_PARA_IN;
            }
        }
        if (SPV_ID_SYM_USED_LOAD_AS_DEST(spv->operands[i + 1]))
        {
            SPV_ID_FUNC_ARG_STORAGE(func)[i] = SPV_PARA_CONST;
        }
    }

    if (funcDef)
    {
        VIR_Operand_SetFunction(VIR_Inst_GetDest(virInst), SPV_ID_FUNC_VIR_FUNCION(func));
    }

    if (spv->operandSize > 1)
    {
        /* not define, add parameter to caller */
        spvAllocate(spv->spvMemPool, sizeof(SpvId) * (spv->operandSize - 1), (gctPOINTER *)(&SPV_ID_FUNC_CALLER_SPV_ARG(func, index)));

        for (i = 1; i < spv->operandSize; i++)
        {
            SPV_ID_FUNC_CALLER_SPV_ARG(func, index)[i - 1] = spv->operands[i];
        }
    }

    SPV_ID_FUNC_VIR_CALLER_VIR_FUNC(func, callerListIndex) = spv->virFunction;
    SPV_ID_FUNC_VIR_CALLER_INST(func, callerListIndex) = virInst;
    SPV_ID_FUNC_CALL_SPV_RET_ID(func, callerListIndex) = spv->resultId;
    SPV_ID_FUNC_CALLER_NUM(func)++;

    /* add return type id, in case we need get information after this call */
    SPV_ID_FUNC_RETURN_SPV_TYPE(func) = spv->resultTypeId;

    return virErrCode;
}

static VSC_ErrCode __SpvAddReturnValue(gcSPV spv, VIR_Shader * virShader)
{
    /* generate mov instructions */
    SpvId spvRet = spv->operands[0];
    if (SPV_ID_TYPE(spvRet) == SPV_ID_TYPE_CONST)
    {
        __SpvInsertInstruction3(spv, virShader, gcvNULL, SPV_INSERT_CURRENT, gcvNULL, VIR_OP_MOV,
            spv->operands[0], SPV_ID_CST_SPV_TYPE(spv->operands[0]),
            SPV_ID_FUNC_VIR_RET_SYM(spv->func), SPV_ID_FUNC_RETURN_SPV_TYPE(spv->func));
    }
    else
    {
        __SpvInsertInstruction2(spv, virShader, gcvNULL, SPV_INSERT_CURRENT, gcvNULL, VIR_OP_MOV,
            SPV_ID_VIR_SYM(spv->operands[0]), SPV_ID_SYM_SPV_TYPE(spv->operands[0]),
            SPV_ID_FUNC_VIR_RET_SYM(spv->func), SPV_ID_FUNC_RETURN_SPV_TYPE(spv->func));
    }

    __SpvAddReturn(spv, virShader);

    return VSC_ERR_NONE;
}

static VSC_ErrCode __SpvAddFunctionParameter(gcSPV spv, VIR_Shader * virShader)
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    SpvId id = spv->resultId;
    SpvId type = spv->resultTypeId;
    VIR_NameId nameId;
    VIR_SymId symId;
    VIR_Symbol * sym;
    VIR_TypeId virTypeId;
    gctSTRING paramName;

    if (SPV_ID_FUNC_SPV_ARG(spv->func) == gcvNULL)
    {
        spvAllocate(spv->spvMemPool, (SPV_ID_FUNC_ARG_NUM(spv->func) + 1) * gcmSIZEOF(SpvId), (gctPOINTER *)&SPV_ID_FUNC_SPV_ARG(spv->func));
    }

    SPV_ID_FUNC_SPV_ARG(spv->func)[spv->argIndex] = spv->resultId;

    if (SPV_ID_TYPE_IS_POINTER(type))
    {
        type = SPV_ID_TYPE_POINTER_OBJECT_SPV_TYPE(type);
    }

    if (SPV_ID_VIR_NAME_ID(spv->resultId) == VIR_INVALID_ID)
    {
        /* Generate a name and add name to VIR */
        __SpvGenerateVIRName(spv, id);
        VIR_Shader_AddString(virShader, spv->virName, &nameId);
    }
    else
    {
        nameId = SPV_ID_VIR_NAME_ID(spv->resultId);
    }

    if (SPV_ID_TYPE_IS_POINTER(type))
    {
        virTypeId = SPV_ID_TYPE_VIR_TYPE_ID(SPV_ID_TYPE_POINTER_OBJECT_SPV_TYPE(type));
    }
    else
    {
        virTypeId = SPV_ID_TYPE_VIR_TYPE_ID(type);
    }

    paramName = VIR_Shader_GetStringFromId(virShader, nameId);

    /*Create VIR sym, we don't know type or other attribute*/
    virErrCode = VIR_Function_AddParameter(
        spv->virFunction,
        paramName,
        virTypeId,
        VIR_STORAGE_UNKNOWN,
        &symId);

    sym = VIR_Function_GetSymFromId(spv->virFunction, symId);

    VIR_Symbol_SetPrecision(sym, VIR_PRECISION_HIGH);
    VIR_Symbol_SetTyQualifier(sym, VIR_TYQUAL_NONE);
    VIR_Symbol_SetLayoutQualifier(sym, VIR_LAYQUAL_NONE);
    VIR_Symbol_SetFlag(sym,VIR_SYMFLAG_WITHOUT_REG);
    VIR_Symbol_SetLocation(sym, -1);

    SPV_SET_IDDESCRIPTOR_NAME(spv, id, nameId);

    /* record symID, so we could get sym from id */
    SPV_SET_IDDESCRIPTOR_SYM(spv, id, symId);
    SPV_SET_IDDESCRIPTOR_TYPE(spv, id, SPV_ID_TYPE_VIR_TYPE_ID(type));
    SPV_SET_IDDESCRIPTOR_SPV_TYPE(spv, id, type);
    SPV_ID_TYPE(id) = SPV_ID_TYPE_SYMBOL;
    SPV_ID_SYM_IS_FUNC_PARAM(id) = gcvTRUE;
    SPV_ID_SYM_VIR_FUNC(id) = spv->virFunction;

    spv->argIndex++;

    SPV_ID_SYM_USED_STORE_AS_DEST(spv->resultId) = gcvTRUE;

    return virErrCode;
}

static VSC_ErrCode __SpvSetSampledImage(gcSPV spv, VIR_Shader * virShader, SpvId OperandId)
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    SpvId spvOperand = OperandId;

    gcmASSERT(SPV_ID_SYM_SAMPLED_IMAGE(spvOperand));

    VIR_Symbol_SetSeparateImageId(SPV_ID_VIR_SYM(spvOperand),SPV_ID_VIR_SYM_ID(SPV_ID_SYM_SAMPLEDIMAGE_IMAGE(spvOperand)));

    /* accessChain from array, check type */
    if (SPV_ID_SYM_OFFSET_TYPE(SPV_ID_SYM_SAMPLEDIMAGE_IMAGE(spvOperand)) == VIR_SYM_CONST)
    {
        VIR_Symbol_SetImgIdxRange(SPV_ID_VIR_SYM(spvOperand), SPV_ID_SYM_OFFSET_VALUE(SPV_ID_SYM_SAMPLEDIMAGE_IMAGE(spvOperand)));
    }
    /* directly symbel */
    else if (SPV_ID_SYM_OFFSET_TYPE(SPV_ID_SYM_SAMPLEDIMAGE_IMAGE(spvOperand)) == VIR_SYM_UNKNOWN)
    {
        VIR_Symbol_SetImgIdxRange(SPV_ID_VIR_SYM(spvOperand), 0);
    }
    /* accessChain from array, and it's dynamic */
    else
    {
        gcmASSERT(SPV_ID_SYM_OFFSET_TYPE(SPV_ID_SYM_SAMPLEDIMAGE_IMAGE(spvOperand)) == VIR_SYM_VARIABLE);
        gcmASSERT(gcvFALSE);
        VIR_Symbol_SetImgIdxRange(SPV_ID_VIR_SYM(spvOperand), -1);
    }

    VIR_Symbol_SetSeparateSamplerId(SPV_ID_VIR_SYM(spvOperand),SPV_ID_VIR_SYM_ID(SPV_ID_SYM_SAMPLEDIMAGE_SAMPLER(spvOperand)));

    if (SPV_ID_SYM_OFFSET_TYPE(SPV_ID_SYM_SAMPLEDIMAGE_SAMPLER(spvOperand)) == VIR_SYM_CONST)
    {
        VIR_Symbol_SetSamplerIdxRange(SPV_ID_VIR_SYM(spvOperand), SPV_ID_SYM_OFFSET_VALUE(SPV_ID_SYM_SAMPLEDIMAGE_SAMPLER(spvOperand)));
    }
    else if (SPV_ID_SYM_OFFSET_TYPE(SPV_ID_SYM_SAMPLEDIMAGE_SAMPLER(spvOperand)) == VIR_SYM_UNKNOWN)
    {
        VIR_Symbol_SetSamplerIdxRange(SPV_ID_VIR_SYM(spvOperand), 0);
    }
    else
    {
        gcmASSERT(SPV_ID_SYM_OFFSET_TYPE(SPV_ID_SYM_SAMPLEDIMAGE_SAMPLER(spvOperand)) == VIR_SYM_VARIABLE);
        gcmASSERT(gcvFALSE);
        VIR_Symbol_SetSamplerIdxRange(SPV_ID_VIR_SYM(spvOperand), -1);
    }

    VIR_Symbol_SetFlag(SPV_ID_VIR_SYM(spvOperand), VIR_Symbol_GetFlag(SPV_ID_VIR_SYM(spvOperand)) | VIR_SYMFLAG_COMBINED_SAMPLER);

    return virErrCode;
}

static VSC_ErrCode __SpvAddIntrisicFunction(gcSPV spv, VIR_Shader * virShader)
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    gctUINT setId = spv->operands[0];
    gctUINT instId = spv->operands[1];
    gctUINT argCount = spv->operandSize - 2;
    gctBOOL hasDest = gcvFALSE;
    VIR_TypeId instTypeId;
    VIR_Symbol * dstVirSym = gcvNULL;

    VIR_Instruction *virInst;
    gctUINT i;
    VIR_IntrinsicsKind intrsicId;
    VIR_ParmPassing     *parmOpnd;

    SpvId       pointedTy = spv->resultTypeId;

    while (SPV_ID_TYPE_IS_POINTER(pointedTy))
    {
        pointedTy = SPV_ID_TYPE_POINTER_OBJECT_SPV_TYPE(pointedTy);
    }

    /* Check return type. */
    if (InstructionDesc[spv->opCode].resultPresent)
    {
        hasDest = gcvTRUE;
        if (!SPV_ID_TYPE_IS_VOID(spv->resultTypeId))
        {
            if (SPV_ID_VIR_NAME_ID(spv->resultId) == VIR_INVALID_ID)
            {
                __SpvAddIdSymbol(spv, virShader, gcvNULL, spv->resultId, spv->resultTypeId, VIR_SYM_VARIABLE, VIR_STORAGE_GLOBAL, gcvFALSE);
            }

            dstVirSym = VIR_Shader_GetSymFromId(virShader, SPV_ID_VIR_SYM_ID(spv->resultId));
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }
        instTypeId = SPV_ID_TYPE_VIR_TYPE_ID(pointedTy);
    }
    else
    {
        instTypeId = VIR_TYPE_UNKNOWN;
    }

    virErrCode = VIR_Function_AddInstruction(spv->virFunction,
        VIR_OP_INTRINSIC,
        instTypeId,
        &virInst);

    VIR_Inst_SetConditionOp(virInst, VIR_COP_ALWAYS);

    /* destination */
    if (hasDest)
    {
        VIR_Operand_SetRoundMode(VIR_Inst_GetDest(virInst), VIR_ROUND_DEFAULT);
        VIR_Operand_SetModifier(VIR_Inst_GetDest(virInst), VIR_MOD_NONE);
        VIR_Operand_SetEnable(VIR_Inst_GetDest(virInst), __SpvGenEnable(spv, SPV_ID_TYPE_VIR_TYPE(pointedTy), spv->resultTypeId));
        VIR_Operand_SetOpKind(VIR_Inst_GetDest(virInst), VIR_OPND_SYMBOL);
        VIR_Operand_SetTypeId(VIR_Inst_GetDest(virInst), SPV_ID_TYPE_VIR_TYPE_ID(pointedTy));
        VIR_Operand_SetSym(VIR_Inst_GetDest(virInst), dstVirSym);
    }

    /* src0 is intrinsicKind */
    intrsicId = VIR_IntrisicGetKind(setId, instId);
    VIR_Operand_SetIntrinsic(VIR_Inst_GetSource(virInst, 0), intrsicId);

    /* src1 is parameter */
    VIR_Function_NewParameters(spv->virFunction, argCount, &parmOpnd);
    if (argCount > 0)
    {
        for (i = 2; i < argCount + 2; i++)
        {
            VIR_Swizzle virSwizzle;
            VIR_Operand *srcOpnd = parmOpnd->args[i-2];

            if ((i >= 4 && (spv->opCode == SpvOpImageRead || spv->opCode == SpvOpImageFetch))
                ||
                (i >= 5 && spv->opCode == SpvOpImageWrite))
            {
                __SpvDecodeImageOperand(
                    spv,
                    virShader,
                    srcOpnd,
                    spv->operands[i],
                    &spv->operands[i + 1],
                    spv->operandSize - i - 1
                    );

                /* after handle the image fetch, we can finish the call */
                break;
            }

            virSwizzle = __SpvID2Swizzle(spv, spv->operands[i]);
            if ((i == 3) &&
                (spv->opCode == SpvOpImageRead) &&
                (VIR_TypeId_isImageSubPassData(SPV_ID_TYPE_VIR_TYPE_ID(spv->operands[2]))))
            {
                /* handle subpass */
                VIR_Operand_SetSwizzle(srcOpnd, virSwizzle);
                VIR_Operand_SetSym(srcOpnd, spv->internalSym);
                VIR_Operand_SetOpKind(srcOpnd, VIR_OPND_SYMBOL);
                VIR_Operand_SetTypeId(srcOpnd, SPV_ID_VIR_TYPE_ID(spv->operands[i]));
                VIR_Operand_SetPrecision(srcOpnd, VIR_PRECISION_HIGH);
                VIR_Operand_SetRoundMode(srcOpnd, VIR_ROUND_DEFAULT);
                VIR_Operand_SetModifier(srcOpnd, VIR_MOD_NONE);
            }
            else if (SPV_ID_TYPE(spv->operands[i]) == SPV_ID_TYPE_SYMBOL)
            {
                gctUINT resultTypeId = spv->operands[i];

                /* For some sampled image opcodes, use its sampler directly */
                if (SPV_ID_SYM_SAMPLED_IMAGE(resultTypeId)
                    &&
                    (spv->opCode == SpvOpImageFetch         ||
                     spv->opCode == SpvOpImageQuerySizeLod  ||
                     spv->opCode == SpvOpImageQuerySize     ||
                     spv->opCode == SpvOpImageQueryLod      ||
                     spv->opCode == SpvOpImageQueryLevels   ||
                     spv->opCode == SpvOpImageQuerySamples)
                    &&
                    (i == 2))
                {
                    resultTypeId = SPV_ID_SYM_SAMPLEDIMAGE_SAMPLER(resultTypeId);
                }
                VIR_Operand_SetSwizzle(srcOpnd, virSwizzle);
                VIR_Operand_SetSym(srcOpnd, SPV_ID_VIR_SYM(resultTypeId));
                VIR_Operand_SetOpKind(srcOpnd, VIR_OPND_SYMBOL);
                VIR_Operand_SetTypeId(srcOpnd, SPV_ID_VIR_TYPE_ID(resultTypeId));
                VIR_Operand_SetPrecision(srcOpnd, VIR_PRECISION_HIGH);
                VIR_Operand_SetRoundMode(srcOpnd, VIR_ROUND_DEFAULT);
                VIR_Operand_SetModifier(srcOpnd, VIR_MOD_NONE);

                if (SPV_ID_SYM_SAMPLED_IMAGE(resultTypeId))
                {
                    __SpvSetSampledImage(spv, virShader, resultTypeId);
                }
            }
            else if (SPV_ID_TYPE(spv->operands[i]) == SPV_ID_TYPE_CONST)
            {
                VIR_Operand_SetSwizzle(srcOpnd, virSwizzle);
                VIR_Operand_SetConstId(srcOpnd, SPV_ID_VIR_CONST_ID(spv->operands[i]));
                VIR_Operand_SetOpKind(srcOpnd, VIR_OPND_CONST);
                VIR_Operand_SetTypeId(srcOpnd, SPV_ID_VIR_TYPE_ID(spv->operands[i]));
                VIR_Operand_SetPrecision(srcOpnd, VIR_PRECISION_HIGH);
                VIR_Operand_SetRoundMode(srcOpnd, VIR_ROUND_DEFAULT);
                VIR_Operand_SetModifier(srcOpnd, VIR_MOD_NONE);
            }
            else
            {
                gcmASSERT(0);
            }
            __SetAccessChainOffsetToOperand(spv, spv->operands[i], srcOpnd, SpvOffsetType_Normal);
        }
    }
    VIR_Operand_SetParameters(VIR_Inst_GetSource(virInst, 1), parmOpnd);

    /* Update resOpType. */
    VIR_Inst_UpdateResOpType(virInst);

    return virErrCode;
}

static VSC_ErrCode __SpvCheckUnhandledWorkGroupVar(gcSPV spv, VIR_Shader * virShader)
{
    if (SPV_HAS_WORKGROUP())
    {
        VIR_Operand        *operand = VIR_Inst_GetSource(SPV_WORKGROUP_INFO()->offsetCalVirInst, 1);
        VIR_StorageBlock   *sbo;
        VIR_ScalarConstVal  constVal;
        VIR_Symbol         *sym;

        constVal.uValue = SPV_WORKGROUP_INFO()->groupSize;
        VIR_Operand_SetImmediate(operand, VIR_TYPE_UINT32, constVal);
        VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
        VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XXXX);
        VIR_Operand_SetOpKind(operand, VIR_OPND_IMMEDIATE);
        VIR_Operand_SetTypeId(operand, VIR_TYPE_UINT32);
        VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);

        sym = VIR_Shader_GetSymFromId(virShader, SPV_WORKGROUP_INFO()->sharedSboSymId);

        /* set SBO data. */
        sbo = sym->u2.sbo;
        gcmASSERT(sbo);
        VIR_SBO_SetBlockSize(sbo, SPV_WORKGROUP_INFO()->groupSize);

        if (VIR_IB_GetFlags(sbo) & VIR_IB_FOR_SHARED_VARIABLE)
        {
            VIR_Shader_SetLocalMemorySize(virShader, VIR_SBO_GetBlockSize(sbo));
        }
    }

    return VSC_ERR_NONE;
}

static VSC_ErrCode __SpvInsertInstAtFuncBegin(gcSPV spv, VIR_Shader * virShader)
{
    VIR_NameId nameId;
    VIR_SymId symId;
    VIR_Instruction * virInst;
    VIR_Operand * operand;
    VIR_Symbol *sym, *workgroupIdSym;
    VIR_ScalarConstVal  constVal;

    if (spv->shaderStage == VSC_SHADER_STAGE_CS && SPV_HAS_WORKGROUP())
    {
        /* we should generate mod before getting workgroupId */
        VIR_Shader_AddString(virShader, "#sh_workgroupId", &nameId);

        VIR_Shader_AddSymbol(
            virShader,
            VIR_SYM_VARIABLE,
            nameId,
            VIR_Shader_GetTypeFromId(virShader, VIR_TYPE_UINT16),
            VIR_STORAGE_GLOBAL,
            &symId
            );
        workgroupIdSym = VIR_Shader_GetSymFromId(virShader, symId);
        VIR_Symbol_SetPrecision(workgroupIdSym, VIR_PRECISION_HIGH);
        VIR_Symbol_SetTyQualifier(workgroupIdSym, VIR_TYQUAL_NONE);
        VIR_Symbol_SetLayoutQualifier(workgroupIdSym, VIR_LAYQUAL_NONE);
        VIR_Symbol_SetFlag(workgroupIdSym, VIR_SYMFLAG_WITHOUT_REG);
        VIR_Symbol_SetLocation(workgroupIdSym, -1);

        VIR_Function_AddInstruction(spv->virFunction, VIR_OP_MOD, VIR_TYPE_UINT16, &virInst);

        operand = VIR_Inst_GetDest(virInst);
        VIR_Operand_SetSym(operand, workgroupIdSym);
        VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
        VIR_Operand_SetEnable(operand, VIR_TypeId_Conv2Enable(VIR_TYPE_UINT16));
        VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
        VIR_Operand_SetTypeId(operand, VIR_TYPE_UINT16);
        VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);

        operand = VIR_Inst_GetSource(virInst, 0);
        VIR_Operand_SetSym(operand, VIR_Shader_GetSymFromId(virShader, SPV_WORKGROUP_INFO()->invocationIdSymId));
        VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
        VIR_Operand_SetSwizzle(operand, VIR_Enable_2_Swizzle_WShift(VIR_TypeId_Conv2Enable(VIR_TYPE_UINT16)));
        VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
        VIR_Operand_SetTypeId(operand, VIR_TYPE_UINT16);
        VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);

        /* The src1 should be the number of concurrent workgroups, which will be known after register allocation */
        constVal.uValue = 100;
        operand = VIR_Inst_GetSource(virInst, 1);
        VIR_Operand_SetImmediate(operand, VIR_TYPE_UINT16, constVal);
        VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
        VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XXXX);
        VIR_Operand_SetOpKind(operand, VIR_OPND_IMMEDIATE);
        VIR_Operand_SetTypeId(operand, VIR_TYPE_UINT16);
        VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);

        /* add instructions to get the offset of current group */
        VIR_Shader_AddString(virShader, "#sh_spv_groupoffset", &nameId);

        VIR_Shader_AddSymbol(
            virShader,
            VIR_SYM_VARIABLE,
            nameId,
            VIR_Shader_GetTypeFromId(virShader, VIR_TYPE_UINT32),
            VIR_STORAGE_GLOBAL,
            &symId
            );
        sym = VIR_Shader_GetSymFromId(virShader, symId);
        VIR_Symbol_SetPrecision(sym, VIR_PRECISION_HIGH);
        VIR_Symbol_SetTyQualifier(sym, VIR_TYQUAL_NONE);
        VIR_Symbol_SetLayoutQualifier(sym, VIR_LAYQUAL_NONE);
        VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_WITHOUT_REG);
        VIR_Symbol_SetLocation(sym, -1);

        VIR_Function_AddInstruction(spv->virFunction, VIR_OP_MUL, VIR_TYPE_UINT32, &virInst);

        operand = VIR_Inst_GetDest(virInst);
        VIR_Operand_SetSym(operand, sym);
        VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
        VIR_Operand_SetEnable(operand, VIR_TypeId_Conv2Enable(VIR_TYPE_UINT32));
        VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
        VIR_Operand_SetTypeId(operand, VIR_TYPE_UINT32);
        VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);

        operand = VIR_Inst_GetSource(virInst, 0);
        VIR_Operand_SetSym(operand, workgroupIdSym);
        VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
        VIR_Operand_SetSwizzle(operand, VIR_Enable_2_Swizzle_WShift(VIR_TypeId_Conv2Enable(VIR_TYPE_UINT16)));
        VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
        VIR_Operand_SetTypeId(operand, VIR_TYPE_UINT16);
        VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);

        /* The src1 should be set after the adding of ssbo member */

        SPV_WORKGROUP_INFO()->groupOffsetSymId = symId;
        SPV_WORKGROUP_INFO()->groupOffsetNameId = nameId;
        SPV_WORKGROUP_INFO()->offsetCalVirInst = virInst;
    }

    return VSC_ERR_NONE;
}

static gctBOOL __SpvIsInvalidEntryId(gcSPV spv, SpvId target)
{
    gctBOOL isInvalidEntry = gcvFALSE;
    gctUINT i;

    for (i = 0; i < spv->invalidEntryCount; i++)
    {
        if (target == spv->invalidEntryId[i])
        {
            isInvalidEntry = gcvTRUE;
            break;
        }
    }

    return isInvalidEntry;
}

static VSC_ErrCode __SpvAddFunction(gcSPV spv, VIR_Shader * virShader)
{
    VIR_Function *virFunction;
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VIR_Instruction * virInst = gcvNULL;
    VIR_NameId funcNameId;
    gctCHAR * name;
    gctBOOL isMain = gcvFALSE;
    gctBOOL isKernel = gcvFALSE;
    SpvFunctionControlMask funcControl = spv->operands[0];
    SpvId funcTypeId = spv->operands[1];
    gctUINT i;
    SpvId retSpvType;

    /* do dynamic size check if needed */
    SPV_CHECK_FUNC_CALLER(spv, spv->resultId);

    SPV_ID_TYPE(spv->resultId) = SPV_ID_TYPE_FUNC_DEFINE;

    /* func control */
    SPV_ID_FUNC_CONTROL(spv->resultId) = funcControl;
    /* func ID */
    SPV_ID_FUNC_TYPE_ID(spv->resultId) = funcTypeId;

    SPV_ID_FUNC_LABEL(spv->resultId) = SPV_INVALID_ID;

    SPV_ID_FUNC_VIR_LABEL(spv->resultId) = gcvNULL;

    SPV_ID_FUNC_RETURN_SPV_TYPE(spv->resultId) = spv->resultTypeId;

    retSpvType = SPV_ID_FUNC_RETURN_SPV_TYPE(spv->resultId);

    if (spv->spvSpecFlag & SPV_SPECFLAG_ENTRYPOINT)
    {
        name = gcvNULL;
        funcNameId = SPV_ID_VIR_NAME_ID(spv->resultId);
        if (funcNameId != VIR_INVALID_ID)
        {
            name = VIR_Shader_GetStringFromId(virShader, funcNameId);
        }

        if (!SPV_IS_EMPTY_STRING(name))
        {
            gctCHAR castName[SPV_VIR_NAME_SIZE];
            gctSIZE_T castNameLen = 0;
            gctUINT i;

            /* if this function name is main, we need skip it */
            if ((gcoOS_StrCmp(name, "main") == gcvSTATUS_OK))
            {
                spv->isInValidArea = gcvFALSE;
                return virErrCode;
            }

            gcoOS_MemFill(castName, 0, gcmSIZEOF(gctCHAR) * SPV_VIR_NAME_SIZE);
            gcoOS_StrLen(name, &castNameLen);

            /* if we met (, suppose the function name is end */
            for (i = 0; i < (gctUINT)castNameLen; i++)
            {
                if (name[i] == '(')
                {
                    break;
                }
                castName[i] = name[i];
            }

            /* if this function name is our special entry point, we set this to entry point */
            if ((!SPV_IS_EMPTY_STRING(spv->entryPointName)) &&
                gcoOS_StrCmp(castName, spv->entryPointName) == gcvSTATUS_OK)
            {
                spv->entryID = spv->resultId;
            }
        }
    }

    /* End the init function. */
    if (spv->initFunction == spv->virFunction)
    {
        virErrCode = VIR_Function_AddInstruction(spv->virFunction,
            SPV_OPCODE_2_VIR_OPCODE(SpvOpReturn),
            VIR_TYPE_VOID,
            &virInst);
        if (virErrCode != VSC_ERR_NONE)
        {
            return virErrCode;
        }

        spv->virFunction = gcvNULL;
    }

    if (spv->resultId == spv->entryID)
    {
        if (spv->entryExeMode != SpvExecutionModelKernel)
        {
            name = "main";
            isMain = gcvTRUE;
        }
        else
        {
            name = spv->entryPointName;
            isKernel = gcvTRUE;
        }
    }
    else if (__SpvIsInvalidEntryId(spv, spv->resultId))
    {
        /* This is a invalid entry point, we skip this function */
        spv->isInValidArea = gcvFALSE;
        return virErrCode;
    }
    else
    {
        funcNameId = SPV_ID_VIR_NAME_ID(spv->resultId);
        if (funcNameId == VIR_INVALID_ID)
        {
            __SpvGenerateFuncName(spv, spv->resultId);
            VIR_Shader_AddString(virShader, spv->virName, &funcNameId);
        }
        name = VIR_Shader_GetStringFromId(virShader, funcNameId);
    }

    /* VIR add function, TO_DO< iskernel????? */
    VIR_Shader_AddFunction(virShader,
                           isKernel,
                           name,
                           SPV_ID_VIR_TYPE_ID(retSpvType),
                           &virFunction);

    spv->virFunction = virFunction;

    if (isMain)
    {
        spv->virFunction->flags |= VIR_FUNCFLAG_MAIN;

        __SpvInsertInstAtFuncBegin(spv, virShader);
    }

    if (!SPV_ID_TYPE_IS_VOID(retSpvType))
    {
        VIR_Symbol * sym;
        VIR_TypeId virTypeId;
        VIR_NameId  retValueNameId = VIR_INVALID_ID;
        VIR_SymId symId;

        /* Allocate return symbol */
        __SpvGenerateFuncReturnName(spv, name);
        VIR_Shader_AddString(virShader, spv->virName, &retValueNameId);
        if (SPV_ID_TYPE_IS_POINTER(retSpvType))
        {
            virTypeId = SPV_ID_TYPE_VIR_TYPE_ID(SPV_ID_TYPE_POINTER_OBJECT_SPV_TYPE(retSpvType));
        }
        else
        {
            virTypeId = SPV_ID_TYPE_VIR_TYPE_ID(retSpvType);
        }

        virErrCode = VIR_Function_AddParameter(
            spv->virFunction,
            spv->virName,
            virTypeId,
            VIR_STORAGE_OUTPARM,
            &symId);

        sym = VIR_Function_GetSymFromId(spv->virFunction, symId);
        VIR_Symbol_SetFlag(sym,VIR_SYMFLAG_WITHOUT_REG);
        VIR_Symbol_SetPrecision(sym, VIR_PRECISION_HIGH);
        SPV_ID_FUNC_VIR_RET_SYM(spv->resultId) = sym;
    }
    else
    {
        SPV_ID_FUNC_VIR_RET_SYM(spv->resultId) = gcvNULL;
    }

    /* Check if we have allocate the parameters before(in OpFunctionCall). */
    if (SPV_ID_FUNC_ARG_NUM(spv->resultId) != 0)
    {
        gcmASSERT(SPV_ID_FUNC_ARG_NUM(spv->resultId) == SPV_ID_TYPE_FUNC_ARG_NUM(spv->operands[1]));
    }
    else
    {
        SPV_ID_FUNC_ARG_NUM(spv->resultId) = SPV_ID_TYPE_FUNC_ARG_NUM(spv->operands[1]);
        if (SPV_ID_FUNC_ARG_NUM(spv->resultId) > 0)
        {
            spvAllocate(spv->spvMemPool, sizeof(SpvId) * SPV_ID_FUNC_ARG_NUM(spv->resultId), (gctPOINTER *)(&SPV_ID_FUNC_SPV_ARG(spv->resultId)));
            spvAllocate(spv->spvMemPool, sizeof(SpvParameterStorage) * SPV_ID_FUNC_ARG_NUM(spv->resultId), (gctPOINTER *)(&SPV_ID_FUNC_ARG_STORAGE(spv->resultId)));

            for(i = 0; i < SPV_ID_FUNC_ARG_NUM(spv->resultId); i++)
            {
                SPV_ID_FUNC_SPV_ARG(spv->resultId)[i] = SPV_INVALID_ID;
                SPV_ID_FUNC_ARG_STORAGE(spv->resultId)[i] = SPV_PARA_NONE;
            }
        }
    }
    SPV_ID_FUNC_VIR_FUNCION(spv->resultId) = spv->virFunction;
    spv->func = spv->resultId;
    spv->argIndex = 0;

    return virErrCode;
}

static VSC_ErrCode __SpvAddFunctionEnd(gcSPV spv, VIR_Shader * virShader)
{
    VIR_SymId               retSymId;
    VIR_Function           *func = spv->virFunction;
    VIR_VariableIdList     *paramList = VIR_Function_GetParameters(func);

    /* Move the return value symbol to the last parameter. */
    if (SPV_ID_FUNC_VIR_RET_SYM(spv->func) != gcvNULL)
    {
        retSymId = VIR_Symbol_GetIndex(SPV_ID_FUNC_VIR_RET_SYM(spv->func));
        VIR_IdList_DeleteByValue(paramList, retSymId);
        VIR_IdList_Add(paramList, retSymId);
    }

    spv->virFunction = gcvNULL;
    spv->func = SPV_INVALID_ID;
    spv->argIndex = 0;
    return VSC_ERR_NONE;
}

struct SPV_2_VIR_OPERAND
{
    gctUINT                 virOpndKind;
    gctUINT                 virRoundMode;
    gctUINT                 virModifier;
    VIR_TypeId              virOpndType;

    gctUINT                 virSwizzleOrEnable;
    gctUINT                 virPrecision;

    union
    {
        VIR_SymId           sym;
    } u1;
    union
    {
        struct
        {
            gctUINT         isConstIndexing;
            gctUINT         relAddrMode;
            gctUINT         matrixConstIndex;
            gctINT          relIndexing;
            gctUINT         relAddrLevel;
        } vlInfo;
    } u2;
};

typedef struct _SPV_2_VIR_INSTRUCTION
{
    VIR_OpCode                          virOpcode;
    gctUINT                             virCondOp;
    gctUINT                             spvSrcOpndNum;
    VIR_TypeId                          virOpTypeId;
    struct SPV_2_VIR_OPERAND            dest;
    struct SPV_2_VIR_OPERAND            src[VIR_MAX_SRC_NUM];
}
SPV_2_VIR_INSTRUCTN;

#define gcmEMIT_HEADER() \
    VSC_ErrCode virErrCode = VSC_ERR_NONE; \
    VIR_OpCode virOpcode = SPV_OPCODE_2_VIR_OPCODE(spv->opCode); \
    SpvId resultId = spv->resultId; \
    SpvId resultTypeId = spv->resultTypeId; \
    VIR_Type *  dstVirType = gcvNULL; \
    VIR_TypeId dstVirTypeId = VIR_TYPE_UNKNOWN; \
    VIR_Instruction * virInst; \
    VIR_Operand * operand; \
    VIR_NameId nameId; \
    VIR_SymId virSymId; \
    VIR_Symbol * dstVirSym = gcvNULL;

#define gcmEMIT_GET_ARGS() \
    dstVirTypeId = SPV_ID_TYPE_VIR_TYPE_ID(resultTypeId); \
    dstVirType = SPV_ID_TYPE_VIR_TYPE(resultTypeId); \
    nameId = SPV_ID_VIR_NAME_ID(resultId); \
    if (nameId == VIR_INVALID_ID) \
        { \
        __SpvAddIdSymbol(spv, virShader, gcvNULL, resultId, resultTypeId, VIR_SYM_VARIABLE, VIR_STORAGE_GLOBAL, gcvFALSE); \
        } \
    virSymId = SPV_ID_VIR_SYM_ID(resultId); \
    dstVirSym = VIR_Shader_GetSymFromId(virShader, virSymId);

/* for SpvOpAccessChain */
static VSC_ErrCode __SpvEmitAccessChain(gcSPV spv, VIR_Shader * virShader)
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    gctUINT src;
    VIR_Symbol * baseSymbol;
    gctUINT *accessChain = gcvNULL;
    VIR_SymbolKind *accessChainType = gcvNULL;
    gctUINT accessChainLength;
    gctUINT i = 0;
    VIR_AC_OFFSET_INFO virAcOffsetInfo;
    gctBOOL copyFromObject = (spv->opCode == SpvOpCompositeInsert);
    gctBOOL isAllIndexLiteral = (spv->opCode == SpvOpCompositeInsert);

    gcoOS_ZeroMemory(&virAcOffsetInfo, gcmSIZEOF(VIR_AC_OFFSET_INFO));

    src = spv->operands[0];
    if (!copyFromObject)
    {
        SPV_ID_CLONE(src, spv->resultId);
    }
    baseSymbol = SPV_ID_VIR_SYM(src);

    /* Create the index array. */
    accessChainLength = spv->operandSize - 1;
    spvAllocate(spv->spvMemPool, accessChainLength * gcmSIZEOF(gctUINT), (gctPOINTER *)&accessChain);
    gcoOS_ZeroMemory(accessChain, accessChainLength * gcmSIZEOF(gctUINT));
    spvAllocate(spv->spvMemPool, accessChainLength * gcmSIZEOF(VIR_SymbolKind), (gctPOINTER *)&accessChainType);
    gcoOS_ZeroMemory(accessChainType, accessChainLength * gcmSIZEOF(VIR_SymbolKind));

    /* Set the index type and value. */
    for (i = 1; i < spv->operandSize; i++)
    {
        if (isAllIndexLiteral)
        {
            accessChainType[i - 1] = VIR_SYM_CONST;
            accessChain[i - 1] = spv->operands[i];
        }
        else if ( SPV_ID_TYPE(spv->operands[i]) == SPV_ID_TYPE_CONST)
        {
            accessChainType[i - 1] = VIR_SYM_CONST;
            accessChain[i - 1] = SPV_ID_VIR_CONST(spv->operands[i]).scalarVal.uValue;
        }
        else
        {
            accessChainType[i - 1] = VIR_SYM_VARIABLE;
            accessChain[i - 1] = SPV_ID_VIR_SYM_ID(spv->operands[i]);
        }
    }

    /* Evaluate offset. */
    VIR_Operand_EvaluateOffsetByAccessChain(
        virShader,
        spv->virFunction,
        spv->resultId,
        baseSymbol,
        accessChain,
        accessChainType,
        accessChainLength,
        &virAcOffsetInfo);

    SPV_SET_IDDESCRIPTOR_SPV_OFFSET(spv, spv->resultId, virAcOffsetInfo);
    if (!copyFromObject)
    {
        SPV_ID_SYM_SRC_SPV_TYPE(spv->resultId) = SPV_ID_SYM_SPV_TYPE(spv->resultId);
        SPV_ID_SYM_SPV_TYPE(spv->resultId) = spv->resultTypeId;
        SPV_ID_TYPE_VIR_TYPE_ID(spv->resultId) = SPV_ID_TYPE_VIR_TYPE_ID(SPV_ID_TYPE_POINTER_OBJECT_SPV_TYPE(spv->resultTypeId));
    }

    spvFree(gcvNULL, accessChain);
    spvFree(gcvNULL, accessChainType);

    return virErrCode;
}

/* for SpvOpEmitVertex and SpvOpEndPrimitive */
static VSC_ErrCode __SpvEmitVertexPrimitive(gcSPV spv, VIR_Shader * virShader)
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VIR_OpCode virOpcode = SPV_OPCODE_2_VIR_OPCODE(spv->opCode);
    VIR_Instruction * virInst;

    VIR_Function_AddInstruction(spv->virFunction,
        virOpcode,
        VIR_TYPE_UNKNOWN,
        &virInst);

    VIR_Inst_SetConditionOp(virInst, VIR_COP_ALWAYS);

    return virErrCode;
}

static VSC_ErrCode __SpvEmitVectorShuffle(gcSPV spv, VIR_Shader * virShader)
{
    gctUINT i = 0;
    gctUINT emitRound = 0;
    SpvId resultSpvTypeId[2] = { 0, 0 };

    gcmEMIT_HEADER();

    gcmASSERT(resultId);

    gcmEMIT_GET_ARGS();

    for (i = 0; i < 2; i++)
    {
        if (SPV_ID_TYPE(spv->operands[i]) == SPV_ID_TYPE_SYMBOL)
        {
            resultSpvTypeId[i] = SPV_ID_SYM_SPV_TYPE(spv->operands[i]);
        }
        else if (SPV_ID_TYPE(spv->operands[i]) == SPV_ID_TYPE_CONST)
        {
            resultSpvTypeId[i] = SPV_ID_CST_SPV_TYPE(spv->operands[i]);
        }
        else if (SPV_ID_TYPE(spv->operands[i]) == SPV_ID_TYPE_FUNC_DEFINE)
        {
            resultSpvTypeId[i] = SPV_ID_FUNC_TYPE_ID(spv->operands[i]);
        }
        else
        {
            gcmASSERT(0);
        }
    }

    if (SPV_ID_TYPE_IS_VECTOR(resultSpvTypeId[0]) &&
        SPV_ID_TYPE_IS_VECTOR(resultSpvTypeId[1]))
    {
        /* split vector shuffle into 2 mov instruction */
        VIR_TypeId typeId;
        VIR_Swizzle srcSwizzle[2] = { VIR_SWIZZLE_X, VIR_SWIZZLE_X };
        VIR_Enable dstEnable[2] = { VIR_ENABLE_NONE, VIR_ENABLE_NONE };
        gctUINT srcVec0CompNum = SPV_ID_TYPE_VEC_COMP_NUM(resultSpvTypeId[0]);
        gctUINT src0Component[4] = { 0 };
        gctUINT src1Component[4] = { 0 };
        gctUINT componentCount[2] = {0, 0};
        gctBOOL bHasUndef = gcvFALSE;

        emitRound = (spv->operands[0] == spv->operands[1] ? 1 : 2);

        /* determine component */
        for (i = 2; i < spv->operandSize; i++)
        {
            if (spv->operands[i] == 0xFFFFFFFF)
            {
                bHasUndef = gcvTRUE;
            }
            else if (spv->operands[i] < srcVec0CompNum)
            {
                dstEnable[0] |= virEnable[i - 2];
                src0Component[componentCount[0]] = spv->operands[i];
                componentCount[0]++;
            }
            else
            {
                /* operands exceed the vec1's component number is vec2's component */
                dstEnable[1] |= virEnable[i - 2];
                src1Component[componentCount[1]] = spv->operands[i] - srcVec0CompNum;
                componentCount[1]++;
            }
        }

        /* copy the last swizzle */
        if (componentCount[0] > 0)
        {
            for (i = componentCount[0]; i < 4; i++)
            {
                src0Component[i] = src0Component[componentCount[0] - 1];
            }
        }
        if (componentCount[1] > 0)
        {
            for (i = componentCount[1]; i < 4; i++)
            {
                src1Component[i] = src1Component[componentCount[1] - 1];
            }
        }

        /* generate swizzle */
        srcSwizzle[0] = src0Component[0] << 0 | src0Component[1] << 2 | src0Component[2] << 4 | src0Component[3] << 6;
        srcSwizzle[1] = src1Component[0] << 0 | src1Component[1] << 2 | src1Component[2] << 4 | src1Component[3] << 6;

        srcSwizzle[0] = VIR_Swizzle_SwizzleWShiftEnable(srcSwizzle[0], dstEnable[0]);
        srcSwizzle[1] = VIR_Swizzle_SwizzleWShiftEnable(srcSwizzle[1], dstEnable[1]);

        for (i = 0; i < emitRound; i++)
        {
            SpvId elementType = SPV_ID_TYPE_VEC_COMP_TYPE(SPV_ID_SYM_SPV_TYPE(spv->resultId));

            if (componentCount[i] == 0)
            {
                continue;
            }

            /* Get vector element base type id */
            typeId = SPV_ID_VIR_TYPE_ID(elementType);

            typeId = virVector[spvVecMagicBase[componentCount[i]] + __GetMagicOffsetByTypeId(typeId)];

            VIR_Function_AddInstruction(spv->virFunction,
                virOpcode,
                typeId,
                &virInst);

            VIR_Inst_SetConditionOp(virInst, VIR_COP_ALWAYS);
            operand = VIR_Inst_GetSource(virInst, 0);
            /* we need to consider the swizzle with shift */
            VIR_Operand_SetSwizzle(operand, srcSwizzle[i]);
            VIR_Operand_SetTypeId(operand, typeId);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            if (SPV_ID_TYPE(spv->operands[i]) == SPV_ID_TYPE_SYMBOL)
            {
                VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
                VIR_Operand_SetSym(operand, SPV_ID_VIR_SYM(spv->operands[i]));
            }
            else
            {
                VIR_Operand_SetOpKind(operand, VIR_OPND_CONST);
                VIR_Operand_SetConstId(operand, SPV_ID_VIR_CONST_ID(spv->operands[i]));
            }

            operand = VIR_Inst_GetDest(virInst);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            VIR_Operand_SetEnable(operand, dstEnable[i]);
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetTypeId(operand, typeId);
            VIR_Operand_SetSym(operand, dstVirSym);
        }
    }
    else
    {
        gcmASSERT(0);
    }

    return virErrCode;
}

static VSC_ErrCode __SpvEmitPhi(gcSPV spv, VIR_Shader * virShader)
{
    VIR_Swizzle virSwizzle;
    VIR_Enable virEnableMask;
    gctUINT i;
    VIR_PhiOperand * phiOperand;

    gcmEMIT_HEADER();

    gcmEMIT_GET_ARGS();

    virEnableMask = __SpvGenEnable(spv, dstVirType, resultTypeId);

    VIR_Function_AddInstruction(spv->virFunction,
        virOpcode,
        SPV_ID_VIR_TYPE_ID(resultId),
        &virInst);
    VIR_Function_AddPhiOperandArrayForInst(spv->virFunction, virInst, spv->operandSize / 2);

    VIR_Inst_SetConditionOp(virInst, __SpvOpCode2VIRCop(spv->opCode));

    VIR_Shader_SetFlag(virShader, VIR_SHFLAG_BY_SPV_SSA_FORM);

    operand = VIR_Inst_GetDest(virInst);
    VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
    VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
    VIR_Operand_SetEnable(operand, virEnableMask);
    VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
    VIR_Operand_SetTypeId(operand, dstVirTypeId);
    VIR_Operand_SetSym(operand, dstVirSym);
    __SetAccessChainOffsetToOperand(spv, spv->resultId, operand, SpvOffsetType_Normal);

    operand = VIR_Inst_GetSource(virInst, 0);
    VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
    VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
    VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
    VIR_Operand_SetTypeId(operand, SPV_ID_VIR_TYPE_ID(resultTypeId));

    for (i = 0; 2 * i < spv->operandSize; i++)
    {
        gctUINT label = spv->operands[i * 2 + 1];
        gctUINT spvOperand = spv->operands[i * 2];
        VIR_LabelId labelId;
        VIR_Label * virLabel;
        VIR_Operand * newOperands;

        virErrCode = VIR_Function_NewOperand(spv->virFunction, &newOperands);
        if (virErrCode != VSC_ERR_NONE) return virErrCode;

        phiOperand = VIR_Operand_GetNthPhiOperand(operand, i);

        if (SPV_ID_TYPE(spvOperand) != SPV_ID_TYPE_UNKNOW)
        {
            virSwizzle = __SpvID2Swizzle(spv, spvOperand);

            if (SPV_ID_TYPE(spvOperand) == SPV_ID_TYPE_SYMBOL)
            {
                VIR_Symbol* sym = SPV_ID_VIR_SYM(spvOperand);
                VIR_Operand_SetSym(newOperands, sym);
                VIR_Operand_SetOpKind(newOperands, VIR_OPND_SYMBOL);
                VIR_Operand_SetTypeId(newOperands, SPV_ID_VIR_TYPE_ID(resultTypeId));
                VIR_Operand_SetPrecision(newOperands, VIR_PRECISION_HIGH);
                VIR_Operand_SetSwizzle(newOperands, virSwizzle);
                VIR_Operand_SetRoundMode(newOperands, VIR_ROUND_DEFAULT);
                VIR_Operand_SetModifier(newOperands, VIR_MOD_NONE);
            }
            else if (SPV_ID_TYPE(spvOperand) == SPV_ID_TYPE_CONST)
            {
                VIR_Operand_SetSwizzle(newOperands, virSwizzle);
                VIR_Operand_SetConstId(newOperands, SPV_ID_VIR_CONST_ID(spvOperand));
                VIR_Operand_SetOpKind(newOperands, VIR_OPND_CONST);
                VIR_Operand_SetTypeId(newOperands, SPV_ID_VIR_TYPE_ID(resultTypeId));
                VIR_Operand_SetPrecision(newOperands, VIR_PRECISION_HIGH);
                VIR_Operand_SetRoundMode(newOperands, VIR_ROUND_DEFAULT);
                VIR_Operand_SetModifier(newOperands, VIR_MOD_NONE);
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }
        }
        else
        {
            /* Check the unhandled variabled in _SpvCheckUnhandleVariables. */
            SPV_SET_UNHANDLE_VAR_OPERAND(spv, spvOperand, virInst, newOperands);
        }

        /* TODO: the initialization should in SPV or VSC? */
        VIR_PhiOperand_SetValue(phiOperand, newOperands);
        VIR_PhiOperand_SetLabel(phiOperand, gcvNULL);
        VIR_PhiOperand_SetFlags(phiOperand, 0);

        /* Set label, the label may not defined, we need delay the setting */
        if (SPV_ID_TYPE(label) == SPV_ID_TYPE_LABEL)
        {
            labelId = SPV_ID_VIR_NAME_ID(label);
            gcmASSERT(labelId != VIR_INVALID_ID);

            virLabel = VIR_Function_GetLabelFromId(spv->virFunction, labelId);
            VIR_PhiOperand_SetLabel(phiOperand, virLabel);
        }
        else
        {
            SPV_SET_UNHANDLE_LABEL_PHI(spv, label, phiOperand);
        }
    }

    return virErrCode;
}

static VSC_ErrCode __SpvEmitCompositeExtract(gcSPV spv, VIR_Shader * virShader)
{
    SpvId spvTypeId = __SpvGetResultTypeId(spv, spv->operands[0]);
    VIR_Enable virEnable;
    gctBOOL isNested = gcvFALSE;

    gcmEMIT_HEADER();

    gcmASSERT(resultId);
    gcmASSERT(spvTypeId != SPV_INVALID_ID);

    gcmEMIT_GET_ARGS();

    (void)dstVirType;

    /* If the composite is a matrix, then it must be a variable. */
    if (SPV_ID_TYPE_IS_VECTOR(spvTypeId))
    {
        VIR_Function_AddInstruction(spv->virFunction,
            virOpcode,
            dstVirTypeId,
            &virInst);

        VIR_Inst_SetConditionOp(virInst, VIR_COP_ALWAYS);
        operand = VIR_Inst_GetSource(virInst, 0);

        virEnable = VIR_TypeId_Conv2Enable(dstVirTypeId);

        if (SPV_ID_TYPE(spv->operands[0]) == SPV_ID_TYPE_SYMBOL)
        {
            if (SPV_ID_TYPE(spv->operands[1]) == SPV_ID_TYPE_CONST)
            {
                /* index should be integer or unsigned-integer */
                gctUINT vectorIndex = SPV_CONST_SCALAR_INT(spv->operands[1]);
                VIR_Operand_SetSwizzle(operand, __SpvConstIndexToVIRSwizzle(vectorIndex));
            }
            else
            {
                /* this is a immediate number */
                VIR_Operand_SetSwizzle(operand, __SpvConstIndexToVIRSwizzle(spv->operands[1]));
            }
            VIR_Operand_SetSym(operand, SPV_ID_VIR_SYM(spv->operands[0]));
            VIR_Operand_SetTypeId(operand, SPV_ID_TYPE_VIR_TYPE_ID(SPV_ID_TYPE_VEC_COMP_TYPE(SPV_ID_SYM_SPV_TYPE(spv->operands[0]))));
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
        }
        else if (SPV_ID_TYPE(spv->operands[0]) == SPV_ID_TYPE_CONST)
        {
            VIR_ScalarConstVal constVal;
            VIR_TypeId componentType = VIR_GetTypeComponentType(SPV_ID_TYPE_VIR_TYPE_ID(spv->operands[0]));

            constVal.uValue = 0;

            if (componentType == VIR_TYPE_FLOAT32)
            {
                constVal.fValue = SPV_ID_VIR_CONST(spv->operands[0]).vecVal.f32Value[spv->operands[1]];
            }
            else if (componentType == VIR_TYPE_INT32)
            {
                constVal.iValue = SPV_ID_VIR_CONST(spv->operands[0]).vecVal.i32Value[spv->operands[1]];
            }
            else if (componentType == VIR_TYPE_INT16)
            {
                constVal.iValue = SPV_ID_VIR_CONST(spv->operands[0]).vecVal.i16Value[spv->operands[1]];
            }
            else if (componentType == VIR_TYPE_INT8)
            {
                constVal.iValue = SPV_ID_VIR_CONST(spv->operands[0]).vecVal.i8Value[spv->operands[1]];
            }
            else if (componentType == VIR_TYPE_UINT32)
            {
                constVal.uValue = SPV_ID_VIR_CONST(spv->operands[0]).vecVal.u32Value[spv->operands[1]];
            }
            else if (componentType == VIR_TYPE_UINT16)
            {
                constVal.uValue = SPV_ID_VIR_CONST(spv->operands[0]).vecVal.u16Value[spv->operands[1]];
            }
            else if (componentType == VIR_TYPE_UINT8)
            {
                constVal.uValue = SPV_ID_VIR_CONST(spv->operands[0]).vecVal.u8Value[spv->operands[1]];
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }
            VIR_Operand_SetImmediate(operand, componentType, constVal);
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }

        operand = VIR_Inst_GetDest(virInst);
        VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
        VIR_Operand_SetEnable(operand, virEnable);
        VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
        VIR_Operand_SetTypeId(operand, dstVirTypeId);
        VIR_Operand_SetSym(operand, dstVirSym);
    }
    /* If the composite is a matrix, then it must be a variable. */
    else if (SPV_ID_TYPE_IS_MATRIX(spvTypeId))
    {
        virEnable = VIR_TypeId_Conv2Enable(dstVirTypeId);

        if (SPV_ID_TYPE_IS_FLOAT(spv->resultTypeId))
        {
            /* operandsize must be 3 to extract a float from matrix */
            gcmASSERT(spv->operandSize == 3);

            VIR_Function_AddInstruction(spv->virFunction,
                virOpcode,
                dstVirTypeId,
                &virInst);

            VIR_Inst_SetConditionOp(virInst, VIR_COP_ALWAYS);
            operand = VIR_Inst_GetSource(virInst, 0);
            VIR_Operand_SetSwizzle(operand, __SpvConstIndexToVIRSwizzle(spv->operands[2]));
            VIR_Operand_SetSym(operand, SPV_ID_VIR_SYM(spv->operands[0]));
            VIR_Operand_SetTypeId(operand, SPV_ID_TYPE_VIR_TYPE_ID(SPV_ID_TYPE_MAT_COL_TYPE(SPV_ID_SYM_SPV_TYPE(spv->operands[0]))));
            VIR_Operand_SetMatrixConstIndex(operand, spv->operands[1]);
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);

            operand = VIR_Inst_GetDest(virInst);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            VIR_Operand_SetEnable(operand, virEnable);
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetTypeId(operand, dstVirTypeId);
            VIR_Operand_SetSym(operand, dstVirSym);
        }
        else if (SPV_ID_TYPE_IS_VECTOR(spv->resultTypeId))
        {
            /* first operand is vector index, second operand is scalar index */
            gcmASSERT(spv->operandSize == 2);

            VIR_Function_AddInstruction(spv->virFunction,
                virOpcode,
                dstVirTypeId,
                &virInst);

            VIR_Inst_SetConditionOp(virInst, VIR_COP_ALWAYS);
            operand = VIR_Inst_GetSource(virInst, 0);
            VIR_Operand_SetSwizzle(operand, VIR_Enable_2_Swizzle_WShift(virEnable));
            VIR_Operand_SetSym(operand, SPV_ID_VIR_SYM(spv->operands[0]));
            VIR_Operand_SetTypeId(operand, SPV_ID_TYPE_VIR_TYPE_ID(SPV_ID_TYPE_MAT_COL_TYPE(SPV_ID_SYM_SPV_TYPE(spv->operands[0]))));
            VIR_Operand_SetMatrixConstIndex(operand, spv->operands[1]);
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);

            operand = VIR_Inst_GetDest(virInst);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            VIR_Operand_SetEnable(operand, virEnable);
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetTypeId(operand, dstVirTypeId);
            VIR_Operand_SetSym(operand, dstVirSym);
        }
        else
        {
            /* result type is not float, we need handle more */
            gcmASSERT(0);
        }
    }
    /* If the composite is an array or a struct, then it must be a variable. */
    else if (SPV_ID_TYPE_IS_ARRAY(spvTypeId) ||
             SPV_ID_TYPE_IS_STRUCT(spvTypeId))
    {
        gctUINT src;
        VIR_Symbol * baseSymbol;
        gctUINT *accessChain = gcvNULL;
        VIR_SymbolKind *accessChainType = gcvNULL;
        gctUINT accessChainLength;
        gctUINT i = 0;
        VIR_AC_OFFSET_INFO virAcOffsetInfo;

        gcoOS_ZeroMemory(&virAcOffsetInfo, gcmSIZEOF(VIR_AC_OFFSET_INFO));

        /* The souce must be a variable symbol. */
        gcmASSERT(SPV_ID_TYPE(spv->operands[0]) == SPV_ID_TYPE_SYMBOL);

        src = spv->operands[0];
        SPV_ID_CLONE(src, spv->resultId);
        baseSymbol = SPV_ID_VIR_SYM(spv->operands[0]);

        /* Create the index array. */
        accessChainLength = spv->operandSize - 1;
        spvAllocate(spv->spvMemPool, accessChainLength * gcmSIZEOF(gctUINT), (gctPOINTER *)&accessChain);
        gcoOS_ZeroMemory(accessChain, accessChainLength * gcmSIZEOF(gctUINT));
        spvAllocate(spv->spvMemPool, accessChainLength * gcmSIZEOF(VIR_SymbolKind), (gctPOINTER *)&accessChainType);
        gcoOS_ZeroMemory(accessChainType, accessChainLength * gcmSIZEOF(VIR_SymbolKind));

        /* check its base offset to see if this is a nested type */
        if ((SPV_ID_SYM_OFFSET_TYPE(src) == VIR_SYM_CONST ||
            SPV_ID_SYM_OFFSET_TYPE(src) == VIR_SYM_VARIABLE) &&
            SPV_ID_SYM_OFFSET_VALUE(src) != VIR_INVALID_ID)
        {
            gctUINT offset = 0;
            VIR_SymId nestSymId;

            gcoOS_MemFill(spv->virName, 0, SPV_VIR_NAME_SIZE * gcmSIZEOF(gctCHAR));
            gcoOS_PrintStrSafe(spv->virName, SPV_VIR_NAME_SIZE, &offset, "#sh_spv_nested_%d", spv->resultId);
            VIR_Shader_AddString(virShader, spv->virName, &nameId);

            VIR_Shader_AddSymbol(
                virShader,
                VIR_SYM_VARIABLE,
                nameId,
                SPV_ID_VIR_TYPE(src),
                VIR_STORAGE_GLOBAL,
                &nestSymId);

            baseSymbol = VIR_Shader_GetSymFromId(virShader, nestSymId);
            isNested = gcvTRUE;

            VIR_Symbol_SetPrecision(baseSymbol, VIR_PRECISION_HIGH);
            VIR_Symbol_SetTyQualifier(baseSymbol, VIR_TYQUAL_NONE);
            VIR_Symbol_SetLayoutQualifier(baseSymbol, VIR_LAYQUAL_NONE);
            VIR_Symbol_SetLocation(baseSymbol, -1);
            if (!(VIR_Symbol_isUniform(baseSymbol) ||
                VIR_Symbol_isSampler(baseSymbol) ||
                VIR_Symbol_isTexure(baseSymbol) ||
                VIR_Symbol_isImage(baseSymbol)))
            {
                VIR_Symbol_SetFlag(baseSymbol, VIR_SYMFLAG_WITHOUT_REG);
            }
        }

        /* Set the index type and value. */
        for (i = 1; i < spv->operandSize; i++)
        {
            accessChainType[i - 1] = VIR_SYM_CONST;
            accessChain[i - 1] = spv->operands[i];
        }

        /* Evaluate offset. */
        VIR_Operand_EvaluateOffsetByAccessChain(
            virShader,
            spv->virFunction,
            spv->resultId,
            baseSymbol,
            accessChain,
            accessChainType,
            accessChainLength,
            &virAcOffsetInfo);

        if (isNested)
        {
            /* we need add back the offset to parent struct */
            VIR_SymId orgBaseOffset = SPV_ID_SYM_OFFSET_VALUE(src);
            VIR_SymbolKind orgBaseOffsetType = SPV_ID_SYM_OFFSET_TYPE(src);
            VIR_SymId baseOffset = virAcOffsetInfo.baseOffset;
            VIR_SymbolKind baseOffsetType = virAcOffsetInfo.baseOffsetType;
            gctUINT offset = 0;
            VIR_SymId offsetSymId;
            VIR_Symbol * offsetSym;

            if (baseOffset == VIR_INVALID_ID)
            {
                baseOffset = 0;
            }

            if (orgBaseOffsetType == VIR_SYM_CONST && baseOffsetType == VIR_SYM_CONST)
            {
                virAcOffsetInfo.baseOffset = baseOffset + orgBaseOffset;
            }
            else
            {
                gcoOS_MemFill(spv->virName, 0, SPV_VIR_NAME_SIZE * gcmSIZEOF(gctCHAR));
                gcoOS_PrintStrSafe(spv->virName, SPV_VIR_NAME_SIZE, &offset, "#sh_spv_nested_offset_%d", spv->resultId);
                VIR_Shader_AddString(virShader, spv->virName, &nameId);

                VIR_Shader_AddSymbol(
                    virShader,
                    VIR_SYM_VARIABLE,
                    nameId,
                    VIR_Shader_GetTypeFromId(virShader, VIR_TYPE_UINT32),
                    VIR_STORAGE_GLOBAL,
                    &offsetSymId);

                offsetSym = VIR_Shader_GetSymFromId(virShader, offsetSymId);

                VIR_Symbol_SetPrecision(offsetSym, VIR_PRECISION_HIGH);
                VIR_Symbol_SetTyQualifier(offsetSym, VIR_TYQUAL_NONE);
                VIR_Symbol_SetLayoutQualifier(offsetSym, VIR_LAYQUAL_NONE);
                VIR_Symbol_SetLocation(offsetSym, -1);
                if (!(VIR_Symbol_isUniform(offsetSym) ||
                    VIR_Symbol_isSampler(offsetSym) ||
                    VIR_Symbol_isTexure(offsetSym) ||
                    VIR_Symbol_isImage(offsetSym)))
                {
                    VIR_Symbol_SetFlag(offsetSym, VIR_SYMFLAG_WITHOUT_REG);
                }

                VIR_Function_AddInstruction(
                    spv->virFunction,
                    VIR_OP_ADD,
                    VIR_TYPE_UINT32,
                    &virInst);

                VIR_Inst_SetConditionOp(virInst, VIR_COP_ALWAYS);

                operand = VIR_Inst_GetDest(virInst);
                VIR_Operand_SetSym(operand, offsetSym);
                VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
                VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
                VIR_Operand_SetEnable(operand, VIR_ENABLE_X);
                VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
                VIR_Operand_SetTypeId(operand, VIR_TYPE_UINT32);

                operand = VIR_Inst_GetSource(virInst, 0);
                VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_X);
                VIR_Operand_SetTypeId(operand, VIR_TYPE_UINT32);
                VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
                if (orgBaseOffsetType == VIR_SYM_VARIABLE)
                {
                    VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
                    VIR_Operand_SetSym(operand, VIR_Shader_GetSymFromId(virShader, orgBaseOffset));
                }
                else
                {
                    VIR_ScalarConstVal constVal;
                    constVal.uValue = orgBaseOffset;
                    VIR_Operand_SetImmediate(operand, VIR_TYPE_UINT32, constVal);
                }

                operand = VIR_Inst_GetSource(virInst, 1);
                VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_X);
                VIR_Operand_SetTypeId(operand, VIR_TYPE_UINT32);
                VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
                if (baseOffsetType == VIR_SYM_VARIABLE)
                {
                    VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
                    VIR_Operand_SetSym(operand, VIR_Shader_GetSymFromId(virShader, baseOffset));
                }
                else
                {
                    VIR_ScalarConstVal constVal;
                    constVal.uValue = baseOffset;
                    VIR_Operand_SetImmediate(operand, VIR_TYPE_UINT32, constVal);
                }

                /* save the result back to offset info */
                virAcOffsetInfo.baseOffset = offsetSymId;
                virAcOffsetInfo.baseOffsetType = VIR_SYM_VARIABLE;
            }
        }

        SPV_SET_IDDESCRIPTOR_SPV_OFFSET(spv, spv->resultId, virAcOffsetInfo);

        SPV_ID_SYM_SRC_SPV_TYPE(spv->resultId) = SPV_ID_SYM_SPV_TYPE(spv->resultId);
        SPV_ID_SYM_SPV_TYPE(spv->resultId) = spv->resultTypeId;
        SPV_ID_TYPE_VIR_TYPE_ID(spv->resultId) = SPV_ID_TYPE_VIR_TYPE_ID(spv->resultTypeId);

        spvFree(gcvNULL, accessChain);
        spvFree(gcvNULL, accessChainType);

        return virErrCode;
    }
    else if (SPV_ID_TYPE_IS_POINTER(spvTypeId))
    {
        /* TODO, pointer type, we need get object type */
        gcmASSERT(0);
    }
    else
    {
        gcmASSERT(0);
    }

    return virErrCode;
}

static VSC_ErrCode __SpvEmitCompositeInsert(gcSPV spv, VIR_Shader * virShader)
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    SpvId objectTypeId = 0;
    gctUINT objectOperandId = spv->operands[0];
    gctUINT i;

    /* I don't want to generate new symbol, let's clone*/
    /* SPV_ID_CLONE(spv->operands[1], spv->resultId); */
    __SpvIDCopy(spv, virShader, spv->operands[1], spv->resultId);

    /* Call OpAccessChain to get the object. */
    spv->operandSize--;
    for (i = 0; i < spv->operandSize; i++)
    {
        spv->operands[i] = spv->operands[i + 1];
    }

    __SpvEmitAccessChain(spv, virShader);

    /* Call OpLoad to update the modified part. */
    switch (SPV_ID_TYPE(objectOperandId))
    {
    case SPV_ID_TYPE_SYMBOL:
        objectTypeId = SPV_ID_SYM_SPV_TYPE(objectOperandId);
        break;

    case SPV_ID_TYPE_CONST:
        objectTypeId = SPV_ID_CST_SPV_TYPE(objectOperandId);
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    spv->operandSize = 1;
    spv->operands[0] = objectOperandId;
    spv->resultTypeId = objectTypeId;
    __SpvEmitInstructions(spv, virShader);

    /* Reset the offset. */
    SPV_RESET_IDDESCRIPTOR_SPV_OFFSET(spv, spv->resultId);

    return virErrCode;
}

static VSC_ErrCode __SpvEmitVectorExtractDynamic(gcSPV spv, VIR_Shader * virShader)
{
    VSC_ErrCode         virErrCode = VSC_ERR_NONE;
    SpvId               resultId = spv->resultId;
    SpvId               vectorId = spv->operands[0];
    SpvId               indexId = spv->operands[1];
    VIR_Symbol         *dstVirSym;
    VIR_Symbol         *sourceSym;
    VIR_Instruction    *virInst;
    VIR_Operand        *operand;
    VIR_TypeId          vectorTypeId, componentTypeId;
    gctUINT             componentCount;
    VIR_OpCode          virOpcode;

    /* Create the result variable. */
    if (SPV_ID_VIR_NAME_ID(resultId) == VIR_INVALID_ID)
    {
        __SpvAddIdSymbol(spv, virShader, gcvNULL, resultId, spv->resultTypeId, VIR_SYM_VARIABLE, VIR_STORAGE_GLOBAL, gcvFALSE);
    }

    /* Get the dest symbol. */
    dstVirSym = SPV_ID_VIR_SYM(resultId);

    /* Get the component type.*/
    vectorTypeId = SPV_ID_VIR_TYPE_ID(vectorId);
    componentTypeId = VIR_GetTypeComponentType(vectorTypeId);
    componentCount = VIR_GetTypeComponents(vectorTypeId);

    /* If the index ID is a constant, then just generate a MOV. */
    if (SPV_ID_TYPE(indexId) == SPV_ID_TYPE_CONST)
    {
        VIR_ConstId     constId = SPV_ID_VIR_CONST_ID(indexId);
        VIR_Const      *constant = VIR_Shader_GetConstFromId(virShader, constId);
        gctUINT         indexValue = constant->value.scalarVal.uValue;

        virOpcode = VIR_OP_MOV;
        VIR_Function_AddInstruction(spv->virFunction,
                                    virOpcode,
                                    componentTypeId,
                                    &virInst);

        /* Set DEST. */
        operand = VIR_Inst_GetDest(virInst);
        VIR_Operand_SetSym(operand, dstVirSym);
        VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
        VIR_Operand_SetEnable(operand, VIR_ENABLE_X);
        VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
        VIR_Operand_SetTypeId(operand, componentTypeId);

        /* Set SOURCE0 */
        operand = VIR_Inst_GetSource(virInst, 0);
        VIR_Operand_SetSwizzle(operand, __SpvConstIndexToVIRSwizzle(indexValue));
        VIR_Operand_SetTypeId(operand, componentTypeId);
        VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);

        switch (SPV_ID_TYPE(vectorId))
        {
        case SPV_ID_TYPE_SYMBOL:
            sourceSym = SPV_ID_VIR_SYM(vectorId);
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetSym(operand, sourceSym);
            break;

        case SPV_ID_TYPE_CONST:
            VIR_Operand_SetOpKind(operand, VIR_OPND_CONST);
            VIR_Operand_SetConstId(operand, SPV_ID_VIR_CONST_ID(vectorId));
            break;

        default:
            gcmASSERT(gcvFALSE);
            break;
        }
    }
    /* If the index ID is a variable, then we need to create a extra array to do this dynamic index. */
    else
    {
        VIR_SymId       tempArraySymId;
        VIR_Symbol     *tempArraySym;
        VIR_Symbol     *indexSym;
        gctUINT         i;

        gcmASSERT(componentCount <= 4);

        indexSym = SPV_ID_VIR_SYM(indexId);

        /* Add a temp array variable. */
        tempArraySymId = __SpvGenerateVectorDynamicIndexSym(spv, virShader, spv->resultId, componentTypeId, componentCount);
        tempArraySym = VIR_Shader_GetSymFromId(virShader, tempArraySymId);

        /* Read the vector into this array. */
        for (i = 0; i < componentCount; i++)
        {
            virOpcode = VIR_OP_MOV;
            VIR_Function_AddInstruction(spv->virFunction,
                                        virOpcode,
                                        componentTypeId,
                                        &virInst);

            /* Set DEST. */
            operand = VIR_Inst_GetDest(virInst);
            VIR_Operand_SetSym(operand, tempArraySym);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            VIR_Operand_SetEnable(operand, VIR_ENABLE_X);
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetTypeId(operand, componentTypeId);
            if (i != 0)
            {
                VIR_Operand_SetIsConstIndexing(operand, gcvTRUE);
                VIR_Operand_SetRelIndex(operand, i);
            }

            /* Set SOURCE0 */
            operand = VIR_Inst_GetSource(virInst, 0);
            VIR_Operand_SetSwizzle(operand, __SpvConstIndexToVIRSwizzle(i));
            VIR_Operand_SetTypeId(operand, componentTypeId);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);

            switch (SPV_ID_TYPE(vectorId))
            {
            case SPV_ID_TYPE_SYMBOL:
                sourceSym = SPV_ID_VIR_SYM(vectorId);
                VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
                VIR_Operand_SetSym(operand, sourceSym);
                break;

            case SPV_ID_TYPE_CONST:
                VIR_Operand_SetOpKind(operand, VIR_OPND_CONST);
                VIR_Operand_SetConstId(operand, SPV_ID_VIR_CONST_ID(vectorId));
                break;

            default:
                gcmASSERT(gcvFALSE);
                break;
            }
        }

        /* Write the data to the result. */
        virOpcode = VIR_OP_MOV;
        VIR_Function_AddInstruction(spv->virFunction,
                                    virOpcode,
                                    componentTypeId,
                                    &virInst);
        /* Set DEST. */
        operand = VIR_Inst_GetDest(virInst);
        VIR_Operand_SetSym(operand, dstVirSym);
        VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
        VIR_Operand_SetEnable(operand, VIR_ENABLE_X);
        VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
        VIR_Operand_SetTypeId(operand, componentTypeId);

        /* Set SOURCE0. */
        operand = VIR_Inst_GetSource(virInst, 0);
        VIR_Operand_SetSym(operand, tempArraySym);
        VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
        VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XXXX);
        VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
        VIR_Operand_SetTypeId(operand, componentTypeId);
        VIR_Operand_SetIsConstIndexing(operand, gcvFALSE);
        VIR_Operand_SetRelIndex(operand, VIR_Symbol_GetIndex(indexSym));
        VIR_Operand_SetRelAddrMode(operand, VIR_INDEXED_X);
    }

    return virErrCode;
}

static VSC_ErrCode __SpvEmitVectorInsertDynamic(gcSPV spv, VIR_Shader * virShader)
{
    VSC_ErrCode         virErrCode = VSC_ERR_NONE;
    SpvId               vectorId = spv->operands[0];
    SpvId               componentDataId = spv->operands[1];
    SpvId               indexId = spv->operands[2];
    VIR_Symbol         *dstVirSym;
    VIR_Symbol         *sourceSym;
    VIR_Instruction    *virInst;
    VIR_Operand        *operand;
    VIR_TypeId          vectorTypeId, componentTypeId;
    gctUINT             componentCount;
    VIR_OpCode          virOpcode;
    VIR_Enable          virEnable = VIR_ENABLE_X;

    /* Create the result variable. */
    if (SPV_ID_VIR_NAME_ID(spv->resultId) == VIR_INVALID_ID)
    {
        __SpvAddIdSymbol(spv, virShader, gcvNULL, spv->resultId, spv->resultTypeId, VIR_SYM_VARIABLE, VIR_STORAGE_GLOBAL, gcvFALSE);
    }

    /* Get the dest symbol. */
    dstVirSym = SPV_ID_VIR_SYM(spv->resultId);

    /* Get the component type.*/
    vectorTypeId = SPV_ID_VIR_TYPE_ID(spv->operands[0]);
    componentTypeId = VIR_GetTypeComponentType(vectorTypeId);
    componentCount = VIR_GetTypeComponents(vectorTypeId);

    /* Copy the data from vector source first. */
    virOpcode = VIR_OP_MOV;
    VIR_Function_AddInstruction(spv->virFunction,
                                virOpcode,
                                vectorTypeId,
                                &virInst);

    /* Set DEST. */
    operand = VIR_Inst_GetDest(virInst);
    VIR_Operand_SetSym(operand, dstVirSym);
    VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
    VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
    VIR_Operand_SetEnable(operand, VIR_TypeId_Conv2Enable(vectorTypeId));
    VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
    VIR_Operand_SetTypeId(operand, vectorTypeId);

    /* Set SOURCE0 */
    operand = VIR_Inst_GetSource(virInst, 0);
    VIR_Operand_SetSwizzle(operand, VIR_Swizzle_GenSwizzleByComponentCount(componentCount));
    VIR_Operand_SetTypeId(operand, vectorTypeId);
    VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);

    switch (SPV_ID_TYPE(vectorId))
    {
    case SPV_ID_TYPE_SYMBOL:
        sourceSym = SPV_ID_VIR_SYM(vectorId);
        VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
        VIR_Operand_SetSym(operand, sourceSym);
        break;

    case SPV_ID_TYPE_CONST:
        VIR_Operand_SetOpKind(operand, VIR_OPND_CONST);
        VIR_Operand_SetConstId(operand, SPV_ID_VIR_CONST_ID(vectorId));
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    /* If the index ID is a constant, then just generate a MOV. */
    if (SPV_ID_TYPE(indexId) == SPV_ID_TYPE_CONST)
    {
        VIR_ConstId     constId = SPV_ID_VIR_CONST_ID(indexId);
        VIR_Const      *constant = VIR_Shader_GetConstFromId(virShader, constId);
        gctUINT         indexValue = constant->value.scalarVal.uValue;

        virEnable = virEnable << indexValue;

        virOpcode = VIR_OP_MOV;
        VIR_Function_AddInstruction(spv->virFunction,
                                    virOpcode,
                                    componentTypeId,
                                    &virInst);

        /* Set DEST. */
        operand = VIR_Inst_GetDest(virInst);
        VIR_Operand_SetSym(operand, dstVirSym);
        VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
        VIR_Operand_SetEnable(operand, virEnable);
        VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
        VIR_Operand_SetTypeId(operand, vectorTypeId);

        /* Set SOURCE0 */
        operand = VIR_Inst_GetSource(virInst, 0);
        VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XXXX);
        VIR_Operand_SetTypeId(operand, componentTypeId);
        VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);

        switch (SPV_ID_TYPE(componentDataId))
        {
        case SPV_ID_TYPE_SYMBOL:
            sourceSym = SPV_ID_VIR_SYM(componentDataId);
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetSym(operand, sourceSym);
            break;

        case SPV_ID_TYPE_CONST:
            VIR_Operand_SetOpKind(operand, VIR_OPND_CONST);
            VIR_Operand_SetConstId(operand, SPV_ID_VIR_CONST_ID(componentDataId));
            break;

        default:
            gcmASSERT(gcvFALSE);
            break;
        }
    }
    /* If the index ID is a variable, then we need to create a extra array to do this dynamic index. */
    else
    {
        VIR_SymId       tempArraySymId;
        VIR_Symbol     *tempArraySym;
        VIR_Symbol     *indexSym;
        gctUINT         i;

        gcmASSERT(componentCount <= 4);

        indexSym = SPV_ID_VIR_SYM(indexId);

        /* Add a temp array variable. */
        tempArraySymId = __SpvGenerateVectorDynamicIndexSym(spv, virShader, spv->resultId, componentTypeId, componentCount);
        tempArraySym = VIR_Shader_GetSymFromId(virShader, tempArraySymId);

        /* Read the vector into this array. */
        for (i = 0; i < componentCount; i++)
        {
            virOpcode = VIR_OP_MOV;
            VIR_Function_AddInstruction(spv->virFunction,
                                        virOpcode,
                                        componentTypeId,
                                        &virInst);

            /* Set DEST. */
            operand = VIR_Inst_GetDest(virInst);
            VIR_Operand_SetSym(operand, tempArraySym);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            VIR_Operand_SetEnable(operand, VIR_ENABLE_X);
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetTypeId(operand, componentTypeId);
            if (i != 0)
            {
                VIR_Operand_SetIsConstIndexing(operand, gcvTRUE);
                VIR_Operand_SetRelIndex(operand, i);
            }

            /* Set SOURCE0 */
            operand = VIR_Inst_GetSource(virInst, 0);
            VIR_Operand_SetSym(operand, dstVirSym);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            VIR_Operand_SetSwizzle(operand, __SpvConstIndexToVIRSwizzle(i));
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetTypeId(operand, componentTypeId);
        }

        /* Write the component data by index. */
        virOpcode = VIR_OP_MOV;
        VIR_Function_AddInstruction(spv->virFunction,
                                    virOpcode,
                                    componentTypeId,
                                    &virInst);
        /* Set DEST. */
        operand = VIR_Inst_GetDest(virInst);
        VIR_Operand_SetSym(operand, tempArraySym);
        VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
        VIR_Operand_SetEnable(operand, VIR_ENABLE_X);
        VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
        VIR_Operand_SetTypeId(operand, componentTypeId);
        VIR_Operand_SetIsConstIndexing(operand, gcvFALSE);
        VIR_Operand_SetRelIndex(operand, VIR_Symbol_GetIndex(indexSym));
        VIR_Operand_SetRelAddrMode(operand, VIR_INDEXED_X);

        /* Set SOURCE. */
        operand = VIR_Inst_GetSource(virInst, 0);
        VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XXXX);
        VIR_Operand_SetTypeId(operand, componentTypeId);
        VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);

        switch (SPV_ID_TYPE(componentDataId))
        {
        case SPV_ID_TYPE_SYMBOL:
            sourceSym = SPV_ID_VIR_SYM(componentDataId);
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetSym(operand, sourceSym);
            break;

        case SPV_ID_TYPE_CONST:
            VIR_Operand_SetOpKind(operand, VIR_OPND_CONST);
            VIR_Operand_SetConstId(operand, SPV_ID_VIR_CONST_ID(componentDataId));
            break;

        default:
            gcmASSERT(gcvFALSE);
            break;
        }

        /* Write back the temp array into the vector. */
        for (i = 0; i < componentCount; i++)
        {
            virOpcode = VIR_OP_MOV;
            VIR_Function_AddInstruction(spv->virFunction,
                                        virOpcode,
                                        componentTypeId,
                                        &virInst);

            /* Set DEST. */
            operand = VIR_Inst_GetDest(virInst);
            virEnable = (VIR_Enable)(1 << i);
            VIR_Operand_SetSym(operand, dstVirSym);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            VIR_Operand_SetEnable(operand, virEnable);
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetTypeId(operand, componentTypeId);

            /* Set SOURCE0 */
            operand = VIR_Inst_GetSource(virInst, 0);
            VIR_Operand_SetSym(operand, tempArraySym);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XXXX);
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetTypeId(operand, componentTypeId);
            if (i != 0)
            {
                VIR_Operand_SetIsConstIndexing(operand, gcvTRUE);
                VIR_Operand_SetRelIndex(operand, i);
            }
        }
    }

    return virErrCode;
}

static VSC_ErrCode __SpvEmitCompositeConstruct(gcSPV spv, VIR_Shader * virShader)
{
    VIR_SymId destSymId;
    VIR_SymId *compositeSymId = gcvNULL;
    VIR_SymbolKind *compositeSymKind = gcvNULL;
    VIR_TypeId typeId;
    gctUINT i;

    gcmEMIT_HEADER();

    gcmEMIT_GET_ARGS();

    (void)operand;
    (void)virInst;
    (void)virOpcode;
    (void)dstVirSym;
    (void)dstVirTypeId;
    (void)dstVirType;

    /* Initialize sym list. */
    spvAllocate(spv->spvMemPool, spv->operandSize * gcmSIZEOF(VIR_SymId), (gctPOINTER *)&compositeSymId);
    gcoOS_ZeroMemory(compositeSymId, spv->operandSize * gcmSIZEOF(VIR_SymId));

    spvAllocate(spv->spvMemPool, spv->operandSize * gcmSIZEOF(VIR_SymbolKind), (gctPOINTER *)&compositeSymKind);
    gcoOS_ZeroMemory(compositeSymKind, spv->operandSize * gcmSIZEOF(VIR_SymbolKind));

    /* Get result sym id. */
    destSymId = SPV_ID_VIR_SYM_ID(spv->resultId);

    /* Get type id. */
    typeId = SPV_ID_TYPE_VIR_TYPE_ID(SPV_ID_SYM_SPV_TYPE(spv->resultId));

    /* Get composite source sym id. */
    for (i = 0; i < spv->operandSize; i++)
    {
        if (SPV_ID_TYPE(spv->operands[i]) == SPV_ID_TYPE_CONST)
        {
            compositeSymKind[i] = VIR_SYM_CONST;
            compositeSymId[i] = SPV_ID_VIR_CONST_ID(spv->operands[i]);
        }
        else
        {
            compositeSymKind[i] = VIR_SYM_VARIABLE;
            compositeSymId[i] = SPV_ID_VIR_SYM_ID(spv->operands[i]);
        }
    }

    VIR_Shader_CompositeConstruct(virShader,
        spv->virFunction,
        gcvNULL,
        destSymId,
        typeId,
        gcvFALSE,
        compositeSymId,
        compositeSymKind,
        spv->operandSize);

    spvFree(gcvNULL, compositeSymId);
    spvFree(gcvNULL, compositeSymKind);

    return virErrCode;
}

static VSC_ErrCode __SpvOpcode2Intrisic(
    IN gcSPV spv,
    IN gctUINT opCode,
    INOUT gctUINT * setId,
    INOUT gctUINT * instId)
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VIR_INTRINSIC_SET setid = VIR_INTRINSIC_SET_INTERNAL;
    VIR_INTRINSIC_INTERNAL_ID instid = 0;

    switch (opCode)
    {
    case SpvOpTranspose:
        instid = VIR_INTRINSIC_INTERNAL_TRANSPOSE;
        break;

    case SpvOpImageWrite:
        instid = VIR_INTRINSIC_INTERNAL_IMAGESTORE;
        break;

    case SpvOpImageRead:
        instid = VIR_INTRINSIC_INTERNAL_IMAGELOAD;
        break;

    case SpvOpIAddCarry:
        instid = VIR_INTRINSIC_INTERNAL_ADDCARRY;
        break;

    case SpvOpISubBorrow:
        instid = VIR_INTRINSIC_INTERNAL_SUBBORROW;
        break;

    case SpvOpUMulExtended:
        instid = VIR_INTRINSIC_INTERNAL_UMULEXTENDED;
        break;

    case SpvOpSMulExtended:
        instid = VIR_INTRINSIC_INTERNAL_SMULEXTENDED;
        break;

    case SpvOpQuantizeToF16:
        instid = VIR_INTRINSIC_INTERNAL_QUANTIZE_TO_F16;
        break;

    case SpvOpImageFetch:
        instid = VIR_INTRINSIC_INTERNAL_IMAGEFETCH;
        break;

    case SpvOpImageTexelPointer:
        instid = VIR_INTRINSIC_INTERNAL_IMAGEADDR;
        break;

    case SpvOpImageQueryFormat:
        instid = VIR_INTRINSIC_INTERNAL_IMAGE_QUERY_FORMAT;
        break;

    case SpvOpImageQueryOrder:
        instid = VIR_INTRINSIC_INTERNAL_IMAGE_QUERY_ORDER;
        break;

    case SpvOpImageQuerySizeLod:
        instid = VIR_INTRINSIC_INTERNAL_IMAGE_QUERY_SIZE_LOD;
        break;

    case SpvOpImageQuerySize:
        instid = VIR_INTRINSIC_INTERNAL_IMAGE_QUERY_SIZE;
        break;

    case SpvOpImageQueryLod:
        instid = VIR_INTRINSIC_INTERNAL_IMAGE_QUERY_LOD;
        break;

    case SpvOpImageQueryLevels:
        instid = VIR_INTRINSIC_INTERNAL_IMAGE_QUERY_LEVELS;
        break;

    case SpvOpImageQuerySamples:
        instid = VIR_INTRINSIC_INTERNAL_IMAGE_QUERY_SAMPLES;
        break;

    default:
        setid = 0;
        virErrCode = VSC_ERR_INVALID_ARGUMENT;
        break;
    }

    if (setId)
    {
        *setId = setid;
    }

    if (instId)
    {
        *instId = instid;
    }

    return virErrCode;
}

static VSC_ErrCode __SpvEmitIntrisicCall(gcSPV spv, VIR_Shader * virShader)
{
    /* fake a intrisic */
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    gctUINT argCount = spv->operandSize;
    gctUINT tmpOperand[SPV_MAX_OPERAND_NUM];
    gctUINT setId = 0;
    gctUINT instId = 0;

    if (spv->opCode == SpvOpImageRead &&
        (VIR_TypeId_isImageSubPassData(SPV_ID_TYPE_VIR_TYPE_ID(spv->operands[0]))
         ||
         (SPV_ID_SYM_ATTACHMENT_FLAG(spv->operands[0]) & SPV_ATTACHMENTFLAG_TREAT_AS_SAMPLER)))
    {
        /* add current fragment location with coordinate */
        VIR_Instruction * virInst;
        VIR_Operand * operand;
        VIR_Symbol * dstCoordSym = gcvNULL;
        VIR_Symbol * convCoordSym = gcvNULL;
        VIR_SymId symId;
        VIR_Swizzle virSwizzle = __SpvID2Swizzle(spv, spv->operands[1]);
        VIR_TypeId virTypeId = VIR_Symbol_GetTypeId(SPV_ID_VIR_SYM(spv->operands[1]));
        VIR_NameId nameId;
        gctUINT offset = 0;

        gcoOS_MemFill(spv->virName, 0, SPV_VIR_NAME_SIZE * gcmSIZEOF(gctCHAR));
        gcoOS_PrintStrSafe(spv->virName, SPV_VIR_NAME_SIZE, &offset, "#sh_spv_conv_int_%d", spv->operands[1]);
        VIR_Shader_AddString(virShader, spv->virName, &nameId);

        VIR_Shader_AddSymbol(
            virShader,
            VIR_SYM_VARIABLE,
            nameId,
            VIR_Shader_GetTypeFromId(virShader, virTypeId),
            VIR_STORAGE_GLOBAL,
            &symId);

        convCoordSym = VIR_Shader_GetSymFromId(virShader, symId);

        VIR_Symbol_SetPrecision(convCoordSym, VIR_PRECISION_HIGH);
        VIR_Symbol_SetTyQualifier(convCoordSym, VIR_TYQUAL_NONE);
        VIR_Symbol_SetLayoutQualifier(convCoordSym, VIR_LAYQUAL_NONE);
        VIR_Symbol_SetLocation(convCoordSym, -1);
        VIR_Symbol_SetFlag(convCoordSym, VIR_SYMFLAG_WITHOUT_REG);

        /* transform vec4 to ivec2 */
        VIR_Function_AddInstruction(
            spv->virFunction,
            VIR_OP_CONVERT,
            virTypeId,
            &virInst
            );

        VIR_Inst_SetConditionOp(virInst, VIR_COP_ALWAYS);

        operand = VIR_Inst_GetDest(virInst);
        VIR_Operand_SetSym(operand, convCoordSym);
        VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
        VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
        VIR_Operand_SetEnable(operand, VIR_Swizzle_2_Enable(virSwizzle));
        VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
        VIR_Operand_SetTypeId(operand, virTypeId);

        /* gl_FragCoord */
        operand = VIR_Inst_GetSource(virInst, 0);
        VIR_Operand_SetSwizzle(operand, virSwizzle);
        VIR_Operand_SetTypeId(operand, VIR_TYPE_FLOAT_X4);
        VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
        VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
        VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
        VIR_Operand_SetSym(operand, VIR_Shader_GetSymFromId(virShader, spv->builtinVariable[SPV_FRAGCOORD]));


        offset = 0;
        gcoOS_MemFill(spv->virName, 0, SPV_VIR_NAME_SIZE * gcmSIZEOF(gctCHAR));
        gcoOS_PrintStrSafe(spv->virName, SPV_VIR_NAME_SIZE, &offset, "#sh_spv_subpass_coord_%d", spv->operands[1]);
        VIR_Shader_AddString(virShader, spv->virName, &nameId);

        VIR_Shader_AddSymbol(
            virShader,
            VIR_SYM_VARIABLE,
            nameId,
            VIR_Shader_GetTypeFromId(virShader, virTypeId),
            VIR_STORAGE_GLOBAL,
            &symId);

        dstCoordSym = VIR_Shader_GetSymFromId(virShader, symId);

        VIR_Symbol_SetPrecision(dstCoordSym, VIR_PRECISION_HIGH);
        VIR_Symbol_SetTyQualifier(dstCoordSym, VIR_TYQUAL_NONE);
        VIR_Symbol_SetLayoutQualifier(dstCoordSym, VIR_LAYQUAL_NONE);
        VIR_Symbol_SetLocation(dstCoordSym, -1);
        VIR_Symbol_SetFlag(dstCoordSym, VIR_SYMFLAG_WITHOUT_REG);

        /* add coordinate to gl_FragCoord */
        VIR_Function_AddInstruction(
            spv->virFunction,
            VIR_OP_ADD,
            virTypeId,
            &virInst);

        VIR_Inst_SetConditionOp(virInst, VIR_COP_ALWAYS);

        operand = VIR_Inst_GetDest(virInst);
        VIR_Operand_SetSym(operand, dstCoordSym);
        VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
        VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
        VIR_Operand_SetEnable(operand, VIR_Swizzle_2_Enable(virSwizzle));
        VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
        VIR_Operand_SetTypeId(operand, virTypeId);

        operand = VIR_Inst_GetSource(virInst, 0);
        VIR_Operand_SetSwizzle(operand, virSwizzle);
        VIR_Operand_SetTypeId(operand, virTypeId);
        VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
        VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
        VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
        VIR_Operand_SetSym(operand, convCoordSym);

        /* Coordinate */
        operand = VIR_Inst_GetSource(virInst, 1);
        VIR_Operand_SetSwizzle(operand, virSwizzle);
        VIR_Operand_SetTypeId(operand, virTypeId);
        VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
        VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
        if (SPV_ID_TYPE(spv->operands[1]) == SPV_ID_TYPE_SYMBOL)
        {
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetSym(operand, SPV_ID_VIR_SYM(spv->operands[1]));
        }
        else
        {
            VIR_Operand_SetConstId(operand, SPV_ID_VIR_CONST_ID(spv->operands[1]));
            VIR_Operand_SetOpKind(operand, VIR_OPND_CONST);
        }

        spv->internalSym = dstCoordSym;
    }

    /* Generate TEXLD for this kind of image. */
    if (SPV_ID_SYM_ATTACHMENT_FLAG(spv->operands[0]) & SPV_ATTACHMENTFLAG_TREAT_AS_SAMPLER)
    {
        spv->opCode = SpvOpImageSampleImplicitLod;
        virErrCode = __SpvEmitImageSample(spv, virShader);
        return virErrCode;
    }

    /* Copy operands to begin at 2 */
    gcoOS_MemCopy(tmpOperand, spv->operands, argCount * gcmSIZEOF(gctUINT));
    gcoOS_MemCopy((gctPOINTER*)&spv->operands[2], tmpOperand, argCount * gcmSIZEOF(gctUINT));

    /* enlarge the size, we need add operand 0 and operand 1 */
    spv->operandSize += 2;

    virErrCode = __SpvOpcode2Intrisic(spv, spv->opCode, &setId, &instId);
    if (virErrCode != VSC_ERR_NONE) return virErrCode;

    spv->operands[0] = setId;
    spv->operands[1] = instId;

    virErrCode = __SpvAddIntrisicFunction(spv, virShader);

    return virErrCode;
}

static VSC_ErrCode __SpvAddOpImage(gcSPV spv, VIR_Shader * virShader)
{
    gctUINT sampledImageId = spv->operands[0];

    __SpvAddIdSymbol(spv,
                     virShader,
                     gcvNULL,
                     spv->resultId,
                     spv->resultTypeId,
                     VIR_SYM_VARIABLE,
                     VIR_STORAGE_GLOBAL,
                     gcvFALSE);
    {
        SPV_ID_SYM_SAMPLED_IMAGE(spv->resultId) = gcvTRUE;
        /* record the sampler and image */
        SPV_ID_SYM_SAMPLEDIMAGE_IMAGE(spv->resultId) = SPV_ID_SYM_SAMPLEDIMAGE_IMAGE(sampledImageId);
        SPV_ID_SYM_SAMPLEDIMAGE_SAMPLER(spv->resultId) = SPV_ID_SYM_SAMPLEDIMAGE_SAMPLER(sampledImageId);
    }

    return VSC_ERR_NONE;
}

static VSC_ErrCode __SpvEmitSampledImage(gcSPV spv, VIR_Shader * virShader)
{
    __SpvAddIdSymbol(spv,
                     virShader,
                     gcvNULL,
                     spv->resultId,
                     spv->resultTypeId,
                     VIR_SYM_VARIABLE,
                     VIR_STORAGE_GLOBAL,
                     gcvFALSE);

    SPV_ID_SYM_SAMPLED_IMAGE(spv->resultId) = gcvTRUE;
    SPV_ID_SYM_SAMPLEDIMAGE_IMAGE(spv->resultId) = spv->operands[0];
    SPV_ID_SYM_SAMPLEDIMAGE_SAMPLER(spv->resultId) = spv->operands[1];

    return VSC_ERR_NONE;
}

static VSC_ErrCode __SpvSetTexldModifier(
    gcSPV spv,
    VIR_Shader * virShader,
    VIR_Operand * operand,
    VIR_Operand ** modifier,
    gctUINT modifierCount,
    SpvImageOperandsMask singleMask)
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;

    gcmASSERT((modifierCount <= 2) && (modifierCount > 0)); /* max operand count is 2 */

    if (singleMask & SpvImageOperandsBiasMask)
    {
        VIR_Operand_SetTexldBias(operand, modifier[0]);
    }
    else if (singleMask & SpvImageOperandsLodMask)
    {
        VIR_Operand_SetTexldLod(operand, modifier[0]);
    }
    else if (singleMask & SpvImageOperandsGradMask)
    {
        VIR_Operand_SetTexldGradient(operand, modifier[0], modifier[1]);
    }
    else if (singleMask & SpvImageOperandsSampleMask)
    {
        VIR_Operand_SetTexldFetchMS(operand, modifier[0]);
    }
    else if ((singleMask & SpvImageOperandsConstOffsetMask) ||
        (singleMask & SpvImageOperandsOffsetMask))
    {
        VIR_Operand_SetTexldOffset(operand, modifier[0]);
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    return virErrCode;
}

static gctUINT32 __SpvImageFlag2VirFlagIndex(gcSPV spv, SpvImageOperandsMask flag)
{
    gctUINT32 index = VIR_INVALID_ID;

    switch (flag)
    {
    case SpvImageOperandsBiasMask:      index = 0; break;
    case SpvImageOperandsLodMask:       index = 1; break;
    case SpvImageOperandsGradMask:      index = 2; break;
    case SpvImageOperandsConstOffsetMask:
    case SpvImageOperandsOffsetMask:    index = 4; break;

    case SpvImageOperandsSampleMask:    index = 1; break;

    case SpvImageOperandsConstOffsetsMask:
    case SpvImageOperandsMinLodMask:
        gcmASSERT(gcvFALSE);
        break;

    default: break;
    }

    return index;
}

static VSC_ErrCode __SpvDecodeImageOperand(
    gcSPV spv,
    VIR_Shader * virShader,
    VIR_Operand * operand,
    SpvImageOperandsMask operandMask,
    SpvId * operands,
    gctUINT operandCount)
{
    VSC_ErrCode                 virErrCode = VSC_ERR_NONE;
    gctUINT                     i = 0, j, k;
    SpvId                       spvOperand;

    for (j = 0; j < SPV_MAX_IMAGE_OPERAND_MASK; j++)
    {
        SpvImageOperandsMask singleMask = operandMask & (1 << j);
        gctUINT32 paramIndex = __SpvImageFlag2VirFlagIndex(spv, singleMask);
        gctUINT32 maskCount = 1;
        VIR_Operand * paramOperand[2] = { 0 };

        if (i >= operandCount)
        {
            /* exhausted the spv operands */
            break;
        }

        if (paramIndex != VIR_INVALID_ID)
        {
            if (singleMask & SpvImageOperandsGradMask)
            {
                maskCount = 2;
            }

            for (k = 0; k < maskCount; k++)
            {
                spvOperand = operands[i++];

                virErrCode = VIR_Function_NewOperand(spv->virFunction, &paramOperand[k]);
                if (virErrCode != VSC_ERR_NONE) return virErrCode;

                VIR_Operand_SetTypeId(paramOperand[k], SPV_ID_VIR_TYPE_ID(spvOperand));
                VIR_Operand_SetPrecision(paramOperand[k], VIR_PRECISION_HIGH);
                VIR_Operand_SetRoundMode(paramOperand[k], VIR_ROUND_DEFAULT);
                VIR_Operand_SetModifier(paramOperand[k], VIR_MOD_NONE);
                VIR_Operand_SetSwizzle(paramOperand[k], __SpvID2Swizzle(spv, spvOperand));

                if (SPV_ID_TYPE(spvOperand) == SPV_ID_TYPE_SYMBOL)
                {
                    VIR_Symbol* sym = SPV_ID_VIR_SYM(spvOperand);
                    VIR_Operand_SetSym(paramOperand[k], sym);
                    VIR_Operand_SetOpKind(paramOperand[k], VIR_OPND_SYMBOL);
                }
                else if (SPV_ID_TYPE(spvOperand) == SPV_ID_TYPE_CONST)
                {
                    VIR_Operand_SetConstId(paramOperand[k], SPV_ID_VIR_CONST_ID(spvOperand));
                    VIR_Operand_SetOpKind(paramOperand[k], VIR_OPND_CONST);
                }
                else
                {
                    gcmASSERT(gcvFALSE);
                }
                __SetAccessChainOffsetToOperand(spv, spvOperand, paramOperand[k], SpvOffsetType_Normal);
            }

            __SpvSetTexldModifier(spv, virShader, operand, paramOperand, maskCount, singleMask);
        }
    }

    return virErrCode;
}

static gctBOOL __SpvConvImageSampleToIntrinsicFunc(
    IN  gcSPV           spv,
    IN  VIR_Shader     *virShader,
    IN  gctUINT         fixedOperandSize
    )
{
    SpvId               samplerId = spv->operands[0];
    VIR_TypeId          virSamplerTypeId = SPV_ID_VIR_TYPE_ID(samplerId);

    /* 1: This is an array sampler, then use a intrinsic function. */
    if ((VIR_TypeId_isSampler1D(virSamplerTypeId) || VIR_TypeId_isSampler2D(virSamplerTypeId)) &&
        VIR_TypeId_isSamplerArray(virSamplerTypeId))
    {
        return gcvTRUE;
    }

    /* 2: There is Offset image operand, then use a intrinsic function. */

    return gcvFALSE;
}

static VSC_ErrCode __SpvEmitImageSample(gcSPV spv, VIR_Shader * virShader)
{
    gctBOOL             hasOptionalImageOperand = gcvFALSE;
    gctBOOL             generateTexldParam = gcvFALSE;
    gctUINT32           flags = SPV_INVALID_ID;
    gctUINT32           i;
    SpvId               spvOperand;
    gctUINT32           fixedOperandSize = 2;
    gctUINT32           mixedOperandSize = fixedOperandSize;
    gctUINT32           virOpIndex = 0;
    gctBOOL             useIntrinsicFunc = gcvFALSE;
    gctBOOL             isSetDepth = gcvFALSE;
    gctBOOL             isSetComp = gcvFALSE;
    gctBOOL             isZeroComp = gcvFALSE;
    VIR_ParmPassing    *parmOpnd = gcvNULL;
    VIR_IntrinsicsKind  intrinsickKind = VIR_IK_NONE;
    VIR_Type           *samplerType = VIR_Shader_GetTypeFromId(virShader, SPV_ID_VIR_TYPE_ID(spv->operands[0]));
    VIR_TypeId          samplerTypeId = VIR_Type_GetBaseTypeId(samplerType);

    gcmEMIT_HEADER();

    gcmEMIT_GET_ARGS();

    if (spv->opCode == SpvOpImageSampleDrefImplicitLod      ||
        spv->opCode == SpvOpImageSampleDrefExplicitLod      ||
        spv->opCode == SpvOpImageSampleProjDrefImplicitLod  ||
        spv->opCode == SpvOpImageSampleProjDrefExplicitLod  ||
        spv->opCode == SpvOpImageDrefGather                 ||
        spv->opCode == SpvOpImageSparseDrefGather)
    {
        isSetDepth = gcvTRUE;
        mixedOperandSize++;

        if (spv->opCode == SpvOpImageDrefGather || spv->opCode == SpvOpImageSparseDrefGather)
        {
            isSetComp = gcvTRUE;
            isZeroComp = gcvTRUE;
        }
    }
    else if (spv->opCode == SpvOpImageGather                ||
             spv->opCode == SpvOpImageSparseGather)
    {
        isSetComp = gcvTRUE;
        mixedOperandSize++;
    }

    /* If this sample is an image, the Coordinate is an integer operand, we need to use TEXLD_U. */
    if (SPV_ID_SYM_ATTACHMENT_FLAG(spv->operands[0]) & SPV_ATTACHMENTFLAG_TREAT_AS_SAMPLER)
    {
        switch (virOpcode)
        {
        case VIR_OP_TEXLD:
            virOpcode = VIR_OP_TEXLD_U;
            break;

        default:
            break;
        }
    }
    if (SPV_ID_SYM_ATTACHMENT_FLAG(spv->operands[0]) & SPV_ATTACHMENTFLAG_MULTI_SAMPLE)
    {
        switch (virOpcode)
        {
        case VIR_OP_TEXLD_U:
            virOpcode = VIR_OP_TEXLD;
            break;

        default:
            break;
        }
    }

    /* Check if there is any optional modifier, lod/bias. */
    hasOptionalImageOperand = (spv->operandSize > mixedOperandSize);

    /* Check if need to use texld parameter. */
    generateTexldParam = hasOptionalImageOperand || isSetDepth || isSetComp;

    /* Generate a intrinsic function for this ImageSample if needed. */
    if (__SpvConvImageSampleToIntrinsicFunc(spv, virShader, fixedOperandSize))
    {
        switch (virOpcode)
        {
        case VIR_OP_TEXLD:
            if (VIR_TypeId_isSamplerShadow(samplerTypeId))
            {
                intrinsickKind = VIR_IK_texldpcf;
            }
            else
            {
                intrinsickKind = VIR_IK_texld;
            }
            break;

        case VIR_OP_TEXLDPROJ:
            intrinsickKind = VIR_IK_texld_proj;
            break;

        case VIR_OP_TEXLD_GATHER:
            intrinsickKind = VIR_IK_texld_gather;
            break;

        case VIR_OP_TEXLD_FETCH_MS:
            intrinsickKind = VIR_IK_texld_fetch_ms;
            break;

        default:
            break;
        }

        virOpcode = VIR_OP_INTRINSIC;
        useIntrinsicFunc = gcvTRUE;
    }
    else
    {
        /* Check if it is shadow. */
        if (VIR_TypeId_isSamplerShadow(samplerTypeId))
        {
            switch (virOpcode)
            {
                case VIR_OP_TEXLD:
                case VIR_OP_TEXLD_U:
                    virOpcode = VIR_OP_TEXLDPCF;
                    break;

                case VIR_OP_TEXLDPROJ:
                    virOpcode = VIR_OP_TEXLDPCFPROJ;
                    break;

                case VIR_OP_TEXLD_GATHER:
                    virOpcode = VIR_OP_TEXLD_GATHER_PCF;
                    break;

                default:
                    break;
            }
        }
    }

    VIR_Function_AddInstruction(spv->virFunction,
        virOpcode,
        dstVirTypeId,
        &virInst);

    VIR_Inst_SetConditionOp(virInst, VIR_COP_ALWAYS);

    operand = VIR_Inst_GetDest(virInst);
    VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
    VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
    VIR_Operand_SetEnable(operand, __SpvGenEnable(spv, dstVirType, resultTypeId));
    VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
    VIR_Operand_SetTypeId(operand, dstVirTypeId);
    VIR_Operand_SetSym(operand, dstVirSym);
    VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
    __SetAccessChainOffsetToOperand(spv, spv->resultId, operand, SpvOffsetType_Normal);

    if (useIntrinsicFunc)
    {
        /* Use one more operand for texld parameters. */
        if (generateTexldParam)
        {
            VIR_Function_NewParameters(spv->virFunction, fixedOperandSize + 1, &parmOpnd);
        }
        else
        {
            VIR_Function_NewParameters(spv->virFunction, fixedOperandSize, &parmOpnd);
        }
        VIR_Operand_SetIntrinsic(VIR_Inst_GetSource(virInst, 0), intrinsickKind);
        VIR_Operand_SetParameters(VIR_Inst_GetSource(virInst, 1), parmOpnd);
    }

    for (i = 0; i < fixedOperandSize; i++)
    {
        if (useIntrinsicFunc)
        {
            operand = parmOpnd->args[virOpIndex];
        }
        else
        {
            operand = VIR_Inst_GetSource(virInst, virOpIndex);
        }

        spvOperand = spv->operands[i];

        if ((i == 1) &&
            (SPV_ID_SYM_ATTACHMENT_FLAG(spv->operands[0]) & SPV_ATTACHMENTFLAG_TREAT_AS_SAMPLER))
        {
            /* handle subpass */
            VIR_Operand_SetSwizzle(operand, __SpvID2Swizzle(spv, spvOperand));
            VIR_Operand_SetSym(operand, spv->internalSym);
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetTypeId(operand, SPV_ID_VIR_TYPE_ID(spvOperand));
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            virOpIndex++;
            continue;
        }

        VIR_Operand_SetTypeId(operand, SPV_ID_VIR_TYPE_ID(spvOperand));
        VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
        VIR_Operand_SetSwizzle(operand, __SpvID2Swizzle(spv, spvOperand));
        VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);

        if (SPV_ID_TYPE(spvOperand) == SPV_ID_TYPE_SYMBOL)
        {
            VIR_Operand_SetSym(operand, SPV_ID_VIR_SYM(spvOperand));
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);

            if (SPV_ID_SYM_SAMPLED_IMAGE(spvOperand))
            {
                __SpvSetSampledImage(spv, virShader, spvOperand);
            }
        }
        else if (SPV_ID_TYPE(spvOperand) == SPV_ID_TYPE_CONST)
        {
            VIR_Operand_SetConstId(operand, SPV_ID_VIR_CONST_ID(spvOperand));
            VIR_Operand_SetOpKind(operand, VIR_OPND_CONST);
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }
        __SetAccessChainOffsetToOperand(spv, spvOperand, operand, SpvOffsetType_Normal);

        virOpIndex++;
    }

    if (generateTexldParam)
    {
        VIR_Operand * texldOperand;

        if (useIntrinsicFunc)
        {
            operand = parmOpnd->args[virOpIndex];
        }
        else
        {
            operand = VIR_Inst_GetSource(virInst, virOpIndex);
        }

        /* Initialize the texldModifier */
        texldOperand = (VIR_Operand *)operand;
        gcoOS_ZeroMemory((gctPOINTER)texldOperand->u.tmodifier, VIR_TEXLDMODIFIER_COUNT * gcmSIZEOF(VIR_Operand *));

        VIR_Operand_SetOpKind(operand, VIR_OPND_TEXLDPARM);
        VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
    }

    /* Set Dref modifier. */
    if (isSetDepth)
    {
        VIR_Operand * paramOperand;
        VIR_ScalarConstVal constVal;
        SpvId spvOperand = spv->operands[i++];

        virErrCode = VIR_Function_NewOperand(spv->virFunction, &paramOperand);
        if (virErrCode != VSC_ERR_NONE) return virErrCode;

        VIR_Operand_SetTypeId(paramOperand, SPV_ID_VIR_TYPE_ID(spvOperand));
        VIR_Operand_SetPrecision(paramOperand, VIR_PRECISION_HIGH);
        VIR_Operand_SetRoundMode(paramOperand, VIR_ROUND_DEFAULT);
        VIR_Operand_SetModifier(paramOperand, VIR_MOD_NONE);
        VIR_Operand_SetSwizzle(paramOperand, __SpvID2Swizzle(spv, spvOperand));

        if (SPV_ID_TYPE(spvOperand) == SPV_ID_TYPE_SYMBOL)
        {
            VIR_Symbol* sym = SPV_ID_VIR_SYM(spvOperand);
            VIR_Operand_SetSym(paramOperand, sym);
            VIR_Operand_SetOpKind(paramOperand, VIR_OPND_SYMBOL);
        }
        else if (SPV_ID_TYPE(spvOperand) == SPV_ID_TYPE_CONST)
        {
            constVal.uValue = SPV_CONST_SCALAR_UINT(spvOperand);
            VIR_Operand_SetImmediate(paramOperand, SPV_ID_VIR_TYPE_ID(spvOperand), constVal);
            VIR_Operand_SetOpKind(paramOperand, VIR_OPND_IMMEDIATE);
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }

        __SetAccessChainOffsetToOperand(spv, spvOperand, paramOperand, SpvOffsetType_Normal);

        VIR_Operand_SetTexldGatherRefZ(operand, paramOperand);
    }

    /* Set component modifier. */
    if (isSetComp)
    {
        VIR_Operand * paramOperand;

        virErrCode = VIR_Function_NewOperand(spv->virFunction, &paramOperand);
        if (virErrCode != VSC_ERR_NONE) return virErrCode;

        if (isZeroComp)
        {
            VIR_Operand_SetImmediateInt(paramOperand, 0);
        }
        else
        {
            VIR_ScalarConstVal constVal;
            SpvId spvOperand = spv->operands[i++];

            VIR_Operand_SetTypeId(paramOperand, SPV_ID_VIR_TYPE_ID(spvOperand));
            VIR_Operand_SetPrecision(paramOperand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(paramOperand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(paramOperand, VIR_MOD_NONE);
            VIR_Operand_SetSwizzle(paramOperand, __SpvID2Swizzle(spv, spvOperand));

            if (SPV_ID_TYPE(spvOperand) == SPV_ID_TYPE_SYMBOL)
            {
                VIR_Symbol* sym = SPV_ID_VIR_SYM(spvOperand);
                VIR_Operand_SetSym(paramOperand, sym);
                VIR_Operand_SetOpKind(paramOperand, VIR_OPND_SYMBOL);
            }
            else if (SPV_ID_TYPE(spvOperand) == SPV_ID_TYPE_CONST)
            {
                constVal.uValue = SPV_CONST_SCALAR_UINT(spvOperand);
                VIR_Operand_SetImmediate(paramOperand, SPV_ID_VIR_TYPE_ID(spvOperand), constVal);
                VIR_Operand_SetOpKind(paramOperand, VIR_OPND_IMMEDIATE);
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }

            __SetAccessChainOffsetToOperand(spv, spvOperand, paramOperand, SpvOffsetType_Normal);
        }

        VIR_Operand_SetTexldGatherComp(operand, paramOperand);
    }

    /* Optional Image Operands. */
    if (hasOptionalImageOperand)
    {
        if (flags == SPV_INVALID_ID)
        {
            flags = spv->operands[i++];
        }

        __SpvDecodeImageOperand(spv, virShader, operand, flags, &spv->operands[i], spv->operandSize - i);
    }

    /* Update resOpType. */
    VIR_Inst_UpdateResOpType(virInst);

    return virErrCode;
}


static VSC_ErrCode __SpvInsertWorkGroupOffsetInst(
    gcSPV spv,
    VIR_Shader * virShader,
    VIR_Instruction * dstVirInst,
    gctUINT32 baseOffset,
    VIR_SymbolKind baseOffsetType)
{
    VIR_Instruction * virInst;
    SpvId dstSpvId = spv->operands[0];
    VIR_Symbol * sym;
    VIR_Operand * operand;
    VIR_Swizzle virSwizzle = VIR_SWIZZLE_XXXX;
    VIR_NameId nameId;
    VIR_SymId symId;
    gctUINT offset = 0;

    if (!SPV_HAS_WORKGROUP())
    {
        return VSC_ERR_NONE;
    }

    gcoOS_MemFill(spv->virName, 0, SPV_VIR_NAME_SIZE * gcmSIZEOF(gctCHAR));
    gcoOS_PrintStrSafe(spv->virName, SPV_VIR_NAME_SIZE, &offset, "#sh_spv_shared_%d", dstSpvId);
    VIR_Shader_AddString(virShader, spv->virName, &nameId);

    VIR_Shader_AddSymbol(
        virShader,
        VIR_SYM_VARIABLE,
        nameId,
        VIR_Shader_GetTypeFromId(virShader, VIR_TYPE_UINT32),
        VIR_STORAGE_GLOBAL,
        &symId);

    SPV_WORKGROUP_INFO()->curOffsetSymId = symId;

    sym = VIR_Shader_GetSymFromId(virShader, symId);

    VIR_Symbol_SetPrecision(sym, VIR_PRECISION_HIGH);
    VIR_Symbol_SetTyQualifier(sym, VIR_TYQUAL_NONE);
    VIR_Symbol_SetLayoutQualifier(sym, VIR_LAYQUAL_NONE);
    VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_WITHOUT_REG);
    VIR_Symbol_SetLocation(sym, -1);

    VIR_Function_AddInstructionBefore(spv->virFunction,
        VIR_OP_ADD,
        VIR_TYPE_UINT32,
        dstVirInst,
        gcvTRUE,
        &virInst);

    operand = VIR_Inst_GetDest(virInst);
    VIR_Operand_SetSym(operand, sym);
    VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
    VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
    VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
    VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
    VIR_Operand_SetEnable(operand, VIR_Swizzle_2_Enable(virSwizzle));
    VIR_Operand_SetTypeId(operand, VIR_TYPE_UINT32);

    operand = VIR_Inst_GetSource(virInst, 0);
    sym = VIR_Shader_GetSymFromId(virShader, SPV_WORKGROUP_INFO()->groupOffsetSymId);
    VIR_Operand_SetSym(operand, sym);
    VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
    VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
    VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
    VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
    VIR_Operand_SetSwizzle(operand, virSwizzle);
    VIR_Operand_SetTypeId(operand, VIR_TYPE_UINT32);

    operand = VIR_Inst_GetSource(virInst, 1);
    if (baseOffsetType == VIR_SYM_VARIABLE)
    {
        sym = VIR_Shader_GetSymFromId(virShader, baseOffset);
        VIR_Operand_SetSym(operand, sym);
        VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
    }
    else
    {
        VIR_ScalarConstVal constVal;
        constVal.uValue = baseOffset;
        VIR_Operand_SetImmediate(operand, VIR_TYPE_UINT32, constVal);
    }
    VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
    VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
    VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
    VIR_Operand_SetSwizzle(operand, virSwizzle);
    VIR_Operand_SetTypeId(operand, VIR_TYPE_UINT32);

    return VSC_ERR_NONE;
}

static gctBOOL __SpvUseLoadStoreToAccessBlock(gcSPV spv, VIR_Shader * virShader, SpvId id)
{
    gctBOOL ret = gcvFALSE;

    /* check if it is block, only works for accesschain for now */
    if (SPV_ID_TYPE(id) == SPV_ID_TYPE_SYMBOL)
    {
        SpvId spvTypeId = SPV_ID_SYM_SRC_SPV_TYPE(spv->operands[0]);

        if (SPV_ID_TYPE_IS_ARRAY(spvTypeId))
        {
            spvTypeId = SPV_ID_TYPE_ARRAY_BASE_TYPE_ID(spvTypeId);
        }

        if (SPV_ID_TYPE_IS_BLOCK(spvTypeId))
        {
            VIR_Symbol * blockSym = VIR_Shader_GetSymFromId(virShader, SPV_ID_VIR_SYM_ID(id));

            gcmASSERT(blockSym);

            /* push constant do not generate load and store */
            if (VIR_Symbol_GetUniformKind(blockSym) != VIR_UNIFORM_PUSH_CONSTANT)
            {
                switch (VIR_Symbol_GetKind(blockSym))
                {
                case VIR_SYM_UBO:
                case VIR_SYM_SBO:
                    ret = gcvTRUE;
                    break;

                case VIR_SYM_IOBLOCK:
                    ret = gcvFALSE;
                    break;

                default:
                    gcmASSERT(gcvFALSE);
                    break;
                }
            }
        }
        else if (SPV_ID_SYM_IS_WORKGROUP(id))
        {
            ret = gcvTRUE;
        }
    }

    return ret;
}

static VSC_ErrCode __spvGenerateInstrinsicVecGet(gcSPV spv,
                                             VIR_Symbol *destSym,
                                             VIR_Symbol *src0Sym,
                                             VIR_Swizzle src0Swizzle,
                                             VIR_Symbol *src1Sym,
                                             VIR_Swizzle src1Swizzle)

{
    VIR_Instruction *virVecGetInst;
    VIR_ParmPassing *parmOpnd;
    gctUINT argCount = 2;
    VIR_Operand *src0Opnd;
    VIR_Operand *src1Opnd;
    VIR_TypeId dstTypeId;
    VSC_ErrCode virErrCode;
    gcmASSERT(destSym && src0Sym && src1Sym);
    dstTypeId = VIR_Symbol_GetTypeId(destSym);
    virErrCode = VIR_Function_AddInstruction(spv->virFunction,
                                             VIR_OP_INTRINSIC,
                                             dstTypeId,
                                             &virVecGetInst);
    VIR_Operand_SetRoundMode(VIR_Inst_GetDest(virVecGetInst), VIR_ROUND_DEFAULT);
    VIR_Operand_SetModifier(VIR_Inst_GetDest(virVecGetInst), VIR_MOD_NONE);
    VIR_Operand_SetEnable(VIR_Inst_GetDest(virVecGetInst), __SpvGenEnable(spv, VIR_Symbol_GetType(destSym), dstTypeId));
    VIR_Operand_SetOpKind(VIR_Inst_GetDest(virVecGetInst), VIR_OPND_SYMBOL);
    VIR_Operand_SetTypeId(VIR_Inst_GetDest(virVecGetInst), VIR_Symbol_GetTypeId(destSym));
    VIR_Operand_SetSym(VIR_Inst_GetDest(virVecGetInst), destSym);

    /*src0 is instrinsic Kind */
    VIR_Operand_SetIntrinsic(VIR_Inst_GetSource(virVecGetInst, 0), VIR_IK_vecGet);
    /* src1 is parameter */
    VIR_Function_NewParameters(spv->virFunction, argCount, &parmOpnd);
    VIR_Operand_SetParameters(VIR_Inst_GetSource(virVecGetInst, 1), parmOpnd);

    /* fill two parmOpnds */
    src0Opnd = parmOpnd->args[0];
    VIR_Operand_SetSwizzle(src0Opnd, src0Swizzle);
    VIR_Operand_SetSym(src0Opnd, src0Sym);
    VIR_Operand_SetOpKind(src0Opnd, VIR_OPND_SYMBOL);
    VIR_Operand_SetTypeId(src0Opnd,  VIR_Symbol_GetTypeId(src0Sym));
    VIR_Operand_SetPrecision(src0Opnd, VIR_PRECISION_HIGH);
    VIR_Operand_SetRoundMode(src0Opnd, VIR_ROUND_DEFAULT);
    VIR_Operand_SetModifier(src0Opnd, VIR_MOD_NONE);

    src1Opnd = parmOpnd->args[1];
    VIR_Operand_SetSwizzle(src1Opnd, src1Swizzle);
    VIR_Operand_SetSym(src1Opnd, src1Sym);
    VIR_Operand_SetOpKind(src1Opnd, VIR_OPND_SYMBOL);
    VIR_Operand_SetTypeId(src1Opnd,  VIR_Symbol_GetTypeId(src1Sym));
    VIR_Operand_SetPrecision(src1Opnd, VIR_PRECISION_HIGH);
    VIR_Operand_SetRoundMode(src1Opnd, VIR_ROUND_DEFAULT);
    VIR_Operand_SetModifier(src1Opnd, VIR_MOD_NONE);

    return virErrCode;
}

static VSC_ErrCode __SpvEmitLoad(gcSPV spv, VIR_Shader * virShader)
{
    gctUINT i, j, paramToFunc;
    VIR_OpCode virOpcode;
    SpvId resultId = SPV_INVALID_ID;
    SpvId resultTypeId = SPV_INVALID_ID;
    SpvOp opCode = (SpvOp)spv->opCode;
    VIR_NameId nameId;
    VIR_Symbol * dstVirSym = gcvNULL;
    VIR_Type *  dstVirType = gcvNULL;
    VIR_TypeId dstVirTypeId = VIR_TYPE_UNKNOWN;
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VIR_SymId virSymId;
    VIR_Operand * operand;
    VIR_Instruction * virInst;
    VIR_Swizzle virSwizzle;
    VIR_Enable virEnableMask;
    gctBOOL isArray = gcvFALSE;
    gctBOOL useLoadToAccessBlock = gcvFALSE;
    gctBOOL hasDest;
    gctBOOL isWorkGroup = gcvFALSE;
    gctUINT virOpndId = 0;
    /*  following variables are copy of dest of MOV
        and only used if spv->operand[0]
     *  visiting vector component by variable
     */
    VIR_Symbol *dstVirScalarSym = gcvNULL;
    VIR_Type   *dstVirScalarSymType= gcvNULL;
    VIR_TypeId  dstVirScalarSymTypeId = VIR_TYPE_UNKNOWN;

    /* map spvopcode to vir opcode, if not support, add more inst */
    virOpcode = VIR_OP_MOV;

    hasDest = VIR_OPCODE_hasDest(virOpcode);

    gcmASSERT(spv->opCode == SpvOpLoad);

    if (spv->resultId)
    {
        /* if this is a opLoad and operand is Image, skip the emit instruction */
        if (SPV_ID_TYPE(spv->operands[0]) == SPV_ID_TYPE_SYMBOL)
        {
            SpvId spvTypeId = SPV_ID_SYM_SPV_TYPE(spv->operands[0]);

            while (SPV_ID_TYPE_IS_POINTER(spvTypeId))
            {
                spvTypeId = SPV_ID_TYPE_POINTER_OBJECT_SPV_TYPE(spvTypeId);
            }

            if (SPV_ID_TYPE(spvTypeId) == SPV_ID_TYPE_TYPE &&
                (SPV_ID_TYPE_IS_IMAGE(spvTypeId) || SPV_ID_TYPE_IS_SAMPLER(spvTypeId)))
            {
                SPV_ID_CLONE(spv->operands[0], spv->resultId);
                /* record the sampler */
                if (SPV_ID_TYPE_IS_SAMPLER(spvTypeId))
                {
                    SPV_ID_SYM_SAMPLEDIMAGE_SAMPLER(spv->resultId) = spv->operands[0];
                }
                return virErrCode;
            }
        }

        resultId = spv->resultId;
        resultTypeId = spv->resultTypeId;

        dstVirTypeId = SPV_ID_TYPE_VIR_TYPE_ID(resultTypeId);
        dstVirType = SPV_ID_TYPE_VIR_TYPE(resultTypeId);

        isWorkGroup = SPV_ID_SYM_IS_WORKGROUP(spv->operands[0]);
    }
    else
    {
        gcmASSERT(!hasDest);

        dstVirTypeId = VIR_TYPE_UNKNOWN;
        dstVirType = gcvNULL;
    }

    /* If this instruction has dest, generate symbol if needed. */
    if (hasDest)
    {
        nameId = SPV_ID_VIR_NAME_ID(resultId);
        /* Check if we need to allocate a new symbol for this dest. */
        if (nameId == VIR_INVALID_ID)
        {
            __SpvAddIdSymbol(spv, virShader, gcvNULL, resultId, resultTypeId, VIR_SYM_VARIABLE, VIR_STORAGE_GLOBAL, gcvFALSE);
        }

        virSymId = SPV_ID_VIR_SYM_ID(resultId);
        if (VIR_Id_isFunctionScope(virSymId))
        {
            dstVirSym = VIR_Function_GetSymFromId(spv->virFunction, virSymId);
        }
        else
        {
            dstVirSym = VIR_Shader_GetSymFromId(virShader, virSymId);
        }
    }

    /* copy dstVirsym to dstVirScalar and create a vec symbol as the dstVirSym
     * intrinsic VECGET will be added after LOAD/mov if
     * spv->operand[0] visiting vector component by variable
     */
    if (hasDest && SPV_ID_SYM_OFFSET_ACCESSVECCOMPBYVARIABLE(spv->operands[0]))
    {
       gctCHAR name[32];
       VIR_NameId nameId;
       VIR_SymId tempSymId;
       gctUINT    offset = 0;
       /* save VirSym/type to copy variable*/
       dstVirScalarSym = dstVirSym;
       dstVirScalarSymType = dstVirType;
       dstVirScalarSymTypeId = dstVirTypeId;
       /* create a new VEC variable as the dest of MOV */
       gcoOS_PrintStrSafe(name, 32, &offset, "_spv_temp_vec_%d", resultId);
       dstVirTypeId = SPV_ID_SYM_OFFSET_VECTYPE(spv->operands[0]);
       dstVirType = VIR_Shader_GetTypeFromId(virShader, dstVirTypeId);
       VIR_Shader_AddString(virShader, name, &nameId);
       VIR_Shader_AddSymbol(virShader,
                    VIR_SYM_VARIABLE,
                    nameId,
                    dstVirType,
                    VIR_STORAGE_GLOBAL,
                    &tempSymId);
        dstVirSym = VIR_Shader_GetSymFromId(virShader, tempSymId);
        VIR_Symbol_SetFlag(dstVirSym, VIR_SYMFLAG_WITHOUT_REG);
    }

    /* check if it is block, only works for accesschain for now */
    if (SPV_ID_TYPE(spv->operands[0]) == SPV_ID_TYPE_SYMBOL)
    {
        SpvId spvTypeId = SPV_ID_SYM_SRC_SPV_TYPE(spv->operands[0]);

        if (SPV_ID_TYPE_IS_ARRAY(spvTypeId))
        {
            isArray = gcvTRUE;
        }

        useLoadToAccessBlock = __SpvUseLoadStoreToAccessBlock(spv, virShader, spv->operands[0]);

        /* Change MOV to LOAD/STORE if needed. */
        if (useLoadToAccessBlock)
        {
            virOpcode = VIR_OP_LOAD;
        }

        if (SPV_ID_SYM_PER_VERTEX(spv->operands[0]))
        {
            virOpcode = VIR_OP_ATTR_LD;
        }
    }

    if (isWorkGroup &&
        useLoadToAccessBlock &&
        SPV_ID_SYM_BLOCK_OFFSET_TYPE(spv->operands[0]) == VIR_SYM_UNKNOWN &&
        SPV_ID_SYM_OFFSET_TYPE(spv->operands[0]) == VIR_SYM_UNKNOWN &&
        SPV_ID_SYM_VECTOR_OFFSET_TYPE(spv->operands[0]) == VIR_SYM_UNKNOWN)
    {
        /* If this is work group symbol and original opcode is mov, we need envaluate offset information.
        We need fake a accesschain, which source is shared ssbo, dest is the reg offset of this ssbo */
        VIR_Symbol * baseSymbol;
        gctUINT accessChain;
        VIR_SymbolKind accessChainType;
        gctUINT accessChainLength = 1;
        gctUINT i = 0;
        VIR_AC_OFFSET_INFO virAcOffsetInfo;
        gctUINT memberIndex = ~0U;
        SpvId src = spv->operands[0];

        gcoOS_ZeroMemory(&virAcOffsetInfo, gcmSIZEOF(VIR_AC_OFFSET_INFO));

        baseSymbol = VIR_Shader_GetSymFromId(virShader, SPV_WORKGROUP_INFO()->sharedSboSymId);

        for (i = 0; i < SPV_WORKGROUP_INFO()->memberCount; i++)
        {
            if (src == SPV_WORKGROUP_INFO()->sboMembers[i])
            {
                memberIndex = i;
                break;
            }
        }

        if (memberIndex == -1)
        {
            /* cannot find the source, return error */
            return VSC_ERR_INVALID_DATA;
        }

        /* Set the index type and value. */
        accessChainType = VIR_SYM_CONST;
        accessChain = SPV_WORKGROUP_INFO()->sboMemberOffset[memberIndex];

        VIR_Operand_EvaluateOffsetByAccessChain(
            virShader,
            spv->virFunction,
            src,
            baseSymbol,
            &accessChain,
            &accessChainType,
            accessChainLength,
            &virAcOffsetInfo);

        SPV_SET_IDDESCRIPTOR_SPV_OFFSET(spv, src, virAcOffsetInfo);
    }

    /* Check function parameters. */
    SPV_ID_SYM_USED_LOAD_AS_DEST(spv->resultId) = gcvTRUE;
    paramToFunc = SPV_ID_SYM_PARAM_TO_FUNC(spv->operands[0]);
    if (paramToFunc)
    {
        for (i = 0; i < SPV_ID_FUNC_CALLER_NUM(paramToFunc); i++)
        {
            for (j = 0 ; j < SPV_ID_FUNC_ARG_NUM(paramToFunc); j++)
            {
                if (spv->operands[0] == SPV_ID_FUNC_CALLER_SPV_ARG(paramToFunc, i)[j])
                {
                    if (SPV_ID_FUNC_ARG_STORAGE(paramToFunc)[j] == SPV_PARA_IN)
                    {
                        SPV_ID_FUNC_ARG_STORAGE(paramToFunc)[j] = SPV_PARA_INOUT;
                    }
                    else
                    {
                        SPV_ID_FUNC_ARG_STORAGE(paramToFunc)[j] = SPV_PARA_OUT;
                    }
                    SPV_ID_INITIALIZED(spv->operands[0]) = gcvTRUE;
                }
            }
        }
    }

    virEnableMask = __SpvGenEnable(spv, dstVirType, resultTypeId);

    if (virOpcode == VIR_OP_ATTR_LD)
    {
        if (SPV_ID_SYM_VECTOR_OFFSET_TYPE(spv->operands[0]) == VIR_SYM_CONST)
        {
            spv->idDescriptor[spv->resultId].u.sym.offsetInfo.virAcOffsetInfo.vectorIndexType = VIR_SYM_CONST;
            spv->idDescriptor[spv->resultId].u.sym.offsetInfo.virAcOffsetInfo.vectorIndex =
                SPV_ID_SYM_VECTOR_OFFSET_VALUE(spv->operands[0]);
        }
    }

    VIR_Function_AddInstruction(spv->virFunction,
        virOpcode,
        dstVirTypeId,
        &virInst);

    VIR_Inst_SetConditionOp(virInst, __SpvOpCode2VIRCop(opCode));

    if (hasDest)
    {
        operand = VIR_Inst_GetDest(virInst);
        VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
        VIR_Operand_SetEnable(operand, virEnableMask);
        VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
        VIR_Operand_SetTypeId(operand, dstVirTypeId);
        VIR_Operand_SetSym(operand, dstVirSym);
        VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
        __SetAccessChainOffsetToOperand(spv, spv->resultId, operand, SpvOffsetType_Normal);
    }

    virOpndId = 0;

    virSwizzle = __SpvID2Swizzle(spv, spv->operands[0]);

    if (SPV_ID_TYPE(spv->operands[0]) == SPV_ID_TYPE_SYMBOL)
    {
        SpvId spvOperand = spv->operands[0];
        SpvId resultTypeId = spv->operands[0];

        operand = virInst->src[virOpndId];

        if (virOpcode == VIR_OP_ATTR_LD)
        {
            VIR_SymbolKind  blockIndexType = SPV_ID_SYM_BLOCK_OFFSET_TYPE(spvOperand);
            gctUINT         blockOffset = (SPV_ID_SYM_BLOCK_OFFSET_VALUE(spvOperand) == VIR_INVALID_ID) ? 0 : SPV_ID_SYM_BLOCK_OFFSET_VALUE(spvOperand);
            VIR_SymbolKind  baseOffsetType = SPV_ID_SYM_OFFSET_TYPE(spvOperand);
            gctUINT         baseOffset = (SPV_ID_SYM_OFFSET_VALUE(spvOperand) == VIR_INVALID_ID) ? 0 : SPV_ID_SYM_OFFSET_VALUE(spvOperand);

            VIR_Operand_SetSwizzle(operand, virSwizzle);
            VIR_Operand_SetSym(operand, SPV_ID_VIR_SYM(spvOperand));
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetTypeId(operand, SPV_ID_VIR_TYPE_ID(resultTypeId));
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);

            virOpndId++;
            operand = virInst->src[virOpndId];

            /* we need record invocationId */
            if (blockIndexType == VIR_SYM_VARIABLE)
            {
                VIR_Operand_SetSym(operand, VIR_Shader_GetSymFromId(virShader, blockOffset));
                VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_X);
                VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
                VIR_Operand_SetTypeId(operand, VIR_Symbol_GetTypeId(VIR_Shader_GetSymFromId(virShader, blockOffset)));
            }
            else if (blockIndexType == VIR_SYM_CONST)
            {
                VIR_ScalarConstVal constVal;
                constVal.uValue = blockOffset;
                VIR_Operand_SetImmediate(operand, VIR_TYPE_UINT32, constVal);
                VIR_Operand_SetTypeId(operand, VIR_TYPE_UINT32);
                VIR_Operand_SetSwizzle(operand, virSwizzle);
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }

            virOpndId++;

            operand = virInst->src[virOpndId];

            if (baseOffsetType == VIR_SYM_VARIABLE)
            {
                VIR_Symbol *baseOffsetSym = gcvNULL;
                if (isWorkGroup)
                {
                    __SpvInsertWorkGroupOffsetInst(spv, virShader, virInst, baseOffset, baseOffsetType);
                        baseOffsetSym = VIR_Shader_GetSymFromId(virShader, SPV_WORKGROUP_INFO()->curOffsetSymId);
                }
                else
                {
                    baseOffsetSym = VIR_Shader_GetSymFromId(virShader, baseOffset);
                }
                VIR_Operand_SetSym(operand, baseOffsetSym);
                VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_X);
                VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
                VIR_Operand_SetTypeId(operand, VIR_Symbol_GetTypeId(baseOffsetSym));
            }
            else if (baseOffsetType == VIR_SYM_CONST)
            {
                if (isWorkGroup)
                {
                    /* for workgroup, we need add group offset */
                    __SpvInsertWorkGroupOffsetInst(spv, virShader, virInst, baseOffset, baseOffsetType);
                    VIR_Operand_SetSym(operand, VIR_Shader_GetSymFromId(virShader, SPV_WORKGROUP_INFO()->curOffsetSymId));
                    VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
                    VIR_Operand_SetTypeId(operand, SPV_ID_VIR_TYPE_ID(resultTypeId));
                }
                else
                {
                    VIR_ScalarConstVal constVal;
                    constVal.uValue = baseOffset;
                    VIR_Operand_SetImmediate(operand, VIR_TYPE_UINT32, constVal);
                }
                VIR_Operand_SetTypeId(operand, VIR_TYPE_UINT32);
                VIR_Operand_SetSwizzle(operand, virSwizzle);
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);

            /* add instruction "intrinsic vecget" */
            if (dstVirSym && SPV_ID_SYM_OFFSET_ACCESSVECCOMPBYVARIABLE(spv->operands[0]))
            {
                VIR_Swizzle param0swizzle;
                VIR_Symbol  *vecIdxSym = gcvNULL;
                gcmASSERT(VIR_SYM_VARIABLE == SPV_ID_SYM_VECTOR_OFFSET_TYPE(spv->operands[0]));
                vecIdxSym = VIR_Shader_GetSymFromId(virShader, SPV_ID_SYM_VECTOR_OFFSET_VALUE(spv->operands[0]));
                gcmASSERT(dstVirSym && VIR_Type_isVector(VIR_Symbol_GetType(dstVirSym)));    /* first parameter should be vector type */
                gcmASSERT(vecIdxSym && VIR_Type_isScalar(VIR_Symbol_GetType(vecIdxSym)));    /* second parameter should be scalar */
                param0swizzle = VIR_Swizzle_GenSwizzleByComponentCount(VIR_GetTypeComponents(VIR_Symbol_GetTypeId(dstVirSym)));
                __spvGenerateInstrinsicVecGet(spv, dstVirScalarSym, dstVirSym, param0swizzle, vecIdxSym, VIR_SWIZZLE_XXXX);
            }
        }
        else if (useLoadToAccessBlock)
        {
            VIR_Swizzle     swizzle = VIR_Swizzle_GetChannel(virSwizzle, 0);
            VIR_SymbolKind  baseOffsetType = SPV_ID_SYM_OFFSET_TYPE(spvOperand);
            gctUINT         baseOffset = (SPV_ID_SYM_OFFSET_VALUE(spvOperand) == VIR_INVALID_ID) ? 0 : SPV_ID_SYM_OFFSET_VALUE(spvOperand);

            swizzle = swizzle << 2 | swizzle << 4 | swizzle << 6;

            /* for opload and is block, we need set src0 to base address,
            if it is array, we also need set block offset*/
            VIR_Operand_SetSwizzle(operand, swizzle);
            if (isWorkGroup)
            {
                VIR_Operand_SetSym(operand, VIR_Shader_GetSymFromId(virShader, SPV_WORKGROUP_INFO()->sharedSboSymId));
            }
            else
            {
                VIR_Operand_SetSym(operand, SPV_ID_VIR_SYM(spvOperand));
            }
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetTypeId(operand, VIR_TYPE_UINT32);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);

            __SetAccessChainOffsetToOperand(
                spv,
                spvOperand,
                operand,
                isArray ? SpvOffsetType_UBO_Array : SpvOffsetType_None);

            virOpndId++;

            /* for opload and is block, src1 need set the base offset */
            operand = virInst->src[virOpndId];

            if (baseOffsetType == VIR_SYM_VARIABLE)
            {
                if (isWorkGroup)
                {
                    __SpvInsertWorkGroupOffsetInst(spv, virShader, virInst, baseOffset, baseOffsetType);
                    VIR_Operand_SetSym(operand, VIR_Shader_GetSymFromId(virShader, SPV_WORKGROUP_INFO()->curOffsetSymId));
                }
                else
                {
                    VIR_Operand_SetSym(operand, VIR_Shader_GetSymFromId(virShader, baseOffset));
                }
                VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_X);
                VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            }
            else if (baseOffsetType == VIR_SYM_CONST)
            {
                if (isWorkGroup)
                {
                    /* for workgroup, we need add group offset */
                    __SpvInsertWorkGroupOffsetInst(spv, virShader, virInst, baseOffset, baseOffsetType);
                    VIR_Operand_SetSym(operand, VIR_Shader_GetSymFromId(virShader, SPV_WORKGROUP_INFO()->curOffsetSymId));
                    VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
                    VIR_Operand_SetTypeId(operand, SPV_ID_VIR_TYPE_ID(resultTypeId));
                }
                else
                {
                    VIR_ScalarConstVal constVal;
                    constVal.uValue = baseOffset;
                    VIR_Operand_SetImmediate(operand, VIR_TYPE_UINT32, constVal);
                }
                VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_X);
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }
            VIR_Operand_SetTypeId(operand, VIR_TYPE_UINT32);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);

            /* if it's loading a column from a row major matrix, we need multiple loads here */
            if(SPV_ID_SYM_IS_ROW_MAJOR_MATRIX_COLUMN_INDEX(spvOperand))
            {
                gctCHAR name[32];
                VIR_NameId nameId;
                VIR_SymId tempSymId;
                VIR_Symbol* tempSym;
                VIR_TypeId matrixComponentType = VIR_GetTypeComponentType(dstVirTypeId);
                VIR_Instruction* movInst;
                gctUINT rowCount = VIR_GetTypeComponents(dstVirTypeId);
                gctUINT i, offset = 0;

                gcoOS_PrintStrSafe(name, 32, &offset, "_spv_rmmci_id_%d", resultId);
                VIR_Shader_AddString(virShader, name, &nameId);
                VIR_Shader_AddSymbol(virShader,
                    VIR_SYM_VARIABLE,
                    nameId,
                    VIR_Shader_GetTypeFromId(virShader, matrixComponentType),
                    VIR_STORAGE_GLOBAL,
                    &tempSymId);
                tempSym = VIR_Shader_GetSymFromId(virShader, tempSymId);
                VIR_Symbol_SetFlag(tempSym, VIR_SYMFLAG_WITHOUT_REG);

                VIR_Function_AddInstructionAfter(spv->virFunction, VIR_OP_MOV, matrixComponentType, virInst, gcvTRUE, &movInst);
                VIR_Operand_Copy(VIR_Inst_GetDest(movInst), VIR_Inst_GetDest(virInst));
                VIR_Operand_SetEnable(VIR_Inst_GetDest(movInst), VIR_ENABLE_X);
                VIR_Operand_SetSymbol(VIR_Inst_GetSource(movInst, 0), spv->virFunction, tempSymId);
                VIR_Operand_SetSwizzle(VIR_Inst_GetSource(movInst, 0), VIR_SWIZZLE_XXXX);

                VIR_Operand_SetSymbol(VIR_Inst_GetDest(virInst), spv->virFunction, tempSymId);
                VIR_Operand_SetEnable(VIR_Inst_GetDest(virInst), VIR_ENABLE_X);
                VIR_Operand_SetPrecision(VIR_Inst_GetDest(virInst), VIR_PRECISION_HIGH);
                VIR_Operand_SetRoundMode(VIR_Inst_GetDest(virInst), VIR_ROUND_DEFAULT);
                VIR_Operand_SetModifier(VIR_Inst_GetDest(virInst), VIR_MOD_NONE);

                for(i = 1; i < rowCount; i++)
                {
                    VIR_Instruction* addInst;
                    VIR_Instruction* loadInst;

                    if(VIR_Operand_isImm(VIR_Inst_GetSource(virInst, 1)))
                    {
                        VIR_Function_AddCopiedInstructionAfter(spv->virFunction, virInst, movInst, gcvTRUE, &loadInst);
                        VIR_Operand_SetImmediateUint(VIR_Inst_GetSource(loadInst, 1), VIR_Operand_GetImmediateUint(VIR_Inst_GetSource(virInst, 1)) + SPV_ID_SYM_MATRIX_STRIDE(spvOperand) * i);
                    }
                    else
                    {
                        offset = 0;
                        gcoOS_PrintStrSafe(name, 32, &offset, "_spv_rmmci_offset_id_%d", resultId);
                        VIR_Shader_AddString(virShader, name, &nameId);
                        VIR_Shader_AddSymbol(virShader,
                            VIR_SYM_VARIABLE,
                            nameId,
                            VIR_Shader_GetTypeFromId(virShader, VIR_TYPE_UINT32),
                            VIR_STORAGE_GLOBAL,
                            &tempSymId);
                        tempSym = VIR_Shader_GetSymFromId(virShader, tempSymId);
                        VIR_Symbol_SetFlag(tempSym, VIR_SYMFLAG_WITHOUT_REG);

                        VIR_Function_AddInstructionAfter(spv->virFunction, VIR_OP_ADD, VIR_TYPE_UINT32, movInst, gcvTRUE, &addInst);
                        VIR_Operand_SetSymbol(VIR_Inst_GetDest(addInst), spv->virFunction, tempSymId);
                        VIR_Operand_SetEnable(VIR_Inst_GetDest(addInst), VIR_ENABLE_X);
                        VIR_Operand_SetPrecision(VIR_Inst_GetDest(addInst), VIR_PRECISION_HIGH);
                        VIR_Operand_SetRoundMode(VIR_Inst_GetDest(addInst), VIR_ROUND_DEFAULT);
                        VIR_Operand_SetModifier(VIR_Inst_GetDest(addInst), VIR_MOD_NONE);
                        VIR_Operand_Copy(VIR_Inst_GetSource(addInst, 0), VIR_Inst_GetSource(virInst, 0));
                        VIR_Operand_SetImmediateUint(VIR_Inst_GetSource(addInst, 1), SPV_ID_SYM_MATRIX_STRIDE(spvOperand) * i);

                        VIR_Function_AddCopiedInstructionAfter(spv->virFunction, virInst, addInst, gcvTRUE, &loadInst);
                        VIR_Operand_ReplaceUseOperandWithDef(VIR_Inst_GetDest(addInst), VIR_Inst_GetSource(loadInst, 0));
                    }

                    VIR_Function_AddCopiedInstructionAfter(spv->virFunction, movInst, loadInst, gcvTRUE, &movInst);
                    VIR_Operand_SetEnable(VIR_Inst_GetDest(movInst), (VIR_Enable)(1 << i));
                }
            }
        }
        else
        {
            VIR_Operand_SetSym(operand, SPV_ID_SYM_MAPPING_ARRAY_SYM(spvOperand) ? SPV_ID_SYM_MAPPING_ARRAY_SYM(spvOperand) : SPV_ID_VIR_SYM(spvOperand));
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetTypeId(operand, SPV_ID_VIR_TYPE_ID(resultTypeId));
            VIR_Operand_SetSwizzle(operand, virSwizzle);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);

            if (SPV_ID_SYM_PER_VERTEX(spvOperand))
            {
                __SetAccessChainOffsetToOperand(spv, spvOperand, operand, SpvOffsetType_PER_VERTEX);

            }
            else
            {
                __SetAccessChainOffsetToOperand(spv, spvOperand, operand, SpvOffsetType_Normal);
            }
        }
    }
    else if (SPV_ID_TYPE(spv->operands[0]) == SPV_ID_TYPE_CONST)
    {
        operand = virInst->src[virOpndId];
        VIR_Operand_SetSwizzle(operand, virSwizzle);
        VIR_Operand_SetConstId(operand, SPV_ID_VIR_CONST_ID(spv->operands[0]));
        VIR_Operand_SetOpKind(operand, VIR_OPND_CONST);
        VIR_Operand_SetTypeId(operand, SPV_ID_VIR_TYPE_ID(spv->operands[0]));
        VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
        VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);

        __SetAccessChainOffsetToOperand(spv, spv->operands[0], operand, SpvOffsetType_Normal);
    }
    else
    {
        gcmASSERT(0);
    }

    return virErrCode;
}

static VSC_ErrCode __SpvEmitStore(gcSPV spv, VIR_Shader * virShader)
{
    VIR_OpCode virOpcode;
    SpvId resultId = SPV_INVALID_ID;
    SpvId resultTypeId = SPV_INVALID_ID;
    VIR_Symbol * dstVirSym = gcvNULL;
    VIR_Type *  dstVirType = gcvNULL;
    VIR_TypeId dstVirTypeId = VIR_TYPE_UNKNOWN;
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VIR_SymId virSymId;
    VIR_Operand * operand;
    VIR_Instruction * virInst;
    VIR_Swizzle virSwizzle;
    VIR_Enable virEnableMask;
    gctBOOL isArray = gcvFALSE;
    gctBOOL useLoadToAccessBlock = gcvFALSE;
    gctBOOL hasDest;
    gctBOOL isWorkGroup = gcvFALSE;
    gctUINT spvTypeId;
    SpvId spvOperand;

    /* map spvopcode to vir opcode, if not support, add more inst */
    virOpcode = VIR_OP_MOV;

    hasDest = VIR_OPCODE_hasDest(virOpcode);

    gcmASSERT(spv->opCode == SpvOpStore);
    gcmASSERT(!SPV_ID_SYM_OFFSET_ACCESSVECCOMPBYVARIABLE(spv->operands[0])); /* unsupport vecset yet */
    spvTypeId = SPV_ID_SYM_SPV_TYPE(spv->operands[0]);
    /* First operand is pointer, let's find out the object */
    resultId = SPV_POINTER_VAR_OBJ_SPV_NAME(spv->operands[0]);
    if (SPV_ID_TYPE_IS_POINTER(spvTypeId))
    {
        resultTypeId = SPV_POINTER_VAR_OBJ_SPV_TYPE(spv->operands[0]);
    }
    else
    {
        resultTypeId = spvTypeId;
    }

    dstVirTypeId = SPV_ID_TYPE_VIR_TYPE_ID(resultTypeId);
    dstVirType = SPV_ID_TYPE_VIR_TYPE(resultTypeId);
    isWorkGroup = SPV_ID_SYM_IS_WORKGROUP(spv->operands[0]);

    /* If this instruction has dest, generate symbol if needed. */
    if (hasDest)
    {
        /* Comment result name id check, the store already has a created symbol,
        if this already had symbol has no name, the check will destroy our symbol */
        gcmASSERT(SPV_ID_VIR_SYM(resultId));

        virSymId = SPV_ID_SYM_MAPPING_ARRAY_SYM(resultId) ? VIR_Symbol_GetIndex(SPV_ID_SYM_MAPPING_ARRAY_SYM(resultId)) : SPV_ID_VIR_SYM_ID(resultId);
        if (VIR_Id_isFunctionScope(virSymId))
        {
            dstVirSym = VIR_Function_GetSymFromId(spv->virFunction, virSymId);
        }
        else
        {
            dstVirSym = VIR_Shader_GetSymFromId(virShader, virSymId);
        }
    }

    /* check if it is block, only works for accesschain for now */
    if (SPV_ID_TYPE(spv->operands[0]) == SPV_ID_TYPE_SYMBOL)
    {
        SpvId spvTypeId = SPV_ID_SYM_SRC_SPV_TYPE(spv->operands[0]);

        if (SPV_ID_TYPE_IS_ARRAY(spvTypeId))
        {
            isArray = gcvTRUE;
        }

        useLoadToAccessBlock = __SpvUseLoadStoreToAccessBlock(spv, virShader, spv->operands[0]);

        /* Change MOV to LOAD/STORE if needed. */
        if (useLoadToAccessBlock)
        {
            virOpcode = VIR_OP_STORE;
        }

        if (SPV_ID_SYM_PER_VERTEX(spv->operands[0]))
        {
            virOpcode = VIR_OP_ATTR_ST;
        }
    }

    spvOperand = spv->operands[0];

    if (SPV_ID_TYPE(spv->operands[1]) == SPV_ID_TYPE_SYMBOL ||
        SPV_ID_TYPE(spv->operands[1]) == SPV_ID_TYPE_CONST)
    {
        virEnableMask = __SpvGenEnable(spv, dstVirType, resultTypeId);

        virSwizzle = __SpvID2Swizzle(spv, spv->operands[1]);

        VIR_Function_AddInstruction(spv->virFunction,
            virOpcode,
            dstVirTypeId,
            &virInst);

        VIR_Inst_SetConditionOp(virInst, VIR_COP_ALWAYS);

        if (virOpcode == VIR_OP_ATTR_ST)
        {
            VIR_SymbolKind  blockIndexType = SPV_ID_SYM_BLOCK_OFFSET_TYPE(spvOperand);
            gctUINT         blockOffset = (SPV_ID_SYM_BLOCK_OFFSET_VALUE(spvOperand) == VIR_INVALID_ID) ? 0 : SPV_ID_SYM_BLOCK_OFFSET_VALUE(spvOperand);
            VIR_SymbolKind baseOffsetType = SPV_ID_SYM_OFFSET_TYPE(spvOperand);
            gctUINT baseOffset = (SPV_ID_SYM_OFFSET_VALUE(spvOperand) == VIR_INVALID_ID) ? 0 : SPV_ID_SYM_OFFSET_VALUE(spvOperand);
            VIR_Swizzle swizzle = virSwizzle;

            /* ATTR_ST  Output, InvocationIndex, offset, value */
            /* Set DEST by operands[0]. */
            spvOperand = spv->operands[0];
            operand = VIR_Inst_GetDest(virInst);
            if (SPV_ID_TYPE(spvOperand) == SPV_ID_TYPE_SYMBOL)
            {
                VIR_Operand_SetSym(operand, SPV_ID_VIR_SYM(spvOperand));
                VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            }
            else
            {
                VIR_Operand_SetConstId(operand, SPV_ID_VIR_CONST_ID(spvOperand));
                VIR_Operand_SetOpKind(operand, VIR_OPND_CONST);
            }
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            VIR_Operand_SetEnable(operand, VIR_Swizzle_2_Enable(virSwizzle));
            VIR_Operand_SetTypeId(operand, SPV_ID_VIR_TYPE_ID(spvOperand));

            /* operand[0].blockIndex->virOperand[0](invocation) */
            operand = VIR_Inst_GetSource(virInst, 0);
            if (blockIndexType == VIR_SYM_VARIABLE)
            {
                VIR_Operand_SetSym(operand, VIR_Shader_GetSymFromId(virShader, blockOffset));
                VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_X);
                VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
                VIR_Operand_SetTypeId(operand, VIR_Symbol_GetTypeId(VIR_Shader_GetSymFromId(virShader, blockOffset)));
            }
            else
            {
                VIR_ScalarConstVal constVal;
                constVal.uValue = blockOffset;
                VIR_Operand_SetImmediate(operand, VIR_TYPE_UINT32, constVal);
                VIR_Operand_SetSwizzle(operand, virSwizzle);
            }
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);


            /* operand[0].offset->virOperand[1] */
            operand = VIR_Inst_GetSource(virInst, 1);
            if (baseOffsetType == VIR_SYM_VARIABLE)
            {
                VIR_Symbol *baseOffsetSym = VIR_Shader_GetSymFromId(virShader, baseOffset);
                VIR_Operand_SetSym(operand, baseOffsetSym);
                VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_X);
                VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
                VIR_Operand_SetTypeId(operand, VIR_Symbol_GetTypeId(baseOffsetSym));
            }
            else
            {
                VIR_ScalarConstVal constVal;
                constVal.uValue = baseOffset;
                VIR_Operand_SetImmediate(operand, VIR_TYPE_UINT32, constVal);
                VIR_Operand_SetSwizzle(operand, virSwizzle);
            }
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);

            /* operand[0] -> virOperand[2] */
            spvOperand = spv->operands[1];
            operand = VIR_Inst_GetSource(virInst, 2);
            if (SPV_ID_TYPE(spvOperand) == SPV_ID_TYPE_SYMBOL)
            {
                VIR_Operand_SetSym(operand, SPV_ID_VIR_SYM(spvOperand));
                VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            }
            else
            {
                VIR_Operand_SetConstId(operand, SPV_ID_VIR_CONST_ID(spvOperand));
                VIR_Operand_SetOpKind(operand, VIR_OPND_CONST);
            }
            VIR_Operand_SetSwizzle(operand, swizzle);
            VIR_Operand_SetTypeId(operand, SPV_ID_VIR_TYPE_ID(resultTypeId));
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            __SetAccessChainOffsetToOperand(spv, spvOperand, operand, SpvOffsetType_Normal);
        }
        else if (useLoadToAccessBlock)
        {
            VIR_SymbolKind baseOffsetType = SPV_ID_SYM_OFFSET_TYPE(spvOperand);
            gctUINT baseOffset = (SPV_ID_SYM_OFFSET_VALUE(spvOperand) == VIR_INVALID_ID) ? 0 : SPV_ID_SYM_OFFSET_VALUE(spvOperand);
            VIR_Swizzle swizzle = VIR_Swizzle_GetChannel(virSwizzle, 0);

            swizzle = swizzle << 2 | swizzle << 4 | swizzle << 6;

            /* Set DEST by operands[1]. */
            spvOperand = spv->operands[1];
            operand = VIR_Inst_GetDest(virInst);
            if (SPV_ID_TYPE(spvOperand) == SPV_ID_TYPE_SYMBOL)
            {
                VIR_Operand_SetSym(operand, SPV_ID_VIR_SYM(spvOperand));
                VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            }
            else
            {
                VIR_Operand_SetConstId(operand, SPV_ID_VIR_CONST_ID(spvOperand));
                VIR_Operand_SetOpKind(operand, VIR_OPND_CONST);
            }
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            VIR_Operand_SetEnable(operand, VIR_Swizzle_2_Enable(virSwizzle));
            VIR_Operand_SetTypeId(operand, SPV_ID_VIR_TYPE_ID(spvOperand));
            __SetAccessChainOffsetToOperand(spv, spvOperand, operand, SpvOffsetType_Normal);

            /* for opstore and is block, we need set src0 to base address,
            src1 base offset, src2 is dest value */
            /* Set SOURCE0 by operands[0]. */
            spvOperand = spv->operands[0];
            operand = VIR_Inst_GetSource(virInst, 0);
            VIR_Operand_SetSwizzle(operand, swizzle);
            if (isWorkGroup)
            {
                VIR_Operand_SetSym(operand, VIR_Shader_GetSymFromId(virShader, SPV_WORKGROUP_INFO()->sharedSboSymId));
            }
            else
            {
                VIR_Operand_SetSym(operand, SPV_ID_VIR_SYM(spvOperand));
            }
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetTypeId(operand, SPV_ID_VIR_TYPE_ID(resultTypeId));
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            __SetAccessChainOffsetToOperand(
                spv,
                spvOperand,
                operand,
                isArray ? SpvOffsetType_UBO_Array : SpvOffsetType_None);

            /* Set SOURCE1 by offset. */
            operand = VIR_Inst_GetSource(virInst, 1);

            if (baseOffsetType == VIR_SYM_VARIABLE)
            {
                if (isWorkGroup)
                {
                    /* for workgroup, we need add group offset */
                    __SpvInsertWorkGroupOffsetInst(spv, virShader, virInst, baseOffset, baseOffsetType);
                    VIR_Operand_SetSym(operand, VIR_Shader_GetSymFromId(virShader, SPV_WORKGROUP_INFO()->curOffsetSymId));
                }
                else
                {
                    VIR_Operand_SetSym(operand, VIR_Shader_GetSymFromId(virShader, baseOffset));
                }
                VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_X);
                VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
                VIR_Operand_SetTypeId(operand, SPV_ID_VIR_TYPE_ID(resultTypeId));
            }
            else
            {
                if (isWorkGroup)
                {
                    /* for workgroup, we need add group offset */
                    __SpvInsertWorkGroupOffsetInst(spv, virShader, virInst, baseOffset, baseOffsetType);
                    VIR_Operand_SetSym(operand, VIR_Shader_GetSymFromId(virShader, SPV_WORKGROUP_INFO()->curOffsetSymId));
                    VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
                    VIR_Operand_SetTypeId(operand, SPV_ID_VIR_TYPE_ID(resultTypeId));
                }
                else
                {
                    VIR_ScalarConstVal constVal;
                    constVal.uValue = baseOffset;
                    VIR_Operand_SetImmediate(operand, VIR_TYPE_UINT32, constVal);
                }
                VIR_Operand_SetSwizzle(operand, virSwizzle);
            }
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);

            /* Set SOURCE2 by operands[1]. */
            spvOperand = spv->operands[1];
            operand = VIR_Inst_GetSource(virInst, 2);
            if (SPV_ID_TYPE(spvOperand) == SPV_ID_TYPE_SYMBOL)
            {
                VIR_Operand_SetSym(operand, SPV_ID_VIR_SYM(spvOperand));
                VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            }
            else
            {
                VIR_Operand_SetConstId(operand, SPV_ID_VIR_CONST_ID(spvOperand));
                VIR_Operand_SetOpKind(operand, VIR_OPND_CONST);
            }
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            VIR_Operand_SetSwizzle(operand, virSwizzle);
            VIR_Operand_SetTypeId(operand, SPV_ID_VIR_TYPE_ID(spvOperand));
            __SetAccessChainOffsetToOperand(spv, spvOperand, operand, SpvOffsetType_Normal);
        }
        else
        {
            /* Set DEST by operands[0]. */
            spvOperand = spv->operands[0];
            operand = VIR_Inst_GetDest(virInst);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            VIR_Operand_SetEnable(operand, virEnableMask);
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetTypeId(operand, dstVirTypeId);
            VIR_Operand_SetSym(operand, dstVirSym);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            __SetAccessChainOffsetToOperand(spv, spvOperand, operand, SpvOffsetType_Normal);

            /* Set SOURCE0 by operands[1]. */
            spvOperand = spv->operands[1];
            operand = VIR_Inst_GetSource(virInst, 0);
            VIR_Operand_SetSwizzle(operand, virSwizzle);
            VIR_Operand_SetTypeId(operand, SPV_ID_VIR_TYPE_ID(spvOperand));
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            if (SPV_ID_TYPE(spvOperand) == SPV_ID_TYPE_CONST)
            {
                VIR_Operand_SetConstId(operand, SPV_ID_VIR_CONST_ID(spvOperand));
                VIR_Operand_SetOpKind(operand, VIR_OPND_CONST);
            }
            else
            {
                VIR_Operand_SetSym(operand, SPV_ID_VIR_SYM(spvOperand));
                VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            }
            __SetAccessChainOffsetToOperand(spv, spvOperand, operand, SpvOffsetType_Normal);

            if(VIR_GetTypeComponentType(VIR_Operand_GetTypeId(VIR_Inst_GetDest(virInst))) == VIR_TYPE_BOOLEAN &&
               VIR_GetTypeComponentType(VIR_Operand_GetTypeId(VIR_Inst_GetSource(virInst, 0))) != VIR_TYPE_BOOLEAN)
            {
                VIR_Operand* source1;
                VIR_Operand* source2;

                VIR_Inst_SetOpcode(virInst, VIR_OP_CSELECT);
                VIR_Inst_SetSrcNum(virInst, 3);
                VIR_Function_NewOperand(spv->virFunction, &source1);
                VIR_Function_NewOperand(spv->virFunction, &source2);
                VIR_Inst_SetSource(virInst, 1, source1);
                VIR_Inst_SetSource(virInst, 2, source2);
                VIR_Inst_SetConditionOp(virInst, VIR_COP_NOT_ZERO);
                VIR_Operand_SetImmediateUint(source1, 1);
                VIR_Operand_SetImmediateUint(source2, 0);
            }

            if(SPV_ID_SYM_MAPPING_ARRAY_SYM(resultId))
            {
                VIR_Type* arraySymType = VIR_Symbol_GetType(dstVirSym);
                VIR_Instruction* restoreVectorInst;
                gctUINT i;

                for(i = 0; i < VIR_Type_GetArrayLength(arraySymType); i++)
                {
                    VIR_Function_AddInstruction(spv->virFunction,
                                                VIR_OP_MOV,
                                                dstVirTypeId,
                                                &restoreVectorInst);

                    VIR_Operand_SetSymbol(VIR_Inst_GetDest(restoreVectorInst), spv->virFunction, SPV_ID_VIR_SYM_ID(resultId));
                    VIR_Operand_SetEnable(VIR_Inst_GetDest(restoreVectorInst), (VIR_Enable)(1 << i));
                    VIR_Operand_SetSym(VIR_Inst_GetSource(restoreVectorInst, 0), dstVirSym);
                    VIR_Operand_SetOpKind(VIR_Inst_GetSource(restoreVectorInst, 0), VIR_OPND_SYMBOL);
                    VIR_Operand_SetTypeId(VIR_Inst_GetSource(restoreVectorInst, 0), dstVirTypeId);
                    VIR_Operand_SetIsConstIndexing(VIR_Inst_GetSource(restoreVectorInst, 0), 1);
                    VIR_Operand_SetRelIndex(VIR_Inst_GetSource(restoreVectorInst, 0), i);
                }
            }
        }
    }
    else
    {
        gcmASSERT(0);
    }

    /* if this is parameter, record */
    SPV_ID_SYM_USED_STORE_AS_DEST(spv->operands[0]) = gcvTRUE;

    return virErrCode;
}

static VSC_ErrCode __SpvEmitArrayLength(gcSPV spv, VIR_Shader * virShader)
{
    VIR_OpCode virOpcode;
    SpvId resultId = SPV_INVALID_ID;
    SpvId resultTypeId = SPV_INVALID_ID;
    VIR_NameId nameId;
    VIR_Symbol * dstVirSym = gcvNULL;
    VIR_Type *  dstVirType = gcvNULL;
    VIR_TypeId dstVirTypeId = VIR_TYPE_UNKNOWN;
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VIR_SymId virSymId;
    VIR_Operand * operand;
    VIR_Instruction * virInst;
    VIR_Swizzle virSwizzle;
    VIR_Enable virEnableMask;
    gctBOOL hasDest;
    SpvId spvOperand;

    /* map spvopcode to vir opcode, if not support, add more inst */
    virOpcode = VIR_OP_MOV;

    hasDest = VIR_OPCODE_hasDest(virOpcode);

    if (spv->resultId)
    {
        resultId = spv->resultId;
        resultTypeId = spv->resultTypeId;

        dstVirTypeId = SPV_ID_TYPE_VIR_TYPE_ID(resultTypeId);
        dstVirType = SPV_ID_TYPE_VIR_TYPE(resultTypeId);
    }
    else
    {
        gcmASSERT(!hasDest);

        dstVirTypeId = VIR_TYPE_UNKNOWN;
        dstVirType = gcvNULL;
    }

    /* If this instruction has dest, generate symbol if needed. */
    if (hasDest)
    {
        nameId = SPV_ID_VIR_NAME_ID(resultId);
        /* Check if we need to allocate a new symbol for this dest. */
        if (nameId == VIR_INVALID_ID)
        {
            __SpvAddIdSymbol(spv, virShader, gcvNULL, resultId, resultTypeId, VIR_SYM_VARIABLE, VIR_STORAGE_GLOBAL, gcvFALSE);
        }

        virSymId = SPV_ID_VIR_SYM_ID(resultId);
        if (VIR_Id_isFunctionScope(virSymId))
        {
            dstVirSym = VIR_Function_GetSymFromId(spv->virFunction, virSymId);
        }
        else
        {
            dstVirSym = VIR_Shader_GetSymFromId(virShader, virSymId);
        }
    }

    /* Length of a run-time array.
    src0: is a structure, whose last member is a run-time array.
    src1: is the last member number of structure and must have a type from
    */
    spvOperand = spv->operands[0];

    virEnableMask = __SpvGenEnable(spv, dstVirType, resultTypeId);

    virSwizzle = VIR_SWIZZLE_YYYY;

    VIR_Function_AddInstruction(spv->virFunction,
        virOpcode,
        dstVirTypeId,
        &virInst);

    VIR_Inst_SetConditionOp(virInst, VIR_COP_ALWAYS);

    /* Dest */
    operand = VIR_Inst_GetDest(virInst);
    VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
    VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
    VIR_Operand_SetEnable(operand, virEnableMask);
    VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
    VIR_Operand_SetTypeId(operand, dstVirTypeId);
    VIR_Operand_SetSym(operand, dstVirSym);
    VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
    __SetAccessChainOffsetToOperand(spv, spv->resultId, operand, SpvOffsetType_Normal);

    /* operands[0] */
    if (SPV_ID_TYPE(spvOperand) == SPV_ID_TYPE_SYMBOL)
    {
        VIR_Symbol *virSym0 = SPV_ID_VIR_SYM(spvOperand);

        if (VIR_Symbol_GetKind(virSym0) == VIR_SYM_SBO)
        {
            VIR_StorageBlock    *storageBlock = gcvNULL;

            storageBlock = virSym0->u2.sbo;
            if (VIR_IB_GetFlags(storageBlock) & VIR_IB_UNSIZED)
            {
                operand = VIR_Inst_GetSource(virInst, 0);
                VIR_Operand_SetSwizzle(operand, virSwizzle);
                VIR_Operand_SetTypeId(operand, SPV_ID_VIR_TYPE_ID(spvOperand));
                VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
                VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
                VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
                VIR_Operand_SetSym(operand, virSym0);
                VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
                __SetAccessChainOffsetToOperand(spv, spvOperand, operand, SpvOffsetType_Normal);

                /* spvOperand1 is last field: to-check */
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    return virErrCode;
}

static VSC_ErrCode __SpvEmitAtomic(gcSPV spv, VIR_Shader * virShader)
{
    gctUINT i;
    VIR_OpCode virOpcode;
    SpvId resultId = SPV_INVALID_ID;
    SpvId resultTypeId = SPV_INVALID_ID;
    SpvOp opCode = (SpvOp)spv->opCode;
    VIR_NameId nameId;
    VIR_Symbol * dstVirSym = gcvNULL;
    VIR_Type *  dstVirType = gcvNULL;
    VIR_TypeId dstVirTypeId = VIR_TYPE_UNKNOWN;
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VIR_SymId virSymId;
    VIR_Operand * operand;
    VIR_Instruction * virInst;
    VIR_Swizzle virSwizzle;
    VIR_Enable virEnableMask;
    gctBOOL hasDest;
    gctBOOL isWorkGroup = gcvFALSE;
    gctUINT virOpndId;

    /* map spvopcode to vir opcode, if not support, add more inst */
    virOpcode = SPV_OPCODE_2_VIR_OPCODE(spv->opCode);

    hasDest = VIR_OPCODE_hasDest(virOpcode);

    if (spv->resultId)
    {
        resultId = spv->resultId;
        resultTypeId = spv->resultTypeId;

        dstVirTypeId = SPV_ID_TYPE_VIR_TYPE_ID(resultTypeId);
        dstVirType = SPV_ID_TYPE_VIR_TYPE(resultTypeId);

        isWorkGroup = SPV_ID_SYM_IS_WORKGROUP(spv->operands[0]);
    }
    else
    {
        hasDest = gcvFALSE;
        dstVirTypeId = VIR_TYPE_UNKNOWN;
        dstVirType = gcvNULL;
    }

    /* If this instruction has dest, generate symbol if needed. */
    if (hasDest)
    {
        nameId = SPV_ID_VIR_NAME_ID(resultId);
        /* Check if we need to allocate a new symbol for this dest. */
        if (nameId == VIR_INVALID_ID)
        {
            __SpvAddIdSymbol(spv, virShader, gcvNULL, resultId, resultTypeId, VIR_SYM_VARIABLE, VIR_STORAGE_GLOBAL, gcvFALSE);
        }

        virSymId = SPV_ID_VIR_SYM_ID(resultId);
        if (VIR_Id_isFunctionScope(virSymId))
        {
            dstVirSym = VIR_Function_GetSymFromId(spv->virFunction, virSymId);
        }
        else
        {
            dstVirSym = VIR_Shader_GetSymFromId(virShader, virSymId);
        }
    }

    virOpndId = 0;

    virEnableMask = __SpvGenEnable(spv, dstVirType, resultTypeId);

    VIR_Function_AddInstruction(spv->virFunction,
        virOpcode,
        dstVirTypeId,
        &virInst);

    VIR_Inst_SetConditionOp(virInst, __SpvOpCode2VIRCop(opCode));

    if (hasDest)
    {
        operand = VIR_Inst_GetDest(virInst);
        VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
        VIR_Operand_SetEnable(operand, virEnableMask);
        VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
        VIR_Operand_SetTypeId(operand, dstVirTypeId);
        VIR_Operand_SetSym(operand, dstVirSym);
        VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
        __SetAccessChainOffsetToOperand(spv, spv->resultId, operand, SpvOffsetType_Normal);
    }

    /*
    ** For VIR_OP_ATOMCMPXCHG, the Value is saved in the x channel of src2, the Comparator is saved in the y channel of src2.
    ** so we need to create a new symbol to hold this fixed value.
    */
    if (virOpcode == VIR_OP_ATOMCMPXCHG)
    {
        gctUINT offset = 0;
        VIR_SymId newSymId = VIR_INVALID_ID;
        VIR_Symbol * newSymbol = gcvNULL;
        VIR_TypeId newSymTypeId = VIR_TypeId_ComposeNonOpaqueType(dstVirTypeId, 2, 1);
        VIR_Instruction * newInst = gcvNULL;

        gcmASSERT(spv->operandSize == 3);

        /* Create a new symbol. */
        gcoOS_MemFill(spv->virName, 0, SPV_VIR_NAME_SIZE * gcmSIZEOF(gctCHAR));
        gcoOS_PrintStrSafe(spv->virName, SPV_VIR_NAME_SIZE, &offset, "#sh_atomcmpxchg_%d", spv->resultId);
        VIR_Shader_AddString(virShader, spv->virName, &nameId);

        VIR_Shader_AddSymbol(
            virShader,
            VIR_SYM_VARIABLE,
            nameId,
            VIR_Shader_GetTypeFromId(virShader, newSymTypeId),
            VIR_STORAGE_GLOBAL,
            &newSymId);
        newSymbol = VIR_Shader_GetSymFromId(virShader, newSymId);
        VIR_Symbol_SetFlag(newSymbol,VIR_SYMFLAG_WITHOUT_REG);

        /* Add two instructions to assign the data. */
        for (i = 0; i < 2; i++)
        {
            SpvId spvOperand;

            VIR_Function_AddInstructionBefore(spv->virFunction,
                                              VIR_OP_MOV,
                                              dstVirTypeId,
                                              virInst,
                                              gcvTRUE,
                                              &newInst);
            operand = VIR_Inst_GetDest(newInst);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            VIR_Operand_SetEnable(operand, (i == 0) ? VIR_ENABLE_X : VIR_ENABLE_Y);
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetTypeId(operand, dstVirTypeId);
            VIR_Operand_SetSym(operand, newSymbol);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);

            spvOperand = spv->operands[i + 1];
            virSwizzle = __SpvID2Swizzle(spv, spvOperand);
            operand = VIR_Inst_GetSource(newInst, 0);
            VIR_Operand_SetTypeId(operand, SPV_ID_VIR_TYPE_ID(resultTypeId));
            VIR_Operand_SetSwizzle(operand, virSwizzle);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            __SetAccessChainOffsetToOperand(spv, spvOperand, operand, SpvOffsetType_Normal);
            if (SPV_ID_TYPE(spvOperand) == SPV_ID_TYPE_SYMBOL)
            {
                VIR_Operand_SetSym(operand, SPV_ID_VIR_SYM(spvOperand));
                VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            }
            else if (SPV_ID_TYPE(spvOperand) == SPV_ID_TYPE_CONST)
            {
                VIR_Operand_SetConstId(operand, SPV_ID_VIR_CONST_ID(spvOperand));
                VIR_Operand_SetOpKind(operand, VIR_OPND_CONST);
            }
            else
            {
                gcmASSERT(0);
            }
        }

        /* Set the new value to the src2 of atom inst. */
        operand = VIR_Inst_GetSource(virInst, 2);
        VIR_Operand_SetTypeId(operand, newSymTypeId);
        VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XYYY);
        VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
        VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
        VIR_Operand_SetSym(operand, newSymbol);
        VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);

        /* Remap the operand. */
        spv->operandSize -= 2;
    }
    else if (spv->opCode == SpvOpAtomicIIncrement ||
             spv->opCode == SpvOpAtomicIDecrement)
    {
        operand = VIR_Inst_GetSource(virInst, 2);
        VIR_Operand_SetImmediateInt(operand, 1);
    }
    else if (spv->opCode == SpvOpAtomicLoad)
    {
        operand = VIR_Inst_GetSource(virInst, 2);
        VIR_Operand_SetImmediateInt(operand, 0);
    }

    /* atomic (similar to load/store) generates
       atomic dst, src0 (base), src1 (offset), src2 (value) */
    for (i = 0; i < spv->operandSize; i++)
    {
        SpvId spvOperand = spv->operands[i];
        SpvId resultTypeId = spv->operands[i];

        virSwizzle = __SpvID2Swizzle(spv, spvOperand);

        /* if operand 0 is workgroup, we need use ssbo and calculate offset */
        if (isWorkGroup && i == 0)
        {
            gctUINT memberIndex = ~0U;
            gctUINT memberWalker = 0;

            for (memberWalker = 0; memberWalker < SPV_WORKGROUP_INFO()->memberCount; memberWalker++)
            {
                if (spv->operands[0] == SPV_WORKGROUP_INFO()->sboMembers[memberWalker])
                {
                    memberIndex = memberWalker;
                    break;
                }
            }

            if (memberIndex == -1)
            {
                /* this is not expected */
                return VSC_ERR_INVALID_DATA;
            }

            __SpvInsertWorkGroupOffsetInst(spv, virShader, virInst, SPV_WORKGROUP_INFO()->sboMemberOffset[memberIndex], VIR_SYM_CONST);
        }

        if (SPV_ID_TYPE(spv->operands[i]) == SPV_ID_TYPE_SYMBOL ||
            SPV_ID_TYPE(spv->operands[i]) == SPV_ID_TYPE_FUNC_DEFINE)
        {
            VIR_SymbolKind  baseOffsetType = SPV_ID_SYM_OFFSET_TYPE(spvOperand);
            gctUINT         baseOffset = SPV_ID_SYM_OFFSET_VALUE(spvOperand);

            if (SPV_ID_TYPE(spv->operands[i]) == SPV_ID_TYPE_FUNC_DEFINE)
            {
                resultTypeId = SPV_ID_FUNC_TYPE_ID(spvOperand);
            }

            operand = virInst->src[virOpndId];

            if (isWorkGroup)
            {
                VIR_Operand_SetSym(operand, VIR_Shader_GetSymFromId(virShader, SPV_WORKGROUP_INFO()->sharedSboSymId));
            }
            else
            {
                VIR_Operand_SetSym(operand, SPV_ID_VIR_SYM(spvOperand));
            }
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetTypeId(operand, VIR_Symbol_GetTypeId(VIR_Operand_GetSymbol(operand)));
            VIR_Operand_SetSwizzle(operand, virSwizzle);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);

            if (i == 0)
            {
                __SetAccessChainOffsetToOperand(spv, spvOperand, operand, SpvOffsetType_None);

                operand = virInst->src[++virOpndId];
                if (isWorkGroup)
                {
                    VIR_Operand_SetSym(operand, VIR_Shader_GetSymFromId(virShader, SPV_WORKGROUP_INFO()->curOffsetSymId));
                    VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_X);
                    VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
                    VIR_Operand_SetTypeId(operand, SPV_ID_VIR_TYPE_ID(resultTypeId));
                }
                else
                {
                    if (baseOffsetType == VIR_SYM_VARIABLE)
                    {
                        VIR_Operand_SetSym(operand, VIR_Shader_GetSymFromId(virShader, baseOffset));
                        VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_X);
                        VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
                        VIR_Operand_SetTypeId(operand, SPV_ID_VIR_TYPE_ID(resultTypeId));
                    }
                    else
                    {
                        VIR_ScalarConstVal constVal;
                        constVal.uValue = baseOffset;
                        VIR_Operand_SetImmediate(operand, VIR_TYPE_UINT32, constVal);
                        VIR_Operand_SetTypeId(operand, VIR_TYPE_UINT32);
                        VIR_Operand_SetSwizzle(operand, virSwizzle);
                    }
                }
                VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
                VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
                VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            }
            else
            {
                __SetAccessChainOffsetToOperand(spv, spvOperand, operand, SpvOffsetType_Normal);
            }
        }
        else if (SPV_ID_TYPE(spv->operands[i]) == SPV_ID_TYPE_CONST)
        {
            gcmASSERT(virOpndId < VIR_Inst_GetSrcNum(virInst));
            operand = virInst->src[virOpndId];
            VIR_Operand_SetSwizzle(operand, virSwizzle);
            VIR_Operand_SetConstId(operand, SPV_ID_VIR_CONST_ID(spv->operands[i]));
            VIR_Operand_SetOpKind(operand, VIR_OPND_CONST);
            VIR_Operand_SetTypeId(operand, SPV_ID_VIR_TYPE_ID(spv->operands[i]));
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);

            __SetAccessChainOffsetToOperand(spv, spv->operands[i], operand, SpvOffsetType_Normal);
        }
        else
        {
            gcmASSERT(0);
        }
        virOpndId++;
    }

    return virErrCode;
}

static VSC_ErrCode __SpvEmitInstructions(gcSPV spv, VIR_Shader * virShader)
{
    gctUINT i;
    VIR_OpCode virOpcode;
    SpvId resultId = SPV_INVALID_ID;
    SpvId resultTypeId = SPV_INVALID_ID;
    SpvOp opCode = (SpvOp)spv->opCode;
    VIR_NameId nameId;
    VIR_Symbol * dstVirSym = gcvNULL;
    VIR_Type *  dstVirType = gcvNULL;
    VIR_TypeId dstVirTypeId = VIR_TYPE_UNKNOWN;
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VIR_SymId virSymId;
    VIR_Operand * operand;
    VIR_Instruction * virInst;
    VIR_Swizzle virSwizzle;
    VIR_Enable virEnableMask;
    gctBOOL hasDest;
    gctUINT virOpndId = 0;

    /* map spvopcode to vir opcode, if not support, add more inst */
    virOpcode = SPV_OPCODE_2_VIR_OPCODE(spv->opCode);

    if ((InstructionDesc[opCode].opClass == OpClassRelationalLogical) &&
        (InstructionDesc[opCode].virOpCode == VIR_OP_NOP))
    {
        virOpcode = VIR_OP_COMPARE;
    }

    hasDest = VIR_OPCODE_hasDest(virOpcode);
    if (spv->resultId)
    {
        resultId = spv->resultId;
        resultTypeId = spv->resultTypeId;

        dstVirTypeId = SPV_ID_TYPE_VIR_TYPE_ID(resultTypeId);
        dstVirType = SPV_ID_TYPE_VIR_TYPE(resultTypeId);
    }
    else
    {
        gcmASSERT(!hasDest);

        dstVirTypeId = VIR_TYPE_UNKNOWN;
        dstVirType = gcvNULL;
    }

    /* If this instruction has dest, generate symbol if needed. */
    if (hasDest)
    {
        nameId = SPV_ID_VIR_NAME_ID(resultId);
        /* Check if we need to allocate a new symbol for this dest. */
        if (nameId == VIR_INVALID_ID)
        {
            __SpvAddIdSymbol(spv, virShader, gcvNULL, resultId, resultTypeId, VIR_SYM_VARIABLE, VIR_STORAGE_GLOBAL, gcvFALSE);
        }

        virSymId = SPV_ID_VIR_SYM_ID(resultId);
        if (VIR_Id_isFunctionScope(virSymId))
        {
            dstVirSym = VIR_Function_GetSymFromId(spv->virFunction, virSymId);
        }
        else
        {
            dstVirSym = VIR_Shader_GetSymFromId(virShader, virSymId);
        }
    }

    if (virOpcode == VIR_OP_BARRIER)
    {
        VIR_Shader_SetFlag(virShader, VIR_SHFLAG_HAS_BARRIER);
    }

    virEnableMask = __SpvGenEnable(spv, dstVirType, resultTypeId);

    VIR_Function_AddInstruction(spv->virFunction,
        virOpcode,
        dstVirTypeId,
        &virInst);

    VIR_Inst_SetConditionOp(virInst, __SpvOpCode2VIRCop(opCode));

    if (hasDest)
    {
        operand = VIR_Inst_GetDest(virInst);
        VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
        VIR_Operand_SetEnable(operand, virEnableMask);
        VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
        VIR_Operand_SetTypeId(operand, dstVirTypeId);
        VIR_Operand_SetSym(operand, dstVirSym);
        VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
        __SetAccessChainOffsetToOperand(spv, spv->resultId, operand, SpvOffsetType_Normal);
    }

    virOpndId = 0;
    for (i = 0; i < spv->operandSize; i++)
    {
        virSwizzle = __SpvID2Swizzle(spv, spv->operands[i]);

        if (SPV_ID_TYPE(spv->operands[i]) == SPV_ID_TYPE_SYMBOL ||
            SPV_ID_TYPE(spv->operands[i]) == SPV_ID_TYPE_FUNC_DEFINE)
        {
            SpvId spvOperand = spv->operands[i];
            SpvId resultTypeId = spv->operands[i];

            if (SPV_ID_TYPE(spv->operands[i]) == SPV_ID_TYPE_FUNC_DEFINE)
            {
                resultTypeId = SPV_ID_FUNC_TYPE_ID(spvOperand);
            }

            operand = virInst->src[virOpndId];

            VIR_Operand_SetSym(operand, SPV_ID_VIR_SYM(spvOperand));
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetTypeId(operand, SPV_ID_VIR_TYPE_ID(resultTypeId));
            VIR_Operand_SetSwizzle(operand, virSwizzle);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);

            __SetAccessChainOffsetToOperand(spv, spvOperand, operand, SpvOffsetType_Normal);
        }
        else if (SPV_ID_TYPE(spv->operands[i]) == SPV_ID_TYPE_CONST)
        {
            gcmASSERT(virOpndId < VIR_Inst_GetSrcNum(virInst));
            operand = virInst->src[virOpndId];
            VIR_Operand_SetSwizzle(operand, virSwizzle);
            VIR_Operand_SetConstId(operand, SPV_ID_VIR_CONST_ID(spv->operands[i]));
            VIR_Operand_SetOpKind(operand, VIR_OPND_CONST);
            VIR_Operand_SetTypeId(operand, SPV_ID_VIR_TYPE_ID(spv->operands[i]));
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);

            __SetAccessChainOffsetToOperand(spv, spv->operands[i], operand, SpvOffsetType_Normal);
        }
        else
        {
            gcmASSERT(0);
        }
        virOpndId++;
    }

    if(virOpcode == VIR_OP_COMPARE)
    {
        VIR_Instruction* selectInst;

        VIR_Function_AddInstruction(spv->virFunction,
                                    VIR_OP_CSELECT,
                                    dstVirTypeId,
                                    &selectInst);
        VIR_Inst_SetConditionOp(selectInst, VIR_COP_SELMSB);
        VIR_Operand_Copy(VIR_Inst_GetDest(selectInst), VIR_Inst_GetDest(virInst));
        VIR_Operand_Copy(VIR_Inst_GetSource(selectInst, 0), VIR_Inst_GetDest(virInst));
        VIR_Operand_Change2Src_WShift(VIR_Inst_GetSource(selectInst, 0));
        VIR_Operand_SetImmediateUint(VIR_Inst_GetSource(selectInst, 1), 1);
        VIR_Operand_SetImmediateUint(VIR_Inst_GetSource(selectInst, 2), 0);
    }

    return virErrCode;
}

static VIR_Symbol * __SpvGetBlockSymbol(gcSPV spv, VIR_Shader * virShader, SpvId id)
{
    VIR_Symbol         *sym = SPV_ID_VIR_SYM(id);

    if (VIR_Symbol_isUBO(sym) || VIR_Symbol_isSBO(sym))
    {
        return sym;
    }

    return gcvNULL;
}

static VSC_ErrCode __SpvEmitCopyMemory(gcSPV spv, VIR_Shader * virShader)
{
    VSC_ErrCode         virErrCode = VSC_ERR_NONE;
    SpvOp               spvOpCode = spv->opCode;
    SpvId               targetId = spv->operands[0];
    SpvId               targetTypeId = SPV_ID_SYM_SPV_TYPE(targetId);
    VIR_TypeId          virDstTypeId = SPV_ID_VIR_TYPE_ID(targetId);
    gctBOOL             targetBlockArray = gcvFALSE;
    SpvId               srcId = spv->operands[1];
    gctBOOL             srcBlockArray = gcvFALSE;
    gctBOOL             sized = (spvOpCode == SpvOpCopyMemorySized);
    SpvMemoryAccessMask memoryAccessMask = SpvMemoryAccessMaskNone;
    VIR_Symbol         *dstBlockSym = gcvNULL;
    VIR_Symbol         *srcBlockSym = gcvNULL;
    VIR_SymId           dstSymId = VIR_INVALID_ID;
    VIR_Symbol         *dstSym = gcvNULL;
    VIR_NameId          nameId;
    VIR_Instruction    *virInst = gcvNULL;
    VIR_Operand        *operand = gcvNULL;
    VIR_Enable          virDstEnable = __SpvGenEnable(spv, VIR_Shader_GetTypeFromId(virShader, virDstTypeId), targetTypeId);
    VIR_Swizzle         virSrcSwizzle = __SpvID2Swizzle(spv, srcId);

    /* Get memory access mask. */
    if (sized && spv->operandSize == 4)
    {
        memoryAccessMask = (SpvMemoryAccessMask)spv->operands[3];
    }
    else if (!sized && spv->operandSize == 3)
    {
        memoryAccessMask = (SpvMemoryAccessMask)spv->operands[2];
    }

    /* this should be used later, just set it as used here */
    (void)memoryAccessMask;

    /* Check target. */
    dstBlockSym = __SpvGetBlockSymbol(spv, virShader, targetId);

    /* Check source. */
    srcBlockSym = __SpvGetBlockSymbol(spv, virShader, srcId);

    if (dstBlockSym)
    {
        /* Add a temp variable to hold the data. */
        __SpvGenerateCopyMemoryName(spv, targetId);
        dstSymId = __SpvAddIdSymbol(spv,
                                    virShader,
                                    spv->virName,
                                    targetId,
                                    targetTypeId,
                                    VIR_SYM_VARIABLE,
                                    VIR_STORAGE_GLOBAL,
                                    gcvFALSE);
    }
    else
    {
        nameId = SPV_ID_VIR_NAME_ID(targetId);
        /* Check if we need to allocate a new symbol for this dest. */
        if (nameId == VIR_INVALID_ID)
        {
            __SpvAddIdSymbol(spv, virShader, gcvNULL, targetId, targetTypeId, VIR_SYM_VARIABLE, VIR_STORAGE_GLOBAL, gcvFALSE);
        }
        dstSymId = SPV_ID_VIR_SYM_ID(targetId);
    }
    dstSym = VIR_Shader_GetSymFromId(virShader, dstSymId);

    /* Source is a block member
    ** 1) If target is not a block member, just use LOAD.
    ** 2) If target is a block member, then we need to LOAD the data into a temp variable, then use STORE to save the data.
    */
    if (srcBlockSym)
    {
        VIR_SymbolKind  baseOffsetType = SPV_ID_SYM_OFFSET_TYPE(srcId);
        gctUINT         baseOffset = SPV_ID_SYM_OFFSET_VALUE(srcId);

        /* Load the data. */
        if (sized)
        {
            gcmASSERT(gcvFALSE);
        }
        else
        {
            VIR_Function_AddInstruction(spv->virFunction,
                                        VIR_OP_LOAD,
                                        virDstTypeId,
                                        &virInst);
            /* Set DEST. */
            operand = VIR_Inst_GetDest(virInst);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            VIR_Operand_SetEnable(operand, virDstEnable);
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetTypeId(operand, virDstTypeId);
            VIR_Operand_SetSym(operand, dstSym);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            if (!dstBlockSym)
            {
                __SetAccessChainOffsetToOperand(spv, targetId, operand, SpvOffsetType_Normal);
            }

            /* Set src block addr. */
            operand = VIR_Inst_GetSource(virInst, 0);
            VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XYZW);
            VIR_Operand_SetSym(operand, srcBlockSym);
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetTypeId(operand, VIR_Type_GetIndex(VIR_Symbol_GetType(srcBlockSym)));
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            __SetAccessChainOffsetToOperand(
                spv,
                srcId,
                operand,
                srcBlockArray ? SpvOffsetType_UBO_Array : SpvOffsetType_None);

            /* Set src offset. */
            operand = VIR_Inst_GetSource(virInst, 1);
            if (baseOffsetType == VIR_SYM_VARIABLE)
            {
                VIR_Operand_SetSym(operand, VIR_Shader_GetSymFromId(virShader, baseOffset));
                VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            }
            else
            {
                VIR_ScalarConstVal constVal;
                constVal.uValue = baseOffset;
                VIR_Operand_SetImmediate(operand, VIR_TYPE_UINT32, constVal);
            }

            VIR_Operand_SetTypeId(operand, VIR_TYPE_UINT32);
            VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XXXX);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            __SetAccessChainOffsetToOperand(spv, srcId, operand, SpvOffsetType_None);
        }

        /* Write the data if need. */
        if (dstBlockSym)
        {
            baseOffsetType = SPV_ID_SYM_OFFSET_TYPE(targetId);
            baseOffset = SPV_ID_SYM_OFFSET_VALUE(targetId);

            VIR_Function_AddInstruction(spv->virFunction,
                                        VIR_OP_STORE,
                                        virDstTypeId,
                                        &virInst);
            /* Set DEST. */
            operand = VIR_Inst_GetDest(virInst);
            VIR_Operand_SetSym(operand, dstSym);
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            VIR_Operand_SetEnable(operand, virDstEnable);
            VIR_Operand_SetTypeId(operand, virDstTypeId);

            /* Set src block addr. */
            operand = VIR_Inst_GetSource(virInst, 0);
            VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XYZW);
            VIR_Operand_SetSym(operand, dstBlockSym);
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetTypeId(operand, VIR_Type_GetIndex(VIR_Symbol_GetType(dstBlockSym)));
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            __SetAccessChainOffsetToOperand(
                spv,
                targetId,
                operand,
                targetBlockArray ? SpvOffsetType_UBO_Array : SpvOffsetType_None);

            /* Set src offset. */
            operand = VIR_Inst_GetSource(virInst, 1);
            if (baseOffsetType == VIR_SYM_VARIABLE)
            {
                VIR_Operand_SetSym(operand, VIR_Shader_GetSymFromId(virShader, baseOffset));
                VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            }
            else
            {
                VIR_ScalarConstVal constVal;
                constVal.uValue = baseOffset;
                VIR_Operand_SetImmediate(operand, VIR_TYPE_UINT32, constVal);
            }

            VIR_Operand_SetTypeId(operand, VIR_TYPE_UINT32);
            VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XXXX);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            __SetAccessChainOffsetToOperand(spv, targetId, operand, SpvOffsetType_None);

            /* Set data. */
            operand = VIR_Inst_GetSource(virInst, 2);
            VIR_Operand_SetSym(operand, dstSym);
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            VIR_Operand_SetSwizzle(operand, virSrcSwizzle);
            VIR_Operand_SetTypeId(operand, virDstTypeId);
        }
    }
    /* Source is not a block member
    ** 1) If target is not a block member, just use MOV.
    ** 2) If target is a block member, just use STORE.
    */
    else
    {
        if (dstBlockSym)
        {
            VIR_SymbolKind  baseOffsetType = SPV_ID_SYM_OFFSET_TYPE(srcId);
            gctUINT         baseOffset = SPV_ID_SYM_OFFSET_VALUE(srcId);

            baseOffsetType = SPV_ID_SYM_OFFSET_TYPE(targetId);
            baseOffset = SPV_ID_SYM_OFFSET_VALUE(targetId);
            VIR_Function_AddInstruction(spv->virFunction,
                                        VIR_OP_STORE,
                                        virDstTypeId,
                                        &virInst);
            /* Set DEST. */
            operand = VIR_Inst_GetDest(virInst);
            if (SPV_ID_TYPE(srcId) == SPV_ID_TYPE_SYMBOL)
            {
                VIR_Operand_SetSym(operand, SPV_ID_VIR_SYM(srcId));
                VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            }
            else
            {
                VIR_Operand_SetConstId(operand, SPV_ID_VIR_CONST_ID(srcId));
                VIR_Operand_SetOpKind(operand, VIR_OPND_CONST);
            }

            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            VIR_Operand_SetEnable(operand, virDstEnable);
            VIR_Operand_SetTypeId(operand, virDstTypeId);

            /* Set src block addr. */
            operand = VIR_Inst_GetSource(virInst, 0);
            VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XYZW);
            VIR_Operand_SetSym(operand, dstBlockSym);
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetTypeId(operand, VIR_Type_GetIndex(VIR_Symbol_GetType(dstBlockSym)));
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            __SetAccessChainOffsetToOperand(
                spv,
                targetId,
                operand,
                targetBlockArray ? SpvOffsetType_UBO_Array : SpvOffsetType_None);

            /* Set src offset. */
            operand = VIR_Inst_GetSource(virInst, 1);
            if (baseOffsetType == VIR_SYM_VARIABLE)
            {
                VIR_Operand_SetSym(operand, VIR_Shader_GetSymFromId(virShader, baseOffset));
                VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            }
            else
            {
                VIR_ScalarConstVal constVal;
                constVal.uValue = baseOffset;
                VIR_Operand_SetImmediate(operand, VIR_TYPE_UINT32, constVal);
            }

            VIR_Operand_SetTypeId(operand, VIR_TYPE_UINT32);
            VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XXXX);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            __SetAccessChainOffsetToOperand(spv, targetId, operand, SpvOffsetType_None);

            /* Set data. */
            operand = VIR_Inst_GetSource(virInst, 2);
            if (SPV_ID_TYPE(srcId) == SPV_ID_TYPE_SYMBOL)
            {
                VIR_Operand_SetSym(operand, SPV_ID_VIR_SYM(srcId));
                VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            }
            else
            {
                VIR_Operand_SetConstId(operand, SPV_ID_VIR_CONST_ID(srcId));
                VIR_Operand_SetOpKind(operand, VIR_OPND_CONST);
            }

            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            VIR_Operand_SetSwizzle(operand, virSrcSwizzle);
            VIR_Operand_SetTypeId(operand, SPV_ID_VIR_TYPE_ID(srcId));
            __SetAccessChainOffsetToOperand(spv, srcId, operand, SpvOffsetType_Normal);
        }
        else
        {
            VIR_Function_AddInstruction(spv->virFunction,
                                        VIR_OP_MOV,
                                        virDstTypeId,
                                        &virInst);

            /* Set dest*/
            operand = VIR_Inst_GetDest(virInst);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            VIR_Operand_SetEnable(operand, virDstEnable);
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetTypeId(operand, virDstTypeId);
            VIR_Operand_SetSym(operand, dstSym);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            __SetAccessChainOffsetToOperand(spv, targetId, operand, SpvOffsetType_Normal);

            operand = VIR_Inst_GetSource(virInst, 0);
            if (SPV_ID_TYPE(srcId) == SPV_ID_TYPE_SYMBOL)
            {
                VIR_Operand_SetSym(operand, SPV_ID_VIR_SYM(srcId));
                VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            }
            else
            {
                VIR_Operand_SetConstId(operand, SPV_ID_VIR_CONST_ID(srcId));
                VIR_Operand_SetOpKind(operand, VIR_OPND_CONST);
            }

            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
            VIR_Operand_SetSwizzle(operand, virSrcSwizzle);
            VIR_Operand_SetTypeId(operand, SPV_ID_VIR_TYPE_ID(srcId));
            __SetAccessChainOffsetToOperand(spv, srcId, operand, SpvOffsetType_Normal);
        }
    }

    return virErrCode;
}

static VSC_ErrCode __SpvEmitSwitch(gcSPV spv, VIR_Shader * virShader)
{
    VSC_ErrCode         virErrCode = VSC_ERR_NONE;
    VIR_Instruction    *virInst = gcvNULL;
    VIR_TypeId          virType = VIR_TYPE_INT32;
    VIR_LabelId         virLabelId;
    VIR_Label          *virLabel;
    VIR_Link           *virLink;
    VIR_Operand        *operand;
    VIR_Symbol         *sym = gcvNULL;
    VIR_ScalarConstVal  constVal;
    gctUINT             selectorId, defaultId, labelId, literalValue;
    gctUINT             i;
    gctUINT             labelCount = (spv->operandSize - 2) / 2;

    /* Get selector ID and default ID. */
    selectorId = spv->operands[0];
    defaultId = spv->operands[1];

    /* Generate JMPs to label. */
    for (i = 0; i < labelCount; i++)
    {
        /* Generate a JMP. */
        virErrCode =  VIR_Function_AddInstruction(spv->virFunction,
                                                  VIR_OP_JMPC,
                                                  virType,
                                                  &virInst);
        VIR_Inst_SetConditionOp(virInst, VIR_COP_EQUAL);

        /* Set DEST. */
        operand = VIR_Inst_GetDest(virInst);
        VIR_Operand_SetModifier(operand, VIR_MOD_NONE);

        gcmASSERT(2 + 2 * i + 1 < spv->operandSize);
        labelId = spv->operands[2 + 2 * i + 1];

        virLabelId = SPV_ID_VIR_NAME_ID(labelId);
        if (virLabelId == VIR_INVALID_ID)
        {
            SPV_SET_UNHANDLE_LABEL_VIRDEST(spv, labelId, virInst, operand);
        }
        else
        {
            virLabel = VIR_Function_GetLabelFromId(spv->virFunction, virLabelId);
            VIR_Operand_SetLabel(operand, virLabel);
            VIR_Function_NewLink(spv->virFunction, &virLink);
            VIR_Link_SetReference(virLink, (gctUINTPTR_T)virInst);
            VIR_Link_AddLink(&(virLabel->referenced), virLink);
        }

        /* Set SOURCE0: selector. */
        operand = VIR_Inst_GetSource(virInst, 0);
        switch (SPV_ID_TYPE(selectorId))
        {
        case SPV_ID_TYPE_SYMBOL:
            sym = SPV_ID_VIR_SYM(selectorId);
            VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_X);
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetTypeId(operand, virType);
            VIR_Operand_SetSym(operand, sym);
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            break;

        case SPV_ID_TYPE_CONST:
            VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_X);
            VIR_Operand_SetOpKind(operand, VIR_OPND_CONST);
            VIR_Operand_SetTypeId(operand, virType);
            VIR_Operand_SetConstId(operand, SPV_ID_VIR_CONST_ID(selectorId));
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            break;

        default:
            gcmASSERT(gcvFALSE);
            break;
        }

        /* Set SOURCE1: label. */
        operand = VIR_Inst_GetSource(virInst, 1);

        gcmASSERT(2 + 2 * i < spv->operandSize);
        literalValue = spv->operands[2 + 2 * i];

        constVal.uValue = literalValue;
        VIR_Operand_SetImmediate(operand, virType, constVal);
        VIR_Operand_SetOpKind(operand, VIR_OPND_IMMEDIATE);
        VIR_Operand_SetTypeId(operand, virType);
    }

    /* Generate a JMP to the default label. */
    virErrCode =  VIR_Function_AddInstruction(spv->virFunction,
                                              VIR_OP_JMP,
                                              virType,
                                              &virInst);
    VIR_Inst_SetConditionOp(virInst, VIR_COP_ALWAYS);

    /* Set DEST. */
    operand = VIR_Inst_GetDest(virInst);
    VIR_Operand_SetModifier(operand, VIR_MOD_NONE);

    virLabelId = SPV_ID_VIR_NAME_ID(defaultId);
    if (virLabelId == VIR_INVALID_ID)
    {
        SPV_SET_UNHANDLE_LABEL_VIRDEST(spv, defaultId, virInst, operand);
    }
    else
    {
        virLabel = VIR_Function_GetLabelFromId(spv->virFunction, virLabelId);
        VIR_Operand_SetLabel(operand, virLabel);
        VIR_Function_NewLink(spv->virFunction, &virLink);
        VIR_Link_SetReference(virLink, (gctUINTPTR_T)virInst);
        VIR_Link_AddLink(&(virLabel->referenced), virLink);
    }

    return virErrCode;
}

static VSC_ErrCode __SpvNop(gcSPV spv, VIR_Shader * virShader)
{
    return VSC_ERR_NONE;
}

static VSC_ErrCode __SpvAddName(gcSPV spv, VIR_Shader * virShader)
{
    gctUINT i = 0;
    gctUINT id;
    gctUINT field;
    gctCHAR * name;
    VIR_NameId nameId;
    VSC_ErrCode virErrCode = VSC_ERR_NONE;

    id = spv->operands[i++];

    if (spv->opCode == SpvOpMemberName)
    {
        field = spv->operands[i++];
        /* TO_DO, 64 bit pointer need handle for name */
        name = (gctCHAR*)*((gctUINTPTR_T*)(&(spv->operands[i++])));

        /* TO_DO, if string size is not enough */
        /* Don't cat base struct name. */
        gcoOS_StrCopySafe(spv->virName, SPV_VIR_NAME_SIZE, name);

        VIR_Shader_AddString(virShader, spv->virName, &nameId);
        SPV_SET_IDDESCRIPTOR_FIELD_NAME(spv, id, field, nameId);
    }
    else
    {
        gctUINT offset = 0;
        name = (gctCHAR*)*((gctUINTPTR_T*)(&(spv->operands[i++])));
        if (gcmIS_SUCCESS(gcoOS_StrNCmp(name, SPV_DEFAULT_PARAM_NAME, SPV_DEFAULT_PARAM_LENGTH)))
        {
            gcoOS_PrintStrSafe(spv->virName, SPV_VIR_NAME_SIZE, &offset, "#sh_%s_%d", name, spv->operands[0]);
            VIR_Shader_AddString(virShader, spv->virName, &nameId);
        }
        else
        {
            VIR_Shader_AddString(virShader, name, &nameId);
        }
        SPV_SET_IDDESCRIPTOR_NAME(spv, id, nameId);
    }

    return virErrCode;
}

gceSTATUS __SpvAllocateDecoDescriptor(
    IN gcSPV spv,
    OUT SpvCovDecorator **decoDesc
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    SpvCovDecorator *newDeco = gcvNULL;

    gcmHEADER();

    gcmVERIFY_ARGUMENT(decoDesc != gcvNULL);

    gcmONERROR(spvAllocate(spv->spvMemPool, gcmSIZEOF(SpvCovDecorator), (gctPOINTER *)&newDeco));
    gcoOS_ZeroMemory(newDeco, gcmSIZEOF(SpvCovDecorator));

    SPV_DECORATION_Initialize(&newDeco->decorationData);

    *decoDesc = newDeco;

OnError:
    gcmFOOTER();
    return status;
}

/* uncondition jump for now */
static VSC_ErrCode __SpvAddBranch(gcSPV spv, VIR_Shader * virShader)
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VIR_Instruction *   virInst = gcvNULL;
    gctUINT label = spv->operands[0];
    VIR_LabelId labelId;
    VIR_Label * virLabel;
    VIR_Link* link;
    VIR_Operand *       virDest;
    VIR_ConditionOp virCond = VIR_COP_ALWAYS;

    virErrCode =  VIR_Function_AddInstruction(spv->virFunction,
                                              SPV_OPCODE_2_VIR_OPCODE(spv->opCode),
                                              SPV_VIR_OP_FORMAT(spv->opCode),
                                              &virInst);

    virDest = VIR_Inst_GetDest(virInst);
    VIR_Operand_SetModifier(virDest, VIR_MOD_NONE);

    labelId = SPV_ID_VIR_NAME_ID(label);

    VIR_Inst_SetConditionOp(virInst, virCond);

    if (labelId == VIR_INVALID_ID)
    {
        SPV_SET_UNHANDLE_LABEL_VIRDEST(spv, label, virInst, virDest);
    }
    else
    {
        virLabel = VIR_Function_GetLabelFromId(spv->virFunction, labelId);
        VIR_Operand_SetLabel(virDest, virLabel);
        VIR_Function_NewLink(spv->virFunction, &link);
        VIR_Link_SetReference(link, (gctUINTPTR_T)virInst);
        VIR_Link_AddLink(&(virLabel->referenced), link);
    }

    return virErrCode;
}

/* condition jump for now:
** operands[0]    --> Condition(a boolean type scalar)
** operands[1]    --> True Label
** operands[2]    --> False Label
** operands[3]..  --> Branch weights
*/
static VSC_ErrCode __SpvAddBranchConditional(gcSPV spv, VIR_Shader * virShader)
{
    VSC_ErrCode         virErrCode = VSC_ERR_NONE;
    VIR_Instruction    *virInst = gcvNULL;
    gctUINT             label;
    VIR_LabelId         virLabelId;
    VIR_Label          *virLabel;
    VIR_Link           *virLink;
    VIR_Operand        *operand;
    VIR_ConditionOp     virCond = VIR_COP_NOT_EQUAL;
    VIR_Symbol         *sym = gcvNULL;
    VIR_ScalarConstVal  constVal;

    /* Add a JMPC to jump to the true label. */
    virErrCode =  VIR_Function_AddInstruction(spv->virFunction,
        SPV_OPCODE_2_VIR_OPCODE(spv->opCode),
        SPV_VIR_OP_FORMAT(spv->opCode),
        &virInst);
    VIR_Inst_SetConditionOp(virInst, virCond);

    /* Set DEST:label. */
    operand = VIR_Inst_GetDest(virInst);
    VIR_Operand_SetModifier(operand, VIR_MOD_NONE);

    label = spv->operands[1];
    virLabelId = SPV_ID_VIR_NAME_ID(label);

    if (virLabelId == VIR_INVALID_ID)
    {
        SPV_SET_UNHANDLE_LABEL_VIRDEST(spv, label, virInst, operand);
    }
    else
    {
        virLabel = VIR_Function_GetLabelFromId(spv->virFunction, virLabelId);
        VIR_Operand_SetLabel(operand, virLabel);
        VIR_Function_NewLink(spv->virFunction, &virLink);
        VIR_Link_SetReference(virLink, (gctUINTPTR_T)virInst);
        VIR_Link_AddLink(&(virLabel->referenced), virLink);
    }

    /* Set SOURCE0. */
    operand = VIR_Inst_GetSource(virInst, 0);
    switch (SPV_ID_TYPE(spv->operands[0]))
    {
    case SPV_ID_TYPE_SYMBOL:
        sym = SPV_ID_VIR_SYM(spv->operands[0]);
        VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_X);
        VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
        VIR_Operand_SetTypeId(operand, VIR_TYPE_BOOLEAN);
        VIR_Operand_SetSym(operand, sym);
        VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
        break;

    case SPV_ID_TYPE_CONST:
        VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_X);
        VIR_Operand_SetOpKind(operand, VIR_OPND_CONST);
        VIR_Operand_SetTypeId(operand, VIR_TYPE_BOOLEAN);
        VIR_Operand_SetConstId(operand, SPV_ID_VIR_CONST_ID(spv->operands[0]));
        VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    /* Set SOURCE1, actually we don't need this source, so set it to 0. */
    operand = VIR_Inst_GetSource(virInst, 1);
    constVal.uValue = 0;
    VIR_Operand_SetImmediate(operand, VIR_TYPE_BOOLEAN, constVal);
    VIR_Operand_SetOpKind(operand, VIR_OPND_IMMEDIATE);
    VIR_Operand_SetTypeId(operand, VIR_TYPE_BOOLEAN);

    /* Add a JMP to jump to the false label. */
    virErrCode =  VIR_Function_AddInstruction(spv->virFunction,
                                              VIR_OP_JMP,
                                              SPV_VIR_OP_FORMAT(spv->opCode),
                                              &virInst);

    /* Set DEST:label. */
    operand = VIR_Inst_GetDest(virInst);

    label = spv->operands[2];
    virLabelId = SPV_ID_VIR_NAME_ID(label);

    if (virLabelId == VIR_INVALID_ID)
    {
        SPV_SET_UNHANDLE_LABEL_VIRDEST(spv, label, virInst, operand);
    }
    else
    {
        virLabel = VIR_Function_GetLabelFromId(spv->virFunction, virLabelId);
        VIR_Operand_SetLabel(operand, virLabel);
        VIR_Function_NewLink(spv->virFunction, &virLink);
        VIR_Link_SetReference(virLink, (gctUINTPTR_T)virInst);
        VIR_Link_AddLink(&(virLabel->referenced), virLink);
    }

    return virErrCode;
}

static VSC_ErrCode __SpvAddReturn(gcSPV spv, VIR_Shader * virShader)
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VIR_Instruction *   virInst = gcvNULL;

    virErrCode = VIR_Function_AddInstruction(spv->virFunction,
        SPV_OPCODE_2_VIR_OPCODE(SpvOpReturn),
        SPV_VIR_OP_FORMAT(spv->opCode),
        &virInst);

    return virErrCode;
}

/* record decoration, later we will return to handle decorator when hit variable/type define*/
static VSC_ErrCode __SpvAddDecorator(gcSPV spv, VIR_Shader * virShader)
{
    gctUINT                 target = spv->operands[0];
    gctINT                  memberIndex;
    SpvDecoration           decorationType = (SpvDecoration)0;
    gctUINT                *literalNumber = gcvNULL;
    SpvCovDecorator        *dec = spv->decorationList;
    SpvConvDecorationData  *decorationData = gcvNULL;
    gctBOOL                 isMemberDecorator = gcvFALSE, copyDecoratorToStruct = gcvFALSE;
    gctUINT                 instIndex;
    gctUINT                 i, j;

    if (spv->opCode == SpvOpDecorationGroup)
    {
        /* save the decoration which precede it */
        return VSC_ERR_NONE;
    }
    else if (spv->opCode == SpvOpGroupDecorate || spv->opCode == SpvOpGroupMemberDecorate)
    {
        gctBOOL isMember = spv->opCode == SpvOpGroupMemberDecorate;
        SpvId operands[SPV_MAX_OPERAND_NUM] = { 0 };
        SpvId operandSize = spv->operandSize;

        /* save the original operans */
        gcoOS_MemCopy(operands, spv->operands, operandSize * gcmSIZEOF(SpvId));

        /* handle all the target */
        /*
        ** ??The decoration in group will overwrite the decoration in the ID, is it correct?
        */
        for (j = 0; j < SPV_CACHED_INST_COUNT(); j++)
        {
            if (SPV_CACHED_INST(j).opcode == SpvOpDecorate &&
                SPV_CACHED_INST(j).operands[0] == target)
            {
                gcoOS_MemCopy(spv->operands, SPV_CACHED_INST(j).operands, SPV_CACHED_INST(j).operandSize * gcmSIZEOF(SpvId));
                spv->operandSize = SPV_CACHED_INST(j).operandSize;
                spv->opCode = isMember ? SpvOpMemberDecorate : SpvOpDecorate;

                for (i = 1; i < operandSize; i++)
                {
                    spv->operands[0] = operands[i];
                    __SpvAddDecorator(spv, virShader);
                }
            }
        }

        return VSC_ERR_NONE;
    }
    /* Initialize decoration, set the default. */
    else if (spv->opCode == SpvOpMemberDecorate)
    {
        isMemberDecorator = gcvTRUE;
        memberIndex = (gctINT)spv->operands[1];
        decorationType = (SpvDecoration)spv->operands[2];
        literalNumber = &spv->operands[3];
    }
    else
    {
        memberIndex = -1;
        decorationType = (SpvDecoration)spv->operands[1];
        literalNumber = &spv->operands[2];
    }

    /* Find the match decoration by using target and member index.*/
    SPV_GET_DECORATOR(dec, target, memberIndex);

    /* If we don't find the existed, create a new one. */
    if (dec == gcvNULL)
    {
        if (gcmIS_ERROR(__SpvAllocateDecoDescriptor(spv, &dec)))
        {
            return VSC_ERR_OUT_OF_MEMORY;
        }
        dec->target = target;
        dec->memberIndex = memberIndex;
        dec->next = spv->decorationList;
        spv->decorationList = dec;
    }

    gcmASSERT(dec);

    decorationData = &dec->decorationData;

    /* base on nextID, we know what this decorator is, then do things */
    switch (decorationType)
    {
        case SpvDecorationRelaxedPrecision:
            decorationData->isRelaxedPrecision = gcvTRUE;
            break;

        /* Those decorations need extra operands. */
        case SpvDecorationSpecId:
            decorationData->specId = (gctINT)literalNumber[0];
            break;

        case SpvDecorationArrayStride:
            decorationData->arrayStride = (gctINT)literalNumber[0];
            break;

        case SpvDecorationMatrixStride:
            decorationData->matrixStride = (gctINT)literalNumber[0];
            break;

        case SpvDecorationBuiltIn:
            decorationData->builtIn = (gctINT)literalNumber[0];
            break;

        case SpvDecorationStream:
            decorationData->stream = (gctINT)literalNumber[0];
            gcmASSERT(decorationData->stream == 0);
            break;

        case SpvDecorationLocation:
            decorationData->location = (gctINT)literalNumber[0];
            break;

        case SpvDecorationComponent:
            decorationData->component = (gctINT)literalNumber[0];
            break;

        case SpvDecorationIndex:
            decorationData->index = (gctINT)literalNumber[0];
            break;

        case SpvDecorationBinding:
            decorationData->binding = (gctINT)literalNumber[0];
            break;

        case SpvDecorationDescriptorSet:
            decorationData->descriptorSet = (gctINT)literalNumber[0];
            break;

        case SpvDecorationOffset:
            decorationData->offset = (gctINT)literalNumber[0];
            break;

        case SpvDecorationXfbBuffer:
            decorationData->xfbBuffer = (gctINT)literalNumber[0];
            break;

        case SpvDecorationXfbStride:
            decorationData->xfbStride = (gctINT)literalNumber[0];
            break;

        case SpvDecorationFuncParamAttr:
            decorationData->funcParamAttrib = (SpvFunctionParameterAttribute)literalNumber[0];
            break;

        case SpvDecorationFPRoundingMode:
            decorationData->fpRoundingMode = (SpvFPRoundingMode)literalNumber[0];
            break;

        case SpvDecorationFPFastMathMode:
            decorationData->fpFastMathMode = (SpvFPFastMathModeMask)literalNumber[0];
            break;

        case SpvDecorationLinkageAttributes:
            decorationData->linkageAttrib.stringName = literalNumber[0];
            decorationData->linkageAttrib.type = (SpvLinkageType)literalNumber[1];
            /* we don't know how driver/compiler this works yet */
            gcmASSERT(0);
            break;

        case SpvDecorationInputAttachmentIndex:
            decorationData->inputAttachmentIndex = (gctINT)literalNumber[0];
            break;

        case SpvDecorationAlignment:
            decorationData->alignment = (gctINT)literalNumber[0];
            break;

        /* Those decorations have no extra operands. */
        case SpvDecorationBlock:
            decorationData->isBlock = gcvTRUE;
            SPV_ID_TYPE_IS_BLOCK(target) = gcvTRUE;
            break;

        case SpvDecorationBufferBlock:
            decorationData->isBufferBlock = gcvTRUE;
            SPV_ID_TYPE_IS_BLOCK(target) = gcvTRUE;
            break;

        /* Layout qualifier */
        case SpvDecorationColMajor:
            decorationData->layoutQualifier |= SPV_LAYOUT_COL_MAJOR;
            break;

        case SpvDecorationRowMajor:
            decorationData->layoutQualifier |= SPV_LAYOUT_ROW_MAJOR;
            break;

        case SpvDecorationGLSLShared:
            decorationData->layoutQualifier |= SPV_LAYOUT_GLSL_SHARED;
            break;

        case SpvDecorationGLSLPacked:
            decorationData->layoutQualifier |= SPV_LAYOUT_GLSL_PACKED;
            break;

        case SpvDecorationCPacked:
            decorationData->layoutQualifier |= SPV_LAYOUT_C_PACKED;
            break;

        /* Interpolation qualifier. */
        case SpvDecorationNoPerspective:
            decorationData->interpolationQualifier = SPV_INTERPOLATION_NOPERSPECTIVE;
            break;

        case SpvDecorationFlat:
            decorationData->interpolationQualifier = SPV_INTERPOLATION_FLAT;
            break;

        /* Auxiliary qualifier. */
        case SpvDecorationPatch:
            copyDecoratorToStruct = gcvTRUE;
            decorationData->auxiliaryQualifier = SPV_AUXILIARY_PATCH;
            break;

        case SpvDecorationCentroid:
            copyDecoratorToStruct = gcvTRUE;
            decorationData->auxiliaryQualifier = SPV_AUXILIARY_CENTROID;
            break;

        case SpvDecorationSample:
            copyDecoratorToStruct = gcvTRUE;
            decorationData->auxiliaryQualifier = SPV_AUXILIARY_SAMPLE;
            break;

        /* Variance qualifier. */
        case SpvDecorationInvariant:
            copyDecoratorToStruct = gcvTRUE;
            decorationData->varianceQualifier = SPV_VARIANCE_INVARIANT;
            break;

        case SpvDecorationRestrict:
            copyDecoratorToStruct = gcvTRUE;
            decorationData->preciseQualifier = SPV_PRECISE_ENABLE;
            break;

        case SpvDecorationUniform:
            break;

        case SpvDecorationAliased:
        case SpvDecorationVolatile:
        case SpvDecorationConstant:
        case SpvDecorationCoherent:
        case SpvDecorationSaturatedConversion:
        case SpvDecorationNonWritable:
        case SpvDecorationNonReadable:
        case SpvDecorationNoContraction:
        default:
            break;
    }

    if (isMemberDecorator && copyDecoratorToStruct)
    {
        spv->opCode = SpvOpDecorate;
        for (i = 1; i < spv->operandSize - 1; i++)
        {
            spv->operands[i] = spv->operands[i + 1];
        }
        spv->operandSize--;
        __SpvAddDecorator(spv, virShader);
    }

    /* fake a instruction, it may used by gourp decoration */
    if (spv->isCacheInst)
    {
        instIndex = SPV_CACHED_INST_COUNT();

        SPV_CHECK_CACHED_INST(spv);

        SPV_CACHED_INST(instIndex).opcode = SpvOpDecorate;

        for (i = 0; i < spv->operandSize; i++)
        {
            SPV_CACHED_INST(instIndex).operands[i] = spv->operands[i];
        }

        SPV_CACHED_INST(instIndex).operandSize = spv->operandSize;
        SPV_CACHED_INST_COUNT()++;
    }

    return VSC_ERR_NONE;
}

static void __SpvSetClientVersion(
    IN gcSPV spv,
    IN VIR_Shader *virShader
    )
{
    gctUINT shaderKindValue = 0;

    /* Create VIR_SHADER */
    switch (virShader->shaderKind)
    {
    case VIR_SHADER_VERTEX:
        shaderKindValue = 1 /*gcSHADER_TYPE_VERTEX*/;
        break;
    case VIR_SHADER_TESSELLATION_CONTROL:
        shaderKindValue = 9 /*gcSHADER_TYPE_TCS*/;
        break;
    case VIR_SHADER_TESSELLATION_EVALUATION:
        shaderKindValue = 10 /*gcSHADER_TYPE_TES*/;
        break;
    case VIR_SHADER_GEOMETRY:
        shaderKindValue = 11 /*gcSHADER_TYPE_GEOMETRY*/;
        break;
    case VIR_SHADER_FRAGMENT:
        shaderKindValue = 2 /*gcSHADER_TYPE_FRAGMENT*/;
        break;
    case VIR_SHADER_COMPUTE:
        shaderKindValue = (spv->srcLanguage == SpvSourceLanguageOpenCL_C) ? 4 /*gcSHADER_TYPE_CL*/ : 3 /*gcSHADER_TYPE_COMPUTE*/;
        break;
    default:
        gcmASSERT(0);
    }

    /*
    ** OpSource is belong to debug information, which is not required.
    ** If there is OpSource in this binary, get the srcLanguage from OpMemoryModel, which is required.
    */
    if (spv->srcLanguage == SpvSourceLanguageUnknown)
    {
        if (spv->srcMemoryMode == SpvMemoryModelGLSL450)
        {
            spv->srcLanguage = SpvSourceLanguageGLSL;
            spv->srcLanguageVersion = 450;
        }
        else if (spv->srcMemoryMode == SpvMemoryModelOpenCL)
        {
            spv->srcLanguage = SpvSourceLanguageOpenCL_C;
            /* Assume it is OCL 1.1. */
            spv->srcLanguageVersion = 110;
        }
    }

    /* base on srcLanguageVersion/srcLanguage to decide client version */
    if (spv->srcLanguage == SpvSourceLanguageESSL ||
        spv->srcLanguage == SpvSourceLanguageGLSL)
    {
        virShader->compilerVersion[0] = _SHADER_GL_LANGUAGE_TYPE | (shaderKindValue << 16);
    }
    else if (spv->srcLanguage == SpvSourceLanguageOpenCL_C)
    {
        virShader->compilerVersion[0] = _cldLanguageType | (shaderKindValue << 16);
    }
    virShader->compilerVersion[1] = VIR_Shader_DecodeLangVersionToCompilerVersion(virShader, spv->srcLanguageVersion);

    spv->setClientVersion = gcvTRUE;
}

static void __SpvSetWorkgroupSize(
    IN gcSPV spv,
    IN VIR_Shader *virShader
    )
{
    gctUINT execModeCount = spv->exeModeCount;

    if (virShader->shaderKind == VIR_SHADER_COMPUTE)
    {
        gcmASSERT(spv->exeModeDescriptor[execModeCount].exeMode == SpvExecutionModeLocalSize);
        gcmASSERT(spv->exeModeDescriptor[execModeCount].dimension == 3);

        if (spv->spvSpecFlag & SPV_SPECFLAG_SPECIFIED_LOCAL_SIZE)
        {
            virShader->shaderLayout.compute.workGroupSize[0] = spv->localSize[0];
            virShader->shaderLayout.compute.workGroupSize[1] = spv->localSize[1];
            virShader->shaderLayout.compute.workGroupSize[2] = spv->localSize[2];
        }
        else
        {
            virShader->shaderLayout.compute.workGroupSize[0] = spv->exeModeDescriptor[execModeCount].extraOp[0];
            virShader->shaderLayout.compute.workGroupSize[1] = spv->exeModeDescriptor[execModeCount].extraOp[1];
            virShader->shaderLayout.compute.workGroupSize[2] = spv->exeModeDescriptor[execModeCount].extraOp[2];
        }
    }
}

static VSC_ErrCode __SpvDecodeInstruction(gcSPV spv, VIR_Shader * virShader)
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    gctUINT i;
    gctUINT j;
    gctUINT optionalLiteralCount;
    gctUINT scopeId = 0;
    gctUINT memSematicId = 0;

    switch (spv->opCode)
    {
    case SpvOpMemoryModel:
        spv->srcAddrMode = (SpvAddressingModel)SPV_NEXT_WORD;
        spv->srcMemoryMode = (SpvMemoryModel)SPV_NEXT_WORD;
        break;

    case SpvOpExecutionMode:
        {
            gctUINT32 entryID = SPV_NEXT_WORD;

            if (entryID != spv->entryID)
            {
                if (spv->spvSpecFlag & SPV_SPECFLAG_SPECIFIED_LOCAL_SIZE)
                {
                    virShader->shaderLayout.compute.workGroupSize[0] = spv->localSize[0];
                    virShader->shaderLayout.compute.workGroupSize[1] = spv->localSize[1];
                    virShader->shaderLayout.compute.workGroupSize[2] = spv->localSize[2];
                }
                /* skip this mode */
                SPV_NEXT_INST;
            }
            else
            {
                i = spv->exeModeCount;
                SPV_CHECK_EXEMODE(spv, i);
                spv->exeModeDescriptor[i].entryID = entryID;
                spv->exeModeDescriptor[i].exeMode = (SpvExecutionMode)SPV_NEXT_WORD;

                optionalLiteralCount = spv->nextInst - spv->word;
                spv->exeModeDescriptor[i].dimension = optionalLiteralCount;

                /* There are more optional mode need to be handled */
                for (j = 0; j < optionalLiteralCount; j++)
                {
                    spv->exeModeDescriptor[i].extraOp[j] = SPV_NEXT_WORD;
                }

                switch (spv->exeModeDescriptor[i].exeMode)
                {
                case SpvExecutionModeInvocations:
                    virShader->shaderLayout.geo.geoInvocations = spv->exeModeDescriptor[i].extraOp[0];
                    break;

                case SpvExecutionModeSpacingEqual:
                    if (spv->entryExeMode == SpvExecutionModelTessellationEvaluation)
                    {
                        virShader->shaderLayout.tes.tessVertexSpacing = VIR_TESS_SPACING_EQUAL;
                    }
                    else if (spv->entryExeMode == SpvExecutionModelTessellationControl)
                    {
                        virShader->shaderLayout.tcs.tessVertexSpacing = VIR_TESS_SPACING_EQUAL;
                    }
                    break;

                case SpvExecutionModeSpacingFractionalEven:
                    if (spv->entryExeMode == SpvExecutionModelTessellationEvaluation)
                    {
                        virShader->shaderLayout.tes.tessVertexSpacing = VIR_TESS_SPACING_EVEN;
                    }
                    else if (spv->entryExeMode == SpvExecutionModelTessellationControl)
                    {
                        virShader->shaderLayout.tcs.tessVertexSpacing = VIR_TESS_SPACING_EVEN;
                    }
                    break;

                case SpvExecutionModeSpacingFractionalOdd:
                    if (spv->entryExeMode == SpvExecutionModelTessellationEvaluation)
                    {
                        virShader->shaderLayout.tes.tessVertexSpacing = VIR_TESS_SPACING_ODD;
                    }
                    else if (spv->entryExeMode == SpvExecutionModelTessellationControl)
                    {
                        virShader->shaderLayout.tcs.tessVertexSpacing = VIR_TESS_SPACING_ODD;
                    }
                    break;

                case SpvExecutionModeVertexOrderCw:
                    if (spv->entryExeMode == SpvExecutionModelTessellationEvaluation)
                    {
                        virShader->shaderLayout.tes.tessOrdering = VIR_TESS_ORDER_CW;
                    }
                    else if (spv->entryExeMode == SpvExecutionModelTessellationControl)
                    {
                        virShader->shaderLayout.tcs.tessOrdering = VIR_TESS_ORDER_CW;
                    }
                    break;

                case SpvExecutionModeVertexOrderCcw:
                    if (spv->entryExeMode == SpvExecutionModelTessellationEvaluation)
                    {
                        virShader->shaderLayout.tes.tessOrdering = VIR_TESS_ORDER_CCW;
                    }
                    else if (spv->entryExeMode == SpvExecutionModelTessellationControl)
                    {
                        virShader->shaderLayout.tcs.tessOrdering = VIR_TESS_ORDER_CCW;
                    }
                    break;

                case SpvExecutionModePixelCenterInteger:
                case SpvExecutionModeOriginUpperLeft:
                case SpvExecutionModeOriginLowerLeft:
                    break;

                case SpvExecutionModeEarlyFragmentTests:
                    virShader->useEarlyFragTest = gcvTRUE;
                    break;

                case SpvExecutionModePointMode:
                    if (spv->entryExeMode == SpvExecutionModelTessellationEvaluation)
                    {
                        virShader->shaderLayout.tes.tessPointMode = gcvTRUE;
                    }
                    else if (spv->entryExeMode == SpvExecutionModelTessellationControl)
                    {
                        virShader->shaderLayout.tcs.tessPointMode = gcvTRUE;
                    }
                    break;

                case SpvExecutionModeXfb:
                    gcmASSERT(0);
                    break;

                case SpvExecutionModeDepthReplacing:
                case SpvExecutionModeDepthGreater:
                case SpvExecutionModeDepthLess:
                case SpvExecutionModeDepthUnchanged:
                    break;

                case SpvExecutionModeLocalSize:
                    __SpvSetWorkgroupSize(spv, virShader);
                    break;

                case SpvExecutionModeLocalSizeHint:
                    break;

                case SpvExecutionModeInputPoints:
                    virShader->shaderLayout.geo.geoInPrimitive = VIR_GEO_POINTS;
                    break;

                case SpvExecutionModeInputLines:
                    virShader->shaderLayout.geo.geoInPrimitive = VIR_GEO_LINES;
                    break;

                case SpvExecutionModeInputLinesAdjacency:
                    virShader->shaderLayout.geo.geoInPrimitive = VIR_GEO_LINES_ADJACENCY;
                    break;

                case SpvExecutionModeTriangles:
                    if (spv->entryExeMode == SpvExecutionModelGeometry)
                        virShader->shaderLayout.geo.geoInPrimitive = VIR_GEO_TRIANGLES;
                    else if (spv->entryExeMode == SpvExecutionModelTessellationEvaluation)
                        virShader->shaderLayout.tes.tessPrimitiveMode = VIR_TESS_PMODE_TRIANGLE;
                    else if (spv->entryExeMode == SpvExecutionModelTessellationControl)
                        virShader->shaderLayout.tcs.tessPrimitiveMode = VIR_TESS_PMODE_TRIANGLE;
                    break;

                case SpvExecutionModeInputTrianglesAdjacency:
                    virShader->shaderLayout.geo.geoInPrimitive = VIR_GEO_TRIANGLES_ADJACENCY;
                    break;

                case SpvExecutionModeQuads:
                    if (spv->entryExeMode == SpvExecutionModelTessellationEvaluation)
                    {
                        virShader->shaderLayout.tes.tessPrimitiveMode = VIR_TESS_PMODE_QUAD;
                    }
                    else if (spv->entryExeMode == SpvExecutionModelTessellationControl)
                    {
                        virShader->shaderLayout.tcs.tessPrimitiveMode = VIR_TESS_PMODE_QUAD;
                    }
                    break;

                case SpvExecutionModeIsolines:
                    if (spv->entryExeMode == SpvExecutionModelTessellationEvaluation)
                    {
                        virShader->shaderLayout.tes.tessPrimitiveMode = VIR_TESS_PMODE_ISOLINE;
                    }
                    else if (spv->entryExeMode == SpvExecutionModelTessellationControl)
                    {
                        virShader->shaderLayout.tcs.tessPrimitiveMode = VIR_TESS_PMODE_ISOLINE;
                    }
                    break;

                case SpvExecutionModeOutputVertices:
                    if (spv->shaderStage == VSC_SHADER_STAGE_HS)
                    {
                        virShader->shaderLayout.tcs.tcsPatchOutputVertices = spv->exeModeDescriptor[i].extraOp[0];
                        virShader->shaderLayout.tcs.tcsPatchInputVertices = (gctINT)spv->tcsInputVertices;

                        /* VK tcs input is always got from driver
                           setting this flag to avoid compiler guess the inputVertices */
                        VIR_Shader_SetFlag(virShader, VIR_SHFLAG_TCS_USE_DRIVER_INPUT);
                    }
                    else if (spv->shaderStage == VSC_SHADER_STAGE_GS)
                    {
                        virShader->shaderLayout.geo.geoMaxVertices = spv->exeModeDescriptor[i].extraOp[0];;
                    }
                    break;

                case SpvExecutionModeOutputPoints:
                    virShader->shaderLayout.geo.geoOutPrimitive = VIR_GEO_POINTS;
                        break;

                case SpvExecutionModeOutputLineStrip:
                        virShader->shaderLayout.geo.geoOutPrimitive = VIR_GEO_LINE_STRIP;
                        break;

                case SpvExecutionModeOutputTriangleStrip:
                        virShader->shaderLayout.geo.geoOutPrimitive = VIR_GEO_TRIANGLE_STRIP;
                        break;

                case SpvExecutionModeVecTypeHint:
                case SpvExecutionModeContractionOff:
                    break;

                default:
                    break;
                }

                spv->exeModeCount++;
            }
        }
        break;

    case SpvOpSource:
        spv->srcLanguage = (SpvSourceLanguage)SPV_NEXT_WORD;
        spv->srcLanguageVersion = SPV_NEXT_WORD;

        optionalLiteralCount = spv->nextInst - spv->word;
        for (j = 0; j < optionalLiteralCount; j++)
        {
            /* TODO: there are more information for opSource, do we need them??? */
            SPV_NEXT_WORD;
        }

        __SpvSetClientVersion(spv, virShader);
        break;

    case SpvOpSourceExtension:
        __SpvDecodeString(spv, &spv->srcLanguageExternsion);
        SPV_NEXT_INST;
        break;

    case SpvOpExtInstImport:
        __SpvDecodeString(spv, &spv->srcExtInstImport);
        break;

    case SpvOpCapability:
        __SpvSetCapability(spv, SPV_NEXT_WORD);
        break;

    default:
        if (!spv->setClientVersion)
        {
            __SpvSetClientVersion(spv, virShader);
        }

        spv->operandSize = 0;

        for (i=0; i < InstructionDesc[spv->opCode].oprandSize && (spv->numOperands > 0); i++)
        {
            gctUINT num = spv->numOperands;
            SpvOperandClass operandClass = InstructionDesc[spv->opCode].operandClass[i];

            switch (operandClass)
            {
            case OperandId:
            case OperandLiteralNumber:
            case OperandDecoration:
            case OperandStorage:
            case OperandFunction:
            case OperandSelect:
            case OperandLoop:
            case OperandDimensionality:
            case OperandSamplerImageFormat:
            case OperandImageOperands:
                spv->operands[spv->operandSize] = SPV_NEXT_WORD_WO_OPERAND;
                spv->operandSize ++;
                ;
                break;

            case OperandVariableIds:
            case OperandVariableLiterals:
            case OperandVariableIdLiteral:
            case OperandMemoryAccess:
                for(j = 0 ; j < num; j++)
                {
                    spv->operands[spv->operandSize] = SPV_NEXT_WORD_WO_OPERAND;
                    spv->operandSize ++;
                }
                break;

            case OperandVariableLiteralId:
                while(spv->numOperands > 0)
                {
                    spv->operands[spv->operandSize] = SPV_NEXT_WORD_WO_OPERAND;
                    spv->operandSize ++;
                    spv->operands[spv->operandSize] = SPV_NEXT_WORD_WO_OPERAND;
                    spv->operandSize ++;

                    spv->numOperands -= 2;
                }
                break;

            case OperandOptionalLiteralString:
            case OperandLiteralString:
                __SpvDecodeString(spv, (gctSTRING *)&spv->operands[spv->operandSize]);
                spv->operandSize++;
                break;

            case OperandScope:
                if (scopeId < SPV_MAX_SCOPE_ID_NUM)
                {
                    spv->scope[scopeId] = SPV_NEXT_WORD_WO_OPERAND;
                    scopeId ++;
                }
                else
                {
                    gcmASSERT(0);
                }
                break;

            case OperandMemorySemantics:
                if (memSematicId < SPV_MAX_MEM_SEMANTIC_ID_NUM)
                {
                    spv->memSematic[memSematicId] = SPV_NEXT_WORD_WO_OPERAND;
                    memSematicId ++;
                }
                else
                {
                    gcmASSERT(0);
                }
                break;

            default:
                gcmASSERT(0);
                break;
            }
            --spv->numOperands;
        }

        if (InstructionDesc[spv->opCode].hasLOperand)
        {
            SPV_ID_INITIALIZED(spv->resultId) = gcvTRUE;
        }

        /* do some pre-flag check */
        __SpvCheckFlag(spv, virShader, spv->operands, spv->operandSize);

        if (InstructionDesc[spv->opCode].opFunc)
        {
            virErrCode = InstructionDesc[spv->opCode].opFunc(spv, virShader);
        }
        else
        {
            gcmASSERT(0);
        }
    }

    gcmASSERT(spv->word == spv->nextInst);

    SPV_NEXT_INST;

    return virErrCode;
}

static void __SpvParameterize(gcSPV spv)
{

}

static VSC_ErrCode __SpvAddBuiltinVariable(gcSPV spv, VIR_Shader * virShader)
{
    VIR_NameId nameId;
    VIR_Symbol * sym = gcvNULL;
    VIR_SymId symId;
    VSC_ErrCode virErrCode = VSC_ERR_NONE;

    if (spv->shaderStage == VSC_SHADER_STAGE_PS)
    {
        /* For BE, we use gl_Position to replace gl_FragCoord. */
        VIR_Shader_AddString(virShader, "gl_Position", &nameId);

        VIR_Shader_AddSymbol(
            virShader,
            VIR_SYM_VARIABLE,
            nameId,
            VIR_Shader_GetTypeFromId(virShader, VIR_TYPE_FLOAT_X4),
            VIR_STORAGE_INPUT,
            &symId);

        sym = VIR_Shader_GetSymFromId(virShader, symId);

        VIR_Symbol_SetPrecision(sym, VIR_PRECISION_HIGH);
        VIR_Symbol_SetTyQualifier(sym, VIR_TYQUAL_NONE);
        VIR_Symbol_SetLayoutQualifier(sym, VIR_LAYQUAL_NONE);
        VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_WITHOUT_REG);
        VIR_Symbol_SetLocation(sym, -1);

        spv->builtinVariable[SPV_FRAGCOORD] = symId;
    }

    return virErrCode;
}

static gceSTATUS __SpvConstructAndInitializeVIRShader(
    IN gcSPV spv,
    IN SpvExecutionModel model,
    INOUT VIR_Shader ** virShader
    )
{
    gctUINT8 shaderStage = VSC_SHADER_STAGE_UNKNOWN;
    gctBOOL  isCLKernel = gcvFALSE;
    static int theShaderId = 0;

    /* Create VIR_SHADER */
    switch (model)
    {
    case SpvExecutionModelVertex:
        shaderStage = VSC_SHADER_STAGE_VS;
        break;
    case SpvExecutionModelTessellationControl:
        shaderStage = VSC_SHADER_STAGE_HS;
        break;
    case SpvExecutionModelTessellationEvaluation:
        shaderStage = VSC_SHADER_STAGE_DS;
        break;
    case SpvExecutionModelGeometry:
        shaderStage = VSC_SHADER_STAGE_GS;
        break;
    case SpvExecutionModelFragment:
        shaderStage = VSC_SHADER_STAGE_PS;
        break;
    case SpvExecutionModelGLCompute:
        shaderStage = VSC_SHADER_STAGE_CS;
        break;
    case SpvExecutionModelKernel:
        shaderStage = VSC_SHADER_STAGE_CS;
        isCLKernel = gcvTRUE;
        break;

    default:
        return gcvSTATUS_INVALID_DATA;
    }

    if (vscCreateShader((SHADER_HANDLE*)virShader, shaderStage) != gcvSTATUS_OK)
    {
        gcmASSERT(gcvFALSE);
    }

    /* Set default layout for some shader stages. */
    if (shaderStage == VSC_SHADER_STAGE_GS)
    {
        /* The default is to run once for each input primitive. */
        (*virShader)->shaderLayout.geo.geoInvocations = 1;
    }

    /* OCL uses uvec8 for image descriptors */
    VIR_Adjust_Imagetypesize(isCLKernel);

    VIR_Shader_SetId((*virShader), theShaderId++);
    VIR_Shader_SetClientApiVersion((*virShader), gcvAPI_OPENVK);

    (*virShader)->_constVectorId = 0;
    (*virShader)->_dummyUniformCount = 0;

    /* It is a pre-high level shader. */
    VIR_Shader_SetLevel((*virShader), VIR_SHLEVEL_Pre_High);

    VIR_Shader_SetFlag((*virShader), VIR_SHFLAG_GENERATED_BY_SPIRV);

    (*virShader)->useEarlyFragTest = 0;

    {
        VIR_Function       *function;

        /* Add a internal function "#sh_viv_InitializeFunc" here to
        ** initialize all constant variables and global variables. */
        VIR_Shader_AddFunction(*virShader,
                               gcvFALSE,
                               "#sh_viv_InitializeFunc",
                               VIR_TYPE_VOID,
                               &function);
        VIR_Function_SetFlag(function, VIR_FUNCFLAG_INITIALIZE_FUNC);

        spv->virFunction = function;
        spv->initFunction = function;
    }

    spv->shaderStage = shaderStage;

    __SpvAddBuiltinVariable(spv, *virShader);

    return gcvSTATUS_OK;
}

static gceSTATUS __SpvCreateEntryPoint(
    IN gcSPV spv,
    INOUT VIR_Shader ** virShader
    )
{
    SpvEntryInfo * entryInfo = &spv->entryInfo;
    SpvExecutionModel model = SPV_NEXT_WORD;
    gctUINT32 entryId = SPV_NEXT_WORD;
    gctBOOL isEntryPoint = gcvFALSE;
    gctUINT entryPointNameLength;
    gctCHAR *entryPointName = gcvNULL;
    gctUINT i;

    /* decode the entry point name. */
    __SpvDecodeLiteralString(spv, &entryPointNameLength, entryPointName, gcvFALSE);
    gcmASSERT(entryPointNameLength);
    spvAllocate(spv->spvMemPool, entryPointNameLength, (gctPOINTER *)&entryPointName);
    gcoOS_ZeroMemory(entryPointName, entryPointNameLength);
    __SpvDecodeLiteralString(spv, gcvNULL, entryPointName, gcvTRUE);

    if (spv->entryID != 0)
    {
        /* we already have one entry point, skip it */
        isEntryPoint = gcvFALSE;
    }
    else if (entryInfo && entryInfo->entryName)
    {
        if (entryInfo->model == model &&
            gcoOS_StrCmp(entryPointName, entryInfo->entryName) == gcvSTATUS_OK)
        {
            isEntryPoint = gcvTRUE;
        }
        else
        {
            isEntryPoint = gcvFALSE;
        }
    }
    else
    {
        isEntryPoint = gcvTRUE;
    }

    if (isEntryPoint)
    {
        __SpvConstructAndInitializeVIRShader(spv, model, virShader);
        spv->entryPointNameLength = entryPointNameLength;
        spv->entryPointName = entryPointName;
        spv->entryID = entryId;
        spv->entryExeMode = model;
    }
    else
    {
        /* save this entry point, we need check if this entry function skip */
        spv->invalidEntryId[spv->invalidEntryCount] = entryId;
        spv->invalidEntryCount++;
        spv->isMultiEntry = gcvTRUE;
    }

    for (i = spv->word; i < spv->nextInst; i++)
    {
        gctUINT index = SPV_NEXT_WORD;

        /* the interface flag is default all to be 1, here we need set all to 0 when we met the index */
        gcoOS_ZeroMemory(&spv->idDescriptor[index].interfaceFlag, gcmSIZEOF(SpvInterfaceFlag));

        switch (model)
        {
        case SpvExecutionModelVertex: SPV_ID_INTERFACE_FLAG(index).inVert = 1; break;
        case SpvExecutionModelTessellationControl: SPV_ID_INTERFACE_FLAG(index).inTesc = 1; break;
        case SpvExecutionModelTessellationEvaluation: SPV_ID_INTERFACE_FLAG(index).inTese = 1; break;
        case SpvExecutionModelGeometry: SPV_ID_INTERFACE_FLAG(index).inGeom = 1; break;
        case SpvExecutionModelFragment: SPV_ID_INTERFACE_FLAG(index).inFrag = 1; break;
        case SpvExecutionModelGLCompute: SPV_ID_INTERFACE_FLAG(index).inComp = 1; break;
        case SpvExecutionModelKernel: SPV_ID_INTERFACE_FLAG(index).inKernel = 1; break;
        default: break;
        }
    }

    gcmASSERT(spv->word == spv->nextInst);

    return gcvSTATUS_OK;
}

void __SpvCreateFuncCallInfo(
    SpvMemPool * memPool,
    INOUT SpvFuncCallInfo ** FuncInfo)
{
    SpvFuncCallInfo * funcInfo = gcvNULL;

    if (FuncInfo == gcvNULL)
    {
        return;
    }

    spvAllocate(memPool, gcmSIZEOF(SpvFuncCallInfo), (gctPOINTER *)&funcInfo);

    funcInfo->funcId = SPV_INVALID_ID;
    funcInfo->isEntry = gcvFALSE;

    funcInfo->calleeAllocated = 0;
    funcInfo->calleeCount = 0;
    funcInfo->calleeIds = gcvNULL;

    funcInfo->varAllocated = 0;
    funcInfo->varCount = 0;
    funcInfo->varIds = gcvNULL;

    *FuncInfo = funcInfo;
}

gctBOOL __SpvIsFuncCallInTable(SpvFuncCallTable * funcTable, gctUINT funcId)
{
    gctBOOL inContext = gcvFALSE;
    gctUINT i;

    for (i = 0; i < funcTable->funcCount; i++)
    {
        SpvFuncCallInfo * funcCall = funcTable->funcs[i];
        if (funcCall->funcId == funcId)
        {
            inContext = gcvTRUE;
            break;
        }
    }

    return inContext;
}

SpvFuncCallInfo * __SpvGetFuncCallFromTable(SpvFuncCallTable * funcTable, gctUINT funcId)
{
    gctUINT i;

    for (i = 0; i < funcTable->funcCount; i++)
    {
        SpvFuncCallInfo * funcCall = funcTable->funcs[i];
        if (funcCall->funcId == funcId)
        {
            return funcCall;
        }
    }

    return gcvNULL;
}

gctBOOL __SpvIsFuncIdInCallInfo(SpvFuncCallInfo * funcInfo, gctUINT funcId)
{
    gctUINT i;

    for (i = 0; i < funcInfo->calleeCount; i++)
    {
        if (funcInfo->calleeIds[i] == funcId)
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

static gceSTATUS __SpvAddNewFuncCallToTable(
    IN SpvMemPool * memPool,
    IN SpvFuncCallTable * funcTable,
    IN SpvFuncCallInfo * funcCall)
{
    gceSTATUS status = gcvSTATUS_OK;

    if (funcTable == gcvNULL || memPool == gcvNULL)
    {
        return status;
    }

    SPV_CHECK_DYNAMIC_SIZE(
        memPool,
        funcTable->funcs,
        SpvFuncCallInfo*,
        funcTable->funcAllocated,
        funcTable->funcCount + 1,
        SPV_GENERAL_PAGESIZE);

    funcTable->funcs[funcTable->funcCount] = funcCall;
    funcTable->funcCount++;

    return status;
}

static gceSTATUS __SpvAddNewFuncToCallInfo(
    IN SpvMemPool * memPool,
    IN SpvFuncCallInfo * funcCall,
    IN gctUINT funcId)
{
    gceSTATUS status = gcvSTATUS_OK;

    if (memPool == gcvNULL || funcCall == gcvNULL)
    {
        return status;
    }

    SPV_CHECK_DYNAMIC_SIZE(
        memPool,
        funcCall->calleeIds,
        gctUINT,
        funcCall->calleeAllocated,
        funcCall->calleeCount + 1,
        SPV_GENERAL_PAGESIZE);

    funcCall->calleeIds[funcCall->calleeCount] = funcId;
    funcCall->calleeCount++;

    return status;
}

/*
    check if a function is in another function's call stack
*/
static gctBOOL __SpvIsFuncInCallInfoCallStack(SpvFuncCallTable * funcTable, SpvFuncCallInfo * funcInfo, gctUINT funcId)
{
    gctBOOL inCallStack = gcvFALSE;
    gctUINT i;

    /* in top level, we already checked this function, return true */
    for (i = 0; i < funcInfo->calleeCount; i++)
    {
        if (funcId == funcInfo->calleeIds[i])
        {
            return gcvTRUE;
        }
    }

    if (funcInfo->calleeCount > 0)
    {
        for (i = 0; i < funcInfo->calleeCount; i++)
        {
            gctUINT calleeId = funcInfo->calleeIds[i];
            SpvFuncCallInfo * calleeInfo = __SpvGetFuncCallFromTable(funcTable, calleeId);
            inCallStack = __SpvIsFuncInCallInfoCallStack(funcTable, calleeInfo, funcId);
            if (inCallStack)
            {
                return inCallStack;
            }
        }
    }
    else
    {
        if (funcInfo->funcId == funcId)
        {
            inCallStack = gcvTRUE;
        }
    }

    return inCallStack;
}

/* This function is used to merge function id into entry function */
static void __SpvIntegrateCalls(SpvMemPool * memPool, SpvFuncCallTable * funcTable)
{
    gctUINT i, j;

    for (i = 0; i < funcTable->funcCount; i++)
    {
        SpvFuncCallInfo * entryFuncInfo = funcTable->funcs[i];

        if (entryFuncInfo->isEntry)
        {
            for (j = 0; j < funcTable->funcCount; j++)
            {
                SpvFuncCallInfo * funcInfo = funcTable->funcs[j];
                if (!funcInfo->isEntry)
                {
                    if (!__SpvIsFuncIdInCallInfo(entryFuncInfo, funcInfo->funcId) &&
                        __SpvIsFuncInCallInfoCallStack(funcTable, entryFuncInfo, funcInfo->funcId))
                    {
                        __SpvAddNewFuncToCallInfo(memPool, entryFuncInfo, funcInfo->funcId);
                    }
                }
            }
        }
    }
}

static gceSTATUS __SpvProcessFuncCall(
    IN gcSPV spv,
    IN SpvFuncCallTable * funcTable)
{
    SpvFuncCallInfo * curFuncInfo = gcvNULL;
    spv->word = SPV_INSTRUCTION_START;

    while (spv->word < spv->size)
    {
        gctUINT firstWord = spv->src[spv->word];
        gctUINT wordCount = firstWord >> SpvWordCountShift;

        spv->opCode = (SpvOp)(firstWord & SpvOpCodeMask);
        spv->nextInst = spv->word + wordCount;

        ++spv->word;

        /* Presence of full instruction */
        if (spv->nextInst > spv->size)
        {
            return gcvSTATUS_INVALID_DATA;
        }

        /* Base for computing number of operands; will be updated as more is learned */
        spv->numOperands = wordCount - 1;

        /* Type <id> */
        spv->resultTypeId = 0;

        if (InstructionDesc[spv->opCode].typePresent)
        {
            spv->resultTypeId = SPV_NEXT_WORD;
        }

        /* Result <id> */
        spv->resultId = 0;

        if (InstructionDesc[spv->opCode].resultPresent)
        {
            spv->resultId = SPV_NEXT_WORD;
        }

        if (spv->opCode == SpvOpEntryPoint)
        {
            SpvExecutionModel model = SPV_NEXT_WORD;
            gctUINT32 entryId = SPV_NEXT_WORD;
            gctUINT entryPointNameLength;
            gctCHAR *entryPointName = gcvNULL;
            gctUINT i;

            (void)model;

            if (!__SpvIsFuncCallInTable(funcTable, entryId))
            {
                SpvFuncCallInfo * funcInfo = gcvNULL;
                __SpvCreateFuncCallInfo(spv->spvMemPool, &funcInfo);

                funcInfo->funcId = entryId;
                funcInfo->isEntry = gcvTRUE;

                __SpvAddNewFuncCallToTable(spv->spvMemPool, funcTable, funcInfo);
            }

            __SpvDecodeLiteralString(spv, &entryPointNameLength, entryPointName, gcvTRUE);

            for (i = spv->word; i < spv->nextInst; i++)
            {
                /* handle variables */
            }
        }
        else if (spv->opCode == SpvOpFunction)
        {
            gctUINT funcId = spv->resultId;

            if (__SpvIsFuncCallInTable(funcTable, spv->resultId))
            {
                curFuncInfo = __SpvGetFuncCallFromTable(funcTable, spv->resultId);
            }
            else
            {
                __SpvCreateFuncCallInfo(spv->spvMemPool, &curFuncInfo);

                curFuncInfo->funcId = funcId;

                __SpvAddNewFuncCallToTable(spv->spvMemPool, funcTable, curFuncInfo);
            }
        }
        else if (spv->opCode == SpvOpFunctionCall && curFuncInfo)
        {
            gctUINT calleeId = SPV_NEXT_WORD;
            __SpvAddNewFuncToCallInfo(spv->spvMemPool, curFuncInfo, calleeId);
        }
        else if (spv->opCode == SpvOpFunctionEnd)
        {
            curFuncInfo = gcvNULL;
        }

        SPV_NEXT_INST;
    }

    __SpvIntegrateCalls(spv->spvMemPool, funcTable);

    return gcvSTATUS_OK;
}

static void __SpvUpdateValidArea(gcSPV spv)
{
    if ((spv->isInValidArea == gcvTRUE) &&
        (spv->entryID != SPV_INVALID_ID) &&
        (spv->funcTable != gcvNULL) &&
        (spv->opCode == SpvOpFunction))
    {
        gctUINT i;
        SpvFuncCallTable * funcTable = spv->funcTable;
        SpvFuncCallInfo * entryFuncInfo = gcvNULL;

        for (i = 0; i < funcTable->funcCount; i++)
        {
            SpvFuncCallInfo * funcInfo = funcTable->funcs[i];

            if (funcInfo->funcId == spv->entryID)
            {
                entryFuncInfo = funcInfo;
                break;
            }
        }

        /* we must have entry function here */
        gcmASSERT(entryFuncInfo);

        if ((entryFuncInfo->funcId != spv->resultId) &&
            !__SpvIsFuncIdInCallInfo(entryFuncInfo, spv->resultId))
        {
            spv->isInValidArea = gcvFALSE;
        }
    }
}

static gceSTATUS __SpvProcessInstruction(
    IN gcSPV spv,
    INOUT VIR_Shader ** virShader
    )
{
    spv->word = SPV_INSTRUCTION_START;

    while (spv->word < spv->size)
    {
        VSC_ErrCode virErrCode = VSC_ERR_NONE;
        /* Instruction wordCount and opcode */
        gctUINT firstWord = spv->src[spv->word];
        gctUINT wordCount = firstWord >> SpvWordCountShift;

        spv->opCode = (SpvOp)(firstWord & SpvOpCodeMask);

        spv->nextInst = spv->word + wordCount;

        ++spv->word;

        /* Presence of full instruction */
        if (spv->nextInst > spv->size)
        {
            return gcvSTATUS_INVALID_DATA;
        }

        /* Base for computing number of operands; will be updated as more is learned */
        spv->numOperands = wordCount - 1;

        /* Type <id> */
        spv->resultTypeId = 0;

        if (InstructionDesc[spv->opCode].typePresent)
        {
            spv->resultTypeId = SPV_NEXT_WORD;
        }

        /* Result <id> */
        spv->resultId = 0;

        if (InstructionDesc[spv->opCode].resultPresent)
        {
            spv->resultId = SPV_NEXT_WORD;
        }

        __SpvUpdateValidArea(spv);

        if (spv->isInValidArea == gcvFALSE)
        {
            if (spv->opCode == SpvOpFunctionEnd)
            {
                spv->isInValidArea = gcvTRUE;
            }
            /* we are not in valid scope, skip this instruction */
            SPV_NEXT_INST;
            continue;
        }

#if gcmDUMP_SPIRV_ASIIC
        __SpvDumpLine(
            spv->resultId,
            spv->resultTypeId,
            spv->opCode,
            &spv->src[spv->word],
            spv->numOperands);
#endif

        if (spv->opCode == SpvOpEntryPoint)
        {
            if (spv->spvSpecFlag & SPV_SPECFLAG_ENTRYPOINT)
            {
                /* if we have our special entry point, let skip this instruction */
                SPV_NEXT_INST;
            }
            else
            {
                __SpvCreateEntryPoint(spv, virShader);
            }
            continue;
        }

        /* Hand off the Op and all its operands */
        virErrCode = __SpvDecodeInstruction(spv, *virShader);

        if ((spv->word != spv->nextInst) ||
            (virErrCode != VSC_ERR_NONE))
        {
            return gcvSTATUS_INVALID_DATA;
        }
    }

    return gcvSTATUS_OK;
}

static gceSTATUS __SpvValidate(
    IN gcSPV spv
    )
{
    gctUINT * stream = spv->src;
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT schema = 0;
    gctUINT i;

    gcmHEADER_ARG("spv=0x%x", spv);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(spv, gcvOBJ_SHADER);

    if (spv->size < 4)
    {
        /* Instruction is too short */
        gcmONERROR(gcvSTATUS_INVALID_DATA);
    }

    if (stream[spv->word++] != (gctUINT)SpvMagicNumber)
    {
        /* Bad magic number */
        gcmONERROR(gcvSTATUS_INVALID_DATA);
    }

    spv->version = stream[spv->word++];

    spv->generator = stream[spv->word++];

    spv->bound = stream[spv->word++];

    gcmASSERT(spv->bound > 0);

    /* Initialize internal ID. */
    SPV_CHECK_IDDESCRIPTOR(spv, spv->bound + SPV_INTERNAL_ID_NUM);
    for (i = 0; i < SPV_INTERNAL_ID_NUM; i++)
    {
        spv->internalId[i] = spv->bound + i;
    }

    /* reserved schema, must be 0 for now */
    schema = stream[spv->word++];
    if (schema != 0)
    {
        gcmONERROR(gcvSTATUS_INVALID_DATA);
    }

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS __SpvInitialize(
    IN gcSPV Spv,
    IN SpvMemPool * spvMemPool
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("spv=0x%x", Spv);

    Spv->object.type = gcvOBJ_SHADER;

    Spv->src = gcvNULL;
    Spv->size = 0;
    Spv->spvId = 0;

    Spv->spvMemPool = spvMemPool;

    Spv->version = 0;
    Spv->generator = 0;
    Spv->bound = 0;

    Spv->isInValidArea = gcvTRUE; /* default is true */

    Spv->srcLanguage = SpvSourceLanguageUnknown;
    Spv->srcLanguageVersion = 0;
    Spv->srcLanguageExternsion = gcvNULL;
    Spv->srcExtInstImport = gcvNULL;
    Spv->srcAddrMode = SpvAddressingModelLogical;
    Spv->srcMemoryMode = SpvMemoryModelSimple;
    Spv->shaderStage = VSC_SHADER_STAGE_UNKNOWN;

    Spv->exeModeDescriptor = gcvNULL;
    Spv->exeModeSize = 0;
    Spv->exeModeCount = 0;

    Spv->shaderId = 0;
    Spv->nameId = 0;
    Spv->word = 0;
    Spv->nextInst = 0;
    Spv->numOperands = 0;
    Spv->resultId = 0;
    Spv->resultTypeId = 0;
    Spv->opCode = SpvOpNop;
    Spv->virFunction = gcvNULL;
    Spv->initFunction = gcvNULL;
    Spv->func = SPV_INVALID_ID;
    Spv->argIndex = 0;

    gcoOS_MemFill(Spv->virName, 0, SPV_VIR_NAME_SIZE * gcmSIZEOF(gctCHAR));
    gcoOS_MemFill(Spv->tempName, 0, SPV_VIR_NAME_SIZE * gcmSIZEOF(gctCHAR));
    gcoOS_MemFill(Spv->operands, 0, SPV_MAX_OPERAND_NUM * gcmSIZEOF(gctUINT));

    Spv->operandSize = 0;
    Spv->unknowId = 0;

    Spv->entryID = 0;
    Spv->entryPointName = gcvNULL;
    Spv->entryPointNameLength = 0;
    Spv->isMultiEntry = gcvFALSE;

    gcoOS_MemFill(Spv->invalidEntryId, 0, SPV_MAX_INVALID_ENTRY_COUNT * gcmSIZEOF(gctUINT));
    gcoOS_MemFill(&Spv->entryInfo, 0, gcmSIZEOF(SpvEntryInfo));
    Spv->invalidEntryCount = 0;

    /* Set idDescriptor */
    Spv->idDescSize = 0;
    Spv->idDescriptor = gcvNULL;

    Spv->decorationList = gcvNULL;

    Spv->cachedInst = gcvNULL;
    Spv->cachedInstSize = 0;
    Spv->cachedInstCount = 0;
    Spv->isCacheInst = gcvTRUE; /* cache default */

    Spv->specInfo = gcvNULL;
    Spv->workgroupInfo = gcvNULL;
    Spv->hasWorkGroup = gcvFALSE;

    Spv->renderpassInfo = gcvNULL;
    Spv->subPass = ~0U;

    Spv->internalSym = gcvNULL;

    gcmFOOTER();
    return status;
}

static gceSTATUS _SpvCheckUnhandleVariables(
    IN gcSPV            spv,
    IN VIR_Shader      *virShader,
    IN gctUINT          spvId
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    VIR_SymAliasTable  *symAliasTable = VIR_Shader_GetCreatedSymAliasTable(virShader);
    gctUINT             unHandledCount;
    SpvUnhandleInfo    *infoList = gcvNULL;
    SpvUnhandleInfo     info;
    VIR_Instruction    *inst = gcvNULL;
    VIR_Operand        *dest = gcvNULL;
    VIR_Operand        *operand = gcvNULL;
    VIR_Swizzle         operandSwizzle;
    gctUINT             i;

    unHandledCount = SPV_IDDESCRIPTOR_VAR(spv, spvId).listCount;
    infoList = SPV_IDDESCRIPTOR_VAR(spv, spvId).infoList;

    for (i = 0; i < unHandledCount; i++)
    {
        info = infoList[i];
        inst = info.inst;
        dest = VIR_Inst_GetDest(inst);
        operand = info.operand;
        operandSwizzle = __SpvID2Swizzle(spv, spvId);

        if (SPV_ID_TYPE(spvId) == SPV_ID_TYPE_SYMBOL)
        {
            VIR_Symbol* sym = SPV_ID_VIR_SYM(spvId);
            VIR_Operand_SetSym(operand, sym);
            VIR_SymAliasTable_Insert(symAliasTable, sym, VIR_Operand_GetSymbol(dest));
            VIR_Operand_SetOpKind(operand, VIR_OPND_SYMBOL);
            VIR_Operand_SetTypeId(operand, VIR_Operand_GetTypeId(dest));
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetSwizzle(operand, operandSwizzle);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
        }
        else if (SPV_ID_TYPE(spvId) == SPV_ID_TYPE_CONST)
        {
            VIR_Operand_SetSwizzle(operand, operandSwizzle);
            VIR_Operand_SetConstId(operand, SPV_ID_VIR_CONST_ID(spvId));
            VIR_Operand_SetOpKind(operand, VIR_OPND_CONST);
            VIR_Operand_SetTypeId(operand, VIR_Operand_GetTypeId(dest));
            VIR_Operand_SetPrecision(operand, VIR_PRECISION_HIGH);
            VIR_Operand_SetRoundMode(operand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(operand, VIR_MOD_NONE);
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }
    }

    return status;
}

static gceSTATUS _SpvGenFuncParaAssignments(
    IN gcSPV            spv,
    IN VIR_Shader      *virShader,
    IN gctUINT          FuncId
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    VIR_IdList         *paramList;
    VIR_Symbol         *paramSym;
    gctUINT             i;

    if (SPV_ID_TYPE(FuncId) != SPV_ID_TYPE_FUNC_DEFINE)
    {
        spv->func = SPV_INVALID_ID;
        return status;
    }

    spv->func = FuncId;
    spv->virFunction = SPV_ID_FUNC_VIR_FUNCION(FuncId);
    paramList = VIR_Function_GetParameters(spv->virFunction);

    /* Update the qualifier for vir argument first. */
    for (i = 0; i < SPV_ID_FUNC_ARG_NUM(FuncId); i++)
    {
        gcmASSERT(i < VIR_IdList_Count(paramList));

        paramSym = VIR_Function_GetSymFromId(spv->virFunction, paramList->ids[i]);

        switch (SPV_ID_FUNC_ARG_STORAGE(FuncId)[i])
        {
        case SPV_PARA_IN:
            VIR_Symbol_SetStorageClass(paramSym, VIR_STORAGE_INPARM);
            break;

        case SPV_PARA_OUT:
            VIR_Symbol_SetStorageClass(paramSym, VIR_STORAGE_OUTPARM);
            break;

        case SPV_PARA_INOUT:
            VIR_Symbol_SetStorageClass(paramSym, VIR_STORAGE_INOUTPARM);
            break;

        default:
            VIR_Symbol_SetStorageClass(paramSym, VIR_STORAGE_INPARM);
            break;
        }
    }

#if !SPV_NEW_FUNCPARAM
    /* Gen parameter assignments. */
    for (i = 0; i < SPV_ID_FUNC_CALLER_NUM(FuncId); i++)
    {
        for (j = 0; j < SPV_ID_FUNC_ARG_NUM(FuncId); j++)
        {
            if (SPV_ID_FUNC_ARG_STORAGE(FuncId)[j] == SPV_PARA_IN    ||
                SPV_ID_FUNC_ARG_STORAGE(FuncId)[j] == SPV_PARA_INOUT ||
                SPV_ID_FUNC_ARG_STORAGE(FuncId)[j] == SPV_PARA_CONST)
            {
                __SpvInsertInstruction(spv, virShader, SPV_ID_FUNC_VIR_CALLER_VIR_FUNC(FuncId , i),
                    SPV_INSERT_BEFORE, SPV_ID_FUNC_VIR_CALLER_INST(FuncId , i),
                    VIR_OP_MOV,SPV_ID_FUNC_CALLER_SPV_ARG(FuncId, i)[j] , SPV_ID_FUNC_SPV_ARG(FuncId)[j]);
            }

            if (SPV_ID_FUNC_ARG_STORAGE(FuncId)[j] == SPV_PARA_OUT ||
                SPV_ID_FUNC_ARG_STORAGE(FuncId)[j] == SPV_PARA_INOUT)
            {
                __SpvInsertInstruction(spv, virShader, SPV_ID_FUNC_VIR_CALLER_VIR_FUNC(FuncId , i),
                    SPV_INSERT_AFTER, SPV_ID_FUNC_VIR_CALLER_INST(FuncId , i),
                    VIR_OP_MOV,SPV_ID_FUNC_SPV_ARG(FuncId)[j] , SPV_ID_FUNC_CALLER_SPV_ARG(FuncId, i)[j]);
            }
        }

        if (!SPV_ID_TYPE_IS_VOID(SPV_ID_FUNC_RETURN_SPV_TYPE(FuncId)))
        {
            __SpvInsertInstruction2(spv, virShader, SPV_ID_FUNC_VIR_CALLER_VIR_FUNC(FuncId, i),
                SPV_INSERT_AFTER, SPV_ID_FUNC_VIR_CALLER_INST(FuncId, i), VIR_OP_MOV,
                SPV_ID_FUNC_VIR_RET_SYM(FuncId), SPV_ID_FUNC_RETURN_SPV_TYPE(FuncId),
                SPV_ID_VIR_SYM(SPV_ID_FUNC_CALL_SPV_RET_ID(FuncId, i)), SPV_ID_SYM_SPV_TYPE(SPV_ID_FUNC_CALL_SPV_RET_ID(FuncId, i)));
        }
    }
#endif

    SPV_ID_FUNC_CALLER_NUM(FuncId) = 0;
    spv->func = SPV_INVALID_ID;
    spv->virFunction = gcvNULL;

    return status;
}

static gceSTATUS _SpvUpdatePerVertexArrayList(
    IN gcSPV            spv,
    IN VIR_Shader      *virShader,
    IN VIR_IdList      *IdList
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    VIR_Symbol         *sym;
    VIR_Type           *type;
    gctUINT             i, listLength;

    listLength = VIR_IdList_Count(IdList);
    for (i = 0; i < listLength; i++)
    {
        sym = VIR_Shader_GetSymFromId(virShader, IdList->ids[i]);

        /* Check per-vertex array. */
        if (!isSymArrayedPerVertex(sym))
        {
            continue;
        }

        /* The original type must be an array. */
        type = VIR_Symbol_GetType(sym);
        gcmASSERT(VIR_Type_isArray(type));

        /* Update the type by using the base type. */
        type = VIR_Shader_GetTypeFromId(virShader, VIR_Type_GetBaseTypeId(type));
        VIR_Symbol_SetType(sym, type);
    }

    return status;
}

static gceSTATUS _SpvUpdatePerVertexArray(
    IN gcSPV            spv,
    IN VIR_Shader      *virShader
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    VIR_ShaderKind      shaderKind = VIR_Shader_GetKind(virShader);

    if (shaderKind != VIR_SHADER_TESSELLATION_CONTROL       &&
        shaderKind != VIR_SHADER_TESSELLATION_EVALUATION    &&
        shaderKind != VIR_SHADER_GEOMETRY)
    {
        return status;
    }

    /* Check IO block. */
    gcmONERROR(_SpvUpdatePerVertexArrayList(spv, virShader, VIR_Shader_GetIOBlocks(virShader)));

    /* Check input. */
    gcmONERROR(_SpvUpdatePerVertexArrayList(spv, virShader, VIR_Shader_GetAttributes(virShader)));

    /* Check output. */
    gcmONERROR(_SpvUpdatePerVertexArrayList(spv, virShader, VIR_Shader_GetOutputs(virShader)));

OnError:
    return status;
}

gceSTATUS __SpvCleanUpShader(
    IN gcSPV            spv,
    IN VIR_Shader      *virShader
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gctUINT             descSize = spv->idDescSize;
    gctUINT             i;

    for (i = 0; i < descSize; i++)
    {
        /* Check all unhandle variables.*/
        gcmONERROR(_SpvCheckUnhandleVariables(spv, virShader, i));

        /* Assign all function parameters. */
        gcmONERROR(_SpvGenFuncParaAssignments(spv, virShader, i));
    }

    /* set the group size */
    __SpvCheckUnhandledWorkGroupVar(spv, virShader);

    /* Update per-vertex array symbols. */
    _SpvUpdatePerVertexArray(spv, virShader);

OnError:
    return status;
}

static SpvExecutionModel __SpvVkStageBitToSpvModel(VkShaderStageFlagBits stage)
{
    SpvExecutionModel model = -1;

    switch (stage)
    {
    case VK_SHADER_STAGE_VERTEX_BIT:                    model = SpvExecutionModelVertex; break;
    case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:      model = SpvExecutionModelTessellationControl; break;
    case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:   model = SpvExecutionModelTessellationEvaluation; break;
    case VK_SHADER_STAGE_GEOMETRY_BIT:                  model = SpvExecutionModelGeometry; break;
    case VK_SHADER_STAGE_FRAGMENT_BIT:                  model = SpvExecutionModelFragment; break;
    case VK_SHADER_STAGE_COMPUTE_BIT:                   model = SpvExecutionModelGLCompute; break;
    case VK_SHADER_STAGE_ALL_GRAPHICS:
    case VK_SHADER_STAGE_ALL:
    default: break;
    }

     return model;
}

void * gcSPV_CreateSPV(
    SpvMemPool * spvMemPool,
    SpvDecodeInfo * info)
{
    gceSTATUS status = gcvSTATUS_OK;
    gcSPV spv = gcvNULL;
    VkPipelineShaderStageCreateInfo * vkStageInfo = (VkPipelineShaderStageCreateInfo*)info->stageInfo;
    SpvRenderPassInfo *renderpassInfo = info->renderpassInfo;

    gcmHEADER();

    gcmONERROR(spvAllocate(spvMemPool, sizeof(struct _gcSPV), (gctPOINTER *)&spv));

    gcoOS_MemFill(spv, 0 , sizeof(struct _gcSPV));

    gcmONERROR(__SpvInitialize(spv, spvMemPool));

    spv->src = info->binary;
    spv->size = info->sizeInByte / 4;

    if (vkStageInfo)
    {
        spv->specInfo = (VkSpecializationInfo *)vkStageInfo->pSpecializationInfo;
        spv->entryInfo.entryName = (gctCHAR *)vkStageInfo->pName;
        spv->entryInfo.model = __SpvVkStageBitToSpvModel(vkStageInfo->stage);
    }

    if (renderpassInfo)
    {
        spv->renderpassInfo = renderpassInfo;
        spv->subPass = info->subPass;
    }

    /*TO_DO, this id must be unique for this process,
       we use this id to create unique name which may need to do link, like functin return symbol*/
    spv->spvId = 0;
    spv->spvSpecFlag = info->specFlag;

    spv->localSize[0] =info->localSize[0];
    spv->localSize[1] =info->localSize[1];
    spv->localSize[2] =info->localSize[2];
    spv->tcsInputVertices = info->tcsInputVertices;

    spv->funcTable = info->funcCtx;

    gcmFOOTER();
    return (spv);

OnError:
    gcmFOOTER();
    return gcvNULL;
}

gceSTATUS
gcSPV_Decode(
    IN SpvDecodeInfo * info,
    IN OUT SHADER_HANDLE* hVirShader
    )
{
    gceSTATUS   status = gcvSTATUS_OK;
    gcSPV Spv = gcvNULL;
    VIR_Shader** VirShader = (VIR_Shader**)hVirShader;
    SpvMemPool *spvMemPool = gcvNULL;

    gcmHEADER();

    /* Initialize current memory pool */
    spvInitializeMemPool(SPV_MEMPOOL_PAGESIZE, &spvMemPool);

    Spv = (gcSPV)gcSPV_CreateSPV(spvMemPool, info);

    if (Spv->src == gcvNULL)
    {
        gcmFOOTER();
        return gcvSTATUS_INVALID_DATA;
    }

    if (gcmGetOptimizerOption()->dumpShaderSource)
    {
        __SpvDumpSpriv(info->binary, info->sizeInByte);
    }

    /* handle special flag */
    if (Spv->spvSpecFlag & SPV_SPECFLAG_ENTRYPOINT)
    {
        /* if we have special entry point, we need create vir shader first and skip the
           original SpvOpEntryPoint's shader construction */
        __SpvConstructAndInitializeVIRShader(Spv, Spv->entryInfo.model, VirShader);
        Spv->entryPointName = Spv->entryInfo.entryName;
        Spv->entryExeMode = Spv->entryInfo.model;
    }

    __SpvParameterize(Spv);

    /* check first several byte for magic number/version/genereator/boundID/schema*/
    if (gcmIS_ERROR(__SpvValidate(Spv)))
    {
        gcmFOOTER();
        return gcvSTATUS_INVALID_DATA;
    }

#if gcmDUMP_SPIRV_ASIIC
    __SpvDumpValidator(Spv->src, Spv->size * 4);
#endif

    /*  Porcess Instructions */
    __SpvProcessInstruction(Spv, VirShader);

    /* Do some clean up which can only be done after we finish all instructions processing. */
    __SpvCleanUpShader(Spv, *VirShader);

#if _DEBUG_VIR_IO_COPY
    {
        VIR_Shader * copiedShader = gcvNULL;
        VIR_Shader * virShader = *VirShader;

        if (gcmIS_ERROR(vscCopyShader((SHADER_HANDLE *)&copiedShader, virShader)))
        {
            gcmFOOTER();
            return gcvSTATUS_INVALID_DATA;
        }
        vscDestroyShader(virShader);
        if (gcmGetOptimizerOption()->dumpSpirvIR)
        {
            vscPrintShader(copiedShader, gcvNULL, "Converted and Copied VIR Shader", gcvTRUE);
        }
        {
            gctCHAR *       binary     = gcvNULL;
            gctUINT         szInByte   = 0;
            VIR_Shader *    readShader = gcvNULL;

            if (gcmIS_ERROR(vscSaveShaderToBinary(copiedShader, &binary, &szInByte)))
            {
                gcmFOOTER();
                return gcvSTATUS_INVALID_DATA;
            }

            if (gcmIS_ERROR(vscLoadShaderFromBinary(binary, szInByte, (SHADER_HANDLE *)&readShader, gcvTRUE)))
            {
                gcmFOOTER();
                return gcvSTATUS_INVALID_DATA;
            }

            vscDestroyShader(copiedShader);

            *VirShader = readShader;
        }
    }
#endif

    if (gcmGetOptimizerOption()->dumpSpirvIR)
    {
        vscPrintShader(*VirShader, gcvNULL, "", gcvTRUE);
    }

    /* Uninitialize, this will destroy Spv */
    spvUninitializeMemPool(spvMemPool);

    /* Success. */
    gcmFOOTER();
    return status;
}

gceSTATUS __SpvCreateFuncCallTable(
    IN SpvMemPool * MemPool,
    INOUT SpvFuncCallTable ** FuncCallTable)
{
    gceSTATUS status = gcvSTATUS_OK;

    spvAllocate(MemPool, gcmSIZEOF(SpvFuncCallTable), (gctPOINTER *)FuncCallTable);

    (*FuncCallTable)->funcAllocated = 0;
    (*FuncCallTable)->funcCount = 0;
    (*FuncCallTable)->funcs = gcvNULL;
    (*FuncCallTable)->memPool = MemPool;

    return status;
}

gceSTATUS
gcSPV_PreDecode(
    IN SpvDecodeInfo * info,
    INOUT gctPOINTER* FuncTable
)
{
    gceSTATUS               status = gcvSTATUS_OK;
    gcSPV                   Spv = gcvNULL;
    SpvMemPool             *spvMemPool = gcvNULL;
    SpvFuncCallTable       *funcTable = gcvNULL;

    spvInitializeMemPool(SPV_MEMPOOL_PAGESIZE, &spvMemPool);

    Spv = (gcSPV)gcSPV_CreateSPV(spvMemPool, info);

    if (Spv->src == gcvNULL)
    {
        return gcvSTATUS_INVALID_DATA;
    }

    __SpvParameterize(Spv);

    /* check first several byte for magic number/version/genereator/boundID/schema*/
    if (gcmIS_ERROR(__SpvValidate(Spv)))
    {
        return gcvSTATUS_INVALID_DATA;
    }

    if (gcmIS_ERROR(__SpvCreateFuncCallTable(spvMemPool, &funcTable)))
    {
        return gcvSTATUS_INVALID_DATA;
    }

    if (gcmIS_ERROR(__SpvProcessFuncCall(Spv, funcTable)))
    {
        return gcvSTATUS_INVALID_DATA;
    }

    /* the memory is created by spirv converter, we need free it by call gcSPV_PostDecode */

    *FuncTable = funcTable;

    return status;
}

gceSTATUS
gcSPV_PostDecode(
    IN gctPOINTER FuncTable)
{
    SpvFuncCallTable * funcTable = (SpvFuncCallTable *)FuncTable;

    if (funcTable == gcvNULL)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    spvUninitializeMemPool(funcTable->memPool);

    return gcvSTATUS_OK;
}

gceSTATUS
gcSPV_Conv2VIR(
    IN gcSPV Spv,
    IN SHADER_HANDLE hVirShader
    )
{
    gceSTATUS   status = gcvSTATUS_OK;
    gcmHEADER();

    gcmFOOTER();
    return status;
}
