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


#ifndef __GC2D_ACCELERATOR_H__
#define __GC2D_ACCELERATOR_H__

#include    <windows.h>
#include    <winddi.h>
#include    <ddrawi.h>
#include    <ddgpe.h>
#include    <ceddk.h>

/****************************************************************************
* Return values:
*****************************************************************************/
#define FACILITY_GC2D               0x02D

#define GC2D_SUCCESS                S_OK
#define GC2D_NOT_SUPPORTED          0xE02D0001
#define GC2D_REFUSED_BY_CONFIG      0xE02D0002
#define GC2D_NOT_MY_BLIT            0xE02D0003

class GC2D_Accelerator;
class GC2D_SurfWrapper;
class GC2DSurf;
class GC2DParms;

typedef class GC2D_Accelerator *GPE2D_PTR;
typedef SCODE (GPE::*GPEBLTFUN) (struct GPEBltParms*);

// GC2D_SurfWrapper can derived from GPESurf or DDGPESurf
class GC2D_SurfWrapper :
#ifdef DERIVED_FROM_GPE
    public GPESurf
#else
    public DDGPESurf
#endif
{
public:
    GC2D_SurfWrapper(
        GPE2D_PTR            Acc2D,
        int                  width,
        int                  height,
        EGPEFormat           format);

#ifndef DERIVED_FROM_GPE
    GC2D_SurfWrapper(
        GPE2D_PTR            Acc2D,
        int                  width,
        int                  height,
        int                  stride,
        EGPEFormat           format,
        EDDGPEPixelFormat    pixelFormat);
#endif

    GC2D_SurfWrapper(
        GPE2D_PTR            Acc2D,
        int                  width,
        int                  height,
        void *               pBits,            // virtual address of allocated bits
        int                  stride,
        EGPEFormat           format);

    ~GC2D_SurfWrapper();
    BOOL IsWrapped() const;

    UINT                    mAllocFlagByUser;
    GPE2D_PTR               mGPE2D;
    GC2DSurf*               mGC2DSurf;

private:
    GC2D_SurfWrapper();
};

// GC2D_Accelerator can derived from GPE or DDGPE
class GC2D_Accelerator:
#ifdef DERIVED_FROM_GPE
    public GPE
#else
    public DDGPE
#endif
{
    friend class GC2DSurf;
    friend class GC2DParms;

public:
    GC2D_Accelerator ()
    {
        mParms = NULL;
        mCallbackBlit = NULL;
    }

    ~GC2D_Accelerator ()
    {
        mParms = NULL;
    }

    BOOL GC2DIsValid() const
    {
        return mParms != NULL;
    }

    BOOL        GC2DInitialize(GPEBLTFUN CallbackBlit = NULL);
    BOOL        GC2DDeinitialize();
    SCODE       GC2DBltPrepare(GPEBltParms *pBltParms);
    SCODE       GC2DBltComplete(GPEBltParms *pBltParms);
    int         GC2DIsBusy();
    void        GC2DWaitForNotBusy();

    UINT        GC2DSetEnable(UINT* const pEnable = NULL);

    UINT        GC2DChangeSyncMode(UINT* const pMode = NULL);

    UINT        GC2DPowerHandler(BOOL bOff);

    // DisplayWaitVSync&DisplayUpdateRegion need to be implemented by the display driver.
    // DisplayWaitVSync called by GC2D_Accelerator before rendering to the target surface.
    // DisplayUpdateRegion called by GC2D_Accelerator after rendering to the target surface.
    void virtual DisplayUpdateRegion(GPEBltParms *pBltParms) {}
    void virtual DisplayWaitVSync(GPEBltParms *pBltParms) {}

protected:
    SCODE    AnyBlt(GPEBltParms * pBltParms);

    GPEBLTFUN mCallbackBlit;

    GC2DParms *mParms;
};

#endif //__GC2D_ACCELERATOR_H__
