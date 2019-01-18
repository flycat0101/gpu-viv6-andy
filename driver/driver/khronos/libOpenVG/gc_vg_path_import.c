/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
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

/* Segment import function name. */
#define vgmIMPORTSEGMENT(Command) \
    _Import_ ## Command

/* Segment import function definition. */
#define vgmDEFINEIMPORT(Command) \
    static gceSTATUS vgmIMPORTSEGMENT(Command) ( \
        vgmSEGMENTHANDLERPARAMETERS \
        )


/*******************************************************************************
**
** _ImportArc
**
** Generate an ARC buffer and append it to the specified destination.
**
** INPUT:
**
**    Destination
**       Pointer to the destination path writer.
**
**    Source
**       Pointer to the import data reader.
**
**    CounterClockwise, RightCenter, Relative
**       ARC control flags.
**
** OUTPUT:
**
**    Nothing.
*/

static gceSTATUS _ImportArc(
    vgsCONTEXT_PTR Context,
    vgsPATHWALKER_PTR Destination,
    vgsPATHWALKER_PTR Source,
    gctBOOL CounterClockwise,
    gctBOOL Large,
    gctBOOL Relative
    )
{
    gceSTATUS status;
    vgmENTERSUBAPI(_ImportArc);
    do
    {
        vgsCONTEXT_PTR context;
        vgsPATHWALKER arc;
        gctFLOAT horRadius;
        gctFLOAT verRadius;
        gctFLOAT rotAngle;
        gctFLOAT endX;
        gctFLOAT endY;

        /* Get the input coordinates. */
        horRadius = Source->get(Context, Source);
        verRadius = Source->get(Context, Source);
        rotAngle  = Source->get(Context, Source);
        endX      = Source->get(Context, Source);
        endY      = Source->get(Context, Source);

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
            CounterClockwise, Large, Relative
            ));

        /* Append to the existing buffer. */
        vgsPATHWALKER_AppendData(
            Context, Destination, &arc, 1, 5
            );

        /* Set the ARC present flag. */
        Destination->path->hasArcs = gcvTRUE;
    }
    while (gcvFALSE);

    vgmLEAVESUBAPI(_ImportArc);
    /* Return status. */
    return status;
}


/******************************************************************************\
******************************** VG_CLOSE_PATH *********************************
\******************************************************************************/

vgmDEFINEIMPORT(VG_CLOSE_PATH)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEIMPORT_VG_CLOSE_PATH);
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
    vgmLEAVESUBAPI(vgmDEFINEIMPORT_VG_CLOSE_PATH);
    /* Return status. */
    return status;
}


/******************************************************************************\
********************************** VG_MOVE_TO **********************************
\******************************************************************************/

vgmDEFINEIMPORT(VG_MOVE_TO_ABS)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEIMPORT_VG_MOVE_TO_ABS);
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

            /* Extract/copy the command coordinates. */
            moveToX = Source->getcopy(Context, Source, Destination);
            moveToY = Source->getcopy(Context, Source, Destination);

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
    vgmLEAVESUBAPI(vgmDEFINEIMPORT_VG_MOVE_TO_ABS);
    /* Return status. */
    return status;
}

vgmDEFINEIMPORT(VG_MOVE_TO_REL)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEIMPORT_VG_MOVE_TO_REL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT moveToX, moveToY;

        do
        {
            /* Get a shortcut to the control coordinates. */
            coords = Destination->coords;

            /* Add new command. */
            gcmERR_BREAK(vgsPATHWALKER_WriteCommand(
                Context, Destination, gcvVGCMD_MOVE_REL
                ));

            /* Extract/copy the command coordinates. */
            moveToX = Source->getcopy(Context, Source, Destination);
            moveToY = Source->getcopy(Context, Source, Destination);

            /* Determine the absolute coordinates. */
            moveToX += coords->lastX;
            moveToY += coords->lastY;

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
    vgmLEAVESUBAPI(vgmDEFINEIMPORT_VG_MOVE_TO_REL);
    /* Return status. */
    return status;
}


/******************************************************************************\
********************************** VG_LINE_TO **********************************
\******************************************************************************/

vgmDEFINEIMPORT(VG_LINE_TO_ABS)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEIMPORT_VG_LINE_TO_ABS);
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

            /* Extract/copy the command coordinates. */
            lineToX = Source->getcopy(Context, Source, Destination);
            lineToY = Source->getcopy(Context, Source, Destination);

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
    vgmLEAVESUBAPI(vgmDEFINEIMPORT_VG_LINE_TO_ABS);
    /* Return status. */
    return status;
}

vgmDEFINEIMPORT(VG_LINE_TO_REL)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEIMPORT_VG_LINE_TO_REL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT lineToX, lineToY;

        do
        {
            /* Get a shortcut to the control coordinates. */
            coords = Destination->coords;

            /* Add new command. */
            gcmERR_BREAK(vgsPATHWALKER_WriteCommand(
                Context, Destination, gcvVGCMD_LINE_REL
                ));

            /* Extract/copy the command coordinates. */
            lineToX = Source->getcopy(Context, Source, Destination);
            lineToY = Source->getcopy(Context, Source, Destination);

            /* Determine the absolute coordinates. */
            lineToX += coords->lastX;
            lineToY += coords->lastY;

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
    vgmLEAVESUBAPI(vgmDEFINEIMPORT_VG_LINE_TO_REL);
    /* Return status. */
    return status;
}


/******************************************************************************\
********************************* VG_HLINE_TO **********************************
\******************************************************************************/

vgmDEFINEIMPORT(VG_HLINE_TO_ABS)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEIMPORT_VG_HLINE_TO_ABS);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT lineToX;

        do
        {
            /* Get a shortcut to the control coordinates. */
            coords = Destination->coords;

            /* Add new command. */
            gcmERR_BREAK(vgsPATHWALKER_WriteCommand(
                Context, Destination, gcvVGCMD_HLINE_EMUL
                ));

            /* Extract/copy the command coordinate. */
            lineToX = Source->getcopy(Context, Source, Destination);

            /* Set the vertical coordinate. */
            Destination->set(Context, Destination, coords->lastY);

            /* Update the control coordinates. */
            coords->lastX    = lineToX;
            coords->controlX = lineToX;
            coords->controlY = coords->lastY;

            /* Success. */
            status = gcvSTATUS_OK;
        }
        while (gcvFALSE);
    }
    vgmLEAVESUBAPI(vgmDEFINEIMPORT_VG_HLINE_TO_ABS);
    /* Return status. */
    return status;
}

vgmDEFINEIMPORT(VG_HLINE_TO_REL)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEIMPORT_VG_HLINE_TO_REL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT lineToX;

        do
        {
            /* Get a shortcut to the control coordinates. */
            coords = Destination->coords;

            /* Add new command. */
            gcmERR_BREAK(vgsPATHWALKER_WriteCommand(
                Context, Destination, gcvVGCMD_HLINE_EMUL_REL
                ));

            /* Extract/copy the command coordinate. */
            lineToX = Source->getcopy(Context, Source, Destination);

            /* Set the vertical coordinate to zero. */
            Destination->set(Context, Destination, 0.0f);

            /* Determine the absolute coordinates. */
            lineToX += coords->lastX;

            /* Update the control coordinates. */
            coords->lastX    = lineToX;
            coords->controlX = lineToX;
            coords->controlY = coords->lastY;

            /* Success. */
            status = gcvSTATUS_OK;
        }
        while (gcvFALSE);
    }
    vgmLEAVESUBAPI(vgmDEFINEIMPORT_VG_HLINE_TO_REL);
    /* Return status. */
    return status;
}


/******************************************************************************\
********************************** VG_VLINE_TO *********************************
\******************************************************************************/

vgmDEFINEIMPORT(VG_VLINE_TO_ABS)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEIMPORT_VG_VLINE_TO_ABS);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT lineToY;

        do
        {
            /* Get a shortcut to the control coordinates. */
            coords = Destination->coords;

            /* Add new command. */
            gcmERR_BREAK(vgsPATHWALKER_WriteCommand(
                Context, Destination, gcvVGCMD_VLINE_EMUL
                ));

            /* Set the horizontal coordinate. */
            Destination->set(Context, Destination, coords->lastX);

            /* Extract/copy the command coordinate. */
            lineToY = Source->getcopy(Context, Source, Destination);

            /* Update the control coordinates. */
            coords->lastY    = lineToY;
            coords->controlX = coords->lastX;
            coords->controlY = lineToY;

            /* Success. */
            status = gcvSTATUS_OK;
        }
        while (gcvFALSE);
    }
    vgmLEAVESUBAPI(vgmDEFINEIMPORT_VG_VLINE_TO_ABS);
    /* Return status. */
    return status;
}

vgmDEFINEIMPORT(VG_VLINE_TO_REL)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEIMPORT_VG_VLINE_TO_REL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT lineToY;

        do
        {
            /* Get a shortcut to the control coordinates. */
            coords = Destination->coords;

            /* Add new command. */
            gcmERR_BREAK(vgsPATHWALKER_WriteCommand(
                Context, Destination, gcvVGCMD_VLINE_EMUL_REL
                ));

            /* Set the horizontal coordinate to zero. */
            Destination->set(Context, Destination, 0.0f);

            /* Extract/copy the command coordinate. */
            lineToY = Source->getcopy(Context, Source, Destination);

            /* Determine the absolute coordinates. */
            lineToY += coords->lastY;

            /* Update the control coordinates. */
            coords->lastY    = lineToY;
            coords->controlX = coords->lastX;
            coords->controlY = lineToY;

            /* Success. */
            status = gcvSTATUS_OK;
        }
        while (gcvFALSE);
    }
    vgmLEAVESUBAPI(vgmDEFINEIMPORT_VG_VLINE_TO_REL);
    /* Return status. */
    return status;
}


/******************************************************************************\
*********************************** VG_QUAD_TO *********************************
\******************************************************************************/

vgmDEFINEIMPORT(VG_QUAD_TO_ABS)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEIMPORT_VG_QUAD_TO_ABS);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT controlX, controlY;
        gctFLOAT quadToX, quadToY;

        do
        {
            /* Get a shortcut to the control coordinates. */
            coords = Destination->coords;

            /* Add new command. */
            gcmERR_BREAK(vgsPATHWALKER_WriteCommand(
                Context, Destination, gcvVGCMD_QUAD
                ));

            /* Extract/copy the command coordinates. */
            controlX = Source->getcopy(Context, Source, Destination);
            controlY = Source->getcopy(Context, Source, Destination);
            quadToX  = Source->getcopy(Context, Source, Destination);
            quadToY  = Source->getcopy(Context, Source, Destination);

            /* Update the control coordinates. */
            coords->lastX    = quadToX;
            coords->lastY    = quadToY;
            coords->controlX = controlX;
            coords->controlY = controlY;

            /* Success. */
            status = gcvSTATUS_OK;
        }
        while (gcvFALSE);
    }
    vgmLEAVESUBAPI(vgmDEFINEIMPORT_VG_QUAD_TO_ABS);
    /* Return status. */
    return status;
}

vgmDEFINEIMPORT(VG_QUAD_TO_REL)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEIMPORT_VG_QUAD_TO_REL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT controlX, controlY;
        gctFLOAT quadToX, quadToY;

        do
        {
            /* Get a shortcut to the control coordinates. */
            coords = Destination->coords;

            /* Add new command. */
            gcmERR_BREAK(vgsPATHWALKER_WriteCommand(
                Context, Destination, gcvVGCMD_QUAD_REL
                ));

            /* Extract/copy the command coordinates. */
            controlX = Source->getcopy(Context, Source, Destination);
            controlY = Source->getcopy(Context, Source, Destination);
            quadToX  = Source->getcopy(Context, Source, Destination);
            quadToY  = Source->getcopy(Context, Source, Destination);

            /* Determine the absolute coordinates. */
            controlX += coords->lastX;
            controlY += coords->lastY;
            quadToX  += coords->lastX;
            quadToY  += coords->lastY;

            /* Update the last coordinates. */
            coords->lastX    = quadToX;
            coords->lastY    = quadToY;
            coords->controlX = controlX;
            coords->controlY = controlY;

            /* Success. */
            status = gcvSTATUS_OK;
        }
        while (gcvFALSE);
    }
    vgmLEAVESUBAPI(vgmDEFINEIMPORT_VG_QUAD_TO_REL);
    /* Return status. */
    return status;
}


/******************************************************************************\
********************************** VG_CUBIC_TO *********************************
\******************************************************************************/

vgmDEFINEIMPORT(VG_CUBIC_TO_ABS)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEIMPORT_VG_CUBIC_TO_ABS);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT controlX, controlY;
        gctFLOAT cubicToX, cubicToY;

        do
        {
            /* Get a shortcut to the control coordinates. */
            coords = Destination->coords;

            /* Add new command. */
            gcmERR_BREAK(vgsPATHWALKER_WriteCommand(
                Context, Destination, gcvVGCMD_CUBIC
                ));

            /* Copy the first control points. */
            Source->copy(Context, Source, Destination);
            Source->copy(Context, Source, Destination);

            /* Extract/copy the command coordinates. */
            controlX = Source->getcopy(Context, Source, Destination);
            controlY = Source->getcopy(Context, Source, Destination);
            cubicToX = Source->getcopy(Context, Source, Destination);
            cubicToY = Source->getcopy(Context, Source, Destination);

            /* Update the control coordinates. */
            coords->lastX    = cubicToX;
            coords->lastY    = cubicToY;
            coords->controlX = controlX;
            coords->controlY = controlY;

            /* Success. */
            status = gcvSTATUS_OK;
        }
        while (gcvFALSE);
    }
    vgmLEAVESUBAPI(vgmDEFINEIMPORT_VG_CUBIC_TO_ABS);
    /* Return status. */
    return status;
}

vgmDEFINEIMPORT(VG_CUBIC_TO_REL)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEIMPORT_VG_CUBIC_TO_REL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT controlX, controlY;
        gctFLOAT cubicToX, cubicToY;

        do
        {
            /* Get a shortcut to the control coordinates. */
            coords = Destination->coords;

            /* Add new command. */
            gcmERR_BREAK(vgsPATHWALKER_WriteCommand(
                Context, Destination, gcvVGCMD_CUBIC_REL
                ));

            /* Copy the first control points. */
            Source->copy(Context, Source, Destination);
            Source->copy(Context, Source, Destination);

            /* Extract/copy the command coordinates. */
            controlX = Source->getcopy(Context, Source, Destination);
            controlY = Source->getcopy(Context, Source, Destination);
            cubicToX = Source->getcopy(Context, Source, Destination);
            cubicToY = Source->getcopy(Context, Source, Destination);

            /* Determine the absolute coordinates. */
            controlX += coords->lastX;
            controlY += coords->lastY;
            cubicToX += coords->lastX;
            cubicToY += coords->lastY;

            /* Update the control coordinates. */
            coords->lastX    = cubicToX;
            coords->lastY    = cubicToY;
            coords->controlX = controlX;
            coords->controlY = controlY;

            /* Success. */
            status = gcvSTATUS_OK;
        }
        while (gcvFALSE);
    }
    vgmLEAVESUBAPI(vgmDEFINEIMPORT_VG_CUBIC_TO_REL);
    /* Return status. */
    return status;
}


/******************************************************************************\
********************************** VG_SQUAD_TO *********************************
\******************************************************************************/

vgmDEFINEIMPORT(VG_SQUAD_TO_ABS)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEIMPORT_VG_SQUAD_TO_ABS);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT controlX, controlY;
        gctFLOAT quadToX, quadToY;

        do
        {
            /* Get a shortcut to the control coordinates. */
            coords = Destination->coords;

            /* Compute the control point. */
            controlX = 2.0f * coords->lastX - coords->controlX;
            controlY = 2.0f * coords->lastY - coords->controlY;

            /* Add new command. */
            gcmERR_BREAK(vgsPATHWALKER_WriteCommand(
                Context, Destination, gcvVGCMD_SQUAD_EMUL
                ));

            /* Store the control point. */
            Destination->set(Context, Destination, controlX);
            Destination->set(Context, Destination, controlY);

            /* Extract/copy the command coordinates. */
            quadToX = Source->getcopy(Context, Source, Destination);
            quadToY = Source->getcopy(Context, Source, Destination);

            /* Update the control coordinates. */
            coords->lastX    = quadToX;
            coords->lastY    = quadToY;
            coords->controlX = controlX;
            coords->controlY = controlY;

            /* Success. */
            status = gcvSTATUS_OK;
        }
        while (gcvFALSE);
    }
    vgmLEAVESUBAPI(vgmDEFINEIMPORT_VG_SQUAD_TO_ABS);
    /* Return status. */
    return status;
}

vgmDEFINEIMPORT(VG_SQUAD_TO_REL)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEIMPORT_VG_SQUAD_TO_REL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT controlX, controlY;
        gctFLOAT quadToX, quadToY;

        do
        {
            /* Get a shortcut to the control coordinates. */
            coords = Destination->coords;

            /* Compute the relative control point. */
            controlX = coords->lastX - coords->controlX;
            controlY = coords->lastY - coords->controlY;

            /* Add new command. */
            gcmERR_BREAK(vgsPATHWALKER_WriteCommand(
                Context,Destination, gcvVGCMD_SQUAD_EMUL_REL
                ));

            /* Store the control point. */
            Destination->set(Context, Destination, controlX);
            Destination->set(Context, Destination, controlY);

            /* Extract/copy the command coordinates. */
            quadToX = Source->getcopy(Context, Source, Destination);
            quadToY = Source->getcopy(Context, Source, Destination);

            /* Determine the absolute coordinates. */
            controlX += coords->lastX;
            controlY += coords->lastY;
            quadToX  += coords->lastX;
            quadToY  += coords->lastY;

            /* Update the control coordinates. */
            coords->lastX    = quadToX;
            coords->lastY    = quadToY;
            coords->controlX = controlX;
            coords->controlY = controlY;

            /* Success. */
            status = gcvSTATUS_OK;
        }
        while (gcvFALSE);
    }
    vgmLEAVESUBAPI(vgmDEFINEIMPORT_VG_SQUAD_TO_REL);
    /* Return status. */
    return status;
}


/******************************************************************************\
********************************* VG_SCUBIC_TO *********************************
\******************************************************************************/

vgmDEFINEIMPORT(VG_SCUBIC_TO_ABS)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEIMPORT_VG_SCUBIC_TO_ABS);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT controlX1, controlY1;
        gctFLOAT controlX2, controlY2;
        gctFLOAT cubicToX, cubicToY;

        do
        {
            /* Get a shortcut to the control coordinates. */
            coords = Destination->coords;

            /* Compute the control point. */
            controlX1 = 2.0f * coords->lastX - coords->controlX;
            controlY1 = 2.0f * coords->lastY - coords->controlY;

            /* Add new command. */
            gcmERR_BREAK(vgsPATHWALKER_WriteCommand(
                Context, Destination, gcvVGCMD_SCUBIC_EMUL
                ));

            /* Store the control point. */
            Destination->set(Context, Destination, controlX1);
            Destination->set(Context, Destination, controlY1);

            /* Extract/copy the command coordinates. */
            controlX2 = Source->getcopy(Context, Source, Destination);
            controlY2 = Source->getcopy(Context, Source, Destination);
            cubicToX  = Source->getcopy(Context, Source, Destination);
            cubicToY  = Source->getcopy(Context, Source, Destination);

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
    vgmLEAVESUBAPI(vgmDEFINEIMPORT_VG_SCUBIC_TO_ABS);
    /* Return status. */
    return status;
}

vgmDEFINEIMPORT(VG_SCUBIC_TO_REL)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEIMPORT_VG_SCUBIC_TO_REL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT controlX1, controlY1;
        gctFLOAT controlX2, controlY2;
        gctFLOAT cubicToX, cubicToY;

        do
        {
            /* Get a shortcut to the control coordinates. */
            coords = Destination->coords;

            /* Compute the relative control point. */
            controlX1 = coords->lastX - coords->controlX;
            controlY1 = coords->lastY - coords->controlY;

            /* Add new command. */
            gcmERR_BREAK(vgsPATHWALKER_WriteCommand(
                Context, Destination, gcvVGCMD_SCUBIC_EMUL_REL
                ));

            /* Store the control point. */
            Destination->set(Context, Destination, controlX1);
            Destination->set(Context, Destination, controlY1);

            /* Extract/copy the command coordinates. */
            controlX2 = Source->getcopy(Context, Source, Destination);
            controlY2 = Source->getcopy(Context, Source, Destination);
            cubicToX  = Source->getcopy(Context, Source, Destination);
            cubicToY  = Source->getcopy(Context, Source, Destination);

            /* Determine the absolute coordinates. */
            controlX2 += coords->lastX;
            controlY2 += coords->lastY;
            cubicToX  += coords->lastX;
            cubicToY  += coords->lastY;

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
    vgmLEAVESUBAPI(vgmDEFINEIMPORT_VG_SCUBIC_TO_REL);
    /* Return status. */
    return status;
}


/******************************************************************************\
********************************* VG_SCCWARC_TO ********************************
\******************************************************************************/

vgmDEFINEIMPORT(VG_SCCWARC_TO_ABS)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEIMPORT_VG_SCCWARC_TO_ABS);
    status = _ImportArc(Context, Destination, Source, gcvTRUE, gcvFALSE, gcvFALSE);
    vgmLEAVESUBAPI(vgmDEFINEIMPORT_VG_SCCWARC_TO_ABS);
    return status;
}

vgmDEFINEIMPORT(VG_SCCWARC_TO_REL)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEIMPORT_VG_SCCWARC_TO_REL);
    status = _ImportArc(Context, Destination, Source, gcvTRUE, gcvFALSE, gcvTRUE);
    vgmLEAVESUBAPI(vgmDEFINEIMPORT_VG_SCCWARC_TO_REL);
    return status;
}


/******************************************************************************\
********************************* VG_SCWARC_TO *********************************
\******************************************************************************/

vgmDEFINEIMPORT(VG_SCWARC_TO_ABS)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEIMPORT_VG_SCWARC_TO_ABS);
    status = _ImportArc(Context, Destination, Source, gcvFALSE, gcvFALSE, gcvFALSE);
    vgmLEAVESUBAPI(vgmDEFINEIMPORT_VG_SCWARC_TO_ABS);
    return status;
}

vgmDEFINEIMPORT(VG_SCWARC_TO_REL)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEIMPORT_VG_SCWARC_TO_REL);
    status = _ImportArc(Context, Destination, Source, gcvFALSE, gcvFALSE, gcvTRUE);
    vgmLEAVESUBAPI(vgmDEFINEIMPORT_VG_SCWARC_TO_REL);
    return status;
}


/******************************************************************************\
********************************* VG_LCCWARC_TO ********************************
\******************************************************************************/

vgmDEFINEIMPORT(VG_LCCWARC_TO_ABS)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEIMPORT_VG_LCCWARC_TO_ABS);
    status = _ImportArc(Context, Destination, Source, gcvTRUE, gcvTRUE, gcvFALSE);
    vgmLEAVESUBAPI(vgmDEFINEIMPORT_VG_LCCWARC_TO_ABS);
    return status;
}

vgmDEFINEIMPORT(VG_LCCWARC_TO_REL)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEIMPORT_VG_LCCWARC_TO_REL);
    status = _ImportArc(Context, Destination, Source, gcvTRUE, gcvTRUE, gcvTRUE);
    vgmLEAVESUBAPI(vgmDEFINEIMPORT_VG_LCCWARC_TO_REL);
    return status;
}


/******************************************************************************\
********************************** VG_LCWARC_TO ********************************
\******************************************************************************/

vgmDEFINEIMPORT(VG_LCWARC_TO_ABS)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEIMPORT_VG_LCWARC_TO_ABS);
    status = _ImportArc(Context, Destination, Source, gcvFALSE, gcvTRUE, gcvFALSE);
    vgmLEAVESUBAPI(vgmDEFINEIMPORT_VG_LCWARC_TO_ABS);
    return status;
}

vgmDEFINEIMPORT(VG_LCWARC_TO_REL)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEIMPORT_VG_LCWARC_TO_REL);
    status = _ImportArc(Context, Destination, Source, gcvFALSE, gcvTRUE, gcvTRUE);
    vgmLEAVESUBAPI(vgmDEFINEIMPORT_VG_LCWARC_TO_REL);
    return status;
}


/*******************************************************************************
**
** vgfGetImportArray
**
** Returns the pointer to the array of import functions.
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

void vgfGetImportArray(
    vgsCONTEXT_PTR Context,
    vgtSEGMENTHANDLER ** Array,
    gctUINT_PTR Count
    )
{
    vgmENTERSUBAPI(vgfGetImportArray);
    {
        static vgtSEGMENTHANDLER _importCommand[] =
        {
            vgmIMPORTSEGMENT(VG_CLOSE_PATH),
            vgmIMPORTSEGMENT(VG_CLOSE_PATH),
            vgmIMPORTSEGMENT(VG_MOVE_TO_ABS),
            vgmIMPORTSEGMENT(VG_MOVE_TO_REL),
            vgmIMPORTSEGMENT(VG_LINE_TO_ABS),
            vgmIMPORTSEGMENT(VG_LINE_TO_REL),
            vgmIMPORTSEGMENT(VG_HLINE_TO_ABS),
            vgmIMPORTSEGMENT(VG_HLINE_TO_REL),
            vgmIMPORTSEGMENT(VG_VLINE_TO_ABS),
            vgmIMPORTSEGMENT(VG_VLINE_TO_REL),
            vgmIMPORTSEGMENT(VG_QUAD_TO_ABS),
            vgmIMPORTSEGMENT(VG_QUAD_TO_REL),
            vgmIMPORTSEGMENT(VG_CUBIC_TO_ABS),
            vgmIMPORTSEGMENT(VG_CUBIC_TO_REL),
            vgmIMPORTSEGMENT(VG_SQUAD_TO_ABS),
            vgmIMPORTSEGMENT(VG_SQUAD_TO_REL),
            vgmIMPORTSEGMENT(VG_SCUBIC_TO_ABS),
            vgmIMPORTSEGMENT(VG_SCUBIC_TO_REL),
            vgmIMPORTSEGMENT(VG_SCCWARC_TO_ABS),
            vgmIMPORTSEGMENT(VG_SCCWARC_TO_REL),
            vgmIMPORTSEGMENT(VG_SCWARC_TO_ABS),
            vgmIMPORTSEGMENT(VG_SCWARC_TO_REL),
            vgmIMPORTSEGMENT(VG_LCCWARC_TO_ABS),
            vgmIMPORTSEGMENT(VG_LCCWARC_TO_REL),
            vgmIMPORTSEGMENT(VG_LCWARC_TO_ABS),
            vgmIMPORTSEGMENT(VG_LCWARC_TO_REL)
        };

        * Array = _importCommand;
        * Count = gcmCOUNTOF(_importCommand);
    }
    vgmLEAVESUBAPI(vgfGetImportArray);
}
