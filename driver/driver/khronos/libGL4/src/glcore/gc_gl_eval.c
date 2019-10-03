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


#include "gc_es_context.h"
#include "gc_es_debug.h"

#define CHANGE_COLOR    1
#define CHANGE_NORMAL   2
#define CHANGE_TEXTURE  4
#define CHANGE_VERTEX3  8
#define CHANGE_VERTEX4  16
#define TYPE_COEFF_AND_DERIV    1
#define TYPE_COEFF      2

#define __GL_ACTIVE_TEXCOORD(gc) ((gc)->state.current.texture[(gc)->state.texture.activeTexIndex])

/*
*** Functions in math.c
*/
extern GLvoid __glNormalize(GLfloat vout[3], const GLfloat v[3]);
extern GLvoid __glNormalizeTwo(const GLfloat v[3], GLfloat voa[3], GLfloat vob[3]);
/* Functions in s_get.c */
extern GLvoid __glConvertResult(__GLcontext *gc, GLint fromType, const GLvoid *rawdata,
                              GLint toType, GLvoid *result, GLint size);

static const
struct defaultMap {
    GLint    index;
    GLint    k;
    __GLfloat    values[4];
} defaultMaps[__GL_MAP_RANGE_COUNT] = {
    {__GL_C4, 4, {1.0, 1.0, 1.0, 1.0}},
    {__GL_I , 1, {1.0, 0.0, 0.0, 0.0}},
    {__GL_N3, 3, {0.0, 0.0, 1.0, 0.0}},
    {__GL_T1, 1, {0.0, 0.0, 0.0, 0.0}},
    {__GL_T2, 2, {0.0, 0.0, 0.0, 0.0}},
    {__GL_T3, 3, {0.0, 0.0, 0.0, 0.0}},
    {__GL_T4, 4, {0.0, 0.0, 0.0, 1.0}},
    {__GL_V3, 3, {0.0, 0.0, 0.0, 0.0}},
    {__GL_V4, 4, {0.0, 0.0, 0.0, 1.0}},
};

GLvoid __glInitEvaluatorState(__GLcontext *gc)
{
    GLint i,j;
    const struct defaultMap *defMap;
    __GLevaluator1 *eval1;
    __GLevaluator2 *eval2;
    __GLfloat **eval1Data;
    __GLfloat **eval2Data;

    for (i = 0; i < __GL_MAP_RANGE_COUNT; i++) {
        defMap = &(defaultMaps[i]);
        eval1 = &(gc->eval.eval1[i]);
        eval2 = &(gc->eval.eval2[i]);
        eval1Data = &(gc->eval.eval1Data[i]);
        eval2Data = &(gc->eval.eval2Data[i]);

        eval1->order = 1;
        eval1->u1 = __glZero;
        eval1->u2 = __glOne;
        eval1->k = defMap->k;
        eval2->majorOrder = 1;
        eval2->minorOrder = 1;
        eval2->u1 = __glZero;
        eval2->u2 = __glOne;
        eval2->v1 = __glZero;
        eval2->v2 = __glOne;
        eval2->k = defMap->k;
        *eval1Data = (__GLfloat *)
            (*gc->imports.malloc)(gc, (size_t) (sizeof(__GLfloat) * defMap->k) );
        *eval2Data = (__GLfloat *)
            (*gc->imports.malloc)(gc, (size_t) (sizeof(__GLfloat) * defMap->k) );
        for (j = 0; j < defMap->k; j++) {
            (*eval1Data)[j] = defMap->values[j];
            (*eval2Data)[j] = defMap->values[j];
        }
    }

    gc->eval.uorder = 0;
    gc->eval.vorder = 0;

    gc->state.evaluator.u1.start = __glZero;
    gc->state.evaluator.u2.start = __glZero;
    gc->state.evaluator.v2.start = __glZero;
    gc->state.evaluator.u1.finish = __glOne;
    gc->state.evaluator.u2.finish = __glOne;
    gc->state.evaluator.v2.finish = __glOne;
    gc->state.evaluator.u1.n = 1;
    gc->state.evaluator.u2.n = 1;
    gc->state.evaluator.v2.n = 1;
}

GLint __glMap1_size(GLint k, GLint order)
{
    return (k * order);
}

GLint __glMap2_size(GLint k, GLint majorOrder, GLint minorOrder)
{
    return (k * majorOrder * minorOrder);
}

/*
** Fill our data from user data
*/
GLvoid __glFillMap1fInternal(GLint k, GLint order, GLint stride,
                   const GLfloat *points, GLfloat *data)
{
    GLint i,j;

#ifndef __GL_DOUBLE
    /* Optimization always hit during display list execution */
    if (k == stride)
    {
        __GL_MEMCOPY(data, points,
            __glMap1_size(k, order) * sizeof(GLfloat));
        return;
    }
#endif
    for (i=0; i<order; i++)
    {
        for (j=0; j<k; j++)
        {
            data[j] = points[j];
        }
        points += stride;
        data += k;
    }
}

GLvoid __glFillMap1dInternal(GLint k, GLint order, GLint stride,
                   const GLdouble *points, GLfloat *data)
{
    GLint i,j;

    for (i=0; i<order; i++)
    {
        for (j=0; j<k; j++)
        {
            data[j] = (GLfloat)points[j];
        }
        points += stride;
        data += k;
    }
}

GLvoid __glFillMap2fInternal(GLint k, GLint majorOrder, GLint minorOrder,
                   GLint majorStride, GLint minorStride,
                   const GLfloat *points, GLfloat *data)
{
    GLint i,j,x;

#ifndef __GL_DOUBLE
    /* Optimization always hit during display list execution */
    if (k == minorStride && majorStride == k * minorOrder)
    {
        __GL_MEMCOPY(data, points,
            __glMap2_size(k, majorOrder, minorOrder) * sizeof(GLfloat));
        return;
    }
#endif
    for (i=0; i<majorOrder; i++)
    {
        for (j=0; j<minorOrder; j++)
        {
            for (x=0; x<k; x++)
            {
                data[x] = points[x];
            }
            points += minorStride;
            data += k;
        }
        points += majorStride - minorStride * minorOrder;
    }
}

GLvoid __glFillMap2dInternal(GLint k, GLint majorOrder, GLint minorOrder,
                   GLint majorStride, GLint minorStride,
                   const GLdouble *points, GLfloat *data)
{
    GLint i,j,x;

    for (i=0; i<majorOrder; i++)
    {
        for (j=0; j<minorOrder; j++)
        {
            for (x=0; x<k; x++)
            {
                data[x] = (GLfloat)points[x];
            }
            points += minorStride;
            data += k;
        }
        points += majorStride - minorStride * minorOrder;
    }
}

/*
** Optimization to precompute coefficients for polynomial evaluation.
*/
GLvoid PreEvaluate(GLint order, GLfloat vprime, GLfloat *coeff)
{
    GLint i, j;
    GLfloat oldval, temp;
    GLfloat oneMinusvprime;

    /*
    ** Minor optimization
    ** Compute orders 1 and 2 outright, and set coeff[0], coeff[1] to
    ** their i==1 loop values to avoid the initialization and the i==1 loop.
    */
    if (order == 1)
    {
        coeff[0] = ((GLfloat) 1.0);
        return;
    }

    oneMinusvprime = 1-vprime;
    coeff[0] = oneMinusvprime;
    coeff[1] = vprime;
    if (order == 2) return;

    for (i = 2; i < order; i++)
    {
        oldval = coeff[0] * vprime;
        coeff[0] = oneMinusvprime * coeff[0];
        for (j = 1; j < i; j++)
        {
            temp = oldval;
            oldval = coeff[j] * vprime;
            coeff[j] = temp + oneMinusvprime * coeff[j];
        }
        coeff[j] = oldval;
    }
}

/*
** Optimization to precompute coefficients for polynomial evaluation.
*/
GLvoid PreEvaluateWithDeriv(GLint order, GLfloat vprime,
                          GLfloat *coeff, GLfloat *coeffDeriv)
{
    GLint i, j;
    GLfloat oldval, temp;
    GLfloat oneMinusvprime;

    oneMinusvprime = 1-vprime;
    /*
    ** Minor optimization
    ** Compute orders 1 and 2 outright, and set coeff[0], coeff[1] to
    ** their i==1 loop values to avoid the initialization and the i==1 loop.
    */
    if (order == 1)
    {
        coeff[0] = ((GLfloat) 1.0);
        coeffDeriv[0] = __glZero;
        return;
    }
    else if (order == 2)
    {
        coeffDeriv[0] = __glMinusOne;
        coeffDeriv[1] = ((GLfloat) 1.0);
        coeff[0] = oneMinusvprime;
        coeff[1] = vprime;
        return;
    }
    coeff[0] = oneMinusvprime;
    coeff[1] = vprime;
    for (i = 2; i < order - 1; i++)
    {
        oldval = coeff[0] * vprime;
        coeff[0] = oneMinusvprime * coeff[0];
        for (j = 1; j < i; j++)
        {
            temp = oldval;
            oldval = coeff[j] * vprime;
            coeff[j] = temp + oneMinusvprime * coeff[j];
        }
        coeff[j] = oldval;
    }
    coeffDeriv[0] = -coeff[0];
    /*
    ** Minor optimization:
    ** Would make this a "for (j=1; j<order-1; j++)" loop, but it is always
    ** executed at least once, so this is more efficient.
    */
    j=1;
    do
    {
        coeffDeriv[j] = coeff[j-1] - coeff[j];
        j++;
    } while (j < order - 1);
    coeffDeriv[j] = coeff[j-1];

    oldval = coeff[0] * vprime;
    coeff[0] = oneMinusvprime * coeff[0];
    for (j = 1; j < i; j++)
    {
        temp = oldval;
        oldval = coeff[j] * vprime;
        coeff[j] = temp + oneMinusvprime * coeff[j];
    }
    coeff[j] = oldval;
}

GLvoid DoDomain1(__GLevaluatorMachine *em, GLfloat u, __GLevaluator1 *e,
               GLfloat *v, GLfloat *baseData)
{
    GLint j, row;
    GLfloat uprime;
    GLfloat *data;
    GLint k;

    if (e->u2 == e->u1)
        return;
    uprime = (u - e->u1) / (e->u2 - e->u1);

    /* Use already cached values if possible */
    if (em->uvalue != uprime || em->uorder != e->order)
    {
        /* Compute coefficients for values */
        PreEvaluate(e->order, uprime, em->ucoeff);
        em->utype = TYPE_COEFF;
        em->uorder = e->order;
        em->uvalue = uprime;
    }

    k=e->k;
    for (j = 0; j < k; j++)
    {
        data=baseData+j;
        v[j] = 0;
        for (row = 0; row < e->order; row++)
        {
            v[j] += em->ucoeff[row] * (*data);
            data += k;
        }
    }
}

GLvoid DoEvalCoord1(__GLcontext *gc, GLfloat u)
{
    __GLevaluator1 *eval = gc->eval.eval1;
    GLfloat **evalData = gc->eval.eval1Data;
    GLboolean restorecolor0 = GL_FALSE;
    __GLcolor color0;
    __GLevaluatorMachine em = gc->eval;

    color0.r = color0.g = color0.b = color0.a = 0.0F;

    {
        if (gc->state.enables.eval.map1[__GL_C4])
        {

            restorecolor0 = GL_TRUE;
            color0 = gc->state.current.color;

            DoDomain1(&em, u, &eval[__GL_C4], &gc->state.current.color.r,
                evalData[__GL_C4]);
            (*gc->immedModeDispatch.Color4fv)(gc ,&gc->state.current.color.r);
        }
    }

    if (gc->state.enables.eval.map1[__GL_T4])
    {
        DoDomain1(&em, u, &eval[__GL_T4], &__GL_ACTIVE_TEXCOORD(gc).f.x,
            evalData[__GL_T4]);
        (*gc->immedModeDispatch.TexCoord4fv)(gc ,&__GL_ACTIVE_TEXCOORD(gc).f.x);
    }
    else if (gc->state.enables.eval.map1[__GL_T3])
    {
        DoDomain1(&em, u, &eval[__GL_T3], &__GL_ACTIVE_TEXCOORD(gc).f.x,
            evalData[__GL_T3]);
        (*gc->immedModeDispatch.TexCoord3fv)(gc ,&__GL_ACTIVE_TEXCOORD(gc).f.x);
    }
    else if (gc->state.enables.eval.map1[__GL_T2])
    {
        DoDomain1(&em, u, &eval[__GL_T2], &__GL_ACTIVE_TEXCOORD(gc).f.x,
            evalData[__GL_T2]);
        (*gc->immedModeDispatch.TexCoord2fv)(gc ,&__GL_ACTIVE_TEXCOORD(gc).f.x);
    }
    else if (gc->state.enables.eval.map1[__GL_T1])
    {
        DoDomain1(&em, u, &eval[__GL_T1], &__GL_ACTIVE_TEXCOORD(gc).f.x,
            evalData[__GL_T1]);
        (*gc->immedModeDispatch.TexCoord1fv)(gc ,&__GL_ACTIVE_TEXCOORD(gc).f.x);
    }

    if (gc->state.enables.eval.map1[__GL_N3])
    {
        DoDomain1(&em, u, &eval[__GL_N3], &gc->state.current.normal.f.x,
            evalData[__GL_N3]);
        (*gc->immedModeDispatch.Normal3fv)(gc ,&gc->state.current.normal.f.x);
    }

    if (gc->state.enables.eval.map1[__GL_V4])
    {
        __GLcoord vvec;

        DoDomain1(&em, u, &eval[__GL_V4], &vvec.f.x, evalData[__GL_V4]);
        (*gc->immedModeDispatch.Vertex4fv)(gc ,&vvec.f.x);
    }
    else if (gc->state.enables.eval.map1[__GL_V3])
    {
        __GLcoord vvec;

        DoDomain1(&em, u, &eval[__GL_V3], &vvec.f.x, evalData[__GL_V3]);
        (*gc->immedModeDispatch.Vertex3fv)(gc ,&vvec.f.x);
    }
    if (restorecolor0)
    {
        gc->state.current.color = color0;
    }
}

GLvoid ComputeFirstPartials(GLfloat *p, GLfloat *pu, GLfloat *pv)
{
    pu[0] = pu[0]*p[3] - pu[3]*p[0];
    pu[1] = pu[1]*p[3] - pu[3]*p[1];
    pu[2] = pu[2]*p[3] - pu[3]*p[2];

    pv[0] = pv[0]*p[3] - pv[3]*p[0];
    pv[1] = pv[1]*p[3] - pv[3]*p[1];
    pv[2] = pv[2]*p[3] - pv[3]*p[2];
}

GLvoid ComputeNormal2(__GLcontext *gc, GLfloat *n, GLfloat *pu,
                    GLfloat *pv)
{
    n[0] = pu[1]*pv[2] - pu[2]*pv[1];
    n[1] = pu[2]*pv[0] - pu[0]*pv[2];
    n[2] = pu[0]*pv[1] - pu[1]*pv[0];
    __glNormalize(n, n);
}

GLvoid DoDomain2(__GLevaluatorMachine *em, GLfloat u, GLfloat v,
               __GLevaluator2 *e, GLfloat *r, GLfloat *baseData)
{
    GLint j, row, col;
    GLfloat uprime;
    GLfloat vprime;
    GLfloat p;
    GLfloat *data;
    GLint k;

    if ((e->u2 == e->u1) || (e->v2 == e->v1))
        return;
    uprime = (u - e->u1) / (e->u2 - e->u1);
    vprime = (v - e->v1) / (e->v2 - e->v1);

    /* Compute coefficients for values */

    /* Use already cached values if possible */
    if (em->uvalue != uprime || em->uorder != e->majorOrder)
    {
        PreEvaluate(e->majorOrder, uprime, em->ucoeff);
        em->utype = TYPE_COEFF;
        em->uorder = e->majorOrder;
        em->uvalue = uprime;
    }
    if (em->vvalue != vprime || em->vorder != e->minorOrder)
    {
        PreEvaluate(e->minorOrder, vprime, em->vcoeff);
        em->vtype = TYPE_COEFF;
        em->vorder = e->minorOrder;
        em->vvalue = vprime;
    }

    k=e->k;
    for (j = 0; j < k; j++)
    {
        data=baseData+j;
        r[j] = 0;
        for (row = 0; row < e->majorOrder; row++)
        {
            /*
            ** Minor optimization.
            ** The col == 0 part of the loop is extracted so we don't
            ** have to initialize p to 0.
            */
            p=em->vcoeff[0] * (*data);
            data += k;
            for (col = 1; col < e->minorOrder; col++)
            {
                p += em->vcoeff[col] * (*data);
                data += k;
            }
            r[j] += em->ucoeff[row] * p;
        }
    }
}

GLvoid DoDomain2WithDerivs(__GLevaluatorMachine *em, GLfloat u,
                         GLfloat v, __GLevaluator2 *e, GLfloat *r,
                         GLfloat *du, GLfloat *dv, GLfloat *baseData)
{
    GLint j, row, col;
    GLfloat uprime;
    GLfloat vprime;
    GLfloat p;
    GLfloat pdv;
    GLfloat *data;
    GLint k;

    if ((e->u2 == e->u1) || (e->v2 == e->v1))
        return;
    uprime = (u - e->u1) / (e->u2 - e->u1);
    vprime = (v - e->v1) / (e->v2 - e->v1);

    /* Compute coefficients for values and derivs */

    /* Use already cached values if possible */
    if (em->uvalue != uprime || em->utype != TYPE_COEFF_AND_DERIV ||
        em->uorder != e->majorOrder)
    {
        PreEvaluateWithDeriv(e->majorOrder, uprime, em->ucoeff, em->ucoeffDeriv);
        em->utype = TYPE_COEFF_AND_DERIV;
        em->uorder = e->majorOrder;
        em->uvalue = uprime;
    }
    if (em->vvalue != vprime || em->vtype != TYPE_COEFF_AND_DERIV ||
        em->vorder != e->minorOrder)
    {
        PreEvaluateWithDeriv(e->minorOrder, vprime, em->vcoeff, em->vcoeffDeriv);
        em->vtype = TYPE_COEFF_AND_DERIV;
        em->vorder = e->minorOrder;
        em->vvalue = vprime;
    }

    k=e->k;
    for (j = 0; j < k; j++)
    {
        data=baseData+j;
        r[j] = du[j] = dv[j] = __glZero;
        for (row = 0; row < e->majorOrder; row++)
        {
            /*
            ** Minor optimization.
            ** The col == 0 part of the loop is extracted so we don't
            ** have to initialize p and pdv to 0.
            */
            p = em->vcoeff[0] * (*data);
            pdv = em->vcoeffDeriv[0] * (*data);
            data += k;
            for (col = 1; col < e->minorOrder; col++)
            {
                /* Incrementally build up p, pdv value */
                p += em->vcoeff[col] * (*data);
                pdv += em->vcoeffDeriv[col] * (*data);
                data += k;
            }
            /* Use p, pdv value to incrementally add up r, du, dv */
            r[j] += em->ucoeff[row] * p;
            du[j] += em->ucoeffDeriv[row] * p;
            dv[j] += em->ucoeff[row] * pdv;
        }
    }
}

/*
** Compute the color, texture, normal, vertex based upon u and v.  If
** changes != NULL, then save the info as we go (in changes, of course).
*/
GLvoid DoEvalCoord2(__GLcontext *gc, GLfloat u, GLfloat v,
                  StateChange *changes)
{
    __GLevaluator2 *eval = gc->eval.eval2;
    GLfloat **evalData = gc->eval.eval2Data;
    GLint vertexType;
    __GLcoord vvec,vnormal;
    __GLcoord texcoord;
    __GLevaluatorMachine em = gc->eval;

    vnormal = gc->state.current.normal;
    texcoord = __GL_ACTIVE_TEXCOORD(gc);
    if (changes)
    {
        changes->changed = 0;
    }

    vertexType = -1;
    if (gc->state.enables.eval.autonormal)
    {
        if (gc->state.enables.eval.map2[__GL_V4])
        {
            GLfloat du[4];
            GLfloat dv[4];
            DoDomain2WithDerivs(&em, u, v, &eval[__GL_V4], &vvec.f.x, du, dv,
                evalData[__GL_V4]);
            ComputeFirstPartials(&vvec.f.x, du, dv);
            ComputeNormal2(gc, &vnormal.f.x, du, dv);
            (*gc->immedModeDispatch.Normal3fv)(gc ,&vnormal.f.x);
            if (changes)
            {
                changes->changed |= CHANGE_NORMAL | CHANGE_VERTEX4;
                changes->normal = vnormal;
                changes->vertex = vvec;
            }
            vertexType = 4;
        }
        else if (gc->state.enables.eval.map2[__GL_V3])
        {
            GLfloat du[3];
            GLfloat dv[3];
            DoDomain2WithDerivs(&em, u, v, &eval[__GL_V3], &vvec.f.x, du, dv,
                evalData[__GL_V3]);
            ComputeNormal2(gc, &vnormal.f.x, du, dv);
            (*gc->immedModeDispatch.Normal3fv)(gc ,&vnormal.f.x);
            if (changes)
            {
                changes->changed |= CHANGE_NORMAL | CHANGE_VERTEX3;
                changes->normal = vnormal;
                changes->vertex = vvec;
            }
            vertexType = 3;
        }
    }
    else
    {
        if (gc->state.enables.eval.map2[__GL_N3])
        {
            DoDomain2(&em, u, v, &eval[__GL_N3], &vnormal.f.x,
                evalData[__GL_N3]);
            (*gc->immedModeDispatch.Normal3fv)(gc ,&vnormal.f.x);
            if (changes)
            {
                changes->changed |= CHANGE_NORMAL;
                changes->normal = vnormal;
            }
        }
        if (gc->state.enables.eval.map2[__GL_V4])
        {
            DoDomain2(&em, u, v, &eval[__GL_V4], &vvec.f.x,
                evalData[__GL_V4]);
            if (changes)
            {
                changes->changed |= CHANGE_VERTEX4;
                changes->vertex = vvec;
            }
            vertexType = 4;
        }
        else if (gc->state.enables.eval.map2[__GL_V3])
        {
            DoDomain2(&em, u, v, &eval[__GL_V3], &vvec.f.x,
                evalData[__GL_V3]);
            if (changes)
            {
                changes->changed |= CHANGE_VERTEX3;
                changes->vertex = vvec;
            }
            vertexType = 3;
        }
    }

    {
        if (gc->state.enables.eval.map2[__GL_C4])
        {
            __GLcolor savedColor = gc->state.current.color;
            __GLcolor color = gc->state.current.color;

            DoDomain2(&em, u, v, &eval[__GL_C4], &color.r, evalData[__GL_C4]);
            (*gc->immedModeDispatch.Color4fv)(gc, &color.r);
            if (changes)
            {
                changes->changed |= CHANGE_COLOR;
                changes->color = color;
            }
            gc->state.current.color = savedColor;
        }
    }

    if (gc->state.enables.eval.map2[__GL_T4])
    {
        DoDomain2(&em, u, v, &eval[__GL_T4], &__GL_ACTIVE_TEXCOORD(gc).f.x,
            evalData[__GL_T4]);
        (*gc->immedModeDispatch.TexCoord4fv)(gc ,&texcoord.f.x);
        if (changes)
        {
            changes->changed |= CHANGE_TEXTURE;
            changes->texture = texcoord;
        }
    }
    else if (gc->state.enables.eval.map2[__GL_T3])
    {
        DoDomain2(&em, u, v, &eval[__GL_T3], &texcoord.f.x,
            evalData[__GL_T3]);
        (*gc->immedModeDispatch.TexCoord3fv)(gc ,&texcoord.f.x);
        if (changes)
        {
            changes->changed |= CHANGE_TEXTURE;
            changes->texture = texcoord;
        }
    }
    else if (gc->state.enables.eval.map2[__GL_T2])
    {
        DoDomain2(&em, u, v, &eval[__GL_T2], &texcoord.f.x,
            evalData[__GL_T2]);
                (*gc->immedModeDispatch.TexCoord2fv)(gc ,&texcoord.f.x);
        if (changes)
        {
            changes->changed |= CHANGE_TEXTURE;
            changes->texture = texcoord;
        }
    }
    else if (gc->state.enables.eval.map2[__GL_T1])
    {
        DoDomain2(&em, u, v, &eval[__GL_T1], &texcoord.f.x,
            evalData[__GL_T1]);
        (*gc->immedModeDispatch.TexCoord1fv)(gc ,&texcoord.f.x);
        if (changes)
        {
            changes->changed |= CHANGE_TEXTURE;
            changes->texture = texcoord;
        }
    }

    if (vertexType == 3)
    {
        (*gc->immedModeDispatch.Vertex3fv)(gc ,&vvec.f.x);
    }
    else if (vertexType == 4)
    {
        (*gc->immedModeDispatch.Vertex4fv)(gc ,&vvec.f.x);
    }
}

GLvoid __glDoEvalCoord1(__GLcontext *gc, GLfloat u)
{
    __GLcolor currentColor;
    __GLcoord currentNormal, currentTexture;

    currentColor = gc->state.current.color;
    currentNormal = gc->state.current.normal;
    currentTexture = __GL_ACTIVE_TEXCOORD(gc);

    DoEvalCoord1(gc, u);

    gc->state.current.color = currentColor;
    gc->state.current.normal = currentNormal;
    __GL_ACTIVE_TEXCOORD(gc) = currentTexture;
}

GLvoid __glDoEvalCoord2(__GLcontext *gc, GLfloat u, GLfloat v)
{
    __GLcolor currentColor;
    __GLcoord currentNormal, currentTexture;

    currentColor = gc->state.current.color;
    currentNormal = gc->state.current.normal;
    currentTexture = __GL_ACTIVE_TEXCOORD(gc);

    DoEvalCoord2(gc, u, v, NULL);

    gc->state.current.color = currentColor;
    gc->state.current.normal = currentNormal;
    __GL_ACTIVE_TEXCOORD(gc) = currentTexture;
}

GLvoid __glEvalMesh1Line(__GLcontext *gc, GLint low, GLint high)
{
    __GLevaluatorGrid *u = &gc->state.evaluator.u1;
    GLfloat du;
    GLint i;
    __GLcolor currentColor;
    __GLcoord currentNormal, currentTexture;

    if ( u->n == 0)
        return;
    du = (u->finish - u->start)/(GLfloat)u->n;

    currentColor = gc->state.current.color;
    currentNormal = gc->state.current.normal;
    currentTexture = __GL_ACTIVE_TEXCOORD(gc);

    /*
    ** multiplication instead of iterative adding done to prevent
    ** accumulation of error.
    */
    (*gc->immedModeDispatch.Begin)(gc ,GL_LINE_STRIP);
    for (i = low; i <= high ; i++)
    {
        DoEvalCoord1(gc, (i == u->n) ? u->finish : (u->start + i * du));
    }
    (*gc->immedModeDispatch.End)(gc);

    gc->state.current.color = currentColor;
    gc->state.current.normal = currentNormal;
    __GL_ACTIVE_TEXCOORD(gc) = currentTexture;
}

GLvoid __glEvalMesh1Point(__GLcontext *gc, GLint low, GLint high)
{
    __GLevaluatorGrid *u = &gc->state.evaluator.u1;
    GLfloat du;
    GLint i;
    __GLcolor currentColor;
    __GLcoord currentNormal, currentTexture;

    if ( u->n == 0)
        return;
    du = (u->finish - u->start)/(GLfloat)u->n;

    currentColor = gc->state.current.color;
    currentNormal = gc->state.current.normal;
    currentTexture = __GL_ACTIVE_TEXCOORD(gc);

    /*
    ** multiplication instead of iterative adding done to prevent
    ** accumulation of error.
    */
    (*gc->immedModeDispatch.Begin)(gc ,GL_POINTS);
    for (i = low; i <= high ; i++)
    {
        DoEvalCoord1(gc, (i == u->n) ? u->finish : (u->start + i * du));
    }
    (*gc->immedModeDispatch.End)(gc);

    gc->state.current.color = currentColor;
    gc->state.current.normal = currentNormal;
    __GL_ACTIVE_TEXCOORD(gc) = currentTexture;
}

GLvoid sendChange(__GLcontext *gc, StateChange *change)
{
    if (change->changed & CHANGE_COLOR)
    {
        gc->state.current.color = change->color;
        (*gc->immedModeDispatch.Color4fv)(gc, &change->color.r);
    }
    if (change->changed & CHANGE_TEXTURE)
    {
        __GL_ACTIVE_TEXCOORD(gc) = change->texture;
        (*gc->immedModeDispatch.TexCoord4fv)(gc, &change->texture.f.x);
    }
    if (change->changed & CHANGE_NORMAL)
    {
        gc->state.current.normal = change->normal;
        (*gc->immedModeDispatch.Normal3fv)(gc, &change->normal.f.x);
    }
    if (change->changed & CHANGE_VERTEX3)
    {
        (*gc->immedModeDispatch.Vertex3fv)(gc, &change->vertex.f.x);
    }
    else if (change->changed & CHANGE_VERTEX4)
    {
        (*gc->immedModeDispatch.Vertex4fv)(gc, &change->vertex.f.x);
    }


}

/*
** MAX_WIDTH is the largest grid size we expect in one direction.  If
** a grid size larger than this is asked for, then the last n rows will
** have all of their points calculated twice!
*/
#define MAX_WIDTH   1024

GLvoid __glEvalMesh2Fill(__GLcontext *gc, GLint lowU, GLint lowV,
                       GLint highU, GLint highV)
{
    __GLevaluatorGrid *u = &gc->state.evaluator.u2;
    __GLevaluatorGrid *v = &gc->state.evaluator.v2;
    GLfloat du, dv;
    GLint i, j;
    __GLcolor currentColor;
    __GLcoord currentNormal, currentTexture;
    GLint row;
    StateChange changes[MAX_WIDTH];

    if ((u->n == 0)|| (v->n == 0))
        return;
    du = (u->finish - u->start)/(GLfloat)u->n;
    dv = (v->finish - v->start)/(GLfloat)v->n;

    currentColor = gc->state.current.color;
    currentNormal = gc->state.current.normal;
    currentTexture = __GL_ACTIVE_TEXCOORD(gc);

    for (i = lowU; i < highU; i++)
    {
        GLfloat u1 = (i == u->n) ? u->finish : (u->start + i * du);
        GLfloat u2 = ((i+1) == u->n) ? u->finish : (u->start + (i+1) * du);
        (*gc->immedModeDispatch.Begin)(gc, GL_QUAD_STRIP);
        row=0;
        for (j = highV; j >= lowV; j--)
        {
            GLfloat v1 = (j == v->n) ? v->finish : (v->start + j * dv);

            if (row < MAX_WIDTH)
            {
                /* use cached info if possible */
                if (i != lowU)
                {
                    sendChange(gc, changes+row);
                }
                else
                {
                    DoEvalCoord2(gc, u1, v1, NULL);
                }
                /* cache the answer for next iteration of i */
                DoEvalCoord2(gc, u2, v1, changes+row);
            }
            else
            {
                /* row larger than we expected likely.  No data is cached */
                DoEvalCoord2(gc, u1, v1, NULL);
                DoEvalCoord2(gc, u2, v1, NULL);
            }
            row++;

        }
        (*gc->immedModeDispatch.End)(gc);
    }

    gc->state.current.color = currentColor;
    gc->state.current.normal = currentNormal;
    __GL_ACTIVE_TEXCOORD(gc) = currentTexture;
}

GLvoid __glEvalMesh2Point(__GLcontext *gc, GLint lowU, GLint lowV,
                        GLint highU, GLint highV)
{
    __GLevaluatorGrid *u = &gc->state.evaluator.u2;
    __GLevaluatorGrid *v = &gc->state.evaluator.v2;
    GLfloat du, dv;
    GLint i, j;
    __GLcolor currentColor;
    __GLcoord currentNormal, currentTexture;

    if ((u->n == 0)|| (v->n == 0))
        return;
    du = (u->finish - u->start)/(GLfloat)u->n;
    dv = (v->finish - v->start)/(GLfloat)v->n;

    currentColor = gc->state.current.color;
    currentNormal = gc->state.current.normal;
    currentTexture = __GL_ACTIVE_TEXCOORD(gc);

    (*gc->immedModeDispatch.Begin)(gc, GL_POINTS);
    for (i = lowU; i <= highU; i++)
    {
        GLfloat u1 = (i == u->n) ? u->finish : (u->start + i * du);
        for (j = lowV; j <= highV; j++)
        {
            GLfloat v1 = (j == v->n) ? v->finish : (v->start + j * dv);

            DoEvalCoord2(gc, u1, v1, NULL);
        }
    }
    (*gc->immedModeDispatch.End)(gc);

    gc->state.current.color = currentColor;
    gc->state.current.normal = currentNormal;
    __GL_ACTIVE_TEXCOORD(gc) = currentTexture;
}

GLvoid __glEvalMesh2Line(__GLcontext *gc, GLint lowU, GLint lowV,
                       GLint highU, GLint highV)
{
    __GLevaluatorGrid *u = &gc->state.evaluator.u2;
    __GLevaluatorGrid *v = &gc->state.evaluator.v2;
    GLfloat du, dv;
    GLint i, j;
    __GLcolor currentColor;
    __GLcoord currentNormal, currentTexture;
    GLfloat u1;
    GLint row = 0;
    StateChange changes[MAX_WIDTH];

    if ((u->n == 0) || (v->n == 0))
        return;
    du = (u->finish - u->start)/(GLfloat)u->n;
    dv = (v->finish - v->start)/(GLfloat)v->n;

    currentColor = gc->state.current.color;
    currentNormal = gc->state.current.normal;
    currentTexture = __GL_ACTIVE_TEXCOORD(gc);

    for (i = lowU; i < highU; i++)
    {
        GLfloat u2 = ((i+1) == u->n) ? u->finish : (u->start + (i+1) * du);
        u1 = (i == u->n) ? u->finish : (u->start + i * du);

        row=0;
        for (j = lowV; j <= highV; j++)
        {
            GLfloat v1 = (j == v->n) ? v->finish : (v->start + j * dv);
            GLfloat v2 = ((j+1) == v->n) ? v->finish : (v->start + (j+1)*dv);

            (*gc->immedModeDispatch.Begin)(gc, GL_LINE_STRIP);
            if (j != highV)
            {
                if (row < MAX_WIDTH-1)
                {
                    if (i != lowU)
                    {
                        sendChange(gc, changes+row+1);
                    }
                    else
                    {
                        DoEvalCoord2(gc, u1, v2, changes+row+1);
                    }
                }
                else
                {
                    DoEvalCoord2(gc, u1, v2, NULL);
                }
            }
            if (row < MAX_WIDTH)
            {
                if (i != lowU && j != lowV)
                {
                    sendChange(gc, changes+row);
                }
                else
                {
                    DoEvalCoord2(gc, u1, v1, NULL);
                }
                DoEvalCoord2(gc, u2, v1, changes+row);
            }
            else
            {
                DoEvalCoord2(gc, u1, v1, NULL);
                DoEvalCoord2(gc, u2, v1, NULL);
            }
            (*gc->immedModeDispatch.End)(gc);
            row++;
        }
    }
    /* Now we need to do the i==highU iteration.  We will do it backwards
    ** (from highV to lowV) so the outline is drawn in the same direction
    ** as the other mesh lines.
    */
    row--;
    u1 = (i == u->n) ? u->finish : (u->start + i * du);
    (*gc->immedModeDispatch.Begin)(gc, GL_LINE_STRIP);
    for (j = highV; j >= lowV; j--)
    {
        GLfloat v1 = (j == v->n) ? v->finish : (v->start + j * dv);

        if (row >= 0 && row < MAX_WIDTH)
        {
            sendChange(gc, changes+row);
        }
        else
        {
            DoEvalCoord2(gc, u1, v1, NULL);
        }

        row--;
    }
    (*gc->immedModeDispatch.End)(gc);

    gc->state.current.color = currentColor;
    gc->state.current.normal = currentNormal;
    __GL_ACTIVE_TEXCOORD(gc) = currentTexture;
}


/*
** initialize the evaluator state
*/
#define MaxK 4
#define MaxOrder 40/*XXX*/

GLint __glEvalComputeK(GLenum target)
{
    switch (target)
    {
    case GL_MAP1_VERTEX_4:
    case GL_MAP1_COLOR_4:
    case GL_MAP1_TEXTURE_COORD_4:
    case GL_MAP2_VERTEX_4:
    case GL_MAP2_COLOR_4:
    case GL_MAP2_TEXTURE_COORD_4:
        return (4);
    case GL_MAP1_VERTEX_3:
    case GL_MAP1_TEXTURE_COORD_3:
    case GL_MAP1_NORMAL:
    case GL_MAP2_VERTEX_3:
    case GL_MAP2_TEXTURE_COORD_3:
    case GL_MAP2_NORMAL:
        return (3);
    case GL_MAP1_TEXTURE_COORD_2:
    case GL_MAP2_TEXTURE_COORD_2:
        return (2);
    case GL_MAP1_TEXTURE_COORD_1:
    case GL_MAP2_TEXTURE_COORD_1:
    case GL_MAP1_INDEX:
    case GL_MAP2_INDEX:
        return (1);
    default:
        return (-1);
    }
}

#if defined(_WIN32)
#pragma warning(disable: 4244)
#endif
GLvoid __glFreeEvaluatorState(__GLcontext *gc)
{
    GLint i;
    __GLevaluatorMachine *evals = &gc->eval;

    for (i = 0; i < __GL_MAP_RANGE_COUNT; i++)
    {
        if (evals->eval1Data[i])
        {
            (*gc->imports.free)(gc, evals->eval1Data[i]);
            evals->eval1Data[i] = 0;
        }
        if (evals->eval2Data[i])
        {
            (*gc->imports.free)(gc, evals->eval2Data[i]);
            evals->eval2Data[i] = 0;
        }
    }
}
/*
** define a one dimensional map
*/
__GLevaluator1 *__glSetUpMap1(__GLcontext *gc, GLenum type,
                              GLint order, GLfloat u1, GLfloat u2)
{
    __GLevaluator1 *ev;
    GLfloat **evData;

    switch (type)
    {
    case GL_MAP1_COLOR_4:
    case GL_MAP1_INDEX:
    case GL_MAP1_NORMAL:
    case GL_MAP1_TEXTURE_COORD_1:
    case GL_MAP1_TEXTURE_COORD_2:
    case GL_MAP1_TEXTURE_COORD_3:
    case GL_MAP1_TEXTURE_COORD_4:
    case GL_MAP1_VERTEX_3:
    case GL_MAP1_VERTEX_4:
        ev = &gc->eval.eval1[__GL_EVAL1D_INDEX(type)];
        evData = &gc->eval.eval1Data[__GL_EVAL1D_INDEX(type)];
        break;
    default:
        __glSetError(gc ,GL_INVALID_ENUM);
        return (0);
    }
    if (u1 == u2 || order < 1 || order > (GLint)gc->constants.maxEvalOrder)
    {
        __glSetError(gc ,GL_INVALID_VALUE);
        return (0);
    }
    ev->order = order;
    ev->u1 = u1;
    ev->u2 = u2;
    *evData = (GLfloat *)
        (*gc->imports.realloc)(gc, *evData,
        (size_t) (__glMap1_size(ev->k, order) * sizeof(GLfloat)) );

    return (ev);
}

/*
** define a two dimensional map
*/
__GLevaluator2 *__glSetUpMap2(__GLcontext *gc, GLenum type,
                              GLint majorOrder, GLint minorOrder,
                              GLfloat u1, GLfloat u2,
                              GLfloat v1, GLfloat v2)
{
    __GLevaluator2 *ev;
    GLfloat **evData;

    switch (type)
    {
    case GL_MAP2_COLOR_4:
    case GL_MAP2_INDEX:
    case GL_MAP2_NORMAL:
    case GL_MAP2_TEXTURE_COORD_1:
    case GL_MAP2_TEXTURE_COORD_2:
    case GL_MAP2_TEXTURE_COORD_3:
    case GL_MAP2_TEXTURE_COORD_4:
    case GL_MAP2_VERTEX_3:
    case GL_MAP2_VERTEX_4:
        ev = &gc->eval.eval2[__GL_EVAL2D_INDEX(type)];
        evData = &gc->eval.eval2Data[__GL_EVAL2D_INDEX(type)];
        break;
    default:
        __glSetError(gc ,GL_INVALID_ENUM);
        return (0);
    }
    if (minorOrder < 1 || minorOrder > (GLint)gc->constants.maxEvalOrder ||
        majorOrder < 1 || majorOrder > (GLint)gc->constants.maxEvalOrder ||
        u1 == u2 || v1 == v2)
    {
        __glSetError(gc ,GL_INVALID_VALUE);
        return (0);
    }
    ev->majorOrder = majorOrder;
    ev->minorOrder = minorOrder;
    ev->u1 = u1;
    ev->u2 = u2;
    ev->v1 = v1;
    ev->v2 = v2;
    *evData = (GLfloat *)
        (*gc->imports.realloc)(gc, *evData,
        (size_t) (__glMap2_size(ev->k, majorOrder, minorOrder)
        * sizeof(GLfloat)) );

    return (ev);
}


GLvoid APIENTRY __glim_Map1d(__GLcontext *gc, GLenum type, GLdouble u1, GLdouble u2,
                           GLint stride, GLint order, const GLdouble *points)
{
    __GLevaluator1 *ev;
    GLfloat *data;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    ev = __glSetUpMap1(gc, type, order, u1, u2);
    if (ev == 0)
    {
        return;
    }
    if (stride < ev->k)
    {
        __glSetError(gc ,GL_INVALID_VALUE);
        return;
    }

    data = gc->eval.eval1Data[__GL_EVAL1D_INDEX(type)];
    __glFillMap1dInternal(ev->k, order, stride, points, data);
}

GLvoid APIENTRY __glim_Map1f(__GLcontext *gc, GLenum type, GLfloat u1, GLfloat u2,
                           GLint stride, GLint order, const GLfloat *points)
{
    __GLevaluator1 *ev;
    GLfloat *data;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    ev = __glSetUpMap1(gc, type, order, u1, u2);
    if (ev == 0)
    {
        return;
    }
    if (stride < ev->k)
    {
        __glSetError(gc ,GL_INVALID_VALUE);
        return;
    }

    data = gc->eval.eval1Data[__GL_EVAL1D_INDEX(type)];
    __glFillMap1fInternal(ev->k, order, stride, points, data);
}

GLvoid APIENTRY __glim_Map2d(__GLcontext *gc, GLenum type,
                           GLdouble u1, GLdouble u2,
                           GLint uStride, GLint uOrder,
                           GLdouble v1, GLdouble v2,
                           GLint vStride, GLint vOrder,
                           const GLdouble *points)
{
    __GLevaluator2 *ev;
    GLfloat *data;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    ev = __glSetUpMap2(gc, type, uOrder, vOrder, u1, u2, v1, v2);
    if (ev == 0)
    {
        return;
    }
    if (uStride < ev->k)
    {
        __glSetError(gc ,GL_INVALID_VALUE);
        return;
    }
    if (vStride < ev->k)
    {
        __glSetError(gc ,GL_INVALID_VALUE);
        return;
    }

    data = gc->eval.eval2Data[__GL_EVAL2D_INDEX(type)];
    __glFillMap2dInternal(ev->k, uOrder, vOrder, uStride, vStride,
        points, data);
}

GLvoid APIENTRY __glim_Map2f(__GLcontext *gc, GLenum type,
                           GLfloat u1, GLfloat u2,
                           GLint uStride, GLint uOrder,
                           GLfloat v1, GLfloat v2,
                           GLint vStride, GLint vOrder,
                           const GLfloat *points)
{
    __GLevaluator2 *ev;
    GLfloat *data;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    ev = __glSetUpMap2(gc, type, uOrder, vOrder, u1, u2, v1, v2);
    if (ev == 0)
    {
        return;
    }
    if (uStride < ev->k)
    {
        __glSetError(gc ,GL_INVALID_VALUE);
        return;
    }
    if (vStride < ev->k)
    {
        __glSetError(gc ,GL_INVALID_VALUE);
        return;
    }

    data = gc->eval.eval2Data[__GL_EVAL2D_INDEX(type)];
    __glFillMap2fInternal(ev->k, uOrder, vOrder, uStride, vStride,
        points, data);
}

GLvoid APIENTRY __glim_EvalCoord1d(__GLcontext *gc, GLdouble u)
{
    __glDoEvalCoord1(gc, u);
}

GLvoid APIENTRY __glim_EvalCoord1dv(__GLcontext *gc, const GLdouble u[1])
{
    __glDoEvalCoord1(gc, u[0]);
}

GLvoid APIENTRY __glim_EvalCoord1f(__GLcontext *gc, GLfloat u)
{
    __glDoEvalCoord1(gc, u);
}

GLvoid APIENTRY __glim_EvalCoord1fv(__GLcontext *gc, const GLfloat u[1])
{
    __glDoEvalCoord1(gc, u[0]);
}

GLvoid APIENTRY __glim_EvalCoord2d(__GLcontext *gc, GLdouble u, GLdouble v)
{
    __glDoEvalCoord2(gc, u, v);
}

GLvoid APIENTRY __glim_EvalCoord2dv(__GLcontext *gc, const GLdouble u[2])
{
    __glDoEvalCoord2(gc, u[0], u[1]);
}

GLvoid APIENTRY __glim_EvalCoord2f(__GLcontext *gc, GLfloat u, GLfloat v)
{
    __glDoEvalCoord2(gc, u, v);
}

GLvoid APIENTRY __glim_EvalCoord2fv(__GLcontext *gc, const GLfloat u[2])
{
    __glDoEvalCoord2(gc, u[0], u[1]);
}

GLvoid APIENTRY __glim_MapGrid1d(__GLcontext *gc, GLint nu, GLdouble u0, GLdouble u1)
{
    gc->state.evaluator.u1.start = (GLfloat)u0;
    gc->state.evaluator.u1.finish = (GLfloat)u1;
    gc->state.evaluator.u1.n = nu;
}

GLvoid APIENTRY __glim_MapGrid1f(__GLcontext *gc, GLint nu, GLfloat u0, GLfloat u1)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    gc->state.evaluator.u1.start = (GLfloat)u0;
    gc->state.evaluator.u1.finish = (GLfloat)u1;
    gc->state.evaluator.u1.n = nu;
}

GLvoid APIENTRY __glim_MapGrid2d(__GLcontext *gc, GLint nu, GLdouble u0, GLdouble u1,
                               GLint nv, GLdouble v0, GLdouble v1)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    gc->state.evaluator.u2.start = (GLfloat)u0;
    gc->state.evaluator.u2.finish = (GLfloat)u1;
    gc->state.evaluator.u2.n = nu;
    gc->state.evaluator.v2.start = (GLfloat)v0;
    gc->state.evaluator.v2.finish = (GLfloat)v1;
    gc->state.evaluator.v2.n = nv;
}

GLvoid APIENTRY __glim_MapGrid2f(__GLcontext *gc, GLint nu, GLfloat u0, GLfloat u1,
                               GLint nv, GLfloat v0, GLfloat v1)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    gc->state.evaluator.u2.start = (GLfloat)u0;
    gc->state.evaluator.u2.finish = (GLfloat)u1;
    gc->state.evaluator.u2.n = nu;
    gc->state.evaluator.v2.start = (GLfloat)v0;
    gc->state.evaluator.v2.finish = (GLfloat)v1;
    gc->state.evaluator.v2.n = nv;
}

GLvoid APIENTRY __glim_EvalMesh1(__GLcontext *gc, GLenum mode, GLint low, GLint high)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    if (gc->conditionalRenderDiscard)
    {
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    switch (mode)
    {
    case GL_LINE:
        __glEvalMesh1Line(gc, low, high);
        break;
    case GL_POINT:
        __glEvalMesh1Point(gc, low, high);
        break;
    default:
        __glSetError(gc ,GL_INVALID_ENUM);
        return;
    }
}

GLvoid APIENTRY __glim_EvalPoint1(__GLcontext *gc, GLint i)
{
    __GLevaluatorGrid *u;
    GLfloat du;

    u = &gc->state.evaluator.u1;
    du = (u->finish - u->start)/(GLfloat)u->n;/*XXX cache? */
    __glDoEvalCoord1(gc, (i == u->n) ? u->finish : (u->start + i * du) );
}

GLvoid APIENTRY __glim_EvalMesh2(__GLcontext *gc, GLenum mode,
                               GLint lowU, GLint highU,
                               GLint lowV, GLint highV)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    if (gc->conditionalRenderDiscard)
    {
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    switch (mode)
    {
    case GL_FILL:
        __glEvalMesh2Fill(gc, lowU, lowV, highU, highV);
        break;
    case GL_LINE:
        __glEvalMesh2Line(gc, lowU, lowV, highU, highV);
        break;
    case GL_POINT:
        __glEvalMesh2Point(gc, lowU, lowV, highU, highV);
        break;
    default:
        __glSetError(gc ,GL_INVALID_ENUM);
        return;
    }
}

GLvoid APIENTRY __glim_EvalPoint2(__GLcontext *gc, GLint i, GLint j)
{
    __GLevaluatorGrid *u;
    __GLevaluatorGrid *v;
    GLfloat du;
    GLfloat dv;

    u = &gc->state.evaluator.u2;
    v = &gc->state.evaluator.v2;
    du = (u->finish - u->start)/(GLfloat)u->n;/*XXX cache? */
    dv = (v->finish - v->start)/(GLfloat)v->n;/*XXX cache? */
    __glDoEvalCoord2(gc, (i == u->n) ? u->finish : (u->start + i * du),
        (j == v->n) ? v->finish : (v->start + j * dv));
}


GLvoid APIENTRY __glim_GetMapfv(__GLcontext *gc, GLenum target, GLenum query, GLfloat buf[])
{
    __GLevaluator1 *eval1;
    __GLevaluator2 *eval2;
    __GLfloat *eval1Data, *eval2Data;
    GLfloat *rp;
    GLint index, i, t;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    /*
    ** Check if target is valid.
    */
    rp = buf;
    switch (target)
    {
    case GL_MAP1_COLOR_4:
    case GL_MAP1_INDEX:
    case GL_MAP1_NORMAL:
    case GL_MAP1_TEXTURE_COORD_1:
    case GL_MAP1_TEXTURE_COORD_2:
    case GL_MAP1_TEXTURE_COORD_3:
    case GL_MAP1_TEXTURE_COORD_4:
    case GL_MAP1_VERTEX_3:
    case GL_MAP1_VERTEX_4:
        index = __GL_EVAL1D_INDEX(target);
        eval1 = &gc->eval.eval1[index];
        switch (query)
        {
        case GL_COEFF:
            t = eval1->order * eval1->k;
            eval1Data = gc->eval.eval1Data[index];
            for (i = 0; i < t; i++)
            {
                *rp++ = eval1Data[i];
            }
            break;
        case GL_DOMAIN:
            *rp++ = eval1->u1;
            *rp++ = eval1->u2;
            break;
        case GL_ORDER:
            *rp++ = gc->eval.eval1[index].order;
            break;
        default:
            __glSetError(gc ,GL_INVALID_ENUM);
            return;
        }
        break;
    case GL_MAP2_COLOR_4:
    case GL_MAP2_INDEX:
    case GL_MAP2_NORMAL:
    case GL_MAP2_TEXTURE_COORD_1:
    case GL_MAP2_TEXTURE_COORD_2:
    case GL_MAP2_TEXTURE_COORD_3:
    case GL_MAP2_TEXTURE_COORD_4:
    case GL_MAP2_VERTEX_3:
    case GL_MAP2_VERTEX_4:
        index = __GL_EVAL2D_INDEX(target);
        eval2 = &gc->eval.eval2[index];
        switch (query)
        {
        case GL_COEFF:
            eval2Data = gc->eval.eval2Data[index];
            t = eval2->majorOrder * eval2->minorOrder * eval2->k;
            for (i = 0; i < t; i++)
            {
                *rp++ = eval2Data[i];
            }
            break;
        case GL_DOMAIN:
            *rp++ = eval2->u1;
            *rp++ = eval2->u2;
            *rp++ = eval2->v1;
            *rp++ = eval2->v2;
            break;
        case GL_ORDER:
            *rp++ = gc->eval.eval2[index].majorOrder;
            *rp++ = gc->eval.eval2[index].minorOrder;
            break;
        default:
            __glSetError(gc ,GL_INVALID_ENUM);
            return;
        }
        break;
    default:
        __glSetError(gc ,GL_INVALID_ENUM);
        return;
    }
}

GLvoid APIENTRY __glim_GetMapdv(__GLcontext *gc, GLenum target, GLenum query, GLdouble buf[])
{
    __GLevaluator1 *eval1;
    __GLevaluator2 *eval2;
    __GLfloat *eval1Data, *eval2Data;
    GLdouble *rp;
    GLint index, i, t;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    /*
    ** Check if target is valid.
    */
    rp = buf;
    switch (target)
    {
    case GL_MAP1_COLOR_4:
    case GL_MAP1_INDEX:
    case GL_MAP1_NORMAL:
    case GL_MAP1_TEXTURE_COORD_1:
    case GL_MAP1_TEXTURE_COORD_2:
    case GL_MAP1_TEXTURE_COORD_3:
    case GL_MAP1_TEXTURE_COORD_4:
    case GL_MAP1_VERTEX_3:
    case GL_MAP1_VERTEX_4:
        index = __GL_EVAL1D_INDEX(target);
        eval1 = &gc->eval.eval1[index];
        switch (query)
        {
        case GL_COEFF:
            eval1Data = gc->eval.eval1Data[index];
            t = eval1->order * eval1->k;
            for (i = 0; i < t; i++)
            {
                *rp++ = eval1Data[i];
            }
            break;
        case GL_DOMAIN:
            *rp++ = eval1->u1;
            *rp++ = eval1->u2;
            break;
        case GL_ORDER:
            *rp++ = gc->eval.eval1[index].order;
            break;
        default:
            __glSetError(gc ,GL_INVALID_ENUM);
            return;
        }
        break;
    case GL_MAP2_COLOR_4:
    case GL_MAP2_INDEX:
    case GL_MAP2_NORMAL:
    case GL_MAP2_TEXTURE_COORD_1:
    case GL_MAP2_TEXTURE_COORD_2:
    case GL_MAP2_TEXTURE_COORD_3:
    case GL_MAP2_TEXTURE_COORD_4:
    case GL_MAP2_VERTEX_3:
    case GL_MAP2_VERTEX_4:
        index = __GL_EVAL2D_INDEX(target);
        eval2 = &gc->eval.eval2[index];
        switch (query)
        {
        case GL_COEFF:
            eval2Data = gc->eval.eval2Data[index];
            t = eval2->majorOrder * eval2->minorOrder * eval2->k;
            for (i = 0; i < t; i++)
            {
                *rp++ = eval2Data[i];
            }
            break;
        case GL_DOMAIN:
            *rp++ = eval2->u1;
            *rp++ = eval2->u2;
            *rp++ = eval2->v1;
            *rp++ = eval2->v2;
            break;
        case GL_ORDER:
            *rp++ = gc->eval.eval2[index].majorOrder;
            *rp++ = gc->eval.eval2[index].minorOrder;
            break;
        default:
            __glSetError(gc ,GL_INVALID_ENUM);
            return;
        }
        break;
    default:
        __glSetError(gc ,GL_INVALID_ENUM);
        return;
    }
}

GLvoid APIENTRY __glim_GetMapiv(__GLcontext *gc, GLenum target, GLenum query, GLint buf[])
{
    __GLevaluator1 *eval1;
    __GLevaluator2 *eval2;
    __GLfloat *eval1Data, *eval2Data;
    GLint *rp;
    GLint index, t;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    /*
    ** Check if target is valid.
    */
    rp = buf;
    switch (target)
    {
    case GL_MAP1_COLOR_4:
    case GL_MAP1_INDEX:
    case GL_MAP1_NORMAL:
    case GL_MAP1_TEXTURE_COORD_1:
    case GL_MAP1_TEXTURE_COORD_2:
    case GL_MAP1_TEXTURE_COORD_3:
    case GL_MAP1_TEXTURE_COORD_4:
    case GL_MAP1_VERTEX_3:
    case GL_MAP1_VERTEX_4:
        index = __GL_EVAL1D_INDEX(target);
        eval1 = &gc->eval.eval1[index];
        switch (query)
        {
        case GL_COEFF:
            eval1Data = gc->eval.eval1Data[index];
            t = eval1->order * eval1->k;
            __glConvertResult(gc, __GL_FLOAT, eval1Data,
                __GL_INT32, rp, t);
            break;
        case GL_DOMAIN:
            __glConvertResult(gc, __GL_FLOAT, &eval1->u1,
                __GL_INT32, rp, 2);
            break;
        case GL_ORDER:
            *rp++ = gc->eval.eval1[index].order;
            break;
        default:
            __glSetError(gc ,GL_INVALID_ENUM);
            return;
        }
        break;
    case GL_MAP2_COLOR_4:
    case GL_MAP2_INDEX:
    case GL_MAP2_NORMAL:
    case GL_MAP2_TEXTURE_COORD_1:
    case GL_MAP2_TEXTURE_COORD_2:
    case GL_MAP2_TEXTURE_COORD_3:
    case GL_MAP2_TEXTURE_COORD_4:
    case GL_MAP2_VERTEX_3:
    case GL_MAP2_VERTEX_4:
        index = __GL_EVAL2D_INDEX(target);
        eval2 = &gc->eval.eval2[index];
        switch (query)
        {
        case GL_COEFF:
            eval2Data = gc->eval.eval2Data[index];
            t = eval2->majorOrder * eval2->minorOrder * eval2->k;
            __glConvertResult(gc, __GL_FLOAT, eval2Data,
                __GL_INT32, rp, t);
            break;
        case GL_DOMAIN:
            __glConvertResult(gc, __GL_FLOAT, &eval2->u1,
                __GL_INT32, rp, 4);
            break;
        case GL_ORDER:
            *rp++ = gc->eval.eval2[index].majorOrder;
            *rp++ = gc->eval.eval2[index].minorOrder;
            break;
        default:
            __glSetError(gc ,GL_INVALID_ENUM);
            return;
        }
        break;
    default:
        __glSetError(gc ,GL_INVALID_ENUM);
        return;
    }
}

