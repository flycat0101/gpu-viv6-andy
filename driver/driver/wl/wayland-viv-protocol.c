/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <stdlib.h>
#include <stdint.h>
#include "wayland-util.h"

extern const struct wl_interface wl_buffer_interface;

static const struct wl_interface *types[] = {
    &wl_buffer_interface,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

static const struct wl_message wl_viv_requests[] = {
    { "create_buffer", "nuuuiiiu", types + 0 },
};

WL_EXPORT const struct wl_interface wl_viv_interface = {
    "wl_viv", 1,
    1, wl_viv_requests,
    0, NULL,
};

