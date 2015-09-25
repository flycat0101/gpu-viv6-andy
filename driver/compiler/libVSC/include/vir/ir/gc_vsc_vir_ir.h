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


#ifndef __gc_vsc_vir_ir_h_
#define __gc_vsc_vir_ir_h_

#include "utils/gc_vsc_utils_table.h"

BEGIN_EXTERN_C()


/* forward declarations */

typedef struct _VIR_UNIFORM         VIR_Uniform;
typedef struct _VIR_FUNCTION        VIR_Function;
typedef struct _VIR_UNIFORMBLOCK    VIR_UniformBlock;
typedef struct _VIR_STORAGEBLOCK    VIR_StorageBlock;
typedef struct _VIR_IOBLOCK         VIR_IOBlock;
typedef struct _VIR_LABEL           VIR_Label;
typedef struct _VIR_CONST           VIR_Const;
typedef struct _VIR_INSTRUCTION     VIR_Instruction;
typedef struct _VIR_OPERAND         VIR_Operand;
typedef struct _VIR_PARMPASSING     VIR_ParmPassing;
typedef struct _VIR_TEXLDPARM       VIR_TexldParm;
typedef struct _VIR_OPERANDLIST     VIR_OperandList;
typedef struct _VIR_SYMBOL          VIR_Symbol;
typedef struct _VIR_DUMPER          VIR_Dumper;
typedef struct _VIR_TYPE            VIR_Type;

typedef gctUINT  VIR_Id;            /* general id */
typedef VIR_Id   VIR_NameId;        /* index to string table */
typedef VIR_Id   VIR_SymId;         /* index to symbol table */
typedef VIR_Id   VIR_ConstId;       /* index to constant value table */
typedef VIR_Id   VIR_VirRegId;      /* virtual register index number: 10 in temp(10) */
typedef VIR_Id   VIR_FunctionId;    /* index to function table */
typedef VIR_Id   VIR_LabelId;       /* index to label table */
typedef VIR_Id   VIR_HwRegId;       /* hardware register */
typedef VIR_Id   VIR_OperandId;     /* index to operand table */
typedef VIR_Id   VIR_UniformId;     /* uniform index number: 3 in uniform(3) */

typedef VSC_BLOCK_TABLE  VIR_StringTable;    /* string table */
typedef VSC_BLOCK_TABLE  VIR_SymTable;       /* the element size is sizeof(VIR_Sym) */
typedef VSC_BLOCK_TABLE  VIR_TypeTable;
typedef VSC_BLOCK_TABLE  VIR_LabelTable;
typedef VSC_BLOCK_TABLE  VIR_ConstTable;
typedef VSC_BLOCK_TABLE  VIR_TempTable;
typedef VSC_BLOCK_TABLE  VIR_OperandTable;

typedef struct _VIR_IDLIST VIR_IdList;
typedef VIR_IdList      VIR_SymIdList;
typedef VIR_IdList      VIR_VariableIdList;
typedef VIR_IdList      VIR_TypeIdList;
typedef VIR_IdList      VIR_UniformIdList;
typedef VIR_IdList      VIR_UBOIdList;
typedef VIR_IdList      VIR_SBOIdList;          /* ES 3.1: storage block object */
typedef VIR_IdList      VIR_IOBIdList;          /* ES 3.1 ext: IO block object */
typedef VIR_IdList      VIR_AttributeIdList;
typedef VIR_AttributeIdList  VIR_InputIdList;   /* input aliased as attribute */
typedef VIR_IdList      VIR_OutputIdList;
typedef VIR_IdList      VIR_BufferIdList;
typedef VIR_IdList      VIR_VirRegIdList;     /* temp register id list */

typedef struct _VIR_VALUELIST VIR_ValueList;

/* Simple names of structures of CFA that IR needs refererence */
typedef struct _VIR_CONTROL_FLOW_GRAPH  VIR_CFG;
typedef struct _VIR_CALL_GRAPH          VIR_CG;
typedef struct _VIR_BASIC_BLOCK         VIR_BB;
typedef struct _VIR_FUNC_BLOCK          VIR_FB;

/* accessors */
#define VIR_OPCODE_GetSrcOperandNum(Opcode)   \
    (VIR_OpcodeInfo[(Opcode)].srcNum)

#define VIR_OPCODE_isWritten2Dest(Opcode)   \
    (VIR_OpcodeInfo[(Opcode)].write2Dest)

#define VIR_OPCODE_level(Opcode)   \
    (VIR_OpcodeInfo[(Opcode)].level)

#define VIR_OPCODE_hasDest(Opcode)   \
    (VIR_OpcodeInfo[(Opcode)].flags & VIR_OPFLAG_HasDest)

#define VIR_OPCODE_isTranscendental(Opcode)   \
    (VIR_OpcodeInfo[(Opcode)].flags & VIR_OPFLAG_Transcendental)

#define VIR_OPCODE_isIntegerOnly(Opcode)   \
    (VIR_OpcodeInfo[(Opcode)].flags & VIR_OPFLAG_IntegerOnly)

#define VIR_OPCODE_isControlFlow(Opcode)   \
    (VIR_OpcodeInfo[(Opcode)].flags & VIR_OPFLAG_ControlFlow)

#define VIR_OPCODE_isOCLOnly(Opcode)   \
    (VIR_OpcodeInfo[(Opcode)].flags & VIR_OPFLAG_OCLOnly)

#define VIR_OPCODE_isOGLOnly(Opcode)   \
    (VIR_OpcodeInfo[(Opcode)].flags & VIR_OPFLAG_OGLOnly)

#define VIR_OPCODE_isVXOnly(Opcode)   \
    (VIR_OpcodeInfo[(Opcode)].flags & VIR_OPFLAG_VXOnly)

#define VIR_OPCODE_EVISModifier_SrcNo(Opcode)    \
    (((VIR_OpcodeInfo[(Opcode)].flags & VIR_OPFLAG_EVIS_Modifier_MASK)  >> VIR_OPFLAG_EVIS_Modifier_SHIFT) - 1)

/* SrcNo start from 1 */
#define VIR_SrcNo2EVISModifier(SrcNo)    \
    ((SrcNo) << VIR_OPFLAG_EVIS_Modifier_SHIFT)

#define VIR_OPCODE_useCondCode(Opcode)   \
    (VIR_OpcodeInfo[(Opcode)].flags & VIR_OPFLAG_UseCondCode)

#define VIR_OPCODE_isComponentwise(Opcode)   \
    (VIR_OpcodeInfo[(Opcode)].flags & VIR_OPFLAG_Componentwise)

#define VIR_OPCODE_GetSourceComponentWiseFlag(SrcNum)       \
    ((SrcNum) == 0 ? VIR_OPFLAG_Src0Componentwise :         \
        (SrcNum) == 1 ? VIR_OPFLAG_Src1Componentwise :      \
        (SrcNum) == 2 ? VIR_OPFLAG_Src2Componentwise : VIR_OPFLAG_Src3Componentwise)

#define VIR_OPCODE_isSourceComponentwise(Opcode, SrcNum)    \
    (VIR_OpcodeInfo[(Opcode)].flags & (VIR_OPCODE_GetSourceComponentWiseFlag(SrcNum)))

#define VIR_OPCODE_DestOnlyUseEnable(Opcode)   \
    (VIR_OpcodeInfo[(Opcode)].flags & VIR_OPFLAG_OnlyUseEnable)

#define VIR_OPCODE_ReadMem(Opcode)   \
    (VIR_OpcodeInfo[(Opcode)].flags & VIR_OPFLAG_MemoryRead)

#define VIR_OPCODE_WriteMem(Opcode)   \
    (VIR_OpcodeInfo[(Opcode)].flags & VIR_OPFLAG_MemoryWrite)

#define VIR_OPCODE_ReadMemOnly(Opcode)   \
    (VIR_OPCODE_ReadMem(Opcode) && (!(VIR_OPCODE_WriteMem(Opcode))))

#define VIR_OPCODE_ReadAndWriteMem(Opcode)   \
    (VIR_OPCODE_ReadMem(Opcode) &&  VIR_OPCODE_WriteMem(Opcode))

#define VIR_OPCODE_MemOp(Opcode)   \
    (VIR_OPCODE_ReadMem(Opcode) ||  VIR_OPCODE_WriteMem(Opcode))

#define VIR_OPCODE_IsExpr(Opcode)   \
    (VIR_OpcodeInfo[(Opcode)].flags & VIR_OPFLAG_Expression)

#define VIR_OPCODE_ExpectedResultPrecisionFromHighest(Opcode)   \
    ((VIR_OpcodeInfo[(Opcode)].flags & VIR_OPFLAG_ExpdPrecFromBits) == VIR_OPFLAG_ExpdPrecFromHighest)

#define VIR_OPCODE_ExpectedResultPrecisionFromSrc0(Opcode)   \
    ((VIR_OpcodeInfo[(Opcode)].flags & VIR_OPFLAG_ExpdPrecFromBits) == VIR_OPFLAG_ExpdPrecFromSrc0)

#define VIR_OPCODE_ExpectedResultPrecisionFromHighestInSrc1Src2(Opcode)   \
    ((VIR_OpcodeInfo[(Opcode)].flags & VIR_OPFLAG_ExpdPrecFromBits) == VIR_OPFLAG_ExpdPrecFromSrc12)

#define VIR_OPCODE_ExpectedResultPrecisionFromSrc2(Opcode)   \
    ((VIR_OpcodeInfo[(Opcode)].flags & VIR_OPFLAG_ExpdPrecFromBits) == VIR_OPFLAG_ExpdPrecFromSrc2)

#define VIR_OPCODE_ExpectedResultPrecisionFromNone(Opcode)   \
    ((VIR_OpcodeInfo[(Opcode)].flags & VIR_OPFLAG_ExpdPrecFromBits) == 0)

#define VIR_OPCODE_isConditionBranch(Opcode)     \
    ((Opcode) == VIR_OP_JMPC ||          \
     (Opcode) == VIR_OP_JMP_ANY)

#define VIR_OPCODE_isBranch(Opcode)     \
    ((Opcode) == VIR_OP_JMP ||          \
     (Opcode) == VIR_OP_JMP_ANY ||          \
     (Opcode) == VIR_OP_JMPC)

#define VIR_OPCODE_isBBPrefix(Opcode)    \
    ((Opcode) == VIR_OP_LABEL)

#define VIR_OPCODE_isBBSuffix(Opcode)      \
    (VIR_OPCODE_isBranch(Opcode) ||     \
     (Opcode) == VIR_OP_RET)

#define VIR_OPCODE_isCall(Opcode)       \
    ((Opcode) == VIR_OP_CALL ||         \
     (Opcode) == VIR_OP_ICALL)

#define VIR_OPCODE_isTexLd(Opcode)          \
    ((Opcode) == VIR_OP_TEXLD ||            \
     (Opcode) == VIR_OP_TEXLDPROJ ||        \
     (Opcode) == VIR_OP_TEXLDPCF ||         \
     (Opcode) == VIR_OP_TEXLDPCFPROJ ||     \
     (Opcode) == VIR_OP_TEXLD_BIAS ||       \
     (Opcode) == VIR_OP_TEXLD_BIAS_PCF ||   \
     (Opcode) == VIR_OP_TEXLD_PLAIN ||      \
     (Opcode) == VIR_OP_TEXLD_PCF ||        \
     (Opcode) == VIR_OP_TEXLDB ||           \
     (Opcode) == VIR_OP_TEXLDD ||           \
     (Opcode) == VIR_OP_TEXLD_G ||          \
     (Opcode) == VIR_OP_TEXLDL ||           \
     (Opcode) == VIR_OP_TEXLDP ||           \
     (Opcode) == VIR_OP_TEXLD_LOD ||        \
     (Opcode) == VIR_OP_TEXLD_LOD_PCF ||    \
     (Opcode) == VIR_OP_TEXLD_G_PCF ||      \
     (Opcode) == VIR_OP_TEXLD_U_PLAIN ||    \
     (Opcode) == VIR_OP_TEXLD_U_LOD ||      \
     (Opcode) == VIR_OP_TEXLD_U_BIAS ||     \
     (Opcode) == VIR_OP_TEXLD_GATHER ||     \
     (Opcode) == VIR_OP_TEXLD_GATHER_PCF || \
     (Opcode) == VIR_OP_TEXLD_FETCH_MS ||   \
     (Opcode) == VIR_OP_LODQ)

#define VIR_OPCODE_isUnComponentWised(Opcode) \
    ((Opcode) == VIR_OP_DP2         ||      \
     (Opcode) == VIR_OP_DP3         ||      \
     (Opcode) == VIR_OP_DP4         ||      \
     (Opcode) == VIR_OP_NORM_DP2    ||      \
     (Opcode) == VIR_OP_NORM_DP3    ||      \
     (Opcode) == VIR_OP_NORM_DP4    ||      \
     (Opcode) == VIR_OP_NORM        ||      \
     (Opcode) == VIR_OP_CROSS       ||      \
     (Opcode) == VIR_OP_IMG_ADDR    ||      \
     (Opcode) == VIR_OP_IMG_ADDR_3D ||      \
     (Opcode) == VIR_OP_ATOMCMPXCHG ||      \
     (Opcode) == VIR_OP_LOAD_ATTR   ||      \
     (Opcode) == VIR_OP_LOAD        ||      \
     VIR_OPCODE_isTexLd(Opcode))

#define VIR_OPCODE_isMemLd(Opcode)      \
    ((Opcode) == VIR_OP_LOAD)

#define VIR_OPCODE_isMemSt(Opcode)      \
    ((Opcode) == VIR_OP_STORE)

#define VIR_OPCODE_isAtom(Opcode)           \
    ((Opcode) == VIR_OP_ATOMADD     ||      \
    (Opcode) == VIR_OP_ATOMSUB      ||      \
    (Opcode) == VIR_OP_ATOMCMPXCHG  ||      \
    (Opcode) == VIR_OP_ATOMMAX      ||      \
    (Opcode) == VIR_OP_ATOMMIN      ||      \
    (Opcode) == VIR_OP_ATOMOR       ||      \
    (Opcode) == VIR_OP_ATOMAND      ||      \
    (Opcode) == VIR_OP_ATOMXOR      ||      \
    (Opcode) == VIR_OP_ATOMXCHG)

#define VIR_OPCODE_isNormalOp(Opcode)   \
    (!VIR_OPCODE_isTexLd(Opcode) && !VIR_OPCODE_isMemLd(Opcode))

/* not finished */
#define VIR_OPCODE_BITWISE(Opcode)     \
    ((Opcode == VIR_OP_AND_BITWISE)  ||      \
     (Opcode == VIR_OP_XOR_BITWISE)  ||      \
     (Opcode == VIR_OP_OR_BITWISE)   ||      \
     (Opcode == VIR_OP_NOT_BITWISE)          \
    )

/* not finished */
#define VIR_OPCODE_UseCondition(Opcode)     \
    ((Opcode) == VIR_OP_MOVA ||             \
     (Opcode) == VIR_OP_SETP ||             \
     (Opcode) == VIR_OP_SELECT ||           \
     /*(Opcode) == VIR_OP_Texkill ||*/      \
     (Opcode) == VIR_OP_CMP ||              \
     VIR_OPCODE_isBranch(Opcode))

#define VIR_OPCODE_CONDITIONAL_WRITE(Opcode) \
    ((Opcode) == VIR_OP_CMOV)

#define VIR_SymTable_MaxValidId(SymTable)   BT_GET_MAX_VALID_ID(SymTable)

#define VIR_Function_GetLastInstId(Func)    ((Func)->_lastInstId)
#define VIR_Function_GetAndIncressLastInstId(Func)  ((Func)->_lastInstId++)
#define VIR_Function_GetLabelId(Func)       ((Func)->_labelId)
#define VIR_Function_GetAndIncressLabelId(Func)     ((Func)->_labelId++)
#define VIR_Function_GetSymId(Func)     ((Func)->funcSym)
#define VIR_Function_GetSymbol(Func)    VIR_Shader_GetSymFromId((Func)->hostShader, VIR_Function_GetSymId(Func))
#define VIR_Function_GetType(Func)      VIR_Symbol_GetType(VIR_Function_GetSymbol(Func))
#define VIR_Function_GetNameID(Func)    VIR_Symbol_GetName(VIR_Function_GetSymbol(Func))
#define VIR_Function_GetNameString(Func)    VIR_Shader_GetStringFromId((Func)->hostShader, VIR_Function_GetNameID(Func))
#define VIR_Function_GetInstList(Func)  ((VIR_InstList*)&((Func)->instList))
#define VIR_Function_GetInstCount(Func) (VIR_InstList_GetCount(&(Func)->instList))
#define VIR_Function_GetInstStart(Func) ((Func)->instList.pHead)
#define VIR_Function_GetInstEnd(Func)   ((Func)->instList.pTail)
#define VIR_Function_GetShader(Func)    ((Func)->hostShader)
#define VIR_Function_GetFlags(Func)      ((Func)->flags)
#define VIR_Function_GetSymTable(Func)  ((VIR_SymTable*)&((Func)->symTable))
#define VIR_Function_GetLabelTable(Func)  ((VIR_LabelTable*)&((Func)->labelTable))
#define VIR_Function_GetOperandTable(Func)  ((VIR_OperandTable*)&((Func)->operandTable))

#define VIR_Function_GetLabelFromId(Func, LabelId)  (VIR_Label*)(VIR_GetEntryFromId(&((Func)->labelTable), (LabelId)))
#define VIR_Function_GetOperandFromId(Func, OpndId)  (VIR_Operand*)(VIR_GetEntryFromId(&((Func)->operandTable), (OpndId)))

#define VIR_Function_GetFuncBlock(Func) ((Func)->pFuncBlock)
#define VIR_Function_GetCFG(Func)       (&((Func)->pFuncBlock->cfg))

#define VIR_InstList_GetCount(IList)    (vscBILST_GetNodeCount((VSC_BI_LIST *)IList))
#define VIR_Inst_GetPrev(inst)          ((VIR_Instruction*)((inst)->biLstNode.pPrevNode))
#define VIR_Inst_GetNext(inst)          ((VIR_Instruction*)((inst)->biLstNode.pNextNode))
#define VIR_Inst_GetFunction(Inst)      ((Inst)->_parentUseBB ?                     \
                                            BB_GET_FUNC((Inst)->parent.BB) :        \
                                            (Inst)->parent.function)
#define VIR_Inst_GetBasicBlock(Inst)    ((Inst)->_parentUseBB ?                     \
                                            (Inst)->parent.BB : gcvNULL)
#define VIR_Inst_GetCaleeId(Inst)       ((Inst)->_opcode == VIR_OP_CALL ?           \
                                            VIR_Operand_GetFunctionId((Inst)->dest) \
                                            : VIR_INVALID_ID)
#define VIR_Inst_GetJmpLabel(Inst)      VIR_Operand_GetLabel(VIR_Inst_GetDest(Inst))
#define VIR_Inst_GetDest(Inst)          ((Inst)->dest)
#define VIR_Inst_GetSource(Inst, SrcNo) (((SrcNo) >= VIR_MAX_SRC_NUM || ((gctUINT)SrcNo >= VIR_Inst_GetSrcNum(Inst))) ? \
                                              gcvNULL : (Inst)->src[(SrcNo)])
#define VIR_Inst_GetShader(Inst)        (VIR_Function_GetShader(VIR_Inst_GetFunction(Inst)))

#define VIR_Inst_GetId(Inst)            ((Inst)->id_)
#define VIR_Inst_GetOpcode(Inst)        ((VIR_OpCode)(Inst)->_opcode)
#define VIR_Inst_GetInstType(Inst)      ((Inst)->_instType)
#define VIR_Inst_GetConditionOp(Inst)   ((VIR_ConditionOp)(Inst)->_condOp)
#define VIR_Inst_GetFlag(Inst)          ((VIR_InstFlag)(Inst)->_instFlag)
#define VIR_Inst_GetSrcNum(Inst)        ((Inst)->_srcOpndNum)
#define VIR_Inst_GetThreadMode(Inst)    ((VIR_ThreadMode)(Inst)->_threadMode)
#define VIR_Inst_GetEnable(Inst)        (VIR_Operand_GetEnable((Inst)->dest))
#define VIR_Inst_isPrecise(Inst)        ((Inst)->_isPrecise)
#define VIR_Inst_GetPatched(Inst)       ((Inst)->_patched)
#define VIR_Inst_GetDual16ExpandSeq(Inst) ((Inst)->_dual16ExpandSeq)
#define VIR_Inst_isWritten2Dest(Inst)   (VIR_Opcode_isWritten2Dest(VIR_Inst_GetOpcode(Inst))

#define VIR_Inst_SetNext(inst, next)        ((inst)->biLstNode.pNextNode = (VIR_Instruction*)(next))
#define VIR_Inst_SetPrev(inst, prev)        ((inst)->biLstNode.pPrevNode = (VIR_Instruction*)(prev))
#define VIR_Inst_SetFunction(Inst, pFunc)   do {                                  \
                                                (Inst)->_parentUseBB = 0;          \
                                                (Inst)->parent.function = (pFunc);\
                                            } while(0)

#define VIR_Inst_SetBasicBlock(Inst, pBB)   do {                                 \
                                                (Inst)->_parentUseBB = 1;         \
                                                (Inst)->parent.BB = (pBB);       \
                                            } while(0)
#define VIR_Inst_SetId(Inst, Id)            do { (Inst)->id_ = (Id); } while (0)
#define VIR_Inst_SetOpcode(Inst, Opcode)    do { (Inst)->_opcode = (Opcode); } while (0)
#define VIR_Inst_SetInstType(Inst, Val)     do { (Inst)->_instType = (Val); } while (0)
#define VIR_Inst_SetSrcNum(Inst, Val)       do { (Inst)->_srcOpndNum = (Val); } while (0)
#define VIR_Inst_SetFlag(Inst, Val)         do { (Inst)->_instFlag = (Val); } while (0)
#define VIR_Inst_SetIsPrecise(Inst, Val)    do { (Inst)->_isPrecise = ((Val) == 0) ? 0 : 1; } while (0)
#define VIR_Inst_SetConditionOp(Inst, COP)  do { (Inst)->_condOp = (gctUINT)(COP); } while (0)
#define VIR_Inst_SetEnable(Inst, Enable)    do { VIR_Operand_SetEnable((Inst)->dest, (Enable)); } while (0)
#define VIR_Inst_SetThreadMode(Inst, TMode) do { (Inst)->_threadMode = (gctUINT)(TMode); } while (0)
#define VIR_Inst_SetDest(Inst, Dest)        do { (Inst)->dest = (Dest); } while (0)
#define VIR_Inst_SetSource(Inst, SrcNo, Src)    do { gcmASSERT(SrcNo < VIR_MAX_SRC_NUM); (Inst)->src[(SrcNo)] = (Src); } while (0)
#define VIR_Inst_SetPatched(Inst, Val)      do { (Inst)->_patched = (Val); } while (0)
#define VIR_Inst_SetDual16ExpandSeq(Inst, Val)  do { (Inst)->_dual16ExpandSeq = (Val); } while (0)
/* Inst iterator */
typedef VSC_BL_ITERATOR VIR_InstIterator;
#define VIR_InstIterator_Init(Iter, InstList) vscBLIterator_Init((Iter), (VSC_BI_LIST *)(InstList))
#define VIR_InstIterator_First(Iter)          (VIR_Instruction *)vscBLIterator_First((Iter))
#define VIR_InstIterator_Next(Iter)           (VIR_Instruction *)vscBLIterator_Next((Iter))
#define VIR_InstIterator_Prev(Iter)           (VIR_Instruction *)vscBLIterator_Prev((Iter))
#define VIR_InstIterator_Last(Iter)           (VIR_Instruction *)vscBLIterator_Last((Iter))

#define VIR_Inst_Count(InstList)              vscBILST_GetNodeCount((VSC_BI_LIST *)InstList)



#define VIR_OPCODE_GetName(OpCode)          (VIR_OpName[(OpCode)])
#define VIR_Intrinsic_GetName(Instrinsic)   (VIR_IntrinsicName[(Instrinsic)])
#define VIR_CondOp_GetName(CondOp)          (VIR_CondOpName[(CondOp)])
#define VIR_RoundMode_GetName(RoundMode)    (VIR_RoundModeName[(RoundMode)])
#define VIR_DestModifier_GetName(DMod)      (VIR_DestModifierName[(DMod)])
#define VIR_SrcModifier_GetName(SMod)       (VIR_SrcModifierName[(SMod)])


#define VIR_Operand_GetOpKind(Opnd)         ((Opnd)->header._opndKind)
#define VIR_Operand_isUndef(Opnd)           (VIR_Operand_GetOpKind(Opnd) == VIR_OPND_UNDEF)
#define VIR_Operand_isVirReg(Opnd)          (VIR_Operand_GetOpKind(Opnd) == VIR_OPND_VIRREG)
#define VIR_Operand_isSymbol(Opnd)          (VIR_Operand_GetOpKind(Opnd) == VIR_OPND_SYMBOL)
#define VIR_Operand_isTexldParm(Opnd)       (VIR_Operand_GetOpKind(Opnd) == VIR_OPND_TEXLDPARM)

#define VIR_Operand_GetHeader(Opnd)         ((Opnd)->header)
#define VIR_Operand_GetIndex(Opnd)          ((Opnd)->header._index)
#define VIR_Operand_isLvalue(Opnd)          (!VIR_Operand_isTexldParm(Opnd) && (Opnd)->header._lvalue)
#define VIR_Operand_isRegAllocated(Opnd)    ((Opnd)->_regAllocated)
#define VIR_Operand_GetHwRegClass(Opnd)     ((Opnd)->_regClass)
#define VIR_Operand_GetHwRegId(Opnd)        ((VIR_HwRegId)(Opnd)->_hwRegId)
#define VIR_Operand_GetHwShift(Opnd)        ((Opnd)->_hwShift)
#define VIR_Operand_GetHIHwRegId(Opnd)      ((VIR_HwRegId)(Opnd)->_HIhwRegId)
#define VIR_Operand_GetHIHwShift(Opnd)      ((Opnd)->_HIhwShift)
#define VIR_Operand_GetTexModifierFlag(Opnd)  (((VIR_Operand_Header_TM*)&((Opnd)->header))->_texmodifiers)

#define VIR_Operand_GetRoundMode(Opnd)      ((VIR_RoundMode)(Opnd)->header._roundMode)
#define VIR_Operand_GetModifier(Opnd)       ((VIR_Modifier)(Opnd)->header._modifier)

#define VIR_Operand_GetType(Opnd)           ((Opnd)->_opndType)
#define VIR_Operand_GetEnable(Opnd)         ((VIR_Enable)(Opnd)->_swizzleOrEnable)
#define VIR_Operand_GetSwizzle(Opnd)        ((VIR_Swizzle)(Opnd)->_swizzleOrEnable)

#define VIR_Operand_GetSymbol(Opnd)         ((Opnd)->u1.sym)
#define VIR_Operand_GetSymbolId_(Opnd)      VIR_Symbol_GetIndex(VIR_Operand_GetSymbol(Opnd))

#define VIR_Operand_GetFunction(Opnd)       ((Opnd)->u1.func)
#define VIR_Operand_GetFunctionId_(Opnd)     (VIR_Function_GetSymId((Opnd)->u1.func))

#define VIR_Operand_GetLabel(Opnd)          ((Opnd)->u1.label)
#define VIR_Operand_GetLabelId_(Opnd)       VIR_Symbol_GetIndex((Opnd)->u1.label)

#define VIR_Operand_GetParameters(Opnd)     ((Opnd)->u1.argList)
#define VIR_Operand_GetIntrinsicKind(Opnd)  ((Opnd)->u1.intrinsic)
#define VIR_Operand_GetFieldBase(Opnd)      ((Opnd)->u1.base)
#define VIR_Operand_GetFieldId(Opnd)        ((Opnd)->u2.fieldId)
#define VIR_Operand_GetConstId(Opnd)        ((Opnd)->u1.constId)
#define VIR_Operand_GetArrayBase(Opnd)      ((Opnd)->u1.base)
#define VIR_Operand_GetImmediateInt(Opnd)   ((Opnd)->u1.iConst)
#define VIR_Operand_GetImmediateUint(Opnd)  ((Opnd)->u1.uConst)
#define VIR_Operand_GetImmediateFloat(Opnd) ((Opnd)->u1.fConst)
#define VIR_Operand_GetEvisModifier(Opnd)   ((Opnd)->u1.evisModifier.u1)

#define VIR_Operand_GetIsConstIndexing(Opnd)    ((Opnd)->u2.vlInfo._isConstIndexing)
#define VIR_Operand_GetRelAddrMode(Opnd)        ((VIR_Indexed)(Opnd)->u2.vlInfo._relAddrMode)
#define VIR_Operand_GetRelAddrLevel(Opnd)       ((VIR_INDEXED_LEVEL)(Opnd)->u2.vlInfo._relAddrLevel)
#define VIR_Operand_GetMatrixConstIndex(Opnd)   ((Opnd)->u2.vlInfo._matrixConstIndex)
#define VIR_Operand_GetRelIndexing(Opnd)        ((VIR_SymId)(Opnd)->u2.vlInfo._relIndexing)
#define VIR_Operand_GetConstIndexingImmed(Opnd) ((Opnd)->u2.vlInfo._relIndexing)
#define VIR_Operand_isSymLocal(Opnd)            ((Opnd)->u2.vlInfo._isSymLocal != 0)
#define VIR_Operand_GetArrayIndex(Opnd)         ((Opnd)->u2.arrayIndex)
#define VIR_Operand_GetFieldId(Opnd)            ((Opnd)->u2.fieldId)
#define VIR_Operand_GetTexldModifier(Opnd, Mod) (((VIR_Operand_TexldModifier *)(Opnd))->tmodifier[(Mod)])
#define VIR_Operand_GetTexldBias(Opnd)          (VIR_Operand_GetTexldModifier(Opnd, VIR_TEXLDMODIFIER_BIAS))
#define VIR_Operand_GetTexldLod(Opnd)           (VIR_Operand_GetTexldModifier(Opnd, VIR_TEXLDMODIFIER_LOD))
#define VIR_Operand_GetTexldOffset(Opnd)        (VIR_Operand_GetTexldModifier(Opnd, VIR_TEXLDMODIFIER_OFFSET))
#define VIR_Operand_GetTexldGrad_dx(Opnd)       (VIR_Operand_GetTexldModifier(Opnd, VIR_TEXLDMODIFIER_DPDX))
#define VIR_Operand_GetTexldGrad_dy(Opnd)       (VIR_Operand_GetTexldModifier(Opnd, VIR_TEXLDMODIFIER_DPDY))
#define VIR_Operand_GetTexldGather_comp(Opnd)   (VIR_Operand_GetTexldModifier(Opnd, VIR_TEXLDMODIFIER_GATHERCOMP))
#define VIR_Operand_GetTexldGather_refz(Opnd)   (VIR_Operand_GetTexldModifier(Opnd, VIR_TEXLDMODIFIER_GATHERREFZ))
#define VIR_Operand_GetTexldFetchMS_sample(Opnd)   (VIR_Operand_GetTexldModifier(Opnd, VIR_TEXLDMODIFIER_FETCHMS_SAMPLE))


#define VIR_Operand_SetOpKind(Opnd, Val)        do { (Opnd)->header._opndKind = (Val); } while (0)
#define VIR_Operand_SetIndex(Opnd, Val)         do { (Opnd)->header._index = (gctUINT)(Val); } while (0)
#define VIR_Operand_SetLvalue(Opnd, Lvalue)     do { (Opnd)->header._lvalue = (gctUINT)((Lvalue) != 0); } while (0)
#define VIR_Operand_SetRegAllocated(Opnd, Val)  do { (Opnd)->_regAllocated = (gctUINT)(Val); } while (0)
#define VIR_Operand_SetHwRegClass(Opnd, Val)    do { (Opnd)->_regClass = (gctUINT)(Val); } while (0)
#define VIR_Operand_SetHwRegId(Opnd, Val)       do { (Opnd)->_hwRegId = (VIR_HwRegId) (Val); } while (0)
#define VIR_Operand_SetHwShift(Opnd, Val)       do { (Opnd)->_hwShift = (gctUINT) (Val); } while (0)
#define VIR_Operand_SetHIHwRegId(Opnd, Val)     do { (Opnd)->_HIhwRegId = (VIR_HwRegId) (Val); } while (0)
#define VIR_Operand_SetHIHwShift(Opnd, Val)     do { (Opnd)->_HIhwShift = (gctUINT) (Val); } while (0)

#define VIR_Operand_SetRoundMode(Opnd, Round)   do { (Opnd)->header._roundMode = (gctUINT)(Round); } while (0)
#define VIR_Operand_SetModifier(Opnd, Val)      do { (Opnd)->header._modifier = (gctUINT)(Val); } while (0)
#define VIR_Operand_SetTexModifierFlag(Opnd, F) do { ((VIR_Operand_Header_TM *)&((Opnd)->header))->_texmodifiers |= (F); } while (0)
#define VIR_Operand_SetPrecision(Opnd, Val)     do { (Opnd)->_precision = (gctUINT)Val; } while (0)
#define VIR_Operand_SetType(Opnd, TypeId)       do { (Opnd)->_opndType = (TypeId); } while (0)
#define VIR_Operand_SetSym(Opnd, Val)           do { (Opnd)->u1.sym = (Val); } while (0)
#define VIR_Operand_SetFunc(Opnd, Val)          do { (Opnd)->u1.func = (Val); } while (0)
#define VIR_Operand_SetIntrinsicKind(Opnd, Val) do { (Opnd)->u1.intrinsic = (Val); } while (0)
#define VIR_Operand_SetFieldBase(Opnd, Val)     do { (Opnd)->u1.base = (Val); } while (0)
#define VIR_Operand_SetFieldId(Opnd, Val)       do { (Opnd)->u2.fieldId = (Val); } while (0)
#define VIR_Operand_SetConstId(Opnd, Val)       do { (Opnd)->u1.constId = (Val); } while (0)
#define VIR_Operand_SetArrayBase(Opnd, Val)     do { (Opnd)->u1.base = (Val); } while (0)
#define VIR_Operand_SetEvisModifier(Opnd, Val)  do { (Opnd)->u1.evisModifier.u1 = (Val); } while (0)

#define VIR_Operand_SetIsConstIndexing(Opnd, Val)   do { (Opnd)->u2.vlInfo._isConstIndexing = (Val); } while (0)
#define VIR_Operand_SetRelAddrMode(Opnd, Val)       do { (Opnd)->u2.vlInfo._relAddrMode = (Val); } while (0)
#define VIR_Operand_SetRelAddrLevel(Opnd, Val)      do { (Opnd)->u2.vlInfo._relAddrLevel = (Val); } while (0)
#define VIR_Operand_SetMatrixConstIndex(Opnd, Val)  do { (Opnd)->u2.vlInfo._matrixConstIndex = (Val); } while (0)
#define VIR_Operand_SetIsSymLocal(Opnd, Val)        do { (Opnd)->u2.vlInfo._isSymLocal = (Val); } while (0)
#define VIR_Operand_SetArrayIndex(Opnd, Val)        do { (Opnd)->u2.arrayIndex = (Val); } while (0)
#define VIR_Operand_SetFieldId(Opnd, Val)           do { (Opnd)->u2.fieldId = (Val); } while (0)
#define VIR_Operand_SetTexldParameter(Opnd, Val)    do { (Opnd)->u2.texldParm = (Val); } while (0)

#define VIR_Link_GetReference(Link)                 ((Link)->referenced)
#define VIR_Link_SetReference(Link, Ref)            do { (Link)->referenced = (gctUINTPTR_T)(Ref); } while (0)

#define VIR_Shader_GetClientApiVersion(Shader)      ((Shader)->clientApiVersion)
#define VIR_Shader_GetId(Shader)                    ((Shader)->_id)
#define VIR_Shader_GetKind(Shader)                  ((Shader)->shaderKind)
#define VIR_Shader_GetDefaultUBOIndex(Shader)       ((Shader)->defaultUniformBlockIndex)
#define VIR_Shader_SetDefaultUBOIndex(Shader, d)    ((Shader)->defaultUniformBlockIndex = (d))
#define VIR_Shader_GetConstantUBOIndex(Shader)      ((Shader)->constUniformBlockIndex)
#define VIR_Shader_SetConstantUBOIndex(Shader, c)   ((Shader)->constUniformBlockIndex = (c))
#define VIR_Shader_GetSymFromId(Shader, SymId)      VIR_GetSymFromId(&((Shader)->symTable), (SymId))
#define VIR_Shader_GetTypeFromId(Shader, TypeId)    ((VIR_Type *)VIR_GetEntryFromId(&((Shader)->typeTable), (TypeId)))
#define VIR_Shader_GetStringFromId(Shader, StrId)   ((gctSTRING)VIR_GetEntryFromId(&((Shader)->stringTable), (StrId)))
#define VIR_Shader_GetConstFromId(Shader, ConstId)  ((VIR_Const*)VIR_GetEntryFromId(&((Shader)->constTable), (ConstId)))

#define VIR_Shader_GetSymNameString(Shader, Sym)    (VIR_Shader_GetStringFromId((Shader), (Sym)->u1.name))
#define VIR_Shader_GetSymNameStringFromId(Shader, SymId) \
    VIR_Shader_GetSymNameString((Shader), VIR_Shader_GetSymFromId((Shader), (SymId)))

#define VIR_Shader_GetTypeNameString(Shader, Type)  (VIR_Shader_GetStringFromId((Shader), (Type)->u1.nameId))
#define VIR_Shader_GetAttributes(Shader)            (&((Shader)->attributes))
#define VIR_Shader_GetPerpatchAttributes(Shader)    (&((Shader)->perpatchInput))
#define VIR_Shader_GetOutputs(Shader)               (&((Shader)->outputs))
#define VIR_Shader_GetPerpatchOutputs(Shader)       (&((Shader)->perpatchOutput))
#define VIR_Shader_GetOutputVregs(Shader)           (&((Shader)->outputVregs))
#define VIR_Shader_GetPerpatchOutputVregs(Shader)   (&((Shader)->perpatchOutputVregs))
#define VIR_Shader_GetVaribles(Shader)              (&((Shader)->variables))
#define VIR_Shader_GetUniforms(Shader)              (&((Shader)->uniforms))
#define VIR_Shader_GetUniformCount(Shader)          ((Shader)->uniformCount)
#define VIR_Shader_GetUniformSym(Shader, Uniform)   VIR_Shader_GetSymFromId(Shader, (Uniform)->sym)
#define VIR_Shader_GetUniformType(Shader, Uniform)  VIR_Symbol_GetType(VIR_Shader_GetUniformSym(Shader, Uniform))
#define VIR_Shader_GetUniformTypeID(Shader, Uniform)  VIR_Type_GetIndex(VIR_Shader_GetUniformType(Shader, Uniform))
#define VIR_Shader_GetUniformBlocks(Shader)         (&((Shader)->uniformBlocks))
#define VIR_Shader_GetIOBlocks(Shader)              (&((Shader)->ioBlocks))
#define VIR_Shader_GetFunctions(Shader)             ((VIR_FunctionList*)&((Shader)->functions))
#define VIR_Shader_GetCurrentFunction(Shader)       ((Shader)->currentFunction)
#define VIR_Shader_GetMainFunction(Shader)          ((Shader)->mainFunction)

#define VIR_Shader_SetCurrentFunction(Shader, func) do { (Shader)->currentFunction = (func); } while (0)
#define VIR_Shader_SetId(Shader, id)                do { (Shader)->_id = (id); } while(0)

#define VIR_Shader_isRegAllocated(Shader)           ((Shader)->hwRegAllocated)
#define VIR_Shader_SetRegAllocated(Shader, Val)     do { (Shader)->hwRegAllocated = (Val); } while (0)
#define VIR_Shader_isConstRegAllocated(Shader)      ((Shader)->constRegAllocated)
#define VIR_Shader_SetConstRegAllocated(Shader, Val) do { (Shader)->constRegAllocated = (Val); } while (0)
#define VIR_Shader_isRAEnabled(Shader)              ((Shader)->RAEnabled)
#define VIR_Shader_SetRAEnabled(Shader, Val)        do { (Shader)->RAEnabled = (Val); } while (0)
#define VIR_Shader_GetRegWatermark(Shader)          ((Shader)->hwRegWatermark)
#define VIR_Shader_SetRegWatermark(Shader, Val)     do { (Shader)->hwRegWatermark = (Val); } while (0)
#define VIR_Shader_GetLevel(Shader)                 ((Shader)->shLevel)
#define VIR_Shader_SetLevel(Shader, level)          do { (Shader)->shLevel = (level); } while (0)
#define VIR_Shader_GetOrgRegCount(Shader)           ((Shader)->_orgTempCount)
#define VIR_Shader_SetOrgRegCount(Shader, count)    do { (Shader)->_orgTempCount = (count); } while (0)

#define VIR_Shader_isDual16Mode(Shader)             ((Shader)->__IsDual16Shader)
#define VIR_Shader_SetDual16Mode(Shader, Val)       do { (Shader)->__IsDual16Shader = (Val); } while (0)

#define VIR_Shader_GetSamplerBaseOffset(Shader)     ((Shader)->samplerBaseOffset)
#define VIR_Shader_SetSamplerBaseOffset(Shader, V)  do { (Shader)->samplerBaseOffset = (V); } while (0)

#define VIR_Shader_GetBaseSamplerId(Shader)         ((Shader)->baseSamplerId)
#define VIR_Shader_SetBaseSamplerId(Shader, V)      do { (Shader)->baseSamplerId = (V); } while (0)

#define VIR_Shader_isPackUnifiedSampler(Shader)     ((Shader)->packUnifiedSampler)
#define VIR_Shader_SetPackUnifiedSampler(Shader, V) do { (Shader)->packUnifiedSampler = (V); } while (0)

#define VIR_Shader_NeedToAjustSamplerPhysical(Shader)       ((Shader)->needToAdjustSamplerPhysical)
#define VIR_Shader_SetNeedToAjustSamplerPhysical(Shader, V) do { (Shader)->needToAdjustSamplerPhysical = (V); } while (0)

#define VIR_Shader_SetComputeShaderLayout(s, ws_x, ws_y, ws_z)              \
        do {                                                                \
             gcmASSERT(VIR_Shader_GetKind(s) == VIR_SHADER_COMPUTE);        \
             (s)->shaderLayout.compute.workGroupSize[0] = (ws_x);           \
             (s)->shaderLayout.compute.workGroupSize[1] = (ws_y);           \
             (s)->shaderLayout.compute.workGroupSize[2] = (ws_z);           \
        } while(0)

#define VIR_Shader_SetTCShaderLayout(s, outVertices, inVertices)            \
        do {                                                                \
             gcmASSERT(VIR_Shader_GetKind(s) == VIR_SHADER_TESSELLATION_CONTROL);  \
             (s)->shaderLayout.tcs.tcsPatchOutputVertices = (outVertices);  \
             (s)->shaderLayout.tcs.tcsInputVerticesUniform = (inVertices);  \
        } while(0)

#define VIR_Shader_SetTEShaderLayout(s, pMode, vSpacing, order, \
                                     pointMode, inVertices)                 \
        do {                                                                \
             gcmASSERT(VIR_Shader_GetKind(s) == VIR_SHADER_TESSELLATION_EVALUATION);  \
             (s)->shaderLayout.tes.tessPrimitiveMode      = (pMode);        \
             (s)->shaderLayout.tes.tessVertexSpacing      = (vSpacing);     \
             (s)->shaderLayout.tes.tessOrdering           = (order);        \
             (s)->shaderLayout.tes.tessPointMode          = (pointMode);    \
             (s)->shaderLayout.tes.tessPatchInputVertices = (inVertices);   \
        } while(0)

#define VIR_Shader_SetGeoShaderLayout(s, invoc, maxVertices, inPrimitive, outPrimitive) \
        do {                                                                            \
             gcmASSERT(VIR_Shader_GetKind(s) == VIR_SHADER_GEOMETRY);                   \
             (s)->shaderLayout.geo.geoInvocations = (invoc);                            \
             (s)->shaderLayout.geo.geoMaxVertices = (maxVertices);                      \
             (s)->shaderLayout.geo.geoInPrimitive = (inPrimitive);                      \
             (s)->shaderLayout.geo.geoOutPrimitive = (outPrimitive);                    \
        } while(0)

#define VIR_Shader_IsGPipe(S)                                                   \
        (VIR_Shader_GetKind(S) == VIR_SHADER_VERTEX                     ||      \
         VIR_Shader_GetKind(S) == VIR_SHADER_TESSELLATION_CONTROL       ||      \
         VIR_Shader_GetKind(S) == VIR_SHADER_TESSELLATION_EVALUATION    ||      \
         VIR_Shader_GetKind(S) == VIR_SHADER_GEOMETRY)

#define VIR_GetOperandFromId(Func, Id)              ((VIR_Operand *)VIR_GetEntryFromId(&((Func)->operandTable), (Id)))
#define VIR_GetFuncSymFromId(Func, SymId)           (VIR_GetSymFromId(&(Func)->symTable, (SymId)))
#define VIR_GetLabelFromId(Func, Id)                ((VIR_Label *)VIR_GetEntryFromId(&((Func)->labelTable), (Id)))

#define VIR_Type_isPrimitive(Ty)            ((gctUINT)(Ty)->_tyIndex <= VIR_TYPE_LAST_PRIMITIVETYPE)

#define VIR_Type_GetBaseTypeId(Ty)            ((Ty)->_base)
#define VIR_Type_GetKind(Ty)                ((VIR_TypeKind)(Ty)->_kind)
#define VIR_Type_GetFlags(Ty)               ((VIR_TyFlag)(Ty)->_flags)
#define VIR_Type_GetIndex(Ty)               ((Ty)->_tyIndex)
#define VIR_Type_GetAlignement(Ty)          (0x01 << (Ty)->_alignment)
#define VIR_Type_GetQualifier(Ty)           ((VIR_TyQualifier)(Ty)->_qualifier)
#define VIR_Type_GetAddrSpace(Ty)           ((VIR_AddrSpace )(Ty)->_addrSpace)

#define VIR_Type_GetSymId(Ty)               ((Ty)->u1.symId)
#define VIR_Type_GetNameId(Ty)              ((Ty)->u1.nameId)
#define VIR_Type_GetSize(Ty)                ((Ty)->u2.size)
#define VIR_Type_GetArrayLength(Ty)         ((Ty)->u2.arrayLength)
#define VIR_Type_GetFields(Ty)              ((Ty)->u2.fields)
#define VIR_Type_GetParameters(Ty)          ((Ty)->u2.params)
#define VIR_Type_isScalar(Ty)               (VIR_Type_GetKind(Ty) == VIR_TY_SCALAR)
#define VIR_Type_isVector(Ty)               (VIR_Type_GetKind(Ty) == VIR_TY_VECTOR)
#define VIR_Type_isMatrix(Ty)               (VIR_Type_GetKind(Ty) == VIR_TY_MATRIX)
#define VIR_Type_isStruct(Ty)               (VIR_Type_GetKind(Ty) == VIR_TY_STRUCT)
#define VIR_Type_isArray(Ty)                (VIR_Type_GetKind(Ty) == VIR_TY_ARRAY)
#define VIR_Type_isPointer(Ty)              (VIR_Type_GetKind(Ty) == VIR_TY_POINTER)
#define VIR_Type_isFunction(Ty)             (VIR_Type_GetKind(Ty) == VIR_TY_FUNCTION)

#define VIR_TypeId_isPrimitive(Id)          ((gctUINT)Id <= VIR_TYPE_LAST_PRIMITIVETYPE)
#define VIR_TypeId_isSampler(Id)            ((gctUINT)Id >= VIR_TYPE_Min_SAMPLER_TYID &&    \
                                             (gctUINT)Id <= VIR_TYPE_Max_SAMPLER_TYID)
#define VIR_TypeId_isImage(Id)              ((gctUINT)Id >= VIR_TYPE_Min_IMAGE_TYID &&    \
                                             (gctUINT)Id <= VIR_TYPE_Max_IMAGE_TYID)
#define VIR_TypeId_is2DImage(Id)            ((gctUINT)Id == VIR_TYPE_IMAGE_2D ||    \
                                             (gctUINT)Id == VIR_TYPE_IIMAGE_2D ||    \
                                             (gctUINT)Id == VIR_TYPE_UIMAGE_2D)
#define VIR_TypeId_isSamplerOrImage(Id)     (VIR_TypeId_isSampler(Id) || VIR_TypeId_isImage(Id))

#define VIR_TypeId_isSamplerBuffer(Id)      ((gctUINT)Id >= VIR_TYPE_SAMPLER_BUFFER &&    \
                                             (gctUINT)Id <= VIR_TYPE_USAMPLER_BUFFER)
#define VIR_TypeId_isImageBuffer(Id)        ((gctUINT)Id >= VIR_TYPE_IMAGE_BUFFER &&    \
                                             (gctUINT)Id <= VIR_TYPE_UIMAGE_BUFFER)
#define VIR_TypeId_isBuffer(Id)             (VIR_TypeId_isSamplerBuffer(Id) || VIR_TypeId_isImageBuffer(Id))
#define VIR_TypeId_is3DImage(Id)            (!VIR_TypeId_is2DImage(Id) && !VIR_TypeId_isImageBuffer(Id))

#define VIR_Symbol_isVariable(Sym)   ((Sym)->_kind == VIR_SYM_VARIABLE)
#define VIR_Symbol_isSBO(Sym)        ((Sym)->_kind == VIR_SYM_SBO)
#define VIR_Symbol_isVreg(Sym)       ((Sym)->_kind == VIR_SYM_VIRREG)
#define VIR_Symbol_isUniform(Sym)    ((Sym)->_kind == VIR_SYM_UNIFORM)
#define VIR_Symbol_isUBO(Sym)        ((Sym)->_kind == VIR_SYM_UBO)
#define VIR_Symbol_isIOB(Sym)        ((Sym)->_kind == VIR_SYM_IOBLOCK)
#define VIR_Symbol_isSampler(Sym)    ((Sym)->_kind == VIR_SYM_SAMPLER)
#define VIR_Symbol_isTexure(Sym)     ((Sym)->_kind == VIR_SYM_TEXTURE)
#define VIR_Symbol_isImage(Sym)      ((Sym)->_kind == VIR_SYM_IMAGE)
#define VIR_Symbol_isFunction(Sym)   ((Sym)->_kind == VIR_SYM_FUNCTION)
#define VIR_Symbol_isField(Sym)      ((Sym)->_kind == VIR_SYM_FIELD)
#define VIR_Symbol_isTypedef(Sym)    ((Sym)->_kind == VIR_SYM_TYPE)
#define VIR_Symbol_isLabel(Sym)      ((Sym)->_kind == VIR_SYM_LABEL)
#define VIR_Symbol_isConst(Sym)      ((Sym)->_kind == VIR_SYM_CONST)

#define VIR_Symbol_isAttribute(Sym)  ((VIR_Symbol_isVariable(Sym) ||      \
                                       VIR_Symbol_isField(Sym))        && \
                                       ((Sym)->_storageClass == VIR_STORAGE_INPUT || \
                                        (Sym)->_storageClass == VIR_STORAGE_INOUTPUT) )
#define VIR_Symbol_isInput(Sym)      ((Sym) && VIR_Symbol_isAttribute(Sym))
#define VIR_Symbol_isOutput(Sym)     ((Sym) && (VIR_Symbol_isVariable(Sym) ||           \
                                                VIR_Symbol_isField(Sym)      )  &&      \
                                       ((Sym)->_storageClass == VIR_STORAGE_OUTPUT||    \
                                        (Sym)->_storageClass == VIR_STORAGE_INOUTPUT))

#define VIR_Symbol_isPerPatchInput(Sym)   ((VIR_Symbol_isVariable(Sym) ||      \
                                            VIR_Symbol_isField(Sym))        && \
                                            ((Sym)->_storageClass == VIR_STORAGE_PERPATCH_INPUT) )
#define VIR_Symbol_isPerPatchOutput(Sym)  ((Sym) && (VIR_Symbol_isVariable(Sym) ||           \
                                                     VIR_Symbol_isField(Sym)      )  &&      \
                                           ((Sym)->_storageClass == VIR_STORAGE_PERPATCH_OUTPUT))
#define VIR_Symbol_isPerPatch(Sym)       ((Sym) && (VIR_Symbol_isVariable(Sym) ||           \
                                                     VIR_Symbol_isField(Sym)      )  &&      \
                                           ((Sym)->_storageClass == VIR_STORAGE_PERPATCH_OUTPUT || \
                                            (Sym)->_storageClass == VIR_STORAGE_PERPATCH_INPUT))
#define VIR_Symbol_isInputOrOutput(sym) (VIR_Symbol_isAttribute((sym)) || \
                                         VIR_Symbol_isInput((sym)) || \
                                         VIR_Symbol_isOutput((sym)) || \
                                         VIR_Symbol_isPerPatchInput((sym)) || \
                                         VIR_Symbol_isPerPatchOutput((sym)) || \
                                         VIR_Symbol_isPerPatch((sym)))

#define VIR_Symbol_isInParam(Sym)    ((Sym) && (VIR_Symbol_isVariable(Sym) &&           \
                                       ((Sym)->_storageClass == VIR_STORAGE_INPARM||    \
                                        (Sym)->_storageClass == VIR_STORAGE_INOUTPARM))
#define VIR_Symbol_isOutParam(Sym)   ((Sym) && (VIR_Symbol_isVariable(Sym) &&           \
                                       ((Sym)->_storageClass == VIR_STORAGE_OUTPARM||   \
                                        (Sym)->_storageClass == VIR_STORAGE_INOUTPARM)))

#define VIR_Symbol_GetKind(Sym)         ((Sym)->_kind)
#define VIR_Symbol_GetStorageClass(Sym) ((VIR_StorageClass)(Sym)->_storageClass)
#define VIR_Symbol_GetAddrSpace(Sym)    ((VIR_AddrSpace)(Sym)->_addrSpace)
#define VIR_Symbol_GetPrecision(Sym)    ((VIR_Precision)(Sym)->_precision)
#define VIR_Symbol_GetTyQualifier(Sym)  ((VIR_TyQualifier)(Sym)->_qualifier)
#define VIR_Symbol_GetHwRegId(Sym)      ((VIR_HwRegId)(Sym)->_hwRegId)
#define VIR_Symbol_GetHwShift(Sym)      ((Sym)->_hwShift)
#define VIR_Symbol_GetHIHwRegId(Sym)    ((VIR_HwRegId)(Sym)->_HIhwRegId)
#define VIR_Symbol_GetHIHwShift(Sym)    ((Sym)->_HIhwShift)
#define VIR_Symbol_GetType(Sym)         ((Sym)->type)
#define VIR_Symbol_GetFlag(Sym)         ((Sym)->flags)
#define VIR_Symbol_HasFlag(Sym, f)      (((Sym)->flags & (f)) != 0)
#define VIR_Symbol_GetIndex(Sym)        ((Sym)->index)
#define VIR_Symbol_GetIOBlockIndex(Sym) ((Sym)->ioBlockIndex)
#define VIR_Symbol_GetVariableVregIndex(Sym)        ((Sym)->u2.tempIndex)
#define VIR_Symbol_GetName(Sym)         ((Sym)->u1.name)
#define VIR_Symbol_GetConstId(Sym)      (VIR_Symbol_isConst(Sym) ? (Sym)->u1.constId : VIR_INVALID_ID)
#define VIR_Symbol_GetUniformStructIndex(Sym, Val)  ((Sym)->u2.uniformIndex)
#define VIR_Symbol_GetVregVariable(Sym) ((Sym)->u2.variable)
#define VIR_Symbol_GetFieldInfo(Sym)    ((Sym)->u2.fieldInfo)
#define VIR_Symbol_GetFieldVreg(Sym)    (VIR_Symbol_GetFieldInfo(Sym)->tempRegOrUniformOffset)
#define VIR_Symbol_GetVregIndex(Sym)    (VIR_Symbol_isVreg(Sym)      ? (Sym)->u1.vregIndex :                  \
                                         (VIR_Symbol_isVariable(Sym) ? VIR_Symbol_GetVariableVregIndex(Sym) : \
                                         (VIR_Symbol_isField(Sym)    ? VIR_Symbol_GetFieldVreg(Sym) :         \
                                                                       VIR_INVALID_ID)))
#define VIR_Symbol_GetFunction(Sym)     (VIR_Symbol_isFunction(Sym) ?      \
                                            (Sym)->u2.function : gcvNULL)
#define VIR_Symbol_GetUniform(Sym)      (VIR_Symbol_isUniform(Sym) ?      \
                                            (Sym)->u2.uniform : gcvNULL)
#define VIR_Symbol_GetSampler(Sym)      (VIR_Symbol_isSampler(Sym) ?      \
                                            (Sym)->u2.sampler : gcvNULL)
#define VIR_Symbol_GetImage(Sym)        (VIR_Symbol_isImage(Sym) ?      \
                                            (Sym)->u2.image : gcvNULL)
#define VIR_Symbol_GetUBO(Sym)          (VIR_Symbol_isUBO(Sym) ? (Sym)->u2.ubo : gcvNULL)
#define VIR_Symbol_GetSBO(Sym)          (VIR_Symbol_isSBO(Sym) ? (Sym)->u2.sbo : gcvNULL)
#define VIR_Symbol_GetIOB(Sym)          (VIR_Symbol_isIOB(Sym) ? (Sym)->u2.ioBlock : gcvNULL)
#define VIR_Symbol_GetFuncMangleName(Sym)   ((VIR_Symbol_isFunction(Sym) ?    \
                                             (Sym)->u3.mangledName) : VIR_INVALID_ID)
#define VIR_Symbol_GetStructType(Sym)       (VIR_Symbol_isField(Sym) ? (Sym)->u3.structType : gcvNULL)
#define VIR_Symbol_GetLayout(Sym)           ((Sym)->layout)
#define VIR_Symbol_GetLayoutQualifier(Sym)  ((Sym)->layout.layoutQualifier)
#define VIR_Symbol_GetLocation(Sym)         ((Sym)->layout.location)
#define VIR_Symbol_GetBinding(Sym)          ((Sym)->layout.binding)
#define VIR_Symbol_GetLayoutOffset(Sym)     ((Sym)->layout.offset)
#define VIR_Symbol_GetArraySlot(Sym)        ((Sym)->layout.llArraySlot)
#define VIR_Symbol_GetFirstSlot(Sym)        ((Sym)->layout.llFirstSlot)
#define VIR_Symbol_GetHwFirstCompIndex(Sym) ((Sym)->layout.hwFirstCompIndex)
#define VIR_Symbol_GetOffsetInVar(Sym)      ((Sym)->u3.offsetInVar)
#define VIR_Symbol_GetIndexRange(Sym)       ((Sym)->indexRange)

#define VIR_Symbol_GetSize(Sym)         (VIR_Symbol_isStruct(Sym) ? (Sym)->u3.size : 0)
#define VIR_Symbol_GetUniformKind(Sym)  ((VIR_UniformKind)((Sym)->_storageClass))

#define VIR_Symbol_GetOffsetTempIndex(Sym, Offset)  (VIR_Symbol_GetVregIndex(Sym) != VIR_INVALID_ID ?  \
                                                     VIR_Symbol_GetVregIndex(Sym) + (Offset) : VIR_INVALID_ID)

#define VIR_Symbol_SetLayoutQualifier(Sym, Qual)        \
    do {(Sym)->layout.layoutQualifier = (Qual); } while(0)
#define VIR_Symbol_SetLocation(Sym, Val)                \
    do {(Sym)->layout.location = (Val); } while(0)
#define VIR_Symbol_SetBinding(Sym, Val)                 \
    do {(Sym)->layout.binding = (Val); } while(0)
#define VIR_Symbol_SetLayoutOffset(Sym, Val)                  \
    do {(Sym)->layout.offset = (Val); } while(0)
#define VIR_Symbol_SetFirstSlot(Sym, Slot)              \
    do {(Sym)->layout.llFirstSlot = (Slot); } while(0)
#define VIR_Symbol_SetHwFirstCompIndex(Sym, Slot)              \
    do {(Sym)->layout.hwFirstCompIndex = (Slot); } while(0)
#define VIR_Symbol_SetArraySlot(Sym, Slot)              \
    do {(Sym)->layout.llArraySlot = (Slot); } while(0)

#define VIR_Symbol_SetKind(Sym, Kind)       do {(Sym)->_kind = Kind; } while (0)
#define VIR_Symbol_SetStorageClass(Sym, SC) do {(Sym)->_storageClass = SC; } while (0)
#define VIR_Symbol_SetAddrSpace(Sym, AS)    do {(Sym)->_addrSpace = (gctUINT)AS; } while (0)
#define VIR_Symbol_SetPrecision(Sym, Val)   do {(Sym)->_precision = (gctUINT)Val; } while (0)
#define VIR_Symbol_SetHwRegId(Sym, Val)     do {(Sym)->_hwRegId = (gctUINT)(Val); } while (0)
#define VIR_Symbol_SetHwShift(Sym, Val)     do {(Sym)->_hwShift = (gctUINT)(Val); } while (0)
#define VIR_Symbol_SetHIHwRegId(Sym, Val)   do {(Sym)->_HIhwRegId = (gctUINT)(Val); } while (0)
#define VIR_Symbol_SetHIHwShift(Sym, Val)   do {(Sym)->_HIhwShift = (gctUINT)(Val); } while (0)
#define VIR_Symbol_SetType(Sym, Val)        do {(Sym)->type = (Val); } while (0)
#define VIR_Symbol_SetFlag(Sym, Val)        do {(Sym)->flags |= (Val); } while (0)
#define VIR_Symbol_ClrFlag(Sym, Val)        do {(Sym)->flags &= ~(Val); } while (0)
#define VIR_Symbol_SetIndex(Sym, Val)       do {(Sym)->index = (Val); } while (0)
#define VIR_Symbol_SetVirRegVariable(Sym, Var)  do { (Sym)->u2.variable = (Var); } while (0)
#define VIR_Symbol_SetTyQualifier(Sym, Val)     do {(Sym)->_qualifier = (gctUINT)(Val); } while (0)
#define VIR_Symbol_SetVregIndex(Sym, Val)       do {(Sym)->u1.vregIndex = (Val); } while (0)
#define VIR_Symbol_SetSymbolName(Sym, Val)      do {(Sym)->u1.name = (Val); } while (0)
#define VIR_Symbol_SetConstId(Sym, Val)         do {(Sym)->u1.constId = (Val); } while (0)
#define VIR_Symbol_SetVariableVregIndex(Sym, Val) do {(Sym)->u2.tempIndex = (Val); } while (0)
#define VIR_Symbol_SetUniformStructIndex(Sym, Val) do {(Sym)->u2.uniformIndex = (Val); } while (0)
#define VIR_Symbol_SetVregVariable(Sym, Val)    do {(Sym)->u2.variable = (Val); } while (0)
#define VIR_Symbol_SetFieldInfo(Sym, Val)       do {(Sym)->u2.fieldInfo = (Val); } while (0)
#define VIR_Symbol_SetFunction(Sym, Val)        do {(Sym)->u2.function = (Val); } while (0)
#define VIR_Symbol_SetUniform(Sym, Val)         do {(Sym)->u2.uniform = (Val); } while (0)
#define VIR_Symbol_SetSampler(Sym, Val)         do {(Sym)->u2.uniform = (Val); } while (0)
#define VIR_Symbol_SetUBO(Sym, Val)             do {(Sym)->u2.ubo = (Val); } while (0)
#define VIR_Symbol_SetSBO(Sym, Val)             do {(Sym)->u2.sbo = (Val); } while (0)
#define VIR_Symbol_SetFuncMangleName(Sym, Val)  do {(Sym)->u3.mangledName = (Val); } while (0)
#define VIR_Symbol_SetStructType(Sym, Val)      do {(Sym)->u3.structType = (Val); } while (0)
#define VIR_Symbol_SetLayout(Sym, Val)          do {(Sym)->layout = (Val); } while (0)
#define VIR_Symbol_SetSize(Sym, Val)            do {(Sym)->u3.size = (Val); } while (0)
#define VIR_Symbol_SetOffsetInVar(Sym, Val)     do {(Sym)->u3.offsetInVar = (Val); } while (0)
#define VIR_Symbol_SetIndexRange(Sym, Val)      do {(Sym)->indexRange = (Val); } while (0)

#define VIR_Symbol_SetUniformKind(Sym, Val)     do {(Sym)->_storageClass = (Val); } while (0)

#define VIR_Label_GetId(Label)          ((Label)->index)
#define VIR_Label_GetSymId(Label)       ((Label)->sym)
#define VIR_Label_GetDefInst(Label)     ((Label)->defined)

#define VIR_OpndInfo_Is_Virtual_Reg(OpndInfo)   ((!(OpndInfo)->isImmVal) && \
                                                 (!(OpndInfo)->isVecConst) && \
                                                 (!(OpndInfo)->isUniform) && \
                                                 ((OpndInfo)->u1.virRegInfo.virReg != VIR_INVALID_ID))

#define VIR_OpndInfo_Is_Output(OpndInfo)        ((OpndInfo)->isOutput)

#define VIR_Uniform_GetSymID(Uniform)                   ((Uniform)->sym)
#define VIR_Uniform_GetID(Uniform)                      ((Uniform)->index)
#define VIR_Uniform_GetBlockIndex(Uniform)              ((Uniform)->blockIndex)
#define VIR_Uniform_GetGLUniformIndex(Uniform)          ((Uniform)->glUniformIndex)
#define VIR_Uniform_GetVarCategory(Uniform)             ((Uniform)->_varCategory)
#define VIR_Uniform_SetVarCategory(Uniform, vc)         ((Uniform)->_varCategory = (vc))
#define VIR_Uniform_GetImageSamplerIndex(Uniform)       ((Uniform)->imageSamplerIndex)
#define VIR_Uniform_GetMatchIndex(Uniform)              ((Uniform)->matchIndex)
#define VIR_Uniform_GetPhysical(Uniform)                ((Uniform)->physical)
#define VIR_Uniform_SetPhysical(Uniform, Val)           (VIR_Uniform_GetPhysical(Uniform) = (Val))
#define VIR_Uniform_GetOrigPhysical(Uniform)            ((Uniform)->origPhysical)
#define VIR_Uniform_SetOrigPhysical(Uniform, Val)       (VIR_Uniform_GetOrigPhysical(Uniform) = (Val))
#define VIR_Uniform_GetSwizzle(Uniform)                 ((Uniform)->swizzle)
#define VIR_Uniform_GetAddress(Uniform)                 ((Uniform)->address)
#define VIR_Uniform_GetOffset(Uniform)                  ((Uniform)->offset)
#define VIR_Uniform_SetOffset(Uniform, offset)          ((Uniform)->offset = offset)
#define VIR_Uniform_GetInitializer(Uniform)             ((Uniform)->u.initializer)

#define VIR_UniformBlock_GetBlockSize(UB)               ((UB)->blockSize)

typedef enum _VIR_PRIMITIVETYPEID
{
    VIR_TYPE_UNKNOWN,
    VIR_TYPE_VOID,
    /* scalar types */
    /* types can be mapped to equivalent machine type directly */
    VIR_TYPE_FLOAT32, /* float, VIR_TYPE_FLOAT_X1 */
    VIR_TYPE_FLOAT16, /* half float */
    VIR_TYPE_INT32, /* signed 32bit int, VIR_TYPE_INT_X1 */
    VIR_TYPE_INT16, /* short */
    VIR_TYPE_INT8, /* signed char */
    VIR_TYPE_UINT32, /* unsigned 32bit int, VIR_TYPE_UINT_X1 */
    VIR_TYPE_UINT16, /* unsigned short */
    VIR_TYPE_UINT8, /* unsigned char, uchar */
    VIR_TYPE_SNORM16, /* 16 bit normalized signed integer */
    VIR_TYPE_SNORM8, /* 8 bit normalized signed integer */
    VIR_TYPE_UNORM16, /* 16 bit normalized unsigned integer */
    VIR_TYPE_UNORM8, /* 8 bit normalized unsigned integer */

    /* scalar types not supported by HW */
    VIR_TYPE_INT64, /* long, 64 bit signed integer */
    VIR_TYPE_UINT64, /* ulong, 64 bit unsigned integer */
    VIR_TYPE_FLOAT64, /* double */
    VIR_TYPE_BOOLEAN, /* bool, VIR_TYPE_BOOLEAN_X1 */

    /* vector types */
    /* openCL support vector 8, 16 for all scalar types: int16, int8, etc */
    VIR_TYPE_FLOAT_X2, /* float2, vec2 */
    VIR_TYPE_FLOAT_X3, /* float3, vec3 */
    VIR_TYPE_FLOAT_X4, /* float4, vec4 */
    VIR_TYPE_FLOAT_X8, /* float8 */
    VIR_TYPE_FLOAT_X16, /* float16 */
    VIR_TYPE_FLOAT_X32, /* float32 */

    VIR_TYPE_FLOAT16_X2, /* half2 */
    VIR_TYPE_FLOAT16_X3, /* half3 */
    VIR_TYPE_FLOAT16_X4, /* half4 */
    VIR_TYPE_FLOAT16_X8, /* half8 */
    VIR_TYPE_FLOAT16_X16, /* half16 */
    VIR_TYPE_FLOAT16_X32, /* half32 */

    VIR_TYPE_BOOLEAN_X2, /* bool2, bvec2 */
    VIR_TYPE_BOOLEAN_X3,
    VIR_TYPE_BOOLEAN_X4,
    VIR_TYPE_BOOLEAN_X8,
    VIR_TYPE_BOOLEAN_X16,
    VIR_TYPE_BOOLEAN_X32,

    VIR_TYPE_INTEGER_X2,
    VIR_TYPE_INTEGER_X3,
    VIR_TYPE_INTEGER_X4,
    VIR_TYPE_INTEGER_X8,
    VIR_TYPE_INTEGER_X16,
    VIR_TYPE_INTEGER_X32,

    VIR_TYPE_UINT_X2,
    VIR_TYPE_UINT_X3,
    VIR_TYPE_UINT_X4,
    VIR_TYPE_UINT_X8,
    VIR_TYPE_UINT_X16,
    VIR_TYPE_UINT_X32,

    /* uchar vectors */
    VIR_TYPE_UINT8_X2,
    VIR_TYPE_UINT8_X3,
    VIR_TYPE_UINT8_X4,
    VIR_TYPE_UINT8_X8,
    VIR_TYPE_UINT8_X16,
    VIR_TYPE_UINT8_X32,

    /* char vectors */
    VIR_TYPE_INT8_X2,
    VIR_TYPE_INT8_X3,
    VIR_TYPE_INT8_X4,
    VIR_TYPE_INT8_X8,
    VIR_TYPE_INT8_X16,
    VIR_TYPE_INT8_X32,

    /* ushort vectors */
    VIR_TYPE_UINT16_X2,
    VIR_TYPE_UINT16_X3,
    VIR_TYPE_UINT16_X4,
    VIR_TYPE_UINT16_X8,
    VIR_TYPE_UINT16_X16,
    VIR_TYPE_UINT16_X32,

    /* short vectors */
    VIR_TYPE_INT16_X2,
    VIR_TYPE_INT16_X3,
    VIR_TYPE_INT16_X4,
    VIR_TYPE_INT16_X8,
    VIR_TYPE_INT16_X16,
    VIR_TYPE_INT16_X32,

    /* uint64 vectors */
    VIR_TYPE_UINT64_X2,
    VIR_TYPE_UINT64_X3,
    VIR_TYPE_UINT64_X4,
    VIR_TYPE_UINT64_X8,
    VIR_TYPE_UINT64_X16,
    VIR_TYPE_UINT64_X32,

    /* int64 vectors */
    VIR_TYPE_INT64_X2,
    VIR_TYPE_INT64_X3,
    VIR_TYPE_INT64_X4,
    VIR_TYPE_INT64_X8,
    VIR_TYPE_INT64_X16,
    VIR_TYPE_INT64_X32,

    /* packed data type */

    /* packed float16 (2 bytes per element) */
    VIR_TYPE_FLOAT16_P2, /* half2 */
    VIR_TYPE_FLOAT16_P3, /* half3 */
    VIR_TYPE_FLOAT16_P4, /* half4 */
    VIR_TYPE_FLOAT16_P8, /* half8 */
    VIR_TYPE_FLOAT16_P16, /* half16 */
    VIR_TYPE_FLOAT16_P32, /* half32 */

    /* packed boolean (1 byte per element) */
    VIR_TYPE_BOOLEAN_P2, /* bool2, bvec2 */
    VIR_TYPE_BOOLEAN_P3,
    VIR_TYPE_BOOLEAN_P4,
    VIR_TYPE_BOOLEAN_P8,
    VIR_TYPE_BOOLEAN_P16,
    VIR_TYPE_BOOLEAN_P32,

    /* uchar vectors (1 byte per element) */
    VIR_TYPE_UINT8_P2,
    VIR_TYPE_UINT8_P3,
    VIR_TYPE_UINT8_P4,
    VIR_TYPE_UINT8_P8,
    VIR_TYPE_UINT8_P16,
    VIR_TYPE_UINT8_P32,

    /* char vectors (1 byte per element) */
    VIR_TYPE_INT8_P2,
    VIR_TYPE_INT8_P3,
    VIR_TYPE_INT8_P4,
    VIR_TYPE_INT8_P8,
    VIR_TYPE_INT8_P16,
    VIR_TYPE_INT8_P32,

    /* ushort vectors (2 bytes per element) */
    VIR_TYPE_UINT16_P2,
    VIR_TYPE_UINT16_P3,
    VIR_TYPE_UINT16_P4,
    VIR_TYPE_UINT16_P8,
    VIR_TYPE_UINT16_P16,
    VIR_TYPE_UINT16_P32,

    /* short vectors (2 bytes per element) */
    VIR_TYPE_INT16_P2,
    VIR_TYPE_INT16_P3,
    VIR_TYPE_INT16_P4,
    VIR_TYPE_INT16_P8,
    VIR_TYPE_INT16_P16,
    VIR_TYPE_INT16_P32,

    /* matrix type: only support float type */
    VIR_TYPE_FLOAT_2X2,
    VIR_TYPE_FLOAT_3X3,
    VIR_TYPE_FLOAT_4X4,
    VIR_TYPE_FLOAT_2X3,
    VIR_TYPE_FLOAT_2X4,
    VIR_TYPE_FLOAT_3X2,
    VIR_TYPE_FLOAT_3X4,
    VIR_TYPE_FLOAT_4X2,
    VIR_TYPE_FLOAT_4X3,

    /* sampler type */
    VIR_TYPE_Min_SAMPLER_TYID,
    VIR_TYPE_SAMPLER_GENERIC = VIR_TYPE_Min_SAMPLER_TYID, /* generic sampler type */
    VIR_TYPE_SAMPLER_1D,
    VIR_TYPE_SAMPLER_2D,
    VIR_TYPE_SAMPLER_3D,
    VIR_TYPE_SAMPLER_CUBIC,
    VIR_TYPE_SAMPLER_CUBE_ARRAY,
    VIR_TYPE_SAMPLER,
    VIR_TYPE_ISAMPLER_2D,
    VIR_TYPE_ISAMPLER_3D,
    VIR_TYPE_ISAMPLER_CUBIC,
    VIR_TYPE_ISAMPLER_CUBE_ARRAY,
    VIR_TYPE_USAMPLER_2D,
    VIR_TYPE_USAMPLER_3D,
    VIR_TYPE_USAMPLER_CUBIC,
    VIR_TYPE_USAMPLER_CUBE_ARRAY,
    VIR_TYPE_SAMPLER_EXTERNAL_OES,

    VIR_TYPE_SAMPLER_2D_SHADOW,
    VIR_TYPE_SAMPLER_CUBE_SHADOW,
    VIR_TYPE_SAMPLER_CUBE_ARRAY_SHADOW,

    VIR_TYPE_SAMPLER_1D_ARRAY,
    VIR_TYPE_SAMPLER_1D_ARRAY_SHADOW,
    VIR_TYPE_SAMPLER_2D_ARRAY,
    VIR_TYPE_ISAMPLER_2D_ARRAY,
    VIR_TYPE_USAMPLER_2D_ARRAY,
    VIR_TYPE_SAMPLER_2D_ARRAY_SHADOW,

    VIR_TYPE_SAMPLER_2D_MS,
    VIR_TYPE_ISAMPLER_2D_MS,
    VIR_TYPE_USAMPLER_2D_MS,
    VIR_TYPE_SAMPLER_2D_MS_ARRAY,
    VIR_TYPE_ISAMPLER_2D_MS_ARRAY,
    VIR_TYPE_USAMPLER_2D_MS_ARRAY,
    VIR_TYPE_SAMPLER_BUFFER,
    VIR_TYPE_ISAMPLER_BUFFER,
    VIR_TYPE_USAMPLER_BUFFER,
    VIR_TYPE_Max_SAMPLER_TYID = VIR_TYPE_USAMPLER_BUFFER,

    /* image type */
    VIR_TYPE_Min_IMAGE_TYID,
    VIR_TYPE_IMAGE_1D = VIR_TYPE_Min_IMAGE_TYID, /* OCL image1d_t */
    VIR_TYPE_IMAGE_1D_ARRAY, /* OCL image1d_array_t */
    VIR_TYPE_IMAGE_1D_BUFFER, /* OCL image1d_buffer_t */
    VIR_TYPE_IMAGE_2D, /* OCL image2d_t */
    VIR_TYPE_IMAGE_2D_ARRAY, /* OCL image2d_array_t */
    VIR_TYPE_IMAGE_3D, /* OCL image3d_t */
    VIR_TYPE_IMAGE_2D_ARRAY_MSSA, /* OCL image2d_array_mssa_t */
    VIR_TYPE_IMAGE_2D_MSSA_DEPTH, /* OCL image2d_mssa_depth_t */
    VIR_TYPE_IMAGE_2D_ARRAY_MSSA_DEPTH, /* OCL image2d_array_mssa_depth_t */
    VIR_TYPE_IMAGE_2D_DEPTH, /* OCL image2d_depth_t */
    VIR_TYPE_IMAGE_2D_ARRAY_DEPTH, /* OCL image2d_array_depth_t */
    VIR_TYPE_IIMAGE_2D,
    VIR_TYPE_UIMAGE_2D,
    VIR_TYPE_IIMAGE_3D,
    VIR_TYPE_UIMAGE_3D,
    VIR_TYPE_IIMAGE_2D_ARRAY,
    VIR_TYPE_UIMAGE_2D_ARRAY,
    VIR_TYPE_IMAGE_CUBE,
    VIR_TYPE_IMAGE_CUBE_ARRAY,
    VIR_TYPE_IIMAGE_CUBE,
    VIR_TYPE_IIMAGE_CUBE_ARRAY,
    VIR_TYPE_UIMAGE_CUBE,
    VIR_TYPE_UIMAGE_CUBE_ARRAY,
    VIR_TYPE_IMAGE_BUFFER,
    VIR_TYPE_IIMAGE_BUFFER,
    VIR_TYPE_UIMAGE_BUFFER,
    VIR_TYPE_Max_IMAGE_TYID = VIR_TYPE_UIMAGE_BUFFER,

    VIR_TYPE_EVENT, /* OCL event_t */
    VIR_TYPE_ATOMIC_UINT, /* atomic counter */

    VIR_TYPE_LAST_PRIMITIVETYPE = VIR_TYPE_ATOMIC_UINT,
    VIR_TYPE_PRIMITIVETYPE_COUNT, /* must to change _builtinTypes at the
                                         * same time if you add any new type! */
} VIR_PrimitiveTypeId;

typedef VIR_PrimitiveTypeId   VIR_TypeId;        /* index to type table */

#define VIR_ListResize(OldSize) ((gctUINT)((OldSize) * 1.5))

struct _VIR_IDLIST
{
    VSC_MM *        memPool;
    gctUINT         allocated;      /* allocated entries */
    gctUINT         count;          /* the number of symbols in the list */
    VIR_Id *        ids;            /* an array of ids */
} ;

struct _VIR_VALUELIST
{
    VSC_MM *        memPool;
    gctUINT         allocated;      /* allocated entries */
    gctUINT         count;          /* the number of symbols in the list */
    gctUINT         elemSize;       /* size of each value element */
    gctCHAR *       values;         /* an array of values */
} ;

/* string table operations: set string table block size to 64KB */
#define VIR_ST_BLOCKSIZE            (2<<16)   /* 64KB */

/* VIR_Id has level info in the Id, level 0 is global scope, level 1 is function scope */
#define VIR_ID_SHADER_SCOPE         0x00
#define VIR_ID_FUNCTION_SCOPE       0x01

#define VIR_INVALID_ID              INVALID_BT_ENTRY_ID
#define VIR_Id_GetIndex(Id)         ((gctUINT)(Id) & VALID_BT_ENTRY_ID_MASK)
#define VIR_Id_GetScope(Id)         (((gctUINT)(Id) & BT_ENTRY_ID_SCOPE_MASK) >> BT_ENTRY_VALID_BITS)
#define VIR_Id_isFunctionScope(Id)  (((Id) & BT_ENTRY_ID_FUNC_SCOPE))
#define VIR_Id_SetFunctionScope(Id) do { (Id) |= BT_ENTRY_ID_FUNC_SCOPE; } while (0)
#define VIR_Id_isValid(Id)          (VIR_Id_GetIndex(Id) != VIR_INVALID_ID)
#define VIR_Id_isInvalid(Id)        (VIR_Id_GetIndex(Id) == VIR_INVALID_ID)

#define VIR_GetEntryFromId(BlockTable, Id)  (BT_GET_ENTRY_DATA(BlockTable, (Id)))

extern VIR_NameId VIR_ST_Add(VIR_StringTable *, const gctSTRING);

typedef struct _VIR_FUNCTIONNODE
{
    VSC_BI_LIST_NODE    blNode;
    VIR_Function *      function;
} VIR_FunctionNode;

typedef VSC_BI_LIST VIR_FunctionList;

/* Func iterator */
typedef VSC_BL_ITERATOR VIR_FuncIterator;
#define VIR_FuncIterator_Init(Iter, funcList) vscBLIterator_Init((Iter), (VSC_BI_LIST*)(funcList))
#define VIR_FuncIterator_First(Iter)          (VIR_FunctionNode *)vscBLIterator_First((Iter))
#define VIR_FuncIterator_Next(Iter)           (VIR_FunctionNode *)vscBLIterator_Next((Iter))
#define VIR_FuncIterator_Prev(Iter)           (VIR_FunctionNode *)vscBLIterator_Prev((Iter))
#define VIR_FuncIterator_Last(Iter)           (VIR_FunctionNode *)vscBLIterator_Last((Iter))

typedef struct _VSC_INSTLIST
{
    VIR_Instruction *         pHead;
    VIR_Instruction *         pTail;

    VSC_LIST_INFO             info;
} VIR_InstList;

/* creat a new IdList struct if *IdList is null,
 * allocate id arrays with InitSize in MemPool */
VSC_ErrCode
VIR_IdList_Init(
    IN VSC_MM *         MemPool,
    IN gctUINT          InitSize,
    IN OUT VIR_IdList **IdList);

VSC_ErrCode
VIR_IdList_Add(
    IN VIR_IdList *     IdList,
    IN VIR_Id           Id);

#define VIR_IdList_Count(IdList)        ((IdList)->count)
#if !defined(_DEBUG)
#define VIR_IdList_GetId(IdList, No)    ((VIR_Id)(IdList)->ids[(No)])
#else
VIR_Id VIR_IdList_GetId(VIR_IdList *, gctUINT);
#endif
#define VIR_IdList_SetId(IdList, No, Id) do { gcmASSERT((No) < VIR_IdList_Count(IdList)); (IdList)->ids[(No)] = (Id); } while(0)

/* How to iterate IdList:
 *  for (int i = 0; i < VIR_IdList_Count(lst); i++)
 *  {
 *      VIR_Id id = VIR_IdList_GetId(lst, i);
 *  }
 */

/* creat a new IdList struct if *IdList is null,
 * allocate id arrays with InitSize in MemPool */
VSC_ErrCode
VIR_ValueList_Init(
    IN VSC_MM *             MemPool,
    IN gctUINT              InitSize,
    IN gctUINT              ElemSize, /* sizeof(typeof(value)) */
    IN OUT VIR_ValueList ** ValueList);

VSC_ErrCode
VIR_ValueList_Add(
    IN VIR_ValueList *     ValueList,
    IN gctCHAR *           Value);

#define VIR_ValueList_ElemSize(ValueList)     ((ValueList)->elemSize)
#define VIR_ValueList_Count(ValueList)        ((ValueList)->count)
#if !defined(_NDEBUG)
#define VIR_ValueList_GetValue(ValueList, No) ((gctCHAR *)((ValueList)->values +(No) * ValueList->elemSize))
#else
gctCHAR * VIR_ValueList_GetValue(VIR_ValueList *, gctUINT);
#endif
VSC_ErrCode
VIR_ValueList_SetValue(
    IN VIR_ValueList *     IdList,
    IN gctUINT             Index,
    IN gctCHAR *           Value);

/* How to iterate IdList:
 *  for (int i = 0; i < VIR_IdList_Count(lst); i++)
 *  {
 *      VIR_Id id = VIR_IdList_GetId(lst, i);
 *  }
 */

/* inherited gcSL defines */

/* inherited gcSHADER defines */
typedef enum _VIR_PRECISION
{
    VIR_PRECISION_MEDIUM, /* 0x00 */
    VIR_PRECISION_HIGH, /* 0x01 */
    VIR_PRECISION_LOW, /* 0x02 */
    VIR_PRECISION_DEFAULT, /* 0x03 */
} VIR_Precision;

/******************************************************************************\
|******************************* SHADER LANGUAGE ******************************|
\******************************************************************************/

/* Opcode conditions. */
typedef enum _VIR_ConditionOp
{
    VIR_COP_ALWAYS, /* 0x00     0x00 */
    VIR_COP_GREATER, /* 0x01       0x01 */
    VIR_COP_LESS, /* 0x02       0x02 */
    VIR_COP_GREATER_OR_EQUAL, /* 0x03       0x03 */
    VIR_COP_LESS_OR_EQUAL, /* 0x04       0x04 */
    VIR_COP_EQUAL, /* 0x05       0x05 */
    VIR_COP_NOT_EQUAL, /* 0x06       0x06 */
    VIR_COP_AND, /* 0x07      0x07 */
    VIR_COP_OR, /* 0x08       0x08 */
    VIR_COP_XOR, /* 0x09      0x09 */
    VIR_COP_NOT, /* 0x0A      0x0A */
    VIR_COP_NOT_ZERO, /* 0x0B       0x0B */
    VIR_COP_GREATER_OR_EQUAL_ZERO, /* 0x0C      0x0C */
    VIR_COP_GREATER_ZERO, /* 0x0D       0x0D */
    VIR_COP_LESS_OREQUAL_ZERO, /* 0x0E      0x0E */
    VIR_COP_LESS_ZERO, /* 0x0F       0x0F */
    VIR_COP_FINITE, /* 0x10   0x10 */
    VIR_COP_INFINITE, /* 0x11 0x11 */
    VIR_COP_NAN, /* 0x12      0x12 */
    VIR_COP_NORMAL, /* 0x13   0x13 */
    VIR_COP_ANYMSB, /* 0x14   0x14 */
    VIR_COP_ALLMSB, /* 0x15   0x15 */
    VIR_COP_SELMSB, /* 0x16   0x16 */
    VIR_COP_UCARRY, /* 0x17   0x17 */
    VIR_COP_HELPER, /* 0x18   0x18 */
    VIR_COP_NOTHELPER, /* 0x19 0x19 */
}
VIR_ConditionOp;

#define VIR_ConditionOp_ComparingWithZero(c)        ((c) == VIR_COP_NOT || \
                                                     (c) == VIR_COP_NOT_ZERO || \
                                                     (c) == VIR_COP_GREATER_OR_EQUAL_ZERO || \
                                                     (c) == VIR_COP_GREATER_ZERO || \
                                                     (c) == VIR_COP_LESS_OREQUAL_ZERO || \
                                                     (c) == VIR_COP_LESS_ZERO)

#define VIR_ConditionOp_SingleOperand(c)            (VIR_ConditionOp_ComparingWithZero(c) || \
                                                     (c) == VIR_COP_FINITE || \
                                                     (c) == VIR_COP_INFINITE || \
                                                     (c) == VIR_COP_NAN || \
                                                     (c) == VIR_COP_NORMAL || \
                                                     (c) == VIR_COP_ANYMSB || \
                                                     (c) == VIR_COP_ALLMSB || \
                                                     (c) == VIR_COP_SELMSB)

#define VIR_ConditionOp_DoubleOperand(c)            ((c) == VIR_COP_GREATER || \
                                                     (c) == VIR_COP_LESS || \
                                                     (c) == VIR_COP_GREATER_OR_EQUAL || \
                                                     (c) == VIR_COP_LESS_OR_EQUAL || \
                                                     (c) == VIR_COP_EQUAL || \
                                                     (c) == VIR_COP_NOT_EQUAL || \
                                                     (c) == VIR_COP_AND || \
                                                     (c) == VIR_COP_OR || \
                                                     (c) == VIR_COP_XOR)

#define VIR_CHANNEL_COUNT           4
/* Swizzle generator macro. */
#define virmSWIZZLE(Component1, Component2, Component3, Component4) \
    (\
    (VIR_SWIZZLE_ ## Component1 << 0) | \
    (VIR_SWIZZLE_ ## Component2 << 2) | \
    (VIR_SWIZZLE_ ## Component3 << 4) | \
    (VIR_SWIZZLE_ ## Component4 << 6)   \
    )

typedef enum _VIR_SWIZZLE
{
    VIR_SWIZZLE_X = 0, /* 0x0 */
    VIR_SWIZZLE_Y = 1, /* 0x1 */
    VIR_SWIZZLE_Z = 2, /* 0x2 */
    VIR_SWIZZLE_W = 3, /* 0x3 */
    /* Combinations. */
    VIR_SWIZZLE_XXXX = virmSWIZZLE(X, X, X, X),
    VIR_SWIZZLE_YYYY = virmSWIZZLE(Y, Y, Y, Y),
    VIR_SWIZZLE_ZZZZ = virmSWIZZLE(Z, Z, Z, Z),
    VIR_SWIZZLE_WWWW = virmSWIZZLE(W, W, W, W),
    VIR_SWIZZLE_XYYY = virmSWIZZLE(X, Y, Y, Y),
    VIR_SWIZZLE_XZZZ = virmSWIZZLE(X, Z, Z, Z),
    VIR_SWIZZLE_XXZZ = virmSWIZZLE(X, X, Z, Z),
    VIR_SWIZZLE_XXXW = virmSWIZZLE(X, X, X, W),
    VIR_SWIZZLE_XWWW = virmSWIZZLE(X, W, W, W),
    VIR_SWIZZLE_XXZW = virmSWIZZLE(X, X, Z, W),
    VIR_SWIZZLE_YZZZ = virmSWIZZLE(Y, Z, Z, Z),
    VIR_SWIZZLE_YWWW = virmSWIZZLE(Y, W, W, W),
    VIR_SWIZZLE_ZWWW = virmSWIZZLE(Z, W, W, W),
    VIR_SWIZZLE_XYZZ = virmSWIZZLE(X, Y, Z, Z),
    VIR_SWIZZLE_XYWW = virmSWIZZLE(X, Y, W, W),
    VIR_SWIZZLE_XZWW = virmSWIZZLE(X, Z, W, W),
    VIR_SWIZZLE_YZWW = virmSWIZZLE(Y, Z, W, W),
    VIR_SWIZZLE_XXYZ = virmSWIZZLE(X, X, Y, Z),
    VIR_SWIZZLE_XYZW = virmSWIZZLE(X, Y, Z, W),
    VIR_SWIZZLE_XXXY = virmSWIZZLE(X, X, X, Y),
    VIR_SWIZZLE_XYXY = virmSWIZZLE(X, Y, X, Y),
    VIR_SWIZZLE_YYZZ = virmSWIZZLE(Y, Y, Z, Z),
    VIR_SWIZZLE_YYWW = virmSWIZZLE(Y, Y, W, W),
    VIR_SWIZZLE_YYYW = virmSWIZZLE(Y, Y, Y, W),
    VIR_SWIZZLE_ZZZW = virmSWIZZLE(Z, Z, Z, W),
    VIR_SWIZZLE_XZZW = virmSWIZZLE(X, Z, Z, W),
    VIR_SWIZZLE_YYZW = virmSWIZZLE(Y, Y, Z, W),

    VIR_SWIZZLE_INVALID = 0x7FFFFFFF,
} VIR_Swizzle;

typedef enum _VIR_ENABLE
{
    VIR_ENABLE_NONE         = 0x0, /* none is enabled, error/uninitialized state */
    VIR_ENABLE_X            = 0x1,
    VIR_ENABLE_Y            = 0x2,
    VIR_ENABLE_Z            = 0x4,
    VIR_ENABLE_W            = 0x8,
    /* Combinations. */
    VIR_ENABLE_XY           = VIR_ENABLE_X | VIR_ENABLE_Y,
    VIR_ENABLE_XYZ          = VIR_ENABLE_X | VIR_ENABLE_Y | VIR_ENABLE_Z,
    VIR_ENABLE_XYZW         = VIR_ENABLE_X | VIR_ENABLE_Y | VIR_ENABLE_Z | VIR_ENABLE_W,
    VIR_ENABLE_XYW          = VIR_ENABLE_X | VIR_ENABLE_Y | VIR_ENABLE_W,
    VIR_ENABLE_XZ           = VIR_ENABLE_X | VIR_ENABLE_Z,
    VIR_ENABLE_XZW          = VIR_ENABLE_X | VIR_ENABLE_Z | VIR_ENABLE_W,
    VIR_ENABLE_XW           = VIR_ENABLE_X | VIR_ENABLE_W,
    VIR_ENABLE_YZ           = VIR_ENABLE_Y | VIR_ENABLE_Z,
    VIR_ENABLE_YZW          = VIR_ENABLE_Y | VIR_ENABLE_Z | VIR_ENABLE_W,
    VIR_ENABLE_YW           = VIR_ENABLE_Y | VIR_ENABLE_W,
    VIR_ENABLE_ZW           = VIR_ENABLE_Z | VIR_ENABLE_W
}
VIR_Enable;

#define VIR_Swizzle_GetChannel(swizzle, channel)  \
    (VIR_Swizzle)(((swizzle) >> ((channel) << 1)) & 0x3)
#define VIR_Swizzle_SetChannel(swizzle_dest, channel, swizzle_src) \
    swizzle_dest = (VIR_Swizzle)(((swizzle_dest) & (~(0x3 << ((channel) << 1)))) | ((swizzle_src) << ((channel) << 1)))
#define VIR_Swizzle_2_Enable(swizzle)             \
    (VIR_Enable)((1 << VIR_Swizzle_GetChannel(swizzle, 0)) | \
                 (1 << VIR_Swizzle_GetChannel(swizzle, 1)) | \
                 (1 << VIR_Swizzle_GetChannel(swizzle, 2)) | \
                 (1 << VIR_Swizzle_GetChannel(swizzle, 3)))
#define VIR_Enable_Channel_Count(enable)          \
    (((enable) & 0x1) + (((enable) & 0x2) >> 1) + (((enable) & 0x4) >> 2) + (((enable) & 0x8) >> 3))
#define VIR_Swizzle_Channel_Count(swizzle)        \
    VIR_Enable_Channel_Count(VIR_Swizzle_2_Enable(swizzle))
#define VIR_Enable_Covers(enable1, enable2)       \
    ((((enable1) ^ (enable2)) | (enable1)) == (enable1))
#define VIR_Swizzle_Covers(swizzle1, swizzle2)    \
    VIR_Enable_Covers(VIR_Swizzle_2_Enable(swizzle1), VIR_Swizzle_2_Enable(swizzle2))


/* Special register indices. */
extern VIR_NameId   VIR_NAME_UNKNOWN,
                    VIR_NAME_POSITION,
                    VIR_NAME_POINT_SIZE,
                    VIR_NAME_COLOR,
                    VIR_NAME_FRONT_FACING,
                    VIR_NAME_POINT_COORD,
                    VIR_NAME_POSITION_W,
                    VIR_NAME_DEPTH,
                    VIR_NAME_FOG_COORD,
                    VIR_NAME_VERTEX_ID,
                    VIR_NAME_FRONT_COLOR,
                    VIR_NAME_BACK_COLOR,
                    VIR_NAME_FRONT_SECONDARY_COLOR,
                    VIR_NAME_BACK_SECONDARY_COLOR,
                    VIR_NAME_TEX_COORD,
                    VIR_NAME_INSTANCE_ID,
                    VIR_NAME_NUM_GROUPS,
                    VIR_NAME_WORKGROUPSIZE,
                    VIR_NAME_WORK_GROUP_ID,
                    VIR_NAME_LOCAL_INVOCATION_ID,
                    VIR_NAME_GLOBAL_INVOCATION_ID,
                    VIR_NAME_LOCALINVOCATIONINDEX,
                    VIR_NAME_HELPER_INVOCATION,
                    VIR_NAME_SUBSAMPLE_DEPTH,
                    VIR_NAME_PERVERTEX, /* gl_PerVertex */
                    VIR_NAME_IN, /* gl_in */
                    VIR_NAME_OUT, /* gl_out */
                    VIR_NAME_INVOCATION_ID, /* gl_InvocationID */
                    VIR_NAME_PATCH_VERTICES_IN, /* gl_PatchVerticesIn */
                    VIR_NAME_PRIMITIVE_ID, /* gl_PrimitiveID */
                    VIR_NAME_TESS_LEVEL_OUTER, /* gl_TessLevelOuter */
                    VIR_NAME_TESS_LEVEL_INNER, /* gl_TessLevelInner */
                    VIR_NAME_LAYER, /* gl_Layer */
                    VIR_NAME_PS_OUT_LAYER, /* gl_Layer only for ps's output */
                    VIR_NAME_PRIMITIVE_ID_IN, /* gl_PrimitiveIDIn */
                    VIR_NAME_TESS_COORD, /* gl_TessCoord */
                    VIR_NAME_SAMPLE_ID, /* gl_SampleID */
                    VIR_NAME_SAMPLE_POSITION, /* gl_SamplePosition */
                    VIR_NAME_SAMPLE_MASK_IN, /* gl_SampleMaskIn */
                    VIR_NAME_SAMPLE_MASK, /* gl_SampleMask */
                    VIR_NAME_IN_POSITION, /* gl_in.gl_Position*/
                    VIR_NAME_IN_POINT_SIZE, /* gl_in.gl_PointSize*/
                    VIR_NAME_BOUNDING_BOX, /* gl_BoundingBox*/
                    VIR_NAME_BUILTIN_LAST;

typedef enum _VIR_ROUNDMODE
{
    VIR_ROUND_DEFAULT   = 0,
    VIR_ROUND_RTE, /* Round to nearest even */
    VIR_ROUND_RTZ, /* Round toward zero */
    VIR_ROUND_RTP, /* Round toward positive infinity */
    VIR_ROUND_RTN               /* Round toward negative infinity */
} VIR_RoundMode;

typedef enum _VIR_MODIFIER
{
    /* destination modifiers */
    VIR_MOD_NONE            = 0,
    VIR_MOD_SAT_0_TO_1      = 1, /* Satruate the value between [0.0, 1.0] */
    VIR_MOD_SAT_0_TO_INF    = 2, /* Satruate the value between [0.0, +inf) */
    VIR_MOD_SAT_NINF_TO_1   = 3, /* Satruate the value between (-inf, 1.0] */
    VIR_MOD_SAT_TO_MAX_UINT = 4, /* Based on integer bit count, saturate to max uint */
    /* source modifiers */
    VIR_MOD_NEG             = 0x01, /* source negate modifier */
    VIR_MOD_ABS             = 0x02  /* source absolute modfier */
} VIR_Modifier;

/* Possible indices. */
typedef enum _VIR_INDEXED
{
    VIR_INDEXED_NONE, /* 0 */
    VIR_INDEXED_X, /* 1 */
    VIR_INDEXED_Y, /* 2 */
    VIR_INDEXED_Z, /* 3 */
    VIR_INDEXED_W, /* 4 */
    VIR_INDEXED_AL, /* 5, VL, loop count register */
    VIR_INDEXED_VERTEX_ID, /* 6, VL, indexing with vertex id */
}
VIR_Indexed;

/* Possible indices. */
typedef enum _VIR_INDEXED_LEVEL
{
    VIR_NONE_INDEXED, /* 0 */
    VIR_LEAF_INDEXED, /* 1 */
    VIR_NODE_INDEXED, /* 2 */
    VIR_LEAF_AND_NODE_INDEXED             /* 3 */
}
VIR_INDEXED_LEVEL;

/* OCL sampler_t fields value ??? */
typedef enum _VIR_SAMPLERSTATE
{
    VIR_SAMPLER_ADDRESS_NONE            = 0,
    VIR_SAMPLER_ADDRESS_CLAMP           = 1,
    VIR_SAMPLER_ADDRESS_CLAMP_TO_EDGE   = 2,
    VIR_SAMPLER_ADDRESS_REPEAT          = 3,
    VIR_SAMPLER_ADDRESS_MIRRORED_REPEAT = 4,
    VIR_SAMPLER_NORMALIZED_COORDS_FALSE = 0,
    VIR_SAMPLER_NORMALIZED_COORDS_TRUE  = 8,
    VIR_SAMPLER_FILTER_NEAREST          = 0,
    VIR_SAMPLER_FILTER_LINEAR           = 16,
} VIR_SamplerState;

/* OCL image channel data type */
typedef enum _VIR_IMAGEDATATYPE
{
    VIR_IMAGE_SNORM_INT8            = 0,
    VIR_IMAGE_SNORM_INT16           = 1,
    VIR_IMAGE_UNORM_INT8            = 2,
    VIR_IMAGE_UNORM_INT16           = 3,
    VIR_IMAGE_UNORM_SHORT_565       = 4,
    VIR_IMAGE_UNORM_SHORT_555       = 5,
    VIR_IMAGE_UNORM_SHORT_101010    = 6,
    VIR_IMAGE_SIGNED_INT8           = 7,
    VIR_IMAGE_SIGNED_INT16          = 8,
    VIR_IMAGE_SIGNED_INT32          = 9,
    VIR_IMAGE_UNSIGNED_INT8         = 10,
    VIR_IMAGE_UNSIGNED_INT16        = 11,
    VIR_IMAGE_UNSIGNED_INT32        = 12,
    VIR_IMAGE_HALF_FLOAT            = 13,
    VIR_IMAGE_FLOAT                 = 14,
    VIR_IMAGE_UNORM_INT24           = 15
} VIR_ImageDataType;

#define VIR_INTRINSIC_INFO(Intrinsic)   VIR_IK_##Intrinsic
typedef enum _VIR_INTRINSICSKIND
{
#include "gc_vsc_vir_intrinsic_kind.def.h"
} VIR_IntrinsicsKind;
#undef VIR_INTRINSIC_INFO


#define VIR_OPINFO(OPCODE, OPNDNUM, FLAGS, WRITE2DEST, LEVEL)   VIR_OP_##OPCODE
typedef enum _VIR_OPCODE
{
#include "gc_vsc_vir_opcode.def.h"
} VIR_OpCode;
#undef VIR_OPINFO

typedef enum _VIR_OP_FLAG
{
    VIR_OPFLAG_NoDest               = 0x00,
    VIR_OPFLAG_HasDest              = 0x01,
    VIR_OPFLAG_Transcendental       = 0x02,
    VIR_OPFLAG_IntegerOnly          = 0x04,
    VIR_OPFLAG_ControlFlow          = 0x08,
    VIR_OPFLAG_OCLOnly              = 0x10,
    VIR_OPFLAG_OGLOnly              = 0x20,
    VIR_OPFLAG_UseCondCode          = 0x40,
    VIR_OPFLAG_Componentwise        = 0x80, /* the operation is componentwise */
    VIR_OPFLAG_Src0Componentwise    = 0x100,
    VIR_OPFLAG_Src1Componentwise    = 0x200,
    VIR_OPFLAG_Src2Componentwise    = 0x400,
    VIR_OPFLAG_Src3Componentwise    = 0x800,
    VIR_OPFLAG_OnlyUseEnable        = 0x1000,
    VIR_OPFLAG_MemoryRead           = 0x2000,
    VIR_OPFLAG_MemoryWrite          = 0x4000,
    VIR_OPFLAG_Expression           = 0x8000,
    VIR_OPFLAG_ExpdPrecFromHighest  = 0x10000, /*expected result precision should be the same as */
    VIR_OPFLAG_ExpdPrecFromSrc0     = 0x20000,
    VIR_OPFLAG_ExpdPrecFromSrc12    = 0x30000,
    VIR_OPFLAG_ExpdPrecFromSrc2     = 0x40000,
    VIR_OPFLAG_ExpdPrecFromBits     = 0x70000,
    VIR_OPFLAG_VXOnly               = 0x80000, /* VX only instruction (EVIS) */
    VIR_OPFLAG_EVIS_Modifier_MASK   = 0x700000, /* VX inst EVIS_Modifier operand number */
    VIR_OPFLAG_EVIS_Modifier_SHIFT  = 20, /* VX inst EVIS_Modifier shift count */
    VIR_OPFLAG_Use512Unifrom        = 0x800000, /* VX instruction, use 512 bit uniform */
} VIR_OpFlag;

typedef enum _VIR_OP_LEVEL
{
    VIR_OPLEVEL_NotUsed     = 0x00, /* the opcode is currently not used */
    VIR_OPLEVEL_High        = 0x01,
    VIR_OPLEVEL_Medium      = 0x02,
    VIR_OPLEVEL_Low         = 0x04,
    VIR_OPLEVEL_Machine     = 0x08, /* machine level */
    VIR_OPLEVEL_HighMedium  = 0x03, /* high and medium level */
    VIR_OPLEVEL_NotMachine  = 0x07, /* not in machine level */
    VIR_OPLEVEL_LowUnder    = 0x0c, /* low and machine level */
    VIR_OPLEVEL_All         = 0x0f
} VIR_OpLevel;

typedef struct _VIR_OPCODE_INFO
{
    VIR_OpCode          opcode      : 10;
    gctUINT             srcNum      : 4;    /* source operand number */
    gctUINT             write2Dest  : 1;
    VIR_OpLevel         level       : 5;    /* instruction levels */
    gctUINT                         : 0;    /* padding */
    VIR_OpFlag          flags;
} VIR_Opcode_Info;

extern const VIR_Opcode_Info VIR_OpcodeInfo[];

extern const gctSTRING VIR_OpName[];
extern const gctSTRING VIR_CondOpName[];
extern const gctSTRING VIR_RoundModeName[];
extern const gctSTRING VIR_DestModifierName[];
extern const gctSTRING VIR_SrcModifierName[];
extern const gctSTRING VIR_IntrinsicName[];

/* Shader types. */
typedef enum _VIR_SHADERKIND {
    VIR_SHADER_UNKNOWN = 0,
    VIR_SHADER_VERTEX,
    VIR_SHADER_FRAGMENT,
    VIR_SHADER_CL,
    VIR_SHADER_PRECOMPILED,
    VIR_SHADER_COMPUTE, /* OGL, DX11 */
    VIR_SHADER_TESSELLATION_CONTROL, /* OGL, DOMAIN shader for DX11 */
    VIR_SHADER_TESSELLATION_EVALUATION, /* OGL, HULL shader for DX11 */
    VIR_SHADER_GEOMETRY, /* OGL, DX10/11 */
    VIR_SHADER_LIBRARY,
    VIR_SHADER_KIND_COUNT
} VIR_ShaderKind;

/* Structure that defines a VIR instruction. */

typedef struct _VIR_SOURCE_FILE_LOC
{
    gctUINT32       fileId : 6;
    gctUINT32       colNo  : 10;
    gctUINT32       lineNo : 16;
} VIR_SourceFileLoc;

typedef enum _VIR_SYMBOLKIND
{
    VIR_SYM_UNKNOWN   = 0,
    VIR_SYM_UNIFORM,
    VIR_SYM_UBO, /* uniform block object */
    VIR_SYM_VARIABLE, /* global/local variables, input/output */
    VIR_SYM_SBO, /* storage buffer variables, use memory to read/write */
    VIR_SYM_FIELD, /* the field of class/struct/union/ubo/sbo */
    VIR_SYM_FUNCTION, /* function */
    VIR_SYM_SAMPLER,
    VIR_SYM_TEXTURE,
    VIR_SYM_IMAGE,
    VIR_SYM_CONST,
    VIR_SYM_VIRREG, /* virtual register */
    VIR_SYM_TYPE, /* typedef */
    VIR_SYM_LABEL,
    VIR_SYM_IOBLOCK, /* in/out block */
    VIR_SYMKIND_COUNT       /* need to change VIR_GetSymbolKindName
                               if symbol kind is changed */
} VIR_SymbolKind;

typedef enum _VIR_STORAGECLASS
{
    VIR_STORAGE_UNKNOWN,
    VIR_STORAGE_INPUT, /* input attribute/block */
    VIR_STORAGE_OUTPUT, /* output variable/block */
    VIR_STORAGE_PERPATCH_INPUT, /* per-patch input attribute/block */
    VIR_STORAGE_PERPATCH_OUTPUT, /* per-patch output variable/block */
    VIR_STORAGE_PERPATCH_INOUT, /* per-patch input or output variable/block, depends on shader type */
    VIR_STORAGE_INOUTPUT, /* input and output variable */
    VIR_STORAGE_LOCAL, /* local variable */
    VIR_STORAGE_GLOBAL, /* global variable */
    VIR_STORAGE_INPARM, /* input parameter */
    VIR_STORAGE_OUTPARM, /* output parameter */
    VIR_STORAGE_INOUTPARM, /* input output parameter */
    VIR_STORAGE_FUNCSTATIC, /* function scope static variable */
    VIR_STORAGE_FILESTATIC, /* file scope static variable */
    VIR_STORAGE_EXTERN, /* undefined external data or code */
    VIR_STORAGE_REGISTER, /* temp register */
    VIR_STORAGE_LOCALSTORAGE, /* local storage */
} VIR_StorageClass;

/* TODO: create a map later to hide the detail of uniform implemenation */
typedef enum _VIR_UNIFORMKIND
{
    VIR_UNIFORM_NORMAL, /* normal uniform */
    VIR_UNIFORM_KERNEL_ARG,
    VIR_UNIFORM_KERNEL_ARG_LOCAL,
    VIR_UNIFORM_KERNEL_ARG_SAMPLER,
    VIR_UNIFORM_LOCAL_ADDRESS_SPACE,
    VIR_UNIFORM_PRIVATE_ADDRESS_SPACE,
    VIR_UNIFORM_CONSTANT_ADDRESS_SPACE,
    VIR_UNIFORM_GLOBAL_SIZE,
    VIR_UNIFORM_LOCAL_SIZE,
    VIR_UNIFORM_NUM_GROUPS,
    VIR_UNIFORM_GLOBAL_OFFSET,
    VIR_UNIFORM_WORK_DIM,
    VIR_UNIFORM_KERNEL_ARG_CONSTANT,
    VIR_UNIFORM_KERNEL_ARG_LOCAL_MEM_SIZE,
    VIR_UNIFORM_KERNEL_ARG_PRIVATE,
    VIR_UNIFORM_LOADTIME_CONSTANT,
    VIR_UNIFORM_TRANSFORM_FEEDBACK_BUFFER,
    VIR_UNIFORM_TRANSFORM_FEEDBACK_STATE,
    VIR_UNIFORM_OPT_CONSTANT_TEXLD_COORD,
    VIR_UNIFORM_BLOCK_MEMBER,
    VIR_UNIFORM_UNIFORM_BLOCK_ADDRESS,
    VIR_UNIFORM_LOD_MIN_MAX,
    VIR_UNIFORM_LEVEL_BASE_SIZE,
    VIR_UNIFORM_STRUCT,
    VIR_UNIFORM_STORAGE_BLOCK_ADDRESS,
    VIR_UNIFORM_SAMPLE_LOCATION,
    VIR_UNIFORM_ENABLE_MULTISAMPLE_BUFFERS,
    VIR_UNIFORM_TEMP_REG_SPILL_MEM_ADDRESS,
} VIR_UniformKind;

typedef enum _VIR_ADDRSPACE
{
    VIR_AS_PRIVATE, /* private address space */
    VIR_AS_GLOBAL, /* global address space */
    VIR_AS_CONSTANT, /* constant address space, uniform mapped to this space */
    VIR_AS_LOCAL            /* local address space, function scope locals mappped
                               into this space */
} VIR_AddrSpace;

typedef struct _VIR_ARRAYDIM VIR_ArrayDim;
struct _VIR_ARRAYDIM
{
    gctUINT             elements;
    gctUINT             stride;
    VIR_ArrayDim *      next;       /* next dimension*/
};

typedef enum _VIR_LAYOUTQUAL
{
    VIR_LAYQUAL_NONE                                 = 0x0,
    VIR_LAYQUAL_PACKED                               = 0x1,
    VIR_LAYQUAL_SHARED                               = 0x2,
    VIR_LAYQUAL_STD140                               = 0x4,
    VIR_LAYQUAL_ROW_MAJOR                            = 0x8,
    VIR_LAYQUAL_COLUMN_MAJOR                         = 0x10,
    VIR_LAYQUAL_LOCATION                             = 0x20,
    VIR_LAYQUAL_STD430                               = 0x40,
    VIR_LAYQUAL_BINDING                              = 0x80,
    VIR_LAYQUAL_OFFSET                               = 0x100,
    VIR_LAYQUAL_BLEND                                = 0x200,
    VIR_LAYQUAL_IMAGE_FORMAT                         = 0x400,

    VIR_LAYQUAL_BLEND_MASK                           = 0xf00000,
    VIR_LAYQUAL_BLEND_SUPPORT_NONE                   = 0x0,
    VIR_LAYQUAL_BLEND_SUPPORT_MULTIPLY               = 0x100000,
    VIR_LAYQUAL_BLEND_SUPPORT_OVERLAY                = 0x200000,
    VIR_LAYQUAL_BLEND_SUPPORT_DARKEN                 = 0x300000,
    VIR_LAYQUAL_BLEND_SUPPORT_LIGHTEN                = 0x400000,
    VIR_LAYQUAL_BLEND_SUPPORT_COLORDODGE             = 0x500000,
    VIR_LAYQUAL_BLEND_SUPPORT_COLORBURN              = 0x600000,
    VIR_LAYQUAL_BLEND_SUPPORT_HARDLIGHT              = 0x700000,
    VIR_LAYQUAL_BLEND_SUPPORT_SOFTLIGHT              = 0x800000,
    VIR_LAYQUAL_BLEND_SUPPORT_DIFFERENCE             = 0x900000,
    VIR_LAYQUAL_BLEND_SUPPORT_EXCLUSION              = 0xA00000,
    VIR_LAYQUAL_BLEND_SUPPORT_HSL_HUE                = 0xB00000,
    VIR_LAYQUAL_BLEND_SUPPORT_HSL_SATURATION         = 0xC00000,
    VIR_LAYQUAL_BLEND_SUPPORT_HSL_COLOR              = 0xD00000,
    VIR_LAYQUAL_BLEND_SUPPORT_HSL_LUMINOSITY         = 0xE00000,
    VIR_LAYQUAL_BLEND_SUPPORT_SCREEN                 = 0xF00000,

    VIR_LAYQUAL_IMAGE_FORMAT_MASK                    = 0xF000000,
    VIR_LAYQUAL_IMAGE_FORMAT_NONE                    = 0x0,
    VIR_LAYQUAL_IMAGE_FORMAT_RGBA32F                 = 0x1000000,
    VIR_LAYQUAL_IMAGE_FORMAT_RGBA16F                 = 0x2000000,
    VIR_LAYQUAL_IMAGE_FORMAT_R32F                    = 0x3000000,
    VIR_LAYQUAL_IMAGE_FORMAT_RGBA8                   = 0x4000000,
    VIR_LAYQUAL_IMAGE_FORMAT_RGBA8_SNORM             = 0x5000000,
    VIR_LAYQUAL_IMAGE_FORMAT_RGBA32I                 = 0x6000000,
    VIR_LAYQUAL_IMAGE_FORMAT_RGBA16I                 = 0x7000000,
    VIR_LAYQUAL_IMAGE_FORMAT_RGBA8I                  = 0x8000000,
    VIR_LAYQUAL_IMAGE_FORMAT_R32I                    = 0x9000000,
    VIR_LAYQUAL_IMAGE_FORMAT_RGBA32UI                = 0xA000000,
    VIR_LAYQUAL_IMAGE_FORMAT_RGBA16UI                = 0xB000000,
    VIR_LAYQUAL_IMAGE_FORMAT_RGBA8UI                 = 0xC000000,
    VIR_LAYQUAL_IMAGE_FORMAT_R32UI                   = 0xD000000,
} VIR_LayoutQual;

typedef struct _VIR_LAYOUT
{
    VIR_LayoutQual layoutQualifier;     /* layout quliafiers */
    gctINT         location;            /* location of in/out variable, uniform, interface block */
    gctUINT        binding;             /* binding for SBO/UBO/AtomicCounter/sampler/image */
    gctUINT        offset;              /* offset in bound mem resource for atomicCounter */
    gctUINT        llFirstSlot;         /* low level first slot for SEP mapping tables */
    gctUINT        llArraySlot;         /* low level array slot for SEP mapping tables */
    gctUINT        hwFirstCompIndex;    /* HW's 'attributeIndex' to index USC, just for I/O */
} VIR_Layout;

#define VIR_Layout_GetLocation(Layout)      ((Layout)->location)
#define VIR_Layout_GetBinding(Layout)       ((Layout)->binding)
#define VIR_Layout_GetOffset(Layout)        ((Layout)->offset)
#define VIR_Layout_GetLlFirstSlot(Layout)       ((Layout)->llFirstSlot)
#define VIR_Layout_GetLlArraySlot(Layout)       ((Layout)->llArraySlot)
#define VIR_Layout_GetHwFirstCompIndex(Layout)  ((Layout)->hwFirstCompIndex)

typedef enum _VIR_TYQUALIFIER
{
    VIR_TYQUAL_NONE         = 0x00, /* unqualified */
    VIR_TYQUAL_CONST        = 0x01, /* const */
    VIR_TYQUAL_VOLATILE     = 0x02, /* volatile */
    VIR_TYQUAL_RESTRICT     = 0x04, /* restrict */
    VIR_TYQUAL_READ_ONLY    = 0x08, /* readonly */
    VIR_TYQUAL_WRITE_ONLY   = 0x10, /* writeonly */
    VIR_TYQUAL_CONSTANT     = 0x20, /* constant address space */
    VIR_TYQUAL_GLOBAL       = 0x40, /* global address space */
    VIR_TYQUAL_LOCAL        = 0x80, /* local address space */
    VIR_TYQUAL_PRIVATE      = 0x100, /* private address space */
} VIR_TyQualifier;

typedef struct _VIR_FIELDINFO
{
    gctUINT32   offset;             /* byte offset from the begin of struct */
    gctUINT     isBitfield  : 1;
    gctUINT     bitSize     : 7;    /* number of bits */
    gctUINT     startBit    : 8;    /* start bit from the first byte of this field */
    gctUINT     tempRegOrUniformOffset : 16;   /* the temp register or uniform offset of this field */
} VIR_FieldInfo;

typedef enum VIR_SYMFLAG
{
    /* General flags */
    VIR_SYMFLAG_NONE                            = 0x00000000, /* no flag */
    VIR_SYMFLAG_ENABLED                         = 0x00000001, /* Flag to indicate this attribute is enabeld or not. */
    VIR_SYMFLAG_INACTIVE                        = 0x00000002,
    VIR_SYMFLAG_FLAT                            = 0x00000004,
    VIR_SYMFLAG_INVARIANT                       = 0x00000008,
    VIR_SYMFLAG_IS_ARRAY                        = 0x00000010, /* variable/uniform is array type */
    VIR_SYMFLAG_IS_POINTER                      = 0x00000020, /* Wheter the uniform is a pointer. */
    VIR_SYMFLAG_IS_STRUCT                       = 0x00000040, /* variable/uniform is struct type */
    VIR_SYMFLAG_IS_FIELD                        = 0x00000080, /* variable/uniform is struct field */
    VIR_SYMFLAG_COMPILER_GEN                    = 0x00000100, /* set the symbol compiler generated */
    VIR_SYMFLAG_BUILTIN                         = 0x00000200, /* set the symbol builtin */
    VIR_SYMFLAG_DIRECTLY_ADDRESSED              = 0x00000400, /* set the symbol directly addressed */
    VIR_SYMFLAG_ARRAYED_PER_VERTEX              = 0x00000800, /* set the symbol directly addressed */
    VIR_SYMFLAG_PRECISE                         = 0x00001000, /* symbol is precise */
    VIR_SYMFLAG_LOAD_STORE_ATTR                 = 0x00002000, /* only used for inputs/outputs */
    VIR_SYMFLAG_STATICALLY_USED                 = 0x00004000, /* set the symbol statically used */
    VIR_SYMFLAG_IS_IOBLOCK_MEMBER               = 0x00008000, /* variable is a member of an IO block */
    VIR_SYMFLAG_IS_INSTANCE_MEMBER              = 0x00010000, /* variable is a member of an IO block with a instance name*/

    /* uniform flags: may need the seperate flags inside the uniform or
                      combine with the uniformKind  */
    VIR_SYMUNIFORMFLAG_LOADTIME_CONSTANT        = 0x00020000,
    VIR_SYMUNIFORMFLAG_COMPILETIME_INITIALIZED  = 0x00040000,
    VIR_SYMUNIFORMFLAG_USED_IN_SHADER           = 0x00080000,
    VIR_SYMUNIFORMFLAG_USED_IN_LTC              = 0x00100000, /* it may be not used in
                                                                   * shader, but is used in LTC */
    VIR_SYMUNIFORMFLAG_MOVED_TO_DUB             = 0x00200000, /*set the uniform moved from user defined uniform block
                                                                    to default uniform block */
    VIR_SYMUNIFORMFLAG_SAMPLER_CALCULATE_TEX_SIZE = 0x00400000,
    VIR_SYMUNIFORMFLAG_IMPLICITLY_USED          = 0x00800000,
    VIR_SYMUNIFORMFLAG_FORCE_ACTIVE             = 0x01000000,
    VIR_SYMUNIFORMFLAG_MOVING_TO_DUBO           = 0x02000000, /* set the symbol to be moved to default UBO */
    VIR_SYMUNIFORMFLAG_ALWAYS_IN_DUB            = 0x04000000, /* set the symbol always in default uniform block */
    VIR_SYMUNIFORMFLAG_MOVED_TO_DUBO            = 0x08000000, /* set the uniform moved from default uniform block to DUBO */
    VIR_SYMUNIFORMFLAG_MOVED_TO_CUBO            = 0x10000000, /* set the uniform moved from constant uniform block to DUBO */
    VIR_SYMUNIFORMFLAG_ATOMIC_COUNTER           = 0x20000000, /* set the symblo atomic counter */
    VIR_SYMUNIFORMFLAG_VIR_NEW_ADDED            = 0x40000000, /* gcSL has no such uniform, for converter only,
                                                                   will be removed when we fully switch to vir
                                                                   path */
    VIR_SYMUNIFORMFLAG_TREAT_SAMPLER_AS_CONST   = 0x80000000,

    /* input/output flags */
    VIR_SYMFLAG_ISCENTROID                      = 0x00020000,
    VIR_SYMFLAG_ISSAMPLE                        = 0x00040000,

    /* input attribute flags */
    VIR_SYMFLAG_ISTEXTURE                       = 0x00080000,
    VIR_SYMFLAG_PACKEDAWAY                      = 0x00100000, /* the input is packed with other input
                                                                   * it is no longer visible spearately */
    VIR_SYMFLAG_ZWTEXTURE                       = 0x00200000, /* when another texture coord packed
                                                                   * with this attribute (2-2 packing) */
    VIR_SYMFLAG_POSITION                        = 0x00400000, /* the input attribute is position */
    VIR_SYMFLAG_CONV2REG                        = 0x00800000, /* Converted to physical register */
    VIR_SYMFLAG_ALWAYSUSED                      = 0x01000000, /* set the attribute always used so the
                                                                   * recompilation wouldn't remove this
                                                                   * attribute later */
    VIR_SYMFLAG_UNUSED                          = 0x02000000, /* set the attribute always used so the
                                                                   * recompilation wouldn't remove this
                                                                   * attribute later */
    VIR_SYMFLAG_POINTSPRITE_TC                  = 0x04000000,

    VIR_SYMFLAG_VECTORIZED_OUT                  = 0x08000000, /* This input/output has been vectorized into
                                                                     a new one */
} VIR_SymFlag;

#define isSymEnabled(sym)                       (((sym)->flags & VIR_SYMFLAG_ENABLED) != 0)
#define isSymInactive(sym)                      (((sym)->flags & VIR_SYMFLAG_INACTIVE) != 0)
#define isSymFlat(sym)                          (((sym)->flags & VIR_SYMFLAG_FLAT) != 0)
#define isSymInvariant(sym)                     (((sym)->flags & VIR_SYMFLAG_INVARIANT) != 0)
#define isSymArray(sym)                         (((sym)->flags & VIR_SYMFLAG_IS_ARRAY) != 0)
#define isSymPointer(sym)                       (((sym)->flags & VIR_SYMFLAG_IS_POINTER) != 0)
#define isSymStruct(sym)                        (((sym)->flags & VIR_SYMFLAG_IS_STRUCT) != 0)
#define isSymField(sym)                         (((sym)->flags & VIR_SYMFLAG_IS_FIELD) != 0)
#define isSymCompilerGen(sym)                   (((sym)->flags & VIR_SYMFLAG_COMPILER_GEN) != 0)
#define isSymBuildin(sym)                       (((sym)->flags & VIR_SYMFLAG_BUILTIN) != 0)
#define isSymDirectlyAddressed(sym)             (((sym)->flags & VIR_SYMFLAG_DIRECTLY_ADDRESSED) != 0)
#define isSymArrayedPerVertex(sym)              (((sym)->flags & VIR_SYMFLAG_ARRAYED_PER_VERTEX) != 0)
#define isSymPrecise(sym)                       (((sym)->flags & VIR_SYMFLAG_PRECISE) != 0)
#define isSymLoadStoreAttr(sym)                 (((sym)->flags & VIR_SYMFLAG_LOAD_STORE_ATTR) != 0)
#define isSymStaticallyUsed(sym)                (((sym)->flags & VIR_SYMFLAG_STATICALLY_USED) != 0)
#define isSymIOBlockMember(sym)                 (((sym)->flags & VIR_SYMFLAG_IS_IOBLOCK_MEMBER) != 0)
#define isSymInstanceMember(sym)                (((sym)->flags & VIR_SYMFLAG_IS_INSTANCE_MEMBER) != 0)

#define isSymUniformLoadtimeConst(u)            (((u)->flags & VIR_SYMUNIFORMFLAG_LOADTIME_CONSTANT) != 0)
#define isSymUniformCompiletimeInitialized(u)   (((u)->flags & VIR_SYMUNIFORMFLAG_COMPILETIME_INITIALIZED) != 0)
#define isSymUniformUsedInShader(u)             (((u)->flags & VIR_SYMUNIFORMFLAG_USED_IN_SHADER) != 0)
#define isSymUniformUsedInLTC(u)                (((u)->flags & VIR_SYMUNIFORMFLAG_USED_IN_LTC) != 0)
#define isSymUniformMovedToDUB(u)               (((u)->flags & VIR_SYMUNIFORMFLAG_MOVED_TO_DUB) != 0)
#define isSymUniformUsedInTextureSize(u)        (((u)->flags & VIR_SYMUNIFORMFLAG_SAMPLER_CALCULATE_TEX_SIZE) != 0)
#define isSymUniformImplicitlyUsed(u)           (((u)->flags & VIR_SYMUNIFORMFLAG_IMPLICITLY_USED) != 0)
#define isSymUniformForcedToActive(u)           (((u)->flags & VIR_SYMUNIFORMFLAG_FORCE_ACTIVE) != 0)
#define isSymUniformMovingToDUBO(u)             (((u)->flags & VIR_SYMUNIFORMFLAG_MOVING_TO_DUBO) != 0)
#define isSymUniformAlwaysInDUBO(u)             (((u)->flags & VIR_SYMUNIFORMFLAG_ALWAYS_IN_DUB) != 0)
#define isSymUniformMovedToDUBO(u)              (((u)->flags & VIR_SYMUNIFORMFLAG_MOVED_TO_DUBO) != 0)
#define isSymUniformMovedToCUBO(u)              (((u)->flags & VIR_SYMUNIFORMFLAG_MOVED_TO_CUBO) != 0)
#define isSymUniformMovedToAUBO(u)              (isSymUniformMovedToDUBO(u) || isSymUniformMovedToCUBO(u))
#define isSymUniformAtomicCounter(u)            (((u)->flags & VIR_SYMUNIFORMFLAG_ATOMIC_COUNTER) != 0)
#define isSymUniformVIRNewAdded(u)              (((u)->flags & VIR_SYMUNIFORMFLAG_VIR_NEW_ADDED) != 0)
#define isSymUniformTreatSamplerAsConst(u)      (((u)->flags & VIR_SYMUNIFORMFLAG_TREAT_SAMPLER_AS_CONST) != 0)

#define isSymCentroid(sym)                      (((sym)->flags & VIR_SYMFLAG_ISCENTROID) != 0)
#define isSymSample(sym)                        (((sym)->flags & VIR_SYMFLAG_ISSAMPLE) != 0)

#define isSymUnused(sym)                        (((sym)->flags & VIR_SYMFLAG_UNUSED) != 0)

#define isSymAlwaysUsed(sym)                    (((sym)->flags & VIR_SYMFLAG_ALWAYSUSED) != 0)

#define isSymVectorizedOut(sym)                 (((sym)->flags & VIR_SYMFLAG_VECTORIZED_OUT) != 0)

/*   o UBO symbol
 *      . no instance name: we create an anonymous instance name for it,
 *        the anonymous instance has UniformBlockObject type, which
 *        specifies the layout, type and offset of each member
 *      . with instance name: the instance name is the symbol name
 *        which has the type of UBO
 *      . array of instance: the instance name is the symbol name
 *        which has the type of array of UBO
 */
struct _VIR_SYMBOL
{
    VIR_SymbolKind      _kind         : 5;
    gctUINT             _storageClass : 5;   /* storage class for variables
                                             or uniform Kind for uniform */
    gctUINT             _addrSpace    : 2;
    gctUINT             _precision    : 2;
    gctUINT             _qualifier    : 2;
    gctUINT             _reserved1    : 16; /* not used bits */

    VIR_HwRegId         _hwRegId      : 10;/* allocated HW register for the temp or attribute */
    gctUINT             _hwShift      : 2; /* shift for HW register */
    VIR_HwRegId         _HIhwRegId    : 10; /* in dual16, we need to assign a register pair to high precison attribute */
    gctUINT             _HIhwShift    : 2;  /* shift for HW register */
    gctUINT             _reserved2    : 8;  /* not used bits */

    VIR_Type *          type;               /* the type of the symbol */

    VIR_SymFlag         flags;

    VIR_SymId           index;          /* index of this entry in symtab */

    VIR_SymId           ioBlockIndex;
    VIR_Layout          layout;         /* layout info for in/out/unifrom */
    union
    {
        VIR_VirRegId    vregIndex;      /* the virtual reg index for VIRREG */
        VIR_NameId      name;           /* name of the symbol */
        VIR_ConstId     constId;        /* the id of constant value */
    } u1;

    union
    {
        VIR_VirRegId    tempIndex;      /* the start virtual register for VARIABLE */
        VIR_UniformId   uniformIndex;   /* the start uniform index for UNIFORMSTRUCT */
        VIR_Symbol *    variable;       /* the corresponding variable for VIRREG */
        VIR_FieldInfo * fieldInfo;      /* pointer to the field info for FIELD */
        VIR_Function *  function;       /* point to function for FUNCTION symbol */
        VIR_Uniform *   uniform;        /* point to Uniform for UNIFORM symbol */
        VIR_Uniform *   sampler;        /* point to Uniform for SAMPLER symbol */
        VIR_Uniform *   image;          /* point to Uniform for IMAGE symbol */
        VIR_UniformBlock * ubo;         /* point to uniform block for UBO */
        VIR_StorageBlock * sbo;         /* point to storage block for SBO */
        VIR_IOBlock      * ioBlock;     /* point to io block */
    } u2;

    union
    {
        VIR_NameId      mangledName;    /* mangled name for function */
        gctUINT32       size;           /* the size for struct */
        VIR_Type *      structType;     /* the struc type in which the field is defined */
        gctUINT         offsetInVar;    /* for VIRREG, it is offset from begining of
                                         * corresponding variable (one variable may map
                                         * to multiple VIRREG) */
    } u3;
    gctINT              indexRange;     /* for VIRREG, UNIFORM, VARIABLE or FIELD,
                                           the end virReg or uniform it may be indexed */
};

typedef struct _VIR_SymIndexingInfo
{
    VIR_Symbol * virRegSym;         /* e.g. for temp(16) with def: out mat4 a[3];  ==> temp(10 - 21) */
    VIR_Symbol * underlyingSym;     /* the underlying symbol of the virReg: "a" */
    gctINT       arrayIndexing;     /* 0: temp(10 - 13), 1: temp(14 - 17), 2: temp(18 - 21) */
    gctINT       elemOffset;        /* 0: temp(10), temp(14), temp(18)
                                     * 1: temp(11), temp(15), temp(19)
                                     * 2: temp(12), temp(16), temp(20)
                                     * 3: temp(13), temp(17), temp(21)
                                     */
} VIR_SymIndexingInfo;

/* Constant */
typedef union _VIR_SCALARCONSTVAL
{
    gctFLOAT    fValue;
    gctUINT     uValue;
    gctINT      iValue;
} VIR_ScalarConstVal;

typedef union _VIR_VECCONSTVAL
{
    /* TODO: change array to pointer and use local variable or pool memory
     *       for values
     */
    gctFLOAT    f32Value[16];
    gctUINT     u32Value[16];
    gctINT      i32Value[16];
    gctUINT16   u16Value[16];
    gctINT16    i16Value[16];
    gctUINT8    u8Value[16];
    gctINT8     i8Value[16];
}VIR_VecConstVal;

typedef union _VIR_CONSTVAL
{
    VIR_VecConstVal     vecVal;
    VIR_ScalarConstVal  scalarVal;
}  VIR_ConstVal;

struct _VIR_CONST
{
    VIR_ConstId      index;      /* the index of this entry in const table */
    VIR_TypeId       type;       /* type of the constant */
    VIR_ConstVal     value;
};

/* Types */

/* types:
 *   primitive types
 *      scalar  types
 *      vector  types
 *      matrix  types
 *      sampler types
 *      image   types
 *      void    types
 *  derived types
 *      pointer types
 *      function types
 *      aggregate types
 *          array types
 *          struct types
 */
typedef enum _VIR_TYPEKIND
{
    VIR_TY_INVALID,
    VIR_TY_SCALAR,
    VIR_TY_VECTOR,
    VIR_TY_MATRIX,
    VIR_TY_SAMPLER,
    VIR_TY_IMAGE,
    VIR_TY_VOID,
    VIR_TY_POINTER,
    VIR_TY_ARRAY,
    VIR_TY_STRUCT,
    VIR_TY_FUNCTION,
    VIR_TY_META,
} VIR_TypeKind;

typedef enum _VIR_TYFLAG
{
    /* common flags for all type */
    VIR_TYFLAG_NONE       = 0x000,
    VIR_TYFLAG_SIZED      = 0x001, /* the type has size */
    VIR_TYFLAG_BUILTIN    = 0x002, /* builtin type */
    VIR_TYFLAG_PACKED     = 0x004, /* packed type */
    /* flags for struct */
    VIR_TYFLAG_ISUNION      = 0x010,
    VIR_TYFLAG_PACKEDSTRUCT = 0x020, /* packed struct */
    VIR_TYFLAG_ANONYMOUS    = 0x040, /* the struct/union/ is anonymous */
    VIR_TYFLAG_HASBODY      = 0x080, /* body definition is seen */
    /* flags for saclar/vector/matrix */
    VIR_TYFLAG_ISFLOAT      = 0x010, /* is float type */
    VIR_TYFLAG_ISINTEGER    = 0x020, /* is integer type */

    /* flags for function */

} VIR_TyFlag;

typedef struct _VIR_BUILTINTYPEINFO
{
    const char *    name;               /* e.g. float_2X4 */
    VIR_TypeId      type;               /* e.g. VIR_TYPE_FLOAT_2X4 */
    gctINT          components;         /* e.g. 4 components each row */
    gctINT          rows;               /* e.g. 2 rows             */
    VIR_TypeId      rowType;            /* e.g. VIR_TYPE_FLOAT_X4  */
    VIR_TypeId      componentType;      /* e.g. VIR_TYPE_FLOAT_X1  */
    size_t          sz;
    gctUINT         _unused;
    VIR_TyFlag      flag;
    gctUINT         alignment;
    VIR_TypeKind    kind;
} VIR_BuiltinTypeInfo;

/* the maximium type we can support  in VIR is 2^16, the symbol is 2^20 */
struct _VIR_TYPE
{
    VIR_TypeId          _base      : 20; /* type id for primitive type, points to type,
                                           array base type, function return type
                                         */
    VIR_TyFlag          _flags     : 12; /* flags for  the type */

    VIR_TypeId          _tyIndex   : 20; /* the index for this entry */
    gctUINT             _kind      : 4;
    gctUINT             _alignment : 3;  /* power of 2 alignment, up to 2^7 (128) */
    gctUINT             _qualifier : 3;  /* type qualifiers of pointer points to */
    gctUINT             _addrSpace : 2;  /* address space of pointer points to */

    struct
    {
        VIR_SymId         symId;        /* the symbol info for struct */
        VIR_NameId        nameId;       /* the name id for other type */
    } u1;

    union
    {
        gctUINT           size;         /* the size of type for non-array,
                                           non-struct, non-function type */
        gctUINT           arrayLength;  /* the element of the array */
        VIR_SymIdList *   fields;       /* points to first field for struct */
        VIR_TypeIdList *  params;       /* parameter type list */
    } u2;
};


extern VIR_BuiltinTypeInfo VIR_builtinTypes[];

/* get type info from primitive type id, a faster way than from type node VIR_Type_GetXXX */
#define VIR_GetTypeName(Ty)             (VIR_builtinTypes[(Ty)].name)
#define VIR_GetTypeType(Ty)             (VIR_builtinTypes[(Ty)].type)
#define VIR_GetTypeComponents(Ty)       (VIR_builtinTypes[(Ty)].components)
#define VIR_GetTypeRows(Ty)             (VIR_builtinTypes[(Ty)].rows)
#define VIR_GetTypeRowType(Ty)          (VIR_builtinTypes[(Ty)].rowType)
#define VIR_GetTypeComponentType(Ty)    (VIR_builtinTypes[(Ty)].componentType)
#define VIR_GetTypeSize(Ty)             (VIR_builtinTypes[(Ty)].sz)
#define VIR_GetTypeFlag(Ty)             (VIR_builtinTypes[(Ty)].flag)
#define VIR_GetTypeAlignment(Ty)        (VIR_builtinTypes[(Ty)].alignment)
#define VIR_GetTypeTypeKind(Ty)         (VIR_builtinTypes[(Ty)].kind)

#if VIR_POINTER_64BIT
#define POINTER_SIZE    8
#else
#define POINTER_SIZE    4
#endif

typedef enum _VIR_OPERANDKIND
{
    VIR_OPND_NONE,
    VIR_OPND_UNDEF, /* 1, the operand is undefined */
    VIR_OPND_SYMBOL, /* 2, variable, input, output */
    VIR_OPND_VIRREG, /* 3, virtual register, to speedup access */
    VIR_OPND_SAMPLER_INDEXING, /* 4, for sampler parameter passing:
                                   samplerArray[i] => Sampler */
    VIR_OPND_PARAMETERS, /* 5, parameter passing list */
    VIR_OPND_TEXLDPARM, /* 6, texld/imgld parameters */
    VIR_OPND_ARRAY, /* 7, high level: array indexing */
    VIR_OPND_FIELD, /* 8, high level: field access */
    VIR_OPND_LABEL, /* 9 */
    VIR_OPND_FUNCTION, /* 10, function pointer */
    VIR_OPND_INTRINSIC, /* 11, intrinsic id */
    VIR_OPND_IMMEDIATE, /* 12, scalar const value */
    VIR_OPND_CONST, /* 13, vector const value */
    VIR_OPND_ADDRESS_OF, /* 14, address of a symbol */
    VIR_OPND_UNUSED, /* 15, the operand node is in free list */
    VIR_OPND_EVIS_MODIFIER, /* 16, evis instruction modifier */
    /* max 31, need to enlarge operand kind field width when exceeding 31 */
} VIR_OperandKind;

#define VIR_OperandKind_isHighLevel(Kind)  ((Kind) == VIR_OPND_ARRAY || (Kind) == VIR_OPND_FIELD)

enum _VX_FilterMode
{
    VX_FM_BOX      = 0,
    VX_FM_Guassian = 1,
    VX_FM_SobelX   = 2,
    VX_FM_SobelY   = 3,
    VX_ScharrX     = 4,
    VX_FM_ScharrY  = 5,
    VX_FM_Max      = 8,
    VX_FM_Min      = 9,
    VX_FM_Median   = 10
};
typedef gctUINT VX_FilterMode;

typedef enum _VX_RoundMode
{
    VX_RM_TowardZero    = 0,
    VX_RM_TowardInf     = 1,
    VX_RM_ToNearestEven = 2
} VX_RoundMode;

typedef enum _VX_SRCFORMAT
{
    VX_FMT_FP32  = 0,
    VX_FMT_FP16  = 1,
    VX_FMT_S32   = 2,
    VX_FMT_S16   = 3,
    VX_FMT_S8    = 4,
    VX_FMT_U32   = 5,
    VX_FMT_U16   = 6,
    VX_FMT_U8    = 7
} VX_SrcFormat;

typedef union _VIR_EVIS_MODIFIER
{
    struct {
        gctUINT         startBin      : 4;
        gctUINT         endBin        : 4;
        gctUINT         sourceBin     : 4;
        gctUINT         filterMode    : 4;
        gctUINT         roundingMode  : 2;
        gctUINT         signExt       : 1;
        gctUINT         preAdjust     : 1;
        gctUINT         rangePi       : 1;
        gctUINT         enableBool    : 1;
        gctUINT         src0Format    : 3;
        gctUINT         src1Format    : 3;
        gctUINT         src2Format    : 3;
    } u0;
    gctUINT   u1;
} VIR_EVIS_Modifier;

typedef union _VIR_EVIS_STATE
{
    gctUINT    u1;
    struct {
        gctUINT     roundingMode  : 2;
    } u2;  /* AbsDiff */
    struct {
        gctUINT     src0Format  : 3;
        gctUINT     src1Format  : 3;
        gctUINT     src2Format  : 3;
    } u3;  /* IAdd */
    struct {
        gctUINT     src1Format  : 3;
        gctUINT     signExt     : 1;
    } u4;  /* IAccSq */
    struct {
        gctUINT     src0Format  : 3;
        gctUINT     src1Format  : 3;
    } u5;  /* Lerp */
    struct {
        gctUINT     srcFormat   : 3;
        gctUINT     filter      : 4;
    } u6;  /* Lerp */
    struct {
        gctUINT     srcFormat      : 3;
        gctUINT     disablePreAdj  : 1;
        gctUINT     rangePi        : 1;
    } u7;  /* MagPhase */
    struct {
        gctUINT     srcFormat0  : 3;
        gctUINT     srcFormat1  : 3;
        gctUINT     rounding    : 1;  /* RTNE if set */
    } u8; /* MullShift */
    struct {
        gctUINT     srcFormat0  : 3;
        gctUINT     srcFormat1  : 3;
        gctUINT     rounding    : 2;
    } u9; /* DP 16 instructions */
    struct {
        gctUINT     srcFormat01 : 3;
        gctUINT     rounding    : 2;
        gctUINT     srcFormat1  : 3;
    } u10; /* DP 32 instrucitons */
    struct {
        gctUINT     srcFormat   : 3;
        gctUINT     enableBool  : 2;
    } u11; /* clamp */
    struct {
        gctUINT     srcFormat01 : 3;
        gctUINT     srcFormat2  : 3;
    } u12; /* BiLinear */
    struct {
        gctUINT     srcFormat0  : 3;
        gctUINT     srcFormat1  : 3;
    } u13; /* SelectAdd */
    struct {
        gctUINT     srcFormat2  : 3;
    } u14; /* AtomicAdd */
    /* bitextract and bitReplace have no state bits */
} VIR_EVIS_State;

struct _VIR_PARMPASSING
{
    VIR_SymId           arg;        /* argument value in temp register */
    VIR_SymId           parm;       /* corresponding parameter */
    VIR_ParmPassing *   next;       /* next parameter */
};


struct _VIR_OPERANDLIST
{
    VIR_Operand *           value;  /* argument value in temp register */
    VIR_OperandList *       next;   /* next parameter */
};

typedef gctUINT   VIR_PhiStatus;

typedef struct _VIR_PHIVALUELIST VIR_PhiValueList;
struct _VIR_PHIVALUELIST
{
    VIR_Operand *           value;  /* argument value in temp register */
    VIR_Label *             label;  /* label operand, branch target */
    VIR_PhiStatus           flag;
    VIR_PhiValueList *      next;   /* next phi value pair */
} ;

typedef enum _VIR_REGCLASS
{
    VIR_RC_TEMP_MP, /* medium precision temp register */
    VIR_RC_TEMP_HP, /* high precision temp register */
    VIR_RC_UNBOUNDED_CONST, /* unbounded constant register */
    VIR_RC_BOUNDED_CONST, /* bounded constant register */
    VIR_RC_LOCAL_STORAGE, /* local storage for each thread, low laterncy */
    VIR_RC_SPECIAL, /* special registers: a0/al/vertexId/instanceId, etc. */
} VIR_RegClass;

typedef enum _VIR_HWREG_NAME
{

    /* special registers */
    VIR_SR_Begin     = 512,
    VIR_SR_INSTATNCEID = VIR_SR_Begin, /* instance id register */
    VIR_SR_VERTEXID, /* vertex id register */
    VIR_SR_FACE, /* face register */
    VIR_SR_A0, /* special index register, 4 components */
    VIR_SR_AL, /* loop register */
    VIR_SR_NEXTPC, /* next PC address */
    VIR_REG_MULTISAMPLEDEPTH, /* not accessed by shader program, automatically
                                   mapped to last register of the shader */
    VIR_REG_SAMPLE_POS,
    VIR_REG_SAMPLE_ID,
    VIR_REG_SAMPLE_MASK_IN,
    VIR_HwReg_Invalid,
    VIR_SR_End,
} VIR_HwReg_Name;

typedef struct _VIR_OPERAND_HEADER VIR_Operand_Header;

struct _VIR_OPERAND_HEADER
{
    gctUINT                 _opndKind     : 5;
    VIR_OperandId           _index        : 20;  /* entry index of this operand */
    gctUINT                 _lvalue       : 1;   /* if the operand is LValue */
    gctUINT                 _roundMode    : 3;
    gctUINT                 _modifier     : 3;   /* dest: saturate modifier
                                                   src:  negate/absolute */
};

typedef enum _VIR_TEXMODIFIER_FLAG
{
    VIR_TMFLAG_NONE    = 0x00,
    VIR_TMFLAG_BIAS    = 0x01,
    VIR_TMFLAG_LOD     = 0x02,
    VIR_TMFLAG_GRAD    = 0x04,
    VIR_TMFLAG_OFFSET  = 0x08,
    VIR_TMFLAG_GATHER  = 0x10,
    VIR_TMFLAG_FETCHMS = 0x20,
} VIR_TexModifier_Flag;

/* header for texld modifier operand */
typedef struct _VIR_OPERAND_HEADER_TM
{
    VIR_OperandKind         _opndKind     : 5;
    VIR_OperandId           _index        : 20;  /* entry index of this operand */
    gctUINT                 _texmodifiers : 7;   /* texture modifier */
} VIR_Operand_Header_TM;

typedef enum _VIR_OPND_INDEXINGKIND
{
    VIR_OPND_NOINDEXING         = 0,
    VIR_OPND_SAMPLERINDEXING    = 1,
    VIR_OPND_TEXUREINDEXING     = 2,
} VIR_Opnd_IndexingKind;

typedef struct _VIR_OPERANDINFO
{
    VIR_Operand *   opnd;
    union {
        struct {
            VIR_VirRegId    virReg;         /* the virReg used in the operand */
            VIR_VirRegId    startVirReg;    /* start virReg of correponding variable */
            gctUINT         virRegCount;    /* virReg count of correponding variable:
                                             * so the virReg range is:
                                             *  [startVirReg, startVirReg + virRegCount - 1]
                                             */
        } virRegInfo;
        VIR_ScalarConstVal  immValue;
        VIR_ConstId         vecConstId;
        gctUINT             uniformIdx;
    } u1;
    gctUINT         isArray  : 1;   /* is corresponding variable array */
    gctUINT         isInput  : 1;   /* does the the operand access input variable? */
    gctUINT         isOutput : 1;   /* does the the operand access output variable? */
    gctUINT         isImmVal : 1;   /* Is this operand an immediate value */
    gctUINT         isVecConst: 1;  /* is vector const */
    gctUINT         isUniform: 1;   /* is uniform */
    gctUINT         isTempVar: 1;   /* is temp variable which has no coresponding
                                     * user defined variable */
    gctUINT         needHwSpecialDef : 1;   /* does the the operand need hw special def? */
    VIR_Opnd_IndexingKind indexingKind : 4;  /* e.g.: sampler indexing: sampler(0+temp(10).x)
                                              * virreg(10) info is returned */
    gctUINT         halfChannelMask : 2; /* For non-dual16, must set it to VIR_HALF_CHANNEL_MASK_FULL */
} VIR_OperandInfo;

typedef enum _VIR_TEXLDMODIFIER_NAME
{
    VIR_TEXLDMODIFIER_BIAS      = 0,
    VIR_TEXLDMODIFIER_LOD       = 1,
    VIR_TEXLDMODIFIER_FETCHMS_SAMPLE = 1, /* shared with modifier for lod as LOD is always at base level */
    VIR_TEXLDMODIFIER_DPDX      = 2,
    VIR_TEXLDMODIFIER_DPDY      = 3,
    VIR_TEXLDMODIFIER_GATHERCOMP= 2, /* GATHERCOMP, GATHERREFZ shared with modifier for TEXGRAD DPDX and DPDY */
    VIR_TEXLDMODIFIER_GATHERREFZ= 3,
    VIR_TEXLDMODIFIER_OFFSET    = 4,
    VIR_TEXLDMODIFIER_COUNT
} Vir_TexldModifier_Name;

typedef struct _VIR_OPERAND_TEXLDMODIFIER
{
    VIR_Operand_Header      header;
    VIR_Operand *           tmodifier[VIR_TEXLDMODIFIER_COUNT];
} VIR_Operand_TexldModifier;

struct _VIR_OPERAND
{
    VIR_Operand_Header      header;
    VIR_TypeId              _opndType       : 20;  /* operand type, derived from
                                                   tempRegister, variable, etc. */
    gctUINT                 _swizzleOrEnable: 8;
    gctUINT                 _precision      : 2; /* operand precision */
    gctUINT                 _reserved1      : 2; /* not used bits */

    /* hardware register info */
    gctUINT                 _regAllocated : 1;   /* hardware physical register allocated */
    VIR_HwRegId             _hwRegId      : 10;  /* hardware physical register index */
    gctUINT                 _hwShift      : 2;   /* the shift amount of the hwReg
                                                 *   temp(9).y shift 2 becomes r1.w
                                                 *      if hwRegIndex == 1
                                                 *   temp(9).xy shift 1 becomes r1.yz
                                                 */
    gctUINT                 _HIhwRegId    : 10;  /* in dual16, a pair of register is needed */
    gctUINT                 _HIhwShift    : 2;   /* in dual16, a pair of register is needed */
    gctUINT                 _reserved2    : 7;   /* not used bits */

    union
    {
        VIR_Symbol *        sym;        /* temp, variable, uniform, sampler,
                                           local storage, addressOf ... */
        VIR_ParmPassing *   argList;    /* arg value assigned to parameters */
        gctFLOAT            fConst;     /* float constant value */
        gctINT              iConst;     /* int constant value */
        gctUINT             uConst;     /* unsigned constant value */
        VIR_EVIS_Modifier   evisModifier;/* evis instruction modifier */
        VIR_ConstId         constId;    /* vector constant */
        VIR_Label *         label;      /* label operand, branch target */
        VIR_Function *      func;       /* callee, call target */
        VIR_IntrinsicsKind  intrinsic;
        VIR_Operand *       base;       /* base expression for array indexing
                                         * or field access */
        VIR_PhiValueList *  phiValues;
    } u1;
    union
    {
        /* VL (constIndexed is medium level concept), relative addressing:
           can be translated from ARRAY operand */
        struct
        {
            /* relative addressing info: TEMP, UNIFORM, VARIABLE, ATTRIBUTE,
               OUTPUT, SAMPLER, ADDRESSOF */
            gctUINT         _isConstIndexing  : 1;  /* is the relIndex const number */
            gctUINT         _relAddrMode      : 3;  /* relative address mode */
            gctUINT         _matrixConstIndex : 2;  /* matrix operand can be indexed with
                                                       const value 0-3 */
            gctINT          _relIndexing      : 20; /* symbol id used in indexing,
                                                      or 20bit immediate integer
                                                      if isConstIndexing */
            gctUINT         _isSymLocal       : 1;   /* keep the symbol's scope info */
            gctUINT         _relAddrLevel     : 2;   /* relative address level. */
        } vlInfo;
        /* HL, will be lowered to relAddr */
        VIR_OperandList *   arrayIndex;         /* array index list, support multi-dim array:
                                                 *  base[arrayIndex->value]
                                                 *      [arrayindex->next->value]
                                                 *      ...
                                                 */
        VIR_SymId           fieldId;            /* base->field, offset can be obtained from
                                                 * VIR_GetOffset(opndTye, fieldId)
                                                 */
    } u2;

    /* These are ONLY used under SSA form because under SSA form, we don't have extra structures
       to maitain simple single DU chain. */
    VIR_Operand *           nextUse;            /* next use of the operand */
    VIR_Instruction *       defInst;            /* the instruction define this operand */

};

#define VIR_Operand_isHighLevel(Opnd)  VIR_OperandKind_isHighLevel(VIR_Operand_GetOpKind(Opnd))

/* symbol for: pseudo register, uniform, face, instanceID, vertexID, vaiable,
               attribute, output, sampler */

typedef enum _VIR_OPERANDNO
{
    VIR_Operand_Src0,
    VIR_Operand_Src1,
    VIR_Operand_Src2,
    VIR_Operand_Src3,
} VIR_OperandNo;

typedef enum _VIR_INSTFLAG
{
    VIR_INSTFLAG_NONE       = 0x00,
}
VIR_InstFlag;

typedef enum _VIR_THREADMODE
{
    VIR_THREAD_SINGLE32       = 0x00, /* single thread float/int 32 bit */
    VIR_THREAD_D16_DUAL_16    = 0x00, /* dual threads, each thread float/int 16 bit */
    VIR_THREAD_D16_DUAL_32    = 0x01, /* dual threads, each thread float/int 32 bit,
                                           * HW not support this yet, need to expand T0 & T1 */
    VIR_THREAD_D16_SINGLE_T0  = 0x02, /* dual16, thread 0 execute in 32 bit mode */
    VIR_THREAD_D16_SINGLE_T1  = 0x03      /* dual16, thread 1 execute in 32 bit mode */
} VIR_ThreadMode;

/* must be the same order as VSC_BI_LIST_NODE */
typedef struct _VIR_INSTHEADER
{
    VIR_Instruction * pPrevNode;
    VIR_Instruction * pNextNode;
} VIR_InstHeader;

#define VIR_MAX_SRC_NUM       5
struct _VIR_INSTRUCTION
{
    /* Base bi-link list node. It must be put at FIRST place!!!!! */
    VIR_InstHeader          biLstNode;
    union {
        VIR_BB *            BB;
        VIR_Function *      function;   /* the containing function */
    } parent;
    VIR_SourceFileLoc       sourceLoc;
    gctINT                  id_         : 20; /* the id of the instruction */
    gctUINT                 _isPrecise  : 1;  /* the inst is precise */

    VIR_OpCode              _opcode     : 9;
    VIR_TypeId              _instType   : 8; /* HW supported type */
    gctUINT                 _condOp     : 5;
    gctUINT                 _instFlag   : 3; /* VL concept, etc */
    gctUINT                 _srcOpndNum : 3; /* number of source operands */
    gctUINT                 _threadMode : 2;
    gctUINT                 _parentUseBB: 1; /* parent union uses BB */
    gctBOOL                 _patched    : 1; /* some hardware opcodes have bug, we need patch to avoid the bug. */
    gctUINT                 _dual16ExpandSeq;

    VIR_Operand *           dest;           /* dest operands*/
    VIR_Operand *           src[VIR_MAX_SRC_NUM];     /* source operands (up to 3
                                                         normal source + 1 sampler) */

    void *                  mcInst;
    gctUINT                 mcInstCount;
};

/* inst source operand iterator */
typedef struct _VIR_SRCOPERAND_ITERATOR
{
    VIR_Instruction *       inst;
    gctUINT                 speicalNode : 1;    /* the node need special handling */
    gctUINT                 useOpndList : 1;    /* the special node use operand list */
    gctUINT                 curSrcNo;
    gctUINT                 inSrcNo;    /* some operands may contain multiple
                                           operands itself */
    VIR_OperandList *       curNode;    /* array operation is a list of operands */
} VIR_SrcOperand_Iterator;

/* inst operand iterator */
typedef struct _VIR_OPERAND_ITERATOR
{
    VIR_SrcOperand_Iterator header;
    Vir_TexldModifier_Name  texldModifierName;
    gctBOOL                 dest;
    gctUINT                 curNo;
} VIR_Operand_Iterator;

void VIR_SrcOperand_Iterator_Init(VIR_Instruction *Inst, VIR_SrcOperand_Iterator *Iter);
VIR_Operand * VIR_SrcOperand_Iterator_First(VIR_SrcOperand_Iterator *Iter);
VIR_Operand * VIR_SrcOperand_Iterator_Next(VIR_SrcOperand_Iterator *Iter);

void          VIR_Operand_Iterator_Init(VIR_Instruction *Inst, VIR_Operand_Iterator *Iter);
VIR_Operand * VIR_Operand_Iterator_First(VIR_Operand_Iterator *Iter);
VIR_Operand * VIR_Operand_Iterator_Next(VIR_Operand_Iterator *Iter);

VIR_Function * VIR_Inst_GetCallee(VIR_Instruction *);
VIR_BB *       VIR_Inst_GetBranchTargetBB(VIR_Instruction *);

/******************************************************************************\
|*********************************** SHADERS **********************************|
\******************************************************************************/

/* Structure that defines a uniform block for a shader. */
struct _VIR_UNIFORMBLOCK
{
    VIR_SymId           sym;    /* the symbol for this uniform block */

    /* Uniform block index */
    gctINT16            blockIndex;

    /* symbol id of the uniform to keep the base address. */
    VIR_SymId           baseAddr;

    /* block size in bytes */
    gctUINT32           blockSize;

    /* number of uniforms in block */
    gctUINT32           uniformCount;

    /* array of pointers to uniforms in block */
    VIR_Uniform **       uniforms;
};


/* Structure that defines an uniform (constant register) for a shader. */
struct _VIR_UNIFORM
{
    /* compatible with gcUniform */
    gcsOBJECT               object;
    VIR_UniformId           index : 16 ;   /* uniform index number: uniform(10) */
    gctINT16                gcslIndex ;    /* corresponding glsl uniform's index */

    /* A conservative uniform indexing range check.
       NOTE: we should remove this after using better algorithm for constant RA. Because
             current implementation is same as old one, we must use this WAR as VIR has
             no structure hierarchy maintainment as old gcSL
    */
    gctINT16                lastIndexingIndex;

    gctINT16                blockIndex;    /* Uniform block index: Default = -1 */
    gctUINT16               glUniformIndex;/* Corresponding Index of Program's GLUniform */
    gctUINT16               imageSamplerIndex;    /* Index to image sampler for OpenCL */

    gctINT16                matchIndex;    /* If a uniform is used on both VS and PS,
                                            * the index of this uniform on the other
                                            * shader would be saved by this.
                                            */
    gcSHADER_VAR_CATEGORY   _varCategory : 8;
    /* Physically assigned values. */
    gctUINT8                swizzle;
    gctUINT32               address;
    /* Physical register index that from gcUNIFORM. */
    gctINT                  origPhysical;
    gctINT                  physical;

    VIR_Uniform            *baseBindingUniform;

    /* offset from uniform block's base address */
    gctINT32                offset;

    gctINT32                realUseArraySize;

    /*gceUNIFORM_FLAGS        _flags;*/

    union
    {
        VIR_ConstId         initializer;    /* Compile-time constant value, */
        struct
        {
            VIR_Symbol     *lodMinMax;
            VIR_Symbol     *levelBaseSize;
        } samplerOrImageAttr;
        VIR_Uniform        *parentSampler;
    } u;

    VIR_SymId           sym;    /* the symbol for this uniform */
};

/* Structure that defines a stroage block for a shader. */
struct _VIR_STORAGEBLOCK
{
    VIR_SymId           sym;    /* the symbol for this uniform block */

    /* Uniform block index */
    gctINT16            blockIndex;

    /* symbol id of the uniform to keep the base address. */
    VIR_SymId           baseAddr;

    /* block size in bytes */
    gctUINT32           blockSize;

    /* number of variables in block */
    gctUINT32           variableCount;

    /* array of pointers to variables in block */
    VIR_Symbol *        variables;
};

/* Structure that defines a io block for a shader. */
struct _VIR_IOBLOCK
{
    /* the symbol for this io block */
    VIR_SymId           sym;

    /* IO block index of gcSL shader. */
    gctINT              blockIndex;

    /* The length of block name. */
    gctINT              blockNameLength;

    /* The length of instance name. */
    gctINT              instanceNameLength;

    /* Input/Output IO block. */
    VIR_StorageClass    Storage;
};

#define VIR_IOBLOCK_GetSymId(IOBlock)                   ((IOBlock)->sym)
#define VIR_IOBLOCK_SetSymId(IOBlock, s)                (VIR_IOBLOCK_GetSymId(IOBlock) = s)
#define VIR_IOBLOCK_GetBlockIndex(IOBlock)              ((IOBlock)->blockIndex)
#define VIR_IOBLOCK_SetBlockIndex(IOBlock, s)           (VIR_IOBLOCK_GetBlockIndex(IOBlock) = s)
#define VIR_IOBLOCK_GetBlockNameLength(IOBlock)         ((IOBlock)->blockNameLength)
#define VIR_IOBLOCK_SetBlockNameLength(IOBlock, s)      (VIR_IOBLOCK_GetBlockNameLength(IOBlock) = s)
#define VIR_IOBLOCK_GetInstanceNameLength(IOBlock)      ((IOBlock)->instanceNameLength)
#define VIR_IOBLOCK_SetInstanceNameLength(IOBlock, s)   (VIR_IOBLOCK_GetInstanceNameLength(IOBlock) = s)
#define VIR_IOBLOCK_GetStorage(IOBlock)                 ((IOBlock)->Storage)
#define VIR_IOBLOCK_SetStorage(IOBlock, s)              (VIR_IOBLOCK_GetStorage(IOBlock) = s)
#define VIR_IOBLOCK_GetFirstChild(IOBlock)              ((IOBlock)->firstChild)
#define VIR_IOBLOCK_SetFirstChild(IOBlock, s)           (VIR_IOBLOCK_GetFirstChild(IOBlock) = s)

typedef enum _VIR_FUNCTIONFLAG
{
  VIR_FUNCFLAG_NOATTR                   = 0x00,
  VIR_FUNCFLAG_INTRINSICS               = 0x01, /* Function is openCL/OpenGL builtin function */
  VIR_FUNCFLAG_ALWAYSINLINE             = 0x02, /* Always inline */
  VIR_FUNCFLAG_NOINLINE                 = 0x04, /* Neve inline */
  VIR_FUNCFLAG_INLINEHINT               = 0x08, /* Inline is desirable */

  VIR_FUNCFLAG_READNONE                 = 0x10, /* Function does not access memory */
  VIR_FUNCFLAG_READONLY                 = 0x20, /* Function only reads from memory */
  VIR_FUNCFLAG_STRUCTRET                = 0x40, /* Hidden pointer to structure to return */
  VIR_FUNCFLAG_NORETURN                 = 0x80, /* Function is not returning */

  VIR_FUNCFLAG_INREG                    = 0x100, /* Force argument to be passed in register */
  VIR_FUNCFLAG_BYVAL                    = 0x200, /* Pass structure by value */
  VIR_FUNCFLAG_KERNEL                   = 0x400, /* OpenCL Kernel function */
  VIR_FUNCFLAG_RECURSIVE                = 0x800, /* is recursive function */
  VIR_FUNCFLAG_MAIN                     = 0x1000, /* is main function */
  VIR_FUNCFLAG_STATIC                   = 0x2000, /* static function */
  VIR_FUNCFLAG_EXTERN                   = 0x4000, /* extern function with no body */
  VIR_FUNCFLAG_NAME_MANGLED             = 0x8000, /* name mangled */
  VIR_FUNCFLAG_RECOMPILER               = 0x10000, /* A recompile function. */
  VIR_FUNCFLAG_RECOMPILER_STUB          = 0x20000, /* The function to stub a recompile function. */
  VIR_FUNCFLAG_HAS_SAMPLER_INDEXINED    = 0x40000, /* This function has sampler indexing used. */
} VIR_FunctionFlag;

typedef struct _VIR_KERNELPROPERTY
{
    gctINT              propertyType;
    gctUINT32           propertySize;
} VIR_KernelProperty;

typedef struct _VIR_IMAGESAMPLER
{
    /* Kernel function argument # associated with the image passed to the
     * kernel function
     */
    gctUINT8            imageNum;

    /* Sampler type either passed in as a kernel function argument which will
     * be an argument # or defined as a constant variable inside the program
     * which will be an unsigend integer value
     */
    gctBOOL             isConstantSamplerType : 8;

    gctUINT32           samplerType;
} VIR_ImageSampler;

typedef struct _VIR_KERNELINFO
{
    /* Local address space size */
    gctUINT32           localMemorySize;

    /* Uniforms Args */
    VIR_UniformIdList   uniformArguments;
    gctINT              samplerIndex;

    /* Image-Sampler associations */
    VIR_ValueList       imageSamplers;  /* a list of VIR_ImageSampler */

    /* Kernel function properties */
    VIR_ValueList       properties;     /* a list of VIR_KernelProperty */
    VIR_IdList          propertyValues;

    VIR_NameId          kernelName;     /* keep the name of the kernel
                                         * after it is merged with main */
    gctBOOL             isMain;
} VIR_KernelInfo;

struct _VIR_FUNCTION
{
    /* A bi-link list of instructions that this function maintains */
    VIR_InstList        instList;

    /* private date */
    gctINT              _lastInstId;

    gctUINT             _labelId;       /* label id for new added labels */

    VIR_Shader *        hostShader;
    VIR_SymId           funcSym;

    VIR_SymTable        symTable;
    VIR_LabelTable      labelTable;
    VIR_OperandTable    operandTable;   /* operands used in instructions */

    VIR_FunctionFlag    flags;
    gctUINT             maxCallDepth;   /* the max call stack depth required
                                           by the function */

    /* Local variables. */
    VIR_VariableIdList  localVariables;
    /* Parameters */
    VIR_VariableIdList  paramters;
    /* Temp Registers */
    VIR_VirRegIdList    temps;

    /* kernel info */
    VIR_KernelInfo *    kernelInfo;

    /* for converting from/to gcSL function */
    gctINT              tempIndexStart;
    gctINT              tempIndexCount;

    /* Pointer to FunctionBlock which used to build call graph of whole shader */
    VIR_FB*             pFuncBlock;
};


typedef struct _VIR_LINK  VIR_Link;

/* Structure defining a linked references for a label. */
struct _VIR_LINK
{
    VIR_Link *          next;
    gctUINTPTR_T        referenced;
};

/* Structure defining a label. */
 struct _VIR_LABEL
{
    VIR_LabelId         index;
    VIR_SymId           sym;        /* symbol id for the label */
    VIR_Instruction *   defined;    /* the instruction defines the label */
    VIR_Link *          referenced;
};

typedef struct _VIR_VARTEMPREGINFO
{
    VIR_SymId           variable;
    gctUINT32           streamoutSize;  /* size to write on feedback buffer */
    gctINT              tempRegCount;   /* number of temp register assigned
                                           to this variable */
    VIR_TypeId *        tempRegTypes;   /* the type for each temp reg */
}
VIR_VarTempRegInfo;

typedef enum _VIR_FEEDBACKBUFFERMODE
{
    VIR_FEEDBACK_INTERLEAVED        = 0x00,
    VIR_FEEDBACK_SEPARATE           = 0x01
} VIR_FeedbackBufferMode;

typedef struct _VIR_TRANSFORMFEEDBACK
{
    /* pointer to varyings to be streamed out */
    VIR_SymIdList *                 varyings;

    VIR_FeedbackBufferMode          bufferMode;
    /* driver set to 1 if transform feedback is active and not
       paused, 0 if inactive or paused */
    VIR_Uniform *                   stateUniform;
    /* the temp register info for each varying */
    VIR_ValueList *                 varRegInfos;
    union {
        /* array of uniform for separate transform feedback buffers */
        VIR_Uniform **              separateBufUniforms;
        /* transfom feedback buffer for interleaved mode */
        VIR_Uniform *               interleavedBufUniform;
    } feedbackBuffer;
    gctINT                          shaderTempCount;
    /* total size to write to interleaved buffer for one vertex */
    gctUINT32                       totalSize;
}
VIR_TransformFeedback;

typedef struct _VIR_BUILTINSTEMPINDEX
{
   gctINT       PositionTempIndex;
   gctINT       PointSizeTempIndex;
   gctINT       ColorTempIndex;
   gctINT       FrontFacingTempIndex;
   gctINT       PointCoordTempIndex;
   gctINT       PositionWTempIndex;
   gctINT       DepthTempIndex;
   gctINT       FogCoordTempIndex;
   gctINT       InstanceIDTempIndex;
   gctINT       VertexIDTempIndex;
} VIR_BuiltinsTempIndex;

typedef struct _VIR_SHADERCODEINFO
{
    gctUINT                 codeCounter[VIR_OP_MAXOPCODE];
    VIR_BuiltinsTempIndex   builtinsTempIndex;
    gctBOOL                 hasUnsupportDual16Inst;
    gctBOOL                 hasLoop;
    gctBOOL                 hasBranch;
    gctBOOL                 hadBiasedTexld;
    gctBOOL                 hasLodTexld;
    gctBOOL                 useInstanceID;
    gctBOOL                 useVertexID;
    gctBOOL                 hasFragDepth;      /* has glFragDepth in PS */
    gctBOOL                 usePosition;       /* use glFragCoord in PS */
    gctBOOL                 useFace;           /* use gl_FrontFacing in PS */
    gctBOOL                 useSubsampleDepth; /* inplicit subsample depth register */
    gctBOOL                 srcUseLocalStorage;
    gctBOOL                 destUseLocalStorage;
    gctBOOL                 useHighPrecision;  /* true if use high precision data
                                                  other than position */
    gctBOOL                 hasInt32OrUint32;  /* true if instruction has int32 or
                                                  uint32 operands*/
    gctINT                  effectiveTexld;    /* the estimated effective
                                                  dynamic texld count */
    gctINT                  estimatedInst;     /* estimated GPU instruction count */
    gctINT                  runSignleTInstCount;  /* instructions ONLY run at single T in dual16 mode
                                                   * need to count them as two instructions */
    gctINT                  highPInstCount;      /* high precision instruction count */
} VIR_ShaderCodeInfo;


/* cached shader/program data base
 *
 *   VS: pre-linked vertex shader
 *   FS: pre-linked fragment shader
 *   PG: linked program
 *   DP: dynamic patch info
 *
 */

typedef enum _VIR_SHADERFLAGS
{
    VIR_SHFLAG_OLDHEADER        = 0x01, /* the old header word 5 is gcdSL_IR_VERSION,
                                              which always be 0x01 */
    VIR_SHFLAG_OPENVG           = 0x02, /* the shader is an OpenVG shader */
    VIR_SHFLAG_HAS_UNSIZED_SBO  = 0x04, /* the shader has unsized array of Storage Buffer Object */
    VIR_SHFLAG_SEPARATED        = 0x08, /* the shader is a SSO */

    /* common code characteristic flag */
    VIR_SHFLAG_HAS_INSTANCEID   = 0x100, /* whether the shader has instance id */
    VIR_SHFLAG_HAS_PRIMITIVEID  = 0x200, /* whether the shader has primitive id */
    VIR_SHFLAG_HAS_SAMPLER_INDEXING  = 0x400, /* whether the shader has sampler indexing */
    VIR_SHFLAG_HAS_OUTPUT_ARRAY_HIGHP  = 0x1000, /* whether the shader has output array that is highp */
    VIR_SHFLAG_HAS_BARRIER      = 0x2000, /* whether the shader has barrier */
    VIR_SHFLAG_HAS_32BITMODULUS = 0x4000, /* whether the shader 32 bit modulus */

    /* shader specific code characteristic flag */
    VIR_SHFLAG_GS_HAS_STREAM_OUT  = 0x10000, /* whether the shader has stream out, only in GS */
    VIR_SHFLAG_GS_HAS_RESTART_OP  = 0x20000, /* whether the shader has restart op, only in GS */
    VIR_SHFLAG_PS_NEED_SAMPLE_MASK_ID = 0x40000, /* whether the shader need sample-mask (in & out), sample_id and
                                                   sample_pos (it is got from sample_id), only in PS */
    VIR_SHFLAG_PS_SPECIALLY_ALLOC_POINT_COORD = 0x80000, /* Alloc pt-coord at (last - 1) among all attributes of ps */
    VIR_SHFLAG_PS_SPECIALLY_ALLOC_PRIMTIVE_ID = 0x100000, /* Alloc primId at last among all attributes of ps */
    VIR_SHFLAG_PS_SAMPLE_SHADING = 0x200000, /* ps runs on sample-shading mode */
    VIR_SHFLAG_PS_NEED_ALPHA_KILL_PATCH = 0x400000, /* ps needs alpha-kill patch */

    /* common shader control flag */

    /* shader specific control flag */
    VIR_SHFLAG_TCS_USE_DRIVER_INPUT   = 0x1000000, /* whether the shader's input vertex count
                                                      coming from driver, only affect TCS */
    VIR_SHFLAG_TCS_USE_PACKED_REMAP   = 0x2000000, /* whether the shader's input remap and output
                                                      remap are packed in one register, only affect TCS */

} VIR_ShaderFlags;

#define VIR_Shader_GetFlags(Shader)                 (Shader)->flags)
#define VIR_Shader_SetFlags(Shader, Flags)          do { (Shader)->flags = (Flags); } while (0)

#define VIR_Shader_SetFlag(Shader, Val)             do {(Shader)->flags |= (Val); } while (0)
#define VIR_Shader_ClrFlag(Shader, Val)             do {(Shader)->flags &= ~(Val); } while (0)

#define VIR_Shader_IsOldHeader(Shader)              (((Shader)->flags & VIR_SHFLAG_OLDHEADER) != 0)
#define VIR_Shader_IsOpenVG(Shader)                 (((Shader)->flags & VIR_SHFLAG_OPENVG) != 0)
#define VIR_Shader_HasUnsizedSBO(Shader)            (((Shader)->flags & VIR_SHFLAG_HAS_UNSIZED_SBO) != 0)
#define VIR_Shader_IsSeparated(Shader)              (((Shader)->flags & VIR_SHFLAG_SEPARATED) != 0)
#define VIR_Shader_HasInstanceId(Shader)            (((Shader)->flags & VIR_SHFLAG_HAS_INSTANCEID) != 0)
#define VIR_Shader_HasPrimitiveId(Shader)           (((Shader)->flags & VIR_SHFLAG_HAS_PRIMITIVEID) != 0)
#define VIR_Shader_HasSamplerIndexing(Shader)       (((Shader)->flags & VIR_SHFLAG_HAS_SAMPLER_INDEXING) != 0)
#define VIR_Shader_HasOutputArrayHighp(Shader)      (((Shader)->flags & VIR_SHFLAG_HAS_OUTPUT_ARRAY_HIGHP) != 0)
#define VIR_Shader_HasBarrier(Shader)               (((Shader)->flags & VIR_SHFLAG_HAS_BARRIER) != 0)
#define VIR_Shader_Has32BitModulus(Shader)          (((Shader)->flags & VIR_SHFLAG_HAS_32BITMODULUS) != 0)

/* let the client make sure the shaderKind is right.
   Otherwise, it is wrong when the flag is used in !flag case. */
#define VIR_Shader_GS_HasStreamOut(Shader)          (((Shader)->flags & VIR_SHFLAG_GS_HAS_STREAM_OUT) != 0)
#define VIR_Shader_GS_HasRestartOp(Shader)          (((Shader)->flags & VIR_SHFLAG_GS_HAS_RESTART_OP) != 0)
#define VIR_Shader_PS_NeedSampleMaskId(Shader)      (((Shader)->flags & VIR_SHFLAG_PS_NEED_SAMPLE_MASK_ID) != 0)
#define VIR_Shader_PS_NeedSpecAllocPrimId(Shader)   (((Shader)->flags & VIR_SHFLAG_PS_SPECIALLY_ALLOC_PRIMTIVE_ID) != 0)
#define VIR_Shader_PS_NeedSpecAllocPtCoord(Shader)  (((Shader)->flags & VIR_SHFLAG_PS_SPECIALLY_ALLOC_POINT_COORD) != 0)
#define VIR_Shader_PS_RunOnSampleShading(Shader)    (((Shader)->flags & VIR_SHFLAG_PS_SAMPLE_SHADING) != 0)
#define VIR_Shader_PS_NeedAlphaKillPatch(Shader)    (((Shader)->flags & VIR_SHFLAG_PS_NEED_ALPHA_KILL_PATCH) != 0)
#define VIR_Shader_TCS_UseDriverInput(Shader)       (((Shader)->flags & VIR_SHFLAG_TCS_USE_DRIVER_INPUT) != 0)
#define VIR_Shader_TCS_UsePackedRemap(Shader)       (((Shader)->flags & VIR_SHFLAG_TCS_USE_PACKED_REMAP)!= 0)

typedef struct _VIR_LIBRARYLIST VIR_LibraryList;

struct _VIR_LIBRARYLIST
{
    VIR_Shader *        lib;
    VIR_LibraryList *     next;
};

typedef struct _VIR_CONTEXT
{
    VSC_MM              mempool;    /* global memory pool, shared by shaders
                                     * in one program */
    VIR_TypeTable *     typeTable;  /* global type table, shared by shaders
                                     * in one program */
} VIR_Context;

typedef enum _VIR_SH_LEVEL
{
    VIR_SHLEVEL_Unknown     = 0,

    /* 'Pre' means VIR is at the begin of corresponding level, no any transformation
        is done yet. 'Post' VIR is at the end of corresponding level, all expected
        opts have been acted on this level */

    VIR_SHLEVEL_Pre_High,
    VIR_SHLEVEL_Post_High,

    VIR_SHLEVEL_Pre_Medium,
    VIR_SHLEVEL_Post_Medium,

    VIR_SHLEVEL_Pre_Low,
    VIR_SHLEVEL_Post_Low,

    VIR_SHLEVEL_Pre_Machine,
    VIR_SHLEVEL_Post_Machine
} VIR_ShLevel;

extern VIR_Context theVIRGlobalContext;

#define DEFAULTUNIFORMBLOCKINDEX -2     /* used to represent default uniform block index when VIR
                                           is being converted back to gcsl. will be removed when
                                           this conversion is not needed
                                        */
typedef enum _VIR_TESSPRIMITIVEMODE
{
    VIR_TESS_PMODE_TRIANGLE = 0,
    VIR_TESS_PMODE_QUAD,
    VIR_TESS_PMODE_ISOLINE
} VIR_TessPrimitiveMode;

typedef enum _VIR_TESSOUTPUTPRIMITIVE
{
    VIR_TESS_OUTPUT_PRIM_POINT       = 0,
    VIR_TESS_OUTPUT_PRIM_LINE,
    VIR_TESS_OUTPUT_PRIM_TRIANGLE_CW,
    VIR_TESS_OUTPUT_PRIM_TRIANGLE_CCW,
} VIR_TessOutputPrimitive;

typedef enum _VIR_TESSVERTEXSPACING
{
    VIR_TESS_SPACING_EQUAL = 0, /* equal_spacing */
    VIR_TESS_SPACING_EVEN, /* fractional_even_spacing */
    VIR_TESS_SPACING_ODD            /* fractional_odd_spacing */
} VIR_TessVertexSpacing;

typedef enum _VIR_TESSORDERING
{
    VIR_TESS_ORDER_CCW = 0, /* Counter Clockwise */
    VIR_TESS_ORDER_CW               /* Clockwise */
} VIR_TessOrdering;

typedef enum _VIR_GEOPRIMITIVE
{
    VIR_GEO_POINTS,
    VIR_GEO_LINES,
    VIR_GEO_LINES_ADJACENCY,
    VIR_GEO_TRIANGLES,
    VIR_GEO_TRIANGLES_ADJACENCY,
    VIR_GEO_LINE_STRIP,
    VIR_GEO_TRIANGLE_STRIP,
} VIR_GeoPrimitive;

typedef struct _VIR_COMPUTELAYOUT
{
    /* Compute shader layout qualifiers */
    gctUINT32           workGroupSize[3];  /* local group size in the first(0), second(1), and
                                               third(2) dimension */
} VIR_ComputeLayout;

typedef struct _VIR_TCSLAYOUT
{
    /* Tesselation Control Shader layout */
    gctINT                  tcsPatchInputVertices;
    gctINT                  tcsPatchOutputVertices;
    VIR_Uniform *           tcsInputVerticesUniform;
    gctBOOL                 hasOutputVertexAccess;
} VIR_TCSLayout;

typedef struct _VIR_TESLAYOUT
{
    /* Tessellation Evaluation Shader layout qualifiers*/
    VIR_TessPrimitiveMode   tessPrimitiveMode : 4;
    VIR_TessVertexSpacing   tessVertexSpacing : 3;
    VIR_TessOrdering        tessOrdering      : 2;
    gctBOOL                 tessPointMode     : 2;
    gctINT                  tessPatchInputVertices;   /* same value as tcsPatchOutputVertices after linking */
} VIR_TESLayout;

typedef struct _VIR_GEOLAYOUT
{
    /*  Geometry Shader layout */
    /* times of geometry shader executable is invoked for each input primitive received */
    gctINT                  geoInvocations;
    /* the maximum number of vertices the shader will ever emit in a single invocation */
    gctINT                  geoMaxVertices;
    /* type of input primitive accepted by geometry shader */
    VIR_GeoPrimitive        geoInPrimitive;
    /* type of output primitive accepted by geometry shader */
    VIR_GeoPrimitive        geoOutPrimitive;
} VIR_GEOLayout;

typedef struct _VSC_PASS_MANAGER* PVSC_PASS_MANAGER;

/* The structure that defines the gcSHADER object to the outside world. */
struct _VIR_SHADER
{
    /* The base object. */
    gcsOBJECT           object;
    gceAPI              clientApiVersion;  /* client API version. */
    gctUINT             _id;                /* unique id used for triage */
    gctUINT             _constVectorId;     /* unique constant uniform vector id */
    gctUINT             _dummyUniformCount;
    gctUINT             _orgTempCount;      /* the org temp register count of gcSL shader */
    gctUINT             _tempRegCount;      /* the temp register count of the shader */

    /* At which level that this VIR shader is */
    VIR_ShLevel         shLevel;

    /* Index of the uniform block that hold the const uniforms generated by compiler.*/
    gctINT              constUniformBlockIndex;

    /* The size of the uniform block that hold the const uniforms. */
    gctINT              constUBOSize;

    /* A memory buffer to store constant uniform. */
    gctUINT32 *         constUBOData;

    /* Index of the uniform block that hold the const uniforms exceeding the hw cap.*/
    gctINT              defaultUniformBlockIndex;

    /* Frontend compiler version */
    gctUINT32           compilerVersion[2];

    /* Type of shader. */
    VIR_ShaderKind      shaderKind;

    /* Flags */
    VIR_ShaderFlags     flags;

    /* Maximum of kernel function arguments, used to calculation the starting uniform index */
    gctUINT32           maxKernelFunctionArgs;

    /* Constant memory address space size for openCL */
    gctUINT32           constantMemorySize;
    gctCHAR    *        constantMemoryBuffer;

    /* Private memory address space size for openCL */
    gctUINT32           privateMemorySize;

    /* Local memory address space size for openCL, inherited from chosen kernel function */
    gctUINT32           localMemorySize;

    /* Attributes. */
    VIR_AttributeIdList attributes;

    /* Outputs. */
    VIR_OutputIdList    outputs;    /* the size of 'outputs' be allocated */
    VIR_OutputIdList    outputVregs; /* keep the outputs' vreg */

    /* per-patch input/output for Tessellation Control/Evaluation Shader */
    VIR_InputIdList     perpatchInput;
    VIR_OutputIdList    perpatchOutput;
    VIR_OutputIdList    perpatchOutputVregs; /* keep the outputs' vreg */

    /* Buffers */
    VIR_BufferIdList    buffers;        /* a list of buffer objects */

    /* Uniforms. */
    VIR_UniformIdList   uniforms;
    gctUINT             uniformCount;        /* after gcsl->VIR, it is set to the count of
                                                gcsl uniforms, which may not be all converted
                                                to VIR */
    gctUINT32           uniformVectorCount;  /* the vector count of all uniforms */
    gctINT              samplerIndex;

    VIR_SymId           baseSamplerId;       /* Create a base sampler symbol, use it as the operand symbol of SAMLER. */

    /* Sampler base offset. */
    gctINT              samplerBaseOffset;

    /* Global variables. */
    VIR_VariableIdList  variables;

    /* HALTI extras: Uniform block. */
    VIR_UBOIdList       uniformBlocks;

    /* ES 3.1: storage block. */
    VIR_SBOIdList       storageBlocks;

    /* ES 3.1 ext: IO block object */
    VIR_IOBIdList       ioBlocks;

    /* Functions. */
    VIR_FunctionList    functions;
    VIR_Function *      currentFunction;    /* private member */
    VIR_Function *      mainFunction;
    VIR_Function *      initFunction;

    /* Kernel Functions. */
    VIR_KernelInfo *    mainKernelInfo;     /* the kernel info when it is merged with main */
    VIR_FunctionList    kernelFunctions;
    VIR_Function *      currentKernelFunction;

    /* Shaderwise layout info */
    union {
        VIR_ComputeLayout   compute;
        VIR_TCSLayout       tcs;
        VIR_TESLayout       tes;
        VIR_GEOLayout       geo;
    } shaderLayout;

    gctINT *            loadUsers;

    /* load-time optimization uniforms */
    gctINT              ltcUniformCount;      /* load-time constant uniform count */
    gctUINT             ltcUniformBegin;      /* the begin offset of ltc in uniforms */
    gctINT *            ltcCodeUniformIndex;  /* an array to map code index to uniform index,
                                                 element which has 0 value means no uniform for the code*/
    gctUINT             ltcInstructionCount;  /* the total instruction count of the LTC expressions */
    VIR_Instruction *   ltcExpressions;       /* the expression array for ltc uniforms, which is a list of instructions */

    gctBOOL             vsPositionZDependsOnW;    /* for wClip */
    gctBOOL             psHasDiscard;
    gctBOOL             hasLoadStore;
    gctBOOL             hasImgReadWrite;
    gctBOOL             hasAtomicOp;
    gctBOOL             hasMemoryAccess;
    gctBOOL             hasBarrier;
    gctBOOL             useEarlyFragTest;
    gctBOOL             hasDsy;

    /* Optimization option. */
    gctUINT             optimizationOption;

    /* Transform feedback varyings */
    VIR_TransformFeedback   transformFeedback;

    /* Source code string */
    gctUINT             sourceLength;            /* including terminating '\0' */
    gctSTRING           source;

    /* mapping from library temp index to linked to shader temp index */
    gctUINT32           mappingTableExntries;
    gctINT *            mappingTable;
    gctUINT             linkedToShaderId;

    /* a list of library linked to this shader */
    VIR_LibraryList *   libraryList;

#if gcdSHADER_SRC_BY_MACHINECODE
    /* It is used to do high level GLSL shader detection, and give a BE a
       replacement index hint to replace automatically compiled machine code
       with manually written extremely optimized machine code. So it is
       a dynamic info based on driver client code, and supposedly it is not
       related to VIR structure. So DO NOT CONSIDER IT IN LOAD/SAVE ROUTINES.
       Putting this info in VIR structure because we don't want to break
       existed prototype of gcLinkShaders. If later somebody find another
       better place to pass this info into gcLinkShaders, feel free to change
       it.
     */
    gctUINT32           replaceIndex;
#endif

    /* Need some hardware infomation for lower. Add to here temperarily. */
    gctBOOL             __IsDual16Shader;

    /* for recompile only - whether the recompile's master shader is dual16 or not,
       if yes, we need to force recompile shader to be dual16 if possible */
    gctBOOL             __IsMasterDual16Shader;

    /* For unified sampler, all GPipe stages are packed one by one, and the start index
       of all stages are not 0. And we use bottom sampler memory for PS.
    */
    gctBOOL             packUnifiedSampler;

    /* Need to adjust sampler physical assignment for function argument. */
    gctBOOL             needToAdjustSamplerPhysical;

    gctUINT             _anonymousNameId;
    gctBOOL             _enableDefaultUBO;

    /* VIR Specific data */

    VIR_StringTable     stringTable;
    VIR_TypeTable       typeTable;          /* local type table fot the shader,
                                             * it later unified with global type
                                             * table when linking */
    VIR_SymTable        symTable;
    VIR_ConstTable      constTable;

    VIR_Context *       context;            /* TODO: change to Program Object */
    VSC_MM              mempool;            /* memory pool used by the shader */
    VIR_Dumper *        dumper;             /* dumper info */

    /* register allocation is enabled*/
    gctBOOL             RAEnabled;
    /* use hw reg information generated by register allocation*/
    gctBOOL             hwRegAllocated;
    gctUINT             hwRegWatermark;
    gctBOOL             constRegAllocated;

    /* the information that is needed for CG, which shows which reg channel regmap start*/
    gctUINT             remapRegStart;
    gctUINT8            remapChannelStart;

    /* reg and its channel for sample-mask and sample-index */
    gctUINT             sampleMaskIdRegStart;
    gctUINT             sampleMaskIdChannelStart;

    /* shader has spill */
    gctBOOL             hasRegisterSpill;
    gctUINT             vidmemSizeOfSpill;

    /* For pos/point-coord inputs of ps, we need know exact channel valid info to trigger some
       recompiling process.
     */
    gctBOOL             psInputPosCompValid[VIR_CHANNEL_COUNT];
    gctBOOL             psInputPCCompValid[VIR_CHANNEL_COUNT];

    /* Pass manager who manage compiling procedure for this shader */
    PVSC_PASS_MANAGER   passMnger;
};

/* VIR interface */
extern gctUINT
VIR_Inst_GetSourceIndex(
    IN VIR_Instruction     *pInst,
    IN VIR_Operand         *pOpnd
    );

extern gctUINT
VIR_Inst_GetEvisState(
    IN VIR_Instruction     *pInst,
    IN VIR_Operand         *pOpnd
    );

/* VIR_ConditionOp */
extern gctBOOL
VIR_ConditionOp_Reversable(
    IN VIR_ConditionOp cond_op
    );

extern VIR_ConditionOp
VIR_ConditionOp_Reverse(
    IN VIR_ConditionOp cond_op
    );

extern gctBOOL
VIR_ConditionOp_CouldCompareWithZero(
    IN VIR_ConditionOp cond_op
    );

extern VIR_ConditionOp
VIR_ConditionOp_SetCompareWithZero(
    IN VIR_ConditionOp cond_op
    );

extern VIR_ConditionOp
VIR_ConditionOp_SwitchLeftRight(
    IN VIR_ConditionOp cond_op
    );

gctUINT
VIR_ShaderKind_Map2KindId(
    IN VIR_ShaderKind kind
    );

extern VIR_Symbol *
VIR_GetSymFromId(
    VIR_SymTable *  SymTable,
    VIR_SymId       SymId);

extern VIR_NameId
vscStringTable_Find(
    VIR_StringTable* pStringTbl,
    const char* pStr,
    gctUINT len);

extern VSC_ErrCode
VIR_Shader_Construct(
    IN gcoHAL         Hal,
    IN VIR_ShaderKind ShaderType,
    IN VSC_MM *       MemPool,
    OUT VIR_Shader ** Shader
    );

VSC_ErrCode
VIR_Shader_Destroy(
    IN VIR_Shader * Shader
    );

gctBOOL
VIR_Shader_IsESCompiler(
    IN VIR_Shader * Shader
    );

gctBOOL
VIR_Shader_IsES30Compiler(
    IN VIR_Shader * Shader
    );

gctBOOL
VIR_Shader_IsES31Compiler(
    IN VIR_Shader * Shader
    );

VSC_ErrCode
VIR_Shader_AddFunction(
    IN  VIR_Shader *    Shader,
    IN  gctBOOL         IsKernel,
    IN  gctSTRING       Name,
    IN  VIR_TypeId      Type,
    OUT VIR_Function ** Function
    );

VSC_ErrCode
VIR_Shader_RemoveFunction(
    IN  VIR_Shader *    Shader,
    IN  VIR_Function *  Function
    );

gceSTATUS
VIR_Shader_GetFunctionByName(
    IN  VIR_Shader *    Shader,
    IN  gctCONST_STRING FunctionName,
    OUT VIR_Function **    Function
    );

gceSTATUS
VIR_Shader_CopyFunction(
    IN OUT  VIR_Shader *    ToShader,
    IN VIR_Shader *         FromShader,
    IN gctSTRING            FunctionName,
    OUT VIR_Function **     NewFunction
    );

VSC_ErrCode
VIR_Shader_AddString(
    IN  VIR_Shader *    Shader,
    IN  gctSTRING       String,
    OUT VIR_NameId *    Name
    );

VSC_ErrCode
VIR_Shader_AddBuiltinType(
    IN  VIR_Shader *    Shader,
    IN  VIR_BuiltinTypeInfo * TypeInfo,
    OUT VIR_TypeId*     TypeId
    );

VSC_ErrCode
VIR_Shader_AddArrayType(
    IN  VIR_Shader *    Shader,
    IN  VIR_TypeId      BaseType,
    IN  gctUINT32       ArrayLength,
    OUT VIR_TypeId *    TypeId
    );

VSC_ErrCode
VIR_Shader_AddPointerType(
    IN  VIR_Shader *    Shader,
    IN  VIR_TypeId      BaseType,
    IN  VIR_TyQualifier Qualifier,
    IN  VIR_AddrSpace   AS,
    OUT VIR_TypeId*     TypeId
    );

VSC_ErrCode
VIR_Shader_AddFunctionType(
    IN  VIR_Shader *    Shader,
    IN  VIR_TypeId      ReturnType,
    IN  VIR_TypeIdList *Params,
    OUT VIR_TypeId *    TypeId
    );

VSC_ErrCode
VIR_Shader_AddStructType(
    IN  VIR_Shader *    Shader,
    IN  gctBOOL         IsUnion,
    IN  VIR_NameId      NameId,
    OUT VIR_TypeId*     TypeId
    );

VSC_ErrCode
VIR_Shader_AddConstant(
    IN  VIR_Shader *    Shader,
    IN  VIR_TypeId      Type,
    IN  VIR_ConstVal *  Value,
    OUT VIR_ConstId *   ConstId
    );

gctBOOL
VIR_Shader_FindUniformByConstantValue(
    IN  VIR_Shader *      Shader,
    IN  VIR_Const *       Constant,
    OUT VIR_Uniform **    Uniform,
    OUT VIR_Swizzle *     Swizzle
    );

VSC_ErrCode
VIR_Shader_AddInitializedUniform(
    IN  VIR_Shader *      Shader,
    IN  VIR_Const *       Constant,
    OUT VIR_Uniform **    Uniform,
    OUT VIR_Swizzle *     Swizzle
    );

VSC_ErrCode
VIR_Shader_AddInitializedConstUniform(
    IN  VIR_Shader *      Shader,
    IN  VIR_Const *       Constant,
    OUT VIR_Uniform **    Uniform
    );

gctUINT
VIR_Shader_GetTypeByteSize(
    IN  VIR_Shader *    Shader,
    IN  VIR_Type *      Type
    );

gctUINT
VIR_Shader_GetTypeLocationRange(
    IN  VIR_Shader *    Shader,
    IN  VIR_Type *      Type
    );

VIR_Symbol*
VIR_Shader_AddBuiltinAttribute(
    IN  VIR_Shader *    VirShader,
    IN  VIR_TypeId      TypeId,
    IN  gctBOOL         isPerpatch,
    IN  VIR_NameId      builtinName
    );

VIR_Symbol*
VIR_Shader_AddBuiltinOutput(
    IN  VIR_Shader *    VirShader,
    IN  VIR_TypeId      TypeId,
    IN  gctBOOL         isPerpatch,
    IN  VIR_NameId      builtinName
    );

/* types */
void
VIR_Type_SetAlignment(
    IN OUT VIR_Type *   Type,
    IN  gctUINT         Alignment
    );

void
VIR_Shader_AddAddrSpace(
    IN OUT VIR_Type *   Type,
    IN  VIR_AddrSpace   AS
    );

VSC_ErrCode
VIR_Type_AddField(
    IN  VIR_Shader *    Shader,
    IN OUT VIR_Type *   Type,
    IN VIR_SymId        Field
    );

VSC_ErrCode
VIR_Type_AddFieldAndInfo(
    IN  VIR_Shader *    Shader,
    IN  VIR_Type *      StructType,
    IN  gctSTRING       FieldName,
    IN  gctUINT32       Offset,
    IN  gctBOOL         isBitField,
    IN  gctUINT         StartBit,
    IN  gctUINT         BitSize,
    IN  gctUINT         TempRegOffset,
    OUT VIR_SymId *     FieldSym
    );

gctUINT VIR_Type_GetVirRegCount(VIR_Shader * Shader, VIR_Type *);
gctUINT VIR_Symbol_GetVirIoRegCount(VIR_Shader * Shader, VIR_Symbol*);

/* For dual16 shader, the input symbol could be in 2 registers */
gctINT  VIR_Symbol_GetRegSize(
    IN VIR_Shader       *pShader,
    IN VSC_HW_CONFIG    *pHwCfg,
    IN VIR_Symbol       *Sym);

VIR_SymIndexingInfo VIR_Symbol_GetIndexingInfo(VIR_Shader * Shader, VIR_Symbol *Sym);

gctUINT VIR_Type_GetIndexingRange(VIR_Shader *, VIR_Type *);

VIR_Enable
VIR_Type_Conv2Enable(
    IN VIR_Type         *Type
    );

gctBOOL
VIR_Type_Identical(
    VIR_Shader* Shader1,
    VIR_Type*   Type1,
    VIR_Shader* Shader2,
    VIR_Type*   Type2
    );

VSC_ErrCode
VIR_Uniform_Identical(
    IN VIR_Shader* Shader1,
    IN VIR_Symbol* Sym1,
    IN VIR_Shader* Shader2,
    IN VIR_Symbol* Sym2,
    IN gctBOOL     CheckPrecision,
    OUT gctBOOL*   Matched
    );

VIR_Enable
VIR_TypeId_Conv2Enable(
    IN VIR_TypeId       TypeId
    );

VIR_TypeId
VIR_TypeId_ComposeNonOpaqueType(
    IN VIR_TypeId       ComponentType,
    IN gctUINT          CompCount,
    IN gctUINT          RowCount
    );

VIR_TypeId
VIR_TypeId_ComposeNonOpaqueArrayedType(
    IN VIR_Shader *     Shader,
    IN VIR_TypeId       ComponentType,
    IN gctUINT          CompCount,
    IN gctUINT          RowCount,
    IN gctINT           arrayLength
    );

/* shader symbols */

/* return NULL if symbol is not found */
VIR_Symbol *
VIR_Shader_FindSymbolById(
    IN  VIR_Shader *    Shader,
    IN  VIR_SymbolKind  SymbolKind,
    IN  VIR_Id          NameOrConstId);

VIR_Symbol *
VIR_Shader_FindSymbolByTempIndex(
    IN  VIR_Shader *    Shader,
    IN  VIR_Id          TempIndex);

VIR_Symbol *
VIR_Shader_FindSymbolByName(
    IN  VIR_Shader *    Shader,
    IN  VIR_SymbolKind  SymbolKind,
    IN  gctSTRING       Name);

VSC_ErrCode
VIR_Shader_AddFieldSymbol(
    IN  VIR_Shader *    Shader,
    IN  VIR_Id          NameId, /* field nameId */
    IN  VIR_Type *      Type, /* field type */
    IN  VIR_Type *      StructType, /* struct type of the field */
    IN  VIR_StorageClass Storage,
    OUT VIR_SymId*      SymId);

VSC_ErrCode
VIR_Shader_AddSymbol(
    IN  VIR_Shader *    Shader,
    IN  VIR_SymbolKind  SymbolKind,
    IN  VIR_NameId      Name,
    IN  VIR_Type*       Type,
    IN  VIR_StorageClass Storage,
    OUT VIR_SymId*      SymId);

VSC_ErrCode
VIR_Shader_AddSymbolWithName(
    IN  VIR_Shader *    Shader,
    IN  VIR_SymbolKind  SymbolKind,
    IN  gctSTRING       Name,
    IN  VIR_Type *      Type,
    IN  VIR_StorageClass Storage,
    OUT VIR_SymId*      SymId);

/* get RegCount of VirReg from the Shader, return the first VirRegId
 * user needs to add VIRREG symbol with the VirRegId to shader symbol table
 */
VIR_VirRegId
VIR_Shader_NewVirRegId(
    IN VIR_Shader *       Shader,
    IN gctUINT            RegCount
    );

VIR_VirRegId
VIR_Shader_UpdateVirRegCount(
    IN VIR_Shader *        Shader,
    IN VIR_VirRegId        RegIndex
    );

gctUINT
VIR_Shader_GetVirRegCount(
    IN VIR_Shader *        Shader
    );

VIR_VarTempRegInfo *
VIR_Shader_GetXFBVaryingTempRegInfo(
    IN VIR_Shader *    Shader,
    IN gctUINT         VaryingIndex
    );

/* shaders */

gctUINT32
VIR_Shader_GetTotalInstructionCount(
    IN  VIR_Shader *    Shader
    );

gctUINT
VIR_Shader_RenumberInstId(
    IN  VIR_Shader *  Shader
    );

VIR_Uniform*
VIR_Shader_GetUniformFromGCSLIndex(
    IN  VIR_Shader *  Shader,
    IN  gctINT        GCSLIndex
    );

/* setters */
void
VIR_Symbol_SetName(
    IN OUT VIR_Symbol *     Symbol,
    IN  VIR_NameId          Name
    );

gctSTRING
VIR_Symbol_GetAttrName(
    IN VIR_Shader* pShader,
    IN VIR_Symbol *     AttrSymbol
    );

void
VIR_Symbol_SetConst(
    IN OUT VIR_Symbol *     Symbol,
    IN  VIR_ConstId         Constant
    );


void
VIR_Symbol_AddFlag(
    IN OUT VIR_Symbol * Symbol,
    IN VIR_SymFlag      Flag
    );

void
VIR_Symbol_RemoveFlag(
    IN OUT VIR_Symbol * Symbol,
    IN VIR_SymFlag      Flag
    );

void
VIR_Symbol_SetOffset(
    IN OUT VIR_Symbol * Symbol,
    IN gctUINT32        Offset,
    IN gctUINT          TempRegOffset
    );

/* return true if the name of symbol1 in shader1 matches
 * the name of symbol2 in shader2 */
gctBOOL
VIR_Symbol_isNameMatch(
    IN VIR_Shader *        Shader1,
    IN VIR_Symbol *        Symbol1,
    IN VIR_Shader *        Shader2,
    IN VIR_Symbol *        Symbol2
    );

gctUINT VIR_Symbol_GetComponents(VIR_Symbol *pSym);

/* getters */
gctSTRING VIR_GetSymbolKindName(VIR_SymbolKind  SymbolKind);

/* functions */
VSC_ErrCode
VIR_Function_AddSymbol(
    IN  VIR_Function *  Function,
    IN  VIR_SymbolKind  SymbolKind,
    IN  VIR_Id          NameOrConstIdOrRegId, /* constId for VIR_SYM_CONST,
                                                  VirRegId for VIR_SYM_VIRREG,
                                                  otherwise nameId */
    IN  VIR_Type *       Type, /* for VIR_SYM_FIELD, use struct type */
    IN  VIR_StorageClass Storage,
    OUT VIR_SymId *     SymId
    );

VSC_ErrCode
VIR_Function_AddSymbolWithName(
    IN  VIR_Function *  Function,
    IN  VIR_SymbolKind  SymbolKind,
    IN  gctSTRING       Name,
    IN  VIR_Type *      Type, /* for VIR_SYM_FIELD, use struct type */
    IN  VIR_StorageClass Storage,
    OUT VIR_SymId*      SymId
    );

VIR_Symbol *
VIR_Function_GetSymFromId(
    IN  VIR_Function *  Function,
    IN  VIR_SymId       SymId
    );

VSC_ErrCode
VIR_Function_AddLabel(
    IN  VIR_Function *  Function,
    IN  gctSTRING       LabelName,
    OUT VIR_LabelId *   Label
    );

VSC_ErrCode
VIR_Function_FreeLabel(
    IN  VIR_Function *  Function,
    IN  VIR_LabelId     Label
    );


VSC_ErrCode
VIR_Function_AddParameter(
    IN  VIR_Function *  Function,
    IN  gctSTRING       ParamName,
    IN  VIR_TypeId      Type,
    IN  VIR_StorageClass Storage,
    OUT VIR_SymId *     SymId
    );

VSC_ErrCode
VIR_Function_AddLocalVar(
    IN  VIR_Function *  Function,
    IN  gctSTRING       VarName,
    IN  VIR_TypeId      Type,
    OUT VIR_SymId *     SymId
    );

VSC_ErrCode
VIR_Function_NewInstruction(
    IN  VIR_Function *  Function,
    IN  VIR_OpCode      Opcode,
    IN  VIR_TypeId      ResType,
    OUT VIR_Instruction **Inst
    );

VSC_ErrCode
VIR_Function_AddInstruction(
    IN  VIR_Function *  Function,
    IN  VIR_OpCode      Opcode,
    IN  VIR_TypeId      ResType,
    OUT VIR_Instruction **Inst
    );

VSC_ErrCode
VIR_Function_PrependInstruction(
    IN  VIR_Function *  Function,
    IN  VIR_OpCode      Opcode,
    IN  VIR_TypeId      ResType,
    OUT VIR_Instruction **Inst
    );

VSC_ErrCode
VIR_Function_AddInstructionAfter(
    IN  VIR_Function *  Function,
    IN  VIR_OpCode      Opcode,
    IN  VIR_TypeId      ResType,
    IN  VIR_Instruction *AfterMe,
    OUT VIR_Instruction **Inst
    );

VSC_ErrCode
VIR_Function_AddInstructionBefore(
    IN  VIR_Function *  Function,
    IN  VIR_OpCode      Opcode,
    IN  VIR_TypeId      ResType,
    IN  VIR_Instruction *BeforeMe,
    OUT VIR_Instruction **Inst
    );

VSC_ErrCode
VIR_Function_RemoveInstruction(
    IN VIR_Function *   Function,
    IN VIR_Instruction *Inst
    );

VSC_ErrCode
VIR_Function_MoveInstructionBefore(
    IN  VIR_Function *  Function,
    IN  VIR_Instruction *BeforeMe,
    IN VIR_Instruction  *Inst
    );

VSC_ErrCode
VIR_Function_NewLink(
    IN  VIR_Function *  Function,
    OUT VIR_Link **  Link
    );

VSC_ErrCode
VIR_Function_FreeLink(
    IN  VIR_Function *  Function,
    OUT VIR_Link *      Link
    );

VSC_ErrCode
VIR_Function_NewOperand(
    IN  VIR_Function *  Function,
    OUT VIR_Operand **  Operand
    );

VSC_ErrCode
VIR_Function_DupOperand(
    IN  VIR_Function *  Function,
    IN  VIR_Operand *   Src,
    OUT VIR_Operand **  Dup
    );

VSC_ErrCode
VIR_Function_FreeOperand(
    IN  VIR_Function *  Function,
    IN  VIR_Operand *   Operand
    );

void VIR_Function_SetVirtualInstStart(VIR_Function *, gctUINT);

VSC_ErrCode
VIR_Inst_ConstructArg(
    IN OUT VIR_Instruction * Inst,
    IN VIR_Function * Function,
    IN gctUINT ParmIndex,
    IN VIR_Operand* Operand
    );

VSC_ErrCode
VIR_Inst_ConstructCall(
    IN OUT VIR_Instruction * Inst,
    IN VIR_Function * Function
    );

VSC_ErrCode
VIR_Inst_ConstructRetRetValue(
    IN OUT VIR_Instruction * Inst,
    IN VIR_Function * Callee,
    IN VIR_Function * Caller
    );

VSC_ErrCode
VIR_Inst_ConstructRet(
    IN OUT VIR_Instruction * Inst
    );

gctBOOL
VIR_Inst_Store_Have_Dst(
    IN VIR_Instruction * Inst
);


VSC_ErrCode
VIR_Inst_FreeOperand(
    IN OUT VIR_Instruction * Inst,
    IN gctINT                SrcNum
    );

VSC_ErrCode
VIR_Inst_CopyDest(
    IN OUT VIR_Instruction * Inst,
    IN VIR_Operand *         FromDest,
    IN gctBOOL               KeepOrgType
    );

VSC_ErrCode
VIR_Inst_CopySource(
    IN OUT VIR_Instruction * Inst,
    IN gctINT                SrcNum,
    IN VIR_Operand *         FromOperand,
    IN gctBOOL               KeepSrcType   /* keep original source type */
    );

gctBOOL
VIR_Inst_IdenticalExpression(
    IN VIR_Instruction  *Inst0,
    IN VIR_Instruction  *Inst1,
    IN VIR_Shader       *Shader,
    IN gctBOOL          precisionMatters
    );

VIR_TypeId
VIR_Inst_GetExpressionTypeID(
    IN VIR_Instruction  *Inst,
    IN VIR_Shader       *Shader
    );

VIR_Precision
VIR_Inst_GetExpectedResultPrecision(
    IN VIR_Instruction  *Inst
    );

/* swizzle */

gctBOOL
VIR_Swizzle_IsEnable(
    IN VIR_Swizzle swizzle
    );

VIR_Swizzle
VIR_Swizzle_GetSwizzlingSwizzle(
    IN VIR_Swizzle swizzle1,
    IN VIR_Swizzle swizzle2
    );

/* it is a channel swizzling problem, like transfering xxyy with zyxw to yxxy */
VIR_Swizzle
VIR_Swizzle_ApplySwizzlingSwizzle(
    IN VIR_Swizzle swizzle,
    IN VIR_Swizzle trans
    );

VIR_Swizzle
VIR_Swizzle_GetMappingSwizzle2Swizzle(
    IN VIR_Swizzle swizzle1,
    IN VIR_Swizzle swizzle2
    );

gctBOOL
VIR_Swizzle_GetMappingSwizzle2Enable(
    IN VIR_Swizzle swizzle,
    IN VIR_Enable enable,
    OUT VIR_Swizzle * mapping_swizzle
    );

VIR_Swizzle
VIR_Swizzle_MergeMappingSwizzles(
    IN VIR_Swizzle map1,
    IN VIR_Swizzle map2
    );

/* it is a channel mapping problem, like mapping x->y, y->z, z->x, w->z.
   with maping yzxz, wxxy would be mapped to zyyz */
VIR_Swizzle
VIR_Swizzle_ApplyMappingSwizzle(
    IN VIR_Swizzle swizzle,
    IN VIR_Swizzle map
    );

VIR_Swizzle
VIR_Swizzle_Trim(
    IN VIR_Swizzle swizzle,
    IN VIR_Enable enable
    );

VIR_Swizzle
VIR_Swizzle_MappingNewSwizzle(
    IN VIR_Enable Enable0,
    IN VIR_Enable Enable1,
    IN VIR_Swizzle Swizzle0,
    IN VIR_Swizzle Swizzle1
    );

/* enable */

VIR_Swizzle
VIR_Enable_2_Swizzle(
    IN VIR_Enable enable
    );

VIR_Swizzle
VIR_Enable_2_Swizzle_WShift(
    IN VIR_Enable Enable
    );

VIR_Swizzle
VIR_Enable_GetMappingSwizzle(
    IN VIR_Enable enable,
    IN VIR_Swizzle swizzle
    );

VIR_Enable
VIR_Enable_ApplyMappingSwizzle(
    IN VIR_Enable enable,
    IN VIR_Swizzle mappingSwizzle
    );

/* set operands */

void
VIR_Operand_SetSymbol(
    IN OUT VIR_Operand*    Operand,
    IN     VIR_Function *  Function,
    IN     VIR_SymId       SymId
    );

void
VIR_Operand_SetImmediate(
    IN OUT VIR_Operand*    Operand,
    IN  VIR_TypeId         Type,
    IN  VIR_ScalarConstVal Immed
    );

void
VIR_Operand_SetLabel(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_Label *         Label
    );

void
 VIR_Operand_SetImmediateInt(
     IN OUT VIR_Operand *    Operand,
     IN gctINT              Val
     );

void
 VIR_Operand_SetImmediateUint(
     IN OUT VIR_Operand *    Operand,
     IN gctUINT              Val
     );

void
 VIR_Operand_SetImmediateFloat(
     IN OUT VIR_Operand *    Operand,
     IN gctFLOAT             Val
     );

void
VIR_Operand_SetConst(
    IN OUT VIR_Operand *Operand,
    IN  VIR_TypeId      Type,
    IN  VIR_ConstId     ConstVector
    );

void
VIR_Operand_SetUniform(
    IN OUT VIR_Operand *Operand,
    IN  VIR_Uniform *   Uniform,
    IN  VIR_Shader *    Shader
    );

void
VIR_Operand_SetParameters(
    IN OUT VIR_Operand *Operand,
    IN  VIR_ParmPassing *Parms
    );

void
VIR_Operand_SetFunction(
    IN OUT VIR_Operand * Operand,
    IN  VIR_Function *   Function
    );

void
VIR_Operand_SetIntrinsic(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_IntrinsicsKind  Intrinsic
    );

void
VIR_Operand_SetFieldAccess(
    IN OUT VIR_Operand *Operand,
    IN  VIR_Operand *   Base,
    IN  VIR_SymId       FieldId
    );

void
VIR_Operand_SetArrayIndexing(
    IN OUT VIR_Operand * Operand,
    IN  VIR_Operand *   Base,
    IN  VIR_OperandList* ArrayIndex
    );

void
VIR_Operand_SetSamplerIndexing(
    IN OUT VIR_Operand * Operand,
    IN VIR_Shader *      Shader,
    IN  VIR_Symbol *     SamplerIndexing
    );

void
VIR_Operand_SetTempRegister(
    IN OUT VIR_Operand *    Operand,
    IN     VIR_Function *   Function,
    IN     VIR_SymId        TempSymId,
    IN     VIR_TypeId       OpernadType
    );

void
VIR_Operand_SetTexldBias(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_Operand    *    Bias
    );

void
VIR_Operand_SetTexldLod(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_Operand    *    Lod
    );

void
VIR_Operand_SetTexldGradient(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_Operand    *    Pdx,
    IN  VIR_Operand    *    Pdy
    );

void
VIR_Operand_SetTexldGather(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_Operand    *    Component,
    IN  VIR_Operand    *    RefZ
    );

void
VIR_Operand_SetTexldFetchMS(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_Operand    *    Sample
    );

void
VIR_Operand_SetTexldOffset(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_Operand    *    Offset
    );

/* for source operand only */
void
VIR_Operand_SetSwizzle(
    IN OUT VIR_Operand *Operand,
    IN  VIR_Swizzle     Swizzle
    );

void
VIR_Operand_ShrinkSwizzle(
    IN OUT VIR_Operand *Operand,
    IN  VIR_Swizzle     Swizzle
    );

/* for  dest operand only */
void
VIR_Operand_SetEnable(
    IN OUT VIR_Operand *Operand,
    IN  VIR_Enable      Enable
    );

void
VIR_Operand_ShrinkEnable(
    IN OUT VIR_Operand *Operand,
    IN  VIR_Enable      Enable
    );

void
VIR_Operand_SetRelIndexing(
    IN OUT VIR_Operand *Operand,
    IN VIR_SymId        IndexSym
    );

void
VIR_Operand_SetRelIndexingImmed(
    IN OUT VIR_Operand *Operand,
    IN gctINT           IndexImmed
    );

void
VIR_Operand_GetOperandInfo(
    IN  VIR_Instruction *   Inst,
    IN  VIR_Operand *       Operand,
    OUT VIR_OperandInfo *   Info);

VIR_Symbol *
VIR_Operand_GetUnderlyingSymbol(
    IN VIR_Operand * Operand
    );

/* whether an operand could be negated by adding a neg modifier
   or negate its const/imm value directly */
gctBOOL
VIR_Operand_IsNegatable(
    IN  VIR_Shader *        Shader,
    IN  VIR_Operand *       Operand);

void
VIR_Operand_NegateOperand(
    IN  VIR_Shader *        Shader,
    IN  VIR_Operand *       Operand);

gctBOOL
VIR_Operand_SameLocation(
    IN  VIR_Instruction *   Inst1,
    IN  VIR_Operand *       Operand1,
    IN  VIR_Instruction *   Inst2,
    IN  VIR_Operand *       Operand2);

gctBOOL
VIR_Operand_SameSymbol(
    IN VIR_Operand  *Opnd0,
    IN VIR_Operand  *Opnd1
    );

gctBOOL
VIR_Operand_Identical(
    IN VIR_Operand  *Opnd0,
    IN VIR_Operand  *Opnd1,
    IN VIR_Shader   *Shader
    );

gctBOOL
VIR_Operand_Defines(
    IN VIR_Operand  *Opnd0,
    IN VIR_Operand  *Opnd1
    );

void
VIR_Operand_Change2Dest(
    IN OUT VIR_Operand*     Operand);

void
VIR_Operand_Change2Src(
    IN OUT VIR_Operand*     Operand);

void
VIR_Operand_ReplaceDefOperandWithDef(
    IN OUT VIR_Operand *    Def,
    IN VIR_Operand *        New_Def,
    IN VIR_Enable           New_Enable
    );

void
VIR_Operand_ReplaceUseOperandWithDef(
    IN  VIR_Operand *       Def,
    IN OUT VIR_Operand *    Use
    );

void
VIR_Operand_ReplaceUseOperandWithUse(
    IN OUT VIR_Operand *    Use,
    IN VIR_Operand *        New_Use,
    IN VIR_Swizzle          New_Swizzle
    );

gctBOOL
VIR_Operand_IsOwnerInst(
    IN VIR_Operand *     Operand,
    IN VIR_Instruction * Inst
    );

gctBOOL
VIR_Operand_isInputVariable(
    VIR_Operand * Operand
    );

VIR_Swizzle
VIR_NormalizeSwizzleByEnable(
    IN VIR_Enable       Enable,
    IN VIR_Swizzle      Swizzle
    );

VIR_Enable
VIR_GetEnableByEnableComponent(
    IN VIR_Enable          Enable
    );

VIR_Enable
VIR_Operand_GetRealUsedChannels(
    IN VIR_Operand *     Operand,
    IN VIR_Instruction * Inst,
    VIR_Swizzle*         RealSwizzle);

gctBOOL
VIR_Operand_IsPerPatch(
    IN VIR_Operand *Operand
);

gctBOOL
VIR_Operand_IsArrayedPerVertex(
    IN VIR_Operand *Operand
);

gctUINT
VIR_Operand_GetPrecision(
    IN VIR_Operand *Operand
);

gctBOOL
VIR_Opnd_ValueFit16Bits(
    IN VIR_Operand *Operand
);

/* If src is imm, return -1, and value is filled with IMM,
   otherwise, return regNo, and value is filled with swizzle */
gctUINT
VIR_Opnd_GetCompWiseSrcChannelValue(
    IN VIR_Shader* pShader,
    IN VIR_Instruction* Inst,
    IN VIR_Operand *srcOpnd,
    IN gctUINT8 channel,
    OUT gctUINT* pValue
);

void
VIR_Link_AddLink(
    IN  VIR_Link **         Head,
    IN  VIR_Link *          Link
    );

void
VIR_Link_RemoveLink(
    IN  VIR_Link **         Head,
    IN  gctUINTPTR_T        Reference
    );

gctUINT
VIR_Link_Count(
    IN  VIR_Link *          Head
    );

gctBOOL
VIR_Link_IsLinkContained(
    IN  VIR_Link *          Head,
    IN  gctUINTPTR_T        Reference
    );

void
VIR_ScalarConstVal_GetNeg(
    IN  VIR_PrimitiveTypeId type,
    IN  VIR_ScalarConstVal* in_imm,
    OUT VIR_ScalarConstVal* out_imm
    );

void
VIR_ScalarConstVal_AddScalarConstVal(
    IN  VIR_PrimitiveTypeId type,
    IN  VIR_ScalarConstVal* in_imm0,
    IN  VIR_ScalarConstVal* in_imm1,
    OUT VIR_ScalarConstVal* out_imm
    );

void
VIR_ScalarConstVal_MulScalarConstVal(
    IN  VIR_PrimitiveTypeId type,
    IN  VIR_ScalarConstVal* in_imm0,
    IN  VIR_ScalarConstVal* in_imm1,
    OUT VIR_ScalarConstVal* out_imm
    );

gctBOOL
VIR_ScalarConstVal_One(
    IN  VIR_PrimitiveTypeId type,
    IN  VIR_ScalarConstVal* in_imm
    );

void
VIR_VecConstVal_GetNeg(
    IN  VIR_PrimitiveTypeId type,
    IN  VIR_VecConstVal* in_const,
    OUT VIR_VecConstVal* out_const
    );

void
VIR_VecConstVal_AddScalarConstVal(
    IN  VIR_PrimitiveTypeId type,
    IN  VIR_VecConstVal* in_const,
    IN  VIR_ScalarConstVal* in_imm,
    OUT VIR_VecConstVal* out_const
    );

void
VIR_VecConstVal_MulScalarConstVal(
    IN  VIR_PrimitiveTypeId type,
    IN  VIR_VecConstVal* in_const,
    IN  VIR_ScalarConstVal* in_imm,
    OUT VIR_VecConstVal* out_const
    );

void
VIR_VecConstVal_AddVecConstVal(
    IN  VIR_PrimitiveTypeId type,
    IN  VIR_VecConstVal* in_const0,
    IN  VIR_VecConstVal* in_const1,
    OUT VIR_VecConstVal* out_const
    );

void
VIR_VecConstVal_MulVecConstVal(
    IN  VIR_PrimitiveTypeId type,
    IN  VIR_VecConstVal* in_const0,
    IN  VIR_VecConstVal* in_const1,
    OUT VIR_VecConstVal* out_const
    );

gctBOOL
VIR_VecConstVal_AllZero(
    IN  VIR_PrimitiveTypeId type,
    IN  VIR_VecConstVal* in_const
    );

gctBOOL
VIR_VecConstVal_AllOne(
    IN  VIR_PrimitiveTypeId type,
    IN  VIR_VecConstVal* in_const
    );

gctBOOL
VIR_Evaluate_JMPC_Condition(
    IN  VIR_Shader      *pShader,
    IN  VIR_Instruction *inst,
    OUT gctBOOL         *checkingResult
    );

const VIR_Opcode_Info* VIR_Opcode_GetInfo(
    IN VIR_OpCode opcode);

VIR_TessOutputPrimitive
VIR_ConvertTESLayoutToOutputPrimitive(
    IN VIR_TESLayout* TesLayout);

VSC_ErrCode
VIR_Shader_CheckDual16(
    IN  VIR_Shader *           Shader,
    OUT VIR_ShaderCodeInfo *   CodeInfo
    );

gctBOOL
VIR_Shader_IsDual16able(
    IN OUT  VIR_Shader          *Shader,
    IN      VSC_HW_CONFIG       *pHwCfg);

VSC_ErrCode
VIR_Shader_CalcSamplerCount(
    IN      VIR_Shader *         Shader,
    IN OUT  gctINT*              SamplerCount
    );

END_EXTERN_C()

#endif /* __gc_vsc_vir_ir_h_ */

