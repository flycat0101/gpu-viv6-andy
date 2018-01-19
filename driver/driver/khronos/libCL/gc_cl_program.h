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


#ifndef __gc_cl_program_h_
#define __gc_cl_program_h_

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************\
************************* Program Object Definition *************************
\******************************************************************************/

typedef struct _cl_program
{
    clsIcdDispatch_PTR      dispatch;
    cleOBJECT_TYPE          objectType;
    gctUINT                 id;
    gcsATOM_PTR             referenceCount;

    clsContext_PTR          context;
    gctUINT                 numDevices;
    clsDeviceId_PTR *       devices;
    clsKernel_PTR           kernels;

    gctSTRING               source;
    gctUINT                 binarySize;
    gctUINT8_PTR            binary;
    gctSTRING               buildOptions;
    gctSTRING               linkOptions;
    gctSTRING               compileOptions;
    gctSTRING               buildLog;
    cl_build_status         buildStatus;
    cl_program_binary_type  binaryType;
    gctINT                  status;
}
clsProgram;

gctINT
clfRetainProgram(
    cl_program Program
    );

gctINT
clfReleaseProgram(
    cl_program Program
    );

#ifdef __cplusplus
}
#endif

#endif  /* __gc_cl_program_h_ */
