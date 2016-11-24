/**
 * \file xf86drm.h
 * OS-independent header for DRM user-level library interface.
 *
 * \author Rickard E. (Rik) Faith <faith@valinux.com>
 */

/**************************************************************************

Copyright 2000, 2001 VA Linux Systems Inc., Fremont, California.

All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

#ifndef _DRMGL_H_
#define _DRMGL_H_

#include <drm.h>
#include <xf86drm.h>

#if defined(__GNUC__)

    #if defined(__arm__) || defined(__arm64) || defined(__arm64__)

    #if defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || \
        defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__)

        #define DRMGL_CAS(lock,old,new,__ret)                 \
        do {                                            \
        __asm__ __volatile__ (                          \
        "1: ldrex %0, [%1]\n"                           \
        "   teq %0, %2\n"                               \
        "   strexeq %0, %3, [%1]\n"                     \
        : "=&r" (__ret)                                 \
        : "r" (lock), "r" (old), "r" (new)              \
        :  "cc","memory");                                        \
        } while (0)

    #endif

    #if defined(__ARM64_ARCH_8__)

    #endif

    #elif defined(__sparc__)

        #define DRMGL_CAS(lock,old,new,__ret)                 \
        do {    register unsigned int __old __asm("o0");                     \
        register unsigned int __new __asm("o1");                     \
        register volatile unsigned int *__lock __asm("o2");                     \
        __old = old;                     \
        __new = new;                     \
        __lock = (volatile unsigned int *)lock;                     \
        __asm__ __volatile__(                     \
        /*"cas [%2], %3, %0"*/                     \
        ".word 0xd3e29008\n\t"                     \
        /*"membar #StoreStore | #StoreLoad"*/                     \
        ".word 0x8143e00a"                     \
        : "=&r" (__new)                     \
        : "0" (__new),                     \
         "r" (__lock),                     \
         "r" (__old)                     \
         : "memory");                     \
         __ret = (__new != __old);                       \
         } while(0)


    #elif defined(__e2k__)

    #define DRMGL_CAS(lock, old, new, __ret)                         \
    do {                                                           \
           __ret = !__sync_bool_compare_and_swap(                  \
                           &__drm_dummy_lock(lock),                \
                           old, new);                              \
    } while (0)

    #endif

#endif


#ifndef DRMGL_CAS

    #define DRMGL_CAS(lock, old, new, __ret)                         \
    do {                                                           \
           __ret = !__sync_bool_compare_and_swap(                  \
                           &__drm_dummy_lock(lock),                \
                           old, new);                              \
    } while (0)

#endif

#if defined(__alpha__) || defined(__powerpc__)
#define DRMGL_CAS_RESULT(_result)                int _result
#else
#define DRMGL_CAS_RESULT(_result)                char _result
#endif

#define DRMGL_LIGHT_LOCK(fd,lock,context)                                \
        do {                                                           \
                DRMGL_CAS_RESULT(__ret);                                 \
                DRMGL_CAS(lock,context,DRM_LOCK_HELD|context,__ret);     \
                if (__ret) drmGetLock(fd,context,0);                   \
        } while(0)

                                /* This one counts fast locks -- for
                                   benchmarking only. */
#define DRMGL_LIGHT_LOCK_COUNT(fd,lock,context,count)                    \
        do {                                                           \
                DRMGL_CAS_RESULT(__ret);                                 \
                DRMGL_CAS(lock,context,DRM_LOCK_HELD|context,__ret);     \
                if (__ret) drmGetLock(fd,context,0);                   \
                else       ++count;                                    \
        } while(0)

#define DRMGL_LOCK(fd,lock,context,flags)                                \
        do {                                                           \
                if (flags) drmGetLock(fd,context,flags);               \
                else       DRMGL_LIGHT_LOCK(fd,lock,context);            \
        } while(0)

#define DRMGL_UNLOCK(fd,lock,context)                                    \
        do {                                                           \
                DRMGL_CAS_RESULT(__ret);                                 \
                DRMGL_CAS(lock,DRM_LOCK_HELD|context,context,__ret);     \
                if (__ret) drmUnlock(fd,context);                      \
        } while(0)

                                /* Simple spin locks */
#define DRMGL_SPINLOCK(spin,val)                                         \
        do {                                                           \
            DRMGL_CAS_RESULT(__ret);                                     \
            do {                                                       \
                DRMGL_CAS(spin,0,val,__ret);                             \
                if (__ret) while ((spin)->lock);                       \
            } while (__ret);                                           \
        } while(0)

#define DRMGL_SPINLOCK_TAKE(spin,val)                                    \
        do {                                                           \
            DRMGL_CAS_RESULT(__ret);                                     \
            int  cur;                                                  \
            do {                                                       \
                cur = (*spin).lock;                                    \
                DRMGL_CAS(spin,cur,val,__ret);                           \
            } while (__ret);                                           \
        } while(0)

#define DRMGL_SPINLOCK_COUNT(spin,val,count,__ret)                       \
        do {                                                           \
            int  __i;                                                  \
            __ret = 1;                                                 \
            for (__i = 0; __ret && __i < count; __i++) {               \
                DRMGL_CAS(spin,0,val,__ret);                             \
                if (__ret) for (;__i < count && (spin)->lock; __i++);  \
            }                                                          \
        } while(0)

#define DRMGL_SPINUNLOCK(spin,val)                                       \
        do {                                                           \
            DRMGL_CAS_RESULT(__ret);                                     \
            if ((*spin).lock == val) { /* else server stole lock */    \
                do {                                                   \
                    DRMGL_CAS(spin,val,0,__ret);                         \
                } while (__ret);                                       \
            }                                                          \
        } while(0)



#endif
