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

#define __GL_NOMALIZE_SQRT  1

#define __GL_MATRIX_FLOAT_SMALL_PRECISION     ( 3.0e-07 )
#define __GL_MATRIX_FLOAT_BIG_PRECISION     ( 3.0e07 )

GLvoid __glInvertTransposePoorMatrix(__GLmatrix *inverse, const __GLmatrix *srcRaw);
/*
** Normalize v into vout.
*/
GLvoid __glNormalize(GLfloat vout[3], const GLfloat v[3])
{
    GLfloat len, zero = __glZero;

    len = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
    if (len <= zero)
    {
        vout[0] = zero;
        vout[1] = zero;
        vout[2] = zero;
        return;
    }
    if (len == 1.0F)
    {
        vout[0] = v[0];
        vout[1] = v[1];
        vout[2] = v[2];
        return;
    }
    else
    {
#if __GL_NOMALIZE_SQRT
        union
        {
            GLuint i;
            GLfloat f;
        } seed;
        GLfloat xy, subexp;

        /*
        ** This code calculates a reciprocal square root accurate to well over
        ** 16 bits using Newton-Raphson approximation.
        **
        ** To calculate the seed, the shift compresses the floating-point
        ** range just as sqrt() does, and the subtract inverts the range
        ** like reciprocation does.  The constant was chosen by trial-and-error
        ** to minimize the maximum error of the iterated result for all values
        ** over the range .5 to 2.
        */
        seed.f = len;
        seed.i = 0x5f375a00u - (seed.i >> 1);

        /*
        ** The Newton-Raphson iteration to approximate X = 1/sqrt(Y) is:
        **
        **  X[1] = .5*X[0]*(3 - Y*X[0]^2)
        **
        ** A double iteration is:
        **
        **  X[2] = .0625*X[0]*(3 - Y*X[0]^2)*[12 - (Y*X[0]^2)*(3 - Y*X[0]^2)^2]
        **
        */
        xy = len * seed.f * seed.f;
        subexp = 3.f - xy;
        len = .0625f * seed.f * subexp * (12.f - xy * subexp * subexp);
#else
        len = ((GLfloat) 1.0) / __GL_SQRTF(len);
#endif
        vout[0] = v[0] * len;
        vout[1] = v[1] * len;
        vout[2] = v[2] * len;
        return;
    }
}

/*
** Normalize v into vout.
**   voa = normalize(v)
**   vob = normalize(voa + (0 0 1))
*/
GLvoid __glNormalizeTwo(const GLfloat v[3], GLfloat voa[3], GLfloat vob[3])
{
    GLfloat len, voaZ;

    len = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
    if (len <= __glZero)
    {
        voa[0] = __glZero;
        voa[1] = __glZero;
        voa[2] = __glZero;
        vob[0] = __glZero;
        vob[1] = __glZero;
        vob[2] = 1.0F;
        return;
    }
    if (len == 1.0F)
    {
        voa[0] = v[0];
        voa[1] = v[1];
        voa[2] = v[2];
    }
    else
    {
#if __GL_NOMALIZE_SQRT
        union
        {
            GLuint i;
            GLfloat f;
        } seed;
        GLfloat xy, subexp;

        seed.f = len;
        seed.i = 0x5f375a00u - (seed.i >> 1);

        xy = len * seed.f * seed.f;
        subexp = 3.f - xy;
        len = .0625f * seed.f * subexp * (12.f - xy * subexp * subexp);
#else
        len = ((GLfloat) 1.0) / __GL_SQRTF(len);
#endif
        voa[0] = v[0] * len;
        voa[1] = v[1] * len;
        voa[2] = v[2] * len;
    }

    voaZ = voa[2] + 1.0F;
    len = 2.0f * voaZ;
    if (len <= __glZero)
    {
        vob[0] = __glZero;
        vob[1] = __glZero;
        vob[2] = __glZero;
        return;
    }
    if (len == 1.0F)
    {
        vob[0] = voa[0];
        vob[1] = voa[1];
        vob[2] = voaZ;
    }
    else
    {
#if __GL_NOMALIZE_SQRT
        union
        {
            GLuint i;
            GLfloat f;
        } seed;
        GLfloat xy, subexp;

        seed.f = len;
        seed.i = 0x5f375a00u - (seed.i >> 1);

        xy = len * seed.f * seed.f;
        subexp = 3.f - xy;
        len = .0625f * seed.f * subexp * (12.f - xy * subexp * subexp);
#else
        len = ((GLfloat) 1.0) / __GL_SQRTF(len);
#endif
        vob[0] = voa[0] * len;
        vob[1] = voa[1] * len;
        vob[2] = voaZ * len;
    }
}

/*
** Copy src to dst
*/
GLvoid __glCopyMatrix(__GLmatrix *dst, const __GLmatrix *src)
{
    dst->matrixType = src->matrixType;
    dst->matrix[0][0] = src->matrix[0][0];
    dst->matrix[0][1] = src->matrix[0][1];
    dst->matrix[0][2] = src->matrix[0][2];
    dst->matrix[0][3] = src->matrix[0][3];

    dst->matrix[1][0] = src->matrix[1][0];
    dst->matrix[1][1] = src->matrix[1][1];
    dst->matrix[1][2] = src->matrix[1][2];
    dst->matrix[1][3] = src->matrix[1][3];

    dst->matrix[2][0] = src->matrix[2][0];
    dst->matrix[2][1] = src->matrix[2][1];
    dst->matrix[2][2] = src->matrix[2][2];
    dst->matrix[2][3] = src->matrix[2][3];

    dst->matrix[3][0] = src->matrix[3][0];
    dst->matrix[3][1] = src->matrix[3][1];
    dst->matrix[3][2] = src->matrix[3][2];
    dst->matrix[3][3] = src->matrix[3][3];
}

/*
** Make m an identity matrix
*/
GLvoid __glMakeIdentity(__GLmatrix *m)
{
    GLfloat zer = __glZero;
    GLfloat one = ((GLfloat) 1.0);
    m->matrix[0][0] = one; m->matrix[0][1] = zer;
    m->matrix[0][2] = zer; m->matrix[0][3] = zer;
    m->matrix[1][0] = zer; m->matrix[1][1] = one;
    m->matrix[1][2] = zer; m->matrix[1][3] = zer;
    m->matrix[2][0] = zer; m->matrix[2][1] = zer;
    m->matrix[2][2] = one; m->matrix[2][3] = zer;
    m->matrix[3][0] = zer; m->matrix[3][1] = zer;
    m->matrix[3][2] = zer; m->matrix[3][3] = one;
    m->matrixType = __GL_MT_IDENTITY;
}

/*
** Compute r = a * b, where r can equal b.
*/
GLvoid __glMultMatrix(__GLmatrix *r, const __GLmatrix *a, const __GLmatrix *b)
{
    __GLfloat b00, b01, b02, b03;
    __GLfloat b10, b11, b12, b13;
    __GLfloat b20, b21, b22, b23;
    __GLfloat b30, b31, b32, b33;
    GLint i;

    b00 = b->matrix[0][0];
    b01 = b->matrix[0][1];
    b02 = b->matrix[0][2];
    b03 = b->matrix[0][3];
    b10 = b->matrix[1][0];
    b11 = b->matrix[1][1];
    b12 = b->matrix[1][2];
    b13 = b->matrix[1][3];
    b20 = b->matrix[2][0];
    b21 = b->matrix[2][1];
    b22 = b->matrix[2][2];
    b23 = b->matrix[2][3];
    b30 = b->matrix[3][0];
    b31 = b->matrix[3][1];
    b32 = b->matrix[3][2];
    b33 = b->matrix[3][3];

    for (i = 0; i < 4; i++)
    {
        r->matrix[i][0] = a->matrix[i][0]*b00 +
                          a->matrix[i][1]*b10 +
                          a->matrix[i][2]*b20 +
                          a->matrix[i][3]*b30;
        r->matrix[i][1] = a->matrix[i][0]*b01 +
                          a->matrix[i][1]*b11 +
                          a->matrix[i][2]*b21 +
                          a->matrix[i][3]*b31;
        r->matrix[i][2] = a->matrix[i][0]*b02 +
                          a->matrix[i][1]*b12 +
                          a->matrix[i][2]*b22 +
                          a->matrix[i][3]*b32;
        r->matrix[i][3] = a->matrix[i][0]*b03 +
                          a->matrix[i][1]*b13 +
                          a->matrix[i][2]*b23 +
                          a->matrix[i][3]*b33;
    }
}

GLvoid __glCheckMatrixPricesion(__GLmatrix *dest, const __GLmatrix *src)
{
    GLint iRow, iCol;
    GLdouble fTemp;
    GL_ASSERT( dest );
    GL_ASSERT( src );
    memcpy( dest, src, sizeof(__GLmatrix) );
    for ( iRow = 0; iRow < 4; iRow++ )
    {
        for ( iCol = 0; iCol < 4; iCol++ )
        {
            fTemp = dest->matrix[iRow][iCol];
            if ( -__GL_MATRIX_FLOAT_SMALL_PRECISION < fTemp && fTemp < 0)
            {
                dest->matrix[iRow][iCol] = (GLfloat)-__GL_MATRIX_FLOAT_SMALL_PRECISION;
            }
            else if ( 0 < fTemp && fTemp < __GL_MATRIX_FLOAT_SMALL_PRECISION )
            {
                dest->matrix[iRow][iCol] = (GLfloat)__GL_MATRIX_FLOAT_SMALL_PRECISION;
            }
        }
    }
}

/*
** inverse = invert(transpose(src))

This code uses Cramer's Rule to calculate the matrix inverse.
In general, the inverse transpose has this form:

[          ] -t    [                                   ]
[          ]       [             -t             -t t   ]
[  Q    P  ]       [   S(SQ - PT)     -(SQ - PT)  T    ]
[          ]       [                                   ]
[          ]       [                                   ]
[          ]    =  [                                   ]
[          ]       [        -1  t                      ]
[          ]       [     -(Q  P)             1         ]
[  T    S  ]       [   -------------   -------------   ]
[          ]       [         -1  t t         -1  t t   ]
[          ]       [   S - (Q  P) T    S - (Q  P) T    ]

But in the usual case that P,S == [0, 0, 0, 1], this is enough:

[          ] -t    [                                   ]
[          ]       [         -t              -t t      ]
[  Q    0  ]       [        Q              -Q  T       ]
[          ]       [                                   ]
[          ]       [                                   ]
[          ]    =  [                                   ]
[          ]       [                                   ]
[          ]       [                                   ]
[  T    1  ]       [        0                1         ]
[          ]       [                                   ]
[          ]       [                                   ]

*/
GLvoid __glInvertTransposeMatrix(__GLmatrix *inverse, const __GLmatrix *src)
{
    GLfloat x00, x01, x02;
    GLfloat x10, x11, x12;
    GLfloat x20, x21, x22;
    GLfloat rcp;

    /* propagate matrix type & branch if general */
    if (inverse->matrixType == src->matrixType)
    {
        GLfloat z00, z01, z02;
        GLfloat z10, z11, z12;
        GLfloat z20, z21, z22;

        /* read 3x3 matrix into registers */
        x00 = src->matrix[0][0];
        x01 = src->matrix[0][1];
        x02 = src->matrix[0][2];
        x10 = src->matrix[1][0];
        x11 = src->matrix[1][1];
        x12 = src->matrix[1][2];
        x20 = src->matrix[2][0];
        x21 = src->matrix[2][1];
        x22 = src->matrix[2][2];

        /* compute first three 2x2 cofactors */
        z20 = x01*x12 - x11*x02;
        z10 = x21*x02 - x01*x22;
        z00 = x11*x22 - x12*x21;

        /* compute 3x3 determinant & its reciprocal */
        rcp = x20*z20 + x10*z10 + x00*z00;
        if (rcp == (GLfloat)0)
        {
            return;
        }
        else if ( -__GL_MATRIX_FLOAT_SMALL_PRECISION < rcp && rcp < __GL_MATRIX_FLOAT_SMALL_PRECISION  )
        {
            //it's a poor matrix, we should treat it special
            __glInvertTransposePoorMatrix( inverse, src);
            return ;
        }

        rcp = (GLfloat)1/rcp;

        /* compute other six 2x2 cofactors */
        z01 = x20*x12 - x10*x22;
        z02 = x10*x21 - x20*x11;
        z11 = x00*x22 - x20*x02;
        z12 = x20*x01 - x00*x21;
        z21 = x10*x02 - x00*x12;
        z22 = x00*x11 - x10*x01;

        /* multiply all cofactors by reciprocal */
        inverse->matrix[0][0] = z00*rcp;
        inverse->matrix[0][1] = z01*rcp;
        inverse->matrix[0][2] = z02*rcp;
        inverse->matrix[1][0] = z10*rcp;
        inverse->matrix[1][1] = z11*rcp;
        inverse->matrix[1][2] = z12*rcp;
        inverse->matrix[2][0] = z20*rcp;
        inverse->matrix[2][1] = z21*rcp;
        inverse->matrix[2][2] = z22*rcp;

        /* read translation vector & negate */
        x00 = -src->matrix[3][0];
        x01 = -src->matrix[3][1];
        x02 = -src->matrix[3][2];

        /* store bottom row of inverse transpose */
        inverse->matrix[3][0] = 0;
        inverse->matrix[3][1] = 0;
        inverse->matrix[3][2] = 0;
        inverse->matrix[3][3] = 1;

        /* finish by tranforming translation vector */
        inverse->matrix[0][3] = inverse->matrix[0][0]*x00 +
                                inverse->matrix[0][1]*x01 +
                                inverse->matrix[0][2]*x02;
        inverse->matrix[1][3] = inverse->matrix[1][0]*x00 +
                                inverse->matrix[1][1]*x01 +
                                inverse->matrix[1][2]*x02;
        inverse->matrix[2][3] = inverse->matrix[2][0]*x00 +
                                inverse->matrix[2][1]*x01 +
                                inverse->matrix[2][2]*x02;
    }
    else
    {
        GLfloat x30, x31, x32;
        GLfloat y01, y02, y03, y12, y13, y23;
        GLfloat z02, z03, z12, z13, z22, z23, z32, z33;

#define x03 x01
#define x13 x11
#define x23 x21
#define x33 x31
#define z00 x02
#define z10 x12
#define z20 x22
#define z30 x32
#define z01 x03
#define z11 x13
#define z21 x23
#define z31 x33


        /* read 1st two columns of matrix into registers */
        x00 = src->matrix[0][0];
        x01 = src->matrix[0][1];
        x10 = src->matrix[1][0];
        x11 = src->matrix[1][1];
        x20 = src->matrix[2][0];
        x21 = src->matrix[2][1];
        x30 = src->matrix[3][0];
        x31 = src->matrix[3][1];

        /* compute all six 2x2 determinants of 1st two columns */
        y01 = x00*x11 - x10*x01;
        y02 = x00*x21 - x20*x01;
        y03 = x00*x31 - x30*x01;
        y12 = x10*x21 - x20*x11;
        y13 = x10*x31 - x30*x11;
        y23 = x20*x31 - x30*x21;

        /* read 2nd two columns of matrix into registers */
        x02 = src->matrix[0][2];
        x03 = src->matrix[0][3];
        x12 = src->matrix[1][2];
        x13 = src->matrix[1][3];
        x22 = src->matrix[2][2];
        x23 = src->matrix[2][3];
        x32 = src->matrix[3][2];
        x33 = src->matrix[3][3];

        /* compute all 3x3 cofactors for 2nd two columns */
        z33 = x02*y12 - x12*y02 + x22*y01;
        z23 = x12*y03 - x32*y01 - x02*y13;
        z13 = x02*y23 - x22*y03 + x32*y02;
        z03 = x22*y13 - x32*y12 - x12*y23;
        z32 = x13*y02 - x23*y01 - x03*y12;
        z22 = x03*y13 - x13*y03 + x33*y01;
        z12 = x23*y03 - x33*y02 - x03*y23;
        z02 = x13*y23 - x23*y13 + x33*y12;

        /* compute all six 2x2 determinants of 2nd two columns */
        y01 = x02*x13 - x12*x03;
        y02 = x02*x23 - x22*x03;
        y03 = x02*x33 - x32*x03;
        y12 = x12*x23 - x22*x13;
        y13 = x12*x33 - x32*x13;
        y23 = x22*x33 - x32*x23;

        /* read 1st two columns of matrix into registers */
        x00 = src->matrix[0][0];
        x01 = src->matrix[0][1];
        x10 = src->matrix[1][0];
        x11 = src->matrix[1][1];
        x20 = src->matrix[2][0];
        x21 = src->matrix[2][1];
        x30 = src->matrix[3][0];
        x31 = src->matrix[3][1];

        /* compute all 3x3 cofactors for 1st column */
        z30 = x11*y02 - x21*y01 - x01*y12;
        z20 = x01*y13 - x11*y03 + x31*y01;
        z10 = x21*y03 - x31*y02 - x01*y23;
        z00 = x11*y23 - x21*y13 + x31*y12;

        /* compute 4x4 determinant & its reciprocal */
        rcp = x30*z30 + x20*z20 + x10*z10 + x00*z00;
        if (rcp == (GLfloat)0)
        {
            return ;
        }
        else if ( -__GL_MATRIX_FLOAT_SMALL_PRECISION < rcp && rcp < __GL_MATRIX_FLOAT_SMALL_PRECISION  )
        {
            //it's a poor matrix, we should treat it special
            __glInvertTransposePoorMatrix( inverse, src);
            return ;
        }

        rcp = (GLfloat)1/rcp;

        /* compute all 3x3 cofactors for 2nd column */
        z31 = x00*y12 - x10*y02 + x20*y01;
        z21 = x10*y03 - x30*y01 - x00*y13;
        z11 = x00*y23 - x20*y03 + x30*y02;
        z01 = x20*y13 - x30*y12 - x10*y23;

        /* multiply all 3x3 cofactors by reciprocal */
        inverse->matrix[0][0] = z00*rcp;
        inverse->matrix[0][1] = z01*rcp;
        inverse->matrix[1][0] = z10*rcp;
        inverse->matrix[0][2] = z02*rcp;
        inverse->matrix[2][0] = z20*rcp;
        inverse->matrix[0][3] = z03*rcp;
        inverse->matrix[3][0] = z30*rcp;
        inverse->matrix[1][1] = z11*rcp;
        inverse->matrix[1][2] = z12*rcp;
        inverse->matrix[2][1] = z21*rcp;
        inverse->matrix[1][3] = z13*rcp;
        inverse->matrix[3][1] = z31*rcp;
        inverse->matrix[2][2] = z22*rcp;
        inverse->matrix[2][3] = z23*rcp;
        inverse->matrix[3][2] = z32*rcp;
        inverse->matrix[3][3] = z33*rcp;
    }
    inverse->matrixType = __GL_MT_GENERAL;
}

GLvoid __glTransposeMatrix(__GLmatrix *transpose, const __GLmatrix *src)
{
    transpose->matrix[0][0] = src->matrix[0][0];
    transpose->matrix[0][1] = src->matrix[1][0];
    transpose->matrix[0][2] = src->matrix[2][0];
    transpose->matrix[0][3] = src->matrix[3][0];
    transpose->matrix[1][0] = src->matrix[0][1];
    transpose->matrix[1][1] = src->matrix[1][1];
    transpose->matrix[1][2] = src->matrix[2][1];
    transpose->matrix[1][3] = src->matrix[3][1];
    transpose->matrix[2][0] = src->matrix[0][2];
    transpose->matrix[2][1] = src->matrix[1][2];
    transpose->matrix[2][2] = src->matrix[2][2];
    transpose->matrix[2][3] = src->matrix[3][2];
    transpose->matrix[3][0] = src->matrix[0][3];
    transpose->matrix[3][1] = src->matrix[1][3];
    transpose->matrix[3][2] = src->matrix[2][3];
    transpose->matrix[3][3] = src->matrix[3][3];
}

/*
** Full 4x4 transformation.
*/
GLvoid __glTransformCoord(__GLcoord *res, __GLcoord *v, __GLmatrix *m)
{
    __GLfloat x = v->f.x;
    __GLfloat y = v->f.y;
    __GLfloat z = v->f.z;
    __GLfloat w = v->f.w;

    if (w == ((__GLfloat) 1.0)) {
        res->f.x = x*m->matrix[0][0] + y*m->matrix[1][0] + z*m->matrix[2][0]
            + m->matrix[3][0];
        res->f.y = x*m->matrix[0][1] + y*m->matrix[1][1] + z*m->matrix[2][1]
            + m->matrix[3][1];
        res->f.z = x*m->matrix[0][2] + y*m->matrix[1][2] + z*m->matrix[2][2]
            + m->matrix[3][2];
        res->f.w = x*m->matrix[0][3] + y*m->matrix[1][3] + z*m->matrix[2][3]
            + m->matrix[3][3];
    } else {
        res->f.x = x*m->matrix[0][0] + y*m->matrix[1][0] + z*m->matrix[2][0]
            + w*m->matrix[3][0];
        res->f.y = x*m->matrix[0][1] + y*m->matrix[1][1] + z*m->matrix[2][1]
            + w*m->matrix[3][1];
        res->f.z = x*m->matrix[0][2] + y*m->matrix[1][2] + z*m->matrix[2][2]
            + w*m->matrix[3][2];
        res->f.w = x*m->matrix[0][3] + y*m->matrix[1][3] + z*m->matrix[2][3]
            + w*m->matrix[3][3];
    }
}

GLvoid __glInvertTransposePoorMatrix(__GLmatrix *inverse, const __GLmatrix *srcRaw)
{
    __GLmatrix srcTemp;
    __GLmatrix *src = &srcTemp ;
    GLdouble x00, x01, x02;
    GLdouble x10, x11, x12;
    GLdouble x20, x21, x22;
    GLdouble rcp;

    __glCheckMatrixPricesion( src, srcRaw );

    /* propagate matrix type & branch if general */
    if (inverse->matrixType == src->matrixType)
    {
        GLdouble z00, z01, z02;
        GLdouble z10, z11, z12;
        GLdouble z20, z21, z22;

        /* read 3x3 matrix into registers */
        x00 = src->matrix[0][0];
        x01 = src->matrix[0][1];
        x02 = src->matrix[0][2];
        x10 = src->matrix[1][0];
        x11 = src->matrix[1][1];
        x12 = src->matrix[1][2];
        x20 = src->matrix[2][0];
        x21 = src->matrix[2][1];
        x22 = src->matrix[2][2];

        /* compute first three 2x2 cofactors */
        z20 = x01*x12 - x11*x02;
        z10 = x21*x02 - x01*x22;
        z00 = x11*x22 - x12*x21;

        /* compute 3x3 determinant & its reciprocal */
        rcp = x20*z20 + x10*z10 + x00*z00;
        if (rcp == (GLdouble)0)
        {
            return;
        }
        else if ( -__GL_MATRIX_FLOAT_SMALL_PRECISION < rcp && rcp < 0)
        {
            rcp = -__GL_MATRIX_FLOAT_SMALL_PRECISION;
        }
        else if ( 0 < rcp && rcp < __GL_MATRIX_FLOAT_SMALL_PRECISION )
        {
            rcp = __GL_MATRIX_FLOAT_SMALL_PRECISION;
        }

        rcp = (GLdouble)1/rcp;

        /* compute other six 2x2 cofactors */
        z01 = x20*x12 - x10*x22;
        z02 = x10*x21 - x20*x11;
        z11 = x00*x22 - x20*x02;
        z12 = x20*x01 - x00*x21;
        z21 = x10*x02 - x00*x12;
        z22 = x00*x11 - x10*x01;

        /* multiply all cofactors by reciprocal */
        inverse->matrix[0][0] = (GLfloat)(z00*rcp);
        inverse->matrix[0][1] = (GLfloat)(z01*rcp);
        inverse->matrix[0][2] = (GLfloat)(z02*rcp);
        inverse->matrix[1][0] = (GLfloat)(z10*rcp);
        inverse->matrix[1][1] = (GLfloat)(z11*rcp);
        inverse->matrix[1][2] = (GLfloat)(z12*rcp);
        inverse->matrix[2][0] = (GLfloat)(z20*rcp);
        inverse->matrix[2][1] = (GLfloat)(z21*rcp);
        inverse->matrix[2][2] = (GLfloat)(z22*rcp);

        /* read translation vector & negate */
        x00 = -src->matrix[3][0];
        x01 = -src->matrix[3][1];
        x02 = -src->matrix[3][2];

        /* store bottom row of inverse transpose */
        inverse->matrix[3][0] = 0;
        inverse->matrix[3][1] = 0;
        inverse->matrix[3][2] = 0;
        inverse->matrix[3][3] = 1;

        /* finish by tranforming translation vector */
        inverse->matrix[0][3] = (GLfloat)( inverse->matrix[0][0]*x00 +
            inverse->matrix[0][1]*x01 +
            inverse->matrix[0][2]*x02);
        inverse->matrix[1][3] = (GLfloat)(inverse->matrix[1][0]*x00 +
            inverse->matrix[1][1]*x01 +
            inverse->matrix[1][2]*x02);
        inverse->matrix[2][3] = (GLfloat)(inverse->matrix[2][0]*x00 +
            inverse->matrix[2][1]*x01 +
            inverse->matrix[2][2]*x02);
    }
    else
    {
        GLdouble x30, x31, x32;
        GLdouble y01, y02, y03, y12, y13, y23;
        GLdouble z02, z03, z12, z13, z22, z23, z32, z33;

#define x03 x01
#define x13 x11
#define x23 x21
#define x33 x31
#define z00 x02
#define z10 x12
#define z20 x22
#define z30 x32
#define z01 x03
#define z11 x13
#define z21 x23
#define z31 x33


        /* read 1st two columns of matrix into registers */
        x00 = src->matrix[0][0];
        x01 = src->matrix[0][1];
        x10 = src->matrix[1][0];
        x11 = src->matrix[1][1];
        x20 = src->matrix[2][0];
        x21 = src->matrix[2][1];
        x30 = src->matrix[3][0];
        x31 = src->matrix[3][1];

        /* compute all six 2x2 determinants of 1st two columns */
        y01 = x00*x11 - x10*x01;
        y02 = x00*x21 - x20*x01;
        y03 = x00*x31 - x30*x01;
        y12 = x10*x21 - x20*x11;
        y13 = x10*x31 - x30*x11;
        y23 = x20*x31 - x30*x21;

        /* read 2nd two columns of matrix into registers */
        x02 = src->matrix[0][2];
        x03 = src->matrix[0][3];
        x12 = src->matrix[1][2];
        x13 = src->matrix[1][3];
        x22 = src->matrix[2][2];
        x23 = src->matrix[2][3];
        x32 = src->matrix[3][2];
        x33 = src->matrix[3][3];

        /* compute all 3x3 cofactors for 2nd two columns */
        z33 = x02*y12 - x12*y02 + x22*y01;
        z23 = x12*y03 - x32*y01 - x02*y13;
        z13 = x02*y23 - x22*y03 + x32*y02;
        z03 = x22*y13 - x32*y12 - x12*y23;
        z32 = x13*y02 - x23*y01 - x03*y12;
        z22 = x03*y13 - x13*y03 + x33*y01;
        z12 = x23*y03 - x33*y02 - x03*y23;
        z02 = x13*y23 - x23*y13 + x33*y12;

        /* compute all six 2x2 determinants of 2nd two columns */
        y01 = x02*x13 - x12*x03;
        y02 = x02*x23 - x22*x03;
        y03 = x02*x33 - x32*x03;
        y12 = x12*x23 - x22*x13;
        y13 = x12*x33 - x32*x13;
        y23 = x22*x33 - x32*x23;

        /* read 1st two columns of matrix into registers */
        x00 = src->matrix[0][0];
        x01 = src->matrix[0][1];
        x10 = src->matrix[1][0];
        x11 = src->matrix[1][1];
        x20 = src->matrix[2][0];
        x21 = src->matrix[2][1];
        x30 = src->matrix[3][0];
        x31 = src->matrix[3][1];

        /* compute all 3x3 cofactors for 1st column */
        z30 = x11*y02 - x21*y01 - x01*y12;
        z20 = x01*y13 - x11*y03 + x31*y01;
        z10 = x21*y03 - x31*y02 - x01*y23;
        z00 = x11*y23 - x21*y13 + x31*y12;

        /* compute 4x4 determinant & its reciprocal */
        rcp = x30*z30 + x20*z20 + x10*z10 + x00*z00;
        if (rcp == (GLdouble)0)
        {
            return ;
        }
        else if ( -__GL_MATRIX_FLOAT_SMALL_PRECISION < rcp && rcp < 0)
        {
            rcp = -__GL_MATRIX_FLOAT_SMALL_PRECISION;
        }
        else if ( 0 < rcp && rcp < __GL_MATRIX_FLOAT_SMALL_PRECISION )
        {
            rcp = __GL_MATRIX_FLOAT_SMALL_PRECISION;
        }

        rcp = (GLdouble)1/rcp;

        /* compute all 3x3 cofactors for 2nd column */
        z31 = x00*y12 - x10*y02 + x20*y01;
        z21 = x10*y03 - x30*y01 - x00*y13;
        z11 = x00*y23 - x20*y03 + x30*y02;
        z01 = x20*y13 - x30*y12 - x10*y23;

        /* multiply all 3x3 cofactors by reciprocal */
        inverse->matrix[0][0] = (GLfloat)(z00*rcp);
        inverse->matrix[0][1] = (GLfloat)(z01*rcp);
        inverse->matrix[1][0] = (GLfloat)(z10*rcp);
        inverse->matrix[0][2] = (GLfloat)(z02*rcp);
        inverse->matrix[2][0] = (GLfloat)(z20*rcp);
        inverse->matrix[0][3] = (GLfloat)(z03*rcp);
        inverse->matrix[3][0] = (GLfloat)(z30*rcp);
        inverse->matrix[1][1] = (GLfloat)(z11*rcp);
        inverse->matrix[1][2] = (GLfloat)(z12*rcp);
        inverse->matrix[2][1] = (GLfloat)(z21*rcp);
        inverse->matrix[1][3] = (GLfloat)(z13*rcp);
        inverse->matrix[3][1] = (GLfloat)(z31*rcp);
        inverse->matrix[2][2] = (GLfloat)(z22*rcp);
        inverse->matrix[2][3] = (GLfloat)(z23*rcp);
        inverse->matrix[3][2] = (GLfloat)(z32*rcp);
        inverse->matrix[3][3] = (GLfloat)(z33*rcp);
    }

}


GLvoid __glTransformVector(__GLcontext *gc, __GLcoord *eyeDir, __GLcoord *mvDir,
                         __GLtransform *tr, GLboolean normalize)
{
    __GLcoord dir;

    if (tr->updateInverse) {
        (*gc->transform.matrix.invertTranspose)(&tr->inverseTranspose, &tr->matrix);
        tr->updateInverse = GL_FALSE;
    }

    if (normalize) {
        __glTransformCoord(&dir, mvDir, &tr->inverseTranspose);
        __glNormalize((GLfloat*)eyeDir, (GLfloat*)&dir);
    }
    else {
        __glTransformCoord(eyeDir, mvDir, &tr->inverseTranspose);
    }
}

