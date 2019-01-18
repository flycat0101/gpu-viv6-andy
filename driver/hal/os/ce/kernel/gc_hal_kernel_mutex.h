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


#ifndef _gc_hal_kernel_mutex_h_
#define _gc_hal_kernel_mutex_h_

#ifdef __cplusplus
extern "C" {
#endif

/* Create a new mutex. */
gceSTATUS
gckOS_CreateMutex(
    IN gckOS Os,
    OUT gctPOINTER * Mutex
    );

#ifdef __cplusplus
}
#endif

#endif


