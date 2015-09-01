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


#ifndef __gc_cl_device_h_
#define __gc_cl_device_h_

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************\
************************* Device Object Definition *************************
\******************************************************************************/

typedef struct _cl_device_id
{
    clsIcdDispatch_PTR      dispatch;
    cleOBJECT_TYPE          objectType;
    gctUINT                 id;
    gcsATOM_PTR             referenceCount;

    clsPlatformId_PTR       platform;

    gctCHAR                 name[64];
    gctSTRING               vendor;
    gctSTRING               deviceVersion;
    gctSTRING               driverVersion;
    gctSTRING               openCLCVersion;
    gctSTRING               profile;
    gctSTRING               extensions;
    cl_device_type          type;
    gctUINT                 vendorId;

    gcoCL_DEVICE_INFO       deviceInfo;

    gctUINT                 gpuId;
}
clsDeviceId;


#ifdef __cplusplus
}
#endif

#endif  /* __gc_cl_device_h_ */
