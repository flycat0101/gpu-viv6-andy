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

///////////////////////////////////////////////////////////////////////
// check the pBltParms
///////////////////////////////////////////////////////////////////////
SCODE GC2DParms::BltParmsFilter(GPEBltParms *pBltParms)
{
    GC2D_ENTER_ARG("pBltParms = 0x%08p", pBltParms);

    do {
        ///////////////////////////////////////////////////////////////////////
        // check flags
        ///////////////////////////////////////////////////////////////////////
        if (pBltParms->bltFlags & BLT_STRETCH)
        {
            if ((pBltParms->iMode == BILINEAR) && (pBltParms->rop4 != 0xCCCC))
            {
                GC2D_BREAK(GC2D_NOT_SUPPORT);
            }

#if UNDER_CE >= 600
            if (pBltParms->iMode == HALFTONE)
            {
                GC2D_BREAK(GC2D_NOT_SUPPORT);
            }
#endif
        }

        if ((pBltParms->bltFlags & BLT_TRANSPARENT)
            && (pBltParms->rop4 != 0xCCCC))
        {
            GC2D_BREAK(GC2D_NOT_SUPPORT);
        }

        ///////////////////////////////////////////////////////////////////////
        // check dst parameters
        ///////////////////////////////////////////////////////////////////////
        GPESurf* pGPEDst = (GPESurf*) pBltParms->pDst;
        GPESurf* pGPESrc = (GPESurf*) pBltParms->pSrc;
        GPESurf* pGPEMask = (GPESurf*) pBltParms->pMask;
        GPESurf* pGPEBrush = (GPESurf*) pBltParms->pBrush;

#if GC2D_GATHER_STATISTICS
        if (pGPEDst)
        {
            gssDst.RecordSurf(pGPEDst);

            GC2D_DebugTrace(GC2D_DISPLAY_BLIT, ("GC2DBltParmsFilter: " \
                "Dest Width=%d Height=%d Format=%d Stride=%d GPESurf=%08X\r\n"),
                    pGPEDst->Width(), pGPEDst->Height(),
                    pGPEDst->Format(), pGPEDst->Stride(),
                    pGPEDst);
        }

        if (pGPESrc)
        {
            gssSrc.RecordSurf(pGPESrc);

            GC2D_DebugTrace(GC2D_DISPLAY_BLIT, ("GC2DBltParmsFilter: "\
                "Src Width=%d Height=%d Format=%d Stride=%d GPESurf=%08X\r\n"),
                    pGPESrc->Width(), pGPESrc->Height(),
                    pGPESrc->Format(), pGPESrc->Stride(),
                    pGPESrc);
        }

        if (pGPEMask)
        {
            gssMask.RecordSurf(pGPEMask);

            GC2D_DebugTrace(GC2D_DISPLAY_BLIT, ("GC2DBltParmsFilter: "\
                "Mask Width=%d Height=%d Format=%d Stride=%d GPESurf=%08X\r\n"),
                    pGPEMask->Width(), pGPEMask->Height(),
                    pGPEMask->Format(), pGPEMask->Stride(),
                    pGPEMask);
        }

        if (pGPEBrush)
        {
            gssBrush.RecordSurf(pGPEBrush);

            GC2D_DebugTrace(GC2D_DISPLAY_BLIT, ("GC2DBltParmsFilter: "\
                "Brush Width=%d Height=%d Format=%d Stride=%d GPESurf=%08X\r\n"),
                    pGPEBrush->Width(), pGPEBrush->Height(),
                    pGPEBrush->Format(), pGPEBrush->Stride(),
                    pGPEBrush);
        }
#endif
        // check whether dst surface is right and can be wrapped
        if (!pGPEDst)
        {
            GC2D_BREAK(GC2D_NOT_SUPPORT);
        }
        else if (!IsGPESurfSupported(pGPEDst, gcvFALSE))
        {
            GC2D_BREAK(GC2D_NOT_SUPPORT);
        }

        gceSURF_ROTATION dRot = GPERotationToGALRotation(pGPEDst->Rotate());
        if (dRot == -1)
        {
            GC2D_BREAK(GC2D_NOT_SUPPORT);
        }

        ///////////////////////////////////////////////////////////////////////
        // check src parameters
        ///////////////////////////////////////////////////////////////////////
        gctUINT32 Rop4 = pBltParms->rop4;
        gctUINT8 fRop = (gctUINT8)(Rop4 & 0x00FF);
        gctUINT8 bRop = (gctUINT8)((Rop4 >> 8) & 0x00FF);
        gctBOOL UseSource = ROP4_USE_SOURCE(Rop4);
        gceSURF_ROTATION sRot;

        // alphablend related check
        if (pBltParms->bltFlags & BLT_ALPHABLEND)
        {
            // Src will be refered anyway
            if(!pGPESrc)
            {
                GC2D_BREAK(GC2D_NOT_SUPPORT);
            }

            UseSource = gcvTRUE;

            if (!mHwVersion)
            {
                // PE1.0 format limitation
                if (Rop4 != 0xCCCC)
                {
                    GC2D_BREAK(GC2D_NOT_SUPPORT);
                }

                if (pBltParms->blendFunction.AlphaFormat &&
                    (gcvSURF_R5G6B5 == GPEFormatToGALFormat(pGPEDst->Format(), pGPEDst->FormatPtr())))
                {
                    GC2D_BREAK(GC2D_NOT_SUPPORT);
                }
            }
        }

        // check whether src surface is right and can be wrapped
        if (UseSource)
        {
            if (!pGPESrc || (pGPESrc->Stride() == 0))
            {
                GC2D_BREAK(GC2D_NOT_SUPPORT);
            }

            sRot = GPERotationToGALRotation(pGPESrc->Rotate());
            if (sRot == -1)
            {
                GC2D_BREAK(GC2D_NOT_SUPPORT);
            }

            if (pGPESrc->Format() == gpe1Bpp)
            {
                if (mMonoSupported)
                {
                    if (pBltParms->bltFlags & BLT_STRETCH)
                    {
                        GC2D_BREAK(GC2D_NOT_SUPPORT);
                    }

                    if (!mHwMirrorEx && pGPEDst->Stride() <= 0)
                    {
                        GC2D_BREAK(GC2D_NOT_SUPPORT);
                    }

                    if (!mHwVersion)
                    {
                        gceSURF_ROTATION rot = dRot;
                        if (!GetRelativeRotation(sRot, &rot))
                        {
                            GC2D_BREAK(GC2D_NOT_SUPPORT);
                        }

                        if ((rot != gcvSURF_0_DEGREE)
                            || (rot != gcvSURF_90_DEGREE))
                        {
                            // PE 1.0 not support
                            GC2D_BREAK(GC2D_NOT_SUPPORT);
                        }
                    }
                }
                else
                {
                    // not support mono.
                    GC2D_BREAK(GC2D_NOT_SUPPORT);
                }
            }
            else if (!IsGPESurfSupported(pGPESrc, gcvTRUE))
            {
                GC2D_BREAK(GC2D_NOT_SUPPORT);
            }
        }

        ///////////////////////////////////////////////////////////////////////
        // check mask parameters
        ///////////////////////////////////////////////////////////////////////
        gctBOOL UseMask = ROP4_USE_MASK(Rop4);

        // check the mask surfce
        if (UseMask)
        {
            if (!(mEnableFlag & GC2D_FEATURE_ENABLE_MASK))
            {
                GC2D_BREAK(GC2D_NOT_SUPPORT);
            }

            if (!pGPEMask || (!mHwMirrorEx && pGPEDst->Stride() <= 0))
            {
                GC2D_BREAK(GC2D_NOT_SUPPORT);
            }

            gceSURF_ROTATION mRot = GPERotationToGALRotation(pGPEMask->Rotate());
            if (mRot == -1)
            {
                GC2D_BREAK(GC2D_NOT_SUPPORT);
            }

            if ((pGPEMask->Format() != gpe1Bpp)
                || (pBltParms->bltFlags & BLT_STRETCH))
            {
                GC2D_BREAK(GC2D_NOT_SUPPORT);
            }

            int stride = abs(pGPEMask->Stride());
            if ((stride == 0) || (stride & 3))
            {
                GC2D_BREAK(GC2D_NOT_SUPPORT);
            }

            if (UseSource)
            {
                if ((pGPESrc->Format() == gpe1Bpp)
                   || (!mHwMirrorEx && pGPESrc->Stride() <= 0))
                {
                    GC2D_BREAK(GC2D_NOT_SUPPORT);
                }

                if ((pBltParms->xPositive < 1) || (pBltParms->yPositive < 1))
                {
                    GC2D_BREAK(GC2D_NOT_SUPPORT);
                }

                if (!mHwVersion && (pGPESrc->Stride() <= 0))
                {
                    // PE1.0 not support
                    GC2D_BREAK(GC2D_NOT_SUPPORT);
                }

                if (mRot != sRot)
                {
                    GC2D_BREAK(GC2D_NOT_SUPPORT);
                }
            }
            else if (!mMonoSupported)
            {
                GC2D_BREAK(GC2D_NOT_SUPPORT);
            }

            if (!mHwVersion)
            {
                gceSURF_ROTATION rot = dRot;
                if (!GetRelativeRotation(mRot, &rot))
                {
                    GC2D_BREAK(GC2D_NOT_SUPPORT);
                }

                if ((rot != gcvSURF_0_DEGREE)
                    && (rot != gcvSURF_90_DEGREE))
                {
                    // PE 1.0 not support
                    GC2D_BREAK(GC2D_NOT_SUPPORT);
                }

                if (pGPEDst->Stride() <= 0)
                {
                    // PE 1.0 not support
                    GC2D_BREAK(GC2D_NOT_SUPPORT);
                }
            }
        }

        ///////////////////////////////////////////////////////////////////////
        // check brush parameters
        ///////////////////////////////////////////////////////////////////////
        gctBOOL UsePattern = ROP4_USE_PATTERN(Rop4);
        if (UsePattern)
        {
            GPESurf *pBrush = pBltParms->pBrush;

            if(pBrush)
            {
                if ((pBrush->Format() == gpe1Bpp)
                    || (pBrush->Format() == gpe24Bpp)
                    || (pBrush->IsRotate()))
                {
                    GC2D_BREAK(GC2D_NOT_SUPPORT);
                }

                gctINT stride = abs(pBrush->Stride());
                gceSURF_FORMAT format = GPEFormatToGALFormat(pBrush->Format(), pBrush->FormatPtr());

                if ((format == gcvSURF_UNKNOWN)
                    || (format == gcvSURF_INDEX8)
                    || (stride == 0))
                {
                    GC2D_BREAK(GC2D_NOT_SUPPORT);
                }

                if ((pBrush->Width() != 8) || (pBrush->Height() != 8)
                    || (stride != (GetSurfBytesPerPixel(pBrush) * 8)))
                {
                    GC2D_BREAK(GC2D_NOT_SUPPORT);
                }
            }
        }

        GC2D_LEAVE_ARG("%d", S_OK);
        return(S_OK);

    } while (gcvFALSE);

EXIT:
    GC2D_LEAVE_ARG("%d", -ERROR_NOT_SUPPORTED);

    return -ERROR_NOT_SUPPORTED;
}

SCODE GC2DParms::BltPrepare(GPEBltParms *pBltParms)
{
    SCODE sc = -ERROR_NOT_SUPPORTED;
    GC2D_ENTER_ARG("pBltParms = 0x%08p", pBltParms);

    do {
        ///////////////////////////////////////////////////////////////////////
        // check GC2D object
        ///////////////////////////////////////////////////////////////////////
        if (!IsValid())
        {
            GC2D_BREAK(GC2D_NOT_SUPPORT);
        }

        // if disable the prepare
        if (!(mEnableFlag & GC2D_FEATURE_ENABLE_BLTPREPARE))
        {
            GC2D_BREAK(GC2D_NOT_SUPPORT);
        }

#if GC2D_GATHER_STATISTICS

        if (!(nPrepare % 400))
        {
            RETAILMSG(1, (TEXT("GC2D Stat: Prepare:(%d/%d), ROP:(%d/%d), Alpha:(%d/%d), Sync:(-%d/%d)\n"),
                nPrepareSucceed,nPrepare, nROPBlitSucceed, nROPBlit, nAlphaBlendSucceed, nAlphaBlend,
                nASyncCommitFailed, nASyncCommit));
        }

        ++nPrepare;
#endif

        ///////////////////////////////////////////////////////////////////////
        // check bltFlags
        ///////////////////////////////////////////////////////////////////////
        if (pBltParms->bltFlags & (~(
            BLT_ALPHABLEND | BLT_STRETCH | BLT_TRANSPARENT
            #if UNDER_CE >= 600
            | BLT_WAITNOTBUSY
            #endif
            )))
        {
            GC2D_BREAK(GC2D_NOT_SUPPORT);
        }

        if ((pBltParms->bltFlags & BLT_STRETCH)
            && (!(mEnableFlag & GC2D_FEATURE_ENABLE_STRETCH)))
        {
            GC2D_BREAK(GC2D_NOT_SUPPORT);
        }

        if ((pBltParms->bltFlags & BLT_TRANSPARENT)
            && (!(mEnableFlag & GC2D_FEATURE_ENABLE_TRANSPARENT)))
        {
            GC2D_BREAK(GC2D_NOT_SUPPORT);
        }

        if ((pBltParms->bltFlags & BLT_ALPHABLEND)
            && (!(mEnableFlag & GC2D_FEATURE_ENABLE_ALPHABLEND)))
        {
            GC2D_BREAK(GC2D_NOT_SUPPORT);
        }

        sc = BltParmsFilter(pBltParms);

    } while (gcvFALSE);

EXIT:
    if (sc == S_OK)
    {
#if GC2D_GATHER_STATISTICS
        nPrepareSucceed++;
#endif
        GC2D_DebugTrace(GC2D_DISPLAY_BLIT, ("GC2DBltPrepare: AnyBlt is selected\n"));
        pBltParms->pBlt = GPEBLTFUN (&GC2D_Accelerator::AnyBlt);

        GC2D_LEAVE_ARG("%d", S_OK);
        return S_OK;
    }
    else
    {
        GC2D_LEAVE_ARG("%d", sc);
        return sc;
    }
}

SCODE GC2DParms::BltComplete(GPEBltParms *pBltParms)
{
    SCODE sc = GC2D_NOT_MY_BLIT;
    GC2D_ENTER_ARG("pBltParms = 0x%08p", pBltParms);

    if (pBltParms->pBlt == &GC2D_Accelerator::AnyBlt)
    {
        if (mSyncMode == GC2D_SYNC_MODE_ASYNC)
        {
            if (IsValid())
            {
                WaitForNotBusy();
            }
        }

        sc = GC2D_SUCCESS;
    }

    GC2D_LEAVE_ARG("%d", sc);
    return sc;
}

SCODE GC2DParms::Blit(GPEBltParms * pBltParms)
{
    gctBOOL bSucceed = gcvFALSE;
    gctBOOL bForce = gcvTRUE;

    GC2D_ENTER_ARG("pBltParms = 0x%08p", pBltParms);

    WaitForNotBusy();
    Reset();

    do {
        GPESurf* pGPESrc = (GPESurf*) pBltParms->pSrc;
        GPESurf* pGPEDst = (GPESurf*) pBltParms->pDst;
        GPESurf* pGPEMask = (GPESurf*) pBltParms->pMask;
        // NOTE: the struct RECTL should be the same with gcsRECT
        gcsRECT_PTR pDstRect = (gcsRECT_PTR)pBltParms->prclDst;
        gcsRECT_PTR pSrcRect = (gcsRECT_PTR)pBltParms->prclSrc;
        gcsRECT_PTR pMskRect = (gcsRECT_PTR)pBltParms->prclMask;
        gcsRECT_PTR pClpRect = (gcsRECT_PTR)pBltParms->prclClip;

        gceSURF_ROTATION dRot, sRot, mRot;
        dRot = sRot = mRot = (gceSURF_ROTATION) -1;

        gctUINT32 Rop4 = pBltParms->rop4;
        mFgRop = (gctUINT8)(Rop4 & 0xFF);
        mBgRop = (gctUINT8)((Rop4 >> 8) & 0xFF);

        gctBOOL UseSource  = ROP4_USE_SOURCE(Rop4);
        gctBOOL UsePattern = ROP4_USE_PATTERN(Rop4);
        gctBOOL UseMask    = ROP4_USE_MASK(Rop4);

        gctBOOL OptMask = (!pBltParms->pBrush)
            && ((Rop4 == 0xAAF0) || (Rop4 == 0x55F0)
             || (Rop4 == 0x00F0) || (Rop4 == 0xFFF0));
        if (OptMask)
        {
            Rop4 = 0xAACC;
            mFgRop = 0xCC;
            UsePattern = UseMask = gcvFALSE;
            UseSource = gcvTRUE;
            pGPESrc = pGPEMask;
            pGPEMask = gcvNULL;
            pSrcRect = pMskRect;
            pMskRect = gcvNULL;
            mBlit = BLIT_MONO_SOURCE;
        }

        gcmASSERT(pGPEDst);

        // 1. Set Rects & blit type
        if (pDstRect)
        {
            if (pDstRect->left < 0 || pDstRect->top < 0
                || pDstRect->right <= 0 || pDstRect->bottom <= 0)
            {
                GC2D_BREAK(GC2D_NOT_SUPPORT);
            }

            mDstRect = *pDstRect;
        }
        else
        {
            mDstRect.left = 0;
            mDstRect.top = 0;
            mDstRect.right = pGPEDst->Width();
            mDstRect.bottom = pGPEDst->Height();
        }

        gctINT32 dwRect = mDstRect.right - mDstRect.left;
        gctINT32 dhRect = mDstRect.bottom - mDstRect.top;
        gctINT32 pixel = dwRect * dhRect;
        if ((dwRect <= 0) || (dhRect <= 0) || (pixel < mMinPixelNum))
        {
            GC2D_BREAK(GC2D_FORWARD_TO_SW);
        }

        if (!(pBltParms->bltFlags & BLT_ALPHABLEND))
        {
            if (((Rop4 == 0xF0F0) || (Rop4 == 0xFFFF) || (Rop4 == 0x0))
                && (pixel < mMinSolidFillPixelNum))
            {
                GC2D_BREAK(GC2D_FORWARD_TO_SW);
            }

            if ((!UsePattern) && (!UseMask)
                && (pixel < mMinSrcCopyPixelNum))
            {
                GC2D_BREAK(GC2D_FORWARD_TO_SW);
            }

            if (UseMask
                && (pixel < mMinMaskPixelNum))
            {
                GC2D_BREAK(GC2D_FORWARD_TO_SW);
            }
        }

        if (pClpRect)
        {
            if (pClpRect->left < 0 || pClpRect->top < 0
                || pClpRect->right <= 0 || pClpRect->bottom <= 0)
            {
                GC2D_BREAK(GC2D_NOT_SUPPORT);
            }

            mClipRect = *pClpRect;
        }
        else
        {
            mClipRect = mDstRect;
        }

        gctINT32 swRect, shRect;
        if (UseSource)
        {
            gcmASSERT(pGPESrc);

            if (pSrcRect)
            {
                if (pSrcRect->left < 0 || pSrcRect->top < 0
                    || pSrcRect->right <= 0 || pSrcRect->bottom <= 0)
                {
                    GC2D_BREAK(GC2D_NOT_SUPPORT);
                }

                mSrcRect = *pSrcRect;
            }
            else
            {
                mSrcRect.left = 0;
                mSrcRect.top = 0;
                mSrcRect.right = pGPESrc->Width();
                mSrcRect.bottom = pGPESrc->Height();
            }

            swRect = mSrcRect.right - mSrcRect.left;
            shRect = mSrcRect.bottom - mSrcRect.top;
            if ((swRect <= 0) || (shRect <= 0))
            {
                GC2D_BREAK(GC2D_NOT_SUPPORT);
            }

            if (pGPESrc->Format() == gpe1Bpp)
            {
                gcmASSERT(!UseMask);

                if ((pBltParms->bltFlags & BLT_STRETCH)
                    || (swRect != dwRect)
                    || (shRect != dhRect))
                {
                    GC2D_BREAK(GC2D_NOT_SUPPORT);
                }

                mBlit = BLIT_MONO_SOURCE;
            }
            else
            {
                if ((swRect != dwRect)
                    || (shRect != dhRect))
                {
                    if ((pBltParms->iMode == BLACKONWHITE)
                        && ((swRect > dwRect) || (shRect > dhRect)))
                    {
                        GC2D_BREAK(GC2D_NOT_SUPPORT);
                    }

                    if (pBltParms->iMode == BILINEAR)
                    {
                        mBlit = BLIT_FILTER;
                    }
                    else
                    {
                        mBlit = BLIT_STRETCH;
                    }
                }
                else
                {
                    mBlit = BLIT_NORMAL;
                }
            }

            sRot = GPERotationToGALRotation(pGPESrc->Rotate());
        }
        else
        {
            swRect = shRect = -1;
            mBlit = BLIT_NORMAL;
        }

        gctINT32 mwRect = -1;
        gctINT32 mhRect = -1;
        if (UseMask)
        {
            gcmASSERT(pGPEMask);

            if (!pMskRect)
            {
                GC2D_BREAK(GC2D_NOT_SUPPORT);
            }
            else if (pMskRect->left < 0 || pMskRect->top < 0
            || pMskRect->right <= 0 || pMskRect->bottom <= 0)
            {
                GC2D_BREAK(GC2D_NOT_SUPPORT);
            }

            mwRect = pMskRect->right - pMskRect->left;
            mhRect = pMskRect->bottom - pMskRect->top;
            if ((mwRect <= 0) || (mhRect <= 0))
            {
                GC2D_BREAK(GC2D_NOT_SUPPORT);
            }

            if ((pGPEMask->Format() != gpe1Bpp)
                || (pBltParms->bltFlags & BLT_STRETCH)
                || (mwRect != dwRect)
                || (mhRect != dhRect))
            {
                GC2D_BREAK(GC2D_NOT_SUPPORT);
            }

            int stride = abs(pGPEMask->Stride());
            if ((stride == 0) || (stride & 3))
            {
                GC2D_BREAK(GC2D_NOT_SUPPORT);
            }

            int MaskWidth = pGPEMask->Width();
            int MaskHeight = pGPEMask->Height();

            if (MaskWidth == 0)
            {
                MaskWidth = pMskRect->right;
            }

            if (MaskHeight == 0)
            {
                MaskHeight = pMskRect->bottom;
            }

            if (MaskWidth > (stride << 3))
            {
                GC2D_BREAK(GC2D_NOT_SUPPORT);
            }

            MaskWidth = stride << 3;
            AlignWidthHeight((gctUINT32*)&MaskWidth, (gctUINT32*)&MaskHeight);
            if ((gcmALIGN(MaskWidth * MaskHeight, 8) >> 3) == 0)
            {
                GC2D_BREAK(GC2D_NOT_SUPPORT);
            }

            mBlit = BLIT_MASKED_SOURCE;
            mMonoRect = *pMskRect;

            mRot = GPERotationToGALRotation(pGPEMask->Rotate());
            if (UseSource)
            {
                gcmASSERT((mHwVersion > 0 || pGPESrc->Stride() > 0));

                if ((pGPESrc->Format() == gpe1Bpp)
                    || (mRot != sRot)
                    || (mhRect != shRect)
                    || (mwRect != swRect))
                {
                    GC2D_BREAK(GC2D_NOT_SUPPORT);
                }
            }
            else
            {
                sRot = mRot;
            }
        }

        // 2. Set rotations
        dRot = GPERotationToGALRotation(pGPEDst->Rotate());

        if (UseSource || UseMask)
        {
            gceSURF_ROTATION dRot2 = dRot;

            gcmVERIFY((GetRelativeRotation(sRot, &dRot2)));

            if ((mBlit == BLIT_MONO_SOURCE)
                || (mBlit == BLIT_MASKED_SOURCE))
            {
                if (!mHwVersion
                    && (dRot2 != gcvSURF_0_DEGREE)
                    && (dRot2 != gcvSURF_90_DEGREE))
                {
                    GC2D_BREAK(GC2D_NOT_SUPPORT);
                }

                if (UseSource)
                {
                    gcmASSERT(!UseMask || (mRot == sRot));
                    gcmVERIFY(TransRect(
                        &mSrcRect,
                        sRot, gcvSURF_0_DEGREE,
                        GetSurfWidth(pGPESrc),
                        GetSurfHeight(pGPESrc)));

                    sRot = gcvSURF_0_DEGREE;
                }

                if (UseMask)
                {
                    gcmVERIFY(TransRect(
                        &mMonoRect,
                        mRot, gcvSURF_0_DEGREE,
                        GetSurfWidth(pGPEMask),
                        GetSurfHeight(pGPEMask)));

                    mRot = gcvSURF_0_DEGREE;
                }
            }
            else
            {
                gcmASSERT(!UseMask);
                gcmASSERT(UseSource);

                gceSURF_ROTATION sRot2;

                switch (dRot2)
                {
                case gcvSURF_0_DEGREE:
                    sRot2 = dRot2 = gcvSURF_0_DEGREE;
                    break;

                case gcvSURF_90_DEGREE:
                    sRot2 = gcvSURF_0_DEGREE;
                    dRot2 = gcvSURF_90_DEGREE;
                    break;

                case gcvSURF_180_DEGREE:
                    sRot2 = gcvSURF_0_DEGREE;
                    dRot2 = gcvSURF_180_DEGREE;
                    break;

                case gcvSURF_270_DEGREE:
                    sRot2 = gcvSURF_90_DEGREE;
                    dRot2 = gcvSURF_0_DEGREE;
                    break;

                default:
                    GC2D_BREAK(GC2D_NOT_SUPPORT);
                }

                gcmVERIFY(TransRect(
                    &mSrcRect,
                    sRot, sRot2,
                    GetSurfWidth(pGPESrc),
                    GetSurfHeight(pGPESrc)));

                sRot = sRot2;
            }

            if (!mHwVersion && (dRot2 == gcvSURF_180_DEGREE))
            {
                dRot2 = gcvSURF_0_DEGREE;
                mMirrorX = !mMirrorX;
                mMirrorY = !mMirrorY;
            }

            gcmVERIFY(TransRect(
                    &mDstRect,
                    dRot, dRot2,
                    GetSurfWidth(pGPEDst),
                    GetSurfHeight(pGPEDst)));

            gcmVERIFY(TransRect(
                    &mClipRect,
                    dRot, dRot2,
                    GetSurfWidth(pGPEDst),
                    GetSurfHeight(pGPEDst)));

            dRot = dRot2;
        }
        else
        {
            gcmVERIFY(TransRect(
                    &mDstRect,
                    dRot, gcvSURF_0_DEGREE,
                    GetSurfWidth(pGPEDst),
                    GetSurfHeight(pGPEDst)));

            gcmVERIFY(TransRect(
                    &mClipRect,
                    dRot, gcvSURF_0_DEGREE,
                    GetSurfWidth(pGPEDst),
                    GetSurfHeight(pGPEDst)));

            dRot = gcvSURF_0_DEGREE;
        }

        // 3. Set src if necessary
        if (UseSource)
        {
            if (pGPESrc->Format() == gpe1Bpp)
            {
                gcmASSERT(!UseMask);

                if (!SetMono(
                        pBltParms,
                        gcvTRUE,
                        pGPESrc,
                        sRot,
                        OptMask))
                {
                    GC2D_BREAK(GC2D_NOT_SUPPORT);
                }

                mMonoRect = mSrcRect;
            }
            else if (!SetSurface(pBltParms, gcvTRUE, pGPESrc, sRot))
            {
                GC2D_BREAK(GC2D_NOT_SUPPORT);
            }

            if(pBltParms->bltFlags & BLT_TRANSPARENT)
            {
                gcmASSERT(!UseMask);

                // BUGBUG: BLT_TRANSPARENT with a wrong ROP4 = 0xCCCC by GDI
                //            Change the bRop from 0xCC to 0xAA to fix it up
                Rop4 = 0xAACC;
                mBgRop = 0xAA;
                mTrsp = gcvSURF_SOURCE_MATCH;
                mMono.mTsColor =
                mSrcTrspColor = pBltParms->solidColor;
            }
        }

        // 3. Set mask if necessary
        if (UseMask)
        {
            if (!SetMono(
                    pBltParms,
                    gcvFALSE,
                    pGPEMask,
                    mRot,
                    OptMask))
            {
                GC2D_BREAK(GC2D_NOT_SUPPORT);
            }

            if (!UseSource)
            {
                gcmASSERT(mSrcSurf.mType == GC2D_OBJ_UNKNOWN);

                mSrcRect = mMonoRect;

                // suppose the foreground pixels were less
                mFgRop |= 0xCC;
                if (!((mFgRop ^ (mFgRop >> 2)) & 0x33))
                {
                    mBgRop |= 0xCC;
                    gcmASSERT((mBgRop ^ ( mBgRop >> 2)) & 0x33);
                }

                // src must have alpha channel and its rotate must be 0 degree
                mSrcSurf.mFormat = gcvSURF_A1R5G5B5;
                mSrcSurf.mRotate = gcvSURF_0_DEGREE;
                mSrcSurf.mWidth = mMono.mWidth;
                mSrcSurf.mHeight = mMono.mHeight;
                mSrcSurf.mStride = mMono.mWidth * 2;
                mSrcSurf.mSize = mSrcSurf.mStride * mSrcSurf.mHeight;
                gcmASSERT(mSrcSurf.mSize <= mSrcBuffer->mSize);
                mSrcSurf.mVirtualAddr = mSrcBuffer->mVirtualAddr;
                mSrcSurf.mPhysicalAddr = mSrcBuffer->mPhysicalAddr;
                mSrcSurf.mDMAAddr = mSrcBuffer->mDMAAddr;
                mSrcSurf.mType = GC2D_OBJ_SURF;
            }

            if (mBlit == BLIT_MASKED_SOURCE)
            {

                if (abs(mSrcRect.left - mMonoRect.left)
                    % (16 / GetFormatSize(mSrcSurf.mFormat)))
                {
                    GC2D_BREAK(GC2D_NOT_SUPPORT);
                }
            }
        }

        // 4. Set dst
        if (!SetSurface(pBltParms, gcvFALSE, pGPEDst, dRot))
        {
            GC2D_BREAK(GC2D_NOT_SUPPORT);
        }

        // 5. Set brush if necessary
        if (UsePattern)
        {
            if (!SetBrush(pBltParms, dRot))
            {
                GC2D_BREAK(GC2D_NOT_SUPPORT);
            }
        }

        // 6. Set Alphablend
        if (!SetAlphaBlend(pBltParms))
        {
            GC2D_BREAK(GC2D_NOT_SUPPORT);
        }

#if GC2D_GATHER_STATISTICS
        if (mABEnable)
        {
            ++nAlphaBlend;
        }
        else
        {
            ++nROPBlit;
        }
#endif
        // Wait for VSync.
        mGPE2D->DisplayWaitVSync(pBltParms);

        // 7. Do blit
        if (!Blit())
        {
            GC2D_DebugTrace(GC2D_ERROR_TRACE, ("GC2DROPBlt failed"));
            GC2D_BREAK(GC2D_NOT_SUPPORT);
        }

        switch (mSyncMode)
        {
        case GC2D_SYNC_MODE_ASYNC:
        case GC2D_SYNC_MODE_FULL_ASYNC:
                // commit command buffer without wait
                gcmVERIFY_OK(FlushAndCommit(gcvTRUE, gcvFALSE));
                bForce = gcvFALSE;
                break;

        case GC2D_SYNC_MODE_FORCE:
        default:
            gcmVERIFY_OK(FlushAndCommit(gcvTRUE, gcvTRUE));
            break;
        }

        bSucceed = gcvTRUE;

    } while (gcvFALSE);

EXIT:
    if (bSucceed)
    {
        UpdateBusyStatus(bForce);

#if GC2D_GATHER_STATISTICS
        if (mABEnable)
        {
            ++nAlphaBlendSucceed;
        }
        else
        {
            ++nROPBlitSucceed;
        }
#endif

        mGPE2D->DisplayUpdateRegion(pBltParms);

        return S_OK;
    }
    else
    {
        GC2D_LEAVE_ARG("%d", -ERROR_NOT_SUPPORTED);
        return -ERROR_NOT_SUPPORTED;
    }
}

gctBOOL GC2DParms::DumpGPESurf(GPESurf* surf)
{
    GC2D_ENTER_ARG("surf = 0x%08p", surf);

    GC2D_DebugTrace(GC2D_DUMP_TRACE,
        ("DumpGPESurf: InVideoMemory=%d, Buffer=0x%08x, Format=%d,"\
         " BPP=%d, Rotate=%d, Width=%d, Height=%d, Stride=%d\r\n"),
         surf->InVideoMemory(), surf->Buffer(), surf->Format(),
         GetSurfBytesPerPixel(surf), surf->Rotate(), surf->Width(),
         surf->Height(), surf->Stride());

    GC2D_LEAVE();
    return gcvTRUE;
}

gctBOOL GC2DParms::DumpGPEBltParms(GPEBltParms * pBltParms)
{
    GPESurf* pGPESrc = (GPESurf*) pBltParms->pSrc;
    GPESurf* pGPEDst = (GPESurf*) pBltParms->pDst;
    GPESurf* pGPEMask = (GPESurf*) pBltParms->pMask;
    GPESurf* pGPEBrush = (GPESurf*) pBltParms->pBrush;
    gcsRECT_PTR pDstRect = (gcsRECT_PTR)pBltParms->prclDst;
    gcsRECT_PTR pSrcRect = (gcsRECT_PTR)pBltParms->prclSrc;
    gcsRECT_PTR pClipRect = (gcsRECT_PTR)pBltParms->prclClip;
    gcsRECT_PTR pMaskRect = (gcsRECT_PTR)pBltParms->prclMask;

    GC2D_ENTER_ARG("pBltParms = 0x%08p", pBltParms);

    GC2D_DebugTrace(GC2D_DUMP_TRACE,
        ("DumpBltPara: ***********************************************\r\n"));
    GC2D_DebugTrace(GC2D_DUMP_TRACE,
        ("DumpBltPara: ROP4=0x%08x, solid color=0x%08x, bltFlags=0x%08x\r\n"),
        pBltParms->rop4, pBltParms->solidColor, pBltParms->bltFlags);

    GC2D_DebugTrace(GC2D_DUMP_TRACE,
        ("DumpBltPara: xPos=%d, yPos=%d, iMode=%d\r\n"),
        pBltParms->xPositive, pBltParms->yPositive, pBltParms->iMode);

#if UNDER_CE > 700
    GC2D_DebugTrace(GC2D_DUMP_TRACE,
        ("DumpBltPara: Lookup=0x%08x, Convert=0x%08x, ColorConverter=0x%08x,"\
         " blendFunction=0x%08x\r\n"),
         pBltParms->pLookup, pBltParms->pConvert, pBltParms->pColorConverter,
         pBltParms->blendFunction);
#else
    GC2D_DebugTrace(GC2D_DUMP_TRACE,
        ("DumpBltPara: Lookup=0x%08x, Convert=0x%08x, ColorConverter=0x%08x,"\
         " blendFunction=0x%08x, bltSig=0x%08x\r\n"),
         pBltParms->pLookup, pBltParms->pConvert, pBltParms->pColorConverter,
         pBltParms->blendFunction, pBltParms->bltSig);
#endif

    if (pGPEDst)
    {
        GC2D_DebugTrace(GC2D_DUMP_TRACE,
            ("DumpBltPara: Dst Surf:\r\n"));
        DumpGPESurf(pGPEDst);

        if (pDstRect)
        {
            GC2D_DebugTrace(GC2D_DUMP_TRACE,
                ("DumpBltPara: DstRect(%d,%d, %d,%d) size(%d * %d)\r\n"),
                pDstRect->left, pDstRect->top, pDstRect->right, pDstRect->bottom,
                pDstRect->right - pDstRect->left, pDstRect->bottom - pDstRect->top);
        }

        if (pClipRect)
        {
            GC2D_DebugTrace(GC2D_DUMP_TRACE,
                ("DumpBltPara: ClpRect(%d,%d, %d,%d)\r\n"),
                pClipRect->left, pClipRect->top, pClipRect->right, pClipRect->bottom);
        }
    }

    if (pGPESrc)
    {
        GC2D_DebugTrace(GC2D_DUMP_TRACE,
            ("DumpBltPara: Src Surf:\r\n"));
        DumpGPESurf(pGPESrc);

        if (pSrcRect)
        {
            GC2D_DebugTrace(GC2D_DUMP_TRACE,
                ("DumpBltPara: SrcRect(%d,%d, %d,%d) size(%d * %d)\r\n"),
                pSrcRect->left, pSrcRect->top, pSrcRect->right, pSrcRect->bottom,
                pSrcRect->right - pSrcRect->left, pSrcRect->bottom - pSrcRect->top);
        }
    }

    if (pGPEMask)
    {
        GC2D_DebugTrace(GC2D_DUMP_TRACE,
            ("DumpBltPara: Mask Surf:\r\n"));
        DumpGPESurf(pGPEMask);

        if (pMaskRect)
        {
            GC2D_DebugTrace(GC2D_DUMP_TRACE,
                ("DumpBltPara: MskRect(%d,%d, %d,%d) size(%d * %d)\r\n"),
                pMaskRect->left, pMaskRect->top, pMaskRect->right, pMaskRect->bottom,
                pMaskRect->right - pMaskRect->left, pMaskRect->bottom - pMaskRect->top);
        }
    }

    if (pGPEBrush)
    {
        GC2D_DebugTrace(GC2D_DUMP_TRACE,
            ("DumpBltPara: Brush Surf:\r\n"));
        DumpGPESurf(pGPEBrush);

        if (pBltParms->pptlBrush)
        {
            GC2D_DebugTrace(GC2D_DUMP_TRACE,
                ("DumpBltPara: BrushPoint(%d, %d)\r\n"),
                pBltParms->pptlBrush->x, pBltParms->pptlBrush->y);
        }
    }

    GC2D_LEAVE();
    return gcvTRUE;
}
