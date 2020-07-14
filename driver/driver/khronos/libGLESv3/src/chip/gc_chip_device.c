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


/*
*** Fill global information when process is attached
*** all DP related globals need to be in this structure
*/

#include "gc_es_context.h"
#include "gc_chip_context.h"
#include "gc_es_device.h"

#define _GC_OBJ_ZONE    gcdZONE_ES30_DEVICE

__GLchipGlobal dpGlobalInfo;

GLboolean
__glDpDeinitialize()
{
    return GL_TRUE;
}

GLboolean
__glDpInitialize(
    __GLdeviceStruct *deviceEntry
    )
{
    gcmHEADER_ARG("deviceEntry=0x%x", deviceEntry);

    deviceEntry->devDeinitialize    = __glDpDeinitialize;
    deviceEntry->devCreateContext   = __glChipCreateContext;
    deviceEntry->devGetConstants    = __glChipGetDeviceConstants;
    deviceEntry->devUpdateDrawable  = __glChipUpdateDrawable;
    deviceEntry->devDestroyDrawable = __glChipDestroyDrawable;

    __GL_MEMZERO(&dpGlobalInfo, sizeof(__GLchipGlobal));
    dpGlobalInfo.processID = (GLuint)(gctUINTPTR_T)gcoOS_GetCurrentProcessID();

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;
}

