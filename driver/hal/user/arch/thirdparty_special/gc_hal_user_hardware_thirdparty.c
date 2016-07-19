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


#if gcdENABLE_2D
gceSTATUS
gcoTPHARDWARE_CheckSurface(
    IN gctUINT Addr,
    IN gctUINT StatusAddr,
    IN gceSURF_FORMAT Format,
    IN gctUINT Width,
    IN gctUINT Height,
    IN gctUINT Stride,
    IN gceSURF_ROTATION Rotation,
    IN gctUINT Tiling,
    IN gceTPTYPE TPCType
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT align = 0, alignS = 0;

    switch (TPCType)
    {
        case gcvTP_V10:
            if (Addr & 1023)
            {
                status = gcvSTATUS_INVALID_ARGUMENT;
            }
            else if (Width & 15)
            {
                status = gcvSTATUS_INVALID_ARGUMENT;
            }
            else if (Height & 15)
            {
                status = gcvSTATUS_INVALID_ARGUMENT;
            }
            else if (Stride & 15)
            {
                status = gcvSTATUS_INVALID_ARGUMENT;
            }
            else
            {
                switch (Format)
                {
                    case gcvSURF_A8R8G8B8:
                    case gcvSURF_A8B8G8R8:
                    case gcvSURF_R8G8B8A8:
                    case gcvSURF_A4R4G4B4:
                    case gcvSURF_A4B4G4R4:
                    case gcvSURF_R4G4B4A4:
                    case gcvSURF_X8R8G8B8:
                    case gcvSURF_X8B8G8R8:
                    case gcvSURF_R8G8B8X8:
                    case gcvSURF_B8G8R8X8:
                    case gcvSURF_R5G6B5:
                    case gcvSURF_B5G6R5:
                    case gcvSURF_A1R5G5B5:
                    case gcvSURF_A1B5G5R5:
                    case gcvSURF_R5G5B5A1:
                    case gcvSURF_RG16:
                    case gcvSURF_R8:
                        break;

                    default:
                        status = gcvSTATUS_INVALID_ARGUMENT;
                        break;
                }
            }

            break;

        case gcvTP_V11:
            if (StatusAddr >= Addr)
            {
                status = gcvSTATUS_INVALID_ARGUMENT;
            }

            if (Width & 31)
            {
                status = gcvSTATUS_INVALID_ARGUMENT;
            }
            else if (Height & 7)
            {
                status = gcvSTATUS_INVALID_ARGUMENT;
            }

            switch (Format)
            {
                case gcvSURF_A8R8G8B8:
                case gcvSURF_X8R8G8B8:
                case gcvSURF_A2R10G10B10:
                    align = 1023;
                    alignS = 127;
                    break;

                case gcvSURF_NV12:
                    align = 255;
                    alignS = 31;
                    break;

                case gcvSURF_P010:
                    align = 511;
                    alignS = 63;
                    break;

                default:
                    status = gcvSTATUS_NOT_SUPPORTED;
                    break;
            }

            if (Addr & align)
            {
                status = gcvSTATUS_INVALID_ARGUMENT;
            }

            if (Stride & alignS)
            {
                status = gcvSTATUS_INVALID_ARGUMENT;
            }

            break;

        default:
            break;
    }

    return status;
}

gceSTATUS
gcoTPHARDWARE_QueryStateCmdLen(
    IN gceTPCMDTYPE CmdType,
    IN gctUINT Num,
    IN gceTPTYPE TPCType,
    OUT gctUINT* StateCmdLen
    )
{
    gctUINT len = 0;

    switch (TPCType)
    {
        case gcvTP_V10:
        {
            gctUINT NoCompressionCmdLen = 2;
            gctUINT SrcCompressionCmdLen = 6;
            gctUINT DstCompressionCmdLen = 6;

            switch(CmdType)
            {
                case gcvTP_CMD_NO_COMPRESSION:
                    len = gcmTPALIGN(NoCompressionCmdLen, 2);
                    break;

                case gcvTP_CMD_HAS_COMPRESSION:
                    len = gcmTPALIGN(NoCompressionCmdLen, 2);
                    break;

                case gcvTP_CMD_SRC_COMPRESSION:
                    len = gcmTPALIGN(SrcCompressionCmdLen, 2) * Num;
                    break;

                case gcvTP_CMD_DST_COMPRESSION:
                    len = gcmTPALIGN(DstCompressionCmdLen, 2) * Num;
                    break;

                default:
                    return gcvSTATUS_NOT_SUPPORTED;
            }
            break;
        }

        case gcvTP_V11:
        {
            gctUINT NoCompressionCmdLen = 24;
            gctUINT HasCompressionCmdLen = 24;
            gctUINT SrcCompressionCmdLen = 4;
            gctUINT DstCompressionCmdLen = 12;

            switch(CmdType)
            {
                case gcvTP_CMD_NO_COMPRESSION:
                    len = gcmTPALIGN(NoCompressionCmdLen, 2);
                    break;

                case gcvTP_CMD_HAS_COMPRESSION:
                    len = gcmTPALIGN(HasCompressionCmdLen, 2);
                    break;

                case gcvTP_CMD_SRC_COMPRESSION:
                    len = gcmTPALIGN(SrcCompressionCmdLen, 2) * Num;
                    break;

                case gcvTP_CMD_DST_COMPRESSION:
                    len = gcmTPALIGN(DstCompressionCmdLen, 2) * Num;
                    break;

                default:
                    return gcvSTATUS_NOT_SUPPORTED;
            }
            break;
        }

        default:
            break;
    }

    if (StateCmdLen != gcvNULL)
        *StateCmdLen = len;

    return gcvSTATUS_OK;
}
#endif

