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


#ifndef __gc_cl_built_ins_ks_h_
#define __gc_cl_built_ins_ks_h_

static clsBUILTIN_FUNCTION    KSBuiltinFunctions[] =
{
    {clvEXTENSION_NONE,    "get_global_id",       T_SIZE_T,    1, {T_UINT}, {0}, {1}, 1},
    {clvEXTENSION_NONE,    "get_local_id",        T_SIZE_T,    1, {T_UINT}, {0}, {1}, 1},
    {clvEXTENSION_NONE,    "get_group_id",        T_SIZE_T,    1, {T_UINT}, {0}, {1}, 1},
    {clvEXTENSION_NONE,    "get_work_dim",        T_UINT,      0, {T_VOID}, {0}, {1}, 1},
    {clvEXTENSION_NONE,    "get_global_size",     T_SIZE_T,    1, {T_UINT}, {0}, {1}, 1},
    {clvEXTENSION_NONE,    "get_local_size",      T_SIZE_T,    1, {T_UINT}, {0}, {1}, 1},
    {clvEXTENSION_NONE,    "get_global_offset",   T_SIZE_T,    1, {T_UINT}, {0}, {1}, 1},
    {clvEXTENSION_NONE,    "get_num_groups",      T_SIZE_T,    1, {T_UINT}, {0}, {1}, 1},

    {clvEXTENSION_NONE,    "barrier",          T_VOID, 1, {T_UINT}, {0}, {1}, 1},
    {clvEXTENSION_NONE,    "mem_fence",        T_VOID, 1, {T_UINT}, {0}, {1}, 1},

};

#define _cldKSBuiltinFunctionCount (sizeof(KSBuiltinFunctions) / sizeof(clsBUILTIN_FUNCTION))

static gceSTATUS
_GenGetGlobalIdCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
   gceSTATUS status;
   clsROPERAND *argument;
   clsNAME *globalId;
   gctINT type;
   clsLOPERAND lOperand[1];
   clsROPERAND rOperand[1];
   clsROPERAND    twoROperand[1], oneROperand[1], zeroROperand[1];

/* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
   gcmASSERT(OperandCount == 1);
   gcmASSERT(OperandsParameters);
   gcmASSERT(IOperand);


   clsROPERAND_InitializeIntOrIVecConstant(twoROperand,
                       clmGenCodeDataType(T_UINT),
                       (gctUINT)2);
   clsROPERAND_InitializeIntOrIVecConstant(oneROperand,
                       clmGenCodeDataType(T_UINT),
                       (gctUINT)1);
   clsROPERAND_InitializeIntOrIVecConstant(zeroROperand,
                       clmGenCodeDataType(T_UINT),
                       (gctUINT)0);

   globalId = _cldUnnamedVariable(clvBUILTIN_GLOBAL_ID);
/* Allocate all logical registers */
   status = clsNAME_AllocLogicalRegs(Compiler,
                                     CodeGenerator,
                                     globalId);
   if(gcmIS_ERROR(status)) return status;

   clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);
   clsROPERAND_InitializeReg(rOperand,
                             _cldUnnamedVariableRegs(clvBUILTIN_GLOBAL_ID));
   type = clGetVectorTerminalToken(globalId->decl.dataType->elementType, 1);
   rOperand->dataType = clmGenCodeDataType(type);
   argument = OperandsParameters->rOperands;

   if(argument->isReg) { /* variable argument */
   /* The condition part: t0 == 0 */
      clmGEN_CODE_IF(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo,
                     &OperandsParameters[0].rOperands[0],
                     clvCONDITION_EQUAL,
                     zeroROperand);

        rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
        rOperand->vectorIndex.u.constant = (gctREG_INDEX)0;
        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lOperand,
                                   rOperand));

      /* The false part, "!==0" */
      clmGEN_CODE_ELSE(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo);

     /* The condition part: t0 == 1 */
         clmGEN_CODE_IF(Compiler,
                        CodeGenerator,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        &OperandsParameters[0].rOperands[0],
                        clvCONDITION_EQUAL,
                        oneROperand);

            rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
            rOperand->vectorIndex.u.constant = (gctREG_INDEX)1;
            gcmONERROR(clGenAssignCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       lOperand,
                                       rOperand));

     /* The false part, "!==0 && !==1" */
         clmGEN_CODE_ELSE(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

         /* The condition part: t0 == 2 */
             clmGEN_CODE_IF(Compiler,
                            CodeGenerator,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            &OperandsParameters[0].rOperands[0],
                            clvCONDITION_EQUAL,
                            twoROperand);

                rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
                rOperand->vectorIndex.u.constant = (gctREG_INDEX)2;
                gcmONERROR(clGenAssignCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           lOperand,
                                           rOperand));

         /* The false part, "!==0 && !==1" && "!==2" */
             clmGEN_CODE_ELSE(Compiler,
                              CodeGenerator,
                              PolynaryExpr->exprBase.base.lineNo,
                              PolynaryExpr->exprBase.base.stringNo);

                gcmONERROR(clGenAssignCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           lOperand,
                                           zeroROperand));

             clmGEN_CODE_ENDIF(Compiler,
                               CodeGenerator,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo);

          clmGEN_CODE_ENDIF(Compiler,
                            CodeGenerator,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo);

       clmGEN_CODE_ENDIF(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);
   }

   else {  /* constant argument */
      gctUINT argValue;
      argValue = argument->u.constant.values[0].intValue;
      if(argValue < cldMaxWorkDim) {
         rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
         rOperand->vectorIndex.u.constant = (gctREG_INDEX)argValue;
         gcmONERROR(clGenAssignCode(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    lOperand,
                                    rOperand));
      }
      else {
         gcmONERROR(clGenAssignCode(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    lOperand,
                                    zeroROperand));
      }
   }

OnError:
   return status;
}

static gceSTATUS
_GenGetLocalIdCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
   gceSTATUS status;
   clsROPERAND *argument;
   clsNAME *localId;
   gctINT type;
   clsLOPERAND lOperand[1];
   clsROPERAND rOperand[1];
   clsROPERAND    twoROperand[1], oneROperand[1], zeroROperand[1];


/* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
   clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
   gcmASSERT(OperandCount == 1);
   gcmASSERT(OperandsParameters);
   gcmASSERT(IOperand);



   clsROPERAND_InitializeIntOrIVecConstant(twoROperand,
                       clmGenCodeDataType(T_UINT),
                       (gctUINT)2);
   clsROPERAND_InitializeIntOrIVecConstant(oneROperand,
                       clmGenCodeDataType(T_UINT),
                       (gctUINT)1);
   clsROPERAND_InitializeIntOrIVecConstant(zeroROperand,
                       clmGenCodeDataType(T_UINT),
                       (gctUINT)0);
   localId = _cldUnnamedVariable(clvBUILTIN_LOCAL_ID);
/* Allocate all logical registers */
   status = clsNAME_AllocLogicalRegs(Compiler,
                                     CodeGenerator,
                                     localId);
   if(gcmIS_ERROR(status)) return status;

   clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);
   clsROPERAND_InitializeReg(rOperand,
                             _cldUnnamedVariableRegs(clvBUILTIN_LOCAL_ID));
   type = clGetVectorTerminalToken(localId->decl.dataType->elementType, 1);
   rOperand->dataType = clmGenCodeDataType(type);
   argument = OperandsParameters->rOperands;

   if(argument->isReg) { /* variable argument */

   /* The condition part: t0 == 0 */
      clmGEN_CODE_IF(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo,
                     &OperandsParameters[0].rOperands[0],
                     clvCONDITION_EQUAL,
                     zeroROperand);

        rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
        rOperand->vectorIndex.u.constant = (gctREG_INDEX)0;
        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lOperand,
                                   rOperand));

      /* The false part, "!==0" */
      clmGEN_CODE_ELSE(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo);

     /* The condition part: t0 == 1 */
         clmGEN_CODE_IF(Compiler,
                        CodeGenerator,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        &OperandsParameters[0].rOperands[0],
                        clvCONDITION_EQUAL,
                        oneROperand);

            rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
            rOperand->vectorIndex.u.constant = (gctREG_INDEX)1;
            gcmONERROR(clGenAssignCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       lOperand,
                                       rOperand));

     /* The false part, "!==0 && !==1" */
         clmGEN_CODE_ELSE(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

         /* The condition part: t0 == 2 */
             clmGEN_CODE_IF(Compiler,
                            CodeGenerator,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            &OperandsParameters[0].rOperands[0],
                            clvCONDITION_EQUAL,
                            twoROperand);

                rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
                rOperand->vectorIndex.u.constant = (gctREG_INDEX)2;
                gcmONERROR(clGenAssignCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           lOperand,
                                           rOperand));

         /* The false part, "!==0 && !==1" && "!==2" */
             clmGEN_CODE_ELSE(Compiler,
                              CodeGenerator,
                              PolynaryExpr->exprBase.base.lineNo,
                              PolynaryExpr->exprBase.base.stringNo);

                gcmONERROR(clGenAssignCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           lOperand,
                                           zeroROperand));

             clmGEN_CODE_ENDIF(Compiler,
                               CodeGenerator,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo);

          clmGEN_CODE_ENDIF(Compiler,
                            CodeGenerator,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo);

       clmGEN_CODE_ENDIF(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);
   }
   else {  /* constant argument */
      gctUINT argValue;
      argValue = argument->u.constant.values[0].intValue;
      if(argValue < cldMaxWorkDim) {
         rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
         rOperand->vectorIndex.u.constant = (gctREG_INDEX)argValue;
         gcmONERROR(clGenAssignCode(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    lOperand,
                                    rOperand));
      }
      else {
         gcmONERROR(clGenAssignCode(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    lOperand,
                                    zeroROperand));
      }
   }
OnError:
   return status;
}

static gceSTATUS
_GenGetGroupIdCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
   gceSTATUS status;
   clsROPERAND *argument;
   clsNAME *groupId;
   gctINT type;
   clsLOPERAND lOperand[1];
   clsROPERAND rOperand[1];

   clsROPERAND    twoROperand[1], oneROperand[1], zeroROperand[1];

/* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
   clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
   gcmASSERT(OperandCount == 1);
   gcmASSERT(OperandsParameters);
   gcmASSERT(IOperand);

   clsROPERAND_InitializeIntOrIVecConstant(twoROperand,
                       clmGenCodeDataType(T_UINT),
                       (gctUINT)2);
   clsROPERAND_InitializeIntOrIVecConstant(oneROperand,
                       clmGenCodeDataType(T_UINT),
                       (gctUINT)1);
   clsROPERAND_InitializeIntOrIVecConstant(zeroROperand,
                       clmGenCodeDataType(T_UINT),
                       (gctUINT)0);
   groupId = _cldUnnamedVariable(clvBUILTIN_GROUP_ID);
/* Allocate all logical registers */
   status = clsNAME_AllocLogicalRegs(Compiler,
                                     CodeGenerator,
                                     groupId);
   if(gcmIS_ERROR(status)) return status;

   clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);
   clsROPERAND_InitializeReg(rOperand,
                             _cldUnnamedVariableRegs(clvBUILTIN_GROUP_ID));
   type = clGetVectorTerminalToken(groupId->decl.dataType->elementType, 1);
   rOperand->dataType = clmGenCodeDataType(type);
   argument = OperandsParameters->rOperands;

   if(argument->isReg) { /* variable argument */
   /* The condition part: t0 == 0 */
      clmGEN_CODE_IF(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo,
                     &OperandsParameters[0].rOperands[0],
                     clvCONDITION_EQUAL,
                     zeroROperand);

        rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
        rOperand->vectorIndex.u.constant = (gctREG_INDEX)0;
        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lOperand,
                                   rOperand));

      /* The false part, "!==0" */
      clmGEN_CODE_ELSE(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo);

     /* The condition part: t0 == 1 */
         clmGEN_CODE_IF(Compiler,
                        CodeGenerator,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        &OperandsParameters[0].rOperands[0],
                        clvCONDITION_EQUAL,
                        oneROperand);

            rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
            rOperand->vectorIndex.u.constant = (gctREG_INDEX)1;
            gcmONERROR(clGenAssignCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       lOperand,
                                       rOperand));

     /* The false part, "!==0 && !==1" */
         clmGEN_CODE_ELSE(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

         /* The condition part: t0 == 2 */
             clmGEN_CODE_IF(Compiler,
                            CodeGenerator,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            &OperandsParameters[0].rOperands[0],
                            clvCONDITION_EQUAL,
                            twoROperand);

                rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
                rOperand->vectorIndex.u.constant = (gctREG_INDEX)2;
                gcmONERROR(clGenAssignCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           lOperand,
                                           rOperand));

         /* The false part, "!==0 && !==1" && "!==2" */
             clmGEN_CODE_ELSE(Compiler,
                              CodeGenerator,
                              PolynaryExpr->exprBase.base.lineNo,
                              PolynaryExpr->exprBase.base.stringNo);

                gcmONERROR(clGenAssignCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           lOperand,
                                           zeroROperand));

             clmGEN_CODE_ENDIF(Compiler,
                               CodeGenerator,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo);

          clmGEN_CODE_ENDIF(Compiler,
                            CodeGenerator,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo);

       clmGEN_CODE_ENDIF(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);
   }
   else {  /* constant argument */
      gctUINT argValue;
      argValue = argument->u.constant.values[0].intValue;
      if(argValue < cldMaxWorkDim) {
         rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
         rOperand->vectorIndex.u.constant = (gctREG_INDEX)argValue;
         gcmONERROR(clGenAssignCode(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    lOperand,
                                    rOperand));
      }
      else {
         gcmONERROR(clGenAssignCode(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    lOperand,
                                    zeroROperand));
      }
   }

OnError:
   return status;
}

static gceSTATUS
_GenGetWorkDimCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
   gceSTATUS status;
   clsNAME *workDim;
   clsLOPERAND lOperand[1];
   clsROPERAND rOperand[1];

/* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
   clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
   gcmASSERT(OperandCount == 0);
   gcmASSERT(IOperand);

   workDim = _cldUnnamedVariable(clvBUILTIN_WORK_DIM);
/* Allocate all logical registers */
   status = clsNAME_AllocLogicalRegs(Compiler,
                                     CodeGenerator,
                                     workDim);
   if(gcmIS_ERROR(status)) return status;

   clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);
   clsROPERAND_InitializeReg(rOperand,
                             _cldUnnamedVariableRegs(clvBUILTIN_WORK_DIM));
   rOperand->dataType = clmGenCodeDataType(T_INT);
   rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
   rOperand->vectorIndex.u.constant = (gctREG_INDEX)0;

   return clGenAssignCode(Compiler,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo,
                          lOperand,
                          rOperand);
}

static gceSTATUS
_GenGetGlobalSizeCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
   gceSTATUS status;
   clsROPERAND *argument;
   clsNAME *globalSize;
   clsLOPERAND lOperand[1];
   clsROPERAND rOperand[1];
   clsROPERAND    twoROperand[1], oneROperand[1], zeroROperand[1];


/* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
   clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
   gcmASSERT(OperandCount == 1);
   gcmASSERT(OperandsParameters);
   gcmASSERT(IOperand);

   clsROPERAND_InitializeIntOrIVecConstant(twoROperand,
                       clmGenCodeDataType(T_INT),
                       (gctUINT)2);
   clsROPERAND_InitializeIntOrIVecConstant(oneROperand,
                       clmGenCodeDataType(T_INT),
                       (gctUINT)1);
   clsROPERAND_InitializeIntOrIVecConstant(zeroROperand,
                       clmGenCodeDataType(T_INT),
                       (gctUINT)0);
   globalSize = _cldUnnamedVariable(clvBUILTIN_GLOBAL_SIZE);
/* Allocate all logical registers */
   status = clsNAME_AllocLogicalRegs(Compiler,
                                     CodeGenerator,
                                     globalSize);
   if(gcmIS_ERROR(status)) return status;

   clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);
   clsROPERAND_InitializeReg(rOperand,
                             _cldUnnamedVariableRegs(clvBUILTIN_GLOBAL_SIZE));
   rOperand->dataType = clmGenCodeDataType(T_INT);
   argument = OperandsParameters->rOperands;

   if(argument->isReg) { /* variable argument */
   /* The condition part: t0 == 0 */
      clmGEN_CODE_IF(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo,
                     &OperandsParameters[0].rOperands[0],
                     clvCONDITION_EQUAL,
                     zeroROperand);

        rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
        rOperand->vectorIndex.u.constant = (gctREG_INDEX)0;
        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lOperand,
                                   rOperand));

      /* The false part, "!==0" */
      clmGEN_CODE_ELSE(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo);

     /* The condition part: t0 == 1 */
         clmGEN_CODE_IF(Compiler,
                        CodeGenerator,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        &OperandsParameters[0].rOperands[0],
                        clvCONDITION_EQUAL,
                        oneROperand);

            rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
            rOperand->vectorIndex.u.constant = (gctREG_INDEX)1;
            gcmONERROR(clGenAssignCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       lOperand,
                                       rOperand));

     /* The false part, "!==0 && !==1" */
         clmGEN_CODE_ELSE(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

         /* The condition part: t0 == 2 */
             clmGEN_CODE_IF(Compiler,
                            CodeGenerator,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            &OperandsParameters[0].rOperands[0],
                            clvCONDITION_EQUAL,
                            twoROperand);

                rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
                rOperand->vectorIndex.u.constant = (gctREG_INDEX)2;
                gcmONERROR(clGenAssignCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           lOperand,
                                           rOperand));

         /* The false part, "!==0 && !==1" && "!==2" */
             clmGEN_CODE_ELSE(Compiler,
                              CodeGenerator,
                              PolynaryExpr->exprBase.base.lineNo,
                              PolynaryExpr->exprBase.base.stringNo);

                gcmONERROR(clGenAssignCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           lOperand,
                                           oneROperand));

             clmGEN_CODE_ENDIF(Compiler,
                               CodeGenerator,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo);

          clmGEN_CODE_ENDIF(Compiler,
                            CodeGenerator,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo);

       clmGEN_CODE_ENDIF(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);
   }
   else {  /* constant argument */
      gctUINT argValue;
      argValue = argument->u.constant.values[0].intValue;
      if(argValue < cldMaxWorkDim) {
         rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
         rOperand->vectorIndex.u.constant = (gctREG_INDEX)argValue;
         gcmONERROR(clGenAssignCode(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    lOperand,
                                    rOperand));
      }
      else {
         gcmONERROR(clGenAssignCode(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    lOperand,
                                    oneROperand));
      }
   }

OnError:
   return status;
}

static gceSTATUS
_GenGetLocalSizeCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
   gceSTATUS status;
   clsROPERAND *argument;
   clsNAME *localSize;
   clsLOPERAND lOperand[1];
   clsROPERAND rOperand[1];
   clsROPERAND    twoROperand[1], oneROperand[1], zeroROperand[1];

/* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
   clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
   gcmASSERT(OperandCount == 1);
   gcmASSERT(OperandsParameters);
   gcmASSERT(IOperand);

   clsROPERAND_InitializeIntOrIVecConstant(twoROperand,
                       clmGenCodeDataType(T_INT),
                       (gctUINT)2);
   clsROPERAND_InitializeIntOrIVecConstant(oneROperand,
                       clmGenCodeDataType(T_INT),
                       (gctUINT)1);
   clsROPERAND_InitializeIntOrIVecConstant(zeroROperand,
                       clmGenCodeDataType(T_INT),
                       (gctUINT)0);
   localSize = _cldUnnamedVariable(clvBUILTIN_LOCAL_SIZE);
/* Allocate all logical registers */
   status = clsNAME_AllocLogicalRegs(Compiler,
                                     CodeGenerator,
                                     localSize);
   if(gcmIS_ERROR(status)) return status;

   clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);
   clsROPERAND_InitializeReg(rOperand,
                             _cldUnnamedVariableRegs(clvBUILTIN_LOCAL_SIZE));
   rOperand->dataType = clmGenCodeDataType(T_INT);
   argument = OperandsParameters->rOperands;

   if(argument->isReg) { /* variable argument */
   /* The condition part: t0 == 0 */
      clmGEN_CODE_IF(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo,
                     &OperandsParameters[0].rOperands[0],
                     clvCONDITION_EQUAL,
                     zeroROperand);

        rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
        rOperand->vectorIndex.u.constant = (gctREG_INDEX)0;
        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lOperand,
                                   rOperand));

      /* The false part, "!==0" */
      clmGEN_CODE_ELSE(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo);

     /* The condition part: t0 == 1 */
         clmGEN_CODE_IF(Compiler,
                        CodeGenerator,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        &OperandsParameters[0].rOperands[0],
                        clvCONDITION_EQUAL,
                        oneROperand);

            rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
            rOperand->vectorIndex.u.constant = (gctREG_INDEX)1;
            gcmONERROR(clGenAssignCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       lOperand,
                                       rOperand));

     /* The false part, "!==0 && !==1" */
         clmGEN_CODE_ELSE(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

         /* The condition part: t0 == 2 */
             clmGEN_CODE_IF(Compiler,
                            CodeGenerator,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            &OperandsParameters[0].rOperands[0],
                            clvCONDITION_EQUAL,
                            twoROperand);

                rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
                rOperand->vectorIndex.u.constant = (gctREG_INDEX)2;
                gcmONERROR(clGenAssignCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           lOperand,
                                           rOperand));

         /* The false part, "!==0 && !==1" && "!==2" */
             clmGEN_CODE_ELSE(Compiler,
                              CodeGenerator,
                              PolynaryExpr->exprBase.base.lineNo,
                              PolynaryExpr->exprBase.base.stringNo);

                gcmONERROR(clGenAssignCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           lOperand,
                                           oneROperand));

             clmGEN_CODE_ENDIF(Compiler,
                               CodeGenerator,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo);

          clmGEN_CODE_ENDIF(Compiler,
                            CodeGenerator,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo);

       clmGEN_CODE_ENDIF(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);
   }
   else {  /* constant argument */
      gctUINT argValue;
      argValue = argument->u.constant.values[0].intValue;
      if(argValue < cldMaxWorkDim) {
         rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
         rOperand->vectorIndex.u.constant = (gctREG_INDEX)argValue;
         gcmONERROR(clGenAssignCode(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    lOperand,
                                    rOperand));
      }
      else {
         gcmONERROR(clGenAssignCode(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    lOperand,
                                    oneROperand));
      }
   }

OnError:
   return status;
}

static gceSTATUS
_GenGetNumGroupsCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
   gceSTATUS status;
   clsROPERAND *argument;
   clsNAME *numGroups;
   clsLOPERAND lOperand[1];
   clsROPERAND rOperand[1];
   clsROPERAND    twoROperand[1], oneROperand[1], zeroROperand[1];

/* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
   clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
   gcmASSERT(OperandCount == 1);
   gcmASSERT(OperandsParameters);
   gcmASSERT(IOperand);


   clsROPERAND_InitializeIntOrIVecConstant(twoROperand,
                       clmGenCodeDataType(T_INT),
                       (gctUINT)2);
   clsROPERAND_InitializeIntOrIVecConstant(oneROperand,
                       clmGenCodeDataType(T_INT),
                       (gctUINT)1);
   clsROPERAND_InitializeIntOrIVecConstant(zeroROperand,
                       clmGenCodeDataType(T_INT),
                       (gctUINT)0);
   numGroups = _cldUnnamedVariable(clvBUILTIN_NUM_GROUPS);
/* Allocate all logical registers */
   status = clsNAME_AllocLogicalRegs(Compiler,
                                     CodeGenerator,
                                     numGroups);
   if(gcmIS_ERROR(status)) return status;

   clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);
   clsROPERAND_InitializeReg(rOperand,
                             _cldUnnamedVariableRegs(clvBUILTIN_NUM_GROUPS));
   rOperand->dataType = clmGenCodeDataType(T_INT);
   argument = OperandsParameters->rOperands;

   if(argument->isReg) { /* variable argument */
   /* The condition part: t0 == 0 */
      clmGEN_CODE_IF(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo,
                     &OperandsParameters[0].rOperands[0],
                     clvCONDITION_EQUAL,
                     zeroROperand);

        rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
        rOperand->vectorIndex.u.constant = (gctREG_INDEX)0;
        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lOperand,
                                   rOperand));

      /* The false part, "!==0" */
      clmGEN_CODE_ELSE(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo);

     /* The condition part: t0 == 1 */
         clmGEN_CODE_IF(Compiler,
                        CodeGenerator,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        &OperandsParameters[0].rOperands[0],
                        clvCONDITION_EQUAL,
                        oneROperand);

            rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
            rOperand->vectorIndex.u.constant = (gctREG_INDEX)1;
            gcmONERROR(clGenAssignCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       lOperand,
                                       rOperand));

     /* The false part, "!==0 && !==1" */
         clmGEN_CODE_ELSE(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

         /* The condition part: t0 == 2 */
             clmGEN_CODE_IF(Compiler,
                            CodeGenerator,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            &OperandsParameters[0].rOperands[0],
                            clvCONDITION_EQUAL,
                            twoROperand);

                rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
                rOperand->vectorIndex.u.constant = (gctREG_INDEX)2;
                gcmONERROR(clGenAssignCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           lOperand,
                                           rOperand));

         /* The false part, "!==0 && !==1" && "!==2" */
             clmGEN_CODE_ELSE(Compiler,
                              CodeGenerator,
                              PolynaryExpr->exprBase.base.lineNo,
                              PolynaryExpr->exprBase.base.stringNo);

                gcmONERROR(clGenAssignCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           lOperand,
                                           oneROperand));

             clmGEN_CODE_ENDIF(Compiler,
                               CodeGenerator,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo);

          clmGEN_CODE_ENDIF(Compiler,
                            CodeGenerator,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo);

       clmGEN_CODE_ENDIF(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);
   }
   else {  /* constant argument */
      gctUINT argValue;
      argValue = argument->u.constant.values[0].intValue;
      if(argValue < cldMaxWorkDim) {
         rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
         rOperand->vectorIndex.u.constant = (gctREG_INDEX)argValue;
         gcmONERROR(clGenAssignCode(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    lOperand,
                                    rOperand));
      }
      else {
         gcmONERROR(clGenAssignCode(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    lOperand,
                                    oneROperand));
      }
   }

OnError:
   return status;
}

static gceSTATUS
_GenGetGlobalOffsetCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
   gceSTATUS status;
   clsROPERAND *argument;
   clsNAME *globalOffset;
   clsLOPERAND lOperand[1];
   clsROPERAND rOperand[1];
   clsROPERAND    twoROperand[1], oneROperand[1], zeroROperand[1];

/* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
   clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
   gcmASSERT(OperandCount == 1);
   gcmASSERT(OperandsParameters);
   gcmASSERT(IOperand);

   clsROPERAND_InitializeIntOrIVecConstant(twoROperand,
                       clmGenCodeDataType(T_INT),
                       (gctUINT)2);
   clsROPERAND_InitializeIntOrIVecConstant(oneROperand,
                       clmGenCodeDataType(T_INT),
                       (gctUINT)1);
   clsROPERAND_InitializeIntOrIVecConstant(zeroROperand,
                       clmGenCodeDataType(T_INT),
                       (gctUINT)0);
   globalOffset = _cldUnnamedVariable(clvBUILTIN_GLOBAL_OFFSET);
/* Allocate all logical registers */
   status = clsNAME_AllocLogicalRegs(Compiler,
                                     CodeGenerator,
                                     globalOffset);
   if(gcmIS_ERROR(status)) return status;

   clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);
   clsROPERAND_InitializeReg(rOperand,
                             _cldUnnamedVariableRegs(clvBUILTIN_GLOBAL_OFFSET));
   rOperand->dataType = clmGenCodeDataType(T_INT);
   argument = OperandsParameters->rOperands;

   if(argument->isReg) { /* variable argument */
   /* The condition part: t0 == 0 */
      clmGEN_CODE_IF(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo,
                     &OperandsParameters[0].rOperands[0],
                     clvCONDITION_EQUAL,
                     zeroROperand);

        rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
        rOperand->vectorIndex.u.constant = (gctREG_INDEX)0;
        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lOperand,
                                   rOperand));

      /* The false part, "!==0" */
      clmGEN_CODE_ELSE(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo);

     /* The condition part: t0 == 1 */
         clmGEN_CODE_IF(Compiler,
                        CodeGenerator,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        &OperandsParameters[0].rOperands[0],
                        clvCONDITION_EQUAL,
                        oneROperand);

            rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
            rOperand->vectorIndex.u.constant = (gctREG_INDEX)1;
            gcmONERROR(clGenAssignCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       lOperand,
                                       rOperand));

     /* The false part, "!==0 && !==1" */
         clmGEN_CODE_ELSE(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

         /* The condition part: t0 == 2 */
             clmGEN_CODE_IF(Compiler,
                            CodeGenerator,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            &OperandsParameters[0].rOperands[0],
                            clvCONDITION_EQUAL,
                            twoROperand);

                rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
                rOperand->vectorIndex.u.constant = (gctREG_INDEX)2;
                gcmONERROR(clGenAssignCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           lOperand,
                                           rOperand));

         /* The false part, "!==0 && !==1" && "!==2" */
             clmGEN_CODE_ELSE(Compiler,
                              CodeGenerator,
                              PolynaryExpr->exprBase.base.lineNo,
                              PolynaryExpr->exprBase.base.stringNo);

                gcmONERROR(clGenAssignCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           lOperand,
                                           zeroROperand));

             clmGEN_CODE_ENDIF(Compiler,
                               CodeGenerator,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo);

          clmGEN_CODE_ENDIF(Compiler,
                            CodeGenerator,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo);

       clmGEN_CODE_ENDIF(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);
   }
   else {  /* constant argument */
      gctUINT argValue;
      argValue = argument->u.constant.values[0].intValue;
      if(argValue < cldMaxWorkDim) {
         rOperand->vectorIndex.mode = clvINDEX_CONSTANT;
         rOperand->vectorIndex.u.constant = (gctREG_INDEX)argValue;
         gcmONERROR(clGenAssignCode(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    lOperand,
                                    rOperand));
      }
      else {
         gcmONERROR(clGenAssignCode(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    lOperand,
                                    zeroROperand));
      }
   }

OnError:
   return status;
}

static gceSTATUS
_GenBarrierCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    clsROPERAND     memoryScope[1];
    clsROPERAND     memorySemantic[1];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand == gcvNULL);

    clsROPERAND_InitializeIntOrIVecConstant(memoryScope,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) gcSL_MEMORY_SCOPE_WORKGROUP);

    clsROPERAND_InitializeIntOrIVecConstant(memorySemantic,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) gcSL_MEMORY_SEMANTIC_ACQUIRERELEASE);

    status = clGenGenericNullTargetCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        clvOPCODE_BARRIER,
                                        memoryScope,
                                        memorySemantic);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenMemFenceCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand == gcvNULL);

        status = clGenGenericNullTargetCode(Compiler,
                                            PolynaryExpr->exprBase.base.lineNo,
                                            PolynaryExpr->exprBase.base.stringNo,
                                            clvOPCODE_MEM_FENCE,
                                            &OperandsParameters[0].rOperands[0],
                                            gcvNULL);
        if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

#endif /* __gc_cl_built_ins_ks_h_ */
