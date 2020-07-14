/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_gpe_local.hpp>

BOOL GC2D_Accelerator::GC2DInitialize(GPEBLTFUN CallbackBlit)
{
    BOOL ret = gcvTRUE;

    GC2D_ENTER();

    gctUINT32 maxWidth = ScreenWidth();
    gctUINT32 maxHeight = ScreenHeight();
    gctUINT32 videoPhyBase, videoMemSize;

    GetPhysicalVideoMemory((unsigned long *)&videoPhyBase, (unsigned long *)&videoMemSize);

    AlignWidthHeight(&maxWidth, &maxHeight);

    mParms = new GC2DParms;

    ret = mParms->Initialize(this);
    if (!ret)
    {
        mParms->Denitialize();
        delete mParms;
        mParms = gcvNULL;
    }

    mCallbackBlit = CallbackBlit;

    GC2D_LEAVE_ARG("ret = %d", ret);

    return ret;
}

BOOL GC2D_Accelerator::GC2DDeinitialize()
{
    BOOL ret = gcvFALSE;

    GC2D_ENTER();

    if (mParms)
    {
        ret = mParms->Denitialize();
        delete mParms;
        mParms = gcvNULL;
    }

    GC2D_LEAVE_ARG("ret = %d", ret);

    return ret;
}

gctUINT GC2D_Accelerator::GC2DSetEnable(gctUINT* const pEnable)
{
    return mParms->Enable(pEnable);
}

SCODE GC2D_Accelerator::GC2DBltPrepare(GPEBltParms *pBltParms)
{
    return mParms->BltPrepare(pBltParms);
}

SCODE GC2D_Accelerator::GC2DBltComplete(GPEBltParms *pBltParms)
{
    return mParms->BltComplete(pBltParms);
}

int GC2D_Accelerator::GC2DIsBusy()
{
    return mParms->IsBusy();
}

void GC2D_Accelerator::GC2DWaitForNotBusy()
{
    return mParms->WaitForNotBusy();
}

gctUINT GC2D_Accelerator::GC2DChangeSyncMode(gctUINT* const pMode)
{
    return mParms->ChangeSyncMode(pMode);
}

gctUINT GC2D_Accelerator::GC2DPowerHandler(BOOL bOff)
{
    return mParms->PowerHandler(bOff);
}

SCODE GC2D_Accelerator::AnyBlt(GPEBltParms * pBltParms)
{
    SCODE sc = -ERROR_NOT_SUPPORTED;

    GC2D_ENTER_ARG("pBltParms = 0x%08p", pBltParms);

    sc = mParms->Blit(pBltParms);
    if (sc != S_OK)
    {
        GC2D_DebugTrace(GC2D_DISPLAY_BLIT, ("GC2DAnyBlt: call EmulatedBlt\r\n"));
        pBltParms->pBlt = &GPE::EmulatedBlt;
        sc = EmulatedBlt(pBltParms);
    }

    GC2D_LEAVE_ARG("%d", sc);

    return sc;
}
