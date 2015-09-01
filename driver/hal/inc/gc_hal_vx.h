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


#ifndef __gc_hal_user_vx_h_
#define __gc_hal_user_vx_h_

#if gcdUSE_VX
#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************\
****************************** Object Declarations *****************************
\******************************************************************************/

/* VX kernel parameters. */
typedef struct _gcsVX_KERNEL_PARAMETERS * gcsVX_KERNEL_PARAMETERS_PTR;

typedef struct _gcsVX_KERNEL_PARAMETERS
{
    gctUINT32           step;

    gctUINT32           kernel;
    gctUINT32           xmin;
    gctUINT32           xmax;
    gctUINT32           xstep;

    gctUINT32           ymin;
    gctUINT32           ymax;
    gctUINT32           ystep;

    gctUINT32           groupSizeX;
    gctUINT32           groupSizeY;

    gctUINT32           threadcount;
    gctUINT32           policy;
    gctUINT32           rounding;
    gctFLOAT            scale;
    gctUINT32           borders;
    gctUINT32           constant_value;
    gctUINT32           volume;
    gctUINT32           clamp;
    gctUINT32           inputhack;
    gctUINT32           outputhack;

    gctUINT32           inputMultipleWidth;
    gctUINT32           outputMultipleWidth;

    gctUINT32           order;

    gctUINT32           itype[10];
    gctUINT32           otype[10];
    gctUINT32           input_count;
    gctUINT32           output_count;

    gcoVX_Instructions  instructions;

    gctINT16            *matrix;
    gctUINT32           col;
    gctUINT32           row;

    gcsSURF_NODE_PTR   node;

    gceSURF_FORMAT      inputFormat;
    gceSURF_FORMAT      outputFormat;

    gctUINT8 isUseInitialEstimate;
    gctINT32 maxLevel;
    gctINT32 winSize;
}
gcsVX_KERNEL_PARAMETERS;

/******************************************************************************\
****************************** API Declarations *****************************
\******************************************************************************/
gceSTATUS
gcoVX_Initialize();

gceSTATUS
gcoVX_BindImage(
    IN gctUINT32            Index,
    IN gcsVX_IMAGE_INFO_PTR Info
    );

gceSTATUS
gcoVX_BindUniform(
    IN gctUINT32            RegAddress,
    IN gctUINT32            Index,
    IN gctUINT32            *Value,
    IN gctUINT32            Num
    );

gceSTATUS
gcoVX_Commit(
    IN gctBOOL Flush,
    IN gctBOOL Stall,
    INOUT gctPOINTER *pCmdBuffer,
    INOUT gctUINT32  *pCmdBytes
    );

gceSTATUS
gcoVX_Replay(
    IN gctPOINTER CmdBuffer,
    IN gctUINT32  CmdBytes
    );

gceSTATUS
gcoVX_InvokeKernel(
    IN gcsVX_KERNEL_PARAMETERS_PTR  Parameters
    );

gceSTATUS
gcoVX_Upload(
    IN gctUINT32_PTR    Point,
    IN gctUINT32        Size,
    IN gctBOOL          Upload,
    OUT gctUINT32_PTR   Physical,
    OUT gctUINT32_PTR   Logical,
    OUT gcsSURF_NODE_PTR* Node
    );

gceSTATUS
gcoVX_AllocateMemory(
    IN gctUINT32        Size,
    OUT gctUINT32_PTR   Logical,
    OUT gctUINT32_PTR   Physical,
    OUT gcsSURF_NODE_PTR* Node
    );

gceSTATUS
gcoVX_FreeMemory(
    IN gcsSURF_NODE_PTR Node
    );

gceSTATUS
gcoVX_DestroyNode(
    IN gcsSURF_NODE_PTR Node
    );

gceSTATUS
gcoVX_KernelConstruct(
    IN OUT gcoVX_Hardware_Context   *Context
    );

#ifdef __cplusplus
}
#endif
#endif
#endif /* __gc_hal_user_vx_h_ */
