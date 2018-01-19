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


#ifndef __gc_cl_sampler_h_
#define __gc_cl_sampler_h_

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************\
************************* Sampler Object Definition *************************
\******************************************************************************/

typedef enum _cleSAMPLER
{
    /* First byte: addressing mode. */
    CLK_ADDRESS_NONE                = (CL_ADDRESS_NONE             & 0xF),
    CLK_ADDRESS_CLAMP_TO_EDGE       = (CL_ADDRESS_CLAMP_TO_EDGE    & 0xF),
    CLK_ADDRESS_CLAMP               = (CL_ADDRESS_CLAMP            & 0xF),
    CLK_ADDRESS_REPEAT              = (CL_ADDRESS_REPEAT           & 0xF),
    CLK_ADDRESS_MIRRORED_REPEAT     = (CL_ADDRESS_MIRRORED_REPEAT  & 0xF),

    /* Second byte: filter mode. */
    CLK_FILTER_NEAREST              = ((CL_FILTER_NEAREST          & 0xF) << 8),
    CLK_FILTER_LINEAR               = ((CL_FILTER_LINEAR           & 0xF) << 8),

    /* Third byte: normalized coords. */
    CLK_NORMALIZED_COORDS_FALSE     = (0x0 << 16),
    CLK_NORMALIZED_COORDS_TRUE      = (0x1 << 16)
}
cleSAMPLER;

typedef struct _cl_sampler
{
    clsIcdDispatch_PTR      dispatch;
    cleOBJECT_TYPE          objectType;
    gctUINT                 id;
    gcsATOM_PTR             referenceCount;

    clsContext_PTR          context;

    gctBOOL                 normalizedCoords;
    cl_addressing_mode      addressingMode;
    cl_filter_mode          filterMode;

    /* The value used in kernel function. */
    cleSAMPLER              samplerValue;
}
clsSampler;


#ifdef __cplusplus
}
#endif

#endif  /* __gc_cl_sampler_h_ */
