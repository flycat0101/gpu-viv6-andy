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


#ifndef __gralloc_helper_h_
#define __gralloc_helper_h_

#include <stdint.h>
#include <hardware/gralloc.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _gcoSURF;
typedef struct _gcoSURF * gcoSURF;

/*
 * Create surface from gralloc buffer.
 * Use gcoSURF_Destroy to free it.
 *
 * Returns errno, negative values on error, 0 on success.
 */
int gralloc_buffer_create_surface(buffer_handle_t handle,
            gcoSURF *pSurface);

/*
 * Sync surface parameter (tile status) from underlying handle fd.
 * @pTimeStamp      IN-OUT, IN last timeStamp value and OUT current timestamp
 *                  of handle content. Use to check if handle content is updated.
 *
 * Tile status parameters are saved in underlying fd. So it's passed accross
 * processes.
 *
 * Returns errno, negative value on error, 0 on success, 1 on success and not
 * updated.
 */
int gralloc_buffer_sync_surface(buffer_handle_t handle,
            gcoSURF surface, uint64_t *pTimeStamp);

/*
 * Sync parameter from surface to underlying handle fd.
 *
 * Tile status parameters are saved in underlying fd. So it's passed accross
 * processes.
 *
 * Returns errno, negative values on error, 0 on success.
 */
int gralloc_buffer_sync_from_surface(buffer_handle_t handle,
            gcoSURF surface);

#ifdef __cplusplus
}
#endif
#endif
