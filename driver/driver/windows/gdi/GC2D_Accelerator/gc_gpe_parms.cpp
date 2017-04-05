/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_gpe_local.hpp>

gctBOOL GC2DParms::SetBrush(
        GPEBltParms * pBltParms,
        gceSURF_ROTATION DestRotate)
{
    gceSTATUS status;

    gcmASSERT(pBltParms);
    gcmASSERT(mBrush.mType == GC2D_OBJ_UNKNOWN);

    GPESurf* pBrush = (GPESurf*)pBltParms->pBrush;
    if(pBrush)
    {
        gcmASSERT(pBrush->Format() != gpe1Bpp);
        gcmASSERT(pBrush->Format() != gpe24Bpp);
        gcmASSERT(pBrush->Width() == 8);
        gcmASSERT(pBrush->Height() == 8);
        gcmASSERT(!pBrush->IsRotate());

        gctINT stride = pBrush->Stride();
        gceSURF_FORMAT format = GPEFormatToGALFormat(pBrush->Format(), pBrush->FormatPtr());

        gcmASSERT(format != gcvSURF_UNKNOWN);
        gcmASSERT(format != gcvSURF_INDEX8);
        gcmASSERT(stride != 0);

        LONG x;
        LONG y;
        if (pBltParms->pptlBrush)
        {
            x = pBltParms->pptlBrush->x;
            y = pBltParms->pptlBrush->y;
        }
        else
        {
            x = y = 0;
        }

        if (x < 0)
        {
            x = 8 - ((-x)%8);
        }
        else
        {
            x %= 8;
        }

        if (y < 0)
        {
            y = 8 - ((-y)%8);
        }
        else
        {
            y %= 8;
        }

        gceSURF_ROTATION dRot = GPERotationToGALRotation(pBltParms->pDst->Rotate());

        if ((mHwVersion == 0) && (DestRotate == gcvSURF_90_DEGREE))
        {
            gcmVERIFY(GetRelativeRotation(DestRotate, &dRot));
        }

        switch (dRot)
        {
        case gcvSURF_0_DEGREE:
            mBrush.mX = x;
            mBrush.mY = y;
            break;

        case gcvSURF_90_DEGREE:
            mBrush.mX = (8 - y) % 8;
            mBrush.mY = x;
            break;

        case gcvSURF_180_DEGREE:
            mBrush.mX = (8 - x) % 8;
            mBrush.mY = (8 - y) % 8;
            break;

        case gcvSURF_270_DEGREE:
            mBrush.mX = y;
            mBrush.mY = (8 - x) % 8;
            break;

        default:
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        gctUINT8 *pBrushBits = (gctUINT8*)pBrush->Buffer();
        if (stride < 0)
        {
            gctUINT8 *dst = (gctUINT8*)mBrush.mVirtualAddr + 0x100;
            gctINT n;

            stride = -stride;
            for (n = 0; n < 8; n++)
            {
                memcpy(dst + n * stride, pBrushBits - n * stride, stride);
            }

            pBrushBits = dst;
        }

        if (!TransBrush(mBrush.mVirtualAddr, pBrushBits, GetSurfBytesPerPixel(pBrush), dRot))
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        mBrush.mFormat = format;
        mBrush.mSolidEnable = gcvFALSE;
        mBrush.mType = GC2D_OBJ_BRUSH;
    }
    else
    {
        // color should be mask with the dst color mask, but it was not needed in our hardware
        mBrush.mSolidColor = pBltParms->solidColor;
        mBrush.mSolidEnable = gcvTRUE;
        mBrush.mFormat = mDstSurf.mFormat;
        mBrush.mConvert = gcvFALSE;
        mBrush.mType = GC2D_OBJ_BRUSH;
    }

    return gcvTRUE;

OnError:

    return gcvFALSE;
}

gctBOOL GC2DParms::SetMono(
        GPEBltParms * pBltParms,
        gctBOOL bSrc,
        GPESurf *Surf,
        gceSURF_ROTATION Rot,
        gctBOOL RopAAF0)
{
    if ((Surf->Format() != gpe1Bpp)
        || (Rot != gcvSURF_0_DEGREE))
    {
        return gcvFALSE;
    }

    // Reset Mono before using.
    gcmASSERT(mMono.mType == GC2D_OBJ_UNKNOWN);

    gcsRECT *rect = bSrc ? &mSrcRect : &mMonoRect;

    gcmASSERT((rect->bottom - rect->top)
           == (mDstRect.bottom - mDstRect.top));
    gcmASSERT((rect->right - rect->left)
           == (mDstRect.right - mDstRect.left));

    mMono.mWidth = GetSurfWidth(Surf);
    mMono.mHeight = GetSurfHeight(Surf);
    if (mMono.mWidth == 0)
    {
        mMono.mWidth = rect->right;
    }

    if (mMono.mHeight == 0)
    {
        mMono.mHeight = rect->bottom;
    }

    mMono.mStream = Surf->Buffer();
    mMono.mStride = Surf->Stride();
    if (mMono.mStride < 0)
    {
        gctUINT y, off = 0;
        gctUINT size;
        gctUINT8 *dst;
        gctUINT8 *src = (gctUINT8*)mMono.mStream;

        mMono.mStride = -mMono.mStride;

        size = mMono.mStride * mMono.mHeight;

        if (mMonoBufferSize < size)
        {
            if (mMonoBuffer)
            {
                delete [] mMonoBuffer;
            }

            mMonoBuffer = new UINT8[size];
            mMonoBufferSize = size;
        }

        dst = (gctUINT8*)mMonoBuffer;

        for (y = 0; y < mMono.mHeight; y++)
        {
            gcoOS_MemCopy(
                dst + off,
                src - off,
                mMono.mStride);

            off += mMono.mStride;
        }

        mMono.mStream = mMonoBuffer;
    }

    gcmASSERT(mMono.mWidth <= ((gctUINT32)mMono.mStride << 3));
    mMono.mWidth = mMono.mStride << 3;

    if (bSrc)
    {
        if (RopAAF0)
        {
            mTrsp = gcvSURF_SOURCE_MATCH;
            mMono.mTsColor = 0;
            mMono.mBgColor = 0;
            mMono.mFgColor = pBltParms->solidColor;
        }
        else
        {
            mTrsp = gcvSURF_OPAQUE;
            if (pBltParms->pLookup)
            {
                mMono.mBgColor = (pBltParms->pLookup)[0];
                mMono.mFgColor = (pBltParms->pLookup)[1];
            }
            else
            {
                return gcvFALSE;
            }

            if (pBltParms->pConvert)
            {
                mMono.mBgColor = (pBltParms->pColorConverter->*(pBltParms->pConvert))(mMono.mBgColor);
                mMono.mFgColor = (pBltParms->pColorConverter->*(pBltParms->pConvert))(mMono.mFgColor);
            }
        }
    }
    else
    {
        mTrsp = gcvSURF_SOURCE_MASK;
    }

    mMono.mType = GC2D_OBJ_MONO;

    return gcvTRUE;
}

gctBOOL GC2DParms::SetSurface(
        GPEBltParms * pBltParms,
        gctBOOL bSrc,
        GPESurf *pGPESurf,
        gceSURF_ROTATION Rotation)
{
    GC2D_ENTER_ARG("pBltParms = 0x%08p, bSrc = %d, pGPESurf = 0x%08p", pBltParms, bSrc, pGPESurf);

    if (pGPESurf == NULL)
    {
        GC2D_LEAVE();
        return gcvFALSE;
    }

    GC2DSurf *surf = bSrc ? &mSrcSurf : &mDstSurf;
    gcsRECT  *rect = bSrc ? &mSrcRect : &mDstRect;

    // Reset surfaces before using.
    gcmASSERT(surf->mType == GC2D_OBJ_UNKNOWN);

    do {
        if (ContainGC2DSurf(pGPESurf))
        {
            GC2DSurf *tmp = ((GC2D_SurfWrapper*)(pGPESurf))->mGC2DSurf;
            gcmASSERT(tmp->IsValid());

            surf->mFormat = tmp->mFormat;
            surf->mRotate = tmp->mRotate;
            surf->mWidth = tmp->mWidth;
            surf->mHeight = tmp->mHeight;
            surf->mStride = tmp->mStride;
            surf->mSize = tmp->mSize;

            surf->mVirtualAddr = tmp->mVirtualAddr;
            surf->mPhysicalAddr = tmp->mPhysicalAddr;
            surf->mDMAAddr = tmp->mDMAAddr;

            surf->mType = GC2D_OBJ_SURF;
            break;
        }

        gceSURF_FORMAT format = GPEFormatToGALFormat(pGPESurf->Format(), pGPESurf->FormatPtr());
        if (format == gcvSURF_UNKNOWN)
        {
            break;
        }

        gctUINT8 *bits = (gctUINT8*)pGPESurf->Buffer();
        gctINT32 stride = pGPESurf->Stride();
        gctINT32 width = GetSurfWidth(pGPESurf);
        gctINT32 height = GetSurfHeight(pGPESurf);

        gcmASSERT(bits);
        gcmASSERT(stride);

        if (stride < 0)
        {
            gctINT tmp;

            bits += stride * (height - 1);
            stride =- stride;

            switch (Rotation)
            {
            case gcvSURF_0_DEGREE:
            case gcvSURF_180_DEGREE:

                mMirrorY = !mMirrorY;

                tmp = rect->bottom;
                rect->bottom = height - rect->top;
                rect->top = height - tmp;

                if (!bSrc)
                {
                    tmp = mClipRect.bottom;
                    mClipRect.bottom = height - mClipRect.top;
                    mClipRect.top = height - tmp;
                }
                break;

            case gcvSURF_90_DEGREE:
            case gcvSURF_270_DEGREE:

                mMirrorX = !mMirrorX;

                tmp = rect->right;
                rect->right = height - rect->left;
                rect->left = height - tmp;

                if (!bSrc)
                {
                    tmp = mClipRect.right;
                    mClipRect.right = height - mClipRect.left;
                    mClipRect.left = height - tmp;
                }
                break;

            default:
                gcmASSERT(0);
            }
        }

        surf->mFormat = format;
        surf->mRotate = Rotation;
        surf->mWidth = width;
        surf->mHeight = height;
        surf->mStride = stride;
        surf->mSize = stride * height;
        surf->mVirtualAddr = bits;

        gctUINT32 phys = INVALID_PHYSICAL_ADDRESS;
        if (pGPESurf->InVideoMemory())
        {
            phys = pGPESurf->OffsetInVideoMemory() + mVideoPhyBase;
        }

        if ((mSrcSurf.mType == GC2D_OBJ_SURF) && (surf->mSize == mSrcSurf.mSize)
           && (bits == mSrcSurf.mVirtualAddr) && (phys == mSrcSurf.mPhysicalAddr))
        {
            surf->mWrapped = gcvFALSE;
            surf->mDMAAddr = mSrcSurf.mDMAAddr;
            surf->mPhysicalAddr = phys;
            surf->mType = GC2D_OBJ_SURF;
        }
        else
        {
            gcsUSER_MEMORY_DESC desc;
            gctUINT32 node, address;
            gceSTATUS status;

            gcoOS_ZeroMemory(&desc, sizeof(desc));

            desc.flag     = gcvALLOC_FLAG_USERMEMORY;
            desc.logical  = gcmPTR_TO_UINT64(bits);
            desc.physical = phys;
            desc.size     = surf->mSize;

            status = gcoHAL_WrapUserMemory(&desc, &node);
            if (gcmIS_ERROR(status))
            {
                GC2D_DebugTrace(GC2D_ERROR_TRACE,
                    ("%s(%d): Failed to wrap user memory: error=%d!\r\n"),
                    __FUNCTION__, __LINE__, status);

                GC2D_LEAVE();
                return gcvFALSE;
            }

            status = gcoHAL_LockVideoMemory(node, gcvFALSE, &address, gcvNULL);
            if (gcmIS_ERROR(status))
            {
                gcmVERIFY_OK(gcoHAL_ReleaseVideoMemory(node));

                GC2D_DebugTrace(GC2D_ERROR_TRACE,
                    ("%s(%d): Failed to lock video memory: error=%d!\r\n"),
                    __FUNCTION__, __LINE__, status);

                GC2D_LEAVE();
                return gcvFALSE;
            }

            surf->mDMAAddr      = address;
            surf->mWrapped      = gcvTRUE;
            surf->mWrappedNode  = node;
            surf->mPhysicalAddr = phys;
            surf->mType         = GC2D_OBJ_SURF;

            break;
        }
    } while (gcvFALSE);

    if (surf->mType == GC2D_OBJ_SURF)
    {
        if (bSrc && (surf->mFormat == gcvSURF_INDEX8))
        {
            GPEFormat *gpeFormatPtr = pGPESurf->FormatPtr();

            if (!gpeFormatPtr && (gpeFormatPtr->m_PaletteEntries == 256) && gpeFormatPtr->m_pPalette)
            {
                memcpy(mPalette, gpeFormatPtr->m_pPalette, 1024);

                mPaletteConvert = gcvTRUE;
            }
            else if (pBltParms->pLookup)
            {
                gctINT i;

                for (i = 0; i < 256; i++)
                {
                    if (pBltParms->pConvert)
                        mPalette[i] = (pBltParms->pColorConverter->*(pBltParms->pConvert))(pBltParms->pLookup[i]);
                    else
                        mPalette[i] = pBltParms->pLookup[i];
                }

                mPaletteConvert = gcvFALSE;
            }
            else
            {
                gcmASSERT(0);
                GC2D_LEAVE();
                return gcvFALSE;
            }
        }

        GC2D_LEAVE();
        return gcvTRUE;
    }
    else
    {
        GC2D_LEAVE();
        return gcvFALSE;
    }
}

gctBOOL GC2DParms::ResetSurface(gctBOOL bSrc)
{
    gctBOOL ret = gcvTRUE;
    GC2DSurf *surf = bSrc ? &mSrcSurf : &mDstSurf;
    gcsRECT  *rect = bSrc ? &mSrcRect : &mDstRect;

    if (surf->mWrapped)
    {
        gcmVERIFY_OK(gcoHAL_UnlockVideoMemory(
            surf->mWrappedNode,
            gcvSURF_BITMAP));

        gcmVERIFY_OK(gcoHAL_ReleaseVideoMemory(
            surf->mWrappedNode));

        surf->mWrapped = gcvFALSE;
        surf->mWrappedNode = 0;
    }

    surf->mFormat = gcvSURF_UNKNOWN;
    surf->mRotate = gcvSURF_0_DEGREE;
    surf->mWidth = 0;
    surf->mHeight = 0;
    surf->mStride = 0;
    surf->mSize = 0;
    surf->mVirtualAddr = gcvNULL;
    surf->mPhysicalAddr = INVALID_PHYSICAL_ADDRESS;
    surf->mDMAAddr = INVALID_PHYSICAL_ADDRESS;
    surf->mType = GC2D_OBJ_UNKNOWN;

    return ret;
}

gctBOOL GC2DParms::Reset()
{
    if (mBrush.mType != GC2D_OBJ_UNKNOWN)
    {
        mBrush.mType = GC2D_OBJ_UNKNOWN;
        mBrush.mConvert = gcvFALSE;
    }

    if (mDstSurf.mType != GC2D_OBJ_UNKNOWN)
    {
        if (!ResetSurface(gcvFALSE))
        {
            GC2D_DebugTrace(GC2D_ERROR_TRACE,
                ("%s(%d): Failed to reset dst surface!\r\n"),
                __FUNCTION__, __LINE__);
        }
    }

    if (mSrcSurf.mType != GC2D_OBJ_UNKNOWN)
    {
        if (!ResetSurface(gcvTRUE))
        {
            GC2D_DebugTrace(GC2D_ERROR_TRACE,
                ("%s(%d): Failed to reset src surface!\r\n"),
                __FUNCTION__, __LINE__);
        }
    }

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

    mBlit = BLIT_INVALID;

    mMono.mType = GC2D_OBJ_UNKNOWN;

    mTrsp = gcvSURF_OPAQUE;

    if (mABEnable)
    {
        gco2D_DisableAlphaBlend(mEngine2D);
        gco2D_SetPixelMultiplyModeAdvanced(
            mEngine2D,
            gcv2D_COLOR_MULTIPLY_DISABLE,
            gcv2D_COLOR_MULTIPLY_DISABLE,
            gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE,
            gcv2D_COLOR_MULTIPLY_DISABLE);

        mABEnable = gcvFALSE;
    }

    return gcvTRUE;
}

#define KERNEL_TABLE_SIZE 153

static void CalculateBilinearTable(gctUINT32 Factor, gctUINT16 * Kernel)
{
    gctUINT idx = 3 - gcmMIN(Factor >> 1, 3);
    gctINT i;

    memset(Kernel, 0, KERNEL_TABLE_SIZE * sizeof(gctUINT16));

    for (i = 0; i < 17; i++)
    {
        gctUINT16 *coef = Kernel + 9 * i;
        coef[idx] = 0x200 * (16 - i);
        coef[4] = 0x4000 - coef[idx];
    }
}

gctBOOL GC2DParms::Blit()
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT16 rop4 = mFgRop | (mBgRop << 8);
    gctBOOL setSrc = ROP4_USE_SOURCE(rop4);
    gctBOOL setBrh = ROP4_USE_PATTERN(rop4);

    if (!mDstSurf.IsValid())
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (mFgRop == mBgRop)
    {
        mTrsp = gcvSURF_OPAQUE;
    }

    switch (mTrsp)
    {
    case gcvSURF_OPAQUE:
        break;

    case gcvSURF_SOURCE_MATCH:
    case gcvSURF_SOURCE_MASK:
        if (mMono.mStride & 3)
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        setSrc = gcvTRUE;
        break;

    case gcvSURF_PATTERN_MASK:
        setBrh = gcvTRUE;
        break;

    default:
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    switch (mBlit)
    {
    case BLIT_NORMAL:
        break;

    case BLIT_MASKED_SOURCE:
    case BLIT_MONO_SOURCE:
    case BLIT_STRETCH:
        setSrc = gcvTRUE;
        break;

    case BLIT_FILTER:
        setSrc = gcvTRUE;
        setBrh = gcvFALSE;
        break;

    default:
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (mABEnable)
    {
        gcmONERROR(gco2D_EnableAlphaBlend(
            mEngine2D,
            mABSrcAg,
            mABDstAg,
            mABSrcAMode,
            mABDstAMode,
            mABSrcAgMode,
            mABDstAgMode,
            mABSrcFactorMode,
            mABDstFactorMode,
            mABSrcColorMode,
            mABDstColorMode
            ));
    }

    if (setSrc)
    {
        if ((mBlit == BLIT_NORMAL)
            || (mBlit == BLIT_STRETCH))
        {
            if (!mSrcSurf.IsValid())
            {
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }

            /* Configure color source. */
            gcmONERROR(gco2D_SetColorSourceEx(
                mEngine2D,
                mSrcSurf.mDMAAddr,
                mSrcSurf.mStride,
                mSrcSurf.mFormat,
                mSrcSurf.mRotate,
                mSrcSurf.mWidth,
                mSrcSurf.mHeight,
                gcvFALSE,
                mTrsp,
                mSrcTrspColor
                ));

            /* Set source rectangle size. */
            gcmONERROR(gco2D_SetSource(
                mEngine2D,
                &mSrcRect
                ));

            if (mSrcSurf.mFormat == gcvSURF_INDEX8)
            {
                gcmONERROR(gco2D_LoadPalette(
                    mEngine2D, 0, 256, mPalette, mPaletteConvert));
            }

            if (mBlit == BLIT_STRETCH)
            {
                /* Calculate the stretch factors. */
                gctUINT32 horFactor;
                gctUINT32 verFactor;

                gcmONERROR(gco2D_CalcStretchFactor(
                    mEngine2D,
                    mSrcRect.right - mSrcRect.left,
                    mDstRect.right - mDstRect.left,
                    &horFactor
                    ));

                gcmONERROR(gco2D_CalcStretchFactor(
                    mEngine2D,
                    mSrcRect.bottom - mSrcRect.top,
                    mDstRect.bottom - mDstRect.top,
                    &verFactor
                    ));

                /* Program the stretch factors. */
                gcmONERROR(gco2D_SetStretchFactors(
                    mEngine2D,
                    horFactor,
                    verFactor
                    ));
            }
        }
        else if ((mBlit == BLIT_MASKED_SOURCE)
            || (mBlit == BLIT_MONO_SOURCE))
        {
            if (!mMono.IsValid()
                || ((mMonoRect.right - mMonoRect.left) != (mDstRect.right - mDstRect.left))
                || ((mMonoRect.bottom - mMonoRect.top) != (mDstRect.bottom - mDstRect.top)))
            {
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }

            gctINT32 tx = mMonoRect.left & 0x1F;
            gctINT32 ty = 0;

            gctINT32 dx = mMonoRect.left - tx;
            gctINT32 dy = mMonoRect.top  - ty;

            mMono.mStream = (gctUINT8*)mMono.mStream + dy * mMono.mStride + (dx >> 3);
            mMonoRect.left = tx;
            mMonoRect.right -= dx;
            mMonoRect.top  = ty;
            mMonoRect.bottom -= dy;

            if (mBlit == BLIT_MASKED_SOURCE)
            {
                if (((mSrcRect.right - mSrcRect.left) != (mDstRect.right - mDstRect.left))
                    || ((mSrcRect.bottom - mSrcRect.top) != (mDstRect.bottom - mDstRect.top)))
                {
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }

                dx = mSrcRect.left - tx;
                dy = mSrcRect.top  - ty;

                gctUINT off = dy * mSrcSurf.mStride + (dx * GetFormatSize(mSrcSurf.mFormat));

                mSrcSurf.mVirtualAddr = (gctUINT8*)mSrcSurf.mVirtualAddr + off;
                mSrcSurf.mPhysicalAddr += off;
                mSrcSurf.mDMAAddr += off;
                mSrcRect.left = tx;
                mSrcRect.right -= dx;
                mSrcRect.top  = ty;
                mSrcRect.bottom -= dy;
                gcmASSERT(!(mSrcSurf.mDMAAddr & 0xF));

                if (mSrcSurf.mFormat == gcvSURF_INDEX8)
                {
                    gcmONERROR(gco2D_LoadPalette(
                        mEngine2D, 0, 256, mPalette, mPaletteConvert));
                }
            }
        }
    }
    else
    {
        /* Set dummy source. */
        gcmONERROR(gco2D_SetColorSourceEx(
            mEngine2D,
            mSrcBuffer->mDMAAddr,
            mSrcBuffer->mStride,
            mSrcBuffer->mFormat,
            mSrcBuffer->mRotate,
            mSrcBuffer->mWidth,
            mSrcBuffer->mHeight,
            gcvFALSE,
            mTrsp,
            mSrcTrspColor
            ));

        /* Set dummy source rectangle. */
        mSrcRect.left = mSrcRect.top
            = mSrcRect.right = mSrcRect.bottom = 0;
        gcmONERROR(gco2D_SetSource(
            mEngine2D,
            &mSrcRect
            ));
    }

    /* Set clipping rectangle. */
    mClipRect.left = gcmMAX(mClipRect.left, mDstRect.left);
    mClipRect.top = gcmMAX(mClipRect.top, mDstRect.top);
    mClipRect.right = gcmMIN(mClipRect.right, mDstRect.right);
    mClipRect.bottom = gcmMIN(mClipRect.bottom, mDstRect.bottom);
    if (mMirrorY)
    {
        gctINT tmp = mClipRect.bottom;
        mClipRect.bottom = mDstRect.top + mDstRect.bottom - mClipRect.top;
        mClipRect.top = mDstRect.top + mDstRect.bottom - tmp;
    }

    if (mMirrorX)
    {
        gctINT tmp = mClipRect.right;
        mClipRect.right = mDstRect.left + mDstRect.right - mClipRect.left;
        mClipRect.left = mDstRect.left + mDstRect.right - tmp;
    }

    gcmONERROR(gco2D_SetClipping(mEngine2D, &mClipRect));

    /* Set destination. */
    gcmONERROR(gco2D_SetTargetEx(
        mEngine2D,
        mDstSurf.mDMAAddr,
        mDstSurf.mStride,
        mDstSurf.mRotate,
        mDstSurf.mWidth,
        mDstSurf.mHeight
        ));

    /* Set mirrors. */
    gcmONERROR(gco2D_SetBitBlitMirror(
        mEngine2D,
        mMirrorX,
        mMirrorY
        ));

    /* Setup the brush if necessary. */
    if (setBrh)
    {
        /* Flush the brush. */
        if (mBrush.IsValid())
        {
            if (mBrush.mSolidEnable)
            {
                gcmONERROR(gco2D_LoadSolidBrush(
                    mEngine2D,
                    mBrush.mFormat,
                    mBrush.mConvert,
                    mBrush.mSolidColor,
                    0xFFFFFFFFFFFFFFFF));
            }
            else
            {
                gcmONERROR(gco2D_LoadColorBrush(
                    mEngine2D,
                    mBrush.mX,
                    mBrush.mY,
                    mBrush.mDMAAddr,
                    mBrush.mFormat,
                    0xFFFFFFFFFFFFFFFF));
            }
        }
        else
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    if (mBlit == BLIT_NORMAL)
    {
        gcmONERROR(gco2D_Blit(
            mEngine2D,
            1,
            &mDstRect,
            mFgRop,
            mBgRop,
            mDstSurf.mFormat
            ));
    }
    else if (mBlit == BLIT_STRETCH)
    {
        gcmONERROR(gco2D_StretchBlit(
            mEngine2D,
            1,
            &mDstRect,
            mFgRop,
            mBgRop,
            mDstSurf.mFormat
            ));
    }
    else if (mBlit == BLIT_FILTER)
    {
        gctUINT16 horKernelArray[KERNEL_TABLE_SIZE];
        gctUINT16 verKernelArray[KERNEL_TABLE_SIZE];
        gctBOOL hovPass = mSrcRect.right - mSrcRect.left != mDstRect.right - mDstRect.left;
        gctBOOL verPass = mSrcRect.bottom - mSrcRect.top != mDstRect.bottom - mDstRect.top;
        gctUINT32 horFactor = 0x10000;
        gctUINT32 verFactor = 0x10000;

        gcmONERROR(gco2D_SetFilterType(mEngine2D, gcvFILTER_USER));

        if (hovPass)
        {
            gcmONERROR(gco2D_CalcStretchFactor(
                    mEngine2D,
                    mSrcRect.right - mSrcRect.left,
                    mDstRect.right - mDstRect.left,
                    &horFactor
                    ));

            CalculateBilinearTable(horFactor >> 16, horKernelArray);

            gcmONERROR(gco2D_SetUserFilterKernel(
                mEngine2D,
                gcvFILTER_HOR_PASS,
                horKernelArray
                ));
        }

        if (verPass)
        {
            gcmONERROR(gco2D_CalcStretchFactor(
                    mEngine2D,
                    mSrcRect.bottom - mSrcRect.top,
                    mDstRect.bottom - mDstRect.top,
                    &verFactor
                    ));

            CalculateBilinearTable(verFactor >> 16, verKernelArray);

            gcmONERROR(gco2D_SetUserFilterKernel(
                mEngine2D,
                gcvFILTER_VER_PASS,
                verKernelArray
                ));
        }

        gcmONERROR(gco2D_SetStretchFactors(
            mEngine2D,
            horFactor,
            verFactor
            ));

        gcmONERROR(gco2D_EnableUserFilterPasses(
            mEngine2D,
            hovPass,
            verPass
            ));

        gcmONERROR(gco2D_FilterBlitEx(
            mEngine2D,
            mSrcSurf.mDMAAddr,
            mSrcSurf.mStride,
            INVALID_PHYSICAL_ADDRESS,
            0,
            INVALID_PHYSICAL_ADDRESS,
            0,
            mSrcSurf.mFormat,
            mSrcSurf.mRotate,
            mSrcSurf.mWidth,
            mSrcSurf.mHeight,
            &mSrcRect,
            mDstSurf.mDMAAddr,
            mDstSurf.mStride,
            mDstSurf.mFormat,
            mDstSurf.mRotate,
            mDstSurf.mWidth,
            mDstSurf.mHeight,
            &mDstRect,
            gcvNULL
            ));
    }
    else if ((mBlit == BLIT_MASKED_SOURCE) || (mBlit == BLIT_MONO_SOURCE))
    {
        gcsRECT destSubRect = mDstRect;
        gcsRECT monoRect = mMonoRect;

        gctINT32 maxHeight = (gco2D_GetMaximumDataCount() << 5) / gcmALIGN(monoRect.right, 32);
        gcsPOINT monoSize = {mMono.mWidth, mMono.mHeight};
        gctUINT8 *monoBase = (gctUINT8 *)mMono.mStream;
        gctUINT32 srcBase;

        /* Set source rectangle size. */
        gcmONERROR(gco2D_SetSource(
                    mEngine2D,
                    &monoRect
                    ));

        if (mBlit == BLIT_MASKED_SOURCE)
        {
            srcBase = mSrcSurf.mDMAAddr;
        }
        else
        {
            /* Program the source. */
            gcmONERROR(gco2D_SetMonochromeSource(
                mEngine2D,
                gcvFALSE,
                mMono.mTsColor,
                gcvSURF_UNPACKED,
                gcvFALSE,
                mTrsp,
                mMono.mFgColor,
                mMono.mBgColor
                ));
        }

        gcmASSERT(monoRect.top == 0);

        monoRect.left &= ~0x1F;
        monoRect.right = gcmALIGN(monoRect.right, 32);

        gctUINT32 line2render = monoRect.bottom;
        do {
            gcsRECT_PTR destMonoRect;
            gcsRECT     mirrorRect;

            if (monoRect.bottom > maxHeight)
            {
                monoRect.bottom = maxHeight;
            }

            destSubRect.bottom = destSubRect.top + monoRect.bottom;

            if (mBlit == BLIT_MASKED_SOURCE)
            {
                /* Configure masked source. */
                gcmONERROR(gco2D_SetMaskedSourceEx(
                    mEngine2D,
                    srcBase,
                    mSrcSurf.mStride,
                    mSrcSurf.mFormat,
                    gcvFALSE,
                    gcvSURF_UNPACKED,
                    mSrcSurf.mRotate,
                    mSrcSurf.mWidth,
                    mSrcSurf.mHeight
                    ));
            }

            if (mMirrorY)
            {
                mirrorRect.left = destSubRect.left;
                mirrorRect.right = destSubRect.right;
                mirrorRect.bottom = mDstRect.bottom - destSubRect.top + mDstRect.top;
                mirrorRect.top = mDstRect.bottom - destSubRect.bottom + mDstRect.top;
                destMonoRect = &mirrorRect;
            }
            else
            {
                destMonoRect = &destSubRect;
            }

            /* Do the blit. */
            gcmONERROR(gco2D_MonoBlit(
                mEngine2D,
                monoBase,
                &monoSize,
                &monoRect,
                gcvSURF_UNPACKED,
                gcvSURF_UNPACKED,
                destMonoRect,
                mFgRop, mBgRop,
                mDstSurf.mFormat
                ));

            line2render -= monoRect.bottom;

            if (line2render)
            {
                monoBase += monoRect.bottom * mMono.mStride;

                if (mBlit == BLIT_MASKED_SOURCE)
                {
                    srcBase += monoRect.bottom * mSrcSurf.mStride;
                }

                destSubRect.top = destSubRect.bottom;
                monoRect.bottom = line2render;
            }
        } while (line2render);
    }
    else
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    return gcvTRUE;

OnError:

    return gcvFALSE;
}

BOOL GC2DParms::ContainGC2DSurf(GPESurf *pGPESurf) const
{
    BOOL ret;

    GC2D_ENTER_ARG("pGPESurf = 0x%08p", pGPESurf);

    ret = ((GC2D_SurfWrapper*)(pGPESurf))->IsWrapped();

    GC2D_LEAVE_ARG("%d", ret);

    return ret;
}

BOOL GC2DParms::IsGPESurfSupported(GPESurf *pGPESurf, gctBOOL bSrc)
{
    GC2D_ENTER_ARG("pGPESurf = 0x%08p, bSrc = %d", pGPESurf, bSrc);

    do {
        if (!pGPESurf)
        {
            // Wrong parameter
            GC2D_BREAK(GC2D_NOT_SUPPORT);
        }

        // 1. GC2DSurf wrapped ?
        if (ContainGC2DSurf(pGPESurf))
        {
            return gcvTRUE;
        }

        // 2. rotation check
        int rotate = pGPESurf->Rotate();
        if ((rotate != DMDO_0) && (rotate != DMDO_90)
            && (rotate != DMDO_180) && (rotate != DMDO_270))
        {
            GC2D_BREAK(GC2D_NOT_SUPPORT);
        }

        // 3. width & height
        gctINT width  = GetSurfWidth(pGPESurf);
        gctINT height = GetSurfHeight(pGPESurf);
        if (width <= 0 || height <= 0)
        {
            GC2D_BREAK(GC2D_NOT_SUPPORT);
        }

        // too small?
        gctINT32 pixel = width * height;
        if (pixel < mMinPixelNum)
        {
            GC2D_BREAK(GC2D_NOT_SUPPORT);
        }

        // too large?
        if (pixel > mMaxPixelNum)
        {
            GC2D_BREAK(GC2D_NOT_SUPPORT);
        }

        // 4. format check
        EGPEFormat format = pGPESurf->Format();
        GPEFormat *gpeFormatPtr = pGPESurf->FormatPtr();
        if ((format == gpe16Bpp) || (format == gpe32Bpp))
        {
            if (GPEFormatToGALFormat(format, gpeFormatPtr) == gcvSURF_UNKNOWN)
            {
                GC2D_BREAK(GC2D_NOT_SUPPORT);
            }
        }
        else if (!bSrc || (format != gpe8Bpp))
        {
            GC2D_BREAK(GC2D_NOT_SUPPORT);
        }

        gcmASSERT(format == gpe8Bpp || format == gpe16Bpp || format == gpe32Bpp);

        // 5. Aligned?
        gctUINT8 *bits = (gctUINT8*)pGPESurf->Buffer();
        gctINT32 stride = pGPESurf->Stride();

        if (stride == 0)
        {
            GC2D_BREAK(GC2D_NOT_SUPPORT);
        }
        else if (stride < 0)
        {
            bits += stride * (height - 1);
            stride =- stride;
        }

        if (stride & mStrideMask)
        {
            GC2D_BREAK(GC2D_NOT_SUPPORT);
        }

        if (((gctUINT32)bits) & mAddrMask)
        {
            GC2D_BREAK(GC2D_NOT_SUPPORT);
        }

        GC2D_LEAVE_ARG("%d", gcvTRUE);

        return gcvTRUE;

    } while (gcvFALSE);

EXIT:
    GC2D_LEAVE_ARG("%d", gcvFALSE);

    return gcvFALSE;
}

gctBOOL GC2DParms::SetAlphaBlend(
        GPEBltParms * pBltParms)
{
        mABSrcAg = pBltParms->blendFunction.SourceConstantAlpha;
        mABDstAg = 0;
        mABSrcAMode = gcvSURF_PIXEL_ALPHA_STRAIGHT;
        mABDstAMode = gcvSURF_PIXEL_ALPHA_STRAIGHT;

        if (pBltParms->bltFlags & BLT_ALPHABLEND)
        {
            // Enable alphablend
            mABEnable = gcvTRUE;
        }
        else
        {
            // Disable alphablend
            mABEnable = gcvFALSE;

            return gcvTRUE;
        }

#if UNDER_CE >= 600
        // Negate source alpha if ALPHASRCNEG was requested
        if (pBltParms->blendFunction.BlendFlags & BLT_ALPHASRCNEG)
        {
            mABSrcAMode = gcvSURF_PIXEL_ALPHA_INVERSED;
            mABSrcAg = 0xFF - mABSrcAg;
        }
        // Negate destination alpha if ALPHADESTNEG was requested
        if (pBltParms->blendFunction.BlendFlags & BLT_ALPHADESTNEG)
        {
            mABDstAMode = gcvSURF_PIXEL_ALPHA_INVERSED;
        }
#endif

        if (pBltParms->blendFunction.AlphaFormat)
        {
            mABSrcAgMode = gcvSURF_GLOBAL_ALPHA_SCALE;
            mABDstAgMode = gcvSURF_GLOBAL_ALPHA_OFF;
#ifdef AC_SRC_ALPHA_NONPREMULT
            if (pBltParms->iMode == 1)
            {
                mABSrcColorMode = gcvSURF_COLOR_STRAIGHT;
            }
            else
            {
                mABSrcColorMode =
                    (pBltParms->blendFunction.AlphaFormat == AC_SRC_ALPHA_NONPREMULT) ?
                    gcvSURF_COLOR_STRAIGHT : gcvSURF_COLOR_MULTIPLY;
            }
#else
            mABSrcColorMode = gcvSURF_COLOR_MULTIPLY;
#endif
            mABDstColorMode = gcvSURF_COLOR_STRAIGHT;
            mABSrcFactorMode = gcvSURF_BLEND_ONE;
            mABDstFactorMode = gcvSURF_BLEND_INVERSED;
        }
        else
        {
            mABSrcAgMode = gcvSURF_GLOBAL_ALPHA_ON;
            mABDstAgMode = gcvSURF_GLOBAL_ALPHA_OFF;
            mABSrcColorMode = gcvSURF_COLOR_MULTIPLY;
            mABDstColorMode = gcvSURF_COLOR_STRAIGHT;
            mABSrcFactorMode = gcvSURF_BLEND_ONE;
            mABDstFactorMode = gcvSURF_BLEND_INVERSED;
        }

        return gcvTRUE;
}
