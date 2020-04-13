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


#ifndef __gc_hal_edma_api_h_
#define __gc_hal_edma_api_h_
#include "gc_hal_edma.h"

#ifdef __cplusplus
extern "C" {
#endif
/*******************************************************************************
**  gcoOS_DMAAllocate
**      allocate edma resource for tranaction.
**
**  INPUT:
**      gcvEDMA_DIRECTION dir
**          dma direction.
**
**  OUTPUT:
**      gctEDMA_HANDLE *handle
**          dma channel handle for tranaction.
*/
gceSTATUS gcoOS_DMAAllocate(
    IN gcvEDMA_DIRECTION dir,
    OUT gctEDMA_HANDLE *handle
    );

/*******************************************************************************
**  gcoOS_DMACopy
**      copy data from host memory/device to device/host memory.
**
**  INPUT:
**      gctEDMA_HANDLE handle
**          dma channel handle for tranaction.
**      gctUINT32 node
**          vodeo memory handle
**      gctSIZE_T len
**          transaction size in bytes
**  OUTPUT:
**
*/
gceSTATUS gcoOS_DMACopy(
    IN gctEDMA_HANDLE handle,
    IN gctUINT32 node,
    IN gctSIZE_T len
    );

/*******************************************************************************
**  gcoOS_DMAFree
**      allocate edma resource for tranaction.
**
**  INPUT:
**      gctEDMA_HANDLE *handle
**          dma channel handle for tranaction.
**
**
**  OUTPUT:
*/
gceSTATUS gcoOS_DMAFree(
     IN gctEDMA_HANDLE handle);

#ifdef __cplusplus
}
#endif
#endif/*__gc_hal_edma_api_h_*/


