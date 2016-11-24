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


#include "gc_glff_precomp.h"

#define readDataForWLimit(Logical, result)                               \
{                                                                        \
    gctUINT8* ptr;                                                       \
    gctUINT32 data;                                                      \
    ptr = (gctUINT8*)(Logical);                                          \
    data = ((ptr[0]) | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3]) << 24); \
    result = *((gctFLOAT*)(&data));                                      \
}

/* help code for dump frame buffer */

#define _GC_OBJ_ZONE    glvZONE_DRAW

#define PER_VERTEX_WLIMIT_NUM    36

#if gcdDEBUG
gctBOOL kickOffPerDraw = gcvFALSE;
#endif


/******************************************************************************\
*********************** Support Functions and Definitions **********************
\******************************************************************************/

/*******************************************************************************
**
**  glfInitializeDraw/glfDeinitializeDraw
**
**  Draw system start up and shut down routines.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**  OUTPUT:
**
**      Nothing
*/

gceSTATUS
glfInitializeDraw(
    glsCONTEXT_PTR Context
    )
{
    gceSTATUS status;
    gctSIZE_T i;
    gcmHEADER_ARG("Context=0x%x", Context);

    /* Initialize the generic values (not used in OpenGL ES 1.1). */
    for (i = 0; i < gcmCOUNTOF(Context->attributeArray); ++i)
    {
        Context->attributeArray[i].genericValue[0] = 0.0f;
        Context->attributeArray[i].genericValue[1] = 0.0f;
        Context->attributeArray[i].genericValue[2] = 0.0f;
        Context->attributeArray[i].genericValue[3] = 1.0f;

        Context->attributeArray[i].genericSize = 4;
        Context->attributeArray[i].enable = gcvTRUE;
        Context->attributeArray[i].divisor = 0;
    }

    for (i = 0; i < gldSTREAM_SIGNAL_NUM; i++)
    {
        gcmONERROR(gcoOS_CreateSignal(
            gcvNULL, gcvFALSE, &(Context->streamSignals[i])
            ));

        gcmONERROR(gcoOS_Signal(
            gcvNULL, Context->streamSignals[i], gcvTRUE
            ));
    }

    /* Construct the vertex array. */
    status = gcoVERTEXARRAY_Construct(Context->hal, &Context->vertexArray);

    gcmFOOTER();
    /* Return status. */
    return status;

OnError:
    /* Roll back. */
    gcmVERIFY_OK(glfDeinitializeDraw(Context));

    /* Set error. */
    glmERROR(GL_OUT_OF_MEMORY);

    /* Return status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
glfDeinitializeDraw(
    glsCONTEXT_PTR Context
    )
{
    gceSTATUS   status;
    gctSIZE_T   i;

    gcmHEADER_ARG("Context=0x%x", Context);

    status = gcoVERTEXARRAY_Destroy(Context->vertexArray);

    for (i = 0; i < gldSTREAM_POOL_SIZE; i += 1)
    {
        if (Context->streams[i] != gcvNULL)
        {
            gcmONERROR(gcoSTREAM_Destroy(Context->streams[i]));
            Context->streams[i] = gcvNULL;
        }
    }

    /* before we destroy the signal, we need do stall to make sure
     * the event with signal has been send to hardware, we need this
     * sync, or the destroy maybe before the event signal.
     * _FreeStream will produce this kind of event*/
    gcmONERROR(gcoHAL_Commit(Context->hal, gcvTRUE));

    for (i = 0; i < gldSTREAM_SIGNAL_NUM; i += 1)
    {
        if (Context->streamSignals[i] != gcvNULL)
        {
            gcmONERROR(gcoOS_Signal(
                gcvNULL, Context->streamSignals[i], gcvTRUE
                ));

            gcmONERROR(gcoOS_DestroySignal(
                gcvNULL, Context->streamSignals[i]
                ));

            Context->streamSignals[i] = gcvNULL;
        }
    }

    gcmFOOTER();
    return status;

OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  _LogicOpPreProcess/_LogicOpPostProcess
**
**  Logic operation emulation.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**  OUTPUT:
**
**      Nothing
*/

#define glvCOLOR_KEY 0x4C3D2E1F

static gceSTATUS _LogicOpPreProcess(
    glsCONTEXT_PTR Context
    )
{
    gctUINT width;
    gctUINT height;
    gctUINT samples;
    gceSURF_FORMAT format;
    gceSTATUS status;

    gcsSURF_CLEAR_ARGS clearArgs;

    gcmHEADER_ARG("Context=0x%x", Context);

    do
    {
        gcsSURF_VIEW drawView = {Context->draw, 0, 1};
        gcsSURF_VIEW tmpView = {gcvNULL, 0, 1};
        gcsSURF_VIEW fbView = {gcvNULL, 0, 1};

        /* Get the frame buffer characteristics. */
        gcmERR_BREAK(gcoSURF_GetSize(Context->draw, &width, &height, gcvNULL));
        gcmERR_BREAK(gcoSURF_GetFormat(Context->draw, gcvNULL, &format));
        gcmERR_BREAK(gcoSURF_GetSamples(Context->draw, &samples));

        /* Disable tile status on the render target. */
        gcmERR_BREAK(gcoSURF_DisableTileStatus(&drawView, gcvTRUE));

        /* Create a linear buffer of the same size and format as frame buffer. */
        gcmERR_BREAK(gcoSURF_Construct(
            Context->hal,
            width, height, 1,
            gcvSURF_BITMAP, format,
            gcvPOOL_DEFAULT,
            &Context->fbLinear
            ));

        /* Set the samples. */
        gcmERR_BREAK(gcoSURF_SetSamples(
            Context->fbLinear,
            samples
            ));

        /* Resolve the frame buffer to the linear destination buffer. */
        fbView.surf = Context->fbLinear;
        gcmERR_BREAK(gcoSURF_ResolveRect(&drawView, &fbView, gcvNULL));

        /* Create a temporary frame buffer. */
        gcmERR_BREAK(gcoSURF_Construct(
            Context->hal,
            width, height, 1,
            gcvSURF_RENDER_TARGET_NO_TILE_STATUS,
            gcvSURF_A8R8G8B8,
            gcvPOOL_DEFAULT,
            &Context->tempDraw
            ));

        /* Set the samples. */
        gcmERR_BREAK(gcoSURF_SetSamples(
            Context->tempDraw,
            samples
            ));

        tmpView.surf = Context->tempDraw;

        /* Program the PE with the temporary frame buffer. */
        gcmERR_BREAK(gco3D_SetTarget(Context->hw, 0, &tmpView, 0));

        /* Clear the temporary frame buffer with an ugly background color. */
        gcmERR_BREAK(gco3D_SetClearColor(
            Context->hw,
            (gctUINT8) ((glvCOLOR_KEY >> 16) & 0xFF),
            (gctUINT8) ((glvCOLOR_KEY >>  8) & 0xFF),
            (gctUINT8) ((glvCOLOR_KEY >>  0) & 0xFF),
            (gctUINT8) ((glvCOLOR_KEY >> 24) & 0xFF)
            ));

        gcoOS_ZeroMemory(&clearArgs, sizeof(clearArgs));
        clearArgs.clearRect = gcvNULL;
        clearArgs.color.r.floatValue = ((gctUINT8) ((glvCOLOR_KEY >> 16) & 0xFF)) * glvFLOATONEOVER255;
        clearArgs.color.g.floatValue = ((gctUINT8) ((glvCOLOR_KEY >>  8) & 0xFF)) * glvFLOATONEOVER255;
        clearArgs.color.b.floatValue = ((gctUINT8) ((glvCOLOR_KEY >>  0) & 0xFF)) * glvFLOATONEOVER255;
        clearArgs.color.a.floatValue = ((gctUINT8) ((glvCOLOR_KEY >> 24) & 0xFF)) * glvFLOATONEOVER255;
        clearArgs.color.valueType = gcvVALUE_FLOAT;
        clearArgs.colorMask =   (gctUINT8)Context->colorMask[0]       |
                                ((gctUINT8)Context->colorMask[1] << 1) |
                                ((gctUINT8)Context->colorMask[2] << 2) |
                                ((gctUINT8)Context->colorMask[3] << 3);
        clearArgs.flags = gcvCLEAR_COLOR;

        gcmERR_BREAK(gcoSURF_Clear(&tmpView, &clearArgs));
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return the status. */
    return status;
}

static gceSTATUS _LogicOpPostProcess(
    glsCONTEXT_PTR Context
    )
{
    gctUINT width;
    gctUINT height;
    gctUINT samples;
    gcsSURF_VIEW tempView = {gcvNULL, 0, 1};
    gceSTATUS status;

    gcmHEADER_ARG("Context=0x%x", Context);

    do
    {
        gcsSURF_VIEW tempDrawView = {Context->tempDraw, 0, 1};
        gcsSURF_VIEW drawView = {Context->draw, 0, 1};
        gcsSURF_VIEW fbView = {gcvNULL, 0, 1};

        /* Get the frame buffer characteristics. */
        gcmERR_BREAK(gcoSURF_GetSize(Context->draw, &width, &height, gcvNULL));
        gcmERR_BREAK(gcoSURF_GetSamples(Context->draw, &samples));

        /* Create temporary linear frame buffers. */
        gcmONERROR(gcoSURF_Construct(
            Context->hal,
            width, height, 1,
            gcvSURF_BITMAP, gcvSURF_A8R8G8B8,
            gcvPOOL_DEFAULT,
            &tempView.surf
            ));

        /* Set the samples. */
        gcmONERROR(gcoSURF_SetSamples(
            tempView.surf,
            samples
            ));

        /* Resolve the temporary frame buffer to the linear source buffer. */
        gcmONERROR(gcoSURF_ResolveRect(&tempDrawView, &tempView, gcvNULL));

        /* Delete the temporary frame buffer. */
        gcmONERROR(gcoSURF_Destroy(Context->tempDraw));
        Context->tempDraw = gcvNULL;

#if gcdENABLE_2D
        /* Set the clipping rectangle. */
        gcmONERROR(gcoSURF_SetClipping(Context->fbLinear));

        /* Run the 2D operation using color keying. */
        gcmONERROR(gcoSURF_Blit(
            tempView.surf,
            Context->fbLinear,
            1, gcvNULL, gcvNULL,
            gcvNULL,
            Context->logicOp.rop,
            0xAA,
            gcvSURF_SOURCE_MATCH, glvCOLOR_KEY,
            gcvNULL, gcvSURF_UNPACKED
            ));
#else
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
#endif
        /* Flush the frame buffer. */
        gcmONERROR(gcoSURF_Flush(Context->fbLinear));

        /* Tile the linear destination buffer back to the frame buffer. */
        fbView.surf = Context->fbLinear;
        gcmONERROR(gcoSURF_ResolveRect(&fbView, &drawView, gcvNULL));

        /* Delete the linear frame buffers. */
        gcmONERROR(gcoSURF_Destroy(tempView.surf));
        tempView.surf = gcvNULL;
        gcmONERROR(gcoSURF_Destroy(Context->fbLinear));
        Context->fbLinear = gcvNULL;

        gcmVERIFY_OK(gco3D_SetClearColorF(
            Context->hw,
            Context->clearColor.value[0],
            Context->clearColor.value[1],
            Context->clearColor.value[2],
            Context->clearColor.value[3]
            ));

        /* Reprogram the PE. */
        gcmONERROR(gco3D_SetTarget(Context->hw, 0, &drawView, 0));

        /* Disable tile status on the render target. */
        gcmONERROR(
            gcoSURF_DisableTileStatus(&drawView, gcvFALSE));
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return the staus. */
    return status;

OnError:
    if (tempView.surf != gcvNULL)
    {
        gcoSURF_Destroy(tempView.surf);
        tempView.surf = gcvNULL;
    }
    gcmFOOTER();
    return status;
}


/*******************************************************************************
**
**  _GetPrimitiveCount
**
**  Determines the number of primitives to render given the mode and vertex
**  count.
**
**  INPUT:
**
**      PrimitiveMode
**          Primitive mode.
**
**      VertexCount
**          Vertex count.
**
**  OUTPUT:
**
**      PrimitiveCount
**          Number of primitives.
**
**      HalPrimitive
**          Primitive type for HAL.
*/
static gceSTATUS
glfGetPrimitiveCount(
    IN gcePRIMITIVE PrimitiveMode,
    IN gctINT VertexCount,
    OUT gctINT * PrimitiveCount
    )
{
    gceSTATUS result = gcvSTATUS_OK;

    /* Translate primitive count. */
    switch (PrimitiveMode)
    {
    case gcvPRIMITIVE_POINT_LIST:
        *PrimitiveCount = VertexCount;
        break;

    case gcvPRIMITIVE_LINE_LIST:
        *PrimitiveCount = VertexCount / 2;
        break;

    case gcvPRIMITIVE_LINE_LOOP:
        *PrimitiveCount = VertexCount;
        break;

    case gcvPRIMITIVE_LINE_STRIP:
        *PrimitiveCount = VertexCount - 1;
        break;

    case gcvPRIMITIVE_TRIANGLE_LIST:
        *PrimitiveCount = VertexCount / 3;
        break;

    case gcvPRIMITIVE_TRIANGLE_STRIP:
    case gcvPRIMITIVE_TRIANGLE_FAN:
        *PrimitiveCount = VertexCount - 2;
        break;

    default:
        result = gcvSTATUS_INVALID_ARGUMENT;
    }

    /* Return result. */
    return result;
}

static GLboolean _GetPrimitiveCount(
    IN GLenum PrimitiveMode,
    IN GLsizei VertexCount,
    OUT GLsizei * PrimitiveCount,
    OUT gcePRIMITIVE * HalPrimitive
    )
{
    GLboolean result = GL_TRUE;

    gcmHEADER_ARG("PrimitiveMode=0x%04x VertexCount=%d PrimitiveCount=0x%x HalPrimitive=0x%x",
                    PrimitiveMode, VertexCount, PrimitiveCount, HalPrimitive);

    /* Translate primitive count. */
    switch (PrimitiveMode)
    {
    case GL_TRIANGLE_STRIP:
        *PrimitiveCount = VertexCount - 2;
        *HalPrimitive   = gcvPRIMITIVE_TRIANGLE_STRIP;
        break;

    case GL_TRIANGLE_FAN:
        *PrimitiveCount = VertexCount - 2;
        *HalPrimitive   = gcvPRIMITIVE_TRIANGLE_FAN;
        break;

    case GL_TRIANGLES:
        *PrimitiveCount = VertexCount / 3;
        *HalPrimitive   = gcvPRIMITIVE_TRIANGLE_LIST;
        break;

    case GL_POINTS:
        *PrimitiveCount = VertexCount;
        *HalPrimitive   = gcvPRIMITIVE_POINT_LIST;
        break;

    case GL_LINES:
        *PrimitiveCount = VertexCount / 2;
        *HalPrimitive   = gcvPRIMITIVE_LINE_LIST;
        break;

    case GL_LINE_LOOP:
        *PrimitiveCount = VertexCount;
        *HalPrimitive   = gcvPRIMITIVE_LINE_LOOP;
        break;

    case GL_LINE_STRIP:
        *PrimitiveCount = VertexCount - 1;
        *HalPrimitive   = gcvPRIMITIVE_LINE_STRIP;
        break;

    default:
        result = GL_FALSE;
    }

    gcmFOOTER_ARG("result=%d", result);
    /* Return result. */
    return result;
}


/*******************************************************************************
**
**  _IsFullCulled
**
**  Check if need to discard this draw, since HW doesn't support
**  FRONT_AND_BACK cull mode.
**
**  INPUT:
**
**      Context
**          Current drawing context.
**
**      PrimitiveMode
**          Specifies the Primitive Mode.
**
**  OUTPUT:
**
**      GL_TRUE if the all primitive will be culled.
*/

static GLboolean _IsFullCulled(
    IN glsCONTEXT_PTR Context,
    IN GLenum PrimitiveMode
    )
{
    gcmHEADER_ARG("Context=0x%04x PrimitiveMode=%d", Context, PrimitiveMode);

    if (Context->cullStates.enabled
    &&  !Context->drawTexOESEnabled
    &&  (Context->cullStates.cullFace == GL_FRONT_AND_BACK)
    )
    {
        switch (PrimitiveMode)
        {
        case GL_TRIANGLES:
        case GL_TRIANGLE_FAN:
        case GL_TRIANGLE_STRIP:
            gcmFOOTER_ARG("result=%d", GL_TRUE);
            return GL_TRUE;
        }
    }

    gcmFOOTER_ARG("result=%d", GL_FALSE);
    return GL_FALSE;
}


/*******************************************************************************
**
**  _InvalidPalette
**
**  Verify whether proper matrix palette has been specified.
**
**  INPUT:
**
**      Context
**          Current drawing context.
**
**  OUTPUT:
**
**      GL_TRUE if the specified matrix palette is invalid.
*/

static GLboolean _InvalidPalette(
    IN glsCONTEXT_PTR Context
    )
{
    gcmHEADER_ARG("Context=0x%x", Context);
    if
    (
        /* Matrix palette enabled? */
        Context->matrixPaletteEnabled &&
        (
            /* Both streams have to be enabled. */
               !Context->aMatrixIndexInfo.streamEnabled
            || !Context->aWeightInfo.streamEnabled

            /* Number of components has to be within range. */
            || (Context->aMatrixIndexInfo.components < 1)
            || (Context->aMatrixIndexInfo.components > glvMAX_VERTEX_UNITS)

            || (Context->aWeightInfo.components < 1)
            || (Context->aWeightInfo.components > glvMAX_VERTEX_UNITS)
        )
    )
    {
        gcmFOOTER_ARG("result=%d", GL_TRUE);
        return GL_TRUE;
    }

    gcmFOOTER_ARG("result=%d", GL_FALSE);
    return GL_FALSE;
}

__GL_INLINE gceSTATUS
_computeAttribMask(
    IN glsCONTEXT_PTR Context,
    OUT gctUINT* AttribMask
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT i, count;
    gctUINT attr = 0, j;
    gctUINT enableBits = 0;

    gcmHEADER();

    /* Get number of attributes for the vertex shader. */
    gcmONERROR(gcSHADER_GetAttributeCount(Context->currProgram->vs.shader,
                                          &count));

    Context->attributeArray[gldATTRIBUTE_POSITION].enable = Context->aPositionInfo.streamEnabled;
    /* Walk all vertex shader attributes. */
    for (i = 0, j = 0; i < count; ++i)
    {
        gctBOOL attributeEnabled;

        /* Get the attribute linkage. */
        attr = Context->currProgram->vs.attributes[i].binding;

        gcmERR_BREAK(gcATTRIBUTE_IsEnabled(
            Context->currProgram->vs.attributes[i].attribute,
            &attributeEnabled
            ));

        if (attributeEnabled)
        {
            glsATTRIBUTEINFO_PTR attribute = Context->currProgram->vs.attributes[i].info;
            glsNAMEDOBJECT_PTR   buffer = attribute->buffer;

            /* Link the attribute to the vertex shader input. */
            Context->attributeArray[attr].linkage = j++;

            /* Enable the attribute. */
            enableBits |= 1 << attr;

            /* Update stream information. */
            Context->attributeArray[attr].stream   =
                ((buffer != gcvNULL) && (buffer->object != gcvNULL))
                ? ((glsBUFFER_PTR) buffer->object)->stream
                : gcvNULL;

#if gcdSYNC
            gcoSTREAM_GetFence(Context->attributeArray[attr].stream);
#endif

            if (buffer && buffer->object)
            {
                /* Check stream. */
                if (((glsBUFFER_PTR) buffer->object)->stream == gcvNULL)
                {
                    gcmONERROR(gcvSTATUS_INVALID_DATA);
                }
            }
        }
    }

    *AttribMask = enableBits;

    gcmFOOTER();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER();
    return status;
}

static gceSTATUS _VertexArray(
    IN glsCONTEXT_PTR Context,
    IN GLint First,
    IN GLsizei * Count,
    IN gceINDEX_TYPE IndexType,
    IN gcoINDEX IndexBuffer,
    IN const void * Indices,
    IN OUT gcePRIMITIVE * PrimitiveType,
    IN OUT gctUINT * PrimitiveCount
    )
{
    gctUINT enableBits = 0;
    gcoINDEX index;
    gceSTATUS status;
    gctSIZE_T vertexCount = *Count;

    gcmHEADER();

    index = IndexBuffer;

    gcmONERROR(_computeAttribMask(Context, &enableBits));

    /* Bind the vertex array to the hardware. */
    gcmONERROR(gcoVERTEXARRAY_Bind(Context->vertexArray,
                                   enableBits, Context->attributeArray,
                                   First, &vertexCount,
                                   IndexType, index, (gctPOINTER) Indices,
                                   PrimitiveType,
                                   PrimitiveCount,
                                   gcvNULL,
                                   gcvNULL));

    *Count = (GLsizei)vertexCount;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER();
    return status;
}

/*
**                   _           _
**                  |  1          |
** Flip Matrix: F = |    -1    1  |
**                  |       1     |
**                  |_         1 _|
**
** apply flip matrix as follows to patch mirrored texture image.
**
**      C' = F * C
**
** (C is current texture matrix)
*/
void _applyFlip(glsMATRIX_PTR matrix)
{
    glmMATFLOAT(matrix, 0, 1) = (glmMATFLOAT(matrix, 0, 3)) - (glmMATFLOAT(matrix, 0, 1));
    glmMATFLOAT(matrix, 1, 1) = (glmMATFLOAT(matrix, 1, 3)) - (glmMATFLOAT(matrix, 1, 1));
    glmMATFLOAT(matrix, 2, 1) = (glmMATFLOAT(matrix, 2, 3)) - (glmMATFLOAT(matrix, 2, 1));
    glmMATFLOAT(matrix, 3, 1) = (glmMATFLOAT(matrix, 3, 3)) - (glmMATFLOAT(matrix, 3, 1));

    matrix->identity = GL_FALSE;
}

/*******************************************************************************
**
**  _FlipBottomTopTextures
**
**  Apply the flip matrix to flip Y.
**
**  INPUT:
**
**      Context
**          Current drawing context.
**
*/
static gceSTATUS _FlipBottomTopTextures(
    IN glsCONTEXT_PTR Context,
    OUT GLboolean * Flipped
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER();

    *Flipped = gcvFALSE;

    do
    {
        gctUINT i;

        /* Iterate though the attributes. */
        for (i = 0; i < (gctUINT) Context->texture.pixelSamplers; i++)
        {
            glsTEXTURESAMPLER_PTR sampler;
            glsTEXTUREWRAPPER_PTR texture;
            gcoSURF surface = gcvNULL;
            gceORIENTATION orientation = gcvORIENTATION_TOP_BOTTOM;

            /* Get a shortcut to the current sampler. */
            sampler = &Context->texture.sampler[i];

            /* Skip the stage if disabled. */
            if (!sampler->stageEnabled)
            {
                continue;
            }

            gcmASSERT(sampler->binding != gcvNULL);
            gcmASSERT(sampler->binding->object != gcvNULL);

            /* Get a shortcut to the current sampler's bound texture. */
            texture = sampler->binding;

            if (gcmIS_ERROR(gcoTEXTURE_GetMipMap(texture->object, 0, &surface)))
            {
                gcmTRACE(gcvLEVEL_WARNING, "Failed to retrieve surface from texture. Ignored.");
                continue;
            }
            gcmASSERT(surface != gcvNULL);

            if (gcmIS_ERROR(gcoSURF_QueryOrientation(surface, &orientation)))
            {
                gcmTRACE(gcvLEVEL_WARNING, "Failed to retrieve orientation from surface. Ignored.");
                continue;
            }

            if (orientation == gcvORIENTATION_BOTTOM_TOP)
            {
                glsMATRIX_PTR topMatrix;
                GLuint matrixID;

                gcmTRACE(gcvLEVEL_VERBOSE, "texture %d is %s", i, (orientation ? "bottom-top" : "top-bottom"));

                topMatrix = Context->matrixStackArray[glvTEXTURE_MATRIX_0 + i].topMatrix;

                _applyFlip(topMatrix);

                matrixID = Context->matrixStackArray[glvTEXTURE_MATRIX_0 + i].matrixID;
                (*Context->matrixStackArray[glvTEXTURE_MATRIX_0 + i].dataChanged)(Context, matrixID);

                *Flipped = gcvTRUE;
            }
        }
    }
    while (gcvFALSE);

    gcmFOOTER_NO();
    return status;
}

#if gcdDUMP_API
static void
_DumpVertices(
    IN glsCONTEXT_PTR Context,
    IN GLint First,
    IN GLsizei Count
    )
{
    gctINT i;

    if(Context->arrayBuffer == gcvNULL)        /* Not using vbo. */
    {
        if (Context->aPositionInfo.streamEnabled &&
            Context->aPositionInfo.pointer != gcvNULL)
        {
            gcmDUMP_API_DATA((gctUINT8_PTR)Context->aPositionInfo.pointer +
                First * Context->aPositionInfo.stride,
                Count * Context->aPositionInfo.stride);
        }
        if (Context->aColorInfo.streamEnabled)
        {
            gcmDUMP_API_DATA((gctUINT8_PTR)Context->aColorInfo.pointer +
                First * Context->aColorInfo.stride,
                Count * Context->aColorInfo.stride);
        }
        if (Context->aNormalInfo.streamEnabled)
        {
            gcmDUMP_API_DATA((gctUINT8_PTR)Context->aNormalInfo.pointer +
                First * Context->aNormalInfo.stride,
                Count * Context->aNormalInfo.stride);
        }

        /* Check each texture unit. */
        for (i = 0; i < Context->texture.pixelSamplers; i++)
        {
            glsTEXTURESAMPLER_PTR sampler = Context->texture.sampler + i;
            if (sampler->aTexCoordInfo.streamEnabled &&
                sampler->aTexCoordInfo.pointer != gcvNULL)
            {
                gcmDUMP_API_DATA((gctUINT8_PTR)sampler->aTexCoordInfo.pointer +
                    First * sampler->aTexCoordInfo.stride,
                    Count * sampler->aTexCoordInfo.stride);
            }
        }
    }
    else                                    /* Using a vbo. */
    {
    }
}
#endif

/* Invert 3x3 block of matrix, into invMatrix. */
gctINT _invertMatrix(
    gctFLOAT* matrix,
    gctFLOAT* invMatrix
   )
{
    gctFLOAT determinant = 0;
    gctINT i, j;
    gctFLOAT TransposeMatrix[3][3];

    /* Copy transpose of top 3x3 block in matrix (4x4). */
    for(i = 0; i < 3; i++)
    {
        for(j = 0; j < 3; j++)
        {
            TransposeMatrix[i][j] = matrix[(j * 4) + i];
        }
    }

    /* Compute determinant of TransposeMatrix. */
    for(i = 0, j = 0; j < 3; j++)
    {
        if (j == 2)
        {
            determinant += TransposeMatrix[i][j]
                           * TransposeMatrix[i+1][0]
                           * TransposeMatrix[i+2][1];
        }
        else if (j == 1)
        {
            determinant += TransposeMatrix[i][j]
                           * TransposeMatrix[i+1][j+1]
                           * TransposeMatrix[i+2][0];
        }
        else
        {
            determinant += TransposeMatrix[i][j]
                           * TransposeMatrix[i+1][j+1]
                           * TransposeMatrix[i+2][j+2];
        }
    }

    for(i = 2, j = 0; j < 3; j++)
    {
        if (j == 2)
        {
            determinant -= TransposeMatrix[i][j]
                           * TransposeMatrix[i-1][0]
                           * TransposeMatrix[i-2][1];
        }
        else if(j == 1)
        {
            determinant -= TransposeMatrix[i][j]
                           * TransposeMatrix[i-1][j+1]
                           * TransposeMatrix[i-2][0];
        }
        else
        {
            determinant -= TransposeMatrix[i][j]
                           * TransposeMatrix[i-1][j+1]
                           * TransposeMatrix[i-2][j+2];
        }
    }

    if (determinant == 0)
    {
        return 0;
    }

    determinant = 1/determinant;

    invMatrix[0*3 + 0] = determinant *
                        ((TransposeMatrix[1][1] * TransposeMatrix[2][2])
                         - (TransposeMatrix[2][1] * TransposeMatrix[1][2]));

    invMatrix[0*3 + 1] = determinant *
                        ((TransposeMatrix[2][0] * TransposeMatrix[1][2])
                         - (TransposeMatrix[2][2] * TransposeMatrix[1][0]));

    invMatrix[0*3 + 2] = determinant *
                        ((TransposeMatrix[1][0] * TransposeMatrix[2][1])
                         - (TransposeMatrix[2][0] * TransposeMatrix[1][1]));

    invMatrix[1*3 + 0] = determinant *
                        ((TransposeMatrix[2][1] * TransposeMatrix[0][2])
                         - (TransposeMatrix[0][1] * TransposeMatrix[2][2]));

    invMatrix[1*3 + 1] = determinant *
                        ((TransposeMatrix[0][0] * TransposeMatrix[2][2])
                         - (TransposeMatrix[2][0] * TransposeMatrix[0][2]));

    invMatrix[1*3 + 2] = determinant *
                        ((TransposeMatrix[2][0] * TransposeMatrix[0][1])
                         - (TransposeMatrix[0][0] * TransposeMatrix[2][1]));

    invMatrix[2*3 + 0] = determinant *
                        ((TransposeMatrix[0][1] * TransposeMatrix[1][2])
                         - (TransposeMatrix[1][1] * TransposeMatrix[0][2]));

    invMatrix[2*3 + 1] = determinant *
                        ((TransposeMatrix[1][0] * TransposeMatrix[0][2])
                         - (TransposeMatrix[0][0] * TransposeMatrix[1][2]));

    invMatrix[2*3 + 2] = determinant *
                        ((TransposeMatrix[0][0] * TransposeMatrix[1][1])
                         - (TransposeMatrix[1][0] * TransposeMatrix[0][1]));

    return 1;
}

gctBOOL _computeWlimitByData(
    IN glsCONTEXT_PTR Context,
    IN GLint First,
    IN gctUINT Count,
    IN gctFLOAT* Matrix,
    IN gctFLOAT ZNear,
    IN gctUINT IndexType,
    IN const GLvoid* Indices
    )
{
    gctFLOAT limit = 0.0f;
    gctFLOAT wLimit = 0.0f;
    gctFLOAT fBound = 8388608.0f;
    gctUINT j,i,elementSize = 0;
    gctBOOL ok = gcvFALSE;
    gctFLOAT * mvp = Matrix;
    gctFLOAT zNear = gcoMATH_Absolute(ZNear);
    gctFLOAT_PTR vertexPtr = 0;
    gctPOINTER indexPtr = 0;
    gctUINT component = 0;
    gctBOOL disableWlimit = gcvTRUE;
    gctUINT  sampleCount;
    gctUINT  sampleStep;
    gctUINT wlimitVertexStride;
    gctBOOL indexDraw = gcvFALSE;
    gceVERTEX_FORMAT vertexFormat;

    wlimitVertexStride = Context->aPositionInfo.stride;
    component =  Context->aPositionInfo.components;

    vertexFormat = Context->aPositionInfo.format;

    /* For now, only float vertex */
    if (vertexFormat != gcvVERTEX_FLOAT)
        return gcvFALSE;

    if(zNear == 0.0f)
        return gcvFALSE;

    if(component <= 2)
    {
        gco3D_SetWClipEnable(Context->hw, gcvFALSE);
        return gcvTRUE;
    }

    if(Context->aPositionInfo.buffer == gcvNULL)
    {
        /* Vertex array */
        vertexPtr =  (gctFLOAT_PTR)((gctUINT8_PTR)Context->aPositionInfo.pointer + wlimitVertexStride * First);
    }
    else
    {
        gctPOINTER memory = gcvNULL;

        glsBUFFER_PTR object = (glsBUFFER_PTR)(Context->aPositionInfo.buffer->object);
        gcoSTREAM_Lock(object->stream, &memory,gcvNULL);
        vertexPtr =  (gctFLOAT_PTR)((gctUINTPTR_T)memory + (gctUINTPTR_T)Context->aPositionInfo.pointer
                                     + wlimitVertexStride * First);
        gcoSTREAM_Unlock(object->stream);
    }

    switch(IndexType)
    {
    case GL_UNSIGNED_BYTE:
        elementSize = sizeof(GLubyte);
        indexDraw = gcvTRUE;
        break;

    case GL_UNSIGNED_SHORT:
        elementSize = sizeof(GLushort);
        indexDraw = gcvTRUE;
        break;

    case GL_UNSIGNED_INT:
        elementSize = sizeof(GLuint);
        indexDraw = gcvTRUE;
        break;

    default:
        indexPtr = 0;
    }

    if (indexDraw)
    {
        glsNAMEDOBJECT_PTR elementArrayBuffer = Context->elementArrayBuffer;

        if(elementArrayBuffer == gcvNULL)
        {
            indexPtr =  (gctPOINTER)Indices;
        }
        else
        {
            gctPOINTER memory = gcvNULL;

            glsBUFFER_PTR object = (glsBUFFER_PTR)elementArrayBuffer->object;
            gcoINDEX_Lock(object->index, gcvNULL, &memory);
            indexPtr = (gctPOINTER)((gctUINTPTR_T)memory + (gctUINTPTR_T)Indices);
            gcoINDEX_Unlock(object->index);
        }
    }
    sampleCount = Context->wLimitSampleCount > Count ? Count : Context->wLimitSampleCount;
    sampleStep  = Count / sampleCount;

    for(i=0; i < Count; i += sampleStep)
    {
        gctFLOAT vector[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        gctFLOAT x, y, z, w,absX,absY,absW;
        gctINT vertex;
        gctFLOAT refZ = 0.0f;
        gctFLOAT refW = 0.0f;
        gctFLOAT_PTR vertexPtTmp = 0;

        if (indexDraw && indexPtr)
        {
            switch(IndexType)
            {
            case GL_UNSIGNED_BYTE:
                vertex = *(gctUINT8 *)((gctUINT8_PTR)indexPtr + i * elementSize);
                break;

            case GL_UNSIGNED_SHORT:
                vertex = *(gctUINT16 *)((gctUINT8_PTR)indexPtr + i * elementSize);
                break;

            case GL_UNSIGNED_INT:
                vertex = *(gctUINT *)((gctUINT8_PTR)indexPtr + i * elementSize);
                break;

            default:
                return gcvFALSE;
            }
        }
        else
        {
             vertex = i;
        }

        vertexPtTmp = (gctFLOAT_PTR)((gctUINT8_PTR)vertexPtr + wlimitVertexStride * vertex);

        /* Get Value */

        if((((gcmPTR2INT(vertexPtTmp)) & 3) == 0))
        {
            for(j=0; j < component; j++)
            {
                vector[j] = vertexPtTmp[j];
            }
        }
        else
        {
            for(j=0; j < component; j++)
            {
                readDataForWLimit(vertexPtTmp + j, vector[j])
            }
        }


        x = mvp[ 0] * vector[0]
          + mvp[ 4] * vector[1]
          + mvp[ 8] * vector[2]
          + mvp[12] * vector[3];

        y = mvp[ 1] * vector[0]
          + mvp[ 5] * vector[1]
          + mvp[ 9] * vector[2]
          + mvp[13] * vector[3];

        z = mvp[ 2] * vector[0]
          + mvp[ 6] * vector[1]
          + mvp[10] * vector[2]
          + mvp[14] * vector[3];

        w = mvp[ 3] * vector[0]
          + mvp[ 7] * vector[1]
          + mvp[11] * vector[2]
          + mvp[15] * vector[3];

        if(i == 0)
        {
            refZ = z;
            refW = w;
        }
        else if(!(refZ == z && refW == w))
        {
            disableWlimit = gcvFALSE;
        }

        absX = gcoMATH_Absolute(x);
        absY = gcoMATH_Absolute(y);

        absW = gcoMATH_Absolute(w);

        if(absX < absW && absY < absW && z > 0.0f)
        {
            /* Do nothing */
        }
        else if(z > 0.0f && w > 0)
        {
            gctFLOAT xMax = absX / absW * Context->viewportStates.viewportBox[2] / 2.0f;
            gctFLOAT yMax = absY / absW * Context->viewportStates.viewportBox[3] / 2.0f;

            if(xMax > fBound || yMax > fBound)
            {
                gctFLOAT max = xMax > yMax ? xMax : yMax;

                limit = max * absW / ((1 << 22) - 1);
            }
        }
        else if(z < 0.0f)
        {
            gctFLOAT xMax = absX / zNear * Context->viewportStates.viewportBox[2] / 2.0f;
            gctFLOAT yMax = absY / zNear * Context->viewportStates.viewportBox[3] / 2.0f;

            if(xMax > fBound || yMax > fBound)
            {
                gctFLOAT max = xMax > yMax ? xMax : yMax;

                limit = max * zNear / ((1 << 22) - 1);
            }

            if(w < 0.0f)
            {
                gctFLOAT wMin = absW > 5.0f ? 5.0f : absW;
                gctFLOAT wMinLimit = 0.0f;

                xMax = absX / wMin * Context->viewportStates.viewportBox[2] / 2.0f;
                yMax = absY / wMin * Context->viewportStates.viewportBox[3] / 2.0f;

                if(xMax > fBound || yMax > fBound)
                {
                    gctFLOAT max = xMax > yMax ? xMax : yMax;

                    wMinLimit = max * wMin / ((1 << 22) - 1);
                }

                limit = limit > wMinLimit ? limit : wMinLimit;
            }
        }
        else
        {
            gcmTRACE(gcvLEVEL_WARNING,"wlimit can't handle this issue");
        }

        if(limit > wLimit)
        {
            wLimit = limit;
            ok = gcvTRUE;
        }
    }

    if(disableWlimit)
    {
        gco3D_SetWClipEnable(Context->hw, gcvFALSE);
        return gcvTRUE;
    }
    else if (ok)
    {
        gco3D_SetWPlaneLimitF(Context->hw, wLimit);
        gco3D_SetWClipEnable(Context->hw, gcvTRUE);
        return gcvTRUE;
    }
    else
    {
        gco3D_SetWClipEnable(Context->hw, gcvFALSE);
    }

    return gcvFALSE;
}

void _fixWlimit(
                IN glsCONTEXT_PTR Context,
                IN GLint First,
                IN gctUINT Count,
                IN GLenum Type,
                IN const GLvoid* Indices
                )
{
    gctFLOAT wLimit = 0;
    gctFLOAT zNear = 0;
    gctFLOAT* matrix;
    gctFLOAT invMatrix[9];
    glsMATRIX_PTR mat;
    gctBOOL recompute;

    /*No vertex enable, no wlimit*/
    if(!Context->aPositionInfo.streamEnabled)
        return;
    recompute = Context->modelViewProjectionMatrix.recompute;

    mat = glfGetModelViewProjectionMatrix(Context);
    matrix = mat->value;

    if(recompute)
    {
        if (!_invertMatrix(matrix, invMatrix))
        {
            zNear = 0;
        }
        else
        {
            gctFLOAT Xmv, Ymv, Zmv;

            Xmv = -1 * (invMatrix[0*3 + 0]*matrix[0*4 + 3] + invMatrix[0*3 + 1]*matrix[1*4 + 3] + invMatrix[0*3 + 2]*matrix[2*4 + 3]);
            Ymv = -1 * (invMatrix[1*3 + 0]*matrix[0*4 + 3] + invMatrix[1*3 + 1]*matrix[1*4 + 3] + invMatrix[1*3 + 2]*matrix[2*4 + 3]);
            Zmv = -1 * (invMatrix[2*3 + 0]*matrix[0*4 + 3] + invMatrix[2*3 + 1]*matrix[1*4 + 3] + invMatrix[2*3 + 2]*matrix[2*4 + 3]);

            zNear = (matrix[3*4 + 0] * Xmv) + (matrix[3*4 + 1] * Ymv) + (matrix[3*4 + 2] * Zmv) + matrix[3*4 + 3];
        }
        Context->zNear = zNear;
    }
    else
    {
        zNear = Context->zNear;
    }

    if (Context->bComputeWlimitByVertex && !Context->drawTexOESEnabled)
    {
        /* TODO, check vbo or vertex array changed */
        if (_computeWlimitByData(Context, First, Count, matrix, zNear, Type, Indices))
        {
            return;
        }
    }

    if (recompute)
    {
        if (zNear != 0)
        {
            gctFLOAT wMinX, wMinY, wMin;
            gctFLOAT xMax, yMax;
            gctFLOAT MaxCoordClip;
            gcePATCH_ID patchId = gcvPATCH_INVALID;

            gcoHAL_GetPatchID(gcvNULL, &patchId);

            /* Compute wLimit for X. */
            xMax = gcoMATH_Absolute(matrix[0])
                + gcoMATH_Absolute(matrix[4])
                + gcoMATH_Absolute(matrix[8]);

            MaxCoordClip = xMax / zNear;
            wMinX = (Context->drawWidth/2) * MaxCoordClip;

            /* Compute wLimit for Y. */
            yMax = gcoMATH_Absolute(matrix[1])
                + gcoMATH_Absolute(matrix[5])
                + gcoMATH_Absolute(matrix[9]);

            MaxCoordClip = yMax / zNear;

            /* Compute max wMin. */
            wMinY = (Context->drawHeight/2) * MaxCoordClip;

            wMin = gcmMAX(wMinX, wMinY);

            if (wMin > ((1 << 22) - 1) || (patchId == gcvPATCH_SMARTBENCH))
            {
                wLimit = wMin * zNear / ((1 << 22) - 1);

                gco3D_SetWPlaneLimitF(Context->hw, wLimit);
                gco3D_SetWClipEnable(Context->hw, gcvTRUE);
            }
            else
            {
                gco3D_SetWClipEnable(Context->hw, gcvFALSE);
            }
        }
        else
        {
            gco3D_SetWClipEnable(Context->hw, gcvFALSE);
        }
    }
}

/******************************************************************************\
************************* OpenGL Primitive Drawing Code ************************
\******************************************************************************/
gceSTATUS
glfDrawArrays(
    glsCONTEXT_PTR Context,
    glsINSTANT_DRAW_PTR instantDraw
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL lineLoopPatch = gcvFALSE;
    gctBOOL instanceDraw = gcvFALSE;
    gctINT i = 0;

    gcmHEADER();

    /* Test for line loop patch. */
    lineLoopPatch = (instantDraw->primMode == gcvPRIMITIVE_LINE_LOOP) && !gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_LINE_LOOP);

    /* Setup the vertex array. */
    gcmONERROR(_VertexArray(Context,
                            instantDraw->first,
                            &instantDraw->count,
                            gcvINDEX_8, gcvNULL, gcvNULL,
                            &instantDraw->primMode,
                            (gctUINT *)(&instantDraw->primCount)));

    instanceDraw = gcoHAL_IsFeatureAvailable(gcvNULL,gcvFEATURE_HALTI5);

    /* LOGICOP enabled? */
    if (Context->logicOp.perform)
    {
        /* In LOGICOP we have to do one primitive at a time. */
        for (i = 0; i < instantDraw->primCount ; i++)
        {
            /* Create temporary target as destination. */
            gcmONERROR(_LogicOpPreProcess(Context));

            if (lineLoopPatch)
            {
                if (instanceDraw)
                {
                    /* Draw the primitives. */
                    gcmONERROR(gco3D_DrawInstancedPrimitives(Context->hw,
                                                             gcvPRIMITIVE_LINE_STRIP,
                                                             gcvTRUE, i, 0, 1, 2,
                                                             1
                                                             ));
                }
                else
                {
                    /* Draw the primitives. */
                    gcmONERROR(gco3D_DrawIndexedPrimitives(Context->hw,
                                                           gcvPRIMITIVE_LINE_STRIP,
                                                           0, i, 1
                                                           ));
                }
            }
            else
            {
                if (instanceDraw)
                {
                    /* Draw the primitives. */
                    gcmONERROR(gco3D_DrawInstancedPrimitives(Context->hw,
                                                             instantDraw->primMode,
                                                             gcvFALSE, instantDraw->first + i, 0, 1, 3, /* Point,line,triangle all send count = 3 */
                                                             1
                                                             ));
                }
                else
                {
                    /* Draw the primitives. */
                    gcmONERROR(gco3D_DrawPrimitives(Context->hw,
                                                    instantDraw->primMode,
                                                    instantDraw->first + i, 1
                                                    ));
                }
            }

            /* Run the post processing pass. */
            gcmONERROR(_LogicOpPostProcess(Context));
        }
    }
    else
    {
        if (lineLoopPatch)
        {
            if (instanceDraw)
            {
                /* Draw the primitives. */
                gcmONERROR(gco3D_DrawInstancedPrimitives(Context->hw,
                                                         gcvPRIMITIVE_LINE_STRIP,
                                                         gcvTRUE, 0, 0, instantDraw->primCount, instantDraw->count,
                                                         1
                                                         ));
            }
            else
            {
                /* Draw the primitives. */
                gcmONERROR(gco3D_DrawIndexedPrimitives(Context->hw,
                                                       gcvPRIMITIVE_LINE_STRIP,
                                                       0, 0, instantDraw->primCount
                                                       ));
            }
        }
        else
        {
            if (instanceDraw)
            {
                /* Draw the primitives. */
                gcmONERROR(gco3D_DrawInstancedPrimitives(Context->hw,
                                                         instantDraw->primMode,
                                                         gcvFALSE, instantDraw->first, 0, instantDraw->primCount, instantDraw->count,
                                                         1
                                                         ));
            }
            else
            {
                /* Draw the primitives. */
                gcmONERROR(gco3D_DrawPrimitives(Context->hw,
                                                instantDraw->primMode,
                                                instantDraw->first, instantDraw->primCount
                                                ));
            }
        }
    }

    gcmFOOTER();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
glfCollectSplitDrawArraysInfo(
    IN glsCONTEXT_PTR Context,
    IN glsINSTANT_DRAW_PTR instantDraw,
    IN OUT gcsSPLIT_DRAW_INFO_PTR splitDrawInfo
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    /* Fake line loop path, converte triangle list can move the this level.
    ** now, there is no split draw path for draw array.*/
    return status;
}

#define SPILIT_INDEX_OFFSET       48
#define SPILIT_INDEX_CHUNCK_BYTE  64

gceSTATUS
glfSplitIndexFetch(
    IN glsINSTANT_DRAW_PTR instantDraw,
    IN OUT gcsSPLIT_DRAW_INFO_PTR splitDrawInfo
    )
{
    gceSTATUS status = gcvSTATUS_TRUE;
    gctUINT32 address = 0, tempAddress;
    gctUINT32 indexSize = 0;
    gctUINT32 spilitIndexMod;
    gctSIZE_T cutCount = 0;

    gcmHEADER();

    gcmASSERT(instantDraw->indexBuffer != gcvNULL);

    gcmGET_INDEX_SIZE(instantDraw->indexType, indexSize);
    gcmONERROR(gcoINDEX_Lock(instantDraw->indexBuffer, &address, gcvNULL));
    /* Add offset */
    address += gcmPTR2INT32(instantDraw->indexMemory);
    /* Unlock the bufobj buffer. */
    gcmONERROR(gcoINDEX_Unlock(instantDraw->indexBuffer));

    if (instantDraw->primMode == gcvPRIMITIVE_TRIANGLE_LIST)
    {
        cutCount = instantDraw->count % 3;
    }
    else if (instantDraw->primMode == gcvPRIMITIVE_LINE_LIST)
    {
        cutCount = instantDraw->count % 2;
    }

    /* compute the last index address.*/
    tempAddress = address + (gctUINT32)(instantDraw->count-cutCount-1) * indexSize;
    spilitIndexMod = tempAddress % SPILIT_INDEX_CHUNCK_BYTE;

    if (spilitIndexMod >= SPILIT_INDEX_OFFSET)
    {
        gcmFOOTER();
        return gcvSTATUS_FALSE;
    }

    /* Get primMode and split count.*/
    switch (instantDraw->primMode)
    {
        case gcvPRIMITIVE_POINT_LIST:
        case gcvPRIMITIVE_LINE_STRIP:
        case gcvPRIMITIVE_TRIANGLE_STRIP:
        case gcvPRIMITIVE_TRIANGLE_FAN:
            splitDrawInfo->u.info_index_fetch.splitCount = spilitIndexMod /indexSize +1;
            break;
        case gcvPRIMITIVE_LINE_LOOP:
            splitDrawInfo->u.info_index_fetch.splitCount = spilitIndexMod /indexSize +1;
            break;
        case gcvPRIMITIVE_TRIANGLE_LIST:
            splitDrawInfo->u.info_index_fetch.splitCount = ((spilitIndexMod /(indexSize*3))+1)*3 + cutCount;
            break;
        case gcvPRIMITIVE_LINE_LIST:
            splitDrawInfo->u.info_index_fetch.splitCount = ((spilitIndexMod /(indexSize*2))+1)*2 + cutCount;
            break;
        default:
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmFOOTER();
    return gcvSTATUS_TRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
glfCopySpilitIndex(
    IN glsINSTANT_DRAW_PTR instantDraw,
    IN OUT gcsSPLIT_DRAW_INFO_PTR splitDrawInfo,
    IN OUT gctPOINTER * Buffer
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctPOINTER indexBase = gcvNULL;
    gctPOINTER tempIndices = gcvNULL;
    gctPOINTER indexMemory = gcvNULL;
    gctSIZE_T count, primCount, i, j, bytes;
    gctSIZE_T indexSize = 0;
    gctSIZE_T offset = 0;
    gcePATCH_ID patchId = gcvPATCH_INVALID;

    gcmHEADER();

    gcmGET_INDEX_SIZE(instantDraw->indexType, indexSize);
    offset = (instantDraw->count - splitDrawInfo->u.info_index_fetch.splitCount) * indexSize;
    gcoHAL_GetPatchID(gcvNULL, &patchId);

    /* Lock the index buffer. */
#if gcdSYNC
    if (patchId == gcvPATCH_GTFES30)
    {
        gcoINDEX_WaitFence(instantDraw->indexBuffer, gcvFENCE_TYPE_WRITE);
    }
#endif
    gcmONERROR(gcoINDEX_Lock(instantDraw->indexBuffer, gcvNULL, &indexBase));
    indexBase = (gctUINT8_PTR)indexBase + gcmPTR2INT32(instantDraw->indexMemory);
    gcmONERROR(gcoINDEX_Unlock(instantDraw->indexBuffer));
    bytes = splitDrawInfo->u.info_index_fetch.splitCount * indexSize;

    splitDrawInfo->u.info_index_fetch.splitPrimMode = instantDraw->primMode;

    switch (instantDraw->primMode)
    {
    case gcvPRIMITIVE_POINT_LIST:
    case gcvPRIMITIVE_LINE_LIST:
    case gcvPRIMITIVE_TRIANGLE_LIST:
        {
            indexMemory = (gctUINT8_PTR)indexBase + offset;

            gcmONERROR(gcoOS_Allocate(gcvNULL,
                                      bytes,
                                      &tempIndices));
            gcoOS_MemCopy(tempIndices , indexMemory, bytes);
        }
        break;
    case gcvPRIMITIVE_LINE_STRIP:
        {
            indexMemory = (gctUINT8_PTR)indexBase + offset - indexSize;
            bytes += indexSize;
            gcmONERROR(gcoOS_Allocate(gcvNULL,
                                      bytes,
                                      &tempIndices));
            /* line strip need copy the last data */
            gcoOS_MemCopy(tempIndices , indexMemory, bytes);
        }
        break;
    case gcvPRIMITIVE_LINE_LOOP:
        {
            indexMemory = (gctUINT8_PTR)indexBase + offset - indexSize;
            bytes += 2 * indexSize;
            gcmONERROR(gcoOS_Allocate(gcvNULL,
                                      bytes,
                                      &tempIndices));

            /* line loop need copy the last data and the first data */
            gcoOS_MemCopy(tempIndices , indexMemory, bytes - indexSize);
            gcoOS_MemCopy((gctUINT8_PTR)tempIndices + bytes - indexSize, indexBase, indexSize);
            splitDrawInfo->u.info_index_fetch.splitPrimMode = gcvPRIMITIVE_LINE_STRIP;
        }
        break;
    case gcvPRIMITIVE_TRIANGLE_STRIP:
        {
            primCount = bytes / indexSize;
            bytes = 3 * primCount * indexSize;
            gcmONERROR(gcoOS_Allocate(gcvNULL,
                                      bytes,
                                      &tempIndices));
            count = offset / indexSize;
            switch (instantDraw->indexType)
            {
            case gcvINDEX_8:
                {
                    gctUINT8_PTR src = (gctUINT8_PTR)indexBase;
                    gctUINT8_PTR dst = (gctUINT8_PTR)tempIndices;
                    for(i = 0, j = count - 2; i < primCount; i++, j++)
                    {
                        dst[i * 3]     = src[(j % 2) == 0? j : j + 1];
                        dst[i * 3 + 1] = src[(j % 2) == 0? j + 1 : j];
                        dst[i * 3 + 2] = src[j + 2];
                    }
                }
                break;
            case gcvINDEX_16:
                {
                    gctUINT16_PTR src = (gctUINT16_PTR)indexBase;
                    gctUINT16_PTR dst = (gctUINT16_PTR)tempIndices;
                    for(i = 0, j = count - 2; i < primCount; i++, j++)
                    {
                        dst[i * 3]     = src[(j % 2) == 0? j : j + 1];
                        dst[i * 3 + 1] = src[(j % 2) == 0? j + 1 : j];
                        dst[i * 3 + 2] = src[j + 2];
                    }
                }
                break;
            case gcvINDEX_32:
                {
                    gctUINT32_PTR src = (gctUINT32_PTR)indexBase;
                    gctUINT32_PTR dst = (gctUINT32_PTR)tempIndices;
                    for(i = 0, j = count - 2; i < primCount; i++, j++)
                    {
                        dst[i * 3]     = src[(j % 2) == 0? j : j + 1];
                        dst[i * 3 + 1] = src[(j % 2) == 0? j + 1 : j];
                        dst[i * 3 + 2] = src[j + 2];
                    }
                }
                break;
            default:
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }
            splitDrawInfo->u.info_index_fetch.splitPrimMode = gcvPRIMITIVE_TRIANGLE_LIST;
        }
        break;
    case gcvPRIMITIVE_TRIANGLE_FAN:
        {
            indexMemory = (gctUINT8_PTR)indexBase + offset - indexSize;
            bytes += 2 * indexSize;
            gcmONERROR(gcoOS_Allocate(gcvNULL,
                                      bytes,
                                      &tempIndices));

            /* trianglefan need copy the first data */
            gcoOS_MemCopy(tempIndices , indexBase, indexSize);
            gcoOS_MemCopy((gctUINT8_PTR)tempIndices + indexSize, indexMemory, bytes - indexSize);
        }
        break;
    default:
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    splitDrawInfo->u.info_index_fetch.splitCount = bytes / indexSize;

    /* Translate primitive count. */
    switch (splitDrawInfo->u.info_index_fetch.splitPrimMode)
    {
    case gcvPRIMITIVE_POINT_LIST:
        splitDrawInfo->u.info_index_fetch.splitPrimCount = splitDrawInfo->u.info_index_fetch.splitCount;
        break;

    case gcvPRIMITIVE_LINE_LIST:
        splitDrawInfo->u.info_index_fetch.splitPrimCount = splitDrawInfo->u.info_index_fetch.splitCount / 2;
        break;

    case gcvPRIMITIVE_LINE_LOOP:
        splitDrawInfo->u.info_index_fetch.splitPrimCount = splitDrawInfo->u.info_index_fetch.splitCount;
        break;

    case gcvPRIMITIVE_LINE_STRIP:
        splitDrawInfo->u.info_index_fetch.splitPrimCount = splitDrawInfo->u.info_index_fetch.splitCount - 1;
        break;

    case gcvPRIMITIVE_TRIANGLE_LIST:
        splitDrawInfo->u.info_index_fetch.splitPrimCount = splitDrawInfo->u.info_index_fetch.splitCount / 3;
        break;

    case gcvPRIMITIVE_TRIANGLE_STRIP:
    case gcvPRIMITIVE_TRIANGLE_FAN:
        splitDrawInfo->u.info_index_fetch.splitPrimCount = splitDrawInfo->u.info_index_fetch.splitCount - 2;
        break;

    default:
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    *Buffer = tempIndices;

OnError:

    /* Return the status. */
    gcmFOOTER();
    return status;
}

#define gcmES11_COLLECT_STREAM_INFO(streamInfo, instantDraw) \
    streamInfo.attribMask = instantDraw->attribMask; \
    streamInfo.u.es11.attributes = instantDraw->attributes; \
    streamInfo.first = instantDraw->first; \
    streamInfo.count = instantDraw->count; \
    streamInfo.instanced = gcvFALSE; \
    streamInfo.instanceCount = 1; \
    streamInfo.primMode = instantDraw->primMode; \
    streamInfo.primCount = instantDraw->primCount

#define gcmES11_COLLECT_INDEX_INFO(indexInfo, instantDraw) \
    indexInfo.count = instantDraw->count; \
    indexInfo.indexType = instantDraw->indexType; \
    indexInfo.u.es11.indexBuffer = instantDraw->indexBuffer; \
    indexInfo.indexMemory = instantDraw->indexMemory

gceSTATUS
glfSplitDrawIndexFetch(
    IN gctPOINTER Context,
    IN gctPOINTER InstantDraw,
    IN gctPOINTER SplitDrawInfo
    )
{
    gceSTATUS status                     = gcvSTATUS_OK;
    glsCONTEXT_PTR context               = (glsCONTEXT_PTR)(Context);
    glsINSTANT_DRAW_PTR instantDraw      = (glsINSTANT_DRAW_PTR)(InstantDraw);
    gcsSPLIT_DRAW_INFO_PTR splitDrawInfo = (gcsSPLIT_DRAW_INFO_PTR)(SplitDrawInfo);

    gcePATCH_ID patchId = gcvPATCH_INVALID;
    gctPOINTER splitIndexMemory = gcvNULL;
    gctBOOL bAllocate = gcvFALSE;
    glsINSTANT_DRAW tmpInstantDraw;
    gctBOOL instanceDraw = gcvFALSE;
    gcsVERTEXARRAY_STREAM_INFO streamInfo;
    gcsVERTEXARRAY_INDEX_INFO  indexInfo;

    gcmHEADER();

    gcmONERROR(_computeAttribMask(context, &instantDraw->attribMask));
    instantDraw->attributes = context->attributeArray;
    /* Stream data not change, only need bind once.*/
    /* Collect info for hal level.*/
    gcmES11_COLLECT_STREAM_INFO(streamInfo, instantDraw);
    gcmES11_COLLECT_INDEX_INFO(indexInfo, instantDraw);

#if gcdUSE_WCLIP_PATCH
    gcmONERROR(gcoVERTEXARRAY_StreamBind_Ex(context->vertexArray,
                                            gcvNULL,
                                            gcvNULL,
                                            &streamInfo,
                                            &indexInfo));
#else
    gcmONERROR(gcoVERTEXARRAY_StreamBind_Ex(chipCtx->vertexArray,
                                            &streamInfo,
                                            &indexInfo));
#endif

    /************************************************************************************
    **              first draw
    ************************************************************************************/
    gcoOS_MemCopy(&tmpInstantDraw, instantDraw, sizeof(glsINSTANT_DRAW));
    /* es11 driver will update streaminfo.*/
    tmpInstantDraw.primMode = streamInfo.primMode;
    tmpInstantDraw.primCount = (gctINT)streamInfo.primCount;

    gcoHAL_GetPatchID(gcvNULL, &patchId);

#if gcdSYNC
    if (patchId == gcvPATCH_GTFES30)
    {
        gcoINDEX_WaitFence(instantDraw->indexBuffer, gcvFENCE_TYPE_WRITE);
    }
#endif
    if ((gctUINT)instantDraw->count <= splitDrawInfo->u.info_index_fetch.splitCount)
    {
        tmpInstantDraw.count = 0;
    }
    else
    {
        tmpInstantDraw.count = instantDraw->count - (gctINT)splitDrawInfo->u.info_index_fetch.splitCount;
        if (instantDraw->primMode == gcvPRIMITIVE_LINE_LOOP)
        {
            tmpInstantDraw.primMode = gcvPRIMITIVE_LINE_STRIP;
        }
    }

    instanceDraw = gcoHAL_IsFeatureAvailable(gcvNULL,gcvFEATURE_HALTI5);

    if (tmpInstantDraw.count > 0)
    {
        /* Update index */
        indexInfo.count = tmpInstantDraw.count;
        gcmONERROR(gcoVERTEXARRAY_IndexBind_Ex(context->vertexArray,
                                               &streamInfo,
                                               &indexInfo));

        if (instanceDraw)
        {
            /* Draw */
            gcmONERROR(gco3D_DrawInstancedPrimitives(context->hw,
                                                    tmpInstantDraw.primMode,
                                                    gcvTRUE,
                                                    tmpInstantDraw.first,
                                                    0,
                                                    tmpInstantDraw.primCount,
                                                    tmpInstantDraw.count,
                                                    1));
        }
        else
        {
            /* For es11, non instance draw need update primCount.*/
            gcmONERROR(glfGetPrimitiveCount(tmpInstantDraw.primMode, tmpInstantDraw.count, &tmpInstantDraw.primCount));
            /* Draw the primitives. */
            gcmONERROR(gco3D_DrawIndexedPrimitives(context->hw,
                                                   tmpInstantDraw.primMode,
                                                   0,
                                                   0,
                                                   tmpInstantDraw.primCount
                                                   ));
        }
    }

    /************************************************************************************
    **              second draw
    ************************************************************************************/
    gcoOS_MemCopy(&tmpInstantDraw, instantDraw, sizeof(glsINSTANT_DRAW));
    /* es11 driver will update streaminfo.*/
    tmpInstantDraw.primMode = streamInfo.primMode;
    tmpInstantDraw.primCount = (gctINT)streamInfo.primCount;

    if ((gctUINT)instantDraw->count <= splitDrawInfo->u.info_index_fetch.splitCount)
    {
        /* Already lock when collect info.*/
        gcmONERROR(gcoINDEX_Lock(instantDraw->indexBuffer, gcvNULL, &splitIndexMemory));
        splitIndexMemory =(gctUINT8_PTR)splitIndexMemory + gcmPTR2INT32(instantDraw->indexMemory);
        gcmONERROR(gcoINDEX_Unlock(instantDraw->indexBuffer));
        tmpInstantDraw.count = instantDraw->count;
    }
    else
    {
        gcmONERROR(glfCopySpilitIndex(&tmpInstantDraw,
                                      splitDrawInfo,
                                      &splitIndexMemory));
        bAllocate = gcvTRUE;
        tmpInstantDraw.count = (gctINT)splitDrawInfo->u.info_index_fetch.splitCount;
        tmpInstantDraw.primMode = splitDrawInfo->u.info_index_fetch.splitPrimMode;
        tmpInstantDraw.primCount = (gctINT)splitDrawInfo->u.info_index_fetch.splitPrimCount;
    }
    /* set tmpInstantDraw.*/
    tmpInstantDraw.indexMemory = splitIndexMemory;
    tmpInstantDraw.indexBuffer = gcvNULL;

    /* Update index */
    indexInfo.count = tmpInstantDraw.count;
    indexInfo.indexMemory = tmpInstantDraw.indexMemory;
    indexInfo.u.es11.indexBuffer = tmpInstantDraw.indexBuffer;
    gcmONERROR(gcoVERTEXARRAY_IndexBind_Ex(context->vertexArray,
                                           &streamInfo,
                                           &indexInfo));

    /* Draw */
    if (instanceDraw)
    {
        /* Draw */
        gcmONERROR(gco3D_DrawInstancedPrimitives(context->hw,
                                                tmpInstantDraw.primMode,
                                                gcvTRUE,
                                                tmpInstantDraw.first,
                                                0,
                                                tmpInstantDraw.primCount,
                                                tmpInstantDraw.count,
                                                1));
    }
    else
    {
        /* Draw the primitives. */
        gcmONERROR(gco3D_DrawIndexedPrimitives(context->hw,
                                               tmpInstantDraw.primMode,
                                               0,
                                               0,
                                               tmpInstantDraw.primCount
                                               ));
    }


OnError:
    if (bAllocate && splitIndexMemory != gcvNULL)
    {
        gcmOS_SAFE_FREE(context->os, splitIndexMemory);
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
glfCollectSplitDrawElementInfo(
    IN glsCONTEXT_PTR Context,
    IN glsINSTANT_DRAW_PTR instantDraw,
    IN OUT gcsSPLIT_DRAW_INFO_PTR splitDrawInfo
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER();

    if ((gcoHAL_IsFeatureAvailable(gcvNULL,gcvFEATURE_INDEX_FETCH_FIX) != gcvSTATUS_TRUE)
    &&  instantDraw->indexBuffer != gcvNULL
    &&  (gcvSTATUS_TRUE == glfSplitIndexFetch(instantDraw, splitDrawInfo))
    )
    {
        splitDrawInfo->splitDrawType = gcvSPLIT_DRAW_INDEX_FETCH;
        splitDrawInfo->splitDrawFunc = glfSplitDrawIndexFetch;

        gcmFOOTER();
        return gcvSTATUS_OK;
    }

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  glDrawArrays
**
**  glDrawArrays specifies multiple geometric primitives with very few
**  subroutine calls. Instead of calling a GL procedure to pass each individual
**  vertex, normal, texture coordinate, edge flag, or color, you can prespecify
**  separate arrays of vertices, normals, and colors and use them to construct
**  a sequence of primitives with a single call to glDrawArrays.
**
**  When glDrawArrays is called, it uses count sequential elements from each
**  enabled array to construct a sequence of geometric primitives, beginning
**  with element first. mode specifies what kind of primitives are constructed
**  and how the array elements construct those primitives. If GL_VERTEX_ARRAY
**  is not enabled, no geometric primitives are generated.
**
**  Vertex attributes that are modified by glDrawArrays have an unspecified
**  value after glDrawArrays returns. For example, if GL_COLOR_ARRAY is enabled,
**  the value of the current color is undefined after glDrawArrays executes.
**  Attributes that aren't modified remain well defined.
**
**  INPUT:
**
**      Mode
**          Specifies what kind of primitives to render. Symbolic constants
**          GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP, GL_LINES, GL_TRIANGLE_STRIP,
**          GL_TRIANGLE_FAN and GL_TRIANGLES are accepted.
**
**      First
**          Specifies the starting index in the enabled arrays.
**
**      Count
**          Specifies the number of indices to be rendered.
**
**  OUTPUT:
**
**      Nothing.
*/

GL_API void GL_APIENTRY glDrawArrays(
    GLenum Mode,
    GLint First,
    GLsizei Count
    )
{
    gceSTATUS status          = gcvSTATUS_OK;
    GLboolean flippedTextures = GL_FALSE;
    glsINSTANT_DRAW instantDraw;
    gcsSPLIT_DRAW_INFO splitDrawInfo;

    glmENTER3(glmARGENUM, Mode, glmARGINT, First, glmARGINT, Count)
    {
#if VIVANTE_PROFILER && VIVANTE_PROFILER_PERDRAW
        glmPROFILE(context, GLES1_DRAWARRAYS, 0);
        _glffProfiler(&context->profiler, GL1_PROFILER_DRAW_BEGIN, 0);
#endif
        glmPROFILE(context, GLES1_DRAWARRAYS, 0);
        if (context->profiler.enable)
        {
            _glffProfiler_NEW_Set(context, GL1_PROFILER_DRAW_BEGIN, 0);
        }
        do
        {
#if gcdDUMP_API
            gcmDUMP_API("${ES11 glDrawArrays 0x%08X 0x%08X 0x%08X",
                        Mode, First, Count);
            _DumpVertices(context, First, Count);
            gcmDUMP_API("$}");
#endif

#if gcdDEBUG_OPTION && gcdDEBUG_OPTION_NO_GL_DRAWS
            {
                gcePATCH_ID patchId = gcvPATCH_INVALID;

                gcoHAL_GetPatchID(gcvNULL, &patchId);

                if (patchId == gcvPATCH_DEBUG)
                    break;
            }
#endif

            {
                gcePATCH_ID patchId;
                gcoHAL_GetPatchID(gcvNULL, &patchId);

                if(patchId == gcvPATCH_WATER2_CHUKONG && (Count == 0x10 || Count == 0x1A) && First == 0 )
                {
                    if(context->arrayBuffer )
                    {
                        glsBUFFER_PTR object;
                        object = (glsBUFFER_PTR ) context->arrayBuffer->object;

                        if(object->dataInvalid)
                            break;

                    }
                }

            }

            /* Check the count and first. */
            if ((Count < 0) || (First < 0) || (First + Count < 0))
            {
                glmERROR(GL_INVALID_VALUE);
                break;
            }

            /* init instantDraw.*/
            gcoOS_ZeroMemory(&instantDraw, gcmSIZEOF(glsINSTANT_DRAW));
            instantDraw.first = First;
            instantDraw.count = Count;
            instantDraw.primMode = gcvPRIMITIVE_POINT_LIST;

            /* Validate mode and determine the number of primitives. */
            if (!_GetPrimitiveCount(Mode,
                                    Count,
                                    (GLsizei*) &instantDraw.primCount,
                                    &instantDraw.primMode))
            {
                glmERROR(GL_INVALID_ENUM);
                break;
            }

            /* Don't do anything if primitive count is invalid. */
            if ((instantDraw.primCount <= 0)
                /* Or primitive is fully culled. */
            ||  _IsFullCulled(context, Mode)
                /* Or matrix palette is invalid. */
            ||  _InvalidPalette(context)
            )
            {
                break;
            }

            /* Count the primitives. */
            glmPROFILE(context, GL1_PROFILER_PRIMITIVE_TYPE,  (gctUINTPTR_T)instantDraw.primMode);
            glmPROFILE(context, GL1_PROFILER_PRIMITIVE_COUNT, (gctUINTPTR_T)instantDraw.primCount);

            /* Update stencil states. */
            gcmERR_BREAK(glfUpdateStencil(context));

            /* Update frame buffer. */
            gcmERR_BREAK(glfUpdateFrameBuffer(context));

            /* Check buffer preserve. */
            gcmERR_BREAK(glfUpdateBufferPreserve(context));

            /* Update clipping box. */
            gcmERR_BREAK(glfUpdateClpping(context));

            /* Completely clipped out? */
            if (context->viewportStates.clippedOut)
            {
                break;
            }

            gcmERR_BREAK(glfUpdateViewport(context));
            gcmERR_BREAK(glfUpdateCulling(context));

            /* Update polygon offset states. */
            gcmERR_BREAK(glfUpdatePolygonOffset(context));

            /* Update primitive info. */
            gcmERR_BREAK(glfUpdatePrimitveType(context, Mode));

            /* Update texture states. */
            gcmERR_BREAK(glfUpdateTextureStates(context));

            /* Flip bottom-top textures. */
            gcmERR_BREAK(_FlipBottomTopTextures(context, &flippedTextures));

            /* W-Limit patch needs to added BEFORE loading shader,
               as it checks if matrix states are dirty. */
            if (context->wLimitPatch)
            {
                _fixWlimit(context,First,Count,0,0);
            }

            if (context->useFragmentProcessor)
            {
                /* Program the fragment processor. */
                gcmERR_BREAK(glfUpdateFragmentProcessor(context));
            }
            else
            {
                /* Load shader program. */
                gcmERR_BREAK(glfLoadShader(context));
            }

            /* Load texture states. */
            gcmERR_BREAK(glfLoadTexture(context));

            /* Collect split draw info.*/
            gcoOS_ZeroMemory(&splitDrawInfo, sizeof(gcsSPLIT_DRAW_INFO));
            gcmERR_BREAK(glfCollectSplitDrawArraysInfo(context, &instantDraw, &splitDrawInfo));

            if (splitDrawInfo.splitDrawType != gcvSPLIT_DRAW_UNKNOWN)
            {
                gcmERR_BREAK((*splitDrawInfo.splitDrawFunc)(context, &instantDraw, &splitDrawInfo));
            }
            else
            {
                gcmERR_BREAK(glfDrawArrays(context, &instantDraw));
            }

            /* Restore flipped bottom-top textures. */
            if (flippedTextures)
            {
                gcmERR_BREAK(_FlipBottomTopTextures(context, &flippedTextures));
            }

#if defined(ANDROID) && gldENABLE_MEMORY_REDUCTION
            /* Memory reduction. */
            for (i = 0; i < context->texture.pixelSamplers; i++)
            {
                glsTEXTURESAMPLER_PTR sampler = &context->texture.sampler[i];

                if ((sampler->enableTexturing
                  || sampler->enableExternalTexturing)
                &&  (sampler->binding->image.source != gcvNULL)
                &&  (sampler->binding->image.directSample == gcvFALSE)
                &&  (sampler->binding->object != gcvNULL)
                )
                {
                    /* Dynamically free mipmap level 0 for EGLImage case. */
                    /* Get shortcuts. */
                    glsTEXTUREWRAPPER_PTR texture = sampler->binding;

                    /* Destroy texture to destroy mipmap surfaces.
                     * And then allocate an empty texture object. */
                    gcmERR_BREAK(gcoTEXTURE_Destroy(texture->object));

                    texture->object      = gcvNULL;
                    texture->image.dirty = gcvTRUE;
                }
            }
#endif
#if gcdDEBUG
            if (kickOffPerDraw)
            {
                gcmVERIFY_OK(gcoSURF_Flush(gcvNULL));
                /* Commit command buffer. */
                gcmVERIFY_OK(gcoHAL_Commit(context->hal, gcvTRUE));
            }
#endif

#if DUMP_DRAW
            SaveAppFrame("result", "glDrawArrays", context->draw);
#endif

            /* Set drawableDirty flag. */
            {
                gcoSURF color = gcvNULL, depth = gcvNULL;

                if (context->frameBuffer == gcvNULL)
                {
                    color = context->draw;
                    depth = context->depth;
                }
                else
                {
                    color = glfGetFramebufferSurface(&context->frameBuffer->color);
                    depth = glfGetFramebufferSurface(&context->frameBuffer->depth);
                }

                if(color != gcvNULL && !context->depthStates.depthOnly)
                {
                    gcoSURF_SetFlags(color, gcvSURF_FLAG_CONTENT_UPDATED, gcvTRUE);
                }
                if(depth != gcvNULL && context->depthStates.testEnabled)
                {
                    gcoSURF_SetFlags(depth, gcvSURF_FLAG_CONTENT_UPDATED, gcvTRUE);
                }
            }

#if VIVANTE_PROFILER
            glmPROFILE(context, GL1_PROFILER_PRIMITIVE_END, (gctUINTPTR_T)instantDraw.primMode);

            if (context->profiler.enable)
            {
                _glffProfiler_NEW_Set(context, GL1_PROFILER_DRAW_END, 0);
            }
#if VIVANTE_PROFILER_PERDRAW
            if (context->profiler.enable)
            {
                gcoHAL_Commit(context->hal, gcvTRUE);
            }
#endif
#if VIVANTE_PROFILER_PROBE | VIVANTE_PROFILER_PERDRAW
            _glffProfiler(&context->profiler, GL1_PROFILER_DRAW_END, 0);
#endif
#endif

        }
        while (gcvFALSE);
    }
    glmLEAVE();
}

static GLboolean
_PatchIndex(
    IN glsCONTEXT_PTR Context,
    const GLvoid * Indices,
    GLenum Mode,
    GLenum Type,
    GLsizei Count,
    const  GLvoid * * patchedIndices,
    GLenum * patchedMode,
    GLsizei * patchedCount,
    gcoINDEX * patchedIndex,
    gctBOOL  * newIndices
    )
{
#if gcdNULL_DRIVER < 3
    GLvoid  *  indices     = gcvNULL;
    GLubyte *  base        = gcvNULL;
    GLubyte    elementSize = 0;
    GLubyte *  start       = gcvNULL;
    GLsizeiptr triangles   = 0;
    gctBOOL    oddOffset   = gcvFALSE;
    gctBOOL    locked      = gcvFALSE;
    gcoINDEX * targetIndex = gcvNULL;
    GLint      i = 0, j = 0;

    glsNAMEDOBJECT_PTR wrapper;
    glsBUFFER_PTR  boundIndex = gcvNULL;

    gcmASSERT(patchedIndices != gcvNULL);
    gcmASSERT(patchedMode    != gcvNULL);
    gcmASSERT(patchedCount   != gcvNULL);
    gcmASSERT(patchedIndex   != gcvNULL);

    *patchedIndices = Indices;
    *patchedMode    = Mode;
    *patchedCount   = Count;

    wrapper = Context->elementArrayBuffer;
    if ((wrapper != gcvNULL) && (wrapper->object != gcvNULL))
    {
         boundIndex    = (glsBUFFER_PTR)Context->elementArrayBuffer->object;
         *patchedIndex = boundIndex->index;
    }
    else
    {
         *patchedIndex = gcvNULL;
         boundIndex    = gcvNULL;
    }
    *newIndices = gcvFALSE;

    if (Mode != GL_TRIANGLE_STRIP)
    {
        return GL_FALSE;
    }

    switch(Type)
    {
    case GL_UNSIGNED_BYTE:
        elementSize = sizeof(GLubyte);
        break;

    case GL_UNSIGNED_SHORT:
        elementSize = sizeof(GLushort);
        break;

    case GL_UNSIGNED_INT:
        elementSize = sizeof(GLuint);
        break;

    default:
        return GL_FALSE;
    }


    /* If has index object. */
    if (boundIndex)
    {
        if (boundIndex->index == gcvNULL ||
            ((gcmPTR2INT(Indices) + Count * elementSize) > (gctSIZE_T)boundIndex->size))
        {
            return GL_FALSE;
        }

        /* Reset patched triangle list indices if dirtied. */
        if (boundIndex->patchDirty)
        {
            if (boundIndex->listIndexEven)
            {
                gcoINDEX_Destroy(boundIndex->listIndexEven);
                boundIndex->listIndexEven = gcvNULL;
            }

            if (boundIndex->listIndexOdd)
            {
                gcoINDEX_Destroy(boundIndex->listIndexOdd);
                boundIndex->listIndexOdd = gcvNULL;
            }

            boundIndex->patchDirty = gcvFALSE;
        }

        /* Make sure the offset must be aligned to elementSize. */
        gcmASSERT(gcmPTR2INT(Indices) % elementSize == 0);

        oddOffset = (gcmPTR2INT(Indices) / elementSize) % 2 ? gcvTRUE : gcvFALSE;
        targetIndex = oddOffset ? &boundIndex->listIndexOdd : &boundIndex->listIndexEven;

        if(targetIndex == gcvNULL)
        {
            return GL_FALSE;
        }

        if (*targetIndex)
        {
            *patchedIndices = (GLubyte * )(3 * (gcmPTR2INT(Indices) - (oddOffset ? elementSize : 0)));
            *patchedMode    = GL_TRIANGLES;
            *patchedCount   = (Count - 2) * 3;
            *patchedIndex   = *targetIndex;

            return GL_FALSE;
        }

        if (gcmIS_ERROR(gcoINDEX_Construct(Context->hal, targetIndex)))
        {
            return GL_FALSE;
        }

        gcmVERIFY_OK(gcoINDEX_SetSharedLock(*targetIndex,
                                            Context->bufferList->sharedLock));

        if (gcmIS_ERROR(gcoINDEX_Lock(boundIndex->index, gcvNULL, (GLvoid**)&base)))
        {
            return GL_FALSE;
        }
        locked = gcvTRUE;

        /* For IBOs, we calc triangle list indices for all the even/odd indices onces. */
        start     = base + (oddOffset ? elementSize : 0);
        triangles = boundIndex->size / elementSize - (oddOffset ? 3 : 2);
    }
    else
    {
        start = (GLubyte *)Indices;
        /* For client memory index buffer, only calc for this draw. */
        triangles = Count - 2;
    }

    if (gcmIS_ERROR(gcoOS_Allocate(Context->os, triangles * 3 * elementSize, &indices)))
    {
        if (locked)
        {
            gcoINDEX_Unlock(boundIndex->index);
        }
        return GL_FALSE;
    }

    switch (Type)
    {
    case GL_UNSIGNED_BYTE:
        {
            const GLubyte *src = (GLubyte *)start;
            GLubyte       *dst = (GLubyte *)indices;

            for (i = 0, j = 0; i < triangles; ++i, ++j)
            {
                dst[j * 3]     = src[(i % 2) == 0 ? i : i + 1];
                dst[j * 3 + 1] = src[(i % 2) == 0 ? i + 1 : i];
                dst[j * 3 + 2] = src[i + 2];
            }
        }
        break;

    case GL_UNSIGNED_SHORT:
        {
            const GLushort *src = (GLushort *)start;
            GLushort       *dst = (GLushort *)indices;

            for (i = 0, j = 0; i < triangles; ++i, ++j)
            {
                dst[j * 3]     = src[(i % 2) == 0 ? i : i + 1];
                dst[j * 3 + 1] = src[(i % 2) == 0 ? i + 1 : i];
                dst[j * 3 + 2] = src[i + 2];
            }
        }
        break;

    case GL_UNSIGNED_INT:
        {
            const GLuint *src = (GLuint *)start;
            GLuint       *dst = (GLuint *)indices;

            for (i = 0, j = 0; i < triangles; ++i, ++j)
            {
                dst[j * 3]     = src[(i % 2) == 0 ? i : i + 1];
                dst[j * 3 + 1] = src[(i % 2) == 0 ? i + 1 : i];
                dst[j * 3 + 2] = src[i + 2];
            }
        }
        break;

    default:
        break;
    }

    /* Patch mode, count, and start index. */
    *patchedMode  = GL_TRIANGLES;
    *patchedCount = (Count - 2) * 3;

    if (*targetIndex)
    {
        gcoINDEX_Upload(*targetIndex, indices, triangles * 3 * elementSize);

        *patchedIndices = (GLubyte*)(3 * (gcmPTR2INT(Indices) - (oddOffset ? elementSize : 0)));
        *patchedIndex   = *targetIndex;

        gcmVERIFY_OK(gcoOS_Free(Context->os, indices));
    }
    else
    {
        *patchedIndices = indices;
        *patchedIndex   = gcvNULL;
        *newIndices     = gcvTRUE;
    }

    if (locked)
    {
        gcoINDEX_Unlock(boundIndex->index);
    }

    return GL_TRUE;
#else
    return GL_FALSE;
#endif
}

gceSTATUS
glfDrawElements(
    glsCONTEXT_PTR Context,
    glsINSTANT_DRAW_PTR instantDraw
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL instanceDraw = gcvFALSE;

    gcmHEADER();

    /* Setup the vertex array. */
    gcmONERROR(_VertexArray(Context,
                            0,
                            (GLsizei *)&instantDraw->count,
                            instantDraw->indexType,
                            instantDraw->indexBuffer,
                            instantDraw->indexMemory,
                            &instantDraw->primMode,
                            (gctUINT *)(&instantDraw->primCount)));

    instanceDraw = gcoHAL_IsFeatureAvailable(gcvNULL,gcvFEATURE_HALTI5);

    if (instanceDraw)
    {
        gcmONERROR(gco3D_DrawInstancedPrimitives(Context->hw,
                                                 instantDraw->primMode,
                                                 gcvTRUE,
                                                 0,
                                                 0,
                                                 instantDraw->primCount,
                                                 instantDraw->count,
                                                 1
                                                 ));
    }
    else
    {
        /* Draw the primitives. */
        gcmONERROR(gco3D_DrawIndexedPrimitives(Context->hw,
                                               instantDraw->primMode,
                                               0,
                                               0,
                                               instantDraw->primCount
                                               ));
    }

    gcmFOOTER();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  glDrawElements
**
**  glDrawElements specifies multiple geometric primitives with very few
**  subroutine calls. Instead of calling a GL function to pass each individual
**  vertex, normal, texture coordinate, edge flag, or color, you can prespecify
**  separate arrays of vertices, normals, and so on, and use them to construct
**  a sequence of primitives with a single call to glDrawElements.
**
**  When glDrawElements is called, it uses count sequential elements from an
**  enabled array, starting at indices to construct a sequence of geometric
**  primitives. mode specifies what kind of primitives are constructed and how
**  the array elements construct these primitives. If more than one array is
**  enabled, each is used. If GL_VERTEX_ARRAY is not enabled, no geometric
**  primitives are constructed.
**
**  Vertex attributes that are modified by glDrawElements have an unspecified
**  value after glDrawElements returns. For example, if GL_COLOR_ARRAY is
**  enabled, the value of the current color is undefined after glDrawElements
**  executes. Attributes that aren't modified maintain their previous values.
**
**  INPUT:
**
**      Mode
**          Specifies what kind of primitives to render. Symbolic constants
**          GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP, GL_LINES, GL_TRIANGLE_STRIP,
**          GL_TRIANGLE_FAN and GL_TRIANGLES are accepted.
**
**      Count
**          Specifies the number of elements to be rendered.
**
**      Type
**          Specifies the type of the values in indices.
**          Must be GL_UNSIGNED_BYTE or GL_UNSIGNED_SHORT.
**
**      Indices
**          Specifies a pointer to the location where the indices are stored.
**
**  OUTPUT:
**
**      Nothing.
*/

GL_API void GL_APIENTRY glDrawElements(
    GLenum Mode,
    GLsizei Count,
    GLenum Type,
    const GLvoid* Indices
    )
{
    gceSTATUS       status          = gcvSTATUS_OK;
    GLboolean        indexPatched = GL_FALSE;
    glsNAMEDOBJECT_PTR    elementBuffer;
    gctBOOL         bNewIndices = gcvFALSE;
    GLsizei         origCount = Count;
    const   GLvoid* origIndices = Indices;
    gctBOOL         instanceDraw = gcvFALSE;
    glsINSTANT_DRAW instantDraw;
    gcsSPLIT_DRAW_INFO splitDrawInfo;

    glmENTER4(glmARGENUM, Mode, glmARGINT, Count, glmARGENUM, Type,
              glmARGPTR, Indices)
    {

#if VIVANTE_PROFILER && VIVANTE_PROFILER_PERDRAW
        glmPROFILE(context, GLES1_DRAWELEMENTS, 0);
        _glffProfiler(&context->profiler, GL1_PROFILER_DRAW_BEGIN, 0);
#endif
        glmPROFILE(context, GLES1_DRAWELEMENTS, 0);
        if (context->profiler.enable)
        {
            _glffProfiler_NEW_Set(context, GL1_PROFILER_DRAW_BEGIN, 0);
        }
        do
        {

#if gcdDEBUG_OPTION && gcdDEBUG_OPTION_NO_GL_DRAWS
            gcePATCH_ID patchId = gcvPATCH_INVALID;

            gcoHAL_GetPatchID(gcvNULL, &patchId);

            if (patchId == gcvPATCH_DEBUG)
                break;
#endif

            /* Check the count. */
            if (Count < 0)
            {
                glmERROR(GL_INVALID_VALUE);
                break;
            }

            /* init instantDraw.*/
            gcoOS_ZeroMemory(&instantDraw, gcmSIZEOF(glsINSTANT_DRAW));

            elementBuffer = context->elementArrayBuffer;

            if (context->patchStrip)
            {
                /* Patch Index object here if needed.
                   Index memory is patched in hal during upload. */
                if ((Mode == GL_TRIANGLE_STRIP)
                 && (elementBuffer != gcvNULL) && (elementBuffer->object != gcvNULL))
                {
                    gcmTRACE(gcvLEVEL_WARNING, "Convert triangle strip.");
                    indexPatched = _PatchIndex(context,
                                           Indices,
                                           Mode,
                                           Type,
                                           Count,
                                           &Indices,
                                           &Mode,
                                           &Count,
                                           &instantDraw.indexBuffer,
                                           &bNewIndices);
                }
                else
                {
                    instantDraw.indexBuffer = ((elementBuffer != gcvNULL) && (elementBuffer->object != gcvNULL))
                                ? ((glsBUFFER_PTR)elementBuffer->object)->index
                                : gcvNULL;

#if gcdSYNC
                    gcoINDEX_GetFence(instantDraw.indexBuffer);
#endif
                }
            }
            else
            {
                instantDraw.indexBuffer = ((elementBuffer != gcvNULL) && (elementBuffer->object != gcvNULL))
                            ? ((glsBUFFER_PTR)elementBuffer->object)->index
                            : gcvNULL;

#if gcdSYNC
                    gcoINDEX_GetFence(instantDraw.indexBuffer);
#endif
            }

            /* update count/ indices.*/
            instantDraw.count = Count;
            instantDraw.indexMemory = (gctPOINTER)Indices;

            /* Validate mode and determine the number of primitives. */
            if (!_GetPrimitiveCount(Mode, Count, &instantDraw.primCount, &instantDraw.primMode))
            {
                glmERROR(GL_INVALID_ENUM);
                break;
            }

            /* Validate and translate the index type. */
            if (Type == GL_UNSIGNED_BYTE)
            {
                instantDraw.indexType = gcvINDEX_8;
            }
            else if (Type == GL_UNSIGNED_SHORT)
            {
                instantDraw.indexType = gcvINDEX_16;
            }
            else if (Type == GL_UNSIGNED_INT)
            {
                instantDraw.indexType = gcvINDEX_32;
            }
            else
            {
                glmERROR(GL_INVALID_ENUM);
                break;
            }

            if((instantDraw.primCount <= 0)  /* ...primitive count is invalid. */
            ||  _IsFullCulled(context, Mode) /* ...primitive is fully culled. */
            ||  _InvalidPalette(context)     /* ...matrix palette is invalid. */
            )
            {
                break;
            }

            /* Update stencil states. */
            gcmERR_BREAK(glfUpdateStencil(context));

            /* Update frame buffer. */
            gcmERR_BREAK(glfUpdateFrameBuffer(context));

            /* Check buffer preserve. */
            gcmERR_BREAK(glfUpdateBufferPreserve(context));

            /* Update clipping box. */
            gcmERR_BREAK(glfUpdateClpping(context));

            /* Completely clipped out? */
            if (context->viewportStates.clippedOut)
            {
                break;
            }

            gcmERR_BREAK(glfUpdateViewport(context));
            gcmERR_BREAK(glfUpdateCulling(context));

            /* Count the primitives. */
            glmPROFILE(context, GL1_PROFILER_PRIMITIVE_TYPE,  (gctUINTPTR_T)instantDraw.primMode);
            glmPROFILE(context, GL1_PROFILER_PRIMITIVE_COUNT, (gctUINTPTR_T)instantDraw.primCount);

            /* Update polygon offset states. */
            gcmERR_BREAK(glfUpdatePolygonOffset(context));

            /* Update primitive info. */
            gcmERR_BREAK(glfUpdatePrimitveType(context, Mode));

            /* Update texture states. */
            gcmERR_BREAK(glfUpdateTextureStates(context));

            /* W-Limit patch needs to added BEFORE loading shader,
               as it checks if matrix states are dirty. */
            if (context->wLimitPatch)
            {
                _fixWlimit(context, 0, origCount, Type, origIndices);
            }

            if (context->useFragmentProcessor)
            {
                /* Program the fragment processor. */
                gcmERR_BREAK(glfUpdateFragmentProcessor(context));
            }
            else
            {
                /* Load shader program. */
                gcmERR_BREAK(glfLoadShader(context));
            }

            /* Load texture states. */
            gcmERR_BREAK(glfLoadTexture(context));


#if gcdDUMP_API
            gcmDUMP_API("${ES11 glDrawElements 0x%08X 0x%08X 0x%08X (0x%08X)",
                        Mode, Count, Type, Indices);
            {
                GLint min = -1, max = -1, element;
                GLsizei i;
                glsNAMEDOBJECT_PTR elementArrayBuffer = context->elementArrayBuffer;

                if (elementArrayBuffer == gcvNULL)        /* No Buffer Object. */
                {
                    switch (Type)
                    {
                    case GL_UNSIGNED_BYTE:
                        gcmDUMP_API_DATA(Indices, Count);
                        for (i = 0; i < Count; ++i)
                        {
                            element = ((GLubyte *) Indices)[i];
                            if ((min < 0) || (element < min))
                            {
                                min = element;
                            }
                            if ((max < 0) || (element > max))
                            {
                                max = element;
                            }
                        }
                        break;

                    case GL_UNSIGNED_SHORT:
                        gcmDUMP_API_DATA(Indices, Count * 2);
                        for (i = 0; i < Count; ++i)
                        {
                            element = ((GLushort *) Indices)[i];
                            if ((min < 0) || (element < min))
                            {
                                min = element;
                            }
                            if ((max < 0) || (element > max))
                            {
                                max = element;
                            }
                        }
                        break;

                    case GL_UNSIGNED_INT:
                        gcmDUMP_API_DATA(Indices, Count * 4);
                        for (i = 0; i < Count; ++i)
                        {
                            element = ((GLuint *) Indices)[i];
                            if ((min < 0) || (element < min))
                            {
                                min = element;
                            }
                            if ((max < 0) || (element > max))
                            {
                                max = element;
                            }
                        }
                        break;

                    default:
                        break;
                    }
                }
                else
                {
                    glsBUFFER_PTR object = (glsBUFFER_PTR)elementArrayBuffer->object;
                    switch (Type)
                    {
                    case GL_UNSIGNED_BYTE:
                        gcoINDEX_GetIndexRange(object->index,
                                               gcvINDEX_8,
                                               (gctUINT32) (gctUINTPTR_T) Indices,
                                               Count,
                                               (gctUINT32_PTR) &min,
                                               (gctUINT32_PTR) &max);
                        break;

                    case GL_UNSIGNED_SHORT:
                        gcoINDEX_GetIndexRange(object->index,
                                               gcvINDEX_16,
                                               (gctUINT32) (gctUINTPTR_T) Indices,
                                               Count,
                                               (gctUINT32_PTR) &min,
                                               (gctUINT32_PTR) &max);
                        break;

                    case GL_UNSIGNED_INT:
                        gcoINDEX_GetIndexRange(object->index,
                                               gcvINDEX_32,
                                               (gctUINT32) (gctUINTPTR_T) Indices,
                                               Count,
                                               (gctUINT32_PTR) &min,
                                               (gctUINT32_PTR) &max);
                        break;

                    default:
                        break;
                    }
                }

                if (min >= 0)
                {
                    _DumpVertices(context, 0, max + 1);
                }
            }
            gcmDUMP_API("$}");
#endif
            /* Temp fix for quadrant hang */
            if (context->isQuadrant)
            {
                gceCHIPMODEL chipModel;
                gctUINT32 chipRevision;

                gcoHAL_QueryChipIdentity(gcvNULL,&chipModel,&chipRevision,gcvNULL,gcvNULL);

                if(chipModel == gcv2000 &&
                   chipRevision == 0x5108  &&
                   context->elementArrayBuffer != gcvNULL &&
                   context->stencilStates.writeMask != 0x0 &&
                   context->depthStates.testEnabled)
                {
                    gco3D_Semaphore(context->hw, gcvWHERE_RASTER, gcvWHERE_PIXEL, gcvHOW_SEMAPHORE_STALL);
                }
            }

            /* Collect split draw info.*/
            gcoOS_ZeroMemory(&splitDrawInfo, sizeof(gcsSPLIT_DRAW_INFO));
            gcmERR_BREAK(glfCollectSplitDrawElementInfo(context, &instantDraw, &splitDrawInfo));

            if (splitDrawInfo.splitDrawType != gcvSPLIT_DRAW_UNKNOWN)
            {
                gcmERR_BREAK((*splitDrawInfo.splitDrawFunc)(context, &instantDraw, &splitDrawInfo));
            }
            else
            {
                gcmERR_BREAK(glfDrawElements(context, &instantDraw));
            }

            /* Set drawableDirty flag. */
            {
                gcoSURF color = gcvNULL, depth = gcvNULL;

                if (context->frameBuffer == gcvNULL)
                {
                    color = context->draw;
                    depth = context->depth;
                }
                else
                {
                    color = glfGetFramebufferSurface(&context->frameBuffer->color);
                    depth = glfGetFramebufferSurface(&context->frameBuffer->depth);
                }

                if(color != gcvNULL && !context->depthStates.depthOnly)
                {
                    gcoSURF_SetFlags(color, gcvSURF_FLAG_CONTENT_UPDATED, gcvTRUE);
                }
                if(depth != gcvNULL && context->depthStates.testEnabled)
                {
                    gcoSURF_SetFlags(depth, gcvSURF_FLAG_CONTENT_UPDATED, gcvTRUE);
                }
            }

#if VIVANTE_PROFILER
            glmPROFILE(context, GL1_PROFILER_PRIMITIVE_END, (gctUINTPTR_T)instantDraw.primMode);

            if (context->profiler.enable)
            {
                _glffProfiler_NEW_Set(context, GL1_PROFILER_DRAW_END, 0);
            }

#if VIVANTE_PROFILER_PERDRAW
            if (context->profiler.enable)
            {
                gcoHAL_Commit(context->hal, gcvTRUE);
            }
#endif
#if VIVANTE_PROFILER_PROBE | VIVANTE_PROFILER_PERDRAW
            _glffProfiler(&context->profiler, GL1_PROFILER_DRAW_END, 0);
#endif
#endif

        }
        while (gcvFALSE);

        /*******************************************************************************
        ** Bypass for stream cache limitation bug2631.
        ** If our vertex data is bigger than 1M which is stream cache size,
        ** _VertexArray just returns gcvSTATUS_INVALID_REQUEST and doesn't continue to
        ** render.
        ** We have to use _BuildStream to create a new stream and allocate enough memroy
        ** to store vertex data. This stream doesn't use cache.
        ** After using this stream, we'll release it in glfDeinitializeDraw.
        *******************************************************************************/
        if (status == gcvSTATUS_INVALID_REQUEST)
        {
            GLint       first = 0;
            glsSTREAM   stream;

            do
            {
                gcmERR_BREAK(_BuildStream(context,
                                          0,
                                          0,
                                          (GLsizei)instantDraw.count,
                                          instantDraw.indexType,
                                          Indices,
                                          &stream,
                                          &first));

               instanceDraw = gcoHAL_IsFeatureAvailable(gcvNULL,gcvFEATURE_HALTI5);

                if (instanceDraw)
                {
                    gcmERR_BREAK(gco3D_DrawInstancedPrimitives(context->hw,
                                                               instantDraw.primMode,
                                                               gcvTRUE,
                                                               first,
                                                               0,
                                                               instantDraw.primCount,
                                                               Count,
                                                               1
                                                               ));
                }
                else
                {
                    gcmERR_BREAK(gco3D_DrawIndexedPrimitives(context->hw,
                                                             instantDraw.primMode,
                                                             0,
                                                             first,
                                                             instantDraw.primCount
                                                             ));
                }

                /* Set drawableDirty flag. */
                {
                    gcoSURF color = gcvNULL, depth = gcvNULL;

                    if (context->frameBuffer == gcvNULL)
                    {
                        color = context->draw;
                        depth = context->depth;
                    }
                    else
                    {
                        color = glfGetFramebufferSurface(&context->frameBuffer->color);
                        depth = glfGetFramebufferSurface(&context->frameBuffer->depth);
                    }

                    if(color != gcvNULL && !context->depthStates.depthOnly)
                    {
                        gcoSURF_SetFlags(color, gcvSURF_FLAG_CONTENT_UPDATED, gcvTRUE);
                    }
                    if(depth != gcvNULL && context->depthStates.testEnabled)
                    {
                        gcoSURF_SetFlags(depth, gcvSURF_FLAG_CONTENT_UPDATED, gcvTRUE);
                    }
                }
            }
            while (gcvFALSE);

            /* Destroy the stream. */
            _FreeStream(context, &stream);
        }

        if (indexPatched && bNewIndices)
        {
            gcmVERIFY_OK(gcoOS_Free(context->os, (gctPOINTER)Indices));
        }
#if gcdDEBUG
        if (kickOffPerDraw)
        {
            gcmVERIFY_OK(gcoSURF_Flush(gcvNULL));
            /* Commit command buffer. */
            gcmVERIFY_OK(gcoHAL_Commit(context->hal, gcvTRUE));
        }
#endif
    }
    glmLEAVE();
}


/*******************************************************************************
**
**  glMultiDrawArraysEXT
**
**  glMultiDrawArraysEXT Behaves identically to DrawArrays except that a list
**  of arrays is specified instead. The number of lists is specified in the
**  primcount parameter. It has the same effect as:
**
**  for(i=0; i<primcount; i++) {
**      if (*(count+i)>0) DrawArrays(mode, *(first+i), *(count+i));
**  }
**
**  INPUT:
**
**      Mode
**          Specifies what kind of primitives to render. Symbolic constants
**          GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP, GL_LINES, GL_TRIANGLE_STRIP,
**          GL_TRIANGLE_FAN and GL_TRIANGLES are accepted.
**
**      First
**          Points to an array of starting indices in the enabled arrays.
**
**      Count
**          Points to an array of the number of indices
**          to be rendered.
**
**      Primcount
**          Specifies the size of first and count.
**
**  OUTPUT:
**
**      Nothing.
*/

GL_API void GL_APIENTRY glMultiDrawArraysEXT(
    GLenum Mode,
    const GLint* First,
    const GLsizei* Count,
    GLsizei Primcount)
{
    gceSTATUS status = gcvSTATUS_OK;

    glmENTER4(glmARGENUM, Mode, glmARGPTR, First, glmARGPTR, Count, glmARGINT, Primcount)
    {
        do
        {
            GLsizei i, j, primitiveCount;
            gcePRIMITIVE halPrimitive;
            GLboolean lineLoopPatch;
            GLsizei count;
            GLboolean instanceDraw = GL_FALSE;

            /* Check Primcount. */
            if (Primcount < 0)
            {
                glmERROR(GL_INVALID_VALUE);
                break;
            }

            /* Check all the count and first for error before proceeding. */
            for (j = 0; j < Primcount; j++)
            {
                if ((Count[j] < 0)
                ||  (First[j] < 0)
                )
                {
                    glmERROR(GL_INVALID_VALUE);
                    break;
                }
            }

            if (j != Primcount)
            {
                /* Some error occurred in above loop. */
                break;
            }

            if
            (
                _IsFullCulled(context, Mode) /* ...primitive is fully culled. */
                || _InvalidPalette(context)     /* ...matrix palette is invalid. */
            )
            {
                break;
            }

            /* Initialize common states for all the primitives. */

            /* Update stencil states. */
            gcmERR_BREAK(glfUpdateStencil(context));

            /* Update frame buffer. */
            gcmERR_BREAK(glfUpdateFrameBuffer(context));

            /* Check buffer preserve. */
            gcmERR_BREAK(glfUpdateBufferPreserve(context));

            /* Update clipping box. */
            gcmERR_BREAK(glfUpdateClpping(context));

            /* Completely clipped out? */
            if (context->viewportStates.clippedOut)
            {
                break;
            }

            /* Update polygon offset states. */
            gcmERR_BREAK(glfUpdatePolygonOffset(context));

            /* Update primitive info. */
            gcmERR_BREAK(glfUpdatePrimitveType(context, Mode));

            /* Update texture states. */
            gcmERR_BREAK(glfUpdateTextureStates(context));

            if (context->useFragmentProcessor)
            {
                /* Program the fragment processor. */
                gcmERR_BREAK(glfUpdateFragmentProcessor(context));
            }
            else
            {
                /* Load shader program. */
                gcmERR_BREAK(glfLoadShader(context));
            }

            /* Load texture states. */
            gcmERR_BREAK(glfLoadTexture(context));

            instanceDraw = gcoHAL_IsFeatureAvailable(gcvNULL,gcvFEATURE_HALTI5);

            if (Mode != GL_LINE_LOOP)
            {
                GLint first = First[0];
                GLsizei last = first + Count[0];

                /* Union all the First/Count pairs. */
                for (j = 1; j < Primcount; j++)
                {
                    if (first > First[j])
                    {
                        first = First[j];
                    }
                    if (First[j] + Count[j] > last)
                    {
                        last = First[j] + Count[j];
                    }
                }

                if(_GetPrimitiveCount(Mode, Count[0], &primitiveCount, &halPrimitive)
                    != GL_TRUE)
                {
                    glmERROR(GL_INVALID_VALUE);
                    break;
                }


                /* Setup the vertex array. */
                count = last - first;
                gcmERR_BREAK(_VertexArray(context,
                                          first, &count,
                                          gcvINDEX_8, gcvNULL, gcvNULL,
                                          &halPrimitive,
                                          (gctUINT *)(&primitiveCount)));

                if (instanceDraw)
                {
                    gctINT primitiveCount;

                    for (i = 0; i < Primcount; i++)
                    {
                        gcmVERIFY_OK(_GetPrimitiveCount(halPrimitive, Count[i], &primitiveCount, &halPrimitive));

                        /* Draw the primitives. */
                        gcmERR_BREAK(gco3D_DrawInstancedPrimitives(
                            context->hw,
                            halPrimitive,
                            gcvFALSE, First[i], 0, primitiveCount, Count[i],
                            1
                           ));
                    }
                }
                else
                {
                    /* Draw all the primitives at once. */
                    gcmERR_BREAK(gco3D_DrawPrimitivesCount(
                        context->hw,
                        halPrimitive,
                        (GLint*)First, (gctSIZE_T*)Count,
                        Primcount
                        ));
                }
            }
            else
            {
                for (j = 0; j < Primcount; j++)
                {
                    if(_GetPrimitiveCount(Mode, Count[j], &primitiveCount, &halPrimitive)
                        != GL_TRUE)
                    {
                        glmERROR(GL_INVALID_VALUE);
                        break;
                    }

                    if (primitiveCount <= 0)
                    {
                        continue;
                    }

                    /* Setup the vertex array. */
                    count = Count[j];
                    gcmERR_BREAK(_VertexArray(context,
                                              First[j], &count,
                                              gcvINDEX_8, gcvNULL, gcvNULL,
                                              &halPrimitive,
                                              (gctUINT *)(&primitiveCount)));

                    /* Test for line loop patch. */
                    lineLoopPatch = (Mode == GL_LINE_LOOP)
                                 && (halPrimitive != gcvPRIMITIVE_LINE_LOOP);

                    /* LOGICOP enabled? */
                    if (context->logicOp.perform)
                    {
                        /* In LOGICOP we have to do one primitive at a time. */
                        for (i = 0; i < primitiveCount; i++)
                        {
                            /* Create temporary target as destination. */
                            gcmERR_BREAK(_LogicOpPreProcess(context));

                            /* Draw the primitives. */
                            if (lineLoopPatch)
                            {
                                if (instanceDraw)
                                {
                                    /* Draw the primitives. */
                                    gcmERR_BREAK(gco3D_DrawInstancedPrimitives(
                                        context->hw,
                                        gcvPRIMITIVE_LINE_STRIP,
                                        gcvTRUE, i, 0, 1, 2,
                                        1
                                        ));
                                }
                                else
                                {

                                    gcmERR_BREAK(gco3D_DrawIndexedPrimitives(
                                        context->hw,
                                        gcvPRIMITIVE_LINE_STRIP,
                                        0, i, 1
                                        ));
                                }
                            }
                            else
                            {
                                if (instanceDraw)
                                {
                                    /* Draw the primitives. */
                                    gcmERR_BREAK(gco3D_DrawInstancedPrimitives(
                                        context->hw,
                                        halPrimitive,
                                        gcvFALSE, First[j] + i, 0, 1, 3, /* Point,line,triangle all send count = 3 */
                                        1
                                        ));
                                }
                                else
                                {
                                    gcmERR_BREAK(gco3D_DrawPrimitives(
                                        context->hw,
                                        halPrimitive,
                                        First[j] + i, 1
                                        ));
                                }
                            }

                            /* Run the post processing pass. */
                            gcmERR_BREAK(_LogicOpPostProcess(context));
                        }
                    }
                    else
                    {
                        /* Draw the primitives. */
                        if (lineLoopPatch)
                        {
                            if (instanceDraw)
                            {
                                /* Draw the primitives. */
                                gcmERR_BREAK(gco3D_DrawInstancedPrimitives(
                                    context->hw,
                                    gcvPRIMITIVE_LINE_STRIP,
                                    gcvTRUE, 0, 0, primitiveCount, Count[j],
                                    1
                                    ));
                            }
                            else
                            {
                                gcmERR_BREAK(gco3D_DrawIndexedPrimitives(
                                    context->hw,
                                    gcvPRIMITIVE_LINE_STRIP,
                                    0, 0, primitiveCount
                                    ));
                            }
                        }
                        else
                        {
                            if (instanceDraw)
                            {
                                /* Draw the primitives. */
                                gcmERR_BREAK(gco3D_DrawInstancedPrimitives(
                                    context->hw,
                                    halPrimitive,
                                    gcvFALSE, First[j], 0, primitiveCount, Count[j],
                                    1
                                    ));
                            }
                            else
                            {
                                gcmERR_BREAK(gco3D_DrawPrimitives(
                                    context->hw,
                                    halPrimitive,
                                    First[j], primitiveCount
                                    ));
                            }
                        }
                    }
                }
            }

            /* Set drawableDirty flag. */
            {
                gcoSURF color = gcvNULL, depth = gcvNULL;

                if (context->frameBuffer == gcvNULL)
                {
                    color = context->draw;
                    depth = context->depth;
                }
                else
                {
                    color = glfGetFramebufferSurface(&context->frameBuffer->color);
                    depth = glfGetFramebufferSurface(&context->frameBuffer->depth);
                }

                if(color != gcvNULL && !context->depthStates.depthOnly)
                {
                    gcoSURF_SetFlags(color, gcvSURF_FLAG_CONTENT_UPDATED, gcvTRUE);
                }
                if(depth != gcvNULL && context->depthStates.testEnabled)
                {
                    gcoSURF_SetFlags(depth, gcvSURF_FLAG_CONTENT_UPDATED, gcvTRUE);
                }
            }

#if gcdDEBUG
            if (kickOffPerDraw)
            {
                gcmVERIFY_OK(gcoSURF_Flush(gcvNULL));
                /* Commit command buffer. */
                gcmVERIFY_OK(gcoHAL_Commit(context->hal, gcvTRUE));
            }
#endif

        }
        while (gcvFALSE);
    }
    glmLEAVE();
}

/*******************************************************************************
**
**  glMultiDrawElementsEXT
**
**  glMultiDrawArraysEXT behaves identically to DrawElements except that a list
**  of arrays is specified instead. The number of lists is specified in the
**  primcount parameter. It has the same effect as:
**
**  for(i=0; i<primcount; i++) {
**      if (*(count+i)>0) DrawElements(mode, *(count+i), type,
**                                     *(indices+i));
**  }
**
**  INPUT:
**
**      Mode
**          Specifies what kind of primitives to render. Symbolic constants
**          GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP, GL_LINES, GL_TRIANGLE_STRIP,
**          GL_TRIANGLE_FAN and GL_TRIANGLES are accepted.
**
**      Count
**          Points to and array of the element counts.
**
**      Type
**          Specifies the type of the values in indices.
**          Must be GL_UNSIGNED_BYTE or GL_UNSIGNED_SHORT.
**
**      Indices
**          Specifies a pointer to the location where the indices are stored.
**
**      Primcount
**          Specifies the size of of the count array.
**
**  OUTPUT:
**
**      Nothing.
*/

GL_API void GL_APIENTRY glMultiDrawElementsEXT(
    GLenum Mode,
    const GLsizei* Count,
    GLenum Type,
    const void*const* Indices,
    GLsizei Primcount
    )
{
    glmENTER5(glmARGENUM, Mode, glmARGPTR, Count, glmARGENUM, Type,
              glmARGPTR, Indices, glmARGINT, Primcount)
    {
        do
        {
            GLsizei i;

            /* Check Primcount. */
            if (Primcount < 0)
            {
                glmERROR(GL_INVALID_VALUE);
                break;
            }

            /* Use the unoptimized way for now. */
            for (i = 0; i < Primcount; ++i)
            {
                if (*(Count + i) > 0)
                {
                    glDrawElements(Mode, *(Count + i), Type, *(Indices + i));
                }
            }



            /* Set drawableDirty flag. */
            {
                gcoSURF color = gcvNULL, depth = gcvNULL;

                if (context->frameBuffer == gcvNULL)
                {
                    color = context->draw;
                    depth = context->depth;
                }
                else
                {
                    color = glfGetFramebufferSurface(&context->frameBuffer->color);
                    depth = glfGetFramebufferSurface(&context->frameBuffer->depth);
                }

                if(color != gcvNULL && !context->depthStates.depthOnly)
                {
                    gcoSURF_SetFlags(color, gcvSURF_FLAG_CONTENT_UPDATED, gcvTRUE);
                }
                if(depth != gcvNULL && context->depthStates.testEnabled)
                {
                    gcoSURF_SetFlags(depth, gcvSURF_FLAG_CONTENT_UPDATED, gcvTRUE);
                }
            }
        }
        while (gcvFALSE);
    }
    glmLEAVE();
}

/*******************************************************************************
**
**  _GetArrayRange
**
**  Scan the index buffer to determine the active range of enabled arrays.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      IndexCount
**          Number of indices in the index buffer.
**
**      IndexType
**          Index type.
**
**      IndexBuffer
**          Pointer to the index buffer.
**
**  OUTPUT:
**
**      Length
**          Length of enabled arrays.
**
**      Length
**          Length of enabled arrays.
*/
static gceSTATUS _GetArrayRange(
    IN glsCONTEXT_PTR Context,
    IN GLsizei IndexCount,
    IN gceINDEX_TYPE IndexType,
    IN const GLvoid* IndexBuffer,
    OUT GLint * First,
    OUT GLsizei * Count
    )
{
    gceSTATUS status;
    gctUINT32 minIndex = ~0U;
    gctUINT32 maxIndex = 0;

    /* Is index buffer buffered? */
    if (Context->elementArrayBuffer)
    {
        /* Cast the buffer object. */
        glsBUFFER_PTR object =
            (glsBUFFER_PTR) Context->elementArrayBuffer->object;

        /* Get the index range. */
        status = gcoINDEX_GetIndexRange(
            object->index,
            IndexType,
            gcmPTR2INT32(IndexBuffer),
            IndexCount,
            &minIndex,
            &maxIndex
            );
    }
    else
    {
        GLsizei i;

        /* Assume success. */
        status = gcvSTATUS_OK;

        switch (IndexType)
        {
        case gcvINDEX_8:
            {
                gctUINT8_PTR indexBuffer = (gctUINT8_PTR) IndexBuffer;

                for (i = 0; i < IndexCount; i++)
                {
                    gctUINT32 curIndex = *indexBuffer++;

                    if (curIndex < minIndex)
                    {
                        minIndex = curIndex;
                    }

                    if (curIndex > maxIndex)
                    {
                        maxIndex = curIndex;
                    }
                }
            }
            break;

        case gcvINDEX_16:
            {
                gctUINT16_PTR indexBuffer = (gctUINT16_PTR) IndexBuffer;

                for (i = 0; i < IndexCount; i++)
                {
                    gctUINT32 curIndex = *indexBuffer++;

                    if (curIndex < minIndex)
                    {
                        minIndex = curIndex;
                    }

                    if (curIndex > maxIndex)
                    {
                        maxIndex = curIndex;
                    }
                }
            }
            break;

        case gcvINDEX_32:
            {
                gcmFATAL("_GetArrayRange: 32-bit indeces are not supported.");
                status = gcvSTATUS_NOT_SUPPORTED;
            }
            break;
        }
    }

    /* Set output. */
    *First = minIndex;
    *Count = maxIndex - minIndex + 1;

    /* Return status. */
    return status;
}

/*******************************************************************************
**
**  _BuildStream
**
**  Build the input stream.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      First
**          Specifies the starting index in the enabled arrays.
**
**      Count
**          Specifies the number of items in the enabled arrays to be rendered.
**
**      IndexCount
**          Number of indices in the index buffer.
**
**      IndexType
**          Index type.
**
**      IndexBuffer
**          Pointer to the index buffer.
**
**  OUTPUT:
**
**      Stream
**          Pointer to the list of created streams.
**
**      Start
**          Pointer to the starting vertex number.
*/
gceSTATUS _BuildStream(
    IN glsCONTEXT_PTR   Context,
    IN GLint            First,
    IN GLsizei          Count,
    IN GLsizei          IndexCount,
    IN gceINDEX_TYPE    IndexType,
    IN const GLvoid     *IndexBuffer,
    OUT glsSTREAM_PTR   Stream,
    OUT GLint           *Start
    )
{
    gceSTATUS status;

    do
    {
        gctUINT32           i;
        gctUINT32           attributeCount;
        gctUINT32           activeAttributeCount = 0;
        glsSTREAMINFO       info[glvATTRIBUTE_VS_COUNT];
        glsSTREAMINFO_PTR   infoList;
        gctUINT32           totalSize   = 0;
        gctUINT32           totalStride = 0;
        GLint               first       = First;
        gctBOOL             rebuild     = gcvFALSE;
        gcoSTREAM           stream      = gcvNULL;

        /* Initialize the attribute wrap list pointer. */
        glsATTRIBUTEWRAP_PTR attribubteWrap = Context->currProgram->vs.attributes;

        /* Construct a vertex object. */
        gcmERR_BREAK(gcoVERTEX_Construct(
            Context->hal,
            &Stream->vertex
            ));

        /* Query the number of attributes. */
        gcmERR_BREAK(gcSHADER_GetAttributeCount(
            Context->currProgram->vs.shader,
            &attributeCount
            ));

        /* Initialize the info list pointer. */
        infoList = (glsSTREAMINFO_PTR) &info;

        for (i = 0; i < attributeCount; i++)
        {
            gctBOOL attributeEnabled;
            gcmERR_BREAK(gcATTRIBUTE_IsEnabled(
                attribubteWrap->attribute,
                &attributeEnabled
                ));

            if (attributeEnabled)
            {
                glsATTRIBUTEINFO_PTR attribute = attribubteWrap->info;

                infoList->attribute = attribute;

                activeAttributeCount++;

                if (attribute->buffer == gcvNULL)
                {
                    if (attribute->pointer == gcvNULL)
                    {
                        status = gcvSTATUS_INVALID_OBJECT;
                        break;
                    }

                    rebuild = gcvTRUE;
                }
                else
                {
                    /* Cast the object. */
                    glsBUFFER_PTR object =
                        (glsBUFFER_PTR) attribute->buffer->object;

                    /* Is there a stream attached? */
                    if (object->stream == gcvNULL)
                    {
                        status = gcvSTATUS_INVALID_OBJECT;
                        break;
                    }

                    if (stream != gcvNULL && stream != object->stream)
                    {
                        rebuild = gcvTRUE;
                    }

                    stream = object->stream;

                    /* Set the stream object. */
                    infoList->stream = stream;
                }

                totalStride += infoList->attribute->attributeSize;

                infoList++;
            }

            attribubteWrap++;
        }

        /* Failed? */
        if (gcmIS_ERROR(status))
        {
            break;
        }

        infoList        = (glsSTREAMINFO_PTR) &info;
        attribubteWrap  = Context->currProgram->vs.attributes;

        if (rebuild)
        {
            gctUINT8_PTR src, dest, destAddress;
            gctUINT32 offset = 0;
            gctBOOL dirty = gcvFALSE;
            gcoVERTEX vertex = Stream->vertex;

            /* Determine array range. */
            if (Count == 0)
            {
                gcmERR_BREAK(_GetArrayRange(
                    Context,
                    IndexCount,
                    IndexType,
                    IndexBuffer,
                    &first,
                    &Count
                    ));
            }

            /* If index count is 0, do not change First.     */
            /* Otherwise, the First is 0 for DrawElements to */
            /* locate vertex correctly.                      */
            *Start = (IndexCount == 0)
                    ? first
                    : 0;

#if gldUSE_VERTEX_CACHE
            /* Check if all attribute is cached. */
            if (_IsVertexCacheHit(
                Context, activeAttributeCount, infoList, first, Count,
                IndexCount, IndexType, IndexBuffer,
                &streamCache
                ))
            {
                return gcvSTATUS_OK;
            }
#endif

            totalSize = totalStride * first + totalStride * Count;

#if gldUSE_VERTEX_CACHE
            /* Cache not hit, but get a cache slot. */
            if (streamCache)
            {
                /* Save stream info to the cache. */
                vertex = streamCache->stream.vertex;
                streamCache->stride         = totalStride;
                streamCache->attributeCount = activeAttributeCount;
                streamCache->start          = first;
                streamCache->end            = first + Count -1;

                /* The vertice will be upload to cache. */
                stream = streamCache->stream.stream[0];
            }
            else
#endif
            {
                if ((Context->streamIndex % gldSTREAM_GROUP_SIZE) == 0)
                {
                    while (gcvTRUE)
                    {
                        status = gcoOS_WaitSignal(
                            gcvNULL,
                            Context->streamSignals
                                [Context->streamIndex / gldSTREAM_GROUP_SIZE],
                            10
                            );

                        if (gcmIS_SUCCESS(status))
                        {
                            break;
                        }

                        gcmERR_BREAK(gcoHAL_Commit(Context->hal, gcvFALSE));
                    }
                }

                if (!Context->streams[Context->streamIndex])
                {
                    /* Construct stream wrapper. */
                    gcmERR_BREAK(gcoSTREAM_Construct(Context->hal, &stream));
                    Context->streams[Context->streamIndex] = stream;
                    gcmVERIFY_OK(gcoSTREAM_SetSharedLock(stream,
                                                         Context->bufferList->sharedLock));
                }
                else
                {
                    stream = Context->streams[Context->streamIndex];
                }

                Context->streamIndex   = (Context->streamIndex + 1) % gldSTREAM_POOL_SIZE;
                Context->streamPending = gcvTRUE;
            }

            gcmERR_BREAK(gcoSTREAM_Reserve(
                stream, totalSize
                ));

            gcmERR_BREAK(gcoSTREAM_SetStride(
                stream, totalStride
                ));

            gcmERR_BREAK(gcoSTREAM_Lock(
                stream, (gctPOINTER) &destAddress, gcvNULL
                ));

#if gldUSE_VERTEX_CACHE
            /* These info should not be put into the temp Stream. */
            if (!streamCache)
#endif
            {
                Stream->attributeCount++;
            }

            for (i = 0; i < activeAttributeCount; ++i)
            {
                GLsizei j;
                gctUINT32 srcOffset = 0, destOffset = 0;
                gctUINT32 srcStride = 0;
                gctUINT32 size;

                dest = (gctUINT8_PTR)destAddress + totalStride * first + offset;
                size = infoList->attribute->attributeSize;

                if (infoList->attribute->buffer == gcvNULL)
                {
                    src = (gctUINT8_PTR) infoList->attribute->pointer
                        + first * infoList->attribute->stride;

                    srcStride = infoList->attribute->stride;
                }
                else
                {
                    glsBUFFER_PTR object = (glsBUFFER_PTR) infoList->attribute->buffer->object;

                    gcmERR_BREAK(gcoSTREAM_Lock(
                        object->stream, (gctPOINTER) &src, gcvNULL
                        ));

                    src = src + (gctUINTPTR_T)infoList->attribute->pointer + first * infoList->attribute->stride;
                    srcStride = infoList->attribute->stride;
                }

                for (j = 0; j < Count; j += 1)
                {
                    gcoOS_MemCopy(dest + destOffset, src + srcOffset, size);

                    destOffset += totalStride;
                    srcOffset  += srcStride;
                }

                /* Add the stream to the vertex. */
                gcmERR_BREAK(gcoVERTEX_EnableAttribute(
                    vertex, i,
                    infoList->attribute->format,
                    infoList->attribute->normalize,
                    infoList->attribute->components,
                    stream,
                    offset,
                    totalStride
                    ));

                dirty = gcvTRUE;

#if gldUSE_VERTEX_CACHE
                /* Save the attribute info into the cache. */
                if (streamCache)
                {
                    gcoOS_MemCopy(
                        &streamCache->attributes[i],
                        infoList->attribute,
                        sizeof(glsATTRIBUTEINFO)
                        );

                    streamCache->offset[i] = offset;
                }
#endif

                offset += infoList->attribute->attributeSize;
                infoList++;
            }

            if (dirty)
            {
                gcmERR_BREAK(gcoSTREAM_Flush(stream));
            }

            gcmERR_BREAK(gcoVERTEX_Bind(vertex));
        }
        else
        {
            /* Determine active attributes. */
            for (i = 0; i < activeAttributeCount; i++)
            {
                gctUINT32 offset = gcmPTR2INT32(infoList->attribute->pointer);

                /* Set the stream stride. */
                gcmERR_BREAK(gcoSTREAM_SetStride(
                    infoList->stream,
                    infoList->attribute->stride
                    ));

                /* Add the stream to the vertex. */
                gcmERR_BREAK(gcoVERTEX_EnableAttribute(
                    Stream->vertex, i,
                    infoList->attribute->format,
                    infoList->attribute->normalize,
                    infoList->attribute->components,
                    infoList->stream,
                    offset,
                    infoList->attribute->stride
                    ));

                /* Advance to the next attribute information structure. */
                infoList++;
            }

            /* If draw with index, start at zero vertex. Or if all attributes
               come from client pointer, start at zero because we have enabled
               attributes at proper offset. Else, start at the given offset. */
            *Start = (IndexCount == 0)
                    ? First
                    : 0;

            /* Bind the vertex. */
            gcmERR_BREAK(gcoVERTEX_Bind(Stream->vertex));
        }
    }
    while (GL_FALSE);

    /* Return status. */
    return status;
}

/*******************************************************************************
**
**  _FreeStream
**
**  Free the input stream.
**
**  INPUT:
**
**      Stream
**          Pointer to the stream to be deleted.
**
**  OUTPUT:
**
**      Nothing.
*/

void _FreeStream(
    IN glsCONTEXT_PTR Context,
    IN glsSTREAM_PTR Stream
    )
{
    if (Stream->vertex != gcvNULL)
    {
        gcmVERIFY_OK(gcoVERTEX_Destroy(Stream->vertex));
        Stream->vertex = gcvNULL;
    }

    if (Context->streamPending)
    {
        if ((Context->streamIndex % gldSTREAM_GROUP_SIZE) == 0)
        {
            gcsHAL_INTERFACE    halInterface;
            gctUINT32           signalIndex;

            signalIndex = (Context->streamIndex == 0)
                ? ((gldSTREAM_POOL_SIZE  - 1) / gldSTREAM_GROUP_SIZE)
                :  (Context->streamIndex - 1) / gldSTREAM_GROUP_SIZE;

            halInterface.command            = gcvHAL_SIGNAL;
            halInterface.u.Signal.signal    = gcmPTR_TO_UINT64(Context->streamSignals[signalIndex]);
            halInterface.u.Signal.auxSignal = 0;
            halInterface.u.Signal.process   = gcmPTR_TO_UINT64(gcoOS_GetCurrentProcessID());
            halInterface.u.Signal.fromWhere = gcvKERNEL_COMMAND;

            /* Schedule the event. */
            gcmVERIFY_OK(gcoHAL_ScheduleEvent(
                Context->hal, &halInterface
                ));
        }

        Context->streamPending = gcvFALSE;
    }
}
