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


#include "gc_vg_precomp.h"
#include <string.h>

/******************************************************************************\
*********************** Support Functions and Definitions **********************
\******************************************************************************/
/* Column-major identity matrix. */
static vgsMATRIX _identityMatrix =
{
    /* Matrix values. */
    {
        /* First column. */
        1.0f,
        0.0f,
        0.0f,

        /* Second column. */
        0.0f,
        1.0f,
        0.0f,

        /* Third column. */
        0.0f,
        0.0f,
        1.0f
    },
    gcvTRUE,            /* Matrix values have changed. */
    gcvFALSE,           /* No force load by default. */

    /* Identity. */
    gcvTRUE,            /* Identity matrix. */
    gcvFALSE,           /* Not dirty. */

    /* Determinant. */
    1.0f,               /* Determinant = 1. */
    gcvFALSE,           /* Not dirty. */
    gcvFALSE
};



/*******************************************************************************
**
** _LoadIdentity
**
** Sets the specified matrix to the identity and invalidate the values.
**
** INPUT:
**
**    Container
**       Pointer to the matrix container.
**
** OUTPUT:
**
**    Nothing.
*/
static vgmINLINE void _LoadIdentity(
    IN vgsCONTEXT_PTR Context,
    IN OUT vgsMATRIXCONTAINER_PTR Container
    )
{
    vgmENTERSUBAPI(_LoadIdentity);
    /* Copy identity matrix. */
    gcoOS_MemCopy(
        vgmGETMATRIX(Container),
        &_identityMatrix,
        gcmSIZEOF(_identityMatrix)
        );

    /* Invalidate the matrix dependents. */
    Container->invalidate(Context);
    vgmLEAVESUBAPI(_LoadIdentity);
}


/*******************************************************************************
**
** _InitializeContainer
**
** Initialize the specified VG matrix container.
**
** INPUT:
**
**    Matrix
**       Pointer to the matrix.
**
** OUTPUT:
**
**    Nothing.
*/

void _InitializeContainer(
    IN vgsCONTEXT_PTR Context,
    IN OUT vgsMATRIXCONTAINER_PTR Container,
    IN vgtMATRIXINVALIDATE InvalidateCallback,
    IN vgtMATRIXUPDATE UpdateCallback,
    IN gctBOOL LoadIdentity
    )
{
    vgmENTERSUBAPI(_InitializeContainer);
    /* Set the callbacks. */
    Container->invalidate = InvalidateCallback;
    Container->update     = UpdateCallback;
    Container->matrix.forceLoad = gcvFALSE;
    /* Load identity to the matrix. */
    if (LoadIdentity)
    {
        _LoadIdentity(Context, Container);
    }
    vgmLEAVESUBAPI(_InitializeContainer);
}

/*******************************************************************************
**
** _SetUserToSurface
**
** Update HAL as necessary with the current user-to-surface transformation.
**
** INPUT:
**
**    Context
**       Pointer to the context.
**
**    UserToSurface
**       Pointer to the user-to-surface matrix.
**
** OUTPUT:
**
**    Nothing.
*/

static gceSTATUS _SetUserToSurface(
    IN vgsCONTEXT_PTR Context,
    IN vgsMATRIX_PTR UserToSurface
    )
{
    gceSTATUS status;
    vgmENTERSUBAPI(_SetUserToSurface);
    do
    {
        /* Dump current user-to-surface. */
        vgmDUMP_MATRIX("USER-TO-SURFACE", UserToSurface);

        /* Did the matrix change? */
        if (UserToSurface->valuesDirty ||
            (Context->currentUserToSurface != UserToSurface))
        {
#if gcdGC355_PROFILER
            /* Set the matrix in HAL. */
            gcmERR_BREAK(gcoVG_SetUserToSurface(
                Context->vg,
                Context->TreeDepth,
                Context->saveLayerTreeDepth,
                Context->varTreeDepth,
                vgmGETMATRIXVALUES(UserToSurface)
                ));
#else
            gcmERR_BREAK(gcoVG_SetUserToSurface(
                Context->vg,
                vgmGETMATRIXVALUES(UserToSurface)
                ));
#endif
            /* Validate the matrix. */
            UserToSurface->valuesDirty = gcvFALSE;
            UserToSurface->forceLoad = gcvFALSE;
            /* Set as current. */
            Context->currentUserToSurface = UserToSurface;
        }
        else if (UserToSurface->forceLoad)
        {
            UserToSurface->forceLoad = gcvFALSE;
             gcmERR_BREAK(gcoVG_SetUserToSurface(
                Context->vg,
                vgmGETMATRIXVALUES(UserToSurface)
                ));
        }
        else
        {
            /* Success. */
            status = gcvSTATUS_OK;
        }
    }
    while (gcvFALSE);
    vgmLEAVESUBAPI(_SetUserToSurface);
    /* Return status. */
    return status;
}


/*******************************************************************************
**
** _UpdateSurfaceToPaintMatrix
**
** Updates the current specified surface-to-paint matrix as needed.
**
** INPUT:
**
**    Context
**       Pointer to the context.
**
**    UserToSurface
**       Pointer to the user-to-surface matrix.
**
**    PaintToUser
**       Pointer to the paint-to-user matrix.
**
**    Paint
**       Pointer to the corresponding VG paint object.
**
** OUTPUT:
**
**    SurfaceToPaint
**       Pointer to the surface-to-paint matrix to compute.
*/

static gceSTATUS _UpdateSurfaceToPaintMatrix(
    IN vgsCONTEXT_PTR Context,
    IN vgsMATRIXCONTAINER_PTR UserToSurface,
    IN vgsMATRIXCONTAINER_PTR PaintToUser,
    IN OUT vgsMATRIX_PTR SurfaceToPaint,
    IN vgsPAINT_PTR Paint
    )
{
    gceSTATUS status;
    vgmENTERSUBAPI(_UpdateSurfaceToPaintMatrix);
    do
    {
        gctBOOL doReload;

        /* Assume the matrix has been loaded already. */
        doReload = gcvFALSE;

        /* Update user-to-surface matrix. */
        gcmERR_BREAK(UserToSurface->update(Context));

        /* Does the matrix need to be recomputed? */
        if (SurfaceToPaint->valuesDirty)
        {
            vgsMATRIX invertedSurfaceToPaint;

            /* Compute the inverted surface-to-paint matrix. */
            vgfMultiplyMatrix3x3(
                Context,
                vgmGETMATRIX(UserToSurface),
                vgmGETMATRIX(PaintToUser),
                &invertedSurfaceToPaint
                );

            /* Compute the surface to paint matrix. */
            doReload = SurfaceToPaint->valid = vgfInvertMatrix(
                Context,
                &invertedSurfaceToPaint,
                SurfaceToPaint
                );

            /* Notify the paint that the matrix has cahnged. */
            if (doReload)
            {
                vgfPaintMatrixChanged(Context, Paint, SurfaceToPaint);
            }
            else
            {
                gcmERR_BREAK(gcvSTATUS_INVALID_ARGUMENT);
            }

            /* Validate. */
            SurfaceToPaint->valuesDirty = gcvFALSE;
        }

        /* Ensure that the matrix is current for the paint. */
        vgfPaintMatrixVerifyCurrent(Context, Paint, SurfaceToPaint);
    }
    while (gcvFALSE);
    vgmLEAVESUBAPI(_UpdateSurfaceToPaintMatrix);
    /* Return status. */
    return status;
}


/*******************************************************************************
**
** _UpdateSurfaceToImageMatrix
**
** Updates the current specified surface-to-image matrix as needed.
**
** INPUT:
**
**    Context
**       Pointer to the context.
**
**    UserToSurface
**       Pointer to the user-to-surface matrix.
**
** OUTPUT:
**
**    SurfaceToImage
**       Pointer to the surface-to-image matrix to compute.
*/

static gceSTATUS _UpdateSurfaceToImageMatrix(
    IN vgsCONTEXT_PTR Context,
    IN vgsMATRIXCONTAINER_PTR UserToSurface,
    IN OUT vgsMATRIX_PTR SurfaceToImage
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    vgmENTERSUBAPI(_UpdateSurfaceToImageMatrix);
    do
    {
        /* Update user-to-surface matrix. */
        gcmERR_BREAK(UserToSurface->update(Context));

        /* Does the matrix need to be recomputed? */
        if (SurfaceToImage->valuesDirty)
        {
            /* Compute the surface to paint matrix. */
            SurfaceToImage->valid = vgfInvertMatrix(
                Context,
                vgmGETMATRIX(UserToSurface),
                SurfaceToImage
                );

            /* Set the matrix in HAL. */
            if (SurfaceToImage->valid)
            {
#if gcdGC355_PROFILER
                status = gcoVG_SetSurfaceToImage(
                    Context->vg,
                    Context->TreeDepth,
                    Context->saveLayerTreeDepth,
                    Context->varTreeDepth,
                    vgmGETMATRIXVALUES(SurfaceToImage)
                    );
#else
                status = gcoVG_SetSurfaceToImage(
                    Context->vg,
                    vgmGETMATRIXVALUES(SurfaceToImage)
                    );
#endif
            }

            /* Validate. */
            SurfaceToImage->valuesDirty = gcvFALSE;
            SurfaceToImage->forceLoad = gcvFALSE;
        }
        else if (SurfaceToImage->forceLoad)
        {
                status = gcoVG_SetSurfaceToImage(
                    Context->vg,
                    vgmGETMATRIXVALUES(SurfaceToImage)
                    );
                SurfaceToImage->forceLoad = gcvFALSE;
        }
    }
    while (gcvFALSE);
    vgmLEAVESUBAPI(_UpdateSurfaceToImageMatrix);
    /* Return status. */
    return status;
}


/*******************************************************************************
**
** MATRIX CALLBACKS: image matrices.
**
** INPUT:
**
**    Context
**       Pointer to the context.
**
** OUTPUT:
**
**    Nothing.
*/

/* imageFillSurfaceToPaint */
static void _InvalidateImageFillSurfaceToPaint(
    IN vgsCONTEXT_PTR Context
    )
{
    vgmENTERSUBAPI(_InvalidateImageFillSurfaceToPaint);
    vgmLEAVESUBAPI(_InvalidateImageFillSurfaceToPaint);
    /* Nothing to do here. */
}

static gceSTATUS _UpdateImageFillSurfaceToPaint(
    IN vgsCONTEXT_PTR Context
    )
{
    gceSTATUS status;
    vgmENTERSUBAPI(_UpdateImageFillSurfaceToPaint);
    status = _UpdateSurfaceToPaintMatrix(
                Context,
                &Context->imageUserToSurface,
                &Context->fillPaintToUser,
                vgmGETMATRIX(&Context->imageFillSurfaceToPaint),
                Context->fillPaint
                );
    vgmLEAVESUBAPI(_UpdateImageFillSurfaceToPaint);
    return status;
}

/* glyphSurfaceToImage */
static void _InvalidateGlyphSurfaceToImage(
    IN vgsCONTEXT_PTR Context
    )
{
    vgmENTERSUBAPI(_InvalidateGlyphSurfaceToImage);
    vgmLEAVESUBAPI(_InvalidateGlyphSurfaceToImage);
    /* Nothing to do here. */
}

static gceSTATUS _UpdateGlyphSurfaceToImage(
    IN vgsCONTEXT_PTR Context
    )
{
    gceSTATUS status;
    vgmENTERSUBAPI(_UpdateGlyphSurfaceToImage);
    status = _UpdateSurfaceToImageMatrix(
                Context,
                &Context->translatedGlyphUserToSurface,
                vgmGETMATRIX(&Context->glyphSurfaceToImage)
                );
    vgmLEAVESUBAPI(_UpdateGlyphSurfaceToImage);
    return status;
}

/* imageSurfaceToImage */
static void _InvalidateImageSurfaceToImage(
    IN vgsCONTEXT_PTR Context
    )
{
    vgmENTERSUBAPI(_InvalidateImageSurfaceToImage);
    vgmLEAVESUBAPI(_InvalidateImageSurfaceToImage);
    /* Nothing to do here. */
}

static gceSTATUS _UpdateImageSurfaceToImage(
    IN vgsCONTEXT_PTR Context
    )
{
    gceSTATUS status;
    vgmENTERSUBAPI(_UpdateImageSurfaceToImage);
    status = _UpdateSurfaceToImageMatrix(
                Context,
                &Context->imageUserToSurface,
                vgmGETMATRIX(&Context->imageSurfaceToImage)
                );
    vgmLEAVESUBAPI(_UpdateImageSurfaceToImage);
    return status;
}


/*******************************************************************************
**
** MATRIX CALLBACKS: path matrices.
**
** INPUT:
**
**    Context
**       Pointer to the context.
**
** OUTPUT:
**
**    Nothing.
*/

/* pathFillSurfaceToPaint */
static void _InvalidatePathFillSurfaceToPaint(
    IN vgsCONTEXT_PTR Context
    )
{
    vgmENTERSUBAPI(_InvalidatePathFillSurfaceToPaint);
    vgmLEAVESUBAPI(_InvalidatePathFillSurfaceToPaint);
    /* Nothing to do here. */
}

static gceSTATUS _UpdatePathFillSurfaceToPaint(
    IN vgsCONTEXT_PTR Context
    )
{
    gceSTATUS status;
    vgmENTERSUBAPI(_UpdatePathFillSurfaceToPaint);
    status = _UpdateSurfaceToPaintMatrix(
                Context,
                &Context->pathUserToSurface,
                &Context->fillPaintToUser,
                vgmGETMATRIX(&Context->pathFillSurfaceToPaint),
                Context->fillPaint
                );
    vgmLEAVESUBAPI(_UpdatePathFillSurfaceToPaint);
    return status;
}

/* pathStrokeSurfaceToPaint */
static void _InvalidatePathStrokeSurfaceToPaint(
    IN vgsCONTEXT_PTR Context
    )
{
    vgmENTERSUBAPI(_InvalidatePathStrokeSurfaceToPaint);
    vgmLEAVESUBAPI(_InvalidatePathStrokeSurfaceToPaint);
    /* Nothing to do here. */
}

static gceSTATUS _UpdatePathStrokeSurfaceToPaint(
    IN vgsCONTEXT_PTR Context
    )
{
    gceSTATUS status;
    vgmENTERSUBAPI(_UpdatePathStrokeSurfaceToPaint);
    status = _UpdateSurfaceToPaintMatrix(
                Context,
                &Context->pathUserToSurface,
                &Context->strokePaintToUser,
                vgmGETMATRIX(&Context->pathStrokeSurfaceToPaint),
                Context->strokePaint
                );
    vgmLEAVESUBAPI(_UpdatePathStrokeSurfaceToPaint);
    return status;
}

/* glyphFillSurfaceToPaint */
static void _InvalidateGlyphFillSurfaceToPaint(
    IN vgsCONTEXT_PTR Context
    )
{
    vgmENTERSUBAPI(_InvalidateGlyphFillSurfaceToPaint);
    vgmLEAVESUBAPI(_InvalidateGlyphFillSurfaceToPaint);
    /* Nothing to do here. */
}

static gceSTATUS _UpdateGlyphFillSurfaceToPaint(
    IN vgsCONTEXT_PTR Context
    )
{
    gceSTATUS status;
    vgmENTERSUBAPI(_UpdateGlyphFillSurfaceToPaint);
    status = _UpdateSurfaceToPaintMatrix(
                Context,
                &Context->translatedGlyphUserToSurface,
                &Context->fillPaintToUser,
                vgmGETMATRIX(&Context->glyphFillSurfaceToPaint),
                Context->fillPaint
                );
    vgmLEAVESUBAPI(_UpdateGlyphFillSurfaceToPaint);
    return status;
}

/* glyphStrokeSurfaceToPaint */
static void _InvalidateGlyphStrokeSurfaceToPaint(
    IN vgsCONTEXT_PTR Context
    )
{
    vgmENTERSUBAPI(_InvalidateGlyphStrokeSurfaceToPaint);
    vgmLEAVESUBAPI(_InvalidateGlyphStrokeSurfaceToPaint);
    /* Nothing to do here. */
}

static gceSTATUS _UpdateGlyphStrokeSurfaceToPaint(
    IN vgsCONTEXT_PTR Context
    )
{
    gceSTATUS status;
    vgmENTERSUBAPI(_UpdateGlyphStrokeSurfaceToPaint);
    status = _UpdateSurfaceToPaintMatrix(
                Context,
                &Context->translatedGlyphUserToSurface,
                &Context->strokePaintToUser,
                vgmGETMATRIX(&Context->glyphStrokeSurfaceToPaint),
                Context->strokePaint
                );
    vgmLEAVESUBAPI(_UpdateGlyphStrokeSurfaceToPaint);
    return status;
}


/*******************************************************************************
**
** MATRIX CALLBACKS: intermediate matrices.
**
** INPUT:
**
**    Context
**       Pointer to the context.
**
** OUTPUT:
**
**    Nothing.
*/

/* translatedGlyphUserToSurface */
static void _InvalidateTranslatedGlyphUserToSurface(
    IN vgsCONTEXT_PTR Context
    )
{
    vgmENTERSUBAPI(_InvalidateTranslatedGlyphUserToSurface);
    vgfInvalidateContainer(Context, &Context->glyphFillSurfaceToPaint);
    vgfInvalidateContainer(Context, &Context->glyphStrokeSurfaceToPaint);
    vgfInvalidateContainer(Context, &Context->glyphSurfaceToImage);
    vgmLEAVESUBAPI(_InvalidateTranslatedGlyphUserToSurface);
}

static gceSTATUS _UpdateTranslatedGlyphUserToSurface(
    IN vgsCONTEXT_PTR Context
    )
{
    gceSTATUS status;
    vgmENTERSUBAPI(_UpdateTranslatedGlyphUserToSurface);
    {
        vgsMATRIX_PTR translatedGlyphUserToSurface;

        /* Get a shortcut to the matrix. */
        translatedGlyphUserToSurface
            = vgmGETMATRIX(&Context->translatedGlyphUserToSurface);

        /* Did the matrix change? */
        if (translatedGlyphUserToSurface->valuesDirty)
        {
            vgsMATRIX_PTR glyphUserToSurface;
            gctFLOAT m00, m01, m02;
            gctFLOAT m10, m11, m12;
            gctFLOAT translateX;
            gctFLOAT translateY;

            /* Get a shortcut to the glyph-user-to-surface matrix. */
            glyphUserToSurface = vgmGETMATRIX(&Context->glyphUserToSurface);

            /* Get shortcuts to the translation values. */
            translateX = Context->glyphOffsetX;
            translateY = Context->glyphOffsetY;

            /* Read the top left 2x2 matrix values. */
            m00 = vgmMAT(glyphUserToSurface, 0, 0);
            m01 = vgmMAT(glyphUserToSurface, 0, 1);
            m02 = vgmMAT(glyphUserToSurface, 0, 2);
            m10 = vgmMAT(glyphUserToSurface, 1, 0);
            m11 = vgmMAT(glyphUserToSurface, 1, 1);
            m12 = vgmMAT(glyphUserToSurface, 1, 2);

            /* Compute the translated values. */
            m02 += m00 * translateX + m01 * translateY;
            m12 += m10 * translateX + m11 * translateY;

            /* Set the first row. */
            vgmMAT(translatedGlyphUserToSurface, 0, 0) = m00;
            vgmMAT(translatedGlyphUserToSurface, 0, 1) = m01;
            vgmMAT(translatedGlyphUserToSurface, 0, 2) = m02;

            /* Set the second row. */
            vgmMAT(translatedGlyphUserToSurface, 1, 0) = m10;
            vgmMAT(translatedGlyphUserToSurface, 1, 1) = m11;
            vgmMAT(translatedGlyphUserToSurface, 1, 2) = m12;

            /* Set the third row. */
            vgmMAT(translatedGlyphUserToSurface, 2, 0) = 0.0f;
            vgmMAT(translatedGlyphUserToSurface, 2, 1) = 0.0f;
            vgmMAT(translatedGlyphUserToSurface, 2, 2) = 1.0f;
        }

        /* Update HAL. */
        status = _SetUserToSurface(Context, translatedGlyphUserToSurface);
    }
    vgmLEAVESUBAPI(_UpdateTranslatedGlyphUserToSurface);
    return status;
}


/*******************************************************************************
**
** MATRIX CALLBACKS: root VG matrices.
**
** INPUT:
**
**    Context
**       Pointer to the context.
**
** OUTPUT:
**
**    Nothing.
*/

/* pathUserToSurface */
static void _InvalidatePathUserToSurface(
    IN vgsCONTEXT_PTR Context
    )
{
    vgmENTERSUBAPI(_InvalidatePathUserToSurface);
    vgfInvalidateContainer(Context, &Context->pathFillSurfaceToPaint);
    vgfInvalidateContainer(Context, &Context->pathStrokeSurfaceToPaint);
    vgmLEAVESUBAPI(_InvalidatePathUserToSurface);
}

static gceSTATUS _UpdatePathUserToSurface(
    IN vgsCONTEXT_PTR Context
    )
{
    gceSTATUS status;
    vgmENTERSUBAPI(_UpdatePathUserToSurface);
    status = _SetUserToSurface(
                Context, vgmGETMATRIX(&Context->pathUserToSurface)
                );
    vgmLEAVESUBAPI(_UpdatePathUserToSurface);
    return status;
}

/* glyphUserToSurface */
static void _InvalidateGlyphUserToSurface(
    IN vgsCONTEXT_PTR Context
    )
{
    vgmENTERSUBAPI(_InvalidateGlyphUserToSurface);
    vgfInvalidateContainer(Context, &Context->translatedGlyphUserToSurface);
    vgmLEAVESUBAPI(_InvalidateGlyphUserToSurface);
}

static gceSTATUS _UpdateGlyphUserToSurface(
    IN vgsCONTEXT_PTR Context
    )
{
    vgmENTERSUBAPI(_UpdateGlyphUserToSurface);
    vgmLEAVESUBAPI(_UpdateGlyphUserToSurface);
    /* Nothing to do here. */
    return gcvSTATUS_OK;
}

/* imageUserToSurface */
static void _InvalidateImageUserToSurface(
    IN vgsCONTEXT_PTR Context
    )
{
    vgmENTERSUBAPI(_InvalidateImageUserToSurface);
    vgfInvalidateContainer(Context, &Context->imageFillSurfaceToPaint);
    vgfInvalidateContainer(Context, &Context->imageSurfaceToImage);
    vgmLEAVESUBAPI(_InvalidateImageUserToSurface);
}

static gceSTATUS _UpdateImageUserToSurface(
    IN vgsCONTEXT_PTR Context
    )
{
    gceSTATUS status;
    vgmENTERSUBAPI(_UpdateImageUserToSurface);
    status = _SetUserToSurface(
                Context, vgmGETMATRIX(&Context->imageUserToSurface)
                );
    vgmLEAVESUBAPI(_UpdateImageUserToSurface);
    return status;
}

/* fillPaintToUser */
static void _InvalidateFillPaintToUser(
    IN vgsCONTEXT_PTR Context
    )
{
    vgmENTERSUBAPI(_InvalidateFillPaintToUser);
    vgfInvalidateContainer(Context, &Context->pathFillSurfaceToPaint);
    vgfInvalidateContainer(Context, &Context->glyphFillSurfaceToPaint);
    vgfInvalidateContainer(Context, &Context->imageFillSurfaceToPaint);
    vgmLEAVESUBAPI(_InvalidateFillPaintToUser);
}

static gceSTATUS _UpdateFillPaintToUser(
    IN vgsCONTEXT_PTR Context
    )
{
    vgmENTERSUBAPI(_UpdateFillPaintToUser);
    vgmLEAVESUBAPI(_UpdateFillPaintToUser);
    /* Nothing to do here. */
    return gcvSTATUS_OK;
}

/* strokePaintToUser */
static void _InvalidateStrokePaintToUser(
    IN vgsCONTEXT_PTR Context
    )
{
    vgmENTERSUBAPI(_InvalidateStrokePaintToUser);
    vgfInvalidateContainer(Context, &Context->pathStrokeSurfaceToPaint);
    vgfInvalidateContainer(Context, &Context->glyphStrokeSurfaceToPaint);
    vgmLEAVESUBAPI(_InvalidateStrokePaintToUser);
}

static gceSTATUS _UpdateStrokePaintToUser(
    IN vgsCONTEXT_PTR Context
    )
{
    vgmENTERSUBAPI(_UpdateStrokePaintToUser);
    vgmLEAVESUBAPI(_UpdateStrokePaintToUser);
    /* Nothing to do here. */
    return gcvSTATUS_OK;
}


/******************************************************************************\
************************** Internal matrix functions. **************************
\******************************************************************************/

/*******************************************************************************
**
** vgfInitializeMatrixSet
**
** Initialize the VG matrix set.
**
** INPUT:
**
**    Context
**       Pointer to the context.
**
** OUTPUT:
**
**    Nothing.
*/

void vgfInitializeMatrixSet(
    IN vgsCONTEXT_PTR Context
    )
{
    /***************************************************************************
    ** Initialize image matrices.
    */
    vgmENTERSUBAPI(vgfInitializeMatrixSet);
    _InitializeContainer(
        Context,
        &Context->imageFillSurfaceToPaint,
        _InvalidateImageFillSurfaceToPaint,
        _UpdateImageFillSurfaceToPaint,
        gcvFALSE
        );

    _InitializeContainer(
        Context,
        &Context->glyphSurfaceToImage,
        _InvalidateGlyphSurfaceToImage,
        _UpdateGlyphSurfaceToImage,
        gcvFALSE
        );

    _InitializeContainer(
        Context,
        &Context->imageSurfaceToImage,
        _InvalidateImageSurfaceToImage,
        _UpdateImageSurfaceToImage,
        gcvFALSE
        );


    /***************************************************************************
    ** Initialize path matrices.
    */

    _InitializeContainer(
        Context,
        &Context->pathFillSurfaceToPaint,
        _InvalidatePathFillSurfaceToPaint,
        _UpdatePathFillSurfaceToPaint,
        gcvFALSE
        );

    _InitializeContainer(
        Context,
        &Context->pathStrokeSurfaceToPaint,
        _InvalidatePathStrokeSurfaceToPaint,
        _UpdatePathStrokeSurfaceToPaint,
        gcvFALSE
        );

    _InitializeContainer(
        Context,
        &Context->glyphFillSurfaceToPaint,
        _InvalidateGlyphFillSurfaceToPaint,
        _UpdateGlyphFillSurfaceToPaint,
        gcvFALSE
        );

    _InitializeContainer(
        Context,
        &Context->glyphStrokeSurfaceToPaint,
        _InvalidateGlyphStrokeSurfaceToPaint,
        _UpdateGlyphStrokeSurfaceToPaint,
        gcvFALSE
        );


    /***************************************************************************
    ** Initialize intermediate matrices.
    */

    _InitializeContainer(
        Context,
        &Context->translatedGlyphUserToSurface,
        _InvalidateTranslatedGlyphUserToSurface,
        _UpdateTranslatedGlyphUserToSurface,
        gcvFALSE
        );


    /***************************************************************************
    ** Initialize root VG matrices.
    */

    _InitializeContainer(
        Context,
        &Context->pathUserToSurface,
        _InvalidatePathUserToSurface,
        _UpdatePathUserToSurface,
        gcvTRUE
        );

    _InitializeContainer(
        Context,
        &Context->glyphUserToSurface,
        _InvalidateGlyphUserToSurface,
        _UpdateGlyphUserToSurface,
        gcvTRUE
        );

    _InitializeContainer(
        Context,
        &Context->imageUserToSurface,
        _InvalidateImageUserToSurface,
        _UpdateImageUserToSurface,
        gcvTRUE
        );

    _InitializeContainer(
        Context,
        &Context->fillPaintToUser,
        _InvalidateFillPaintToUser,
        _UpdateFillPaintToUser,
        gcvTRUE
        );

    _InitializeContainer(
        Context,
        &Context->strokePaintToUser,
        _InvalidateStrokePaintToUser,
        _UpdateStrokePaintToUser,
        gcvTRUE
        );


    /***************************************************************************
    ** Set the current matrix.
    */

    Context->matrixMode = VG_MATRIX_PATH_USER_TO_SURFACE;
    Context->matrix = &Context->pathUserToSurface;
    Context->currentUserToSurface = gcvNULL;
    vgmLEAVESUBAPI(vgfInitializeMatrixSet);
}


/*******************************************************************************
**
** vgfGetIdentity
**
** Returns a pointer to the identity matrix.
**
** INPUT:
**
**    Nothing.
**
** OUTPUT:
**
**    Pointer to the read-only identity matrix.
*/

vgsMATRIX_PTR vgfGetIdentity(
    IN vgsCONTEXT_PTR Context
    )
{
    vgmENTERSUBAPI(vgfGetIdentity);
    vgmLEAVESUBAPI(vgfGetIdentity);
    return &_identityMatrix;
}


/*******************************************************************************
**
** vgfIsAffine
**
** A 3x3 matrix is affine if its last row is equal to (0, 0, 1).
**
** INPUT:
**
**    Matrix
**       Pointer to the matrix.
**
** OUTPUT:
**
**    Not zero if the matrix is affine.
*/

gctBOOL vgfIsAffine(
    IN vgsCONTEXT_PTR Context,
    IN const vgsMATRIX_PTR Matrix
    )
{
    vgmENTERSUBAPI(vgfIsAffine);
    vgmLEAVESUBAPI(vgfIsAffine);
    return
        (vgmMAT(Matrix, 2, 0) == 0.0f) &&
        (vgmMAT(Matrix, 2, 1) == 0.0f) &&
        (vgmMAT(Matrix, 2, 2) == 1.0f);
}


/*******************************************************************************
**
** vgfIsIdentity
**
** Verifies whether the specified matrix is an identity matrix.
**
** INPUT:
**
**    Matrix
**       Pointer to the matrix.
**
** OUTPUT:
**
**    Returns gcvTRUE if the matrix is indentity.
*/

gctBOOL vgfIsIdentity(
    IN vgsCONTEXT_PTR Context,
    IN OUT vgsMATRIX_PTR Matrix
    )
{
    gcmVERIFY_BOOLEAN(Matrix->identityDirty);
    gcmVERIFY_BOOLEAN(Matrix->identity);
    {
        vgmENTERSUBAPI(vgfIsIdentity);
        if (Matrix->identityDirty)
        {
            gctINT compare;

            /* Compare the matrix values to the values of identity matrix. */
            compare = memcmp(
                vgmGETMATRIXVALUES(&_identityMatrix),
                vgmGETMATRIXVALUES(Matrix),
                gcmSIZEOF(vgtMATRIXVALUES)
                );

            /* Set idenity flags. */
            Matrix->identityDirty = gcvFALSE;
            Matrix->identity = (compare == 0);
        }
        vgmLEAVESUBAPI(vgfIsIdentity);
    }
    /* Return identity flag. */
    return Matrix->identity;
}


/*******************************************************************************
**
** vgfInvalidateMatrix
**
** Invalidate the internal flags for intermediate matrices.
**
** INPUT:
**
**    Matrix
**       Pointer to the matrix.
**
** OUTPUT:
**
**    Matrix determinant.
*/

#if defined(__QNXNTO__)
void vgfInvalidateMatrix(
    IN vgsCONTEXT_PTR Context,
    IN OUT vgsMATRIX_PTR Matrix
    )
#else
vgmINLINE void vgfInvalidateMatrix(
    IN vgsCONTEXT_PTR Context,
    IN OUT vgsMATRIX_PTR Matrix
    )
#endif
{
    vgmENTERSUBAPI(vgfInvalidateMatrix);
    /* Reset internal flags. */
    Matrix->valuesDirty = gcvTRUE;
    Matrix->detDirty    = gcvTRUE;

    /* For simplicity assume not identity. */
    Matrix->identityDirty = gcvFALSE;
    Matrix->identity      = gcvFALSE;
    vgmLEAVESUBAPI(vgfInvalidateMatrix);
}


/*******************************************************************************
**
** vgfInvalidateContainer
**
** Invalidate the internal flags of a contained matrix.
**
** INPUT:
**
**    Context
**       Pointer to the context.
**
**    Container
**       Pointer to the matrix container.
**
** OUTPUT:
**
**    Matrix determinant.
*/
#if defined(__QNXNTO__)
void vgfInvalidateContainer(
    IN vgsCONTEXT_PTR Context,
    IN OUT vgsMATRIXCONTAINER_PTR Container
    )
#else
vgmINLINE void vgfInvalidateContainer(
    IN vgsCONTEXT_PTR Context,
    IN OUT vgsMATRIXCONTAINER_PTR Container
    )
#endif
{
    vgmENTERSUBAPI(vgfInvalidateContainer);
    {
        /* Invalidate all flags. */
        vgfInvalidateMatrix(Context, vgmGETMATRIX(Container));

        /* Invalidate the matrix dependents. */
        Container->invalidate(Context);
    }
    vgmLEAVESUBAPI(vgfInvalidateContainer);
}


/*******************************************************************************
**
** vgfGetDeterminant
**
** Returns determinant of the specified 3x3 matrix.
**
** INPUT:
**
**    Matrix
**       Pointer to the matrix.
**
** OUTPUT:
**
**    Matrix determinant.
*/
gctFLOAT vgfGetDeterminant(
    IN vgsCONTEXT_PTR Context,
    IN OUT vgsMATRIX_PTR Matrix
    )
{
    gcmVERIFY_BOOLEAN(Matrix->detDirty);
    {
    vgmENTERSUBAPI(vgfGetDeterminant);
    if (Matrix->detDirty)
    {
        Matrix->detDirty = gcvFALSE;

        if (vgfIsIdentity(Context, Matrix))
        {
            Matrix->det = 1.0f;
        }
        else
        {
            Matrix->det

                = vgmMAT(Matrix, 0, 0) *
                    (
                        vgmMAT(Matrix, 1, 1) * vgmMAT(Matrix, 2, 2) -
                        vgmMAT(Matrix, 1, 2) * vgmMAT(Matrix, 2, 1)
                    )

                + vgmMAT(Matrix, 0, 1) *
                    (
                        vgmMAT(Matrix, 1, 2) * vgmMAT(Matrix, 2, 0) -
                        vgmMAT(Matrix, 1, 0) * vgmMAT(Matrix, 2, 2)
                    )

                + vgmMAT(Matrix, 0, 2) *
                    (
                        vgmMAT(Matrix, 1, 0) * vgmMAT(Matrix, 2, 1) -
                        vgmMAT(Matrix, 1, 1) * vgmMAT(Matrix, 2, 0)
                    );
        }
    }
    vgmLEAVESUBAPI(vgfGetDeterminant);
    }
    /* Return identity flag. */
    return Matrix->det;
}


/*******************************************************************************
**
** vgfInvertMatrix
**
** Computes the inverse of the specified matrix.
**
** INPUT:
**
**    Matrix
**       Pointer to the matrix.
**
** OUTPUT:
**
**    Matrix determinant.
*/

gctBOOL vgfInvertMatrix(
    IN vgsCONTEXT_PTR Context,
    IN const vgsMATRIX_PTR Matrix,
    OUT vgsMATRIX_PTR Result
    )
{
    gctBOOL result = gcvTRUE;
    vgmENTERSUBAPI(vgfInvertMatrix);


    /* Identity matrix? */
    if (vgfIsIdentity(Context, Matrix))
    {
        gcoOS_MemCopy(
            Result, Matrix, gcmSIZEOF(vgsMATRIX)
            );
    }
    else
    {
        /* Get the matrix determinant. */
        gctFLOAT det = vgfGetDeterminant(Context, Matrix);

        /* Can only compute inverse if the determinant is not zero. */
        if (det == 0.0f)
        {
            result = gcvFALSE;
            break;
        }

        /* Find the 1 over determinant. */
        det = 1.0f / det;

        /* ROW 0. */
        vgmMAT(Result, 0, 0)
            = (vgmMAT(Matrix, 2, 2) * vgmMAT(Matrix, 1, 1)
            -  vgmMAT(Matrix, 2, 1) * vgmMAT(Matrix, 1, 2)) * det;

        vgmMAT(Result, 0, 1)
            = (vgmMAT(Matrix, 2, 1) * vgmMAT(Matrix, 0, 2)
            -  vgmMAT(Matrix, 2, 2) * vgmMAT(Matrix, 0, 1)) * det;

        vgmMAT(Result, 0, 2)
            = (vgmMAT(Matrix, 1, 2) * vgmMAT(Matrix, 0, 1)
            -  vgmMAT(Matrix, 1, 1) * vgmMAT(Matrix, 0, 2)) * det;

        /* ROW 1. */
        vgmMAT(Result, 1, 0)
            = (vgmMAT(Matrix, 2, 0) * vgmMAT(Matrix, 1, 2)
            -  vgmMAT(Matrix, 2, 2) * vgmMAT(Matrix, 1, 0)) * det;

        vgmMAT(Result, 1, 1)
            = (vgmMAT(Matrix, 2, 2) * vgmMAT(Matrix, 0, 0)
            -  vgmMAT(Matrix, 2, 0) * vgmMAT(Matrix, 0, 2)) * det;

        vgmMAT(Result, 1, 2)
            = (vgmMAT(Matrix, 1, 0) * vgmMAT(Matrix, 0, 2)
            -  vgmMAT(Matrix, 1, 2) * vgmMAT(Matrix, 0, 0)) * det;

        /* ROW 2. */
        if (vgfIsAffine(Context, Matrix))
        {
            vgmMAT(Result, 2, 0) = 0.0f;
            vgmMAT(Result, 2, 1) = 0.0f;
            vgmMAT(Result, 2, 2) = 1.0f;
        }
        else
        {
            vgmMAT(Result, 2, 0)
                = (vgmMAT(Matrix, 2, 1) * vgmMAT(Matrix, 1, 0)
                -  vgmMAT(Matrix, 2, 0) * vgmMAT(Matrix, 1, 1)) * det;

            vgmMAT(Result, 2, 1)
                = (vgmMAT(Matrix, 2, 0) * vgmMAT(Matrix, 0, 1)
                -  vgmMAT(Matrix, 2, 1) * vgmMAT(Matrix, 0, 0)) * det;

            vgmMAT(Result, 2, 2)
                = (vgmMAT(Matrix, 1, 1) * vgmMAT(Matrix, 0, 0)
                -  vgmMAT(Matrix, 1, 0) * vgmMAT(Matrix, 0, 1)) * det;
        }
        /* Set matrix flags. */
        Result->identityDirty = gcvFALSE;
        Result->identity      = gcvFALSE;
        Result->detDirty      = gcvTRUE;
    }

    vgmLEAVESUBAPI(vgfInvertMatrix);
    /* Success. */
    return result;
}


/*******************************************************************************
**
** vgfMultiplyVector2ByMatrix2x2
** vgfMultiplyVector2ByMatrix3x2
** vgfMultiplyVector3ByMatrix3x3
**
** Multiply the specified vertical vector and matrix.
**
** INPUT:
**
**    Vector
**       Pointer to the vertical vector.
**
**    Matrix
**       Pointer to the matrix.
**
** OUTPUT:
**
**    Result
**       Pointer to the result product vector.
*/

void vgfMultiplyVector2ByMatrix2x2(
    IN vgsCONTEXT_PTR Context,
    IN const vgtFLOATVECTOR2 Vector,
    IN const vgsMATRIX_PTR Matrix,
    OUT vgtFLOATVECTOR2 Result
    )
{
    vgmENTERSUBAPI(vgfMultiplyVector2ByMatrix2x2);

    if (vgfIsIdentity(Context, Matrix))
    {
        gcoOS_MemCopy(
            Result, Vector, gcmSIZEOF(vgtFLOATVECTOR2)
            );
    }
    else
    {
        VGfloat x
            = vgmMAT(Matrix, 0, 0) * Vector[0]
            + vgmMAT(Matrix, 0, 1) * Vector[1];

        VGfloat y
            = vgmMAT(Matrix, 1, 0) * Vector[0]
            + vgmMAT(Matrix, 1, 1) * Vector[1];

        Result[0] = x;
        Result[1] = y;
    }
    vgmLEAVESUBAPI(vgfMultiplyVector2ByMatrix2x2);
}

void vgfMultiplyVector2ByMatrix3x2(
    IN vgsCONTEXT_PTR Context,
    IN const vgtFLOATVECTOR2 Vector,
    IN const vgsMATRIX_PTR Matrix,
    OUT vgtFLOATVECTOR2 Result
    )
{
    vgmENTERSUBAPI(vgfMultiplyVector2ByMatrix3x2);

    if (vgfIsIdentity(Context, Matrix))
    {
        gcoOS_MemCopy(
            Result, Vector, gcmSIZEOF(vgtFLOATVECTOR2)
            );
    }
    else
    {
        VGfloat x
            = vgmMAT(Matrix, 0, 0) * Vector[0]
            + vgmMAT(Matrix, 0, 1) * Vector[1]
            + vgmMAT(Matrix, 0, 2);

        VGfloat y
            = vgmMAT(Matrix, 1, 0) * Vector[0]
            + vgmMAT(Matrix, 1, 1) * Vector[1]
            + vgmMAT(Matrix, 1, 2);

        Result[0] = x;
        Result[1] = y;
    }
    vgmLEAVESUBAPI(vgfMultiplyVector2ByMatrix3x2);
}


void vgfMultiplyVector3ByMatrix3x3(
    IN vgsCONTEXT_PTR Context,
    IN const vgtFLOATVECTOR3 Vector,
    IN const vgsMATRIX_PTR Matrix,
    OUT vgtFLOATVECTOR3 Result
    )
{
    vgmENTERSUBAPI(vgfMultiplyVector3ByMatrix3x3);
    if (vgfIsIdentity(Context, Matrix))
    {
        gcoOS_MemCopy(
            Result, Vector, gcmSIZEOF(vgtFLOATVECTOR3)
            );
    }
    else
    {
        VGfloat x
            = vgmMAT(Matrix, 0, 0) * Vector[0]
            + vgmMAT(Matrix, 0, 1) * Vector[1]
            + vgmMAT(Matrix, 0, 2) * Vector[2];

        VGfloat y
            = vgmMAT(Matrix, 1, 0) * Vector[0]
            + vgmMAT(Matrix, 1, 1) * Vector[1]
            + vgmMAT(Matrix, 1, 2) * Vector[2];

        VGfloat z
            = vgmMAT(Matrix, 2, 0) * Vector[0]
            + vgmMAT(Matrix, 2, 1) * Vector[1]
            + vgmMAT(Matrix, 2, 2) * Vector[2];

        Result[0] = x;
        Result[1] = y;
        Result[2] = z;
    }
    vgmLEAVESUBAPI(vgfMultiplyVector3ByMatrix3x3);
}


/*******************************************************************************
**
** vgfMultiplyMatrix3x3
**
** Multiply the specified 3x3 matrices.
**
** INPUT:
**
**    Matrix1
**       Pointer to the first matrix.
**
**    Matrix2
**       Pointer to the second matrix.
**
** OUTPUT:
**
**    Result
**       Pointer to the result matrix.
*/

void vgfMultiplyMatrix3x3(
    IN vgsCONTEXT_PTR Context,
    IN const vgsMATRIX_PTR Matrix1,
    IN const vgsMATRIX_PTR Matrix2,
    OUT vgsMATRIX_PTR Result
    )
{
    vgmENTERSUBAPI(vgfMultiplyMatrix3x3);
    if (vgfIsIdentity(Context, Matrix1))
    {
        gcoOS_MemCopy(
            Result, Matrix2, gcmSIZEOF(vgsMATRIX)
            );
    }

    else if (vgfIsIdentity(Context, Matrix2))
    {
        gcoOS_MemCopy(
            Result, Matrix1, gcmSIZEOF(vgsMATRIX)
            );
    }

    else
    {

        gctFLOAT e00
            = vgmMAT(Matrix1, 0, 0) * vgmMAT(Matrix2, 0, 0)
            + vgmMAT(Matrix1, 0, 1) * vgmMAT(Matrix2, 1, 0)
            + vgmMAT(Matrix1, 0, 2) * vgmMAT(Matrix2, 2, 0);

        gctFLOAT e01
            = vgmMAT(Matrix1, 0, 0) * vgmMAT(Matrix2, 0, 1)
            + vgmMAT(Matrix1, 0, 1) * vgmMAT(Matrix2, 1, 1)
            + vgmMAT(Matrix1, 0, 2) * vgmMAT(Matrix2, 2, 1);

        gctFLOAT e02
            = vgmMAT(Matrix1, 0, 0) * vgmMAT(Matrix2, 0, 2)
            + vgmMAT(Matrix1, 0, 1) * vgmMAT(Matrix2, 1, 2)
            + vgmMAT(Matrix1, 0, 2) * vgmMAT(Matrix2, 2, 2);

        gctFLOAT e10
            = vgmMAT(Matrix1, 1, 0) * vgmMAT(Matrix2, 0, 0)
            + vgmMAT(Matrix1, 1, 1) * vgmMAT(Matrix2, 1, 0)
            + vgmMAT(Matrix1, 1, 2) * vgmMAT(Matrix2, 2, 0);

        gctFLOAT e11
            = vgmMAT(Matrix1, 1, 0) * vgmMAT(Matrix2, 0, 1)
            + vgmMAT(Matrix1, 1, 1) * vgmMAT(Matrix2, 1, 1)
            + vgmMAT(Matrix1, 1, 2) * vgmMAT(Matrix2, 2, 1);

        gctFLOAT e12
            = vgmMAT(Matrix1, 1, 0) * vgmMAT(Matrix2, 0, 2)
            + vgmMAT(Matrix1, 1, 1) * vgmMAT(Matrix2, 1, 2)
            + vgmMAT(Matrix1, 1, 2) * vgmMAT(Matrix2, 2, 2);

        gctFLOAT e20
            = vgmMAT(Matrix1, 2, 0) * vgmMAT(Matrix2, 0, 0)
            + vgmMAT(Matrix1, 2, 1) * vgmMAT(Matrix2, 1, 0)
            + vgmMAT(Matrix1, 2, 2) * vgmMAT(Matrix2, 2, 0);

        gctFLOAT e21
            = vgmMAT(Matrix1, 2, 0) * vgmMAT(Matrix2, 0, 1)
            + vgmMAT(Matrix1, 2, 1) * vgmMAT(Matrix2, 1, 1)
            + vgmMAT(Matrix1, 2, 2) * vgmMAT(Matrix2, 2, 1);

        gctFLOAT e22
            = vgmMAT(Matrix1, 2, 0) * vgmMAT(Matrix2, 0, 2)
            + vgmMAT(Matrix1, 2, 1) * vgmMAT(Matrix2, 1, 2)
            + vgmMAT(Matrix1, 2, 2) * vgmMAT(Matrix2, 2, 2);
        Result->identityDirty = gcvFALSE;
        Result->identity      = gcvFALSE;
        Result->detDirty      = gcvTRUE;

        vgmMAT(Result, 0, 0) = e00;
        vgmMAT(Result, 0, 1) = e01;
        vgmMAT(Result, 0, 2) = e02;

        vgmMAT(Result, 1, 0) = e10;
        vgmMAT(Result, 1, 1) = e11;
        vgmMAT(Result, 1, 2) = e12;

        vgmMAT(Result, 2, 0) = e20;
        vgmMAT(Result, 2, 1) = e21;
        vgmMAT(Result, 2, 2) = e22;
    }
    vgmLEAVESUBAPI(vgfMultiplyMatrix3x3);
}


/******************************************************************************\
****************************** OpenVG Matrix API. ******************************
\******************************************************************************/

/*******************************************************************************
**
** vgLoadIdentity
**
** The vgLoadIdentity function sets the current matrix M to the identity
** matrix:
**
**        | 1 0 0 |
**    M = | 0 1 0 |
**        | 0 0 1 |
**
** INPUT:
**
**    Nothing.
**
** OUTPUT:
**
**    Nothing.
*/

VG_API_CALL void VG_API_ENTRY vgLoadIdentity(
    void
    )
{
    vgmENTERAPI(vgLoadIdentity)
    {
        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s();\n",
            __FUNCTION__
            );

        /* Set the current matrix to identity. */
        _LoadIdentity(Context, Context->matrix);
    }
    vgmLEAVEAPI(vgLoadIdentity);
}

VG_API_CALL void VG_API_ENTRY vgLoadMatrixInvImage(VGfloat matrix[], VGfloat invMatrix[])
{
    vgmENTERAPI(vgLoadMatrixInvImage)
    {
        gcoOS_MemCopy(Context->imageUserToSurface.matrix.values, matrix, sizeof(VGfloat) * 9);
        gcoOS_MemCopy(Context->imageSurfaceToImage.matrix.values, invMatrix, sizeof(VGfloat) * 9);
        Context->imageUserToSurface.matrix.valuesDirty = gcvFALSE;
        Context->imageUserToSurface.matrix.forceLoad = gcvTRUE;
        Context->imageSurfaceToImage.matrix.valuesDirty = gcvFALSE;
        Context->imageSurfaceToImage.matrix.forceLoad = gcvTRUE;
    }
    vgmLEAVEAPI(vgLoadMatrixInvImage);
}

/*******************************************************************************
**
** vgLoadInverseMatrix
**
** The vgLoadInverseMatrix function loads a non-singular inverse matrix values into the
** DrawPath matrix or the DrawImage matrix. Nine matrix values are read from Matrix.
**
** INPUT:
**
**    Matrix
**       Pointer to the inverse matrix to be loaded.
**    MatrixMode
**       Load the matrix for vgDrawPath or vgDrawImage.
**    PaintModes
**       Bitwise OR of VG_FILL_PATH and/or VG_STROKE_PATH (VGPaintMode).
**
** OUTPUT:
**
**    Nothing.
*/

VG_API_CALL void VG_API_ENTRY vgLoadInverseMatrix(
    const VGfloat * Matrix,
    VGMatrixMode MatrixMode,
    VGbitfield PaintModes
    )
{
    vgmENTERAPI(vgLoadInverseMatrix)
    {
        vgsMATRIXCONTAINER_PTR matrix, matrix1, matrix2;

        vgmDUMP_FLOAT_ARRAY(
            Matrix, 9
            );

        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s(Matrix);\n",
            __FUNCTION__
            );

        /* Validate the input matrix. */
        if (vgmIS_INVALID_PTR(Matrix, 4))
        {
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            break;
        }

        if (MatrixMode == VG_MATRIX_PATH_USER_TO_SURFACE)
        {
            if (PaintModes == 1)
            {
                matrix = &Context->pathStrokeSurfaceToPaint;
            }
            else if (PaintModes == 2)
            {
                matrix = &Context->pathFillSurfaceToPaint;
            }
            else if (PaintModes == 3)
            {
                matrix1 = &Context->pathStrokeSurfaceToPaint;
                matrix2 = &Context->pathFillSurfaceToPaint;
            }
            else
            {
                vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
                return;
            }

        }
        else if (MatrixMode == VG_MATRIX_IMAGE_USER_TO_SURFACE)
        {
            matrix = &Context->imageFillSurfaceToPaint;
            PaintModes = 0;
        }
        else
        {
            gcoOS_Print("not support load inverse matrix except for vgDrawPath or vgDrawImage!");
            return;
        }

        if ((PaintModes == 3))
        {
            /* Load the current matrix. */
            gcoOS_MemCopy(
                matrix1,
                Matrix,
                gcmSIZEOF(vgtMATRIXVALUES)
                );

            gcoOS_MemCopy(
                matrix2,
                Matrix,
                gcmSIZEOF(vgtMATRIXVALUES)
                );

            matrix1->matrix.valid = 1;
            matrix2->matrix.valid = 1;
            /* Reset matrix. */
            vgmGETMATRIX(matrix1)->valuesDirty = gcvFALSE;
            vgmGETMATRIX(matrix1)->detDirty    = gcvTRUE;
            vgmGETMATRIX(matrix2)->valuesDirty = gcvFALSE;
            vgmGETMATRIX(matrix2)->detDirty    = gcvTRUE;

            /* For simplicity assume not identity. */
            vgmGETMATRIX(matrix1)->identityDirty = gcvFALSE;
            vgmGETMATRIX(matrix1)->identity      = gcvFALSE;
            vgmGETMATRIX(matrix2)->identityDirty = gcvFALSE;
            vgmGETMATRIX(matrix2)->identity      = gcvFALSE;
        }
        else
        {
            /* Load the current matrix. */
            gcoOS_MemCopy(
                matrix,
                Matrix,
                gcmSIZEOF(vgtMATRIXVALUES)
                );
            matrix->matrix.valid = 1;
            /* Reset matrix. */
            vgmGETMATRIX(matrix)->valuesDirty = gcvFALSE;
            vgmGETMATRIX(matrix)->detDirty    = gcvTRUE;

            /* For simplicity assume not identity. */
            vgmGETMATRIX(matrix)->identityDirty = gcvFALSE;
            vgmGETMATRIX(matrix)->identity      = gcvFALSE;

        }

    }
    vgmLEAVEAPI(vgLoadInverseMatrix);
}

/*******************************************************************************
**
** vgLoadMatrix
**
** The vgLoadMatrix function loads an arbitrary set of matrix values into the
** current matrix. Nine matrix values are read from m, in the order:
**
**    { ScaleX, ShearY, w0, ShearX, ScaleY, w1, TransformX, TransformY, w2 }
**
** defining the matrix:
**
**        | ScaleX ShearX TransformX |
**    M = | ShearY ScaleY TransformY |
**        | w0     w1     w2         |
**
** However, if the targeted matrix is affine (i.e., the matrix mode is not
** VG_MATRIX_IMAGE_USER_TO_SURFACE), the values { w0, w1, w2 } are ignored and
** replaced by the values { 0, 0, 1 }, resulting in the affine matrix:
**
**        | ScaleX ShearX TransformX |
**    M = | ShearY ScaleY TransformY |
**        | 0      0      1          |
**
** INPUT:
**
**    Matrix
**       Pointer to the matrix to be loaded.
**
** OUTPUT:
**
**    Nothing.
*/

VG_API_CALL void VG_API_ENTRY vgLoadMatrix(
    const VGfloat * Matrix
    )
{
    vgmENTERAPI(vgLoadMatrix)
    {
        vgsMATRIXCONTAINER_PTR matrix;

        vgmDUMP_FLOAT_ARRAY(
            Matrix, 9
            );

        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s(Matrix);\n",
            __FUNCTION__
            );

        /* Validate the input matrix. */
        if (vgmIS_INVALID_PTR(Matrix, 4))
        {
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            break;
        }

        /* Get a shortcut to the matrix. */
        matrix = Context->matrix;

        /* Load the current matrix. */
        gcoOS_MemCopy(
            matrix,
            Matrix,
            gcmSIZEOF(vgtMATRIXVALUES)
            );

        /* Replace the last row. */
        if (Context->matrixMode != VG_MATRIX_IMAGE_USER_TO_SURFACE)
        {
            vgmMAT(matrix, 2, 0) = 0.0f;
            vgmMAT(matrix, 2, 1) = 0.0f;
            vgmMAT(matrix, 2, 2) = 1.0f;
        }
        /* Reset matrix. */
        vgfInvalidateContainer(Context, matrix);
    }
    vgmLEAVEAPI(vgLoadMatrix);
}


/*******************************************************************************
**
** vgGetMatrix
**
** It is possible to retrieve the value of the current transformation by
** calling vgGetMatrix. Nine values are written to Matrix in the order:
**
**    { ScaleX, ShearY, w0, ShearX, ScaleY, w1, TransformX, TransformY, w2 }
**
** For an affine matrix, w0 and w1 will always be 0 and w2 will always be 1.
**
** INPUT:
**
**    Matrix
**       Pointer to the matrix receiving the value of the current
**       transformation.
**
** OUTPUT:
**
**    Matrix
**       Copied value of the current transformation.
*/

VG_API_CALL void VG_API_ENTRY vgGetMatrix(
    VGfloat * Matrix
    )
{
    vgmENTERAPI(vgGetMatrix)
    {
        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s(0x%08X);\n",
            __FUNCTION__,
            Matrix
            );

        /* Validate the output matrix. */
        if (vgmIS_INVALID_PTR(Matrix, 4))
        {
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            break;
        }


        /* Return the current matrix. */
        gcoOS_MemCopy(
            Matrix,
            vgmGETMATRIXVALUES(Context->matrix),
            gcmSIZEOF(vgtMATRIXVALUES)
            );
    }
    vgmLEAVEAPI(vgGetMatrix);
}


/*******************************************************************************
**
** vgMultMatrix
**
** The vgMultMatrix function right-multiplies the current matrix M by a given
** matrix:
**
**              | ScaleX ShearX TransformX |
**    M <-- M * | ShearY ScaleY TransformY |
**              | w0     w1     w2         |
**
** Nine matrix values are read from Matrix in the order:
**
**    { ScaleX, ShearY, w0, ShearX, ScaleY, w1, TransformX, TransformY, w2 }
**
** and the current matrix is multiplied by the resulting matrix. However, if
** the targeted matrix is affine (i.e., the matrix mode is not
** VG_MATRIX_IMAGE_USER_TO_SURFACE), the values { w0, w1, w2 } are ignored
** and replaced by the values { 0, 0, 1 } prior to multiplication.
**
** INPUT:
**
**    Matrix
**       Pointer to the matrix to be multiplied with the current
**       transformation.
**
** OUTPUT:
**
**    Nothing.
*/

VG_API_CALL void VG_API_ENTRY vgMultMatrix(
    const VGfloat * Matrix
    )
{
    vgmENTERAPI(vgMultMatrix)
    {
        vgsMATRIX result;
        vgsMATRIXCONTAINER_PTR matrix;

        vgmDUMP_FLOAT_ARRAY(
            Matrix, 9
            );

        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s(Matrix);\n",
            __FUNCTION__
            );

        /* Validate the input matrix. */
        if (vgmIS_INVALID_PTR(Matrix, 4))
        {
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            break;
        }

        /* Create a shortcut to the matrix. */
        matrix = Context->matrix;


        if (Context->matrixMode == VG_MATRIX_IMAGE_USER_TO_SURFACE)
        {
            /* ROW 0. */
            vgmMAT(&result, 0, 0)
                = vgmMAT(matrix, 0, 0) * vgmMAT(Matrix, 0, 0)
                + vgmMAT(matrix, 0, 1) * vgmMAT(Matrix, 1, 0)
                + vgmMAT(matrix, 0, 2) * vgmMAT(Matrix, 2, 0);

            vgmMAT(&result, 0, 1)
                = vgmMAT(matrix, 0, 0) * vgmMAT(Matrix, 0, 1)
                + vgmMAT(matrix, 0, 1) * vgmMAT(Matrix, 1, 1)
                + vgmMAT(matrix, 0, 2) * vgmMAT(Matrix, 2, 1);

            vgmMAT(&result, 0, 2)
                = vgmMAT(matrix, 0, 0) * vgmMAT(Matrix, 0, 2)
                + vgmMAT(matrix, 0, 1) * vgmMAT(Matrix, 1, 2)
                + vgmMAT(matrix, 0, 2) * vgmMAT(Matrix, 2, 2);

            /* ROW 1. */
            vgmMAT(&result, 1, 0)
                = vgmMAT(matrix, 1, 0) * vgmMAT(Matrix, 0, 0)
                + vgmMAT(matrix, 1, 1) * vgmMAT(Matrix, 1, 0)
                + vgmMAT(matrix, 1, 2) * vgmMAT(Matrix, 2, 0);

            vgmMAT(&result, 1, 1)
                = vgmMAT(matrix, 1, 0) * vgmMAT(Matrix, 0, 1)
                + vgmMAT(matrix, 1, 1) * vgmMAT(Matrix, 1, 1)
                + vgmMAT(matrix, 1, 2) * vgmMAT(Matrix, 2, 1);

            vgmMAT(&result, 1, 2)
                = vgmMAT(matrix, 1, 0) * vgmMAT(Matrix, 0, 2)
                + vgmMAT(matrix, 1, 1) * vgmMAT(Matrix, 1, 2)
                + vgmMAT(matrix, 1, 2) * vgmMAT(Matrix, 2, 2);

            /* ROW 2. */
            vgmMAT(&result, 2, 0)
                = vgmMAT(matrix, 2, 0) * vgmMAT(Matrix, 0, 0)
                + vgmMAT(matrix, 2, 1) * vgmMAT(Matrix, 1, 0)
                + vgmMAT(matrix, 2, 2) * vgmMAT(Matrix, 2, 0);

            vgmMAT(&result, 2, 1)
                = vgmMAT(matrix, 2, 0) * vgmMAT(Matrix, 0, 1)
                + vgmMAT(matrix, 2, 1) * vgmMAT(Matrix, 1, 1)
                + vgmMAT(matrix, 2, 2) * vgmMAT(Matrix, 2, 1);

            vgmMAT(&result, 2, 2)
                = vgmMAT(matrix, 2, 0) * vgmMAT(Matrix, 0, 2)
                + vgmMAT(matrix, 2, 1) * vgmMAT(Matrix, 1, 2)
                + vgmMAT(matrix, 2, 2) * vgmMAT(Matrix, 2, 2);
        }
        else
        {
            /* ROW 0. */
            vgmMAT(&result, 0, 0)
                = vgmMAT(matrix, 0, 0) * vgmMAT(Matrix, 0, 0)
                + vgmMAT(matrix, 0, 1) * vgmMAT(Matrix, 1, 0);

            vgmMAT(&result, 0, 1)
                = vgmMAT(matrix, 0, 0) * vgmMAT(Matrix, 0, 1)
                + vgmMAT(matrix, 0, 1) * vgmMAT(Matrix, 1, 1);

            vgmMAT(&result, 0, 2)
                = vgmMAT(matrix, 0, 0) * vgmMAT(Matrix, 0, 2)
                + vgmMAT(matrix, 0, 1) * vgmMAT(Matrix, 1, 2)
                + vgmMAT(matrix, 0, 2);

            /* ROW 1. */
            vgmMAT(&result, 1, 0)
                = vgmMAT(matrix, 1, 0) * vgmMAT(Matrix, 0, 0)
                + vgmMAT(matrix, 1, 1) * vgmMAT(Matrix, 1, 0);

            vgmMAT(&result, 1, 1)
                = vgmMAT(matrix, 1, 0) * vgmMAT(Matrix, 0, 1)
                + vgmMAT(matrix, 1, 1) * vgmMAT(Matrix, 1, 1);

            vgmMAT(&result, 1, 2)
                = vgmMAT(matrix, 1, 0) * vgmMAT(Matrix, 0, 2)
                + vgmMAT(matrix, 1, 1) * vgmMAT(Matrix, 1, 2)
                + vgmMAT(matrix, 1, 2);

            /* ROW 2. */
            vgmMAT(&result, 2, 0) = 0.0f;
            vgmMAT(&result, 2, 1) = 0.0f;
            vgmMAT(&result, 2, 2) = 1.0f;
        }
        /* Set the result matrix. */
        gcoOS_MemCopy(
            vgmGETMATRIXVALUES(matrix),
            vgmGETMATRIXVALUES(&result),
            gcmSIZEOF(vgtMATRIXVALUES)
            );

        /* Reset matrix. */
        vgfInvalidateContainer(Context, matrix);
    }
    vgmLEAVEAPI(vgMultMatrix);
}


/*******************************************************************************
**
** vgTranslate
**
** The vgTranslate function modifies the current transformation by appending a
** translation. This is equivalent to right-multiplying the current matrix M
** by a translation matrix:
**
**              | 1 0 TransformX |
**    M <-- M * | 0 1 TransformY |
**              | 0 0 1          |
**
** INPUT:
**
**    TranslateX, TranslateY
**       Translation matrix values.
**
** OUTPUT:
**
**    Nothing.
*/

VG_API_CALL void VG_API_ENTRY vgTranslate(
    VGfloat TranslateX,
    VGfloat TranslateY
    )
{
    vgmENTERAPI(vgTranslate)
    {
        /* Create a shortcut to the matrix. */
        vgsMATRIXCONTAINER_PTR matrix = Context->matrix;

        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s(%.10ff, %.10ff);\n",
            __FUNCTION__,
            TranslateX, TranslateY
            );

        if (gcmIS_NAN(TranslateX) || gcmIS_NAN(TranslateY))
        {
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            break;
        }

        if (Context->matrixMode == VG_MATRIX_IMAGE_USER_TO_SURFACE)
        {
            vgmMAT(matrix, 0, 2)
                += vgmMAT(matrix, 0, 0) * TranslateX
                +  vgmMAT(matrix, 0, 1) * TranslateY;

            vgmMAT(matrix, 1, 2)
                += vgmMAT(matrix, 1, 0) * TranslateX
                +  vgmMAT(matrix, 1, 1) * TranslateY;

            vgmMAT(matrix, 2, 2)
                += vgmMAT(matrix, 2, 0) * TranslateX
                +  vgmMAT(matrix, 2, 1) * TranslateY;
        }
        else
        {
            vgmMAT(matrix, 0, 2)
                += vgmMAT(matrix, 0, 0) * TranslateX
                +  vgmMAT(matrix, 0, 1) * TranslateY;

            vgmMAT(matrix, 1, 2)
                += vgmMAT(matrix, 1, 0) * TranslateX
                +  vgmMAT(matrix, 1, 1) * TranslateY;
        }
        /* Reset matrix. */
        vgfInvalidateContainer(Context, matrix);
    }
    vgmLEAVEAPI(vgTranslate);
}


/*******************************************************************************
**
** vgScale
**
** The vgScale function modifies the current transformation by appending
** a scale. This is equivalent to right-multiplying the current matrix M
** by a scale matrix:
**
**              | ScaleX 0      0 |
**    M <-- M * | 0      ScaleY 0 |
**              | 0      0      1 |
**
** INPUT:
**
**    ScaleX, ScaleY
**       Scale matrix values.
**
** OUTPUT:
**
**    Nothing.
*/

VG_API_CALL void VG_API_ENTRY vgScale(
    VGfloat ScaleX,
    VGfloat ScaleY
    )
{
    vgmENTERAPI(vgScale)
    {
        /* Create a shortcut to the matrix. */
        vgsMATRIXCONTAINER_PTR matrix = Context->matrix;

        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s(%.10ff, %.10ff);\n",
            __FUNCTION__,
            ScaleX, ScaleY
            );

        if (gcmIS_NAN(ScaleX) || gcmIS_NAN(ScaleY))
        {
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            break;
        }

        if (Context->matrixMode == VG_MATRIX_IMAGE_USER_TO_SURFACE)
        {
            vgmMAT(matrix, 0, 0) *= ScaleX;
            vgmMAT(matrix, 0, 1) *= ScaleY;
            vgmMAT(matrix, 1, 0) *= ScaleX;
            vgmMAT(matrix, 1, 1) *= ScaleY;
            vgmMAT(matrix, 2, 0) *= ScaleX;
            vgmMAT(matrix, 2, 1) *= ScaleY;
        }
        else
        {
            vgmMAT(matrix, 0, 0) *= ScaleX;
            vgmMAT(matrix, 0, 1) *= ScaleY;
            vgmMAT(matrix, 1, 0) *= ScaleX;
            vgmMAT(matrix, 1, 1) *= ScaleY;
        }
        /* Reset matrix. */
        vgfInvalidateContainer(Context, matrix);
    }
    vgmLEAVEAPI(vgScale);
}


/*******************************************************************************
**
** vgShear
**
** The vgShear function modifies the current transformation by appending
** a shear. This is equivalent to right-multiplying the current matrix M
** by a shear matrix:
**
**              | 1      ShearX 0 |
**    M <-- M * | ShearY 1      0 |
**              | 0      0      1 |
**
** INPUT:
**
**    ShearX, ShearY
**       Shear matrix values.
**
** OUTPUT:
**
**    Nothing.
*/

VG_API_CALL void VG_API_ENTRY vgShear(
    VGfloat ShearX,
    VGfloat ShearY
    )
{
    vgmENTERAPI(vgShear)
    {
        /* Create a shortcut to the matrix. */
        vgsMATRIXCONTAINER_PTR matrix = Context->matrix;

        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s(%.10ff, %.10ff);\n",
            __FUNCTION__,
            ShearX, ShearY
            );

        if (gcmIS_NAN(ShearX) || gcmIS_NAN(ShearY))
        {
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            break;
        }

        if (Context->matrixMode == VG_MATRIX_IMAGE_USER_TO_SURFACE)
        {
            VGfloat m00 = vgmMAT(matrix, 0, 0);
            VGfloat m10 = vgmMAT(matrix, 1, 0);
            VGfloat m20 = vgmMAT(matrix, 2, 0);

            vgmMAT(matrix, 0, 0)
                += vgmMAT(matrix, 0, 1) * ShearY;

            vgmMAT(matrix, 1, 0)
                += vgmMAT(matrix, 1, 1) * ShearY;

            vgmMAT(matrix, 2, 0)
                += vgmMAT(matrix, 2, 1) * ShearY;

            vgmMAT(matrix, 0, 1) += m00 * ShearX;
            vgmMAT(matrix, 1, 1) += m10 * ShearX;
            vgmMAT(matrix, 2, 1) += m20 * ShearX;
        }
        else
        {
            VGfloat m00 = vgmMAT(matrix, 0, 0);
            VGfloat m10 = vgmMAT(matrix, 1, 0);

            vgmMAT(matrix, 0, 0)
                += vgmMAT(matrix, 0, 1) * ShearY;

            vgmMAT(matrix, 1, 0)
                += vgmMAT(matrix, 1, 1) * ShearY;

            vgmMAT(matrix, 0, 1) += m00 * ShearX;
            vgmMAT(matrix, 1, 1) += m10 * ShearX;
        }
        /* Reset matrix. */
        vgfInvalidateContainer(Context, matrix);
    }
    vgmLEAVEAPI(vgShear);
}


/*******************************************************************************
**
** vgRotate
**
** The vgRotate function modifies the current transformation by appending
** a counterclockwise rotation by a given angle (expressed in degrees) about
** the origin. This is equivalent to right-multiplying the current matrix M
** by the following matrix:
**
**              | cos(Angle) -sin(Angle) 0 |
**    M <-- M * | sin(Angle)  cos(Angle) 0 |
**              | 0           0          1 |
**
** To rotate about a center point (cx, cy) other than the origin, the
** application may perform a translation by (cx, cy), followed by
** the rotation, followed by a translation by (-cx, -cy).
**
** INPUT:
**
**    Angle
**       Rotation angle in degrees.
**
** OUTPUT:
**
**    Nothing.
*/

VG_API_CALL void VG_API_ENTRY vgRotate(
    VGfloat Angle
    )
{
    vgmENTERAPI(vgRotate)
    {
        /* Create a shortcut to the matrix. */
        vgsMATRIXCONTAINER_PTR matrix = Context->matrix;

        /* Compute angle in radians. */
        VGfloat radians = Angle * vgvPI / 180.0f;

        /* Compute sine and cosine of the angle. */
        VGfloat cosAngle = gcmCOSF(radians);
        VGfloat sinAngle = gcmSINF(radians);

        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s(%.10ff);\n",
            __FUNCTION__,
            Angle
            );

        if (gcmIS_NAN(radians))
        {
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            break;
        }

        if (Context->matrixMode == VG_MATRIX_IMAGE_USER_TO_SURFACE)
        {
            VGfloat m00 = vgmMAT(matrix, 0, 0);
            VGfloat m10 = vgmMAT(matrix, 1, 0);
            VGfloat m20 = vgmMAT(matrix, 2, 0);

            vgmMAT(matrix, 0, 0)
                = sinAngle * vgmMAT(matrix, 0, 1)
                + cosAngle * m00;

            vgmMAT(matrix, 0, 1)
                = cosAngle * vgmMAT(matrix, 0, 1)
                - sinAngle * m00;

            vgmMAT(matrix, 1, 0)
                = sinAngle * vgmMAT(matrix, 1, 1)
                + cosAngle * m10;

            vgmMAT(matrix, 1, 1)
                = cosAngle * vgmMAT(matrix, 1, 1)
                - sinAngle * m10;

            vgmMAT(matrix, 2, 0)
                = sinAngle * vgmMAT(matrix, 2, 1)
                + cosAngle * m20;

            vgmMAT(matrix, 2, 1)
                = cosAngle * vgmMAT(matrix, 2, 1)
                - sinAngle * m20;
        }
        else
        {
            VGfloat m00 = vgmMAT(matrix, 0, 0);
            VGfloat m10 = vgmMAT(matrix, 1, 0);

            vgmMAT(matrix, 0, 0)
                = sinAngle * vgmMAT(matrix, 0, 1)
                + cosAngle * m00;

            vgmMAT(matrix, 0, 1)
                = cosAngle * vgmMAT(matrix, 0, 1)
                - sinAngle * m00;

            vgmMAT(matrix, 1, 0)
                = sinAngle * vgmMAT(matrix, 1, 1)
                + cosAngle * m10;

            vgmMAT(matrix, 1, 1)
                = cosAngle * vgmMAT(matrix, 1, 1)
                - sinAngle * m10;
        }
        /* Reset matrix. */
        vgfInvalidateContainer(Context, matrix);
    }
    vgmLEAVEAPI(vgRotate);
}
