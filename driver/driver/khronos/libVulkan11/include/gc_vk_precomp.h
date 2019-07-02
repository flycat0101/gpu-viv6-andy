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


#ifndef __gc_vk_precomp_h__
#define __gc_vk_precomp_h__

#include "gc_hal.h"
#include "gc_feature_database.h"
#include "gc_hal_user.h"
#include "gc_hal_base.h"
#include "gc_hal_user_hardware.h"
#include "gc_spirv_cvter.h"

#include "gc_vk_os.h"
#include "gc_vk_types.h"
#include "gc_vk_debug.h"
#include "gc_vk_core.h"
#include "gc_vk_dispatch.h"
#include "gc_vk_context.h"
#include "gc_vk_devqueue.h"
#include "gc_vk_resource.h"
#include "gc_vk_desc.h"
#include "gc_vk_framebuffer.h"
#include "gc_vk_pipeline.h"
#include "gc_vk_query.h"
#include "gc_vk_cmdbuf.h"
#include "gc_vk_wsi.h"
#include "gc_vk_utils.h"

#include "chip/gc_halti5_chip.h"

#define __VK_DRIVER_VERSION     VK_MAKE_VERSION(gcvVERSION_MAJOR, gcvVERSION_MINOR, gcvVERSION_PATCH)
#define __VK_DEVICE_VENDOR_ID   ((gctUINT32) 0x10002)

#endif /* __gc_vk_precomp_h__ */



