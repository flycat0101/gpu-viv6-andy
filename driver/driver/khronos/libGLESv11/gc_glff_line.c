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


#include "gc_glff_precomp.h"

#define _GC_OBJ_ZONE glvZONE_TRACE

/******************************************************************************\
********************** Individual State Setting Functions **********************
\******************************************************************************/

static GLenum _SetLineWidth(
    glsCONTEXT_PTR Context,
    const GLfloat LineWidth
    )
{
    GLenum result;
    GLfloat lineWidth;

    gcmHEADER_ARG("Context=0x%x LineWidth=%1.4f", Context, LineWidth);

    do
    {
        /* Convert the width value. */
        lineWidth = LineWidth;

        /* Validate the width. */
        if (lineWidth <= glvFLOATZERO)
        {
            result = GL_INVALID_VALUE;
            break;
        }

        /* Make sure the width is within the supported range. */

        if(Context->lineStates.smooth)
        {
            if (lineWidth < Context->smoothLineWidthRange[0])
            {
                lineWidth = glmINT2FLOAT(Context->smoothLineWidthRange[0]);
            }

            if (lineWidth > Context->smoothLineWidthRange[1])
            {
                lineWidth = glmINT2FLOAT(Context->smoothLineWidthRange[1]);
            }

        }
        else
        {
            if (lineWidth < Context->lineWidthRange[0])
            {
                lineWidth = glmINT2FLOAT(Context->lineWidthRange[0]);
            }

            if (lineWidth > Context->lineWidthRange[1])
            {
                lineWidth = glmINT2FLOAT(Context->lineWidthRange[1]);
            }

        }

        /* Set the new value. */
        Context->lineStates.width = lineWidth;

        /* Set the query value. */
        Context->lineStates.queryWidth = lineWidth;

        if (Context->lineWidthRange[1] > 1)
        {
            /* Set the line width for the hardware. */
            glmERR_BREAK(glmTRANSLATEHALSTATUS(gco3D_SetAALineWidth(
                 Context->hw,
                 gcoMATH_Floor(gcoMATH_Add(lineWidth, 0.5f)))));
        }

        result = GL_NO_ERROR;
    }
    while (gcvFALSE);

    gcmFOOTER_ARG("return=0x%04x", result);

    return result;
}


/******************************************************************************\
*********************** Support Functions and Definitions **********************
\******************************************************************************/

/*******************************************************************************
**
**  glfSetDefaultLineStates
**
**  Set default line states.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**  OUTPUT:
**
**      Nothing.
*/

gceSTATUS glfSetDefaultLineStates(
    glsCONTEXT_PTR Context
    )
{
    GLenum result;

    gceSTATUS status;

    gcmHEADER_ARG("Context=0x%x", Context);

    do
    {
        static GLfloat value1 = glvFLOATONE;

        /* Set default hint. */
        Context->lineStates.hint = GL_DONT_CARE;

        if (gcoHAL_IsFeatureAvailable(Context->hal,
                                      gcvFEATURE_WIDE_LINE) == gcvSTATUS_TRUE)
        {
            /* Wide lines are supported. */
            Context->lineWidthRange[0] = 1;
            Context->lineWidthRange[1] = 16;
            Context->smoothLineWidthRange[0] = 1;
            Context->smoothLineWidthRange[1] = 16;

            /* Set the hardware states. */
            glmERR_BREAK(glmTRANSLATEHALSTATUS(gco3D_SetAntiAliasLine(
                Context->hw,
                gcvTRUE)));
        }
        else
        {
            /* Wide lines are supported. */
            Context->lineWidthRange[0] = 1;
            Context->lineWidthRange[1] = 1;
            Context->smoothLineWidthRange[0] = 1;
            Context->smoothLineWidthRange[1] = 1;
        }

        /* Set default line width. */
        glmERR_BREAK(_SetLineWidth(Context, value1));
    }
    while (gcvFALSE);

    status = glmTRANSLATEGLRESULT(result);

    gcmFOOTER();

    return status;
}


/*******************************************************************************
**
**  glfFlushLineStates
**
**  Flush line states.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**  OUTPUT:
**
**      Nothing.
*/

gceSTATUS glfFlushLineStates(
    glsCONTEXT_PTR Context
    )
{
    GLenum result = GL_NO_ERROR;

    gceSTATUS status;

    gcmHEADER_ARG("Context=0x%x", Context);

    do
    {
        if (gcoHAL_IsFeatureAvailable(Context->hal,
                                      gcvFEATURE_WIDE_LINE) == gcvSTATUS_TRUE)
        {
            /* Set the hardware states. */
            glmERR_BREAK(glmTRANSLATEHALSTATUS(gco3D_SetAntiAliasLine(
                Context->hw,
                gcvTRUE)));
        }

        /* Flush line width. */
        glmERR_BREAK(_SetLineWidth(
            Context,
            Context->lineStates.queryWidth
            ));
    }
    while (gcvFALSE);

    status = glmTRANSLATEGLRESULT(result);

    gcmFOOTER();

    return status;
}


/*******************************************************************************
**
**  glfQueryLineState
**
**  Queries line state values.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      Name
**          Specifies the symbolic name of the state to get.
**
**      Type
**          Data format.
**
**  OUTPUT:
**
**      Value
**          Points to the data.
*/

GLboolean glfQueryLineState(
    glsCONTEXT_PTR Context,
    GLenum Name,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLboolean result = GL_TRUE;

    gcmHEADER_ARG("Context=0x%x Name=0x%04x Value=0x%x Type=0x%04x",
                    Context, Name, Value, Type);

    switch (Name)
    {
    case GL_ALIASED_LINE_WIDTH_RANGE:
        glfGetFromIntArray(
            Context->lineWidthRange,
            gcmCOUNTOF(Context->lineWidthRange),
            Value,
            Type
            );
        break;

    case GL_SMOOTH_LINE_WIDTH_RANGE:
        glfGetFromIntArray(
            Context->smoothLineWidthRange,
            gcmCOUNTOF(Context->smoothLineWidthRange),
            Value,
            Type
            );
        break;

    case GL_LINE_SMOOTH:
        glfGetFromInt(
            Context->lineStates.smooth,
            Value,
            Type
            );
        break;

    case GL_LINE_WIDTH:
        glfGetFromFloat(
            Context->lineStates.queryWidth,
            Value,
            Type
            );
        break;

    case GL_LINE_SMOOTH_HINT:
        glfGetFromEnum(
            Context->lineStates.hint,
            Value,
            Type
            );
        break;

    default:
        result = GL_FALSE;
    }

    gcmFOOTER_ARG("return=%d", result);

    /* Return result. */
    return result;
}


/******************************************************************************\
****************************** Line Management Code ****************************
\******************************************************************************/

/*******************************************************************************
**
**  glLineWidth
**
**  glLineWidth specifies the rasterized width of both aliased and antialiased
**  lines. Using a line width other than 1 has different effects, depending on
**  whether line antialiasing is enabled. To enable and disable line
**  antialiasing, call glEnable and glDisable with argument GL_LINE_SMOOTH.
**  Line antialiasing is initially disabled.
**
**  If line antialiasing is disabled, the actual width is determined by rounding
**  the supplied width to the nearest integer. (If the rounding results in the
**  value 0, it is as if the line width were 1.) If | ?x | ? | ?y | , i pixels
**  are filled in each column that is rasterized, where i is the rounded value
**  of width. Otherwise, i pixels are filled in each row that is rasterized.
**
**  If antialiasing is enabled, line rasterization produces a fragment for each
**  pixel square that intersects the region lying within the rectangle having
**  width equal to the current line width, length equal to the actual length
**  of the line, and centered on the mathematical line segment. The coverage
**  value for each fragment is the window coordinate area of the intersection
**  of the rectangular region with the corresponding pixel square. This value
**  is saved and used in the final rasterization step.
**
**  Not all widths can be supported when line antialiasing is enabled. If an
**  unsupported width is requested, the nearest supported width is used. Only
**  width 1 is guaranteed to be supported; others depend on the implementation.
**  Likewise, there is a range for aliased line widths as well. To query the
**  range of supported widths and the size difference between supported widths
**  within the range, call glGetInteger with arguments
**  GL_ALIASED_LINE_WIDTH_RANGE, GL_SMOOTH_LINE_WIDTH_RANGE,
**  GL_SMOOTH_LINE_WIDTH_GRANULARITY.
**
**  INPUT:
**
**      Width
**          Specifies the width of rasterized lines. The initial value is 1.
**
**  OUTPUT:
**
**      Nothing.
*/
#ifdef _GC_OBJ_ZONE
#undef _GC_OBJ_ZONE
#endif
#define _GC_OBJ_ZONE    glvZONE_POLIGON

GL_API void GL_APIENTRY glLineWidth(
    GLfloat Width
    )
{
    glmENTER1(glmARGFLOAT, Width)
    {
        gcmDUMP_API("${ES11 glLineWidth 0x%08X}", *(GLuint*)&Width);

        glmPROFILE(context, GLES1_LINEWIDTH, 0);
        glmERROR(_SetLineWidth(context, Width));
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glLineWidthx(
    GLfixed Width
    )
{
    glmENTER1(glmARGFIXED, Width)
    {
        GLfloat width;

        gcmDUMP_API("${ES11 glLineWidthx 0x%08X}", Width);
        glmPROFILE(context, GLES1_LINEWIDTHX, 0);

        /* Convert to float */
        width = glmFIXED2FLOAT(Width);

        glmERROR(_SetLineWidth(context, width));
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glLineWidthxOES(
    GLfixed Width
    )
{
    glmENTER1(glmARGFIXED, Width)
    {
        GLfloat width;

        gcmDUMP_API("${ES11 glLineWidthxOES 0x%08X}", Width);

        /* Convert to float */
        width = glmFIXED2FLOAT(Width);

        glmERROR(_SetLineWidth(context, width));
    }
    glmLEAVE();
}
