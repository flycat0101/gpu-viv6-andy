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

//////////////////////////////////////////////////////////////////////////////
//              GC2DSurf
//////////////////////////////////////////////////////////////////////////////

GC2DSurf::GC2DSurf()
{
    mType = GC2D_OBJ_UNKNOWN;
    mFlag = 0;
    mGPE2D = gcvNULL;

    mDMAAddr = INVALID_PHYSICAL_ADDRESS;
    mPhysicalAddr = INVALID_PHYSICAL_ADDRESS;
    mVirtualAddr = gcvNULL;

    mWrapped = gcvFALSE;
    mWrappedNode = 0;

    mStride = 0;
    mWidth = 0;
    mHeight = 0;
    mSize = 0;
    mFormat = gcvSURF_UNKNOWN;
    mRotate = gcvSURF_0_DEGREE;

    mOwnerFlag = 0;

    mUDMAAddr = INVALID_PHYSICAL_ADDRESS;
    mUVirtualAddr = gcvNULL;
    mUPhysicalAddr = INVALID_PHYSICAL_ADDRESS;
    mUstride = 0;
    mUWrapped = gcvFALSE;
    mUWrappedNode = 0;

    mVDMAAddr = INVALID_PHYSICAL_ADDRESS;
    mVVirtualAddr = gcvNULL;
    mVPhysicalAddr = INVALID_PHYSICAL_ADDRESS;
    mVstride = 0;
    mVWrapped = gcvFALSE;
    mVWrappedNode = 0;
}

GC2DSurf::~GC2DSurf()
{
    if (IsValid())
    {
        Denitialize(gcvTRUE);
    }
}

gceSTATUS GC2DSurf::Initialize(
        GPE2D_PTR pAcc2D,
        gctUINT flag, gctUINT PhysAddr, gctPOINTER VirtAddr,
        gctUINT width, gctUINT height, gctUINT stride,
        gceSURF_FORMAT format, gceSURF_ROTATION rotate)
{
    gceSTATUS status;
    gcsUSER_MEMORY_DESC desc;
    gctUINT32 node, address;

    // already initialized
    if (mType != GC2D_OBJ_UNKNOWN)
    {
        return gcvSTATUS_INVALID_OBJECT;
    }

    if (pAcc2D == gcvNULL)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    // check format
    switch (format)
    {
    // list of the supported formats except planar YUV
    case    gcvSURF_X4R4G4B4:
    case    gcvSURF_A4R4G4B4:
    case    gcvSURF_X1R5G5B5:
    case    gcvSURF_A1R5G5B5:
    case    gcvSURF_R5G6B5  :
    case    gcvSURF_X8R8G8B8:
    case    gcvSURF_A8R8G8B8:
    case    gcvSURF_INDEX8  :
    case    gcvSURF_YUY2:
    case    gcvSURF_UYVY:
        break;

    default:
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    // check width , height & stride
    if (width >= MAX_WIDTH_HEIGHT || height >= MAX_WIDTH_HEIGHT
        || stride >= MAX_STRIDE || width > stride)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    // alignment check
    if (gcmNO_SUCCESS(AlignWidthHeight(&width, &height)))
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    // check buffer, can not wrap without buffer
    if ((PhysAddr == INVALID_PHYSICAL_ADDRESS) && (VirtAddr == gcvNULL))
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    // check flags
    if (flag & (~GC2DSURF_PROPERTY_ALL))
    {
        // unknown flag
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    mFormat = format;
    mRotate = rotate;
    mWidth = width;
    mHeight = height;
    mStride = stride;
    mSize = stride * height;
    mFlag = flag;
    mPhysicalAddr = PhysAddr;
    mVirtualAddr = VirtAddr;

    // Map to GPU address
    gcoOS_ZeroMemory(&desc, sizeof(desc));

    desc.flag     = gcvALLOC_FLAG_USERMEMORY;
    desc.logical  = gcmPTR_TO_UINT64(mVirtualAddr);
    desc.physical = mPhysicalAddr;
    desc.size     = mSize;

    status = gcoHAL_WrapUserMemory(&desc, gcvVIDMEM_TYPE_BITMAP, &node);
    if (gcmIS_ERROR(status))
    {
        GC2D_DebugTrace(GC2D_ERROR_TRACE,
            ("%s(%d): Failed to wrap user memory: error=%d!\r\n"),
            __FUNCTION__, __LINE__, status);

        return status;
    }

    status = gcoHAL_LockVideoMemory(node, gcvFALSE, gcvENGINE_RENDER, &address, gcvNULL);
    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(gcoHAL_ReleaseVideoMemory(node));

        GC2D_DebugTrace(GC2D_ERROR_TRACE,
            ("%s(%d): Failed to lock video memory: error=%d!\r\n"),
            __FUNCTION__, __LINE__, status);

        return status;
    }

    mDMAAddr = address;
    mWrapped = gcvTRUE;
    mWrappedNode = node;

    // Init the member variables.
    mGPE2D = pAcc2D;

    mUDMAAddr = INVALID_PHYSICAL_ADDRESS;
    mUPhysicalAddr = INVALID_PHYSICAL_ADDRESS;
    mUVirtualAddr = gcvNULL;
    mUstride = 0;
    mUWrapped = gcvFALSE;
    mUWrappedNode = 0;

    mVDMAAddr = INVALID_PHYSICAL_ADDRESS;
    mVPhysicalAddr = INVALID_PHYSICAL_ADDRESS;
    mVVirtualAddr = gcvNULL;
    mVstride = 0;
    mVWrapped = gcvFALSE;
    mVWrappedNode = 0;

    mType = GC2D_OBJ_SURF;

    return gcvSTATUS_OK;
}

// if both UXXXXAddr and VXXXXAddr are NULL, we take the U component and V component
//  contiguous with Y component
gceSTATUS GC2DSurf::InitializeYUV(
        GPE2D_PTR pAcc2D,
        gctUINT flag, gceSURF_FORMAT format,
        gctUINT YPhysAddr, gctPOINTER YVirtAddr, gctUINT YStride,
        gctUINT UPhysAddr, gctPOINTER UVirtAddr, gctUINT UStride,
        gctUINT VPhysAddr, gctPOINTER VVirtAddr, gctUINT VStride,
        gctUINT width, gctUINT height, gceSURF_ROTATION rotate
        )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsUSER_MEMORY_DESC desc;
    gctUINT32 node, address;
    gctBOOL UVContingous =
            (UPhysAddr == INVALID_PHYSICAL_ADDRESS && UVirtAddr == gcvNULL
            && VPhysAddr == INVALID_PHYSICAL_ADDRESS && VVirtAddr == gcvNULL);
    gctUINT32 UVSize;

    // already initialized
    if (mType != GC2D_OBJ_UNKNOWN)
    {
        return gcvSTATUS_INVALID_OBJECT;
    }

    if ((pAcc2D == gcvNULL) || (!pAcc2D->GC2DIsValid()))
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    // check format
    switch (format)
    {
    // list of the supported formats
    case    gcvSURF_YV12:
    case    gcvSURF_I420:
        if (!UVContingous)
        {
            if (((UPhysAddr == INVALID_PHYSICAL_ADDRESS) && (UVirtAddr == gcvNULL))
                || ((VPhysAddr == INVALID_PHYSICAL_ADDRESS) && (VVirtAddr == gcvNULL)))
            {
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }

        if ((YPhysAddr == INVALID_PHYSICAL_ADDRESS) && (YVirtAddr == gcvNULL))
        {
            return gcvSTATUS_INVALID_ARGUMENT;
        }
        break;

    case    gcvSURF_YUY2:
    case    gcvSURF_UYVY:
        if (!UVContingous)
        {
            return gcvSTATUS_INVALID_ARGUMENT;
        }
        else
        {
            return Initialize(pAcc2D, flag, YPhysAddr, YVirtAddr, width, height, YStride, format, rotate);
        }

    default:
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    // check width , height & stride
    if (width >= MAX_WIDTH_HEIGHT || height >= MAX_WIDTH_HEIGHT
        || YStride >= MAX_STRIDE || UStride >= MAX_STRIDE
        || VStride >= MAX_STRIDE)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    // alignment check
    if (gcmNO_SUCCESS(AlignWidthHeight(&width, &height)))
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    // check flags
    if (flag & (~GC2DSURF_PROPERTY_ALL))
    {
        // unknown flag
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if (!(flag & GC2DSURF_PROPERTY_CONTIGUOUS) && YVirtAddr == gcvNULL)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    // Total size.
    mSize = height * YStride;

    if (UVContingous)
    {
        mSize += mSize >> 1;
    }

    // Map to GPU address
    gcoOS_ZeroMemory(&desc, sizeof(desc));

    desc.flag     = gcvALLOC_FLAG_USERMEMORY;
    desc.logical  = gcmPTR_TO_UINT64(YVirtAddr);
    desc.physical = YPhysAddr;
    desc.size     = mSize;

    status = gcoHAL_WrapUserMemory(&desc, gcvVIDMEM_TYPE_BITMAP, &node);
    if (gcmIS_ERROR(status))
    {
        GC2D_DebugTrace(GC2D_ERROR_TRACE,
            ("%s(%d): Failed to wrap user memory: error=%d!\r\n"),
            __FUNCTION__, __LINE__, status);

        return status;
    }

    status = gcoHAL_LockVideoMemory(node, gcvFALSE, gcvENGINE_RENDER, &address, gcvNULL);
    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(gcoHAL_ReleaseVideoMemory(node));

        GC2D_DebugTrace(GC2D_ERROR_TRACE,
            ("%s(%d): Failed to lock video memory: error=%d!\r\n"),
            __FUNCTION__, __LINE__, status);

        return status;
    }

    mDMAAddr = address;
    mWrapped = gcvTRUE;
    mWrappedNode = node;
    mPhysicalAddr = YPhysAddr;
    mVirtualAddr = YVirtAddr;
    mWidth = width;
    mHeight = height;
    mStride = YStride;

    if (UStride == 0)
    {
        mUstride = mStride >> 1;
    }

    if (VStride == 0)
    {
        mVstride = mStride >> 1;
    }

    UVSize = (mHeight >> 1) * mUstride;

    if (UVContingous)
    {
        if (gcvSURF_I420 == format)
        {
            mUDMAAddr = mDMAAddr + mHeight * mStride;
            mVDMAAddr = mUDMAAddr + UVSize;
        }
        else if (gcvSURF_YV12 == format)
        {
            mVDMAAddr = mDMAAddr + mHeight * mStride;
            mUDMAAddr = mVDMAAddr + UVSize;
        }
        else
        {
            goto ERROR_EXIT;
        }
    }
    else
    {
        // Map U to GPU address
        gcoOS_ZeroMemory(&desc, sizeof(desc));

        desc.flag     = gcvALLOC_FLAG_USERMEMORY;
        desc.logical  = gcmPTR_TO_UINT64(UVirtAddr);
        desc.physical = UPhysAddr;
        desc.size     = UVSize;

        status = gcoHAL_WrapUserMemory(&desc, gcvVIDMEM_TYPE_BITMAP, &node);
        if (gcmIS_ERROR(status))
        {
            GC2D_DebugTrace(GC2D_ERROR_TRACE,
                ("%s(%d): Failed to wrap user memory: error=%d!\r\n"),
                __FUNCTION__, __LINE__, status);

            goto ERROR_EXIT;
        }

        status = gcoHAL_LockVideoMemory(node, gcvFALSE, gcvENGINE_RENDER, &address, gcvNULL);
        if (gcmIS_ERROR(status))
        {
            gcmVERIFY_OK(gcoHAL_ReleaseVideoMemory(node));

            GC2D_DebugTrace(GC2D_ERROR_TRACE,
                ("%s(%d): Failed to lock video memory: error=%d!\r\n"),
                __FUNCTION__, __LINE__, status);

            goto ERROR_EXIT;
        }

        mUDMAAddr = address;
        mUWrapped = gcvTRUE;
        mUWrappedNode = node;

        // Map V to GPU address
        gcoOS_ZeroMemory(&desc, sizeof(desc));

        desc.flag     = gcvALLOC_FLAG_USERMEMORY;
        desc.logical  = gcmPTR_TO_UINT64(VVirtAddr);
        desc.physical = VPhysAddr;
        desc.size     = UVSize;

        status = gcoHAL_WrapUserMemory(&desc, gcvVIDMEM_TYPE_BITMAP, &node);
        if (gcmIS_ERROR(status))
        {
            GC2D_DebugTrace(GC2D_ERROR_TRACE,
                ("%s(%d): Failed to wrap user memory: error=%d!\r\n"),
                __FUNCTION__, __LINE__, status);

            return status;
        }

        status = gcoHAL_LockVideoMemory(node, gcvFALSE, gcvENGINE_RENDER, &address, gcvNULL);
        if (gcmIS_ERROR(status))
        {
            gcmVERIFY_OK(gcoHAL_ReleaseVideoMemory(node));

            GC2D_DebugTrace(GC2D_ERROR_TRACE,
                ("%s(%d): Failed to lock video memory: error=%d!\r\n"),
                __FUNCTION__, __LINE__, status);

            return status;
        }

        mVDMAAddr = address;
        mVWrapped = gcvTRUE;
        mVWrappedNode = node;
    }

    mUPhysicalAddr = UPhysAddr;
    mUVirtualAddr = UVirtAddr;
    mVPhysicalAddr = VPhysAddr;
    mVVirtualAddr = VVirtAddr;

    // init the member variables
    mGPE2D = pAcc2D;
    mFormat = format;
    mRotate = rotate;

    mFlag = flag;

    mType = GC2D_OBJ_SURF;

    return gcvSTATUS_OK;

ERROR_EXIT:

    if (mWrapped)
    {
        gcmVERIFY_OK(gcoHAL_UnlockVideoMemory(mWrappedNode, gcvVIDMEM_TYPE_BITMAP, gcvENGINE_RENDER));
        gcmVERIFY_OK(gcoHAL_ReleaseVideoMemory(mWrappedNode));
    }

    if (mUWrapped)
    {
        gcmVERIFY_OK(gcoHAL_UnlockVideoMemory(mUWrappedNode, gcvVIDMEM_TYPE_BITMAP, gcvENGINE_RENDER));
        gcmVERIFY_OK(gcoHAL_ReleaseVideoMemory(mUWrappedNode));
    }

    if (mVWrapped)
    {
        gcmVERIFY_OK(gcoHAL_UnlockVideoMemory(mVWrappedNode, gcvVIDMEM_TYPE_BITMAP, gcvENGINE_RENDER));
        gcmVERIFY_OK(gcoHAL_ReleaseVideoMemory(mVWrappedNode));
    }

    // clear this object
    mType = GC2D_OBJ_UNKNOWN;
    mFlag = 0;
    mGPE2D = gcvNULL;

    mDMAAddr = INVALID_PHYSICAL_ADDRESS;
    mPhysicalAddr = INVALID_PHYSICAL_ADDRESS;
    mVirtualAddr = 0;

    mWrapped = gcvFALSE;
    mWrappedNode = 0;

    mStride = 0;
    mWidth = 0;
    mHeight = 0;
    mSize = 0;
    mFormat = gcvSURF_UNKNOWN;
    mRotate = gcvSURF_0_DEGREE;

    mOwnerFlag = 0;

    mUDMAAddr = INVALID_PHYSICAL_ADDRESS;
    mUVirtualAddr = gcvNULL;
    mUPhysicalAddr = INVALID_PHYSICAL_ADDRESS;
    mUstride = 0;
    mUWrapped = gcvFALSE;
    mUWrappedNode = 0;

    mVDMAAddr = INVALID_PHYSICAL_ADDRESS;
    mVVirtualAddr = gcvNULL;
    mVPhysicalAddr = INVALID_PHYSICAL_ADDRESS;
    mVstride = 0;
    mVWrapped = gcvFALSE;
    mVWrappedNode = 0;

    return gcvSTATUS_INVALID_ARGUMENT;
}

gctBOOL GC2DSurf::IsValid() const
{
#ifdef DEBUG
    if (!mGPE2D || !mGPE2D->GC2DIsValid())
    {
        return gcvFALSE;
    }

    if ((mFormat == gcvSURF_UNKNOWN) || (mGPE2D == gcvNULL))
    {
        return gcvFALSE;
    }
#endif
    return (mType == GC2D_OBJ_SURF);
}

gceSTATUS GC2DSurf::Denitialize(gctBOOL sync)
{
    gctBOOL UVContingous = gcvFALSE;
    gctUINT32 UVSize = 0;

    gcmASSERT(IsValid());

    if (!IsValid())
    {
        return gcvSTATUS_INVALID_OBJECT;
    }

    switch (mFormat)
    {
    case    gcvSURF_YV12:
    case    gcvSURF_I420:
        gcmASSERT((mUWrapped == gcvFALSE && mVWrapped == gcvFALSE)
            || (mUWrapped != gcvFALSE && mVWrapped != gcvFALSE));

        if ((mUWrapped == gcvFALSE) && (mVWrapped == gcvFALSE))
        {
            UVContingous = gcvTRUE;
        }

        UVSize = mHeight * mUstride;

    default:
        break;
    }

    if (mWrapped)
    {
        gcmVERIFY_OK(gcoHAL_UnlockVideoMemory(mWrappedNode, gcvVIDMEM_TYPE_BITMAP, gcvENGINE_RENDER));
        gcmVERIFY_OK(gcoHAL_ReleaseVideoMemory(mWrappedNode));
    }

    if (mUWrapped)
    {
        gcmVERIFY_OK(gcoHAL_UnlockVideoMemory(mUWrappedNode, gcvVIDMEM_TYPE_BITMAP, gcvENGINE_RENDER));
        gcmVERIFY_OK(gcoHAL_ReleaseVideoMemory(mUWrappedNode));
    }

    if (mVWrapped)
    {
        gcmVERIFY_OK(gcoHAL_UnlockVideoMemory(mVWrappedNode, gcvVIDMEM_TYPE_BITMAP, gcvENGINE_RENDER));
        gcmVERIFY_OK(gcoHAL_ReleaseVideoMemory(mVWrappedNode));
    }

    // clear this object
    mType = GC2D_OBJ_UNKNOWN;
    mFlag = 0;
    mGPE2D = gcvNULL;

    mDMAAddr = INVALID_PHYSICAL_ADDRESS;
    mPhysicalAddr = INVALID_PHYSICAL_ADDRESS;
    mVirtualAddr = 0;

    mWrapped = gcvFALSE;
    mWrappedNode = 0;

    mStride = 0;
    mWidth = 0;
    mHeight = 0;
    mSize = 0;
    mFormat = gcvSURF_UNKNOWN;
    mRotate = gcvSURF_0_DEGREE;

    mOwnerFlag = 0;

    mUDMAAddr = INVALID_PHYSICAL_ADDRESS;
    mUVirtualAddr = gcvNULL;
    mUPhysicalAddr = INVALID_PHYSICAL_ADDRESS;
    mUstride = 0;
    mUWrapped = gcvFALSE;
    mUWrappedNode = 0;

    mVDMAAddr = INVALID_PHYSICAL_ADDRESS;
    mVVirtualAddr = gcvNULL;
    mVPhysicalAddr = INVALID_PHYSICAL_ADDRESS;
    mVstride = 0;
    mVWrapped = gcvFALSE;
    mVWrappedNode = 0;

    return gcvSTATUS_OK;
}

//////////////////////////////////////////////////////////////////////////////
//              GC2D_SurfWrapper
//////////////////////////////////////////////////////////////////////////////
GC2D_SurfWrapper::GC2D_SurfWrapper(
        GPE2D_PTR pAcc2D,
        int                  width,
        int                  height,
        EGPEFormat           format): mAllocFlagByUser(0), mGPE2D(gcvNULL)
{
    gceSURF_FORMAT    gcFormat = EGPEFormatToGALFormat(format);
    int                bpp = GetFormatSize(gcFormat);

    GC2D_ENTER_ARG("pAcc2D = 0x%08p, width = %d, height = %d, format = %d",
        pAcc2D, width, height, format);

    mGC2DSurf = gcvNULL;
    if (pAcc2D && pAcc2D->GC2DIsValid() && (gcFormat != gcvSURF_UNKNOWN) && (bpp != 0))
    {
        gctUINT32   AlignedWidth = width;
        gctUINT32   AlignedHeight = height;

        AlignWidthHeight(&AlignedWidth, &AlignedHeight);
        if (AlignedWidth == width && AlignedHeight == height)
        {
            void *bits = AllocNonCacheMem(width*height*bpp);
            if (bits)
            {
                // wrapped by GC2DSurf
                mGPE2D = pAcc2D;
                mGC2DSurf = new GC2DSurf;

                if (gcmIS_SUCCESS(mGC2DSurf->Initialize(pAcc2D,
                        GC2DSURF_PROPERTY_DEFAULT, 0, bits,
                        width, height, width*GetFormatSize(gcFormat), gcFormat, gcvSURF_0_DEGREE)))
                {
                    GPESurf::Init(width, height, mGC2DSurf->mVirtualAddr, mGC2DSurf->mStride, format);
                    mAllocFlagByUser = 1;
                    m_fOwnsBuffer = GC2D_OBJ_SURF;

                    GC2D_LEAVE();
                    return;
                }
                else
                {
                    FreeNonCacheMem(bits);
                    mGPE2D = gcvNULL;
                }
            }
        }
    }

    DDGPESurf::DDGPESurf(width, height, format);

    GC2D_LEAVE();
}

#ifndef DERIVED_FROM_GPE
GC2D_SurfWrapper::GC2D_SurfWrapper(
        GPE2D_PTR  pAcc2D,
        int                   width,
        int                   height,
        int                   stride,
        EGPEFormat            format,
        EDDGPEPixelFormat     pixelFormat): mAllocFlagByUser(0), mGPE2D(gcvNULL)
{
    gceSURF_FORMAT    gcFormat = DDGPEFormatToGALFormat(pixelFormat);
    int                bpp = GetFormatSize(gcFormat);

    GC2D_ENTER_ARG("pAcc2D = 0x%08p, width = %d, height = %d, stride = %, format = %d, pixelFormat = %d",
        pAcc2D, width, height, stride, format, pixelFormat);

    if (pAcc2D && pAcc2D->GC2DIsValid() && (gcFormat != gcvSURF_UNKNOWN) && (bpp != 0))
    {
        gctUINT32   AlignedWidth = width;
        gctUINT32   AlignedHeight = height;

        AlignWidthHeight(&AlignedWidth, &AlignedHeight);
        if (AlignedWidth == width && AlignedHeight == height)
        {
            // wrapped by GC2DSurf
            void *bits = AllocNonCacheMem(width*height*bpp);
            if (bits)
            {
                mGPE2D = pAcc2D;
                mGC2DSurf = new GC2DSurf;

                if (gcmIS_SUCCESS(mGC2DSurf->Initialize(pAcc2D,
                        GC2DSURF_PROPERTY_DEFAULT, 0, bits,
                        width, height, width*GetFormatSize(gcFormat), gcFormat, gcvSURF_0_DEGREE)))
                {
                    Init(width, height, mGC2DSurf->mVirtualAddr, mGC2DSurf->mStride, format, pixelFormat);
                    mAllocFlagByUser = 1;
                    m_fOwnsBuffer = GC2D_OBJ_SURF;

                    GC2D_LEAVE();
                    return;
                }
                else
                {
                    FreeNonCacheMem(bits);
                    mGPE2D = gcvNULL;
                }
            }
        }
    }

    DDGPESurf::DDGPESurf(width,
                         height,
                         stride,
                         format,
                         pixelFormat);
    GC2D_LEAVE();
}
#endif

GC2D_SurfWrapper::GC2D_SurfWrapper(               // Create video-memory surface
                        GPE2D_PTR  pAcc2D,
                        int                   width,
                        int                   height,
                        void *                pBits,            // virtual address of allocated bits
                        int                   stride,
                        EGPEFormat            format): mAllocFlagByUser(0), mGPE2D(gcvNULL)
{
    gceSURF_FORMAT    gcFormat = EGPEFormatToGALFormat(format);
    EDDGPEPixelFormat    pixelFormat = EGPEFormatToEDDGPEPixelFormat[format];

    GC2D_ENTER_ARG("pAcc2D = 0x%08p, width = %d, height = %d, pBits = 0x%08p, stride = %, format = %d",
        pAcc2D, width, height, pBits, stride, format);

    if (pAcc2D && pAcc2D->GC2DIsValid() && (gcFormat != gcvSURF_UNKNOWN))
    {
        gctUINT32   AlignedWidth = width;
        gctUINT32   AlignedHeight = height;

        AlignWidthHeight(&AlignedWidth, &AlignedHeight);
        if (AlignedWidth == width && AlignedHeight == height)
        {
            // wrapped by GC2DSurf
            mGPE2D = pAcc2D;
            mGC2DSurf = new GC2DSurf;

            if (gcmIS_SUCCESS(mGC2DSurf->Initialize(pAcc2D,
                        GC2DSURF_PROPERTY_DEFAULT, 0, pBits,
                        width, height, width*GetFormatSize(gcFormat), gcFormat, gcvSURF_0_DEGREE)))
            {
            #ifdef DERIVED_FROM_GPE
                    Init(width, height, mGC2DSurf->mVirtualAddr, mGC2DSurf->mStride, format);
            #else
                Init(width, height, mGC2DSurf->mVirtualAddr, mGC2DSurf->mStride, format, pixelFormat);
            #endif
                m_fOwnsBuffer = GC2D_OBJ_SURF;

                GC2D_LEAVE();
                return;
            }
            else
            {
                mGPE2D = gcvNULL;
            }
        }
    }

    DDGPESurf::DDGPESurf(width,
                         height,
                         pBits,
                         stride,
                         format);
    GC2D_LEAVE();
}

GC2D_SurfWrapper::~GC2D_SurfWrapper()
{
    GC2D_ENTER();

    if (m_fOwnsBuffer == GC2D_OBJ_SURF)
    {
        gcmASSERT(mGC2DSurf->IsValid());

        mGC2DSurf->Denitialize(gcvTRUE);
        if (mAllocFlagByUser)
        {
            FreeNonCacheMem(mGC2DSurf->mVirtualAddr);
            mAllocFlagByUser = 0;
        }

        delete mGC2DSurf;
        mGPE2D = gcvNULL;
        m_fOwnsBuffer = 0;
    }

    GC2D_LEAVE();
}

BOOL GC2D_SurfWrapper::IsWrapped() const
{
    return m_fOwnsBuffer == GC2D_OBJ_SURF;
}
