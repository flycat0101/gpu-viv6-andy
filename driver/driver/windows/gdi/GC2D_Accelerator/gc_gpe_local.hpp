/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __MISC_H__
#define __MISC_H__

#include    "gc_gpe_accelerator.hpp"
#include    <gc_hal.h>
#include    <gc_hal_raster.h>
#include    <gc_hal_driver.h>
#include    <gc_hal_user_os_memory.h>

/****************************************************************************
* Option:
*        GC2D_GATHER_STATISTICS: enable/disable gathering statistics
*        GC2D_MIN_RENDERING_PIXEL: the default minimal number for the hardware
*            rendering.
*
*****************************************************************************/
#define GC2D_GATHER_STATISTICS   0
#define GC2D_MIN_RENDERING_PIXEL 0x100
#define GC2D_WAIT_TIME_OUT       3000

/****************************************************************************/

#define GC2D_REGISTRY_PATH                   TEXT("Drivers\\BuiltIn\\GCHAL")
#define GC2D_ENABLE_VALNAME                  TEXT("GC2DAcceleratorEnable")
#define GC2D_SYNCMODE_VALNAME                TEXT("GC2DAcceleratorSyncMode")
#define GC2D_MIN_SIZE_VALNAME                TEXT("GC2DAcceleratorMinSize")
#define GC2D_MIN_SCRCOPY_VALNAME             TEXT("GC2DAcceleratorMinPixelForSimpleSrcCopy")
#define GC2D_MIN_SOLIDFILL_VALNAME           TEXT("GC2DAcceleratorMinPixelForSimpleSolidFill")
#define GC2D_MIN_MASK_VALNAME                TEXT("GC2DAcceleratorMinPixelForMask")
#define GC2D_PRIMARY_SURFACE_ADDRESS_VALNAME TEXT("GC2DAcceleratorPrimarySurfaceAddress")

#define MAX_GC2D_FORMAT_EMUN 20

#define GC2D_DOMAIN(n)          (1 << (n))
#define GC2D_GCHAL              GC2D_DOMAIN(0)
#define GC2D_DISPLAY_BLIT       GC2D_DOMAIN(1)
#define GC2D_FUNCTION_TRACE     GC2D_DOMAIN(3)
#define GC2D_TEST               GC2D_DOMAIN(20)
#define GC2D_FORWARD_TO_SW      GC2D_DOMAIN(26)
#define GC2D_NOT_SUPPORT        GC2D_DOMAIN(27)
#define GC2D_ALWAYS_OUT         GC2D_DOMAIN(28)
#define GC2D_DUMP_TRACE         GC2D_DOMAIN(29)
#define GC2D_DEBUG_TRACE        GC2D_DOMAIN(30)
#define GC2D_ERROR_TRACE        GC2D_DOMAIN(31)
#define GC2D_TRACE_DOMAIN      (GC2D_ALWAYS_OUT)

void GC2D_DebugTrace(
    IN gctUINT32 Flag,
    IN char* Message,
    ...
    );

void GC2D_DebugFatal(
    IN char* Message,
    ...
    );

#if defined(DEBUG) && (UNDER_CE >= 600)

#define GC2D_ENTER() \
    GC2D_DebugTrace(GC2D_FUNCTION_TRACE, "Enter %s(%d)", __FUNCTION__, __LINE__)

#define GC2D_LEAVE() \
    GC2D_DebugTrace(GC2D_FUNCTION_TRACE, "Leave %s(%d)", __FUNCTION__, __LINE__)

#define GC2D_ENTER_ARG(TEXT, ...) \
    GC2D_DebugTrace(GC2D_FUNCTION_TRACE, "Enter %s(%d): "TEXT, __FUNCTION__, __LINE__, __VA_ARGS__)

#define GC2D_LEAVE_ARG(TEXT, ...) \
    GC2D_DebugTrace(GC2D_FUNCTION_TRACE, "Leave %s(%d): "TEXT, __FUNCTION__, __LINE__, __VA_ARGS__)

#define GC2D_BREAK(flag) \
    do { \
        GC2D_DebugTrace(flag, "exit from %s(%d)", __FUNCTION__, __LINE__); \
        goto EXIT; \
    } while (gcvFALSE)

#else

#define GC2D_ENTER()
#define GC2D_LEAVE()
static void dummy(...) {}
#define GC2D_ENTER_ARG dummy
#define GC2D_LEAVE_ARG dummy
#define GC2D_BREAK(flag) do { goto EXIT; } while (gcvFALSE)

#endif

enum GC2D_FEATURE_ENABLE
{
    GC2D_FEATURE_ENABLE_BLTPREPARE  = 0x00000001,
    GC2D_FEATURE_ENABLE_ALPHABLEND  = 0x00000002,
    GC2D_FEATURE_ENABLE_TRANSPARENT = 0x00000004,
    GC2D_FEATURE_ENABLE_STRETCH     = 0x00000008,
    GC2D_FEATURE_ENABLE_MASK        = 0x00000010,
};

enum GC2D_SYNC_MODE
{
    GC2D_SYNC_MODE_FORCE        = 0,
    GC2D_SYNC_MODE_ASYNC        = 1,
    GC2D_SYNC_MODE_FULL_ASYNC   = 2,
};

enum GC2D_OBJECT_TYPE
{
    GC2D_OBJ_UNKNOWN                 = 0,
    GC2D_OBJ_SURF                    = gcmCC('S','U','R','F'),
    GC2D_OBJ_BRUSH                   = gcmCC('B','R','U','S'),
    GC2D_OBJ_MONO                    = gcmCC('M','O','N','O'),
    GC2D_OBJ_PARMS                   = gcmCC('P','A','R','M'),
};

enum GC2DSURF_PROPERTY
{
    GC2DSURF_PROPERTY_DEFAULT        = 0, /* non-cachable & non-contiguous */
    GC2DSURF_PROPERTY_CACHABLE       = 0x00000001,
    GC2DSURF_PROPERTY_CONTIGUOUS     = 0x00000002,
    GC2DSURF_PROPERTY_ALL            = GC2DSURF_PROPERTY_CACHABLE
                                     | GC2DSURF_PROPERTY_CONTIGUOUS,
};

#define INVALID_PHYSICAL_ADDRESS    0xFFFFFFFF
#define MAX_WIDTH_HEIGHT            0x10000
#define MAX_STRIDE                  0x40000
#define MAX_LOCK_PAGES              0x400

#define ROP4_USE_SOURCE(Rop4) \
    ((( (Rop4) ^ ( (Rop4) >> 2 ) ) & 0x3333 ) ? gcvTRUE : gcvFALSE)

#define ROP4_USE_PATTERN(Rop4) \
    ((( (Rop4) ^ ( (Rop4) >> 4 ) ) & 0x0F0F ) ? gcvTRUE : gcvFALSE)

#define ROP4_USE_MASK(Rop4) \
    ((( (Rop4) ^ ( (Rop4) >> 8 ) ) & 0x00FF ) ? gcvTRUE : gcvFALSE)


// Local surface class
class GC2DSurf {
public:
    GC2DSurf();
    ~GC2DSurf();

    gceSTATUS Initialize(
        GPE2D_PTR Acc2D,
        gctUINT flag, gctUINT PhysAddr, gctPOINTER VirtAddr,
        gctUINT width, gctUINT height, gctUINT stride,
        gceSURF_FORMAT format, gceSURF_ROTATION rotate);

    gceSTATUS InitializeYUV(
        GPE2D_PTR Acc2D,
        gctUINT flag, gceSURF_FORMAT format,
        gctUINT YPhysAddr, gctPOINTER YVirtAddr, gctUINT YStride,
        gctUINT UPhysAddr, gctPOINTER UVirtAddr, gctUINT UStride,
        gctUINT VPhysAddr, gctPOINTER VVirtAddr, gctUINT VStride,
        gctUINT width, gctUINT height, gceSURF_ROTATION rotate
        );

    gceSTATUS Denitialize(gctBOOL sync);

    gctBOOL IsValid() const;

public:
    GC2D_OBJECT_TYPE    mType;
    gctUINT             mFlag;
    GPE2D_PTR           mGPE2D;

    gctUINT             mDMAAddr;
    gctUINT             mPhysicalAddr;
    gctPOINTER          mVirtualAddr;

    gctBOOL             mWrapped;
    gctUINT32           mWrappedNode;

    gctINT              mStride;
    gctUINT             mWidth;
    gctUINT             mHeight;
    gctUINT             mSize;
    gceSURF_FORMAT      mFormat;
    gceSURF_ROTATION    mRotate;

    gctUINT             mOwnerFlag;

    // for YUV Format
    gctUINT             mUDMAAddr;
    gctPOINTER          mUVirtualAddr;
    gctUINT             mUPhysicalAddr;
    gctINT              mUstride;
    gctBOOL             mUWrapped;
    gctUINT32           mUWrappedNode;

    gctUINT             mVDMAAddr;
    gctPOINTER          mVVirtualAddr;
    gctUINT             mVPhysicalAddr;
    gctINT              mVstride;
    gctBOOL             mVWrapped;
    gctUINT32           mVWrappedNode;
};

/******************************************************************************\
|* The definations for class GC2DParms                                         *|
\******************************************************************************/

enum BLIT_TYPE
{
    BLIT_INVALID,
    BLIT_NORMAL,
    BLIT_MASKED_SOURCE,
    BLIT_MONO_SOURCE,
    BLIT_STRETCH,
    BLIT_FILTER,
};

class GC2DBrush
{
public:
    GC2D_OBJECT_TYPE    mType;
    gceSURF_FORMAT      mFormat;
    gctUINT32           mX;
    gctUINT32           mY;
    gctUINT32           mDMAAddr;
    gctPOINTER          mVirtualAddr;
    gctUINT32           mSolidColor;
    gctBOOL             mSolidEnable;
    gctBOOL             mConvert;

public:
    GC2DBrush ()
    {
        mType = GC2D_OBJ_UNKNOWN;
        mFormat = gcvSURF_UNKNOWN;
        mDMAAddr = INVALID_PHYSICAL_ADDRESS;
        mVirtualAddr = 0;
        mSolidEnable = gcvFALSE;
    }

    gctBOOL Init();
    gctBOOL Denit();

    gctBOOL IsValid() const { return mType == GC2D_OBJ_BRUSH; }
};

class GC2DMono
{
public:
    GC2D_OBJECT_TYPE    mType;
    gctPOINTER          mStream;
    gctINT32            mStride;
    gctUINT32           mWidth;
    gctUINT32           mHeight;
    gctUINT32           mFgColor;
    gctUINT32           mBgColor;
    gctUINT32           mTsColor;

public:
    GC2DMono ()
    {
        mType = GC2D_OBJ_UNKNOWN;
    }

    gctBOOL IsValid() const { return mType == GC2D_OBJ_MONO; }
};
typedef class GC2DSurf *GC2DSurf_PTR;

class GC2DParms
{
public:
    GC2DParms();
    ~GC2DParms();

    BOOL        Initialize(GPE2D_PTR Acc2D);
    BOOL        Denitialize();
    SCODE       BltPrepare(GPEBltParms *pBltParms);
    SCODE       BltComplete(GPEBltParms *pBltParms);
    int         IsBusy();
    void        WaitForNotBusy();
    UINT        Enable(UINT* const pEnable = NULL);
    UINT        ChangeSyncMode(UINT* const pMode = NULL);
    UINT        PowerHandler(BOOL bOff);
    SCODE       Blit(GPEBltParms * pBltParms);


///////////////////////////////////////////////////////////////////////////
protected:
    gctBOOL InitTempBuffer(
        GC2DSurf_PTR *Buffer,
        gctUINT Width,
        gctUINT Height);

    gctBOOL DenitTempBuffer(
        GC2DSurf_PTR *Buffer);

    gctBOOL IsValid() const { return mType == GC2D_OBJ_PARMS; }

    BOOL ContainGC2DSurf(GPESurf *pGPESurf) const;
    BOOL IsGPESurfSupported(GPESurf *pGPESurf, gctBOOL bSrc);

    gctBOOL SetBrush(
        GPEBltParms * pBltParms,
        gceSURF_ROTATION DestRotate);

    gctBOOL SetMono(
        GPEBltParms * pBltParms,
        gctBOOL bSrc,
        GPESurf *Surf,
        gceSURF_ROTATION Rot,
        gctBOOL RopAAF0);

    gctBOOL SetSurface(
        GPEBltParms * pBltParms,
        gctBOOL bSrc,
        GPESurf *pGPESurf,
        gceSURF_ROTATION Rotation);

    gctBOOL SetAlphaBlend(
        GPEBltParms * pBltParms);

    gctBOOL ResetSurface(gctBOOL bSrc);

    gctBOOL Reset();

    gctBOOL Blit();

    gctBOOL ReadRegistry();

    gceSTATUS   FlushAndCommit(gctBOOL bFlush = gcvFALSE, gctBOOL bWait = gcvTRUE);
    gceSTATUS   WaitForHW();
    int         UpdateBusyStatus(gctBOOL clear);

    SCODE    BltParmsFilter(GPEBltParms *pBltParms);

    gctBOOL DumpGPEBltParms(GPEBltParms * pBltParms);
    gctBOOL DumpGPESurf(GPESurf* surf);

///////////////////////////////////////////////////////////////////////////
protected:
    GC2D_OBJECT_TYPE        mType;
    GPE2D_PTR               mGPE2D;
    gcoOS                   mOs;
    gco2D                   mEngine2D;

    gctUINT                 mEnableFlag;
    gctUINT                 mDumpFlag;

    gctUINT32               mSyncMode;
    gctUINT32               mCommitCounter;
    gctUINT32_PTR           mFinishFlag;
    gctUINT32               mFinishFlagPhys;

    gctINT32                mMinSrcCopyPixelNum;
    gctINT32                mMinSolidFillPixelNum;
    gctINT32                mMinMaskPixelNum;

    gctUINT                 mPowerHandle;

    gctSIGNAL               mStallSignal;

    gctUINT32               mHwVersion;
    gctUINT32               mHwMirrorEx;

    GC2DSurf_PTR            mSrcBuffer;
    gctPOINTER              mSrcBufferPointer;

    gctBOOL                 mMonoSupported;

    gctUINT8_PTR            mMonoBuffer;
    gctUINT32               mMonoBufferSize;

    gctUINT32               mVideoPhyBase;
    gctUINT32               mVideoMemSize;

    gctINT32                mMinPixelNum;
    gctINT32                mMaxPixelNum;

    gctUINT32               mAddrMask;
    gctUINT32               mStrideMask;

    GC2DSurf                mDstSurf;
    gcsRECT                 mDstRect;
    gcsRECT                 mClipRect;

    GC2DSurf                mSrcSurf;
    gcsRECT                 mSrcRect;
    gctUINT32               mSrcTrspColor;
    gctUINT32               mPalette[256];
    gctBOOL                 mPaletteConvert;

    GC2DMono                mMono;
    gcsRECT                 mMonoRect;

    GC2DBrush               mBrush;

    gctBOOL                 mMirrorX;
    gctBOOL                 mMirrorY;

    gctUINT8                mFgRop;
    gctUINT8                mBgRop;

    gceSURF_TRANSPARENCY    mTrsp;
    BLIT_TYPE               mBlit;

    //////// AlphaBlend parameters////////////
    gctBOOL                     mABEnable;
    gctUINT8                    mABSrcAg;
    gctUINT8                    mABDstAg;
    gceSURF_PIXEL_ALPHA_MODE    mABSrcAMode;
    gceSURF_PIXEL_ALPHA_MODE    mABDstAMode;
    gceSURF_GLOBAL_ALPHA_MODE   mABSrcAgMode;
    gceSURF_GLOBAL_ALPHA_MODE   mABDstAgMode;
    gceSURF_BLEND_FACTOR_MODE   mABSrcFactorMode;
    gceSURF_BLEND_FACTOR_MODE   mABDstFactorMode;
    gceSURF_PIXEL_COLOR_MODE    mABSrcColorMode;
    gceSURF_PIXEL_COLOR_MODE    mABDstColorMode;

#if GC2D_GATHER_STATISTICS
protected:
    gctUINT nPrepare;
    gctUINT nPrepareSucceed;
    gctUINT nROPBlit;
    gctUINT nROPBlitSucceed;
    gctUINT nAlphaBlend;
    gctUINT nAlphaBlendSucceed;
    gctUINT nASyncCommit;
    gctUINT nASyncCommitFailed;

    class GATHER_STATISTICS_SURF {
    public:
        GATHER_STATISTICS_SURF () {
            memset(nMemoryPool, 0, sizeof(nMemoryPool));
            memset(nRotate, 0, sizeof(nRotate));
            memset(nFormat, 0, sizeof(nFormat));
        }

        void RecordSurf(GPESurf* pSurf)
        {
            gcmASSERT(pSurf->Format() < MAX_GC2D_FORMAT_EMUN);
            ++nFormat[pSurf->Format()];

            switch (pSurf->Rotate())
            {
            case DMDO_0:
                ++nRotate[0];
                break;

            case DMDO_90:
                ++nRotate[1];
                break;

            case DMDO_180:
                ++nRotate[2];
                break;

            case DMDO_270:
                ++nRotate[3];
                break;

            default:
                break;
            }

            if (pSurf->InVideoMemory())
                ++nMemoryPool[0];
            else
                ++nMemoryPool[1];
        }

    protected:
        gctUINT        nMemoryPool[2];
        gctUINT        nRotate[4];
        gctUINT        nFormat[MAX_GC2D_FORMAT_EMUN];
    };
    GATHER_STATISTICS_SURF gssDst;
    GATHER_STATISTICS_SURF gssSrc;
    GATHER_STATISTICS_SURF gssMask;
    GATHER_STATISTICS_SURF gssBrush;
#endif
};

/******************************************************************************\
|*                 Local utility functions                                    *|
\******************************************************************************/

gceSURF_FORMAT
EGPEFormatToGALFormat(
    EGPEFormat format
    );

gceSURF_FORMAT
GPEFormatToGALFormat(
    EGPEFormat format,
    GPEFormat * gpeFormatPtr
    );

gceSURF_FORMAT
DDGPEFormatToGALFormat(
    EDDGPEPixelFormat format
    );

gceSURF_ROTATION
GPERotationToGALRotation(
    int iRotate
    );

int
GetSurfBytesPerPixel(
    GPESurf* surf
    );

int
GetSurfWidth(
    GPESurf* surf
    );

int
GetSurfHeight(
    GPESurf* surf
    );

gctBOOL
TransBrush(
    gctPOINTER pBrushDst,
    gctPOINTER pBrushSrc,
    gctINT Bpp,
    gceSURF_ROTATION Rotate
    );

gctBOOL
GetRelativeRotation(
    IN gceSURF_ROTATION Orientation,
    IN OUT gceSURF_ROTATION *Relation);

gctBOOL
TransRect(
    IN OUT gcsRECT_PTR Rect,
    IN gceSURF_ROTATION Rotation,
    IN gceSURF_ROTATION toRotation,
    IN gctINT32 SurfaceWidth,
    IN gctINT32 SurfaceHeight
    );

gceSTATUS
AlignWidthHeight(
    IN OUT gctUINT32 * Width,
    IN OUT gctUINT32 * Height);

gceSTATUS QueryAlignment(
    OUT gctUINT32 * Width,
    OUT gctUINT32 * Height);

gctPOINTER
AllocNonCacheMem(
    gctUINT32 size
    );

gctBOOL
FreeNonCacheMem(
    gctPOINTER addr
    );

gctUINT32
GetFormatSize(
    gceSURF_FORMAT format
    );

#define gcmNO_SUCCESS(status)        (status != gcvSTATUS_OK)

#endif //__MISC_H__
