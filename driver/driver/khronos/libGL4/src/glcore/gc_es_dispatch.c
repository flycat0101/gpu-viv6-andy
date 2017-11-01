/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_es_context.h"
#include "g_lefncs.h"
#include "g_lcfncs.h"

#define  SetDLTABLE(srccontext, dstcontext, funcname)  do { \
    (dstcontext)->funcname = (srccontext)->funcname; \
    }while(0)


/* ES 3.1 + Extension API Function Dispatch Table */
typedef GLvoid (GL_APIENTRY *NPFUNCTION)(GLvoid);
typedef const GLubyte * (*GLLEFUNCTION)(__GLcontext*, const GLubyte*);

#define __gles(func) __gles_##func
#define __glim(func) __glim_##func
#define __gllc(func) __gllc_##func
#define __gl4nop(func) __glnop_##func
#define __glle(func)  (GLLEFUNCTION)__glle_##func


GLvoid GL_APIENTRY __glapi_Nop(GLvoid) {}

const GLubyte *__glle_Noop(__GLcontext *gc, const GLubyte *pc)
{
    gcmFATAL("******************  __glle_Noop() should not be called!!!\n");
    return 0;
}

/* OGL Nop function */
GLboolean GL_APIENTRY __glnop_IsList(__GLcontext *gc,  GLuint list ){return GL_FALSE;}
GLvoid GL_APIENTRY __glnop_DeleteLists(__GLcontext *gc,  GLuint list, GLsizei range){}
GLuint GL_APIENTRY __glnop_GenLists(__GLcontext *gc,  GLsizei range ){return 0;}
GLvoid GL_APIENTRY __glnop_NewList(__GLcontext *gc,  GLuint list, GLenum mode ){}
GLvoid GL_APIENTRY __glnop_EndList(__GLcontext *gc){}
GLvoid GL_APIENTRY __glnop_CallList(__GLcontext *gc,  GLuint list ){}
GLvoid GL_APIENTRY __glnop_CallLists(__GLcontext *gc,  GLsizei n, GLenum type, const GLvoid *lists ){}
GLvoid GL_APIENTRY __glnop_ListBase(__GLcontext *gc,  GLuint base ){}
GLvoid GL_APIENTRY __glnop_Begin(__GLcontext *gc,  GLenum mode ){}
GLvoid GL_APIENTRY __glnop_Bitmap(__GLcontext *gc,  GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap ){}
GLvoid GL_APIENTRY __glnop_Color3b(__GLcontext *gc,  GLbyte red, GLbyte green, GLbyte blue ){}
GLvoid GL_APIENTRY __glnop_Color3d(__GLcontext *gc,  GLdouble red, GLdouble green, GLdouble blue ){}
GLvoid GL_APIENTRY __glnop_Color3f(__GLcontext *gc,  GLfloat red, GLfloat green, GLfloat blue ){}
GLvoid GL_APIENTRY __glnop_Color3i(__GLcontext *gc,  GLint red, GLint green, GLint blue ){}
GLvoid GL_APIENTRY __glnop_Color3s(__GLcontext *gc,  GLshort red, GLshort green, GLshort blue ){}
GLvoid GL_APIENTRY __glnop_Color3ub(__GLcontext *gc,  GLubyte red, GLubyte green, GLubyte blue ){}
GLvoid GL_APIENTRY __glnop_Color3ui(__GLcontext *gc,  GLuint red, GLuint green, GLuint blue ){}
GLvoid GL_APIENTRY __glnop_Color3us(__GLcontext *gc,  GLushort red, GLushort green, GLushort blue ){}
GLvoid GL_APIENTRY __glnop_Color3bv(__GLcontext *gc,  const GLbyte *v ){}
GLvoid GL_APIENTRY __glnop_Color3dv(__GLcontext *gc,  const GLdouble *v ){}
GLvoid GL_APIENTRY __glnop_Color3fv(__GLcontext *gc,  const GLfloat *v ){}
GLvoid GL_APIENTRY __glnop_Color3iv(__GLcontext *gc,  const GLint *v ){}
GLvoid GL_APIENTRY __glnop_Color3sv(__GLcontext *gc,  const GLshort *v ){}
GLvoid GL_APIENTRY __glnop_Color3ubv(__GLcontext *gc,  const GLubyte *v ){}
GLvoid GL_APIENTRY __glnop_Color3uiv(__GLcontext *gc,  const GLuint *v ){}
GLvoid GL_APIENTRY __glnop_Color3usv(__GLcontext *gc,  const GLushort *v ){}
GLvoid GL_APIENTRY __glnop_Color4b(__GLcontext *gc,  GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha ){}
GLvoid GL_APIENTRY __glnop_Color4d(__GLcontext *gc,  GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha ){}
GLvoid GL_APIENTRY __glnop_Color4f(__GLcontext *gc,  GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ){}
GLvoid GL_APIENTRY __glnop_Color4i(__GLcontext *gc,  GLint red, GLint green, GLint blue, GLint alpha ){}
GLvoid GL_APIENTRY __glnop_Color4s(__GLcontext *gc,  GLshort red, GLshort green, GLshort blue, GLshort alpha ){}
GLvoid GL_APIENTRY __glnop_Color4ub(__GLcontext *gc,  GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha ){}
GLvoid GL_APIENTRY __glnop_Color4ui(__GLcontext *gc,  GLuint red, GLuint green, GLuint blue, GLuint alpha ){}
GLvoid GL_APIENTRY __glnop_Color4us(__GLcontext *gc,  GLushort red, GLushort green, GLushort blue, GLushort alpha ){}
GLvoid GL_APIENTRY __glnop_Color4bv(__GLcontext *gc,  const GLbyte *v ){}
GLvoid GL_APIENTRY __glnop_Color4dv(__GLcontext *gc,  const GLdouble *v ){}
GLvoid GL_APIENTRY __glnop_Color4fv(__GLcontext *gc,  const GLfloat *v ){}
GLvoid GL_APIENTRY __glnop_Color4iv(__GLcontext *gc,  const GLint *v ){}
GLvoid GL_APIENTRY __glnop_Color4sv(__GLcontext *gc,  const GLshort *v ){}
GLvoid GL_APIENTRY __glnop_Color4ubv(__GLcontext *gc,  const GLubyte *v ){}
GLvoid GL_APIENTRY __glnop_Color4uiv(__GLcontext *gc,  const GLuint *v ){}
GLvoid GL_APIENTRY __glnop_Color4usv(__GLcontext *gc,  const GLushort *v ){}
GLvoid GL_APIENTRY __glnop_EdgeFlag(__GLcontext *gc,  GLboolean flag ){}
GLvoid GL_APIENTRY __glnop_EdgeFlagv(__GLcontext *gc,  const GLboolean *flag ){}
GLvoid GL_APIENTRY __glnop_End(__GLcontext *gc){}
GLvoid GL_APIENTRY __glnop_Indexd(__GLcontext *gc,  GLdouble c ){}
GLvoid GL_APIENTRY __glnop_Indexf(__GLcontext *gc,  GLfloat c ){}
GLvoid GL_APIENTRY __glnop_Indexi(__GLcontext *gc,  GLint c ){}
GLvoid GL_APIENTRY __glnop_Indexs(__GLcontext *gc,  GLshort c ){}
GLvoid GL_APIENTRY __glnop_Indexdv(__GLcontext *gc,  const GLdouble *c ){}
GLvoid GL_APIENTRY __glnop_Indexfv(__GLcontext *gc,  const GLfloat *c ){}
GLvoid GL_APIENTRY __glnop_Indexiv(__GLcontext *gc,  const GLint *c ){}
GLvoid GL_APIENTRY __glnop_Indexsv(__GLcontext *gc,  const GLshort *c ){}
GLvoid GL_APIENTRY __glnop_Normal3b(__GLcontext *gc,  GLbyte nx, GLbyte ny, GLbyte nz ){}
GLvoid GL_APIENTRY __glnop_Normal3d(__GLcontext *gc,  GLdouble nx, GLdouble ny, GLdouble nz ){}
GLvoid GL_APIENTRY __glnop_Normal3f(__GLcontext *gc,  GLfloat nx, GLfloat ny, GLfloat nz ){}
GLvoid GL_APIENTRY __glnop_Normal3i(__GLcontext *gc,  GLint nx, GLint ny, GLint nz ){}
GLvoid GL_APIENTRY __glnop_Normal3s(__GLcontext *gc,  GLshort nx, GLshort ny, GLshort nz ){}
GLvoid GL_APIENTRY __glnop_Normal3bv(__GLcontext *gc,  const GLbyte *v ){}
GLvoid GL_APIENTRY __glnop_Normal3dv(__GLcontext *gc,  const GLdouble *v ){}
GLvoid GL_APIENTRY __glnop_Normal3fv(__GLcontext *gc,  const GLfloat *v ){}
GLvoid GL_APIENTRY __glnop_Normal3iv(__GLcontext *gc,  const GLint *v ){}
GLvoid GL_APIENTRY __glnop_Normal3sv(__GLcontext *gc,  const GLshort *v ){}
GLvoid GL_APIENTRY __glnop_RasterPos2d(__GLcontext *gc,  GLdouble x, GLdouble y ){}
GLvoid GL_APIENTRY __glnop_RasterPos2f(__GLcontext *gc,  GLfloat x, GLfloat y ){}
GLvoid GL_APIENTRY __glnop_RasterPos2i(__GLcontext *gc,  GLint x, GLint y ){}
GLvoid GL_APIENTRY __glnop_RasterPos2s(__GLcontext *gc,  GLshort x, GLshort y ){}
GLvoid GL_APIENTRY __glnop_RasterPos3d(__GLcontext *gc,  GLdouble x, GLdouble y, GLdouble z ){}
GLvoid GL_APIENTRY __glnop_RasterPos3f(__GLcontext *gc,  GLfloat x, GLfloat y, GLfloat z ){}
GLvoid GL_APIENTRY __glnop_RasterPos3i(__GLcontext *gc,  GLint x, GLint y, GLint z ){}
GLvoid GL_APIENTRY __glnop_RasterPos3s(__GLcontext *gc,  GLshort x, GLshort y, GLshort z ){}
GLvoid GL_APIENTRY __glnop_RasterPos4d(__GLcontext *gc,  GLdouble x, GLdouble y, GLdouble z, GLdouble w ){}
GLvoid GL_APIENTRY __glnop_RasterPos4f(__GLcontext *gc,  GLfloat x, GLfloat y, GLfloat z, GLfloat w ){}
GLvoid GL_APIENTRY __glnop_RasterPos4i(__GLcontext *gc,  GLint x, GLint y, GLint z, GLint w ){}
GLvoid GL_APIENTRY __glnop_RasterPos4s(__GLcontext *gc,  GLshort x, GLshort y, GLshort z, GLshort w ){}
GLvoid GL_APIENTRY __glnop_RasterPos2dv(__GLcontext *gc,  const GLdouble *v ){}
GLvoid GL_APIENTRY __glnop_RasterPos2fv(__GLcontext *gc,  const GLfloat *v ){}
GLvoid GL_APIENTRY __glnop_RasterPos2iv(__GLcontext *gc,  const GLint *v ){}
GLvoid GL_APIENTRY __glnop_RasterPos2sv(__GLcontext *gc,  const GLshort *v ){}
GLvoid GL_APIENTRY __glnop_RasterPos3dv(__GLcontext *gc,  const GLdouble *v ){}
GLvoid GL_APIENTRY __glnop_RasterPos3fv(__GLcontext *gc,  const GLfloat *v ){}
GLvoid GL_APIENTRY __glnop_RasterPos3iv(__GLcontext *gc,  const GLint *v ){}
GLvoid GL_APIENTRY __glnop_RasterPos3sv(__GLcontext *gc,  const GLshort *v ){}
GLvoid GL_APIENTRY __glnop_RasterPos4dv(__GLcontext *gc,  const GLdouble *v ){}
GLvoid GL_APIENTRY __glnop_RasterPos4fv(__GLcontext *gc,  const GLfloat *v ){}
GLvoid GL_APIENTRY __glnop_RasterPos4iv(__GLcontext *gc,  const GLint *v ){}
GLvoid GL_APIENTRY __glnop_RasterPos4sv(__GLcontext *gc,  const GLshort *v ){}
GLvoid GL_APIENTRY __glnop_Rectd(__GLcontext *gc,  GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2 ){}
GLvoid GL_APIENTRY __glnop_Rectf(__GLcontext *gc,  GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 ){}
GLvoid GL_APIENTRY __glnop_Recti(__GLcontext *gc,  GLint x1, GLint y1, GLint x2, GLint y2 ){}
GLvoid GL_APIENTRY __glnop_Rects(__GLcontext *gc,  GLshort x1, GLshort y1, GLshort x2, GLshort y2 ){}
GLvoid GL_APIENTRY __glnop_Rectdv(__GLcontext *gc,  const GLdouble *v1, const GLdouble *v2 ){}
GLvoid GL_APIENTRY __glnop_Rectfv(__GLcontext *gc,  const GLfloat *v1, const GLfloat *v2 ){}
GLvoid GL_APIENTRY __glnop_Rectiv(__GLcontext *gc,  const GLint *v1, const GLint *v2 ){}
GLvoid GL_APIENTRY __glnop_Rectsv(__GLcontext *gc,  const GLshort *v1, const GLshort *v2 ){}
GLvoid GL_APIENTRY __glnop_TexCoord1d(__GLcontext *gc,  GLdouble s ){}
GLvoid GL_APIENTRY __glnop_TexCoord1f(__GLcontext *gc,  GLfloat s ){}
GLvoid GL_APIENTRY __glnop_TexCoord1i(__GLcontext *gc,  GLint s ){}
GLvoid GL_APIENTRY __glnop_TexCoord1s(__GLcontext *gc,  GLshort s ){}
GLvoid GL_APIENTRY __glnop_TexCoord2d(__GLcontext *gc,  GLdouble s, GLdouble t ){}
GLvoid GL_APIENTRY __glnop_TexCoord2f(__GLcontext *gc,  GLfloat s, GLfloat t ){}
GLvoid GL_APIENTRY __glnop_TexCoord2i(__GLcontext *gc,  GLint s, GLint t ){}
GLvoid GL_APIENTRY __glnop_TexCoord2s(__GLcontext *gc,  GLshort s, GLshort t ){}
GLvoid GL_APIENTRY __glnop_TexCoord3d(__GLcontext *gc,  GLdouble s, GLdouble t, GLdouble r ){}
GLvoid GL_APIENTRY __glnop_TexCoord3f(__GLcontext *gc,  GLfloat s, GLfloat t, GLfloat r ){}
GLvoid GL_APIENTRY __glnop_TexCoord3i(__GLcontext *gc,  GLint s, GLint t, GLint r ){}
GLvoid GL_APIENTRY __glnop_TexCoord3s(__GLcontext *gc,  GLshort s, GLshort t, GLshort r ){}
GLvoid GL_APIENTRY __glnop_TexCoord4d(__GLcontext *gc,  GLdouble s, GLdouble t, GLdouble r, GLdouble q ){}
GLvoid GL_APIENTRY __glnop_TexCoord4f(__GLcontext *gc,  GLfloat s, GLfloat t, GLfloat r, GLfloat q ){}
GLvoid GL_APIENTRY __glnop_TexCoord4i(__GLcontext *gc,  GLint s, GLint t, GLint r, GLint q ){}
GLvoid GL_APIENTRY __glnop_TexCoord4s(__GLcontext *gc,  GLshort s, GLshort t, GLshort r, GLshort q ){}
GLvoid GL_APIENTRY __glnop_TexCoord1dv(__GLcontext *gc,  const GLdouble *v ){}
GLvoid GL_APIENTRY __glnop_TexCoord1fv(__GLcontext *gc,  const GLfloat *v ){}
GLvoid GL_APIENTRY __glnop_TexCoord1iv(__GLcontext *gc,  const GLint *v ){}
GLvoid GL_APIENTRY __glnop_TexCoord1sv(__GLcontext *gc,  const GLshort *v ){}
GLvoid GL_APIENTRY __glnop_TexCoord2dv(__GLcontext *gc,  const GLdouble *v ){}
GLvoid GL_APIENTRY __glnop_TexCoord2fv(__GLcontext *gc,  const GLfloat *v ){}
GLvoid GL_APIENTRY __glnop_TexCoord2iv(__GLcontext *gc,  const GLint *v ){}
GLvoid GL_APIENTRY __glnop_TexCoord2sv(__GLcontext *gc,  const GLshort *v ){}
GLvoid GL_APIENTRY __glnop_TexCoord3dv(__GLcontext *gc,  const GLdouble *v ){}
GLvoid GL_APIENTRY __glnop_TexCoord3fv(__GLcontext *gc,  const GLfloat *v ){}
GLvoid GL_APIENTRY __glnop_TexCoord3iv(__GLcontext *gc,  const GLint *v ){}
GLvoid GL_APIENTRY __glnop_TexCoord3sv(__GLcontext *gc,  const GLshort *v ){}
GLvoid GL_APIENTRY __glnop_TexCoord4dv(__GLcontext *gc,  const GLdouble *v ){}
GLvoid GL_APIENTRY __glnop_TexCoord4fv(__GLcontext *gc,  const GLfloat *v ){}
GLvoid GL_APIENTRY __glnop_TexCoord4iv(__GLcontext *gc,  const GLint *v ){}
GLvoid GL_APIENTRY __glnop_TexCoord4sv(__GLcontext *gc,  const GLshort *v ){}
GLvoid GL_APIENTRY __glnop_Vertex2d(__GLcontext *gc,  GLdouble x, GLdouble y ){}
GLvoid GL_APIENTRY __glnop_Vertex2f(__GLcontext *gc,  GLfloat x, GLfloat y ){}
GLvoid GL_APIENTRY __glnop_Vertex2i(__GLcontext *gc,  GLint x, GLint y ){}
GLvoid GL_APIENTRY __glnop_Vertex2s(__GLcontext *gc,  GLshort x, GLshort y ){}
GLvoid GL_APIENTRY __glnop_Vertex3d(__GLcontext *gc,  GLdouble x, GLdouble y, GLdouble z ){}
GLvoid GL_APIENTRY __glnop_Vertex3f(__GLcontext *gc,  GLfloat x, GLfloat y, GLfloat z ){}
GLvoid GL_APIENTRY __glnop_Vertex3i(__GLcontext *gc,  GLint x, GLint y, GLint z ){}
GLvoid GL_APIENTRY __glnop_Vertex3s(__GLcontext *gc,  GLshort x, GLshort y, GLshort z ){}
GLvoid GL_APIENTRY __glnop_Vertex4d(__GLcontext *gc,  GLdouble x, GLdouble y, GLdouble z, GLdouble w ){}
GLvoid GL_APIENTRY __glnop_Vertex4f(__GLcontext *gc,  GLfloat x, GLfloat y, GLfloat z, GLfloat w ){}
GLvoid GL_APIENTRY __glnop_Vertex4i(__GLcontext *gc,  GLint x, GLint y, GLint z, GLint w ){}
GLvoid GL_APIENTRY __glnop_Vertex4s(__GLcontext *gc,  GLshort x, GLshort y, GLshort z, GLshort w ){}
GLvoid GL_APIENTRY __glnop_Vertex2dv(__GLcontext *gc,  const GLdouble *v ){}
GLvoid GL_APIENTRY __glnop_Vertex2fv(__GLcontext *gc,  const GLfloat *v ){}
GLvoid GL_APIENTRY __glnop_Vertex2iv(__GLcontext *gc,  const GLint *v ){}
GLvoid GL_APIENTRY __glnop_Vertex2sv(__GLcontext *gc,  const GLshort *v ){}
GLvoid GL_APIENTRY __glnop_Vertex3dv(__GLcontext *gc,  const GLdouble *v ){}
GLvoid GL_APIENTRY __glnop_Vertex3fv(__GLcontext *gc,  const GLfloat *v ){}
GLvoid GL_APIENTRY __glnop_Vertex3iv(__GLcontext *gc,  const GLint *v ){}
GLvoid GL_APIENTRY __glnop_Vertex3sv(__GLcontext *gc,  const GLshort *v ){}
GLvoid GL_APIENTRY __glnop_Vertex4dv(__GLcontext *gc,  const GLdouble *v ){}
GLvoid GL_APIENTRY __glnop_Vertex4fv(__GLcontext *gc,  const GLfloat *v ){}
GLvoid GL_APIENTRY __glnop_Vertex4iv(__GLcontext *gc,  const GLint *v ){}
GLvoid GL_APIENTRY __glnop_Vertex4sv(__GLcontext *gc,  const GLshort *v ){}
GLvoid GL_APIENTRY __glnop_ClipPlane(__GLcontext *gc,  GLenum plane, const GLdouble *equation ){}
GLvoid GL_APIENTRY __glnop_ColorMaterial(__GLcontext *gc,  GLenum face, GLenum mode ){}
GLvoid GL_APIENTRY __glnop_Fogf(__GLcontext *gc,  GLenum pname, GLfloat param ){}
GLvoid GL_APIENTRY __glnop_Fogi(__GLcontext *gc,  GLenum pname, GLint param ){}
GLvoid GL_APIENTRY __glnop_Fogfv(__GLcontext *gc,  GLenum pname, const GLfloat *params ){}
GLvoid GL_APIENTRY __glnop_Fogiv(__GLcontext *gc,  GLenum pname, const GLint *params ){}
GLvoid GL_APIENTRY __glnop_Lightf(__GLcontext *gc,  GLenum light, GLenum pname, GLfloat param ){}
GLvoid GL_APIENTRY __glnop_Lighti(__GLcontext *gc,  GLenum light, GLenum pname, GLint param ){}
GLvoid GL_APIENTRY __glnop_Lightfv(__GLcontext *gc,  GLenum light, GLenum pname, const GLfloat *params ){}
GLvoid GL_APIENTRY __glnop_Lightiv(__GLcontext *gc,  GLenum light, GLenum pname, const GLint *params ){}
GLvoid GL_APIENTRY __glnop_LightModelf(__GLcontext *gc,  GLenum pname, GLfloat param ){}
GLvoid GL_APIENTRY __glnop_LightModeli(__GLcontext *gc,  GLenum pname, GLint param ){}
GLvoid GL_APIENTRY __glnop_LightModelfv(__GLcontext *gc,  GLenum pname, const GLfloat *params ){}
GLvoid GL_APIENTRY __glnop_LightModeliv(__GLcontext *gc,  GLenum pname, const GLint *params ){}
GLvoid GL_APIENTRY __glnop_LineStipple(__GLcontext *gc,  GLint factor, GLushort pattern ){}
GLvoid GL_APIENTRY __glnop_Materialf(__GLcontext *gc,  GLenum face, GLenum pname, GLfloat param ){}
GLvoid GL_APIENTRY __glnop_Materiali(__GLcontext *gc,  GLenum face, GLenum pname, GLint param ){}
GLvoid GL_APIENTRY __glnop_Materialfv(__GLcontext *gc,  GLenum face, GLenum pname, const GLfloat *params ){}
GLvoid GL_APIENTRY __glnop_Materialiv(__GLcontext *gc,  GLenum face, GLenum pname, const GLint *params ){}
GLvoid GL_APIENTRY __glnop_PointSize(__GLcontext *gc,  GLfloat size ){}
GLvoid GL_APIENTRY __glnop_PolygonMode(__GLcontext *gc,  GLenum face, GLenum mode ){}
GLvoid GL_APIENTRY __glnop_PolygonStipple(__GLcontext *gc,  const GLubyte *mask ){}
GLvoid GL_APIENTRY __glnop_ShadeModel(__GLcontext *gc,  GLenum mode ){}
GLvoid GL_APIENTRY __glnop_TexImage1D(__GLcontext *gc,  GLenum target, GLint level, GLint internalFormat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels ){}
GLvoid GL_APIENTRY __glnop_TexEnvf(__GLcontext *gc,  GLenum target, GLenum pname, GLfloat param ){}
GLvoid GL_APIENTRY __glnop_TexEnvi(__GLcontext *gc,  GLenum target, GLenum pname, GLint param ){}
GLvoid GL_APIENTRY __glnop_TexEnvfv(__GLcontext *gc,  GLenum target, GLenum pname, const GLfloat *params ){}
GLvoid GL_APIENTRY __glnop_TexEnviv(__GLcontext *gc,  GLenum target, GLenum pname, const GLint *params ){}
GLvoid GL_APIENTRY __glnop_TexGend(__GLcontext *gc,  GLenum coord, GLenum pname, GLdouble param ){}
GLvoid GL_APIENTRY __glnop_TexGenf(__GLcontext *gc,  GLenum coord, GLenum pname, GLfloat param ){}
GLvoid GL_APIENTRY __glnop_TexGeni(__GLcontext *gc,  GLenum coord, GLenum pname, GLint param ){}
GLvoid GL_APIENTRY __glnop_TexGendv(__GLcontext *gc,  GLenum coord, GLenum pname, const GLdouble *params ){}
GLvoid GL_APIENTRY __glnop_TexGenfv(__GLcontext *gc,  GLenum coord, GLenum pname, const GLfloat *params ){}
GLvoid GL_APIENTRY __glnop_TexGeniv(__GLcontext *gc,  GLenum coord, GLenum pname, const GLint *params ){}
GLvoid GL_APIENTRY __glnop_FeedbackBuffer(__GLcontext *gc,  GLsizei size, GLenum type, GLfloat *buffer ){}
GLvoid GL_APIENTRY __glnop_SelectBuffer(__GLcontext *gc,  GLsizei size, GLuint *buffer ){}
GLint  GL_APIENTRY __glnop_RenderMode(__GLcontext *gc,  GLenum mode ){return 0;}
GLvoid GL_APIENTRY __glnop_InitNames(__GLcontext *gc){}
GLvoid GL_APIENTRY __glnop_LoadName(__GLcontext *gc,  GLuint name ){}
GLvoid GL_APIENTRY __glnop_PushName(__GLcontext *gc,  GLuint name ){}
GLvoid GL_APIENTRY __glnop_PopName(__GLcontext *gc){}
GLvoid GL_APIENTRY __glnop_PassThrough(__GLcontext *gc,  GLfloat token ){}
GLvoid GL_APIENTRY __glnop_DrawBuffer(__GLcontext *gc,  GLenum mode ){}
GLvoid GL_APIENTRY __glnop_ClearAccum(__GLcontext *gc,  GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ){}
GLvoid GL_APIENTRY __glnop_ClearIndex(__GLcontext *gc,  GLfloat c ){}
GLvoid GL_APIENTRY __glnop_ClearDepth(__GLcontext *gc,  GLclampd depth ){}
GLvoid GL_APIENTRY __glnop_IndexMask(__GLcontext *gc,  GLuint mask ){}
GLvoid GL_APIENTRY __glnop_Accum(__GLcontext *gc,  GLenum op, GLfloat value ){}
GLvoid GL_APIENTRY __glnop_PushAttrib(__GLcontext *gc,  GLbitfield mask ){}
GLvoid GL_APIENTRY __glnop_PopAttrib(__GLcontext *gc){}
GLvoid GL_APIENTRY __glnop_Map1d(__GLcontext *gc,  GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points ){}
GLvoid GL_APIENTRY __glnop_Map1f(__GLcontext *gc,  GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points ){}
GLvoid GL_APIENTRY __glnop_Map2d(__GLcontext *gc,  GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points ){}
GLvoid GL_APIENTRY __glnop_Map2f(__GLcontext *gc,  GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points ){}
GLvoid GL_APIENTRY __glnop_MapGrid1d(__GLcontext *gc,  GLint un, GLdouble u1, GLdouble u2 ){}
GLvoid GL_APIENTRY __glnop_MapGrid1f(__GLcontext *gc,  GLint un, GLfloat u1, GLfloat u2 ){}
GLvoid GL_APIENTRY __glnop_MapGrid2d(__GLcontext *gc,  GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2 ){}
GLvoid GL_APIENTRY __glnop_MapGrid2f(__GLcontext *gc,  GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2 ){}
GLvoid GL_APIENTRY __glnop_EvalCoord1d(__GLcontext *gc,  GLdouble u ){}
GLvoid GL_APIENTRY __glnop_EvalCoord1f(__GLcontext *gc,  GLfloat u ){}
GLvoid GL_APIENTRY __glnop_EvalCoord1dv(__GLcontext *gc,  const GLdouble *u ){}
GLvoid GL_APIENTRY __glnop_EvalCoord1fv(__GLcontext *gc,  const GLfloat *u ){}
GLvoid GL_APIENTRY __glnop_EvalCoord2d(__GLcontext *gc,  GLdouble u, GLdouble v ){}
GLvoid GL_APIENTRY __glnop_EvalCoord2f(__GLcontext *gc,  GLfloat u, GLfloat v ){}
GLvoid GL_APIENTRY __glnop_EvalCoord2dv(__GLcontext *gc,  const GLdouble *u ){}
GLvoid GL_APIENTRY __glnop_EvalCoord2fv(__GLcontext *gc,  const GLfloat *u ){}
GLvoid GL_APIENTRY __glnop_EvalPoint1(__GLcontext *gc,  GLint i ){}
GLvoid GL_APIENTRY __glnop_EvalPoint2(__GLcontext *gc,  GLint i, GLint j ){}
GLvoid GL_APIENTRY __glnop_EvalMesh1(__GLcontext *gc,  GLenum mode, GLint i1, GLint i2 ){}
GLvoid GL_APIENTRY __glnop_EvalMesh2(__GLcontext *gc,  GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2 ){}
GLvoid GL_APIENTRY __glnop_AlphaFunc(__GLcontext *gc,  GLenum func, GLclampf ref ){}
GLvoid GL_APIENTRY __glnop_LogicOp(__GLcontext *gc,  GLenum opcode ){}
GLvoid GL_APIENTRY __glnop_PixelZoom(__GLcontext *gc,  GLfloat xfactor, GLfloat yfactor ){}
GLvoid GL_APIENTRY __glnop_PixelTransferf(__GLcontext *gc,  GLenum pname, GLfloat param ){}
GLvoid GL_APIENTRY __glnop_PixelTransferi(__GLcontext *gc,  GLenum pname, GLint param ){}
GLvoid GL_APIENTRY __glnop_PixelStoref(__GLcontext *gc,  GLenum pname, GLfloat param ){}
GLvoid GL_APIENTRY __glnop_PixelMapfv(__GLcontext *gc,  GLenum map, GLsizei mapsize, const GLfloat *values ){}
GLvoid GL_APIENTRY __glnop_PixelMapuiv(__GLcontext *gc,  GLenum map, GLsizei mapsize, const GLuint *values ){}
GLvoid GL_APIENTRY __glnop_PixelMapusv(__GLcontext *gc,  GLenum map, GLsizei mapsize, const GLushort *values ){}
GLvoid GL_APIENTRY __glnop_GetClipPlane(__GLcontext *gc,  GLenum plane, GLdouble *equation ){}
GLvoid GL_APIENTRY __glnop_GetDoublev(__GLcontext *gc,  GLenum pname, GLdouble *params ){}
GLvoid GL_APIENTRY __glnop_GetLightfv(__GLcontext *gc,  GLenum light, GLenum pname, GLfloat *params ){}
GLvoid GL_APIENTRY __glnop_GetLightiv(__GLcontext *gc,  GLenum light, GLenum pname, GLint *params ){}
GLvoid GL_APIENTRY __glnop_GetMapdv(__GLcontext *gc,  GLenum target, GLenum query, GLdouble *v ){}
GLvoid GL_APIENTRY __glnop_GetMapfv(__GLcontext *gc,  GLenum target, GLenum query, GLfloat *v ){}
GLvoid GL_APIENTRY __glnop_GetMapiv(__GLcontext *gc,  GLenum target, GLenum query, GLint *v ){}
GLvoid GL_APIENTRY __glnop_GetMaterialfv(__GLcontext *gc,  GLenum face, GLenum pname, GLfloat *params ){}
GLvoid GL_APIENTRY __glnop_GetMaterialiv(__GLcontext *gc,  GLenum face, GLenum pname, GLint *params ){}
GLvoid GL_APIENTRY __glnop_GetPixelMapfv(__GLcontext *gc,  GLenum map, GLfloat *values ){}
GLvoid GL_APIENTRY __glnop_GetPixelMapuiv(__GLcontext *gc,  GLenum map, GLuint *values ){}
GLvoid GL_APIENTRY __glnop_GetPixelMapusv(__GLcontext *gc,  GLenum map, GLushort *values ){}
GLvoid GL_APIENTRY __glnop_GetPolygonStipple(__GLcontext *gc,  GLubyte *mask ){}
GLvoid GL_APIENTRY __glnop_GetTexEnvfv(__GLcontext *gc,  GLenum target, GLenum pname, GLfloat *params ){}
GLvoid GL_APIENTRY __glnop_GetTexEnviv(__GLcontext *gc,  GLenum target, GLenum pname, GLint *params ){}
GLvoid GL_APIENTRY __glnop_GetTexGendv(__GLcontext *gc,  GLenum coord, GLenum pname, GLdouble *params ){}
GLvoid GL_APIENTRY __glnop_GetTexGenfv(__GLcontext *gc,  GLenum coord, GLenum pname, GLfloat *params ){}
GLvoid GL_APIENTRY __glnop_GetTexGeniv(__GLcontext *gc,  GLenum coord, GLenum pname, GLint *params ){}
GLvoid GL_APIENTRY __glnop_GetTexImage(__GLcontext *gc,  GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels ){}
GLvoid GL_APIENTRY __glnop_DepthRange(__GLcontext *gc,  GLclampd near_val, GLclampd far_val ){}
GLvoid GL_APIENTRY __glnop_Frustum(__GLcontext *gc,  GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val ){}
GLvoid GL_APIENTRY __glnop_LoadIdentity(__GLcontext *gc){}
GLvoid GL_APIENTRY __glnop_LoadMatrixd(__GLcontext *gc,  const GLdouble *m ){}
GLvoid GL_APIENTRY __glnop_LoadMatrixf(__GLcontext *gc,  const GLfloat *m ){}
GLvoid GL_APIENTRY __glnop_MatrixMode(__GLcontext *gc,  GLenum mode ){}
GLvoid GL_APIENTRY __glnop_MultMatrixd(__GLcontext *gc,  const GLdouble *m ){}
GLvoid GL_APIENTRY __glnop_MultMatrixf(__GLcontext *gc,  const GLfloat *m ){}
GLvoid GL_APIENTRY __glnop_Ortho(__GLcontext *gc,  GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val ){}
GLvoid GL_APIENTRY __glnop_PushMatrix(__GLcontext *gc){}
GLvoid GL_APIENTRY __glnop_PopMatrix(__GLcontext *gc){}
GLvoid GL_APIENTRY __glnop_Rotated(__GLcontext *gc,  GLdouble angle, GLdouble x, GLdouble y, GLdouble z ){}
GLvoid GL_APIENTRY __glnop_Rotatef(__GLcontext *gc,  GLfloat angle, GLfloat x, GLfloat y, GLfloat z ){}
GLvoid GL_APIENTRY __glnop_Scaled(__GLcontext *gc,  GLdouble x, GLdouble y, GLdouble z ){}
GLvoid GL_APIENTRY __glnop_Scalef(__GLcontext *gc,  GLfloat x, GLfloat y, GLfloat z ){}
GLvoid GL_APIENTRY __glnop_Translated(__GLcontext *gc,  GLdouble x, GLdouble y, GLdouble z ){}
GLvoid GL_APIENTRY __glnop_Translatef(__GLcontext *gc,  GLfloat x, GLfloat y, GLfloat z ){}
GLvoid GL_APIENTRY __glnop_ArrayElement(__GLcontext *gc,  GLint i ){}
GLvoid GL_APIENTRY __glnop_ColorPointer(__GLcontext *gc,  GLint size, GLenum type, GLsizei stride, const GLvoid *ptr ){}
GLvoid GL_APIENTRY __glnop_EnableClientState(__GLcontext *gc,  GLenum cap ){}
GLvoid GL_APIENTRY __glnop_DisableClientState(__GLcontext *gc,  GLenum cap ){}
GLvoid GL_APIENTRY __glnop_EdgeFlagPointer(__GLcontext *gc,  GLsizei stride, const GLvoid *ptr ){}
GLvoid GL_APIENTRY __glnop_IndexPointer(__GLcontext *gc,  GLenum type, GLsizei stride, const GLvoid *ptr ){}
GLvoid GL_APIENTRY __glnop_InterleavedArrays(__GLcontext *gc,  GLenum format, GLsizei stride, const GLvoid *pointer ){}
GLvoid GL_APIENTRY __glnop_NormalPointer(__GLcontext *gc,  GLenum type, GLsizei stride,  const GLvoid *ptr ){}
GLvoid GL_APIENTRY __glnop_VertexPointer(__GLcontext *gc,  GLint size, GLenum type, GLsizei stride, const GLvoid *ptr ){}
GLvoid GL_APIENTRY __glnop_TexCoordPointer(__GLcontext *gc,  GLint size, GLenum type, GLsizei stride, const GLvoid *ptr ){}
GLboolean GL_APIENTRY __glnop_AreTexturesResident(__GLcontext *gc,  GLsizei n, const GLuint *textures, GLboolean *residences ){return GL_FALSE;}
GLvoid GL_APIENTRY __glnop_CopyTexImage1D(__GLcontext *gc,  GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border ){}
GLvoid GL_APIENTRY __glnop_CopyTexSubImage1D(__GLcontext *gc,  GLenum target, GLint level,  GLint xoffset, GLint x, GLint y,  GLsizei width ){}
GLvoid GL_APIENTRY __glnop_PrioritizeTextures(__GLcontext *gc,  GLsizei n,  const GLuint *textures, const GLclampf *priorities ){}
GLvoid GL_APIENTRY __glnop_TexSubImage1D(__GLcontext *gc,  GLenum target, GLint level,  GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels ){}
GLvoid GL_APIENTRY __glnop_PushClientAttrib(__GLcontext *gc,  GLbitfield mask ){}
GLvoid GL_APIENTRY __glnop_PopClientAttrib(__GLcontext *gc){}
GLvoid GL_APIENTRY __glnop_ColorTable(__GLcontext *gc,  GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table ){}
GLvoid GL_APIENTRY __glnop_ColorTableParameteriv(__GLcontext *gc, GLenum target, GLenum pname, const GLint *params){}
GLvoid GL_APIENTRY __glnop_ColorTableParameterfv(__GLcontext *gc, GLenum target, GLenum pname, const GLfloat *params){}
GLvoid GL_APIENTRY __glnop_CopyColorTable(__GLcontext *gc,  GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width ){}
GLvoid GL_APIENTRY __glnop_GetColorTable(__GLcontext *gc,  GLenum target, GLenum format,  GLenum type, GLvoid *table ){}
GLvoid GL_APIENTRY __glnop_GetColorTableParameterfv(__GLcontext *gc,  GLenum target, GLenum pname, GLfloat *params ){}
GLvoid GL_APIENTRY __glnop_GetColorTableParameteriv(__GLcontext *gc,  GLenum target, GLenum pname,  GLint *params ){}
GLvoid GL_APIENTRY __glnop_ColorSubTable(__GLcontext *gc,  GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data ){}
GLvoid GL_APIENTRY __glnop_CopyColorSubTable(__GLcontext *gc,  GLenum target, GLsizei start, GLint x, GLint y, GLsizei width ){}
GLvoid GL_APIENTRY __glnop_ConvolutionFilter1D(__GLcontext *gc,  GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image ){}
GLvoid GL_APIENTRY __glnop_ConvolutionFilter2D(__GLcontext *gc,  GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image ){}
GLvoid GL_APIENTRY __glnop_ConvolutionParameterf(__GLcontext *gc,  GLenum target, GLenum pname, GLfloat params ){}
GLvoid GL_APIENTRY __glnop_ConvolutionParameterfv(__GLcontext *gc,  GLenum target, GLenum pname, const GLfloat *params ){}
GLvoid GL_APIENTRY __glnop_ConvolutionParameteri(__GLcontext *gc,  GLenum target, GLenum pname, GLint params ){}
GLvoid GL_APIENTRY __glnop_ConvolutionParameteriv(__GLcontext *gc,  GLenum target, GLenum pname, const GLint *params ){}
GLvoid GL_APIENTRY __glnop_CopyConvolutionFilter1D(__GLcontext *gc,  GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width ){}
GLvoid GL_APIENTRY __glnop_CopyConvolutionFilter2D(__GLcontext *gc,  GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height){}
GLvoid GL_APIENTRY __glnop_GetConvolutionFilter(__GLcontext *gc,  GLenum target, GLenum format, GLenum type, GLvoid *image ){}
GLvoid GL_APIENTRY __glnop_GetConvolutionParameterfv(__GLcontext *gc,  GLenum target, GLenum pname, GLfloat *params ){}
GLvoid GL_APIENTRY __glnop_GetConvolutionParameteriv(__GLcontext *gc,  GLenum target, GLenum pname, GLint *params ){}
GLvoid GL_APIENTRY __glnop_SeparableFilter2D(__GLcontext *gc,  GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column ){}
GLvoid GL_APIENTRY __glnop_GetSeparableFilter(__GLcontext *gc,  GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span ){}
GLvoid GL_APIENTRY __glnop_Histogram(__GLcontext *gc,  GLenum target, GLsizei width, GLenum internalformat, GLboolean sink ){}
GLvoid GL_APIENTRY __glnop_ResetHistogram(__GLcontext *gc,  GLenum target ){}
GLvoid GL_APIENTRY __glnop_GetHistogram(__GLcontext *gc,  GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values ){}
GLvoid GL_APIENTRY __glnop_GetHistogramParameterfv(__GLcontext *gc,  GLenum target, GLenum pname, GLfloat *params ){}
GLvoid GL_APIENTRY __glnop_GetHistogramParameteriv(__GLcontext *gc,  GLenum target, GLenum pname, GLint *params ){}
GLvoid GL_APIENTRY __glnop_Minmax(__GLcontext *gc,  GLenum target, GLenum internalformat, GLboolean sink ){}
GLvoid GL_APIENTRY __glnop_ResetMinmax(__GLcontext *gc,  GLenum target ){}
GLvoid GL_APIENTRY __glnop_GetMinmax(__GLcontext *gc,  GLenum target, GLboolean reset, GLenum format, GLenum types, GLvoid *values ){}
GLvoid GL_APIENTRY __glnop_GetMinmaxParameterfv(__GLcontext *gc,  GLenum target, GLenum pname,  GLfloat *params ){}
GLvoid GL_APIENTRY __glnop_GetMinmaxParameteriv(__GLcontext *gc,  GLenum target, GLenum pname, GLint *params ){}
GLvoid GL_APIENTRY __glnop_ClientActiveTexture(__GLcontext *gc,  GLenum texture ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord1d(__GLcontext *gc,  GLenum target, GLdouble s ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord1dv(__GLcontext *gc,  GLenum target, const GLdouble *v ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord1f(__GLcontext *gc,  GLenum target, GLfloat s ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord1fv(__GLcontext *gc,  GLenum target, const GLfloat *v ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord1i(__GLcontext *gc,  GLenum target, GLint s ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord1iv(__GLcontext *gc,  GLenum target, const GLint *v ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord1s(__GLcontext *gc,  GLenum target, GLshort s ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord1sv(__GLcontext *gc,  GLenum target, const GLshort *v ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord2d(__GLcontext *gc,  GLenum target, GLdouble s, GLdouble t ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord2dv(__GLcontext *gc,  GLenum target, const GLdouble *v ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord2f(__GLcontext *gc,  GLenum target, GLfloat s, GLfloat t ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord2fv(__GLcontext *gc,  GLenum target, const GLfloat *v ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord2i(__GLcontext *gc,  GLenum target, GLint s, GLint t ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord2iv(__GLcontext *gc,  GLenum target, const GLint *v ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord2s(__GLcontext *gc,  GLenum target, GLshort s, GLshort t ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord2sv(__GLcontext *gc,  GLenum target, const GLshort *v ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord3d(__GLcontext *gc,  GLenum target, GLdouble s, GLdouble t, GLdouble r ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord3dv(__GLcontext *gc,  GLenum target, const GLdouble *v ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord3f(__GLcontext *gc,  GLenum target, GLfloat s, GLfloat t, GLfloat r ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord3fv(__GLcontext *gc,  GLenum target, const GLfloat *v ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord3i(__GLcontext *gc,  GLenum target, GLint s, GLint t, GLint r ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord3iv(__GLcontext *gc,  GLenum target, const GLint *v ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord3s(__GLcontext *gc,  GLenum target, GLshort s, GLshort t, GLshort r ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord3sv(__GLcontext *gc,  GLenum target, const GLshort *v ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord4d(__GLcontext *gc,  GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord4dv(__GLcontext *gc,  GLenum target, const GLdouble *v ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord4f(__GLcontext *gc,  GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord4fv(__GLcontext *gc,  GLenum target, const GLfloat *v ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord4i(__GLcontext *gc,  GLenum target, GLint s, GLint t, GLint r, GLint q ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord4iv(__GLcontext *gc,  GLenum target, const GLint *v ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord4s(__GLcontext *gc,  GLenum target, GLshort s, GLshort t, GLshort r, GLshort q ){}
GLvoid GL_APIENTRY __glnop_MultiTexCoord4sv(__GLcontext *gc,  GLenum target, const GLshort *v ){}
GLvoid GL_APIENTRY __glnop_LoadTransposeMatrixd(__GLcontext *gc,  const GLdouble m[16] ){}
GLvoid GL_APIENTRY __glnop_LoadTransposeMatrixf(__GLcontext *gc,  const GLfloat m[16] ){}
GLvoid GL_APIENTRY __glnop_MultTransposeMatrixd(__GLcontext *gc,  const GLdouble m[16] ){}
GLvoid GL_APIENTRY __glnop_MultTransposeMatrixf(__GLcontext *gc,  const GLfloat m[16] ){}
GLvoid GL_APIENTRY __glnop_CompressedTexImage1D(__GLcontext *gc,  GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data ){}
GLvoid GL_APIENTRY __glnop_CompressedTexSubImage1D(__GLcontext *gc,  GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data ){}
GLvoid GL_APIENTRY __glnop_GetCompressedTexImage(__GLcontext *gc,  GLenum target, GLint lod, GLvoid *img ){}
GLvoid GL_APIENTRY __glnop_FogCoordf(__GLcontext *gc, GLfloat coord){}
GLvoid GL_APIENTRY __glnop_FogCoordfv(__GLcontext *gc, const GLfloat *coord){}
GLvoid GL_APIENTRY __glnop_FogCoordd(__GLcontext *gc, GLdouble coord){}
GLvoid GL_APIENTRY __glnop_FogCoorddv(__GLcontext *gc, const GLdouble *coord){}
GLvoid GL_APIENTRY __glnop_FogCoordPointer(__GLcontext *gc, GLenum type, GLsizei stride, const void *pointer){}
GLvoid GL_APIENTRY __glnop_PointParameterf(__GLcontext *gc, GLenum pname, GLfloat param){}
GLvoid GL_APIENTRY __glnop_PointParameterfv(__GLcontext *gc, GLenum pname, const GLfloat *params){}
GLvoid GL_APIENTRY __glnop_PointParameteri(__GLcontext *gc, GLenum pname, GLint param){}
GLvoid GL_APIENTRY __glnop_PointParameteriv(__GLcontext *gc, GLenum pname, const GLint *params){}
GLvoid GL_APIENTRY __glnop_SecondaryColor3b(__GLcontext *gc, GLbyte red, GLbyte green, GLbyte blue){}
GLvoid GL_APIENTRY __glnop_SecondaryColor3bv(__GLcontext *gc, const GLbyte *v){}
GLvoid GL_APIENTRY __glnop_SecondaryColor3d(__GLcontext *gc, GLdouble red, GLdouble green, GLdouble blue){}
GLvoid GL_APIENTRY __glnop_SecondaryColor3dv(__GLcontext *gc, const GLdouble *v){}
GLvoid GL_APIENTRY __glnop_SecondaryColor3f(__GLcontext *gc, GLfloat red, GLfloat green, GLfloat blue){}
GLvoid GL_APIENTRY __glnop_SecondaryColor3fv(__GLcontext *gc, const GLfloat *v){}
GLvoid GL_APIENTRY __glnop_SecondaryColor3i(__GLcontext *gc, GLint red, GLint green, GLint blue){}
GLvoid GL_APIENTRY __glnop_SecondaryColor3iv(__GLcontext *gc, const GLint *v){}
GLvoid GL_APIENTRY __glnop_SecondaryColor3s(__GLcontext *gc, GLshort red, GLshort green, GLshort blue){}
GLvoid GL_APIENTRY __glnop_SecondaryColor3sv(__GLcontext *gc, const GLshort *v){}
GLvoid GL_APIENTRY __glnop_SecondaryColor3ub(__GLcontext *gc, GLubyte red, GLubyte green, GLubyte blue){}
GLvoid GL_APIENTRY __glnop_SecondaryColor3ubv(__GLcontext *gc, const GLubyte *v){}
GLvoid GL_APIENTRY __glnop_SecondaryColor3ui(__GLcontext *gc, GLuint red, GLuint green, GLuint blue){}
GLvoid GL_APIENTRY __glnop_SecondaryColor3uiv(__GLcontext *gc, const GLuint *v){}
GLvoid GL_APIENTRY __glnop_SecondaryColor3us(__GLcontext *gc, GLushort red, GLushort green, GLushort blue){}
GLvoid GL_APIENTRY __glnop_SecondaryColor3usv(__GLcontext *gc, const GLushort *v){}
GLvoid GL_APIENTRY __glnop_SecondaryColorPointer(__GLcontext *gc, GLint size, GLenum type, GLsizei stride, const void *pointer){}
GLvoid GL_APIENTRY __glnop_WindowPos2d(__GLcontext *gc, GLdouble x, GLdouble y){}
GLvoid GL_APIENTRY __glnop_WindowPos2dv(__GLcontext *gc, const GLdouble *v){}
GLvoid GL_APIENTRY __glnop_WindowPos2f(__GLcontext *gc, GLfloat x, GLfloat y){}
GLvoid GL_APIENTRY __glnop_WindowPos2fv(__GLcontext *gc, const GLfloat *v){}
GLvoid GL_APIENTRY __glnop_WindowPos2i(__GLcontext *gc, GLint x, GLint y){}
GLvoid GL_APIENTRY __glnop_WindowPos2iv(__GLcontext *gc, const GLint *v){}
GLvoid GL_APIENTRY __glnop_WindowPos2s(__GLcontext *gc, GLshort x, GLshort y){}
GLvoid GL_APIENTRY __glnop_WindowPos2sv(__GLcontext *gc, const GLshort *v){}
GLvoid GL_APIENTRY __glnop_WindowPos3d(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z){}
GLvoid GL_APIENTRY __glnop_WindowPos3dv(__GLcontext *gc, const GLdouble *v){}
GLvoid GL_APIENTRY __glnop_WindowPos3f(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z){}
GLvoid GL_APIENTRY __glnop_WindowPos3fv(__GLcontext *gc, const GLfloat *v){}
GLvoid GL_APIENTRY __glnop_WindowPos3i(__GLcontext *gc, GLint x, GLint y, GLint z){}
GLvoid GL_APIENTRY __glnop_WindowPos3iv(__GLcontext *gc, const GLint *v){}
GLvoid GL_APIENTRY __glnop_WindowPos3s(__GLcontext *gc, GLshort x, GLshort y, GLshort z){}
GLvoid GL_APIENTRY __glnop_WindowPos3sv(__GLcontext *gc, const GLshort *v){}
GLvoid GL_APIENTRY __glnop_DrawPixels(__GLcontext *gc,  GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels ){}
GLvoid GL_APIENTRY __glnop_CopyPixels(__GLcontext *gc,  GLint x, GLint y,  GLsizei width, GLsizei height, GLenum type ){}
GLvoid GL_APIENTRY __glnop_Indexub(__GLcontext *gc,  GLubyte c ){}
GLvoid GL_APIENTRY __glnop_Indexubv(__GLcontext *gc,  const GLubyte *c ){}

/* OpenGL ES 2.0 */

GLvoid GL_APIENTRY __glnop_ActiveTexture(__GLcontext *gc, GLenum texture){}
GLvoid GL_APIENTRY __glnop_AttachShader(__GLcontext *gc, GLuint program, GLuint shader){}
GLvoid GL_APIENTRY __glnop_BindAttribLocation(__GLcontext *gc, GLuint program, GLuint index, const GLchar* name){}
GLvoid GL_APIENTRY __glnop_BindBuffer(__GLcontext *gc, GLenum target, GLuint buffer){}
GLvoid GL_APIENTRY __glnop_BindFramebuffer(__GLcontext *gc, GLenum target, GLuint framebuffer){}
GLvoid GL_APIENTRY __glnop_BindRenderbuffer(__GLcontext *gc, GLenum target, GLuint renderbuffer){}
GLvoid GL_APIENTRY __glnop_BindTexture(__GLcontext *gc, GLenum target, GLuint texture){}
GLvoid GL_APIENTRY __glnop_BlendColor(__GLcontext *gc, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha){}
GLvoid GL_APIENTRY __glnop_BlendEquation(__GLcontext *gc, GLenum mode){}
GLvoid GL_APIENTRY __glnop_BlendEquationSeparate(__GLcontext *gc, GLenum modeRGB, GLenum modeAlpha){}
GLvoid GL_APIENTRY __glnop_BlendFunc(__GLcontext *gc, GLenum sfactor, GLenum dfactor){}
GLvoid GL_APIENTRY __glnop_BlendFuncSeparate(__GLcontext *gc, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha){}
GLvoid GL_APIENTRY __glnop_BufferData(__GLcontext *gc, GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage){}
GLvoid GL_APIENTRY __glnop_BufferSubData(__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data){}
GLenum GL_APIENTRY __glnop_CheckFramebufferStatus(__GLcontext *gc, GLenum target){return 0;}
GLvoid GL_APIENTRY __glnop_Clear(__GLcontext *gc, GLbitfield mask){}
GLvoid GL_APIENTRY __glnop_ClearColor(__GLcontext *gc, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha){}
GLvoid GL_APIENTRY __glnop_ClearDepthf(__GLcontext *gc, GLfloat depth){}
GLvoid GL_APIENTRY __glnop_ClearStencil(__GLcontext *gc, GLint s){}
GLvoid GL_APIENTRY __glnop_ColorMask(__GLcontext *gc, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha){}
GLvoid GL_APIENTRY __glnop_CompileShader(__GLcontext *gc, GLuint shader){}
GLvoid GL_APIENTRY __glnop_CompressedTexImage2D(__GLcontext *gc, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data){}
GLvoid GL_APIENTRY __glnop_CompressedTexSubImage2D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data){}
GLvoid GL_APIENTRY __glnop_CopyTexImage2D(__GLcontext *gc, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border){}
GLvoid GL_APIENTRY __glnop_CopyTexSubImage2D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height){}
GLuint GL_APIENTRY __glnop_CreateProgram(__GLcontext *gc){return 0;}
GLuint GL_APIENTRY __glnop_CreateShader(__GLcontext *gc, GLenum type){return 0;}
GLvoid GL_APIENTRY __glnop_CullFace(__GLcontext *gc, GLenum mode){}
GLvoid GL_APIENTRY __glnop_DeleteBuffers(__GLcontext *gc, GLsizei n, const GLuint* buffers){}
GLvoid GL_APIENTRY __glnop_DeleteFramebuffers(__GLcontext *gc, GLsizei n, const GLuint* framebuffers){}
GLvoid GL_APIENTRY __glnop_DeleteProgram(__GLcontext *gc, GLuint program){}
GLvoid GL_APIENTRY __glnop_DeleteRenderbuffers(__GLcontext *gc, GLsizei n, const GLuint* renderbuffers){}
GLvoid GL_APIENTRY __glnop_DeleteShader(__GLcontext *gc, GLuint shader){}
GLvoid GL_APIENTRY __glnop_DeleteTextures(__GLcontext *gc, GLsizei n, const GLuint* textures){}
GLvoid GL_APIENTRY __glnop_DepthFunc(__GLcontext *gc, GLenum func){}
GLvoid GL_APIENTRY __glnop_DepthMask(__GLcontext *gc, GLboolean flag){}
GLvoid GL_APIENTRY __glnop_DepthRangef(__GLcontext *gc, GLfloat n, GLfloat f){}
GLvoid GL_APIENTRY __glnop_DetachShader(__GLcontext *gc, GLuint program, GLuint shader){}
GLvoid GL_APIENTRY __glnop_Disable(__GLcontext *gc, GLenum cap){}
GLvoid GL_APIENTRY __glnop_DisableVertexAttribArray(__GLcontext *gc, GLuint index){}
GLvoid GL_APIENTRY __glnop_DrawArrays(__GLcontext *gc, GLenum mode, GLint first, GLsizei count){}
GLvoid GL_APIENTRY __glnop_DrawElements(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid* indices){}
GLvoid GL_APIENTRY __glnop_Enable(__GLcontext *gc, GLenum cap){}
GLvoid GL_APIENTRY __glnop_EnableVertexAttribArray(__GLcontext *gc, GLuint index){}
GLvoid GL_APIENTRY __glnop_Finish(__GLcontext *gc){}
GLvoid GL_APIENTRY __glnop_Flush(__GLcontext *gc){}
GLvoid GL_APIENTRY __glnop_FramebufferRenderbuffer(__GLcontext *gc, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer){}
GLvoid GL_APIENTRY __glnop_FramebufferTexture2D(__GLcontext *gc, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level){}
GLvoid GL_APIENTRY __glnop_FrontFace(__GLcontext *gc, GLenum mode){}
GLvoid GL_APIENTRY __glnop_GenBuffers(__GLcontext *gc, GLsizei n, GLuint* buffers){}
GLvoid GL_APIENTRY __glnop_GenerateMipmap(__GLcontext *gc, GLenum target){}
GLvoid GL_APIENTRY __glnop_GenFramebuffers(__GLcontext *gc, GLsizei n, GLuint* framebuffers){}
GLvoid GL_APIENTRY __glnop_GenRenderbuffers(__GLcontext *gc, GLsizei n, GLuint* renderbuffers){}
GLvoid GL_APIENTRY __glnop_GenTextures(__GLcontext *gc, GLsizei n, GLuint* textures){}
GLvoid GL_APIENTRY __glnop_GetActiveAttrib(__GLcontext *gc, GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name){}
GLvoid GL_APIENTRY __glnop_GetActiveUniform(__GLcontext *gc, GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name){}
GLvoid GL_APIENTRY __glnop_GetAttachedShaders(__GLcontext *gc, GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders){}
GLint  GL_APIENTRY __glnop_GetAttribLocation(__GLcontext *gc, GLuint program, const GLchar* name){return -1;}
GLvoid GL_APIENTRY __glnop_GetBooleanv(__GLcontext *gc, GLenum pname, GLboolean* params){}
GLvoid GL_APIENTRY __glnop_GetBufferParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint* params){}
GLenum GL_APIENTRY __glnop_GetError(__GLcontext *gc){return GL_NO_ERROR;}
GLvoid GL_APIENTRY __glnop_GetFloatv(__GLcontext *gc, GLenum pname, GLfloat* params){}
GLvoid GL_APIENTRY __glnop_GetFramebufferAttachmentParameteriv(__GLcontext *gc, GLenum target, GLenum attachment, GLenum pname, GLint* params){}
GLvoid GL_APIENTRY __glnop_GetIntegerv(__GLcontext *gc, GLenum pname, GLint* params){}
GLvoid GL_APIENTRY __glnop_GetProgramiv(__GLcontext *gc, GLuint program, GLenum pname, GLint* params){}
GLvoid GL_APIENTRY __glnop_GetProgramInfoLog(__GLcontext *gc, GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog){}
GLvoid GL_APIENTRY __glnop_GetRenderbufferParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint* params){}
GLvoid GL_APIENTRY __glnop_GetShaderiv(__GLcontext *gc, GLuint shader, GLenum pname, GLint* params){}
GLvoid GL_APIENTRY __glnop_GetShaderInfoLog(__GLcontext *gc, GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog){}
GLvoid GL_APIENTRY __glnop_GetShaderPrecisionFormat(__GLcontext *gc, GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision){}
GLvoid GL_APIENTRY __glnop_GetShaderSource(__GLcontext *gc, GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source){}
const GLubyte* GL_APIENTRY __glnop_GetString(__GLcontext *gc, GLenum name){ return NULL;}
GLvoid GL_APIENTRY __glnop_GetTexParameterfv(__GLcontext *gc, GLenum target, GLenum pname, GLfloat* params){}
GLvoid GL_APIENTRY __glnop_GetTexParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint* params){}
GLvoid GL_APIENTRY __glnop_GetUniformfv(__GLcontext *gc, GLuint program, GLint location, GLfloat* params){}
GLvoid GL_APIENTRY __glnop_GetUniformiv(__GLcontext *gc, GLuint program, GLint location, GLint* params){}
GLint  GL_APIENTRY __glnop_GetUniformLocation(__GLcontext *gc, GLuint program, const GLchar* name){return 0;}
GLvoid GL_APIENTRY __glnop_GetVertexAttribfv(__GLcontext *gc, GLuint index, GLenum pname, GLfloat* params){}
GLvoid GL_APIENTRY __glnop_GetVertexAttribiv(__GLcontext *gc, GLuint index, GLenum pname, GLint* params){}
GLvoid GL_APIENTRY __glnop_GetVertexAttribPointerv(__GLcontext *gc, GLuint index, GLenum pname, GLvoid** pointer){}
GLvoid GL_APIENTRY __glnop_Hint(__GLcontext *gc, GLenum target, GLenum mode){}
GLboolean GL_APIENTRY __glnop_IsBuffer(__GLcontext *gc, GLuint buffer){return GL_FALSE;}
GLboolean GL_APIENTRY __glnop_IsEnabled(__GLcontext *gc, GLenum cap){return GL_FALSE;}
GLboolean GL_APIENTRY __glnop_IsFramebuffer(__GLcontext *gc, GLuint framebuffer){return GL_FALSE;}
GLboolean GL_APIENTRY __glnop_IsProgram(__GLcontext *gc, GLuint program){return GL_FALSE;}
GLboolean GL_APIENTRY __glnop_IsRenderbuffer(__GLcontext *gc, GLuint renderbuffer){return GL_FALSE;}
GLboolean GL_APIENTRY __glnop_IsShader(__GLcontext *gc, GLuint shader){return GL_FALSE;}
GLboolean GL_APIENTRY __glnop_IsTexture(__GLcontext *gc, GLuint texture){return GL_FALSE;}
GLvoid GL_APIENTRY __glnop_LineWidth(__GLcontext *gc, GLfloat width){}
GLvoid GL_APIENTRY __glnop_LinkProgram(__GLcontext *gc, GLuint program){}
GLvoid GL_APIENTRY __glnop_PixelStorei(__GLcontext *gc, GLenum pname, GLint param){}
GLvoid GL_APIENTRY __glnop_PolygonOffset(__GLcontext *gc, GLfloat factor, GLfloat units){}
GLvoid GL_APIENTRY __glnop_ReadPixels(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels){}
GLvoid GL_APIENTRY __glnop_ReleaseShaderCompiler(__GLcontext *gc){}
GLvoid GL_APIENTRY __glnop_RenderbufferStorage(__GLcontext *gc, GLenum target, GLenum internalformat, GLsizei width, GLsizei height){}
GLvoid GL_APIENTRY __glnop_SampleCoverage(__GLcontext *gc, GLfloat value, GLboolean invert){}
GLvoid GL_APIENTRY __glnop_Scissor(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height){}
GLvoid GL_APIENTRY __glnop_ShaderBinary(__GLcontext *gc, GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length){}
GLvoid GL_APIENTRY __glnop_ShaderSource(__GLcontext *gc, GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length){}
GLvoid GL_APIENTRY __glnop_StencilFunc(__GLcontext *gc, GLenum func, GLint ref, GLuint mask){}
GLvoid GL_APIENTRY __glnop_StencilFuncSeparate(__GLcontext *gc, GLenum face, GLenum func, GLint ref, GLuint mask){}
GLvoid GL_APIENTRY __glnop_StencilMask(__GLcontext *gc, GLuint mask){}
GLvoid GL_APIENTRY __glnop_StencilMaskSeparate(__GLcontext *gc, GLenum face, GLuint mask){}
GLvoid GL_APIENTRY __glnop_StencilOp(__GLcontext *gc, GLenum fail, GLenum zfail, GLenum zpass){}
GLvoid GL_APIENTRY __glnop_StencilOpSeparate(__GLcontext *gc, GLenum face, GLenum fail, GLenum zfail, GLenum zpass){}
GLvoid GL_APIENTRY __glnop_TexImage2D(__GLcontext *gc, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels){}
GLvoid GL_APIENTRY __glnop_TexParameterf(__GLcontext *gc, GLenum target, GLenum pname, GLfloat param){}
GLvoid GL_APIENTRY __glnop_TexParameterfv(__GLcontext *gc, GLenum target, GLenum pname, const GLfloat* params){}
GLvoid GL_APIENTRY __glnop_TexParameteri(__GLcontext *gc, GLenum target, GLenum pname, GLint param){}
GLvoid GL_APIENTRY __glnop_TexParameteriv(__GLcontext *gc, GLenum target, GLenum pname, const GLint* params){}
GLvoid GL_APIENTRY __glnop_TexSubImage2D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels){}
GLvoid GL_APIENTRY __glnop_Uniform1f(__GLcontext *gc, GLint location, GLfloat x){}
GLvoid GL_APIENTRY __glnop_Uniform1fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat* v){}
GLvoid GL_APIENTRY __glnop_Uniform1i(__GLcontext *gc, GLint location, GLint x){}
GLvoid GL_APIENTRY __glnop_Uniform1iv(__GLcontext *gc, GLint location, GLsizei count, const GLint* v){}
GLvoid GL_APIENTRY __glnop_Uniform2f(__GLcontext *gc, GLint location, GLfloat x, GLfloat y){}
GLvoid GL_APIENTRY __glnop_Uniform2fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat* v){}
GLvoid GL_APIENTRY __glnop_Uniform2i(__GLcontext *gc, GLint location, GLint x, GLint y){}
GLvoid GL_APIENTRY __glnop_Uniform2iv(__GLcontext *gc, GLint location, GLsizei count, const GLint* v){}
GLvoid GL_APIENTRY __glnop_Uniform3f(__GLcontext *gc, GLint location, GLfloat x, GLfloat y, GLfloat z){}
GLvoid GL_APIENTRY __glnop_Uniform3fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat* v){}
GLvoid GL_APIENTRY __glnop_Uniform3i(__GLcontext *gc, GLint location, GLint x, GLint y, GLint z){}
GLvoid GL_APIENTRY __glnop_Uniform3iv(__GLcontext *gc, GLint location, GLsizei count, const GLint* v){}
GLvoid GL_APIENTRY __glnop_Uniform4f(__GLcontext *gc, GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w){}
GLvoid GL_APIENTRY __glnop_Uniform4fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat* v){}
GLvoid GL_APIENTRY __glnop_Uniform4i(__GLcontext *gc, GLint location, GLint x, GLint y, GLint z, GLint w){}
GLvoid GL_APIENTRY __glnop_Uniform4iv(__GLcontext *gc, GLint location, GLsizei count, const GLint* v){}
GLvoid GL_APIENTRY __glnop_UniformMatrix2fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value){}
GLvoid GL_APIENTRY __glnop_UniformMatrix3fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value){}
GLvoid GL_APIENTRY __glnop_UniformMatrix4fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value){}
GLvoid GL_APIENTRY __glnop_UseProgram(__GLcontext *gc, GLuint program){}
GLvoid GL_APIENTRY __glnop_ValidateProgram(__GLcontext *gc, GLuint program){}
GLvoid GLAPIENTRY __glnop_VertexAttrib1d(__GLcontext *gc, GLuint indx, GLdouble x){}
GLvoid GLAPIENTRY __glnop_VertexAttrib1dv(__GLcontext *gc, GLuint indx, const GLdouble *values){}
GLvoid GL_APIENTRY __glnop_VertexAttrib1f(__GLcontext *gc, GLuint indx, GLfloat x){}
GLvoid GL_APIENTRY __glnop_VertexAttrib1fv(__GLcontext *gc, GLuint indx, const GLfloat* values){}
GLvoid GLAPIENTRY __glnop_VertexAttrib1s(__GLcontext *gc, GLuint indx, GLshort x){}
GLvoid GLAPIENTRY __glnop_VertexAttrib1sv(__GLcontext *gc, GLuint indx, const GLshort * values){}
GLvoid GLAPIENTRY __glnop_VertexAttrib2d(__GLcontext *gc, GLuint indx, GLdouble x, GLdouble y){}
GLvoid GLAPIENTRY __glnop_VertexAttrib2dv(__GLcontext *gc, GLuint indx, const GLdouble *values){}
GLvoid GL_APIENTRY __glnop_VertexAttrib2f(__GLcontext *gc, GLuint indx, GLfloat x, GLfloat y){}
GLvoid GL_APIENTRY __glnop_VertexAttrib2fv(__GLcontext *gc, GLuint indx, const GLfloat* values){}
GLvoid GLAPIENTRY __glnop_VertexAttrib2s(__GLcontext *gc, GLuint indx, GLshort x, GLshort y){}
GLvoid GLAPIENTRY __glnop_VertexAttrib2sv(__GLcontext *gc, GLuint indx, const GLshort * values){}
GLvoid GLAPIENTRY __glnop_VertexAttrib3d(__GLcontext *gc, GLuint indx, GLdouble x, GLdouble y, GLdouble z){}
GLvoid GLAPIENTRY __glnop_VertexAttrib3dv(__GLcontext *gc, GLuint indx, const GLdouble *values){}
GLvoid GL_APIENTRY __glnop_VertexAttrib3f(__GLcontext *gc, GLuint indx, GLfloat x, GLfloat y, GLfloat z){}
GLvoid GL_APIENTRY __glnop_VertexAttrib3fv(__GLcontext *gc, GLuint indx, const GLfloat* values){}
GLvoid GLAPIENTRY __glnop_VertexAttrib3s(__GLcontext *gc, GLuint indx, GLshort x, GLshort y, GLshort z){}
GLvoid GLAPIENTRY __glnop_VertexAttrib3sv(__GLcontext *gc, GLuint indx, const GLshort * values){}
GLvoid GLAPIENTRY __glnop_VertexAttrib4Nbv(__GLcontext *gc, GLuint indx, const GLbyte * values){}
GLvoid GLAPIENTRY __glnop_VertexAttrib4Niv(__GLcontext *gc, GLuint indx, const GLint * values){}
GLvoid GLAPIENTRY __glnop_VertexAttrib4Nsv(__GLcontext *gc, GLuint indx, const GLshort * values){}
GLvoid GLAPIENTRY __glnop_VertexAttrib4Nub(__GLcontext *gc, GLuint indx, GLubyte x, GLubyte y, GLubyte z, GLubyte w){}
GLvoid GLAPIENTRY __glnop_VertexAttrib4Nubv(__GLcontext *gc, GLuint indx, const GLubyte * values){}
GLvoid GLAPIENTRY __glnop_VertexAttrib4Nuiv(__GLcontext *gc, GLuint indx, const GLuint * values){}
GLvoid GLAPIENTRY __glnop_VertexAttrib4Nusv(__GLcontext *gc, GLuint indx, const GLushort * values){}
GLvoid GLAPIENTRY __glnop_VertexAttrib4bv(__GLcontext *gc, GLuint indx, const GLbyte * values){}
GLvoid GLAPIENTRY __glnop_VertexAttrib4d(__GLcontext *gc, GLuint indx, GLdouble x, GLdouble y, GLdouble z, GLdouble w){}
GLvoid GLAPIENTRY __glnop_VertexAttrib4dv(__GLcontext *gc, GLuint indx, const GLdouble *values){}
GLvoid GL_APIENTRY __glnop_VertexAttrib4f(__GLcontext *gc, GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w){}
GLvoid GL_APIENTRY __glnop_VertexAttrib4fv(__GLcontext *gc, GLuint indx, const GLfloat* values){}
GLvoid GLAPIENTRY __glnop_VertexAttrib4iv(__GLcontext *gc, GLuint indx, const GLint * values){}
GLvoid GLAPIENTRY __glnop_VertexAttrib4s(__GLcontext *gc, GLuint indx, GLshort x, GLshort y, GLshort z, GLshort w){}
GLvoid GLAPIENTRY __glnop_VertexAttrib4sv(__GLcontext *gc, GLuint indx, const GLshort * values){}
GLvoid GLAPIENTRY __glnop_VertexAttrib4ubv(__GLcontext *gc, GLuint indx, const GLubyte * values){}
GLvoid GLAPIENTRY __glnop_VertexAttrib4uiv(__GLcontext *gc, GLuint indx, const GLuint * values){}
GLvoid GLAPIENTRY __glnop_VertexAttrib4usv(__GLcontext *gc, GLuint indx, const GLushort * values){}
GLvoid GL_APIENTRY __glnop_VertexAttribPointer(__GLcontext *gc, GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr){}
GLvoid GL_APIENTRY __glnop_Viewport(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height){}

/* OpenGL ES 3.0 */

GLvoid GL_APIENTRY __glnop_ReadBuffer(__GLcontext *gc, GLenum mode){}
GLvoid GL_APIENTRY __glnop_DrawRangeElements(__GLcontext *gc, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices){}
GLvoid GL_APIENTRY __glnop_TexImage3D(__GLcontext *gc, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels){}
GLvoid GL_APIENTRY __glnop_TexSubImage3D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels){}
GLvoid GL_APIENTRY __glnop_CopyTexSubImage3D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height){}
GLvoid GL_APIENTRY __glnop_CompressedTexImage3D(__GLcontext *gc, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data){}
GLvoid GL_APIENTRY __glnop_CompressedTexSubImage3D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data){}
GLvoid GL_APIENTRY __glnop_GenQueries(__GLcontext *gc, GLsizei n, GLuint* ids){}
GLvoid GL_APIENTRY __glnop_DeleteQueries(__GLcontext *gc, GLsizei n, const GLuint* ids){}
GLboolean GL_APIENTRY __glnop_IsQuery(__GLcontext *gc, GLuint id){return GL_FALSE;}
GLvoid GL_APIENTRY __glnop_BeginQuery(__GLcontext *gc, GLenum target, GLuint id){}
GLvoid GL_APIENTRY __glnop_EndQuery(__GLcontext *gc, GLenum target){}
GLvoid GL_APIENTRY __glnop_GetQueryiv(__GLcontext *gc, GLenum target, GLenum pname, GLint* params){}
GLvoid GL_APIENTRY __glnop_GetQueryObjectuiv(__GLcontext *gc, GLuint id, GLenum pname, GLuint* params){}
GLboolean GL_APIENTRY __glnop_UnmapBuffer(__GLcontext *gc, GLenum target){return GL_FALSE;}
GLvoid GL_APIENTRY __glnop_GetBufferPointerv(__GLcontext *gc, GLenum target, GLenum pname, GLvoid** params){}
GLvoid GL_APIENTRY __glnop_DrawBuffers(__GLcontext *gc, GLsizei n, const GLenum* bufs){}
GLvoid GL_APIENTRY __glnop_UniformMatrix2x3fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value){}
GLvoid GL_APIENTRY __glnop_UniformMatrix3x2fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value){}
GLvoid GL_APIENTRY __glnop_UniformMatrix2x4fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value){}
GLvoid GL_APIENTRY __glnop_UniformMatrix4x2fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value){}
GLvoid GL_APIENTRY __glnop_UniformMatrix3x4fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value){}
GLvoid GL_APIENTRY __glnop_UniformMatrix4x3fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value){}
GLvoid GL_APIENTRY __glnop_BlitFramebuffer(__GLcontext *gc, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter){}
GLvoid GL_APIENTRY __glnop_RenderbufferStorageMultisample(__GLcontext *gc, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height){}
GLvoid GL_APIENTRY __glnop_FramebufferTextureLayer(__GLcontext *gc, GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer){}
GLvoid* GL_APIENTRY __glnop_MapBufferRange(__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access){return NULL;}
GLvoid GL_APIENTRY __glnop_FlushMappedBufferRange(__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr length){}
GLvoid GL_APIENTRY __glnop_BindVertexArray(__GLcontext *gc, GLuint array){}
GLvoid GL_APIENTRY __glnop_DeleteVertexArrays(__GLcontext *gc, GLsizei n, const GLuint* arrays){}
GLvoid GL_APIENTRY __glnop_GenVertexArrays(__GLcontext *gc, GLsizei n, GLuint* arrays){}
GLboolean GL_APIENTRY __glnop_IsVertexArray(__GLcontext *gc, GLuint array){return GL_FALSE;}
GLvoid GL_APIENTRY __glnop_GetIntegeri_v(__GLcontext *gc, GLenum target, GLuint index, GLint* data){}
GLvoid GL_APIENTRY __glnop_BeginTransformFeedback(__GLcontext *gc, GLenum primitiveMode){}
GLvoid GL_APIENTRY __glnop_EndTransformFeedback(__GLcontext *gc){}
GLvoid GL_APIENTRY __glnop_BindBufferRange(__GLcontext *gc, GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size){}
GLvoid GL_APIENTRY __glnop_BindBufferBase(__GLcontext *gc, GLenum target, GLuint index, GLuint buffer){}
GLvoid GL_APIENTRY __glnop_TransformFeedbackVaryings(__GLcontext *gc, GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode){}
GLvoid GL_APIENTRY __glnop_GetTransformFeedbackVarying(__GLcontext *gc, GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name){}
GLvoid GL_APIENTRY __glnop_VertexAttribIPointer(__GLcontext *gc, GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer){}
GLvoid GL_APIENTRY __glnop_GetVertexAttribIiv(__GLcontext *gc, GLuint index, GLenum pname, GLint* params){}
GLvoid GL_APIENTRY __glnop_GetVertexAttribIuiv(__GLcontext *gc, GLuint index, GLenum pname, GLuint* params){}
GLvoid GL_APIENTRY __glnop_VertexAttribI4i(__GLcontext *gc, GLuint index, GLint x, GLint y, GLint z, GLint w){}
GLvoid GL_APIENTRY __glnop_VertexAttribI4ui(__GLcontext *gc, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w){}
GLvoid GL_APIENTRY __glnop_VertexAttribI4iv(__GLcontext *gc, GLuint index, const GLint* v){}
GLvoid GL_APIENTRY __glnop_VertexAttribI4uiv(__GLcontext *gc, GLuint index, const GLuint* v){}
GLvoid GL_APIENTRY __glnop_GetUniformuiv(__GLcontext *gc, GLuint program, GLint location, GLuint* params){}
GLint  GL_APIENTRY __glnop_GetFragDataLocation(__GLcontext *gc, GLuint program, const GLchar *name){return -1;}
GLvoid GL_APIENTRY __glnop_BindFragDataLocation(__GLcontext *gc, GLuint program, GLuint colorNumber, const GLchar *name){}
GLvoid GL_APIENTRY __glnop_Uniform1ui(__GLcontext *gc, GLint location, GLuint v0){}
GLvoid GL_APIENTRY __glnop_Uniform2ui(__GLcontext *gc, GLint location, GLuint v0, GLuint v1){}
GLvoid GL_APIENTRY __glnop_Uniform3ui(__GLcontext *gc, GLint location, GLuint v0, GLuint v1, GLuint v2){}
GLvoid GL_APIENTRY __glnop_Uniform4ui(__GLcontext *gc, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3){}
GLvoid GL_APIENTRY __glnop_Uniform1uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint* value){}
GLvoid GL_APIENTRY __glnop_Uniform2uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint* value){}
GLvoid GL_APIENTRY __glnop_Uniform3uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint* value){}
GLvoid GL_APIENTRY __glnop_Uniform4uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint* value){}
GLvoid GL_APIENTRY __glnop_ClearBufferiv(__GLcontext *gc, GLenum buffer, GLint drawbuffer, const GLint* value){}
GLvoid GL_APIENTRY __glnop_ClearBufferuiv(__GLcontext *gc, GLenum buffer, GLint drawbuffer, const GLuint* value){}
GLvoid GL_APIENTRY __glnop_ClearBufferfv(__GLcontext *gc, GLenum buffer, GLint drawbuffer, const GLfloat* value){}
GLvoid GL_APIENTRY __glnop_ClearBufferfi(__GLcontext *gc, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil){}
const GLubyte* GL_APIENTRY __glnop_GetStringi(__GLcontext *gc, GLenum name, GLuint index){return NULL;}
GLvoid GL_APIENTRY __glnop_CopyBufferSubData(__GLcontext *gc, GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size){}
GLvoid GL_APIENTRY __glnop_GetUniformIndices(__GLcontext *gc, GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices){}
GLvoid GL_APIENTRY __glnop_GetActiveUniformsiv(__GLcontext *gc, GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params){}
GLuint GL_APIENTRY __glnop_GetUniformBlockIndex(__GLcontext *gc, GLuint program, const GLchar* uniformBlockName){return 0;}
GLvoid GL_APIENTRY __glnop_GetActiveUniformBlockiv(__GLcontext *gc, GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params){}
GLvoid GL_APIENTRY __glnop_GetActiveUniformBlockName(__GLcontext *gc, GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName){}
GLvoid GL_APIENTRY __glnop_UniformBlockBinding(__GLcontext *gc, GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding){}
GLvoid GL_APIENTRY __glnop_DrawArraysInstanced(__GLcontext *gc, GLenum mode, GLint first, GLsizei count, GLsizei instanceCount){}
GLvoid GL_APIENTRY __glnop_DrawElementsInstanced(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount){}
GLsync GL_APIENTRY __glnop_FenceSync(__GLcontext *gc, GLenum condition, GLbitfield flags){return NULL;}
GLboolean GL_APIENTRY __glnop_IsSync(__GLcontext *gc, GLsync sync){return GL_FALSE;}
GLvoid GL_APIENTRY __glnop_DeleteSync(__GLcontext *gc, GLsync sync){}
GLenum GL_APIENTRY __glnop_ClientWaitSync(__GLcontext *gc, GLsync sync, GLbitfield flags, GLuint64 timeout){return 0;}
GLvoid GL_APIENTRY __glnop_WaitSync(__GLcontext *gc, GLsync sync, GLbitfield flags, GLuint64 timeout){}
GLvoid GL_APIENTRY __glnop_GetInteger64v(__GLcontext *gc, GLenum pname, GLint64* params){}
GLvoid GL_APIENTRY __glnop_GetSynciv(__GLcontext *gc, GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values){}
GLvoid GL_APIENTRY __glnop_GetInteger64i_v(__GLcontext *gc, GLenum target, GLuint index, GLint64* data){}
GLvoid GL_APIENTRY __glnop_GetBufferParameteri64v(__GLcontext *gc, GLenum target, GLenum pname, GLint64* params){}
GLvoid GL_APIENTRY __glnop_GenSamplers(__GLcontext *gc, GLsizei count, GLuint* samplers){}
GLvoid GL_APIENTRY __glnop_DeleteSamplers(__GLcontext *gc, GLsizei count, const GLuint* samplers){}
GLboolean GL_APIENTRY __glnop_IsSampler(__GLcontext *gc, GLuint sampler){return GL_FALSE;}
GLvoid GL_APIENTRY __glnop_BindSampler(__GLcontext *gc, GLuint unit, GLuint sampler){}
GLvoid GL_APIENTRY __glnop_SamplerParameteri(__GLcontext *gc, GLuint sampler, GLenum pname, GLint param){}
GLvoid GL_APIENTRY __glnop_SamplerParameteriv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLint* param){}
GLvoid GL_APIENTRY __glnop_SamplerParameterf(__GLcontext *gc, GLuint sampler, GLenum pname, GLfloat param){}
GLvoid GL_APIENTRY __glnop_SamplerParameterfv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLfloat* param){}
GLvoid GL_APIENTRY __glnop_GetSamplerParameteriv(__GLcontext *gc, GLuint sampler, GLenum pname, GLint* params){}
GLvoid GL_APIENTRY __glnop_GetSamplerParameterfv(__GLcontext *gc, GLuint sampler, GLenum pname, GLfloat* params){}
GLvoid GL_APIENTRY __glnop_VertexAttribDivisor(__GLcontext *gc, GLuint index, GLuint divisor){}
GLvoid GL_APIENTRY __glnop_BindTransformFeedback(__GLcontext *gc, GLenum target, GLuint id){}
GLvoid GL_APIENTRY __glnop_DeleteTransformFeedbacks(__GLcontext *gc, GLsizei n, const GLuint* ids){}
GLvoid GL_APIENTRY __glnop_GenTransformFeedbacks(__GLcontext *gc, GLsizei n, GLuint* ids){}
GLboolean GL_APIENTRY __glnop_IsTransformFeedback(__GLcontext *gc, GLuint id){return GL_FALSE;}
GLvoid GL_APIENTRY __glnop_PauseTransformFeedback(__GLcontext *gc){}
GLvoid GL_APIENTRY __glnop_ResumeTransformFeedback(__GLcontext *gc){}
GLvoid GL_APIENTRY __glnop_GetProgramBinary(__GLcontext *gc, GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary){}
GLvoid GL_APIENTRY __glnop_ProgramBinary(__GLcontext *gc, GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length){}
GLvoid GL_APIENTRY __glnop_ProgramParameteri(__GLcontext *gc, GLuint program, GLenum pname, GLint value){}
GLvoid GL_APIENTRY __glnop_InvalidateFramebuffer(__GLcontext *gc, GLenum target, GLsizei numAttachments, const GLenum* attachments){}
GLvoid GL_APIENTRY __glnop_InvalidateSubFramebuffer(__GLcontext *gc, GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height){}
GLvoid GL_APIENTRY __glnop_TexStorage2D(__GLcontext *gc, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height){}
GLvoid GL_APIENTRY __glnop_TexStorage3D(__GLcontext *gc, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth){}
GLvoid GL_APIENTRY __glnop_GetInternalformativ(__GLcontext *gc, GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params){}

/* OpenGL ES 3.1 */

GLvoid GL_APIENTRY __glnop_DispatchCompute(__GLcontext *gc, GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z){}
GLvoid GL_APIENTRY __glnop_DispatchComputeIndirect(__GLcontext *gc, GLintptr indirect){}
GLvoid GL_APIENTRY __glnop_DrawArraysIndirect(__GLcontext *gc, GLenum mode, const void *indirect){}
GLvoid GL_APIENTRY __glnop_DrawElementsIndirect(__GLcontext *gc, GLenum mode, GLenum type, const void *indirect){}
GLvoid GL_APIENTRY __glnop_FramebufferParameteri(__GLcontext *gc, GLenum target, GLenum pname, GLint param){}
GLvoid GL_APIENTRY __glnop_GetFramebufferParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint *params){}
GLvoid GL_APIENTRY __glnop_GetProgramInterfaceiv(__GLcontext *gc, GLuint program, GLenum programInterface, GLenum pname, GLint *params){}
GLuint GL_APIENTRY __glnop_GetProgramResourceIndex(__GLcontext *gc, GLuint program, GLenum programInterface, const GLchar *name){return 0;}
GLvoid GL_APIENTRY __glnop_GetProgramResourceName(__GLcontext *gc, GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name){}
GLvoid GL_APIENTRY __glnop_GetProgramResourceiv(__GLcontext *gc, GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params){}
GLint GL_APIENTRY __glnop_GetProgramResourceLocation(__GLcontext *gc, GLuint program, GLenum programInterface, const GLchar *name){return -1;}
GLvoid GL_APIENTRY __glnop_UseProgramStages(__GLcontext *gc, GLuint pipeline, GLbitfield stages, GLuint program){}
GLvoid GL_APIENTRY __glnop_ActiveShaderProgram(__GLcontext *gc, GLuint pipeline, GLuint program){}
GLuint GL_APIENTRY __glnop_CreateShaderProgramv(__GLcontext *gc, GLenum type, GLsizei count, const GLchar *const*strings){return 0;}
GLvoid GL_APIENTRY __glnop_BindProgramPipeline(__GLcontext *gc, GLuint pipeline){}
GLvoid GL_APIENTRY __glnop_DeleteProgramPipelines(__GLcontext *gc, GLsizei n, const GLuint *pipelines){}
GLvoid GL_APIENTRY __glnop_GenProgramPipelines(__GLcontext *gc, GLsizei n, GLuint *pipelines){}
GLboolean GL_APIENTRY __glnop_IsProgramPipeline(__GLcontext *gc, GLuint pipeline){return GL_FALSE;}
GLvoid GL_APIENTRY __glnop_GetProgramPipelineiv(__GLcontext *gc, GLuint pipeline, GLenum pname, GLint *params){}
GLvoid GL_APIENTRY __glnop_ProgramUniform1i(__GLcontext *gc, GLuint program, GLint location, GLint v0){}
GLvoid GL_APIENTRY __glnop_ProgramUniform2i(__GLcontext *gc, GLuint program, GLint location, GLint v0, GLint v1){}
GLvoid GL_APIENTRY __glnop_ProgramUniform3i(__GLcontext *gc, GLuint program, GLint location, GLint v0, GLint v1, GLint v2){}
GLvoid GL_APIENTRY __glnop_ProgramUniform4i(__GLcontext *gc, GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3){}
GLvoid GL_APIENTRY __glnop_ProgramUniform1ui(__GLcontext *gc, GLuint program, GLint location, GLuint v0){}
GLvoid GL_APIENTRY __glnop_ProgramUniform2ui(__GLcontext *gc, GLuint program, GLint location, GLuint v0, GLuint v1){}
GLvoid GL_APIENTRY __glnop_ProgramUniform3ui(__GLcontext *gc, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2){}
GLvoid GL_APIENTRY __glnop_ProgramUniform4ui(__GLcontext *gc, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3){}
GLvoid GL_APIENTRY __glnop_ProgramUniform1f(__GLcontext *gc, GLuint program, GLint location, GLfloat v0){}
GLvoid GL_APIENTRY __glnop_ProgramUniform2f(__GLcontext *gc, GLuint program, GLint location, GLfloat v0, GLfloat v1){}
GLvoid GL_APIENTRY __glnop_ProgramUniform3f(__GLcontext *gc, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2){}
GLvoid GL_APIENTRY __glnop_ProgramUniform4f(__GLcontext *gc, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3){}
GLvoid GL_APIENTRY __glnop_ProgramUniform1iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value){}
GLvoid GL_APIENTRY __glnop_ProgramUniform2iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value){}
GLvoid GL_APIENTRY __glnop_ProgramUniform3iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value){}
GLvoid GL_APIENTRY __glnop_ProgramUniform4iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value){}
GLvoid GL_APIENTRY __glnop_ProgramUniform1uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value){}
GLvoid GL_APIENTRY __glnop_ProgramUniform2uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value){}
GLvoid GL_APIENTRY __glnop_ProgramUniform3uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value){}
GLvoid GL_APIENTRY __glnop_ProgramUniform4uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value){}
GLvoid GL_APIENTRY __glnop_ProgramUniform1fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value){}
GLvoid GL_APIENTRY __glnop_ProgramUniform2fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value){}
GLvoid GL_APIENTRY __glnop_ProgramUniform3fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value){}
GLvoid GL_APIENTRY __glnop_ProgramUniform4fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value){}
GLvoid GL_APIENTRY __glnop_ProgramUniformMatrix2fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value){}
GLvoid GL_APIENTRY __glnop_ProgramUniformMatrix3fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value){}
GLvoid GL_APIENTRY __glnop_ProgramUniformMatrix4fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value){}
GLvoid GL_APIENTRY __glnop_ProgramUniformMatrix2x3fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value){}
GLvoid GL_APIENTRY __glnop_ProgramUniformMatrix3x2fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value){}
GLvoid GL_APIENTRY __glnop_ProgramUniformMatrix2x4fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value){}
GLvoid GL_APIENTRY __glnop_ProgramUniformMatrix4x2fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value){}
GLvoid GL_APIENTRY __glnop_ProgramUniformMatrix3x4fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value){}
GLvoid GL_APIENTRY __glnop_ProgramUniformMatrix4x3fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value){}
GLvoid GL_APIENTRY __glnop_ValidateProgramPipeline(__GLcontext *gc, GLuint pipeline){}
GLvoid GL_APIENTRY __glnop_GetProgramPipelineInfoLog(__GLcontext *gc, GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog){}
GLvoid GL_APIENTRY __glnop_BindImageTexture(__GLcontext *gc, GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format){}
GLvoid GL_APIENTRY __glnop_GetBooleani_v(__GLcontext *gc, GLenum target, GLuint index, GLboolean *data){}
GLvoid GL_APIENTRY __glnop_MemoryBarrier(__GLcontext *gc, GLbitfield barriers){}
GLvoid GL_APIENTRY __glnop_MemoryBarrierByRegion(__GLcontext *gc, GLbitfield barriers){}
GLvoid GL_APIENTRY __glnop_TexStorage2DMultisample(__GLcontext *gc, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations){}
GLvoid GL_APIENTRY __glnop_GetMultisamplefv(__GLcontext *gc, GLenum pname, GLuint index, GLfloat *val){}
GLvoid GL_APIENTRY __glnop_SampleMaski(__GLcontext *gc, GLuint maskNumber, GLbitfield mask){}
GLvoid GL_APIENTRY __glnop_GetTexLevelParameteriv(__GLcontext *gc, GLenum target, GLint level, GLenum pname, GLint *params){}
GLvoid GL_APIENTRY __glnop_GetTexLevelParameterfv(__GLcontext *gc, GLenum target, GLint level, GLenum pname, GLfloat *params){}
GLvoid GL_APIENTRY __glnop_BindVertexBuffer(__GLcontext *gc, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride){}
GLvoid GL_APIENTRY __glnop_VertexAttribFormat(__GLcontext *gc, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset){}
GLvoid GL_APIENTRY __glnop_VertexAttribIFormat(__GLcontext *gc, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset){}
GLvoid GL_APIENTRY __glnop_VertexAttribBinding(__GLcontext *gc, GLuint attribindex, GLuint bindingindex){}
GLvoid GL_APIENTRY __glnop_VertexBindingDivisor(__GLcontext *gc, GLuint bindingindex, GLuint divisor){}

/* OpenGL ES 3.2 */
GLvoid GL_APIENTRY __glnop_TexStorage3DMultisample(__GLcontext *gc, GLenum target, GLsizei samples, GLenum sizedinternalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations){}

GLvoid GL_APIENTRY __glnop_BlendBarrier(__GLcontext *gc){}

GLvoid GL_APIENTRY __glnop_DebugMessageControl(__GLcontext *gc, GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled){}
GLvoid GL_APIENTRY __glnop_DebugMessageInsert(__GLcontext *gc, GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf){}
GLvoid GL_APIENTRY __glnop_DebugMessageCallback(__GLcontext *gc, GLDEBUGPROCKHR callback, const GLvoid* userParam){}
GLuint GL_APIENTRY __glnop_GetDebugMessageLog(__GLcontext *gc, GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog){return 0;}
GLvoid GL_APIENTRY __glnop_GetPointerv(__GLcontext *gc, GLenum pname, GLvoid** params){}
GLvoid GL_APIENTRY __glnop_PushDebugGroup(__GLcontext *gc, GLenum source, GLuint id, GLsizei length, const GLchar * message){}
GLvoid GL_APIENTRY __glnop_PopDebugGroup(__GLcontext *gc){}
GLvoid GL_APIENTRY __glnop_ObjectLabel(__GLcontext *gc, GLenum identifier, GLuint name, GLsizei length, const GLchar *label){}
GLvoid GL_APIENTRY __glnop_GetObjectLabel(__GLcontext *gc, GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label){}
GLvoid GL_APIENTRY __glnop_ObjectPtrLabel(__GLcontext *gc, const GLvoid* ptr, GLsizei length, const GLchar *label){}
GLvoid GL_APIENTRY __glnop_GetObjectPtrLabel(__GLcontext *gc, const GLvoid* ptr, GLsizei bufSize, GLsizei *length, GLchar *label){}


GLenum GL_APIENTRY __glnop_GetGraphicsResetStatus(__GLcontext *gc){return 0;}
GLvoid GL_APIENTRY __glnop_ReadnPixels(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLvoid *data){}
GLvoid GL_APIENTRY __glnop_GetnUniformfv(__GLcontext *gc, GLuint program, GLint location, GLsizei bufSize, GLfloat *params){}
GLvoid GL_APIENTRY __glnop_GetnUniformiv(__GLcontext *gc, GLuint program, GLint location, GLsizei bufSize, GLint *params){}
GLvoid GL_APIENTRY __glnop_GetnUniformuiv(__GLcontext *gc, GLuint program, GLint location, GLsizei bufSize, GLuint *params){}

GLvoid GL_APIENTRY __glnop_BlendEquationi(__GLcontext * gc, GLuint buf, GLenum mode){}
GLvoid GL_APIENTRY __glnop_BlendEquationSeparatei(__GLcontext * gc, GLuint buf, GLenum modeRGB, GLenum modeAlpha){}
GLvoid GL_APIENTRY __glnop_BlendFunci(__GLcontext * gc, GLuint buf, GLenum sfactor, GLenum dfactor){}
GLvoid GL_APIENTRY __glnop_BlendFuncSeparatei(__GLcontext * gc, GLuint buf, GLenum sfactorRGB,GLenum dfactorRGB,GLenum sfactorAlpha,GLenum dfactorAlpha){}
GLvoid GL_APIENTRY __glnop_ColorMaski(__GLcontext * gc,GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a){}
GLvoid GL_APIENTRY __glnop_Enablei(__GLcontext *gc, GLenum target, GLuint index){}
GLvoid GL_APIENTRY __glnop_Disablei(__GLcontext *gc, GLenum target, GLuint index){}
GLboolean GL_APIENTRY __glnop_IsEnabledi(__GLcontext * gc, GLenum target, GLuint index){return GL_FALSE;}

GLvoid GL_APIENTRY __glnop_TexParameterIiv(__GLcontext *gc, GLenum target, GLenum pname, const GLint *params){}
GLvoid GL_APIENTRY __glnop_TexParameterIuiv(__GLcontext *gc, GLenum target, GLenum pname, const GLuint *params){}
GLvoid GL_APIENTRY __glnop_GetTexParameterIiv(__GLcontext *gc, GLenum target, GLenum pname, GLint *params){}
GLvoid GL_APIENTRY __glnop_GetTexParameterIuiv(__GLcontext *gc, GLenum target, GLenum pname, GLuint *params){}
GLvoid GL_APIENTRY __glnop_SamplerParameterIiv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLint *param){}
GLvoid GL_APIENTRY __glnop_SamplerParameterIuiv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLuint *param){}
GLvoid GL_APIENTRY __glnop_GetSamplerParameterIiv(__GLcontext *gc, GLuint sampler, GLenum pname, GLint *params){}
GLvoid GL_APIENTRY __glnop_GetSamplerParameterIuiv(__GLcontext *gc, GLuint sampler, GLenum pname, GLuint *params){}

GLvoid GL_APIENTRY __glnop_TexBuffer(__GLcontext *gc, GLenum target, GLenum internalformat, GLuint buffer){}
GLvoid GL_APIENTRY __glnop_TexBufferRange(__GLcontext *gc, GLenum target, GLenum internalformat,
                                                   GLuint buffer, GLintptr offset, GLsizeiptr size){}
GLvoid GL_APIENTRY __glnop_PatchParameteri(__GLcontext *gc, GLenum pname, GLint value){}

GLvoid GL_APIENTRY __glnop_FramebufferTexture(__GLcontext *gc, GLenum target, GLenum attachment, GLuint texture, GLint level){}

GLvoid GL_APIENTRY __glnop_MinSampleShading(__GLcontext *gc, GLfloat value){}

GLvoid GL_APIENTRY __glnop_CopyImageSubData(__GLcontext *gc,
                                           GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ,
                                           GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ,
                                           GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth){}

GLvoid GL_APIENTRY __glnop_DrawElementsBaseVertex(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex){}
GLvoid GL_APIENTRY __glnop_DrawRangeElementsBaseVertex(__GLcontext *gc, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex){}
GLvoid GL_APIENTRY __glnop_DrawElementsInstancedBaseVertex(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex){}
GLvoid GL_APIENTRY __glnop_PrimitiveBoundingBox(__GLcontext *gc, GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW){}

/* OpenGL ES extension */
#if GL_OES_EGL_image
GLvoid GL_APIENTRY __glnop_EGLImageTargetTexture2DOES(__GLcontext *gc, GLenum target, GLeglImageOES image){}
GLvoid GL_APIENTRY __glnop_EGLImageTargetRenderbufferStorageOES(__GLcontext *gc, GLenum target, GLeglImageOES image){}
#endif

#if GL_EXT_multi_draw_arrays
GLvoid GL_APIENTRY __glnop_MultiDrawArraysEXT(__GLcontext *gc, GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount){}
GLvoid GL_APIENTRY __glnop_MultiDrawElementsEXT(__GLcontext *gc, GLenum mode, const GLsizei *count, GLenum type, const GLvoid*const*indices, GLsizei primcount){}
#if GL_EXT_draw_elements_base_vertex
GLvoid GL_APIENTRY __glnop_MultiDrawElementsBaseVertexEXT(__GLcontext *gc, GLenum mode, const GLsizei * count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint * basevertex){}
#endif
#endif

#if GL_OES_mapbuffer
GLvoid GL_APIENTRY __glnop_GetBufferPointervOES(__GLcontext *gc, GLenum target, GLenum pname, GLvoid** params){}
GLvoid* GL_APIENTRY __glnop_MapBufferOES(__GLcontext *gc, GLenum target, GLenum access){return NULL;}
GLboolean GL_APIENTRY __glnop_UnmapBufferOES(__GLcontext *gc, GLenum target){return GL_FALSE;}
#endif

#if GL_EXT_discard_framebuffer
GLvoid GL_APIENTRY __glnop_DiscardFramebufferEXT(__GLcontext *gc, GLenum target, GLsizei numAttachments, const GLenum *attachments){}
#endif

#if GL_EXT_multisampled_render_to_texture
GLvoid GL_APIENTRY __glnop_RenderbufferStorageMultisampleEXT(__GLcontext *gc, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height){}
GLvoid GL_APIENTRY __glnop_FramebufferTexture2DMultisampleEXT(__GLcontext *gc, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples){}
#endif

#if GL_VIV_direct_texture
GLvoid GL_APIENTRY __glnop_TexDirectVIV(__GLcontext *gc, GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** pixels){}
GLvoid GL_APIENTRY __glnop_TexDirectInvalidateVIV(__GLcontext *gc, GLenum target){}
GLvoid GL_APIENTRY __glnop_TexDirectVIVMap(__GLcontext *gc, GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** logical, const GLuint * physical){}
GLvoid GL_APIENTRY __glnop_TexDirectTiledMapVIV(__GLcontext *gc, GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** logical, const GLuint * physical){}
#endif

#if GL_EXT_multi_draw_indirect
GLvoid GL_APIENTRY __glnop_MultiDrawArraysIndirectEXT(__GLcontext *gc, GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride){}
GLvoid GL_APIENTRY __glnop_MultiDrawElementsIndirectEXT(__GLcontext *gc, GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride){}
#endif

GLvoid GL_APIENTRY __glnop_GetObjectParameterivARB (__GLcontext *gc, UINT obj, GLenum pname, GLint *params){}
/* To do */
GLvoid GLAPIENTRY __glnop_GetVertexAttribdv(__GLcontext *gc, GLuint index, GLenum pname, GLdouble* params){}
GLvoid GLAPIENTRY __glnop_GetQueryObjectiv(__GLcontext *gc, GLuint id, GLenum pname, GLint* params){}
GLvoid GLAPIENTRY __glnop_GetBufferSubData (__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data){}
GLvoid* GLAPIENTRY __glnop_MapBuffer(__GLcontext *gc, GLenum target, GLenum access){return NULL;}
GLvoid GLAPIENTRY __glnop_DeleteObjectARB (__GLcontext *gc, UINT obj){}
GLvoid GLAPIENTRY __glnop_GetInfoLogARB (__GLcontext *gc, UINT obj, GLsizei maxLength, GLsizei *length, char *infoLog){}
GLvoid GLAPIENTRY __glnop_ClampColorARB (__GLcontext *gc, GLenum target, GLenum clamp){}
GLboolean GLAPIENTRY __glnop_IsRenderbufferEXT( __GLcontext *gc,  GLuint renderbuffer){return GL_FALSE;}
GLvoid GLAPIENTRY __glnop_BindRenderbufferEXT(__GLcontext *gc,  GLenum target, GLuint renderbuffer){}
GLvoid GLAPIENTRY __glnop_DeleteRenderbuffersEXT(__GLcontext *gc,  GLsizei n, const GLuint *renderbuffers){}
GLvoid GLAPIENTRY __glnop_GenRenderbuffersEXT(__GLcontext *gc,  GLsizei n, GLuint *renderbuffers){}
GLvoid GLAPIENTRY __glnop_RenderbufferStorageEXT(__GLcontext *gc,  GLenum target, GLenum internalformat, GLsizei width, GLsizei height){}
GLvoid GLAPIENTRY __glnop_GetRenderbufferParameterivEXT(__GLcontext *gc,  GLenum target, GLenum pname, GLint* params){}
GLboolean GLAPIENTRY __glnop_IsFramebufferEXT(__GLcontext *gc,  GLuint framebuffer){return GL_FALSE;}
GLvoid GLAPIENTRY __glnop_BindFramebufferEXT(__GLcontext *gc,  GLenum target, GLuint framebuffer){}
GLvoid GLAPIENTRY __glnop_DeleteFramebuffersEXT(__GLcontext *gc,  GLsizei n, const GLuint *framebuffers){}
GLvoid GLAPIENTRY __glnop_GenFramebuffersEXT(__GLcontext *gc,  GLsizei n, GLuint *framebuffers){}
GLenum GLAPIENTRY __glnop_CheckFramebufferStatusEXT(__GLcontext *gc,  GLenum target){return GL_FRAMEBUFFER_COMPLETE;}
GLvoid GLAPIENTRY __glnop_FramebufferTexture1DEXT(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level){}
GLvoid GLAPIENTRY __glnop_FramebufferTexture2DEXT(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level){}
GLvoid GLAPIENTRY __glnop_FramebufferTexture3DEXT(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset){}
GLvoid GLAPIENTRY __glnop_FramebufferRenderbufferEXT(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer){}
GLvoid GLAPIENTRY __glnop_GetFramebufferAttachmentParameterivEXT(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum pname, GLint *params){}
GLvoid GLAPIENTRY __glnop_GenerateMipmapEXT(__GLcontext *gc,  GLenum target){}
GLvoid GLAPIENTRY __glnop_BlitFramebufferEXT(__GLcontext *gc,  GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,GLbitfield mask, GLenum filter){}
GLvoid GLAPIENTRY __glnop_GetUniformdv(__GLcontext *gc,  GLuint program, GLint location, GLdouble * params){}
GLvoid GLAPIENTRY __glnop_Uniform1d(__GLcontext *gc, GLint location, GLdouble v0){}
GLvoid GLAPIENTRY __glnop_Uniform2d(__GLcontext *gc, GLint location, GLdouble v0, GLdouble v1){}
GLvoid GLAPIENTRY __glnop_Uniform3d(__GLcontext *gc, GLint location, GLdouble v0, GLdouble v1, GLdouble v2){}
GLvoid GLAPIENTRY __glnop_Uniform4d(__GLcontext *gc, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3){}
GLvoid GLAPIENTRY __glnop_Uniform1dv(__GLcontext *gc, GLint location, GLsizei count, const GLdouble* value){}
GLvoid GLAPIENTRY __glnop_Uniform2dv(__GLcontext *gc, GLint location, GLsizei count, const GLdouble* value){}
GLvoid GLAPIENTRY __glnop_Uniform3dv(__GLcontext *gc, GLint location, GLsizei count, const GLdouble* value){}
GLvoid GLAPIENTRY __glnop_Uniform4dv(__GLcontext *gc, GLint location, GLsizei count, const GLdouble* value){}
GLvoid GLAPIENTRY __glnop_UniformMatrix2dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value){}
GLvoid GLAPIENTRY __glnop_UniformMatrix3dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value){}
GLvoid GLAPIENTRY __glnop_UniformMatrix4dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value){}
GLvoid GLAPIENTRY __glnop_UniformMatrix2x3dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value){}
GLvoid GLAPIENTRY __glnop_UniformMatrix3x2dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value){}
GLvoid GLAPIENTRY __glnop_UniformMatrix2x4dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value){}
GLvoid GLAPIENTRY __glnop_UniformMatrix4x2dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value){}
GLvoid GLAPIENTRY __glnop_UniformMatrix3x4dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value){}
GLvoid GLAPIENTRY __glnop_UniformMatrix4x3dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value){}
GLvoid GLAPIENTRY __glnop_ClampColor(__GLcontext *gc, GLenum target, GLenum clamp){}
GLvoid GLAPIENTRY __glnop_BeginConditionalRender(__GLcontext *gc, GLuint id, GLenum mode){}
GLvoid GLAPIENTRY __glnop_EndConditionalRender(__GLcontext *gc){}
GLvoid GLAPIENTRY __glnop_VertexAttribI1i(__GLcontext *gc, GLuint index, GLint x){}
GLvoid GLAPIENTRY __glnop_VertexAttribI2i(__GLcontext *gc, GLuint index, GLint x, GLint y){}
GLvoid GLAPIENTRY __glnop_VertexAttribI3i(__GLcontext *gc, GLuint index, GLint x, GLint y, GLint z){}

GLvoid GLAPIENTRY __glnop_VertexAttribI1ui(__GLcontext *gc, GLuint index, GLuint x){}
GLvoid GLAPIENTRY __glnop_VertexAttribI2ui(__GLcontext *gc, GLuint index, GLuint x, GLuint y){}
GLvoid GLAPIENTRY __glnop_VertexAttribI3ui(__GLcontext *gc, GLuint index, GLuint x, GLuint y, GLuint z){}

GLvoid GLAPIENTRY __glnop_VertexAttribI1iv(__GLcontext *gc, GLuint index, const GLint *v){}
GLvoid GLAPIENTRY __glnop_VertexAttribI2iv(__GLcontext *gc, GLuint index, const GLint *v){}
GLvoid GLAPIENTRY __glnop_VertexAttribI3iv(__GLcontext *gc, GLuint index, const GLint *v){}

GLvoid GLAPIENTRY __glnop_VertexAttribI1uiv(__GLcontext *gc, GLuint index, const GLuint *v){}
GLvoid GLAPIENTRY __glnop_VertexAttribI2uiv(__GLcontext *gc, GLuint index, const GLuint *v){}
GLvoid GLAPIENTRY __glnop_VertexAttribI3uiv(__GLcontext *gc, GLuint index, const GLuint *v){}

GLvoid GLAPIENTRY __glnop_VertexAttribI4bv(__GLcontext *gc, GLuint index, const GLbyte *v){}
GLvoid GLAPIENTRY __glnop_VertexAttribI4sv(__GLcontext *gc, GLuint index, const GLshort *v){}
GLvoid GLAPIENTRY __glnop_VertexAttribI4ubv(__GLcontext *gc, GLuint index, const GLubyte *v){}
GLvoid GLAPIENTRY __glnop_VertexAttribI4usv(__GLcontext *gc, GLuint index, const GLushort *v){}

GLvoid GLAPIENTRY __glnop_FramebufferTexture1D(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level){}
GLvoid GLAPIENTRY __glnop_FramebufferTexture3D(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset){}

/* OpenGL 3.1 */
GLvoid GLAPIENTRY __glnop_PrimitiveRestartIndex(__GLcontext *gc, GLuint index){}
GLvoid GLAPIENTRY __glnop_GetActiveUniformName(__GLcontext *gc, GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName){}

GLvoid GL_APIENTRY __glnop_MultiDrawArrays(__GLcontext *gc, GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount){}
GLvoid GL_APIENTRY __glnop_MultiDrawElements(__GLcontext *gc, GLenum mode, const GLsizei *count, GLenum type, const GLvoid*const*indices, GLsizei primcount){}

/* add GL api */
GLboolean GLAPIENTRY __gles(IsList)(__GLcontext *gc,   GLuint list ) { return GL_FALSE;}
GLvoid GLAPIENTRY __gles(DeleteLists)(__GLcontext *gc,   GLuint list, GLsizei range ) { }
GLuint GLAPIENTRY __gles(GenLists)(__GLcontext *gc,   GLsizei range ) { return 0; }
GLvoid GLAPIENTRY __gles(NewList)(__GLcontext *gc,   GLuint list, GLenum mode ) { }
GLvoid GLAPIENTRY __gles(EndList)(__GLcontext *gc) { }
GLvoid GLAPIENTRY __gles(CallList)(__GLcontext *gc,   GLuint list ) { }
GLvoid GLAPIENTRY __gles(CallLists)(__GLcontext *gc,   GLsizei n, GLenum type, const GLvoid *lists ) { }
GLvoid GLAPIENTRY __gles(ListBase)(__GLcontext *gc,   GLuint base ) { }
GLvoid GLAPIENTRY __gles(Begin)(__GLcontext *gc,   GLenum mode ) { }
GLvoid GLAPIENTRY __gles(Bitmap)(__GLcontext *gc,   GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap ) {}
GLvoid GLAPIENTRY __gles(Color3b)(__GLcontext *gc,   GLbyte red, GLbyte green, GLbyte blue ){}
GLvoid GLAPIENTRY __gles(Color3d)(__GLcontext *gc,   GLdouble red, GLdouble green, GLdouble blue ){}
GLvoid GLAPIENTRY __gles(Color3f)(__GLcontext *gc,   GLfloat red, GLfloat green, GLfloat blue ){}
GLvoid GLAPIENTRY __gles(Color3i)(__GLcontext *gc,   GLint red, GLint green, GLint blue ){}
GLvoid GLAPIENTRY __gles(Color3s)(__GLcontext *gc,   GLshort red, GLshort green, GLshort blue ){}
GLvoid GLAPIENTRY __gles(Color3ub)(__GLcontext *gc,   GLubyte red, GLubyte green, GLubyte blue ){}
GLvoid GLAPIENTRY __gles(Color3ui)(__GLcontext *gc,   GLuint red, GLuint green, GLuint blue ){}
GLvoid GLAPIENTRY __gles(Color3us)(__GLcontext *gc,   GLushort red, GLushort green, GLushort blue ){}
GLvoid GLAPIENTRY __gles(Color3bv)(__GLcontext *gc,   const GLbyte *v ){}
GLvoid GLAPIENTRY __gles(Color3dv)(__GLcontext *gc,   const GLdouble *v ){}
GLvoid GLAPIENTRY __gles(Color3fv)(__GLcontext *gc,   const GLfloat *v ){}
GLvoid GLAPIENTRY __gles(Color3iv)(__GLcontext *gc,   const GLint *v ){}
GLvoid GLAPIENTRY __gles(Color3sv)(__GLcontext *gc,   const GLshort *v ){}
GLvoid GLAPIENTRY __gles(Color3ubv)(__GLcontext *gc,   const GLubyte *v ){}
GLvoid GLAPIENTRY __gles(Color3uiv)(__GLcontext *gc,   const GLuint *v ){}
GLvoid GLAPIENTRY __gles(Color3usv)(__GLcontext *gc,   const GLushort *v ){}
GLvoid GLAPIENTRY __gles(Color4b)(__GLcontext *gc,   GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha ){}
GLvoid GLAPIENTRY __gles(Color4d)(__GLcontext *gc,   GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha ){}
GLvoid GLAPIENTRY __gles(Color4f)(__GLcontext *gc,   GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ){}
GLvoid GLAPIENTRY __gles(Color4i)(__GLcontext *gc,   GLint red, GLint green, GLint blue, GLint alpha ){}
GLvoid GLAPIENTRY __gles(Color4s)(__GLcontext *gc,   GLshort red, GLshort green, GLshort blue, GLshort alpha ){}
GLvoid GLAPIENTRY __gles(Color4ub)(__GLcontext *gc,   GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha ){}
GLvoid GLAPIENTRY __gles(Color4ui)(__GLcontext *gc,   GLuint red, GLuint green, GLuint blue, GLuint alpha ){}
GLvoid GLAPIENTRY __gles(Color4us)(__GLcontext *gc,   GLushort red, GLushort green, GLushort blue, GLushort alpha ){}
GLvoid GLAPIENTRY __gles(Color4bv)(__GLcontext *gc,   const GLbyte *v ){}
GLvoid GLAPIENTRY __gles(Color4dv)(__GLcontext *gc,   const GLdouble *v ){}
GLvoid GLAPIENTRY __gles(Color4fv)(__GLcontext *gc,   const GLfloat *v ){}
GLvoid GLAPIENTRY __gles(Color4iv)(__GLcontext *gc,   const GLint *v ){}
GLvoid GLAPIENTRY __gles(Color4sv)(__GLcontext *gc,   const GLshort *v ){}
GLvoid GLAPIENTRY __gles(Color4ubv)(__GLcontext *gc,   const GLubyte *v ){}
GLvoid GLAPIENTRY __gles(Color4uiv)(__GLcontext *gc,   const GLuint *v ){}
GLvoid GLAPIENTRY __gles(Color4usv)(__GLcontext *gc,   const GLushort *v ){}
GLvoid GLAPIENTRY __gles(EdgeFlag)(__GLcontext *gc,   GLboolean flag ){}
GLvoid GLAPIENTRY __gles(EdgeFlagv)(__GLcontext *gc,   const GLboolean *flag ){}
GLvoid GLAPIENTRY __gles(End)(__GLcontext *gc){}
GLvoid GLAPIENTRY __gles(Indexd)(__GLcontext *gc,   GLdouble c ){}
GLvoid GLAPIENTRY __gles(Indexf)(__GLcontext *gc,   GLfloat c ){}
GLvoid GLAPIENTRY __gles(Indexi)(__GLcontext *gc,   GLint c ){}
GLvoid GLAPIENTRY __gles(Indexs)(__GLcontext *gc,   GLshort c ){}
GLvoid GLAPIENTRY __gles(Indexdv)(__GLcontext *gc,   const GLdouble *c ){}
GLvoid GLAPIENTRY __gles(Indexfv)(__GLcontext *gc,   const GLfloat *c ){}
GLvoid GLAPIENTRY __gles(Indexiv)(__GLcontext *gc,   const GLint *c ){}
GLvoid GLAPIENTRY __gles(Indexsv)(__GLcontext *gc,   const GLshort *c ){}
GLvoid GLAPIENTRY __gles(Normal3b)(__GLcontext *gc,   GLbyte nx, GLbyte ny, GLbyte nz ){}
GLvoid GLAPIENTRY __gles(Normal3d)(__GLcontext *gc,   GLdouble nx, GLdouble ny, GLdouble nz ){}
GLvoid GLAPIENTRY __gles(Normal3f)(__GLcontext *gc,   GLfloat nx, GLfloat ny, GLfloat nz ){}
GLvoid GLAPIENTRY __gles(Normal3i)(__GLcontext *gc,   GLint nx, GLint ny, GLint nz ){}
GLvoid GLAPIENTRY __gles(Normal3s)(__GLcontext *gc,   GLshort nx, GLshort ny, GLshort nz ){}
GLvoid GLAPIENTRY __gles(Normal3bv)(__GLcontext *gc,   const GLbyte *v ){}
GLvoid GLAPIENTRY __gles(Normal3dv)(__GLcontext *gc,   const GLdouble *v ){}
GLvoid GLAPIENTRY __gles(Normal3fv)(__GLcontext *gc,   const GLfloat *v ){}
GLvoid GLAPIENTRY __gles(Normal3iv)(__GLcontext *gc,   const GLint *v ){}
GLvoid GLAPIENTRY __gles(Normal3sv)(__GLcontext *gc,   const GLshort *v ){}
GLvoid GLAPIENTRY __gles(RasterPos2d)(__GLcontext *gc,   GLdouble x, GLdouble y ){}
GLvoid GLAPIENTRY __gles(RasterPos2f)(__GLcontext *gc,   GLfloat x, GLfloat y ){}
GLvoid GLAPIENTRY __gles(RasterPos2i)(__GLcontext *gc,   GLint x, GLint y ){}
GLvoid GLAPIENTRY __gles(RasterPos2s)(__GLcontext *gc,   GLshort x, GLshort y ){}
GLvoid GLAPIENTRY __gles(RasterPos3d)(__GLcontext *gc,   GLdouble x, GLdouble y, GLdouble z ){}
GLvoid GLAPIENTRY __gles(RasterPos3f)(__GLcontext *gc,   GLfloat x, GLfloat y, GLfloat z ){}
GLvoid GLAPIENTRY __gles(RasterPos3i)(__GLcontext *gc,   GLint x, GLint y, GLint z ){}
GLvoid GLAPIENTRY __gles(RasterPos3s)(__GLcontext *gc,   GLshort x, GLshort y, GLshort z ){}
GLvoid GLAPIENTRY __gles(RasterPos4d)(__GLcontext *gc,   GLdouble x, GLdouble y, GLdouble z, GLdouble w ){}
GLvoid GLAPIENTRY __gles(RasterPos4f)(__GLcontext *gc,   GLfloat x, GLfloat y, GLfloat z, GLfloat w ){}
GLvoid GLAPIENTRY __gles(RasterPos4i)(__GLcontext *gc,   GLint x, GLint y, GLint z, GLint w ){}
GLvoid GLAPIENTRY __gles(RasterPos4s)(__GLcontext *gc,   GLshort x, GLshort y, GLshort z, GLshort w ){}
GLvoid GLAPIENTRY __gles(RasterPos2dv)(__GLcontext *gc,   const GLdouble *v ){}
GLvoid GLAPIENTRY __gles(RasterPos2fv)(__GLcontext *gc,   const GLfloat *v ){}
GLvoid GLAPIENTRY __gles(RasterPos2iv)(__GLcontext *gc,   const GLint *v ){}
GLvoid GLAPIENTRY __gles(RasterPos2sv)(__GLcontext *gc,   const GLshort *v ){}
GLvoid GLAPIENTRY __gles(RasterPos3dv)(__GLcontext *gc,   const GLdouble *v ){}
GLvoid GLAPIENTRY __gles(RasterPos3fv)(__GLcontext *gc,   const GLfloat *v ){}
GLvoid GLAPIENTRY __gles(RasterPos3iv)(__GLcontext *gc,   const GLint *v ){}
GLvoid GLAPIENTRY __gles(RasterPos3sv)(__GLcontext *gc,   const GLshort *v ){}
GLvoid GLAPIENTRY __gles(RasterPos4dv)(__GLcontext *gc,   const GLdouble *v ){}
GLvoid GLAPIENTRY __gles(RasterPos4fv)(__GLcontext *gc,   const GLfloat *v ){}
GLvoid GLAPIENTRY __gles(RasterPos4iv)(__GLcontext *gc,   const GLint *v ){}
GLvoid GLAPIENTRY __gles(RasterPos4sv)(__GLcontext *gc,   const GLshort *v ){}
GLvoid GLAPIENTRY __gles(Rectd)(__GLcontext *gc,   GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2 ){}
GLvoid GLAPIENTRY __gles(Rectf)(__GLcontext *gc,   GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 ){}
GLvoid GLAPIENTRY __gles(Recti)(__GLcontext *gc,   GLint x1, GLint y1, GLint x2, GLint y2 ){}
GLvoid GLAPIENTRY __gles(Rects)(__GLcontext *gc,   GLshort x1, GLshort y1, GLshort x2, GLshort y2 ){}
GLvoid GLAPIENTRY __gles(Rectdv)(__GLcontext *gc,   const GLdouble *v1, const GLdouble *v2 ){}
GLvoid GLAPIENTRY __gles(Rectfv)(__GLcontext *gc,   const GLfloat *v1, const GLfloat *v2 ){}
GLvoid GLAPIENTRY __gles(Rectiv)(__GLcontext *gc,   const GLint *v1, const GLint *v2 ){}
GLvoid GLAPIENTRY __gles(Rectsv)(__GLcontext *gc,   const GLshort *v1, const GLshort *v2 ){}
GLvoid GLAPIENTRY __gles(TexCoord1d)(__GLcontext *gc,   GLdouble s ){}
GLvoid GLAPIENTRY __gles(TexCoord1f)(__GLcontext *gc,   GLfloat s ){}
GLvoid GLAPIENTRY __gles(TexCoord1i)(__GLcontext *gc,   GLint s ){}
GLvoid GLAPIENTRY __gles(TexCoord1s)(__GLcontext *gc,   GLshort s ){}
GLvoid GLAPIENTRY __gles(TexCoord2d)(__GLcontext *gc,   GLdouble s, GLdouble t ){}
GLvoid GLAPIENTRY __gles(TexCoord2f)(__GLcontext *gc,   GLfloat s, GLfloat t ){}
GLvoid GLAPIENTRY __gles(TexCoord2i)(__GLcontext *gc,   GLint s, GLint t ){}
GLvoid GLAPIENTRY __gles(TexCoord2s)(__GLcontext *gc,   GLshort s, GLshort t ){}
GLvoid GLAPIENTRY __gles(TexCoord3d)(__GLcontext *gc,   GLdouble s, GLdouble t, GLdouble r ){}
GLvoid GLAPIENTRY __gles(TexCoord3f)(__GLcontext *gc,   GLfloat s, GLfloat t, GLfloat r ){}
GLvoid GLAPIENTRY __gles(TexCoord3i)(__GLcontext *gc,   GLint s, GLint t, GLint r ){}
GLvoid GLAPIENTRY __gles(TexCoord3s)(__GLcontext *gc,   GLshort s, GLshort t, GLshort r ){}
GLvoid GLAPIENTRY __gles(TexCoord4d)(__GLcontext *gc,   GLdouble s, GLdouble t, GLdouble r, GLdouble q ){}
GLvoid GLAPIENTRY __gles(TexCoord4f)(__GLcontext *gc,   GLfloat s, GLfloat t, GLfloat r, GLfloat q ){}
GLvoid GLAPIENTRY __gles(TexCoord4i)(__GLcontext *gc,   GLint s, GLint t, GLint r, GLint q ){}
GLvoid GLAPIENTRY __gles(TexCoord4s)(__GLcontext *gc,   GLshort s, GLshort t, GLshort r, GLshort q ){}
GLvoid GLAPIENTRY __gles(TexCoord1dv)(__GLcontext *gc,   const GLdouble *v ){}
GLvoid GLAPIENTRY __gles(TexCoord1fv)(__GLcontext *gc,   const GLfloat *v ){}
GLvoid GLAPIENTRY __gles(TexCoord1iv)(__GLcontext *gc,   const GLint *v ){}
GLvoid GLAPIENTRY __gles(TexCoord1sv)(__GLcontext *gc,   const GLshort *v ){}
GLvoid GLAPIENTRY __gles(TexCoord2dv)(__GLcontext *gc,   const GLdouble *v ){}
GLvoid GLAPIENTRY __gles(TexCoord2fv)(__GLcontext *gc,   const GLfloat *v ){}
GLvoid GLAPIENTRY __gles(TexCoord2iv)(__GLcontext *gc,   const GLint *v ){}
GLvoid GLAPIENTRY __gles(TexCoord2sv)(__GLcontext *gc,   const GLshort *v ){}
GLvoid GLAPIENTRY __gles(TexCoord3dv)(__GLcontext *gc,   const GLdouble *v ){}
GLvoid GLAPIENTRY __gles(TexCoord3fv)(__GLcontext *gc,   const GLfloat *v ){}
GLvoid GLAPIENTRY __gles(TexCoord3iv)(__GLcontext *gc,   const GLint *v ){}
GLvoid GLAPIENTRY __gles(TexCoord3sv)(__GLcontext *gc,   const GLshort *v ){}
GLvoid GLAPIENTRY __gles(TexCoord4dv)(__GLcontext *gc,   const GLdouble *v ){}
GLvoid GLAPIENTRY __gles(TexCoord4fv)(__GLcontext *gc,   const GLfloat *v ){}
GLvoid GLAPIENTRY __gles(TexCoord4iv)(__GLcontext *gc,   const GLint *v ){}
GLvoid GLAPIENTRY __gles(TexCoord4sv)(__GLcontext *gc,   const GLshort *v ){}
GLvoid GLAPIENTRY __gles(Vertex2d)(__GLcontext *gc,   GLdouble x, GLdouble y ){}
GLvoid GLAPIENTRY __gles(Vertex2f)(__GLcontext *gc,   GLfloat x, GLfloat y ){}
GLvoid GLAPIENTRY __gles(Vertex2i)(__GLcontext *gc,   GLint x, GLint y ){}
GLvoid GLAPIENTRY __gles(Vertex2s)(__GLcontext *gc,   GLshort x, GLshort y ){}
GLvoid GLAPIENTRY __gles(Vertex3d)(__GLcontext *gc,   GLdouble x, GLdouble y, GLdouble z ){}
GLvoid GLAPIENTRY __gles(Vertex3f)(__GLcontext *gc,   GLfloat x, GLfloat y, GLfloat z ){}
GLvoid GLAPIENTRY __gles(Vertex3i)(__GLcontext *gc,   GLint x, GLint y, GLint z ){}
GLvoid GLAPIENTRY __gles(Vertex3s)(__GLcontext *gc,   GLshort x, GLshort y, GLshort z ){}
GLvoid GLAPIENTRY __gles(Vertex4d)(__GLcontext *gc,   GLdouble x, GLdouble y, GLdouble z, GLdouble w ){}
GLvoid GLAPIENTRY __gles(Vertex4f)(__GLcontext *gc,   GLfloat x, GLfloat y, GLfloat z, GLfloat w ){}
GLvoid GLAPIENTRY __gles(Vertex4i)(__GLcontext *gc,   GLint x, GLint y, GLint z, GLint w ){}
GLvoid GLAPIENTRY __gles(Vertex4s)(__GLcontext *gc,   GLshort x, GLshort y, GLshort z, GLshort w ){}
GLvoid GLAPIENTRY __gles(Vertex2dv)(__GLcontext *gc,   const GLdouble *v ){}
GLvoid GLAPIENTRY __gles(Vertex2fv)(__GLcontext *gc,   const GLfloat *v ){}
GLvoid GLAPIENTRY __gles(Vertex2iv)(__GLcontext *gc,   const GLint *v ){}
GLvoid GLAPIENTRY __gles(Vertex2sv)(__GLcontext *gc,   const GLshort *v ){}
GLvoid GLAPIENTRY __gles(Vertex3dv)(__GLcontext *gc,   const GLdouble *v ){}
GLvoid GLAPIENTRY __gles(Vertex3fv)(__GLcontext *gc,   const GLfloat *v ){}
GLvoid GLAPIENTRY __gles(Vertex3iv)(__GLcontext *gc,   const GLint *v ){}
GLvoid GLAPIENTRY __gles(Vertex3sv)(__GLcontext *gc,   const GLshort *v ){}
GLvoid GLAPIENTRY __gles(Vertex4dv)(__GLcontext *gc,   const GLdouble *v ){}
GLvoid GLAPIENTRY __gles(Vertex4fv)(__GLcontext *gc,   const GLfloat *v ){}
GLvoid GLAPIENTRY __gles(Vertex4iv)(__GLcontext *gc,   const GLint *v ){}
GLvoid GLAPIENTRY __gles(Vertex4sv)(__GLcontext *gc,   const GLshort *v ){}
GLvoid GLAPIENTRY __gles(ClipPlane)(__GLcontext *gc,   GLenum plane, const GLdouble *equation ){}
GLvoid GLAPIENTRY __gles(ColorMaterial)(__GLcontext *gc,   GLenum face, GLenum mode ){}
GLvoid GLAPIENTRY __gles(Fogf)(__GLcontext *gc,   GLenum pname, GLfloat param ){}
GLvoid GLAPIENTRY __gles(Fogi)(__GLcontext *gc,   GLenum pname, GLint param ){}
GLvoid GLAPIENTRY __gles(Fogfv)(__GLcontext *gc,   GLenum pname, const GLfloat *params ){}
GLvoid GLAPIENTRY __gles(Fogiv)(__GLcontext *gc,   GLenum pname, const GLint *params ){}
GLvoid GLAPIENTRY __gles(Lightf)(__GLcontext *gc,   GLenum light, GLenum pname, GLfloat param ){}
GLvoid GLAPIENTRY __gles(Lighti)(__GLcontext *gc,   GLenum light, GLenum pname, GLint param ){}
GLvoid GLAPIENTRY __gles(Lightfv)(__GLcontext *gc,   GLenum light, GLenum pname, const GLfloat *params ){}
GLvoid GLAPIENTRY __gles(Lightiv)(__GLcontext *gc,   GLenum light, GLenum pname, const GLint *params ){}
GLvoid GLAPIENTRY __gles(LightModelf)(__GLcontext *gc,   GLenum pname, GLfloat param ){}
GLvoid GLAPIENTRY __gles(LightModeli)(__GLcontext *gc,   GLenum pname, GLint param ){}
GLvoid GLAPIENTRY __gles(LightModelfv)(__GLcontext *gc,   GLenum pname, const GLfloat *params ){}
GLvoid GLAPIENTRY __gles(LightModeliv)(__GLcontext *gc,   GLenum pname, const GLint *params ){}
GLvoid GLAPIENTRY __gles(LineStipple)(__GLcontext *gc,   GLint factor, GLushort pattern ){}
GLvoid GLAPIENTRY __gles(Materialf)(__GLcontext *gc,   GLenum face, GLenum pname, GLfloat param ){}
GLvoid GLAPIENTRY __gles(Materiali)(__GLcontext *gc,   GLenum face, GLenum pname, GLint param ){}
GLvoid GLAPIENTRY __gles(Materialfv)(__GLcontext *gc,   GLenum face, GLenum pname, const GLfloat *params ){}
GLvoid GLAPIENTRY __gles(Materialiv)(__GLcontext *gc,   GLenum face, GLenum pname, const GLint *params ){}
GLvoid GLAPIENTRY __gles(PointSize)(__GLcontext *gc,   GLfloat size ){}
GLvoid GLAPIENTRY __gles(PolygonMode)(__GLcontext *gc,   GLenum face, GLenum mode ){}
GLvoid GLAPIENTRY __gles(PolygonStipple)(__GLcontext *gc,   const GLubyte *mask ){}
GLvoid GLAPIENTRY __gles(ShadeModel)(__GLcontext *gc,   GLenum mode ){}
GLvoid GLAPIENTRY __gles(TexImage1D)(__GLcontext *gc,   GLenum target, GLint level, GLint internalFormat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels ){}
GLvoid GLAPIENTRY __gles(TexEnvf)(__GLcontext *gc,   GLenum target, GLenum pname, GLfloat param ){}
GLvoid GLAPIENTRY __gles(TexEnvi)(__GLcontext *gc,   GLenum target, GLenum pname, GLint param ){}
GLvoid GLAPIENTRY __gles(TexEnvfv)(__GLcontext *gc,   GLenum target, GLenum pname, const GLfloat *params ){}
GLvoid GLAPIENTRY __gles(TexEnviv)(__GLcontext *gc,   GLenum target, GLenum pname, const GLint *params ){}
GLvoid GLAPIENTRY __gles(TexGend)(__GLcontext *gc,   GLenum coord, GLenum pname, GLdouble param ){}
GLvoid GLAPIENTRY __gles(TexGenf)(__GLcontext *gc,   GLenum coord, GLenum pname, GLfloat param ){}
GLvoid GLAPIENTRY __gles(TexGeni)(__GLcontext *gc,   GLenum coord, GLenum pname, GLint param ){}
GLvoid GLAPIENTRY __gles(TexGendv)(__GLcontext *gc,   GLenum coord, GLenum pname, const GLdouble *params ){}
GLvoid GLAPIENTRY __gles(TexGenfv)(__GLcontext *gc,   GLenum coord, GLenum pname, const GLfloat *params ){}
GLvoid GLAPIENTRY __gles(TexGeniv)(__GLcontext *gc,   GLenum coord, GLenum pname, const GLint *params ){}
GLvoid GLAPIENTRY __gles(FeedbackBuffer)(__GLcontext *gc,   GLsizei size, GLenum type, GLfloat *buffer ){}
GLvoid GLAPIENTRY __gles(SelectBuffer)(__GLcontext *gc,   GLsizei size, GLuint *buffer ){}
GLint GLAPIENTRY __gles(RenderMode)(__GLcontext *gc,   GLenum mode ) { return 0;}
GLvoid GLAPIENTRY __gles(InitNames)(__GLcontext *gc){}
GLvoid GLAPIENTRY __gles(LoadName)(__GLcontext *gc,   GLuint name ){}
GLvoid GLAPIENTRY __gles(PushName)(__GLcontext *gc,   GLuint name ){}
GLvoid GLAPIENTRY __gles(PopName)(__GLcontext *gc){}
GLvoid GLAPIENTRY __gles(PassThrough)(__GLcontext *gc,   GLfloat token ){}
GLvoid GLAPIENTRY __gles(DrawBuffer)(__GLcontext *gc,   GLenum mode ){}
GLvoid GLAPIENTRY __gles(ClearAccum)(__GLcontext *gc,   GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ){}
GLvoid GLAPIENTRY __gles(ClearIndex)(__GLcontext *gc,   GLfloat c ){}
GLvoid GLAPIENTRY __gles(ClearDepth)(__GLcontext *gc,   GLclampd depth ){}
GLvoid GLAPIENTRY __gles(IndexMask)(__GLcontext *gc,   GLuint mask ){}
GLvoid GLAPIENTRY __gles(Accum)(__GLcontext *gc,   GLenum op, GLfloat value ){}
GLvoid GLAPIENTRY __gles(PushAttrib)(__GLcontext *gc,   GLbitfield mask ){}
GLvoid GLAPIENTRY __gles(PopAttrib)(__GLcontext *gc){}
GLvoid GLAPIENTRY __gles(Map1d)(__GLcontext *gc,   GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points ){}
GLvoid GLAPIENTRY __gles(Map1f)(__GLcontext *gc,   GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points ){}
GLvoid GLAPIENTRY __gles(Map2d)(__GLcontext *gc,   GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points ){}
GLvoid GLAPIENTRY __gles(Map2f)(__GLcontext *gc,   GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points ){}
GLvoid GLAPIENTRY __gles(MapGrid1d)(__GLcontext *gc,   GLint un, GLdouble u1, GLdouble u2 ){}
GLvoid GLAPIENTRY __gles(MapGrid1f)(__GLcontext *gc,   GLint un, GLfloat u1, GLfloat u2 ){}
GLvoid GLAPIENTRY __gles(MapGrid2d)(__GLcontext *gc,   GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2 ){}
GLvoid GLAPIENTRY __gles(MapGrid2f)(__GLcontext *gc,   GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2 ){}
GLvoid GLAPIENTRY __gles(EvalCoord1d)(__GLcontext *gc,   GLdouble u ){}
GLvoid GLAPIENTRY __gles(EvalCoord1f)(__GLcontext *gc,   GLfloat u ){}
GLvoid GLAPIENTRY __gles(EvalCoord1dv)(__GLcontext *gc,   const GLdouble *u ){}
GLvoid GLAPIENTRY __gles(EvalCoord1fv)(__GLcontext *gc,   const GLfloat *u ){}
GLvoid GLAPIENTRY __gles(EvalCoord2d)(__GLcontext *gc,   GLdouble u, GLdouble v ){}
GLvoid GLAPIENTRY __gles(EvalCoord2f)(__GLcontext *gc,   GLfloat u, GLfloat v ){}
GLvoid GLAPIENTRY __gles(EvalCoord2dv)(__GLcontext *gc,   const GLdouble *u ){}
GLvoid GLAPIENTRY __gles(EvalCoord2fv)(__GLcontext *gc,   const GLfloat *u ){}
GLvoid GLAPIENTRY __gles(EvalPoint1)(__GLcontext *gc,   GLint i ){}
GLvoid GLAPIENTRY __gles(EvalPoint2)(__GLcontext *gc,   GLint i, GLint j ){}
GLvoid GLAPIENTRY __gles(EvalMesh1)(__GLcontext *gc,   GLenum mode, GLint i1, GLint i2 ){}
GLvoid GLAPIENTRY __gles(EvalMesh2)(__GLcontext *gc,   GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2 ){}
GLvoid GLAPIENTRY __gles(AlphaFunc)(__GLcontext *gc,   GLenum func, GLclampf ref ){}
GLvoid GLAPIENTRY __gles(LogicOp)(__GLcontext *gc,   GLenum opcode ){}
GLvoid GLAPIENTRY __gles(PixelZoom)(__GLcontext *gc,   GLfloat xfactor, GLfloat yfactor ){}
GLvoid GLAPIENTRY __gles(PixelTransferf)(__GLcontext *gc,   GLenum pname, GLfloat param ){}
GLvoid GLAPIENTRY __gles(PixelTransferi)(__GLcontext *gc,   GLenum pname, GLint param ){}
GLvoid GLAPIENTRY __gles(PixelStoref)(__GLcontext *gc,   GLenum pname, GLfloat param ){}
GLvoid GLAPIENTRY __gles(PixelMapfv)(__GLcontext *gc,   GLenum map, GLsizei mapsize, const GLfloat *values ){}
GLvoid GLAPIENTRY __gles(PixelMapuiv)(__GLcontext *gc,   GLenum map, GLsizei mapsize, const GLuint *values ){}
GLvoid GLAPIENTRY __gles(PixelMapusv)(__GLcontext *gc,   GLenum map, GLsizei mapsize, const GLushort *values ){}
GLvoid GLAPIENTRY __gles(GetClipPlane)(__GLcontext *gc,   GLenum plane, GLdouble *equation ){}
GLvoid GLAPIENTRY __gles(GetDoublev)(__GLcontext *gc,   GLenum pname, GLdouble *params ){}
GLvoid GLAPIENTRY __gles(GetLightfv)(__GLcontext *gc,   GLenum light, GLenum pname, GLfloat *params ){}
GLvoid GLAPIENTRY __gles(GetLightiv)(__GLcontext *gc,   GLenum light, GLenum pname, GLint *params ){}
GLvoid GLAPIENTRY __gles(GetMapdv)(__GLcontext *gc,   GLenum target, GLenum query, GLdouble *v ){}
GLvoid GLAPIENTRY __gles(GetMapfv)(__GLcontext *gc,   GLenum target, GLenum query, GLfloat *v ){}
GLvoid GLAPIENTRY __gles(GetMapiv)(__GLcontext *gc,   GLenum target, GLenum query, GLint *v ){}
GLvoid GLAPIENTRY __gles(GetMaterialfv)(__GLcontext *gc,   GLenum face, GLenum pname, GLfloat *params ){}
GLvoid GLAPIENTRY __gles(GetMaterialiv)(__GLcontext *gc,   GLenum face, GLenum pname, GLint *params ){}
GLvoid GLAPIENTRY __gles(GetPixelMapfv)(__GLcontext *gc,   GLenum map, GLfloat *values ){}
GLvoid GLAPIENTRY __gles(GetPixelMapuiv)(__GLcontext *gc,   GLenum map, GLuint *values ){}
GLvoid GLAPIENTRY __gles(GetPixelMapusv)(__GLcontext *gc,   GLenum map, GLushort *values ){}
GLvoid GLAPIENTRY __gles(GetPolygonStipple)(__GLcontext *gc,   GLubyte *mask ){}
GLvoid GLAPIENTRY __gles(GetTexEnvfv)(__GLcontext *gc,   GLenum target, GLenum pname, GLfloat *params ){}
GLvoid GLAPIENTRY __gles(GetTexEnviv)(__GLcontext *gc,   GLenum target, GLenum pname, GLint *params ){}
GLvoid GLAPIENTRY __gles(GetTexGendv)(__GLcontext *gc,   GLenum coord, GLenum pname, GLdouble *params ){}
GLvoid GLAPIENTRY __gles(GetTexGenfv)(__GLcontext *gc,   GLenum coord, GLenum pname, GLfloat *params ){}
GLvoid GLAPIENTRY __gles(GetTexGeniv)(__GLcontext *gc,   GLenum coord, GLenum pname, GLint *params ){}
GLvoid GLAPIENTRY __gles(DepthRange)(__GLcontext *gc,   GLclampd near_val, GLclampd far_val ){}
GLvoid GLAPIENTRY __gles(Frustum)(__GLcontext *gc,   GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val ){}
GLvoid GLAPIENTRY __gles(LoadIdentity)(__GLcontext *gc){}
GLvoid GLAPIENTRY __gles(LoadMatrixd)(__GLcontext *gc,   const GLdouble *m ){}
GLvoid GLAPIENTRY __gles(LoadMatrixf)(__GLcontext *gc,   const GLfloat *m ){}
GLvoid GLAPIENTRY __gles(MatrixMode)(__GLcontext *gc,   GLenum mode ){}
GLvoid GLAPIENTRY __gles(MultMatrixd)(__GLcontext *gc,   const GLdouble *m ){}
GLvoid GLAPIENTRY __gles(MultMatrixf)(__GLcontext *gc,   const GLfloat *m ){}
GLvoid GLAPIENTRY __gles(Ortho)(__GLcontext *gc,   GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val ){}
GLvoid GLAPIENTRY __gles(PushMatrix)(__GLcontext *gc ){}
GLvoid GLAPIENTRY __gles(PopMatrix)(__GLcontext *gc){}
GLvoid GLAPIENTRY __gles(Rotated)(__GLcontext *gc,   GLdouble angle, GLdouble x, GLdouble y, GLdouble z ){}
GLvoid GLAPIENTRY __gles(Rotatef)(__GLcontext *gc,   GLfloat angle, GLfloat x, GLfloat y, GLfloat z ){}
GLvoid GLAPIENTRY __gles(Scaled)(__GLcontext *gc,   GLdouble x, GLdouble y, GLdouble z ){}
GLvoid GLAPIENTRY __gles(Scalef)(__GLcontext *gc,   GLfloat x, GLfloat y, GLfloat z ){}
GLvoid GLAPIENTRY __gles(Translated)(__GLcontext *gc,   GLdouble x, GLdouble y, GLdouble z ){}
GLvoid GLAPIENTRY __gles(Translatef)(__GLcontext *gc,   GLfloat x, GLfloat y, GLfloat z ){}
GLvoid GLAPIENTRY __gles(ArrayElement)(__GLcontext *gc,   GLint i ){}
GLvoid GLAPIENTRY __gles(ColorPointer)(__GLcontext *gc,   GLint size, GLenum type, GLsizei stride, const GLvoid *ptr ){}
GLvoid GLAPIENTRY __gles(EnableClientState)(__GLcontext *gc,   GLenum cap ){}
GLvoid GLAPIENTRY __gles(DisableClientState)(__GLcontext *gc,   GLenum cap ){}
GLvoid GLAPIENTRY __gles(EdgeFlagPointer)(__GLcontext *gc,   GLsizei stride, const GLvoid *ptr ){}
GLvoid GLAPIENTRY __gles(IndexPointer)(__GLcontext *gc,   GLenum type, GLsizei stride, const GLvoid *ptr ){}
GLvoid GLAPIENTRY __gles(InterleavedArrays)(__GLcontext *gc,   GLenum format, GLsizei stride, const GLvoid *pointer ){}
GLvoid GLAPIENTRY __gles(NormalPointer)(__GLcontext *gc,   GLenum type, GLsizei stride,  const GLvoid *ptr ){}
GLvoid GLAPIENTRY __gles(VertexPointer)(__GLcontext *gc,   GLint size, GLenum type, GLsizei stride, const GLvoid *ptr ){}
GLvoid GLAPIENTRY __gles(TexCoordPointer)(__GLcontext *gc, GLint size, GLenum type, GLsizei stride, const GLvoid *ptr){}
GLboolean GLAPIENTRY __gles(AreTexturesResident)(__GLcontext *gc,   GLsizei n, const GLuint *textures, GLboolean *residences ) { return GL_FALSE;}
GLvoid GLAPIENTRY __gles(CopyTexImage1D)(__GLcontext *gc,   GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border ){}
GLvoid GLAPIENTRY __gles(CopyTexSubImage1D)(__GLcontext *gc,   GLenum target, GLint level,  GLint xoffset, GLint x, GLint y,  GLsizei width ){}
GLvoid GLAPIENTRY __gles(PrioritizeTextures)(__GLcontext *gc,   GLsizei n,  const GLuint *textures, const GLclampf *priorities ){}
GLvoid GLAPIENTRY __gles(TexSubImage1D)(__GLcontext *gc,   GLenum target, GLint level,  GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels ){}
GLvoid GLAPIENTRY __gles(PushClientAttrib)(__GLcontext *gc,   GLbitfield mask ){}
GLvoid GLAPIENTRY __gles(PopClientAttrib)(__GLcontext *gc){}
GLvoid GLAPIENTRY __gles(ColorTable)(__GLcontext *gc,   GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table ){}
GLvoid GLAPIENTRY __gles(ColorTableParameteriv)(__GLcontext *gc,  GLenum target, GLenum pname, const GLint *params){}
GLvoid GLAPIENTRY __gles(ColorTableParameterfv)(__GLcontext *gc,  GLenum target, GLenum pname, const GLfloat *params){}
GLvoid GLAPIENTRY __gles(CopyColorTable)(__GLcontext *gc,   GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width ){}
GLvoid GLAPIENTRY __gles(GetColorTable)(__GLcontext *gc,   GLenum target, GLenum format,  GLenum type, GLvoid *table ){}
GLvoid GLAPIENTRY __gles(GetColorTableParameterfv)(__GLcontext *gc,   GLenum target, GLenum pname, GLfloat *params ){}
GLvoid GLAPIENTRY __gles(GetColorTableParameteriv)(__GLcontext *gc,   GLenum target, GLenum pname,  GLint *params ){}
GLvoid GLAPIENTRY __gles(ColorSubTable)(__GLcontext *gc,   GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data ){}
GLvoid GLAPIENTRY __gles(CopyColorSubTable)(__GLcontext *gc,   GLenum target, GLsizei start, GLint x, GLint y, GLsizei width ){}
GLvoid GLAPIENTRY __gles(ConvolutionFilter1D)(__GLcontext *gc,   GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image ){}
GLvoid GLAPIENTRY __gles(ConvolutionFilter2D)(__GLcontext *gc,   GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image ){}
GLvoid GLAPIENTRY __gles(ConvolutionParameterf)(__GLcontext *gc,   GLenum target, GLenum pname, GLfloat params ){}
GLvoid GLAPIENTRY __gles(ConvolutionParameterfv)(__GLcontext *gc,   GLenum target, GLenum pname, const GLfloat *params ){}
GLvoid GLAPIENTRY __gles(ConvolutionParameteri)(__GLcontext *gc,   GLenum target, GLenum pname, GLint params ){}
GLvoid GLAPIENTRY __gles(ConvolutionParameteriv)(__GLcontext *gc,   GLenum target, GLenum pname, const GLint *params ){}
GLvoid GLAPIENTRY __gles(CopyConvolutionFilter1D)(__GLcontext *gc,   GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width ){}
GLvoid GLAPIENTRY __gles(CopyConvolutionFilter2D)(__GLcontext *gc,   GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height){}
GLvoid GLAPIENTRY __gles(GetConvolutionFilter)(__GLcontext *gc,   GLenum target, GLenum format, GLenum type, GLvoid *image ){}
GLvoid GLAPIENTRY __gles(GetConvolutionParameterfv)(__GLcontext *gc,   GLenum target, GLenum pname, GLfloat *params ){}
GLvoid GLAPIENTRY __gles(GetConvolutionParameteriv)(__GLcontext *gc,   GLenum target, GLenum pname, GLint *params ){}
GLvoid GLAPIENTRY __gles(SeparableFilter2D)(__GLcontext *gc,   GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column ){}
GLvoid GLAPIENTRY __gles(GetSeparableFilter)(__GLcontext *gc,   GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span ){}
GLvoid GLAPIENTRY __gles(Histogram)(__GLcontext *gc,   GLenum target, GLsizei width, GLenum internalformat, GLboolean sink ){}
GLvoid GLAPIENTRY __gles(ResetHistogram)(__GLcontext *gc,   GLenum target ){}
GLvoid GLAPIENTRY __gles(GetHistogram)(__GLcontext *gc,   GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values ){}
GLvoid GLAPIENTRY __gles(GetHistogramParameterfv)(__GLcontext *gc,   GLenum target, GLenum pname, GLfloat *params ){}
GLvoid GLAPIENTRY __gles(GetHistogramParameteriv)(__GLcontext *gc,   GLenum target, GLenum pname, GLint *params ){}
GLvoid GLAPIENTRY __gles(Minmax)(__GLcontext *gc,   GLenum target, GLenum internalformat, GLboolean sink ){}
GLvoid GLAPIENTRY __gles(ResetMinmax)(__GLcontext *gc,   GLenum target ){}
GLvoid GLAPIENTRY __gles(GetMinmax)(__GLcontext *gc,   GLenum target, GLboolean reset, GLenum format, GLenum types, GLvoid *values ){}
GLvoid GLAPIENTRY __gles(GetMinmaxParameterfv)(__GLcontext *gc,   GLenum target, GLenum pname,  GLfloat *params ){}
GLvoid GLAPIENTRY __gles(GetMinmaxParameteriv)(__GLcontext *gc,   GLenum target, GLenum pname, GLint *params ){}
GLvoid GLAPIENTRY __gles(ClientActiveTexture)(__GLcontext *gc,   GLenum texture ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord1d)(__GLcontext *gc,   GLenum target, GLdouble s ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord1dv)(__GLcontext *gc,   GLenum target, const GLdouble *v ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord1f)(__GLcontext *gc,   GLenum target, GLfloat s ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord1fv)(__GLcontext *gc,   GLenum target, const GLfloat *v ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord1i)(__GLcontext *gc,   GLenum target, GLint s ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord1iv)(__GLcontext *gc,   GLenum target, const GLint *v ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord1s)(__GLcontext *gc,   GLenum target, GLshort s ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord1sv)(__GLcontext *gc,   GLenum target, const GLshort *v ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord2d)(__GLcontext *gc,   GLenum target, GLdouble s, GLdouble t ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord2dv)(__GLcontext *gc,   GLenum target, const GLdouble *v ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord2f)(__GLcontext *gc,   GLenum target, GLfloat s, GLfloat t ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord2fv)(__GLcontext *gc,   GLenum target, const GLfloat *v ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord2i)(__GLcontext *gc,   GLenum target, GLint s, GLint t ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord2iv)(__GLcontext *gc,   GLenum target, const GLint *v ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord2s)(__GLcontext *gc,   GLenum target, GLshort s, GLshort t ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord2sv)(__GLcontext *gc,   GLenum target, const GLshort *v ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord3d)(__GLcontext *gc,   GLenum target, GLdouble s, GLdouble t, GLdouble r ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord3dv)(__GLcontext *gc,   GLenum target, const GLdouble *v ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord3f)(__GLcontext *gc,   GLenum target, GLfloat s, GLfloat t, GLfloat r ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord3fv)(__GLcontext *gc,   GLenum target, const GLfloat *v ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord3i)(__GLcontext *gc,   GLenum target, GLint s, GLint t, GLint r ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord3iv)(__GLcontext *gc,   GLenum target, const GLint *v ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord3s)(__GLcontext *gc,   GLenum target, GLshort s, GLshort t, GLshort r ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord3sv)(__GLcontext *gc,   GLenum target, const GLshort *v ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord4d)(__GLcontext *gc,   GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord4dv)(__GLcontext *gc,   GLenum target, const GLdouble *v ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord4f)(__GLcontext *gc,   GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord4fv)(__GLcontext *gc,   GLenum target, const GLfloat *v ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord4i)(__GLcontext *gc,   GLenum target, GLint s, GLint t, GLint r, GLint q ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord4iv)(__GLcontext *gc,   GLenum target, const GLint *v ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord4s)(__GLcontext *gc,   GLenum target, GLshort s, GLshort t, GLshort r, GLshort q ){}
GLvoid GLAPIENTRY __gles(MultiTexCoord4sv)(__GLcontext *gc,   GLenum target, const GLshort *v ){}
GLvoid GLAPIENTRY __gles(LoadTransposeMatrixd)(__GLcontext *gc,   const GLdouble m[16] ){}
GLvoid GLAPIENTRY __gles(LoadTransposeMatrixf)(__GLcontext *gc,   const GLfloat m[16] ){}
GLvoid GLAPIENTRY __gles(MultTransposeMatrixd)(__GLcontext *gc,   const GLdouble m[16] ){}
GLvoid GLAPIENTRY __gles(MultTransposeMatrixf)(__GLcontext *gc,   const GLfloat m[16] ){}
GLvoid GLAPIENTRY __gles(CompressedTexImage1D)(__GLcontext *gc,   GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data ){}
GLvoid GLAPIENTRY __gles(CompressedTexSubImage1D)(__GLcontext *gc,   GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data ){}
GLvoid GLAPIENTRY __gles(GetCompressedTexImage)(__GLcontext *gc,   GLenum target, GLint lod, GLvoid *img ){}
GLvoid GLAPIENTRY __gles(FogCoordf)(__GLcontext *gc,  GLfloat coord){}
GLvoid GLAPIENTRY __gles(FogCoordfv)(__GLcontext *gc,  const GLfloat *coord){}
GLvoid GLAPIENTRY __gles(FogCoordd)(__GLcontext *gc,  GLdouble coord){}
GLvoid GLAPIENTRY __gles(FogCoorddv)(__GLcontext *gc,  const GLdouble *coord){}
GLvoid GLAPIENTRY __gles(FogCoordPointer)(__GLcontext *gc,  GLenum type, GLsizei stride, const void *pointer){}
GLvoid GLAPIENTRY __gles(PointParameterf)(__GLcontext *gc,  GLenum pname, GLfloat param){}
GLvoid GLAPIENTRY __gles(PointParameterfv)(__GLcontext *gc,  GLenum pname, const GLfloat *params){}
GLvoid GLAPIENTRY __gles(PointParameteri)(__GLcontext *gc,  GLenum pname, GLint param){}
GLvoid GLAPIENTRY __gles(PointParameteriv)(__GLcontext *gc,  GLenum pname, const GLint *params){}
GLvoid GLAPIENTRY __gles(SecondaryColor3b)(__GLcontext *gc,  GLbyte red, GLbyte green, GLbyte blue){}
GLvoid GLAPIENTRY __gles(SecondaryColor3bv)(__GLcontext *gc,  const GLbyte *v){}
GLvoid GLAPIENTRY __gles(SecondaryColor3d)(__GLcontext *gc,  GLdouble red, GLdouble green, GLdouble blue){}
GLvoid GLAPIENTRY __gles(SecondaryColor3dv)(__GLcontext *gc,  const GLdouble *v){}
GLvoid GLAPIENTRY __gles(SecondaryColor3f)(__GLcontext *gc,  GLfloat red, GLfloat green, GLfloat blue){}
GLvoid GLAPIENTRY __gles(SecondaryColor3fv)(__GLcontext *gc,  const GLfloat *v){}
GLvoid GLAPIENTRY __gles(SecondaryColor3i)(__GLcontext *gc,  GLint red, GLint green, GLint blue){}
GLvoid GLAPIENTRY __gles(SecondaryColor3iv)(__GLcontext *gc,  const GLint *v){}
GLvoid GLAPIENTRY __gles(SecondaryColor3s)(__GLcontext *gc,  GLshort red, GLshort green, GLshort blue){}
GLvoid GLAPIENTRY __gles(SecondaryColor3sv)(__GLcontext *gc,  const GLshort *v){}
GLvoid GLAPIENTRY __gles(SecondaryColor3ub)(__GLcontext *gc,  GLubyte red, GLubyte green, GLubyte blue){}
GLvoid GLAPIENTRY __gles(SecondaryColor3ubv)(__GLcontext *gc,  const GLubyte *v){}
GLvoid GLAPIENTRY __gles(SecondaryColor3ui)(__GLcontext *gc,  GLuint red, GLuint green, GLuint blue){}
GLvoid GLAPIENTRY __gles(SecondaryColor3uiv)(__GLcontext *gc,  const GLuint *v){}
GLvoid GLAPIENTRY __gles(SecondaryColor3us)(__GLcontext *gc,  GLushort red, GLushort green, GLushort blue){}
GLvoid GLAPIENTRY __gles(SecondaryColor3usv)(__GLcontext *gc,  const GLushort *v){}
GLvoid GLAPIENTRY __gles(SecondaryColorPointer)(__GLcontext *gc,  GLint size, GLenum type, GLsizei stride, const void *pointer){}
GLvoid GLAPIENTRY __gles(WindowPos2d)(__GLcontext *gc,  GLdouble x, GLdouble y){}
GLvoid GLAPIENTRY __gles(WindowPos2dv)(__GLcontext *gc,  const GLdouble *v){}
GLvoid GLAPIENTRY __gles(WindowPos2f)(__GLcontext *gc,  GLfloat x, GLfloat y){}
GLvoid GLAPIENTRY __gles(WindowPos2fv)(__GLcontext *gc,  const GLfloat *v){}
GLvoid GLAPIENTRY __gles(WindowPos2i)(__GLcontext *gc,  GLint x, GLint y){}
GLvoid GLAPIENTRY __gles(WindowPos2iv)(__GLcontext *gc,  const GLint *v){}
GLvoid GLAPIENTRY __gles(WindowPos2s)(__GLcontext *gc,  GLshort x, GLshort y){}
GLvoid GLAPIENTRY __gles(WindowPos2sv)(__GLcontext *gc,  const GLshort *v){}
GLvoid GLAPIENTRY __gles(WindowPos3d)(__GLcontext *gc,  GLdouble x, GLdouble y, GLdouble z){}
GLvoid GLAPIENTRY __gles(WindowPos3dv)(__GLcontext *gc,  const GLdouble *v){}
GLvoid GLAPIENTRY __gles(WindowPos3f)(__GLcontext *gc,  GLfloat x, GLfloat y, GLfloat z){}
GLvoid GLAPIENTRY __gles(WindowPos3fv)(__GLcontext *gc,  const GLfloat *v){}
GLvoid GLAPIENTRY __gles(WindowPos3i)(__GLcontext *gc,  GLint x, GLint y, GLint z){}
GLvoid GLAPIENTRY __gles(WindowPos3iv)(__GLcontext *gc,  const GLint *v){}
GLvoid GLAPIENTRY __gles(WindowPos3s)(__GLcontext *gc,  GLshort x, GLshort y, GLshort z){}
GLvoid GLAPIENTRY __gles(WindowPos3sv)(__GLcontext *gc,  const GLshort *v){}
GLvoid GLAPIENTRY __gles(VertexAttrib1d)(__GLcontext *gc, GLuint indx, GLdouble x){}
GLvoid GLAPIENTRY __gles(VertexAttrib1dv)(__GLcontext *gc, GLuint indx, const GLdouble *values){}
GLvoid GLAPIENTRY __gles(VertexAttrib1s)(__GLcontext *gc, GLuint indx, GLshort x){}
GLvoid GLAPIENTRY __gles(VertexAttrib1sv)(__GLcontext *gc, GLuint indx, const GLshort * values){}
GLvoid GLAPIENTRY __gles(VertexAttrib2d)(__GLcontext *gc, GLuint indx, GLdouble x, GLdouble y){}
GLvoid GLAPIENTRY __gles(VertexAttrib2dv)(__GLcontext *gc, GLuint indx, const GLdouble * values){}
GLvoid GLAPIENTRY __gles(VertexAttrib2s)(__GLcontext *gc, GLuint indx, GLshort x, GLshort y){}
GLvoid GLAPIENTRY __gles(VertexAttrib2sv)(__GLcontext *gc, GLuint indx, const GLshort * values){}
GLvoid GLAPIENTRY __gles(VertexAttrib3d)(__GLcontext *gc, GLuint indx, GLdouble x, GLdouble y, GLdouble z){}
GLvoid GLAPIENTRY __gles(VertexAttrib3dv)(__GLcontext *gc, GLuint indx, const GLdouble * values){}
GLvoid GLAPIENTRY __gles(VertexAttrib3s)(__GLcontext *gc, GLuint indx, GLshort x, GLshort y, GLshort z){}
GLvoid GLAPIENTRY __gles(VertexAttrib3sv)(__GLcontext *gc, GLuint indx, const GLshort * values){}
GLvoid GLAPIENTRY __gles(VertexAttrib4Nbv)(__GLcontext *gc, GLuint indx, const GLbyte * values){}
GLvoid GLAPIENTRY __gles(VertexAttrib4Niv)(__GLcontext *gc, GLuint indx, const GLint * values){}
GLvoid GLAPIENTRY __gles(VertexAttrib4Nsv)(__GLcontext *gc, GLuint indx, const GLshort * values){}
GLvoid GLAPIENTRY __gles(VertexAttrib4Nub)(__GLcontext *gc, GLuint indx, GLubyte x, GLubyte y, GLubyte z, GLubyte w){}
GLvoid GLAPIENTRY __gles(VertexAttrib4Nubv)(__GLcontext *gc, GLuint indx, const GLubyte * values){}
GLvoid GLAPIENTRY __gles(VertexAttrib4Nuiv)(__GLcontext *gc, GLuint indx, const GLuint * values){}
GLvoid GLAPIENTRY __gles(VertexAttrib4Nusv)(__GLcontext *gc, GLuint indx, const GLushort * values){}
GLvoid GLAPIENTRY __gles(VertexAttrib4bv)(__GLcontext *gc, GLuint indx, const GLbyte * values){}
GLvoid GLAPIENTRY __gles(VertexAttrib4d)(__GLcontext *gc, GLuint indx, GLdouble x, GLdouble y, GLdouble z, GLdouble w){}
GLvoid GLAPIENTRY __gles(VertexAttrib4dv)(__GLcontext *gc, GLuint indx, const GLdouble * values){}
GLvoid GLAPIENTRY __gles(VertexAttrib4iv)(__GLcontext *gc, GLuint indx, const GLint * values){}
GLvoid GLAPIENTRY __gles(VertexAttrib4s)(__GLcontext *gc, GLuint indx, GLshort x, GLshort y, GLshort z, GLshort w){}
GLvoid GLAPIENTRY __gles(VertexAttrib4sv)(__GLcontext *gc, GLuint indx, const GLshort * values){}
GLvoid GLAPIENTRY __gles(VertexAttrib4ubv)(__GLcontext *gc, GLuint indx, const GLubyte * values){}
GLvoid GLAPIENTRY __gles(VertexAttrib4uiv)(__GLcontext *gc, GLuint indx, const GLuint * values){}
GLvoid GLAPIENTRY __gles(VertexAttrib4usv)(__GLcontext *gc, GLuint indx, const GLushort * values){}
GLvoid GLAPIENTRY __gles(DrawPixels)(__GLcontext *gc,   GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels ){}
GLvoid GLAPIENTRY __gles(CopyPixels)(__GLcontext *gc,   GLint x, GLint y,  GLsizei width, GLsizei height, GLenum type ){}
GLvoid GLAPIENTRY __gles(Indexub)(__GLcontext *gc,   GLubyte c ){}
GLvoid GLAPIENTRY __gles(Indexubv)(__GLcontext *gc,   const GLubyte *c ){}
GLvoid GLAPIENTRY __gles(GetVertexAttribdv) (__GLcontext *gc, GLuint index, GLenum pname, GLdouble* params){}
GLvoid GLAPIENTRY __gles(GetQueryObjectiv) (__GLcontext *gc, GLuint id, GLenum pname, GLint* params){}
GLvoid GLAPIENTRY __gles(GetBufferSubData) (__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data){}
GLvoid* GLAPIENTRY __gles(MapBuffer)(__GLcontext *gc, GLenum target, GLenum access){return gcvNULL;}
GLvoid GLAPIENTRY __gles(DeleteObjectARB) (__GLcontext *gc, UINT obj){}
GLvoid GLAPIENTRY __gles(GetInfoLogARB) (__GLcontext *gc, UINT obj, GLsizei maxLength, GLsizei *length, char *infoLog){}
GLvoid GLAPIENTRY __gles(ClampColorARB) (__GLcontext *gc, GLenum target, GLenum clamp){}

//GLvoid GLAPIENTRY __gles(GetObjectParameterivARB) (__GLcontext *gc, UINT obj, GLenum pname, GLint *params){}
GLboolean GLAPIENTRY __gles(IsRenderbufferEXT)( __GLcontext *gc,  GLuint renderbuffer){return GL_FALSE;}
GLvoid GLAPIENTRY __gles(BindRenderbufferEXT)(__GLcontext *gc,  GLenum target, GLuint renderbuffer){}
GLvoid GLAPIENTRY __gles(DeleteRenderbuffersEXT)(__GLcontext *gc,  GLsizei n, const GLuint *renderbuffers){}
GLvoid GLAPIENTRY __gles(GenRenderbuffersEXT)(__GLcontext *gc,  GLsizei n, GLuint *renderbuffers){}
GLvoid GLAPIENTRY __gles(RenderbufferStorageEXT)(__GLcontext *gc,  GLenum target, GLenum internalformat, GLsizei width, GLsizei height){}
GLvoid GLAPIENTRY __gles(GetRenderbufferParameterivEXT)(__GLcontext *gc,  GLenum target, GLenum pname, GLint* params){}
GLboolean GLAPIENTRY __gles(IsFramebufferEXT)(__GLcontext *gc,  GLuint framebuffer){return GL_FALSE;}
GLvoid GLAPIENTRY __gles(BindFramebufferEXT)(__GLcontext *gc,  GLenum target, GLuint framebuffer){}
GLvoid GLAPIENTRY __gles(DeleteFramebuffersEXT)(__GLcontext *gc,  GLsizei n, const GLuint *framebuffers){}
GLvoid GLAPIENTRY __gles(GenFramebuffersEXT)(__GLcontext *gc,  GLsizei n, GLuint *framebuffers){}
GLenum GLAPIENTRY __gles(CheckFramebufferStatusEXT)(__GLcontext *gc,  GLenum target){return GL_FRAMEBUFFER_COMPLETE;}
GLvoid GLAPIENTRY __gles(FramebufferTexture1DEXT)(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level){}
GLvoid GLAPIENTRY __gles(FramebufferTexture2DEXT)(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level){}
GLvoid GLAPIENTRY __gles(FramebufferTexture3DEXT)(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset){}
GLvoid GLAPIENTRY __gles(FramebufferRenderbufferEXT)(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer){}
GLvoid GLAPIENTRY __gles(GetFramebufferAttachmentParameterivEXT)(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum pname, GLint *params){}
GLvoid GLAPIENTRY __gles(GenerateMipmapEXT)(__GLcontext *gc,  GLenum target){}
GLvoid GLAPIENTRY __gles(BlitFramebufferEXT)(__GLcontext *gc,  GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,GLbitfield mask, GLenum filter){}
GLvoid GLAPIENTRY __gles(BindFragDataLocation) (__GLcontext *gc, GLuint program, GLuint colorNumber, const GLchar *name){}
GLvoid GLAPIENTRY __gles(ClampColor) (__GLcontext *gc, GLenum target, GLenum clamp){}
GLvoid GLAPIENTRY __gles(BeginConditionalRender)(__GLcontext *gc, GLuint id, GLenum mode){}
GLvoid GLAPIENTRY __gles(EndConditionalRender)(__GLcontext *gc){}
GLvoid GLAPIENTRY __gles(VertexAttribI1i)(__GLcontext *gc, GLuint index, GLint x){}
GLvoid GLAPIENTRY __gles(VertexAttribI2i)(__GLcontext *gc, GLuint index, GLint x, GLint y){}
GLvoid GLAPIENTRY __gles(VertexAttribI3i)(__GLcontext *gc, GLuint index, GLint x, GLint y, GLint z){}
GLvoid GLAPIENTRY __gles(VertexAttribI1ui)(__GLcontext *gc, GLuint index, GLuint x){}
GLvoid GLAPIENTRY __gles(VertexAttribI2ui)(__GLcontext *gc, GLuint index, GLuint x, GLuint y){}
GLvoid GLAPIENTRY __gles(VertexAttribI3ui)(__GLcontext *gc, GLuint index, GLuint x, GLuint y, GLuint z){}
GLvoid GLAPIENTRY __gles(VertexAttribI1iv)(__GLcontext *gc, GLuint index, const GLint *v){}
GLvoid GLAPIENTRY __gles(VertexAttribI2iv)(__GLcontext *gc, GLuint index, const GLint *v){}
GLvoid GLAPIENTRY __gles(VertexAttribI3iv)(__GLcontext *gc, GLuint index, const GLint *v){}
GLvoid GLAPIENTRY __gles(VertexAttribI1uiv)(__GLcontext *gc, GLuint index, const GLuint *v){}
GLvoid GLAPIENTRY __gles(VertexAttribI2uiv)(__GLcontext *gc, GLuint index, const GLuint *v){}
GLvoid GLAPIENTRY __gles(VertexAttribI3uiv)(__GLcontext *gc, GLuint index, const GLuint *v){}
GLvoid GLAPIENTRY __gles(VertexAttribI4bv)(__GLcontext *gc, GLuint index, const GLbyte *v){}
GLvoid GLAPIENTRY __gles(VertexAttribI4sv)(__GLcontext *gc, GLuint index, const GLshort *v){}
GLvoid GLAPIENTRY __gles(VertexAttribI4ubv)(__GLcontext *gc, GLuint index, const GLubyte *v){}
GLvoid GLAPIENTRY __gles(VertexAttribI4usv)(__GLcontext *gc, GLuint index, const GLushort *v){}
GLvoid GLAPIENTRY __gles(FramebufferTexture1D)(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level){}
GLvoid GLAPIENTRY __gles(FramebufferTexture3D)(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset){}
GLvoid GLAPIENTRY __gles(PrimitiveRestartIndex)(__GLcontext *gc, GLuint index){}
GLvoid GLAPIENTRY __gles(GetActiveUniformName)(__GLcontext *gc, GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName){}

GLvoid GLAPIENTRY __gles(MultiDrawArrays)(__GLcontext *gc, GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount){}
GLvoid GLAPIENTRY __gles(MultiDrawElements)(__GLcontext *gc, GLenum mode, const GLsizei *count, GLenum type, const GLvoid*const*indices, GLsizei primcount){}

#ifdef OPENGL40
/* OGL type */
//GLboolean GL_APIENTRY __glim_IsList(__GLcontext *gc,  GLuint list ){}
//GLvoid GL_APIENTRY __glim_DeleteLists(__GLcontext *gc,  GLuint list, GLsizei range ){}
//GLuint GL_APIENTRY __glim_GenLists(__GLcontext *gc,  GLsizei range ){}
//GLvoid GL_APIENTRY __glim_NewList(__GLcontext *gc,  GLuint list, GLenum mode ){}
//GLvoid GL_APIENTRY __glim_EndList(__GLcontext *gc){}
//GLvoid GL_APIENTRY __glim_CallList(__GLcontext *gc,  GLuint list ){}
//GLvoid GL_APIENTRY __glim_CallLists(__GLcontext *gc,  GLsizei n, GLenum type, const GLvoid *lists ){}
//GLvoid GL_APIENTRY __glim_ListBase(__GLcontext *gc,  GLuint base ){}
//GLvoid GL_APIENTRY __glim_Begin(__GLcontext *gc,  GLenum mode ){}
//GLvoid GL_APIENTRY __glim_Bitmap(__GLcontext *gc,  GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap ){}
//GLvoid GL_APIENTRY __glim_Color3b(__GLcontext *gc,  GLbyte red, GLbyte green, GLbyte blue ){}
//GLvoid GL_APIENTRY __glim_Color3d(__GLcontext *gc,  GLdouble red, GLdouble green, GLdouble blue ){}
//GLvoid GL_APIENTRY __glim_Color3f(__GLcontext *gc,  GLfloat red, GLfloat green, GLfloat blue ){}
//GLvoid GL_APIENTRY __glim_Color3i(__GLcontext *gc,  GLint red, GLint green, GLint blue ){}
//GLvoid GL_APIENTRY __glim_Color3s(__GLcontext *gc,  GLshort red, GLshort green, GLshort blue ){}
//GLvoid GL_APIENTRY __glim_Color3ub(__GLcontext *gc,  GLubyte red, GLubyte green, GLubyte blue ){}
//GLvoid GL_APIENTRY __glim_Color3ui(__GLcontext *gc,  GLuint red, GLuint green, GLuint blue ){}
//GLvoid GL_APIENTRY __glim_Color3us(__GLcontext *gc,  GLushort red, GLushort green, GLushort blue ){}
//GLvoid GL_APIENTRY __glim_Color3bv(__GLcontext *gc,  const GLbyte *v ){}
//GLvoid GL_APIENTRY __glim_Color3dv(__GLcontext *gc,  const GLdouble *v ){}
//GLvoid GL_APIENTRY __glim_Color3fv(__GLcontext *gc,  const GLfloat *v ){}
//GLvoid GL_APIENTRY __glim_Color3iv(__GLcontext *gc,  const GLint *v ){}
//GLvoid GL_APIENTRY __glim_Color3sv(__GLcontext *gc,  const GLshort *v ){}
//GLvoid GL_APIENTRY __glim_Color3ubv(__GLcontext *gc,  const GLubyte *v ){}
//GLvoid GL_APIENTRY __glim_Color3uiv(__GLcontext *gc,  const GLuint *v ){}
//GLvoid GL_APIENTRY __glim_Color3usv(__GLcontext *gc,  const GLushort *v ){}
//GLvoid GL_APIENTRY __glim_Color4b(__GLcontext *gc,  GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha ){}
//GLvoid GL_APIENTRY __glim_Color4d(__GLcontext *gc,  GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha ){}
//GLvoid GL_APIENTRY __glim_Color4f(__GLcontext *gc,  GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ){}
//GLvoid GL_APIENTRY __glim_Color4i(__GLcontext *gc,  GLint red, GLint green, GLint blue, GLint alpha ){}
//GLvoid GL_APIENTRY __glim_Color4s(__GLcontext *gc,  GLshort red, GLshort green, GLshort blue, GLshort alpha ){}
//GLvoid GL_APIENTRY __glim_Color4ub(__GLcontext *gc,  GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha ){}
//GLvoid GL_APIENTRY __glim_Color4ui(__GLcontext *gc,  GLuint red, GLuint green, GLuint blue, GLuint alpha ){}
//GLvoid GL_APIENTRY __glim_Color4us(__GLcontext *gc,  GLushort red, GLushort green, GLushort blue, GLushort alpha ){}
//GLvoid GL_APIENTRY __glim_Color4bv(__GLcontext *gc,  const GLbyte *v ){}
//GLvoid GL_APIENTRY __glim_Color4dv(__GLcontext *gc,  const GLdouble *v ){}
//GLvoid GL_APIENTRY __glim_Color4fv(__GLcontext *gc,  const GLfloat *v ){}
//GLvoid GL_APIENTRY __glim_Color4iv(__GLcontext *gc,  const GLint *v ){}
//GLvoid GL_APIENTRY __glim_Color4sv(__GLcontext *gc,  const GLshort *v ){}
//GLvoid GL_APIENTRY __glim_Color4ubv(__GLcontext *gc,  const GLubyte *v ){}
//GLvoid GL_APIENTRY __glim_Color4uiv(__GLcontext *gc,  const GLuint *v ){}
//GLvoid GL_APIENTRY __glim_Color4usv(__GLcontext *gc,  const GLushort *v ){}
//GLvoid GL_APIENTRY __glim_EdgeFlag(__GLcontext *gc,  GLboolean flag ){}
//GLvoid GL_APIENTRY __glim_EdgeFlagv(__GLcontext *gc,  const GLboolean *flag ){}
//GLvoid GL_APIENTRY __glim_End(__GLcontext *gc){}
//GLvoid GL_APIENTRY __glim_Indexd(__GLcontext *gc,  GLdouble c ){}
//GLvoid GL_APIENTRY __glim_Indexf(__GLcontext *gc,  GLfloat c ){}
//GLvoid GL_APIENTRY __glim_Indexi(__GLcontext *gc,  GLint c ){}
//GLvoid GL_APIENTRY __glim_Indexs(__GLcontext *gc,  GLshort c ){}
//GLvoid GL_APIENTRY __glim_Indexdv(__GLcontext *gc,  const GLdouble *c ){}
//GLvoid GL_APIENTRY __glim_Indexfv(__GLcontext *gc,  const GLfloat *c ){}
//GLvoid GL_APIENTRY __glim_Indexiv(__GLcontext *gc,  const GLint *c ){}
//GLvoid GL_APIENTRY __glim_Indexsv(__GLcontext *gc,  const GLshort *c ){}
//GLvoid GL_APIENTRY __glim_Normal3b(__GLcontext *gc,  GLbyte nx, GLbyte ny, GLbyte nz ){}
//GLvoid GL_APIENTRY __glim_Normal3d(__GLcontext *gc,  GLdouble nx, GLdouble ny, GLdouble nz ){}
//GLvoid GL_APIENTRY __glim_Normal3f(__GLcontext *gc,  GLfloat nx, GLfloat ny, GLfloat nz ){}
//GLvoid GL_APIENTRY __glim_Normal3i(__GLcontext *gc,  GLint nx, GLint ny, GLint nz ){}
//GLvoid GL_APIENTRY __glim_Normal3s(__GLcontext *gc,  GLshort nx, GLshort ny, GLshort nz ){}
//GLvoid GL_APIENTRY __glim_Normal3bv(__GLcontext *gc,  const GLbyte *v ){}
//GLvoid GL_APIENTRY __glim_Normal3dv(__GLcontext *gc,  const GLdouble *v ){}
//GLvoid GL_APIENTRY __glim_Normal3fv(__GLcontext *gc,  const GLfloat *v ){}
//GLvoid GL_APIENTRY __glim_Normal3iv(__GLcontext *gc,  const GLint *v ){}
//GLvoid GL_APIENTRY __glim_Normal3sv(__GLcontext *gc,  const GLshort *v ){}
//GLvoid GL_APIENTRY __glim_RasterPos2d(__GLcontext *gc,  GLdouble x, GLdouble y ){}
//GLvoid GL_APIENTRY __glim_RasterPos2f(__GLcontext *gc,  GLfloat x, GLfloat y ){}
//GLvoid GL_APIENTRY __glim_RasterPos2i(__GLcontext *gc,  GLint x, GLint y ){}
//GLvoid GL_APIENTRY __glim_RasterPos2s(__GLcontext *gc,  GLshort x, GLshort y ){}
//GLvoid GL_APIENTRY __glim_RasterPos3d(__GLcontext *gc,  GLdouble x, GLdouble y, GLdouble z ){}
//GLvoid GL_APIENTRY __glim_RasterPos3f(__GLcontext *gc,  GLfloat x, GLfloat y, GLfloat z ){}
//GLvoid GL_APIENTRY __glim_RasterPos3i(__GLcontext *gc,  GLint x, GLint y, GLint z ){}
//GLvoid GL_APIENTRY __glim_RasterPos3s(__GLcontext *gc,  GLshort x, GLshort y, GLshort z ){}
//GLvoid GL_APIENTRY __glim_RasterPos4d(__GLcontext *gc,  GLdouble x, GLdouble y, GLdouble z, GLdouble w ){}
//GLvoid GL_APIENTRY __glim_RasterPos4f(__GLcontext *gc,  GLfloat x, GLfloat y, GLfloat z, GLfloat w ){}
//GLvoid GL_APIENTRY __glim_RasterPos4i(__GLcontext *gc,  GLint x, GLint y, GLint z, GLint w ){}
//GLvoid GL_APIENTRY __glim_RasterPos4s(__GLcontext *gc,  GLshort x, GLshort y, GLshort z, GLshort w ){}
//GLvoid GL_APIENTRY __glim_RasterPos2dv(__GLcontext *gc,  const GLdouble *v ){}
//GLvoid GL_APIENTRY __glim_RasterPos2fv(__GLcontext *gc,  const GLfloat *v ){}
//GLvoid GL_APIENTRY __glim_RasterPos2iv(__GLcontext *gc,  const GLint *v ){}
//GLvoid GL_APIENTRY __glim_RasterPos2sv(__GLcontext *gc,  const GLshort *v ){}
//GLvoid GL_APIENTRY __glim_RasterPos3dv(__GLcontext *gc,  const GLdouble *v ){}
//GLvoid GL_APIENTRY __glim_RasterPos3fv(__GLcontext *gc,  const GLfloat *v ){}
//GLvoid GL_APIENTRY __glim_RasterPos3iv(__GLcontext *gc,  const GLint *v ){}
//GLvoid GL_APIENTRY __glim_RasterPos3sv(__GLcontext *gc,  const GLshort *v ){}
//GLvoid GL_APIENTRY __glim_RasterPos4dv(__GLcontext *gc,  const GLdouble *v ){}
//GLvoid GL_APIENTRY __glim_RasterPos4fv(__GLcontext *gc,  const GLfloat *v ){}
//GLvoid GL_APIENTRY __glim_RasterPos4iv(__GLcontext *gc,  const GLint *v ){}
//GLvoid GL_APIENTRY __glim_RasterPos4sv(__GLcontext *gc,  const GLshort *v ){}
//GLvoid GL_APIENTRY __glim_Rectd(__GLcontext *gc,  GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2 ){}
//GLvoid GL_APIENTRY __glim_Rectf(__GLcontext *gc,  GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 ){}
//GLvoid GL_APIENTRY __glim_Recti(__GLcontext *gc,  GLint x1, GLint y1, GLint x2, GLint y2 ){}
//GLvoid GL_APIENTRY __glim_Rects(__GLcontext *gc,  GLshort x1, GLshort y1, GLshort x2, GLshort y2 ){}
//GLvoid GL_APIENTRY __glim_Rectdv(__GLcontext *gc,  const GLdouble *v1, const GLdouble *v2 ){}
//GLvoid GL_APIENTRY __glim_Rectfv(__GLcontext *gc,  const GLfloat *v1, const GLfloat *v2 ){}
//GLvoid GL_APIENTRY __glim_Rectiv(__GLcontext *gc,  const GLint *v1, const GLint *v2 ){}
//GLvoid GL_APIENTRY __glim_Rectsv(__GLcontext *gc,  const GLshort *v1, const GLshort *v2 ){}
//GLvoid GL_APIENTRY __glim_TexCoord1d(__GLcontext *gc,  GLdouble s ){}
//GLvoid GL_APIENTRY __glim_TexCoord1f(__GLcontext *gc,  GLfloat s ){}
//GLvoid GL_APIENTRY __glim_TexCoord1i(__GLcontext *gc,  GLint s ){}
//GLvoid GL_APIENTRY __glim_TexCoord1s(__GLcontext *gc,  GLshort s ){}
//GLvoid GL_APIENTRY __glim_TexCoord2d(__GLcontext *gc,  GLdouble s, GLdouble t ){}
//GLvoid GL_APIENTRY __glim_TexCoord2f(__GLcontext *gc,  GLfloat s, GLfloat t ){}
//GLvoid GL_APIENTRY __glim_TexCoord2i(__GLcontext *gc,  GLint s, GLint t ){}
//GLvoid GL_APIENTRY __glim_TexCoord2s(__GLcontext *gc,  GLshort s, GLshort t ){}
//GLvoid GL_APIENTRY __glim_TexCoord3d(__GLcontext *gc,  GLdouble s, GLdouble t, GLdouble r ){}
//GLvoid GL_APIENTRY __glim_TexCoord3f(__GLcontext *gc,  GLfloat s, GLfloat t, GLfloat r ){}
//GLvoid GL_APIENTRY __glim_TexCoord3i(__GLcontext *gc,  GLint s, GLint t, GLint r ){}
//GLvoid GL_APIENTRY __glim_TexCoord3s(__GLcontext *gc,  GLshort s, GLshort t, GLshort r ){}
//GLvoid GL_APIENTRY __glim_TexCoord4d(__GLcontext *gc,  GLdouble s, GLdouble t, GLdouble r, GLdouble q ){}
//GLvoid GL_APIENTRY __glim_TexCoord4f(__GLcontext *gc,  GLfloat s, GLfloat t, GLfloat r, GLfloat q ){}
//GLvoid GL_APIENTRY __glim_TexCoord4i(__GLcontext *gc,  GLint s, GLint t, GLint r, GLint q ){}
//GLvoid GL_APIENTRY __glim_TexCoord4s(__GLcontext *gc,  GLshort s, GLshort t, GLshort r, GLshort q ){}
//GLvoid GL_APIENTRY __glim_TexCoord1dv(__GLcontext *gc,  const GLdouble *v ){}
//GLvoid GL_APIENTRY __glim_TexCoord1fv(__GLcontext *gc,  const GLfloat *v ){}
//GLvoid GL_APIENTRY __glim_TexCoord1iv(__GLcontext *gc,  const GLint *v ){}
//GLvoid GL_APIENTRY __glim_TexCoord1sv(__GLcontext *gc,  const GLshort *v ){}
//GLvoid GL_APIENTRY __glim_TexCoord2dv(__GLcontext *gc,  const GLdouble *v ){}
//GLvoid GL_APIENTRY __glim_TexCoord2fv(__GLcontext *gc,  const GLfloat *v ){}
//GLvoid GL_APIENTRY __glim_TexCoord2iv(__GLcontext *gc,  const GLint *v ){}
//GLvoid GL_APIENTRY __glim_TexCoord2sv(__GLcontext *gc,  const GLshort *v ){}
//GLvoid GL_APIENTRY __glim_TexCoord3dv(__GLcontext *gc,  const GLdouble *v ){}
//GLvoid GL_APIENTRY __glim_TexCoord3fv(__GLcontext *gc,  const GLfloat *v ){}
//GLvoid GL_APIENTRY __glim_TexCoord3iv(__GLcontext *gc,  const GLint *v ){}
//GLvoid GL_APIENTRY __glim_TexCoord3sv(__GLcontext *gc,  const GLshort *v ){}
//GLvoid GL_APIENTRY __glim_TexCoord4dv(__GLcontext *gc,  const GLdouble *v ){}
//GLvoid GL_APIENTRY __glim_TexCoord4fv(__GLcontext *gc,  const GLfloat *v ){}
//GLvoid GL_APIENTRY __glim_TexCoord4iv(__GLcontext *gc,  const GLint *v ){}
//GLvoid GL_APIENTRY __glim_TexCoord4sv(__GLcontext *gc,  const GLshort *v ){}
//GLvoid GL_APIENTRY __glim_Vertex2d(__GLcontext *gc,  GLdouble x, GLdouble y ){}
//GLvoid GL_APIENTRY __glim_Vertex2f(__GLcontext *gc,  GLfloat x, GLfloat y ){}
//GLvoid GL_APIENTRY __glim_Vertex2i(__GLcontext *gc,  GLint x, GLint y ){}
//GLvoid GL_APIENTRY __glim_Vertex2s(__GLcontext *gc,  GLshort x, GLshort y ){}
//GLvoid GL_APIENTRY __glim_Vertex3d(__GLcontext *gc,  GLdouble x, GLdouble y, GLdouble z ){}
//GLvoid GL_APIENTRY __glim_Vertex3f(__GLcontext *gc,  GLfloat x, GLfloat y, GLfloat z ){}
//GLvoid GL_APIENTRY __glim_Vertex3i(__GLcontext *gc,  GLint x, GLint y, GLint z ){}
//GLvoid GL_APIENTRY __glim_Vertex3s(__GLcontext *gc,  GLshort x, GLshort y, GLshort z ){}
//GLvoid GL_APIENTRY __glim_Vertex4d(__GLcontext *gc,  GLdouble x, GLdouble y, GLdouble z, GLdouble w ){}
//GLvoid GL_APIENTRY __glim_Vertex4f(__GLcontext *gc,  GLfloat x, GLfloat y, GLfloat z, GLfloat w ){}
//GLvoid GL_APIENTRY __glim_Vertex4i(__GLcontext *gc,  GLint x, GLint y, GLint z, GLint w ){}
//GLvoid GL_APIENTRY __glim_Vertex4s(__GLcontext *gc,  GLshort x, GLshort y, GLshort z, GLshort w ){}
//GLvoid GL_APIENTRY __glim_Vertex2dv(__GLcontext *gc,  const GLdouble *v ){}
//GLvoid GL_APIENTRY __glim_Vertex2fv(__GLcontext *gc,  const GLfloat *v ){}
//GLvoid GL_APIENTRY __glim_Vertex2iv(__GLcontext *gc,  const GLint *v ){}
//GLvoid GL_APIENTRY __glim_Vertex2sv(__GLcontext *gc,  const GLshort *v ){}
//GLvoid GL_APIENTRY __glim_Vertex3dv(__GLcontext *gc,  const GLdouble *v ){}
//GLvoid GL_APIENTRY __glim_Vertex3fv(__GLcontext *gc,  const GLfloat *v ){}
//GLvoid GL_APIENTRY __glim_Vertex3iv(__GLcontext *gc,  const GLint *v ){}
//GLvoid GL_APIENTRY __glim_Vertex3sv(__GLcontext *gc,  const GLshort *v ){}
//GLvoid GL_APIENTRY __glim_Vertex4dv(__GLcontext *gc,  const GLdouble *v ){}
//GLvoid GL_APIENTRY __glim_Vertex4fv(__GLcontext *gc,  const GLfloat *v ){}
//GLvoid GL_APIENTRY __glim_Vertex4iv(__GLcontext *gc,  const GLint *v ){}
//GLvoid GL_APIENTRY __glim_Vertex4sv(__GLcontext *gc,  const GLshort *v ){}
//GLvoid GL_APIENTRY __glim_ClipPlane(__GLcontext *gc,  GLenum plane, const GLdouble *equation ){}
//GLvoid GL_APIENTRY __glim_ColorMaterial(__GLcontext *gc,  GLenum face, GLenum mode ){}
//GLvoid GL_APIENTRY __glim_Fogf(__GLcontext *gc,  GLenum pname, GLfloat param ){}
//GLvoid GL_APIENTRY __glim_Fogi(__GLcontext *gc,  GLenum pname, GLint param ){}
//GLvoid GL_APIENTRY __glim_Fogfv(__GLcontext *gc,  GLenum pname, const GLfloat *params ){}
//GLvoid GL_APIENTRY __glim_Fogiv(__GLcontext *gc,  GLenum pname, const GLint *params ){}
//GLvoid GL_APIENTRY __glim_Lightf(__GLcontext *gc,  GLenum light, GLenum pname, GLfloat param ){}
//GLvoid GL_APIENTRY __glim_Lighti(__GLcontext *gc,  GLenum light, GLenum pname, GLint param ){}
//GLvoid GL_APIENTRY __glim_Lightfv(__GLcontext *gc,  GLenum light, GLenum pname, const GLfloat *params ){}
//GLvoid GL_APIENTRY __glim_Lightiv(__GLcontext *gc,  GLenum light, GLenum pname, const GLint *params ){}
//GLvoid GL_APIENTRY __glim_LightModelf(__GLcontext *gc,  GLenum pname, GLfloat param ){}
//GLvoid GL_APIENTRY __glim_LightModeli(__GLcontext *gc,  GLenum pname, GLint param ){}
//GLvoid GL_APIENTRY __glim_LightModelfv(__GLcontext *gc,  GLenum pname, const GLfloat *params ){}
//GLvoid GL_APIENTRY __glim_LightModeliv(__GLcontext *gc,  GLenum pname, const GLint *params ){}
//GLvoid GL_APIENTRY __glim_LineStipple(__GLcontext *gc,  GLint factor, GLushort pattern ){}
//GLvoid GL_APIENTRY __glim_Materialf(__GLcontext *gc,  GLenum face, GLenum pname, GLfloat param ){}
//GLvoid GL_APIENTRY __glim_Materiali(__GLcontext *gc,  GLenum face, GLenum pname, GLint param ){}
//GLvoid GL_APIENTRY __glim_Materialfv(__GLcontext *gc,  GLenum face, GLenum pname, const GLfloat *params ){}
//GLvoid GL_APIENTRY __glim_Materialiv(__GLcontext *gc,  GLenum face, GLenum pname, const GLint *params ){}
//GLvoid GL_APIENTRY __glim_PointSize(__GLcontext *gc,  GLfloat size ){}
//GLvoid GL_APIENTRY __glim_PolygonMode(__GLcontext *gc,  GLenum face, GLenum mode ){}
//GLvoid GL_APIENTRY __glim_PolygonStipple(__GLcontext *gc,  const GLubyte *mask ){}
//GLvoid GL_APIENTRY __glim_ShadeModel(__GLcontext *gc,  GLenum mode ){}
//GLvoid GL_APIENTRY __glim_TexImage1D(__GLcontext *gc,  GLenum target, GLint level, GLint internalFormat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels ){}
//GLvoid GL_APIENTRY __glim_TexEnvf(__GLcontext *gc,  GLenum target, GLenum pname, GLfloat param ){}
//GLvoid GL_APIENTRY __glim_TexEnvi(__GLcontext *gc,  GLenum target, GLenum pname, GLint param ){}
//GLvoid GL_APIENTRY __glim_TexEnvfv(__GLcontext *gc,  GLenum target, GLenum pname, const GLfloat *params ){}
//GLvoid GL_APIENTRY __glim_TexEnviv(__GLcontext *gc,  GLenum target, GLenum pname, const GLint *params ){}
//GLvoid GL_APIENTRY __glim_TexGend(__GLcontext *gc,  GLenum coord, GLenum pname, GLdouble param ){}
//GLvoid GL_APIENTRY __glim_TexGenf(__GLcontext *gc,  GLenum coord, GLenum pname, GLfloat param ){}
//GLvoid GL_APIENTRY __glim_TexGeni(__GLcontext *gc,  GLenum coord, GLenum pname, GLint param ){}
//GLvoid GL_APIENTRY __glim_TexGendv(__GLcontext *gc,  GLenum coord, GLenum pname, const GLdouble *params ){}
//GLvoid GL_APIENTRY __glim_TexGenfv(__GLcontext *gc,  GLenum coord, GLenum pname, const GLfloat *params ){}
//GLvoid GL_APIENTRY __glim_TexGeniv(__GLcontext *gc,  GLenum coord, GLenum pname, const GLint *params ){}
//GLvoid GL_APIENTRY __glim_FeedbackBuffer(__GLcontext *gc,  GLsizei size, GLenum type, GLfloat *buffer ){}
//GLvoid GL_APIENTRY __glim_SelectBuffer(__GLcontext *gc,  GLsizei size, GLuint *buffer ){}
//GLint GL_APIENTRY __glim_RenderMode(__GLcontext *gc,  GLenum mode ){}
//GLvoid GL_APIENTRY __glim_InitNames(__GLcontext *gc){}
//GLvoid GL_APIENTRY __glim_LoadName(__GLcontext *gc,  GLuint name ){}
//GLvoid GL_APIENTRY __glim_PushName(__GLcontext *gc,  GLuint name ){}
//GLvoid GL_APIENTRY __glim_PopName(__GLcontext *gc){}
//GLvoid GL_APIENTRY __glim_PassThrough(__GLcontext *gc,  GLfloat token ){}
//GLvoid GL_APIENTRY __glim_DrawBuffer(__GLcontext *gc,  GLenum mode ){}
//GLvoid GL_APIENTRY __glim_ClearAccum(__GLcontext *gc,  GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ){}
//GLvoid GL_APIENTRY __glim_ClearIndex(__GLcontext *gc,  GLfloat c ){}
//GLvoid GL_APIENTRY __glim_ClearDepth(__GLcontext *gc,  GLclampd depth ){}
//GLvoid GL_APIENTRY __glim_IndexMask(__GLcontext *gc,  GLuint mask ){}
//GLvoid GL_APIENTRY __glim_Accum(__GLcontext *gc,  GLenum op, GLfloat value ){}
//GLvoid GL_APIENTRY __glim_PushAttrib(__GLcontext *gc,  GLbitfield mask ){}
//GLvoid GL_APIENTRY __glim_PopAttrib(__GLcontext *gc){}
//GLvoid GL_APIENTRY __glim_Map1d(__GLcontext *gc,  GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points ){}
//GLvoid GL_APIENTRY __glim_Map1f(__GLcontext *gc,  GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points ){}
//GLvoid GL_APIENTRY __glim_Map2d(__GLcontext *gc,  GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points ){}
//GLvoid GL_APIENTRY __glim_Map2f(__GLcontext *gc,  GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points ){}
//GLvoid GL_APIENTRY __glim_MapGrid1d(__GLcontext *gc,  GLint un, GLdouble u1, GLdouble u2 ){}
//GLvoid GL_APIENTRY __glim_MapGrid1f(__GLcontext *gc,  GLint un, GLfloat u1, GLfloat u2 ){}
//GLvoid GL_APIENTRY __glim_MapGrid2d(__GLcontext *gc,  GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2 ){}
//GLvoid GL_APIENTRY __glim_MapGrid2f(__GLcontext *gc,  GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2 ){}
//GLvoid GL_APIENTRY __glim_EvalCoord1d(__GLcontext *gc,  GLdouble u ){}
//GLvoid GL_APIENTRY __glim_EvalCoord1f(__GLcontext *gc,  GLfloat u ){}
//GLvoid GL_APIENTRY __glim_EvalCoord1dv(__GLcontext *gc,  const GLdouble *u ){}
//GLvoid GL_APIENTRY __glim_EvalCoord1fv(__GLcontext *gc,  const GLfloat *u ){}
//GLvoid GL_APIENTRY __glim_EvalCoord2d(__GLcontext *gc,  GLdouble u, GLdouble v ){}
//GLvoid GL_APIENTRY __glim_EvalCoord2f(__GLcontext *gc,  GLfloat u, GLfloat v ){}
//GLvoid GL_APIENTRY __glim_EvalCoord2dv(__GLcontext *gc,  const GLdouble *u ){}
//GLvoid GL_APIENTRY __glim_EvalCoord2fv(__GLcontext *gc,  const GLfloat *u ){}
//GLvoid GL_APIENTRY __glim_EvalPoint1(__GLcontext *gc,  GLint i ){}
//GLvoid GL_APIENTRY __glim_EvalPoint2(__GLcontext *gc,  GLint i, GLint j ){}
//GLvoid GL_APIENTRY __glim_EvalMesh1(__GLcontext *gc,  GLenum mode, GLint i1, GLint i2 ){}
//GLvoid GL_APIENTRY __glim_EvalMesh2(__GLcontext *gc,  GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2 ){}
//GLvoid GL_APIENTRY __glim_AlphaFunc(__GLcontext *gc,  GLenum func, GLclampf ref ){}
//GLvoid GL_APIENTRY __glim_LogicOp(__GLcontext *gc,  GLenum opcode ){}
//GLvoid GL_APIENTRY __glim_PixelZoom(__GLcontext *gc,  GLfloat xfactor, GLfloat yfactor ){}
//GLvoid GL_APIENTRY __glim_PixelTransferf(__GLcontext *gc,  GLenum pname, GLfloat param ){}
//GLvoid GL_APIENTRY __glim_PixelTransferi(__GLcontext *gc,  GLenum pname, GLint param ){}
//GLvoid GL_APIENTRY __glim_PixelStoref(__GLcontext *gc,  GLenum pname, GLfloat param ){}
//GLvoid GL_APIENTRY __glim_PixelMapfv(__GLcontext *gc,  GLenum map, GLsizei mapsize, const GLfloat *values ){}
//GLvoid GL_APIENTRY __glim_PixelMapuiv(__GLcontext *gc,  GLenum map, GLsizei mapsize, const GLuint *values ){}
//GLvoid GL_APIENTRY __glim_PixelMapusv(__GLcontext *gc,  GLenum map, GLsizei mapsize, const GLushort *values ){}
//GLvoid GL_APIENTRY __glim_GetClipPlane(__GLcontext *gc,  GLenum plane, GLdouble *equation ){}
GLvoid GL_APIENTRY __glim_GetDoublev(__GLcontext *gc,  GLenum pname, GLdouble *params ){}
//GLvoid GL_APIENTRY __glim_GetLightfv(__GLcontext *gc,  GLenum light, GLenum pname, GLfloat *params ){}
//GLvoid GL_APIENTRY __glim_GetLightiv(__GLcontext *gc,  GLenum light, GLenum pname, GLint *params ){}
//GLvoid GL_APIENTRY __glim_GetMapdv(__GLcontext *gc,  GLenum target, GLenum query, GLdouble *v ){}
//GLvoid GL_APIENTRY __glim_GetMapfv(__GLcontext *gc,  GLenum target, GLenum query, GLfloat *v ){}
//GLvoid GL_APIENTRY __glim_GetMapiv(__GLcontext *gc,  GLenum target, GLenum query, GLint *v ){}
//GLvoid GL_APIENTRY __glim_GetMaterialfv(__GLcontext *gc,  GLenum face, GLenum pname, GLfloat *params ){}
//GLvoid GL_APIENTRY __glim_GetMaterialiv(__GLcontext *gc,  GLenum face, GLenum pname, GLint *params ){}
GLvoid GL_APIENTRY __glim_GetPixelMapfv(__GLcontext *gc,  GLenum map, GLfloat *values ){}
GLvoid GL_APIENTRY __glim_GetPixelMapuiv(__GLcontext *gc,  GLenum map, GLuint *values ){}
GLvoid GL_APIENTRY __glim_GetPixelMapusv(__GLcontext *gc,  GLenum map, GLushort *values ){}
//GLvoid GL_APIENTRY __glim_GetPolygonStipple(__GLcontext *gc,  GLubyte *mask ){}
//GLvoid GL_APIENTRY __glim_GetTexEnvfv(__GLcontext *gc,  GLenum target, GLenum pname, GLfloat *params ){}
//GLvoid GL_APIENTRY __glim_GetTexEnviv(__GLcontext *gc,  GLenum target, GLenum pname, GLint *params ){}
//GLvoid GL_APIENTRY __glim_GetTexGendv(__GLcontext *gc,  GLenum coord, GLenum pname, GLdouble *params ){}
//GLvoid GL_APIENTRY __glim_GetTexGenfv(__GLcontext *gc,  GLenum coord, GLenum pname, GLfloat *params ){}
//GLvoid GL_APIENTRY __glim_GetTexGeniv(__GLcontext *gc,  GLenum coord, GLenum pname, GLint *params ){}
GLvoid GL_APIENTRY __glim_GetTexImage(__GLcontext *gc,  GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels ){}
//GLvoid GL_APIENTRY __glim_DepthRange(__GLcontext *gc,  GLclampd near_val, GLclampd far_val ){}
//GLvoid GL_APIENTRY __glim_Frustum(__GLcontext *gc,  GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val ){}
//GLvoid GL_APIENTRY __glim_LoadIdentity(__GLcontext *gc){}
//GLvoid GL_APIENTRY __glim_LoadMatrixd(__GLcontext *gc,  const GLdouble *m ){}
//GLvoid GL_APIENTRY __glim_LoadMatrixf(__GLcontext *gc,  const GLfloat *m ){}
//GLvoid GL_APIENTRY __glim_MatrixMode(__GLcontext *gc,  GLenum mode ){}
//GLvoid GL_APIENTRY __glim_MultMatrixd(__GLcontext *gc,  const GLdouble *m ){}
//GLvoid GL_APIENTRY __glim_MultMatrixf(__GLcontext *gc,  const GLfloat *m ){}
//GLvoid GL_APIENTRY __glim_Ortho(__GLcontext *gc,  GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val ){}
//GLvoid GL_APIENTRY __glim_PushMatrix(__GLcontext *gc){}
//GLvoid GL_APIENTRY __glim_PopMatrix(__GLcontext *gc){}
//GLvoid GL_APIENTRY __glim_Rotated(__GLcontext *gc,  GLdouble angle, GLdouble x, GLdouble y, GLdouble z ){}
//GLvoid GL_APIENTRY __glim_Rotatef(__GLcontext *gc,  GLfloat angle, GLfloat x, GLfloat y, GLfloat z ){}
//GLvoid GL_APIENTRY __glim_Scaled(__GLcontext *gc,  GLdouble x, GLdouble y, GLdouble z ){}
//GLvoid GL_APIENTRY __glim_Scalef(__GLcontext *gc,  GLfloat x, GLfloat y, GLfloat z ){}
//GLvoid GL_APIENTRY __glim_Translated(__GLcontext *gc,  GLdouble x, GLdouble y, GLdouble z ){}
//GLvoid GL_APIENTRY __glim_Translatef(__GLcontext *gc,  GLfloat x, GLfloat y, GLfloat z ){}
//GLvoid GL_APIENTRY __glim_ArrayElement(__GLcontext *gc,  GLint i ){}
//GLvoid GL_APIENTRY __glim_ColorPointer(__GLcontext *gc,  GLint size, GLenum type, GLsizei stride, const GLvoid *ptr ){}
//GLvoid GL_APIENTRY __glim_EnableClientState(__GLcontext *gc,  GLenum cap ){}
//GLvoid GL_APIENTRY __glim_DisableClientState(__GLcontext *gc,  GLenum cap ){}
//GLvoid GL_APIENTRY __glim_EdgeFlagPointer(__GLcontext *gc,  GLsizei stride, const GLvoid *ptr ){}
//GLvoid GL_APIENTRY __glim_IndexPointer(__GLcontext *gc,  GLenum type, GLsizei stride, const GLvoid *ptr ){}
//void GL_APIENTRY __glim_InterleavedArrays(__GLcontext *gc,  GLenum format, GLsizei stride, const GLvoid *pointer ){}
//GLvoid GL_APIENTRY __glim_NormalPointer(__GLcontext *gc,  GLenum type, GLsizei stride,  const GLvoid *ptr ){}
//GLvoid GL_APIENTRY __glim_VertexPointer(__GLcontext *gc,  GLint size, GLenum type, GLsizei stride, const GLvoid *ptr ){}
//GLvoid GL_APIENTRY __glim_TexCoordPointer(__GLcontext *gc,  GLint size, GLenum type, GLsizei stride, const GLvoid *ptr ){}
//GLboolean GL_APIENTRY __glim_AreTexturesResident(__GLcontext *gc,  GLsizei n, const GLuint *textures, GLboolean *residences ){}
//GLvoid GL_APIENTRY __glim_CopyTexImage1D(__GLcontext *gc,  GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border ){}
//GLvoid GL_APIENTRY __glim_CopyTexSubImage1D(__GLcontext *gc,  GLenum target, GLint level,  GLint xoffset, GLint x, GLint y,  GLsizei width ){}
//GLvoid GL_APIENTRY __glim_PrioritizeTextures(__GLcontext *gc,  GLsizei n,  const GLuint *textures, const GLclampf *priorities ){}
//GLvoid GL_APIENTRY __glim_TexSubImage1D(__GLcontext *gc,  GLenum target, GLint level,  GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels ){}
//GLvoid GL_APIENTRY __glim_PushClientAttrib(__GLcontext *gc,  GLbitfield mask ){}
//GLvoid GL_APIENTRY __glim_PopClientAttrib(__GLcontext *gc){}
GLvoid GL_APIENTRY __glim_ColorTable(__GLcontext *gc,  GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table ){}
GLvoid GL_APIENTRY __glim_ColorTableParameteriv(__GLcontext *gc, GLenum target, GLenum pname, const GLint *params){}
GLvoid GL_APIENTRY __glim_ColorTableParameterfv(__GLcontext *gc, GLenum target, GLenum pname, const GLfloat *params){}
GLvoid GL_APIENTRY __glim_CopyColorTable(__GLcontext *gc,  GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width ){}
GLvoid GL_APIENTRY __glim_GetColorTable(__GLcontext *gc,  GLenum target, GLenum format,  GLenum type, GLvoid *table ){}
GLvoid GL_APIENTRY __glim_GetColorTableParameterfv(__GLcontext *gc,  GLenum target, GLenum pname, GLfloat *params ){}
GLvoid GL_APIENTRY __glim_GetColorTableParameteriv(__GLcontext *gc,  GLenum target, GLenum pname,  GLint *params ){}
GLvoid GL_APIENTRY __glim_ColorSubTable(__GLcontext *gc,  GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data ){}
GLvoid GL_APIENTRY __glim_CopyColorSubTable(__GLcontext *gc,  GLenum target, GLsizei start, GLint x, GLint y, GLsizei width ){}
GLvoid GL_APIENTRY __glim_ConvolutionFilter1D(__GLcontext *gc,  GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image ){}
GLvoid GL_APIENTRY __glim_ConvolutionFilter2D(__GLcontext *gc,  GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image ){}
GLvoid GL_APIENTRY __glim_ConvolutionParameterf(__GLcontext *gc,  GLenum target, GLenum pname, GLfloat params ){}
GLvoid GL_APIENTRY __glim_ConvolutionParameterfv(__GLcontext *gc,  GLenum target, GLenum pname, const GLfloat *params ){}
GLvoid GL_APIENTRY __glim_ConvolutionParameteri(__GLcontext *gc,  GLenum target, GLenum pname, GLint params ){}
GLvoid GL_APIENTRY __glim_ConvolutionParameteriv(__GLcontext *gc,  GLenum target, GLenum pname, const GLint *params ){}
GLvoid GL_APIENTRY __glim_CopyConvolutionFilter1D(__GLcontext *gc,  GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width ){}
GLvoid GL_APIENTRY __glim_CopyConvolutionFilter2D(__GLcontext *gc,  GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height){}
GLvoid GL_APIENTRY __glim_GetConvolutionFilter(__GLcontext *gc,  GLenum target, GLenum format, GLenum type, GLvoid *image ){}
GLvoid GL_APIENTRY __glim_GetConvolutionParameterfv(__GLcontext *gc,  GLenum target, GLenum pname, GLfloat *params ){}
GLvoid GL_APIENTRY __glim_GetConvolutionParameteriv(__GLcontext *gc,  GLenum target, GLenum pname, GLint *params ){}
GLvoid GL_APIENTRY __glim_SeparableFilter2D(__GLcontext *gc,  GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column ){}
GLvoid GL_APIENTRY __glim_GetSeparableFilter(__GLcontext *gc,  GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span ){}
GLvoid GL_APIENTRY __glim_Histogram(__GLcontext *gc,  GLenum target, GLsizei width, GLenum internalformat, GLboolean sink ){}
GLvoid GL_APIENTRY __glim_ResetHistogram(__GLcontext *gc,  GLenum target ){}
GLvoid GL_APIENTRY __glim_GetHistogram(__GLcontext *gc,  GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values ){}
GLvoid GL_APIENTRY __glim_GetHistogramParameterfv(__GLcontext *gc,  GLenum target, GLenum pname, GLfloat *params ){}
GLvoid GL_APIENTRY __glim_GetHistogramParameteriv(__GLcontext *gc,  GLenum target, GLenum pname, GLint *params ){}
GLvoid GL_APIENTRY __glim_Minmax(__GLcontext *gc,  GLenum target, GLenum internalformat, GLboolean sink ){}
GLvoid GL_APIENTRY __glim_ResetMinmax(__GLcontext *gc,  GLenum target ){}
GLvoid GL_APIENTRY __glim_GetMinmax(__GLcontext *gc,  GLenum target, GLboolean reset, GLenum format, GLenum types, GLvoid *values ){}
GLvoid GL_APIENTRY __glim_GetMinmaxParameterfv(__GLcontext *gc,  GLenum target, GLenum pname,  GLfloat *params ){}
GLvoid GL_APIENTRY __glim_GetMinmaxParameteriv(__GLcontext *gc,  GLenum target, GLenum pname, GLint *params ){}
//GLvoid GL_APIENTRY __glim_ClientActiveTexture(__GLcontext *gc,  GLenum texture ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord1d(__GLcontext *gc,  GLenum target, GLdouble s ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord1dv(__GLcontext *gc,  GLenum target, const GLdouble *v ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord1f(__GLcontext *gc,  GLenum target, GLfloat s ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord1fv(__GLcontext *gc,  GLenum target, const GLfloat *v ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord1i(__GLcontext *gc,  GLenum target, GLint s ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord1iv(__GLcontext *gc,  GLenum target, const GLint *v ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord1s(__GLcontext *gc,  GLenum target, GLshort s ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord1sv(__GLcontext *gc,  GLenum target, const GLshort *v ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord2d(__GLcontext *gc,  GLenum target, GLdouble s, GLdouble t ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord2dv(__GLcontext *gc,  GLenum target, const GLdouble *v ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord2f(__GLcontext *gc,  GLenum target, GLfloat s, GLfloat t ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord2fv(__GLcontext *gc,  GLenum target, const GLfloat *v ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord2i(__GLcontext *gc,  GLenum target, GLint s, GLint t ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord2iv(__GLcontext *gc,  GLenum target, const GLint *v ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord2s(__GLcontext *gc,  GLenum target, GLshort s, GLshort t ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord2sv(__GLcontext *gc,  GLenum target, const GLshort *v ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord3d(__GLcontext *gc,  GLenum target, GLdouble s, GLdouble t, GLdouble r ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord3dv(__GLcontext *gc,  GLenum target, const GLdouble *v ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord3f(__GLcontext *gc,  GLenum target, GLfloat s, GLfloat t, GLfloat r ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord3fv(__GLcontext *gc,  GLenum target, const GLfloat *v ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord3i(__GLcontext *gc,  GLenum target, GLint s, GLint t, GLint r ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord3iv(__GLcontext *gc,  GLenum target, const GLint *v ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord3s(__GLcontext *gc,  GLenum target, GLshort s, GLshort t, GLshort r ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord3sv(__GLcontext *gc,  GLenum target, const GLshort *v ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord4d(__GLcontext *gc,  GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord4dv(__GLcontext *gc,  GLenum target, const GLdouble *v ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord4f(__GLcontext *gc,  GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord4fv(__GLcontext *gc,  GLenum target, const GLfloat *v ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord4i(__GLcontext *gc,  GLenum target, GLint s, GLint t, GLint r, GLint q ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord4iv(__GLcontext *gc,  GLenum target, const GLint *v ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord4s(__GLcontext *gc,  GLenum target, GLshort s, GLshort t, GLshort r, GLshort q ){}
//GLvoid GL_APIENTRY __glim_MultiTexCoord4sv(__GLcontext *gc,  GLenum target, const GLshort *v ){}
//GLvoid GL_APIENTRY __glim_LoadTransposeMatrixd(__GLcontext *gc,  const GLdouble m[16] ){}
//GLvoid GL_APIENTRY __glim_LoadTransposeMatrixf(__GLcontext *gc,  const GLfloat m[16] ){}
//GLvoid GL_APIENTRY __glim_MultTransposeMatrixd(__GLcontext *gc,  const GLdouble m[16] ){}
//GLvoid GL_APIENTRY __glim_MultTransposeMatrixf(__GLcontext *gc,  const GLfloat m[16] ){}
//GLvoid GL_APIENTRY __glim_CompressedTexImage1D(__GLcontext *gc,  GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data ){}
//GLvoid GL_APIENTRY __glim_CompressedTexSubImage1D(__GLcontext *gc,  GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data ){}
//GLvoid GL_APIENTRY __glim_GetCompressedTexImage(__GLcontext *gc,  GLenum target, GLint lod, GLvoid *img ){}
//GLvoid GL_APIENTRY __glim_FogCoordf(__GLcontext *gc, GLfloat coord){}
//GLvoid GL_APIENTRY __glim_FogCoordfv(__GLcontext *gc, const GLfloat *coord){}
//GLvoid GL_APIENTRY __glim_FogCoordd(__GLcontext *gc, GLdouble coord){}
//GLvoid GL_APIENTRY __glim_FogCoorddv(__GLcontext *gc, const GLdouble *coord){}
//GLvoid GL_APIENTRY __glim_FogCoordPointer(__GLcontext *gc, GLenum type, GLsizei stride, const void *pointer){}
//GLvoid GL_APIENTRY __glim_PointParameterf(__GLcontext *gc, GLenum pname, GLfloat param){}
//GLvoid GL_APIENTRY __glim_PointParameterfv(__GLcontext *gc, GLenum pname, const GLfloat *params){}
//GLvoid GL_APIENTRY __glim_PointParameteri(__GLcontext *gc, GLenum pname, GLint param){}
//GLvoid GL_APIENTRY __glim_PointParameteriv(__GLcontext *gc, GLenum pname, const GLint *params){}
//GLvoid GL_APIENTRY __glim_SecondaryColor3b(__GLcontext *gc, GLbyte red, GLbyte green, GLbyte blue){}
//GLvoid GL_APIENTRY __glim_SecondaryColor3bv(__GLcontext *gc, const GLbyte *v){}
//GLvoid GL_APIENTRY __glim_SecondaryColor3d(__GLcontext *gc, GLdouble red, GLdouble green, GLdouble blue){}
//GLvoid GL_APIENTRY __glim_SecondaryColor3dv(__GLcontext *gc, const GLdouble *v){}
//GLvoid GL_APIENTRY __glim_SecondaryColor3f(__GLcontext *gc, GLfloat red, GLfloat green, GLfloat blue){}
//GLvoid GL_APIENTRY __glim_SecondaryColor3fv(__GLcontext *gc, const GLfloat *v){}
//GLvoid GL_APIENTRY __glim_SecondaryColor3i(__GLcontext *gc, GLint red, GLint green, GLint blue){}
//GLvoid GL_APIENTRY __glim_SecondaryColor3iv(__GLcontext *gc, const GLint *v){}
//GLvoid GL_APIENTRY __glim_SecondaryColor3s(__GLcontext *gc, GLshort red, GLshort green, GLshort blue){}
//GLvoid GL_APIENTRY __glim_SecondaryColor3sv(__GLcontext *gc, const GLshort *v){}
//GLvoid GL_APIENTRY __glim_SecondaryColor3ub(__GLcontext *gc, GLubyte red, GLubyte green, GLubyte blue){}
//GLvoid GL_APIENTRY __glim_SecondaryColor3ubv(__GLcontext *gc, const GLubyte *v){}
//GLvoid GL_APIENTRY __glim_SecondaryColor3ui(__GLcontext *gc, GLuint red, GLuint green, GLuint blue){}
//GLvoid GL_APIENTRY __glim_SecondaryColor3uiv(__GLcontext *gc, const GLuint *v){}
//GLvoid GL_APIENTRY __glim_SecondaryColor3us(__GLcontext *gc, GLushort red, GLushort green, GLushort blue){}
//GLvoid GL_APIENTRY __glim_SecondaryColor3usv(__GLcontext *gc, const GLushort *v){}
//GLvoid GL_APIENTRY __glim_SecondaryColorPointer(__GLcontext *gc, GLint size, GLenum type, GLsizei stride, const void *pointer){}
//GLvoid GL_APIENTRY __glim_WindowPos2d(__GLcontext *gc, GLdouble x, GLdouble y){}
//GLvoid GL_APIENTRY __glim_WindowPos2dv(__GLcontext *gc, const GLdouble *v){}
//GLvoid GL_APIENTRY __glim_WindowPos2f(__GLcontext *gc, GLfloat x, GLfloat y){}
//GLvoid GL_APIENTRY __glim_WindowPos2fv(__GLcontext *gc, const GLfloat *v){}
//GLvoid GL_APIENTRY __glim_WindowPos2i(__GLcontext *gc, GLint x, GLint y){}
//GLvoid GL_APIENTRY __glim_WindowPos2iv(__GLcontext *gc, const GLint *v){}
//GLvoid GL_APIENTRY __glim_WindowPos2s(__GLcontext *gc, GLshort x, GLshort y){}
//GLvoid GL_APIENTRY __glim_WindowPos2sv(__GLcontext *gc, const GLshort *v){}
//GLvoid GL_APIENTRY __glim_WindowPos3d(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z){}
//GLvoid GL_APIENTRY __glim_WindowPos3dv(__GLcontext *gc, const GLdouble *v){}
//GLvoid GL_APIENTRY __glim_WindowPos3f(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z){}
//GLvoid GL_APIENTRY __glim_WindowPos3fv(__GLcontext *gc, const GLfloat *v){}
//GLvoid GL_APIENTRY __glim_WindowPos3i(__GLcontext *gc, GLint x, GLint y, GLint z){}
//GLvoid GL_APIENTRY __glim_WindowPos3iv(__GLcontext *gc, const GLint *v){}
//GLvoid GL_APIENTRY __glim_WindowPos3s(__GLcontext *gc, GLshort x, GLshort y, GLshort z){}
//GLvoid GL_APIENTRY __glim_WindowPos3sv(__GLcontext *gc, const GLshort *v){}
//GLvoid GL_APIENTRY __glim_DrawPixels(__GLcontext *gc,  GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels ){}
//GLvoid GL_APIENTRY __glim_CopyPixels(__GLcontext *gc,  GLint x, GLint y,  GLsizei width, GLsizei height, GLenum type ){}
//GLvoid GL_APIENTRY __glim_Indexub(__GLcontext *gc,  GLubyte c ){}
//GLvoid GL_APIENTRY __glim_Indexubv(__GLcontext *gc,  const GLubyte *c ){}
//GLboolean GLAPIENTRY __glim_IsRenderbufferEXT( __GLcontext *gc,  GLuint renderbuffer){return GL_FALSE;}
//GLvoid GLAPIENTRY __glim_BindRenderbufferEXT(__GLcontext *gc,  GLenum target, GLuint renderbuffer){}
//GLvoid GLAPIENTRY __glim_DeleteRenderbuffersEXT(__GLcontext *gc,  GLsizei n, const GLuint *renderbuffers){}
//GLvoid GLAPIENTRY __glim_GenRenderbuffersEXT(__GLcontext *gc,  GLsizei n, GLuint *renderbuffers){}
//GLvoid GLAPIENTRY __glim_RenderbufferStorageEXT(__GLcontext *gc,  GLenum target, GLenum internalformat, GLsizei width, GLsizei height){}
//GLvoid GLAPIENTRY __glim_GetRenderbufferParameterivEXT(__GLcontext *gc,  GLenum target, GLenum pname, GLint* params){}
//GLboolean GLAPIENTRY __glim_IsFramebufferEXT(__GLcontext *gc,  GLuint framebuffer){return GL_FALSE;}
//GLvoid GLAPIENTRY __glim_BindFramebufferEXT(__GLcontext *gc,  GLenum target, GLuint framebuffer){}
//GLvoid GLAPIENTRY __glim_DeleteFramebuffersEXT(__GLcontext *gc,  GLsizei n, const GLuint *framebuffers){}
//GLvoid GLAPIENTRY __glim_GenFramebuffersEXT(__GLcontext *gc,  GLsizei n, GLuint *framebuffers){}
//GLenum GLAPIENTRY __glim_CheckFramebufferStatusEXT(__GLcontext *gc,  GLenum target){return 0;}
//GLvoid GLAPIENTRY __glim_FramebufferTexture1DEXT(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level){}
//GLvoid GLAPIENTRY __glim_FramebufferTexture2DEXT(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level){}
//GLvoid GLAPIENTRY __glim_FramebufferTexture3DEXT(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset){}
//GLvoid GLAPIENTRY __glim_FramebufferRenderbufferEXT(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer){}
//GLvoid GLAPIENTRY __glim_GetFramebufferAttachmentParameterivEXT(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum pname, GLint *params){}
//GLvoid GLAPIENTRY __glim_GenerateMipmapEXT(__GLcontext *gc,  GLenum target){}
//GLvoid GLAPIENTRY __glim_BlitFramebufferEXT(__GLcontext *gc,  GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,GLbitfield mask, GLenum filter){}
//GLvoid GLAPIENTRY __glim_RenderbufferStorageMultisampleEXT(__GLcontext *gc,  GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height){}

/* OpenGL ES 2.0 */

GLvoid GL_APIENTRY __glim_ActiveTexture(__GLcontext *gc, GLenum texture){}
GLvoid GL_APIENTRY __glim_AttachShader(__GLcontext *gc, GLuint program, GLuint shader){}
GLvoid GL_APIENTRY __glim_BindAttribLocation(__GLcontext *gc, GLuint program, GLuint index, const GLchar* name){}
GLvoid GL_APIENTRY __glim_BindBuffer(__GLcontext *gc, GLenum target, GLuint buffer){}
GLvoid GL_APIENTRY __glim_BindFramebuffer(__GLcontext *gc, GLenum target, GLuint framebuffer){}
GLvoid GL_APIENTRY __glim_BindRenderbuffer(__GLcontext *gc, GLenum target, GLuint renderbuffer){}
GLvoid GL_APIENTRY __glim_BindTexture(__GLcontext *gc, GLenum target, GLuint texture){}
GLvoid GL_APIENTRY __glim_BlendColor(__GLcontext *gc, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha){}
GLvoid GL_APIENTRY __glim_BlendEquation(__GLcontext *gc, GLenum mode){}
GLvoid GL_APIENTRY __glim_BlendEquationSeparate(__GLcontext *gc, GLenum modeRGB, GLenum modeAlpha){}
GLvoid GL_APIENTRY __glim_BlendFunc(__GLcontext *gc, GLenum sfactor, GLenum dfactor){}
GLvoid GL_APIENTRY __glim_BlendFuncSeparate(__GLcontext *gc, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha){}
GLvoid GL_APIENTRY __glim_BufferData(__GLcontext *gc, GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage){}
GLvoid GL_APIENTRY __glim_BufferSubData(__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data){}
GLenum GL_APIENTRY __glim_CheckFramebufferStatus(__GLcontext *gc, GLenum target){return GL_FRAMEBUFFER_COMPLETE;}
GLvoid GL_APIENTRY __glim_Clear(__GLcontext *gc, GLbitfield mask){}
GLvoid GL_APIENTRY __glim_ClearColor(__GLcontext *gc, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha){}
GLvoid GL_APIENTRY __glim_ClearDepthf(__GLcontext *gc, GLfloat depth){}
GLvoid GL_APIENTRY __glim_ClearStencil(__GLcontext *gc, GLint s){}
GLvoid GL_APIENTRY __glim_ColorMask(__GLcontext *gc, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha){}
GLvoid GL_APIENTRY __glim_CompileShader(__GLcontext *gc, GLuint shader){}
GLvoid GL_APIENTRY __glim_CompressedTexImage2D(__GLcontext *gc, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data){}
GLvoid GL_APIENTRY __glim_CompressedTexSubImage2D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data){}
GLvoid GL_APIENTRY __glim_CopyTexImage2D(__GLcontext *gc, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border){}
GLvoid GL_APIENTRY __glim_CopyTexSubImage2D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height){}
GLuint GL_APIENTRY __glim_CreateProgram(__GLcontext *gc){return 0;}
GLuint GL_APIENTRY __glim_CreateShader(__GLcontext *gc, GLenum type){return 0;}
GLvoid GL_APIENTRY __glim_CullFace(__GLcontext *gc, GLenum mode){}
GLvoid GL_APIENTRY __glim_DeleteBuffers(__GLcontext *gc, GLsizei n, const GLuint* buffers){}
GLvoid GL_APIENTRY __glim_DeleteFramebuffers(__GLcontext *gc, GLsizei n, const GLuint* framebuffers){}
GLvoid GL_APIENTRY __glim_DeleteProgram(__GLcontext *gc, GLuint program){}
GLvoid GL_APIENTRY __glim_DeleteRenderbuffers(__GLcontext *gc, GLsizei n, const GLuint* renderbuffers){}
GLvoid GL_APIENTRY __glim_DeleteShader(__GLcontext *gc, GLuint shader){}
GLvoid GL_APIENTRY __glim_DeleteTextures(__GLcontext *gc, GLsizei n, const GLuint* textures){}
GLvoid GL_APIENTRY __glim_DepthFunc(__GLcontext *gc, GLenum func){}
GLvoid GL_APIENTRY __glim_DepthMask(__GLcontext *gc, GLboolean flag){}
GLvoid GL_APIENTRY __glim_DepthRangef(__GLcontext *gc, GLfloat n, GLfloat f){}
GLvoid GL_APIENTRY __glim_DetachShader(__GLcontext *gc, GLuint program, GLuint shader){}
GLvoid GL_APIENTRY __glim_Disable(__GLcontext *gc, GLenum cap){}
GLvoid GL_APIENTRY __glim_DisableVertexAttribArray(__GLcontext *gc, GLuint index){}
GLvoid GL_APIENTRY __glim_DrawArrays(__GLcontext *gc, GLenum mode, GLint first, GLsizei count){}
GLvoid GL_APIENTRY __glim_DrawElements(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid* indices){}
GLvoid GL_APIENTRY __glim_Enable(__GLcontext *gc, GLenum cap){}
GLvoid GL_APIENTRY __glim_EnableVertexAttribArray(__GLcontext *gc, GLuint index){}
GLvoid GL_APIENTRY __glim_Finish(__GLcontext *gc){}
GLvoid GL_APIENTRY __glim_Flush(__GLcontext *gc){}
GLvoid GL_APIENTRY __glim_FramebufferRenderbuffer(__GLcontext *gc, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer){}
GLvoid GL_APIENTRY __glim_FramebufferTexture2D(__GLcontext *gc, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level){}
GLvoid GL_APIENTRY __glim_FrontFace(__GLcontext *gc, GLenum mode){}
GLvoid GL_APIENTRY __glim_GenBuffers(__GLcontext *gc, GLsizei n, GLuint* buffers){}
GLvoid GL_APIENTRY __glim_GenerateMipmap(__GLcontext *gc, GLenum target){}
GLvoid GL_APIENTRY __glim_GenFramebuffers(__GLcontext *gc, GLsizei n, GLuint* framebuffers){}
GLvoid GL_APIENTRY __glim_GenRenderbuffers(__GLcontext *gc, GLsizei n, GLuint* renderbuffers){}
GLvoid GL_APIENTRY __glim_GenTextures(__GLcontext *gc, GLsizei n, GLuint* textures){}
GLvoid GL_APIENTRY __glim_GetActiveAttrib(__GLcontext *gc, GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name){}
GLvoid GL_APIENTRY __glim_GetActiveUniform(__GLcontext *gc, GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name){}
GLvoid GL_APIENTRY __glim_GetAttachedShaders(__GLcontext *gc, GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders){}
GLint  GL_APIENTRY __glim_GetAttribLocation(__GLcontext *gc, GLuint program, const GLchar* name){return -1;}
GLvoid GL_APIENTRY __glim_GetBooleanv(__GLcontext *gc, GLenum pname, GLboolean* params){}
GLvoid GL_APIENTRY __glim_GetBufferParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint* params){}
GLenum GL_APIENTRY __glim_GetError(__GLcontext *gc){return GL_NO_ERROR;}
GLvoid GL_APIENTRY __glim_GetFloatv(__GLcontext *gc, GLenum pname, GLfloat* params){}
GLvoid GL_APIENTRY __glim_GetFramebufferAttachmentParameteriv(__GLcontext *gc, GLenum target, GLenum attachment, GLenum pname, GLint* params){}
GLvoid GL_APIENTRY __glim_GetIntegerv(__GLcontext *gc, GLenum pname, GLint* params){}
GLvoid GL_APIENTRY __glim_GetProgramiv(__GLcontext *gc, GLuint program, GLenum pname, GLint* params){}
GLvoid GL_APIENTRY __glim_GetProgramInfoLog(__GLcontext *gc, GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog){}
GLvoid GL_APIENTRY __glim_GetRenderbufferParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint* params){}
GLvoid GL_APIENTRY __glim_GetShaderiv(__GLcontext *gc, GLuint shader, GLenum pname, GLint* params){}
GLvoid GL_APIENTRY __glim_GetShaderInfoLog(__GLcontext *gc, GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog){}
GLvoid GL_APIENTRY __glim_GetShaderPrecisionFormat(__GLcontext *gc, GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision){}
GLvoid GL_APIENTRY __glim_GetShaderSource(__GLcontext *gc, GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source){}
const GLubyte* GL_APIENTRY __glim_GetString(__GLcontext *gc, GLenum name){return gcvNULL;}
//GLvoid GL_APIENTRY __glim_GetTexParameterfv(__GLcontext *gc, GLenum target, GLenum pname, GLfloat* params){}
//GLvoid GL_APIENTRY __glim_GetTexParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint* params){}
GLvoid GL_APIENTRY __glim_GetUniformfv(__GLcontext *gc, GLuint program, GLint location, GLfloat* params){}
GLvoid GL_APIENTRY __glim_GetUniformiv(__GLcontext *gc, GLuint program, GLint location, GLint* params){}
GLint  GL_APIENTRY __glim_GetUniformLocation(__GLcontext *gc, GLuint program, const GLchar* name){return -1;}
GLvoid GL_APIENTRY __glim_GetVertexAttribfv(__GLcontext *gc, GLuint index, GLenum pname, GLfloat* params){}
GLvoid GL_APIENTRY __glim_GetVertexAttribiv(__GLcontext *gc, GLuint index, GLenum pname, GLint* params){}
GLvoid GL_APIENTRY __glim_GetVertexAttribPointerv(__GLcontext *gc, GLuint index, GLenum pname, GLvoid** pointer){}
GLvoid GL_APIENTRY __glim_Hint(__GLcontext *gc, GLenum target, GLenum mode){}
GLboolean GL_APIENTRY __glim_IsBuffer(__GLcontext *gc, GLuint buffer){return GL_FALSE;}
GLboolean GL_APIENTRY __glim_IsEnabled(__GLcontext *gc, GLenum cap){return GL_FALSE;}
GLboolean GL_APIENTRY __glim_IsFramebuffer(__GLcontext *gc, GLuint framebuffer){return GL_FALSE;}
GLboolean GL_APIENTRY __glim_IsProgram(__GLcontext *gc, GLuint program){return GL_FALSE;}
GLboolean GL_APIENTRY __glim_IsRenderbuffer(__GLcontext *gc, GLuint renderbuffer){return GL_FALSE;}
GLboolean GL_APIENTRY __glim_IsShader(__GLcontext *gc, GLuint shader){return GL_FALSE;}
GLboolean GL_APIENTRY __glim_IsTexture(__GLcontext *gc, GLuint texture){return GL_FALSE;}
GLvoid GL_APIENTRY __glim_LineWidth(__GLcontext *gc, GLfloat width){}
GLvoid GL_APIENTRY __glim_LinkProgram(__GLcontext *gc, GLuint program){}
GLvoid GL_APIENTRY __glim_PixelStorei(__GLcontext *gc, GLenum pname, GLint param){}
GLvoid GL_APIENTRY __glim_PolygonOffset(__GLcontext *gc, GLfloat factor, GLfloat units){}
GLvoid GL_APIENTRY __glim_ReadPixels(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels){}
GLvoid GL_APIENTRY __glim_ReleaseShaderCompiler(__GLcontext *gc){}
GLvoid GL_APIENTRY __glim_RenderbufferStorage(__GLcontext *gc, GLenum target, GLenum internalformat, GLsizei width, GLsizei height){}
GLvoid GL_APIENTRY __glim_SampleCoverage(__GLcontext *gc, GLfloat value, GLboolean invert){}
GLvoid GL_APIENTRY __glim_Scissor(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height){}
GLvoid GL_APIENTRY __glim_ShaderBinary(__GLcontext *gc, GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length){}
GLvoid GL_APIENTRY __glim_ShaderSource(__GLcontext *gc, GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length){}
GLvoid GL_APIENTRY __glim_StencilFunc(__GLcontext *gc, GLenum func, GLint ref, GLuint mask){}
GLvoid GL_APIENTRY __glim_StencilFuncSeparate(__GLcontext *gc, GLenum face, GLenum func, GLint ref, GLuint mask){}
GLvoid GL_APIENTRY __glim_StencilMask(__GLcontext *gc, GLuint mask){}
GLvoid GL_APIENTRY __glim_StencilMaskSeparate(__GLcontext *gc, GLenum face, GLuint mask){}
GLvoid GL_APIENTRY __glim_StencilOp(__GLcontext *gc, GLenum fail, GLenum zfail, GLenum zpass){}
GLvoid GL_APIENTRY __glim_StencilOpSeparate(__GLcontext *gc, GLenum face, GLenum fail, GLenum zfail, GLenum zpass){}
GLvoid GL_APIENTRY __glim_TexImage2D(__GLcontext *gc, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels){}
GLvoid GL_APIENTRY __glim_TexParameterf(__GLcontext *gc, GLenum target, GLenum pname, GLfloat param){}
GLvoid GL_APIENTRY __glim_TexParameterfv(__GLcontext *gc, GLenum target, GLenum pname, const GLfloat* params){}
GLvoid GL_APIENTRY __glim_TexParameteri(__GLcontext *gc, GLenum target, GLenum pname, GLint param){}
GLvoid GL_APIENTRY __glim_TexParameteriv(__GLcontext *gc, GLenum target, GLenum pname, const GLint* params){}
GLvoid GL_APIENTRY __glim_TexSubImage2D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels){}
GLvoid GL_APIENTRY __glim_Uniform1f(__GLcontext *gc, GLint location, GLfloat x){}
GLvoid GL_APIENTRY __glim_Uniform1fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat* v){}
GLvoid GL_APIENTRY __glim_Uniform1i(__GLcontext *gc, GLint location, GLint x){}
GLvoid GL_APIENTRY __glim_Uniform1iv(__GLcontext *gc, GLint location, GLsizei count, const GLint* v){}
GLvoid GL_APIENTRY __glim_Uniform2f(__GLcontext *gc, GLint location, GLfloat x, GLfloat y){}
GLvoid GL_APIENTRY __glim_Uniform2fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat* v){}
GLvoid GL_APIENTRY __glim_Uniform2i(__GLcontext *gc, GLint location, GLint x, GLint y){}
GLvoid GL_APIENTRY __glim_Uniform2iv(__GLcontext *gc, GLint location, GLsizei count, const GLint* v){}
GLvoid GL_APIENTRY __glim_Uniform3f(__GLcontext *gc, GLint location, GLfloat x, GLfloat y, GLfloat z){}
GLvoid GL_APIENTRY __glim_Uniform3fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat* v){}
GLvoid GL_APIENTRY __glim_Uniform3i(__GLcontext *gc, GLint location, GLint x, GLint y, GLint z){}
GLvoid GL_APIENTRY __glim_Uniform3iv(__GLcontext *gc, GLint location, GLsizei count, const GLint* v){}
GLvoid GL_APIENTRY __glim_Uniform4f(__GLcontext *gc, GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w){}
GLvoid GL_APIENTRY __glim_Uniform4fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat* v){}
GLvoid GL_APIENTRY __glim_Uniform4i(__GLcontext *gc, GLint location, GLint x, GLint y, GLint z, GLint w){}
GLvoid GL_APIENTRY __glim_Uniform4iv(__GLcontext *gc, GLint location, GLsizei count, const GLint* v){}
GLvoid GL_APIENTRY __glim_UniformMatrix2fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value){}
GLvoid GL_APIENTRY __glim_UniformMatrix3fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value){}
GLvoid GL_APIENTRY __glim_UniformMatrix4fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value){}
GLvoid GL_APIENTRY __glim_UseProgram(__GLcontext *gc, GLuint program){}
GLvoid GL_APIENTRY __glim_ValidateProgram(__GLcontext *gc, GLuint program){}
GLvoid GL_APIENTRY __glim_VertexAttrib1f(__GLcontext *gc, GLuint indx, GLfloat x){}
GLvoid GL_APIENTRY __glim_VertexAttrib1fv(__GLcontext *gc, GLuint indx, const GLfloat* values){}
GLvoid GL_APIENTRY __glim_VertexAttrib2f(__GLcontext *gc, GLuint indx, GLfloat x, GLfloat y){}
GLvoid GL_APIENTRY __glim_VertexAttrib2fv(__GLcontext *gc, GLuint indx, const GLfloat* values){}
GLvoid GL_APIENTRY __glim_VertexAttrib3f(__GLcontext *gc, GLuint indx, GLfloat x, GLfloat y, GLfloat z){}
GLvoid GL_APIENTRY __glim_VertexAttrib3fv(__GLcontext *gc, GLuint indx, const GLfloat* values){}
GLvoid GL_APIENTRY __glim_VertexAttrib4f(__GLcontext *gc, GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w){}
GLvoid GL_APIENTRY __glim_VertexAttrib4fv(__GLcontext *gc, GLuint indx, const GLfloat* values){}
GLvoid GL_APIENTRY __glim_VertexAttribPointer(__GLcontext *gc, GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr){}
GLvoid GL_APIENTRY __glim_Viewport(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height){}

/* OpenGL ES 3.0 */

GLvoid GL_APIENTRY __glim_ReadBuffer(__GLcontext *gc, GLenum mode){}
GLvoid GL_APIENTRY __glim_DrawRangeElements(__GLcontext *gc, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices){}
GLvoid GL_APIENTRY __glim_TexImage3D(__GLcontext *gc, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels){}
GLvoid GL_APIENTRY __glim_TexSubImage3D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels){}
GLvoid GL_APIENTRY __glim_CopyTexSubImage3D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height){}
GLvoid GL_APIENTRY __glim_CompressedTexImage3D(__GLcontext *gc, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data){}
GLvoid GL_APIENTRY __glim_CompressedTexSubImage3D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data){}
GLvoid GL_APIENTRY __glim_GenQueries(__GLcontext *gc, GLsizei n, GLuint* ids){}
GLvoid GL_APIENTRY __glim_DeleteQueries(__GLcontext *gc, GLsizei n, const GLuint* ids){}
GLboolean GL_APIENTRY __glim_IsQuery(__GLcontext *gc, GLuint id){return GL_FALSE;}
GLvoid GL_APIENTRY __glim_BeginQuery(__GLcontext *gc, GLenum target, GLuint id){}
GLvoid GL_APIENTRY __glim_EndQuery(__GLcontext *gc, GLenum target){}
GLvoid GL_APIENTRY __glim_GetQueryiv(__GLcontext *gc, GLenum target, GLenum pname, GLint* params){}
GLvoid GL_APIENTRY __glim_GetQueryObjectuiv(__GLcontext *gc, GLuint id, GLenum pname, GLuint* params){}
GLboolean GL_APIENTRY __glim_UnmapBuffer(__GLcontext *gc, GLenum target){return GL_FALSE;}
GLvoid GL_APIENTRY __glim_GetBufferPointerv(__GLcontext *gc, GLenum target, GLenum pname, GLvoid** params){}
GLvoid GL_APIENTRY __glim_DrawBuffers(__GLcontext *gc, GLsizei n, const GLenum* bufs){}
GLvoid GL_APIENTRY __glim_UniformMatrix2x3fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value){}
GLvoid GL_APIENTRY __glim_UniformMatrix3x2fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value){}
GLvoid GL_APIENTRY __glim_UniformMatrix2x4fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value){}
GLvoid GL_APIENTRY __glim_UniformMatrix4x2fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value){}
GLvoid GL_APIENTRY __glim_UniformMatrix3x4fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value){}
GLvoid GL_APIENTRY __glim_UniformMatrix4x3fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value){}
GLvoid GL_APIENTRY __glim_BlitFramebuffer(__GLcontext *gc, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter){}
GLvoid GL_APIENTRY __glim_RenderbufferStorageMultisample(__GLcontext *gc, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height){}
GLvoid GL_APIENTRY __glim_FramebufferTextureLayer(__GLcontext *gc, GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer){}
GLvoid* GL_APIENTRY __glim_MapBufferRange(__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access){return gcvNULL;}
GLvoid GL_APIENTRY __glim_FlushMappedBufferRange(__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr length){}
GLvoid GL_APIENTRY __glim_BindVertexArray(__GLcontext *gc, GLuint array){}
GLvoid GL_APIENTRY __glim_DeleteVertexArrays(__GLcontext *gc, GLsizei n, const GLuint* arrays){}
GLvoid GL_APIENTRY __glim_GenVertexArrays(__GLcontext *gc, GLsizei n, GLuint* arrays){}
GLboolean GL_APIENTRY __glim_IsVertexArray(__GLcontext *gc, GLuint array){return GL_FALSE;}
GLvoid GL_APIENTRY __glim_GetIntegeri_v(__GLcontext *gc, GLenum target, GLuint index, GLint* data){}
GLvoid GL_APIENTRY __glim_BeginTransformFeedback(__GLcontext *gc, GLenum primitiveMode){}
GLvoid GL_APIENTRY __glim_EndTransformFeedback(__GLcontext *gc){}
GLvoid GL_APIENTRY __glim_BindBufferRange(__GLcontext *gc, GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size){}
GLvoid GL_APIENTRY __glim_BindBufferBase(__GLcontext *gc, GLenum target, GLuint index, GLuint buffer){}
GLvoid GL_APIENTRY __glim_TransformFeedbackVaryings(__GLcontext *gc, GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode){}
GLvoid GL_APIENTRY __glim_GetTransformFeedbackVarying(__GLcontext *gc, GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name){}
GLvoid GL_APIENTRY __glim_VertexAttribIPointer(__GLcontext *gc, GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer){}
GLvoid GL_APIENTRY __glim_GetVertexAttribIiv(__GLcontext *gc, GLuint index, GLenum pname, GLint* params){}
GLvoid GL_APIENTRY __glim_GetVertexAttribIuiv(__GLcontext *gc, GLuint index, GLenum pname, GLuint* params){}
GLvoid GL_APIENTRY __glim_VertexAttribI4i(__GLcontext *gc, GLuint index, GLint x, GLint y, GLint z, GLint w){}
GLvoid GL_APIENTRY __glim_VertexAttribI4ui(__GLcontext *gc, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w){}
GLvoid GL_APIENTRY __glim_VertexAttribI4iv(__GLcontext *gc, GLuint index, const GLint* v){}
GLvoid GL_APIENTRY __glim_VertexAttribI4uiv(__GLcontext *gc, GLuint index, const GLuint* v){}
GLvoid GL_APIENTRY __glim_GetUniformuiv(__GLcontext *gc, GLuint program, GLint location, GLuint* params){}
GLint  GL_APIENTRY __glim_GetFragDataLocation(__GLcontext *gc, GLuint program, const GLchar *name){return -1;}
GLvoid GL_APIENTRY __glim_Uniform1ui(__GLcontext *gc, GLint location, GLuint v0){}
GLvoid GL_APIENTRY __glim_Uniform2ui(__GLcontext *gc, GLint location, GLuint v0, GLuint v1){}
GLvoid GL_APIENTRY __glim_Uniform3ui(__GLcontext *gc, GLint location, GLuint v0, GLuint v1, GLuint v2){}
GLvoid GL_APIENTRY __glim_Uniform4ui(__GLcontext *gc, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3){}
GLvoid GL_APIENTRY __glim_Uniform1uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint* value){}
GLvoid GL_APIENTRY __glim_Uniform2uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint* value){}
GLvoid GL_APIENTRY __glim_Uniform3uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint* value){}
GLvoid GL_APIENTRY __glim_Uniform4uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint* value){}
GLvoid GL_APIENTRY __glim_ClearBufferiv(__GLcontext *gc, GLenum buffer, GLint drawbuffer, const GLint* value){}
GLvoid GL_APIENTRY __glim_ClearBufferuiv(__GLcontext *gc, GLenum buffer, GLint drawbuffer, const GLuint* value){}
GLvoid GL_APIENTRY __glim_ClearBufferfv(__GLcontext *gc, GLenum buffer, GLint drawbuffer, const GLfloat* value){}
GLvoid GL_APIENTRY __glim_ClearBufferfi(__GLcontext *gc, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil){}
const GLubyte* GL_APIENTRY __glim_GetStringi(__GLcontext *gc, GLenum name, GLuint index){return gcvNULL;}
GLvoid GL_APIENTRY __glim_CopyBufferSubData(__GLcontext *gc, GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size){}
GLvoid GL_APIENTRY __glim_GetUniformIndices(__GLcontext *gc, GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices){}
GLvoid GL_APIENTRY __glim_GetActiveUniformsiv(__GLcontext *gc, GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params){}
GLuint GL_APIENTRY __glim_GetUniformBlockIndex(__GLcontext *gc, GLuint program, const GLchar* uniformBlockName){return (GLuint)-1;}
GLvoid GL_APIENTRY __glim_GetActiveUniformBlockiv(__GLcontext *gc, GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params){}
GLvoid GL_APIENTRY __glim_GetActiveUniformBlockName(__GLcontext *gc, GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName){}
GLvoid GL_APIENTRY __glim_UniformBlockBinding(__GLcontext *gc, GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding){}
GLvoid GL_APIENTRY __glim_DrawArraysInstanced(__GLcontext *gc, GLenum mode, GLint first, GLsizei count, GLsizei instanceCount){}
GLvoid GL_APIENTRY __glim_DrawElementsInstanced(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount){}
GLsync GL_APIENTRY __glim_FenceSync(__GLcontext *gc, GLenum condition, GLbitfield flags){return gcvNULL;}
GLboolean GL_APIENTRY __glim_IsSync(__GLcontext *gc, GLsync sync){return GL_FALSE;}
GLvoid GL_APIENTRY __glim_DeleteSync(__GLcontext *gc, GLsync sync){}
GLenum GL_APIENTRY __glim_ClientWaitSync(__GLcontext *gc, GLsync sync, GLbitfield flags, GLuint64 timeout){return GL_ALREADY_SIGNALED;}
GLvoid GL_APIENTRY __glim_WaitSync(__GLcontext *gc, GLsync sync, GLbitfield flags, GLuint64 timeout){}
GLvoid GL_APIENTRY __glim_GetInteger64v(__GLcontext *gc, GLenum pname, GLint64* params){}
GLvoid GL_APIENTRY __glim_GetSynciv(__GLcontext *gc, GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values){}
GLvoid GL_APIENTRY __glim_GetInteger64i_v(__GLcontext *gc, GLenum target, GLuint index, GLint64* data){}
GLvoid GL_APIENTRY __glim_GetBufferParameteri64v(__GLcontext *gc, GLenum target, GLenum pname, GLint64* params){}
GLvoid GL_APIENTRY __glim_GenSamplers(__GLcontext *gc, GLsizei count, GLuint* samplers){}
GLvoid GL_APIENTRY __glim_DeleteSamplers(__GLcontext *gc, GLsizei count, const GLuint* samplers){}
GLboolean GL_APIENTRY __glim_IsSampler(__GLcontext *gc, GLuint sampler){return GL_FALSE;}
GLvoid GL_APIENTRY __glim_BindSampler(__GLcontext *gc, GLuint unit, GLuint sampler){}
GLvoid GL_APIENTRY __glim_SamplerParameteri(__GLcontext *gc, GLuint sampler, GLenum pname, GLint param){}
GLvoid GL_APIENTRY __glim_SamplerParameteriv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLint* param){}
GLvoid GL_APIENTRY __glim_SamplerParameterf(__GLcontext *gc, GLuint sampler, GLenum pname, GLfloat param){}
GLvoid GL_APIENTRY __glim_SamplerParameterfv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLfloat* param){}
GLvoid GL_APIENTRY __glim_GetSamplerParameteriv(__GLcontext *gc, GLuint sampler, GLenum pname, GLint* params){}
GLvoid GL_APIENTRY __glim_GetSamplerParameterfv(__GLcontext *gc, GLuint sampler, GLenum pname, GLfloat* params){}
GLvoid GL_APIENTRY __glim_VertexAttribDivisor(__GLcontext *gc, GLuint index, GLuint divisor){}
GLvoid GL_APIENTRY __glim_BindTransformFeedback(__GLcontext *gc, GLenum target, GLuint id){}
GLvoid GL_APIENTRY __glim_DeleteTransformFeedbacks(__GLcontext *gc, GLsizei n, const GLuint* ids){}
GLvoid GL_APIENTRY __glim_GenTransformFeedbacks(__GLcontext *gc, GLsizei n, GLuint* ids){}
GLboolean GL_APIENTRY __glim_IsTransformFeedback(__GLcontext *gc, GLuint id){return GL_FALSE;}
GLvoid GL_APIENTRY __glim_PauseTransformFeedback(__GLcontext *gc){}
GLvoid GL_APIENTRY __glim_ResumeTransformFeedback(__GLcontext *gc){}
GLvoid GL_APIENTRY __glim_GetProgramBinary(__GLcontext *gc, GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary){}
GLvoid GL_APIENTRY __glim_ProgramBinary(__GLcontext *gc, GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length){}
GLvoid GL_APIENTRY __glim_ProgramParameteri(__GLcontext *gc, GLuint program, GLenum pname, GLint value){}
GLvoid GL_APIENTRY __glim_InvalidateFramebuffer(__GLcontext *gc, GLenum target, GLsizei numAttachments, const GLenum* attachments){}
GLvoid GL_APIENTRY __glim_InvalidateSubFramebuffer(__GLcontext *gc, GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height){}
GLvoid GL_APIENTRY __glim_TexStorage2D(__GLcontext *gc, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height){}
GLvoid GL_APIENTRY __glim_TexStorage3D(__GLcontext *gc, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth){}
GLvoid GL_APIENTRY __glim_GetInternalformativ(__GLcontext *gc, GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params){}

/* OpenGL ES 3.1 */

GLvoid GL_APIENTRY __glim_DispatchCompute(__GLcontext *gc, GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z){}
GLvoid GL_APIENTRY __glim_DispatchComputeIndirect(__GLcontext *gc, GLintptr indirect){}
GLvoid GL_APIENTRY __glim_DrawArraysIndirect(__GLcontext *gc, GLenum mode, const void *indirect){}
GLvoid GL_APIENTRY __glim_DrawElementsIndirect(__GLcontext *gc, GLenum mode, GLenum type, const void *indirect){}
GLvoid GL_APIENTRY __glim_FramebufferParameteri(__GLcontext *gc, GLenum target, GLenum pname, GLint param){}
GLvoid GL_APIENTRY __glim_GetFramebufferParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint *params){}
GLvoid GL_APIENTRY __glim_GetProgramInterfaceiv(__GLcontext *gc, GLuint program, GLenum programInterface, GLenum pname, GLint *params){}
GLuint GL_APIENTRY __glim_GetProgramResourceIndex(__GLcontext *gc, GLuint program, GLenum programInterface, const GLchar *name){return 0;}
GLvoid GL_APIENTRY __glim_GetProgramResourceName(__GLcontext *gc, GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name){}
GLvoid GL_APIENTRY __glim_GetProgramResourceiv(__GLcontext *gc, GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params){}
GLint GL_APIENTRY __glim_GetProgramResourceLocation(__GLcontext *gc, GLuint program, GLenum programInterface, const GLchar *name){return -1;}
GLvoid GL_APIENTRY __glim_UseProgramStages(__GLcontext *gc, GLuint pipeline, GLbitfield stages, GLuint program){}
GLvoid GL_APIENTRY __glim_ActiveShaderProgram(__GLcontext *gc, GLuint pipeline, GLuint program){}
GLuint GL_APIENTRY __glim_CreateShaderProgramv(__GLcontext *gc, GLenum type, GLsizei count, const GLchar *const*strings){return 0;}
GLvoid GL_APIENTRY __glim_BindProgramPipeline(__GLcontext *gc, GLuint pipeline){}
GLvoid GL_APIENTRY __glim_DeleteProgramPipelines(__GLcontext *gc, GLsizei n, const GLuint *pipelines){}
GLvoid GL_APIENTRY __glim_GenProgramPipelines(__GLcontext *gc, GLsizei n, GLuint *pipelines){}
GLboolean GL_APIENTRY __glim_IsProgramPipeline(__GLcontext *gc, GLuint pipeline){return GL_FALSE;}
GLvoid GL_APIENTRY __glim_GetProgramPipelineiv(__GLcontext *gc, GLuint pipeline, GLenum pname, GLint *params){}
GLvoid GL_APIENTRY __glim_ProgramUniform1i(__GLcontext *gc, GLuint program, GLint location, GLint v0){}
GLvoid GL_APIENTRY __glim_ProgramUniform2i(__GLcontext *gc, GLuint program, GLint location, GLint v0, GLint v1){}
GLvoid GL_APIENTRY __glim_ProgramUniform3i(__GLcontext *gc, GLuint program, GLint location, GLint v0, GLint v1, GLint v2){}
GLvoid GL_APIENTRY __glim_ProgramUniform4i(__GLcontext *gc, GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3){}
GLvoid GL_APIENTRY __glim_ProgramUniform1ui(__GLcontext *gc, GLuint program, GLint location, GLuint v0){}
GLvoid GL_APIENTRY __glim_ProgramUniform2ui(__GLcontext *gc, GLuint program, GLint location, GLuint v0, GLuint v1){}
GLvoid GL_APIENTRY __glim_ProgramUniform3ui(__GLcontext *gc, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2){}
GLvoid GL_APIENTRY __glim_ProgramUniform4ui(__GLcontext *gc, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3){}
GLvoid GL_APIENTRY __glim_ProgramUniform1f(__GLcontext *gc, GLuint program, GLint location, GLfloat v0){}
GLvoid GL_APIENTRY __glim_ProgramUniform2f(__GLcontext *gc, GLuint program, GLint location, GLfloat v0, GLfloat v1){}
GLvoid GL_APIENTRY __glim_ProgramUniform3f(__GLcontext *gc, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2){}
GLvoid GL_APIENTRY __glim_ProgramUniform4f(__GLcontext *gc, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3){}
GLvoid GL_APIENTRY __glim_ProgramUniform1iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value){}
GLvoid GL_APIENTRY __glim_ProgramUniform2iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value){}
GLvoid GL_APIENTRY __glim_ProgramUniform3iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value){}
GLvoid GL_APIENTRY __glim_ProgramUniform4iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value){}
GLvoid GL_APIENTRY __glim_ProgramUniform1uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value){}
GLvoid GL_APIENTRY __glim_ProgramUniform2uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value){}
GLvoid GL_APIENTRY __glim_ProgramUniform3uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value){}
GLvoid GL_APIENTRY __glim_ProgramUniform4uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value){}
GLvoid GL_APIENTRY __glim_ProgramUniform1fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value){}
GLvoid GL_APIENTRY __glim_ProgramUniform2fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value){}
GLvoid GL_APIENTRY __glim_ProgramUniform3fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value){}
GLvoid GL_APIENTRY __glim_ProgramUniform4fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value){}
GLvoid GL_APIENTRY __glim_ProgramUniformMatrix2fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value){}
GLvoid GL_APIENTRY __glim_ProgramUniformMatrix3fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value){}
GLvoid GL_APIENTRY __glim_ProgramUniformMatrix4fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value){}
GLvoid GL_APIENTRY __glim_ProgramUniformMatrix2x3fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value){}
GLvoid GL_APIENTRY __glim_ProgramUniformMatrix3x2fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value){}
GLvoid GL_APIENTRY __glim_ProgramUniformMatrix2x4fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value){}
GLvoid GL_APIENTRY __glim_ProgramUniformMatrix4x2fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value){}
GLvoid GL_APIENTRY __glim_ProgramUniformMatrix3x4fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value){}
GLvoid GL_APIENTRY __glim_ProgramUniformMatrix4x3fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value){}
GLvoid GL_APIENTRY __glim_ValidateProgramPipeline(__GLcontext *gc, GLuint pipeline){}
GLvoid GL_APIENTRY __glim_GetProgramPipelineInfoLog(__GLcontext *gc, GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog){}
GLvoid GL_APIENTRY __glim_BindImageTexture(__GLcontext *gc, GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format){}
GLvoid GL_APIENTRY __glim_GetBooleani_v(__GLcontext *gc, GLenum target, GLuint index, GLboolean *data){}
GLvoid GL_APIENTRY __glim_MemoryBarrier(__GLcontext *gc, GLbitfield barriers){}
GLvoid GL_APIENTRY __glim_MemoryBarrierByRegion(__GLcontext *gc, GLbitfield barriers){}
GLvoid GL_APIENTRY __glim_TexStorage2DMultisample(__GLcontext *gc, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations){}
GLvoid GL_APIENTRY __glim_GetMultisamplefv(__GLcontext *gc, GLenum pname, GLuint index, GLfloat *val){}
GLvoid GL_APIENTRY __glim_SampleMaski(__GLcontext *gc, GLuint maskNumber, GLbitfield mask){}
//GLvoid GL_APIENTRY __glim_GetTexLevelParameteriv(__GLcontext *gc, GLenum target, GLint level, GLenum pname, GLint *params){}
//GLvoid GL_APIENTRY __glim_GetTexLevelParameterfv(__GLcontext *gc, GLenum target, GLint level, GLenum pname, GLfloat *params){}
GLvoid GL_APIENTRY __glim_BindVertexBuffer(__GLcontext *gc, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride){}
GLvoid GL_APIENTRY __glim_VertexAttribFormat(__GLcontext *gc, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset){}
GLvoid GL_APIENTRY __glim_VertexAttribIFormat(__GLcontext *gc, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset){}
GLvoid GL_APIENTRY __glim_VertexAttribBinding(__GLcontext *gc, GLuint attribindex, GLuint bindingindex){}
GLvoid GL_APIENTRY __glim_VertexBindingDivisor(__GLcontext *gc, GLuint bindingindex, GLuint divisor){}

/* OpenGL ES 3.2 */
GLvoid GL_APIENTRY __glim_TexStorage3DMultisample(__GLcontext *gc, GLenum target, GLsizei samples, GLenum sizedinternalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations){}

GLvoid GL_APIENTRY __glim_BlendBarrier(__GLcontext *gc){}

GLvoid GL_APIENTRY __glim_DebugMessageControl(__GLcontext *gc, GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled){}
GLvoid GL_APIENTRY __glim_DebugMessageInsert(__GLcontext *gc, GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf){}
GLvoid GL_APIENTRY __glim_DebugMessageCallback(__GLcontext *gc, GLDEBUGPROCKHR callback, const GLvoid* userParam){}
GLuint GL_APIENTRY __glim_GetDebugMessageLog(__GLcontext *gc, GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog){return 0;}
GLvoid GL_APIENTRY __glim_GetPointerv(__GLcontext *gc, GLenum pname, GLvoid** params){}
GLvoid GL_APIENTRY __glim_PushDebugGroup(__GLcontext *gc, GLenum source, GLuint id, GLsizei length, const GLchar * message){}
GLvoid GL_APIENTRY __glim_PopDebugGroup(__GLcontext *gc){}
GLvoid GL_APIENTRY __glim_ObjectLabel(__GLcontext *gc, GLenum identifier, GLuint name, GLsizei length, const GLchar *label){}
GLvoid GL_APIENTRY __glim_GetObjectLabel(__GLcontext *gc, GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label){}
GLvoid GL_APIENTRY __glim_ObjectPtrLabel(__GLcontext *gc, const GLvoid* ptr, GLsizei length, const GLchar *label){}
GLvoid GL_APIENTRY __glim_GetObjectPtrLabel(__GLcontext *gc, const GLvoid* ptr, GLsizei bufSize, GLsizei *length, GLchar *label){}


GLenum GL_APIENTRY __glim_GetGraphicsResetStatus(__GLcontext *gc){return GL_NO_ERROR;}
GLvoid GL_APIENTRY __glim_ReadnPixels(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLvoid *data){}
GLvoid GL_APIENTRY __glim_GetnUniformfv(__GLcontext *gc, GLuint program, GLint location, GLsizei bufSize, GLfloat *params){}
GLvoid GL_APIENTRY __glim_GetnUniformiv(__GLcontext *gc, GLuint program, GLint location, GLsizei bufSize, GLint *params){}
GLvoid GL_APIENTRY __glim_GetnUniformuiv(__GLcontext *gc, GLuint program, GLint location, GLsizei bufSize, GLuint *params){}

GLvoid GL_APIENTRY __glim_BlendEquationi(__GLcontext * gc, GLuint buf, GLenum mode){}
GLvoid GL_APIENTRY __glim_BlendEquationSeparatei(__GLcontext * gc, GLuint buf, GLenum modeRGB, GLenum modeAlpha){}
GLvoid GL_APIENTRY __glim_BlendFunci(__GLcontext * gc, GLuint buf, GLenum sfactor, GLenum dfactor){}
GLvoid GL_APIENTRY __glim_BlendFuncSeparatei(__GLcontext * gc, GLuint buf, GLenum sfactorRGB,GLenum dfactorRGB,GLenum sfactorAlpha,GLenum dfactorAlpha){}
GLvoid GL_APIENTRY __glim_ColorMaski(__GLcontext * gc,GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a){}
GLvoid GL_APIENTRY __glim_Enablei(__GLcontext *gc, GLenum target, GLuint index){}
GLvoid GL_APIENTRY __glim_Disablei(__GLcontext *gc, GLenum target, GLuint index){}
GLboolean GL_APIENTRY __glim_IsEnabledi(__GLcontext * gc, GLenum target, GLuint index){return GL_FALSE;}

GLvoid GL_APIENTRY __glim_TexParameterIiv(__GLcontext *gc, GLenum target, GLenum pname, const GLint *params){}
GLvoid GL_APIENTRY __glim_TexParameterIuiv(__GLcontext *gc, GLenum target, GLenum pname, const GLuint *params){}
GLvoid GL_APIENTRY __glim_GetTexParameterIiv(__GLcontext *gc, GLenum target, GLenum pname, GLint *params){}
GLvoid GL_APIENTRY __glim_GetTexParameterIuiv(__GLcontext *gc, GLenum target, GLenum pname, GLuint *params){}
GLvoid GL_APIENTRY __glim_SamplerParameterIiv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLint *param){}
GLvoid GL_APIENTRY __glim_SamplerParameterIuiv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLuint *param){}
GLvoid GL_APIENTRY __glim_GetSamplerParameterIiv(__GLcontext *gc, GLuint sampler, GLenum pname, GLint *params){}
GLvoid GL_APIENTRY __glim_GetSamplerParameterIuiv(__GLcontext *gc, GLuint sampler, GLenum pname, GLuint *params){}

GLvoid GL_APIENTRY __glim_TexBuffer(__GLcontext *gc, GLenum target, GLenum internalformat, GLuint buffer){}
GLvoid GL_APIENTRY __glim_TexBufferRange(__GLcontext *gc, GLenum target, GLenum internalformat,
                                                   GLuint buffer, GLintptr offset, GLsizeiptr size){}
GLvoid GL_APIENTRY __glim_PatchParameteri(__GLcontext *gc, GLenum pname, GLint value){}

GLvoid GL_APIENTRY __glim_FramebufferTexture(__GLcontext *gc, GLenum target, GLenum attachment, GLuint texture, GLint level){}

GLvoid GL_APIENTRY __glim_MinSampleShading(__GLcontext *gc, GLfloat value){}

GLvoid GL_APIENTRY __glim_CopyImageSubData(__GLcontext *gc,
                                           GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ,
                                           GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ,
                                           GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth){}

GLvoid GL_APIENTRY __glim_DrawElementsBaseVertex(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex){}
GLvoid GL_APIENTRY __glim_DrawRangeElementsBaseVertex(__GLcontext *gc, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex){}
GLvoid GL_APIENTRY __glim_DrawElementsInstancedBaseVertex(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex){}
GLvoid GL_APIENTRY __glim_PrimitiveBoundingBox(__GLcontext *gc, GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW){}

/* OpenGL ES extension */
#if GL_OES_EGL_image
GLvoid GL_APIENTRY __glim_EGLImageTargetTexture2DOES(__GLcontext *gc, GLenum target, GLeglImageOES image){}
GLvoid GL_APIENTRY __glim_EGLImageTargetRenderbufferStorageOES(__GLcontext *gc, GLenum target, GLeglImageOES image){}
#endif

#if GL_EXT_multi_draw_arrays
GLvoid GL_APIENTRY __glim_MultiDrawArraysEXT(__GLcontext *gc, GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount){}
GLvoid GL_APIENTRY __glim_MultiDrawElementsEXT(__GLcontext *gc, GLenum mode, const GLsizei *count, GLenum type, const GLvoid*const*indices, GLsizei primcount){}
#if GL_EXT_draw_elements_base_vertex
GLvoid GL_APIENTRY __glim_MultiDrawElementsBaseVertexEXT(__GLcontext *gc, GLenum mode, const GLsizei * count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint * basevertex){}
#endif
#endif

#if GL_OES_mapbuffer
GLvoid GL_APIENTRY __glim_GetBufferPointervOES(__GLcontext *gc, GLenum target, GLenum pname, GLvoid** params){}
GLvoid* GL_APIENTRY __glim_MapBufferOES(__GLcontext *gc, GLenum target, GLenum access){return gcvNULL;}
GLboolean GL_APIENTRY __glim_UnmapBufferOES(__GLcontext *gc, GLenum target){return GL_FALSE;}
#endif

#if GL_EXT_discard_framebuffer
GLvoid GL_APIENTRY __glim_DiscardFramebufferEXT(__GLcontext *gc, GLenum target, GLsizei numAttachments, const GLenum *attachments){}
#endif

#if GL_EXT_multisampled_render_to_texture
GLvoid GL_APIENTRY __glim_RenderbufferStorageMultisampleEXT(__GLcontext *gc, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height){}
GLvoid GL_APIENTRY __glim_FramebufferTexture2DMultisampleEXT(__GLcontext *gc, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples){}
#endif

#if GL_VIV_direct_texture
GLvoid GL_APIENTRY __glim_TexDirectVIV(__GLcontext *gc, GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** pixels){}
GLvoid GL_APIENTRY __glim_TexDirectInvalidateVIV(__GLcontext *gc, GLenum target){}
GLvoid GL_APIENTRY __glim_TexDirectVIVMap(__GLcontext *gc, GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** logical, const GLuint * physical){}
GLvoid GL_APIENTRY __glim_TexDirectTiledMapVIV(__GLcontext *gc, GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** logical, const GLuint * physical){}
#endif

#if GL_EXT_multi_draw_indirect
GLvoid GL_APIENTRY __glim_MultiDrawArraysIndirectEXT(__GLcontext *gc, GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride){}
GLvoid GL_APIENTRY __glim_MultiDrawElementsIndirectEXT(__GLcontext *gc, GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride){}
#endif

GLvoid GL_APIENTRY __glim_GetObjectParameterivARB (__GLcontext *gc, UINT obj, GLenum pname, GLint *params){}
/* To do */
//GLvoid GLAPIENTRY __glim_GetVertexAttribdv(__GLcontext *gc, GLuint index, GLenum pname, GLdouble* params){}
GLvoid GL_APIENTRY __glim_GetQueryObjectiv(__GLcontext *gc, GLuint id, GLenum pname, GLint* params){}
GLvoid GL_APIENTRY __glim_GetBufferSubData (__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data){}
GLvoid GL_APIENTRY __glim_DeleteObjectARB (__GLcontext *gc, UINT obj){}
GLvoid GL_APIENTRY __glim_GetInfoLogARB (__GLcontext *gc, UINT obj, GLsizei maxLength, GLsizei *length, char *infoLog){}
GLvoid GL_APIENTRY __glim_ClampColorARB (__GLcontext *gc, GLenum target, GLenum clamp){}

GLvoid GL_APIENTRY __glim_GetUniformdv(__GLcontext *gc,  GLuint program, GLint location, GLdouble *params){}
GLvoid GL_APIENTRY __glim_Uniform1d(__GLcontext *gc, GLint location, GLdouble v0){}
GLvoid GL_APIENTRY __glim_Uniform2d(__GLcontext *gc, GLint location, GLdouble v0, GLdouble v1){}
GLvoid GL_APIENTRY __glim_Uniform3d(__GLcontext *gc, GLint location, GLdouble v0, GLdouble v1, GLdouble v2){}
GLvoid GL_APIENTRY __glim_Uniform4d(__GLcontext *gc, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3){}
GLvoid GL_APIENTRY __glim_Uniform1dv(__GLcontext *gc, GLint location, GLsizei count, const GLdouble* value){}
GLvoid GL_APIENTRY __glim_Uniform2dv(__GLcontext *gc, GLint location, GLsizei count, const GLdouble* value){}
GLvoid GL_APIENTRY __glim_Uniform3dv(__GLcontext *gc, GLint location, GLsizei count, const GLdouble* value){}
GLvoid GL_APIENTRY __glim_Uniform4dv(__GLcontext *gc, GLint location, GLsizei count, const GLdouble* value){}
GLvoid GL_APIENTRY __glim_UniformMatrix2dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value){}
GLvoid GL_APIENTRY __glim_UniformMatrix3dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value){}
GLvoid GL_APIENTRY __glim_UniformMatrix4dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value){}
GLvoid GL_APIENTRY __glim_UniformMatrix2x3dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value){}
GLvoid GL_APIENTRY __glim_UniformMatrix3x2dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value){}
GLvoid GL_APIENTRY __glim_UniformMatrix2x4dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value){}
GLvoid GL_APIENTRY __glim_UniformMatrix4x2dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value){}
GLvoid GL_APIENTRY __glim_UniformMatrix3x4dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value){}
GLvoid GL_APIENTRY __glim_UniformMatrix4x3dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value){}



#define __glle_NewList                     ((__T_NewList                   )__glapi_Nop)
#define __glle_EndList                     ((__T_EndList                   )__glapi_Nop)
#define __glle_Color3b                     ((__T_Color3b                   )__glapi_Nop)
#define __glle_Color3bv                    ((__T_Color3bv                   )__glapi_Nop)
#define __glle_Color3d                     ((__T_Color3d                   )__glapi_Nop)
#define __glle_Color3dv                    ((__T_Color3dv                   )__glapi_Nop)
#define __glle_Color3f                     ((__T_Color3f                   )__glapi_Nop)
#define __glle_Color3i                     ((__T_Color3i                   )__glapi_Nop)
#define __glle_Color3iv                    ((__T_Color3iv                   )__glapi_Nop)
#define __glle_Color3s                     ((__T_Color3s                   )__glapi_Nop)
#define __glle_Color3sv                    ((__T_Color3sv                   )__glapi_Nop)
#define __glle_Color3ub                    ((__T_Color3ub                   )__glapi_Nop)
#define __glle_Color3ubv                   ((__T_Color3ubv                   )__glapi_Nop)
#define __glle_Color3ui                    ((__T_Color3ui                   )__glapi_Nop)
#define __glle_Color3uiv                   ((__T_Color3uiv                   )__glapi_Nop)
#define __glle_Color3us                    ((__T_Color3us                   )__glapi_Nop)
#define __glle_Color3usv                   ((__T_Color3usv                   )__glapi_Nop)
#define __glle_Color4b                     ((__T_Color4b                   )__glapi_Nop)
#define __glle_Color4bv                    ((__T_Color4bv                   )__glapi_Nop)
#define __glle_Color4d                     ((__T_Color4d                   )__glapi_Nop)
#define __glle_Color4dv                    ((__T_Color4dv                   )__glapi_Nop)
#define __glle_Color4f                     ((__T_Color4f                   )__glapi_Nop)
#define __glle_Color4i                     ((__T_Color4i                   )__glapi_Nop)
#define __glle_Color4iv                    ((__T_Color4iv                   )__glapi_Nop)
#define __glle_Color4s                     ((__T_Color4s                   )__glapi_Nop)
#define __glle_Color4sv                    ((__T_Color4sv                   )__glapi_Nop)
#define __glle_Color4ub                    ((__T_Color4ub                   )__glapi_Nop)
#define __glle_Color4ui                    ((__T_Color4ui                   )__glapi_Nop)
#define __glle_Color4uiv                   ((__T_Color4uiv                   )__glapi_Nop)
#define __glle_Color4us                    ((__T_Color4us                   )__glapi_Nop)
#define __glle_Color4usv                   ((__T_Color4usv                   )__glapi_Nop)
#define __glle_EdgeFlagv                   ((__T_EdgeFlagv                   )__glapi_Nop)
#define __glle_Indexd                      ((__T_Indexd                   )__glapi_Nop)
#define __glle_Indexdv                     ((__T_Indexdv                   )__glapi_Nop)
#define __glle_Indexfv                     ((__T_Indexfv                   )__glapi_Nop)
#define __glle_Indexi                      ((__T_Indexi                   )__glapi_Nop)
#define __glle_Indexiv                     ((__T_Indexiv                   )__glapi_Nop)
#define __glle_Indexs                      ((__T_Indexs                   )__glapi_Nop)
#define __glle_Indexsv                     ((__T_Indexsv                   )__glapi_Nop)
#define __glle_Normal3b                    ((__T_Normal3b                   )__glapi_Nop)
#define __glle_Normal3bv                   ((__T_Normal3bv                    )__glapi_Nop)
#define __glle_Normal3d                    ((__T_Normal3d                   )__glapi_Nop)
#define __glle_Normal3dv                   ((__T_Normal3dv                   )__glapi_Nop)
#define __glle_Normal3f                    ((__T_Normal3f                   )__glapi_Nop)
#define __glle_Normal3i                    ((__T_Normal3i                   )__glapi_Nop)
#define __glle_Normal3iv                   ((__T_Normal3iv                   )__glapi_Nop)
#define __glle_Normal3s                    ((__T_Normal3s                   )__glapi_Nop)
#define __glle_Normal3sv                   ((__T_Normal3sv                   )__glapi_Nop)
#define __glle_RasterPos2d                 ((__T_RasterPos2d                   )__glapi_Nop)
#define __glle_RasterPos2dv                ((__T_RasterPos2dv                   )__glapi_Nop)
#define __glle_RasterPos2f                 ((__T_RasterPos2f                   )__glapi_Nop)
#define __glle_RasterPos2i                 ((__T_RasterPos2i                   )__glapi_Nop)
#define __glle_RasterPos2iv                ((__T_RasterPos2iv                   )__glapi_Nop)
#define __glle_RasterPos2s                 ((__T_RasterPos2s                   )__glapi_Nop)
#define __glle_RasterPos2sv                ((__T_RasterPos2sv                   )__glapi_Nop)
#define __glle_RasterPos3d                 ((__T_RasterPos3d                   )__glapi_Nop)
#define __glle_RasterPos3dv                ((__T_RasterPos3dv                   )__glapi_Nop)
#define __glle_RasterPos3f                 ((__T_RasterPos3f                   )__glapi_Nop)
#define __glle_RasterPos3i                 ((__T_RasterPos3i                   )__glapi_Nop)
#define __glle_RasterPos3iv                ((__T_RasterPos3iv                   )__glapi_Nop)
#define __glle_RasterPos3s                 ((__T_RasterPos3s                   )__glapi_Nop)
#define __glle_RasterPos3sv                ((__T_RasterPos3sv                   )__glapi_Nop)
#define __glle_RasterPos4d                 ((__T_RasterPos4d                   )__glapi_Nop)
#define __glle_RasterPos4dv                ((__T_RasterPos4dv                   )__glapi_Nop)
#define __glle_RasterPos4f                 ((__T_RasterPos4f                   )__glapi_Nop)
#define __glle_RasterPos4i                 ((__T_RasterPos4i                   )__glapi_Nop)
#define __glle_RasterPos4iv                ((__T_RasterPos4iv                   )__glapi_Nop)
#define __glle_RasterPos4s                 ((__T_RasterPos4s                   )__glapi_Nop)
#define __glle_RasterPos4sv                ((__T_RasterPos4sv                   )__glapi_Nop)
#define __glle_Rectd                       ((__T_Rectd                   )__glapi_Nop)
#define __glle_Rectdv                      ((__T_Rectdv                   )__glapi_Nop)
#define __glle_Rectfv                      ((__T_Rectfv                   )__glapi_Nop)
#define __glle_Recti                       ((__T_Recti                   )__glapi_Nop)
#define __glle_Rectiv                      ((__T_Rectiv                   )__glapi_Nop)
#define __glle_Rects                       ((__T_Rects                   )__glapi_Nop)
#define __glle_Rectsv                      ((__T_Rectsv                   )__glapi_Nop)
#define __glle_TexCoord1d                  ((__T_TexCoord1d                   )__glapi_Nop)
#define __glle_TexCoord1dv                 ((__T_TexCoord1dv                   )__glapi_Nop)
#define __glle_TexCoord1f                  ((__T_TexCoord1f                   )__glapi_Nop)
#define __glle_TexCoord1fv                 ((__T_TexCoord1fv                   )__glapi_Nop)
#define __glle_TexCoord1i                  ((__T_TexCoord1i                   )__glapi_Nop)
#define __glle_TexCoord1iv                 ((__T_TexCoord1iv                  )__glapi_Nop)
#define __glle_TexCoord1s                  ((__T_TexCoord1s                   )__glapi_Nop)
#define __glle_TexCoord1sv                 ((__T_TexCoord1sv                   )__glapi_Nop)
#define __glle_TexCoord2d                  ((__T_TexCoord2d                   )__glapi_Nop)
#define __glle_TexCoord2dv                 ((__T_TexCoord2dv                   )__glapi_Nop)
#define __glle_TexCoord2f                  ((__T_TexCoord2f                   )__glapi_Nop)
#define __glle_TexCoord2i                  ((__T_TexCoord2i                   )__glapi_Nop)
#define __glle_TexCoord2iv                 ((__T_TexCoord2iv                   )__glapi_Nop)
#define __glle_TexCoord2s                  ((__T_TexCoord2s                   )__glapi_Nop)
#define __glle_TexCoord2sv                 ((__T_TexCoord2sv                   )__glapi_Nop)
#define __glle_TexCoord3d                  ((__T_TexCoord3d                   )__glapi_Nop)
#define __glle_TexCoord3dv                 ((__T_TexCoord3dv                   )__glapi_Nop)
#define __glle_TexCoord3f                  ((__T_TexCoord3f                   )__glapi_Nop)
#define __glle_TexCoord3i                  ((__T_TexCoord3i                   )__glapi_Nop)
#define __glle_TexCoord3iv                 ((__T_TexCoord3iv                   )__glapi_Nop)
#define __glle_TexCoord3s                  ((__T_TexCoord3s                   )__glapi_Nop)
#define __glle_TexCoord3sv                 ((__T_TexCoord3sv                   )__glapi_Nop)
#define __glle_TexCoord4d                  ((__T_TexCoord4d                   )__glapi_Nop)
#define __glle_TexCoord4dv                 ((__T_TexCoord4dv                   )__glapi_Nop)
#define __glle_TexCoord4f                 ((__T_TexCoord4f                   )__glapi_Nop)

#define __glle_TexCoord4i          ((__T_TexCoord4i                           )__glapi_Nop)
#define __glle_TexCoord4iv         ((__T_TexCoord4iv                          )__glapi_Nop)
#define __glle_TexCoord4s          ((__T_TexCoord4s                      )__glapi_Nop)
#define __glle_TexCoord4sv         ((__T_TexCoord4sv                      )__glapi_Nop)
#define __glle_Vertex2d            ((__T_Vertex2d                        )__glapi_Nop)
#define __glle_Vertex2dv           ((__T_Vertex2dv                        )__glapi_Nop)
#define __glle_Vertex2f            ((__T_Vertex2f                        )__glapi_Nop)
#define __glle_Vertex2i            ((__T_Vertex2i                        )__glapi_Nop)
#define __glle_Vertex2iv           ((__T_Vertex2iv                        )__glapi_Nop)
#define __glle_Vertex2s            ((__T_Vertex2s                        )__glapi_Nop)
#define __glle_Vertex2sv           ((__T_Vertex2sv                        )__glapi_Nop)
#define __glle_Vertex3d            ((__T_Vertex3d                         )__glapi_Nop)
#define __glle_Vertex3dv           ((__T_Vertex3dv                         )__glapi_Nop)
#define __glle_Vertex3f            ((__T_Vertex3f                         )__glapi_Nop)
#define __glle_Vertex3i            ((__T_Vertex3i                          )__glapi_Nop)
#define __glle_Vertex3iv           ((__T_Vertex3iv                        )__glapi_Nop)
#define __glle_Vertex3s            ((__T_Vertex3s                          )__glapi_Nop)
#define __glle_Vertex3sv           ((__T_Vertex3sv                       )__glapi_Nop)
#define __glle_Vertex4d            ((__T_Vertex4d                         )__glapi_Nop)
#define __glle_Vertex4dv           ((__T_Vertex4dv                       )__glapi_Nop)
#define __glle_Vertex4f            ((__T_Vertex4f                         )__glapi_Nop)
#define __glle_Vertex4i            ((__T_Vertex4i                        )__glapi_Nop)
#define __glle_Vertex4iv           ((__T_Vertex4iv                       )__glapi_Nop)
#define __glle_Vertex4s            ((__T_Vertex4s                         )__glapi_Nop)
#define __glle_Vertex4sv           ((__T_Vertex4sv                       )__glapi_Nop)
#define __glle_Fogf                ((__T_Fogf                             )__glapi_Nop)
#define __glle_Fogi                ((__T_Fogi                             )__glapi_Nop)
#define __glle_Lightf              ((__T_Lightf                           )__glapi_Nop)
#define __glle_Lighti              ((__T_Lighti                            )__glapi_Nop)
#define __glle_LightModelf         ((__T_LightModelf                      )__glapi_Nop)
#define __glle_LightModeli         ((__T_LightModeli                       )__glapi_Nop)
#define __glle_Materialf           ((__T_Materialf                         )__glapi_Nop)
#define __glle_Materiali           ((__T_Materiali                      )__glapi_Nop)
#define __glle_TexParameterf       ((__T_TexParameterf                   )__glapi_Nop)
#define __glle_TexParameteri       ((__T_TexParameteri                   )__glapi_Nop)
#define __glle_TexEnvf             ((__T_TexEnvf                        )__glapi_Nop)
#define __glle_TexEnvi             ((__T_TexEnvi                         )__glapi_Nop)
#define __glle_TexGend             ((__T_TexGend                        )__glapi_Nop)
#define __glle_TexGenf             ((__T_TexGenf                         )__glapi_Nop)
#define __glle_TexGeni             ((__T_TexGeni                          )__glapi_Nop)
#define __glle_ArrayElement        ((__T_ArrayElement                       )__glapi_Nop)
#define __glle_DrawArrays          ((__T_DrawArrays                       )__glapi_Nop)
#define __glle_DrawElements        ((__T_DrawElements                      )__glapi_Nop)
#define __glle_Indexub             ((__T_Indexub                          )__glapi_Nop)
#define __glle_Indexubv            ((__T_Indexubv                         )__glapi_Nop)
#define __glle_DrawRangeElements   ((__T_DrawRangeElements                 )__glapi_Nop)
#define __glle_MultiTexCoord1d     ((__T_MultiTexCoord1d                  )__glapi_Nop)
#define __glle_MultiTexCoord1dv    ((__T_MultiTexCoord1dv                  )__glapi_Nop)
#define __glle_MultiTexCoord1f     ((__T_MultiTexCoord1f                     )__glapi_Nop)
#define __glle_MultiTexCoord1fv    ((__T_MultiTexCoord1fv                     )__glapi_Nop)
#define __glle_MultiTexCoord1i     ((__T_MultiTexCoord1i                     )__glapi_Nop)
#define __glle_MultiTexCoord1iv    ((__T_MultiTexCoord1iv                    )__glapi_Nop)
#define __glle_MultiTexCoord1s     ((__T_MultiTexCoord1s                      )__glapi_Nop)
#define __glle_MultiTexCoord1sv    ((__T_MultiTexCoord1sv                    )__glapi_Nop)
#define __glle_MultiTexCoord2d     ((__T_MultiTexCoord2d                      )__glapi_Nop)
#define __glle_MultiTexCoord2dv    ((__T_MultiTexCoord2dv                    )__glapi_Nop)
#define __glle_MultiTexCoord2f     ((__T_MultiTexCoord2f                      )__glapi_Nop)
#define __glle_MultiTexCoord2i     ((__T_MultiTexCoord2i                     )__glapi_Nop)
#define __glle_MultiTexCoord2iv    ((__T_MultiTexCoord2iv                    )__glapi_Nop)
#define __glle_MultiTexCoord2s     ((__T_MultiTexCoord2s                      )__glapi_Nop)
#define __glle_MultiTexCoord2sv    ((__T_MultiTexCoord2sv                    )__glapi_Nop)
#define __glle_MultiTexCoord3d     ((__T_MultiTexCoord3d                      )__glapi_Nop)
#define __glle_MultiTexCoord3dv    ((__T_MultiTexCoord3dv                    )__glapi_Nop)
#define __glle_MultiTexCoord3f     ((__T_MultiTexCoord3f                      )__glapi_Nop)
#define __glle_MultiTexCoord3i     ((__T_MultiTexCoord3i                     )__glapi_Nop)
#define __glle_MultiTexCoord3iv    ((__T_MultiTexCoord3iv                    )__glapi_Nop)
#define __glle_MultiTexCoord3s     ((__T_MultiTexCoord3s                      )__glapi_Nop)
#define __glle_MultiTexCoord3sv    ((__T_MultiTexCoord3sv                    )__glapi_Nop)
#define __glle_MultiTexCoord4d     ((__T_MultiTexCoord4d                      )__glapi_Nop)
#define __glle_MultiTexCoord4dv    ((__T_MultiTexCoord4dv              )__glapi_Nop)
#define __glle_MultiTexCoord4f     ((__T_MultiTexCoord4f                )__glapi_Nop)
#define __glle_MultiTexCoord4i     ((__T_MultiTexCoord4i                )__glapi_Nop)
#define __glle_MultiTexCoord4iv    ((__T_MultiTexCoord4iv              )__glapi_Nop)
#define __glle_MultiTexCoord4s     ((__T_MultiTexCoord4s                )__glapi_Nop)
#define __glle_MultiTexCoord4sv    ((__T_MultiTexCoord4sv              )__glapi_Nop)
#define __glle_MultiDrawArraysEXT  ((__T_MultiDrawArraysEXT             )__glapi_Nop)
#define __glle_MultiDrawElementsEXT    ((__T_MultiDrawElementsEXT               )__glapi_Nop)
#define __glle_PointParameterf     ((__T_PointParameterf                     )__glapi_Nop)
#define __glle_PointParameteri     ((__T_PointParameteri                    )__glapi_Nop)
#define __glle_SecondaryColor3b    ((__T_SecondaryColor3b                    )__glapi_Nop)
#define __glle_SecondaryColor3bv   ((__T_SecondaryColor3bv                  )__glapi_Nop)
#define __glle_SecondaryColor3d    ((__T_SecondaryColor3d                   )__glapi_Nop)
#define __glle_SecondaryColor3dv   ((__T_SecondaryColor3dv                  )__glapi_Nop)
#define __glle_SecondaryColor3f    ((__T_SecondaryColor3f                    )__glapi_Nop)
#define __glle_SecondaryColor3i    ((__T_SecondaryColor3i                   )__glapi_Nop)
#define __glle_SecondaryColor3iv   ((__T_SecondaryColor3iv                   )__glapi_Nop)
#define __glle_SecondaryColor3s    ((__T_SecondaryColor3s                   )__glapi_Nop)
#define __glle_SecondaryColor3sv   ((__T_SecondaryColor3sv                  )__glapi_Nop)
#define __glle_SecondaryColor3ub   ((__T_SecondaryColor3ub                   )__glapi_Nop)
#define __glle_SecondaryColor3ubv  ((__T_SecondaryColor3ubv                 )__glapi_Nop)
#define __glle_SecondaryColor3ui   ((__T_SecondaryColor3ui                   )__glapi_Nop)
#define __glle_SecondaryColor3uiv  ((__T_SecondaryColor3uiv                 )__glapi_Nop)
#define __glle_SecondaryColor3us   ((__T_SecondaryColor3us                   )__glapi_Nop)
#define __glle_SecondaryColor3usv  ((__T_SecondaryColor3usv                 )__glapi_Nop)
#define __glle_WindowPos2d         ((__T_WindowPos2d                        )__glapi_Nop)
#define __glle_WindowPos2dv        ((__T_WindowPos2dv                        )__glapi_Nop)
#define __glle_WindowPos2f         ((__T_WindowPos2f                        )__glapi_Nop)
#define __glle_WindowPos2i         ((__T_WindowPos2i                         )__glapi_Nop)
#define __glle_WindowPos2iv        ((__T_WindowPos2iv                       )__glapi_Nop)
#define __glle_WindowPos2s                                            ((__T_WindowPos2s                                           )__glapi_Nop)
#define __glle_WindowPos2sv                                           ((__T_WindowPos2sv                                          )__glapi_Nop)
#define __glle_WindowPos3d                                            ((__T_WindowPos3d                                      )__glapi_Nop)
#define __glle_WindowPos3dv                                           ((__T_WindowPos3dv                                      )__glapi_Nop)
#define __glle_WindowPos3f                                            ((__T_WindowPos3f                                      )__glapi_Nop)
#define __glle_WindowPos3i                                            ((__T_WindowPos3i                                       )__glapi_Nop)
#define __glle_WindowPos3iv                                           ((__T_WindowPos3iv                                     )__glapi_Nop)
#define __glle_WindowPos3s                                            ((__T_WindowPos3s                                      )__glapi_Nop)
#define __glle_WindowPos3sv                                           ((__T_WindowPos3sv                                      )__glapi_Nop)
#define __glle_MapBufferOES                                           ((__T_MapBufferOES                                     )__glapi_Nop)
#define __glle_UnmapBufferOES                                         ((__T_UnmapBufferOES                                    )__glapi_Nop)
#define __glle_VertexAttrib1d                                         ((__T_VertexAttrib1d                                    )__glapi_Nop)
#define __glle_VertexAttrib1dv                                         ((__T_VertexAttrib1dv                                    )__glapi_Nop)
#define __glle_VertexAttrib1f                                         ((__T_VertexAttrib1f                                    )__glapi_Nop)
#define __glle_VertexAttrib1fv                                        ((__T_VertexAttrib1fv                                    )__glapi_Nop)
#define __glle_VertexAttrib1s                                         ((__T_VertexAttrib1s                                    )__glapi_Nop)
#define __glle_VertexAttrib1sv                                         ((__T_VertexAttrib1sv                                    )__glapi_Nop)
#define __glle_VertexAttrib2d                                         ((__T_VertexAttrib2d                                    )__glapi_Nop)
#define __glle_VertexAttrib2dv                                        ((__T_VertexAttrib2dv                                    )__glapi_Nop)
#define __glle_VertexAttrib2f                                         ((__T_VertexAttrib2f                                    )__glapi_Nop)
#define __glle_VertexAttrib2fv                                        ((__T_VertexAttrib2fv                                    )__glapi_Nop)
#define __glle_VertexAttrib2s                                         ((__T_VertexAttrib2s                                    )__glapi_Nop)
#define __glle_VertexAttrib2sv                                         ((__T_VertexAttrib2sv                                    )__glapi_Nop)
#define __glle_VertexAttrib3d                                         ((__T_VertexAttrib3d                                    )__glapi_Nop)
#define __glle_VertexAttrib3dv                                        ((__T_VertexAttrib3dv                                    )__glapi_Nop)
#define __glle_VertexAttrib3f                                         ((__T_VertexAttrib3f                                    )__glapi_Nop)
#define __glle_VertexAttrib3fv                                        ((__T_VertexAttrib3fv                                    )__glapi_Nop)
#define __glle_VertexAttrib3s                                         ((__T_VertexAttrib3s                                    )__glapi_Nop)
#define __glle_VertexAttrib3sv                                         ((__T_VertexAttrib3sv                                    )__glapi_Nop)
#define __glle_VertexAttrib4Nbv                                         ((__T_VertexAttrib4Nbv                                   )__glapi_Nop)
#define __glle_VertexAttrib4Niv                                         ((__T_VertexAttrib4Niv                                   )__glapi_Nop)
#define __glle_VertexAttrib4Nsv                                         ((__T_VertexAttrib4Nsv                                   )__glapi_Nop)
#define __glle_VertexAttrib4Nub                                         ((__T_VertexAttrib4Nub                                   )__glapi_Nop)
#define __glle_VertexAttrib4Nubv                                         ((__T_VertexAttrib4Nubv                                   )__glapi_Nop)
#define __glle_VertexAttrib4Nuiv                                         ((__T_VertexAttrib4Nuiv                                   )__glapi_Nop)
#define __glle_VertexAttrib4Nusv                                         ((__T_VertexAttrib4Nusv                                   )__glapi_Nop)
#define __glle_VertexAttrib4bv                                         ((__T_VertexAttrib4bv                                    )__glapi_Nop)
#define __glle_VertexAttrib4d                                         ((__T_VertexAttrib4d                                    )__glapi_Nop)
#define __glle_VertexAttrib4dv                                        ((__T_VertexAttrib4dv                                    )__glapi_Nop)
#define __glle_VertexAttrib4f                                         ((__T_VertexAttrib4f                                   )__glapi_Nop)
#define __glle_VertexAttrib4fv                                         ((__T_VertexAttrib4fv                                   )__glapi_Nop)
#define __glle_VertexAttrib4iv                                         ((__T_VertexAttrib4iv                                   )__glapi_Nop)
#define __glle_VertexAttrib4s                                         ((__T_VertexAttrib4s                                   )__glapi_Nop)
#define __glle_VertexAttrib4sv                                         ((__T_VertexAttrib4sv                                   )__glapi_Nop)
#define __glle_VertexAttrib4ubv                                         ((__T_VertexAttrib4ubv                                   )__glapi_Nop)
#define __glle_VertexAttrib4uiv                                         ((__T_VertexAttrib4uiv                                   )__glapi_Nop)
#define __glle_VertexAttrib4usv                                         ((__T_VertexAttrib4usv                                   )__glapi_Nop)
#define __glle_BindFramebuffer                                        ((__T_BindFramebuffer                                   )__glapi_Nop)
#define __glle_BindRenderbuffer                                       ((__T_BindRenderbuffer                                 )__glapi_Nop)
#define __glle_CheckFramebufferStatus                                 ((__T_CheckFramebufferStatus                            )__glapi_Nop)
#define __glle_ClearDepthf                                            ((__T_ClearDepthf                                      )__glapi_Nop)
#define __glle_DeleteFramebuffers                                     ((__T_DeleteFramebuffers                               )__glapi_Nop)
#define __glle_DeleteRenderbuffers                                    ((__T_DeleteRenderbuffers                               )__glapi_Nop)
#define __glle_DepthRangef                                            ((__T_DepthRangef                                      )__glapi_Nop)
#define __glle_FramebufferRenderbuffer                                ((__T_FramebufferRenderbuffer                           )__glapi_Nop)
#define __glle_FramebufferTexture2D                                   ((__T_FramebufferTexture2D                              )__glapi_Nop)
#define __glle_GenerateMipmap                                         ((__T_GenerateMipmap                                    )__glapi_Nop)
#define __glle_GenFramebuffers                                        ((__T_GenFramebuffers                                    )__glapi_Nop)
#define __glle_GenRenderbuffers                                       ((__T_GenRenderbuffers                                  )__glapi_Nop)
#define __glle_GetFramebufferAttachmentParameteriv                    ((__T_GetFramebufferAttachmentParameteriv                )__glapi_Nop)
#define __glle_GetRenderbufferParameteriv                             ((__T_GetRenderbufferParameteriv                         )__glapi_Nop)
#define __glle_GetShaderPrecisionFormat                               ((__T_GetShaderPrecisionFormat                        )__glapi_Nop)
#define __glle_IsFramebuffer                                          ((__T_IsFramebuffer                                    )__glapi_Nop)
#define __glle_IsRenderbuffer                                         ((__T_IsRenderbuffer                                   )__glapi_Nop)
#define __glle_ReleaseShaderCompiler                                  ((__T_ReleaseShaderCompiler                           )__glapi_Nop)
#define __glle_RenderbufferStorage                                    ((__T_RenderbufferStorage                              )__glapi_Nop)
#define __glle_ShaderBinary                                           ((__T_ShaderBinary                                    )__glapi_Nop)
#define __glle_BlitFramebuffer                                        ((__T_BlitFramebuffer                                  )__glapi_Nop)
#define __glle_RenderbufferStorageMultisample                         ((__T_RenderbufferStorageMultisample                    )__glapi_Nop)
#define __glle_FramebufferTextureLayer                                ((__T_FramebufferTextureLayer                             )__glapi_Nop)
#define __glle_MapBufferRange                                         ((__T_MapBufferRange                                    )__glapi_Nop)
#define __glle_FlushMappedBufferRange                                 ((__T_FlushMappedBufferRange                             )__glapi_Nop)
#define __glle_BindVertexArray                                        ((__T_BindVertexArray                                   )__glapi_Nop)
#define __glle_DeleteVertexArrays                                     ((__T_DeleteVertexArrays                                )__glapi_Nop)
#define __glle_GenVertexArrays                                        ((__T_GenVertexArrays                                    )__glapi_Nop)
#define __glle_IsVertexArray                                          ((__T_IsVertexArray                                     )__glapi_Nop)
#define __glle_GetIntegeri_v                                          ((__T_GetIntegeri_v                                      )__glapi_Nop)
#define __glle_BeginTransformFeedback                                 ((__T_BeginTransformFeedback                               )__glapi_Nop)
#define __glle_EndTransformFeedback                                   ((__T_EndTransformFeedback                                  )__glapi_Nop)
#define __glle_BindBufferRange                                        ((__T_BindBufferRange                                      )__glapi_Nop)
#define __glle_BindBufferBase                                         ((__T_BindBufferBase                                       )__glapi_Nop)
#define __glle_TransformFeedbackVaryings                              ((__T_TransformFeedbackVaryings                             )__glapi_Nop)
#define __glle_GetTransformFeedbackVarying                            ((__T_GetTransformFeedbackVarying                          )__glapi_Nop)
#define __glle_VertexAttribIPointer                                   ((__T_VertexAttribIPointer                                  )__glapi_Nop)
#define __glle_GetVertexAttribIiv                                     ((__T_GetVertexAttribIiv                                   )__glapi_Nop)
#define __glle_GetVertexAttribIuiv                                    ((__T_GetVertexAttribIuiv                                   )__glapi_Nop)
#define __glle_VertexAttribI4i                                        ((__T_VertexAttribI4i                                      )__glapi_Nop)
#define __glle_VertexAttribI4ui                                       ((__T_VertexAttribI4ui                                     )__glapi_Nop)
#define __glle_VertexAttribI4iv                                       ((__T_VertexAttribI4iv                                      )__glapi_Nop)
#define __glle_VertexAttribI4uiv                                      ((__T_VertexAttribI4uiv                                    )__glapi_Nop)
#define __glle_GetUniformuiv                                          ((__T_GetUniformuiv                                         )__glapi_Nop)
#define __glle_GetFragDataLocation                                    ((__T_GetFragDataLocation                                  )__glapi_Nop)
#define __glle_BindFragDataLocation                                    ((__T_BindFragDataLocation                                  )__glapi_Nop)
#define __glle_Uniform1ui                                             ((__T_Uniform1ui                                            )__glapi_Nop)
#define __glle_Uniform2ui                                             ((__T_Uniform2ui                                           )__glapi_Nop)
#define __glle_Uniform3ui                                             ((__T_Uniform3ui                                           )__glapi_Nop)
#define __glle_Uniform4ui                                             ((__T_Uniform4ui                                            )__glapi_Nop)
#define __glle_Uniform1uiv                                            ((__T_Uniform1uiv                                          )__glapi_Nop)
#define __glle_Uniform2uiv                                            ((__T_Uniform2uiv                                           )__glapi_Nop)
#define __glle_Uniform3uiv                                            ((__T_Uniform3uiv                                    )__glapi_Nop)
#define __glle_Uniform4uiv                                            ((__T_Uniform4uiv                                     )__glapi_Nop)
#define __glle_ClearBufferiv                                          ((__T_ClearBufferiv                                   )__glapi_Nop)
#define __glle_ClearBufferuiv                                         ((__T_ClearBufferuiv                                 )__glapi_Nop)
#define __glle_ClearBufferfv                                          ((__T_ClearBufferfv                                   )__glapi_Nop)
#define __glle_ClearBufferfi                                          ((__T_ClearBufferfi                                  )__glapi_Nop)
#define __glle_GetStringi                                             ((__T_GetStringi                                      )__glapi_Nop)
#define __glle_CopyBufferSubData                                      ((__T_CopyBufferSubData                                   )__glapi_Nop)
#define __glle_GetUniformIndices                                      ((__T_GetUniformIndices                                    )__glapi_Nop)
#define __glle_GetActiveUniformsiv                                    ((__T_GetActiveUniformsiv                                 )__glapi_Nop)
#define __glle_GetUniformBlockIndex                                   ((__T_GetUniformBlockIndex                                 )__glapi_Nop)
#define __glle_GetActiveUniformBlockiv                                ((__T_GetActiveUniformBlockiv                             )__glapi_Nop)
#define __glle_GetActiveUniformBlockName                              ((__T_GetActiveUniformBlockName                           )__glapi_Nop)
#define __glle_UniformBlockBinding                                    ((__T_UniformBlockBinding                                 )__glapi_Nop)
#define __glle_DrawArraysInstanced                                    ((__T_DrawArraysInstanced                                  )__glapi_Nop)
#define __glle_DrawElementsInstanced                                  ((__T_DrawElementsInstanced                               )__glapi_Nop)
#define __glle_FenceSync                                              ((__T_FenceSync                                            )__glapi_Nop)
#define __glle_IsSync                                                 ((__T_IsSync                                              )__glapi_Nop)
#define __glle_DeleteSync                                             ((__T_DeleteSync                                          )__glapi_Nop)
#define __glle_ClientWaitSync                                         ((__T_ClientWaitSync                                       )__glapi_Nop)
#define __glle_WaitSync                                               ((__T_WaitSync                                            )__glapi_Nop)
#define __glle_GetInteger64v                                          ((__T_GetInteger64v                                        )__glapi_Nop)
#define __glle_GetSynciv                                              ((__T_GetSynciv                                           )__glapi_Nop)
#define __glle_GetInteger64i_v                                        ((__T_GetInteger64i_v                                      )__glapi_Nop)
#define __glle_GetBufferParameteri64v                                 ((__T_GetBufferParameteri64v                              )__glapi_Nop)
#define __glle_GenSamplers                                            ((__T_GenSamplers                                         )__glapi_Nop)
#define __glle_DeleteSamplers                                         ((__T_DeleteSamplers                                       )__glapi_Nop)
#define __glle_IsSampler                                              ((__T_IsSampler                                           )__glapi_Nop)
#define __glle_BindSampler                                            ((__T_BindSampler                                          )__glapi_Nop)
#define __glle_SamplerParameteri                                      ((__T_SamplerParameteri                                   )__glapi_Nop)
#define __glle_SamplerParameteriv                                     ((__T_SamplerParameteriv                                   )__glapi_Nop)

#define __glle_SamplerParameterf                            ((__T_SamplerParameterf                   )__glapi_Nop)
#define __glle_SamplerParameterfv                           ((__T_SamplerParameterfv                  )__glapi_Nop)
#define __glle_GetSamplerParameteriv                        ((__T_GetSamplerParameteriv          )__glapi_Nop)
#define __glle_GetSamplerParameterfv                        ((__T_GetSamplerParameterfv           )__glapi_Nop)
#define __glle_VertexAttribDivisor                          ((__T_VertexAttribDivisor            )__glapi_Nop)
#define __glle_BindTransformFeedback                        ((__T_BindTransformFeedback           )__glapi_Nop)
#define __glle_DeleteTransformFeedbacks                     ((__T_DeleteTransformFeedbacks       )__glapi_Nop)
#define __glle_GenTransformFeedbacks                        ((__T_GenTransformFeedbacks          )__glapi_Nop)
#define __glle_IsTransformFeedback                          ((__T_IsTransformFeedback             )__glapi_Nop)
#define __glle_PauseTransformFeedback                       ((__T_PauseTransformFeedback         )__glapi_Nop)
#define __glle_ResumeTransformFeedback                      ((__T_ResumeTransformFeedback         )__glapi_Nop)
#define __glle_GetProgramBinary                             ((__T_GetProgramBinary                )__glapi_Nop)
#define __glle_ProgramBinary                                ((__T_ProgramBinary                    )__glapi_Nop)
#define __glle_ProgramParameteri                            ((__T_ProgramParameteri               )__glapi_Nop)
#define __glle_InvalidateFramebuffer                        ((__T_InvalidateFramebuffer            )__glapi_Nop)
#define __glle_InvalidateSubFramebuffer                     ((__T_InvalidateSubFramebuffer        )__glapi_Nop)
#define __glle_TexStorage2D                                 ((__T_TexStorage2D                     )__glapi_Nop)
#define __glle_TexStorage3D                                 ((__T_TexStorage3D                   )__glapi_Nop)
#define __glle_GetInternalformativ                          ((__T_GetInternalformativ             )__glapi_Nop)
#define __glle_DispatchCompute                              ((__T_DispatchCompute                )__glapi_Nop)
#define __glle_DispatchComputeIndirect                      ((__T_DispatchComputeIndirect         )__glapi_Nop)
#define __glle_DrawArraysIndirect                           ((__T_DrawArraysIndirect             )__glapi_Nop)
#define __glle_DrawElementsIndirect                         ((__T_DrawElementsIndirect           )__glapi_Nop)
#define __glle_FramebufferParameteri                        ((__T_FramebufferParameteri           )__glapi_Nop)
#define __glle_GetFramebufferParameteriv                    ((__T_GetFramebufferParameteriv      )__glapi_Nop)
#define __glle_GetProgramInterfaceiv                        ((__T_GetProgramInterfaceiv           )__glapi_Nop)
#define __glle_GetProgramResourceIndex                      ((__T_GetProgramResourceIndex         )__glapi_Nop)
#define __glle_GetProgramResourceName                       ((__T_GetProgramResourceName          )__glapi_Nop)
#define __glle_GetProgramResourceiv                         ((__T_GetProgramResourceiv             )__glapi_Nop)
#define __glle_GetProgramResourceLocation                   ((__T_GetProgramResourceLocation      )__glapi_Nop)
#define __glle_UseProgramStages                             ((__T_UseProgramStages              )__glapi_Nop)
#define __glle_ActiveShaderProgram                          ((__T_ActiveShaderProgram           )__glapi_Nop)
#define __glle_CreateShaderProgramv                         ((__T_CreateShaderProgramv       )__glapi_Nop)
#define __glle_BindProgramPipeline                          ((__T_BindProgramPipeline         )__glapi_Nop)
#define __glle_DeleteProgramPipelines                       ((__T_DeleteProgramPipelines      )__glapi_Nop)
#define __glle_GenProgramPipelines                          ((__T_GenProgramPipelines        )__glapi_Nop)
#define __glle_IsProgramPipeline                            ((__T_IsProgramPipeline           )__glapi_Nop)
#define __glle_GetProgramPipelineiv                         ((__T_GetProgramPipelineiv       )__glapi_Nop)
#define __glle_ProgramUniform1i                             ((__T_ProgramUniform1i            )__glapi_Nop)
#define __glle_ProgramUniform2i                             ((__T_ProgramUniform2i             )__glapi_Nop)
#define __glle_ProgramUniform3i                             ((__T_ProgramUniform3i               )__glapi_Nop)
#define __glle_ProgramUniform4i                             ((__T_ProgramUniform4i             )__glapi_Nop)
#define __glle_ProgramUniform1ui                            ((__T_ProgramUniform1ui             )__glapi_Nop)
#define __glle_ProgramUniform2ui                            ((__T_ProgramUniform2ui            )__glapi_Nop)
#define __glle_ProgramUniform3ui                            ((__T_ProgramUniform3ui            )__glapi_Nop)
#define __glle_ProgramUniform4ui                            ((__T_ProgramUniform4ui             )__glapi_Nop)
#define __glle_ProgramUniform1f                             ((__T_ProgramUniform1f             )__glapi_Nop)
#define __glle_ProgramUniform2f                             ((__T_ProgramUniform2f              )__glapi_Nop)
#define __glle_ProgramUniform3f                             ((__T_ProgramUniform3f                )__glapi_Nop)
#define __glle_ProgramUniform4f                             ((__T_ProgramUniform4f                 )__glapi_Nop)
#define __glle_ProgramUniform1iv                            ((__T_ProgramUniform1iv               )__glapi_Nop)
#define __glle_ProgramUniform2iv                            ((__T_ProgramUniform2iv               )__glapi_Nop)
#define __glle_ProgramUniform3iv                            ((__T_ProgramUniform3iv                )__glapi_Nop)
#define __glle_ProgramUniform4iv                            ((__T_ProgramUniform4iv               )__glapi_Nop)
#define __glle_ProgramUniform1uiv                           ((__T_ProgramUniform1uiv               )__glapi_Nop)
#define __glle_ProgramUniform2uiv                           ((__T_ProgramUniform2uiv              )__glapi_Nop)
#define __glle_ProgramUniform3uiv                           ((__T_ProgramUniform3uiv               )__glapi_Nop)
#define __glle_ProgramUniform4uiv                           ((__T_ProgramUniform4uiv              )__glapi_Nop)
#define __glle_ProgramUniform1fv                            ((__T_ProgramUniform1fv               )__glapi_Nop)
#define __glle_ProgramUniform2fv                            ((__T_ProgramUniform2fv                )__glapi_Nop)
#define __glle_ProgramUniform3fv                            ((__T_ProgramUniform3fv               )__glapi_Nop)
#define __glle_ProgramUniform4fv                            ((__T_ProgramUniform4fv                )__glapi_Nop)
#define __glle_ProgramUniformMatrix2fv                      ((__T_ProgramUniformMatrix2fv         )__glapi_Nop)
#define __glle_ProgramUniformMatrix3fv                      ((__T_ProgramUniformMatrix3fv          )__glapi_Nop)
#define __glle_ProgramUniformMatrix4fv                      ((__T_ProgramUniformMatrix4fv         )__glapi_Nop)
#define __glle_ProgramUniformMatrix2x3fv                    ((__T_ProgramUniformMatrix2x3fv       )__glapi_Nop)
#define __glle_ProgramUniformMatrix3x2fv                    ((__T_ProgramUniformMatrix3x2fv        )__glapi_Nop)
#define __glle_ProgramUniformMatrix2x4fv                    ((__T_ProgramUniformMatrix2x4fv       )__glapi_Nop)
#define __glle_ProgramUniformMatrix4x2fv                    ((__T_ProgramUniformMatrix4x2fv        )__glapi_Nop)
#define __glle_ProgramUniformMatrix3x4fv                    ((__T_ProgramUniformMatrix3x4fv        )__glapi_Nop)
#define __glle_ProgramUniformMatrix4x3fv                    ((__T_ProgramUniformMatrix4x3fv              )__glapi_Nop)
#define __glle_ValidateProgramPipeline                      ((__T_ValidateProgramPipeline                )__glapi_Nop)
#define __glle_GetProgramPipelineInfoLog                    ((__T_GetProgramPipelineInfoLog         )__glapi_Nop)
#define __glle_BindImageTexture                             ((__T_BindImageTexture                   )__glapi_Nop)
#define __glle_GetBooleani_v                                ((__T_GetBooleani_v                     )__glapi_Nop)
#define __glle_MemoryBarrier                                ((__T_MemoryBarrier                      )__glapi_Nop)
#define __glle_MemoryBarrierByRegion                        ((__T_MemoryBarrierByRegion             )__glapi_Nop)
#define __glle_TexStorage2DMultisample                      ((__T_TexStorage2DMultisample           )__glapi_Nop)
#define __glle_GetMultisamplefv                             ((__T_GetMultisamplefv                   )__glapi_Nop)
#define __glle_SampleMaski                                  ((__T_SampleMaski                       )__glapi_Nop)
#define __glle_BindVertexBuffer                             ((__T_BindVertexBuffer                   )__glapi_Nop)
#define __glle_VertexAttribFormat                           ((__T_VertexAttribFormat                 )__glapi_Nop)
#define __glle_VertexAttribIFormat                          ((__T_VertexAttribIFormat                 )__glapi_Nop)
#define __glle_VertexAttribBinding                          ((__T_VertexAttribBinding                )__glapi_Nop)
#define __glle_VertexBindingDivisor                         ((__T_VertexBindingDivisor                )__glapi_Nop)
#define __glle_TexStorage3DMultisample                      ((__T_TexStorage3DMultisample            )__glapi_Nop)
#define __glle_BlendBarrier                                 ((__T_BlendBarrier                        )__glapi_Nop)
#define __glle_DebugMessageControl                          ((__T_DebugMessageControl               )__glapi_Nop)
#define __glle_DebugMessageInsert                           ((__T_DebugMessageInsert                 )__glapi_Nop)
#define __glle_DebugMessageCallback                         ((__T_DebugMessageCallback              )__glapi_Nop)
#define __glle_GetDebugMessageLog                           ((__T_GetDebugMessageLog                 )__glapi_Nop)
#define __glle_PushDebugGroup                               ((__T_PushDebugGroup                    )__glapi_Nop)
#define __glle_PopDebugGroup                                ((__T_PopDebugGroup                     )__glapi_Nop)
#define __glle_ObjectLabel                                  ((__T_ObjectLabel                        )__glapi_Nop)
#define __glle_GetObjectLabel                               ((__T_GetObjectLabel                    )__glapi_Nop)
#define __glle_ObjectPtrLabel                               ((__T_ObjectPtrLabel                     )__glapi_Nop)
#define __glle_GetObjectPtrLabel                            ((__T_GetObjectPtrLabel                  )__glapi_Nop)
#define __glle_GetGraphicsResetStatus                       ((__T_GetGraphicsResetStatus             )__glapi_Nop)
#define __glle_ReadnPixels                                  ((__T_ReadnPixels                         )__glapi_Nop)
#define __glle_GetnUniformfv                                ((__T_GetnUniformfv                      )__glapi_Nop)

#define __glle_IsEnabledi                                          ((__T_IsEnabledi                         )__glapi_Nop)
#define __glle_TexParameterIiv                                     ((__T_TexParameterIiv                    )__glapi_Nop)
#define __glle_TexParameterIuiv                                    ((__T_TexParameterIuiv                   )__glapi_Nop)
#define __glle_GetTexParameterIiv                                  ((__T_GetTexParameterIiv                     )__glapi_Nop)
#define __glle_GetTexParameterIuiv                                 ((__T_GetTexParameterIuiv                   )__glapi_Nop)
#define __glle_SamplerParameterIiv                                 ((__T_SamplerParameterIiv                    )__glapi_Nop)
#define __glle_SamplerParameterIuiv                                ((__T_SamplerParameterIuiv                  )__glapi_Nop)
#define __glle_GetSamplerParameterIiv                              ((__T_GetSamplerParameterIiv                )__glapi_Nop)
#define __glle_GetSamplerParameterIuiv                             ((__T_GetSamplerParameterIuiv                )__glapi_Nop)
#define __glle_TexBuffer                                           ((__T_TexBuffer                             )__glapi_Nop)
#define __glle_TexBufferRange                                      ((__T_TexBufferRange                         )__glapi_Nop)
#define __glle_PatchParameteri                                     ((__T_PatchParameteri                        )__glapi_Nop)
#define __glle_FramebufferTexture                                  ((__T_FramebufferTexture                      )__glapi_Nop)
#define __glle_MinSampleShading                                    ((__T_MinSampleShading                       )__glapi_Nop)
#define __glle_CopyImageSubData                                    ((__T_CopyImageSubData                        )__glapi_Nop)
#define __glle_DrawElementsBaseVertex                              ((__T_DrawElementsBaseVertex                 )__glapi_Nop)
#define __glle_DrawRangeElementsBaseVertex                         ((__T_DrawRangeElementsBaseVertex             )__glapi_Nop)
#define __glle_DrawElementsInstancedBaseVertex                     ((__T_DrawElementsInstancedBaseVertex       )__glapi_Nop)
#define __glle_PrimitiveBoundingBox                                ((__T_PrimitiveBoundingBox                   )__glapi_Nop)
#define __glle_EGLImageTargetTexture2DOES                          ((__T_EGLImageTargetTexture2DOES            )__glapi_Nop)
#define __glle_EGLImageTargetRenderbufferStorageOES                ((__T_EGLImageTargetRenderbufferStorageOES   )__glapi_Nop)
#define __glle_MultiDrawElementsBaseVertexEXT                      ((__T_MultiDrawElementsBaseVertexEXT        )__glapi_Nop)
#define __glle_GetBufferPointervOES                                ((__T_GetBufferPointervOES                  )__glapi_Nop)
#define __glle_DiscardFramebufferEXT                               ((__T_DiscardFramebufferEXT                  )__glapi_Nop)
#define __glle_RenderbufferStorageMultisampleEXT                   ((__T_RenderbufferStorageMultisampleEXT     )__glapi_Nop)
#define __glle_FramebufferTexture2DMultisampleEXT                  ((__T_FramebufferTexture2DMultisampleEXT     )__glapi_Nop)
#define __glle_TexDirectVIV                                        ((__T_TexDirectVIV                           )__glapi_Nop)
#define __glle_TexDirectInvalidateVIV                              ((__T_TexDirectInvalidateVIV                 )__glapi_Nop)
#define __glle_TexDirectVIVMap                                     ((__T_TexDirectVIVMap                         )__glapi_Nop)
#define __glle_TexDirectTiledMapVIV                                ((__T_TexDirectTiledMapVIV                   )__glapi_Nop)
#define __glle_MultiDrawArraysIndirectEXT                          ((__T_MultiDrawArraysIndirectEXT             )__glapi_Nop)
#define __glle_MultiDrawElementsIndirectEXT                        ((__T_MultiDrawElementsIndirectEXT           )__glapi_Nop)

#define __glle_GetnUniformiv                  ((__T_GetnUniformiv)__glapi_Nop)
#define __glle_GetnUniformuiv                 ((__T_GetnUniformuiv)__glapi_Nop)
#define __glle_BlendEquationi                 ((__T_BlendEquationi)__glapi_Nop)
#define __glle_BlendEquationSeparatei         ((__T_BlendEquationSeparatei)__glapi_Nop)
#define __glle_BlendFunci                     ((__T_BlendFunci)__glapi_Nop)
#define __glle_BlendFuncSeparatei             ((__T_BlendFuncSeparatei)__glapi_Nop)
#define __glle_ColorMaski                     ((__T_ColorMaski)__glapi_Nop)
#define __glle_Enablei                        ((__T_Enablei)__glapi_Nop)
#define __glle_Disablei                       ((__T_Disablei)__glapi_Nop)

#define __glle_IsRenderbufferEXT ((__T_IsRenderbufferEXT)__glapi_Nop)
#define __glle_BindRenderbufferEXT ((__T_BindRenderbufferEXT)__glapi_Nop)
#define __glle_DeleteRenderbuffersEXT ((__T_DeleteRenderbuffersEXT)__glapi_Nop)
#define __glle_GenRenderbuffersEXT ((__T_GenRenderbuffersEXT)__glapi_Nop)
#define __glle_RenderbufferStorageEXT ((__T_RenderbufferStorageEXT)__glapi_Nop)
#define __glle_GetRenderbufferParameterivEXT ((__T_GetRenderbufferParameterivEXT)__glapi_Nop)
#define __glle_IsFramebufferEXT ((__T_IsFramebufferEXT)__glapi_Nop)
#define __glle_BindFramebufferEXT ((__T_BindFramebufferEXT)__glapi_Nop)
#define __glle_DeleteFramebuffersEXT ((__T_DeleteFramebuffersEXT)__glapi_Nop)
#define __glle_GenFramebuffersEXT ((__T_GenFramebuffersEXT)__glapi_Nop)
#define __glle_CheckFramebufferStatusEXT ((__T_CheckFramebufferStatusEXT)__glapi_Nop)
#define __glle_FramebufferTexture1DEXT ((__T_FramebufferTexture1DEXT)__glapi_Nop)
#define __glle_FramebufferTexture2DEXT ((__T_FramebufferTexture2DEXT)__glapi_Nop)
#define __glle_FramebufferTexture3DEXT ((__T_FramebufferTexture3DEXT)__glapi_Nop)
#define __glle_FramebufferRenderbufferEXT ((__T_FramebufferRenderbufferEXT)__glapi_Nop)
#define __glle_GetFramebufferAttachmentParameterivEXT ((__T_GetFramebufferAttachmentParameterivEXT)__glapi_Nop)
#define __glle_GenerateMipmapEXT ((__T_GenerateMipmapEXT)__glapi_Nop)
#define __glle_BlitFramebufferEXT ((__T_BlitFramebufferEXT)__glapi_Nop)
#define __glle_GetUniformdv ((__T_GetUniformdv)__glapi_Nop)
#define __glle_Uniform1d ((__T_Uniform1d)__glapi_Nop)
#define __glle_Uniform2d ((__T_Uniform2d)__glapi_Nop)
#define __glle_Uniform3d ((__T_Uniform3d)__glapi_Nop)
#define __glle_Uniform4d ((__T_Uniform4d)__glapi_Nop)
#define __glle_Uniform1dv ((__T_Uniform1dv)__glapi_Nop)
#define __glle_Uniform2dv ((__T_Uniform2dv)__glapi_Nop)
#define __glle_Uniform3dv ((__T_Uniform3dv)__glapi_Nop)
#define __glle_Uniform4dv ((__T_Uniform4dv)__glapi_Nop)
#define __glle_UniformMatrix2dv ((__T_UniformMatrix2dv)__glapi_Nop)
#define __glle_UniformMatrix3dv ((__T_UniformMatrix3dv)__glapi_Nop)
#define __glle_UniformMatrix4dv ((__T_UniformMatrix4dv)__glapi_Nop)
#define __glle_UniformMatrix2x3dv ((__T_UniformMatrix2x3dv)__glapi_Nop)
#define __glle_UniformMatrix3x2dv ((__T_UniformMatrix3x2dv)__glapi_Nop)
#define __glle_UniformMatrix2x4dv ((__T_UniformMatrix2x4dv)__glapi_Nop)
#define __glle_UniformMatrix4x2dv ((__T_UniformMatrix4x2dv)__glapi_Nop)
#define __glle_UniformMatrix3x4dv ((__T_UniformMatrix3x4dv)__glapi_Nop)
#define __glle_UniformMatrix4x3dv ((__T_UniformMatrix4x3dv)__glapi_Nop)

#define __glle_ClampColor ((__T_ClampColor)__glapi_Nop)
#define __glle_BeginConditionalRender ((__T_BeginConditionalRender)__glapi_Nop)
#define __glle_EndConditionalRender ((__T_EndConditionalRender)__glapi_Nop)
#define __glle_VertexAttribI1i ((__T_VertexAttribI1i)__glapi_Nop)
#define __glle_VertexAttribI2i ((__T_VertexAttribI2i)__glapi_Nop)
#define __glle_VertexAttribI3i ((__T_VertexAttribI3i)__glapi_Nop)
#define __glle_VertexAttribI1ui ((__T_VertexAttribI1ui)__glapi_Nop)
#define __glle_VertexAttribI2ui ((__T_VertexAttribI2ui)__glapi_Nop)
#define __glle_VertexAttribI3ui ((__T_VertexAttribI3ui)__glapi_Nop)
#define __glle_VertexAttribI1iv ((__T_VertexAttribI1iv)__glapi_Nop)
#define __glle_VertexAttribI2iv ((__T_VertexAttribI2iv)__glapi_Nop)
#define __glle_VertexAttribI3iv ((__T_VertexAttribI3iv)__glapi_Nop)
#define __glle_VertexAttribI1uiv ((__T_VertexAttribI1uiv)__glapi_Nop)
#define __glle_VertexAttribI2uiv ((__T_VertexAttribI2uiv)__glapi_Nop)
#define __glle_VertexAttribI3uiv ((__T_VertexAttribI3uiv)__glapi_Nop)
#define __glle_VertexAttribI4bv ((__T_VertexAttribI4bv)__glapi_Nop)
#define __glle_VertexAttribI4sv ((__T_VertexAttribI4sv)__glapi_Nop)
#define __glle_VertexAttribI4ubv ((__T_VertexAttribI4ubv)__glapi_Nop)
#define __glle_VertexAttribI4usv ((__T_VertexAttribI4usv)__glapi_Nop)
#define __glle_FramebufferTexture1D ((__T_FramebufferTexture1D)__glapi_Nop)
#define __glle_FramebufferTexture3D ((__T_FramebufferTexture3D)__glapi_Nop)
#define __glle_PrimitiveRestartIndex ((__T_PrimitiveRestartIndex)__glapi_Nop)
#define __glle_GetActiveUniformName ((__T_GetActiveUniformName)__glapi_Nop)
#define __glle_MultiDrawArrays ((__T_MultiDrawArrays)__glapi_Nop)
#define __glle_MultiDrawElements ((__T_MultiDrawElements)__glapi_Nop)


#define __gllc_NewList                          ((__T_NewList                   )__glapi_Nop)
#define __gllc_EndList                          ((__T_EndList                   )__glapi_Nop)
#define __gllc_GenLists                         ((__T_GenLists                  )__glapi_Nop)
#define __gllc_DeleteLists                      ((__T_DeleteLists               )__glapi_Nop)
#define __gllc_FeedbackBuffer                   ((__T_FeedbackBuffer            )__glapi_Nop)
#define __gllc_SelectBuffer                     ((__T_SelectBuffer              )__glapi_Nop)
#define __gllc_RenderMode                       ((__T_RenderMode                )__glapi_Nop)
#define __gllc_ClientActiveTexture              ((__T_ClientActiveTexture       )__glapi_Nop)
#define __gllc_ColorPointer                     ((__T_ColorPointer              )__glapi_Nop)
#define __gllc_EdgeFlagPointer                  ((__T_EdgeFlagPointer           )__glapi_Nop)
#define __gllc_FogCoordPointer                  ((__T_FogCoordPointer           )__glapi_Nop)
#define __gllc_IndexPointer                     ((__T_IndexPointer              )__glapi_Nop)
#define __gllc_InterleavedArrays                ((__T_InterleavedArrays         )__glapi_Nop)
#define __gllc_NormalPointer                    ((__T_NormalPointer             )__glapi_Nop)
#define __gllc_SecondaryColorPointer            ((__T_SecondaryColorPointer     )__glapi_Nop)
#define __gllc_TexCoordPointer                  ((__T_TexCoordPointer           )__glapi_Nop)
#define __gllc_VertexAttribPointer              ((__T_VertexAttribPointer       )__glapi_Nop)
#define __gllc_VertexPointer                    ((__T_VertexPointer             )__glapi_Nop)
#define __gllc_EnableClientState                ((__T_EnableClientState         )__glapi_Nop)
#define __gllc_DisableClientState               ((__T_DisableClientState        )__glapi_Nop)
#define __gllc_EnableVertexAttribArray          ((__T_EnableVertexAttribArray   )__glapi_Nop)
#define __gllc_DisableVertexAttribArray         ((__T_DisableVertexAttribArray  )__glapi_Nop)
#define __gllc_PushClientAttrib                 ((__T_PushClientAttrib          )__glapi_Nop)
#define __gllc_PopClientAttrib                  ((__T_PopClientAttrib           )__glapi_Nop)
#define __gllc_PixelStorei                      ((__T_PixelStorei               )__glapi_Nop)
#define __gllc_PixelStoref                      ((__T_PixelStoref               )__glapi_Nop)
#define __gllc_ReadPixels                       ((__T_ReadPixels                )__glapi_Nop)
#define __gllc_GenTextures                      ((__T_GenTextures               )__glapi_Nop)
#define __gllc_DeleteTextures                   ((__T_DeleteTextures            )__glapi_Nop)
#define __gllc_AreTexturesResident              ((__T_AreTexturesResident       )__glapi_Nop)
#define __gllc_GenQueries                       ((__T_GenQueries                )__glapi_Nop)
#define __gllc_DeleteQueries                    ((__T_DeleteQueries             )__glapi_Nop)
#define __gllc_GenBuffers                       ((__T_GenBuffers                )__glapi_Nop)
#define __gllc_DeleteBuffers                    ((__T_DeleteBuffers             )__glapi_Nop)
#define __gllc_BindBuffer                       ((__T_BindBuffer                )__glapi_Nop)
#define __gllc_BufferData                       ((__T_BufferData                )__glapi_Nop)
#define __gllc_BufferSubData                    ((__T_BufferSubData             )__glapi_Nop)
#define __gllc_MapBuffer                        ((__T_MapBuffer                 )__glapi_Nop)
#define __gllc_UnmapBuffer                      ((__T_UnmapBuffer               )__glapi_Nop)
#define __gllc_CreateProgram                    ((__T_CreateProgram             )__glapi_Nop)
#define __gllc_CreateShader                     ((__T_CreateShader              )__glapi_Nop)
#define __gllc_DeleteProgram                    ((__T_DeleteProgram             )__glapi_Nop)
#define __gllc_DeleteShader                     ((__T_DeleteShader              )__glapi_Nop)
#define __gllc_AttachShader                     ((__T_AttachShader              )__glapi_Nop)
#define __gllc_DetachShader                     ((__T_DetachShader              )__glapi_Nop)
#define __gllc_BindAttribLocation               ((__T_BindAttribLocation        )__glapi_Nop)
#define __gllc_CompileShader                    ((__T_CompileShader             )__glapi_Nop)
#define __gllc_ShaderSource                     ((__T_ShaderSource              )__glapi_Nop)
#define __gllc_LinkProgram                      ((__T_LinkProgram               )__glapi_Nop)
#define __gllc_ValidateProgram                  ((__T_ValidateProgram           )__glapi_Nop)
#define __gllc_Flush                            ((__T_Flush                     )__glapi_Nop)
#define __gllc_Finish                           ((__T_Finish                    )__glapi_Nop)
#define __gllc_IsBuffer                         ((__T_IsBuffer                  )__glapi_Nop)
#define __gllc_IsShader                         ((__T_IsShader                  )__glapi_Nop)
#define __gllc_IsProgram                        ((__T_IsProgram                 )__glapi_Nop)
#define __gllc_IsTexture                        ((__T_IsTexture                 )__glapi_Nop)
#define __gllc_IsList                           ((__T_IsList                    )__glapi_Nop)
#define __gllc_IsEnabled                        ((__T_IsEnabled                 )__glapi_Nop)
#define __gllc_IsQuery                          ((__T_IsQuery                   )__glapi_Nop)
#define __gllc_GetBufferSubData                 ((__T_GetBufferSubData          )__glapi_Nop)
#define __gllc_GetPointerv                      ((__T_GetPointerv               )__glapi_Nop)
#define __gllc_GetVertexAttribPointerv          ((__T_GetVertexAttribPointerv   )__glapi_Nop)
#define __gllc_GetVertexAttribiv                ((__T_GetVertexAttribiv         )__glapi_Nop)
#define __gllc_GetVertexAttribfv                ((__T_GetVertexAttribfv         )__glapi_Nop)
#define __gllc_GetVertexAttribdv                ((__T_GetVertexAttribdv         )__glapi_Nop)
#define __gllc_GetUniformiv                     ((__T_GetUniformiv              )__glapi_Nop)
#define __gllc_GetUniformfv                     ((__T_GetUniformfv              )__glapi_Nop)
#define __gllc_GetUniformLocation               ((__T_GetUniformLocation        )__glapi_Nop)
#define __gllc_GetShaderSource                  ((__T_GetShaderSource           )__glapi_Nop)
#define __gllc_GetShaderInfoLog                 ((__T_GetShaderInfoLog          )__glapi_Nop)
#define __gllc_GetShaderiv                      ((__T_GetShaderiv               )__glapi_Nop)
#define __gllc_GetProgramInfoLog                ((__T_GetProgramInfoLog         )__glapi_Nop)
#define __gllc_GetProgramiv                     ((__T_GetProgramiv              )__glapi_Nop)
#define __gllc_GetAttribLocation                ((__T_GetAttribLocation         )__glapi_Nop)
#define __gllc_GetAttachedShaders               ((__T_GetAttachedShaders        )__glapi_Nop)
#define __gllc_GetActiveUniform                 ((__T_GetActiveUniform          )__glapi_Nop)
#define __gllc_GetActiveAttrib                  ((__T_GetActiveAttrib           )__glapi_Nop)
#define __gllc_GetBufferPointerv                ((__T_GetBufferPointerv         )__glapi_Nop)
#define __gllc_GetBufferParameteriv             ((__T_GetBufferParameteriv      )__glapi_Nop)
#define __gllc_GetQueryObjectuiv                ((__T_GetQueryObjectuiv         )__glapi_Nop)
#define __gllc_GetQueryObjectiv                 ((__T_GetQueryObjectiv          )__glapi_Nop)
#define __gllc_GetQueryiv                       ((__T_GetQueryiv                )__glapi_Nop)
#define __gllc_GetTexLevelParameteriv           ((__T_GetTexLevelParameteriv    )__glapi_Nop)
#define __gllc_GetTexLevelParameterfv           ((__T_GetTexLevelParameterfv    )__glapi_Nop)
#define __gllc_GetTexParameteriv                ((__T_GetTexParameteriv         )__glapi_Nop)
#define __gllc_GetTexParameterfv                ((__T_GetTexParameterfv         )__glapi_Nop)
#define __gllc_GetTexImage                      ((__T_GetTexImage               )__glapi_Nop)
#define __gllc_GetCompressedTexImage            ((__T_GetCompressedTexImage     )__glapi_Nop)
#define __gllc_GetTexGeniv                      ((__T_GetTexGeniv               )__glapi_Nop)
#define __gllc_GetTexGenfv                      ((__T_GetTexGenfv               )__glapi_Nop)
#define __gllc_GetTexGendv                      ((__T_GetTexGendv               )__glapi_Nop)
#define __gllc_GetTexEnviv                      ((__T_GetTexEnviv               )__glapi_Nop)
#define __gllc_GetTexEnvfv                      ((__T_GetTexEnvfv               )__glapi_Nop)
#define __gllc_GetString                        ((__T_GetString                 )__glapi_Nop)
#define __gllc_GetPolygonStipple                ((__T_GetPolygonStipple         )__glapi_Nop)
#define __gllc_GetPixelMapusv                   ((__T_GetPixelMapusv            )__glapi_Nop)
#define __gllc_GetPixelMapuiv                   ((__T_GetPixelMapuiv            )__glapi_Nop)
#define __gllc_GetPixelMapfv                    ((__T_GetPixelMapfv             )__glapi_Nop)
#define __gllc_GetMaterialiv                    ((__T_GetMaterialiv             )__glapi_Nop)
#define __gllc_GetMaterialfv                    ((__T_GetMaterialfv             )__glapi_Nop)
#define __gllc_GetMapiv                         ((__T_GetMapiv                  )__glapi_Nop)
#define __gllc_GetMapfv                         ((__T_GetMapfv                  )__glapi_Nop)
#define __gllc_GetMapdv                         ((__T_GetMapdv                  )__glapi_Nop)
#define __gllc_GetLightiv                       ((__T_GetLightiv                )__glapi_Nop)
#define __gllc_GetLightfv                       ((__T_GetLightfv                )__glapi_Nop)
#define __gllc_GetIntegerv                      ((__T_GetIntegerv               )__glapi_Nop)
#define __gllc_GetFloatv                        ((__T_GetFloatv                 )__glapi_Nop)
#define __gllc_GetError                         ((__T_GetError                  )__glapi_Nop)
#define __gllc_GetDoublev                       ((__T_GetDoublev                )__glapi_Nop)
#define __gllc_GetClipPlane                     ((__T_GetClipPlane              )__glapi_Nop)
#define __gllc_GetBooleanv                      ((__T_GetBooleanv               )__glapi_Nop)
#define __gllc_GetColorTable                    ((__T_GetColorTable             )__glapi_Nop)
#define __gllc_GetMinmaxParameteriv             ((__T_GetMinmaxParameteriv      )__glapi_Nop)
#define __gllc_GetColorTableParameteriv         ((__T_GetColorTableParameteriv  )__glapi_Nop)
#define __gllc_GetColorTableParameterfv         ((__T_GetColorTableParameterfv  )__glapi_Nop)
#define __gllc_GetMinmaxParameterfv             ((__T_GetMinmaxParameterfv      )__glapi_Nop)
#define __gllc_GetMinmax                        ((__T_GetMinmax                 )__glapi_Nop)
#define __gllc_GetHistogramParameteriv          ((__T_GetHistogramParameteriv   )__glapi_Nop)
#define __gllc_GetHistogramParameterfv          ((__T_GetHistogramParameterfv   )__glapi_Nop)
#define __gllc_GetHistogram                     ((__T_GetHistogram              )__glapi_Nop)
#define __gllc_GetSeparableFilter               ((__T_GetSeparableFilter        )__glapi_Nop)
#define __gllc_GetConvolutionParameteriv        ((__T_GetConvolutionParameteriv )__glapi_Nop)
#define __gllc_GetConvolutionParameterfv        ((__T_GetConvolutionParameterfv )__glapi_Nop)
#define __gllc_GetConvolutionFilter             ((__T_GetConvolutionFilter      )__glapi_Nop)
#define __gllc_GenProgramsARB                   ((__T_GenProgramsARB                  )__glapi_Nop)
#define __gllc_DeleteProgramsARB                ((__T_DeleteProgramsARB               )__glapi_Nop)
#define __gllc_ProgramStringARB                 ((__T_ProgramStringARB                )__glapi_Nop)
#define __gllc_IsProgramARB                     ((__T_IsProgramARB                    )__glapi_Nop)
#define __gllc_GetProgramStringARB              ((__T_GetProgramStringARB             )__glapi_Nop)
#define __gllc_GetProgramivARB                  ((__T_GetProgramivARB                 )__glapi_Nop)
#define __gllc_GetProgramLocalParameterfvARB    ((__T_GetProgramLocalParameterfvARB   )__glapi_Nop)
#define __gllc_GetProgramLocalParameterdvARB    ((__T_GetProgramLocalParameterdvARB   )__glapi_Nop)
#define __gllc_GetProgramEnvParameterfvARB      ((__T_GetProgramEnvParameterfvARB     )__glapi_Nop)
#define __gllc_GetProgramEnvParameterdvARB      ((__T_GetProgramEnvParameterdvARB     )__glapi_Nop)
#define __gllc_DeleteObjectARB                  ((__T_DeleteObjectARB          )__glapi_Nop)
#define __gllc_GetHandleARB                     ((__T_GetHandleARB             )__glapi_Nop)
#define __gllc_GetInfoLogARB                    ((__T_GetInfoLogARB            )__glapi_Nop)
#define __gllc_GetObjectParameterfvARB          ((__T_GetObjectParameterfvARB  )__glapi_Nop)
#define __gllc_GetObjectParameterivARB          ((__T_GetObjectParameterivARB  )__glapi_Nop)
#define __gllc_NewObjectBufferATI               ((__T_NewObjectBufferATI           )__glapi_Nop)
#define __gllc_IsObjectBufferATI                ((__T_IsObjectBufferATI            )__glapi_Nop)
#define __gllc_UpdateObjectBufferATI            ((__T_UpdateObjectBufferATI        )__glapi_Nop)
#define __gllc_GetObjectBufferfvATI             ((__T_GetObjectBufferfvATI         )__glapi_Nop)
#define __gllc_GetObjectBufferivATI             ((__T_GetObjectBufferivATI         )__glapi_Nop)
#define __gllc_FreeObjectBufferATI              ((__T_FreeObjectBufferATI          )__glapi_Nop)
#define __gllc_ArrayObjectATI                   ((__T_ArrayObjectATI               )__glapi_Nop)
#define __gllc_GetArrayObjectfvATI              ((__T_GetArrayObjectfvATI          )__glapi_Nop)
#define __gllc_GetArrayObjectivATI              ((__T_GetArrayObjectivATI          )__glapi_Nop)
#define __gllc_VariantArrayObjectATI            ((__T_VariantArrayObjectATI        )__glapi_Nop)
#define __gllc_GetVariantArrayObjectfvATI       ((__T_GetVariantArrayObjectfvATI   )__glapi_Nop)
#define __gllc_GetVariantArrayObjectivATI       ((__T_GetVariantArrayObjectivATI   )__glapi_Nop)
#define __gllc_VertexAttribArrayObjectATI       ((__T_VertexAttribArrayObjectATI       )__glapi_Nop)
#define __gllc_GetVertexAttribArrayObjectfvATI  ((__T_GetVertexAttribArrayObjectfvATI  )__glapi_Nop)
#define __gllc_GetVertexAttribArrayObjectivATI  ((__T_GetVertexAttribArrayObjectivATI  )__glapi_Nop)
#define __gllc_ElementPointerATI                ((__T_ElementPointerATI )__glapi_Nop)
#define __gllc_AddSwapHintRectWIN               ((__T_AddSwapHintRectWIN )__glapi_Nop)

#define __gllc_IsRenderbufferEXT                ((__T_IsRenderbufferEXT              )__glapi_Nop)
#define __gllc_BindRenderbufferEXT              ((__T_BindRenderbufferEXT            )__glapi_Nop)
#define __gllc_DeleteRenderbuffersEXT           ((__T_DeleteRenderbuffersEXT         )__glapi_Nop)
#define __gllc_GenRenderbuffersEXT              ((__T_GenRenderbuffersEXT            )__glapi_Nop)
#define __gllc_RenderbufferStorageEXT           ((__T_RenderbufferStorageEXT         )__glapi_Nop)
#define __gllc_GetRenderbufferParameterivEXT    ((__T_GetRenderbufferParameterivEXT  )__glapi_Nop)
#define __gllc_IsFramebufferEXT                 ((__T_IsFramebufferEXT               )__glapi_Nop)
#define __gllc_BindFramebufferEXT               ((__T_BindFramebufferEXT             )__glapi_Nop)
#define __gllc_DeleteFramebuffersEXT            ((__T_DeleteFramebuffersEXT          )__glapi_Nop)
#define __gllc_GenFramebuffersEXT               ((__T_GenFramebuffersEXT             )__glapi_Nop)
#define __gllc_CheckFramebufferStatusEXT        ((__T_CheckFramebufferStatusEXT      )__glapi_Nop)
#define __gllc_FramebufferTexture1DEXT          ((__T_FramebufferTexture1DEXT        )__glapi_Nop)
#define __gllc_FramebufferTexture2DEXT          ((__T_FramebufferTexture2DEXT        )__glapi_Nop)
#define __gllc_FramebufferTexture3DEXT          ((__T_FramebufferTexture3DEXT        )__glapi_Nop)
#define __gllc_FramebufferRenderbufferEXT       ((__T_FramebufferRenderbufferEXT     )__glapi_Nop)
#define __gllc_GetFramebufferAttachmentParameterivEXT ((__T_GetFramebufferAttachmentParameterivEXT )__glapi_Nop)
#define __gllc_GenerateMipmapEXT                ((__T_GenerateMipmapEXT )__glapi_Nop)
#define __gllc_BlitFramebufferEXT               ((__T_BlitFramebufferEXT )__glapi_Nop)
#define __gllc_RenderbufferStorageMultisampleEXT ((__T_RenderbufferStorageMultisampleEXT )__glapi_Nop)
#define __gllc_UniformBufferEXT                 ((__T_UniformBufferEXT         )__glapi_Nop)
#define __gllc_GetUniformBufferSizeEXT          ((__T_GetUniformBufferSizeEXT  )__glapi_Nop)
#define __gllc_GetUniformOffsetEXT              ((__T_GetUniformOffsetEXT      )__glapi_Nop)
#define __gllc_GetTexParameterIivEXT            ((__T_GetTexParameterIivEXT   )__glapi_Nop)
#define __gllc_GetTexParameterIuivEXT           ((__T_GetTexParameterIuivEXT  )__glapi_Nop)
#define __gllc_VertexAttribI1iEXT               ((__T_VertexAttribI1iEXT       )__glapi_Nop)
#define __gllc_VertexAttribI2iEXT               ((__T_VertexAttribI2iEXT       )__glapi_Nop)
#define __gllc_VertexAttribI3iEXT               ((__T_VertexAttribI3iEXT       )__glapi_Nop)
#define __gllc_VertexAttribI4iEXT               ((__T_VertexAttribI4iEXT       )__glapi_Nop)
#define __gllc_VertexAttribI1uiEXT              ((__T_VertexAttribI1uiEXT      )__glapi_Nop)
#define __gllc_VertexAttribI2uiEXT              ((__T_VertexAttribI2uiEXT      )__glapi_Nop)
#define __gllc_VertexAttribI3uiEXT              ((__T_VertexAttribI3uiEXT      )__glapi_Nop)
#define __gllc_VertexAttribI4uiEXT              ((__T_VertexAttribI4uiEXT      )__glapi_Nop)
#define __gllc_VertexAttribI1ivEXT              ((__T_VertexAttribI1ivEXT      )__glapi_Nop)
#define __gllc_VertexAttribI2ivEXT              ((__T_VertexAttribI2ivEXT      )__glapi_Nop)
#define __gllc_VertexAttribI3ivEXT              ((__T_VertexAttribI3ivEXT      )__glapi_Nop)
#define __gllc_VertexAttribI4ivEXT              ((__T_VertexAttribI4ivEXT      )__glapi_Nop)
#define __gllc_VertexAttribI1uivEXT             ((__T_VertexAttribI1uivEXT     )__glapi_Nop)
#define __gllc_VertexAttribI2uivEXT             ((__T_VertexAttribI2uivEXT     )__glapi_Nop)
#define __gllc_VertexAttribI3uivEXT             ((__T_VertexAttribI3uivEXT     )__glapi_Nop)
#define __gllc_VertexAttribI4uivEXT             ((__T_VertexAttribI4uivEXT     )__glapi_Nop)
#define __gllc_VertexAttribI4bvEXT              ((__T_VertexAttribI4bvEXT      )__glapi_Nop)
#define __gllc_VertexAttribI4svEXT              ((__T_VertexAttribI4svEXT      )__glapi_Nop)
#define __gllc_VertexAttribI4ubvEXT             ((__T_VertexAttribI4ubvEXT     )__glapi_Nop)
#define __gllc_VertexAttribI4usvEXT             ((__T_VertexAttribI4usvEXT     )__glapi_Nop)
#define __gllc_VertexAttribIPointerEXT          ((__T_VertexAttribIPointerEXT  )__glapi_Nop)
#define __gllc_GetVertexAttribIivEXT            ((__T_GetVertexAttribIivEXT    )__glapi_Nop)
#define __gllc_GetVertexAttribIuivEXT           ((__T_GetVertexAttribIuivEXT   )__glapi_Nop)
#define __gllc_Uniform1uiEXT                    ((__T_Uniform1uiEXT            )__glapi_Nop)
#define __gllc_Uniform2uiEXT                    ((__T_Uniform2uiEXT            )__glapi_Nop)
#define __gllc_Uniform3uiEXT                    ((__T_Uniform3uiEXT            )__glapi_Nop)
#define __gllc_Uniform4uiEXT                    ((__T_Uniform4uiEXT            )__glapi_Nop)
#define __gllc_Uniform1uivEXT                   ((__T_Uniform1uivEXT           )__glapi_Nop)
#define __gllc_Uniform2uivEXT                   ((__T_Uniform2uivEXT           )__glapi_Nop)
#define __gllc_Uniform3uivEXT                   ((__T_Uniform3uivEXT           )__glapi_Nop)
#define __gllc_Uniform4uivEXT                   ((__T_Uniform4uivEXT           )__glapi_Nop)
#define __gllc_GetUniformuivEXT                 ((__T_GetUniformuivEXT         )__glapi_Nop)
#define __gllc_BindFragDataLocationEXT          ((__T_BindFragDataLocationEXT  )__glapi_Nop)
#define __gllc_GetFragDataLocationEXT           ((__T_GetFragDataLocationEXT   )__glapi_Nop)
#define __gllc_ProgramParameteriEXT             ((__T_ProgramParameteriEXT        )__glapi_Nop)
#define __gllc_FramebufferTextureEXT            ((__T_FramebufferTextureEXT       )__glapi_Nop)
#define __gllc_FramebufferTextureLayerEXT       ((__T_FramebufferTextureLayerEXT  )__glapi_Nop)
#define __gllc_FramebufferTextureFaceEXT        ((__T_FramebufferTextureFaceEXT   )__glapi_Nop)
#define __gllc_IsEnabledIndexedEXT              ((__T_IsEnabledIndexedEXT    )__glapi_Nop)
#define __gllc_GetIntegerIndexedvEXT            ((__T_GetIntegerIndexedvEXT  )__glapi_Nop)
#define __gllc_GetBooleanIndexedvEXT            ((__T_GetBooleanIndexedvEXT  )__glapi_Nop)
#define __gllc_TexBufferEXT                     ((__T_TexBufferEXT )__glapi_Nop)
#define __gllc_DrawArraysInstancedEXT           ((__T_DrawArraysInstancedEXT    )__glapi_Nop)
#define __gllc_DrawElementsInstancedEXT         ((__T_DrawElementsInstancedEXT  )__glapi_Nop)
#define __gllc_GetQueryObjecti64vEXT            ((__T_GetQueryObjecti64vEXT    )__glapi_Nop)
#define __gllc_GetQueryObjectui64vEXT           ((__T_GetQueryObjectui64vEXT   )__glapi_Nop)
#define __gllc_BindFramebuffer                              ((__T_BindFramebuffer)__glapi_Nop)
#define __gllc_BindRenderbuffer                             ((__T_BindRenderbuffer)__glapi_Nop)
#define __gllc_CheckFramebufferStatus                       ((__T_CheckFramebufferStatus)__glapi_Nop)
#define __gllc_ClearDepthf                                  ((__T_ClearDepthf)__glapi_Nop)
#define __gllc_DeleteFramebuffers                           ((__T_DeleteFramebuffers)__glapi_Nop)
#define __gllc_DeleteRenderbuffers                          ((__T_DeleteRenderbuffers)__glapi_Nop)
#define __gllc_DepthRangef                                  ((__T_DepthRangef)__glapi_Nop)
#define __gllc_FramebufferRenderbuffer                      ((__T_FramebufferRenderbuffer)__glapi_Nop)
#define __gllc_FramebufferTexture2D                         ((__T_FramebufferTexture2D)__glapi_Nop)
#define __gllc_GenerateMipmap                               ((__T_GenerateMipmap)__glapi_Nop)
#define __gllc_GenFramebuffers                              ((__T_GenFramebuffers)__glapi_Nop)
#define __gllc_GenRenderbuffers                             ((__T_GenRenderbuffers)__glapi_Nop)
#define __gllc_GetFramebufferAttachmentParameteriv          ((__T_GetFramebufferAttachmentParameteriv)__glapi_Nop)
#define __gllc_GetRenderbufferParameteriv                   ((__T_GetRenderbufferParameteriv)__glapi_Nop)
#define __gllc_GetShaderPrecisionFormat                     ((__T_GetShaderPrecisionFormat)__glapi_Nop)
#define __gllc_IsFramebuffer                                ((__T_IsFramebuffer)__glapi_Nop)
#define __gllc_IsRenderbuffer                               ((__T_IsRenderbuffer)__glapi_Nop)
#define __gllc_ReleaseShaderCompiler                        ((__T_ReleaseShaderCompiler)__glapi_Nop)
#define __gllc_RenderbufferStorage                          ((__T_RenderbufferStorage)__glapi_Nop)
#define __gllc_ShaderBinary                                 ((__T_ShaderBinary)__glapi_Nop)
#define __gllc_BlitFramebuffer                                            ((__T_BlitFramebuffer                             )__glapi_Nop)
#define __gllc_RenderbufferStorageMultisample                             ((__T_RenderbufferStorageMultisample              )__glapi_Nop)
#define __gllc_FramebufferTextureLayer                                    ((__T_FramebufferTextureLayer                     )__glapi_Nop)
#define __gllc_MapBufferRange                                             ((__T_MapBufferRange                                  )__glapi_Nop)
#define __gllc_FlushMappedBufferRange                                     ((__T_FlushMappedBufferRange                         )__glapi_Nop)
#define __gllc_BindVertexArray                                            ((__T_BindVertexArray                                 )__glapi_Nop)
#define __gllc_DeleteVertexArrays                                         ((__T_DeleteVertexArrays                             )__glapi_Nop)
#define __gllc_GenVertexArrays                                            ((__T_GenVertexArrays                                )__glapi_Nop)
#define __gllc_IsVertexArray                                              ((__T_IsVertexArray                                   )__glapi_Nop)
#define __gllc_GetIntegeri_v                                              ((__T_GetIntegeri_v                                  )__glapi_Nop)
#define __gllc_BeginTransformFeedback                                     ((__T_BeginTransformFeedback                          )__glapi_Nop)
#define __gllc_EndTransformFeedback                                       ((__T_EndTransformFeedback                            )__glapi_Nop)
#define __gllc_BindBufferRange                                            ((__T_BindBufferRange                                  )__glapi_Nop)
#define __gllc_BindBufferBase                                             ((__T_BindBufferBase                                  )__glapi_Nop)
#define __gllc_TransformFeedbackVaryings                                  ((__T_TransformFeedbackVaryings                        )__glapi_Nop)
#define __gllc_GetTransformFeedbackVarying                                ((__T_GetTransformFeedbackVarying                     )__glapi_Nop)
#define __gllc_VertexAttribIPointer                                       ((__T_VertexAttribIPointer                             )__glapi_Nop)
#define __gllc_GetVertexAttribIiv                                         ((__T_GetVertexAttribIiv                             )__glapi_Nop)
#define __gllc_GetVertexAttribIuiv                                        ((__T_GetVertexAttribIuiv                             )__glapi_Nop)
#define __gllc_VertexAttribI4i                                            ((__T_VertexAttribI4i                                )__glapi_Nop)
#define __gllc_VertexAttribI4ui                                           ((__T_VertexAttribI4ui                               )__glapi_Nop)
#define __gllc_VertexAttribI4iv                                           ((__T_VertexAttribI4iv                               )__glapi_Nop)
#define __gllc_VertexAttribI4uiv                                          ((__T_VertexAttribI4uiv                              )__glapi_Nop)
#define __gllc_GetUniformuiv                                              ((__T_GetUniformuiv                                   )__glapi_Nop)
#define __gllc_GetFragDataLocation                                        ((__T_GetFragDataLocation                            )__glapi_Nop)
#define __gllc_BindFragDataLocation                                        ((__T_BindFragDataLocation                            )__glapi_Nop)
#define __gllc_Uniform1ui                                                 ((__T_Uniform1ui                                      )__glapi_Nop)
#define __gllc_Uniform2ui                                                 ((__T_Uniform2ui                                      )__glapi_Nop)
#define __gllc_Uniform3ui                                                 ((__T_Uniform3ui                                      )__glapi_Nop)
#define __gllc_Uniform4ui                                                 ((__T_Uniform4ui                                       )__glapi_Nop)
#define __gllc_Uniform1uiv                                                ((__T_Uniform1uiv                                     )__glapi_Nop)
#define __gllc_Uniform2uiv                                                ((__T_Uniform2uiv                                     )__glapi_Nop)
#define __gllc_Uniform3uiv                                                ((__T_Uniform3uiv                                     )__glapi_Nop)
#define __gllc_Uniform4uiv                                                ((__T_Uniform4uiv                                     )__glapi_Nop)
#define __gllc_ClearBufferiv                                              ((__T_ClearBufferiv                                   )__glapi_Nop)
#define __gllc_ClearBufferuiv                                             ((__T_ClearBufferuiv                                  )__glapi_Nop)
#define __gllc_ClearBufferfv                                              ((__T_ClearBufferfv                                       )__glapi_Nop)
#define __gllc_ClearBufferfi                                              ((__T_ClearBufferfi                                      )__glapi_Nop)
#define __gllc_GetStringi                                                 ((__T_GetStringi                                          )__glapi_Nop)
#define __gllc_CopyBufferSubData                                          ((__T_CopyBufferSubData                                  )__glapi_Nop)
#define __gllc_GetUniformIndices                                          ((__T_GetUniformIndices                                  )__glapi_Nop)
#define __gllc_GetActiveUniformsiv                                        ((__T_GetActiveUniformsiv                                 )__glapi_Nop)
#define __gllc_GetUniformBlockIndex                                       ((__T_GetUniformBlockIndex                               )__glapi_Nop)
#define __gllc_GetActiveUniformBlockiv                                    ((__T_GetActiveUniformBlockiv                             )__glapi_Nop)
#define __gllc_GetActiveUniformBlockName                                  ((__T_GetActiveUniformBlockName                           )__glapi_Nop)
#define __gllc_UniformBlockBinding                                        ((__T_UniformBlockBinding                                  )__glapi_Nop)
#define __gllc_DrawArraysInstanced                                        ((__T_DrawArraysInstanced                                 )__glapi_Nop)
#define __gllc_DrawElementsInstanced                                      ((__T_DrawElementsInstanced                                )__glapi_Nop)
#define __gllc_FenceSync                                                  ((__T_FenceSync                                           )__glapi_Nop)
#define __gllc_IsSync                                                     ((__T_IsSync                                               )__glapi_Nop)
#define __gllc_DeleteSync                                                 ((__T_DeleteSync                                         )__glapi_Nop)
#define __gllc_ClientWaitSync                                             ((__T_ClientWaitSync                                      )__glapi_Nop)
#define __gllc_WaitSync                                                   ((__T_WaitSync                                           )__glapi_Nop)
#define __gllc_GetInteger64v                                              ((__T_GetInteger64v                                      )__glapi_Nop)
#define __gllc_GetSynciv                                                  ((__T_GetSynciv                                          )__glapi_Nop)
#define __gllc_GetInteger64i_v                                            ((__T_GetInteger64i_v                                    )__glapi_Nop)
#define __gllc_GetBufferParameteri64v                                     ((__T_GetBufferParameteri64v                              )__glapi_Nop)
#define __gllc_GenSamplers                                                ((__T_GenSamplers                                        )__glapi_Nop)
#define __gllc_DeleteSamplers                                             ((__T_DeleteSamplers                                      )__glapi_Nop)
#define __gllc_IsSampler                                                  ((__T_IsSampler                                           )__glapi_Nop)
#define __gllc_BindSampler                                                ((__T_BindSampler                                         )__glapi_Nop)
#define __gllc_SamplerParameteri                                          ((__T_SamplerParameteri                                    )__glapi_Nop)
#define __gllc_SamplerParameteriv                                         ((__T_SamplerParameteriv                                  )__glapi_Nop)
#define __gllc_SamplerParameterf                                          ((__T_SamplerParameterf                                   )__glapi_Nop)
#define __gllc_SamplerParameterfv                                         ((__T_SamplerParameterfv                                  )__glapi_Nop)
#define __gllc_GetSamplerParameteriv                                      ((__T_GetSamplerParameteriv                               )__glapi_Nop)
#define __gllc_GetSamplerParameterfv                                      ((__T_GetSamplerParameterfv                               )__glapi_Nop)
#define __gllc_VertexAttribDivisor                                        ((__T_VertexAttribDivisor                                 )__glapi_Nop)
#define __gllc_BindTransformFeedback                                      ((__T_BindTransformFeedback                                   )__glapi_Nop)
#define __gllc_DeleteTransformFeedbacks                                   ((__T_DeleteTransformFeedbacks                               )__glapi_Nop)
#define __gllc_GenTransformFeedbacks                                      ((__T_GenTransformFeedbacks                                   )__glapi_Nop)
#define __gllc_IsTransformFeedback                                        ((__T_IsTransformFeedback                                    )__glapi_Nop)
#define __gllc_PauseTransformFeedback                                     ((__T_PauseTransformFeedback                                 )__glapi_Nop)
#define __gllc_ResumeTransformFeedback                                    ((__T_ResumeTransformFeedback                                 )__glapi_Nop)
#define __gllc_GetProgramBinary                                           ((__T_GetProgramBinary                                       )__glapi_Nop)
#define __gllc_ProgramBinary                                              ((__T_ProgramBinary                                           )__glapi_Nop)
#define __gllc_ProgramParameteri                                          ((__T_ProgramParameteri                                       )__glapi_Nop)
#define __gllc_InvalidateFramebuffer                                      ((__T_InvalidateFramebuffer                                    )__glapi_Nop)
#define __gllc_InvalidateSubFramebuffer                                   ((__T_InvalidateSubFramebuffer                                )__glapi_Nop)
#define __gllc_TexStorage2D                                               ((__T_TexStorage2D                                             )__glapi_Nop)
#define __gllc_TexStorage3D                                               ((__T_TexStorage3D                                            )__glapi_Nop)
#define __gllc_GetInternalformativ                                        ((__T_GetInternalformativ                                      )__glapi_Nop)
#define __gllc_DispatchCompute                                            ((__T_DispatchCompute                                        )__glapi_Nop)
#define __gllc_DispatchComputeIndirect                                    ((__T_DispatchComputeIndirect                                 )__glapi_Nop)
#define __gllc_DrawArraysIndirect                                         ((__T_DrawArraysIndirect                                     )__glapi_Nop)
#define __gllc_DrawElementsIndirect                                       ((__T_DrawElementsIndirect                                   )__glapi_Nop)
#define __gllc_FramebufferParameteri                                      ((__T_FramebufferParameteri                                  )__glapi_Nop)
#define __gllc_GetFramebufferParameteriv                                  ((__T_GetFramebufferParameteriv                              )__glapi_Nop)
#define __gllc_GetProgramInterfaceiv                                      ((__T_GetProgramInterfaceiv                                   )__glapi_Nop)
#define __gllc_GetProgramResourceIndex                                    ((__T_GetProgramResourceIndex                                )__glapi_Nop)
#define __gllc_GetProgramResourceName                                     ((__T_GetProgramResourceName                                  )__glapi_Nop)
#define __gllc_GetProgramResourceiv                                       ((__T_GetProgramResourceiv                                    )__glapi_Nop)
#define __gllc_GetProgramResourceLocation                                 ((__T_GetProgramResourceLocation                              )__glapi_Nop)
#define __gllc_UseProgramStages                                           ((__T_UseProgramStages                                         )__glapi_Nop)
#define __gllc_ActiveShaderProgram                                        ((__T_ActiveShaderProgram                                     )__glapi_Nop)
#define __gllc_CreateShaderProgramv                                       ((__T_CreateShaderProgramv                                    )__glapi_Nop)
#define __gllc_BindProgramPipeline                                        ((__T_BindProgramPipeline                                     )__glapi_Nop)
#define __gllc_DeleteProgramPipelines                                             ((__T_DeleteProgramPipelines                                 )__glapi_Nop)
#define __gllc_GenProgramPipelines                                                ((__T_GenProgramPipelines                                    )__glapi_Nop)
#define __gllc_IsProgramPipeline                                                  ((__T_IsProgramPipeline                                      )__glapi_Nop)
#define __gllc_GetProgramPipelineiv                                               ((__T_GetProgramPipelineiv                                       )__glapi_Nop)
#define __gllc_ProgramUniform1i                                                   ((__T_ProgramUniform1i                                          )__glapi_Nop)
#define __gllc_ProgramUniform2i                                                   ((__T_ProgramUniform2i                                           )__glapi_Nop)
#define __gllc_ProgramUniform3i                                                   ((__T_ProgramUniform3i                                          )__glapi_Nop)
#define __gllc_ProgramUniform4i                                                   ((__T_ProgramUniform4i                                          )__glapi_Nop)
#define __gllc_ProgramUniform1ui                                                  ((__T_ProgramUniform1ui                                          )__glapi_Nop)
#define __gllc_ProgramUniform2ui                                                  ((__T_ProgramUniform2ui                                         )__glapi_Nop)
#define __gllc_ProgramUniform3ui                                                  ((__T_ProgramUniform3ui                                          )__glapi_Nop)
#define __gllc_ProgramUniform4ui                                                  ((__T_ProgramUniform4ui                                          )__glapi_Nop)
#define __gllc_ProgramUniform1f                                                   ((__T_ProgramUniform1f                                            )__glapi_Nop)
#define __gllc_ProgramUniform2f                                                   ((__T_ProgramUniform2f                                           )__glapi_Nop)
#define __gllc_ProgramUniform3f                                                   ((__T_ProgramUniform3f                                            )__glapi_Nop)
#define __gllc_ProgramUniform4f                                                   ((__T_ProgramUniform4f                                           )__glapi_Nop)
#define __gllc_ProgramUniform1iv                                                  ((__T_ProgramUniform1iv                                           )__glapi_Nop)
#define __gllc_ProgramUniform2iv                                                  ((__T_ProgramUniform2iv                                         )__glapi_Nop)
#define __gllc_ProgramUniform3iv                                                  ((__T_ProgramUniform3iv                                          )__glapi_Nop)
#define __gllc_ProgramUniform4iv                                                  ((__T_ProgramUniform4iv                                         )__glapi_Nop)
#define __gllc_ProgramUniform1uiv                                                 ((__T_ProgramUniform1uiv                                        )__glapi_Nop)
#define __gllc_ProgramUniform2uiv                                                 ((__T_ProgramUniform2uiv                                        )__glapi_Nop)
#define __gllc_ProgramUniform3uiv                                                 ((__T_ProgramUniform3uiv                                        )__glapi_Nop)
#define __gllc_ProgramUniform4uiv                                                 ((__T_ProgramUniform4uiv                                         )__glapi_Nop)
#define __gllc_ProgramUniform1fv                                                  ((__T_ProgramUniform1fv                                         )__glapi_Nop)
#define __gllc_ProgramUniform2fv                                                  ((__T_ProgramUniform2fv                                          )__glapi_Nop)
#define __gllc_ProgramUniform3fv                                                  ((__T_ProgramUniform3fv                                          )__glapi_Nop)
#define __gllc_ProgramUniform4fv                                                  ((__T_ProgramUniform4fv                                          )__glapi_Nop)
#define __gllc_ProgramUniformMatrix2fv                                            ((__T_ProgramUniformMatrix2fv                                     )__glapi_Nop)
#define __gllc_ProgramUniformMatrix3fv                                            ((__T_ProgramUniformMatrix3fv                                    )__glapi_Nop)
#define __gllc_ProgramUniformMatrix4fv                                            ((__T_ProgramUniformMatrix4fv                                    )__glapi_Nop)
#define __gllc_ProgramUniformMatrix2x3fv                                          ((__T_ProgramUniformMatrix2x3fv                                  )__glapi_Nop)
#define __gllc_ProgramUniformMatrix3x2fv                                          ((__T_ProgramUniformMatrix3x2fv                                  )__glapi_Nop)
#define __gllc_ProgramUniformMatrix2x4fv                                          ((__T_ProgramUniformMatrix2x4fv                                  )__glapi_Nop)
#define __gllc_ProgramUniformMatrix4x2fv                                          ((__T_ProgramUniformMatrix4x2fv                                  )__glapi_Nop)
#define __gllc_ProgramUniformMatrix3x4fv                                          ((__T_ProgramUniformMatrix3x4fv                                      )__glapi_Nop)
#define __gllc_ProgramUniformMatrix4x3fv                                          ((__T_ProgramUniformMatrix4x3fv                                     )__glapi_Nop)
#define __gllc_ValidateProgramPipeline                                            ((__T_ValidateProgramPipeline                                        )__glapi_Nop)
#define __gllc_GetProgramPipelineInfoLog                                          ((__T_GetProgramPipelineInfoLog                                     )__glapi_Nop)
#define __gllc_BindImageTexture                                                   ((__T_BindImageTexture                                              )__glapi_Nop)
#define __gllc_GetBooleani_v                                                      ((__T_GetBooleani_v                                                  )__glapi_Nop)
#define __gllc_MemoryBarrier                                                      ((__T_MemoryBarrier                                                 )__glapi_Nop)
#define __gllc_MemoryBarrierByRegion                                              ((__T_MemoryBarrierByRegion                                          )__glapi_Nop)
#define __gllc_TexStorage2DMultisample                                            ((__T_TexStorage2DMultisample                                        )__glapi_Nop)
#define __gllc_GetMultisamplefv                                                   ((__T_GetMultisamplefv                                                )__glapi_Nop)
#define __gllc_SampleMaski                                                        ((__T_SampleMaski                                                    )__glapi_Nop)
#define __gllc_BindVertexBuffer                                                   ((__T_BindVertexBuffer                                                )__glapi_Nop)
#define __gllc_VertexAttribFormat                                                 ((__T_VertexAttribFormat                                             )__glapi_Nop)
#define __gllc_VertexAttribIFormat                                                ((__T_VertexAttribIFormat                                             )__glapi_Nop)
#define __gllc_VertexAttribBinding                                                ((__T_VertexAttribBinding                                           )__glapi_Nop)
#define __gllc_VertexBindingDivisor                                               ((__T_VertexBindingDivisor                                           )__glapi_Nop)
#define __gllc_TexStorage3DMultisample                                            ((__T_TexStorage3DMultisample                                       )__glapi_Nop)
#define __gllc_BlendBarrier                                                       ((__T_BlendBarrier                                                  )__glapi_Nop)
#define __gllc_DebugMessageControl                                                ((__T_DebugMessageControl                                           )__glapi_Nop)
#define __gllc_DebugMessageInsert                                                 ((__T_DebugMessageInsert                                            )__glapi_Nop)
#define __gllc_DebugMessageCallback                                               ((__T_DebugMessageCallback                                           )__glapi_Nop)
#define __gllc_GetDebugMessageLog                                                 ((__T_GetDebugMessageLog                                            )__glapi_Nop)
#define __gllc_PushDebugGroup                                                     ((__T_PushDebugGroup                                                 )__glapi_Nop)
#define __gllc_PopDebugGroup                                                      ((__T_PopDebugGroup                                                  )__glapi_Nop)
#define __gllc_ObjectLabel                                                        ((__T_ObjectLabel                                                    )__glapi_Nop)
#define __gllc_GetObjectLabel                                                     ((__T_GetObjectLabel                                                  )__glapi_Nop)
#define __gllc_ObjectPtrLabel                                                     ((__T_ObjectPtrLabel                                                 )__glapi_Nop)
#define __gllc_GetObjectPtrLabel                                                  ((__T_GetObjectPtrLabel                                              )__glapi_Nop)
#define __gllc_GetGraphicsResetStatus                                             ((__T_GetGraphicsResetStatus                                         )__glapi_Nop)
#define __gllc_ReadnPixels                                                        ((__T_ReadnPixels                                                    )__glapi_Nop)
#define __gllc_GetnUniformfv                                                      ((__T_GetnUniformfv                                                  )__glapi_Nop)
#define __gllc_GetnUniformiv                                                      ((__T_GetnUniformiv                                                  )__glapi_Nop)
#define __gllc_GetnUniformuiv                                                     ((__T_GetnUniformuiv                                                     )__glapi_Nop)
#define __gllc_BlendEquationi                                                     ((__T_BlendEquationi                                                    )__glapi_Nop)
#define __gllc_BlendEquationSeparatei                                             ((__T_BlendEquationSeparatei                                             )__glapi_Nop)
#define __gllc_BlendFunci                                                         ((__T_BlendFunci                                                        )__glapi_Nop)
#define __gllc_BlendFuncSeparatei                                                 ((__T_BlendFuncSeparatei                                                )__glapi_Nop)
#define __gllc_ColorMaski                                                         ((__T_ColorMaski                                                         )__glapi_Nop)
#define __gllc_Enablei                                                            ((__T_Enablei                                                           )__glapi_Nop)
#define __gllc_Disablei                                                           ((__T_Disablei                                                           )__glapi_Nop)
#define __gllc_IsEnabledi                                                         ((__T_IsEnabledi                                                         )__glapi_Nop)
#define __gllc_TexParameterIiv                                                    ((__T_TexParameterIiv                                                     )__glapi_Nop)
#define __gllc_TexParameterIuiv                                                   ((__T_TexParameterIuiv                                                   )__glapi_Nop)
#define __gllc_GetTexParameterIiv                                                 ((__T_GetTexParameterIiv                                                  )__glapi_Nop)
#define __gllc_GetTexParameterIuiv                                                ((__T_GetTexParameterIuiv                                                )__glapi_Nop)
#define __gllc_SamplerParameterIiv                                                ((__T_SamplerParameterIiv                                                 )__glapi_Nop)
#define __gllc_SamplerParameterIuiv                                               ((__T_SamplerParameterIuiv                                              )__glapi_Nop)
#define __gllc_GetSamplerParameterIiv                                             ((__T_GetSamplerParameterIiv                                             )__glapi_Nop)
#define __gllc_GetSamplerParameterIuiv                                            ((__T_GetSamplerParameterIuiv                                           )__glapi_Nop)
#define __gllc_TexBuffer                                                          ((__T_TexBuffer                                                         )__glapi_Nop)
#define __gllc_TexBufferRange                                                     ((__T_TexBufferRange                                                    )__glapi_Nop)
#define __gllc_PatchParameteri                                                    ((__T_PatchParameteri                                                   )__glapi_Nop)
#define __gllc_FramebufferTexture                                                 ((__T_FramebufferTexture                                                 )__glapi_Nop)
#define __gllc_MinSampleShading                                                   ((__T_MinSampleShading                                                  )__glapi_Nop)
#define __gllc_CopyImageSubData                                                   ((__T_CopyImageSubData                                                   )__glapi_Nop)
#define __gllc_DrawElementsBaseVertex                                             ((__T_DrawElementsBaseVertex                                             )__glapi_Nop)
#define __gllc_DrawRangeElementsBaseVertex                                        ((__T_DrawRangeElementsBaseVertex                                        )__glapi_Nop)
#define __gllc_DrawElementsInstancedBaseVertex                                    ((__T_DrawElementsInstancedBaseVertex                                     )__glapi_Nop)
#define __gllc_PrimitiveBoundingBox                                               ((__T_PrimitiveBoundingBox                                               )__glapi_Nop)
#define __gllc_EGLImageTargetTexture2DOES                                         ((__T_EGLImageTargetTexture2DOES                                         )__glapi_Nop)
#define __gllc_EGLImageTargetRenderbufferStorageOES                               ((__T_EGLImageTargetRenderbufferStorageOES                               )__glapi_Nop)
#define __gllc_MultiDrawArraysEXT                                ((__T_MultiDrawArraysEXT                     )__glapi_Nop)
#define __gllc_MultiDrawElementsEXT                              ((__T_MultiDrawElementsEXT                   )__glapi_Nop)
#define __gllc_MapBufferOES                                      ((__T_MapBufferOES                           )__glapi_Nop)
#define __gllc_UnmapBufferOES                                    ((__T_UnmapBufferOES                         )__glapi_Nop)
#define __gllc_FramebufferTexture2DMultisampleEXT                ((__T_FramebufferTexture2DMultisampleEXT      )__glapi_Nop)
#define __gllc_TexDirectVIV                                      ((__T_TexDirectVIV                           )__glapi_Nop)
#define __gllc_TexDirectInvalidateVIV                            ((__T_TexDirectInvalidateVIV                  )__glapi_Nop)
#define __gllc_TexDirectVIVMap                                   ((__T_TexDirectVIVMap                         )__glapi_Nop)
#define __gllc_TexDirectTiledMapVIV                              ((__T_TexDirectTiledMapVIV                    )__glapi_Nop)
#define __gllc_MultiDrawArraysIndirectEXT                        ((__T_MultiDrawArraysIndirectEXT               )__glapi_Nop)
#define __gllc_MultiDrawElementsIndirectEXT                      ((__T_MultiDrawElementsIndirectEXT            )__glapi_Nop)
#define __gllc_MultiDrawElementsBaseVertexEXT                    ((__T_MultiDrawElementsBaseVertexEXT          )__glapi_Nop)
#define __gllc_GetBufferPointervOES                              ((__T_GetBufferPointervOES                    )__glapi_Nop)
#define __gllc_DiscardFramebufferEXT                             ((__T_DiscardFramebufferEXT                   )__glapi_Nop)
#define __gllc_GetUniformdv (( __T_GetUniformdv)__glapi_Nop)
#define __gllc_Uniform1d (( __T_Uniform1d)__glapi_Nop)
#define __gllc_Uniform2d (( __T_Uniform2d)__glapi_Nop)
#define __gllc_Uniform3d (( __T_Uniform3d)__glapi_Nop)
#define __gllc_Uniform4d (( __T_Uniform4d)__glapi_Nop)
#define __gllc_Uniform1dv (( __T_Uniform1dv)__glapi_Nop)
#define __gllc_Uniform2dv (( __T_Uniform2dv)__glapi_Nop)
#define __gllc_Uniform3dv (( __T_Uniform3dv)__glapi_Nop)
#define __gllc_Uniform4dv (( __T_Uniform4dv)__glapi_Nop)
#define __gllc_UniformMatrix2dv (( __T_UniformMatrix2dv)__glapi_Nop)
#define __gllc_UniformMatrix3dv (( __T_UniformMatrix3dv)__glapi_Nop)
#define __gllc_UniformMatrix4dv (( __T_UniformMatrix4dv)__glapi_Nop)
#define __gllc_UniformMatrix2x3dv (( __T_UniformMatrix2x3dv)__glapi_Nop)
#define __gllc_UniformMatrix3x2dv (( __T_UniformMatrix3x2dv)__glapi_Nop)
#define __gllc_UniformMatrix2x4dv (( __T_UniformMatrix2x4dv)__glapi_Nop)
#define __gllc_UniformMatrix4x2dv (( __T_UniformMatrix4x2dv)__glapi_Nop)
#define __gllc_UniformMatrix3x4dv (( __T_UniformMatrix3x4dv)__glapi_Nop)
#define __gllc_UniformMatrix4x3dv (( __T_UniformMatrix4x3dv)__glapi_Nop)
#define __gllc_ClampColor (( __T_ClampColor)__glapi_Nop)
#define __gllc_BeginConditionalRender (( __T_BeginConditionalRender)__glapi_Nop)
#define __gllc_EndConditionalRender (( __T_EndConditionalRender)__glapi_Nop)
#define __gllc_VertexAttribI1i (( __T_VertexAttribI1i)__glapi_Nop)
#define __gllc_VertexAttribI2i (( __T_VertexAttribI2i)__glapi_Nop)
#define __gllc_VertexAttribI3i (( __T_VertexAttribI3i)__glapi_Nop)
#define __gllc_VertexAttribI1ui ((__T_VertexAttribI1ui)__glapi_Nop)
#define __gllc_VertexAttribI2ui ((__T_VertexAttribI2ui)__glapi_Nop)
#define __gllc_VertexAttribI3ui ((__T_VertexAttribI3ui)__glapi_Nop)
#define __gllc_VertexAttribI1iv ((__T_VertexAttribI1iv)__glapi_Nop)
#define __gllc_VertexAttribI2iv ((__T_VertexAttribI2iv)__glapi_Nop)
#define __gllc_VertexAttribI3iv ((__T_VertexAttribI3iv)__glapi_Nop)
#define __gllc_VertexAttribI1uiv ((__T_VertexAttribI1uiv)__glapi_Nop)
#define __gllc_VertexAttribI2uiv ((__T_VertexAttribI2uiv)__glapi_Nop)
#define __gllc_VertexAttribI3uiv ((__T_VertexAttribI3uiv)__glapi_Nop)
#define __gllc_VertexAttribI4bv ((__T_VertexAttribI4bv)__glapi_Nop)
#define __gllc_VertexAttribI4sv ((__T_VertexAttribI4sv)__glapi_Nop)
#define __gllc_VertexAttribI4ubv ((__T_VertexAttribI4ubv)__glapi_Nop)
#define __gllc_VertexAttribI4usv ((__T_VertexAttribI4usv)__glapi_Nop)
#define __gllc_FramebufferTexture1D ((__T_FramebufferTexture1D)__glapi_Nop)
#define __gllc_FramebufferTexture3D ((__T_FramebufferTexture3D)__glapi_Nop)
#define __gllc_PrimitiveRestartIndex ((__T_PrimitiveRestartIndex)__glapi_Nop)
#define __gllc_GetActiveUniformName ((__T_GetActiveUniformName)__glapi_Nop)
#define __gllc_MultiDrawArrays ((__T_MultiDrawArrays)__glapi_Nop)
#define __gllc_MultiDrawElements ((__T_MultiDrawElements)__glapi_Nop)
const GLubyte * (*__glListExecFuncTable[])(__GLcontext *, const GLubyte *) = {
    __GL_LISTEXEC_ENTRIES(__glle, )
};

#endif

__GLesDispatchTable __glesApiFuncDispatchTable = {
    __GLES_API_ENTRIES(__gles)
};

__GLesDispatchTable __glImmediateFuncTable = {
    __GLES_API_ENTRIES(__glim)
};

__GLesDispatchTable __glListCompileFuncTable = {
    __GLES_API_ENTRIES(__gllc)
};

__GLesDispatchTable __glNopFuncTable = {
    __GLES_API_ENTRIES(__gl4nop)
};

/* temporily for passing WGL */
GLvoid __glCoreNopDispatch(GLvoid)
{

}
GLvoid __glRestoreDispatch(__GLcontext *gc)
{

}
GLvoid __glSaveDispatch(__GLcontext *gc)
{


}
GLvoid __glUnSupportModeExit(__GLcontext *gc)
{

}

GLvoid __glOverWriteListCompileTable(__GLcontext *gc)
{
    __GLesDispatchTable *__gllc_Table = &__glListCompileFuncTable;

    __gllc_Table->NewList = __glim_NewList;
    __gllc_Table->EndList = __glim_EndList;
    __gllc_Table->GenLists = __glim_GenLists;
    __gllc_Table->DeleteLists = __glim_DeleteLists;
    __gllc_Table->CallList = __glim_CallList;
    __gllc_Table->CallLists = __glim_CallLists;
    __gllc_Table->FeedbackBuffer = __glim_FeedbackBuffer;
    __gllc_Table->SelectBuffer = __glim_SelectBuffer;
    __gllc_Table->RenderMode = __glim_RenderMode;
    __gllc_Table->ClientActiveTexture = __glim_ClientActiveTexture;
    __gllc_Table->ColorPointer = __glim_ColorPointer;
    __gllc_Table->EdgeFlagPointer = __glim_EdgeFlagPointer;
    __gllc_Table->FogCoordPointer = __glim_FogCoordPointer;
    __gllc_Table->IndexPointer = __glim_IndexPointer;
    __gllc_Table->InterleavedArrays = __glim_InterleavedArrays;
    __gllc_Table->NormalPointer = __glim_NormalPointer;
    __gllc_Table->SecondaryColorPointer = __glim_SecondaryColorPointer;
    __gllc_Table->TexCoordPointer = __glim_TexCoordPointer;
    __gllc_Table->VertexAttribPointer = __glim_VertexAttribPointer;
    __gllc_Table->VertexPointer = __glim_VertexPointer;
    __gllc_Table->EnableClientState = __glim_EnableClientState;
    __gllc_Table->DisableClientState = __glim_DisableClientState;
    __gllc_Table->EnableVertexAttribArray = __glim_EnableVertexAttribArray;
    __gllc_Table->DisableVertexAttribArray = __glim_DisableVertexAttribArray;
    __gllc_Table->PushClientAttrib = __glim_PushClientAttrib;
    __gllc_Table->PopClientAttrib = __glim_PopClientAttrib;
    __gllc_Table->PixelStorei = __glim_PixelStorei;
    __gllc_Table->PixelStoref = __glim_PixelStoref;
    __gllc_Table->ReadPixels = __glim_ReadPixels;
    __gllc_Table->GenTextures = __glim_GenTextures;
    __gllc_Table->DeleteTextures = __glim_DeleteTextures;
    __gllc_Table->AreTexturesResident = __glim_AreTexturesResident;
    __gllc_Table->GenQueries = __glim_GenQueries;
    __gllc_Table->DeleteQueries = __glim_DeleteQueries;
    __gllc_Table->GenBuffers = __glim_GenBuffers;
    __gllc_Table->DeleteBuffers = __glim_DeleteBuffers;
    __gllc_Table->BindBuffer = __glim_BindBuffer;
    __gllc_Table->BufferData = __glim_BufferData;
    __gllc_Table->BufferSubData = __glim_BufferSubData;
    /* to do if neccessary */
    __gllc_Table->MapBuffer = __glim_MapBuffer;
    __gllc_Table->UnmapBuffer = __glim_UnmapBuffer;
    __gllc_Table->CreateProgram = __glim_CreateProgram;
    __gllc_Table->CreateShader = __glim_CreateShader;
    __gllc_Table->VertexPointer = __glim_VertexPointer;
    __gllc_Table->DeleteProgram = __glim_DeleteProgram;
    __gllc_Table->DeleteShader = __glim_DeleteShader;
    __gllc_Table->AttachShader = __glim_AttachShader;
    __gllc_Table->DetachShader = __glim_DetachShader;
    __gllc_Table->BindAttribLocation = __glim_BindAttribLocation;
    __gllc_Table->CompileShader = __glim_CompileShader;
    __gllc_Table->ShaderSource = __glim_ShaderSource;
    __gllc_Table->LinkProgram = __glim_LinkProgram;
    __gllc_Table->ValidateProgram = __glim_ValidateProgram;
    __gllc_Table->Flush = __glim_Flush;
    __gllc_Table->Finish = __glim_Finish;
    __gllc_Table->IsBuffer = __glim_IsBuffer;
    __gllc_Table->IsShader = __glim_IsShader;
    __gllc_Table->IsProgram = __glim_IsProgram;
    __gllc_Table->IsTexture = __glim_IsTexture;
    __gllc_Table->IsList = __glim_IsList;
    __gllc_Table->IsEnabled = __glim_IsEnabled;
    __gllc_Table->IsQuery = __glim_IsQuery;
    /* To do */
    /*
    __gllc_Table->GetBufferSubData = __glim_GetBufferSubData;
    */
    __gllc_Table->GetPointerv = __glim_GetPointerv;
    __gllc_Table->GetVertexAttribPointerv = __glim_GetVertexAttribPointerv;
    __gllc_Table->GetVertexAttribiv = __glim_GetVertexAttribiv;
    __gllc_Table->GetVertexAttribfv = __glim_GetVertexAttribfv;
    /* To do */

    __gllc_Table->GetVertexAttribdv = __glim_GetVertexAttribdv;

    __gllc_Table->GetUniformiv = __glim_GetUniformiv;
    __gllc_Table->GetUniformfv = __glim_GetUniformfv;
    __gllc_Table->GetUniformLocation = __glim_GetUniformLocation;
    __gllc_Table->GetShaderSource = __glim_GetShaderSource;
    __gllc_Table->GetShaderInfoLog = __glim_GetShaderInfoLog;
    __gllc_Table->GetShaderiv = __glim_GetShaderiv;
    __gllc_Table->GetProgramInfoLog = __glim_GetProgramInfoLog;
    __gllc_Table->GetProgramiv = __glim_GetProgramiv;
    __gllc_Table->GetAttribLocation = __glim_GetAttribLocation;
    __gllc_Table->GetAttachedShaders = __glim_GetAttachedShaders;
    __gllc_Table->GetAttachedShaders = __glim_GetAttachedShaders;
    __gllc_Table->GetActiveUniform = __glim_GetActiveUniform;
    __gllc_Table->GetActiveUniform = __glim_GetActiveUniform;
    __gllc_Table->GetActiveUniform = __glim_GetActiveUniform;
    __gllc_Table->GetActiveAttrib = __glim_GetActiveAttrib;
    __gllc_Table->GetBufferPointerv = __glim_GetBufferPointerv;
    __gllc_Table->GetBufferParameteriv = __glim_GetBufferParameteriv;
    __gllc_Table->GetQueryObjectuiv = __glim_GetQueryObjectuiv;
    /* To do */
    /*
    __gllc_Table->GetQueryObjectiv = __glim_GetQueryObjectiv;
    */
    __gllc_Table->GetQueryiv = __glim_GetQueryiv;
    __gllc_Table->GetTexLevelParameteriv = __glim_GetTexLevelParameteriv;
    __gllc_Table->GetTexLevelParameterfv = __glim_GetTexLevelParameterfv;
    __gllc_Table->GetTexParameteriv = __glim_GetTexParameteriv;
    __gllc_Table->GetTexParameterfv = __glim_GetTexParameterfv;
    __gllc_Table->GetTexImage = __glim_GetTexImage;
    __gllc_Table->GetCompressedTexImage = __glim_GetCompressedTexImage;
    __gllc_Table->GetTexGeniv = __glim_GetTexGeniv;
    __gllc_Table->GetTexGenfv = __glim_GetTexGenfv;
    __gllc_Table->GetTexGendv = __glim_GetTexGendv;
    __gllc_Table->GetTexEnviv = __glim_GetTexEnviv;
    __gllc_Table->GetTexEnvfv = __glim_GetTexEnvfv;
    __gllc_Table->GetString = __glim_GetString;
    __gllc_Table->GetPolygonStipple = __glim_GetPolygonStipple;
    __gllc_Table->GetPixelMapusv = __glim_GetPixelMapusv;
    __gllc_Table->GetPixelMapuiv = __glim_GetPixelMapuiv;
    __gllc_Table->GetPixelMapfv = __glim_GetPixelMapfv;
    __gllc_Table->GetMaterialiv = __glim_GetMaterialiv;
    __gllc_Table->GetMaterialfv = __glim_GetMaterialfv;
    __gllc_Table->GetMapiv = __glim_GetMapiv;
    __gllc_Table->GetMapfv = __glim_GetMapfv;
    __gllc_Table->GetMapdv = __glim_GetMapdv;
    __gllc_Table->GetLightiv = __glim_GetLightiv;
    __gllc_Table->GetLightfv = __glim_GetLightfv;
    __gllc_Table->GetIntegerv = __glim_GetIntegerv;
    __gllc_Table->GetFloatv = __glim_GetFloatv;
    __gllc_Table->GetError = __glim_GetError;
    __gllc_Table->GetDoublev = __glim_GetDoublev;
    __gllc_Table->GetClipPlane = __glim_GetClipPlane;
    __gllc_Table->GetBooleanv = __glim_GetBooleanv;
    __gllc_Table->GetColorTable = __glim_GetColorTable;
    __gllc_Table->GetMinmaxParameteriv = __glim_GetMinmaxParameteriv;
    __gllc_Table->GetColorTableParameteriv = __glim_GetColorTableParameteriv;
    __gllc_Table->GetColorTableParameterfv = __glim_GetColorTableParameterfv;
    __gllc_Table->GetMinmaxParameterfv = __glim_GetMinmaxParameterfv;
    __gllc_Table->GetMinmax = __glim_GetMinmax;
    __gllc_Table->GetHistogramParameteriv = __glim_GetHistogramParameteriv;
    __gllc_Table->GetHistogramParameterfv = __glim_GetHistogramParameterfv;
    __gllc_Table->GetHistogram = __glim_GetHistogram;
    __gllc_Table->GetSeparableFilter = __glim_GetSeparableFilter;
    __gllc_Table->GetConvolutionParameteriv = __glim_GetConvolutionParameteriv;
    __gllc_Table->GetConvolutionParameterfv = __glim_GetConvolutionParameterfv;
    __gllc_Table->GetConvolutionFilter = __glim_GetConvolutionFilter;

#if GL_ARB_vertex_program
    /* To do */
    /*
    __gllc_Table->GenProgramsARB = __glim_GenProgramsARB;
    __gllc_Table->DeleteProgramsARB = __glim_DeleteProgramsARB;
    __gllc_Table->ProgramStringARB = __glim_ProgramStringARB;
    __gllc_Table->IsProgramARB = __glim_IsProgramARB;
    __gllc_Table->GetProgramStringARB = __glim_GetProgramStringARB;
    __gllc_Table->GetProgramivARB = __glim_GetProgramivARB;
    __gllc_Table->GetProgramLocalParameterfvARB = __glim_GetProgramLocalParameterfvARB;
    __gllc_Table->GetProgramLocalParameterdvARB = __glim_GetProgramLocalParameterdvARB;
    __gllc_Table->GetProgramEnvParameterfvARB = __glim_GetProgramEnvParameterfvARB;
    __gllc_Table->GetProgramEnvParameterdvARB = __glim_GetProgramEnvParameterdvARB;
    */
#endif

#if GL_ARB_shader_objects
    /* To do */
    /*
    __gllc_Table->DeleteObjectARB = __glim_DeleteObjectARB;
    __gllc_Table->GetHandleARB = __glim_GetHandleARB;
    __gllc_Table->GetInfoLogARB = __glim_GetInfoLogARB;
    __gllc_Table->GetObjectParameterfvARB = __glim_GetObjectParameterfvARB;
    __gllc_Table->GetObjectParameterivARB = __glim_GetObjectParameterivARB;
    */
#endif

#if GL_ATI_vertex_array_object
    /* To do */
    /*
    __gllc_Table->NewObjectBufferATI  = __glim_NewObjectBufferATI;
    __gllc_Table->IsObjectBufferATI   = __glim_IsObjectBufferATI;
    __gllc_Table->UpdateObjectBufferATI = __glim_UpdateObjectBufferATI;
    __gllc_Table->GetObjectBufferfvATI = __glim_GetObjectBufferfvATI;
    __gllc_Table->GetObjectBufferivATI = __glim_GetObjectBufferivATI;
    __gllc_Table->FreeObjectBufferATI =  __glim_FreeObjectBufferATI;
    __gllc_Table->ArrayObjectATI = __glim_ArrayObjectATI;
    __gllc_Table->GetArrayObjectfvATI =  __glim_GetArrayObjectfvATI;
    __gllc_Table->GetArrayObjectivATI =  __glim_GetArrayObjectivATI;
    __gllc_Table->VariantArrayObjectATI =  __glim_VariantArrayObjectATI;
    __gllc_Table->GetVariantArrayObjectfvATI =  __glim_GetVariantArrayObjectfvATI;
    __gllc_Table->GetVariantArrayObjectivATI =  __glim_GetVariantArrayObjectivATI;
    */
#endif

#if GL_ATI_element_array
    /* To do */
    /*
    __gllc_Table->ElementPointerATI = __glim_ElementPointerATI;
    */
#endif
    /* To do */
    /*
    __gllc_Table->AddSwapHintRectWIN = __glim_AddSwapHintRectWIN;
    */

 #if GL_EXT_framebuffer_object
    /* To do */
    /*
    __gllc_Table->IsRenderbufferEXT = __glim_IsRenderbufferEXT;
    __gllc_Table->BindRenderbufferEXT = __glim_BindRenderbufferEXT;
    __gllc_Table->DeleteRenderbuffersEXT = __glim_DeleteRenderbuffersEXT;
    __gllc_Table->GenRenderbuffersEXT = __glim_GenRenderbuffersEXT;
    __gllc_Table->RenderbufferStorageEXT = __glim_RenderbufferStorageEXT;
    __gllc_Table->GetRenderbufferParameterivEXT = __glim_GetRenderbufferParameterivEXT;
    __gllc_Table->IsFramebufferEXT = __glim_IsFramebufferEXT;
    __gllc_Table->BindFramebufferEXT = __glim_BindFramebufferEXT;
    __gllc_Table->DeleteFramebuffersEXT = __glim_DeleteFramebuffersEXT;
    __gllc_Table->GenFramebuffersEXT = __glim_GenFramebuffersEXT;
    __gllc_Table->CheckFramebufferStatusEXT = __glim_CheckFramebufferStatusEXT;
    __gllc_Table->FramebufferTexture1DEXT = __glim_FramebufferTexture1DEXT;
    __gllc_Table->FramebufferTexture2DEXT = __glim_FramebufferTexture2DEXT;
    __gllc_Table->FramebufferTexture3DEXT = __glim_FramebufferTexture3DEXT;
    __gllc_Table->FramebufferRenderbufferEXT = __glim_FramebufferRenderbufferEXT;
    __gllc_Table->GetFramebufferAttachmentParameterivEXT = __glim_GetFramebufferAttachmentParameterivEXT;
    __gllc_Table->GenerateMipmapEXT = __glim_GenerateMipmapEXT;
    __gllc_Table->BlitFramebufferEXT = __glim_BlitFramebufferEXT;
    */
    __gllc_Table->RenderbufferStorageMultisampleEXT = __glim_RenderbufferStorageMultisampleEXT;
#endif

#if GL_EXT_bindable_uniform
    /* To do */
    /*
    __gllc_Table->UniformBufferEXT = __glim_UniformBufferEXT;
    __gllc_Table->GetUniformBufferSizeEXT = __glim_GetUniformBufferSizeEXT;
    __gllc_Table->GetUniformOffsetEXT = __glim_GetUniformOffsetEXT;
    */
#endif

#if GL_EXT_texture_integer
    /* To do */
    /*
    __gllc_Table->GetTexParameterIivEXT = __glim_GetTexParameterIivEXT;
    __gllc_Table->GetTexParameterIuivEXT = __glim_GetTexParameterIuivEXT;
    */
#endif

#if GL_EXT_gpu_shader4
    /* To do */
    /*
    __gllc_Table->VertexAttribI1iEXT = __glim_VertexAttribI1iEXT;
    __gllc_Table->VertexAttribI2iEXT = __glim_VertexAttribI2iEXT;
    __gllc_Table->VertexAttribI3iEXT = __glim_VertexAttribI3iEXT;
    __gllc_Table->VertexAttribI4iEXT = __glim_VertexAttribI4iEXT;
    __gllc_Table->VertexAttribI1uiEXT = __glim_VertexAttribI1uiEXT;
    __gllc_Table->VertexAttribI2uiEXT = __glim_VertexAttribI2uiEXT;
    __gllc_Table->VertexAttribI3uiEXT = __glim_VertexAttribI3uiEXT;
    __gllc_Table->VertexAttribI4uiEXT = __glim_VertexAttribI4uiEXT;
    __gllc_Table->VertexAttribI1ivEXT = __glim_VertexAttribI1ivEXT;
    __gllc_Table->VertexAttribI2ivEXT = __glim_VertexAttribI2ivEXT;
    __gllc_Table->VertexAttribI3ivEXT = __glim_VertexAttribI3ivEXT;
    __gllc_Table->VertexAttribI4ivEXT = __glim_VertexAttribI4ivEXT;
    __gllc_Table->VertexAttribI1uivEXT = __glim_VertexAttribI1uivEXT;
    __gllc_Table->VertexAttribI2uivEXT = __glim_VertexAttribI2uivEXT;
    __gllc_Table->VertexAttribI3uivEXT = __glim_VertexAttribI3uivEXT;
    __gllc_Table->VertexAttribI4uivEXT = __glim_VertexAttribI4uivEXT;
    __gllc_Table->VertexAttribI4bvEXT = __glim_VertexAttribI4bvEXT;
    __gllc_Table->VertexAttribI4svEXT = __glim_VertexAttribI4svEXT;
    __gllc_Table->VertexAttribI4ubvEXT = __glim_VertexAttribI4ubvEXT;
    __gllc_Table->VertexAttribI4usvEXT = __glim_VertexAttribI4usvEXT;
    __gllc_Table->VertexAttribIPointerEXT = __glim_VertexAttribIPointerEXT;
    */
    /* To do */
    /*
    __gllc_Table->GetVertexAttribIivEXT = __glim_GetVertexAttribIivEXT;
    __gllc_Table->GetVertexAttribIuivEXT = __glim_GetVertexAttribIuivEXT;
    __gllc_Table->Uniform1uiEXT = __glim_Uniform1uiEXT;
    __gllc_Table->Uniform2uiEXT = __glim_Uniform2uiEXT;
    __gllc_Table->Uniform3uiEXT = __glim_Uniform3uiEXT;
    __gllc_Table->Uniform4uiEXT = __glim_Uniform4uiEXT;
    __gllc_Table->Uniform1uivEXT = __glim_Uniform1uivEXT;
    __gllc_Table->Uniform2uivEXT = __glim_Uniform2uivEXT;
    __gllc_Table->Uniform3uivEXT = __glim_Uniform3uivEXT;
    __gllc_Table->Uniform4uivEXT = __glim_Uniform4uivEXT;
    __gllc_Table->GetUniformuivEXT = __glim_GetUniformuivEXT;
    __gllc_Table->BindFragDataLocationEXT = __glim_BindFragDataLocationEXT;
    __gllc_Table->GetFragDataLocationEXT = __glim_GetFragDataLocationEXT;
    */
#endif

#if GL_EXT_geometry_shader4
    /* To do */
    /*
    __gllc_Table->ProgramParameteriEXT = __glim_ProgramParameteriEXT;
    __gllc_Table->FramebufferTextureEXT = __glim_FramebufferTextureEXT;
    __gllc_Table->FramebufferTextureLayerEXT = __glim_FramebufferTextureLayerEXT;
    __gllc_Table->FramebufferTextureFaceEXT = __glim_FramebufferTextureFaceEXT;
    */
#endif

#if GL_EXT_texture_buffer_object
    /* To do */
    /*
    __gllc_Table->TexBufferEXT = __glim_TexBufferEXT;
    */
#endif

#if GL_EXT_draw_instanced
    /* To do */
    /*
    __gllc_Table->DrawArraysInstancedEXT = __glim_DrawArraysInstancedEXT;
    __gllc_Table->DrawElementsInstancedEXT = __glim_DrawElementsInstancedEXT;

#endif

#if GL_EXT_timer_query
    /* To do */
    /*
    __gllc_Table->GetQueryObjecti64vEXT = __glim_GetQueryObjecti64vEXT;
    __gllc_Table->GetQueryObjectui64vEXT = __glim_GetQueryObjectui64vEXT;
    */
#endif
    memcpy((char *)(&gc->listCompileDispatchTable), (char *)__gllc_Table,sizeof(__GLesDispatchTable));
}

GLvoid __gIMDispatch(__GLcontext *gc)
{
    memcpy((char *)(&gc->apiDispatchTable), (char *)(&gc->apiSaveDispatchTable),sizeof(gc->apiDispatchTable));
    memcpy((char *)(&gc->immediateDispatchTable), (char *)(&__glImmediateFuncTable),sizeof(__GLesDispatchTable));
    gc->currentImmediateTable = &gc->immediateDispatchTable;
}

GLvoid __gDLDispatch(__GLcontext *gc)
{
    memcpy((char *)(&gc->apiSaveDispatchTable), (char *)(&gc->apiDispatchTable), sizeof(gc->apiDispatchTable));
    __glOverWriteListCompileTable(gc);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, IsList);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, DeleteLists);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GenLists);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, NewList);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, EndList);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, CallList);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, CallLists);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, ListBase);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Begin);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Bitmap);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color3b);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color3d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color3f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color3i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color3s);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color3ub);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color3ui);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color3us);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color3bv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color3dv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color3fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color3iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color3sv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color3ubv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color3uiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color3usv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color4b);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color4d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color4f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color4i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color4s);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color4ub);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color4ui);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color4us);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color4bv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color4dv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color4fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color4iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color4sv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color4ubv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color4uiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Color4usv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, EdgeFlag);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, EdgeFlagv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, End);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Indexd);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Indexf);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Indexi);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Indexs);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Indexdv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Indexfv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Indexiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Indexsv);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Normal3b);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Normal3d);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Normal3f);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Normal3i);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Normal3s);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Normal3bv);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Normal3dv);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Normal3fv);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Normal3iv);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Normal3sv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, RasterPos2d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, RasterPos2f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, RasterPos2i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, RasterPos2s);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, RasterPos3d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, RasterPos3f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, RasterPos3i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, RasterPos3s);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, RasterPos4d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, RasterPos4f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, RasterPos4i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, RasterPos4s);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, RasterPos2dv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, RasterPos2fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, RasterPos2iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, RasterPos2sv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, RasterPos3dv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, RasterPos3fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, RasterPos3iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, RasterPos3sv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, RasterPos4dv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, RasterPos4fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, RasterPos4iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, RasterPos4sv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Rectd);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Rectf);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Recti);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Rects);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Rectdv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Rectfv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Rectiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Rectsv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord1d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord1f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord1i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord1s);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord2d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord2f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord2i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord2s);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord3d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord3f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord3i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord3s);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord4d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord4f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord4i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord4s);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord1dv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord1fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord1iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord1sv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord2dv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord2fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord2iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord2sv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord3dv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord3fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord3iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord3sv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord4dv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord4fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord4iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoord4sv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Vertex2d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Vertex2f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Vertex2i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Vertex2s);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Vertex3d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Vertex3f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Vertex3i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Vertex3s);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Vertex4d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Vertex4f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Vertex4i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Vertex4s);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Vertex2dv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Vertex2fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Vertex2iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Vertex2sv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Vertex3dv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Vertex3fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Vertex3iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Vertex3sv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Vertex4dv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Vertex4fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Vertex4iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Vertex4sv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, ClipPlane);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, ColorMaterial);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Fogf);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Fogi);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Fogfv);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Fogiv);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Lightf);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Lighti);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Lightfv);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Lightiv);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, LightModelf);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, LightModeli);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, LightModelfv);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, LightModeliv);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, LineStipple);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Materialf);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Materiali);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Materialfv);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Materialiv);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, PointSize);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, PolygonMode);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, PolygonStipple);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, ShadeModel);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexImage1D);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexEnvf);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexEnvi);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexEnvfv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexEnviv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexGend);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexGenf);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexGeni);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexGendv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexGenfv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexGeniv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, FeedbackBuffer);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, SelectBuffer);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, RenderMode);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, InitNames);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, LoadName);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, PushName);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, PopName);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, PassThrough);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, DrawBuffer);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, ClearAccum);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, ClearIndex);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, ClearDepth);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, IndexMask);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Accum);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, PushAttrib);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, PopAttrib);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Map1d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Map1f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Map2d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Map2f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MapGrid1d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MapGrid1f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MapGrid2d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MapGrid2f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, EvalCoord1d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, EvalCoord1f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, EvalCoord1dv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, EvalCoord1fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, EvalCoord2d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, EvalCoord2f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, EvalCoord2dv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, EvalCoord2fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, EvalPoint1);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, EvalPoint2);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, EvalMesh1);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, EvalMesh2);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, AlphaFunc);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, LogicOp);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, PixelZoom);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, PixelTransferf);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, PixelTransferi);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, PixelStoref);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, PixelMapfv);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, PixelMapuiv);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, PixelMapusv);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetClipPlane);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetDoublev);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetLightfv);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetLightiv);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetMapdv);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetMapfv);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetMapiv);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetMaterialfv);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetMaterialiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetPixelMapfv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetPixelMapuiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetPixelMapusv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetPolygonStipple);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetTexEnvfv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetTexEnviv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetTexGendv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetTexGenfv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetTexGeniv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetTexImage);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, DepthRange);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Frustum);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, LoadIdentity);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, LoadMatrixd);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, LoadMatrixf);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MatrixMode);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultMatrixd);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultMatrixf);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Ortho);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, PushMatrix);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, PopMatrix);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Rotated);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Rotatef);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Scaled);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Scalef);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Translated);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Translatef);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, ArrayElement);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, ColorPointer);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, EnableClientState);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, DisableClientState);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, EdgeFlagPointer);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, IndexPointer);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, InterleavedArrays);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, NormalPointer);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, VertexPointer);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexCoordPointer);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, AreTexturesResident);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, CopyTexImage1D);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, CopyTexSubImage1D);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, PrioritizeTextures);
   SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, TexSubImage1D);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, PushClientAttrib);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, PopClientAttrib);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, ColorTable);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, ColorTableParameteriv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, ColorTableParameterfv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, CopyColorTable);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetColorTable);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetColorTableParameterfv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetColorTableParameteriv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, ColorSubTable);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, CopyColorSubTable);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, ConvolutionFilter1D);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, ConvolutionFilter2D);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, ConvolutionParameterf);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, ConvolutionParameterfv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, ConvolutionParameteri);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, ConvolutionParameteriv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, CopyConvolutionFilter1D);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, CopyConvolutionFilter2D);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetConvolutionFilter);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetConvolutionParameterfv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetConvolutionParameteriv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, SeparableFilter2D);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetSeparableFilter);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Histogram);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, ResetHistogram);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetHistogram);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetHistogramParameterfv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetHistogramParameteriv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Minmax);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, ResetMinmax);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetMinmax);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetMinmaxParameterfv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetMinmaxParameteriv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, ClientActiveTexture);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord1d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord1dv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord1f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord1fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord1i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord1iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord1s);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord1sv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord2d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord2dv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord2f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord2fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord2i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord2iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord2s);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord2sv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord3d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord3dv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord3f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord3fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord3i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord3iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord3s);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord3sv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord4d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord4dv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord4f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord4fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord4i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord4iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord4s);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultiTexCoord4sv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, LoadTransposeMatrixd);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, LoadTransposeMatrixf);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultTransposeMatrixd);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MultTransposeMatrixf);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, CompressedTexImage1D);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, CompressedTexSubImage1D);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, GetCompressedTexImage);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, FogCoordf);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, FogCoordfv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, FogCoordd);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, FogCoorddv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, FogCoordPointer);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, PointParameterf);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, PointParameterfv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, PointParameteri);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, PointParameteriv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, SecondaryColor3b);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, SecondaryColor3bv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, SecondaryColor3d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, SecondaryColor3dv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, SecondaryColor3f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, SecondaryColor3fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, SecondaryColor3i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, SecondaryColor3iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, SecondaryColor3s);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, SecondaryColor3sv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, SecondaryColor3ub);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, SecondaryColor3ubv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, SecondaryColor3ui);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, SecondaryColor3uiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, SecondaryColor3us);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, SecondaryColor3usv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, SecondaryColorPointer);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, WindowPos2d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, WindowPos2dv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, WindowPos2f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, WindowPos2fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, WindowPos2i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, WindowPos2iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, WindowPos2s);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, WindowPos2sv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, WindowPos3d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, WindowPos3dv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, WindowPos3f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, WindowPos3fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, WindowPos3i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, WindowPos3iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, WindowPos3s);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, WindowPos3sv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib1d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib1dv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib1s);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib1sv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib2d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib2dv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib2s);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib2sv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib3d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib3dv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib3s);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib3sv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib4Nbv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib4Niv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib4Nsv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib4Nub);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib4Nubv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib4Nuiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib4Nusv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib4bv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib4d);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib4dv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib4iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib4s);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib4sv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib4ubv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib4uiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable ,VertexAttrib4usv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, MapBuffer);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, DrawPixels);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, CopyPixels);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Indexub);
    SetDLTABLE((&(gc->listCompileDispatchTable)),gc->currentImmediateTable, Indexubv);

/* OpenGL ES 2.0 */

    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ActiveTexture);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,AttachShader);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BindAttribLocation);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BindBuffer);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BindFramebuffer);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BindRenderbuffer);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BindTexture);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BlendColor);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BlendEquation);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BlendEquationSeparate);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BlendFunc);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BlendFuncSeparate);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BufferData);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BufferSubData);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,CheckFramebufferStatus);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Clear);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ClearColor);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ClearDepthf);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ClearStencil);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ColorMask);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,CompileShader);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,CompressedTexImage2D);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,CompressedTexSubImage2D);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,CopyTexImage2D);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,CopyTexSubImage2D);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,CreateProgram);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,CreateShader);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,CullFace);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DeleteBuffers);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DeleteFramebuffers);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DeleteProgram);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DeleteRenderbuffers);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DeleteShader);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DeleteTextures);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DepthFunc);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DepthMask);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DepthRangef);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DetachShader);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Disable);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DisableVertexAttribArray);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DrawArrays);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DrawElements);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Enable);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,EnableVertexAttribArray);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Finish);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Flush);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,FramebufferRenderbuffer);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,FramebufferTexture2D);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,FrontFace);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GenBuffers);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GenerateMipmap);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GenFramebuffers);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GenRenderbuffers);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GenTextures);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetActiveAttrib);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetActiveUniform);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetAttachedShaders);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetAttribLocation);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetBooleanv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetBufferParameteriv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetError);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetFloatv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetFramebufferAttachmentParameteriv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetIntegerv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetProgramiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetProgramInfoLog);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetRenderbufferParameteriv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetShaderiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetShaderInfoLog);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetShaderPrecisionFormat);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetShaderSource);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetString);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetTexParameterfv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetTexParameteriv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetUniformfv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetUniformiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetUniformLocation);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetVertexAttribfv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetVertexAttribiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetVertexAttribPointerv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Hint);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,IsBuffer);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,IsEnabled);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,IsFramebuffer);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,IsProgram);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,IsRenderbuffer);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,IsShader);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,IsTexture);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,LineWidth);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,LinkProgram);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,PixelStorei);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,PolygonOffset);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ReadPixels);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ReleaseShaderCompiler);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,RenderbufferStorage);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,SampleCoverage);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Scissor);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ShaderBinary);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ShaderSource);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,StencilFunc);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,StencilFuncSeparate);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,StencilMask);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,StencilMaskSeparate);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,StencilOp);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,StencilOpSeparate);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,TexImage2D);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,TexParameterf);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,TexParameterfv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,TexParameteri);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,TexParameteriv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,TexSubImage2D);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Uniform1f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Uniform1fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Uniform1i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Uniform1iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Uniform2f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Uniform2fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Uniform2i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Uniform2iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Uniform3f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Uniform3fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Uniform3i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Uniform3iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Uniform4f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Uniform4fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Uniform4i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Uniform4iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,UniformMatrix2fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,UniformMatrix3fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,UniformMatrix4fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,UseProgram);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ValidateProgram);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,VertexAttrib1f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,VertexAttrib1fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,VertexAttrib2f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,VertexAttrib2fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,VertexAttrib3f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,VertexAttrib3fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,VertexAttrib4f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,VertexAttrib4fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,VertexAttribPointer);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Viewport);

/* OpenGL ES 3.0 */

    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ReadBuffer);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DrawRangeElements);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,TexImage3D);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,TexSubImage3D);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,CopyTexSubImage3D);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,CompressedTexImage3D);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,CompressedTexSubImage3D);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GenQueries);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DeleteQueries);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,IsQuery);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BeginQuery);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,EndQuery);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetQueryiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetQueryObjectuiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,UnmapBuffer);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetBufferPointerv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DrawBuffers);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,UniformMatrix2x3fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,UniformMatrix3x2fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,UniformMatrix2x4fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,UniformMatrix4x2fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,UniformMatrix3x4fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,UniformMatrix4x3fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BlitFramebuffer);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,RenderbufferStorageMultisample);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,FramebufferTextureLayer);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,MapBufferRange);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,FlushMappedBufferRange);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BindVertexArray);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DeleteVertexArrays);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GenVertexArrays);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,IsVertexArray);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetIntegeri_v);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BeginTransformFeedback);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,EndTransformFeedback);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BindBufferRange);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BindBufferBase);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,TransformFeedbackVaryings);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetTransformFeedbackVarying);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,VertexAttribIPointer);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetVertexAttribIiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetVertexAttribIuiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,VertexAttribI4i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,VertexAttribI4ui);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,VertexAttribI4iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,VertexAttribI4uiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetUniformuiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetFragDataLocation);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Uniform1ui);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Uniform2ui);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Uniform3ui);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Uniform4ui);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Uniform1uiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Uniform2uiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Uniform3uiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Uniform4uiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ClearBufferiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ClearBufferuiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ClearBufferfv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ClearBufferfi);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetStringi);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,CopyBufferSubData);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetUniformIndices);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetActiveUniformsiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetUniformBlockIndex);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetActiveUniformBlockiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetActiveUniformBlockName);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,UniformBlockBinding);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DrawArraysInstanced);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DrawElementsInstanced);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,FenceSync);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,IsSync);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DeleteSync);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ClientWaitSync);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,WaitSync);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetInteger64v);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetSynciv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetInteger64i_v);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetBufferParameteri64v);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GenSamplers);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DeleteSamplers);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,IsSampler);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BindSampler);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,SamplerParameteri);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,SamplerParameteriv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,SamplerParameterf);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,SamplerParameterfv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetSamplerParameteriv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetSamplerParameterfv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,VertexAttribDivisor);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BindTransformFeedback);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DeleteTransformFeedbacks);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GenTransformFeedbacks);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,IsTransformFeedback);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,PauseTransformFeedback);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ResumeTransformFeedback);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetProgramBinary);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramBinary);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramParameteri);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,InvalidateFramebuffer);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,InvalidateSubFramebuffer);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,TexStorage2D);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,TexStorage3D);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetInternalformativ);

/* OpenGL ES 3.1 */

    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DispatchCompute);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DispatchComputeIndirect);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DrawArraysIndirect);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DrawElementsIndirect);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,FramebufferParameteri);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetFramebufferParameteriv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetProgramInterfaceiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetProgramResourceIndex);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetProgramResourceName);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetProgramResourceiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetProgramResourceLocation);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,UseProgramStages);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ActiveShaderProgram);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,CreateShaderProgramv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BindProgramPipeline);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DeleteProgramPipelines);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GenProgramPipelines);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,IsProgramPipeline);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetProgramPipelineiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniform1i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniform2i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniform3i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniform4i);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniform1ui);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniform2ui);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniform3ui);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniform4ui);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniform1f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniform2f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniform3f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniform4f);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniform1iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniform2iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniform3iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniform4iv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniform1uiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniform2uiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniform3uiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniform4uiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniform1fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniform2fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniform3fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniform4fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniformMatrix2fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniformMatrix3fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniformMatrix4fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniformMatrix2x3fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniformMatrix3x2fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniformMatrix2x4fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniformMatrix4x2fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniformMatrix3x4fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ProgramUniformMatrix4x3fv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ValidateProgramPipeline);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetProgramPipelineInfoLog);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BindImageTexture);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetBooleani_v);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,MemoryBarrier);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,MemoryBarrierByRegion);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,TexStorage2DMultisample);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetMultisamplefv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,SampleMaski);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetTexLevelParameteriv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetTexLevelParameterfv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BindVertexBuffer);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,VertexAttribFormat);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,VertexAttribIFormat);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,VertexAttribBinding);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,VertexBindingDivisor);

/* OpenGL ES 3.2 */
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,TexStorage3DMultisample);

    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BlendBarrier);

    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DebugMessageControl);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DebugMessageInsert);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DebugMessageCallback);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetDebugMessageLog);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetPointerv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,PushDebugGroup);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,PopDebugGroup);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ObjectLabel);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetObjectLabel);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ObjectPtrLabel);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetObjectPtrLabel);


    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetGraphicsResetStatus);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ReadnPixels);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetnUniformfv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetnUniformiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetnUniformuiv);

    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BlendEquationi);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BlendEquationSeparatei);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BlendFunci);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,BlendFuncSeparatei);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,ColorMaski);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Enablei);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,Disablei);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,IsEnabledi);

    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,TexParameterIiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,TexParameterIuiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetTexParameterIiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetTexParameterIuiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,SamplerParameterIiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,SamplerParameterIuiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetSamplerParameterIiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetSamplerParameterIuiv);

    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,TexBuffer);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,TexBufferRange);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,PatchParameteri);

    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,FramebufferTexture);

    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,MinSampleShading);

    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,CopyImageSubData);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DrawElementsBaseVertex);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DrawRangeElementsBaseVertex);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DrawElementsInstancedBaseVertex);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,PrimitiveBoundingBox);

/* OpenGL ES extension */
#if GL_OES_EGL_image
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,EGLImageTargetTexture2DOES);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,EGLImageTargetRenderbufferStorageOES);
#endif

#if GL_EXT_multi_draw_arrays
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,MultiDrawArraysEXT);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,MultiDrawElementsEXT);
#if GL_EXT_draw_elements_base_vertex
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,MultiDrawElementsBaseVertexEXT);
#endif
#endif

#if GL_OES_mapbuffer
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetBufferPointervOES);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,MapBufferOES);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,UnmapBufferOES);
#endif

#if GL_EXT_discard_framebuffer
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DiscardFramebufferEXT);
#endif

#if GL_EXT_multisampled_render_to_texture
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,RenderbufferStorageMultisampleEXT);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,FramebufferTexture2DMultisampleEXT);
#endif

#if GL_VIV_direct_texture
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,TexDirectVIV);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,TexDirectInvalidateVIV);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,TexDirectVIVMap);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,TexDirectTiledMapVIV);
#endif

#if GL_EXT_multi_draw_indirect
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,MultiDrawArraysIndirectEXT);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,MultiDrawElementsIndirectEXT);
#endif

    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetObjectParameterivARB);
/* To do */

    SetDLTABLE((&(gc->listCompileDispatchTable)),(gc->currentImmediateTable) ,GetVertexAttribdv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(gc->currentImmediateTable) ,BindFragDataLocation);
/*
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetQueryObjectiv);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetBufferSubData);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,DeleteObjectARB);
    SetDLTABLE((&(gc->listCompileDispatchTable)),(&(gc->apiDispatchTable)) ,GetInfoLogARB);
*/
}
