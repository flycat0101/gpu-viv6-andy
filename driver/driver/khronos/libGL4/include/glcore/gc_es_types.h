/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gl_es_types_h__
#define __gl_es_types_h__

/*
** Low level data types.
*/
#include "gc_hal_user.h"

/*
** Define GL_APICALL and GL_GLEXT_PROTOTYPES before #include <GLES3/gl3.h>
*/
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#ifdef OPENGL40
#include <GL/gl.h>
#include <GL/glext.h>

#define GL_APICALL  GLAPI
#define GL_APIENTRY GLAPIENTRY
#include <KHR/khrplatform.h>
#include <GLES2/gl2ext.h>
#else
#include <GLES3/gl32.h>
#include <GLES2/gl2ext.h>
#endif


typedef union __GLvector2Union
{
    GLfloat v[2];
    struct __GLvertexVector2Rec
    {
        GLfloat x, y;
    } f;
    struct __GLintegerVertexVector2Rec
    {
        GLuint ix, iy;
    } i;
    struct __GLtexVector2Rec
    {
        GLfloat s, t;
    } fTex;
    struct __GLintegerTexVector2Rec
    {
        GLuint is, it;
    } iTex;
} __GLvector2;

typedef union __GLvector3Union
{
    GLfloat v[3];
    struct __GLvector3Rec
    {
        GLfloat x, y, z;
    } f;
    struct __GLintegerVector3Rec
    {
        GLuint ix, iy, iz;
    } i;
} __GLvector3;

typedef union __GLvector4Union
{
    GLfloat v[4];
    struct __GLvertexVector4Rec
    {
        GLfloat x, y, z, w;
    } f;
    struct __GLintegerVertexVector4Rec
    {
        GLuint ix, iy, iz, iw;
    } i;
    struct __GLtexVector4Rec
    {
        GLfloat s, t, r, q;
    } fTex;
    struct __GLintegerTexVector4Rec
    {
        GLuint is, it, ir, iq;
    } iTex;
} __GLvertex4, __GLcoord;

/*
** Color structure.  Colors are composed of red, green, blue and alpha.
*/
typedef struct __GLcolorRec
{
    GLfloat r, g, b, a;
} __GLcolor;

typedef struct __GLcolorIuiRec
{
    GLuint r,g,b,a;
}__GLcolorIui;

typedef struct __GLcolorIiRec
{
    GLint r,g,b,a;
}__GLcolorIi;

#ifdef OPENGL40
/*
** Implementation data types.  The implementation is designed to run in
** single precision or double precision mode, all of which is controlled
** by an ifdef and the following typedef's.
**
** Use _GL_MAX_FLOAT to get the largest representable GL_FLOAT
*/
typedef float __GLfloat;
#define __GL_MAX_FLOAT __glFloatMax

typedef unsigned int __GLzValue;
typedef GLushort __GLzValue16;

typedef struct __GLpixelSpanInfoRec __GLpixelSpanInfo;

/*
** Memory management for display list
*/
typedef struct __GLarenaRec __GLarena;

typedef __GLvector2 __GLvertex2;
typedef __GLvector3 __GLvertex3;

#endif

/************************************************************************/

#if defined(_WINDOWS)
#define __GL_INLINE static __forceinline
#else
#define __GL_INLINE static __inline
#endif

#define __GL_PTR2UINT(ptr)  ((GLuint)(gctUINTPTR_T)(ptr))
#define __GL_PTR2INT(ptr)   ((GLint)(gctUINTPTR_T)(ptr))

#define __GL_PTR2SIZE(v)    ((gctSIZE_T)(GLvoid*)(v))
#define __GL_SIZE2PTR(v)    ((void*)(gctSIZE_T)(v))

#define __GL_TABLE_SIZE(x) (GLsizei)(sizeof((x)) / sizeof((x)[0]))

#define __GL_TO_UINT32(t) \
( \
    (GLuint) (uintptr_t) (t)\
)

#define glALL_TO_UINT32(t) \
( \
    (GLuint) (uintptr_t) (t)\
)

#if defined(__arm__) || defined(i386)           || defined(__i386__) || defined(__x86__) || \
    defined(_M_IX86) || defined(__x86_64__)     || defined(_M_AMD64) || defined (_M_X64) || \
    defined(__e2k__) || defined(_LITTLE_ENDIAN) || defined(__arm64)  || defined(__arm64__)
#define __GL_LITTLE_ENDIAN
#endif

#endif /* __gl_es_types_h__ */
