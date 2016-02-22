/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_cl_platform_h_
#define __gc_cl_platform_h_

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************\
************************* Platform Object Definition *************************
\******************************************************************************/

typedef struct _cl_platform_id
{
    clsIcdDispatch_PTR      dispatch;
    cleOBJECT_TYPE          objectType;
    gctUINT                 id;
    gcsATOM_PTR             referenceCount;

    gctUINT                 numDevices;
    clsDeviceId_PTR         devices;
    clsContext_PTR          contexts;

    gctSTRING               name;
    gctSTRING               vendor;
    gctSTRING               version;
    gctSTRING               Cversion;
    gctSTRING               profile;
    gctSTRING               extensions;
    gctSTRING               suffix;

    gctPOINTER              compilerMutex;
    gctHANDLE               dll;

    /* gcCompileKernel */
    gctCLCompiler           compiler11;

    /* gcCLCompilePrgram */
    gceSTATUS               (*compiler)(
                                IN gcoHAL Hal,
                                IN gctUINT SourceSize,
                                IN gctCONST_STRING Source,
                                IN gctCONST_STRING Options,
                                IN gctUINT NumInputHeaders,
                                IN gctCONST_STRING *InputHeaders,
                                IN gctCONST_STRING *HeaderIncludeNames,
                                OUT gcSHADER * Binary,
                                OUT gctSTRING * Log
                                );

    gceSTATUS   (*loadCompiler)(void);
    gceSTATUS   (*unloadCompiler)(void);
}
clsPlatformId;

void
clfGetDefaultPlatformID(
    clsPlatformId_PTR * Platform
    );


#ifdef __cplusplus
}
#endif

#endif  /* __gc_cl_platform_h_ */
