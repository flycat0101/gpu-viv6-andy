
/*
 * Mesa 3-D graphics library
 * Version:  4.1
 *
 * Copyright (C) 1999-2002  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * POSIX threads. This should be your choice in the Unix world
 * whenever possible.  When building with POSIX threads, be sure
 * to enable any compiler flags which will cause the MT-safe
 * libc (if one exists) to be used when linking, as well as any
 * header macros for MT-safe errno, etc.  For Solaris, this is the -mt
 * compiler flag.  On Solaris with gcc, use -D_REENTRANT to enable
 * proper compiling for MT-safe libc etc.
 */

#include "glxclient.h"
#include "gl/gl_thread.h"
#include <stdio.h>

#if defined(PTHREADS)
/*
 * Error messages
 */
#define INIT_TSD_ERROR "_glthread_: failed to allocate key for thread specific data"
#define GET_TSD_ERROR "_glthread_: failed to get thread specific data"
#define SET_TSD_ERROR "_glthread_: thread failed to set thread specific data"


#define INIT_MAGIC 0xff8adc98



/*
 * POSIX Threads -- The best way to go if your platform supports them.
 *                  Solaris >= 2.5 have POSIX threads, IRIX >= 6.4 reportedly
 *                  has them, and many of the free Unixes now have them.
 *                  Be sure to use appropriate -mt or -D_REENTRANT type
 *                  compile flags when building.
 */

unsigned long
_glthread_GetID(GLvoid)
{
    return (unsigned long) pthread_self();
}


GLvoid
_glthread_InitTSD(_glthread_TSD *tsd)
{
    if (pthread_key_create(&tsd->key, NULL/*free*/) != 0) {
        perror(INIT_TSD_ERROR);
        exit(-1);
    }
    tsd->initMagic = INIT_MAGIC;
}


GLvoid *
_glthread_GetTSD(_glthread_TSD *tsd)
{
    if (tsd->initMagic != (int) INIT_MAGIC) {
        _glthread_InitTSD(tsd);
    }
    return pthread_getspecific(tsd->key);
}


GLvoid
_glthread_SetTSD(_glthread_TSD *tsd, GLvoid *ptr)
{
    if (tsd->initMagic != (int) INIT_MAGIC) {
        _glthread_InitTSD(tsd);
    }
    if (pthread_setspecific(tsd->key, ptr) != 0) {
        perror(SET_TSD_ERROR);
        exit(-1);
    }
}


/**
 * \name Current dispatch and current context control variables
 *
 * Depending on whether or not multithreading is support, and the type of
 * support available, several variables are used to store the current context
 * pointer and the current dispatch table pointer.  In the non-threaded case,
 * the variables \c _glapi_Dispatch and \c _glapi_Context are used for this
 * purpose.
 *
 * In the "normal" threaded case, the variables \c _glapi_Dispatch and
 * \c _glapi_Context will be \c NULL if an application is detected as being
 * multithreaded.  Single-threaded applications will use \c _glapi_Dispatch
 * and \c _glapi_Context just like the case without any threading support.
 * When \c _glapi_Dispatch and \c _glapi_Context are \c NULL, the thread state
 * data \c _gl_DispatchTSD and \c _gl_ContextTSD are used.  Drivers and the
 * static dispatch functions access these variables via \c _glapi_get_dispatch
 * and \c _glapi_get_context.
 *
 * There is a race condition in setting \c _glapi_Dispatch to \c NULL.  It is
 * possible for the original thread to be setting it at the same instant a new
 * thread, perhaps running on a different processor, is clearing it.  Because
 * of that, \c ThreadSafe, which can only ever be changed to \c GL_TRUE, is
 * used to determine whether or not the application is multithreaded.
 *
 * In the TLS case, the variables \c _glapi_Dispatch and \c _glapi_Context are
 * hardcoded to \c NULL.  Instead the TLS variables \c _glapi_tls_Dispatch and
 * \c _glapi_tls_Context are used.  Having \c _glapi_Dispatch and
 * \c _glapi_Context be hardcoded to \c NULL maintains binary compatability
 * between TLS enabled loaders and non-TLS DRI drivers.
 */

static GLboolean ThreadSafe = GL_FALSE;  /**< In thread-safe mode? */

_glthread_TSD _gl_DispatchTSD;           /**< Per-thread dispatch pointer */
static _glthread_TSD _gl_ContextTSD;     /**< Per-thread context pointer */

#endif /* PTHREADS */

/*
** This is the global GL API dispatch pointer.
** It points to glcore's __glVVT_EntryDispatchFuncTable.dispatch table for direct rendering,
** or it points to GLX's __glIndirectDispatchTab table for indirect rendering.
** All GL contexts will share this global API dispatch table in single thread mode.
*/
__GLesDispatchTable *_glapi_Dispatch = NULL;
GLvoid *_glapi_Context = NULL;

extern __GLesDispatchTable *__glNopDispatchTab;
extern __GLcontext *__glxNopContext;

/**
 * Set the current context pointer for this thread.
 * The context pointer is an opaque type which should be cast to
 * GLvoid from the real context pointer type.
 */
GLvoid
_glapi_set_context(GLvoid *context)
{
#if defined(PTHREADS)
    _glthread_SetTSD(&_gl_ContextTSD, context);
    _glapi_Context = (ThreadSafe) ? NULL : context;
#else
    _glapi_Context = context;
#endif
}

/**
 * Get the current context pointer for this thread.
 * The context pointer is an opaque type which should be cast from
 * GLvoid to the real context pointer type.
 */
GLvoid *
_glapi_get_context(GLvoid)
{
#if defined(PTHREADS)
    if (ThreadSafe) {
        return _glthread_GetTSD(&_gl_ContextTSD);
    }
    else {
        return _glapi_Context;
    }
#else
    return _glapi_Context;
#endif
}


/**
 * Set the global or per-thread dispatch table pointer.
 */
GLvoid
_glapi_set_dispatch(GLvoid *dispatch)
{
#if defined(PTHREADS)
    _glthread_SetTSD(&_gl_DispatchTSD, (GLvoid *) dispatch);
    _glapi_Dispatch = (ThreadSafe) ? NULL : dispatch;
#else
    _glapi_Dispatch = dispatch;
#endif
}

/**
 * Return pointer to current dispatch table for calling thread.
 */
GLvoid *
_glapi_get_dispatch(GLvoid)
{
    __GLesDispatchTable *api;

#if defined(PTHREADS)
    api = (ThreadSafe) ?
        (__GLesDispatchTable *) _glthread_GetTSD(&_gl_DispatchTSD) : _glapi_Dispatch;
#else
    api = _glapi_Dispatch;
#endif

    return (GLvoid *)api;
}


/**
 * We should call this periodically from a function such as glXMakeCurrent
 * in order to test if multiple threads are being used.
 */
GLvoid
_glapi_check_multithread(GLvoid)
{
#if defined(PTHREADS)
    if (!ThreadSafe) {
        static unsigned long knownID;
        static GLboolean firstCall = GL_TRUE;
        if (firstCall) {
            knownID = _glthread_GetID();
            firstCall = GL_FALSE;
        }
        else if (knownID != _glthread_GetID()) {
            ThreadSafe = GL_TRUE;
            _glapi_set_context((GLvoid *)__glxNopContext);
        }
    }
#endif
}
