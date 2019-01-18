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


#include "gc_hwc.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>

#include <utils/Timers.h>

extern "C" int clock_nanosleep(clockid_t clock_id, int flags,
                           const struct timespec *request,
                           struct timespec *remain);

/*
 * NOTICE & TODO (Soc-vendor):
 * This is only reference implementation for hwc vsync.
 * And it is a fake vsync, which does not wait for real framebuffer vsync.
 *
 * VSYNC SHOULD BE IMPLEMENTED BY VENDOR!
 */

/* HWC vsync thread data. */
struct vsyncContext
{
    /* thread id. */
    pthread_t       thread;

    /* Mutex and condition for synchronization. */
    pthread_mutex_t mutex;
    pthread_cond_t  cond;

    /* Thread exit flag. */
    volatile int    done;

    /* Vsync enabling flag. */
    volatile int    enabled;
};


static void *
vsyncThread(
    void * param
    );

/*
 * Vsync thread setup.
 * This function is called only once when hwcomposer setup.
 */
int
hwcVsyncSetup(
    IN hwcContext * Context
    )
{
    int err;
    pthread_attr_t attr;
    vsyncContext * vsync = (vsyncContext *) malloc(sizeof (vsyncContext));

    pthread_mutex_init(&vsync->mutex, NULL);
    pthread_cond_init(&vsync->cond, NULL);

    /* Thread should be running. */
    vsync->done     = 0;

    /* Not enabled. */
    vsync->enabled  = 0;

    /* Save vsync context to hwc context. */
    Context->vsync = vsync;

    /* Initialize attr. */
    pthread_attr_init(&attr);


    /* Create thread to handle vsync events. */
    err = pthread_create(&vsync->thread, &attr, vsyncThread, Context);

    if (err < 0)
    {
        LOGE("%s: Create vsync thread failed", __FUNCTION__);
        free(vsync);
        Context->vsync = 0;
        return -EINVAL;
    }

    return 0;
}

int
hwcVsyncPeriod(
    IN hwcContext * Context
    )
{
    /* TODO (Soc-vendor): vsync period in nano seconds. */
    return (1000000000 / 60);
}

int
hwcVsyncStop(
    IN hwcContext * Context
    )
{
    /* Retrieve vsync context. */
    vsyncContext * vsync = (vsyncContext *) Context->vsync;

    if (vsync == NULL)
    {
        /* Nothing to do. */
        return 0;
    }

    /* Set exit flag. */
    vsync->done = 1;

    /* Do not block vsync thread. */
    vsync->enabled = 1;

    /* Wake up vsync thread. */
    pthread_cond_broadcast(&vsync->cond);

    return 0;
}

int
hwcVsyncControl(
    IN hwcContext * Context,
    IN int Enable
    )
{
    /* Retrieve vsync context. */
    vsyncContext * vsync = (vsyncContext *) Context->vsync;

    if (vsync == NULL)
    {
        /* Invalid operation. */
        return -EINVAL;
    }

    /* Set enabling flag. */
    vsync->enabled = Enable;

    /* Wake up vsync thread. */
    pthread_cond_broadcast(&vsync->cond);

    return 0;
}

void *
vsyncThread(
    void * param
    )
{
    hwcContext * context = (hwcContext *) param;
    vsyncContext * vsync = (vsyncContext *) context->vsync;

    const nsecs_t period = (1000000000 / 60);

    /* TODO (Soc-vendor): Implement real vsync. */
    nsecs_t nextFakeVsync = 0;

    for (;;)
    {
        /* Lock. */
        pthread_mutex_lock(&vsync->mutex);

        while (!vsync->enabled)
        {
            /* Wait for condition. */
            pthread_cond_wait(&vsync->cond, &vsync->mutex);
        }

        /* Release lock. */
        pthread_mutex_unlock(&vsync->mutex);

        if (vsync->done)
        {
            /* Check thread exiting flag. */
            break;
        }

        /*--------------- TODO (Soc-vendor): Implement real vsync. ---------------*/
        nsecs_t now       = systemTime(CLOCK_MONOTONIC);
        nsecs_t nextVsync = nextFakeVsync;
        nsecs_t sleep     = nextVsync - now;

        if (sleep < 0)
        {
            /* We missed. find where the next vsync should be */
            sleep = (period - ((now - nextVsync) % period));
            nextVsync = now + sleep;
        }

        nextFakeVsync = nextVsync + period;

        struct timespec spec;
        spec.tv_sec  = nextVsync / 1000000000;
        spec.tv_nsec = nextVsync % 1000000000;

        int err;
        do
        {

            err = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &spec, NULL);
        }
        while (err<0 && errno == EINTR);
        /*--------------- End implement real vsync. ---------------*/

        /* Vsync comes. */
        if (err == 0 && context->callback)
        {
            context->callback->vsync(
                const_cast<hwc_procs_t *>(context->callback), 0, nextVsync);
        }
    }

    return NULL;
}

