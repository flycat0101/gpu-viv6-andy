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


/**
**  @file
**  gcoSURF object for user HAL layers.
**
*/

#include "gc_hal_user_precomp.h"

#define _GC_OBJ_ZONE    gcvZONE_SURFACE

/******************************************************************************\
**************************** gcoSURF API Support Code **************************
\******************************************************************************/

const gcsSAMPLES g_sampleInfos[] =
    {
        {1, 1, 1},  /* 0x msaa */
        {1, 1, 1},  /* 1x msaa */
        {2, 1, 2},  /* 2x msaa */
        {0, 0, 0},  /* 3x msaa: invalid */
        {2, 2, 4},  /* 4x msaa */
    };

#if gcdENABLE_3D
/*******************************************************************************
**
**  _LockAuxiliary
**
**  Lock auxiliary node (ts/hz/hz ts...) and make sure the lockCount same
**  as main node.
**
*/
static gceSTATUS
_LockAuxiliaryNode(
    IN gcsSURF_NODE_PTR Node,
    IN gcsSURF_NODE_PTR Reference
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT i,j;
    gceHARDWARE_TYPE type;

    gcmHEADER_ARG("Node=0x%x Reference=0x%x", Node, Reference);

    gcmGETCURRENTHARDWARE(type);

    for (i = 0 ; i < gcvHARDWARE_NUM_TYPES; i++)
    {
        for (j = 0; j < gcvENGINE_COUNT; j++)
        {
            gcmASSERT(Node->lockCounts[i][j] <= Reference->lockCounts[i][j]);

            while (Node->lockCounts[i][j] < Reference->lockCounts[i][j])
            {
                gcoHAL_SetHardwareType(gcvNULL, (gceHARDWARE_TYPE)i);

                gcmONERROR(gcoHARDWARE_LockEx(Node, (gceENGINE)j, gcvNULL, gcvNULL));
            }
        }
    }

OnError:
    /* Restore type. */
    gcoHAL_SetHardwareType(gcvNULL, type);

    /* Return status. */
    gcmFOOTER();
    return status;
}
/*******************************************************************************
**
**  gcoSURF_LockTileStatus
**
**  Locked tile status buffer of surface
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**
**  OUTPUT:
**
*/
gceSTATUS
gcoSURF_LockTileStatus(
    IN gcoSURF Surface
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 address;

    gcmHEADER_ARG("Surface=0x%x", Surface);

    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    if (Surface->info.tileStatusNode.pool != gcvPOOL_UNKNOWN)
    {
        /* Lock the tile status buffer. */
        gcmONERROR(
            _LockAuxiliaryNode(&Surface->info.tileStatusNode,
                               &Surface->info.node));

        gcmGETHARDWAREADDRESS(Surface->info.tileStatusNode, address);

        gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                      "Locked tile status 0x%x: physical=0x%08X logical=0x%x "
                      "lockedCount=%d",
                      &Surface->info.tileStatusNode,
                      address,
                      Surface->info.tileStatusNode.logical,
                      Surface->info.tileStatusNode.lockCount);

        /* Only 1 address. */
        Surface->info.tileStatusNode.count = 1;

        /* Check if this is the first lock. */
        if (Surface->info.tileStatusFirstLock)
        {
            /* Fill the tile status memory with the filler. */
            gcoOS_MemFill(Surface->info.tileStatusNode.logical,
                          (gctUINT8) Surface->info.tileStatusFiller,
                          Surface->info.tileStatusNode.size);

            /* Flush the node from cache. */
            gcmONERROR(
                gcoSURF_NODE_Cache(&Surface->info.tileStatusNode,
                                 Surface->info.tileStatusNode.logical,
                                 Surface->info.tileStatusNode.size,
                                 gcvCACHE_CLEAN));

            /* Dump the memory write. */
            gcmDUMP_BUFFER(gcvNULL,
                           "memory",
                           address,
                           Surface->info.tileStatusNode.logical,
                           0,
                           Surface->info.tileStatusNode.size);

#if gcdDUMP
            if (Surface->info.tileStatusFiller == 0x0)
            {
                gcmGETHARDWAREADDRESS(Surface->info.node, address);
                gcmDUMP_BUFFER(gcvNULL,
                               "memory",
                               address,
                               Surface->info.node.logical,
                               0,
                               Surface->info.node.size);
            }
#endif

            /* No longer first lock. */
            Surface->info.tileStatusFirstLock = gcvFALSE;
        }
    }

    /* Lock the hierarchical Z tile status buffer. */
    if (Surface->info.hzTileStatusNode.pool != gcvPOOL_UNKNOWN)
    {
       /* Lock the tile status buffer. */
        gcmONERROR(
            _LockAuxiliaryNode(&Surface->info.hzTileStatusNode,
                               &Surface->info.node));

        gcmGETHARDWAREADDRESS(Surface->info.hzTileStatusNode, address);

        gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                      "Locked HZ tile status 0x%x: physical=0x%08X logical=0x%x "
                      "lockedCount=%d",
                      &Surface->info.hzTileStatusNode,
                      address,
                      Surface->info.hzTileStatusNode.logical,
                      Surface->info.hzTileStatusNode.lockCount);

        /* Only 1 address. */
        Surface->info.hzTileStatusNode.count = 1;

        /* Check if this is the first lock. */
        if (Surface->info.hzTileStatusFirstLock)
        {
            /* Fill the tile status memory with the filler. */
            gcoOS_MemFill(Surface->info.hzTileStatusNode.logical,
                          (gctUINT8) Surface->info.hzTileStatusFiller,
                          Surface->info.hzTileStatusNode.size);

            /* Flush the node from cache. */
            gcmONERROR(
                gcoSURF_NODE_Cache(&Surface->info.hzTileStatusNode,
                                 Surface->info.hzTileStatusNode.logical,
                                 Surface->info.hzTileStatusNode.size,
                                 gcvCACHE_CLEAN));

            /* Dump the memory write. */
            gcmDUMP_BUFFER(gcvNULL,
                           "memory",
                           address,
                           Surface->info.hzTileStatusNode.logical,
                           0,
                           Surface->info.hzTileStatusNode.size);

            /* No longer first lock. */
            Surface->info.hzTileStatusFirstLock = gcvFALSE;
        }
    }
OnError:
    gcmFOOTER_NO();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_LockHzBuffer
**
**  Locked HZ buffer buffer of surface
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**
**  OUTPUT:
**
*/
gceSTATUS
gcoSURF_LockHzBuffer(
    IN gcoSURF Surface
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 address;

    gcmHEADER_ARG("Surface=0x%x", Surface);

    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    if (Surface->info.hzNode.pool != gcvPOOL_UNKNOWN)
    {
        gcmONERROR(
            _LockAuxiliaryNode(&Surface->info.hzNode, &Surface->info.node));

        gcmGETHARDWAREADDRESS(Surface->info.hzNode, address);

        gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                      "Locked HZ surface 0x%x: physical=0x%08X logical=0x%x "
                      "lockCount=%d",
                      &Surface->info.hzNode,
                      address,
                      Surface->info.hzNode.logical,
                      Surface->info.hzNode.lockCount);

        /* Only 1 address. */
        Surface->info.hzNode.count = 1;
    }
OnError:
    gcmFOOTER_NO();
    return status;

}

/*******************************************************************************
**
**  gcoSURF_AllocateTileStatus
**
**  Allocate tile status buffer for surface
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**
**  OUTPUT:
**
*/
gceSTATUS
gcoSURF_AllocateTileStatus(
    IN gcoSURF Surface
    )
{
    gceSTATUS status;
    gctSIZE_T bytes;
    gctUINT alignment;
    gctBOOL tileStatusInVirtual;

    gcmHEADER_ARG("Surface=0x%x", Surface);

    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    if (Surface->info.tileStatusNode.pool != gcvPOOL_UNKNOWN)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    /* No tile status buffer allocated. */
    Surface->info.tileStatusNode.pool             = gcvPOOL_UNKNOWN;
    Surface->info.hzTileStatusNode.pool           = gcvPOOL_UNKNOWN;

    /* Set tile status disabled at the beginging to be consistent with POOL value */
    Surface->info.tileStatusDisabled = gcvTRUE;
    Surface->info.dirty = gcvFALSE;

    tileStatusInVirtual = gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_MC20);

    /*   Verify if the type requires a tile status buffer:
    ** - do not turn on fast clear if the surface is virtual;
    */
    if ((Surface->info.node.pool == gcvPOOL_VIRTUAL) && !tileStatusInVirtual)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    if ((Surface->info.type != gcvSURF_RENDER_TARGET) &&
        (Surface->info.type != gcvSURF_DEPTH) &&
        ((Surface->info.hints & gcvSURF_DEC) != gcvSURF_DEC))
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    if (Surface->info.hints & gcvSURF_NO_TILE_STATUS)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    if ((Surface->info.formatInfo.fakedFormat &&
         !Surface->info.paddingFormat
        ) ||
        ((Surface->info.bitsPerPixel > 32) &&
         (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_64BPP_HW_CLEAR_SUPPORT) == gcvFALSE)
        )
       )
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    /* Can't support multi-slice surface*/
    if (Surface->info.requestD > 1)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

#if gcdENABLE_VG
    {
        gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;
        gcmGETCURRENTHARDWARE(currentType);

        gcmASSERT(currentType != gcvHARDWARE_VG);
    }
#endif

    /* Set default fill color. */
    switch (Surface->info.format)
    {
    case gcvSURF_D16:
        Surface->info.clearValue[0] =
        Surface->info.fcValueUpper  =
        Surface->info.fcValue       = 0xFFFFFFFF;
        gcmONERROR(gcoHARDWARE_HzClearValueControl(Surface->info.format,
                                                   Surface->info.fcValue,
                                                   &Surface->info.fcValueHz,
                                                   gcvNULL));
        break;

    case gcvSURF_D24X8:
    case gcvSURF_D24S8:
        Surface->info.clearValue[0] =
        Surface->info.fcValueUpper  =
        Surface->info.fcValue       = 0xFFFFFF00;
        gcmONERROR(gcoHARDWARE_HzClearValueControl(Surface->info.format,
                                                   Surface->info.fcValue,
                                                   &Surface->info.fcValueHz,
                                                   gcvNULL));
        break;

    case gcvSURF_S8:
    case gcvSURF_X24S8:
        Surface->info.clearValue[0] =
        Surface->info.fcValueUpper  =
        Surface->info.fcValue       = 0x00000000;
        break;

    case gcvSURF_R8_1_X8R8G8B8:
    case gcvSURF_G8R8_1_X8R8G8B8:
        Surface->info.clearValue[0]      =
        Surface->info.clearValueUpper[0] =
        Surface->info.fcValue            =
        Surface->info.fcValueUpper       = 0xFF000000;
        break;

    default:
        Surface->info.clearValue[0]      =
        Surface->info.clearValueUpper[0] =
        Surface->info.fcValue            =
        Surface->info.fcValueUpper       = 0x00000000;
        break;
    }

    /* Query the linear size for the tile status buffer. */
    status = gcoHARDWARE_QueryTileStatus(gcvNULL,
                                         Surface->info.alignedW,
                                         Surface->info.alignedH,
                                         Surface->info.size,
                                         Surface->info.edgeAA,
                                         &bytes,
                                         &alignment,
                                         &Surface->info.tileStatusFiller);

    /* Tile status supported? */
    if ((status == gcvSTATUS_NOT_SUPPORTED) || (0 == bytes))
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }
    else if (gcmIS_ERROR(status))
    {
        gcmFOOTER_NO();
        return status;
    }

    /* Copy filler. */
    Surface->info.hzTileStatusFiller = Surface->info.tileStatusFiller;

    if (!(Surface->info.hints & gcvSURF_NO_VIDMEM))
    {
        /* Allocate the tile status buffer. */
        status = gcsSURF_NODE_Construct(
            &Surface->info.tileStatusNode,
            bytes,
            alignment,
            gcvSURF_TILE_STATUS,
            gcvALLOC_FLAG_NONE,
            gcvPOOL_DEFAULT
            );

        if (gcmIS_ERROR(status))
        {
            /* Commit any command buffer and wait for idle hardware. */
            status = gcoHAL_Commit(gcvNULL, gcvTRUE);

            if (gcmIS_SUCCESS(status))
            {
                /* Try allocating again. */
                status = gcsSURF_NODE_Construct(
                    &Surface->info.tileStatusNode,
                    bytes,
                    alignment,
                    gcvSURF_TILE_STATUS,
                    gcvALLOC_FLAG_NONE,
                    gcvPOOL_DEFAULT
                    );
            }
        }
    }

    if (gcmIS_SUCCESS(status))
    {
        /* When allocate successfully, set tile status is enabled for this surface by default.
        ** Logically, we should disable tile status buffer initially.
        ** But for MSAA, we always enable FC, otherwise it will hang up on hw.
        ** So for non-cleared we also need enable FC by default.
        */
        if (Surface->info.TSDirty)
        {
            Surface->info.tileStatusFiller = 0x0;
            Surface->info.TSDirty = gcvFALSE;
        }

        Surface->info.tileStatusDisabled = gcvFALSE;

        /* Only set garbagePadded=0 if by default cleared tile status. */
        if (Surface->info.paddingFormat)
        {
            Surface->info.garbagePadded = gcvFALSE;
        }

        /*
        ** Get surface compression setting.
        */
        gcoHARDWARE_QueryCompression(gcvNULL,
                                     &Surface->info,
                                     &Surface->info.compressed,
                                     &Surface->info.compressFormat,
                                     &Surface->info.compressDecFormat);
        Surface->info.tileStatusFirstLock = gcvTRUE;


        gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                      "Allocated tile status 0x%x: pool=%d size=%u",
                      &Surface->info.tileStatusNode,
                      Surface->info.tileStatusNode.pool,
                      Surface->info.tileStatusNode.size);

        /* Allocate tile status for hierarchical Z buffer. */
        if (Surface->info.hzNode.pool != gcvPOOL_UNKNOWN)
        {
            /* Query the linear size for the tile status buffer. */
            status = gcoHARDWARE_QueryTileStatus(gcvNULL,
                                                 0,
                                                 0,
                                                 Surface->info.hzNode.size,
                                                 gcvFALSE,
                                                 &bytes,
                                                 &alignment,
                                                 gcvNULL);

            /* Tile status supported? */
            if (status == gcvSTATUS_NOT_SUPPORTED)
            {
                return gcvSTATUS_OK;
            }

            if (!(Surface->info.hints & gcvSURF_NO_VIDMEM))
            {
                status = gcsSURF_NODE_Construct(
                             &Surface->info.hzTileStatusNode,
                             bytes,
                             alignment,
                             gcvSURF_TILE_STATUS,
                             gcvALLOC_FLAG_NONE,
                             gcvPOOL_DEFAULT
                             );
            }

            if (gcmIS_SUCCESS(status))
            {
                Surface->info.hzTileStatusFirstLock = gcvTRUE;

                gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                              "Allocated HZ tile status 0x%x: pool=%d size=%u",
                              &Surface->info.hzTileStatusNode,
                              Surface->info.hzTileStatusNode.pool,
                              Surface->info.hzTileStatusNode.size);
            }
        }
    }

OnError:
    gcmFOOTER_NO();
    /* Return the status. */
    return status;
}

/* Append tile status for user pool render target. */
gceSTATUS
gcoSURF_AppendTileStatus(
    IN gcoSURF Surface
    )
{
#if gcdENABLE_3D
    gceSTATUS status;
    gcmHEADER_ARG("Surface=0x%x", Surface);

    if ((Surface->info.node.pool != gcvPOOL_USER) ||
        (Surface->info.tileStatusNode.pool != gcvPOOL_UNKNOWN))
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    if ((Surface->info.type != gcvSURF_RENDER_TARGET) &&
        (Surface->info.type != gcvSURF_DEPTH))
    {
        /* Only render target and depth can not tile status. */
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Try to allocate tile status buffer. */
    gcmONERROR(gcoSURF_AllocateTileStatus(Surface));

    /* Lock Tile status buffer */
    gcmONERROR(gcoSURF_LockTileStatus(Surface));

OnError:
    gcmFOOTER();
    return status;
#else

    return gcvSTATUS_OK;
#endif
}


/*******************************************************************************
**
**  gcoSURF_AllocateHzBuffer
**
**  Allocate HZ buffer for surface
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**
**  OUTPUT:
**
*/
gceSTATUS
gcoSURF_AllocateHzBuffer(
    IN gcoSURF Surface
    )
{
    gcePOOL  pool;
    gceSTATUS status;
    gcmHEADER_ARG("Surface=0x%x", Surface);

    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    if (Surface->info.hzNode.pool != gcvPOOL_UNKNOWN)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    pool = Surface->info.node.pool;

    /* No Hierarchical Z buffer allocated. */
    Surface->info.hzNode.pool = gcvPOOL_UNKNOWN;

    Surface->info.hzDisabled = gcvTRUE;

    /* Can't support multi-slice surface*/
    if (Surface->info.requestD > 1)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    /* Check if this is a depth buffer and the GPU supports hierarchical Z. */
    if ((Surface->info.type == gcvSURF_DEPTH) &&
        (Surface->info.format != gcvSURF_S8) &&
        (Surface->info.format != gcvSURF_X24S8) &&
        (pool != gcvPOOL_USER) &&
        ((Surface->info.hints & gcvSURF_NO_VIDMEM) == 0) &&
        (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_HZ) == gcvSTATUS_TRUE))
    {
        gctSIZE_T bytes;
        gctUINT32 sizeAlignment = 32 * 32 * 4;

        gctSIZE_T unalignedBytes = (Surface->info.size + 63)/64 * 4;

        /* Compute the hierarchical Z buffer size.  Allocate enough for
        ** 16-bit min/max values. */
        if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NEW_RA) == gcvSTATUS_TRUE)
        {
            bytes = gcmALIGN(unalignedBytes / 2, sizeAlignment);
        }
        else
        {
            bytes = gcmALIGN(unalignedBytes, sizeAlignment);
        }

        /* Allocate the hierarchical Z buffer. */
        status = gcsSURF_NODE_Construct(
                    &Surface->info.hzNode,
                    bytes,
                    64,
                    gcvSURF_HIERARCHICAL_DEPTH,
                    gcvALLOC_FLAG_NONE,
                    pool
                    );

        if (gcmIS_SUCCESS(status))
        {
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                          "Allocated HZ surface 0x%x: pool=%d size=%u",
                          &Surface->info.hzNode,
                          Surface->info.hzNode.pool,
                          Surface->info.hzNode.size);
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

}
#endif

static gceSTATUS
_Lock(
    IN gcoSURF Surface
    )
{
    gceSTATUS status;

    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;
    gctUINT32 address;

    gcmGETCURRENTHARDWARE(currentType);

#if gcdENABLE_VG
    if (currentType == gcvHARDWARE_VG)
    {
        gcmONERROR(
            gcoVGHARDWARE_Lock(gcvNULL,
                               &Surface->info.node,
                               &address,
                               gcvNULL));

        if (Surface->info.node2.pool != gcvPOOL_UNKNOWN)
        {
            gcmONERROR(
                gcoVGHARDWARE_Lock(gcvNULL,
                                   &Surface->info.node2,
                                   gcvNULL,
                                   gcvNULL));
        }

        if (Surface->info.node3.pool != gcvPOOL_UNKNOWN)
        {
            gcmONERROR(
                gcoVGHARDWARE_Lock(gcvNULL,
                                   &Surface->info.node3,
                                   gcvNULL,
                                   gcvNULL));
        }
    }
    else
#endif
    {
        /* Lock the video memory. */
        gcmONERROR(
            gcoHARDWARE_Lock(&Surface->info.node,
                             &address,
                             gcvNULL));

        /* Lock for mulit-buffer surfaces. */
        if (Surface->info.node2.pool != gcvPOOL_UNKNOWN)
        {
            /* Lock video memory for node 2. */
            gcmONERROR(
                gcoHARDWARE_Lock(&Surface->info.node2,
                                 gcvNULL,
                                 gcvNULL));
        }

        if (Surface->info.node3.pool != gcvPOOL_UNKNOWN)
        {
            /* Lock video memory for node 3. */
            gcmONERROR(
                gcoHARDWARE_Lock(&Surface->info.node3,
                                 gcvNULL,
                                 gcvNULL));
        }
    }

    Surface->info.node.logicalBottom = Surface->info.node.logical + Surface->info.bottomBufferOffset;

    Surface->info.node.hardwareAddressesBottom[currentType] =
    Surface->info.node.physicalBottom =
        address + Surface->info.bottomBufferOffset;

    gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                  "Locked surface 0x%x: physical=0x%08X logical=0x%x lockCount=%d",
                  &Surface->info.node,
                  address,
                  Surface->info.node.logical,
                  Surface->info.node.lockCount);

#if gcdENABLE_3D
    /* Lock the hierarchical Z node. */
    gcmONERROR(gcoSURF_LockHzBuffer(Surface));

    /* Lock the tile status buffer and hierarchical Z tile status buffer. */
    gcmONERROR(gcoSURF_LockTileStatus(Surface));

#endif /* gcdENABLE_3D */
    /* Success. */
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    return status;
}

static gceSTATUS
_Unlock(
    IN gcoSURF Surface
    )
{
    gceSTATUS status;

#if gcdENABLE_VG
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;

    gcmGETCURRENTHARDWARE(currentType);

    if (currentType == gcvHARDWARE_VG)
    {
        /* Unlock the surface. */
        gcmONERROR(
            gcoVGHARDWARE_Unlock(gcvNULL,
                                &Surface->info.node,
                                Surface->info.type));

        if (Surface->info.node2.pool != gcvPOOL_UNKNOWN)
        {
            gcmONERROR(
                gcoVGHARDWARE_Unlock(gcvNULL,
                                     &Surface->info.node2,
                                     Surface->info.type));
        }

        if (Surface->info.node3.pool != gcvPOOL_UNKNOWN)
        {
            gcmONERROR(
                gcoVGHARDWARE_Unlock(gcvNULL,
                                     &Surface->info.node3,
                                     Surface->info.type));
        }
    }
    else
#endif
    {
        /* Unlock the surface. */
        gcmONERROR(
            gcoHARDWARE_Unlock(&Surface->info.node, Surface->info.type));

        if (Surface->info.node2.pool != gcvPOOL_UNKNOWN)
        {
            gcmONERROR(
                gcoHARDWARE_Unlock(&Surface->info.node2, Surface->info.type));
        }

        if (Surface->info.node3.pool != gcvPOOL_UNKNOWN)
        {
            gcmONERROR(
                gcoHARDWARE_Unlock(&Surface->info.node3, Surface->info.type));
        }
    }

    gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                  "Unlocked surface 0x%x: lockCount=%d",
                  &Surface->info.node,
                  Surface->info.node.lockCount);

#if gcdENABLE_3D
    /* Unlock the hierarchical Z buffer. */
    if (Surface->info.hzNode.pool != gcvPOOL_UNKNOWN)
    {
        gcmONERROR(
            gcoHARDWARE_Unlock(&Surface->info.hzNode,
                               gcvSURF_HIERARCHICAL_DEPTH));

        gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                      "Unlocked HZ surface 0x%x: lockCount=%d",
                      &Surface->info.hzNode,
                      Surface->info.hzNode.lockCount);
    }

    /* Unlock the tile status buffer. */
    if (Surface->info.tileStatusNode.pool != gcvPOOL_UNKNOWN)
    {
        gcmONERROR(
            gcoHARDWARE_Unlock(&Surface->info.tileStatusNode,
                               gcvSURF_TILE_STATUS));

        gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                      "Unlocked tile status 0x%x: lockCount=%d",
                      &Surface->info.hzNode,
                      Surface->info.hzNode.lockCount);
    }

    /* Unlock the hierarchical tile status buffer. */
    if (Surface->info.hzTileStatusNode.pool != gcvPOOL_UNKNOWN)
    {
        gcmONERROR(
            gcoHARDWARE_Unlock(&Surface->info.hzTileStatusNode,
                               gcvSURF_TILE_STATUS));

        gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                      "Unlocked HZ tile status 0x%x: lockCount=%d",
                      &Surface->info.hzNode,
                      Surface->info.hzNode.lockCount);
    }
#endif /* gcdENABLE_3D */

    /* Success. */
    return gcvSTATUS_OK;

OnError:
    /* Return the error. */
    return status;
}

static gceSTATUS
_FreeSurface(
    IN gcoSURF Surface
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x", Surface);

    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* We only manage valid and non-user pools. */
    if (Surface->info.node.pool != gcvPOOL_UNKNOWN)
    {
        /* Unlock the video memory. */
        gcmONERROR(_Unlock(Surface));

        if (Surface->info.node.u.normal.node != 0)
        {
#if gcdENABLE_VG
            gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;
            gcmGETCURRENTHARDWARE(currentType);

            if (currentType == gcvHARDWARE_VG)
            {
                /* Free the video memory. */
                gcmONERROR(
                    gcoVGHARDWARE_ScheduleVideoMemory(gcvNULL, Surface->info.node.u.normal.node, gcvFALSE));
            }
            else
#endif
            {
                /* Free the video memory. */
                gcmONERROR(gcsSURF_NODE_Destroy(&Surface->info.node));
            }
        }

        /* Mark the memory as freed. */
        Surface->info.node.pool = gcvPOOL_UNKNOWN;

        gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                      "Freed surface 0x%x",
                      &Surface->info.node);
    }

#if gcdENABLE_3D
    if (Surface->info.hzNode.pool != gcvPOOL_UNKNOWN)
    {
        /* Free the hierarchical Z video memory. */
        gcmONERROR(
            gcsSURF_NODE_Destroy(&Surface->info.hzNode));

        gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                      "Freed HZ surface 0x%x",
                      &Surface->info.hzNode);
    }

    if (Surface->info.tileStatusNode.pool != gcvPOOL_UNKNOWN)
    {
        /* Free the tile status memory. */
        gcmONERROR(
            gcsSURF_NODE_Destroy(&Surface->info.tileStatusNode));

        gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                      "Freed tile status 0x%x",
                      &Surface->info.tileStatusNode);
    }

    if (Surface->info.hzTileStatusNode.pool != gcvPOOL_UNKNOWN)
    {
        /* Free the hierarchical Z tile status memory. */
        gcmONERROR(
            gcsSURF_NODE_Destroy(&Surface->info.hzTileStatusNode));

        gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                      "Freed HZ tile status 0x%x",
                      &Surface->info.hzTileStatusNode);
    }
#endif /* gcdENABLE_3D */

    if (Surface->info.shBuf != gcvNULL)
    {
        /* Destroy shared buffer. */
        gcmVERIFY_OK(
            gcoHAL_DestroyShBuffer(Surface->info.shBuf));

        /* Mark it as freed. */
        Surface->info.shBuf = gcvNULL;
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

#if gcdENABLE_3D
#if gcdENABLE_BUFFER_ALIGNMENT

#if gcdENABLE_BANK_ALIGNMENT

#if !gcdBANK_BIT_START
#error gcdBANK_BIT_START not defined.
#endif

#if !gcdBANK_BIT_END
#error gcdBANK_BIT_END not defined.
#endif
#endif

/*******************************************************************************
**  _GetBankOffsetBytes
**
**  Return the bytes needed to offset sub-buffers to different banks.
**
**  ARGUMENTS:
**
**      gceSURF_TYPE Type
**          Type of buffer.
**
**      gctUINT32 TopBufferSize
**          Size of the top buffer, need\ed to compute offset of the second buffer.
**
**  OUTPUT:
**
**      gctUINT32_PTR Bytes
**          Pointer to a variable that will receive the byte offset needed.
**
*/
gceSTATUS
_GetBankOffsetBytes(
    IN gcoSURF Surface,
    IN gceSURF_TYPE Type,
    IN gctUINT32 TopBufferSize,
    OUT gctUINT32_PTR Bytes
    )

{
    gctUINT32 baseOffset = 0;
    gctUINT32 offset     = 0;

#if gcdENABLE_BANK_ALIGNMENT
    gctUINT32 bank;
    /* To retrieve the bank. */
    static const gctUINT32 bankMask = (0xFFFFFFFF << gcdBANK_BIT_START)
                                    ^ (0xFFFFFFFF << (gcdBANK_BIT_END + 1));
#endif

    gcmHEADER_ARG("Type=%d TopBufferSize=%x Bytes=0x%x", Type, TopBufferSize, Bytes);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Bytes != gcvNULL);

    switch(Type)
    {
    case gcvSURF_RENDER_TARGET:
        /* Put second buffer atleast 16KB away. */
        baseOffset = offset = (1 << 14);

#if gcdENABLE_BANK_ALIGNMENT
        TopBufferSize += (1 << 14);
        bank = (TopBufferSize & bankMask) >> (gcdBANK_BIT_START);

        /* Put second buffer (c1 or z1) 5 banks away. */
        if (bank <= 5)
        {
            offset += (5 - bank) << (gcdBANK_BIT_START);
        }
        else
        {
            offset += (8 + 5 - bank) << (gcdBANK_BIT_START);
        }
#if gcdBANK_CHANNEL_BIT
        /* Minimum 256 byte alignment needed for fast_msaa or small msaa. */
        if ((gcdBANK_CHANNEL_BIT > 7) ||
            ((gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_FAST_MSAA) != gcvSTATUS_TRUE) &&
             (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SMALL_MSAA) != gcvSTATUS_TRUE)))

        {
            /* Add a channel offset at the channel bit. */
            offset += (1 << gcdBANK_CHANNEL_BIT);
        }

#endif
#endif
        break;

    case gcvSURF_DEPTH:
        /* Put second buffer atleast 16KB away. */
        baseOffset = offset = (1 << 14);

#if gcdENABLE_BANK_ALIGNMENT
        TopBufferSize += (1 << 14);
        bank = (TopBufferSize & bankMask) >> (gcdBANK_BIT_START);

        /* Put second buffer (c1 or z1) 5 banks away. */
        if (bank <= 5)
        {
            offset += (5 - bank) << (gcdBANK_BIT_START);
        }
        else
        {
            offset += (8 + 5 - bank) << (gcdBANK_BIT_START);
        }

#if gcdBANK_CHANNEL_BIT
        /* Subtract the channel bit, as it's added by kernel side. */
        if (offset >= (1 << gcdBANK_CHANNEL_BIT))
        {
            /* Minimum 256 byte alignment needed for fast_msaa or small msaa. */
            if ((gcdBANK_CHANNEL_BIT > 7) ||
                ((gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_FAST_MSAA) != gcvSTATUS_TRUE) &&
                 (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SMALL_MSAA) != gcvSTATUS_TRUE)))
            {
                offset -= (1 << gcdBANK_CHANNEL_BIT);
            }
        }
#endif
#endif

        break;

    default:
        /* No alignment needed. */
        baseOffset = offset = 0;
    }

    *Bytes = offset;

    /* Avoid compiler warnings. */
    baseOffset = baseOffset;

    /* Only disable bottom-buffer-offset on android system. */
#if gcdPARTIAL_FAST_CLEAR && defined(ANDROID)
    if (!Surface->info.isMsaa)
    {
        /* In NOAA mode, disable extra bottom-buffer-offset if want
         * partial fast clear. 'baseOffset' is 16KB aligned, it can still
         * support. */
        *Bytes = baseOffset;
    }
#endif

    gcmFOOTER_ARG("*Bytes=0x%x", *Bytes);
    return gcvSTATUS_OK;
}

#endif
#endif

static void
_ComputeSurfacePlacement(
    gcoSURF Surface
    )
{
    gctUINT32 blockSize;
    gcsSURF_FORMAT_INFO_PTR formatInfo = &Surface->info.formatInfo;
    blockSize = formatInfo->blockSize / formatInfo->layers;

    switch (Surface->info.format)
    {
    case gcvSURF_YV12:
        /*  WxH Y plane followed by (W/2)x(H/2) V and U planes. */
        Surface->info.stride
            = Surface->info.alignedW;

        Surface->info.uStride =
        Surface->info.vStride
#if defined(ANDROID)
            /*
             * Per google's requirement, we need u/v plane align to 16,
             * and there is no gap between YV plane
             */
            = (Surface->info.alignedW / 2 + 0xf) & ~0xf;
#else
            = (Surface->info.alignedW / 2);
#endif

        Surface->info.vOffset
            = Surface->info.stride * Surface->info.alignedH;

        Surface->info.uOffset
            = Surface->info.vOffset
            + Surface->info.vStride * Surface->info.alignedH / 2;

        Surface->info.sliceSize
            = Surface->info.uOffset
            + Surface->info.uStride * Surface->info.alignedH / 2;
        break;

    case gcvSURF_I420:
        /*  WxH Y plane followed by (W/2)x(H/2) U and V planes. */
        Surface->info.stride
            = Surface->info.alignedW;

        Surface->info.uStride =
        Surface->info.vStride
#if defined(ANDROID)
            /*
             * Per google's requirement, we need u/v plane align to 16,
             * and there is no gap between YV plane
             */
            = (Surface->info.alignedW / 2 + 0xf) & ~0xf;
#else
            = (Surface->info.alignedW / 2);
#endif

        Surface->info.uOffset
            = Surface->info.stride * Surface->info.alignedH;

        Surface->info.vOffset
            = Surface->info.uOffset
            + Surface->info.uStride * Surface->info.alignedH / 2;

        Surface->info.sliceSize
            = Surface->info.vOffset
            + Surface->info.vStride * Surface->info.alignedH / 2;
        break;

    case gcvSURF_NV12:
    case gcvSURF_NV21:
        /*  WxH Y plane followed by (W)x(H/2) interleaved U/V plane. */
        Surface->info.stride  =
        Surface->info.uStride =
        Surface->info.vStride
            = Surface->info.alignedW;

        Surface->info.uOffset =
        Surface->info.vOffset
            = Surface->info.stride * Surface->info.alignedH;

        Surface->info.sliceSize
            = Surface->info.uOffset
            + Surface->info.uStride * Surface->info.alignedH / 2;
        break;

    case gcvSURF_NV16:
    case gcvSURF_NV61:
        /*  WxH Y plane followed by WxH interleaved U/V(V/U) plane. */
        Surface->info.stride  =
        Surface->info.uStride =
        Surface->info.vStride
            = Surface->info.alignedW;

        Surface->info.uOffset =
        Surface->info.vOffset
            = Surface->info.stride * Surface->info.alignedH;

        Surface->info.sliceSize
            = Surface->info.uOffset
            + Surface->info.uStride * Surface->info.alignedH;
        break;

    case gcvSURF_YUY2:
    case gcvSURF_UYVY:
    case gcvSURF_YVYU:
    case gcvSURF_VYUY:
        /*  WxH interleaved Y/U/Y/V plane. */
        Surface->info.stride  =
        Surface->info.uStride =
        Surface->info.vStride
            = Surface->info.alignedW * 2;

        Surface->info.uOffset = Surface->info.vOffset = 0;

        Surface->info.sliceSize
            = Surface->info.stride * Surface->info.alignedH;
        break;

    default:
        Surface->info.stride
            = (Surface->info.alignedW / formatInfo->blockWidth)
            * blockSize / 8;

        Surface->info.uStride = Surface->info.vStride = 0;

        Surface->info.uOffset = Surface->info.vOffset = 0;

        Surface->info.sliceSize
            = (Surface->info.alignedW / formatInfo->blockWidth)
            * (Surface->info.alignedH / formatInfo->blockHeight)
            * blockSize / 8;
        break;
    }
}

static gceSTATUS
_AllocateSurface(
    IN gcoSURF Surface,
    IN gctUINT Width,
    IN gctUINT Height,
    IN gctUINT Depth,
    IN gceSURF_TYPE Type,
    IN gceSURF_FORMAT Format,
    IN gctUINT Samples,
    IN gcePOOL Pool
    )
{
    gceSTATUS status;
    gceSURF_FORMAT format;
    gcsSURF_FORMAT_INFO_PTR formatInfo;
    /* Extra pages needed to offset sub-buffers to different banks. */
    gctUINT32 bankOffsetBytes = 0;
    gctUINT32 layers;
#if gcdENABLE_3D
    gctUINT32 blockSize;
#endif

#if gcdENABLE_VG
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;

    gcmGETCURRENTHARDWARE(currentType);
#endif

    format = (gceSURF_FORMAT) (Format & ~gcvSURF_FORMAT_OCL);
    gcmONERROR(gcoSURF_QueryFormat(format, &formatInfo));
    Surface->info.formatInfo = *formatInfo;
    layers = formatInfo->layers;
#if gcdENABLE_3D
    blockSize = formatInfo->blockSize / layers;
#endif

#if gcdENABLE_VG
    if (currentType == gcvHARDWARE_VG)
    {
        /* Compute bits per pixel. */
        gcmONERROR(
            gcoVGHARDWARE_ConvertFormat(gcvNULL,
                                        format,
                                        (gctUINT32_PTR)&Surface->info.bitsPerPixel,
                                        gcvNULL));
    }
    else
#endif
    {
        Surface->info.bitsPerPixel = formatInfo->bitsPerPixel;
    }

    if (Samples > 4 || Samples == 3)
    {
        /* Invalid multi-sample request. */
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }
    Surface->info.sampleInfo = g_sampleInfos[Samples];
    Surface->info.isMsaa     = (Surface->info.sampleInfo.product > 1);

    if (Surface->info.isMsaa &&
        gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_VMSAA))
    {
        Surface->info.edgeAA = gcvTRUE;
    }
    else
    {
        Surface->info.edgeAA = gcvFALSE;
    }

    /* Set dimensions of surface. */
    Surface->info.requestW = Width;
    Surface->info.requestH = Height;
    Surface->info.requestD = Depth;
    Surface->info.allocedW = Width  * Surface->info.sampleInfo.x;
    Surface->info.allocedH = Height * Surface->info.sampleInfo.y;

    /* Initialize rotation. */
    Surface->info.rotation    = gcvSURF_0_DEGREE;
#if gcdENABLE_3D
    Surface->info.orientation = gcvORIENTATION_TOP_BOTTOM;
    Surface->resolvable       = gcvTRUE;
#endif

    /* Obtain canonical type of surface. */
    Surface->info.type   = (gceSURF_TYPE) ((gctUINT32) Type & 0xFF);
    /* Get 'hints' of this surface. */
    Surface->info.hints  = (gceSURF_TYPE) ((gctUINT32) Type & ~0xFF);
    /* Append texture surface flag */
    Surface->info.hints |= (Surface->info.type == gcvSURF_TEXTURE) ? gcvSURF_CREATE_AS_TEXTURE : 0;
    /* Set format of surface. */
    Surface->info.format = format;
    Surface->info.tiling = (Surface->info.type == gcvSURF_TEXTURE)
                         ? (((Surface->info.hints & gcvSURF_LINEAR) == gcvSURF_LINEAR)
                            ? gcvLINEAR
                            : gcvTILED)
                         : gcvLINEAR
                         ;
    Surface->info.cacheMode  = gcvCACHE_NONE;

    /* Set aligned surface size. */
    Surface->info.alignedW = Surface->info.allocedW;
    Surface->info.alignedH = Surface->info.allocedH;

#if gcdENABLE_3D
    /* Tile status disabled currently. */
    Surface->info.tileStatusDisabled = gcvTRUE;

    /* Init superTiled info. */
    Surface->info.superTiled = gcvFALSE;
#endif

    /* User pool? */
    if (Pool == gcvPOOL_USER)
    {
        /* Init the node as the user allocated. */
        Surface->info.node.pool                    = gcvPOOL_USER;
        Surface->info.node.u.wrapped.mappingInfo   = gcvNULL;

        /* Align the dimensions by the block size. */
        Surface->info.alignedW  = gcmALIGN_NP2(Surface->info.alignedW,
                                                   formatInfo->blockWidth);
        Surface->info.alignedH = gcmALIGN_NP2(Surface->info.alignedH,
                                                   formatInfo->blockHeight);

        /* Always single layer for user surface */
        gcmASSERT(layers == 1);

        /* Compute the surface placement parameters. */
        _ComputeSurfacePlacement(Surface);

        Surface->info.layerSize = Surface->info.sliceSize * Surface->info.requestD;

        Surface->info.size = Surface->info.layerSize * layers;

        if (Surface->info.type == gcvSURF_TEXTURE)
        {
            Surface->info.tiling = gcvTILED;
        }
    }

    /* No --> allocate video memory. */
    else
    {
#if gcdENABLE_VG
        if (currentType == gcvHARDWARE_VG)
        {
            gcmONERROR(
                gcoVGHARDWARE_AlignToTile(gcvNULL,
                                          Surface->info.type,
                                          &Surface->info.alignedW,
                                          &Surface->info.alignedH));
        }
        else
#endif
        {
            /* Align width and height to tiles. */
#if gcdENABLE_3D
            if (Surface->info.isMsaa &&
                ((Surface->info.type != gcvSURF_TEXTURE) ||
                 (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_MSAA_TEXTURE)))
               )
            {
                gcmONERROR(
                    gcoHARDWARE_AlignToTileCompatible(gcvNULL,
                                                      Surface->info.type,
                                                      Surface->info.hints,
                                                      Format,
                                                      &Width,
                                                      &Height,
                                                      Depth,
                                                      &Surface->info.tiling,
                                                      &Surface->info.superTiled,
                                                      &Surface->info.hAlignment));

                Surface->info.alignedW = Width  * Surface->info.sampleInfo.x;
                Surface->info.alignedH = Height * Surface->info.sampleInfo.y;
            }
            else
            {
                gcmONERROR(
                    gcoHARDWARE_AlignToTileCompatible(gcvNULL,
                                                      Surface->info.type,
                                                      Surface->info.hints,
                                                      Format,
                                                      &Surface->info.alignedW,
                                                      &Surface->info.alignedH,
                                                      Depth,
                                                      &Surface->info.tiling,
                                                      &Surface->info.superTiled,
                                                      &Surface->info.hAlignment));
            }
#else
            gcmONERROR(
                gcoHARDWARE_AlignToTileCompatible(gcvNULL,
                                                  Surface->info.type,
                                                  Surface->info.hints,
                                                  Format,
                                                  &Surface->info.alignedW,
                                                  &Surface->info.alignedH,
                                                  Depth,
                                                  &Surface->info.tiling,
                                                  gcvNULL,
                                                  gcvNULL));
#endif /* gcdENABLE_3D */
        }

        /*
        ** We cannot use multi tiled/supertiled to create 3D surface, this surface cannot be recognized
        ** by texture unit now.
        */
        if ((Depth > 1) &&
            (Surface->info.tiling & gcvTILING_SPLIT_BUFFER))
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }
        /* Determine bank offset bytes. */
#if gcdENABLE_3D
        /* If HW need to be programmed as multi pipe with 2 addresses,
         * bottom addresses need to calculated now, no matter the surface itself
         * was split or not.
         */
        if (Surface->info.tiling & gcvTILING_SPLIT_BUFFER)
        {
            gctUINT halfHeight = gcmALIGN(Surface->info.alignedH / 2,
                                          Surface->info.superTiled ? 64 : 4);

            gctUINT32 topBufferSize = ((Surface->info.alignedW/ formatInfo->blockWidth)
                                    *  (halfHeight/ formatInfo->blockHeight)
                                    *  blockSize) / 8;

#if gcdENABLE_BUFFER_ALIGNMENT
            if (Surface->info.tiling & gcvTILING_SPLIT_BUFFER)
            {
                gcmONERROR(
                    _GetBankOffsetBytes(Surface,
                                        Surface->info.type,
                                        topBufferSize,
                                        &bankOffsetBytes));
            }
#endif
            Surface->info.bottomBufferOffset = topBufferSize + bankOffsetBytes;
        }
        else
#endif
        {
            Surface->info.bottomBufferOffset = 0;
        }

        /* Compute the surface placement parameters. */
        _ComputeSurfacePlacement(Surface);

        /* Append bank offset bytes. */
        Surface->info.sliceSize += bankOffsetBytes;

        if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_128BTILE))
        {
            /* Align to 256B */
            Surface->info.sliceSize = gcmALIGN(Surface->info.sliceSize, 256);
        }

        Surface->info.layerSize = Surface->info.sliceSize * Surface->info.requestD;

        Surface->info.size = Surface->info.layerSize * layers;
    }

    if (!(Surface->info.hints & gcvSURF_NO_VIDMEM) && (Pool != gcvPOOL_USER))
    {
        gctUINT bytes = Surface->info.size;
        gctUINT alignment = 0;
        gctUINT32 allocFlags = gcvALLOC_FLAG_NONE;

#if gcdENABLE_3D
        if ((gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TILE_FILLER)) &&
            (gcoHARDWARE_QuerySurfaceRenderable(gcvNULL, &Surface->info) == gcvSTATUS_OK) &&
            Depth == 1 )
        {
            if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_TILEFILLER_32TILE_ALIGNED))
            {
                /* 32 tile alignment. */
                bytes = gcmALIGN(bytes, 32 * 64);
            }
            else
            {
                /* 256 tile alignment for fast clear fill feature. */
                bytes = gcmALIGN(bytes, 256 * 64);
            }
        }

        if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_128BTILE))
        {
            bytes = gcmALIGN(bytes,256);

            alignment = 256;
        }
        else if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_COMPRESSION_V4) &&
            formatInfo->bitsPerPixel >= 64)
        {
            alignment = 512;
        }
        else if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NEW_RA) ||
            gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_COLOR_COMPRESSION) ||
            gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_COMPRESSION_V4))
        {
            /* - Depth surfaces need 256 byte alignment.
             * - Color compression need 256 byte alignment(same in all versions?)
             *   Doing this generically.
             */
            alignment = 256;
        }
        else if (Surface->info.isMsaa)
        {
            /* 256 byte alignment for MSAA surface */
            alignment = 256;
        }
        else if (Surface->info.hints & gcvSURF_DEC)
        {
            /* 128 byte alignment for DEC format surface */
            alignment = 128;
        }
        else
#endif
        {
            /* alignment should be 16(pixels) * byte per pixels for tiled surface*/
            alignment = (formatInfo->bitsPerPixel >= 64) ? (4*4*formatInfo->bitsPerPixel/8) : 64;
        }

#if gcdENABLE_2D
        if ((Surface->info.type == gcvSURF_BITMAP)
#if gcdENABLE_VG
        &&  (currentType != gcvHARDWARE_VG)
#   endif
        &&  gcoHARDWARE_Is2DAvailable(gcvNULL))
        {
            gcoHARDWARE_Query2DSurfaceAllocationInfo(gcvNULL,
                                                     &Surface->info,
                                                     &bytes,
                                                     &alignment);
        }
#endif

        if (Surface->info.hints & gcvSURF_PROTECTED_CONTENT)
        {
            allocFlags |= gcvALLOC_FLAG_SECURITY;
        }

        if (Surface->info.hints & gcvSURF_CONTIGUOUS)
        {
            allocFlags |= gcvALLOC_FLAG_CONTIGUOUS;
        }

        if (Surface->info.hints & gcvSURF_CACHEABLE)
        {
            allocFlags |= gcvALLOC_FLAG_CACHEABLE;
        }

        if (Format & gcvSURF_FORMAT_OCL)
        {
            bytes = gcmALIGN(bytes + 64,  64);
        }

#if gcdENABLE_2D
        /* Allocate extra stride to avoid cache overflow of quad */
        if (Surface->info.tiling == gcvLINEAR)
        {
            gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;

            /* Save the current hardware type */
            gcmVERIFY_OK(gcoHAL_GetHardwareType(gcvNULL, &currentType));
            /* Change to the 2d hardware type */
            gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_2D));

            if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_ALL_QUAD))
            {
                bytes += Surface->info.stride;
            }

            /* Restore the current hardware type */
            gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, currentType));
        }
#endif

        gcmONERROR(gcsSURF_NODE_Construct(
            &Surface->info.node,
            bytes,
            alignment,
            Surface->info.type,
            allocFlags,
            Pool
            ));

        gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                      "Allocated surface 0x%x: pool=%d size=%dx%dx%d bytes=%u",
                      &Surface->info.node,
                      Surface->info.node.pool,
                      Surface->info.alignedW,
                      Surface->info.alignedH,
                      Surface->info.requestD,
                      Surface->info.size);
    }

#if gcdENABLE_3D
    /* No Hierarchical Z buffer allocated. */
    gcmONERROR(gcoSURF_AllocateHzBuffer(Surface));

    if ((Pool != gcvPOOL_USER) &&
        ((Surface->info.type == gcvSURF_DEPTH) ||
         (Surface->info.type == gcvSURF_RENDER_TARGET)))
    {
        /* Allocate tile status buffer after HZ buffer
        ** b/c we need allocate HZ tile status if HZ exist.
        */
        gcmONERROR(gcoSURF_AllocateTileStatus(Surface));
    }

    Surface->info.hasStencilComponent = (format == gcvSURF_D24S8             ||
                                         format == gcvSURF_S8D32F            ||
                                         format == gcvSURF_S8D32F_2_A8R8G8B8 ||
                                         format == gcvSURF_S8D32F_1_G32R32F  ||
                                         format == gcvSURF_D24S8_1_A8R8G8B8  ||
                                         format == gcvSURF_S8                ||
                                         format == gcvSURF_X24S8);

    Surface->info.canDropStencilPlane = gcvTRUE;
#endif

    if (Pool != gcvPOOL_USER)
    {
        if (!(Surface->info.hints & gcvSURF_NO_VIDMEM))
        {
            /* Lock the surface. */
            gcmONERROR(_Lock(Surface));
        }
    }

    Surface->info.pfGetAddr = gcoHARDWARE_GetProcCalcPixelAddr(gcvNULL, &Surface->info);

    /* Success. */
    return gcvSTATUS_OK;

OnError:
    /* Free the memory allocated to the surface. */
    _FreeSurface(Surface);

    /* Return the status. */
    return status;
}

static gceSTATUS
_UnmapUserBuffer(
    IN gcoSURF Surface,
    IN gctBOOL ForceUnmap
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Surface=0x%x ForceUnmap=%d", Surface, ForceUnmap);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    do
    {
        /* Cannot be negative. */
        gcmASSERT(Surface->info.node.lockCount >= 0);

        if (Surface->info.node.lockCount == 0)
        {
            /* Nothing to do. */
            status = gcvSTATUS_OK;
            break;
        }

        /* Make sure the reference counter is proper. */
        if (Surface->info.node.lockCount > 1)
        {
            /* Forced unmap? */
            if (ForceUnmap)
            {
                /* Invalid reference count. */
                gcmASSERT(gcvFALSE);
            }
            else
            {
                /* Decrement. */
                Surface->info.node.lockCount -= 1;

                /* Done for now. */
                status = gcvSTATUS_OK;
                break;
            }
        }

        /* Unmap the physical memory. */
        if (Surface->info.node.u.wrapped.mappingInfo != gcvNULL)
        {
            gctUINT32 address;

            gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;

            /* Save the current hardware type */
            gcmVERIFY_OK(gcoHAL_GetHardwareType(gcvNULL, &currentType));

            if (Surface->info.node.u.wrapped.mappingHardwareType != currentType)
            {
                /* Change to the mapping hardware type */
                gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL,
                                                    Surface->info.node.u.wrapped.mappingHardwareType));
            }

            gcmGETHARDWAREADDRESS(Surface->info.node, address);

            gcmERR_BREAK(gcoHAL_ScheduleUnmapUserMemory(
                gcvNULL,
                Surface->info.node.u.wrapped.mappingInfo,
                Surface->info.size,
                address,
                Surface->info.node.logical
                ));

            Surface->info.node.logical = gcvNULL;
            Surface->info.node.u.wrapped.mappingInfo = gcvNULL;

            if (Surface->info.node.u.wrapped.mappingHardwareType != currentType)
            {
                /* Restore the current hardware type */
                gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, currentType));
            }
        }

#if gcdSECURE_USER
        gcmERR_BREAK(gcoHAL_ScheduleUnmapUserMemory(
            gcvNULL,
            gcvNULL,
            Surface->info.size,
            0,
            Surface->info.node.logical));
#endif

        /* Reset the surface. */
        Surface->info.node.lockCount = 0;
        Surface->info.node.valid = gcvFALSE;
    }
    while (gcvFALSE);

    /* Return the status. */
    gcmFOOTER();
    return status;
}


/******************************************************************************\
******************************** gcoSURF API Code *******************************
\******************************************************************************/

/*******************************************************************************
**
**  gcoSURF_Construct
**
**  Create a new gcoSURF object.
**
**  INPUT:
**
**      gcoHAL Hal
**          Pointer to an gcoHAL object.
**
**      gctUINT Width
**          Width of surface to create in pixels.
**
**      gctUINT Height
**          Height of surface to create in lines.
**
**      gctUINT Depth
**          Depth of surface to create in planes.
**
**      gceSURF_TYPE Type
**          Type of surface to create.
**
**      gceSURF_FORMAT Format
**          Format of surface to create.
**
**      gcePOOL Pool
**          Pool to allocate surface from.
**
**  OUTPUT:
**
**      gcoSURF * Surface
**          Pointer to the variable that will hold the gcoSURF object pointer.
*/
gceSTATUS
gcoSURF_Construct(
    IN gcoHAL Hal,
    IN gctUINT Width,
    IN gctUINT Height,
    IN gctUINT Depth,
    IN gceSURF_TYPE Type,
    IN gceSURF_FORMAT Format,
    IN gcePOOL Pool,
    OUT gcoSURF * Surface
    )
{
    gcoSURF surface = gcvNULL;
    gceSTATUS status;

    gcmHEADER_ARG("Width=%u Height=%u Depth=%u Type=%d Format=%d Pool=%d",
                  Width, Height, Depth, Type, Format, Pool);

    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT(Surface != gcvNULL);

    /* Allocate the gcoSURF object. */
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(struct _gcoSURF), (gctPOINTER*)&surface));
    gcoOS_ZeroMemory(surface, gcmSIZEOF(struct _gcoSURF));


    /* Initialize the gcoSURF object.*/
    surface->object.type    = gcvOBJ_SURF;
    surface->info.offset    = 0;

    surface->info.dither2D      = gcvFALSE;
    surface->info.deferDither3D = gcvFALSE;
    surface->info.paddingFormat = (Format == gcvSURF_R8_1_X8R8G8B8 || Format == gcvSURF_G8R8_1_X8R8G8B8)
                                ? gcvTRUE : gcvFALSE;
    surface->info.garbagePadded = gcvTRUE;

#if gcdENABLE_3D
    surface->info.colorType = gcvSURF_COLOR_UNKNOWN;
    surface->info.flags = gcvSURF_FLAG_NONE;
    surface->info.colorSpace = gcd_QUERY_COLOR_SPACE(Format);
    surface->info.hzNode.pool = gcvPOOL_UNKNOWN;
    surface->info.tileStatusNode.pool = gcvPOOL_UNKNOWN;
    surface->info.hzTileStatusNode.pool = gcvPOOL_UNKNOWN;
#endif /* gcdENABLE_3D */

    if (Type & gcvSURF_CACHEABLE)
    {
        gcmASSERT(Pool != gcvPOOL_USER);
        surface->info.node.u.normal.cacheable = gcvTRUE;
    }
    else if (Pool != gcvPOOL_USER)
    {
        surface->info.node.u.normal.cacheable = gcvFALSE;
    }

#if gcdENABLE_3D
    if (Type & gcvSURF_TILE_STATUS_DIRTY)
    {
        surface->info.TSDirty = gcvTRUE;
        Type &= ~gcvSURF_TILE_STATUS_DIRTY;
    }
#endif

    if (Depth < 1)
    {
        /* One plane. */
        Depth = 1;
    }

    /* Allocate surface. */
    gcmONERROR(
        _AllocateSurface(surface,
                         Width, Height, Depth,
                         Type,
                         Format,
                         1,
                         Pool));

    surface->referenceCount = 1;

    gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                  "Created gcoSURF 0x%x",
                  surface);

    /* Return pointer to the gcoSURF object. */
    *Surface = surface;

    /* Success. */
    gcmFOOTER_ARG("*Surface=0x%x", *Surface);
    return gcvSTATUS_OK;

OnError:
    /* Free the allocated memory. */
    if (surface != gcvNULL)
    {
        gcmOS_SAFE_FREE(gcvNULL, surface);
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_Destroy
**
**  Destroy an gcoSURF object.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object to be destroyed.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_Destroy(
    IN gcoSURF Surface
    )
{
#if gcdENABLE_3D
    gcsTLS_PTR tls;
#endif

    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Decrement surface reference count. */
    if (--Surface->referenceCount != 0)
    {
        /* Still references to this surface. */
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

#if gcdENABLE_3D

    if (gcmIS_ERROR(gcoOS_GetTLS(&tls)))
    {
        gcmFOOTER_NO();
        return gcvSTATUS_NOT_SUPPORTED;
    }

#if gcdENABLE_VG
    /* Unset VG target. */
    if (tls->engineVG != gcvNULL)
    {
#if gcdGC355_PROFILER
        gcmVERIFY_OK(
            gcoVG_UnsetTarget(tls->engineVG,
            0,0,0,
            Surface));
#else
        gcmVERIFY_OK(
            gcoVG_UnsetTarget(tls->engineVG,
            Surface));
#endif
    }
#endif

    /* Special case for 3D surfaces. */
    if (tls->engine3D != gcvNULL)
    {
        /* If this is a render target, unset it. */
        if ((Surface->info.type == gcvSURF_RENDER_TARGET)
        ||  (Surface->info.type == gcvSURF_TEXTURE)
        )
        {
            gctUINT32 rtIdx;

            for (rtIdx = 0; rtIdx < 4; ++rtIdx)
            {
                gcmVERIFY_OK(gco3D_UnsetTarget(tls->engine3D, rtIdx, Surface));
            }
        }

        /* If this is a depth buffer, unset it. */
        else if (Surface->info.type == gcvSURF_DEPTH)
        {
            gcmVERIFY_OK(gco3D_UnsetDepth(tls->engine3D, Surface));
        }
    }
#endif

#if gcdGC355_MEM_PRINT
#ifdef LINUX
    gcoOS_AddRecordAllocation(-(gctINT32)Surface->info.node.size);
#endif
#endif

    /* Free the video memory. */
    gcmVERIFY_OK(_FreeSurface(Surface));

    /* Mark gcoSURF object as unknown. */
    Surface->object.type = gcvOBJ_UNKNOWN;

    /* Free the gcoSURF object. */
    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Surface));

    gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                  "Destroyed gcoSURF 0x%x",
                  Surface);

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoSURF_QueryVidMemNode
**
**  Query the video memory node attributes of a surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**  OUTPUT:
**
**      gctUINT32 * Node
**          Pointer to a variable receiving the video memory node.
**
**      gcePOOL * Pool
**          Pointer to a variable receiving the pool the video memory node originated from.
**
**      gctUINT_PTR Bytes
**          Pointer to a variable receiving the video memory node size in bytes.
**
*/
gceSTATUS
gcoSURF_QueryVidMemNode(
    IN gcoSURF Surface,
    OUT gctUINT32 * Node,
    OUT gcePOOL * Pool,
    OUT gctSIZE_T_PTR Bytes
    )
{
    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);
    gcmVERIFY_ARGUMENT(Node != gcvNULL);
    gcmVERIFY_ARGUMENT(Pool != gcvNULL);
    gcmVERIFY_ARGUMENT(Bytes != gcvNULL);

    /* Return the video memory attributes. */
    *Node = Surface->info.node.u.normal.node;
    *Pool = Surface->info.node.pool;
    *Bytes = Surface->info.node.size;

    /* Success. */
    gcmFOOTER_ARG("*Node=0x%x *Pool=%d *Bytes=%d", *Node, *Pool, *Bytes);
    return gcvSTATUS_OK;
}


/*******************************************************************************
**
**  gcoSURF_WrapSurface
**
**  Wrap gcoSURF_Object with known logica address (CPU) and physical address(GPU)
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object to be destroyed.
**
**      gctUINT Alignment
**          Alignment of each pixel row in bytes.
**
**      gctPOINTER Logical
**          Logical pointer to the user allocated surface or gcvNULL if no
**          logical pointer has been provided.
**
**      gctUINT32 Physical
**          Physical pointer(GPU address) to the user allocated surface.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_WrapSurface(
    IN gcoSURF Surface,
    IN gctUINT Alignment,
    IN gctPOINTER Logical,
    IN gctUINT32 Physical
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 address;

#if gcdENABLE_VG
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;
#endif

    gcmHEADER_ARG("Surface=0x%x Alignment=%u Logical=0x%x Physical=%08x",
              Surface, Alignment, Logical, Physical);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    do
    {
        /* Has to be user-allocated surface. */
        if (Surface->info.node.pool != gcvPOOL_USER)
        {
            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }

        /* Already mapped? */
        if (Surface->info.node.lockCount > 0)
        {
            if ((Logical != gcvNULL) &&
                (Logical != Surface->info.node.logical))
            {
                status = gcvSTATUS_INVALID_ARGUMENT;
                break;
            }

            gcmGETHARDWAREADDRESS(Surface->info.node, address);

            if ((Physical != gcvINVALID_ADDRESS) &&
                (Physical != address))
            {
                status = gcvSTATUS_INVALID_ARGUMENT;
                break;
            }

            /* Success. */
            break;
        }

#if gcdENABLE_VG
        gcmGETCURRENTHARDWARE(currentType);
#endif
        /* Set new alignment. */
        if (Alignment != 0)
        {
            /* Compute the unaligned stride. */
            gctUINT32 stride;
            gctUINT32 bytesPerPixel;
            gcsSURF_FORMAT_INFO_PTR formatInfo = &Surface->info.formatInfo;

            switch (Surface->info.format)
            {
            case gcvSURF_YV12:
            case gcvSURF_I420:
            case gcvSURF_NV12:
            case gcvSURF_NV21:
            case gcvSURF_NV16:
            case gcvSURF_NV61:
                bytesPerPixel = 1;
                stride = Surface->info.alignedW;
                break;

            default:
                bytesPerPixel = Surface->info.bitsPerPixel / 8;
                stride = Surface->info.alignedW * Surface->info.bitsPerPixel / 8;
                break;
            }

            /* Align the stride (Alignment can be not a power of number). */
            if (gcoMATH_ModuloUInt(stride, Alignment) != 0)
            {
                stride = gcoMATH_DivideUInt(stride, Alignment)  * Alignment
                       + Alignment;

                Surface->info.alignedW = stride / bytesPerPixel;
            }

            /* Compute the new surface placement parameters. */
            _ComputeSurfacePlacement(Surface);

            if (stride != Surface->info.stride)
            {
                /*
                 * Still not equal, which means user stride is not pixel aligned, ie,
                 * stride != alignedWidth(user) * bytesPerPixel
                 */
                Surface->info.stride = stride;

                /* Re-calculate slice size. */
                Surface->info.sliceSize = stride * (Surface->info.alignedH / formatInfo->blockHeight);
            }

            Surface->info.layerSize = Surface->info.sliceSize * Surface->info.requestD;

            /* We won't map multi-layer surface. */
            gcmASSERT(formatInfo->layers == 1);

            /* Compute the new size. */
            Surface->info.size = Surface->info.layerSize * formatInfo->layers;
        }

        /* Validate the surface. */
        Surface->info.node.valid = gcvTRUE;

        /* Set the lock count. */
        Surface->info.node.lockCount++;

        /* Set the node parameters. */
        Surface->info.node.u.wrapped.mappingInfo   = gcvNULL;

        Surface->info.node.logical                 = Logical;
        gcsSURF_NODE_SetHardwareAddress(&Surface->info.node, Physical);
        Surface->info.node.count                   = 1;

        Surface->info.node.u.normal.node = 0;
        Surface->info.node.u.wrapped.physical = Physical;
    }
    while (gcvFALSE);

    /* Return the status. */
    gcmFOOTER();
    return status;
}


/*******************************************************************************
**
**  gcoSURF_MapUserSurface
**
**  Store the logical and physical pointers to the user-allocated surface in
**  the gcoSURF object and map the pointers as necessary.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object to be destroyed.
**
**      gctUINT Alignment
**          Alignment of each pixel row in bytes.
**
**      gctPOINTER Logical
**          Logical pointer to the user allocated surface or gcvNULL if no
**          logical pointer has been provided.
**
**      gctUINT32 Physical
**          Physical pointer to the user allocated surface or gcvINVALID_ADDRESS if no
**          physical pointer has been provided.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_MapUserSurface(
    IN gcoSURF Surface,
    IN gctUINT Alignment,
    IN gctPOINTER Logical,
    IN gctUINT32 Physical
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gctPOINTER mappingInfo = gcvNULL;

    gctPOINTER logical = gcvNULL;
    gctUINT32 physical = 0;
#if gcdENABLE_VG
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;
#endif
    gctUINT32 address;
    gcsUSER_MEMORY_DESC desc;

    gcmHEADER_ARG("Surface=0x%x Alignment=%u Logical=0x%x Physical=%08x",
              Surface, Alignment, Logical, Physical);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    do
    {
        /* Has to be user-allocated surface. */
        if (Surface->info.node.pool != gcvPOOL_USER)
        {
            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }

        /* Already mapped? */
        if (Surface->info.node.lockCount > 0)
        {
            if ((Logical != gcvNULL) &&
                (Logical != Surface->info.node.logical))
            {
                status = gcvSTATUS_INVALID_ARGUMENT;
                break;
            }

            gcmGETHARDWAREADDRESS(Surface->info.node, address);

            if ((Physical != gcvINVALID_ADDRESS) &&
                (Physical != address))
            {
                status = gcvSTATUS_INVALID_ARGUMENT;
                break;
            }

            /* Success. */
            break;
        }

#if gcdENABLE_VG
        gcmGETCURRENTHARDWARE(currentType);
#endif
        /* Set new alignment. */
        if (Alignment != 0)
        {
            /* Compute the unaligned stride. */
            gctUINT32 stride;
            gctUINT32 bytesPerPixel;
            gcsSURF_FORMAT_INFO_PTR formatInfo = &Surface->info.formatInfo;

            switch (Surface->info.format)
            {
            case gcvSURF_YV12:
            case gcvSURF_I420:
            case gcvSURF_NV12:
            case gcvSURF_NV21:
            case gcvSURF_NV16:
            case gcvSURF_NV61:
                bytesPerPixel = 1;
                stride = Surface->info.alignedW;
                break;

            default:
                bytesPerPixel = Surface->info.bitsPerPixel / 8;
                stride = Surface->info.alignedW * Surface->info.bitsPerPixel / 8;
                break;
            }

            /* Align the stride (Alignment can be not a power of number). */
            if (gcoMATH_ModuloUInt(stride, Alignment) != 0)
            {
                stride = gcoMATH_DivideUInt(stride, Alignment)  * Alignment
                       + Alignment;

                Surface->info.alignedW = stride / bytesPerPixel;
            }

            /* Compute the new surface placement parameters. */
            _ComputeSurfacePlacement(Surface);

            if (stride != Surface->info.stride)
            {
                /*
                 * Still not equal, which means user stride is not pixel aligned, ie,
                 * stride != alignedWidth(user) * bytesPerPixel
                 */
                Surface->info.stride = stride;

                /* Re-calculate slice size. */
                Surface->info.sliceSize = stride * (Surface->info.alignedH / formatInfo->blockHeight);
            }

            Surface->info.layerSize = Surface->info.sliceSize * Surface->info.requestD;

            /* We won't map multi-layer surface. */
            gcmASSERT(formatInfo->layers == 1);

            /* Compute the new size. */
            Surface->info.size = Surface->info.layerSize * formatInfo->layers;
        }

        /* Map logical pointer if not specified. */
        if (Logical == gcvNULL)
        {
            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }
        else
        {
            /* Set the logical pointer. */
            logical = Logical;
        }

        desc.physical = Physical;
        desc.logical = gcmPTR_TO_UINT64(Logical);
        desc.size = Surface->info.size;
        desc.flag = gcvALLOC_FLAG_USERMEMORY;

        gcmERR_BREAK(gcoHAL_WrapUserMemory(
            &desc,
            &Surface->info.node.u.normal.node
            ));

        Surface->info.node.u.wrapped.physical = Physical;
        Surface->info.node.logical = logical;

        /* Because _FreeSurface() is used to free user surface too, we need to
        ** reference this node for current hardware here, like gcoSRUF_Construct() does.
        */
        gcmERR_BREAK(gcoHARDWARE_Lock(
            &Surface->info.node,
            gcvNULL,
            gcvNULL
            ));
    }
    while (gcvFALSE);

    /* Roll back. */
    if (gcmIS_ERROR(status))
    {
        if (mappingInfo != gcvNULL)
        {
            gcmVERIFY_OK(gcoOS_UnmapUserMemory(
                gcvNULL,
                logical,
                Surface->info.size,
                mappingInfo,
                physical
                ));
        }
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_IsValid
**
**  Verify whether the surface is a valid gcoSURF object.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to a gcoSURF object.
**
**  RETURNS:
**
**      The return value of the function is set to gcvSTATUS_TRUE if the
**      surface is valid.
*/
gceSTATUS
gcoSURF_IsValid(
    IN gcoSURF Surface
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Set return value. */
    status = ((Surface != gcvNULL)
        && (Surface->object.type != gcvOBJ_SURF))
        ? gcvSTATUS_FALSE
        : gcvSTATUS_TRUE;

    /* Return the status. */
    gcmFOOTER();
    return status;
}

#if gcdENABLE_3D
/*******************************************************************************
**
**  gcoSURF_IsTileStatusSupported
**
**  Verify whether the tile status is supported by the surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to a gcoSURF object.
**
**  RETURNS:
**
**      The return value of the function is set to gcvSTATUS_TRUE if the
**      tile status is supported.
*/
gceSTATUS
gcoSURF_IsTileStatusSupported(
    IN gcoSURF Surface
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Set return value. */
    status = (Surface->info.tileStatusNode.pool == gcvPOOL_UNKNOWN)
        ? gcvSTATUS_NOT_SUPPORTED
        : gcvSTATUS_TRUE;

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_IsTileStatusEnabled
**
**  Verify whether the tile status is enabled on this surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to a gcoSURF object.
**
**  RETURNS:
**
**      The return value of the function is set to gcvSTATUS_TRUE if the
**      tile status is enabled.
*/
gceSTATUS
gcoSURF_IsTileStatusEnabled(
    IN gcoSURF Surface
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Check whether the surface has enabled tile status. */
    if ((Surface->info.tileStatusNode.pool != gcvPOOL_UNKNOWN) &&
        (Surface->info.tileStatusDisabled == gcvFALSE))
    {
        status = gcvSTATUS_TRUE;
    }
    else
    {
        status = gcvSTATUS_FALSE;
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_IsCompressed
**
**  Verify whether the surface is compressed.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to a gcoSURF object.
**
**  RETURNS:
**
**      The return value of the function is set to gcvSTATUS_TRUE if the
**      tile status is supported.
*/
gceSTATUS
gcoSURF_IsCompressed(
    IN gcoSURF Surface
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Check whether the surface is compressed. */
    if ((Surface->info.tileStatusNode.pool != gcvPOOL_UNKNOWN) &&
        (Surface->info.tileStatusDisabled == gcvFALSE) &&
        Surface->info.compressed)
    {
        status = gcvSTATUS_TRUE;
    }
    else
    {
        status = gcvSTATUS_FALSE;
    }

    /* Return status. */
    gcmFOOTER_NO();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_EnableTileStatusEx
**
**  Enable tile status for the specified surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**      gctUINT RtIndex
**          Which RT slot will be bound for this surface
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_EnableTileStatusEx(
    IN gcoSURF Surface,
    IN gctUINT RtIndex
    )
{
    gceSTATUS status;
    gctUINT32 tileStatusAddress = 0;

    gcmHEADER_ARG("Surface=0x%x RtIndex=%d", Surface, RtIndex);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    do
    {
        if (Surface->info.tileStatusNode.pool != gcvPOOL_UNKNOWN)
        {
            gcmGETHARDWAREADDRESS(Surface->info.tileStatusNode, tileStatusAddress);
        }

        /* Enable tile status. */
        gcmERR_BREAK(
            gcoHARDWARE_EnableTileStatus(gcvNULL,
                                         &Surface->info,
                                         tileStatusAddress,
                                         &Surface->info.hzTileStatusNode,
                                         RtIndex));

        /* Success. */
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_EnableTileStatus
**
**  Enable tile status for the specified surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_EnableTileStatus(
    IN gcoSURF Surface
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    status = gcoSURF_EnableTileStatusEx(Surface, 0);

    /* Return the status. */
    gcmFOOTER();
    return status;
}


/*******************************************************************************
**
**  gcoSURF_DisableTileStatus
**
**  Disable tile status for the specified surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**      gctBOOL Decompress
**          Set if the render target needs to decompressed by issuing a resolve
**          onto itself.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_DisableTileStatus(
    IN gcoSURF Surface,
    IN gctBOOL Decompress
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    do
    {
        if (Surface->info.tileStatusNode.pool != gcvPOOL_UNKNOWN)
        {
            /* Disable tile status. */
            gcmERR_BREAK(
                gcoHARDWARE_DisableTileStatus(gcvNULL, &Surface->info,
                                              Decompress));
        }

        /* Success. */
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    /* Return the status. */
    gcmFOOTER();
    return status;
}


/*******************************************************************************
**
**  gcoSURF_FlushTileStatus
**
**  Flush tile status for the specified surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**      gctBOOL Decompress
**          Set if the render target needs to decompressed by issuing a resolve
**          onto itself.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_FlushTileStatus(
    IN gcoSURF Surface,
    IN gctBOOL Decompress
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    if (Surface->info.tileStatusNode.pool != gcvPOOL_UNKNOWN)
    {
        /* Disable tile status. */
        gcmONERROR(
            gcoHARDWARE_FlushTileStatus(gcvNULL, &Surface->info,
                                        Decompress));
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

#endif /* gcdENABLE_3D */

/*******************************************************************************
**
**  gcoSURF_GetSize
**
**  Get the size of an gcoSURF object.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**  OUTPUT:
**
**      gctUINT * Width
**          Pointer to variable that will receive the width of the gcoSURF
**          object.  If 'Width' is gcvNULL, no width information shall be returned.
**
**      gctUINT * Height
**          Pointer to variable that will receive the height of the gcoSURF
**          object.  If 'Height' is gcvNULL, no height information shall be returned.
**
**      gctUINT * Depth
**          Pointer to variable that will receive the depth of the gcoSURF
**          object.  If 'Depth' is gcvNULL, no depth information shall be returned.
*/
gceSTATUS
gcoSURF_GetSize(
    IN gcoSURF Surface,
    OUT gctUINT * Width,
    OUT gctUINT * Height,
    OUT gctUINT * Depth
    )
{
    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    if (Width != gcvNULL)
    {
        /* Return the width. */
        *Width = Surface->info.requestW;
    }

    if (Height != gcvNULL)
    {
        /* Return the height. */
        *Height = Surface->info.requestH;
    }

    if (Depth != gcvNULL)
    {
        /* Return the depth. */
        *Depth = Surface->info.requestD;
    }

    /* Success. */
    gcmFOOTER_ARG("*Width=%u *Height=%u *Depth=%u",
                  (Width  == gcvNULL) ? 0 : *Width,
                  (Height == gcvNULL) ? 0 : *Height,
                  (Depth  == gcvNULL) ? 0 : *Depth);
    return gcvSTATUS_OK;
}


/*******************************************************************************
**
**  gcoSURF_GetInfo
**
**  Get information of an gcoSURF object.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**      gceSURF_INFO_TYPE InfoType.
**          Information type need to be queried.
**
**  OUTPUT:
**
**      gctINT32 *Value
**          Pointer to variable that will receive return value.
**
*/
gceSTATUS
gcoSURF_GetInfo(
    IN gcoSURF Surface,
    IN gceSURF_INFO_TYPE InfoType,
    IN OUT gctINT32 *Value
    )
{
    gcmHEADER_ARG("Surface=0x%x InfoType=%d Value=0x%x", Surface, InfoType, Value);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    if (Value)
    {
        switch (InfoType)
        {
        case gcvSURF_INFO_LAYERSIZE:
            *Value = Surface->info.layerSize;
            break;

        case gcvSURF_INFO_SLICESIZE:
            *Value = Surface->info.sliceSize;
            break;

        default:
            gcmPRINT("Invalid surface info type query");
            break;
        }
    }

    gcmFOOTER_ARG("*value=%d", gcmOPT_VALUE(Value));
    return gcvSTATUS_OK;

}

/*******************************************************************************
**
**  gcoSURF_GetAlignedSize
**
**  Get the aligned size of an gcoSURF object.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**  OUTPUT:
**
**      gctUINT * Width
**          Pointer to variable that receives the aligned width of the gcoSURF
**          object.  If 'Width' is gcvNULL, no width information shall be returned.
**
**      gctUINT * Height
**          Pointer to variable that receives the aligned height of the gcoSURF
**          object.  If 'Height' is gcvNULL, no height information shall be
**          returned.
**
**      gctINT * Stride
**          Pointer to variable that receives the stride of the gcoSURF object.
**          If 'Stride' is gcvNULL, no stride information shall be returned.
*/
gceSTATUS
gcoSURF_GetAlignedSize(
    IN gcoSURF Surface,
    OUT gctUINT * Width,
    OUT gctUINT * Height,
    OUT gctINT * Stride
    )
{
    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    if (Width != gcvNULL)
    {
        /* Return the aligned width. */
        *Width = Surface->info.alignedW;
    }

    if (Height != gcvNULL)
    {
        /* Return the aligned height. */
        *Height = Surface->info.alignedH;
    }

    if (Stride != gcvNULL)
    {
        /* Return the stride. */
        *Stride = Surface->info.stride;
    }

    /* Success. */
    gcmFOOTER_ARG("*Width=%u *Height=%u *Stride=%d",
                  (Width  == gcvNULL) ? 0 : *Width,
                  (Height == gcvNULL) ? 0 : *Height,
                  (Stride == gcvNULL) ? 0 : *Stride);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoSURF_GetAlignment
**
**  INPUT:
**
**      gcoHAL Hal
**          Pointer to an gcoHAL object.
**
**      gceSURF_TYPE Type
**          Type of surface.
**
**      gceSURF_FORMAT Format
**          Format of surface.
**
**  OUTPUT:
**
**      gctUINT * addressAlignment
**          Pointer to the variable of address alignment.
**      gctUINT * xAlignmenet
**          Pointer to the variable of x Alignment.
**      gctUINT * yAlignment
**          Pointer to the variable of y Alignment.
*/
gceSTATUS
gcoSURF_GetAlignment(
    IN gceSURF_TYPE Type,
    IN gceSURF_FORMAT Format,
    OUT gctUINT * AddressAlignment,
    OUT gctUINT * XAlignment,
    OUT gctUINT * YAlignment
    )
{
    gceSTATUS status;
    gctUINT32 bitsPerPixel;
    gctUINT xAlign = (gcvSURF_TEXTURE == Type) ? 4 : 16;
    gctUINT yAlign = 4;
#if gcdENABLE_VG
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;
#endif

    gcmHEADER_ARG("Type=%d Format=%d", Type, Format);

    /* Compute alignment factors. */
    if (XAlignment != gcvNULL)
    {
        *XAlignment = xAlign;
    }

    if (YAlignment != gcvNULL)
    {
        *YAlignment = yAlign;
    }

#if gcdENABLE_VG
    gcmGETCURRENTHARDWARE(currentType);

    if (currentType == gcvHARDWARE_VG)
    {
        /* Compute bits per pixel. */
        gcmONERROR(gcoVGHARDWARE_ConvertFormat(gcvNULL,
                                             Format,
                                             &bitsPerPixel,
                                             gcvNULL));
    }
    else
#endif
    {
        /* Compute bits per pixel. */
        gcmONERROR(gcoHARDWARE_ConvertFormat(Format,
                                             &bitsPerPixel,
                                             gcvNULL));
    }

    if (AddressAlignment != gcvNULL)
    {
        *AddressAlignment = xAlign * yAlign * bitsPerPixel / 8;
    }

    gcmFOOTER_ARG("*XAlignment=0x%x  *YAlignment=0x%x *AddressAlignment=0x%x",
        gcmOPT_VALUE(XAlignment), gcmOPT_VALUE(YAlignment), gcmOPT_VALUE(AddressAlignment));
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

#if gcdENABLE_3D
/*******************************************************************************
**
**  gcoSURF_AlignResolveRect (need to modify)
**
**  INPUT:
**
**      gcoHAL Hal
**          Pointer to an gcoHAL object.
**
**      gceSURF_TYPE Type
**          Type of surface.
**
**      gceSURF_FORMAT Format
**          Format of surface.
**
**  OUTPUT:
**
**      gctUINT * addressAlignment
**          Pointer to the variable of address alignment.
**      gctUINT * xAlignmenet
**          Pointer to the variable of x Alignment.
**      gctUINT * yAlignment
**          Pointer to the variable of y Alignment.
*/
gceSTATUS
gcoSURF_AlignResolveRect(
    IN gcoSURF Surf,
    IN gcsPOINT_PTR RectOrigin,
    IN gcsPOINT_PTR RectSize,
    OUT gcsPOINT_PTR AlignedOrigin,
    OUT gcsPOINT_PTR AlignedSize
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Surf=0x%x RectOrigin=0x%x RectSize=0x%x",
        Surf, RectOrigin, RectSize);

    status = gcoHARDWARE_AlignResolveRect(&Surf->info, RectOrigin, RectSize,
                                         AlignedOrigin, AlignedSize);

    /* Success. */
    gcmFOOTER();

    return status;
}
#endif

/*******************************************************************************
**
**  gcoSURF_GetFormat
**
**  Get surface type and format.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**  OUTPUT:
**
**      gceSURF_TYPE * Type
**          Pointer to variable that receives the type of the gcoSURF object.
**          If 'Type' is gcvNULL, no type information shall be returned.
**
**      gceSURF_FORMAT * Format
**          Pointer to variable that receives the format of the gcoSURF object.
**          If 'Format' is gcvNULL, no format information shall be returned.
*/
gceSTATUS
gcoSURF_GetFormat(
    IN gcoSURF Surface,
    OUT gceSURF_TYPE * Type,
    OUT gceSURF_FORMAT * Format
    )
{
    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    if (Type != gcvNULL)
    {
        /* Return the surface type. */
        *Type = Surface->info.type;
    }

    if (Format != gcvNULL)
    {
        /* Return the surface format. */
        *Format = Surface->info.format;
    }

    /* Success. */
    gcmFOOTER_ARG("*Type=%d *Format=%d",
                  (Type   == gcvNULL) ? 0 : *Type,
                  (Format == gcvNULL) ? 0 : *Format);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoSURF_GetFormatInfo
**
**  Get surface format information.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**  OUTPUT:
**
**      gcsSURF_FORMAT_INFO_PTR * formatInfo
**          Pointer to variable that receives the format informationof the gcoSURF object.
**          If 'formatInfo' is gcvNULL, no format information shall be returned.
*/
gceSTATUS
gcoSURF_GetFormatInfo(
    IN gcoSURF Surface,
    OUT gcsSURF_FORMAT_INFO_PTR * formatInfo
    )
{
    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    if (formatInfo != gcvNULL)
    {
        /* Return the surface format. */
        *formatInfo = &Surface->info.formatInfo;
    }

    /* Success. */
    gcmFOOTER_ARG("*Format=0x%x",
                  (formatInfo == gcvNULL) ? 0 : *formatInfo);
    return gcvSTATUS_OK;
}



/*******************************************************************************
**
**  gcoSURF_GetPackedFormat
**
**  Get surface packed format for multiple-layer surface
**  gcvSURF_A32B32G32R32UI_2 ->gcvSURF_A32B32G32R32UI
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**  OUTPUT:
**
**
**      gceSURF_FORMAT * Format
**          Pointer to variable that receives the format of the gcoSURF object.
**          If 'Format' is gcvNULL, no format information shall be returned.
*/
gceSTATUS
gcoSURF_GetPackedFormat(
    IN gcoSURF Surface,
    OUT gceSURF_FORMAT * Format
    )
{
    gceSURF_FORMAT format;
    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    switch (Surface->info.format)
    {
    case gcvSURF_X16R16G16B16_2_A8R8G8B8:
        format = gcvSURF_X16R16G16B16;
        break;

    case gcvSURF_A16R16G16B16_2_A8R8G8B8:
        format = gcvSURF_A16R16G16B16;
        break;

    case gcvSURF_A32R32G32B32_2_G32R32F:
    case gcvSURF_A32R32G32B32_4_A8R8G8B8:
        format = gcvSURF_A32R32G32B32;
        break;

    case gcvSURF_S8D32F_1_G32R32F:
    case gcvSURF_S8D32F_2_A8R8G8B8:
        format = gcvSURF_S8D32F;
        break;

    case gcvSURF_X16B16G16R16F_2_A8R8G8B8:
        format = gcvSURF_X16B16G16R16F;
        break;

    case gcvSURF_A16B16G16R16F_2_A8R8G8B8:
        format = gcvSURF_A16B16G16R16F;
        break;

    case gcvSURF_A16B16G16R16F_2_G16R16F:
        format = gcvSURF_A16B16G16R16F;
        break;

    case gcvSURF_G32R32F_2_A8R8G8B8:
        format = gcvSURF_G32R32F;
        break;

    case gcvSURF_X32B32G32R32F_2_G32R32F:
    case gcvSURF_X32B32G32R32F_4_A8R8G8B8:
        format = gcvSURF_X32B32G32R32F;
        break;

    case gcvSURF_A32B32G32R32F_2_G32R32F:
    case gcvSURF_A32B32G32R32F_4_A8R8G8B8:
        format = gcvSURF_A32B32G32R32F;
        break;

    case gcvSURF_R16F_1_A4R4G4B4:
        format = gcvSURF_R16F;
        break;

    case gcvSURF_G16R16F_1_A8R8G8B8:
        format = gcvSURF_G16R16F;
        break;

    case gcvSURF_B16G16R16F_2_A8R8G8B8:
        format = gcvSURF_B16G16R16F;
        break;

    case gcvSURF_R32F_1_A8R8G8B8:
        format = gcvSURF_R32F;
        break;

    case gcvSURF_B32G32R32F_3_A8R8G8B8:
        format = gcvSURF_B32G32R32F;
        break;

    case gcvSURF_B10G11R11F_1_A8R8G8B8:
        format = gcvSURF_B10G11R11F;
        break;

    case gcvSURF_G32R32I_2_A8R8G8B8:
        format = gcvSURF_G32R32I;
        break;

    case gcvSURF_G32R32UI_2_A8R8G8B8:
        format = gcvSURF_G32R32UI;
        break;

    case gcvSURF_X16B16G16R16I_2_A8R8G8B8:
        format = gcvSURF_X16B16G16R16I;
        break;

    case gcvSURF_A16B16G16R16I_2_A8R8G8B8:
        format = gcvSURF_A16B16G16R16I;
        break;

    case gcvSURF_X16B16G16R16UI_2_A8R8G8B8:
        format = gcvSURF_X16B16G16R16UI;
        break;

    case gcvSURF_A16B16G16R16UI_2_A8R8G8B8:
        format = gcvSURF_A16B16G16R16UI;
        break;

    case gcvSURF_X32B32G32R32I_2_G32R32I:
    case gcvSURF_X32B32G32R32I_3_A8R8G8B8:
        format = gcvSURF_X32B32G32R32I;
        break;

    case gcvSURF_A32B32G32R32I_2_G32R32I:
    case gcvSURF_A32B32G32R32I_4_A8R8G8B8:
        format = gcvSURF_A32B32G32R32I;
        break;

    case gcvSURF_X32B32G32R32UI_2_G32R32UI:
    case gcvSURF_X32B32G32R32UI_3_A8R8G8B8:
        format = gcvSURF_X32B32G32R32UI;
        break;

    case gcvSURF_A32B32G32R32UI_2_G32R32UI:
    case gcvSURF_A32B32G32R32UI_4_A8R8G8B8:
        format = gcvSURF_A32B32G32R32UI;
        break;

    case gcvSURF_A2B10G10R10UI_1_A8R8G8B8:
        format = gcvSURF_A2B10G10R10UI;
        break;

    case gcvSURF_A8B8G8R8I_1_A8R8G8B8:
        format = gcvSURF_A8B8G8R8I;
        break;

    case gcvSURF_A8B8G8R8UI_1_A8R8G8B8:
        format = gcvSURF_A8B8G8R8UI;
        break;

    case gcvSURF_R8I_1_A4R4G4B4:
        format = gcvSURF_R8I;
        break;

    case gcvSURF_R8UI_1_A4R4G4B4:
        format = gcvSURF_R8UI;
        break;

    case gcvSURF_R16I_1_A4R4G4B4:
        format = gcvSURF_R16I;
        break;

    case gcvSURF_R16UI_1_A4R4G4B4:
        format = gcvSURF_R16UI;
        break;

    case gcvSURF_R32I_1_A8R8G8B8:
        format = gcvSURF_R32I;
        break;

    case gcvSURF_R32UI_1_A8R8G8B8:
        format = gcvSURF_R32UI;
        break;

    case gcvSURF_X8R8I_1_A4R4G4B4:
        format = gcvSURF_X8R8I;
        break;

    case gcvSURF_X8R8UI_1_A4R4G4B4:
        format = gcvSURF_X8R8UI;
        break;

    case gcvSURF_G8R8I_1_A4R4G4B4:
        format = gcvSURF_G8R8I;
        break;

    case gcvSURF_G8R8UI_1_A4R4G4B4:
        format = gcvSURF_G8R8UI;
        break;

    case gcvSURF_X16R16I_1_A4R4G4B4:
        format = gcvSURF_X16R16I;
        break;

    case gcvSURF_X16R16UI_1_A4R4G4B4:
        format = gcvSURF_X16R16UI;
        break;

    case gcvSURF_G16R16I_1_A8R8G8B8:
        format = gcvSURF_G16R16I;
        break;

    case gcvSURF_G16R16UI_1_A8R8G8B8:
        format = gcvSURF_G16R16UI;
        break;
    case gcvSURF_X32R32I_1_A8R8G8B8:
        format = gcvSURF_X32R32I;
        break;

    case gcvSURF_X32R32UI_1_A8R8G8B8:
        format = gcvSURF_X32R32UI;
        break;

    case gcvSURF_X8G8R8I_1_A4R4G4B4:
        format = gcvSURF_X8G8R8I;
        break;

    case gcvSURF_X8G8R8UI_1_A4R4G4B4:
        format = gcvSURF_X8G8R8UI;
        break;

    case gcvSURF_B8G8R8I_1_A8R8G8B8:
        format = gcvSURF_B8G8R8I;
        break;

    case gcvSURF_B8G8R8UI_1_A8R8G8B8:
        format = gcvSURF_B8G8R8UI;
        break;

    case gcvSURF_B16G16R16I_2_A8R8G8B8:
        format = gcvSURF_B16G16R16I;
        break;

    case gcvSURF_B16G16R16UI_2_A8R8G8B8:
        format = gcvSURF_B16G16R16UI;
        break;

    case gcvSURF_B32G32R32I_3_A8R8G8B8:
        format = gcvSURF_B32G32R32I;
        break;

    case gcvSURF_B32G32R32UI_3_A8R8G8B8:
        format = gcvSURF_B32G32R32UI;
        break;

    default:
        format = Surface->info.format;
        break;
    }

    if (Format != gcvNULL)
    {
        /* Return the surface format. */
        *Format = format;
    }

    /* Success. */
    gcmFOOTER_ARG("*Format=%d",
                  (Format == gcvNULL) ? 0 : *Format);
    return gcvSTATUS_OK;
}


/*******************************************************************************
**
**  gcoSURF_GetTiling
**
**  Get surface tiling.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**  OUTPUT:
**
**      gceTILING * Tiling
**          Pointer to variable that receives the tiling of the gcoSURF object.
*/
gceSTATUS
gcoSURF_GetTiling(
    IN gcoSURF Surface,
    OUT gceTILING * Tiling
    )
{
    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    if (Tiling != gcvNULL)
    {
        /* Return the surface tiling. */
        *Tiling = Surface->info.tiling;
    }

    /* Success. */
    gcmFOOTER_ARG("*Tiling=%d",
                  (Tiling == gcvNULL) ? 0 : *Tiling);
    return gcvSTATUS_OK;
}


/*******************************************************************************
**
**  gcoSURF_GetBottomBufferOffset
**
**  Get bottom buffer offset for split tiled surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**  OUTPUT:
**
**      gctUINT_PTR BottomBufferOffset
**          Pointer to variable that receives the offset value.
*/
gceSTATUS
gcoSURF_GetBottomBufferOffset(
    IN gcoSURF Surface,
    OUT gctUINT_PTR BottomBufferOffset
    )
{
    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    if (BottomBufferOffset != gcvNULL)
    {
        /* Return the surface tiling. */
        *BottomBufferOffset = Surface->info.bottomBufferOffset;
    }

    /* Success. */
    gcmFOOTER_ARG("*BottomBufferOffset=%d",
                  (BottomBufferOffset == gcvNULL) ? 0 : *BottomBufferOffset);
    return gcvSTATUS_OK;
}


#if gcdENABLE_3D
gceSTATUS
gcoSURF_SetSharedLock(
    IN gcoSURF Surface,
    IN gctPOINTER SharedLock
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Surface=0x%x SharedLock=0x%x", Surface, SharedLock);

    if (Surface)
    {
        status = gcsSURF_NODE_SetSharedLock(&Surface->info.node, SharedLock);
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
gcoSURF_GetFence(
    IN gcoSURF Surface,
    IN gceFENCE_TYPE Type
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Surface=0x%x Type=%d", Surface, Type);

    if (Surface)
    {
        status = gcsSURF_NODE_GetFence(&Surface->info.node, Type);
    }

    gcmFOOTER();
    return status;

}

gceSTATUS
gcoSURF_WaitFence(
    IN gcoSURF Surface
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Surface=0x%x", Surface);

    if (Surface)
    {
        status = gcsSURF_NODE_WaitFence(&Surface->info.node, gcvFENCE_TYPE_ALL);
    }

    gcmFOOTER();
    return status;

}
#endif

/*******************************************************************************
**
**  gcoSURF_Lock
**
**  Lock the surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**  OUTPUT:
**
**      gctUINT32 * Address
**          Physical address array of the surface:
**          For YV12, Address[0] is for Y channel,
**                    Address[1] is for U channel and
**                    Address[2] is for V channel;
**          For I420, Address[0] is for Y channel,
**                    Address[1] is for U channel and
**                    Address[2] is for V channel;
**          For NV12, Address[0] is for Y channel and
**                    Address[1] is for UV channel;
**          For all other formats, only Address[0] is used to return the
**          physical address.
**
**      gctPOINTER * Memory
**          Logical address array of the surface:
**          For YV12, Memory[0] is for Y channel,
**                    Memory[1] is for V channel and
**                    Memory[2] is for U channel;
**          For I420, Memory[0] is for Y channel,
**                    Memory[1] is for U channel and
**                    Memory[2] is for V channel;
**          For NV12, Memory[0] is for Y channel and
**                    Memory[1] is for UV channel;
**          For all other formats, only Memory[0] is used to return the logical
**          address.
*/
gceSTATUS
gcoSURF_Lock(
    IN gcoSURF Surface,
    OPTIONAL OUT gctUINT32 * Address,
    OPTIONAL OUT gctPOINTER * Memory
    )
{
    gceSTATUS status;
    gctUINT32 address = 0, address2 = 0, address3 = 0;
    gctUINT8_PTR logical = gcvNULL, logical2 = gcvNULL, logical3 = gcvNULL;
    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Lock the surface. */
    gcmONERROR(_Lock(Surface));

    gcmGETHARDWAREADDRESS(Surface->info.node, address);

    if (Surface->info.flags & gcvSURF_FLAG_MULTI_NODE)
    {
        /* Multiple nodes. */
        gcmGETHARDWAREADDRESS(Surface->info.node2, address2);
        gcmGETHARDWAREADDRESS(Surface->info.node3, address3);

        logical = Surface->info.node.logical;

        /* Calculate buffer address. */
        switch (Surface->info.format)
        {
        case gcvSURF_YV12:
        case gcvSURF_I420:
            Surface->info.node.count = 3;

            logical2 = Surface->info.node2.logical;

            logical3 = Surface->info.node3.logical;
            break;

        case gcvSURF_NV12:
        case gcvSURF_NV21:
        case gcvSURF_NV16:
        case gcvSURF_NV61:
            Surface->info.node.count = 2;

            logical2 = Surface->info.node2.logical;
            break;

        default:
            Surface->info.node.count = 1;
            break;
        }
    }
    else
    {
        /* Determine surface addresses. */
        logical = Surface->info.node.logical;

        switch (Surface->info.format)
        {
        case gcvSURF_YV12:
        case gcvSURF_I420:
            Surface->info.node.count = 3;

            logical2 = Surface->info.node.logical
                     + Surface->info.uOffset;

            address2 = Surface->info.node.physical2
                     = address + Surface->info.uOffset;

            logical3 = Surface->info.node.logical
                     + Surface->info.vOffset;

            address3 = Surface->info.node.physical3
                     = address + Surface->info.vOffset;
            break;

        case gcvSURF_NV12:
        case gcvSURF_NV21:
        case gcvSURF_NV16:
        case gcvSURF_NV61:
            Surface->info.node.count = 2;

            logical2 = Surface->info.node.logical
                     + Surface->info.uOffset;

            address2 = Surface->info.node.physical2
                     = address + Surface->info.uOffset;
            break;

        default:
            Surface->info.node.count = 1;
            break;
        }
    }

    /* Set result. */
    if (Address != gcvNULL)
    {
        switch (Surface->info.node.count)
        {
        case 3:
            Address[2] = address3;

            /* FALLTHROUGH */
        case 2:
            Address[1] = address2;

            /* FALLTHROUGH */
        case 1:
            Address[0] = address;

            /* FALLTHROUGH */
        default:
            break;
        }
    }

    if (Memory != gcvNULL)
    {
        switch (Surface->info.node.count)
        {
        case 3:
            Memory[2] = logical3;

            /* FALLTHROUGH */
        case 2:
            Memory[1] = logical2;

            /* FALLTHROUGH */
        case 1:
            Memory[0] = logical;

            /* FALLTHROUGH */
        default:
            break;
        }
    }

    /* Success. */
    gcmFOOTER_ARG("*Address=%08X *Memory=0x%x",
                  (Address == gcvNULL) ? 0 : *Address,
                  (Memory  == gcvNULL) ? gcvNULL : *Memory);
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_Unlock
**
**  Unlock the surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**      gctPOINTER Memory
**          Pointer to mapped memory.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_Unlock(
    IN gcoSURF Surface,
    IN gctPOINTER Memory
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x Memory=0x%x", Surface, Memory);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Unlock the surface. */
    gcmONERROR(_Unlock(Surface));

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_Fill
**
**  Fill surface with specified value.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**      gcsPOINT_PTR Origin
**          Pointer to the origin of the area to be filled.
**          Assumed to (0, 0) if gcvNULL is given.
**
**      gcsSIZE_PTR Size
**          Pointer to the size of the area to be filled.
**          Assumed to the size of the surface if gcvNULL is given.
**
**      gctUINT32 Value
**          Value to be set.
**
**      gctUINT32 Mask
**          Value mask.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_Fill(
    IN gcoSURF Surface,
    IN gcsPOINT_PTR Origin,
    IN gcsSIZE_PTR Size,
    IN gctUINT32 Value,
    IN gctUINT32 Mask
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

/*******************************************************************************
**
**  gcoSURF_Blend
**
**  Alpha blend two surfaces together.
**
**  INPUT:
**
**      gcoSURF SrcSurface
**          Pointer to the source gcoSURF object.
**
**      gcoSURF DestSurface
**          Pointer to the destination gcoSURF object.
**
**      gcsPOINT_PTR SrcOrigin
**          The origin within the source.
**          If gcvNULL is specified, (0, 0) origin is assumed.
**
**      gcsPOINT_PTR DestOrigin
**          The origin within the destination.
**          If gcvNULL is specified, (0, 0) origin is assumed.
**
**      gcsSIZE_PTR Size
**          The size of the area to be blended.
**
**      gceSURF_BLEND_MODE Mode
**          One of the blending modes.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_Blend(
    IN gcoSURF SrcSurface,
    IN gcoSURF DestSurface,
    IN gcsPOINT_PTR SrcOrigin,
    IN gcsPOINT_PTR DestOrigin,
    IN gcsSIZE_PTR Size,
    IN gceSURF_BLEND_MODE Mode
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

#if gcdENABLE_3D
/******************************************************************************\
********************************* Support Code *********************************
\******************************************************************************/

/*******************************************************************************
**
**  _ConvertValue
**
**  Convert a value to a 32-bit color or depth value.
**
**  INPUT:
**
**      gceVALUE_TYPE ValueType
**          Type of value.
**
**      gcuVALUE Value
**          Value data.
**
**      gctUINT Bits
**          Number of bits used in output.
**
**  OUTPUT:
**
**      Nothing.
**
**  RETURNS:
**
**      gctUINT32
**          Converted or casted value.
*/
extern gctFLOAT _LinearToNonLinearConv(gctFLOAT lFloat);

static gctUINT32
_ConvertValue(
    IN gceVALUE_TYPE ValueType,
    IN gcuVALUE Value,
    IN gctUINT Bits
    )
{
    /* Setup maximum value. */
    gctUINT uMaxValue = (Bits == 32) ? ~0 : ((1 << Bits) - 1);
    gctUINT32 tmpRet = 0;
    gcmASSERT(Bits <= 32);

    /*
    ** Data conversion clear path:
    ** Here we need handle clamp here coz client driver just pass clear value down.
    ** Now we plan to support INT/UINT RT, floating depth or floating RT later, we need set gcvVALUE_FLAG_NEED_CLAMP
    ** base on surface format.
    */
    switch (ValueType & ~gcvVALUE_FLAG_MASK)
    {
    case gcvVALUE_UINT:
        return ((Value.uintValue > uMaxValue) ? uMaxValue : Value.uintValue);

    case gcvVALUE_INT:
        {
            gctUINT32 mask = (Bits == 32) ? ~0 : ((1 << Bits) - 1);
            gctINT iMinValue = (Bits == 32)? (1 << (Bits-1))   :((~( 1 << (Bits -1))) + 1);
            gctINT iMaxValue = (Bits == 32)? (~(1 << (Bits-1))): ((1 << (Bits - 1)) - 1);
            return gcmCLAMP(Value.intValue, iMinValue, iMaxValue) & mask;
        }

    case gcvVALUE_FIXED:
        {
            gctFIXED_POINT tmpFixedValue = Value.fixedValue;
            if (ValueType & gcvVALUE_FLAG_UNSIGNED_DENORM)
            {
                tmpFixedValue = gcmFIXEDCLAMP_0_TO_1(tmpFixedValue);
                /* Convert fixed point (0.0 - 1.0) into color value. */
                return (gctUINT32) (((gctUINT64)uMaxValue * tmpFixedValue) >> 16);
            }
            else if (ValueType & gcvVALUE_FLAG_SIGNED_DENORM)
            {
                gcmASSERT(0);
                return 0;
            }
            else
            {
                gcmASSERT(0);
                return 0;
            }
        }
        break;

    case gcvVALUE_FLOAT:
        {
            gctFLOAT tmpFloat = Value.floatValue;
            gctFLOAT sFloat = tmpFloat;

            if (ValueType & gcvVALUE_FLAG_GAMMAR)
            {
                gcmASSERT ((ValueType & gcvVALUE_FLAG_FLOAT_TO_FLOAT16) == 0);

                sFloat = _LinearToNonLinearConv(tmpFloat);
            }

            if (ValueType & gcvVALUE_FLAG_FLOAT_TO_FLOAT16)
            {
                gcmASSERT ((ValueType & gcvVALUE_FLAG_GAMMAR) == 0);
                tmpRet = (gctUINT32)gcoMATH_FloatToFloat16(*(gctUINT32*)&tmpFloat);
                return tmpRet;
            }
            else if (ValueType & gcvVALUE_FLAG_UNSIGNED_DENORM)
            {
                sFloat = gcmFLOATCLAMP_0_TO_1(sFloat);
                /* Convert floating point (0.0 - 1.0) into color value. */
                tmpRet = gcoMATH_Float2UInt(gcoMATH_Multiply(gcoMATH_UInt2Float(uMaxValue), sFloat));
                return tmpRet > uMaxValue ? uMaxValue : tmpRet;
            }
            else if (ValueType & gcvVALUE_FLAG_SIGNED_DENORM)
            {
                gcmASSERT(0);
                return 0;
            }
            else
            {
                tmpRet = *(gctUINT32*)&sFloat;
                return tmpRet > uMaxValue ? uMaxValue : tmpRet;
            }
        }
        break;

    default:
        return 0;
        break;
    }
}
gctUINT32
_ByteMaskToBitMask(
    gctUINT32 ClearMask
    )
{
    gctUINT32 clearMask = 0;

    /* Byte mask to bit mask. */
    if (ClearMask & 0x1)
    {
        clearMask |= 0xFF;
    }

    if (ClearMask & 0x2)
    {
        clearMask |= (0xFF << 8);
    }

    if (ClearMask & 0x4)
    {
        clearMask |= (0xFF << 16);
    }

    if (ClearMask & 0x8)
    {
        clearMask |= (0xFF << 24);
    }

    return clearMask;
}

#define CHANNEL_BITMASK(x) ((mask##x & ((1 << width##x) - 1)) << start##x)

static gceSTATUS
_ComputeClear(
    IN gcsSURF_INFO_PTR Surface,
    IN gcsSURF_CLEAR_ARGS_PTR ClearArgs,
    IN gctUINT32 LayerIndex
    )
{
    gceSTATUS status;
    gctUINT32 maskRed, maskGreen, maskBlue, maskAlpha;
    gctUINT32 clearBitMask32, tempMask;
    gceVALUE_TYPE clearValueType;
    gcsSURF_FORMAT_INFO_PTR info = &Surface->formatInfo;
    gctUINT32 widthRed    = info->u.rgba.red.width;
    gctUINT32 widthGreen  = info->u.rgba.green.width;
    gctUINT32 widthBlue   = info->u.rgba.blue.width;
    gctUINT32 widthAlpha  = info->u.rgba.alpha.width;
    gctUINT32 startRed    = info->u.rgba.red.start & 0x1f;
    gctUINT32 startGreen  = info->u.rgba.green.start & 0x1f;
    gctUINT32 startBlue   = info->u.rgba.blue.start & 0x1f;
    gctUINT32 startAlpha  = info->u.rgba.alpha.start & 0x1f;

    gcmHEADER_ARG("Surface=0x%x ClearArgs=0x%x LayerIndex=0x%d", Surface, ClearArgs, LayerIndex);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Surface != gcvNULL);
    gcmDEBUG_VERIFY_ARGUMENT(ClearArgs != gcvNULL);

    /* Test for clearing render target. */
    if (ClearArgs->flags & gcvCLEAR_COLOR)
    {
        Surface->clearMask[LayerIndex] = ((ClearArgs->colorMask & 0x1) << 2) | /* Red   */
                                          (ClearArgs->colorMask & 0x2)       | /* Green */
                                         ((ClearArgs->colorMask & 0x4) >> 2) | /* Blue  */
                                          (ClearArgs->colorMask & 0x8);        /* Alpha */
        maskRed   = (ClearArgs->colorMask & 0x1) ? 0xFFFFFFFF: 0;
        maskGreen = (ClearArgs->colorMask & 0x2) ? 0xFFFFFFFF: 0;
        maskBlue  = (ClearArgs->colorMask & 0x4) ? 0xFFFFFFFF: 0;
        maskAlpha = (ClearArgs->colorMask & 0x8) ? 0xFFFFFFFF: 0;

        clearBitMask32 =
            CHANNEL_BITMASK(Red) | CHANNEL_BITMASK(Green) | CHANNEL_BITMASK(Blue) | CHANNEL_BITMASK(Alpha);

        tempMask = (info->bitsPerPixel < 32 ) ?
                        ((gctUINT32)0xFFFFFFFF) >> (32 - info->bitsPerPixel) : 0xFFFFFFFF;

        clearBitMask32 &= tempMask;

        /* Dispatch on render target format. */
        switch (Surface->format)
        {
        case gcvSURF_X4R4G4B4: /* 12-bit RGB color without alpha channel. */
        case gcvSURF_A4R4G4B4: /* 12-bit RGB color with alpha channel. */
            gcmASSERT(LayerIndex == 0);
            clearValueType = (gceVALUE_TYPE) (ClearArgs->color.valueType | gcvVALUE_FLAG_UNSIGNED_DENORM);

            Surface->clearValue[0]
                /* Convert red into 4-bit. */
                = (_ConvertValue(clearValueType,
                                 ClearArgs->color.r, 4) << 8)
                /* Convert green into 4-bit. */
                | (_ConvertValue(clearValueType,
                                 ClearArgs->color.g, 4) << 4)
                /* Convert blue into 4-bit. */
                | _ConvertValue(clearValueType,
                                ClearArgs->color.b, 4)
                /* Convert alpha into 4-bit. */
                | (_ConvertValue(clearValueType,
                                 ClearArgs->color.a, 4) << 12);

            /* Expand 16-bit color into 32-bit color. */
            Surface->clearValue[0] |= Surface->clearValue[0] << 16;
            Surface->clearValueUpper[0] = Surface->clearValue[0];
            Surface->clearBitMask[LayerIndex] = clearBitMask32;
            Surface->clearBitMask[LayerIndex] |= Surface->clearBitMask[LayerIndex] << 16;
            Surface->clearBitMaskUpper[LayerIndex] = Surface->clearBitMask[LayerIndex];
            break;

        case gcvSURF_X1R5G5B5: /* 15-bit RGB color without alpha channel. */
        case gcvSURF_A1R5G5B5: /* 15-bit RGB color with alpha channel. */
            gcmASSERT(LayerIndex == 0);
            clearValueType = (gceVALUE_TYPE) (ClearArgs->color.valueType | gcvVALUE_FLAG_UNSIGNED_DENORM);

            Surface->clearValue[0]
                /* Convert red into 5-bit. */
                = (_ConvertValue(clearValueType,
                                 ClearArgs->color.r, 5) << 10)
                /* Convert green into 5-bit. */
                | (_ConvertValue(clearValueType,
                                 ClearArgs->color.g, 5) << 5)
                /* Convert blue into 5-bit. */
                | _ConvertValue(clearValueType,
                                ClearArgs->color.b, 5)
                /* Convert alpha into 1-bit. */
                | (_ConvertValue(clearValueType,
                                 ClearArgs->color.a, 1) << 15);

            /* Expand 16-bit color into 32-bit color. */
            Surface->clearValue[0] |= Surface->clearValue[0] << 16;
            Surface->clearValueUpper[0] = Surface->clearValue[0];
            Surface->clearBitMask[LayerIndex] = clearBitMask32;
            Surface->clearBitMask[LayerIndex] |= Surface->clearBitMask[LayerIndex] << 16;
            Surface->clearBitMaskUpper[LayerIndex] = Surface->clearBitMask[LayerIndex];
            break;

        case gcvSURF_R5G6B5: /* 16-bit RGB color without alpha channel. */
            gcmASSERT(LayerIndex == 0);
            clearValueType = (ClearArgs->color.valueType | gcvVALUE_FLAG_UNSIGNED_DENORM);

            Surface->clearValue[0]
                /* Convert red into 5-bit. */
                = (_ConvertValue(clearValueType,
                                 ClearArgs->color.r, 5) << 11)
                /* Convert green into 6-bit. */
                | (_ConvertValue(clearValueType,
                                 ClearArgs->color.g, 6) << 5)
                /* Convert blue into 5-bit. */
                | _ConvertValue(clearValueType,
                                ClearArgs->color.b, 5);

            /* Expand 16-bit color into 32-bit color. */
            Surface->clearValue[0] |= Surface->clearValue[0] << 16;
            Surface->clearValueUpper[0] = Surface->clearValue[0];
            Surface->clearBitMask[LayerIndex] = clearBitMask32;
            Surface->clearBitMask[LayerIndex] |= Surface->clearBitMask[LayerIndex] << 16;
            Surface->clearBitMaskUpper[LayerIndex] = Surface->clearBitMask[LayerIndex];
            break;

        case gcvSURF_YUY2:
            {
                gctUINT8 r, g, b;
                gctUINT8 y, u, v;
                gcmASSERT(LayerIndex == 0);
                /* Query YUY2 render target support. */
                if (gcoHAL_IsFeatureAvailable(gcvNULL,
                                              gcvFEATURE_YUY2_RENDER_TARGET)
                    != gcvSTATUS_TRUE)
                {
                    /* No, reject. */
                    gcmFOOTER_NO();
                    return gcvSTATUS_INVALID_ARGUMENT;
                }
                clearValueType = (ClearArgs->color.valueType | gcvVALUE_FLAG_UNSIGNED_DENORM);

                /* Get 8-bit RGB values. */
                r = (gctUINT8) _ConvertValue(clearValueType,
                                  ClearArgs->color.r, 8);

                g = (gctUINT8) _ConvertValue(clearValueType,
                                  ClearArgs->color.g, 8);

                b = (gctUINT8) _ConvertValue(clearValueType,
                                  ClearArgs->color.b, 8);

                /* Convert to YUV. */
                gcoHARDWARE_RGB2YUV(r, g, b, &y, &u, &v);

                /* Set the clear value. */
                Surface->clearValueUpper[0] =
                Surface->clearValue[0] =  ((gctUINT32) y)
                                     | (((gctUINT32) u) <<  8)
                                     | (((gctUINT32) y) << 16)
                                     | (((gctUINT32) v) << 24);
                Surface->clearBitMask[LayerIndex] = clearBitMask32;
                Surface->clearBitMaskUpper[LayerIndex] = clearBitMask32;
            }
            break;

        case gcvSURF_X8R8G8B8: /* 24-bit RGB without alpha channel. */
        case gcvSURF_A8R8G8B8: /* 24-bit RGB with alpha channel. */
            gcmASSERT(LayerIndex == 0);
            clearValueType = (gceVALUE_TYPE) (ClearArgs->color.valueType | gcvVALUE_FLAG_UNSIGNED_DENORM);

            Surface->clearValue[0]
                /* Convert red to 8-bit. */
                = (_ConvertValue(clearValueType,
                                 ClearArgs->color.r, 8) << 16)
                /* Convert green to 8-bit. */
                | (_ConvertValue(clearValueType,
                                 ClearArgs->color.g, 8) << 8)
                /* Convert blue to 8-bit. */
                |  _ConvertValue(clearValueType,
                                ClearArgs->color.b, 8)
                    /* Convert alpha to 8-bit. */
                | (_ConvertValue(clearValueType,
                                 ClearArgs->color.a, 8) << 24);

            Surface->clearValueUpper[0] = Surface->clearValue[0];
            Surface->clearBitMask[LayerIndex] = clearBitMask32;
            Surface->clearBitMaskUpper[LayerIndex] = clearBitMask32;
            break;

        case gcvSURF_R8_1_X8R8G8B8: /* 32-bit R8 without bga channel. */
            gcmASSERT(LayerIndex == 0);
            clearValueType = (gceVALUE_TYPE) (ClearArgs->color.valueType | gcvVALUE_FLAG_UNSIGNED_DENORM);

            Surface->clearValue[0]
                /* Convert red to 8-bit. */
                = (_ConvertValue(clearValueType,
                                 ClearArgs->color.r, 8) << 16)
                /* Convert green to 8-bit. */
                | 0xFF000000;

            Surface->clearValueUpper[0] = Surface->clearValue[0];
            Surface->clearBitMask[LayerIndex] = clearBitMask32;
            Surface->clearBitMaskUpper[LayerIndex] = clearBitMask32;
            break;

        case gcvSURF_R8I_1_A4R4G4B4:  /* 32-bit R8I  */
        case gcvSURF_R8UI_1_A4R4G4B4: /* 32-bit R8UI */
            gcmASSERT(LayerIndex == 0);
            clearValueType = ClearArgs->color.valueType;;

            Surface->clearValue[0]
                = _ConvertValue(clearValueType, ClearArgs->color.r, 8);

            Surface->clearValueUpper[0] = Surface->clearValue[0];
            Surface->clearBitMask[LayerIndex] = clearBitMask32;
            Surface->clearBitMaskUpper[LayerIndex] = clearBitMask32;
            break;

        case gcvSURF_G8R8_1_X8R8G8B8: /* 32-bit R8 without bga channel. */
            gcmASSERT(LayerIndex == 0);
            clearValueType = (gceVALUE_TYPE) (ClearArgs->color.valueType | gcvVALUE_FLAG_UNSIGNED_DENORM);

            Surface->clearValue[0]
                /* Convert red to 8-bit. */
                = (_ConvertValue(clearValueType,
                                 ClearArgs->color.r, 8) << 16)
                /* Convert green to 8-bit. */
                | (_ConvertValue(clearValueType,
                                 ClearArgs->color.g, 8) << 8)
                /* Convert green to 8-bit. */
                | 0xFF000000;

            Surface->clearValueUpper[0] = Surface->clearValue[0];
            Surface->clearBitMask[LayerIndex] = clearBitMask32;
            Surface->clearBitMaskUpper[LayerIndex] = clearBitMask32;
            break;

         case gcvSURF_G8R8:
             /* 16-bit RG color without alpha channel. */
             gcmASSERT(LayerIndex == 0);
             clearValueType = (gceVALUE_TYPE) (ClearArgs->color.valueType | gcvVALUE_FLAG_UNSIGNED_DENORM);
             Surface->clearValue[0]
                 /* Convert red to 8-bit. */
                 = (_ConvertValue(clearValueType,
                                  ClearArgs->color.r, 8))
                 /* Convert green to 8-bit. */
                 | (_ConvertValue(clearValueType,
                                  ClearArgs->color.g, 8) << 8);

             /* Expand 16-bit color into 32-bit color. */
             Surface->clearValue[0] |= Surface->clearValue[0] << 16;
             Surface->clearValueUpper[0] = Surface->clearValue[0];
             Surface->clearBitMask[LayerIndex] = clearBitMask32;
             Surface->clearBitMask[LayerIndex] |= Surface->clearBitMask[LayerIndex] << 16;
             Surface->clearBitMaskUpper[LayerIndex] = Surface->clearBitMask[LayerIndex];
             break;

         case gcvSURF_A8_SBGR8: /* 24-bit RGB with alpha channel. */
             gcmASSERT(LayerIndex == 0);
             clearValueType = (gceVALUE_TYPE)   (ClearArgs->color.valueType   |
                                                gcvVALUE_FLAG_UNSIGNED_DENORM |
                                                gcvVALUE_FLAG_GAMMAR);
             Surface->clearValue[0]
                /* Convert red to 8-bit. */
                = (_ConvertValue(clearValueType,
                                 ClearArgs->color.r, 8))
                 /* Convert green to 8-bit. */
                 | (_ConvertValue(clearValueType,
                                  ClearArgs->color.g, 8) << 8)
                 /* Convert blue to 8-bit. */
                 | (_ConvertValue(clearValueType,
                                  ClearArgs->color.b, 8) << 16);

             clearValueType &= ~gcvVALUE_FLAG_GAMMAR;
             Surface->clearValue[0] |= (_ConvertValue(clearValueType,
                                                    ClearArgs->color.a, 8) << 24);
             Surface->clearValueUpper[0] = Surface->clearValue[0];
             Surface->clearBitMask[LayerIndex] = clearBitMask32;
             Surface->clearBitMaskUpper[LayerIndex] = clearBitMask32;
             break;

         case gcvSURF_A8_SRGB8: /* 24-bit RGB with alpha channel. */
             gcmASSERT(LayerIndex == 0);
             clearValueType = (gceVALUE_TYPE)   (ClearArgs->color.valueType   |
                                                gcvVALUE_FLAG_UNSIGNED_DENORM |
                                                gcvVALUE_FLAG_GAMMAR);

             Surface->clearValue[0]
                /* Convert red to 8-bit. */
                = (_ConvertValue(clearValueType,
                                 ClearArgs->color.r, 8) << 16)
                 /* Convert green to 8-bit. */
                 | (_ConvertValue(clearValueType,
                                  ClearArgs->color.g, 8) << 8)
                 /* Convert blue to 8-bit. */
                 | (_ConvertValue(clearValueType,
                                  ClearArgs->color.b, 8));

             clearValueType &= ~gcvVALUE_FLAG_GAMMAR;
             Surface->clearValue[0] |= (_ConvertValue(clearValueType,
                                                    ClearArgs->color.a, 8) << 24);
             Surface->clearValueUpper[0] = Surface->clearValue[0];
             Surface->clearBitMask[LayerIndex] = clearBitMask32;
             Surface->clearBitMaskUpper[LayerIndex] = clearBitMask32;
             break;


         case gcvSURF_X2R10G10B10:
         case gcvSURF_A2R10G10B10:
             gcmASSERT(LayerIndex == 0);
             clearValueType = (ClearArgs->color.valueType | gcvVALUE_FLAG_UNSIGNED_DENORM);
             Surface->clearValueUpper[0] =
             Surface->clearValue[0]
                 /* Convert red to 10-bit. */
                 = (_ConvertValue(clearValueType,
                                  ClearArgs->color.r, 10) << 20)
                 /* Convert green to 10-bit. */
                 | (_ConvertValue(clearValueType,
                                  ClearArgs->color.g, 10) << 10)
                 /* Convert blue to 10-bit. */
                 | _ConvertValue(clearValueType,
                                 ClearArgs->color.b, 10)
                 /* Convert alpha to 2-bit. */
                 | (_ConvertValue(clearValueType,
                                  ClearArgs->color.a, 2) << 30);
             Surface->clearBitMask[LayerIndex] = clearBitMask32;
             Surface->clearBitMaskUpper[LayerIndex] = clearBitMask32;

             break;

         case gcvSURF_X2B10G10R10:
         case gcvSURF_A2B10G10R10:
             gcmASSERT(LayerIndex == 0);
             clearValueType = (ClearArgs->color.valueType| gcvVALUE_FLAG_UNSIGNED_DENORM);
             Surface->clearValueUpper[0] =
             Surface->clearValue[0]
                 /* Convert red to 10-bit. */
                 = _ConvertValue(clearValueType,
                                 ClearArgs->color.r, 10)
                 /* Convert green to 10-bit. */
                 | (_ConvertValue(clearValueType,
                                  ClearArgs->color.g, 10) << 10)
                 /* Convert blue to 10-bit. */
                 | (_ConvertValue(clearValueType,
                                  ClearArgs->color.b, 10) << 20)
                 /* Convert alpha to 2-bit. */
                 | (_ConvertValue(clearValueType,
                                  ClearArgs->color.a, 2) << 30);
             Surface->clearBitMask[LayerIndex] = clearBitMask32;
             Surface->clearBitMaskUpper[LayerIndex] = clearBitMask32;

             break;

          case gcvSURF_A8B12G12R12_2_A8R8G8B8:
             clearValueType = (ClearArgs->color.valueType| gcvVALUE_FLAG_UNSIGNED_DENORM);
             switch (LayerIndex)
             {
             case 0:
                 Surface->clearValueUpper[0] =
                 Surface->clearValue[0] =
                    /* Convert red upper 4 bits to 8-bit*/
                    ( ((_ConvertValue(clearValueType,
                                     ClearArgs->color.r, 12) & 0xf00) >> 4) << 16)
                    /* Convert green upper 4 bits to 8-bit*/
                    | (((_ConvertValue(clearValueType,
                                     ClearArgs->color.g, 12) & 0xf00) >> 4) << 8)
                    /* Convert blue upper 4 bits to 8-bit*/
                    | ((_ConvertValue(clearValueType,
                                    ClearArgs->color.b, 12) & 0xf00) >> 4)
                        /* Convert alpha to 8-bit. */
                    | (_ConvertValue(clearValueType,
                                     ClearArgs->color.a, 8) << 24);
                 break;

             case 1:
                 Surface->clearValueUpper[1] =
                 Surface->clearValue[1] =
                    /* Convert red lower 8 bits to 8-bit. */
                    ( (_ConvertValue(clearValueType,
                                     ClearArgs->color.r, 12) & 0xff) << 16)
                    /* Convert green lower 8 bits to 8-bit. */
                    | ((_ConvertValue(clearValueType,
                                     ClearArgs->color.g, 12) & 0xff) << 8)
                    /* Convert blue lower 8 bits to 8-bit. */
                    | (_ConvertValue(clearValueType,
                                    ClearArgs->color.b, 12) & 0xff)
                        /* Convert alpha to 8-bit. */
                    | (_ConvertValue(clearValueType,
                                     ClearArgs->color.a, 8) << 24);
                 break;

             default:
                 gcmASSERT(0);
                 break;
             }

             Surface->clearBitMask[LayerIndex] = clearBitMask32;
             Surface->clearBitMaskUpper[LayerIndex] = clearBitMask32;
             break;

         case gcvSURF_R5G5B5A1:
             gcmASSERT(LayerIndex == 0);
             clearValueType = (ClearArgs->color.valueType| gcvVALUE_FLAG_UNSIGNED_DENORM);
             Surface->clearValue[0]
                 /* Convert red into 5-bit. */
                 = (_ConvertValue(clearValueType,
                                  ClearArgs->color.r, 5) << 11)
                 /* Convert green into 5-bit. */
                 | (_ConvertValue(clearValueType,
                                  ClearArgs->color.g, 5) << 6)
                 /* Convert blue into 5-bit. */
                 | (_ConvertValue(clearValueType,
                                 ClearArgs->color.b, 5) << 1)
                 /* Convert alpha into 1-bit. */
                 | _ConvertValue(clearValueType,
                                  ClearArgs->color.a, 1);

             /* Expand 16-bit color into 32-bit color. */
             Surface->clearValue[0] |= Surface->clearValue[0] << 16;
             Surface->clearValueUpper[0] = Surface->clearValue[0];
             Surface->clearBitMask[LayerIndex] = clearBitMask32;
             Surface->clearBitMask[LayerIndex] |= Surface->clearBitMask[LayerIndex] << 16;
             Surface->clearBitMaskUpper[LayerIndex] = Surface->clearBitMask[LayerIndex];
             break;

         case gcvSURF_A2B10G10R10UI:
         case gcvSURF_A2B10G10R10UI_1_A8R8G8B8:
             gcmASSERT(LayerIndex == 0);
             clearValueType = ClearArgs->color.valueType;

             Surface->clearValueUpper[0] =
             Surface->clearValue[0]
                 /* Convert red to 10-bit. */
                 = _ConvertValue(clearValueType,
                                 ClearArgs->color.r, 10)
                 /* Convert green to 10-bit. */
                 | (_ConvertValue(clearValueType,
                                  ClearArgs->color.g, 10) << 10)
                 /* Convert blue to 10-bit. */
                 | (_ConvertValue(clearValueType,
                                  ClearArgs->color.b, 10) << 20)
                 /* Convert alpha to 2-bit. */
                 | (_ConvertValue(clearValueType,
                                  ClearArgs->color.a, 2) << 30);
             Surface->clearBitMask[LayerIndex] = clearBitMask32;
             Surface->clearBitMaskUpper[LayerIndex] = clearBitMask32;
             break;

         case gcvSURF_A8B8G8R8I:
         case gcvSURF_A8B8G8R8UI:
             gcmASSERT(LayerIndex == 0);
             clearValueType = ClearArgs->color.valueType;

             Surface->clearValueUpper[0] =
             Surface->clearValue[0]
                 /* Convert red to 8-bit. */
                 = (_ConvertValue(clearValueType,
                                  ClearArgs->color.r, 8))
                 /* Convert green to 8-bit. */
                 | (_ConvertValue(clearValueType,
                                  ClearArgs->color.g, 8) << 8)
                 /* Convert blue to 8-bit. */
                 | (_ConvertValue(clearValueType,
                                  ClearArgs->color.b, 8) << 16)
                 /* Convert alpha to 8-bit. */
                 | (_ConvertValue(clearValueType,
                                  ClearArgs->color.a, 8) << 24);
             Surface->clearBitMask[LayerIndex] = clearBitMask32;
             Surface->clearBitMaskUpper[LayerIndex] = clearBitMask32;
             break;

         case gcvSURF_A8B8G8R8I_1_A8R8G8B8:
         case gcvSURF_A8B8G8R8UI_1_A8R8G8B8:
             gcmASSERT(LayerIndex == 0);
             clearValueType = ClearArgs->color.valueType;

             Surface->clearValueUpper[0] =
             Surface->clearValue[0]
                 /* Convert blue  to 8-bit. */
                 = (_ConvertValue(clearValueType,
                                  ClearArgs->color.b, 8))
                 /* Convert green to 8-bit. */
                 | (_ConvertValue(clearValueType,
                                  ClearArgs->color.g, 8) << 8)
                 /* Convert red   to 8-bit. */
                 | (_ConvertValue(clearValueType,
                                  ClearArgs->color.r, 8) << 16)
                 /* Convert alpha to 8-bit. */
                 | (_ConvertValue(clearValueType,
                                  ClearArgs->color.a, 8) << 24);
             Surface->clearBitMask[LayerIndex] = clearBitMask32;
             Surface->clearBitMaskUpper[LayerIndex] = clearBitMask32;
             break;

         case gcvSURF_R8I:
         case gcvSURF_R8UI:
             gcmASSERT(LayerIndex == 0);
             clearValueType = ClearArgs->color.valueType;

             Surface->clearValue[0] = _ConvertValue(clearValueType, ClearArgs->color.r, 8);
             Surface->clearValue[0] |= (Surface->clearValue[0] << 8);
             Surface->clearValue[0] |= (Surface->clearValue[0] << 16);
             Surface->clearValueUpper[0] = Surface->clearValue[0];
             Surface->clearBitMask[LayerIndex] = clearBitMask32;
             Surface->clearBitMask[LayerIndex] |= Surface->clearBitMask[LayerIndex] << 8;
             Surface->clearBitMask[LayerIndex] |= Surface->clearBitMask[LayerIndex] << 16;
             Surface->clearBitMaskUpper[LayerIndex] = Surface->clearBitMask[LayerIndex];
             break;

         case gcvSURF_R8:
             gcmASSERT(LayerIndex == 0);
             clearValueType = (ClearArgs->color.valueType| gcvVALUE_FLAG_UNSIGNED_DENORM);

             Surface->clearValue[0] = _ConvertValue(clearValueType, ClearArgs->color.r, 8);
             Surface->clearValue[0] |= (Surface->clearValue[0] << 8);
             Surface->clearValue[0] |= (Surface->clearValue[0] << 16);
             Surface->clearValueUpper[0] = Surface->clearValue[0];
             Surface->clearBitMask[LayerIndex] = clearBitMask32;
             Surface->clearBitMask[LayerIndex] |= Surface->clearBitMask[LayerIndex] << 8;
             Surface->clearBitMask[LayerIndex] |= Surface->clearBitMask[LayerIndex] << 16;
             Surface->clearBitMaskUpper[LayerIndex] = Surface->clearBitMask[LayerIndex];
             break;

         case gcvSURF_G8R8I:
         case gcvSURF_G8R8UI:
             gcmASSERT(LayerIndex == 0);
             clearValueType = ClearArgs->color.valueType;

             Surface->clearValue[0]
                 = (_ConvertValue(clearValueType,
                                  ClearArgs->color.r, 8)
                 | (_ConvertValue(clearValueType,
                                  ClearArgs->color.g, 8) << 8));

             Surface->clearValue[0] |= (Surface->clearValue[0] << 16);
             Surface->clearValueUpper[0] = Surface->clearValue[0];
             Surface->clearBitMask[LayerIndex] = clearBitMask32;
             Surface->clearBitMask[LayerIndex] |= Surface->clearBitMask[LayerIndex] << 16;
             Surface->clearBitMaskUpper[LayerIndex] = Surface->clearBitMask[LayerIndex];

             break;

         case gcvSURF_R16F:
             gcmASSERT(LayerIndex == 0);
             clearValueType = (gceVALUE_TYPE) (ClearArgs->color.valueType | gcvVALUE_FLAG_FLOAT_TO_FLOAT16);

             Surface->clearValue[0] =
                 _ConvertValue(clearValueType, ClearArgs->color.r, 16);
             Surface->clearValue[0] |= Surface->clearValue[0] << 16;
             Surface->clearValueUpper[0] = Surface->clearValue[0];
             Surface->clearBitMask[LayerIndex] = clearBitMask32;
             Surface->clearBitMask[LayerIndex] |= Surface->clearBitMask[LayerIndex] << 16;
             Surface->clearBitMaskUpper[LayerIndex] = Surface->clearBitMask[LayerIndex];

             break;

         case gcvSURF_R16I:
         case gcvSURF_R16UI:
             gcmASSERT(LayerIndex == 0);
             clearValueType = ClearArgs->color.valueType;

             Surface->clearValue[0] =
                 _ConvertValue(clearValueType, ClearArgs->color.r, 16);
             Surface->clearValue[0] |= Surface->clearValue[0] << 16;
             Surface->clearValueUpper[0] = Surface->clearValue[0];
             Surface->clearBitMask[LayerIndex] = clearBitMask32;
             Surface->clearBitMask[LayerIndex] |= Surface->clearBitMask[LayerIndex] << 16;
             Surface->clearBitMaskUpper[LayerIndex] = Surface->clearBitMask[LayerIndex];

             break;

         case gcvSURF_G16R16F:
             gcmASSERT(LayerIndex == 0);
             clearValueType = (gceVALUE_TYPE) (ClearArgs->color.valueType | gcvVALUE_FLAG_FLOAT_TO_FLOAT16);

             Surface->clearValueUpper[0] =
             Surface->clearValue[0]
                 = (_ConvertValue(clearValueType, ClearArgs->color.r, 16)
                 | (_ConvertValue(clearValueType, ClearArgs->color.g, 16) << 16));
             Surface->clearBitMask[LayerIndex] = clearBitMask32;
             Surface->clearBitMaskUpper[LayerIndex] = clearBitMask32;
             break;

         case gcvSURF_G16R16I:
         case gcvSURF_G16R16UI:
             gcmASSERT(LayerIndex == 0);
             clearValueType = ClearArgs->color.valueType;

             Surface->clearValueUpper[0] =
             Surface->clearValue[0]
                 = (_ConvertValue(clearValueType, ClearArgs->color.r, 16)
                 | (_ConvertValue(clearValueType, ClearArgs->color.g, 16) << 16));
            Surface->clearBitMask[LayerIndex] = clearBitMask32;
            Surface->clearBitMaskUpper[LayerIndex] = clearBitMask32;
             break;

         case gcvSURF_A16B16G16R16F:
         case gcvSURF_X16B16G16R16F:
             gcmASSERT(LayerIndex == 0);
             clearValueType = (gceVALUE_TYPE) (ClearArgs->color.valueType | gcvVALUE_FLAG_FLOAT_TO_FLOAT16);

             Surface->clearValue[0]
                 = (_ConvertValue(clearValueType,
                                  ClearArgs->color.r, 16)
                 | (_ConvertValue(clearValueType,
                                  ClearArgs->color.g, 16) << 16));
             Surface->clearValueUpper[0]
                 = (_ConvertValue(clearValueType,
                                  ClearArgs->color.b, 16)
                 | (_ConvertValue(clearValueType,
                                  ClearArgs->color.a, 16) << 16));

             Surface->clearBitMask[LayerIndex] = CHANNEL_BITMASK(Red) | CHANNEL_BITMASK(Green);
             Surface->clearBitMaskUpper[LayerIndex] = CHANNEL_BITMASK(Blue) | CHANNEL_BITMASK(Alpha);
              break;

         case gcvSURF_A16B16G16R16F_2_A8R8G8B8:
             clearValueType = (ClearArgs->color.valueType | gcvVALUE_FLAG_FLOAT_TO_FLOAT16);

             switch (LayerIndex)
             {
             case 0:
                 Surface->clearValueUpper[0] =
                 Surface->clearValue[0]
                     = (_ConvertValue(clearValueType, ClearArgs->color.r, 16)
                     | (_ConvertValue(clearValueType, ClearArgs->color.g, 16) << 16));

                 Surface->clearBitMask[LayerIndex] = CHANNEL_BITMASK(Red) | CHANNEL_BITMASK(Green);
                 break;
             case 1:
                 Surface->clearValueUpper[1] =
                 Surface->clearValue[1]
                     = (_ConvertValue(clearValueType, ClearArgs->color.b, 16)
                     | (_ConvertValue(clearValueType, ClearArgs->color.a, 16) << 16));

                 Surface->clearBitMask[LayerIndex] = CHANNEL_BITMASK(Blue) | CHANNEL_BITMASK(Alpha);
                 break;

             default:
                 gcmASSERT(0);
                 break;
             }

             Surface->clearBitMaskUpper[LayerIndex] = Surface->clearBitMask[LayerIndex];
             break;

         case gcvSURF_A16B16G16R16F_2_G16R16F:
             clearValueType = (gceVALUE_TYPE) (ClearArgs->color.valueType | gcvVALUE_FLAG_FLOAT_TO_FLOAT16);

             switch (LayerIndex)
             {
             case 0:
                 Surface->clearValueUpper[0] =
                     Surface->clearValue[0]
                 = (_ConvertValue(clearValueType,
                     ClearArgs->color.r, 16)
                     | (_ConvertValue(clearValueType, ClearArgs->color.g, 16) << 16));
                 Surface->clearBitMask[LayerIndex] = CHANNEL_BITMASK(Red) | CHANNEL_BITMASK(Green);

                 break;
             case 1:
                 Surface->clearValueUpper[1] =
                     Surface->clearValue[1]
                 = (_ConvertValue(clearValueType, ClearArgs->color.b, 16)
                     | (_ConvertValue(clearValueType, ClearArgs->color.a, 16) << 16));
                 Surface->clearBitMask[LayerIndex] = CHANNEL_BITMASK(Blue) | CHANNEL_BITMASK(Alpha);

                 break;
             default:
                 gcmASSERT(0);
                 break;
             }

             Surface->clearBitMaskUpper[LayerIndex] = Surface->clearBitMask[LayerIndex];
             break;

         case gcvSURF_A16B16G16R16I:
         case gcvSURF_X16B16G16R16I:
         case gcvSURF_A16B16G16R16UI:
         case gcvSURF_X16B16G16R16UI:
         case gcvSURF_A16B16G16R16I_1_G32R32F:
         case gcvSURF_A16B16G16R16UI_1_G32R32F:
             gcmASSERT(LayerIndex == 0);
             clearValueType = ClearArgs->color.valueType;

             Surface->clearValue[0]
                 = (_ConvertValue(clearValueType, ClearArgs->color.r, 16)
                 | (_ConvertValue(clearValueType, ClearArgs->color.g, 16) << 16));
             Surface->clearValueUpper[0]
                 = (_ConvertValue(clearValueType, ClearArgs->color.b, 16)
                 | (_ConvertValue(clearValueType, ClearArgs->color.a, 16) << 16));
             Surface->clearBitMask[LayerIndex] = CHANNEL_BITMASK(Red) | CHANNEL_BITMASK(Green);
             Surface->clearBitMaskUpper[LayerIndex] = CHANNEL_BITMASK(Blue) | CHANNEL_BITMASK(Alpha);

             break;

         case gcvSURF_A16B16G16R16I_2_A8R8G8B8:
         case gcvSURF_A16B16G16R16UI_2_A8R8G8B8:
             clearValueType = ClearArgs->color.valueType;

             switch (LayerIndex)
             {
             case 0:
                 Surface->clearValueUpper[0] =
                 Surface->clearValue[0]
                     = (_ConvertValue(clearValueType, ClearArgs->color.r, 16)
                     | (_ConvertValue(clearValueType, ClearArgs->color.g, 16) << 16));
                 Surface->clearBitMask[LayerIndex] = CHANNEL_BITMASK(Red) | CHANNEL_BITMASK(Green);

                 break;

             case 1:
                 Surface->clearValueUpper[1] =
                 Surface->clearValue[1]
                     = (_ConvertValue(clearValueType, ClearArgs->color.b, 16)
                     | (_ConvertValue(clearValueType, ClearArgs->color.a, 16) << 16));
                 Surface->clearBitMask[LayerIndex] = CHANNEL_BITMASK(Blue) | CHANNEL_BITMASK(Alpha);

                 break;

             default:
                 gcmASSERT(0);
                 break;
             }

             Surface->clearBitMaskUpper[LayerIndex] = Surface->clearBitMask[LayerIndex];
             break;

         case gcvSURF_R32F:
         case gcvSURF_R32I:
         case gcvSURF_R32UI:
         case gcvSURF_R32UI_1_A8R8G8B8:
         case gcvSURF_R32I_1_A8R8G8B8:
             gcmASSERT(LayerIndex == 0);
             clearValueType = ClearArgs->color.valueType;

             Surface->clearValueUpper[0] =
             Surface->clearValue[0]
                 = (_ConvertValue(clearValueType, ClearArgs->color.r, 32));
             Surface->clearBitMask[LayerIndex] = maskRed;
             Surface->clearBitMaskUpper[LayerIndex] = Surface->clearBitMask[LayerIndex];
             break;

         case gcvSURF_G32R32F:
         case gcvSURF_G32R32I:
         case gcvSURF_G32R32UI:
             gcmASSERT(LayerIndex == 0);
             clearValueType = ClearArgs->color.valueType;

             Surface->clearValue[0]
                 = (_ConvertValue(clearValueType, ClearArgs->color.r, 32));
             Surface->clearValueUpper[0]
                 = (_ConvertValue(clearValueType, ClearArgs->color.g, 32));

             Surface->clearBitMask[LayerIndex] = maskRed;
             Surface->clearBitMaskUpper[LayerIndex] = maskGreen;
             break;

         case gcvSURF_A32B32G32R32F_2_G32R32F:
         case gcvSURF_X32B32G32R32F_2_G32R32F:
         case gcvSURF_A32B32G32R32I_2_G32R32I:
         case gcvSURF_X32B32G32R32I_2_G32R32I:
         case gcvSURF_A32B32G32R32UI_2_G32R32UI:
         case gcvSURF_X32B32G32R32UI_2_G32R32UI:
             clearValueType = ClearArgs->color.valueType;

             switch (LayerIndex)
             {
             case 0:
                 Surface->clearValue[0]
                     = (_ConvertValue(clearValueType, ClearArgs->color.r, 32));
                 Surface->clearValueUpper[0]
                     = (_ConvertValue(clearValueType, ClearArgs->color.g, 32));

                 Surface->clearBitMask[LayerIndex] = maskRed;
                 Surface->clearBitMaskUpper[LayerIndex] = maskGreen;
                 break;

             case 1:
                 Surface->clearValue[1]
                     = (_ConvertValue(clearValueType, ClearArgs->color.b, 32));
                 Surface->clearValueUpper[1]
                     = (_ConvertValue(clearValueType, ClearArgs->color.a, 32));

                 Surface->clearBitMask[LayerIndex] = maskBlue;
                 Surface->clearBitMaskUpper[LayerIndex] = maskAlpha;
                 break;
             default:
                 gcmASSERT(0);
                 break;
             }
             break;

         case gcvSURF_A32B32G32R32I_4_A8R8G8B8:
         case gcvSURF_A32B32G32R32UI_4_A8R8G8B8:
             clearValueType = ClearArgs->color.valueType;

             switch (LayerIndex)
             {
             case 0:
                 Surface->clearValueUpper[0] =
                 Surface->clearValue[0]
                     = (_ConvertValue(clearValueType, ClearArgs->color.r, 32));
                 Surface->clearBitMask[LayerIndex] = maskRed;
                 break;
             case 1:
                 Surface->clearValueUpper[1] =
                 Surface->clearValue[1]
                     = (_ConvertValue(clearValueType, ClearArgs->color.g, 32));
                 Surface->clearBitMask[LayerIndex] = maskGreen;
                 break;
             case 2:
                 Surface->clearValueUpper[2] =
                 Surface->clearValue[2]
                     = (_ConvertValue(clearValueType, ClearArgs->color.b, 32));
                Surface->clearBitMask[LayerIndex] = maskBlue;
                 break;
             case 3:
                 Surface->clearValueUpper[3] =
                 Surface->clearValue[3]
                     = (_ConvertValue(clearValueType, ClearArgs->color.a, 32));
                 Surface->clearBitMask[LayerIndex] = maskAlpha;
                 break;
             default:
                 gcmASSERT(0);
                 break;
             }
             Surface->clearBitMaskUpper[LayerIndex] = Surface->clearBitMask[LayerIndex];
             break;

         default:
            gcmTRACE(
                gcvLEVEL_ERROR,
                "%s(%d): Unknown format=%d",
                __FUNCTION__, __LINE__, Surface->format
                );

            /* Invalid surface format. */
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
         }
    }

    /* Test for clearing depth or stencil buffer. */
    if (ClearArgs->flags & (gcvCLEAR_DEPTH | gcvCLEAR_STENCIL))
    {
        gctBOOL  clearDepth   = (ClearArgs->flags & gcvCLEAR_DEPTH) && ClearArgs->depthMask;
        gctBOOL  clearStencil = (ClearArgs->flags & gcvCLEAR_STENCIL) && ClearArgs->stencilMask;
        gctUINT32 clearValue = 0;

        clearValueType = (gceVALUE_TYPE)(gcvVALUE_FLOAT | gcvVALUE_FLAG_UNSIGNED_DENORM);
        tempMask = 0;
        Surface->clearMask[0] = 0;

            /* Dispatch on depth format. */
        switch (Surface->format)
        {
        case gcvSURF_D16: /* 16-bit depth without stencil. */
            /* Convert depth value to 16-bit. */
            if(clearDepth)
            {
                clearValue = (_ConvertValue(clearValueType, ClearArgs->depth, 16));
                clearValue |= (clearValue << 16);
                tempMask = 0xFFFFFFFF;
                Surface->clearMask[0] |= 0xF;
            }

            break;

        case gcvSURF_D24S8: /* 24-bit depth with 8-bit stencil. */
            /* Convert depth value to 24-bit. */
            if(clearDepth)
            {
                clearValue = (_ConvertValue(clearValueType, ClearArgs->depth, 24) << 8 );
                tempMask = 0xFFFFFF00;
                Surface->clearMask[0] |= 0xE;
            }

            if(clearStencil)
            {
                clearValue &= ~0xFF;
                clearValue |= ClearArgs->stencil;
                tempMask |= ClearArgs->stencilMask;
                Surface->clearMask[0] |= 0x1;
            }

            break;

        case gcvSURF_D24X8: /* 24-bit depth with no stencil. */
            /* Convert depth value to 24-bit. */
            if(clearDepth)
            {
                clearValue = (_ConvertValue(clearValueType, ClearArgs->depth, 24) << 8 );
                tempMask = 0xFFFFFF00;
                Surface->clearMask[0] = 0xF;
            }

            break;

        case gcvSURF_S8:
        case gcvSURF_X24S8: /* 8-bit stencil with no depth. */
            if(clearStencil)
            {
                clearValue = ClearArgs->stencil;
                tempMask |= ClearArgs->stencilMask;
                Surface->clearMask[0] = 0xF;
            }

            clearValue  |= (clearValue << 8);
            clearValue  |= (clearValue << 16);
            tempMask    |= (tempMask << 8 );
            tempMask    |= (tempMask << 16 );

            break;

        default:
            /* Invalid depth buffer format. */
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        Surface->clearValue[0] = clearValue;
        Surface->clearValueUpper[0] = clearValue;
        Surface->clearBitMask[0] = tempMask;
        Surface->clearBitMaskUpper[0] = 0xFFFFFFFF;

        /* Compute HZ clear value. */
        gcmONERROR(gcoHARDWARE_HzClearValueControl(Surface->format,
                                                   Surface->clearValue[0],
                                                   &Surface->clearValueHz,
                                                   gcvNULL));

    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the error. */
    gcmFOOTER();
    return status;
}

static gceSTATUS
_ClearRect_v2(
    IN gcsSURF_VIEW *SurfView,
    IN gcsSURF_CLEAR_ARGS_PTR ClearArgs,
    IN gctUINT32 LayerIndex
    )
{
    gceSTATUS status;
    gctUINT32 address;
    gcsRECT_PTR rect = ClearArgs->clearRect;

    gcoSURF surf = SurfView->surf;
    gcsSURF_INFO_PTR surfInfo = &surf->info;

    gcmHEADER_ARG("SurfView=0x%x ClearArgs=0x%x, LayerIndex=%d",
                   SurfView, ClearArgs, LayerIndex);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(surf, gcvOBJ_SURF);
    gcmDEBUG_VERIFY_ARGUMENT(ClearArgs != gcvNULL);

    /* LayerIndex only apply for color buffer */
    gcmASSERT(LayerIndex == 0 || (ClearArgs->flags & gcvCLEAR_COLOR));

    gcmGETHARDWAREADDRESS(surfInfo->node, address);
    gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                  "_ClearRect_v2: Clearing surface 0x%x @ 0x%08X",
                  surf, address);

    if (!(ClearArgs->flags & gcvCLEAR_WITH_CPU_ONLY)
        && (surfInfo->isMsaa)
        && ((gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_FAST_MSAA) == gcvTRUE) ||
            (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_SMALL_MSAA) == gcvTRUE)))
    {
        gcmONERROR(gcvSTATUS_NOT_ALIGNED);
    }

    if (ClearArgs->flags & gcvCLEAR_WITH_GPU_ONLY)
    {
        gctUINT sizeX, sizeY;
        gctUINT originX, originY;

        /* Query resolve clear alignment for this surface. */
        gcmONERROR(
            gcoHARDWARE_GetSurfaceResolveAlignment(gcvNULL,
                                                   surfInfo,
                                                   &originX,
                                                   &originY,
                                                   &sizeX,
                                                   &sizeY));

        if ((rect->left & (originX - 1)) ||
            (rect->top  & (originY - 1)) ||
            ((rect->right  * surfInfo->sampleInfo.x < (gctINT)surfInfo->requestW) &&
             ((rect->right  - rect->left) & (sizeX - 1))
            ) ||
            ((rect->bottom * surfInfo->sampleInfo.y < (gctINT)surfInfo->requestH) &&
             ((rect->bottom - rect->top ) & (sizeY - 1))
            )
           )
        {
            /* Quickly reject if not resolve aligned.
             * Avoid decompress and disable tile status below.
            */
            status = gcvSTATUS_NOT_ALIGNED;
            goto OnError;
        }
    }

    /* Flush the tile status and decompress the buffers. */
    gcmONERROR(gcoSURF_DisableTileStatus(surf, gcvTRUE));

    /* Compute clear values. */
    gcmONERROR(_ComputeClear(surfInfo, ClearArgs, LayerIndex));

    if (surfInfo->clearMask[LayerIndex])
    {
        gcsRECT msaaRect = *rect;
        /* Adjust the rect according to the msaa sample. */
        msaaRect.left   *= surfInfo->sampleInfo.x;
        msaaRect.right  *= surfInfo->sampleInfo.x;
        msaaRect.top    *= surfInfo->sampleInfo.y;
        msaaRect.bottom *= surfInfo->sampleInfo.y;

        /* Clamp the coordinates to surface dimensions. */
        msaaRect.left   = gcmMAX(msaaRect.left,   0);
        msaaRect.top    = gcmMAX(msaaRect.top,    0);
        msaaRect.right  = gcmMIN(msaaRect.right,  (gctINT32)surfInfo->alignedW);
        msaaRect.bottom = gcmMIN(msaaRect.bottom, (gctINT32)surfInfo->alignedH);

        status = gcvSTATUS_NOT_SUPPORTED;

        /* Try hardware clear if requested or default. */
        if (!(ClearArgs->flags & gcvCLEAR_WITH_CPU_ONLY) &&
            /* HW stencil clear only support full mask */
            (!(ClearArgs->flags & gcvCLEAR_STENCIL) || (ClearArgs->stencilMask == 0xff))
           )
        {
            status = gcoHARDWARE_Clear_v2(gcvNULL,
                                          SurfView,
                                          LayerIndex,
                                          &msaaRect,
                                          surfInfo->clearValue[LayerIndex],
                                          surfInfo->clearValueUpper[LayerIndex],
                                          surfInfo->clearMask[LayerIndex]);
        }

        /* Try software clear if requested or default. */
        if (gcmIS_ERROR(status) && !(ClearArgs->flags & gcvCLEAR_WITH_GPU_ONLY))
        {
            gctUINT8 stencilWriteMask = (ClearArgs->flags & (gcvCLEAR_DEPTH | gcvCLEAR_STENCIL))
                                      ? ClearArgs->stencilMask
                                      : 0xFF;
            status = gcoHARDWARE_ClearSoftware_v2(gcvNULL,
                                                  SurfView,
                                                  LayerIndex,
                                                  &msaaRect,
                                                  surfInfo->clearValue[LayerIndex],
                                                  surfInfo->clearValueUpper[LayerIndex],
                                                  surfInfo->clearMask[LayerIndex],
                                                  stencilWriteMask);
        }

        /* Break now if status is error. */
        if (status == gcvSTATUS_NOT_ALIGNED)
        {
            goto OnError;
        }
        gcmONERROR(status);

        /* Clear HZ if have */
        if ((ClearArgs->flags & gcvCLEAR_DEPTH) && (surfInfo->hzNode.size > 0))
        {
            gcsRECT hzRect = {0};
            struct _gcoSURF hzSurf;
            gcsSURF_VIEW hzView = {gcvNULL, 0, 1};
            gctUINT width = 0, height = 0;
            gcsSURF_FORMAT_INFO_PTR fmtInfo = gcvNULL;

            /* Compute the hw specific clear window. */
            gcmONERROR(gcoHARDWARE_ComputeClearWindow(gcvNULL, surfInfo->hzNode.size, &width, &height));

            gcoOS_ZeroMemory(&hzSurf, gcmSIZEOF(hzSurf));
            hzSurf.object.type     = gcvOBJ_SURF;
            hzSurf.info.requestW   = width;
            hzSurf.info.requestH   = height;
            hzSurf.info.requestD   = 1;
            hzSurf.info.allocedW   = width;
            hzSurf.info.allocedH   = height;
            hzSurf.info.alignedW   = width;
            hzSurf.info.alignedH   = height;
            hzSurf.info.sampleInfo = g_sampleInfos[1];
            hzSurf.info.isMsaa     = gcvFALSE;
            hzSurf.info.edgeAA     = gcvFALSE;
            hzSurf.info.format     = gcvSURF_A8R8G8B8;
            hzSurf.info.node       = surfInfo->hzNode;
            hzSurf.info.stride     = width * 4;
            hzSurf.info.colorSpace = gcvSURF_COLOR_SPACE_LINEAR;

            gcoSURF_QueryFormat(gcvSURF_A8R8G8B8, &fmtInfo);
            hzSurf.info.formatInfo = *fmtInfo;
            hzSurf.info.bitsPerPixel = fmtInfo->bitsPerPixel;
            hzSurf.info.sliceSize = (gctUINT)surfInfo->hzNode.size;
            hzSurf.info.layerSize = (gctUINT)surfInfo->hzNode.size;

            /* If full clear, clear HZ to clear value*/
            if (rect->left == 0 && rect->right  >= (gctINT)surfInfo->requestW &&
                rect->top == 0  && rect->bottom >= (gctINT)surfInfo->requestH)
            {
                hzSurf.info.clearValueHz = surfInfo->clearValueHz;
            }
            else
            {
                hzSurf.info.clearValueHz = 0xffffffff;
            }

            if (surfInfo->tiling & gcvTILING_SPLIT_BUFFER)
            {
                hzSurf.info.tiling = gcvMULTI_TILED;
                hzSurf.info.bottomBufferOffset = (gctUINT32)surfInfo->hzNode.size / 2;
            }
            else
            {
                hzSurf.info.tiling = gcvTILED;
            }
            hzSurf.info.pfGetAddr = gcoHARDWARE_GetProcCalcPixelAddr(gcvNULL, & hzSurf.info);

            hzRect.right = width;
            hzRect.bottom = height;

            hzView.surf = &hzSurf;
            /* Send clear command to hardware. */
            gcmONERROR(gcoHARDWARE_Clear_v2(gcvNULL,
                                            &hzView,
                                            0,
                                            &hzRect,
                                            hzSurf.info.clearValueHz,
                                            hzSurf.info.clearValueHz,
                                            0xF));

            surfInfo->hzDisabled = gcvFALSE;
        }
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

static gceSTATUS
_DoClearTileStatus(
    IN gcsSURF_INFO_PTR Surface,
    IN gctUINT32 TileStatusAddress,
    IN gcsSURF_CLEAR_ARGS_PTR ClearArgs,
    IN gctINT32 LayerIndex
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x TileStatusAddress=0x%08x ClearArgs=0x%x, LayerIndex=%d",
                  Surface, TileStatusAddress, ClearArgs, LayerIndex);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Surface != 0);
    gcmDEBUG_VERIFY_ARGUMENT(TileStatusAddress != 0);

    /* Compute clear values. */
    gcmONERROR(
        _ComputeClear(Surface, ClearArgs, LayerIndex));

    /* Always flush (and invalidate) the tile status cache. */
    gcmONERROR(
        gcoHARDWARE_FlushTileStatus(gcvNULL,
                                    Surface,
                                    gcvFALSE));

    gcmASSERT(LayerIndex == 0);

    /* Test for clearing render target. */
    if (ClearArgs->flags & gcvCLEAR_COLOR)
    {
        if (Surface->clearMask[0] != 0)
        {
            /* Send the clear command to the hardware. */
            status =
                gcoHARDWARE_ClearTileStatus(gcvNULL,
                                            Surface,
                                            TileStatusAddress,
                                            0,
                                            gcvSURF_RENDER_TARGET,
                                            Surface->clearValue[0],
                                            Surface->clearValueUpper[0],
                                            Surface->clearMask[0]);

            if (status == gcvSTATUS_NOT_SUPPORTED)
            {
                goto OnError;
            }

            gcmONERROR(status);
        }
        else
        {
            gcmFOOTER_ARG("%s", "gcvSTATUS_SKIP");
            return gcvSTATUS_SKIP;
        }
    }

    /* Test for clearing depth or stencil buffer. */
    if (ClearArgs->flags & (gcvCLEAR_DEPTH | gcvCLEAR_STENCIL))
    {
        if (Surface->clearMask[0] != 0)
        {
            status = gcvSTATUS_NOT_SUPPORTED;

            if (!(ClearArgs->flags & gcvCLEAR_STENCIL) || (ClearArgs->stencilMask == 0xff))
            {
                /* Send clear command to hardware. */
                status = gcoHARDWARE_ClearTileStatus(gcvNULL,
                                                     Surface,
                                                     TileStatusAddress,
                                                     0,
                                                     gcvSURF_DEPTH,
                                                     Surface->clearValue[0],
                                                     Surface->clearValueUpper[0],
                                                     Surface->clearMask[0]);
            }

            if (status == gcvSTATUS_NOT_SUPPORTED)
            {
                goto OnError;
            }
            gcmONERROR(status);

            /* Send semaphore from RASTER to PIXEL. */
            gcmONERROR(
                gcoHARDWARE_Semaphore(gcvNULL,
                                      gcvWHERE_RASTER,
                                      gcvWHERE_PIXEL,
                                      gcvHOW_SEMAPHORE,
                                      gcvNULL));
        }
        else
        {
            gcmFOOTER_ARG("%s", "gcvSTATUS_SKIP");
            return gcvSTATUS_SKIP;
        }
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  _ClearHzTileStatus
**
**  Perform a hierarchical Z tile status clear with the current depth values.
**  Note that this function does not recompute the clear colors, since it must
**  be called after gco3D_ClearTileStatus has cleared the depth tile status.
**
**  INPUT:
**
**      gco3D Engine
**          Pointer to an gco3D object.
**
**      gcsSURF_INFO_PTR Surface
**          Pointer of the depth surface to clear.
**
**      gcsSURF_NODE_PTR TileStatusAddress
**          Pointer to the hierarhical Z tile status node toclear.
**
**  OUTPUT:
**
**      Nothing.
*/
static gceSTATUS
_ClearHzTileStatus(
    IN gcsSURF_INFO_PTR Surface,
    IN gcsSURF_NODE_PTR TileStatus
    )
{
    gceSTATUS status;
    gctUINT32 address;

    gcmHEADER_ARG("Surface=0x%x TileStatus=0x%x",
                   Surface, TileStatus);

    gcmGETHARDWAREADDRESS(*TileStatus, address);

    /* Send clear command to hardware. */
    gcmONERROR(
        gcoHARDWARE_ClearTileStatus(gcvNULL,
                                    Surface,
                                    address,
                                    TileStatus->size,
                                    gcvSURF_HIERARCHICAL_DEPTH,
                                    Surface->clearValueHz,
                                    Surface->clearValueHz,
                                    0xF));

    /* Send semaphore from RASTER to PIXEL. */
    gcmONERROR(
        gcoHARDWARE_Semaphore(gcvNULL,
                              gcvWHERE_RASTER,
                              gcvWHERE_PIXEL,
                              gcvHOW_SEMAPHORE,
                              gcvNULL));

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}


/* Attempt to clear using tile status. */
static gceSTATUS
_ClearTileStatus(
    IN gcoSURF Surface,
    IN gcsSURF_CLEAR_ARGS_PTR ClearArgs,
    IN gctINT32 LayerIndex
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;
    gctUINT32 address;

    gcmHEADER_ARG("Surface=0x%x ClearArgs=0x%x LayerIndex=%d", Surface, ClearArgs, LayerIndex);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);
    gcmDEBUG_VERIFY_ARGUMENT(ClearArgs != gcvNULL);

    if (Surface->info.tileStatusNode.pool != gcvPOOL_UNKNOWN)
    {
        gctBOOL saved = Surface->info.tileStatusDisabled;

        do
        {
            gcmGETHARDWAREADDRESS(Surface->info.tileStatusNode, address);

            gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                          "_ClearTileStatus: Clearing tile status 0x%x @ 0x%08X for"
                          "surface 0x%x",
                          Surface->info.tileStatusNode,
                          address,
                          Surface);
            /* Turn on the tile status on in the beginning,
            ** So the later invalidate ts cache will not be skipped. */
            Surface->info.tileStatusDisabled = gcvFALSE;

            /* Clear the tile status. */
            status = _DoClearTileStatus(&Surface->info,
                                        address,
                                        ClearArgs,
                                        LayerIndex);

            if (status == gcvSTATUS_SKIP)
            {
                /* Should restore the tile status when skip the clear. */
                Surface->info.tileStatusDisabled = saved;

                /* Nothing needed clearing, and no error has occurred. */
                status = gcvSTATUS_OK;
                break;
            }

            if (status == gcvSTATUS_NOT_SUPPORTED)
            {
                break;
            }

            gcmERR_BREAK(status);

            if ((ClearArgs->flags & gcvCLEAR_DEPTH) &&
                (Surface->info.hzTileStatusNode.pool != gcvPOOL_UNKNOWN))
            {
                /* Clear the hierarchical Z tile status. */
                status = _ClearHzTileStatus(&Surface->info,
                                            &Surface->info.hzTileStatusNode);

                if (status == gcvSTATUS_NOT_SUPPORTED)
                {
                    break;
                }

                gcmERR_BREAK(status);

                Surface->info.hzDisabled = gcvFALSE;
            }

            /* Reset the tile status. */
            gcmERR_BREAK(
                gcoSURF_EnableTileStatus(Surface));
        }
        while (gcvFALSE);

        /* Restore if failed. */
        if (gcmIS_ERROR(status))
        {
            Surface->info.tileStatusDisabled = saved;
        }
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}

#if gcdPARTIAL_FAST_CLEAR
static gceSTATUS
_ClearTileStatusWindowAligned(
    IN gcsSURF_INFO_PTR Surface,
    IN gcsSURF_CLEAR_ARGS_PTR ClearArgs,
    IN gctINT32 LayerIndex,
    OUT gcsRECT_PTR AlignedRect
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("Surface=0x%x ClearArgs=0x%x LayerIndex=%d",
                  Surface, ClearArgs, LayerIndex);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(ClearArgs != 0);

    /* No MRT support when tile status enabled for now. */
    gcmASSERT(LayerIndex == 0);

    /* Compute clear values. */
    gcmONERROR(_ComputeClear(Surface, ClearArgs, LayerIndex));

    /* Check clear masks. */
    if (Surface->clearMask[0] == 0)
    {
        /* Nothing to clear. */
        gcmFOOTER_ARG("%s", "gcvSTATUS_SKIP");
        return gcvSTATUS_SKIP;
    }

    /* Check clearValue changes. */
    if ((Surface->tileStatusDisabled == gcvFALSE)
    &&  (   (Surface->fcValue != Surface->clearValue[0])
        ||  (Surface->fcValueUpper != Surface->clearValueUpper[0]))
    )
    {
        /*
         * Tile status is on but clear value changed.
         * Fail back to 3D draw clear or disable tile status and continue
         * partial fast clear.
         */

        status = gcvSTATUS_NOT_SUPPORTED;
        gcmFOOTER();
        return status;
    }

    /* Flush the tile status cache. */
    gcmONERROR(gcoHARDWARE_FlushTileStatus(gcvNULL, Surface, gcvFALSE));

    /* Test for clearing render target. */
    if (ClearArgs->flags & gcvCLEAR_COLOR)
    {
        /* Send the clear command to the hardware. */
        status =
            gcoHARDWARE_ClearTileStatusWindowAligned(gcvNULL,
                                                     Surface,
                                                     gcvSURF_RENDER_TARGET,
                                                     Surface->clearValue[0],
                                                     Surface->clearValueUpper[0],
                                                     Surface->clearMask[0],
                                                     ClearArgs->clearRect,
                                                     AlignedRect);

        if (status == gcvSTATUS_NOT_SUPPORTED)
        {
            goto OnError;
        }

        gcmONERROR(status);
    }

    /* Test for clearing depth or stencil buffer. */
    if (ClearArgs->flags & (gcvCLEAR_DEPTH | gcvCLEAR_STENCIL))
    {
        if (Surface->hzNode.pool != gcvPOOL_UNKNOWN)
        {
            /* Can not support clear depth when HZ buffer exists. */
            status = gcvSTATUS_NOT_SUPPORTED;
            goto OnError;
        }

        /* Send the clear command to the hardware. */
        status =
            gcoHARDWARE_ClearTileStatusWindowAligned(gcvNULL,
                                                     Surface,
                                                     gcvSURF_DEPTH,
                                                     Surface->clearValue[0],
                                                     Surface->clearValueUpper[0],
                                                     Surface->clearMask[0],
                                                     ClearArgs->clearRect,
                                                     AlignedRect);

        if (status == gcvSTATUS_NOT_SUPPORTED)
        {
            goto OnError;
        }

        gcmONERROR(status);

        gcmONERROR(
            gcoHARDWARE_Semaphore(gcvNULL,
                                  gcvWHERE_RASTER,
                                  gcvWHERE_PIXEL,
                                  gcvHOW_SEMAPHORE_STALL,
                                  gcvNULL));
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

static gceSTATUS
_PartialFastClear_v2(
    IN gcsSURF_VIEW *SurfView,
    IN gcsSURF_CLEAR_ARGS_PTR ClearArgs,
    IN gctINT32 LayerIndex
    )
{
    gceSTATUS status;
    gcsRECT alignedRect;
    gctBOOL saved;
    gcoSURF surf = SurfView->surf;
    gcsSURF_INFO_PTR surfInfo = &surf->info;

    gcmHEADER_ARG("SurfView=0x%x ClearArgs=0x%x LayerIndex=%d",
                  SurfView, ClearArgs, LayerIndex);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(surf, gcvOBJ_SURF);
    gcmVERIFY_ARGUMENT(ClearArgs != gcvNULL);

    if ((surfInfo->tileStatusNode.pool == gcvPOOL_UNKNOWN) ||
        (surfInfo->isMsaa))
    {
        /* No tile status buffer or AA. */
        status = gcvSTATUS_NOT_SUPPORTED;
        goto OnError;
    }

    /* Backup previous tile status is state. */
    saved = surfInfo->tileStatusDisabled;

    /* Do the partial fast clear. */
    status = _ClearTileStatusWindowAligned(surfInfo,
                                           ClearArgs,
                                           LayerIndex,
                                           &alignedRect);

    if (gcmIS_SUCCESS(status))
    {
        gctINT i, count;
        gcsRECT rect[4];
        gcsRECT_PTR clearRect;
        gcsSURF_CLEAR_ARGS newArgs;
        gcsSURF_VIEW *rtView = (ClearArgs->flags & gcvCLEAR_COLOR) ? SurfView : gcvNULL;
        gcsSURF_VIEW *dsView = (ClearArgs->flags & gcvCLEAR_COLOR) ? gcvNULL  : SurfView;

        /* Tile status is enabled if success, turn it on. */
        surfInfo->tileStatusDisabled = gcvFALSE;

        surfInfo->dirty = gcvTRUE;

        /* Reset the tile status. */
        gcmONERROR(gcoSURF_EnableTileStatus(surf));

        if (saved)
        {
            /*
             * Invalidate tile status cache.
             * A case is that tile status is decompressed and disabled but still
             * cached. Tile status memory is changed in above clear, we need
             * invalidate tile status cache here.
             * Decompressing by resolve onto self will only read tile status,
             * hardware will drop tile status cache without write back in that
             * case. So here only 'invalidate' is done in 'flush'.
             */
            gcmONERROR(gcoSURF_FlushTileStatus(surf, gcvFALSE));
        }

        /* Get not cleared area count. */
        clearRect = ClearArgs->clearRect;
        count = 0;

        if (alignedRect.left > clearRect->left)
        {
            rect[count].left   = clearRect->left;
            rect[count].top    = alignedRect.top;
            rect[count].right  = alignedRect.left;
            rect[count].bottom = alignedRect.bottom;
            count++;
        }

        if (alignedRect.top > clearRect->top)
        {
            rect[count].left   = clearRect->left;
            rect[count].top    = clearRect->top;
            rect[count].right  = clearRect->right;
            rect[count].bottom = alignedRect.top;
            count++;
        }

        if (alignedRect.right < clearRect->right)
        {
            rect[count].left   = alignedRect.right;
            rect[count].top    = alignedRect.top;
            rect[count].right  = clearRect->right;
            rect[count].bottom = alignedRect.bottom;
            count++;
        }

        if (alignedRect.bottom < clearRect->bottom)
        {
            rect[count].left   = clearRect->left;
            rect[count].top    = alignedRect.bottom;
            rect[count].right  = clearRect->right;
            rect[count].bottom = clearRect->bottom;
            count++;
        }

        gcoOS_MemCopy(&newArgs, ClearArgs, gcmSIZEOF(gcsSURF_CLEAR_ARGS));

        for (i = 0; i < count; i++)
        {
            /* Call blit draw to clear. */
            newArgs.clearRect = &rect[i];
            gcmONERROR(gcoHARDWARE_DrawClear_v2(rtView, dsView, &newArgs));
        }
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}
#endif

static gceSTATUS
_Clear_v2(
    IN gcsSURF_VIEW *SurfView,
    IN gcsSURF_CLEAR_ARGS_PTR ClearArgs,
    IN gctINT32 LayerIndex,
    OUT gctBOOL *BlitDraw
    )
{
    gctBOOL fullSize = gcvFALSE;
    gctPOINTER surfAddr[3] = {gcvNULL};
    gceSTATUS status = gcvSTATUS_OK;

    gcoSURF surf = SurfView->surf;
    gcsSURF_INFO_PTR surfInfo = &surf->info;

    gcmHEADER_ARG("SurfView=0x%x ClearArg=0x%x LayerIndex=%d",
                   SurfView, ClearArgs, LayerIndex);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(surf, gcvOBJ_SURF);
    gcmVERIFY_ARGUMENT(ClearArgs != gcvNULL);

    /* Lock the surface. */
    gcmONERROR(gcoSURF_Lock(surf, gcvNULL, surfAddr));

    if (ClearArgs->flags & gcvCLEAR_WITH_CPU_ONLY)
    {
        /* Software clear only. */
        status = _ClearRect_v2(SurfView, ClearArgs, LayerIndex);
    }
    else
    {
        do
        {
            gceCLEAR savedFlags;
            gcsRECT_PTR rect = ClearArgs->clearRect;

            /* Test for entire surface clear. */
            if ((rect->left == 0) &&
                (rect->top == 0) &&
                (rect->right  >= (gctINT)surfInfo->requestW) &&
                (rect->bottom >= (gctINT)surfInfo->requestH))
            {
                fullSize = gcvTRUE;
                /* 1. Full fast clear when it is an entire surface clear. */
                status = _ClearTileStatus(surf, ClearArgs, LayerIndex);
            }
#if gcdPARTIAL_FAST_CLEAR
            else
            {
                /* 2. Partial fast clear + 3D draw clear.
                 * Clear tile status window and then draw clear for not aligned parts.
                 */
                status = _PartialFastClear_v2(SurfView, ClearArgs, LayerIndex);
            }
#endif

            if (gcmIS_SUCCESS(status))
            {
                /* Done. */
                break;
            }

            /* 3. Try resolve clear if tile status is disabled.
             * resolve clear is better than below draw clear when no tile status.
            */
            status = _ClearRect_v2(SurfView, ClearArgs, LayerIndex);
            if (gcmIS_SUCCESS(status))
            {
                break;
            }

            if (gcoSURF_IsRenderable(surf) == gcvSTATUS_OK)
            {
                /* 4. Try 3D draw clear.
                 * Resolve clear will need to decompress and disable tile status
                 * before the actual clear. So 3D draw clear is better if tile
                 * status is currently enabled.
                 *
                 * Another thing is that when draw clear succeeds, all layers of the
                 * surface are cleared at the same time.
                 */

                gcsSURF_VIEW *rtView = (ClearArgs->flags & gcvCLEAR_COLOR) ? SurfView : gcvNULL;
                gcsSURF_VIEW *dsView = (ClearArgs->flags & gcvCLEAR_COLOR) ? gcvNULL  : SurfView;

                status = gcoHARDWARE_DrawClear_v2(rtView, dsView, ClearArgs);

                if (gcmIS_SUCCESS(status))
                {
                    /* Report the flag to caller, which means all layers of the surface are cleared. */
                    *BlitDraw = gcvTRUE;
                    break;
                }
            }

            /* 6. Last, use software clear.
             *    If no GPU-ONLY requested, try software clear.
            */
            savedFlags = ClearArgs->flags;
            ClearArgs->flags &= ~gcvCLEAR_WITH_GPU_ONLY;
            ClearArgs->flags |= gcvCLEAR_WITH_CPU_ONLY;
            status = _ClearRect_v2(SurfView, ClearArgs, LayerIndex);
            ClearArgs->flags = savedFlags;
        } while (gcvFALSE);
    }

    if ((ClearArgs->flags & gcvCLEAR_STENCIL) && surfInfo->hasStencilComponent)
    {
        surfInfo->canDropStencilPlane = gcvFALSE;
    }

    if (gcmIS_SUCCESS(status) && fullSize)
    {
        /* Set garbagePadded=0 if full clear */
        if (surfInfo->paddingFormat)
        {
            surfInfo->garbagePadded = gcvFALSE;
        }

        if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_PE_DITHER_FIX) == gcvFALSE)
        {
            gctUINT8 clearMask = surfInfo->clearMask[LayerIndex];

            /* Full mask clear can reset the deferDither3D flag */
            if ((clearMask == 0xF) ||
                (clearMask == 0x7 && (surfInfo->format == gcvSURF_X8R8G8B8 || surfInfo->format == gcvSURF_R5G6B5)) ||
                (clearMask == 0xE && surfInfo->hasStencilComponent && surfInfo->canDropStencilPlane)
               )
            {
                surfInfo->deferDither3D = gcvFALSE;
            }
        }
    }

OnError:
    if (surfAddr[0])
    {
        gcmVERIFY_OK(gcoSURF_Unlock(surf, gcvNULL));
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}

static gceSTATUS
_3DBlitClearRect_v2(
    IN gcsSURF_VIEW *SurfView,
    IN gcsSURF_CLEAR_ARGS_PTR ClearArgs,
    IN gctUINT32 LayerIndex
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsPOINT origin, rectSize;
    gcs3DBLIT_INFO clearInfo = {0};
    gcs3DBLIT_INFO hzClearInfo = {0};
    gctBOOL fastClear = gcvTRUE;
    gcsRECT_PTR rect = ClearArgs->clearRect;

    gcoSURF surf = SurfView->surf;
    gcsSURF_INFO_PTR surfInfo = &surf->info;

    gctBOOL clearHZ = ((ClearArgs->flags & gcvCLEAR_DEPTH) && surfInfo->hzNode.pool != gcvPOOL_UNKNOWN);

    gcmHEADER_ARG("SurfView=0x%x ClearArg=0x%x LayerIndex=%d",
                   SurfView, ClearArgs, LayerIndex);

    origin.x = ClearArgs->clearRect->left;
    origin.y = ClearArgs->clearRect->top;
    rectSize.x = ClearArgs->clearRect->right - ClearArgs->clearRect->left;
    rectSize.y = ClearArgs->clearRect->bottom - ClearArgs->clearRect->top;

    if ((ClearArgs->flags & gcvCLEAR_STENCIL) && surfInfo->hasStencilComponent)
    {
        surfInfo->canDropStencilPlane = gcvFALSE;
    }

    /* Compute clear values. */
    gcmONERROR(_ComputeClear(surfInfo, ClearArgs, LayerIndex));

    /* Prepare clearInfo. */
    clearInfo.clearValue[0] = surfInfo->clearValue[LayerIndex];
    clearInfo.clearValue[1] = surfInfo->clearValueUpper[LayerIndex];
    clearInfo.clearMask = surfInfo->clearBitMask[LayerIndex];
    clearInfo.clearMaskUpper = surfInfo->clearBitMaskUpper[LayerIndex];
    gcmGETHARDWAREADDRESS(surfInfo->node, clearInfo.destAddress);
    clearInfo.destAddress += LayerIndex * surfInfo->layerSize
                          +  SurfView->firstSlice * surfInfo->sliceSize;
    gcmGETHARDWAREADDRESS(surfInfo->tileStatusNode, clearInfo.destTileStatusAddress);
    clearInfo.origin = &origin;
    clearInfo.rect = &rectSize;
    if (clearInfo.clearMask == 0 && clearInfo.clearMaskUpper == 0)
    {
        gcmFOOTER();
        return gcvSTATUS_OK;
    }

    /* Test for entire surface clear. */
    if ((rect->left == 0)  &&  (rect->top == 0)
    &&  (rect->right  >= (gctINT)surfInfo->requestW)
    &&  (rect->bottom >= (gctINT)surfInfo->requestH)
    &&  (clearInfo.clearMask == 0xFFFFFFFF && clearInfo.clearMaskUpper == 0xFFFFFFFF)
    )
    {
        clearInfo.fcClearValue[0] = clearInfo.clearValue[0];
        clearInfo.fcClearValue[1] = clearInfo.clearValue[1];
    }
    else
    {
        clearInfo.fcClearValue[0] = surfInfo->fcValue;
        clearInfo.fcClearValue[1] = surfInfo->fcValueUpper;
    }

    if (clearHZ)
    {
       /* we don't expect to support HZ anymore in hardware with 3dblit for now */
        gcmASSERT(0);
        /* Prepare hzClearInfo. */
        hzClearInfo.clearValue[0] = surfInfo->clearValueHz;
        hzClearInfo.clearValue[1] = surfInfo->clearValueHz;
        hzClearInfo.fcClearValue[0] = hzClearInfo.clearValue[0];
        hzClearInfo.fcClearValue[1] = hzClearInfo.clearValue[1];
        hzClearInfo.clearMask = _ByteMaskToBitMask(0xF);
        gcmGETHARDWAREADDRESS(surfInfo->hzNode, hzClearInfo.destAddress);
        gcmGETHARDWAREADDRESS(surfInfo->hzTileStatusNode, hzClearInfo.destTileStatusAddress);
        hzClearInfo.origin = &origin;
        hzClearInfo.rect = &rectSize;
    }

    fastClear &= !surfInfo->tileStatusDisabled;

    if (clearHZ)
    {
        fastClear = (surfInfo->hzTileStatusNode.pool != gcvPOOL_UNKNOWN);
    }

    if ((surfInfo->bitsPerPixel == 8) &&
        (surfInfo->isMsaa) &&
        !gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_BLT_8bit_256TILE_FC_FIX))
    {
        fastClear = gcvFALSE;
    }

    /* Flush the tile status cache. */
    gcmONERROR(gcoHARDWARE_SelectPipe(gcvNULL, gcvPIPE_3D, gcvNULL));
    gcmONERROR(gcoHARDWARE_FlushTileStatus(gcvNULL, surfInfo, gcvFALSE));
    gcmONERROR(gcoHARDWARE_FlushPipe(gcvNULL, gcvNULL));

    if (!fastClear)
    {
        clearInfo.destTileStatusAddress = 0;

        if (clearHZ)
        {
            hzClearInfo.destTileStatusAddress = 0;
        }
        gcmONERROR(gcoSURF_DisableTileStatus(surf, gcvTRUE));
    }

    /* Clear. */
    if ((surfInfo->bitsPerPixel >= 64) &&
        !gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_BLT_64bpp_MASKED_CLEAR_FIX) &&
        (surfInfo->tileStatusNode.pool == gcvPOOL_UNKNOWN) &&
        (surfInfo->clearBitMask[LayerIndex] != surfInfo->clearBitMaskUpper[LayerIndex]))
    {
        status = gcoHARDWARE_ClearSoftware_v2(gcvNULL,
                                              SurfView,
                                              LayerIndex,
                                              rect,
                                              surfInfo->clearValue[LayerIndex],
                                              surfInfo->clearValueUpper[LayerIndex],
                                              ClearArgs->colorMask,
                                              0);
    }
    else
    {
        gcmONERROR(gcoHARDWARE_3DBlitClear(gcvNULL, gcvENGINE_RENDER, surfInfo, &clearInfo, &origin, &rectSize));
    }

    if (clearHZ)
    {
        /* Clear HZ. */
        gcmONERROR(gcoHARDWARE_3DBlitClear(gcvNULL, gcvENGINE_RENDER, surfInfo, &hzClearInfo, &origin, &rectSize));
    }

    if (fastClear)
    {
        /* Record FC value. */
        surfInfo->fcValue = clearInfo.fcClearValue[0];
        surfInfo->fcValueUpper = clearInfo.fcClearValue[1];

        if (clearHZ)
        {
            /* Record HZ FC value. */
            surfInfo->fcValueHz = hzClearInfo.clearValue[0];
        }

        /* Turn the tile status on again. */
        surfInfo->tileStatusDisabled = gcvFALSE;

        /* Reset the tile status. */
        gcmONERROR(gcoSURF_EnableTileStatus(surf));
    }

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoSURF_Clear_v2(
    IN gcsSURF_VIEW *SurfView,
    IN gcsSURF_CLEAR_ARGS_PTR ClearArgs
    )
{
    gctUINT layerIndex;
    gcsRECT adjustRect;
    gcsSURF_CLEAR_ARGS newArgs;
    gcoSURF surf = SurfView->surf;
    gcsSURF_INFO_PTR surfInfo;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("SurfView=0x%x ClearArgs=0x%x", SurfView, ClearArgs);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(surf, gcvOBJ_SURF);
    gcmVERIFY_ARGUMENT(ClearArgs != 0);

    surfInfo = &surf->info;

    gcoOS_MemCopy(&newArgs, ClearArgs, sizeof(gcsSURF_CLEAR_ARGS));
    if (ClearArgs->clearRect == gcvNULL)
    {
        /* Full screen. */
        adjustRect.left = 0;
        adjustRect.top  = 0;
        adjustRect.right  = surfInfo->requestW;
        adjustRect.bottom = surfInfo->requestH;
    }
    else
    {
        /* Intersect with surface size. */
        adjustRect.left   = gcmMAX(0, ClearArgs->clearRect->left);
        adjustRect.top    = gcmMAX(0, ClearArgs->clearRect->top);
        adjustRect.right  = gcmMIN((gctINT)surfInfo->requestW, ClearArgs->clearRect->right);
        adjustRect.bottom = gcmMIN((gctINT)surfInfo->requestH, ClearArgs->clearRect->bottom);

        /* Skip clear for invalid rect */
        if ((adjustRect.left >= adjustRect.right) || (adjustRect.top >= adjustRect.bottom))
        {
            goto OnExit;
        }
    }
    newArgs.clearRect = &adjustRect;

    if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_BLT_ENGINE))
    {
        gcsSURF_VIEW tmpView;

        if (ClearArgs->clearRect == gcvNULL)
        {
            surfInfo->dither3D = gcvFALSE;
        }

        tmpView.surf = surf;
        tmpView.numSlices = 1;
        for (layerIndex = 0; layerIndex < surfInfo->formatInfo.layers; layerIndex++)
        {
            if (ClearArgs->flags & gcvCLEAR_MULTI_SLICES)
            {
                gctUINT i;
                for (i = 0; i < surfInfo->requestD; i++)
                {
                    tmpView.firstSlice = i;
                    gcmONERROR(_3DBlitClearRect_v2(&tmpView, &newArgs, layerIndex));
                }
            }
            else
            {
                gcmONERROR(_3DBlitClearRect_v2(SurfView, &newArgs, layerIndex));
            }
        }
    }
    else
    {
        gcoHARDWARE_SetHWSlot(gcvNULL, gcvENGINE_RENDER, gcvHWSLOT_BLT_DST, surfInfo->node.u.normal.node, 0);

        gcmASSERT(!(ClearArgs->flags & gcvCLEAR_MULTI_SLICES));

        for (layerIndex = 0; layerIndex < surfInfo->formatInfo.layers; layerIndex++)
        {
            gctBOOL blitDraw = gcvFALSE;

            status = _Clear_v2(SurfView, &newArgs, layerIndex, &blitDraw);

            if (status == gcvSTATUS_NOT_ALIGNED)
            {
                goto OnError;
            }

            gcmONERROR(status);

            if (blitDraw)
            {
                break;
            }
        }
    }

OnExit:
OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  depr_gcoSURF_Resolve
**
**  Resolve the surface to the frame buffer.  Resolve means that the frame is
**  finished and should be displayed into the frame buffer, either by copying
**  the data or by flipping to the surface, depending on the hardware's
**  capabilities.
**
**  INPUT:
**
**      gcoSURF SrcSurface
**          Pointer to a gcoSURF object that represents the source surface
**          to be resolved.
**
**      gcoSURF DestSurface
**          Pointer to a gcoSURF object that represents the destination surface
**          to resolve into, or gcvNULL if the resolve surface is
**          a physical address.
**
**      gctUINT32 DestAddress
**          Physical address of the destination surface.
**
**      gctPOINTER DestBits
**          Logical address of the destination surface.
**
**      gctINT DestStride
**          Stride of the destination surface.
**          If 'DestSurface' is not gcvNULL, this parameter is ignored.
**
**      gceSURF_TYPE DestType
**          Type of the destination surface.
**          If 'DestSurface' is not gcvNULL, this parameter is ignored.
**
**      gceSURF_FORMAT DestFormat
**          Format of the destination surface.
**          If 'DestSurface' is not gcvNULL, this parameter is ignored.
**
**      gctUINT DestWidth
**          Width of the destination surface.
**          If 'DestSurface' is not gcvNULL, this parameter is ignored.
**
**      gctUINT DestHeight
**          Height of the destination surface.
**          If 'DestSurface' is not gcvNULL, this parameter is ignored.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
depr_gcoSURF_Resolve(
    IN gcoSURF SrcSurface,
    IN gcoSURF DestSurface,
    IN gctUINT32 DestAddress,
    IN gctPOINTER DestBits,
    IN gctINT DestStride,
    IN gceSURF_TYPE DestType,
    IN gceSURF_FORMAT DestFormat,
    IN gctUINT DestWidth,
    IN gctUINT DestHeight
    )
{
    gcsPOINT rectOrigin;
    gcsPOINT rectSize;
    gceSTATUS status;

    gcmHEADER_ARG("SrcSurface=0x%x DestSurface=0x%x DestAddress=%08x DestBits=0x%x "
              "DestStride=%d DestType=%d DestFormat=%d DestWidth=%u "
              "DestHeight=%u",
              SrcSurface, DestSurface, DestAddress, DestBits, DestStride,
              DestType, DestFormat, DestWidth, DestHeight);

    /* Validate the source surface. */
    gcmVERIFY_OBJECT(SrcSurface, gcvOBJ_SURF);

    /* Fill in coordinates. */
    rectOrigin.x = 0;
    rectOrigin.y = 0;

    if (DestSurface == gcvNULL)
    {
        rectSize.x = DestWidth;
        rectSize.y = DestHeight;
    }
    else
    {
        rectSize.x = DestSurface->info.alignedW;
        rectSize.y = DestSurface->info.alignedH;
    }

    /* Call generic function. */
    status = depr_gcoSURF_ResolveRect(
        SrcSurface, DestSurface,
        DestAddress, DestBits, DestStride, DestType, DestFormat,
        DestWidth, DestHeight,
        &rectOrigin, &rectOrigin, &rectSize
        );
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  depr_gcoSURF_ResolveRect
**
**  Resolve a rectangular area of a surface to another surface.
**
**  INPUT:
**
**      gcoSURF SrcSurface
**          Pointer to a gcoSURF object that represents the source surface
**          to be resolved.
**
**      gcoSURF DestSurface
**          Pointer to a gcoSURF object that represents the destination surface
**          to resolve into, or gcvNULL if the resolve surface is
**          a physical address.
**
**      gctUINT32 DestAddress
**          Physical address of the destination surface.
**          If 'DestSurface' is not gcvNULL, this parameter is ignored.
**
**      gctPOINTER DestBits
**          Logical address of the destination surface.
**
**      gctINT DestStride
**          Stride of the destination surface.
**          If 'DestSurface' is not gcvNULL, this parameter is ignored.
**
**      gceSURF_TYPE DestType
**          Type of the destination surface.
**          If 'DestSurface' is not gcvNULL, this parameter is ignored.
**
**      gceSURF_FORMAT DestFormat
**          Format of the destination surface.
**          If 'DestSurface' is not gcvNULL, this parameter is ignored.
**
**      gctUINT DestWidth
**          Width of the destination surface.
**          If 'DestSurface' is not gcvNULL, this parameter is ignored.
**
**      gctUINT DestHeight
**          Height of the destination surface.
**          If 'DestSurface' is not gcvNULL, this parameter is ignored.
**
**      gcsPOINT_PTR SrcOrigin
**          The origin of the source area to be resolved.
**
**      gcsPOINT_PTR DestOrigin
**          The origin of the destination area to be resolved.
**
**      gcsPOINT_PTR RectSize
**          The size of the rectangular area to be resolved.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
depr_gcoSURF_ResolveRect(
    IN gcoSURF SrcSurface,
    IN gcoSURF DestSurface,
    IN gctUINT32 DestAddress,
    IN gctPOINTER DestBits,
    IN gctINT DestStride,
    IN gceSURF_TYPE DestType,
    IN gceSURF_FORMAT DestFormat,
    IN gctUINT DestWidth,
    IN gctUINT DestHeight,
    IN gcsPOINT_PTR SrcOrigin,
    IN gcsPOINT_PTR DestOrigin,
    IN gcsPOINT_PTR RectSize
    )
{
    gceSTATUS status;
    gctPOINTER destination[3] = {gcvNULL};
    gctPOINTER mapInfo = gcvNULL;
    gcsSURF_INFO_PTR dstInfo;
    struct _gcoSURF dstSurf;
    gctUINT32 address;

    gcmHEADER_ARG("SrcSurface=0x%x DestSurface=0x%x DestAddress=%08x DestBits=0x%x "
              "DestStride=%d DestType=%d DestFormat=%d DestWidth=%u "
              "DestHeight=%u SrcOrigin=0x%x DestOrigin=0x%x RectSize=0x%x",
              SrcSurface, DestSurface, DestAddress, DestBits, DestStride,
              DestType, DestFormat, DestWidth, SrcOrigin, DestOrigin,
              RectSize);

    /* Validate the source surface. */
    gcmVERIFY_OBJECT(SrcSurface, gcvOBJ_SURF);

    do
    {
        gcsSURF_VIEW srcView = {SrcSurface, 0, 1};
        gcsSURF_VIEW dstView = {DestSurface, 0, 1};
        gcsSURF_RESOLVE_ARGS rlvArgs = {0};

        /* Destination surface specified? */
        if (DestSurface != gcvNULL)
        {
            /* Set the pointer to the structure. */
            dstInfo = &DestSurface->info;

            /* Lock the surface. */
            if (DestBits == gcvNULL)
            {
                gcmERR_BREAK(
                    gcoSURF_Lock(DestSurface,
                                 gcvNULL,
                                 destination));
            }
        }

        /* Create a surface wrapper. */
        else
        {
            gcoOS_ZeroMemory(&dstSurf, gcmSIZEOF(dstSurf));

            dstInfo = &dstSurf.info;
            /* Fill the surface info structure. */
            dstInfo->type          = DestType;
            dstInfo->format        = DestFormat;
            dstInfo->requestW      = DestWidth;
            dstInfo->requestH      = DestHeight;
            dstInfo->requestD      = 1;
            dstInfo->allocedW      = DestWidth;
            dstInfo->allocedH      = DestHeight;
            dstInfo->alignedW      = DestWidth;
            dstInfo->alignedH      = DestHeight;
            dstInfo->rotation      = gcvSURF_0_DEGREE;
            dstInfo->orientation   = gcvORIENTATION_TOP_BOTTOM;
            dstInfo->stride        = DestStride;
            dstInfo->sliceSize     =
            dstInfo->layerSize     =
            dstInfo->size          = DestStride * DestHeight;
            dstInfo->node.valid    = gcvTRUE;
            dstInfo->node.pool     = gcvPOOL_UNKNOWN;
            gcsSURF_NODE_SetHardwareAddress(&(dstInfo->node), DestAddress);
            dstInfo->node.logical  = DestBits;
            dstInfo->sampleInfo    = g_sampleInfos[1];
            dstInfo->isMsaa        = gcvFALSE;
            dstInfo->edgeAA        = gcvFALSE;
            dstInfo->colorSpace    = gcd_QUERY_COLOR_SPACE(DestFormat);

            gcmERR_BREAK(gcoHARDWARE_ConvertFormat(DestFormat, &dstInfo->bitsPerPixel, gcvNULL));
            gcmERR_BREAK(
                gcoHARDWARE_AlignToTileCompatible(gcvNULL,
                                                  DestType,
                                                  0,
                                                  DestFormat,
                                                  &dstInfo->alignedW,
                                                  &dstInfo->alignedH,
                                                  1,
                                                  &dstInfo->tiling,
                                                  &dstInfo->superTiled,
                                                  &dstInfo->hAlignment));

            /* Map the user memory. */
            if (DestBits != gcvNULL)
            {
                gcmERR_BREAK(
                    gcoOS_MapUserMemory(gcvNULL,
                                        DestBits,
                                        dstInfo->size,
                                        &mapInfo,
                                        &address));

                gcsSURF_NODE_SetHardwareAddress(&dstInfo->node, address);
            }

            dstInfo->pfGetAddr = gcoHARDWARE_GetProcCalcPixelAddr(gcvNULL, dstInfo);
            dstView.surf = &dstSurf;
        }

        rlvArgs.version = gcvHAL_ARG_VERSION_V2;
        rlvArgs.uArgs.v2.numSlices = 1;
        rlvArgs.uArgs.v2.srcOrigin = *SrcOrigin;
        rlvArgs.uArgs.v2.dstOrigin = *DestOrigin;

        /* Determine the resolve size. */
        if ((DestOrigin->x == 0)
        &&  (DestOrigin->y == 0)
        &&  (RectSize->x == (gctINT)dstInfo->requestW)
        &&  (RectSize->y == (gctINT)dstInfo->requestH)
        )
        {
            /* Full destination resolve, a special case. */
            rlvArgs.uArgs.v2.rectSize.x = dstInfo->alignedW;
            rlvArgs.uArgs.v2.rectSize.y = dstInfo->alignedH;
        }
        else
        {
            rlvArgs.uArgs.v2.rectSize.x = RectSize->x;
            rlvArgs.uArgs.v2.rectSize.y = RectSize->y;
        }

        /* Perform resolve. */
        gcmERR_BREAK(gcoHARDWARE_ResolveRect_v2(gcvNULL, &srcView, &dstView, &rlvArgs));
    }
    while (gcvFALSE);

    /* Unlock the surface. */
    if (destination[0] != gcvNULL)
    {
        /* Unlock the resolve buffer. */
        gcmVERIFY_OK(
            gcoSURF_Unlock(DestSurface,
                           destination[0]));
    }

    /* Unmap the surface. */
    if (mapInfo != gcvNULL)
    {
        gcmGETHARDWAREADDRESS(dstInfo->node, address);

        /* Unmap the user memory. */
        gcmVERIFY_OK(
            gcoHAL_ScheduleUnmapUserMemory(gcvNULL,
                                           mapInfo,
                                           dstInfo->size,
                                           address,
                                           DestBits));
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}

#define gcdCOLOR_SPACE_CONVERSION_NONE         0
#define gcdCOLOR_SPACE_CONVERSION_TO_LINEAR    1
#define gcdCOLOR_SPACE_CONVERSION_TO_NONLINEAR 2

gceSTATUS
gcoSURF_MixSurfacesCPU(
    IN gcoSURF TargetSurface,
    IN gctUINT TargetSliceIndex,
    IN gcoSURF *SourceSurface,
    IN gctUINT *SourceSliceIndices,
    IN gctFLOAT *Weights,
    IN gctINT Count
    )
{
    gceSTATUS status;
    gcoSURF srcSurf = gcvNULL, dstSurf = gcvNULL;
    gctPOINTER srcAddr[3] = {gcvNULL};
    gctPOINTER dstAddr[3] = {gcvNULL};
    _PFNreadPixel pfReadPixel = gcvNULL;
    _PFNwritePixel pfWritePixel = gcvNULL;
    gctUINT defaultSliceIndices[MAX_SURF_MIX_SRC_NUM] = { 0 };

    gctUINT i, j, k;
    gcsSURF_FORMAT_INFO *srcFmtInfo, *dstFmtInfo;
    gctPOINTER srcAddr_l[gcdMAX_SURF_LAYERS];
    gctPOINTER dstAddr_l[gcdMAX_SURF_LAYERS];
    gcsPIXEL internalSrc, internalDst;

    dstSurf = TargetSurface;
    dstFmtInfo = &dstSurf->info.formatInfo;

    /* MSAA case not supported. */
    if (dstSurf->info.sampleInfo.product > 1)
    {
        return gcvSTATUS_NOT_SUPPORTED;
    }

    for (k = 0; k < (gctUINT)Count; ++k)
    {
        srcSurf = SourceSurface[k];
        srcFmtInfo = &SourceSurface[k]->info.formatInfo;

        /* Target and Source surfaces need to have same dimensions and format. */
        if ((dstSurf->info.requestW != srcSurf->info.requestW)
         || (dstSurf->info.requestH != srcSurf->info.requestH)
         || (dstSurf->info.format != srcSurf->info.format)
         || (dstSurf->info.type != srcSurf->info.type)
         || (dstSurf->info.tiling != srcSurf->info.tiling))
        {
            return gcvSTATUS_INVALID_ARGUMENT;
        }

        /* MSAA case not supported. */
        if (srcSurf->info.sampleInfo.product > 1)
        {
            return gcvSTATUS_NOT_SUPPORTED;
        }

        /*
        ** Integer format upload/blit, the data type must be totally matched.
        */
        if (((srcFmtInfo->fmtDataType == gcvFORMAT_DATATYPE_UNSIGNED_INTEGER) ||
             (srcFmtInfo->fmtDataType == gcvFORMAT_DATATYPE_SIGNED_INTEGER)) &&
             (srcFmtInfo->fmtDataType != dstFmtInfo->fmtDataType))
        {
            return gcvSTATUS_NOT_SUPPORTED;
        }

    }

    if ( SourceSliceIndices == gcvNULL )
    {
        SourceSliceIndices = defaultSliceIndices;
    }

    /* TODO: those function pointers can be recorded in gcoSURF */
    pfReadPixel  = gcoSURF_GetReadPixelFunc(srcSurf);
    pfWritePixel = gcoSURF_GetWritePixelFunc(dstSurf);

    if (!pfReadPixel || !pfWritePixel)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    /* Synchronize with the GPU. */
    /* TODO: if both of the surfaces previously were not write by GPU,
    ** or already did the sync, no need to do it again.
    */
    gcmONERROR(gcoHARDWARE_FlushPipe(gcvNULL, gcvNULL));
    gcmONERROR(gcoHARDWARE_Commit(gcvNULL));
    gcmONERROR(gcoHARDWARE_Stall(gcvNULL));

    gcmONERROR(gcoHARDWARE_DisableTileStatus(gcvNULL, &dstSurf->info, gcvTRUE));
    gcmONERROR(gcoSURF_Lock(dstSurf, gcvNULL, dstAddr));
    gcmONERROR(gcoSURF_NODE_Cache(&dstSurf->info.node,
                                  dstAddr[0],
                                  dstSurf->info.size,
                                  gcvCACHE_INVALIDATE));

    for (k = 0; k < (gctUINT)Count; ++k)
    {
        srcSurf = SourceSurface[k];
        srcFmtInfo = &SourceSurface[k]->info.formatInfo;

        /* set color space conversion flag */
        gcmASSERT(srcSurf->info.colorSpace == dstSurf->info.colorSpace);

        /* Flush the GPU cache */
        gcmONERROR(gcoHARDWARE_FlushTileStatus(gcvNULL, &srcSurf->info, gcvTRUE));
        /* Lock the surfaces. */
        gcmONERROR(gcoSURF_Lock(srcSurf, gcvNULL, srcAddr));
        gcmONERROR(gcoSURF_NODE_Cache(&srcSurf->info.node,
                                      srcAddr[0],
                                      srcSurf->info.size,
                                      gcvCACHE_INVALIDATE));
    }

    for (j = 0; j < srcSurf->info.requestH; ++j)
    {
        for (i = 0; i < srcSurf->info.requestW; ++i)
        {
            internalDst.color.f.r = 0;
            internalDst.color.f.g = 0;
            internalDst.color.f.b = 0;
            internalDst.color.f.a = 0;
            internalDst.d = 0;
            internalDst.s = 0;

            for (k = 0; k < (gctUINT)Count; ++k)
            {
                gctFLOAT    factor = Weights[k];
                gctUINT     z = SourceSliceIndices[k];

                srcSurf = SourceSurface[k];
                srcFmtInfo = &SourceSurface[k]->info.formatInfo;

                internalSrc.color.f.r = 0;
                internalSrc.color.f.g = 0;
                internalSrc.color.f.b = 0;
                internalSrc.color.f.a = 0;
                internalSrc.d = 0;
                internalSrc.s = 0;

                srcSurf->info.pfGetAddr(&srcSurf->info, (gctSIZE_T)i, (gctSIZE_T)j, (gctSIZE_T)z, srcAddr_l);
                pfReadPixel(srcAddr_l, &internalSrc);

                if (srcSurf->info.colorSpace == gcvSURF_COLOR_SPACE_NONLINEAR)
                {
                    gcoSURF_PixelToLinear(&internalSrc);
                }

                /* Mix the pixels. */
                internalDst.color.f.r += internalSrc.color.f.r * factor;
                internalDst.color.f.g += internalSrc.color.f.g * factor;
                internalDst.color.f.b += internalSrc.color.f.b * factor;
                internalDst.color.f.a += internalSrc.color.f.a * factor;
                internalDst.d += internalSrc.d * factor;
                internalDst.s += (gctUINT32)(internalSrc.s * factor);

            }

            if (srcSurf->info.colorSpace == gcvSURF_COLOR_SPACE_NONLINEAR)
            {
                gcoSURF_PixelToNonLinear(&internalDst);
            }

            dstSurf->info.pfGetAddr(&dstSurf->info, (gctSIZE_T)i, (gctSIZE_T)j, (gctSIZE_T)TargetSliceIndex, dstAddr_l);
            pfWritePixel(&internalDst, dstAddr_l, 0);

        }
    }

    /* Dst surface was written by CPU and might be accessed by GPU later */
    gcmONERROR(gcoSURF_NODE_Cache(&dstSurf->info.node,
                                  dstAddr[0],
                                  dstSurf->info.size,
                                  gcvCACHE_CLEAN));

#if gcdDUMP
        gcmDUMP(gcvNULL, "#verify mix surface source");
        /* verify the source */
        gcmDUMP_BUFFER(gcvNULL,
                       "verify",
                       gcsSURF_NODE_GetHWAddress(&srcSurf->info.node),
                       srcSurf->info.node.logical,
                       0,
                       srcSurf->info.size);
        /* upload the destination */
        gcmDUMP_BUFFER(gcvNULL,
                       "memory",
                       gcsSURF_NODE_GetHWAddress(&dstSurf->info.node),
                       dstSurf->info.node.logical,
                       0,
                       dstSurf->info.size);
#endif

OnError:
    /* Unlock the surfaces. */
    gcoSURF_Unlock(dstSurf, gcvNULL);

    for ( k = 0; k < (gctUINT)Count; ++k )
    {
        srcSurf = SourceSurface[k];

        /* Lock the surfaces. */
        gcoSURF_Unlock(srcSurf, gcvNULL);
    }

    return gcvSTATUS_OK;
}

gceSTATUS
gcoSURF_Resample_v2(
    IN gcoSURF SrcSurf,
    IN gcoSURF DstSurf
    )
{
    gctUINT i;
    gcsSURF_INFO_PTR srcInfo, dstInfo;
    gcsSURF_RESOLVE_ARGS rlvArgs = {0};
    gcsSURF_VIEW srcView, dstView;
    gcoSURF tmpSurf = gcvNULL;
    gcsPOINT rectSize = {0, 0};
    gcsSAMPLES srcSampleInfo = {1, 1, 1};
    gcsSAMPLES dstSampleInfo = {1, 1, 1};
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("SrcSurf=0x%x DstSurf=0x%x", SrcSurf, DstSurf);

    /* Validate the surfaces. */
    gcmVERIFY_OBJECT(SrcSurf, gcvOBJ_SURF);
    gcmVERIFY_OBJECT(DstSurf, gcvOBJ_SURF);

    srcInfo = &SrcSurf->info;
    dstInfo = &DstSurf->info;

    /* Both surfaces have to be non-multisampled. */
    if (srcInfo->isMsaa || dstInfo->isMsaa)
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_BLT_ENGINE))
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

#if gcdDUMP
    {
        gctSTRING env;
        gctINT simFrame = 0;
        gctUINT frameCount;
        gcoOS_GetEnv(gcvNULL, "SIM_Frame", &env);
        gcoHAL_FrameInfoOps(gcvNULL,
                            gcvFRAMEINFO_FRAME_NUM,
                            gcvFRAMEINFO_OP_GET,
                            &frameCount);
        if (env)
        {
            gcoOS_StrToInt(env, &simFrame);
        }

        if ((gctINT)frameCount < simFrame)
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }
    }
#endif

    /* Determine the samples and fill in coordinates. */
    if (srcInfo->requestW == dstInfo->requestW)
    {
        srcSampleInfo.x = 1;
        dstSampleInfo.x = 1;
        rectSize.x      = srcInfo->requestW;
    }
    else if (srcInfo->requestW / 2 == dstInfo->requestW)
    {
        srcSampleInfo.x = 2;
        dstSampleInfo.x = 1;
        rectSize.x      = dstInfo->requestW;
    }
    else if (srcInfo->requestW == dstInfo->requestW / 2)
    {
        srcSampleInfo.x = 1;
        dstSampleInfo.x = 2;
        rectSize.x      = srcInfo->requestW;
    }
    else
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    if (srcInfo->requestH == dstInfo->requestH)
    {
        srcSampleInfo.y = 1;
        dstSampleInfo.y = 1;
        rectSize.y      = srcInfo->requestH;
    }
    else if (srcInfo->requestH / 2 == dstInfo->requestH)
    {
        srcSampleInfo.y = 2;
        dstSampleInfo.y = 1;
        rectSize.y      = dstInfo->requestH;
    }
    else if (srcInfo->requestH == dstInfo->requestH / 2)
    {
        srcSampleInfo.y = 1;
        dstSampleInfo.y = 2;
        rectSize.y      = srcInfo->requestH;
    }
    else
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }
    srcSampleInfo.product = srcSampleInfo.x * srcSampleInfo.y;
    dstSampleInfo.product = dstSampleInfo.x * dstSampleInfo.y;

    /* Overwrite surface samples.
    ** Note: isMsaa flag was still kept as 0, to indicate it's not real msaa buf and no need to set cache256 down
    */
    srcInfo->sampleInfo = srcSampleInfo;
    dstInfo->sampleInfo = dstSampleInfo;

    srcView.surf = SrcSurf;
    srcView.numSlices = 1;
    dstView.surf = DstSurf;
    dstView.numSlices = 1;

    gcoOS_ZeroMemory(&rlvArgs, gcmSIZEOF(rlvArgs));
    rlvArgs.version = gcvHAL_ARG_VERSION_V2;
    rlvArgs.uArgs.v2.resample  = gcvTRUE;
    rlvArgs.uArgs.v2.rectSize  = rectSize;
    rlvArgs.uArgs.v2.numSlices = 1;

    /* 3D texture. */
    if (srcInfo->requestD != dstInfo->requestD)
    {
        gcsSURF_VIEW tmpView;
        gcoSURF mixSrcSurfs[2];
        gctUINT mixSrcSliceIndices[2] = {0};
        gctFLOAT mixSrcWeights[2] = {0.5f, 0.5f};

        /* Need to be reducing depth, with resample. */
        if (srcInfo->requestD < dstInfo->requestD)
        {
            gcmONERROR(gcvSTATUS_INVALID_REQUEST);
        }

        gcoSURF_Construct(gcvNULL,
                          dstInfo->requestW,
                          dstInfo->requestH,
                          1,
                          dstInfo->type,
                          dstInfo->format,
                          gcvPOOL_DEFAULT,
                          &tmpSurf);

        tmpView.surf = tmpSurf;
        tmpView.firstSlice = 0;
        tmpView.numSlices = 1;

        mixSrcSurfs[0] = DstSurf;
        mixSrcSurfs[1] = tmpSurf;

        /* Downsample 2 slices, and mix them together.
        ** Ignore the last Src slice, if the size is odd.
        */
        for (i = 0; i < dstInfo->requestD; ++i)
        {
            dstView.firstSlice = i;
            mixSrcSliceIndices[0] = dstView.firstSlice;

            srcView.firstSlice = 2 * i;
            gcmONERROR(gcoSURF_ResolveRect_v2(&srcView, &dstView, &rlvArgs));

            srcView.firstSlice = 2 * i + 1;
            gcmONERROR(gcoSURF_ResolveRect_v2(&srcView, &tmpView, &rlvArgs));

            gcmONERROR(gcoSURF_MixSurfacesCPU(DstSurf, dstView.firstSlice,
                                              mixSrcSurfs, mixSrcSliceIndices,
                                              mixSrcWeights, 2));
        }
    }
    else
    {
        for (i = 0; i < srcInfo->requestD; ++i)
        {
            srcView.firstSlice = dstView.firstSlice = i;
            gcmONERROR(gcoSURF_ResolveRect_v2(&srcView, &dstView, &rlvArgs));
        }
    }

OnError:
    /* Restore samples. */
    srcInfo->sampleInfo = g_sampleInfos[1];
    dstInfo->sampleInfo = g_sampleInfos[1];

    if (tmpSurf != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(tmpSurf));
    }

    /* Fallback to CPU resampling */
    if (gcmIS_ERROR(status))
    {
        gcsSURF_BLIT_ARGS blitArgs;

#if !gcdDUMP
        gcePATCH_ID patchID = gcvPATCH_INVALID;
        gcoHAL_GetPatchID(gcvNULL, &patchID);

        if (patchID != gcvPATCH_GFXBENCH)
#endif
        {
            gcoOS_ZeroMemory(&blitArgs, sizeof(blitArgs));
            blitArgs.srcX               = 0;
            blitArgs.srcY               = 0;
            blitArgs.srcZ               = 0;
            blitArgs.srcWidth           = srcInfo->requestW;
            blitArgs.srcHeight          = srcInfo->requestH;
            blitArgs.srcDepth           = srcInfo->requestD;
            blitArgs.srcSurface         = SrcSurf;
            blitArgs.dstX               = 0;
            blitArgs.dstY               = 0;
            blitArgs.dstZ               = 0;
            blitArgs.dstWidth           = dstInfo->requestW;
            blitArgs.dstHeight          = dstInfo->requestH;
            blitArgs.dstDepth           = dstInfo->requestD;
            blitArgs.dstSurface         = DstSurf;
            blitArgs.xReverse           = gcvFALSE;
            blitArgs.yReverse           = gcvFALSE;
            blitArgs.scissorTest        = gcvFALSE;
            status = gcoSURF_BlitCPU(&blitArgs);
        }
#if !gcdDUMP
        else
        {
            /* Skip SW GENERATION for low levels */
        }
#endif
    }

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_GetResolveAlignment
**
**  Query the resolve alignment of the hardware.
**
**  INPUT:
**
**      gcsSURF_INFO_PTR Surface,
**          Pointer to an surface info.
**
**  OUTPUT:
**
**      gctUINT *originX,
**          X direction origin alignment
**
**      gctUINT *originY
**          Y direction origin alignemnt
**
**      gctUINT *sizeX,
**          X direction size alignment
**
**      gctUINT *sizeY
**          Y direction size alignemnt
**
*/
gceSTATUS
gcoSURF_GetResolveAlignment(
    IN gcoSURF Surface,
    OUT gctUINT *originX,
    OUT gctUINT *originY,
    OUT gctUINT *sizeX,
    OUT gctUINT *sizeY
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x", Surface);

    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Query the tile sizes through gcoHARDWARE. */
    status = gcoHARDWARE_GetSurfaceResolveAlignment(gcvNULL, &Surface->info, originX, originY, sizeX, sizeY);

    gcmFOOTER_ARG("status=%d *originX=%d *originY=%d *sizeX=%d *sizeY=%d",
                  status, gcmOPT_VALUE(originX), gcmOPT_VALUE(originY),
                  gcmOPT_VALUE(sizeX), gcmOPT_VALUE(sizeY));
    return status;
}


gceSTATUS gcoSURF_TranslateRotationRect(
    gcsSIZE_PTR rtSize,
    gceSURF_ROTATION rotation,
    gcsRECT * rect
    )
{
    gctFLOAT tx, ty, tz, tw;
    gctFLOAT tmp;

    tx  = (gctFLOAT)rect->left;
    ty  = (gctFLOAT)rect->top;

    tz  = (gctFLOAT)rect->right;
    tw  = (gctFLOAT)rect->bottom;

    /* 1, translate to rt center */
    tx = tx - (gctFLOAT)rtSize->width / 2.0f;
    ty = ty - (gctFLOAT)rtSize->height / 2.0f;

    tz = tz - (gctFLOAT)rtSize->width / 2.0f;
    tw = tw - (gctFLOAT)rtSize->height / 2.0f;

    /* cos? -sin?    90D  x = -y  180D x = -x 270D x = y
       sin? cos?          y = x        y = -y      y = -x */

    switch (rotation)
    {
        case gcvSURF_90_DEGREE:
            /* 2, rotate */
            tmp = tx;
            tx  = -ty;
            ty  = tmp;

            tmp = tz;
            tz  = -tw;
            tw  = tmp;
            /* 3, translate back  */
            tx = tx + (gctFLOAT)rtSize->height / 2.0f;
            ty = ty + (gctFLOAT)rtSize->width / 2.0f;

            tz = tz + (gctFLOAT)rtSize->height / 2.0f;
            tw = tw + (gctFLOAT)rtSize->width / 2.0f;

            /* Form the new (left,top) (right,bottom) */
            rect->left   = (gctINT32)tz;
            rect->top    = (gctINT32)ty;
            rect->right  = (gctINT32)tx;
            rect->bottom = (gctINT32)tw;
            break;

        case gcvSURF_270_DEGREE:
            /* 2, rotate */
            tmp = tx;
            tx  = ty;
            ty  = -tmp;

            tmp = tz;
            tz  = tw;
            tw  = -tmp;
            /* 3, translate back  */
            tx = tx + (gctFLOAT)rtSize->height / 2.0f;
            ty = ty + (gctFLOAT)rtSize->width / 2.0f;

            tz = tz + (gctFLOAT)rtSize->height / 2.0f;
            tw = tw + (gctFLOAT)rtSize->width / 2.0f;

            /* Form the new (left,top) (right,bottom) */
            rect->left   = (gctINT32)tx;
            rect->top    = (gctINT32)tw;
            rect->right  = (gctINT32)tz;
            rect->bottom = (gctINT32)ty;
            break;

         default :
            break;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
gcoSURF_3DBlitBltRect_v2(
    IN gcsSURF_VIEW *SrcView,
    IN gcsSURF_VIEW *DstView,
    IN gcsSURF_RESOLVE_ARGS *Args
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsSURF_INFO_PTR srcInfo = &SrcView->surf->info;
    gcsSURF_INFO_PTR dstInfo = &DstView->surf->info;

    gcmHEADER_ARG("SrcView=0x%x DstView=0x%x Args=0x%x", SrcView, DstView, Args);

    if (Args->version != gcvHAL_ARG_VERSION_V2)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmONERROR(gcoHARDWARE_FlushTileStatus(gcvNULL, srcInfo, gcvFALSE));

    if (!dstInfo->tileStatusDisabled)
    {
        gcmONERROR(gcoHARDWARE_DisableTileStatus(gcvNULL, dstInfo, gcvTRUE));
    }

    gcmONERROR(gcoHARDWARE_3DBlitBlt_v2(gcvNULL, SrcView, DstView, Args));

OnError:
    gcmFOOTER();
    return status;
}


gceSTATUS
gcoSURF_ResolveRect_v2(
    IN gcsSURF_VIEW *SrcView,
    IN gcsSURF_VIEW *DstView,
    IN gcsSURF_RESOLVE_ARGS *Args
    )
{
    gctPOINTER srcBase[3] = {gcvNULL};
    gctPOINTER dstBase[3] = {gcvNULL};
    gcoSURF srcSurf = SrcView->surf;
    gcoSURF dstSurf = DstView->surf;
    gcsSURF_INFO_PTR srcInfo = &srcSurf->info;
    gcsSURF_INFO_PTR dstInfo = &dstSurf->info;
    gcsPOINT_PTR srcOrigin, dstOrigin;
    gceSURF_FORMAT savedSrcFmt = gcvSURF_UNKNOWN;
    gceFORMAT_DATATYPE savedSrcDataType = gcvFORMAT_DATATYPE_UNSIGNED_NORMALIZED;
    gcsSURF_RESOLVE_ARGS fullSizeArgs = {0};
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("SrcView=0x%x DstView=0x%x Args=0x%x", SrcView, DstView, Args);

    /* default full size resolve */
    if (!Args)
    {
        fullSizeArgs.version = gcvHAL_ARG_VERSION_V2;
        fullSizeArgs.uArgs.v2.rectSize.x = srcInfo->requestW;
        fullSizeArgs.uArgs.v2.rectSize.y = srcInfo->requestH;
        fullSizeArgs.uArgs.v2.numSlices  = 1;
        Args = &fullSizeArgs;
    }

    if (Args->version != gcvHAL_ARG_VERSION_V2)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (Args->uArgs.v2.directCopy && srcInfo->format != dstInfo->format)
    {
        /* This direct Copy only works for bpp same format, so fake to same format.
        ** TODO, fake compressed format to RGBA format
        */
        savedSrcFmt = srcInfo->format;
        srcInfo->format = dstInfo->format;

        savedSrcDataType = srcInfo->formatInfo.fmtDataType;
        srcInfo->formatInfo.fmtDataType = dstInfo->formatInfo.fmtDataType;
    }

    srcOrigin = &Args->uArgs.v2.srcOrigin;
    dstOrigin = &Args->uArgs.v2.dstOrigin;

    /* Lock the surfaces. */
    gcmONERROR(gcoSURF_Lock(srcSurf, gcvNULL, srcBase));
    gcmONERROR(gcoSURF_Lock(dstSurf, gcvNULL, dstBase));

    if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_BLT_ENGINE))
    {
        gcmASSERT(!Args->uArgs.v2.resample);

        Args->uArgs.v2.engine = gcvENGINE_RENDER;

        if ((srcInfo->format == gcvSURF_YV12) ||
            (srcInfo->format == gcvSURF_I420) ||
            (srcInfo->format == gcvSURF_NV12) ||
            (srcInfo->format == gcvSURF_NV21))
        {
            if ((gcvLINEAR == srcInfo->tiling) &&
                (gcvLINEAR != dstInfo->tiling) &&
                (dstInfo->format == gcvSURF_YUY2))
            {
                status = gcoHARDWARE_3DBlit420Tiler(gcvNULL,
                                                    gcvENGINE_RENDER,
                                                    srcInfo,
                                                    dstInfo,
                                                    srcOrigin,
                                                    dstOrigin,
                                                    &Args->uArgs.v2.rectSize);
            }
            else
            {
                status = gcvSTATUS_NOT_SUPPORTED;
            }
        }
        else if (gcmIS_ERROR(gcoSURF_3DBlitBltRect_v2(SrcView, DstView, Args)))
        {
            status = gcoSURF_CopyPixels_v2(SrcView, DstView, Args);
        }
    }
    else
    {
        gcsPOINT_PTR pAdjustRect;
        gcsSURF_RESOLVE_ARGS newArgs = {0};
        gctINT srcMaxW = srcInfo->alignedW - srcOrigin->x;
        gctINT srcMaxH = srcInfo->alignedH - srcOrigin->y;
        gctINT dstMaxW = dstInfo->alignedW - dstOrigin->x;
        gctINT dstMaxH = dstInfo->alignedH - dstOrigin->y;

        dstInfo->canDropStencilPlane = srcInfo->canDropStencilPlane;

        gcmONERROR(gcoHARDWARE_FlushTileStatus(gcvNULL, srcInfo, gcvFALSE));

        if (srcInfo->type == gcvSURF_BITMAP)
        {
            /* Flush the CPU cache. Source would've been rendered by the CPU. */
            gcmONERROR(gcoSURF_NODE_Cache(
                &srcInfo->node,
                srcBase[0],
                srcInfo->size,
                gcvCACHE_CLEAN));
        }

        if (dstInfo->type == gcvSURF_BITMAP)
        {
            gcmONERROR(gcoSURF_NODE_Cache(
                &dstInfo->node,
                dstBase[0],
                dstInfo->size,
                gcvCACHE_FLUSH));
        }

        /* Make sure we don't go beyond the src/dst surface. */

        gcoOS_MemCopy(&newArgs, Args, sizeof(newArgs));
        pAdjustRect = &newArgs.uArgs.v2.rectSize;
        if (Args->uArgs.v2.resample)
        {
            gcmASSERT(srcOrigin->x == dstOrigin->x && srcOrigin->y == dstOrigin->y);

            /* Determine the resolve size. */
            if ((dstOrigin->x == 0) &&
                (pAdjustRect->x == (gctINT)dstInfo->requestW) &&
                (dstInfo->alignedW <= srcInfo->alignedW / srcInfo->sampleInfo.x))
            {
                /* take destination aligned size only if source is MSAA-required aligned accordingly. */
                pAdjustRect->x = dstInfo->alignedW;
            }
            else
            {
                pAdjustRect->x = gcmMIN(dstMaxW, (gctINT)(srcInfo->alignedW / srcInfo->sampleInfo.x));
            }

            /* Determine the resolve size. */
            if ((dstOrigin->y == 0) &&
                (pAdjustRect->y == (gctINT)dstInfo->requestH) &&
                (dstInfo->alignedH <= srcInfo->alignedH / srcInfo->sampleInfo.y))
            {
                /* take destination aligned size only if source is MSAA-required aligned accordingly. */
                pAdjustRect->y = dstInfo->alignedH;
            }
            else
            {
                pAdjustRect->y = gcmMIN(dstMaxH, (gctINT)(srcInfo->alignedH / srcInfo->sampleInfo.y));
            }

            gcmASSERT(pAdjustRect->x <= srcMaxW);
            gcmASSERT(pAdjustRect->y <= srcMaxH);
            gcmASSERT(pAdjustRect->x <= dstMaxW);
            gcmASSERT(pAdjustRect->y <= dstMaxH);
        }
        else
        {
            /* Determine the resolve size. */
            if ((dstOrigin->x == 0) && (pAdjustRect->x >= (gctINT)dstInfo->requestW))
            {
                /* Full width resolve, a special case. */
                pAdjustRect->x = dstInfo->alignedW;
            }

            if ((dstOrigin->y == 0) &&
                (pAdjustRect->y >= (gctINT)dstInfo->requestH))
            {
                /* Full height resolve, a special case. */
                pAdjustRect->y = dstInfo->alignedH;
            }

            pAdjustRect->x = gcmMIN(srcMaxW, pAdjustRect->x);
            pAdjustRect->y = gcmMIN(srcMaxH, pAdjustRect->y);
            pAdjustRect->x = gcmMIN(dstMaxW, pAdjustRect->x);
            pAdjustRect->y = gcmMIN(dstMaxH, pAdjustRect->y);
        }

        if (dstInfo->hzNode.valid)
        {
            /* Disable any HZ attached to destination. */
            dstInfo->hzDisabled = gcvTRUE;
        }

        /*
        ** 1, gcoHARDWARE_ResolveRect can't handle multi-layer src/dst.
        ** 2, Fake format, except padding channel ones.
        ** 3, gcoHARDWARE_ResolveRect can't handle non unsigned normalized src/dst
        ** For those cases, just fall back to generic copy pixels path.
        */
        if (((srcInfo->formatInfo.layers > 1)                                            ||
             (dstInfo->formatInfo.layers > 1)                                            ||
             (srcInfo->formatInfo.fakedFormat &&
             /* Faked format, but not paddingFormat with default value padded, go SW */
              !(srcInfo->paddingFormat && !srcInfo->garbagePadded)
             )                                                                           ||
             (dstInfo->formatInfo.fakedFormat && !dstInfo->paddingFormat)                ||
             (srcInfo->formatInfo.fmtDataType != gcvFORMAT_DATATYPE_UNSIGNED_NORMALIZED) ||
             (dstInfo->formatInfo.fmtDataType != gcvFORMAT_DATATYPE_UNSIGNED_NORMALIZED)
            ) &&
            ((srcInfo->format != gcvSURF_S8) || (dstInfo->format != gcvSURF_S8))
           )
        {
            gcmONERROR(gcoSURF_CopyPixels_v2(SrcView, DstView, Args));
        }
        /* Special case a resolve from the depth buffer with tile status. */
        else if ((srcInfo->type == gcvSURF_DEPTH)
        &&  (srcInfo->tileStatusNode.pool != gcvPOOL_UNKNOWN)
        )
        {
            /* Resolve a depth buffer. */
            if (gcmIS_ERROR(gcoHARDWARE_ResolveDepth_v2(gcvNULL, SrcView, DstView, &newArgs)))
            {
                gcmONERROR(gcoSURF_CopyPixels_v2(SrcView, DstView, Args));
            }
        }
        else
        {
            /* Perform resolve. */
            if (gcmIS_ERROR(gcoHARDWARE_ResolveRect_v2(gcvNULL, SrcView, DstView, &newArgs)))
            {
                gcePATCH_ID patchID = gcvPATCH_INVALID;
                gcoHAL_GetPatchID(gcvNULL, &patchID);

                if (patchID != gcvPATCH_GFXBENCH)
                {
                    gcmONERROR(gcoSURF_CopyPixels_v2(SrcView, DstView, Args));
                }
            }
        }

        /* If dst surface was fully overwritten, reset the deferDither3D flag. */
        if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_PE_DITHER_FIX) == gcvFALSE &&
            dstOrigin->x == 0 && pAdjustRect->x >= (gctINT)dstInfo->requestW &&
            dstOrigin->y == 0 && pAdjustRect->y >= (gctINT)dstInfo->requestH)
        {
            dstInfo->deferDither3D = gcvFALSE;
        }
    }

OnError:
    /* Unlock the surfaces. */
    if (dstBase[0] != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(dstSurf, dstBase[0]));
    }

    if (srcBase[0] != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(srcSurf, srcBase[0]));
    }

    if (savedSrcFmt != gcvSURF_UNKNOWN)
    {
        srcInfo->format = savedSrcFmt;
        srcInfo->formatInfo.fmtDataType = savedSrcDataType;
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoSURF_3DBlitCopy(
    IN gceENGINE Engine,
    IN gctUINT32 SrcAddress,
    IN gctUINT32 DestAddress,
    IN gctUINT32 Bytes
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("SrcAddress=0x%x DestAddress=0x%x Bytes=0x%x",
                  SrcAddress, DestAddress, Bytes);

    gcmONERROR(gcoHARDWARE_3DBlitCopy(gcvNULL, Engine, SrcAddress, DestAddress, Bytes));

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoSURF_Blit_v2(
    IN gcsSURF_VIEW *SrcView,
    IN gcsSURF_VIEW *DstView,
    IN gcsSURF_RESOLVE_ARGS *Args
)
{
    gctUINT slice;
    gceSTATUS status = gcvSTATUS_OK;

    if (Args->version != gcvHAL_ARG_VERSION_V2)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    for (slice = 0; slice < Args->uArgs.v2.numSlices; ++slice)
    {
        if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_BLT_ENGINE))
        {
        }
    }

OnError:
    return status;
}

#endif /* gcdENABLE_3D */

#if gcdENABLE_2D
/*******************************************************************************
**
**  gcoSURF_SetClipping
**
**  Set clipping rectangle to the size of the surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_SetClipping(
    IN gcoSURF Surface
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    do
    {
        gco2D engine;
        gcsRECT rect = {0};

        gcmERR_BREAK(gcoHAL_Get2DEngine(gcvNULL, &engine));

        rect.right  = Surface->info.allocedW;
        rect.bottom = Surface->info.allocedH;
        gcmERR_BREAK(gco2D_SetClipping(engine, &rect));
    }
    while (gcvFALSE);

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_Clear2D
**
**  Clear one or more rectangular areas.
**
**  INPUT:
**
**      gcoSURF DestSurface
**          Pointer to the destination surface.
**
**      gctUINT32 RectCount
**          The number of rectangles to draw. The array of rectangles
**          pointed to by Rect parameter must have at least RectCount items.
**          Note, that for masked source blits only one destination rectangle
**          is supported.
**
**      gcsRECT_PTR DestRect
**          Pointer to a list of destination rectangles.
**
**      gctUINT32 LoColor
**          Low 32-bit clear color values.
**
**      gctUINT32 HiColor
**          high 32-bit clear color values.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_Clear2D(
    IN gcoSURF DestSurface,
    IN gctUINT32 RectCount,
    IN gcsRECT_PTR DestRect,
    IN gctUINT32 LoColor,
    IN gctUINT32 HiColor
    )
{
    gceSTATUS status;
    gctPOINTER destMemory[3] = {gcvNULL};
    gctUINT32 destAddress[3] = {0};
    gco2D engine;

    gcmHEADER_ARG("DestSurface=0x%x RectCount=%u DestRect=0x%x LoColor=%u HiColor=%u",
              DestSurface, RectCount, DestRect, LoColor, HiColor);

    do
    {
        gcsRECT dstRect = {0};

        /* Validate the object. */
        gcmBADOBJECT_BREAK(DestSurface, gcvOBJ_SURF);

        gcmERR_BREAK(gcoHAL_Get2DEngine(gcvNULL, &engine));

        /* Use surface rect if not specified. */
        if (DestRect == gcvNULL)
        {
            if (RectCount != 1)
            {
                status = gcvSTATUS_INVALID_ARGUMENT;
                break;
            }

            dstRect.right  = DestSurface->info.allocedW;
            dstRect.bottom = DestSurface->info.allocedH;
            DestRect = &dstRect;
        }

        /* Lock the destination. */
        gcmERR_BREAK(gcoSURF_Lock(
            DestSurface,
            destAddress,
            destMemory
            ));

        /* Program the destination. */
        gcmERR_BREAK(gco2D_SetTargetEx(
            engine,
            destAddress[0],
            DestSurface->info.stride,
            DestSurface->info.rotation,
            DestSurface->info.alignedW,
            DestSurface->info.alignedH
            ));

        gcmERR_BREAK(gco2D_SetTransparencyAdvanced(
            engine,
            gcv2D_OPAQUE,
            gcv2D_OPAQUE,
            gcv2D_OPAQUE
            ));

        /* Form a CLEAR command. */
        gcmERR_BREAK(gco2D_Clear(
            engine,
            RectCount,
            DestRect,
            LoColor,
            0xCC,
            0xCC,
            DestSurface->info.format
            ));
    }
    while (gcvFALSE);

    /* Unlock the destination. */
    if (destMemory[0] != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(DestSurface, destMemory[0]));
    }

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_Line
**
**  Draw one or more Bresenham lines.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**      gctUINT32 LineCount
**          The number of lines to draw. The array of line positions pointed
**          to by Position parameter must have at least LineCount items.
**
**      gcsRECT_PTR Position
**          Points to an array of positions in (x0, y0)-(x1, y1) format.
**
**      gcoBRUSH Brush
**          Brush to use for drawing.
**
**      gctUINT8 FgRop
**          Foreground ROP to use with opaque pixels.
**
**      gctUINT8 BgRop
**          Background ROP to use with transparent pixels.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_Line(
    IN gcoSURF DestSurface,
    IN gctUINT32 LineCount,
    IN gcsRECT_PTR Position,
    IN gcoBRUSH Brush,
    IN gctUINT8 FgRop,
    IN gctUINT8 BgRop
    )
{
    gceSTATUS status;
    gctPOINTER destMemory[3] = {gcvNULL};
    gctUINT32 destAddress[3] = {0};
    gco2D engine;

    gcmHEADER_ARG("DestSurface=0x%x LineCount=%u Position=0x%x Brush=0x%x FgRop=%02x "
              "BgRop=%02x",
              DestSurface, LineCount, Position, Brush, FgRop, BgRop);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(DestSurface, gcvOBJ_SURF);

    do
    {
        gcmERR_BREAK(gcoHAL_Get2DEngine(gcvNULL, &engine));

        /* Lock the destination. */
        gcmERR_BREAK(gcoSURF_Lock(
            DestSurface,
            destAddress,
            destMemory
            ));

        /* Program the destination. */
        gcmERR_BREAK(gco2D_SetTargetEx(
            engine,
            destAddress[0],
            DestSurface->info.stride,
            DestSurface->info.rotation,
            DestSurface->info.alignedW,
            DestSurface->info.alignedH
            ));

        gcmERR_BREAK(gco2D_SetTransparencyAdvanced(
            engine,
            gcv2D_OPAQUE,
            gcv2D_OPAQUE,
            gcv2D_OPAQUE
            ));

        /* Draw a LINE command. */
        gcmERR_BREAK(gco2D_Line(
            engine,
            LineCount,
            Position,
            Brush,
            FgRop,
            BgRop,
            DestSurface->info.format
            ));
    }
    while (gcvFALSE);

    /* Unlock the destination. */
    if (destMemory[0] != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(DestSurface, destMemory[0]));
    }

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_Blit
**
**  Generic rectangular blit.
**
**  INPUT:
**
**      OPTIONAL gcoSURF SrcSurface
**          Pointer to the source surface.
**
**      gcoSURF DestSurface
**          Pointer to the destination surface.
**
**      gctUINT32 RectCount
**          The number of rectangles to draw. The array of rectangles
**          pointed to by Rect parameter must have at least RectCount items.
**          Note, that for masked source blits only one destination rectangle
**          is supported.
**
**      OPTIONAL gcsRECT_PTR SrcRect
**          If RectCount is 1, SrcRect represents an absolute rectangle within
**          the source surface.
**          If RectCount is greater then 1, (right,bottom) members of SrcRect
**          are ignored and (left,top) members are used as the offset from
**          the origin of each destination rectangle in DestRect list to
**          determine the corresponding source rectangle. In this case the width
**          and the height of the source are assumed the same as of the
**          corresponding destination rectangle.
**
**      gcsRECT_PTR DestRect
**          Pointer to a list of destination rectangles.
**
**      OPTIONAL gcoBRUSH Brush
**          Brush to use for drawing.
**
**      gctUINT8 FgRop
**          Foreground ROP to use with opaque pixels.
**
**      gctUINT8 BgRop
**          Background ROP to use with transparent pixels.
**
**      OPTIONAL gceSURF_TRANSPARENCY Transparency
**          gcvSURF_OPAQUE - each pixel of the bitmap overwrites the destination.
**          gcvSURF_SOURCE_MATCH - source pixels compared against register value
**              to determine the transparency. In simple terms, the transparency
**              comes down to selecting the ROP code to use. Opaque pixels use
**              foreground ROP and transparent ones use background ROP.
**          gcvSURF_SOURCE_MASK - monochrome source mask defines transparency.
**          gcvSURF_PATTERN_MASK - pattern mask defines transparency.
**
**      OPTIONAL gctUINT32 TransparencyColor
**          This value is used in gcvSURF_SOURCE_MATCH transparency mode.
**          The value is compared against each pixel to determine transparency
**          of the pixel. If the values found equal, the pixel is transparent;
**          otherwise it is opaque.
**
**      OPTIONAL gctPOINTER Mask
**          A pointer to monochrome mask for masked source blits.
**
**      OPTIONAL gceSURF_MONOPACK MaskPack
**          Determines how many horizontal pixels are there per each 32-bit
**          chunk of monochrome mask. For example, if set to gcvSURF_PACKED8,
**          each 32-bit chunk is 8-pixel wide, which also means that it defines
**          4 vertical lines of pixel mask.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_Blit(
    IN OPTIONAL gcoSURF SrcSurface,
    IN gcoSURF DestSurface,
    IN gctUINT32 RectCount,
    IN OPTIONAL gcsRECT_PTR SrcRect,
    IN gcsRECT_PTR DestRect,
    IN OPTIONAL gcoBRUSH Brush,
    IN gctUINT8 FgRop,
    IN gctUINT8 BgRop,
    IN OPTIONAL gceSURF_TRANSPARENCY Transparency,
    IN OPTIONAL gctUINT32 TransparencyColor,
    IN OPTIONAL gctPOINTER Mask,
    IN OPTIONAL gceSURF_MONOPACK MaskPack
    )
{
    gceSTATUS status;

    /*gcoHARDWARE hardware;*/
    gco2D engine;

    gce2D_TRANSPARENCY srcTransparency;
    gce2D_TRANSPARENCY dstTransparency;
    gce2D_TRANSPARENCY patTransparency;

    gctBOOL useBrush;
    gctBOOL useSource;

    gctBOOL stretchBlt = gcvFALSE;
    gctBOOL relativeSource = gcvFALSE;

    gctPOINTER srcMemory[3]  = {gcvNULL};
    gctUINT32 srcAddress[3]  = {0};
    gctPOINTER destMemory[3] = {gcvNULL};
    gctUINT32 destAddress[3]  = {0};

    gctBOOL useSoftEngine = gcvFALSE;

    gcmHEADER_ARG("SrcSurface=0x%x DestSurface=0x%x RectCount=%u SrcRect=0x%x "
              "DestRect=0x%x Brush=0x%x FgRop=%02x BgRop=%02x Transparency=%d "
              "TransparencyColor=%08x Mask=0x%x MaskPack=%d",
              SrcSurface, DestSurface, RectCount, SrcRect, DestRect, Brush,
              FgRop, BgRop, Transparency, TransparencyColor, Mask, MaskPack);

    do
    {
        gcsRECT srcRect = {0};
        gcsRECT dstRect = {0};
        gctUINT32 destFormat, destFormatSwizzle, destIsYUV;

        /* Validate the object. */
        gcmBADOBJECT_BREAK(DestSurface, gcvOBJ_SURF);

        if (Mask != gcvNULL && gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_ANDROID_ONLY) == gcvTRUE)
        {
            gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
            return gcvSTATUS_NOT_SUPPORTED;
        }

        /* Is 2D Hardware available? */
        if (!gcoHARDWARE_Is2DAvailable(gcvNULL))
        {
            /* No, use software renderer. */
            gcmERR_BREAK(gcoHARDWARE_UseSoftware2D(gcvNULL, gcvTRUE));
            useSoftEngine = gcvTRUE;
        }

        /* Is the destination format supported? */
        if (gcmIS_ERROR(gcoHARDWARE_TranslateDestinationFormat(
                gcvNULL, DestSurface->info.format, gcvTRUE,
                &destFormat, &destFormatSwizzle, &destIsYUV)))
        {
            /* No, use software renderer. */
            gcmERR_BREAK(gcoHARDWARE_UseSoftware2D(gcvNULL, gcvTRUE));
            useSoftEngine = gcvTRUE;
        }

        /* Translate the specified transparency mode. */
        gcmERR_BREAK(gcoHARDWARE_TranslateSurfTransparency(
            Transparency,
            &srcTransparency,
            &dstTransparency,
            &patTransparency
            ));

        /* Determine the resource usage. */
        gcoHARDWARE_Get2DResourceUsage(
            FgRop, BgRop,
            srcTransparency,
            &useSource, &useBrush, gcvNULL
            );

        /* Use surface rect if not specified. */
        if (DestRect == gcvNULL)
        {
            if (RectCount != 1)
            {
                status = gcvSTATUS_INVALID_ARGUMENT;
                break;
            }
            dstRect.right  = DestSurface->info.allocedW;
            dstRect.bottom = DestSurface->info.allocedH;
            DestRect = &dstRect;
        }

        /* Get 2D engine. */
        gcmERR_BREAK(gcoHAL_Get2DEngine(gcvNULL, &engine));

        /* Setup the brush if needed. */
        if (useBrush)
        {
            /* Flush the brush. */
            gcmERR_BREAK(gco2D_FlushBrush(
                engine,
                Brush,
                DestSurface->info.format
                ));
        }

        /* Setup the source if needed. */
        if (useSource)
        {
            /* Validate the object. */
            gcmBADOBJECT_BREAK(SrcSurface, gcvOBJ_SURF);

            /* Use surface rect if not specified. */
            if (SrcRect == gcvNULL)
            {
                srcRect.right = SrcSurface->info.allocedW;
                srcRect.bottom = SrcSurface->info.allocedH;
                SrcRect = &srcRect;
            }

            /* Lock the source. */
            gcmERR_BREAK(gcoSURF_Lock(
                SrcSurface,
                srcAddress,
                srcMemory
                ));

            /* Determine the relative flag. */
            relativeSource = (RectCount > 1) ? gcvTRUE : gcvFALSE;

            /* Program the source. */
            if (Mask == gcvNULL)
            {
                gctBOOL equal;

                /* Check whether this should be a stretch/shrink blit. */
                if ( (gcsRECT_IsOfEqualSize(SrcRect, DestRect, &equal) ==
                          gcvSTATUS_OK) &&
                     !equal )
                {
                    /* Calculate the stretch factors. */
                    gcmERR_BREAK(gco2D_SetStretchRectFactors(
                        engine,
                        SrcRect, DestRect
                        ));

                    /* Mark as stretch blit. */
                    stretchBlt = gcvTRUE;
                }

                gcmERR_BREAK(gco2D_SetColorSourceEx(
                    engine,
                    useSoftEngine ?
                        (gctUINT32)(gctUINTPTR_T)SrcSurface->info.node.logical
                        : srcAddress[0],
                    SrcSurface->info.stride,
                    SrcSurface->info.format,
                    SrcSurface->info.rotation,
                    SrcSurface->info.alignedW,
                    SrcSurface->info.alignedH,
                    relativeSource,
                    Transparency,
                    TransparencyColor
                    ));

                gcmERR_BREAK(gco2D_SetSource(
                    engine,
                    SrcRect
                    ));
            }
        }

        /* Lock the destination. */
        gcmERR_BREAK(gcoSURF_Lock(
            DestSurface,
            destAddress,
            destMemory
            ));

        gcmERR_BREAK(gco2D_SetTargetEx(
            engine,
            useSoftEngine ?
                (gctUINT32)(gctUINTPTR_T)DestSurface->info.node.logical
                : destAddress[0],
            DestSurface->info.stride,
            DestSurface->info.rotation,
            DestSurface->info.alignedW,
            DestSurface->info.alignedH
            ));

        /* Masked sources need to be handled differently. */
        if (useSource && (Mask != gcvNULL))
        {
            gctUINT32 streamPackHeightMask;
            gcsSURF_FORMAT_INFO_PTR srcFormat[2];
            gctUINT32 srcAlignedLeft, srcAlignedTop;
            gctINT32 tileWidth, tileHeight;
            gctUINT32 tileHeightMask;
            gctUINT32 maxHeight;
            gctUINT32 srcBaseAddress;
            gcsRECT srcSubRect;
            gcsRECT destSubRect;
            gcsRECT maskRect;
            gcsPOINT maskSize;
            gctUINT32 lines2render;
            gctUINT32 streamWidth;
            gceSURF_MONOPACK streamPack;

            /* Compute the destination size. */
            gctUINT32 destWidth  = DestRect->right  - DestRect->left;
            gctUINT32 destHeight = DestRect->bottom - DestRect->top;

            /* Query tile size. */
            gcmASSERT(SrcSurface->info.type == gcvSURF_BITMAP);
            gcoHARDWARE_QueryTileSize(
                &tileWidth, &tileHeight,
                gcvNULL, gcvNULL,
                gcvNULL
                );

            tileHeightMask = tileHeight - 1;

            /* Determine left source coordinate. */
            srcSubRect.left = SrcRect->left & 7;

            /* Assume 8-pixel packed stream. */
            streamWidth = gcmALIGN(srcSubRect.left + destWidth, 8);

            /* Do we fit? */
            if (streamWidth == 8)
            {
                streamPack = gcvSURF_PACKED8;
                streamPackHeightMask = ~3U;
            }

            /* Nope, don't fit. */
            else
            {
                /* Assume 16-pixel packed stream. */
                streamWidth = gcmALIGN(srcSubRect.left + destWidth, 16);

                /* Do we fit now? */
                if (streamWidth == 16)
                {
                    streamPack = gcvSURF_PACKED16;
                    streamPackHeightMask = ~1U;
                }

                /* Still don't. */
                else
                {
                    /* Assume unpacked stream. */
                    streamWidth = gcmALIGN(srcSubRect.left + destWidth, 32);
                    streamPack = gcvSURF_UNPACKED;
                    streamPackHeightMask = ~0U;
                }
            }

            /* Determine the maximum stream height. */
            maxHeight  = gcoMATH_DivideUInt(gco2D_GetMaximumDataCount() << 5,
                                            streamWidth);
            maxHeight &= streamPackHeightMask;

            /* Determine the sub source rectangle. */
            srcSubRect.top    = SrcRect->top & tileHeightMask;
            srcSubRect.right  = srcSubRect.left + destWidth;
            srcSubRect.bottom = srcSubRect.top;

            /* Init destination subrectangle. */
            destSubRect.left   = DestRect->left;
            destSubRect.top    = DestRect->top;
            destSubRect.right  = DestRect->right;
            destSubRect.bottom = destSubRect.top;

            /* Determine the number of lines to render. */
            lines2render = srcSubRect.top + destHeight;

            /* Determine the aligned source coordinates. */
            srcAlignedLeft = SrcRect->left - srcSubRect.left;
            srcAlignedTop  = SrcRect->top  - srcSubRect.top;
            gcmASSERT((srcAlignedLeft % tileWidth) == 0);

            /* Get format characteristics. */
            gcmERR_BREAK(gcoSURF_QueryFormat(SrcSurface->info.format, srcFormat));

            /* Determine the initial source address. */
            srcBaseAddress
                = (useSoftEngine ?
                        (gctUINT32)(gctUINTPTR_T)SrcSurface->info.node.logical
                        : srcAddress[0])
                +   srcAlignedTop  * SrcSurface->info.stride
                + ((srcAlignedLeft * srcFormat[0]->bitsPerPixel) >> 3);

            /* Set initial mask coordinates. */
            maskRect.left   = srcAlignedLeft;
            maskRect.top    = srcAlignedTop;
            maskRect.right  = maskRect.left + streamWidth;
            maskRect.bottom = maskRect.top;

            /* Set mask size. */
            maskSize.x = SrcSurface->info.allocedW;
            maskSize.y = SrcSurface->info.allocedH;

            do
            {
                /* Determine the area to render in this pass. */
                srcSubRect.top = srcSubRect.bottom & tileHeightMask;
                srcSubRect.bottom = srcSubRect.top + lines2render;
                if (srcSubRect.bottom > (gctINT32) maxHeight)
                    srcSubRect.bottom = maxHeight & ~tileHeightMask;

                destSubRect.top = destSubRect.bottom;
                destSubRect.bottom
                    = destSubRect.top
                    + (srcSubRect.bottom - srcSubRect.top);

                maskRect.top = maskRect.bottom;
                maskRect.bottom = maskRect.top + srcSubRect.bottom;

                /* Set source rectangle size. */
                gcmERR_BREAK(gco2D_SetSource(
                    engine,
                    &srcSubRect
                    ));

                /* Configure masked source. */
                gcmERR_BREAK(gco2D_SetMaskedSource(
                    engine,
                    srcBaseAddress,
                    SrcSurface->info.stride,
                    SrcSurface->info.format,
                    relativeSource,
                    streamPack
                    ));

                /* Do the blit. */
                gcmERR_BREAK(gco2D_MonoBlit(
                    engine,
                    Mask,
                    &maskSize,
                    &maskRect,
                    MaskPack,
                    streamPack,
                    &destSubRect,
                    FgRop,
                    BgRop,
                    DestSurface->info.format
                    ));

                /* Update the source address. */
                srcBaseAddress += srcSubRect.bottom * SrcSurface->info.stride;

                /* Update the line counter. */
                lines2render -= srcSubRect.bottom;
            }
            while (lines2render);
        }
        else if (stretchBlt)
        {
            gcmERR_BREAK(gco2D_StretchBlit(
                engine,
                RectCount,
                DestRect,
                FgRop,
                BgRop,
                DestSurface->info.format
                ));
        }
        else
        {
            gcmERR_BREAK(gco2D_Blit(
                engine,
                RectCount,
                DestRect,
                FgRop,
                BgRop,
                DestSurface->info.format
                ));
        }
    }
    while (gcvFALSE);

    /* Unlock the source. */
    if (srcMemory[0] != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(SrcSurface, srcMemory[0]));
    }

    /* Unlock the destination. */
    if (destMemory[0] != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(DestSurface, destMemory[0]));
    }

    /*gcmGETHARDWARE(hardware);*/

    if (useSoftEngine)
    {
        /* Disable software renderer. */
        gcmVERIFY_OK(gcoHARDWARE_UseSoftware2D(gcvNULL, gcvFALSE));
    }

/*OnError:*/
    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_MonoBlit
**
**  Monochrome blit.
**
**  INPUT:
**
**      gcoSURF DestSurface
**          Pointer to the destination surface.
**
**      gctPOINTER Source
**          A pointer to the monochrome bitmap.
**
**      gceSURF_MONOPACK SourcePack
**          Determines how many horizontal pixels are there per each 32-bit
**          chunk of monochrome bitmap. For example, if set to gcvSURF_PACKED8,
**          each 32-bit chunk is 8-pixel wide, which also means that it defines
**          4 vertical lines of pixels.
**
**      gcsPOINT_PTR SourceSize
**          Size of the source monochrome bitmap in pixels.
**
**      gcsPOINT_PTR SourceOrigin
**          Top left coordinate of the source within the bitmap.
**
**      gcsRECT_PTR DestRect
**          Pointer to a list of destination rectangles.
**
**      OPTIONAL gcoBRUSH Brush
**          Brush to use for drawing.
**
**      gctUINT8 FgRop
**          Foreground ROP to use with opaque pixels.
**
**      gctUINT8 BgRop
**          Background ROP to use with transparent pixels.
**
**      gctBOOL ColorConvert
**          The values of FgColor and BgColor parameters are stored directly in
**          internal color registers and are used either directly as the source
**          color or converted to the format of destination before actually
**          used. The later happens if ColorConvert is not zero.
**
**      gctUINT8 MonoTransparency
**          This value is used in gcvSURF_SOURCE_MATCH transparency mode.
**          The value can be either 0 or 1 and is compared against each mono
**          pixel to determine transparency of the pixel. If the values found
**          equal, the pixel is transparent; otherwise it is opaque.
**
**      gceSURF_TRANSPARENCY Transparency
**          gcvSURF_OPAQUE - each pixel of the bitmap overwrites the destination.
**          gcvSURF_SOURCE_MATCH - source pixels compared against register value
**              to determine the transparency. In simple terms, the transparency
**              comes down to selecting the ROP code to use. Opaque pixels use
**              foreground ROP and transparent ones use background ROP.
**          gcvSURF_SOURCE_MASK - monochrome source mask defines transparency.
**          gcvSURF_PATTERN_MASK - pattern mask defines transparency.
**
**      gctUINT32 FgColor
**          The values are used to represent foreground color
**          of the source. If the values are in destination format, set
**          ColorConvert to 0. Otherwise, provide the values in ARGB8 format
**          and set ColorConvert to 1 to instruct the hardware to convert the
**          values to the destination format before they are actually used.
**
**      gctUINT32 BgColor
**          The values are used to represent background color
**          of the source. If the values are in destination format, set
**          ColorConvert to 0. Otherwise, provide the values in ARGB8 format
**          and set ColorConvert to 1 to instruct the hardware to convert the
**          values to the destination format before they are actually used.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_MonoBlit(
    IN gcoSURF DestSurface,
    IN gctPOINTER Source,
    IN gceSURF_MONOPACK SourcePack,
    IN gcsPOINT_PTR SourceSize,
    IN gcsPOINT_PTR SourceOrigin,
    IN gcsRECT_PTR DestRect,
    IN OPTIONAL gcoBRUSH Brush,
    IN gctUINT8 FgRop,
    IN gctUINT8 BgRop,
    IN gctBOOL ColorConvert,
    IN gctUINT8 MonoTransparency,
    IN gceSURF_TRANSPARENCY Transparency,
    IN gctUINT32 FgColor,
    IN gctUINT32 BgColor
    )
{
    gceSTATUS status;

    gco2D engine;

    gce2D_TRANSPARENCY srcTransparency;
    gce2D_TRANSPARENCY dstTransparency;
    gce2D_TRANSPARENCY patTransparency;

    gctBOOL useBrush;
    gctBOOL useSource;

    gctUINT32 destWidth;
    gctUINT32 destHeight;

    gctUINT32 maxHeight;
    gctUINT32 streamPackHeightMask;
    gcsPOINT sourceOrigin;
    gcsRECT srcSubRect;
    gcsRECT destSubRect;
    gcsRECT streamRect;
    gctUINT32 lines2render;
    gctUINT32 streamWidth;
    gceSURF_MONOPACK streamPack;

    gctPOINTER destMemory[3] = {gcvNULL};
    gctUINT32 destAddress[3] = {0};

    gctBOOL useSotfEngine = gcvFALSE;

    gcmHEADER_ARG("DestSurface=0x%x Source=0x%x SourceSize=0x%x SourceOrigin=0x%x "
              "DestRect=0x%x Brush=0x%x FgRop=%02x BgRop=%02x ColorConvert=%d "
              "MonoTransparency=%u Transparency=%d FgColor=%08x BgColor=%08x",
              DestSurface, Source, SourceSize, SourceOrigin, DestRect, Brush,
              FgRop, BgRop, ColorConvert, MonoTransparency, Transparency,
              FgColor, BgColor);

    do
    {
        gcsRECT dstRect = {0};
        gctUINT32 destFormat, destFormatSwizzle, destIsYUV;

        /* Validate the object. */
        gcmBADOBJECT_BREAK(DestSurface, gcvOBJ_SURF);

        gcmERR_BREAK(gcoHAL_Get2DEngine(gcvNULL, &engine));

        /* Is the destination format supported? */
        if (gcmIS_ERROR(gcoHARDWARE_TranslateDestinationFormat(
                gcvNULL, DestSurface->info.format, gcvTRUE,
                &destFormat, &destFormatSwizzle, &destIsYUV)))
        {
            /* No, use software renderer. */
            gcmERR_BREAK(gcoHARDWARE_UseSoftware2D(gcvNULL, gcvTRUE));
            useSotfEngine = gcvTRUE;
        }

        /* Translate the specified transparency mode. */
        gcmERR_BREAK(gcoHARDWARE_TranslateSurfTransparency(
            Transparency,
            &srcTransparency,
            &dstTransparency,
            &patTransparency
            ));

        /* Determine the resource usage. */
        gcoHARDWARE_Get2DResourceUsage(
            FgRop, BgRop,
            srcTransparency,
            &useSource, &useBrush, gcvNULL
            );

        /* Source must be used. */
        if (!useSource)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        /* Use surface rect if not specified. */
        if (DestRect == gcvNULL)
        {
            dstRect.right  = DestSurface->info.allocedW;
            dstRect.bottom = DestSurface->info.allocedH;
            DestRect = &dstRect;
        }

        /* Default to 0 origin. */
        if (SourceOrigin == gcvNULL)
        {
            SourceOrigin = &sourceOrigin;
            SourceOrigin->x = 0;
            SourceOrigin->y = 0;
        }

        /* Lock the destination. */
        gcmERR_BREAK(gcoSURF_Lock(
            DestSurface,
            destAddress,
            destMemory
            ));

        gcmERR_BREAK(gco2D_SetTargetEx(
            engine,
            useSotfEngine ?
                (gctUINT32)(gctUINTPTR_T)DestSurface->info.node.logical
                : destAddress[0],
            DestSurface->info.stride,
            DestSurface->info.rotation,
            DestSurface->info.alignedW,
            DestSurface->info.alignedH
            ));

        /* Setup the brush if needed. */
        if (useBrush)
        {
            /* Flush the brush. */
            gcmERR_BREAK(gco2D_FlushBrush(
                engine,
                Brush,
                DestSurface->info.format
                ));
        }

        /* Compute the destination size. */
        destWidth  = DestRect->right  - DestRect->left;
        destHeight = DestRect->bottom - DestRect->top;

        /* Determine the number of lines to render. */
        lines2render = destHeight;

        /* Determine left source coordinate. */
        srcSubRect.left = SourceOrigin->x & 7;

        /* Assume 8-pixel packed stream. */
        streamWidth = gcmALIGN(srcSubRect.left + destWidth, 8);

        /* Do we fit? */
        if (streamWidth == 8)
        {
            streamPack = gcvSURF_PACKED8;
            streamPackHeightMask = ~3U;
        }

        /* Nope, don't fit. */
        else
        {
            /* Assume 16-pixel packed stream. */
            streamWidth = gcmALIGN(srcSubRect.left + destWidth, 16);

            /* Do we fit now? */
            if (streamWidth == 16)
            {
                streamPack = gcvSURF_PACKED16;
                streamPackHeightMask = ~1U;
            }

            /* Still don't. */
            else
            {
                /* Assume unpacked stream. */
                streamWidth = gcmALIGN(srcSubRect.left + destWidth, 32);
                streamPack = gcvSURF_UNPACKED;
                streamPackHeightMask = ~0U;
            }
        }

        /* Set the rectangle value. */
        srcSubRect.top = srcSubRect.right = srcSubRect.bottom = 0;

        /* Set source rectangle size. */
        gcmERR_BREAK(gco2D_SetSource(
            engine,
            &srcSubRect
            ));

        /* Program the source. */
        gcmERR_BREAK(gco2D_SetMonochromeSource(
            engine,
            ColorConvert,
            MonoTransparency,
            streamPack,
            gcvFALSE,
            Transparency,
            FgColor,
            BgColor
            ));

        /* Determine the maxumum stream height. */
        maxHeight  = gcoMATH_DivideUInt(gco2D_GetMaximumDataCount() << 5,
                                        streamWidth);
        maxHeight &= streamPackHeightMask;

        /* Set the stream rectangle. */
        streamRect.left   = SourceOrigin->x - srcSubRect.left;
        streamRect.top    = SourceOrigin->y;
        streamRect.right  = streamRect.left + streamWidth;
        streamRect.bottom = streamRect.top;

        /* Init destination subrectangle. */
        destSubRect.left   = DestRect->left;
        destSubRect.top    = DestRect->top;
        destSubRect.right  = DestRect->right;
        destSubRect.bottom = destSubRect.top;

        do
        {
            /* Determine the area to render in this pass. */
            gctUINT32 currLines = (lines2render > maxHeight)
                ? maxHeight
                : lines2render;

            streamRect.top     = streamRect.bottom;
            streamRect.bottom += currLines;

            destSubRect.top     = destSubRect.bottom;
            destSubRect.bottom += currLines;

            /* Do the blit. */
            gcmERR_BREAK(gco2D_MonoBlit(
                engine,
                Source,
                SourceSize,
                &streamRect,
                SourcePack,
                streamPack,
                &destSubRect,
                FgRop, BgRop,
                DestSurface->info.format
                ));

            /* Update the line counter. */
            lines2render -= currLines;
        }
        while (lines2render);
    }
    while (gcvFALSE);

    /* Unlock the destination. */
    if (destMemory[0] != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(DestSurface, destMemory[0]));
    }

    if (useSotfEngine)
    {
        /* Disable software renderer. */
        gcmVERIFY_OK(gcoHARDWARE_UseSoftware2D(gcvNULL, gcvFALSE));
    }

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_FilterBlit
**
**  Filter blit.
**
**  INPUT:
**
**      gcoSURF SrcSurface
**          Pointer to the source surface.
**
**      gcoSURF DestSurface
**          Pointer to the destination surface.
**
**      gcsRECT_PTR SrcRect
**          Coordinates of the entire source image.
**
**      gcsRECT_PTR DestRect
**          Coordinates of the entire destination image.
**
**      gcsRECT_PTR DestSubRect
**          Coordinates of a sub area within the destination to render.
**          If DestSubRect is gcvNULL, the complete image will be rendered
**          using coordinates set by DestRect.
**          If DestSubRect is not gcvNULL and DestSubRect and DestRect are
**          no equal, DestSubRect is assumed to be within DestRect and
**          will be used to render the sub area only.
**
**  OUTPUT:
**
**      Nothing.
*/

gceSTATUS
gcoSURF_FilterBlit(
    IN gcoSURF SrcSurface,
    IN gcoSURF DestSurface,
    IN gcsRECT_PTR SrcRect,
    IN gcsRECT_PTR DestRect,
    IN gcsRECT_PTR DestSubRect
    )
{
    gceSTATUS status;
    gctBOOL ditherBy3D = gcvFALSE;
    gctBOOL ditherNotSupported = gcvFALSE;
    gctBOOL rotateWK =  gcvFALSE;
    gctBOOL enable2DDither =  gcvFALSE;

    gctPOINTER srcMemory[3] = {gcvNULL, };
    gctUINT32 srcAddress[3] = {0, };
    gctPOINTER destMemory[3] = {gcvNULL, };
    gctUINT32 destAddress[3] = {0, };
    gctUINT32 tempAddress;

    gcsSURF_INFO_PTR tempSurf = gcvNULL;
    gcsSURF_INFO_PTR temp2Surf = gcvNULL;

    gco2D engine = gcvNULL;

    gceSURF_ROTATION srcRotBackup = (gceSURF_ROTATION)-1, dstRotBackup = (gceSURF_ROTATION)-1;

    gcmHEADER_ARG("SrcSurface=0x%x DestSurface=0x%x SrcRect=0x%x DestRect=0x%x "
              "DestSubRect=0x%x",
              SrcSurface, DestSurface, SrcRect, DestRect, DestSubRect);

    if (SrcSurface == gcvNULL || DestSurface == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    do
    {
        gcsRECT srcRect = {0};
        gcsRECT dstRect = {0};
        gcsRECT destSubRect;
        gcsSURF_FORMAT_INFO_PTR srcFormat[2];
        gcsSURF_FORMAT_INFO_PTR destFormat[2];

        /* Verify the surfaces. */
        gcmBADOBJECT_BREAK(SrcSurface, gcvOBJ_SURF);
        gcmBADOBJECT_BREAK(DestSurface, gcvOBJ_SURF);

        gcmERR_BREAK(gcoHAL_Get2DEngine(gcvNULL, &engine));

        /* Use surface rect if not specified. */
        if (SrcRect == gcvNULL)
        {
            srcRect.right  = SrcSurface->info.allocedW;
            srcRect.bottom = SrcSurface->info.allocedH;
            SrcRect = &srcRect;
        }

        /* Use surface rect if not specified. */
        if (DestRect == gcvNULL)
        {
            dstRect.right  = DestSurface->info.allocedW;
            dstRect.bottom = DestSurface->info.allocedH;
            DestRect = &dstRect;
        }

        /* Make sure the destination sub rectangle is set. */
        if (DestSubRect == gcvNULL)
        {
            destSubRect.left   = 0;
            destSubRect.top    = 0;
            destSubRect.right  = DestRect->right  - DestRect->left;
            destSubRect.bottom = DestRect->bottom - DestRect->top;

            DestSubRect = &destSubRect;
        }

        gcmERR_BREAK(gcoSURF_QueryFormat(SrcSurface->info.format, srcFormat));
        gcmERR_BREAK(gcoSURF_QueryFormat(DestSurface->info.format, destFormat));

        if ((SrcSurface->info.dither2D || DestSurface->info.dither2D)
            && ((srcFormat[0]->bitsPerPixel > destFormat[0]->bitsPerPixel)
            || (srcFormat[0]->fmtClass == gcvFORMAT_CLASS_YUV)))
        {
            if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_DITHER))
            {
                gcmERR_BREAK(gco2D_EnableDither(
                    engine,
                    gcvTRUE));

                enable2DDither = gcvTRUE;
            }
            else if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_PIPE_3D))
            {
                ditherBy3D = gcvTRUE;
            }
            else
            {
                /* Hardware does not support dither. */
                ditherNotSupported = gcvTRUE;
            }
        }

        if ((SrcSurface->info.rotation != gcvSURF_0_DEGREE || DestSurface->info.rotation != gcvSURF_0_DEGREE)
            && !gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_FILTERBLIT_FULLROTATION))
        {
            rotateWK = gcvTRUE;
        }
        else if (ditherBy3D && (((DestSubRect->right - DestSubRect->left) & 15)
            || ((DestSubRect->bottom - DestSubRect->top) & 3)))
        {
            rotateWK = gcvTRUE;
        }

        /* Lock the destination. */
        gcmERR_BREAK(gcoSURF_Lock(
            DestSurface,
            destAddress,
            destMemory
            ));

        /* Lock the source. */
        gcmERR_BREAK(gcoSURF_Lock(
            SrcSurface,
            srcAddress,
            srcMemory
            ));

        if (ditherBy3D || rotateWK)
        {
            gcsRECT srcRotRect, dstRotRect, tempRect;

            srcRotBackup = SrcSurface->info.rotation;
            dstRotBackup = DestSurface->info.rotation;

            srcRotRect = *SrcRect;
            dstRotRect = *DestRect;
            destSubRect = *DestSubRect;

            if (rotateWK)
            {
                if (SrcSurface->info.rotation != gcvSURF_0_DEGREE)
                {
                    SrcSurface->info.rotation = gcvSURF_0_DEGREE;
                    gcmERR_BREAK(gcsRECT_RelativeRotation(srcRotBackup, &DestSurface->info.rotation));

                    gcmERR_BREAK(gcsRECT_Rotate(
                        &srcRotRect,
                        srcRotBackup,
                        gcvSURF_0_DEGREE,
                        SrcSurface->info.alignedW,
                        SrcSurface->info.alignedH));

                    destSubRect.left   += dstRotRect.left;
                    destSubRect.right  += dstRotRect.left;
                    destSubRect.top    += dstRotRect.top;
                    destSubRect.bottom += dstRotRect.top;

                    gcmERR_BREAK(gcsRECT_Rotate(
                        &destSubRect,
                        dstRotBackup,
                        DestSurface->info.rotation,
                        DestSurface->info.alignedW,
                        DestSurface->info.alignedH));

                    gcmERR_BREAK(gcsRECT_Rotate(
                        &dstRotRect,
                        dstRotBackup,
                        DestSurface->info.rotation,
                        DestSurface->info.alignedW,
                        DestSurface->info.alignedH));

                    destSubRect.left   -= dstRotRect.left;
                    destSubRect.right  -= dstRotRect.left;
                    destSubRect.top    -= dstRotRect.top;
                    destSubRect.bottom -= dstRotRect.top;
                }

                tempRect.left   = 0;
                tempRect.top    = 0;
                tempRect.right  = dstRotRect.right  - dstRotRect.left;
                tempRect.bottom = dstRotRect.bottom - dstRotRect.top;

                gcmERR_BREAK(gcoHARDWARE_Get2DTempSurface(
                    gcvNULL,
                    tempRect.right,
                    tempRect.bottom,
                    ditherBy3D ? gcvSURF_A8R8G8B8 : DestSurface->info.format,
                    DestSurface->info.hints,
                    &tempSurf));

                tempSurf->rotation = gcvSURF_0_DEGREE;
            }
#if gcdENABLE_3D
            else
            {
                /* Only dither. */
                gctBOOL swap = (DestSurface->info.rotation == gcvSURF_90_DEGREE)
                    || (DestSurface->info.rotation == gcvSURF_270_DEGREE);

                tempRect.left   = 0;
                tempRect.top    = 0;
                tempRect.right  = dstRotRect.right  - dstRotRect.left;
                tempRect.bottom = dstRotRect.bottom - dstRotRect.top;

                gcmERR_BREAK(gcoHARDWARE_Get2DTempSurface(
                    gcvNULL,
                    swap ? tempRect.bottom : tempRect.right,
                    swap ? tempRect.right : tempRect.bottom,
                    gcvSURF_A8R8G8B8,
                    DestSurface->info.hints,
                    &tempSurf
                    ));

                tempSurf->rotation = DestSurface->info.rotation;
            }
#endif /* gcdENABLE_3D */

            gcmGETHARDWAREADDRESS(tempSurf->node, tempAddress);

            gcmERR_BREAK(gco2D_FilterBlitEx(
                engine,
                srcAddress[0],
                SrcSurface->info.stride,
                srcAddress[1],
                SrcSurface->info.uStride,
                srcAddress[2],
                SrcSurface->info.vStride,
                SrcSurface->info.format,
                SrcSurface->info.rotation,
                SrcSurface->info.alignedW,
                SrcSurface->info.alignedH,
                &srcRotRect,
                tempAddress,
                tempSurf->stride,
                tempSurf->format,
                tempSurf->rotation,
                tempSurf->alignedW,
                tempSurf->alignedH,
                &tempRect,
                &destSubRect
                ));

            tempRect = destSubRect;
#if gcdENABLE_3D
            if (ditherBy3D)
            {
                gcsSURF_INFO * DitherDest;
                gctBOOL savedDeferDither3D;
                struct _gcoSURF tmpSurface, dstSurface;
                gcsSURF_VIEW tmpView = {gcvNULL, 0, 1};
                gcsSURF_VIEW dstView = {gcvNULL, 0, 1};
                gcsSURF_RESOLVE_ARGS rlvArgs = {0};

                rlvArgs.version = gcvHAL_ARG_VERSION_V2;
                rlvArgs.uArgs.v2.numSlices = 1;
                if (rotateWK)
                {
                    rlvArgs.uArgs.v2.srcOrigin.x = tempRect.left;
                    rlvArgs.uArgs.v2.srcOrigin.y = tempRect.top;

                    rlvArgs.uArgs.v2.rectSize.x = tempRect.right - tempRect.left;
                    rlvArgs.uArgs.v2.rectSize.y = tempRect.bottom - tempRect.top;

                    rlvArgs.uArgs.v2.dstOrigin.x = 0;
                    rlvArgs.uArgs.v2.dstOrigin.y = 0;

                    tempRect.left = 0;
                    tempRect.top = 0;
                    tempRect.right = rlvArgs.uArgs.v2.rectSize.x;
                    tempRect.bottom = rlvArgs.uArgs.v2.rectSize.y;

                    gcmERR_BREAK(gcoHARDWARE_Get2DTempSurface(
                        gcvNULL,
                        tempRect.right,
                        tempRect.bottom,
                        DestSurface->info.format,
                        DestSurface->info.hints,
                        &temp2Surf
                        ));

                    temp2Surf->rotation = gcvSURF_0_DEGREE;

                    DitherDest = temp2Surf;
                }
                else
                {
                    destSubRect.left   += dstRotRect.left;
                    destSubRect.right  += dstRotRect.left;
                    destSubRect.top    += dstRotRect.top;
                    destSubRect.bottom += dstRotRect.top;

                    if (DestSurface->info.rotation != gcvSURF_0_DEGREE)
                    {
                        gcmERR_BREAK(gcsRECT_Rotate(
                            &destSubRect,
                            DestSurface->info.rotation,
                            gcvSURF_0_DEGREE,
                            DestSurface->info.alignedW,
                            DestSurface->info.alignedH));

                        gcmERR_BREAK(gcsRECT_Rotate(
                            &tempRect,
                            DestSurface->info.rotation,
                            gcvSURF_0_DEGREE,
                            tempSurf->alignedW,
                            tempSurf->alignedH));

                        DestSurface->info.rotation = gcvSURF_0_DEGREE;
                        tempSurf->rotation = gcvSURF_0_DEGREE;
                    }

                    rlvArgs.uArgs.v2.srcOrigin.x = tempRect.left;
                    rlvArgs.uArgs.v2.srcOrigin.y = tempRect.top;

                    rlvArgs.uArgs.v2.dstOrigin.x = destSubRect.left;
                    rlvArgs.uArgs.v2.dstOrigin.y = destSubRect.top;

                    rlvArgs.uArgs.v2.rectSize.x = destSubRect.right - destSubRect.left;
                    rlvArgs.uArgs.v2.rectSize.y = destSubRect.bottom - destSubRect.top;

                    DitherDest = &DestSurface->info;
                }

                gcmERR_BREAK(gcoHARDWARE_FlushPipe(gcvNULL, gcvNULL));

                /* Mark resolve to enable dither */
                savedDeferDither3D = tempSurf->deferDither3D;
                tempSurf->deferDither3D = gcvTRUE;

                rlvArgs.uArgs.v2.rectSize.x = gcmALIGN(rlvArgs.uArgs.v2.rectSize.x, 16);
                rlvArgs.uArgs.v2.rectSize.y = gcmALIGN(rlvArgs.uArgs.v2.rectSize.y, 4);

                gcoOS_ZeroMemory(&tmpSurface, gcmSIZEOF(tmpSurface));
                gcoOS_ZeroMemory(&dstSurface, gcmSIZEOF(dstSurface));
                gcoOS_MemCopy(&tmpSurface.info, tempSurf,   gcmSIZEOF(struct _gcsSURF_INFO));
                gcoOS_MemCopy(&dstSurface.info, DitherDest, gcmSIZEOF(struct _gcsSURF_INFO));
                tmpView.surf = &tmpSurface;
                dstView.surf = &dstSurface;

                gcmERR_BREAK(gcoHARDWARE_ResolveRect_v2(gcvNULL, &tmpView, &dstView, &rlvArgs));

                gcmERR_BREAK(gcoHARDWARE_FlushPipe(gcvNULL, gcvNULL));

                tempSurf->deferDither3D = savedDeferDither3D;
            }
#endif /* gcdENABLE_3D */
            if (rotateWK)
            {
                /* bitblit rotate. */
                gcsSURF_INFO * srcSurf = ditherBy3D ? temp2Surf : tempSurf;
                gceSURF_ROTATION tSrcRot = (gceSURF_ROTATION)-1, tDestRot = (gceSURF_ROTATION)-1;
                gctBOOL bMirror = gcvFALSE;

                destSubRect.left   += dstRotRect.left;
                destSubRect.right  += dstRotRect.left;
                destSubRect.top    += dstRotRect.top;
                destSubRect.bottom += dstRotRect.top;

                if (!gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_BITBLIT_FULLROTATION))
                {
                    tDestRot = DestSurface->info.rotation;

                    gcmERR_BREAK(gcsRECT_RelativeRotation(srcSurf->rotation, &tDestRot));
                    switch (tDestRot)
                    {
                    case gcvSURF_0_DEGREE:
                        tSrcRot = tDestRot = gcvSURF_0_DEGREE;
                        break;

                    case gcvSURF_90_DEGREE:
                        tSrcRot = gcvSURF_0_DEGREE;
                        tDestRot = gcvSURF_90_DEGREE;
                        break;

                    case gcvSURF_180_DEGREE:
                        tSrcRot = tDestRot = gcvSURF_0_DEGREE;
                        bMirror = gcvTRUE;
                        break;

                    case gcvSURF_270_DEGREE:
                        tSrcRot = gcvSURF_90_DEGREE;
                        tDestRot = gcvSURF_0_DEGREE;
                        break;

                    default:
                        status = gcvSTATUS_NOT_SUPPORTED;
                        break;
                    }

                    gcmERR_BREAK(status);

                    gcmERR_BREAK(gcsRECT_Rotate(
                        &tempRect,
                        srcSurf->rotation,
                        tSrcRot,
                        srcSurf->alignedW,
                        srcSurf->alignedH));

                    gcmERR_BREAK(gcsRECT_Rotate(
                        &destSubRect,
                        DestSurface->info.rotation,
                        tDestRot,
                        DestSurface->info.alignedW,
                        DestSurface->info.alignedH));

                    srcSurf->rotation = tSrcRot;
                    DestSurface->info.rotation = tDestRot;

                    if (bMirror)
                    {
                        gcmERR_BREAK(gco2D_SetBitBlitMirror(
                            engine,
                            gcvTRUE,
                            gcvTRUE));
                    }
                }

                gcmERR_BREAK(gco2D_SetClipping(
                    engine,
                    &destSubRect));

                gcmERR_BREAK(gco2D_SetColorSourceEx(
                    engine,
                    srcAddress[0],
                    srcSurf->stride,
                    srcSurf->format,
                    srcSurf->rotation,
                    srcSurf->alignedW,
                    srcSurf->alignedH,
                    gcvFALSE,
                    gcvSURF_OPAQUE,
                    0
                    ));

                gcmERR_BREAK(gco2D_SetSource(
                    engine,
                    &tempRect
                    ));

                gcmERR_BREAK(gco2D_SetTargetEx(
                    engine,
                    destAddress[0],
                    DestSurface->info.stride,
                    DestSurface->info.rotation,
                    DestSurface->info.alignedW,
                    DestSurface->info.alignedH
                    ));

                gcmERR_BREAK(gco2D_Blit(
                    engine,
                    1,
                    &destSubRect,
                    0xCC,
                    0xCC,
                    DestSurface->info.format
                    ));

                if (bMirror)
                {
                    gcmERR_BREAK(gco2D_SetBitBlitMirror(
                        engine,
                        gcvFALSE,
                        gcvFALSE));
                }
            }
        }
        else
        {
            /* Call gco2D object to complete the blit. */
            gcmERR_BREAK(gco2D_FilterBlitEx(
                engine,
                srcAddress[0],
                SrcSurface->info.stride,
                srcAddress[1],
                SrcSurface->info.uStride,
                srcAddress[2],
                SrcSurface->info.vStride,
                SrcSurface->info.format,
                SrcSurface->info.rotation,
                SrcSurface->info.alignedW,
                SrcSurface->info.alignedH,
                SrcRect,
                destAddress[0],
                DestSurface->info.stride,
                DestSurface->info.format,
                DestSurface->info.rotation,
                DestSurface->info.alignedW,
                DestSurface->info.alignedH,
                DestRect,
                DestSubRect
                ));
        }
    }
    while (gcvFALSE);

    if (enable2DDither)
    {
        gcmVERIFY_OK(gco2D_EnableDither(
            engine,
            gcvFALSE));
    }

    if (srcRotBackup != (gceSURF_ROTATION)-1)
    {
        SrcSurface->info.rotation = srcRotBackup;
    }

    if (dstRotBackup != (gceSURF_ROTATION)-1)
    {
        DestSurface->info.rotation = dstRotBackup;
    }

    /* Unlock the source. */
    if (srcMemory[0] != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(SrcSurface, srcMemory[0]));
    }

    /* Unlock the destination. */
    if (destMemory[0] != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(DestSurface, destMemory[0]));
    }

    /* Free temp buffer. */
    if (tempSurf != gcvNULL)
    {
        gcmVERIFY_OK(gcoHARDWARE_Put2DTempSurface(gcvNULL, tempSurf));
    }

    if (temp2Surf != gcvNULL)
    {
        gcmVERIFY_OK(gcoHARDWARE_Put2DTempSurface(gcvNULL, temp2Surf));
    }

    if (ditherNotSupported)
    {
        status = gcvSTATUS_NOT_SUPPORT_DITHER;
    }

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_EnableAlphaBlend
**
**  Enable alpha blending engine in the hardware and disengage the ROP engine.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**      gctUINT8 SrcGlobalAlphaValue
**          Global alpha value for the color components.
**
**      gctUINT8 DstGlobalAlphaValue
**          Global alpha value for the color components.
**
**      gceSURF_PIXEL_ALPHA_MODE SrcAlphaMode
**          Per-pixel alpha component mode.
**
**      gceSURF_PIXEL_ALPHA_MODE DstAlphaMode
**          Per-pixel alpha component mode.
**
**      gceSURF_GLOBAL_ALPHA_MODE SrcGlobalAlphaMode
**          Global/per-pixel alpha values selection.
**
**      gceSURF_GLOBAL_ALPHA_MODE DstGlobalAlphaMode
**          Global/per-pixel alpha values selection.
**
**      gceSURF_BLEND_FACTOR_MODE SrcFactorMode
**          Final blending factor mode.
**
**      gceSURF_BLEND_FACTOR_MODE DstFactorMode
**          Final blending factor mode.
**
**      gceSURF_PIXEL_COLOR_MODE SrcColorMode
**          Per-pixel color component mode.
**
**      gceSURF_PIXEL_COLOR_MODE DstColorMode
**          Per-pixel color component mode.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_EnableAlphaBlend(
    IN gcoSURF Surface,
    IN gctUINT8 SrcGlobalAlphaValue,
    IN gctUINT8 DstGlobalAlphaValue,
    IN gceSURF_PIXEL_ALPHA_MODE SrcAlphaMode,
    IN gceSURF_PIXEL_ALPHA_MODE DstAlphaMode,
    IN gceSURF_GLOBAL_ALPHA_MODE SrcGlobalAlphaMode,
    IN gceSURF_GLOBAL_ALPHA_MODE DstGlobalAlphaMode,
    IN gceSURF_BLEND_FACTOR_MODE SrcFactorMode,
    IN gceSURF_BLEND_FACTOR_MODE DstFactorMode,
    IN gceSURF_PIXEL_COLOR_MODE SrcColorMode,
    IN gceSURF_PIXEL_COLOR_MODE DstColorMode
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x SrcGlobalAlphaValue=%u DstGlobalAlphaValue=%u "
              "SrcAlphaMode=%d DstAlphaMode=%d SrcGlobalAlphaMode=%d "
              "DstGlobalAlphaMode=%d SrcFactorMode=%d DstFactorMode=%d "
              "SrcColorMode=%d DstColorMode=%d",
              Surface, SrcGlobalAlphaValue, DstGlobalAlphaValue, SrcAlphaMode,
              DstAlphaMode, SrcGlobalAlphaMode, DstGlobalAlphaMode,
              SrcFactorMode, DstFactorMode, SrcColorMode, DstColorMode);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    do {
        gco2D engine;

        gcmERR_BREAK(gcoHAL_Get2DEngine(gcvNULL, &engine));

        gcmERR_BREAK(gco2D_EnableAlphaBlend(
                engine,
                (gctUINT32)SrcGlobalAlphaValue << 24,
                (gctUINT32)DstGlobalAlphaValue << 24,
                SrcAlphaMode,
                DstAlphaMode,
                SrcGlobalAlphaMode,
                DstGlobalAlphaMode,
                SrcFactorMode,
                DstFactorMode,
                SrcColorMode,
                DstColorMode
                ));

    } while (gcvFALSE);

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_DisableAlphaBlend
**
**  Disable alpha blending engine in the hardware and engage the ROP engine.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_DisableAlphaBlend(
    IN gcoSURF Surface
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    do
    {
        gco2D engine;
        gcmERR_BREAK(gcoHAL_Get2DEngine(gcvNULL, &engine));
        gcmERR_BREAK(gco2D_DisableAlphaBlend(engine));
    }
    while (gcvFALSE);

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_Set2DSource
**
**  Set source surface for 2D engine.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**      gceSURF_ROTATION Rotation
**          Source rotation parameter.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_Set2DSource(
    gcoSURF Surface,
    gceSURF_ROTATION Rotation
    )
{
    gceSTATUS status;
    gco2D engine;
    gctUINT addressNum;
    gctUINT physical[3];
    gctUINT stride[3];
    gctUINT width;
    gctUINT height;
    gce2D_TILE_STATUS_CONFIG tileStatusConfig;
    gctBOOL locked = gcvFALSE;
#if gcdENABLE_3D
    gctUINT32 tileStatusAddress;
#endif

    gcmHEADER_ARG("Source=0x%x Rotation=%d", Surface, Rotation);
    gcmVERIFY_ARGUMENT(Surface != gcvNULL);

    /* Get 2D engine. */
    gcmONERROR(gcoHAL_Get2DEngine(gcvNULL, &engine));

    /* Get shortcut. */
    addressNum = Surface->info.node.count;

    /* Calculate surface size. */
    width  = Surface->info.requestW;
    height = Surface->info.requestH;

    gcmONERROR(gcoSURF_Lock(
        Surface,
        physical,
        gcvNULL
        ));

    locked = gcvTRUE;

    /* Get physical addresses and strides. */
    switch (addressNum)
    {
    case 3:
        stride  [2] = Surface->info.vStride;
    case 2:
        stride  [1] = Surface->info.uStride;
    case 1:
        stride  [0] = Surface->info.stride;
        break;
    }

    if (Surface->info.tiling & gcvTILING_SPLIT_BUFFER)
    {
        /* Split buffer needs two addresses. */
        gcmASSERT(addressNum == 1);

        physical[1] = Surface->info.node.physicalBottom;
        stride  [1] = Surface->info.stride;
        addressNum  = 2;
    }

    /* Extract master surface arguments and call 2D api. */
    gcmONERROR(
        gco2D_SetGenericSource(engine,
                               physical,
                               addressNum,
                               stride,
                               addressNum,
                               Surface->info.tiling,
                               Surface->info.format,
                               Rotation,
                               width,
                               height));

#if gcdENABLE_3D
    /* Extract tile status arguments. */
    if ((Surface->info.tileStatusNode.pool == gcvPOOL_UNKNOWN) ||
        (Surface->info.tileStatusDisabled) ||
        (Surface->info.dirty == gcvFALSE))
    {
        /* No tile status or tile status disabled. */
        tileStatusConfig = gcv2D_TSC_DISABLE;
    }
    else
    {
        /* Tile status enabled. */
        tileStatusConfig = Surface->info.compressed ? gcv2D_TSC_COMPRESSED
                         : gcv2D_TSC_ENABLE;
    }

    if (Surface->info.sampleInfo.product > 1)
    {
        /* Down-sample. */
        tileStatusConfig |= gcv2D_TSC_DOWN_SAMPLER;
    }

    gcmGETHARDWAREADDRESS(Surface->info.tileStatusNode, tileStatusAddress)

    /* Set tile status states. */
    gcmONERROR(
        gco2D_SetSourceTileStatus(engine,
                                  tileStatusConfig,
                                  Surface->info.format,
                                  Surface->info.fcValue,
                                  tileStatusAddress));

#else
    /* No 3D. */
    tileStatusConfig = gcv2D_TSC_DISABLE;

    /* Set tile status states. */
    gcmONERROR(
        gco2D_SetSourceTileStatus(engine,
                                  tileStatusConfig,
                                  gcvSURF_UNKNOWN,
                                  0,
                                  ~0U));
#endif

    if (locked)
    {
        gcoSURF_Unlock(Surface, gcvNULL);
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    if (locked)
    {
        gcoSURF_Unlock(Surface, gcvNULL);
    }

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_Set2DTarget
**
**  Set target surface for 2D engine.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**      gceSURF_ROTATION Rotation
**          Destination rotation parameter.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_Set2DTarget(
    gcoSURF Surface,
    gceSURF_ROTATION Rotation
    )
{
    gceSTATUS status;
    gco2D engine;
    gctUINT addressNum;
    gctUINT physical[3];
    gctUINT stride[3];
    gctUINT width;
    gctUINT height;
    gce2D_TILE_STATUS_CONFIG tileStatusConfig;
    gctBOOL locked = gcvFALSE;
#if gcdENABLE_3D
    gctUINT32 tileStatusAddress;
#endif

    gcmHEADER_ARG("Source=0x%x Rotation=%d", Surface, Rotation);
    gcmVERIFY_ARGUMENT(Surface != gcvNULL);

    /* Get 2D engine. */
    gcmONERROR(gcoHAL_Get2DEngine(gcvNULL, &engine));

    /* Get shortcut. */
    addressNum = Surface->info.node.count;

    /* Calculate surface size. */
    width  = Surface->info.requestW;
    height = Surface->info.requestH;

    gcmONERROR(gcoSURF_Lock(
        Surface,
        physical,
        gcvNULL
        ));

    locked = gcvTRUE;

    /* Get physical addresses and strides. */
    switch (addressNum)
    {
    case 3:
        stride  [2] = Surface->info.vStride;
    case 2:
        stride  [1] = Surface->info.uStride;
    case 1:
        stride  [0] = Surface->info.stride;
        break;
    }

    if (Surface->info.tiling & gcvTILING_SPLIT_BUFFER)
    {
        /* Split buffer needs two addresses. */
        gcmASSERT(addressNum == 1);

        physical[1] = Surface->info.node.physicalBottom;
        stride  [1] = Surface->info.stride;
        addressNum  = 2;
    }

    /* Extract master surface arguments and call 2D api. */
    gcmONERROR(
        gco2D_SetGenericTarget(engine,
                               physical,
                               addressNum,
                               stride,
                               addressNum,
                               Surface->info.tiling,
                               Surface->info.format,
                               Rotation,
                               width,
                               height));

#if gcdENABLE_3D
    /* Extract tile status arguments. */
    if ((Surface->info.tileStatusNode.pool == gcvPOOL_UNKNOWN) ||
        (Surface->info.tileStatusDisabled) ||
        (Surface->info.dirty == gcvFALSE))
    {
        /* No tile status or tile status disabled. */
        tileStatusConfig = gcv2D_TSC_DISABLE;
    }
    else
    {
        /* Tile status enabled. */
        tileStatusConfig = Surface->info.compressed ? gcv2D_TSC_COMPRESSED
                         : gcv2D_TSC_ENABLE;
    }

    gcmGETHARDWAREADDRESS(Surface->info.tileStatusNode, tileStatusAddress)

    /* Set tile status states. */
    gcmONERROR(
        gco2D_SetTargetTileStatus(engine,
                                  tileStatusConfig,
                                  Surface->info.format,
                                  Surface->info.fcValue,
                                  tileStatusAddress));

#else
    /* No 3D. */
    tileStatusConfig = gcv2D_TSC_DISABLE;

    /* Set tile status states. */
    gcmONERROR(
        gco2D_SetTargetTileStatus(engine,
                                  tileStatusConfig,
                                  gcvSURF_UNKNOWN,
                                  0,
                                  ~0U));
#endif

    if (locked)
    {
        gcoSURF_Unlock(Surface, gcvNULL);
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    if (locked)
    {
        gcoSURF_Unlock(Surface, gcvNULL);
    }

    gcmFOOTER();
    return status;
}
#endif  /* gcdENABLE_2D */

#if gcdENABLE_3D
/*******************************************************************************
**
**  gcoSURF_CopyPixels
**
**  Copy a rectangular area from one surface to another with format conversion.
**
**  INPUT:
**
**      gcoSURF Source
**          Pointer to the source surface.
**
**      gcoSURF Target
**          Pointer to the target surface.
**
**      gctINT SourceX, SourceY
**          Source surface origin.
**
**      gctINT TargetX, TargetY
**          Target surface origin.
**
**      gctINT Width, Height
**          The size of the area. If Height is negative, the area will
**          be copied with a vertical flip.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_CopyPixels_v2(
    IN gcsSURF_VIEW *SrcView,
    IN gcsSURF_VIEW *DstView,
    IN gcsSURF_RESOLVE_ARGS *Args
    )
{
    gctPOINTER srcBase[3] = {gcvNULL};
    gctPOINTER dstBase[3] = {gcvNULL};
    gcoSURF srcSurf = SrcView->surf;
    gcoSURF dstSurf = DstView->surf;
    gcsPOINT_PTR srcOrigin = &Args->uArgs.v2.srcOrigin;
    gcsPOINT_PTR dstOrigin = &Args->uArgs.v2.dstOrigin;
    gcsPOINT rectSize = Args->uArgs.v2.rectSize;
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL cpuBlt = gcvFALSE;

    gcmHEADER_ARG("SrcView=0x%x DstView=0x%x Args=0x%x", SrcView, DstView, Args);

    if (Args->version != gcvHAL_ARG_VERSION_V2)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Lock the surfaces. */
    gcmONERROR(gcoSURF_Lock(srcSurf, gcvNULL, srcBase));
    gcmONERROR(gcoSURF_Lock(dstSurf, gcvNULL, dstBase));

    do
    {
        /* Limit copy rectangle not out of range */
        if (rectSize.x > (gctINT)srcSurf->info.allocedW - srcOrigin->x)
        {
            rectSize.x = (gctINT)srcSurf->info.allocedW - srcOrigin->x;
        }
        if (rectSize.x > (gctINT)dstSurf->info.allocedW - dstOrigin->x)
        {
            rectSize.x = (gctINT)dstSurf->info.allocedW - dstOrigin->x;
        }
        if (rectSize.y > (gctINT)srcSurf->info.allocedH - srcOrigin->y)
        {
            rectSize.y = (gctINT)srcSurf->info.allocedH - srcOrigin->y;
        }
        if (rectSize.y > (gctINT)dstSurf->info.allocedH - dstOrigin->y)
        {
            rectSize.y = (gctINT)dstSurf->info.allocedH - dstOrigin->y;
        }

        if (srcSurf->info.type == gcvSURF_BITMAP)
        {
            /* Flush the CPU cache. Source would've been rendered by the CPU. */
            gcmONERROR(gcoSURF_NODE_Cache(
                &srcSurf->info.node,
                srcBase[0],
                srcSurf->info.size,
                gcvCACHE_CLEAN));
        }

        if (dstSurf->info.type == gcvSURF_BITMAP)
        {
            gcmONERROR(gcoSURF_NODE_Cache(
                &dstSurf->info.node,
                dstBase[0],
                dstSurf->info.size,
                gcvCACHE_FLUSH));
        }

        /* Flush the surfaces. */
        gcmONERROR(gcoSURF_Flush(srcSurf));
        gcmONERROR(gcoSURF_Flush(dstSurf));

#if gcdENABLE_3D
        if (!srcSurf->info.isMsaa)
        {
            gcmONERROR(gcoSURF_DisableTileStatus(srcSurf, gcvTRUE));
        }

        /* Disable the tile status for the destination. */
        gcmONERROR(gcoSURF_DisableTileStatus(dstSurf, gcvTRUE));
#endif /* gcdENABLE_3D */

        /* Only unsigned normalized data type and no space conversion needed go hardware copy pixels path.
        ** as this path can't handle other cases.
        */
        if ((srcSurf->info.formatInfo.fmtDataType == gcvFORMAT_DATATYPE_UNSIGNED_NORMALIZED) &&
            (dstSurf->info.formatInfo.fmtDataType == gcvFORMAT_DATATYPE_UNSIGNED_NORMALIZED) &&
            (!srcSurf->info.formatInfo.fakedFormat || (srcSurf->info.paddingFormat && !srcSurf->info.garbagePadded)) &&
            (!dstSurf->info.formatInfo.fakedFormat || dstSurf->info.paddingFormat) &&
            (srcSurf->info.colorSpace == dstSurf->info.colorSpace))
        {
            /* Read the pixel. */
            gcsSURF_RESOLVE_ARGS adjustArgs = {0};

            gcoOS_MemCopy(&adjustArgs, Args, gcmSIZEOF(adjustArgs));
            adjustArgs.uArgs.v2.rectSize = rectSize;
            if (gcmIS_ERROR(gcoHARDWARE_CopyPixels_v2(gcvNULL, SrcView, DstView, &adjustArgs)))
            {
                cpuBlt = gcvTRUE;
            }
        }
        else
        {
            cpuBlt = gcvTRUE;

        }

        if (cpuBlt)
        {
            gcsSURF_BLIT_ARGS arg;

            gcoOS_ZeroMemory(&arg, sizeof(arg));

            arg.srcSurface = srcSurf;
            arg.srcX       = srcOrigin->x;
            arg.srcY       = srcOrigin->y;
            arg.srcZ       = SrcView->firstSlice;
            arg.dstSurface = dstSurf;
            arg.dstX       = dstOrigin->x;
            arg.dstY       = dstOrigin->y;
            arg.dstZ       = DstView->firstSlice;
            arg.srcWidth   = arg.dstWidth  = rectSize.x;
            arg.srcHeight  = arg.dstHeight = rectSize.y;
            arg.srcDepth   = arg.dstDepth  = 1;
            arg.yReverse   = Args->uArgs.v2.yInverted;
            gcmERR_BREAK(gcoSURF_BlitCPU(&arg));
        }

        if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_PE_DITHER_FIX) == gcvFALSE &&
            dstOrigin->x == 0 && rectSize.x >= (gctINT)dstSurf->info.requestW &&
            dstOrigin->y == 0 && rectSize.y >= (gctINT)dstSurf->info.requestH)
        {
            dstSurf->info.deferDither3D = gcvFALSE;
        }
    }
    while (gcvFALSE);

OnError:
    /* Unlock the surfaces. */
    if (dstBase[0] != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(dstSurf, dstBase[0]));
    }

    if (srcBase[0] != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(srcSurf, srcBase[0]));
    }

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_ReadPixel
**
**  gcoSURF_ReadPixel reads and returns the current value of the pixel from
**  the specified surface. The pixel value is returned in the specified format.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**      gctPOINTER Memory
**          Pointer to the actual surface bits returned by gcoSURF_Lock.
**
**      gctINT X, Y
**          Coordinates of the pixel.
**
**      gceSURF_FORMAT Format
**          Format of the pixel value to be returned.
**
**  OUTPUT:
**
**      gctPOINTER PixelValue
**          Pointer to the placeholder for the result.
*/
gceSTATUS
gcoSURF_ReadPixel(
    IN gcoSURF Surface,
    IN gctPOINTER Memory,
    IN gctINT X,
    IN gctINT Y,
    IN gceSURF_FORMAT Format,
    OUT gctPOINTER PixelValue
    )
{
    gcmPRINT("ERROR: %s was deprecated!!!\n", __FUNCTION__);
    gcmASSERT(gcvFALSE);
    return gcvSTATUS_INVALID_REQUEST;
}

/*******************************************************************************
**
**  gcoSURF_WritePixel
**
**  gcoSURF_WritePixel writes a color value to a pixel of the specified surface.
**  The pixel value is specified in the specified format.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**      gctPOINTER Memory
**          Pointer to the actual surface bits returned by gcoSURF_Lock.
**
**      gctINT X, Y
**          Coordinates of the pixel.
**
**      gceSURF_FORMAT Format
**          Format of the pixel value to be returned.
**
**      gctPOINTER PixelValue
**          Pointer to the pixel value.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_WritePixel(
    IN gcoSURF Surface,
    IN gctPOINTER Memory,
    IN gctINT X,
    IN gctINT Y,
    IN gceSURF_FORMAT Format,
    IN gctPOINTER PixelValue
    )
{
    gcmPRINT("ERROR: %s was deprecated!!!\n", __FUNCTION__);
    gcmASSERT(gcvFALSE);
    return gcvSTATUS_INVALID_REQUEST;
}
#endif /* gcdENABLE_3D */

gceSTATUS
gcoSURF_NODE_Cache(
    IN gcsSURF_NODE_PTR Node,
    IN gctPOINTER Logical,
    IN gctSIZE_T Bytes,
    IN gceCACHEOPERATION Operation
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Node=0x%x, Operation=%d, Bytes=%u", Node, Operation, Bytes);

    if (Node->pool == gcvPOOL_USER)
    {
        gcmFOOTER();
        return gcvSTATUS_OK;
    }

#if !gcdPAGED_MEMORY_CACHEABLE
    if (Node->u.normal.cacheable == gcvFALSE)
    {
        gcmFOOTER();
        return gcvSTATUS_OK;
    }
#endif

    switch (Operation)
    {
        case gcvCACHE_CLEAN:
            gcmONERROR(gcoOS_CacheClean(gcvNULL, Node->u.normal.node, Logical, Bytes));
            break;

        case gcvCACHE_INVALIDATE:
            gcmONERROR(gcoOS_CacheInvalidate(gcvNULL, Node->u.normal.node, Logical, Bytes));
            break;

        case gcvCACHE_FLUSH:
            gcmONERROR(gcoOS_CacheFlush(gcvNULL, Node->u.normal.node, Logical, Bytes));
            break;

        default:
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            break;
    }

    gcmFOOTER();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_NODE_CPUCacheOperation
**
**  Perform the specified CPU cache operation on the surface node.
**
**  INPUT:
**
**      gcsSURF_NODE_PTR Node
**          Pointer to the surface node.
**      gctPOINTER Logical
**          logical address to flush.
**      gctSIZE_T Bytes
**          bytes to flush.
**      gceSURF_CPU_CACHE_OP_TYPE Operation
**          Cache operation to be performed.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_NODE_CPUCacheOperation(
    IN gcsSURF_NODE_PTR Node,
    IN gceSURF_TYPE Type,
    IN gctSIZE_T Offset,
    IN gctSIZE_T Length,
    IN gceCACHEOPERATION Operation
    )
{

    gceSTATUS status;
    gctPOINTER memory;
    gctBOOL locked = gcvFALSE;

    gcmHEADER_ARG("Node=0x%x, Type=%u, Offset=%u, Length=%u, Operation=%d", Node, Type, Offset, Length, Operation);

    /* Lock the node. */
    gcmONERROR(gcoHARDWARE_Lock(Node, gcvNULL, &memory));
    locked = gcvTRUE;

    gcmONERROR(gcoSURF_NODE_Cache(Node,
                                  (gctUINT8_PTR)memory + Offset,
                                  Length,
                                  Operation));


    /* Unlock the node. */
    gcmONERROR(gcoHARDWARE_Unlock(Node, Type));
    locked = gcvFALSE;

    gcmFOOTER();
    return gcvSTATUS_OK;

OnError:
    if (locked)
    {
        gcmVERIFY_OK(gcoHARDWARE_Unlock(Node, Type));
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
gcoSURF_LockNode(
                 IN gcsSURF_NODE_PTR Node,
                 OUT gctUINT32 * Address,
                 OUT gctPOINTER * Memory
                 )
{
    gceSTATUS status;

    gcmHEADER_ARG("Node=0x%x, Address=0x%x, Memory=0x%x", Node, Address, Memory);

    gcmONERROR(gcoHARDWARE_Lock(Node, Address, Memory));

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoSURF_UnLockNode(
                   IN gcsSURF_NODE_PTR Node,
                   IN gceSURF_TYPE Type
                   )
{
    gceSTATUS status;

    gcmHEADER_ARG("Node=0x%x, Type=%u", Node, Type);

    gcmONERROR(gcoHARDWARE_Unlock(Node, Type));

OnError:
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_CPUCacheOperation
**
**  Perform the specified CPU cache operation on the surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**      gceSURF_CPU_CACHE_OP_TYPE Operation
**          Cache operation to be performed.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_CPUCacheOperation(
    IN gcoSURF Surface,
    IN gceCACHEOPERATION Operation
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctPOINTER source[3] = {0};
    gctBOOL locked = gcvFALSE;

    gcmHEADER_ARG("Surface=0x%x, Operation=%d", Surface, Operation);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Lock the surfaces. */
    gcmONERROR(gcoSURF_Lock(Surface, gcvNULL, source));
    locked = gcvTRUE;

    gcmONERROR(gcoSURF_NODE_Cache(&Surface->info.node,
                                  source[0],
                                  Surface->info.node.size,
                                  Operation));

    /* Unlock the surfaces. */
    gcmONERROR(gcoSURF_Unlock(Surface, source[0]));
    locked = gcvFALSE;

    gcmFOOTER();
    return gcvSTATUS_OK;

OnError:
    if (locked)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(Surface, source[0]));
    }

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_Flush
**
**  Flush the caches to make sure the surface has all pixels.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_Flush(
    IN gcoSURF Surface
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x", Surface);


    /* Flush the current pipe. */
    status = gcoHARDWARE_FlushPipe(gcvNULL, gcvNULL);
    gcmFOOTER();
    return status;
}

#if gcdENABLE_3D
/*******************************************************************************
**
**  gcoSURF_FillFromTile
**
**  Fill the surface from the information in the tile status buffer.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_FillFromTile(
    IN gcoSURF Surface
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_TILE_FILLER)
    &&  (Surface->info.type == gcvSURF_RENDER_TARGET)
    &&  (!Surface->info.isMsaa)
    &&  (Surface->info.compressed == gcvFALSE)
    &&  (Surface->info.tileStatusNode.pool != gcvPOOL_UNKNOWN)
    &&  (Surface->info.tileStatusDisabled == gcvFALSE)
    &&  ((Surface->info.node.size & 0x3fff) == 0))
    {
        /*
         * Call underlying tile status disable to do FC fill:
         * 1. Flush pipe / tile status cache
         * 2. Decompress (Fill) tile status
         * 3. Set surface tileStatusDisabled to true.
         */
        gcmONERROR(
            gcoHARDWARE_DisableTileStatus(gcvNULL,
                                          &Surface->info,
                                          gcvTRUE));
    }
    else
    if ((Surface->info.tileStatusNode.pool == gcvPOOL_UNKNOWN)
    ||  (Surface->info.tileStatusDisabled == gcvTRUE))
    {
        /* Flush pipe cache. */
        gcmONERROR(gcoHARDWARE_FlushPipe(gcvNULL, gcvNULL));

        /*
         * No need to fill if tile status disabled.
         * Return OK here to tell the caller that FC fill is done, because
         * the caller(drivers) may be unable to know it.
         */
        status = gcvSTATUS_OK;
    }
    else
    {
        /* Set return value. */
        status = gcvSTATUS_NOT_SUPPORTED;
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}
#endif

#if gcdENABLE_3D || gcdENABLE_VG
/*******************************************************************************
**
**  gcoSURF_SetSamples
**
**  Set the number of samples per pixel.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**      gctUINT Samples
**          Number of samples per pixel.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_SetSamples(
    IN gcoSURF Surface,
    IN gctUINT Samples
    )
{
    gctUINT samples;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Surface=0x%x Samples=%u", Surface, Samples);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Make sure this is not user-allocated surface. */
    if (Surface->info.node.pool == gcvPOOL_USER)
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    samples = (Samples == 0) ? 1 : Samples;

    if (Surface->info.sampleInfo.product != samples)
    {
        gceSURF_TYPE type = Surface->info.type;

        /* TODO: Shall we add hints bits? */
        type = (gceSURF_TYPE)(type | Surface->info.hints);

        /* Destroy existing surface memory. */
        gcmONERROR(_FreeSurface(Surface));

        /* Allocate new surface. */
        gcmONERROR(
            _AllocateSurface(Surface,
                             Surface->info.requestW,
                             Surface->info.requestH,
                             Surface->info.requestD,
                             type,
                             Surface->info.format,
                             samples,
                             gcvPOOL_DEFAULT));
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_GetSamples
**
**  Get the number of samples per pixel.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**  OUTPUT:
**
**      gctUINT_PTR Samples
**          Pointer to variable receiving the number of samples per pixel.
**
*/
gceSTATUS
gcoSURF_GetSamples(
    IN gcoSURF Surface,
    OUT gctUINT_PTR Samples
    )
{
    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);
    gcmVERIFY_ARGUMENT(Samples != gcvNULL);

    /* Return samples. */
    *Samples = Surface->info.sampleInfo.product;

    /* Success. */
    gcmFOOTER_ARG("*Samples=%u", *Samples);
    return gcvSTATUS_OK;
}
#endif

/*******************************************************************************
**
**  gcoSURF_SetResolvability
**
**  Set the resolvability of a surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**      gctBOOL Resolvable
**          gcvTRUE if the surface is resolvable or gcvFALSE if not.  This is
**          required for alignment purposes.
**
**  OUTPUT:
**
*/
gceSTATUS
gcoSURF_SetResolvability(
    IN gcoSURF Surface,
    IN gctBOOL Resolvable
    )
{
#if gcdENABLE_3D
    gcmHEADER_ARG("Surface=0x%x Resolvable=%d", Surface, Resolvable);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Set the resolvability. */
    Surface->resolvable = Resolvable;

    /* Success. */
    gcmFOOTER_NO();
#endif
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoSURF_SetOrientation
**
**  Set the orientation of a surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**      gceORIENTATION Orientation
**          The requested surface orientation.  Orientation can be one of the
**          following values:
**
**              gcvORIENTATION_TOP_BOTTOM - Surface is from top to bottom.
**              gcvORIENTATION_BOTTOM_TOP - Surface is from bottom to top.
**
**  OUTPUT:
**
*/
gceSTATUS
gcoSURF_SetOrientation(
    IN gcoSURF Surface,
    IN gceORIENTATION Orientation
    )
{
    gcmHEADER_ARG("Surface=0x%x Orientation=%d", Surface, Orientation);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

#if !gcdREMOVE_SURF_ORIENTATION
    /* Set the orientation. */
    Surface->info.orientation = Orientation;

#endif
    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoSURF_QueryOrientation
**
**  Query the orientation of a surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**  OUTPUT:
**
**      gceORIENTATION * Orientation
**          Pointer to a variable receiving the surface orientation.
**
*/
gceSTATUS
gcoSURF_QueryOrientation(
    IN gcoSURF Surface,
    OUT gceORIENTATION * Orientation
    )
{
    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);
    gcmVERIFY_ARGUMENT(Orientation != gcvNULL);

    /* Return the orientation. */
    *Orientation = Surface->info.orientation;

    /* Success. */
    gcmFOOTER_ARG("*Orientation=%d", *Orientation);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoSURF_QueryFlags
**
**  Query status of the flag.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to surface object.
**      gceSURF_FLAG Flag
**          Flag which is queried
**
**  OUTPUT:
**      None
**
*/
gceSTATUS
gcoSURF_QueryFlags(
    IN gcoSURF Surface,
    IN gceSURF_FLAG Flag
    )
{
    gceSTATUS status = gcvSTATUS_TRUE;
    gcmHEADER_ARG("Surface=0x%x Flag=0x%x", Surface, Flag);

    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    if (Surface->info.flags & Flag)
    {
        status = gcvSTATUS_TRUE;
    }
    else
    {
        status = gcvSTATUS_FALSE;
    }
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoSURF_QueryHints(
    IN gcoSURF Surface,
    IN gceSURF_TYPE Hints
    )
{
    gceSTATUS status = gcvSTATUS_TRUE;
    gcmHEADER_ARG("Surface=0x%x Hints=0x%x", Surface, Hints);

    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    if (Surface->info.hints & Hints)
    {
        status = gcvSTATUS_TRUE;
    }
    else
    {
        status = gcvSTATUS_FALSE;
    }
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_QueryFormat
**
**  Return pixel format parameters.
**
**  INPUT:
**
**      gceSURF_FORMAT Format
**          API format.
**
**  OUTPUT:
**
**      gcsSURF_FORMAT_INFO_PTR * Info
**          Pointer to a variable that will hold the format description entry.
**          If the format in question is interleaved, two pointers will be
**          returned stored in an array fashion.
**
*/
gceSTATUS
gcoSURF_QueryFormat(
    IN gceSURF_FORMAT Format,
    OUT gcsSURF_FORMAT_INFO_PTR * Info
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Format=%d", Format);

    status = gcoHARDWARE_QueryFormat(Format, Info);

    gcmFOOTER();
    return status;
}



/*******************************************************************************
**
**  gcoSURF_SetColorType
**
**  Set the color type of the surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**      gceSURF_COLOR_TYPE colorType
**          color type of the surface.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_SetColorType(
    IN gcoSURF Surface,
    IN gceSURF_COLOR_TYPE ColorType
    )
{
    gcmHEADER_ARG("Surface=0x%x ColorType=%d", Surface, ColorType);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Set the color type. */
    Surface->info.colorType = ColorType;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoSURF_GetColorType
**
**  Get the color type of the surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**  OUTPUT:
**
**      gceSURF_COLOR_TYPE *colorType
**          pointer to the variable receiving color type of the surface.
**
*/
gceSTATUS
gcoSURF_GetColorType(
    IN gcoSURF Surface,
    OUT gceSURF_COLOR_TYPE *ColorType
    )
{
    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);
    gcmVERIFY_ARGUMENT(ColorType != gcvNULL);

    /* Return the color type. */
    *ColorType = Surface->info.colorType;

    /* Success. */
    gcmFOOTER_ARG("*ColorType=%d", *ColorType);
    return gcvSTATUS_OK;
}




/*******************************************************************************
**
**  gcoSURF_SetColorSpace
**
**  Set the color type of the surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**      gceSURF_COLOR_SPACE ColorSpace
**          color space of the surface.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_SetColorSpace(
    IN gcoSURF Surface,
    IN gceSURF_COLOR_SPACE ColorSpace
    )
{
    gcmHEADER_ARG("Surface=0x%x ColorSpace=%d", Surface, ColorSpace);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Set the color space. */
    Surface->info.colorSpace = ColorSpace;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoSURF_GetColorSpace
**
**  Get the color space of the surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**  OUTPUT:
**
**      gceSURF_COLOR_SPACE *ColorSpace
**          pointer to the variable receiving color space of the surface.
**
*/
gceSTATUS
gcoSURF_GetColorSpace(
    IN gcoSURF Surface,
    OUT gceSURF_COLOR_SPACE *ColorSpace
    )
{
    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);
    gcmVERIFY_ARGUMENT(ColorSpace != gcvNULL);

    /* Return the color type. */
    *ColorSpace = Surface->info.colorSpace;

    /* Success. */
    gcmFOOTER_ARG("*ColorSpace=%d", *ColorSpace);
    return gcvSTATUS_OK;
}


/*******************************************************************************
**
**  gcoSURF_SetRotation
**
**  Set the surface ration angle.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**      gceSURF_ROTATION Rotation
**          Rotation angle.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_SetRotation(
    IN gcoSURF Surface,
    IN gceSURF_ROTATION Rotation
    )
{
    gcmHEADER_ARG("Surface=0x%x Rotation=%d", Surface, Rotation);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Support only 2D surfaces. */
    if (Surface->info.type != gcvSURF_BITMAP)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* Set new rotation. */
    Surface->info.rotation = Rotation;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoSURF_SetDither
**
**  Set the surface dither flag.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**      gceSURF_ROTATION Dither
**          dither enable or not.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_SetDither(
    IN gcoSURF Surface,
    IN gctBOOL Dither
    )
{
    gcmHEADER_ARG("Surface=0x%x Dither=%d", Surface, Dither);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Support only 2D surfaces. */
    if (Surface->info.type != gcvSURF_BITMAP)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* Set new rotation. */
    Surface->info.dither2D = Dither;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

#if gcdENABLE_3D || gcdENABLE_VG
/*******************************************************************************
**
**  gcoSURF_Copy
**
**  Copy one tiled surface to another tiled surfaces.  This is used for handling
**  unaligned surfaces.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the gcoSURF object that describes the surface to copy
**          into.
**
**      gcoSURF Source
**          Pointer to the gcoSURF object that describes the source surface to
**          copy from.
**
**  OUTPUT:
**
**      Nothing.
**
*/
gceSTATUS
gcoSURF_Copy(
    IN gcoSURF Surface,
    IN gcoSURF Source
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT8_PTR source = gcvNULL, target = gcvNULL;

#if gcdENABLE_VG
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;
#endif

    gcmHEADER_ARG("Surface=0x%x Source=0x%x", Surface, Source);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);
    gcmVERIFY_OBJECT(Source, gcvOBJ_SURF);


    if ((Surface->info.tiling != Source->info.tiling) ||
        ((Surface->info.tiling != gcvTILED) &&
         (Surface->info.tiling != gcvSUPERTILED)
        )
       )
    {
        /* Both surfaces need to the same tiled.
        ** only tile and supertile are supported.
        */
        gcmFOOTER();
        return gcvSTATUS_INVALID_REQUEST;
    }

    do
    {
        gctUINT y;
        gctUINT sourceOffset, targetOffset;
        gctINT height = 0;
        gctPOINTER pointer[3] = { gcvNULL };

#if gcdENABLE_VG
        gcmGETCURRENTHARDWARE(currentType);
        if (currentType == gcvHARDWARE_VG)
        {
            /* Flush the pipe. */
            gcmERR_BREAK(gcoVGHARDWARE_FlushPipe(gcvNULL));

            /* Commit and stall the pipe. */
            gcmERR_BREAK(gcoHAL_Commit(gcvNULL, gcvTRUE));

            /* Get the tile height. */
            gcmERR_BREAK(gcoVGHARDWARE_QueryTileSize(
                                                   gcvNULL, gcvNULL,
                                                   gcvNULL, &height,
                                                   gcvNULL));
        }
        else
#endif
        {
            /* Flush the pipe. */
            gcmERR_BREAK(gcoHARDWARE_FlushPipe(gcvNULL, gcvNULL));

            /* Commit and stall the pipe. */
            gcmERR_BREAK(gcoHAL_Commit(gcvNULL, gcvTRUE));

            switch (Surface->info.tiling)
            {
            case gcvTILED:
                /* Get the tile height. */
                gcmERR_BREAK(gcoHARDWARE_QueryTileSize(gcvNULL, gcvNULL,
                                                       gcvNULL, &height,
                                                       gcvNULL));
                break;
            case gcvSUPERTILED:
                height = 64;
                break;
            default:
                gcmASSERT(0);
                height = 4;
                break;
            }
        }
        /* Lock the surfaces. */
        gcmERR_BREAK(gcoSURF_Lock(Source, gcvNULL, pointer));

        source = pointer[0];

        gcmERR_BREAK(gcoSURF_Lock(Surface, gcvNULL, pointer));

        target = pointer[0];

        /* Reset initial offsets. */
        sourceOffset = 0;
        targetOffset = 0;

        /* Loop target surface, one row of tiles at a time. */
        for (y = 0; y < Surface->info.alignedH; y += height)
        {
            /* Copy one row of tiles. */
            gcoOS_MemCopy(target + targetOffset,
                          source + sourceOffset,
                          Surface->info.stride * height);

            /* Move to next row of tiles. */
            sourceOffset += Source->info.stride  * height;
            targetOffset += Surface->info.stride * height;
        }
    }
    while (gcvFALSE);

    if (source != gcvNULL)
    {
        /* Unlock source surface. */
        gcmVERIFY_OK(gcoSURF_Unlock(Source, source));
    }

    if (target != gcvNULL)
    {
        /* Unlock target surface. */
        gcmVERIFY_OK(gcoSURF_Unlock(Surface, target));
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}
#endif

#if gcdENABLE_3D
gceSTATUS
gcoSURF_IsHWResolveable(
    IN gcoSURF SrcSurface,
    IN gcoSURF DestSurface,
    IN gcsPOINT_PTR SrcOrigin,
    IN gcsPOINT_PTR DestOrigin,
    IN gcsPOINT_PTR RectSize
    )
{
    gceSTATUS status;
    gcsPOINT  rectSize;
    gctINT maxWidth;
    gctINT maxHeight;

    gcmHEADER_ARG("SrcSurface=0x%x DestSurface=0x%x SrcOrigin=0x%x "
                  "DestOrigin=0x%x RectSize=0x%x",
                  SrcSurface, DestSurface, SrcOrigin, DestOrigin, RectSize);

    if ((DestOrigin->x == 0) &&
        (DestOrigin->y == 0) &&
        (RectSize->x == (gctINT)DestSurface->info.requestW) &&
        (RectSize->y == (gctINT)DestSurface->info.requestH))
    {
        /* Full destination resolve, a special case. */
        rectSize.x = DestSurface->info.alignedW;
        rectSize.y = DestSurface->info.alignedH;
    }
    else
    {
        rectSize.x = RectSize->x;
        rectSize.y = RectSize->y;
    }

    /* Make sure we don't go beyond the source surface. */
    maxWidth  = SrcSurface->info.alignedW - SrcOrigin->x;
    maxHeight = SrcSurface->info.alignedH - SrcOrigin->y;

    rectSize.x = gcmMIN(maxWidth,  rectSize.x);
    rectSize.y = gcmMIN(maxHeight, rectSize.y);

    /* Make sure we don't go beyond the target surface. */
    maxWidth  = DestSurface->info.alignedW - DestOrigin->x;
    maxHeight = DestSurface->info.alignedH - DestOrigin->y;

    rectSize.x = gcmMIN(maxWidth,  rectSize.x);
    rectSize.y = gcmMIN(maxHeight, rectSize.y);

    if ((SrcSurface->info.type == gcvSURF_DEPTH)
    &&  (SrcSurface->info.tileStatusNode.pool != gcvPOOL_UNKNOWN)
    )
    {
        status = gcvSTATUS_FALSE;
    }
    else
    {
         status = gcoHARDWARE_IsHWResolveable(&SrcSurface->info,
                                              &DestSurface->info,
                                              SrcOrigin,
                                              DestOrigin,
                                              &rectSize);
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}
#endif /* gcdENABLE_3D */

/*******************************************************************************
**
**  gcoSURF_ConstructWrapper
**
**  Create a new gcoSURF wrapper object.
**
**  INPUT:
**
**      gcoHAL Hal
**          Pointer to an gcoHAL object.
**
**  OUTPUT:
**
**      gcoSURF * Surface
**          Pointer to the variable that will hold the gcoSURF object pointer.
*/
gceSTATUS
gcoSURF_ConstructWrapper(
    IN gcoHAL Hal,
    OUT gcoSURF * Surface
    )
{
    gcoSURF surface;
    gceSTATUS status;
    gctUINT i;

    gcmHEADER();

    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT(Surface != gcvNULL);

    do
    {
        /* Allocate the gcoSURF object. */
        gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(struct _gcoSURF), (gctPOINTER*)&surface));

        /* Reset the object. */
        gcoOS_ZeroMemory(surface, gcmSIZEOF(struct _gcoSURF));

        /* Initialize the gcoSURF object.*/
        surface->object.type = gcvOBJ_SURF;

#if gcdENABLE_3D
        /* 1 sample per pixel. */
        surface->info.sampleInfo = g_sampleInfos[1];
        surface->info.isMsaa     = gcvFALSE;
        surface->info.edgeAA     = gcvFALSE;
#endif /* gcdENABLE_3D */

        /* One plane. */
        surface->info.requestD = 1;

        /* Initialize the node. */
        surface->info.node.pool      = gcvPOOL_USER;
        surface->info.node.physical2 = ~0U;
        surface->info.node.physical3 = ~0U;
        surface->info.node.count     = 1;
        surface->referenceCount = 1;

        surface->info.flags = gcvSURF_FLAG_NONE;

        for (i = 0; i < gcvHARDWARE_NUM_TYPES; i++)
        {
            surface->info.node.hardwareAddresses[i] = ~0U;
        }

        surface->info.pfGetAddr = gcoHARDWARE_GetProcCalcPixelAddr(gcvNULL, &surface->info);

        /* Return pointer to the gcoSURF object. */
        *Surface = surface;

        /* Success. */
        gcmFOOTER_ARG("*Surface=0x%x", *Surface);
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

OnError:

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_SetFlags
**
**  Set status of the flag.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to surface object.
**      gceSURF_FLAG Flag
**          Surface Flag
**      gctBOOL Value
**          New value for this flag.
**
**  OUTPUT:
**      None
**
*/
gceSTATUS
gcoSURF_SetFlags(
    IN gcoSURF Surface,
    IN gceSURF_FLAG Flag,
    IN gctBOOL Value
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Surface=0x%x Flag=0x%x Value=0x%x", Surface, Flag, Value);

    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    if (Value)
    {
        Surface->info.flags |= Flag;
    }
    else
    {
        Surface->info.flags &= ~Flag;
    }

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_SetBuffer
**
**  Set the underlying buffer for the surface wrapper.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to gcoSURF object.
**
**      gceSURF_TYPE Type
**          Type of surface to create.
**
**      gceSURF_FORMAT Format
**          Format of surface to create.
**
**      gctINT Stride
**          Surface stride. Is set to ~0 the stride will be autocomputed.
**
**      gctPOINTER Logical
**          Logical pointer to the user allocated surface or gcvNULL if no
**          logical pointer has been provided.
**
**      gctUINT32 Physical
**          Physical address (NOT GPU address) of a contiguous buffer.
**          It should be gcvINVALID_ADDRESS for non-contiguous buffer or
**          buffer whose physical address is unknown.
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_SetBuffer(
    IN gcoSURF Surface,
    IN gceSURF_TYPE Type,
    IN gceSURF_FORMAT Format,
    IN gctUINT Stride,
    IN gctPOINTER Logical,
    IN gctUINT32 Physical
    )
{
    gceSTATUS status;
#if gcdENABLE_VG
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;
#endif
    gcsSURF_FORMAT_INFO_PTR surfInfo;

    gcmHEADER_ARG("Surface=0x%x Type=%d Format=%d Stride=%u Logical=0x%x "
                  "Physical=%08x",
                  Surface, Type, Format, Stride, Logical, Physical);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Has to be user-allocated surface. */
    if (Surface->info.node.pool != gcvPOOL_USER)
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /* Unmap the current buffer if any. */
    gcmONERROR(_UnmapUserBuffer(Surface, gcvTRUE));

    /* Determine the stride parameters. */
    Surface->autoStride = (Stride == ~0U);

    /* Set surface parameters. */
    Surface->info.type   = (gceSURF_TYPE) ((gctUINT32) Type &  0xFF);
    Surface->info.hints  = (gceSURF_TYPE) ((gctUINT32) Type & ~0xFF);
    Surface->info.format = Format;
    Surface->info.stride = Stride;

    /* Set node pointers. */
    Surface->logical  = (gctUINT8_PTR) Logical;
    Surface->physical = Physical;

#if gcdENABLE_VG
    gcmGETCURRENTHARDWARE(currentType);
    if (currentType == gcvHARDWARE_VG)
    {
        /* Compute bits per pixel. */
        gcmONERROR(gcoVGHARDWARE_ConvertFormat(gcvNULL,
                                             Format,
                                             (gctUINT32_PTR)&Surface->info.bitsPerPixel,
                                             gcvNULL));
    }
    else
#endif
    {
        /* Compute bits per pixel. */
        gcmONERROR(gcoHARDWARE_ConvertFormat(Format,
                                             (gctUINT32_PTR)&Surface->info.bitsPerPixel,
                                             gcvNULL));
    }

    /* Initialize Surface->info.formatInfo */
    gcmONERROR(gcoSURF_QueryFormat(Format, &surfInfo));
    Surface->info.formatInfo = *surfInfo;
    Surface->info.bitsPerPixel = surfInfo->bitsPerPixel;

#if gcdENABLE_3D
    Surface->info.colorSpace = gcd_QUERY_COLOR_SPACE(Format);
#endif

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_SetVideoBuffer
**
**  Set the underlying video buffer for the surface wrapper.
**  The video plane addresses should be specified individually.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to gcoSURF object.
**
**      gceSURF_TYPE Type
**          Type of surface to create.
**
**      gceSURF_FORMAT Format
**          Format of surface to create.
**
**      gctINT Stride
**          Surface stride. Is set to ~0 the stride will be autocomputed.
**
**      gctPOINTER LogicalPlane1
**          Logical pointer to the first plane of the user allocated surface
**          or gcvNULL if no logical pointer has been provided.
**
**      gctUINT32 PhysicalPlane1
**          Physical pointer to the user allocated surface or ~0 if no
**          physical pointer has been provided.
**
**  OUTPUT:
**
**      Nothing.
*/

/*******************************************************************************
**
**  gcoSURF_SetWindow
**
**  Set the size of the surface in pixels and map the underlying buffer set by
**  gcoSURF_SetBuffer if necessary.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to gcoSURF object.
**
**      gctINT X, Y
**          The origin of the surface.
**
**      gctINT Width, Height
**          Size of the surface in pixels.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_SetWindow(
    IN gcoSURF Surface,
    IN gctUINT X,
    IN gctUINT Y,
    IN gctUINT Width,
    IN gctUINT Height
    )
{
    gceSTATUS status;
    gctUINT32 offsetX;
    gctUINT32 offsetY;
    gctUINT   userStride;
    gctUINT   halStride;
    gctINT    bytesPerPixel;
#if gcdSECURE_USER
    gcsHAL_INTERFACE iface;
#endif
#if gcdENABLE_VG
    gceHARDWARE_TYPE currentHW = gcvHARDWARE_INVALID;
#endif
    gcsSURF_FORMAT_INFO_PTR formatInfo;
    gcsUSER_MEMORY_DESC desc;

    gcmHEADER_ARG("Surface=0x%x X=%u Y=%u Width=%u Height=%u",
                  Surface, X, Y, Width, Height);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

#if gcdENABLE_VG
    gcmGETCURRENTHARDWARE(currentHW);
#endif

    /* Unmap the current buffer if any. */
    gcmONERROR(
        _UnmapUserBuffer(Surface, gcvTRUE));

    /* Make sure at least one of the surface pointers is set. */
    if ((Surface->logical == gcvNULL) && (Surface->physical == ~0U))
    {
        gcmONERROR(gcvSTATUS_INVALID_ADDRESS);
    }

    /* Set the size. */
    Surface->info.requestW = Width;
    Surface->info.requestH = Height;
    Surface->info.requestD = 1;
    Surface->info.allocedW = Width;
    Surface->info.allocedH = Height;
    Surface->info.alignedW = Width;
    Surface->info.alignedH = Height;

    /* Stride is the same as the width? */
    if (Surface->autoStride)
    {
        /* Compute the stride. */
        Surface->info.stride = Width * Surface->info.bitsPerPixel / 8;
    }
    else
    {
#if gcdENABLE_VG
        if (currentHW == gcvHARDWARE_VG)
        {
            gcmONERROR(
                gcoVGHARDWARE_AlignToTile(gcvNULL,
                                          Surface->info.type,
                                          &Surface->info.alignedW,
                                          &Surface->info.alignedH));
        }
        else
#endif
        {
            /* Align the surface size. */
#if gcdENABLE_3D
            gcmONERROR(
                gcoHARDWARE_AlignToTileCompatible(gcvNULL,
                                                  Surface->info.type,
                                                  0,
                                                  Surface->info.format,
                                                  &Surface->info.alignedW,
                                                  &Surface->info.alignedH,
                                                  Surface->info.requestD,
                                                  &Surface->info.tiling,
                                                  &Surface->info.superTiled,
                                                  &Surface->info.hAlignment));
#else
            gcmONERROR(
                gcoHARDWARE_AlignToTileCompatible(gcvNULL,
                                                  Surface->info.type,
                                                  0,
                                                  Surface->info.format,
                                                  &Surface->info.alignedW,
                                                  &Surface->info.alignedH,
                                                  Surface->info.requestD,
                                                  &Surface->info.tiling,
                                                  gcvNULL,
                                                  gcvNULL));
#endif /* gcdENABLE_3D */
        }
    }

#if gcdENABLE_VG
    if (currentHW != gcvHARDWARE_VG)
#endif
    {

    /* Get shortcut. */
    formatInfo = &Surface->info.formatInfo;

    /* bytes per pixel of first plane. */
    switch (Surface->info.format)
    {
    case gcvSURF_YV12:
    case gcvSURF_I420:
    case gcvSURF_NV12:
    case gcvSURF_NV21:
    case gcvSURF_NV16:
    case gcvSURF_NV61:
        bytesPerPixel = 1;
        halStride = Surface->info.alignedW;
        break;

    default:
        bytesPerPixel = Surface->info.bitsPerPixel / 8;
        halStride = (Surface->info.alignedW / formatInfo->blockWidth)
                  * (formatInfo->blockSize / formatInfo->layers) / 8;
        break;
    }

    /* Backup user stride. */
    userStride = Surface->info.stride;

    if (userStride != halStride)
    {
        if ((Surface->info.type != gcvSURF_BITMAP)
        ||  (Surface->info.stride < Width * bytesPerPixel)
        ||  (Surface->info.stride & (4 * bytesPerPixel - 1))
        )
        {
            /*
             * 1. For Vivante internal surfaces types, user buffer placement
             * must be the same as what defined in HAL, otherwise the user
             * buffer is not compatible with HAL.
             * 2. For bitmap surface, user buffer stride may be larger than
             * least stride defined in HAL, and should be aligned.
             */
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        /* Calculte alignedWidth from user stride. */
        Surface->info.alignedW = userStride / bytesPerPixel;
    }

    /* Compute the surface placement parameters. */
    _ComputeSurfacePlacement(Surface);

    if (userStride != Surface->info.stride)
    {
        /*
         * Still not equal, which means user stride is not pixel aligned, ie,
         * userStride != alignedWidth(user) * bytesPerPixel
         */
        Surface->info.stride = userStride;

        /* Re-calculate slice size. */
        Surface->info.sliceSize = userStride * (Surface->info.alignedH / formatInfo->blockHeight);
    }

    /* Restore alignedWidth. */
    Surface->info.alignedW = halStride / bytesPerPixel;
    }

    offsetX = X * Surface->info.bitsPerPixel / 8;
    offsetY = Y * Surface->info.stride;

    /* Compute the surface sliceSize and size. */
    Surface->info.sliceSize = (Surface->info.alignedW / Surface->info.formatInfo.blockWidth)
                            * (Surface->info.alignedH / Surface->info.formatInfo.blockHeight)
                            * Surface->info.formatInfo.blockSize / 8;

    Surface->info.layerSize = Surface->info.sliceSize
                            * Surface->info.requestD;

    /* Always single layer for user surface */
    gcmASSERT(Surface->info.formatInfo.layers == 1);

    Surface->info.size      = Surface->info.layerSize
                            * Surface->info.formatInfo.layers;

    /* Set user pool node size. */
    Surface->info.node.size = Surface->info.size;

    /* Need to map logical pointer? */
    if (Surface->logical == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }
    else
    {
        Surface->info.node.logical = (gctUINT8_PTR) Surface->logical + offsetY;
    }

    if (offsetX || offsetY)
    {
        gcmASSERT(0);
    }

    desc.physical = Surface->physical;
    desc.logical = gcmPTR_TO_UINT64(Surface->info.node.logical);
    desc.size = Surface->info.size;
    desc.flag = gcvALLOC_FLAG_USERMEMORY;

    gcmONERROR(gcoHAL_WrapUserMemory(
        &desc,
        &Surface->info.node.u.normal.node
        ));

    Surface->info.node.u.wrapped.physical = Surface->physical;
    Surface->info.node.logical = Surface->logical;
    Surface->info.pfGetAddr = gcoHARDWARE_GetProcCalcPixelAddr(gcvNULL, &Surface->info);

#if (gcdENABLE_3D==0) && (gcdENABLE_2D==0) && (gcdENABLE_VG==1)
    Surface->info.node.u.wrapped.mappingInfo = gcvNULL;
#endif

    Surface->info.node.lockCount = 1;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}


/*******************************************************************************
**
**  gcoSURF_SetAlignment
**
**  Set the alignment width/height of the surface and calculate stride/size.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to gcoSURF object.
**
**      gctINT Width, Height
**          Size of the surface in pixels.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_SetAlignment(
    IN gcoSURF Surface,
    IN gctUINT Width,
    IN gctUINT Height
    )
{
    gcmHEADER_ARG("Surface=0x%x Width=%u Height=%u", Surface, Width, Height);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    Surface->info.alignedW = Width;
    Surface->info.alignedH = Height;

    /* Compute the surface stride. */
    Surface->info.stride = Surface->info.alignedW
                           * Surface->info.bitsPerPixel / 8;

    /* Compute the surface size. */
    Surface->info.size
        = Surface->info.stride
        * Surface->info.alignedH;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoSURF_ReferenceSurface
**
**  Increase reference count of a surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to gcoSURF object.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_ReferenceSurface(
    IN gcoSURF Surface
    )
{
    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    Surface->referenceCount++;

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}


/*******************************************************************************
**
**  gcoSURF_QueryReferenceCount
**
**  Query reference count of a surface
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to gcoSURF object.
**
**  OUTPUT:
**
**      gctINT32 ReferenceCount
**          Reference count of a surface
*/
gceSTATUS
gcoSURF_QueryReferenceCount(
    IN gcoSURF Surface,
    OUT gctINT32 * ReferenceCount
    )
{
    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    *ReferenceCount = Surface->referenceCount;

    gcmFOOTER_ARG("*ReferenceCount=%d", *ReferenceCount);
    return gcvSTATUS_OK;
}

#if gcdENABLE_3D
gceSTATUS
gcoSURF_IsRenderable(
    IN gcoSURF Surface
    )
{
    gceSTATUS   status;

    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Check whether the surface is renderable. */
    status = gcoHARDWARE_QuerySurfaceRenderable(gcvNULL, &Surface->info);

    /* Return status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoSURF_IsFormatRenderableAsRT(
    IN gcoSURF Surface
    )
{
    gceSTATUS               status;
    gceSURF_FORMAT          format;

    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Check whether the surface format is renderable when
       Tex bind to fbo. */
    format = Surface->info.format;

    status = (format >= 700) ? gcvSTATUS_FALSE
           : gcvSTATUS_TRUE;

    /* Return status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoSURF_Swap(
    IN gcoSURF Surface1,
    IN gcoSURF Surface2
    )
{
    struct _gcoSURF temp;

    gcmHEADER_ARG("Surface1=%p Surface2=%p", Surface1, Surface2);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface1, gcvOBJ_SURF);
    gcmVERIFY_OBJECT(Surface2, gcvOBJ_SURF);

    /* Swap the surfaces. */
    temp      = *Surface1;
    *Surface1 = *Surface2;
    *Surface2 = temp;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

#define gcdCOLOR_SPACE_CONVERSION_NONE         0
#define gcdCOLOR_SPACE_CONVERSION_TO_LINEAR    1
#define gcdCOLOR_SPACE_CONVERSION_TO_NONLINEAR 2

static
gceSTATUS
_AveragePixels(
    gcsPIXEL *pixels,
    gctINT pixelCount,
    gceFORMAT_DATATYPE inputFormat,
    gcsPIXEL *outPixel
    )
{
    gctINT i;
    gcsPIXEL mergePixel;

    gcoOS_ZeroMemory(&mergePixel, sizeof(gcsPIXEL));

    switch(inputFormat)
    {
    case gcvFORMAT_DATATYPE_UNSIGNED_INTEGER:
        for (i = 0; i < pixelCount; i++)
        {
            mergePixel.color.ui.r += pixels[i].color.ui.r;
            mergePixel.color.ui.g += pixels[i].color.ui.g;
            mergePixel.color.ui.b += pixels[i].color.ui.b;
            mergePixel.color.ui.a += pixels[i].color.ui.a;
            mergePixel.d += pixels[i].d;
            mergePixel.s += pixels[i].s;
        }

        mergePixel.color.ui.r /= pixelCount;
        mergePixel.color.ui.g /= pixelCount;
        mergePixel.color.ui.b /= pixelCount;
        mergePixel.color.ui.a /= pixelCount;
        mergePixel.d /= pixelCount;
        mergePixel.s /= pixelCount;
        break;

    case gcvFORMAT_DATATYPE_SIGNED_INTEGER:
        for (i = 0; i < pixelCount; i++)
        {
            mergePixel.color.i.r += pixels[i].color.i.r;
            mergePixel.color.i.g += pixels[i].color.i.g;
            mergePixel.color.i.b += pixels[i].color.i.b;
            mergePixel.color.i.a += pixels[i].color.i.a;
            mergePixel.d += pixels[i].d;
            mergePixel.s += pixels[i].s;
        }

        mergePixel.color.i.r /= pixelCount;
        mergePixel.color.i.g /= pixelCount;
        mergePixel.color.i.b /= pixelCount;
        mergePixel.color.i.a /= pixelCount;
        mergePixel.d /= pixelCount;
        mergePixel.s /= pixelCount;

        break;
    default:
        for (i = 0; i < pixelCount; i++)
        {
            mergePixel.color.f.r += pixels[i].color.f.r;
            mergePixel.color.f.g += pixels[i].color.f.g;
            mergePixel.color.f.b += pixels[i].color.f.b;
            mergePixel.color.f.a += pixels[i].color.f.a;
            mergePixel.d += pixels[i].d;
            mergePixel.s += pixels[i].s;
        }

        mergePixel.color.f.r /= pixelCount;
        mergePixel.color.f.g /= pixelCount;
        mergePixel.color.f.b /= pixelCount;
        mergePixel.color.f.a /= pixelCount;
        mergePixel.d /= pixelCount;
        mergePixel.s /= pixelCount;
        break;
    }

    *outPixel = mergePixel;

    return gcvSTATUS_OK;
}

gceSTATUS
gcoSURF_BlitCPU(
    IN gcsSURF_BLIT_ARGS* args
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcoSURF srcSurf, dstSurf;
    gctPOINTER srcAddr[3] = {gcvNULL};
    gctPOINTER dstAddr[3] = {gcvNULL};
    _PFNreadPixel pfReadPixel = gcvNULL;
    _PFNwritePixel pfWritePixel = gcvNULL;
    gctFLOAT xScale, yScale, zScale;
    gctINT iDst, jDst, kDst;
    gctINT colorSpaceConvert = gcdCOLOR_SPACE_CONVERSION_NONE;
    gcsSURF_BLIT_ARGS blitArgs;
    gctINT scissorHeight = 0;
    gctINT scissorWidth = 0;
    gctBOOL averagePixels = gcvTRUE;
    gcsSURF_FORMAT_INFO *srcFmtInfo, *dstFmtInfo;

    gcoOS_MemCopy(&blitArgs, args, sizeof(gcsSURF_BLIT_ARGS));

    srcSurf = args->srcSurface;
    dstSurf = args->dstSurface;

    /* MSAA surface should multiple samples.*/
    blitArgs.srcWidth  *= srcSurf->info.sampleInfo.x;
    blitArgs.srcHeight *= srcSurf->info.sampleInfo.y;
    blitArgs.srcX      *= srcSurf->info.sampleInfo.x;
    blitArgs.srcY      *= srcSurf->info.sampleInfo.y;

    blitArgs.dstWidth  *= dstSurf->info.sampleInfo.x;
    blitArgs.dstHeight *= dstSurf->info.sampleInfo.y;
    blitArgs.dstX      *= dstSurf->info.sampleInfo.x;
    blitArgs.dstY      *= dstSurf->info.sampleInfo.y;

    if (blitArgs.scissorTest)
    {
        scissorHeight = blitArgs.scissor.bottom - blitArgs.scissor.top;
        scissorWidth  = blitArgs.scissor.right - blitArgs.scissor.left;
        blitArgs.scissor.top    *= dstSurf->info.sampleInfo.y;
        blitArgs.scissor.left   *= dstSurf->info.sampleInfo.x;
        blitArgs.scissor.bottom  = scissorHeight * dstSurf->info.sampleInfo.y + blitArgs.scissor.top;
        blitArgs.scissor.right   = scissorWidth * dstSurf->info.sampleInfo.x + blitArgs.scissor.left;
    }

    if (!args || !args->srcSurface || !args->dstSurface)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if (args->srcSurface == args->dstSurface)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    /* If either the src or dst rect are negative */
    if (0 > blitArgs.srcWidth || 0 > blitArgs.srcHeight || 0 > blitArgs.srcDepth ||
        0 > blitArgs.dstWidth || 0 > blitArgs.dstHeight || 0 > blitArgs.dstDepth)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    /* If either the src or dst rect are zero, skip the blit */
    if (0 == blitArgs.srcWidth || 0 == blitArgs.srcHeight || 0 == blitArgs.srcDepth ||
        0 == blitArgs.dstWidth || 0 == blitArgs.dstHeight || 0 == blitArgs.dstDepth)
    {
        return gcvSTATUS_OK;
    }

    srcFmtInfo = &srcSurf->info.formatInfo;
    dstFmtInfo = &dstSurf->info.formatInfo;

    /* If either the src or dst rect has no intersection with the surface, skip the blit. */
    if (blitArgs.srcX + blitArgs.srcWidth  <= 0 || blitArgs.srcX >= (gctINT)(srcSurf->info.alignedW) ||
        blitArgs.srcY + blitArgs.srcHeight <= 0 || blitArgs.srcY >= (gctINT)(srcSurf->info.alignedH) ||
        blitArgs.srcZ + blitArgs.srcDepth  <= 0 || blitArgs.srcZ >= (gctINT)(srcSurf->info.requestD) ||
        blitArgs.dstX + blitArgs.dstWidth  <= 0 || blitArgs.dstX >= (gctINT)(dstSurf->info.alignedW) ||
        blitArgs.dstY + blitArgs.dstHeight <= 0 || blitArgs.dstY >= (gctINT)(dstSurf->info.alignedH) ||
        blitArgs.dstZ + blitArgs.dstDepth  <= 0 || blitArgs.dstZ >= (gctINT)(dstSurf->info.requestD))
    {
        return gcvSTATUS_OK;
    }

    /* Propagate canDropStencil flag to the destination surface */
    dstSurf->info.canDropStencilPlane = srcSurf->info.canDropStencilPlane;

    if (dstSurf->info.hzNode.valid)
    {
        /* Disable any HZ attached to destination. */
        dstSurf->info.hzDisabled = gcvTRUE;
    }

    /*
    ** For integer format upload/blit, the data type must be totally matched.
    ** And we should not do conversion to float per spec, or precision will be lost.
    */
    if (((srcFmtInfo->fmtDataType == gcvFORMAT_DATATYPE_UNSIGNED_INTEGER) ||
         (srcFmtInfo->fmtDataType == gcvFORMAT_DATATYPE_SIGNED_INTEGER)) &&
         (srcFmtInfo->fmtDataType != dstFmtInfo->fmtDataType))
    {
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* For (sign)integer format, just pick one of them.
    */
    if ((srcFmtInfo->fmtDataType == gcvFORMAT_DATATYPE_UNSIGNED_INTEGER) ||
        (srcFmtInfo->fmtDataType == gcvFORMAT_DATATYPE_SIGNED_INTEGER))
    {
        averagePixels = gcvFALSE;
    }


    /* TODO: those function pointers can be recorded in gcoSURF */
    pfReadPixel  = gcoSURF_GetReadPixelFunc(srcSurf);
    pfWritePixel = gcoSURF_GetWritePixelFunc(dstSurf);

    /* set color space conversion flag */
    if (srcSurf->info.colorSpace != dstSurf->info.colorSpace)
    {
        gcmASSERT(dstSurf->info.colorSpace != gcvSURF_COLOR_SPACE_UNKNOWN);

        if (srcSurf->info.colorSpace == gcvSURF_COLOR_SPACE_LINEAR)
        {
            colorSpaceConvert = gcdCOLOR_SPACE_CONVERSION_TO_NONLINEAR;
        }
        else if (srcSurf->info.colorSpace == gcvSURF_COLOR_SPACE_NONLINEAR)
        {
            colorSpaceConvert = gcdCOLOR_SPACE_CONVERSION_TO_LINEAR;
        }
        else
        {
            /* color space should NOT be gcvSURF_COLOR_SPACE_UNKNOWN */
            gcmASSERT(0);
        }
    }

    if (!pfReadPixel || !pfWritePixel)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    /* Flush the GPU cache */
    gcmONERROR(gcoHARDWARE_FlushTileStatus(gcvNULL, &srcSurf->info, gcvTRUE));
    gcmONERROR(gcoHARDWARE_DisableTileStatus(gcvNULL, &dstSurf->info, gcvTRUE));

    /* Synchronize with the GPU. */
    /* TODO: if both of the surfaces previously were not write by GPU,
    ** or already did the sync, no need to do it again.
    */
    gcmONERROR(gcoHARDWARE_FlushPipe(gcvNULL, gcvNULL));
    gcmONERROR(gcoHARDWARE_Commit(gcvNULL));
    gcmONERROR(gcoHARDWARE_Stall(gcvNULL));

    /* Lock the surfaces. */
    gcmONERROR(gcoSURF_Lock(srcSurf, gcvNULL, srcAddr));
    gcmONERROR(gcoSURF_Lock(dstSurf, gcvNULL, dstAddr));

    /* Src surface might be written by GPU previously, CPU need to invalidate
    ** its cache before reading.
    ** Dst surface alo need invalidate CPU cache to guarantee CPU cache is coherent
    ** with memory, so it's correct to flush out after writing.
    */
    gcmONERROR(gcoSURF_NODE_Cache(&srcSurf->info.node,
                                  srcAddr[0],
                                  srcSurf->info.size,
                                  gcvCACHE_INVALIDATE));
    gcmONERROR(gcoSURF_NODE_Cache(&dstSurf->info.node,
                                  dstAddr[0],
                                  dstSurf->info.size,
                                  gcvCACHE_INVALIDATE));

    xScale = blitArgs.srcWidth  / (gctFLOAT)blitArgs.dstWidth;
    yScale = blitArgs.srcHeight / (gctFLOAT)blitArgs.dstHeight;
    zScale = blitArgs.srcDepth  / (gctFLOAT)blitArgs.dstDepth;

    for (kDst = blitArgs.dstZ; kDst < blitArgs.dstZ + blitArgs.dstDepth; ++kDst)
    {
        gctINT kSrc;
        gctINT paceI, paceJ, paceK;
        gctINT paceImax, paceJmax, paceKmax;

        if (kDst < 0 || kDst >= (gctINT)dstSurf->info.requestD)
        {
            continue;
        }

        kSrc = blitArgs.srcZ + (gctINT)((kDst - blitArgs.dstZ) * zScale);

        if (kSrc < 0 || kSrc >= (gctINT)srcSurf->info.requestD)
        {
            continue;
        }

        for (jDst = blitArgs.dstY; jDst < blitArgs.dstY + blitArgs.dstHeight; ++jDst)
        {
            gctINT jSrc;

            if (jDst < 0 || jDst >= (gctINT)dstSurf->info.alignedH)
            {
                continue;
            }

            /* scissor test */
            if (blitArgs.scissorTest &&
                (jDst < blitArgs.scissor.top || jDst >= blitArgs.scissor.bottom))
            {
                continue;
            }

            jSrc = (gctINT)((jDst - blitArgs.dstY) * yScale);

            if (jSrc > blitArgs.srcHeight - 1)
            {
                jSrc = blitArgs.srcHeight - 1;
            }

            if (blitArgs.yReverse)
            {
                jSrc = blitArgs.srcHeight - 1 - jSrc;
            }

            jSrc += blitArgs.srcY;

            if (jSrc < 0 || jSrc >= (gctINT)srcSurf->info.alignedH)
            {
                continue;
            }

            for (iDst = blitArgs.dstX; iDst < blitArgs.dstX + blitArgs.dstWidth; ++iDst)
            {
                gcsPIXEL internal;
                gcsPIXEL samplePixels[32];
                gctUINT sampleCount = 0;
                gctPOINTER srcAddr_l[gcdMAX_SURF_LAYERS] = {gcvNULL};
                gctPOINTER dstAddr_l[gcdMAX_SURF_LAYERS] = {gcvNULL};
                gctINT iSrc;

                if (iDst < 0 || iDst >= (gctINT)dstSurf->info.alignedW)
                {
                    continue;
                }

                /* scissor test */
                if (blitArgs.scissorTest &&
                    (iDst < blitArgs.scissor.left || iDst >= blitArgs.scissor.right))
                {
                    continue;
                }

                iSrc = (gctINT)((iDst - blitArgs.dstX) * xScale);

                if (iSrc > blitArgs.srcWidth - 1)
                {
                    iSrc = blitArgs.srcWidth - 1;
                }

                if (blitArgs.xReverse)
                {
                    iSrc = blitArgs.srcWidth - 1 - iSrc;
                }

                iSrc += blitArgs.srcX;

                if (iSrc < 0 || iSrc >= (gctINT)srcSurf->info.alignedW)
                {
                    continue;
                }

                paceImax = (gctINT)(xScale + 0.5f) > 1 ? (gctINT)(xScale + 0.5f) : 1;
                paceJmax = (gctINT)(yScale + 0.5f) > 1 ? (gctINT)(yScale + 0.5f) : 1;
                paceKmax = (gctINT)(zScale + 0.5f) > 1 ? (gctINT)(zScale + 0.5f) : 1;

                for (paceK = 0; paceK < paceKmax; paceK++)
                {
                    for (paceJ = 0; paceJ < paceJmax; paceJ++)
                    {
                        for (paceI = 0; paceI < paceImax; paceI++)
                        {
                            gctINT sampleI = iSrc + paceI * (blitArgs.xReverse ? -1 : 1);
                            gctINT sampleJ = jSrc + paceJ * (blitArgs.yReverse ? -1 : 1);
                            gctINT sampleK = kSrc + paceK;

                            sampleI = gcmCLAMP(sampleI, 0, (gctINT)(srcSurf->info.alignedW - 1));
                            sampleJ = gcmCLAMP(sampleJ, 0, (gctINT)(srcSurf->info.alignedH - 1));
                            sampleK = gcmCLAMP(sampleK, 0, (gctINT)(srcSurf->info.requestD - 1));

                            srcSurf->info.pfGetAddr(&srcSurf->info, (gctSIZE_T)sampleI, (gctSIZE_T)sampleJ, (gctSIZE_T)sampleK, srcAddr_l);

                            pfReadPixel(srcAddr_l, &samplePixels[sampleCount]);

                            if (colorSpaceConvert == gcdCOLOR_SPACE_CONVERSION_TO_LINEAR)
                            {
                                gcoSURF_PixelToLinear(&samplePixels[sampleCount]);
                            }
                            else if (colorSpaceConvert == gcdCOLOR_SPACE_CONVERSION_TO_NONLINEAR)
                            {
                                gcoSURF_PixelToNonLinear(&samplePixels[sampleCount]);

                            }

                            sampleCount++;
                        }
                    }
                }

                if ((sampleCount > 1) && averagePixels)
                {
                    _AveragePixels(samplePixels, sampleCount, srcFmtInfo->fmtDataType, &internal);
                }
                else
                {
                    internal = samplePixels[0];
                }

                dstSurf->info.pfGetAddr(&dstSurf->info, (gctSIZE_T)iDst, (gctSIZE_T)jDst, (gctSIZE_T)kDst, dstAddr_l);

                pfWritePixel(&internal, dstAddr_l, args->flags);

                /* TODO: If they are of same type, we can skip some conversion */
            }
        }
    }

    /* Dst surface was written by CPU and might be accessed by GPU later */
    gcmONERROR(gcoSURF_NODE_Cache(&dstSurf->info.node,
                                  dstAddr[0],
                                  dstSurf->info.size,
                                  gcvCACHE_CLEAN));

    if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_PE_DITHER_FIX) == gcvFALSE &&
        args->flags == 0 && /* Full mask overwritten */
        args->dstX == 0 && args->dstWidth  >= (gctINT)dstSurf->info.requestW &&
        args->dstY == 0 && args->dstHeight >= (gctINT)dstSurf->info.requestH)
    {
        dstSurf->info.deferDither3D = gcvFALSE;
    }

    if (dstSurf->info.paddingFormat &&
        args->dstX == 0 && args->dstWidth  >= (gctINT)dstSurf->info.requestW &&
        args->dstY == 0 && args->dstHeight >= (gctINT)dstSurf->info.requestH)
    {
        dstSurf->info.garbagePadded = gcvFALSE;
    }

#if gcdDUMP

    if (gcvSURF_BITMAP != srcSurf->info.type)
    {
        gcmDUMP(gcvNULL, "#verify BlitCPU source");
        /* verify the source */
        gcmDUMP_BUFFER(gcvNULL,
                       "verify",
                       gcsSURF_NODE_GetHWAddress(&srcSurf->info.node),
                       srcSurf->info.node.logical,
                       0,
                       srcSurf->info.size);

    }
    /* upload the destination */
    gcmDUMP_BUFFER(gcvNULL,
                   "memory",
                   gcsSURF_NODE_GetHWAddress(&dstSurf->info.node),
                   dstSurf->info.node.logical,
                   0,
                   dstSurf->info.size);
#endif

OnError:
    /* Unlock the surfaces. */
    if (srcAddr[0])
    {
        gcoSURF_Unlock(srcSurf, srcAddr[0]);
    }
    if (dstAddr[0])
    {
        gcoSURF_Unlock(dstSurf, dstAddr[0]);
    }

    return status;
}


gceSTATUS
gcoSURF_DrawBlit_v2(
    gcsSURF_VIEW *SrcView,
    gcsSURF_VIEW *DstView,
    gscSURF_BLITDRAW_BLIT *Args
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("SrcView=0x%x DstView=0x%x Args=0x%x", SrcView, DstView, Args);

    status = gcoHARDWARE_DrawBlit_v2(SrcView, DstView, Args);

    gcmFOOTER_NO();
    return status;
}

gceSTATUS
gcoSURF_Preserve(
    IN gcoSURF Source,
    IN gcoSURF Dest,
    IN gcsRECT_PTR MaskRect
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsRECT rects[4];
    gctINT  count = 0;
    gctINT  width, height;
    gctUINT resolveAlignmentX = 0;
    gctUINT resolveAlignmentY = 0;
    gctUINT tileAlignmentX    = 0;
    gctUINT tileAlignmentY    = 0;
    gcsSURF_VIEW srcView = {Source, 0, 1};
    gcsSURF_VIEW dstView = {Dest  , 0, 1};

    gcmHEADER_ARG("Source=0x%x Dest=0x%x MaskRect=0x%x",
                  Source, Dest,
                  MaskRect);

    gcmASSERT(!(Dest->info.flags & gcvSURF_FLAG_CONTENT_UPDATED));

    /* Get surface size. */
    width  = Dest->info.requestW;
    height = Dest->info.requestH;

    if ((MaskRect != gcvNULL) &&
        (MaskRect->left   <= 0) &&
        (MaskRect->top    <= 0) &&
        (MaskRect->right  >= (gctINT) width) &&
        (MaskRect->bottom >= (gctINT) height))
    {
        gcmFOOTER_NO();
        /* Full screen clear. No copy. */
        return gcvSTATUS_OK;
    }

    if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_BLT_ENGINE))
    {
        /* Pixel alignment in 3DBlit engine. */
        tileAlignmentX    = tileAlignmentY    =
        resolveAlignmentX = resolveAlignmentY = 1;
    }
    else
    {
        /* Query surface resolve alignment parameters. */
        gcmONERROR(
            gcoHARDWARE_GetSurfaceResolveAlignment(gcvNULL,
                                                   &Dest->info,
                                                   &tileAlignmentX,
                                                   &tileAlignmentY,
                                                   &resolveAlignmentX,
                                                   &resolveAlignmentY));
    }

    if ((MaskRect == gcvNULL) ||
        (MaskRect->left == MaskRect->right) ||
        (MaskRect->top  == MaskRect->bottom))
    {
        /* Zarro clear rect, need copy full surface. */
        rects[0].left   = 0;
        rects[0].top    = 0;
        rects[0].right  = gcmALIGN(width,  resolveAlignmentX);
        rects[0].bottom = gcmALIGN(height, resolveAlignmentY);
        count = 1;
    }
    else
    {
        gcsRECT maskRect;

        if (Dest->info.flags & gcvSURF_FLAG_CONTENT_YINVERTED)
        {
            /* Y inverted content. */
            maskRect.left   = MaskRect->left;
            maskRect.top    = height - MaskRect->bottom;
            maskRect.right  = MaskRect->right;
            maskRect.bottom = height - MaskRect->top;
        }
        else
        {
            maskRect = *MaskRect;
        }

        /* Avoid right,bottom coordinate exceeding surface boundary. */
        if (tileAlignmentX < resolveAlignmentX)
        {
            tileAlignmentX = resolveAlignmentX;
        }

        if (tileAlignmentY < resolveAlignmentY)
        {
            tileAlignmentY = resolveAlignmentY;
        }

        /*
         *  +------------------------+
         *  |                        |
         *  |           r1           |
         *  |                        |
         *  |......+----------+......|
         *  |      |          |      |
         *  |  r0  |  mask    |  r2  |
         *  |      |   rect   |      |
         *  |......+----------+......|
         *  |                        |
         *  |           r3           |
         *  |                        |
         *  +------------------------+
         */

        /* Get real size of clear  */
        if (maskRect.left > 0)
        {
            rects[count].left   = 0;
            rects[count].top    = gcmALIGN_BASE(maskRect.top, tileAlignmentY);
            rects[count].right  = gcmALIGN(maskRect.left, resolveAlignmentX);
            rects[count].bottom = rects[count].top + gcmALIGN(maskRect.bottom - rects[count].top, resolveAlignmentY);
            count++;
        }

        if (maskRect.top > 0)
        {
            rects[count].left   = 0;
            rects[count].top    = 0;
            rects[count].right  = gcmALIGN(width, resolveAlignmentX);
            rects[count].bottom = gcmALIGN(maskRect.top, resolveAlignmentY);
            count++;
        }

        if (maskRect.right < width)
        {
            rects[count].left   = gcmALIGN_BASE(maskRect.right, tileAlignmentX);
            rects[count].top    = gcmALIGN_BASE(maskRect.top,   tileAlignmentY);
            rects[count].right  = rects[count].left + gcmALIGN(width - rects[count].left, resolveAlignmentX);
            rects[count].bottom = rects[count].top  + gcmALIGN(maskRect.bottom - rects[count].top, resolveAlignmentY);
            count++;
        }

        if (maskRect.bottom < height)
        {
            rects[count].left   = 0;
            rects[count].top    = gcmALIGN_BASE(maskRect.bottom, tileAlignmentY);
            rects[count].right  = gcmALIGN(width, resolveAlignmentX);
            rects[count].bottom = rects[count].top + gcmALIGN(height - rects[count].top, resolveAlignmentY);
            count++;
        }
    }

    /* Preserve calculated rects. */
    gcmONERROR(
        gcoHARDWARE_PreserveRects(gcvNULL,
                                  &srcView,
                                  &dstView,
                                  rects,
                                  count));

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER();
    return status;
}
#endif

gceSTATUS
gcoSURF_ResetSurWH(
    IN gcoSURF Surface,
    IN gctUINT oriw,
    IN gctUINT orih,
    IN gctUINT alignw,
    IN gctUINT alignh,
    IN gceSURF_FORMAT fmt
    )
{
    gceSTATUS status;
    Surface->info.requestW = oriw;
    Surface->info.requestH = orih;
    Surface->info.requestD = 1;
    Surface->info.allocedW = oriw;
    Surface->info.allocedH = orih;
    Surface->info.alignedW = alignw;
    Surface->info.alignedH = alignh;

    gcmONERROR(gcoHARDWARE_ConvertFormat(
                          fmt,
                          (gctUINT32_PTR)&Surface->info.bitsPerPixel,
                          gcvNULL));

    /* Compute surface placement parameters. */
    _ComputeSurfacePlacement(Surface);

    Surface->info.layerSize = Surface->info.sliceSize * Surface->info.requestD;

    gcmASSERT(Surface->info.formatInfo.layers == 1);

    Surface->info.size = Surface->info.layerSize * Surface->info.formatInfo.layers;

    /* Success. */
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    return status;

}

/*******************************************************************************
**
**  gcoSURF_UpdateTimeStamp
**
**  Increase timestamp of a surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to gcoSURF object.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_UpdateTimeStamp(
    IN gcoSURF Surface
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Surface=0x%X", Surface);
    gcmVERIFY_ARGUMENT(Surface != gcvNULL);

    /* Increase timestamp value. */
    Surface->info.timeStamp++;

    gcmFOOTER_NO();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_UpdateTimeStamp
**
**  Query timestamp of a surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to gcoSURF object.
**
**  OUTPUT:
**
**      gctUINT64 * TimeStamp
**          Pointer to hold the timestamp. Can not be null.
*/
gceSTATUS
gcoSURF_QueryTimeStamp(
    IN gcoSURF Surface,
    OUT gctUINT64 * TimeStamp
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Surface=0x%x", Surface);
    gcmVERIFY_ARGUMENT(Surface != gcvNULL);
    gcmVERIFY_ARGUMENT(TimeStamp != gcvNULL);

    /* Increase timestamp value. */
    *TimeStamp = Surface->info.timeStamp;

    gcmFOOTER_ARG("*TimeStamp=%lld", *TimeStamp);
    return status;
}

/*******************************************************************************
**
**  gcoSURF_AllocShBuffer
**
**  Allocate shared buffer (gctSHBUF) for this surface, so that its shared
**  states can be accessed across processes.
**
**  Shared buffer is freed when surface is destroyed.
**
**  Usage:
**  1. Process (A) constructed a surface (a) which is to be used by other
**     processes such as process (B).
**
**  2. Process (A) need alloc ShBuf (s) by gcoSURF_AllocShBuffer for surface
**     (a) if (a) need shared its states to other processes.
**
**  3. Process (B) need get surface node and other information of surface (a)
**     includes the ShBuf handle by some IPC method (such as android
**     Binder mechanism). So process (B) wrapps it as surface (b).
**
**  4. Process (B) need call gcoSURF_BindShBuffer on surface (b) with ShBuf (s)
**     to connect.
**
**  5. Processes can then call gcoSURF_PushSharedInfo/gcoSURF_PopSharedInfo to
**     shared states.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to gcoSURF object.
**
**  OUTPUT:
**
**      gctSHBUF * ShBuf
**          Pointer to hold shared buffer handle.
*/
gceSTATUS
gcoSURF_AllocShBuffer(
    IN gcoSURF Surface,
    OUT gctSHBUF * ShBuf
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x ShBuf=%d",
                  Surface, (gctUINT32) (gctUINTPTR_T) ShBuf);

    gcmVERIFY_ARGUMENT(Surface != gcvNULL);

    if (Surface->info.shBuf == gcvNULL)
    {
        /* Create ShBuf. */
        gcmONERROR(
            gcoHAL_CreateShBuffer(sizeof (gcsSURF_SHARED_INFO),
                                  &Surface->info.shBuf));
    }

    /* Returns shared buffer handle. */
    *ShBuf = Surface->info.shBuf;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_BindShBuffer
**
**  Bind surface to a shared buffer. The share buffer should be allocated by
**  gcoSURF_AllocShBuffer.
**
**  See gcoSURF_AllocShBuffer.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to gcoSURF object.
**
**  OUTPUT:
**
**      gctSHBUF ShBuf
**          Pointer shared buffer handle to connect to.
*/
gceSTATUS
gcoSURF_BindShBuffer(
    IN gcoSURF Surface,
    OUT gctSHBUF ShBuf
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x ShBuf=%d",
                  Surface, (gctUINT32) (gctUINTPTR_T) ShBuf);

    gcmVERIFY_ARGUMENT(Surface != gcvNULL);

    if (Surface->info.shBuf == gcvNULL)
    {
        /* Map/reference ShBuf. */
        gcmONERROR(gcoHAL_MapShBuffer(ShBuf));
        Surface->info.shBuf = ShBuf;
    }
    else
    {
        /* Already has a ShBuf. */
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_PushSharedInfo
**
**  Push surface shared states to shared buffer. Shared buffer should be
**  initialized before this function either by gcoSURF_AllocShBuffer or
**  gcoSURF_BindShBuffer. gcvSTATUS_NOT_SUPPORTED if not ever bound.
**
**  See gcoSURF_AllocShBuffer.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to gcoSURF object.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_PushSharedInfo(
    IN gcoSURF Surface
    )
{
    gceSTATUS status;
    gcsSURF_SHARED_INFO info;

    gcmHEADER_ARG("Surface=0x%x", Surface);

    gcmVERIFY_ARGUMENT(Surface != gcvNULL);

    if (Surface->info.shBuf == gcvNULL)
    {
        /* No shared buffer bound. */
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /* Gather states. */
    info.magic              = gcvSURF_SHARED_INFO_MAGIC;
    info.timeStamp          = Surface->info.timeStamp;

#if gcdENABLE_3D
    info.tileStatusDisabled = Surface->info.tileStatusDisabled;
    info.dirty              = Surface->info.dirty;
    info.fcValue            = Surface->info.fcValue;
    info.fcValueUpper       = Surface->info.fcValueUpper;
    info.compressed         = Surface->info.compressed;
#endif

    /* Put structure to shared buffer object. */
    gcmONERROR(
        gcoHAL_WriteShBuffer(Surface->info.shBuf,
                             &info,
                             sizeof (gcsSURF_SHARED_INFO)));

    /* Success. */
    gcmFOOTER_NO();
    return status;

OnError:
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_PopSharedInfo
**
**  Pop surface shared states from shared buffer. Shared buffer must be
**  initialized before this function either by gcoSURF_AllocShBuffer or
**  gcoSURF_BindShBuffer. gcvSTATUS_NOT_SUPPORTED if not ever bound.
**
**  Before sync shared states to this surface, timestamp is checked. If
**  timestamp is not newer than current, sync is discard and gcvSTATUS_SKIP
**  is returned.
**
**  See gcoSURF_AllocShBuffer.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to gcoSURF object.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_PopSharedInfo(
    IN gcoSURF Surface
    )
{
    gceSTATUS status;
    gcsSURF_SHARED_INFO info;
    gctUINT32 size = sizeof (gcsSURF_SHARED_INFO);
    gctUINT32 bytesRead = 0;

    gcmHEADER_ARG("Surface=0x%x", Surface);

    gcmVERIFY_ARGUMENT(Surface != gcvNULL);

    if (Surface->info.shBuf == gcvNULL)
    {
        /* No shared buffer bound. */
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /* Get structure from shared buffer object. */
    gcmONERROR(
        gcoHAL_ReadShBuffer(Surface->info.shBuf,
                            &info,
                            size,
                            &bytesRead));

    if (status == gcvSTATUS_SKIP)
    {
        /* No data in shared buffer. */
        goto OnError;
    }

    /* Check magic. */
    if ((info.magic != gcvSURF_SHARED_INFO_MAGIC) ||
        (bytesRead  != sizeof (gcsSURF_SHARED_INFO)))
    {
        /* Magic mismatch. */
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Check time stamp. */
    if (info.timeStamp <= Surface->info.timeStamp)
    {
        status = gcvSTATUS_SKIP;
        goto OnError;
    }

    /* Update surface states. */
    Surface->info.timeStamp          = info.timeStamp;

#if gcdENABLE_3D
    Surface->info.tileStatusDisabled = info.tileStatusDisabled;
    Surface->info.dirty              = info.dirty;
    Surface->info.fcValue            = info.fcValue;
    Surface->info.fcValueUpper       = info.fcValueUpper;
    Surface->info.compressed         = info.compressed;
#endif

    /* Success. */
    gcmFOOTER_NO();
    return status;

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcsSURF_NODE_Construct(
    IN gcsSURF_NODE_PTR Node,
    IN gctSIZE_T Bytes,
    IN gctUINT Alignment,
    IN gceSURF_TYPE Type,
    IN gctUINT32 Flag,
    IN gcePOOL Pool
    )
{
    gceSTATUS status;
    gcsHAL_INTERFACE iface = {0};
    struct _gcsHAL_ALLOCATE_LINEAR_VIDEO_MEMORY * alvm
        = (struct _gcsHAL_ALLOCATE_LINEAR_VIDEO_MEMORY *) &iface.u;
    gctUINT i;

#if gcdDEBUG_OPTION && gcdDEBUG_OPTION_SPECIFY_POOL
    static gcePOOL poolPerType[gcvSURF_NUM_TYPES] =
    {
        /* gcvSURF_TYPE_UNKNOWN. */
        gcvPOOL_DEFAULT,
        /* gcvSURF_INDEX */
        gcvPOOL_DEFAULT,
        /* gcvSURF_VERTEX */
        gcvPOOL_DEFAULT,
        /* gcvSURF_TEXTURE */
        gcvPOOL_DEFAULT,
        /* gcvSURF_RENDER_TARGET */
        gcvPOOL_DEFAULT,
        /* gcvSURF_DEPTH */
        gcvPOOL_DEFAULT,
        /* gcvSURF_BITMAP */
        gcvPOOL_DEFAULT,
        /* gcvSURF_TILE_STATUS */
        gcvPOOL_DEFAULT,
        /* gcvSURF_IMAGE */
        gcvPOOL_DEFAULT,
        /* gcvSURF_MASK */
        gcvPOOL_DEFAULT,
        /* gcvSURF_SCISSOR */
        gcvPOOL_DEFAULT,
        /* gcvSURF_HIERARCHICAL_DEPTH */
        gcvPOOL_DEFAULT,
    };
#endif

    gcmHEADER_ARG("Node=%p, Bytes=%llu, Alignement=%d, Type=%d, Flag=%d, Pool=%d",
                  Node, Bytes, Alignment, Type, Flag, Pool);

#ifdef LINUX
#ifndef ANDROID
#if gcdENABLE_3D
    gcePATCH_ID patchID = gcvPATCH_INVALID;
    gcoHAL_GetPatchID(gcvNULL, &patchID);

    if (gcdPROC_IS_WEBGL(patchID))
    {
        Flag |= gcvALLOC_FLAG_MEMLIMIT;
    }
#endif
#endif
#endif

    iface.command   = gcvHAL_ALLOCATE_LINEAR_VIDEO_MEMORY;

    gcmSAFECASTSIZET(alvm->bytes, Bytes);

    alvm->alignment = Alignment;
    alvm->type      = Type;
    alvm->pool      = Pool;
    alvm->flag      = Flag;

#if gcdDEBUG_OPTION && gcdDEBUG_OPTION_SPECIFY_POOL
    if (Pool == gcvPOOL_DEFAULT)
    {
        /* If pool is not specified, tune it to debug value now. */
        alvm->pool  = poolPerType[Type & 0xF];
    }
#endif

    gcoOS_ZeroMemory(Node, gcmSIZEOF(gcsSURF_NODE));

    gcmONERROR(gcoHAL_Call(gcvNULL, &iface));

    Node->u.normal.node = alvm->node;
    Node->pool          = alvm->pool;
    Node->size          = alvm->bytes;

    Node->physical2     = ~0U;
    Node->physical3     = ~0U;

    for (i = 0; i < gcvHARDWARE_NUM_TYPES; i++)
    {
        Node->hardwareAddresses[i] = ~0U;
    }

#if gcdGC355_MEM_PRINT
#ifdef LINUX
    gcoOS_AddRecordAllocation((gctINT32)Node->size);
#endif
#endif

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcsSURF_NODE_Destroy(
    IN gcsSURF_NODE_PTR Node
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("Node=0x%x", Node);
#if gcdSYNC
    {
        gcsSYNC_CONTEXT_PTR ptr = Node->fenceCtx;

        while(ptr)
        {
            Node->fenceCtx = ptr->next;

            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL,ptr));

            ptr = Node->fenceCtx;
        }
    }
#endif

    status = gcoHARDWARE_ScheduleVideoMemory(Node);

    /* Reset the node. */
    Node->pool  = gcvPOOL_UNKNOWN;
    Node->valid = gcvFALSE;

    gcmFOOTER();
    return status;
}

#if gcdENABLE_3D
gceSTATUS
gcsSURF_NODE_GetFence(
    IN gcsSURF_NODE_PTR Node,
    IN gceFENCE_TYPE Type
)
{
#if gcdSYNC
    if (!gcoHAL_GetOption(gcvNULL, gcvOPTION_KERNEL_FENCE))
    {
         if (Node)
         {
            gctBOOL fenceEnable;
            gcoHARDWARE_GetFenceEnabled(gcvNULL, &fenceEnable);
            if(fenceEnable)
            {
                gcoHARDWARE_GetFence(gcvNULL, &Node->fenceCtx, Type);
                Node->fenceStatus = gcvFENCE_ENABLE;
            }
            else
            {
                Node->fenceStatus = gcvFENCE_GET;
            }
         }
    }
#endif
    return gcvSTATUS_OK;
}

gceSTATUS
gcsSURF_NODE_WaitFence(
    IN gcsSURF_NODE_PTR Node,
    IN gceFENCE_TYPE Type
)
{
#if gcdSYNC
    if (gcoHAL_GetOption(gcvNULL, gcvOPTION_KERNEL_FENCE))
    {
        gcoHAL_WaitFence(Node->u.normal.node, gcvINFINITE);
    }
    else if (Node)
    {
        gctBOOL fenceEnable;
        gcoHARDWARE_GetFenceEnabled(gcvNULL, &fenceEnable);
        if(fenceEnable)
        {
            gcoHARDWARE_WaitFence(gcvNULL, Node->fenceCtx, Type);
        }
        else
        {
            if(Node->fenceStatus == gcvFENCE_GET)
            {
                Node->fenceStatus = gcvFENCE_ENABLE;
                gcoHARDWARE_SetFenceEnabled(gcvNULL, gcvTRUE);
                gcoHAL_Commit(gcvNULL, gcvTRUE);
            }
        }

    }
#endif
    return gcvSTATUS_OK;
}

gceSTATUS
gcsSURF_NODE_IsFenceEnabled(
    IN gcsSURF_NODE_PTR Node
    )
{
#if gcdSYNC
    gceSTATUS status = gcvSTATUS_FALSE;

    if (gcoHAL_GetOption(gcvNULL, gcvOPTION_KERNEL_FENCE))
    {
        return gcvSTATUS_TRUE;
    }
    else if (Node)
    {
        return (Node->fenceStatus != gcvFENCE_DISABLE) ?
                gcvSTATUS_TRUE : gcvSTATUS_FALSE;
    }
    return status;

#else
    return gcvSTATUS_FALSE;
#endif
}

gceSTATUS
gcsSURF_NODE_SetSharedLock(
    IN gcsSURF_NODE_PTR Node,
    IN gctPOINTER SharedLock
    )
{
#if gcdSYNC
    if(Node != gcvNULL)
    {
        Node->sharedLock = SharedLock;
    }
#endif
    return gcvSTATUS_OK;
}
#endif

gceSTATUS
gcsSURF_NODE_SetHardwareAddress(
    IN gcsSURF_NODE_PTR Node,
    IN gctUINT32 Address
    )
{
    gceHARDWARE_TYPE type;

    gcmVERIFY_OK(gcoHAL_GetHardwareType(gcvNULL, &type));

    gcmASSERT(type != gcvHARDWARE_INVALID);

    Node->hardwareAddresses[type] = Address;

    return gcvSTATUS_OK;
}

gceSTATUS
gcsSURF_NODE_GetHardwareAddress(
    IN gcsSURF_NODE_PTR Node,
    OUT gctUINT32_PTR Physical,
    OUT gctUINT32_PTR Physical2,
    OUT gctUINT32_PTR Physical3,
    OUT gctUINT32_PTR PhysicalBottom
    )
{
    gceHARDWARE_TYPE type;

    gcmVERIFY_OK(gcoHAL_GetHardwareType(gcvNULL, &type));

    gcmASSERT(type != gcvHARDWARE_INVALID);

    if (Physical != gcvNULL)
    {
        *Physical = Node->hardwareAddresses[type];
    }

    if (PhysicalBottom != gcvNULL)
    {
        *PhysicalBottom = Node->hardwareAddressesBottom[type];
    }

    return gcvSTATUS_OK;
}

#if gcdDUMP || gcdDUMP_COMMAND
gctUINT32
gcsSURF_NODE_GetHWAddress(
    IN gcsSURF_NODE_PTR Node
    )
{
    gceHARDWARE_TYPE type;

    gcmVERIFY_OK(gcoHAL_GetHardwareType(gcvNULL, &type));

    gcmASSERT(type != gcvHARDWARE_INVALID);

    return Node->hardwareAddresses[type];
}
#endif

gceSTATUS
gcoSURF_WrapUserMemory(
    IN gcoHAL Hal,
    IN gctUINT Width,
    IN gctUINT Height,
    IN gctUINT Stride,
    IN gctUINT Depth,
    IN gceSURF_TYPE Type,
    IN gceSURF_FORMAT Format,
    IN gctUINT32 Handle,
    IN gctUINT32 Flag,
    OUT gcoSURF * Surface
    )
{
    gceSTATUS status;
    gcoSURF surface = gcvNULL;
    gctUINT32 node;
    gceSURF_TYPE type;
    gctUINT bytesPerPixel;
    gctUINT halStride;
    gcsSURF_FORMAT_INFO_PTR formatInfo;
    gcsUSER_MEMORY_DESC desc;

    gcmHEADER_ARG("Handle=%d, Flag=%d", Handle, Flag);

    /* Create a no video memory node surface. */
    type  = Type | gcvSURF_NO_VIDMEM;

    gcmONERROR(gcoSURF_Construct(
        gcvNULL, Width, Height, Depth, type, Format, gcvPOOL_VIRTUAL, &surface));

    /* Get shortcut. */
    formatInfo = &surface->info.formatInfo;

    /* bytes per pixel of first plane. */
    switch (surface->info.format)
    {
    case gcvSURF_YV12:
    case gcvSURF_I420:
    case gcvSURF_NV12:
    case gcvSURF_NV21:
    case gcvSURF_NV16:
    case gcvSURF_NV61:
        bytesPerPixel = 1;
        halStride = surface->info.alignedW;
        break;

    default:
        bytesPerPixel = surface->info.bitsPerPixel / 8;
        halStride = (surface->info.alignedW / formatInfo->blockWidth)
                  * (formatInfo->blockSize / formatInfo->layers) / 8;
        break;
    }

    /* Apply user specified stride. */
    if (Stride != halStride)
    {
        if ((surface->info.type != gcvSURF_BITMAP)
        ||  (surface->info.stride < Width * bytesPerPixel)
        ||  (surface->info.stride & (4 * bytesPerPixel - 1))
        )
        {
            /*
             * 1. For Vivante internal surfaces types, user buffer placement
             * must be the same as what defined in HAL, otherwise the user
             * buffer is not compatible with HAL.
             * 2. For bitmap surface, user buffer stride may be larger than
             * least stride defined in HAL, and should be aligned.
             */
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        /* Calculte alignedWidth from user stride. */
        surface->info.alignedW = Stride / bytesPerPixel;
    }

    /* Compute the surface placement parameters. */
    _ComputeSurfacePlacement(surface);

    if (Stride != surface->info.stride)
    {
        /*
         * Still not equal, which means user stride is not pixel aligned, ie,
         * Stride != alignedWidth(user) * bytesPerPixel
         */
        surface->info.stride = Stride;

        /* Re-calculate slice size. */
        surface->info.sliceSize = Stride * (surface->info.alignedH / formatInfo->blockHeight);
    }

    /* Restore alignedWidth. */
    surface->info.alignedW = halStride / bytesPerPixel;

    surface->info.layerSize = surface->info.sliceSize
                            * surface->info.requestD;

    /* Always single layer for user surface */
    gcmASSERT(surface->info.formatInfo.layers == 1);

    surface->info.size      = surface->info.layerSize
                            * surface->info.formatInfo.layers;

    desc.flag = Flag;
    desc.handle = Handle;

    /* Wrap user memory to a video memory node. */
    gcmONERROR(gcoHAL_WrapUserMemory(&desc, &node));

    /* Import wrapped video memory node to the surface. */
    surface->info.node.u.normal.node = node;
    surface->info.node.pool          = gcvPOOL_VIRTUAL;
    surface->info.node.size          = surface->info.size;

    /* Get a normal surface. */
    *Surface = surface;

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    if (surface)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(surface));
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
gcoSURF_WrapUserMultiBuffer(
    IN gcoHAL Hal,
    IN gctUINT Width,
    IN gctUINT Height,
    IN gceSURF_TYPE Type,
    IN gceSURF_FORMAT Format,
    IN gctUINT Stride[3],
    IN gctUINT32 Handle[3],
    IN gctUINT BufferOffset[3],
    IN gctUINT32 Flag,
    OUT gcoSURF * Surface
    )
{
    gctUINT i;
    gceSTATUS status;
    gcoSURF surface = gcvNULL;
    gctUINT32 node[3] = {0, 0, 0};
    gctUINT nodeCount;
    gctUINT bitsPerPixel[3];

    gcmHEADER_ARG("Handle=%d, Flag=%d", Handle, Flag);

    if (Flag != gcvALLOC_FLAG_DMABUF)
    {
        /* Only for DMABUF for now. */
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /* Create a no video memory node surface. */
    Type = (gceSURF_TYPE) (Type | gcvSURF_NO_VIDMEM);

    if (Width > 0x2000 || Height > 0x2000)
    {
        /* Size should not be too large, use 8K x 8K as threshold. */
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmONERROR(gcoSURF_Construct(
        gcvNULL, Width, Height, 1, Type, Format, gcvPOOL_VIRTUAL, &surface));

    surface->info.flags |= gcvSURF_FLAG_MULTI_NODE;
    surface->info.size   = 0;

    /* bytes per pixel of first plane. */
    switch (surface->info.format)
    {
    case gcvSURF_YV12:
    case gcvSURF_I420:
        bitsPerPixel[0] = 8;
        bitsPerPixel[1] = 4;
        bitsPerPixel[2] = 4;
        nodeCount = 3;
        break;

    case gcvSURF_NV12:
    case gcvSURF_NV21:
        bitsPerPixel[0] = 8;
        bitsPerPixel[1] = 8;
        nodeCount = 2;
        break;

    case gcvSURF_NV16:
    case gcvSURF_NV61:
        bitsPerPixel[0] = 8;
        bitsPerPixel[1] = 8;
        nodeCount = 2;
        break;

    default:
        nodeCount = 1;
        bitsPerPixel[0] = surface->info.bitsPerPixel;
        break;
    }

    /* Check parameters. */
    for (i = 0; i < nodeCount; i++)
    {
        /*
         * Checking for following items:
         * [sw] DMA buffer handle is file descriptor, should be GE 0.
         * [hw] stride must be 4 pixel aligned.
         * [sw] stride should not be too large (roughly use 16K pixel threshold).
         * [hw] buffer start address (offset) must be 64 byte aligned.
         * [sw] buffer offset should not be too large (roughly use 64M byte threshold)
         */
        if ((Handle[i] >= 0x80000000) ||
            (Stride[i] & (bitsPerPixel[i] * 4 / 8 - 1)) ||
            (Stride[i] > (bitsPerPixel[i] * 0x4000 / 8)) ||
            (BufferOffset[i] & 0x3F) ||
            (BufferOffset[i] > 0x4000000))
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    /* Compute the surface placement parameters. */
    switch (surface->info.format)
    {
    case gcvSURF_YV12:
        /*  WxH Y plane followed by (W/2)x(H/2) V and U planes. */
        surface->info.stride  = Stride[0];
        surface->info.uStride = Stride[2];
        surface->info.vStride = Stride[1];

        /* No offsets to first node. */
        surface->info.vOffset = surface->info.uOffset = 0;

        surface->info.sliceSize
            = surface->info.stride  * surface->info.alignedH
            + surface->info.uStride * surface->info.alignedH / 2;
        break;

    case gcvSURF_I420:
        /*  WxH Y plane followed by (W/2)x(H/2) U and V planes. */
        surface->info.stride  = Stride[0];
        surface->info.uStride = Stride[1];
        surface->info.vStride = Stride[2];

        /* No offsets to first node. */
        surface->info.uOffset = surface->info.vOffset = 0;

        surface->info.sliceSize
            = surface->info.stride  * surface->info.alignedH
            + surface->info.uStride * surface->info.alignedH / 2;
        break;

    case gcvSURF_NV12:
    case gcvSURF_NV21:
        /*  WxH Y plane followed by (W)x(H/2) interleaved U/V plane. */
        surface->info.stride  = Stride[0];
        surface->info.uStride =
        surface->info.vStride = Stride[1];

        /* No offsets to first node. */
        surface->info.uOffset = surface->info.vOffset = 0;

        surface->info.sliceSize
            = surface->info.stride  * surface->info.alignedH
            + surface->info.uStride * surface->info.alignedH / 2;
        break;

    case gcvSURF_NV16:
    case gcvSURF_NV61:
        /*  WxH Y plane followed by WxH interleaved U/V(V/U) plane. */
        surface->info.stride  = Stride[0];
        surface->info.uStride =
        surface->info.vStride = Stride[1];

        /* No offsets to first node. */
        surface->info.uOffset = surface->info.vOffset = 0;

        surface->info.sliceSize
            = surface->info.stride  * surface->info.alignedH
            + surface->info.uStride * surface->info.alignedH;
        break;

    case gcvSURF_YUY2:
    case gcvSURF_UYVY:
    case gcvSURF_YVYU:
    case gcvSURF_VYUY:
        /*  WxH interleaved Y/U/Y/V plane. */
        surface->info.stride  =
        surface->info.uStride =
        surface->info.vStride = Stride[0];

        /* No offsets to first node. */
        surface->info.uOffset = surface->info.vOffset = 0;

        surface->info.sliceSize
            = surface->info.stride * surface->info.alignedH;
        break;

    default:
        surface->info.stride = Stride[0];

        /* No u,v strides. */
        surface->info.uStride = surface->info.vStride = 0;

        /* No offsets to first node. */
        surface->info.uOffset = surface->info.vOffset = 0;

        surface->info.sliceSize
            = surface->info.stride * surface->info.alignedH;
        break;
    }

    surface->info.layerSize = surface->info.sliceSize
                            * surface->info.requestD;

    /* Always single layer for user surface */
    gcmASSERT(surface->info.formatInfo.layers == 1);

    surface->info.size = surface->info.layerSize
                       * surface->info.formatInfo.layers;

    /* Wrap handles into Vivante HAL. */
    for (i = 0; i < nodeCount; i++)
    {
        gcsUSER_MEMORY_DESC desc;

        desc.handle = Handle[i];
        desc.flag = Flag;

        gcmONERROR(gcoHAL_WrapUserMemory(&desc, &node[i]));
    }

    /* Import wrapped video memory node to the surface. */
    switch (nodeCount)
    {
    case 3:
        surface->info.node3.u.normal.node = node[2];
        surface->info.node3.bufferOffset  = BufferOffset[2];
        surface->info.node3.pool = gcvPOOL_VIRTUAL;
        surface->info.node3.size = Stride[2] * Height + BufferOffset[2];
    case 2:
        surface->info.node2.u.normal.node = node[1];
        surface->info.node2.bufferOffset  = BufferOffset[1];
        surface->info.node2.pool = gcvPOOL_VIRTUAL;
        surface->info.node3.size = Stride[1] * Height + BufferOffset[1];
    default:
        surface->info.node.u.normal.node  = node[0];
        surface->info.node.bufferOffset   = BufferOffset[0];
        surface->info.node.pool  = gcvPOOL_VIRTUAL;
        surface->info.node.size  = Stride[0] * Height + BufferOffset[0];
        break;
    }

    /* Get a normal surface. */
    *Surface = surface;

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    for (i = 0; i < 3; i++)
    {
        if (node[i] > 0) {
            gcmVERIFY_OK(gcoHAL_ReleaseVideoMemory(node[i]));
        }
    }

    if (surface)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(surface));
    }

    gcmFOOTER();
    return status;
}






#if gcdENABLE_3D

gceSTATUS
gcoSURF_3DBlitBltRect(
    IN gceENGINE Engine,
    IN gcoSURF SrcSurf,
    IN gcoSURF DestSurf,
    IN gcsPOINT_PTR SrcOrigin,
    IN gcsPOINT_PTR DestOrigin,
    IN gcsPOINT_PTR RectSize,
    IN gctBOOL YInverted
    )
{
    gcmPRINT("ERROR: %s was deprecated, please use %s_v2 instead!!!\n", __FUNCTION__, __FUNCTION__);
    gcmASSERT(gcvFALSE);
    return gcvSTATUS_INVALID_REQUEST;
}

gceSTATUS
gcoSURF_CopyPixels(
    IN gcoSURF Source,
    IN gcoSURF Target,
    IN gctINT SourceX,
    IN gctINT SourceY,
    IN gctINT TargetX,
    IN gctINT TargetY,
    IN gctINT Width,
    IN gctINT Height
    )
{
    gcmPRINT("ERROR: %s was deprecated, please use %s_v2 instead!!!\n", __FUNCTION__, __FUNCTION__);
    gcmASSERT(gcvFALSE);
    return gcvSTATUS_INVALID_REQUEST;
}



/*******************************************************************************
**
**  gcoSURF_Resolve
**
**  Resolve the surface to the frame buffer.  Resolve means that the frame is
**  finished and should be displayed into the frame buffer, either by copying
**  the data or by flipping to the surface, depending on the hardware's
**  capabilities.
**
**  INPUT:
**
**      gcoSURF SrcSurface
**          Pointer to a gcoSURF object that represents the source surface
**          to be resolved.
**
**      gcoSURF DestSurface
**          Pointer to a gcoSURF object that represents the destination surface
**          to resolve into.
**
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_Resolve(
    IN gcoSURF SrcSurface,
    IN gcoSURF DestSurface
    )
{
    gcmPRINT("ERROR: %s was deprecated, please use gcoSURF_ResolveRect_v2 instead!!!\n", __FUNCTION__);
    gcmASSERT(gcvFALSE);
    return gcvSTATUS_INVALID_REQUEST;

}


/*******************************************************************************
**
**  gcoSURF_ResolveEx
**
**  Resolve the surface to the frame buffer.  Resolve means that the frame is
**  finished and should be displayed into the frame buffer, either by copying
**  the data or by flipping to the surface, depending on the hardware's
**  capabilities.
**
**  INPUT:
**
**      gcoSURF SrcSurface
**          Pointer to a gcoSURF object that represents the source surface
**          to be resolved.
**
**      gcoSURF DestSurface
**          Pointer to a gcoSURF object that represents the destination surface
**          to resolve into.
**
**      gcsSURF_RESOLVE_ARGS *args
**          Pointer to extra resolve argument (multi-version)
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_ResolveEx(
    IN gcoSURF SrcSurface,
    IN gcoSURF DestSurface,
    IN gcsSURF_RESOLVE_ARGS *args
    )
{
    gcmPRINT("ERROR: %s was deprecated, please use gcoSURF_ResolveRect_v2 instead!!!\n", __FUNCTION__);
    gcmASSERT(gcvFALSE);
    return gcvSTATUS_INVALID_REQUEST;
}

/*******************************************************************************
**
**  gcoSURF_ResolveRect
**
**  Resolve a rectangular area of a surface to another surface.
**
**  INPUT:
**
**      gcoSURF SrcSurface
**          Pointer to a gcoSURF object that represents the source surface
**          to be resolved.
**
**      gcoSURF DestSurface
**          Pointer to a gcoSURF object that represents the destination surface
**          to resolve into.
**
**      gcsPOINT_PTR SrcOrigin
**          The origin of the source area to be resolved.
**
**      gcsPOINT_PTR DestOrigin
**          The origin of the destination area to be resolved.
**
**      gcsPOINT_PTR RectSize
**          The size of the rectangular area to be resolved.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_ResolveRect(
    IN gcoSURF SrcSurface,
    IN gcoSURF DestSurface,
    IN gcsPOINT_PTR SrcOrigin,
    IN gcsPOINT_PTR DestOrigin,
    IN gcsPOINT_PTR RectSize
    )
{
    gcmPRINT("ERROR: %s was deprecated, please use gcoSURF_ResolveRect_v2 instead!!!\n", __FUNCTION__);
    gcmASSERT(gcvFALSE);
    return gcvSTATUS_INVALID_REQUEST;
}


/*******************************************************************************
**
**  gcoSURF_ResolveRectEx
**
**  Resolve a rectangular area of a surface to another surface.
**  Support explicilitly yInverted resolve request in argument.
**  Note: RS only support tile<->linear yInverted resolve, other
**        case will be fallbacked to SW.
**
**  INPUT:
**
**      gcoSURF SrcSurface
**          Pointer to a gcoSURF object that represents the source surface
**          to be resolved.
**
**      gcoSURF DestSurface
**          Pointer to a gcoSURF object that represents the destination surface
**          to resolve into.
**
**      gcsPOINT_PTR SrcOrigin
**          The origin of the source area to be resolved.
**
**      gcsPOINT_PTR DestOrigin
**          The origin of the destination area to be resolved.
**
**      gcsPOINT_PTR RectSize
**          The size of the rectangular area to be resolved.
**
**      gcsSURF_RESOLVE_ARGS *args
**          Point to extra resolve argument (multiple-version)
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_ResolveRectEx(
    IN gcoSURF SrcSurface,
    IN gcoSURF DestSurface,
    IN gcsPOINT_PTR SrcOrigin,
    IN gcsPOINT_PTR DestOrigin,
    IN gcsPOINT_PTR RectSize,
    IN gcsSURF_RESOLVE_ARGS *args
    )
{
    gcmPRINT("ERROR: %s was deprecated, please use gcoSURF_ResolveRect_v2 instead!!!\n", __FUNCTION__);
    gcmASSERT(gcvFALSE);
    return gcvSTATUS_INVALID_REQUEST;
}

/*******************************************************************************
**
**  gcoSURF_Resample
**
**  Determine the ratio between the two non-multisampled surfaces and resample
**  based on the ratio.
**
**  INPUT:
**
**      gcoSURF SrcSurface
**          Pointer to a gcoSURF object that represents the source surface.
**
**      gcoSURF DestSurface
**          Pointer to a gcoSURF object that represents the destination surface.
**
**  OUTPUT:
**
**      Nothing.
*/

#define RESAMPLE_FLAG 0xBAAD1234

gceSTATUS
gcoSURF_Resample(
    IN gcoSURF SrcSurface,
    IN gcoSURF DestSurface
    )
{
    gcmPRINT("ERROR: %s was deprecated, please use gcoSURF_ResolveRect_v2 instead!!!\n", __FUNCTION__);
    gcmASSERT(gcvFALSE);
    return gcvSTATUS_INVALID_REQUEST;
}


gceSTATUS
gcoSURF_3DBlitClearRect(
    IN gcoSURF Surface,
    IN gcsSURF_CLEAR_ARGS_PTR ClearArgs
    )
{
    gcmPRINT("ERROR: %s was deprecated!!!\n", __FUNCTION__);
    gcmASSERT(gcvFALSE);
    return gcvSTATUS_INVALID_REQUEST;
}


gceSTATUS
gcoSURF_Clear(
    IN gcoSURF Surface,
    IN gcsSURF_CLEAR_ARGS_PTR  ClearArgs)
{
    gcmPRINT("ERROR: %s was deprecated, please use %s_v2 instead!!!\n", __FUNCTION__, __FUNCTION__);
    gcmASSERT(gcvFALSE);
    return gcvSTATUS_INVALID_REQUEST;
}

gceSTATUS
gcoSURF_BlitDraw(
    IN gcsSURF_BLITDRAW_ARGS *args
    )
{
    gcmPRINT("ERROR: %s was deprecated, please use gcoSURF_DrawBlit_v2 instead!!!\n", __FUNCTION__);
    gcmASSERT(gcvFALSE);
    return gcvSTATUS_INVALID_REQUEST;
}

gceSTATUS
gcoSURF_SetOffset(
    IN gcoSURF Surface,
    IN gctSIZE_T Offset
    )
{
    gcmPRINT("ERROR: %s was deprecated!!!\n", __FUNCTION__);
    gcmASSERT(gcvFALSE);
    return gcvSTATUS_INVALID_REQUEST;
}

#endif

