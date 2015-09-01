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


#ifndef __gc_vsc_err_h_
#define __gc_vsc_err_h_

BEGIN_EXTERN_C()

typedef enum _VSC_ERRCODE
{
    /* No error */
    VSC_ERR_NONE                  =   0,

    /* General errors */
    VSC_ERR_INVALID_ARGUMENT      =   1,
    VSC_ERR_NOT_SUPPORTED         =   2,
    VSC_ERR_INVALID_DATA          =   3,

    VSC_ERR_OUT_OF_MEMORY         =   4,
    VSC_ERR_OUT_OF_RESOURCE       =   5,
    VSC_ERR_VERSION_MISMATCH      =   6,
    VSC_ERR_REDEFINITION          =   7,

    VSC_ERR_CG_NOT_BUILT          =   8,

    /* register allocation errors */
    VSC_RA_ERR_OUT_OF_REG_FAIL    =   100, /* not enough registers, RA could not succeed */
    VSC_RA_ERR_OUT_OF_REG_SPILL   =   101, /* not enough registers, RA will try spilling */

    /* Linker errors. */
    VSC_ERR_GLOBAL_TYPE_MISMATCH  =   1000,
    VSC_ERR_TOO_MANY_ATTRIBUTES   =   1001,
    VSC_ERR_TOO_MANY_VARYINGS     =   1002,
    VSC_ERR_TOO_MANY_OUTPUTS      =   1003,
    VSC_ERR_UNDECLARED_VARYING    =   1004,
    VSC_ERR_VARYING_TYPE_MISMATCH =   1005,
    VSC_ERR_MISSING_MAIN          =   1006,
    VSC_ERR_NAME_MISMATCH         =   1007,
    VSC_ERR_INVALID_INDEX         =   1008,
    VSC_ERR_UNIFORMS_TOO_MANY     =   1009,
    VSC_ERR_UNIFORM_TYPE_MISMATCH =   1010,
    VSC_ERR_UNIFORM_LOC_OVERLAP   =   1011,
    VSC_ERR_UNIFORM_LOC_MISMATCH  =   1012,

    /* misc error */
    VSC_ERR_INVALID_TYPE          =   4000,
} VSC_ErrCode;

void vscERR_ReportError(const char* file, gctUINT line, VSC_ErrCode status, const char* format, ...);
void vscERR_ReportWarning(const char* file, gctUINT line, VSC_ErrCode status, const char* format, ...);

#define VSC_ERR_DUMP 0

#if VSC_ERR_DUMP
#define ERR_REPORT(status, ...)   vscERR_ReportError(__FILE__, __LINE__, (status), __VA_ARGS__)
#else
#define ERR_REPORT(status, ...)
#endif

#define CHECK_ERROR(Err, ...)        if ((Err) != VSC_ERR_NONE) {ERR_REPORT((Err), __VA_ARGS__); return (Err); }
#define ON_ERROR(Err, ...)           if ((Err) != VSC_ERR_NONE) {ERR_REPORT((Err), __VA_ARGS__); goto OnError; }
#define CHECK_ERROR2STATUS(Err, ...) if ((Err) != VSC_ERR_NONE) {ERR_REPORT((Err), __VA_ARGS__); return vscERR_CastErrCode2GcStatus(Err); }
#define ON_ERROR2STATUS(Err, ...)    if ((Err) != VSC_ERR_NONE) {ERR_REPORT((Err), __VA_ARGS__); status = vscERR_CastErrCode2GcStatus(Err); goto OnError; }
#define VERIFY_OK(Err)               if ((Err) != VSC_ERR_NONE) {ERR_REPORT((Err), "status verify failed."); }

gceSTATUS vscERR_CastErrCode2GcStatus(VSC_ErrCode errCode);

END_EXTERN_C()

#endif /* __gc_vsc_err_h_ */

