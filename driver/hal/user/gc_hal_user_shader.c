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


/**
**
** Manage shader related stuff except compiler in HAL
**
*/

#include "gc_hal_user_precomp.h"

#if gcdENABLE_3D

#define _GC_OBJ_ZONE      gcvZONE_SHADER

gceSTATUS gcQueryShaderCompilerHwCfg(
    IN  gcoHAL Hal,
    OUT PVSC_HW_CONFIG pVscHwCfg)
{
    gceSTATUS status;

    gcmHEADER_ARG("Hal=0x%x pVscHwCfg=%d", Hal, pVscHwCfg);

    /* Call down to the hardware object. */
    status = gcoHARDWARE_QueryShaderCompilerHwCfg(gcvNULL, pVscHwCfg);

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**                                gcLoadShaders
********************************************************************************
**
**  Load a pre-compiled and pre-linked shader program into the hardware.
**
**  INPUT:
**
**      gcoHAL Hal
**          Pointer to a gcoHAL object.
**
**      gctSIZE_T StateBufferSize
**          The number of bytes in the 'StateBuffer'.
**
**      gctPOINTER StateBuffer
**          Pointer to the states that make up the shader program.
**
**      gcsHINT_PTR Hints
**          Pointer to a gcsHINT structure that contains information required
**          when loading the shader states.
**
**      gcePRIMITIVE PrimitiveType
**          Primitive type to be rendered.
*/
gceSTATUS
gcLoadShaders(
    IN gcoHAL Hal,
    IN gctSIZE_T StateBufferSize,
    IN gctPOINTER StateBuffer,
    IN gcsHINT_PTR Hints
    )
{
    gceSTATUS status;
    gcsPROGRAM_STATE programState;

    gcmHEADER_ARG("Hal=0x%x StateBufferSize=%d StateBuffer=0x%x Hints=0x%x",
        Hal, StateBufferSize, StateBuffer, Hints);

    programState.stateBuffer = StateBuffer;
    programState.stateBufferSize = (gctUINT)StateBufferSize;
    programState.hints = Hints;

    /* Call down to the hardware object. */
    status = gcoHARDWARE_LoadProgram(gcvNULL, Hints->stageBits, &programState);

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**                                gcLoadKernel
********************************************************************************
**
**  Load a pre-compiled and pre-linked kernel program into the hardware.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctSIZE_T StateBufferSize
**          The number of bytes in the 'StateBuffer'.
**
**      gctPOINTER StateBuffer
**          Pointer to the states that make up the shader program.
**
**      gcsHINT_PTR Hints
**          Pointer to a gcsHINT structure that contains information required
**          when loading the shader states.
*/
gceSTATUS
gcLoadKernel(
    IN gcoHARDWARE Hardware,
    IN gctSIZE_T StateBufferSize,
    IN gctPOINTER StateBuffer,
    IN gcsHINT_PTR Hints
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x StateBufferSize=%d StateBuffer=0x%x Hints=0x%x",
        Hardware, StateBufferSize, StateBuffer, Hints);

    /* Call down to the hardware object. */
    status = gcoHARDWARE_LoadKernel(Hardware, StateBufferSize, StateBuffer, Hints);

    gcmFOOTER();
    return status;
}

gceSTATUS
gcInvokeThreadWalker(
    IN gcoHARDWARE Hardware,
    IN gcsTHREAD_WALKER_INFO_PTR Info
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Info=0x%x", Hardware, Info);

    /* Call down to the hardware object. */
    status = gcoHARDWARE_InvokeThreadWalkerCL(Hardware, Info);

    gcmFOOTER();
    return status;
}

gceSTATUS
gcSHADER_SpecialHint(
    IN  gcePATCH_ID patchId,
    OUT gctUINT32_PTR hint
    )
{
    *hint = 0;

    /* Setting hint bit 0 means using "disable trilinear texture mode" */
    /* Setting hint bit 1 means using "don't always convert RT to 32bits" */
    switch (patchId)
    {
    case gcvPATCH_BM21:
    case gcvPATCH_BM3:
    case gcvPATCH_BMGUI:
    case gcvPATCH_MM06:
    case gcvPATCH_MM07:
    case gcvPATCH_QUADRANT:
    case gcvPATCH_ANTUTU:
    case gcvPATCH_ANTUTU4X:
    case gcvPATCH_ANTUTU5X:
    case gcvPATCH_SMARTBENCH:
    case gcvPATCH_JPCT:
    case gcvPATCH_NEOCORE:
    case gcvPATCH_RTESTVA:
    case gcvPATCH_GLBM11:
    case gcvPATCH_GLBM21:
    case gcvPATCH_GLBM25:
    case gcvPATCH_GLBM27:
    case gcvPATCH_GLBMGUI:
    case gcvPATCH_GFXBENCH:
    case gcvPATCH_BASEMARKX:
    case gcvPATCH_NENAMARK:
    case gcvPATCH_NENAMARK2:
        *hint |= (1 << 1) | (1 << 2);
        break;
    default:
        break;
    }

    /* The following benchmark list can't enable "disable trilinear texture mode" */
    switch (patchId)
    {
    case gcvPATCH_BM21:
    case gcvPATCH_MM06:
    case gcvPATCH_MM07:
    case gcvPATCH_GLBM11:
        *hint &= ~(1 << 1);
        break;
    default:
        break;
    }

     /* The following benchmark list can't enable "don't always convert RT to 32bits" */
    switch (patchId)
    {
    case gcvPATCH_NEOCORE:
    case gcvPATCH_NENAMARK:
    case gcvPATCH_NENAMARK2:
        *hint &= ~(1 << 2);
        break;
    default:
        break;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
gcoSHADER_ProgramUniform(
    IN gcoHAL Hal,
    IN gctUINT32 Address,
    IN gctUINT Columns,
    IN gctUINT Rows,
    IN gctCONST_POINTER Values,
    IN gctBOOL FixedPoint,
    IN gctBOOL ConvertToFloat,
    IN gcSHADER_KIND Type
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hal=0x%x, Address=%u Columns=%u Rows=%u Values=%p FixedPoint=%d Type=%d",
        Hal, Address, Columns, Rows, Values, FixedPoint, Type);

    status = gcoHARDWARE_ProgramUniform(gcvNULL, Address, Columns, Rows, Values,
                                        FixedPoint, ConvertToFloat, Type);

    gcmFOOTER();
    return status;
}

gceSTATUS
gcoSHADER_ProgramUniformEx(
    IN gcoHAL Hal,
    IN gctUINT32 Address,
    IN gctSIZE_T Columns,
    IN gctSIZE_T Rows,
    IN gctSIZE_T Arrays,
    IN gctBOOL   IsRowMajor,
    IN gctSIZE_T MatrixStride,
    IN gctSIZE_T ArrayStride,
    IN gctCONST_POINTER Values,
    IN gceUNIFORMCVT Convert,
    IN gcSHADER_KIND Type
    )
{
    gceSTATUS status;
    gctUINT32 columns, rows, arrays, matrixStride, arrayStride;

    gcmHEADER_ARG("Hal=0x%x, Address=%u Columns=%u Rows=%u Arrays=%u IsRowMajor=%d "
                  "MatrixStride=%d ArrayStride=%d Values=%p Convert=%d Type=%d",
                  Hal, Address, Columns, Rows, Arrays, IsRowMajor,
                  MatrixStride, ArrayStride, Values, Convert, Type);

    gcmSAFECASTSIZET(columns, Columns);
    gcmSAFECASTSIZET(rows, Rows);
    gcmSAFECASTSIZET(arrays, Arrays);
    gcmSAFECASTSIZET(matrixStride, MatrixStride);
    gcmSAFECASTSIZET(arrayStride, ArrayStride);
    status = gcoHARDWARE_ProgramUniformEx(gcvNULL, Address, columns, rows, arrays, IsRowMajor,
                                          matrixStride, arrayStride, Values, Convert, Type);

    gcmFOOTER();
    return status;
}

gceSTATUS
gcoSHADER_BindBufferBlock(
    IN gcoHAL Hal,
    IN gctUINT32 Address,
    IN gctUINT32 Base,
    IN gctSIZE_T Offset,
    IN gctSIZE_T Size,
    IN gcSHADER_KIND Type
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hal=0x%x Address=%u Base=%u Offset=%lu Size=%lu Type=%d",
        Hal, Address, Base, Offset, Size, Type);

    status = gcoHARDWARE_BindBufferBlock(gcvNULL, Address, Base, Offset, Size, Type);

    gcmFOOTER();
    return status;
}

gceSTATUS
gcCreateInstVidMem(
    IN gcoHAL Hal,
    IN gctPOINTER InstPtr,
    IN gctSIZE_T  instSize,
    OUT gcsSURF_NODE_PTR* Node,
    OUT gctUINT32* PhysicalAddr
)
{
    gctUINT32           physical = (gctUINT32)~0;
    gctPOINTER          logical = gcvNULL;
    gceSTATUS           status = gcvSTATUS_OK;
    gcsSURF_NODE_PTR    node = gcvNULL;
    gctPOINTER          pointer;

    gcmHEADER_ARG("Hal=0x%x, InstPtr=%u instSize=%u Node=%p PhysicalAddr=%p",
        Hal, InstPtr, instSize, Node, PhysicalAddr);

    if (instSize && InstPtr)
    {
        /* Allocate node. */
        gcmONERROR(gcoOS_Allocate(gcvNULL,
                                  gcmSIZEOF(gcsSURF_NODE),
                                  &pointer));

        node = pointer;

        gcmONERROR(gcsSURF_NODE_Construct(
            node,
            instSize,
            256,
            gcvSURF_ICACHE,
            gcvALLOC_FLAG_NONE,
            gcvPOOL_DEFAULT
            ));

        /* Lock the inst buffer. */
        gcmONERROR(gcoSURF_LockNode(node,
                                    &physical,
                                    &logical));

        gcoOS_MemCopy(logical, InstPtr, instSize);

        gcmDUMP_BUFFER(gcvNULL, "memory", physical, logical, 0, instSize);

        if (node->pool == gcvPOOL_VIRTUAL)
        {
            gcmONERROR(gcoOS_CacheFlush(gcvNULL, node->u.normal.node, logical, instSize));
        }
    }

    *Node = node;
    *PhysicalAddr = physical;

OnError:
    if (gcmIS_ERROR(status) && node != gcvNULL)
    {
        gcoOS_Free(gcvNULL, node);
    }
    gcmFOOTER();
    return status;
}

gceSTATUS
gcDestroyInstVidMem(
    IN gcoHAL Hal,
    IN gcsSURF_NODE_PTR Node
)
{
    gceSTATUS           status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hal=0x%x, Node=%p", Hal, Node);

    if (Node && Node->pool != gcvPOOL_UNKNOWN)
    {
        /* Borrow as index buffer. */
        gcmONERROR(gcoHARDWARE_Unlock(Node,
                                      gcvSURF_ICACHE));

        /* Create an event to free the video memory. */
        gcmONERROR(gcsSURF_NODE_Destroy(Node));

        /* Free node. */
        gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Node));
    }

OnError:
    gcmFOOTER();
    return status;
}

/* create video memory for register spill */
gceSTATUS
gcCreateSpillVidMem(
    IN gcoHAL Hal,
    IN gctSIZE_T  spillSize,
    OUT gcsSURF_NODE_PTR* Node,
    OUT gctUINT32* PhysicalAddr
)
{
    gctUINT32           physical = (gctUINT32)~0;
    gctPOINTER          logical = gcvNULL;
    gceSTATUS           status = gcvSTATUS_OK;
    gcsSURF_NODE_PTR    node = gcvNULL;
    gctPOINTER          pointer;

    gcmHEADER_ARG("Hal=0x%x, instSize=%u Node=%p PhysicalAddr=%p",
        Hal, spillSize, Node, PhysicalAddr);

    if (spillSize)
    {
        /* Allocate node. */
        gcmONERROR(gcoOS_Allocate(gcvNULL,
                                  gcmSIZEOF(gcsSURF_NODE),
                                  &pointer));

        node = pointer;

        gcmONERROR(gcsSURF_NODE_Construct(
            node,
            spillSize,
            64,
            gcvSURF_VERTEX,
            gcvALLOC_FLAG_NONE,
            gcvPOOL_DEFAULT
            ));

        /* Lock the inst buffer. */
        gcmONERROR(gcoSURF_LockNode(node,
                                    &physical,
                                    &logical));

        gcoOS_ZeroMemory(logical, 4);

        gcmDUMP_BUFFER(gcvNULL, "memory", physical, logical, 0, spillSize);

        if (node->pool == gcvPOOL_VIRTUAL)
        {
            gcmONERROR(gcoOS_CacheFlush(gcvNULL, node->u.normal.node, logical, spillSize));
        }
    }

    *Node = node;
    *PhysicalAddr = physical;

OnError:
    if (gcmIS_ERROR(status) && node != gcvNULL)
    {
        gcoOS_Free(gcvNULL, node);
    }
    gcmFOOTER();
    return status;
}
gceSTATUS
gcDestroySpillVidMem(
    IN gcoHAL Hal,
    IN gcsSURF_NODE_PTR Node
)
{
    gceSTATUS           status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hal=0x%x, Node=%p", Hal, Node);

    if (Node && Node->pool != gcvPOOL_UNKNOWN)
    {
        /* Borrow as index buffer. */
        gcmONERROR(gcoHARDWARE_Unlock(Node,
                                      gcvSURF_VERTEX));

        /* Create an event to free the video memory. */
        gcmONERROR(gcsSURF_NODE_Destroy(Node));

        /* Free node. */
        gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Node));
    }

OnError:
    gcmFOOTER();
    return status;
}


#endif /*gcdENABLE_3D*/

