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


#include "gc_vg_precomp.h"


/******************************************************************************\
*********************** Support Functions and Definitions **********************
\******************************************************************************/

/* Constant color values. */
vgtFLOATVECTOR4 vgvFLOATCOLOR0000 = { 0.0f, 0.0f, 0.0f, 0.0f };
vgtBYTEVECTOR4  vgvBYTECOLOR0000  = { 0x00, 0x00, 0x00, 0x00 };

vgtFLOATVECTOR4 vgvFLOATCOLOR0001 = { 0.0f, 0.0f, 0.0f, 1.0f };
vgtBYTEVECTOR4  vgvBYTECOLOR0001  = { 0x00, 0x00, 0x00, 0xFF };

vgtFLOATVECTOR4 vgvFLOATCOLOR1111 = { 1.0f, 1.0f, 1.0f, 1.0f };
vgtBYTEVECTOR4  vgvBYTECOLOR1111  = { 0xFF, 0xFF, 0xFF, 0xFF };


/*******************************************************************************
**
** _GetSource
**
** Determine whether the source and target rectangles intersect and if they do,
** substitute the source image with its copy.
**
** INPUT:
**
**    Image
**       Image object.
**
**    X, Y
**       Origin within the image.
**
** OUTPUT:
**
**    Info
**       Pointer to the setup information structure.
*/

static gceSTATUS _GetSource(
    IN vgsCONTEXT_PTR Context,
    IN OUT vgsIMAGE_PTR * Source,
    IN vgsIMAGE_PTR Target,
    IN OUT gctINT_PTR SourceX,
    IN OUT gctINT_PTR SourceY,
    IN gctINT TargetX,
    IN gctINT TargetY,
    IN gctINT Width,
    IN gctINT Height,
    IN gctBOOL isDrawImage
    )
{
    gceSTATUS status;
    vgmENTERSUBAPI(_GetSource);

    do
    {
        vgsIMAGE_PTR source;
        gctINT sourceX, sourceY;
        gctBOOL horIntersection;
        gctBOOL verIntersection;


        /***********************************************************************
        ** Verify rectangle intersection.
        */

        /* Get shortcuts to the image. */
        source = *Source;

        /* Make shortcuts to the inputs. */
        sourceX = * SourceX;
        sourceY = * SourceY;

        /* Are the images related? */
        if (!vgfImagesRelated(Context, source, Target))
        {
            /* Not related, no way to intersect. */
            status = gcvSTATUS_OK;
            break;
        }

        /* Do the rectangles intersect horizontally? */
        horIntersection = gcmINTERSECT(
            source->origin.x + sourceX,
            Target->origin.x + TargetX,
            Width
            );

        if (!horIntersection)
        {
            /* No horizontal intersection. */
            status = gcvSTATUS_OK;
            break;
        }

        /* Do the rectangles intersect vertically? */
        verIntersection = gcmINTERSECT(
            source->origin.y + sourceY,
            Target->origin.y + TargetY,
            Height
            );

        if (!verIntersection)
        {
            /* No vertical intersection. */
            status = gcvSTATUS_OK;
            break;
        }

        /***********************************************************************
        ** Rectangles intersect, make a copy of the source.
        */

        /* Initialize the temporary image. */
        gcmERR_BREAK(vgfInitializeTempImage(
            Context, source->format, Width, Height
            ));

        /* Copy the source. */
        gcmERR_BREAK(vgfDrawImage(
            Context,
            source, &Context->tempImage,
            sourceX, sourceY, 0, 0,
            Width, Height,
            gcvVG_BLEND_SRC,
            gcvFALSE,
            gcvFALSE,
            gcvFALSE,
            gcvFALSE,
            gcvFALSE,
            isDrawImage
            ));

        /* Substitute the source. */
        * Source  = &Context->tempImage;
        * SourceX = 0;
        * SourceY = 0;
    }
    while (gcvFALSE);

    vgmLEAVESUBAPI(_GetSource);
    /* Return status. */
    return status;
}


/*******************************************************************************
**
** _ImageDestructor
**
** Image object destructor. Called when the object is being destroyed.
**
** INPUT:
**
**    Context
**       Pointer to the context.
**
**    Object
**       Pointer to the object.
**
** OUTPUT:
**
**    Nothing.
*/

static gceSTATUS _ImageDestructor(
    vgsCONTEXT_PTR Context,
    vgsOBJECT_PTR Object
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    vgmENTERSUBAPI(_ImageDestructor);

    do
    {
        /* Cast the object. */
        vgsIMAGE_PTR image = (vgsIMAGE_PTR) Object;

        /* Is this the root image? */
        if ((image->parent == image) ||
            (image->parent == gcvNULL))
        {
            /* Should have no children. */
            gcmASSERT(image->childrenCount == 0);

            /* Unlock. */
            if (image->buffer != gcvNULL)
            {
                gcmERR_BREAK(gcoSURF_Unlock(
                    image->surface, image->buffer
                    ));

                image->buffer = gcvNULL;
            }

            /* Destroy the surface. */
            if (image->surface)
            {

#if gcdENABLE_3D
                gcsTLS_PTR tls;
                gcmERR_BREAK(gcoOS_GetTLS(&tls));

                /* Unset VG target. */
                if (tls->engineVG != gcvNULL)
                {
#if gcdGC355_PROFILER
                    gcmERR_BREAK(
                        gcoVG_UnsetTarget(tls->engineVG,
                        0,0,0,
                        image->surface));
#else
                    gcmERR_BREAK(
                        gcoVG_UnsetTarget(tls->engineVG,
                        image->surface));
#endif
                }
#endif

                gcmERR_BREAK(gcoSURF_Destroy(
                    image->surface
                    ));

                image->surface = gcvNULL;
            }
        }

        /* No, it's a child. */
        else
        {
            /* Decrement paprent's children count. */
            image->parent->childrenCount--;

            /* Dereference the parent. */
            gcmERR_BREAK(vgfDereferenceObject(
                Context, (vgsOBJECT_PTR *) &image->parent
                ));
        }
    }
    while (gcvFALSE);

    vgmLEAVESUBAPI(_ImageDestructor);
    return status;
}

#if gcdVG_ONLY
/*******************************************************************************
**
** _CreateIndexedImage: Index Color states.
**
** Create an image object with indexed format (1, 2, 4, 8).
**
** INPUT:
**
**    Context
**       Pointer to the context.
**
**    DataFormat
**       Format of the image.
**
**    Width, Height
**       The size of the image.
**
**    AllowedQuality
**       Allowed quality of the image.
**
** OUTPUT:
**
**    Image
**       Pointer to the initialized image.
*/
static gceSTATUS _CreateIndexedImage(
    IN vgsCONTEXT_PTR Context,
    IN VGImageFormat DataFormat,
    IN gctINT Width,
    IN gctINT Height,
    IN VGImageQuality AllowedQuality,
    IN vgsIMAGE_PTR * Image
    )
{
    gceSTATUS status;
    gcoSURF surface = gcvNULL;
    gctPOINTER buffer[3] = {gcvNULL};
    vgsIMAGE_PTR image;
    gctBOOL imageAllocated;
    vgmENTERSUBAPI(_CreateIndexedImage);

    do
    {
        gceSURF_COLOR_TYPE colorType;
        gceSURF_FORMAT  surfFormat;
        gctINT stride;

        /* Assume image as locally allocated. */
        imageAllocated = gcvTRUE;

        /* Get the pointer to the image object. */
        image = * Image;

        /* Not yet allocated? */
        if (image == gcvNULL)
        {
            /* Allocate an image object. */
            status = vgfReferenceImage(Context, &image);

            if (gcmIS_ERROR(status))
            {
                vgmERROR(VG_OUT_OF_MEMORY_ERROR);
                break;
            }

            /* Set the image pointer. */
            * Image = image;
        }
        else
        {
            /* Initialize the object. */
            image->object.type           = vgvOBJECTTYPE_IMAGE;
            image->object.prev           = gcvNULL;
            image->object.next           = gcvNULL;
            image->object.referenceCount = 1;
            image->object.userValid      = VG_FALSE;

            /* Mark image as allocated somewhere else. */
            imageAllocated = gcvFALSE;
        }

        /* Get the format information. */
        switch (DataFormat)
        {
        case VG_EXT_INDEX1:
            surfFormat = gcvSURF_INDEX1;
            break;

        case VG_EXT_INDEX2:
            surfFormat = gcvSURF_INDEX2;
            break;

        case VG_EXT_INDEX4:
            surfFormat = gcvSURF_INDEX4;
            break;

        case VG_EXT_INDEX8:
            surfFormat = gcvSURF_INDEX8;
            break;

        default:
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            return gcvSTATUS_INVALID_ARGUMENT;
        }

        /* Allocate the surface. */
        status = gcoSURF_Construct(
            Context->hal,
            Width, Height, 1,
            gcvSURF_BITMAP,
            surfFormat,
            gcvPOOL_DEFAULT,
            &surface
            );

        if (gcmIS_ERROR(status))
        {
            vgmERROR(VG_OUT_OF_MEMORY_ERROR);
            break;
        }

        /* Get the logical pointer to the surface. */
        status = gcoSURF_Lock(
            surface, gcvNULL, buffer
            );

        if (gcmIS_ERROR(status))
        {
            vgmERROR(VG_OUT_OF_MEMORY_ERROR);
            break;
        }

        /* Get the surface stride. */
        status = gcoSURF_GetAlignedSize(
            surface, gcvNULL, gcvNULL, &stride
            );

        if (gcmIS_ERROR(status))
        {
            vgmERROR(VG_OUT_OF_MEMORY_ERROR);
            break;
        }

        /* Set the proper color space. */
        colorType = gcvSURF_COLOR_UNKNOWN;

        status = gcoSURF_SetColorType(surface, colorType);

        if (gcmIS_ERROR(status))
        {
            vgmERROR(VG_OUT_OF_MEMORY_ERROR);
            break;
        }

        /* Set image parameters. */
        image->size.width  = Width;
        image->size.height = Height;
        image->origin.x    = 0;
        image->origin.y    = 0;
        image->stride      = stride;

        image->allowedQuality = AllowedQuality;

        image->format         = DataFormat;
        image->upsample       = gcvFALSE;
        image->wrapperFormat  = gcvNULL;
        image->surfaceFormat  = gcvNULL;
        image->orientation    = vgvIMAGE_ORIENTATION;
        image->surOrientation = vgvIMAGE_ORIENTATION;

        image->glyph        = 0;
        image->pattern      = 0;
        image->renderTarget = 0;

        image->parent        = image;
        image->childrenCount = 0;
        image->surface       = surface;
        image->buffer        = (gctUINT8_PTR)(buffer[0]);

        image->imageAllocated   = imageAllocated;
        image->surfaceAllocated = gcvTRUE;
        image->surfaceLocked    = gcvTRUE;
        image->imageDirty       = vgvIMAGE_READY;
        image->imageDirtyPtr    = &image->imageDirty;
        image->surfArgValid     = gcvTRUE;
        /* Success. */
        status = gcvSTATUS_OK;
        goto ErrorHandler;
    }
    while (gcvFALSE);

    /* Roll back. */
    if (buffer[0] != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(surface, buffer[0]));
    }

    if (surface != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(surface));
    }

    if (imageAllocated)
    {
        gcmVERIFY_OK(vgfDereferenceObject(Context, (vgsOBJECT_PTR *) &image));
    }

ErrorHandler:
    break;
    vgmLEAVESUBAPI(_CreateIndexedImage);
    /* Return status. */
    return status;
}
#endif

/******************************************************************************\
************************** Individual State Functions **************************
\******************************************************************************/

/*----------------------------------------------------------------------------*/
/*------------------------------ VG_IMAGE_FORMAT -----------------------------*/

vgmGETSTATE_FUNCTION(VG_IMAGE_FORMAT)
{
    ValueConverter(
        Values, &((vgsIMAGE_PTR) Object)->format, 1, VG_FALSE, VG_TRUE
        );
}

/*----------------------------------------------------------------------------*/
/*------------------------------ VG_IMAGE_WIDTH ------------------------------*/

vgmGETSTATE_FUNCTION(VG_IMAGE_WIDTH)
{
    ValueConverter(
        Values, &((vgsIMAGE_PTR) Object)->size.width, 1, VG_FALSE, VG_TRUE
        );
}

/*----------------------------------------------------------------------------*/
/*----------------------------- VG_IMAGE_HEIGHT ------------------------------*/

vgmGETSTATE_FUNCTION(VG_IMAGE_HEIGHT)
{
    ValueConverter(
        Values, &((vgsIMAGE_PTR) Object)->size.height, 1, VG_FALSE, VG_TRUE
        );
}


/******************************************************************************\
***************************** Context State Array ******************************
\******************************************************************************/

static vgsSTATEENTRY _stateArray[] =
{
    vgmDEFINE_SCALAR_READONLY_ENTRY(VG_IMAGE_FORMAT, INT),
    vgmDEFINE_SCALAR_READONLY_ENTRY(VG_IMAGE_WIDTH,  INT),
    vgmDEFINE_SCALAR_READONLY_ENTRY(VG_IMAGE_HEIGHT, INT)
};


/******************************************************************************\
************************ Internal Image Management API *************************
\******************************************************************************/

/*******************************************************************************
**
** vgfReferenceImage
**
** vgfReferenceImage increments the reference count of the given VGImage object.
** If the object does not exist yet, the object will be created and initialized
** with the default values.
**
** INPUT:
**
**    Context
**       Pointer to the context.
**
** OUTPUT:
**
**    vgsIMAGE_PTR * Image
**       Handle to the new VGImage object.
*/

gceSTATUS vgfReferenceImage(
    vgsCONTEXT_PTR Context,
    vgsIMAGE_PTR * Image
    )
{
    gceSTATUS status, last;
    vgsIMAGE_PTR image = gcvNULL;
    vgmENTERSUBAPI(vgfReferenceImage);


    do
    {
        /* Create the object if does not exist yet. */
        if (*Image == gcvNULL)
        {
            /* Allocate the object structure. */
            gcmERR_BREAK(gcoOS_Allocate(
                Context->os,
                gcmSIZEOF(vgsIMAGE),
                (gctPOINTER *) &image
                ));

            /* Initialize the object structure. */
            image->object.type           = vgvOBJECTTYPE_IMAGE;
            image->object.prev           = gcvNULL;
            image->object.next           = gcvNULL;
            image->object.referenceCount = 0;
            image->object.userValid      = VG_TRUE;

            /* Insert the object into the cache. */
            gcmERR_BREAK(vgfObjectCacheInsert(
                Context, (vgsOBJECT_PTR) image
                ));

            /* Set default object states. */
            image->format        = VG_IMAGE_FORMAT_FORCE_SIZE;
            image->origin.x      = 0;
            image->origin.y      = 0;
            image->size.width    = 0;
            image->size.height   = 0;
            image->glyph         = 0;
            image->pattern       = 0;
            image->renderTarget  = 0;
            image->parent        = gcvNULL;
            image->childrenCount = 0;
            image->surface       = gcvNULL;
            image->buffer        = gcvNULL;
            image->imageDirty    = vgvIMAGE_READY;
            image->imageDirtyPtr = &image->imageDirty;
            image->surfArgValid  = gcvTRUE;

            /* Set the result pointer. */
            *Image = image;
        }

        /* Increment the reference count. */
        (*Image)->object.referenceCount++;

        /* Success. */
        status = gcvSTATUS_OK;
        goto OnSuccess;
    }
    while (gcvFALSE);

    /* Roll back. */
    if (image != gcvNULL)
    {
        gcmCHECK_STATUS(gcoOS_Free(Context->os, image));
    }
OnSuccess:
    break;

    vgmLEAVESUBAPI(vgfReferenceImage);
    /* Return status. */
    return status;
}


/*******************************************************************************
**
** vgfSetImageObjectList
**
** Initialize object cache list information.
**
** INPUT:
**
**    ListEntry
**       Entry to initialize.
**
** OUTPUT:
**
**    Nothing.
*/

void vgfSetImageObjectList(
    vgsCONTEXT_PTR Context,
    vgsOBJECT_LIST_PTR ListEntry
    )
{
    vgmENTERSUBAPI(vgfSetImageObjectList);
    {
        ListEntry->stateArray     = _stateArray;
        ListEntry->stateArraySize = gcmCOUNTOF(_stateArray);
        ListEntry->destructor     = _ImageDestructor;
    }
    vgmLEAVESUBAPI(vgfSetImageObjectList);
}


/*******************************************************************************
**
** vgfInitializeImage
**
** Initialize preallocated image object.
**
** INPUT:
**
**    Context
**       Pointer to the context.
**
**    Image
**       Pointer to the image object to initialize.
**
**    Surface
**       Handle of the preallocated image surface.
**
** OUTPUT:
**
**    Nothing.
*/

gceSTATUS vgfInitializeImage(
    vgsCONTEXT_PTR Context,
    vgsIMAGE_PTR Image,
    gcoSURF Surface
    )
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgfInitializeImage);

    do
    {
        gceSURF_FORMAT format;
        gceSURF_COLOR_TYPE colorType;
        gctBOOL linear, premultiplied;
        VGImageFormat vgFormat;
        vgsFORMAT_PTR wrapperFormat;
        vgsFORMAT_PTR surfaceFormat;
        gceORIENTATION orientation;
        gctBOOL upsample;
        gctUINT width, height;
        gctINT stride;
        gctUINT8_PTR buffer[3];

        /* Query surface format. */
        gcmERR_BREAK(gcoSURF_GetFormat(Surface, gcvNULL, &format));

        /* Get color space information. */
        gcmERR_BREAK(gcoSURF_GetColorType(Surface, &colorType));

        /* Get surface size. */
        gcmERR_BREAK(gcoSURF_GetSize(Surface, &width, &height, gcvNULL));

        /* Get the surface stride. */
        gcmERR_BREAK(gcoSURF_GetAlignedSize(Surface, gcvNULL, gcvNULL, &stride));

        /* Get orientation. */
        gcmERR_BREAK(gcoSURF_QueryOrientation(Surface, &orientation));

        /* Lock the surface. */
        gcmASSERT(Image->buffer == gcvNULL);
        gcmERR_BREAK(gcoSURF_Lock(Surface, gcvNULL, (gctPOINTER*)buffer));

        /* Determine color space characteristics. */
        linear        = ((colorType & gcvSURF_COLOR_LINEAR)    != 0);
        premultiplied = ((colorType & gcvSURF_COLOR_ALPHA_PRE) != 0);

        /* Determine OpenVG format. */
        vgFormat = vgfGetOpenVGFormat(Context, format, linear, premultiplied);

        /* Get the format information. */
        wrapperFormat = vgfGetFormatInfo(Context, vgFormat);

        /* Determine whether upsampling is required. */
        upsample = (wrapperFormat->upsampledFormat != gcvNULL);

        /* Determine upsampled format. */
        surfaceFormat = upsample
            ? wrapperFormat->upsampledFormat
            : wrapperFormat;

        /* Initialize the object. */
        Image->object.type           = vgvOBJECTTYPE_IMAGE;
        Image->object.prev           = gcvNULL;
        Image->object.next           = gcvNULL;
        Image->object.referenceCount = 1;
        Image->object.userValid      = VG_FALSE;

        /* Set image parameters. */
        Image->size.width  = width;
        Image->size.height = height;
        Image->origin.x    = 0;
        Image->origin.y    = 0;
        Image->stride      = stride;

        Image->allowedQuality = vgvIMAGE_QUALITY_ALL;

        Image->format         = vgFormat;
        Image->upsample       = upsample;
        Image->wrapperFormat  = wrapperFormat;
        Image->surfaceFormat  = surfaceFormat;
        Image->orientation    = orientation;
        Image->surOrientation = orientation;

        Image->glyph        = 0;
        Image->pattern      = 0;
        Image->renderTarget = 0;

        Image->parent        = Image;
        Image->childrenCount = 0;
        Image->surface       = Surface;
        Image->buffer        = buffer[0];

        Image->imageAllocated   = gcvFALSE;
        Image->surfaceAllocated = gcvFALSE;
        Image->surfaceLocked    = gcvTRUE;
        Image->imageDirty       = vgvIMAGE_READY;
        Image->imageDirtyPtr    = &Image->imageDirty;

        /* Assume the surface arguments are valid by default. */
        Image->surfArgValid     = gcvTRUE;
    }
    while (gcvFALSE);

    vgmLEAVESUBAPI(vgfInitializeImage);
    /* Return status. */
    return status;
}


/*******************************************************************************
**
** vgfInitializeWrapper
**
** Initialize the image wrapper.
**
** INPUT:
**
**    Context
**       Pointer to the context.
**
**    DataFormat
**       Format of the image.
**
**    DataStride
**       Stride of the image.
**
**    Width, Height
**       The size of the image.
**
**    Logical
**       Pointer to the image bits.
**
**    Physical
**       Physical address of the image bits.
**
** OUTPUT:
**
**    Wrapper
**       Pointer to the initialized image.
*/

gceSTATUS vgfInitializeWrapper(
    IN vgsCONTEXT_PTR Context,
    IN vgsIMAGE_PTR Wrapper,
    IN gctBOOL      ForHWUse,
    IN VGImageFormat DataFormat,
    IN vgsFORMAT_PTR FormatInfo,
    IN gctINT DataStride,
    IN gctINT Width,
    IN gctINT Height,
    IN gctCONST_POINTER Logical,
    IN gctUINT32 Physical
    )
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgfInitializeWrapper);

    do
    {
        gceSURF_COLOR_TYPE colorType;
        gctUINT alignment;

        /* Validate the size. */
        if ((Width <= 0) || (Height <= 0))
        {
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        /* Determine pixel alignment. */
        alignment = (FormatInfo->bitsPerPixel + 7) / 8;

        /* Data pointer cannot be NULL. */
        if (vgmIS_INVALID_PTR(Logical, alignment))
        {
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        /* Construct the wrapper surface. */
        if (Wrapper->surface == gcvNULL)
        {
            gcmERR_BREAK(gcoSURF_ConstructWrapper(
                Context->hal,
                &Wrapper->surface
                ));

            /* Initialize the object. */
            Wrapper->object.type           = vgvOBJECTTYPE_IMAGE;
            Wrapper->object.prev           = gcvNULL;
            Wrapper->object.next           = gcvNULL;
            Wrapper->object.referenceCount = 1;
            Wrapper->object.userValid      = VG_FALSE;

            /* Set image parameters. */
            Wrapper->origin.x = 0;
            Wrapper->origin.y = 0;

            Wrapper->allowedQuality = vgvIMAGE_QUALITY_ALL;
            Wrapper->upsample       = gcvFALSE;
            Wrapper->orientation    = vgvIMAGE_ORIENTATION;

            Wrapper->glyph        = 0;
            Wrapper->pattern      = 0;
            Wrapper->renderTarget = 0;

            Wrapper->parent        = Wrapper;
            Wrapper->childrenCount = 0;
            Wrapper->surfArgValid = gcvTRUE;

            Wrapper->imageAllocated   = gcvFALSE;
            Wrapper->surfaceAllocated = gcvTRUE;
            Wrapper->surfaceLocked    = gcvFALSE;

            /* Set default orientation. */
            Wrapper->surOrientation = vgvIMAGE_ORIENTATION;
        }

        /* Set the underlying frame buffer surface. */
        gcmERR_BREAK(gcoSURF_SetBuffer(
            Wrapper->surface,
            gcvSURF_BITMAP,
            FormatInfo->internalFormat,
            DataStride,
            (gctPOINTER) Logical,
            Physical
            ));

        /* Set the window. */
        if (ForHWUse)
        {
        gcmERR_BREAK(gcoSURF_SetWindow(
            Wrapper->surface,
            0, 0,
            Width, Height
            ));
        }

        /* Set the proper color space. */
        colorType = gcvSURF_COLOR_UNKNOWN;

        if (FormatInfo->linear)
        {
            colorType |= gcvSURF_COLOR_LINEAR;
        }

        if (FormatInfo->premultiplied)
        {
            colorType |= gcvSURF_COLOR_ALPHA_PRE;
        }

        gcmERR_BREAK(gcoSURF_SetColorType(
            Wrapper->surface,
            colorType
            ));

        /* Set image parameters. */
        Wrapper->size.width  = Width;
        Wrapper->size.height = Height;
        Wrapper->stride      = DataStride;

        Wrapper->format        = DataFormat;
        Wrapper->wrapperFormat = FormatInfo;
        Wrapper->surfaceFormat = FormatInfo;

        Wrapper->buffer     = (gctUINT8_PTR) Logical;
        Wrapper->imageDirty = vgvIMAGE_READY;

        Wrapper->imageDirtyPtr = &Wrapper->imageDirty;
    }
    while (gcvFALSE);

    vgmLEAVESUBAPI(vgfInitializeWrapper);
    /* Return status. */
    return status;
}


/*******************************************************************************
**
** vgfInitializeTempImage
**
** Initialize the temporary image.
**
** INPUT:
**
**    Context
**       Pointer to the context.
**
**    DataFormat
**       Format of the image.
**
**    Width, Height
**       The size of the image.
**
** OUTPUT:
**
**    Nothing.
*/

gceSTATUS vgfInitializeTempImage(
    IN vgsCONTEXT_PTR Context,
    IN VGImageFormat DataFormat,
    IN gctINT Width,
    IN gctINT Height
    )
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgfInitializeTempImage);

    do
    {
        vgsFORMAT_PTR surfaceFormat;
        gctUINT surfaceSize;
        gctUINT alignedWidth, alignedHeight;
        gctUINT stride;

        /* Get the format information. */
        surfaceFormat = vgfGetFormatInfo(Context, DataFormat);

        if (surfaceFormat == gcvNULL)
        {
            vgmERROR(VG_UNSUPPORTED_IMAGE_FORMAT_ERROR);
            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }

        /* Init the aligned surface size. */
        alignedWidth  = Width;
        alignedHeight = Height;

#if gcdGC355_PROFILER
        /* Align the image size. */
        gcmERR_BREAK(gcoHAL_GetAlignedSurfaceSize(
            Context->hal,
            Context->vg,
            Context->TreeDepth, Context->saveLayerTreeDepth, Context->varTreeDepth,
            gcvSURF_BITMAP, &alignedWidth, &alignedHeight
            ));
#else
        gcmERR_BREAK(gcoHAL_GetAlignedSurfaceSize(
            Context->hal,
            gcvSURF_BITMAP, &alignedWidth, &alignedHeight
            ));
#endif

        /* Determine the stride. */
        stride = (alignedWidth * surfaceFormat->bitsPerPixel) / 8;

        /* Compute the size of the surface needed. */
        surfaceSize = stride * alignedHeight;

        /* Is the existing surface too small? */
        if (Context->tempSize < surfaceSize)
        {
            /* Is there anything allocated? */
            if (Context->tempNode != 0)
            {
#if gcdGC355_PROFILER
                /* Schedule the current buffer for deletion. */
                gcmERR_BREAK(gcoHAL_ScheduleVideoMemory(
                    Context->hal,
                    Context->vg,
                    Context->TreeDepth, Context->saveLayerTreeDepth, Context->varTreeDepth,
                    Context->tempNode
                    ));
#else
                gcmERR_BREAK(gcoHAL_ScheduleVideoMemory(
                    Context->hal,
                    Context->tempNode
                    ));
#endif

                /* Reset temporary surface. */
                Context->tempNode     = 0;
                Context->tempPhysical = ~0;
                Context->tempLogical  = gcvNULL;
                Context->tempSize     = 0;
            }

            /* Make a nice aligned size. */
            surfaceSize = gcmALIGN(surfaceSize, 4096);

#if gcdGC355_PROFILER
            /* Try to allocate the new buffer. */
            gcmERR_BREAK(gcoHAL_AllocateLinearVideoMemory(
                Context->hal,
                Context->vg,
                Context->TreeDepth,
                Context->saveLayerTreeDepth,
                Context->varTreeDepth,
                surfaceSize,
                64,
                gcvPOOL_DEFAULT,
                &Context->tempNode,
                &Context->tempPhysical,
                (gctPOINTER *) &Context->tempLogical
                ));
#else
            gcmERR_BREAK(gcoHAL_AllocateLinearVideoMemory(
                Context->hal,
                surfaceSize,
                64,
                gcvPOOL_DEFAULT,
                &Context->tempNode,
                &Context->tempPhysical,
                (gctPOINTER *) &Context->tempLogical
                ));
#endif

            /* Store the new size. */
            Context->tempSize = surfaceSize;
        }

        /* Initialize the wrapper surface. */
        gcmERR_BREAK(vgfInitializeWrapper(
            Context,
            &Context->tempImage,
            gcvTRUE,
            DataFormat,
            surfaceFormat,
            stride,
            Width, Height,
            Context->tempLogical,
            Context->tempPhysical
            ));
    }
    while (gcvFALSE);

    vgmLEAVESUBAPI(vgfInitializeTempImage);
    /* Return status. */
    return status;
}


/*******************************************************************************
**
** vgfCreateImage
**
** Create an image object.
**
** INPUT:
**
**    Context
**       Pointer to the context.
**
**    DataFormat
**       Format of the image.
**
**    Width, Height
**       The size of the image.
**
**    AllowedQuality
**       Allowed quality of the image.
**
** OUTPUT:
**
**    Image
**       Pointer to the initialized image.
*/

gceSTATUS vgfCreateImage(
    IN vgsCONTEXT_PTR Context,
    IN VGImageFormat DataFormat,
    IN gctINT Width,
    IN gctINT Height,
    IN VGImageQuality AllowedQuality,
    IN vgsIMAGE_PTR * Image,
    IN gctPOINTER     UserMemory
    )
{
    gceSTATUS status;
    gcoSURF surface = gcvNULL;
    gctPOINTER buffer[3] = {gcvNULL};
    vgsIMAGE_PTR image;
    gctBOOL imageAllocated;
    vgmENTERSUBAPI(vgfCreateImage);

    do
    {
        gctBOOL upsample;
        vgsFORMAT_PTR wrapperFormat;
        vgsFORMAT_PTR surfaceFormat;
        gceSURF_COLOR_TYPE colorType;
        gctINT stride;

        /* Assume image as locally allocated. */
        imageAllocated = gcvTRUE;

        /* Get the pointer to the image object. */
        image = * Image;

        /* Not yet allocated? */
        if (image == gcvNULL)
        {
            /* Allocate an image object. */
            status = vgfReferenceImage(Context, &image);

            if (gcmIS_ERROR(status))
            {
                vgmERROR(VG_OUT_OF_MEMORY_ERROR);
                break;
            }

            /* Set the image pointer. */
            * Image = image;
        }
        else
        {
            /* Initialize the object. */
            image->object.type           = vgvOBJECTTYPE_IMAGE;
            image->object.prev           = gcvNULL;
            image->object.next           = gcvNULL;
            image->object.referenceCount = 1;
            image->object.userValid      = VG_FALSE;

            /* Mark image as allocated somewhere else. */
            imageAllocated = gcvFALSE;
        }

        /* Read only? */
        (*Image)->readOnly = UserMemory ? gcvTRUE : gcvFALSE;

        /* Get the format information. */
        wrapperFormat = vgfGetFormatInfo(Context, DataFormat);
        gcmASSERT(wrapperFormat != gcvNULL);

        /* Determine whether upsampling is required. */
        upsample = (wrapperFormat->upsampledFormat != gcvNULL);

        /* Determine upsampled format. */
        surfaceFormat = upsample
            ? wrapperFormat->upsampledFormat
            : wrapperFormat;

        if (UserMemory != gcvNULL)
        {
            status = gcoSURF_ConstructWrapper(Context->hal, &surface);
            if (gcmIS_ERROR(status))
            {
                vgmERROR(VG_OUT_OF_MEMORY_ERROR);
                break;
            }

            status = gcoSURF_SetBuffer(surface,
                        gcvSURF_BITMAP,
                        surfaceFormat->internalFormat,
                        ~0U, UserMemory, gcvINVALID_ADDRESS);
            if (gcmIS_ERROR(status))
            {
                vgmERROR(VG_OUT_OF_MEMORY_ERROR);
                break;
            }

            status = gcoSURF_SetWindow(surface, 0, 0, Width, Height);
            if (gcmIS_ERROR(status))
            {
                vgmERROR(VG_OUT_OF_MEMORY_ERROR);
                break;
            }

            status = gcoSURF_CPUCacheOperation(surface, gcvCACHE_FLUSH);
        }
        else
        {
            /* Allocate the surface. */
            status = gcoSURF_Construct(
                Context->hal,
                Width, Height, 1,
                gcvSURF_BITMAP,
                surfaceFormat->internalFormat,
                gcvPOOL_DEFAULT,
                &surface
                );
        }

        if (gcmIS_ERROR(status))
        {
            vgmERROR(VG_OUT_OF_MEMORY_ERROR);
            break;
        }

        /* Get the logical pointer to the surface. */
        status = gcoSURF_Lock(
            surface, gcvNULL, buffer
            );

        if (gcmIS_ERROR(status))
        {
            vgmERROR(VG_OUT_OF_MEMORY_ERROR);
            break;
        }

        /* Get the surface stride. */
        status = gcoSURF_GetAlignedSize(
            surface, gcvNULL, gcvNULL, &stride
            );

        if (gcmIS_ERROR(status))
        {
            vgmERROR(VG_OUT_OF_MEMORY_ERROR);
            break;
        }

        /* Set the proper color space. */
        colorType = gcvSURF_COLOR_UNKNOWN;

        if (surfaceFormat->linear)
        {
            colorType |= gcvSURF_COLOR_LINEAR;
        }

        if (surfaceFormat->premultiplied)
        {
            colorType |= gcvSURF_COLOR_ALPHA_PRE;
        }

        status = gcoSURF_SetColorType(surface, colorType);

        if (gcmIS_ERROR(status))
        {
            vgmERROR(VG_OUT_OF_MEMORY_ERROR);
            break;
        }

        /* Set image parameters. */
        image->size.width  = Width;
        image->size.height = Height;
        image->origin.x    = 0;
        image->origin.y    = 0;
        image->stride      = stride;

        image->allowedQuality = AllowedQuality;

        image->format         = DataFormat;
        image->upsample       = upsample;
        image->wrapperFormat  = wrapperFormat;
        image->surfaceFormat  = surfaceFormat;
        image->orientation    = vgvIMAGE_ORIENTATION;
        image->surOrientation = vgvIMAGE_ORIENTATION;

        image->glyph        = 0;
        image->pattern      = 0;
        image->renderTarget = 0;

        image->parent        = image;
        image->childrenCount = 0;
        image->surface       = surface;
        image->buffer        = (gctUINT8_PTR)(buffer[0]);

        image->imageAllocated   = imageAllocated;
        image->surfaceAllocated = gcvTRUE;
        image->surfaceLocked    = gcvTRUE;
        image->imageDirty       = vgvIMAGE_READY;
        image->imageDirtyPtr    = &image->imageDirty;
        image->surfArgValid     = gcvTRUE;
        /* Success. */
        status = gcvSTATUS_OK;
        goto OnSuccess;
    }
    while (gcvFALSE);

    /* Roll back. */
    if (buffer[0] != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(surface, buffer[0]));
    }

    if (surface != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(surface));
    }

    if (imageAllocated)
    {
        gcmVERIFY_OK(vgfDereferenceObject(Context, (vgsOBJECT_PTR *) &image));
    }

OnSuccess:
    break;
    vgmLEAVESUBAPI(vgfCreateImage);
    /* Return status. */
    return status;
}


/*******************************************************************************
**
** vgfReleaseImage
**
** vgfReleaseImage reverses the effect of vgfInitializeImage.
**
** INPUT:
**
**    Image
**       Pointer to the image object.
**
** OUTPUT:
**
**    Nothing.
*/

gceSTATUS vgfReleaseImage(
    IN vgsCONTEXT_PTR Context,
    IN vgsIMAGE_PTR Image
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    vgmENTERSUBAPI(vgfReleaseImage);

    do
    {
        /* Unlock. */
        if (Image->surfaceLocked)
        {
            gcmASSERT(Image->buffer != gcvNULL);

            gcmERR_BREAK(gcoSURF_Unlock(
                Image->surface, Image->buffer
                ));

            Image->buffer        = gcvNULL;
            Image->surfaceLocked = gcvFALSE;
        }

        /* Destroy the surface. */
        if (Image->surfaceAllocated)
        {

#if gcdENABLE_3D
            gcsTLS_PTR tls;
            gcmERR_BREAK(gcoOS_GetTLS(&tls));

            /* Unset VG target. */
            if (tls->engineVG != gcvNULL)
            {
#if gcdGC355_PROFILER
                gcmERR_BREAK(
                    gcoVG_UnsetTarget(tls->engineVG,
                    0,0,0,
                    Image->surface));
#else
                gcmERR_BREAK(
                    gcoVG_UnsetTarget(tls->engineVG,
                    Image->surface));
#endif
            }
#endif

            gcmERR_BREAK(gcoSURF_Destroy(
                Image->surface
                ));

            Image->surfaceAllocated = gcvFALSE;
        }

        /* Reset the surface handle. */
        Image->surface = gcvNULL;

        /* Dereference the image object. */
        if (Image->imageAllocated)
        {
            gcmERR_BREAK(vgfDereferenceObject(
                Context, (vgsOBJECT_PTR *) &Image
                ));
            gcmASSERT(Image == gcvNULL);
        }

    }
    while (gcvFALSE);

    vgmLEAVESUBAPI(vgfReleaseImage);
    /* Return status. */
    return status;
}


/*******************************************************************************
**
** vgfUseImageAsGlyph
**
** If Enable is not zero, the function will attempt to designate the given
** image object for use as a glyph image.
**
** If Enable is zero, the function will release the given image object from
** the duty of being a glyph image.
**
** INPUT:
**
**    Context
**       Pointer to the context.
**
**    Image
**       Pointer to the image object.
**
**    Enable
**       Designate/release selection.
**
** OUTPUT:
**
**    Nothing.
*/

gceSTATUS vgfUseImageAsGlyph(
    vgsCONTEXT_PTR Context,
    vgsIMAGE_PTR Image,
    VGboolean Enable
    )
{
    /* Assume success. */
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Context=0x%x Image=0x%x Enable=0x%x", Context, Image, Enable);
    /* Verify the image handle. */
    gcmVERIFY_ARGUMENT(Image != gcvNULL);
    {
        vgmENTERSUBAPI(vgfUseImageAsGlyph);

        do
        {
            /* Update the usage flag. */
            if (Enable)
            {
                if (Image->renderTarget > 0)
                {
                    gcmERR_BREAK(gcvSTATUS_INVALID_REQUEST);
                }
                else
                {
                    /* Increment image reference count. */
                    gcmVERIFY_OK(vgfReferenceImage(
                        Context, &Image
                        ));

                    /* Increment glyph reference count. */
                    Image->glyph++;
                }
            }
            else
            {
                if (Image->glyph > 0)
                {
                    /* Decrement glyph reference count. */
                    Image->glyph--;

                    /* Decrement image reference count. */
                    vgfDereferenceObject(
                        Context, (vgsOBJECT_PTR *) &Image
                        );
                }
                else
                {
                    gcmERR_BREAK(gcvSTATUS_INVALID_REQUEST);
                }
            }
        }
        while (gcvFALSE);

        vgmLEAVESUBAPI(vgfUseImageAsGlyph);
    }
    /* Return status. */
    return status;
}


/*******************************************************************************
**
** vgfUseImageAsPattern
**
** If Enable is not zero, the function will attempt to designate the given
** image object for use as a pattern.
**
** If Enable is zero, the function will release the given image object from
** the duty of being a pattern.
**
** INPUT:
**
**    Context
**       Pointer to the context.
**
**    Image
**       Pointer to the image object.
**
**    Enable
**       Designate/release selection.
**
** OUTPUT:
**
**    Nothing.
*/

gceSTATUS vgfUseImageAsPattern(
    vgsCONTEXT_PTR Context,
    vgsIMAGE_PTR Image,
    VGboolean Enable
    )
{
    /* Assume success. */
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Context=0x%x Image=0x%x Enable=0x%x", Context, Image, Enable);
    /* Verify the image handle. */
    gcmVERIFY_ARGUMENT(Image != gcvNULL);
    {
        vgmENTERSUBAPI(vgfUseImageAsPattern);
        do
        {
            /* Update the usage flag. */
            if (Enable)
            {
                if (Image->renderTarget > 0)
                {
                    gcmERR_BREAK(gcvSTATUS_INVALID_REQUEST);
                }
                else
                {
                    /* Increment image reference count. */
                    gcmVERIFY_OK(vgfReferenceImage(
                        Context, &Image
                        ));

                    /* Increment pattern reference count. */
                    Image->pattern++;
                }
            }
            else
            {
                if (Image->pattern > 0)
                {
                    /* Decrement pattern reference count. */
                    Image->pattern--;

                    /* Decrement image reference count. */
                    gcmERR_BREAK(vgfDereferenceObject(
                        Context, (vgsOBJECT_PTR *) &Image
                        ));
                }
                else
                {
                    gcmERR_BREAK(gcvSTATUS_INVALID_REQUEST);
                }
            }
        }
        while (gcvFALSE);

        vgmLEAVESUBAPI(vgfUseImageAsPattern);
    }
    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
** vgfUseImageAsRenderTarget
**
** If Enable is not zero, the function will attempt to designate the given
** image object for use as the render target.
**
** If Enable is zero, the function will release the given image object from
** the duty of being the render target.
**
** INPUT:
**
**    Context
**       Pointer to the context.
**
**    Image
**       Pointer to the image object.
**
**    Enable
**       Designate/release selection.
**
** OUTPUT:
**
**    Nothing.
*/

gceSTATUS vgfUseImageAsRenderTarget(
    vgsCONTEXT_PTR Context,
    vgsIMAGE_PTR Image,
    VGboolean Enable
    )
{
    /* Assume success. */
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Context=0x%x Image=0x%x Enable=0x%x", Context, Image, Enable);
    /* Verify the image handle. */
    gcmVERIFY_ARGUMENT(Image != gcvNULL);
    {
        vgmENTERSUBAPI(vgfUseImageAsRenderTarget);

        do
        {
            /* Update the usage flag. */
            if (Enable)
            {
                if ((Image->glyph > 0) || (Image->pattern > 0))
                {
                    gcmERR_BREAK(gcvSTATUS_INVALID_REQUEST);
                }
                else
                {
                    /* Increment image reference count. */
                    gcmVERIFY_OK(vgfReferenceImage(
                        Context, &Image
                        ));

                    /* Increment render target reference count. */
                    Image->renderTarget++;
                }
            }
            else
            {
                if (Image->renderTarget > 0)
                {
                    /* Decrement render target reference count. */
                    Image->renderTarget--;

                    /* Decrement image reference count. */
                    gcmERR_BREAK(vgfDereferenceObject(
                        Context, (vgsOBJECT_PTR *) &Image
                        ));
                }
                else
                {
                    gcmERR_BREAK(gcvSTATUS_INVALID_REQUEST);
                }
            }
        }
        while (gcvFALSE);
        vgmLEAVESUBAPI(vgfUseImageAsRenderTarget);
    }
    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
** vgfIsImageRenderTarget
**
** Verifies whether the specified image is currently used as the render target.
**
** INPUT:
**
**    Image
**       Pointer to the image object.
**
** OUTPUT:
**
**    Returns not zero if the specified image is currently in use as the
**    render target.
*/

gctBOOL vgfIsImageRenderTarget(
    vgsCONTEXT_PTR Context,
    vgsIMAGE_PTR Image
    )
{
    vgmENTERSUBAPI(vgfIsImageRenderTarget);
    vgmLEAVESUBAPI(vgfIsImageRenderTarget);
    return (Image->renderTarget > 0);
}


/*******************************************************************************
**
** vgfGetRootImage
**
** Returns the root image for the specified image.
**
** INPUT:
**
**    Image
**       Pointer to the image object.
**
** OUTPUT:
**
**    Root image object.
*/

vgsIMAGE_PTR vgfGetRootImage(
    vgsCONTEXT_PTR Context,
    vgsIMAGE_PTR Image
    )
{
    vgsIMAGE_PTR root;
    vgmENTERSUBAPI(vgfGetRootImage);
    /* Cannot be NULL. */
    gcmASSERT(Image != gcvNULL);

    /* Start with the given image. */
    root = Image;

    /* Go down the chain. */
    while (root->parent != root)
    {
        root = root->parent;
    }
    vgmLEAVESUBAPI(vgfGetRootImage);
    /* Return result. */
    return root;
}


/*******************************************************************************
**
** vgfImagesRelated
**
** Verifies whether the specified images are related.
**
** INPUT:
**
**    First
**    Second
**       Pointers to the image objects.
**
** OUTPUT:
**
**    Returns not zero if the specified images are related.
*/

gctBOOL vgfImagesRelated(
    vgsCONTEXT_PTR Context,
    vgsIMAGE_PTR First,
    vgsIMAGE_PTR Second
    )
{
    vgsIMAGE_PTR firstRoot;
    vgsIMAGE_PTR secondRoot;
    vgmENTERSUBAPI(vgfImagesRelated);
    /* Get roots. */
    firstRoot  = vgfGetRootImage(Context, First);
    secondRoot = vgfGetRootImage(Context, Second);
    vgmLEAVESUBAPI(vgfImagesRelated);
    /* Determine relationship. */
    return (firstRoot == secondRoot);
}


/*******************************************************************************
**
** vgfFlushImage
**
** Flush and stall the hardware if needed.
**
** INPUT:
**
**    Context
**       Pointer to the context.
**
**    Image
**       Pointers to the image object to be used as the source.
**
**    Finish
**       If not zero, makes sure the hardware is idle.
**
** OUTPUT:
**
**    Nothing.
*/

gceSTATUS vgfFlushImage(
    IN vgsCONTEXT_PTR Context,
    IN vgsIMAGE_PTR Image,
    IN gctBOOL Finish
    )
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgfFlushImage);

    do
    {
        /* Did the image change? */
        if (*Image->imageDirtyPtr == vgvIMAGE_READY)
        {
            /* No change, return. */
            status = gcvSTATUS_OK;
            break;
        }

        /* Was the hardware already flushed and stalled? */
        if (Context->imageDirty == vgvIMAGE_READY)
        {
            /* Reset the image state. */
            *Image->imageDirtyPtr = vgvIMAGE_READY;

            /* Success. */
            status = gcvSTATUS_OK;
            break;
        }

        /* Does the image need flushing? */
        if (((Context->imageDirty & vgvIMAGE_NOT_FLUSHED) != 0) ||
            ((*Image->imageDirtyPtr   & vgvIMAGE_NOT_FLUSHED) != 0))
        {
#if gcdGC355_PROFILER
            /* Flush the surface. */
            gcmERR_BREAK(gcoHAL_Flush(Context->hal,
                            Context->vg,
                            Context->TreeDepth,
                            Context->saveLayerTreeDepth,
                            Context->varTreeDepth
                            ));
#else
            gcmERR_BREAK(gcoHAL_Flush(Context->hal
                            ));
#endif
        }

        /* Need to stall the hardware as well? */
        if (Finish)
        {
            /* Make sure the hardware is idle. */
            gcmERR_BREAK(gcoHAL_Commit(Context->hal, gcvTRUE));

            /* Reset the states. */
            *Image->imageDirtyPtr   = vgvIMAGE_READY;
            Context->imageDirty     = vgvIMAGE_READY;
        }
        else
        {
            /* Mark the states as not finished. */
            *Image->imageDirtyPtr   = vgvIMAGE_NOT_FINISHED;
            Context->imageDirty     = vgvIMAGE_NOT_FINISHED;

            /* Success. */
            status = gcvSTATUS_OK;
        }
    }
    while (gcvFALSE);

    vgmLEAVESUBAPI(vgfFlushImage);
    /* Return state. */
    return status;
}


gctFLOAT vgfDitherChannel(
    vgsCONTEXT_PTR Context,
    gctFLOAT Channel,
    gctINT32 Bits,
    gctFLOAT Value)
{
    gctFLOAT fc, ic;
    vgmENTERSUBAPI(vgfDitherChannel);
    fc = Channel * (gctFLOAT)((1<<Bits)-1);
    ic = (gctFLOAT)floor(fc);
    if(fc - ic > Value) ic += 1.0f;
    vgmLEAVESUBAPI(vgfDitherChannel);
    return gcmMIN(ic / (gctFLOAT)((1<<Bits)-1), 1.0f);
}

/*******************************************************************************
**
** vgfCPUBlit
**
** Blit a rectangular area from one image to another.
**
** INPUT:
**
**    Context
**       Pointer to the context.
**
**    Source
**       Pointer to the source image object.
**
**    Target
**       Pointer to the target image object.
**
**    SourceX, SourceY
**       Origin within the source surface.
**
**    TargetX, TargetY
**       Origin within the source surface.
**
**    Width, Height
**       The size of the area.
**
** OUTPUT:
**
**    Nothing.
*/

gceSTATUS vgfCPUBlit(
    IN vgsCONTEXT_PTR Context,
    IN vgsIMAGE_PTR Source,
    IN vgsIMAGE_PTR Target,
    IN gctINT SourceX,
    IN gctINT SourceY,
    IN gctINT TargetX,
    IN gctINT TargetY,
    IN gctINT Width,
    IN gctINT Height,
    IN gctBOOL Dither
    )
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgfCPUBlit);

    do
    {
        gctBOOL normalized;
        gctUINT readPixelIndex;
        gctUINT writePixelIndex;
        vgtREAD_PIXEL readPixel;
        vgtWRITE_PIXEL writePixel;
        vgsPIXELWALKER srcPixel;
        vgsPIXELWALKER trgPixel;
        vgtFLOATVECTOR4 pixelValue;
        gctINT32 x, y;

        /* Normalize coordinates. */
        normalized = vgfNormalizeCoordinatePairs(
            Context,
            &SourceX, &SourceY,
            &TargetX, &TargetY,
            &Width, &Height,
            &Source->size,
            &Target->size
            );

        if (!normalized)
        {
            /* Normalization failed. */
            status = gcvSTATUS_OK;
            break;
        }

        /* Get the source image. */
        gcmERR_BREAK(_GetSource(
            Context,
            &Source, Target,
            &SourceX, &SourceY,
            TargetX, TargetY,
            Width, Height,
            gcvTRUE
            ));

        /* Determine the pixel access routines. */
        readPixelIndex
            = (Target->surfaceFormat->linear << 1)
            |  Target->surfaceFormat->premultiplied;

        writePixelIndex
            = (readPixelIndex << 4)
            |  vgvRGBA;

        readPixel
            = Source->surfaceFormat->readPixel[readPixelIndex];

        writePixel
            = Target->surfaceFormat->writePixel[writePixelIndex];

        /* Flush the images. */
        gcmERR_BREAK(vgfFlushImage(Context, Source, gcvTRUE));
        gcmERR_BREAK(vgfFlushImage(Context, Target, gcvTRUE));

        /* Initialize walkers. */
        vgsPIXELWALKER_Initialize(Context, &srcPixel, Source, SourceX, SourceY);
        vgsPIXELWALKER_Initialize(Context, &trgPixel, Target, TargetX, TargetY);

        /* If the formats are the same, do the fast copy. */
        if ((Source->format == Target->format) &&
            (Source->surfaceFormat->internalFormat == Target->surfaceFormat->internalFormat ) &&
            !Context->conformance && !Dither)
        {
            /* Determine the number of bytes to copy. */
            gctUINT copyBytes
                = gcmALIGN(Width * Source->surfaceFormat->bitsPerPixel, 8) >> 3;

            /* Cannot be zero bytes. */
            gcmASSERT(copyBytes != 0);

            /* Walk the rows. */
            for (y = 0; y < Height; y++)
            {
                /* Copy the entire row. */
                gcoOS_MemCopy(
                    trgPixel.current, srcPixel.current, copyBytes
                    );

                /* Advance to the next line. */
                vgsPIXELWALKER_NextLine(Context, &srcPixel);
                vgsPIXELWALKER_NextLine(Context, &trgPixel);
            }
        }
        else
        {
            vgsFORMAT_PTR surfaceFormat = vgfGetFormatInfo(Context, Target->format);
            gctINT32 rbits = surfaceFormat->r.width;
            gctINT32 gbits = surfaceFormat->g.width;
            gctINT32 bbits = surfaceFormat->b.width;
            gctINT32 abits = surfaceFormat->a.width;

            if(surfaceFormat->luminance)
            {
                gbits = bbits = rbits;
                abits = 0;
            }

            /* Walk the pixels. */
            for (y = 0; y < Height; y++)
            {
                for (x = 0; x < Width; x++)
                {
                    /* Read the pixel and advance to the next. */
                    readPixel(&srcPixel, pixelValue);

                    if(Dither)
                    {
                        static const gctINT32 matrix[16] = {
                            0,  8,  2,  10,
                            12, 4,  14, 6,
                            3,  11, 1,  9,
                            15, 7,  13, 5};

                        gctFLOAT m = matrix[(y & 3) * 4 + (x & 3)] / 16.0f;

                        if (Target->surfaceFormat->grayscale)
                        {
                            gcmASSERT(!Target->surfaceFormat->premultiplied);

                            pixelValue[0] =
                            pixelValue[1] =
                            pixelValue[2] = vgfGetGrayScale(pixelValue[0], pixelValue[1], pixelValue[2]);
                        }

                        if(rbits) pixelValue[0] = vgfDitherChannel(Context, pixelValue[0], rbits, m);
                        if(gbits) pixelValue[1] = vgfDitherChannel(Context, pixelValue[1], gbits, m);
                        if(bbits) pixelValue[2] = vgfDitherChannel(Context, pixelValue[2], bbits, m);
                        if(abits) pixelValue[3] = vgfDitherChannel(Context, pixelValue[3], abits, m);
                    }

                    /* Write the result pixel and advance to the next. */
                    writePixel(&trgPixel, pixelValue, vgvRGBA);
                }

                /* Advance to the next line. */
                vgsPIXELWALKER_NextLine(Context, &srcPixel);
                vgsPIXELWALKER_NextLine(Context, &trgPixel);
            }
        }
    }
    while (gcvFALSE);

    vgmLEAVESUBAPI(vgfCPUBlit);
    /* Return status. */
    return status;
}


/*******************************************************************************
**
** vgfCPUFill
**
** Fill a rectangular area of an image.
**
** INPUT:
**
**    Context
**       Pointer to the context.
**
**    Target
**       Pointer to the target image object.
**
**    UpSampleTarget
**       Target upsampling enable.
**
**    TargetX, TargetY
**       Origin within the source surface.
**
**    Width, Height
**       The size of the area.
**
**    Red, Green, Blue, Alpha
**       Color value in non-premultiplied sRGBA (sRGB color plus alpha) format.
**
** OUTPUT:
**
**    Nothing.
*/

gceSTATUS vgfCPUFill(
    IN vgsCONTEXT_PTR Context,
    IN vgsIMAGE_PTR Target,
    IN gctINT TargetX,
    IN gctINT TargetY,
    IN gctINT Width,
    IN gctINT Height,
    IN gctFLOAT Red,
    IN gctFLOAT Green,
    IN gctFLOAT Blue,
    IN gctFLOAT Alpha
    )
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgfCPUFill);

    do
    {
        gctBOOL normalized;
        gctUINT writePixelIndex;
        vgtWRITE_PIXEL writePixel;
        vgsPIXELWALKER trgPixel;
        vgtFLOATVECTOR4 pixelValue;
        vgsFORMAT_PTR format;
        gctINT32 x, y;

        /* Normalize coordinates. */
        normalized = vgfNormalizeCoordinates(
            Context,
            &TargetX, &TargetY,
            &Width, &Height,
            &Target->size
            );

        if (!normalized)
        {
            /* Normalization failed. */
            status = gcvSTATUS_OK;
            break;
        }

        /* Get shortcut to the image format. */
        format = Target->surfaceFormat;

        /* Set preliminary color value. */
        pixelValue[0] = Red;
        pixelValue[1] = Green;
        pixelValue[2] = Blue;
        pixelValue[3] = Alpha;

        /* Convert pixel value. */
        vgfConvertColor(
            Context,
            pixelValue, pixelValue,
            format->premultiplied,
            format->linear,
            format->grayscale
            );

        /* Determine the pixel access routines. */
        writePixelIndex
            = (format->linear        << 5)
            | (format->premultiplied << 4)
            |  vgvRGBA;

        writePixel
            = Target->surfaceFormat->writePixel[writePixelIndex];

        /* Flush the image. */
        gcmERR_BREAK(vgfFlushImage(Context, Target, gcvTRUE));

        /* Initialize walkers. */
        vgsPIXELWALKER_Initialize(Context, &trgPixel, Target, TargetX, TargetY);

        /* Walk the pixels. */
        for (y = 0; y < Height; y++)
        {
            for (x = 0; x < Width; x++)
            {
                /* Write the result pixel and advance to the next. */
                writePixel(&trgPixel, pixelValue, vgvRGBA);
            }

            /* Advance to the next line. */
            vgsPIXELWALKER_NextLine(Context, &trgPixel);
        }
    }
    while (gcvFALSE);

    vgmLEAVESUBAPI(vgfCPUFill);
    /* Return status. */
    return status;
}


/*******************************************************************************
**
** vgfTesselateImage
**
** Draw the specified image.
**
** INPUT:
**
**    Context
**       Pointer to the context.
**
**    Target
**       Pointer to the target image object.
**
**    Image
**       Handle to a valid VGImage object to be drawn.
**
** OUTPUT:
**
**    Nothing.
*/

gceSTATUS vgfTesselateImage(
    IN vgsCONTEXT_PTR Context,
    IN vgsIMAGE_PTR Target,
    IN vgsIMAGE_PTR Image
#if gcdMOVG
    ,
    IN gctINT   Width,
    IN gctINT   Height
#endif
    )
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgfTesselateImage);
    do
    {
        gcsVG_RECT rectangle;
        VGImageQuality imageQuality;
        gceIMAGE_FILTER halImageQuality;

        /* Setup the rectangle. */
        rectangle.x      = Image->origin.x;
        rectangle.y      = Image->origin.y;
        rectangle.width  = Image->size.width;
        rectangle.height = Image->size.height;

        /* Flush the source image. */
        gcmERR_BREAK(vgfFlushImage(
            Context, Image, gcvFALSE
            ));

#if gcdGC355_PROFILER
        /* Set target. */
        gcmERR_BREAK(gcoVG_SetTarget(
            Context->vg,
            Context->TreeDepth,
            Context->saveLayerTreeDepth,
            Context->varTreeDepth,
            Target->surface,
            Target->surOrientation
            ));

        /* VG_FILL_RULE */
        gcmERR_BREAK(gcoVG_SetFillRule(
            Context->vg,
            Context->TreeDepth,
            Context->saveLayerTreeDepth,
            Context->varTreeDepth,
            gcvVG_NON_ZERO
            ));

#else
        gcmERR_BREAK(gcoVG_SetTarget(
            Context->vg,
            Target->surface,
            Target->surOrientation
            ));

        /* VG_FILL_RULE */
        gcmERR_BREAK(gcoVG_SetFillRule(
            Context->vg,
            gcvVG_NON_ZERO
            ));
#endif

        /* Update surface-to-image matrix. */
        Context->drawImageSurfaceToImage->update(Context);

        /* Update states. */
        gcmERR_BREAK(vgfUpdateStates(
            Context,
            Context->halImageMode,
            Context->halBlendMode,
            Context->colTransform,
            Context->scissoringEnable,
            Context->maskingEnable,
            gcvFALSE
            ));

        /* Update paint. */
        gcmERR_BREAK(vgfUpdatePaint(
            Context,
            Context->drawImageSurfaceToPaint,
            Context->fillPaint,
            gcvFALSE
            ));

        /* Determine the image quality. */
        imageQuality = Context->imageQuality & Image->allowedQuality;

        switch (imageQuality)
        {
        case VG_IMAGE_QUALITY_FASTER:
            halImageQuality = gcvFILTER_LINEAR;
            break;

        case VG_IMAGE_QUALITY_BETTER:
            halImageQuality = gcvFILTER_BI_LINEAR;
            break;

        default:
            halImageQuality = gcvFILTER_POINT;
        }

#if gcdMOVG
#if gcdGC355_PROFILER
        /* Draw the image. */
        gcmERR_BREAK(gcoVG_TesselateImage(
            Context->vg,
            Context->TreeDepth,
            Context->saveLayerTreeDepth,
            Context->varTreeDepth,
            Image->surface,
            &rectangle,
            halImageQuality,
            gcvFALSE,
            Context->useSoftwareTS,
            Context->halBlendMode,
            Width,
            Height
            ));
#else
        gcmERR_BREAK(gcoVG_TesselateImage(
            Context->vg,
            Image->surface,
            &rectangle,
            halImageQuality,
            gcvFALSE,
            Context->useSoftwareTS,
            Context->halBlendMode,
            Width,
            Height
            ));
#endif
#else
#if gcdGC355_PROFILER
        /* Draw the image. */
        gcmERR_BREAK(gcoVG_TesselateImage(
            Context->vg,
            Context->TreeDepth,
            Context->saveLayerTreeDepth,
            Context->varTreeDepth,
            Image->surface,
            &rectangle,
            halImageQuality,
            gcvFALSE,
            Context->useSoftwareTS
            ));
#else
        gcmERR_BREAK(gcoVG_TesselateImage(
            Context->vg,
            Image->surface,
            &rectangle,
            halImageQuality,
            gcvFALSE,
            Context->useSoftwareTS
            ));
#endif
#endif

        /* Arm the flush states. */
        *Image->imageDirtyPtr   = vgvIMAGE_NOT_FINISHED;
        *Target->imageDirtyPtr  = vgvIMAGE_NOT_READY;
        Context->imageDirty = vgvIMAGE_NOT_READY;
    }
    while (gcvFALSE);
    vgmLEAVESUBAPI(vgfTesselateImage);
    /* Return status. */
    return status;
}


/*******************************************************************************
**
** vgfDrawImage
**
** Draw the specified image.
**
** INPUT:
**
**    Context
**       Pointer to the context.
**
**    Source
**       Pointer to the source image object.
**
**    Target
**       Pointer to the target image object.
**
**    SourceX, SourceY
**       Source origin coordinates.
**
**    TargetX, TargetY
**       Target origin coordinates.
**
**    Width, Height
**       The size of the image to render.
**
**    BlendMode
**       Specifies the blending mode.
**
**    ColorTransformEnable
**       Color transformation enable.
**
**    ScissorEnable
**       Scissor enable flag.
**
**    MaskingEnable
**       Masking mode.
**
**    Mask
**       Image drawing in mask mode.
**
** OUTPUT:
**
**    Nothing.
*/

gceSTATUS vgfDrawImage(
    IN vgsCONTEXT_PTR Context,
    IN vgsIMAGE_PTR Source,
    IN vgsIMAGE_PTR Target,
    IN gctINT SourceX,
    IN gctINT SourceY,
    IN gctINT TargetX,
    IN gctINT TargetY,
    IN gctINT Width,
    IN gctINT Height,
    IN gceVG_BLEND BlendMode,
    IN gctBOOL ColorTransformEnable,
    IN gctBOOL ScissorEnable,
    IN gctBOOL MaskingEnable,
    IN gctBOOL DitherEnable,
    IN gctBOOL Mask,
    IN gctBOOL isDrawImage
    )
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgfDrawImage);
    do
    {
        /* Is the format supported for targets? */
#if gcdGC355_PROFILER
        if (gcoVG_IsTargetSupported(Context->vg, Context->TreeDepth, Context->saveLayerTreeDepth, Context->varTreeDepth, Target->surfaceFormat->internalFormat))
#else
        if (gcoVG_IsTargetSupported(Target->surfaceFormat->internalFormat))
#endif
        {
            gctBOOL normalized;

            /* Normalize coordinates. */
            normalized = vgfNormalizeCoordinatePairs(
                Context,
                &SourceX, &SourceY,
                &TargetX, &TargetY,
                &Width, &Height,
                &Source->size,
                &Target->size
                );

            if (!normalized)
            {
                /* Normalization failed. */
                status = gcvSTATUS_OK;
                break;
            }

            /* Get the source image. */
            gcmERR_BREAK(_GetSource(
                Context,
                &Source, Target,
                &SourceX, &SourceY,
                TargetX, TargetY,
                Width, Height,
                isDrawImage
                ));

            /* Flush the source image. */
            gcmERR_BREAK(vgfFlushImage(
                Context, Source, gcvFALSE
                ));

#if gcdGC355_PROFILER
            /* Set target. */
            gcmERR_BREAK(gcoVG_SetTarget(
                Context->vg,
                Context->TreeDepth,
                Context->saveLayerTreeDepth,
                Context->varTreeDepth,
                Target->surface,
                Target->surOrientation
                ));
#else
            gcmERR_BREAK(gcoVG_SetTarget(
                Context->vg,
                Target->surface,
                Target->surOrientation
                ));
#endif

            /* Update states. */
            gcmERR_BREAK(vgfUpdateStates(
                Context,
                gcvVG_IMAGE_NORMAL,
                BlendMode,
                ColorTransformEnable,
                ScissorEnable,
                MaskingEnable,
                DitherEnable
                ));

#if gcdGC355_PROFILER
            /* Draw the image. */
            gcmERR_BREAK(gcoVG_DrawImage(
                Context->vg,
                Context->TreeDepth,
                Context->saveLayerTreeDepth,
                Context->varTreeDepth,
                Source->surOrientation,
                Source->surface,
                &Source->origin,
                &Target->origin,
                &Source->size,
                SourceX, SourceY,
                TargetX, TargetY,
                Width, Height,
                Mask,
                isDrawImage
                ));
#else
            gcmERR_BREAK(gcoVG_DrawImage(
                Context->vg,
                Source->surOrientation,
                Source->surface,
                &Source->origin,
                &Target->origin,
                &Source->size,
                SourceX, SourceY,
                TargetX, TargetY,
                Width, Height,
                Mask,
                isDrawImage
                ));
#endif

            /* Arm the flush states. */
            *Source->imageDirtyPtr  = vgvIMAGE_NOT_FINISHED;
            *Target->imageDirtyPtr  = vgvIMAGE_NOT_READY;
            Context->imageDirty = vgvIMAGE_NOT_READY;
        }
        else
        {
            /* Copy pixels. */
            gcmERR_BREAK(vgfCPUBlit(
                Context,
                Source, Target,
                SourceX, SourceY,
                TargetX, TargetY,
                Width, Height, DitherEnable
                ));
        }
    }
    while (gcvFALSE);
    vgmLEAVESUBAPI(vgfDrawImage);
    /* Return status. */
    return status;
}


/*******************************************************************************
**
** vgfFillColor
**
** Fill the specified surface with a color.
**
** INPUT:
**
**    Context
**       Pointer to the context.
**
**    Target
**       Pointer to the target image object.
**
**    TargetX, TargetY, Width, Height
**       The area to fill.
**
**    FloatColor, ByteColor
**       Fill color value.
**
**    ScissorEnable
**       Scissor enable flag.
**
** OUTPUT:
**
**    Nothing.
*/

gceSTATUS vgfFillColor(
    IN vgsCONTEXT_PTR Context,
    IN vgsIMAGE_PTR Target,
    IN gctINT TargetX,
    IN gctINT TargetY,
    IN gctINT Width,
    IN gctINT Height,
    IN vgtFLOATVECTOR4 FloatColor,
    IN vgtBYTEVECTOR4 ByteColor,
    IN gctBOOL ScissorEnable
    )
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgfFillColor);
    do
    {
        /* Is the format supported for targets? */
#if gcdGC355_PROFILER
        if (gcoVG_IsTargetSupported(Context->vg, Context->TreeDepth, Context->saveLayerTreeDepth, Context->varTreeDepth, Target->surfaceFormat->internalFormat))
#else
        if (gcoVG_IsTargetSupported(Target->surfaceFormat->internalFormat))
#endif
        {
            gctBOOL normalized;

            /* Normalize coordinates. */
            normalized = vgfNormalizeCoordinates(
                Context,
                &TargetX, &TargetY,
                &Width, &Height,
                &Target->size
                );

            if (!normalized)
            {
                /* Normalization failed. */
                status = gcvSTATUS_OK;
                break;
            }

#if gcdGC355_PROFILER
            gcmERR_BREAK(gcoVG_SetTarget(
                Context->vg,
                Context->TreeDepth,
                Context->saveLayerTreeDepth,
                Context->varTreeDepth,
                Target->surface,
                Target->surOrientation
                ));

            gcmERR_BREAK(gcoVG_EnableMask(
                Context->vg,
                Context->TreeDepth,
                Context->saveLayerTreeDepth,
                Context->varTreeDepth,
                gcvFALSE
                ));

            gcmERR_BREAK(gcoVG_SetImageMode(
                Context->vg,
                Context->TreeDepth,
                Context->saveLayerTreeDepth,
                Context->varTreeDepth,
                gcvVG_IMAGE_NONE
                ));

            gcmERR_BREAK(gcoVG_SetBlendMode(
                Context->vg,
                Context->TreeDepth, Context->saveLayerTreeDepth, Context->varTreeDepth,
                gcvVG_BLEND_SRC
                ));

            gcmERR_BREAK(gcoVG_EnableScissor(
                Context->vg,
                Context->TreeDepth, Context->saveLayerTreeDepth, Context->varTreeDepth,
                ScissorEnable
                ));
#else
            gcmERR_BREAK(gcoVG_SetTarget(
                Context->vg,
                Target->surface,
                Target->surOrientation
                ));

            gcmERR_BREAK(gcoVG_EnableMask(
                Context->vg,
                gcvFALSE
                ));

            gcmERR_BREAK(gcoVG_SetImageMode(
                Context->vg,
                gcvVG_IMAGE_NONE
                ));

            gcmERR_BREAK(gcoVG_SetBlendMode(
                Context->vg,
                gcvVG_BLEND_SRC
                ));

            gcmERR_BREAK(gcoVG_EnableScissor(
                Context->vg,
                ScissorEnable
                ));
#endif

            if (ScissorEnable && Context->scissoringRectsDirty)
            {
#if gcdGC355_PROFILER
                gcmERR_BREAK(gcoVG_SetScissor(
                    Context->vg,
                    Context->TreeDepth,
                    Context->saveLayerTreeDepth,
                    Context->varTreeDepth,
                    Context->scissoringRectsCount,
                    Context->scissoringRects
                    ));
#else
                gcmERR_BREAK(gcoVG_SetScissor(
                    Context->vg,
                    Context->scissoringRectsCount,
                    Context->scissoringRects
                    ));
#endif

                Context->scissoringRectsDirty = VG_FALSE;
            }

#if gcdGC355_PROFILER
            gcmERR_BREAK(gcoVG_EnableColorTransform(
                Context->vg,
                Context->TreeDepth,
                Context->saveLayerTreeDepth,
                Context->varTreeDepth,
                gcvFALSE
                ));

            gcmERR_BREAK(gcoVG_SetSolidPaint(
                Context->vg,
                Context->TreeDepth,
                Context->saveLayerTreeDepth,
                Context->varTreeDepth,
                ByteColor[0],
                ByteColor[1],
                ByteColor[2],
                ByteColor[3]
                ));

            gcmERR_BREAK(gcoVG_Clear(
                Context->vg,
                Context->TreeDepth, Context->saveLayerTreeDepth, Context->varTreeDepth,
                Target->origin.x + TargetX,
                Target->origin.y + TargetY,
                Width, Height
                ));
#else
            gcmERR_BREAK(gcoVG_EnableColorTransform(
                Context->vg,
                gcvFALSE
                ));

            gcmERR_BREAK(gcoVG_SetSolidPaint(
                Context->vg,
                ByteColor[0],
                ByteColor[1],
                ByteColor[2],
                ByteColor[3]
                ));

            gcmERR_BREAK(gcoVG_Clear(
                Context->vg,
                Target->origin.x + TargetX,
                Target->origin.y + TargetY,
                Width, Height
                ));
#endif

            /* Arm the flush states. */
            *Target->imageDirtyPtr  = vgvIMAGE_NOT_READY;
            Context->imageDirty = vgvIMAGE_NOT_READY;
        }
        else
        {
            /* Fill the image. */
            gcmERR_BREAK(vgfCPUFill(
                Context,
                Target,
                TargetX, TargetY,
                Width, Height,
                FloatColor[0],
                FloatColor[1],
                FloatColor[2],
                FloatColor[3]
                ));
        }
    }
    while (gcvFALSE);
    vgmLEAVESUBAPI(vgfFillColor);
    /* Return status. */
    return status;
}


/******************************************************************************\
************************** OpenVG Image Management API *************************
\******************************************************************************/

/*******************************************************************************
**
** vgCreateImage
**
** vgCreateImage creates an image with the given width, height, and pixel format
** and returns a VGImage handle to it. If an error occurs, VG_INVALID_HANDLE is
** returned. All color and alpha channel values are initially set to zero. The
** format parameter must contain a value from the VGImageFormat enumeration.
**
** The AllowedQuality parameter is a bitwise OR of values from the
** VGImageQuality enumeration, indicating which levels of resampling
** quality may be used to draw the image. It is always possible to draw
** an image using the VG_IMAGE_QUALITY_NONANTIALIASED quality setting even
** if it is not explicitly specified.
**
** INPUT:
**
**    Format
**       Pixel format of the image to be created.
**
**    Width, Height
**       Size of the image to be created.
**
**    AllowedQuality
**       Bitwise OR of VGImageQuality.
**
** OUTPUT:
**
**    VGImage
**       Handle to the new VGImage object.
*/

VG_API_CALL VGImage VG_API_ENTRY vgCreateImage(
    VGImageFormat Format,
    VGint Width,
    VGint Height,
    VGbitfield AllowedQuality
    )
{
    gceSTATUS status;
    vgsIMAGE_PTR image = gcvNULL;

    vgmENTERAPI(vgCreateImage)
    {
        vgsFORMAT_PTR surfaceFormat;
        gctINT pixelCount;
        gctINT byteCount;

        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s(0x%04X, %d, %d, 0x%08X);\n",
            __FUNCTION__,
            Format, Width, Height, AllowedQuality
            );

        /* Get the format descriptor. */
        surfaceFormat = vgfGetFormatInfo(Context, Format);

        /* Validate the format. */
        if (((surfaceFormat == gcvNULL) || !surfaceFormat->nativeFormat)
#if gcdVG_ONLY
            &&
            (Format != VG_EXT_INDEX1) &&    /* Index Color states. */
            (Format != VG_EXT_INDEX2) &&
            (Format != VG_EXT_INDEX4) &&
            (Format != VG_EXT_INDEX8)
#endif
           )
        {
            vgmERROR(VG_UNSUPPORTED_IMAGE_FORMAT_ERROR);
            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }

        /* Validate the image size. */
        if ((Width  <= 0) || (Width  > Context->maxImageWidth) ||
            (Height <= 0) || (Height > Context->maxImageHeight))
        {
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        /* Determine the size of the image in pixels and bytes. */
        pixelCount = Width * Height;
#if gcdVG_ONLY
        if ((Format == VG_EXT_INDEX1) ||    /* Index Color states. */
            (Format == VG_EXT_INDEX2) ||
            (Format == VG_EXT_INDEX4) ||
            (Format == VG_EXT_INDEX8)
            )
        {
            byteCount = 1;
        }
        else
#endif
        {
            byteCount  = pixelCount * surfaceFormat->bitsPerPixel / 8;
        }

        /* Make sure we are within range. */
        if ((pixelCount > Context->maxImagePixels) ||
            (byteCount  > Context->maxImageBytes))
        {
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        /* Validate image quality. */
        if ((AllowedQuality == 0) ||
            ((AllowedQuality & vgvIMAGE_QUALITY_MASK) != 0))
        {
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

#if gcdGC355_MEM_PRINT
        gcoOS_RecordAllocation();
#endif

#if gcdVG_ONLY
        /* Index Color states. */
        if ((Format == VG_EXT_INDEX1) ||
            (Format == VG_EXT_INDEX2) ||
            (Format == VG_EXT_INDEX4) ||
            (Format == VG_EXT_INDEX8))
        {
            /* Go a different way for Index format since it's totally different from regular RGBA formats. */
            gcmERR_BREAK(_CreateIndexedImage(
                Context,
                Format,
                Width, Height,
                AllowedQuality,
                &image
                ));
        }
        else
#endif
        {        /* Create the image. */
            gcmERR_BREAK(vgfCreateImage(
                Context,
                Format,
                Width, Height,
                AllowedQuality,
                &image,
                gcvNULL
                ));

            /* Clear the image with the default value. */
            gcmERR_BREAK(vgfFillColor(
                Context,
                image,
                0, 0, Width, Height,
                vgvFLOATCOLOR0000,
                vgvBYTECOLOR0000,
                gcvFALSE
                ));
        }

#if gcdGC355_MEM_PRINT
        Context->curMemImage += gcoOS_EndRecordAllocation();
        if (Context->maxMemImage < Context->curMemImage)
        {
            Context->maxMemImage = Context->curMemImage;
        }
#endif

        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s() = 0x%08X (surface 0x%08X);\n",
            __FUNCTION__,
            image, image->surface
            );
    }
    vgmLEAVEAPI(vgCreateImage);


    return (VGImage) image;
}


/*******************************************************************************
**
** vgDestroyImage
**
** The resources associated with an image may be deallocated by calling
** vgDestroyImage. Following the call, the image handle is no longer valid in
** any context that shared it. If the image is currently in use as a rendering
** target, is the ancestor of another image (see vgChildImage), is set as a
** paint pattern image on a VGPaint object, or is set as a glyph an a VGFont
** object, its definition remains available to those consumers as long as they
** remain valid, but the handle may no longer be used. When those uses cease,
** the images resources will automatically be deallocated.
**
** INPUT:
**
**    Image
**       Handle to a valid VGImage object.
**
** OUTPUT:
**
**    Nothing.
*/

VG_API_CALL void VG_API_ENTRY vgDestroyImage(
    VGImage Image
    )
{
    vgmENTERAPI(vgDestroyImage)
    {
        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s(0x%08X);\n",
            __FUNCTION__,
            Image
            );

        if (!vgfVerifyUserObject(Context, Image, vgvOBJECTTYPE_IMAGE))
        {
            vgmERROR(VG_BAD_HANDLE_ERROR);
            break;
        }

        /* Invalidate the object. */
        ((vgsOBJECT_PTR) Image)->userValid = VG_FALSE;

#if gcdGC355_MEM_PRINT
        gcoOS_RecordAllocation();
#endif

        /* Decrement the reference count. */
        gcmVERIFY_OK(vgfDereferenceObject(
            Context, (vgsOBJECT_PTR *) &Image
            ));

#if gcdGC355_MEM_PRINT
        Context->curMemImage += gcoOS_EndRecordAllocation();
        if (Context->maxMemImage < Context->curMemImage)
        {
            Context->maxMemImage = Context->curMemImage;
        }
#endif
    }
    vgmLEAVEAPI(vgDestroyImage);
}

VG_API_CALL void VG_API_ENTRY vgImageGetDirectVIV(
                                VGImage     Image,
                                void      **Memory,
                                VGuint     *Height,
                                VGint      *Stride)
{
    vgmENTERAPI(vgImageGetDirectVIV)
    {
        vgsIMAGE_PTR    image = (vgsIMAGE_PTR)Image;

        if (!vgfVerifyUserObject(Context, Image, vgvOBJECTTYPE_IMAGE))
        {
            vgmERROR(VG_BAD_HANDLE_ERROR);
            break;
        }

        if (Memory != gcvNULL)
        {
            *Memory = image->buffer;
        }

        if (image->surface != gcvNULL)
        {
            gcoSURF_GetAlignedSize(image->surface, gcvNULL, Height, Stride);
        }
        else
        {
            if (Height)
                *Height = 0;
            if (Stride)
                *Stride = -1;
        }
    }
    vgmLEAVEAPI(vgImageGetDirectVIV)
}

VG_API_CALL void VG_API_ENTRY vgImageFlushDirectVIV(
    VGImage Image
    )
{
    vgmENTERAPI(vgImageFlushDirectVIV)
    {
        if (!vgfVerifyUserObject(Context, Image, vgvOBJECTTYPE_IMAGE))
        {
            vgmERROR(VG_BAD_HANDLE_ERROR);
            break;
        }

        vgfFlushImage(Context, (vgsIMAGE_PTR)Image, gcvTRUE);
    }
    vgmLEAVEAPI(vgImageFlushDirectVIV)
}


/*******************************************************************************
**
** vgClearImage
**
** The vgClearImage function fills a given rectangle of an image with the color
** specified by the VG_CLEAR_COLOR parameter. The rectangle to be cleared is
** given by X, Y, Width, and Height, which must define a positive region. The
** rectangle is clipped to the bounds of the image.
**
** INPUT:
**
**    Image
**       Handle to a valid VGImage object.
**
**    X, Y
**    Width, Height
**       The rectangular area of the image to be cleared.
**
** OUTPUT:
**
**    Nothing.
*/

VG_API_CALL void VG_API_ENTRY vgClearImage(
    VGImage Image,
    VGint X,
    VGint Y,
    VGint Width,
    VGint Height
    )
{
    vgmENTERAPI(vgClearImage)
    {
        vgsIMAGE_PTR image;

        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s(0x%08X, %d, %d, %d, %d);\n",
            __FUNCTION__,
            Image, X, Y, Width, Height
            );

        if (!vgfVerifyUserObject(Context, Image, vgvOBJECTTYPE_IMAGE))
        {
            vgmERROR(VG_BAD_HANDLE_ERROR);
            break;
        }

        /* Validate the size. */
        if ((Width <= 0) || (Height <= 0))
        {
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            break;
        }

        /* Cast the object. */
        image = (vgsIMAGE_PTR) Image;

        /* Cannot be the render target. */
        if (vgfIsImageRenderTarget(Context, image))
        {
            vgmERROR(VG_IMAGE_IN_USE_ERROR);
            break;
        }

        /* Fill the image. */
        vgfFillColor(
            Context,
            image,
            X, Y, Width, Height,
            Context->clearColor,
            Context->halClearColor,
            gcvFALSE
            );
    }
    vgmLEAVEAPI(vgClearImage);
}


/*******************************************************************************
**
** vgImageSubData
**
** The vgImageSubData function reads pixel values from memory, performs format
** conversion if necessary, and stores the resulting pixels into a rectangular
** portion of an image.
**
** Pixel values are read starting at the address given by the pointer data;
** adjacent scanlines are separated by DataStride bytes. Negative or zero
** values of DataStride are allowed. The region to be written is given by X, Y,
** Width, and Height, which must define a positive region. Pixels that fall
** outside the bounds of the image are ignored.
**
** Pixel values in memory are formatted according to the DataFormat parameter,
** which must contain a value from the VGImageFormat enumeration. The data
** pointer must be aligned according to the number of bytes of the pixel format
** specified by DataFormat, unless dataFormat is equal to VG_BW_1, VG_A_1, or
** VG_A_4, in which case 1 byte alignment is sufficient. Each pixel is converted
** into the format of the destination image as it is written.
**
** If DataFormat is not equal to VG_BW_1, VG_A_1, or VG_A_4, the destination
** image pixel (X + i, Y + j) for 0 <= i < Width and 0 <= j < Height is taken
** from the N bytes of memory starting at Data + j * DataStride + i * N, where
** N is the number of bytes per pixel. For multi-byte pixels, the bits are
** arranged in the same order used to store native multi-byte primitive
** datatypes. For example, a 16-bit pixel would be written to memory in the same
** format as when writing through a pointer with a native 16-bit integral
** datatype.
**
** If DataFormat is equal to VG_BW_1 or VG_A_1, pixel (X + i, Y + j) of the
** destination image is taken from the bit at position (i % 8) within the byte
** at Data + j * DataStride + floor(i / 8) where the least significant bit (LSB)
** of a byte is considered to be at position 0 and the most significant bit
** (MSB) is at position 7. Each scanline must be padded to a multiple of 8 bits.
** Note that dataStride is always given in terms of bytes, not bits.
**
** If DataFormat is equal to VG_A_4, pixel (X + i, Y + j) of the destination
** image is taken from the 4 bits from position (4 * (i % 2)) to
** (4 * (i % 2) + 3) within the byte at Data + j * DataStride + floor(i / 2).
** Each scanline must be padded to a multiple of 8 bits.
**
** If DataFormat specifies a premultiplied format (VG_sRGBA_8888_PRE or
** VG_lRGBA_8888_PRE), color channel values of a pixel greater than their
** corresponding alpha value are clamped to the range [0, alpha].
**
** INPUT:
**
**    Image
**       Handle to a valid VGImage object.
**
**    Data
**       Points to the source pixel array.
**
**    DataStride
**       The stride of the source pixel array.
**
**    DataFormat
**       Format of the source pixel array.
**
**    X, Y
**    Width, Height
**       The rectangular area of the image to be set.
**
** OUTPUT:
**
**    Image
**       Image containing the modified pixel area.
*/

VG_API_CALL void VG_API_ENTRY vgImageSubData(
    VGImage Image,
    const void * Data,
    VGint DataStride,
    VGImageFormat DataFormat,
    VGint X,
    VGint Y,
    VGint Width,
    VGint Height
    )
{
    gceSTATUS status;

    vgmENTERAPI(vgImageSubData)
#if gcdGC355_MEM_PRINT
    gcoOS_RecordAllocation();
#endif
    {
        vgsIMAGE_PTR image;
        vgsFORMAT_PTR surfaceFormat;
        gcsSURF_FORMAT_INFO *info;

        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s(0x%08X, Data, %d, 0x%04X, %d, %d, %d, %d);\n",
            __FUNCTION__,
            Image, DataStride, DataFormat, X, Y, Width, Height
            );

        if (!vgfVerifyUserObject(Context, Image, vgvOBJECTTYPE_IMAGE))
        {
            vgmERROR(VG_BAD_HANDLE_ERROR);
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        /* Cast the object. */
        image = (vgsIMAGE_PTR) Image;

        /* Get the format information. */
        surfaceFormat = vgfGetFormatInfo(Context, DataFormat);

#if gcdVG_ONLY
        /* Index Color states: go a special path. */
        if ((image->format == VG_EXT_INDEX1) ||
            (image->format == VG_EXT_INDEX2) ||
            (image->format == VG_EXT_INDEX4) ||
            (image->format == VG_EXT_INDEX8)
           )
        {
            /* Suppose bpp here is 8. */
            gctPOINTER memory[3];
            gctUINT8    *p, *pData;
            gctINT stride, y;
            gcoSURF surface = image->surface;
            gcoSURF_Lock(surface, gcvNULL, memory);
            gcoSURF_GetAlignedSize(surface, gcvNULL, gcvNULL, &stride);
            p = (gctUINT8*)memory[0];
            pData = (gctUINT8*)Data;

            if (DataStride < 0)
                DataStride = -DataStride;

            p += stride * Y + X * stride / Width;
            for (y = 0; y < Height; y++)
            {
                gcoOS_MemCopy(p, pData, DataStride);
                p += stride;
                pData += DataStride;
            }

            gcoSURF_Unlock(surface, memory[0]);
            gcoSURF_CPUCacheOperation(surface, gcvCACHE_FLUSH);
            return;
        }
        else
#endif
        if ((surfaceFormat == gcvNULL) || !surfaceFormat->nativeFormat)
        {
            vgmERROR(VG_UNSUPPORTED_IMAGE_FORMAT_ERROR);
            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }

        /* Initialize the wrapper surface. */
        gcmERR_BREAK(vgfInitializeWrapper(
            Context,
            &Context->wrapperImage,
            gcvFALSE,
            DataFormat,
            surfaceFormat,
            DataStride,
            Width, Height,
            Data, 0
            ));


        Context->wrapperImage.surfArgValid = gcvFALSE;
        gcoSURF_GetFormatInfo(Context->wrapperImage.surface, &info);
        Context->wrapperImage.surfStride = DataStride;
        Context->wrapperImage.size.height= Height;

        /* Cannot be the render target. */
        if (vgfIsImageRenderTarget(Context, image))
        {
            vgmERROR(VG_IMAGE_IN_USE_ERROR);
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        /* Copy pixels. */
        gcmERR_BREAK(vgfCPUBlit(
            Context,
            &Context->wrapperImage,
            image,
            0, 0, X, Y,
            Width, Height, gcvFALSE
            ));

        /* Restore the flag. */
        Context->wrapperImage.surfArgValid = gcvTRUE;
    }
#if gcdGC355_MEM_PRINT
    Context->curMemImage += gcoOS_EndRecordAllocation();
    if (Context->maxMemImage < Context->curMemImage)
    {
        Context->maxMemImage = Context->curMemImage;
    }
#endif
    vgmLEAVEAPI(vgImageSubData);
}


/*******************************************************************************
**
** vgGetImageSubData
**
** The vgGetImageSubData function reads pixel values from a rectangular portion
** of an image, performs format conversion if necessary, and stores the
** resulting pixels into memory.
**
** Pixel values are written starting at the address given by the pointer Data;
** adjacent scanlines are separated by DataStride bytes. Negative or zero values
** of DataStride are allowed. The region to be read is given by X, Y, Width, and
** Height, which must define a positive region. Pixels that fall outside the
** bounds of the image are ignored.
**
** Pixel values in memory are formatted according to the DataFormat parameter,
** which must contain a value from the VGImageFormat enumeration. If DataFormat
** specifies a premultiplied format (VG_sRGBA_8888_PRE or VG_lRGBA_8888_PRE),
** color channel values of a pixel that are greater than their corresponding
** alpha value are clamped to the range [0, alpha]. The data pointer alignment
** and the pixel layout in memory are as described in the vgImageSubData
** section.
**
** INPUT:
**
**    Image
**       Handle to a valid VGImage object.
**
**    Data
**       Points to the resulting pixel array.
**
**    DataStride
**       The stride of the resulting pixel array.
**
**    DataFormat
**       Format of the resulting pixel array.
**
**    X, Y
**    Width, Height
**       The rectangular area of the image to get the pixels from.
**
** OUTPUT:
**
**    Data
**       Pixel array containing the modified pixel area.
*/

VG_API_CALL void VG_API_ENTRY vgGetImageSubData(
    VGImage Image,
    void * Data,
    VGint DataStride,
    VGImageFormat DataFormat,
    VGint X,
    VGint Y,
    VGint Width,
    VGint Height
    )
{
    gceSTATUS status;

    vgmENTERAPI(vgGetImageSubData)
#if gcdGC355_MEM_PRINT
    gcoOS_RecordAllocation();
#endif
    {
        vgsIMAGE_PTR image;
        vgsFORMAT_PTR surfaceFormat;
        gcsSURF_FORMAT_INFO *info;

        if (!vgfVerifyUserObject(Context, Image, vgvOBJECTTYPE_IMAGE))
        {
            vgmERROR(VG_BAD_HANDLE_ERROR);
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        /* Get the format information. */
        surfaceFormat = vgfGetFormatInfo(Context, DataFormat);

        if ((surfaceFormat == gcvNULL) || !surfaceFormat->nativeFormat)
        {
            vgmERROR(VG_UNSUPPORTED_IMAGE_FORMAT_ERROR);
            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }

        /* Initialize the wrapper surface. */
        gcmERR_BREAK(vgfInitializeWrapper(
            Context,
            &Context->wrapperImage,
            gcvFALSE,
            DataFormat,
            surfaceFormat,
            DataStride,
            Width, Height,
            Data, 0
            ));

        Context->wrapperImage.surfArgValid = gcvFALSE;
        gcoSURF_GetFormatInfo(Context->wrapperImage.surface, &info);
        Context->wrapperImage.surfStride = DataStride;
        Context->wrapperImage.size.height= Height;

        /* Cast the object. */
        image = (vgsIMAGE_PTR) Image;

        /* Cannot be the render target. */
        if (vgfIsImageRenderTarget(Context, image))
        {
            vgmERROR(VG_IMAGE_IN_USE_ERROR);
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        /* Copy pixels. */
        gcmERR_BREAK(vgfCPUBlit(
            Context,
            image,
            &Context->wrapperImage,
            X, Y, 0, 0,
            Width, Height, gcvFALSE
            ));

        /* Restore the flag. */
        Context->wrapperImage.surfArgValid = gcvTRUE;
    }
#if gcdGC355_MEM_PRINT
    Context->curMemImage += gcoOS_EndRecordAllocation();
    if (Context->maxMemImage < Context->curMemImage)
    {
        Context->maxMemImage = Context->curMemImage;
    }
#endif
    vgmLEAVEAPI(vgGetImageSubData);
}


/*******************************************************************************
**
** vgChildImage
**
** The vgChildImage function returns a new VGImage handle that refers to
** a portion of the parent image. The region is given by the intersection
** of the bounds of the parent image with the rectangle beginning at pixel
** (X, Y) with dimensions width and height, which must define a positive
** region contained entirely within parent.
**
** INPUT:
**
**    Parent
**       Handle to a valid VGImage object.
**
**    X, Y
**    Width, Height
**       The rectangular area of the child image.
**
** OUTPUT:
**
**    VGImage
**       Handle to the new VGImage child object.
*/

VG_API_CALL VGImage VG_API_ENTRY vgChildImage(
    VGImage Parent,
    VGint X,
    VGint Y,
    VGint Width,
    VGint Height
    )
{
    vgsIMAGE_PTR child = gcvNULL;

    vgmENTERAPI(vgChildImage)
    {
        vgsIMAGE_PTR parent;

        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s(0x%08X, %d, %d, %d, %d);\n",
            __FUNCTION__,
            Parent, X, Y, Width, Height
            );

        if (!vgfVerifyUserObject(Context, Parent, vgvOBJECTTYPE_IMAGE))
        {
            vgmERROR(VG_BAD_HANDLE_ERROR);
            break;
        }

        /* Cast the object. */
        parent = (vgsIMAGE_PTR) Parent;

        /* Validate the origin. */
        if ((X < 0) || (X >= parent->size.width) ||
            (Y < 0) || (Y >= parent->size.height))
        {
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            break;
        }

        /* Validate the size. */
        if ((Width  <= 0) || (X > parent->size.width  - Width) ||
            (Height <= 0) || (Y > parent->size.height - Height))
        {
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            break;
        }

        /* Cannot be the render target. */
        if (vgfIsImageRenderTarget(Context, parent))
        {
            vgmERROR(VG_IMAGE_IN_USE_ERROR);
            break;
        }

        /* Allocate a new object. */
        if (gcmIS_ERROR(vgfReferenceImage(Context, &child)))
        {
            vgmERROR(VG_OUT_OF_MEMORY_ERROR);
            break;
        }

        /* Increment parent's reference count. */
        gcmVERIFY_OK(vgfReferenceImage(Context, &parent));

        /* Create new relationship. */
        parent->childrenCount++;
        child->parent = parent;

        /* Initialize the child. */
        child->format           = parent->format;
        child->origin.x         = parent->origin.x + X;
        child->origin.y         = parent->origin.y + Y;
        child->size.width       = Width;
        child->size.height      = Height;
        child->stride           = parent->stride;
        child->allowedQuality   = parent->allowedQuality;
        child->upsample         = parent->upsample;
        child->wrapperFormat    = parent->wrapperFormat;
        child->surfaceFormat    = parent->surfaceFormat;
        child->surface          = parent->surface;
        child->buffer           = parent->buffer;
        child->imageAllocated   = gcvFALSE;
        child->surfaceAllocated = gcvFALSE;
        child->surfaceLocked    = gcvFALSE;
        child->imageDirty       = parent->imageDirty;
        child->imageDirtyPtr    = parent->imageDirtyPtr;
        child->orientation      = parent->orientation;
        child->surOrientation  = parent->surOrientation;
        child->surfArgValid     = parent->surfArgValid;

        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s() = 0x%08X;\n",
            __FUNCTION__,
            child
            );
    }
    vgmLEAVEAPI(vgChildImage);

    /* Return the handle. */
    return (VGImage) child;
}


/*******************************************************************************
**
** vgGetParent
**
** The vgGetParent function returns the closest valid ancestor (i.e., one that
** has not been the target of a vgDestroyImage call) of the given image. If
** image has no ancestors, image is returned.
**
** INPUT:
**
**    Image
**       Handle to a valid VGImage object.
**
** OUTPUT:
**
**    VGImage
**       Handle to the parent VGImage object.
*/

VG_API_CALL VGImage VG_API_ENTRY vgGetParent(
    VGImage Image
    )
{
    vgsIMAGE_PTR parent = gcvNULL;

    vgmENTERAPI(vgGetParent)
    {
        vgsIMAGE_PTR child;
        vgsIMAGE_PTR grandParent;

        if (!vgfVerifyUserObject(Context, Image, vgvOBJECTTYPE_IMAGE))
        {
            vgmERROR(VG_BAD_HANDLE_ERROR);
            break;
        }

        /* Cast the object. */
        child = (vgsIMAGE_PTR) Image;

        /* Cannot be the render target. */
        if (vgfIsImageRenderTarget(Context, child))
        {
            vgmERROR(VG_IMAGE_IN_USE_ERROR);
            break;
        }

        /* Set the initial parent. */
        parent = child;

        /* Scan through all parents. */
        while (gcvTRUE)
        {
            /* Get the grand parent. */
            grandParent = parent->parent;

            /* Found a valid parent? */
            if (grandParent->object.userValid)
            {
                parent = grandParent;
                break;
            }

            /* The root image? */
            if (grandParent == parent)
            {
                /* No valid ancestors. */
                parent = child;
                break;
            }

            /* Set new parent. */
            parent = grandParent;
        }
    }
    vgmLEAVEAPI(vgGetParent);

    /* Return result. */
    return (VGImage) parent;
}


/*******************************************************************************
**
** vgCopyImage
**
** Pixels may be copied between images using the vgCopyImage function. The
** source image pixel (SourceX + i, SourceY + j) is copied to the destination
** image pixel (TargetX + i, TargetY + j), for 0 <= i < Width and
** 0 <= j < Height. Pixels whose source or destination lie outside of the
** bounds of the respective image are ignored. Pixel format conversion is
** applied as needed.
**
** If the dither flag is equal to VG_TRUE, an implementation-dependent
** dithering algorithm may be applied. This may be useful when copying
** into a destination image with a smaller color bit depth than that of
** the source image. Implementations should choose an algorithm that will
** provide good results when the output images are displayed as successive
** frames in an animation.
**
** If SourceImage and DestinationImage are the same image, or are related,
** the copy will occur in a consistent fashion as though the source pixels
** were first copied into a temporary buffer and then copied from the
** temporary buffer to the destination.
**
** INPUT:
**
**    DestinationImage
**       Handle to a valid VGImage destination object.
**
**    TargetX, TargetY
**       Origin within the destination image.
**
**    SourceImage
**       Handle to a valid VGImage source object.
**
**    SourceX, SourceY
**       Origin within the source image.
**
**    Width, Height
**       The size of the subimage to be copied.
**
**    Dither
**       Dither enable flag.
**
** OUTPUT:
**
**    DestinationImage
**       Image containing the modified pixel area.
*/

VG_API_CALL void VG_API_ENTRY vgCopyImage(
    VGImage DestinationImage,
    VGint TargetX,
    VGint TargetY,
    VGImage SourceImage,
    VGint SourceX,
    VGint SourceY,
    VGint Width,
    VGint Height,
    VGboolean Dither
    )
{
    vgmENTERAPI(vgCopyImage)
#if gcdGC355_MEM_PRINT
    gcoOS_RecordAllocation();
#endif
    {
        vgsIMAGE_PTR source;
        vgsIMAGE_PTR target;

        vgmADVANCE_COPYIMAGE_COUNT(Context);

        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s(0x%08X, %d, %d, 0x%08X, %d, %d, %d, %d, %d);\n",
            __FUNCTION__,
            DestinationImage, TargetX, TargetY,
            SourceImage, SourceX, SourceY, Width, Height,
            Dither
            );

        if (!vgfVerifyUserObject(Context, SourceImage, vgvOBJECTTYPE_IMAGE))
        {
            vgmERROR(VG_BAD_HANDLE_ERROR);
            break;
        }

        if (!vgfVerifyUserObject(Context, DestinationImage, vgvOBJECTTYPE_IMAGE))
        {
            vgmERROR(VG_BAD_HANDLE_ERROR);
            break;
        }

        /* Validate the size. */
        if ((Width <= 0) || (Height <= 0))
        {
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            break;
        }

        /* Cast the objects. */
        source = (vgsIMAGE_PTR) SourceImage;
        target = (vgsIMAGE_PTR) DestinationImage;

        /* Cannot be the render targets. */
        if (vgfIsImageRenderTarget(Context, source) ||
            vgfIsImageRenderTarget(Context, target))
        {
            vgmERROR(VG_IMAGE_IN_USE_ERROR);
            break;
        }

        /* Copy the image. */
        vgfDrawImage(
            Context,
            source, target,
            SourceX, SourceY,
            TargetX, TargetY,
            Width, Height,
            gcvVG_BLEND_SRC,
            Context->colTransform,
            gcvFALSE,
            gcvFALSE,
            Dither,
            gcvFALSE,
            gcvFALSE
            );
    }
#if gcdGC355_MEM_PRINT
    Context->curMemImage += gcoOS_EndRecordAllocation();
    if (Context->maxMemImage < Context->curMemImage)
    {
        Context->maxMemImage = Context->curMemImage;
    }
#endif
    vgmLEAVEAPI(vgCopyImage);
}


/*******************************************************************************
**
** vgDrawImage
**
** An image may be drawn to the current drawing surface using the vgDrawImage
** function. The current image-user-to-surface transformation Ti is applied to
** the image, so that the image pixel centered at (px + 1/2, py + 1/2) is mapped
** to the point (Ti)(px + 1/2, py + 1/2). In practice, backwards mapping may be
** used. That is, a sample located at (x, y) in the surface coordinate system is
** colored according to an interpolated image pixel value at the point
** (Ti)^-1 (x, y) in the image coordinate system. If Ti is non-invertible (or
** nearly so, within the limits of numerical accuracy), no drawing occurs.
**
** Interpolation is done in the color space of the image. Image color values are
** processed in premultiplied alpha format during interpolation. Color channel
** values are clamped to the range [0, alpha] before interpolation.
**
** When a projective transformation is used (i.e., the bottom row of the
** image-user-to-surface transformation contains values [ w0 w1 w2 ] different
** from [ 0 0 1 ]), each corner point (x, y) of the image must result in a
** positive value of d = (x * w0 + y * w1 + w2), or else nothing is drawn.
** This rule prevents degeneracies due to transformed image points passing
** through infinity, which occurs when d passes through 0. By requiring d to be
** positive at the corners, it is guaranteed to be positive at all interior
** points as well.
**
** When a projective transformation is used, the value of the VG_IMAGE_MODE
** parameter is ignored and the behavior of VG_DRAW_IMAGE_NORMAL is substituted.
** This avoids the need to generate paint pixels in perspective.
**
** The set of pixels affected consists of the quadrilateral with vertices
** (Ti)(0, 0), (Ti)(w, 0), (Ti)(w, h), and (Ti)(0, h) (where w and h are
** respectively the width and height of the image), plus a boundary of up to
** 1.5 pixels for filtering purposes.
**
** Clipping, masking, and scissoring are applied in the same manner as with
** vgDrawPath. To limit drawing to a subregion of the image, create a child
** image using vgChildImage.
**
** The image quality will be the maximum quality allowed by the image
** (as determined by the AllowedQuality parameter to vgCreateImage) that is not
** higher than the current setting of VG_IMAGE_QUALITY.
**
** INPUT:
**
**    Image
**       Handle to a valid VGImage object.
**
** OUTPUT:
**
**    Nothing.
*/

VG_API_CALL void VG_API_ENTRY vgDrawImage(
    VGImage Image
    )
{
    vgmENTERAPI(vgDrawImage)
    {
        vgsIMAGE_PTR image;

        vgmADVANCE_DRAWIMAGE_COUNT(Context);

        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s(0x%08X);\n",
            __FUNCTION__,
            Image
            );

        if (!vgfVerifyUserObject(Context, Image, vgvOBJECTTYPE_IMAGE))
        {
            vgmERROR(VG_BAD_HANDLE_ERROR);
            break;
        }

        /* Cast the object. */
        image = (vgsIMAGE_PTR) Image;

        /* Cannot be the render targets. */
        if (vgfIsImageRenderTarget(Context, image))
        {
            vgmERROR(VG_IMAGE_IN_USE_ERROR);
            break;
        }

        /* Set the proper matrices. */
        Context->drawImageSurfaceToPaint
            = &Context->imageFillSurfaceToPaint;

        Context->drawImageSurfaceToImage
            = &Context->imageSurfaceToImage;

        /* Draw the image. */
        vgfTesselateImage(
            Context,
            &Context->targetImage,
            image
#if gcdMOVG
            ,
            Context->tsWidth, Context->tsHeight
#endif
            );
    }
    vgmLEAVEAPI(vgDrawImage);

#if gcdGC355_PROFILER
     if (Context->enableGetAPITimes)
    {
        gctSIZE_T length;
        gctCHAR savedValue[256] = {'\0'};
        gctUINT offset = 0;

        gcoOS_PrintStrSafe(savedValue, gcmSIZEOF(savedValue), &offset, "T4-T3 for %s process Data\n",__FUNCTION__);
        length = gcoOS_StrLen((gctSTRING)savedValue, gcvNULL);
        gcoOS_Write(gcvNULL, Context->apiTimeFile, length, savedValue);
        vgFinish();
    }
#endif
}


/*******************************************************************************
**
** vgSetPixels
**
** The vgSetPixels function copies pixel data from the image SourceImage onto
** the drawing surface. The image pixel (SourceX + i, SourceY + j) is copied
** to the drawing surface pixel (TargetX + i, TargetY + j), for
** 0 <= i < Width and 0 <= j < Height. Pixels whose source lies outside of the
** bounds of SourceImage or whose destination lies outside the bounds of the
** drawing surface are ignored. Pixel format conversion is applied as needed.
** Scissoring takes place normally. Transformations, masking, and blending are
** not applied.
**
** INPUT:
**
**    TargetX, TargetY
**       Origin within the drawing surface.
**
**    SourceImage
**       Handle to a valid VGImage source object.
**
**    SourceX, SourceY
**       Origin within the source image.
**
**    Width, Height
**       The size of the subimage to be copied.
**
** OUTPUT:
**
**    Nothing.
*/

VG_API_CALL void VG_API_ENTRY vgSetPixels(
    VGint TargetX,
    VGint TargetY,
    VGImage SourceImage,
    VGint SourceX,
    VGint SourceY,
    VGint Width,
    VGint Height
    )
{
    vgmENTERAPI(vgSetPixels)
#if gcdGC355_MEM_PRINT
    gcoOS_RecordAllocation();
#endif
    {
        vgsIMAGE_PTR source;

        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s(%d, %d, 0x%08X, %d, %d, %d, %d);\n",
            __FUNCTION__,
            TargetX, TargetY,
            SourceImage, SourceX, SourceY, Width, Height
            );

        if (!vgfVerifyUserObject(Context, SourceImage, vgvOBJECTTYPE_IMAGE))
        {
            vgmERROR(VG_BAD_HANDLE_ERROR);
            break;
        }

        /* Validate the size. */
        if ((Width <= 0) || (Height <= 0))
        {
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            break;
        }

        /* Cast the object. */
        source = (vgsIMAGE_PTR) SourceImage;

        /* Cannot be the render target. */
        if (vgfIsImageRenderTarget(Context, source))
        {
            vgmERROR(VG_IMAGE_IN_USE_ERROR);
            break;
        }

        /* Set the pixels. */
        vgfDrawImage(
            Context,
            source, &Context->targetImage,
            SourceX, SourceY,
            TargetX, TargetY,
            Width, Height,
            gcvVG_BLEND_SRC,
            gcvFALSE,
            Context->scissoringEnable,
            gcvFALSE,
            gcvFALSE,
            gcvFALSE,
            gcvFALSE
            );
    }
#if gcdGC355_MEM_PRINT
    Context->curMemImage += gcoOS_EndRecordAllocation();
    if (Context->maxMemImage < Context->curMemImage)
    {
        Context->maxMemImage = Context->curMemImage;
    }
#endif
    vgmLEAVEAPI(vgSetPixels);
}


/*******************************************************************************
**
** vgWritePixels
**
** The vgWritePixels function allows pixel data to be copied to the drawing
** surface without the creation of a VGImage object. The pixel values to be
** drawn are taken from the data pointer at the time of the vgWritePixels call,
** so future changes to the data have no effect. The effects of changes to the
** data by another thread at the time of the call to vgWritePixels are
** undefined.
**
** The DataFormat parameter must contain a value from the VGImageFormat
** enumeration. The alignment and layout of pixels is the same as for
** vgImageSubData.
**
** If DataFormat specifies a premultiplied format (VG_sRGBA_8888_PRE or
** VG_lRGBA_8888_PRE), color channel values of a pixel greater than their
** corresponding alpha value are clamped to the range [0, alpha].
**
** Pixels whose destination coordinate lies outside the bounds of the drawing
** surface are ignored. Pixel format conversion is applied as needed. Scissoring
** takes place normally. Transformations, masking, and blending are not applied.
**
** INPUT:
**
**    Data
**       Points to the source pixel array.
**
**    DataStride
**       The stride of the source pixel array.
**
**    DataFormat
**       Format of the source pixel array.
**
**    TargetX, TargetY
**       Origin within the drawing surface.
**
**    Width, Height
**       The size of the subimage to be copied.
**
** OUTPUT:
**
**    Nothing.
*/

VG_API_CALL void VG_API_ENTRY vgWritePixels(
    const void * Data,
    VGint DataStride,
    VGImageFormat DataFormat,
    VGint TargetX,
    VGint TargetY,
    VGint Width,
    VGint Height
    )
{
    gceSTATUS status;
    vgsIMAGE _tempImage;
    vgsIMAGE_PTR tempImage;

    vgmENTERAPI(vgWritePixels)
#if gcdGC355_MEM_PRINT
    gcoOS_RecordAllocation();
#endif
    {
        vgsFORMAT_PTR surfaceFormat;
        gcsSURF_FORMAT_INFO *info;

        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s(Data, %d, 0x%04X, %d, %d, %d, %d);\n",
            __FUNCTION__,
            DataStride, DataFormat, TargetX, TargetY, Width, Height
            );

        /* Get the format information. */
        surfaceFormat = vgfGetFormatInfo(Context, DataFormat);

        if ((surfaceFormat == gcvNULL) || !surfaceFormat->nativeFormat)
        {
            vgmERROR(VG_UNSUPPORTED_IMAGE_FORMAT_ERROR);
            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }

        /* Initialize image object. */
        tempImage = &_tempImage;
        tempImage->buffer           = gcvNULL;
        tempImage->imageAllocated   = gcvFALSE;
        tempImage->surfaceAllocated = gcvFALSE;
        tempImage->surfaceLocked    = gcvFALSE;

        /* Initialize the wrapper surface. */
        gcmERR_BREAK(vgfInitializeWrapper(
            Context,
            &Context->wrapperImage,
            gcvFALSE,
            DataFormat,
            surfaceFormat,
            DataStride,
            Width, Height,
            Data, 0
            ));

        Context->wrapperImage.surfArgValid = gcvFALSE;
        gcoSURF_GetFormatInfo(Context->wrapperImage.surface, &info);
        Context->wrapperImage.surfStride = DataStride;
        Context->wrapperImage.size.height= Height;

        /* Create the image. */
        gcmERR_BREAK(vgfCreateImage(
            Context,
            DataFormat,
            Width, Height,
            vgvIMAGE_QUALITY_ALL,
            &tempImage,
            gcvNULL
            ));

        /* Copy pixels. */
        gcmVERIFY_OK(vgfCPUBlit(
            Context,
            &Context->wrapperImage,
            tempImage,
            0, 0, 0, 0,
            Width, Height, gcvFALSE
            ));

        /* Set the pixels. */
        vgfDrawImage(
            Context,
            tempImage, &Context->targetImage,
            0, 0,
            TargetX, TargetY,
            Width, Height,
            gcvVG_BLEND_SRC,
            gcvFALSE,
            Context->scissoringEnable,
            gcvFALSE,
            gcvFALSE,
            gcvFALSE,
            gcvFALSE
            );

        /* Release the image resources. */
        gcmVERIFY_OK(vgfReleaseImage(Context, tempImage));

        /* Restore the flag. */
        Context->wrapperImage.surfArgValid = gcvTRUE;
    }
#if gcdGC355_MEM_PRINT
    Context->curMemImage += gcoOS_EndRecordAllocation();
    if (Context->maxMemImage < Context->curMemImage)
    {
        Context->maxMemImage = Context->curMemImage;
    }
#endif
    vgmLEAVEAPI(vgWritePixels);
}


/*******************************************************************************
**
** vgGetPixels
**
** The vgGetPixels function retrieves pixel data from the drawing surface into
** the image DestinationImage.
**
** The drawing surface pixel (SourceX + i, SourceY + j) is copied to pixel
** (TargetX + i, TargetY + j) of the image DestinationImage, for
** 0 <= i < Width and 0 <= j < Height. Pixels whose source lies outside of
** the bounds of the drawing surface or whose destination lies outside the
** bounds of DestinationImage are ignored. Pixel format conversion is applied
** as needed. The scissoring region does not affect the reading of pixels.
**
** INPUT:
**
**    DestinationImage
**       Handle to a valid VGImage destination object.
**
**    TargetX, TargetY
**       Origin within the given image.
**
**    SourceX, SourceY
**       Origin within the drawing surface.
**
**    Width, Height
**       The size of the subimage to be copied.
**
** OUTPUT:
**
**    DestinationImage
**       Image containing the modified pixel area.
*/

VG_API_CALL void VG_API_ENTRY vgGetPixels(
    VGImage DestinationImage,
    VGint TargetX,
    VGint TargetY,
    VGint SourceX,
    VGint SourceY,
    VGint Width,
    VGint Height
    )
{
    vgmENTERAPI(vgGetPixels)
#if gcdGC355_MEM_PRINT
        gcoOS_RecordAllocation();
#endif
    {
        vgsIMAGE_PTR target;

        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s(0x%08X, %d, %d, %d, %d, %d, %d);\n",
            __FUNCTION__,
            DestinationImage,
            TargetX, TargetY, SourceX, SourceY, Width, Height
            );

        if (!vgfVerifyUserObject(Context, DestinationImage, vgvOBJECTTYPE_IMAGE))
        {
            vgmERROR(VG_BAD_HANDLE_ERROR);
            break;
        }

        /* Validate the size. */
        if ((Width <= 0) || (Height <= 0))
        {
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            break;
        }

        /* Cast the object. */
        target = (vgsIMAGE_PTR) DestinationImage;

        /* Cannot be the render target. */
        if (vgfIsImageRenderTarget(Context, target))
        {
            vgmERROR(VG_IMAGE_IN_USE_ERROR);
            break;
        }

        /* Copy pixels. */
        gcmVERIFY_OK(vgfCPUBlit(
            Context,
            &Context->targetImage,
            target,
            SourceX,
            SourceY,
            TargetX,
            TargetY,
            Width, Height, gcvFALSE
            ));
    }
#if gcdGC355_MEM_PRINT
        Context->curMemImage += gcoOS_EndRecordAllocation();
        if (Context->maxMemImage < Context->curMemImage)
        {
            Context->maxMemImage = Context->curMemImage;
        }
#endif
    vgmLEAVEAPI(vgGetPixels);
}


/*******************************************************************************
**
** vgReadPixels
**
** The vgReadPixels function allows pixel data to be copied from the drawing
** surface without the creation of a VGImage object.
**
** Pixel values are written starting at the address given by the pointer Data;
** adjacent scanlines are separated by DataStride bytes. Negative or zero values
** of DataStride are allowed. The region to be read is given by X, Y, Width, and
** Height, which must define a positive region.
**
** Pixel values in memory are formatted according to the DataFormat parameter,
** which must contain a value from the VGImageFormat enumeration. The data
** pointer alignment and the pixel layout in memory is as described in the
** vgImageSubData section.
**
** Pixels whose source lies outside of the bounds of the drawing surface are
** ignored. Pixel format conversion is applied as needed. The scissoring region
** does not affect the reading of pixels.
**
** INPUT:
**
**    Data
**       Points to the destination pixel array.
**
**    DataStride
**       The stride of the destination pixel array.
**
**    DataFormat
**       Format of the destination pixel array.
**
**    SourceX, SourceY
**       Origin within the drawing surface.
**
**    Width, Height
**       The size of the subimage to be copied.
**
** OUTPUT:
**
**    Data
**       Pixel array containing the modified pixel area.
*/

VG_API_CALL void VG_API_ENTRY vgReadPixels(
    void * Data,
    VGint DataStride,
    VGImageFormat DataFormat,
    VGint SourceX,
    VGint SourceY,
    VGint Width,
    VGint Height
    )
{
    gceSTATUS status;

    vgmENTERAPI(vgReadPixels)

#if gcdGC355_MEM_PRINT
        gcoOS_RecordAllocation();
#endif
    {
        vgsFORMAT_PTR surfaceFormat;
        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s(Data, %d, 0x%04X, %d, %d, %d, %d);\n",
            __FUNCTION__,
            DataStride, DataFormat, SourceX, SourceY, Width, Height
            );

        /* Get the format information. */
        surfaceFormat = vgfGetFormatInfo(Context, DataFormat);
        if ((surfaceFormat == gcvNULL) || !surfaceFormat->nativeFormat)
        {
            vgmERROR(VG_UNSUPPORTED_IMAGE_FORMAT_ERROR);
            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }

        /* Initialize the wrapper surface. */
        gcmERR_BREAK(vgfInitializeWrapper(
            Context,
            &Context->wrapperImage,
            gcvFALSE,
            DataFormat,
            surfaceFormat,
            DataStride,
            Width, Height,
            Data, 0
            ));
        Context->wrapperImage.surfArgValid = gcvFALSE;
        Context->wrapperImage.surfStride = DataStride;
        Context->wrapperImage.size.height= Height;

        /* Copy pixels. */
        gcmERR_BREAK(vgfCPUBlit(
            Context,
            &Context->targetImage,
            &Context->wrapperImage,
            SourceX,
            SourceY,
            0, 0,
            Width, Height, gcvFALSE
            ));

        /* Restore the flag. */
        Context->wrapperImage.surfArgValid = gcvTRUE;
    }

#if gcdGC355_MEM_PRINT
        Context->curMemImage += gcoOS_EndRecordAllocation();
        if (Context->maxMemImage < Context->curMemImage)
        {
            Context->maxMemImage = Context->curMemImage;
        }
#endif
    vgmLEAVEAPI(vgReadPixels);
}


/*******************************************************************************
**
** vgCopyPixels
**
** The vgCopyPixels function copies pixels from one region of the drawing
** surface to another. Copies between overlapping regions are allowed and
** always produce consistent results identical to copying the entire source
** region to a scratch buffer followed by copying the scratch buffer into
** the destination region.
**
** The drawing surface pixel (SourceX + i, SourceY + j) is copied to pixel
** (TargetX + i, TargetY + j) for 0 <= i < Width and 0 <= j < Height.
** Pixels whose source or destination lies outside of the bounds of the drawing
** surface are ignored. Transformations, masking, and blending are not applied.
** Scissoring is applied to the destination, but does not affect the reading of
** pixels.
**
** INPUT:
**
**    TargetX, TargetY
**       Destination origin within the drawing surface.
**
**    SourceX, SourceY
**       Source origin within the drawing surface.
**
**    Width, Height
**       The size of the subimage to be copied.
**
** OUTPUT:
**
**    Nothing.
*/

VG_API_CALL void VG_API_ENTRY vgCopyPixels(
    VGint TargetX,
    VGint TargetY,
    VGint SourceX,
    VGint SourceY,
    VGint Width,
    VGint Height
    )
{
    vgmENTERAPI(vgCopyPixels)

#if gcdGC355_MEM_PRINT
    gcoOS_RecordAllocation();
#endif
    {
        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s(%d, %d, %d, %d, %d, %d);\n",
            __FUNCTION__,
            TargetX, TargetY, SourceX, SourceY, Width, Height
            );

        /* Validate the size. */
        if ((Width <= 0) || (Height <= 0))
        {
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            break;
        }

        /* Copy the pixels. */
        vgfDrawImage(
            Context,
            &Context->targetImage,
            &Context->targetImage,
            SourceX, SourceY,
            TargetX, TargetY,
            Width, Height,
            gcvVG_BLEND_SRC,
            gcvFALSE,
            Context->scissoringEnable,
            gcvFALSE,
            gcvFALSE,
            gcvFALSE,
            gcvFALSE
            );

    }
#if gcdGC355_MEM_PRINT
    Context->curMemImage += gcoOS_EndRecordAllocation();
    if (Context->maxMemImage < Context->curMemImage)
    {
        Context->maxMemImage = Context->curMemImage;
    }
#endif
    vgmLEAVEAPI(vgCopyPixels);
}

VG_API_CALL void VG_API_ENTRY vgDrawWarpedImageVIV(
    VGImage Image,
    VGuint RectangleCount,
    const VGRectangle* SrcRectangles,
    const VGRectangle* DstRectangles,
    const VGMatrix* Matrices
    )
{
    vgmENTERAPI(vgDrawWarpedImageVIV)
    {
        VGImage PaddedImage;
        vgsIMAGE_PTR image;
        VGuint i;
        gceSTATUS status;
        VGImageQuality imageQuality;
        gceIMAGE_FILTER halImageQuality;

        gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_PARAMETERS,
                      "%s(%p, %u, %p, %p, %p);\n",
                      __FUNCTION__,
                      Image, RectangleCount, SrcRectangles, DstRectangles, Matrices
                      );

        if (RectangleCount == 0)
        {
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            break;
        }

        if (!vgfVerifyUserObject(Context, Image, vgvOBJECTTYPE_IMAGE))
        {
            vgmERROR(VG_BAD_HANDLE_ERROR);
            break;
        }

        PaddedImage = vgCreateImage((VGImageFormat)vgGetParameteri(Image, VG_IMAGE_FORMAT), vgGetParameteri(Image, VG_IMAGE_WIDTH) + 10, vgGetParameteri(Image, VG_IMAGE_HEIGHT) + 10, vgGeti(VG_IMAGE_QUALITY));
        vgCopyImage(PaddedImage, 1, 1, Image, 0, 0, vgGetParameteri(Image, VG_IMAGE_WIDTH), vgGetParameteri(Image, VG_IMAGE_HEIGHT), VG_FALSE);
        image = (vgsIMAGE_PTR) PaddedImage;

        if (vgfIsImageRenderTarget(Context, image))
        {
            vgmERROR(VG_IMAGE_IN_USE_ERROR);
            break;
        }

        Context->drawImageSurfaceToPaint = &Context->imageFillSurfaceToPaint;
        Context->drawImageSurfaceToImage = &Context->imageSurfaceToImage;

        gcmERR_BREAK(vgfFlushImage(Context, image, gcvFALSE));

#if gcdGC355_PROFILER
        gcmERR_BREAK(gcoVG_SetTarget(
            Context->vg,
            Context->TreeDepth,
            Context->saveLayerTreeDepth,
            Context->varTreeDepth,
            Context->targetImage.surface,
            Context->targetImage.surOrientation
            ));

        gcmERR_BREAK(gcoVG_SetFillRule(
            Context->vg,
            Context->TreeDepth,
            Context->saveLayerTreeDepth,
            Context->varTreeDepth,
            gcvVG_NON_ZERO
            ));
#else
        gcmERR_BREAK(gcoVG_SetTarget(
            Context->vg,
            Context->targetImage.surface,
            Context->targetImage.surOrientation
            ));

        gcmERR_BREAK(gcoVG_SetFillRule(
            Context->vg,
            gcvVG_NON_ZERO
            ));
#endif

        gcmERR_BREAK(vgfUpdateStates(
            Context,
            Context->halImageMode,
            Context->halBlendMode,
            Context->colTransform,
            Context->scissoringEnable,
            Context->maskingEnable,
            gcvFALSE
            ));

        gcmERR_BREAK(vgfUpdatePaint(
            Context,
            Context->drawImageSurfaceToPaint,
            Context->fillPaint,
            gcvFALSE
            ));

        imageQuality = Context->imageQuality & image->allowedQuality;

        switch (imageQuality)
        {
        case VG_IMAGE_QUALITY_FASTER:
            halImageQuality = gcvFILTER_LINEAR;
            break;

        case VG_IMAGE_QUALITY_BETTER:
            halImageQuality = gcvFILTER_BI_LINEAR;
            break;

        default:
            halImageQuality = gcvFILTER_POINT;
        }

        for (i = 0; i < RectangleCount; i++)
        {
#if gcdGC355_PROFILER
            gcmERR_BREAK(gcoVG_DrawSurfaceToImage(
                Context->vg,
                Context->TreeDepth,
                Context->saveLayerTreeDepth,
                Context->varTreeDepth,
                image->surface,
                (gcsVG_RECT_PTR) &SrcRectangles[i],
                (gcsVG_RECT_PTR) &DstRectangles[i],
                (gctFLOAT *) Matrices[i].m,
                halImageQuality,
                Context->maskingEnable,
                (i == 0) ? gcvTRUE : gcvFALSE
                ));
#else
            gcmERR_BREAK(gcoVG_DrawSurfaceToImage(
                Context->vg,
                image->surface,
                (gcsVG_RECT_PTR) &SrcRectangles[i],
                (gcsVG_RECT_PTR) &DstRectangles[i],
                (gctFLOAT *) Matrices[i].m,
                halImageQuality,
                Context->maskingEnable,
                (i == 0) ? gcvTRUE : gcvFALSE
                ));
#endif
        }

        *image->imageDirtyPtr = vgvIMAGE_NOT_FINISHED;
        *Context->targetImage.imageDirtyPtr = vgvIMAGE_NOT_READY;
        Context->imageDirty = vgvIMAGE_NOT_READY;
        vgDestroyImage(PaddedImage);
    }
    vgmLEAVEAPI(vgDrawWarpedImageVIV);
}

VG_API_CALL VGImage VG_API_ENTRY vgCreateImageConstVIV(
    VGImageFormat Format,
    VGint Width,
    VGint Height,
    VGbitfield AllowedQuality,
    void *Memory
    )
{
    gceSTATUS status;
    vgsIMAGE_PTR image = gcvNULL;

    vgmENTERAPI(vgCreateImageConstVIV)
    {
        vgsFORMAT_PTR surfaceFormat;
        gctINT pixelCount;
        gctINT byteCount;

        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s(0x%04X, %d, %d, 0x%08X);\n",
            __FUNCTION__,
            Format, Width, Height, AllowedQuality
            );

        /* Get the format descriptor. */
        surfaceFormat = vgfGetFormatInfo(Context, Format);

        /* Validate the format. */
        if ((surfaceFormat == gcvNULL) || !surfaceFormat->nativeFormat)
        {
            vgmERROR(VG_UNSUPPORTED_IMAGE_FORMAT_ERROR);
            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }

        /* Validate the image size. */
        if ((Width  <= 0) || (Width  > Context->maxImageWidth) ||
            (Height <= 0) || (Height > Context->maxImageHeight))
        {
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        /* Determine the size of the image in pixels and bytes. */
        pixelCount = Width * Height;
        byteCount  = pixelCount * surfaceFormat->bitsPerPixel / 8;

        /* Make sure we are within range. */
        if ((pixelCount > Context->maxImagePixels) ||
            (byteCount  > Context->maxImageBytes))
        {
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        /* Validate image quality. */
        if ((AllowedQuality == 0) ||
            ((AllowedQuality & vgvIMAGE_QUALITY_MASK) != 0))
        {
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        /* Create the image. */
        gcmERR_BREAK(vgfCreateImage(
            Context,
            Format,
            Width, Height,
            AllowedQuality,
            &image,
            Memory
            ));

        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s() = 0x%08X (surface 0x%08X);\n",
            __FUNCTION__,
            image, image->surface
            );
    }
    vgmLEAVEAPI(vgCreateImageConstVIV);


    return (VGImage) image;
}

#if gcdVG_ONLY
static gceSTATUS _CreateYUVImage(
    vgsCONTEXT_PTR  Context,
    VGImageFormat   Format,
    VGint           Width,
    VGint           Height,
    vgsIMAGE_PTR   *Image,
    void           **Address
    )
{
    gceSTATUS status;
    gcoSURF surface;
    gctUINT8_PTR buffer;
    vgsIMAGE_PTR image;
    gctBOOL imageAllocated;
    vgmENTERSUBAPI(_CreateYUVImage);

    do
    {
        gceSURF_COLOR_TYPE colorType;
        gceSURF_FORMAT  surfFormat;
        gctINT stride;

        /* Reset surface pointers. */
        surface = gcvNULL;
        buffer  = gcvNULL;

        /* Assume image as locally allocated. */
        imageAllocated = gcvTRUE;

        /* Get the pointer to the image object. */
        image = * Image;

        /* Not yet allocated? */
        if (image == gcvNULL)
        {
            /* Allocate an image object. */
            status = vgfReferenceImage(Context, &image);

            if (gcmIS_ERROR(status))
            {
                vgmERROR(VG_OUT_OF_MEMORY_ERROR);
                break;
            }

            /* Set the image pointer. */
            * Image = image;
        }
        else
        {
            /* Initialize the object. */
            image->object.type           = vgvOBJECTTYPE_IMAGE;
            image->object.prev           = gcvNULL;
            image->object.next           = gcvNULL;
            image->object.referenceCount = 1;
            image->object.userValid      = VG_FALSE;

            /* Mark image as allocated somewhere else. */
            imageAllocated = gcvFALSE;
        }

        /* Get the format information. */
        switch (Format)
        {
        case VG_EXT_YUY2:
            surfFormat = gcvSURF_YUY2;
            break;

        case VG_EXT_NV12:
            surfFormat = gcvSURF_NV12;
            break;

        case VG_EXT_NV16:
            surfFormat = gcvSURF_NV16;
            break;

        case VG_EXT_AYUY2:
            surfFormat = gcvSURF_AYUY2;
            break;

        case VG_EXT_ANV16:
            surfFormat = gcvSURF_ANV16;
            break;

        case VG_EXT_ANV12:
            surfFormat = gcvSURF_ANV12;
            break;

        default:
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            return gcvSTATUS_INVALID_ARGUMENT;
        }

        /* Allocate the surface. */
        status = gcoSURF_Construct(
            Context->hal,
            Width, Height, 1,
            gcvSURF_BITMAP,
            surfFormat,
            gcvPOOL_DEFAULT,
            &surface
            );

        if (gcmIS_ERROR(status))
        {
            vgmERROR(VG_OUT_OF_MEMORY_ERROR);
            break;
        }

        /* Get the logical pointer to the surface. */
        buffer = gcvNULL;
        status = gcoSURF_Lock(
            surface, gcvNULL, (gctPOINTER *) Address
            );

        if (gcmIS_ERROR(status))
        {
            vgmERROR(VG_OUT_OF_MEMORY_ERROR);
            break;
        }

        /* Get the surface stride. */
        status = gcoSURF_GetAlignedSize(
            surface, gcvNULL, gcvNULL, &stride
            );

        if (gcmIS_ERROR(status))
        {
            vgmERROR(VG_OUT_OF_MEMORY_ERROR);
            break;
        }

        /* Set the proper color space. */
        colorType = gcvSURF_COLOR_UNKNOWN;

        status = gcoSURF_SetColorType(surface, colorType);

        if (gcmIS_ERROR(status))
        {
            vgmERROR(VG_OUT_OF_MEMORY_ERROR);
            break;
        }

        /* Set image parameters. */
        image->size.width  = Width;
        image->size.height = Height;
        image->origin.x    = 0;
        image->origin.y    = 0;
        image->stride      = stride;

        image->allowedQuality = VG_IMAGE_QUALITY_NONANTIALIASED;

        image->format         = Format;
        image->upsample       = gcvFALSE;
        image->wrapperFormat  = gcvNULL;
        image->surfaceFormat  = gcvNULL;
        image->orientation    = vgvIMAGE_ORIENTATION;
        image->surOrientation = vgvIMAGE_ORIENTATION;

        image->glyph        = 0;
        image->pattern      = 0;
        image->renderTarget = 0;

        image->parent        = image;
        image->childrenCount = 0;
        image->surface       = surface;
        image->buffer        = buffer;

        image->imageAllocated   = imageAllocated;
        image->surfaceAllocated = gcvTRUE;
        image->surfaceLocked    = gcvTRUE;
        image->imageDirty       = vgvIMAGE_READY;
        image->imageDirtyPtr    = &image->imageDirty;
        image->surfArgValid     = gcvTRUE;
        /* Success. */
        status = gcvSTATUS_OK;
        goto ErrorHandler;
    }
    while (gcvFALSE);

    /* Roll back. */
    if (surface != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(surface));
    }

    if (imageAllocated)
    {
        gcmVERIFY_OK(vgfDereferenceObject(Context, (vgsOBJECT_PTR *) &image));
    }

ErrorHandler:
    break;
    vgmLEAVESUBAPI(_CreateYUVImage);
    /* Return status. */
    return status;
}

VG_API_CALL VGImage VG_API_ENTRY vgCreateImageDirectVIV(
    VGImageFormat Format,
    VGint Width, VGint Height,
    VGint *Count, void **Address)
{
    vgmENTERAPI(vgCreateImageDirectVIV)
    {
        vgsIMAGE_PTR    image = gcvNULL;
        gceSTATUS       status = gcvSTATUS_OK;
        gctINT          pixelCount, byteCount;

        if ((Format != VG_EXT_YUY2) &&
            (Format != VG_EXT_NV12) &&
            (Format != VG_EXT_NV16) &&
            (Format != VG_EXT_AYUY2) &&
            (Format != VG_EXT_ANV12) &&
            (Format != VG_EXT_ANV16)
            )
        {
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            return VG_INVALID_HANDLE;
        }

        if ((Width <= 0) || (Height <= 0))
        {
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            return VG_INVALID_HANDLE;
        }

        pixelCount = Width * Height;
        switch (Format)
        {
        case VG_EXT_YUY2:
        case VG_EXT_NV16:
            byteCount = 16;
            break;

        case VG_EXT_NV12:
            byteCount = 12;
            break;

        case VG_EXT_AYUY2:
        case VG_EXT_ANV16:
            byteCount = 24;
            break;

        case VG_EXT_ANV12:
            byteCount = 20;
            break;

        default:
            byteCount = 0;
            break;
        }

        byteCount = pixelCount * byteCount / 8;
        if ((pixelCount > Context->maxImagePixels) ||
            (byteCount > Context->maxImageBytes))
        {
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            break;
        }

        /* Create the vg image object. */
        gcmERR_BREAK(_CreateYUVImage(
            Context,
            Format,
            Width, Height,
            &image,
            Address
            ));

        if (Count != gcvNULL)
        {
            switch (Format)
            {
            case VG_EXT_YUY2:
                *Count = 1;
                break;
            case VG_EXT_NV12:
            case VG_EXT_NV16:
            case VG_EXT_AYUY2:
                *Count = 2;
                break;

            case VG_EXT_ANV12:
            case VG_EXT_ANV16:
            default:
                *Count = 3;
                break;
            }
        }
        return (VGImage)image;
    }
    vgmLEAVEAPI(vgCreateImageDirectVIV);


    return VG_INVALID_HANDLE;
}

/* After App copies yuv data to the surface memory,
   this API needs to be called to flush the content for GPU to access.
 */
VG_API_CALL void VG_API_ENTRY vgInvalidateImageDirectVIV(VGImage Image)
{
    vgmENTERAPI(vgInvalidateImageDirectVIV)
    {
        vgsIMAGE_PTR image;

        image = (vgsIMAGE_PTR)Image;
        gcoSURF_CPUCacheOperation(image->surface, gcvCACHE_FLUSH);
    }
    vgmLEAVEAPI(vgInvalidateImageDirectVIV);
}

VG_API_CALL void VG_API_ENTRY vgResolveImageVIV(
    VGImage Src,
    VGImage Dst,
    VGint Src_x, VGint Src_y,
    VGint Width, VGint Height,
    VGint Dst_x, VGint Dst_y,
    VGint Src_uv, VGint Src_standard,
    VGint Dst_uv, VGint Dst_standard, VGint Dst_alpha)
{
    vgmENTERAPI(vgResolveImageVIV)
    {
        vgsIMAGE_PTR image_src, image_dst;
        gceSTATUS status = gcvSTATUS_OK;

        image_src = (vgsIMAGE_PTR) Src;
        image_dst = (vgsIMAGE_PTR) Dst;
        gcmERR_BREAK(gcoVG_Resolve(
            Context->vg,
            image_src->surface,
            image_dst->surface,
            Src_x, Src_y,
            Dst_x, Dst_y,
            Width, Height,
            Src_uv, Src_standard,
            Dst_uv, Dst_standard, Dst_alpha
            ));

        *image_src->imageDirtyPtr  = vgvIMAGE_NOT_FINISHED;
        *image_dst->imageDirtyPtr  = vgvIMAGE_NOT_READY;
        Context->imageDirty = vgvIMAGE_NOT_READY;
        vgfFlushImage(Context, image_dst, gcvTRUE);
    }
    vgmLEAVEAPI(vgResolveImageVIV);
}
#endif
