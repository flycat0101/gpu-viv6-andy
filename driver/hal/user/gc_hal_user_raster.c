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


/**
**  @file
**  2D support functions.
**
*/

#include "gc_hal_user_precomp.h"
#include "gc_hal_user_brush.h"

/* Zone used for header/footer. */
#define _GC_OBJ_ZONE    gcvZONE_2D

/******************************************************************************\
********************************** Structures **********************************
\******************************************************************************/

struct _gco2D
{
    gcsOBJECT           object;
    gctBOOL             hwAvailable;
    gcoBRUSH_CACHE      brushCache;
    gctBOOL             alignImproved;
    gctBOOL             fullRotation;
    gctBOOL             tiling;
    gctBOOL             minorTiling;
    gcs2D_State         state;
    gcoHARDWARE         hardware;
};

#define _DestroyKernelArray(KernelInfo)                                        \
    if (KernelInfo.kernelStates != gcvNULL)                                    \
    {                                                                          \
        /* Free the array. */                                                  \
        if(gcmIS_ERROR(gcoOS_Free(gcvNULL, KernelInfo.kernelStates)))          \
        {                                                                      \
            gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_2D,                          \
                "2D Engine: Failed to free kernel table.");                    \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            KernelInfo.kernelStates = gcvNULL;                                 \
        }                                                                      \
                                                                               \
        /* Reset the pointer. */                                               \
        KernelInfo.kernelStates = gcvNULL;                                     \
    }

#define _FreeKernelArray(State)                                                \
        _DestroyKernelArray(State.horSyncFilterKernel)                         \
        _DestroyKernelArray(State.verSyncFilterKernel)                         \
        _DestroyKernelArray(State.horBlurFilterKernel)                         \
        _DestroyKernelArray(State.verBlurFilterKernel)                         \
        _DestroyKernelArray(State.horUserFilterKernel)                         \
        _DestroyKernelArray(State.verUserFilterKernel)

/******************************************************************************\
******************************** gco2D API Code ********************************
\******************************************************************************/
#if gcdENABLE_2D
static gceSTATUS _CheckFormat(
    gceSURF_FORMAT Format,
    gctUINT32_PTR PlaneNum,
    gctUINT32_PTR BitPerPixel,
    gctBOOL_PTR IsYUV)
{
    gctUINT32 plane;
    gctUINT32 bpp;
    gctBOOL isYUV = gcvFALSE;

    switch (Format)
    {
    case gcvSURF_INDEX8:
    case gcvSURF_A8:
    case gcvSURF_R8:
        plane = 1;
        bpp = 8;
        break;

    case gcvSURF_RG16:
    case gcvSURF_R5G6B5:
    case gcvSURF_B5G6R5:

    case gcvSURF_X4R4G4B4:
    case gcvSURF_A4R4G4B4:
    case gcvSURF_R4G4B4A4:
    case gcvSURF_R4G4B4X4:
    case gcvSURF_A4B4G4R4:
    case gcvSURF_X4B4G4R4:
    case gcvSURF_B4G4R4A4:
    case gcvSURF_B4G4R4X4:

    case gcvSURF_A1R5G5B5:
    case gcvSURF_X1R5G5B5:
    case gcvSURF_R5G5B5A1:
    case gcvSURF_R5G5B5X1:
    case gcvSURF_A1B5G5R5:
    case gcvSURF_X1B5G5R5:
    case gcvSURF_B5G5R5A1:
    case gcvSURF_B5G5R5X1:
        plane = 1;
        bpp = 16;
        break;

    case gcvSURF_X8R8G8B8:
    case gcvSURF_A8R8G8B8:
    case gcvSURF_R8G8B8A8:
    case gcvSURF_R8G8B8X8:
    case gcvSURF_X8B8G8R8:
    case gcvSURF_A8B8G8R8:
    case gcvSURF_B8G8R8X8:
    case gcvSURF_B8G8R8A8:
        plane = 1;
        bpp = 32;
        break;

    case gcvSURF_YV12:
    case gcvSURF_I420:
        plane = 3;
        bpp = 12;
        isYUV = gcvTRUE;
        break;

    case gcvSURF_NV12:
    case gcvSURF_NV21:
        plane = 2;
        bpp = 12;
        isYUV = gcvTRUE;
        break;

    case gcvSURF_NV16:
    case gcvSURF_NV61:
        plane = 2;
        bpp = 16;
        isYUV = gcvTRUE;
        break;

    case gcvSURF_YVYU:
    case gcvSURF_VYUY:
    case gcvSURF_YUY2:
    case gcvSURF_UYVY:
        plane = 1;
        bpp = 16;
        isYUV = gcvTRUE;
        break;

    case gcvSURF_A2B10G10R10:
    case gcvSURF_A2R10G10B10:
    case gcvSURF_R10G10B10A2:
    case gcvSURF_B10G10R10A2:
        plane = 1;
        bpp = 32;
        break;

    case gcvSURF_NV12_10BIT:
    case gcvSURF_NV21_10BIT:
        plane = 2;
        bpp = 15;
        isYUV = gcvTRUE;
        break;

    case gcvSURF_NV16_10BIT:
    case gcvSURF_NV61_10BIT:
        plane = 2;
        bpp = 20;
        isYUV = gcvTRUE;
        break;

    case gcvSURF_P010:
        plane = 2;
        bpp = 24;
        isYUV = gcvTRUE;
        break;

    default:
        return gcvSTATUS_NOT_SUPPORTED;
    }

    if (PlaneNum)
    {
        *PlaneNum = plane;
    }

    if (BitPerPixel)
    {
        *BitPerPixel = bpp;
    }

    if (IsYUV)
    {
        *IsYUV = isYUV;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS _CheckSurface(
    gco2D Engine,
    gctBOOL Src,
    gceSURF_FORMAT Format,
    gctUINT32_PTR Address,
    gctUINT32_PTR Stride,
    gctUINT32 Width,
    gctUINT32 Height,
    gceSURF_ROTATION Rotation,
    gceTILING Tiling)
{
    gceSURF_ROTATION rot = gcmGET_PRE_ROTATION(Rotation);

    if ((Width >= 0x10000) || (Height >= 0x10000)
        || (!Engine->fullRotation && (rot != gcvSURF_0_DEGREE) && (rot != gcvSURF_90_DEGREE)))
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    switch (Tiling)
    {
    case gcvLINEAR:
        break;

    case gcvTILED:
    case gcvSUPERTILED:
    case gcvMULTI_TILED:
    case gcvMULTI_SUPERTILED:
        if (!Engine->tiling)
        {
            return gcvSTATUS_NOT_SUPPORTED;
        }
        break;

    case gcvMINORTILED:
        if (!Engine->minorTiling ||
            gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_ANDROID_ONLY) == gcvTRUE)
        {
            return gcvSTATUS_NOT_SUPPORTED;
        }
        break;

    case gcvYMAJOR_SUPERTILED:
        if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_MAJOR_SUPER_TILE) != gcvTRUE)
        {
            return gcvSTATUS_NOT_SUPPORTED;
        }
        break;

    case gcvTILED_8X4:
    case gcvTILED_4X8:
    case gcvTILED_32X4:
    case gcvTILED_64X4:
    case gcvTILED_8X8_XMAJOR:
    case gcvTILED_8X8_YMAJOR:
    case gcvSUPERTILED_128B:
    case gcvSUPERTILED_256B:
        if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_DEC400_COMPRESSION) != gcvTRUE)
        {
            return gcvSTATUS_NOT_SUPPORTED;
        }
        break;

    default:
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if (Engine->alignImproved)
    {
        switch (Format)
        {
        case gcvSURF_INDEX8:
        case gcvSURF_A8:
        case gcvSURF_R8:
            break;

        case gcvSURF_RG16:
        case gcvSURF_R5G6B5:
        case gcvSURF_B5G6R5:

        case gcvSURF_X4R4G4B4:
        case gcvSURF_A4R4G4B4:
        case gcvSURF_R4G4B4A4:
        case gcvSURF_R4G4B4X4:
        case gcvSURF_A4B4G4R4:
        case gcvSURF_X4B4G4R4:
        case gcvSURF_B4G4R4A4:
        case gcvSURF_B4G4R4X4:

        case gcvSURF_A1R5G5B5:
        case gcvSURF_X1R5G5B5:
        case gcvSURF_R5G5B5A1:
        case gcvSURF_R5G5B5X1:
        case gcvSURF_A1B5G5R5:
        case gcvSURF_X1B5G5R5:
        case gcvSURF_B5G5R5A1:
        case gcvSURF_B5G5R5X1:
            if ((Address[0] | Stride[0]) & 1)
            {
                return gcvSTATUS_NOT_ALIGNED;
            }

            if (Stride[0] >= 0x40000)
            {
                return gcvSTATUS_INVALID_ARGUMENT;
            }
            break;

        case gcvSURF_X8R8G8B8:
        case gcvSURF_A8R8G8B8:
        case gcvSURF_R8G8B8A8:
        case gcvSURF_R8G8B8X8:
        case gcvSURF_X8B8G8R8:
        case gcvSURF_A8B8G8R8:
        case gcvSURF_B8G8R8X8:
        case gcvSURF_B8G8R8A8:

        case gcvSURF_A2B10G10R10:
        case gcvSURF_A2R10G10B10:
        case gcvSURF_R10G10B10A2:
        case gcvSURF_B10G10R10A2:
            if ((Address[0] | Stride[0]) & 3)
            {
                return gcvSTATUS_NOT_ALIGNED;
            }

            if (Stride[0] >= 0x40000)
            {
                return gcvSTATUS_INVALID_ARGUMENT;
            }
            break;

        case gcvSURF_YV12:
        case gcvSURF_I420:
            if (((Address[0] | Address[1] | Address[2]) & 63)
                || (Stride[0] & 7) || ((Stride[1] | Stride[2]) & 3))
            {
                return gcvSTATUS_NOT_ALIGNED;
            }

            if ((Stride[0] >= 0x40000) || (Stride[1] >= 0x40000)
                || (Stride[2] >= 0x40000))
            {
                return gcvSTATUS_INVALID_ARGUMENT;
            }
            break;

        case gcvSURF_NV12:
        case gcvSURF_NV21:
        case gcvSURF_NV16:
        case gcvSURF_NV61:
        case gcvSURF_P010:
            if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TPCV11_COMPRESSION) == gcvTRUE)
            {
                if (Format == gcvSURF_NV12)
                {
                    if (((Address[0] | Address[1]) & 255) ||
                        (( Stride[0] |  Stride[1]) &  31) ||
                        (Width & 31))
                    {
                        return gcvSTATUS_NOT_ALIGNED;
                    }
                }
                else if (Format == gcvSURF_P010)
                {
                    if (((Address[0] | Address[1]) & 511) ||
                        (( Stride[0] |  Stride[1]) &  63) ||
                        (Width & 31))
                    {
                        return gcvSTATUS_NOT_ALIGNED;
                    }
                }
            }
            else if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_DEC400_COMPRESSION) == gcvTRUE)
            {
                if (Format == gcvSURF_NV12 || Format == gcvSURF_P010)
                {
                    if (((Address[0] | Address[1]) & 255) ||
                        (( Stride[0] |  Stride[1]) &  15) ||
                        (Width & 15))
                    {
                        return gcvSTATUS_NOT_ALIGNED;
                    }
                }
            }

            if (((Address[0] | Address[1]) & 63)
                || ((Stride[0] | Stride[1]) & 7))
            {
                return gcvSTATUS_NOT_ALIGNED;
            }

            if ((Stride[0] >= 0x40000) || (Stride[1] >= 0x40000))
            {
                return gcvSTATUS_INVALID_ARGUMENT;
            }
            break;

        case gcvSURF_NV12_10BIT:
        case gcvSURF_NV21_10BIT:
            if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TPCV11_COMPRESSION) == gcvTRUE)
            {
                if (((Address[0] + Address[1]) % 320) ||
                    ((Stride[0] + Stride[1]) % 320))
                {
                    return gcvSTATUS_NOT_ALIGNED;
                }

                if (Width & 255)
                {
                    return gcvSTATUS_NOT_ALIGNED;
                }
            }
            else if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_DEC400_COMPRESSION) == gcvTRUE)
            {
                if (((Address[0] + Address[1]) % 80) ||
                    ((Stride[0] + Stride[1]) % 80))
                {
                    return gcvSTATUS_NOT_ALIGNED;
                }

                if (Width & 7)
                {
                    return gcvSTATUS_NOT_ALIGNED;
                }
            }
            break;

        case gcvSURF_NV16_10BIT:
        case gcvSURF_NV61_10BIT:
            if (((Address[0] + Address[1]) % 80) ||
                ((Stride[0] + Stride[1]) % 80))
            {
                return gcvSTATUS_NOT_ALIGNED;
            }
            break;

        case gcvSURF_YVYU:
        case gcvSURF_VYUY:
        case gcvSURF_YUY2:
        case gcvSURF_UYVY:
            if ((Src && (Address[0] & 63))
                || (!Src && (Address[0] & 4))
                || (Stride[0] & 3))
            {
                return gcvSTATUS_NOT_ALIGNED;
            }

            if (Stride[0] >= 0x40000)
            {
                return gcvSTATUS_INVALID_ARGUMENT;
            }

            break;

        default:
            return gcvSTATUS_NOT_SUPPORTED;
        }
    }
    else
    {
        switch (Format)
        {
        case gcvSURF_INDEX8:
        case gcvSURF_A8:

        case gcvSURF_R5G6B5:
        case gcvSURF_B5G6R5:

        case gcvSURF_X4R4G4B4:
        case gcvSURF_A4R4G4B4:
        case gcvSURF_R4G4B4A4:
        case gcvSURF_R4G4B4X4:
        case gcvSURF_A4B4G4R4:
        case gcvSURF_X4B4G4R4:
        case gcvSURF_B4G4R4A4:
        case gcvSURF_B4G4R4X4:

        case gcvSURF_A1R5G5B5:
        case gcvSURF_X1R5G5B5:
        case gcvSURF_R5G5B5A1:
        case gcvSURF_R5G5B5X1:
        case gcvSURF_A1B5G5R5:
        case gcvSURF_X1B5G5R5:
        case gcvSURF_B5G5R5A1:
        case gcvSURF_B5G5R5X1:

        case gcvSURF_X8R8G8B8:
        case gcvSURF_A8R8G8B8:
        case gcvSURF_R8G8B8A8:
        case gcvSURF_R8G8B8X8:
        case gcvSURF_X8B8G8R8:
        case gcvSURF_A8B8G8R8:
        case gcvSURF_B8G8R8X8:
        case gcvSURF_B8G8R8A8:

        case gcvSURF_A2B10G10R10:
        case gcvSURF_A2R10G10B10:
        case gcvSURF_R10G10B10A2:
        case gcvSURF_B10G10R10A2:
            if ((Address[0] | Stride[0]) & 15)
            {
                return gcvSTATUS_NOT_ALIGNED;
            }

            if (Stride[0] >= 0x40000)
            {
                return gcvSTATUS_INVALID_ARGUMENT;
            }

            break;

        case gcvSURF_YV12:
        case gcvSURF_I420:
            if (((Address[0] | Address[1] | Address[2]) & 63)
                || (Stride[0] & 7) || ((Stride[1] | Stride[2]) & 3))
            {
                return gcvSTATUS_NOT_ALIGNED;
            }

            if ((Stride[0] >= 0x40000) || (Stride[1] >= 0x40000)
                || (Stride[2] >= 0x40000))
            {
                return gcvSTATUS_INVALID_ARGUMENT;
            }

            break;

        case gcvSURF_NV12:
        case gcvSURF_NV21:
        case gcvSURF_NV16:
        case gcvSURF_NV61:
        case gcvSURF_P010:
        case gcvSURF_NV12_10BIT:
        case gcvSURF_NV21_10BIT:
        case gcvSURF_NV16_10BIT:
        case gcvSURF_NV61_10BIT:
            if (((Address[0] | Address[1]) & 63)
                || ((Stride[0] | Stride[1]) & 7))
            {
                return gcvSTATUS_NOT_ALIGNED;
            }

            if ((Stride[0] >= 0x40000) || (Stride[1] >= 0x40000))
            {
                return gcvSTATUS_INVALID_ARGUMENT;
            }

            break;

        case gcvSURF_YVYU:
        case gcvSURF_VYUY:
        case gcvSURF_YUY2:
        case gcvSURF_UYVY:
            if ((Address[0] & 15)
                || (Stride[0] & 7))
            {
                return gcvSTATUS_NOT_ALIGNED;
            }

            if (Stride[0] >= 0x40000)
            {
                return gcvSTATUS_INVALID_ARGUMENT;
            }

            break;

        default:
            return gcvSTATUS_NOT_SUPPORTED;
        }
    }

    return gcvSTATUS_OK;
}
#endif

/*******************************************************************************
**
**  gco2D_Construct
**
**  Construct a new gco2D object.
**
**  INPUT:
**
**      gcoHAL Hal
**          Poniter to an gcoHAL object.
**
**  OUTPUT:
**
**      gco2D * Engine
**          Pointer to a variable that will hold the pointer to the gco2D object.
*/
gceSTATUS
gco2D_Construct(
    IN gcoHAL Hal,
    OUT gco2D * Engine
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gco2D engine = gcvNULL;
    gctPOINTER pointer = gcvNULL;

    gcmHEADER();

    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT(Engine != gcvNULL);

    do
    {
        gctUINT i;

        /* Allocate the gco2D object. */
        gcmERR_BREAK(gcoOS_Allocate(
            gcvNULL,
            sizeof(struct _gco2D),
            &pointer
            ));

        gcoOS_ZeroMemory(pointer, sizeof(struct _gco2D));

        engine = pointer;

        /* Initialize the gco2D object. */
        engine->object.type = gcvOBJ_2D;

        /* Is 2D pipe available? */
        engine->hwAvailable = gcoHARDWARE_Is2DAvailable(gcvNULL);

        /* Construct brush cache object. */
        gcmERR_BREAK(gcoBRUSH_CACHE_Construct(gcvNULL, &engine->brushCache));

        for (i = 0; i < gcdMULTI_SOURCE_NUM; i++)
        {
            gcs2D_MULTI_SOURCE_PTR src = engine->state.multiSrc + i;

            src->srcType = gcv2D_SOURCE_INVALID;
            src->srcSurface.tiling = gcvLINEAR;
            if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TPCV11_COMPRESSION) == gcvTRUE)
            {
                src->srcYUVMode = gcv2D_YUV_709;
            }
            else
            {
                src->srcYUVMode = gcv2D_YUV_601;
            }
            src->srcDeGamma = gcvFALSE;
            src->enableGDIStretch = gcvFALSE;
        }

        engine->state.dstSurface.tiling = gcvLINEAR;
        engine->state.dstEnGamma = gcvFALSE;
        if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TPCV11_COMPRESSION) == gcvTRUE)
        {
            engine->state.dstYUVMode = gcv2D_YUV_709;
        }
        else
        {
            engine->state.dstYUVMode = gcv2D_YUV_601;
        }

        engine->state.superTileVersion = gcv2D_SUPER_TILE_VERSION_V2;
        engine->state.unifiedDstRect = gcvFALSE;
        engine->state.multiBilinearFilter = gcvFALSE;

        for (i = 0; i < gcd2D_GAMMA_TABLE_SIZE; i++)
        {
            engine->state.deGamma[i] = (i | (i << 8) | (i << 16));
            engine->state.enGamma[i] = (i | (i << 10) | (i << 20));
        }

        /***********************************************************************
        ** Initialize filter blit states.
        */

        /* Sync filter variables. */
        gcoOS_ZeroMemory(
            &engine->state.horSyncFilterKernel,
            gcmSIZEOF(gcsFILTER_BLIT_ARRAY)
            );

        gcoOS_ZeroMemory(
            &engine->state.verSyncFilterKernel,
            gcmSIZEOF(gcsFILTER_BLIT_ARRAY)
            );

        engine->state.horSyncFilterKernel.filterType = gcvFILTER_SYNC;
        engine->state.verSyncFilterKernel.filterType = gcvFILTER_SYNC;

        engine->state.horSyncFilterKernel.kernelChanged = gcvTRUE;
        engine->state.verSyncFilterKernel.kernelChanged = gcvTRUE;

        /* Blur filter variables. */
        gcoOS_ZeroMemory(
            &engine->state.horBlurFilterKernel,
            gcmSIZEOF(gcsFILTER_BLIT_ARRAY)
            );

        gcoOS_ZeroMemory(
            &engine->state.verBlurFilterKernel,
            gcmSIZEOF(gcsFILTER_BLIT_ARRAY)
            );

        engine->state.horBlurFilterKernel.filterType = gcvFILTER_BLUR;
        engine->state.verBlurFilterKernel.filterType = gcvFILTER_BLUR;

        engine->state.horBlurFilterKernel.kernelChanged = gcvTRUE;
        engine->state.verBlurFilterKernel.kernelChanged = gcvTRUE;

        /* User defined filter variables. */
        engine->state.horUserFilterKernel.filterType = gcvFILTER_USER;
        engine->state.verUserFilterKernel.filterType = gcvFILTER_USER;

        engine->state.horUserFilterKernel.kernelChanged = gcvTRUE;
        engine->state.verUserFilterKernel.kernelChanged = gcvTRUE;

        /* Filter blit variables. */
        engine->state.horUserFilterPass = gcvTRUE;
        engine->state.verUserFilterPass = gcvTRUE;

        engine->state.newFilterType     = gcvFILTER_SYNC;
        engine->state.newHorKernelSize  = 9;
        engine->state.newVerKernelSize  = 9;

        engine->alignImproved =
            (gcoHAL_IsFeatureAvailable(Hal, gcvFEATURE_2D_PIXEL_ALIGNMENT) == gcvTRUE);
        engine->fullRotation =
            (gcoHAL_IsFeatureAvailable(Hal, gcvFEATURE_2D_BITBLIT_FULLROTATION) == gcvTRUE)
         && (gcoHAL_IsFeatureAvailable(Hal, gcvFEATURE_2D_FILTERBLIT_FULLROTATION) == gcvTRUE);
        engine->tiling =
            (gcoHAL_IsFeatureAvailable(Hal, gcvFEATURE_2D_TILING) == gcvTRUE);
        engine->minorTiling =
            (gcoHAL_IsFeatureAvailable(Hal, gcvFEATURE_2D_MINOR_TILING) == gcvTRUE);

        engine->state.specialFilterMirror = gcvFALSE;

        /* Force 2D HW hang by detecting the process name "galTrigger2DHang"
            This feature is for the recovery testing. */
        engine->state.forceHWStuck = gcoOS_DetectProcessByEncryptedName("\x98\x9e\x93\xab\x8d\x96\x98\x98\x9a\x8d\xcd\xbb\xb7\x9e\x91\x98");

        /* Return pointer to the gco2D object. */
        *Engine = engine;

        /* Success. */
        gcmFOOTER_ARG("*Engine=0x%x", *Engine);
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    /* Roll back. */
    if (engine != gcvNULL)
    {
        if (engine->brushCache != gcvNULL)
        {
            gcmVERIFY_OK(gcoBRUSH_CACHE_Destroy(engine->brushCache));
        }

        _FreeKernelArray(engine->state);

        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, engine));
    }

    /* Success. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

/*******************************************************************************
**
**  gco2D_Destroy
**
**  Destroy an gco2D object.
**
**  INPUT:
**
**      gco2D Engine
**          Poniter to an gco2D object to destroy.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_Destroy(
    IN gco2D Engine
    )
{
#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x", Engine);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    /* Mark the gco2D object as unknown. */
    Engine->object.type = gcvOBJ_UNKNOWN;

    if (Engine->state.paletteTable != gcvNULL)
    {
        if(gcmIS_ERROR(gcoOS_Free(
            gcvNULL,
            Engine->state.paletteTable
            )))
        {
            gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_2D,
                "2D Engine: Failed to free palette table.");
        }
        else
        {
            Engine->state.paletteTable = gcvNULL;
        }
    }

    if (Engine->brushCache != gcvNULL)
    {
        if(gcmIS_ERROR(gcoBRUSH_CACHE_Destroy(Engine->brushCache)))
        {
            gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_2D,
                "2D Engine: Failed to free brush cache.");
        }
        else
        {
            Engine->brushCache = gcvNULL;
        }
    }

    _FreeKernelArray(Engine->state);

    /* Free the gco2D object. */
    if(gcmIS_ERROR(gcoOS_Free(gcvNULL, Engine)))
    {
        gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_2D,
            "2D Engine: Failed to free gco2D object.");
    }
    else
    {
        Engine = gcvNULL;
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_SetBrushLimit
**
**  Sets the maximum number of brushes in the cache.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT MaxCount
**          Maximum number of brushes allowed in the cache at the same time.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetBrushLimit(
    IN gco2D Engine,
    IN gctUINT MaxCount
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x MaxCount=%d", Engine, MaxCount);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    status = gcoBRUSH_CACHE_SetBrushLimit(
        Engine->brushCache,
        MaxCount
        );

    /* Return status. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

/*******************************************************************************
**
**  gco2D_GetBrushCache
**
**  Return a pointer to the brush cache.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**  OUTPUT:
**
**      gcoBRUSH_CACHE * BrushCache
**          A pointer to gcoBRUSH_CACHE object.
*/
gceSTATUS
gco2D_GetBrushCache(
    IN gco2D Engine,
    IN OUT gcoBRUSH_CACHE * BrushCache
    )
{
#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x, BrushCache=0x%x", Engine, BrushCache);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);
    gcmVERIFY_ARGUMENT(BrushCache != gcvNULL);

    *BrushCache = Engine->brushCache;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_FlushBrush
**
**  Flush the brush.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gcoBRUSH Brush
**          A pointer to a valid gcoBRUSH object.
**
**      gceSURF_FORMAT Format
**          Format for destination surface when using color conversion.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_FlushBrush(
    IN gco2D Engine,
    IN gcoBRUSH Brush,
    IN gceSURF_FORMAT Format
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D

    gcmHEADER_ARG("Engine=0x%x Brush=0x%x Format=%d", Engine, Brush, Format);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);
    gcmVERIFY_OBJECT(Brush, gcvOBJ_BRUSH);
    gcmVERIFY_ARGUMENT(Format != gcvSURF_UNKNOWN);

    /* Ignore the destination format. */

    status = gcoBRUSH_CACHE_FlushBrush(
            Engine->brushCache,
            Brush);

    /* Return status. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

/*******************************************************************************
**
**  gco2D_LoadSolidBrush
**
**  Program the specified solid color brush.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gceSURF_FORMAT Format
**          Format for destination surface when using color conversion.
**
**      gctUINT32 ColorConvert
**          The value of the Color parameter is stored directly in internal
**          color register and is used either directly to initialize pattern
**          or is converted to the format of destination before it is used.
**          The later happens if ColorConvert is not zero.
**
**      gctUINT32 Color
**          The color value of the pattern. The value will be used to
**          initialize 8x8 pattern. If the value is in destination format,
**          set ColorConvert to 0. Otherwise, provide the value in ARGB8
**          format and set ColorConvert to 1 to instruct the hardware to
**          convert the value to the destination format before it is
**          actually used.
**
**      gctUINT64 Mask
**          64 bits of mask, where each bit corresponds to one pixel of 8x8
**          pattern. Each bit of the mask is used to determine transparency
**          of the corresponding pixel, in other words, each mask bit is used
**          to select between foreground or background ROPs. If the bit is 0,
**          background ROP is used on the pixel; if 1, the foreground ROP
**          is used. The mapping between Mask parameter bits and actual
**          pattern pixels is as follows:
**
**          +----+----+----+----+----+----+----+----+
**          |  7 |  6 |  5 |  4 |  3 |  2 |  1 |  0 |
**          +----+----+----+----+----+----+----+----+
**          | 15 | 14 | 13 | 12 | 11 | 10 |  9 |  8 |
**          +----+----+----+----+----+----+----+----+
**          | 23 | 22 | 21 | 20 | 19 | 18 | 17 | 16 |
**          +----+----+----+----+----+----+----+----+
**          | 31 | 30 | 29 | 28 | 27 | 26 | 25 | 24 |
**          +----+----+----+----+----+----+----+----+
**          | 39 | 38 | 37 | 36 | 35 | 34 | 33 | 32 |
**          +----+----+----+----+----+----+----+----+
**          | 47 | 46 | 45 | 44 | 43 | 42 | 41 | 40 |
**          +----+----+----+----+----+----+----+----+
**          | 55 | 54 | 53 | 52 | 51 | 50 | 49 | 48 |
**          +----+----+----+----+----+----+----+----+
**          | 63 | 62 | 61 | 60 | 59 | 58 | 57 | 56 |
**          +----+----+----+----+----+----+----+----+
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_LoadSolidBrush(
    IN gco2D Engine,
    IN gceSURF_FORMAT Format,
    IN gctUINT32 ColorConvert,
    IN gctUINT32 Color,
    IN gctUINT64 Mask
    )
{
#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x Format=%d ColorConvert=%d Color=%x Mask=%llx",
                    Engine, Format, ColorConvert, Color, Mask);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    if (Mask && gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_NO_COLORBRUSH_INDEX8) == gcvTRUE)
    {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* Ignore the parameter Format. */

    Engine->state.brushType = gcv2D_PATTERN_SOLID;
    Engine->state.brushMask = Mask;
    Engine->state.brushColorConvert = ColorConvert;
    Engine->state.brushFgColor = Color;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_LoadMonochromeBrush
**
**  Create a new monochrome gcoBRUSH object.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT32 OriginX
**      gctUINT32 OriginY
**          Specifies the origin of the pattern in 0..7 range.
**
**      gctUINT32 ColorConvert
**          The values of FgColor and BgColor parameters are stored directly in
**          internal color registers and are used either directly to initialize
**          pattern or converted to the format of destination before actually
**          used. The later happens if ColorConvert is not zero.
**
**      gctUINT32 FgColor
**      gctUINT32 BgColor
**          Foreground and background colors of the pattern. The values will be
**          used to initialize 8x8 pattern. If the values are in destination
**          format, set ColorConvert to 0. Otherwise, provide the values in
**          ARGB8 format and set ColorConvert to 1 to instruct the hardware to
**          convert the values to the destination format before they are
**          actually used.
**
**      gctUINT64 Bits
**          64 bits of pixel bits. Each bit represents one pixel and is used
**          to choose between foreground and background colors. If the bit
**          is 0, the background color is used; otherwise the foreground color
**          is used. The mapping between Bits parameter and the actual pattern
**          pixels is the same as of the Mask parameter.
**
**      gctUINT64 Mask
**          64 bits of mask, where each bit corresponds to one pixel of 8x8
**          pattern. Each bit of the mask is used to determine transparency
**          of the corresponding pixel, in other words, each mask bit is used
**          to select between foreground or background ROPs. If the bit is 0,
**          background ROP is used on the pixel; if 1, the foreground ROP
**          is used. The mapping between Mask parameter bits and the actual
**          pattern pixels is as follows:
**
**          +----+----+----+----+----+----+----+----+
**          |  7 |  6 |  5 |  4 |  3 |  2 |  1 |  0 |
**          +----+----+----+----+----+----+----+----+
**          | 15 | 14 | 13 | 12 | 11 | 10 |  9 |  8 |
**          +----+----+----+----+----+----+----+----+
**          | 23 | 22 | 21 | 20 | 19 | 18 | 17 | 16 |
**          +----+----+----+----+----+----+----+----+
**          | 31 | 30 | 29 | 28 | 27 | 26 | 25 | 24 |
**          +----+----+----+----+----+----+----+----+
**          | 39 | 38 | 37 | 36 | 35 | 34 | 33 | 32 |
**          +----+----+----+----+----+----+----+----+
**          | 47 | 46 | 45 | 44 | 43 | 42 | 41 | 40 |
**          +----+----+----+----+----+----+----+----+
**          | 55 | 54 | 53 | 52 | 51 | 50 | 49 | 48 |
**          +----+----+----+----+----+----+----+----+
**          | 63 | 62 | 61 | 60 | 59 | 58 | 57 | 56 |
**          +----+----+----+----+----+----+----+----+
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_LoadMonochromeBrush(
    IN gco2D Engine,
    IN gctUINT32 OriginX,
    IN gctUINT32 OriginY,
    IN gctUINT32 ColorConvert,
    IN gctUINT32 FgColor,
    IN gctUINT32 BgColor,
    IN gctUINT64 Bits,
    IN gctUINT64 Mask
    )
{
#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x OriginX=%d OriginY=%d ColorConvert=%d "
                    "FgColor=%x BgColor=%x Bits=%lld Mask=%llx",
                    Engine, OriginX, OriginY, ColorConvert,
                    FgColor, BgColor, Bits, Mask);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);
    gcmVERIFY_ARGUMENT(OriginX < 8);
    gcmVERIFY_ARGUMENT(OriginY < 8);

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_NO_COLORBRUSH_INDEX8) == gcvTRUE)
    {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    Engine->state.brushType = gcv2D_PATTERN_MONO;
    Engine->state.brushBits = Bits;
    Engine->state.brushMask = Mask;
    Engine->state.brushColorConvert = ColorConvert;
    Engine->state.brushFgColor = FgColor;
    Engine->state.brushBgColor = BgColor;
    Engine->state.brushOriginX = OriginX;
    Engine->state.brushOriginY = OriginY;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_LoadColorBrush
**
**  Create a color gcoBRUSH object.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT32 OriginX
**      gctUINT32 OriginY
**          Specifies the origin of the pattern in 0..7 range.
**
**      gctPOINTER Address
**          Location of the pattern bitmap in video memory.
**
**      gceSURF_FORMAT Format
**          Format of the source bitmap.
**
**      gctUINT64 Mask
**          64 bits of mask, where each bit corresponds to one pixel of 8x8
**          pattern. Each bit of the mask is used to determine transparency
**          of the corresponding pixel, in other words, each mask bit is used
**          to select between foreground or background ROPs. If the bit is 0,
**          background ROP is used on the pixel; if 1, the foreground ROP
**          is used. The mapping between Mask parameter bits and the actual
**          pattern pixels is as follows:
**
**          +----+----+----+----+----+----+----+----+
**          |  7 |  6 |  5 |  4 |  3 |  2 |  1 |  0 |
**          +----+----+----+----+----+----+----+----+
**          | 15 | 14 | 13 | 12 | 11 | 10 |  9 |  8 |
**          +----+----+----+----+----+----+----+----+
**          | 23 | 22 | 21 | 20 | 19 | 18 | 17 | 16 |
**          +----+----+----+----+----+----+----+----+
**          | 31 | 30 | 29 | 28 | 27 | 26 | 25 | 24 |
**          +----+----+----+----+----+----+----+----+
**          | 39 | 38 | 37 | 36 | 35 | 34 | 33 | 32 |
**          +----+----+----+----+----+----+----+----+
**          | 47 | 46 | 45 | 44 | 43 | 42 | 41 | 40 |
**          +----+----+----+----+----+----+----+----+
**          | 55 | 54 | 53 | 52 | 51 | 50 | 49 | 48 |
**          +----+----+----+----+----+----+----+----+
**          | 63 | 62 | 61 | 60 | 59 | 58 | 57 | 56 |
**          +----+----+----+----+----+----+----+----+
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_LoadColorBrush(
    IN gco2D Engine,
    IN gctUINT32 OriginX,
    IN gctUINT32 OriginY,
    IN gctUINT32 Address,
    IN gceSURF_FORMAT Format,
    IN gctUINT64 Mask
    )
{
#if gcdENABLE_2D
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 n;
    gctBOOL isYUV;
    gcmHEADER_ARG("Engine=0x%x OriginX=%d OriginY=%d Address=0x%x Format=%d Mask=%llx",
                    Engine, OriginX, OriginY, Address, Format, Mask);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_NO_COLORBRUSH_INDEX8) == gcvTRUE)
    {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    gcmONERROR(_CheckFormat(Format, &n, gcvNULL, &isYUV));

    if ((n != 1) || (OriginX > 7) || (OriginY > 7) || isYUV)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    n = 0;
    gcmONERROR(_CheckSurface(Engine, gcvTRUE, Format, &Address, &n, 0, 0, gcvSURF_0_DEGREE, gcvLINEAR));

    Engine->state.brushType = gcv2D_PATTERN_COLOR;
    Engine->state.brushAddress = Address;
    Engine->state.brushFormat = Format;
    Engine->state.brushMask = Mask;
    Engine->state.brushOriginX = OriginX;
    Engine->state.brushOriginY = OriginY;

OnError:
    gcmFOOTER();
    return status;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_SetMonochromeSource
**
**  Configure color source.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctBOOL ColorConvert
**          The values of FgColor and BgColor parameters are stored directly in
**          internal color registers and are used either directly as the source
**          color or converted to the format of destination before actually
**          used.  The later happens if ColorConvert is gcvTRUE.
**
**      gctUINT8 MonoTransparency
**          This value is used in gcvSURF_SOURCE_MATCH transparency mode.  The
**          value can be either 0 or 1 and is compared against each mono pixel
**          to determine transparency of the pixel.  If the values found are
**          equal, the pixel is transparent; otherwise it is opaque.
**
**      gceSURF_MONOPACK DataPack
**          Determines how many horizontal pixels are there per each 32-bit
**          chunk of monochrome bitmap.  For example, if set to gcvSURF_PACKED8,
**          each 32-bit chunk is 8-pixel wide, which also means that it defines
**          4 vertical lines of pixels.
**
**      gctBOOL CoordRelative
**          If gcvFALSE, the source origin represents absolute pixel coordinate
**          within the source surface. If gcvTRUE, the source origin represents the
**          offset from the destination origin.
**
**      gceSURF_TRANSPARENCY Transparency
**          gcvSURF_OPAQUE - each pixel of the bitmap overwrites the destination.
**          gcvSURF_SOURCE_MATCH - source pixels compared against register value
**              to determine the transparency.  In simple terms, the
**              transaprency comes down to selecting the ROP code to use.
**              Opaque pixels use foreground ROP and transparent ones use
**              background ROP.
**          gcvSURF_SOURCE_MASK - monochrome source mask defines transparency.
**          gcvSURF_PATTERN_MASK - pattern mask defines transparency.
**
**      gctUINT32 FgColor
**      gctUINT32 BgColor
**          The values are used to represent foreground and background colors
**          of the source.  If the values are in destination format, set
**          ColorConvert to gcvFALSE. Otherwise, provide the values in A8R8G8B8
**          format and set ColorConvert to gcvTRUE to instruct the hardware to
**          convert the values to the destination format before they are
**          actually used.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetMonochromeSource(
    IN gco2D Engine,
    IN gctBOOL ColorConvert,
    IN gctUINT8 MonoTransparency,
    IN gceSURF_MONOPACK DataPack,
    IN gctBOOL CoordRelative,
    IN gceSURF_TRANSPARENCY Transparency,
    IN gctUINT32 FgColor,
    IN gctUINT32 BgColor
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gcs2D_MULTI_SOURCE_PTR curSrc;

    gcmHEADER_ARG("Engine=0x%x ColorConvert=%d MonoTransparency=%d DataPack=%lld "
                    "CoordRelative=%d Transparency=%d FgColor=%x BgColor=%x",
                    Engine, ColorConvert, MonoTransparency, DataPack,
                    CoordRelative, Transparency, FgColor, BgColor);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_ANDROID_ONLY) == gcvTRUE)
    {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    do
    {
        curSrc = &Engine->state.multiSrc[Engine->state.currentSrcIndex];

        /* Set the transparency. */
        gcmERR_BREAK(gcoHARDWARE_TranslateSurfTransparency(
            Transparency,
            &curSrc->srcTransparency,
            &curSrc->dstTransparency,
            &curSrc->patTransparency
            ));

        /* Set the source. */
        curSrc->srcFgColor               = FgColor;
        curSrc->srcBgColor               = BgColor;
        curSrc->srcMonoTransparencyColor = MonoTransparency;
        curSrc->srcRelativeCoord         = CoordRelative;
        curSrc->srcMonoPack              = DataPack;
        curSrc->srcColorConvert          = ColorConvert;
        curSrc->srcStream                = gcvTRUE;
        curSrc->srcSurface.format        = gcvSURF_INDEX1;

        curSrc->srcType = gcv2D_SOURCE_MONO;

        /* Succeed. */
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    curSrc->srcType = gcv2D_SOURCE_INVALID;

    /* Return status. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

/*******************************************************************************
**
**  gco2D_SetColorSource
**
**  Configure color source. This function is deprecated. Please use
**  gco2D_SetColorSourceEx instead.
**
**  This function is only working with old PE (<2.0).
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT32 Address
**          Source surface base address.
**
**      gctUINT32 Stride
**          Stride of the source surface in bytes.
**
**      gceSURF_FORMAT Format
**          Color format of the source surface.
**
**      gceSURF_ROTATION Rotation
**          Type of rotation.
**
**      gctUINT32 SurfaceWidth
**          Required only if the surface is rotated. Equal to the width
**          of the source surface in pixels.
**
**      gctBOOL CoordRelative
**          If gcvFALSE, the source origin represents absolute pixel coordinate
**          within the source surface.  If gcvTRUE, the source origin represents
**          the offset from the destination origin.
**
**      gceSURF_TRANSPARENCY Transparency
**          gcvSURF_OPAQUE - each pixel of the bitmap overwrites the destination.
**          gcvSURF_SOURCE_MATCH - source pixels compared against register value
**              to determine the transparency.  In simple terms, the
**              transaprency comes down to selecting the ROP code to use.
**              Opaque pixels use foreground ROP and transparent ones use
**              background ROP.
**          gcvSURF_SOURCE_MASK - monochrome source mask defines transparency.
**          gcvSURF_PATTERN_MASK - pattern mask defines transparency.
**
**      gctUINT32 TransparencyColor
**          This value is used in gcvSURF_SOURCE_MATCH transparency mode.  The
**          value is compared against each pixel to determine transparency of
**          the pixel.  If the values found are equal, the pixel is transparent;
**          otherwise it is opaque.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetColorSource(
    IN gco2D Engine,
    IN gctUINT32 Address,
    IN gctUINT32 Stride,
    IN gceSURF_FORMAT Format,
    IN gceSURF_ROTATION Rotation,
    IN gctUINT32 SurfaceWidth,
    IN gctBOOL CoordRelative,
    IN gceSURF_TRANSPARENCY Transparency,
    IN gctUINT32 TransparencyColor
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x Address=%x Stride=%d Format=%d Rotation=%d "
                    "SurfaceWidth=%d CoordRelative=%d Transparency=%d "
                    "TransparencyColor=%x",
                    Engine, Address, Stride, Format, Rotation,
                    SurfaceWidth, CoordRelative, Transparency, TransparencyColor);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);
    gcmVERIFY_ARGUMENT(Format != gcvSURF_UNKNOWN);

    /* This interface does not support full rotations. */
    gcmVERIFY_ARGUMENT((Rotation == gcvSURF_0_DEGREE) || (Rotation == gcvSURF_90_DEGREE));

    /* Forward to gco2D_SetColorSourceEx with the SurfaceHeight set to 0. */
    status = gco2D_SetColorSourceEx(
            Engine,
            Address,
            Stride,
            Format,
            Rotation,
            SurfaceWidth,
            0,
            CoordRelative,
            Transparency,
            TransparencyColor);

    /* Return status. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

/*******************************************************************************
**
**  gco2D_SetColorSourceEx
**
**  Configure color source.
**
**  This function is only working with old PE (<2.0).
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT32 Address
**          Source surface base address.
**
**      gctUINT32 Stride
**          Stride of the source surface in bytes.
**
**      gceSURF_FORMAT Format
**          Color format of the source surface.
**
**      gceSURF_ROTATION Rotation
**          Type of rotation.
**
**      gctUINT32 SurfaceWidth
**          Required only if the surface is rotated. Equal to the width
**          of the source surface in pixels.
**
**      gctUINT32 SurfaceHeight
**          Required only if the surface is rotated in PE2.0. Equal to the height
**          of the source surface in pixels.
**
**      gctBOOL CoordRelative
**          If gcvFALSE, the source origin represents absolute pixel coordinate
**          within the source surface.  If gcvTRUE, the source origin represents
**          the offset from the destination origin.
**
**      gceSURF_TRANSPARENCY Transparency
**          gcvSURF_OPAQUE - each pixel of the bitmap overwrites the destination.
**          gcvSURF_SOURCE_MATCH - source pixels compared against register value
**              to determine the transparency.  In simple terms, the
**              transaprency comes down to selecting the ROP code to use.
**              Opaque pixels use foreground ROP and transparent ones use
**              background ROP.
**          gcvSURF_SOURCE_MASK - monochrome source mask defines transparency.
**          gcvSURF_PATTERN_MASK - pattern mask defines transparency.
**
**      gctUINT32 TransparencyColor
**          This value is used in gcvSURF_SOURCE_MATCH transparency mode.  The
**          value is compared against each pixel to determine transparency of
**          the pixel.  If the values found are equal, the pixel is transparent;
**          otherwise it is opaque.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetColorSourceEx(
    IN gco2D Engine,
    IN gctUINT32 Address,
    IN gctUINT32 Stride,
    IN gceSURF_FORMAT Format,
    IN gceSURF_ROTATION Rotation,
    IN gctUINT32 SurfaceWidth,
    IN gctUINT32 SurfaceHeight,
    IN gctBOOL CoordRelative,
    IN gceSURF_TRANSPARENCY Transparency,
    IN gctUINT32 TransparencyColor
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gcoSURF surface;
    gcs2D_MULTI_SOURCE_PTR curSrc;
    gctUINT32 n;

    gcmHEADER_ARG("Engine=0x%x Address=%x Stride=%d Format=%d Rotation=%d "
                    "SurfaceWidth=%d SurfaceHeight=%d CoordRelative=%d "
                    "Transparency=%d TransparencyColor=%x",
                    Engine, Address, Stride, Format, Rotation,
                    SurfaceWidth, SurfaceHeight, CoordRelative,
                    Transparency, TransparencyColor);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    gcmONERROR(_CheckFormat(Format, &n, gcvNULL, gcvNULL));

    if (n != 1)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmONERROR(_CheckSurface(Engine, gcvTRUE, Format, &Address, &Stride,
        SurfaceWidth, SurfaceHeight, Rotation, gcvLINEAR));

    curSrc = Engine->state.multiSrc + Engine->state.currentSrcIndex;

    if (Format != gcvSURF_INDEX8)
    {
        /* Set the transparency color. */
        gcmONERROR(gcoHARDWARE_ColorPackToARGB8(
            Format,
            TransparencyColor,
            &TransparencyColor
            ));
    }

    /* Set the source. */
    gcmONERROR(gcoHARDWARE_TranslateSurfTransparency(
        Transparency,
        &curSrc->srcTransparency,
        &curSrc->dstTransparency,
        &curSrc->patTransparency
        ));

    curSrc->srcColorKeyHigh =
    curSrc->srcColorKeyLow = TransparencyColor;

    surface                     = &curSrc->srcSurface;
    surface->type               = gcvSURF_BITMAP;
    surface->format             = Format;
    surface->alignedW           = SurfaceWidth;
    surface->alignedH           = SurfaceHeight;
    surface->rotation           = Rotation;
    surface->stride             = Stride;
    curSrc->srcRelativeCoord    = CoordRelative;

    if (Engine->hwAvailable)
    {
        gcsSURF_NODE_SetHardwareAddress(&surface->node, Address);
    }
    else
    {
        surface->node.logical = (gctUINT8_PTR)(gctUINTPTR_T) Address;
    }

    curSrc->srcType = gcv2D_SOURCE_COLOR;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:

    /* Return status. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

/* Same as gco2D_SetColorSourceEx, but with better 64bit SW-path support.
** Please do NOT export the API now.
*/
gceSTATUS
gco2D_SetColorSource64(
    IN gco2D Engine,
    IN gctUINT32 Address,
    IN gctPOINTER Logical,
    IN gctUINT32 Stride,
    IN gceSURF_FORMAT Format,
    IN gceSURF_ROTATION Rotation,
    IN gctUINT32 SurfaceWidth,
    IN gctUINT32 SurfaceHeight,
    IN gctBOOL CoordRelative,
    IN gceSURF_TRANSPARENCY Transparency,
    IN gctUINT32 TransparencyColor
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gcoSURF surface;
    gcs2D_MULTI_SOURCE_PTR curSrc;
    gctUINT32 n;

    gcmHEADER_ARG("Engine=0x%x Address=%x Logical=%p Stride=%d Format=%d "
                  "Rotation=%d SurfaceWidth=%d SurfaceHeight=%d "
                  "CoordRelative=%d Transparency=%d TransparencyColor=%x",
                  Engine, Address, Logical, Stride, Format, Rotation,
                  SurfaceWidth, SurfaceHeight, CoordRelative,
                  Transparency, TransparencyColor);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    gcmONERROR(_CheckFormat(Format, &n, gcvNULL, gcvNULL));

    if (n != 1)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmONERROR(_CheckSurface(Engine, gcvTRUE, Format, &Address, &Stride,
        SurfaceWidth, SurfaceHeight, Rotation, gcvLINEAR));

    curSrc = Engine->state.multiSrc + Engine->state.currentSrcIndex;

    if (Format != gcvSURF_INDEX8)
    {
        /* Set the transparency color. */
        gcmONERROR(gcoHARDWARE_ColorPackToARGB8(
            Format,
            TransparencyColor,
            &TransparencyColor
            ));
    }

    /* Set the source. */
    gcmONERROR(gcoHARDWARE_TranslateSurfTransparency(
        Transparency,
        &curSrc->srcTransparency,
        &curSrc->dstTransparency,
        &curSrc->patTransparency
        ));

    curSrc->srcColorKeyHigh =
    curSrc->srcColorKeyLow = TransparencyColor;

    surface                     = &curSrc->srcSurface;
    surface->type               = gcvSURF_BITMAP;
    surface->format             = Format;
    surface->alignedW           = SurfaceWidth;
    surface->alignedH           = SurfaceHeight;
    surface->rotation           = Rotation;
    surface->stride             = Stride;
    curSrc->srcRelativeCoord    = CoordRelative;

    if (Engine->hwAvailable)
    {
        gcsSURF_NODE_SetHardwareAddress(&surface->node, Address);
    }
    else
    {
        surface->node.logical = (gctUINT8_PTR)Logical;
    }

    curSrc->srcType = gcv2D_SOURCE_COLOR;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:

    /* Return status. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

/*******************************************************************************
**
**  gco2D_SetColorSourceAdvanced
**
**  Configure color source.
**
**  This function is only working with PE 2.0 and above.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gcoSURF Source
**          Source surface.
**
**      gctBOOL CoordRelative
**          If gcvFALSE, the source origin represents absolute pixel coordinate
**          within the source surface.  If gcvTRUE, the source origin represents
**          the offset from the destination origin.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetColorSourceAdvanced(
    IN gco2D Engine,
    IN gctUINT32 Address,
    IN gctUINT32 Stride,
    IN gceSURF_FORMAT Format,
    IN gceSURF_ROTATION Rotation,
    IN gctUINT32 SurfaceWidth,
    IN gctUINT32 SurfaceHeight,
    IN gctBOOL CoordRelative
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gcoSURF surface;
    gctUINT32 n;

    gcmHEADER_ARG("Engine=0x%x Address=%x Stride=%d Format=%d Rotation=%d "
                    "SurfaceWidth=%d SurfaceHeight=%d CoordRelative=%d ",
                    Engine, Address, Stride, Format, Rotation,
                    SurfaceWidth, SurfaceHeight, CoordRelative);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    gcmONERROR(_CheckFormat(Format, &n, gcvNULL, gcvNULL));

    if (n != 1)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmONERROR(_CheckSurface(Engine, gcvTRUE, Format, &Address, &Stride,
        SurfaceWidth, SurfaceHeight, Rotation, gcvLINEAR));

    /* Fill in the structure. */
    surface                = &Engine->state.multiSrc[Engine->state.currentSrcIndex].srcSurface;
    surface->type          = gcvSURF_BITMAP;
    surface->format        = Format;
    surface->alignedW      = SurfaceWidth;
    surface->alignedH      = SurfaceHeight;
    surface->rotation      = Rotation;
    surface->stride        = Stride;

    if (Engine->hwAvailable)
    {
        gcsSURF_NODE_SetHardwareAddress(&surface->node, Address);
    }
    else
    {
        surface->node.logical = (gctUINT8_PTR)(gctUINTPTR_T) Address;
    }

    Engine->state.multiSrc[Engine->state.currentSrcIndex].srcRelativeCoord = CoordRelative;

    Engine->state.multiSrc[Engine->state.currentSrcIndex].srcType = gcv2D_SOURCE_COLOR;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:

    /* Return status. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

/*******************************************************************************
**
**  gco2D_SetMaskedSource
**
**  Configure masked color source. This function is deprecated. Please use
**  gco2D_SetMaskedSourceEx instead.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT32 Address
**          Source surface base address.
**
**      gctUINT32 Stride
**          Stride of the source surface in bytes.
**
**      gceSURF_FORMAT Format
**          Color format of the source surface.
**
**      gctBOOL CoordRelative
**          If gcvFALSE, the source origin represents absolute pixel coordinate
**          within the source surface.  If gcvTRUE, the source origin represents
**          the offset from the destination origin.
**
**      gceSURF_MONOPACK MaskPack
**          Determines how many horizontal pixels are there per each 32-bit
**          chunk of monochrome mask.  For example, if set to gcvSURF_PACKED8,
**          each 32-bit chunk is 8-pixel wide, which also means that it defines
**          4 vertical lines of pixel mask.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetMaskedSource(
    IN gco2D Engine,
    IN gctUINT32 Address,
    IN gctUINT32 Stride,
    IN gceSURF_FORMAT Format,
    IN gctBOOL CoordRelative,
    IN gceSURF_MONOPACK MaskPack
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x Address=%x Stride=%d Format=%d CoordRelative=%d MaskPack=%d",
                    Engine, Address, Stride, Format, CoordRelative, MaskPack);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);
    gcmVERIFY_ARGUMENT(Format != gcvSURF_UNKNOWN);

    /* Forward to gco2D_SetMaskedSourceEx with the Rotation set to gcvSURF_0_DEGREE,
    the SurfaceWidth set to 0 and SurfaceHeight set to 0. */
    status = gco2D_SetMaskedSourceEx(
            Engine,
            Address,
            Stride,
            Format,
            CoordRelative,
            MaskPack,
            gcvSURF_0_DEGREE,
            0,
            0
            );

    /* Return status. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

/*******************************************************************************
**
**  gco2D_SetMaskedSourceEx
**
**  Configure masked color source.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT32 Address
**          Source surface base address.
**
**      gctUINT32 Stride
**          Stride of the source surface in bytes.
**
**      gceSURF_FORMAT Format
**          Color format of the source surface.
**
**      gctBOOL CoordRelative
**          If gcvFALSE, the source origin represents absolute pixel coordinate
**          within the source surface.  If gcvTRUE, the source origin represents
**          the offset from the destination origin.
**
**      gceSURF_MONOPACK MaskPack
**          Determines how many horizontal pixels are there per each 32-bit
**          chunk of monochrome mask.  For example, if set to gcvSURF_PACKED8,
**          each 32-bit chunk is 8-pixel wide, which also means that it defines
**          4 vertical lines of pixel mask.
**
**      gceSURF_ROTATION Rotation
**          Type of rotation in PE2.0.
**
**      gctUINT32 SurfaceWidth
**          Required only if the surface is rotated in PE2.0. Equal to the width
**          of the source surface in pixels.
**
**      gctUINT32 SurfaceHeight
**          Required only if the surface is rotated in PE2.0. Equal to the height
**          of the source surface in pixels.
**
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetMaskedSourceEx(
    IN gco2D Engine,
    IN gctUINT32 Address,
    IN gctUINT32 Stride,
    IN gceSURF_FORMAT Format,
    IN gctBOOL CoordRelative,
    IN gceSURF_MONOPACK MaskPack,
    IN gceSURF_ROTATION Rotation,
    IN gctUINT32 SurfaceWidth,
    IN gctUINT32 SurfaceHeight
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gctUINT32 n;
    gcoSURF surface;
    gcs2D_MULTI_SOURCE_PTR curSrc;

    gcmHEADER_ARG("Engine=0x%x Address=%x Stride=%d Format=%d CoordRelative=%d "
                    "MaskPack=%d Rotation=%d SurfaceWidth=%d SurfaceHeight=%d",
                    Engine, Address, Stride, Format, CoordRelative,
                    MaskPack, Rotation, SurfaceWidth, SurfaceHeight);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    gcmONERROR(_CheckFormat(Format, &n, gcvNULL, gcvNULL));

    if (n != 1)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmONERROR(_CheckSurface(Engine, gcvTRUE, Format, &Address, &Stride,
        SurfaceWidth, SurfaceHeight, Rotation, gcvLINEAR));

    curSrc = Engine->state.multiSrc + Engine->state.currentSrcIndex;

    /* Set the transparency. */
    gcmONERROR(gcoHARDWARE_TranslateSurfTransparency(
        gcvSURF_SOURCE_MASK,
        &curSrc->srcTransparency,
        &curSrc->dstTransparency,
        &curSrc->patTransparency
        ));

    /* Set the surface of the current source. */
    surface                  = &curSrc->srcSurface;
    surface->type            = gcvSURF_BITMAP;
    surface->format          = Format;
    surface->stride          = Stride;
    surface->rotation        = Rotation;
    surface->alignedW        = SurfaceWidth;
    surface->alignedH        = SurfaceHeight;
    curSrc->srcMonoPack      = MaskPack;
    curSrc->srcRelativeCoord = CoordRelative;
    curSrc->srcStream        = gcvFALSE;

    if (Engine->hwAvailable)
    {
        gcsSURF_NODE_SetHardwareAddress(&surface->node, Address);
    }
    else
    {
        surface->node.logical = (gctUINT8_PTR)(gctUINTPTR_T) Address;
    }

    curSrc->srcType = gcv2D_SOURCE_MASKED;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:

    /* Return status. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

/* Same as gco2D_SetMaskedSourceEx, but with better 64bit SW-path support.
** Please do NOT export the API now.
*/
gceSTATUS
gco2D_SetMaskedSource64(
    IN gco2D Engine,
    IN gctUINT32 Address,
    IN gctPOINTER Logical,
    IN gctUINT32 Stride,
    IN gceSURF_FORMAT Format,
    IN gctBOOL CoordRelative,
    IN gceSURF_MONOPACK MaskPack,
    IN gceSURF_ROTATION Rotation,
    IN gctUINT32 SurfaceWidth,
    IN gctUINT32 SurfaceHeight
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gctUINT32 n;
    gcoSURF surface;
    gcs2D_MULTI_SOURCE_PTR curSrc;

    gcmHEADER_ARG("Engine=0x%x Address=%x Stride=%d Format=%d CoordRelative=%d "
                    "MaskPack=%d Rotation=%d SurfaceWidth=%d SurfaceHeight=%d",
                    Engine, Address, Stride, Format, CoordRelative,
                    MaskPack, Rotation, SurfaceWidth, SurfaceHeight);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    gcmONERROR(_CheckFormat(Format, &n, gcvNULL, gcvNULL));

    if (n != 1)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmONERROR(_CheckSurface(Engine, gcvTRUE, Format, &Address, &Stride,
        SurfaceWidth, SurfaceHeight, Rotation, gcvLINEAR));

    curSrc = Engine->state.multiSrc + Engine->state.currentSrcIndex;

    /* Set the transparency. */
    gcmONERROR(gcoHARDWARE_TranslateSurfTransparency(
        gcvSURF_SOURCE_MASK,
        &curSrc->srcTransparency,
        &curSrc->dstTransparency,
        &curSrc->patTransparency
        ));

    /* Set the surface of the current source. */
    surface                  = &curSrc->srcSurface;
    surface->type            = gcvSURF_BITMAP;
    surface->format          = Format;
    surface->stride          = Stride;
    surface->rotation        = Rotation;
    surface->alignedW        = SurfaceWidth;
    surface->alignedH        = SurfaceHeight;
    curSrc->srcMonoPack      = MaskPack;
    curSrc->srcRelativeCoord = CoordRelative;
    curSrc->srcStream        = gcvFALSE;

    if (Engine->hwAvailable)
    {
        gcsSURF_NODE_SetHardwareAddress(&surface->node, Address);
    }
    else
    {
        surface->node.logical = (gctUINT8_PTR)Logical;
    }

    curSrc->srcType = gcv2D_SOURCE_MASKED;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:

    /* Return status. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

/*******************************************************************************
**
**  gco2D_SetSource
**
**  Setup the source rectangle.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gcsRECT_PTR SrcRect
**          Pointer to a valid source rectangle.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetSource(
    IN gco2D Engine,
    IN gcsRECT_PTR SrcRect
    )
{
#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x SrcRect=0x%x", Engine, SrcRect);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);
    gcmVERIFY_ARGUMENT(SrcRect != gcvNULL);
    gcmVERIFY_ARGUMENT(SrcRect->left < 0x10000);
    gcmVERIFY_ARGUMENT(SrcRect->right < 0x10000);
    gcmVERIFY_ARGUMENT(SrcRect->top < 0x10000);
    gcmVERIFY_ARGUMENT(SrcRect->bottom < 0x10000);

    Engine->state.multiSrc[Engine->state.currentSrcIndex].srcRect = *SrcRect;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_SetTargetRect
**
**  Setup the target rectangle for multi src blit.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gcsRECT_PTR Rect
**          Pointer to a valid target rectangle.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetTargetRect(
    IN gco2D Engine,
    IN gcsRECT_PTR Rect
    )
{
#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x SrcRect=0x%x", Engine, Rect);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_MULTI_SOURCE_BLT_EX2) != gcvTRUE)
    {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    gcmVERIFY_ARGUMENT(Rect != gcvNULL);
    gcmVERIFY_ARGUMENT(Rect->left < 0x10000);
    gcmVERIFY_ARGUMENT(Rect->right < 0x10000);
    gcmVERIFY_ARGUMENT(Rect->top < 0x10000);
    gcmVERIFY_ARGUMENT(Rect->bottom < 0x10000);

    Engine->state.multiSrc[Engine->state.currentSrcIndex].dstRect = *Rect;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_SetClipping
**
**  Set clipping rectangle.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gcsRECT_PTR Rect
**          Pointer to a valid destination rectangle.
**          The valid range of the coordinates is 0..32768.
**          A pixel is valid if the following is true:
**              (pixelX >= Left) && (pixelX < Right) &&
**              (pixelY >= Top)  && (pixelY < Bottom)
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetClipping(
    IN gco2D Engine,
    IN gcsRECT_PTR Rect
    )
{
#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x Rect=%d", Engine, Rect);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    /* Reset default if rect is not specified. */
    if (Rect == gcvNULL)
    {
        /* Set to the largest rectangle. */

        Engine->state.dstClipRect.left   = 0;
        Engine->state.dstClipRect.top    = 0;
        Engine->state.dstClipRect.right  = 32767;
        Engine->state.dstClipRect.bottom = 32767;
    }
    else
    {
        Engine->state.dstClipRect = *Rect;
    }

    Engine->state.multiSrc[Engine->state.currentSrcIndex].clipRect =
        Engine->state.dstClipRect;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_SetTarget
**
**  Configure destination. This function is deprecated. Please use
**  gco2D_SetTargetEx instead.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT32 Address
**          Destination surface base address.
**
**      gctUINT32 Stride
**          Stride of the destination surface in bytes.
**
**      gceSURF_ROTATION Rotation
**          Set to not zero if the destination surface is 90 degree rotated.
**
**      gctUINT32 SurfaceWidth
**          Required only if the surface is rotated. Equal to the width
**          of the destination surface in pixels.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetTarget(
    IN gco2D Engine,
    IN gctUINT32 Address,
    IN gctUINT32 Stride,
    IN gceSURF_ROTATION Rotation,
    IN gctUINT32 SurfaceWidth
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x Address=%x Stride=%d Rotation=%d SurfaceWidth=%d",
                    Engine, Address, Stride, Rotation, SurfaceWidth);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    /* This interface does not support full rotations. */
    gcmVERIFY_ARGUMENT((Rotation == gcvSURF_0_DEGREE) || (Rotation == gcvSURF_90_DEGREE));

    /* Forward to gco2D_SetTargetEx with the SurfaceHeight set to 0. */
    status = gco2D_SetTargetEx(
                Engine,
                Address,
                Stride,
                Rotation,
                SurfaceWidth,
                0);

    /* Return status. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

/*******************************************************************************
**
**  gco2D_SetTargetEx
**
**  Configure destination.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT32 Address
**          Destination surface base address.
**
**      gctUINT32 Stride
**          Stride of the destination surface in bytes.
**
**      gceSURF_ROTATION Rotation
**          Set to not zero if the destination surface is 90 degree rotated.
**
**      gctUINT32 SurfaceWidth
**          Required only if the surface is rotated. Equal to the width
**          of the destination surface in pixels.
**
**      gctUINT32 SurfaceHeight
**          Required only if the surface is rotated in PE 2.0. Equal to the height
**          of the destination surface in pixels.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetTargetEx(
    IN gco2D Engine,
    IN gctUINT32 Address,
    IN gctUINT32 Stride,
    IN gceSURF_ROTATION Rotation,
    IN gctUINT32 SurfaceWidth,
    IN gctUINT32 SurfaceHeight
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gcoSURF surface;

    gcmHEADER_ARG("Engine=0x%x Address=%x Stride=%d Rotation=%d "
                    "SurfaceWidth=%d SurfaceHeight=%d",
                    Engine, Address, Stride, Rotation, SurfaceWidth, SurfaceHeight);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    /* Assume the target format is gcvSURF_A8R8G8B8 because this API is only used
        to receive the RGB/BGR format surface and 32bpp would apply the max alignment. */
    gcmONERROR(_CheckSurface(Engine, gcvFALSE, gcvSURF_A8R8G8B8, &Address, &Stride,
        SurfaceWidth, SurfaceHeight, Rotation, gcvLINEAR));

    /* Fill in the structure. */
    surface                = &Engine->state.dstSurface;
    surface->type          = gcvSURF_BITMAP;
    surface->alignedW      = SurfaceWidth;
    surface->alignedH      = SurfaceHeight;
    surface->rotation      = Rotation;
    surface->stride        = Stride;

    if (Engine->hwAvailable)
    {
        gcsSURF_NODE_SetHardwareAddress(&surface->node, Address);
    }
    else
    {
        surface->node.logical = (gctUINT8_PTR)(gctUINTPTR_T) Address;
    }

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:

    /* Return status. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

/* Same as gco2D_SetTargetEx, but with better 64bit SW-path support.
** Please do NOT export the API now.
*/
gceSTATUS
gco2D_SetTarget64(
    IN gco2D Engine,
    IN gctUINT32 Address,
    IN gctPOINTER Logical,
    IN gctUINT32 Stride,
    IN gceSURF_ROTATION Rotation,
    IN gctUINT32 SurfaceWidth,
    IN gctUINT32 SurfaceHeight
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gcoSURF surface;

    gcmHEADER_ARG("Engine=0x%x Address=%x Logical=%p Stride=%d "
                  "Rotation=%d SurfaceWidth=%d SurfaceHeight=%d",
                  Engine, Address, Logical, Stride, Rotation, SurfaceWidth, SurfaceHeight);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    /* Assume the target format is gcvSURF_A8R8G8B8 because this API is only used
        to receive the RGB/BGR format surface and 32bpp would apply the max alignment. */
    gcmONERROR(_CheckSurface(Engine, gcvFALSE, gcvSURF_A8R8G8B8, &Address, &Stride,
        SurfaceWidth, SurfaceHeight, Rotation, gcvLINEAR));

    /* Fill in the structure. */
    surface                = &Engine->state.dstSurface;
    surface->type          = gcvSURF_BITMAP;
    surface->alignedW      = SurfaceWidth;
    surface->alignedH      = SurfaceHeight;
    surface->rotation      = Rotation;
    surface->stride        = Stride;

    if (Engine->hwAvailable)
    {
        gcsSURF_NODE_SetHardwareAddress(&surface->node, Address);
    }
    else
    {
        surface->node.logical = (gctUINT8_PTR)Logical;
    }

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:

    /* Return status. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

/*******************************************************************************
**
**  gco2D_CalcStretchFactor
**
**  Calculate the stretch factors based on the sizes.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctINT32 SrcSize
**          Source size for horizontal or vertical direction.
**
**      gctINT32 DstSize
**          Destination size for horizontal or vertical direction.
**
**  OUTPUT:
**
**      gctINT32_PTR Factor
**          Stretch factor in 16.16 fixed point format.
*/
gceSTATUS
gco2D_CalcStretchFactor(
    IN gco2D Engine,
    IN gctINT32 SrcSize,
    IN gctINT32 DstSize,
    OUT gctUINT32_PTR Factor
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gctUINT32 factor;

    gcmHEADER_ARG("Engine=0x%x SrcSize=0x%x DstSize=0x%x", Engine, SrcSize, DstSize);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);
    gcmVERIFY_ARGUMENT(Factor != gcvNULL);

    /* Calculate the stretch factors. */
    factor = gcoHARDWARE_GetStretchFactor(
        Engine->state.multiSrc[Engine->state.currentSrcIndex].enableGDIStretch,
        SrcSize, DstSize
        );

    if (factor == 0)
    {
        status = gcvSTATUS_NOT_SUPPORTED;
    }
    else
    {
        status = gcvSTATUS_OK;
        *Factor = factor;
    }

    /* Return status. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

/*******************************************************************************
**
**  gco2D_SetStretchFactors
**
**  Calculate and program the stretch factors.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT32 HorFactor
**          Horizontal stretch factor.
**
**      gctUINT32 VerFactor
**          Vertical stretch factor.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetStretchFactors(
    IN gco2D Engine,
    IN gctUINT32 HorFactor,
    IN gctUINT32 VerFactor
    )
{
#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x HorFactor=%d VerFactor=%d", Engine, HorFactor, VerFactor);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    Engine->state.multiSrc[Engine->state.currentSrcIndex].horFactor = HorFactor;
    Engine->state.multiSrc[Engine->state.currentSrcIndex].verFactor = VerFactor;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_SetStretchRectFactors
**
**  Calculate and program the stretch factors based on the rectangles.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gcsRECT_PTR SrcRect
**          Pointer to a valid source rectangle.
**
**      gcsRECT_PTR DstRect
**          Pointer to a valid destination rectangle.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetStretchRectFactors(
    IN gco2D Engine,
    IN gcsRECT_PTR SrcRect,
    IN gcsRECT_PTR DstRect
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x SrcRect=0x%x DstRect=0x%x", Engine, SrcRect, DstRect);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);
    gcmVERIFY_ARGUMENT(SrcRect != gcvNULL);
    gcmVERIFY_ARGUMENT(DstRect != gcvNULL);

    /* Calculate the stretch factors. */
    status = gcoHARDWARE_GetStretchFactors(
        Engine->state.multiSrc[Engine->state.currentSrcIndex].enableGDIStretch,
        SrcRect, DstRect,
        &Engine->state.multiSrc[Engine->state.currentSrcIndex].horFactor,
        &Engine->state.multiSrc[Engine->state.currentSrcIndex].verFactor
        );

    /* Return status. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

/*******************************************************************************
**
**  gco2D_ConstructSingleColorBrush
**
**  Create a new solid color gcoBRUSH object.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT32 ColorConvert
**          The value of the Color parameter is stored directly in internal
**          color register and is used either directly to initialize pattern
**          or is converted to the format of destination before it is used.
**          The later happens if ColorConvert is not zero.
**
**      gctUINT32 Color
**          The color value of the pattern. The value will be used to
**          initialize 8x8 pattern. If the value is in destination format,
**          set ColorConvert to 0. Otherwise, provide the value in ARGB8
**          format and set ColorConvert to 1 to instruct the hardware to
**          convert the value to the destination format before it is
**          actually used.
**
**      gctUINT64 Mask
**          64 bits of mask, where each bit corresponds to one pixel of 8x8
**          pattern. Each bit of the mask is used to determine transparency
**          of the corresponding pixel, in other words, each mask bit is used
**          to select between foreground or background ROPs. If the bit is 0,
**          background ROP is used on the pixel; if 1, the foreground ROP
**          is used. The mapping between Mask parameter bits and actual
**          pattern pixels is as follows:
**
**          +----+----+----+----+----+----+----+----+
**          |  7 |  6 |  5 |  4 |  3 |  2 |  1 |  0 |
**          +----+----+----+----+----+----+----+----+
**          | 15 | 14 | 13 | 12 | 11 | 10 |  9 |  8 |
**          +----+----+----+----+----+----+----+----+
**          | 23 | 22 | 21 | 20 | 19 | 18 | 17 | 16 |
**          +----+----+----+----+----+----+----+----+
**          | 31 | 30 | 29 | 28 | 27 | 26 | 25 | 24 |
**          +----+----+----+----+----+----+----+----+
**          | 39 | 38 | 37 | 36 | 35 | 34 | 33 | 32 |
**          +----+----+----+----+----+----+----+----+
**          | 47 | 46 | 45 | 44 | 43 | 42 | 41 | 40 |
**          +----+----+----+----+----+----+----+----+
**          | 55 | 54 | 53 | 52 | 51 | 50 | 49 | 48 |
**          +----+----+----+----+----+----+----+----+
**          | 63 | 62 | 61 | 60 | 59 | 58 | 57 | 56 |
**          +----+----+----+----+----+----+----+----+
**
**  OUTPUT:
**
**      gcoBRUSH * Brush
**          Pointer to the variable that will hold the gcoBRUSH object pointer.
*/
gceSTATUS
gco2D_ConstructSingleColorBrush(
    IN gco2D Engine,
    IN gctUINT32 ColorConvert,
    IN gctUINT32 Color,
    IN gctUINT64 Mask,
    gcoBRUSH * Brush
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x ColorConvert=%d Color=%x Mask=%llx",
                    Engine, ColorConvert, Color, Mask);

    if (Mask && gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_NO_COLORBRUSH_INDEX8) == gcvTRUE)
    {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);
    gcmVERIFY_ARGUMENT(Brush != gcvNULL);

    status = gcoBRUSH_ConstructSingleColor(
        gcvNULL,
        ColorConvert,
        Color,
        Mask,
        Brush
        );

    /* Return status. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

/*******************************************************************************
**
**  gco2D_ConstructMonochromeBrush
**
**  Create a new monochrome gcoBRUSH object.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT32 OriginX
**      gctUINT32 OriginY
**          Specifies the origin of the pattern in 0..7 range.
**
**      gctUINT32 ColorConvert
**          The values of FgColor and BgColor parameters are stored directly in
**          internal color registers and are used either directly to initialize
**          pattern or converted to the format of destination before actually
**          used. The later happens if ColorConvert is not zero.
**
**      gctUINT32 FgColor
**      gctUINT32 BgColor
**          Foreground and background colors of the pattern. The values will be
**          used to initialize 8x8 pattern. If the values are in destination
**          format, set ColorConvert to 0. Otherwise, provide the values in
**          ARGB8 format and set ColorConvert to 1 to instruct the hardware to
**          convert the values to the destination format before they are
**          actually used.
**
**      gctUINT64 Bits
**          64 bits of pixel bits. Each bit represents one pixel and is used
**          to choose between foreground and background colors. If the bit
**          is 0, the background color is used; otherwise the foreground color
**          is used. The mapping between Bits parameter and the actual pattern
**          pixels is the same as of the Mask parameter.
**
**      gctUINT64 Mask
**          64 bits of mask, where each bit corresponds to one pixel of 8x8
**          pattern. Each bit of the mask is used to determine transparency
**          of the corresponding pixel, in other words, each mask bit is used
**          to select between foreground or background ROPs. If the bit is 0,
**          background ROP is used on the pixel; if 1, the foreground ROP
**          is used. The mapping between Mask parameter bits and the actual
**          pattern pixels is as follows:
**
**          +----+----+----+----+----+----+----+----+
**          |  7 |  6 |  5 |  4 |  3 |  2 |  1 |  0 |
**          +----+----+----+----+----+----+----+----+
**          | 15 | 14 | 13 | 12 | 11 | 10 |  9 |  8 |
**          +----+----+----+----+----+----+----+----+
**          | 23 | 22 | 21 | 20 | 19 | 18 | 17 | 16 |
**          +----+----+----+----+----+----+----+----+
**          | 31 | 30 | 29 | 28 | 27 | 26 | 25 | 24 |
**          +----+----+----+----+----+----+----+----+
**          | 39 | 38 | 37 | 36 | 35 | 34 | 33 | 32 |
**          +----+----+----+----+----+----+----+----+
**          | 47 | 46 | 45 | 44 | 43 | 42 | 41 | 40 |
**          +----+----+----+----+----+----+----+----+
**          | 55 | 54 | 53 | 52 | 51 | 50 | 49 | 48 |
**          +----+----+----+----+----+----+----+----+
**          | 63 | 62 | 61 | 60 | 59 | 58 | 57 | 56 |
**          +----+----+----+----+----+----+----+----+
**
**  OUTPUT:
**
**      gcoBRUSH * Brush
**          Pointer to the variable that will hold the gcoBRUSH object pointer.
*/
gceSTATUS
gco2D_ConstructMonochromeBrush(
    IN gco2D Engine,
    IN gctUINT32 OriginX,
    IN gctUINT32 OriginY,
    IN gctUINT32 ColorConvert,
    IN gctUINT32 FgColor,
    IN gctUINT32 BgColor,
    IN gctUINT64 Bits,
    IN gctUINT64 Mask,
    gcoBRUSH * Brush
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x OriginX=%d OriginY=%d ColorConvert=%d "
                    "FgColor=%x BgColor=%x Bits=%lld Mask=%llx",
                    Engine, OriginX, OriginY, ColorConvert,
                    FgColor, BgColor, Bits, Mask);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);
    gcmVERIFY_ARGUMENT(Brush != gcvNULL);

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_NO_COLORBRUSH_INDEX8) == gcvTRUE)
    {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    status = gcoBRUSH_ConstructMonochrome(
        gcvNULL,
        OriginX,
        OriginY,
        ColorConvert,
        FgColor,
        BgColor,
        Bits,
        Mask,
        Brush
        );

    /* Return status. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

/*******************************************************************************
**
**  gco2D_ConstructColorBrush
**
**  Create a color gcoBRUSH object.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT32 OriginX
**      gctUINT32 OriginY
**          Specifies the origin of the pattern in 0..7 range.
**
**      gctPOINTER Address
**          Location of the pattern bitmap in system memory.
**
**      gceSURF_FORMAT Format
**          Format of the source bitmap.
**
**      gctUINT64 Mask
**          64 bits of mask, where each bit corresponds to one pixel of 8x8
**          pattern. Each bit of the mask is used to determine transparency
**          of the corresponding pixel, in other words, each mask bit is used
**          to select between foreground or background ROPs. If the bit is 0,
**          background ROP is used on the pixel; if 1, the foreground ROP
**          is used. The mapping between Mask parameter bits and the actual
**          pattern pixels is as follows:
**
**          +----+----+----+----+----+----+----+----+
**          |  7 |  6 |  5 |  4 |  3 |  2 |  1 |  0 |
**          +----+----+----+----+----+----+----+----+
**          | 15 | 14 | 13 | 12 | 11 | 10 |  9 |  8 |
**          +----+----+----+----+----+----+----+----+
**          | 23 | 22 | 21 | 20 | 19 | 18 | 17 | 16 |
**          +----+----+----+----+----+----+----+----+
**          | 31 | 30 | 29 | 28 | 27 | 26 | 25 | 24 |
**          +----+----+----+----+----+----+----+----+
**          | 39 | 38 | 37 | 36 | 35 | 34 | 33 | 32 |
**          +----+----+----+----+----+----+----+----+
**          | 47 | 46 | 45 | 44 | 43 | 42 | 41 | 40 |
**          +----+----+----+----+----+----+----+----+
**          | 55 | 54 | 53 | 52 | 51 | 50 | 49 | 48 |
**          +----+----+----+----+----+----+----+----+
**          | 63 | 62 | 61 | 60 | 59 | 58 | 57 | 56 |
**          +----+----+----+----+----+----+----+----+
**
**  OUTPUT:
**
**      gcoBRUSH * Brush
**          Pointer to the variable that will hold the gcoBRUSH object pointer.
*/
gceSTATUS
gco2D_ConstructColorBrush(
    IN gco2D Engine,
    IN gctUINT32 OriginX,
    IN gctUINT32 OriginY,
    IN gctPOINTER Address,
    IN gceSURF_FORMAT Format,
    IN gctUINT64 Mask,
    gcoBRUSH * Brush
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x OriginX=%d OriginY=%d Address=0x%x Format=%d Mask=%llx",
                    Engine, OriginX, OriginY, Address, Format, Mask);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);
    gcmVERIFY_ARGUMENT(Brush != gcvNULL);
    gcmVERIFY_ARGUMENT(Format != gcvSURF_UNKNOWN);

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_NO_COLORBRUSH_INDEX8) == gcvTRUE)
    {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    status = gcoBRUSH_ConstructColor(
        gcvNULL,
        OriginX,
        OriginY,
        Address,
        Format,
        Mask,
        Brush
        );

    /* Return status. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

/*******************************************************************************
**
**  gco2D_Clear
**
**  Clear one or more rectangular areas.
**  The color is specified in A8R8G8B8 format.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT32 RectCount
**          The number of rectangles to draw. The array of line positions
**          pointed to by Position parameter must have at least RectCount
**          positions.
**
**      gcsRECT_PTR Rect
**          Points to an array of positions in (x0, y0)-(x1, y1) format.
**
**      gctUINT32 Color32
**          A8R8G8B8 clear color value.
**
**      gctUINT8 FgRop
**          Foreground ROP to use with opaque pixels.
**
**      gctUINT8 BgRop
**          Background ROP to use with transparent pixels.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_Clear(
    IN gco2D Engine,
    IN gctUINT32 RectCount,
    IN gcsRECT_PTR Rect,
    IN gctUINT32 Color32,
    IN gctUINT8 FgRop,
    IN gctUINT8 BgRop,
    IN gceSURF_FORMAT DstFormat
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x RectCount=%d Rect=0x%x Color32=%x "
                    "FgRop=%x BgRop=%x DstFormat=%d",
                    Engine, RectCount, Rect, Color32,
                    FgRop, BgRop, DstFormat);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);
    gcmVERIFY_ARGUMENT(Rect != gcvNULL);
    gcmVERIFY_ARGUMENT(DstFormat != gcvSURF_UNKNOWN);

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_ANDROID_ONLY) == gcvTRUE &&
        (FgRop != BgRop ||
         (FgRop != 0xCC && FgRop != 0xF0 && FgRop != 0xAA)))
    {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    do {
        Engine->state.multiSrc[Engine->state.currentSrcIndex].fgRop = FgRop;
        Engine->state.multiSrc[Engine->state.currentSrcIndex].bgRop = BgRop;

        /* Set the target format. */
        Engine->state.dstSurface.format = DstFormat;

        Engine->state.clearColor = Color32;

        /* Clear. */
        gcmERR_BREAK(gcoHARDWARE_Clear2D(
            Engine->hardware,
            &Engine->state,
            RectCount,
            Rect
            ));
    }
    while (0);

    /* Return status. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

/*******************************************************************************
**
**  gco2D_Line
**
**  Draw one or more Bresenham lines.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT32 LineCount
**          The number of lines to draw. The array of line positions pointed
**          to by Position parameter must have at least LineCount positions.
**
**      gcsRECT_PTR Position
**          Points to an array of positions in (x0, y0)-(x1, y1) format.
**
**      gcoBRUSH Brush
**          Brush to use for drawing.
**
**      gctUINT8 FgRop
**          Foreground ROP to use with opaque pixels.
**
**      gctUINT8 BgRop
**          Background ROP to use with transparent pixels.
**
**      gceSURF_FORMAT DstFormat
**          Format of the destination buffer.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_Line(
    IN gco2D Engine,
    IN gctUINT32 LineCount,
    IN gcsRECT_PTR Position,
    IN gcoBRUSH Brush,
    IN gctUINT8 FgRop,
    IN gctUINT8 BgRop,
    IN gceSURF_FORMAT DstFormat
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gcs2D_State_PTR state = gcvNULL;
    gctBOOL useSrc = gcvFALSE;

    gcmHEADER_ARG("Engine=0x%x LineCount=%d Position=0x%x Brush=0x%x "
                    "FgRop=%x BgRop=%x DstFormat=%d",
                    Engine, LineCount, Position, Brush,
                    FgRop, BgRop, DstFormat);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);
    gcmVERIFY_ARGUMENT(LineCount > 0);
    gcmVERIFY_ARGUMENT(Position != gcvNULL);
    gcmVERIFY_OBJECT(Brush, gcvOBJ_BRUSH);
    gcmVERIFY_ARGUMENT(DstFormat != gcvSURF_UNKNOWN);

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_ANDROID_ONLY) == gcvTRUE &&
        !gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_DEC400_COMPRESSION))
    {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    state = &Engine->state;

    gcoHARDWARE_Get2DResourceUsage(
        FgRop,
        BgRop,
        state->multiSrc[state->currentSrcIndex].srcTransparency,
        &useSrc, gcvNULL, gcvNULL
        );

    if (!useSrc)

    {
        state->multiSrc[state->currentSrcIndex].fgRop = FgRop;
        state->multiSrc[state->currentSrcIndex].bgRop = BgRop;

        /* Set the target format. */
        Engine->state.dstSurface.format = DstFormat;

        gcmONERROR(gcoBRUSH_CACHE_FlushBrush(
            Engine->brushCache,
            Brush
            ));

        /* Draw the lines. */
        gcmONERROR(gcoHARDWARE_StartDELine(
            Engine->hardware,
            state,
            gcv2D_LINE,
            LineCount,
            Position,
            0,
            gcvNULL
            ));
    }
    else
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

OnError:
    /* Return status. */
    gcmFOOTER();
#endif
    return status;
}

/*******************************************************************************
**
**  gco2D_ColorLine
**
**  Draw one or more Bresenham lines based on the 32-bit color.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT32 LineCount
**          The number of lines to draw. The array of line positions pointed
**          to by Position parameter must have at least LineCount positions.
**
**      gcsRECT_PTR Position
**          Points to an array of positions in (x0, y0)-(x1, y1) format.
**
**      gctUINT32 Color32
**          Source color in A8R8G8B8 format.
**
**      gctUINT8 FgRop
**          Foreground ROP to use with opaque pixels.
**
**      gctUINT8 BgRop
**          Background ROP to use with transparent pixels.
**
**      gceSURF_FORMAT DstFormat
**          Format of the destination buffer.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_ColorLine(
    IN gco2D Engine,
    IN gctUINT32 LineCount,
    IN gcsRECT_PTR Position,
    IN gctUINT32 Color32,
    IN gctUINT8 FgRop,
    IN gctUINT8 BgRop,
    IN gceSURF_FORMAT DstFormat
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x LineCount=%d Position=0x%x Color32=%x "
                    "FgRop=%x BgRop=%x DstFormat=%d",
                    Engine, LineCount, Position, Color32,
                    FgRop, BgRop, DstFormat);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);
    gcmVERIFY_ARGUMENT(LineCount > 0);
    gcmVERIFY_ARGUMENT(Position != gcvNULL);
    gcmVERIFY_ARGUMENT(DstFormat != gcvSURF_UNKNOWN);

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_ANDROID_ONLY) == gcvTRUE &&
        !gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_DEC400_COMPRESSION))
    {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    do {
        Engine->state.multiSrc[Engine->state.currentSrcIndex].fgRop = FgRop;
        Engine->state.multiSrc[Engine->state.currentSrcIndex].bgRop = BgRop;

        /* Set the target format. */
        Engine->state.dstSurface.format = DstFormat;

        /* Draw the lines. */
        gcmERR_BREAK(gcoHARDWARE_Line2DEx(
            Engine->hardware,
            &Engine->state,
            LineCount,
            Position,
            1,
            &Color32
            ));
    }
    while (0);

    /* Return status. */
    gcmFOOTER();
#endif

    return status;
}

/*******************************************************************************
**
**  gco2D_Blit
**
**  Generic blit.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT32 RectCount
**          The number of rectangles to draw. The array of rectangle positions
**          pointed to by Rect parameter must have at least RectCount
**          positions.
**
**      gcsRECT_PTR Rect
**          Points to an array of positions in (x0, y0)-(x1, y1) format.
**
**      gctUINT8 FgRop
**          Foreground ROP to use with opaque pixels.
**
**      gctUINT8 BgRop
**          Background ROP to use with transparent pixels.
**
**      gceSURF_FORMAT DstFormat
**          Format of the destination buffer.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_Blit(
    IN gco2D Engine,
    IN gctUINT32 RectCount,
    IN gcsRECT_PTR Rect,
    IN gctUINT8 FgRop,
    IN gctUINT8 BgRop,
    IN gceSURF_FORMAT DstFormat
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x RectCount=%d Rect=0x%x FgRop=%x BgRop=%x DstFormat=%d",
                    Engine, RectCount, Rect, FgRop, BgRop, DstFormat);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_ANDROID_ONLY) == gcvTRUE &&
        (FgRop != BgRop ||
         (FgRop != 0xCC && FgRop != 0xF0 && FgRop != 0xAA)))
    {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    if ((RectCount == 0) || (Rect == gcvNULL)
        || (DstFormat == gcvSURF_UNKNOWN)
        || ((Engine->state.dstSurface.tileStatusConfig == gcv2D_TSC_2D_COMPRESSED)
        && (DstFormat != gcvSURF_A8R8G8B8) && (DstFormat != gcvSURF_X8R8G8B8)))
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    Engine->state.multiSrc[Engine->state.currentSrcIndex].fgRop = FgRop;
    Engine->state.multiSrc[Engine->state.currentSrcIndex].bgRop = BgRop;

    Engine->state.dstSurface.format = DstFormat;

    gcmONERROR(gcoHARDWARE_Blit(
        Engine->hardware,
        &Engine->state,
        0,
        gcvNULL,
        RectCount,
        Rect
        ));

OnError:
    /* Return status. */
    gcmFOOTER();
#endif

    return status;
}

/*******************************************************************************
**
**  gco2D_BatchBlit
**
**  Generic blit for a batch of source destination rectangle pairs.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT32 RectCount
**          The number of rectangles to draw. The array of rectangle positions
**          pointed to by SrcRect and DstRect parameters must have at least
**          RectCount positions.
**
**      gcsRECT_PTR SrcRect
**          Points to an array of positions in (x0, y0)-(x1, y1) format.
**
**      gcsRECT_PTR DstRect
**          Points to an array of positions in (x0, y0)-(x1, y1) format.
**
**      gctUINT8 FgRop
**          Foreground ROP to use with opaque pixels.
**
**      gctUINT8 BgRop
**          Background ROP to use with transparent pixels.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_BatchBlit(
    IN gco2D Engine,
    IN gctUINT32 RectCount,
    IN gcsRECT_PTR SrcRect,
    IN gcsRECT_PTR DstRect,
    IN gctUINT8 FgRop,
    IN gctUINT8 BgRop,
    IN gceSURF_FORMAT DstFormat
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;
#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x RectCount=%d SrcRect=0x%x DstRect=0x%x "
                    "FgRop=%x BgRop=%x DstFormat=%d",
                    Engine, RectCount, SrcRect, DstRect,
                    FgRop, BgRop, DstFormat);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);
    gcmVERIFY_ARGUMENT(RectCount > 0);
    gcmVERIFY_ARGUMENT(SrcRect != gcvNULL);
    gcmVERIFY_ARGUMENT(DstRect != gcvNULL);
    gcmVERIFY_ARGUMENT(DstFormat != gcvSURF_UNKNOWN);

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_ANDROID_ONLY) == gcvTRUE &&
        (FgRop != BgRop ||
         (FgRop != 0xCC && FgRop != 0xF0 && FgRop != 0xAA)))
    {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    do {
        Engine->state.multiSrc[Engine->state.currentSrcIndex].fgRop = FgRop;
        Engine->state.multiSrc[Engine->state.currentSrcIndex].bgRop = BgRop;

        /* Set the target format. */
        Engine->state.dstSurface.format = DstFormat;

        /* Start the DE engine. */
        gcmERR_BREAK(gcoHARDWARE_Blit(
             Engine->hardware,
             &Engine->state,
             RectCount,
             SrcRect,
             RectCount,
             DstRect
             ));
    }
    while (gcvFALSE);

    /* Return status. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

/*******************************************************************************
**
**  gco2D_StretchBlit
**
**  Stretch blit.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT32 RectCount
**          The number of rectangles to draw. The array of rectangle positions
**          pointed to by Rect parameter must have at least RectCount
**          positions.
**
**      gcsRECT_PTR Rect
**          Points to an array of rectangles. All rectangles are assumed to be
**          of the same size.
**
**      gctUINT8 FgRop
**          Foreground ROP to use with opaque pixels.
**
**      gctUINT8 BgRop
**          Background ROP to use with transparent pixels.
**
**      gceSURF_FORMAT DstFormat
**          Format of the destination buffer.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_StretchBlit(
    IN gco2D Engine,
    IN gctUINT32 RectCount,
    IN gcsRECT_PTR Rect,
    IN gctUINT8 FgRop,
    IN gctUINT8 BgRop,
    IN gceSURF_FORMAT DstFormat
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;
#if gcdENABLE_2D
    gctBOOL isYUV;
    gctUINT32 planes;

    gcmHEADER_ARG("Engine=0x%x RectCount=%d Rect=0x%x FgRop=%x BgRop=%x DstFormat=%d",
                    Engine, RectCount, Rect, FgRop, BgRop, DstFormat);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_ANDROID_ONLY) == gcvTRUE &&
        (FgRop != BgRop ||
         (FgRop != 0xCC && FgRop != 0xF0 && FgRop != 0xAA)))
    {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    gcmONERROR(_CheckFormat(DstFormat, &planes, gcvNULL, &isYUV));

    if ((!gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_YUV420_OUTPUT_LINEAR) &&
         ((planes != 1) || isYUV))
        || (RectCount == 0)
        || (Rect == gcvNULL))
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    Engine->state.multiSrc[Engine->state.currentSrcIndex].fgRop = FgRop;
    Engine->state.multiSrc[Engine->state.currentSrcIndex].bgRop = BgRop;

    /* Set the target format. */
    Engine->state.dstSurface.format = DstFormat;

    /* Set the source. */
    gcmONERROR(gcoHARDWARE_StartDE(
        Engine->hardware,
        &Engine->state,
        gcv2D_STRETCH,
        0,
        gcvNULL,
        RectCount,
        Rect
        ));

OnError:

    /* Return status. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

/*******************************************************************************
**
**  gco2D_MonoBlit
**
**  Monochrome blit.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctPOINTER StreamBits
**          Pointer to the monochrome bitmap.
**
**      gcsPOINT_PTR StreamSize
**          Size of the monochrome bitmap in pixels.
**
**      gcsRECT_PTR StreamRect
**          Bounding rectangle of the area within the bitmap to render.
**
**      gceSURF_MONOPACK SrcStreamPack
**          Source bitmap packing.
**
**      gceSURF_MONOPACK DstStreamPack
**          Packing of the bitmap in the command stream.
**
**      gcsRECT_PTR DstRect
**          Pointer to an array of destination rectangles.
**
**      gctUINT32 FgRop
**          Foreground and background ROP codes.
**
**      gctUINT32 BgRop
**          Background ROP to use with transparent pixels.
**
**      gceSURF_FORMAT DstFormat
**          Dstination surface format.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_MonoBlit(
    IN gco2D Engine,
    IN gctPOINTER StreamBits,
    IN gcsPOINT_PTR StreamSize,
    IN gcsRECT_PTR StreamRect,
    IN gceSURF_MONOPACK SrcStreamPack,
    IN gceSURF_MONOPACK DstStreamPack,
    IN gcsRECT_PTR DstRect,
    IN gctUINT32 FgRop,
    IN gctUINT32 BgRop,
    IN gceSURF_FORMAT DstFormat
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x StreamBits=0x%x StreamSize=%d StreamRect=0x%x "
                    "SrcStreamPack=%d DstStreamPack=%d DstRect=0x%x "
                    "FgRop=%x BgRop=%x DstFormat=%d",
                    Engine, StreamBits, StreamSize, StreamRect,
                    SrcStreamPack, DstStreamPack, DstRect,
                    FgRop, BgRop, DstFormat);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);
    gcmVERIFY_ARGUMENT(StreamBits != gcvNULL);
    gcmVERIFY_ARGUMENT(StreamSize != gcvNULL);
    gcmVERIFY_ARGUMENT(StreamRect != gcvNULL);
    gcmVERIFY_ARGUMENT(DstRect != gcvNULL);
    gcmVERIFY_ARGUMENT(DstFormat != gcvSURF_UNKNOWN);

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_ANDROID_ONLY) == gcvTRUE)
    {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    do
    {
        Engine->state.multiSrc[Engine->state.currentSrcIndex].fgRop = (gctUINT8)FgRop;
        Engine->state.multiSrc[Engine->state.currentSrcIndex].bgRop = (gctUINT8)BgRop;

        /* Set the target format. */
        Engine->state.dstSurface.format = DstFormat;

        /* Set the source. */
        gcmERR_BREAK(gcoHARDWARE_MonoBlit(
            Engine->hardware,
            &Engine->state,
            StreamBits,
            StreamSize,
            StreamRect,
            SrcStreamPack,
            DstStreamPack,
            DstRect
            ));
    }
    while (gcvFALSE);

    /* Return status. */
    gcmFOOTER();
#endif

    return status;
}

/*******************************************************************************
**
**  gco2D_SetKernelSize
**
**  Set kernel size.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT8 HorKernelSize
**          Kernel size for the horizontal pass.
**
**      gctUINT8 VerKernelSize
**          Kernel size for the vertical pass.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetKernelSize(
    IN gco2D Engine,
    IN gctUINT8 HorKernelSize,
    IN gctUINT8 VerKernelSize
    )
{
#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x HorKernelSize=%d VerKernelSize=%d",
                    Engine, HorKernelSize, VerKernelSize);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);
    gcmVERIFY_ARGUMENT((HorKernelSize & 1) && (HorKernelSize <= gcvMAXKERNELSIZE));
    gcmVERIFY_ARGUMENT((VerKernelSize & 1) && (VerKernelSize <= gcvMAXKERNELSIZE));

    /* Set sizes. */
    Engine->state.newHorKernelSize = HorKernelSize;
    Engine->state.newVerKernelSize = VerKernelSize;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif      /* gcvENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_SetFilterType
**
**  Set filter type.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gceFILTER_TYPE FilterType
**          Filter type for the filter blit.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetFilterType(
    IN gco2D Engine,
    IN gceFILTER_TYPE FilterType
    )
{
#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x FilterType=%d", Engine, FilterType);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    /* Set the new filter type. */
    switch (FilterType)
    {
    case gcvFILTER_SYNC:
        Engine->state.newFilterType = gcvFILTER_SYNC;
        break;

    case gcvFILTER_BLUR:
        Engine->state.newFilterType = gcvFILTER_BLUR;
        break;

    case gcvFILTER_USER:
        Engine->state.newFilterType = gcvFILTER_USER;
        break;

    default:
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* Return status. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif      /* gcvENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_SetUserFilterKernel
**
**  Set the user defined filter kernel.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gceFILTER_PASS_TYPE PassType
**          Pass type for the filter blit.
**
**      gctUINT16_PTR KernelArray
**          Pointer to the kernel array from user.
**
**      gctINT ArrayLen
**          Length of the kernel array in bytes.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetUserFilterKernel(
    IN gco2D Engine,
    IN gceFILTER_PASS_TYPE PassType,
    IN gctUINT16_PTR KernelArray
    )
{
#if gcdENABLE_2D
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Engine=0x%x PassType=%d KernelArray=0x%x", Engine, PassType, KernelArray);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);
    gcmVERIFY_ARGUMENT(KernelArray != gcvNULL);

    do
    {
        gcsFILTER_BLIT_ARRAY_PTR kernelInfo = gcvNULL;

        if (PassType == gcvFILTER_HOR_PASS)
        {
            kernelInfo = &Engine->state.horUserFilterKernel;
        }
        else if (PassType == gcvFILTER_VER_PASS)
        {
            kernelInfo = &Engine->state.verUserFilterKernel;
        }
        else
        {
            gcmTRACE_ZONE(gcvLEVEL_ERROR,
                          gcvZONE_HARDWARE,
                          "Unknown filter pass type.");

            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }

        /* Allocate the array if not allocated yet. */
        if (kernelInfo->kernelStates == gcvNULL)
        {
            gctPOINTER pointer = gcvNULL;

            /* Allocate the array. */
            gcmERR_BREAK(
                gcoOS_Allocate(gcvNULL,
                               gcvKERNELSTATES,
                               &pointer));

            kernelInfo->kernelStates = pointer;
        }

        gcoOS_MemCopy(kernelInfo->kernelStates + 1,
                      KernelArray,
                      gcvKERNELTABLESIZE);

        kernelInfo->kernelChanged = gcvTRUE;
    }
    while (gcvFALSE);

    if (gcmIS_ERROR(status))
    {
        gcmTRACE_ZONE(gcvLEVEL_INFO,
                gcvZONE_HARDWARE,
                "Failed to set the user filter array."
                );
    }

    /* Return status. */
    gcmFOOTER();
    return status;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_EnableUserFilterPasses
**
**  Select the pass(es) to be done for user defined filter.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctBOOL HorPass
**          Enable horizontal filter pass if HorPass is gcvTRUE.
**          Otherwise disable this pass.
**
**      gctBOOL VerPass
**          Enable vertical filter pass if VerPass is gcvTRUE.
**          Otherwise disable this pass.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_EnableUserFilterPasses(
    IN gco2D Engine,
    IN gctBOOL HorPass,
    IN gctBOOL VerPass
    )
{
#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x HorPass=%d VerPass=%d", Engine, HorPass, VerPass);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    Engine->state.horUserFilterPass = HorPass;
    Engine->state.verUserFilterPass = VerPass;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_FreeFilterBuffer
**
**  Frees the temporary buffer allocated by filter blit operation.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_FreeFilterBuffer(
    IN gco2D Engine
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x", Engine);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    status = gcoHARDWARE_FreeFilterBuffer(
        Engine->hardware
        );

    /* Return status. */
    gcmFOOTER();
#endif
    return status;
}

/*******************************************************************************
**
**  gco2D_FilterBlit
**
**  Filter blit. This function is deprecated. Please use gco2D_ FilterBlitEx2
**  instead.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT32 SrcAddress
**          Base address of the source surface in local memory.
**
**      gctUINT SrcStride
**          Stride of the source surface in bytes.
**
**      gctUINT32 SrcUAddress
**          Base address of U channel of the source surface in local memory for YUV format.
**
**      gctUINT SrcUStride
**          Stride of U channel of the source surface in bytes for YUV format.
**
**      gctUINT32 SrcVAddress
**          Base address of V channel of the source surface in local memory for YUV format.
**
**      gctUINT SrcVStride
**          Stride of of V channel the source surface in bytes for YUV format.
**
**      gceSURF_FORMAT SrcFormat
**          Format of the source surface.
**
**      gceSURF_ROTATION SrcRotation
**          Specifies the source surface rotation angle.
**
**      gctUINT32 SrcSurfaceWidth
**          The width in pixels of the source surface.
**
**      gcsRECT_PTR SrcRect
**          Coordinates of the entire source image.
**
**      gctUINT32 DstAddress
**          Base address of the destination surface in local memory.
**
**      gctUINT DstStride
**          Stride of the destination surface in bytes.
**
**      gceSURF_FORMAT DstFormat
**          Format of the destination surface.
**
**      gceSURF_ROTATION DstRotation
**          Specifies the destination surface rotation angle.
**
**      gctUINT32 DstSurfaceWidth
**          The width in pixels of the destination surface.
**
**      gcsRECT_PTR DstRect
**          Coordinates of the entire destination image.
**
**      gcsRECT_PTR DstSubRect
**          Coordinates of a sub area within the destination to render.
**          If DstSubRect is gcvNULL, the complete image will be rendered
**          using coordinates set by DstRect.
**          If DstSubRect is not gcvNULL and DstSubRect and DstRect are
**          no equal, DstSubRect is assumed to be within DstRect and
**          will be used to render the sub area only.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_FilterBlit(
    IN gco2D Engine,
    IN gctUINT32 SrcAddress,
    IN gctUINT SrcStride,
    IN gctUINT32 SrcUAddress,
    IN gctUINT SrcUStride,
    IN gctUINT32 SrcVAddress,
    IN gctUINT SrcVStride,
    IN gceSURF_FORMAT SrcFormat,
    IN gceSURF_ROTATION SrcRotation,
    IN gctUINT32 SrcSurfaceWidth,
    IN gcsRECT_PTR SrcRect,
    IN gctUINT32 DstAddress,
    IN gctUINT DstStride,
    IN gceSURF_FORMAT DstFormat,
    IN gceSURF_ROTATION DstRotation,
    IN gctUINT32 DstSurfaceWidth,
    IN gcsRECT_PTR DstRect,
    IN gcsRECT_PTR DstSubRect
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x SrcAddress=%x SrcStride=%d SrcUAddress=%x SrcUStride=%d "
                    "SrcVAddress=%x SrcVStride=%d SrcFormat=%d SrcRotation=%d "
                    "SrcSurfaceWidth=%d SrcRect=0x%x "
                    "DstAddress=%x DstStride=%d DstFormat=%d DstRotation=%d "
                    "DstSurfaceWidth=%d DstRect=0x%x DstSubRect=0x%x",
                    Engine, SrcAddress, SrcStride, SrcUAddress, SrcUStride,
                    SrcVAddress, SrcVStride, SrcFormat, SrcRotation,
                    SrcSurfaceWidth, SrcRect,
                    DstAddress, DstStride, DstFormat, DstRotation,
                    DstSurfaceWidth, DstRect, DstSubRect);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);
    gcmVERIFY_ARGUMENT(SrcFormat != gcvSURF_UNKNOWN);
    gcmVERIFY_ARGUMENT(SrcRect != gcvNULL);
    gcmVERIFY_ARGUMENT(DstFormat != gcvSURF_UNKNOWN);
    gcmVERIFY_ARGUMENT(DstRect != gcvNULL);

    /* This interface does not support full rotations. */
    gcmVERIFY_ARGUMENT((SrcRotation == gcvSURF_0_DEGREE) || (SrcRotation == gcvSURF_90_DEGREE));
    gcmVERIFY_ARGUMENT((DstRotation == gcvSURF_0_DEGREE) || (DstRotation == gcvSURF_90_DEGREE));

    /* Forward to gco2D_FilterBlitEx with the DstHeight set to 0
        and SrcHeight set to 0. */
    status = gco2D_FilterBlitEx(
                            Engine,
                            SrcAddress,
                            SrcStride,
                            SrcUAddress,
                            SrcUStride,
                            SrcVAddress,
                            SrcVStride,
                            SrcFormat,
                            SrcRotation,
                            SrcSurfaceWidth,
                            0,
                            SrcRect,
                            DstAddress,
                            DstStride,
                            DstFormat,
                            DstRotation,
                            DstSurfaceWidth,
                            0,
                            DstRect,
                            DstSubRect
                            );

    /* Return status. */
    gcmFOOTER();
#endif
    return status;
}

/*******************************************************************************
**
**  gco2D_FilterBlitEx
**
**  Filter blit. This function is deprecated. Please use gco2D_ FilterBlitEx2
**  instead.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT32 SrcAddress
**          Base address of the source surface in local memory.
**
**      gctUINT SrcStride
**          Stride of the source surface in bytes.
**
**      gctUINT32 SrcUAddress
**          Base address of U channel of the source surface in local memory for YUV format.
**
**      gctUINT SrcUStride
**          Stride of U channel of the source surface in bytes for YUV format.
**
**      gctUINT32 SrcVAddress
**          Base address of V channel of the source surface in local memory for YUV format.
**
**      gctUINT SrcVStride
**          Stride of of V channel the source surface in bytes for YUV format.
**
**      gceSURF_FORMAT SrcFormat
**          Format of the source surface.
**
**      gceSURF_ROTATION SrcRotation
**          Specifies the source surface rotation angle.
**
**      gctUINT32 SrcSurfaceWidth
**          The width in pixels of the source surface.
**
**      gctUINT32 SrcSurfaceHeight
**          The height in pixels of the source surface for the rotaion in PE 2.0.
**
**      gcsRECT_PTR SrcRect
**          Coordinates of the entire source image.
**
**      gctUINT32 DstAddress
**          Base address of the destination surface in local memory.
**
**      gctUINT DstStride
**          Stride of the destination surface in bytes.
**
**      gceSURF_FORMAT DstFormat
**          Format of the destination surface.
**
**      gceSURF_ROTATION DstRotation
**          Specifies the destination surface rotation angle.
**
**      gctUINT32 DstSurfaceWidth
**          The width in pixels of the destination surface.
**
**      gctUINT32 DstSurfaceHeight
**          The height in pixels of the destination surface for the rotation in PE 2.0.
**
**      gcsRECT_PTR DstRect
**          Coordinates of the entire destination image.
**
**      gcsRECT_PTR DstSubRect
**          Coordinates of a sub area within the destination to render.
**          If DstSubRect is gcvNULL, the complete image will be rendered
**          using coordinates set by DstRect.
**          If DstSubRect is not gcvNULL and DstSubRect and DstRect are
**          no equal, DstSubRect is assumed to be within DstRect and
**          will be used to render the sub area only.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_FilterBlitEx(
    IN gco2D Engine,
    IN gctUINT32 SrcAddress,
    IN gctUINT SrcStride,
    IN gctUINT32 SrcUAddress,
    IN gctUINT SrcUStride,
    IN gctUINT32 SrcVAddress,
    IN gctUINT SrcVStride,
    IN gceSURF_FORMAT SrcFormat,
    IN gceSURF_ROTATION SrcRotation,
    IN gctUINT32 SrcSurfaceWidth,
    IN gctUINT32 SrcSurfaceHeight,
    IN gcsRECT_PTR SrcRect,
    IN gctUINT32 DstAddress,
    IN gctUINT DstStride,
    IN gceSURF_FORMAT DstFormat,
    IN gceSURF_ROTATION DstRotation,
    IN gctUINT32 DstSurfaceWidth,
    IN gctUINT32 DstSurfaceHeight,
    IN gcsRECT_PTR DstRect,
    IN gcsRECT_PTR DstSubRect
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gctUINT32 n;
    gctUINT32 addr[3]={0}, stride[3]={0};
    gcoSURF srcSurface;
    gcoSURF destSurface;

    gcmHEADER_ARG("Engine=0x%x SrcAddress=%x SrcStride=%d SrcUAddress=%x SrcUStride=%d "
                    "SrcVAddress=%x SrcVStride=%d SrcFormat=%d SrcRotation=%d "
                    "SrcSurfaceWidth=%d SrcSurfaceHeight=%d SrcRect=0x%x "
                    "DstAddress=%x DstStride=%d DstFormat=%d DstRotation=%d "
                    "DstSurfaceWidth=%d DstSurfaceHeight=%d DstRect=0x%x DstSubRect=0x%x",
                    Engine, SrcAddress, SrcStride, SrcUAddress, SrcUStride,
                    SrcVAddress, SrcVStride, SrcFormat, SrcRotation,
                    SrcSurfaceWidth, SrcSurfaceHeight, SrcRect,
                    DstAddress, DstStride, DstFormat, DstRotation,
                    DstSurfaceWidth, DstSurfaceHeight, DstRect, DstSubRect);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);
    gcmVERIFY_ARGUMENT(SrcRect != gcvNULL);
    gcmVERIFY_ARGUMENT((SrcRect->left < SrcRect->right)
        && (SrcRect->top < SrcRect->bottom)
        && (SrcRect->right >= 0) && (SrcRect->right < 0x8000)
        && (SrcRect->bottom >= 0) && (SrcRect->bottom < 0x8000));
    gcmVERIFY_ARGUMENT(DstRect != gcvNULL);
    gcmVERIFY_ARGUMENT((DstRect->left < DstRect->right)
        && (DstRect->top < DstRect->bottom)
        && (DstRect->right >= 0) && (DstRect->right < 0x8000)
        && (DstRect->bottom >= 0) && (DstRect->bottom < 0x8000));

    gcmONERROR(_CheckFormat(SrcFormat, &n, gcvNULL, gcvNULL));

    switch (n)
    {
    case 3:
        addr[2] = SrcVAddress;
        stride[2] = SrcVStride;

    case 2:
        addr[1] = SrcUAddress;
        stride[1] = SrcUStride;

    case 1:
        addr[0] = SrcAddress;
        stride[0] = SrcStride;
        break;

    default:
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmONERROR(_CheckSurface(Engine, gcvTRUE, SrcFormat, addr, stride,
        SrcSurfaceWidth, SrcSurfaceHeight, SrcRotation, gcvLINEAR));

    gcmONERROR(_CheckFormat(DstFormat, &n, gcvNULL, gcvNULL));

    if (n != 1
        || gcmHAS2DCOMPRESSION(&Engine->state.dstSurface)
        || gcmHAS2DCOMPRESSION(&Engine->state.multiSrc[Engine->state.currentSrcIndex].srcSurface))
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmONERROR(_CheckSurface(Engine, gcvFALSE, DstFormat, &DstAddress, &DstStride,
        DstSurfaceWidth, DstSurfaceHeight, DstRotation, gcvLINEAR));

    /* Fill in the source structure. */
    srcSurface = &Engine->state.multiSrc[Engine->state.currentSrcIndex].srcSurface;
    srcSurface->type             = gcvSURF_BITMAP;
    srcSurface->format           = SrcFormat;
    srcSurface->alignedW         = SrcSurfaceWidth;
    srcSurface->alignedH         = SrcSurfaceHeight;
    srcSurface->rotation         = SrcRotation;
    srcSurface->stride           = SrcStride;

    gcsSURF_NODE_SetHardwareAddress(&srcSurface->node, SrcAddress);

    srcSurface->uStride          = SrcUStride;
    srcSurface->node.physical2   = SrcUAddress;
    srcSurface->vStride          = SrcVStride;
    srcSurface->node.physical3   = SrcVAddress;
    srcSurface->tiling           = gcvLINEAR;

    /* Fill in the target structure. */
    destSurface = &Engine->state.dstSurface;
    destSurface->type          = gcvSURF_BITMAP;
    destSurface->format        = DstFormat;
    destSurface->alignedW      = DstSurfaceWidth;
    destSurface->alignedH      = DstSurfaceHeight;
    destSurface->rotation      = DstRotation;
    destSurface->stride        = DstStride;

    gcsSURF_NODE_SetHardwareAddress(&destSurface->node, DstAddress);

    destSurface->tiling        = gcvLINEAR;

    status = gcoHARDWARE_SplitFilterBlit(
        Engine->hardware,
        &Engine->state,
        srcSurface,
        destSurface,
        SrcRect,
        DstRect,
        DstSubRect);

    if (status != gcvSTATUS_OK)
    {
        status = gcoHARDWARE_FilterBlit(
            Engine->hardware,
            &Engine->state,
            srcSurface,
            destSurface,
            SrcRect,
            DstRect,
            DstSubRect
            );
    }

OnError:
    /* Return status. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

/*******************************************************************************
**
**  gco2D_FilterBlitEx2
**
**  Filter blit.
**
**  Note:
**      If the output format is multi planar YUV, do only color conversion.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT32 SrcAddress
**          Base address of the source surface in local memory.
**
**      gctUINT SrcStride
**          Stride of the source surface in bytes.
**
**      gctUINT32 SrcUAddress
**          Base address of U channel of the source surface in local memory for YUV format.
**
**      gctUINT SrcUStride
**          Stride of U channel of the source surface in bytes for YUV format.
**
**      gctUINT32 SrcVAddress
**          Base address of V channel of the source surface in local memory for YUV format.
**
**      gctUINT SrcVStride
**          Stride of of V channel the source surface in bytes for YUV format.
**
**      gceSURF_FORMAT SrcFormat
**          Format of the source surface.
**
**      gceSURF_ROTATION SrcRotation
**          Specifies the source surface rotation angle.
**
**      gctUINT32 SrcSurfaceWidth
**          The width in pixels of the source surface.
**
**      gctUINT32 SrcSurfaceHeight
**          The height in pixels of the source surface for the rotaion in PE 2.0.
**
**      gcsRECT_PTR SrcRect
**          Coordinates of the entire source image.
**
**      gctUINT32 DstAddress
**          Base address of the destination surface in local memory.
**
**      gctUINT DstStride
**          Stride of the destination surface in bytes.
**
**      gceSURF_FORMAT DstFormat
**          Format of the destination surface.
**
**      gceSURF_ROTATION DstRotation
**          Specifies the destination surface rotation angle.
**
**      gctUINT32 DstSurfaceWidth
**          The width in pixels of the destination surface.
**
**      gctUINT32 DstSurfaceHeight
**          The height in pixels of the destination surface for the rotaion in PE 2.0.
**
**      gcsRECT_PTR DstRect
**          Coordinates of the entire destination image.
**
**      gcsRECT_PTR DstSubRect
**          Coordinates of a sub area within the destination to render.
**          If DstSubRect is gcvNULL, the complete image will be rendered
**          using coordinates set by DstRect.
**          If DstSubRect is not gcvNULL and DstSubRect and DstRect are
**          no equal, DstSubRect is assumed to be within DstRect and
**          will be used to render the sub area only.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_FilterBlitEx2(
    IN gco2D                Engine,
    IN gctUINT32_PTR        SrcAddresses,
    IN gctUINT32            SrcAddressNum,
    IN gctUINT32_PTR        SrcStrides,
    IN gctUINT32            SrcStrideNum,
    IN gceTILING            SrcTiling,
    IN gceSURF_FORMAT       SrcFormat,
    IN gceSURF_ROTATION     SrcRotation,
    IN gctUINT32            SrcSurfaceWidth,
    IN gctUINT32            SrcSurfaceHeight,
    IN gcsRECT_PTR          SrcRect,
    IN gctUINT32_PTR        DstAddresses,
    IN gctUINT32            DstAddressNum,
    IN gctUINT32_PTR        DstStrides,
    IN gctUINT32            DstStrideNum,
    IN gceTILING            DstTiling,
    IN gceSURF_FORMAT       DstFormat,
    IN gceSURF_ROTATION     DstRotation,
    IN gctUINT32            DstSurfaceWidth,
    IN gctUINT32            DstSurfaceHeight,
    IN gcsRECT_PTR          DstRect,
    IN gcsRECT_PTR          DstSubRect
    )
{
#if gcdENABLE_2D
    gcoSURF srcSurface;
    gcoSURF destSurface;
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 n;
    gctBOOL multiYUV = gcvFALSE;

    gcmHEADER_ARG("Engine=0x%x SrcAddresses=%x SrcStrideNum=%d SrcStride=%x SrcStrideNum=%d "
                    "SrcTiling=%d SrcFormat=%d SrcRotation=%d "
                    "SrcSurfaceWidth=%d SrcSurfaceHeight=%d SrcRect=0x%x "
                    "DstAddresses=%x DstAddressNum=%d DstStrides=%x DstStrideNum=%d DstTiling=%d DstFormat=%d DstRotation=%d "
                    "DstSurfaceWidth=%d DstSurfaceHeight=%d DstRect=0x%x DstSubRect=0x%x",
                    Engine, SrcAddresses, SrcStrideNum, SrcStrides, SrcStrideNum,
                    SrcTiling, SrcFormat, SrcRotation,
                    SrcSurfaceWidth, SrcSurfaceHeight, SrcRect,
                    DstAddresses, DstAddressNum, DstStrides, DstStrideNum, DstTiling, DstFormat, DstRotation,
                    DstSurfaceWidth, DstSurfaceHeight, DstRect, DstSubRect);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);
    gcmVERIFY_ARGUMENT(SrcRect != gcvNULL);
    gcmVERIFY_ARGUMENT((SrcRect->left < SrcRect->right)
        && (SrcRect->top < SrcRect->bottom)
        && (SrcRect->right >= 0) && (SrcRect->right < 0x8000)
        && (SrcRect->bottom >= 0) && (SrcRect->bottom < 0x8000));
    gcmVERIFY_ARGUMENT(DstRect != gcvNULL);
    gcmVERIFY_ARGUMENT((DstRect->left < DstRect->right)
        && (DstRect->top < DstRect->bottom)
        && (DstRect->right >= 0) && (DstRect->right < 0x8000)
        && (DstRect->bottom >= 0) && (DstRect->bottom < 0x8000));

    gcmONERROR(_CheckFormat(SrcFormat, &n, gcvNULL, gcvNULL));

    if ((n > SrcAddressNum) || (SrcAddressNum > 3) || (SrcStrideNum > 3))
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmONERROR(_CheckSurface(Engine, gcvTRUE, SrcFormat, SrcAddresses, SrcStrides,
        SrcSurfaceWidth, SrcSurfaceHeight, SrcRotation, SrcTiling));

    gcmONERROR(_CheckFormat(DstFormat, &n, gcvNULL, gcvNULL));

    if ((n > DstAddressNum) || (DstAddressNum > 3) || (DstStrideNum > 3)
        || gcmHAS2DCOMPRESSION(&Engine->state.dstSurface)
        || gcmHAS2DCOMPRESSION(&Engine->state.multiSrc[Engine->state.currentSrcIndex].srcSurface))
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmONERROR(_CheckSurface(Engine, gcvFALSE, DstFormat, DstAddresses, DstStrides,
        DstSurfaceWidth, DstSurfaceHeight, DstRotation, DstTiling));

    switch (DstFormat)
    {
    case gcvSURF_I420:
    case gcvSURF_YV12:
    case gcvSURF_NV16:
    case gcvSURF_NV12:
    case gcvSURF_NV61:
    case gcvSURF_NV21:
        if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_YUV420_OUTPUT_LINEAR) != gcvTRUE)
        {
            /*All version of multisource blit support split YUV*/
            if ((((gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_MULTI_SOURCE_BLT) != gcvTRUE)  &&
                (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_MULTI_SOURCE_BLT_EX) != gcvTRUE) &&
                (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_MULTI_SOURCE_BLT_EX2) != gcvTRUE) &&
                (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_MULTI_SRC_BLT_TO_UNIFIED_DST_RECT) != gcvTRUE))
                 || ((SrcFormat != gcvSURF_YUY2) && (SrcFormat != gcvSURF_UYVY)
                     && (SrcFormat != gcvSURF_YVYU) && (SrcFormat != gcvSURF_VYUY)))
                &&(SrcFormat != DstFormat))
            {
                gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            }

            if (SrcFormat != DstFormat &&
                ((SrcRect->right != DstRect->right)
                || (SrcRect->left !=  DstRect->left)
                || (SrcRect->bottom != DstRect->bottom)
                || (SrcRect->top != DstRect->top)
                || (SrcRotation != gcvSURF_0_DEGREE)
                || (DstRotation != gcvSURF_0_DEGREE)))
            {
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }

            multiYUV = gcvTRUE;
        }
        break;

    default:
        break;
    }

    /* Fill in the source structure. */
    srcSurface = &Engine->state.multiSrc[Engine->state.currentSrcIndex].srcSurface;
    srcSurface->type             = gcvSURF_BITMAP;
    srcSurface->format           = SrcFormat;
    srcSurface->tiling           = SrcTiling;
    srcSurface->alignedW         = SrcSurfaceWidth;
    srcSurface->alignedH         = SrcSurfaceHeight;
    srcSurface->rotation         = SrcRotation;

    switch (SrcAddressNum)
    {
        case 3:
            srcSurface->node.physical3 = SrcAddresses[2];
            /*fall through*/

        case 2:
            srcSurface->node.physical2 = SrcAddresses[1];
            /*fall through*/

        case 1:
            gcsSURF_NODE_SetHardwareAddress(&srcSurface->node, SrcAddresses[0]);
            break;

        default:
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    switch (SrcStrideNum)
    {
        case 3:
            srcSurface->vStride = SrcStrides[2];
            /*fall through*/

        case 2:
            srcSurface->uStride = SrcStrides[1];
            /*fall through*/

        case 1:
            srcSurface->stride = SrcStrides[0];
            break;

        default:
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Fill in the target structure. */
    destSurface = &Engine->state.dstSurface;
    destSurface->type          = gcvSURF_BITMAP;
    destSurface->format        = DstFormat;
    destSurface->tiling        = DstTiling;
    destSurface->alignedW      = DstSurfaceWidth;
    destSurface->alignedH      = DstSurfaceHeight;
    destSurface->rotation      = DstRotation;

    switch (DstAddressNum)
    {
        case 3:
            destSurface->node.physical3 = DstAddresses[2];
            /*fall through*/

        case 2:
            destSurface->node.physical2 = DstAddresses[1];
            /*fall through*/

        case 1:
            gcsSURF_NODE_SetHardwareAddress(&destSurface->node, DstAddresses[0]);
            break;

        default:
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    switch (DstStrideNum)
    {
        case 3:
            destSurface->vStride = DstStrides[2];
            /*fall through*/

        case 2:
            destSurface->uStride = DstStrides[1];
            /*fall through*/

        case 1:
            destSurface->stride = DstStrides[0];
            break;

        default:
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (multiYUV)
    {
        if ((SrcFormat == gcvSURF_I420 && DstFormat == gcvSURF_I420) ||
            (SrcFormat == gcvSURF_YV12 && DstFormat == gcvSURF_YV12) ||
            (SrcFormat == gcvSURF_NV12 && DstFormat == gcvSURF_NV12) ||
            (SrcFormat == gcvSURF_NV21 && DstFormat == gcvSURF_NV21) ||
            (SrcFormat == gcvSURF_NV16 && DstFormat == gcvSURF_NV16) ||
            (SrcFormat == gcvSURF_NV61 && DstFormat == gcvSURF_NV61))
        {
            status = gcoHARDWARE_SplitYUVFilterBlit(
                    Engine->hardware,
                    &Engine->state,
                    srcSurface,
                    destSurface,
                    SrcRect,
                    DstRect,
                    DstSubRect
                    );
        }
        else
        {
            status = gcoHARDWARE_MultiPlanarYUVConvert(
                    Engine->hardware,
                    &Engine->state,
                    srcSurface,
                    destSurface,
                    DstRect
                    );
        }
    }
    else
    {
        status = gcoHARDWARE_SplitFilterBlit(
            Engine->hardware,
            &Engine->state,
            srcSurface,
            destSurface,
            SrcRect,
            DstRect,
            DstSubRect);

        if (status != gcvSTATUS_OK)
        {
            status = gcoHARDWARE_FilterBlit(
                Engine->hardware,
                &Engine->state,
                srcSurface,
                destSurface,
                SrcRect,
                DstRect,
                DstSubRect
                );
        }
    }

OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif
}

/*******************************************************************************
**
**  gco2D_EnableAlphaBlend
**
**  For normal Blit,
**   On old PE (<2.0), ROP4 must be set as 0x8888 when enable alpha blending.
**   On new PE (>=2.0), ROP4 is not limited when enable alpha blending.
**
**  For FilterBlit:
**   ROP4 is always set as 0xCCCC internally.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT8 SrcGlobalAlphaValue
**      gctUINT8 DstGlobalAlphaValue
**          Global alpha value for the color components.
**
**      gceSURF_PIXEL_ALPHA_MODE SrcAlphaMode
**      gceSURF_PIXEL_ALPHA_MODE DstAlphaMode
**          Per-pixel alpha component mode.
**
**      gceSURF_GLOBAL_ALPHA_MODE SrcGlobalAlphaMode
**      gceSURF_GLOBAL_ALPHA_MODE DstGlobalAlphaMode
**          Global/per-pixel alpha values selection.
**
**      gceSURF_BLEND_FACTOR_MODE SrcFactorMode
**      gceSURF_BLEND_FACTOR_MODE DstFactorMode
**          Final blending factor mode.
**
**      gceSURF_PIXEL_COLOR_MODE SrcColorMode
**      gceSURF_PIXEL_COLOR_MODE DstColorMode
**          Per-pixel color component mode.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_EnableAlphaBlend(
    IN gco2D Engine,
    IN gctUINT8 SrcGlobalAlphaValue,
    IN gctUINT8 DstGlobalAlphaValue,
    IN gceSURF_PIXEL_ALPHA_MODE SrcAlphaMode,
    IN gceSURF_PIXEL_ALPHA_MODE DstAlphaMode,
    IN gceSURF_GLOBAL_ALPHA_MODE SrcGlobalAlphaMode,
    IN gceSURF_GLOBAL_ALPHA_MODE DstGlobalAlphaMode,
    IN gceSURF_BLEND_FACTOR_MODE SrcFactorMode,
    IN gceSURF_BLEND_FACTOR_MODE DstFactorMode,
    IN gceSURF_PIXEL_COLOR_MODE SrcColorMode,
    IN gceSURF_PIXEL_COLOR_MODE DstColorMode
    )
{
#if gcdENABLE_2D
    gcs2D_MULTI_SOURCE_PTR curSrc;

    gcmHEADER_ARG("Engine=0x%x SrcGlobalAlphaValue=%x DstGlobalAlphaValue=%d "
                    "SrcAlphaMode=%x DstAlphaMode=%d "
                    "SrcGlobalAlphaMode=%d DstGlobalAlphaMode=%d "
                    "SrcFactorMode=%x DstFactorMode=%d SrcColorMode=%d DstColorMode=%d",
                    Engine, SrcGlobalAlphaValue, DstGlobalAlphaValue,
                    SrcAlphaMode, DstAlphaMode, SrcGlobalAlphaMode, DstGlobalAlphaMode,
                    SrcFactorMode, DstFactorMode, SrcColorMode, DstColorMode);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    /* Enable blending. */
    curSrc = Engine->state.multiSrc + Engine->state.currentSrcIndex;

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2DPE20) == gcvTRUE)
    {
        if ((SrcColorMode == gcvSURF_COLOR_MULTIPLY)
           ||(DstColorMode == gcvSURF_COLOR_MULTIPLY))
        {
            gce2D_PIXEL_COLOR_MULTIPLY_MODE srcPremultiply = gcv2D_COLOR_MULTIPLY_DISABLE;
            gce2D_PIXEL_COLOR_MULTIPLY_MODE dstPremultiply = gcv2D_COLOR_MULTIPLY_DISABLE;
            gce2D_GLOBAL_COLOR_MULTIPLY_MODE srcPremultiplyGlobal = gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE;

            /* Color modes are not directly supported in new PE.
               User should use Premultiply modes instead.
               Driver using premultiply modes for old PE where possible. */
            if (SrcColorMode == gcvSURF_COLOR_MULTIPLY)
            {
                if (SrcAlphaMode != gcvSURF_PIXEL_ALPHA_STRAIGHT)
                {
                    gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
                    return gcvSTATUS_NOT_SUPPORTED;
                }

                if ((SrcGlobalAlphaMode == gcvSURF_GLOBAL_ALPHA_OFF)
                  ||(SrcGlobalAlphaMode == gcvSURF_GLOBAL_ALPHA_SCALE))
                {
                    srcPremultiply = gcv2D_COLOR_MULTIPLY_ENABLE;
                }

                if ((SrcGlobalAlphaMode == gcvSURF_GLOBAL_ALPHA_ON)
                  ||(SrcGlobalAlphaMode == gcvSURF_GLOBAL_ALPHA_SCALE))
                {
                    srcPremultiplyGlobal = gcv2D_GLOBAL_COLOR_MULTIPLY_ALPHA;
                }
            }

            if (DstColorMode == gcvSURF_COLOR_MULTIPLY)
            {
                if (DstAlphaMode != gcvSURF_PIXEL_ALPHA_STRAIGHT)
                {
                    gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
                    return gcvSTATUS_NOT_SUPPORTED;
                }

                if (DstGlobalAlphaMode == gcvSURF_GLOBAL_ALPHA_OFF)
                {
                    dstPremultiply = gcv2D_COLOR_MULTIPLY_ENABLE;
                }
                else
                {
                    gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
                    return gcvSTATUS_NOT_SUPPORTED;
                }
            }

            if (srcPremultiply != gcv2D_COLOR_MULTIPLY_DISABLE)
            {
                curSrc->srcPremultiplyMode = srcPremultiply;
            }

            if (srcPremultiplyGlobal != gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE)
            {
                curSrc->srcPremultiplyGlobalMode = srcPremultiplyGlobal;
            }

            if (dstPremultiply != gcv2D_COLOR_MULTIPLY_DISABLE)
            {
                curSrc->dstPremultiplyMode = dstPremultiply;
            }
        }

        curSrc->srcColorMode = gcvSURF_COLOR_STRAIGHT;
        curSrc->dstColorMode = gcvSURF_COLOR_STRAIGHT;
    }
    else
    {
        curSrc->srcColorMode = SrcColorMode;
        curSrc->dstColorMode = DstColorMode;
    }

    curSrc->enableAlpha = gcvTRUE;
    curSrc->srcGlobalColor = (curSrc->srcGlobalColor & 0x00FFFFFF)
        | (((gctUINT32)SrcGlobalAlphaValue & 0xFF) << 24);
    curSrc->dstGlobalColor = (curSrc->dstGlobalColor & 0x00FFFFFF)
        | (((gctUINT32)DstGlobalAlphaValue & 0xFF) << 24);
    curSrc->srcAlphaMode = SrcAlphaMode;
    curSrc->dstAlphaMode = DstAlphaMode;
    curSrc->srcGlobalAlphaMode = SrcGlobalAlphaMode;
    curSrc->dstGlobalAlphaMode = DstGlobalAlphaMode;
    curSrc->srcFactorMode = SrcFactorMode;
    curSrc->dstFactorMode = DstFactorMode;

    /* Return status. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_EnableAlphaBlendAdvanced
**
**  Enable alpha blending engine in the hardware.
**
**  This function is only working with PE 2.0 and above.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gceSURF_PIXEL_ALPHA_MODE SrcAlphaMode
**      gceSURF_PIXEL_ALPHA_MODE DstAlphaMode
**          Per-pixel alpha component mode.
**
**      gceSURF_GLOBAL_ALPHA_MODE SrcGlobalAlphaMode
**      gceSURF_GLOBAL_ALPHA_MODE DstGlobalAlphaMode
**          Global/per-pixel alpha values selection.
**
**      gceSURF_BLEND_FACTOR_MODE SrcFactorMode
**      gceSURF_BLEND_FACTOR_MODE DstFactorMode
**          Final blending factor mode.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_EnableAlphaBlendAdvanced(
    IN gco2D Engine,
    IN gceSURF_PIXEL_ALPHA_MODE SrcAlphaMode,
    IN gceSURF_PIXEL_ALPHA_MODE DstAlphaMode,
    IN gceSURF_GLOBAL_ALPHA_MODE SrcGlobalAlphaMode,
    IN gceSURF_GLOBAL_ALPHA_MODE DstGlobalAlphaMode,
    IN gceSURF_BLEND_FACTOR_MODE SrcFactorMode,
    IN gceSURF_BLEND_FACTOR_MODE DstFactorMode
    )
{
#if gcdENABLE_2D
    gcs2D_MULTI_SOURCE_PTR curSrc;

    gcmHEADER_ARG("Engine=0x%x SrcAlphaMode=%x DstAlphaMode=%d "
                    "SrcGlobalAlphaMode=%x DstGlobalAlphaMode=%d "
                    "SrcFactorMode=%x DstFactorMode=%d",
                    Engine, SrcAlphaMode, DstAlphaMode,
                    SrcGlobalAlphaMode, DstGlobalAlphaMode,
                    SrcFactorMode, DstFactorMode);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    curSrc = Engine->state.multiSrc + Engine->state.currentSrcIndex;
    curSrc->enableAlpha = gcvTRUE;
    curSrc->srcAlphaMode = SrcAlphaMode;
    curSrc->dstAlphaMode = DstAlphaMode;
    curSrc->srcGlobalAlphaMode = SrcGlobalAlphaMode;
    curSrc->dstGlobalAlphaMode = DstGlobalAlphaMode;
    curSrc->srcFactorMode = SrcFactorMode;
    curSrc->dstFactorMode = DstFactorMode;
    curSrc->srcColorMode = gcvSURF_COLOR_STRAIGHT;
    curSrc->dstColorMode = gcvSURF_COLOR_STRAIGHT;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_SetPorterDuffBlending
**
**  Enable alpha blending engine in the hardware and setup the blending modes
**  using the Porter Duff rule defined.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gce2D_PORTER_DUFF_RULE Rule
**          Porter Duff blending rule.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetPorterDuffBlending(
    IN gco2D Engine,
    IN gce2D_PORTER_DUFF_RULE Rule
    )
{
#if gcdENABLE_2D
    gceSURF_BLEND_FACTOR_MODE srcFactor, dstFactor;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Engine=0x%x Rule=%d", Engine, Rule);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    switch(Rule)
    {
    case gcvPD_CLEAR:
        srcFactor = gcvSURF_BLEND_ZERO;
        dstFactor = gcvSURF_BLEND_ZERO;
        break;

    case gcvPD_SRC:
        srcFactor = gcvSURF_BLEND_ONE;
        dstFactor = gcvSURF_BLEND_ZERO;
        break;

    case gcvPD_SRC_OVER:
        srcFactor = gcvSURF_BLEND_ONE;
        dstFactor = gcvSURF_BLEND_INVERSED;
        break;

    case gcvPD_DST_OVER:
        srcFactor = gcvSURF_BLEND_INVERSED;
        dstFactor = gcvSURF_BLEND_ONE;
        break;

    case gcvPD_SRC_IN:
        srcFactor = gcvSURF_BLEND_STRAIGHT;
        dstFactor = gcvSURF_BLEND_ZERO;
        break;

    case gcvPD_DST_IN:
        srcFactor = gcvSURF_BLEND_ZERO;
        dstFactor = gcvSURF_BLEND_STRAIGHT;
        break;

    case gcvPD_SRC_OUT:
        srcFactor = gcvSURF_BLEND_INVERSED;
        dstFactor = gcvSURF_BLEND_ZERO;
        break;

    case gcvPD_DST_OUT:
        srcFactor = gcvSURF_BLEND_ZERO;
        dstFactor = gcvSURF_BLEND_INVERSED;
        break;

    case gcvPD_SRC_ATOP:
        srcFactor = gcvSURF_BLEND_STRAIGHT;
        dstFactor = gcvSURF_BLEND_INVERSED;
        break;

    case gcvPD_DST_ATOP:
        srcFactor = gcvSURF_BLEND_INVERSED;
        dstFactor = gcvSURF_BLEND_STRAIGHT;
        break;

    case gcvPD_ADD:
        srcFactor = gcvSURF_BLEND_ONE;
        dstFactor = gcvSURF_BLEND_ONE;
        break;

    case gcvPD_XOR:
        srcFactor = gcvSURF_BLEND_INVERSED;
        dstFactor = gcvSURF_BLEND_INVERSED;
        break;

    default:
        gcmFOOTER_NO();
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    status = gco2D_EnableAlphaBlendAdvanced(
        Engine,
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_GLOBAL_ALPHA_OFF,
        srcFactor,
        dstFactor
        );

    /* Return status. */
    gcmFOOTER();
    return status;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_DisableAlphaBlend
**
**  Disable alpha blending engine in the hardware and engage the ROP engine.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_DisableAlphaBlend(
    IN gco2D Engine
    )
{
#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x", Engine);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    Engine->state.multiSrc[Engine->state.currentSrcIndex].enableAlpha = gcvFALSE;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_GetPackSize
**
**  Retrieve monochrome stream pack size.
**
**  INPUT:
**
**      gceSURF_MONOPACK StreamPack
**          Stream pack code.
**
**  OUTPUT:
**
**      gctUINT32 * PackWidth
**      gctUINT32 * PackHeight
**          Monochrome stream pack size.
*/
gceSTATUS
gco2D_GetPackSize(
    IN gceSURF_MONOPACK StreamPack,
    OUT gctUINT32 * PackWidth,
    OUT gctUINT32 * PackHeight
    )
{
#if gcdENABLE_2D
    gcmHEADER_ARG("StreamPack=0x%x", StreamPack);

    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT(PackWidth != gcvNULL);
    gcmVERIFY_ARGUMENT(PackHeight != gcvNULL);

    /* Dispatch on monochrome packing. */
    switch (StreamPack)
    {
    case gcvSURF_PACKED8:
        *PackWidth  = 8;
        *PackHeight = 4;
        break;

    case gcvSURF_PACKED16:
        *PackWidth  = 16;
        *PackHeight = 2;
        break;

    case gcvSURF_PACKED32:
    case gcvSURF_UNPACKED:
        *PackWidth  = 32;
        *PackHeight = 1;
        break;

    default:
        /* Not supprted. */
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* Success. */
    gcmFOOTER_ARG("*PackWidth=%d *PackHeight=%d",
                    *PackWidth, *PackHeight);
    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_Flush
**
**  Flush the 2D pipeline.
**
**  INPUT:
**      gco2D Engine
**          Pointer to an gco2D object.
**
**  OUTPUT:
**
**      Nothing.
*/

gceSTATUS
gco2D_Flush(
    IN gco2D Engine
    )
{
#if gcdENABLE_2D
    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif
}

/*******************************************************************************
**
**  gco2D_LoadPalette
**
**  Load 256-entry color table for INDEX8 source surfaces.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to the gco2D object.
**
**      gctUINT FirstIndex
**          The index to start loading from (0..255).
**
**      gctUINT IndexCount
**          The number of indices to load (FirstIndex + IndexCount <= 256).
**
**      gctPOINTER ColorTable
**          Pointer to the color table to load. The value of the pointer should
**          be set to the first value to load no matter what the value of
**          FirstIndex is. The table must consist of 32-bit entries that contain
**          color values in either ARGB8 or the destination color format
**          (see ColorConvert).
**
**      gctBOOL ColorConvert
**          If set to gcvTRUE, the 32-bit values in the table are assumed to be
**          in ARGB8 format and will be converted by the hardware to the
**          destination format as needed.
**          If set to gcvFALSE, the 32-bit values in the table are assumed to be
**          preconverted to the destination format.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_LoadPalette(
    IN gco2D Engine,
    IN gctUINT FirstIndex,
    IN gctUINT IndexCount,
    IN gctPOINTER ColorTable,
    IN gctBOOL ColorConvert
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x FirstIndex=%d IndexCount=%d ColorTable=0x%x ColorConvert=%d",
                    Engine, FirstIndex, IndexCount, ColorTable, ColorConvert);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);
    gcmVERIFY_ARGUMENT(FirstIndex < 256);
    gcmVERIFY_ARGUMENT(IndexCount <= 256);
    gcmVERIFY_ARGUMENT(ColorTable != gcvNULL);

    if (Engine->state.paletteTable == gcvNULL)
    {
        gctPOINTER pointer = gcvNULL;

        gcmONERROR(gcoOS_Allocate(
            gcvNULL,
            sizeof(gctUINT32)*256,
            &pointer
            ));

        Engine->state.paletteTable = pointer;
    }

    gcoOS_MemCopy(Engine->state.paletteTable, ColorTable, IndexCount * 4);
    Engine->state.paletteIndexCount = IndexCount;
    Engine->state.paletteFirstIndex = FirstIndex;
    Engine->state.paletteConvert    = ColorConvert;
    Engine->state.paletteProgram    = gcvTRUE;

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:

    /* Return status. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

/*******************************************************************************
**
**  gco2D_SetBitBlitMirror
**
**  Enable/disable 2D BitBlt mirrorring.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to the gco2D object.
**
**      gctBOOL HorizontalMirror
**          Horizontal mirror enable flag.
**
**      gctBOOL VerticalMirror
**          Vertical mirror enable flag.
**
**  OUTPUT:
**
**      gceSTATUS
**          Returns gcvSTATUS_OK if successful.
*/
gceSTATUS
gco2D_SetBitBlitMirror(
    IN gco2D Engine,
    IN gctBOOL HorizontalMirror,
    IN gctBOOL VerticalMirror
    )
{
#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x HorizontalMirror=%d VerticalMirror=%d",
                    Engine, HorizontalMirror, VerticalMirror);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    Engine->state.multiSrc[Engine->state.currentSrcIndex].horMirror = HorizontalMirror;
    Engine->state.multiSrc[Engine->state.currentSrcIndex].verMirror = VerticalMirror;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_SetTransparencyAdvancedEx
**
**  Setup the source, target and pattern transparency modes.
**  It also enable or disable DFB color key mode.
**
**  This function is only working with full DFB 2D core.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to the gco2D object.
**
**      gce2D_TRANSPARENCY SrcTransparency
**          Source Transparency.
**
**      gce2D_TRANSPARENCY DstTransparency
**          Destination Transparency.
**
**      gce2D_TRANSPARENCY PatTransparency
**          Pattern Transparency.
**
**      gctBOOL EnableDFBColorKeyMode
**          Enable/disable DFB color key mode.
**          The transparent pixels will be bypassed when
**          enabling DFB color key mode. Otherwise those
**          pixels maybe processed by the following pipes.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetTransparencyAdvancedEx(
    IN gco2D Engine,
    IN gce2D_TRANSPARENCY SrcTransparency,
    IN gce2D_TRANSPARENCY DstTransparency,
    IN gce2D_TRANSPARENCY PatTransparency,
    IN gctBOOL EnableDFBColorKeyMode
    )
{
#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x SrcTransparency=%d DstTransparency=%d PatTransparency=%d, EnableDFBColorKeyMode=%d",
                    Engine, SrcTransparency, DstTransparency, PatTransparency, EnableDFBColorKeyMode);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_ANDROID_ONLY) == gcvTRUE)
    {
        if (SrcTransparency != gcv2D_OPAQUE ||
            DstTransparency != gcv2D_OPAQUE ||
            PatTransparency != gcv2D_OPAQUE)
        {
            gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
            return gcvSTATUS_NOT_SUPPORTED;
        }
    }

    Engine->state.multiSrc[Engine->state.currentSrcIndex].srcTransparency = SrcTransparency;
    Engine->state.multiSrc[Engine->state.currentSrcIndex].dstTransparency = DstTransparency;
    Engine->state.multiSrc[Engine->state.currentSrcIndex].patTransparency = PatTransparency;
    Engine->state.multiSrc[Engine->state.currentSrcIndex].enableDFBColorKeyMode = EnableDFBColorKeyMode;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_SetTransparencyAdvanced
**
**  Set the transparency for source, destination and pattern. This function is
**  deprecated. Please use gco2D_SetTransparencyAdvancedEx instead.
**
**  This function is only working with PE 2.0 and above.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to the gco2D object.
**
**      gce2D_TRANSPARENCY SrcTransparency
**          Source Transparency.
**
**      gce2D_TRANSPARENCY DstTransparency
**          Destination Transparency.
**
**      gce2D_TRANSPARENCY PatTransparency
**          Pattern Transparency.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetTransparencyAdvanced(
    IN gco2D Engine,
    IN gce2D_TRANSPARENCY SrcTransparency,
    IN gce2D_TRANSPARENCY DstTransparency,
    IN gce2D_TRANSPARENCY PatTransparency
    )
{
#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x SrcTransparency=%d DstTransparency=%d PatTransparency=%d",
                    Engine, SrcTransparency, DstTransparency, PatTransparency);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_ANDROID_ONLY) == gcvTRUE)
    {
        if (SrcTransparency != gcv2D_OPAQUE ||
            DstTransparency != gcv2D_OPAQUE ||
            PatTransparency != gcv2D_OPAQUE)
        {
            gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
            return gcvSTATUS_NOT_SUPPORTED;
        }
    }

    Engine->state.multiSrc[Engine->state.currentSrcIndex].srcTransparency = SrcTransparency;
    Engine->state.multiSrc[Engine->state.currentSrcIndex].dstTransparency = DstTransparency;
    Engine->state.multiSrc[Engine->state.currentSrcIndex].patTransparency = PatTransparency;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_SetROP
**
**  Set the ROP for source, destination and pattern.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to the gco2D object.
**
**      gctUINT8 FgROP
**          Foreground ROP.
**
**      gctUINT8 BgROP
**          Background ROP.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetROP(
    IN gco2D Engine,
    IN gctUINT8 FgROP,
    IN gctUINT8 BgROP
    )
{
#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x FgROP=%02x BgROP=%02x",
                    Engine, FgROP, BgROP);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    Engine->state.multiSrc[Engine->state.currentSrcIndex].fgRop = FgROP;
    Engine->state.multiSrc[Engine->state.currentSrcIndex].bgRop = BgROP;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_SetSourceColorKeyAdvanced
**
**  Set the source color key.
**  Color channel values should specified in the range allowed by the source format.
**  When target format is A8, only Alpha component is used. Otherwise, Alpha component
**  is not used.
**
**  This function is only working with PE 2.0 and above.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to the gco2D object.
**
**      gctUINT32 ColorKey
**          The color key value in A8R8G8B8 format.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetSourceColorKeyAdvanced(
    IN gco2D Engine,
    IN gctUINT32 ColorKey
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;
#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x ColorKey=%d", Engine, ColorKey);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    status = gco2D_SetSourceColorKeyRangeAdvanced(Engine, ColorKey, ColorKey);

    /* Return status. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

/*******************************************************************************
**
**  gco2D_SetSourceColorKeyRangeAdvanced
**
**  Set the source color key range.
**  Color channel values should specified in the range allowed by the source format.
**  Lower color key's color channel values should be less than or equal to
**  the corresponding color channel value of ColorKeyHigh.
**  When target format is A8, only Alpha components are used. Otherwise, Alpha
**  components are not used.
**
**  This function is only working with PE 2.0 and above.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to the gco2D object.
**
**      gctUINT32 ColorKeyLow
**          The low color key value in A8R8G8B8 format.
**
**      gctUINT8 ColorKeyHigh
**          The high color key value in A8R8G8B8 format.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetSourceColorKeyRangeAdvanced(
    IN gco2D Engine,
    IN gctUINT32 ColorKeyLow,
    IN gctUINT32 ColorKeyHigh
    )
{
#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x ColorKeyLow=%d ColorKeyHigh=%d",
                    Engine, ColorKeyLow, ColorKeyHigh);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);
    gcmVERIFY_ARGUMENT(ColorKeyLow <= ColorKeyHigh);

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2DPE20) != gcvTRUE)
    {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    Engine->state.multiSrc[Engine->state.currentSrcIndex].srcMonoTransparencyColor =
    Engine->state.multiSrc[Engine->state.currentSrcIndex].srcColorKeyLow = ColorKeyLow;
    Engine->state.multiSrc[Engine->state.currentSrcIndex].srcColorKeyHigh = ColorKeyHigh;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif      /* gcvENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_SetTargetColorKeyAdvanced
**
**  Set the target color key.
**  Color channel values should specified in the range allowed by the target format.
**  When target format is A8, only Alpha component is used. Otherwise, Alpha component
**  is not used.
**
**  This function is only working with PE 2.0 and above.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to the gco2D object.
**
**      gctUINT32 ColorKey
**          The color key value in A8R8G8B8 format.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetTargetColorKeyAdvanced(
    IN gco2D Engine,
    IN gctUINT32 ColorKey
    )
{
#if gcdENABLE_2D
    gceSTATUS status;

    gcmHEADER_ARG("Engine=0x%x ColorKey=%d", Engine, ColorKey);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    /* Relay the call. */
    status = gco2D_SetTargetColorKeyRangeAdvanced(
        Engine,
        ColorKey,
        ColorKey
        );

    /* Return status. */
    gcmFOOTER();
    return status;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_SetTargetColorKeyRangeAdvanced
**
**  Set the source color key range.
**  Color channel values should specified in the range allowed by the target format.
**  Lower color key's color channel values should be less than or equal to
**  the corresponding color channel value of ColorKeyHigh.
**  When target format is A8, only Alpha components are used. Otherwise, Alpha
**  components are not used.
**
**  This function is only working with PE 2.0 and above.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to the gco2D object.
**
**      gctUINT32 ColorKeyLow
**          The low color key value in A8R8G8B8 format.
**
**      gctUINT32 ColorKeyHigh
**          The high color key value in A8R8G8B8 format.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetTargetColorKeyRangeAdvanced(
    IN gco2D Engine,
    IN gctUINT32 ColorKeyLow,
    IN gctUINT32 ColorKeyHigh
    )
{
#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x ColorKeyLow=%d ColorKeyHigh=%d",
                    Engine, ColorKeyLow, ColorKeyHigh);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);
    gcmVERIFY_ARGUMENT(ColorKeyLow <= ColorKeyHigh);

    if ((gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2DPE20) != gcvTRUE)
        && (ColorKeyLow != ColorKeyHigh))
    {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    Engine->state.dstColorKeyHigh = ColorKeyHigh;
    Engine->state.dstColorKeyLow = ColorKeyLow;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_SetYUVColorMode
**
**  Set the YUV color space mode.
**
**  This function is only working with PE 2.0 and above.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to the gco2D object.
**
**      gce2D_YUV_COLOR_MODE Mode
**          Mode is 601, 709 or user defined conversion.
**          gcv2D_YUV_USER_DEFINED_CLAMP means clampping Y to [16, 235], and
**          U/V to [16, 240].
**
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetYUVColorMode(
    IN gco2D Engine,
    IN gce2D_YUV_COLOR_MODE Mode
    )
{
#if gcdENABLE_2D
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Engine=0x%x Mode=%d", Engine, Mode);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);
    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_YUV_MODE) != gcvTRUE)
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TPCV11_COMPRESSION) == gcvTRUE &&
        Mode == gcv2D_YUV_601)
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    if (((gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_COLOR_SPACE_CONVERSION) != gcvTRUE) ||
         (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NO_USER_CSC) == gcvTRUE))
        && ((Mode & gcv2D_YUV_DST) || (Mode > gcv2D_YUV_709)))
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    if (Mode & gcv2D_YUV_DST)
    {
        Engine->state.dstYUVMode = Mode & (~gcv2D_YUV_DST);
    }
    else
    {
        Engine->state.multiSrc[Engine->state.currentSrcIndex].srcYUVMode = Mode;
    }

OnError:
    /* Succeed. */
    gcmFOOTER();
    return status;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_SetSourceGlobalColorAdvanced
**
**  Set the source global color value in A8R8G8B8 format.
**
**  This function is only working with PE 2.0 and above.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to the gco2D object.
**
**      gctUINT32 Color32
**          Source color.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS gco2D_SetSourceGlobalColorAdvanced(
    IN gco2D Engine,
    IN gctUINT32 Color32
    )
{
#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x Color32=%x", Engine, Color32);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2DPE20) != gcvTRUE)
    {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    Engine->state.multiSrc[Engine->state.currentSrcIndex].srcGlobalColor = Color32;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_SetTargetGlobalColor
**
**  Set the source global color value in A8R8G8B8 format.
**
**  This function is only working with PE 2.0 and above.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to the gco2D object.
**
**      gctUINT32 Color32
**          Target color.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS gco2D_SetTargetGlobalColorAdvanced(
    IN gco2D Engine,
    IN gctUINT32 Color32
    )
{
#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x Color32=%x", Engine, Color32);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2DPE20) != gcvTRUE)
    {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    Engine->state.multiSrc[Engine->state.currentSrcIndex].dstGlobalColor = Color32;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_SetPixelMultiplyModesAdvanced
**
**  Set the source and target pixel multiply modes.
**
**  This function is only working with PE 2.0 and above.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to the gco2D object.
**
**      gce2D_PIXEL_COLOR_MULTIPLY_MODE SrcPremultiplySrcAlpha
**          Source color premultiply with Source Alpha.
**
**      gce2D_PIXEL_COLOR_MULTIPLY_MODE DstPremultiplyDstAlpha
**          Destination color premultiply with Destination Alpha.
**
**      gce2D_GLOBAL_COLOR_MULTIPLY_MODE SrcPremultiplyGlobalMode
**          Source color premultiply with Global color's Alpha or Color.
**
**      gce2D_PIXEL_COLOR_MULTIPLY_MODE DstDemultiplyDstAlpha
**          Destination color demultiply with Destination Alpha.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS gco2D_SetPixelMultiplyModeAdvanced(
    IN gco2D Engine,
    IN gce2D_PIXEL_COLOR_MULTIPLY_MODE SrcPremultiplySrcAlpha,
    IN gce2D_PIXEL_COLOR_MULTIPLY_MODE DstPremultiplyDstAlpha,
    IN gce2D_GLOBAL_COLOR_MULTIPLY_MODE SrcPremultiplyGlobalMode,
    IN gce2D_PIXEL_COLOR_MULTIPLY_MODE DstDemultiplyDstAlpha
    )
{
#if gcdENABLE_2D
    gcs2D_MULTI_SOURCE_PTR curSrc;
    gceCHIPMODEL chipModel;
    gctUINT32 chipRevision;

    gcmHEADER_ARG("Engine=0x%x SrcPremultiplySrcAlpha=%d DstPremultiplyDstAlpha=%d "
                    "SrcPremultiplyGlobalMode=%d DstDemultiplyDstAlpha=%d",
                    Engine, SrcPremultiplySrcAlpha, DstPremultiplyDstAlpha,
                    SrcPremultiplyGlobalMode, DstDemultiplyDstAlpha);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2DPE20) != gcvTRUE)
    {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    if (DstDemultiplyDstAlpha == gcv2D_COLOR_MULTIPLY_ENABLE)
    {
        gcmVERIFY_OK(gcoHAL_QueryChipIdentity(gcvNULL, &chipModel, &chipRevision, gcvNULL, gcvNULL));

        if (chipModel == gcv520 && chipRevision < 0x5520)
        {
            gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
            return gcvSTATUS_NOT_SUPPORTED;
        }
    }

    curSrc = Engine->state.multiSrc + Engine->state.currentSrcIndex;
    curSrc->srcPremultiplyMode = SrcPremultiplySrcAlpha;
    curSrc->dstPremultiplyMode = DstPremultiplyDstAlpha;
    curSrc->srcPremultiplyGlobalMode = SrcPremultiplyGlobalMode;
    curSrc->dstDemultiplyMode = DstDemultiplyDstAlpha;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_SetAutoFlushCycles
**
**  Set the GPU clock cycles, after which the idle 2D engine
**  will trigger a flush.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to the gco2D object.
**
**      UINT32 Cycles
**          Source color premultiply with Source Alpha.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS gco2D_SetAutoFlushCycles(
    IN gco2D Engine,
    IN gctUINT32 Cycles
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;
#if gcdENABLE_2D

    gcmHEADER_ARG("Engine=0x%x Cycles=%d", Engine, Cycles);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2DPE20) != gcvTRUE)
    {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* Relay the call. */
    status = gcoHARDWARE_SetAutoFlushCycles(
        Engine->hardware,
        Cycles
        );

    /* Return status. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

/*******************************************************************************
**
**  gco2D_ProfileEngine
**
**  Read the profile registers available in the 2D engine and set them in the profile.
**  pixelsRendered counter is reset to 0 after reading.
**
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to the gco2D object.
**
**      OPTIONAL gcs2D_PROFILE_PTR Profile
**          Pointer to a gcs2D_Profile structure.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_ProfileEngine(
    IN gco2D Engine,
    OPTIONAL gcs2D_PROFILE_PTR Profile
    )
{
#if (VIVANTE_PROFILER && gcdENABLE_2D)
    gcsHAL_INTERFACE iface;
    gceSTATUS status;

    gcmHEADER_ARG("Engine=0x%x Profile=0x%x", Engine, Profile);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    /* Read all 2D profile registers. */
    iface.ignoreTLS = gcvFALSE;
    iface.command = gcvHAL_PROFILE_REGISTERS_2D;
    iface.u.RegisterProfileData2D.hwProfile2D = gcmPTR_TO_UINT64(Profile);

    /* Call the kernel. */
    status = gcoOS_DeviceControl(
        gcvNULL,
        IOCTL_GCHAL_INTERFACE,
        &iface, gcmSIZEOF(iface),
        &iface, gcmSIZEOF(iface)
        );

    /* Return status. */
    gcmFOOTER();
    return status;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif /* VIVANTE_PROFILER */
}

/*******************************************************************************
**
**  gco2D_EnableDither.
**
**  Enable or disable dithering.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctBOOL Enable
**          gcvTRUE to enable dithering, gcvFALSE to disable.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_EnableDither(
    IN gco2D Engine,
    IN gctBOOL Enable
    )
{
#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x Enable=%d", Engine, Enable);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    if ((gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_DITHER) != gcvTRUE)
        && (Enable != gcvFALSE))
    {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* Program dithering. */
    Engine->state.dither    = Enable;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_SetGenericSource
**
**  Configure color source for linear, tile, super-tile, multi-tile. Also for
**      YUV format source.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT32 Address
**          Source surface base addresses.
**
**      gctUINT32 AddressNum
**          Number of source surface base addresses.
**
**      gctUINT32 Strides
**          Strides of the source surface in bytes.
**
**      gctUINT32 StrideNum
**          Number of stride of the source surface.
**
**      gceSURF_FORMAT Format
**          Color format of the source surface.
**
**      gceSURF_ROTATION Rotation
**          Type of rotation.
**
**      gctUINT32 SurfaceWidth
**          Required only if the surface is rotated. Equal to the width
**          of the source surface in pixels.
**
**      gctUINT32 SurfaceHeight
**          Required only if the surface is rotated in PE2.0. Equal to the height
**          of the source surface in pixels.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetGenericSource(
    IN gco2D               Engine,
    IN gctUINT32_PTR       Addresses,
    IN gctUINT32           AddressNum,
    IN gctUINT32_PTR       Strides,
    IN gctUINT32           StrideNum,
    IN gceTILING           Tiling,
    IN gceSURF_FORMAT      Format,
    IN gceSURF_ROTATION    Rotation,
    IN gctUINT32           SurfaceWidth,
    IN gctUINT32           SurfaceHeight
)
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gcoSURF surface;
    gctUINT32 n;

    gcmHEADER_ARG("Engine=0x%x Addresses=0x%08x AddressNum=%d Strides=%d StrideNum=%d "
                    "Tiling=%d Format=%d Rotation=%d "
                    "SurfaceWidth=%d SurfaceHeight=%d ",
                    Engine, Addresses, AddressNum, Strides, StrideNum,
                    Tiling, Format, Rotation,
                    SurfaceWidth, SurfaceHeight);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    gcmONERROR(_CheckFormat(Format, &n, gcvNULL, gcvNULL));

    if ((n > AddressNum) || (AddressNum > 3) || (StrideNum > 3))
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmONERROR(_CheckSurface(Engine, gcvTRUE, Format, Addresses, Strides,
        SurfaceWidth, SurfaceHeight, Rotation, Tiling));

    /* Fill in the structure. */
    surface                = &Engine->state.multiSrc[Engine->state.currentSrcIndex].srcSurface;
    surface->type          = gcvSURF_BITMAP;
    surface->format        = Format;
    surface->tiling        = Tiling;
    surface->alignedW      = SurfaceWidth;
    surface->alignedH      = SurfaceHeight;
    surface->rotation      = Rotation;

    switch (AddressNum)
    {
        case 3:
            surface->node.physical3 = Addresses[2];
            /*fall through*/

        case 2:
            surface->node.physical2 = Addresses[1];
            /*fall through*/

        case 1:
            gcsSURF_NODE_SetHardwareAddress(&surface->node, Addresses[0]);
            break;

        default:
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    switch (StrideNum)
    {
        case 3:
            surface->vStride = Strides[2];
            /*fall through*/

        case 2:
            surface->uStride = Strides[1];
            /*fall through*/

        case 1:
            surface->stride = Strides[0];
            break;

        default:
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    Engine->state.multiSrc[Engine->state.currentSrcIndex].srcType = gcv2D_SOURCE_COLOR;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:

    /* Return status. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

/*******************************************************************************
**
**  gco2D_SetGenericTarget
**
**  Configure color source for linear, tile, super-tile, multi-tile. Also for
**      YUV format source.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT32 Address
**          Source surface base addresses.
**
**      gctUINT32 AddressNum
**          Number of source surface base addresses.
**
**      gctUINT32 Strides
**          Strides of the source surface in bytes.
**
**      gctUINT32 StrideNum
**          Number of stride of the source surface.
**
**      gceSURF_FORMAT Format
**          Color format of the source surface.
**
**      gceSURF_ROTATION Rotation
**          Type of rotation.
**
**      gctUINT32 SurfaceWidth
**          Required only if the surface is rotated. Equal to the width
**          of the source surface in pixels.
**
**      gctUINT32 SurfaceHeight
**          Required only if the surface is rotated in PE2.0. Equal to the height
**          of the source surface in pixels.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetGenericTarget(
    IN gco2D               Engine,
    IN gctUINT32_PTR       Addresses,
    IN gctUINT32           AddressNum,
    IN gctUINT32_PTR       Strides,
    IN gctUINT32           StrideNum,
    IN gceTILING           Tiling,
    IN gceSURF_FORMAT      Format,
    IN gceSURF_ROTATION    Rotation,
    IN gctUINT32           SurfaceWidth,
    IN gctUINT32           SurfaceHeight
)
{
#if gcdENABLE_2D
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 n;
    gcoSURF surface;

    gcmHEADER_ARG("Engine=0x%x Addresses=0x%08x AddressNum=%d Strides=%d StrideNum=%d "
                    "Tiling=%d Format=%d Rotation=%d "
                    "SurfaceWidth=%d SurfaceHeight=%d ",
                    Engine, Addresses, AddressNum, Strides, StrideNum,
                    Tiling, Format, Rotation,
                    SurfaceWidth, SurfaceHeight);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    gcmONERROR(_CheckFormat(Format, &n, gcvNULL, gcvNULL));

    if ((n > AddressNum) || (AddressNum > 3) || (StrideNum > 3))
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmONERROR(_CheckSurface(Engine, gcvFALSE, Format, Addresses, Strides,
        SurfaceWidth, SurfaceHeight, Rotation, Tiling));

    surface = &Engine->state.dstSurface;

    switch (AddressNum)
    {
        case 3:
            surface->node.physical3 = Addresses[2];
            /*fall through*/

        case 2:
            surface->node.physical2 = Addresses[1];
            /*fall through*/

        case 1:
            gcsSURF_NODE_SetHardwareAddress(&surface->node, Addresses[0]);
            break;

        default:
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    switch (StrideNum)
    {
        case 3:
            surface->vStride = Strides[2];
            /*fall through*/

        case 2:
            surface->uStride = Strides[1];
            /*fall through*/

        case 1:
            surface->stride = Strides[0];
            break;

        default:
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    surface->type          = gcvSURF_BITMAP;
    surface->format        = Format;
    surface->tiling        = Tiling;
    surface->alignedW      = SurfaceWidth;
    surface->alignedH      = SurfaceHeight;
    surface->rotation      = Rotation;

OnError:

    /* Return status. */
    gcmFOOTER();
    return status;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_SetCurrentSource
**
**  Support multi-source.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT32 Address
**          Source surface base addresses.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_SetCurrentSourceIndex(
    IN gco2D        Engine,
    IN gctUINT32    SrcIndex
    )
{
#if gcdENABLE_2D
    gcmHEADER_ARG("Engine=0x%x SrcIndex=%d",
                    Engine, SrcIndex);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);
    gcmVERIFY_ARGUMENT(SrcIndex < gcdMULTI_SOURCE_NUM);

    Engine->state.currentSrcIndex = SrcIndex;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_MultiSourceBlit
**
**  Multi blit with mulit sources.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to an gco2D object.
**
**      gctUINT32 SourceMask
**          Indicate which source of the total 4 or 8 would be used to do
**          MultiSrcBlit(composition). Bit N represents the source index N.
**
**      gcsRECT_PTR Rect
**          Points to an array of positions in (x0, y0)-(x1, y1) format.
**
**      gctUINT32 RectCount
**          The number of rectangles to draw. The array of rectangle positions
**          pointed to by Rect parameter must have at least RectCount
**          positions. If RectCount equals to 0, use multi dest rectangle set
**          through gco2D_SetTargetRect for every source rectangle.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_MultiSourceBlit(
    IN gco2D Engine,
    IN gctUINT32 SourceMask,
    IN gcsRECT_PTR DstRect,
    IN gctUINT32 RectCount
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    gctUINT i, maxSrc = 0;
    gctBOOL mpSrc = gcvFALSE;
    gctBOOL supportMinorTile = gcvFALSE;
    gctINT dRectWidth = 0, dRecHeight = 0;
    gcoSURF surf;
    gceSURF_ROTATION rot;
    gctINT w = 32768;
    gctINT h = 32768;

    gcmHEADER_ARG("Engine=0x%x SourceMask=0x%08x DstRect=0x%x RectCount=%d",
                    Engine, SourceMask, DstRect, RectCount);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    if ((Engine->state.dstSurface.tileStatusConfig == gcv2D_TSC_2D_COMPRESSED)
        && !gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_ANDROID_ONLY)
        && (((Engine->state.dstSurface.format != gcvSURF_A8R8G8B8)
        && (Engine->state.dstSurface.format != gcvSURF_X8R8G8B8))
        || !Engine->state.unifiedDstRect))
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (RectCount > 0)
    {
        gctUINT32 n;

        gcmVERIFY_ARGUMENT(DstRect != gcvNULL);

        if (gcoHAL_IsFeatureAvailable(gcvNULL,
            gcvFEATURE_2D_MULTI_SRC_BLT_BILINEAR_FILTER) == gcvTRUE)
        {
            maxSrc = 8;
        }
        else if (gcoHAL_IsFeatureAvailable(gcvNULL,
            gcvFEATURE_2D_MULTI_SOURCE_BLT_EX) == gcvTRUE)
        {
            maxSrc = 8;
            supportMinorTile = gcvTRUE;
        }
        else if (gcoHAL_IsFeatureAvailable(gcvNULL,
            gcvFEATURE_2D_MULTI_SOURCE_BLT) == gcvTRUE)
        {
            maxSrc = 4;
        }
        else if (gcoHAL_IsFeatureAvailable(gcvNULL,
            gcvFEATURE_2D_MULTI_SRC_BLT_TO_UNIFIED_DST_RECT) == gcvTRUE)
        {
            maxSrc = 8;
        }
        else if (gcoHAL_IsFeatureAvailable(gcvNULL,
            gcvFEATURE_2D_MULTI_SRC_BLT_1_5_ENHANCEMENT) == gcvTRUE)
        {
            maxSrc = 8;
        }
        else
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        surf = &Engine->state.dstSurface;
        rot = gcmGET_PRE_ROTATION(surf->rotation);
        if ((rot == gcvSURF_90_DEGREE)
            || (rot == gcvSURF_270_DEGREE))
        {
            w = surf->alignedH;
            h = surf->alignedW;
        }
        else
        {
            w = surf->alignedW;
            h = surf->alignedH;
        }

        for (n = 0; n < RectCount; n++)
        {
            if ((DstRect[n].right < DstRect[n].left)
                || (DstRect[n].bottom < DstRect[n].top)
                || (DstRect[n].right > w)
                || (DstRect[n].bottom > h))
            {
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }
        }

        if (Engine->state.unifiedDstRect)
        {
            dRectWidth = DstRect->right - DstRect->left;
            dRecHeight = DstRect->bottom - DstRect->top;

            for (n = 1; n < RectCount; n++)
            {
                if ((DstRect[n].right - DstRect[n].left != dRectWidth)
                    || (DstRect[n].bottom - DstRect[n].top != dRecHeight))
                {
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }
            }
        }
    }
    else
    {
        if (gcoHAL_IsFeatureAvailable(gcvNULL,
            gcvFEATURE_2D_MULTI_SOURCE_BLT_EX2) == gcvTRUE)
        {
            maxSrc = 8;
            supportMinorTile = gcvTRUE;
            DstRect = gcvNULL;
        }
        else
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }
    }

    if ((maxSrc > gcdMULTI_SOURCE_NUM)
        || (SourceMask & (~0U << maxSrc))
        || !(SourceMask & (~(~0U << maxSrc))))
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    for (i = 0; i < maxSrc; i++)
    {
        gcsRECT_PTR srcRect;

        if (!(SourceMask & (1 << i)))
        {
            continue;
        }

        surf = &Engine->state.multiSrc[i].srcSurface;
        srcRect = &Engine->state.multiSrc[i].srcRect;
        rot = gcmGET_PRE_ROTATION(surf->rotation);

        if ((rot == gcvSURF_90_DEGREE)
            || (rot == gcvSURF_270_DEGREE))
        {
            w = surf->alignedH;
            h = surf->alignedW;
        }
        else
        {
            w = surf->alignedW;
            h = surf->alignedH;
        }

        /* HW does not support. */
        switch (surf->tiling)
        {
            case gcvLINEAR:
            case gcvTILED:
            case gcvSUPERTILED:
            case gcvMULTI_TILED:
            case gcvMULTI_SUPERTILED:
                break;

            case gcvMINORTILED:
                if (supportMinorTile &&
                    !gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_ANDROID_ONLY))
                {
                    break;
                }

            case gcvYMAJOR_SUPERTILED:
                if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_MAJOR_SUPER_TILE) != gcvTRUE)
                {
                    return gcvSTATUS_NOT_SUPPORTED;
                }
                break;

            case gcvTILED_4X8:
            case gcvTILED_8X4:
            case gcvTILED_8X8_XMAJOR:
            case gcvTILED_8X8_YMAJOR:
            case gcvTILED_32X4:
            case gcvTILED_64X4:
            case gcvSUPERTILED_128B:
            case gcvSUPERTILED_256B:
                if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_DEC400_COMPRESSION) != gcvTRUE)
                {
                    return gcvSTATUS_NOT_SUPPORTED;
                }
                break;

            /*Fall through*/

            default:
                gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
                break;
        }

        switch (surf->format)
        {
        case gcvSURF_I420:
        case gcvSURF_YV12:
        case gcvSURF_NV16:
        case gcvSURF_NV12:
        case gcvSURF_NV61:
        case gcvSURF_NV21:
            if (mpSrc)
            {
                if (gcoHAL_IsFeatureAvailable(gcvNULL,
                    gcvFEATURE_2D_MULTI_SOURCE_BLT_EX2) != gcvTRUE &&
                    gcoHAL_IsFeatureAvailable(gcvNULL,
                    gcvFEATURE_2D_MULTI_SRC_BLT_1_5_ENHANCEMENT) != gcvTRUE &&
                    gcoHAL_IsFeatureAvailable(gcvNULL,
                    gcvFEATURE_2D_MULTI_SRC_BLT_BILINEAR_FILTER) != gcvTRUE)
                {
                    /* HW cannot support more than one before V2. */
                    gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
                }
            }
            else
            {
                mpSrc = gcvTRUE;
            }

            break;

        default:
            break;
        }

        if ((surf->tileStatusConfig == gcv2D_TSC_2D_COMPRESSED)
            && !gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_ANDROID_ONLY)
            && (((surf->format != gcvSURF_A8R8G8B8)
            && (surf->format != gcvSURF_X8R8G8B8))
            || !Engine->state.unifiedDstRect))
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        if ((srcRect->right < srcRect->left)
            || (srcRect->bottom < srcRect->top)
            || (srcRect->right - srcRect->left > w)
            || (srcRect->bottom - srcRect->top > h))
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        if (Engine->state.unifiedDstRect)
        {
            if ((srcRect->right - srcRect->left!= dRectWidth)
                || (srcRect->bottom - srcRect->top != dRecHeight))
            {
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }
        }
    }

    Engine->state.srcMask = SourceMask;

    /* Start multi source blit. */
    gcmONERROR(gcoHARDWARE_StartDE(
        Engine->hardware,
        &Engine->state,
        gcv2D_MULTI_SOURCE_BLT,
        0,
        gcvNULL,
        RectCount,
        DstRect
        ));

OnError:

    /* Return status. */
    gcmFOOTER();
#endif /* gcdENABLE_2D */

    return status;
}

/*******************************************************************************
**
**  gco2D_SetGdiStretchMode
**
**  Enable/disable 2D GDI stretch mode for integral multiple stretch.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to the gco2D object.
**
**      gctBOOL Enable
**          Enable/disable integral multiple stretch.
**
**  OUTPUT:
**
**      gceSTATUS
**          Returns gcvSTATUS_OK if successful.
*/
gceSTATUS
gco2D_SetGdiStretchMode(
    IN gco2D Engine,
    IN gctBOOL Enable
    )
{
#if gcdENABLE_2D
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Engine=0x%x Enable=%d",
                    Engine, Enable);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);
    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_MULTI_SOURCE_BLT) != gcvTRUE &&
        gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_MULTI_SRC_BLT_1_5_ENHANCEMENT) != gcvTRUE)
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    Engine->state.multiSrc[Engine->state.currentSrcIndex].enableGDIStretch = Enable;

OnError:

    /* Succeed. */
    gcmFOOTER();
    return status;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_SetSourceTileStatus
**
**  Config tile status for source surface.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to the gco2D object.
**
**      gce2D_TILE_STATUS_CONFIG TSControl
**          Config tile status.
**
**      gceSURF_FORMAT CompressedFormat
**          Compressed format.
**
**      gctUINT32 ClearValue
**          Value for tiles that are marked as clear.
**
**      gctUINT32 GpuAddress
**          GpuAddress for tile status buffer.
**
**  OUTPUT:
**
**      gceSTATUS
**          Returns gcvSTATUS_OK if successful.
*/
gceSTATUS
gco2D_SetSourceTileStatus(
    IN gco2D Engine,
    IN gce2D_TILE_STATUS_CONFIG TileStatusConfig,
    IN gceSURF_FORMAT CompressedFormat,
    IN gctUINT32 ClearValue,
    IN gctUINT32 GpuAddress
    )
{
#if gcdENABLE_2D
    gceSTATUS status = gcvSTATUS_OK;
    gcoSURF surface;

    gcmHEADER_ARG("Engine=0x%x TSControl=%x CompressedFormat=%d ClearValue=%d GpuAddress=%d",
                    Engine, TileStatusConfig, CompressedFormat, ClearValue, GpuAddress);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    if (TileStatusConfig == gcv2D_TSC_2D_COMPRESSED)
    {
        if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_COMPRESSION) != gcvTRUE)
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        if ((CompressedFormat != gcvSURF_X8R8G8B8) && (CompressedFormat != gcvSURF_A8R8G8B8))
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }
    else if (TileStatusConfig & gcv2D_TSC_TPC_COMPRESSED)
    {
        if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TPC_COMPRESSION) != gcvTRUE &&
            gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TPCV11_COMPRESSION) != gcvTRUE)
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }
    }
    else if (TileStatusConfig & gcv2D_TSC_DEC_COMPRESSED)
    {
        if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_DEC_COMPRESSION) != gcvTRUE)
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        if (TileStatusConfig & gcv2D_TSC_DEC_TPC)
        {
            if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_DEC_TPC_COMPRESSION) != gcvTRUE)
            {
                gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            }
        }
    }
    else if (TileStatusConfig == gcv2D_TSC_DISABLE)
    {
        ClearValue = 0;
        GpuAddress = ~0U;
        CompressedFormat = gcvSURF_UNKNOWN;
    }
    else if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_FC_SOURCE) != gcvTRUE &&
             gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_V4COMPRESSION) != gcvTRUE)
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    surface = &Engine->state.multiSrc[Engine->state.currentSrcIndex].srcSurface;
    surface->tileStatusConfig    = TileStatusConfig;
    surface->tileStatusFormat     = CompressedFormat;
    surface->tileStatusClearValue = ClearValue;
    surface->tileStatusGpuAddress = GpuAddress;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:

    /* Return status. */
    gcmFOOTER();
    return status;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_SetTargetTileStatus
**
**  Config tile status for target surface.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to the gco2D object.
**
**      gce2D_TILE_STATUS_CONFIG TSControl
**          Config tile status.
**
**      gceSURF_FORMAT CompressedFormat
**          Compressed format.
**
**      gctUINT32 ClearValue
**          Value for tiles that are marked as clear.
**
**      gctUINT32 GpuAddress
**          GpuAddress for tile status buffer.
**
**  OUTPUT:
**
**      gceSTATUS
**          Returns gcvSTATUS_OK if successful.
*/
gceSTATUS
gco2D_SetTargetTileStatus(
    IN gco2D Engine,
    IN gce2D_TILE_STATUS_CONFIG TileStatusConfig,
    IN gceSURF_FORMAT CompressedFormat,
    IN gctUINT32 ClearValue,
    IN gctUINT32 GpuAddress
    )
{
#if gcdENABLE_2D
    gceSTATUS status = gcvSTATUS_OK;
    gcoSURF surface;

    gcmHEADER_ARG("Engine=0x%x TSControl=%x CompressedFormat=%d ClearValue=%d GpuAddress=%d",
                    Engine, TileStatusConfig, CompressedFormat, ClearValue, GpuAddress);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    if (TileStatusConfig == gcv2D_TSC_2D_COMPRESSED)
    {
        if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_COMPRESSION) != gcvTRUE)
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        if ((CompressedFormat != gcvSURF_X8R8G8B8) && (CompressedFormat != gcvSURF_A8R8G8B8))
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }
    else if (TileStatusConfig & gcv2D_TSC_TPC_COMPRESSED)
    {
        if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TPC_COMPRESSION) != gcvTRUE &&
            gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TPCV11_COMPRESSION) != gcvTRUE)
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }
    }
    else if (TileStatusConfig & gcv2D_TSC_DEC_COMPRESSED)
    {
        if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_DEC_COMPRESSION) != gcvTRUE)
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        if (TileStatusConfig & gcv2D_TSC_DEC_TPC)
        {
            if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_DEC_TPC_COMPRESSION) != gcvTRUE)
            {
                gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            }
        }
    }
    else if (TileStatusConfig == gcv2D_TSC_DISABLE)
    {
        ClearValue = 0;
        GpuAddress = ~0U;
        CompressedFormat = gcvSURF_UNKNOWN;
    }
    else if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_FC_SOURCE) != gcvTRUE &&
             gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_V4COMPRESSION) != gcvTRUE)
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    surface = &Engine->state.dstSurface;
    surface->tileStatusConfig    = TileStatusConfig;
    surface->tileStatusFormat     = CompressedFormat;
    surface->tileStatusClearValue = ClearValue;
    surface->tileStatusGpuAddress = GpuAddress;

    /* Succeed. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:

    /* Return status. */
    gcmFOOTER();
    return status;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_QueryU32
**
**  Query 2D engine for unsigned 32 bit information.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to the gco2D object.
**
**      gce2D_QUERY Item
**          Item to query.
**
**  OUTPUT:
**
**      gceSTATUS
**          Returns gcvSTATUS_OK if successful.
**
**      gctUINT32_PTR Value
**          Value for the queried item.
**
*/
gceSTATUS
gco2D_QueryU32(
    IN gco2D Engine,
    IN gce2D_QUERY Item,
    OUT gctUINT32_PTR Value
    )
{
#if gcdENABLE_2D
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Engine=0x%x Item=%d", Engine, Item);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    if (Value == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    switch (Item)
    {
    case gcv2D_QUERY_RGB_ADDRESS_MIN_ALIGN:
    case gcv2D_QUERY_RGB_STRIDE_MIN_ALIGN:
        if (Engine->alignImproved)
        {
            *Value = 4;
        }
        else
        {
            *Value = 16;
        }
        break;

    case gcv2D_QUERY_YUV_ADDRESS_MIN_ALIGN:
        *Value = 64;
        break;

    case gcv2D_QUERY_YUV_STRIDE_MIN_ALIGN:
        *Value = 8;
        break;

    default:
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

OnError:

    /* Return status. */
    gcmFOOTER();
    return status;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_SetStateU32
**
**  Set 2D engine state with unsigned 32 bit information.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to the gco2D object.
**
**      gce2D_STATE State
**          State to change.
**
**      gctUINT32 Value
**          Value for State.
**
**
**  SUPPORTED State:
**
**      gcv2D_STATE_SPECIAL_FILTER_MIRROR_MODE
**          Mirror sub rectangles.
**
**      gcv2D_STATE_SUPER_TILE_VERSION
**          Support different super tile versions of
**          gcv2D_SUPER_TILE_VERSION_V1, gcv2D_SUPER_TILE_VERSION_V2
**          and gcv2D_SUPER_TILE_VERSION_V1.
**
**      gcv2D_STATE_EN_GAMMA
**          Enalbe or disable the en-GAMMA pass for output.
**
**      gcv2D_STATE_DE_GAMMA
**          Enalbe or disable the de-GAMMA pass for input.
**
**      gcv2D_STATE_PROFILE_ENABLE
**          Support different profile flags of gcv2D_STATE_PROFILE_NONE,
**          gcv2D_STATE_PROFILE_COMMAND, gcv2D_STATE_PROFILE_SURFACE and
**          gcv2D_STATE_PROFILE_ALL.
**
**      gcv2D_STATE_XRGB_ENABLE
**          Enalbe or disable XRGB.
**
****
**  OUTPUT:
**
**      gceSTATUS
**          Returns gcvSTATUS_OK if successful.
**
*/
gceSTATUS
gco2D_SetStateU32(
    IN gco2D Engine,
    IN gce2D_STATE State,
    IN gctUINT32 Value
    )
{
#if gcdENABLE_2D
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Engine=0x%x State=%d", Engine, State);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    switch (State)
    {
    case gcv2D_STATE_SPECIAL_FILTER_MIRROR_MODE:
        Engine->state.specialFilterMirror = (Value == 0) ? gcvFALSE : gcvTRUE;
        break;

    case gcv2D_STATE_SUPER_TILE_VERSION:
        if ((Value != gcv2D_SUPER_TILE_VERSION_V1)
            && (Value != gcv2D_SUPER_TILE_VERSION_V2)
            && (Value != gcv2D_SUPER_TILE_VERSION_V3))
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        if (Value == gcv2D_SUPER_TILE_VERSION_V1 &&
            (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_ANDROID_ONLY) == gcvTRUE))
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        Engine->state.superTileVersion = Value;

        break;

    case gcv2D_STATE_MULTI_SRC_BLIT_UNIFIED_DST_RECT:
        if (Value && (gcoHAL_IsFeatureAvailable(gcvNULL,
            gcvFEATURE_2D_MULTI_SRC_BLT_TO_UNIFIED_DST_RECT) != gcvTRUE))
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        Engine->state.unifiedDstRect = (Value == 0) ? gcvFALSE : gcvTRUE;
        break;

    case gcv2D_STATE_MULTI_SRC_BLIT_BILINEAR_FILTER:
        if (Value && (gcoHAL_IsFeatureAvailable(gcvNULL,
            gcvFEATURE_2D_MULTI_SRC_BLT_BILINEAR_FILTER) != gcvTRUE))
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        Engine->state.multiBilinearFilter = (Value == 0) ? gcvFALSE : gcvTRUE;
        break;

    case gcv2D_STATE_EN_GAMMA:
        if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_GAMMA) != gcvTRUE)
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        Engine->state.dstEnGamma = (Value == 0) ? gcvFALSE : gcvTRUE;

        break;

    case gcv2D_STATE_DE_GAMMA:
        if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_GAMMA) != gcvTRUE)
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        Engine->state.multiSrc[Engine->state.currentSrcIndex].srcDeGamma = (Value == 0) ? gcvFALSE : gcvTRUE;

        break;

    case gcv2D_STATE_PROFILE_ENABLE:
#if gcdDUMP_2D
        dump2DFlag = (Value & gcv2D_STATE_PROFILE_ALL);
#else
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
#endif
        break;

    case gcv2D_STATE_DEC_TPC_NV12_10BIT:
        Engine->state.multiSrc[Engine->state.currentSrcIndex].srcSurface.srcDECTPC10BitNV12 = (Value == 0) ? gcvFALSE : gcvTRUE;
        break;

    case gcv2D_STATE_XRGB_ENABLE:
        Engine->state.enableXRGB = (Value == 0) ? gcvFALSE : gcvTRUE;
        break;

    default:
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

OnError:

    /* Return status. */
    gcmFOOTER();
    return status;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_SetStateArrayU32
**
**  Set 2D engine state with array of unsigned 32 integer.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to the gco2D object.
**
**      gce2D_STATE State
**          State to change.
**
**      gctUINT32_PTR Array
**          Pointer to input array.
**
**      gctINT32 ArraySize
**          Size of Array.
**
**  OUTPUT:
**
**      gceSTATUS
**          Returns gcvSTATUS_OK if successful.
**
**
**  SUPPORTED State:
**
**      gcv2D_STATE_ARRAY_EN_GAMMA
**          For every item of Array, bits[9:0] are for RED, bits[19:10] are for
**          GREEN, bits[29:20] are for BLUE. ArraySize must be
**          gcd2D_GAMMA_TABLE_SIZE.
**
**      gcv2D_STATE_ARRAY_DE_GAMMA
**          For every item of Array, bits[7:0] are for RED, bits[15:8] are for
**          GREEN, bits[23:16] are for BLUE. ArraySize must be
**          gcd2D_GAMMA_TABLE_SIZE.
**
*/
gceSTATUS
gco2D_SetStateArrayU32(
    IN gco2D Engine,
    IN gce2D_STATE State,
    IN gctUINT32_PTR Array,
    IN gctINT32 ArraySize
    )
{
#if gcdENABLE_2D
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Engine=0x%x State=%d", Engine, State);

    if ((Array == gcvNULL) || (ArraySize == 0))
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    switch (State)
    {
    case gcv2D_STATE_ARRAY_EN_GAMMA:
    case gcv2D_STATE_ARRAY_DE_GAMMA:
        {
            gctINT i;
            gctUINT32_PTR table = gcvNULL;

            if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_GAMMA) != gcvTRUE)
            {
                gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            }

            if (ArraySize != gcd2D_GAMMA_TABLE_SIZE)
            {
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }

            if (State == gcv2D_STATE_ARRAY_EN_GAMMA)
            {
                table = Engine->state.enGamma;
            }
            else
            {
                table = Engine->state.deGamma;
            }

            for (i = 0; i < gcd2D_GAMMA_TABLE_SIZE; ++i)
            {
                table[i] = Array[i];
            }
        }

        break;

    case gcv2D_STATE_ARRAY_YUV_SRC_TILE_STATUS_ADDR:
    case gcv2D_STATE_ARRAY_YUV_DST_TILE_STATUS_ADDR:
        {
            gcoSURF surface;
            gctINT i;

            if (ArraySize != 1 && ArraySize != 2)
            {
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }

            if (State == gcv2D_STATE_ARRAY_YUV_SRC_TILE_STATUS_ADDR)
                surface = &Engine->state.multiSrc[Engine->state.currentSrcIndex].srcSurface;
            else
                surface = &Engine->state.dstSurface;

            for (i = 0; i < ArraySize; i++)
                surface->tileStatusGpuAddressEx[i] = Array[i];
        }
        break;

    default:
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

OnError:

    /* Return status. */
    gcmFOOTER();
    return status;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_SetStateArrayI32
**
**  Set 2D engine state with array of signed 32 integer.
**
**  INPUT:
**
**      gco2D Engine
**          Pointer to the gco2D object.
**
**      gce2D_STATE State
**          State to change.
**
**      gctUINT32_PTR Array
**          Pointer to input array.
**
**      gctINT32 ArraySize
**          Size of Array.
**
**  OUTPUT:
**
**      gceSTATUS
**          Returns gcvSTATUS_OK if successful.
**
**
**  SUPPORTED State:
**
**      gce2D_STATE_ARRAY_CSC_YUV_TO_RGB
**      gce2D_STATE_ARRAY_CSC_RGB_TO_YUV
**
**      The formula for gce2D_STATE_ARRAY_CSC_RGB_TO_YUV
**        Y = ((C00*R + C01*G + C02*B) + D0 + 128)>>8
**        U = ((C10*R + C11*G + C12*B) + D1 + 128)>>8
**        V = ((C20*R + C21*G + C22*B) + D2 + 128)>>8
**
**      The formula for gce2D_STATE_ARRAY_CSC_YUV_TO_RGB
**        R = ((C00*Y + C01*U + C02*V) + D0 + 128)>>8
**        G = ((C10*Y + C11*U + C12*V) + D1 + 128)>>8
**        B = ((C20*Y + C21*U + C22*V) + D2 + 128)>>8
**
**        Array contains the coefficients in the sequence:
**        C00, C01, C02, C10, C11, C12, C20, C21, C22, D0, D1, D2.
**
**        Note: all the input conversion shares the same coefficients.
**              Cxx must be integer from 32767 to -32768.
**              Dx must be integer from 16777215 to -16777216.
**
*/
gceSTATUS
gco2D_SetStateArrayI32(
    IN gco2D Engine,
    IN gce2D_STATE State,
    IN gctINT32_PTR Array,
    IN gctINT32 ArraySize
    )
{
#if gcdENABLE_2D
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Engine=0x%x State=%d", Engine, State);

    if ((Array == gcvNULL) || (ArraySize == 0))
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    switch (State)
    {
    case gcv2D_STATE_ARRAY_CSC_YUV_TO_RGB:
    case gcv2D_STATE_ARRAY_CSC_RGB_TO_YUV:
        {
            gctINT i;
            gctINT32_PTR table = gcvNULL;

            if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_COLOR_SPACE_CONVERSION) != gcvTRUE ||
                gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NO_USER_CSC) == gcvTRUE)
            {
                gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            }

            if (ArraySize != gcd2D_CSC_TABLE_SIZE)
            {
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }

            for (i = 0; i < gcd2D_CSC_TABLE_SIZE; ++i)
            {
                if (i < 9)
                {
                    if (Array[i] > 32767 || Array[i] < -32768)
                    {
                        gcmONERROR(gcvSTATUS_INVALID_DATA);
                    }
                }
                else
                {
                    if (Array[i] > 16777215 || Array[i] < -16777216)
                    {
                        gcmONERROR(gcvSTATUS_INVALID_DATA);
                    }
                }
            }

            if (State == gcv2D_STATE_ARRAY_CSC_YUV_TO_RGB)
            {
                table = Engine->state.cscYUV2RGB;
            }
            else
            {
                table = Engine->state.cscRGB2YUV;
            }

            for (i = 0; i < gcd2D_CSC_TABLE_SIZE; ++i)
            {
                table[i] = Array[i];
            }
        }

        break;

    default:
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

OnError:

    /* Return status. */
    gcmFOOTER();
    return status;

#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_MonoBlitEx
**
**  Extend Monochrome blit for 8x1 packed stream.
**
**  NOTE:
**      1.  If SrcRect equals to gcvNULL, the stream is used as monochrome
**          source; Else, it is used as the source mask.
**      2.  The Stream rectangle size is the same with DstRect.
**
**  INPUT:
**
**      gctPOINTER   StreamBits
**          Pointer to the 8x1 packed stream.
**
**      gctINT32     StreamStride
**          The stride in byte of stream.
**
**      gctINT32     StreamWidth
**          The width in pixel of stream.
**
**      gctINT32     StreamHeight
**          The height in pixel of stream.
**
**      gctINT32     StreamX, StreamY
**          The starting offest in stream.
**
**      gctUINT32    FgColor, BgColor
**          Foreground/Background color values in A8R8G8B8 only useful when
**          SrcRect equals to gcvNULL .
**
**      gcsRECT_PTR  SrcRect
**          Pointer to src rectangle for masked source blit.
**
**      gcsRECT_PTR  DstRect
**          Pointer to dst rectangle.
**
**      gctUINT32 FgRop
**          Foreground ROP to use with opaque pixels.
**
**      gctUINT32 BgRop
**          Background ROP to use with transparent pixels.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gco2D_MonoBlitEx(
    IN gco2D        Engine,
    IN gctPOINTER   StreamBits,
    IN gctINT32     StreamStride,
    IN gctINT32     StreamWidth,
    IN gctINT32     StreamHeight,
    IN gctINT32     StreamX,
    IN gctINT32     StreamY,
    IN gctUINT32    FgColor,
    IN gctUINT32    BgColor,
    IN gcsRECT_PTR  SrcRect,
    IN gcsRECT_PTR  DstRect,
    IN gctUINT8     FgRop,
    IN gctUINT8     BgRop
    )
{
#if gcdENABLE_2D
    gceSTATUS status = gcvSTATUS_OK;
    gctINT32 dstRectWidth, dstRectHeight, maxLines, lineSize, lines, i;
    gctUINT32 Bpp = 0, srcOrgPhys = ~0u, pw, ph;
    gceSURF_ROTATION rot = -1;
    gce2D_TRANSPARENCY srcTrans = gcv2D_OPAQUE;
    gcsRECT_PTR rect;
    gcsRECT dRect, srcOrgRect = {0,};
    gcs2D_MULTI_SOURCE_PTR src = gcvNULL;
    gctBOOL ret = gcvFALSE;
    gctBOOL yuv;
    gctUINT32 planes;
    gctUINT8_PTR srcByte;
    gctUINT8_PTR destByte;
    gctUINT16 data = 0xff00;
    gctBOOL bigEndian = (*(gctUINT8*)&data == 0xff);

    gcmHEADER_ARG("Engine=0x%x StreamBits=0x%x StreamStride=0x%x "
        "StreamWidth=0x%x StreamHeight=0x%x StreamX=0x%x StreamY=0x%x "
        "FgColor=0x%x BgColor=0x%x SrcRect=0x%x DstRect=0x%x FgRop=0x%x "
        "BgRop=0x%x", Engine, StreamBits, StreamStride, StreamWidth,
        StreamHeight, StreamX, StreamY, FgColor, BgColor, SrcRect, DstRect,
        FgRop, BgRop);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_ANDROID_ONLY) == gcvTRUE)
    {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    if ((StreamBits == gcvNULL)
        || (StreamX < 0) || (StreamX >= StreamWidth)
        || (StreamY < 0) || (StreamY >= StreamHeight)
        || (StreamWidth  > (gcmABS(StreamStride) << 3))
        || (DstRect == gcvNULL))
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    src = &Engine->state.multiSrc[Engine->state.currentSrcIndex];

    dstRectWidth = DstRect->right - DstRect->left;
    dstRectHeight = DstRect->bottom - DstRect->top;
    rot = Engine->state.dstSurface.rotation;

    gcmONERROR(_CheckFormat(Engine->state.dstSurface.format, &planes, &Bpp, &yuv));

    if (((gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_BITBLIT_FULLROTATION) != gcvTRUE)
        && (rot != gcvSURF_0_DEGREE) && (rot != gcvSURF_90_DEGREE))
        || yuv || (planes != 1)
        || (StreamX + dstRectWidth > StreamWidth)
        || (StreamY + dstRectHeight > StreamHeight))
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    if (SrcRect != gcvNULL)
    {
        rot = src->srcSurface.rotation;

        gcmONERROR(_CheckFormat(src->srcSurface.format, &planes, &Bpp, &yuv));

        if (!Engine->alignImproved || yuv || (planes != 1) || gcmGET_POST_ROTATION(rot)
            || (SrcRect->right - SrcRect->left != dstRectWidth)
            || (SrcRect->bottom - SrcRect->top != dstRectHeight)
            || ((gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_BITBLIT_FULLROTATION) != gcvTRUE)
                && (rot != gcvSURF_0_DEGREE) && (rot != gcvSURF_90_DEGREE)))
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        Bpp >>= 3;
        srcTrans = src->srcTransparency;
        gcmGETHARDWAREADDRESS(src->srcSurface.node, srcOrgPhys);
    }
    else if (src->srcTransparency == gcv2D_MASKED)
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    rect = &src->srcRect;
    srcOrgRect = *rect;
    dRect = *DstRect;

    ret = gcvTRUE;
    rect->left   = StreamX & 7;
    rect->top    = 0;
    rect->right  = rect->left + dstRectWidth;
    rect->bottom = dstRectHeight;

    if (rect->right <= 8)
    {
        src->srcMonoPack = gcvSURF_PACKED8;
        pw = 1;
        ph = 4;
    }
    else if (rect->right <= 16)
    {
        src->srcMonoPack = gcvSURF_PACKED16;
        pw = 2;
        ph = 2;
    }
    else
    {
        src->srcMonoPack = gcvSURF_UNPACKED;
        pw = gcmALIGN(rect->right, 32) >> 3;
        ph = 1;
    }

    if (SrcRect != gcvNULL)
    {
        gctINT32 offset = SrcRect->left - rect->left;
        gctUINT32 srcSurfaceAddress;
        gctUINT32 addValue = 0;

        gcmGETHARDWAREADDRESS(src->srcSurface.node, srcSurfaceAddress);

        switch (src->srcSurface.rotation)
        {
        case gcvSURF_0_DEGREE:
            addValue = SrcRect->top * src->srcSurface.stride + offset * Bpp;
            break;

        case gcvSURF_90_DEGREE:
            addValue = offset * src->srcSurface.stride;
            src->srcSurface.alignedW -= SrcRect->top;
            break;

        case gcvSURF_180_DEGREE:
            src->srcSurface.alignedW -= offset;
            src->srcSurface.alignedH -= SrcRect->top;
            break;

        case gcvSURF_270_DEGREE:
            addValue = SrcRect->top * Bpp;
            src->srcSurface.alignedH -= offset;
            break;

        case gcvSURF_FLIP_X:
            addValue = SrcRect->top * src->srcSurface.stride;
            src->srcSurface.alignedW  -= offset;
            break;

        case gcvSURF_FLIP_Y:
            addValue = offset * Bpp;
            src->srcSurface.alignedH -= SrcRect->top;
            break;

        default:
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        gcsSURF_NODE_SetHardwareAddress(
            &src->srcSurface.node,
            srcSurfaceAddress + addValue
            );

        src->srcRelativeCoord = gcvFALSE;
        src->srcStream        = gcvFALSE;
        src->srcType          = gcv2D_SOURCE_MASKED;
        src->srcTransparency  = gcv2D_MASKED;
    }
    else
    {
        src->srcFgColor               = FgColor;
        src->srcBgColor               = BgColor;
        src->srcRelativeCoord         = gcvFALSE;
        src->srcColorConvert          = gcvTRUE;
        src->srcStream                = gcvTRUE;
        src->srcSurface.format        = gcvSURF_INDEX1;
        src->srcType                  = gcv2D_SOURCE_MONO;
    }

    StreamBits = (gctUINT8_PTR)StreamBits + StreamY * StreamStride + (StreamX >> 3);
    maxLines = ((gco2D_GetMaximumDataCount() << 2) / pw) & (~(ph - 1));
    lineSize = gcmALIGN(rect->right, 8) >> 3;
    lines = rect->bottom;
    rect->bottom = 0;

    src->fgRop = FgRop;
    src->bgRop = BgRop;

    do {
        gctUINT8_PTR buffer;
        gctINT n;

        if (SrcRect != gcvNULL)
        {
            gctUINT32 srcSurfaceAddress;
            gctUINT32 offset = 0;

            gcmGETHARDWAREADDRESS(src->srcSurface.node, srcSurfaceAddress);

            switch (src->srcSurface.rotation)
            {
            case gcvSURF_0_DEGREE:
                offset = src->srcSurface.stride * rect->bottom;
                break;

            case gcvSURF_90_DEGREE:
                src->srcSurface.alignedW -= rect->bottom;
                break;

            case gcvSURF_180_DEGREE:
                src->srcSurface.alignedH -= rect->bottom;
                break;

            case gcvSURF_270_DEGREE:
                offset = rect->bottom * Bpp;
                break;

            case gcvSURF_FLIP_X:
                offset = rect->bottom * src->srcSurface.stride;
                break;

            case gcvSURF_FLIP_Y:
                src->srcSurface.alignedH -= rect->bottom;
                break;

            default:
                gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            }

            gcsSURF_NODE_SetHardwareAddress(
                &src->srcSurface.node,
                srcSurfaceAddress + offset
                );
        }

        dRect.top += rect->bottom;

        rect->bottom = gcmMIN(maxLines, lines);

        dRect.bottom = dRect.top + rect->bottom;

        /* Call lower layer to form a StartDE command. */
        gcmONERROR(gcoHARDWARE_StartDEStream(
            Engine->hardware,
            &Engine->state,
            &dRect,
            pw * gcmALIGN(rect->bottom, ph),
            (gctPOINTER*)&buffer
            ));

        for (n = 0; n < rect->bottom; n++)
        {
            if(!bigEndian)
            {
                gcoOS_MemCopy(buffer, StreamBits, lineSize);
            }
            else
            {
                srcByte = StreamBits;
                destByte = buffer;
                for(i = 0; i < lineSize; i++)
                {
                    * (gctUINT8_PTR) gcmINT2PTR((gcmPTR2SIZE(destByte) & (~3)) + (3 - (gcmPTR2SIZE(destByte) % 4))) =
                        * (gctUINT8_PTR) gcmINT2PTR((gcmPTR2SIZE(srcByte) & (~3)) + (3 - (gcmPTR2SIZE(srcByte) % 4)));

                    destByte++;
                    srcByte++;
                }
            }

            buffer += pw;
            StreamBits = (gctUINT8_PTR)StreamBits + StreamStride;
        }

        lines -= rect->bottom;

    } while (lines > 0);

OnError:
    if (ret)
    {
        src->srcRect = srcOrgRect;

        if (SrcRect != gcvNULL)
        {
            gcsSURF_NODE_SetHardwareAddress(&src->srcSurface.node, srcOrgPhys);
            src->srcTransparency = srcTrans;
        }
    }

    /* Return status. */
    gcmFOOTER();
    return status;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*******************************************************************************
**
**  gco2D_Get2DEngine
**
**  Get the pointer to the gco2D object which is the current one of this thread
**
**  OUTPUT:
**
**      gco2D * Engine
**          Pointer to a variable receiving the gco2D object pointer.
*/
gceSTATUS
gco2D_Get2DEngine(
    OUT gco2D * Engine
    )
{
#if gcdENABLE_2D
    gceSTATUS status;
    gcsTLS_PTR tls;

    gcmHEADER();

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Engine != gcvNULL);

    do
    {
        gcmONERROR(gcoOS_GetTLS(&tls));

        /* Return pointer to the gco2D object. */
        *Engine = tls->engine2D;

        if (*Engine == gcvNULL)
        {
            status = gcvSTATUS_INVALID_OBJECT;
            gcmERR_BREAK(status);
        }
        /* Success. */
        gcmFOOTER_ARG("*Engine=0x%x", *Engine);

        return gcvSTATUS_OK;

    }while (gcvFALSE);

 OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

/*****************************************************************************
*********
**
**  gco2D_Set2DEngine
**
**  Set the pointer as the gco2D object to be current 2D engine of this thread
**
**
**   gco2D Engine
**       The gco2D object that needs to be set.
**
**  OUTPUT:
**
**   nothing.
*/
gceSTATUS
gco2D_Set2DEngine(
     IN gco2D Engine
     )
{
#if gcdENABLE_2D
     gceSTATUS status;
     gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;

     gcmHEADER();

     /* Verify the arguments. */
     gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

     gcmONERROR(gcoHAL_GetHardwareType(gcvNULL, &currentType));
     gcmONERROR(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_2D));

     if (Engine->hardware == gcvNULL)
     {
        gcmONERROR(gcoHARDWARE_Construct(gcPLS.hal, gcvTRUE, gcvFALSE, &Engine->hardware));
     }

OnError:
     if (currentType != gcvHARDWARE_INVALID)
     {
        gcmONERROR(gcoHAL_SetHardwareType(gcvNULL, currentType));
     }

     /* Return the status. */
     gcmFOOTER();
     return status;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}


/*****************************************************************************
*********
**
**  gco2D_UnSet2DEngine
**
**  UnSet the pointer as the gco2D object from this thread, restore the old
**  hardware object.
**
**  INPUT:
**
**
**   gco2D Engine
**       The gco2D object that needs to be unset.
**
**  OUTPUT:
**
**   nothing.
*/
gceSTATUS
gco2D_UnSet2DEngine(
     IN gco2D Engine
     )
{
#if gcdENABLE_2D
     gcmHEADER();

     /* Verify the arguments. */
     gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

    if (Engine->hardware != gcvNULL)
    {
        if(gcmIS_ERROR(gcoHARDWARE_Destroy(Engine->hardware, gcvTRUE)))
        {
            gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_2D,
                "2D Engine: Failed to free hardware in 2D engine.");
        }

        Engine->hardware = gcvNULL;
    }

     /* Success. */
     gcmFOOTER_NO();
     return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}

gceSTATUS
gco2D_Commit(
    IN gco2D Engine,
    IN gctBOOL Stall
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Engine=%x Stall=%d", Engine, Stall);

    /* Commit the command buffer to hardware. */
    gcmONERROR(gcoHARDWARE_Commit(Engine->hardware));

    if (Stall)
    {
        /* Stall the hardware. */
        gcmONERROR(gcoHARDWARE_Stall(Engine->hardware));
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gco2D_NatureRotateTranslation
**
**    Translate the nature rotation rule to the gpu 2D specific rotation rule.
**
**  INPUT:
**
**      gctBOOL IsSrcRot
**          Tranlate to src rotation or dst rotation.
**
**      gce2D_NATURE_ROTATION NatureRotation
**          Nature rotation rule.
**
**      gctINT32 SrcSurfaceWidth
**          The width of the src surface.
**
**      gctINT32 SrcSurfaceHeight
**          The height of the src surface.
**
**      gctINT32 DstSurfaceWidth
**          The width of the dst surface.
**
**      gctINT32 DstSurfaceHeight
**          The height of the dst surface.
**
**      gcsRECT_PTR SrcRect
**          Pointer to the src rectangle before rotation.
**
**      gcsRECT_PTR DstRect
**          Pointer to the dst rectangle before rotation.
**
**  OUTPUT:
**
**      gcsRECT_PTR SrcRect
**          Pointer to the dst rectangle after rotation.
**          Only change when IsSrcRot is enabled.
**
**      gcsRECT_PTR DstRect
**          Pointer to the dst rectangle after rotation.
**          Only change when IsSrcRot is disabled.
**
**      gceSURF_ROTATION* SrcRotation
**          Pointer to the src rotation after translation.
**          Only change when IsSrcRot is enabled.
**
**      gceSURF_ROTATION* DstRotation
**          Pointer to the dst rotation after translation.
**          Only change when IsSrcRot is disabled.
**
*/
gceSTATUS
gco2D_NatureRotateTranslation(
    IN gctBOOL IsSrcRot,
    IN gce2D_NATURE_ROTATION NatureRotation,
    IN gctINT32 SrcSurfaceWidth,
    IN gctINT32 SrcSurfaceHeight,
    IN gctINT32 DstSurfaceWidth,
    IN gctINT32 DstSurfaceHeight,
    IN OUT gcsRECT_PTR SrcRect,
    IN OUT gcsRECT_PTR DstRect,
    OUT gceSURF_ROTATION * SrcRotation,
    OUT gceSURF_ROTATION * DstRotation
    )
{
#if gcdENABLE_2D
    gceSTATUS status = gcvSTATUS_OK;
    gctINT x, y;
    gctUINT32 sw=0, sh=0, dw=0, dh=0;

    gcmHEADER_ARG("IsSrcRot=%d NatureRotation=%d SrcSurface=%dx%d DstSurface=%dx%d SrcRect=%p DstRect=%p",
            IsSrcRot, NatureRotation, SrcSurfaceWidth, SrcSurfaceHeight, DstSurfaceWidth, DstSurfaceHeight,
            SrcRect, DstRect);

    /* Verify the arguments. */
    if (SrcRect == gcvNULL || DstRect == gcvNULL ||
        SrcRotation == gcvNULL || DstRotation == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (IsSrcRot)
    {
        sw = SrcRect->right - SrcRect->left;
        sh = SrcRect->bottom - SrcRect->top;
    }
    else
    {
        dw = DstRect->right - DstRect->left;
        dh = DstRect->bottom - DstRect->top;
    }

    switch (NatureRotation)
    {
        case gcvNR_0_DEGREE:

            *SrcRotation = gcvSURF_0_DEGREE;
            *DstRotation = gcvSURF_0_DEGREE;
            break;

        case gcvNR_LEFT_90_DEGREE:

            if (IsSrcRot)
            {
                x = SrcRect->top;
                y = SrcSurfaceWidth - sw - SrcRect->left;

                SrcRect->left = x;
                SrcRect->top = y;
                SrcRect->right = x + sh;
                SrcRect->bottom = y + sw;

                *SrcRotation = gcvSURF_90_DEGREE;
                *DstRotation = gcvSURF_0_DEGREE;
            }
            else
            {
                x = DstSurfaceHeight - dh - DstRect->top;
                y = DstRect->left;

                DstRect->left = x;
                DstRect->top = y;
                DstRect->right = x + dh;
                DstRect->bottom = y + dw;

                *SrcRotation = gcvSURF_0_DEGREE;
                *DstRotation = gcvSURF_270_DEGREE;
            }
            break;

        case gcvNR_RIGHT_90_DEGREE:

            if (IsSrcRot)
            {
                x = SrcSurfaceHeight - sh - SrcRect->top;
                y = SrcRect->left;

                SrcRect->left = x;
                SrcRect->top = y;
                SrcRect->right = x + sh;
                SrcRect->bottom = y + sw;

                *SrcRotation = gcvSURF_270_DEGREE;
                *DstRotation = gcvSURF_0_DEGREE;
            }
            else
            {
                x = DstRect->top;
                y = DstSurfaceWidth - dw - DstRect->left;

                DstRect->left = x;
                DstRect->top = y;
                DstRect->right = x + dh;
                DstRect->bottom = y + dw;

                *SrcRotation = gcvSURF_0_DEGREE;
                *DstRotation = gcvSURF_90_DEGREE;
            }
            break;

        case gcvNR_180_DEGREE:

            if (IsSrcRot)
            {
                x = SrcSurfaceWidth - sw - SrcRect->left;
                y = SrcSurfaceHeight - sh - SrcRect->top;

                SrcRect->left = x;
                SrcRect->top = y;
                SrcRect->right = x + sw;
                SrcRect->bottom = y + sh;

                *SrcRotation = gcvSURF_180_DEGREE;
                *DstRotation = gcvSURF_0_DEGREE;
            }
            else
            {
                x = DstSurfaceWidth - dw - DstRect->left;
                y = DstSurfaceHeight - dh - DstRect->top;

                DstRect->left = x;
                DstRect->top = y;
                DstRect->right = x + dw;
                DstRect->bottom = y + dh;

                *SrcRotation = gcvSURF_0_DEGREE;
                *DstRotation = gcvSURF_180_DEGREE;
            }
            break;

        case gcvNR_FLIP_X:

            if (IsSrcRot)
            {
                x = SrcSurfaceWidth - sw - SrcRect->left;
                y = SrcRect->top;

                SrcRect->left = x;
                SrcRect->top = y;
                SrcRect->right = x + sw;
                SrcRect->bottom = y + sh;

                *SrcRotation = gcvSURF_FLIP_X;
                *DstRotation = gcvSURF_0_DEGREE;
            }
            else
            {
                x = DstSurfaceWidth - dw - DstRect->left;
                y = DstRect->top;

                DstRect->left = x;
                DstRect->top = y;
                DstRect->right = x + dw;
                DstRect->bottom = y + dh;

                *SrcRotation = gcvSURF_0_DEGREE;
                *DstRotation = gcvSURF_FLIP_X;
            }
            break;

        case gcvNR_FLIP_Y:

            if (IsSrcRot)
            {
                x = SrcRect->left;
                y = SrcSurfaceHeight - sh - SrcRect->top;

                SrcRect->left = x;
                SrcRect->top = y;
                SrcRect->right = x + sw;
                SrcRect->bottom = y + sh;

                *SrcRotation = gcvSURF_FLIP_Y;
                *DstRotation = gcvSURF_0_DEGREE;
            }
            else
            {
                x = DstRect->left;
                y = DstSurfaceHeight - dh - DstRect->top;

                DstRect->left = x;
                DstRect->top = y;
                DstRect->right = x + dw;
                DstRect->bottom = y + dh;

                *SrcRotation = gcvSURF_0_DEGREE;
                *DstRotation = gcvSURF_FLIP_Y;
            }
            break;

        default:
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            break;
    }

OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif  /* gcdENABLE_2D */
}
