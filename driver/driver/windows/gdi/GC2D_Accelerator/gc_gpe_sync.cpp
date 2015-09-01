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


#include <gc_gpe_local.hpp>

gctUINT GC2DParms::ChangeSyncMode(gctUINT* const pMode)
{
    gctUINT oldMode = mSyncMode;

    GC2D_ENTER_ARG("pMode = 0x%08p", pMode);

    if (pMode)
    {
        WaitForNotBusy();
        mSyncMode = *pMode;
    }

    GC2D_LEAVE_ARG("oldMode = %d", oldMode);
    return oldMode;
}

int GC2DParms::UpdateBusyStatus(gctBOOL clear)
{
    int ret;

    GC2D_ENTER_ARG("clear = %d", clear);

    do {
        if (clear)
        {
            *mFinishFlag = mCommitCounter = 0;
            ret = 0;
            break;
        }

        if ((++mCommitCounter) == 0xFFFFFFFF)
        {
            gcmVERIFY_OK(FlushAndCommit(gcvFALSE, gcvTRUE));
            *mFinishFlag = mCommitCounter = 0;
            ret = 0;
            break;
        }

        gcsHAL_INTERFACE halInterface;
        halInterface.command = gcvHAL_WRITE_DATA;
        halInterface.u.WriteData.address = mFinishFlagPhys;
        halInterface.u.WriteData.data = mCommitCounter;

        // schedule an event to write the FinishFlag
        gcmVERIFY_OK(gcoHAL_ScheduleEvent(gcvNULL, &halInterface));

        // commit command buffer without wait
        gcmVERIFY_OK(FlushAndCommit(gcvFALSE, gcvFALSE));

#if GC2D_GATHER_STATISTICS
        ++nASyncCommit;
#endif

        ret = mCommitCounter;

    } while (gcvFALSE);

    GC2D_LEAVE_ARG("ret = %d", ret);

    return ret;
}

int GC2DParms::IsBusy()
{
    int ret;

    GC2D_ENTER();

    if ((mSyncMode >= GC2D_SYNC_MODE_ASYNC) && mCommitCounter && IsValid())
    {
        ret = (*mFinishFlag) < mCommitCounter;
    }
    else
    {
        ret = 0;
    }

    GC2D_LEAVE_ARG("ret = %d", ret);

    return ret;
}

void GC2DParms::WaitForNotBusy()
{
    GC2D_ENTER();

    if ((mSyncMode >= GC2D_SYNC_MODE_ASYNC) && IsBusy())
    {
        gcmVERIFY_OK(FlushAndCommit(gcvFALSE, gcvTRUE));
        *mFinishFlag = mCommitCounter = 0;
#if GC2D_GATHER_STATISTICS
        ++nASyncCommitFailed;
#endif
    }

    GC2D_LEAVE();
}

gctUINT GC2DParms::PowerHandler(BOOL bOff)
{
    gctUINT ret;

    GC2D_ENTER_ARG("bOff = %d", bOff);

    if (bOff)
    {
        ret = 0;

        WaitForNotBusy();

        // Disable accelerator
        mPowerHandle = Enable(&ret);

        ret = mPowerHandle;
    }
    else
    {
        // Revert accelerator
        ret = Enable(&mPowerHandle);
    }

    GC2D_LEAVE_ARG("%d", ret);

    return ret;
}

gceSTATUS GC2DParms::FlushAndCommit(gctBOOL bFlush, gctBOOL bWait)
{
    gceSTATUS status;

    if (bWait)
    {
        status = WaitForHW();
    }
    else
    {
        status = gcoHAL_Commit(gcvNULL, bWait);
    }

    return status;
}

gceSTATUS GC2DParms::WaitForHW()
{
    gceSTATUS status;

    /* Create a signal event. */
    gcsHAL_INTERFACE iface;
    iface.command            = gcvHAL_SIGNAL;
    iface.u.Signal.signal    = gcmPTR_TO_UINT64(mStallSignal);
    iface.u.Signal.auxSignal = 0;
    iface.u.Signal.process   = gcmPTR_TO_UINT64(gcoOS_GetCurrentProcessID());
    iface.u.Signal.fromWhere = gcvKERNEL_PIXEL;

    /* Schedule the event. */
    status = gcoHAL_ScheduleEvent(gcvNULL, &iface);
    if (gcmIS_ERROR(status))
    {
        return status;
    }

    status = gcoHAL_Commit(gcvNULL, gcvFALSE);
    if (gcmIS_ERROR(status))
    {
        return status;
    }

    /* Wait for the signal. */
    gctUINT32 time = GC2D_WAIT_TIME_OUT;
    do {
        status = gcoOS_WaitSignal(gcvNULL, mStallSignal, time);
        if (status != gcvSTATUS_OK)
        {
            GC2D_DebugFatal("2D hardware hung (timeout:%d)!", time);
        }

        time += GC2D_WAIT_TIME_OUT;
    } while ((status != gcvSTATUS_OK)
#if gcdGPU_TIMEOUT
        && (time < gcdGPU_TIMEOUT)
#endif
        );

    return status;
}
