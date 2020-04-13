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


#ifndef __gc_spirv_to_vir_h_
#define __gc_spirv_to_vir_h_

#define SPV_NEW_FUNCPARAM       1

#define _GC_OBJ_ZONE    gcdZONE_COMPILER

#define SPV_HAS_RESULT (spv->resultId != 0)

#define SPV_NEXT_WORD (spv->numOperands--, spv->src[spv->word++])

#define SPV_NEXT_WORD_NO_OPERAND (spv->word++)

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

/* Macros for the type. */
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
#define SPV_ID_TYPE_STRUCT_MEMBER(id, mem) (spv->idDescriptor[id].u.type.u.st.fields[mem].member])
#define SPV_ID_TYPE_IMAGE_SAMPLED_TYPE(id) (spv->idDescriptor[id].u.type.u.image.sampledType)
#define SPV_ID_TYPE_IMAGE_DIM(id) (spv->idDescriptor[id].u.type.u.image.dimension)
#define SPV_ID_TYPE_IMAGE_DEPTH(id) (spv->idDescriptor[id].u.type.u.image.depth)
#define SPV_ID_TYPE_IMAGE_ARRAY(id) (spv->idDescriptor[id].u.type.u.image.arrayed)
#define SPV_ID_TYPE_IMAGE_MSAA(id) (spv->idDescriptor[id].u.type.u.image.msaa)
#define SPV_ID_TYPE_IMAGE_SAMPLED(id) (spv->idDescriptor[id].u.type.u.image.sampled)
#define SPV_ID_TYPE_IMAGE_FORMAT(id) (spv->idDescriptor[id].u.type.u.image.format)
#define SPV_ID_TYPE_IMAGE_SAMPLER_IMAGE_TYPE(id) (spv->idDescriptor[id].u.type.u.image.sampledImageType)
#define SPV_ID_TYPE_IMAGE_ACCESS_QULIFIER(id) (spv->idDescriptor[id].u.type.u.image.qualifier)
#define SPV_ID_TYPE_SAMPLEDIMAGE_IMAGETYPEID(id) (spv->idDescriptor[id].u.type.u.sampledImage.imageTypeId)
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
#define SPV_ID_TYPE_HAS_8BIT_TYPE(id) (spv->idDescriptor[id].u.type.typeFlags.has8BitType)
#define SPV_ID_TYPE_HAS_16BIT_TYPE(id) (spv->idDescriptor[id].u.type.typeFlags.has16BitType)

/* Macros for the symbol. */
#define SPV_ID_VIR_SYM_ID(id) (spv->idDescriptor[id].u.sym.descriptorHeader.virSymId)
#define SPV_ID_SYM_SPV_TYPE(id) (spv->idDescriptor[id].u.sym.spvBaseTypeId)
#define SPV_ID_SYM_SPV_POINTER_TYPE(id) (spv->idDescriptor[id].u.sym.spvPointerTypeId)
#define SPV_ID_SYM_SRC_SPV_TYPE(id) (spv->idDescriptor[id].u.sym.srcSpvTypeId)
#define SPV_ID_SYM_HAS_ACCESS_CHAIN(id) (spv->idDescriptor[id].u.sym.offsetInfo.virAcOffsetInfo.bHasAccessChain)
#define SPV_ID_SYM_BLOCK_OFFSET_TYPE(id) (spv->idDescriptor[id].u.sym.offsetInfo.virAcOffsetInfo.blockIndexType)
#define SPV_ID_SYM_BLOCK_OFFSET_VALUE(id) (spv->idDescriptor[id].u.sym.offsetInfo.virAcOffsetInfo.blockIndex)
#define SPV_ID_SYM_OFFSET_TYPE(id) (spv->idDescriptor[id].u.sym.offsetInfo.virAcOffsetInfo.baseOffsetType)
#define SPV_ID_SYM_OFFSET_VALUE(id) (spv->idDescriptor[id].u.sym.offsetInfo.virAcOffsetInfo.baseOffset)
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
#define SPV_ID_SYM_BASE_SPV_SYMBOL_ID(id) (spv->idDescriptor[id].u.sym.baseSpvSymId)
#define SPV_ID_SYM_PARAM_TO_FUNC(id) (spv->idDescriptor[id].u.sym.paramToFunc)
#define SPV_ID_SYM_PER_PATCH(id) (spv->idDescriptor[id].u.sym.isPerPatch)
#define SPV_ID_SYM_PER_VERTEX(id) (spv->idDescriptor[id].u.sym.isPerVertex)
#define SPV_ID_SYM_IS_PUSH_CONST_UBO(id) (spv->idDescriptor[id].u.sym.isPushConstUBO)
#define SPV_ID_SYM_ATTACHMENT_FLAG(id) (spv->idDescriptor[id].u.sym.attachmentFlag)
#define SPV_ID_SYM_STORAGE_CLASS(id) (spv->idDescriptor[id].u.sym.storageClass)

/* Macros for the others. */
#define SPV_ID_VIR_TYPE_ID(id) (spv->idDescriptor[id].virTypeId)
#define SPV_ID_VIR_NAME_ID(id) (spv->idDescriptor[id].virNameId)
#define SPV_ID_INITIALIZED(id) (spv->idDescriptor[id].initialized)
#define SPV_ID_IS_MEM_ADDR_CALC(id) (spv->idDescriptor[id].isMemAddrCalc)
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
#define SPV_ID_FIELD_HAS_NAME(id, mem) ((gctUINT)(mem) < spv->idDescriptor[(id)].u.type.u.st.maxNumField ? \
                                       spv->idDescriptor[(id)].u.type.u.st.fields[mem].hasName : gcvFALSE)
#define SPV_ID_FIELD_VIR_NAME_ID(id, mem) ((gctUINT)(mem) < spv->idDescriptor[(id)].u.type.u.st.maxNumField ? \
                                            spv->idDescriptor[(id)].u.type.u.st.fields[mem].field : VIR_INVALID_ID)

/* Macros for the condition. */
#define SPV_ID_COND(id) (spv->idDescriptor[id].u.cond.virCond)
#define SPV_ID_COND_OP(id, index) (spv->idDescriptor[id].u.cond.op[index])

/* Macros for the function. */
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

#define SPV_ID_EXT_INST_SET(id) (spv->idDescriptor[id].u.extInstSet.virIntrinsicSetKind)

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

#define SPV_POINTER_VAR_OBJ_SPV_TYPE(ptr) (spv->idDescriptor[spv->idDescriptor[ptr].u.sym.spvBaseTypeId].u.type.u.pointer.objType)
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
                spv->idDescriptor[i].idType = SPV_ID_TYPE_UNKNOWN;\
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
    do { \
        SPV_CHECK_DYNAMIC_SIZE( spv->spvMemPool, \
                                spv->idDescriptor[id].u.type.u.st.fields, \
                                SpvStructMembers, \
                                spv->idDescriptor[id].u.type.u.st.maxNumField, \
                                (gctUINT)(fieldId) + 1, \
                                SPV_STRUCT_FIELD_NUM); \
        gcmASSERT(spv->idDescriptor[id].u.type.u.st.fields); \
        spv->idDescriptor[id].u.type.u.st.fields[fieldId].field = (sid); \
        spv->idDescriptor[id].u.type.u.st.fields[fieldId].hasName = gcvTRUE; \
    } while (gcvFALSE)

#define SPV_SET_IDDESCRIPTOR_FIELD_MEMBER(spv, id, fieldId, sid) \
    do { \
        SPV_CHECK_DYNAMIC_SIZE( spv->spvMemPool, \
                                spv->idDescriptor[id].u.type.u.st.fields, \
                                SpvStructMembers, \
                                spv->idDescriptor[id].u.type.u.st.maxNumField, \
                                (gctUINT)(fieldId) + 1, \
                                SPV_STRUCT_FIELD_NUM); \
        spv->idDescriptor[id].u.type.u.st.fields[(fieldId)].member = (sid); \
    } while (gcvFALSE)

#define SPV_SET_IDDESCRIPTOR_SYM(spv, id, sid) \
    { \
        spv->idDescriptor[id].u.sym.descriptorHeader.virSymId = sid; \
        SPV_ID_SYM_BASE_SPV_SYMBOL_ID(id) = SPV_INVALID_LABEL; \
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
        spv->idDescriptor[id].u.sym.spvBaseTypeId = tid; \
    }

#define SPV_SET_IDDESCRIPTOR_SPV_POINTER_TYPE(spv, id, tid) \
    { \
        spv->idDescriptor[id].u.sym.spvPointerTypeId = tid; \
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

#define SPV_IS_FP_CONV_OP(opcode)   ((opcode) == SpvOpFConvert)

#define SPV_IS_EMPTY_STRING(str) ((str == gcvNULL) || (gcoOS_MemCmp(str, "", 1) == gcvSTATUS_OK))

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
    VIR_SymFlagExt virSymFlagExt;

    VIR_Precision virPrecision;
    gctINT location;
    gctINT component;
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
        (SymSpv)->component = -1;                                       \
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

/* spec constant op holding. */
VSC_ErrCode __SpvFoldingSpecConstantOp(gcSPV spv, VIR_Shader * virShader);

/* Pre-process instruction. */
VSC_ErrCode __SpvPreprocessInstruction(gcSPV spv, VIR_Shader * virShader);
/* Post-process instruction. */
VSC_ErrCode __SpvPostprocessInstruction(gcSPV spv, VIR_Shader * virShader);

/* Opcode function handle. */
VSC_ErrCode __SpvEmitLabel(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitType(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitConstant(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitUndef(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitVariable(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitFunction(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitFunctionCall(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitFunctionEnd(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitAccessChain(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitPtrAccessChain(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitVertexPrimitive(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitVectorShuffle(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitPhi(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitCompositeExtract(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitCompositeInsert(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitVectorExtractDynamic(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitVectorInsertDynamic(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitCompositeConstruct(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitExtInst(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitIntrinsicCall(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitImageSample(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitSampledImage(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitSpecConstantOp(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitOpImage(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitInstructions(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitCopyMemory(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitSwitch(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitLoad(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitStore(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitArrayLength(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitAtomic(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitDecorator(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitName(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitBranch(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitBranchConditional(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitReturn(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitFunctionParameter(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitReturnValue(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitNop(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitUnsupported(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitLoopMerge(gcSPV spv, VIR_Shader * virShader);
VSC_ErrCode __SpvEmitBarrier(gcSPV spv, VIR_Shader * virShader);

#endif /* __gc_spirv_to_vir_h_ */
