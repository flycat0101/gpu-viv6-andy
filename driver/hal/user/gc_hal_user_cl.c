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
#if gcdENABLE_3D
#include "gc_hal_cl.h"

#define _GC_OBJ_ZONE            gcvZONE_CL

#define _USE_CL_HARDWARE_       0

/******************************************************************************\
|********************************* Structures *********************************|
\******************************************************************************/

/* All OCL functions use the same hardware (different from tls's current hardware)
   to avoid state contamination with other APIs.
 */

/* gcoCL object. */
struct _gcoCL
{
    /* Object. */
    gcsOBJECT                   object;

    gcoOS                       os;
    gcoHAL                      hal;

    /* Hardware object for this CL 3D engine */
    gcoHARDWARE                 hardware;

    /* Mutex for hardware. */
    gctPOINTER                  mutex;
};

gcoCL   _gCL = gcvNULL;


/******************************************************************************\
|*********************************** Macros ***********************************|
\******************************************************************************/

#if _USE_CL_HARDWARE_
#define gcmSWITCHHARDWAREVAR \
    gcsTLS_PTR _tls; \
    gcoHARDWARE _currentHardware; \
    gceAPI _currentApi;

#define gcmSWITCHHARDWARE() \
{ \
    _currentApi = gcvAPI_OPENCL; \
    gcmVERIFY_OK(gcoOS_GetTLS(&_tls)); \
    _currentHardware = _tls->currentHardware; \
    gcmVERIFY_OK(gcoHARDWARE_GetAPI(_currentHardware, &_currentApi, gcvNULL)); \
    gcmASSERT(_gCL->hardware); \
    if (_currentApi != gcvAPI_OPENCL) \
    { \
        gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL, _gCL->mutex, gcvINFINITE)); \
        _tls->currentHardware = _gCL->hardware; \
    } \
}

#define gcmRESTOREHARDWARE() \
{ \
    if (_tls && _currentApi != gcvAPI_OPENCL) \
    { \
        _tls->currentHardware = _currentHardware; \
        gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, _gCL->mutex)); \
    } \
}
#else
#define gcmSWITCHHARDWAREVAR
#define gcmSWITCHHARDWARE()
#define gcmRESTOREHARDWARE()
#endif

/******************************************************************************\
|******************************* gcoCL API Code *******************************|
\******************************************************************************/

/*******************************************************************************
**
**  gcoCL_Construct
**
**  Contruct a new gcoCL object.
**
**  OUTPUT:
**
**      gcoCL * CLObj
**          Pointer to a variable receiving the gcoCL object pointer.
*/
gceSTATUS
gcoCL_Construct(
    OUT gcoCL * CLObj
    )
{
    gcoCL       clObj = gcvNULL;
    gceSTATUS   status;
    gctPOINTER  pointer = gcvNULL;

    gcmHEADER();

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(CLObj != gcvNULL);

    /* Allocate the gco3D object. */
    gcmONERROR(
        gcoOS_Allocate(gcvNULL,
                       gcmSIZEOF(struct _gcoCL),
                       &pointer));

    clObj = pointer;

    /* Initialize the gcoCL object. */
    clObj->object.type = gcvOBJ_CL;

#if _USE_CL_HARDWARE_
    gcmONERROR(gcoOS_Construct(gcvNULL, &clObj->os));
    gcmONERROR(gcoHAL_Construct(gcvNULL, clObj->os, &clObj->hal));

        /* Set the hardware type. */
    gcmONERROR(gcoHAL_SetHardwareType(clObj->hal, gcvHARDWARE_3D));

    /* Construct hardware object for this engine */
    gcmONERROR(gcoHARDWARE_Construct(clObj->hal, gcvFALSE, &clObj->hardware));

    /* Switch to the 3D pipe. */
    gcmONERROR(gcoHARDWARE_SelectPipe(clObj->hardware, gcvPIPE_3D, gcvNULL));

    /* Set API to OpenCL. */
    gcmVERIFY_OK(gcoHARDWARE_SetAPI(clObj->hardware, gcvAPI_OPENCL));

    /* Set rounding mode */
    if (gcoHARDWARE_IsFeatureAvailable(clObj->hardware, gcvFEATURE_SHADER_HAS_RTNE))
    {
        gcmVERIFY_OK(gcoHARDWARE_SetRTNERounding(clObj->hardware, gcvTRUE));
    }

    gcmONERROR(gcoHARDWARE_InvalidateCache(clObj->hardware));

    /* Create thread lock signal. */
    gcmONERROR(gcoOS_CreateMutex(gcvNULL, &clObj->mutex));
#else
    clObj->os       = gcvNULL;
    clObj->hal      = gcvNULL;
    clObj->hardware = gcvNULL;
    clObj->mutex    = gcvNULL;
#endif

    /* Return pointer to the gcoCL object. */
    *CLObj = clObj;

    /* Success. */
    gcmFOOTER_ARG("*Engine=0x%x", *CLObj);
    return gcvSTATUS_OK;

OnError:
    if (clObj != gcvNULL)
    {
        /* Unroll. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, clObj));
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoCL_InitializeHardware
**
**  Initialize hardware.  This is required for each thread.
**
**  INPUT:
**
**      Nothing
**
**  OUTPUT:
**
**      Nothing
*/
gceSTATUS
gcoCL_InitializeHardware()
{
    gceSTATUS status;
    gceAPI currentApi;

    gcmHEADER();

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

    /* Set rounding mode */
    if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_SHADER_HAS_RTNE))
    {
        gcmVERIFY_OK(gcoHARDWARE_SetRTNERounding(gcvNULL, gcvTRUE));
    }

    if (_gCL == gcvNULL)
    {
        /* Construct _gCL. */
        gcmONERROR(gcoCL_Construct(&_gCL));
    }
    gcmONERROR(gcoCLHardware_Construct());

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
#if _USE_CL_HARDWARE_
    if (_gCL != gcvNULL)
    {
        /* Unroll. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, _gCL));
    }
#endif

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/**********************************************************************
**
**  gcoCL_SetHardware
**
**  Set the gcoHARDWARE object for current thread.
**
**  INPUT:
**
**      Nothing
**
**  OUTPUT:
**
**      Nothing
*/
gceSTATUS
gcoCL_SetHardware()
{
#if _USE_CL_HARDWARE_ && 0
    gceSTATUS status;
    gcsTLS_PTR tls;

    gcmHEADER();

    gcmONERROR(gcoOS_GetTLS(&tls));

    gcmASSERT(tls->currentHardware == gcvNULL);

    /* Set current hardware. */
    tls->currentType = gcvHARDWARE_3D;
    tls->currentHardware = _gCL->hardware;

#if gcdDUMP
    gcmDUMP(gcvNULL, "@[start contextid %d, default=%d]", _gCL->hardware->context, _gCL->hardware->threadDefault);
#endif

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
#else
    return gcoCL_InitializeHardware();
#endif
}

/******************************************************************************\
|****************************** MEMORY MANAGEMENT *****************************|
\******************************************************************************/

/*******************************************************************************
**
**  gcoCL_AllocateMemory
**
**  Allocate contiguous memory from the kernel.
**
**  INPUT:
**
**      gctUINT * Bytes
**          Pointer to the number of bytes to allocate.
**
**  OUTPUT:
**
**      gctUINT * Bytes
**          Pointer to a variable that will receive the aligned number of bytes
**          allocated.
**
**      gctPHYS_ADDR * Physical
**          Pointer to a variable that will receive the physical addresses of
**          the allocated memory.
**
**      gctPOINTER * Logical
**          Pointer to a variable that will receive the logical address of the
**          allocation.
**
**      gcsSURF_NODE_PTR  * Node
**          Pointer to a variable that will receive the gcsSURF_NODE structure
**          pointer that describes the video memory to lock.
*/
gceSTATUS
gcoCL_AllocateMemory(
    IN OUT gctUINT *        Bytes,
    OUT gctPHYS_ADDR *      Physical,
    OUT gctPOINTER *        Logical,
    OUT gcsSURF_NODE_PTR *  Node
    )
{
    /*gcmSWITCHHARDWAREVAR*/
    gceSTATUS status;
    gctUINT bytes;
    gcsSURF_NODE_PTR node = gcvNULL;

    gcmHEADER_ARG("*Bytes=%lu", *Bytes);

    /*gcmSWITCHHARDWARE();*/

    /* Allocate extra 64 bytes to avoid cache overflow */
    bytes = gcmALIGN(*Bytes + 64, 64);

#if USE_NEW_MEMORY_ALLOCATION
    /* Allocate the physical buffer for the command. */
    gcmONERROR(gcoHAL_AllocateContiguous(gcvNULL, &bytes, Physical, Logical));

    /* Return allocated number of bytes. */
    *Bytes = bytes;
#else
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
#endif

    /*gcmRESTOREHARDWARE();*/

    /* Success. */
    gcmFOOTER_ARG("*Bytes=%lu *Physical=0x%x *Logical=0x%x *Node=0x%x",
                  *Bytes, *Physical, *Logical, *Node);
    return gcvSTATUS_OK;

OnError:
    /*gcmRESTOREHARDWARE();*/

    /* Return the status. */
    if(node != gcvNULL)
    {
        gcoOS_Free(gcvNULL, node);
        node = gcvNULL;
    }
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoCL_FreeMemory
**
**  Free contiguous memeory to the kernel.
**
**  INPUT:
**
**      gctPHYS_ADDR Physical
**          The physical addresses of the allocated pages.
**
**      gctPOINTER Logical
**          The logical address of the allocation.
**
**      gctUINT Bytes
**          Number of bytes allocated.
**
**      gcsSURF_NODE_PTR  Node
**          Pointer to a gcsSURF_NODE structure
**          that describes the video memory to unlock.
**
**  OUTPUT:
**
**      Nothing
*/
gceSTATUS
gcoCL_FreeMemory(
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

    /*gcmSWITCHHARDWARE();*/

#if USE_NEW_MEMORY_ALLOCATION
    gcmONERROR(gcoOS_FreeContiguous(gcvNULL,
                                    Physical,
                                    Logical,
                                    Bytes));
#else
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
#endif

OnError:
    /*gcmRESTOREHARDWARE();*/

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoCL_FlushMemory
**
**  Flush memory to the kernel.
**
**  INPUT:
**
**      gcsSURF_NODE_PTR  Node
**          Pointer to a gcsSURF_NODE structure
**          that describes the video memory to flush.
**
**      gctPOINTER Logical
**          The logical address of the allocation.
**
**      gctSIZE_T Bytes
**          Number of bytes allocated.
**
**  OUTPUT:
**
**      Nothing
*/
gceSTATUS
gcoCL_FlushMemory(
    IN gcsSURF_NODE_PTR     Node,
    IN gctPOINTER           Logical,
    IN gctSIZE_T            Bytes
    )
{
    /*gcmSWITCHHARDWAREVAR*/
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Node=0x%x Logical=0x%x Bytes=%u",
                  Node, Logical, Bytes);

    /*gcmSWITCHHARDWARE();*/

    if (Node /*&& Node->pool == gcvPOOL_VIRTUAL*/)
    {
        /*gcmONERROR(gcoOS_CacheFlush(gcvNULL, Node->u.normal.node, Logical, Bytes));*/
        gcmONERROR(gcoSURF_NODE_Cache(Node,
                                      Logical,
                                      Bytes,
                                      gcvCACHE_FLUSH));
    }
    else
    {
        gcmONERROR(gcoOS_CacheFlush(gcvNULL, 0, Logical, Bytes));
    }

OnError:
    /*gcmRESTOREHARDWARE();*/

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoCL_InvalidateMemoryCache
**
**  Invalidate memory cache in CPU.
**
**  INPUT:
**
**      gcsSURF_NODE_PTR  Node
**          Pointer to a gcsSURF_NODE structure
**          that describes the video memory to flush.
**
**      gctPOINTER Logical
**          The logical address of the allocation.
**
**      gctSIZE_T Bytes
**          Number of bytes allocated.
**
**  OUTPUT:
**
**      Nothing
*/
gceSTATUS
gcoCL_InvalidateMemoryCache(
    IN gcsSURF_NODE_PTR     Node,
    IN gctPOINTER           Logical,
    IN gctSIZE_T            Bytes
    )
{
    /*gcmSWITCHHARDWAREVAR*/
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Node=0x%x Logical=0x%x Bytes=%u",
                  Node, Logical, Bytes);

    /*gcmSWITCHHARDWARE();*/

    if (Node /*&& Node->pool == gcvPOOL_VIRTUAL*/)
    {
        /*gcmONERROR(gcoOS_CacheInvalidate(gcvNULL, Node->u.normal.node, Logical, Bytes));*/
        gcmONERROR(gcoSURF_NODE_Cache(Node,
                                      Logical,
                                      Bytes,
                                      gcvCACHE_INVALIDATE));
    }
    else
    {
        gcmONERROR(gcoOS_CacheInvalidate(gcvNULL, 0, Logical, Bytes));
    }

OnError:
    /*gcmRESTOREHARDWARE();*/

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoCL_ShareMemoryWithStream
**
**  Share memory with a stream.
**
**  INPUT:
**
**      gcoSTREAM Stream
**          Pointer to the stream object.
**
**  OUTPUT:
**
**      gctSIZE_T * Bytes
**          Pointer to a variable that will receive the aligned number of bytes
**          allocated.
**
**      gctPHYS_ADDR * Physical
**          Pointer to a variable that will receive the physical addresses of
**          the allocated memory.
**
**      gctPOINTER * Logical
**          Pointer to a variable that will receive the logical address of the
**          allocation.
**
**      gcsSURF_NODE_PTR  * Node
**          Pointer to a variable that will receive the gcsSURF_NODE structure
**          pointer that describes the video memory to lock.
*/
gceSTATUS
gcoCL_ShareMemoryWithStream(
    IN gcoSTREAM            Stream,
    OUT gctSIZE_T *         Bytes,
    OUT gctPHYS_ADDR *      Physical,
    OUT gctPOINTER *        Logical,
    OUT gcsSURF_NODE_PTR *  Node
    )
{
    /*gcmSWITCHHARDWAREVAR*/
    gceSTATUS status;

    gcmHEADER_ARG("Stream=0x%x", Stream);

    /*gcmSWITCHHARDWARE();*/

    gcmONERROR(gcoSTREAM_Size(Stream, Bytes));
    gcmONERROR(gcoSTREAM_Node(Stream, Node));

    /* Lock the cl buffer. */
    gcmONERROR(gcoHARDWARE_Lock(*Node,
                                (gctUINT32 *) Physical,
                                Logical));

    /*gcmRESTOREHARDWARE();*/

    /* Success. */
    gcmFOOTER_ARG("*Bytes=%lu *Physical=0x%x *Logical=0x%x *Node=0x%x",
                  *Bytes, *Physical, *Logical, *Node);
    return gcvSTATUS_OK;

OnError:
    /*gcmRESTOREHARDWARE();*/

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoCL_ShareMemoryWithBufObj
**
**  Share memory with a BufObj.
**
**  INPUT:
**
**      gcoBUFOBJ Stream
**          Pointer to the stream object.
**
**  OUTPUT:
**
**      gctSIZE_T * Bytes
**          Pointer to a variable that will receive the aligned number of bytes
**          allocated.
**
**      gctPHYS_ADDR * Physical
**          Pointer to a variable that will receive the physical addresses of
**          the allocated memory.
**
**      gctPOINTER * Logical
**          Pointer to a variable that will receive the logical address of the
**          allocation.
**
**      gcsSURF_NODE_PTR  * Node
**          Pointer to a variable that will receive the gcsSURF_NODE structure
**          pointer that describes the video memory to lock.
*/
gceSTATUS
gcoCL_ShareMemoryWithBufObj(
    IN gcoBUFOBJ            BufObj,
    OUT gctSIZE_T *         Bytes,
    OUT gctPHYS_ADDR *      Physical,
    OUT gctPOINTER *        Logical,
    OUT gcsSURF_NODE_PTR *  Node
    )
{
    /*gcmSWITCHHARDWAREVAR*/
    gceSTATUS status;

    gcmHEADER_ARG("BufObj=0x%x", BufObj);

    /*gcmSWITCHHARDWARE();*/

    gcmONERROR(gcoBUFOBJ_GetSize(BufObj, Bytes));
    gcmONERROR(gcoBUFOBJ_GetNode(BufObj, Node));

    /* Lock the cl buffer. */
    gcmONERROR(gcoHARDWARE_Lock(*Node,
                                (gctUINT32 *) Physical,
                                Logical));

    /*gcmRESTOREHARDWARE();*/

    /* Success. */
    gcmFOOTER_ARG("*Bytes=%lu *Physical=0x%x *Logical=0x%x *Node=0x%x",
                  *Bytes, *Physical, *Logical, *Node);
    return gcvSTATUS_OK;

OnError:
    /*gcmRESTOREHARDWARE();*/

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoCL_UnshareMemory
**
**  Unshare memory object with GL object.
**
**  INPUT:
**
**      gcsSURF_NODE_PTR  Node
**          Pointer to a  gcsSURF_NODE structure
*/
gceSTATUS
gcoCL_UnshareMemory(
    IN gcsSURF_NODE_PTR     Node
    )
{
    /*gcmSWITCHHARDWAREVAR*/
    gceSTATUS status;

    gcmHEADER_ARG("Node=0x%x", Node);

    /*gcmSWITCHHARDWARE();*/

    /* Unlock the index buffer. */
    status = gcoHARDWARE_Unlock(Node, gcvSURF_INDEX);

    /*gcmRESTOREHARDWARE();*/

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoCL_FlushSurface
**
**  Flush surface to the kernel.
**
**  INPUT:
**
**      gcoSURF           Surface
**          gcoSURF structure
**          that describes the surface to flush.
**
**  OUTPUT:
**
**      Nothing
*/
gceSTATUS
gcoCL_FlushSurface(
    IN gcoSURF              Surface
    )
{
    /*gcmSWITCHHARDWAREVAR*/
    gceSTATUS status = gcvSTATUS_OK;
    gctPOINTER srcMemory[3] = {gcvNULL};

    gcmHEADER_ARG("Surface=0x%x", Surface);

    /*gcmSWITCHHARDWARE();*/

    if (Surface /*&& Surface->info.node.pool == gcvPOOL_VIRTUAL*/)
    {
        if (Surface->info.node.pool == gcvPOOL_USER)
        {
            status = gcoOS_CacheFlush(gcvNULL,
                                           0,
                                           Surface->info.node.logical,
                                           Surface->info.size);
        }
        else
        {
            status = gcoSURF_NODE_Cache(&Surface->info.node,
                                        srcMemory[0],
                                        Surface->info.size,
                                        gcvCACHE_FLUSH);
        }
    }

    /*gcmRESTOREHARDWARE();*/

    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoCL_LockSurface(
    IN gcoSURF Surface,
    OUT gctUINT32 * Address,
    OUT gctPOINTER * Memory
    )
{
    /*gcmSWITCHHARDWAREVAR*/
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x", Surface);

    /*gcmSWITCHHARDWARE();*/

    status = gcoSURF_Lock(Surface, Address, Memory);

    /*gcmRESTOREHARDWARE();*/

    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoCL_UnlockSurface(
    IN gcoSURF Surface,
    IN gctPOINTER Memory
    )
{
    /*gcmSWITCHHARDWAREVAR*/
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x", Surface);

    /*gcmSWITCHHARDWARE();*/

    status = gcoSURF_Unlock(Surface, Memory);

    /*gcmRESTOREHARDWARE();*/

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoCL_CreateTexture
**
**  Create texture for image.
**
**  INPUT:
**
**      gctUINT Width
**          Width of the image.
**
**      gctUINT Heighth
**          Heighth of the image.
**
**      gctUINT Depth
**          Depth of the image.
**
**      gctCONST_POINTER Memory
**          Pointer to the data of the input image.
**
**      gctUINT Stride
**          Size of one row.
**
**      gctUINT Slice
**          Size of one plane.
**
**      gceSURF_FORMAT FORMAT
**          Format of the image.
**
**      gceENDIAN_HINT EndianHint
**          Endian needed to handle the image data.
**
**  OUTPUT:
**
**      gcoTEXTURE * Texture
**          Pointer to a variable that will receive the gcoTEXTURE structure.
**
**      gcoSURF * Surface
**          Pointer to a variable that will receive the gcoSURF structure.
**
**      gctPHYS_ADDR * Physical
**          Pointer to a variable that will receive the physical addresses of
**          the allocated memory.
**
**      gctPOINTER * Logical
**          Pointer to a variable that will receive the logical address of the
**          allocation.
**
**      gctUINT * SurfStride
**          Pointer to a variable that will receive the stride of the texture.
*/
gceSTATUS
gcoCL_CreateTexture(
    IN OUT gctBOOL*         MapHostMemory,
    IN gctUINT              Width,
    IN gctUINT              Height,
    IN gctUINT              Depth,
    IN gctCONST_POINTER     Memory,
    IN gctUINT              Stride,
    IN gctUINT              Slice,
    IN gceSURF_FORMAT       Format,
    IN gceENDIAN_HINT       EndianHint,
    OUT gcoTEXTURE *        Texture,
    OUT gcoSURF *           Surface,
    OUT gctUINT32  *        Physical,
    OUT gctPOINTER *        Logical,
    OUT gctUINT *           SurfStride,
    OUT gctUINT *           SurfSliceSize
    )
{
    /*gcmSWITCHHARDWAREVAR*/
    gceSTATUS status;
    gcoTEXTURE texture = gcvNULL;
    gcoSURF surface = gcvNULL;

    gcmHEADER_ARG("Width=%u Height=%u Depth=%u Memory=0x%x "
                  "Stride=%u Slice=%u Format=%u EndianHint=%u",
                  Width, Height, Depth, Memory,
                  Stride, Slice, Format, EndianHint);

    /*gcmSWITCHHARDWARE();*/

    gcmASSERT(gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_TEXTURE_LINEAR));

    /* Try to map host memory first. */
    if (*MapHostMemory)
    {
        gctUINT32 alignedWidth= Width;
        gctUINT32 alignedHeight = Height;

        gcmASSERT(!(gcmALL_TO_UINT32(Memory) & 0x3F));

        gcmONERROR(
            gcoHARDWARE_AlignToTileCompatible(gcvNULL,
                                              gcvSURF_BITMAP,
                                              0,
                                              Format,
                                              &alignedWidth,
                                              &alignedHeight,
                                              Depth,
                                              gcvNULL,
                                              gcvNULL,
                                              gcvNULL));

        /* Check the alignment. */
        if ((alignedWidth == Width) && (alignedHeight == Height))
        {
            do {
                gcmERR_BREAK(gcoSURF_ConstructWrapper(gcvNULL, &surface));

                gcmERR_BREAK(gcoSURF_SetBuffer(surface,
                                     gcvSURF_BITMAP,
                                     Format,
                                     Stride,
                                     (gctPOINTER) Memory,
                                     gcvINVALID_ADDRESS));

                gcmERR_BREAK(gcoSURF_SetWindow(surface,
                                     0,
                                     0,
                                     Width,
                                     Height));
            } while (gcvFALSE);

            if (gcmIS_ERROR(status))
            {
                if (surface)
                {
                    gcoSURF_Destroy(surface);
                    surface = gcvNULL;
                }

                *MapHostMemory = gcvFALSE;
            }
        }
        else
        {
            *MapHostMemory = gcvFALSE;
        }
    }

    /* If mapping failed, create a surface. */
    if (surface == gcvNULL)
    {
        /*gcvSURF_FORMAT_OCL used to add 64byte for cache overflow in this case*/
        Format |= gcvSURF_FORMAT_OCL;

        /* Construct the source surface. */
        gcmONERROR(gcoSURF_Construct(gcvNULL,
                                     Width, Height, Depth,
                                     gcvSURF_BITMAP,
                                     Format,
                                     gcvPOOL_DEFAULT,
                                     &surface));

        gcmASSERT(surface->info.tiling == gcvLINEAR);
        gcmASSERT(surface->info.node.logical);

        if (Memory)
        {
            gctUINT i,j, lineBytes;
            gctUINT8_PTR srcSliceBegin;
            gctUINT8_PTR srcLineBegin;
            gctUINT8_PTR dstSliceBegin;
            gctUINT8_PTR dstLineBegin;
            srcSliceBegin = (gctUINT8_PTR)Memory;
            dstSliceBegin = surface->info.node.logical;
            lineBytes     = surface->info.bitsPerPixel / 8 * Width;
            for (i = 0; i< Depth; ++i)
            {
                srcLineBegin = srcSliceBegin;
                dstLineBegin = dstSliceBegin;
                for (j = 0; j < Height; ++j)
                {
                    gcoOS_MemCopy(
                        dstLineBegin,
                        srcLineBegin,
                        lineBytes);  /*Only copy what needed.*/
                    srcLineBegin += Stride;
                    dstLineBegin += surface->info.stride;
                }
                srcSliceBegin += Slice;
                dstSliceBegin += surface->info.sliceSize;
            }

            gcoCL_FlushMemory(
                &surface->info.node,
                surface->info.node.logical,
                surface->info.stride * Height * Depth);
        }
    }

    gcmONERROR(gcoTEXTURE_Construct(gcvNULL, &texture));

    /*texture->endianHint = EndianHint;*/

    gcmONERROR(gcoTEXTURE_AddMipMapFromClient(texture, 0, surface));

    /* Return physical address. */
    gcmGETHARDWAREADDRESS(surface->info.node, *Physical);

    /* Return logical address. */
    *Logical = surface->info.node.logical;

    /* Return surface stride. */
    *SurfStride = surface->info.stride;
    *SurfSliceSize = surface->info.sliceSize;

    *Texture = texture;
    *Surface = surface;

    /*gcmRESTOREHARDWARE();*/

    /* Success. */
    gcmFOOTER_ARG("*Texture=0x%x *Surface=0x%x *Physical=0x%x *Logical=0x%x",
                  *Texture, *Surface, *Physical, *Logical);
    return gcvSTATUS_OK;

OnError:
    /*gcmRESTOREHARDWARE();*/

    if (surface != gcvNULL)
    {
        gcoSURF_Destroy(surface);
    }

    /* Return the status. */
    if(texture != gcvNULL)
    {
        gcoTEXTURE_Destroy(texture);
        texture = gcvNULL;
    }
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoCL_DestroyTexture
**
**  Destroy an gcoTEXTURE object.
**
**  INPUT:
**
**      gcoTEXTURE Texture
**          Pointer to an gcoTEXTURE object.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoCL_DestroyTexture(
    IN gcoTEXTURE Texture,
    IN gcoSURF    Surface
    )
{
    /*gcmSWITCHHARDWAREVAR*/
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Texture=0x%x", Texture);

    /*gcmSWITCHHARDWARE();*/

    gcoTEXTURE_Destroy(Texture);

    gcoSURF_Destroy(Surface);

    /*gcmRESTOREHARDWARE();*/

    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoCL_SetupTexture(
    IN gcoTEXTURE           Texture,
    IN gcoSURF              Surface,
    IN gctUINT              SamplerNum,
    gceTEXTURE_ADDRESSING   AddressMode,
    gceTEXTURE_FILTER       FilterMode
    )
{
    /*gcmSWITCHHARDWAREVAR*/
    gceSTATUS status = gcvSTATUS_OK;
    gcsTEXTURE info = {0};
    gcoHARDWARE Hardware = gcvNULL;

    gcmHEADER_ARG("Texture=0x%x", Texture);

    /*gcmSWITCHHARDWARE();*/

    info.s = info.t = info.r = AddressMode;
    info.magFilter = info.minFilter = FilterMode;
    /* Mipmap is not supported. */
    /* TODO - need to enhance when suppporting mipmap. */
    info.mipFilter = gcvTEXTURE_NONE;
    info.anisoFilter = 1;

    /* No lod bias allowed. */
    info.lodBias = 0.0f;
    /* Need to relax these when supporting mipmap. */
    info.lodMin = 0.0f;
    info.lodMax = 0.0f;
    info.baseLevel = 0;
    info.maxLevel = 0;

    info.swizzle[gcvTEXTURE_COMPONENT_R] = gcvTEXTURE_SWIZZLE_R;
    info.swizzle[gcvTEXTURE_COMPONENT_G] = gcvTEXTURE_SWIZZLE_G;
    info.swizzle[gcvTEXTURE_COMPONENT_B] = gcvTEXTURE_SWIZZLE_B;
    info.swizzle[gcvTEXTURE_COMPONENT_A] = gcvTEXTURE_SWIZZLE_A;
    info.border[gcvTEXTURE_COMPONENT_R] = 0;
    info.border[gcvTEXTURE_COMPONENT_G] = 0;
    info.border[gcvTEXTURE_COMPONENT_B] = 0;
    /* TODO - need to check format to determine border color. */
    info.border[gcvTEXTURE_COMPONENT_A] = 0 /*255*/;

    /* PCF is not allowed. */
    info.compareMode = gcvTEXTURE_COMPARE_MODE_INVALID;
    info.compareFunc = gcvCOMPARE_INVALID;

    if (gcvSTATUS_TRUE == gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TX_DESCRIPTOR))
    {
        gcoTEXTURE_BindTextureDesc(Texture, SamplerNum, &info, 0);
    }
    else
    {
        gcoTEXTURE_BindTextureEx(Texture, 0, SamplerNum, &info, 0);
    }

    /* TODO - need to check thread walker in PS. */
    gcoTEXTURE_Flush(Texture);
    gcoTEXTURE_FlushVS(Texture);

    if (gcvSTATUS_TRUE == gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TX_DESCRIPTOR))
    {
        /* Program texture states. */
        gcmONERROR(gcoHARDWARE_ProgramTextureDesc(Hardware, gcvNULL));
    }
    else
    {
        /* Program texture states. */
        gcmONERROR(gcoHARDWARE_ProgramTexture(Hardware, gcvNULL));
    }

OnError:
    /*gcmRESTOREHARDWARE();*/

    /* Return the status. */
    gcmFOOTER();
    return status;
}


/*******************************************************************************
**
**  gcoCL_QueryDeviceInfo
**
**  Query the OpenCL capabilities of the device.
**
**  INPUT:
**
**      Nothing
**
**  OUTPUT:
**
**      gcoCL_DEVICE_INFO_PTR DeviceInfo
**          Pointer to the device information
*/
gceSTATUS
gcoCL_QueryDeviceInfo(
    OUT gcoCL_DEVICE_INFO_PTR   DeviceInfo
    )
{
    /*gcmSWITCHHARDWAREVAR*/
    gceSTATUS status;
    gctUINT threadCount;
    gctSIZE_T contiguousSize;
    gctPHYS_ADDR contiguousAddress;
    gctSIZE_T physicalSystemMemSize;

    gcmHEADER();

    /*gcmSWITCHHARDWARE();*/

    gcmASSERT(DeviceInfo != gcvNULL);

    /* Number of shader cores and threads */
    gcmONERROR(
        gcoHARDWARE_QueryShaderCaps(gcvNULL,
                                    gcvNULL,
                                    gcvNULL,
                                    gcvNULL,
                                    gcvNULL,
                                    &DeviceInfo->maxComputeUnits,
                                    &threadCount,
                                    gcvNULL,
                                    gcvNULL));

    DeviceInfo->maxWorkItemDimensions = 3;

    /* The below restrictions are based on 16-bits for Global ID (per component)
     * and 10-bits for Local ID (also per-component).  Technically, the maximum
     * work group size supported is 1024*1024*1024; however, below, since there
     * is only an aggregate number present, we have to restrict ourselves to
     *      min(x,y,z)=1024
     * This should not be overly restrictive since larger than that will not fit
     * in our L1 cache well.
     */
    DeviceInfo->maxWorkItemSizes[0]   = gcmMIN(threadCount, 1024);
    DeviceInfo->maxWorkItemSizes[1]   = gcmMIN(threadCount, 1024);
    DeviceInfo->maxWorkItemSizes[2]   = gcmMIN(threadCount, 1024);
    DeviceInfo->maxWorkGroupSize      = gcmMIN(threadCount, 1024);

    if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_SHADER_ENHANCEMENTS2))
    {
        DeviceInfo->maxGlobalWorkSize     = (gctUINT64) 4*1024*1024*1024;
    }
    else
    {
        DeviceInfo->maxGlobalWorkSize     = 64*1024;
    }

    /* System configuration information */
    if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_MMU))
    {
        gcmONERROR(gcoOS_GetPhysicalSystemMemorySize(&physicalSystemMemSize));
        /* CTS1.1 allocation tests are using global memory size
           CTS1.2 allocation tests are using global memory size. (Qcom changes)
           To avoid OOM in CTS allocation tests on some Android systems, limit CL_DEVICE_GLOBAL_MEM_SIZE no more than 256M.
           The first OCL1.2 submission actually reported 256M.
           Maybe we can increase that to 512M */
        gcmASSERT (physicalSystemMemSize >= 0x20000000);
        DeviceInfo->globalMemSize         = gcmMIN(physicalSystemMemSize / 2, 0x10000000);

        /* Follow FP spec here. The minimum CL_DEVICE_MAX_MEM_ALLOC_SIZE is max (1/4th of CL_DEVICE_GLOBAL_MEM_SIZE, 128*1024*1024).
          CTS thread dimension running time would be too long (>2 days) if CL_DEVICE_MAX_MEM_ALLOC_SIZE < 64M. */

        DeviceInfo->maxMemAllocSize       = gcmMAX(DeviceInfo->globalMemSize / 4, 0x8000000);
    }
    else
    {
        /* Report maxMemAllocSize and globalMemsize based on contiguous video mem size only on old chips without the MMU fix. */

        gcmONERROR(
        gcoOS_QueryVideoMemory(gcvNULL,
                               gcvNULL,            gcvNULL,
                               gcvNULL,            gcvNULL,
                               &contiguousAddress, &contiguousSize));

        DeviceInfo->maxMemAllocSize       = contiguousSize / 4;
        DeviceInfo->globalMemSize         = contiguousSize / 2;
    }

    gcmONERROR(
        gcoHARDWARE_QueryShaderCapsEx(gcvNULL,
                                      &DeviceInfo->localMemSize,
                                      &DeviceInfo->addrBits,
                                      &DeviceInfo->globalMemCachelineSize,
                                      (gctUINT32_PTR)&DeviceInfo->globalMemCacheSize,
                                      &DeviceInfo->clockFrequency));

    DeviceInfo->localMemType          = 0x2         /* CL_GLOBAL */;
    DeviceInfo->globalMemCacheType    = 0x2         /* CL_READ_WRITE_CACHE */;

    /* Hardware capability: Constant size = 256*16B = 4KB
     * If unified constants were implemented, this would be 512*16B=8KB.
     * The constants can be divided any way the OpenCL software stack likes
     * between constant arguments, compiler needed constants, and the
     * required constants from the program itself.
     * The sum of these 3 cannot exceed the total number of constants.
     * maxConstantBufferSize = 256 * 16 - ConstantArgSize - ParameterSize
     */
    DeviceInfo->maxConstantBufferSize = 256 * 16;
    DeviceInfo->maxConstantArgs       = 9;
    DeviceInfo->maxParameterSize      = 256;

    DeviceInfo->maxPrintfBufferSize   = 1024 * 1024;

    /* Size (in bits) of the largest OpenCL built-in data type (long16) */
    DeviceInfo->memBaseAddrAlign      = 1024;

    /* Size (in bytes) of the largest OpenCL builtin data type (long16) */
    DeviceInfo->minDataTypeAlignSize  = 128;

    /* TODO - Need to check if hardware has GL components. */
    DeviceInfo->imageSupport          = gcvTRUE;

    /* Hardware capabilities.  14-bits per dimension */
    gcmONERROR(
        gcoHARDWARE_QueryTextureCaps(gcvNULL,
                                     &DeviceInfo->image3DMaxWidth,
                                     &DeviceInfo->image3DMaxHeight,
                                     &DeviceInfo->image3DMaxDepth,
                                     gcvNULL,
                                     gcvNULL,
                                     gcvNULL,
                                     &DeviceInfo->maxReadImageArgs,
                                     gcvNULL,
                                     gcvNULL));

#if !BUILD_OPENCL_FP
    {
        gceCHIPMODEL  chipModel;
        gctUINT32 chipRevision;

        gcoHAL_QueryChipIdentity(gcvNULL,&chipModel,&chipRevision,gcvNULL,gcvNULL);

        if((chipModel == gcv3000) && ((chipRevision == 0x5435) || (chipRevision == 0x5450) || (chipRevision == 0x5512)))
        {
            DeviceInfo->image3DMaxWidth = 2048;
            DeviceInfo->image3DMaxHeight = 2048;
            DeviceInfo->image3DMaxDepth = 2048;
        }
    }
#endif

    DeviceInfo->image2DMaxWidth       = DeviceInfo->image3DMaxWidth;
    DeviceInfo->image2DMaxHeight      = DeviceInfo->image3DMaxHeight;

    if (DeviceInfo->image3DMaxDepth <= 1)
    {
        /* No image3D support. */
        DeviceInfo->image3DMaxWidth  = 0;
        DeviceInfo->image3DMaxHeight = 0;
        DeviceInfo->image3DMaxDepth  = 0;
    }

    DeviceInfo->imageMaxBufferSize   = 65536;

    DeviceInfo->maxReadImageArgs      = 8;

    /* Not limited by hardware.  Implemented by software.
     * Full profile requires 8.  Embedded profile requires 1.
     */
    DeviceInfo->maxWriteImageArgs     = 8;

    /* These should come from hardware capabilities based on the feature bit
     * that indicates higher perf math for 8-bits and 16-bits relative to 32-bits.
     */
    DeviceInfo->vectorWidthChar       = 4;      /* gc4000:16,   gc2100:4 */
    DeviceInfo->vectorWidthShort      = 4;      /* gc4000:8,    gc2100:4 */
    DeviceInfo->vectorWidthInt        = 4;
    DeviceInfo->vectorWidthLong       = 0;      /* unsupported extension */
    DeviceInfo->vectorWidthFloat      = 4;
    DeviceInfo->vectorWidthDouble     = 0;      /* unsupported extension */
    DeviceInfo->vectorWidthHalf       = 0;      /* unsupported extension */

    /* Until VS/PS samplers are unified for OpenCL.
     * Not presently in any product.
     * Full profile requires 16.  Embedded profile requires 8.
     */
    DeviceInfo->maxSamplers           = 8;

    /* These are software-stack dependent. */
    DeviceInfo->queueProperties       = 0x3;
                                    /*  CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE |
                                        CL_QUEUE_PROFILING_ENABLE;
                                    */

    /* System/driver dependent */
    DeviceInfo->hostUnifiedMemory     = gcvTRUE;

    /* Hardware capability */
    DeviceInfo->errorCorrectionSupport= gcvTRUE;

    /* Not supported:   CL_FP_DENORM
     *                  CL_FP_SOFT_FLOAT
     *                  CL_FP_ROUND_TO_NEAREST
     *                  CL_FP_ROUND_TO_INF
     *                  CL_FP_INF_NAN
     *                  CL_FP_FMA
     *
     * MAD/FMA is supported, but IEEE754-2008 compliance is uncertain
     */
    DeviceInfo->singleFpConfig        = 0x8;    /* CL_FP_ROUND_TO_ZERO */

    if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_SHADER_HAS_RTNE))
    {
        DeviceInfo->singleFpConfig   |= 0x4;    /* CL_FP_ROUND_TO_NEAREST */
    }

#if BUILD_OPENCL_FP
    if(gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_SHADER_HAS_ATOMIC) && gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_SHADER_HAS_RTNE))
    {
        DeviceInfo->singleFpConfig |= 0x2; /* CL_FP_INF_NAN */
        DeviceInfo->localMemSize    = 32*1024;
        DeviceInfo->maxSamplers     = 16;
        DeviceInfo->maxReadImageArgs      = 128;
        DeviceInfo->maxParameterSize      = 1024;
        DeviceInfo->maxConstantBufferSize = 64*1024;
    }
#endif

    DeviceInfo->doubleFpConfig        = 0;      /* unsupported extension */

    /* Computed from system configuration: mc_clk period in nanoseconds */
    DeviceInfo->profilingTimingRes    = 1000;   /* nanoseconds */

    /* System configuration dependent */
    DeviceInfo->endianLittle          = gcvTRUE;
    DeviceInfo->deviceAvail           = gcvTRUE;
    DeviceInfo->compilerAvail         = gcvTRUE;
    DeviceInfo->linkerAvail           = gcvTRUE;

    /* Not supported:   CL_EXEC_NATIVE_KERNEL */
    DeviceInfo->execCapability        = 0x1     /* CL_EXEC_KERNEL */;

    DeviceInfo->atomicSupport         = gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_SHADER_HAS_ATOMIC);

    status = gcvSTATUS_OK;

OnError:
    /*gcmRESTOREHARDWARE();*/

    gcmFOOTER_ARG("status=%d", status);
    return status;
}

gceSTATUS
gcoCL_QueryDeviceCount(
    OUT gctUINT32 * Count
    )
{
    gcmHEADER();

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Count != gcvNULL);

    gcoHAL_Query3DCoreCount(gcvNULL, Count);

    gcmFOOTER_ARG("*Count=%d", *Count);
    return gcvSTATUS_OK;
}

gceSTATUS
gcoCL_SelectDevice(
    IN gctUINT32    DeviceId
    )
{
    /*gcmSWITCHHARDWAREVAR*/
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("DeviceId=%d", DeviceId);

    /*gcmSWITCHHARDWARE();*/

    /*gcmRESTOREHARDWARE();*/

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoCL_Commit
**
**  Commit the current command buffer to hardware and optionally wait until the
**  hardware is finished.
**
**  INPUT:
**
**      gctBOOL Stall
**          gcvTRUE if the thread needs to wait until the hardware has finished
**          executing the committed command buffer.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoCL_Commit(
    IN gctBOOL Stall
    )
{
    /*gcmSWITCHHARDWAREVAR*/
    gceSTATUS status;

    gcmHEADER_ARG("Stall=%d", Stall);

    /*gcmSWITCHHARDWARE();*/

    /* Commit the command buffer to hardware. */
    gcmONERROR(gcoHARDWARE_Commit(gcvNULL));

    if (Stall)
    {
        /* Stall the hardware. */
        gcmONERROR(gcoHARDWARE_Stall(gcvNULL));
    }

OnError:
    /*gcmRESTOREHARDWARE();*/

    /* Return status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoCL_Flush(
    IN gctBOOL      Stall
    )
{
    /*gcmSWITCHHARDWAREVAR*/
    gceSTATUS status;

    gcmHEADER_ARG("Stall=%d", Stall);

    /*gcmSWITCHHARDWARE();*/

    /* Flush the current pipe. */
    gcmONERROR(gcoHARDWARE_FlushPipe(gcvNULL, gcvNULL));

    gcmONERROR(gcoCL_Commit(Stall));

OnError:
    /*gcmRESTOREHARDWARE();*/

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoCL_CreateSignal
**
**  Create a new signal.
**
**  INPUT:
**
**      gctBOOL ManualReset
**          If set to gcvTRUE, gcoOS_Signal with gcvFALSE must be called in
**          order to set the signal to nonsignaled state.
**          If set to gcvFALSE, the signal will automatically be set to
**          nonsignaled state by gcoOS_WaitSignal function.
**
**  OUTPUT:
**
**      gctSIGNAL * Signal
**          Pointer to a variable receiving the created gctSIGNAL.
*/
gceSTATUS
gcoCL_CreateSignal(
    IN gctBOOL ManualReset,
    OUT gctSIGNAL * Signal
    )
{
    /*gcmSWITCHHARDWAREVAR*/
    gceSTATUS   status;

    gcmHEADER_ARG("ManualReset=%u Signal=0x%x", ManualReset, Signal);

    /*gcmSWITCHHARDWARE();*/

    status = gcoOS_CreateSignal(gcvNULL, ManualReset, Signal);

    /*gcmRESTOREHARDWARE();*/

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoCL_DestroySignal
**
**  Destroy a signal.
**
**  INPUT:
**
**      gctSIGNAL Signal
**          Pointer to the gctSIGNAL.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoCL_DestroySignal(
    IN gctSIGNAL Signal
    )
{
    /*gcmSWITCHHARDWAREVAR*/
    gceSTATUS   status;

    gcmHEADER_ARG("Signal=0x%x", Signal);

    /*gcmSWITCHHARDWARE();*/

    status = gcoOS_DestroySignal(gcvNULL, Signal);

    /*gcmRESTOREHARDWARE();*/

    gcmFOOTER();
    return status;
}

gceSTATUS
gcoCL_SubmitSignal(
    IN gctSIGNAL    Signal,
    IN gctHANDLE    Process
    )
{
    /*gcmSWITCHHARDWAREVAR*/
    gceSTATUS       status;
#if COMMAND_PROCESSOR_VERSION > 1
    gcsTASK_SIGNAL_PTR signal;
#else
    gcsHAL_INTERFACE iface;
#endif

    gcmHEADER_ARG("Signal=0x%x Process=0x%x", Signal, Process);

    /*gcmSWITCHHARDWARE();*/

#if COMMAND_PROCESSOR_VERSION > 1
#if gcdGC355_PROFILER
    /* Allocate a cluster event. */
    gcmONERROR(gcoHAL_ReserveTask(_gCL->hal,
                                  gcvNULL,
                                  0,0,0,
                                  gcvBLOCK_PIXEL,
                                  1,
                                  gcmSIZEOF(gcsTASK_SIGNAL),
                                  (gctPOINTER *) &signal));
#else
    gcmONERROR(gcoHAL_ReserveTask(_gCL->hal,
                                  gcvBLOCK_PIXEL,
                                  1,
                                  gcmSIZEOF(gcsTASK_SIGNAL),
                                  (gctPOINTER *) &signal));
#endif
    /* Fill in event info. */
    signal->id       = gcvTASK_SIGNAL;
    signal->process  = Process;
    signal->signal   = Signal;
#else
    iface.command            = gcvHAL_SIGNAL;
    iface.u.Signal.signal    = gcmPTR_TO_UINT64(Signal);
    iface.u.Signal.auxSignal = 0;
    iface.u.Signal.process   = gcmPTR_TO_UINT64(Process);
    iface.u.Signal.fromWhere = gcvKERNEL_COMMAND;

    gcmONERROR(gcoHARDWARE_CallEvent(gcvNULL, &iface));

    /* Commit the buffer. */
    gcmONERROR(gcoHARDWARE_Commit(gcvNULL));
#endif

OnError:
    /*gcmRESTOREHARDWARE();*/

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoCL_WaitSignal
**
**  Wait for a signal to become signaled.
**
**  INPUT:
**
**      gctSIGNAL Signal
**          Pointer to the gctSIGNAL.
**
**      gctUINT32 Wait
**          Number of milliseconds to wait.
**          Pass the value of gcvINFINITE for an infinite wait.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoCL_WaitSignal(
    IN gctSIGNAL Signal,
    IN gctUINT32 Wait
    )
{
    /*gcmSWITCHHARDWAREVAR*/
    gceSTATUS   status;

    gcmHEADER_ARG("Signal=0x%x Wait=%u", Signal, Wait);

    /*gcmSWITCHHARDWARE();*/

    status = gcoOS_WaitSignal(_gCL->os, Signal, Wait);

    /*gcmRESTOREHARDWARE();*/

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoCL_SetSignal
**
**  Make a signal to become signaled.
**
**  INPUT:
**
**      gctSIGNAL Signal
**          Pointer to the gctSIGNAL.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoCL_SetSignal(
    IN gctSIGNAL Signal
    )
{
    /*gcmSWITCHHARDWAREVAR*/
    gceSTATUS   status;

    gcmHEADER_ARG("Signal=0x%x", Signal);

    status = gcoOS_Signal(_gCL->os, Signal, gcvTRUE);

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**                                gcoCL_LoadKernel
********************************************************************************
**
**  Load a pre-compiled and pre-linked kernel program into the hardware.
**
**  INPUT:
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
gcoCL_LoadKernel(
    IN gctSIZE_T StateBufferSize,
    IN gctPOINTER StateBuffer,
    IN gcsHINT_PTR Hints
    )
{
    /*gcmSWITCHHARDWAREVAR*/
    gceSTATUS status;

    gcmHEADER_ARG("StateBufferSize=%d StateBuffer=0x%x Hints=0x%x",
                  StateBufferSize, StateBuffer, Hints);

    /*gcmSWITCHHARDWARE();*/

    /* Load kernel states. */
    status = gcLoadKernel(gcvNULL,
                          StateBufferSize,
                          StateBuffer,
                          Hints);

    /*gcmRESTOREHARDWARE();*/

    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoCL_InvokeThreadWalker(
    IN gcsTHREAD_WALKER_INFO_PTR Info
    )
{
#if _USE_CL_HARDWARE_
    gcmSWITCHHARDWAREVAR
    gceSTATUS status;

    gcmHEADER_ARG("Info=0x%x", Info);

    gcmSWITCHHARDWARE();

    /* Route to hardware. */
    status = gcoHARDWARE_InvokeThreadWalkerCL(gcvNULL, Info);

    gcmRESTOREHARDWARE();

    /* Return the status. */
    gcmFOOTER();
    return status;
#else
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
#endif
}

gceSTATUS
gcoCL_InvokeKernel(
    IN gcSHADER            Kernel,
    IN gctUINT             WorkDim,
    IN size_t              GlobalWorkOffset[3],
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
    info.workGroupSizeX  = LocalWorkSize[0] ? (gctUINT32)LocalWorkSize[0] : 1;
    info.workGroupCountX = info.globalSizeX / info.workGroupSizeX;
    if (WorkDim > 1)
    {
        info.globalSizeY     = (gctUINT32)GlobalWorkSize[1];
        info.globalOffsetY   = (gctUINT32)GlobalWorkOffset[1];
        info.workGroupSizeY  = LocalWorkSize[1] ? (gctUINT32)LocalWorkSize[1] : 1;
        info.workGroupCountY = info.globalSizeY / info.workGroupSizeY;
    }
    if (WorkDim > 2)
    {
        info.globalSizeZ     = (gctUINT32)GlobalWorkSize[2];
        info.globalOffsetZ   = (gctUINT32)GlobalWorkOffset[2];
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

    gcmONERROR(gcoCL_InvokeThreadWalker(&info));

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}
#endif /* gcdENABLE_3D */

