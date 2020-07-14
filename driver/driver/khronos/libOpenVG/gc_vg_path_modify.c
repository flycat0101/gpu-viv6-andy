/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_vg_precomp.h"

#define _GC_OBJ_ZONE                gcdZONE_VG_PATH

/******************************************************************************\
*********************** Support Functions and Definitions **********************
\******************************************************************************/

/* Segment skipping function names. */
#define vgmSHARED(Command) \
    _Common_ ## Command

/* Segment skipping function names. */
#define vgmSKIP(Command) \
    _Skip_ ## Command

/* Segment modification function names. */
#define vgmMODIFY(Command) \
    _Modify_ ## Command

/* Segment update function names. */
#define vgmUPDATE(Command) \
    _Update_ ## Command

/* Segment modification function definition. */
#define vgmDEFINESHARED(Command) \
    static gceSTATUS vgmSHARED(Command) ( \
        vgmSEGMENTHANDLERPARAMETERS \
        )

/* Segment modification function definition. */
#define vgmDEFINESKIP(Command) \
    static gceSTATUS vgmSKIP(Command) ( \
        vgmSEGMENTHANDLERPARAMETERS \
        )

/* Segment modification function definition. */
#define vgmDEFINEMODIFY(Command) \
    static gceSTATUS vgmMODIFY(Command) ( \
        vgmSEGMENTHANDLERPARAMETERS \
        )

/* Segment update function definition. */
#define vgmDEFINEUPDATE(Command) \
    static gceSTATUS vgmUPDATE(Command) ( \
        vgmSEGMENTHANDLERPARAMETERS \
        )


/******************************************************************************\
********************************* Invalid Entry ********************************
\******************************************************************************/

vgmDEFINESHARED(Invalid)
{
    vgmENTERSUBAPI(vgmDEFINESHARED_Invalid);
    /* This function should never be called. */
    gcmASSERT(gcvFALSE);
    vgmLEAVESUBAPI(vgmDEFINESHARED_Invalid);
    return gcvSTATUS_GENERIC_IO;
}


/******************************************************************************\
********************************* gcvVGCMD_CLOSE ********************************
\******************************************************************************/

vgmDEFINESHARED(gcvVGCMD_CLOSE)
{
    vgmENTERSUBAPI(vgmDEFINESHARED_gcvVGCMD_CLOSE);
    {
        /* Get a shortcut to the control coordinates. */
        vgsCONTROL_COORD_PTR coords = Destination->coords;

        /* Update the control coordinates. */
        coords->lastX    = coords->startX;
        coords->lastY    = coords->startY;
        coords->controlX = coords->startX;
        coords->controlY = coords->startY;
    }
    vgmLEAVESUBAPI(vgmDEFINESHARED_gcvVGCMD_CLOSE);
    /* Success. */
    return gcvSTATUS_OK;
}


/******************************************************************************\
********************************* gcvVGCMD_MOVE ********************************
\******************************************************************************/

vgmDEFINESKIP(gcvVGCMD_MOVE)
{
    vgmENTERSUBAPI(vgmDEFINESKIP_gcvVGCMD_MOVE);
    {
        /* Get a shortcut to the control coordinates. */
        vgsCONTROL_COORD_PTR coords = Destination->coords;

        /* Extract the command coordinates. */
        gctFLOAT moveToX = Destination->get(Context, Destination);
        gctFLOAT moveToY = Destination->get(Context, Destination);

        /* Update the control coordinates. */
        coords->startX   = moveToX;
        coords->startY   = moveToY;
        coords->lastX    = moveToX;
        coords->lastY    = moveToY;
        coords->controlX = moveToX;
        coords->controlY = moveToY;
    }
    vgmLEAVESUBAPI(vgmDEFINESKIP_gcvVGCMD_MOVE);
    /* Success. */
    return gcvSTATUS_OK;
}

vgmDEFINESHARED(gcvVGCMD_MOVE_REL)
{
    vgmENTERSUBAPI(vgmDEFINESHARED_gcvVGCMD_MOVE_REL);
    {
        /* Get a shortcut to the control coordinates. */
        vgsCONTROL_COORD_PTR coords = Destination->coords;

        /* Extract the command coordinates. */
        gctFLOAT moveToX = Destination->get(Context, Destination);
        gctFLOAT moveToY = Destination->get(Context, Destination);

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
    }
    vgmLEAVESUBAPI(vgmDEFINESHARED_gcvVGCMD_MOVE_REL);
    /* Success. */
    return gcvSTATUS_OK;
}

/*------------------------------*\
\*------------------------------*/

vgmDEFINEMODIFY(gcvVGCMD_MOVE)
{
    vgmENTERSUBAPI(vgmDEFINEMODIFY_gcvVGCMD_MOVE);
    {
        /* Get a shortcut to the control coordinates. */
        vgsCONTROL_COORD_PTR coords = Destination->coords;

        /* Extract/copy the command coordinates. */
        gctFLOAT moveToX = Source->getcopy(Context, Source, Destination);
        gctFLOAT moveToY = Source->getcopy(Context, Source, Destination);

        /* Update the control coordinates. */
        coords->startX   = moveToX;
        coords->startY   = moveToY;
        coords->lastX    = moveToX;
        coords->lastY    = moveToY;
        coords->controlX = moveToX;
        coords->controlY = moveToY;
    }
    vgmLEAVESUBAPI(vgmDEFINEMODIFY_gcvVGCMD_MOVE);
    /* Success. */
    return gcvSTATUS_OK;
}

vgmDEFINEMODIFY(gcvVGCMD_MOVE_REL)
{
    vgmENTERSUBAPI(vgmDEFINEMODIFY_gcvVGCMD_MOVE_REL);
    {
        /* Get a shortcut to the control coordinates. */
        vgsCONTROL_COORD_PTR coords = Destination->coords;

        /* Extract/copy the command coordinates. */
        gctFLOAT moveToX = Source->getcopy(Context, Source, Destination);
        gctFLOAT moveToY = Source->getcopy(Context, Source, Destination);

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
    }
    vgmLEAVESUBAPI(vgmDEFINEMODIFY_gcvVGCMD_MOVE_REL);
    /* Success. */
    return gcvSTATUS_OK;
}

/*------------------------------*\
\*------------------------------*/

vgmDEFINEUPDATE(gcvVGCMD_MOVE)
{
    vgmENTERSUBAPI(vgmDEFINEUPDATE_gcvVGCMD_MOVE);
    vgmLEAVESUBAPI(vgmDEFINEUPDATE_gcvVGCMD_MOVE);
    /* From this point the control coordinates are not affected
    by the change anymore; break out of the update loop. */
    return gcvSTATUS_NO_MORE_DATA;
}


/******************************************************************************\
******************************** gcvVGCMD_LINE *********************************
\******************************************************************************/

vgmDEFINESKIP(gcvVGCMD_LINE)
{
    vgmENTERSUBAPI(vgmDEFINESKIP_gcvVGCMD_LINE);
    {
        /* Get a shortcut to the control coordinates. */
        vgsCONTROL_COORD_PTR coords = Destination->coords;

        /* Extract the command coordinates. */
        gctFLOAT lineToX = Destination->get(Context, Destination);
        gctFLOAT lineToY = Destination->get(Context, Destination);

        /* Update the control coordinates. */
        coords->lastX    = lineToX;
        coords->lastY    = lineToY;
        coords->controlX = lineToX;
        coords->controlY = lineToY;
    }
    vgmLEAVESUBAPI(vgmDEFINESKIP_gcvVGCMD_LINE);
    /* Success. */
    return gcvSTATUS_OK;
}

vgmDEFINESHARED(gcvVGCMD_LINE_REL)
{
    vgmENTERSUBAPI(vgmDEFINESHARED_gcvVGCMD_LINE_REL);
    {
        /* Get a shortcut to the control coordinates. */
        vgsCONTROL_COORD_PTR coords = Destination->coords;

        /* Extract the command coordinates. */
        gctFLOAT lineToX = Destination->get(Context, Destination);
        gctFLOAT lineToY = Destination->get(Context, Destination);

        /* Determine the absolute coordinates. */
        lineToX += coords->lastX;
        lineToY += coords->lastY;

        /* Update the control coordinates. */
        coords->lastX    = lineToX;
        coords->lastY    = lineToY;
        coords->controlX = lineToX;
        coords->controlY = lineToY;
    }
    vgmLEAVESUBAPI(vgmDEFINESHARED_gcvVGCMD_LINE_REL);
    /* Success. */
    return gcvSTATUS_OK;
}

/*------------------------------*\
\*------------------------------*/

vgmDEFINEMODIFY(gcvVGCMD_LINE)
{
    vgmENTERSUBAPI(vgmDEFINEMODIFY_gcvVGCMD_LINE);
    {
        /* Get a shortcut to the control coordinates. */
        vgsCONTROL_COORD_PTR coords = Destination->coords;

        /* Extract/copy the command coordinates. */
        gctFLOAT lineToX = Source->getcopy(Context, Source, Destination);
        gctFLOAT lineToY = Source->getcopy(Context, Source, Destination);

        /* Update the control coordinates. */
        coords->lastX    = lineToX;
        coords->lastY    = lineToY;
        coords->controlX = lineToX;
        coords->controlY = lineToY;
    }
    vgmLEAVESUBAPI(vgmDEFINEMODIFY_gcvVGCMD_LINE);
    /* Success. */
    return gcvSTATUS_OK;
}

vgmDEFINEMODIFY(gcvVGCMD_LINE_REL)
{
    vgmENTERSUBAPI(vgmDEFINEMODIFY_gcvVGCMD_LINE_REL);
    {
        /* Get a shortcut to the control coordinates. */
        vgsCONTROL_COORD_PTR coords = Destination->coords;

        /* Extract/copy the command coordinates. */
        gctFLOAT lineToX = Source->getcopy(Context, Source, Destination);
        gctFLOAT lineToY = Source->getcopy(Context, Source, Destination);

        /* Determine the absolute coordinates. */
        lineToX += coords->lastX;
        lineToY += coords->lastY;

        /* Update the control coordinates. */
        coords->lastX    = lineToX;
        coords->lastY    = lineToY;
        coords->controlX = lineToX;
        coords->controlY = lineToY;
    }
    vgmLEAVESUBAPI(vgmDEFINEMODIFY_gcvVGCMD_LINE_REL);
    /* Success. */
    return gcvSTATUS_OK;
}

/*------------------------------*\
\*------------------------------*/

vgmDEFINEUPDATE(gcvVGCMD_LINE)
{
    vgmENTERSUBAPI(vgmDEFINEUPDATE_gcvVGCMD_LINE);
    vgmLEAVESUBAPI(vgmDEFINEUPDATE_gcvVGCMD_LINE);
    /* From this point the control coordinates are not affected
    by the change anymore; break out of the update loop. */
    return gcvSTATUS_NO_MORE_DATA;
}


/******************************************************************************\
****************************** gcvVGCMD_HLINE_EMUL *****************************
\******************************************************************************/

vgmDEFINESKIP(gcvVGCMD_HLINE_EMUL)
{
    vgmENTERSUBAPI(vgmDEFINESKIP_gcvVGCMD_HLINE_EMUL);
    {
        /* Get a shortcut to the control coordinates. */
        vgsCONTROL_COORD_PTR coords = Destination->coords;

        /* Extract the command coordinate. */
        gctFLOAT lineToX = Destination->get(Context, Destination);

        /* Update the control coordinates. */
        coords->lastX    = lineToX;
        coords->controlX = lineToX;
        coords->controlY = coords->lastY;
    }
    vgmLEAVESUBAPI(vgmDEFINESKIP_gcvVGCMD_HLINE_EMUL);
    /* Success. */
    return gcvSTATUS_OK;
}

vgmDEFINESHARED(gcvVGCMD_HLINE_EMUL_REL)
{
    vgmENTERSUBAPI(vgmDEFINESHARED_gcvVGCMD_HLINE_EMUL_REL);
    {
        /* Get a shortcut to the control coordinates. */
        vgsCONTROL_COORD_PTR coords = Destination->coords;

        /* Extract the command coordinate. */
        gctFLOAT lineToX = Destination->get(Context, Destination);

        /* Determine the absolute coordinates. */
        lineToX += coords->lastX;

        /* Update the control coordinates. */
        coords->lastX    = lineToX;
        coords->controlX = lineToX;
        coords->controlY = coords->lastY;
    }
    vgmLEAVESUBAPI(vgmDEFINESHARED_gcvVGCMD_HLINE_EMUL_REL);

    /* Success. */
    return gcvSTATUS_OK;
}

/*------------------------------*\
\*------------------------------*/

vgmDEFINEMODIFY(gcvVGCMD_HLINE_EMUL)
{
    vgmENTERSUBAPI(vgmDEFINEMODIFY_gcvVGCMD_HLINE_EMUL);
    {
        /* Get a shortcut to the control coordinates. */
        vgsCONTROL_COORD_PTR coords = Destination->coords;

        /* Extract/copy the command coordinate. */
        gctFLOAT lineToX = Source->getcopy(Context, Source, Destination);

        /* Set the vertical coordinate. */
        Destination->set(Context, Destination, coords->lastY);

        /* Update the control coordinates. */
        coords->lastX    = lineToX;
        coords->controlX = lineToX;
        coords->controlY = coords->lastY;
    }
    vgmLEAVESUBAPI(vgmDEFINEMODIFY_gcvVGCMD_HLINE_EMUL);
    /* Success. */
    return gcvSTATUS_OK;
}

vgmDEFINEMODIFY(gcvVGCMD_HLINE_EMUL_REL)
{
    vgmENTERSUBAPI(vgmDEFINEMODIFY_gcvVGCMD_HLINE_EMUL_REL);
    {
        /* Get a shortcut to the control coordinates. */
        vgsCONTROL_COORD_PTR coords = Destination->coords;

        /* Extract/copy the command coordinate. */
        gctFLOAT lineToX = Source->getcopy(Context, Source, Destination);

        /* Determine the absolute coordinates. */
        lineToX += coords->lastX;

        /* Update the control coordinates. */
        coords->lastX    = lineToX;
        coords->controlX = lineToX;
        coords->controlY = coords->lastY;
    }
    vgmLEAVESUBAPI(vgmDEFINEMODIFY_gcvVGCMD_HLINE_EMUL_REL);
    /* Success. */
    return gcvSTATUS_OK;
}

/*------------------------------*\
\*------------------------------*/

vgmDEFINEUPDATE(gcvVGCMD_HLINE_EMUL)
{
    vgmENTERSUBAPI(vgmDEFINEUPDATE_gcvVGCMD_HLINE_EMUL);
    {
        /* Get a shortcut to the control coordinates. */
        vgsCONTROL_COORD_PTR coords = Destination->coords;

        /* Extract the command coordinate. */
        gctFLOAT lineToX = Destination->get(Context, Destination);

        /* Set the vertical coordinate. */
        Destination->set(Context, Destination, coords->lastY);

        /* Update the control coordinates. */
        coords->lastX    = lineToX;
        coords->controlX = lineToX;
        coords->controlY = coords->lastY;
    }
    vgmLEAVESUBAPI(vgmDEFINEUPDATE_gcvVGCMD_HLINE_EMUL);
    /* Success. */
    return gcvSTATUS_OK;
}


/******************************************************************************\
****************************** gcvVGCMD_VLINE_EMUL *****************************
\******************************************************************************/

vgmDEFINESKIP(gcvVGCMD_VLINE_EMUL)
{
    vgmENTERSUBAPI(vgmDEFINESKIP_gcvVGCMD_VLINE_EMUL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT lineToY;

        /* Get a shortcut to the control coordinates. */
        coords = Destination->coords;

        /* Extract the command coordinate. */
        Destination->get(Context, Destination);
        lineToY = Destination->get(Context, Destination);

        /* Update the control coordinates. */
        coords->lastY    = lineToY;
        coords->controlX = coords->lastX;
        coords->controlY = lineToY;
    }
    vgmLEAVESUBAPI(vgmDEFINESKIP_gcvVGCMD_VLINE_EMUL);
    /* Success. */
    return gcvSTATUS_OK;
}

vgmDEFINESKIP(gcvVGCMD_VLINE_EMUL_REL)
{
    vgmENTERSUBAPI(vgmDEFINESKIP_gcvVGCMD_VLINE_EMUL_REL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT lineToY;

        /* Get a shortcut to the control coordinates. */
        coords = Destination->coords;

        /* Extract the command coordinate. */
        Destination->get(Context, Destination);
        lineToY = Destination->get(Context, Destination);

        /* Determine the absolute coordinates. */
        lineToY += coords->lastY;

        /* Update the control coordinates. */
        coords->lastY    = lineToY;
        coords->controlX = coords->lastX;
        coords->controlY = lineToY;
    }
    vgmLEAVESUBAPI(vgmDEFINESKIP_gcvVGCMD_VLINE_EMUL_REL);
    /* Success. */
    return gcvSTATUS_OK;
}

/*------------------------------*\
\*------------------------------*/

vgmDEFINEMODIFY(gcvVGCMD_VLINE_EMUL)
{
    vgmENTERSUBAPI(vgmDEFINEMODIFY_gcvVGCMD_VLINE_EMUL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT lineToY;

        /* Get a shortcut to the control coordinates. */
        coords = Destination->coords;

        /* Set the horizontal coordinate. */
        Destination->set(Context, Destination, coords->lastX);

        /* Extract/copy the command coordinate. */
        lineToY = Source->getcopy(Context, Source, Destination);

        /* Update the control coordinates. */
        coords->lastY    = lineToY;
        coords->controlX = coords->lastX;
        coords->controlY = lineToY;
    }
    vgmLEAVESUBAPI(vgmDEFINEMODIFY_gcvVGCMD_VLINE_EMUL);
    /* Success. */
    return gcvSTATUS_OK;
}

vgmDEFINEMODIFY(gcvVGCMD_VLINE_EMUL_REL)
{
    vgmENTERSUBAPI(vgmDEFINEMODIFY_gcvVGCMD_VLINE_EMUL_REL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT lineToY;

        /* Get a shortcut to the control coordinates. */
        coords = Destination->coords;

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
    }
    vgmLEAVESUBAPI(vgmDEFINEMODIFY_gcvVGCMD_VLINE_EMUL_REL);
    /* Success. */
    return gcvSTATUS_OK;
}

/*------------------------------*\
\*------------------------------*/

vgmDEFINEUPDATE(gcvVGCMD_VLINE_EMUL)
{
    vgmENTERSUBAPI(vgmDEFINEUPDATE_gcvVGCMD_VLINE_EMUL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT lineToY;

        /* Get a shortcut to the control coordinates. */
        coords = Destination->coords;

        /* Set the horizontal coordinate. */
        Destination->set(Context, Destination, coords->lastX);

        /* Extract the command coordinate. */
        lineToY = Destination->get(Context, Destination);

        /* Update the control coordinates. */
        coords->lastY    = lineToY;
        coords->controlX = coords->lastX;
        coords->controlY = lineToY;
    }
    vgmLEAVESUBAPI(vgmDEFINEUPDATE_gcvVGCMD_VLINE_EMUL);
    /* Success. */
    return gcvSTATUS_OK;
}

vgmDEFINEUPDATE(gcvVGCMD_VLINE_EMUL_REL)
{
    vgmENTERSUBAPI(vgmDEFINEUPDATE_gcvVGCMD_VLINE_EMUL_REL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT lineToY;

        /* Get a shortcut to the control coordinates. */
        coords = Destination->coords;

        /* Set the horizontal coordinate. */
        Destination->set(Context, Destination, 0.0f);

        /* Extract the command coordinate. */
        lineToY = Destination->get(Context, Destination);

        /* Determine the absolute coordinates. */
        lineToY += coords->lastY;

        /* Update the control coordinates. */
        coords->lastY    = lineToY;
        coords->controlX = coords->lastX;
        coords->controlY = lineToY;
    }
    vgmLEAVESUBAPI(vgmDEFINEUPDATE_gcvVGCMD_VLINE_EMUL_REL);
    /* Success. */
    return gcvSTATUS_OK;
}


/******************************************************************************\
********************************** gcvVGCMD_QUAD *******************************
\******************************************************************************/

vgmDEFINESKIP(gcvVGCMD_QUAD)
{
    vgmENTERSUBAPI(vgmDEFINESKIP_gcvVGCMD_QUAD);
    {
        /* Get a shortcut to the control coordinates. */
        vgsCONTROL_COORD_PTR coords = Destination->coords;

        /* Extract/copy the command coordinates. */
        gctFLOAT controlX = Destination->get(Context, Destination);
        gctFLOAT controlY = Destination->get(Context, Destination);
        gctFLOAT quadToX  = Destination->get(Context, Destination);
        gctFLOAT quadToY  = Destination->get(Context, Destination);

        /* Update the last coordinates. */
        coords->lastX    = quadToX;
        coords->lastY    = quadToY;
        coords->controlX = controlX;
        coords->controlY = controlY;
    }
    vgmLEAVESUBAPI(vgmDEFINESKIP_gcvVGCMD_QUAD);
    /* Success. */
    return gcvSTATUS_OK;
}

vgmDEFINESHARED(gcvVGCMD_QUAD_REL)
{
    vgmENTERSUBAPI(vgmDEFINESHARED_gcvVGCMD_QUAD_REL);
    {
        /* Get a shortcut to the control coordinates. */
        vgsCONTROL_COORD_PTR coords = Destination->coords;

        /* Extract/copy the command coordinates. */
        gctFLOAT controlX = Destination->get(Context, Destination);
        gctFLOAT controlY = Destination->get(Context, Destination);
        gctFLOAT quadToX  = Destination->get(Context, Destination);
        gctFLOAT quadToY  = Destination->get(Context, Destination);

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
    }
    vgmLEAVESUBAPI(vgmDEFINESHARED_gcvVGCMD_QUAD_REL);
    /* Success. */
    return gcvSTATUS_OK;
}

/*------------------------------*\
\*------------------------------*/

vgmDEFINEMODIFY(gcvVGCMD_QUAD)
{
    vgmENTERSUBAPI(vgmDEFINEMODIFY_gcvVGCMD_QUAD);
    {
        /* Get a shortcut to the control coordinates. */
        vgsCONTROL_COORD_PTR coords = Destination->coords;

        /* Extract/copy the command coordinates. */
        gctFLOAT controlX = Source->getcopy(Context, Source, Destination);
        gctFLOAT controlY = Source->getcopy(Context, Source, Destination);
        gctFLOAT quadToX  = Source->getcopy(Context, Source, Destination);
        gctFLOAT quadToY  = Source->getcopy(Context, Source, Destination);

        /* Update the control coordinates. */
        coords->lastX    = quadToX;
        coords->lastY    = quadToY;
        coords->controlX = controlX;
        coords->controlY = controlY;
    }
    vgmLEAVESUBAPI(vgmDEFINEMODIFY_gcvVGCMD_QUAD);
    /* Success. */
    return gcvSTATUS_OK;
}

vgmDEFINEMODIFY(gcvVGCMD_QUAD_REL)
{
    vgmENTERSUBAPI(vgmDEFINEMODIFY_gcvVGCMD_QUAD_REL);
    {
        /* Get a shortcut to the control coordinates. */
        vgsCONTROL_COORD_PTR coords = Destination->coords;

        /* Extract/copy the command coordinates. */
        gctFLOAT controlX = Source->getcopy(Context, Source, Destination);
        gctFLOAT controlY = Source->getcopy(Context, Source, Destination);
        gctFLOAT quadToX  = Source->getcopy(Context, Source, Destination);
        gctFLOAT quadToY  = Source->getcopy(Context, Source, Destination);

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
    }
    vgmLEAVESUBAPI(vgmDEFINEMODIFY_gcvVGCMD_QUAD_REL);
    /* Success. */
    return gcvSTATUS_OK;
}

/*------------------------------*\
\*------------------------------*/

vgmDEFINEUPDATE(gcvVGCMD_QUAD)
{
    vgmENTERSUBAPI(vgmDEFINEUPDATE_gcvVGCMD_QUAD);
    vgmLEAVESUBAPI(vgmDEFINEUPDATE_gcvVGCMD_QUAD);
    /* From this point the control coordinates are not affected
    by the change anymore; break out of the update loop. */
    return gcvSTATUS_NO_MORE_DATA;
}


/******************************************************************************\
********************************* gcvVGCMD_CUBIC *******************************
\******************************************************************************/

vgmDEFINESKIP(gcvVGCMD_CUBIC)
{
    vgmENTERSUBAPI(vgmDEFINESKIP_gcvVGCMD_CUBIC);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT controlX2, controlY2;
        gctFLOAT cubicToX, cubicToY;

        /* Get a shortcut to the control coordinates. */
        coords = Destination->coords;

        /* Extract the command coordinates. */
        Destination->get(Context, Destination);
        Destination->get(Context, Destination);
        controlX2 = Destination->get(Context, Destination);
        controlY2 = Destination->get(Context, Destination);
        cubicToX  = Destination->get(Context, Destination);
        cubicToY  = Destination->get(Context, Destination);

        /* Update the control coordinates. */
        coords->lastX    = cubicToX;
        coords->lastY    = cubicToY;
        coords->controlX = controlX2;
        coords->controlY = controlY2;
    }
    vgmLEAVESUBAPI(vgmDEFINESKIP_gcvVGCMD_CUBIC);
    /* Success. */
    return gcvSTATUS_OK;
}

vgmDEFINESHARED(gcvVGCMD_CUBIC_REL)
{
    vgmENTERSUBAPI(vgmDEFINESHARED_gcvVGCMD_CUBIC_REL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT controlX2, controlY2;
        gctFLOAT cubicToX, cubicToY;

        /* Get a shortcut to the control coordinates. */
        coords = Destination->coords;

        /* Extract the command coordinates. */
        Destination->get(Context, Destination);
        Destination->get(Context, Destination);
        controlX2 = Destination->get(Context, Destination);
        controlY2 = Destination->get(Context, Destination);
        cubicToX  = Destination->get(Context, Destination);
        cubicToY  = Destination->get(Context, Destination);

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
    }
    vgmLEAVESUBAPI(vgmDEFINESHARED_gcvVGCMD_CUBIC_REL);
    /* Success. */
    return gcvSTATUS_OK;
}

/*------------------------------*\
\*------------------------------*/

vgmDEFINEMODIFY(gcvVGCMD_CUBIC)
{
    vgmENTERSUBAPI(vgmDEFINEMODIFY_gcvVGCMD_CUBIC);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT controlX, controlY;
        gctFLOAT cubicToX, cubicToY;

        /* Get a shortcut to the control coordinates. */
        coords = Destination->coords;

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
    }
    vgmLEAVESUBAPI(vgmDEFINEMODIFY_gcvVGCMD_CUBIC);
    /* Success. */
    return gcvSTATUS_OK;
}

vgmDEFINEMODIFY(gcvVGCMD_CUBIC_REL)
{
    vgmENTERSUBAPI(vgmDEFINEMODIFY_gcvVGCMD_CUBIC_REL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT controlX, controlY;
        gctFLOAT cubicToX, cubicToY;

        /* Get a shortcut to the control coordinates. */
        coords = Destination->coords;

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
    }
    vgmLEAVESUBAPI(vgmDEFINEMODIFY_gcvVGCMD_CUBIC_REL);
    /* Success. */
    return gcvSTATUS_OK;
}

/*------------------------------*\
\*------------------------------*/

vgmDEFINEUPDATE(gcvVGCMD_CUBIC)
{
    vgmENTERSUBAPI(vgmDEFINEUPDATE_gcvVGCMD_CUBIC);
    vgmLEAVESUBAPI(vgmDEFINEUPDATE_gcvVGCMD_CUBIC);
    /* From this point the control coordinates are not affected
    by the change anymore; break out of the update loop. */
    return gcvSTATUS_NO_MORE_DATA;
}


/******************************************************************************\
******************************* gcvVGCMD_SQUAD_EMUL ****************************
\******************************************************************************/

vgmDEFINESKIP(gcvVGCMD_SQUAD_EMUL)
{
    vgmENTERSUBAPI(vgmDEFINESKIP_gcvVGCMD_SQUAD_EMUL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT controlX, controlY;
        gctFLOAT quadToX, quadToY;

        /* Get a shortcut to the control coordinates. */
        coords = Destination->coords;

        /* Extract the command coordinates. */
        controlX = Destination->get(Context, Destination);
        controlY = Destination->get(Context, Destination);
        quadToX  = Destination->get(Context, Destination);
        quadToY  = Destination->get(Context, Destination);

        /* Update the control coordinates. */
        coords->lastX    = quadToX;
        coords->lastY    = quadToY;
        coords->controlX = controlX;
        coords->controlY = controlY;
    }
    vgmLEAVESUBAPI(vgmDEFINESKIP_gcvVGCMD_SQUAD_EMUL);
    /* Success. */
    return gcvSTATUS_OK;
}

vgmDEFINESKIP(gcvVGCMD_SQUAD_EMUL_REL)
{
    vgmENTERSUBAPI(vgmDEFINESKIP_gcvVGCMD_SQUAD_EMUL_REL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT controlX, controlY;
        gctFLOAT quadToX, quadToY;

        /* Get a shortcut to the control coordinates. */
        coords = Destination->coords;

        /* Extract the command coordinates. */
        controlX = Destination->get(Context, Destination);
        controlY = Destination->get(Context, Destination);
        quadToX  = Destination->get(Context, Destination);
        quadToY  = Destination->get(Context, Destination);

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
    }
    vgmLEAVESUBAPI(vgmDEFINESKIP_gcvVGCMD_SQUAD_EMUL_REL);
    /* Success. */
    return gcvSTATUS_OK;
}

/*------------------------------*\
\*------------------------------*/

vgmDEFINEMODIFY(gcvVGCMD_SQUAD_EMUL)
{
    vgmENTERSUBAPI(vgmDEFINEMODIFY_gcvVGCMD_SQUAD_EMUL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT controlX, controlY;
        gctFLOAT quadToX, quadToY;

        /* Get a shortcut to the control coordinates. */
        coords = Destination->coords;

        /* Compute the control point. */
        controlX = 2.0f * coords->lastX - coords->controlX;
        controlY = 2.0f * coords->lastY - coords->controlY;

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
    }
    vgmLEAVESUBAPI(vgmDEFINEMODIFY_gcvVGCMD_SQUAD_EMUL);
    /* Success. */
    return gcvSTATUS_OK;
}

vgmDEFINEMODIFY(gcvVGCMD_SQUAD_EMUL_REL)
{
    vgmENTERSUBAPI(vgmDEFINEMODIFY_gcvVGCMD_SQUAD_EMUL_REL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT controlX, controlY;
        gctFLOAT quadToX, quadToY;

        /* Get a shortcut to the control coordinates. */
        coords = Destination->coords;

        /* Compute the relative control point. */
        controlX = coords->lastX - coords->controlX;
        controlY = coords->lastY - coords->controlY;

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
    }
    vgmLEAVESUBAPI(vgmDEFINEMODIFY_gcvVGCMD_SQUAD_EMUL_REL);
    /* Success. */
    return gcvSTATUS_OK;
}

/*------------------------------*\
\*------------------------------*/

vgmDEFINEUPDATE(gcvVGCMD_SQUAD_EMUL)
{
    vgmENTERSUBAPI(vgmDEFINEUPDATE_gcvVGCMD_SQUAD_EMUL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT controlX, controlY;
        gctFLOAT quadToX, quadToY;

        /* Get a shortcut to the control coordinates. */
        coords = Destination->coords;

        /* Compute the control point. */
        controlX = 2.0f * coords->lastX - coords->controlX;
        controlY = 2.0f * coords->lastY - coords->controlY;

        /* Store the control point. */
        Destination->set(Context, Destination, controlX);
        Destination->set(Context, Destination, controlY);

        /* Extract the command coordinates. */
        quadToX = Destination->get(Context, Destination);
        quadToY = Destination->get(Context, Destination);

        /* Update the control coordinates. */
        coords->lastX    = quadToX;
        coords->lastY    = quadToY;
        coords->controlX = controlX;
        coords->controlY = controlY;
    }
    vgmLEAVESUBAPI(vgmDEFINEUPDATE_gcvVGCMD_SQUAD_EMUL);
    /* Success. */
    return gcvSTATUS_OK;
}

vgmDEFINEUPDATE(gcvVGCMD_SQUAD_EMUL_REL)
{
    vgmENTERSUBAPI(vgmDEFINEUPDATE_gcvVGCMD_SQUAD_EMUL_REL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT controlX, controlY;
        gctFLOAT quadToX, quadToY;

        /* Get a shortcut to the control coordinates. */
        coords = Destination->coords;

        /* Compute the relative control point. */
        controlX = coords->lastX - coords->controlX;
        controlY = coords->lastY - coords->controlY;

        /* Store the control point. */
        Destination->set(Context, Destination, controlX);
        Destination->set(Context, Destination, controlY);

        /* Extract the command coordinates. */
        quadToX = Destination->get(Context, Destination);
        quadToY = Destination->get(Context, Destination);

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
    }
    vgmLEAVESUBAPI(vgmDEFINEUPDATE_gcvVGCMD_SQUAD_EMUL_REL);
    /* Success. */
    return gcvSTATUS_OK;
}


/******************************************************************************\
****************************** gcvVGCMD_SCUBIC_EMUL ****************************
\******************************************************************************/

vgmDEFINESKIP(gcvVGCMD_SCUBIC_EMUL)
{
    vgmENTERSUBAPI(vgmDEFINESKIP_gcvVGCMD_SCUBIC_EMUL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT controlX2, controlY2;
        gctFLOAT cubicToX, cubicToY;

        /* Get a shortcut to the control coordinates. */
        coords = Destination->coords;

        /* Extract/copy the command coordinates. */
        Destination->get(Context, Destination);
        Destination->get(Context, Destination);
        controlX2 = Destination->get(Context, Destination);
        controlY2 = Destination->get(Context, Destination);
        cubicToX  = Destination->get(Context, Destination);
        cubicToY  = Destination->get(Context, Destination);

        /* Update the control coordinates. */
        coords->lastX    = cubicToX;
        coords->lastY    = cubicToY;
        coords->controlX = controlX2;
        coords->controlY = controlY2;
    }
    vgmLEAVESUBAPI(vgmDEFINESKIP_gcvVGCMD_SCUBIC_EMUL);
    /* Success. */
    return gcvSTATUS_OK;
}

vgmDEFINESKIP(gcvVGCMD_SCUBIC_EMUL_REL)
{
    vgmENTERSUBAPI(vgmDEFINESKIP_gcvVGCMD_SCUBIC_EMUL_REL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT controlX2, controlY2;
        gctFLOAT cubicToX, cubicToY;

        /* Get a shortcut to the control coordinates. */
        coords = Destination->coords;

        /* Extract/copy the command coordinates. */
        Destination->get(Context, Destination);
        Destination->get(Context, Destination);
        controlX2 = Destination->get(Context, Destination);
        controlY2 = Destination->get(Context, Destination);
        cubicToX  = Destination->get(Context, Destination);
        cubicToY  = Destination->get(Context, Destination);

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
    }
    vgmLEAVESUBAPI(vgmDEFINESKIP_gcvVGCMD_SCUBIC_EMUL_REL);
    /* Success. */
    return gcvSTATUS_OK;
}

/*------------------------------*\
\*------------------------------*/

vgmDEFINEMODIFY(gcvVGCMD_SCUBIC_EMUL)
{
    vgmENTERSUBAPI(vgmDEFINEMODIFY_gcvVGCMD_SCUBIC_EMUL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT controlX1, controlY1;
        gctFLOAT controlX2, controlY2;
        gctFLOAT cubicToX, cubicToY;

        /* Get a shortcut to the control coordinates. */
        coords = Destination->coords;

        /* Compute the control point. */
        controlX1 = 2.0f * coords->lastX - coords->controlX;
        controlY1 = 2.0f * coords->lastY - coords->controlY;

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
    }
    vgmLEAVESUBAPI(vgmDEFINEMODIFY_gcvVGCMD_SCUBIC_EMUL);
    /* Success. */
    return gcvSTATUS_OK;
}

vgmDEFINEMODIFY(gcvVGCMD_SCUBIC_EMUL_REL)
{
    vgmENTERSUBAPI(vgmDEFINEMODIFY_gcvVGCMD_SCUBIC_EMUL_REL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT controlX1, controlY1;
        gctFLOAT controlX2, controlY2;
        gctFLOAT cubicToX, cubicToY;

        /* Get a shortcut to the control coordinates. */
        coords = Destination->coords;

        /* Compute the relative control point. */
        controlX1 = coords->lastX - coords->controlX;
        controlY1 = coords->lastY - coords->controlY;

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
    }
    vgmLEAVESUBAPI(vgmDEFINEMODIFY_gcvVGCMD_SCUBIC_EMUL_REL);
    /* Success. */
    return gcvSTATUS_OK;
}

/*------------------------------*\
\*------------------------------*/

vgmDEFINEUPDATE(gcvVGCMD_SCUBIC_EMUL)
{
    vgmENTERSUBAPI(vgmDEFINEUPDATE_gcvVGCMD_SCUBIC_EMUL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT controlX1, controlY1;

        /* Get a shortcut to the control coordinates. */
        coords = Destination->coords;

        /* Compute the control point. */
        controlX1 = 2.0f * coords->lastX - coords->controlX;
        controlY1 = 2.0f * coords->lastY - coords->controlY;

        /* Store the control point. */
        Destination->set(Context, Destination, controlX1);
        Destination->set(Context, Destination, controlY1);
    }
    vgmLEAVESUBAPI(vgmDEFINEUPDATE_gcvVGCMD_SCUBIC_EMUL);
    /* From this point the control coordinates are not affected
    by the change anymore; break out of the update loop. */
    return gcvSTATUS_NO_MORE_DATA;
}

vgmDEFINEUPDATE(gcvVGCMD_SCUBIC_EMUL_REL)
{
    vgmENTERSUBAPI(vgmDEFINEUPDATE_gcvVGCMD_SCUBIC_EMUL_REL);
    {
        vgsCONTROL_COORD_PTR coords;
        gctFLOAT controlX1, controlY1;
        gctFLOAT controlX2, controlY2;
        gctFLOAT cubicToX, cubicToY;

        /* Get a shortcut to the control coordinates. */
        coords = Destination->coords;

        /* Compute the relative control point. */
        controlX1 = coords->lastX - coords->controlX;
        controlY1 = coords->lastY - coords->controlY;

        /* Store the control point. */
        Destination->set(Context, Destination, controlX1);
        Destination->set(Context, Destination, controlY1);

        /* Extract/copy the command coordinates. */
        controlX2 = Destination->get(Context, Destination);
        controlY2 = Destination->get(Context, Destination);
        cubicToX  = Destination->get(Context, Destination);
        cubicToY  = Destination->get(Context, Destination);

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
    }
    vgmLEAVESUBAPI(vgmDEFINEUPDATE_gcvVGCMD_SCUBIC_EMUL_REL);
    /* Success. */
    return gcvSTATUS_OK;
}


/******************************************************************************\
********************************** gcvVGCMD_ARC ********************************
\******************************************************************************/

vgmDEFINESKIP(gcvVGCMD_ARC)
{
    vgmENTERSUBAPI(vgmDEFINESKIP_gcvVGCMD_ARC);
    {
        vgsCONTROL_COORD_PTR coords;
        vgsARCCOORDINATES_PTR arcCoords;
        gctFLOAT arcToX, arcToY;

        /* Get a shortcut to the control coordinates. */
        coords = Destination->coords;

        /* Get a pointer to the original ARC data. */
        arcCoords = (vgsARCCOORDINATES_PTR) Destination->currPathData->extra;

        /* Extract the command coordinates. */
        arcToX = arcCoords->endX;
        arcToY = arcCoords->endY;

        /* Update the control coordinates. */
        coords->lastX    = arcToX;
        coords->lastY    = arcToY;
        coords->controlX = arcToX;
        coords->controlY = arcToY;

        /* Skip the ARC path in the source. */
        vgsPATHWALKER_SeekToEnd(Context, Destination);
    }
    vgmLEAVESUBAPI(vgmDEFINESKIP_gcvVGCMD_ARC);
    /* Success. */
    return gcvSTATUS_OK;
}

vgmDEFINESHARED(gcvVGCMD_ARC_REL)
{
    vgmENTERSUBAPI(vgmDEFINESHARED_gcvVGCMD_ARC_REL);
    {
        vgsCONTROL_COORD_PTR coords;
        vgsARCCOORDINATES_PTR arcCoords;
        gctFLOAT arcToX, arcToY;

        /* Get a shortcut to the control coordinates. */
        coords = Destination->coords;

        /* Get a pointer to the original ARC data. */
        arcCoords = (vgsARCCOORDINATES_PTR) Destination->currPathData->extra;

        /* Extract the command coordinates. */
        arcToX = arcCoords->endX + coords->lastX;
        arcToY = arcCoords->endY + coords->lastY;

        /* Update the control coordinates. */
        coords->lastX    = arcToX;
        coords->lastY    = arcToY;
        coords->controlX = arcToX;
        coords->controlY = arcToY;

        /* Skip the ARC path in the source. */
        vgsPATHWALKER_SeekToEnd(Context, Destination);
    }
    vgmLEAVESUBAPI(vgmDEFINESHARED_gcvVGCMD_ARC_REL);
    /* Success. */
    return gcvSTATUS_OK;
}

/*------------------------------*\
\*------------------------------*/

static gceSTATUS _ModifyArc(
    vgsCONTEXT_PTR Context,
    vgsPATHWALKER_PTR Destination,
    vgsPATHWALKER_PTR Source,
    gctBOOL Relative
    )
{
    gceSTATUS status;
    vgmENTERSUBAPI(_ModifyArc);
    do
    {
        vgsCONTEXT_PTR context;
        vgsARCCOORDINATES_PTR arcCoords;
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

        /* Get a pointer to the original ARC data. */
        arcCoords = (vgsARCCOORDINATES_PTR) Destination->currPathData->extra;

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
            arcCoords->counterClockwise, arcCoords->large, Relative
            ));

        /* Replace the current path data with the new one. */
        vgsPATHWALKER_ReplaceData(
            Context, Destination, &arc
            );
    }
    while (gcvFALSE);

    vgmLEAVESUBAPI(_ModifyArc);
    /* Return status. */
    return status;
}

vgmDEFINEMODIFY(gcvVGCMD_ARC)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEMODIFY_gcvVGCMD_ARC);
    status = _ModifyArc(Context, Destination, Source, gcvFALSE);
    vgmLEAVESUBAPI(vgmDEFINEMODIFY_gcvVGCMD_ARC);
    return status;
}

vgmDEFINEMODIFY(gcvVGCMD_ARC_REL)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEMODIFY_gcvVGCMD_ARC_REL);
    status = _ModifyArc(Context, Destination, Source, gcvTRUE);
    vgmLEAVESUBAPI(vgmDEFINEMODIFY_gcvVGCMD_ARC_REL);
    return status;
}

/*------------------------------*\
\*------------------------------*/

vgmDEFINEUPDATE(gcvVGCMD_ARC)
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgmDEFINEUPDATE_gcvVGCMD_ARC);

    do
    {
        vgsCONTEXT_PTR context;
        vgsARCCOORDINATES_PTR arcCoords;
        vgsPATHWALKER arc;

        /* Get a pointer to the original ARC data. */
        arcCoords = (vgsARCCOORDINATES_PTR) Destination->currPathData->extra;

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
            arcCoords->horRadius, arcCoords->verRadius,
            arcCoords->rotAngle,
            arcCoords->endX, arcCoords->endY,
            arcCoords->counterClockwise,
            arcCoords->large,
            gcvFALSE
            ));

        /* Replace the current path data with the new one. */
        vgsPATHWALKER_ReplaceData(
            Context, Destination, &arc
            );

        /* From this point the control coordinates are not affected
        by the change anymore; break out of the update loop. */
        status = gcvSTATUS_NO_MORE_DATA;
    }
    while (gcvFALSE);

    vgmLEAVESUBAPI(vgmDEFINEUPDATE_gcvVGCMD_ARC);
    /* Return status. */
    return status;
}


/*******************************************************************************
**
** vgfGetSkipArray
**
** Returns the pointer to the array of path updating functions. Update is
** required by some path commands after preseeding coordinates have been
** modified.
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

void vgfGetSkipArray(
    vgsCONTEXT_PTR Context,
    vgtSEGMENTHANDLER ** Array,
    gctUINT_PTR Count
    )
{
    vgmENTERSUBAPI(vgfGetSkipArray);
    {
        static vgtSEGMENTHANDLER _updateCommand[] =
        {
            vgmSHARED(Invalid),                  /*   0: gcvVGCMD_END             */
            vgmSHARED(gcvVGCMD_CLOSE),           /*   1: gcvVGCMD_CLOSE           */
            vgmSKIP  (gcvVGCMD_MOVE),            /*   2: gcvVGCMD_MOVE            */
            vgmSHARED(gcvVGCMD_MOVE_REL),        /*   3: gcvVGCMD_MOVE_REL        */
            vgmSKIP  (gcvVGCMD_LINE),            /*   4: gcvVGCMD_LINE            */
            vgmSHARED(gcvVGCMD_LINE_REL),        /*   5: gcvVGCMD_LINE_REL        */
            vgmSKIP  (gcvVGCMD_QUAD),            /*   6: gcvVGCMD_QUAD            */
            vgmSHARED(gcvVGCMD_QUAD_REL),        /*   7: gcvVGCMD_QUAD_REL        */
            vgmSKIP  (gcvVGCMD_CUBIC),           /*   8: gcvVGCMD_CUBIC           */
            vgmSHARED(gcvVGCMD_CUBIC_REL),       /*   9: gcvVGCMD_CUBIC_REL       */
            vgmSHARED(Invalid),                  /*  10: gcvVGCMD_BREAK           */
            vgmSHARED(Invalid),                  /*  11: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  12: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  13: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  14: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  15: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  16: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  17: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  18: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  19: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  20: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  21: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  22: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  23: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  24: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  25: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  26: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  27: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  28: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  29: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  30: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  31: **** R E S E R V E D *****/

            vgmSHARED(Invalid),                  /*  32: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  33: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  34: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  35: ***** I N V A L I D ******/
            vgmSKIP  (gcvVGCMD_HLINE_EMUL),      /*  36: gcvVGCMD_HLINE_EMUL      */
            vgmSHARED(gcvVGCMD_HLINE_EMUL_REL),  /*  37: gcvVGCMD_HLINE_EMUL_REL  */
            vgmSHARED(Invalid),                  /*  38: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  39: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  40: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  41: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  42: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  43: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  44: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  45: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  46: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  47: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  48: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  49: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  50: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  51: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  52: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  53: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  54: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  55: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  56: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  57: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  58: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  59: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  60: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  61: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  62: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  63: ***** I N V A L I D ******/

            vgmSHARED(Invalid),                  /*  64: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  65: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  66: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  67: ***** I N V A L I D ******/
            vgmSKIP  (gcvVGCMD_VLINE_EMUL),      /*  68: gcvVGCMD_VLINE_EMUL      */
            vgmSKIP  (gcvVGCMD_VLINE_EMUL_REL),  /*  69: gcvVGCMD_VLINE_EMUL_REL  */
            vgmSHARED(Invalid),                  /*  70: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  71: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  72: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  73: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  74: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  75: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  76: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  77: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  78: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  79: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  80: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  81: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  82: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  83: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  84: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  85: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  86: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  87: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  88: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  89: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  90: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  91: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  92: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  93: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  94: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  95: ***** I N V A L I D ******/

            vgmSHARED(Invalid),                  /*  96: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  97: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  98: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  99: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 100: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 101: ***** I N V A L I D ******/
            vgmSKIP(gcvVGCMD_SQUAD_EMUL),        /* 102: gcvVGCMD_SQUAD_EMUL      */
            vgmSKIP(gcvVGCMD_SQUAD_EMUL_REL),    /* 103: gcvVGCMD_SQUAD_EMUL_REL  */
            vgmSKIP(gcvVGCMD_SCUBIC_EMUL),       /* 104: gcvVGCMD_SCUBIC_EMUL     */
            vgmSKIP(gcvVGCMD_SCUBIC_EMUL_REL),   /* 105: gcvVGCMD_SCUBIC_EMUL_REL */
            vgmSHARED(Invalid),                  /* 106: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 107: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 108: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 109: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 110: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 111: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 112: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 113: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 114: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 115: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 116: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 117: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 118: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 119: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 120: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 121: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 122: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 123: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 124: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 125: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 126: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 127: ***** I N V A L I D ******/

            vgmSHARED(Invalid),                  /* 128: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 129: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 130: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 131: ***** I N V A L I D ******/
            vgmSKIP  (gcvVGCMD_ARC),             /* 132: gcvVGCMD_ARC_LINE        */
            vgmSHARED(gcvVGCMD_ARC_REL),         /* 133: gcvVGCMD_ARC_LINE_REL    */
            vgmSKIP  (gcvVGCMD_ARC),             /* 134: gcvVGCMD_ARC_QUAD        */
            vgmSHARED(gcvVGCMD_ARC_REL)          /* 135: gcvVGCMD_ARC_QUAD_REL    */
        };

        * Array = _updateCommand;
        * Count = gcmCOUNTOF(_updateCommand);
    }
    vgmLEAVESUBAPI(vgfGetSkipArray);
}


/*******************************************************************************
**
** vgfGetModifyArray
**
** Returns the pointer to the array of path segment skipping functions.
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

void vgfGetModifyArray(
    vgsCONTEXT_PTR Context,
    vgtSEGMENTHANDLER ** Array,
    gctUINT_PTR Count
    )
{
    vgmENTERSUBAPI(vgfGetModifyArray);
    {
        static vgtSEGMENTHANDLER _modifyCommand[] =
        {
            vgmSHARED(Invalid),                  /*   0: gcvVGCMD_END             */
            vgmSHARED(gcvVGCMD_CLOSE),           /*   1: gcvVGCMD_CLOSE           */
            vgmMODIFY(gcvVGCMD_MOVE),            /*   2: gcvVGCMD_MOVE            */
            vgmMODIFY(gcvVGCMD_MOVE_REL),        /*   3: gcvVGCMD_MOVE_REL        */
            vgmMODIFY(gcvVGCMD_LINE),            /*   4: gcvVGCMD_LINE            */
            vgmMODIFY(gcvVGCMD_LINE_REL),        /*   5: gcvVGCMD_LINE_REL        */
            vgmMODIFY(gcvVGCMD_QUAD),            /*   6: gcvVGCMD_QUAD            */
            vgmMODIFY(gcvVGCMD_QUAD_REL),        /*   7: gcvVGCMD_QUAD_REL        */
            vgmMODIFY(gcvVGCMD_CUBIC),           /*   8: gcvVGCMD_CUBIC           */
            vgmMODIFY(gcvVGCMD_CUBIC_REL),       /*   9: gcvVGCMD_CUBIC_REL       */
            vgmSHARED(Invalid),                  /*  10: gcvVGCMD_BREAK           */
            vgmSHARED(Invalid),                  /*  11: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  12: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  13: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  14: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  15: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  16: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  17: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  18: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  19: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  20: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  21: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  22: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  23: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  24: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  25: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  26: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  27: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  28: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  29: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  30: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  31: **** R E S E R V E D *****/

            vgmSHARED(Invalid),                  /*  32: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  33: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  34: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  35: ***** I N V A L I D ******/
            vgmMODIFY(gcvVGCMD_HLINE_EMUL),      /*  36: gcvVGCMD_HLINE_EMUL      */
            vgmMODIFY(gcvVGCMD_HLINE_EMUL_REL),  /*  37: gcvVGCMD_HLINE_EMUL_REL  */
            vgmSHARED(Invalid),                  /*  38: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  39: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  40: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  41: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  42: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  43: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  44: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  45: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  46: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  47: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  48: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  49: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  50: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  51: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  52: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  53: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  54: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  55: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  56: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  57: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  58: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  59: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  60: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  61: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  62: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  63: ***** I N V A L I D ******/

            vgmSHARED(Invalid),                  /*  64: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  65: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  66: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  67: ***** I N V A L I D ******/
            vgmMODIFY(gcvVGCMD_VLINE_EMUL),      /*  68: gcvVGCMD_VLINE_EMUL      */
            vgmMODIFY(gcvVGCMD_VLINE_EMUL_REL),  /*  69: gcvVGCMD_VLINE_EMUL_REL  */
            vgmSHARED(Invalid),                  /*  70: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  71: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  72: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  73: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  74: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  75: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  76: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  77: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  78: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  79: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  80: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  81: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  82: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  83: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  84: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  85: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  86: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  87: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  88: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  89: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  90: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  91: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  92: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  93: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  94: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  95: ***** I N V A L I D ******/

            vgmSHARED(Invalid),                  /*  96: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  97: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  98: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  99: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 100: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 101: ***** I N V A L I D ******/
            vgmMODIFY(gcvVGCMD_SQUAD_EMUL),      /* 102: gcvVGCMD_SQUAD_EMUL      */
            vgmMODIFY(gcvVGCMD_SQUAD_EMUL_REL),  /* 103: gcvVGCMD_SQUAD_EMUL_REL  */
            vgmMODIFY(gcvVGCMD_SCUBIC_EMUL),     /* 104: gcvVGCMD_SCUBIC_EMUL     */
            vgmMODIFY(gcvVGCMD_SCUBIC_EMUL_REL), /* 105: gcvVGCMD_SCUBIC_EMUL_REL */
            vgmSHARED(Invalid),                  /* 106: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 107: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 108: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 109: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 110: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 111: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 112: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 113: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 114: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 115: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 116: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 117: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 118: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 119: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 120: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 121: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 122: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 123: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 124: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 125: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 126: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 127: ***** I N V A L I D ******/

            vgmSHARED(Invalid),                  /* 128: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 129: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 130: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 131: ***** I N V A L I D ******/
            vgmMODIFY(gcvVGCMD_ARC),             /* 132: gcvVGCMD_ARC_LINE        */
            vgmMODIFY(gcvVGCMD_ARC_REL),         /* 133: gcvVGCMD_ARC_LINE_REL    */
            vgmMODIFY(gcvVGCMD_ARC),             /* 134: gcvVGCMD_ARC_QUAD        */
            vgmMODIFY(gcvVGCMD_ARC_REL)          /* 135: gcvVGCMD_ARC_QUAD_REL    */
        };

        * Array = _modifyCommand;
        * Count = gcmCOUNTOF(_modifyCommand);
    }
    vgmLEAVESUBAPI(vgfGetModifyArray);
}


/*******************************************************************************
**
** vgfGetUpdateArray
**
** Returns the pointer to the array of path updating functions. Update is
** required by some path commands after preseeding coordinates have been
** modified.
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

void vgfGetUpdateArray(
    vgsCONTEXT_PTR Context,
    vgtSEGMENTHANDLER ** Array,
    gctUINT_PTR Count
    )
{
    vgmENTERSUBAPI(vgfGetUpdateArray);
    {
        static vgtSEGMENTHANDLER _updateCommand[] =
        {
            vgmSHARED(Invalid),                  /*   0: gcvVGCMD_END             */
            vgmSHARED(gcvVGCMD_CLOSE),           /*   1: gcvVGCMD_CLOSE           */
            vgmUPDATE(gcvVGCMD_MOVE),            /*   2: gcvVGCMD_MOVE            */
            vgmSHARED(gcvVGCMD_MOVE_REL),        /*   3: gcvVGCMD_MOVE_REL        */
            vgmUPDATE(gcvVGCMD_LINE),            /*   4: gcvVGCMD_LINE            */
            vgmSHARED(gcvVGCMD_LINE_REL),        /*   5: gcvVGCMD_LINE_REL        */
            vgmUPDATE(gcvVGCMD_QUAD),            /*   6: gcvVGCMD_QUAD            */
            vgmSHARED(gcvVGCMD_QUAD_REL),        /*   7: gcvVGCMD_QUAD_REL        */
            vgmUPDATE(gcvVGCMD_CUBIC),           /*   8: gcvVGCMD_CUBIC           */
            vgmSHARED(gcvVGCMD_CUBIC_REL),       /*   9: gcvVGCMD_CUBIC_REL       */
            vgmSHARED(Invalid),                  /*  10: gcvVGCMD_BREAK           */
            vgmSHARED(Invalid),                  /*  11: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  12: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  13: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  14: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  15: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  16: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  17: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  18: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  19: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  20: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  21: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  22: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  23: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  24: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  25: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  26: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  27: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  28: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  29: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  30: **** R E S E R V E D *****/
            vgmSHARED(Invalid),                  /*  31: **** R E S E R V E D *****/

            vgmSHARED(Invalid),                  /*  32: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  33: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  34: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  35: ***** I N V A L I D ******/
            vgmUPDATE(gcvVGCMD_HLINE_EMUL),      /*  36: gcvVGCMD_HLINE_EMUL      */
            vgmSHARED(gcvVGCMD_HLINE_EMUL_REL),  /*  37: gcvVGCMD_HLINE_EMUL_REL  */
            vgmSHARED(Invalid),                  /*  38: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  39: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  40: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  41: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  42: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  43: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  44: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  45: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  46: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  47: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  48: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  49: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  50: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  51: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  52: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  53: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  54: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  55: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  56: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  57: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  58: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  59: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  60: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  61: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  62: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  63: ***** I N V A L I D ******/

            vgmSHARED(Invalid),                  /*  64: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  65: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  66: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  67: ***** I N V A L I D ******/
            vgmUPDATE(gcvVGCMD_VLINE_EMUL),      /*  68: gcvVGCMD_VLINE_EMUL      */
            vgmUPDATE(gcvVGCMD_VLINE_EMUL_REL),  /*  69: gcvVGCMD_VLINE_EMUL_REL  */
            vgmSHARED(Invalid),                  /*  70: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  71: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  72: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  73: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  74: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  75: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  76: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  77: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  78: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  79: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  80: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  81: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  82: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  83: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  84: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  85: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  86: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  87: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  88: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  89: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  90: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  91: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  92: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  93: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  94: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  95: ***** I N V A L I D ******/

            vgmSHARED(Invalid),                  /*  96: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  97: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  98: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /*  99: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 100: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 101: ***** I N V A L I D ******/
            vgmUPDATE(gcvVGCMD_SQUAD_EMUL),      /* 102: gcvVGCMD_SQUAD_EMUL      */
            vgmUPDATE(gcvVGCMD_SQUAD_EMUL_REL),  /* 103: gcvVGCMD_SQUAD_EMUL_REL  */
            vgmUPDATE(gcvVGCMD_SCUBIC_EMUL),     /* 104: gcvVGCMD_SCUBIC_EMUL     */
            vgmUPDATE(gcvVGCMD_SCUBIC_EMUL_REL), /* 105: gcvVGCMD_SCUBIC_EMUL_REL */
            vgmSHARED(Invalid),                  /* 106: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 107: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 108: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 109: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 110: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 111: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 112: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 113: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 114: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 115: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 116: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 117: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 118: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 119: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 120: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 121: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 122: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 123: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 124: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 125: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 126: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 127: ***** I N V A L I D ******/

            vgmSHARED(Invalid),                  /* 128: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 129: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 130: ***** I N V A L I D ******/
            vgmSHARED(Invalid),                  /* 131: ***** I N V A L I D ******/
            vgmUPDATE(gcvVGCMD_ARC),             /* 132: gcvVGCMD_ARC_LINE        */
            vgmSHARED(gcvVGCMD_ARC_REL),         /* 133: gcvVGCMD_ARC_LINE_REL    */
            vgmUPDATE(gcvVGCMD_ARC),             /* 134: gcvVGCMD_ARC_QUAD        */
            vgmSHARED(gcvVGCMD_ARC_REL)          /* 135: gcvVGCMD_ARC_QUAD_REL    */
        };

        * Array = _updateCommand;
        * Count = gcmCOUNTOF(_updateCommand);
    }
    vgmLEAVESUBAPI(vgfGetUpdateArray);
}
