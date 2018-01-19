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


#ifndef __gc_hwc_debug_h_
#define __gc_hwc_debug_h_

/*******************************************************************************
** Debug options.
*/

/*
    hwc.debug.dump_bitmap

       Dump source input and target output into bitmap (/data/dump).
       Make sure /data/dump has write permission.
*/

/*
    hwc.debug.dump_compose

       Dump information of source layers.
       Dump 2D compositioin detail.
       Dump hwc composition time of each frame.
*/

/*
    hwc.debug.dump_split_area

       Dump area spliting information.
       Dump swap area information (swap rectangle optimization).
       Dump only when hwc compsoition.
*/


/******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include "gc_hwc.h"

void
hwcDumpLayer(
    IN hwc_layer_list_t * List
    );

void
hwcDumpBitmap(
    IN hwc_layer_list_t * List,
    IN android_native_buffer_t * BackBuffer
    );

void
hwcDumpArea(
    IN hwcArea * Area
    );

#ifdef __cplusplus
}
#endif

#endif

