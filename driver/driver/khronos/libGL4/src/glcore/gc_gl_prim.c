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


#ifdef OPENGL40
#include "gc_es_context.h"
#include "g_asmoff.h"
#include "api/gc_gl_api_inline.c"

extern GLvoid __glSwitchToNorVertEntriesFunc(__GLcontext *gc);

/**/
extern GLvoid APIENTRY __glim_End_Material (__GLcontext *gc);

extern GLvoid  __glDrawImmedPrimitive(__GLcontext *gc);
extern GLvoid APIENTRY __glImmedDrawArrays_Normal_V3F(__GLcontext *gc, GLenum mode, GLint first, GLsizei count);
extern GLvoid APIENTRY __glImmedDrawArrays_V3F_Select(__GLcontext *gc, GLenum mode, GLint first, GLsizei count);
extern GLvoid APIENTRY __glImmedDrawArrays_Color_V3F(__GLcontext *gc, GLenum mode, GLint first, GLsizei count);

/*
** Lookup table from input index to maximum element size (number of words).
*/
GLuint input2MaxElementSize[] = {
    4,                                        /* __GL_INPUT_VERTEX      */
    4,                                        /* __GL_INPUT_WEIGHT      */
    3,                                        /* __GL_INPUT_NORMAL      */
    4,                                        /* __GL_INPUT_FDIFFUSE    */
    3,                                        /* __GL_INPUT_FSPECULAR   */
    1,                                        /* __GL_INPUT_FOGCOORD    */
    1,                                        /* __GL_INPUT_EDGEFLAG    */
    0,                                        /* __GL_INPUT_V7          */
    4,                                        /* __GL_INPUT_TEX0        */
    4,                                        /* __GL_INPUT_TEX1        */
    4,                                        /* __GL_INPUT_TEX2        */
    4,                                        /* __GL_INPUT_TEX3        */
    4,                                        /* __GL_INPUT_TEX4        */
    4,                                        /* __GL_INPUT_TEX5        */
    4,                                        /* __GL_INPUT_TEX6        */
    4,                                        /* __GL_INPUT_TEX7        */
    4,                                        /* __GL_INPUT_ATT0        */
    4,                                        /* __GL_INPUT_ATT1        */
    4,                                        /* __GL_INPUT_ATT2        */
    4,                                        /* __GL_INPUT_ATT3        */
    4,                                        /* __GL_INPUT_ATT4        */
    4,                                        /* __GL_INPUT_ATT5        */
    4,                                        /* __GL_INPUT_ATT6        */
    4,                                        /* __GL_INPUT_ATT7        */
    4,                                        /* __GL_INPUT_ATT8        */
    4,                                        /* __GL_INPUT_ATT9        */
    4,                                        /* __GL_INPUT_ATT10       */
    4,                                        /* __GL_INPUT_ATT11       */
    4,                                        /* __GL_INPUT_ATT12       */
    4,                                        /* __GL_INPUT_ATT13       */
    4,                                        /* __GL_INPUT_ATT14       */
    4                                         /* __GL_INPUT_ATT15       */
};

/*
** Lookup table from input index to converted element format in inconsistent cases.
*/
GLuint64 input2InconsistFormat[] = {
    __GL_V4F_BIT,                            /* __GL_INPUT_VERTEX       */
    __GL_WEIGHT_BIT,                         /* __GL_INPUT_WEIGHT       */
    __GL_N3F_BIT,                            /* __GL_INPUT_NORMAL       */
    __GL_C4F_BIT,                            /* __GL_INPUT_FDIFFUSE     */
    __GL_SC3F_BIT,                           /* __GL_INPUT_FSPECULAR    */
    __GL_FOG1F_BIT,                          /* __GL_INPUT_FOGCOORD     */
    __GL_EDGEFLAG_BIT,                       /* __GL_INPUT_EDGEFLAG     */
    0,                                       /* __GL_INPUT_V7           */
    __GL_TC4F_BIT,                           /* __GL_INPUT_TEX0         */
    __GL_TC4F_U1_BIT,                        /* __GL_INPUT_TEX1         */
    __GL_TC4F_U2_BIT,                        /* __GL_INPUT_TEX2         */
    __GL_TC4F_U3_BIT,                        /* __GL_INPUT_TEX3         */
    __GL_TC4F_U4_BIT,                        /* __GL_INPUT_TEX4         */
    __GL_TC4F_U5_BIT,                        /* __GL_INPUT_TEX5         */
    __GL_TC4F_U6_BIT,                        /* __GL_INPUT_TEX6         */
    __GL_TC4F_U7_BIT,                        /* __GL_INPUT_TEX7         */
    __GL_AT4F_I0_BIT,                        /* __GL_INPUT_ATT0         */
    __GL_AT4F_I1_BIT,                        /* __GL_INPUT_ATT1         */
    __GL_AT4F_I2_BIT,                        /* __GL_INPUT_ATT2         */
    __GL_AT4F_I3_BIT,                        /* __GL_INPUT_ATT3         */
    __GL_AT4F_I4_BIT,                        /* __GL_INPUT_ATT4         */
    __GL_AT4F_I5_BIT,                        /* __GL_INPUT_ATT5         */
    __GL_AT4F_I6_BIT,                        /* __GL_INPUT_ATT6         */
    __GL_AT4F_I7_BIT,                        /* __GL_INPUT_ATT7         */
    __GL_AT4F_I8_BIT,                        /* __GL_INPUT_ATT8         */
    __GL_AT4F_I9_BIT,                        /* __GL_INPUT_ATT9         */
    __GL_AT4F_I10_BIT,                       /* __GL_INPUT_ATT10        */
    __GL_AT4F_I11_BIT,                       /* __GL_INPUT_ATT11        */
    __GL_AT4F_I12_BIT,                       /* __GL_INPUT_ATT12        */
    __GL_AT4F_I13_BIT,                       /* __GL_INPUT_ATT13        */
    __GL_AT4F_I14_BIT,                       /* __GL_INPUT_ATT14        */
    __GL_AT4F_I15_BIT                        /* __GL_INPUT_ATT15        */
};

/*
** Lookup table from input index to all possible element formats.
*/
GLuint64 input2VertexFormat[] = {
    __GL_V2F_BIT | __GL_V3F_BIT | __GL_V4F_BIT,                /* __GL_INPUT_VERTEX    */
    __GL_WEIGHT_BIT,                                           /* __GL_INPUT_WEIGHT    */
    __GL_N3F_BIT,                                              /* __GL_INPUT_NORMAL    */
    __GL_C3F_BIT | __GL_C4F_BIT | __GL_C4UB_BIT,               /* __GL_INPUT_FDIFFUSE  */
    __GL_SC3F_BIT,                                             /* __GL_INPUT_FSPECULAR */
    __GL_FOG1F_BIT,                                            /* __GL_INPUT_FOGCOORD  */
    __GL_EDGEFLAG_BIT,                                         /* __GL_INPUT_EDGEFLAG  */
    0,                                                         /* __GL_INPUT_V7        */
    __GL_TC2F_BIT | __GL_TC3F_BIT | __GL_TC4F_BIT,             /* __GL_INPUT_TEX0      */
    __GL_TC2F_U1_BIT | __GL_TC3F_U1_BIT | __GL_TC4F_U1_BIT,    /* __GL_INPUT_TEX1      */
    __GL_TC2F_U2_BIT | __GL_TC3F_U2_BIT | __GL_TC4F_U2_BIT,    /* __GL_INPUT_TEX2      */
    __GL_TC2F_U3_BIT | __GL_TC3F_U3_BIT | __GL_TC4F_U3_BIT,    /* __GL_INPUT_TEX3      */
    __GL_TC2F_U4_BIT | __GL_TC3F_U4_BIT | __GL_TC4F_U4_BIT,    /* __GL_INPUT_TEX4      */
    __GL_TC2F_U5_BIT | __GL_TC3F_U5_BIT | __GL_TC4F_U5_BIT,    /* __GL_INPUT_TEX5      */
    __GL_TC2F_U6_BIT | __GL_TC3F_U6_BIT | __GL_TC4F_U6_BIT,    /* __GL_INPUT_TEX6      */
    __GL_TC2F_U7_BIT | __GL_TC3F_U7_BIT | __GL_TC4F_U7_BIT,    /* __GL_INPUT_TEX7      */
    __GL_AT4F_I0_BIT,                                          /* __GL_INPUT_ATT0      */
    __GL_AT4F_I1_BIT,                                          /* __GL_INPUT_ATT1      */
    __GL_AT4F_I2_BIT,                                          /* __GL_INPUT_ATT2      */
    __GL_AT4F_I3_BIT,                                          /* __GL_INPUT_ATT3      */
    __GL_AT4F_I4_BIT,                                          /* __GL_INPUT_ATT4      */
    __GL_AT4F_I5_BIT,                                          /* __GL_INPUT_ATT5      */
    __GL_AT4F_I6_BIT,                                          /* __GL_INPUT_ATT6      */
    __GL_AT4F_I7_BIT,                                          /* __GL_INPUT_ATT7      */
    __GL_AT4F_I8_BIT,                                          /* __GL_INPUT_ATT8      */
    __GL_AT4F_I9_BIT,                                          /* __GL_INPUT_ATT9      */
    __GL_AT4F_I10_BIT,                                         /* __GL_INPUT_ATT10     */
    __GL_AT4F_I11_BIT,                                         /* __GL_INPUT_ATT11     */
    __GL_AT4F_I12_BIT,                                         /* __GL_INPUT_ATT12     */
    __GL_AT4F_I13_BIT,                                         /* __GL_INPUT_ATT13     */
    __GL_AT4F_I14_BIT,                                         /* __GL_INPUT_ATT14     */
    __GL_AT4F_I15_BIT                                          /* __GL_INPUT_ATT15     */
};

/*
** Lookup table from vertex format index to input index.
*/
GLuint fmtIndex2InputIndex[] = {
    __GL_INPUT_VERTEX_INDEX,                  /* __GL_V2F_INDEX        */
    __GL_INPUT_VERTEX_INDEX,                  /* __GL_V3F_INDEX        */
    __GL_INPUT_VERTEX_INDEX,                  /* __GL_V4F_INDEX        */
    __GL_INPUT_DIFFUSE_INDEX,                 /* __GL_C3F_INDEX        */
    __GL_INPUT_DIFFUSE_INDEX,                 /* __GL_C4F_INDEX        */
    __GL_INPUT_DIFFUSE_INDEX,                 /* __GL_C4UB_INDEX       */
    __GL_INPUT_NORMAL_INDEX,                  /* __GL_N3F_INDEX        */
    __GL_INPUT_TEX0_INDEX,                    /* __GL_TC2F_INDEX       */
    __GL_INPUT_TEX1_INDEX,                    /* __GL_TC2F_U1_INDEX    */
    __GL_INPUT_TEX2_INDEX,                    /* __GL_TC2F_U2_INDEX    */
    __GL_INPUT_TEX3_INDEX,                    /* __GL_TC2F_U3_INDEX    */
    __GL_INPUT_TEX4_INDEX,                    /* __GL_TC2F_U4_INDEX    */
    __GL_INPUT_TEX5_INDEX,                    /* __GL_TC2F_U5_INDEX    */
    __GL_INPUT_TEX6_INDEX,                    /* __GL_TC2F_U6_INDEX    */
    __GL_INPUT_TEX7_INDEX,                    /* __GL_TC2F_U7_INDEX    */
    __GL_INPUT_TEX0_INDEX,                    /* __GL_TC3F_INDEX       */
    __GL_INPUT_TEX1_INDEX,                    /* __GL_TC3F_U1_INDEX    */
    __GL_INPUT_TEX2_INDEX,                    /* __GL_TC3F_U2_INDEX    */
    __GL_INPUT_TEX3_INDEX,                    /* __GL_TC3F_U3_INDEX    */
    __GL_INPUT_TEX4_INDEX,                    /* __GL_TC3F_U4_INDEX    */
    __GL_INPUT_TEX5_INDEX,                    /* __GL_TC3F_U5_INDEX    */
    __GL_INPUT_TEX6_INDEX,                    /* __GL_TC3F_U6_INDEX    */
    __GL_INPUT_TEX7_INDEX,                    /* __GL_TC3F_U7_INDEX    */
    __GL_INPUT_TEX0_INDEX,                    /* __GL_TC4F_INDEX       */
    __GL_INPUT_TEX1_INDEX,                    /* __GL_TC4F_U1_INDEX    */
    __GL_INPUT_TEX2_INDEX,                    /* __GL_TC4F_U2_INDEX    */
    __GL_INPUT_TEX3_INDEX,                    /* __GL_TC4F_U3_INDEX    */
    __GL_INPUT_TEX4_INDEX,                    /* __GL_TC4F_U4_INDEX    */
    __GL_INPUT_TEX5_INDEX,                    /* __GL_TC4F_U5_INDEX    */
    __GL_INPUT_TEX6_INDEX,                    /* __GL_TC4F_U6_INDEX    */
    __GL_INPUT_TEX7_INDEX,                    /* __GL_TC4F_U7_INDEX    */
    __GL_INPUT_EDGEFLAG_INDEX,                /* __GL_EDGEFLAG_INDEX   */
    __GL_INPUT_SPECULAR_INDEX,                /* __GL_SC3F_INDEX       */
    __GL_INPUT_FOGCOORD_INDEX,                /* __GL_FOGC_INDEX       */
    __GL_INPUT_ATT0_INDEX,                    /* __GL_AT4F_I0_INDEX    */
    __GL_INPUT_ATT1_INDEX,                    /* __GL_AT4F_I1_INDEX    */
    __GL_INPUT_ATT2_INDEX,                    /* __GL_AT4F_I2_INDEX    */
    __GL_INPUT_ATT3_INDEX,                    /* __GL_AT4F_I3_INDEX    */
    __GL_INPUT_ATT4_INDEX,                    /* __GL_AT4F_I4_INDEX    */
    __GL_INPUT_ATT5_INDEX,                    /* __GL_AT4F_I5_INDEX    */
    __GL_INPUT_ATT6_INDEX,                    /* __GL_AT4F_I6_INDEX    */
    __GL_INPUT_ATT7_INDEX,                    /* __GL_AT4F_I7_INDEX    */
    __GL_INPUT_ATT8_INDEX,                    /* __GL_AT4F_I8_INDEX    */
    __GL_INPUT_ATT9_INDEX,                    /* __GL_AT4F_I9_INDEX    */
    __GL_INPUT_ATT10_INDEX,                   /* __GL_AT4F_I10_INDEX   */
    __GL_INPUT_ATT11_INDEX,                   /* __GL_AT4F_I11_INDEX   */
    __GL_INPUT_ATT12_INDEX,                   /* __GL_AT4F_I12_INDEX   */
    __GL_INPUT_ATT13_INDEX,                   /* __GL_AT4F_I13_INDEX   */
    __GL_INPUT_ATT14_INDEX,                   /* __GL_AT4F_I14_INDEX   */
    __GL_INPUT_ATT15_INDEX,                   /* __GL_AT4F_I15_INDEX   */
    __GL_INPUT_WEIGHT_INDEX                   /* __GL_WEIGHT_INDEX     */
};

/*
** Lookup table from vertex format index to element DW size.
*/
GLuint fmtIndex2DWSize[] = {
    2,                    /* __GL_V2F_INDEX        */
    3,                    /* __GL_V3F_INDEX        */
    4,                    /* __GL_V4F_INDEX        */
    3,                    /* __GL_C3F_INDEX        */
    4,                    /* __GL_C4F_INDEX        */
    1,                    /* __GL_C4UB_INDEX       */
    3,                    /* __GL_N3F_INDEX        */
    2,                    /* __GL_TC2F_INDEX       */
    2,                    /* __GL_TC2F_U1_INDEX    */
    2,                    /* __GL_TC2F_U2_INDEX    */
    2,                    /* __GL_TC2F_U3_INDEX    */
    2,                    /* __GL_TC2F_U4_INDEX    */
    2,                    /* __GL_TC2F_U5_INDEX    */
    2,                    /* __GL_TC2F_U6_INDEX    */
    2,                    /* __GL_TC2F_U7_INDEX    */
    3,                    /* __GL_TC3F_INDEX       */
    3,                    /* __GL_TC3F_U1_INDEX    */
    3,                    /* __GL_TC3F_U2_INDEX    */
    3,                    /* __GL_TC3F_U3_INDEX    */
    3,                    /* __GL_TC3F_U4_INDEX    */
    3,                    /* __GL_TC3F_U5_INDEX    */
    3,                    /* __GL_TC3F_U6_INDEX    */
    3,                    /* __GL_TC3F_U7_INDEX    */
    4,                    /* __GL_TC4F_INDEX       */
    4,                    /* __GL_TC4F_U1_INDEX    */
    4,                    /* __GL_TC4F_U2_INDEX    */
    4,                    /* __GL_TC4F_U3_INDEX    */
    4,                    /* __GL_TC4F_U4_INDEX    */
    4,                    /* __GL_TC4F_U5_INDEX    */
    4,                    /* __GL_TC4F_U6_INDEX    */
    4,                    /* __GL_TC4F_U7_INDEX    */
    0,                    /* __GL_EDGEFLAG_INDEX   */
    3,                    /* __GL_SC3F_INDEX       */
    1,                    /* __GL_FOGC_INDEX       */
    4,                    /* __GL_AT4F_I0_INDEX    */
    4,                    /* __GL_AT4F_I1_INDEX    */
    4,                    /* __GL_AT4F_I2_INDEX    */
    4,                    /* __GL_AT4F_I3_INDEX    */
    4,                    /* __GL_AT4F_I4_INDEX    */
    4,                    /* __GL_AT4F_I5_INDEX    */
    4,                    /* __GL_AT4F_I6_INDEX    */
    4,                    /* __GL_AT4F_I7_INDEX    */
    4,                    /* __GL_AT4F_I8_INDEX    */
    4,                    /* __GL_AT4F_I9_INDEX    */
    4,                    /* __GL_AT4F_I10_INDEX   */
    4,                    /* __GL_AT4F_I11_INDEX   */
    4,                    /* __GL_AT4F_I12_INDEX   */
    4,                    /* __GL_AT4F_I13_INDEX   */
    4,                    /* __GL_AT4F_I14_INDEX   */
    4,                    /* __GL_AT4F_I15_INDEX   */
    0                     /* __GL_WEIGHT_INDEX     */
};

/*
** Minimum vertex numbers for each primitive type.
*/
GLsizei minVertexNumber[] =
{
    1,                                        /* GL_POINTS */
    2,                                        /* GL_LINES */
    2,                                        /* GL_LINE_LOOP */
    2,                                        /* GL_LINE_STRIP */
    3,                                        /* GL_TRIANGLES */
    3,                                        /* GL_TRIANGLE_STRIP */
    3,                                        /* GL_TRIANGLE_FAN */
    4,                                        /* GL_QUADS */
    4,                                        /* GL_QUAD_STRIP */
    3,                                        /* GL_POLYGON */
    4,                                        /* GL_LINES_ADJACENCY_EXT */
    4,                                        /* GL_LINE_STRIP_ADJACENCY_EXT */
    6,                                        /* GL_TRIANGLES_ADJACENCY_EXT */
    6,                                        /* GL_TRIANGLE_STRIP_ADJACENCY_EXT */
};

/*
** edgeflag mask for each primitive type.
*/
GLuint edgeFlagInputMask[] =
{
    ~((GLuint)__GL_INPUT_EDGEFLAG),                     /* GL_POINTS             */
    ~((GLuint)__GL_INPUT_EDGEFLAG),                     /* GL_LINES              */
    ~((GLuint)__GL_INPUT_EDGEFLAG),                     /* GL_LINE_LOOP          */
    ~((GLuint)__GL_INPUT_EDGEFLAG),                     /* GL_LINE_STRIP         */
    ~((GLuint)0),                                       /* GL_TRIANGLES          */
    ~((GLuint)__GL_INPUT_EDGEFLAG),                     /* GL_TRIANGLE_STRIP     */
    ~((GLuint)__GL_INPUT_EDGEFLAG),                     /* GL_TRIANGLE_FAN       */
    ~((GLuint)0),                                       /* GL_QUADS              */
    ~((GLuint)__GL_INPUT_EDGEFLAG),                     /* GL_QUAD_STRIP         */
    ~((GLuint)0),                                       /* GL_POLYGON            */
    ~((GLuint)__GL_INPUT_EDGEFLAG),                     /* GL_LINES_ADJACENCY_EXT */
    ~((GLuint)__GL_INPUT_EDGEFLAG),                     /* GL_LINE_STRIP_ADJACENCY_EXT */
    ~((GLuint)0),                                       /* GL_TRIANGLES_ADJACENCY_EXT */
    ~((GLuint)__GL_INPUT_EDGEFLAG),                     /* GL_TRIANGLE_STRIP_ADJACENCY_EXT */
};

/*
** Internal batch end flag for each primitive type.
*/
GLboolean internalEndTable[] =
{
    GL_FALSE,                                 /* GL_POINTS             */
    GL_FALSE,                                 /* GL_LINES              */
    GL_TRUE,                                  /* GL_LINE_LOOP          */
    GL_TRUE,                                  /* GL_LINE_STRIP         */
    GL_FALSE,                                 /* GL_TRIANGLES          */
    GL_TRUE,                                  /* GL_TRIANGLE_STRIP     */
    GL_TRUE,                                  /* GL_TRIANGLE_FAN       */
    GL_FALSE,                                 /* GL_QUADS              */
    GL_TRUE,                                  /* GL_QUAD_STRIP         */
    GL_TRUE,                                  /* GL_POLYGON            */
    GL_TRUE,                                  /* GL_LINES_ADJACENCY_EXT */
    GL_TRUE,                                  /* GL_LINE_STRIP_ADJACENCY_EXT */
    GL_TRUE,                                  /* GL_TRIANGLES_ADJACENCY_EXT */
    GL_TRUE,                                  /* GL_TRIANGLE_STRIP_ADJACENCY_EXT */
};

/*
** Lookup Table inputTag = inputTagTable[__GL_INPUT_*_INDEX][sizeDW-1]
*/
GLenum inputTagTable[32][4] =
{
    {0, __GL_V2F_TAG, __GL_V3F_TAG, __GL_V4F_TAG},      /* __GL_INPUT_VERTEX_INDEX */
    {0, 0, 0, 0},                                       /* __GL_INPUT_WEIGHT_INDEX */
    {0, 0, __GL_N3F_TAG, 0},                            /* __GL_INPUT_NORMAL_INDEX */
    {__GL_C4UB_TAG, 0, __GL_C3F_TAG, __GL_C4F_TAG},     /* __GL_INPUT_DIFFUSE_INDEX */
    {0, 0, __GL_SC3F_TAG, 0},                           /* __GL_INPUT_SPECULAR_INDEX */
    {__GL_FOG1F_TAG, 0, 0, 0},                          /* __GL_INPUT_FOGCOORD_INDEX */
    {0, 0, 0, 0},                                       /* __GL_INPUT_EDGEFLAG_INDEX */
    {0, 0, 0, 0},                                       /* __GL_INPUT_V7_INDEX */
    {0, __GL_TC2F_TAG, __GL_TC3F_TAG, __GL_TC4F_TAG},   /* __GL_INPUT_TEX0_INDEX */
    {0, __GL_TC2F_U1_TAG, __GL_TC3F_U1_TAG, __GL_TC4F_U1_TAG}, /* __GL_INPUT_TEX1_INDEX */
    {0, __GL_TC2F_U2_TAG, __GL_TC3F_U2_TAG, __GL_TC4F_U2_TAG}, /* __GL_INPUT_TEX2_INDEX */
    {0, __GL_TC2F_U3_TAG, __GL_TC3F_U3_TAG, __GL_TC4F_U3_TAG}, /* __GL_INPUT_TEX3_INDEX */
    {0, __GL_TC2F_U4_TAG, __GL_TC3F_U4_TAG, __GL_TC4F_U4_TAG}, /* __GL_INPUT_TEX4_INDEX */
    {0, __GL_TC2F_U5_TAG, __GL_TC3F_U5_TAG, __GL_TC4F_U5_TAG}, /* __GL_INPUT_TEX5_INDEX */
    {0, __GL_TC2F_U6_TAG, __GL_TC3F_U6_TAG, __GL_TC4F_U6_TAG}, /* __GL_INPUT_TEX6_INDEX */
    {0, __GL_TC2F_U7_TAG, __GL_TC3F_U7_TAG, __GL_TC4F_U7_TAG}, /* __GL_INPUT_TEX7_INDEX */
    {0, 0, 0, __GL_AT4F_I0_TAG},                        /* __GL_INPUT_ATT0_INDEX */
    {0, 0, 0, __GL_AT4F_I1_TAG},                        /* __GL_INPUT_ATT1_INDEX */
    {0, 0, 0, __GL_AT4F_I2_TAG},                        /* __GL_INPUT_ATT2_INDEX */
    {0, 0, 0, __GL_AT4F_I3_TAG},                        /* __GL_INPUT_ATT3_INDEX */
    {0, 0, 0, __GL_AT4F_I4_TAG},                        /* __GL_INPUT_ATT4_INDEX */
    {0, 0, 0, __GL_AT4F_I5_TAG},                        /* __GL_INPUT_ATT5_INDEX */
    {0, 0, 0, __GL_AT4F_I6_TAG},                        /* __GL_INPUT_ATT6_INDEX */
    {0, 0, 0, __GL_AT4F_I7_TAG},                        /* __GL_INPUT_ATT7_INDEX */
    {0, 0, 0, __GL_AT4F_I8_TAG},                        /* __GL_INPUT_ATT8_INDEX */
    {0, 0, 0, __GL_AT4F_I9_TAG},                        /* __GL_INPUT_ATT9_INDEX */
    {0, 0, 0, __GL_AT4F_I10_TAG},                       /* __GL_INPUT_ATT10_INDEX */
    {0, 0, 0, __GL_AT4F_I11_TAG},                       /* __GL_INPUT_ATT11_INDEX */
    {0, 0, 0, __GL_AT4F_I12_TAG},                       /* __GL_INPUT_ATT12_INDEX */
    {0, 0, 0, __GL_AT4F_I13_TAG},                       /* __GL_INPUT_ATT13_INDEX */
    {0, 0, 0, __GL_AT4F_I14_TAG},                       /* __GL_INPUT_ATT14_INDEX */
    {0, 0, 0, __GL_AT4F_I15_TAG},                       /* __GL_INPUT_ATT15_INDEX */
};

GLvoid __glSwitchImmediateDispatch(__GLcontext *gc, __GLdispatchTable *pDispatch)
{
    gc->currentImmediateDispatch = pDispatch;
    if (gc->dlist.mode == 0)
    {
        gc->pModeDispatch = pDispatch;
        if (!gc->apiProfile)
        {
            gc->pEntryDispatch = gc->pModeDispatch;
        }
    }
}

GLvoid __glValidateImmedBegin(__GLcontext *gc, GLenum mode)
{
    /* Compute the required primitive input mask */
    if (gc->input.inputMaskChanged)
    {
        __glComputeRequiredInputMask(gc);
        gc->input.inputMaskChanged = GL_FALSE;
    }
    gc->input.requiredInputMask = gc->input.currentInputMask & edgeFlagInputMask[mode];

    /* Enable indexed primitive only for joint primitives in GL_FILL mode */
    gc->input.indexPrimEnabled = GL_FALSE;

    switch (mode)
    {
    case GL_LINE_STRIP:
    case GL_LINE_LOOP:
        gc->input.indexPrimEnabled = GL_TRUE;
        break;
    case GL_TRIANGLE_STRIP:
    case GL_TRIANGLE_FAN:
    case GL_QUAD_STRIP:
    case GL_POLYGON:
        if (gc->state.polygon.bothFaceFill)
        {
            gc->input.indexPrimEnabled = GL_TRUE;
        }
        break;
    }
}

__GL_INLINE GLuint64 __glInputMask2InconsisFormat(GLuint64 mask)
{
    GLuint i = 0;
    GLuint64 formatMask = 0;

    while (mask) {
        if (mask & 0x1) {
            formatMask |= input2InconsistFormat[i];
        }
        i += 1;
        mask >>= 1;
    }

    return formatMask;
}

__GL_INLINE GLuint __glVertexFormat2InputMask(GLuint64 formatMask)
{
    GLuint inputMask = 0;
    GLuint i;

    /* Convert vertex format mask to input mask */
    i = 0;
    while (formatMask) {
        if (formatMask & 0x1) {
            inputMask |= (1 << fmtIndex2InputIndex[i]);
        }
        formatMask >>= 1;
        i += 1;
    }

    return inputMask;
}

GLvoid __glComputePrimitiveData(__GLcontext *gc)
{
    GLuint i, vE;
    GLuint64 inputMask;

    if (gc->input.inconsistentFormat)
    {
        inputMask = gc->input.requiredInputMask;
    }
    else
    {
        inputMask = __glVertexFormat2InputMask(gc->input.primitiveFormat);
        if (inputMask & __GL_INPUT_EDGEFLAG)
        {
            gc->input.edgeflag.index = gc->input.vertex.index;
        }
    }
    gc->input.primInputMask = inputMask;

    /* Setup stride for the input elements and compute the number of elements */
    inputMask &= ~__GL_INPUT_EDGEFLAG;
    i = 0;
    vE = 0;
    while (inputMask)
    {
        if (inputMask & 0x1) {
            vE += 1;
        }
        inputMask >>= 1;
        i += 1;
    }
    gc->input.numberOfElements = vE;
}

__GL_INLINE void __glRestoreVertexInputMachine(__GLcontext *gc, __GLvertexDataCache *vtxCache, GLint index)
{
    GLuint mask, i;

    gc->input.primMode = vtxCache->primMode;
    gc->input.connectPrimMode = vtxCache->connectPrimMode;
    gc->input.vertTotalStrideDW = vtxCache->vertTotalStrideDW;
    gc->input.primitiveFormat = vtxCache->primitiveFormat;
    gc->input.primElemSequence = vtxCache->primElemSequence;
    gc->input.primInputMask = vtxCache->primInputMask;
    gc->input.numberOfElements = vtxCache->numberOfElements;
    gc->input.indexPrimEnabled = vtxCache->indexPrimEnabled;

    i = 0;
    mask = gc->input.primInputMask & (~__GL_INPUT_EDGEFLAG);
    while (mask)
    {
        if (mask & 0x1)
        {
            gc->input.currentInput[i].offsetDW = vtxCache->elemOffsetDW[i];
            gc->input.currentInput[i].sizeDW = vtxCache->elemSizeDW[i];
            gc->input.currentInput[i].pointer =
                (GLubyte *)(gc->input.defaultDataBuffer + gc->input.currentInput[i].offsetDW);
            gc->input.currentInput[i].currentPtrDW =
                (GLfloat *)gc->input.currentInput[i].pointer + gc->input.vertTotalStrideDW * index;
        }
        mask >>= 1;
        i += 1;
    }
}

void __glComputeCacheBufVertexCount(__GLcontext *gc)
{
    GLuint vertexCount, vertexIndex, indexCount, lastVtxIndex;
    GLuint currentBeginTag;
    __GLvertexInfo *vtxinfo;
    GLuint64 vertexFormat;

    /* Compute vertexCount and indexCount from gc->input.vertexInfoBuffer */
    vtxinfo = gc->input.vertexInfoBuffer;
    if ((vtxinfo->inputTag & __GL_VERTEX_DATA_TAG_MASK) || (vtxinfo->inputTag == __GL_END_TAG))
    {
        /* For continued primitive batch that start with a vertex data tag or glEnd tag */
        vertexIndex = vtxinfo->offsetDW / gc->input.vertTotalStrideDW;
        lastVtxIndex = indexCount = 0;
        vertexFormat = 0;
        currentBeginTag = (gc->input.connectPrimMode | 0x10);
    }
    else
    {
        /* For primitive batch that start with a __GL_BEGIN_*_TAG */
        vertexIndex = lastVtxIndex = indexCount = 0;
        vertexFormat = 0;
        currentBeginTag = 0;
    }

    while (vtxinfo < gc->input.currentInfoBufPtr)
    {
        switch (vtxinfo->inputTag) {
        case __GL_BEGIN_TRIANGLES_TAG:
        case __GL_BEGIN_TRIANGLE_STRIP_TAG:
        case __GL_BEGIN_TRIANGLE_FAN_TAG:
        case __GL_BEGIN_POLYGON_TAG:
        case __GL_BEGIN_QUADS_TAG:
        case __GL_BEGIN_QUAD_STRIP_TAG:
        case __GL_BEGIN_LINES_TAG:
        case __GL_BEGIN_LINE_STRIP_TAG:
        case __GL_BEGIN_LINE_LOOP_TAG:
        case __GL_BEGIN_POINTS_TAG:
            currentBeginTag = vtxinfo->inputTag;
            break;
        case __GL_END_TAG:
            vertexCount = vertexIndex - lastVtxIndex;
            lastVtxIndex = vertexIndex;
            if (vertexCount) {
                switch (currentBeginTag) {
                  case __GL_BEGIN_TRIANGLES_TAG:
                    indexCount += vertexCount;
                    break;
                  case __GL_BEGIN_TRIANGLE_STRIP_TAG:
                  case __GL_BEGIN_TRIANGLE_FAN_TAG:
                  case __GL_BEGIN_POLYGON_TAG:
                    indexCount += (vertexCount - 2) * 3;
                    break;
                  case __GL_BEGIN_QUADS_TAG:
                    indexCount += (vertexCount >> 1) * 3;
                    break;
                  case __GL_BEGIN_QUAD_STRIP_TAG:
                    indexCount += (vertexCount - 2) * 3;
                    break;
                  case __GL_BEGIN_LINES_TAG:
                    indexCount += vertexCount;
                    break;
                  case __GL_BEGIN_LINE_STRIP_TAG:
                    indexCount += (vertexCount - 1) * 2;
                    break;
                  case __GL_BEGIN_LINE_LOOP_TAG:
                    indexCount += vertexCount * 2;
                    break;
                  case __GL_BEGIN_POINTS_TAG:
                    indexCount += vertexCount;
                    break;

                  default:
                    GL_ASSERT(0);
                    break;
                }
            }
            break;
        case __GL_V2F_TAG:
            vertexFormat = 0;
            vertexIndex++;
            break;
        case __GL_V3F_TAG:
            vertexFormat = 0;
            vertexIndex++;
            break;
        case __GL_V4F_TAG:
            vertexFormat = 0;
            vertexIndex++;
            break;
        case __GL_N3F_V3F_TAG:
            vertexFormat = 0;
            vertexIndex++;
            break;
        case __GL_C4F_V3F_TAG:
            vertexFormat = 0;
            vertexIndex++;
            break;
        case __GL_C3F_TAG:
            vertexFormat |= __GL_C3F_BIT;
            break;
        case __GL_C4F_TAG:
            vertexFormat |= __GL_C4F_BIT;
            break;
        case __GL_C4UB_TAG:
            vertexFormat |= __GL_C4UB_BIT;
            break;
        case __GL_N3F_TAG:
            vertexFormat |= __GL_N3F_BIT;
            break;
        case __GL_TC2F_TAG:
            vertexFormat |= __GL_TC2F_BIT;
            break;
        case __GL_TC2F_U1_TAG:
            vertexFormat |= __GL_TC2F_U1_BIT;
            break;
        case __GL_TC2F_U2_TAG:
            vertexFormat |= __GL_TC2F_U2_BIT;
            break;
        case __GL_TC2F_U3_TAG:
            vertexFormat |= __GL_TC2F_U3_BIT;
            break;
        case __GL_TC2F_U4_TAG:
            vertexFormat |= __GL_TC2F_U4_BIT;
            break;
        case __GL_TC2F_U5_TAG:
            vertexFormat |= __GL_TC2F_U5_BIT;
            break;
        case __GL_TC2F_U6_TAG:
            vertexFormat |= __GL_TC2F_U6_BIT;
            break;
        case __GL_TC2F_U7_TAG:
            vertexFormat |= __GL_TC2F_U7_BIT;
            break;
        case __GL_TC3F_TAG:
            vertexFormat |= __GL_TC3F_BIT;
            break;
        case __GL_TC3F_U1_TAG:
            vertexFormat |= __GL_TC3F_U1_BIT;
            break;
        case __GL_TC3F_U2_TAG:
            vertexFormat |= __GL_TC3F_U2_BIT;
            break;
        case __GL_TC3F_U3_TAG:
            vertexFormat |= __GL_TC3F_U3_BIT;
            break;
        case __GL_TC3F_U4_TAG:
            vertexFormat |= __GL_TC3F_U4_BIT;
            break;
        case __GL_TC3F_U5_TAG:
            vertexFormat |= __GL_TC3F_U5_BIT;
            break;
        case __GL_TC3F_U6_TAG:
            vertexFormat |= __GL_TC3F_U6_BIT;
            break;
        case __GL_TC3F_U7_TAG:
            vertexFormat |= __GL_TC3F_U7_BIT;
            break;
        case __GL_TC4F_TAG:
            vertexFormat |= __GL_TC4F_BIT;
            break;
        case __GL_TC4F_U1_TAG:
            vertexFormat |= __GL_TC4F_U1_BIT;
            break;
        case __GL_TC4F_U2_TAG:
            vertexFormat |= __GL_TC4F_U2_BIT;
            break;
        case __GL_TC4F_U3_TAG:
            vertexFormat |= __GL_TC4F_U3_BIT;
            break;
        case __GL_TC4F_U4_TAG:
            vertexFormat |= __GL_TC4F_U4_BIT;
            break;
        case __GL_TC4F_U5_TAG:
            vertexFormat |= __GL_TC4F_U5_BIT;
            break;
        case __GL_TC4F_U6_TAG:
            vertexFormat |= __GL_TC4F_U6_BIT;
            break;
        case __GL_TC4F_U7_TAG:
            vertexFormat |= __GL_TC4F_U7_BIT;
            break;
        case __GL_EDGEFLAG_TAG:
            vertexFormat |= __GL_EDGEFLAG_BIT;
            break;
        case __GL_SC3F_TAG:
            vertexFormat |= __GL_SC3F_BIT;
            break;
        case __GL_FOG1F_TAG:
            vertexFormat |= __GL_FOG1F_BIT;
            break;
        case __GL_AT4F_I0_TAG:
            vertexFormat |= __GL_AT4F_I0_BIT;
            break;
        case __GL_AT4F_I1_TAG:
            vertexFormat |= __GL_AT4F_I1_BIT;
            break;
        case __GL_AT4F_I2_TAG:
            vertexFormat |= __GL_AT4F_I2_BIT;
            break;
        case __GL_AT4F_I3_TAG:
            vertexFormat |= __GL_AT4F_I3_BIT;
            break;
        case __GL_AT4F_I4_TAG:
            vertexFormat |= __GL_AT4F_I4_BIT;
            break;
        case __GL_AT4F_I5_TAG:
            vertexFormat |= __GL_AT4F_I5_BIT;
            break;
        case __GL_AT4F_I6_TAG:
            vertexFormat |= __GL_AT4F_I6_BIT;
            break;
        case __GL_AT4F_I7_TAG:
            vertexFormat |= __GL_AT4F_I7_BIT;
            break;
        case __GL_AT4F_I8_TAG:
            vertexFormat |= __GL_AT4F_I8_BIT;
            break;
        case __GL_AT4F_I9_TAG:
            vertexFormat |= __GL_AT4F_I9_BIT;
            break;
        case __GL_AT4F_I10_TAG:
            vertexFormat |= __GL_AT4F_I10_BIT;
            break;
        case __GL_AT4F_I11_TAG:
            vertexFormat |= __GL_AT4F_I11_BIT;
            break;
        case __GL_AT4F_I12_TAG:
            vertexFormat |= __GL_AT4F_I12_BIT;
            break;
        case __GL_AT4F_I13_TAG:
            vertexFormat |= __GL_AT4F_I13_BIT;
            break;
        case __GL_AT4F_I14_TAG:
            vertexFormat |= __GL_AT4F_I14_BIT;
            break;
        case __GL_AT4F_I15_TAG:
            vertexFormat |= __GL_AT4F_I15_BIT;
            break;
        case __GL_DRAWARRAYS_POINTS_TAG:
        case __GL_DRAWARRAYS_LINES_TAG:
        case __GL_DRAWARRAYS_LINE_LOOP_TAG:
        case __GL_DRAWARRAYS_LINE_STRIP_TAG:
        case __GL_DRAWARRAYS_TRIANGLES_TAG:
        case __GL_DRAWARRAYS_TRIANGLE_STRIP_TAG:
        case __GL_DRAWARRAYS_TRIANGLE_FAN_TAG:
        case __GL_DRAWARRAYS_QUADS_TAG:
        case __GL_DRAWARRAYS_QUAD_STRIP_TAG:
        case __GL_DRAWARRAYS_POLYGON_TAG:
            currentBeginTag = vtxinfo->inputTag;
            vertexIndex += (GLuint)vtxinfo->count;
            break;
        case __GL_DRAWARRAYS_END_TAG:
            vertexCount = vertexIndex - lastVtxIndex;
            lastVtxIndex = vertexIndex;
            if (vertexCount) {
                switch (currentBeginTag) {
                  case __GL_DRAWARRAYS_TRIANGLES_TAG:
                    indexCount += vertexCount;
                    break;
                  case __GL_DRAWARRAYS_TRIANGLE_STRIP_TAG:
                  case __GL_DRAWARRAYS_TRIANGLE_FAN_TAG:
                  case __GL_DRAWARRAYS_POLYGON_TAG:
                    indexCount += (vertexCount - 2) * 3;
                    break;
                  case __GL_DRAWARRAYS_QUADS_TAG:
                    indexCount += (vertexCount >> 1) * 3;
                    break;
                  case __GL_DRAWARRAYS_QUAD_STRIP_TAG:
                    indexCount += (vertexCount - 2) * 3;
                    break;
                  case __GL_DRAWARRAYS_LINES_TAG:
                    indexCount += vertexCount;
                    break;
                  case __GL_DRAWARRAYS_LINE_STRIP_TAG:
                    indexCount += (vertexCount - 1) * 2;
                    break;
                  case __GL_DRAWARRAYS_LINE_LOOP_TAG:
                    indexCount += vertexCount * 2;
                    break;
                  case __GL_DRAWARRAYS_POINTS_TAG:
                    indexCount += vertexCount;
                    break;

                  default:
                    GL_ASSERT(0);
                    break;
                }
            }
            break;
        case __GL_ARRAY_V2F_TAG:
        case __GL_ARRAY_V3F_TAG:
        case __GL_ARRAY_V4F_TAG:
        case __GL_ARRAY_C3F_TAG:
        case __GL_ARRAY_C4F_TAG:
        case __GL_ARRAY_C4UB_TAG:
        case __GL_ARRAY_N3F_TAG:
        case __GL_ARRAY_TC2F_TAG:
        case __GL_ARRAY_TC3F_TAG:
        case __GL_ARRAY_TC4F_TAG:
        case __GL_ARRAY_N3F_V3F_TAG:
        case __GL_ARRAY_C4F_V3F_TAG:
        case __GL_ARRAY_INDEX_TAG:
        case __GL_BATCH_END_TAG:
            break;
        default:
            GL_ASSERT(0);
            break;
        }

        vtxinfo++;
    }
    gc->input.vertex.index = vertexIndex;
    gc->input.lastVertexIndex = lastVtxIndex;
    gc->input.indexCount = (gc->input.indexPrimEnabled) ? indexCount : 0;
    gc->input.vertexFormat = vertexFormat;
}

void __glSwitchToDefaultVertexBuffer(__GLcontext *gc, GLuint inputTag)
{
    GLuint beginOffsetDW = 0;
    GLuint infoOffsetDW = 0;
    GLuint dataOffsetDW = 0;
    GLint dataSize, mask, i, j;
    GLboolean continuedPrim;

    /* Copy the global variable gc->pCurrentInfoBufPtr back to gc->input.currentInfoBufPtr.
    */
    gc->input.currentInfoBufPtr = gc->pCurrentInfoBufPtr;

    /* If the first inputTag in vertexInfoBuffer is a vertex data tag then this
    ** is a continued primitive which is a continuation of last primitive batch.
    */
    continuedPrim = (gc->input.vertexInfoBuffer->inputTag == inputTag &&
                    ((inputTag & __GL_VERTEX_DATA_TAG_MASK) || inputTag == __GL_END_TAG));

    infoOffsetDW = (GLuint)((GLuint *)gc->input.currentInfoBufPtr - (GLuint *)gc->input.vertexInfoBuffer);
    if (infoOffsetDW || continuedPrim)
    {
        dataOffsetDW = gc->input.currentInfoBufPtr->offsetDW;

        /* Compute vertexCount and indexCount from vertexInfoBuffer and dataOffsetDW */
        __glComputeCacheBufVertexCount(gc);
    }

    /* Copy the partially compared vertex info buffer from cache to the default info buffer */
    dataSize = infoOffsetDW << 2;
    if (dataSize)
    {
        __GL_MEMCOPY(gc->input.defaultInfoBuffer, gc->input.vertexInfoBuffer, dataSize);
    }

    /* Copy the partially compared vertex data from cache to the default data buffer */
    dataSize = dataOffsetDW << 2;
    if (dataSize)
    {
        __GL_MEMCOPY(gc->input.defaultDataBuffer, gc->input.vertexDataBuffer, dataSize);
    }

    /* Copy the partially compared indices from the cache index buffer to the default index buffer */
    dataSize = gc->input.indexCount * sizeof(GLushort);
    if (dataSize)
    {
        __GL_MEMCOPY(gc->input.defaultIndexBuffer, gc->input.indexBuffer, dataSize);
    }

    /* Restore the vertex input machine from __GLvertexDataCache if there is partial vertex data match */
    if (dataOffsetDW)
    {
        /* Do not set cacheCompareFailed flag for vertex arrays because we want to
         * do data comparision in __glCheckCachedImmedPrimtive() function.
         */
        if ((inputTag & __GL_DRAWARRAYS_TAG_MASK) == 0)
        {
            gc->input.cacheCompareFailed = GL_TRUE;
        }

        __glRestoreVertexInputMachine(gc, gc->input.currentVertexCache, (gc->input.vertex.index - 1));

        /* Update currentPtrDW pointers based on the partial vertex data */
        i = j = 0;
        mask = __glVertexFormat2InputMask(gc->input.vertexFormat);
        mask &= (~__GL_INPUT_EDGEFLAG);
        while (mask)
        {
            if (mask & 0x1)
            {
                gc->input.currentInput[i].currentPtrDW += gc->input.vertTotalStrideDW;
                j += 1;
            }
            mask >>= 1;
            i += 1;
        }

        gc->input.preVertexFormat = gc->input.primitiveFormat;
        if (gc->input.vertex.index == 0)
        {
            /* Compute the partial primElemSequence for the attributes of the first vertex */
            i = ((gc->input.numberOfElements - j) * __GL_PRIM_ELEMENT_SHIFT);
            gc->input.primElemSequence = (gc->input.primElemSequence >> i);

            /*
            ** Before the first glVertex, gc->input.preVertexFormat equals gc->input.vertexFormat
            ** We must recalculate the preVertexFormat and only reserve attributes that have been
            ** copied to the default data buffer.
            */

            i = 0;
            mask = __glVertexFormat2InputMask(gc->input.primitiveFormat);
            mask &= ~(__GL_INPUT_EDGEFLAG);
            while(mask)
            {
                if((mask & 0x1) && gc->input.currentInput[i].offsetDW >= dataOffsetDW)
                {
                    gc->input.primitiveFormat &= ~(input2VertexFormat[i]);
                }
                mask >>= 1;
                i += 1;
            }
            gc->input.preVertexFormat = gc->input.primitiveFormat;
            gc->input.vertexFormat = gc->input.preVertexFormat;
        }
    }
    else
    {
        /* Reset primMode since this is a new beginning of primitive */
        gc->input.primMode = gc->input.currentPrimMode;
        gc->input.primElemSequence = 0;
        gc->input.preVertexFormat = 0;

        __glValidateImmedBegin(gc, gc->input.primMode);
    }

    /* Reset all the vertex data pointers to the default buffer */
    gc->input.currentInfoBufPtr = (__GLvertexInfo *)((GLuint *)gc->input.defaultInfoBuffer + infoOffsetDW);
    gc->input.currentDataBufPtr = gc->input.defaultDataBuffer + dataOffsetDW;
    gc->input.primBeginAddr = gc->input.defaultDataBuffer + beginOffsetDW;

    /* Finally switch the data and index buffers from the cache buffers to the default buffers */
    gc->input.vertexInfoBuffer = gc->input.defaultInfoBuffer;
    gc->input.vertexDataBuffer = gc->input.defaultDataBuffer;
    gc->input.indexBuffer = gc->input.defaultIndexBuffer;

    gc->input.cacheBufferUsed = GL_FALSE;

    gc->immedModeOutsideDispatch.DrawArrays = __glim_DrawArrays_Validate;
    gc->immedModeOutsideDispatch.DrawElements = __glim_DrawElements_Validate;
    __GL_SET_VARRAY_STOP_CACHE_BIT(gc);

    /* Switch back to the immediate mode dispatch table.
    */
    gc->input.pCurrentImmedModeDispatch = &gc->immedModeDispatch;
    gc->immedModeOutsideDispatch.Begin = __glim_Begin_Info;

    /* Reset immediateDispatchTable Vertex3fv entry according to CacheDispatchTable Vertex3fv entry.
    */
    if (gc->immedModeCacheDispatch.Vertex3fv == __glim_Vertex3fv_Cache)
    {
        gc->immedModeDispatch.Vertex3fv = __glim_Vertex3fv_Info;
    }
    else if (gc->immedModeCacheDispatch.Vertex3fv == __glim_Normal_Vertex3fv_Cache) {
        __glSwitchToNorVertEntriesFunc(gc);
    }

    /* Switch to inside or outside Begin/End dispatch table based on beginMode.
    */
    if (gc->input.beginMode == __GL_IN_BEGIN)
    {
        __glSwitchImmediateDispatch(gc, &gc->immedModeDispatch);
    }
    else
    {
        __glSwitchImmediateDispatch(gc, &gc->immedModeOutsideDispatch);
    }
}

GLvoid __glResetImmedVertexBuffer(__GLcontext *gc, GLboolean enableCache)
{
    __GLvertexDataCache *vtxCache = gc->input.currentVertexCache;
    GLuint prevBufferUsed = gc->input.cacheBufferUsed;

    /* Cache the primInputMask for the just rendered primitive batch */
    gc->input.prevPrimInputMask = gc->input.primInputMask;

    /* If the vertex cache has been setup in previous frame */
    if (vtxCache && enableCache && gc->input.currentFrameIndex > vtxCache->frameIndex &&
        vtxCache->cacheStatus == __GL_CHECK_FULL_VERTEX_CACHE &&
        !( vtxCache->primInputMask & __GL_INPUT_EDGEFLAG ))
    {
        /* Set the input buffers to the corresponding vertex buffer and index buffer in vertexCache */
        gc->pCurrentInfoBufPtr = gc->input.vertexInfoBuffer = vtxCache->vertexInfoBuffer;
        gc->pVertexDataBufPtr = gc->input.vertexDataBuffer = vtxCache->vertexDataBuffer;
        gc->input.indexBuffer = vtxCache->indexBuffer;

        gc->input.cacheBufferUsed = GL_TRUE;
        gc->input.cacheCompareFailed = GL_FALSE;

        __glRestoreVertexInputMachine(gc, vtxCache, -1);
        gc->input.pCurrentImmedModeDispatch = &gc->immedModeCacheDispatch;
        gc->immedModeOutsideDispatch.Begin = __glim_Begin_Cache_First;
    }
    else
    {
        /* Set the input buffer to the default vertex buffer */
        gc->input.vertexInfoBuffer = gc->input.defaultInfoBuffer;
        gc->input.vertexDataBuffer = gc->input.defaultDataBuffer;
        gc->input.indexBuffer = gc->input.defaultIndexBuffer;

        gc->input.cacheBufferUsed = GL_FALSE;
        gc->input.cacheCompareFailed = GL_FALSE;

        gc->input.primElemSequence = 0;

        gc->input.pCurrentImmedModeDispatch = &gc->immedModeDispatch;
        if (gc->immedModeOutsideDispatch.Begin == __glim_Begin_Cache_First)
        {
            gc->immedModeOutsideDispatch.Begin = __glim_Begin_Info;
        }
    }

    gc->input.currentInfoBufPtr = gc->input.vertexInfoBuffer;
    gc->input.currentDataBufPtr = gc->input.vertexDataBuffer;

    gc->input.vertex.index = 0;
    gc->input.vertexTrimCount = 0;
    gc->input.connectVertexCount = 0;
    gc->input.edgeflag.index = 0;
    gc->input.lastVertexIndex = 0;
    gc->input.indexCount = 0;
    gc->input.primBeginAddr = NULL;
    gc->input.preVertexFormat = 0;

    /* Reset dispatch table
    */
    if (gc->input.beginMode == __GL_IN_BEGIN)
    {
        __glSwitchImmediateDispatch(gc, gc->input.pCurrentImmedModeDispatch);
    }
    else
    {
        __glSwitchImmediateDispatch(gc, &gc->immedModeOutsideDispatch);
        /* Clear __GL_SMALL_DRAW_BATCH flag since all primitives have been flushed */
        gc->input.beginMode = __GL_NOT_IN_BEGIN;
    }

    if (prevBufferUsed != gc->input.cacheBufferUsed) {
        gc->immedModeOutsideDispatch.DrawArrays = __glim_DrawArrays_Validate;
        gc->immedModeOutsideDispatch.DrawElements = __glim_DrawElements_Validate;
        __GL_SET_VARRAY_STOP_CACHE_BIT(gc);
    }

    /* Turn on vertex data caching for the next drawPrimitive
    ** only if it's not in display list compilation and execution.
    */
    gc->input.vertexCacheEnabled = gc->input.enableVertexCaching;
}

GLvoid __glComputeRequiredInputMask(__GLcontext *gc)
{
    GLuint unit;
    GLbitfield textureEnableMask;
    __GLTextureEnableState *texUnitEnable;
    __GLtextureUnitState *texUnitState;
    GLuint64 vsInputMask = 0;
    GLuint psInputMask = 0;
    GLuint64 totalInputMask = 0;

    if (gc->shaderProgram.vertShaderEnable)
    {
        if (gc->shaderProgram.currentProgram)
        {
            vsInputMask = gc->shaderProgram.currentProgram->bindingInfo.vsInputMask;
        }
        else
        {
            GL_ASSERT(0);
        }
    }
    else if (gc->state.enables.program.vertexProgram)
    {
        if (gc->program.currentProgram[__GL_VERTEX_PROGRAM_INDEX])
        {
            vsInputMask = gc->program.currentProgram[__GL_VERTEX_PROGRAM_INDEX]->compiledResult.inputMask;
        }
        else
        {
            GL_ASSERT(0);
        }
    }
    else
    {
        /* Compute fixed function inputMask */

        vsInputMask = __GL_INPUT_VERTEX;

        if (gc->state.enables.lighting.lighting)
        {
            vsInputMask |= __GL_INPUT_NORMAL;
            if (gc->state.enables.lighting.colorMaterial)
            {
                vsInputMask |= __GL_INPUT_DIFFUSE;
            }
        }
        else
        {
            vsInputMask |= __GL_INPUT_DIFFUSE;
            if (gc->state.enables.colorSum)
            {
                vsInputMask |= __GL_INPUT_SPECULAR;
            }
        }

        /* When fog is enable, no matter the fog.coordSource is GL_FOG_COORD or GL_FRAGMENT_DEPTH,
        * both need to do a bitwise AND with __GL_INPUT_FOGCOORD for vsInputMask
        */
        if (gc->state.enables.fog)
        {
            vsInputMask |= __GL_INPUT_FOGCOORD;
        }
    }

    if (gc->shaderProgram.fragShaderEnable)
    {
        if (gc->shaderProgram.currentProgram)
        {
            psInputMask = gc->shaderProgram.currentProgram->bindingInfo.psInputMask;
        }
        else
        {
            GL_ASSERT(0);
        }
    }
    else if (gc->state.enables.program.fragmentProgram)
    {
        if (gc->program.currentProgram[__GL_FRAGMENT_PROGRAM_INDEX])
        {
            psInputMask = gc->program.currentProgram[__GL_FRAGMENT_PROGRAM_INDEX]->compiledResult.inputMask;
        }
        else
        {
            GL_ASSERT(0);
        }
    }
    /* Fragment shader might also encounter the case that ffvs(no texture enabled)+pps(texture wanted) */
    /*
    else if (gc->state.enables.program.fragmentShaderATI)
    {
    if (gc->fragmentShaderATI.currentShader))
    {
    inputMask |= gc->fragmentShaderATI.currentShader->inputMask;
    }
    else
    {
    GL_ASSERT(0);
    }
    }
    */
    else
    {
        psInputMask = (gc->texture.enabledMask & __GL_TEXTURE_ENABLED_MASK) << __GL_INPUT_TEX0_INDEX;
    }

    unit = 0;
    textureEnableMask = (psInputMask >> __GL_INPUT_TEX0_INDEX) & __GL_TEXTURE_ENABLED_MASK;
    while (textureEnableMask)
    {
        if (textureEnableMask & 1)
        {
            texUnitEnable = &gc->state.enables.texUnits[unit];
            texUnitState = &gc->state.texture.texUnits[unit];
            if (texUnitEnable->texGen[0])
            {
                if ((texUnitState->s.mode == GL_SPHERE_MAP) ||
                    (texUnitState->s.mode == GL_REFLECTION_MAP) ||
                    (texUnitState->s.mode == GL_NORMAL_MAP))
                {
                        totalInputMask |= __GL_INPUT_NORMAL;
                }
            }
            if (texUnitEnable->texGen[1])
            {
                if ((texUnitState->t.mode == GL_SPHERE_MAP) ||
                    (texUnitState->t.mode == GL_REFLECTION_MAP) ||
                    (texUnitState->t.mode == GL_NORMAL_MAP)) {
                        totalInputMask |= __GL_INPUT_NORMAL;
                }
            }
            if (texUnitEnable->texGen[2])
            {
                if ((texUnitState->r.mode == GL_REFLECTION_MAP) ||
                    (texUnitState->r.mode == GL_NORMAL_MAP))
                {
                        totalInputMask |= __GL_INPUT_NORMAL;
                }
            }
        }
        textureEnableMask >>= 1;
        unit++;
    }

    if (!gc->state.polygon.bothFaceFill)
    {
        totalInputMask |= __GL_INPUT_EDGEFLAG;
    }

    if (gc->state.enables.program.vertexProgram || gc->shaderProgram.vertShaderEnable)
    {
        totalInputMask |= vsInputMask;
    }
    else
    {
        totalInputMask |= vsInputMask | psInputMask;
    }

    /*
    * For select mode, we only need position, so use inputMask calculated before is enough.
    * For feedback mode, if app want texcoord0, we need input it whether texture is enable.
    */
    if (gc->renderMode == GL_FEEDBACK)
    {
        switch(gc->feedback.type)
        {
            case GL_3D_COLOR_TEXTURE:
            case GL_4D_COLOR_TEXTURE:
                totalInputMask |= __GL_INPUT_TEX0;
                break;
            default:
                break;
        }
    }

    if (gc->input.path == __GL_VERTEXINPUT_PATH_NORMAL)
    {
        gc->input.currentInputMask = totalInputMask;
    }
    else
    {
        if (gc->input.path == __GL_VERTEXINPUT_PATH_MANUAL)
        {
            gc->input.currentInputMask = gc->input.manualInputMask;
        }
    }

    if ((totalInputMask & __GL_INPUT_NORMAL) == 0)
    {
        if (gc->input.origVertexCacheFlag)
        {
            gc->immedModeDispatch.Vertex3fv = __glim_Vertex3fv_Info;
            gc->immedModeCacheDispatch.Vertex3fv = __glim_Vertex3fv_Cache;
        }
        else
        {
            gc->immedModeDispatch.Vertex3fv = __glim_Vertex3fv;
            gc->immedModeCacheDispatch.Vertex3fv = __glim_Vertex3fv_Cache;
        }

        /* If normal is not needed then restore the original DrawArray_V3F entry functions. */
        if (gc->immedModeOutsideDispatch.DrawArrays == __glImmedDrawArrays_Normal_V3F)
        {
            gc->immedModeOutsideDispatch.DrawArrays = __glImmedDrawArrays_V3F_Select;
        }
    }
    if ((totalInputMask & __GL_INPUT_DIFFUSE) == 0)
    {
        /* If color is not needed then restore the original DrawArray_V3F entry functions. */
        if (gc->immedModeOutsideDispatch.DrawArrays == __glImmedDrawArrays_Color_V3F)
        {
            gc->immedModeOutsideDispatch.DrawArrays = __glImmedDrawArrays_V3F_Select;
        }
    }
}

/*
** compute required input mask, bypass cache logic
*/
GLvoid __glComputeRequiredInputMaskInstancedEXT(__GLcontext *gc)
{
    GLuint unit;
    GLbitfield textureEnableMask;
    __GLTextureEnableState *texUnitEnable;
    __GLtextureUnitState *texUnitState;
    GLuint64 vsInputMask = 0;
    GLuint psInputMask = 0;
    GLuint64 totalInputMask = 0;

    if (gc->shaderProgram.vertShaderEnable)
    {
        if (gc->shaderProgram.currentProgram)
        {
            vsInputMask = gc->shaderProgram.currentProgram->bindingInfo.vsInputMask;
        }
        else
        {
            GL_ASSERT(0);
        }
    }
    else if (gc->state.enables.program.vertexProgram)
    {
        if (gc->program.currentProgram[__GL_VERTEX_PROGRAM_INDEX])
        {
            vsInputMask = gc->program.currentProgram[__GL_VERTEX_PROGRAM_INDEX]->compiledResult.inputMask;
        }
        else
        {
            GL_ASSERT(0);
        }
    }
    else
    {
        /* Compute fixed function inputMask */

        vsInputMask = __GL_INPUT_VERTEX;

        if (gc->state.enables.lighting.lighting)
        {
            vsInputMask |= __GL_INPUT_NORMAL;
            if (gc->state.enables.lighting.colorMaterial) {
                vsInputMask |= __GL_INPUT_DIFFUSE;
            }
        }
        else
        {
            vsInputMask |= __GL_INPUT_DIFFUSE;
            if (gc->state.enables.colorSum) {
                vsInputMask |= __GL_INPUT_SPECULAR;
            }
        }

        /* When fog is enable, no matter the fog.coordSource is GL_FOG_COORD or GL_FRAGMENT_DEPTH,
        * both need to do a bitwise AND with __GL_INPUT_FOGCOORD for vsInputMask */
        if (gc->state.enables.fog)
        {
            vsInputMask |= __GL_INPUT_FOGCOORD;
        }
    }

    if (gc->shaderProgram.fragShaderEnable)
    {
        if (gc->shaderProgram.currentProgram)
        {
            psInputMask = gc->shaderProgram.currentProgram->bindingInfo.psInputMask;
        }
        else
        {
            GL_ASSERT(0);
        }
    }
    else if (gc->state.enables.program.fragmentProgram)
    {
        if (gc->program.currentProgram[__GL_FRAGMENT_PROGRAM_INDEX])
        {
            psInputMask = gc->program.currentProgram[__GL_FRAGMENT_PROGRAM_INDEX]->compiledResult.inputMask;
        }
        else
        {
            GL_ASSERT(0);
        }
    }
    else
    {
        psInputMask = (gc->texture.enabledMask & __GL_TEXTURE_ENABLED_MASK) << __GL_INPUT_TEX0_INDEX;
    }

    unit = 0;
    textureEnableMask = (psInputMask >> __GL_INPUT_TEX0_INDEX) & __GL_TEXTURE_ENABLED_MASK;
    while (textureEnableMask)
    {
        if (textureEnableMask & 1)
        {
            texUnitEnable = &gc->state.enables.texUnits[unit];
            texUnitState = &gc->state.texture.texUnits[unit];
            if (texUnitEnable->texGen[0]) {
                if ((texUnitState->s.mode == GL_SPHERE_MAP) ||
                    (texUnitState->s.mode == GL_REFLECTION_MAP) ||
                    (texUnitState->s.mode == GL_NORMAL_MAP)) {
                        totalInputMask |= __GL_INPUT_NORMAL;
                }
            }
            if (texUnitEnable->texGen[1]) {
                if ((texUnitState->t.mode == GL_SPHERE_MAP) ||
                    (texUnitState->t.mode == GL_REFLECTION_MAP) ||
                    (texUnitState->t.mode == GL_NORMAL_MAP)) {
                        totalInputMask |= __GL_INPUT_NORMAL;
                }
            }
            if (texUnitEnable->texGen[2]) {
                if ((texUnitState->r.mode == GL_REFLECTION_MAP) ||
                    (texUnitState->r.mode == GL_NORMAL_MAP)) {
                        totalInputMask |= __GL_INPUT_NORMAL;
                }
            }
        }
        textureEnableMask >>= 1;
        unit++;
    }

    if (!gc->state.polygon.bothFaceFill) {
        totalInputMask |= __GL_INPUT_EDGEFLAG;
    }

    if (gc->state.enables.program.vertexProgram || gc->shaderProgram.vertShaderEnable)
    {
        totalInputMask |= vsInputMask;
    }
    else
    {
        totalInputMask |= vsInputMask | psInputMask;
    }

    /*
        For select mode, we only need position, so use inputMask calculated before is enough.
        For feedback mode, if app want texcoord0, we need input it whether texture is enable.
    */
    if (gc->renderMode == GL_FEEDBACK)
    {
        switch(gc->feedback.type){
            case GL_3D_COLOR_TEXTURE:
            case GL_4D_COLOR_TEXTURE:
                totalInputMask |= __GL_INPUT_TEX0;
                break;
            default:
                break;
        }
    }

    if (gc->input.path == __GL_VERTEXINPUT_PATH_NORMAL)
        gc->input.currentInputMask = totalInputMask;
    else if (gc->input.path == __GL_VERTEXINPUT_PATH_MANUAL)
        gc->input.currentInputMask = gc->input.manualInputMask;
}

/* Fill in the missing attributes for the current vertex (only one vertex) */

GLvoid __glFillMissingAttributes(__GLcontext *gc)
{
    GLuint i, vertexIndex, index, dstStride;
    GLuint64 inputMask;
    __GLvertexInput *input;
    GLfloat *src, *dst;

    i = 0;
    vertexIndex = gc->input.vertex.index;
    dstStride = (gc->input.vertTotalStrideDW << 2);
    inputMask = (gc->input.requiredInputMask & ~(__GL_INPUT_VERTEX | __GL_INPUT_EDGEFLAG));
    while(inputMask)
    {
        if (inputMask & 0x1)
        {
            input = &gc->input.currentInput[i];
            index = input->index;
            if (vertexIndex >= index)
            {
                if (index > 0) {
                    src = (GLfloat*)(input->pointer + (index - 1) * dstStride);
                }
                else {
                    src = (GLfloat*)&gc->state.current.currentState[i];
                }
                dst = (GLfloat*)(input->pointer + index * dstStride);

                switch(input->sizeDW)
                {
                case 4:
                    *(dst) = *(src);
                    *(dst + 1) = *(src + 1);
                    *(dst + 2) = *(src + 2);
                    *(dst + 3) = *(src + 3);
                    break;
                case 3:
                    *(dst) = *(src);
                    *(dst + 1) = *(src + 1);
                    *(dst + 2) = *(src + 2);
                    break;
                case 2:
                    *(dst) = *(src);
                    *(dst + 1) = *(src + 1);
                    break;
                case 1:
                    *(dst) = *(src);
                    break;
                default:
                    GL_ASSERT(input->sizeDW);
                    break;
                }
                input->index += 1;
            }
        }
        i++;
        inputMask >>= 1;
    }

    if (gc->input.requiredInputMask & __GL_INPUT_EDGEFLAG)
    {
        GLboolean *edgeaddr = gc->input.edgeflag.pointer;
        GLboolean flag;

        index = gc->input.edgeflag.index;
        if (vertexIndex >= index)
        {
            if (index > 0) {
                flag = edgeaddr[index - 1];
            }
            else
            {
                flag = gc->state.current.edgeflag;
            }
            edgeaddr[index++] = flag;
            gc->input.edgeflag.index = index;
        }
    }
}

GLvoid __glDuplicateVertexAttributes(__GLcontext *gc)
{
    GLuint i, j, vertexIndex, index, dstStride;
    GLuint64 inputMask;
    __GLvertexInput *input;
    GLfloat *src, *dst;

    i = 0;
    vertexIndex = gc->input.vertex.index;
    dstStride = (gc->input.vertTotalStrideDW << 2);
    inputMask = (gc->input.requiredInputMask & ~(__GL_INPUT_VERTEX | __GL_INPUT_EDGEFLAG));
    while(inputMask)
    {
        if (inputMask & 0x1)
        {
            input = &gc->input.currentInput[i];
            index = input->index;
            if (vertexIndex > index)
            {
                if (index > 0) {
                    src = (GLfloat*)(input->pointer + (index - 1) * dstStride);
                }
                else {
                    src = (GLfloat*)&gc->state.current.currentState[i];
                }
                dst = (GLfloat*)(input->pointer + index * dstStride);

                for (j = index; j < vertexIndex; j++)
                {
                    switch(input->sizeDW)
                    {
                    case 4:
                        *(dst) = *(src);
                        *(dst + 1) = *(src + 1);
                        *(dst + 2) = *(src + 2);
                        *(dst + 3) = *(src + 3);
                        break;
                    case 3:
                        *(dst) = *(src);
                        *(dst + 1) = *(src + 1);
                        *(dst + 2) = *(src + 2);
                        break;
                    case 2:
                        *(dst) = *(src);
                        *(dst + 1) = *(src + 1);
                        break;
                    case 1:
                        *(dst) = *(src);
                        break;
                    default:
                        GL_ASSERT(input->sizeDW);
                        break;
                    }
                    dst = (GLfloat*)((GLubyte*)dst + dstStride);
                }
                input->index = vertexIndex;
            }
        }
        i++;
        inputMask >>= 1;
    }

    if (gc->input.requiredInputMask & __GL_INPUT_EDGEFLAG)
    {
        GLboolean *edgeaddr = gc->input.edgeflag.pointer;
        GLboolean flag;

        index = gc->input.edgeflag.index;
        if (vertexIndex > index)
        {
            if (index > 0) {
                flag = edgeaddr[index - 1];
            }
            else {
                flag = gc->state.current.edgeflag;
            }
            for(j = index; j < vertexIndex; j++)
            {
                edgeaddr[j] = flag;
            }
            gc->input.edgeflag.index = vertexIndex;
        }
    }
}

GLvoid __glImmedUpdateVertexState(__GLcontext *gc)
{
    __GLvertexInput *input;
    GLuint i;
    GLuint64 mask;

    mask = (gc->input.primInputMask & ~(__GL_INPUT_VERTEX | __GL_INPUT_EDGEFLAG));

    i = 0;
    while (mask)
    {
        if (mask & 0x1)
        {
            GLfloat *src, *dst;
            input = &gc->input.currentInput[i];
            if ((GLubyte*)input->currentPtrDW < (GLubyte*)input->pointer)
            {
                /* This could happen in number of vertex is 0 case. skip vertex attribute update */
                goto Skip_UpdateVertexState;
            }
            src = (GLfloat*)input->currentPtrDW;
            dst = (GLfloat*)&gc->state.current.currentState[i];
            switch(input->sizeDW)
            {
            case 4:
                *(dst) = *(src);
                *(dst + 1) = *(src + 1);
                *(dst + 2) = *(src + 2);
                *(dst + 3) = *(src + 3);
                break;
            case 3:
                *(dst) = *(src);
                *(dst + 1) = *(src + 1);
                *(dst + 2) = *(src + 2);
                *(dst + 3) = 1.0;
                break;
            case 2:
                *(dst) = *(src);
                *(dst + 1) = *(src + 1);
                *(dst + 2) = 0.0;
                *(dst + 3) = 1.0;
                break;
            case 1:
                if (i == __GL_INPUT_DIFFUSE_INDEX)
                {
                    GLubyte *ubcolor = (GLubyte *)src;
                    *(dst) = __GL_UB_TO_FLOAT(*(ubcolor));              /* R */
                    *(dst + 1) = __GL_UB_TO_FLOAT(*(ubcolor + 1));      /* G */
                    *(dst + 2) = __GL_UB_TO_FLOAT(*(ubcolor + 2));      /* B */
                    *(dst + 3) = __GL_UB_TO_FLOAT(*(ubcolor + 3));      /* A */
                }
                else {
                    *(dst) = *(src);
                    *(dst + 1) = 0.0;
                    *(dst + 2) = 0.0;
                    *(dst + 3) = 1.0;
                }
                break;
            default:
                GL_ASSERT(input->sizeDW);
                break;
            }
        }

Skip_UpdateVertexState:

        i += 1;
        mask >>= 1;
    }

    if (gc->input.primInputMask & __GL_INPUT_EDGEFLAG)
    {
        gc->state.current.edgeflag = gc->input.edgeflag.pointer[gc->input.edgeflag.index - 1];
    }

    /* Copy the cached current state in gc->input to gc->state.current */
    if (gc->input.deferredAttribDirty & __GL_DEFERED_NORMAL_BIT)
    {
        gc->state.current.normal = gc->input.shadowCurrent.normal;
    }
    if (gc->input.deferredAttribDirty & __GL_DEFERED_COLOR_BIT)
    {
        gc->state.current.color = gc->input.shadowCurrent.color;
    }

    /* Use the current color to update material state if color material is enabled */
    if (gc->state.enables.lighting.colorMaterial &&
        ((gc->input.primInputMask & __GL_INPUT_DIFFUSE) ||
         (gc->input.deferredAttribDirty & __GL_DEFERED_COLOR_BIT)))
    {
        __glUpdateMaterialfv(gc, gc->state.light.colorMaterialFace,
            gc->state.light.colorMaterialParam, (GLfloat *)&gc->state.current.color);
    }

    /* Clear Normal and Color bits in deferedAttribDirty since the deferred Normal/Color
    ** have been copied to current states. Note: __GL_DEFERED_ATTRIB_BIT could still be dirty.
    */
    gc->input.deferredAttribDirty &= ~(__GL_DEFERED_NORMAL_BIT | __GL_DEFERED_COLOR_BIT);
}

GLvoid __glGenerateVertexIndex(__GLcontext *gc)
{
    GLushort *idxBuf;
    GLuint i, indexCount, startIndex, index, vertexCount;

    vertexCount = gc->input.vertex.index - gc->input.lastVertexIndex;
    if (vertexCount == 0) {
        return;
    }

    indexCount = gc->input.indexCount;
    index = startIndex = gc->input.lastVertexIndex;
    idxBuf = gc->input.indexBuffer;

    GL_ASSERT((index+3)<__glMaxUshort);

    switch (gc->input.currentPrimMode)
    {
    case GL_TRIANGLES:
        for (i = 0; i < vertexCount; i += 3)
        {
            idxBuf[indexCount++] = (GLushort)index;
            idxBuf[indexCount++] = (GLushort)index + 1;
            idxBuf[indexCount++] = (GLushort)index + 2;
            index = index + 3;
        }
        break;

    case GL_TRIANGLE_STRIP:
        idxBuf[indexCount++] = (GLushort)index;
        idxBuf[indexCount++] = (GLushort)index + 1;
        idxBuf[indexCount++] = (GLushort)index + 2;
        index = index + 3;

        for (i = 3; i < vertexCount; i++)
        {
            if (i & 1)
            {
                idxBuf[indexCount++] = (GLushort)index - 1;
                idxBuf[indexCount++] = (GLushort)index - 2;
                idxBuf[indexCount++] = (GLushort)index;
                index++;
            }
            else
            {
                idxBuf[indexCount++] = (GLushort)index - 2;
                idxBuf[indexCount++] = (GLushort)index - 1;
                idxBuf[indexCount++] = (GLushort)index;
                index++;
            }
        }
        break;
    case GL_TRIANGLE_FAN:
        idxBuf[indexCount++] = (GLushort)index;
        idxBuf[indexCount++] = (GLushort)index + 1;
        idxBuf[indexCount++] = (GLushort)index + 2;
        index = index + 3;

        for (i = 3; i < vertexCount; i++)
        {
            idxBuf[indexCount++] = (GLushort)startIndex;
            idxBuf[indexCount++] = (GLushort)index - 1;
            idxBuf[indexCount++] = (GLushort)index;
            index++;
        }
        break;
    case GL_QUADS:
        for (i = 0; i < vertexCount; i += 4)
        {
            idxBuf[indexCount++] = (GLushort)index;
            idxBuf[indexCount++] = (GLushort)index + 1;
            idxBuf[indexCount++] = (GLushort)index + 3;
            idxBuf[indexCount++] = (GLushort)index + 1;
            idxBuf[indexCount++] = (GLushort)index + 2;
            idxBuf[indexCount++] = (GLushort)index + 3;
            index = index + 4;
        }
        break;
    case GL_QUAD_STRIP:
        for (i = 0; i < (vertexCount - 2); i += 2)
        {
            idxBuf[indexCount++] = (GLushort)index;
            idxBuf[indexCount++] = (GLushort)index + 1;
            idxBuf[indexCount++] = (GLushort)index + 3;
            idxBuf[indexCount++] = (GLushort)index + 2;
            idxBuf[indexCount++] = (GLushort)index;
            idxBuf[indexCount++] = (GLushort)index + 3;
            index = index + 2;
        }
        break;
    case GL_POLYGON:
        idxBuf[indexCount++] = (GLushort)index + 1;
        idxBuf[indexCount++] = (GLushort)index + 2;
        idxBuf[indexCount++] = (GLushort)startIndex;
        index = index + 3;

        for (i = 3; i < vertexCount; i++)
        {
            idxBuf[indexCount++] = (GLushort)index - 1;
            idxBuf[indexCount++] = (GLushort)index;
            idxBuf[indexCount++] = (GLushort)startIndex;
            index++;
        }
        break;
    case GL_LINES:
        for (i = 0; i < vertexCount; i += 2)
        {
            idxBuf[indexCount++] = (GLushort)index;
            idxBuf[indexCount++] = (GLushort)index + 1;
            index = index + 2;
        }
        break;
    case GL_LINE_STRIP:
        idxBuf[indexCount++] = (GLushort)index;
        idxBuf[indexCount++] = (GLushort)index + 1;
        index = index + 2;

        for (i = 2; i < vertexCount; i++)
        {
            idxBuf[indexCount++] = (GLushort)index - 1;
            idxBuf[indexCount++] = (GLushort)index;
            index++;
        }
        break;
    case GL_LINE_LOOP:
        idxBuf[indexCount++] = (GLushort)index;
        idxBuf[indexCount++] = (GLushort)index+1;
        index = index + 2;

        for (i = 2; i < vertexCount; i++)
        {
            idxBuf[indexCount++] = (GLushort)index - 1;
            idxBuf[indexCount++] = (GLushort)index;
            index++;
        }

        idxBuf[indexCount++] = (GLushort)index-1;
        idxBuf[indexCount++] = (GLushort)startIndex;
        break;
    }

    gc->input.indexCount = indexCount;
}

GLvoid __glImmedFlushBuffer_Material(__GLcontext *gc)
{
    GLuint64 inputMask;
    GLint  i;

    /* Cache current primElemSequence and preVertexFormat in gc->input */

    __glImmedFlushPrim_Material(gc, GL_FALSE);

    __glResetImmedVertexBuffer(gc, GL_FALSE);
    gc->tnlAccum.preVertexIndex = gc->input.vertex.index;

    /* There is no need to offset gc->input.currentInfoBufPtr like gc->input.currentDataBufPtr */
    gc->input.primBeginAddr = gc->input.defaultDataBuffer;
    gc->input.currentDataBufPtr = gc->input.defaultDataBuffer;

    /* Reset all the input pointers to the new locations */
    i = 0;
    inputMask = gc->input.primInputMask & (~__GL_INPUT_EDGEFLAG);
    while (inputMask) {
        if (inputMask & 0x1)
        {
            gc->input.currentInput[i].pointer =
                (GLubyte *)(gc->input.defaultDataBuffer + gc->input.currentInput[i].offsetDW);
            gc->input.currentInput[i].currentPtrDW =
                (GLfloat*)gc->input.currentInput[i].pointer;
            gc->input.currentInput[i].index = 0;
        }
        inputMask >>= 1;
        i += 1;
    }
}

/* If a new vertex attribute occurs in the middle of glBegin and glEnd */
GLvoid __glSwitchToNewPrimtiveFormat_Material(__GLcontext *gc, GLuint attFmtIdx)
{
    GLuint attInpIdx = fmtIndex2InputIndex[attFmtIdx];
    GLuint dataSize, origTotalStrideDW, i, j;
    GLuint formatMask, lastVertexIndex, inputTag;
    GLuint64 inputMask;
    GLfloat *lastVertexBlock, *src, *dst;
    GLuint inputOffsetDW[__GL_TOTAL_VERTEX_ATTRIBUTES] = {0};
    GLuint inputSize[__GL_TOTAL_VERTEX_ATTRIBUTES] = {0};

    origTotalStrideDW = gc->input.vertTotalStrideDW;

    /* Save data of last vertex */
    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, origTotalStrideDW<<2, (gctPOINTER*)&lastVertexBlock)))
    {
        return;
    }

    lastVertexIndex = gc->input.vertex.index;
    /* Copy the current vertex date to a scratch area */
    src = (GLfloat *)(gc->input.primBeginAddr + origTotalStrideDW * (gc->input.vertex.index - gc->input.lastVertexIndex + 1));
    __GL_MEMCOPY(lastVertexBlock, src, origTotalStrideDW<<2);

    __glImmedFlushPrim_Material(gc, GL_FALSE);
    __glResetImmedVertexBuffer(gc, GL_FALSE);
    gc->tnlAccum.preVertexIndex = gc->input.vertex.index;

    gc->input.primBeginAddr = gc->input.currentDataBufPtr;

    /* Change the primitive format to include the new attribute */
    gc->input.primInputMask |= (__GL_ONE_64 << attInpIdx);
    gc->input.preVertexFormat |= (__GL_ONE_64 << attFmtIdx);
    gc->input.primitiveFormat = gc->input.preVertexFormat;
    gc->input.currentInput[attInpIdx].sizeDW = fmtIndex2DWSize[attFmtIdx];

    /* Reset the new offsetDW and pointers for all the attributes already accumulated (including new attrib)*/
    inputMask = gc->input.primInputMask & ~(__GL_INPUT_VERTEX | __GL_INPUT_EDGEFLAG);
    j = 0;
    while (inputMask)
    {
        if (inputMask & 0x1)
        {
            inputSize[j] = gc->input.currentInput[j].sizeDW;
            inputOffsetDW[j] = gc->input.currentInput[j].offsetDW;
            gc->input.currentInput[j].pointer = (GLubyte*)gc->input.currentDataBufPtr;
            gc->input.currentInput[j].currentPtrDW = gc->input.currentDataBufPtr;
            gc->input.currentInput[j].offsetDW = (GLuint)(gc->input.currentDataBufPtr - gc->input.primBeginAddr);
            gc->input.currentInput[j].index = 0;
            dataSize = inputSize[j];
            gc->input.currentDataBufPtr += dataSize;
            inputTag = inputTagTable[j][dataSize - 1];
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, inputTag);
        }
        inputMask >>= 1;
        j += 1;
    }

    /* Copy all the attributes already accumulated for the last glVertex to the new location */
    j = 0;
    formatMask = __glVertexFormat2InputMask(gc->input.vertexFormat);
    inputMask = formatMask & ~(__GL_INPUT_VERTEX | __GL_INPUT_EDGEFLAG);
    inputMask &= ~(__GL_ONE_64 << attInpIdx);
    gc->input.vertexFormat = 0;
    while (inputMask)
    {
        if (inputMask & 0x1)
        {
            gc->input.vertexFormat |= input2InconsistFormat[j];
            src = lastVertexBlock+ inputOffsetDW[j];
            dst = gc->input.currentInput[j].currentPtrDW;
            dataSize = input2MaxElementSize[j];
            if (j == __GL_INPUT_DIFFUSE_INDEX && inputSize[j] == 1)
            {
                GLubyte *ubcolor = (GLubyte *)src;
                *(dst) = __GL_UB_TO_FLOAT(*(ubcolor));              /* R */
                *(dst + 1) = __GL_UB_TO_FLOAT(*(ubcolor + 1));      /* G */
                *(dst + 2) = __GL_UB_TO_FLOAT(*(ubcolor + 2));      /* B */
                *(dst + 3) = __GL_UB_TO_FLOAT(*(ubcolor + 3));      /* A */
            }
            else
            {
                if (dataSize == 4)
                {
                    *(dst) = 0.0;
                    *(dst + 1) = 0.0;
                    *(dst + 2) = 0.0;
                    *(dst + 3) = 1.0;
                }
                for (i = 0; i < inputSize[j]; i++)
                {
                    *(dst + i) = *(src + i);
                }
            }
            gc->input.currentInput[j].index = 1;
        }
        inputMask >>= 1;
        j += 1;
    }

    if (formatMask & __GL_INPUT_EDGEFLAG)
    {
        GLboolean *edgeaddr = gc->input.edgeflag.pointer;
        if (lastVertexIndex) {
            edgeaddr[0] = edgeaddr[lastVertexIndex];
        }
    }

    /* Free the scratch memory */
    gcmOS_SAFE_FREE(gcvNULL, lastVertexBlock);

    /* Turn off vertex data caching if the primitive has changed its original format */
    gc->input.vertexCacheEnabled = GL_FALSE;
}

/* The vertex format is changed in the middle of glBegin/glEnd. */
GLvoid __glSwitchToInconsistentFormat_Material(__GLcontext *gc)
{
    GLuint dataSize, newTotalStrideDW, origTotalStrideDW, i, j;
    GLuint formatMask, lastVertexIndex, inputTag;
    GLuint64 inputMask;
    GLfloat *LastVert, *src, *dst;
    GLuint inputOffsetDW[__GL_TOTAL_VERTEX_ATTRIBUTES] = {0};
    GLuint inputSize[__GL_TOTAL_VERTEX_ATTRIBUTES] = {0};

    origTotalStrideDW = gc->input.vertTotalStrideDW;

    lastVertexIndex = gc->input.vertex.index;
    /* Copy the current vertex  to a scratch area */
    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, origTotalStrideDW, (gctPOINTER*)&LastVert)))
    {
        return;
    }

    src = (GLfloat *)(gc->input.primBeginAddr + origTotalStrideDW * (gc->input.vertex.index - gc->input.lastVertexIndex + 1));
    __GL_MEMCOPY(LastVert, src, origTotalStrideDW<<2);

    __glImmedFlushPrim_Material(gc, GL_FALSE);
    __glResetImmedVertexBuffer(gc, GL_FALSE);
    gc->tnlAccum.preVertexIndex = gc->input.vertex.index;

    gc->input.primBeginAddr = gc->input.currentDataBufPtr;

    /* Set the new offsetDW and pointers */
    inputMask = gc->input.requiredInputMask & ~(__GL_INPUT_EDGEFLAG);
    newTotalStrideDW = j = 0;
    while (inputMask)
    {
        if (inputMask & 0x1) {
            inputSize[j] = gc->input.currentInput[j].sizeDW;
            inputOffsetDW[j] = gc->input.currentInput[j].offsetDW;
            gc->input.currentInput[j].pointer = (GLubyte*)gc->input.currentDataBufPtr;
            gc->input.currentInput[j].currentPtrDW = gc->input.currentDataBufPtr;
            gc->input.currentInput[j].offsetDW = (GLuint)(gc->input.currentDataBufPtr - gc->input.primBeginAddr);
            gc->input.currentInput[j].index = 0;
            dataSize = input2MaxElementSize[j];
            gc->input.currentInput[j].sizeDW = dataSize;
            gc->input.currentDataBufPtr += dataSize;
            newTotalStrideDW += dataSize;
            inputTag = inputTagTable[j][dataSize - 1];
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, inputTag);
        }
        inputMask >>= 1;
        j += 1;
    }
    gc->input.vertTotalStrideDW = newTotalStrideDW;

    /* Copy all the attributes already accumulated for the last glVertex to the new location */
    j = 0;
    formatMask = __glVertexFormat2InputMask(gc->input.vertexFormat);
    inputMask = formatMask & ~(__GL_INPUT_VERTEX | __GL_INPUT_EDGEFLAG);
    gc->input.vertexFormat = 0;
    while (inputMask)
    {
        if (inputMask & 0x1)
        {
            gc->input.vertexFormat |= input2InconsistFormat[j];
            src = LastVert+ inputOffsetDW[j];
            dst = gc->input.currentInput[j].currentPtrDW;
            dataSize = input2MaxElementSize[j];
            if (j == __GL_INPUT_DIFFUSE_INDEX && inputSize[j] == 1)
            {
                GLubyte *ubcolor = (GLubyte *)src;
                *(dst) = __GL_UB_TO_FLOAT(*(ubcolor));              /* R */
                *(dst + 1) = __GL_UB_TO_FLOAT(*(ubcolor + 1));      /* G */
                *(dst + 2) = __GL_UB_TO_FLOAT(*(ubcolor + 2));      /* B */
                *(dst + 3) = __GL_UB_TO_FLOAT(*(ubcolor + 3));      /* A */
            }
            else {
                if (dataSize == 4)
                {
                    *(dst) = 0.0;
                    *(dst + 1) = 0.0;
                    *(dst + 2) = 0.0;
                    *(dst + 3) = 1.0;
                }
                for (i = 0; i < inputSize[j]; i++)
                {
                    *(dst + i) = *(src + i);
                }
            }
            gc->input.currentInput[j].currentPtrDW += newTotalStrideDW;
            gc->input.currentInput[j].index = 1;
        }
        inputMask >>= 1;
        j += 1;
    }

    if (formatMask & __GL_INPUT_EDGEFLAG)
    {
        GLboolean* edgeaddr = gc->input.edgeflag.pointer;
        if (lastVertexIndex) {
            edgeaddr[0] = edgeaddr[lastVertexIndex];
        }
        gc->input.edgeflag.index = 1;
        gc->input.vertexFormat |= __GL_EDGEFLAG_BIT;
    }

    /* Free the scratch memory */
    gcmOS_SAFE_FREE(gcvNULL, LastVert);

    gc->input.preVertexFormat = 0;
    gc->input.primitiveFormat = __glInputMask2InconsisFormat(gc->input.requiredInputMask);
    gc->input.inconsistentFormat = GL_TRUE;

    /* Turn off vertex data caching if the primitive has inconsistent format */
    gc->input.vertexCacheEnabled = GL_FALSE;
}

/* Flush the immediate mode vertex buffer when it is full.
*/
GLvoid __glImmediateFlushBuffer(__GLcontext *gc)
{
    GLuint64 primElemSequence, inputMask;
    GLuint startIndex, endIndex;
    GLint vertexCount;
    GLint connectCount = 0, i;
    GLboolean enableCache;

    if (gc->currentImmediateDispatch->End == __glim_End_Material)
    {
        __glImmedFlushBuffer_Material(gc);
        return;
    }

    /* Cache current primElemSequence and preVertexFormat in gc->input */
    primElemSequence = gc->input.primElemSequence;
    if (gc->input.inconsistentFormat == GL_FALSE) {
        gc->input.primitiveFormat = gc->input.preVertexFormat;
    }

    __glComputePrimitiveData(gc);

    startIndex = gc->input.lastVertexIndex;
    endIndex = gc->input.vertex.index;
    vertexCount = endIndex - startIndex;

    if (vertexCount == 0)
    {
        return;
    }

    switch (gc->input.currentPrimMode)
    {
    case GL_POINTS:
        connectCount = 0;
        break;
    case GL_LINES:
        connectCount = vertexCount & 1;
        vertexCount -= connectCount;
        gc->input.connectVertexIndex[0] = endIndex - 1;
        break;
    case GL_LINE_STRIP:
    case GL_LINE_LOOP:
        if (vertexCount >= 1)
        {
            connectCount = 1;
            gc->input.connectVertexIndex[0] = endIndex - 1;
            if (vertexCount == 1)
            {
                vertexCount = 0;
            }
        }
        else
        {
            connectCount = 0;
        }
        break;
    case GL_TRIANGLES:
        connectCount = vertexCount % 3;
        vertexCount -= connectCount;
        for (i = connectCount; i > 0; i--)
        {
            gc->input.connectVertexIndex[connectCount - i] = endIndex - i;
        }
        break;
    case GL_TRIANGLE_STRIP:
        if (vertexCount > 3)
        {
            connectCount = vertexCount & 1;
            vertexCount -= connectCount;
            connectCount += 2;
        }
        else
        {
            connectCount = vertexCount;
            vertexCount = 0;
        }
        for (i = connectCount; i > 0; i--)
        {
            gc->input.connectVertexIndex[connectCount - i] = endIndex - i;
        }
        break;
    case GL_TRIANGLE_FAN:
    case GL_POLYGON:
        if (vertexCount >= 2)
        {
            connectCount = 2;
            gc->input.connectVertexIndex[0] = startIndex;
            gc->input.connectVertexIndex[1] = endIndex - 1;
        }
        else
        {
            connectCount = vertexCount;
            vertexCount = 0;
            gc->input.connectVertexIndex[0] = endIndex - 1;
        }
        break;
    case GL_QUADS:
        connectCount = vertexCount % 4;
        vertexCount -= connectCount;
        for (i = connectCount; i > 0; i--)
        {
            gc->input.connectVertexIndex[connectCount - i] = endIndex - i;
        }
        break;
    case GL_QUAD_STRIP:
        if (vertexCount >= 4)
        {
            connectCount = vertexCount % 2;
            vertexCount -= connectCount;
            connectCount += 2;
        }
        else
        {
            connectCount = vertexCount;
            vertexCount = 0;
        }
        for (i = connectCount; i > 0; i--)
        {
            gc->input.connectVertexIndex[connectCount - i] = endIndex - i;
        }
        break;

    case GL_LINES_ADJACENCY_EXT:
    case GL_LINE_STRIP_ADJACENCY_EXT:
    case GL_TRIANGLES_ADJACENCY_EXT:
    case GL_TRIANGLE_STRIP_ADJACENCY_EXT:
        GL_ASSERT(0);
        break;
    }
    gc->input.vertex.index = startIndex + vertexCount;
    gc->input.connectVertexCount = connectCount;
    gc->input.vertexTrimCount = endIndex - gc->input.vertex.index;

    if (gc->input.indexPrimEnabled &&
        vertexCount >= minVertexNumber[gc->input.currentPrimMode])
    {
        __glGenerateVertexIndex(gc);
    }

    if (gc->input.vertex.index > 0)
    {
        __glDrawImmedPrimitive(gc);
    }
    __glImmedUpdateVertexState(gc);

    /* If the cached vertex format is different from the current vertex format
    ** then there is no need to enable cache buffer for the continued batch.
    */
    enableCache = GL_FALSE;
    if (gc->input.enableVertexCaching &&
        gc->input.inconsistentFormat == GL_FALSE &&
        gc->input.currentVertexCache->primitiveFormat == gc->input.primitiveFormat)
    {
        enableCache = GL_TRUE;
    }

    __glResetImmedVertexBuffer(gc, enableCache);

    /* If next primitive uses cached buffer then we have to check if the connection vertices are
    ** the same as the current primitive batch.
    */
    if (gc->input.cacheBufferUsed)
    {
        GLuint result = 0;
        if (connectCount > 0)
        {
            if (((gc->input.currentVertexCache->vertexInfoBuffer->inputTag & __GL_VERTEX_DATA_TAG_MASK) ||
                 (gc->input.currentVertexCache->vertexInfoBuffer->inputTag == __GL_END_TAG)) &&
                 (gc->input.currentVertexCache->vertexInfoBuffer->offsetDW == connectCount * gc->input.vertTotalStrideDW))
            {
                for (i = 0; i < connectCount; i++)
                {
                    result |= __GL_MEMCMP(gc->input.currentVertexCache->vertexDataBuffer + i * gc->input.vertTotalStrideDW,
                        gc->input.defaultDataBuffer + gc->input.connectVertexIndex[i] * gc->input.vertTotalStrideDW,
                        (gc->input.vertTotalStrideDW << 2));
                }
            }
            else
            {
                result = 1;
            }
        }
        if (result == 0)
        {
            return;
        }
        /* If the connection vertices are different then reset to default vertex buffer */
        __glResetImmedVertexBuffer(gc, GL_FALSE);
    }

    /* Copy the batch connection vertices to the beginnig of vertexDataBuffer */
    for (i = 0; i < connectCount; i++)
    {
        __GL_MEMCOPY(gc->input.defaultDataBuffer + i * gc->input.vertTotalStrideDW,
            gc->input.defaultDataBuffer + gc->input.connectVertexIndex[i] * gc->input.vertTotalStrideDW,
            (gc->input.vertTotalStrideDW << 2));
    }

    /* There is no need to copy the corresponding __GLvertexInfo to the beginning of
    ** vertexInfoBuffer since the connecting vertices will be compared in previous batch.
    */

    /* Copy the last couple of edgeflags (connectCount) to the beginnig of edgeflag buffer */
    if (gc->input.primInputMask & __GL_INPUT_EDGEFLAG)
    {
        GLboolean* edgeaddr = gc->input.edgeflag.pointer;
        for(i = 0; i < connectCount; i++)
        {
            edgeaddr[i] = edgeaddr[gc->input.connectVertexIndex[i]];
        }
    }

    /* There is no need to offset gc->input.currentInfoBufPtr like gc->input.currentDataBufPtr */
    gc->input.connectPrimMode = gc->input.currentPrimMode;
    gc->input.primBeginAddr = gc->input.defaultDataBuffer;
    gc->input.currentDataBufPtr = gc->input.defaultDataBuffer + connectCount * gc->input.vertTotalStrideDW;

    /* Restore the previous primElemSequence and preVertexFormat to continue vertex accumulation */
    gc->input.primElemSequence = primElemSequence;
    if (gc->input.inconsistentFormat == GL_FALSE)
    {
        gc->input.preVertexFormat = gc->input.primitiveFormat;
    }

    /* Reset all the input pointers to the new locations */
    i = 0;
    inputMask = gc->input.primInputMask & (~__GL_INPUT_EDGEFLAG);
    while (inputMask)
    {
        if (inputMask & 0x1)
        {
            gc->input.currentInput[i].pointer =
                (GLubyte *)(gc->input.defaultDataBuffer + gc->input.currentInput[i].offsetDW);
            gc->input.currentInput[i].currentPtrDW =
                (GLfloat *)gc->input.currentInput[i].pointer + (connectCount-1) * gc->input.vertTotalStrideDW;
            gc->input.currentInput[i].index = connectCount;
        }
        inputMask >>= 1;
        i += 1;
    }
}

GLvoid __glConsistentFormatChange(__GLcontext *gc)
{
    GLfloat *src, *dst;
    GLuint i, j, lastVertexIndex, dataSize;
    GLuint inputMask, formatMask, inputTag;
    __GLvertexInfo *vtxinfo = NULL;

    if (gc->currentImmediateDispatch->End == __glim_End_Material)
    {
        GL_ASSERT(0);
    }

    if (gc->input.currentInfoBufPtr > gc->input.vertexInfoBuffer)
    {
        /* Back track vtxinfo until we find the __GL_BEGIN_*_TAG */
        vtxinfo = (gc->input.currentInfoBufPtr - 1);
        while (vtxinfo->inputTag > __GL_END_TAG && vtxinfo > gc->input.vertexInfoBuffer) {
            vtxinfo--;
        }

        /* Move gc->input.currentInfoBufPtr pointer back to the end of last primitive */
        gc->input.currentInfoBufPtr = vtxinfo;
    }

    __glComputePrimitiveData(gc);

    lastVertexIndex = gc->input.lastVertexIndex;

    if (gc->input.vertex.index > 0)
    {
        __glDrawImmedPrimitive(gc);
    }
    __glImmedUpdateVertexState(gc);
    __glResetImmedVertexBuffer(gc, GL_FALSE);

    /* Copy the __GL_BEGIN_*_TAG to the beginning of gc->input.defaultInfoBuffer */
    if (gc->input.currentInfoBufPtr && vtxinfo)
    {
        *gc->input.currentInfoBufPtr = *vtxinfo;
        gc->input.currentInfoBufPtr->offsetDW = 0;
        gc->input.currentInfoBufPtr++;
    }

    j = 0;
    gc->input.primBeginAddr = gc->input.currentDataBufPtr;
    formatMask = __glVertexFormat2InputMask(gc->input.vertexFormat);
    inputMask = formatMask & ~(__GL_INPUT_VERTEX | __GL_INPUT_EDGEFLAG);
    while (inputMask)
    {
        if (inputMask & 0x1)
        {
            /* Move the attributes already accumulated for the first glVertex to the new location */
            gc->input.currentInput[j].pointer = (GLubyte*)gc->input.currentDataBufPtr;
            gc->input.currentInput[j].offsetDW = (GLuint)(gc->input.currentDataBufPtr - gc->input.primBeginAddr);
            src = gc->input.currentInput[j].currentPtrDW;
            dst = (GLfloat*)gc->input.currentInput[j].pointer;
            dataSize = gc->input.currentInput[j].sizeDW;
            for (i = 0; i < dataSize; i++)
            {
                *dst++ = *src++;
            }
            gc->input.currentInput[j].currentPtrDW = (GLfloat*)gc->input.currentInput[j].pointer;
            gc->input.currentDataBufPtr += dataSize;
            inputTag = inputTagTable[j][dataSize - 1];
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, inputTag);

            /* Fill in the corresponding vertex info to gc->input.defaultInfoBuffer */
            if (gc->input.currentInfoBufPtr)
            {
                gc->input.currentInfoBufPtr->inputTag = (GLushort) inputTag;
                gc->input.currentInfoBufPtr->offsetDW = (GLushort) gc->input.currentInput[j].offsetDW;
                gc->input.currentInfoBufPtr->appDataPtr = NULL;
                gc->input.currentInfoBufPtr->ptePointer = NULL;
                gc->input.currentInfoBufPtr++;
            }
        }
        inputMask >>= 1;
        j += 1;
    }

    if (formatMask & __GL_INPUT_EDGEFLAG)
    {
        GLboolean* edgeaddr = gc->input.edgeflag.pointer;
        edgeaddr[0] = edgeaddr[lastVertexIndex];
    }

    /* Before the first glVertex, gc->input.preVertexFormat should be the current vertexFormat */
    gc->input.preVertexFormat = gc->input.vertexFormat;
}

GLvoid __glSwitchToNewPrimtiveFormat(__GLcontext *gc, GLuint attFmtIdx)
{
    GLuint64 primElemSequence = gc->input.primElemSequence;
    GLuint attInpIdx = fmtIndex2InputIndex[attFmtIdx];
    GLuint numVertex, dataSize, newTotalStrideDW, origTotalStrideDW, i, j;
    GLuint formatMask, lastVertexIndex, inputTag, col4ub = 0;
    GLuint64 inputMask;
    GLfloat *curPrimPtr, *src, *dst, *cur;
    __GLvertexInfo *vtxinfo;

    if (gc->currentImmediateDispatch->End == __glim_End_Material)
    {
        __glSwitchToNewPrimtiveFormat_Material(gc, attFmtIdx);
        return;
    }

    if (gc->input.currentInfoBufPtr > gc->input.vertexInfoBuffer)
    {
        /* Back track vtxinfo until we find the __GL_BEGIN_*_TAG */
        vtxinfo = (gc->input.currentInfoBufPtr - 1);
        while (vtxinfo->inputTag > __GL_END_TAG && vtxinfo > gc->input.vertexInfoBuffer) {
            vtxinfo--;
        }

        /* Move gc->input.currentInfoBufPtr pointer back to the end of last primitive */
        gc->input.currentInfoBufPtr = vtxinfo;
    }


    gc->input.primitiveFormat = gc->input.preVertexFormat;

    __glComputePrimitiveData(gc);

    numVertex = gc->input.vertex.index - gc->input.lastVertexIndex;
    dataSize = (numVertex + 1) * (gc->input.vertTotalStrideDW << 2);
    lastVertexIndex = gc->input.lastVertexIndex;

    /* Copy the current primitive date to a scratch area */
    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, dataSize, (gctPOINTER*)&curPrimPtr)))
    {
        return;
    }
    __GL_MEMCOPY(curPrimPtr, gc->input.primBeginAddr, dataSize);

    /* Roll back the vertex index to the previous consistent primitive */
    gc->input.vertex.index = gc->input.lastVertexIndex;

    if (gc->input.vertex.index > 0)
    {
        __glDrawImmedPrimitive(gc);
    }
    __glImmedUpdateVertexState(gc);
    __glResetImmedVertexBuffer(gc, GL_FALSE);
    gc->input.preVertexFormat = gc->input.primitiveFormat;

    /* Mark in begin */
    gc->input.beginMode = __GL_IN_BEGIN;
    gc->input.primBeginAddr = gc->input.currentDataBufPtr;

    /* Setup the input offset and pointers for the new attribute */
    gc->input.currentDataBufPtr += gc->input.vertTotalStrideDW;
    gc->input.currentInput[attInpIdx].offsetDW = (GLuint)(gc->input.currentDataBufPtr - gc->input.primBeginAddr);
    gc->input.currentInput[attInpIdx].currentPtrDW = gc->input.currentDataBufPtr;
    gc->input.currentInput[attInpIdx].pointer = (GLubyte *)gc->input.currentDataBufPtr;
    gc->input.currentInput[attInpIdx].sizeDW = dataSize = fmtIndex2DWSize[attFmtIdx];
    gc->input.currentDataBufPtr = gc->input.currentDataBufPtr + dataSize;
    inputTag = inputTagTable[attInpIdx][dataSize - 1];
    gc->input.primElemSequence = primElemSequence;
    __GL_PRIM_ELEMENT(gc->input.primElemSequence, inputTag);

    origTotalStrideDW = gc->input.vertTotalStrideDW;
    newTotalStrideDW = gc->input.vertTotalStrideDW + dataSize;

    /* Change the primitive format to include the new attribute */
    gc->input.vertex.index = numVertex;
    gc->input.vertTotalStrideDW = newTotalStrideDW;
    gc->input.primInputMask |= (__GL_ONE_64 << attInpIdx);
    gc->input.preVertexFormat |= (__GL_ONE_64 << attFmtIdx);
    gc->input.primitiveFormat = gc->input.preVertexFormat;

    /* Copy the consistent vertex data for the current primitive to the new location
    */
    src = curPrimPtr;
    dst = gc->input.primBeginAddr;
    for (i = 0; i < numVertex; i++)
    {
        __GL_MEMCOPY(dst, src, (origTotalStrideDW << 2));
        switch (dataSize)
        {
        case 1:
            if (attInpIdx == __GL_INPUT_DIFFUSE_INDEX)
            {
                if (i == 0)
                {
                    GLubyte r = __GL_FLOAT_TO_UB(gc->state.current.color.r);
                    GLubyte g = __GL_FLOAT_TO_UB(gc->state.current.color.g);
                    GLubyte b = __GL_FLOAT_TO_UB(gc->state.current.color.b);
                    GLubyte a = __GL_FLOAT_TO_UB(gc->state.current.color.a);
                    col4ub = __GL_PACK_COLOR4UB(r, g, b, a);
                }
                *(dst + origTotalStrideDW) = *(GLfloat *)&col4ub;
            }
            else
            {
                cur = (GLfloat *)&gc->state.current.currentState[attInpIdx];
                *(dst + origTotalStrideDW) = *cur;
            }
            break;
        case 2:
            cur = (GLfloat *)&gc->state.current.currentState[attInpIdx];
            *(dst + origTotalStrideDW) = *cur;
            *(dst + origTotalStrideDW + 1) = *(cur + 1);
            break;
        case 3:
            cur = (GLfloat *)&gc->state.current.currentState[attInpIdx];
            *(dst + origTotalStrideDW) = *cur;
            *(dst + origTotalStrideDW + 1) = *(cur + 1);
            *(dst + origTotalStrideDW + 2) = *(cur + 2);
            break;
        case 4:
            cur = (GLfloat *)&gc->state.current.currentState[attInpIdx];
            *(dst + origTotalStrideDW) = *cur;
            *(dst + origTotalStrideDW + 1) = *(cur + 1);
            *(dst + origTotalStrideDW + 2) = *(cur + 2);
            *(dst + origTotalStrideDW + 3) = *(cur + 3);
            break;
        }
        src += origTotalStrideDW;
        dst += newTotalStrideDW;
    }

    if (gc->input.primInputMask & __GL_INPUT_EDGEFLAG)
    {
        GLboolean *edgeaddr = gc->input.edgeflag.pointer;
        if (lastVertexIndex)
        {
            for (i = 0; i < numVertex; i++) {
                edgeaddr[i] = edgeaddr[lastVertexIndex + i];
            }
        }
    }

    /* Reset primitive element current pointers */
    i = 0;
    inputMask = gc->input.primInputMask & (~__GL_INPUT_EDGEFLAG);
    while (inputMask)
    {
        if (inputMask & 0x1)
        {
            gc->input.currentInput[i].currentPtrDW =
                (GLfloat *)gc->input.currentInput[i].pointer + newTotalStrideDW * (numVertex - 1);
        }
        inputMask >>= 1;
        i += 1;
    }

    /* Copy all the attributes already accumulated for the last glVertex to the new location */
    i = 0;
    formatMask = __glVertexFormat2InputMask(gc->input.vertexFormat);
    inputMask = formatMask & ~(__GL_INPUT_VERTEX | __GL_INPUT_EDGEFLAG);
    while (inputMask)
    {
        if (inputMask & 0x1)
        {
            src = curPrimPtr + gc->input.currentInput[i].offsetDW + numVertex * origTotalStrideDW;
            dst = gc->input.currentInput[i].currentPtrDW += newTotalStrideDW;
            for (j = 0; j < gc->input.currentInput[i].sizeDW; j++)
            {
                *(dst + j) = *(src + j);
            }
        }
        inputMask >>= 1;
        i += 1;
    }

    if (formatMask & __GL_INPUT_EDGEFLAG)
    {
        GLboolean *edgeaddr = gc->input.edgeflag.pointer;
        if (lastVertexIndex) {
            edgeaddr[numVertex] = edgeaddr[lastVertexIndex + numVertex];
        }
    }

    /* Free the scratch memory */
    gcmOS_SAFE_FREE(gcvNULL, curPrimPtr);

    /* Turn off vertex data caching if the primitive has changed its original format */
    gc->input.vertexCacheEnabled = GL_FALSE;
}

GLvoid __glSwitchToInconsistentFormat(__GLcontext *gc)
{
    GLuint numVertex, dataSize, newTotalStrideDW, origTotalStrideDW, i, j, k;
    GLuint formatMask, lastVertexIndex, inputTag;
    GLuint64 inputMask;
    GLfloat *curPrimPtr, *src, *dst;
    GLuint inputOffsetDW[__GL_TOTAL_VERTEX_ATTRIBUTES] = {0};
    GLuint inputSize[__GL_TOTAL_VERTEX_ATTRIBUTES] = {0};
    __GLvertexInfo *vtxinfo;

    if (gc->immedModeDispatch.End == __glim_End_Material)
    {
         __glSwitchToInconsistentFormat_Material(gc);
         return;
    }

    if (gc->input.currentInfoBufPtr > gc->input.vertexInfoBuffer)
    {
        /* Back track vtxinfo until we find the __GL_BEGIN_*_TAG */
        vtxinfo = (gc->input.currentInfoBufPtr - 1);
        while (vtxinfo->inputTag > __GL_END_TAG && vtxinfo > gc->input.vertexInfoBuffer) {
            vtxinfo--;
        }

        /* Move gc->input.currentInfoBufPtr pointer back to the end of last primitive */
        gc->input.currentInfoBufPtr = vtxinfo;
    }

    gc->input.primitiveFormat = gc->input.preVertexFormat;

    __glComputePrimitiveData(gc);

    numVertex = gc->input.vertex.index - gc->input.lastVertexIndex;
    dataSize = (numVertex + 1) * (gc->input.vertTotalStrideDW << 2);
    lastVertexIndex = gc->input.lastVertexIndex;

    /* Copy the current primitive date to a scratch area */
    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, dataSize, (gctPOINTER*)&curPrimPtr)))
    {
        return;
    }
    __GL_MEMCOPY(curPrimPtr, gc->input.primBeginAddr, dataSize);

    /* Roll back the vertex index to the previous consistent primitive */
    gc->input.vertex.index = gc->input.lastVertexIndex;

    if (gc->input.vertex.index > 0)
    {
        __glDrawImmedPrimitive(gc);
    }
    __glImmedUpdateVertexState(gc);
    __glResetImmedVertexBuffer(gc, GL_FALSE);

    gc->input.primBeginAddr = gc->input.currentDataBufPtr;

    /* Set the new offsetDW and pointers */
    inputMask = gc->input.requiredInputMask & ~(__GL_INPUT_EDGEFLAG);
    newTotalStrideDW = j = 0;
    while (inputMask)
    {
        if (inputMask & 0x1) {
            inputSize[j] = gc->input.currentInput[j].sizeDW;
            inputOffsetDW[j] = gc->input.currentInput[j].offsetDW;
            gc->input.currentInput[j].pointer = (GLubyte*)gc->input.currentDataBufPtr;
            gc->input.currentInput[j].currentPtrDW = gc->input.currentDataBufPtr;
            gc->input.currentInput[j].offsetDW = (GLuint)(gc->input.currentDataBufPtr - gc->input.primBeginAddr);
            gc->input.currentInput[j].index = 0;
            dataSize = input2MaxElementSize[j];
            gc->input.currentInput[j].sizeDW = dataSize;
            gc->input.currentDataBufPtr += dataSize;
            newTotalStrideDW += dataSize;
            inputTag = inputTagTable[j][dataSize - 1];
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, inputTag);
        }
        inputMask >>= 1;
        j += 1;
    }
    origTotalStrideDW = gc->input.vertTotalStrideDW;
    gc->input.vertTotalStrideDW = newTotalStrideDW;

    /* Copy the consistent vertex data for the current primitive to the new location
    ** based on gc->input.preVertexFormat
    */
    j = 0;
    formatMask = __glVertexFormat2InputMask(gc->input.primitiveFormat);
    inputMask = formatMask & ~(__GL_INPUT_EDGEFLAG);
    while (inputMask)
    {
        if (inputMask & 0x1)
        {
            src = curPrimPtr + inputOffsetDW[j];
            dst = gc->input.currentInput[j].currentPtrDW;
            dataSize = input2MaxElementSize[j];
            if (j == __GL_INPUT_DIFFUSE_INDEX && inputSize[j] == 1)
            {
                for (k = 0; k < numVertex; k++)
                {
                    GLubyte *ubcolor = (GLubyte *)src;
                    *(dst) = __GL_UB_TO_FLOAT(*(ubcolor));              /* R */
                    *(dst + 1) = __GL_UB_TO_FLOAT(*(ubcolor + 1));      /* G */
                    *(dst + 2) = __GL_UB_TO_FLOAT(*(ubcolor + 2));      /* B */
                    *(dst + 3) = __GL_UB_TO_FLOAT(*(ubcolor + 3));      /* A */
                    src += origTotalStrideDW;
                    dst += newTotalStrideDW;
                }
            }
            else
            {
                for (k = 0; k < numVertex; k++)
                {
                    if (dataSize == 4)
                    {
                        *(dst) = 0.0;
                        *(dst + 1) = 0.0;
                        *(dst + 2) = 0.0;
                        *(dst + 3) = 1.0;
                    }
                    for (i = 0; i < inputSize[j]; i++)
                    {
                        *(dst + i) = *(src + i);
                    }
                    src += origTotalStrideDW;
                    dst += newTotalStrideDW;
                }
            }
            gc->input.currentInput[j].currentPtrDW = dst - newTotalStrideDW;
            gc->input.currentInput[j].index = numVertex;
        }
        inputMask >>= 1;
        j += 1;
    }

    if (formatMask & __GL_INPUT_EDGEFLAG)
    {
        GLboolean *edgeaddr = gc->input.edgeflag.pointer;
        if (lastVertexIndex)
        {
            for (i = 0; i < numVertex; i++)
            {
                edgeaddr[i] = edgeaddr[lastVertexIndex + i];
            }
        }
        gc->input.edgeflag.index = numVertex;
    }

    /* Copy all the attributes already accumulated for the last glVertex to the new location */
    j = 0;
    formatMask = __glVertexFormat2InputMask(gc->input.vertexFormat);
    inputMask = formatMask & ~(__GL_INPUT_VERTEX | __GL_INPUT_EDGEFLAG);
    gc->input.vertexFormat = 0;
    while (inputMask)
    {
        if (inputMask & 0x1)
        {
            gc->input.vertexFormat |= input2InconsistFormat[j];
            src = curPrimPtr + inputOffsetDW[j] + numVertex * origTotalStrideDW;
            dst = gc->input.currentInput[j].currentPtrDW += newTotalStrideDW;
            dataSize = input2MaxElementSize[j];
            if (j == __GL_INPUT_DIFFUSE_INDEX && inputSize[j] == 1)
            {
                GLubyte *ubcolor = (GLubyte *)src;
                *(dst) = __GL_UB_TO_FLOAT(*(ubcolor));              /* R */
                *(dst + 1) = __GL_UB_TO_FLOAT(*(ubcolor + 1));      /* G */
                *(dst + 2) = __GL_UB_TO_FLOAT(*(ubcolor + 2));      /* B */
                *(dst + 3) = __GL_UB_TO_FLOAT(*(ubcolor + 3));      /* A */
            }
            else {
                if (dataSize == 4)
                {
                    *(dst) = 0.0;
                    *(dst + 1) = 0.0;
                    *(dst + 2) = 0.0;
                    *(dst + 3) = 1.0;
                }
                for (i = 0; i < inputSize[j]; i++)
                {
                    *(dst + i) = *(src + i);
                }
            }
            gc->input.currentInput[j].index += 1;
        }
        inputMask >>= 1;
        j += 1;
    }

    if (formatMask & __GL_INPUT_EDGEFLAG)
    {
        GLboolean* edgeaddr = gc->input.edgeflag.pointer;
        if (lastVertexIndex) {
            edgeaddr[numVertex] = edgeaddr[lastVertexIndex + numVertex];
        }
        gc->input.edgeflag.index += 1;
        gc->input.vertexFormat |= __GL_EDGEFLAG_BIT;
    }

    __glDuplicateVertexAttributes(gc);

    /* Free the scratch memory */
    gcmOS_SAFE_FREE(gcvNULL, curPrimPtr);

    gc->input.preVertexFormat = 0;
    gc->input.primitiveFormat = __glInputMask2InconsisFormat(gc->input.requiredInputMask);
    gc->input.inconsistentFormat = GL_TRUE;

    /* Turn off vertex data caching if the primitive has inconsistent format */
    gc->input.vertexCacheEnabled = GL_FALSE;
}

void __glPrimitiveBatchEnd(__GLcontext *gc)
{
    if (gc->input.cacheBufferUsed)
    {
        /* Copy the global variable gc->pCurrentInfoBufPtr back to gc->input.currentInfoBufPtr.
        */
        gc->input.currentInfoBufPtr = gc->pCurrentInfoBufPtr;

        if (gc->input.currentInfoBufPtr->inputTag == __GL_BATCH_END_TAG)
        {
            gc->input.vertex.index = gc->input.currentVertexCache->vertexCount;
            gc->input.indexCount = gc->input.currentVertexCache->indexCount;
            __glDrawImmedPrimitive(gc);
            __glImmedUpdateVertexState(gc);
        }
        else if (gc->input.currentInfoBufPtr > gc->input.vertexInfoBuffer &&
                 gc->input.currentInfoBufPtr->offsetDW > 0)
        {
            __glComputeCacheBufVertexCount(gc);
            __glDrawImmedPrimitive(gc);
            __glImmedUpdateVertexState(gc);
        }
    }
    else
    {
        __glComputePrimitiveData(gc);
        if (gc->input.vertex.index > 0)
        {
            __glDrawImmedPrimitive(gc);
        }
        __glImmedUpdateVertexState(gc);
    }

    __glResetImmedVertexBuffer(gc, gc->input.enableVertexCaching);
}

GLvoid APIENTRY __glim_Begin(__GLcontext *gc, GLenum mode)
{
    GLint lastIndex;

    if (gc->conditionalRenderDiscard)
    {
        return;
    }

    if (mode > GL_TRIANGLE_STRIP_ADJACENCY_EXT)
    {
        __glSetError(gc,GL_INVALID_ENUM);
        return;
    }

    switch (gc->input.beginMode)
    {
    case __GL_NOT_IN_BEGIN:

        if (gc->input.deferredAttribDirty) {
            __glCopyDeferedAttribToCurrent(gc);
        }
        break;

    case __GL_IN_BEGIN:

        __glSetError(gc, GL_INVALID_OPERATION);
        return;

    case __GL_SMALL_LIST_BATCH:

        __glDisplayListBatchEnd(gc);
        break;

    case __GL_SMALL_DRAW_BATCH:

        if (gc->input.deferredAttribDirty)
        {
            /* If there are deferred attribute changes, we have to flush the vertex buffer
            ** and then copy the deferred attribute states to current attribute state.
            */
            if (gc->input.deferredAttribDirty & (__GL_DEFERED_ATTRIB_BIT | __GL_DEFERED_COLOR_MASK_BIT))
            {
                __glPrimitiveBatchEnd(gc);
                __glUpdateDeferedAttributes(gc);
                goto New_Begin;
            }

            if (gc->input.deferredAttribDirty & __GL_DEFERED_NORMAL_BIT &&
                !(gc->input.primitiveFormat & __GL_N3F_BIT))
            {
                /* If previous primitive has no normal (but needs it) in glBegin/glEnd
                ** and normal is really changed after glEnd then the vertex buffer has to be
                ** flushed before the current normal is set.
                */
                if (gc->state.current.normal.f.x != gc->input.shadowCurrent.normal.f.x ||
                    gc->state.current.normal.f.y != gc->input.shadowCurrent.normal.f.y ||
                    gc->state.current.normal.f.z != gc->input.shadowCurrent.normal.f.z)
                {
                    __glPrimitiveBatchEnd(gc);
                    goto New_Begin;
                }

                gc->input.deferredAttribDirty &= ~(__GL_DEFERED_NORMAL_BIT);
            }

            if (gc->input.deferredAttribDirty & __GL_DEFERED_COLOR_BIT &&
                !(gc->input.primitiveFormat & (__GL_C3F_BIT | __GL_C4F_BIT | __GL_C4UB_BIT)))
            {
                /* If previous primitive has no color (but needs it) in glBegin/glEnd
                ** and color is really changed after glEnd then the verex buffer has to be
                ** flushed before the current color is set.
                */
                if (gc->state.current.color.r != gc->input.shadowCurrent.color.r ||
                    gc->state.current.color.g != gc->input.shadowCurrent.color.g ||
                    gc->state.current.color.b != gc->input.shadowCurrent.color.b ||
                    gc->state.current.color.a != gc->input.shadowCurrent.color.a)
                {
                    __glPrimitiveBatchEnd(gc);
                    goto New_Begin;
                }

                gc->input.deferredAttribDirty &= ~(__GL_DEFERED_COLOR_BIT);
            }
        }

        if (gc->input.primMode == mode) {
            goto Continue_Begin;
        }

        switch (gc->input.primMode)
        {
        case GL_TRIANGLE_STRIP:
        case GL_TRIANGLE_FAN:
        case GL_QUAD_STRIP:
        case GL_POLYGON:
            if (mode >= GL_TRIANGLES && gc->state.polygon.bothFaceFill) {
                goto Continue_Begin;
            }
            __glPrimitiveBatchEnd(gc);
            goto New_Begin;

        case GL_TRIANGLES:
        case GL_QUADS:
            if (gc->input.vertex.index < 200 && gc->input.indexBuffer &&
                mode >= GL_TRIANGLES && gc->state.polygon.bothFaceFill)
            {
                gc->input.currentPrimMode = gc->input.primMode;
                gc->input.indexPrimEnabled = GL_TRUE;
                lastIndex = gc->input.lastVertexIndex;
                gc->input.lastVertexIndex = 0;
                __glGenerateVertexIndex(gc);
                gc->input.lastVertexIndex = lastIndex;
                gc->input.primMode = GL_TRIANGLE_STRIP;

                goto Continue_Begin;
            }
            __glPrimitiveBatchEnd(gc);
            goto New_Begin;

        case GL_LINE_STRIP:
        case GL_LINE_LOOP:
            if (mode >= GL_LINES && mode <= GL_LINE_STRIP) {
                goto Continue_Begin;
            }
            __glPrimitiveBatchEnd(gc);
            goto New_Begin;

        case GL_LINES:
            if (gc->input.vertex.index < 200 && gc->input.indexBuffer &&
                mode >= GL_LINE_LOOP && mode <= GL_LINE_STRIP)
            {
                gc->input.currentPrimMode = GL_LINES;
                gc->input.indexPrimEnabled = GL_TRUE;
                lastIndex = gc->input.lastVertexIndex;
                gc->input.lastVertexIndex = 0;
                __glGenerateVertexIndex(gc);
                gc->input.lastVertexIndex = lastIndex;
                gc->input.primMode = GL_LINE_STRIP;

                goto Continue_Begin;
            }
            __glPrimitiveBatchEnd(gc);
            goto New_Begin;

        default:
            __glPrimitiveBatchEnd(gc);
            goto New_Begin;
        }

Continue_Begin:

        gc->input.currentPrimMode = mode;
        gc->input.beginMode = __GL_IN_BEGIN;
        gc->input.preVertexFormat = gc->input.primitiveFormat;
        gc->input.primBeginAddr = gc->input.currentDataBufPtr;

        /* Switch to inside Begin/End dispatch table.
        */
        __glSwitchImmediateDispatch(gc, gc->input.pCurrentImmedModeDispatch);
        return;

    default:

        GL_ASSERT(0);
    }

New_Begin:

    gc->input.primMode = gc->input.currentPrimMode = mode;
    gc->input.beginMode = __GL_IN_BEGIN;
    gc->input.vertexFormat = 0;
    gc->input.preVertexFormat = 0;
    gc->input.primBeginAddr = gc->input.currentDataBufPtr;

    __glValidateImmedBegin(gc, mode);

    /* Switch to inside Begin/End dispatch table.
    */
    __glSwitchImmediateDispatch(gc, gc->input.pCurrentImmedModeDispatch);
}

GLvoid APIENTRY __glim_End(__GLcontext *gc )
{
    GLuint internalEnd = GL_FALSE;
    GLuint discardVertexNum, i, mask;
    GLint vertexCount;

    if (gc->conditionalRenderDiscard)
    {
        return;
    }

    if (gc->input.inconsistentFormat == GL_FALSE)
    {
        gc->input.primitiveFormat = gc->input.preVertexFormat;
        gc->input.preVertexFormat = 0;
    }

    vertexCount = gc->input.vertex.index - gc->input.lastVertexIndex;

    /* Discard the primitive if its vertex number is less than the minimum number.
     */
    discardVertexNum = 0;
    if (vertexCount < minVertexNumber[gc->input.currentPrimMode])
    {
        discardVertexNum = vertexCount;
    }
    else
    {
        switch (gc->input.currentPrimMode)
        {
        case GL_TRIANGLES:
            discardVertexNum = vertexCount % 3;
            break;
        case GL_LINES:
            discardVertexNum = vertexCount % 2;
            break;
        case GL_QUADS:
            discardVertexNum = vertexCount % 4;
            break;
        case GL_QUAD_STRIP:
            discardVertexNum = vertexCount % 2;
            break;
        }
    }

    if (discardVertexNum)
    {
        /* Roll back the vertex index and currentPtrs by "discardVertexNum" */
        gc->input.vertex.index -= discardVertexNum;

        /* Compute gc->input.primInputMask */
        __glComputePrimitiveData(gc);

        i = 0;
        mask = gc->input.primInputMask & (~__GL_INPUT_EDGEFLAG);
        while (mask) {
            if (mask & 0x1) {
                gc->input.currentInput[i].currentPtrDW -= discardVertexNum * gc->input.vertTotalStrideDW;
            }
            mask >>= 1;
            i += 1;
        }
    }

    if (gc->input.indexPrimEnabled)
    {
        __glGenerateVertexIndex(gc);
    }
    else
    {
        internalEnd = internalEndTable[gc->input.primMode];
    }

    if (gc->input.inconsistentFormat || internalEnd || gc->input.vertexFormat)
    {
        __glPrimitiveBatchEnd(gc);
    }

    gc->input.primBeginAddr = NULL;
    gc->input.inconsistentFormat = GL_FALSE;
    gc->input.currentDataBufPtr = gc->input.vertexDataBuffer + gc->input.vertex.index * gc->input.vertTotalStrideDW;
    gc->input.lastVertexIndex = gc->input.vertex.index;
    if (gc->input.vertex.index == 0)
    {
        gc->input.primElemSequence = 0;
    }

    /* Switch to outside Begin/End dispatch table.
    */
    __glSwitchImmediateDispatch(gc, &gc->immedModeOutsideDispatch);

    if (gc->input.beginMode == __GL_IN_BEGIN && gc->input.vertex.index)
    {
        gc->input.beginMode = __GL_SMALL_DRAW_BATCH;
    }
    else
    {
        gc->input.beginMode = __GL_NOT_IN_BEGIN;
    }
}

void APIENTRY __glim_Begin_Info(__GLcontext *gc, GLenum mode)
{
    GLint lastIndex;
    __GLvertexInfo *vtxinfo;

    if (gc->conditionalRenderDiscard)
    {
        return;
    }

    if (mode > GL_TRIANGLE_STRIP_ADJACENCY_EXT)
    {
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }

    switch (gc->input.beginMode)
    {
    case __GL_NOT_IN_BEGIN:

        if (gc->input.deferredAttribDirty) {
            __glCopyDeferedAttribToCurrent(gc);
        }
        break;

    case __GL_IN_BEGIN:

        __glSetError(gc, GL_INVALID_OPERATION);
        return;

    case __GL_SMALL_LIST_BATCH:

        __glDisplayListBatchEnd(gc);
        break;

    case __GL_SMALL_DRAW_BATCH:

        if (gc->input.deferredAttribDirty)
        {
            /* If there are deferred attribute changes, we have to flush the vertex buffer
            ** and then copy the deferred attribute states to current attribute state.
            */
            if (gc->input.deferredAttribDirty & (__GL_DEFERED_ATTRIB_BIT | __GL_DEFERED_COLOR_MASK_BIT))
            {
                __glPrimitiveBatchEnd(gc);
                __glUpdateDeferedAttributes(gc);
                goto New_Begin;
            }

            if (gc->input.deferredAttribDirty & __GL_DEFERED_NORMAL_BIT &&
                !(gc->input.primitiveFormat & __GL_N3F_BIT))
            {
                /* If previous primitive has no normal (but needs it) in glBegin/glEnd
                ** and normal is really changed after glEnd then the vertex buffer has to be
                ** flushed before the current normal is set.
                */
                if (gc->state.current.normal.f.x != gc->input.shadowCurrent.normal.f.x ||
                    gc->state.current.normal.f.y != gc->input.shadowCurrent.normal.f.y ||
                    gc->state.current.normal.f.z != gc->input.shadowCurrent.normal.f.z)
                {
                    __glPrimitiveBatchEnd(gc);
                    goto New_Begin;
                }

                gc->input.deferredAttribDirty &= ~(__GL_DEFERED_NORMAL_BIT);
            }

            if (gc->input.deferredAttribDirty & __GL_DEFERED_COLOR_BIT &&
                !(gc->input.primitiveFormat & (__GL_C3F_BIT | __GL_C4F_BIT | __GL_C4UB_BIT)))
            {
                /* If previous primitive has no color (but needs it) in glBegin/glEnd
                ** and color is really changed after glEnd then the vertex buffer has to be
                ** flushed before the current color is set.
                */
                if (gc->state.current.color.r != gc->input.shadowCurrent.color.r ||
                    gc->state.current.color.g != gc->input.shadowCurrent.color.g ||
                    gc->state.current.color.b != gc->input.shadowCurrent.color.b ||
                    gc->state.current.color.a != gc->input.shadowCurrent.color.a)
                {
                    __glPrimitiveBatchEnd(gc);
                    goto New_Begin;
                }

                gc->input.deferredAttribDirty &= ~(__GL_DEFERED_COLOR_BIT);
            }
        }

        if (gc->input.primMode == mode)
        {
            goto Continue_Begin;
        }

        switch (gc->input.primMode)
        {
        case GL_TRIANGLE_STRIP:
        case GL_TRIANGLE_FAN:
        case GL_QUAD_STRIP:
        case GL_POLYGON:
            if (mode >= GL_TRIANGLES && gc->state.polygon.bothFaceFill) {
                goto Continue_Begin;
            }
            __glPrimitiveBatchEnd(gc);
            goto New_Begin;

        case GL_TRIANGLES:
        case GL_QUADS:
            if (gc->input.vertex.index < 200 && gc->input.indexBuffer &&
                mode >= GL_TRIANGLES && gc->state.polygon.bothFaceFill)
            {
                gc->input.currentPrimMode = gc->input.primMode;
                gc->input.indexPrimEnabled = GL_TRUE;
                lastIndex = gc->input.lastVertexIndex;
                gc->input.lastVertexIndex = 0;
                __glGenerateVertexIndex(gc);
                gc->input.lastVertexIndex = lastIndex;
                gc->input.primMode = GL_TRIANGLE_STRIP;

                goto Continue_Begin;
            }
            __glPrimitiveBatchEnd(gc);
            goto New_Begin;

        case GL_LINE_STRIP:
        case GL_LINE_LOOP:
            if (mode >= GL_LINES && mode <= GL_LINE_STRIP) {
                goto Continue_Begin;
            }
            __glPrimitiveBatchEnd(gc);
            goto New_Begin;

        case GL_LINES:
            if (gc->input.vertex.index < 200 && gc->input.indexBuffer &&
                mode >= GL_LINE_LOOP && mode <= GL_LINE_STRIP)
            {
                gc->input.currentPrimMode = GL_LINES;
                gc->input.indexPrimEnabled = GL_TRUE;
                lastIndex = gc->input.lastVertexIndex;
                gc->input.lastVertexIndex = 0;
                __glGenerateVertexIndex(gc);
                gc->input.lastVertexIndex = lastIndex;
                gc->input.primMode = GL_LINE_STRIP;

                goto Continue_Begin;
            }
            __glPrimitiveBatchEnd(gc);
            goto New_Begin;

        default:
            __glPrimitiveBatchEnd(gc);
            goto New_Begin;
        }

Continue_Begin:

        gc->input.currentPrimMode = mode;
        gc->input.beginMode = __GL_IN_BEGIN;
        gc->input.preVertexFormat = gc->input.primitiveFormat;
        gc->input.primBeginAddr = gc->input.currentDataBufPtr;
        gc->input.vertexFormat = 0;

        /* Save the __GL_BEGIN_PRIM_TAG in gc->input.vertexInfoBuffer.
        */
        vtxinfo = gc->input.currentInfoBufPtr++;
        vtxinfo->inputTag = (mode | 0x10);
        vtxinfo->offsetDW = (GLuint)(gc->input.currentDataBufPtr - gc->input.vertexDataBuffer);
        vtxinfo->appDataPtr = NULL;
        vtxinfo->ptePointer = NULL;

        /* Switch to inside Begin/End dispatch table.
        */
        __glSwitchImmediateDispatch(gc, gc->input.pCurrentImmedModeDispatch);

        return;

    default:

        GL_ASSERT(0);
    }

New_Begin:

    /* Jump to __glim_Begin_Cache function if cache buffer is used.
    */
    if (gc->input.cacheBufferUsed) {
        (*gc->currentImmediateDispatch->Begin)(gc, mode);
        return;
    }

    gc->input.primMode = gc->input.currentPrimMode = mode;
    gc->input.beginMode = __GL_IN_BEGIN;
    gc->input.vertexFormat = 0;
    gc->input.preVertexFormat = 0;
    gc->input.primBeginAddr = gc->input.currentDataBufPtr;
    gc->input.primElemSequence = 0;

    __glValidateImmedBegin(gc, mode);

    /* Save the __GL_BEGIN_PRIM_TAG in gc->input.vertexInfoBuffer.
    */
    vtxinfo = gc->input.currentInfoBufPtr++;
    vtxinfo->inputTag = (mode | 0x10);
    vtxinfo->offsetDW = (GLushort)(gc->input.currentDataBufPtr - gc->input.vertexDataBuffer);
    vtxinfo->appDataPtr = NULL;
    vtxinfo->ptePointer = NULL;

    /* Switch to inside Begin/End dispatch table.
    */
    __glSwitchImmediateDispatch(gc, gc->input.pCurrentImmedModeDispatch);
}

void APIENTRY __glim_End_Info(__GLcontext *gc)
{
    GLuint internalEnd = GL_FALSE;
    GLuint discardVertexNum, i, mask;
    GLint vertexCount;
    __GLvertexInfo *vtxinfo;

    if (gc->input.inconsistentFormat == GL_FALSE)
    {
        gc->input.primitiveFormat = gc->input.preVertexFormat;
        gc->input.preVertexFormat = 0;
    }

    vertexCount = gc->input.vertex.index - gc->input.lastVertexIndex;

    /* Discard the primitive if its vertex number is less than the minimum number.
     */
    discardVertexNum = 0;
    if (vertexCount < minVertexNumber[gc->input.currentPrimMode])
    {
        discardVertexNum = vertexCount;
    }
    else
    {
        switch (gc->input.currentPrimMode)
        {
        case GL_TRIANGLES:
            discardVertexNum = vertexCount % 3;
            break;
        case GL_LINES:
            discardVertexNum = vertexCount % 2;
            break;
        case GL_QUADS:
            discardVertexNum = vertexCount % 4;
            break;
        case GL_QUAD_STRIP:
            discardVertexNum = vertexCount % 2;
            break;
        }
    }
    if (discardVertexNum)
    {
         /* Roll back the vertex index and currentPtrs by "discardVertexNum" */
        gc->input.vertex.index -= discardVertexNum;

        /* Compute gc->input.primInputMask */
        __glComputePrimitiveData(gc);

        i = 0;
        mask = gc->input.primInputMask & (~__GL_INPUT_EDGEFLAG);
        while (mask)
        {
            if (mask & 0x1)
            {
                gc->input.currentInput[i].currentPtrDW -= discardVertexNum * gc->input.vertTotalStrideDW;
            }
            mask >>= 1;
            i += 1;
        }
    }

    gc->input.currentDataBufPtr = gc->input.vertexDataBuffer + gc->input.vertex.index * gc->input.vertTotalStrideDW;

    /* Save the __GL_END_TAG in gc->input.vertexInfoBuffer.
    */
    vtxinfo = gc->input.currentInfoBufPtr++;
    vtxinfo->inputTag = __GL_END_TAG;
    vtxinfo->offsetDW = (GLushort)(gc->input.currentDataBufPtr - gc->input.vertexDataBuffer);
    vtxinfo->appDataPtr = NULL;
    vtxinfo->ptePointer = NULL;

    if (gc->input.indexPrimEnabled)
    {
        __glGenerateVertexIndex(gc);
    }
    else
    {
        internalEnd = internalEndTable[gc->input.primMode];
    }

    if (gc->input.inconsistentFormat || internalEnd || gc->input.vertexFormat)
    {
        __glPrimitiveBatchEnd(gc);
    }

    gc->input.primBeginAddr = NULL;
    gc->input.inconsistentFormat = GL_FALSE;
    gc->input.lastVertexIndex = gc->input.vertex.index;
    if (gc->input.vertex.index == 0)
    {
        gc->input.primElemSequence = 0;
    }

    /* Switch to outside Begin/End dispatch table.
    */
    __glSwitchImmediateDispatch(gc, &gc->immedModeOutsideDispatch);

    if (gc->input.beginMode == __GL_IN_BEGIN && gc->input.vertex.index)
    {
        gc->input.beginMode = __GL_SMALL_DRAW_BATCH;
    }
    else
    {
        gc->input.beginMode = __GL_NOT_IN_BEGIN;
    }
}


GLuint __glInitVertexInputState(__GLcontext *gc)
{
    if (!gc->input.defaultDataBuffer)
    {
        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, __GL_DEFAULT_VERTEX_BUFFER_SIZE, (gctPOINTER*)&gc->input.defaultDataBuffer)))
        {
            goto Failed;
        }

        gc->input.defaultDataBufEnd = gc->input.defaultDataBuffer +
            ((__GL_DEFAULT_VERTEX_BUFFER_SIZE - __GL_DEFAULT_BUFFER_END_ZONE) >> 2);
        gc->input.vertexDataBuffer = gc->input.defaultDataBuffer;
    }

    if (!gc->input.defaultIndexBuffer)
    {
        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, 3 * __GL_MAX_VERTEX_NUMBER * sizeof(GLushort), (gctPOINTER*)&gc->input.defaultIndexBuffer)))
        {
            goto Failed;
        }
        gc->input.indexBuffer = gc->input.defaultIndexBuffer;
    }

    if (!gc->input.edgeflag.pointer)
    {
        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, __GL_MAX_VERTEX_NUMBER * sizeof(GLubyte), (gctPOINTER*)&gc->input.edgeflag.pointer)))
        {
            goto Failed;
        }
    }

    if (gc->input.origVertexCacheFlag == GL_TRUE)
    {
        if (!gc->input.defaultInfoBuffer)
        {
            if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, __GL_MAX_VERTEX_NUMBER * sizeof(__GLvertexInfo) * 10, (gctPOINTER*)&gc->input.defaultInfoBuffer)))
            {
                goto Failed;
            }
            gcoOS_ZeroMemory(gc->input.defaultInfoBuffer, __GL_MAX_VERTEX_NUMBER * sizeof(__GLvertexInfo) * 10);
            gc->input.vertexInfoBuffer = gc->input.defaultInfoBuffer;
        }

        if (!gc->input.vertexCacheBlock)
        {
            /* Allocate one extra cache slot to avoid out-of-bound read error in __glResetImmedVertexBuffer.
            */
            if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof(__GLvertexCacheBlock), (gctPOINTER*)&gc->input.vertexCacheBlock)))
            {
                goto Failed;
            }
            gcoOS_ZeroMemory(gc->input.vertexCacheBlock, sizeof(__GLvertexCacheBlock));
        }

        if (!gc->input.pteInfo.hashTable)
        {
            if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, __GL_MAX_PTE_HASH_TABLE_SIZE * sizeof(void *), (gctPOINTER*)&gc->input.pteInfo.hashTable)))
            {
                goto Failed;
            }
            gcoOS_ZeroMemory(gc->input.pteInfo.hashTable, __GL_MAX_PTE_HASH_TABLE_SIZE * sizeof(void *));
        }

        if (!gc->input.tempPteInfo.hashTable)
        {
            if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, __GL_MAX_PTE_HASH_TABLE_SIZE * sizeof(void *), (gctPOINTER*)&gc->input.tempPteInfo.hashTable)))
            {
                goto Failed;
            }
            gcoOS_ZeroMemory(gc->input.tempPteInfo.hashTable, __GL_MAX_PTE_HASH_TABLE_SIZE * sizeof(void *));
        }

        gc->input.vertexCacheEnabled = GL_TRUE;
        gc->input.vertexCacheBlock->maxVertexCacheIdx = -1;
        gc->input.currentCacheBlock = gc->input.vertexCacheBlock;
        gc->input.currentVertexCache = &gc->input.vertexCacheBlock->cache[0];
        gc->input.vtxCacheNeedReset = GL_TRUE;
        gc->input.maxCacheDrawIndex = __GL_VERTEX_CACHE_BLOCK_SIZE;
    }

    gc->input.currentInfoBufPtr = gc->input.vertexInfoBuffer;
    gc->input.currentDataBufPtr = gc->input.vertexDataBuffer;
    gc->input.cacheBufferUsed = GL_FALSE;
    gc->input.cacheCompareFailed = GL_FALSE;

    gc->input.pCurrentImmedModeDispatch = &gc->immedModeDispatch;

    return GL_TRUE;

Failed:
    if (gc->input.defaultDataBuffer)
    {
        gcmOS_SAFE_FREE(gcvNULL, gc->input.defaultDataBuffer);
    }

    if (gc->input.defaultIndexBuffer)
    {
        gcmOS_SAFE_FREE(gcvNULL, gc->input.defaultIndexBuffer);
    }

    if (gc->input.edgeflag.pointer)
    {
        gcmOS_SAFE_FREE(gcvNULL, gc->input.edgeflag.pointer);
    }

    if (gc->input.defaultInfoBuffer)
    {
        gcmOS_SAFE_FREE(gcvNULL, gc->input.defaultInfoBuffer);
    }

    if (gc->input.vertexCacheBlock)
    {
        gcmOS_SAFE_FREE(gcvNULL, gc->input.vertexCacheBlock);
    }

    if (gc->input.pteInfo.hashTable)
    {
        gcmOS_SAFE_FREE(gcvNULL, gc->input.pteInfo.hashTable);
    }

    if (gc->input.tempPteInfo.hashTable)
    {
        gcmOS_SAFE_FREE(gcvNULL, gc->input.tempPteInfo.hashTable);
    }

    __glSetError(gc, GL_OUT_OF_MEMORY);

    return GL_FALSE;
}

void __glFreeImmedVertexCacheBlocks(__GLcontext *gc)
{
    __GLvertexCacheBlock *cacheBlock = gc->input.vertexCacheBlock;
    __GLvertexDataCache *vertexCache;
    GLint i;

    /* Free all the vertex cache buffers of each cache slot */
    while (cacheBlock)
    {
        for (i = 0; i <= cacheBlock->maxVertexCacheIdx; i++)
        {
            vertexCache = &cacheBlock->cache[i];

            if (vertexCache->privateData)
            {
                (*gc->dp.deletePrimData)(gc, vertexCache->privateData);
                vertexCache->privateData = NULL;
            }
            if (vertexCache->ibPrivateData)
            {
                (*gc->dp.deletePrimData)(gc, vertexCache->ibPrivateData);
                vertexCache->ibPrivateData = NULL;
            }
            if (vertexCache->vertexInfoBuffer)
            {
                gcmOS_SAFE_FREE(gcvNULL, vertexCache->vertexInfoBuffer);
                vertexCache->vertexInfoBuffer = NULL;
                vertexCache->infoBufSize = 0;
            }
            if (vertexCache->vertexDataBuffer)
            {
                gcmOS_SAFE_FREE(gcvNULL, vertexCache->vertexDataBuffer);
                vertexCache->vertexDataBuffer = NULL;
                vertexCache->dataBufSize = 0;
            }
            if (vertexCache->indexBuffer)
            {
                gcmOS_SAFE_FREE(gcvNULL, vertexCache->indexBuffer);
                vertexCache->indexBuffer = NULL;
                vertexCache->indexBufSize = 0;
            }

            vertexCache->vertexCount = 0;
            vertexCache->indexCount = 0;
            vertexCache->connectVertexCount = 0;
            vertexCache->cacheStatus = __GL_FILL_QUICK_VERTEX_CACHE;
        }

        cacheBlock = cacheBlock->next;
    }

    /* Free the vertex cache block linked list except the first cache block */
    cacheBlock = gc->input.vertexCacheBlock->next;
    while (cacheBlock)
    {
        gc->input.vertexCacheBlock->next = cacheBlock->next;
        gcmOS_SAFE_FREE(gcvNULL, cacheBlock);
        cacheBlock = gc->input.vertexCacheBlock->next;
    }

    gc->input.vertexCacheBlock->maxVertexCacheIdx = -1;

    gc->input.vertexCacheStatus = 0;
    gc->input.vertexCacheHistory = 0;
    gc->input.totalCacheMemSize = 0;
    gc->input.vtxCacheNeedReset = GL_TRUE;
    gc->input.maxCacheDrawIndex = __GL_VERTEX_CACHE_BLOCK_SIZE;
    gc->input.cacheHitFrameIndex = gc->input.currentFrameIndex;
}

/* Free IM vertex cache if it is in video memory and system memory */
GLvoid __glFreeImmedVertexCacheBuffer( __GLcontext *gc )
{
    /* Free vertex cache buffers that are already allocated */
    if (gc->input.defaultInfoBuffer)
    {
        gcmOS_SAFE_FREE(gcvNULL, gc->input.defaultInfoBuffer);
        gc->input.defaultInfoBuffer = NULL;
        gc->input.currentInfoBufPtr = NULL;
        gc->input.vertexInfoBuffer = NULL;
    }

    if (gc->input.vertexCacheBlock)
    {
        __glFreeImmedVertexCacheBlocks(gc);
        GL_ASSERT(gc->input.vertexCacheBlock->next == NULL);
        gcmOS_SAFE_FREE(gcvNULL, gc->input.vertexCacheBlock);
        gc->input.vertexCacheBlock = NULL;
        gc->input.currentVertexCache = NULL;
        gc->input.currentCacheBlock = NULL;
    }

    if (gc->input.pteInfo.hashTable)
    {
        __glClearPteInfoHashTable(gc, &gc->input.pteInfo, 1);
        gcmOS_SAFE_FREE(gcvNULL, gc->input.pteInfo.hashTable);
        gc->input.pteInfo.hashTable = NULL;
    }

    if (gc->input.tempPteInfo.hashTable)
    {
        __glClearPteInfoHashTable(gc, &gc->input.tempPteInfo, 0);
        gcmOS_SAFE_FREE(gcvNULL, gc->input.tempPteInfo.hashTable);
        gc->input.tempPteInfo.hashTable = NULL;
    }
}

GLvoid __glFreeVertexInputState(__GLcontext *gc)
{
    if ( gc->input.defaultDataBuffer )
    {
        gcmOS_SAFE_FREE(gcvNULL, gc->input.defaultDataBuffer);
        gc->input.defaultDataBuffer = NULL;
    }

    if ( gc->input.defaultIndexBuffer )
    {
        gcmOS_SAFE_FREE(gcvNULL, gc->input.defaultIndexBuffer);
        gc->input.defaultIndexBuffer = NULL;
    }


    if ( gc->input.edgeflag.pointer )
    {
        gcmOS_SAFE_FREE(gcvNULL, gc->input.edgeflag.pointer);
        gc->input.edgeflag.pointer = NULL;
    }

    __glFreeImmedVertexCacheBuffer( gc );
    __GL_SET_VARRAY_STOP_CACHE_BIT(gc);
}

/* Allocate twice the size of the maximum vertices to facilitate the clipping */
GLuint __glInitVertexOutputState(__GLcontext *gc)
{
    return GL_TRUE;
}

GLvoid __glFreeVertexOutputState(__GLcontext *gc)
{
}

/* Free vertex data cache if it is in video memory when mode changes */
GLboolean __glFreeImmedCacheInVideoMemory(__GLcontext *gc)
{
    __GLvertexCacheBlock *cacheBlock = gc->input.vertexCacheBlock;
    __GLvertexDataCache *vertexCache;
    GLint i;

    /* Free all IM cache memory */
    while (cacheBlock)
    {
        for (i = 0; i <= cacheBlock->maxVertexCacheIdx; i++)
        {
            vertexCache = &cacheBlock->cache[i];

            if (vertexCache->privateData)
            {
                (*gc->dp.deletePrimData)(gc, vertexCache->privateData);
            }

            if (vertexCache->ibPrivateData)
            {
                (*gc->dp.deletePrimData)(gc, vertexCache->ibPrivateData);
            }

            vertexCache->cacheStatus = __GL_FILL_QUICK_VERTEX_CACHE;
        }

        cacheBlock = cacheBlock->next;
    }


    return GL_TRUE;
}
#endif
