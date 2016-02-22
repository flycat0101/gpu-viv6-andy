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


#include "gc_hwc.h"
#include "gc_hwc_display.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <time.h>
#include <pthread.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <cutils/sched_policy.h>
#include <hardware_legacy/uevent.h>
#include <linux/ioctl.h>

#include <utils/Thread.h>
#include <utils/ThreadDefs.h>
#include <utils/Timers.h>

extern "C" int clock_nanosleep(clockid_t clock_id, int flags,
                           const struct timespec *request,
                           struct timespec *remain);

/*
 * NOTICE & TODO:
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

    /* Vsync enabling flag. */
    volatile int    enabled;

    /* Thread exit flag. */
    volatile int    done;
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
hwcInitVsync(
    IN hwcContext * Context,
    IN hwcDisplay * Display
    )
{
    int err;

    /*
     * TODO: Do you need vsync for multiple displays?
     * For fake vsync, we only use one vsync thread.
     */
    if (Display->disp != HWC_DISPLAY_PRIMARY)
    {
        return -EINVAL;
    }

    vsyncContext * vsync = (vsyncContext *) malloc(sizeof (vsyncContext));

    pthread_mutex_init(&vsync->mutex, NULL);
    pthread_cond_init(&vsync->cond, NULL);

    /* Not enabled. */
    vsync->enabled  = 0;

    /* Thread should be running. */
    vsync->done     = 0;

    /* Save vsync context to hwc context. */
    Display->vsync = vsync;

    /* Create thread to handle vsync events. */
    err = pthread_create(&vsync->thread, NULL, vsyncThread, Context);

    if (err < 0)
    {
        ALOGE("%s: Create vsync thread failed", __FUNCTION__);
        free(vsync);
        Display->vsync = 0;
        return -EINVAL;
    }

    return 0;
}

int
hwcFinishVsync(
    IN hwcContext * Context,
    IN hwcDisplay * Display
    )
{
    /* Retrieve vsync context. */
    vsyncContext * vsync = (vsyncContext *) Display->vsync;

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
    IN hwcDisplay * Display,
    IN int Enable
    )
{
    /* Retrieve vsync context. */
    int err = 0;
    vsyncContext * vsync = (vsyncContext *) Display->vsync;

    if (vsync == NULL)
    {
        /* Invalid operation. */
        return -EINVAL;
    }

    /* Lock. */
    pthread_mutex_lock(&vsync->mutex);

    if (vsync->enabled != Enable)
    {
        /* Set enabling flag. */
        vsync->enabled = Enable;

        /* Wake up vsync thread. */
        pthread_cond_broadcast(&vsync->cond);
    }

    /* Release lock. */
    pthread_mutex_unlock(&vsync->mutex);

    return err;
}


void *
vsyncThread(
    void * param
    )
{
    hwcContext * context = (hwcContext *) param;
    hwcDisplay * display = context->displays[HWC_DISPLAY_PRIMARY];
    vsyncContext * vsync = (vsyncContext *) display->vsync;

    /* Set priority. */
    /*
    int prio = ANDROID_PRIORITY_URGENT_DISPLAY
             + ANDROID_PRIORITY_MORE_FAVORABLE;

    setpriority(PRIO_PROCESS, 0, prio);
    set_sched_policy(gettid(), SP_FOREGROUND);
     */

    /* Wait until call procs registered. */
    for (int count = 0; count < 1000; count++)
    {
        usleep(10000);

        if (context->callback)
        {
            /* Registered. */
            break;
        }
    }

    if (!context->callback)
    {
        ALOGI("vsync thread exit: callback procs not registered.");
        return NULL;
    }

    const nsecs_t period = 1e9 / display->device.fps;

    /* TODO: Implement real vsync. */
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

        /*--------------- Fake vsync. ---------------*/
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
        if (err == 0)
        {
            context->callback->vsync(
                const_cast<hwc_procs_t *>(context->callback), 0, nextVsync);
        }
    }

    return NULL;
}

