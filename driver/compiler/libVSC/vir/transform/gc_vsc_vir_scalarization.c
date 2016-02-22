/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_vsc.h"
#include "vir/transform/gc_vsc_vir_scalarization.h"

/* used for creating a uni-directional operand list */
typedef struct _VSC_SCL_OPERANDLISTNODE
{
    VSC_UNI_LIST_NODE node;
    VIR_Operand* opnd;
} VSC_SCL_OperandListNode;

#define VSC_SCL_OperandListNode_GetNode(nd)         ((VSC_UNI_LIST_NODE*)&(nd)->node)
#define VSC_SCL_OperandListNode_GetOpnd(nd)         ((nd)->opnd)
#define VSC_SCL_OperandListNode_SetOpnd(nd, opnd)   ((nd)->opnd = opnd)

typedef struct _VSC_SCL_ARRAYINFO
{
    gctBOOL dynamic_indexing;
    VSC_BIT_VECTOR const_indexes;
    VSC_HASH_TABLE new_symbols;         /* map from const index to new symbol */
    VSC_UNI_LIST opnd_list;              /* operands which are constantly indexed */
    VSC_MM* mm;
} VSC_SCL_ArrayInfo;

#define VSC_SCL_ArrayInfo_GetDynamicIndexing(ai)        ((ai)->dynamic_indexing)
#define VSC_SCL_ArrayInfo_SetDynamicIndexing(ai, b)     ((ai)->dynamic_indexing = (b))
#define VSC_SCL_ArrayInfo_GetConstIndexes(ai)           ((VSC_BIT_VECTOR*)&((ai)->const_indexes))
#define VSC_SCL_ArrayInfo_GetNewSymbols(ai)             ((VSC_HASH_TABLE*)&((ai)->new_symbols))
#define VSC_SCL_ArrayInfo_GetOpndList(ai)               ((VSC_UNI_LIST*)&((ai)->opnd_list))
#define VSC_SCL_ArrayInfo_GetMM(ai)                     ((ai)->mm)
#define VSC_SCL_ArrayInfo_SetMM(ai, m)                ((ai)->mm = (m))

static void _VSC_SCL_ArrayInfo_Init(
    IN OUT VSC_SCL_ArrayInfo* array_info,
    IN VSC_MM* mm
    )
{
    VSC_SCL_ArrayInfo_SetDynamicIndexing(array_info, gcvFALSE);
    vscBV_Initialize(VSC_SCL_ArrayInfo_GetConstIndexes(array_info), mm, 64);
    vscHTBL_Initialize(VSC_SCL_ArrayInfo_GetNewSymbols(array_info), mm, vscHFUNC_Default, vscHKCMP_Default, 512);
    vscUNILST_Initialize(VSC_SCL_ArrayInfo_GetOpndList(array_info), gcvFALSE);
    VSC_SCL_ArrayInfo_SetMM(array_info, mm);
}

static void _VSC_SCL_ArrayInfo_MapNewSymbol(
    IN OUT VSC_SCL_ArrayInfo* array_info,
    IN gctINT index,
    IN VIR_Symbol* sym
    )
{
    vscHTBL_DirectSet(VSC_SCL_ArrayInfo_GetNewSymbols(array_info), (void*)(gctUINTPTR_T)index, (void*)sym);
}

static VIR_Symbol* _VSC_SCL_ArrayInfo_GetNewSymbol(
    IN OUT VSC_SCL_ArrayInfo* array_info,
    IN gctINT index
    )
{
    VIR_Symbol* new_sym = (VIR_Symbol*)vscHTBL_DirectGet(VSC_SCL_ArrayInfo_GetNewSymbols(array_info), (void*)(gctUINTPTR_T)index);
    gcmASSERT(new_sym != gcvNULL);
    return new_sym;
}

static void _VSC_SCL_ArrayInfo_AppandOperand(
    IN OUT VSC_SCL_ArrayInfo* array_info,
    VIR_Operand* opnd
    )
{
    VSC_SCL_OperandListNode* node =
        (VSC_SCL_OperandListNode*)vscMM_Alloc(VSC_SCL_ArrayInfo_GetMM(array_info), sizeof(VSC_SCL_OperandListNode));
    VSC_SCL_OperandListNode_SetOpnd(node, opnd);
    vscUNILST_Append(VSC_SCL_ArrayInfo_GetOpndList(array_info), VSC_SCL_OperandListNode_GetNode(node));
}
static void _VSC_SCL_ArrayInfo_DumpConstIndexes(
    IN VSC_BIT_VECTOR* const_indexes,
    IN VIR_Dumper* dumper
    )
{
    gctUINT32 index;
    index = vscBV_FindSetBitForward(const_indexes, 0);
    while(index != (gctUINT32)INVALID_BIT_LOC)
    {
        VIR_LOG(dumper, "%d ", index);
        index = vscBV_FindSetBitForward(const_indexes, index);
    }
}

static void _VSC_SCL_ArrayInfo_DumpNewSymbols(
    IN VSC_HASH_TABLE* new_symbols,
    IN VIR_Dumper* dumper
    )
{
}

static void _VSC_SCL_ArrayInfo_Dump(
    IN VSC_SCL_ArrayInfo* array_info,
    IN VIR_Dumper* dumper
    )
{
    VIR_LOG(dumper, "dynamic indexing: %s", VSC_SCL_ArrayInfo_GetDynamicIndexing(array_info) ? "true" : "false");
    VIR_LOG(dumper, "constantly indexed symbols:\n");
    _VSC_SCL_ArrayInfo_DumpConstIndexes(VSC_SCL_ArrayInfo_GetConstIndexes(array_info), dumper);
    VIR_LOG(dumper, "new symbols:\n");
    _VSC_SCL_ArrayInfo_DumpNewSymbols(VSC_SCL_ArrayInfo_GetNewSymbols(array_info), dumper);
}

void VSC_SCL_Scalarization_Init(
    IN OUT VSC_SCL_Scalarization* scl,
    IN VIR_Shader* shader,
    IN VSC_OPTN_SCLOptions* options,
    IN VIR_Dumper* dumper
    )
{
    vscPMP_Intialize(VSC_SCL_Scalarization_GetPmp(scl), gcvNULL, 1024,
                     sizeof(void*), gcvTRUE);
    vscHTBL_Initialize(VSC_SCL_Scalarization_GetArrayInfos(scl),
        VSC_SCL_Scalarization_GetMM(scl), vscHFUNC_Default, vscHKCMP_Default, 512);
    VSC_SCL_Scalarization_SetShader(scl, shader);
    VSC_SCL_Scalarization_SetOptions(scl, options);
    VSC_SCL_Scalarization_SetDumper(scl, dumper);
}

static VSC_SCL_ArrayInfo* _VSC_SCL_Scalarization_NewArrayInfo(
    IN VSC_SCL_Scalarization* scl
    )
{
    VSC_SCL_ArrayInfo* array_info = (VSC_SCL_ArrayInfo*)vscMM_Alloc(VSC_SCL_Scalarization_GetMM(scl), sizeof(VSC_SCL_ArrayInfo));
    _VSC_SCL_ArrayInfo_Init(array_info, VSC_SCL_Scalarization_GetMM(scl));

    return array_info;
}

static VSC_SCL_ArrayInfo* _VSC_SCL_Scalarization_GetArrayInfo(
    IN VSC_SCL_Scalarization* scl,
    IN VIR_Symbol* sym
    )
{
    VSC_SCL_ArrayInfo* array_info = gcvNULL;

    if(!vscHTBL_DirectTestAndGet(VSC_SCL_Scalarization_GetArrayInfos(scl), sym, (void**)&array_info))
    {
        array_info = _VSC_SCL_Scalarization_NewArrayInfo(scl);
        vscHTBL_DirectSet(VSC_SCL_Scalarization_GetArrayInfos(scl), sym, array_info);
    }
    return array_info;
}
void VSC_SCL_Scalarization_Final(
    IN VSC_SCL_Scalarization* scl
    )
{
    vscPMP_Finalize(&scl->pmp);
}

static VSC_ErrCode _VSC_SCL_CollectInformationFromOper(
    IN OUT VSC_SCL_Scalarization* scl,
    IN VIR_Operand* opnd
    )
{
    VSC_ErrCode errcode  = VSC_ERR_NONE;
    VIR_TypeId type_id = VIR_Operand_GetType(opnd);
    VIR_Type* type = VIR_Shader_GetTypeFromId(VSC_SCL_Scalarization_GetShader(scl), type_id);

    if(VIR_Type_GetKind(type) == VIR_TY_ARRAY)
    {
        VIR_Symbol* sym = VIR_Operand_GetSymbol(opnd);
        VSC_SCL_ArrayInfo* array_info = _VSC_SCL_Scalarization_GetArrayInfo(scl, sym);
        /* symbol has dynamic indexing, return */
        if(VSC_SCL_ArrayInfo_GetDynamicIndexing(array_info))
        {
            return errcode;
        }

        if(VIR_Operand_GetIsConstIndexing(opnd))
        {
            gctINT index = VIR_Operand_GetRelIndexing(opnd);
            vscBV_SetBit(VSC_SCL_ArrayInfo_GetConstIndexes(array_info), index);
            _VSC_SCL_ArrayInfo_AppandOperand(array_info, opnd);
        }
        else
        {
            VSC_SCL_ArrayInfo_SetDynamicIndexing(array_info, gcvTRUE);

        }
    }
    /*if(VIR_Type_GetKind(type) == VIR_TY_MATRIX)
    {
        VIR_Symbol* sym = VIR_GetFuncSymFromId(func, VIR_Operand_GetSymbolId(opnd));
        VSC_SCL_ArrayInfo* array_info = _VSC_SCL_Scalarization_GetArrayInfo(scl, sym);
        gctINT index = VIR_Operand_GetRelIndexing(opnd);
        vscBV_SetBit(VSC_SCL_ArrayInfo_GetConstIndexes(array_info), index);
        _VSC_SCL_ArrayInfo_AppandOperand(array_info, opnd);
    }*/

    return errcode;
}

static VSC_ErrCode _VSC_SCL_CollectInformationFromInst(
    IN OUT VSC_SCL_Scalarization* scl,
    IN VIR_Instruction* inst
    )
{
    VSC_ErrCode errcode  = VSC_ERR_NONE;
    VIR_Operand* dest;
    VIR_Operand* src;
    gctUINT32 i = 0;

    dest = VIR_Inst_GetDest(inst);
    _VSC_SCL_CollectInformationFromOper(scl, dest);
    src = VIR_Inst_GetSource(inst, i);
    while(src)
    {
        _VSC_SCL_CollectInformationFromOper(scl, src);
        i++;
    }

    return errcode;
}

static VSC_ErrCode _VSC_SCL_CollectInformationforFunction(
    IN OUT VSC_SCL_Scalarization* scl
    )
{
    VSC_ErrCode errcode  = VSC_ERR_NONE;
    VIR_Shader* shader = VSC_SCL_Scalarization_GetShader(scl);
    VIR_Function* func = VIR_Shader_GetCurrentFunction(shader);
    VIR_InstList* inst_list = VIR_Function_GetInstList(func);
    VIR_InstIterator iter;
    VIR_Instruction* inst;

    VIR_InstIterator_Init(&iter, inst_list);
    for(inst = VIR_InstIterator_First(&iter);
        inst != gcvNULL; inst = VIR_InstIterator_Next(&iter))
    {
        errcode = _VSC_SCL_CollectInformationFromInst(scl, inst);
    }

    return errcode;
}

static VSC_ErrCode _VSC_SCL_CollectInformationforShader(
    IN OUT VSC_SCL_Scalarization* scl
    )
{
    VSC_ErrCode errcode  = VSC_ERR_NONE;
    VIR_Shader* shader = VSC_SCL_Scalarization_GetShader(scl);
    VIR_FuncIterator func_iter;
    VIR_FunctionNode* func_node;
    VSC_OPTN_SCLOptions* options = VSC_SCL_Scalarization_GetOptions(scl);

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(shader));
    for(func_node = VIR_FuncIterator_First(&func_iter);
        func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function* func = func_node->function;

        /* dump input functions */
        if(VSC_UTILS_MASK(VSC_OPTN_SCLOptions_GetTrace(options), VSC_OPTN_SCLOptions_TRACE_INPUT_FUNCTIONS))
        {
            VIR_Dumper* dumper = VSC_SCL_Scalarization_GetDumper(scl);
            VIR_LOG(dumper, "Input function:");
            VIR_Function_Dump(dumper, func);
        }

        VIR_Shader_SetCurrentFunction(shader, func);
        _VSC_SCL_CollectInformationforFunction(scl);
    }

    /* dump collected info */
    if(VSC_UTILS_MASK(VSC_OPTN_SCLOptions_GetTrace(options), VSC_OPTN_SCLOptions_TRACE_COLLECTED_INFO))
    {
        VIR_Dumper* dumper = VSC_SCL_Scalarization_GetDumper(scl);
        VSC_HASH_ITERATOR iter;
        VIR_Symbol* sym;
        vscHTBLIterator_Init(&iter, VSC_SCL_Scalarization_GetArrayInfos(scl));
        for(sym = (VIR_Symbol*)vscHTBLIterator_First(&iter); sym != gcvNULL;
            sym = (VIR_Symbol*)vscHTBLIterator_Next(&iter))
        {
            VSC_SCL_ArrayInfo* array_info = _VSC_SCL_Scalarization_GetArrayInfo(scl, sym);
            VIR_Symbol_Dump(dumper, sym, gcvFALSE);
            _VSC_SCL_ArrayInfo_Dump(array_info, dumper);
        }
    }

    return errcode;
}


static VSC_ErrCode _VSC_SCL_GenerateNewSymbol(
    IN OUT VSC_SCL_Scalarization* scl,
    IN VIR_Symbol* sym,
    IN gctINT index,
    OUT VIR_Symbol** new_sym
    )
{
    VSC_ErrCode errcode;
    VIR_Shader* shader;
    VIR_SymbolKind kind;
    VIR_Type* type;
    VIR_TypeId base_type_id;
    VIR_Type * base_type;
    VIR_StorageClass storage;
    VIR_SymId new_sym_id;

    gcmASSERT(new_sym);

    errcode = VSC_ERR_NONE;
    shader = VSC_SCL_Scalarization_GetShader(scl);
    kind = VIR_Symbol_GetKind(sym);
    type = VIR_Symbol_GetType(sym);
    base_type_id = VIR_Type_GetBaseTypeId(type);
    base_type = VIR_Shader_GetTypeFromId(shader, base_type_id);
    storage = VIR_Symbol_GetStorageClass(sym);
    gcmASSERT(kind == VIR_SYM_VIRREG);

    VIR_Shader_AddSymbolWithName(shader, kind, gcvNULL, base_type, storage, &new_sym_id);

    *new_sym = VIR_GetFuncSymFromId(VIR_Shader_GetCurrentFunction(shader), new_sym_id);
    return errcode;
}

static VSC_ErrCode _VSC_SCL_GenerateNewSymbolForShader(
    IN OUT VSC_SCL_Scalarization* scl
    )
{
    VSC_ErrCode errcode  = VSC_ERR_NONE;
    VSC_HASH_ITERATOR iter;
    VIR_Symbol* sym;
    VSC_OPTN_SCLOptions* options = VSC_SCL_Scalarization_GetOptions(scl);

    vscHTBLIterator_Init(&iter, VSC_SCL_Scalarization_GetArrayInfos(scl));
    for(sym = (VIR_Symbol*)vscHTBLIterator_First(&iter);
        sym != gcvNULL; sym = (VIR_Symbol*)vscHTBLIterator_Next(&iter))
    {
        VSC_SCL_ArrayInfo* array_info;
        gctINT index;
        array_info = _VSC_SCL_Scalarization_GetArrayInfo(scl, sym);

        if(!VSC_SCL_ArrayInfo_GetDynamicIndexing(array_info))
        {
            index = vscBV_FindSetBitForward(VSC_SCL_ArrayInfo_GetConstIndexes(array_info), 0);
            while(index != INVALID_BIT_LOC)
            {
                VIR_Symbol* new_sym = gcvNULL;
                errcode = _VSC_SCL_GenerateNewSymbol(scl, sym, index, &new_sym);
                _VSC_SCL_ArrayInfo_MapNewSymbol(array_info, index, new_sym);
                index = vscBV_FindSetBitForward(VSC_SCL_ArrayInfo_GetConstIndexes(array_info), index);
            }
        }

        if(VSC_UTILS_MASK(VSC_OPTN_SCLOptions_GetTrace(options), VSC_OPTN_SCLOptions_TRACE_DUMP_NEW_SYMBOLS))
        {
            VIR_Dumper* dumper = VSC_SCL_Scalarization_GetDumper(scl);
            _VSC_SCL_ArrayInfo_DumpNewSymbols(VSC_SCL_ArrayInfo_GetNewSymbols(array_info), dumper);
        }
    }

    return errcode;
}

static VSC_ErrCode _VSC_SCL_ModifyOperandWithNewSymbol(
    IN OUT VIR_Operand* opnd,
    IN VIR_Symbol* sym
    )
{
    VSC_ErrCode errcode = VSC_ERR_NONE;

    VIR_Operand_SetOpKind(opnd, VIR_OPND_SYMBOL);
    VIR_Operand_SetSym(opnd, sym);
    return errcode;
}

/* do substitution for all operands in the shader */
static VSC_ErrCode _VSC_SCL_DoSubstitutionForShader(
    IN OUT VSC_SCL_Scalarization* scl
    )
{
    VSC_ErrCode errcode  = VSC_ERR_NONE;
    VIR_Shader* shader = VSC_SCL_Scalarization_GetShader(scl);
    VSC_HASH_ITERATOR array_info_iter;
    VIR_Symbol* sym;

    vscHTBLIterator_Init(&array_info_iter, VSC_SCL_Scalarization_GetArrayInfos(scl));
    for(sym = (VIR_Symbol*)vscHTBLIterator_First(&array_info_iter);
            sym != gcvNULL; sym = (VIR_Symbol*)vscHTBLIterator_Next(&array_info_iter))
    {
        VSC_SCL_ArrayInfo* array_info = _VSC_SCL_Scalarization_GetArrayInfo(scl, sym);
        if(HTBL_GET_ITEM_COUNT(VSC_SCL_ArrayInfo_GetNewSymbols(array_info)))
        {
            VSC_UL_ITERATOR opnd_iter;
            VSC_SCL_OperandListNode* opnd_node;
            vscULIterator_Init(&opnd_iter, VSC_SCL_ArrayInfo_GetOpndList(array_info));
            for(opnd_node = (VSC_SCL_OperandListNode*)vscULIterator_First(&opnd_iter);
                opnd_node != gcvNULL; opnd_node = (VSC_SCL_OperandListNode*)vscULIterator_Next(&opnd_iter))
            {
                VIR_Operand* opnd = VSC_SCL_OperandListNode_GetOpnd(opnd_node);
                VIR_TypeId type_id = VIR_Operand_GetType(opnd);
                VIR_Type* type = VIR_Shader_GetTypeFromId(shader, type_id);

                if(VIR_Type_GetKind(type) == VIR_TY_ARRAY)
                {
                    gctINT index = VIR_Operand_GetRelIndexing(opnd);
                    VIR_Symbol* new_sym = _VSC_SCL_ArrayInfo_GetNewSymbol(array_info, index);
                    _VSC_SCL_ModifyOperandWithNewSymbol(opnd, new_sym);
                }
            }
        }
    }

    return errcode;
}

VSC_ErrCode VSC_SCL_Scalarization_PerformOnShader(
    IN OUT VSC_SCL_Scalarization* scl
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_Shader* shader = VSC_SCL_Scalarization_GetShader(scl);
    VIR_Function* old_curr_func = VIR_Shader_GetCurrentFunction(shader);
    VSC_OPTN_SCLOptions* options = VSC_SCL_Scalarization_GetOptions(scl);

    /* print title */
    if(VSC_OPTN_SCLOptions_GetTrace(options))
    {
        VIR_Dumper* dumper = VSC_SCL_Scalarization_GetDumper(scl);
        VIR_LOG(dumper, VSC_TRACE_BAR_LINE);
        VIR_LOG(dumper, "Scalarization");
        VIR_LOG(dumper, VSC_TRACE_BAR_LINE);
    }

    /* dump input shader */
    if(VSC_UTILS_MASK(VSC_OPTN_SCLOptions_GetTrace(options), VSC_OPTN_SCLOptions_TRACE_INPUT_SHADER))
    {
        VIR_Dumper* dumper = VSC_SCL_Scalarization_GetDumper(scl);
        VIR_LOG(dumper, "Input shader:");
        /* VIR_Shader_Dump(dumper, shader); */
    }

    /* collect array information */
    errCode = _VSC_SCL_CollectInformationforShader(scl);

    /* generate new symbols */
    errCode = _VSC_SCL_GenerateNewSymbolForShader(scl);

    /* do symbol substitution */
    errCode = _VSC_SCL_DoSubstitutionForShader(scl);

    /* dump output functions */
    if(VSC_UTILS_MASK(VSC_OPTN_SCLOptions_GetTrace(options), VSC_OPTN_SCLOptions_TRACE_INPUT_FUNCTIONS))
    {
        VIR_Dumper* dumper = VSC_SCL_Scalarization_GetDumper(scl);
        VIR_FuncIterator func_iter;
        VIR_FunctionNode* func_node;

        VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(shader));
        for(func_node = VIR_FuncIterator_First(&func_iter);
            func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
        {
            VIR_Function* func = func_node->function;

            VIR_LOG(dumper, "Output function:");
            VIR_Function_Dump(dumper, func);
        }
    }

    /* dump output shader */
    if(VSC_UTILS_MASK(VSC_OPTN_SCLOptions_GetTrace(options),
                      VSC_OPTN_SCLOptions_TRACE_OUTPUT_SHADER) ||
       gcSHADER_DumpCodeGenVerbose(shader))
    {
            VIR_Shader_Dump(gcvNULL, "After scalar replacement.", shader, gcvTRUE);
    }

    VIR_Shader_SetCurrentFunction(shader, old_curr_func);
    return errCode;
}

