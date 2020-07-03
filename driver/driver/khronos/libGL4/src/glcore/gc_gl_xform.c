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


#include "gc_es_context.h"


#if defined(_WIN32)
#pragma warning(disable: 4244)
#endif

/*
** An amazing thing has happened.  More than 2^32 changes to the projection
** matrix has occurred.  Run through the modelView and projection stacks
** and reset the sequence numbers to force a re-validate on next usage.
*/
GLvoid __glInvalidateSequenceNumbers(__GLcontext *gc)
{
    __GLtransform *tr, *lasttr;
    GLuint s;

    /* Make all mvp matrices refer to sequence number zero */
    s = 0;
    tr = &gc->transform.modelViewStack[0];
    lasttr = tr + gc->constants.maxModelViewStackDepth;
    while (tr < lasttr) {
        tr->sequence = s;
        tr++;
    }

    /* Make all projection matricies sequence up starting at one */
    s = 1;
    tr = &gc->transform.projectionStack[0];
    lasttr = tr + gc->constants.maxProjectionStackDepth;
    while (tr < lasttr) {
        tr->sequence = s++;
        tr++;
    }
    gc->transform.projectionSequence = s;
}

/************************************************************************/

GLvoid __glPushModelViewMatrix(__GLcontext *gc)
{
    __GLtransform **trp, *tr, *stack;
    GLint num;

    num = gc->constants.maxModelViewStackDepth;
    trp = &gc->transform.modelView;
    stack = gc->transform.modelViewStack;
    tr = *trp;
    if (tr < &stack[num-1]) {
        tr[1] = tr[0];
        *trp = tr + 1;
    } else {
        __glSetError(gc, GL_STACK_OVERFLOW);
    }
}

GLvoid __glPopModelViewMatrix(__GLcontext *gc)
{
    __GLtransform **trp, *tr, *stack, *mvtr, *ptr;

    trp = &gc->transform.modelView;
    stack = gc->transform.modelViewStack;
    tr = *trp;
    if (tr > &stack[0]) {
        *trp = tr - 1;

        /*
        ** See if sequence number of modelView matrix is the same as the
        ** sequence number of the projection matrix.  If not, then
        ** recompute the mvp matrix.
        */
        mvtr = gc->transform.modelView;
        ptr = gc->transform.projection;
        if (mvtr->sequence != ptr->sequence) {
            mvtr->sequence = ptr->sequence;
            (*gc->transform.matrix.mult)(&mvtr->mvp, &mvtr->matrix, &ptr->matrix);
        }

        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_MODELVIEW_TRANSFORM_BIT);
    }
    else {
        __glSetError(gc, GL_STACK_UNDERFLOW);
        return;
    }
}

GLvoid __glLoadIdentityModelViewMatrix(__GLcontext *gc)
{
    __GLtransform *mvtr, *ptr;

    mvtr = gc->transform.modelView;
    (*gc->transform.matrix.makeIdentity)(&mvtr->matrix);
    (*gc->transform.matrix.makeIdentity)(&mvtr->inverseTranspose);
    mvtr->updateInverse = GL_FALSE;
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_MODELVIEW_TRANSFORM_BIT);

    /* Update mvp matrix */
    ptr = gc->transform.projection;
    mvtr->sequence = ptr->sequence;
    (*gc->transform.matrix.mult)(&mvtr->mvp, &mvtr->matrix, &ptr->matrix);
}

GLvoid __glPushProjectionMatrix(__GLcontext *gc)
{
    __GLtransform **trp, *tr, *stack;
    GLint num;

    num = gc->constants.maxProjectionStackDepth;
    trp = &gc->transform.projection;
    stack = gc->transform.projectionStack;
    tr = *trp;
    if (tr < &stack[num-1]) {
        tr[1].matrix = tr[0].matrix;
        tr[1].sequence = tr[0].sequence;
        *trp = tr + 1;
    } else {
        __glSetError(gc, GL_STACK_OVERFLOW);
    }
}

GLvoid __glPopProjectionMatrix(__GLcontext *gc)
{
    __GLtransform **trp, *tr, *stack, *mvtr, *ptr;

    trp = &gc->transform.projection;
    stack = gc->transform.projectionStack;
    tr = *trp;
    if (tr > &stack[0]) {
        *trp = tr - 1;

        /*
        ** See if sequence number of modelView matrix is the same as the
        ** sequence number of the projection matrix.  If not, then
        ** recompute the mvp matrix.
        */
        mvtr = gc->transform.modelView;
        ptr = gc->transform.projection;
        if (mvtr->sequence != ptr->sequence) {
            mvtr->sequence = ptr->sequence;
            (*gc->transform.matrix.mult)(&mvtr->mvp, &mvtr->matrix, &ptr->matrix);
        }

        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_PROJECTION_TRANSFORM_BIT);
    }
    else {
        __glSetError(gc, GL_STACK_UNDERFLOW);
        return;
    }
}

GLvoid __glLoadIdentityProjectionMatrix(__GLcontext *gc)
{
    __GLtransform *mvtr, *ptr;

    ptr = gc->transform.projection;
    (*gc->transform.matrix.makeIdentity)(&ptr->matrix);
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_PROJECTION_TRANSFORM_BIT);

    if (++gc->transform.projectionSequence == 0) {
        __glInvalidateSequenceNumbers(gc);
    } else {
        ptr->sequence = gc->transform.projectionSequence;
    }

    /* Update mvp matrix */
    mvtr = gc->transform.modelView;
    mvtr->sequence = ptr->sequence;
    (*gc->transform.matrix.mult)(&mvtr->mvp, &mvtr->matrix, &ptr->matrix);
}

GLvoid __glPushTextureMatrix(__GLcontext *gc)
{
    __GLtransform **trp, *tr, *stack;
    GLuint unit = gc->state.texture.activeTexIndex;
    GLint num;

    num = gc->constants.maxTextureStackDepth;
    trp = &gc->transform.texture[unit];
    stack = gc->transform.textureStack[unit];
    tr = *trp;
    if (tr < &stack[num-1]) {
        tr[1].matrix = tr[0].matrix;
        *trp = tr + 1;
    } else {
        __glSetError(gc, GL_STACK_OVERFLOW);
    }
}

GLvoid __glPopTextureMatrix(__GLcontext *gc)
{
    __GLtransform **trp, *tr, *stack;
    GLuint unit = gc->state.texture.activeTexIndex;

    trp = &gc->transform.texture[unit];
    stack = gc->transform.textureStack[unit];
    tr = *trp;
    if (tr > &stack[0]) {
        *trp = tr - 1;
        __GL_SET_TEX_UNIT_BIT(gc, unit, __GL_TEXTURE_TRANSFORM_BIT);
    }
    else {
        __glSetError(gc, GL_STACK_UNDERFLOW);
        return;
    }
}

GLvoid __glLoadIdentityTextureMatrix(__GLcontext *gc)
{
    GLuint unit = gc->state.texture.activeTexIndex;
    __GLtransform *tr = gc->transform.texture[unit];

    (*gc->transform.matrix.makeIdentity)(&tr->matrix);

    __GL_SET_TEX_UNIT_BIT(gc, unit, __GL_TEXTURE_TRANSFORM_BIT);
}
/************************************************************************/
#if GL_ARB_vertex_program
GLvoid __glPushProgramMatrix(__GLcontext *gc)
{
    __GLtransform **trp, *tr, *stack;
    GLuint index = gc->state.transform.matrixMode - GL_MATRIX0_ARB;
    GLint num;

    num = gc->constants.maxProgramStackDepth;
    trp = &gc->transform.program[index];
    stack = gc->transform.programStack[index];
    tr = *trp;
    if (tr < &stack[num-1]) {
        tr[1].matrix = tr[0].matrix;
        *trp = tr + 1;
    }
    else {
        __glSetError(gc, GL_STACK_OVERFLOW);
    }
}

GLvoid __glPopProgramMatrix(__GLcontext *gc)
{
    __GLtransform **trp, *tr, *stack;
    GLuint index = gc->state.transform.matrixMode - GL_MATRIX0_ARB;

    trp = &gc->transform.program[index];
    stack = gc->transform.programStack[index];
    tr = *trp;
    if (tr > &stack[0]) {
        *trp = tr - 1;
        __GL_PROGRAM_SET_PROGRAMMATRIX_DIRTY(gc,(1<<index));
    }
    else {
        __glSetError(gc, GL_STACK_UNDERFLOW);
        return;
    }
}

GLvoid __glLoadIdentityProgramMatrix(__GLcontext *gc)
{
    GLuint index = gc->state.transform.matrixMode - GL_MATRIX0_ARB;
    __GLtransform *tr = gc->transform.program[index];

    (*gc->transform.matrix.makeIdentity)(&tr->matrix);

    __GL_PROGRAM_SET_PROGRAMMATRIX_DIRTY(gc,(1<<index));
}
#endif

/************************************************************************/

/*
** Assuming that a->matrixType and b->matrixType are already correct,
** and dst = a * b, then compute dst's matrix type.
*/
GLvoid __glPickMatrixType(__GLmatrix *dst, __GLmatrix *a, __GLmatrix *b)
{
    switch(a->matrixType) {
      case __GL_MT_GENERAL:
          dst->matrixType = a->matrixType;
          break;
      case __GL_MT_W0001:
          if (b->matrixType == __GL_MT_GENERAL) {
              dst->matrixType = b->matrixType;
          } else {
              dst->matrixType = a->matrixType;
          }
          break;
      case __GL_MT_IS2D:
          if (b->matrixType < __GL_MT_IS2D) {
              dst->matrixType = b->matrixType;
          } else {
              dst->matrixType = a->matrixType;
          }
          break;
      case __GL_MT_IS2DNR:
          if (b->matrixType < __GL_MT_IS2DNR) {
              dst->matrixType = b->matrixType;
          } else {
              dst->matrixType = a->matrixType;
          }
          break;
      case __GL_MT_IDENTITY:
          dst->matrixType = b->matrixType;
          break;
      case __GL_MT_IS2DNRSC:
          if (b->matrixType == __GL_MT_IDENTITY) {
              dst->matrixType = __GL_MT_IS2DNRSC;
          } else if (b->matrixType < __GL_MT_IS2DNR) {
              dst->matrixType = b->matrixType;
          } else {
              dst->matrixType = __GL_MT_IS2DNR;
          }
          break;
    }
}

/*
** Muliply the first matrix by the second one keeping track of the matrix
** type of the newly combined matrix.
*/
GLvoid __glMultiplyMatrix(__GLcontext *gc, __GLmatrix *m, GLvoid *data)
{
    __GLmatrix *tm;

    tm = (__GLmatrix *)data;
    (*gc->transform.matrix.mult)(m, tm, m);
    __glPickMatrixType(m, tm, m);
}

GLvoid __glDoLoadMatrix(__GLcontext *gc, const __GLmatrix *m)
{
    __GLtransform *tr, *otr;
    GLuint unit;

    switch (gc->state.transform.matrixMode) {
      case GL_MODELVIEW:
          tr = gc->transform.modelView;
          if (__GL_MEMCMP(&tr->matrix.matrix[0][0],&m->matrix[0][0],16*sizeof(GLfloat)))
          {
              (*gc->transform.matrix.copy)(&tr->matrix, m);
              tr->updateInverse = GL_TRUE;
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_MODELVIEW_TRANSFORM_BIT);

              /* Update mvp matrix */
              otr = gc->transform.projection;
              tr->sequence = otr->sequence;
              (*gc->transform.matrix.mult)(&tr->mvp, &tr->matrix, &otr->matrix);
          }
          break;

      case GL_PROJECTION:
          tr = gc->transform.projection;
          if (__GL_MEMCMP(&tr->matrix.matrix[0][0],&m->matrix[0][0],16*sizeof(GLfloat)))
          {
              (*gc->transform.matrix.copy)(&tr->matrix, m);
              tr->updateInverse = GL_TRUE;
              if (++gc->transform.projectionSequence == 0) {
                  __glInvalidateSequenceNumbers(gc);
              } else {
                  tr->sequence = gc->transform.projectionSequence;
              }
              __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_PROJECTION_TRANSFORM_BIT);

              /* Update mvp matrix */
              otr = gc->transform.modelView;
              otr->sequence = tr->sequence;
              (*gc->transform.matrix.mult)(&otr->mvp, &otr->matrix, &tr->matrix);
          }
          break;

      case GL_TEXTURE:
          unit = gc->state.texture.activeTexIndex;
          tr = gc->transform.texture[unit];
          (*gc->transform.matrix.copy)(&tr->matrix, m);
          __GL_SET_TEX_UNIT_BIT(gc, unit, __GL_TEXTURE_TRANSFORM_BIT);
          break;
    }
}

__GL_INLINE GLvoid __glDoMultMatrix(__GLcontext *gc, GLvoid *data,
                                  GLvoid (*multiply)(__GLcontext *gc, __GLmatrix *m, GLvoid *data))
{
    __GLtransform *tr, *otr;
    GLuint unit;

    switch (gc->state.transform.matrixMode) {
      case GL_MODELVIEW:
          tr = gc->transform.modelView;
          (*multiply)(gc, &tr->matrix, data);
          tr->updateInverse = GL_TRUE;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_MODELVIEW_TRANSFORM_BIT);

          /* Update mvp matrix */
          (*multiply)(gc, &tr->mvp, data);
          break;

      case GL_PROJECTION:
          tr = gc->transform.projection;
          (*multiply)(gc, &tr->matrix, data);
          tr->updateInverse = GL_TRUE;
          if (++gc->transform.projectionSequence == 0) {
              __glInvalidateSequenceNumbers(gc);
          } else {
              tr->sequence = gc->transform.projectionSequence;
          }
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_PROJECTION_TRANSFORM_BIT);

          /* Update mvp matrix */
          otr = gc->transform.modelView;
          otr->sequence = tr->sequence;
          (*gc->transform.matrix.mult)(&otr->mvp, &otr->matrix, &tr->matrix);
          break;

      case GL_TEXTURE:
          unit = gc->state.texture.activeTexIndex;
          tr = gc->transform.texture[unit];
          (*multiply)(gc, &tr->matrix, data);
          __GL_SET_TEX_UNIT_BIT(gc, unit, __GL_TEXTURE_TRANSFORM_BIT);
          break;
    }
}

/************************************************************************/

__GL_INLINE GLvoid __glDoRotate(__GLcontext *gc, __GLfloat angle, __GLfloat ax,
                              __GLfloat ay, __GLfloat az)
{
    __GLmatrix m;
    __GLfloat radians, sine, cosine, ab, bc, ca, t;
    __GLfloat av[4], axis[4];

    av[0] = ax;
    av[1] = ay;
    av[2] = az;
    av[3] = 0;
    __glNormalize(axis, av);

    radians = angle * __glDegreesToRadians;
    sine = __GL_SINF(radians);
    cosine = __GL_COSF(radians);
    ab = axis[0] * axis[1] * (1 - cosine);
    bc = axis[1] * axis[2] * (1 - cosine);
    ca = axis[2] * axis[0] * (1 - cosine);

    (*gc->transform.matrix.makeIdentity)(&m);
    t = axis[0] * axis[0];
    m.matrix[0][0] = t + cosine * (1 - t);
    m.matrix[2][1] = bc - axis[0] * sine;
    m.matrix[1][2] = bc + axis[0] * sine;

    t = axis[1] * axis[1];
    m.matrix[1][1] = t + cosine * (1 - t);
    m.matrix[2][0] = ca + axis[1] * sine;
    m.matrix[0][2] = ca - axis[1] * sine;

    t = axis[2] * axis[2];
    m.matrix[2][2] = t + cosine * (1 - t);
    m.matrix[1][0] = ab - axis[2] * sine;
    m.matrix[0][1] = ab + axis[2] * sine;
    if (ax == __glZero && ay == __glZero) {
        m.matrixType = __GL_MT_IS2D;
    } else {
        m.matrixType = __GL_MT_W0001;
    }
    __glDoMultMatrix(gc, &m, __glMultiplyMatrix);
}

struct __glScaleRec {
    __GLfloat x,y,z;
};

/* ARGSUSED */
GLvoid __glScaleMatrix(__GLcontext *gc, __GLmatrix *m, GLvoid *data)
{
    struct __glScaleRec *scale;
    __GLfloat x,y,z;
    __GLfloat M0, M1, M2, M3;

    if (m->matrixType > __GL_MT_IS2DNR) {
        m->matrixType = __GL_MT_IS2DNR;
    }
    scale = (struct __glScaleRec *)data;
    x = scale->x;
    y = scale->y;
    z = scale->z;

    M0 = x * m->matrix[0][0];
    M1 = x * m->matrix[0][1];
    M2 = x * m->matrix[0][2];
    M3 = x * m->matrix[0][3];
    m->matrix[0][0] = M0;
    m->matrix[0][1] = M1;
    m->matrix[0][2] = M2;
    m->matrix[0][3] = M3;

    M0 = y * m->matrix[1][0];
    M1 = y * m->matrix[1][1];
    M2 = y * m->matrix[1][2];
    M3 = y * m->matrix[1][3];
    m->matrix[1][0] = M0;
    m->matrix[1][1] = M1;
    m->matrix[1][2] = M2;
    m->matrix[1][3] = M3;

    M0 = z * m->matrix[2][0];
    M1 = z * m->matrix[2][1];
    M2 = z * m->matrix[2][2];
    M3 = z * m->matrix[2][3];
    m->matrix[2][0] = M0;
    m->matrix[2][1] = M1;
    m->matrix[2][2] = M2;
    m->matrix[2][3] = M3;
}

__GL_INLINE GLvoid __glDoScale(__GLcontext *gc, __GLfloat x, __GLfloat y, __GLfloat z)
{
    struct __glScaleRec scale;

    scale.x = x;
    scale.y = y;
    scale.z = z;
    __glDoMultMatrix(gc, &scale, __glScaleMatrix);
}

struct __glTranslationRec {
    __GLfloat x,y,z;
};

/*
** Matrix type of m stays the same.
*/
GLvoid __glTranslateMatrix(__GLcontext *gc, __GLmatrix *m, GLvoid *data)
{
    struct __glTranslationRec *trans;
    __GLfloat x,y,z;
    __GLfloat _M30, _M31, _M32, _M33;

    if (m->matrixType > __GL_MT_IS2DNR) {
        m->matrixType = __GL_MT_IS2DNR;
    }
    trans = (struct __glTranslationRec *)data;
    x = trans->x;
    y = trans->y;
    z = trans->z;
    _M30 = x * m->matrix[0][0] + y * m->matrix[1][0] + z * m->matrix[2][0] +
        m->matrix[3][0];
    _M31 = x * m->matrix[0][1] + y * m->matrix[1][1] + z * m->matrix[2][1] +
        m->matrix[3][1];
    _M32 = x * m->matrix[0][2] + y * m->matrix[1][2] + z * m->matrix[2][2] +
        m->matrix[3][2];
    _M33 = x * m->matrix[0][3] + y * m->matrix[1][3] + z * m->matrix[2][3] +
        m->matrix[3][3];
    m->matrix[3][0] = _M30;
    m->matrix[3][1] = _M31;
    m->matrix[3][2] = _M32;
    m->matrix[3][3] = _M33;
}

__GL_INLINE GLvoid __glDoTranslate(__GLcontext *gc, __GLfloat x, __GLfloat y, __GLfloat z)
{
    struct __glTranslationRec trans;

    trans.x = x;
    trans.y = y;
    trans.z = z;
    __glDoMultMatrix(gc, &trans, __glTranslateMatrix);
}

GLvoid __glGetCurrentMatrix(__GLcontext * gc, GLfloat *m)
{
    GLuint ctri, index;

    switch (gc->state.transform.matrixMode) {
    case GL_MODELVIEW:
        for(ctri = 0; ctri < 4; ctri++) {
            m[4 * ctri] = gc->transform.modelView->matrix.matrix[ctri][0];
            m[4 * ctri + 1] = gc->transform.modelView->matrix.matrix[ctri][1];
            m[4 * ctri + 2] = gc->transform.modelView->matrix.matrix[ctri][2];
            m[4 * ctri + 3] = gc->transform.modelView->matrix.matrix[ctri][3];
        }
        break;
    case GL_PROJECTION:
        for(ctri = 0; ctri < 4; ctri++) {
            m[4 * ctri] = gc->transform.projection->matrix.matrix[ctri][0];
            m[4 * ctri + 1] = gc->transform.projection->matrix.matrix[ctri][1];
            m[4 * ctri + 2] = gc->transform.projection->matrix.matrix[ctri][2];
            m[4 * ctri + 3] = gc->transform.projection->matrix.matrix[ctri][3];
        }
        break;
    case GL_TEXTURE:
        index = gc->state.texture.activeTexIndex;
        for(ctri = 0; ctri < 4; ctri++) {
            m[4 * ctri] = gc->transform.texture[index]->matrix.matrix[ctri][0];
            m[4 * ctri + 1] = gc->transform.texture[index]->matrix.matrix[ctri][1];
            m[4 * ctri + 2] = gc->transform.texture[index]->matrix.matrix[ctri][2];
            m[4 * ctri + 3] = gc->transform.texture[index]->matrix.matrix[ctri][3];
        }
        break;
    }
}

GLvoid __glGetCurrentTransposeMatrix(__GLcontext * gc, GLfloat *m)
{
    GLuint ctri, index;

    switch(gc->state.transform.matrixMode) {
    case GL_MODELVIEW:
        for(ctri = 0; ctri < 4; ctri++) {
            m[4 * ctri] = gc->transform.modelView->matrix.matrix[0][ctri];
            m[4 * ctri + 1] = gc->transform.modelView->matrix.matrix[1][ctri];
            m[4 * ctri + 2] = gc->transform.modelView->matrix.matrix[2][ctri];
            m[4 * ctri + 3] = gc->transform.modelView->matrix.matrix[3][ctri];
        }
        break;
    case GL_PROJECTION:
        for(ctri = 0; ctri < 4; ctri++) {
            m[4 * ctri] = gc->transform.projection->matrix.matrix[0][ctri];
            m[4 * ctri + 1] = gc->transform.projection->matrix.matrix[1][ctri];
            m[4 * ctri + 2] = gc->transform.projection->matrix.matrix[2][ctri];
            m[4 * ctri + 3] = gc->transform.projection->matrix.matrix[3][ctri];
        }
        break;
    case GL_TEXTURE:
        index = gc->state.texture.activeTexIndex;
        for(ctri = 0; ctri < 4; ctri++) {
            m[4 * ctri] = gc->transform.texture[index]->matrix.matrix[0][ctri];
            m[4 * ctri + 1] = gc->transform.texture[index]->matrix.matrix[1][ctri];
            m[4 * ctri + 2] = gc->transform.texture[index]->matrix.matrix[2][ctri];
            m[4 * ctri + 3] = gc->transform.texture[index]->matrix.matrix[3][ctri];
        }
        break;
    }
}

GLuint __glGetCurrentMatrixStackDepth(__GLcontext * gc)
{
    GLint index;

    switch(gc->state.transform.matrixMode) {
    case GL_MODELVIEW:
        return (GLuint)(1 + (gc->transform.modelView - gc->transform.modelViewStack));
        break;
    case GL_PROJECTION:
        return (GLuint)(1 + (gc->transform.projection - gc->transform.projectionStack));
        break;
    case GL_TEXTURE:
        index = gc->state.texture.activeTexIndex;
        return (GLuint)(1 + (gc->transform.texture[index] - gc->transform.textureStack[index]));
        break;

    default:
        return 0;
        break;
    }
}


/************************************************************************/

GLvoid APIENTRY __glim_MatrixMode(__GLcontext *gc, GLenum mode)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    switch (mode)
    {
    case GL_MODELVIEW:
        gc->transform.pushMatrix   = __glPushModelViewMatrix;
        gc->transform.popMatrix    = __glPopModelViewMatrix;
        gc->transform.loadIdentity = __glLoadIdentityModelViewMatrix;
        break;
    case GL_PROJECTION:
        gc->transform.pushMatrix   = __glPushProjectionMatrix;
        gc->transform.popMatrix    = __glPopProjectionMatrix;
        gc->transform.loadIdentity = __glLoadIdentityProjectionMatrix;
        break;
    case GL_TEXTURE:
        gc->transform.pushMatrix   = __glPushTextureMatrix;
        gc->transform.popMatrix    = __glPopTextureMatrix;
        gc->transform.loadIdentity = __glLoadIdentityTextureMatrix;
        break;
    default:
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }
    gc->state.transform.matrixMode = mode;
}

GLvoid APIENTRY __glim_LoadIdentity(__GLcontext *gc)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    (*gc->transform.loadIdentity)(gc);
}

GLvoid APIENTRY __glim_LoadMatrixf(__GLcontext *gc, const GLfloat *m)
{
    __GLmatrix m1;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    m1.matrix[0][0] = m[0];
    m1.matrix[0][1] = m[1];
    m1.matrix[0][2] = m[2];
    m1.matrix[0][3] = m[3];
    m1.matrix[1][0] = m[4];
    m1.matrix[1][1] = m[5];
    m1.matrix[1][2] = m[6];
    m1.matrix[1][3] = m[7];
    m1.matrix[2][0] = m[8];
    m1.matrix[2][1] = m[9];
    m1.matrix[2][2] = m[10];
    m1.matrix[2][3] = m[11];
    m1.matrix[3][0] = m[12];
    m1.matrix[3][1] = m[13];
    m1.matrix[3][2] = m[14];
    m1.matrix[3][3] = m[15];
    m1.matrixType = __GL_MT_GENERAL;

    __glDoLoadMatrix(gc, &m1);
}

GLvoid APIENTRY __glim_LoadTransposeMatrixf(__GLcontext *gc, const GLfloat *m)
{
    __GLmatrix m1;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    m1.matrix[0][0] = m[0];
    m1.matrix[0][1] = m[4];
    m1.matrix[0][2] = m[8];
    m1.matrix[0][3] = m[12];
    m1.matrix[1][0] = m[1];
    m1.matrix[1][1] = m[5];
    m1.matrix[1][2] = m[9];
    m1.matrix[1][3] = m[13];
    m1.matrix[2][0] = m[2];
    m1.matrix[2][1] = m[6];
    m1.matrix[2][2] = m[10];
    m1.matrix[2][3] = m[14];
    m1.matrix[3][0] = m[3];
    m1.matrix[3][1] = m[7];
    m1.matrix[3][2] = m[11];
    m1.matrix[3][3] = m[15];
    m1.matrixType = __GL_MT_GENERAL;

    __glDoLoadMatrix(gc, &m1);
}


GLvoid APIENTRY __glim_LoadMatrixd(__GLcontext *gc, const GLdouble *m)
{
    __GLmatrix m1;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    m1.matrix[0][0] = m[0];
    m1.matrix[0][1] = m[1];
    m1.matrix[0][2] = m[2];
    m1.matrix[0][3] = m[3];
    m1.matrix[1][0] = m[4];
    m1.matrix[1][1] = m[5];
    m1.matrix[1][2] = m[6];
    m1.matrix[1][3] = m[7];
    m1.matrix[2][0] = m[8];
    m1.matrix[2][1] = m[9];
    m1.matrix[2][2] = m[10];
    m1.matrix[2][3] = m[11];
    m1.matrix[3][0] = m[12];
    m1.matrix[3][1] = m[13];
    m1.matrix[3][2] = m[14];
    m1.matrix[3][3] = m[15];
    m1.matrixType = __GL_MT_GENERAL;

    __glDoLoadMatrix(gc, &m1);
}

GLvoid APIENTRY __glim_LoadTransposeMatrixd(__GLcontext *gc, const GLdouble *m)
{
    __GLmatrix m1;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    m1.matrix[0][0] = m[0];
    m1.matrix[0][1] = m[4];
    m1.matrix[0][2] = m[8];
    m1.matrix[0][3] = m[12];
    m1.matrix[1][0] = m[1];
    m1.matrix[1][1] = m[5];
    m1.matrix[1][2] = m[9];
    m1.matrix[1][3] = m[13];
    m1.matrix[2][0] = m[2];
    m1.matrix[2][1] = m[6];
    m1.matrix[2][2] = m[10];
    m1.matrix[2][3] = m[14];
    m1.matrix[3][0] = m[3];
    m1.matrix[3][1] = m[7];
    m1.matrix[3][2] = m[11];
    m1.matrix[3][3] = m[15];
    m1.matrixType = __GL_MT_GENERAL;

    __glDoLoadMatrix(gc, &m1);
}


GLvoid APIENTRY __glim_MultMatrixf(__GLcontext *gc, const GLfloat *m)
{
    __GLmatrix m1;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    m1.matrix[0][0] = m[0];
    m1.matrix[0][1] = m[1];
    m1.matrix[0][2] = m[2];
    m1.matrix[0][3] = m[3];
    m1.matrix[1][0] = m[4];
    m1.matrix[1][1] = m[5];
    m1.matrix[1][2] = m[6];
    m1.matrix[1][3] = m[7];
    m1.matrix[2][0] = m[8];
    m1.matrix[2][1] = m[9];
    m1.matrix[2][2] = m[10];
    m1.matrix[2][3] = m[11];
    m1.matrix[3][0] = m[12];
    m1.matrix[3][1] = m[13];
    m1.matrix[3][2] = m[14];
    m1.matrix[3][3] = m[15];
    m1.matrixType = __GL_MT_GENERAL;

    __glDoMultMatrix(gc, &m1, __glMultiplyMatrix);
}


GLvoid APIENTRY __glim_MultTransposeMatrixf(__GLcontext *gc, const GLfloat *m)
{
    __GLmatrix m1;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    m1.matrix[0][0] = m[0];
    m1.matrix[0][1] = m[4];
    m1.matrix[0][2] = m[8];
    m1.matrix[0][3] = m[12];
    m1.matrix[1][0] = m[1];
    m1.matrix[1][1] = m[5];
    m1.matrix[1][2] = m[9];
    m1.matrix[1][3] = m[13];
    m1.matrix[2][0] = m[2];
    m1.matrix[2][1] = m[6];
    m1.matrix[2][2] = m[10];
    m1.matrix[2][3] = m[14];
    m1.matrix[3][0] = m[3];
    m1.matrix[3][1] = m[7];
    m1.matrix[3][2] = m[11];
    m1.matrix[3][3] = m[15];
    m1.matrixType = __GL_MT_GENERAL;

    __glDoMultMatrix(gc, &m1, __glMultiplyMatrix);
}

GLvoid APIENTRY __glim_MultMatrixd(__GLcontext *gc, const GLdouble *m)
{
    __GLmatrix m1;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    m1.matrix[0][0] = m[0];
    m1.matrix[0][1] = m[1];
    m1.matrix[0][2] = m[2];
    m1.matrix[0][3] = m[3];
    m1.matrix[1][0] = m[4];
    m1.matrix[1][1] = m[5];
    m1.matrix[1][2] = m[6];
    m1.matrix[1][3] = m[7];
    m1.matrix[2][0] = m[8];
    m1.matrix[2][1] = m[9];
    m1.matrix[2][2] = m[10];
    m1.matrix[2][3] = m[11];
    m1.matrix[3][0] = m[12];
    m1.matrix[3][1] = m[13];
    m1.matrix[3][2] = m[14];
    m1.matrix[3][3] = m[15];
    m1.matrixType = __GL_MT_GENERAL;

    __glDoMultMatrix(gc, &m1, __glMultiplyMatrix);
}

GLvoid APIENTRY __glim_MultTransposeMatrixd(__GLcontext *gc, const GLdouble *m)
{
    __GLmatrix m1;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    m1.matrix[0][0] = m[0];
    m1.matrix[0][1] = m[4];
    m1.matrix[0][2] = m[8];
    m1.matrix[0][3] = m[12];
    m1.matrix[1][0] = m[1];
    m1.matrix[1][1] = m[5];
    m1.matrix[1][2] = m[9];
    m1.matrix[1][3] = m[13];
    m1.matrix[2][0] = m[2];
    m1.matrix[2][1] = m[6];
    m1.matrix[2][2] = m[10];
    m1.matrix[2][3] = m[14];
    m1.matrix[3][0] = m[3];
    m1.matrix[3][1] = m[7];
    m1.matrix[3][2] = m[11];
    m1.matrix[3][3] = m[15];
    m1.matrixType = __GL_MT_GENERAL;

    __glDoMultMatrix(gc, &m1, __glMultiplyMatrix);
}

GLvoid APIENTRY __glim_Rotatef(__GLcontext *gc, GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    __glDoRotate(gc, angle, x, y, z);
}

GLvoid APIENTRY __glim_Rotated(__GLcontext *gc, GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    __glDoRotate(gc, angle, x, y, z);
}

GLvoid APIENTRY __glim_Scalef(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    __glDoScale(gc, x, y, z);
}

GLvoid APIENTRY __glim_Scaled(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    __glDoScale(gc, x, y, z);
}

GLvoid APIENTRY __glim_Translatef(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    __glDoTranslate(gc, x, y, z);
}

GLvoid APIENTRY __glim_Translated(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    __glDoTranslate(gc, x, y, z);
}

GLvoid APIENTRY __glim_PushMatrix(__GLcontext *gc)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    (*gc->transform.pushMatrix)(gc);
}

GLvoid APIENTRY __glim_PopMatrix(__GLcontext *gc)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    (*gc->transform.popMatrix)(gc);
}

GLvoid APIENTRY __glim_Frustum(__GLcontext *gc, GLdouble left, GLdouble right,
                             GLdouble bottom, GLdouble top,
                             GLdouble zNear, GLdouble zFar)
{
    __GLmatrix m;
    __GLfloat deltaX, deltaY, deltaZ;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    deltaX = right - left;
    deltaY = top - bottom;
    deltaZ = zFar - zNear;
    if ((zNear <= __glZero) || (zFar <= __glZero) || (deltaX == __glZero) ||
        (deltaY == __glZero) || (deltaZ == __glZero)) {
            __glSetError(gc, GL_INVALID_VALUE);
            return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    (*gc->transform.matrix.makeIdentity)(&m);
    m.matrix[0][0] = zNear * __glTwo / deltaX;
    m.matrix[1][1] = zNear * __glTwo / deltaY;
    m.matrix[2][0] = (right + left) / deltaX;
    m.matrix[2][1] = (top + bottom) / deltaY;
    m.matrix[2][2] = -(zFar + zNear) / deltaZ;
    m.matrix[2][3] = __glMinusOne;
    m.matrix[3][2] = ((__GLfloat) -2.0) * zNear * zFar / deltaZ;
    m.matrix[3][3] = __glZero;
    m.matrixType = __GL_MT_GENERAL;
    __glDoMultMatrix(gc, &m, __glMultiplyMatrix);
}

GLvoid APIENTRY __glim_Ortho(__GLcontext *gc, GLdouble left, GLdouble right, GLdouble bottom,
                           GLdouble top, GLdouble zNear, GLdouble zFar)
{
    __GLmatrix m;
    GLdouble deltax, deltay, deltaz;
    GLdouble zero;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    deltax = right - left;
    deltay = top - bottom;
    deltaz = zFar - zNear;
    if ((deltax == __glZero) || (deltay == __glZero) || (deltaz == __glZero)) {
        __glSetError(gc, GL_INVALID_VALUE);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    (*gc->transform.matrix.makeIdentity)(&m);
    m.matrix[0][0] = __glTwo / deltax;
    m.matrix[3][0] = -(right + left) / deltax;
    m.matrix[1][1] = __glTwo / deltay;
    m.matrix[3][1] = -(top + bottom) / deltay;
    m.matrix[2][2] = ((__GLfloat) -2.0) / deltaz;
    m.matrix[3][2] = -(zFar + zNear) / deltaz;

    /*
    ** Screen coordinates matrix?
    */
    zero = 0.0;
    if (left == zero &&
        bottom == zero &&
        right == (GLdouble) gc->state.viewport.width &&
        top == (GLdouble) gc->state.viewport.height &&
        zNear <= zero &&
        zFar >= zero) {
            m.matrixType = __GL_MT_IS2DNRSC;
    }
    else {
        m.matrixType = __GL_MT_IS2DNR;
    }

    __glDoMultMatrix(gc, &m, __glMultiplyMatrix);
}

GLvoid __glInitTransformState(__GLcontext *gc)
{
    __GLtransform *tr;
    GLuint unit;

    gc->state.current.normal.f.z = __glOne;

    gc->state.transform.matrixMode = GL_MODELVIEW;
    gc->state.viewport.zNear = __glZero;
    gc->state.viewport.zFar = __glOne;

    gc->transform.clipSeqNum = 0;

    gc->transform.pushMatrix = __glPushModelViewMatrix;
    gc->transform.popMatrix = __glPopModelViewMatrix;
    gc->transform.loadIdentity = __glLoadIdentityModelViewMatrix;
    gc->transform.matrix.copy = __glCopyMatrix;
    gc->transform.matrix.invertTranspose = __glInvertTransposeMatrix;
    gc->transform.matrix.makeIdentity = __glMakeIdentity;
    gc->transform.matrix.mult = __glMultMatrix;

    /* Allocate memory for matrix stacks */
    gc->transform.modelViewStack = (__GLtransform*)(*gc->imports.calloc)
        (gc, gc->constants.maxModelViewStackDepth, sizeof(__GLtransform) );

    gc->transform.modelView = tr = &gc->transform.modelViewStack[0];
    (*gc->transform.matrix.makeIdentity)(&tr->matrix);
    (*gc->transform.matrix.makeIdentity)(&tr->inverseTranspose);
    (*gc->transform.matrix.makeIdentity)(&tr->mvp);
    tr->updateInverse = GL_FALSE;

    gc->transform.projectionStack = (__GLtransform*)(*gc->imports.calloc)
        (gc, gc->constants.maxProjectionStackDepth, sizeof(__GLtransform) );

    gc->transform.projection = tr = &gc->transform.projectionStack[0];
    (*gc->transform.matrix.makeIdentity)(&tr->matrix);

    for (unit = 0; unit < __GL_MAX_TEXTURE_UNITS; unit++) {
        gc->transform.textureStack[unit] = (__GLtransform*)(*gc->imports.calloc)
            (gc, gc->constants.maxTextureStackDepth, sizeof(__GLtransform) );

        gc->transform.texture[unit] = tr = &gc->transform.textureStack[unit][0];
        (*gc->transform.matrix.makeIdentity)(&tr->matrix);
    }

    for (unit = 0; unit < __GL_MAX_PROGRAM_MATRICES; unit++) {
        gc->transform.programStack[unit] = (__GLtransform*)(*gc->imports.calloc)
            (gc, gc->constants.maxProgramStackDepth, sizeof(__GLtransform) );

        gc->transform.program[unit] = tr = &gc->transform.programStack[unit][0];
        (*gc->transform.matrix.makeIdentity)(&tr->matrix);
    }

    gc->state.enables.scissorTest = GL_FALSE;
}

GLvoid __glFreeTransformState(__GLcontext *gc)
{
    GLuint unit;

    (*gc->imports.free)(gc, gc->transform.modelViewStack);
    (*gc->imports.free)(gc, gc->transform.projectionStack);

    for (unit = 0; unit < __GL_MAX_TEXTURE_UNITS; unit++) {
        (*gc->imports.free)(gc, gc->transform.textureStack[unit]);
    }

    for (unit = 0; unit < __GL_MAX_PROGRAM_MATRICES; unit++) {
        (*gc->imports.free)(gc, gc->transform.programStack[unit]);
    }
}


/*
** Compute the clip box from the scissor (if enabled) and the window
** size.  The resulting clip box is used to clip primitive rasterization
** against.  The "window system" is responsible for doing the fine
** grain clipping (i.e., dealing with overlapping windows, etc.).
*/
GLvoid __glComputeClipBox(__GLcontext *gc)
{
#if defined(_LINUX_) && defined(GL4_DRI_BUILD)
    GLint llx, lly, urx, ury;
    GLint y0, y1;

    if (gc->state.enables.scissorTest)
    {
        llx = gc->state.scissor.scissorX;
        lly = gc->state.scissor.scissorY;
        urx = llx + gc->state.scissor.scissorWidth;
        ury = lly + gc->state.scissor.scissorHeight;

        if ((urx < 0) || (ury < 0) ||
            (urx < llx) || (ury < lly) ||
            (llx >= gc->drawablePrivate->width) || (lly >= gc->drawablePrivate->height))
        {
            llx = lly = urx = ury = 0;
        }
        else
        {
            if (llx < 0) llx = 0;
            if (lly < 0) lly = 0;
            if (urx > gc->drawablePrivate->width)
                urx = gc->drawablePrivate->width;
            if (ury > gc->drawablePrivate->height)
                ury = gc->drawablePrivate->height;
        }
    }
    else
    {
        llx = 0;
        lly = 0;
        urx = gc->drawablePrivate->width;
        ury = gc->drawablePrivate->height;
    }

    if (gc->drawablePrivate->yInverted)
    {
        y0 = gc->drawablePrivate->height - ury;
        y1 = gc->drawablePrivate->height - lly;
    }
    else
    {
        y0 = lly;
        y1 = ury;
    }

    gc->transform.clipX0 = llx;
    gc->transform.clipY0 = y0;
    gc->transform.clipX1 = urx;
    gc->transform.clipY1 = y1;
#endif
}

#if defined(_WIN32)
#pragma warning(default: 4244)
#endif

