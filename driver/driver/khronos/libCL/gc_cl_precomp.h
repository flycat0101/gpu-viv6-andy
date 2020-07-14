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


#ifndef __gc_cl_precomp_h_
#define __gc_cl_precomp_h_

#include <gc_hal.h>
#include <gc_hal_user.h>
#include <drvi/gc_vsc_drvi_interface.h>
#include <gc_hal_user_os_memory.h>
#include <gc_hal_cl.h>
#include <old_impl/gc_vsc_old_drvi_interface.h>

#if BUILD_OPENCL_ICD
#ifndef WIN32
/* Need to rename all CL API functions to prevent ICD loader functions calling */
/* themselves via the dispatch table. Include this before cl headers. */
#include "gc_cl_icd_rename_api.h"
#endif
#endif

#include <CL/opencl.h>

#include "gc_cl_icd_structs.h"
#include "gc_cl.h"
#include "gc_cl_platform.h"
#include "gc_cl_device.h"
#include "gc_cl_context.h"
#include "gc_cl_kernel.h"
#include "gc_cl_command.h"
#include "gc_cl_mem.h"
#include "gc_cl_program.h"
#include "gc_cl_event.h"
#include "gc_cl_sampler.h"
#include "gc_cl_log.h"

#define cldVERSION12    "OpenCL 1.2 "                        /* Device Version   */
#define clfVERSION12    "OpenCL 1.2 V" gcvVERSION_STRING     /* Driver Version   */
#define clcVERSION12    "OpenCL C 1.2 "                      /* OpenCL C Version */
#define cldVERSION11    "OpenCL 1.1 "                        /* Device Version   */
#define clfVERSION11    "OpenCL 1.1 V" gcvVERSION_STRING     /* Driver Version   */
#define clcVERSION11    "OpenCL C 1.1 "                      /* OpenCL C Version */

#define NEW_READ_WRITE_IMAGE    0
#define MAX_KEY_DATA_SIZE 512
#define __CL_INSTANCE_HASH_ENTRY_NUM   32
#define __CL_INSTANCE_HASH_ENTRY_SIZE  32
extern cl_device_id clgDefaultDevice;

extern gctUINT
clfEvaluateCRC32(
    gctPOINTER pData,
    gctUINT dataSizeInByte
    );

#endif /* __gc_cl_precomp_h_ */
