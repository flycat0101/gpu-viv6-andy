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


#ifndef __GRALLOC_UTIL_H_
#define __GRALLOC_UTIL_H_

#include <cutils/log.h>
#include <cutils/properties.h>

#ifndef likely
#  define likely(x)       __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#  define unlikely(x)     __builtin_expect((x), 0)
#endif

#define TRACE_IN    0
#define TRACE_OUT   1
#define TRACE_OTHER 2

#define GRALLOC_TRACE 1

#ifdef GRALLOC_TRACE

extern uint32_t __gralloc_traceEnabled;
extern __thread int __gralloc_traceDepth;

#define gralloc_trace_init() \
    do { \
        char property[PROPERTY_VALUE_MAX]; \
        if (property_get("gralloc.trace", property, NULL) > 0) { \
            __gralloc_traceEnabled = property[0] != '0'; \
        } \
    } while (0)

/* Trace with indents. */
#define gralloc_trace(inout, format, ...) \
    do { \
        const char *___s; \
        if (likely(!__gralloc_traceEnabled)) \
            break; \
        if (inout == 0) { \
            ___s = "+ "; \
        } else if (inout == 1) { \
            ___s = "- "; \
            __gralloc_traceDepth--; \
        } else { \
            ___s = "  "; \
        } \
        ALOGD("%*s%s%s#%d: " format, __gralloc_traceDepth * 2, "", \
             ___s, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        if (inout == 0) { \
            __gralloc_traceDepth++; \
        } \
    } while (0)

/* Trace error with indents, log even trace disabled. */
#define gralloc_trace_error(inout, format, ...) \
    do { \
        const char *___s; \
        if (likely(!__gralloc_traceEnabled)) { \
            ALOGE("%s#%d: " format, \
                __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            break; \
        } \
        if (inout == 0) { \
            ___s = "+ "; \
        } else if (inout == 1) { \
            ___s = "- "; \
            __gralloc_traceDepth--; \
        } else { \
            ___s = "  "; \
        } \
        ALOGE("%*s%s%s#%d: " format, __gralloc_traceDepth * 2, "", \
             ___s, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        if (inout == 0) { \
            __gralloc_traceDepth++; \
        } \
    } while (0)

/* Trace without indents. */
#define gralloc_trace_string(format, ...) \
    do { \
        if (likely(!__gralloc_traceEnabled)) \
            break; \
        ALOGD("%*s  " format, __gralloc_traceDepth * 2, "", ##__VA_ARGS__); \
    } while (0)

#else

#define gralloc_trace_init() do {} while (0)

#define gralloc_trace(...)          do {} while (0)

#define gralloc_trace_error(inout, format, ...) \
    do { \
        ALOGE("%s#%d: " format, \
            __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#define gralloc_trace_string(...)   do {} while (0)

#endif

#endif /* __GRALLOC_UTIL_H_ */
