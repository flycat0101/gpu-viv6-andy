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


#include "gc_hal_user_hardware_precomp.h"

#define _GC_OBJ_ZONE    gcvZONE_2D

#if gcdENABLE_2D
gceSTATUS
gcoTPHARDWARE_EnableTPCCompression_V10(
    IN gcoHARDWARE Hardware,
    IN gctUINT Enable
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Enable=%d", Hardware, Enable);

    if (!Enable)
    {
        gcmONERROR(gcoHARDWARE_Load2DState32(
            Hardware,
            0x0138C,
            (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ? 2:2) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 2:2) - (0 ? 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ? 3:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3))))
            ));
    }
    else
    {
        gcmONERROR(gcoHARDWARE_Load2DState32(
            Hardware,
            0x0138C,
            (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ? 2:2) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 2:2) - (0 ? 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ? 3:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3))))
            ));
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoTPHARDWARE_SetSrcTPCCompression_V10(
    IN gcoHARDWARE Hardware,
    IN gctUINT Enable,
    IN gctUINT Index,
    IN gctUINT SrcAddr,
    IN gctUINT SrcStatusAddr,
    IN gctUINT SrcFormat,
    IN gctUINT SrcWidth,
    IN gctUINT SrcHeight,
    IN gctUINT SrcStride,
    IN gceSURF_ROTATION SrcRotation
    )
{
    gceSTATUS status;
    gctUINT config;
    gctUINT regOffset = Index << 2;

    gcmHEADER_ARG("Hardware=0x%x Enable=%d Index=%d SrcAddr=0x%08x SrcStatusAddr=0x%08x"
                  "SrcFormat=%d SrcWidth=%d SrcHeight=%d SrcStride=%d SrcRotation=%d",
                   Hardware, Enable, Index, SrcAddr, SrcStatusAddr, SrcFormat,
                   SrcWidth, SrcHeight, SrcStride, SrcRotation);

    if (Enable == gcvFALSE)
    {
        config = (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))));
    }
    else
    {
        config = (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))));

        switch (SrcFormat)
        {
            case gcvSURF_A8R8G8B8:
            case gcvSURF_A8B8G8R8:
            case gcvSURF_R8G8B8A8:
                config &= (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:2) - (0 ? 6:2) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:2) - (0 ? 6:2) + 1))))))) << (0 ?
 6:2))) | (((gctUINT32) (0x06 & ((gctUINT32) ((((1 ? 6:2) - (0 ? 6:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:2) - (0 ? 6:2) + 1))))))) << (0 ? 6:2))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7))));
                break;

            case gcvSURF_A4R4G4B4:
            case gcvSURF_A4B4G4R4:
            case gcvSURF_R4G4B4A4:
                config &= (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:2) - (0 ? 6:2) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:2) - (0 ? 6:2) + 1))))))) << (0 ?
 6:2))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 6:2) - (0 ? 6:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:2) - (0 ? 6:2) + 1))))))) << (0 ? 6:2))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7))));
                break;

            case gcvSURF_X8R8G8B8:
            case gcvSURF_X8B8G8R8:
            case gcvSURF_R8G8B8X8:
            case gcvSURF_B8G8R8X8:
                config &= (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:2) - (0 ? 6:2) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:2) - (0 ? 6:2) + 1))))))) << (0 ?
 6:2))) | (((gctUINT32) (0x05 & ((gctUINT32) ((((1 ? 6:2) - (0 ? 6:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:2) - (0 ? 6:2) + 1))))))) << (0 ? 6:2))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7))));
                break;

            case gcvSURF_R5G6B5:
            case gcvSURF_B5G6R5:
                config &= (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:2) - (0 ? 6:2) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:2) - (0 ? 6:2) + 1))))))) << (0 ?
 6:2))) | (((gctUINT32) (0x04 & ((gctUINT32) ((((1 ? 6:2) - (0 ? 6:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:2) - (0 ? 6:2) + 1))))))) << (0 ? 6:2))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7))));
                break;

            case gcvSURF_A1R5G5B5:
            case gcvSURF_A1B5G5R5:
            case gcvSURF_R5G5B5A1:
                config &= (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:2) - (0 ? 6:2) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:2) - (0 ? 6:2) + 1))))))) << (0 ?
 6:2))) | (((gctUINT32) (0x03 & ((gctUINT32) ((((1 ? 6:2) - (0 ? 6:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:2) - (0 ? 6:2) + 1))))))) << (0 ? 6:2))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7))));
                break;

            case gcvSURF_RG16:
                config &= (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:2) - (0 ? 6:2) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:2) - (0 ? 6:2) + 1))))))) << (0 ?
 6:2))) | (((gctUINT32) (0x13 & ((gctUINT32) ((((1 ? 6:2) - (0 ? 6:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:2) - (0 ? 6:2) + 1))))))) << (0 ? 6:2))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7))));
                break;

            case gcvSURF_R8:
                config &= (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:2) - (0 ? 6:2) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:2) - (0 ? 6:2) + 1))))))) << (0 ?
 6:2))) | (((gctUINT32) (0x14 & ((gctUINT32) ((((1 ? 6:2) - (0 ? 6:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:2) - (0 ? 6:2) + 1))))))) << (0 ? 6:2))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7))));
                break;

            default:
                return gcvSTATUS_NOT_SUPPORTED;
        }
    }

    gcmONERROR(gcoHARDWARE_Load2DState32(
        Hardware,
        0x12F00 + regOffset,
        config
        ));

    gcmONERROR(gcoHARDWARE_Load2DState32(
        Hardware,
        0x12F40 + regOffset,
        SrcAddr
        ));

    gcmONERROR(gcoHARDWARE_Load2DState32(
        Hardware,
        0x12F20 + regOffset,
        SrcStatusAddr
        ));

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoTPHARDWARE_SetDstTPCCompression_V10(
    IN gcoHARDWARE Hardware,
    IN gctUINT Enable,
    IN gctUINT DstAddr,
    IN gctUINT DstStatusAddr,
    IN gceSURF_FORMAT DstFormat,
    IN gctUINT DstWidth,
    IN gctUINT DstHeight,
    IN gctUINT DstStride,
    IN gceSURF_ROTATION DstRotation
    )
{
    gceSTATUS status;
    gctUINT config;

    gcmHEADER_ARG("Hardware=0x%x Enable=%d DstAddr=0x%08x DstStatusAddr=0x%08x"
                  "DstFormat=%d DstWidth=%d DstHeight=%d DstStride=%d DstRotation=%d",
                   Hardware, Enable, DstAddr, DstStatusAddr, DstFormat,
                   DstWidth, DstHeight, DstStride, DstRotation);

    if (Enable == gcvFALSE)
    {
        config = (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))));
    }
    else
    {
        config = (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))));

        switch (DstFormat)
        {
            case gcvSURF_A8R8G8B8:
            case gcvSURF_A8B8G8R8:
            case gcvSURF_R8G8B8A8:
                config &= (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:4) - (0 ? 8:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 8:4) - (0 ? 8:4) + 1))))))) << (0 ?
 8:4))) | (((gctUINT32) (0x06 & ((gctUINT32) ((((1 ? 8:4) - (0 ? 8:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:4) - (0 ? 8:4) + 1))))))) << (0 ? 8:4))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:9) - (0 ? 9:9) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ?
 9:9))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 9:9) - (0 ? 9:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9))));
                break;

            case gcvSURF_A4R4G4B4:
            case gcvSURF_A4B4G4R4:
            case gcvSURF_R4G4B4A4:
                config &= (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:4) - (0 ? 8:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 8:4) - (0 ? 8:4) + 1))))))) << (0 ?
 8:4))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 8:4) - (0 ? 8:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:4) - (0 ? 8:4) + 1))))))) << (0 ? 8:4))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:9) - (0 ? 9:9) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ?
 9:9))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 9:9) - (0 ? 9:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9))));
                break;

            case gcvSURF_X8R8G8B8:
            case gcvSURF_X8B8G8R8:
            case gcvSURF_R8G8B8X8:
            case gcvSURF_B8G8R8X8:
                config &= (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:4) - (0 ? 8:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 8:4) - (0 ? 8:4) + 1))))))) << (0 ?
 8:4))) | (((gctUINT32) (0x05 & ((gctUINT32) ((((1 ? 8:4) - (0 ? 8:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:4) - (0 ? 8:4) + 1))))))) << (0 ? 8:4))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:9) - (0 ? 9:9) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ?
 9:9))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 9:9) - (0 ? 9:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9))));
                break;

            case gcvSURF_R5G6B5:
            case gcvSURF_B5G6R5:
                config &= (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:4) - (0 ? 8:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 8:4) - (0 ? 8:4) + 1))))))) << (0 ?
 8:4))) | (((gctUINT32) (0x04 & ((gctUINT32) ((((1 ? 8:4) - (0 ? 8:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:4) - (0 ? 8:4) + 1))))))) << (0 ? 8:4))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:9) - (0 ? 9:9) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ?
 9:9))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 9:9) - (0 ? 9:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9))));
                break;

            case gcvSURF_A1R5G5B5:
            case gcvSURF_A1B5G5R5:
            case gcvSURF_R5G5B5A1:
                config &= (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:4) - (0 ? 8:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 8:4) - (0 ? 8:4) + 1))))))) << (0 ?
 8:4))) | (((gctUINT32) (0x03 & ((gctUINT32) ((((1 ? 8:4) - (0 ? 8:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:4) - (0 ? 8:4) + 1))))))) << (0 ? 8:4))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:9) - (0 ? 9:9) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ?
 9:9))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 9:9) - (0 ? 9:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9))));
                break;

            case gcvSURF_RG16:
                config &= (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:4) - (0 ? 8:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 8:4) - (0 ? 8:4) + 1))))))) << (0 ?
 8:4))) | (((gctUINT32) (0x13 & ((gctUINT32) ((((1 ? 8:4) - (0 ? 8:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:4) - (0 ? 8:4) + 1))))))) << (0 ? 8:4))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:9) - (0 ? 9:9) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ?
 9:9))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 9:9) - (0 ? 9:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9))));
                break;

            case gcvSURF_R8:
                config &= (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:4) - (0 ? 8:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 8:4) - (0 ? 8:4) + 1))))))) << (0 ?
 8:4))) | (((gctUINT32) (0x14 & ((gctUINT32) ((((1 ? 8:4) - (0 ? 8:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:4) - (0 ? 8:4) + 1))))))) << (0 ? 8:4))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:9) - (0 ? 9:9) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ?
 9:9))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 9:9) - (0 ? 9:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9))));
                break;

            default:
                return gcvSTATUS_NOT_SUPPORTED;
        }
    }

    gcmONERROR(gcoHARDWARE_Load2DState32(
        Hardware,
        0x0138C,
        config
        ));

    gcmONERROR(gcoHARDWARE_Load2DState32(
        Hardware,
        0x01394,
        DstAddr
        ));

    gcmONERROR(gcoHARDWARE_Load2DState32(
        Hardware,
        0x01390,
        DstStatusAddr
        ));

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoTPHARDWARE_StartTPCCompression_V10(
    IN gcoHARDWARE Hardware,
    IN gctUINT SrcCompressed,
    IN gctUINT DstCompressed
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
gcoTPHARDWARE_EndTPCCompression_V10(
    IN gcoHARDWARE Hardware
    )
{
    return gcvSTATUS_OK;
}
#endif

