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


#ifndef __gc_hal_kernel_debug_h_
#define __gc_hal_kernel_debug_h_

#include <gc_hal_kernel_qnx.h>
#include <pthread.h>
#include <stdarg.h>

#if gcdUSE_FAST_MEM_COPY
#include <fastmemcpy.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************\
****************************** OS-dependent Macros *****************************
\******************************************************************************/

typedef va_list gctARGUMENTS;

#define gcmkARGUMENTS_START(Arguments, Pointer) \
    va_start(Arguments, Pointer)

#define gcmkARGUMENTS_END(Arguments) \
    va_end(Arguments)

#define gcmkARGUMENTS_ARG(Arguments, Type) \
    va_arg(Arguments, Type)

#define gcmkDECLARE_LOCK(__lockHandle__) \
    static pthread_mutex_t __lockHandle__ = PTHREAD_MUTEX_INITIALIZER

#define gcmkLOCKSECTION(__lockHandle__) \
    pthread_mutex_lock(&__lockHandle__)

#define gcmkUNLOCKSECTION(__lockHandle__) \
    pthread_mutex_unlock(&__lockHandle__)

#define gcmkGETPROCESSID() \
    getpid()

#define gcmkGETTHREADID() \
    (gctUINT32)pthread_self()

#define gcmkOUTPUT_STRING(String) \
    printf(String); \
    fflush(stdout)

#define gcmkSPRINTF(Destination, Size, Message, Value) \
    snprintf(Destination, Size, Message, Value)

#define gcmkSPRINTF2(Destination, Size, Message, Value1, Value2) \
    snprintf(Destination, Size, Message, Value1, Value2)

#define gcmkSPRINTF3(Destination, Size, Message, Value1, Value2, Value3) \
    snprintf(Destination, Size, Message, Value1, Value2, Value3)

#define gcmkVSPRINTF(Destination, Size, Message, Arguments) \
    vsnprintf(Destination, Size, Message, *((va_list *)Arguments))

#define gcmkSTRCATSAFE(Destination, Size, String) \
    strncat(Destination, String, (Size) - 1)

#if gcdUSE_FAST_MEM_COPY
#define gcmkMEMCPY(Destination, Source, Size) \
    fast_mem_cpy(Destination, Source, Size);
#else
#define gcmkMEMCPY(Destination, Source, Size) \
    memcpy(Destination, Source, Size)
#endif

#define gcmkSTRLEN(String) \
    strlen(String)

/* If not zero, forces data alignment in the variable argument list
   by its individual size. */
#define gcdALIGNBYSIZE      1

#ifdef __cplusplus
}
#endif

#endif /* __gc_hal_kernel_debug_h_ */
