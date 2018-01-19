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

/******************************************************************************\
*********************** Support Functions and Definitions **********************
\******************************************************************************/

/* Segment normalization function name. */
#define vgmINTERPOL(Command) \
    _Interpolate_ ## Command

/* Segment transformation function definition. */
#define vgmDEFINEINTERPOLATE(Command) \
    static gceSTATUS vgmINTERPOL(Command) ( \
        vgmINTERPOLATEPARAMETERS \
        )


/*******************************************************************************
**
** _Interpolate
**
** Interpolate between two coordinates.
**
** INPUT:
**
**    StartCoordinate
**    EndCoordinate
**       Coordinates to interpolate between.
**
**    Amount
**       Interpolation value.
**
** OUTPUT:
**
**    Nothing.
*/

vgmINLINE static gctFLOAT _Interpolate(
    vgsCONTEXT_PTR Context,
    gctFLOAT StartCoordinate,
    gctFLOAT EndCoordinate,
    gctFLOAT Amount
    )
{
    gctFLOAT coordinate;
    vgmENTERSUBAPI(_Interpolate);
    coordinate
        = StartCoordinate * (1.0f - Amount)
        + EndCoordinate * Amount;
    vgmLEAVESUBAPI(_Interpolate);
    return coordinate;
}


/*******************************************************************************
**
** _InterpolateArc
**
** Generate an ARC buffer and append it to the specified destination.
**
** INPUT:
**
**    Destination
**       Pointer to the existing path reader / modificator.
**
**    StartCoordinate
**    EndCoordinate
**       Pointers to the coordinates to interpolate between.
**
**    Amount
**       Interpolation value.
**
**    CounterClockwise, RightCenter
**       ARC control flags.
**
** OUTPUT:
**
**    Nothing.
*/

static gceSTATUS _InterpolateArc(
    vgsCONTEXT_PTR Context,
    vgsPATHWALKER_PTR Destination,
    gctFLOAT_PTR StartCoordinates,
    gctFLOAT_PTR EndCoordinates,
    gctFLOAT Amount,
    gctBOOL CounterClockwise,
    gctBOOL Large
    )
{
    gceSTATUS status;
    vgmENTERSUBAPI(_InterpolateArc);

    do
    {
        vgsCONTEXT_PTR context;
        vgsPATHWALKER arc;
        gctFLOAT horRadius;
        gctFLOAT verRadius;
        gctFLOAT rotAngle;
        gctFLOAT endX;
        gctFLOAT endY;

        /* Determine the interpolated coordinates. */
        horRadius = _Interpolate(
            Context, StartCoordinates[0], EndCoordinates[0], Amount
            );

        verRadius = _Interpolate(
            Context, StartCoordinates[1], EndCoordinates[1], Amount
            );

        rotAngle = _Interpolate(
            Context, StartCoordinates[2], EndCoordinates[2], Amount
            );

        endX = _Interpolate(
            Context, StartCoordinates[3], EndCoordinates[3], Amount
            );

        endY = _Interpolate(
            Context, StartCoordinates[4], EndCoordinates[4], Amount
            );

        /* Close the current subpath. */
        vgsPATHWALKER_CloseSubpath(Context, Destination);

        /* Get a shortcut to the context. */
        context = Destination->context;

        /* Prepare for writing the destination. */
        vgsPATHWALKER_InitializeWriter(
            context, context->pathStorage, &arc, Destination->path
            );

        /* Generate the ARC buffer. */
        gcmERR_BREAK(vgfConvertArc(
            Context,
            &arc,
            horRadius, verRadius, rotAngle, endX, endY,
            CounterClockwise, Large, gcvFALSE
            ));

        /* Append to the existing buffer. */
        vgsPATHWALKER_AppendData(
             Context, Destination, &arc, 1, 5
            );

        /* Set the ARC present flag. */
        Destination->path->hasArcs = gcvTRUE;
    }
    while (gcvFALSE);
    vgmLEAVESUBAPI(_InterpolateArc);
    /* Return status. */
    return status;
}


/******************************************************************************\
********************************* Invalid Entry ********************************
\******************************************************************************/

vgmDEFINEINTERPOLATE(Invalid)
{
    vgmENTERSUBAPI(vgmDEFINEINTERPOLATE_Invalid);
    /* This function should never be called. */
    gcmASSERT(gcvFALSE);
    vgmLEAVESUBAPI(vgmDEFINEINTERPOLATE_Invalid);
    return gcvSTATUS_GENERIC_IO;
}


/******************************************************************************\
******************************** gcvVGCMD_CLOSE ********************************
\******************************************************************************/

vgmDEFINEINTERPOLATE(gcvVGCMD_CLOSE)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEINTERPOLATE_gcvVGCMD_CLOSE);
    {
        vgsCONTROL_COORD_PTR coords;

        do
        {
            /* Get a shortcut to the control coordinates. */
            coords = Destination->coords;

            /* Add new command. */
            gcmERR_BREAK(vgsPATHWALKER_WriteCommand(
                Context, Destination, gcvVGCMD_CLOSE
                ));

            /* Update the control coordinates. */
            coords->lastX    = coords->startX;
            coords->lastY    = coords->startY;
            coords->controlX = coords->startX;
            coords->controlY = coords->startY;

            /* Success. */
            status = gcvSTATUS_OK;
        }
        while (gcvFALSE);
    }
    vgmLEAVESUBAPI(vgmDEFINEINTERPOLATE_gcvVGCMD_CLOSE);
    /* Return status. */
    return status;
}


/******************************************************************************\
******************************** gcvVGCMD_MOVE *********************************
\******************************************************************************/

vgmDEFINEINTERPOLATE(gcvVGCMD_MOVE)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEINTERPOLATE_gcvVGCMD_MOVE);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT moveToX, moveToY;

        do
        {
            /* Get a shortcut to the control coordinates. */
            coords = Destination->coords;

            /* Add new command. */
            gcmERR_BREAK(vgsPATHWALKER_WriteCommand(
                Context, Destination, gcvVGCMD_MOVE
                ));

            /* Determine the interpolated coordinates. */
            moveToX = _Interpolate(
                Context, StartCoordinates[0], EndCoordinates[0], Amount
                );

            moveToY = _Interpolate(
                Context, StartCoordinates[1], EndCoordinates[1], Amount
                );

            /* Set the command coordinates. */
            Destination->set(Context, Destination, moveToX);
            Destination->set(Context, Destination, moveToY);

            /* Update the control coordinates. */
            coords->startX   = moveToX;
            coords->startY   = moveToY;
            coords->lastX    = moveToX;
            coords->lastY    = moveToY;
            coords->controlX = moveToX;
            coords->controlY = moveToY;

            /* Success. */
            status = gcvSTATUS_OK;
        }
        while (gcvFALSE);
    }
    vgmLEAVESUBAPI(vgmDEFINEINTERPOLATE_gcvVGCMD_MOVE);
    /* Return status. */
    return status;
}


/******************************************************************************\
******************************** gcvVGCMD_LINE *********************************
\******************************************************************************/

vgmDEFINEINTERPOLATE(gcvVGCMD_LINE)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEINTERPOLATE_gcvVGCMD_LINE);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT lineToX, lineToY;

        do
        {
            /* Get a shortcut to the control coordinates. */
            coords = Destination->coords;

            /* Add new command. */
            gcmERR_BREAK(vgsPATHWALKER_WriteCommand(
                Context, Destination, gcvVGCMD_LINE
                ));

            /* Determine the interpolated coordinates. */
            lineToX = _Interpolate(
                Context, StartCoordinates[0], EndCoordinates[0], Amount
                );

            lineToY = _Interpolate(
                Context, StartCoordinates[1], EndCoordinates[1], Amount
                );

            /* Set the command coordinates. */
            Destination->set(Context, Destination, lineToX);
            Destination->set(Context, Destination, lineToY);

            /* Update the control coordinates. */
            coords->lastX    = lineToX;
            coords->lastY    = lineToY;
            coords->controlX = lineToX;
            coords->controlY = lineToY;

            /* Success. */
            status = gcvSTATUS_OK;
        }
        while (gcvFALSE);
    }
    vgmLEAVESUBAPI(vgmDEFINEINTERPOLATE_gcvVGCMD_LINE);
    /* Return status. */
    return status;
}


/******************************************************************************\
******************************** gcvVGCMD_CUBIC ********************************
\******************************************************************************/

vgmDEFINEINTERPOLATE(gcvVGCMD_CUBIC)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEINTERPOLATE_gcvVGCMD_CUBIC);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT controlX1, controlY1;
        gctFLOAT controlX2, controlY2;
        gctFLOAT cubicToX, cubicToY;

        do
        {
            /* Get a shortcut to the control coordinates. */
            coords = Destination->coords;

            /* Add new command. */
            gcmERR_BREAK(vgsPATHWALKER_WriteCommand(
                Context, Destination, gcvVGCMD_CUBIC
                ));

            /* Determine the interpolated coordinates. */
            controlX1 = _Interpolate(
                Context, StartCoordinates[0], EndCoordinates[0], Amount
                );

            controlY1 = _Interpolate(
                Context, StartCoordinates[1], EndCoordinates[1], Amount
                );

            controlX2 = _Interpolate(
                Context, StartCoordinates[2], EndCoordinates[2], Amount
                );

            controlY2 = _Interpolate(
                Context, StartCoordinates[3], EndCoordinates[3], Amount
                );

            cubicToX = _Interpolate(
                Context, StartCoordinates[4], EndCoordinates[4], Amount
                );

            cubicToY = _Interpolate(
                Context, StartCoordinates[5], EndCoordinates[5], Amount
                );

            /* Set the command coordinates. */
            Destination->set(Context, Destination, controlX1);
            Destination->set(Context, Destination, controlY1);
            Destination->set(Context, Destination, controlX2);
            Destination->set(Context, Destination, controlY2);
            Destination->set(Context, Destination, cubicToX);
            Destination->set(Context, Destination, cubicToY);

            /* Update the control coordinates. */
            coords->lastX    = cubicToX;
            coords->lastY    = cubicToY;
            coords->controlX = controlX2;
            coords->controlY = controlY2;

            /* Success. */
            status = gcvSTATUS_OK;
        }
        while (gcvFALSE);
    }
    vgmLEAVESUBAPI(vgmDEFINEINTERPOLATE_gcvVGCMD_CUBIC);
    /* Return status. */
    return status;
}


/******************************************************************************\
******************************* gcvVGCMD_SCCWARC *******************************
\******************************************************************************/

vgmDEFINEINTERPOLATE(gcvVGCMD_SCCWARC)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEINTERPOLATE_gcvVGCMD_SCCWARC);
    /* Generate the ARC. */
    status =_InterpolateArc(
        Context,
        Destination,
        StartCoordinates,
        EndCoordinates,
        Amount,
        gcvTRUE,
        gcvFALSE
        );
    vgmLEAVESUBAPI(vgmDEFINEINTERPOLATE_gcvVGCMD_SCCWARC);
    return status;
}


/******************************************************************************\
******************************** gcvVGCMD_SCWARC *******************************
\******************************************************************************/

vgmDEFINEINTERPOLATE(gcvVGCMD_SCWARC)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEINTERPOLATE_gcvVGCMD_SCWARC);
    /* Generate the ARC. */
    status = _InterpolateArc(
        Context,
        Destination,
        StartCoordinates,
        EndCoordinates,
        Amount,
        gcvFALSE,
        gcvFALSE
        );
    vgmLEAVESUBAPI(vgmDEFINEINTERPOLATE_gcvVGCMD_SCWARC);
    return status;
}


/******************************************************************************\
******************************* gcvVGCMD_LCCWARC *******************************
\******************************************************************************/

vgmDEFINEINTERPOLATE(gcvVGCMD_LCCWARC)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEINTERPOLATE_gcvVGCMD_LCCWARC);
    /* Generate the ARC. */
    status = _InterpolateArc(
        Context,
        Destination,
        StartCoordinates,
        EndCoordinates,
        Amount,
        gcvTRUE,
        gcvTRUE
        );
    vgmLEAVESUBAPI(vgmDEFINEINTERPOLATE_gcvVGCMD_LCCWARC);
    return status;
}


/******************************************************************************\
******************************** gcvVGCMD_LCWARC *******************************
\******************************************************************************/

vgmDEFINEINTERPOLATE(gcvVGCMD_LCWARC)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEINTERPOLATE_gcvVGCMD_LCWARC);
    /* Generate the ARC. */
    status = _InterpolateArc(
        Context,
        Destination,
        StartCoordinates,
        EndCoordinates,
        Amount,
        gcvFALSE,
        gcvTRUE
        );
    vgmLEAVESUBAPI(vgmDEFINEINTERPOLATE_gcvVGCMD_LCWARC);
    return status;
}


/*******************************************************************************
**
** vgfGetInterpolationArray
**
** Returns the pointer to the array of coordinate interpolation functions.
**
** INPUT:
**
**    Nothing.
**
** OUTPUT:
**
**    Array
**       Pointer to the function array.
**
**    Count
**       Pointer to the number of functions in the array.
*/

void vgfGetInterpolationArray(
    IN vgsCONTEXT_PTR Context,
    vgtINTERPOLATEFUNCTION ** Array,
    gctUINT_PTR Count
    )
{
    vgmENTERSUBAPI(vgfGetInterpolationArray);
    {
        static vgtINTERPOLATEFUNCTION _interpolateSegment[] =
        {
            vgmINTERPOL(Invalid),                  /*   0: gcvVGCMD_END             */
            vgmINTERPOL(gcvVGCMD_CLOSE),           /*   1: gcvVGCMD_CLOSE           */
            vgmINTERPOL(gcvVGCMD_MOVE),            /*   2: gcvVGCMD_MOVE            */
            vgmINTERPOL(Invalid),                  /*   3: gcvVGCMD_MOVE_REL        */
            vgmINTERPOL(gcvVGCMD_LINE),            /*   4: gcvVGCMD_LINE            */
            vgmINTERPOL(Invalid),                  /*   5: gcvVGCMD_LINE_REL        */
            vgmINTERPOL(Invalid),                  /*   6: gcvVGCMD_QUAD            */
            vgmINTERPOL(Invalid),                  /*   7: gcvVGCMD_QUAD_REL        */
            vgmINTERPOL(gcvVGCMD_CUBIC),           /*   8: gcvVGCMD_CUBIC           */
            vgmINTERPOL(Invalid),                  /*   9: gcvVGCMD_CUBIC_REL       */
            vgmINTERPOL(Invalid),                  /*  10: gcvVGCMD_BREAK           */
            vgmINTERPOL(Invalid),                  /*  11: **** R E S E R V E D *****/
            vgmINTERPOL(Invalid),                  /*  12: **** R E S E R V E D *****/
            vgmINTERPOL(Invalid),                  /*  13: **** R E S E R V E D *****/
            vgmINTERPOL(Invalid),                  /*  14: **** R E S E R V E D *****/
            vgmINTERPOL(Invalid),                  /*  15: **** R E S E R V E D *****/
            vgmINTERPOL(Invalid),                  /*  16: **** R E S E R V E D *****/
            vgmINTERPOL(Invalid),                  /*  17: **** R E S E R V E D *****/
            vgmINTERPOL(Invalid),                  /*  18: **** R E S E R V E D *****/
            vgmINTERPOL(gcvVGCMD_SCCWARC),         /*  19: gcvVGCMD_SCCWARC         */
            vgmINTERPOL(Invalid),                  /*  20: **** R E S E R V E D *****/
            vgmINTERPOL(gcvVGCMD_SCWARC),          /*  21: gcvVGCMD_SCWARC          */
            vgmINTERPOL(Invalid),                  /*  22: **** R E S E R V E D *****/
            vgmINTERPOL(gcvVGCMD_LCCWARC),         /*  23: gcvVGCMD_LCCWARC         */
            vgmINTERPOL(Invalid),                  /*  24: **** R E S E R V E D *****/
            vgmINTERPOL(gcvVGCMD_LCWARC),          /*  25: gcvVGCMD_LCWARC          */
        };

        * Array = _interpolateSegment;
        * Count = gcmCOUNTOF(_interpolateSegment);
    }
    vgmLEAVESUBAPI(vgfGetInterpolationArray);
}
