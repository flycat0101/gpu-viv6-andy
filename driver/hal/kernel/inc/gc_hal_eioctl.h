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


#ifndef __gc_hal_eioctl_h_
#define __gc_hal_eioctl_h_
#include "gc_hal_types.h"

typedef unsigned int gctEDMA_HANDLE;

typedef enum
{
    gcvEXTERN_IOCTL_CMD_EDMA_GET_CHAN = 0,
    gcvEXTERN_IOCTL_CMD_EDMA_CFG_AND_SUBMIT,
    gcvEXTERN_IOCTL_CMD_EDMA_START,
    gcvEXTERN_IOCTL_CMD_EDMA_GET_STATUS,
    gcvEXTERN_IOCTL_CMD_EDMA_TERMINATE_ALL,
    gcvEXTERN_IOCTL_CMD_EDMA_RELEASE,
}gcvEXTERN_IOCTL_CMD;

typedef enum
{
    gcvDMA_UNUSE = 0,
    gcvDMA_USED,
}gcvEDMA_STATE;

typedef enum
{
    gcvEDMA_MEM_TO_DEV = 1,
    gcvEDMA_DEV_TO_MEM,
}gcvEDMA_DIRECTION;

typedef struct
{
    INOUT gctEDMA_HANDLE edma_chan;
    IN gctUINT32 node;
    INOUT size_t len;
    IN gcvEDMA_DIRECTION dir;
    IN gctBOOL sync_wait;
    OUT gctUINT32 edma_status;
}
gcsEDMA_TRANSFACTION;

typedef struct
{
    gcvEXTERN_IOCTL_CMD cmd;

    union _uu
    {
        gcsEDMA_TRANSFACTION edma;
    }
    u;
}
gcsEXTERN_IOCTL_INTERFACE;

#endif



