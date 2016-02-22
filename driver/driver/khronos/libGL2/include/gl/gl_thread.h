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


#ifndef __gl_thread_h_
#define __gl_thread_h_

#ifdef _LINUX_

#if defined(PTHREADS)

#include <pthread.h> /* POSIX threads headers */
#include <errno.h>

typedef struct {
    pthread_key_t  key;
    GLint initMagic;
} _glthread_TSD;

typedef pthread_t _glthread_Thread;

typedef pthread_mutex_t _glthread_Mutex;

#define _glthread_DECLARE_MUTEX(name) \
    _glthread_Mutex name = PTHREAD_MUTEX_INITIALIZER

#define _glthread_INIT_MUTEX(name) \
    pthread_mutex_init(&(name), NULL)

#define _glthread_DESTROY_MUTEX(name) \
    pthread_mutex_destroy(&(name))

#define _glthread_LOCK_MUTEX(name) \
    (GLvoid) pthread_mutex_lock(&(name))

#define _glthread_UNLOCK_MUTEX(name) \
    (GLvoid) pthread_mutex_unlock(&(name))

#else
#define PTHREAD_MUTEX_INITIALIZER 0
typedef GLint _glthread_Mutex; /* Fake _glthread_Mutex just to pass compiling */
#define _glthread_LOCK_MUTEX(name)
#define _glthread_UNLOCK_MUTEX(name)
#endif

extern GLvoid _glapi_check_multithread(GLvoid);
extern GLvoid _glapi_set_context(GLvoid *context);
extern GLvoid *_glapi_get_context(GLvoid);
extern GLvoid _glapi_set_dispatch(GLvoid *dispatch);
extern GLvoid *_glapi_get_dispatch(GLvoid);
extern unsigned long _glthread_GetID(GLvoid);

typedef struct __GLlockRec {
    GLvoid * lock;
    GLint usageCount;
} __GLlock;

#else /* _LINUX_ */

#ifdef _WIN64
#define WNT_TEB_PTR                     0x1248
#define WNT_TEB_TLS_OFFSET              0x1480
#define WNT_TLS_INDEX_TO_OFFSET(i)      ((i)*sizeof(PVOID)+WNT_TEB_TLS_OFFSET)
#else
#define WNT_TEB_PTR                     0xBF0
#define WNT_TEB_TLS_OFFSET              0xE10
#define WNT_TLS_INDEX_TO_OFFSET(i)      ((i)*sizeof(DWORD)+WNT_TEB_TLS_OFFSET)
#endif

extern unsigned long __wglTLSCXIndex;
extern unsigned long __wglTLSCXOffset;
extern unsigned long __wglTLSIndex;
extern unsigned long __wglTLSOffset;

#if defined(_M_AMD64)

#define __GL_GET_TLSCX_VALUE()            \
    __readgsqword(WNT_TEB_PTR)
#define __GL_SET_TLSCX_VALUE(val)         \
    __writegsqword(WNT_TEB_PTR, (GLuint64)val)
#define __GL_GET_TLS_VALUE()              \
    __readgsqword(WNT_TEB_PTR + __wglTLSOffset)
#define __GL_SET_TLS_VALUE(val)           \
    __writegsqword((WNT_TEB_PTR + __wglTLSOffset), (GLuint64)val)

#elif defined (_X86_)

#define __GL_GET_TLSCX_VALUE()                    \
        __asm mov eax, DWORD PTR fs:[WNT_TEB_PTR]
#define __GL_SET_TLSCX_VALUE(val)                 \
        __asm mov ecx, DWORD PTR (val)            \
        __asm mov DWORD PTR fs:[WNT_TEB_PTR], ecx


#define __GL_GET_TLS_VALUE()                      \
        __asm mov eax, DWORD PTR fs:[WNT_TEB_PTR] \
        __asm mov esi, DWORD PTR __wglTLSOffset   \
        __asm mov eax, DWORD PTR [esi+eax]
#define __GL_SET_TLS_VALUE(val)                   \
        __asm mov eax, DWORD PTR fs:[WNT_TEB_PTR] \
        __asm mov esi, DWORD PTR __wglTLSOffset   \
        __asm mov ecx, DWORD PTR (val)            \
        __asm mov DWORD PTR [esi+eax], ecx
#endif /* defined(_M_AMD64) */

#if defined(_WIN32)
#pragma warning (4:4035)        /* No return value */
#endif
static __inline GLvoid *
__glGetTLSCXValue(GLvoid)
{
#ifdef _WIN64
    return (GLvoid *)__GL_GET_TLSCX_VALUE();
#else
    __GL_GET_TLSCX_VALUE();
#endif
}
static __inline GLvoid *
__glGetTLSValue(GLvoid)
{
#ifdef _WIN64
    return (GLvoid *)__GL_GET_TLS_VALUE();
#else
    __GL_GET_TLS_VALUE();
#endif
}
#if defined(_WIN32)
#pragma warning (3:4035)
#endif

static __inline GLvoid
__glSetTLSCXValue(GLvoid *val)
{
    __GL_SET_TLSCX_VALUE(val);
}
static __inline GLvoid
__glSetTLSValue(GLvoid *val)
{
    __GL_SET_TLS_VALUE(val);
}

typedef struct __GLlockRec {
    CRITICAL_SECTION lock;
    GLint usageCount;
} __GLlock;

#endif /* _LINUX_ */

/*
** __GL_MAXIMUM_THREAD_NUMBER must be 2^n so that a thread hash id can be generated by a simple mask.
*/
#define __GL_MAXIMUM_THREAD_NUMBER      256
#define __GL_THREAD_HASH_ID(threadId)   (threadId & (__GL_MAXIMUM_THREAD_NUMBER - 1))

/*before a context is made current, set the thread hash ID to be invalid */
#define __GL_INVALID_THREAD_HASH_ID ((GLuint)-1)

typedef struct __GLthreadHashTableRec
{
    GLuint threadId;
    GLuint threadHashId;
#ifdef _LINUX_
    GLvoid  *thrArea;
#endif
} __GLthreadHashTable;


#endif /* __gl_thread_h_ */
