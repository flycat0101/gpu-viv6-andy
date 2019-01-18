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


#include <gc_gpe_local.hpp>

GC2DParms::GC2DParms()
{
    mType = GC2D_OBJ_UNKNOWN;
    mBlit = BLIT_INVALID;
    mTrsp = gcvSURF_OPAQUE;

    mMirrorX = gcvFALSE;
    mMirrorY = gcvFALSE;

    mSrcRect.left = 0;
    mSrcRect.top = 0;
    mSrcRect.right = 0;
    mSrcRect.bottom = 0;

    mDstRect.left = 0;
    mDstRect.top = 0;
    mDstRect.right = 0;
    mDstRect.bottom = 0;

    mMonoRect.left = 0;
    mMonoRect.top = 0;
    mMonoRect.right = 0;
    mMonoRect.bottom = 0;

    mFgRop = 0;
    mBgRop = 0;

    mSrcBuffer = gcvNULL;
    mSrcBufferPointer = gcvNULL;

    mMonoSupported = gcvTRUE;

    mMonoBuffer = gcvNULL;
    mMonoBufferSize = 0;

    mHwVersion = 0;
    mHwMirrorEx = 0;

    mABEnable = gcvFALSE;

    mStrideMask = 0;
    mAddrMask = 0;

    mOs = gcvNULL;
    mEngine2D = gcvNULL;
    mEnableFlag = 0;
    mSyncMode = GC2D_SYNC_MODE_ASYNC;
    mCommitCounter = 0;
    mFinishFlag = gcvNULL;
    mMinPixelNum = GC2D_MIN_RENDERING_PIXEL;
    mStallSignal = gcvNULL;
    mMinSrcCopyPixelNum = 0;
    mMinSolidFillPixelNum = 0;
    mMinMaskPixelNum = 0;

#if GC2D_GATHER_STATISTICS
    nPrepare = 0;
    nPrepareSucceed = 0;
    nROPBlit = 0;
    nROPBlitSucceed = 0;
    nAlphaBlend = 0;
    nAlphaBlendSucceed = 0;
    nASyncCommit = 0;
    nASyncCommitFailed = 0;
#endif
}

GC2DParms::~GC2DParms()
{
    gcmASSERT(mType == GC2D_OBJ_UNKNOWN);

    if (mBrush.mVirtualAddr)
    {
        gcmVERIFY(FreeNonCacheMem(mBrush.mVirtualAddr));
    }

    if (mDstSurf.mType != GC2D_OBJ_UNKNOWN)
    {
        ResetSurface(gcvFALSE);
    }

    if (mSrcSurf.mType != GC2D_OBJ_UNKNOWN)
    {
        ResetSurface(gcvTRUE);
    }

    if (mSrcBuffer)
    {
    }

    if (mMonoBuffer)
    {
    }
}

gctBOOL GC2DParms::ReadRegistry()
{
    DWORD status;
    HKEY HKey;
    gctBOOL valid = gcvFALSE;

    GC2D_ENTER();

    // Open the device key.
    status = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                GC2D_REGISTRY_PATH,
                0,
                0,
                &HKey);
    if (status != ERROR_SUCCESS)
    {
        GC2D_LEAVE_ARG("%d", gcvFALSE);
        return gcvFALSE;
    }

    do {
        DWORD valueType, valueSize = sizeof(mEnableFlag);
        if (ERROR_SUCCESS != RegQueryValueEx(HKey,
                                             GC2D_ENABLE_VALNAME,
                                             gcvNULL,
                                             &valueType,
                                             (LPBYTE) &mEnableFlag,
                                             &valueSize))
        {
            GC2D_DebugTrace(GC2D_ALWAYS_OUT, ("ReadRegistry Error: Can not get entry GC2DAcceleratorEnable!"));
            valid = gcvFALSE;
            break;
        }

        // Query the sync mode.
        valueSize = sizeof(mSyncMode);
        if (ERROR_SUCCESS != RegQueryValueEx(HKey,
                                             GC2D_SYNCMODE_VALNAME,
                                             gcvNULL,
                                             &valueType,
                                             (LPBYTE) &mSyncMode,
                                             &valueSize))
        {
            GC2D_DebugTrace(GC2D_ALWAYS_OUT, ("ReadRegistry Error: Can not get entry GC2DAcceleratorSyncMode!"));
            valid = gcvFALSE;
            break;
        }

        // Query the min size.
        valueSize = sizeof(mMinPixelNum);
        if (ERROR_SUCCESS != RegQueryValueEx(HKey,
                                             GC2D_MIN_SIZE_VALNAME,
                                             gcvNULL,
                                             &valueType,
                                             (LPBYTE) &mMinPixelNum,
                                             &valueSize))
        {
            GC2D_DebugTrace(GC2D_ALWAYS_OUT, ("ReadRegistry Error: Can not get entry  GC2DAcceleratorMinSize!"));
            valid = gcvFALSE;
            break;
        }

        valueSize = sizeof(mMinSrcCopyPixelNum);
        if (ERROR_SUCCESS != RegQueryValueEx(HKey,
                                             GC2D_MIN_SCRCOPY_VALNAME,
                                             gcvNULL,
                                             &valueType,
                                             (LPBYTE) &mMinSrcCopyPixelNum,
                                             &valueSize))
        {
            GC2D_DebugTrace(GC2D_ALWAYS_OUT, ("ReadRegistry Error: Can not get entry GC2DAcceleratorMinPixelForSimpleSrcCopy!"));
            valid = gcvFALSE;
            break;
        }

        valueSize = sizeof(mMinSolidFillPixelNum);
        if (ERROR_SUCCESS != RegQueryValueEx(HKey,
                                             GC2D_MIN_SOLIDFILL_VALNAME,
                                             gcvNULL,
                                             &valueType,
                                             (LPBYTE) &mMinSolidFillPixelNum,
                                             &valueSize))
        {
            GC2D_DebugTrace(GC2D_ALWAYS_OUT, ("ReadRegistry Error: Can not get entry GC2DAcceleratorMinPixelForSimpleBrushFill!"));
            valid = gcvFALSE;
            break;
        }

        valueSize = sizeof(mMinMaskPixelNum);
        if (ERROR_SUCCESS != RegQueryValueEx(HKey,
                                             GC2D_MIN_MASK_VALNAME,
                                             gcvNULL,
                                             &valueType,
                                             (LPBYTE) &mMinMaskPixelNum,
                                             &valueSize))
        {
            GC2D_DebugTrace(GC2D_ALWAYS_OUT, ("ReadRegistry Error: Can not get entry GC2DAcceleratorMinPixelForMask!"));
            valid = gcvFALSE;
            break;
        }

        valid = gcvTRUE;

    } while (gcvFALSE);

    // Close the device key.
    RegCloseKey(HKey);

    // Return error status.
    GC2D_LEAVE_ARG("%d", valid);
    return valid;
}

gctUINT GC2DParms::Enable(gctUINT* const pEnable)
{
    gctUINT ret = mEnableFlag;

    GC2D_ENTER_ARG("pEnable = 0x%08p", pEnable);

    if (pEnable)
    {
        mEnableFlag = *pEnable;
    }

    GC2D_LEAVE_ARG("ret = %d", ret);
    return ret;
}

gctBOOL GC2DParms::Initialize(
            GPE2D_PTR Acc2D
            )
{
    gceCHIPMODEL chipModel;
    gctUINT32 chipRevision;

    GC2D_ENTER_ARG("Acc2D = 0x%08x", Acc2D);

    if (IsValid())
    {
        goto OnError;
    }

    if (!ReadRegistry())
    {
        goto OnError;
    }

    if (gcmNO_SUCCESS(gcoOS_Construct(gcvNULL, &mOs)))
    {
        goto OnError;
    }

    if (gcmNO_SUCCESS(gcoHAL_Get2DEngine(gcvNULL, &mEngine2D)))
    {
        goto OnError;
    }

    mGPE2D = Acc2D;

    mGPE2D->GetPhysicalVideoMemory((unsigned long *)&mVideoPhyBase, (unsigned long *)&mVideoMemSize);

    gctUINT32 maxWidth = mGPE2D->ScreenWidth();
    gctUINT32 maxHeight = mGPE2D->ScreenHeight();

    AlignWidthHeight(&maxWidth, &maxHeight);

    mMaxPixelNum = maxWidth * maxHeight;
    if (!mMaxPixelNum)
    {
        goto OnError;
    }

    mFinishFlag = (gctUINT32 *)AllocNonCacheMem(PAGE_SIZE);
    if (!mFinishFlag)
    {
        goto OnError;
    }

    gcmASSERT(!BYTE_OFFSET(mFinishFlag));
    if (!LockPages(mFinishFlag, PAGE_SIZE, (PDWORD)&mFinishFlagPhys, LOCKFLAG_QUERY_ONLY))
    {
        goto OnError;
    }

    mFinishFlagPhys <<= UserKInfo[KINX_PFN_SHIFT];
    *mFinishFlag = 0;

    if (gcmIS_ERROR(gcoOS_CreateSignal(gcvNULL, gcvFALSE, &mStallSignal)))
    {
        goto OnError;
    }

    if (gcmIS_ERROR(gcoHAL_QueryChipIdentity(gcvNULL, &chipModel, &chipRevision, gcvNULL, gcvNULL)))
    {
        goto OnError;
    }

    if ((chipModel == gcv320) && (chipRevision == 0x5007))
    {
        mMonoSupported = gcvFALSE;
    }

    mMonoBufferSize = gcmALIGN(mMaxPixelNum, 8) >> 3;
    mMonoBuffer = new gctUINT8[mMonoBufferSize];
    if (mMonoBuffer == gcvNULL)
    {
        goto OnError;
    }

    if (!InitTempBuffer(&mSrcBuffer, maxWidth, maxHeight))
    {
        goto OnError;
    }

    if (!mBrush.Init())
    {
        goto OnError;
    }

    mDstSurf.mGPE2D =
    mSrcSurf.mGPE2D = mGPE2D;

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2DPE20) == gcvTRUE)
    {
        mHwVersion = 1;
    }

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_MIRROR_EXTENSION) == gcvTRUE)
    {
        mHwMirrorEx = 1;
    }

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_MULTI_SOURCE_BLT) == gcvTRUE)
    {
        gco2D_SetGdiStretchMode(mEngine2D, gcvTRUE);
    }

    if (gcmIS_ERROR(gco2D_QueryU32(mEngine2D, gcv2D_QUERY_RGB_ADDRESS_MIN_ALIGN, &mAddrMask))
        || gcmIS_ERROR(gco2D_QueryU32(mEngine2D, gcv2D_QUERY_RGB_STRIDE_MIN_ALIGN, &mStrideMask)))
    {
        goto OnError;
    }

    --mAddrMask;
    --mStrideMask;

    mType = GC2D_OBJ_PARMS;

    GC2D_LEAVE();
    return gcvTRUE;

OnError:

    GC2D_LEAVE();
    return gcvFALSE;

}

BOOL GC2DParms::Denitialize()
{
    Reset();

    if (!DenitTempBuffer(&mSrcBuffer))
    {
        GC2D_DebugTrace(GC2D_ERROR_TRACE,
            ("%s(%d): Failed to delete src temp surface!\r\n"),
            __FUNCTION__, __LINE__);
    }

    if (mMonoBuffer)
    {
        delete [] mMonoBuffer;
        mMonoBuffer = gcvNULL;
    }

    if (!mBrush.Denit())
    {
        GC2D_DebugTrace(GC2D_ERROR_TRACE,
            ("%s(%d): Failed to delete brush!\r\n"),
            __FUNCTION__, __LINE__);
    }

    mEngine2D = gcvNULL;

    if (mOs)
    {
        if (gcmIS_ERROR(gcoOS_Destroy(mOs)))
        {
            GC2D_DebugTrace(GC2D_ERROR_TRACE,
                ("%s(%d): Failed to delete gcoOS!\r\n"),
                __FUNCTION__, __LINE__);
        }

        mOs = gcvNULL;
    }

    if (mFinishFlag)
    {
        gcmVERIFY(FreeNonCacheMem(mFinishFlag));
        mFinishFlag = gcvNULL;
    }

    if (mStallSignal)
    {
        gcoOS_DestroySignal(gcvNULL, mStallSignal);
        mStallSignal = gcvNULL;
    }

    mType = GC2D_OBJ_UNKNOWN;

    return gcvTRUE;
}

BOOL GC2DParms::InitTempBuffer(
    GC2DSurf_PTR *Buffer,
    gctUINT Width,
    gctUINT Height)
{
    GC2DSurf_PTR g2dSurf = gcvNULL;
    gctUINT32 width, height, stride, size, address;
    gctUINT alignedWidth, alignedHeight, alignedAddress, alignedStride;

    do {
        // Get the aligned address
        if (gcmNO_SUCCESS(gco2D_QueryU32(mEngine2D, gcv2D_QUERY_RGB_ADDRESS_MIN_ALIGN, &alignedAddress)))
        {
            break;
        }

        // Get the aligned stride
        if (gcmNO_SUCCESS(gco2D_QueryU32(mEngine2D, gcv2D_QUERY_RGB_STRIDE_MIN_ALIGN, &alignedStride)))
        {
            break;
        }

        // Align the size
        QueryAlignment(&alignedWidth, &alignedHeight);

        width  = gcmALIGN(Width, alignedWidth);
        height = gcmALIGN(Height, alignedHeight);

        stride = width * GetFormatSize(gcvSURF_A8R8G8B8);
        stride = gcmALIGN(stride, alignedStride);

        // check width, height & stride
        if (width >= MAX_WIDTH_HEIGHT || height >= MAX_WIDTH_HEIGHT
            || stride >= MAX_STRIDE)
        {
            break;
        }

        // Allocate the memory buffer
        size = height * stride + (alignedAddress - 1);

        mSrcBufferPointer = AllocNonCacheMem(size);

        if (mSrcBufferPointer == gcvNULL)
        {
            break;
        }

        memset(mSrcBufferPointer, 0, size);

        address = gcmALIGN((gctUINT32)mSrcBufferPointer, alignedAddress);

        // Allocate the GC2DSurf object
        g2dSurf = new GC2DSurf;
        if (!g2dSurf)
        {
            break;
        }

        // Initialize the buffer
        if (gcmNO_SUCCESS(g2dSurf->Initialize(
                mGPE2D, GC2DSURF_PROPERTY_DEFAULT,
                INVALID_PHYSICAL_ADDRESS, (gctPOINTER)address,
                width, height, stride, gcvSURF_A8R8G8B8, gcvSURF_0_DEGREE)))
        {
            break;
        }

        *Buffer = g2dSurf;

        return gcvTRUE;

    } while (gcvFALSE);

    if (g2dSurf)
    {
        delete g2dSurf;
    }

    if (mSrcBufferPointer != gcvNULL)
    {
        FreeNonCacheMem(mSrcBufferPointer);
        mSrcBufferPointer = gcvNULL;
    }

    *Buffer = gcvNULL;

    return gcvFALSE;
}

gctBOOL GC2DParms::DenitTempBuffer(
    GC2DSurf_PTR *Buffer)
{
    GC2DSurf_PTR g2dSurf = *Buffer;

    if (g2dSurf)
    {
        g2dSurf->Denitialize(gcvTRUE);
        delete g2dSurf;

        if (mSrcBufferPointer != gcvNULL)
        {
            FreeNonCacheMem(mSrcBufferPointer);
            mSrcBufferPointer = gcvNULL;
        }

        *Buffer = gcvNULL;
    }

    return gcvTRUE;
}

gctBOOL GC2DBrush::Init()
{
    if (mVirtualAddr)
    {
        return gcvFALSE;
    }

    mVirtualAddr = AllocNonCacheMem(PAGE_SIZE);
    gcmASSERT(!BYTE_OFFSET(mVirtualAddr));
    if (!mVirtualAddr)
    {
        return gcvFALSE;
    }

    if (!LockPages(mVirtualAddr, PAGE_SIZE, (PDWORD)&mDMAAddr, LOCKFLAG_QUERY_ONLY))
    {
        gcmVERIFY(FreeNonCacheMem(mVirtualAddr));
        return gcvFALSE;
    }

    mDMAAddr <<= UserKInfo[KINX_PFN_SHIFT];

    return gcvTRUE;
}

gctBOOL GC2DBrush::Denit()
{
    if (!mVirtualAddr)
    {
        return gcvFALSE;
    }

    gcmVERIFY(FreeNonCacheMem(mVirtualAddr));
    mVirtualAddr = gcvNULL;
    mDMAAddr = INVALID_PHYSICAL_ADDRESS;

    return gcvTRUE;
}

