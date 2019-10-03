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


#ifdef OPENGL40
#include "gc_es_context.h"
#include "g_asmoff.h"
#include "api/gc_gl_api_inline.c"

extern GLuint fmtIndex2InputIndex[];
extern GLvoid __glSwitchToNorVertEntriesFunc(__GLcontext *gc);

/**/
extern GLvoid APIENTRY __glim_End_Material (__GLcontext *gc);

extern GLvoid  __glDrawImmedPrimitive(__GLcontext *gc);

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
    ~__GL_INPUT_EDGEFLAG,                     /* GL_POINTS             */
    ~__GL_INPUT_EDGEFLAG,                     /* GL_LINES              */
    ~__GL_INPUT_EDGEFLAG,                     /* GL_LINE_LOOP          */
    ~__GL_INPUT_EDGEFLAG,                     /* GL_LINE_STRIP         */
    (GLuint)~0,                                       /* GL_TRIANGLES          */
    ~__GL_INPUT_EDGEFLAG,                     /* GL_TRIANGLE_STRIP     */
    ~__GL_INPUT_EDGEFLAG,                     /* GL_TRIANGLE_FAN       */
    (GLuint)~0,                                       /* GL_QUADS              */
    ~__GL_INPUT_EDGEFLAG,                     /* GL_QUAD_STRIP         */
    (GLuint)~0,                                       /* GL_POLYGON            */
    ~__GL_INPUT_EDGEFLAG,                     /* GL_LINES_ADJACENCY_EXT */
    ~__GL_INPUT_EDGEFLAG,                     /* GL_LINE_STRIP_ADJACENCY_EXT */
    (GLuint)~0,                                       /* GL_TRIANGLES_ADJACENCY_EXT */
    ~__GL_INPUT_EDGEFLAG,                     /* GL_TRIANGLE_STRIP_ADJACENCY_EXT */
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
    else {
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
    while (inputMask) {
        if (inputMask & 0x1) {
            vE += 1;
        }
        inputMask >>= 1;
        i += 1;
    }
    gc->input.numberOfElements = vE;
}

GLvoid __glResetImmedVertexBuffer(__GLcontext *gc)
{
    /* Set the input buffer to the default vertex buffer */
    gc->input.vertexDataBuffer = gc->input.defaultDataBuffer;
    gc->input.indexBuffer = gc->input.defaultIndexBuffer;

    gc->input.primElemSequence = 0;

    gc->input.currentDataBufPtr = gc->input.vertexDataBuffer;

    gc->input.vertex.index = 0;
    gc->input.edgeflag.index = 0;
    gc->input.lastVertexIndex = 0;
    gc->input.indexCount = 0;
    gc->input.primBeginAddr = NULL;
    gc->input.preVertexFormat = 0;
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
        if(gc->shaderProgram.currentProgram)
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

        if (gc->state.enables.lighting.lighting) {
            vsInputMask |= __GL_INPUT_NORMAL;
            if (gc->state.enables.lighting.colorMaterial) {
                vsInputMask |= __GL_INPUT_DIFFUSE;
            }
        }
        else {
            vsInputMask |= __GL_INPUT_DIFFUSE;
            if (gc->state.enables.colorSum) {
                vsInputMask |= __GL_INPUT_SPECULAR;
            }
        }

        if (gc->state.enables.fog && gc->state.fog.coordSource == GL_FOG_COORDINATE) {
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

    gc->input.currentInputMask = totalInputMask;
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
        if(gc->shaderProgram.currentProgram)
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

        if (gc->state.enables.lighting.lighting) {
            vsInputMask |= __GL_INPUT_NORMAL;
            if (gc->state.enables.lighting.colorMaterial) {
                vsInputMask |= __GL_INPUT_DIFFUSE;
            }
        }
        else {
            vsInputMask |= __GL_INPUT_DIFFUSE;
            if (gc->state.enables.colorSum) {
                vsInputMask |= __GL_INPUT_SPECULAR;
            }
        }

        if (gc->state.enables.fog && gc->state.fog.coordSource == GL_FOG_COORDINATE) {
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

    gc->input.currentInputMask = totalInputMask;
}

/* Fill in the missing attribures for the current vertex (only one vertex) */

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
            else {
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
            if((GLubyte*)input->currentPtrDW < (GLubyte*)input->pointer)
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

    /* Use the current color to update material state if color material is enabled */
    if (gc->state.enables.lighting.colorMaterial &&
        (gc->input.primInputMask & __GL_INPUT_DIFFUSE))
    {
        __glUpdateMaterialfv(gc, gc->state.light.colorMaterialFace,
            gc->state.light.colorMaterialParam, (GLfloat *)&gc->state.current.color);
    }
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

    __glResetImmedVertexBuffer(gc);
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
    lastVertexBlock = (GLfloat*)(*gc->imports.malloc)(gc, origTotalStrideDW<<2 );
    if(!lastVertexBlock)
    {
        GL_ASSERT(0);
        return;
    }

    lastVertexIndex = gc->input.vertex.index;
    /* Copy the current vertex date to a scratch area */
    src = (GLfloat *)(gc->input.primBeginAddr + origTotalStrideDW * (gc->input.vertex.index - gc->input.lastVertexIndex + 1));
    __GL_MEMCOPY(lastVertexBlock, src, origTotalStrideDW<<2);

    __glImmedFlushPrim_Material(gc, GL_FALSE);
    __glResetImmedVertexBuffer(gc);
    gc->tnlAccum.preVertexIndex = gc->input.vertex.index;

    gc->input.primBeginAddr = gc->input.currentDataBufPtr;


    /* Change the primitive format to include the new attribute */
    gc->input.primInputMask |= (__GL_ONE_64 << attInpIdx);
    gc->input.preVertexFormat |= (__GL_ONE_64 << attFmtIdx);
    gc->input.primitiveFormat = gc->input.preVertexFormat;
    gc->input.currentInput[attInpIdx].sizeDW = fmtIndex2DWSize[attFmtIdx];

    /* Reset the new offsetDW and pointersfor  all the attributes already accumulated (including new attrib)*/
    inputMask = gc->input.primInputMask & ~(__GL_INPUT_VERTEX | __GL_INPUT_EDGEFLAG);
    j = 0;
    while (inputMask)
    {
        if (inputMask & 0x1) {
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
    (*gc->imports.free)(gc, lastVertexBlock);

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
    LastVert = (GLfloat*)(*gc->imports.malloc)(gc, origTotalStrideDW<<2 );
    src = (GLfloat *)(gc->input.primBeginAddr + origTotalStrideDW * (gc->input.vertex.index - gc->input.lastVertexIndex + 1));
    __GL_MEMCOPY(LastVert, src, origTotalStrideDW<<2);

    __glImmedFlushPrim_Material(gc, GL_FALSE);
    __glResetImmedVertexBuffer(gc);
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
    (*gc->imports.free)(gc, LastVert);

    gc->input.preVertexFormat = 0;
    gc->input.primitiveFormat = __glInputMask2InconsisFormat(gc->input.requiredInputMask);
    gc->input.inconsistentFormat = GL_TRUE;
}

/* Flush the immediate mode vertex buffer when it is full.
*/
GLvoid __glImmediateFlushBuffer(__GLcontext *gc)
{
    GLuint64 primElemSequence, inputMask;
    GLuint startIndex, endIndex;
    GLint vertexCount, connectCount = 0, i;

    if(gc->immedModeDispatch.End == __glim_End_Material)
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

    if (gc->input.indexPrimEnabled &&
        vertexCount >= minVertexNumber[gc->input.currentPrimMode])
    {
        __glGenerateVertexIndex(gc);
    }

    if (gc->input.vertex.index > 0) {
        __glDrawImmedPrimitive(gc);
    }
    __glImmedUpdateVertexState(gc);

    __glResetImmedVertexBuffer(gc);

    /* Copy the batch connection vertices to the beginnig of vertexDataBuffer */
    for (i = 0; i < connectCount; i++) {
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
    gc->input.primBeginAddr = gc->input.defaultDataBuffer;
    gc->input.currentDataBufPtr = gc->input.defaultDataBuffer + connectCount * gc->input.vertTotalStrideDW;

    /* Restore the previous primElemSequence and preVertexFormat to continue vertex accumulation */
    gc->input.primElemSequence = primElemSequence;
    if (gc->input.inconsistentFormat == GL_FALSE) {
        gc->input.preVertexFormat = gc->input.primitiveFormat;
    }

    /* Reset all the input pointers to the new locations */
    i = 0;
    inputMask = gc->input.primInputMask & (~__GL_INPUT_EDGEFLAG);
    while (inputMask) {
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

    if(gc->immedModeDispatch.End == __glim_End_Material)
    {
        GL_ASSERT(0);
    }

    __glComputePrimitiveData(gc);

    lastVertexIndex = gc->input.lastVertexIndex;

    if (gc->input.vertex.index > 0) {
        __glDrawImmedPrimitive(gc);
    }
    __glImmedUpdateVertexState(gc);
    __glResetImmedVertexBuffer(gc);

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

    if(gc->immedModeDispatch.End == __glim_End_Material)
    {
        __glSwitchToNewPrimtiveFormat_Material(gc, attFmtIdx);
        return;
    }

    gc->input.primitiveFormat = gc->input.preVertexFormat;

    __glComputePrimitiveData(gc);

    numVertex = gc->input.vertex.index - gc->input.lastVertexIndex;
    dataSize = (numVertex + 1) * (gc->input.vertTotalStrideDW << 2);
    lastVertexIndex = gc->input.lastVertexIndex;

    /* Copy the current primitive date to a scratch area */
    curPrimPtr = (GLfloat*)(*gc->imports.malloc)(gc, dataSize );
    __GL_MEMCOPY(curPrimPtr, gc->input.primBeginAddr, dataSize);

    /* Roll back the vertex index to the previous consistent primitive */
    gc->input.vertex.index = gc->input.lastVertexIndex;

    if (gc->input.vertex.index > 0) {
        __glDrawImmedPrimitive(gc);
    }
    __glImmedUpdateVertexState(gc);
    __glResetImmedVertexBuffer(gc);
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
    while (inputMask) {
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
    (*gc->imports.free)(gc, curPrimPtr);
}

GLvoid __glSwitchToInconsistentFormat(__GLcontext *gc)
{
    GLuint numVertex, dataSize, newTotalStrideDW, origTotalStrideDW, i, j, k;
    GLuint formatMask, lastVertexIndex, inputTag;
    GLuint64 inputMask;
    GLfloat *curPrimPtr, *src, *dst;
    GLuint inputOffsetDW[__GL_TOTAL_VERTEX_ATTRIBUTES] = {0};
    GLuint inputSize[__GL_TOTAL_VERTEX_ATTRIBUTES] = {0};

    if(gc->immedModeDispatch.End == __glim_End_Material)
    {
         __glSwitchToInconsistentFormat_Material(gc);
         return;
    }

    gc->input.primitiveFormat = gc->input.preVertexFormat;

    __glComputePrimitiveData(gc);

    numVertex = gc->input.vertex.index - gc->input.lastVertexIndex;
    dataSize = (numVertex + 1) * (gc->input.vertTotalStrideDW << 2);
    lastVertexIndex = gc->input.lastVertexIndex;

    /* Copy the current primitive date to a scratch area */
    curPrimPtr = (GLfloat*)(*gc->imports.malloc)(gc, dataSize );
    __GL_MEMCOPY(curPrimPtr, gc->input.primBeginAddr, dataSize);

    /* Roll back the vertex index to the previous consistent primitive */
    gc->input.vertex.index = gc->input.lastVertexIndex;

    if (gc->input.vertex.index > 0) {
        __glDrawImmedPrimitive(gc);
    }
    __glImmedUpdateVertexState(gc);
    __glResetImmedVertexBuffer(gc);

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
            else {
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
    (*gc->imports.free)(gc, curPrimPtr);

    gc->input.preVertexFormat = 0;
    gc->input.primitiveFormat = __glInputMask2InconsisFormat(gc->input.requiredInputMask);
    gc->input.inconsistentFormat = GL_TRUE;
}

GLvoid APIENTRY __glim_Begin(__GLcontext *gc, GLenum mode)
{
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
        break;

    case __GL_IN_BEGIN:
        __glSetError(gc,GL_INVALID_OPERATION);
        return;

    /* add next step
    case __GL_SMALL_LIST_BATCH:
        __glDisplayListBatchEnd(gc);
        break;
    */
    default:

        GL_ASSERT(0);
    }

    gc->input.primMode = gc->input.currentPrimMode = mode;
    gc->input.beginMode = __GL_IN_BEGIN;
    gc->input.vertexFormat = 0;
    gc->input.preVertexFormat = 0;
    gc->input.primBeginAddr = gc->input.currentDataBufPtr;

    __glValidateImmedBegin(gc, mode);
}

GLvoid APIENTRY __glim_End(__GLcontext *gc )
{
    if (gc->conditionalRenderDiscard)
    {
        return;
    }

    if (gc->input.inconsistentFormat == GL_FALSE)
    {
        gc->input.primitiveFormat = gc->input.preVertexFormat;
        gc->input.preVertexFormat = 0;
    }

    if (gc->input.indexPrimEnabled) {
        __glGenerateVertexIndex(gc);
    }

    __glComputePrimitiveData(gc);
    if (gc->input.vertex.index > 0)
    {
        __glDrawImmedPrimitive(gc);
    }
    __glImmedUpdateVertexState(gc);

    __glResetImmedVertexBuffer(gc);

    gc->input.primBeginAddr = NULL;
    gc->input.inconsistentFormat = GL_FALSE;
    gc->input.currentDataBufPtr = gc->input.vertexDataBuffer + gc->input.vertex.index * gc->input.vertTotalStrideDW;
    gc->input.lastVertexIndex = gc->input.vertex.index;
    if (gc->input.vertex.index == 0) {
        gc->input.primElemSequence = 0;
    }

    gc->input.beginMode = __GL_NOT_IN_BEGIN;
}

GLvoid APIENTRY __glim_End_Error(__GLcontext *gc)
{
    if (gc->input.beginMode != __GL_IN_BEGIN)
    {
        __glSetError(gc,GL_INVALID_OPERATION);
        return;
    }
}


GLuint __glInitVertexInputState(__GLcontext *gc)
{
    if (!gc->input.defaultDataBuffer)
    {
        gc->input.defaultDataBuffer = (GLfloat *)(*gc->imports.malloc)
            (0, __GL_DEFAULT_VERTEX_BUFFER_SIZE );
        gc->input.defaultDataBufEnd = gc->input.defaultDataBuffer +
            ((__GL_DEFAULT_VERTEX_BUFFER_SIZE - __GL_DEFAULT_BUFFER_END_ZONE) >> 2);
        gc->input.vertexDataBuffer = gc->input.defaultDataBuffer;
    }

    if (!gc->input.defaultIndexBuffer)
    {
        gc->input.defaultIndexBuffer = (GLushort *)(*gc->imports.malloc)
            (0, 3 * __GL_MAX_VERTEX_NUMBER * sizeof(GLushort) );
        gc->input.indexBuffer = gc->input.defaultIndexBuffer;
    }

    if (!gc->input.edgeflag.pointer)
    {
        gc->input.edgeflag.pointer = (GLubyte *)(*gc->imports.malloc)
            (0, __GL_MAX_VERTEX_NUMBER * sizeof(GLubyte) );
    }

    if (!gc->input.defaultDataBuffer ||
        !gc->input.defaultIndexBuffer ||
        !gc->input.edgeflag.pointer) {
        __glSetError(gc,GL_OUT_OF_MEMORY);
        return GL_FALSE;
    }

    gc->input.currentDataBufPtr = gc->input.vertexDataBuffer;

    return GL_TRUE;

}

GLvoid __glFreeVertexInputState(__GLcontext *gc)
{
    if ( gc->input.defaultDataBuffer )
    {
        (*gc->imports.free)(gc, gc->input.defaultDataBuffer);
        gc->input.defaultDataBuffer = NULL;
    }

    if ( gc->input.defaultIndexBuffer )
    {
        (*gc->imports.free)(gc, gc->input.defaultIndexBuffer);
        gc->input.defaultIndexBuffer = NULL;
    }


    if ( gc->input.edgeflag.pointer )
    {
        (*gc->imports.free)(gc, gc->input.edgeflag.pointer);
        gc->input.edgeflag.pointer = NULL;
    }
}

/* Allocate twice the size of the maximum vertices to facilitate the clipping */
GLuint __glInitVertexOutputState(__GLcontext *gc)
{
/*    __GL_MEMSET(&gc->vsOutputContainer, 0, sizeof(__GLVSOutput));

    if ( !gc->vsOutputContainer.clipCodeBuffer )
    {
        gc->vsOutputContainer.clipCodeBuffer = (GLubyte *)(*gc->imports.malloc)
            (gc, 2 * __GL_MAX_VERTEX_NUMBER * sizeof(GLuint) );
    }

    if(!gc->vsOutputContainer.clipCodeBuffer)
    {
        __glSetError(GL_OUT_OF_MEMORY);
        return GL_FALSE;
    }
    else
    {
        gc->vsOutputContainer.incClipSize =
        gc->vsOutputContainer.outClipSize = 2 * __GL_MAX_VERTEX_NUMBER * sizeof(GLuint);
    }

    if ( !gc->vsOutputContainer.vertexOutputBuffer )
    {
        gc->vsOutputContainer.vertexOutputBuffer = (GLubyte *)(*gc->imports.malloc)
            (gc, 2 * __GL_MAX_VERTEX_NUMBER * sizeof(__GLvertex4) );
    }

    if(!gc->vsOutputContainer.vertexOutputBuffer)
    {
        __glSetError(GL_OUT_OF_MEMORY);
        return GL_FALSE;
    }
    else
    {
        gc->vsOutputContainer.incVBSize =
        gc->vsOutputContainer.outVBSize = 2 * __GL_MAX_VERTEX_NUMBER * sizeof(__GLvertex4);
    }
*/
    return GL_TRUE;
}

GLvoid __glFreeVertexOutputState(__GLcontext *gc)
{
    /*
    if(gc->vsOutputContainer.vertexOutputBuffer)
        (*gc->imports.free)(gc, gc->vsOutputContainer.vertexOutputBuffer);
    gc->vsOutputContainer.outVBSize = 0;
    gc->vsOutputContainer.vertexOutputBuffer = NULL;

    if(gc->vsOutputContainer.clipCodeBuffer)
        (*gc->imports.free)(gc, gc->vsOutputContainer.clipCodeBuffer);
    gc->vsOutputContainer.clipCodeBuffer = NULL;
    gc->vsOutputContainer.outClipSize = 0;

    if(gc->vsOutputContainer.clipSpacePosBuffer)
        (*gc->imports.free)(gc, gc->vsOutputContainer.clipSpacePosBuffer);
    gc->vsOutputContainer.clipSpacePosBuffer = NULL;
    gc->vsOutputContainer.outClipSpacePosSize = 0;
*/
}

/* Free vertex data cache if it is in video memory when mode changes */
GLboolean __glFreeImmedCacheInVideoMemory(__GLcontext *gc)
{
    return GL_TRUE;
}
#endif
