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

#define OPCODE_ALIGN       (24)
#define OPERAND_PLUS_ALIGN (24)
#define _GC_OBJ_ZONE    gcvZONE_COMPILER

typedef enum _VIR_DUMP_CONST_KIND
{
    VIR_DUMP_CONST_NONE,
    VIR_DUMP_CONST_INVALID,
    VIR_DUMP_CONST_32TOPOINTER,
}
VIR_DumpConstKind;

typedef struct _VIR_DUMP_CONST_FORMAT
{
    gctUINT32         Type;
    char             *Format;
    gctUINT32         Count;
    gctINT32          Size;
    VIR_DumpConstKind Kind;
    void* (*pConvert32ToPointer)(gctUINT32);
}
VIR_DumpConstFormat;

typedef struct _VIR_DUMP_TYPE_FORMAT
{
    gctUINT           FullType:1;
    gctUINT           Ellipsis:1;
    gctUINT           NoTypeQual:1;     /* no const/volatile/restrict type quailifer */
    gctUINT           NoAddrQual:1;     /* no const/private/local/global address qualifier */
    gctUINT           NoWSBeforeStar:1; /* no ' ' bfore *: int* */
    gctUINT           NoAccessQual:1;   /* no read_only/write_only/read_write access qualifier */
    gctUINT           Indent  :30;
}
VIR_DumpTypeFormat;

static VSC_ErrCode
_DumpType(
    IN OUT VIR_Dumper     *Dumper,
    IN VIR_Type           *Type,
    IN gctBOOL             LVal,
    IN VIR_DumpTypeFormat  TypeFormat
    );

static VSC_ErrCode
_DumpTypeWithSpace(
    IN OUT VIR_Dumper     *Dumper,
    IN VIR_Type           *Type,
    IN gctBOOL             LVal,
    IN VIR_DumpTypeFormat  TypeFormat
    );

static VSC_ErrCode
_DumpSymbol(
    IN OUT VIR_Dumper *  Dumper,
    IN  VIR_Symbol      *Sym,
    IN  gctBOOL          LVal,
    IN  gctBOOL          FullInfo
    );

static VSC_ErrCode
_DumpOperand(
    IN OUT VIR_Dumper *  Dumper,
    IN VIR_Instruction * Inst,
    IN VIR_Operand  *    Operand,
    IN gctBOOL           DumpType
    );

static VSC_ErrCode
_DumpBasicBlockInOutLength(
    IN OUT VIR_Dumper   *Dumper,
    IN VIR_BB           *Bb
    );

static VSC_ErrCode
_DumpMapedRegister(
    IN OUT VIR_Dumper *Dumper,
    IN VIR_Type       *Type,
    IN gctUINT         RegStart
    );

static VSC_ErrCode
_DumpMapedRegisterWithRegCount(
    IN OUT VIR_Dumper *Dumper,
    IN VIR_Type       *Type,
    IN gctUINT         RegStart
    );

static void *
_BoolToString(
    IN gctUINT32 Bool
    )
{
    if(Bool) return (void *)"true";
    return (void *)"false";
}

static gctCONST_STRING spaceaddr[] = {"", "global_space ", "const_space ", "local_space "};
static gctCONST_STRING operand_precision[] = { ""/* .dp */, ".lp", ".mp", ".hp", ".anyp"};
static gctCONST_STRING symbol_precision[]  = { ""/* dp */, "lp ", "mp ", "hp ", "anyp "};

static VIR_DumpConstFormat formats[] = {
    { VIR_TYPE_UNKNOWN, "%s", 0, 0, VIR_DUMP_CONST_INVALID, gcvNULL},
    { VIR_TYPE_VOID, "%s", 0, 0, VIR_DUMP_CONST_INVALID, gcvNULL},
    { VIR_TYPE_FLOAT32, "%f", 1, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_FLOAT16, "%f", 1, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT32, "%d", 1, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT16, "%hd", 1, 16, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT8, "%hhd", 1, 8, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT32, "%u", 1, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT16, "%hu", 1, 16, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT8, "%hhu", 1, 8, VIR_DUMP_CONST_NONE, gcvNULL},

    { VIR_TYPE_SNORM16, "%hx", 1, 16, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_SNORM8, "%hhx", 1, 8, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UNORM16, "%hx", 1, 16, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UNORM8, "%hhx", 1, 8, VIR_DUMP_CONST_NONE, gcvNULL},

    { VIR_TYPE_INT64, "%lld", 1, 64, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT64, "%llu", 1, 64, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_FLOAT64, "%f", 1, 32, VIR_DUMP_CONST_NONE, gcvNULL}, /* treat as float */
    { VIR_TYPE_BOOLEAN, "%s", 1, -1, VIR_DUMP_CONST_32TOPOINTER, _BoolToString},

    { VIR_TYPE_FLOAT_X2, "%f", 2, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_FLOAT_X3, "%f", 3, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_FLOAT_X4, "%f", 4, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_FLOAT_X8, "%f", 8, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_FLOAT_X16, "%f", 16, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_FLOAT_X32, "%f", 32, 32, VIR_DUMP_CONST_NONE, gcvNULL},

    { VIR_TYPE_FLOAT16_X2, "%f", 2, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_FLOAT16_X3, "%f", 3, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_FLOAT16_X4, "%f", 4, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_FLOAT16_X8, "%f", 8, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_FLOAT16_X16, "%f", 16, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_FLOAT16_X32, "%f", 32, 32, VIR_DUMP_CONST_NONE, gcvNULL},

    { VIR_TYPE_FLOAT64_X2, "%f", 2, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_FLOAT64_X3, "%f", 3, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_FLOAT64_X4, "%f", 4, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_FLOAT64_X8, "%f", 8, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_FLOAT64_X16, "%f", 16, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_FLOAT64_X32, "%f", 32, 32, VIR_DUMP_CONST_NONE, gcvNULL},

    { VIR_TYPE_BOOLEAN_X2, "%s", 2, -1, VIR_DUMP_CONST_32TOPOINTER, _BoolToString},
    { VIR_TYPE_BOOLEAN_X3, "%s", 3, -1, VIR_DUMP_CONST_32TOPOINTER, _BoolToString},
    { VIR_TYPE_BOOLEAN_X4, "%s", 4, -1, VIR_DUMP_CONST_32TOPOINTER, _BoolToString},
    { VIR_TYPE_BOOLEAN_X8, "%s", 8, -1, VIR_DUMP_CONST_32TOPOINTER, _BoolToString},
    { VIR_TYPE_BOOLEAN_X16, "%s", 16, -1, VIR_DUMP_CONST_32TOPOINTER, _BoolToString},
    { VIR_TYPE_BOOLEAN_X32, "%s", 32, -1, VIR_DUMP_CONST_32TOPOINTER, _BoolToString},

    { VIR_TYPE_INTEGER_X2, "%d", 2, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INTEGER_X3, "%d", 3, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INTEGER_X4, "%d", 4, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INTEGER_X8, "%d", 8, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INTEGER_X16, "%d", 16, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INTEGER_X32, "%d", 32, 32, VIR_DUMP_CONST_NONE, gcvNULL},

    { VIR_TYPE_UINT_X2, "%u", 2, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT_X3, "%u", 3, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT_X4, "%u", 4, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT_X8, "%u", 8, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT_X16, "%u", 16, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT_X32, "%u", 32, 32, VIR_DUMP_CONST_NONE, gcvNULL},

    { VIR_TYPE_UINT8_X2, "%hhu", 2, 8, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT8_X3, "%hhu", 3, 8, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT8_X4, "%hhu", 4, 8, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT8_X8, "%hhu", 8, 8, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT8_X16, "%hhu", 16, 8, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT8_X32, "%hhu", 32, 8, VIR_DUMP_CONST_NONE, gcvNULL},

    { VIR_TYPE_INT8_X2, "%hhd", 2, 8, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT8_X3, "%hhd", 3, 8, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT8_X4, "%hhd", 4, 8, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT8_X8, "%hhd", 8, 8, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT8_X16, "%hhd", 16, 8, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT8_X32, "%hhd", 32, 8, VIR_DUMP_CONST_NONE, gcvNULL},

    { VIR_TYPE_UINT16_X2, "%hu", 2, 16, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT16_X3, "%hu", 3, 16, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT16_X4, "%hu", 4, 16, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT16_X8, "%hu", 8, 16, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT16_X16, "%hu", 16, 16, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT16_X32, "%hu", 32, 16, VIR_DUMP_CONST_NONE, gcvNULL},

    { VIR_TYPE_INT16_X2, "%hd", 2, 16, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT16_X3, "%hd", 3, 16, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT16_X4, "%hd", 4, 16, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT16_X8, "%hd", 8, 16, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT16_X16, "%hd", 16, 16, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT16_X32, "%hd", 32, 16, VIR_DUMP_CONST_NONE, gcvNULL},

    { VIR_TYPE_UINT64_X2, "%llu", 2, 64, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT64_X3, "%llu", 3, 64, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT64_X4, "%llu", 4, 64, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT64_X8, "%llu", 8, 64, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT64_X16, "%llu", 16, 64, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT64_X32, "%llu", 32, 64, VIR_DUMP_CONST_NONE, gcvNULL},

    { VIR_TYPE_INT64_X2, "%lld", 2, 64, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT64_X3, "%lld", 3, 64, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT64_X4, "%lld", 4, 64, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT64_X8, "%lld", 8, 64, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT64_X16, "%lld", 16, 64, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT64_X32, "%lld", 32, 64, VIR_DUMP_CONST_NONE, gcvNULL},
    /* packed float16 (2 bytes per element) */
    { VIR_TYPE_FLOAT16_P2, "%f", 2, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_FLOAT16_P3, "%f", 3, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_FLOAT16_P4, "%f", 4, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_FLOAT16_P8, "%f", 8, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_FLOAT16_P16, "%f", 16, 32, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_FLOAT16_P32, "%f", 32, 32, VIR_DUMP_CONST_NONE, gcvNULL},

    /* packed boolean (1 byte per element) */
    { VIR_TYPE_BOOLEAN_P2, "%s", 2, -1, VIR_DUMP_CONST_32TOPOINTER, _BoolToString},
    { VIR_TYPE_BOOLEAN_P3, "%s", 3, -1, VIR_DUMP_CONST_32TOPOINTER, _BoolToString},
    { VIR_TYPE_BOOLEAN_P4, "%s", 4, -1, VIR_DUMP_CONST_32TOPOINTER, _BoolToString},
    { VIR_TYPE_BOOLEAN_P8, "%s", 8, -1, VIR_DUMP_CONST_32TOPOINTER, _BoolToString},
    { VIR_TYPE_BOOLEAN_P16, "%s", 16, -1, VIR_DUMP_CONST_32TOPOINTER, _BoolToString},
    { VIR_TYPE_BOOLEAN_P32, "%s", 32, -1, VIR_DUMP_CONST_32TOPOINTER, _BoolToString},

    /* uchar vectors (1 byte per element) */
    { VIR_TYPE_UINT8_P2, "%hhu", 2, 8, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT8_P3, "%hhu", 3, 8, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT8_P4, "%hhu", 4, 8, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT8_P8, "%hhu", 8, 8, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT8_P16, "%hhu", 16, 8, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT8_P32, "%hhu", 32, 8, VIR_DUMP_CONST_NONE, gcvNULL},

    /* char vectors (1 byte per element) */
    { VIR_TYPE_INT8_P2, "%hhd", 2, 8, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT8_P3, "%hhd", 3, 8, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT8_P4, "%hhd", 4, 8, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT8_P8, "%hhd", 8, 8, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT8_P16, "%hhd", 16, 8, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT8_P32, "%hhd", 32, 8, VIR_DUMP_CONST_NONE, gcvNULL},

    /* ushort vectors (2 bytes per element) */
    { VIR_TYPE_UINT16_P2, "%hu", 2, 16, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT16_P3, "%hu", 3, 16, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT16_P4, "%hu", 4, 16, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT16_P8, "%hu", 8, 16, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT16_P16, "%hu", 16, 16, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_UINT16_P32, "%hu", 32, 16, VIR_DUMP_CONST_NONE, gcvNULL},

    /* short vectors (2 bytes per element) */
    { VIR_TYPE_INT16_P2, "%hd", 2, 16, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT16_P3, "%hd", 3, 16, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT16_P4, "%hd", 4, 16, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT16_P8, "%hd", 8, 16, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT16_P16, "%hd", 16, 16, VIR_DUMP_CONST_NONE, gcvNULL},
    { VIR_TYPE_INT16_P32, "%hd", 32, 16, VIR_DUMP_CONST_NONE, gcvNULL},
};

static void
_DumpTab(
    IN OUT VIR_Dumper *Dumper
    )
{
    if(*Dumper->baseDumper.pOffset % 4 == 0)
    {
        VERIFY_OK(
            VIR_LOG(Dumper, "    "));
    }
    else if(*Dumper->baseDumper.pOffset % 4 == 1)
    {
        VERIFY_OK(
            VIR_LOG(Dumper, "   "));
    }
    else if(*Dumper->baseDumper.pOffset % 4 == 2)
    {
        VERIFY_OK(
            VIR_LOG(Dumper, "  "));
    }
    else if(*Dumper->baseDumper.pOffset % 4 == 3)
    {
        VERIFY_OK(
            VIR_LOG(Dumper, " "));
    }
}

static void
_DumpAlign(
    IN OUT VIR_Dumper *Dumper,
    IN     gctSIZE_T   Align
    )
{
    gcmASSERT(Align % 4 == 0);
    while(*Dumper->baseDumper.pOffset < Align)
    {
        _DumpTab(Dumper);
    }
}

static void
_DumpIndent(
    IN OUT VIR_Dumper           *Dumper,
    IN     VIR_DumpTypeFormat    Format
    )
{
    gctSIZE_T i = 0;

    while(i < Format.Indent)
    {
        _DumpTab(Dumper);
        ++i;
    }
}

static gctCONST_STRING
_GetStorageClassString(
    IN VIR_StorageClass clas
    )
{
    gctUINT storageClass = (gctUINT)clas;
    switch(storageClass)
    {
    case VIR_STORAGE_UNKNOWN:
        return "";
    case VIR_STORAGE_INPUT:
        return "in ";
    case VIR_STORAGE_OUTPUT:
        return "out ";
    case VIR_STORAGE_INOUTPUT:
        return "inout ";
    case VIR_STORAGE_LOCAL:
        return "local ";
    case VIR_STORAGE_GLOBAL:
        return "global ";
    case VIR_STORAGE_INPARM:
        return "inparm ";
    case VIR_STORAGE_OUTPARM:
        return "outparm ";
    case VIR_STORAGE_INOUTPARM:
        return "ioparm ";
    case VIR_STORAGE_FUNCSTATIC:
        return "func_static ";
    case VIR_STORAGE_FILESTATIC:
        return "file_static ";
    case VIR_STORAGE_EXTERN:
        return "extern ";
    case VIR_STORAGE_REGISTER:
        return "register ";
    case VIR_STORAGE_PERPATCH_INPUT:
        return "patch in ";
    case VIR_STORAGE_PERPATCH_OUTPUT:
        return "patch out ";
    case VIR_STORAGE_PERPATCH_INOUT:
        return "patch in out ";
    case VIR_STORAGE_SHARED_VAR:
        return "shared";
    case VIR_STORAGE_INDEX_REGISTER:
        return "addr_reg ";
    case VIR_UNIFORM_EXTRA_LAYER:
        return "";
    case VIR_UNIFORM_TEXELBUFFER_TO_IMAGE:
        return "";
    default:
        gcmASSERT(0);
        return "";
    }
}


static gctCONST_STRING
_GetUniformKindString(
    IN VIR_UniformKind kind
    )
{
    switch(kind)
    {
    case VIR_UNIFORM_NORMAL:
        return "uniform ";
    case VIR_UNIFORM_KERNEL_ARG:
        return "kernel_arg ";
    case VIR_UNIFORM_KERNEL_ARG_LOCAL:
        return "kernel_arg_local ";
    case VIR_UNIFORM_KERNEL_ARG_SAMPLER:
        return "kernel_arg_sampler ";
    case VIR_UNIFORM_LOCAL_ADDRESS_SPACE:
        return "local_addr_space ";
    case VIR_UNIFORM_PRIVATE_ADDRESS_SPACE:
        return "private_addr_space ";
    case VIR_UNIFORM_CONSTANT_ADDRESS_SPACE:
        return "const_addr_space ";
    case VIR_UNIFORM_GLOBAL_SIZE:
        return "global_size ";
    case VIR_UNIFORM_GLOBAL_WORK_SCALE:
        return "global_work_scale";
    case VIR_UNIFORM_LOCAL_SIZE:
        return "local_size ";
    case VIR_UNIFORM_NUM_GROUPS:
        return "num_groups ";
    case VIR_UNIFORM_GLOBAL_OFFSET:
        return "global_offset ";
    case VIR_UNIFORM_WORK_DIM:
        return "work_dim ";
    case VIR_UNIFORM_KERNEL_ARG_CONSTANT:
        return "kernel_arg_const ";
    case VIR_UNIFORM_KERNEL_ARG_LOCAL_MEM_SIZE:
        return "kernel_arg_local_mem_size ";
    case VIR_UNIFORM_KERNEL_ARG_PRIVATE:
        return "kernel_arg_private ";
    case VIR_UNIFORM_LOADTIME_CONSTANT:
        return "localtime_const ";
    case VIR_UNIFORM_TRANSFORM_FEEDBACK_BUFFER:
        return "trans_feedback_buffer ";
    case VIR_UNIFORM_TRANSFORM_FEEDBACK_STATE:
        return "trans_feedback_state ";
    case VIR_UNIFORM_BLOCK_MEMBER:
        return "uniform block_member ";
    case VIR_UNIFORM_UNIFORM_BLOCK_ADDRESS:
        return "uniform block_addr ";
    case VIR_UNIFORM_STORAGE_BLOCK_ADDRESS:
        return "storage block_addr ";
    case VIR_UNIFORM_LOD_MIN_MAX:
        return "lod_min_max ";
    case VIR_UNIFORM_LEVEL_BASE_SIZE:
        return "level_base_size ";
    case VIR_UNIFORM_LEVELS_SAMPLES:
        return "levels_samples ";
    case VIR_UNIFORM_STRUCT:
        return "uniform_struct";
    case VIR_UNIFORM_SAMPLE_LOCATION:
        return "sample location";
    case VIR_UNIFORM_ENABLE_MULTISAMPLE_BUFFERS:
        return "multiSample buffers";
    case VIR_UNIFORM_TEMP_REG_SPILL_MEM_ADDRESS:
        return "uniform_temp_reg_spill_mem_address";
    case VIR_UNIFORM_CONST_BORDER_VALUE:
        return "const_border_value";
    case VIR_UNIFORM_PUSH_CONSTANT:
        return "push_constant";
    case VIR_UNIFORM_SAMPLED_IMAGE:
        return "sampled_image";
    case VIR_UNIFORM_EXTRA_LAYER:
        return "extra_layer";
    case VIR_UNIFORM_BASE_INSTANCE:
        return "base_instance";
    case VIR_UNIFORM_GL_SAMPLER_FOR_IMAGE_T:
        return "gl_sampler_for_image_t";
    case VIR_UNIFORM_GL_IMAGE_FOR_IMAGE_T:
        return "gl_image_for_image_t";
    case VIR_UNIFORM_WORK_THREAD_COUNT:
        return "workThreadCount";
    case VIR_UNIFORM_WORK_GROUP_COUNT:
        return "workGroupCount";
    case VIR_UNIFORM_WORK_GROUP_ID_OFFSET:
        return "workGroupIdOffset";
    case  VIR_UNIFORM_PRINTF_ADDRESS:
        return "printf_address";
    case VIR_UNIFORM_WORKITEM_PRINTF_BUFFER_SIZE:
        return "workitem_printf_buffer_size";
    case VIR_UNIFORM_GENERAL_PATCH:
        return "general_patch";
    case VIR_UNIFORM_TEXELBUFFER_TO_IMAGE:
        return "texelBufferToImage";
    default:
        gcmASSERT(0);
        return "";
    }
}

static gctCONST_STRING
_GetTexldModifierString(
    IN VIR_Operand  *TexldOperand,
    IN Vir_TexldModifier_Name TexldModifierName
    )
{
    switch(TexldModifierName)
    {
    case VIR_TEXLDMODIFIER_BIAS:
        return "bias";

    case VIR_TEXLDMODIFIER_LOD:
        if (VIR_Operand_GetTexModifierFlag(TexldOperand) & VIR_TMFLAG_LOD)
        {
            return "lod";
        }
        else
        {
            gcmASSERT(VIR_Operand_GetTexModifierFlag(TexldOperand) & VIR_TMFLAG_FETCHMS);
            return "fetchms";
        }

    case VIR_TEXLDMODIFIER_DPDX:
        return "dpdx";

    case VIR_TEXLDMODIFIER_DPDY:
        return "dpdy";

    case VIR_TEXLDMODIFIER_GATHERCOMP:
        return "gather_comp";

    case VIR_TEXLDMODIFIER_GATHERREFZ:
        return "gather_refZ";

    case VIR_TEXLDMODIFIER_OFFSET:
        return "offset";
    default:
        gcmASSERT(0);
        return "";
    }
}

static VSC_ErrCode
_DumpVecConst(
    IN OUT VIR_Dumper         *Dumper,
    IN gctUINT                *Value,
    IN VIR_DumpConstFormat    *Format,
    IN VIR_TyFlag              TyFlag
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctSIZE_T  i   = 0;

    gctUINT64 *p64 = (gctUINT64 *)Value;
    gctUINT32 *p32 = (gctUINT32 *)Value;
    gctUINT16 *p16 = (gctUINT16 *)Value;
    gctUINT8  *p8  = (gctUINT8  *)Value;

    void      *pointer = gcvNULL;

    gcmASSERT(Value != gcvNULL &&
        Dumper->baseDumper.pOffset != gcvNULL &&
        Format != gcvNULL &&
        Format->Kind != VIR_DUMP_CONST_INVALID);

    for(i = 0; i < Format->Count; ++i)
    {
        switch(Format->Kind)
        {
        case VIR_DUMP_CONST_NONE:
            break;
        case VIR_DUMP_CONST_32TOPOINTER:
            pointer = Format->pConvert32ToPointer(p32[i]);
            break;
        default:
            gcmASSERT(0);
            break;
        }

        if(TyFlag & VIR_TYFLAG_PACKED)
        {
            switch(Format->Size)
            {
            case -1:
                VERIFY_OK(VIR_LOG(Dumper, Format->Format, pointer));
                break;
            case 64:
                if(TyFlag & VIR_TYFLAG_ISFLOAT)
                {
                    VERIFY_OK(VIR_LOG(Dumper, Format->Format, *(double *)&p64[i]));
                }
                else
                {
                    VERIFY_OK(VIR_LOG(Dumper, Format->Format, p64[i]));
                }
                break;
            case 32:
                if(TyFlag & VIR_TYFLAG_ISFLOAT)
                {
                    VERIFY_OK(VIR_LOG(Dumper, Format->Format, *(gctFLOAT *)&p32[i]));
                    VERIFY_OK(VIR_LOG(Dumper, "[%x]", p32[i]));
                }
                else
                {
                    VERIFY_OK(VIR_LOG(Dumper, Format->Format, p32[i]));
                }
                break;
            case 16:
                VERIFY_OK(VIR_LOG(Dumper, Format->Format, p16[i]));
                break;
            case 8:
                VERIFY_OK(VIR_LOG(Dumper, Format->Format, p8[i]));
                break;
            default:
                gcmASSERT(0);
            }
        }
        else
        {
            switch(Format->Size)
            {
            case -1:
                VERIFY_OK(VIR_LOG(Dumper, Format->Format, pointer));
                break;
            case 64:
                if(TyFlag & VIR_TYFLAG_ISFLOAT)
                {
                    VERIFY_OK(VIR_LOG(Dumper, Format->Format, *(double *)&p64[i]));
                }
                else
                {
                    VERIFY_OK(VIR_LOG(Dumper, Format->Format, p64[i]));
                }
                break;
            case 32:
                if(TyFlag & VIR_TYFLAG_ISFLOAT)
                {
                    VERIFY_OK(VIR_LOG(Dumper, Format->Format, *(gctFLOAT *)&p32[i]));
                    VERIFY_OK(VIR_LOG(Dumper, "[%x]", p32[i]));
                }
                else
                {
                    VERIFY_OK(VIR_LOG(Dumper, Format->Format, p32[i]));
                }
                break;
            case 16:
            case 8:
                VERIFY_OK(VIR_LOG(Dumper, Format->Format, p32[i]));
                break;
            default:
                gcmASSERT(0);
            }
        }

        if(Format->Count != i + 1)
        {
            VERIFY_OK(VIR_LOG(Dumper, ", "));
        }
    }

    return errCode;
}

static VSC_ErrCode
_DumpConst(
    IN OUT VIR_Dumper *  Dumper,
    IN  VIR_Const       *Value
    )
{
    VSC_ErrCode  errCode = VSC_ERR_NONE;
    VIR_Type    *type;
    gctUINT      baseType;
    gcmASSERT(Value != gcvNULL);

    type = VIR_Shader_GetTypeFromId(Dumper->Shader, Value->type);
    if(type == gcvNULL)
    {
        return VSC_ERR_INVALID_ARGUMENT;
    }

    VERIFY_OK(
        VIR_LOG(Dumper,
        "%s(", VIR_Shader_GetTypeNameString(Dumper->Shader, type)));

    baseType = (gctUINT)VIR_Type_GetBaseTypeId(type);
    gcmASSERT(baseType < gcmCOUNTOF(formats) &&
        formats[baseType].Type == baseType);

    errCode = _DumpVecConst(Dumper,
        (gctUINT *)&Value->value.vecVal,
        &formats[baseType],
        VIR_GetTypeFlag(baseType));
    CHECK_ERROR(errCode, "_DumpConst");

    VERIFY_OK(VIR_LOG(Dumper, ")"));
    return errCode;
}

static VSC_ErrCode
_DumpModifier(
    IN OUT VIR_Dumper *  Dumper,
    IN VIR_Instruction * Inst,
    IN  VIR_Operand   *  Operand
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    gcmASSERT(Operand != gcvNULL &&
        Dumper->baseDumper.pOffset != gcvNULL);

    /* dest */
    if(VIR_Operand_isLvalue(Operand))
    {
        VERIFY_OK(
            VIR_LOG(Dumper, "%s",
            VIR_DestModifier_GetName(VIR_Operand_GetModifier(Operand))));
    }
    else if (!VIR_Operand_isTexldParm(Operand))
    {
        if (VIR_OPCODE_isCmplx(VIR_Inst_GetOpcode(Inst)))
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "%s",
                VIR_ComplexSrcModifier_GetName(VIR_Operand_GetModifier(Operand))));
        }
        else
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "%s",
                VIR_SrcModifier_GetName(VIR_Operand_GetModifier(Operand))));
        }
    }
    return errCode;
}

static VSC_ErrCode
_DumpIndexed(
    IN OUT VIR_Dumper  *Dumper,
    IN VIR_Operand     *Operand,
    IN VIR_Function    *Func
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctBOOL     hasPre  = gcvFALSE;
    VIR_Indexed relAddrMode;
    static gctCONST_STRING indexed[] =
    {
        "",
        ".x",
        ".y",
        ".z",
        ".w",
        "al",
        "vertexId"
    };

    gcmASSERT(Operand != gcvNULL &&
        Dumper->baseDumper.pOffset != gcvNULL);

    relAddrMode = VIR_Operand_GetRelAddrMode(Operand);
    if(!(relAddrMode != VIR_INDEXED_NONE ||
        (VIR_Operand_GetIsConstIndexing(Operand) == 1 &&
        VIR_Operand_GetRelIndexing(Operand) != 0) ||
        VIR_Operand_GetMatrixConstIndex(Operand) != 0))
    {
        return errCode;
    }

    VERIFY_OK(
        VIR_LOG(Dumper, "["));

    if(relAddrMode  >= VIR_INDEXED_AL)
    {
        gcmASSERT(relAddrMode  < gcmCOUNTOF(indexed));
        VERIFY_OK(
            VIR_LOG(Dumper, "%s",
            indexed[relAddrMode]));

        hasPre = gcvTRUE;
    }

    if(hasPre &&
        VIR_Operand_GetMatrixConstIndex(Operand) > 0)
    {
        VERIFY_OK(
            VIR_LOG(Dumper, " + "));
    }

    if(VIR_Operand_GetMatrixConstIndex(Operand) > 0)
    {
        VERIFY_OK(
            VIR_LOG(Dumper, "%u",
            VIR_Operand_GetMatrixConstIndex(Operand)));
        hasPre = gcvTRUE;
    }

    if(hasPre &&
        ((VIR_Operand_GetIsConstIndexing(Operand) &&
        VIR_Operand_GetRelIndexing(Operand) != 0) ||
        (!(VIR_Operand_GetIsConstIndexing(Operand)) &&
        VIR_Operand_GetRelAddrMode(Operand) != VIR_INDEXED_NONE &&
        VIR_Operand_GetRelAddrMode(Operand) < VIR_INDEXED_AL)))
    {
        VERIFY_OK(
            VIR_LOG(Dumper, " + "));
    }

    if(VIR_Operand_GetIsConstIndexing(Operand) &&
        VIR_Operand_GetRelIndexing(Operand) != 0)
    {
        VERIFY_OK(
            VIR_LOG(Dumper, "%d",
            VIR_Operand_GetRelIndexing(Operand)));
    }
    else if(!(VIR_Operand_GetIsConstIndexing(Operand)) &&
        VIR_Operand_GetRelAddrMode(Operand) != VIR_INDEXED_NONE &&
        VIR_Operand_GetRelAddrMode(Operand) < VIR_INDEXED_AL)
    {
        VIR_Symbol *sym = VIR_Function_GetSymFromId(Func,
            VIR_Operand_GetRelIndexing(Operand));
        errCode = _DumpSymbol(Dumper, sym, gcvFALSE, gcvFALSE);
        CHECK_ERROR(errCode, "_DumpIndexed");

        gcmASSERT(VIR_Operand_GetRelAddrMode(Operand) < VIR_INDEXED_AL);
        VERIFY_OK(
            VIR_LOG(Dumper, "%s",
            indexed[VIR_Operand_GetRelAddrMode(Operand)]));
    }

    VERIFY_OK(
        VIR_LOG(Dumper, "]"));
    return errCode;
}

static VSC_ErrCode
_DumpHwRegInfo(
    IN OUT VIR_Dumper  *Dumper,
    IN VIR_Operand     *Operand)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    gctUINT hwRegId;

    if (VIR_Operand_isRegAllocated(Operand))
    {
        hwRegId = VIR_Operand_GetHwRegId(Operand);

        VERIFY_OK(
            VIR_LOG(Dumper, "{"));

        if (hwRegId < VIR_SR_Begin)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "r%d.<%d", hwRegId,
                VIR_Operand_GetHwShift(Operand)));
        }
        else if(hwRegId == VIR_SR_INSTATNCEID)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "insId.<%d", VIR_Operand_GetHwShift(Operand)));
        }
        else if(hwRegId == VIR_SR_VERTEXID)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "vxId.<%d", VIR_Operand_GetHwShift(Operand)));
        }
        else if(hwRegId == VIR_SR_FACE)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "face.<%d", VIR_Operand_GetHwShift(Operand)));
        }
        else if(hwRegId == VIR_SR_A0)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "a0.<%d", VIR_Operand_GetHwShift(Operand)));
        }
        else if(hwRegId == VIR_SR_B0)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "b0.<%d", VIR_Operand_GetHwShift(Operand)));
        }
        else if(hwRegId == VIR_SR_AL)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "al.<%d", VIR_Operand_GetHwShift(Operand)));
        }
        else if(hwRegId == VIR_SR_NEXTPC)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "nxpc.<%d", VIR_Operand_GetHwShift(Operand)));
        }
        else if(hwRegId == VIR_REG_MULTISAMPLEDEPTH)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "msdepth.<%d", VIR_Operand_GetHwShift(Operand)));
        }
        else if(hwRegId == VIR_REG_SAMPLE_POS)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "sPos.<%d", VIR_Operand_GetHwShift(Operand)));
        }
        else if(hwRegId == VIR_REG_SAMPLE_ID)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "sId.<%d", VIR_Operand_GetHwShift(Operand)));
        }
        else if(hwRegId == VIR_REG_SAMPLE_MASK_IN)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "smaskIn.<%d", VIR_Operand_GetHwShift(Operand)));
        }

        VERIFY_OK(
            VIR_LOG(Dumper, "} "));
    }

    return errCode;
}

static VSC_ErrCode
_DumpRound(
    IN OUT VIR_Dumper *  Dumper,
    IN  VIR_Operand   *  Operand
    )
{
    VSC_ErrCode errCode   = VSC_ERR_NONE;

    if (!VIR_Operand_isTexldParm(Operand))
    {
        VERIFY_OK(
            VIR_LOG(Dumper, "%s",
            VIR_RoundMode_GetName(VIR_Operand_GetRoundMode(Operand))));
    }
    return errCode;
}

VSC_ErrCode
VIR_Enable_Dump(
    IN OUT VIR_Dumper *  Dumper,
    IN  VIR_Enable       Enable
    )
{
    VSC_ErrCode errCode   = VSC_ERR_NONE;

    gcmASSERT(Dumper->baseDumper.pOffset != gcvNULL);

    if (Enable != VIR_ENABLE_NONE && Enable != VIR_ENABLE_XYZW)
    {
        /* Enable dot. */
        VERIFY_OK(
            VIR_LOG(Dumper, "."));

        if (Enable & VIR_ENABLE_X)
        {
            /* Enable x. */
            VERIFY_OK(
                VIR_LOG(Dumper, "x"));
        }

        if (Enable & VIR_ENABLE_Y)
        {
            /* Enable y. */
            VERIFY_OK(
                VIR_LOG(Dumper, "y"));
        }

        if (Enable & VIR_ENABLE_Z)
        {
            /* Enable z. */
            VERIFY_OK(
                VIR_LOG(Dumper, "z"));
        }

        if (Enable & VIR_ENABLE_W)
        {
            /* Enable w. */
            VERIFY_OK(
                VIR_LOG(Dumper, "w"));
        }
    }

    return errCode;
}

VSC_ErrCode
VIR_Swizzle_Dump(
    IN OUT VIR_Dumper *  Dumper,
    IN  VIR_Swizzle      Swizzle
    )
{

    VSC_ErrCode errCode   = VSC_ERR_NONE;
    VIR_Swizzle x, y, z, w;
    static const char swizzle[] =
    {
        'x', 'y', 'z', 'w',
    };

    gcmASSERT(Dumper->baseDumper.pOffset != gcvNULL);

    x = (Swizzle >> 0) & 0x3;
    y = (Swizzle >> 2) & 0x3;
    z = (Swizzle >> 4) & 0x3;
    w = (Swizzle >> 6) & 0x3;

    if(Swizzle != VIR_SWIZZLE_XYZW &&
        Swizzle != VIR_SWIZZLE_INVALID)
    {
        VERIFY_OK(
            VIR_LOG(Dumper, ".%c",
            swizzle[x]));

        if(y != x ||
           z != x ||
           w != x)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "%c",
                swizzle[y]));

            if(z != y ||
               w != y)
            {
                VERIFY_OK(
                    VIR_LOG(Dumper, "%c",
                    swizzle[z]));

                if(w != z)
                {
                    VERIFY_OK(
                        VIR_LOG(Dumper, "%c",
                        swizzle[w]));
                }
            }
        }
    }

    return errCode;
}

static VSC_ErrCode
_DumpEnableOrSwizzle(
    IN OUT VIR_Dumper *  Dumper,
    IN  VIR_Operand   *  Operand
    )
{
    VSC_ErrCode errCode   = VSC_ERR_NONE;

    gcmASSERT(Operand != gcvNULL && Dumper->baseDumper.pOffset != gcvNULL);

    if(VIR_Operand_isLvalue(Operand))
    {
        errCode = VIR_Enable_Dump(Dumper, VIR_Operand_GetEnable(Operand));
    }
    else
    {
        errCode = VIR_Swizzle_Dump(Dumper, VIR_Operand_GetSwizzle(Operand));
    }

    return errCode;
}

static VSC_ErrCode
_DumpLayout(
    IN OUT VIR_Dumper   *Dumper,
    IN  VIR_Layout      *Layout
    )
{
    VSC_ErrCode          errCode = VSC_ERR_NONE;
    gctBOOL needToDump = gcvFALSE;

    /* do not print layout() if it is empty */
    if (VIR_Layout_GetQualifiers(Layout) == VIR_LAYQUAL_NONE)
        return VSC_ERR_NONE;

    if(VIR_Layout_IsPacked(Layout))
    {
        needToDump = gcvTRUE;
    }
    else if(VIR_Layout_IsShared(Layout))
    {
        needToDump = gcvTRUE;
    }
    else if(VIR_Layout_IsStd140(Layout))
    {
        needToDump = gcvTRUE;
    }
    else if(VIR_Layout_IsStd430(Layout))
    {
        needToDump = gcvTRUE;
    }
    else if(VIR_Layout_IsRowMajor(Layout))
    {
        needToDump = gcvTRUE;
    }
    else if(VIR_Layout_IsColumnMajor(Layout))
    {
        needToDump = gcvTRUE;
    }
    else if(VIR_Layout_HasLocation(Layout) && VIR_Layout_GetLocation(Layout) != 0)
    {
        needToDump = gcvTRUE;
    }
    else if(VIR_Layout_HasBinding(Layout))
    {
        needToDump = gcvTRUE;
    }
    else if(VIR_Layout_HasOffset(Layout))
    {
       needToDump = gcvTRUE;
    }
    else if(VIR_Layout_HasBlend(Layout))
    {
        needToDump = gcvTRUE;
    }
    else if(VIR_Layout_HasImageFormat(Layout))
    {
        needToDump = gcvTRUE;
    }
    else if (VIR_Layout_GetDescriptorSet(Layout) != -1)
    {
       needToDump = gcvTRUE;
    }

    if(!needToDump)
       return errCode;

    VERIFY_OK(VIR_LOG(Dumper, "layout("));

    if(VIR_Layout_IsPacked(Layout))
    {
        VERIFY_OK(VIR_LOG(Dumper, "packed "));
    }
    if(VIR_Layout_IsShared(Layout))
    {
        VERIFY_OK(VIR_LOG(Dumper, "shared "));
    }
    if(VIR_Layout_IsStd140(Layout))
    {
        VERIFY_OK(VIR_LOG(Dumper, "std140 "));
    }
    if(VIR_Layout_IsStd430(Layout))
    {
        VERIFY_OK(VIR_LOG(Dumper, "std430 "));
    }
    if(VIR_Layout_IsRowMajor(Layout))
    {
        VERIFY_OK(VIR_LOG(Dumper, "row_major "));
    }
    if(VIR_Layout_IsColumnMajor(Layout))
    {
        VERIFY_OK(VIR_LOG(Dumper, "column_major "));
    }
    if(VIR_Layout_HasLocation(Layout))
    {
        VERIFY_OK(VIR_LOG(Dumper, "location=%d ", VIR_Layout_GetLocation(Layout)));
    }
    if(VIR_Layout_HasBinding(Layout))
    {
        VERIFY_OK(VIR_LOG(Dumper, "binding=%d ", VIR_Layout_GetBinding(Layout)));
    }
    if(VIR_Layout_HasOffset(Layout))
    {
        VERIFY_OK(VIR_LOG(Dumper, "offset=%d ", VIR_Layout_GetOffset(Layout)));
    }
    if(VIR_Layout_HasBlend(Layout))
    {
        VERIFY_OK(VIR_LOG(Dumper, "blend=%x ", VIR_Layout_GetBlend(Layout)));
    }
    if(VIR_Layout_HasImageFormat(Layout))
    {
        VERIFY_OK(VIR_LOG(Dumper, "image_format=%x ", VIR_Layout_GetImageFormat(Layout)));
    }
    if (VIR_Layout_GetDescriptorSet(Layout) != -1)
    {
        VERIFY_OK(VIR_LOG(Dumper, "set=%d ", VIR_Layout_GetDescriptorSet(Layout)));
    }

    VERIFY_OK(VIR_LOG(Dumper, ") "));

    return errCode;
}

static void
_DumpTyQualifier(
   IN OUT VIR_Dumper   *Dumper,
   IN VIR_TyQualifier qualifier
   )
{
    if (qualifier & VIR_TYQUAL_CONST)
    {
        VERIFY_OK(VIR_LOG(Dumper, "const "));
    }
    if (qualifier & VIR_TYQUAL_VOLATILE)
    {
        VERIFY_OK(VIR_LOG(Dumper, "volatile "));
    }
    if (qualifier & VIR_TYQUAL_RESTRICT)
    {
        VERIFY_OK(VIR_LOG(Dumper, "restrict "));
    }
    if (qualifier & VIR_TYQUAL_READ_ONLY)
    {
        VERIFY_OK(VIR_LOG(Dumper, "read_only "));
    }
    if (qualifier & VIR_TYQUAL_WRITE_ONLY)
    {
        VERIFY_OK(VIR_LOG(Dumper, "write_only "));
    }
    return;
}

static VSC_ErrCode
_DumpSymbol(
    IN OUT VIR_Dumper   *Dumper,
    IN  VIR_Symbol      *Sym,
    IN  gctBOOL          LVal,
    IN  gctBOOL          FullInfo
    )
{
    VSC_ErrCode          errCode = VSC_ERR_NONE;
    VIR_Const           *value;
    gctCONST_STRING      str = gcvNULL;

    gcmASSERT(Dumper->Shader != gcvNULL &&
        Sym != gcvNULL &&
        Dumper->baseDumper.pOffset != gcvNULL);

    if(LVal)
    {
        VERIFY_OK(
            VIR_LOG(Dumper, "%s%s",
            isSymArrayedPerVertex(Sym) ? "(ArrayedPerVertex) " : "",
            spaceaddr[VIR_Symbol_GetAddrSpace(Sym)]));

        _DumpTyQualifier(Dumper, VIR_Symbol_GetTyQualifier(Sym));

        VERIFY_OK(
            VIR_LOG(Dumper, "%s%s",
            VIR_Shader_IsCL(Dumper->Shader) ? "" : symbol_precision[VIR_Symbol_GetPrecision(Sym)],
            isSymPrecise(Sym) ? "precise " : ""));

        if(VIR_SYMFLAG_FLAT & VIR_Symbol_GetFlags(Sym))
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "flat "));
        }

        switch(VIR_Symbol_GetKind(Sym))
        {
        case VIR_SYM_UBO:
        case VIR_SYM_UNIFORM:
        case VIR_SYM_SAMPLER:
        case VIR_SYM_IMAGE:
        case VIR_SYM_IMAGE_T:
        case VIR_SYM_SAMPLER_T:
            str= _GetUniformKindString(VIR_Symbol_GetUniformKind(Sym));
            break;

        case VIR_SYM_SBO:
        case VIR_SYM_VARIABLE:
        case VIR_SYM_FUNCTION:
        case VIR_SYM_TEXTURE:
        case VIR_SYM_TYPE:
        case VIR_SYM_VIRREG:
        case VIR_SYM_CONST:
        case VIR_SYM_IOBLOCK:
            str = _GetStorageClassString(VIR_Symbol_GetStorageClass(Sym));
            break;
        case VIR_SYM_LABEL:
        case VIR_SYM_FIELD:
            break;
        case VIR_SYM_UNKNOWN:
        default:
            gcmASSERT(0);
            break;
        }
        if (str && str[0] != '\0')
        {
            VERIFY_OK(VIR_LOG(Dumper, "%s ", str));
        }
    }
    else
    {
        if (!VIR_Shader_IsCL(Dumper->Shader))
        {
            str = symbol_precision[VIR_Symbol_GetPrecision(Sym)];
            if (str && str[0] != '\0')
            {
                VERIFY_OK(VIR_LOG(Dumper, "%s ", str));
            }
        }
    }

    if (Dumper->baseDumper.verbose)
    {
        if (FullInfo)
        {
            _DumpLayout(Dumper, VIR_Symbol_GetLayout(Sym));
        }
    }
    switch(VIR_Symbol_GetKind(Sym))
    {
    case VIR_SYM_FUNCTION:       /* function */
        VERIFY_OK(
            VIR_LOG(Dumper, "@%s",
            VIR_Shader_GetSymNameString(Dumper->Shader, Sym)));
        break;
    case VIR_SYM_UBO:
    case VIR_SYM_SBO:
    case VIR_SYM_TEXTURE:
    case VIR_SYM_IMAGE:
    case VIR_SYM_IMAGE_T:
    case VIR_SYM_TYPE:           /* typedef */
    case VIR_SYM_LABEL:
    case VIR_SYM_SAMPLER:
    case VIR_SYM_SAMPLER_T:
    case VIR_SYM_UNIFORM:
    case VIR_SYM_VARIABLE:       /* global/local variables, input/output */
    case VIR_SYM_IOBLOCK:
        VERIFY_OK(
            VIR_LOG(Dumper, "%s",
            VIR_Shader_GetSymNameString(Dumper->Shader, Sym)));
        break;
    case VIR_SYM_FIELD:
        {
            VIR_Type *  structType = VIR_Shader_GetTypeFromId(Dumper->Shader,
                                         VIR_Symbol_GetStructTypeId(Sym));
            VIR_DumpTypeFormat typeFormat = { gcvFALSE, gcvFALSE };

            if (structType)
            {
                gcmASSERT(VIR_Type_GetKind(structType) == VIR_TY_STRUCT);
                errCode = _DumpType(Dumper, structType, gcvTRUE, typeFormat);
                CHECK_ERROR(errCode, "_DumpSymbol");
                VERIFY_OK(VIR_LOG(Dumper, "::%s",
                                VIR_Shader_GetSymNameString(Dumper->Shader, Sym)));
            }
            else
            {
                VERIFY_OK(VIR_LOG(Dumper, "??::%s",
                                VIR_Shader_GetSymNameString(Dumper->Shader, Sym)));
            }
            break;
        }
    case VIR_SYM_VIRREG:
        VERIFY_OK(
            VIR_LOG(Dumper, "temp(%u)",
            Sym->u1.vregIndex));
        break;
    case VIR_SYM_CONST:
        value = (VIR_Const *)VIR_GetSymFromId(&Dumper->Shader->constTable,
            Sym->u1.constId);
        errCode = _DumpConst(Dumper, value);
        CHECK_ERROR(errCode, "_DumpSymbol");
        break;
    case VIR_SYM_UNKNOWN:
    default:
        gcmASSERT(0);
        break;
    }

    return errCode;
}

VSC_ErrCode
VIR_Symbol_Dump(
    IN OUT VIR_Dumper   *Dumper,
    IN  VIR_Symbol      *Sym,
    IN  gctBOOL          FullType
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Type *          type;
    VIR_DumpTypeFormat  typeFormat = { FullType, gcvFALSE };
    VIR_SymbolKind      symKind = VIR_Symbol_GetKind(Sym);

    gcmASSERT(Dumper != gcvNULL &&
        Dumper->Shader != gcvNULL &&
        Dumper->baseDumper.pOffset != gcvNULL);

    type = VIR_Symbol_GetType(Sym);
    if(symKind == VIR_SYM_UNIFORM)
    {
        VIR_Uniform * uniform = VIR_Symbol_GetUniform(Sym);
        if (VIR_Uniform_isKernelArg(uniform) &&
            VIR_Uniform_GetDerivedType(uniform) != VIR_TYPE_UNKNOWN)
        {
            type = VIR_Shader_GetTypeFromId(Dumper->Shader, VIR_Uniform_GetDerivedType(uniform));
        }
    }
    if(type == gcvNULL)
    {
        return VSC_ERR_INVALID_ARGUMENT;
    }

    errCode = _DumpTypeWithSpace(Dumper, type, gcvTRUE, typeFormat);
    CHECK_ERROR(errCode, "VIR_Symbol_Dump");

    errCode = _DumpSymbol(Dumper, Sym, gcvTRUE, FullType);
    CHECK_ERROR(errCode, "VIR_Symbol_Dump");

    switch(symKind)
    {
    case VIR_SYM_UNIFORM:
    case VIR_SYM_IMAGE:
    case VIR_SYM_IMAGE_T:
    case VIR_SYM_SAMPLER:
    case VIR_SYM_SAMPLER_T:
        VERIFY_OK(
            VIR_LOG(Dumper, " ==> uniform("));
        errCode = _DumpMapedRegister(Dumper, type,
            Sym->u2.uniform->index);
        CHECK_ERROR(errCode, "VIR_Symbol_Dump");

        VERIFY_OK(
            VIR_LOG(Dumper, ")"));
        if (VIR_Uniform_GetPhysical(Sym->u2.uniform) != -1)
        {
            /* dump allocate register number */
            VERIFY_OK(
                VIR_LOG(Dumper, (symKind == VIR_SYM_SAMPLER && !isSymUniformTreatSamplerAsConst(Sym)) ? " :s(%d)" : " :c(%d)",
                        VIR_Uniform_GetPhysical(Sym->u2.uniform)));
                        VIR_Swizzle_Dump(Dumper, VIR_Uniform_GetSwizzle(Sym->u2.uniform));
        }
        if (isSymUniformCompiletimeInitialized(Sym))
        {
            VIR_Const *      pConstVal;

            if(VIR_Type_isArray(type)) {
                VIR_ConstId   *initializerPtr;
                gctUINT   arrayLength = VIR_Type_GetArrayLength(type);
                gctUINT   i;

                initializerPtr = VIR_Uniform_GetInitializerPtr(Sym->u2.uniform);
                VERIFY_OK(VIR_LOG(Dumper, " = {"));
                for(i = 0; i < arrayLength; i++) {
                    if(i != 0) {
                        VERIFY_OK(VIR_LOG(Dumper, ", "));
                    }
                    pConstVal = (VIR_Const *)VIR_GetSymFromId(&Dumper->Shader->constTable,
                                                              *initializerPtr++);
                    _DumpConst(Dumper, pConstVal);
                }
                VERIFY_OK(VIR_LOG(Dumper, "}"));
            }
            else {
                pConstVal = (VIR_Const *)VIR_GetSymFromId(&Dumper->Shader->constTable,
                                                          VIR_Uniform_GetInitializer(Sym->u2.uniform));
                /* dump initializer */
                VERIFY_OK(VIR_LOG(Dumper, " = "));
                _DumpConst(Dumper, pConstVal);
            }
        }
        break;
    case VIR_SYM_VARIABLE:
    case VIR_SYM_TEXTURE:
        {
            VERIFY_OK(
                VIR_LOG(Dumper, " ==> temp("));
            errCode = _DumpMapedRegisterWithRegCount(Dumper, type,
                VIR_Symbol_GetVariableVregIndex(Sym));
            CHECK_ERROR(errCode, "VIR_Symbol_Dump");

            VERIFY_OK(
                VIR_LOG(Dumper, ")"));
        }
        break;
    case VIR_SYM_FIELD:
        if (VIR_Symbol_GetFieldInfo(Sym) != gcvNULL)
        {
            VIR_FieldInfo * fi = VIR_Symbol_GetFieldInfo(Sym);
            VERIFY_OK(
                VIR_LOG(Dumper, " ==> fieldInfo(offset:%d, ", VIR_FieldInfo_GetOffset(fi)));
            if (VIR_FieldInfo_GetIsBitField(fi))
            {
                VERIFY_OK(
                    VIR_LOG(Dumper, "startBit:%d, bits:%d,",
                            VIR_FieldInfo_GetStartBit(fi), VIR_FieldInfo_GetBitSize(fi)));
            }
            VERIFY_OK(
                VIR_LOG(Dumper, " tempOffset:%d)",
                        VIR_FieldInfo_GetTempRegOrUniformOffset(fi)));
        }
        break;
    case VIR_SYM_FUNCTION:
    case VIR_SYM_UBO:
    case VIR_SYM_TYPE:
    case VIR_SYM_LABEL:
    case VIR_SYM_VIRREG:
    case VIR_SYM_CONST:
    case VIR_SYM_IOBLOCK:
    case VIR_SYM_UNKNOWN:
    default:
        break;
    }

    if (Dumper->baseDumper.verbose)
    {
        /* dump flags */
        VERIFY_OK(VIR_LOG(Dumper, " common_flags:<"));
        if (isSymEnabled(Sym))
        {
            VERIFY_OK(VIR_LOG(Dumper, " enabled"));
        }
        if (isSymInactive(Sym))
        {
            VERIFY_OK(VIR_LOG(Dumper, " inactive"));
        }
        if (isSymFlat(Sym))
        {
            VERIFY_OK(VIR_LOG(Dumper, " flat"));
        }
        if (isSymInvariant(Sym))
        {
            VERIFY_OK(VIR_LOG(Dumper, " invariant"));
        }
        if (isSymField(Sym))
        {
            VERIFY_OK(VIR_LOG(Dumper, " is_field"));
        }
        if (isSymCompilerGen(Sym))
        {
            VERIFY_OK(VIR_LOG(Dumper, " compiler_gen"));
        }
        if (isSymBuildin(Sym))
        {
            VERIFY_OK(VIR_LOG(Dumper, " builtin"));
        }
        if (isSymArrayedPerVertex(Sym))
        {
            VERIFY_OK(VIR_LOG(Dumper, " arrayed_per_vertex"));
        }
        if (isSymPrecise(Sym))
        {
            VERIFY_OK(VIR_LOG(Dumper, " precise"));
        }
        if (isSymLoadStoreAttr(Sym))
        {
             VERIFY_OK(VIR_LOG(Dumper, " ld_st_attr"));
        }
        if (isSymStaticallyUsed(Sym))
        {
            VERIFY_OK(VIR_LOG(Dumper, " statically_used"));
        }
        if (isSymVectorizedOut(Sym))
        {
            VERIFY_OK(VIR_LOG(Dumper, " vectorized_out"));
        }
        if (isSymIOBlockMember(Sym))
        {
            VERIFY_OK(VIR_LOG(Dumper, " is_ioblock_member"));
        }
        if (isSymInstanceMember(Sym))
        {
            VERIFY_OK(VIR_LOG(Dumper, " is_instance_member"));
        }
        if (isSymUnused(Sym))
        {
            VERIFY_OK(VIR_LOG(Dumper, " unused"));
        }
        VERIFY_OK(VIR_LOG(Dumper, " >"));
    }
    return errCode;
}

static VSC_ErrCode
_DumpTypeWithSpace(
    IN OUT VIR_Dumper     *Dumper,
    IN VIR_Type           *Type,
    IN gctBOOL             LVal,
    IN VIR_DumpTypeFormat  TypeFormat
    )
{
    VSC_ErrCode  errCode = VSC_ERR_NONE;

    if(!LVal)
    {
        return errCode;
    }

    errCode = _DumpType(Dumper, Type, LVal, TypeFormat);
    CHECK_ERROR(errCode, "_DumpTypeWithSpace");

    if(VIR_Shader_IsCL(Dumper->Shader) || (VIR_Type_GetBaseTypeId(Type) != VIR_TYPE_FLOAT32 || !TypeFormat.Ellipsis ))
    {
        VERIFY_OK(
            VIR_LOG(Dumper, " "));
    }

    return errCode;
}

static VSC_ErrCode
_DumpType(
    IN OUT VIR_Dumper     *Dumper,
    IN VIR_Type           *Type,
    IN gctBOOL             LVal,
    IN VIR_DumpTypeFormat  TypeFormat
    )
{
    VSC_ErrCode  errCode   = VSC_ERR_NONE;
    VIR_Type    *baseType  = gcvNULL;
    gctSIZE_T    i         = 0;

    gcmASSERT(Type != gcvNULL &&
        Dumper->Shader != gcvNULL &&
        Dumper->baseDumper.pOffset != gcvNULL);

    if(!LVal)
    {
        return errCode;
    }

    switch(VIR_Type_GetKind(Type))
    {
    case VIR_TY_INVALID:
    case VIR_TY_SCALAR:
    case VIR_TY_VECTOR:
    case VIR_TY_MATRIX:
    case VIR_TY_SAMPLER:
    case VIR_TY_IMAGE:
    case VIR_TY_IMAGE_T:
    case VIR_TY_VOID:
        gcmASSERT(VIR_Type_GetFlags(Type) & VIR_TYFLAG_BUILTIN);
        if (VIR_Shader_IsCL(Dumper->Shader))
        {
            VERIFY_OK(VIR_LOG(Dumper, "%s", VIR_GetOCLTypeName(VIR_Type_GetIndex(Type))));
        }
        else if(VIR_Type_GetBaseTypeId(Type) != VIR_TYPE_FLOAT32 || !TypeFormat.Ellipsis )
        {
            VERIFY_OK(VIR_LOG(Dumper, "%s", VIR_GetTypeName(VIR_Type_GetIndex(Type))));
        }
        break;
        /* derived types */
    case VIR_TY_FUNCTION:
        baseType = VIR_Shader_GetTypeFromId(Dumper->Shader,
            VIR_Type_GetBaseTypeId(Type));
        if(baseType == gcvNULL)
        {
            return VSC_ERR_INVALID_ARGUMENT;
        }

        errCode = _DumpTypeWithSpace(Dumper, baseType, LVal, TypeFormat);
        CHECK_ERROR(errCode, "_DumpType");

        for(i = 0; i < VIR_IdList_Count(VIR_Type_GetParameters(Type)); ++i)
        {
            VIR_Id    id        = VIR_IdList_GetId(VIR_Type_GetParameters(Type), i);
            VIR_Type *paramType = VIR_Shader_GetTypeFromId(Dumper->Shader, id);

            errCode = _DumpTypeWithSpace(Dumper, paramType, gcvFALSE, TypeFormat);
            CHECK_ERROR(errCode, "_DumpType");
        }
        break;
    case VIR_TY_POINTER:
        baseType = VIR_Shader_GetTypeFromId(Dumper->Shader,
            VIR_Type_GetBaseTypeId(Type));
        if(baseType == gcvNULL)
        {
            return VSC_ERR_INVALID_ARGUMENT;
        }
        if(!TypeFormat.NoAddrQual)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "%s",
                spaceaddr[VIR_Type_GetAddrSpace(Type)]));

            _DumpTyQualifier(Dumper, VIR_Type_GetQualifier(Type));
        }

        errCode = _DumpType(Dumper, baseType, LVal, TypeFormat);
        CHECK_ERROR(errCode, "_DumpType");

        if(TypeFormat.NoWSBeforeStar)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "*"));
        }
        else
        {
            VERIFY_OK(
                VIR_LOG(Dumper, " *"));
        }
        break;
    case VIR_TY_ARRAY:
        baseType = VIR_Shader_GetTypeFromId(Dumper->Shader, VIR_Type_GetBaseTypeId(Type));
        if(baseType == gcvNULL)
        {
            return VSC_ERR_INVALID_ARGUMENT;
        }

        TypeFormat.Ellipsis = gcvFALSE;

        errCode = _DumpType(Dumper, baseType, LVal, TypeFormat);
        CHECK_ERROR(errCode, "_DumpType");

        if(TypeFormat.NoWSBeforeStar)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "[%d]", VIR_Type_GetArrayLength(Type)));
        }
        else
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "[%d] ", VIR_Type_GetArrayLength(Type)));
        }
        break;
    case VIR_TY_STRUCT:

        if(VIR_Type_GetFlags(Type) & VIR_TYFLAG_ANONYMOUS ||
           VIR_Type_GetNameId(Type) == VIR_INVALID_ID)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "__anonymous "));
        }
        else
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "%s %s",
                VIR_Type_GetFlags(Type) & VIR_TYFLAG_ISUNION ? "union" : "struct",
                VIR_Shader_GetStringFromId(Dumper->Shader, VIR_Type_GetNameId(Type))));
        }

        if(TypeFormat.FullType == gcvTRUE && VIR_Type_GetFields(Type) != gcvNULL)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, " {"));
            VIR_LOG_FLUSH(Dumper);

            TypeFormat.Indent++;

            for(i = 0; i < VIR_IdList_Count(VIR_Type_GetFields(Type)); ++i)
            {
                VIR_Id      id       = VIR_IdList_GetId(VIR_Type_GetFields(Type), i);
                VIR_Symbol *fieldSym = VIR_Shader_GetSymFromId(Dumper->Shader, id);
                VIR_Type   *fieldTy  = VIR_Symbol_GetType(fieldSym);
                VIR_FieldInfo *fInfo = VIR_Symbol_GetFieldInfo(fieldSym);
                /* the field type should not be the same as struct type */
                gcmASSERT(fieldTy != Type);
                _DumpIndent(Dumper, TypeFormat);

                errCode = _DumpTypeWithSpace(Dumper, fieldTy, gcvTRUE, TypeFormat);
                CHECK_ERROR(errCode, "_DumpType");

                errCode = _DumpSymbol(Dumper, fieldSym, gcvFALSE, TypeFormat.FullType);
                CHECK_ERROR(errCode, "_DumpType");

                VERIFY_OK(
                    VIR_LOG(Dumper, ";"));
                if (Dumper->baseDumper.verbose)
                {
                    /* dump field info */
                    VERIFY_OK(
                        VIR_LOG(Dumper, "/* offset:%d, virRegOffset:%d */",
                                VIR_FieldInfo_GetOffset(fInfo), VIR_FieldInfo_GetTempRegOrUniformOffset(fInfo)));
                }
                VIR_LOG_FLUSH(Dumper);
            }

            TypeFormat.Indent--;
            _DumpIndent(Dumper, TypeFormat);
            VERIFY_OK(
                VIR_LOG(Dumper, "}"));
        }
        break;

    case VIR_TY_META:
        break;

    case VIR_TY_TYPEDEF:
        if(VIR_Type_GetNameId(Type) == VIR_INVALID_ID)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "__anonymous "));
        }
        else
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "%s",
                VIR_Shader_GetStringFromId(Dumper->Shader, VIR_Type_GetNameId(Type))));
        }
        break;

    case VIR_TY_ENUM:
        if(VIR_Type_GetNameId(Type) == VIR_INVALID_ID)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "__anonymous "));
        }
        else
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "enum %s",
                VIR_Shader_GetStringFromId(Dumper->Shader, VIR_Type_GetNameId(Type))));
        }
        break;

    default:
        gcmASSERT(0);
    }

    return errCode;
}

static VSC_ErrCode
_DumpOperandTexldModifier(
    IN OUT VIR_Dumper             *Dumper,
    IN VIR_Operand                *TexldOperand,
    IN VIR_Instruction            *Inst,
    IN gctBOOL                     DumpTYpe
    )
{
    VSC_ErrCode      errCode    = VSC_ERR_NONE;
    gctSIZE_T        i          = 0;

    for(i = 0; i < VIR_TEXLDMODIFIER_COUNT; ++i)
    {
        if(VIR_Operand_GetTexldModifier(TexldOperand, i) != gcvNULL)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "[%s, ",
                _GetTexldModifierString(TexldOperand, i)));

            errCode = _DumpOperand(Dumper, Inst,
                                   VIR_Operand_GetTexldModifier(TexldOperand, i),
                                   gcvFALSE);
            CHECK_ERROR(errCode, "_DumpOperandTexldModifier");

            VERIFY_OK(
                VIR_LOG(Dumper, "] "));
        }
    }

    return errCode;
}

static VSC_ErrCode
_DumpOperand(
    IN OUT VIR_Dumper *  Dumper,
    IN VIR_Instruction * Inst,
    IN VIR_Operand  *    Operand,
    IN gctBOOL           DumpType
    )
{
    VSC_ErrCode      errCode   = VSC_ERR_NONE;
    VIR_ParmPassing *parm      = gcvNULL;
    VIR_Label       *label     = gcvNULL;
    VIR_Type        *type      = gcvNULL;

    VIR_Symbol      *sym       = gcvNULL;
    VIR_Const       *vConst    = gcvNULL;
    VIR_OperandList *operandList = gcvNULL;
    VIR_Function    *func = VIR_Inst_GetFunction(Inst);
    VIR_TypeId       componentTyId;
    VIR_DumpTypeFormat typeFormat = { gcvFALSE, gcvTRUE };

    gcmASSERT(Operand != gcvNULL &&
        Dumper->baseDumper.pOffset != gcvNULL &&
        Dumper->Shader != gcvNULL &&
        func != gcvNULL);

    if (Dumper->dumpOperandId)
    {
        VERIFY_OK(
            VIR_LOG(Dumper, "[id:%d] ", VIR_Operand_GetIndex(Operand)));
    }

    switch(VIR_Operand_GetOpKind(Operand))
    {
    case VIR_OPND_NONE:
        VERIFY_OK(
            VIR_LOG(Dumper, "__none"));
        return errCode;
    case VIR_OPND_UNDEF:
        VERIFY_OK(
            VIR_LOG(Dumper, "__undef"));
        break;
    case VIR_OPND_UNUSED:
        VERIFY_OK(
            VIR_LOG(Dumper, "__unused"));
        return errCode;
    case VIR_OPND_IMMEDIATE:
        type = VIR_Shader_GetTypeFromId(Dumper->Shader,
            VIR_Operand_GetTypeId(Operand));
        if(type == gcvNULL)
        {
            return VSC_ERR_INVALID_ARGUMENT;
        }

        if (VIR_Operand_is5BitOffset(Operand))
        {
            gctINT val = VIR_Operand_GetImmediateInt(Operand);
            gctINT i, offset;
            VERIFY_OK(VIR_LOG(Dumper, "offset: <"));
            for (i = 0; i < 4; i++)
            {
                offset = val & 0x1f;
                if (i > 0)
                {
                    VERIFY_OK(VIR_LOG(Dumper, ", "));
                }
                if (offset & 0x10)
                {
                    offset = (gctINT)((gctUINT)offset | 0xFFFFFFF0);
                }
                VERIFY_OK(VIR_LOG(Dumper, "%d", offset));

                val = val >> 5;
                if (val == 0 && i > 0)
                    break;
            }
            VERIFY_OK(VIR_LOG(Dumper, ">"));
       }
        else
        {
            errCode = _DumpTypeWithSpace(Dumper, type,
                gcvTRUE,
                typeFormat);
            CHECK_ERROR(errCode, "DumpOperand");
            componentTyId = VIR_GetTypeComponentType(VIR_Operand_GetTypeId(Operand));
            gcmASSERT((componentTyId < gcmCOUNTOF(formats)) &&
                formats[componentTyId].Type == (gctUINT32)componentTyId);

            gcmASSERT(formats[componentTyId].Count == 1); /* make sure only one value is accessed */
            errCode = _DumpVecConst(Dumper,
                (gctUINT *)&VIR_Operand_GetImmediateUint(Operand),
                &formats[componentTyId],
                VIR_GetTypeFlag(componentTyId));
        }
        CHECK_ERROR(errCode, "DumpOperand");
        break;
    case VIR_OPND_EVIS_MODIFIER:
        VERIFY_OK(
            VIR_LOG(Dumper, "<EvisModifier>%#x", VIR_Operand_GetEvisModifier(Operand)));
        CHECK_ERROR(errCode, "DumpOperand");
        break;
    case VIR_OPND_CONST:
        vConst = VIR_Shader_GetConstFromId(Dumper->Shader,
                        VIR_Operand_GetConstId(Operand));
        if(vConst == gcvNULL)
        {
            return VSC_ERR_INVALID_ARGUMENT;
        }

        errCode = _DumpConst(Dumper, vConst);
        CHECK_ERROR(errCode, "DumpOperand");
        break;
    case VIR_OPND_PARAMETERS:
        {
            gctUINT i;
            parm = VIR_Operand_GetParameters(Operand);
            VERIFY_OK(
                VIR_LOG(Dumper, "["));
            for (i = 0; i < parm->argNum; i++)
            {
                if (i != 0)
                {
                    VERIFY_OK(VIR_LOG(Dumper, ", "));
                }
                errCode = _DumpOperand(Dumper, Inst, parm->args[i], DumpType);
                CHECK_ERROR(errCode, "DumpOperand");
            }
            VERIFY_OK(
                VIR_LOG(Dumper, "]"));
        }
        break;
    case VIR_OPND_LABEL:
        label = VIR_Operand_GetLabel(Operand);
        if(label == gcvNULL)
        {
            return VSC_ERR_INVALID_ARGUMENT;
        }

        sym = VIR_Function_GetSymFromId(func, label->sym);
        if(sym == gcvNULL)
        {
            return VSC_ERR_INVALID_ARGUMENT;
        }

        errCode = _DumpSymbol(Dumper, sym,
            VIR_Operand_isLvalue(Operand) || DumpType, gcvFALSE);
        CHECK_ERROR(errCode, "DumpOperand");

        break;
    case VIR_OPND_FUNCTION:
        {
            VIR_Function    *func = VIR_Operand_GetFunction(Operand);
            if(func == gcvNULL)
            {
                return VSC_ERR_INVALID_ARGUMENT;
            }

            sym = VIR_Shader_GetSymFromId(Dumper->Shader,
                func->funcSym);
            if(sym == gcvNULL)
            {
                return VSC_ERR_INVALID_ARGUMENT;
            }

            errCode = _DumpSymbol(Dumper, sym,
                VIR_Operand_isLvalue(Operand) || DumpType, gcvFALSE);
            CHECK_ERROR(errCode, "DumpOperand");
            break;
        }
    case VIR_OPND_SAMPLER_INDEXING:
    case VIR_OPND_SYMBOL:
    case VIR_OPND_VIRREG:
        type = VIR_Shader_GetTypeFromId(Dumper->Shader,
            VIR_Operand_GetTypeId(Operand));
        sym  = VIR_Operand_GetSymbol(Operand);

        if(type == gcvNULL || sym == gcvNULL)
        {
            return VSC_ERR_INVALID_ARGUMENT;
        }

        errCode = _DumpTypeWithSpace(Dumper, type, gcvTRUE, typeFormat);
        CHECK_ERROR(errCode, "DumpOperand");

        errCode = _DumpSymbol(Dumper, sym,
            VIR_Operand_isLvalue(Operand) || DumpType, gcvFALSE);
        CHECK_ERROR(errCode, "DumpOperand");

        errCode = _DumpIndexed(Dumper, Operand, func);
        CHECK_ERROR(errCode, "DumpOperand");
        break;
    case VIR_OPND_NAME:
        VERIFY_OK(
            VIR_LOG(Dumper, "%s",
            VIR_Shader_GetStringFromId(Dumper->Shader,VIR_Operand_GetNameId(Operand))));
        break;
    case VIR_OPND_INTRINSIC:
        VERIFY_OK(
            VIR_LOG(Dumper, "%s",
            VIR_Intrinsic_GetName(VIR_Operand_GetIntrinsicKind(Operand))));
        break;
    case VIR_OPND_FIELD:
        type = VIR_Shader_GetTypeFromId(Dumper->Shader,
            VIR_Operand_GetTypeId(Operand));
        if(type == gcvNULL)
        {
            return VSC_ERR_INVALID_ARGUMENT;
        }

        errCode = _DumpTypeWithSpace(Dumper, type,
            VIR_Operand_isLvalue(Operand) || DumpType,
            typeFormat);
        CHECK_ERROR(errCode, "DumpOperand");

        errCode = _DumpOperand(Dumper, Inst,
            VIR_Operand_GetFieldBase(Operand), gcvFALSE);
        CHECK_ERROR(errCode, "DumpOperand");

        VERIFY_OK(
            VIR_LOG(Dumper, "->"));

        sym = VIR_Function_GetSymFromId(func,
            VIR_Operand_GetFieldId(Operand));
        if(sym == gcvNULL)
        {
            return VSC_ERR_INVALID_ARGUMENT;
        }

        errCode = _DumpSymbol(Dumper, sym,
            VIR_Operand_isLvalue(Operand) || DumpType, gcvFALSE);
        CHECK_ERROR(errCode, "DumpOperand");
        break;
    case VIR_OPND_ARRAY:
        type = VIR_Shader_GetTypeFromId(Dumper->Shader,
            VIR_Operand_GetTypeId(Operand));
        if(type == gcvNULL)
        {
            return VSC_ERR_INVALID_ARGUMENT;
        }

        typeFormat.Ellipsis = gcvFALSE;
        errCode = _DumpTypeWithSpace(Dumper, type,
            VIR_Operand_isLvalue(Operand) || DumpType,
            typeFormat);
        CHECK_ERROR(errCode, "DumpOperand");

        errCode = _DumpOperand(Dumper, Inst,
            VIR_Operand_GetArrayBase(Operand), gcvFALSE);
        CHECK_ERROR(errCode, "DumpOperand");

        operandList = VIR_Operand_GetArrayIndex(Operand);
        while(operandList)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "["));

            errCode = _DumpOperand(Dumper, Inst,
                operandList->value, gcvFALSE);
            CHECK_ERROR(errCode, "DumpOperand");

            VERIFY_OK(
                VIR_LOG(Dumper, "]"));

            operandList = operandList->next;
        }
        break;
    case VIR_OPND_TEXLDPARM:
        _DumpOperandTexldModifier(Dumper,
            (VIR_Operand *)Operand, Inst, DumpType);
        break;

    case VIR_OPND_SIZEOF:
        VERIFY_OK(VIR_LOG(Dumper, "SizeOf["));
        type = VIR_Shader_GetTypeFromId(Dumper->Shader, VIR_Operand_GetTypeId(Operand));
        sym  = VIR_Operand_GetSymbol(Operand);

        if(type == gcvNULL || sym == gcvNULL)
        {
            return VSC_ERR_INVALID_ARGUMENT;
        }

        errCode = _DumpTypeWithSpace(Dumper, type, gcvTRUE, typeFormat);
        CHECK_ERROR(errCode, "DumpOperand");

        errCode = _DumpSymbol(Dumper, sym,
            VIR_Operand_isLvalue(Operand) || DumpType, gcvFALSE);
        CHECK_ERROR(errCode, "DumpOperand");

        VERIFY_OK(VIR_LOG(Dumper, "]"));
        break;

    case VIR_OPND_OFFSETOF:
        VERIFY_OK(VIR_LOG(Dumper, "OffsetOf["));
        type = VIR_Shader_GetTypeFromId(Dumper->Shader,
            VIR_Operand_GetTypeId(Operand));
        sym  = VIR_Operand_GetSymbol(Operand);

        if(type == gcvNULL || sym == gcvNULL)
        {
            return VSC_ERR_INVALID_ARGUMENT;
        }

        errCode = _DumpTypeWithSpace(Dumper, type, gcvTRUE, typeFormat);
        CHECK_ERROR(errCode, "DumpOperand");

        errCode = _DumpSymbol(Dumper, sym,
            VIR_Operand_isLvalue(Operand) || DumpType, gcvFALSE);
        CHECK_ERROR(errCode, "DumpOperand");

        VERIFY_OK(VIR_LOG(Dumper, "]"));
        break;

    default:
        gcmASSERT(0);
        break;
    }

    errCode = _DumpModifier(Dumper, Inst, Operand);
    CHECK_ERROR(errCode, "DumpOperand");

    errCode = _DumpRound(Dumper, Operand);
    CHECK_ERROR(errCode, "DumpOperand");

    if(VIR_Operand_GetOpKind(Operand)  != VIR_OPND_IMMEDIATE &&
        VIR_Operand_GetOpKind(Operand) != VIR_OPND_FUNCTION &&
        VIR_Operand_GetOpKind(Operand) != VIR_OPND_LABEL &&
        VIR_Operand_GetOpKind(Operand) != VIR_OPND_TEXLDPARM &&
        VIR_Operand_GetOpKind(Operand) != VIR_OPND_SAMPLER_INDEXING &&
        VIR_Operand_GetOpKind(Operand) != VIR_OPND_NAME
        )
    {
        /* dump precision info, mediump is default */
        if (!VIR_Shader_IsCL(Dumper->Shader) &&
            !VIR_Operand_isIntrinsic(Operand) &&
            !VIR_Operand_isParameters(Operand))
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "%s", operand_precision[VIR_Operand_GetPrecision(Operand)]));
        }

        if (!VIR_Operand_isIntrinsic(Operand) &&
            !VIR_Operand_isParameters(Operand))
        {
            errCode = _DumpEnableOrSwizzle(Dumper, Operand);
            CHECK_ERROR(errCode, "DumpOperand");
        }
    }

    if(VIR_Operand_GetLShift(Operand))
    {
        VERIFY_OK(VIR_LOG(Dumper, ".ls%d", VIR_Operand_GetLShift(Operand)));
    }

    if (Dumper->baseDumper.verbose)
    {
        errCode = _DumpHwRegInfo(Dumper, Operand);
        if((VIR_Operand_GetFlags(Operand) & ~VIR_OPNDFLAG_REGALLOCATED) != 0)
        {
            VERIFY_OK(VIR_LOG(Dumper, "< "));

            if(VIR_Operand_isTemp256High(Operand))
            {
                VERIFY_OK(VIR_LOG(Dumper, "T256Hi "));
            }
            if(VIR_Operand_isTemp256Low(Operand))
            {
                VERIFY_OK(VIR_LOG(Dumper, "T256Lo "));
            }
            if(VIR_Operand_is5BitOffset(Operand))
            {
                VERIFY_OK(VIR_LOG(Dumper, "5Bit_Offset "));
            }
            if(VIR_Operand_isUniformIndex(Operand))
            {
                VERIFY_OK(VIR_LOG(Dumper, "Uniform_Index "));
            }
            VERIFY_OK(VIR_LOG(Dumper, ">"));
        }
    }
    CHECK_ERROR(errCode, "DumpOperand");

    return errCode;
}

static VSC_ErrCode
_DumpOpcode(
    IN OUT VIR_Dumper *  Dumper,
    IN  VIR_Instruction *Inst
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    gcmASSERT(Inst != gcvNULL &&
        Dumper->baseDumper.pOffset != gcvNULL);

    /*********************************************** Opcode *************/
    VERIFY_OK(
        VIR_LOG(Dumper, "%s",
        VIR_OPCODE_GetName(VIR_Inst_GetOpcode(Inst))));

    /*********************************************** Thread mode?? ******/
    if (VIR_Shader_isDual16Mode(Dumper->Shader))
    {
        static gctCONST_STRING threadMode[] =
        {
            "",
            ".t0t1",
            ".t0",
            ".t1",
            ".highpvec2"
        };

        VERIFY_OK(
            VIR_LOG(Dumper, "%s",
            threadMode[VIR_Inst_GetThreadMode(Inst)]));
    }

    /*********************************************** Condition *************/
    {
        VERIFY_OK(
            VIR_LOG(Dumper, "%s",
            VIR_CondOp_GetName(VIR_Inst_GetConditionOp(Inst))));
    }

    if (VIR_Inst_GetFlags(Inst) & VIR_INSTFLAG_PACKEDMODE)
    {
        VERIFY_OK(VIR_LOG(Dumper, ".pack"));
    }

    _DumpAlign(Dumper, OPCODE_ALIGN);

    return errCode;
}

static VSC_ErrCode
_DumpGeneralInst(
    IN OUT VIR_Dumper   *Dumper,
    IN  VIR_Instruction *Inst
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctSIZE_T   i       = 0;

    gcmASSERT(Inst != gcvNULL &&
        Dumper->baseDumper.pOffset != gcvNULL);

    errCode = _DumpOpcode(Dumper, Inst);
    CHECK_ERROR(errCode, "GeneralInst");

    if(VIR_Inst_GetDest(Inst) != gcvNULL)
    {
        gcmASSERT(VIR_Operand_isLvalue(VIR_Inst_GetDest(Inst)));
        errCode = _DumpOperand(Dumper, Inst, VIR_Inst_GetDest(Inst), gcvFALSE);
        CHECK_ERROR(errCode, "GeneralInst");
    }

    if(VIR_Inst_GetSrcNum(Inst) != 0)
    {
        VERIFY_OK(
            VIR_LOG(Dumper, ", "));
        _DumpAlign(Dumper, OPERAND_PLUS_ALIGN + OPCODE_ALIGN);
    }

    for(i = 0; i < VIR_Inst_GetSrcNum(Inst); ++i)
    {
        gcmASSERT(VIR_Inst_GetSource(Inst, i) != gcvNULL);
        gcmASSERT(!VIR_Operand_isLvalue(VIR_Inst_GetSource(Inst, i)));

        errCode = _DumpOperand(Dumper, Inst, VIR_Inst_GetSource(Inst, i), gcvFALSE);
        CHECK_ERROR(errCode, "GeneralInst");

        if(i != (gctSIZE_T)(VIR_Inst_GetSrcNum(Inst) - 1))
        {
            VERIFY_OK(
                VIR_LOG(Dumper, ", "));
            _DumpAlign(Dumper, OPERAND_PLUS_ALIGN * (i + 2) + OPCODE_ALIGN);
        }
    }

    if (gcmOPT_EnableDebug())
    {
        VIR_LOG(Dumper, "\t\t #Loc(%d,%d,%d)", Inst->sourceLoc.fileId, Inst->sourceLoc.lineNo, Inst->sourceLoc.colNo);
    }

    return errCode;
}

static VSC_ErrCode
_DumpPhiInst(
    IN OUT VIR_Dumper   *Dumper,
    IN VIR_Instruction  *Inst
    )
{
    VSC_ErrCode       errCode = VSC_ERR_NONE;
    VIR_PhiOperandArray *phiOperands;
    VIR_Symbol       *sym;
    VIR_Function     *func;
    gctUINT32         i = 0;

    gcmASSERT(Inst != gcvNULL &&
        Dumper->baseDumper.pOffset != gcvNULL &&
        Dumper->Shader != gcvNULL &&
        VIR_Inst_GetFunction(Inst) != gcvNULL &&
        VIR_Inst_GetSource(Inst, 0) != gcvNULL);

    func   = VIR_Inst_GetFunction(Inst);

    errCode = _DumpOpcode(Dumper, Inst);
    CHECK_ERROR(errCode, "PhiInst");

    errCode = _DumpOperand(Dumper, Inst, VIR_Inst_GetDest(Inst), gcvFALSE);
    CHECK_ERROR(errCode, "PhiInst");
    VERIFY_OK(
        VIR_LOG(Dumper, ", "));
    _DumpAlign(Dumper, OPERAND_PLUS_ALIGN + OPCODE_ALIGN);

    gcmASSERT(VIR_Operand_isPhi(VIR_Inst_GetSource(Inst, 0)));
    phiOperands = VIR_Operand_GetPhiOperands(VIR_Inst_GetSource(Inst, 0));
    VERIFY_OK(
        VIR_LOG(Dumper, "{"));

    for(i = 0; i < VIR_PhiOperandArray_GetCount(phiOperands); i++)
    {
        VIR_PhiOperand* phiOperand = VIR_PhiOperandArray_GetNthOperand(phiOperands, i);
        VERIFY_OK(
            VIR_LOG(Dumper, "["));

        errCode = _DumpOperand(Dumper, Inst, VIR_PhiOperand_GetValue(phiOperand), gcvFALSE);
        CHECK_ERROR(errCode, "PhiInst");

        sym = VIR_Function_GetSymFromId(func, VIR_PhiOperand_GetLabel(phiOperand)->sym);
        if(sym == gcvNULL)
        {
            return VSC_ERR_INVALID_ARGUMENT;
        }

        VERIFY_OK(
            VIR_LOG(Dumper, ", %s]",
            VIR_Shader_GetSymNameString(Dumper->Shader, sym)));

        if(i + 1 < VIR_PhiOperandArray_GetCount(phiOperands))
        {
            VERIFY_OK(
                VIR_LOG(Dumper, ", "));
        }

        /* Dump two operands of Phi instruction per line */
        if ((i == 0) || (i % 2 == 0))
        {
            VIR_LOG(Dumper, "\t");
            VIR_LOG_FLUSH(Dumper);
        }
        _DumpAlign(Dumper, OPERAND_PLUS_ALIGN);
    }
    VERIFY_OK(
        VIR_LOG(Dumper, "}"));

    if (gcmOPT_EnableDebug())
    {
        VIR_LOG(Dumper, "\t\t #Loc(%d,%d,%d)", Inst->sourceLoc.fileId, Inst->sourceLoc.lineNo, Inst->sourceLoc.colNo);
    }

    return errCode;
}

static VSC_ErrCode
_DumpImgQueryInst(
    IN OUT VIR_Dumper   *Dumper,
    IN VIR_Instruction  *Inst
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctSIZE_T   i       = 0;
    gctBOOL     lastOperand = gcvFALSE;

    gcmASSERT(Inst != gcvNULL &&
        Dumper->baseDumper.pOffset != gcvNULL);

    errCode = _DumpOpcode(Dumper, Inst);
    CHECK_ERROR(errCode, "GeneralInst");

    if (VIR_Inst_GetDest(Inst) != gcvNULL)
    {
        gcmASSERT(VIR_Operand_isLvalue(VIR_Inst_GetDest(Inst)));
        errCode = _DumpOperand(Dumper, Inst, VIR_Inst_GetDest(Inst), gcvFALSE);
        CHECK_ERROR(errCode, "GeneralInst");
    }

    if (VIR_Inst_GetSrcNum(Inst) != 0)
    {
        VERIFY_OK(VIR_LOG(Dumper, ", "));
        _DumpAlign(Dumper, OPERAND_PLUS_ALIGN + OPCODE_ALIGN);
    }

    for (i = 0; i < VIR_Inst_GetSrcNum(Inst); ++i)
    {
        gcmASSERT(VIR_Inst_GetSource(Inst, i) != gcvNULL);
        gcmASSERT(!VIR_Operand_isLvalue(VIR_Inst_GetSource(Inst, i)));

        /* Dump Image query kind. */
        if (i == 1)
        {
            VIR_IMAGE_QUERY_KIND imageQueryKind =
                (VIR_IMAGE_QUERY_KIND)VIR_Operand_GetImmediateUint(VIR_Inst_GetSource(Inst, i));

            switch (imageQueryKind)
            {
            case VIR_IMAGE_QUERY_KIND_FORMAT:
                VERIFY_OK(VIR_LOG(Dumper, "FORMAT"));
                lastOperand = gcvTRUE;
                break;

            case VIR_IMAGE_QUERY_KIND_ORDER:
                VERIFY_OK(VIR_LOG(Dumper, "ORDER"));
                lastOperand = gcvTRUE;
                break;

            case VIR_IMAGE_QUERY_KIND_SIZE_LOD:
                VERIFY_OK(VIR_LOG(Dumper, "SIZE_LOD"));
                break;

            case VIR_IMAGE_QUERY_KIND_SIZE:
                VERIFY_OK(VIR_LOG(Dumper, "SIZE"));
                lastOperand = gcvTRUE;
                break;

            case VIR_IMAGE_QUERY_KIND_LOD:
                VERIFY_OK(VIR_LOG(Dumper, "LOD"));
                break;

            case VIR_IMAGE_QUERY_KIND_LEVELS:
                VERIFY_OK(VIR_LOG(Dumper, "LEVELS"));
                lastOperand = gcvTRUE;
                break;

            case VIR_IMAGE_QUERY_KIND_SAMPLES:
                VERIFY_OK(VIR_LOG(Dumper, "SAMPLES"));
                lastOperand = gcvTRUE;
                break;

            default:
                VERIFY_OK(VIR_LOG(Dumper, "UNKNOWN"));
                gcmASSERT(gcvFALSE);
                break;
            }
        }
        else
        {
            errCode = _DumpOperand(Dumper, Inst, VIR_Inst_GetSource(Inst, i), gcvFALSE);
            CHECK_ERROR(errCode, "GeneralInst");
        }

        if (lastOperand)
        {
            break;
        }

        if (i != (gctSIZE_T)(VIR_Inst_GetSrcNum(Inst) - 1))
        {
            VERIFY_OK(
                VIR_LOG(Dumper, ", "));
            _DumpAlign(Dumper, OPERAND_PLUS_ALIGN * (i + 2) + OPCODE_ALIGN);
        }
    }

    if (gcmOPT_EnableDebug())
    {
        VIR_LOG(Dumper, "\t\t #Loc(%d,%d,%d)", Inst->sourceLoc.fileId, Inst->sourceLoc.lineNo, Inst->sourceLoc.colNo);
    }


    return errCode;
}

VSC_ErrCode
VIR_Inst_Dump(
    IN OUT VIR_Dumper   *Dumper,
    IN VIR_Instruction  *Inst
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_OpCode opcode;
    gcmASSERT(Inst != gcvNULL &&
        Dumper->baseDumper.pOffset != gcvNULL);

    if(Inst <= VIR_ANY_DEF_INST && Inst >= VIR_OUTPUT_USAGE_INST)
    {
        if (Dumper->baseDumper.verbose)
        {
            if(Inst == VIR_ANY_DEF_INST)
            {
                VIR_LOG(Dumper, "ANY_DEF_INST\n");
            }
            if(Inst == VIR_UNDEF_INST)
            {
                VIR_LOG(Dumper, "UNDEF_INST\n");
            }
            if(Inst == VIR_HW_SPECIAL_DEF_INST)
            {
                VIR_LOG(Dumper, "HW_SPECIAL_DEF_INST\n");
            }
            if(Inst == VIR_INPUT_DEF_INST)
            {
                VIR_LOG(Dumper, "INPUT_DEF_INST\n");
            }
            if(Inst == VIR_OUTPUT_USAGE_INST)
            {
                VIR_LOG(Dumper, "OUTPUT_USAGE_INST\n");
            }
            if (gcmOPT_EnableDebug())
            {
                if (Inst->sourceLoc.fileId != 0 ||  Inst->sourceLoc.lineNo != 0 ||Inst->sourceLoc.colNo != 0)
                {
                    VIR_LOG(Dumper, "\t\t #Loc(%d,%d,%d)", Inst->sourceLoc.fileId, Inst->sourceLoc.lineNo, Inst->sourceLoc.colNo);
                }
            }
            VIR_LOG_FLUSH(Dumper);
        }
        return errCode;
    }

    VERIFY_OK(
        VIR_LOG(Dumper, "%03u: ", VIR_Inst_GetId(Inst)));

    opcode = VIR_Inst_GetOpcode(Inst);
    switch(opcode)
    {
    case VIR_OP_IMG_QUERY:
        errCode = _DumpImgQueryInst(Dumper, Inst);
        break;
    case VIR_OP_PHI:
    case VIR_OP_SPV_PHI:
        errCode = _DumpPhiInst(Dumper, Inst);
        break;
    case VIR_OP_LABEL:
        errCode = _DumpGeneralInst(Dumper, Inst);
        VERIFY_OK(
            VIR_LOG(Dumper, ":"));
        break;
    default:
        errCode = _DumpGeneralInst(Dumper, Inst);
        break;
    }
    VIR_LOG_FLUSH(Dumper);
    CHECK_ERROR(errCode, "DumpIR");

    return errCode;
}

static VSC_ErrCode
_DumpMapedRegister(
    IN OUT VIR_Dumper *Dumper,
    IN VIR_Type       *Type,
    IN gctUINT         RegStart
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;

    gcmASSERT(Dumper != gcvNULL &&
        Type != gcvNULL);

    VERIFY_OK(
        VIR_LOG(Dumper, "%u", RegStart));

    return errCode;
}

static VSC_ErrCode
_DumpMapedRegisterWithRegCount(
    IN OUT VIR_Dumper *Dumper,
    IN VIR_Type       *Type,
    IN gctUINT         RegStart
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;
    gctUINT     regCount = 0;

    gcmASSERT(Dumper != gcvNULL &&
        Type != gcvNULL);

    regCount = VIR_Type_GetVirRegCount(Dumper->Shader, Type, -1);

    if(regCount > 1)
    {
        VERIFY_OK(
            VIR_LOG(Dumper, "%u - %u",
            RegStart,
            regCount + RegStart - 1));
    }
    else
    {
        VERIFY_OK(
            VIR_LOG(Dumper, "%u", RegStart));
    }

    return errCode;
}

static VSC_ErrCode
_DumpVariable(
    IN OUT VIR_Dumper   *Dumper,
    IN     VIR_SymTable *SymTable,
    IN     VIR_Id        Id
    )
{
    VSC_ErrCode   errCode = VSC_ERR_NONE;
    VIR_Symbol   *sym = gcvNULL;

    gcmASSERT(Dumper != gcvNULL &&
        Dumper->Shader != gcvNULL &&
        Dumper->baseDumper.pOffset != gcvNULL &&
        SymTable != gcvNULL);

    sym  = VIR_GetSymFromId(SymTable, Id);
    if(sym == gcvNULL)
    {
        return VSC_ERR_INVALID_ARGUMENT;
    }

    errCode = VIR_Symbol_Dump(Dumper, sym, gcvTRUE);
    CHECK_ERROR(errCode, "_DumpVariable");

    return errCode;
}

static VSC_ErrCode
_DumpVariableList(
    IN OUT VIR_Dumper         *Dumper,
    IN     VIR_SymTable       *SymTable,
    IN     VIR_VariableIdList *List,
    IN     gctCONST_STRING     SplitString,
    IN     gctBOOL             PrintLast,
    IN     gctCONST_STRING     Comment
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctSIZE_T      i = 0;

    gcmASSERT(Dumper != gcvNULL &&
        Dumper->Shader != gcvNULL &&
        Dumper->baseDumper.pOffset != gcvNULL &&
        SymTable != gcvNULL &&
        List     != gcvNULL);

    if (Comment != gcvNULL && VIR_IdList_Count(List) > 0)
    {
        VERIFY_OK(
            VIR_LOG(Dumper, "%s\n", Comment));
        VIR_LOG_FLUSH(Dumper);

    }
    for(i = 0; i < VIR_IdList_Count(List); ++i)
    {
        VIR_Id id = VIR_IdList_GetId(List, i);

        errCode = _DumpVariable(Dumper, SymTable, id);
        CHECK_ERROR(errCode, "_DumpVariableList");

        if(i < (VIR_IdList_Count(List) - 1) || PrintLast)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, SplitString));
        }

        VIR_LOG_FLUSH(Dumper);
    }

    return errCode;
}

VSC_ErrCode
VIR_Function_Dump(
    IN OUT VIR_Dumper   *Dumper,
    IN VIR_Function     *Func
    )
{
    VSC_ErrCode  errCode = VSC_ERR_NONE;
    VIR_Symbol  *funcSym;
    VIR_DumpTypeFormat typeFormat = { gcvFALSE, gcvFALSE };

    gcmASSERT(Dumper != gcvNULL &&
        Dumper->Shader != gcvNULL &&
        Dumper->baseDumper.pOffset != gcvNULL&&
        Func != gcvNULL);

    funcSym = VIR_Shader_GetSymFromId(Dumper->Shader, Func->funcSym);
    if(funcSym == gcvNULL)
    {
        return VSC_ERR_INVALID_ARGUMENT;
    }

    if (Dumper->baseDumper.verbose)
    {
        /*********************************************** Function attributes ****/
        /* Function is openCL/OpenGL builtin function */
        if(Func->flags & VIR_FUNCFLAG_INTRINSICS)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "intrinsics "));
        }

        if(Func->flags & VIR_FUNCFLAG_STATIC)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "static "));
        }

        if(Func->flags & VIR_FUNCFLAG_EXTERN)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "extern "));
        }

        /* Always inline */
        if(Func->flags & VIR_FUNCFLAG_ALWAYSINLINE)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "inline "));
        }

        /* Neve inline */
        if(Func->flags & VIR_FUNCFLAG_NOINLINE)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "noinline "));
        }

        /* Inline is desirable */
        if(Func->flags & VIR_FUNCFLAG_INLINEHINT)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "inlinehint "));
        }

        /* Function does not access memory */
        if(Func->flags & VIR_FUNCFLAG_READNONE)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "readnone "));
        }

        /* Function only reads from memory */
        if(Func->flags & VIR_FUNCFLAG_READONLY)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "readonly "));
        }

        /* Hidden pointer to structure to return */
        if(Func->flags & VIR_FUNCFLAG_STRUCTRET)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "structret "));
        }

        /* Function is not returning */
        if(Func->flags & VIR_FUNCFLAG_NORETURN)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "noreturn "));
        }

        /* Force argument to be passed in register */
        if(Func->flags & VIR_FUNCFLAG_INREG)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "inreg "));
        }

        /* Pass structure by value */
        if(Func->flags & VIR_FUNCFLAG_BYVAL)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "byval "));
        }

        /* OpenCL Kernel function */
        if(Func->flags & VIR_FUNCFLAG_KERNEL)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "kernel "));
        }

        /* is recursive function */
        if(Func->flags & VIR_FUNCFLAG_RECURSIVE)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "recursive "));
        }
        VERIFY_OK(
            VIR_LOG(Dumper, "/* function instruction count [%d] */\n\n",
                VIR_Function_GetInstCount(Func)));
    }
    VERIFY_OK(
        VIR_LOG(Dumper, "function "));

    /**************************************************** Function name ****/
    {
        errCode = _DumpSymbol(Dumper, funcSym, gcvTRUE, gcvTRUE);
        CHECK_ERROR(errCode, "DumpFunction");
    }

    /**************************************************** Return type ******/
    VERIFY_OK(
        VIR_LOG(Dumper, "("));
    if(!(Func->flags & VIR_FUNCFLAG_NORETURN))
    {
        VIR_Type *type = VIR_Symbol_GetType(funcSym);
        if(type == gcvNULL)
        {
            return VSC_ERR_INVALID_ARGUMENT;
        }

        errCode = _DumpType(Dumper, type, gcvTRUE, typeFormat);
        CHECK_ERROR(errCode, "DumpFunction");
    }
    VERIFY_OK(
        VIR_LOG(Dumper, ")"));

    /**************************************************** Arguments ********/
    VERIFY_OK(
        VIR_LOG(Dumper, "("));
    if(VIR_IdList_Count(&Func->paramters) > 0)
    {
        errCode = _DumpVariableList(Dumper, &Func->symTable,
            &Func->paramters, ", ", gcvFALSE, "");
        CHECK_ERROR(errCode, "DumpFunction");
    }
    VERIFY_OK(
        VIR_LOG(Dumper, ") {"));
    VIR_LOG_FLUSH(Dumper);

    /**************************************************** Local variables **/
    if(Func->localVariables.count > 0)
    {
        errCode = _DumpVariableList(Dumper, &Func->symTable,
            &Func->localVariables, ";", gcvTRUE, "/* Local variables */");
        CHECK_ERROR(errCode, "DumpFunction");
    }

    /******************************************************* Instructions **/
    {
        VIR_InstIterator           instIter;
        VIR_Instruction           *inst   = gcvNULL;
        VIR_BASIC_BLOCK           *preBB  = gcvNULL;
        VIR_BASIC_BLOCK           *curBB  = gcvNULL;

        VIR_InstIterator_Init(&instIter, &Func->instList);
        inst = (VIR_Instruction *)VIR_InstIterator_First(&instIter);

        for (; inst != gcvNULL; inst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter))
        {
            if (!Dumper->invalidCFG)
            {
                curBB = VIR_Inst_GetBasicBlock(inst);
                if(preBB != curBB)
                {
                    gcmASSERT(curBB != gcvNULL);
                    if (Dumper->baseDumper.verbose)
                    {
                        errCode = _DumpBasicBlockInOutLength(Dumper, curBB);
                    }
                    preBB = curBB;
                    CHECK_ERROR(errCode, "DumpFunction");
                }
            }

            errCode = VIR_Inst_Dump(Dumper, inst);
            CHECK_ERROR(errCode, "DumpFunction");

            VIR_LOG_FLUSH(Dumper);
        }
    }

    VERIFY_OK(
        VIR_LOG(Dumper, "}"));

    VIR_LOG_FLUSH(Dumper);
    return errCode;
}

static VSC_ErrCode
_DumpVersion(
    IN OUT VIR_Dumper  *Dumper
    )
{
    VSC_ErrCode  errCode = VSC_ERR_NONE;

    gcmASSERT(Dumper != gcvNULL &&
        Dumper->Shader != gcvNULL &&
        Dumper->baseDumper.pOffset != gcvNULL);

    {
        static gctCONST_STRING clientAPIVersion[] =
        {
            "UNKNOWN",
            "D3D",
            "ES11",
            "ES20",
            "ES30",
            "ES31",
            "ES32",
            "GL",
            "VG",
            "CL",
            "VK",
        };

        static gctCONST_STRING shaderKind[] =
        {
            "UNKNOWN",
            "vertex",
            "fragment",
            "precompiled",
            "compute",
            "TC",
            "TE",
            "geometry",
            "library",
            "*U#()@#",
        };

        VERIFY_OK(
            VIR_LOG(Dumper, "Dump %s:%s IR. (id:%u)\n",
            clientAPIVersion[VIR_Shader_GetClientApiVersion(Dumper->Shader)],
            shaderKind[Dumper->Shader->shaderKind],
            VIR_Shader_GetId(Dumper->Shader)));
    }

    VIR_LOG_FLUSH(Dumper);
    return errCode;
}

VSC_ErrCode
VIR_Uniform_Dump(
    IN OUT VIR_Dumper  *Dumper,
    IN     VIR_Uniform *Uniform
    )
{
    VSC_ErrCode  errCode = VSC_ERR_NONE;
    VIR_Symbol  *sym     = gcvNULL;

    sym = VIR_Shader_GetSymFromId(Dumper->Shader,
        Uniform->sym);
    if(sym == gcvNULL)
    {
        return VSC_ERR_INVALID_ARGUMENT;
    }

    errCode = VIR_Symbol_Dump(Dumper, sym, gcvTRUE);
    CHECK_ERROR(errCode, "VIR_Uniform_Dump");
    if(VIR_Uniform_GetOffset(Uniform) != -1)
    {
        VIR_LOG(Dumper, " offset %d", VIR_Uniform_GetOffset(Uniform));
    }
    if(VIR_Uniform_GetBlockIndex(Uniform) != -1)
    {
        VIR_LOG(Dumper, " ubIndex %d", VIR_Uniform_GetBlockIndex(Uniform));
    }
    if (Dumper->baseDumper.verbose)
    {
        /* dump flags */
        VERIFY_OK(VIR_LOG(Dumper, " uniform_flags:<"));
        if (isSymUniformLoadtimeConst(sym))
        {
            VERIFY_OK(VIR_LOG(Dumper, " load_time_const"));
        }
        if (isSymUniformCompiletimeInitialized(sym))
        {
            VERIFY_OK(VIR_LOG(Dumper, " compile_time_initialized"));
        }
        if (isSymUniformUsedInShader(sym))
        {
            VERIFY_OK(VIR_LOG(Dumper, " used_in_shader"));
        }
        if (isSymUniformUsedInLTC(sym))
        {
            VERIFY_OK(VIR_LOG(Dumper, " used_in_LTC"));
        }
        if (isSymUniformMovedToDUB(sym))
        {
            VERIFY_OK(VIR_LOG(Dumper, " moved_to_DUB"));
        }
        if (isSymUniformUsedInTextureSize(sym))
        {
            VERIFY_OK(VIR_LOG(Dumper, " used_in_texture_size"));
        }
        if (isSymUniformImplicitlyUsed(sym))
        {
            VERIFY_OK(VIR_LOG(Dumper, " implicitly_used"));
        }
        if (isSymUniformForcedToActive(sym))
        {
            VERIFY_OK(VIR_LOG(Dumper, " forced_to_active"));
        }
        if (isSymUniformMovingToDUBO(sym))
        {
            VERIFY_OK(VIR_LOG(Dumper, " moving_to_DUBO"));
        }
        if (isSymUniformAlwaysInDUB(sym))
        {
            VERIFY_OK(VIR_LOG(Dumper, " always_in_DUB"));
        }
        if (isSymUniformMovedToDUBO(sym))
        {
            VERIFY_OK(VIR_LOG(Dumper, " moved_to_DUBO"));
        }
        if (isSymUniformMovedToCUBO(sym))
        {
            VERIFY_OK(VIR_LOG(Dumper, " moved_to_CUBO"));
        }
        if (isSymUniformAtomicCounter(sym))
        {
             VERIFY_OK(VIR_LOG(Dumper, " atomic_counter"));
        }
        if (isSymUniformTreatSamplerAsConst(sym))
        {
            VERIFY_OK(VIR_LOG(Dumper, " Treat_sampler_as_const"));
        }
        VERIFY_OK(VIR_LOG(Dumper, " >"));
    }
    VERIFY_OK(VIR_LOG(Dumper, ";"));
    VIR_LOG_FLUSH(Dumper);

    return errCode;
}

static VSC_ErrCode
_DumpAllUniforms(
    IN OUT VIR_Dumper  *Dumper,
    IN     VIR_Shader  *Shader
    )
{
    VSC_ErrCode errCode   = VSC_ERR_NONE;
    gctSIZE_T i = 0;
    VIR_Uniform *uniform = gcvNULL;

    if (VIR_IdList_Count(&Shader->uniforms) > 0)
    {
        VERIFY_OK(
            VIR_LOG(Dumper, "%s\n", "/* Uniforms */"));
        VIR_LOG_FLUSH(Dumper);
    }

    for(i = 0; i < VIR_IdList_Count(&Shader->uniforms); ++i)
    {
        VIR_Id      id  = VIR_IdList_GetId(&Shader->uniforms, i);
        VIR_Symbol *sym = VIR_Shader_GetSymFromId(Dumper->Shader, id);

        uniform = sym->u2.uniform;
        errCode = VIR_Uniform_Dump(Dumper, uniform);
        CHECK_ERROR(errCode, "DumpShader");

        if(i == VIR_IdList_Count(&Shader->uniforms) - 1)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "\n"));
        }

        VIR_LOG_FLUSH(Dumper);
    }
    return errCode;
}

VSC_ErrCode
VIR_UniformBlock_Dump(
    IN OUT VIR_Dumper       *Dumper,
    IN     VIR_UniformBlock *UniformBlock
    )
{
    VSC_ErrCode errCode   = VSC_ERR_NONE;
    gctSIZE_T   i = 0;
    VIR_Symbol  *sym     = gcvNULL;
    VIR_Type    *type    = gcvNULL;
    VIR_DumpTypeFormat typeFormat = { gcvTRUE, gcvFALSE };

    sym = VIR_Shader_GetSymFromId(Dumper->Shader,
        UniformBlock->sym);
    if(sym == gcvNULL)
    {
        return VSC_ERR_INVALID_ARGUMENT;
    }

    type = VIR_Symbol_GetType(sym);
    if(type == gcvNULL)
    {
        return VSC_ERR_INVALID_ARGUMENT;
    }

    errCode = _DumpTypeWithSpace(Dumper, type, gcvTRUE, typeFormat);
    CHECK_ERROR(errCode, "DumpUniformBlock");

    errCode = _DumpSymbol(Dumper, sym, gcvTRUE, gcvTRUE);
    VIR_LOG(Dumper, " size %d", VIR_UniformBlock_GetBlockSize(UniformBlock));
    CHECK_ERROR(errCode, "DumpUniformBlock");

    VERIFY_OK(
        VIR_LOG(Dumper, " {\n"));

    for(i = 0; i < UniformBlock->uniformCount; ++i)
    {
        errCode = VIR_Uniform_Dump(Dumper, UniformBlock->uniforms[i]);
        CHECK_ERROR(errCode, "DumpUniformBlock");

        VERIFY_OK(
            VIR_LOG(Dumper, ";\n"));
    }

    VERIFY_OK(
        VIR_LOG(Dumper, "};\n"));
    VIR_LOG_FLUSH(Dumper);
    return errCode;
}

VSC_ErrCode
VIR_StorageBlock_Dump(
    IN OUT VIR_Dumper       *Dumper,
    IN     VIR_StorageBlock *StorageBlock
    )
{
    VSC_ErrCode errCode   = VSC_ERR_NONE;
    VIR_Symbol  *sym     = gcvNULL;
    VIR_Type    *type    = gcvNULL;
    VIR_DumpTypeFormat typeFormat = { gcvTRUE, gcvFALSE };

    sym = VIR_Shader_GetSymFromId(Dumper->Shader,
        StorageBlock->sym);
    if(sym == gcvNULL)
    {
        return VSC_ERR_INVALID_ARGUMENT;
    }

    type = VIR_Symbol_GetType(sym);
    if(type == gcvNULL)
    {
        return VSC_ERR_INVALID_ARGUMENT;
    }

    errCode = _DumpTypeWithSpace(Dumper, type, gcvTRUE, typeFormat);
    CHECK_ERROR(errCode, "DumpStorageBlock");

    errCode = _DumpSymbol(Dumper, sym, gcvTRUE, gcvTRUE);
    VIR_LOG(Dumper, " size %d", VIR_SBO_GetBlockSize(StorageBlock));
    CHECK_ERROR(errCode, "DumpStorageBlock");

    VERIFY_OK(
        VIR_LOG(Dumper, " {\n"));

    VERIFY_OK(
        VIR_LOG(Dumper, "};\n"));
    VIR_LOG_FLUSH(Dumper);
    return errCode;
}

VSC_ErrCode
VIR_Shader_Dump(
    IN gctFILE          File,
    IN gctCONST_STRING  Text,
    IN VIR_Shader      *Shader,
    IN gctBOOL          PrintHeaderFooter
    )
{
    VSC_ErrCode        errCode   = VSC_ERR_NONE;
    VIR_FuncIterator   iter;
    VIR_Dumper*        dumper = Shader->dumper;

    gcmHEADER_ARG("File=%p Text=0x%x Shader=%p",
        File, Text, Shader);

    gcmTRACE(gcvLEVEL_VERBOSE, "Text=%s", Text);

    gcmASSERT(Shader != gcvNULL);

    if (VIR_Shader_IsLibraryShader(Shader) && !gcmGetOptimizerOption()->includeLib)
    {
        gcmFOOTER_ARG("errCode=%d", errCode);
        return errCode;
    }

    /* re-number shader before dump */
    if (gcmGetOptimizerOption()->renumberInst)
    {
        VIR_Shader_RenumberInstId(Shader);
    }

    *dumper->baseDumper.pOffset = 0;

    /******************************************************* Header *********/
    if(PrintHeaderFooter)
    {
        VERIFY_OK(
            VIR_LOG(dumper, "%s\n", VSC_TRACE_STAR_LINE));

        VERIFY_OK(
            VIR_LOG(dumper, "%s\n", Text));

        VERIFY_OK(
            VIR_LOG(dumper, "%s\n", VSC_TRACE_STAR_LINE));

        errCode = _DumpVersion(dumper);
        CHECK_ERROR(errCode, "DumpShader");

        VIR_LOG_FLUSH(dumper);
    }

    /*****************************************************************************
    ******************************************************* Dump declarations ****
    *****************************************************************************/

    /******************************************************* Global variables ***/
    if (VIR_IdList_Count(&Shader->variables) > 0)
    {
        errCode = _DumpVariableList(dumper, &Shader->symTable,
            &Shader->variables, ";\n", gcvTRUE, "/* Global variables */");
        CHECK_ERROR(errCode, "DumpShader");
        VERIFY_OK(
            VIR_LOG(dumper, "\n"));
    }
    /******************************************************* Attributes *********/
    if (VIR_IdList_Count(&Shader->attributes) > 0)
    {
        errCode = _DumpVariableList(dumper, &Shader->symTable,
            &Shader->attributes, ";\n", gcvTRUE, "/* Attributes */");
        CHECK_ERROR(errCode, "DumpShader");
        VERIFY_OK(
            VIR_LOG(dumper, "\n"));
    }

    /******************************************************* Outputs ************/
    if (VIR_IdList_Count(&Shader->outputs) > 0)
    {
        errCode = _DumpVariableList(dumper, &Shader->symTable,
            &Shader->outputs, ";\n", gcvTRUE, "/* Outputs */");
        CHECK_ERROR(errCode, "DumpShader");
        VERIFY_OK(
            VIR_LOG(dumper, "\n"));
    }
    /******************************************************* Per Patch Input *********/
    if (VIR_IdList_Count(&Shader->perpatchInput) > 0)
    {
        errCode = _DumpVariableList(dumper, &Shader->symTable,
            &Shader->perpatchInput, ";\n", gcvTRUE, "/* Per Patch Input */");
        CHECK_ERROR(errCode, "DumpShader");
        VERIFY_OK(
            VIR_LOG(dumper, "\n"));
    }

    /******************************************************* Per Patch Output ************/
    if (VIR_IdList_Count(&Shader->perpatchOutput) > 0)
    {
        errCode = _DumpVariableList(dumper, &Shader->symTable,
            &Shader->perpatchOutput, ";\n", gcvTRUE, "/* Per Patch Output */");
        CHECK_ERROR(errCode, "DumpShader");
        VERIFY_OK(
            VIR_LOG(dumper, "\n"));
    }

    /******************************************************* Uniforms ***********/
    /* Shader->uniforms; */
    errCode = _DumpAllUniforms(dumper, Shader);

    /******************************************************* Uniform blocks *****/
    /* Shader->uniformBlocks; */
    {
        gctSIZE_T i = 0;
        VIR_UniformBlock *uniformBlock = gcvNULL;

        if (VIR_IdList_Count(&Shader->uniformBlocks) > 0)
        {
            VERIFY_OK(
                VIR_LOG(dumper, "%s\n", "/* Uniform blocks */"));
            VIR_LOG_FLUSH(dumper);
        }

        for(i = 0; i < VIR_IdList_Count(&Shader->uniformBlocks); ++i)
        {
            VIR_Id      id  = VIR_IdList_GetId(&Shader->uniformBlocks, i);
            VIR_Symbol *sym = VIR_Shader_GetSymFromId(dumper->Shader, id);
            uniformBlock = sym->u2.ubo;

            errCode = VIR_UniformBlock_Dump(dumper, uniformBlock);
            CHECK_ERROR(errCode, "DumpShader");

            VERIFY_OK(
                VIR_LOG(dumper, "\n"));

            if(i == VIR_IdList_Count(&Shader->uniformBlocks) - 1)
            {
                VERIFY_OK(
                    VIR_LOG(dumper, "\n"));
            }

            VIR_LOG_FLUSH(dumper);
        }
    }

    /******************************************************* Storage blocks *****/
    /* Shader->storageBlocks; */
    {
        gctSIZE_T i = 0;
        VIR_StorageBlock *storageBlock = gcvNULL;

        if (VIR_IdList_Count(&Shader->storageBlocks) > 0)
        {
            VERIFY_OK(
                VIR_LOG(dumper, "%s\n", "/* Storage blocks */"));
            VIR_LOG_FLUSH(dumper);
        }

        for(i = 0; i < VIR_IdList_Count(&Shader->storageBlocks); ++i)
        {
            VIR_Id      id  = VIR_IdList_GetId(&Shader->storageBlocks, i);
            VIR_Symbol *sym = VIR_Shader_GetSymFromId(dumper->Shader, id);
            storageBlock = sym->u2.sbo;

            errCode = VIR_StorageBlock_Dump(dumper, storageBlock);
            CHECK_ERROR(errCode, "DumpShader");

            VERIFY_OK(
                VIR_LOG(dumper, "\n"));

            if(i == VIR_IdList_Count(&Shader->storageBlocks) - 1)
            {
                VERIFY_OK(
                    VIR_LOG(dumper, "\n"));
            }

            VIR_LOG_FLUSH(dumper);
        }
    }

    /******************************************************************************
    ******************************************************** Dump functions *******
    ******************************************************************************/
    {
        VIR_FunctionNode *funcNode;

        VIR_FuncIterator_Init(&iter, &Shader->functions);
        funcNode = VIR_FuncIterator_First(&iter);
        for (; funcNode != gcvNULL; funcNode = VIR_FuncIterator_Next(&iter))
        {
            errCode = VIR_Function_Dump(dumper, funcNode->function);
            CHECK_ERROR(errCode, "DumpShader");

            VERIFY_OK(
                VIR_LOG(dumper, "\n"));
        }
    }

    if(PrintHeaderFooter)
    {
        VERIFY_OK(
            VIR_LOG(dumper, "%s\n", VSC_TRACE_BAR_LINE));
    }

    VIR_LOG_FLUSH(dumper);

    gcmFOOTER_ARG("errCode=%d", errCode);
    return errCode;
}

/* dump instruction to debugger output */
void dbg_dumpVIR(IN VIR_Instruction *Inst)
{
    char         buffer[4096];
    VIR_Dumper   dumper;
    VIR_Function *func = VIR_Inst_GetFunction(Inst);

    gcmASSERT(func != gcvNULL && func->hostShader != gcvNULL);

    gcoOS_ZeroMemory(&dumper, sizeof(dumper));

    dumper.Shader  = func->hostShader;
    dumper.dumpOperandId = VIR_DUMP_OPNDIDX;
    /* reset dumper buffer */
    vscDumper_Initialize(&dumper.baseDumper, gcvNULL, gcvNULL, buffer, sizeof(buffer));

    VIR_Inst_Dump(&dumper, Inst);
    VERIFY_OK(
        VIR_LOG(&dumper, "\n"));
    VIR_LOG_FLUSH(&dumper);
}

void dbg_dumpVType(
    IN VIR_Shader *     Shader,
    IN  VIR_Type *      Type
    )
{
    char         buffer[4096];
    VIR_Dumper   dumper;
    VIR_DumpTypeFormat typeFormat = { gcvTRUE, gcvFALSE };

    gcoOS_ZeroMemory(&dumper, sizeof(dumper));

    dumper.Shader  = Shader;
    dumper.dumpOperandId = VIR_DUMP_OPNDIDX;
    /* reset dumper buffer */
    vscDumper_Initialize(&dumper.baseDumper, gcvNULL, gcvNULL, buffer, sizeof(buffer));

    /* dump type id */
    VERIFY_OK(
        VIR_LOG(&dumper, "Type id(%d) ", VIR_Type_GetIndex(Type)));

    _DumpType(&dumper, Type, gcvTRUE, typeFormat);
    VERIFY_OK(
        VIR_LOG(&dumper, "\n"));
    VIR_LOG_FLUSH(&dumper);
}

void dbg_dumpVTypeId(
    IN VIR_Shader *     Shader,
    IN  VIR_TypeId      TypeId
    )
{
    VIR_Type *    ty;
    ty = VIR_Shader_GetTypeFromId(Shader, TypeId);
    if (ty != gcvNULL)
    {
        dbg_dumpVType(Shader, ty);
    }
}

void dbg_dumpVOperand(
    IN VIR_Instruction * Inst,
    IN VIR_Operand *     Operand
    )
{
    char         buffer[4096];
    VIR_Dumper   dumper;
     VIR_Shader * shader = VIR_Inst_GetShader(Inst);

    gcoOS_ZeroMemory(&dumper, sizeof(dumper));

    dumper.Shader  = shader;
    dumper.dumpOperandId = VIR_DUMP_OPNDIDX;
    /* reset dumper buffer */
    vscDumper_Initialize(&dumper.baseDumper, gcvNULL, gcvNULL, buffer, sizeof(buffer));

    /* dump type id */
    VERIFY_OK(
        VIR_LOG(&dumper, "Operand id(%d): ", VIR_Operand_GetIndex(Operand)));
    _DumpOperand(&dumper, Inst, Operand, gcvTRUE);

    VERIFY_OK(
        VIR_LOG(&dumper, "\n"));
    VIR_LOG_FLUSH(&dumper);
}

void dbg_dumpVSym(
    IN VIR_Shader *     Shader,
    IN  VIR_Symbol *    Sym
    )
{
    char         buffer[4096];
    VIR_Dumper   dumper;
    VIR_DumpTypeFormat typeFormat = { gcvTRUE, gcvFALSE };

    gcoOS_ZeroMemory(&dumper, sizeof(dumper));

    dumper.Shader  = Shader;
    dumper.dumpOperandId = VIR_DUMP_OPNDIDX;

    /* reset dumper buffer */
    vscDumper_Initialize(&dumper.baseDumper, gcvNULL, gcvNULL, buffer, sizeof(buffer));
    VIR_LOG(&dumper, "%s %d(0x%x): ", VIR_GetSymbolKindName(VIR_Symbol_GetKind(Sym)),
                                      VIR_Symbol_GetIndex(Sym), VIR_Symbol_GetIndex(Sym));

    _DumpType(&dumper, VIR_Symbol_GetType(Sym), gcvTRUE, typeFormat);
    VERIFY_OK(VIR_LOG(&dumper, " "));
    _DumpSymbol(&dumper, Sym, gcvTRUE, gcvTRUE);
    VERIFY_OK(VIR_LOG(&dumper, "\n"));
    VIR_LOG_FLUSH(&dumper);
}

void dbg_dumpVSymId(
    IN VIR_Shader *     Shader,
    IN  VIR_SymId       SymId
    )
{
    VIR_Symbol *    sym;
    sym = VIR_Shader_GetSymFromId(Shader, SymId);
    if (sym != gcvNULL)
    {
        dbg_dumpVSym(Shader, sym);
    }
}

void dbg_dumpVLSymId(
    IN  VIR_Function *  Func,
    IN  VIR_SymId       SymId
    )
{
    VIR_Symbol *    sym;
    sym = VIR_Function_GetSymFromId(Func, SymId);
    if (sym != gcvNULL)
    {
        dbg_dumpVSym(Func->hostShader, sym);
    }
}

void dbg_dumpVNameId(
    IN VIR_Shader *     Shader,
    IN VIR_NameId       NameId
    )
{
    char         buffer[4096];
    VIR_Dumper   dumper;

    gcoOS_ZeroMemory(&dumper, sizeof(dumper));

    dumper.Shader  = Shader;
    dumper.dumpOperandId = VIR_DUMP_OPNDIDX;

    /* reset dumper buffer */
    vscDumper_Initialize(&dumper.baseDumper, gcvNULL, gcvNULL, buffer, sizeof(buffer));

    VERIFY_OK(VIR_LOG(&dumper, "%s\n", VIR_Shader_GetStringFromId(Shader, NameId)));
    VIR_LOG_FLUSH(&dumper);
}

/* dump shader to debugger output */
void dbg_dumpVShader(IN VIR_Shader *Shader)
{
    VIR_Shader_Dump(gcvNULL, "Dump Shader", Shader, gcvTRUE);
}

void dbg_dumpVFunc(IN VIR_Function     *Func)
{
    char         buffer[4096];
    VIR_Dumper   dumper;

    gcoOS_ZeroMemory(&dumper, sizeof(dumper));

    dumper.Shader  = VIR_Function_GetShader(Func);
    dumper.dumpOperandId = VIR_DUMP_OPNDIDX;

    /* reset dumper buffer */
    vscDumper_Initialize(&dumper.baseDumper, gcvNULL, gcvNULL, buffer, sizeof(buffer));
    VIR_Function_Dump(&dumper, Func);
    return;
}

void dbg_dumpMCode(
    IN void *  Mcode,
    IN gctBOOL IsDual16Shader
    )
{
    gctUINT       dumpBufferSize = 1024;
    gctCHAR*      pDumpBuffer;
    VSC_DUMPER    vscDumper;
    VSC_MC_CODEC  mcCodec;

    gcoOS_Allocate(gcvNULL, dumpBufferSize, (gctPOINTER*)&pDumpBuffer);

    vscDumper_Initialize(&vscDumper,
                         gcvNULL,
                         gcvNULL,
                         pDumpBuffer,
                         dumpBufferSize);

    vscMC_BeginCodec(&mcCodec, &gcHWCaps, IsDual16Shader, gcvFALSE);
    vscMC_DumpInst(&mcCodec, (VSC_MC_RAW_INST*)Mcode, 0, &vscDumper);

    return;
}

gctINT vscDumpOption =
0; /* gceLTC_DUMP_EVALUATION | gceLTC_DUMP_EXPESSION | gceLTC_DUMP_COLLECTING | gceLTC_DUMP_UNIFORM;*/

/* return true if the dump option Opt is set, otherwise return false */
gctBOOL
vscVIR_DumpOption(gctINT Opt)
{
    return (vscDumpOption & Opt) != 0;
}

VSC_ErrCode
VIR_BasicBlock_Name_Dump(
    IN OUT VIR_Dumper *Dumper,
    IN VIR_BB         *Bb
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    gcmASSERT(Dumper != gcvNULL &&
        Dumper->baseDumper.pOffset != gcvNULL &&
        Dumper->Shader != gcvNULL &&
        Bb != gcvNULL);

    switch(Bb->flowType)
    {
    case VIR_FLOW_TYPE_ENTRY:
        VERIFY_OK(
            VIR_LOG(Dumper, "%d (Entry)", Bb->dgNode.id));
        return errCode;
    case VIR_FLOW_TYPE_EXIT:
        VERIFY_OK(
            VIR_LOG(Dumper, "%d (Exit)", Bb->dgNode.id));
        return errCode;
    default:
        VERIFY_OK(
            VIR_LOG(Dumper, "%d", Bb->dgNode.id));
        return errCode;
    }

}

static VSC_ErrCode
_DumpBasicBlockEdge(
    IN OUT VIR_Dumper    *Dumper,
    IN VSC_ADJACENT_LIST *List
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_ADJACENT_LIST_ITERATOR   edgeIter;
    VIR_CFG_EDGE*                pEdge;

    VSC_ADJACENT_LIST_ITERATOR_INIT(&edgeIter, List);
    pEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edgeIter);
    if(pEdge == gcvNULL)
    {
        VERIFY_OK(
            VIR_LOG(Dumper, "(NULL)"));
    }
    else
    {
        for (; pEdge != gcvNULL; pEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edgeIter))
        {
            VIR_BB *BasicBlk = CFG_EDGE_GET_TO_BB(pEdge);

            errCode = VIR_BasicBlock_Name_Dump(Dumper, BasicBlk);
            CHECK_ERROR(errCode, "_DumpBasicBlockEdge");
            VIR_LOG(Dumper, "%s, ", CFG_EDGE_GET_TYPE(pEdge) == VIR_CFG_EDGE_TYPE_ALWAYS ? "(always)" :
                                    CFG_EDGE_GET_TYPE(pEdge) == VIR_CFG_EDGE_TYPE_TRUE ? "(true)" : "(false)");
        }
    }

    return errCode;
}

static VSC_ErrCode
_DumpBasicBlockInOutLength(
    IN OUT VIR_Dumper *Dumper,
    IN VIR_BB         *Bb
    )
{
    VSC_ErrCode      errCode = VSC_ERR_NONE;

    VERIFY_OK(
        VIR_LOG(Dumper, "/* BasicBlock: "));
    errCode = VIR_BasicBlock_Name_Dump(Dumper, Bb);
    CHECK_ERROR(errCode, "_DumpBasicBlockInOutLength");

    VERIFY_OK(
        VIR_LOG(Dumper, " Pred: "));
    errCode = _DumpBasicBlockEdge(Dumper, &Bb->dgNode.predList);
    CHECK_ERROR(errCode, "_DumpBasicBlockInOutLength");

    VERIFY_OK(
        VIR_LOG(Dumper, " Succ: "));
    errCode = _DumpBasicBlockEdge(Dumper, &Bb->dgNode.succList);
    CHECK_ERROR(errCode, "_DumpBasicBlockInOutLength");

    VERIFY_OK(VIR_LOG(Dumper, " Length: %d", BB_GET_LENGTH(Bb)));

    VERIFY_OK(
        VIR_LOG(Dumper, " */\n"));

    VIR_LOG_FLUSH(Dumper);
    return errCode;
}

VSC_ErrCode
VIR_BasicBlock_Dump(
    IN OUT VIR_Dumper   *Dumper,
    IN VIR_BB           *Bb,
    IN gctBOOL           Indent
    )
{
    VSC_ErrCode      errCode = VSC_ERR_NONE;
    VIR_Instruction *inst    = gcvNULL;
    gctSIZE_T        i       = 0;

    gcmASSERT(Dumper != gcvNULL &&
        Dumper->baseDumper.pOffset != gcvNULL &&
        Dumper->Shader != gcvNULL &&
        Bb != gcvNULL);

    errCode = _DumpBasicBlockInOutLength(Dumper, Bb);
    CHECK_ERROR(errCode, "VIR_BasicBlock_Dump");
    VIR_LOG_FLUSH(Dumper);

    if(BB_GET_LENGTH(Bb) == 0)
    {
        if(Indent)
        {
            _DumpTab(Dumper);
        }
        VERIFY_OK(VIR_LOG(Dumper, "NULL\n"));
    }
    else
    {
        for(i = 0, inst = BB_GET_START_INST(Bb); i < BB_GET_LENGTH(Bb);
            ++i, inst = VIR_Inst_GetNext(inst))
        {
            if(Indent)
            {
                _DumpTab(Dumper);
            }
            errCode = VIR_Inst_Dump(Dumper, inst);
            CHECK_ERROR(errCode, "VIR_BasicBlock_Dump");

            VIR_LOG_FLUSH(Dumper);
        }
    }

    VIR_LOG_FLUSH(Dumper);
    return errCode;
}

VSC_ErrCode
VIR_BasicBlock_DumpRange(
    IN OUT VIR_Dumper  *Dumper,
    IN VIR_BASIC_BLOCK *BbStart,
    IN VIR_BASIC_BLOCK *BbEnd,
    IN gctBOOL          Indent
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_BB* bb = BbStart;

    while(gcvTRUE)
    {
        errCode = VIR_BasicBlock_Dump(Dumper, bb, Indent);
        if(errCode)
        {
            return errCode;
        }

        if(bb == BbEnd)
        {
            break;
        }
        else
        {
            bb = VIR_BB_GetFollowingBB(bb);
        }
    }

    return errCode;
}

static VSC_ErrCode
_DumpTreeNode(
    IN OUT VIR_Dumper    *Dumper,
    IN VIR_DOM_TREE_NODE *Node
    )
{
    VSC_ErrCode    errCode = VSC_ERR_NONE;
    VSC_TREE_NODE *childNode;
    gctSIZE_T      i       = 0;
    VSC_CHILD_LIST_ITERATOR nodeIter;

    gcmASSERT(Dumper != gcvNULL &&
        Dumper->baseDumper.pOffset != gcvNULL &&
        Node != gcvNULL);

    /* Indent */
    for(i = 0; i < Node->treeNode.depth; ++i)
    {
        VERIFY_OK(
            VIR_LOG(Dumper, "  "));
    }


    /* Dump basic block name */
    VERIFY_OK(
        VIR_LOG(Dumper, "Name:"));

    errCode = VIR_BasicBlock_Name_Dump(Dumper, Node->pOwnerBB);
    CHECK_ERROR(errCode, "_DumpTreeNode");

    VERIFY_OK(
        VIR_LOG(Dumper, "\n"));


    /* Dump children */
    VSC_CHILD_LIST_ITERATOR_INIT(&nodeIter, &Node->treeNode);
    for(childNode = VSC_CHILD_LIST_ITERATOR_FIRST(&nodeIter); childNode != gcvNULL;
        childNode = VSC_CHILD_LIST_ITERATOR_NEXT(&nodeIter))
    {
        errCode = _DumpTreeNode(Dumper, (VIR_DOM_TREE_NODE *)childNode);
        CHECK_ERROR(errCode, "_DumpTreeNode");
    }

    return errCode;
}

VSC_ErrCode
VIR_DomTree_Dump(
    IN OUT VIR_Dumper    *Dumper,
    IN VIR_DOM_TREE      *DomTree
    )
{
    VSC_ErrCode        errCode = VSC_ERR_NONE;

    gcmASSERT(Dumper != gcvNULL &&
        Dumper->baseDumper.pOffset != gcvNULL &&
        DomTree != gcvNULL);

    errCode = _DumpTreeNode(Dumper, (VIR_DOM_TREE_NODE *)DomTree->tree.pRootNode);
    CHECK_ERROR(errCode, "VIR_DomTree_Dump");

    VIR_LOG_FLUSH(Dumper);
    return errCode;
}

VSC_ErrCode
VIR_CFG_Dump(
    IN OUT VIR_Dumper   *Dumper,
    IN VIR_CFG          *Cfg,
    IN gctBOOL          Indent
    )
{
    VSC_ErrCode      errCode = VSC_ERR_NONE;
    CFG_ITERATOR     cfg_iter;
    VIR_BB           *bb;

    gcmASSERT(Dumper != gcvNULL &&
        Dumper->Shader != gcvNULL &&
        Dumper->baseDumper.pOffset != gcvNULL &&
        Cfg != gcvNULL);

    VIR_LOG(Dumper, "/* Function instruction count [%d] */\n\n",
            VIR_Function_GetInstCount(Cfg->pOwnerFuncBlk->pVIRFunc));

    /* Dump basic blocks. */
    CFG_ITERATOR_INIT(&cfg_iter, Cfg);
    for(bb = CFG_ITERATOR_FIRST(&cfg_iter); bb != gcvNULL;
        bb = CFG_ITERATOR_NEXT(&cfg_iter))
    {
        errCode = VIR_BasicBlock_Dump(Dumper, bb, Indent);
        CHECK_ERROR(errCode, "VIR_CFG_Dump");
        VIR_LOG_FLUSH(Dumper);
    }


    return errCode;
}

static void
_VIR_Dump_Usage(
    IN OUT VIR_Dumper*      Dumper,
    IN VIR_USAGE*           pUsage
    )
{
    VIR_Instruction* pInst = pUsage->usageKey.pUsageInst;
    gctUINT          srcIdx = 0xFF, i;

    if (pInst == VIR_OUTPUT_USAGE_INST)
    {
        /* Fixed function inst */
        VIR_LOG(Dumper, "FF_INST");
    }
    else
    {
        for(i = 0; i < VIR_Inst_GetSrcNum(pInst); i++)
        {
            if (VIR_Inst_GetSource(pInst, i) == pUsage->usageKey.pOperand)
            {
                srcIdx = i;
                break;
            }
        }

        VIR_LOG(Dumper, "src%d of inst%d(%s)",
                srcIdx,
                pInst->id_,
                VIR_OPCODE_GetName(VIR_Inst_GetOpcode(pInst)));
    }
}

static VSC_ErrCode
_VIR_Def_Dump(
    IN OUT VIR_Dumper       *Dumper,
    IN VIR_DEF_USAGE_INFO   *pDuInfo,
    IN VIR_DEF              *pDef,
    IN gctBOOL              bOnlyKey,
    IN gctBOOL              bSkipPatchVertexDef
    )
{
    VSC_ErrCode              errCode = VSC_ERR_NONE;
    VIR_DU_CHAIN_USAGE_NODE* pUsageNode;
    VSC_DU_ITERATOR          duIter;
    gctBOOL                  bFirstUsage = gcvTRUE;
    VIR_USAGE*               pUsage;
    VSC_BLOCK_TABLE*         pUsageTable = &pDuInfo->usageTable;

    static const char* _ChannelsName[4] =
    {
        "x",
        "y",
        "z",
        "w",
    };

    if (!IS_VALID_DEF(pDef))
    {
        VIR_LOG(Dumper, "deleted\n");
        return errCode;
    }

    if (bSkipPatchVertexDef &&
        (pDef->flags.nativeDefFlags.bIsPerVtxCp ||
         pDef->flags.nativeDefFlags.bIsPerPrim))
    {
        return errCode;
    }

    VIR_LOG(Dumper, "t%d.%s, ", pDef->defKey.regNo,
            _ChannelsName[pDef->defKey.channel]);

    if (!bOnlyKey)
    {
        if (pDef->defKey.pDefInst == VIR_INPUT_DEF_INST)
        {
            /* Fixed function inst */
            VIR_LOG(Dumper, " at FF_INST");
        }
        else if (pDef->defKey.pDefInst == VIR_HW_SPECIAL_DEF_INST)
        {
            /* Hw special inst */
            VIR_LOG(Dumper, " at HW_SPECIAL_INST");
        }
        else
        {
            VIR_LOG(Dumper, " at inst%d (%s)", pDef->defKey.pDefInst->id_,
                    VIR_OPCODE_GetName(VIR_Inst_GetOpcode(pDef->defKey.pDefInst)));
        }
        VIR_LOG(Dumper, ", next def%d  (webIdx:%d nextDefInWeb %d)",
                pDef->nextDefIdxOfSameRegNo == VIR_INVALID_DEF_INDEX ? -1 : pDef->nextDefIdxOfSameRegNo,
                pDef->webIdx,
                pDef->nextDefInWebIdx == VIR_INVALID_DEF_INDEX ? -1 : pDef->nextDefInWebIdx);
        VIR_LOG_FLUSH(Dumper);

        /* DU chain */
        VIR_LOG(Dumper, "    DU-chain [");
        VSC_DU_ITERATOR_INIT(&duIter, &pDef->duChain);
        pUsageNode = VSC_DU_ITERATOR_FIRST(&duIter);
        for (; pUsageNode != gcvNULL; pUsageNode = VSC_DU_ITERATOR_NEXT(&duIter))
        {
            pUsage = GET_USAGE_BY_IDX(pUsageTable, pUsageNode->usageIdx);
            gcmASSERT(pUsage);

            if (IS_VALID_USAGE(pUsage))
            {
                if (!bFirstUsage)
                {
                    VIR_LOG(Dumper, ",");
                    VIR_LOG_FLUSH(Dumper);
                    VIR_LOG(Dumper, "              ");
                }
                _VIR_Dump_Usage(Dumper, pUsage);

                if (bFirstUsage)
                {
                    bFirstUsage = gcvFALSE;
                }
            }
        }
        VIR_LOG(Dumper, " ]");
        VIR_LOG_FLUSH(Dumper);
    }

    return errCode;
}

void _PrintDefVector(
    VIR_Dumper      *pDumper,
    VIR_LIVENESS_INFO   *pLvInfo,
    VSC_BIT_VECTOR  *pBV)
{
    gctUINT     i = 0, index, count = 0;
    VIR_DEF     *pDef;

    while ((index = vscBV_FindSetBitForward(pBV, i)) != (gctUINT)INVALID_BIT_LOC)
    {
        pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, index);
        /* skip the per-vertex/per-patch data,
           since they are load_attr/store_attr and are not in RA's data flow */
        _VIR_Def_Dump(pDumper, pLvInfo->pDuInfo, pDef, gcvTRUE, gcvTRUE);
        i = index + 1;

        /* make print line not too long */
        if ((++count % 12) == 0)
        {
            VIR_LOG_FLUSH(pDumper);
        }
    }
}

VSC_ErrCode
VIR_LIVENESS_BB_Dump(
    IN OUT VIR_Dumper       *Dumper,
    IN VIR_LIVENESS_INFO    *pLvInfo,
    IN VIR_TS_FUNC_FLOW     *pFuncFlow,
    IN VIR_BB               *Bb
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_TS_BLOCK_FLOW*  pBlkFlow = (VIR_TS_BLOCK_FLOW*)vscSRARR_GetElement(
                                    &pFuncFlow->tsBlkFlowArray, Bb->dgNode.id);

    VIR_BasicBlock_Dump(Dumper, Bb, gcvTRUE);
    VIR_LOG(Dumper, "inFlow:[ ");
    _PrintDefVector(Dumper, pLvInfo, &pBlkFlow->inFlow);
    VIR_LOG(Dumper, "]\n");
    VIR_LOG_FLUSH(Dumper);
    VIR_LOG(Dumper, "genFlow:[ ");
    _PrintDefVector(Dumper, pLvInfo, &pBlkFlow->genFlow);
    VIR_LOG(Dumper, "]\n");
    VIR_LOG_FLUSH(Dumper);
    VIR_LOG(Dumper, "killFlow:[ ");
    _PrintDefVector(Dumper, pLvInfo, &pBlkFlow->killFlow);
    VIR_LOG(Dumper, "]\n");
    VIR_LOG_FLUSH(Dumper);
    VIR_LOG(Dumper, "outFlow:[ ");
    _PrintDefVector(Dumper, pLvInfo, &pBlkFlow->outFlow);
    VIR_LOG(Dumper, "]\n\n");
    VIR_LOG_FLUSH(Dumper);

    return errCode;
}

VSC_ErrCode
VIR_CFG_LIVENESS_Dump(
    IN OUT VIR_Dumper       *Dumper,
    IN VIR_LIVENESS_INFO    *pLvInfo,
    IN VIR_CFG              *Cfg)
{
    VSC_ErrCode       errCode = VSC_ERR_NONE;
    CFG_ITERATOR      cfg_iter;
    VIR_BB            *bb;
    gctUINT           funcId = Cfg->pOwnerFuncBlk->dgNode.id;
    VIR_TS_FUNC_FLOW* pFuncFlow = (VIR_TS_FUNC_FLOW*)vscSRARR_GetElement(
                                    &pLvInfo->baseTsDFA.tsFuncFlowArray, funcId);

    gcmASSERT(Dumper != gcvNULL &&
        Dumper->Shader != gcvNULL &&
        Dumper->baseDumper.pOffset != gcvNULL &&
        Cfg != gcvNULL);

    /* Dump basic blocks. */
    CFG_ITERATOR_INIT(&cfg_iter, Cfg);
    for (bb = CFG_ITERATOR_FIRST(&cfg_iter); bb != gcvNULL;
         bb = CFG_ITERATOR_NEXT(&cfg_iter))
    {
        errCode = VIR_LIVENESS_BB_Dump(Dumper, pLvInfo, pFuncFlow, bb);
        VIR_LOG_FLUSH(Dumper);
    }

    return errCode;
}

VSC_ErrCode
VIR_DU_Info_Dump(
    IN VIR_DEF_USAGE_INFO* pDuInfo
)
{
    VSC_ErrCode      errCode = VSC_ERR_NONE;
    VIR_Dumper*      Dumper = pDuInfo->baseTsDFA.baseDFA.pOwnerCG->pOwnerShader->dumper;
    VSC_BLOCK_TABLE* pDefTable = &pDuInfo->defTable;
    gctUINT          defIdx;
    VIR_DEF*         pDef;

    for (defIdx = 0; defIdx < BT_GET_MAX_VALID_ID(pDefTable); defIdx ++)
    {
        pDef = GET_DEF_BY_IDX(pDefTable, defIdx);
        gcmASSERT(pDef);

        VIR_LOG(Dumper, "def%d: ", defIdx);
        _VIR_Def_Dump(Dumper, pDuInfo, pDef, gcvFALSE, gcvFALSE);
        VIR_LOG_FLUSH(Dumper);
    }

    return errCode;
}

/* According OCL spec: the type name returned will be
 * the argument type name as it was declared with any
 * whitespace removed. If argument type name is an
 * unsigned scalar type (i.e. unsigned char, unsigned
 * short, unsigned int, unsigned long), uchar, ushort,
 * uint and ulong will be returned. The argument type
 * name returned does not include any type qualifiers.
 */
VSC_ErrCode
VIR_Dump_OCLTypeName(
    IN VIR_Shader *        pShader,
    IN VIR_TypeId          TypeId,
    IN OUT gctSTRING       Buffer,
    IN gctUINT             Length
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    char                buffer[1024];
    VIR_Dumper          dumper;
    VIR_DumpTypeFormat  typeFormat = { gcvFALSE, gcvFALSE, gcvTRUE, gcvTRUE, gcvTRUE, gcvTRUE };
    VIR_Type *          type = VIR_Shader_GetTypeFromId(pShader, TypeId);

    gcmASSERT(type != gcvNULL);
    gcoOS_ZeroMemory(&dumper, sizeof(dumper));
    vscDumper_Initialize(&dumper.baseDumper, gcvNULL, gcvNULL, buffer, sizeof(buffer));

    dumper.Shader  = pShader;
    dumper.dumpOperandId = VIR_DUMP_OPNDIDX;

    errCode = _DumpType(&dumper, type, gcvTRUE, typeFormat);
    ON_ERROR(errCode, "Dump OCL type name");

    if (Length < (gctUINT)dumper.baseDumper.curOffset)
    {
        errCode = VSC_ERR_OUT_OF_BOUNDS;
        ON_ERROR(errCode, "Dump OCL type name buffer too small");
    }
    gcoOS_StrCopySafe(Buffer, Length, buffer);
OnError:
    return errCode;
}

