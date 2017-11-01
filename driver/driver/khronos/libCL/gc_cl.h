/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_cl_h_
#define __gc_cl_h_

#include <CL/cl_egl.h>

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************\
****************************** Naming Convention  ******************************
\******************************************************************************/

/*
    All names start with cl prefix appended with one character that signifies
    the type of the object:

    - clm: macro produced by 'define' statement;
    - cld: compiler definition produced by 'define' statement;
    - cle: enumerated type;
    - clv: value defined by an enumeration;
    - cls: structure;
    - clt: type defined with 'typedef' statement;
    - clf: internal API function;
    - clg: global variable definition;

    Pointers to structures are defined by adding _PTR postfix to the name.

    For types and names of object: gc<letter>NAME.

    For methods: gco<NAME>_<Function>.

    Variables come in using C++ styling: upper case (ParameterBecauseItHasToBe)
    Local variables start with lower case and use C++ convention (no _). (localVariable)
*/


/******************************************************************************\
|******************************* Type Definitions *****************************|
\******************************************************************************/

/* Redefine OpenCL types for naming convension. */
typedef cl_platform_id          clsPlatformId_PTR;
typedef cl_device_id            clsDeviceId_PTR;
typedef cl_context              clsContext_PTR;
typedef cl_command_queue        clsCommandQueue_PTR;
typedef cl_mem                  clsMem_PTR;
typedef cl_program              clsProgram_PTR;
typedef cl_kernel               clsKernel_PTR;
typedef cl_event                clsEvent_PTR;
typedef cl_sampler              clsSampler_PTR;

typedef cl_image_format         clsImageFormat;
typedef cl_buffer_region        clsBufferRegion;

typedef CLIicdDispatchTable *   clsIcdDispatch_PTR;

/* Internal data structures. */
typedef struct _cl_command *    clsCommand_PTR;
typedef struct _cl_argument *   clsArgument_PTR;
typedef struct _cl_event_callback * clsEventCallback_PTR;
typedef struct _cl_image_header * clsImageHeader_PTR;

/* Mismatched types between OpenCL and internal gct types. */

/* Data types for OpenCL driver. */
typedef enum _cleOBJECT_TYPE
{
    clvOBJECT_UNKNOWN,
    clvOBJECT_PLATFORM,
    clvOBJECT_DEVICE,
    clvOBJECT_CONTEXT,
    clvOBJECT_COMMAND_QUEUE,
    clvOBJECT_MEM,
    clvOBJECT_PROGRAM,
    clvOBJECT_KERNEL,
    clvOBJECT_EVENT,
    clvOBJECT_SAMPLER,
    clvOBJECT_COMMAND,
    clvOBJECT_ARGUMENT,
}
cleOBJECT_TYPE;

typedef struct _cl_object
{
    clsIcdDispatch_PTR      dispatch;
    cleOBJECT_TYPE          objectType;
    gctUINT                 id;
    gcsATOM_PTR             referenceCount;
}
clsObject;

/* Base object for all cl objects. */
typedef struct _cl_object *     clsObject_PTR;


#define cldTUNING               1

/******************************************************************************\
************************** Generic Macro Definitions ***************************
\******************************************************************************/

#define __EMBEDDED_PROFILE__

/* Unique device vendor identifier, CCFOUR version of VIV */
#define clvDEVICE_VENDOR_ID     ((gctUINT32) 0x00564956)

#define _GC_OBJ_ZONE            gcvZONE_API_CL

#define clmIS_ERROR(status)     (status <  CL_SUCCESS)
#define clmNO_ERROR(status)     (status >= CL_SUCCESS)
#define clmIS_SUCCESS(status)   (status == CL_SUCCESS)

#define clmCHECK_ERROR(Exp, ErrorCode) \
    do \
    { \
        if (Exp) \
        { \
            /* gcmBREAK(); */ \
            status = ErrorCode; \
            goto OnError; \
        } \
    } \
    while (gcvFALSE)

#define clmRETURN_ERROR(ErrorCode) \
    do \
    { \
        /* gcmBREAK(); */ \
        status = ErrorCode; \
        goto OnError; \
    } \
    while (gcvFALSE)

#define clmONERROR(Func, ErrorCode) \
    do \
    { \
        status = Func; \
        if (gcmIS_ERROR(status)) \
        { \
            gcmTRACE(gcvLEVEL_ERROR, \
                "clmONERROR: status=%d @ %s(%d) in " __FILE__, \
                status, __FUNCTION__, __LINE__); \
            /* gcmBREAK(); */ \
            status = ErrorCode; \
            goto OnError; \
        } \
    } \
    while (gcvFALSE)

#define clfONERROR(Func) \
    do \
    { \
        status = Func; \
        if (clmIS_ERROR(status)) \
        { \
            gcmTRACE(gcvLEVEL_ERROR, \
                "clfONERROR: status=%d @ %s(%d) in " __FILE__, \
                status, __FUNCTION__, __LINE__); \
            goto OnError; \
        } \
    } \
    while (gcvFALSE)

#define clmASSERT(Exp, ErrorCode) \
    do \
    { \
        if (!(Exp)) \
        { \
            gcmTRACE(gcvLEVEL_ERROR, \
                     "gcmASSERT at %s(%d) in " __FILE__, \
                     __FUNCTION__, __LINE__); \
            gcmTRACE(gcvLEVEL_ERROR, \
                     "(%s)", #Exp); \
            gcmBREAK(); \
            status = ErrorCode; \
            goto OnError; \
        } \
    } \
    while (gcvFALSE)

/* Milliseconds to sleep while waiting for events/commands */
#define CL_DELAY 5

/******************************************************************************\
****************************** Global Variables *******************************
\******************************************************************************/

/* Global ID for all objects. */
extern gcsATOM_PTR clgGlobalId;
/* Helper string for query functions to return empty null-terminated string */
extern const gctSTRING clgEmptyStr;

#if defined(_WINDOWS)
#define __CL_INLINE static __forceinline
#else
#define __CL_INLINE static __inline
#endif

#ifdef __cplusplus
}
#endif

#endif  /* __gc_cl_h_ */
