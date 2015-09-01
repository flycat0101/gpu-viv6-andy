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


#ifndef __gl_types_h_
#define __gl_types_h_

/*
** Low level data types.
*/

#include <limits.h>
#include <float.h>
#include <assert.h>
#include "glos.h"
union __GLvector2
{
    float v[2];
    struct
    {
        float x, y;
    };
    struct
    {
        unsigned int ix, iy;
    };
    struct
    {
        float s, t;
    };
    struct
    {
        unsigned int is, it;
    };
};

union __GLvector3
{
    float v[3];
    struct
    {
        float x, y, z;
    };
    struct
    {
        unsigned int ix, iy, iz;
    };
};

union __GLvector4
{
    float v[4];
    struct
    {
        float x, y, z, w;
    };
    struct
    {
        unsigned int ix, iy, iz, iw;
    };
    struct
    {
        float s, t, r, q;
    };
    struct
    {
        unsigned int is, it, ir, iq;
    };
};

typedef union __GLvector2 __GLvertex2;
typedef union __GLvector3 __GLvertex3;
typedef union __GLvector4 __GLvertex4;

/*
** Coordinate structure.  Coordinates contain x, y, z and w.
*/
typedef union __GLvector4  __GLcoord;
typedef union __GLvector3  __GLcoord3;

/*
** Color structure.  Colors are composed of red, green, blue and alpha.
*/
typedef struct __GLcolorRec {
    float r, g, b, a;
} __GLcolor;

typedef struct __GLcolorIuiRec {
    unsigned int r,g,b,a;
}__GLcolorIui;

typedef struct __GLcolorIiRec {
    GLint r,g,b,a;
}__GLcolorIi;

/************************************************************************/

/*
** Implementation data types.  The implementation is designed to run in
** single precision or double precision mode, all of which is controlled
** by an ifdef and the following typedef's.
**
** Use _GL_MAX_FLOAT to get the largest representable GL_FLOAT
*/
typedef float __GLfloat;
#define __GL_MAX_FLOAT __glFloatMax

/************************************************************************/


typedef unsigned int __GLzValue;
typedef GLushort __GLzValue16;

typedef struct __GLpixelSpanInfoRec __GLpixelSpanInfo;


/*
** Memory management for display list
*/
typedef struct __GLarenaRec __GLarena;

/*
** Polygon stipple word
*/
typedef GLuint __GLstippleWord;

typedef GLubyte __GLstencilCell;

typedef GLshort __GLaccumCellElement;

#define glALL_TO_UINT32(t) \
( \
    (GLuint) (uintptr_t) (t)\
)

#define glPTR_TO_UINT64(p) \
( \
    (GLuint64) (uintptr_t) (p)\
)

#define glUINT64_TO_PTR(u) \
( \
    (GLvoid *) (uintptr_t)(u)\
)

#endif /* __gl_types_h_ */
