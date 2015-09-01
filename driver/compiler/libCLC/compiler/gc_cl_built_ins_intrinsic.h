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


#ifndef __gc_cl_built_ins_intrinsic_h_
#define __gc_cl_built_ins_intrinsic_h_

/******************************************
struct _clsINTRINCSIC_BUILTIN_FUNCTION
{
    cleEXTENSION    extension;
    gctCONST_STRING symbol;
    gceINTRINSICS_KIND intrinsicKind;
    gctCONST_STRING    nameInLibrary;
    gctINT          returnType;
    gctUINT         paramCount;
    gctINT          paramTypes[clmMAX_BUILT_IN_PARAMETER_COUNT];
    gctUINT8        ptrLevels[clmMAX_BUILT_IN_PARAMETER_COUNT];
    gctUINT8        typeConvertible[clmMAX_BUILT_IN_PARAMETER_COUNT];
    gctBOOL         isInline;
    gctBOOL         hasWriteArg;
    gctBOOL         passArgByRef;
    gctBOOL         hasVarArg;
}
*************************************************/

static clsINTRINSIC_BUILTIN_FUNCTION IntrinsicBuiltinFunctions[] =
{
    /* Intrinsic builtin functions */

    {clvEXTENSION_NONE,  "mad_sat",           gceINTRIN_source, "_viv_madsat_long",   T_LONG,  3, {T_LONG,  T_LONG,  T_LONG},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "mad_sat",           gceINTRIN_source, "_viv_madsat_ulong",  T_ULONG, 3, {T_ULONG, T_ULONG, T_ULONG}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_mul_long",     gceINTRIN_source, "_viv_mul_long",      T_LONG,  2, {T_LONG,  T_LONG},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_mul_ulong",    gceINTRIN_source, "_viv_mul_ulong",     T_ULONG, 2, {T_ULONG, T_ULONG}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_mulhi_long",   gceINTRIN_source, "_viv_mulhi_long",    T_LONG,  2, {T_LONG,  T_LONG},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_mulhi_ulong",  gceINTRIN_source, "_viv_mulhi_ulong",   T_ULONG, 2, {T_ULONG, T_ULONG}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "nextafter",         gceINTRIN_source, "_viv_nextafter",     T_FLOAT, 2, {T_FLOAT, T_FLOAT}, {0}, {0}, 1},
};

#define _cldIntrinsicBuiltinFunctionCount (sizeof(IntrinsicBuiltinFunctions) / sizeof(clsINTRINSIC_BUILTIN_FUNCTION))

#endif /* __gc_cl_built_ins_intrinsics_h_ */
