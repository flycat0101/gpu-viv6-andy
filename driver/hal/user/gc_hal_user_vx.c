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


#include "gc_hal_user_precomp.h"
#if gcdUSE_VX
#include "gc_hal_vx.h"
#ifdef _WIN32
#include "..\..\driver\khronos\libOpenVX\kernels\gc_vxk_be_kernel.h"
#elif defined LINUX
#include "../../driver/khronos/libOpenVX/kernels/gc_vxk_be_kernel.h"
#endif

#define _GC_OBJ_ZONE            gcvZONE_VX

/******************************************************************************\
|********************************* Structures *********************************|
\******************************************************************************/

/******************************************************************************\
|*********************************** Macros ***********************************|
\******************************************************************************/

/******************************************************************************\
|******************************* API Code *******************************|
\******************************************************************************/

static gctBOOL Is_OneDimKernel(gctUINT32 kernel) {
    return (kernel == gcvVX_KERNEL_OPTICAL_FLOW_PYR_LK);
}

static gctBOOL Is_HistogramKernel(gctUINT32 kernel) {
    return ((kernel == gcvVX_KERNEL_HISTOGRAM) || (kernel == gcvVX_KERNEL_EQUALIZE_HISTOGRAM));
}

gceSTATUS
gcoVX_Initialize()
{
    gceSTATUS status;
    gceAPI currentApi;
    gcsTLS_PTR __tls__;

    gcmHEADER();

    gcmONERROR(gcoOS_GetTLS(&__tls__));

    if (__tls__->currentHardware != gcvNULL)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    /* Set the hardware type. */
    gcmONERROR(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D));

    /* Switch to the 3D pipe. */
    gcmONERROR(gcoHARDWARE_SelectPipe(gcvNULL, gcvPIPE_3D, gcvNULL));

    /* Get Current API. */
    gcmVERIFY_OK(gcoHARDWARE_GetAPI(gcvNULL, &currentApi, gcvNULL));

    if (currentApi == 0)
    {
        /* Set HAL API to OpenCL only when there is API is not set. */
        gcmVERIFY_OK(gcoHARDWARE_SetAPI(gcvNULL, gcvAPI_OPENCL));
    }

    if (!gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_PIPE_CL))
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }


    gcmVERIFY_OK(gcoHARDWARE_SetMultiGPUMode(gcvNULL, gcvMULTI_GPU_MODE_INDEPENDENT));

    gcmVERIFY_OK(gcoHARDWAREVX_InitVX(gcvNULL));

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}
gceSTATUS
gcoVX_Replay(
    IN gctPOINTER CmdBuffer,
    IN gctUINT32  CmdBytes
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("CmdBuffer=0x%x CmdBytes=%d", CmdBuffer, CmdBytes);

    gcmONERROR(gcoHARDWARE_ReplayCmdVX(gcvNULL, CmdBuffer, CmdBytes));

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoVX_Commit(
    IN gctBOOL Flush,
    IN gctBOOL Stall,
    INOUT gctPOINTER *pCmdBuffer,
    INOUT gctUINT32  *pCmdBytes
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Flush=%d Stall=%d", Flush, Stall);

    if (Flush)
    {
        gcmONERROR(gcoHARDWARE_FlushPipe(gcvNULL, gcvNULL));
    }

    /* Commit the command buffer to hardware. */
    gcmONERROR(gcoHARDWARE_CommitCmdVX(gcvNULL, pCmdBuffer, pCmdBytes));

    if (Stall)
    {
        /* Stall the hardware. */
        gcmONERROR(gcoHARDWARE_Stall(gcvNULL));
    }

    /* Success. */
    status = gcvSTATUS_OK;

OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoVX_BindImage(
    IN gctUINT32            Index,
    IN gcsVX_IMAGE_INFO_PTR Info
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Index=%d, Info=%p", Index, Info);

    gcmONERROR(gcoHARDWARE_BindImageVX(gcvNULL, Index, Info));

    /* Success. */
    status = gcvSTATUS_OK;

OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoVX_BindUniform(
    IN gctUINT32            RegAddress,
    IN gctUINT32            Index,
    IN gctUINT32            *Value,
    IN gctUINT32            Num
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Index=%d, Value=%p", Index, Value);

    /* GCREG_GPIPE_UNIFORMS_Address : 0x34000
     * GCREG_SH_UNIFORMS_Address    : 0x30000
     * GCREG_PIXEL_UNIFORMS_Address : 0x36000
     */
    gcmONERROR(gcoHARDWARE_BindUniform(gcvNULL, RegAddress, Index, Value, Num));

    /* Success. */
    status = gcvSTATUS_OK;

OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
}


gceSTATUS
gcoVX_InvokeKernel(
    IN gcsVX_KERNEL_PARAMETERS_PTR  Parameters
    )
{
    gcsVX_THREAD_WALKER_PARAMETERS  twParameters;
    gceSTATUS                       status = gcvSTATUS_OK;
    gctUINT32                       xCount;
    gctUINT32                       yCount;
    gctUINT32                       groupSizeX;
    gctUINT32                       groupSizeY;
    gctUINT32                       groupCountX;
    gctUINT32                       groupCountY;
    gctUINT32                       maxShaderGroups;
    gctUINT32                       maxGroupSize;
    const gctUINT32                 shaderGroupSize = 32;

    gcmHEADER_ARG("Parameters=%p", Parameters);

    /* Calculate thread allocation. */
    xCount = gcmALIGN_NP2(Parameters->xmax - Parameters->xmin, Parameters->xstep) / Parameters->xstep;
    yCount =  gcmALIGN_NP2((Parameters->ymax - Parameters->ymin), Parameters->ystep) / Parameters->ystep;

    /*  Compute work group size in each direction. Use 109 rather than 128. Some temp regs may be reserved if gpipe is running. */
    gcmASSERT(Parameters->instructions.regs_count);
    maxShaderGroups = 109/Parameters->instructions.regs_count;
    maxGroupSize = maxShaderGroups * shaderGroupSize;

    /* Default groupSize */
    if (Parameters->groupSizeX == 0)
    {
        Parameters->groupSizeX = shaderGroupSize;
    }

    if (Parameters->groupSizeY == 0)
    {
        Parameters->groupSizeY = shaderGroupSize;
    }

    groupSizeX = (xCount >= Parameters->groupSizeX) ? Parameters->groupSizeX : xCount;
    groupSizeY = (yCount >= Parameters->groupSizeY) ? Parameters->groupSizeY : yCount;

    /* Compute number of groups in each direction. */
    groupCountX = (xCount + groupSizeX - 1) / groupSizeX;
    groupCountY = (yCount + groupSizeY - 1) / groupSizeY;

    /* If xCount/yCount is not multiple of corresponding groupsize
     * Most kernels should be OK. But histogram should not count any extra borders in bin[0].
     * So adjust groupSize to a next lower number that can be divided by xCount */
    if (Is_HistogramKernel(Parameters->kernel))
    {
        while (groupCountX * groupSizeX != xCount)
        {
            groupCountX++;
            groupSizeX = gcmMAX(1, xCount / groupCountX);
        }

        while (groupCountY * groupSizeY != yCount)
        {
            groupCountY++;
            groupSizeY = gcmMAX(1, yCount / groupCountY);
        }
    }

    if (groupSizeX * groupSizeY > maxGroupSize)
    {
        if (groupSizeX > groupSizeY)
        {
            groupSizeX = gcmMAX(1, maxGroupSize / groupSizeY);
        }
        else
        {
            groupSizeY = gcmMAX(1, maxGroupSize / groupSizeX);
        }

        /*re-compute number of groups in each direction when groupSize has beeen changed*/
        groupCountX = (xCount + groupSizeX - 1) / groupSizeX;
        groupCountY = (yCount + groupSizeY - 1) / groupSizeY;
    }

    gcoOS_ZeroMemory(&twParameters, gcmSIZEOF(gcsVX_THREAD_WALKER_PARAMETERS));

    twParameters.valueOrder         = Parameters->order;

    /* ToDo: Not to detect kernel for workDim */
    if (Is_OneDimKernel(Parameters->kernel))
        twParameters.workDim = 1;
    else
        twParameters.workDim = 2;

    twParameters.workGroupSizeX     = groupSizeX;
    twParameters.workGroupCountX    = groupCountX;

    twParameters.workGroupSizeY     = groupSizeY;
    twParameters.workGroupCountY    = groupCountY;

    twParameters.globalOffsetX      = Parameters->xmin;
    twParameters.globalScaleX       = Parameters->xstep;

    twParameters.globalOffsetY      = Parameters->ymin;
    twParameters.globalScaleY       = Parameters->ystep;

    gcmONERROR(gcoHARDWARE_InvokeThreadWalkerVX(gcvNULL, &twParameters));

    /* Success. */
    status = gcvSTATUS_OK;
OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gceSTATUS
gcoVX_Upload(
    IN gctUINT32_PTR    Point,
    IN gctUINT32        Size,
    IN gctBOOL          Upload,
    OUT gctUINT32_PTR   Physical,
    OUT gctUINT32_PTR   Logical,
    OUT gcsSURF_NODE_PTR* Node
    )
{
     gceSTATUS          status = gcvSTATUS_OK;
     gctUINT32          physical = (gctUINT32)~0U;
     gctPOINTER         logical = gcvNULL;

     gcmHEADER_ARG("Point=%p Physical=%p", Point, Physical);


    gcmONERROR(gcoOS_Allocate(gcvNULL,
                              gcmSIZEOF(gcsSURF_NODE),
                              (gctPOINTER*)Node));

    gcmONERROR(gcsSURF_NODE_Construct(
        (*Node),
        Size,
        256,
        gcvSURF_ICACHE,
        gcvALLOC_FLAG_NONE,
        gcvPOOL_DEFAULT
        ));

    /* Lock the inst buffer. */
    gcmONERROR(gcoSURF_LockNode((*Node),
                                &physical,
                                &logical));

    if ((*Node) && (*Node)->pool != gcvPOOL_UNKNOWN)
    {

        *Physical   = physical;
        if(Logical != gcvNULL) *Logical  = *(gctUINT32_PTR)(&logical);

        if(Upload)
        {
            gcoOS_MemCopy((gctPOINTER)logical, Point, Size);
            gcmDUMP_BUFFER(gcvNULL, "memory", physical, (gctUINT32*)logical, 0, Size);
        }
        else
            gcoOS_ZeroMemory((gctPOINTER)logical, Size);

     }

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gceSTATUS
gcoVX_KernelConstruct(
    IN OUT gcoVX_Hardware_Context   *Context
    )
{

    gceSTATUS           status = gcvSTATUS_OK;
    gctUINT32           physical = (gctUINT32)~0U;

    gcmHEADER_ARG("Context=%p", Context);

    gcmONERROR(gcoHARDWAREVX_KenrelConstruct(Context));

    gcmONERROR(gcoVX_Upload((gctUINT32_PTR)Context->instructions->binarys, Context->instructions->count * 4 * 4, gcvTRUE, &physical, gcvNULL, &Context->node));

    gcmONERROR(gcoHARDWARE_LoadKernelVX(gcvNULL, physical, Context->instructions->count, Context->instructions->regs_count, Context->order));

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gceSTATUS
gcoVX_AllocateMemory(
    IN gctUINT32        Size,
    OUT gctUINT32_PTR   Logical,
    OUT gctUINT32_PTR   Physical,
    OUT gcsSURF_NODE_PTR* Node
    )
{
    gceSTATUS          status = gcvSTATUS_OK;
    gctUINT32          physical = (gctUINT32)~0U;
    gctPOINTER         logical = gcvNULL;
    gcsSURF_NODE_PTR   node = gcvNULL;

    gcmHEADER_ARG("Size=%d Logical=%p", Size, Logical);

    gcoVX_Initialize();

    gcmONERROR(gcoOS_Allocate(gcvNULL,
                             gcmSIZEOF(gcsSURF_NODE),
                             (gctPOINTER*)&node));

    gcmONERROR(gcsSURF_NODE_Construct(
        node,
        Size,
        256,
        gcvSURF_ICACHE,
        gcvALLOC_FLAG_NONE,
        gcvPOOL_DEFAULT
        ));

    /* Lock the inst buffer. */
    gcmONERROR(gcoSURF_LockNode(node,
                                &physical,
                                &logical));

    if (node && node->pool != gcvPOOL_UNKNOWN)
    {
        *Logical    = *(gctUINT32_PTR)(&logical);
        *Physical   = physical;
        *Node       = node;
    }

    return status;
OnError:
    /* Return the status. */
    if(node != gcvNULL)
    {
        gcoOS_Free(gcvNULL, node);
        node = gcvNULL;
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

gceSTATUS
gcoVX_FreeMemory(
    IN gcsSURF_NODE_PTR Node
    )
{

    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Node=0x%x", Node);

    /* Do we have a buffer allocated? */
    if (Node && Node->pool != gcvPOOL_UNKNOWN)
    {

        /* Unlock the buffer. */
        gcmONERROR(gcoHARDWARE_Unlock(Node,
                                      gcvSURF_ICACHE));

        /* Create an event to free the video memory. */
        gcmONERROR(gcsSURF_NODE_Destroy(Node));

        /* Free node. */
        gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Node));
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}
#endif /* gcdUSE_VX */
