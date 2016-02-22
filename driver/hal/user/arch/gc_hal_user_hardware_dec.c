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


#include "gc_hal_user_hardware_precomp.h"

/* Zone used for header/footer. */
#define _GC_OBJ_ZONE    gcvZONE_HARDWARE

#if gcdENABLE_DEC_COMPRESSION

typedef enum _gcePLANAR
{
   gcvDEC_PLANAR_ONE,
   gcvDEC_PLANAR_TWO,
   gcvDEC_PLANAR_THREE,
}
gcePLANAR;

static gceSTATUS
gcoDECHARDWARE_UploadData(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Address,
    IN gctUINT32 Data
    )
{
    gceSTATUS status;

    gcmONERROR(gcoHARDWARE_Load2DState32(
        Hardware,
        Address,
        Data
        ));

OnError:
    return status;
}

#if gcdDEC_ENABLE_AHB
static gctUINT32
gcoDECHARDWARE_TranslateAHBFormat(
    IN gceSURF_FORMAT Format
    )
{
    switch (Format)
    {
    case gcvSURF_A8R8G8B8:
        return 0;
    case gcvSURF_R8G8B8A8:
        return 1;
    case gcvSURF_A8B8G8R8:
        return 2;
    case gcvSURF_B8G8R8A8:
        return 3;
    case gcvSURF_X8R8G8B8:
        return 4;
    case gcvSURF_R8G8B8X8:
        return 5;
    case gcvSURF_X8B8G8R8:
        return 6;
    case gcvSURF_B8G8R8X8:
        return 7;

    case gcvSURF_A4R4G4B4:
        return 8;
    case gcvSURF_B4G4R4A4:
        return 9;
    case gcvSURF_R4G4B4A4:
        return 10;
    case gcvSURF_A4B4G4R4:
        return 11;
    case gcvSURF_A1R5G5B5:
        return 12;
    case gcvSURF_B5G5R5A1:
        return 13;
    case gcvSURF_R5G5B5A1:
        return 14;
    case gcvSURF_A1B5G5R5:
        return 15;
    case gcvSURF_R5G6B5:
        return 16;

   case gcvSURF_A8:
        return 17;
   case gcvSURF_R8:
        return 18;

   case gcvSURF_UYVY:
        return 19;
   case gcvSURF_YUY2:
        return 20;
   case gcvSURF_NV12:
        return 21;

    default:
        return 0;
    }
}

#else

static gceSTATUS
gcoDECHARDWARE_SetFormatConfig(
    IN gceSURF_FORMAT Format,
    IN gcePLANAR Plane,
    IN gctBOOL IsRead,
    IN gctBOOL Is3D,
    IN OUT gctUINT32* Config
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 config = 0;

    if (Config == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }
    else
    {
        config = *Config;
    }

    if (!Is3D)
    {
        if (IsRead)
        {
            switch (Format)
            {
                case gcvSURF_A8R8G8B8:
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                    break;

                case gcvSURF_R8G8B8A8:
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                    break;

                case gcvSURF_A8B8G8R8:
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                    break;

                case gcvSURF_B8G8R8A8:
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                    break;

                case gcvSURF_X8R8G8B8:
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                    break;

                case gcvSURF_R8G8B8X8:
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                    break;

                case gcvSURF_X8B8G8R8:
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                    break;

                case gcvSURF_B8G8R8X8:
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                    break;

                case gcvSURF_A8:
                case gcvSURF_R8:
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    break;

                case gcvSURF_RG16:
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x6 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    break;

                case gcvSURF_UYVY:
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    break;

                case gcvSURF_YUY2:
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    break;

                case gcvSURF_I420:
                case gcvSURF_YV12:
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    break;

                case gcvSURF_NV12:
                case gcvSURF_NV16:
                    if (Plane == gcvDEC_PLANAR_ONE)
                    {
                        config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    }
                    else
                    {
                        config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x6 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    }
                    break;

                default:
                    gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            }
        }
        else /* Write */
        {
            switch (Format)
            {
                case gcvSURF_A8R8G8B8:
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                    break;

                case gcvSURF_R8G8B8A8:
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                    break;

                case gcvSURF_A8B8G8R8:
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                    break;

                case gcvSURF_B8G8R8A8:
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                    break;

                case gcvSURF_X8R8G8B8:
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                    break;

                case gcvSURF_R8G8B8X8:
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                    break;

                case gcvSURF_X8B8G8R8:
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                    break;

                case gcvSURF_B8G8R8X8:
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                    break;

                case gcvSURF_A8:
                case gcvSURF_R8:
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    break;

                case gcvSURF_RG16:
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x6 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    break;

                case gcvSURF_UYVY:
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    break;

                case gcvSURF_YUY2:
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    break;

                case gcvSURF_I420:
                case gcvSURF_YV12:
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    break;

                case gcvSURF_NV12:
                case gcvSURF_NV16:
                    if (Plane == gcvDEC_PLANAR_ONE)
                    {
                        config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    }
                    else
                    {
                        config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) (0x6 & ((gctUINT32) ((((1 ? 6:3) - (0 ? 6:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                    }
                    break;

                default:
                    gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            }
        }
    }
    else
    {
        switch (Format)
        {
            case gcvSURF_A4R4G4B4:
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 15:12) - (0 ? 15:12) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ? 15:12)));
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                break;

            case gcvSURF_B4G4R4A4:
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 15:12) - (0 ? 15:12) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ? 15:12)));
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                break;

            case gcvSURF_R4G4B4A4:
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 15:12) - (0 ? 15:12) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ? 15:12)));
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                break;

            case gcvSURF_A4B4G4R4:
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 15:12) - (0 ? 15:12) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ? 15:12)));
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                break;

            case gcvSURF_A1R5G5B5:
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 15:12) - (0 ? 15:12) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ? 15:12)));
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                break;

            case gcvSURF_B5G5R5A1:
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 15:12) - (0 ? 15:12) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ? 15:12)));
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                break;

            case gcvSURF_R5G5B5A1:
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 15:12) - (0 ? 15:12) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ? 15:12)));
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                break;

            case gcvSURF_A1B5G5R5:
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 15:12) - (0 ? 15:12) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ? 15:12)));
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                break;

            case gcvSURF_R5G6B5:
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 15:12) - (0 ? 15:12) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ? 15:12)));
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                break;

            case gcvSURF_A8R8G8B8:
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 15:12) - (0 ? 15:12) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ? 15:12)));
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                break;

            case gcvSURF_B8G8R8A8:
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 15:12) - (0 ? 15:12) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ? 15:12)));
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                break;

            case gcvSURF_R8G8B8A8:
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 15:12) - (0 ? 15:12) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ? 15:12)));
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                break;

            case gcvSURF_A8B8G8R8:
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 15:12) - (0 ? 15:12) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ? 15:12)));
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                break;

            case gcvSURF_X8R8G8B8:
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ? 15:12) - (0 ? 15:12) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ? 15:12)));
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                break;

            case gcvSURF_B8G8R8X8:
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ? 15:12) - (0 ? 15:12) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ? 15:12)));
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                break;

            case gcvSURF_R8G8B8X8:
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ? 15:12) - (0 ? 15:12) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ? 15:12)));
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                break;

            case gcvSURF_X8B8G8R8:
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ? 15:12) - (0 ? 15:12) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ? 15:12)));
                config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                break;

            default:
                gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }
    }


OnError:
    *Config = config;
    return status;
}

static gctUINT32
gcoDECHARDWARE_YUVFormatPlanar(
    IN gceSURF_FORMAT Format
    )
{
    switch (Format)
    {
    case gcvSURF_YUY2:
    case gcvSURF_UYVY:
    case gcvSURF_YVYU:
    case gcvSURF_VYUY:
        return 1;
    case gcvSURF_NV16:
    case gcvSURF_NV12:
    case gcvSURF_NV61:
    case gcvSURF_NV21:
        return 2;
    case gcvSURF_I420:
    case gcvSURF_YV12:
        return 3;

    default:
        return 0;
    }
}
#endif

gceSTATUS
gcoDECHARDWARE_CheckSurface(
    IN gcoHARDWARE Hardware,
    IN gcoSURF Surface
    )
{
    gctINT res = gcvTRUE;
    gctUINT32 address;
    gcsSURF_FORMAT_INFO_PTR formatInfo;

    if (Surface->tileStatusConfig & gcv2D_TSC_DEC_COMPRESSED)
    {
        gcmGETHARDWAREADDRESS(Surface->node, address);
        gcmONERROR(gcoHARDWARE_QueryFormat(Surface->format, &formatInfo));

        if (!Hardware->features[gcvFEATURE_DEC_COMPRESSION] ||
            (Surface->tileStatusConfig & gcv2D_TSC_DEC_TPC
             && !Hardware->features[gcvFEATURE_DEC_TPC_COMPRESSION]))
        {
            res = gcvFALSE;
        }
        else if (address & (formatInfo->bitsPerPixel * 2 - 1))
        {
            /* Physical address must be 16 pixels aligned. */
            res = gcvFALSE;
        }
        else if (Surface->stride & 15)
        {
            /* Stride must be 16 pixels aligned. */
            res = gcvFALSE;
        }
        else if (Surface->alignedH & 7)
        {
            /* Height must be 8 pixels aligned. */
            res = gcvFALSE;
        }
    }

    if (res == gcvFALSE)
    {
        return gcvSTATUS_NOT_SUPPORTED;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
gcoDECHARDWARE_QueryStateCmdLen(
    IN gcoHARDWARE Hardware,
    IN gcs2D_State_PTR State,
    IN gce2D_COMMAND Command,
    OUT gctUINT32* Size
    )
{
    gctUINT32 size;

#if gcdDEC_ENABLE_AHB
    size = 2;
#else
    gcs2D_MULTI_SOURCE_PTR src;
    gcoSURF DstSurface = &State->dstSurface;
    gctUINT i, srcMask = 0;

    size = 4;

    /* Dst command buffer size */
    if (DstSurface->tileStatusConfig & gcv2D_TSC_DEC_COMPRESSED)
    {
        size += 6 * 2;

        if (DstSurface->node.physical2 && DstSurface->tileStatusGpuAddressEx[0])
            size += 6 * 2;

        if (DstSurface->node.physical3 && DstSurface->tileStatusGpuAddressEx[1])
            size += 6 * 2;
    }
    else
    {
        size += 2 * 2;
    }

    if (Command == gcv2D_MULTI_SOURCE_BLT)
    {
        srcMask = State->srcMask;
    }
    else
    {
        srcMask = 1 << State->currentSrcIndex;
    }

    /* Src command buffer size */
    for (i = 0; i < gcdMULTI_SOURCE_NUM; i++)
    {
        if (!(srcMask & (1 << i)))
        {
            continue;
        }

        src = State->multiSrc + i;

        if (src->srcSurface.tileStatusConfig & gcv2D_TSC_DEC_COMPRESSED)
        {
            size += 10;

            if (src->srcSurface.node.physical2 && src->srcSurface.tileStatusGpuAddressEx[0])
                size += 3 * 2;

            if (src->srcSurface.node.physical3 && src->srcSurface.tileStatusGpuAddressEx[1])
                size += 3 * 2;
        }
        else
        {
            size += 6;

            if (gcoDECHARDWARE_YUVFormatPlanar(src->srcSurface.format))
                size += 2;
        }
    }
#endif

    if (Size != gcvNULL)
        *Size = size;

    return gcvSTATUS_OK;
}

gceSTATUS
gcoDECHARDWARE_EnableDECCompression(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable
    )
{
    gceSTATUS status;
    gctUINT32 config;

#if gcdDEC_ENABLE_AHB
    config = ((((gctUINT32) (0x0000EE08)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 22:22) - (0 ? 22:22) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ? 22:22)));
#else
    config = ((((gctUINT32) (0x0000EE08)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 22:22) - (0 ? 22:22) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ? 22:22)));
#endif

    if (Enable)
    {
        config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)));
    }
    else
    {
        config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)));
    }


    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 24:24) - (0 ? 24:24) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24)));

    gcmONERROR(gcoDECHARDWARE_UploadData(
        Hardware,
        0x18180,
        config
        ));

OnError:
    return status;
}

gceSTATUS
gcoDECHARDWARE_SetSrcDECCompression(
    IN gcoHARDWARE Hardware,
    IN gcoSURF Surface,
    IN gce2D_TILE_STATUS_CONFIG TileStatusConfig,
    IN gctUINT32 ReadId,
    IN gctBOOL FastClear,
    IN gctUINT32 ClearValue
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL enable, data3D, msaa, tpcEnable, tpcComEnable;
    gceSURF_FORMAT format;
    gctUINT32 address;

    enable = TileStatusConfig & gcv2D_TSC_DEC_COMPRESSED;
    data3D = TileStatusConfig & (gcv2D_TSC_COMPRESSED | gcv2D_TSC_ENABLE | gcv2D_TSC_DOWN_SAMPLER);
    msaa   = TileStatusConfig & gcv2D_TSC_DOWN_SAMPLER;
    tpcEnable = (TileStatusConfig & gcv2D_TSC_DEC_TPC);
    tpcComEnable = (TileStatusConfig & gcv2D_TSC_DEC_TPC_COMPRESSED);

    if (ReadId > 15)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

#if !gcdDEC_ENABLE_AHB
    {
        gctUINT32 config = 0, configEx = 0;
        gctUINT32 regOffset[3] = {0}, regOffsetEx[3] = {0};
        gctUINT32 width = 0;

        gcmGETHARDWAREADDRESS(Surface->node, address);

        /* Non-DEC_Compression or Only DEC TPC DeTile */
        if (!enable || (tpcEnable && !tpcComEnable))
        {
            regOffset[0] = (ReadId + 8)  << 2;
            /* For YUV format */
            regOffset[1] = (ReadId + 9)  << 2;
            regOffset[2] = (ReadId + 10) << 2;

            /* Disable DEC 2D & 3D compression. */
            config = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));
            config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ? 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 9:8) - (0 ? 9:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8)));

            gcmONERROR(gcoDECHARDWARE_UploadData(
                Hardware,
                0x18000 + regOffset[0],
                config
                ));

            /* Disable DEC TPC compression. */
            if (ReadId < 4)
            {
                /* Currently only support NV12 format. 0 is Y, 1 is UV */
                /* Y of DEC TPC use 8 9 10 11 */
                regOffsetEx[0] = (ReadId + 8) << 2;
                /* UV of DEC TPC use 1 2 3 6 */
                if (ReadId == 3)
                    regOffsetEx[1] = 6 << 2;
                else
                    regOffsetEx[1] = (ReadId + 1) << 2;

                configEx = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

                gcmONERROR(gcoDECHARDWARE_UploadData(
                    Hardware,
                    0x18240 + regOffsetEx[0],
                    configEx
                    ));

                if (gcoDECHARDWARE_YUVFormatPlanar(Surface->format))
                {
                    gcmONERROR(gcoDECHARDWARE_UploadData(
                        Hardware,
                        0x18240 + regOffsetEx[1],
                        configEx
                        ));
                }
            }
        }
        else
        {
            if (!tpcEnable)
            {
                regOffset[0] = (ReadId + 8)  << 2;
                /* For YUV format */
                regOffset[1] = (ReadId + 9)  << 2;
                regOffset[2] = (ReadId + 10) << 2;

                /* 2D or 3D compression */
                if (!data3D)
                {
                    gcmONERROR(gcoHARDWARE_TranslateXRGBFormat(
                        Hardware,
                        Surface->format,
                        &format
                        ));

                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:1) - (0 ? 2:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:1) - (0 ? 2:1) + 1))))))) << (0 ?
 2:1))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 2:1) - (0 ? 2:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 2:1) - (0 ? 2:1) + 1))))))) << (0 ? 2:1)));

                    gcmONERROR(gcoDECHARDWARE_SetFormatConfig(format, gcvDEC_PLANAR_ONE, gcvTRUE, gcvFALSE, &config));
                }
                else
                {
                    format = Surface->format;

                    /* 3D DEC Compression */
                    config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ? 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 9:8) - (0 ? 9:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8)));

                    gcmONERROR(gcoDECHARDWARE_SetFormatConfig(format, gcvDEC_PLANAR_ONE, gcvTRUE, gcvTRUE, &config));

                    if (msaa)
                    {
                        config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:1) - (0 ? 2:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:1) - (0 ? 2:1) + 1))))))) << (0 ?
 2:1))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 2:1) - (0 ? 2:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 2:1) - (0 ? 2:1) + 1))))))) << (0 ? 2:1)));
                    }
                    else
                    {
                        config = ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:1) - (0 ? 2:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:1) - (0 ? 2:1) + 1))))))) << (0 ?
 2:1))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 2:1) - (0 ? 2:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 2:1) - (0 ? 2:1) + 1))))))) << (0 ? 2:1)));
                    }

                    gcmONERROR(gcoDECHARDWARE_UploadData(
                        Hardware,
                        0x18200 + regOffset[0],
                        ClearValue
                        ));
                }

                gcmONERROR(gcoDECHARDWARE_UploadData(
                    Hardware,
                    0x18000 + regOffset[0],
                    config
                    ));
            }
            else
            {
                /* TPC DEC compression */
                if (ReadId > 3 || Surface->format != gcvSURF_NV12)
                {
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }

                /* Currently only support NV12 format. 0 is Y, 1 is UV */
                /* Y of DEC TPC use 8 9 10 11 */
                regOffsetEx[0] = (ReadId + 8) << 2;
                /* UV of DEC TPC use 1 2 3 6 */
                if (ReadId == 3)
                    regOffsetEx[1] = 6 << 2;
                else
                    regOffsetEx[1] = (ReadId + 1) << 2;

                configEx = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

                if (!Surface->srcDECTPC10BitNV12)
                {
                    configEx = ((((gctUINT32) (configEx)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)));
                    width = Surface->stride / 8;
                }
                else
                {
                    configEx = ((((gctUINT32) (configEx)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)));
                    width = Surface->stride * 8 / 10 / 8;
                }

                gcmONERROR(gcoDECHARDWARE_UploadData(
                    Hardware,
                    0x18240 + regOffsetEx[0],
                    ((((gctUINT32) (configEx)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:3) - (0 ? 12:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:3) - (0 ? 12:3) + 1))))))) << (0 ?
 12:3))) | (((gctUINT32) ((gctUINT32) (width) & ((gctUINT32) ((((1 ? 12:3) - (0 ?
 12:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:3) - (0 ? 12:3) + 1))))))) << (0 ?
 12:3))) |
                    ((((gctUINT32) (configEx)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ? 2:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 2:2) - (0 ? 2:2) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2)))
                    ));
            }

            gcmONERROR(gcoDECHARDWARE_UploadData(
                Hardware,
                tpcEnable ? 0x18080 + regOffsetEx[0] :
                            0x18080 + regOffset[0],
                address
                ));

            gcmONERROR(gcoDECHARDWARE_UploadData(
                Hardware,
                tpcEnable ? 0x180C0 + regOffsetEx[0] :
                            0x180C0 + regOffset[0],
                Surface->tileStatusGpuAddress
                ));

            if (!data3D && Surface->node.physical2 && Surface->tileStatusGpuAddressEx[0])
            {
                if (!tpcEnable)
                {
                    gcmONERROR(gcoDECHARDWARE_SetFormatConfig(format, gcvDEC_PLANAR_TWO, gcvTRUE, gcvFALSE, &config));

                    gcmONERROR(gcoDECHARDWARE_UploadData(
                        Hardware,
                        0x18000 + regOffset[1],
                        config
                        ));
                }
                else
                {
                    if (!Surface->srcDECTPC10BitNV12)
                    {
                        width = Surface->uStride / 8;
                    }
                    else
                    {
                        width = Surface->uStride * 8 / 10 / 8;
                    }

                    gcmONERROR(gcoDECHARDWARE_UploadData(
                        Hardware,
                        0x18240 + regOffsetEx[1],
                        ((((gctUINT32) (configEx)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:3) - (0 ? 12:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:3) - (0 ? 12:3) + 1))))))) << (0 ?
 12:3))) | (((gctUINT32) ((gctUINT32) (width) & ((gctUINT32) ((((1 ? 12:3) - (0 ?
 12:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:3) - (0 ? 12:3) + 1))))))) << (0 ?
 12:3))) |
                        ((((gctUINT32) (configEx)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ? 2:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 2:2) - (0 ? 2:2) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2)))
                        ));
                }

                gcmONERROR(gcoDECHARDWARE_UploadData(
                    Hardware,
                    tpcEnable ? 0x18080 + regOffsetEx[1] :
                                0x18080 + regOffset[1],
                    Surface->node.physical2
                    ));

                gcmONERROR(gcoDECHARDWARE_UploadData(
                    Hardware,
                    tpcEnable ? 0x180C0 + regOffsetEx[1] :
                                0x180C0 + regOffset[1],
                    Surface->tileStatusGpuAddressEx[0]
                    ));

                if (!tpcEnable && Surface->node.physical3 && Surface->tileStatusGpuAddressEx[1])
                {
                    gcmONERROR(gcoDECHARDWARE_SetFormatConfig(format, gcvDEC_PLANAR_THREE, gcvTRUE, gcvFALSE, &config));

                    gcmONERROR(gcoDECHARDWARE_UploadData(
                        Hardware,
                        0x18000 + regOffset[2],
                        config
                        ));

                    gcmONERROR(gcoDECHARDWARE_UploadData(
                        Hardware,
                        0x18080 + regOffset[2],
                        Surface->node.physical3
                        ));

                    gcmONERROR(gcoDECHARDWARE_UploadData(
                        Hardware,
                        0x180C0 + regOffset[2],
                        Surface->tileStatusGpuAddressEx[0]
                        ));
                }
            }
        }
    }
#else
    {
        gcsHAL_INTERFACE iface;

        gcmGETHARDWAREADDRESS(Surface->node, address);

        gcmONERROR(gcoHARDWARE_TranslateXRGBFormat(
            Hardware,
            Surface->format,
            &format
            ));

        gcoOS_ZeroMemory(&iface, sizeof(iface));

        iface.command = gcvHAL_DEC300_READ;
        iface.u.DEC300Read.enable = enable;
        iface.u.DEC300Read.readId = ReadId;
        iface.u.DEC300Read.format = gcoDECHARDWARE_TranslateAHBFormat(format);
        iface.u.DEC300Read.strides[0] = Surface->stride;
        iface.u.DEC300Read.is3D = data3D;
        iface.u.DEC300Read.isMSAA = msaa;
        iface.u.DEC300Read.clearValue = ClearValue;
        iface.u.DEC300Read.isTPC = tpcEnable;
        iface.u.DEC300Read.isTPCCompressed = tpcComEnable;
        iface.u.DEC300Read.surfAddrs[0] = address;
        iface.u.DEC300Read.tileAddrs[0] = Surface->tileStatusGpuAddress;

        if (enable && tpcComEnable)
        {
            iface.u.DEC300Read.strides[1] = Surface->uStride;
            iface.u.DEC300Read.surfAddrs[1] = Surface->node.physical2;
            iface.u.DEC300Read.tileAddrs[1] = Surface->tileStatusGpuAddressEx[0];
        }

        gcmONERROR(gcoOS_DeviceControl(gcvNULL,
                                       IOCTL_GCHAL_INTERFACE,
                                       &iface, gcmSIZEOF(iface),
                                       &iface, gcmSIZEOF(iface)));
    }
#endif

    {
        gctUINT32 configEx2 = 0;

        configEx2 = tpcEnable ?
                    (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) &  ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))) :
                    (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) &  ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))));

        configEx2 = (tpcEnable && tpcComEnable) ?
                    configEx2 & (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4))) &  ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)))) :
                    configEx2 & (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4))) &  ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5))));

        /* LoadState this command even in AHB mode. */
        gcmONERROR(gcoHARDWARE_Load2DState32(
            Hardware,
            0x12F60 + (ReadId << 2),
            configEx2
            ));
    }

OnError:
    return status;
}

gceSTATUS
gcoDECHARDWARE_SetDstDECCompression(
    IN gcoHARDWARE Hardware,
    IN gcoSURF Surface,
    IN gce2D_TILE_STATUS_CONFIG TileStatusConfig,
    IN gctUINT32 ReadId,
    IN gctUINT32 WriteId
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gceSURF_FORMAT format;
    gctUINT32 address;

#if !gcdDEC_ENABLE_AHB
    gctBOOL enable;
    gctUINT32 configR = 0, configW = 0;
    gctUINT32 regOffsetR[3], regOffsetW[3];

    if (WriteId > 7)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }
    else
    {
        regOffsetR[0] = 0 << 2;
        regOffsetW[0] = 1 << 2;

        /* For YUV format */
        regOffsetR[1] = 1 << 2;
        regOffsetW[1] = 2 << 2;
        regOffsetR[2] = 2 << 2;
        regOffsetW[2] = 3 << 2;
    }

    gcmGETHARDWAREADDRESS(Surface->node, address);

    gcmONERROR(gcoHARDWARE_TranslateXRGBFormat(
        Hardware,
        Surface->format,
        &format
        ));

    enable = TileStatusConfig & gcv2D_TSC_DEC_COMPRESSED;

    /* Dst only need to disable 2D DEC compression. */
    /* Set DST read client. */
    if (!enable)
    {
        configR = ((((gctUINT32) (configR)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));
    }
    else
    {
        configR = ((((gctUINT32) (configR)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

        gcmONERROR(gcoDECHARDWARE_SetFormatConfig(format, gcvDEC_PLANAR_ONE, gcvTRUE, gcvFALSE, &configR));

        configR = ((((gctUINT32) (configR)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:1) - (0 ? 2:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:1) - (0 ? 2:1) + 1))))))) << (0 ?
 2:1))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 2:1) - (0 ? 2:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 2:1) - (0 ? 2:1) + 1))))))) << (0 ? 2:1)));

        gcmONERROR(gcoDECHARDWARE_UploadData(
            Hardware,
            0x18080 + regOffsetR[0],
            address
            ));

        gcmONERROR(gcoDECHARDWARE_UploadData(
            Hardware,
            0x180C0 + regOffsetR[0],
            Surface->tileStatusGpuAddress
            ));
    }

    gcmONERROR(gcoDECHARDWARE_UploadData(
        Hardware,
        0x18000 + regOffsetR[0],
        configR
        ));

    if (enable && Surface->node.physical2 && Surface->tileStatusGpuAddressEx[0])
    {
        gcmONERROR(gcoDECHARDWARE_SetFormatConfig(format, gcvDEC_PLANAR_TWO, gcvTRUE, gcvFALSE, &configR));

        gcmONERROR(gcoDECHARDWARE_UploadData(
            Hardware,
            0x18000 + regOffsetR[1],
            configR
            ));

        gcmONERROR(gcoDECHARDWARE_UploadData(
            Hardware,
            0x18080 + regOffsetR[1],
            Surface->node.physical2
            ));

        gcmONERROR(gcoDECHARDWARE_UploadData(
            Hardware,
            0x180C0 + regOffsetR[1],
            Surface->tileStatusGpuAddressEx[0]
            ));

        if (Surface->node.physical3 && Surface->tileStatusGpuAddressEx[1])
        {
            gcmONERROR(gcoDECHARDWARE_SetFormatConfig(format, gcvDEC_PLANAR_THREE, gcvTRUE, gcvFALSE, &configR));

            gcmONERROR(gcoDECHARDWARE_UploadData(
                Hardware,
                0x18000 + regOffsetR[2],
                configR
                ));

            gcmONERROR(gcoDECHARDWARE_UploadData(
                Hardware,
                0x18080 + regOffsetR[2],
                Surface->node.physical3
                ));

            gcmONERROR(gcoDECHARDWARE_UploadData(
                Hardware,
                0x180C0 + regOffsetR[2],
                Surface->tileStatusGpuAddressEx[1]
                ));
        }
    }


    /* Set DST write client. */
    if (!enable)
    {
        configW = ((((gctUINT32) (configW)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));
    }
    else
    {
        configW = ((((gctUINT32) (configW)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

        gcmONERROR(gcoDECHARDWARE_SetFormatConfig(format, gcvDEC_PLANAR_ONE, gcvFALSE, gcvFALSE, &configW));

        configW = ((((gctUINT32) (configW)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:1) - (0 ? 2:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:1) - (0 ? 2:1) + 1))))))) << (0 ?
 2:1))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 2:1) - (0 ? 2:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 2:1) - (0 ? 2:1) + 1))))))) << (0 ? 2:1)));

        gcmONERROR(gcoDECHARDWARE_UploadData(
            Hardware,
            0x18100 + regOffsetW[0],
            address
            ));

        gcmONERROR(gcoDECHARDWARE_UploadData(
            Hardware,
            0x18140 + regOffsetW[0],
            Surface->tileStatusGpuAddress
            ));
    }

    gcmONERROR(gcoDECHARDWARE_UploadData(
        Hardware,
        0x18040 + regOffsetW[0],
        configW
        ));

    if (enable && Surface->node.physical2 && Surface->tileStatusGpuAddressEx[0])
    {
        gcmONERROR(gcoDECHARDWARE_SetFormatConfig(format, gcvDEC_PLANAR_TWO, gcvTRUE, gcvFALSE, &configW));

        gcmONERROR(gcoDECHARDWARE_UploadData(
            Hardware,
            0x18040 + regOffsetW[1],
            configW
            ));

        gcmONERROR(gcoDECHARDWARE_UploadData(
            Hardware,
            0x18100 + regOffsetW[1],
            Surface->node.physical2
            ));

        gcmONERROR(gcoDECHARDWARE_UploadData(
            Hardware,
            0x18140 + regOffsetW[1],
            Surface->tileStatusGpuAddressEx[0]
            ));

        if (Surface->node.physical3 && Surface->tileStatusGpuAddressEx[1])
        {
            gcmONERROR(gcoDECHARDWARE_SetFormatConfig(format, gcvDEC_PLANAR_THREE, gcvTRUE, gcvFALSE, &configW));

            gcmONERROR(gcoDECHARDWARE_UploadData(
                Hardware,
                0x18040 + regOffsetW[2],
                configW
                ));

            gcmONERROR(gcoDECHARDWARE_UploadData(
                Hardware,
                0x18100 + regOffsetW[2],
                Surface->node.physical3
                ));

            gcmONERROR(gcoDECHARDWARE_UploadData(
                Hardware,
                0x18140 + regOffsetW[2],
                Surface->tileStatusGpuAddressEx[1]
                ));
        }
    }
#else
    gcsHAL_INTERFACE iface;

    if (WriteId > 7)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmGETHARDWAREADDRESS(Surface->node, address);

    gcmONERROR(gcoHARDWARE_TranslateXRGBFormat(
        Hardware,
        Surface->format,
        &format
        ));

    gcoOS_ZeroMemory(&iface, sizeof(iface));

    iface.command = gcvHAL_DEC300_WRITE;
    iface.u.DEC300Write.enable = TileStatusConfig & gcv2D_TSC_DEC_COMPRESSED;
    iface.u.DEC300Write.readId = 0;
    iface.u.DEC300Write.writeId = 1;
    iface.u.DEC300Write.format = gcoDECHARDWARE_TranslateAHBFormat(format);
    iface.u.DEC300Write.surfAddr = address;
    iface.u.DEC300Write.tileAddr = Surface->tileStatusGpuAddress;

    gcmONERROR(gcoOS_DeviceControl(gcvNULL,
                                   IOCTL_GCHAL_INTERFACE,
                                   &iface, gcmSIZEOF(iface),
                                   &iface, gcmSIZEOF(iface)));
#endif

OnError:
    return status;
}

gceSTATUS
gcoDECHARDWARE_FlushDECCompression(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Flush,
    IN gctBOOL Wait
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (!Hardware->hw2DCurrentRenderCompressed)
    {
        return gcvSTATUS_OK;
    }

#if gcdDEC_ENABLE_AHB
    {
        gcsHAL_INTERFACE iface;

        gcoOS_ZeroMemory(&iface, sizeof(iface));

        if (Flush)
        {
            iface.command = gcvHAL_DEC300_FLUSH;
            gcmONERROR(gcoOS_DeviceControl(gcvNULL,
                                           IOCTL_GCHAL_INTERFACE,
                                           &iface, gcmSIZEOF(iface),
                                           &iface, gcmSIZEOF(iface)));
        }

        while (Wait)
        {
            iface.command = gcvHAL_DEC300_FLUSH_WAIT;
            gcmONERROR(gcoOS_DeviceControl(gcvNULL,
                                           IOCTL_GCHAL_INTERFACE,
                                           &iface, gcmSIZEOF(iface),
                                           &iface, gcmSIZEOF(iface)));

            if (iface.u.DEC300FlushWait.done)
            {
                Hardware->hw2DCurrentRenderCompressed = gcvFALSE;
                break;
            }
            else
            {
                gcoOS_Delay(gcvNULL, 100);
            }
        }
    }
#else
    if (Flush)
    {
        gcmONERROR(gcoDECHARDWARE_UploadData(
            Hardware,
            0x18180,
            ((((gctUINT32) (0x0000EE08)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) |
            ((((gctUINT32) (0x0000EE08)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
        ));
    }

    /* LOAD STATE use hardware idle. */
#endif

OnError:
    return status;
}

#else

gceSTATUS
gcoDECHARDWARE_CheckSurface(
    IN gcoHARDWARE Hardware,
    IN gcoSURF Surface
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoDECHARDWARE_QueryStateCmdLen(
    IN gcoHARDWARE Hardware,
    IN gcs2D_State_PTR State,
    IN gce2D_COMMAND Command,
    OUT gctUINT32* Size
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoDECHARDWARE_EnableDECCompression(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoDECHARDWARE_SetSrcDECCompression(
    IN gcoHARDWARE Hardware,
    IN gcoSURF Surface,
    IN gce2D_TILE_STATUS_CONFIG TileStatusConfig,
    IN gctUINT32 ReadId,
    IN gctBOOL FastClear,
    IN gctUINT32 ClearValue
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoDECHARDWARE_SetDstDECCompression(
    IN gcoHARDWARE Hardware,
    IN gcoSURF Surface,
    IN gce2D_TILE_STATUS_CONFIG TileStatusConfig,
    IN gctUINT32 ReadId,
    IN gctUINT32 WriteId
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoDECHARDWARE_FlushDECCompression(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Flush,
    IN gctBOOL Wait
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

#endif

