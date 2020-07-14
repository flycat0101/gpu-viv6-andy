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


#include "gc_hwc.h"


/* overlay. */
int
hwcInitOverlay(
    IN hwcContext * Context,
    IN hwcDisplay * Display
    )
{
    /* TODO (Soc-vendor): Init overlay for display. */
    return 0;
}

int
hwcFinishOverlay(
    IN hwcContext * Context,
    IN hwcDisplay * Display
    )
{
    /* Finish overlay for dislay. */
    return 0;
}

int
hwcOverlayFrame(
    IN hwcContext * Context,
    IN hwcDisplay * Display,
    IN hwc_display_contents_1_t * HwDisplay
    )
{
    /* Setup overlay frame. */
    return 0;
}

