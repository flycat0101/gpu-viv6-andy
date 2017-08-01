/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
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

    gcmHEADER_ARG("Hal=0x%x StateBufferSize=%zu StateBuffer=0x%x Hints=0x%x",
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

    gcmHEADER_ARG("Hardware=0x%x StateBufferSize=%zu StateBuffer=0x%x Hints=0x%x",
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

    gcmHEADER_ARG("Hal=0x%x, Address=%u Columns=%zu Rows=%zu Arrays=%zu IsRowMajor=%d "
                  "MatrixStride=%zu ArrayStride=%zu Values=%p Convert=%d Type=%d",
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

void
gcoSHADER_AllocateVidMem(
    gctPOINTER context,
    gceSURF_TYPE type,
    gctSTRING tag,
    gctSIZE_T size,
    gctUINT32 align,
    gctPOINTER *opaqueNode,
    gctPOINTER *memory,
    gctUINT32 *physical,
    gctPOINTER initialData,
    gctBOOL zeroMemory
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcsSURF_NODE_PTR    node = gcvNULL;
    gctPOINTER          pointer;
    gctPOINTER          logical = gcvNULL;

    gcmHEADER_ARG("context=%p type=%d ta%s size=%zu align=%u opaqueNode=%p"
                  "memory=%p physical=%p initialData=%p zeroMemory=%d",
                  context, type, tag, size, align, opaqueNode,
                  memory, physical, initialData, zeroMemory);

    gcmASSERT(physical);
    gcmASSERT(opaqueNode);
    if (size)
    {
        /* Allocate node. */
        gcmONERROR(gcoOS_Allocate(gcvNULL,
                                  gcmSIZEOF(gcsSURF_NODE),
                                  &pointer));

        node = pointer;

        gcmONERROR(gcsSURF_NODE_Construct(
            node,
            size,
            align,
            type,
            gcvALLOC_FLAG_NONE,
            gcvPOOL_DEFAULT
            ));

        /* Lock the inst buffer. */
        gcmONERROR(gcoSURF_LockNode(node,
                                    physical,
                                    &logical));
        gcmDUMP(gcvNULL, "#[info: video memory allocate for VSC %s", tag);

        if (initialData)
        {
#if gcdENDIAN_BIG
            gctSIZE_T i;
            gctUINT_PTR pDst = (gctUINT_PTR)logical;
            gctUINT_PTR pSrc = (gctUINT_PTR)initialData;

            gcmASSERT(size % 4 == 0);

            for (i = 0; i < size / 4; ++i)
            {
                gctUINT src = *pSrc++;
                *pDst++ = gcmBSWAP32(src);
            }
#else
            gcoOS_MemCopy(logical, initialData, size);
#endif
        }
        else if (zeroMemory)
        {
            gcoOS_ZeroMemory(logical, size);
        }

        gcmDUMP_BUFFER(gcvNULL, "memory", *physical, logical, 0, size);

        if (node->pool == gcvPOOL_VIRTUAL)
        {
            gcmONERROR(gcoOS_CacheFlush(gcvNULL, node->u.normal.node, logical, size));
        }
    }

    *opaqueNode = (gctPOINTER)node;

    if (memory)
    {
        *memory = logical;
    }

OnError:
    if (gcmIS_ERROR(status) && node != gcvNULL)
    {
        gcoOS_Free(gcvNULL, node);
    }
    gcmFOOTER();
    return;
}

void
gcoSHADER_FreeVidMem(
    gctPOINTER context,
    gceSURF_TYPE type,
    gctSTRING tag,
    gctPOINTER opaqueNode
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcsSURF_NODE_PTR    node = (gcsSURF_NODE_PTR)opaqueNode;

    gcmHEADER_ARG("context=%p type=%d tag=%s opaqueNode=%p", context, type, tag, opaqueNode);

    if (node && node->pool != gcvPOOL_UNKNOWN)
    {
        /* Borrow as index buffer. */
        gcmONERROR(gcoHARDWARE_Unlock(node,
                                      type));

        /* Create an event to free the video memory. */
        gcmONERROR(gcsSURF_NODE_Destroy(node));

        /* Free node. */
        gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, node));
    }

OnError:
    gcmFOOTER();
    return;
}

#endif /*gcdENABLE_3D*/

