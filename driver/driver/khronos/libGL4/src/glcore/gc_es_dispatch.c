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
#include "g_lefncs.h"
#include "g_lcfncs.h"

/* ES 3.1 + Extension API Function Dispatch Table */
typedef const GLubyte * (*GLLEFUNCTION)(__GLcontext*, const GLubyte*);

#define __glim(func)    __glim_##func
#define __gllc(func)    __gllc_##func
#define __glnop(func)   __glnop_##func
#define __glle(func)    (GLLEFUNCTION)__glle_##func

/*
** OGL Nop functions
*/
GLvoid GL_APIENTRY __glnop_NewList(__GLcontext *gc, GLuint list, GLenum mode) {}
GLvoid GL_APIENTRY __glnop_EndList(__GLcontext *gc) {}
GLvoid GL_APIENTRY __glnop_CallList(__GLcontext *gc, GLuint list) {}
GLvoid GL_APIENTRY __glnop_CallLists(__GLcontext *gc, GLsizei n, GLenum type, const GLvoid *lists) {}
GLvoid GL_APIENTRY __glnop_DeleteLists(__GLcontext *gc, GLuint list, GLsizei range) {}
GLuint GL_APIENTRY __glnop_GenLists(__GLcontext *gc, GLsizei range) {return 0;}
GLvoid GL_APIENTRY __glnop_ListBase(__GLcontext *gc, GLuint base) {}
GLvoid GL_APIENTRY __glnop_Begin(__GLcontext *gc, GLenum mode) {}
GLvoid GL_APIENTRY __glnop_Bitmap(__GLcontext *gc, GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap) {}
GLvoid GL_APIENTRY __glnop_Color3b(__GLcontext *gc, GLbyte red, GLbyte green, GLbyte blue) {}
GLvoid GL_APIENTRY __glnop_Color3bv(__GLcontext *gc, const GLbyte *v) {}
GLvoid GL_APIENTRY __glnop_Color3d(__GLcontext *gc, GLdouble red, GLdouble green, GLdouble blue) {}
GLvoid GL_APIENTRY __glnop_Color3dv(__GLcontext *gc, const GLdouble *v) {}
GLvoid GL_APIENTRY __glnop_Color3f(__GLcontext *gc, GLfloat red, GLfloat green, GLfloat blue) {}
GLvoid GL_APIENTRY __glnop_Color3fv(__GLcontext *gc, const GLfloat *v) {}
GLvoid GL_APIENTRY __glnop_Color3i(__GLcontext *gc, GLint red, GLint green, GLint blue) {}
GLvoid GL_APIENTRY __glnop_Color3iv(__GLcontext *gc, const GLint *v) {}
GLvoid GL_APIENTRY __glnop_Color3s(__GLcontext *gc, GLshort red, GLshort green, GLshort blue) {}
GLvoid GL_APIENTRY __glnop_Color3sv(__GLcontext *gc, const GLshort *v) {}
GLvoid GL_APIENTRY __glnop_Color3ub(__GLcontext *gc, GLubyte red, GLubyte green, GLubyte blue) {}
GLvoid GL_APIENTRY __glnop_Color3ubv(__GLcontext *gc, const GLubyte *v) {}
GLvoid GL_APIENTRY __glnop_Color3ui(__GLcontext *gc, GLuint red, GLuint green, GLuint blue) {}
GLvoid GL_APIENTRY __glnop_Color3uiv(__GLcontext *gc, const GLuint *v) {}
GLvoid GL_APIENTRY __glnop_Color3us(__GLcontext *gc, GLushort red, GLushort green, GLushort blue) {}
GLvoid GL_APIENTRY __glnop_Color3usv(__GLcontext *gc, const GLushort *v) {}
GLvoid GL_APIENTRY __glnop_Color4b(__GLcontext *gc, GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha) {}
GLvoid GL_APIENTRY __glnop_Color4bv(__GLcontext *gc, const GLbyte *v) {}
GLvoid GL_APIENTRY __glnop_Color4d(__GLcontext *gc, GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha) {}
GLvoid GL_APIENTRY __glnop_Color4dv(__GLcontext *gc, const GLdouble *v) {}
GLvoid GL_APIENTRY __glnop_Color4f(__GLcontext *gc, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {}
GLvoid GL_APIENTRY __glnop_Color4fv(__GLcontext *gc, const GLfloat *v) {}
GLvoid GL_APIENTRY __glnop_Color4i(__GLcontext *gc, GLint red, GLint green, GLint blue, GLint alpha) {}
GLvoid GL_APIENTRY __glnop_Color4iv(__GLcontext *gc, const GLint *v) {}
GLvoid GL_APIENTRY __glnop_Color4s(__GLcontext *gc, GLshort red, GLshort green, GLshort blue, GLshort alpha) {}
GLvoid GL_APIENTRY __glnop_Color4sv(__GLcontext *gc, const GLshort *v) {}
GLvoid GL_APIENTRY __glnop_Color4ub(__GLcontext *gc, GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha) {}
GLvoid GL_APIENTRY __glnop_Color4ubv(__GLcontext *gc, const GLubyte *v) {}
GLvoid GL_APIENTRY __glnop_Color4ui(__GLcontext *gc, GLuint red, GLuint green, GLuint blue, GLuint alpha) {}
GLvoid GL_APIENTRY __glnop_Color4uiv(__GLcontext *gc, const GLuint *v) {}
GLvoid GL_APIENTRY __glnop_Color4us(__GLcontext *gc, GLushort red, GLushort green, GLushort blue, GLushort alpha) {}
GLvoid GL_APIENTRY __glnop_Color4usv(__GLcontext *gc, const GLushort *v) {}
GLvoid GL_APIENTRY __glnop_EdgeFlag(__GLcontext *gc, GLboolean flag) {}
GLvoid GL_APIENTRY __glnop_EdgeFlagv(__GLcontext *gc, const GLboolean *flag) {}
GLvoid GL_APIENTRY __glnop_End(__GLcontext *gc) {}
GLvoid GL_APIENTRY __glnop_Indexd(__GLcontext *gc, GLdouble c) {}
GLvoid GL_APIENTRY __glnop_Indexdv(__GLcontext *gc, const GLdouble *c) {}
GLvoid GL_APIENTRY __glnop_Indexf(__GLcontext *gc, GLfloat c) {}
GLvoid GL_APIENTRY __glnop_Indexfv(__GLcontext *gc, const GLfloat *c) {}
GLvoid GL_APIENTRY __glnop_Indexi(__GLcontext *gc, GLint c) {}
GLvoid GL_APIENTRY __glnop_Indexiv(__GLcontext *gc, const GLint *c) {}
GLvoid GL_APIENTRY __glnop_Indexs(__GLcontext *gc, GLshort c) {}
GLvoid GL_APIENTRY __glnop_Indexsv(__GLcontext *gc, const GLshort *c) {}
GLvoid GL_APIENTRY __glnop_Normal3b(__GLcontext *gc, GLbyte nx, GLbyte ny, GLbyte nz) {}
GLvoid GL_APIENTRY __glnop_Normal3bv(__GLcontext *gc, const GLbyte *v) {}
GLvoid GL_APIENTRY __glnop_Normal3d(__GLcontext *gc, GLdouble nx, GLdouble ny, GLdouble nz) {}
GLvoid GL_APIENTRY __glnop_Normal3dv(__GLcontext *gc, const GLdouble *v) {}
GLvoid GL_APIENTRY __glnop_Normal3f(__GLcontext *gc, GLfloat nx, GLfloat ny, GLfloat nz) {}
GLvoid GL_APIENTRY __glnop_Normal3fv(__GLcontext *gc, const GLfloat *v) {}
GLvoid GL_APIENTRY __glnop_Normal3i(__GLcontext *gc, GLint nx, GLint ny, GLint nz) {}
GLvoid GL_APIENTRY __glnop_Normal3iv(__GLcontext *gc, const GLint *v) {}
GLvoid GL_APIENTRY __glnop_Normal3s(__GLcontext *gc, GLshort nx, GLshort ny, GLshort nz) {}
GLvoid GL_APIENTRY __glnop_Normal3sv(__GLcontext *gc, const GLshort *v) {}
GLvoid GL_APIENTRY __glnop_RasterPos2d(__GLcontext *gc, GLdouble x, GLdouble y) {}
GLvoid GL_APIENTRY __glnop_RasterPos2dv(__GLcontext *gc, const GLdouble *v) {}
GLvoid GL_APIENTRY __glnop_RasterPos2f(__GLcontext *gc, GLfloat x, GLfloat y) {}
GLvoid GL_APIENTRY __glnop_RasterPos2fv(__GLcontext *gc, const GLfloat *v) {}
GLvoid GL_APIENTRY __glnop_RasterPos2i(__GLcontext *gc, GLint x, GLint y) {}
GLvoid GL_APIENTRY __glnop_RasterPos2iv(__GLcontext *gc, const GLint *v) {}
GLvoid GL_APIENTRY __glnop_RasterPos2s(__GLcontext *gc, GLshort x, GLshort y) {}
GLvoid GL_APIENTRY __glnop_RasterPos2sv(__GLcontext *gc, const GLshort *v) {}
GLvoid GL_APIENTRY __glnop_RasterPos3d(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z) {}
GLvoid GL_APIENTRY __glnop_RasterPos3dv(__GLcontext *gc, const GLdouble *v) {}
GLvoid GL_APIENTRY __glnop_RasterPos3f(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z) {}
GLvoid GL_APIENTRY __glnop_RasterPos3fv(__GLcontext *gc, const GLfloat *v) {}
GLvoid GL_APIENTRY __glnop_RasterPos3i(__GLcontext *gc, GLint x, GLint y, GLint z) {}
GLvoid GL_APIENTRY __glnop_RasterPos3iv(__GLcontext *gc, const GLint *v) {}
GLvoid GL_APIENTRY __glnop_RasterPos3s(__GLcontext *gc, GLshort x, GLshort y, GLshort z) {}
GLvoid GL_APIENTRY __glnop_RasterPos3sv(__GLcontext *gc, const GLshort *v) {}
GLvoid GL_APIENTRY __glnop_RasterPos4d(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {}
GLvoid GL_APIENTRY __glnop_RasterPos4dv(__GLcontext *gc, const GLdouble *v) {}
GLvoid GL_APIENTRY __glnop_RasterPos4f(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {}
GLvoid GL_APIENTRY __glnop_RasterPos4fv(__GLcontext *gc, const GLfloat *v) {}
GLvoid GL_APIENTRY __glnop_RasterPos4i(__GLcontext *gc, GLint x, GLint y, GLint z, GLint w) {}
GLvoid GL_APIENTRY __glnop_RasterPos4iv(__GLcontext *gc, const GLint *v) {}
GLvoid GL_APIENTRY __glnop_RasterPos4s(__GLcontext *gc, GLshort x, GLshort y, GLshort z, GLshort w) {}
GLvoid GL_APIENTRY __glnop_RasterPos4sv(__GLcontext *gc, const GLshort *v) {}
GLvoid GL_APIENTRY __glnop_Rectd(__GLcontext *gc, GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2) {}
GLvoid GL_APIENTRY __glnop_Rectdv(__GLcontext *gc, const GLdouble *v1, const GLdouble *v2) {}
GLvoid GL_APIENTRY __glnop_Rectf(__GLcontext *gc, GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {}
GLvoid GL_APIENTRY __glnop_Rectfv(__GLcontext *gc, const GLfloat *v1, const GLfloat *v2) {}
GLvoid GL_APIENTRY __glnop_Recti(__GLcontext *gc, GLint x1, GLint y1, GLint x2, GLint y2) {}
GLvoid GL_APIENTRY __glnop_Rectiv(__GLcontext *gc, const GLint *v1, const GLint *v2) {}
GLvoid GL_APIENTRY __glnop_Rects(__GLcontext *gc, GLshort x1, GLshort y1, GLshort x2, GLshort y2) {}
GLvoid GL_APIENTRY __glnop_Rectsv(__GLcontext *gc, const GLshort *v1, const GLshort *v2) {}
GLvoid GL_APIENTRY __glnop_TexCoord1d(__GLcontext *gc, GLdouble s) {}
GLvoid GL_APIENTRY __glnop_TexCoord1dv(__GLcontext *gc, const GLdouble *v) {}
GLvoid GL_APIENTRY __glnop_TexCoord1f(__GLcontext *gc, GLfloat s) {}
GLvoid GL_APIENTRY __glnop_TexCoord1fv(__GLcontext *gc, const GLfloat *v) {}
GLvoid GL_APIENTRY __glnop_TexCoord1i(__GLcontext *gc, GLint s) {}
GLvoid GL_APIENTRY __glnop_TexCoord1iv(__GLcontext *gc, const GLint *v) {}
GLvoid GL_APIENTRY __glnop_TexCoord1s(__GLcontext *gc, GLshort s) {}
GLvoid GL_APIENTRY __glnop_TexCoord1sv(__GLcontext *gc, const GLshort *v) {}
GLvoid GL_APIENTRY __glnop_TexCoord2d(__GLcontext *gc, GLdouble s, GLdouble t) {}
GLvoid GL_APIENTRY __glnop_TexCoord2dv(__GLcontext *gc, const GLdouble *v) {}
GLvoid GL_APIENTRY __glnop_TexCoord2f(__GLcontext *gc, GLfloat s, GLfloat t) {}
GLvoid GL_APIENTRY __glnop_TexCoord2fv(__GLcontext *gc, const GLfloat *v) {}
GLvoid GL_APIENTRY __glnop_TexCoord2i(__GLcontext *gc, GLint s, GLint t) {}
GLvoid GL_APIENTRY __glnop_TexCoord2iv(__GLcontext *gc, const GLint *v) {}
GLvoid GL_APIENTRY __glnop_TexCoord2s(__GLcontext *gc, GLshort s, GLshort t) {}
GLvoid GL_APIENTRY __glnop_TexCoord2sv(__GLcontext *gc, const GLshort *v) {}
GLvoid GL_APIENTRY __glnop_TexCoord3d(__GLcontext *gc, GLdouble s, GLdouble t, GLdouble r) {}
GLvoid GL_APIENTRY __glnop_TexCoord3dv(__GLcontext *gc, const GLdouble *v) {}
GLvoid GL_APIENTRY __glnop_TexCoord3f(__GLcontext *gc, GLfloat s, GLfloat t, GLfloat r) {}
GLvoid GL_APIENTRY __glnop_TexCoord3fv(__GLcontext *gc, const GLfloat *v) {}
GLvoid GL_APIENTRY __glnop_TexCoord3i(__GLcontext *gc, GLint s, GLint t, GLint r) {}
GLvoid GL_APIENTRY __glnop_TexCoord3iv(__GLcontext *gc, const GLint *v) {}
GLvoid GL_APIENTRY __glnop_TexCoord3s(__GLcontext *gc, GLshort s, GLshort t, GLshort r) {}
GLvoid GL_APIENTRY __glnop_TexCoord3sv(__GLcontext *gc, const GLshort *v) {}
GLvoid GL_APIENTRY __glnop_TexCoord4d(__GLcontext *gc, GLdouble s, GLdouble t, GLdouble r, GLdouble q) {}
GLvoid GL_APIENTRY __glnop_TexCoord4dv(__GLcontext *gc, const GLdouble *v) {}
GLvoid GL_APIENTRY __glnop_TexCoord4f(__GLcontext *gc, GLfloat s, GLfloat t, GLfloat r, GLfloat q) {}
GLvoid GL_APIENTRY __glnop_TexCoord4fv(__GLcontext *gc, const GLfloat *v) {}
GLvoid GL_APIENTRY __glnop_TexCoord4i(__GLcontext *gc, GLint s, GLint t, GLint r, GLint q) {}
GLvoid GL_APIENTRY __glnop_TexCoord4iv(__GLcontext *gc, const GLint *v) {}
GLvoid GL_APIENTRY __glnop_TexCoord4s(__GLcontext *gc, GLshort s, GLshort t, GLshort r, GLshort q) {}
GLvoid GL_APIENTRY __glnop_TexCoord4sv(__GLcontext *gc, const GLshort *v) {}
GLvoid GL_APIENTRY __glnop_Vertex2d(__GLcontext *gc, GLdouble x, GLdouble y) {}
GLvoid GL_APIENTRY __glnop_Vertex2dv(__GLcontext *gc, const GLdouble *v) {}
GLvoid GL_APIENTRY __glnop_Vertex2f(__GLcontext *gc, GLfloat x, GLfloat y) {}
GLvoid GL_APIENTRY __glnop_Vertex2fv(__GLcontext *gc, const GLfloat *v) {}
GLvoid GL_APIENTRY __glnop_Vertex2i(__GLcontext *gc, GLint x, GLint y) {}
GLvoid GL_APIENTRY __glnop_Vertex2iv(__GLcontext *gc, const GLint *v) {}
GLvoid GL_APIENTRY __glnop_Vertex2s(__GLcontext *gc, GLshort x, GLshort y) {}
GLvoid GL_APIENTRY __glnop_Vertex2sv(__GLcontext *gc, const GLshort *v) {}
GLvoid GL_APIENTRY __glnop_Vertex3d(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z) {}
GLvoid GL_APIENTRY __glnop_Vertex3dv(__GLcontext *gc, const GLdouble *v) {}
GLvoid GL_APIENTRY __glnop_Vertex3f(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z) {}
GLvoid GL_APIENTRY __glnop_Vertex3fv(__GLcontext *gc, const GLfloat *v) {}
GLvoid GL_APIENTRY __glnop_Vertex3i(__GLcontext *gc, GLint x, GLint y, GLint z) {}
GLvoid GL_APIENTRY __glnop_Vertex3iv(__GLcontext *gc, const GLint *v) {}
GLvoid GL_APIENTRY __glnop_Vertex3s(__GLcontext *gc, GLshort x, GLshort y, GLshort z) {}
GLvoid GL_APIENTRY __glnop_Vertex3sv(__GLcontext *gc, const GLshort *v) {}
GLvoid GL_APIENTRY __glnop_Vertex4d(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {}
GLvoid GL_APIENTRY __glnop_Vertex4dv(__GLcontext *gc, const GLdouble *v) {}
GLvoid GL_APIENTRY __glnop_Vertex4f(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {}
GLvoid GL_APIENTRY __glnop_Vertex4fv(__GLcontext *gc, const GLfloat *v) {}
GLvoid GL_APIENTRY __glnop_Vertex4i(__GLcontext *gc, GLint x, GLint y, GLint z, GLint w) {}
GLvoid GL_APIENTRY __glnop_Vertex4iv(__GLcontext *gc, const GLint *v) {}
GLvoid GL_APIENTRY __glnop_Vertex4s(__GLcontext *gc, GLshort x, GLshort y, GLshort z, GLshort w) {}
GLvoid GL_APIENTRY __glnop_Vertex4sv(__GLcontext *gc, const GLshort *v) {}
GLvoid GL_APIENTRY __glnop_ClipPlane(__GLcontext *gc, GLenum plane, const GLdouble *equation) {}
GLvoid GL_APIENTRY __glnop_ColorMaterial(__GLcontext *gc, GLenum face, GLenum mode) {}
GLvoid GL_APIENTRY __glnop_CullFace(__GLcontext *gc, GLenum mode) {}
GLvoid GL_APIENTRY __glnop_Fogf(__GLcontext *gc, GLenum pname, GLfloat param) {}
GLvoid GL_APIENTRY __glnop_Fogfv(__GLcontext *gc, GLenum pname, const GLfloat *params) {}
GLvoid GL_APIENTRY __glnop_Fogi(__GLcontext *gc, GLenum pname, GLint param) {}
GLvoid GL_APIENTRY __glnop_Fogiv(__GLcontext *gc, GLenum pname, const GLint *params) {}
GLvoid GL_APIENTRY __glnop_FrontFace(__GLcontext *gc, GLenum mode) {}
GLvoid GL_APIENTRY __glnop_Hint(__GLcontext *gc, GLenum target, GLenum mode) {}
GLvoid GL_APIENTRY __glnop_Lightf(__GLcontext *gc, GLenum light, GLenum pname, GLfloat param) {}
GLvoid GL_APIENTRY __glnop_Lightfv(__GLcontext *gc, GLenum light, GLenum pname, const GLfloat *params) {}
GLvoid GL_APIENTRY __glnop_Lighti(__GLcontext *gc, GLenum light, GLenum pname, GLint param) {}
GLvoid GL_APIENTRY __glnop_Lightiv(__GLcontext *gc, GLenum light, GLenum pname, const GLint *params) {}
GLvoid GL_APIENTRY __glnop_LightModelf(__GLcontext *gc, GLenum pname, GLfloat param) {}
GLvoid GL_APIENTRY __glnop_LightModelfv(__GLcontext *gc, GLenum pname, const GLfloat *params) {}
GLvoid GL_APIENTRY __glnop_LightModeli(__GLcontext *gc, GLenum pname, GLint param) {}
GLvoid GL_APIENTRY __glnop_LightModeliv(__GLcontext *gc, GLenum pname, const GLint *params) {}
GLvoid GL_APIENTRY __glnop_LineStipple(__GLcontext *gc, GLint factor, GLushort pattern) {}
GLvoid GL_APIENTRY __glnop_LineWidth(__GLcontext *gc, GLfloat width) {}
GLvoid GL_APIENTRY __glnop_Materialf(__GLcontext *gc, GLenum face, GLenum pname, GLfloat param) {}
GLvoid GL_APIENTRY __glnop_Materialfv(__GLcontext *gc, GLenum face, GLenum pname, const GLfloat *params) {}
GLvoid GL_APIENTRY __glnop_Materiali(__GLcontext *gc, GLenum face, GLenum pname, GLint param) {}
GLvoid GL_APIENTRY __glnop_Materialiv(__GLcontext *gc, GLenum face, GLenum pname, const GLint *params) {}
GLvoid GL_APIENTRY __glnop_PointSize(__GLcontext *gc, GLfloat size) {}
GLvoid GL_APIENTRY __glnop_PolygonMode(__GLcontext *gc, GLenum face, GLenum mode) {}
GLvoid GL_APIENTRY __glnop_PolygonStipple(__GLcontext *gc, const GLubyte *mask) {}
GLvoid GL_APIENTRY __glnop_Scissor(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height) {}
GLvoid GL_APIENTRY __glnop_ShadeModel(__GLcontext *gc, GLenum mode) {}
GLvoid GL_APIENTRY __glnop_TexParameterf(__GLcontext *gc, GLenum target, GLenum pname, GLfloat param) {}
GLvoid GL_APIENTRY __glnop_TexParameterfv(__GLcontext *gc, GLenum target, GLenum pname, const GLfloat *params) {}
GLvoid GL_APIENTRY __glnop_TexParameteri(__GLcontext *gc, GLenum target, GLenum pname, GLint param) {}
GLvoid GL_APIENTRY __glnop_TexParameteriv(__GLcontext *gc, GLenum target, GLenum pname, const GLint *params) {}
GLvoid GL_APIENTRY __glnop_TexImage1D(__GLcontext *gc, GLenum target, GLint level, GLint internalFormat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels) {}
GLvoid GL_APIENTRY __glnop_TexImage2D(__GLcontext *gc, GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels) {}
GLvoid GL_APIENTRY __glnop_TexEnvf(__GLcontext *gc, GLenum target, GLenum pname, GLfloat param) {}
GLvoid GL_APIENTRY __glnop_TexEnvfv(__GLcontext *gc, GLenum target, GLenum pname, const GLfloat *params) {}
GLvoid GL_APIENTRY __glnop_TexEnvi(__GLcontext *gc, GLenum target, GLenum pname, GLint param) {}
GLvoid GL_APIENTRY __glnop_TexEnviv(__GLcontext *gc, GLenum target, GLenum pname, const GLint *params) {}
GLvoid GL_APIENTRY __glnop_TexGend(__GLcontext *gc, GLenum coord, GLenum pname, GLdouble param) {}
GLvoid GL_APIENTRY __glnop_TexGendv(__GLcontext *gc, GLenum coord, GLenum pname, const GLdouble *params) {}
GLvoid GL_APIENTRY __glnop_TexGenf(__GLcontext *gc, GLenum coord, GLenum pname, GLfloat param) {}
GLvoid GL_APIENTRY __glnop_TexGenfv(__GLcontext *gc, GLenum coord, GLenum pname, const GLfloat *params) {}
GLvoid GL_APIENTRY __glnop_TexGeni(__GLcontext *gc, GLenum coord, GLenum pname, GLint param) {}
GLvoid GL_APIENTRY __glnop_TexGeniv(__GLcontext *gc, GLenum coord, GLenum pname, const GLint *params) {}
GLvoid GL_APIENTRY __glnop_FeedbackBuffer(__GLcontext *gc, GLsizei size, GLenum type, GLfloat *buffer) {}
GLvoid GL_APIENTRY __glnop_SelectBuffer(__GLcontext *gc, GLsizei size, GLuint *buffer) {}
GLint GL_APIENTRY __glnop_RenderMode(__GLcontext *gc, GLenum mode) {return 0;}
GLvoid GL_APIENTRY __glnop_InitNames(__GLcontext *gc) {}
GLvoid GL_APIENTRY __glnop_LoadName(__GLcontext *gc, GLuint name) {}
GLvoid GL_APIENTRY __glnop_PassThrough(__GLcontext *gc, GLfloat token) {}
GLvoid GL_APIENTRY __glnop_PopName(__GLcontext *gc) {}
GLvoid GL_APIENTRY __glnop_PushName(__GLcontext *gc, GLuint name) {}
GLvoid GL_APIENTRY __glnop_DrawBuffer(__GLcontext *gc, GLenum mode) {}
GLvoid GL_APIENTRY __glnop_Clear(__GLcontext *gc, GLbitfield mask) {}
GLvoid GL_APIENTRY __glnop_ClearAccum(__GLcontext *gc, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {}
GLvoid GL_APIENTRY __glnop_ClearIndex(__GLcontext *gc, GLfloat c) {}
GLvoid GL_APIENTRY __glnop_ClearColor(__GLcontext *gc, GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {}
GLvoid GL_APIENTRY __glnop_ClearStencil(__GLcontext *gc, GLint s) {}
GLvoid GL_APIENTRY __glnop_ClearDepth(__GLcontext *gc, GLclampd depth) {}
GLvoid GL_APIENTRY __glnop_StencilMask(__GLcontext *gc, GLuint mask) {}
GLvoid GL_APIENTRY __glnop_ColorMask(__GLcontext *gc, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {}
GLvoid GL_APIENTRY __glnop_DepthMask(__GLcontext *gc, GLboolean flag) {}
GLvoid GL_APIENTRY __glnop_IndexMask(__GLcontext *gc, GLuint mask) {}
GLvoid GL_APIENTRY __glnop_Accum(__GLcontext *gc, GLenum op, GLfloat value) {}
GLvoid GL_APIENTRY __glnop_Disable(__GLcontext *gc, GLenum cap) {}
GLvoid GL_APIENTRY __glnop_Enable(__GLcontext *gc, GLenum cap) {}
GLvoid GL_APIENTRY __glnop_Finish(__GLcontext *gc) {}
GLvoid GL_APIENTRY __glnop_Flush(__GLcontext *gc) {}
GLvoid GL_APIENTRY __glnop_PopAttrib(__GLcontext *gc) {}
GLvoid GL_APIENTRY __glnop_PushAttrib(__GLcontext *gc, GLbitfield mask) {}
GLvoid GL_APIENTRY __glnop_Map1d(__GLcontext *gc, GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points) {}
GLvoid GL_APIENTRY __glnop_Map1f(__GLcontext *gc, GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points) {}
GLvoid GL_APIENTRY __glnop_Map2d(__GLcontext *gc, GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points) {}
GLvoid GL_APIENTRY __glnop_Map2f(__GLcontext *gc, GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points) {}
GLvoid GL_APIENTRY __glnop_MapGrid1d(__GLcontext *gc, GLint un, GLdouble u1, GLdouble u2) {}
GLvoid GL_APIENTRY __glnop_MapGrid1f(__GLcontext *gc, GLint un, GLfloat u1, GLfloat u2) {}
GLvoid GL_APIENTRY __glnop_MapGrid2d(__GLcontext *gc, GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2) {}
GLvoid GL_APIENTRY __glnop_MapGrid2f(__GLcontext *gc, GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2) {}
GLvoid GL_APIENTRY __glnop_EvalCoord1d(__GLcontext *gc, GLdouble u) {}
GLvoid GL_APIENTRY __glnop_EvalCoord1dv(__GLcontext *gc, const GLdouble *u) {}
GLvoid GL_APIENTRY __glnop_EvalCoord1f(__GLcontext *gc, GLfloat u) {}
GLvoid GL_APIENTRY __glnop_EvalCoord1fv(__GLcontext *gc, const GLfloat *u) {}
GLvoid GL_APIENTRY __glnop_EvalCoord2d(__GLcontext *gc, GLdouble u, GLdouble v) {}
GLvoid GL_APIENTRY __glnop_EvalCoord2dv(__GLcontext *gc, const GLdouble *u) {}
GLvoid GL_APIENTRY __glnop_EvalCoord2f(__GLcontext *gc, GLfloat u, GLfloat v) {}
GLvoid GL_APIENTRY __glnop_EvalCoord2fv(__GLcontext *gc, const GLfloat *u) {}
GLvoid GL_APIENTRY __glnop_EvalMesh1(__GLcontext *gc, GLenum mode, GLint i1, GLint i2) {}
GLvoid GL_APIENTRY __glnop_EvalPoint1(__GLcontext *gc, GLint i) {}
GLvoid GL_APIENTRY __glnop_EvalMesh2(__GLcontext *gc, GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2) {}
GLvoid GL_APIENTRY __glnop_EvalPoint2(__GLcontext *gc, GLint i, GLint j) {}
GLvoid GL_APIENTRY __glnop_AlphaFunc(__GLcontext *gc, GLenum func, GLclampf ref) {}
GLvoid GL_APIENTRY __glnop_BlendFunc(__GLcontext *gc, GLenum sfactor, GLenum dfactor) {}
GLvoid GL_APIENTRY __glnop_LogicOp(__GLcontext *gc, GLenum opcode) {}
GLvoid GL_APIENTRY __glnop_StencilFunc(__GLcontext *gc, GLenum func, GLint ref, GLuint mask) {}
GLvoid GL_APIENTRY __glnop_StencilOp(__GLcontext *gc, GLenum fail, GLenum zfail, GLenum zpass) {}
GLvoid GL_APIENTRY __glnop_DepthFunc(__GLcontext *gc, GLenum func) {}
GLvoid GL_APIENTRY __glnop_PixelZoom(__GLcontext *gc, GLfloat xfactor, GLfloat yfactor) {}
GLvoid GL_APIENTRY __glnop_PixelTransferf(__GLcontext *gc, GLenum pname, GLfloat param) {}
GLvoid GL_APIENTRY __glnop_PixelTransferi(__GLcontext *gc, GLenum pname, GLint param) {}
GLvoid GL_APIENTRY __glnop_PixelStoref(__GLcontext *gc, GLenum pname, GLfloat param) {}
GLvoid GL_APIENTRY __glnop_PixelStorei(__GLcontext *gc, GLenum pname, GLint param) {}
GLvoid GL_APIENTRY __glnop_PixelMapfv(__GLcontext *gc, GLenum map, GLsizei mapsize, const GLfloat *values) {}
GLvoid GL_APIENTRY __glnop_PixelMapuiv(__GLcontext *gc, GLenum map, GLsizei mapsize, const GLuint *values) {}
GLvoid GL_APIENTRY __glnop_PixelMapusv(__GLcontext *gc, GLenum map, GLsizei mapsize, const GLushort *values) {}
GLvoid GL_APIENTRY __glnop_ReadBuffer(__GLcontext *gc, GLenum mode) {}
GLvoid GL_APIENTRY __glnop_CopyPixels(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height, GLenum type) {}
GLvoid GL_APIENTRY __glnop_ReadPixels(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels) {}
GLvoid GL_APIENTRY __glnop_DrawPixels(__GLcontext *gc, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels) {}
GLvoid GL_APIENTRY __glnop_GetBooleanv(__GLcontext *gc, GLenum pname, GLboolean *params) {}
GLvoid GL_APIENTRY __glnop_GetClipPlane(__GLcontext *gc, GLenum plane, GLdouble *equation) {}
GLvoid GL_APIENTRY __glnop_GetDoublev(__GLcontext *gc, GLenum pname, GLdouble *params) {}
GLenum GL_APIENTRY __glnop_GetError(__GLcontext *gc) {return GL_NO_ERROR;}
GLvoid GL_APIENTRY __glnop_GetFloatv(__GLcontext *gc, GLenum pname, GLfloat *params) {}
GLvoid GL_APIENTRY __glnop_GetIntegerv(__GLcontext *gc, GLenum pname, GLint *params) {}
GLvoid GL_APIENTRY __glnop_GetLightfv(__GLcontext *gc, GLenum light, GLenum pname, GLfloat *params) {}
GLvoid GL_APIENTRY __glnop_GetLightiv(__GLcontext *gc, GLenum light, GLenum pname, GLint *params) {}
GLvoid GL_APIENTRY __glnop_GetMapdv(__GLcontext *gc, GLenum target, GLenum query, GLdouble *v) {}
GLvoid GL_APIENTRY __glnop_GetMapfv(__GLcontext *gc, GLenum target, GLenum query, GLfloat *v) {}
GLvoid GL_APIENTRY __glnop_GetMapiv(__GLcontext *gc, GLenum target, GLenum query, GLint *v) {}
GLvoid GL_APIENTRY __glnop_GetMaterialfv(__GLcontext *gc, GLenum face, GLenum pname, GLfloat *params) {}
GLvoid GL_APIENTRY __glnop_GetMaterialiv(__GLcontext *gc, GLenum face, GLenum pname, GLint *params) {}
GLvoid GL_APIENTRY __glnop_GetPixelMapfv(__GLcontext *gc, GLenum map, GLfloat *values) {}
GLvoid GL_APIENTRY __glnop_GetPixelMapuiv(__GLcontext *gc, GLenum map, GLuint *values) {}
GLvoid GL_APIENTRY __glnop_GetPixelMapusv(__GLcontext *gc, GLenum map, GLushort *values) {}
GLvoid GL_APIENTRY __glnop_GetPolygonStipple(__GLcontext *gc, GLubyte *mask) {}
const GLubyte * GL_APIENTRY __glnop_GetString(__GLcontext *gc, GLenum name) {return gcvNULL;}
GLvoid GL_APIENTRY __glnop_GetTexEnvfv(__GLcontext *gc, GLenum target, GLenum pname, GLfloat *params) {}
GLvoid GL_APIENTRY __glnop_GetTexEnviv(__GLcontext *gc, GLenum target, GLenum pname, GLint *params) {}
GLvoid GL_APIENTRY __glnop_GetTexGendv(__GLcontext *gc, GLenum coord, GLenum pname, GLdouble *params) {}
GLvoid GL_APIENTRY __glnop_GetTexGenfv(__GLcontext *gc, GLenum coord, GLenum pname, GLfloat *params) {}
GLvoid GL_APIENTRY __glnop_GetTexGeniv(__GLcontext *gc, GLenum coord, GLenum pname, GLint *params) {}
GLvoid GL_APIENTRY __glnop_GetTexImage(__GLcontext *gc, GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels) {}
GLvoid GL_APIENTRY __glnop_GetTexParameterfv(__GLcontext *gc, GLenum target, GLenum pname, GLfloat *params) {}
GLvoid GL_APIENTRY __glnop_GetTexParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint *params) {}
GLvoid GL_APIENTRY __glnop_GetTexLevelParameterfv(__GLcontext *gc, GLenum target, GLint level, GLenum pname, GLfloat *params) {}
GLvoid GL_APIENTRY __glnop_GetTexLevelParameteriv(__GLcontext *gc, GLenum target, GLint level, GLenum pname, GLint *params) {}
GLboolean GL_APIENTRY __glnop_IsEnabled(__GLcontext *gc, GLenum cap) {return GL_FALSE;}
GLboolean GL_APIENTRY __glnop_IsList(__GLcontext *gc, GLuint list) {return GL_FALSE;}
GLvoid GL_APIENTRY __glnop_DepthRange(__GLcontext *gc, GLclampd near_val, GLclampd far_val) {}
GLvoid GL_APIENTRY __glnop_Frustum(__GLcontext *gc, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val) {}
GLvoid GL_APIENTRY __glnop_LoadIdentity(__GLcontext *gc) {}
GLvoid GL_APIENTRY __glnop_LoadMatrixf(__GLcontext *gc, const GLfloat *m) {}
GLvoid GL_APIENTRY __glnop_LoadMatrixd(__GLcontext *gc, const GLdouble *m) {}
GLvoid GL_APIENTRY __glnop_MatrixMode(__GLcontext *gc, GLenum mode) {}
GLvoid GL_APIENTRY __glnop_MultMatrixf(__GLcontext *gc, const GLfloat *m) {}
GLvoid GL_APIENTRY __glnop_MultMatrixd(__GLcontext *gc, const GLdouble *m) {}
GLvoid GL_APIENTRY __glnop_Ortho(__GLcontext *gc, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val) {}
GLvoid GL_APIENTRY __glnop_PopMatrix(__GLcontext *gc) {}
GLvoid GL_APIENTRY __glnop_PushMatrix(__GLcontext *gc) {}
GLvoid GL_APIENTRY __glnop_Rotated(__GLcontext *gc, GLdouble angle, GLdouble x, GLdouble y, GLdouble z) {}
GLvoid GL_APIENTRY __glnop_Rotatef(__GLcontext *gc, GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {}
GLvoid GL_APIENTRY __glnop_Scaled(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z) {}
GLvoid GL_APIENTRY __glnop_Scalef(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z) {}
GLvoid GL_APIENTRY __glnop_Translated(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z) {}
GLvoid GL_APIENTRY __glnop_Translatef(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z) {}
GLvoid GL_APIENTRY __glnop_Viewport(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height) {}
/* GL_VERSION_1_1 */
GLvoid GL_APIENTRY __glnop_ArrayElement(__GLcontext *gc, GLint i) {}
GLvoid GL_APIENTRY __glnop_BindTexture(__GLcontext *gc, GLenum target, GLuint texture) {}
GLvoid GL_APIENTRY __glnop_ColorPointer(__GLcontext *gc, GLint size, GLenum type, GLsizei stride, const GLvoid *ptr) {}
GLvoid GL_APIENTRY __glnop_DisableClientState(__GLcontext *gc, GLenum cap) {}
GLvoid GL_APIENTRY __glnop_DrawArrays(__GLcontext *gc, GLenum mode, GLint first, GLsizei count) {}
GLvoid GL_APIENTRY __glnop_DrawElements(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices) {}
GLvoid GL_APIENTRY __glnop_EdgeFlagPointer(__GLcontext *gc, GLsizei stride, const GLvoid *ptr) {}
GLvoid GL_APIENTRY __glnop_EnableClientState(__GLcontext *gc, GLenum cap) {}
GLvoid GL_APIENTRY __glnop_IndexPointer(__GLcontext *gc, GLenum type, GLsizei stride, const GLvoid *ptr) {}
GLvoid GL_APIENTRY __glnop_Indexub(__GLcontext *gc, GLubyte c) {}
GLvoid GL_APIENTRY __glnop_Indexubv(__GLcontext *gc, const GLubyte *c) {}
GLvoid GL_APIENTRY __glnop_InterleavedArrays(__GLcontext *gc, GLenum format, GLsizei stride, const GLvoid *pointer) {}
GLvoid GL_APIENTRY __glnop_NormalPointer(__GLcontext *gc, GLenum type, GLsizei stride, const GLvoid *ptr) {}
GLvoid GL_APIENTRY __glnop_PolygonOffset(__GLcontext *gc, GLfloat factor, GLfloat units) {}
GLvoid GL_APIENTRY __glnop_TexCoordPointer(__GLcontext *gc, GLint size, GLenum type, GLsizei stride, const GLvoid *ptr) {}
GLvoid GL_APIENTRY __glnop_VertexPointer(__GLcontext *gc, GLint size, GLenum type, GLsizei stride, const GLvoid *ptr) {}
GLboolean GL_APIENTRY __glnop_AreTexturesResident(__GLcontext *gc, GLsizei n, const GLuint *textures, GLboolean *residences) {return GL_FALSE;}
GLvoid GL_APIENTRY __glnop_CopyTexImage1D(__GLcontext *gc, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border) {}
GLvoid GL_APIENTRY __glnop_CopyTexImage2D(__GLcontext *gc, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {}
GLvoid GL_APIENTRY __glnop_CopyTexSubImage1D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width) {}
GLvoid GL_APIENTRY __glnop_CopyTexSubImage2D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {}
GLvoid GL_APIENTRY __glnop_DeleteTextures(__GLcontext *gc, GLsizei n, const GLuint *textures) {}
GLvoid GL_APIENTRY __glnop_GenTextures(__GLcontext *gc, GLsizei n, GLuint *textures) {}
GLvoid GL_APIENTRY __glnop_GetPointerv(__GLcontext *gc, GLenum pname, GLvoid **params) {}
GLboolean GL_APIENTRY __glnop_IsTexture(__GLcontext *gc, GLuint texture) {return GL_FALSE;}
GLvoid GL_APIENTRY __glnop_PrioritizeTextures(__GLcontext *gc, GLsizei n, const GLuint *textures, const GLclampf *priorities) {}
GLvoid GL_APIENTRY __glnop_TexSubImage1D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels) {}
GLvoid GL_APIENTRY __glnop_TexSubImage2D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels) {}
GLvoid GL_APIENTRY __glnop_PopClientAttrib(__GLcontext *gc) {}
GLvoid GL_APIENTRY __glnop_PushClientAttrib(__GLcontext *gc, GLbitfield mask) {}
/* GL_VERSION_1_2 */
GLvoid GL_APIENTRY __glnop_DrawRangeElements(__GLcontext *gc, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices) {}
GLvoid GL_APIENTRY __glnop_TexImage3D(__GLcontext *gc, GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels) {}
GLvoid GL_APIENTRY __glnop_TexSubImage3D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels) {}
GLvoid GL_APIENTRY __glnop_CopyTexSubImage3D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) {}
/* GL_VERSION_1_3 */
GLvoid GL_APIENTRY __glnop_ActiveTexture(__GLcontext *gc, GLenum texture) {}
GLvoid GL_APIENTRY __glnop_SampleCoverage(__GLcontext *gc, GLclampf value, GLboolean invert) {}
GLvoid GL_APIENTRY __glnop_CompressedTexImage3D(__GLcontext *gc, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data) {}
GLvoid GL_APIENTRY __glnop_CompressedTexImage2D(__GLcontext *gc, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data) {}
GLvoid GL_APIENTRY __glnop_CompressedTexImage1D(__GLcontext *gc, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data) {}
GLvoid GL_APIENTRY __glnop_CompressedTexSubImage3D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data) {}
GLvoid GL_APIENTRY __glnop_CompressedTexSubImage2D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data) {}
GLvoid GL_APIENTRY __glnop_CompressedTexSubImage1D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data) {}
GLvoid GL_APIENTRY __glnop_GetCompressedTexImage(__GLcontext *gc, GLenum target, GLint lod, GLvoid *img) {}
GLvoid GL_APIENTRY __glnop_ClientActiveTexture(__GLcontext *gc, GLenum texture) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord1d(__GLcontext *gc, GLenum target, GLdouble s) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord1dv(__GLcontext *gc, GLenum target, const GLdouble *v) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord1f(__GLcontext *gc, GLenum target, GLfloat s) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord1fv(__GLcontext *gc, GLenum target, const GLfloat *v) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord1i(__GLcontext *gc, GLenum target, GLint s) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord1iv(__GLcontext *gc, GLenum target, const GLint *v) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord1s(__GLcontext *gc, GLenum target, GLshort s) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord1sv(__GLcontext *gc, GLenum target, const GLshort *v) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord2d(__GLcontext *gc, GLenum target, GLdouble s, GLdouble t) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord2dv(__GLcontext *gc, GLenum target, const GLdouble *v) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord2f(__GLcontext *gc, GLenum target, GLfloat s, GLfloat t) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord2fv(__GLcontext *gc, GLenum target, const GLfloat *v) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord2i(__GLcontext *gc, GLenum target, GLint s, GLint t) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord2iv(__GLcontext *gc, GLenum target, const GLint *v) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord2s(__GLcontext *gc, GLenum target, GLshort s, GLshort t) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord2sv(__GLcontext *gc, GLenum target, const GLshort *v) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord3d(__GLcontext *gc, GLenum target, GLdouble s, GLdouble t, GLdouble r) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord3dv(__GLcontext *gc, GLenum target, const GLdouble *v) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord3f(__GLcontext *gc, GLenum target, GLfloat s, GLfloat t, GLfloat r) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord3fv(__GLcontext *gc, GLenum target, const GLfloat *v) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord3i(__GLcontext *gc, GLenum target, GLint s, GLint t, GLint r) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord3iv(__GLcontext *gc, GLenum target, const GLint *v) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord3s(__GLcontext *gc, GLenum target, GLshort s, GLshort t, GLshort r) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord3sv(__GLcontext *gc, GLenum target, const GLshort *v) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord4d(__GLcontext *gc, GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord4dv(__GLcontext *gc, GLenum target, const GLdouble *v) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord4f(__GLcontext *gc, GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord4fv(__GLcontext *gc, GLenum target, const GLfloat *v) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord4i(__GLcontext *gc, GLenum target, GLint s, GLint t, GLint r, GLint q) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord4iv(__GLcontext *gc, GLenum target, const GLint *v) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord4s(__GLcontext *gc, GLenum target, GLshort s, GLshort t, GLshort r, GLshort q) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoord4sv(__GLcontext *gc, GLenum target, const GLshort *v) {}
GLvoid GL_APIENTRY __glnop_LoadTransposeMatrixf(__GLcontext *gc, const GLfloat m[16]) {}
GLvoid GL_APIENTRY __glnop_LoadTransposeMatrixd(__GLcontext *gc, const GLdouble m[16]) {}
GLvoid GL_APIENTRY __glnop_MultTransposeMatrixf(__GLcontext *gc, const GLfloat m[16]) {}
GLvoid GL_APIENTRY __glnop_MultTransposeMatrixd(__GLcontext *gc, const GLdouble m[16]) {}
/* GL_VERSION_1_4 */
GLvoid GL_APIENTRY __glnop_BlendFuncSeparate(__GLcontext *gc, GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha) {}
GLvoid GL_APIENTRY __glnop_MultiDrawArrays(__GLcontext *gc, GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount) {}
GLvoid GL_APIENTRY __glnop_MultiDrawElements(__GLcontext *gc, GLenum mode, const GLsizei *count, GLenum type, const GLvoid *const*indices, GLsizei drawcount) {}
GLvoid GL_APIENTRY __glnop_PointParameterf(__GLcontext *gc, GLenum pname, GLfloat param) {}
GLvoid GL_APIENTRY __glnop_PointParameterfv(__GLcontext *gc, GLenum pname, const GLfloat *params) {}
GLvoid GL_APIENTRY __glnop_PointParameteri(__GLcontext *gc, GLenum pname, GLint param) {}
GLvoid GL_APIENTRY __glnop_PointParameteriv(__GLcontext *gc, GLenum pname, const GLint *params) {}
GLvoid GL_APIENTRY __glnop_FogCoordf(__GLcontext *gc, GLfloat coord) {}
GLvoid GL_APIENTRY __glnop_FogCoordfv(__GLcontext *gc, const GLfloat *coord) {}
GLvoid GL_APIENTRY __glnop_FogCoordd(__GLcontext *gc, GLdouble coord) {}
GLvoid GL_APIENTRY __glnop_FogCoorddv(__GLcontext *gc, const GLdouble *coord) {}
GLvoid GL_APIENTRY __glnop_FogCoordPointer(__GLcontext *gc, GLenum type, GLsizei stride, const GLvoid *pointer) {}
GLvoid GL_APIENTRY __glnop_SecondaryColor3b(__GLcontext *gc, GLbyte red, GLbyte green, GLbyte blue) {}
GLvoid GL_APIENTRY __glnop_SecondaryColor3bv(__GLcontext *gc, const GLbyte *v) {}
GLvoid GL_APIENTRY __glnop_SecondaryColor3d(__GLcontext *gc, GLdouble red, GLdouble green, GLdouble blue) {}
GLvoid GL_APIENTRY __glnop_SecondaryColor3dv(__GLcontext *gc, const GLdouble *v) {}
GLvoid GL_APIENTRY __glnop_SecondaryColor3f(__GLcontext *gc, GLfloat red, GLfloat green, GLfloat blue) {}
GLvoid GL_APIENTRY __glnop_SecondaryColor3fv(__GLcontext *gc, const GLfloat *v) {}
GLvoid GL_APIENTRY __glnop_SecondaryColor3i(__GLcontext *gc, GLint red, GLint green, GLint blue) {}
GLvoid GL_APIENTRY __glnop_SecondaryColor3iv(__GLcontext *gc, const GLint *v) {}
GLvoid GL_APIENTRY __glnop_SecondaryColor3s(__GLcontext *gc, GLshort red, GLshort green, GLshort blue) {}
GLvoid GL_APIENTRY __glnop_SecondaryColor3sv(__GLcontext *gc, const GLshort *v) {}
GLvoid GL_APIENTRY __glnop_SecondaryColor3ub(__GLcontext *gc, GLubyte red, GLubyte green, GLubyte blue) {}
GLvoid GL_APIENTRY __glnop_SecondaryColor3ubv(__GLcontext *gc, const GLubyte *v) {}
GLvoid GL_APIENTRY __glnop_SecondaryColor3ui(__GLcontext *gc, GLuint red, GLuint green, GLuint blue) {}
GLvoid GL_APIENTRY __glnop_SecondaryColor3uiv(__GLcontext *gc, const GLuint *v) {}
GLvoid GL_APIENTRY __glnop_SecondaryColor3us(__GLcontext *gc, GLushort red, GLushort green, GLushort blue) {}
GLvoid GL_APIENTRY __glnop_SecondaryColor3usv(__GLcontext *gc, const GLushort *v) {}
GLvoid GL_APIENTRY __glnop_SecondaryColorPointer(__GLcontext *gc, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) {}
GLvoid GL_APIENTRY __glnop_WindowPos2d(__GLcontext *gc, GLdouble x, GLdouble y) {}
GLvoid GL_APIENTRY __glnop_WindowPos2dv(__GLcontext *gc, const GLdouble *v) {}
GLvoid GL_APIENTRY __glnop_WindowPos2f(__GLcontext *gc, GLfloat x, GLfloat y) {}
GLvoid GL_APIENTRY __glnop_WindowPos2fv(__GLcontext *gc, const GLfloat *v) {}
GLvoid GL_APIENTRY __glnop_WindowPos2i(__GLcontext *gc, GLint x, GLint y) {}
GLvoid GL_APIENTRY __glnop_WindowPos2iv(__GLcontext *gc, const GLint *v) {}
GLvoid GL_APIENTRY __glnop_WindowPos2s(__GLcontext *gc, GLshort x, GLshort y) {}
GLvoid GL_APIENTRY __glnop_WindowPos2sv(__GLcontext *gc, const GLshort *v) {}
GLvoid GL_APIENTRY __glnop_WindowPos3d(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z) {}
GLvoid GL_APIENTRY __glnop_WindowPos3dv(__GLcontext *gc, const GLdouble *v) {}
GLvoid GL_APIENTRY __glnop_WindowPos3f(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z) {}
GLvoid GL_APIENTRY __glnop_WindowPos3fv(__GLcontext *gc, const GLfloat *v) {}
GLvoid GL_APIENTRY __glnop_WindowPos3i(__GLcontext *gc, GLint x, GLint y, GLint z) {}
GLvoid GL_APIENTRY __glnop_WindowPos3iv(__GLcontext *gc, const GLint *v) {}
GLvoid GL_APIENTRY __glnop_WindowPos3s(__GLcontext *gc, GLshort x, GLshort y, GLshort z) {}
GLvoid GL_APIENTRY __glnop_WindowPos3sv(__GLcontext *gc, const GLshort *v) {}
GLvoid GL_APIENTRY __glnop_BlendColor(__GLcontext *gc, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {}
GLvoid GL_APIENTRY __glnop_BlendEquation(__GLcontext *gc, GLenum mode) {}
/* GL_VERSION_1_5 */
GLvoid GL_APIENTRY __glnop_GenQueries(__GLcontext *gc, GLsizei n, GLuint *ids) {}
GLvoid GL_APIENTRY __glnop_DeleteQueries(__GLcontext *gc, GLsizei n, const GLuint *ids) {}
GLboolean GL_APIENTRY __glnop_IsQuery(__GLcontext *gc, GLuint id) {return GL_FALSE;}
GLvoid GL_APIENTRY __glnop_BeginQuery(__GLcontext *gc, GLenum target, GLuint id) {}
GLvoid GL_APIENTRY __glnop_EndQuery(__GLcontext *gc, GLenum target) {}
GLvoid GL_APIENTRY __glnop_GetQueryiv(__GLcontext *gc, GLenum target, GLenum pname, GLint *params) {}
GLvoid GL_APIENTRY __glnop_GetQueryObjectiv(__GLcontext *gc, GLuint id, GLenum pname, GLint *params) {}
GLvoid GL_APIENTRY __glnop_GetQueryObjectuiv(__GLcontext *gc, GLuint id, GLenum pname, GLuint *params) {}
GLvoid GL_APIENTRY __glnop_BindBuffer(__GLcontext *gc, GLenum target, GLuint buffer) {}
GLvoid GL_APIENTRY __glnop_DeleteBuffers(__GLcontext *gc, GLsizei n, const GLuint *buffers) {}
GLvoid GL_APIENTRY __glnop_GenBuffers(__GLcontext *gc, GLsizei n, GLuint *buffers) {}
GLboolean GL_APIENTRY __glnop_IsBuffer(__GLcontext *gc, GLuint buffer) {return GL_FALSE;}
GLvoid GL_APIENTRY __glnop_BufferData(__GLcontext *gc, GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage) {}
GLvoid GL_APIENTRY __glnop_BufferSubData(__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data) {}
GLvoid GL_APIENTRY __glnop_GetBufferSubData(__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data) {}
GLvoid* GL_APIENTRY __glnop_MapBuffer(__GLcontext *gc, GLenum target, GLenum access) {return gcvNULL;}
GLboolean GL_APIENTRY __glnop_UnmapBuffer(__GLcontext *gc, GLenum target) {return GL_FALSE;}
GLvoid GL_APIENTRY __glnop_GetBufferParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint *params) {}
GLvoid GL_APIENTRY __glnop_GetBufferPointerv(__GLcontext *gc, GLenum target, GLenum pname, GLvoid **params) {}
/* GL_VERSION_2_0 */
GLvoid GL_APIENTRY __glnop_BlendEquationSeparate(__GLcontext *gc, GLenum modeRGB, GLenum modeAlpha) {}
GLvoid GL_APIENTRY __glnop_DrawBuffers(__GLcontext *gc, GLsizei n, const GLenum *bufs) {}
GLvoid GL_APIENTRY __glnop_StencilOpSeparate(__GLcontext *gc, GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) {}
GLvoid GL_APIENTRY __glnop_StencilFuncSeparate(__GLcontext *gc, GLenum face, GLenum func, GLint ref, GLuint mask) {}
GLvoid GL_APIENTRY __glnop_StencilMaskSeparate(__GLcontext *gc, GLenum face, GLuint mask) {}
GLvoid GL_APIENTRY __glnop_AttachShader(__GLcontext *gc, GLuint program, GLuint shader) {}
GLvoid GL_APIENTRY __glnop_BindAttribLocation(__GLcontext *gc, GLuint program, GLuint index, const GLchar *name) {}
GLvoid GL_APIENTRY __glnop_CompileShader(__GLcontext *gc, GLuint shader) {}
GLuint GL_APIENTRY __glnop_CreateProgram(__GLcontext *gc) {return 0;}
GLuint GL_APIENTRY __glnop_CreateShader(__GLcontext *gc, GLenum type) {return 0;}
GLvoid GL_APIENTRY __glnop_DeleteProgram(__GLcontext *gc, GLuint program) {}
GLvoid GL_APIENTRY __glnop_DeleteShader(__GLcontext *gc, GLuint shader) {}
GLvoid GL_APIENTRY __glnop_DetachShader(__GLcontext *gc, GLuint program, GLuint shader) {}
GLvoid GL_APIENTRY __glnop_DisableVertexAttribArray(__GLcontext *gc, GLuint index) {}
GLvoid GL_APIENTRY __glnop_EnableVertexAttribArray(__GLcontext *gc, GLuint index) {}
GLvoid GL_APIENTRY __glnop_GetActiveAttrib(__GLcontext *gc, GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) {}
GLvoid GL_APIENTRY __glnop_GetActiveUniform(__GLcontext *gc, GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) {}
GLvoid GL_APIENTRY __glnop_GetAttachedShaders(__GLcontext *gc, GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders) {}
GLint GL_APIENTRY __glnop_GetAttribLocation(__GLcontext *gc, GLuint program, const GLchar *name) {return -1;}
GLvoid GL_APIENTRY __glnop_GetProgramiv(__GLcontext *gc, GLuint program, GLenum pname, GLint *params) {}
GLvoid GL_APIENTRY __glnop_GetProgramInfoLog(__GLcontext *gc, GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog) {}
GLvoid GL_APIENTRY __glnop_GetShaderiv(__GLcontext *gc, GLuint shader, GLenum pname, GLint *params) {}
GLvoid GL_APIENTRY __glnop_GetShaderInfoLog(__GLcontext *gc, GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) {}
GLvoid GL_APIENTRY __glnop_GetShaderSource(__GLcontext *gc, GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source) {}
GLint GL_APIENTRY __glnop_GetUniformLocation(__GLcontext *gc, GLuint program, const GLchar *name) {return -1;}
GLvoid GL_APIENTRY __glnop_GetUniformfv(__GLcontext *gc, GLuint program, GLint location, GLfloat *params) {}
GLvoid GL_APIENTRY __glnop_GetUniformiv(__GLcontext *gc, GLuint program, GLint location, GLint *params) {}
GLvoid GL_APIENTRY __glnop_GetVertexAttribdv(__GLcontext *gc, GLuint index, GLenum pname, GLdouble *params) {}
GLvoid GL_APIENTRY __glnop_GetVertexAttribfv(__GLcontext *gc, GLuint index, GLenum pname, GLfloat *params) {}
GLvoid GL_APIENTRY __glnop_GetVertexAttribiv(__GLcontext *gc, GLuint index, GLenum pname, GLint *params) {}
GLvoid GL_APIENTRY __glnop_GetVertexAttribPointerv(__GLcontext *gc, GLuint index, GLenum pname, GLvoid **pointer) {}
GLboolean GL_APIENTRY __glnop_IsProgram(__GLcontext *gc, GLuint program) {return GL_FALSE;}
GLboolean GL_APIENTRY __glnop_IsShader(__GLcontext *gc, GLuint shader) {return GL_FALSE;}
GLvoid GL_APIENTRY __glnop_LinkProgram(__GLcontext *gc, GLuint program) {}
GLvoid GL_APIENTRY __glnop_ShaderSource(__GLcontext *gc, GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length) {}
GLvoid GL_APIENTRY __glnop_UseProgram(__GLcontext *gc, GLuint program) {}
GLvoid GL_APIENTRY __glnop_Uniform1f(__GLcontext *gc, GLint location, GLfloat v0) {}
GLvoid GL_APIENTRY __glnop_Uniform2f(__GLcontext *gc, GLint location, GLfloat v0, GLfloat v1) {}
GLvoid GL_APIENTRY __glnop_Uniform3f(__GLcontext *gc, GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {}
GLvoid GL_APIENTRY __glnop_Uniform4f(__GLcontext *gc, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {}
GLvoid GL_APIENTRY __glnop_Uniform1i(__GLcontext *gc, GLint location, GLint v0) {}
GLvoid GL_APIENTRY __glnop_Uniform2i(__GLcontext *gc, GLint location, GLint v0, GLint v1) {}
GLvoid GL_APIENTRY __glnop_Uniform3i(__GLcontext *gc, GLint location, GLint v0, GLint v1, GLint v2) {}
GLvoid GL_APIENTRY __glnop_Uniform4i(__GLcontext *gc, GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {}
GLvoid GL_APIENTRY __glnop_Uniform1fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat *value) {}
GLvoid GL_APIENTRY __glnop_Uniform2fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat *value) {}
GLvoid GL_APIENTRY __glnop_Uniform3fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat *value) {}
GLvoid GL_APIENTRY __glnop_Uniform4fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat *value) {}
GLvoid GL_APIENTRY __glnop_Uniform1iv(__GLcontext *gc, GLint location, GLsizei count, const GLint *value) {}
GLvoid GL_APIENTRY __glnop_Uniform2iv(__GLcontext *gc, GLint location, GLsizei count, const GLint *value) {}
GLvoid GL_APIENTRY __glnop_Uniform3iv(__GLcontext *gc, GLint location, GLsizei count, const GLint *value) {}
GLvoid GL_APIENTRY __glnop_Uniform4iv(__GLcontext *gc, GLint location, GLsizei count, const GLint *value) {}
GLvoid GL_APIENTRY __glnop_UniformMatrix2fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {}
GLvoid GL_APIENTRY __glnop_UniformMatrix3fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {}
GLvoid GL_APIENTRY __glnop_UniformMatrix4fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {}
GLvoid GL_APIENTRY __glnop_ValidateProgram(__GLcontext *gc, GLuint program) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib1d(__GLcontext *gc, GLuint index, GLdouble x) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib1dv(__GLcontext *gc, GLuint index, const GLdouble *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib1f(__GLcontext *gc, GLuint index, GLfloat x) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib1fv(__GLcontext *gc, GLuint index, const GLfloat *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib1s(__GLcontext *gc, GLuint index, GLshort x) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib1sv(__GLcontext *gc, GLuint index, const GLshort *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib2d(__GLcontext *gc, GLuint index, GLdouble x, GLdouble y) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib2dv(__GLcontext *gc, GLuint index, const GLdouble *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib2f(__GLcontext *gc, GLuint index, GLfloat x, GLfloat y) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib2fv(__GLcontext *gc, GLuint index, const GLfloat *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib2s(__GLcontext *gc, GLuint index, GLshort x, GLshort y) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib2sv(__GLcontext *gc, GLuint index, const GLshort *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib3d(__GLcontext *gc, GLuint index, GLdouble x, GLdouble y, GLdouble z) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib3dv(__GLcontext *gc, GLuint index, const GLdouble *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib3f(__GLcontext *gc, GLuint index, GLfloat x, GLfloat y, GLfloat z) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib3fv(__GLcontext *gc, GLuint index, const GLfloat *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib3s(__GLcontext *gc, GLuint index, GLshort x, GLshort y, GLshort z) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib3sv(__GLcontext *gc, GLuint index, const GLshort *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib4Nbv(__GLcontext *gc, GLuint index, const GLbyte *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib4Niv(__GLcontext *gc, GLuint index, const GLint *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib4Nsv(__GLcontext *gc, GLuint index, const GLshort *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib4Nub(__GLcontext *gc, GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib4Nubv(__GLcontext *gc, GLuint index, const GLubyte *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib4Nuiv(__GLcontext *gc, GLuint index, const GLuint *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib4Nusv(__GLcontext *gc, GLuint index, const GLushort *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib4bv(__GLcontext *gc, GLuint index, const GLbyte *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib4d(__GLcontext *gc, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib4dv(__GLcontext *gc, GLuint index, const GLdouble *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib4f(__GLcontext *gc, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib4fv(__GLcontext *gc, GLuint index, const GLfloat *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib4iv(__GLcontext *gc, GLuint index, const GLint *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib4s(__GLcontext *gc, GLuint index, GLshort x, GLshort y, GLshort z, GLshort w) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib4sv(__GLcontext *gc, GLuint index, const GLshort *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib4ubv(__GLcontext *gc, GLuint index, const GLubyte *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib4uiv(__GLcontext *gc, GLuint index, const GLuint *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttrib4usv(__GLcontext *gc, GLuint index, const GLushort *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttribPointer(__GLcontext *gc, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer) {}
/* GL_VERSION_2_1 */
GLvoid GL_APIENTRY __glnop_UniformMatrix2x3fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {}
GLvoid GL_APIENTRY __glnop_UniformMatrix3x2fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {}
GLvoid GL_APIENTRY __glnop_UniformMatrix2x4fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {}
GLvoid GL_APIENTRY __glnop_UniformMatrix4x2fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {}
GLvoid GL_APIENTRY __glnop_UniformMatrix3x4fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {}
GLvoid GL_APIENTRY __glnop_UniformMatrix4x3fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {}
/* GL_VERSION_3_0 */
GLvoid GL_APIENTRY __glnop_ColorMaski(__GLcontext *gc, GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a) {}
GLvoid GL_APIENTRY __glnop_GetBooleani_v(__GLcontext *gc, GLenum target, GLuint index, GLboolean *data) {}
GLvoid GL_APIENTRY __glnop_GetIntegeri_v(__GLcontext *gc, GLenum target, GLuint index, GLint *data) {}
GLvoid GL_APIENTRY __glnop_Enablei(__GLcontext *gc, GLenum target, GLuint index) {}
GLvoid GL_APIENTRY __glnop_Disablei(__GLcontext *gc, GLenum target, GLuint index) {}
GLboolean GL_APIENTRY __glnop_IsEnabledi(__GLcontext *gc, GLenum target, GLuint index) {return GL_FALSE;}
GLvoid GL_APIENTRY __glnop_BeginTransformFeedback(__GLcontext *gc, GLenum primitiveMode) {}
GLvoid GL_APIENTRY __glnop_EndTransformFeedback(__GLcontext *gc) {}
GLvoid GL_APIENTRY __glnop_BindBufferRange(__GLcontext *gc, GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) {}
GLvoid GL_APIENTRY __glnop_BindBufferBase(__GLcontext *gc, GLenum target, GLuint index, GLuint buffer) {}
GLvoid GL_APIENTRY __glnop_TransformFeedbackVaryings(__GLcontext *gc, GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode) {}
GLvoid GL_APIENTRY __glnop_GetTransformFeedbackVarying(__GLcontext *gc, GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name) {}
GLvoid GL_APIENTRY __glnop_ClampColor(__GLcontext *gc, GLenum target, GLenum clamp) {}
GLvoid GL_APIENTRY __glnop_BeginConditionalRender(__GLcontext *gc, GLuint id, GLenum mode) {}
GLvoid GL_APIENTRY __glnop_EndConditionalRender(__GLcontext *gc) {}
GLvoid GL_APIENTRY __glnop_VertexAttribIPointer(__GLcontext *gc, GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) {}
GLvoid GL_APIENTRY __glnop_GetVertexAttribIiv(__GLcontext *gc, GLuint index, GLenum pname, GLint *params) {}
GLvoid GL_APIENTRY __glnop_GetVertexAttribIuiv(__GLcontext *gc, GLuint index, GLenum pname, GLuint *params) {}
GLvoid GL_APIENTRY __glnop_VertexAttribI1i(__GLcontext *gc, GLuint index, GLint x) {}
GLvoid GL_APIENTRY __glnop_VertexAttribI2i(__GLcontext *gc, GLuint index, GLint x, GLint y) {}
GLvoid GL_APIENTRY __glnop_VertexAttribI3i(__GLcontext *gc, GLuint index, GLint x, GLint y, GLint z) {}
GLvoid GL_APIENTRY __glnop_VertexAttribI4i(__GLcontext *gc, GLuint index, GLint x, GLint y, GLint z, GLint w) {}
GLvoid GL_APIENTRY __glnop_VertexAttribI1ui(__GLcontext *gc, GLuint index, GLuint x) {}
GLvoid GL_APIENTRY __glnop_VertexAttribI2ui(__GLcontext *gc, GLuint index, GLuint x, GLuint y) {}
GLvoid GL_APIENTRY __glnop_VertexAttribI3ui(__GLcontext *gc, GLuint index, GLuint x, GLuint y, GLuint z) {}
GLvoid GL_APIENTRY __glnop_VertexAttribI4ui(__GLcontext *gc, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w) {}
GLvoid GL_APIENTRY __glnop_VertexAttribI1iv(__GLcontext *gc, GLuint index, const GLint *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttribI2iv(__GLcontext *gc, GLuint index, const GLint *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttribI3iv(__GLcontext *gc, GLuint index, const GLint *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttribI4iv(__GLcontext *gc, GLuint index, const GLint *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttribI1uiv(__GLcontext *gc, GLuint index, const GLuint *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttribI2uiv(__GLcontext *gc, GLuint index, const GLuint *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttribI3uiv(__GLcontext *gc, GLuint index, const GLuint *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttribI4uiv(__GLcontext *gc, GLuint index, const GLuint *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttribI4bv(__GLcontext *gc, GLuint index, const GLbyte *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttribI4sv(__GLcontext *gc, GLuint index, const GLshort *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttribI4ubv(__GLcontext *gc, GLuint index, const GLubyte *v) {}
GLvoid GL_APIENTRY __glnop_VertexAttribI4usv(__GLcontext *gc, GLuint index, const GLushort *v) {}
GLvoid GL_APIENTRY __glnop_GetUniformuiv(__GLcontext *gc, GLuint program, GLint location, GLuint *params) {}
GLvoid GL_APIENTRY __glnop_BindFragDataLocation(__GLcontext *gc, GLuint program, GLuint color, const GLchar *name) {}
GLint GL_APIENTRY __glnop_GetFragDataLocation(__GLcontext *gc, GLuint program, const GLchar *name) {return -1;}
GLvoid GL_APIENTRY __glnop_Uniform1ui(__GLcontext *gc, GLint location, GLuint v0) {}
GLvoid GL_APIENTRY __glnop_Uniform2ui(__GLcontext *gc, GLint location, GLuint v0, GLuint v1) {}
GLvoid GL_APIENTRY __glnop_Uniform3ui(__GLcontext *gc, GLint location, GLuint v0, GLuint v1, GLuint v2) {}
GLvoid GL_APIENTRY __glnop_Uniform4ui(__GLcontext *gc, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) {}
GLvoid GL_APIENTRY __glnop_Uniform1uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint *value) {}
GLvoid GL_APIENTRY __glnop_Uniform2uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint *value) {}
GLvoid GL_APIENTRY __glnop_Uniform3uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint *value) {}
GLvoid GL_APIENTRY __glnop_Uniform4uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint *value) {}
GLvoid GL_APIENTRY __glnop_TexParameterIiv(__GLcontext *gc, GLenum target, GLenum pname, const GLint *params) {}
GLvoid GL_APIENTRY __glnop_TexParameterIuiv(__GLcontext *gc, GLenum target, GLenum pname, const GLuint *params) {}
GLvoid GL_APIENTRY __glnop_GetTexParameterIiv(__GLcontext *gc, GLenum target, GLenum pname, GLint *params) {}
GLvoid GL_APIENTRY __glnop_GetTexParameterIuiv(__GLcontext *gc, GLenum target, GLenum pname, GLuint *params) {}
GLvoid GL_APIENTRY __glnop_ClearBufferiv(__GLcontext *gc, GLenum buffer, GLint drawbuffer, const GLint *value) {}
GLvoid GL_APIENTRY __glnop_ClearBufferuiv(__GLcontext *gc, GLenum buffer, GLint drawbuffer, const GLuint *value) {}
GLvoid GL_APIENTRY __glnop_ClearBufferfv(__GLcontext *gc, GLenum buffer, GLint drawbuffer, const GLfloat *value) {}
GLvoid GL_APIENTRY __glnop_ClearBufferfi(__GLcontext *gc, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) {}
const GLubyte *GL_APIENTRY __glnop_GetStringi(__GLcontext *gc, GLenum name, GLuint index) {return gcvNULL;}
GLboolean GL_APIENTRY __glnop_IsRenderbuffer(__GLcontext *gc, GLuint renderbuffer) {return GL_FALSE;}
GLvoid GL_APIENTRY __glnop_BindRenderbuffer(__GLcontext *gc, GLenum target, GLuint renderbuffer) {}
GLvoid GL_APIENTRY __glnop_DeleteRenderbuffers(__GLcontext *gc, GLsizei n, const GLuint *renderbuffers) {}
GLvoid GL_APIENTRY __glnop_GenRenderbuffers(__GLcontext *gc, GLsizei n, GLuint *renderbuffers) {}
GLvoid GL_APIENTRY __glnop_RenderbufferStorage(__GLcontext *gc, GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {}
GLvoid GL_APIENTRY __glnop_GetRenderbufferParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint *params) {}
GLboolean GL_APIENTRY __glnop_IsFramebuffer(__GLcontext *gc, GLuint framebuffer) {return GL_FALSE;}
GLvoid GL_APIENTRY __glnop_BindFramebuffer(__GLcontext *gc, GLenum target, GLuint framebuffer) {}
GLvoid GL_APIENTRY __glnop_DeleteFramebuffers(__GLcontext *gc, GLsizei n, const GLuint *framebuffers) {}
GLvoid GL_APIENTRY __glnop_GenFramebuffers(__GLcontext *gc, GLsizei n, GLuint *framebuffers) {}
GLenum GL_APIENTRY __glnop_CheckFramebufferStatus(__GLcontext *gc, GLenum target) {return GL_FRAMEBUFFER_COMPLETE;}
GLvoid GL_APIENTRY __glnop_FramebufferTexture1D(__GLcontext *gc, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {}
GLvoid GL_APIENTRY __glnop_FramebufferTexture2D(__GLcontext *gc, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {}
GLvoid GL_APIENTRY __glnop_FramebufferTexture3D(__GLcontext *gc, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset) {}
GLvoid GL_APIENTRY __glnop_FramebufferRenderbuffer(__GLcontext *gc, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {}
GLvoid GL_APIENTRY __glnop_GetFramebufferAttachmentParameteriv(__GLcontext *gc, GLenum target, GLenum attachment, GLenum pname, GLint *params) {}
GLvoid GL_APIENTRY __glnop_GenerateMipmap(__GLcontext *gc, GLenum target) {}
GLvoid GL_APIENTRY __glnop_BlitFramebuffer(__GLcontext *gc, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) {}
GLvoid GL_APIENTRY __glnop_RenderbufferStorageMultisample(__GLcontext *gc, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) {}
GLvoid GL_APIENTRY __glnop_FramebufferTextureLayer(__GLcontext *gc, GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer) {}
GLvoid* GL_APIENTRY __glnop_MapBufferRange(__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access) {return gcvNULL;}
GLvoid GL_APIENTRY __glnop_FlushMappedBufferRange(__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr length) {}
GLvoid GL_APIENTRY __glnop_BindVertexArray(__GLcontext *gc, GLuint array) {}
GLvoid GL_APIENTRY __glnop_DeleteVertexArrays(__GLcontext *gc, GLsizei n, const GLuint *arrays) {}
GLvoid GL_APIENTRY __glnop_GenVertexArrays(__GLcontext *gc, GLsizei n, GLuint *arrays) {}
GLboolean GL_APIENTRY __glnop_IsVertexArray(__GLcontext *gc, GLuint array) {return GL_FALSE;}
/* GL_VERSION_3_1 */
GLvoid GL_APIENTRY __glnop_DrawArraysInstanced(__GLcontext *gc, GLenum mode, GLint first, GLsizei count, GLsizei instancecount) {}
GLvoid GL_APIENTRY __glnop_DrawElementsInstanced(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei instancecount) {}
GLvoid GL_APIENTRY __glnop_TexBuffer(__GLcontext *gc, GLenum target, GLenum internalformat, GLuint buffer) {}
GLvoid GL_APIENTRY __glnop_PrimitiveRestartIndex(__GLcontext *gc, GLuint index) {}
GLvoid GL_APIENTRY __glnop_CopyBufferSubData(__GLcontext *gc, GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size) {}
GLvoid GL_APIENTRY __glnop_GetUniformIndices(__GLcontext *gc, GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices) {}
GLvoid GL_APIENTRY __glnop_GetActiveUniformsiv(__GLcontext *gc, GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params) {}
GLvoid GL_APIENTRY __glnop_GetActiveUniformName(__GLcontext *gc, GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName) {}
GLuint GL_APIENTRY __glnop_GetUniformBlockIndex(__GLcontext *gc, GLuint program, const GLchar *uniformBlockName) {return 0;}
GLvoid GL_APIENTRY __glnop_GetActiveUniformBlockiv(__GLcontext *gc, GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params) {}
GLvoid GL_APIENTRY __glnop_GetActiveUniformBlockName(__GLcontext *gc, GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName) {}
GLvoid GL_APIENTRY __glnop_UniformBlockBinding(__GLcontext *gc, GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding) {}
/* GL_VERSION_3_2 */
GLvoid GL_APIENTRY __glnop_DrawElementsBaseVertex(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLint basevertex) {}
GLvoid GL_APIENTRY __glnop_DrawRangeElementsBaseVertex(__GLcontext *gc, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices, GLint basevertex) {}
GLvoid GL_APIENTRY __glnop_DrawElementsInstancedBaseVertex(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei instancecount, GLint basevertex) {}
GLvoid GL_APIENTRY __glnop_MultiDrawElementsBaseVertex(__GLcontext *gc, GLenum mode, const GLsizei *count, GLenum type, const GLvoid *const*indices, GLsizei drawcount, const GLint *basevertex) {}
GLvoid GL_APIENTRY __glnop_ProvokingVertex(__GLcontext *gc, GLenum mode) {}
GLsync GL_APIENTRY __glnop_FenceSync(__GLcontext *gc, GLenum condition, GLbitfield flags) {return gcvNULL;}
GLboolean GL_APIENTRY __glnop_IsSync(__GLcontext *gc, GLsync sync) {return GL_FALSE;}
GLvoid GL_APIENTRY __glnop_DeleteSync(__GLcontext *gc, GLsync sync) {}
GLenum GL_APIENTRY __glnop_ClientWaitSync(__GLcontext *gc, GLsync sync, GLbitfield flags, GLuint64 timeout) {return GL_WAIT_FAILED;}
GLvoid GL_APIENTRY __glnop_WaitSync(__GLcontext *gc, GLsync sync, GLbitfield flags, GLuint64 timeout) {}
GLvoid GL_APIENTRY __glnop_GetInteger64v(__GLcontext *gc, GLenum pname, GLint64 *data) {}
GLvoid GL_APIENTRY __glnop_GetSynciv(__GLcontext *gc, GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values) {}
GLvoid GL_APIENTRY __glnop_GetInteger64i_v(__GLcontext *gc, GLenum target, GLuint index, GLint64 *data) {}
GLvoid GL_APIENTRY __glnop_GetBufferParameteri64v(__GLcontext *gc, GLenum target, GLenum pname, GLint64 *params) {}
GLvoid GL_APIENTRY __glnop_FramebufferTexture(__GLcontext *gc, GLenum target, GLenum attachment, GLuint texture, GLint level) {}
GLvoid GL_APIENTRY __glnop_TexImage2DMultisample(__GLcontext *gc, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations) {}
GLvoid GL_APIENTRY __glnop_TexImage3DMultisample(__GLcontext *gc, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations) {}
GLvoid GL_APIENTRY __glnop_GetMultisamplefv(__GLcontext *gc, GLenum pname, GLuint index, GLfloat *val) {}
GLvoid GL_APIENTRY __glnop_SampleMaski(__GLcontext *gc, GLuint maskNumber, GLbitfield mask) {}
/* GL_VERSION_3_3 */
GLvoid GL_APIENTRY __glnop_BindFragDataLocationIndexed(__GLcontext *gc, GLuint program, GLuint colorNumber, GLuint index, const GLchar *name) {}
GLint GL_APIENTRY __glnop_GetFragDataIndex(__GLcontext *gc, GLuint program, const GLchar *name) {return -1;}
GLvoid GL_APIENTRY __glnop_GenSamplers(__GLcontext *gc, GLsizei count, GLuint *samplers) {}
GLvoid GL_APIENTRY __glnop_DeleteSamplers(__GLcontext *gc, GLsizei count, const GLuint *samplers) {}
GLboolean GL_APIENTRY __glnop_IsSampler(__GLcontext *gc, GLuint sampler) {return GL_FALSE;}
GLvoid GL_APIENTRY __glnop_BindSampler(__GLcontext *gc, GLuint unit, GLuint sampler) {}
GLvoid GL_APIENTRY __glnop_SamplerParameteri(__GLcontext *gc, GLuint sampler, GLenum pname, GLint param) {}
GLvoid GL_APIENTRY __glnop_SamplerParameteriv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLint *param) {}
GLvoid GL_APIENTRY __glnop_SamplerParameterf(__GLcontext *gc, GLuint sampler, GLenum pname, GLfloat param) {}
GLvoid GL_APIENTRY __glnop_SamplerParameterfv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLfloat *param) {}
GLvoid GL_APIENTRY __glnop_SamplerParameterIiv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLint *param) {}
GLvoid GL_APIENTRY __glnop_SamplerParameterIuiv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLuint *param) {}
GLvoid GL_APIENTRY __glnop_GetSamplerParameteriv(__GLcontext *gc, GLuint sampler, GLenum pname, GLint *params) {}
GLvoid GL_APIENTRY __glnop_GetSamplerParameterIiv(__GLcontext *gc, GLuint sampler, GLenum pname, GLint *params) {}
GLvoid GL_APIENTRY __glnop_GetSamplerParameterfv(__GLcontext *gc, GLuint sampler, GLenum pname, GLfloat *params) {}
GLvoid GL_APIENTRY __glnop_GetSamplerParameterIuiv(__GLcontext *gc, GLuint sampler, GLenum pname, GLuint *params) {}
GLvoid GL_APIENTRY __glnop_QueryCounter(__GLcontext *gc, GLuint id, GLenum target) {}
GLvoid GL_APIENTRY __glnop_GetQueryObjecti64v(__GLcontext *gc, GLuint id, GLenum pname, GLint64 *params) {}
GLvoid GL_APIENTRY __glnop_GetQueryObjectui64v(__GLcontext *gc, GLuint id, GLenum pname, GLuint64 *params) {}
GLvoid GL_APIENTRY __glnop_VertexAttribDivisor(__GLcontext *gc, GLuint index, GLuint divisor) {}
GLvoid GL_APIENTRY __glnop_VertexAttribP1ui(__GLcontext *gc, GLuint index, GLenum type, GLboolean normalized, GLuint value) {}
GLvoid GL_APIENTRY __glnop_VertexAttribP1uiv(__GLcontext *gc, GLuint index, GLenum type, GLboolean normalized, const GLuint *value) {}
GLvoid GL_APIENTRY __glnop_VertexAttribP2ui(__GLcontext *gc, GLuint index, GLenum type, GLboolean normalized, GLuint value) {}
GLvoid GL_APIENTRY __glnop_VertexAttribP2uiv(__GLcontext *gc, GLuint index, GLenum type, GLboolean normalized, const GLuint *value) {}
GLvoid GL_APIENTRY __glnop_VertexAttribP3ui(__GLcontext *gc, GLuint index, GLenum type, GLboolean normalized, GLuint value) {}
GLvoid GL_APIENTRY __glnop_VertexAttribP3uiv(__GLcontext *gc, GLuint index, GLenum type, GLboolean normalized, const GLuint *value) {}
GLvoid GL_APIENTRY __glnop_VertexAttribP4ui(__GLcontext *gc, GLuint index, GLenum type, GLboolean normalized, GLuint value) {}
GLvoid GL_APIENTRY __glnop_VertexAttribP4uiv(__GLcontext *gc, GLuint index, GLenum type, GLboolean normalized, const GLuint *value) {}
GLvoid GL_APIENTRY __glnop_VertexP2ui(__GLcontext *gc, GLenum type, GLuint value) {}
GLvoid GL_APIENTRY __glnop_VertexP2uiv(__GLcontext *gc, GLenum type, const GLuint *value) {}
GLvoid GL_APIENTRY __glnop_VertexP3ui(__GLcontext *gc, GLenum type, GLuint value) {}
GLvoid GL_APIENTRY __glnop_VertexP3uiv(__GLcontext *gc, GLenum type, const GLuint *value) {}
GLvoid GL_APIENTRY __glnop_VertexP4ui(__GLcontext *gc, GLenum type, GLuint value) {}
GLvoid GL_APIENTRY __glnop_VertexP4uiv(__GLcontext *gc, GLenum type, const GLuint *value) {}
GLvoid GL_APIENTRY __glnop_TexCoordP1ui(__GLcontext *gc, GLenum type, GLuint coords) {}
GLvoid GL_APIENTRY __glnop_TexCoordP1uiv(__GLcontext *gc, GLenum type, const GLuint *coords) {}
GLvoid GL_APIENTRY __glnop_TexCoordP2ui(__GLcontext *gc, GLenum type, GLuint coords) {}
GLvoid GL_APIENTRY __glnop_TexCoordP2uiv(__GLcontext *gc, GLenum type, const GLuint *coords) {}
GLvoid GL_APIENTRY __glnop_TexCoordP3ui(__GLcontext *gc, GLenum type, GLuint coords) {}
GLvoid GL_APIENTRY __glnop_TexCoordP3uiv(__GLcontext *gc, GLenum type, const GLuint *coords) {}
GLvoid GL_APIENTRY __glnop_TexCoordP4ui(__GLcontext *gc, GLenum type, GLuint coords) {}
GLvoid GL_APIENTRY __glnop_TexCoordP4uiv(__GLcontext *gc, GLenum type, const GLuint *coords) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoordP1ui(__GLcontext *gc, GLenum texture, GLenum type, GLuint coords) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoordP1uiv(__GLcontext *gc, GLenum texture, GLenum type, const GLuint *coords) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoordP2ui(__GLcontext *gc, GLenum texture, GLenum type, GLuint coords) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoordP2uiv(__GLcontext *gc, GLenum texture, GLenum type, const GLuint *coords) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoordP3ui(__GLcontext *gc, GLenum texture, GLenum type, GLuint coords) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoordP3uiv(__GLcontext *gc, GLenum texture, GLenum type, const GLuint *coords) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoordP4ui(__GLcontext *gc, GLenum texture, GLenum type, GLuint coords) {}
GLvoid GL_APIENTRY __glnop_MultiTexCoordP4uiv(__GLcontext *gc, GLenum texture, GLenum type, const GLuint *coords) {}
GLvoid GL_APIENTRY __glnop_NormalP3ui(__GLcontext *gc, GLenum type, GLuint coords) {}
GLvoid GL_APIENTRY __glnop_NormalP3uiv(__GLcontext *gc, GLenum type, const GLuint *coords) {}
GLvoid GL_APIENTRY __glnop_ColorP3ui(__GLcontext *gc, GLenum type, GLuint color) {}
GLvoid GL_APIENTRY __glnop_ColorP3uiv(__GLcontext *gc, GLenum type, const GLuint *color) {}
GLvoid GL_APIENTRY __glnop_ColorP4ui(__GLcontext *gc, GLenum type, GLuint color) {}
GLvoid GL_APIENTRY __glnop_ColorP4uiv(__GLcontext *gc, GLenum type, const GLuint *color) {}
GLvoid GL_APIENTRY __glnop_SecondaryColorP3ui(__GLcontext *gc, GLenum type, GLuint color) {}
GLvoid GL_APIENTRY __glnop_SecondaryColorP3uiv(__GLcontext *gc, GLenum type, const GLuint *color) {}
/* GL_VERSION_4_0 */
GLvoid GL_APIENTRY __glnop_MinSampleShading(__GLcontext *gc, GLfloat value) {}
GLvoid GL_APIENTRY __glnop_BlendEquationi(__GLcontext *gc, GLuint buf, GLenum mode) {}
GLvoid GL_APIENTRY __glnop_BlendEquationSeparatei(__GLcontext *gc, GLuint buf, GLenum modeRGB, GLenum modeAlpha) {}
GLvoid GL_APIENTRY __glnop_BlendFunci(__GLcontext *gc, GLuint buf, GLenum src, GLenum dst) {}
GLvoid GL_APIENTRY __glnop_BlendFuncSeparatei(__GLcontext *gc, GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) {}
GLvoid GL_APIENTRY __glnop_DrawArraysIndirect(__GLcontext *gc, GLenum mode, const GLvoid *indirect) {}
GLvoid GL_APIENTRY __glnop_DrawElementsIndirect(__GLcontext *gc, GLenum mode, GLenum type, const GLvoid *indirect) {}
GLvoid GL_APIENTRY __glnop_Uniform1d(__GLcontext *gc, GLint location, GLdouble x) {}
GLvoid GL_APIENTRY __glnop_Uniform2d(__GLcontext *gc, GLint location, GLdouble x, GLdouble y) {}
GLvoid GL_APIENTRY __glnop_Uniform3d(__GLcontext *gc, GLint location, GLdouble x, GLdouble y, GLdouble z) {}
GLvoid GL_APIENTRY __glnop_Uniform4d(__GLcontext *gc, GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {}
GLvoid GL_APIENTRY __glnop_Uniform1dv(__GLcontext *gc, GLint location, GLsizei count, const GLdouble *value) {}
GLvoid GL_APIENTRY __glnop_Uniform2dv(__GLcontext *gc, GLint location, GLsizei count, const GLdouble *value) {}
GLvoid GL_APIENTRY __glnop_Uniform3dv(__GLcontext *gc, GLint location, GLsizei count, const GLdouble *value) {}
GLvoid GL_APIENTRY __glnop_Uniform4dv(__GLcontext *gc, GLint location, GLsizei count, const GLdouble *value) {}
GLvoid GL_APIENTRY __glnop_UniformMatrix2dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) {}
GLvoid GL_APIENTRY __glnop_UniformMatrix3dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) {}
GLvoid GL_APIENTRY __glnop_UniformMatrix4dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) {}
GLvoid GL_APIENTRY __glnop_UniformMatrix2x3dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) {}
GLvoid GL_APIENTRY __glnop_UniformMatrix2x4dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) {}
GLvoid GL_APIENTRY __glnop_UniformMatrix3x2dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) {}
GLvoid GL_APIENTRY __glnop_UniformMatrix3x4dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) {}
GLvoid GL_APIENTRY __glnop_UniformMatrix4x2dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) {}
GLvoid GL_APIENTRY __glnop_UniformMatrix4x3dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) {}
GLvoid GL_APIENTRY __glnop_GetUniformdv(__GLcontext *gc, GLuint program, GLint location, GLdouble *params) {}
GLint GL_APIENTRY __glnop_GetSubroutineUniformLocation(__GLcontext *gc, GLuint program, GLenum shadertype, const GLchar *name) {return -1;}
GLuint GL_APIENTRY __glnop_GetSubroutineIndex(__GLcontext *gc, GLuint program, GLenum shadertype, const GLchar *name) {return 0;}
GLvoid GL_APIENTRY __glnop_GetActiveSubroutineUniformiv(__GLcontext *gc, GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint *values) {}
GLvoid GL_APIENTRY __glnop_GetActiveSubroutineUniformName(__GLcontext *gc, GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name) {}
GLvoid GL_APIENTRY __glnop_GetActiveSubroutineName(__GLcontext *gc, GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name) {}
GLvoid GL_APIENTRY __glnop_UniformSubroutinesuiv(__GLcontext *gc, GLenum shadertype, GLsizei count, const GLuint *indices) {}
GLvoid GL_APIENTRY __glnop_GetUniformSubroutineuiv(__GLcontext *gc, GLenum shadertype, GLint location, GLuint *params) {}
GLvoid GL_APIENTRY __glnop_GetProgramStageiv(__GLcontext *gc, GLuint program, GLenum shadertype, GLenum pname, GLint *values) {}
GLvoid GL_APIENTRY __glnop_PatchParameteri(__GLcontext *gc, GLenum pname, GLint value) {}
GLvoid GL_APIENTRY __glnop_PatchParameterfv(__GLcontext *gc, GLenum pname, const GLfloat *values) {}
GLvoid GL_APIENTRY __glnop_BindTransformFeedback(__GLcontext *gc, GLenum target, GLuint id) {}
GLvoid GL_APIENTRY __glnop_DeleteTransformFeedbacks(__GLcontext *gc, GLsizei n, const GLuint *ids) {}
GLvoid GL_APIENTRY __glnop_GenTransformFeedbacks(__GLcontext *gc, GLsizei n, GLuint *ids) {}
GLboolean GL_APIENTRY __glnop_IsTransformFeedback(__GLcontext *gc, GLuint id) {return GL_FALSE;}
GLvoid GL_APIENTRY __glnop_PauseTransformFeedback(__GLcontext *gc) {}
GLvoid GL_APIENTRY __glnop_ResumeTransformFeedback(__GLcontext *gc) {}
GLvoid GL_APIENTRY __glnop_DrawTransformFeedback(__GLcontext *gc, GLenum mode, GLuint id) {}
GLvoid GL_APIENTRY __glnop_DrawTransformFeedbackStream(__GLcontext *gc, GLenum mode, GLuint id, GLuint stream) {}
GLvoid GL_APIENTRY __glnop_BeginQueryIndexed(__GLcontext *gc, GLenum target, GLuint index, GLuint id) {}
GLvoid GL_APIENTRY __glnop_EndQueryIndexed(__GLcontext *gc, GLenum target, GLuint index) {}
GLvoid GL_APIENTRY __glnop_GetQueryIndexediv(__GLcontext *gc, GLenum target, GLuint index, GLenum pname, GLint *params) {}
/* GL_VERSION_4_1, incomplete: defined by later GL version but required by ES */
GLvoid GL_APIENTRY __glnop_ReleaseShaderCompiler(__GLcontext *gc) {}
GLvoid GL_APIENTRY __glnop_ShaderBinary(__GLcontext *gc, GLsizei count, const GLuint *shaders, GLenum binaryformat, const GLvoid *binary, GLsizei length) {}
GLvoid GL_APIENTRY __glnop_GetShaderPrecisionFormat(__GLcontext *gc, GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision) {}
GLvoid GL_APIENTRY __glnop_DepthRangef(__GLcontext *gc, GLfloat n, GLfloat f) {}
GLvoid GL_APIENTRY __glnop_ClearDepthf(__GLcontext *gc, GLfloat d) {}
GLvoid GL_APIENTRY __glnop_GetProgramBinary(__GLcontext *gc, GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, GLvoid *binary) {}
GLvoid GL_APIENTRY __glnop_ProgramBinary(__GLcontext *gc, GLuint program, GLenum binaryFormat, const GLvoid *binary, GLsizei length) {}
GLvoid GL_APIENTRY __glnop_ProgramParameteri(__GLcontext *gc, GLuint program, GLenum pname, GLint value) {}
GLvoid GL_APIENTRY __glnop_UseProgramStages(__GLcontext *gc, GLuint pipeline, GLbitfield stages, GLuint program) {}
GLvoid GL_APIENTRY __glnop_ActiveShaderProgram(__GLcontext *gc, GLuint pipeline, GLuint program) {}
GLuint GL_APIENTRY __glnop_CreateShaderProgramv(__GLcontext *gc, GLenum type, GLsizei count, const GLchar *const*strings) {return 0;}
GLvoid GL_APIENTRY __glnop_BindProgramPipeline(__GLcontext *gc, GLuint pipeline) {}
GLvoid GL_APIENTRY __glnop_DeleteProgramPipelines(__GLcontext *gc, GLsizei n, const GLuint *pipelines) {}
GLvoid GL_APIENTRY __glnop_GenProgramPipelines(__GLcontext *gc, GLsizei n, GLuint *pipelines) {}
GLboolean GL_APIENTRY __glnop_IsProgramPipeline(__GLcontext *gc, GLuint pipeline) {return GL_FALSE;}
GLvoid GL_APIENTRY __glnop_GetProgramPipelineiv(__GLcontext *gc, GLuint pipeline, GLenum pname, GLint *params) {}
GLvoid GL_APIENTRY __glnop_ProgramUniform1i(__GLcontext *gc, GLuint program, GLint location, GLint v0) {}
GLvoid GL_APIENTRY __glnop_ProgramUniform1iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value) {}
GLvoid GL_APIENTRY __glnop_ProgramUniform1f(__GLcontext *gc, GLuint program, GLint location, GLfloat v0) {}
GLvoid GL_APIENTRY __glnop_ProgramUniform1fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value) {}
GLvoid GL_APIENTRY __glnop_ProgramUniform1ui(__GLcontext *gc, GLuint program, GLint location, GLuint v0) {}
GLvoid GL_APIENTRY __glnop_ProgramUniform1uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value) {}
GLvoid GL_APIENTRY __glnop_ProgramUniform2i(__GLcontext *gc, GLuint program, GLint location, GLint v0, GLint v1) {}
GLvoid GL_APIENTRY __glnop_ProgramUniform2iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value) {}
GLvoid GL_APIENTRY __glnop_ProgramUniform2f(__GLcontext *gc, GLuint program, GLint location, GLfloat v0, GLfloat v1) {}
GLvoid GL_APIENTRY __glnop_ProgramUniform2fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value) {}
GLvoid GL_APIENTRY __glnop_ProgramUniform2ui(__GLcontext *gc, GLuint program, GLint location, GLuint v0, GLuint v1) {}
GLvoid GL_APIENTRY __glnop_ProgramUniform2uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value) {}
GLvoid GL_APIENTRY __glnop_ProgramUniform3i(__GLcontext *gc, GLuint program, GLint location, GLint v0, GLint v1, GLint v2) {}
GLvoid GL_APIENTRY __glnop_ProgramUniform3iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value) {}
GLvoid GL_APIENTRY __glnop_ProgramUniform3f(__GLcontext *gc, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {}
GLvoid GL_APIENTRY __glnop_ProgramUniform3fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value) {}
GLvoid GL_APIENTRY __glnop_ProgramUniform3ui(__GLcontext *gc, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2) {}
GLvoid GL_APIENTRY __glnop_ProgramUniform3uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value) {}
GLvoid GL_APIENTRY __glnop_ProgramUniform4i(__GLcontext *gc, GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {}
GLvoid GL_APIENTRY __glnop_ProgramUniform4iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value) {}
GLvoid GL_APIENTRY __glnop_ProgramUniform4f(__GLcontext *gc, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {}
GLvoid GL_APIENTRY __glnop_ProgramUniform4fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value) {}
GLvoid GL_APIENTRY __glnop_ProgramUniform4ui(__GLcontext *gc, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) {}
GLvoid GL_APIENTRY __glnop_ProgramUniform4uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value) {}
GLvoid GL_APIENTRY __glnop_ProgramUniformMatrix2fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {}
GLvoid GL_APIENTRY __glnop_ProgramUniformMatrix3fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {}
GLvoid GL_APIENTRY __glnop_ProgramUniformMatrix4fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {}
GLvoid GL_APIENTRY __glnop_ProgramUniformMatrix2x3fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {}
GLvoid GL_APIENTRY __glnop_ProgramUniformMatrix3x2fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {}
GLvoid GL_APIENTRY __glnop_ProgramUniformMatrix2x4fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {}
GLvoid GL_APIENTRY __glnop_ProgramUniformMatrix4x2fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {}
GLvoid GL_APIENTRY __glnop_ProgramUniformMatrix3x4fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {}
GLvoid GL_APIENTRY __glnop_ProgramUniformMatrix4x3fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {}
GLvoid GL_APIENTRY __glnop_ValidateProgramPipeline(__GLcontext *gc, GLuint pipeline) {}
GLvoid GL_APIENTRY __glnop_GetProgramPipelineInfoLog(__GLcontext *gc, GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog) {}
/* GL_VERSION_4_2, incomplete: defined by later GL version but required by ES */
GLvoid GL_APIENTRY __glnop_GetInternalformativ(__GLcontext *gc, GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params) {}
GLvoid GL_APIENTRY __glnop_BindImageTexture(__GLcontext *gc, GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format) {}
GLvoid GL_APIENTRY __glnop_MemoryBarrier(__GLcontext *gc, GLbitfield barriers) {}
GLvoid GL_APIENTRY __glnop_TexStorage2D(__GLcontext *gc, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) {}
GLvoid GL_APIENTRY __glnop_TexStorage3D(__GLcontext *gc, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth) {}
/* GL_VERSION_4_3, incomplete: defined by later GL version but required by ES */
GLvoid GL_APIENTRY __glnop_DispatchCompute(__GLcontext *gc, GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z) {}
GLvoid GL_APIENTRY __glnop_DispatchComputeIndirect(__GLcontext *gc, GLintptr indirect) {}
GLvoid GL_APIENTRY __glnop_CopyImageSubData(__GLcontext *gc, GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth) {}
GLvoid GL_APIENTRY __glnop_FramebufferParameteri(__GLcontext *gc, GLenum target, GLenum pname, GLint param) {}
GLvoid GL_APIENTRY __glnop_GetFramebufferParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint *params) {}
GLvoid GL_APIENTRY __glnop_InvalidateFramebuffer(__GLcontext *gc, GLenum target, GLsizei numAttachments, const GLenum *attachments) {}
GLvoid GL_APIENTRY __glnop_InvalidateSubFramebuffer(__GLcontext *gc, GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height) {}
GLvoid GL_APIENTRY __glnop_MultiDrawArraysIndirect(__GLcontext *gc, GLenum mode, const GLvoid *indirect, GLsizei drawcount, GLsizei stride) {}
GLvoid GL_APIENTRY __glnop_MultiDrawElementsIndirect(__GLcontext *gc, GLenum mode, GLenum type, const GLvoid *indirect, GLsizei drawcount, GLsizei stride) {}
GLvoid GL_APIENTRY __glnop_GetProgramInterfaceiv(__GLcontext *gc, GLuint program, GLenum programInterface, GLenum pname, GLint *params) {}
GLuint GL_APIENTRY __glnop_GetProgramResourceIndex(__GLcontext *gc, GLuint program, GLenum programInterface, const GLchar *name) {return 0;}
GLvoid GL_APIENTRY __glnop_GetProgramResourceName(__GLcontext *gc, GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name) {}
GLvoid GL_APIENTRY __glnop_GetProgramResourceiv(__GLcontext *gc, GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params) {}
GLint  GL_APIENTRY __glnop_GetProgramResourceLocation(__GLcontext *gc, GLuint program, GLenum programInterface, const GLchar *name) {return -1;}
GLvoid GL_APIENTRY __glnop_TexBufferRange(__GLcontext *gc, GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size) {}
GLvoid GL_APIENTRY __glnop_TexStorage2DMultisample(__GLcontext *gc, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations) {}
GLvoid GL_APIENTRY __glnop_TexStorage3DMultisample(__GLcontext *gc, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations) {}
GLvoid GL_APIENTRY __glnop_BindVertexBuffer(__GLcontext *gc, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride) {}
GLvoid GL_APIENTRY __glnop_VertexAttribFormat(__GLcontext *gc, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset) {}
GLvoid GL_APIENTRY __glnop_VertexAttribIFormat(__GLcontext *gc, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset) {}
GLvoid GL_APIENTRY __glnop_VertexAttribBinding(__GLcontext *gc, GLuint attribindex, GLuint bindingindex) {}
GLvoid GL_APIENTRY __glnop_VertexBindingDivisor(__GLcontext *gc, GLuint bindingindex, GLuint divisor) {}
GLvoid GL_APIENTRY __glnop_DebugMessageControl(__GLcontext *gc, GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled) {}
GLvoid GL_APIENTRY __glnop_DebugMessageInsert(__GLcontext *gc, GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf) {}
GLvoid GL_APIENTRY __glnop_DebugMessageCallback(__GLcontext *gc, GLDEBUGPROC callback, const GLvoid *userParam) {}
GLuint GL_APIENTRY __glnop_GetDebugMessageLog(__GLcontext *gc, GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog) {return 0;}
GLvoid GL_APIENTRY __glnop_PushDebugGroup(__GLcontext *gc, GLenum source, GLuint id, GLsizei length, const GLchar *message) {}
GLvoid GL_APIENTRY __glnop_PopDebugGroup(__GLcontext *gc) {}
GLvoid GL_APIENTRY __glnop_ObjectLabel(__GLcontext *gc, GLenum identifier, GLuint name, GLsizei length, const GLchar *label) {}
GLvoid GL_APIENTRY __glnop_GetObjectLabel(__GLcontext *gc, GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label) {}
GLvoid GL_APIENTRY __glnop_ObjectPtrLabel(__GLcontext *gc, const GLvoid *ptr, GLsizei length, const GLchar *label) {}
GLvoid GL_APIENTRY __glnop_GetObjectPtrLabel(__GLcontext *gc, const GLvoid *ptr, GLsizei bufSize, GLsizei *length, GLchar *label) {}
/* GL_VERSION_4_5, incomplete: defined by later GL version but required by ES */
GLvoid GL_APIENTRY __glnop_MemoryBarrierByRegion(__GLcontext *gc, GLbitfield barriers) {}
GLenum GL_APIENTRY __glnop_GetGraphicsResetStatus(__GLcontext *gc) {return GL_NO_ERROR;}
GLvoid GL_APIENTRY __glnop_ReadnPixels(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLvoid *data) {}
GLvoid GL_APIENTRY __glnop_GetnUniformfv(__GLcontext *gc, GLuint program, GLint location, GLsizei bufSize, GLfloat *params) {}
GLvoid GL_APIENTRY __glnop_GetnUniformiv(__GLcontext *gc, GLuint program, GLint location, GLsizei bufSize, GLint *params) {}
GLvoid GL_APIENTRY __glnop_GetnUniformuiv(__GLcontext *gc, GLuint program, GLint location, GLsizei bufSize, GLuint *params) {}
/* OpenGL ES extensions */
/* ES_VERSION_3_2 */
GLvoid GL_APIENTRY __glnop_BlendBarrier(__GLcontext *gc) {}
GLvoid GL_APIENTRY __glnop_PrimitiveBoundingBox(__GLcontext *gc, GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW) {}
/* GL_OES_EGL_image */
GLvoid GL_APIENTRY __glnop_EGLImageTargetTexture2DOES(__GLcontext *gc, GLenum target, GLeglImageOES image) {}
GLvoid GL_APIENTRY __glnop_EGLImageTargetRenderbufferStorageOES(__GLcontext *gc, GLenum target, GLeglImageOES image) {}
/* GL_VIV_direct_texture */
GLvoid GL_APIENTRY __glnop_TexDirectVIV(__GLcontext *gc, GLenum Target, GLsizei Width, GLsizei Height, GLenum Format, GLvoid ** Pixels) {}
GLvoid GL_APIENTRY __glnop_TexDirectInvalidateVIV(__GLcontext *gc, GLenum Target) {}
GLvoid GL_APIENTRY __glnop_TexDirectVIVMap(__GLcontext *gc, GLenum Target, GLsizei Width, GLsizei Height, GLenum Format, GLvoid ** Logical, const GLuint * Physical) {}
GLvoid GL_APIENTRY __glnop_TexDirectTiledMapVIV(__GLcontext *gc, GLenum Target, GLsizei Width, GLsizei Height, GLenum Format, GLvoid ** Logical, const GLuint * Physical) {}
/* GL_EXT_multisampled_render_to_texture */
GLvoid GL_APIENTRY __glnop_FramebufferTexture2DMultisampleEXT(__GLcontext *gc, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples) {}
/* GL_EXT_discard_framebuffer */
GLvoid GL_APIENTRY __glnop_DiscardFramebufferEXT(__GLcontext *gc, GLenum target, GLsizei numAttachments, const GLenum *attachments) {}
/* GL_ARB_shader_objects */
GLvoid GL_APIENTRY __glnop_DeleteObjectARB(__GLcontext *gc, GLhandleARB obj) {}
GLvoid GL_APIENTRY __glnop_GetObjectParameterivARB(__GLcontext *gc, GLhandleARB obj, GLenum pname, GLint *params) {}
GLvoid GL_APIENTRY __glnop_GetInfoLogARB(__GLcontext *gc, GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog) {}


#ifdef OPENGL40
const GLubyte * (*__glListExecFuncTable[])(__GLcontext *, const GLubyte *) = {
    __GL_LISTEXEC_ENTRIES(__glle, )
};
#endif

__GLdispatchTable __glImmediateFuncTable = {
    __GL_API_ENTRIES(__glim)
};

__GLdispatchTable __glListCompileFuncTable = {
    __GL_API_ENTRIES(__glim)
};

__GLdispatchTable __glNopFuncTable = {
    __GL_API_ENTRIES(__glnop)
};

/* temporarily for passing WGL */
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

GLvoid __glOverWriteListCompileTable()
{
    __GLdispatchTable *__gllc_Table = &__glListCompileFuncTable;

    /* GL_VERSION_1_0 */
    __gllc_Table->CallList = __gllc_CallList;
    __gllc_Table->CallLists = __gllc_CallLists;
    __gllc_Table->ListBase = __gllc_ListBase;
    __gllc_Table->Begin = __gllc_Begin;
    __gllc_Table->Bitmap = __gllc_Bitmap;
    __gllc_Table->Color3b = __gllc_Color3b;
    __gllc_Table->Color3bv = __gllc_Color3bv;
    __gllc_Table->Color3d = __gllc_Color3d;
    __gllc_Table->Color3dv = __gllc_Color3dv;
    __gllc_Table->Color3f = __gllc_Color3f;
    __gllc_Table->Color3fv = __gllc_Color3fv;
    __gllc_Table->Color3i = __gllc_Color3i;
    __gllc_Table->Color3iv = __gllc_Color3iv;
    __gllc_Table->Color3s = __gllc_Color3s;
    __gllc_Table->Color3sv = __gllc_Color3sv;
    __gllc_Table->Color3ub = __gllc_Color3ub;
    __gllc_Table->Color3ubv = __gllc_Color3ubv;
    __gllc_Table->Color3ui = __gllc_Color3ui;
    __gllc_Table->Color3uiv = __gllc_Color3uiv;
    __gllc_Table->Color3us = __gllc_Color3us;
    __gllc_Table->Color3usv = __gllc_Color3usv;
    __gllc_Table->Color4b = __gllc_Color4b;
    __gllc_Table->Color4bv = __gllc_Color4bv;
    __gllc_Table->Color4d = __gllc_Color4d;
    __gllc_Table->Color4dv = __gllc_Color4dv;
    __gllc_Table->Color4f = __gllc_Color4f;
    __gllc_Table->Color4fv = __gllc_Color4fv;
    __gllc_Table->Color4i = __gllc_Color4i;
    __gllc_Table->Color4iv = __gllc_Color4iv;
    __gllc_Table->Color4s = __gllc_Color4s;
    __gllc_Table->Color4sv = __gllc_Color4sv;
    __gllc_Table->Color4ub = __gllc_Color4ub;
    __gllc_Table->Color4ubv = __gllc_Color4ubv;
    __gllc_Table->Color4ui = __gllc_Color4ui;
    __gllc_Table->Color4uiv = __gllc_Color4uiv;
    __gllc_Table->Color4us = __gllc_Color4us;
    __gllc_Table->Color4usv = __gllc_Color4usv;
    __gllc_Table->EdgeFlag = __gllc_EdgeFlag;
    __gllc_Table->EdgeFlagv = __gllc_EdgeFlagv;
    __gllc_Table->End = __gllc_End;
    __gllc_Table->Indexd = __gllc_Indexd;
    __gllc_Table->Indexdv = __gllc_Indexdv;
    __gllc_Table->Indexf = __gllc_Indexf;
    __gllc_Table->Indexfv = __gllc_Indexfv;
    __gllc_Table->Indexi = __gllc_Indexi;
    __gllc_Table->Indexiv = __gllc_Indexiv;
    __gllc_Table->Indexs = __gllc_Indexs;
    __gllc_Table->Indexsv = __gllc_Indexsv;
    __gllc_Table->Normal3b = __gllc_Normal3b;
    __gllc_Table->Normal3bv = __gllc_Normal3bv;
    __gllc_Table->Normal3d = __gllc_Normal3d;
    __gllc_Table->Normal3dv = __gllc_Normal3dv;
    __gllc_Table->Normal3f = __gllc_Normal3f;
    __gllc_Table->Normal3fv = __gllc_Normal3fv;
    __gllc_Table->Normal3i = __gllc_Normal3i;
    __gllc_Table->Normal3iv = __gllc_Normal3iv;
    __gllc_Table->Normal3s = __gllc_Normal3s;
    __gllc_Table->Normal3sv = __gllc_Normal3sv;
    __gllc_Table->RasterPos2d = __gllc_RasterPos2d;
    __gllc_Table->RasterPos2dv = __gllc_RasterPos2dv;
    __gllc_Table->RasterPos2f = __gllc_RasterPos2f;
    __gllc_Table->RasterPos2fv = __gllc_RasterPos2fv;
    __gllc_Table->RasterPos2i = __gllc_RasterPos2i;
    __gllc_Table->RasterPos2iv = __gllc_RasterPos2iv;
    __gllc_Table->RasterPos2s = __gllc_RasterPos2s;
    __gllc_Table->RasterPos2sv = __gllc_RasterPos2sv;
    __gllc_Table->RasterPos3d = __gllc_RasterPos3d;
    __gllc_Table->RasterPos3dv = __gllc_RasterPos3dv;
    __gllc_Table->RasterPos3f = __gllc_RasterPos3f;
    __gllc_Table->RasterPos3fv = __gllc_RasterPos3fv;
    __gllc_Table->RasterPos3i = __gllc_RasterPos3i;
    __gllc_Table->RasterPos3iv = __gllc_RasterPos3iv;
    __gllc_Table->RasterPos3s = __gllc_RasterPos3s;
    __gllc_Table->RasterPos3sv = __gllc_RasterPos3sv;
    __gllc_Table->RasterPos4d = __gllc_RasterPos4d;
    __gllc_Table->RasterPos4dv = __gllc_RasterPos4dv;
    __gllc_Table->RasterPos4f = __gllc_RasterPos4f;
    __gllc_Table->RasterPos4fv = __gllc_RasterPos4fv;
    __gllc_Table->RasterPos4i = __gllc_RasterPos4i;
    __gllc_Table->RasterPos4iv = __gllc_RasterPos4iv;
    __gllc_Table->RasterPos4s = __gllc_RasterPos4s;
    __gllc_Table->RasterPos4sv = __gllc_RasterPos4sv;
    __gllc_Table->Rectd = __gllc_Rectd;
    __gllc_Table->Rectdv = __gllc_Rectdv;
    __gllc_Table->Rectf = __gllc_Rectf;
    __gllc_Table->Rectfv = __gllc_Rectfv;
    __gllc_Table->Recti = __gllc_Recti;
    __gllc_Table->Rectiv = __gllc_Rectiv;
    __gllc_Table->Rects = __gllc_Rects;
    __gllc_Table->Rectsv = __gllc_Rectsv;
    __gllc_Table->TexCoord1d = __gllc_TexCoord1d;
    __gllc_Table->TexCoord1dv = __gllc_TexCoord1dv;
    __gllc_Table->TexCoord1f = __gllc_TexCoord1f;
    __gllc_Table->TexCoord1fv = __gllc_TexCoord1fv;
    __gllc_Table->TexCoord1i = __gllc_TexCoord1i;
    __gllc_Table->TexCoord1iv = __gllc_TexCoord1iv;
    __gllc_Table->TexCoord1s = __gllc_TexCoord1s;
    __gllc_Table->TexCoord1sv = __gllc_TexCoord1sv;
    __gllc_Table->TexCoord2d = __gllc_TexCoord2d;
    __gllc_Table->TexCoord2dv = __gllc_TexCoord2dv;
    __gllc_Table->TexCoord2f = __gllc_TexCoord2f;
    __gllc_Table->TexCoord2fv = __gllc_TexCoord2fv;
    __gllc_Table->TexCoord2i = __gllc_TexCoord2i;
    __gllc_Table->TexCoord2iv = __gllc_TexCoord2iv;
    __gllc_Table->TexCoord2s = __gllc_TexCoord2s;
    __gllc_Table->TexCoord2sv = __gllc_TexCoord2sv;
    __gllc_Table->TexCoord3d = __gllc_TexCoord3d;
    __gllc_Table->TexCoord3dv = __gllc_TexCoord3dv;
    __gllc_Table->TexCoord3f = __gllc_TexCoord3f;
    __gllc_Table->TexCoord3fv = __gllc_TexCoord3fv;
    __gllc_Table->TexCoord3i = __gllc_TexCoord3i;
    __gllc_Table->TexCoord3iv = __gllc_TexCoord3iv;
    __gllc_Table->TexCoord3s = __gllc_TexCoord3s;
    __gllc_Table->TexCoord3sv = __gllc_TexCoord3sv;
    __gllc_Table->TexCoord4d = __gllc_TexCoord4d;
    __gllc_Table->TexCoord4dv = __gllc_TexCoord4dv;
    __gllc_Table->TexCoord4f = __gllc_TexCoord4f;
    __gllc_Table->TexCoord4fv = __gllc_TexCoord4fv;
    __gllc_Table->TexCoord4i = __gllc_TexCoord4i;
    __gllc_Table->TexCoord4iv = __gllc_TexCoord4iv;
    __gllc_Table->TexCoord4s = __gllc_TexCoord4s;
    __gllc_Table->TexCoord4sv = __gllc_TexCoord4sv;
    __gllc_Table->Vertex2d = __gllc_Vertex2d;
    __gllc_Table->Vertex2dv = __gllc_Vertex2dv;
    __gllc_Table->Vertex2f = __gllc_Vertex2f;
    __gllc_Table->Vertex2fv = __gllc_Vertex2fv;
    __gllc_Table->Vertex2i = __gllc_Vertex2i;
    __gllc_Table->Vertex2iv = __gllc_Vertex2iv;
    __gllc_Table->Vertex2s = __gllc_Vertex2s;
    __gllc_Table->Vertex2sv = __gllc_Vertex2sv;
    __gllc_Table->Vertex3d = __gllc_Vertex3d;
    __gllc_Table->Vertex3dv = __gllc_Vertex3dv;
    __gllc_Table->Vertex3f = __gllc_Vertex3f;
    __gllc_Table->Vertex3fv = __gllc_Vertex3fv;
    __gllc_Table->Vertex3i = __gllc_Vertex3i;
    __gllc_Table->Vertex3iv = __gllc_Vertex3iv;
    __gllc_Table->Vertex3s = __gllc_Vertex3s;
    __gllc_Table->Vertex3sv = __gllc_Vertex3sv;
    __gllc_Table->Vertex4d = __gllc_Vertex4d;
    __gllc_Table->Vertex4dv = __gllc_Vertex4dv;
    __gllc_Table->Vertex4f = __gllc_Vertex4f;
    __gllc_Table->Vertex4fv = __gllc_Vertex4fv;
    __gllc_Table->Vertex4i = __gllc_Vertex4i;
    __gllc_Table->Vertex4iv = __gllc_Vertex4iv;
    __gllc_Table->Vertex4s = __gllc_Vertex4s;
    __gllc_Table->Vertex4sv = __gllc_Vertex4sv;
    __gllc_Table->ClipPlane = __gllc_ClipPlane;
    __gllc_Table->ColorMaterial = __gllc_ColorMaterial;
    __gllc_Table->CullFace = __gllc_CullFace;
    __gllc_Table->Fogf = __gllc_Fogf;
    __gllc_Table->Fogfv = __gllc_Fogfv;
    __gllc_Table->Fogi = __gllc_Fogi;
    __gllc_Table->Fogiv = __gllc_Fogiv;
    __gllc_Table->FrontFace = __gllc_FrontFace;
    __gllc_Table->Hint = __gllc_Hint;
    __gllc_Table->Lightf = __gllc_Lightf;
    __gllc_Table->Lightfv = __gllc_Lightfv;
    __gllc_Table->Lighti = __gllc_Lighti;
    __gllc_Table->Lightiv = __gllc_Lightiv;
    __gllc_Table->LightModelf = __gllc_LightModelf;
    __gllc_Table->LightModelfv = __gllc_LightModelfv;
    __gllc_Table->LightModeli = __gllc_LightModeli;
    __gllc_Table->LightModeliv = __gllc_LightModeliv;
    __gllc_Table->LineStipple = __gllc_LineStipple;
    __gllc_Table->LineWidth = __gllc_LineWidth;
    __gllc_Table->Materialf = __gllc_Materialf;
    __gllc_Table->Materialfv = __gllc_Materialfv;
    __gllc_Table->Materiali = __gllc_Materiali;
    __gllc_Table->Materialiv = __gllc_Materialiv;
    __gllc_Table->PointSize = __gllc_PointSize;
    __gllc_Table->PolygonMode = __gllc_PolygonMode;
    __gllc_Table->PolygonStipple = __gllc_PolygonStipple;
    __gllc_Table->Scissor = __gllc_Scissor;
    __gllc_Table->ShadeModel = __gllc_ShadeModel;
    __gllc_Table->TexParameterf = __gllc_TexParameterf;
    __gllc_Table->TexParameterfv = __gllc_TexParameterfv;
    __gllc_Table->TexParameteri = __gllc_TexParameteri;
    __gllc_Table->TexParameteriv = __gllc_TexParameteriv;
    __gllc_Table->TexImage1D = __gllc_TexImage1D;
    __gllc_Table->TexImage2D = __gllc_TexImage2D;
    __gllc_Table->TexEnvf = __gllc_TexEnvf;
    __gllc_Table->TexEnvfv = __gllc_TexEnvfv;
    __gllc_Table->TexEnvi = __gllc_TexEnvi;
    __gllc_Table->TexEnviv = __gllc_TexEnviv;
    __gllc_Table->TexGend = __gllc_TexGend;
    __gllc_Table->TexGendv = __gllc_TexGendv;
    __gllc_Table->TexGenf = __gllc_TexGenf;
    __gllc_Table->TexGenfv = __gllc_TexGenfv;
    __gllc_Table->TexGeni = __gllc_TexGeni;
    __gllc_Table->TexGeniv = __gllc_TexGeniv;
    __gllc_Table->InitNames = __gllc_InitNames;
    __gllc_Table->LoadName = __gllc_LoadName;
    __gllc_Table->PassThrough = __gllc_PassThrough;
    __gllc_Table->PopName = __gllc_PopName;
    __gllc_Table->PushName = __gllc_PushName;
    __gllc_Table->DrawBuffer = __gllc_DrawBuffer;
    __gllc_Table->Clear = __gllc_Clear;
    __gllc_Table->ClearAccum = __gllc_ClearAccum;
    __gllc_Table->ClearIndex = __gllc_ClearIndex;
    __gllc_Table->ClearColor = __gllc_ClearColor;
    __gllc_Table->ClearStencil = __gllc_ClearStencil;
    __gllc_Table->ClearDepth = __gllc_ClearDepth;
    __gllc_Table->StencilMask = __gllc_StencilMask;
    __gllc_Table->ColorMask = __gllc_ColorMask;
    __gllc_Table->DepthMask = __gllc_DepthMask;
    __gllc_Table->IndexMask = __gllc_IndexMask;
    __gllc_Table->Accum = __gllc_Accum;
    __gllc_Table->Disable = __gllc_Disable;
    __gllc_Table->Enable = __gllc_Enable;
    __gllc_Table->PopAttrib = __gllc_PopAttrib;
    __gllc_Table->PushAttrib = __gllc_PushAttrib;
    __gllc_Table->Map1d = __gllc_Map1d;
    __gllc_Table->Map1f = __gllc_Map1f;
    __gllc_Table->Map2d = __gllc_Map2d;
    __gllc_Table->Map2f = __gllc_Map2f;
    __gllc_Table->MapGrid1d = __gllc_MapGrid1d;
    __gllc_Table->MapGrid1f = __gllc_MapGrid1f;
    __gllc_Table->MapGrid2d = __gllc_MapGrid2d;
    __gllc_Table->MapGrid2f = __gllc_MapGrid2f;
    __gllc_Table->EvalCoord1d = __gllc_EvalCoord1d;
    __gllc_Table->EvalCoord1dv = __gllc_EvalCoord1dv;
    __gllc_Table->EvalCoord1f = __gllc_EvalCoord1f;
    __gllc_Table->EvalCoord1fv = __gllc_EvalCoord1fv;
    __gllc_Table->EvalCoord2d = __gllc_EvalCoord2d;
    __gllc_Table->EvalCoord2dv = __gllc_EvalCoord2dv;
    __gllc_Table->EvalCoord2f = __gllc_EvalCoord2f;
    __gllc_Table->EvalCoord2fv = __gllc_EvalCoord2fv;
    __gllc_Table->EvalMesh1 = __gllc_EvalMesh1;
    __gllc_Table->EvalPoint1 = __gllc_EvalPoint1;
    __gllc_Table->EvalMesh2 = __gllc_EvalMesh2;
    __gllc_Table->EvalPoint2 = __gllc_EvalPoint2;
    __gllc_Table->AlphaFunc = __gllc_AlphaFunc;
    __gllc_Table->BlendFunc = __gllc_BlendFunc;
    __gllc_Table->LogicOp = __gllc_LogicOp;
    __gllc_Table->StencilFunc = __gllc_StencilFunc;
    __gllc_Table->StencilOp = __gllc_StencilOp;
    __gllc_Table->DepthFunc = __gllc_DepthFunc;
    __gllc_Table->PixelZoom = __gllc_PixelZoom;
    __gllc_Table->PixelTransferf = __gllc_PixelTransferf;
    __gllc_Table->PixelTransferi = __gllc_PixelTransferi;
    __gllc_Table->PixelMapfv = __gllc_PixelMapfv;
    __gllc_Table->PixelMapuiv = __gllc_PixelMapuiv;
    __gllc_Table->PixelMapusv = __gllc_PixelMapusv;
    __gllc_Table->ReadBuffer = __gllc_ReadBuffer;
    __gllc_Table->CopyPixels = __gllc_CopyPixels;
    __gllc_Table->DrawPixels = __gllc_DrawPixels;
    __gllc_Table->DepthRange = __gllc_DepthRange;
    __gllc_Table->Frustum = __gllc_Frustum;
    __gllc_Table->LoadIdentity = __gllc_LoadIdentity;
    __gllc_Table->LoadMatrixf = __gllc_LoadMatrixf;
    __gllc_Table->LoadMatrixd = __gllc_LoadMatrixd;
    __gllc_Table->MatrixMode = __gllc_MatrixMode;
    __gllc_Table->MultMatrixf = __gllc_MultMatrixf;
    __gllc_Table->MultMatrixd = __gllc_MultMatrixd;
    __gllc_Table->Ortho = __gllc_Ortho;
    __gllc_Table->PopMatrix = __gllc_PopMatrix;
    __gllc_Table->PushMatrix = __gllc_PushMatrix;
    __gllc_Table->Rotated = __gllc_Rotated;
    __gllc_Table->Rotatef = __gllc_Rotatef;
    __gllc_Table->Scaled = __gllc_Scaled;
    __gllc_Table->Scalef = __gllc_Scalef;
    __gllc_Table->Translated = __gllc_Translated;
    __gllc_Table->Translatef = __gllc_Translatef;
    __gllc_Table->Viewport = __gllc_Viewport;
    /* GL_VERSION_1_1 */
    __gllc_Table->ArrayElement = __gllc_ArrayElement;
    __gllc_Table->BindTexture = __gllc_BindTexture;
    __gllc_Table->DrawArrays = __gllc_DrawArrays;
    __gllc_Table->DrawElements = __gllc_DrawElements;
    __gllc_Table->Indexub = __gllc_Indexub;
    __gllc_Table->Indexubv = __gllc_Indexubv;
    __gllc_Table->PolygonOffset = __gllc_PolygonOffset;
    __gllc_Table->CopyTexImage1D = __gllc_CopyTexImage1D;
    __gllc_Table->CopyTexImage2D = __gllc_CopyTexImage2D;
    __gllc_Table->CopyTexSubImage1D = __gllc_CopyTexSubImage1D;
    __gllc_Table->CopyTexSubImage2D = __gllc_CopyTexSubImage2D;
    __gllc_Table->PrioritizeTextures = __gllc_PrioritizeTextures;
    __gllc_Table->TexSubImage1D = __gllc_TexSubImage1D;
    __gllc_Table->TexSubImage2D = __gllc_TexSubImage2D;
    /* GL_VERSION_1_2 */
    __gllc_Table->DrawRangeElements = __gllc_DrawRangeElements;
    __gllc_Table->TexImage3D = __gllc_TexImage3D;
    __gllc_Table->TexSubImage3D = __gllc_TexSubImage3D;
    __gllc_Table->CopyTexSubImage3D = __gllc_CopyTexSubImage3D;
    /* GL_VERSION_1_3 */
    __gllc_Table->ActiveTexture = __gllc_ActiveTexture;
    __gllc_Table->SampleCoverage = __gllc_SampleCoverage;
    __gllc_Table->CompressedTexImage3D = __gllc_CompressedTexImage3D;
    __gllc_Table->CompressedTexImage2D = __gllc_CompressedTexImage2D;
    __gllc_Table->CompressedTexImage1D = __gllc_CompressedTexImage1D;
    __gllc_Table->CompressedTexSubImage3D = __gllc_CompressedTexSubImage3D;
    __gllc_Table->CompressedTexSubImage2D = __gllc_CompressedTexSubImage2D;
    __gllc_Table->CompressedTexSubImage1D = __gllc_CompressedTexSubImage1D;
    __gllc_Table->MultiTexCoord1d = __gllc_MultiTexCoord1d;
    __gllc_Table->MultiTexCoord1dv = __gllc_MultiTexCoord1dv;
    __gllc_Table->MultiTexCoord1f = __gllc_MultiTexCoord1f;
    __gllc_Table->MultiTexCoord1fv = __gllc_MultiTexCoord1fv;
    __gllc_Table->MultiTexCoord1i = __gllc_MultiTexCoord1i;
    __gllc_Table->MultiTexCoord1iv = __gllc_MultiTexCoord1iv;
    __gllc_Table->MultiTexCoord1s = __gllc_MultiTexCoord1s;
    __gllc_Table->MultiTexCoord1sv = __gllc_MultiTexCoord1sv;
    __gllc_Table->MultiTexCoord2d = __gllc_MultiTexCoord2d;
    __gllc_Table->MultiTexCoord2dv = __gllc_MultiTexCoord2dv;
    __gllc_Table->MultiTexCoord2f = __gllc_MultiTexCoord2f;
    __gllc_Table->MultiTexCoord2fv = __gllc_MultiTexCoord2fv;
    __gllc_Table->MultiTexCoord2i = __gllc_MultiTexCoord2i;
    __gllc_Table->MultiTexCoord2iv = __gllc_MultiTexCoord2iv;
    __gllc_Table->MultiTexCoord2s = __gllc_MultiTexCoord2s;
    __gllc_Table->MultiTexCoord2sv = __gllc_MultiTexCoord2sv;
    __gllc_Table->MultiTexCoord3d = __gllc_MultiTexCoord3d;
    __gllc_Table->MultiTexCoord3dv = __gllc_MultiTexCoord3dv;
    __gllc_Table->MultiTexCoord3f = __gllc_MultiTexCoord3f;
    __gllc_Table->MultiTexCoord3fv = __gllc_MultiTexCoord3fv;
    __gllc_Table->MultiTexCoord3i = __gllc_MultiTexCoord3i;
    __gllc_Table->MultiTexCoord3iv = __gllc_MultiTexCoord3iv;
    __gllc_Table->MultiTexCoord3s = __gllc_MultiTexCoord3s;
    __gllc_Table->MultiTexCoord3sv = __gllc_MultiTexCoord3sv;
    __gllc_Table->MultiTexCoord4d = __gllc_MultiTexCoord4d;
    __gllc_Table->MultiTexCoord4dv = __gllc_MultiTexCoord4dv;
    __gllc_Table->MultiTexCoord4f = __gllc_MultiTexCoord4f;
    __gllc_Table->MultiTexCoord4fv = __gllc_MultiTexCoord4fv;
    __gllc_Table->MultiTexCoord4i = __gllc_MultiTexCoord4i;
    __gllc_Table->MultiTexCoord4iv = __gllc_MultiTexCoord4iv;
    __gllc_Table->MultiTexCoord4s = __gllc_MultiTexCoord4s;
    __gllc_Table->MultiTexCoord4sv = __gllc_MultiTexCoord4sv;
    __gllc_Table->LoadTransposeMatrixf = __gllc_LoadTransposeMatrixf;
    __gllc_Table->LoadTransposeMatrixd = __gllc_LoadTransposeMatrixd;
    __gllc_Table->MultTransposeMatrixf = __gllc_MultTransposeMatrixf;
    __gllc_Table->MultTransposeMatrixd = __gllc_MultTransposeMatrixd;
    /* GL_VERSION_1_4 */
    __gllc_Table->BlendFuncSeparate = __gllc_BlendFuncSeparate;
    __gllc_Table->MultiDrawArrays = __gllc_MultiDrawArrays;
    __gllc_Table->MultiDrawElements = __gllc_MultiDrawElements;
    __gllc_Table->PointParameterf = __gllc_PointParameterf;
    __gllc_Table->PointParameterfv = __gllc_PointParameterfv;
    __gllc_Table->PointParameteri = __gllc_PointParameteri;
    __gllc_Table->PointParameteriv = __gllc_PointParameteriv;
    __gllc_Table->FogCoordf = __gllc_FogCoordf;
    __gllc_Table->FogCoordfv = __gllc_FogCoordfv;
    __gllc_Table->FogCoordd = __gllc_FogCoordd;
    __gllc_Table->FogCoorddv = __gllc_FogCoorddv;
    __gllc_Table->SecondaryColor3b = __gllc_SecondaryColor3b;
    __gllc_Table->SecondaryColor3bv = __gllc_SecondaryColor3bv;
    __gllc_Table->SecondaryColor3d = __gllc_SecondaryColor3d;
    __gllc_Table->SecondaryColor3dv = __gllc_SecondaryColor3dv;
    __gllc_Table->SecondaryColor3f = __gllc_SecondaryColor3f;
    __gllc_Table->SecondaryColor3fv = __gllc_SecondaryColor3fv;
    __gllc_Table->SecondaryColor3i = __gllc_SecondaryColor3i;
    __gllc_Table->SecondaryColor3iv = __gllc_SecondaryColor3iv;
    __gllc_Table->SecondaryColor3s = __gllc_SecondaryColor3s;
    __gllc_Table->SecondaryColor3sv = __gllc_SecondaryColor3sv;
    __gllc_Table->SecondaryColor3ub = __gllc_SecondaryColor3ub;
    __gllc_Table->SecondaryColor3ubv = __gllc_SecondaryColor3ubv;
    __gllc_Table->SecondaryColor3ui = __gllc_SecondaryColor3ui;
    __gllc_Table->SecondaryColor3uiv = __gllc_SecondaryColor3uiv;
    __gllc_Table->SecondaryColor3us = __gllc_SecondaryColor3us;
    __gllc_Table->SecondaryColor3usv = __gllc_SecondaryColor3usv;
    __gllc_Table->WindowPos2d = __gllc_WindowPos2d;
    __gllc_Table->WindowPos2dv = __gllc_WindowPos2dv;
    __gllc_Table->WindowPos2f = __gllc_WindowPos2f;
    __gllc_Table->WindowPos2fv = __gllc_WindowPos2fv;
    __gllc_Table->WindowPos2i = __gllc_WindowPos2i;
    __gllc_Table->WindowPos2iv = __gllc_WindowPos2iv;
    __gllc_Table->WindowPos2s = __gllc_WindowPos2s;
    __gllc_Table->WindowPos2sv = __gllc_WindowPos2sv;
    __gllc_Table->WindowPos3d = __gllc_WindowPos3d;
    __gllc_Table->WindowPos3dv = __gllc_WindowPos3dv;
    __gllc_Table->WindowPos3f = __gllc_WindowPos3f;
    __gllc_Table->WindowPos3fv = __gllc_WindowPos3fv;
    __gllc_Table->WindowPos3i = __gllc_WindowPos3i;
    __gllc_Table->WindowPos3iv = __gllc_WindowPos3iv;
    __gllc_Table->WindowPos3s = __gllc_WindowPos3s;
    __gllc_Table->WindowPos3sv = __gllc_WindowPos3sv;
    __gllc_Table->BlendColor = __gllc_BlendColor;
    __gllc_Table->BlendEquation = __gllc_BlendEquation;
    /* GL_VERSION_1_5 */
    __gllc_Table->BeginQuery = __gllc_BeginQuery;
    __gllc_Table->EndQuery = __gllc_EndQuery;
    /* GL_VERSION_2_0 */
    __gllc_Table->BlendEquationSeparate = __gllc_BlendEquationSeparate;
    __gllc_Table->DrawBuffers = __gllc_DrawBuffers;
    __gllc_Table->StencilOpSeparate = __gllc_StencilOpSeparate;
    __gllc_Table->StencilFuncSeparate = __gllc_StencilFuncSeparate;
    __gllc_Table->StencilMaskSeparate = __gllc_StencilMaskSeparate;
    __gllc_Table->UseProgram = __gllc_UseProgram;
    __gllc_Table->Uniform1f = __gllc_Uniform1f;
    __gllc_Table->Uniform2f = __gllc_Uniform2f;
    __gllc_Table->Uniform3f = __gllc_Uniform3f;
    __gllc_Table->Uniform4f = __gllc_Uniform4f;
    __gllc_Table->Uniform1i = __gllc_Uniform1i;
    __gllc_Table->Uniform2i = __gllc_Uniform2i;
    __gllc_Table->Uniform3i = __gllc_Uniform3i;
    __gllc_Table->Uniform4i = __gllc_Uniform4i;
    __gllc_Table->Uniform1fv = __gllc_Uniform1fv;
    __gllc_Table->Uniform2fv = __gllc_Uniform2fv;
    __gllc_Table->Uniform3fv = __gllc_Uniform3fv;
    __gllc_Table->Uniform4fv = __gllc_Uniform4fv;
    __gllc_Table->Uniform1iv = __gllc_Uniform1iv;
    __gllc_Table->Uniform2iv = __gllc_Uniform2iv;
    __gllc_Table->Uniform3iv = __gllc_Uniform3iv;
    __gllc_Table->Uniform4iv = __gllc_Uniform4iv;
    __gllc_Table->UniformMatrix2fv = __gllc_UniformMatrix2fv;
    __gllc_Table->UniformMatrix3fv = __gllc_UniformMatrix3fv;
    __gllc_Table->UniformMatrix4fv = __gllc_UniformMatrix4fv;
    __gllc_Table->VertexAttrib1d = __gllc_VertexAttrib1d;
    __gllc_Table->VertexAttrib1dv = __gllc_VertexAttrib1dv;
    __gllc_Table->VertexAttrib1f = __gllc_VertexAttrib1f;
    __gllc_Table->VertexAttrib1fv = __gllc_VertexAttrib1fv;
    __gllc_Table->VertexAttrib1s = __gllc_VertexAttrib1s;
    __gllc_Table->VertexAttrib1sv = __gllc_VertexAttrib1sv;
    __gllc_Table->VertexAttrib2d = __gllc_VertexAttrib2d;
    __gllc_Table->VertexAttrib2dv = __gllc_VertexAttrib2dv;
    __gllc_Table->VertexAttrib2f = __gllc_VertexAttrib2f;
    __gllc_Table->VertexAttrib2fv = __gllc_VertexAttrib2fv;
    __gllc_Table->VertexAttrib2s = __gllc_VertexAttrib2s;
    __gllc_Table->VertexAttrib2sv = __gllc_VertexAttrib2sv;
    __gllc_Table->VertexAttrib3d = __gllc_VertexAttrib3d;
    __gllc_Table->VertexAttrib3dv = __gllc_VertexAttrib3dv;
    __gllc_Table->VertexAttrib3f = __gllc_VertexAttrib3f;
    __gllc_Table->VertexAttrib3fv = __gllc_VertexAttrib3fv;
    __gllc_Table->VertexAttrib3s = __gllc_VertexAttrib3s;
    __gllc_Table->VertexAttrib3sv = __gllc_VertexAttrib3sv;
    __gllc_Table->VertexAttrib4Nbv = __gllc_VertexAttrib4Nbv;
    __gllc_Table->VertexAttrib4Niv = __gllc_VertexAttrib4Niv;
    __gllc_Table->VertexAttrib4Nsv = __gllc_VertexAttrib4Nsv;
    __gllc_Table->VertexAttrib4Nub = __gllc_VertexAttrib4Nub;
    __gllc_Table->VertexAttrib4Nubv = __gllc_VertexAttrib4Nubv;
    __gllc_Table->VertexAttrib4Nuiv = __gllc_VertexAttrib4Nuiv;
    __gllc_Table->VertexAttrib4Nusv = __gllc_VertexAttrib4Nusv;
    __gllc_Table->VertexAttrib4bv = __gllc_VertexAttrib4bv;
    __gllc_Table->VertexAttrib4d = __gllc_VertexAttrib4d;
    __gllc_Table->VertexAttrib4dv = __gllc_VertexAttrib4dv;
    __gllc_Table->VertexAttrib4f = __gllc_VertexAttrib4f;
    __gllc_Table->VertexAttrib4fv = __gllc_VertexAttrib4fv;
    __gllc_Table->VertexAttrib4iv = __gllc_VertexAttrib4iv;
    __gllc_Table->VertexAttrib4s = __gllc_VertexAttrib4s;
    __gllc_Table->VertexAttrib4sv = __gllc_VertexAttrib4sv;
    __gllc_Table->VertexAttrib4ubv = __gllc_VertexAttrib4ubv;
    __gllc_Table->VertexAttrib4uiv = __gllc_VertexAttrib4uiv;
    __gllc_Table->VertexAttrib4usv = __gllc_VertexAttrib4usv;
    /* GL_VERSION_2_1 */
    __gllc_Table->UniformMatrix2x3fv = __gllc_UniformMatrix2x3fv;
    __gllc_Table->UniformMatrix3x2fv = __gllc_UniformMatrix3x2fv;
    __gllc_Table->UniformMatrix2x4fv = __gllc_UniformMatrix2x4fv;
    __gllc_Table->UniformMatrix4x2fv = __gllc_UniformMatrix4x2fv;
    __gllc_Table->UniformMatrix3x4fv = __gllc_UniformMatrix3x4fv;
    __gllc_Table->UniformMatrix4x3fv = __gllc_UniformMatrix4x3fv;


    /* GL_VERSION_3_0 */
    __gllc_Table->ColorMaski = __glnop_ColorMaski;
    __gllc_Table->Enablei = __glnop_Enablei;
    __gllc_Table->Disablei = __glnop_Disablei;
    __gllc_Table->BeginTransformFeedback = __glnop_BeginTransformFeedback;
    __gllc_Table->EndTransformFeedback = __glnop_EndTransformFeedback;
    __gllc_Table->ClampColor = __glnop_ClampColor;
    __gllc_Table->BeginConditionalRender = __glnop_BeginConditionalRender;
    __gllc_Table->EndConditionalRender = __glnop_EndConditionalRender;
    __gllc_Table->VertexAttribI1i = __glnop_VertexAttribI1i;
    __gllc_Table->VertexAttribI2i = __glnop_VertexAttribI2i;
    __gllc_Table->VertexAttribI3i = __glnop_VertexAttribI3i;
    __gllc_Table->VertexAttribI4i = __glnop_VertexAttribI4i;
    __gllc_Table->VertexAttribI1ui = __glnop_VertexAttribI1ui;
    __gllc_Table->VertexAttribI2ui = __glnop_VertexAttribI2ui;
    __gllc_Table->VertexAttribI3ui = __glnop_VertexAttribI3ui;
    __gllc_Table->VertexAttribI4ui = __glnop_VertexAttribI4ui;
    __gllc_Table->VertexAttribI1iv = __glnop_VertexAttribI1iv;
    __gllc_Table->VertexAttribI2iv = __glnop_VertexAttribI2iv;
    __gllc_Table->VertexAttribI3iv = __glnop_VertexAttribI3iv;
    __gllc_Table->VertexAttribI4iv = __glnop_VertexAttribI4iv;
    __gllc_Table->VertexAttribI1uiv = __glnop_VertexAttribI1uiv;
    __gllc_Table->VertexAttribI2uiv = __glnop_VertexAttribI2uiv;
    __gllc_Table->VertexAttribI3uiv = __glnop_VertexAttribI3uiv;
    __gllc_Table->VertexAttribI4uiv = __glnop_VertexAttribI4uiv;
    __gllc_Table->VertexAttribI4bv = __glnop_VertexAttribI4bv;
    __gllc_Table->VertexAttribI4sv = __glnop_VertexAttribI4sv;
    __gllc_Table->VertexAttribI4ubv = __glnop_VertexAttribI4ubv;
    __gllc_Table->VertexAttribI4usv = __glnop_VertexAttribI4usv;
    __gllc_Table->Uniform1ui = __glnop_Uniform1ui;
    __gllc_Table->Uniform2ui = __glnop_Uniform2ui;
    __gllc_Table->Uniform3ui = __glnop_Uniform3ui;
    __gllc_Table->Uniform4ui = __glnop_Uniform4ui;
    __gllc_Table->Uniform1uiv = __glnop_Uniform1uiv;
    __gllc_Table->Uniform2uiv = __glnop_Uniform2uiv;
    __gllc_Table->Uniform3uiv = __glnop_Uniform3uiv;
    __gllc_Table->Uniform4uiv = __glnop_Uniform4uiv;
    __gllc_Table->TexParameterIiv = __glnop_TexParameterIiv;
    __gllc_Table->TexParameterIuiv = __glnop_TexParameterIuiv;
    __gllc_Table->ClearBufferiv = __glnop_ClearBufferiv;
    __gllc_Table->ClearBufferuiv = __glnop_ClearBufferuiv;
    __gllc_Table->ClearBufferfv = __glnop_ClearBufferfv;
    __gllc_Table->ClearBufferfi = __glnop_ClearBufferfi;
    /* GL_VERSION_3_1 */
    __gllc_Table->DrawArraysInstanced = __glnop_DrawArraysInstanced;
    __gllc_Table->DrawElementsInstanced = __glnop_DrawElementsInstanced;
    __gllc_Table->CopyBufferSubData = __glnop_CopyBufferSubData;
    __gllc_Table->UniformBlockBinding = __glnop_UniformBlockBinding;
    /* GL_VERSION_3_2 */
    __gllc_Table->DrawElementsBaseVertex = __glnop_DrawElementsBaseVertex;
    __gllc_Table->DrawRangeElementsBaseVertex = __glnop_DrawRangeElementsBaseVertex;
    __gllc_Table->DrawElementsInstancedBaseVertex = __glnop_DrawElementsInstancedBaseVertex;
    __gllc_Table->MultiDrawElementsBaseVertex = __glnop_MultiDrawElementsBaseVertex;
    __gllc_Table->ProvokingVertex = __glnop_ProvokingVertex;
    __gllc_Table->DeleteSync = __glnop_DeleteSync;
    __gllc_Table->WaitSync = __glnop_WaitSync;
    __gllc_Table->TexImage2DMultisample = __glnop_TexImage2DMultisample;
    __gllc_Table->TexImage3DMultisample = __glnop_TexImage3DMultisample;
    __gllc_Table->SampleMaski = __glnop_SampleMaski;
    /* GL_VERSION_3_3 */
    __gllc_Table->BindFragDataLocationIndexed = __glnop_BindFragDataLocationIndexed;
    __gllc_Table->BindSampler = __glnop_BindSampler;
    __gllc_Table->SamplerParameteri = __glnop_SamplerParameteri;
    __gllc_Table->SamplerParameteriv = __glnop_SamplerParameteriv;
    __gllc_Table->SamplerParameterf = __glnop_SamplerParameterf;
    __gllc_Table->SamplerParameterfv = __glnop_SamplerParameterfv;
    __gllc_Table->SamplerParameterIiv = __glnop_SamplerParameterIiv;
    __gllc_Table->SamplerParameterIuiv = __glnop_SamplerParameterIuiv;
    __gllc_Table->QueryCounter = __glnop_QueryCounter;
    __gllc_Table->VertexAttribDivisor = __glnop_VertexAttribDivisor;
    __gllc_Table->VertexAttribP1ui = __glnop_VertexAttribP1ui;
    __gllc_Table->VertexAttribP1uiv = __glnop_VertexAttribP1uiv;
    __gllc_Table->VertexAttribP2ui = __glnop_VertexAttribP2ui;
    __gllc_Table->VertexAttribP2uiv = __glnop_VertexAttribP2uiv;
    __gllc_Table->VertexAttribP3ui = __glnop_VertexAttribP3ui;
    __gllc_Table->VertexAttribP3uiv = __glnop_VertexAttribP3uiv;
    __gllc_Table->VertexAttribP4ui = __glnop_VertexAttribP4ui;
    __gllc_Table->VertexAttribP4uiv = __glnop_VertexAttribP4uiv;
    __gllc_Table->VertexP2ui = __glnop_VertexP2ui;
    __gllc_Table->VertexP2uiv = __glnop_VertexP2uiv;
    __gllc_Table->VertexP3ui = __glnop_VertexP3ui;
    __gllc_Table->VertexP3uiv = __glnop_VertexP3uiv;
    __gllc_Table->VertexP4ui = __glnop_VertexP4ui;
    __gllc_Table->VertexP4uiv = __glnop_VertexP4uiv;
    __gllc_Table->TexCoordP1ui = __glnop_TexCoordP1ui;
    __gllc_Table->TexCoordP1uiv = __glnop_TexCoordP1uiv;
    __gllc_Table->TexCoordP2ui = __glnop_TexCoordP2ui;
    __gllc_Table->TexCoordP2uiv = __glnop_TexCoordP2uiv;
    __gllc_Table->TexCoordP3ui = __glnop_TexCoordP3ui;
    __gllc_Table->TexCoordP3uiv = __glnop_TexCoordP3uiv;
    __gllc_Table->TexCoordP4ui = __glnop_TexCoordP4ui;
    __gllc_Table->TexCoordP4uiv = __glnop_TexCoordP4uiv;
    __gllc_Table->MultiTexCoordP1ui = __glnop_MultiTexCoordP1ui;
    __gllc_Table->MultiTexCoordP1uiv = __glnop_MultiTexCoordP1uiv;
    __gllc_Table->MultiTexCoordP2ui = __glnop_MultiTexCoordP2ui;
    __gllc_Table->MultiTexCoordP2uiv = __glnop_MultiTexCoordP2uiv;
    __gllc_Table->MultiTexCoordP3ui = __glnop_MultiTexCoordP3ui;
    __gllc_Table->MultiTexCoordP3uiv = __glnop_MultiTexCoordP3uiv;
    __gllc_Table->MultiTexCoordP4ui = __glnop_MultiTexCoordP4ui;
    __gllc_Table->MultiTexCoordP4uiv = __glnop_MultiTexCoordP4uiv;
    __gllc_Table->NormalP3ui = __glnop_NormalP3ui;
    __gllc_Table->NormalP3uiv = __glnop_NormalP3uiv;
    __gllc_Table->ColorP3ui = __glnop_ColorP3ui;
    __gllc_Table->ColorP3uiv = __glnop_ColorP3uiv;
    __gllc_Table->ColorP4ui = __glnop_ColorP4ui;
    __gllc_Table->ColorP4uiv = __glnop_ColorP4uiv;
    __gllc_Table->SecondaryColorP3ui = __glnop_SecondaryColorP3ui;
    __gllc_Table->SecondaryColorP3uiv = __glnop_SecondaryColorP3uiv;
    /* GL_VERSION_4_0 */
    __gllc_Table->MinSampleShading = __glnop_MinSampleShading;
    __gllc_Table->BlendEquationi = __glnop_BlendEquationi;
    __gllc_Table->BlendEquationSeparatei = __glnop_BlendEquationSeparatei;
    __gllc_Table->BlendFunci = __glnop_BlendFunci;
    __gllc_Table->BlendFuncSeparatei = __glnop_BlendFuncSeparatei;
    __gllc_Table->DrawArraysIndirect = __glnop_DrawArraysIndirect;
    __gllc_Table->DrawElementsIndirect = __glnop_DrawElementsIndirect;
    __gllc_Table->Uniform1d = __glnop_Uniform1d;
    __gllc_Table->Uniform2d = __glnop_Uniform2d;
    __gllc_Table->Uniform3d = __glnop_Uniform3d;
    __gllc_Table->Uniform4d = __glnop_Uniform4d;
    __gllc_Table->Uniform1dv = __glnop_Uniform1dv;
    __gllc_Table->Uniform2dv = __glnop_Uniform2dv;
    __gllc_Table->Uniform3dv = __glnop_Uniform3dv;
    __gllc_Table->Uniform4dv = __glnop_Uniform4dv;
    __gllc_Table->UniformMatrix2dv = __glnop_UniformMatrix2dv;
    __gllc_Table->UniformMatrix3dv = __glnop_UniformMatrix3dv;
    __gllc_Table->UniformMatrix4dv = __glnop_UniformMatrix4dv;
    __gllc_Table->UniformMatrix2x3dv = __glnop_UniformMatrix2x3dv;
    __gllc_Table->UniformMatrix2x4dv = __glnop_UniformMatrix2x4dv;
    __gllc_Table->UniformMatrix3x2dv = __glnop_UniformMatrix3x2dv;
    __gllc_Table->UniformMatrix3x4dv = __glnop_UniformMatrix3x4dv;
    __gllc_Table->UniformMatrix4x2dv = __glnop_UniformMatrix4x2dv;
    __gllc_Table->UniformMatrix4x3dv = __glnop_UniformMatrix4x3dv;
    __gllc_Table->UniformSubroutinesuiv = __glnop_UniformSubroutinesuiv;
    __gllc_Table->PatchParameteri = __glnop_PatchParameteri;
    __gllc_Table->PatchParameterfv = __glnop_PatchParameterfv;
    __gllc_Table->BindTransformFeedback = __glnop_BindTransformFeedback;
    __gllc_Table->PauseTransformFeedback = __glnop_PauseTransformFeedback;
    __gllc_Table->ResumeTransformFeedback = __glnop_ResumeTransformFeedback;
    __gllc_Table->DrawTransformFeedback = __glnop_DrawTransformFeedback;
    __gllc_Table->DrawTransformFeedbackStream = __glnop_DrawTransformFeedbackStream;
    __gllc_Table->BeginQueryIndexed = __glnop_BeginQueryIndexed;
    __gllc_Table->EndQueryIndexed = __glnop_EndQueryIndexed;
}


