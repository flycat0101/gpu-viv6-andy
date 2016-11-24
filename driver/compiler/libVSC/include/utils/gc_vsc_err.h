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
    VSC_ERR_OUT_OF_BOUNDS         =   9,
    VSC_ERR_OUT_OF_SAMPLER        =   10,

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
    VSC_ERR_LOCATION_MISMATCH     =   1011,
    VSC_ERR_LOCATION_ALIASED      =   1012,
    VSC_ERR_UNSAT_LIB_SYMBOL      =   1013,

    /* misc error */
    VSC_ERR_INVALID_TYPE          =   4000,
} VSC_ErrCode;

void vscERR_ReportError(const char* file, gctUINT line, VSC_ErrCode status, const char* format, ...);
void vscERR_ReportWarning(const char* file, gctUINT line, VSC_ErrCode status, const char* format, ...);

#if !defined(_DEBUG)
#define VSC_ERR_DUMP     0
#else
#define VSC_ERR_DUMP     1
#endif
#define VSC_WARNING_DUMP 0

#if VSC_ERR_DUMP
#define ERR_REPORT(status, ...)        vscERR_ReportError(__FILE__, __LINE__, (status), __VA_ARGS__)
#define ERR_REPORT0(status)            vscERR_ReportError(__FILE__, __LINE__, (status), "")
#else
#define ERR_REPORT(status, ...)
#define ERR_REPORT0(status)
#endif

#if VSC_WARNING_DUMP
#define WARNING_REPORT(status, ...)    vscERR_ReportWarning(__FILE__, __LINE__, (status), __VA_ARGS__)
#define WARNING_REPORT0(status)        vscERR_ReportWarning(__FILE__, __LINE__, (status), "")
#else
#define WARNING_REPORT(status, ...)
#define WARNING_REPORT0(status)
#endif

#define CHECK_ERROR(Err, ...)                                 \
    do {                                                      \
           VSC_ErrCode _errCode = (Err);                      \
           if (_errCode != VSC_ERR_NONE)                      \
           {                                                  \
               ERR_REPORT(_errCode, __VA_ARGS__);             \
               return _errCode;                               \
           }                                                  \
    } while (0)

#define ON_ERROR(Err, ...)                                    \
    do {                                                      \
           VSC_ErrCode _errCode = (Err);                      \
           if (_errCode != VSC_ERR_NONE)                      \
           {                                                  \
               ERR_REPORT(_errCode, __VA_ARGS__);             \
               goto OnError;                                  \
           }                                                  \
    } while (0)

#define CHECK_ERROR2STATUS(Err, ...)                          \
    do {                                                      \
           VSC_ErrCode _errCode = (Err);                      \
           if (_errCode != VSC_ERR_NONE)                      \
           {                                                  \
               ERR_REPORT(_errCode, __VA_ARGS__);             \
               return vscERR_CastErrCode2GcStatus(Err);       \
           }                                                  \
    } while (0)

#define ON_ERROR2STATUS(Err, ...)                             \
    do {                                                      \
           VSC_ErrCode _errCode = (Err);                      \
           if (_errCode != VSC_ERR_NONE)                      \
           {                                                  \
               ERR_REPORT(_errCode, __VA_ARGS__);             \
               status =  vscERR_CastErrCode2GcStatus(Err);    \
               goto OnError;                                  \
           }                                                  \
    } while (0)

#define CHECK_ERROR0(Err)                                     \
    do {                                                      \
           VSC_ErrCode _errCode = (Err);                      \
           if (_errCode != VSC_ERR_NONE)                      \
           {                                                  \
               ERR_REPORT0(_errCode);                         \
               return _errCode;                               \
           }                                                  \
    } while (0)

#define ON_ERROR0(Err)                                        \
    do {                                                      \
           errCode = (Err);                                   \
           if (errCode != VSC_ERR_NONE)                       \
           {                                                  \
               ERR_REPORT0(errCode);                          \
               goto OnError;                                  \
           }                                                  \
    } while (0)

#define CHECK_ERROR2STATUS0(Err)                              \
    do {                                                      \
           VSC_ErrCode _errCode = (Err);                      \
           if (_errCode != VSC_ERR_NONE)                      \
           {                                                  \
               ERR_REPORT0(_errCode);                         \
               return vscERR_CastErrCode2GcStatus(Err);       \
           }                                                  \
    } while (0)

#define ON_ERROR2STATUS0(Err)                                 \
    do {                                                      \
           VSC_ErrCode _errCode = (Err);                      \
           if (_errCode != VSC_ERR_NONE)                      \
           {                                                  \
               ERR_REPORT0(_errCode);                         \
               status =  vscERR_CastErrCode2GcStatus(Err);    \
               goto OnError;                                  \
           }                                                  \
    } while (0)

#define VERIFY_OK(Err)                                        \
    do {                                                      \
        VSC_ErrCode _errCode = (Err);                         \
        if (_errCode != VSC_ERR_NONE)                         \
        {                                                     \
            ERR_REPORT(_errCode, "status verify failed.");   \
        }                                                     \
    } while (0)

gceSTATUS vscERR_CastErrCode2GcStatus(VSC_ErrCode _errCode);

END_EXTERN_C()

#endif /* __gc_vsc_err_h_ */

