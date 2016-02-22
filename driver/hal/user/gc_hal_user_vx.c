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


#include "gc_hal_user_precomp.h"
#if gcdUSE_VX
#include "gc_hal_vx.h"

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
gcoVX_SetFeatueCap(vx_evis_no_inst_s *evisNoInst)
{
    if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_EVIS_NO_ABSDIFF))
    {
        evisNoInst->noAbsDiff = gcvTRUE;
    }

    if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_EVIS_NO_BITREPLACE))
    {
        evisNoInst->noBitReplace = gcvTRUE;
    }

    if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_EVIS_NO_CORDIAC))
    {
        evisNoInst->noMagPhase = gcvTRUE;
    }

    if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_EVIS_NO_DP32))
    {
        evisNoInst->noDp32 = gcvTRUE;
        evisNoInst->clamp8Output = gcvTRUE;
    }

    if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_EVIS_NO_FILTER))
    {
        evisNoInst->noFilter = gcvTRUE;
    }

    if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_EVIS_NO_IADD))
    {
        evisNoInst->noIAdd = gcvTRUE;
    }

    if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_EVIS_NO_SELECTADD))
    {
        evisNoInst->noSelectAdd = gcvTRUE;
    }

    if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_EVIS_LERP_7OUTPUT))
    {
        evisNoInst->lerp7Output  = gcvTRUE;
    }

    if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_EVIS_ACCSQ_8OUTPUT))
    {
        evisNoInst->accsq8Output = gcvTRUE;
    }

    evisNoInst->isSet = gcvTRUE;

    return gcvSTATUS_OK;
}

gceSTATUS
gcoVX_Initialize(vx_evis_no_inst_s *evisNoInst)
{
    gceSTATUS status;
    gceAPI currentApi;
    gcsTLS_PTR __tls__;

    gcmHEADER();

    gcmONERROR(gcoOS_GetTLS(&__tls__));

    if (__tls__->currentHardware != gcvNULL)
    {
        if (evisNoInst != gcvNULL && evisNoInst->isSet == gcvFALSE)
        {
            gcoVX_SetFeatueCap(evisNoInst);
        }
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

    if (!gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_EVIS))
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    if (evisNoInst != gcvNULL)
    {
        gcoVX_SetFeatueCap(evisNoInst);
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
    gctUINT32                       shaderGroupSize;
    gctUINT                         maxComputeUnits;

    gcmHEADER_ARG("Parameters=%p", Parameters);

    if (!gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_EVIS))
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /* Calculate thread allocation. */
#if gcdVX_OPTIMIZER > 2
    if (Parameters->tiled)
    {
        if (Parameters->xoffset + Parameters->xcount <= Parameters->xmax)
        {
            xCount = gcmALIGN_NP2(Parameters->xcount, Parameters->xstep) / Parameters->xstep;
            yCount = gcmALIGN_NP2(Parameters->ycount, Parameters->ystep) / Parameters->ystep;
        }
        else
        {
            xCount = gcmALIGN_NP2(Parameters->xmax - Parameters->xoffset, Parameters->xstep) / Parameters->xstep;
            yCount = gcmALIGN_NP2(Parameters->ymax - Parameters->yoffset, Parameters->ystep) / Parameters->ystep;
        }
    }
    else
#endif
    {
        xCount = gcmALIGN_NP2(Parameters->xmax - Parameters->xmin, Parameters->xstep) / Parameters->xstep;
        yCount = gcmALIGN_NP2(Parameters->ymax - Parameters->ymin, Parameters->ystep) / Parameters->ystep;
    }

    /* Number of shader cores */
    gcmONERROR(
        gcoHARDWARE_QueryShaderCaps(gcvNULL,
                                    gcvNULL,
                                    gcvNULL,
                                    gcvNULL,
                                    gcvNULL,
                                    &maxComputeUnits,
                                    gcvNULL,
                                    gcvNULL,
                                    gcvNULL));

    shaderGroupSize = 4 * maxComputeUnits;

    /*  Compute work group size in each direction. Use 109 rather than 128. Some temp regs may be reserved if gpipe is running. */
#if gcdVX_OPTIMIZER > 1
    gcmASSERT(Parameters->instructions->regs_count);
    maxShaderGroups = 109/Parameters->instructions->regs_count;
#else
    gcmASSERT(Parameters->instructions.regs_count);
    maxShaderGroups = 109/Parameters->instructions.regs_count;
#endif
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

#if gcdVX_OPTIMIZER > 2
    if (Parameters->tiled)
    {
        twParameters.globalOffsetX  = Parameters->xmin + Parameters->xoffset;
        twParameters.globalScaleX   = Parameters->xstep;

        twParameters.globalOffsetY  = Parameters->ymin + Parameters->yoffset;
        twParameters.globalScaleY   = Parameters->ystep;
    }
    else
#endif
    {
    twParameters.globalOffsetX      = Parameters->xmin;
    twParameters.globalScaleX       = Parameters->xstep;

    twParameters.globalOffsetY      = Parameters->ymin;
    twParameters.globalScaleY       = Parameters->ystep;
    }

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

    gcmHEADER_ARG("Context=%p", Context);

    gcmONERROR(gcoHARDWAREVX_KenrelConstruct(Context));

#if !gcdVX_OPTIMIZER
    if (Context->node == gcvNULL)
    {
        gcmONERROR(gcoVX_Upload((gctUINT32_PTR)Context->instructions->binarys, Context->instructions->count * 4 * 4, gcvTRUE, &Context->nodePhysicalAdress, gcvNULL, &Context->node));
    }

    gcmONERROR(gcoHARDWARE_LoadKernelVX(gcvNULL, Context->nodePhysicalAdress, Context->instructions->count, Context->instructions->regs_count, Context->order));

#endif

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gceSTATUS
gcoVX_LockKernel(
    IN OUT gcoVX_Hardware_Context   *Context
    )
{
    gceSTATUS           status = gcvSTATUS_OK;

    gcmHEADER_ARG("Context=%p", Context);

#if gcdVX_OPTIMIZER
    gcmONERROR(gcoVX_Upload((gctUINT32_PTR)Context->instructions->binarys,
                            Context->instructions->count * 4 * 4,
                            gcvTRUE,
                            &Context->instructions->physical,
                            gcvNULL,
                            &Context->node));

OnError:
#endif
    gcmFOOTER_ARG("%d", status);
    return status;
}

gceSTATUS
gcoVX_BindKernel(
    IN OUT gcoVX_Hardware_Context   *Context
    )
{
    gceSTATUS           status = gcvSTATUS_OK;

    gcmHEADER_ARG("Context=%p", Context);

#if gcdVX_OPTIMIZER
#if gcdVX_OPTIMIZER > 1
    gcmONERROR(gcoHARDWARE_LoadKernelVX(gcvNULL,
                                        Context->instructions->physical,
                                        Context->instructions->count,
                                        Context->instructions->regs_count,
                                        &Context->order));
#else
    gcmONERROR(gcoHARDWARE_LoadKernelVX(gcvNULL,
                                        Context->instructions->physical,
                                        Context->instructions->count,
                                        Context->instructions->regs_count,
                                        Context->order));
#endif

OnError:
#endif
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

    gcoVX_Initialize(gcvNULL);

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

    gcmFOOTER_ARG("%d", status);
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

gceSTATUS
gcoVX_LoadKernelShader(
    IN gctSIZE_T StateBufferSize,
    IN gctPOINTER StateBuffer,
    IN gcsHINT_PTR Hints
    )
{

    gceSTATUS status;

    gcmHEADER_ARG("StateBufferSize=%d StateBuffer=0x%x Hints=0x%x",
                  StateBufferSize, StateBuffer, Hints);


    /* Set the hardware type. */
    gcmONERROR(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D));

    /* Switch to the 3D pipe. */
    gcmONERROR(gcoHARDWARE_SelectPipe(gcvNULL, gcvPIPE_3D, gcvNULL));

    if (!gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_PIPE_CL))
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /* Set rounding mode */
    if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_SHADER_HAS_RTNE))
    {
        gcmVERIFY_OK(gcoHARDWARE_SetRTNERounding(gcvNULL, gcvTRUE));
    }

    /* gcmONERROR(gcoCLHardware_Construct()) ; */

    /* Load kernel states. */
    status = gcLoadKernel(gcvNULL,
                          StateBufferSize,
                          StateBuffer,
                          Hints);

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoVX_InvokeThreadWalker(
    IN gcsTHREAD_WALKER_INFO_PTR Info
    )
{
    gceSTATUS status;
    gceAPI currentApi;

    gcmHEADER_ARG("Info=0x%x", Info);


    /* Get Current API. */
    gcmVERIFY_OK(gcoHARDWARE_GetAPI(gcvNULL, &currentApi, gcvNULL));

    if (currentApi != gcvAPI_OPENCL)
    {
        /* Set HAL API to OpenCL. */
        gcmVERIFY_OK(gcoHARDWARE_SetAPI(gcvNULL, gcvAPI_OPENCL));
    }

    /* Route to hardware. */
    status = gcoHARDWARE_InvokeThreadWalkerCL(gcvNULL, Info);

    if (currentApi != gcvAPI_OPENCL)
    {
        /* Restore HAL API. */
        gcmVERIFY_OK(gcoHARDWARE_SetAPI(gcvNULL, currentApi));
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoVX_InvokeKernelShader(
    IN gcSHADER            Kernel,
    IN gctUINT             WorkDim,
    IN size_t              GlobalWorkOffset[3],
    IN size_t              GlobalWorkScale[3],
    IN size_t              GlobalWorkSize[3],
    IN size_t              LocalWorkSize[3],
    IN gctUINT             ValueOrder
    )
{
    gcsTHREAD_WALKER_INFO   info;
    gceSTATUS               status = gcvSTATUS_OK;

    gcmHEADER_ARG("Kernel=0x%x WorkDim=%d", Kernel, WorkDim);

    gcoOS_ZeroMemory(&info, gcmSIZEOF(gcsTHREAD_WALKER_INFO));

    info.dimensions      = WorkDim;
    info.globalSizeX     = (gctUINT32)GlobalWorkSize[0];
    info.globalOffsetX   = (gctUINT32)GlobalWorkOffset[0];
    info.globalScaleX    = (gctUINT32)GlobalWorkScale[0];
    info.workGroupSizeX  = LocalWorkSize[0] ? (gctUINT32)LocalWorkSize[0] : 1;

    info.workGroupCountX = info.globalSizeX / info.workGroupSizeX;

    if (WorkDim > 1)
    {
        info.globalSizeY     = (gctUINT32)GlobalWorkSize[1];
        info.globalOffsetY   = (gctUINT32)GlobalWorkOffset[1];
        info.globalScaleY    = (gctUINT32)GlobalWorkScale[1];
        info.workGroupSizeY  = LocalWorkSize[1] ? (gctUINT32)LocalWorkSize[1] : 1;

        info.workGroupCountY = info.globalSizeY / info.workGroupSizeY;

    }
    if (WorkDim > 2)
    {
        info.globalSizeZ     = (gctUINT32)GlobalWorkSize[2];
        info.globalOffsetZ   = (gctUINT32)GlobalWorkOffset[2];
        info.globalScaleZ    = (gctUINT32)GlobalWorkScale[2];
        info.workGroupSizeZ  = LocalWorkSize[2] ? (gctUINT32)LocalWorkSize[2] : 1;
        info.workGroupCountZ = info.globalSizeZ / info.workGroupSizeZ;
    }

    /* TODO - Handle GLW order and in-use. */
    info.traverseOrder    = 0;  /* XYZ */
    info.enableSwathX     = 0;
    info.enableSwathY     = 0;
    info.enableSwathZ     = 0;
    info.swathSizeX       = 0;
    info.swathSizeY       = 0;
    info.swathSizeZ       = 0;
    info.valueOrder       = ValueOrder;

    gcmONERROR(gcoVX_InvokeThreadWalker(&info));

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gceSTATUS
gcoVX_Flush(
    IN gctBOOL      Stall
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Stall=%d", Stall);

    /* Flush the current pipe. */
    gcmONERROR(gcoHARDWARE_FlushPipe(gcvNULL, gcvNULL));

    /* Commit the command buffer to hardware. */
    gcmONERROR(gcoHARDWARE_Commit(gcvNULL));

    if (Stall)
    {
        /* Stall the hardware. */
        gcmONERROR(gcoHARDWARE_Stall(gcvNULL));
    }

OnError:

    gcmFOOTER();
    return status;
}

gceSTATUS
gcoVX_AllocateMemoryEx(
    IN OUT gctUINT *        Bytes,
    OUT gctPHYS_ADDR *      Physical,
    OUT gctPOINTER *        Logical,
    OUT gcsSURF_NODE_PTR *  Node
    )
{

    gceSTATUS status;
    gctUINT bytes;
    gcsSURF_NODE_PTR node = gcvNULL;

    gcmHEADER_ARG("*Bytes=%lu", *Bytes);


    /* Allocate extra 64 bytes to avoid cache overflow */
    bytes = gcmALIGN(*Bytes, 64);

    {
    gctPOINTER pointer = gcvNULL;

    /* Allocate node. */
    gcmONERROR(gcoOS_Allocate(gcvNULL,
                              gcmSIZEOF(gcsSURF_NODE),
                              &pointer));

    node = pointer;

    gcmONERROR(gcsSURF_NODE_Construct(
        node,
        bytes,
        64,
        gcvSURF_INDEX,
        gcvALLOC_FLAG_NONE,
        gcvPOOL_DEFAULT
        ));

    /* Lock the cl buffer. */
    gcmONERROR(gcoHARDWARE_Lock(node,
                                (gctUINT32 *) Physical,
                                Logical));

    /* Return allocated number of bytes. */
    *Bytes = bytes;

    /* Return node. */
    *Node = node;
    }


    /* Success. */
    gcmFOOTER_ARG("*Bytes=%lu *Physical=0x%x *Logical=0x%x *Node=0x%x",
                  *Bytes, *Physical, *Logical, *Node);
    return gcvSTATUS_OK;

OnError:

    /* Return the status. */
    if(node != gcvNULL)
    {
        gcoOS_Free(gcvNULL, node);
        node = gcvNULL;
    }
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoVX_FreeMemoryEx(
    IN gctPHYS_ADDR         Physical,
    IN gctPOINTER           Logical,
    IN gctUINT              Bytes,
    IN gcsSURF_NODE_PTR     Node
    )
{
    /*gcmSWITCHHARDWAREVAR*/
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Physical=0x%x Logical=0x%x Bytes=%u Node=0x%x",
                  Physical, Logical, Bytes, Node);


    /* Do we have a buffer allocated? */
    if (Node && Node->pool != gcvPOOL_UNKNOWN)
    {
        /* Unlock the index buffer. */
        gcmONERROR(gcoHARDWARE_Unlock(Node,
                                      gcvSURF_INDEX));

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
