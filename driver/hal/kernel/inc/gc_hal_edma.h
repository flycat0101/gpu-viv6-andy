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


#ifndef __gc_hal_edma_h_
#define __gc_hal_edma_h_
#include "gc_hal_types.h"

typedef unsigned int gctEDMA_HANDLE;

typedef enum
{
    gcvEDMA_IOCTL_CMD_GET_CHAN = 0,
    gcvEDMA_IOCTL_CMD_CFG_AND_SUBMIT,
    gcvEDMA_IOCTL_CMD_START,
    gcvEDMA_IOCTL_CMD_GET_STATUS,
    gcvEDMA_IOCTL_CMD_TERMINATE_ALL,
    gcvEDMA_IOCTL_CMD_RELEASE,
}gcvEDMA_IOCTL_CMD;

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
    IN gcvEDMA_IOCTL_CMD cmd;

    INOUT gctEDMA_HANDLE edma_chan;
    IN gctUINT32 node;
    INOUT size_t len;
    IN gcvEDMA_DIRECTION dir;
    IN gctBOOL sync_wait;
    OUT gctUINT32 edma_status;
}gcsEDMA_TRANSFACTION;
#endif


