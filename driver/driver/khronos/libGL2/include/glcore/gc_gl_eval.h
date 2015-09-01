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


#ifndef __gc_gl_evaluator_h_
#define __gc_gl_evaluator_h_

#define __GL_MAX_ORDER          40

/* Number of maps */
#define __GL_MAP_RANGE_COUNT    9

#define __GL_EVAL1D_INDEX(old)      ((old) - GL_MAP1_COLOR_4)
#define __GL_EVAL2D_INDEX(old)      ((old) - GL_MAP2_COLOR_4)

/* internal form of map range indexes */
#define __GL_C4     __GL_EVAL1D_INDEX(GL_MAP1_COLOR_4)
#define __GL_I      __GL_EVAL1D_INDEX(GL_MAP1_INDEX)
#define __GL_N3     __GL_EVAL1D_INDEX(GL_MAP1_NORMAL)
#define __GL_T1     __GL_EVAL1D_INDEX(GL_MAP1_TEXTURE_COORD_1)
#define __GL_T2     __GL_EVAL1D_INDEX(GL_MAP1_TEXTURE_COORD_2)
#define __GL_T3     __GL_EVAL1D_INDEX(GL_MAP1_TEXTURE_COORD_3)
#define __GL_T4     __GL_EVAL1D_INDEX(GL_MAP1_TEXTURE_COORD_4)
#define __GL_V3     __GL_EVAL1D_INDEX(GL_MAP1_VERTEX_3)
#define __GL_V4     __GL_EVAL1D_INDEX(GL_MAP1_VERTEX_4)

typedef struct StateChangeRec
{
    GLint changed;
    __GLcolor color;
    __GLcoord normal;
    __GLcoord texture;
    __GLcoord vertex;
} StateChange;

typedef struct {
    /*
    ** not strictly necessary since it can be inferred from the index,
    ** but it makes the code simpler.
    */
    GLint k;

    /*
    ** Order of the polynomial + 1
    */
    GLint order;

    GLfloat u1, u2;
} __GLevaluator1;

typedef struct {
    GLint k;
    GLint majorOrder, minorOrder;
    GLfloat u1, u2;
    GLfloat v1, v2;
} __GLevaluator2;

typedef struct {
    GLfloat start;
    GLfloat finish;
    GLfloat step;
    GLint n;
} __GLevaluatorGrid;

typedef struct {
    __GLevaluatorGrid u1, u2, v2;
} __GLevaluatorState;

typedef struct {
    __GLevaluator1 eval1[__GL_MAP_RANGE_COUNT];
    __GLevaluator2 eval2[__GL_MAP_RANGE_COUNT];

    GLfloat *eval1Data[__GL_MAP_RANGE_COUNT];
    GLfloat *eval2Data[__GL_MAP_RANGE_COUNT];

    GLfloat uvalue;
    GLfloat vvalue;
    GLfloat ucoeff[__GL_MAX_ORDER];
    GLfloat vcoeff[__GL_MAX_ORDER];
    GLfloat ucoeffDeriv[__GL_MAX_ORDER];
    GLfloat vcoeffDeriv[__GL_MAX_ORDER];
    GLint uorder;
    GLint vorder;
    GLint utype;
    GLint vtype;
} __GLevaluatorMachine;

#endif /* __gc_gl_evaluator_h_ */
