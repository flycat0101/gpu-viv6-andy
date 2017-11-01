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


#ifndef __gc_es_dispatch_h__
#define __gc_es_dispatch_h__

typedef void (GLAPIENTRY  *GLDEBUGPROCKHR)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam);
typedef GLvoid (GLAPIENTRY *__T_NewList)(__GLcontext *,  GLuint, GLenum);
typedef GLvoid (GLAPIENTRY *__T_EndList)(__GLcontext *);
typedef GLvoid (GLAPIENTRY *__T_CallList)(__GLcontext *,  GLuint);
typedef GLvoid (GLAPIENTRY *__T_CallLists)(__GLcontext *,  GLsizei, GLenum, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_DeleteLists)(__GLcontext *,  GLuint, GLsizei);
typedef GLuint (GLAPIENTRY *__T_GenLists)(__GLcontext *,  GLsizei);
typedef GLvoid (GLAPIENTRY *__T_ListBase)(__GLcontext *,  GLuint);
typedef GLvoid (GLAPIENTRY *__T_Begin)(__GLcontext *,  GLenum);
typedef GLvoid (GLAPIENTRY *__T_Bitmap)(__GLcontext *,  GLsizei, GLsizei, GLfloat, GLfloat, GLfloat, GLfloat, const GLubyte *);
typedef GLvoid (GLAPIENTRY *__T_Color3b)(__GLcontext *,  GLbyte, GLbyte, GLbyte);
typedef GLvoid (GLAPIENTRY *__T_Color3bv)(__GLcontext *,  const GLbyte *);
typedef GLvoid (GLAPIENTRY *__T_Color3d)(__GLcontext *,  GLdouble, GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_Color3dv)(__GLcontext *,  const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_Color3f)(__GLcontext *,  GLfloat, GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_Color3fv)(__GLcontext *,  const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_Color3i)(__GLcontext *,  GLint, GLint, GLint);
typedef GLvoid (GLAPIENTRY *__T_Color3iv)(__GLcontext *,  const GLint *);
typedef GLvoid (GLAPIENTRY *__T_Color3s)(__GLcontext *,  GLshort, GLshort, GLshort);
typedef GLvoid (GLAPIENTRY *__T_Color3sv)(__GLcontext *,  const GLshort *);
typedef GLvoid (GLAPIENTRY *__T_Color3ub)(__GLcontext *,  GLubyte, GLubyte, GLubyte);
typedef GLvoid (GLAPIENTRY *__T_Color3ubv)(__GLcontext *,  const GLubyte *);
typedef GLvoid (GLAPIENTRY *__T_Color3ui)(__GLcontext *,  GLuint, GLuint, GLuint);
typedef GLvoid (GLAPIENTRY *__T_Color3uiv)(__GLcontext *,  const GLuint *);
typedef GLvoid (GLAPIENTRY *__T_Color3us)(__GLcontext *,  GLushort, GLushort, GLushort);
typedef GLvoid (GLAPIENTRY *__T_Color3usv)(__GLcontext *,  const GLushort *);
typedef GLvoid (GLAPIENTRY *__T_Color4b)(__GLcontext *,  GLbyte, GLbyte, GLbyte, GLbyte);
typedef GLvoid (GLAPIENTRY *__T_Color4bv)(__GLcontext *,  const GLbyte *);
typedef GLvoid (GLAPIENTRY *__T_Color4d)(__GLcontext *,  GLdouble, GLdouble, GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_Color4dv)(__GLcontext *,  const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_Color4f)(__GLcontext *,  GLfloat, GLfloat, GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_Color4fv)(__GLcontext *,  const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_Color4i)(__GLcontext *,  GLint, GLint, GLint, GLint);
typedef GLvoid (GLAPIENTRY *__T_Color4iv)(__GLcontext *,  const GLint *);
typedef GLvoid (GLAPIENTRY *__T_Color4s)(__GLcontext *,  GLshort, GLshort, GLshort, GLshort);
typedef GLvoid (GLAPIENTRY *__T_Color4sv)(__GLcontext *,  const GLshort *);
typedef GLvoid (GLAPIENTRY *__T_Color4ub)(__GLcontext *,  GLubyte, GLubyte, GLubyte, GLubyte);
typedef GLvoid (GLAPIENTRY *__T_Color4ubv)(__GLcontext *,  const GLubyte *);
typedef GLvoid (GLAPIENTRY *__T_Color4ui)(__GLcontext *,  GLuint, GLuint, GLuint, GLuint);
typedef GLvoid (GLAPIENTRY *__T_Color4uiv)(__GLcontext *,  const GLuint *);
typedef GLvoid (GLAPIENTRY *__T_Color4us)(__GLcontext *,  GLushort, GLushort, GLushort, GLushort);
typedef GLvoid (GLAPIENTRY *__T_Color4usv)(__GLcontext *,  const GLushort *);
typedef GLvoid (GLAPIENTRY *__T_EdgeFlag)(__GLcontext *,  GLboolean);
typedef GLvoid (GLAPIENTRY *__T_EdgeFlagv)(__GLcontext *,  const GLboolean *);
typedef GLvoid (GLAPIENTRY *__T_End)(__GLcontext *);
typedef GLvoid (GLAPIENTRY *__T_Indexd)(__GLcontext *,  GLdouble);
typedef GLvoid (GLAPIENTRY *__T_Indexdv)(__GLcontext *,  const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_Indexf)(__GLcontext *,  GLfloat);
typedef GLvoid (GLAPIENTRY *__T_Indexfv)(__GLcontext *,  const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_Indexi)(__GLcontext *,  GLint);
typedef GLvoid (GLAPIENTRY *__T_Indexiv)(__GLcontext *,  const GLint *);
typedef GLvoid (GLAPIENTRY *__T_Indexs)(__GLcontext *,  GLshort);
typedef GLvoid (GLAPIENTRY *__T_Indexsv)(__GLcontext *,  const GLshort *);
typedef GLvoid (GLAPIENTRY *__T_Normal3b)(__GLcontext *,  GLbyte, GLbyte, GLbyte);
typedef GLvoid (GLAPIENTRY *__T_Normal3bv)(__GLcontext *,  const GLbyte *);
typedef GLvoid (GLAPIENTRY *__T_Normal3d)(__GLcontext *,  GLdouble, GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_Normal3dv)(__GLcontext *,  const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_Normal3f)(__GLcontext *,  GLfloat, GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_Normal3fv)(__GLcontext *,  const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_Normal3i)(__GLcontext *,  GLint, GLint, GLint);
typedef GLvoid (GLAPIENTRY *__T_Normal3iv)(__GLcontext *,  const GLint *);
typedef GLvoid (GLAPIENTRY *__T_Normal3s)(__GLcontext *,  GLshort, GLshort, GLshort);
typedef GLvoid (GLAPIENTRY *__T_Normal3sv)(__GLcontext *,  const GLshort *);
typedef GLvoid (GLAPIENTRY *__T_RasterPos2d)(__GLcontext *,  GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_RasterPos2dv)(__GLcontext *,  const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_RasterPos2f)(__GLcontext *,  GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_RasterPos2fv)(__GLcontext *,  const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_RasterPos2i)(__GLcontext *,  GLint, GLint);
typedef GLvoid (GLAPIENTRY *__T_RasterPos2iv)(__GLcontext *,  const GLint *);
typedef GLvoid (GLAPIENTRY *__T_RasterPos2s)(__GLcontext *,  GLshort, GLshort);
typedef GLvoid (GLAPIENTRY *__T_RasterPos2sv)(__GLcontext *,  const GLshort *);
typedef GLvoid (GLAPIENTRY *__T_RasterPos3d)(__GLcontext *,  GLdouble, GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_RasterPos3dv)(__GLcontext *,  const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_RasterPos3f)(__GLcontext *,  GLfloat, GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_RasterPos3fv)(__GLcontext *,  const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_RasterPos3i)(__GLcontext *,  GLint, GLint, GLint);
typedef GLvoid (GLAPIENTRY *__T_RasterPos3iv)(__GLcontext *,  const GLint *);
typedef GLvoid (GLAPIENTRY *__T_RasterPos3s)(__GLcontext *,  GLshort, GLshort, GLshort);
typedef GLvoid (GLAPIENTRY *__T_RasterPos3sv)(__GLcontext *,  const GLshort *);
typedef GLvoid (GLAPIENTRY *__T_RasterPos4d)(__GLcontext *,  GLdouble, GLdouble, GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_RasterPos4dv)(__GLcontext *,  const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_RasterPos4f)(__GLcontext *,  GLfloat, GLfloat, GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_RasterPos4fv)(__GLcontext *,  const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_RasterPos4i)(__GLcontext *,  GLint, GLint, GLint, GLint);
typedef GLvoid (GLAPIENTRY *__T_RasterPos4iv)(__GLcontext *,  const GLint *);
typedef GLvoid (GLAPIENTRY *__T_RasterPos4s)(__GLcontext *,  GLshort, GLshort, GLshort, GLshort);
typedef GLvoid (GLAPIENTRY *__T_RasterPos4sv)(__GLcontext *,  const GLshort *);
typedef GLvoid (GLAPIENTRY *__T_Rectd)(__GLcontext *,  GLdouble, GLdouble, GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_Rectdv)(__GLcontext *,  const GLdouble *, const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_Rectf)(__GLcontext *,  GLfloat, GLfloat, GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_Rectfv)(__GLcontext *,  const GLfloat *, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_Recti)(__GLcontext *,  GLint, GLint, GLint, GLint);
typedef GLvoid (GLAPIENTRY *__T_Rectiv)(__GLcontext *,  const GLint *, const GLint *);
typedef GLvoid (GLAPIENTRY *__T_Rects)(__GLcontext *,  GLshort, GLshort, GLshort, GLshort);
typedef GLvoid (GLAPIENTRY *__T_Rectsv)(__GLcontext *,  const GLshort *, const GLshort *);
typedef GLvoid (GLAPIENTRY *__T_TexCoord1d)(__GLcontext *,  GLdouble);
typedef GLvoid (GLAPIENTRY *__T_TexCoord1dv)(__GLcontext *,  const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_TexCoord1f)(__GLcontext *,  GLfloat);
typedef GLvoid (GLAPIENTRY *__T_TexCoord1fv)(__GLcontext *,  const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_TexCoord1i)(__GLcontext *,  GLint);
typedef GLvoid (GLAPIENTRY *__T_TexCoord1iv)(__GLcontext *,  const GLint *);
typedef GLvoid (GLAPIENTRY *__T_TexCoord1s)(__GLcontext *,  GLshort);
typedef GLvoid (GLAPIENTRY *__T_TexCoord1sv)(__GLcontext *,  const GLshort *);
typedef GLvoid (GLAPIENTRY *__T_TexCoord2d)(__GLcontext *,  GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_TexCoord2dv)(__GLcontext *,  const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_TexCoord2f)(__GLcontext *,  GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_TexCoord2fv)(__GLcontext *,  const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_TexCoord2i)(__GLcontext *,  GLint, GLint);
typedef GLvoid (GLAPIENTRY *__T_TexCoord2iv)(__GLcontext *,  const GLint *);
typedef GLvoid (GLAPIENTRY *__T_TexCoord2s)(__GLcontext *,  GLshort, GLshort);
typedef GLvoid (GLAPIENTRY *__T_TexCoord2sv)(__GLcontext *,  const GLshort *);
typedef GLvoid (GLAPIENTRY *__T_TexCoord3d)(__GLcontext *,  GLdouble, GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_TexCoord3dv)(__GLcontext *,  const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_TexCoord3f)(__GLcontext *,  GLfloat, GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_TexCoord3fv)(__GLcontext *,  const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_TexCoord3i)(__GLcontext *,  GLint, GLint, GLint);
typedef GLvoid (GLAPIENTRY *__T_TexCoord3iv)(__GLcontext *,  const GLint *);
typedef GLvoid (GLAPIENTRY *__T_TexCoord3s)(__GLcontext *,  GLshort, GLshort, GLshort);
typedef GLvoid (GLAPIENTRY *__T_TexCoord3sv)(__GLcontext *,  const GLshort *);
typedef GLvoid (GLAPIENTRY *__T_TexCoord4d)(__GLcontext *,  GLdouble, GLdouble, GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_TexCoord4dv)(__GLcontext *,  const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_TexCoord4f)(__GLcontext *,  GLfloat, GLfloat, GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_TexCoord4fv)(__GLcontext *,  const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_TexCoord4i)(__GLcontext *,  GLint, GLint, GLint, GLint);
typedef GLvoid (GLAPIENTRY *__T_TexCoord4iv)(__GLcontext *,  const GLint *);
typedef GLvoid (GLAPIENTRY *__T_TexCoord4s)(__GLcontext *,  GLshort, GLshort, GLshort, GLshort);
typedef GLvoid (GLAPIENTRY *__T_TexCoord4sv)(__GLcontext *,  const GLshort *);
typedef GLvoid (GLAPIENTRY *__T_Vertex2d)(__GLcontext *,  GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_Vertex2dv)(__GLcontext *,  const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_Vertex2f)(__GLcontext *,  GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_Vertex2fv)(__GLcontext *,  const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_Vertex2i)(__GLcontext *,  GLint, GLint);
typedef GLvoid (GLAPIENTRY *__T_Vertex2iv)(__GLcontext *,  const GLint *);
typedef GLvoid (GLAPIENTRY *__T_Vertex2s)(__GLcontext *,  GLshort, GLshort);
typedef GLvoid (GLAPIENTRY *__T_Vertex2sv)(__GLcontext *,  const GLshort *);
typedef GLvoid (GLAPIENTRY *__T_Vertex3d)(__GLcontext *,  GLdouble, GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_Vertex3dv)(__GLcontext *,  const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_Vertex3f)(__GLcontext *,  GLfloat, GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_Vertex3fv)(__GLcontext *,  const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_Vertex3i)(__GLcontext *,  GLint, GLint, GLint);
typedef GLvoid (GLAPIENTRY *__T_Vertex3iv)(__GLcontext *,  const GLint *);
typedef GLvoid (GLAPIENTRY *__T_Vertex3s)(__GLcontext *,  GLshort, GLshort, GLshort);
typedef GLvoid (GLAPIENTRY *__T_Vertex3sv)(__GLcontext *,  const GLshort *);
typedef GLvoid (GLAPIENTRY *__T_Vertex4d)(__GLcontext *,  GLdouble, GLdouble, GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_Vertex4dv)(__GLcontext *,  const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_Vertex4f)(__GLcontext *,  GLfloat, GLfloat, GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_Vertex4fv)(__GLcontext *,  const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_Vertex4i)(__GLcontext *,  GLint, GLint, GLint, GLint);
typedef GLvoid (GLAPIENTRY *__T_Vertex4iv)(__GLcontext *,  const GLint *);
typedef GLvoid (GLAPIENTRY *__T_Vertex4s)(__GLcontext *,  GLshort, GLshort, GLshort, GLshort);
typedef GLvoid (GLAPIENTRY *__T_Vertex4sv)(__GLcontext *,  const GLshort *);
typedef GLvoid (GLAPIENTRY *__T_DrawElements)(__GLcontext *,  GLenum, GLsizei, GLenum, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_ArrayElement)(__GLcontext *,  GLint);
typedef GLvoid (GLAPIENTRY *__T_DrawArrays)(__GLcontext *,  GLenum, GLint, GLsizei);
typedef GLvoid (GLAPIENTRY *__T_ClipPlane)(__GLcontext *,  GLenum, const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_ColorMaterial)(__GLcontext *,  GLenum, GLenum);
typedef GLvoid (GLAPIENTRY *__T_CullFace)(__GLcontext *,  GLenum);
typedef GLvoid (GLAPIENTRY *__T_Fogf)(__GLcontext *,  GLenum, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_Fogfv)(__GLcontext *,  GLenum, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_Fogi)(__GLcontext *,  GLenum, GLint);
typedef GLvoid (GLAPIENTRY *__T_Fogiv)(__GLcontext *,  GLenum, const GLint *);
typedef GLvoid (GLAPIENTRY *__T_FrontFace)(__GLcontext *,  GLenum);
typedef GLvoid (GLAPIENTRY *__T_Hint)(__GLcontext *,  GLenum, GLenum);
typedef GLvoid (GLAPIENTRY *__T_Lightf)(__GLcontext *,  GLenum, GLenum, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_Lightfv)(__GLcontext *,  GLenum, GLenum, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_Lighti)(__GLcontext *,  GLenum, GLenum, GLint);
typedef GLvoid (GLAPIENTRY *__T_Lightiv)(__GLcontext *,  GLenum, GLenum, const GLint *);
typedef GLvoid (GLAPIENTRY *__T_LightModelf)(__GLcontext *,  GLenum, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_LightModelfv)(__GLcontext *,  GLenum, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_LightModeli)(__GLcontext *,  GLenum, GLint);
typedef GLvoid (GLAPIENTRY *__T_LightModeliv)(__GLcontext *,  GLenum, const GLint *);
typedef GLvoid (GLAPIENTRY *__T_LineStipple)(__GLcontext *,  GLint, GLushort);
typedef GLvoid (GLAPIENTRY *__T_LineWidth)(__GLcontext *,  GLfloat);
typedef GLvoid (GLAPIENTRY *__T_Materialf)(__GLcontext *,  GLenum, GLenum, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_Materialfv)(__GLcontext *,  GLenum, GLenum, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_Materiali)(__GLcontext *,  GLenum, GLenum, GLint);
typedef GLvoid (GLAPIENTRY *__T_Materialiv)(__GLcontext *,  GLenum, GLenum, const GLint *);
typedef GLvoid (GLAPIENTRY *__T_PointSize)(__GLcontext *,  GLfloat);
typedef GLvoid (GLAPIENTRY *__T_PolygonMode)(__GLcontext *,  GLenum, GLenum);
typedef GLvoid (GLAPIENTRY *__T_PolygonStipple)(__GLcontext *,  const GLubyte *);
typedef GLvoid (GLAPIENTRY *__T_Scissor)(__GLcontext *,  GLint, GLint, GLsizei, GLsizei);
typedef GLvoid (GLAPIENTRY *__T_ShadeModel)(__GLcontext *,  GLenum);
typedef GLvoid (GLAPIENTRY *__T_TexParameterf)(__GLcontext *,  GLenum, GLenum, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_TexParameterfv)(__GLcontext *,  GLenum, GLenum, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_TexParameteri)(__GLcontext *,  GLenum, GLenum, GLint);
typedef GLvoid (GLAPIENTRY *__T_TexParameteriv)(__GLcontext *,  GLenum, GLenum, const GLint *);
typedef GLvoid (GLAPIENTRY *__T_TexImage1D)(__GLcontext *,  GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_TexImage2D)(__GLcontext *,  GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_TexEnvf)(__GLcontext *,  GLenum, GLenum, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_TexEnvfv)(__GLcontext *,  GLenum, GLenum, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_TexEnvi)(__GLcontext *,  GLenum, GLenum, GLint);
typedef GLvoid (GLAPIENTRY *__T_TexEnviv)(__GLcontext *,  GLenum, GLenum, const GLint *);
typedef GLvoid (GLAPIENTRY *__T_TexGend)(__GLcontext *,  GLenum, GLenum, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_TexGendv)(__GLcontext *,  GLenum, GLenum, const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_TexGenf)(__GLcontext *,  GLenum, GLenum, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_TexGenfv)(__GLcontext *,  GLenum, GLenum, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_TexGeni)(__GLcontext *,  GLenum, GLenum, GLint);
typedef GLvoid (GLAPIENTRY *__T_TexGeniv)(__GLcontext *,  GLenum, GLenum, const GLint *);
typedef GLvoid (GLAPIENTRY *__T_FeedbackBuffer)(__GLcontext *,  GLsizei, GLenum, GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_SelectBuffer)(__GLcontext *,  GLsizei, GLuint *);
typedef GLint (GLAPIENTRY *__T_RenderMode)(__GLcontext *,  GLenum);
typedef GLvoid (GLAPIENTRY *__T_InitNames)(__GLcontext *);
typedef GLvoid (GLAPIENTRY *__T_LoadName)(__GLcontext *,  GLuint);
typedef GLvoid (GLAPIENTRY *__T_PassThrough)(__GLcontext *,  GLfloat);
typedef GLvoid (GLAPIENTRY *__T_PopName)(__GLcontext *);
typedef GLvoid (GLAPIENTRY *__T_PushName)(__GLcontext *,  GLuint);
typedef GLvoid (GLAPIENTRY *__T_DrawBuffer)(__GLcontext *,  GLenum);
typedef GLvoid (GLAPIENTRY *__T_Clear)(__GLcontext *,  GLbitfield);
typedef GLvoid (GLAPIENTRY *__T_ClearAccum)(__GLcontext *,  GLfloat, GLfloat, GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_ClearIndex)(__GLcontext *,  GLfloat);
typedef GLvoid (GLAPIENTRY *__T_ClearColor)(__GLcontext *,  GLclampf, GLclampf, GLclampf, GLclampf);
typedef GLvoid (GLAPIENTRY *__T_ClearStencil)(__GLcontext *,  GLint);
typedef GLvoid (GLAPIENTRY *__T_ClearDepth)(__GLcontext *,  GLclampd);
typedef GLvoid (GLAPIENTRY *__T_StencilMask)(__GLcontext *,  GLuint);
typedef GLvoid (GLAPIENTRY *__T_ColorMask)(__GLcontext *,  GLboolean, GLboolean, GLboolean, GLboolean);
typedef GLvoid (GLAPIENTRY *__T_DepthMask)(__GLcontext *,  GLboolean);
typedef GLvoid (GLAPIENTRY *__T_IndexMask)(__GLcontext *,  GLuint);
typedef GLvoid (GLAPIENTRY *__T_Accum)(__GLcontext *,  GLenum, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_Disable)(__GLcontext *,  GLenum);
typedef GLvoid (GLAPIENTRY *__T_Enable)(__GLcontext *,  GLenum);
typedef GLvoid (GLAPIENTRY *__T_Finish)(__GLcontext *);
typedef GLvoid (GLAPIENTRY *__T_Flush)(__GLcontext *);
typedef GLvoid (GLAPIENTRY *__T_PopAttrib)(__GLcontext *);
typedef GLvoid (GLAPIENTRY *__T_PushAttrib)(__GLcontext *,  GLbitfield);
typedef GLvoid (GLAPIENTRY *__T_Map1d)(__GLcontext *,  GLenum, GLdouble, GLdouble, GLint, GLint, const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_Map1f)(__GLcontext *,  GLenum, GLfloat, GLfloat, GLint, GLint, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_Map2d)(__GLcontext *,  GLenum, GLdouble, GLdouble, GLint, GLint, GLdouble, GLdouble, GLint, GLint, const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_Map2f)(__GLcontext *,  GLenum, GLfloat, GLfloat, GLint, GLint, GLfloat, GLfloat, GLint, GLint, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_MapGrid1d)(__GLcontext *,  GLint, GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_MapGrid1f)(__GLcontext *,  GLint, GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_MapGrid2d)(__GLcontext *,  GLint, GLdouble, GLdouble, GLint, GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_MapGrid2f)(__GLcontext *,  GLint, GLfloat, GLfloat, GLint, GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_EvalCoord1d)(__GLcontext *,  GLdouble);
typedef GLvoid (GLAPIENTRY *__T_EvalCoord1dv)(__GLcontext *,  const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_EvalCoord1f)(__GLcontext *,  GLfloat);
typedef GLvoid (GLAPIENTRY *__T_EvalCoord1fv)(__GLcontext *,  const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_EvalCoord2d)(__GLcontext *,  GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_EvalCoord2dv)(__GLcontext *,  const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_EvalCoord2f)(__GLcontext *,  GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_EvalCoord2fv)(__GLcontext *,  const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_EvalMesh1)(__GLcontext *,  GLenum, GLint, GLint);
typedef GLvoid (GLAPIENTRY *__T_EvalPoint1)(__GLcontext *,  GLint);
typedef GLvoid (GLAPIENTRY *__T_EvalMesh2)(__GLcontext *,  GLenum, GLint, GLint, GLint, GLint);
typedef GLvoid (GLAPIENTRY *__T_EvalPoint2)(__GLcontext *,  GLint, GLint);
typedef GLvoid (GLAPIENTRY *__T_AlphaFunc)(__GLcontext *,  GLenum, GLclampf);
typedef GLvoid (GLAPIENTRY *__T_BlendFunc)(__GLcontext *,  GLenum, GLenum);
typedef GLvoid (GLAPIENTRY *__T_LogicOp)(__GLcontext *,  GLenum);
typedef GLvoid (GLAPIENTRY *__T_StencilFunc)(__GLcontext *,  GLenum, GLint, GLuint);
typedef GLvoid (GLAPIENTRY *__T_StencilOp)(__GLcontext *,  GLenum, GLenum, GLenum);
typedef GLvoid (GLAPIENTRY *__T_DepthFunc)(__GLcontext *,  GLenum);
typedef GLvoid (GLAPIENTRY *__T_PixelZoom)(__GLcontext *,  GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_PixelTransferf)(__GLcontext *,  GLenum, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_PixelTransferi)(__GLcontext *,  GLenum, GLint);
typedef GLvoid (GLAPIENTRY *__T_PixelStoref)(__GLcontext *,  GLenum, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_PixelStorei)(__GLcontext *,  GLenum, GLint);
typedef GLvoid (GLAPIENTRY *__T_PixelMapfv)(__GLcontext *,  GLenum, GLint, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_PixelMapuiv)(__GLcontext *,  GLenum, GLint, const GLuint *);
typedef GLvoid (GLAPIENTRY *__T_PixelMapusv)(__GLcontext *,  GLenum, GLint, const GLushort *);
typedef GLvoid (GLAPIENTRY *__T_ReadBuffer)(__GLcontext *,  GLenum);
typedef GLvoid (GLAPIENTRY *__T_CopyPixels)(__GLcontext *,  GLint, GLint, GLsizei, GLsizei, GLenum);
typedef GLvoid (GLAPIENTRY *__T_ReadPixels)(__GLcontext *,  GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_DrawPixels)(__GLcontext *,  GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_GetBooleanv)(__GLcontext *,  GLenum, GLboolean *);
typedef GLvoid (GLAPIENTRY *__T_GetClipPlane)(__GLcontext *,  GLenum, GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_GetDoublev)(__GLcontext *,  GLenum, GLdouble *);
typedef GLenum (GLAPIENTRY *__T_GetError)(__GLcontext *);
typedef GLvoid (GLAPIENTRY *__T_GetFloatv)(__GLcontext *,  GLenum, GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_GetIntegerv)(__GLcontext *,  GLenum, GLint *);
typedef GLvoid (GLAPIENTRY *__T_GetLightfv)(__GLcontext *,  GLenum, GLenum, GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_GetLightiv)(__GLcontext *,  GLenum, GLenum, GLint *);
typedef GLvoid (GLAPIENTRY *__T_GetMapdv)(__GLcontext *,  GLenum, GLenum, GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_GetMapfv)(__GLcontext *,  GLenum, GLenum, GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_GetMapiv)(__GLcontext *,  GLenum, GLenum, GLint *);
typedef GLvoid (GLAPIENTRY *__T_GetMaterialfv)(__GLcontext *,  GLenum, GLenum, GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_GetMaterialiv)(__GLcontext *,  GLenum, GLenum, GLint *);
typedef GLvoid (GLAPIENTRY *__T_GetPixelMapfv)(__GLcontext *,  GLenum, GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_GetPixelMapuiv)(__GLcontext *,  GLenum, GLuint *);
typedef GLvoid (GLAPIENTRY *__T_GetPixelMapusv)(__GLcontext *,  GLenum, GLushort *);
typedef GLvoid (GLAPIENTRY *__T_GetPolygonStipple)(__GLcontext *,  GLubyte *);
typedef const GLubyte * (GLAPIENTRY *__T_GetString)(__GLcontext *,  GLenum);
typedef GLvoid (GLAPIENTRY *__T_GetTexEnvfv)(__GLcontext *,  GLenum, GLenum, GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_GetTexEnviv)(__GLcontext *,  GLenum, GLenum, GLint *);
typedef GLvoid (GLAPIENTRY *__T_GetTexGendv)(__GLcontext *,  GLenum, GLenum, GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_GetTexGenfv)(__GLcontext *,  GLenum, GLenum, GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_GetTexGeniv)(__GLcontext *,  GLenum, GLenum, GLint *);
typedef GLvoid (GLAPIENTRY *__T_GetTexImage)(__GLcontext *,  GLenum, GLint, GLenum, GLenum, GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_GetTexParameterfv)(__GLcontext *,  GLenum, GLenum, GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_GetTexParameteriv)(__GLcontext *,  GLenum, GLenum, GLint *);
typedef GLvoid (GLAPIENTRY *__T_GetTexLevelParameterfv)(__GLcontext *,  GLenum, GLint, GLenum, GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_GetTexLevelParameteriv)(__GLcontext *,  GLenum, GLint, GLenum, GLint *);
typedef GLboolean (GLAPIENTRY *__T_IsEnabled)(__GLcontext *,  GLenum);
typedef GLboolean (GLAPIENTRY *__T_IsList)(__GLcontext *,  GLuint);
typedef GLvoid (GLAPIENTRY *__T_DepthRange)(__GLcontext *,  GLclampd, GLclampd);
typedef GLvoid (GLAPIENTRY *__T_Frustum)(__GLcontext *,  GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_LoadIdentity)(__GLcontext *);
typedef GLvoid (GLAPIENTRY *__T_LoadMatrixf)(__GLcontext *,  const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_LoadMatrixd)(__GLcontext *,  const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_MatrixMode)(__GLcontext *,  GLenum);
typedef GLvoid (GLAPIENTRY *__T_MultMatrixf)(__GLcontext *,  const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_MultMatrixd)(__GLcontext *,  const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_Ortho)(__GLcontext *,  GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_PopMatrix)(__GLcontext *);
typedef GLvoid (GLAPIENTRY *__T_PushMatrix)(__GLcontext *);
typedef GLvoid (GLAPIENTRY *__T_Rotated)(__GLcontext *,  GLdouble, GLdouble, GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_Rotatef)(__GLcontext *,  GLfloat, GLfloat, GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_Scaled)(__GLcontext *,  GLdouble, GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_Scalef)(__GLcontext *,  GLfloat, GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_Translated)(__GLcontext *,  GLdouble, GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_Translatef)(__GLcontext *,  GLfloat, GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_Viewport)(__GLcontext *,  GLint, GLint, GLsizei, GLsizei);
typedef GLvoid (GLAPIENTRY *__T_ColorSubTable)(__GLcontext *,  GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_ColorTable)(__GLcontext *,  GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_CopyColorTable)(__GLcontext *,  GLenum, GLenum, GLint, GLint, GLsizei);
typedef GLvoid (GLAPIENTRY *__T_GetColorTable)(__GLcontext *,  GLenum, GLenum, GLenum, GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_GetColorTableParameterfv)(__GLcontext *,  GLenum, GLenum, GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_GetColorTableParameteriv)(__GLcontext *,  GLenum, GLenum, GLint *);
typedef GLvoid (GLAPIENTRY *__T_ColorPointer)(__GLcontext *,  GLint, GLenum, GLsizei, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_DisableClientState)(__GLcontext *,  GLenum);
typedef GLvoid (GLAPIENTRY *__T_EdgeFlagPointer)(__GLcontext *,  GLsizei, const GLboolean *);
typedef GLvoid (GLAPIENTRY *__T_EnableClientState)(__GLcontext *,  GLenum);
typedef GLvoid (GLAPIENTRY *__T_GetPointerv)(__GLcontext *,  GLenum, GLvoid* *);
typedef GLvoid (GLAPIENTRY *__T_IndexPointer)(__GLcontext *,  GLenum, GLsizei, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_InterleavedArrays)(__GLcontext *,  GLenum, GLsizei, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_NormalPointer)(__GLcontext *,  GLenum, GLsizei, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_TexCoordPointer)(__GLcontext *,  GLint, GLenum, GLsizei, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_VertexPointer)(__GLcontext *,  GLint, GLenum, GLsizei, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_PolygonOffset)(__GLcontext *,  GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_CopyTexImage1D)(__GLcontext *,  GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLint);
typedef GLvoid (GLAPIENTRY *__T_CopyTexImage2D)(__GLcontext *,  GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint);
typedef GLvoid (GLAPIENTRY *__T_CopyTexSubImage1D)(__GLcontext *,  GLenum, GLint, GLint, GLint, GLint, GLsizei);
typedef GLvoid (GLAPIENTRY *__T_CopyTexSubImage2D)(__GLcontext *,  GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
typedef GLvoid (GLAPIENTRY *__T_TexSubImage1D)(__GLcontext *,  GLenum, GLint, GLint, GLsizei, GLenum, GLenum, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_TexSubImage2D)(__GLcontext *,  GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
typedef GLboolean (GLAPIENTRY *__T_AreTexturesResident)(__GLcontext *,  GLsizei, const GLuint *, GLboolean *);
typedef GLvoid (GLAPIENTRY *__T_BindTexture)(__GLcontext *,  GLenum, GLuint);
typedef GLvoid (GLAPIENTRY *__T_DeleteTextures)(__GLcontext *,  GLsizei, const GLuint *);
typedef GLvoid (GLAPIENTRY *__T_GenTextures)(__GLcontext *,  GLsizei, GLuint *);
typedef GLboolean (GLAPIENTRY *__T_IsTexture)(__GLcontext *,  GLuint);
typedef GLvoid (GLAPIENTRY *__T_PrioritizeTextures)(__GLcontext *,  GLsizei, const GLuint *, const GLclampf *);
typedef GLvoid (GLAPIENTRY *__T_Indexub)(__GLcontext *,  GLubyte);
typedef GLvoid (GLAPIENTRY *__T_Indexubv)(__GLcontext *,  const GLubyte *);
typedef GLvoid (GLAPIENTRY *__T_PopClientAttrib)(__GLcontext *);
typedef GLvoid (GLAPIENTRY *__T_PushClientAttrib)(__GLcontext *,  GLbitfield);

#if GL_VERSION_1_2
typedef GLvoid (GLAPIENTRY *__T_BlendColor)(__GLcontext *,  GLclampf, GLclampf, GLclampf, GLclampf);
typedef GLvoid (GLAPIENTRY *__T_BlendEquation)(__GLcontext *,  GLenum);
typedef GLvoid (GLAPIENTRY *__T_DrawRangeElements)(__GLcontext *,  GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_ColorTable)(__GLcontext *,  GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_ColorTableParameterfv)(__GLcontext *,  GLenum, GLenum, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_ColorTableParameteriv)(__GLcontext *,  GLenum, GLenum, const GLint *);
typedef GLvoid (GLAPIENTRY *__T_CopyColorTable)(__GLcontext *,  GLenum, GLenum, GLint, GLint, GLsizei);
typedef GLvoid (GLAPIENTRY *__T_GetColorTable)(__GLcontext *,  GLenum, GLenum, GLenum, GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_GetColorTableParameterfv)(__GLcontext *,  GLenum, GLenum, GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_GetColorTableParameteriv)(__GLcontext *,  GLenum, GLenum, GLint *);
typedef GLvoid (GLAPIENTRY *__T_ColorSubTable)(__GLcontext *,  GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_CopyColorSubTable)(__GLcontext *,  GLenum, GLsizei, GLint, GLint, GLsizei);
typedef GLvoid (GLAPIENTRY *__T_ConvolutionFilter1D)(__GLcontext *,  GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_ConvolutionFilter2D)(__GLcontext *,  GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_ConvolutionParameterf)(__GLcontext *,  GLenum, GLenum, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_ConvolutionParameterfv)(__GLcontext *,  GLenum, GLenum, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_ConvolutionParameteri)(__GLcontext *,  GLenum, GLenum, GLint);
typedef GLvoid (GLAPIENTRY *__T_ConvolutionParameteriv)(__GLcontext *,  GLenum, GLenum, const GLint *);
typedef GLvoid (GLAPIENTRY *__T_CopyConvolutionFilter1D)(__GLcontext *,  GLenum, GLenum, GLint, GLint, GLsizei);
typedef GLvoid (GLAPIENTRY *__T_CopyConvolutionFilter2D)(__GLcontext *,  GLenum, GLenum, GLint, GLint, GLsizei, GLsizei);
typedef GLvoid (GLAPIENTRY *__T_GetConvolutionFilter)(__GLcontext *,  GLenum, GLenum, GLenum, GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_GetConvolutionParameterfv)(__GLcontext *,  GLenum, GLenum, GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_GetConvolutionParameteriv)(__GLcontext *,  GLenum, GLenum, GLint *);
typedef GLvoid (GLAPIENTRY *__T_GetSeparableFilter)(__GLcontext *,  GLenum, GLenum, GLenum, GLvoid *, GLvoid *, GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_SeparableFilter2D)(__GLcontext *,  GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_GetHistogram)(__GLcontext *,  GLenum, GLboolean, GLenum, GLenum, GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_GetHistogramParameterfv)(__GLcontext *,  GLenum, GLenum, GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_GetHistogramParameteriv)(__GLcontext *,  GLenum, GLenum, GLint *);
typedef GLvoid (GLAPIENTRY *__T_GetMinmax)(__GLcontext *,  GLenum, GLboolean, GLenum, GLenum, GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_GetMinmaxParameterfv)(__GLcontext *,  GLenum, GLenum, GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_GetMinmaxParameteriv)(__GLcontext *,  GLenum, GLenum, GLint *);
typedef GLvoid (GLAPIENTRY *__T_Histogram)(__GLcontext *,  GLenum, GLsizei, GLenum, GLboolean);
typedef GLvoid (GLAPIENTRY *__T_Minmax)(__GLcontext *,  GLenum, GLenum, GLboolean);
typedef GLvoid (GLAPIENTRY *__T_ResetHistogram)(__GLcontext *,  GLenum);
typedef GLvoid (GLAPIENTRY *__T_ResetMinmax)(__GLcontext *,  GLenum);
typedef GLvoid (GLAPIENTRY *__T_TexImage3D)(__GLcontext *,  GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_TexSubImage3D)(__GLcontext *,  GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_CopyTexSubImage3D)(__GLcontext *,  GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
#endif
#if GL_VERSION_1_3
typedef GLvoid (GLAPIENTRY *__T_ActiveTexture)(__GLcontext *,  GLenum);
typedef GLvoid (GLAPIENTRY *__T_ClientActiveTexture)(__GLcontext *,  GLenum);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord1d)(__GLcontext *,  GLenum, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord1dv)(__GLcontext *,  GLenum, const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord1f)(__GLcontext *,  GLenum, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord1fv)(__GLcontext *,  GLenum, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord1i)(__GLcontext *,  GLenum, GLint);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord1iv)(__GLcontext *,  GLenum, const GLint *);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord1s)(__GLcontext *,  GLenum, GLshort);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord1sv)(__GLcontext *,  GLenum, const GLshort *);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord2d)(__GLcontext *,  GLenum, GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord2dv)(__GLcontext *,  GLenum, const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord2f)(__GLcontext *,  GLenum, GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord2fv)(__GLcontext *,  GLenum, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord2i)(__GLcontext *,  GLenum, GLint, GLint);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord2iv)(__GLcontext *,  GLenum, const GLint *);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord2s)(__GLcontext *,  GLenum, GLshort, GLshort);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord2sv)(__GLcontext *,  GLenum, const GLshort *);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord3d)(__GLcontext *,  GLenum, GLdouble, GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord3dv)(__GLcontext *,  GLenum, const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord3f)(__GLcontext *,  GLenum, GLfloat, GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord3fv)(__GLcontext *,  GLenum, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord3i)(__GLcontext *,  GLenum, GLint, GLint, GLint);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord3iv)(__GLcontext *,  GLenum, const GLint *);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord3s)(__GLcontext *,  GLenum, GLshort, GLshort, GLshort);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord3sv)(__GLcontext *,  GLenum, const GLshort *);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord4d)(__GLcontext *,  GLenum, GLdouble, GLdouble, GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord4dv)(__GLcontext *,  GLenum, const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord4f)(__GLcontext *,  GLenum, GLfloat, GLfloat, GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord4fv)(__GLcontext *,  GLenum, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord4i)(__GLcontext *,  GLenum, GLint, GLint, GLint, GLint);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord4iv)(__GLcontext *,  GLenum, const GLint *);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord4s)(__GLcontext *,  GLenum, GLshort, GLshort, GLshort, GLshort);
typedef GLvoid (GLAPIENTRY *__T_MultiTexCoord4sv)(__GLcontext *,  GLenum, const GLshort *);
typedef GLvoid (GLAPIENTRY *__T_LoadTransposeMatrixf)(__GLcontext *,  const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_LoadTransposeMatrixd)(__GLcontext *,  const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_MultTransposeMatrixf)(__GLcontext *,  const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_MultTransposeMatrixd)(__GLcontext *,  const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_SampleCoverage)(__GLcontext *,  GLclampf, GLboolean);
typedef GLvoid (GLAPIENTRY *__T_CompressedTexImage3D)(__GLcontext *,  GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_CompressedTexImage2D)(__GLcontext *,  GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_CompressedTexImage1D)(__GLcontext *,  GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_CompressedTexSubImage3D)(__GLcontext *,  GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_CompressedTexSubImage2D)(__GLcontext *,  GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_CompressedTexSubImage1D)(__GLcontext *,  GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_GetCompressedTexImage)(__GLcontext *,  GLenum, GLint, GLvoid *);
#endif
#if GL_VERSION_1_4
typedef GLvoid (GLAPIENTRY *__T_BlendFuncSeparate)(__GLcontext *,  GLenum, GLenum, GLenum, GLenum);
typedef GLvoid (GLAPIENTRY *__T_FogCoordf)(__GLcontext *,  GLfloat);
typedef GLvoid (GLAPIENTRY *__T_FogCoordfv)(__GLcontext *,  const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_FogCoordd)(__GLcontext *,  GLdouble);
typedef GLvoid (GLAPIENTRY *__T_FogCoorddv)(__GLcontext *,  const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_FogCoordPointer)(__GLcontext *,  GLenum, GLsizei, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_MultiDrawArrays)(__GLcontext *,  GLenum, GLint *, GLsizei *, GLsizei);
typedef GLvoid (GLAPIENTRY *__T_MultiDrawElements)(__GLcontext *,  GLenum, const GLsizei *, GLenum, const GLvoid* *, GLsizei);
typedef GLvoid (GLAPIENTRY *__T_PointParameterf)(__GLcontext *,  GLenum, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_PointParameterfv)(__GLcontext *,  GLenum, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_PointParameteri)(__GLcontext *,  GLenum, GLint);
typedef GLvoid (GLAPIENTRY *__T_PointParameteriv)(__GLcontext *,  GLenum, const GLint *);
typedef GLvoid (GLAPIENTRY *__T_SecondaryColor3b)(__GLcontext *,  GLbyte, GLbyte, GLbyte);
typedef GLvoid (GLAPIENTRY *__T_SecondaryColor3bv)(__GLcontext *,  const GLbyte *);
typedef GLvoid (GLAPIENTRY *__T_SecondaryColor3d)(__GLcontext *,  GLdouble, GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_SecondaryColor3dv)(__GLcontext *,  const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_SecondaryColor3f)(__GLcontext *,  GLfloat, GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_SecondaryColor3fv)(__GLcontext *,  const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_SecondaryColor3i)(__GLcontext *,  GLint, GLint, GLint);
typedef GLvoid (GLAPIENTRY *__T_SecondaryColor3iv)(__GLcontext *,  const GLint *);
typedef GLvoid (GLAPIENTRY *__T_SecondaryColor3s)(__GLcontext *,  GLshort, GLshort, GLshort);
typedef GLvoid (GLAPIENTRY *__T_SecondaryColor3sv)(__GLcontext *,  const GLshort *);
typedef GLvoid (GLAPIENTRY *__T_SecondaryColor3ub)(__GLcontext *,  GLubyte, GLubyte, GLubyte);
typedef GLvoid (GLAPIENTRY *__T_SecondaryColor3ubv)(__GLcontext *,  const GLubyte *);
typedef GLvoid (GLAPIENTRY *__T_SecondaryColor3ui)(__GLcontext *,  GLuint, GLuint, GLuint);
typedef GLvoid (GLAPIENTRY *__T_SecondaryColor3uiv)(__GLcontext *,  const GLuint *);
typedef GLvoid (GLAPIENTRY *__T_SecondaryColor3us)(__GLcontext *,  GLushort, GLushort, GLushort);
typedef GLvoid (GLAPIENTRY *__T_SecondaryColor3usv)(__GLcontext *,  const GLushort *);
typedef GLvoid (GLAPIENTRY *__T_SecondaryColorPointer)(__GLcontext *,  GLint, GLenum, GLsizei, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_WindowPos2d)(__GLcontext *,  GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_WindowPos2dv)(__GLcontext *,  const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_WindowPos2f)(__GLcontext *,  GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_WindowPos2fv)(__GLcontext *,  const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_WindowPos2i)(__GLcontext *,  GLint, GLint);
typedef GLvoid (GLAPIENTRY *__T_WindowPos2iv)(__GLcontext *,  const GLint *);
typedef GLvoid (GLAPIENTRY *__T_WindowPos2s)(__GLcontext *,  GLshort, GLshort);
typedef GLvoid (GLAPIENTRY *__T_WindowPos2sv)(__GLcontext *,  const GLshort *);
typedef GLvoid (GLAPIENTRY *__T_WindowPos3d)(__GLcontext *,  GLdouble, GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_WindowPos3dv)(__GLcontext *,  const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_WindowPos3f)(__GLcontext *,  GLfloat, GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_WindowPos3fv)(__GLcontext *,  const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_WindowPos3i)(__GLcontext *,  GLint, GLint, GLint);
typedef GLvoid (GLAPIENTRY *__T_WindowPos3iv)(__GLcontext *,  const GLint *);
typedef GLvoid (GLAPIENTRY *__T_WindowPos3s)(__GLcontext *,  GLshort, GLshort, GLshort);
typedef GLvoid (GLAPIENTRY *__T_WindowPos3sv)(__GLcontext *,  const GLshort *);
#endif
#if GL_VERSION_1_5
typedef GLvoid (GLAPIENTRY *__T_GenQueries)(__GLcontext *,  GLsizei, GLuint *);
typedef GLvoid (GLAPIENTRY *__T_DeleteQueries)(__GLcontext *,  GLsizei, const GLuint *);
typedef GLboolean (GLAPIENTRY *__T_IsQuery)(__GLcontext *,  GLuint);
typedef GLvoid (GLAPIENTRY *__T_BeginQuery)(__GLcontext *,  GLenum, GLuint);
typedef GLvoid (GLAPIENTRY *__T_EndQuery)(__GLcontext *,  GLenum);
typedef GLvoid (GLAPIENTRY *__T_GetQueryiv)(__GLcontext *,  GLenum, GLenum, GLint *);
typedef GLvoid (GLAPIENTRY *__T_GetQueryObjectiv)(__GLcontext *,  GLuint, GLenum, GLint *);
typedef GLvoid (GLAPIENTRY *__T_GetQueryObjectuiv)(__GLcontext *,  GLuint, GLenum, GLuint *);
typedef GLvoid (GLAPIENTRY *__T_BindBuffer)(__GLcontext *,  GLenum, GLuint);
typedef GLvoid (GLAPIENTRY *__T_DeleteBuffers)(__GLcontext *,  GLsizei, const GLuint *);
typedef GLvoid (GLAPIENTRY *__T_GenBuffers)(__GLcontext *,  GLsizei, GLuint *);
typedef GLboolean (GLAPIENTRY *__T_IsBuffer)(__GLcontext *,  GLuint);
typedef GLvoid (GLAPIENTRY *__T_BufferData)(__GLcontext *,  GLenum, GLsizeiptr, const GLvoid *, GLenum);
typedef GLvoid (GLAPIENTRY *__T_BufferSubData)(__GLcontext *,  GLenum, GLintptr, GLsizeiptr, const GLvoid *);
typedef GLvoid (GLAPIENTRY *__T_GetBufferSubData)(__GLcontext *,  GLenum, GLintptr, GLsizeiptr, const GLvoid *);
typedef GLvoid* (GLAPIENTRY *__T_MapBuffer)(__GLcontext *,  GLenum, GLenum);
typedef GLboolean (GLAPIENTRY *__T_UnmapBuffer)(__GLcontext *,  GLenum);
typedef GLvoid (GLAPIENTRY *__T_GetBufferParameteriv)(__GLcontext *,  GLenum, GLenum, GLint *);
typedef GLvoid (GLAPIENTRY *__T_GetBufferPointerv)(__GLcontext *,  GLenum, GLenum, GLvoid* *);
#endif
#if GL_VERSION_2_0
typedef GLvoid (GLAPIENTRY *__T_BlendEquationSeparate)(__GLcontext *,  GLenum, GLenum);
typedef GLvoid (GLAPIENTRY *__T_DrawBuffers)(__GLcontext *,  GLsizei, const GLenum *);
typedef GLvoid (GLAPIENTRY *__T_StencilOpSeparate)(__GLcontext *,  GLenum, GLenum, GLenum, GLenum);
typedef GLvoid (GLAPIENTRY *__T_StencilFuncSeparate)(__GLcontext *,  GLenum, GLenum, GLint, GLuint);
typedef GLvoid (GLAPIENTRY *__T_StencilMaskSeparate)(__GLcontext *,  GLenum, GLuint);
typedef GLvoid (GLAPIENTRY *__T_AttachShader)(__GLcontext *,  GLuint, GLuint);
typedef GLvoid (GLAPIENTRY *__T_BindAttribLocation)(__GLcontext *,  GLuint, GLuint, const GLchar *);
typedef GLvoid (GLAPIENTRY *__T_CompileShader)(__GLcontext *,  GLuint);
typedef GLuint (GLAPIENTRY *__T_CreateProgram )(__GLcontext *);
typedef GLuint (GLAPIENTRY *__T_CreateShader)(__GLcontext *,  GLenum);
typedef GLvoid (GLAPIENTRY *__T_DeleteProgram)(__GLcontext *,  GLuint);
typedef GLvoid (GLAPIENTRY *__T_DeleteShader)(__GLcontext *,  GLuint);
typedef GLvoid (GLAPIENTRY *__T_DetachShader)(__GLcontext *,  GLuint, GLuint);
typedef GLvoid (GLAPIENTRY *__T_DisableVertexAttribArray)(__GLcontext *,  GLuint);
typedef GLvoid (GLAPIENTRY *__T_EnableVertexAttribArray)(__GLcontext *,  GLuint);
typedef GLvoid (GLAPIENTRY *__T_GetActiveAttrib)(__GLcontext *,  GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *);
typedef GLvoid (GLAPIENTRY *__T_GetActiveUniform)(__GLcontext *,  GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *);
typedef GLvoid (GLAPIENTRY *__T_GetAttachedShaders)(__GLcontext *,  GLuint, GLsizei, GLsizei *, GLuint *);
typedef GLint (GLAPIENTRY *__T_GetAttribLocation)(__GLcontext *,  GLuint, const GLchar *);
typedef GLvoid (GLAPIENTRY *__T_GetProgramiv)(__GLcontext *,  GLuint, GLenum, GLint *);
typedef GLvoid (GLAPIENTRY *__T_GetProgramInfoLog)(__GLcontext *,  GLuint, GLsizei, GLsizei *, GLchar *);
typedef GLvoid (GLAPIENTRY *__T_GetShaderiv)(__GLcontext *,  GLuint, GLenum, GLint *);
typedef GLvoid (GLAPIENTRY *__T_GetShaderInfoLog)(__GLcontext *,  GLuint, GLsizei, GLsizei *, GLchar *);
typedef GLvoid (GLAPIENTRY *__T_GetShaderSource)(__GLcontext *,  GLuint, GLsizei, GLsizei *, GLchar *);
typedef GLint (GLAPIENTRY *__T_GetUniformLocation)(__GLcontext *,  GLuint, const GLchar *);
typedef GLvoid (GLAPIENTRY *__T_GetUniformfv)(__GLcontext *,  GLuint, GLint, GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_GetUniformiv)(__GLcontext *,  GLuint, GLint, GLint *);
typedef GLvoid (GLAPIENTRY *__T_GetVertexAttribdv)(__GLcontext *,  GLuint, GLenum, GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_GetVertexAttribfv)(__GLcontext *,  GLuint, GLenum, GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_GetVertexAttribiv)(__GLcontext *,  GLuint, GLenum, GLint *);
typedef GLvoid (GLAPIENTRY *__T_GetVertexAttribPointerv)(__GLcontext *,  GLuint, GLenum, GLvoid* *);
typedef GLboolean (GLAPIENTRY *__T_IsProgram)(__GLcontext *,  GLuint);
typedef GLboolean (GLAPIENTRY *__T_IsShader)(__GLcontext *,  GLuint);
typedef GLvoid (GLAPIENTRY *__T_LinkProgram)(__GLcontext *,  GLuint);
typedef GLvoid (GLAPIENTRY *__T_UseProgram)(__GLcontext *,  GLuint);
typedef GLvoid (GLAPIENTRY *__T_Uniform1f)(__GLcontext *,  GLint, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_Uniform2f)(__GLcontext *,  GLint, GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_Uniform3f)(__GLcontext *,  GLint, GLfloat, GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_Uniform4f)(__GLcontext *,  GLint, GLfloat, GLfloat, GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_Uniform1i)(__GLcontext *,  GLint, GLint);
typedef GLvoid (GLAPIENTRY *__T_Uniform2i)(__GLcontext *,  GLint, GLint, GLint);
typedef GLvoid (GLAPIENTRY *__T_Uniform3i)(__GLcontext *,  GLint, GLint, GLint, GLint);
typedef GLvoid (GLAPIENTRY *__T_Uniform4i)(__GLcontext *,  GLint, GLint, GLint, GLint, GLint);
typedef GLvoid (GLAPIENTRY *__T_Uniform1fv)(__GLcontext *,  GLint, GLsizei, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_Uniform2fv)(__GLcontext *,  GLint, GLsizei, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_Uniform3fv)(__GLcontext *,  GLint, GLsizei, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_Uniform4fv)(__GLcontext *,  GLint, GLsizei, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_Uniform1iv)(__GLcontext *,  GLint, GLsizei, const GLint *);
typedef GLvoid (GLAPIENTRY *__T_Uniform2iv)(__GLcontext *,  GLint, GLsizei, const GLint *);
typedef GLvoid (GLAPIENTRY *__T_Uniform3iv)(__GLcontext *,  GLint, GLsizei, const GLint *);
typedef GLvoid (GLAPIENTRY *__T_Uniform4iv)(__GLcontext *,  GLint, GLsizei, const GLint *);
typedef GLvoid (GLAPIENTRY *__T_UniformMatrix2fv)(__GLcontext *,  GLint, GLsizei, GLboolean, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_UniformMatrix3fv)(__GLcontext *,  GLint, GLsizei, GLboolean, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_UniformMatrix4fv)(__GLcontext *,  GLint, GLsizei, GLboolean, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_ValidateProgram)(__GLcontext *,  GLuint);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib1d)(__GLcontext *,  GLuint, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib1dv)(__GLcontext *,  GLuint, const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib1f)(__GLcontext *,  GLuint, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib1fv)(__GLcontext *,  GLuint, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib1s)(__GLcontext *,  GLuint, GLshort);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib1sv)(__GLcontext *,  GLuint, const GLshort *);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib2d)(__GLcontext *,  GLuint, GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib2dv)(__GLcontext *,  GLuint, const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib2f)(__GLcontext *,  GLuint, GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib2fv)(__GLcontext *,  GLuint, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib2s)(__GLcontext *,  GLuint, GLshort, GLshort);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib2sv)(__GLcontext *,  GLuint, const GLshort *);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib3d)(__GLcontext *,  GLuint, GLdouble, GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib3dv)(__GLcontext *,  GLuint, const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib3f)(__GLcontext *,  GLuint, GLfloat, GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib3fv)(__GLcontext *,  GLuint, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib3s)(__GLcontext *,  GLuint, GLshort, GLshort, GLshort);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib3sv)(__GLcontext *,  GLuint, const GLshort *);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib4Nbv)(__GLcontext *,  GLuint, const GLbyte *);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib4Niv)(__GLcontext *,  GLuint, const GLint *);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib4Nsv)(__GLcontext *,  GLuint, const GLshort *);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib4Nub)(__GLcontext *,  GLuint, GLubyte, GLubyte, GLubyte, GLubyte);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib4Nubv)(__GLcontext *,  GLuint, const GLubyte *);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib4Nuiv)(__GLcontext *,  GLuint, const GLuint *);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib4Nusv)(__GLcontext *,  GLuint, const GLushort *);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib4bv)(__GLcontext *,  GLuint, const GLbyte *);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib4d)(__GLcontext *,  GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib4dv)(__GLcontext *,  GLuint, const GLdouble *);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib4f)(__GLcontext *,  GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib4fv)(__GLcontext *,  GLuint, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib4iv)(__GLcontext *,  GLuint, const GLint *);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib4s)(__GLcontext *,  GLuint, GLshort, GLshort, GLshort, GLshort);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib4sv)(__GLcontext *,  GLuint, const GLshort *);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib4ubv)(__GLcontext *,  GLuint, const GLubyte *);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib4uiv)(__GLcontext *,  GLuint, const GLuint *);
typedef GLvoid (GLAPIENTRY *__T_VertexAttrib4usv)(__GLcontext *,  GLuint, const GLushort *);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribPointer)(__GLcontext *,  GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *);
#endif

#if GL_VERSION_2_1
typedef GLvoid (GLAPIENTRY *__T_UniformMatrix2x3fv)(__GLcontext *,  GLint, GLsizei, GLboolean, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_UniformMatrix2x4fv)(__GLcontext *,  GLint, GLsizei, GLboolean, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_UniformMatrix3x2fv)(__GLcontext *,  GLint, GLsizei, GLboolean, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_UniformMatrix3x4fv)(__GLcontext *,  GLint, GLsizei, GLboolean, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_UniformMatrix4x2fv)(__GLcontext *,  GLint, GLsizei, GLboolean, const GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_UniformMatrix4x3fv)(__GLcontext *,  GLint, GLsizei, GLboolean, const GLfloat *);
#endif

#if GL_ARB_vertex_program
typedef GLvoid (GLAPIENTRY *__T_ProgramStringARB)(__GLcontext *,  GLenum target, GLenum format, GLsizei len, const GLvoid *string);
typedef GLvoid (GLAPIENTRY *__T_BindProgramARB)(__GLcontext *,  GLenum target, GLuint program);
typedef GLvoid (GLAPIENTRY *__T_DeleteProgramsARB)(__GLcontext *,  GLsizei n, const GLuint *programs);
typedef GLvoid (GLAPIENTRY *__T_GenProgramsARB)(__GLcontext *,  GLsizei n, GLuint *programs);
typedef GLvoid (GLAPIENTRY *__T_ProgramEnvParameter4dARB)(__GLcontext *,  GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef GLvoid (GLAPIENTRY *__T_ProgramEnvParameter4dvARB)(__GLcontext *,  GLenum target, GLuint index, const GLdouble *params);
typedef GLvoid (GLAPIENTRY *__T_ProgramEnvParameter4fARB)(__GLcontext *,  GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef GLvoid (GLAPIENTRY *__T_ProgramEnvParameter4fvARB)(__GLcontext *,  GLenum target, GLuint index, const GLfloat *params);
typedef GLvoid (GLAPIENTRY *__T_ProgramLocalParameter4dARB)(__GLcontext *,  GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef GLvoid (GLAPIENTRY *__T_ProgramLocalParameter4dvARB)(__GLcontext *,  GLenum target, GLuint index, const GLdouble *params);
typedef GLvoid (GLAPIENTRY *__T_ProgramLocalParameter4fARB)(__GLcontext *,  GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef GLvoid (GLAPIENTRY *__T_ProgramLocalParameter4fvARB)(__GLcontext *,  GLenum target, GLuint index, const GLfloat *params);
typedef GLvoid (GLAPIENTRY *__T_GetProgramEnvParameterdvARB)(__GLcontext *,  GLenum target, GLuint index, GLdouble *params);
typedef GLvoid (GLAPIENTRY *__T_GetProgramEnvParameterfvARB)(__GLcontext *,  GLenum target, GLuint index, GLfloat *params);
typedef GLvoid (GLAPIENTRY *__T_GetProgramLocalParameterdvARB)(__GLcontext *,  GLenum target, GLuint index, GLdouble *params);
typedef GLvoid (GLAPIENTRY *__T_GetProgramLocalParameterfvARB)(__GLcontext *,  GLenum target, GLuint index, GLfloat *params);
typedef GLvoid (GLAPIENTRY *__T_GetProgramivARB)(__GLcontext *,  GLenum target, GLenum pname, GLint *params);
typedef GLvoid (GLAPIENTRY *__T_GetProgramStringARB)(__GLcontext *,  GLenum target, GLenum pname, GLvoid *string);
typedef GLboolean (GLAPIENTRY *__T_IsProgramARB)(__GLcontext *,  GLuint program);
#endif

#if GL_ARB_shader_objects
typedef GLvoid (GLAPIENTRY *__T_DeleteObjectARB)(__GLcontext *, GLhandleARB obj);
typedef GLhandleARB (GLAPIENTRY *__T_GetHandleARB)(__GLcontext *, GLenum pname);
typedef GLvoid (GLAPIENTRY *__T_GetInfoLogARB)(__GLcontext *, GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
typedef GLvoid (GLAPIENTRY *__T_GetObjectParameterfvARB)(__GLcontext *, GLhandleARB obj, GLenum pname, GLfloat *params);
typedef GLvoid (GLAPIENTRY *__T_GetObjectParameterivARB)(__GLcontext *, GLhandleARB obj, GLenum pname, GLint *params);
#endif

#if GL_ATI_vertex_array_object
typedef GLuint (GLAPIENTRY *__T_NewObjectBufferATI)(__GLcontext *, GLsizei size, const GLvoid *pointer, GLenum usage);
typedef GLboolean (GLAPIENTRY *__T_IsObjectBufferATI)(__GLcontext *, GLuint buffer);
typedef GLvoid (GLAPIENTRY *__T_UpdateObjectBufferATI)(__GLcontext *, GLuint buffer, GLuint offset, GLsizei size, const GLvoid *pointer, GLenum preserve);
typedef GLvoid (GLAPIENTRY *__T_GetObjectBufferfvATI)(__GLcontext *, GLuint buffer, GLenum pname, GLfloat *params);
typedef GLvoid (GLAPIENTRY *__T_GetObjectBufferivATI)(__GLcontext *, GLuint buffer, GLenum pname, GLint *params);
typedef GLvoid (GLAPIENTRY *__T_FreeObjectBufferATI)(__GLcontext *, GLuint buffer);
typedef GLvoid (GLAPIENTRY *__T_ArrayObjectATI)(__GLcontext *, GLenum array, GLint size, GLenum type, GLsizei stride, GLuint buffer, GLuint offset);
typedef GLvoid (GLAPIENTRY *__T_GetArrayObjectfvATI)(__GLcontext *, GLenum array, GLenum pname, GLfloat * params);
typedef GLvoid (GLAPIENTRY *__T_GetArrayObjectivATI)(__GLcontext *, GLenum array, GLenum pname, GLint * params);
typedef GLvoid (GLAPIENTRY *__T_VariantArrayObjectATI)(__GLcontext *, GLuint, GLenum, GLsizei, GLuint, GLuint);
typedef GLvoid (GLAPIENTRY *__T_GetVariantArrayObjectfvATI)(__GLcontext *, GLuint, GLenum, GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_GetVariantArrayObjectivATI)(__GLcontext *, GLuint, GLenum, GLint *);
#endif

#if GL_ATI_vertex_attrib_array_object
typedef GLvoid (GLAPIENTRY *__T_VertexAttribArrayObjectATI)(__GLcontext *, GLuint, GLint, GLenum, GLboolean, GLsizei, GLuint, GLuint);
typedef GLvoid (GLAPIENTRY *__T_GetVertexAttribArrayObjectfvATI)(__GLcontext *, GLuint, GLenum, GLfloat *);
typedef GLvoid (GLAPIENTRY *__T_GetVertexAttribArrayObjectivATI)(__GLcontext *, GLuint, GLenum, GLint *);
#endif

typedef GLvoid (GLAPIENTRY *__T_AddSwapHintRectWIN)(__GLcontext *, GLint x, GLint y, GLsizei width,
                                        GLsizei height);

#if GL_EXT_framebuffer_object
typedef GLboolean (GLAPIENTRY *__T_IsRenderbufferEXT)(__GLcontext *, GLuint);
typedef GLvoid (GLAPIENTRY *__T_BindRenderbufferEXT)(__GLcontext *, GLenum, GLuint);
typedef GLvoid (GLAPIENTRY *__T_DeleteRenderbuffersEXT)(__GLcontext *, GLsizei, const GLuint *);
typedef GLvoid (GLAPIENTRY *__T_GenRenderbuffersEXT)(__GLcontext *, GLsizei, GLuint *);
typedef GLvoid (GLAPIENTRY *__T_RenderbufferStorageEXT)(__GLcontext *, GLenum, GLenum, GLsizei, GLsizei);
typedef GLvoid (GLAPIENTRY *__T_GetRenderbufferParameterivEXT)(__GLcontext *, GLenum, GLenum, GLint *);
typedef GLboolean (GLAPIENTRY *__T_IsFramebufferEXT)(__GLcontext *, GLuint);
typedef GLvoid (GLAPIENTRY *__T_BindFramebufferEXT)(__GLcontext *, GLenum, GLuint);
typedef GLvoid (GLAPIENTRY *__T_DeleteFramebuffersEXT)(__GLcontext *, GLsizei, const GLuint *);
typedef GLvoid (GLAPIENTRY *__T_GenFramebuffersEXT)(__GLcontext *, GLsizei, GLuint *);
typedef GLenum (GLAPIENTRY *__T_CheckFramebufferStatusEXT)(__GLcontext *, GLenum);
typedef GLvoid (GLAPIENTRY *__T_FramebufferTexture1DEXT)(__GLcontext *, GLenum, GLenum, GLenum, GLuint, GLint);
typedef GLvoid (GLAPIENTRY *__T_FramebufferTexture2DEXT)(__GLcontext *, GLenum, GLenum, GLenum, GLuint, GLint);
typedef GLvoid (GLAPIENTRY *__T_FramebufferTexture3DEXT)(__GLcontext *, GLenum, GLenum, GLenum, GLuint, GLint, GLint);
typedef GLvoid (GLAPIENTRY *__T_FramebufferRenderbufferEXT)(__GLcontext *, GLenum, GLenum, GLenum, GLuint);
typedef GLvoid (GLAPIENTRY *__T_GetFramebufferAttachmentParameterivEXT)(__GLcontext *, GLenum, GLenum, GLenum, GLint *);
typedef GLvoid (GLAPIENTRY *__T_GenerateMipmapEXT)(__GLcontext *, GLenum);
#if GL_EXT_framebuffer_blit
typedef GLvoid (GLAPIENTRY *__T_BlitFramebufferEXT)(__GLcontext *, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum);
#if GL_EXT_framebuffer_multisample
typedef GLvoid (GLAPIENTRY *__T_RenderbufferStorageMultisampleEXT)(__GLcontext *, GLenum, GLsizei, GLenum, GLsizei, GLsizei);
#endif
#endif
#endif

#if GL_ATI_element_array
typedef GLvoid (GLAPIENTRY *__T_ElementPointerATI)(__GLcontext *, GLenum type, const GLvoid *pointer);
typedef GLvoid (GLAPIENTRY *__T_DrawElementArrayATI)(__GLcontext *, GLenum mode, GLsizei count);
typedef GLvoid (GLAPIENTRY *__T_DrawRangeElementArrayATI)(__GLcontext *, GLenum mode, GLuint start, GLuint end, GLsizei count);
#endif

#if GL_EXT_stencil_two_side
typedef GLvoid (GLAPIENTRY *__T_ActiveStencilFaceEXT)(__GLcontext *, GLenum face);
#endif

#if GL_EXT_depth_bounds_test
typedef GLvoid (GLAPIENTRY *__T_DepthBoundsEXT)(__GLcontext *, GLclampd zMin, GLclampd zMax);
#endif

#if GL_NV_occlusion_query
typedef GLvoid (GLAPIENTRY *__T_BeginQueryNV)(__GLcontext *, GLuint);
typedef GLvoid (GLAPIENTRY *__T_EndQueryNV)(__GLcontext *);
#endif


#if GL_EXT_bindable_uniform
typedef GLvoid (GLAPIENTRY *__T_UniformBufferEXT)(__GLcontext *, GLuint program, GLint location, GLuint buffer);
typedef GLint (GLAPIENTRY *__T_GetUniformBufferSizeEXT)(__GLcontext *, GLuint program, GLint location);
typedef GLintptr (GLAPIENTRY *__T_GetUniformOffsetEXT)(__GLcontext *, GLuint program, GLint location);
#endif

#if GL_EXT_texture_integer
typedef GLvoid (GLAPIENTRY *__T_ClearColorIiEXT)(__GLcontext *, GLint r, GLint g, GLint b,GLint a);
typedef GLvoid (GLAPIENTRY *__T_ClearColorIuiEXT)(__GLcontext *, GLuint r, GLuint g, GLuint b, GLuint a);
typedef GLvoid (GLAPIENTRY *__T_TexParameterIivEXT)(__GLcontext *, GLenum target, GLenum pname, GLint *params);
typedef GLvoid (GLAPIENTRY *__T_TexParameterIuivEXT)(__GLcontext *, GLenum target, GLenum pname, GLuint *params);
typedef GLvoid (GLAPIENTRY *__T_GetTexParameterIivEXT)(__GLcontext *, GLenum target, GLenum pname, GLint *params);
typedef GLvoid (GLAPIENTRY *__T_GetTexParameterIuivEXT)(__GLcontext *, GLenum target, GLenum pname, GLuint *params);
#endif

#if GL_EXT_gpu_shader4
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI1iEXT)(__GLcontext *, GLuint index, GLint x);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI2iEXT)(__GLcontext *, GLuint index, GLint x, GLint y);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI3iEXT)(__GLcontext *, GLuint index, GLint x, GLint y, GLint z);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI4iEXT)(__GLcontext *, GLuint index, GLint x, GLint y, GLint z, GLint w);

typedef GLvoid (GLAPIENTRY *__T_VertexAttribI1uiEXT)(__GLcontext *, GLuint index, GLuint x);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI2uiEXT)(__GLcontext *, GLuint index, GLuint x, GLuint y);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI3uiEXT)(__GLcontext *, GLuint index, GLuint x, GLuint y, GLuint z);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI4uiEXT)(__GLcontext *, GLuint index, GLuint x, GLuint y, GLuint z,
                            GLuint w);

typedef GLvoid (GLAPIENTRY *__T_VertexAttribI1ivEXT)(__GLcontext *, GLuint index, const GLint *v);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI2ivEXT)(__GLcontext *, GLuint index, const GLint *v);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI3ivEXT)(__GLcontext *, GLuint index, const GLint *v);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI4ivEXT)(__GLcontext *, GLuint index, const GLint *v);

typedef GLvoid (GLAPIENTRY *__T_VertexAttribI1uivEXT)(__GLcontext *, GLuint index, const GLuint *v);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI2uivEXT)(__GLcontext *, GLuint index, const GLuint *v);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI3uivEXT)(__GLcontext *, GLuint index, const GLuint *v);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI4uivEXT)(__GLcontext *, GLuint index, const GLuint *v);

typedef GLvoid (GLAPIENTRY *__T_VertexAttribI4bvEXT)(__GLcontext *, GLuint index, const GLbyte *v);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI4svEXT)(__GLcontext *, GLuint index, const GLshort *v);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI4ubvEXT)(__GLcontext *, GLuint index, const GLubyte *v);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI4usvEXT)(__GLcontext *, GLuint index, const GLushort *v);

typedef GLvoid (GLAPIENTRY *__T_VertexAttribIPointerEXT)(__GLcontext *, GLuint index, GLint size, GLenum type,
                                GLsizei stride, const GLvoid *pointer);

typedef GLvoid (GLAPIENTRY *__T_GetVertexAttribIivEXT)(__GLcontext *, GLuint index, GLenum pname, GLint *params);
typedef GLvoid (GLAPIENTRY *__T_GetVertexAttribIuivEXT)(__GLcontext *, GLuint index, GLenum pname, GLuint *params);

typedef GLvoid (GLAPIENTRY *__T_Uniform1uiEXT)(__GLcontext *, GLint location, GLuint v0);
typedef GLvoid (GLAPIENTRY *__T_Uniform2uiEXT)(__GLcontext *, GLint location, GLuint v0, GLuint v1);
typedef GLvoid (GLAPIENTRY *__T_Uniform3uiEXT)(__GLcontext *, GLint location, GLuint v0, GLuint v1, GLuint v2);
typedef GLvoid (GLAPIENTRY *__T_Uniform4uiEXT)(__GLcontext *, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);

typedef GLvoid (GLAPIENTRY *__T_Uniform1uivEXT)(__GLcontext *, GLint location, GLsizei count, const GLuint *value);
typedef GLvoid (GLAPIENTRY *__T_Uniform2uivEXT)(__GLcontext *, GLint location, GLsizei count, const GLuint *value);
typedef GLvoid (GLAPIENTRY *__T_Uniform3uivEXT)(__GLcontext *, GLint location, GLsizei count, const GLuint *value);
typedef GLvoid (GLAPIENTRY *__T_Uniform4uivEXT)(__GLcontext *, GLint location, GLsizei count, const GLuint *value);

typedef GLvoid (GLAPIENTRY *__T_GetUniformuivEXT)(__GLcontext *, GLuint program, GLint location, GLuint *params);

typedef GLvoid (GLAPIENTRY *__T_BindFragDataLocationEXT)(__GLcontext *, GLuint program, GLuint colorNumber,
                                const GLbyte *name);
typedef GLint (GLAPIENTRY *__T_GetFragDataLocationEXT)(__GLcontext *, GLuint program, const GLbyte *name);
#endif

#if GL_EXT_geometry_shader4
typedef GLvoid (GLAPIENTRY *__T_ProgramParameteriEXT)(__GLcontext *, GLuint program, GLenum pname, GLint value);
typedef GLvoid (GLAPIENTRY *__T_FramebufferTextureEXT)(__GLcontext *, GLenum target, GLenum attachment, GLuint texture, GLint level);
typedef GLvoid (GLAPIENTRY *__T_FramebufferTextureLayerEXT)(__GLcontext *, GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
typedef GLvoid (GLAPIENTRY *__T_FramebufferTextureFaceEXT)(__GLcontext *, GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face);
#endif

#if GL_EXT_draw_buffers2
typedef GLvoid (GLAPIENTRY *__T_ColorMaskIndexedEXT)(__GLcontext *, GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
typedef GLvoid (GLAPIENTRY *__T_GetBooleanIndexedvEXT)(__GLcontext *, GLenum value, GLuint index, GLboolean *data);
typedef GLvoid (GLAPIENTRY *__T_GetIntegerIndexedvEXT)(__GLcontext *, GLenum value, GLuint index, GLint *data);
typedef GLvoid (GLAPIENTRY *__T_EnableIndexedEXT)(__GLcontext *, GLenum target, GLuint index);
typedef GLvoid (GLAPIENTRY *__T_DisableIndexedEXT)(__GLcontext *, GLenum target, GLuint index);
typedef GLboolean (GLAPIENTRY *__T_IsEnabledIndexedEXT)(__GLcontext *, GLenum target, GLuint index);
#endif

#if GL_EXT_gpu_program_parameters
typedef GLvoid (GLAPIENTRY *__T_ProgramEnvParameters4fvEXT)(__GLcontext *, GLenum target, GLuint index, GLsizei count, const GLfloat *params);
typedef GLvoid (GLAPIENTRY *__T_ProgramLocalParameters4fvEXT)(__GLcontext *, GLenum target, GLuint index, GLsizei count, const GLfloat *params);
#endif

#if GL_ARB_color_buffer_float
typedef GLvoid (GLAPIENTRY *__T_ClampColorARB)(__GLcontext *, GLenum target, GLenum clamp);
#endif

#if GL_ATI_separate_stencil
typedef GLvoid (GLAPIENTRY *__T_StencilFuncSeparateATI)(__GLcontext *, GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask);
#endif

#if GL_EXT_texture_buffer_object
typedef GLvoid (GLAPIENTRY *__T_TexBufferEXT)(__GLcontext *, GLenum target, GLenum internalformat, GLuint buffer);
#endif

#if GL_EXT_draw_instanced
typedef GLvoid (GLAPIENTRY *__T_DrawArraysInstancedEXT)(__GLcontext *, GLenum mode,
                                                GLint first, GLsizei count, GLsizei primCount);
typedef GLvoid (GLAPIENTRY *__T_DrawElementsInstancedEXT)(__GLcontext *, GLenum mode,
                        GLsizei count, GLenum type, const GLvoid *indices, GLsizei primCount);
#endif

#if GL_EXT_timer_query
typedef GLvoid (GLAPIENTRY *__T_GetQueryObjecti64vEXT)(__GLcontext *, GLuint, GLenum, GLint64EXT *);
typedef GLvoid (GLAPIENTRY *__T_GetQueryObjectui64vEXT)(__GLcontext *, GLuint, GLenum, GLuint64EXT *);
#endif


typedef  GLvoid  (GLAPIENTRY *__T_ActiveTexture)(__GLcontext *gc, GLenum texture);
typedef  GLvoid  (GLAPIENTRY *__T_AttachShader)(__GLcontext *gc, GLuint program, GLuint shader);
typedef  GLvoid  (GLAPIENTRY *__T_BindAttribLocation)(__GLcontext *gc, GLuint program, GLuint index, const GLchar* name);
typedef  GLvoid  (GLAPIENTRY *__T_BindBuffer)(__GLcontext *gc, GLenum target, GLuint buffer);
typedef  GLvoid  (GLAPIENTRY *__T_BindFramebuffer)(__GLcontext *gc, GLenum target, GLuint framebuffer);
typedef  GLvoid  (GLAPIENTRY *__T_BindRenderbuffer)(__GLcontext *gc, GLenum target, GLuint renderbuffer);
typedef  GLvoid  (GLAPIENTRY *__T_BindTexture)(__GLcontext *gc, GLenum target, GLuint texture);
typedef  GLvoid  (GLAPIENTRY *__T_BlendColor)(__GLcontext *gc, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef  GLvoid  (GLAPIENTRY *__T_BlendEquation)(__GLcontext *gc, GLenum mode);
typedef  GLvoid  (GLAPIENTRY *__T_BlendEquationSeparate)(__GLcontext *gc, GLenum modeRGB, GLenum modeAlpha);
typedef  GLvoid  (GLAPIENTRY *__T_BlendFunc)(__GLcontext *gc, GLenum sfactor, GLenum dfactor);
typedef  GLvoid  (GLAPIENTRY *__T_BlendFuncSeparate)(__GLcontext *gc, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
typedef  GLvoid  (GLAPIENTRY *__T_BufferData)(__GLcontext *gc, GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
typedef  GLvoid  (GLAPIENTRY *__T_BufferSubData)(__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
typedef  GLenum  (GLAPIENTRY *__T_CheckFramebufferStatus)(__GLcontext *gc, GLenum target);
typedef  GLvoid  (GLAPIENTRY *__T_Clear)(__GLcontext *gc, GLbitfield mask);
typedef  GLvoid  (GLAPIENTRY *__T_ClearColor)(__GLcontext *gc, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef  GLvoid  (GLAPIENTRY *__T_ClearDepthf)(__GLcontext *gc, GLfloat depth);
typedef  GLvoid  (GLAPIENTRY *__T_ClearStencil)(__GLcontext *gc, GLint s);
typedef  GLvoid  (GLAPIENTRY *__T_ColorMask)(__GLcontext *gc, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
typedef  GLvoid  (GLAPIENTRY *__T_CompileShader)(__GLcontext *gc, GLuint shader);
typedef  GLvoid  (GLAPIENTRY *__T_CompressedTexImage2D)(__GLcontext *gc, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data);
typedef  GLvoid  (GLAPIENTRY *__T_CompressedTexSubImage2D)(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data);
typedef  GLvoid  (GLAPIENTRY *__T_CopyTexImage2D)(__GLcontext *gc, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
typedef  GLvoid  (GLAPIENTRY *__T_CopyTexSubImage2D)(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef  GLuint  (GLAPIENTRY *__T_CreateProgram)(__GLcontext *gc);
typedef  GLuint  (GLAPIENTRY *__T_CreateShader)(__GLcontext *gc, GLenum type);
typedef  GLvoid  (GLAPIENTRY *__T_CullFace)(__GLcontext *gc, GLenum mode);
typedef  GLvoid  (GLAPIENTRY *__T_DeleteBuffers)(__GLcontext *gc, GLsizei n, const GLuint* buffers);
typedef  GLvoid  (GLAPIENTRY *__T_DeleteFramebuffers)(__GLcontext *gc, GLsizei n, const GLuint* framebuffers);
typedef  GLvoid  (GLAPIENTRY *__T_DeleteProgram)(__GLcontext *gc, GLuint program);
typedef  GLvoid  (GLAPIENTRY *__T_DeleteRenderbuffers)(__GLcontext *gc, GLsizei n, const GLuint* renderbuffers);
typedef  GLvoid  (GLAPIENTRY *__T_DeleteShader)(__GLcontext *gc, GLuint shader);
typedef  GLvoid  (GLAPIENTRY *__T_DeleteTextures)(__GLcontext *gc, GLsizei n, const GLuint* textures);
typedef  GLvoid  (GLAPIENTRY *__T_DepthFunc)(__GLcontext *gc, GLenum func);
typedef  GLvoid  (GLAPIENTRY *__T_DepthMask)(__GLcontext *gc, GLboolean flag);
typedef  GLvoid  (GLAPIENTRY *__T_DepthRangef)(__GLcontext *gc, GLfloat n, GLfloat f);
typedef  GLvoid  (GLAPIENTRY *__T_DetachShader)(__GLcontext *gc, GLuint program, GLuint shader);
typedef  GLvoid  (GLAPIENTRY *__T_Disable)(__GLcontext *gc, GLenum cap);
typedef  GLvoid  (GLAPIENTRY *__T_DisableVertexAttribArray)(__GLcontext *gc, GLuint index);
typedef  GLvoid  (GLAPIENTRY *__T_DrawArrays)(__GLcontext *gc, GLenum mode, GLint first, GLsizei count);
typedef  GLvoid  (GLAPIENTRY *__T_DrawElements)(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);
typedef  GLvoid  (GLAPIENTRY *__T_Enable)(__GLcontext *gc, GLenum cap);
typedef  GLvoid  (GLAPIENTRY *__T_EnableVertexAttribArray)(__GLcontext *gc, GLuint index);
typedef  GLvoid  (GLAPIENTRY *__T_Finish)(__GLcontext *gc);
typedef  GLvoid  (GLAPIENTRY *__T_Flush)(__GLcontext *gc);
typedef  GLvoid  (GLAPIENTRY *__T_FramebufferRenderbuffer)(__GLcontext *gc, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef  GLvoid  (GLAPIENTRY *__T_FramebufferTexture2D)(__GLcontext *gc, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef  GLvoid  (GLAPIENTRY *__T_FrontFace)(__GLcontext *gc, GLenum mode);
typedef  GLvoid  (GLAPIENTRY *__T_GenBuffers)(__GLcontext *gc, GLsizei n, GLuint* buffers);
typedef  GLvoid  (GLAPIENTRY *__T_GenerateMipmap)(__GLcontext *gc, GLenum target);
typedef  GLvoid  (GLAPIENTRY *__T_GenFramebuffers)(__GLcontext *gc, GLsizei n, GLuint* framebuffers);
typedef  GLvoid  (GLAPIENTRY *__T_GenRenderbuffers)(__GLcontext *gc, GLsizei n, GLuint* renderbuffers);
typedef  GLvoid  (GLAPIENTRY *__T_GenTextures)(__GLcontext *gc, GLsizei n, GLuint* textures);
typedef  GLvoid  (GLAPIENTRY *__T_GetActiveAttrib)(__GLcontext *gc, GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
typedef  GLvoid  (GLAPIENTRY *__T_GetActiveUniform)(__GLcontext *gc, GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
typedef  GLvoid  (GLAPIENTRY *__T_GetAttachedShaders)(__GLcontext *gc, GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders);
typedef  GLint   (GLAPIENTRY *__T_GetAttribLocation)(__GLcontext *gc, GLuint program, const GLchar* name);
typedef  GLvoid  (GLAPIENTRY *__T_GetBooleanv)(__GLcontext *gc, GLenum pname, GLboolean* params);
typedef  GLvoid  (GLAPIENTRY *__T_GetBufferParameteriv)(__GLcontext *gc, GLenum target, GLenum pname, GLint* params);
typedef  GLenum  (GLAPIENTRY *__T_GetError)(__GLcontext *gc);
typedef  GLvoid  (GLAPIENTRY *__T_GetFloatv)(__GLcontext *gc, GLenum pname, GLfloat* params);
typedef  GLvoid  (GLAPIENTRY *__T_GetFramebufferAttachmentParameteriv)(__GLcontext *gc, GLenum target, GLenum attachment, GLenum pname, GLint* params);
typedef  GLvoid  (GLAPIENTRY *__T_GetIntegerv)(__GLcontext *gc, GLenum pname, GLint* params);
typedef  GLvoid  (GLAPIENTRY *__T_GetProgramiv)(__GLcontext *gc, GLuint program, GLenum pname, GLint* params);
typedef  GLvoid  (GLAPIENTRY *__T_GetProgramInfoLog)(__GLcontext *gc, GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog);
typedef  GLvoid  (GLAPIENTRY *__T_GetRenderbufferParameteriv)(__GLcontext *gc, GLenum target, GLenum pname, GLint* params);
typedef  GLvoid  (GLAPIENTRY *__T_GetShaderiv)(__GLcontext *gc, GLuint shader, GLenum pname, GLint* params);
typedef  GLvoid  (GLAPIENTRY *__T_GetShaderInfoLog)(__GLcontext *gc, GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog);
typedef  GLvoid  (GLAPIENTRY *__T_GetShaderPrecisionFormat)(__GLcontext *gc, GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision);
typedef  GLvoid  (GLAPIENTRY *__T_GetShaderSource)(__GLcontext *gc, GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source);
typedef  const GLubyte*  (GLAPIENTRY *__T_GetString)(__GLcontext *gc, GLenum name);
typedef  GLvoid  (GLAPIENTRY *__T_GetTexParameterfv)(__GLcontext *gc, GLenum target, GLenum pname, GLfloat* params);
typedef  GLvoid  (GLAPIENTRY *__T_GetTexParameteriv)(__GLcontext *gc, GLenum target, GLenum pname, GLint* params);
typedef  GLvoid  (GLAPIENTRY *__T_GetUniformfv)(__GLcontext *gc, GLuint program, GLint location, GLfloat* params);
typedef  GLvoid  (GLAPIENTRY *__T_GetUniformiv)(__GLcontext *gc, GLuint program, GLint location, GLint* params);
typedef  GLint   (GLAPIENTRY *__T_GetUniformLocation)(__GLcontext *gc, GLuint program, const GLchar* name);
typedef  GLvoid  (GLAPIENTRY *__T_GetVertexAttribfv)(__GLcontext *gc, GLuint index, GLenum pname, GLfloat* params);
typedef  GLvoid  (GLAPIENTRY *__T_GetVertexAttribiv)(__GLcontext *gc, GLuint index, GLenum pname, GLint* params);
typedef  GLvoid  (GLAPIENTRY *__T_GetVertexAttribPointerv)(__GLcontext *gc, GLuint index, GLenum pname, GLvoid** pointer);
typedef  GLvoid  (GLAPIENTRY *__T_Hint)(__GLcontext *gc, GLenum target, GLenum mode);
typedef  GLboolean  (GLAPIENTRY *__T_IsBuffer)(__GLcontext *gc, GLuint buffer);
typedef  GLboolean  (GLAPIENTRY *__T_IsEnabled)(__GLcontext *gc, GLenum cap);
typedef  GLboolean  (GLAPIENTRY *__T_IsFramebuffer)(__GLcontext *gc, GLuint framebuffer);
typedef  GLboolean  (GLAPIENTRY *__T_IsProgram)(__GLcontext *gc, GLuint program);
typedef  GLboolean  (GLAPIENTRY *__T_IsRenderbuffer)(__GLcontext *gc, GLuint renderbuffer);
typedef  GLboolean  (GLAPIENTRY *__T_IsShader)(__GLcontext *gc, GLuint shader);
typedef  GLboolean  (GLAPIENTRY *__T_IsTexture)(__GLcontext *gc, GLuint texture);
typedef  GLvoid  (GLAPIENTRY *__T_LineWidth)(__GLcontext *gc, GLfloat width);
typedef  GLvoid  (GLAPIENTRY *__T_LinkProgram)(__GLcontext *gc, GLuint program);
typedef  GLvoid  (GLAPIENTRY *__T_PixelStorei)(__GLcontext *gc, GLenum pname, GLint param);
typedef  GLvoid  (GLAPIENTRY *__T_PolygonOffset)(__GLcontext *gc, GLfloat factor, GLfloat units);
typedef  GLvoid  (GLAPIENTRY *__T_ReadPixels)(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
typedef  GLvoid  (GLAPIENTRY *__T_ReleaseShaderCompiler)(__GLcontext *gc);
typedef  GLvoid  (GLAPIENTRY *__T_RenderbufferStorage)(__GLcontext *gc, GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef  GLvoid  (GLAPIENTRY *__T_SampleCoverage)(__GLcontext *gc, GLfloat value, GLboolean invert);
typedef  GLvoid  (GLAPIENTRY *__T_Scissor)(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height);
typedef  GLvoid  (GLAPIENTRY *__T_ShaderBinary)(__GLcontext *gc, GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length);
typedef  GLvoid  (GLAPIENTRY *__T_ShaderSource)(__GLcontext *gc, GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
typedef  GLvoid  (GLAPIENTRY *__T_StencilFunc)(__GLcontext *gc, GLenum func, GLint ref, GLuint mask);
typedef  GLvoid  (GLAPIENTRY *__T_StencilFuncSeparate)(__GLcontext *gc, GLenum face, GLenum func, GLint ref, GLuint mask);
typedef  GLvoid  (GLAPIENTRY *__T_StencilMask)(__GLcontext *gc, GLuint mask);
typedef  GLvoid  (GLAPIENTRY *__T_StencilMaskSeparate)(__GLcontext *gc, GLenum face, GLuint mask);
typedef  GLvoid  (GLAPIENTRY *__T_StencilOp)(__GLcontext *gc, GLenum fail, GLenum zfail, GLenum zpass);
typedef  GLvoid  (GLAPIENTRY *__T_StencilOpSeparate)(__GLcontext *gc, GLenum face, GLenum fail, GLenum zfail, GLenum zpass);
typedef  GLvoid  (GLAPIENTRY *__T_TexImage2D)(__GLcontext *gc, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
typedef  GLvoid  (GLAPIENTRY *__T_TexParameterf)(__GLcontext *gc, GLenum target, GLenum pname, GLfloat param);
typedef  GLvoid  (GLAPIENTRY *__T_TexParameterfv)(__GLcontext *gc, GLenum target, GLenum pname, const GLfloat* params);
typedef  GLvoid  (GLAPIENTRY *__T_TexParameteri)(__GLcontext *gc, GLenum target, GLenum pname, GLint param);
typedef  GLvoid  (GLAPIENTRY *__T_TexParameteriv)(__GLcontext *gc, GLenum target, GLenum pname, const GLint* params);
typedef  GLvoid  (GLAPIENTRY *__T_TexSubImage2D)(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform1f)(__GLcontext *gc, GLint location, GLfloat x);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform1fv)(__GLcontext *gc, GLint location, GLsizei count, const GLfloat* v);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform1i)(__GLcontext *gc, GLint location, GLint x);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform1iv)(__GLcontext *gc, GLint location, GLsizei count, const GLint* v);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform2f)(__GLcontext *gc, GLint location, GLfloat x, GLfloat y);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform2fv)(__GLcontext *gc, GLint location, GLsizei count, const GLfloat* v);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform2i)(__GLcontext *gc, GLint location, GLint x, GLint y);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform2iv)(__GLcontext *gc, GLint location, GLsizei count, const GLint* v);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform3f)(__GLcontext *gc, GLint location, GLfloat x, GLfloat y, GLfloat z);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform3fv)(__GLcontext *gc, GLint location, GLsizei count, const GLfloat* v);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform3i)(__GLcontext *gc, GLint location, GLint x, GLint y, GLint z);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform3iv)(__GLcontext *gc, GLint location, GLsizei count, const GLint* v);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform4f)(__GLcontext *gc, GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform4fv)(__GLcontext *gc, GLint location, GLsizei count, const GLfloat* v);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform4i)(__GLcontext *gc, GLint location, GLint x, GLint y, GLint z, GLint w);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform4iv)(__GLcontext *gc, GLint location, GLsizei count, const GLint* v);
typedef  GLvoid  (GLAPIENTRY *__T_UniformMatrix2fv)(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef  GLvoid  (GLAPIENTRY *__T_UniformMatrix3fv)(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef  GLvoid  (GLAPIENTRY *__T_UniformMatrix4fv)(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef  GLvoid  (GLAPIENTRY *__T_UseProgram)(__GLcontext *gc, GLuint program);
typedef  GLvoid  (GLAPIENTRY *__T_ValidateProgram)(__GLcontext *gc, GLuint program);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib1d) (__GLcontext *gc, GLuint indx, GLdouble x);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib1dv) (__GLcontext *gc, GLuint indx, const GLdouble *values);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib1f) (__GLcontext *gc, GLuint indx, GLfloat x);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib1fv) (__GLcontext *gc, GLuint indx, const GLfloat * values);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib1s) (__GLcontext *gc, GLuint indx, GLshort x);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib1sv) (__GLcontext *gc, GLuint indx, const GLshort * values);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib2d) (__GLcontext *gc, GLuint indx, GLdouble x, GLdouble y);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib2dv) (__GLcontext *gc, GLuint indx, const GLdouble * values);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib2f) (__GLcontext *gc, GLuint indx, GLfloat x, GLfloat y);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib2fv) (__GLcontext *gc, GLuint indx, const GLfloat * values);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib2s) (__GLcontext *gc, GLuint indx, GLshort x, GLshort y);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib2sv) (__GLcontext *gc, GLuint indx, const GLshort * values);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib3d) (__GLcontext *gc, GLuint indx, GLdouble x, GLdouble y, GLdouble z);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib3dv) (__GLcontext *gc, GLuint indx, const GLdouble * values);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib3f) (__GLcontext *gc, GLuint indx, GLfloat x, GLfloat y, GLfloat z);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib3fv) (__GLcontext *gc, GLuint indx, const GLfloat * values);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib3s) (__GLcontext *gc, GLuint indx, GLshort x, GLshort y, GLshort z);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib3sv) (__GLcontext *gc, GLuint indx, const GLshort * values);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib4Nbv) (__GLcontext *gc, GLuint indx, const GLbyte * values);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib4Niv) (__GLcontext *gc, GLuint indx, const GLint * values);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib4Nsv) (__GLcontext *gc, GLuint indx, const GLshort * values);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib4Nub) (__GLcontext *gc, GLuint indx, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib4Nubv) (__GLcontext *gc, GLuint indx, const GLubyte * values);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib4Nuiv) (__GLcontext *gc, GLuint indx, const GLuint * values);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib4Nusv) (__GLcontext *gc, GLuint indx, const GLushort * values);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib4bv) (__GLcontext *gc, GLuint indx, const GLbyte * values);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib4d) (__GLcontext *gc, GLuint indx, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib4dv) (__GLcontext *gc, GLuint indx, const GLdouble * values);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib4f) (__GLcontext *gc, GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib4fv) (__GLcontext *gc, GLuint indx, const GLfloat * values);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib4iv) (__GLcontext *gc, GLuint indx, const GLint * values);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib4s) (__GLcontext *gc, GLuint indx, GLshort x, GLshort y, GLshort z, GLshort w);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib4sv) (__GLcontext *gc, GLuint indx, const GLshort * values);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib4ubv) (__GLcontext *gc, GLuint indx, const GLubyte * values);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib4uiv) (__GLcontext *gc, GLuint indx, const GLuint * values);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttrib4usv) (__GLcontext *gc, GLuint indx, const GLushort * values);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttribPointer)(__GLcontext *gc, GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr);
typedef  GLvoid  (GLAPIENTRY *__T_Viewport)(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height);

  /* OpenGL ES 3.0 */

typedef  GLvoid  (GLAPIENTRY *__T_ReadBuffer)(__GLcontext *gc, GLenum mode);
typedef  GLvoid  (GLAPIENTRY *__T_DrawRangeElements)(__GLcontext *gc, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices);
typedef  GLvoid  (GLAPIENTRY *__T_TexImage3D)(__GLcontext *gc, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
typedef  GLvoid  (GLAPIENTRY *__T_TexSubImage3D)(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels);
typedef  GLvoid  (GLAPIENTRY *__T_CopyTexSubImage3D)(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef  GLvoid  (GLAPIENTRY *__T_CompressedTexImage3D)(__GLcontext *gc, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data);
typedef  GLvoid  (GLAPIENTRY *__T_CompressedTexSubImage3D)(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data);
typedef  GLvoid  (GLAPIENTRY *__T_GenQueries)(__GLcontext *gc, GLsizei n, GLuint* ids);
typedef  GLvoid  (GLAPIENTRY *__T_DeleteQueries)(__GLcontext *gc, GLsizei n, const GLuint* ids);
typedef  GLboolean  (GLAPIENTRY *__T_IsQuery)(__GLcontext *gc, GLuint id);
typedef  GLvoid  (GLAPIENTRY *__T_BeginQuery)(__GLcontext *gc, GLenum target, GLuint id);
typedef  GLvoid  (GLAPIENTRY *__T_EndQuery)(__GLcontext *gc, GLenum target);
typedef  GLvoid  (GLAPIENTRY *__T_GetQueryiv)(__GLcontext *gc, GLenum target, GLenum pname, GLint* params);
typedef  GLvoid  (GLAPIENTRY *__T_GetQueryObjectuiv)(__GLcontext *gc, GLuint id, GLenum pname, GLuint* params);
typedef  GLboolean  (GLAPIENTRY *__T_UnmapBuffer)(__GLcontext *gc, GLenum target);
typedef  GLvoid  (GLAPIENTRY *__T_GetBufferPointerv)(__GLcontext *gc, GLenum target, GLenum pname, GLvoid** params);
typedef  GLvoid  (GLAPIENTRY *__T_DrawBuffers)(__GLcontext *gc, GLsizei n, const GLenum* bufs);
typedef  GLvoid  (GLAPIENTRY *__T_UniformMatrix2x3fv)(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef  GLvoid  (GLAPIENTRY *__T_UniformMatrix3x2fv)(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef  GLvoid  (GLAPIENTRY *__T_UniformMatrix2x4fv)(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef  GLvoid  (GLAPIENTRY *__T_UniformMatrix4x2fv)(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef  GLvoid  (GLAPIENTRY *__T_UniformMatrix3x4fv)(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef  GLvoid  (GLAPIENTRY *__T_UniformMatrix4x3fv)(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef  GLvoid  (GLAPIENTRY *__T_BlitFramebuffer)(__GLcontext *gc, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef  GLvoid  (GLAPIENTRY *__T_RenderbufferStorageMultisample)(__GLcontext *gc, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
typedef  GLvoid  (GLAPIENTRY *__T_FramebufferTextureLayer)(__GLcontext *gc, GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
typedef  GLvoid*  (GLAPIENTRY *__T_MapBufferRange)(__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef  GLvoid  (GLAPIENTRY *__T_FlushMappedBufferRange)(__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr length);
typedef  GLvoid  (GLAPIENTRY *__T_BindVertexArray)(__GLcontext *gc, GLuint array);
typedef  GLvoid  (GLAPIENTRY *__T_DeleteVertexArrays)(__GLcontext *gc, GLsizei n, const GLuint* arrays);
typedef  GLvoid  (GLAPIENTRY *__T_GenVertexArrays)(__GLcontext *gc, GLsizei n, GLuint* arrays);
typedef  GLboolean  (GLAPIENTRY *__T_IsVertexArray)(__GLcontext *gc, GLuint array);
typedef  GLvoid  (GLAPIENTRY *__T_GetIntegeri_v)(__GLcontext *gc, GLenum target, GLuint index, GLint* data);
typedef  GLvoid  (GLAPIENTRY *__T_BeginTransformFeedback)(__GLcontext *gc, GLenum primitiveMode);
typedef  GLvoid  (GLAPIENTRY *__T_EndTransformFeedback)(__GLcontext *gc);
typedef  GLvoid  (GLAPIENTRY *__T_BindBufferRange)(__GLcontext *gc, GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
typedef  GLvoid  (GLAPIENTRY *__T_BindBufferBase)(__GLcontext *gc, GLenum target, GLuint index, GLuint buffer);
typedef  GLvoid  (GLAPIENTRY *__T_TransformFeedbackVaryings)(__GLcontext *gc, GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode);
typedef  GLvoid  (GLAPIENTRY *__T_GetTransformFeedbackVarying)(__GLcontext *gc, GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttribIPointer)(__GLcontext *gc, GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
typedef  GLvoid  (GLAPIENTRY *__T_GetVertexAttribIiv)(__GLcontext *gc, GLuint index, GLenum pname, GLint* params);
typedef  GLvoid  (GLAPIENTRY *__T_GetVertexAttribIuiv)(__GLcontext *gc, GLuint index, GLenum pname, GLuint* params);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttribI4i)(__GLcontext *gc, GLuint index, GLint x, GLint y, GLint z, GLint w);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttribI4ui)(__GLcontext *gc, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttribI4iv)(__GLcontext *gc, GLuint index, const GLint* v);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttribI4uiv)(__GLcontext *gc, GLuint index, const GLuint* v);
typedef  GLvoid  (GLAPIENTRY *__T_GetUniformuiv)(__GLcontext *gc, GLuint program, GLint location, GLuint* params);
typedef  GLint    (GLAPIENTRY *__T_GetFragDataLocation)(__GLcontext *gc, GLuint program, const GLchar *name);
typedef  GLvoid  (GLAPIENTRY *__T_BindFragDataLocation) (__GLcontext *gc, GLuint program, GLuint colorNumber, const GLchar *name);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform1ui)(__GLcontext *gc, GLint location, GLuint v0);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform2ui)(__GLcontext *gc, GLint location, GLuint v0, GLuint v1);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform3ui)(__GLcontext *gc, GLint location, GLuint v0, GLuint v1, GLuint v2);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform4ui)(__GLcontext *gc, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform1uiv)(__GLcontext *gc, GLint location, GLsizei count, const GLuint* value);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform2uiv)(__GLcontext *gc, GLint location, GLsizei count, const GLuint* value);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform3uiv)(__GLcontext *gc, GLint location, GLsizei count, const GLuint* value);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform4uiv)(__GLcontext *gc, GLint location, GLsizei count, const GLuint* value);
typedef  GLvoid  (GLAPIENTRY *__T_ClearBufferiv)(__GLcontext *gc, GLenum buffer, GLint drawbuffer, const GLint* value);
typedef  GLvoid  (GLAPIENTRY *__T_ClearBufferuiv)(__GLcontext *gc, GLenum buffer, GLint drawbuffer, const GLuint* value);
typedef  GLvoid  (GLAPIENTRY *__T_ClearBufferfv)(__GLcontext *gc, GLenum buffer, GLint drawbuffer, const GLfloat* value);
typedef  GLvoid  (GLAPIENTRY *__T_ClearBufferfi)(__GLcontext *gc, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
typedef  const GLubyte*  (GLAPIENTRY *__T_GetStringi)(__GLcontext *gc, GLenum name, GLuint index);
typedef  GLvoid  (GLAPIENTRY *__T_CopyBufferSubData)(__GLcontext *gc, GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
typedef  GLvoid  (GLAPIENTRY *__T_GetUniformIndices)(__GLcontext *gc, GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices);
typedef  GLvoid  (GLAPIENTRY *__T_GetActiveUniformsiv)(__GLcontext *gc, GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params);
typedef  GLuint  (GLAPIENTRY *__T_GetUniformBlockIndex)(__GLcontext *gc, GLuint program, const GLchar* uniformBlockName);
typedef  GLvoid  (GLAPIENTRY *__T_GetActiveUniformBlockiv)(__GLcontext *gc, GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params);
typedef  GLvoid  (GLAPIENTRY *__T_GetActiveUniformBlockName)(__GLcontext *gc, GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName);
typedef  GLvoid  (GLAPIENTRY *__T_UniformBlockBinding)(__GLcontext *gc, GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
typedef  GLvoid  (GLAPIENTRY *__T_DrawArraysInstanced)(__GLcontext *gc, GLenum mode, GLint first, GLsizei count, GLsizei instanceCount);
typedef  GLvoid  (GLAPIENTRY *__T_DrawElementsInstanced)(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount);
typedef  GLsync  (GLAPIENTRY *__T_FenceSync)(__GLcontext *gc, GLenum condition, GLbitfield flags);
typedef  GLboolean  (GLAPIENTRY *__T_IsSync)(__GLcontext *gc, GLsync sync);
typedef  GLvoid  (GLAPIENTRY *__T_DeleteSync)(__GLcontext *gc, GLsync sync);
typedef  GLenum  (GLAPIENTRY *__T_ClientWaitSync)(__GLcontext *gc, GLsync sync, GLbitfield flags, GLuint64 timeout);
typedef  GLvoid  (GLAPIENTRY *__T_WaitSync)(__GLcontext *gc, GLsync sync, GLbitfield flags, GLuint64 timeout);
typedef  GLvoid  (GLAPIENTRY *__T_GetInteger64v)(__GLcontext *gc, GLenum pname, GLint64* params);
typedef  GLvoid  (GLAPIENTRY *__T_GetSynciv)(__GLcontext *gc, GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values);
typedef  GLvoid  (GLAPIENTRY *__T_GetInteger64i_v)(__GLcontext *gc, GLenum target, GLuint index, GLint64* data);
typedef  GLvoid  (GLAPIENTRY *__T_GetBufferParameteri64v)(__GLcontext *gc, GLenum target, GLenum pname, GLint64* params);
typedef  GLvoid  (GLAPIENTRY *__T_GenSamplers)(__GLcontext *gc, GLsizei count, GLuint* samplers);
typedef  GLvoid  (GLAPIENTRY *__T_DeleteSamplers)(__GLcontext *gc, GLsizei count, const GLuint* samplers);
typedef  GLboolean  (GLAPIENTRY *__T_IsSampler)(__GLcontext *gc, GLuint sampler);
typedef  GLvoid  (GLAPIENTRY *__T_BindSampler)(__GLcontext *gc, GLuint unit, GLuint sampler);
typedef  GLvoid  (GLAPIENTRY *__T_SamplerParameteri)(__GLcontext *gc, GLuint sampler, GLenum pname, GLint param);
typedef  GLvoid  (GLAPIENTRY *__T_SamplerParameteriv)(__GLcontext *gc, GLuint sampler, GLenum pname, const GLint* param);
typedef  GLvoid  (GLAPIENTRY *__T_SamplerParameterf)(__GLcontext *gc, GLuint sampler, GLenum pname, GLfloat param);
typedef  GLvoid  (GLAPIENTRY *__T_SamplerParameterfv)(__GLcontext *gc, GLuint sampler, GLenum pname, const GLfloat* param);
typedef  GLvoid  (GLAPIENTRY *__T_GetSamplerParameteriv)(__GLcontext *gc, GLuint sampler, GLenum pname, GLint* params);
typedef  GLvoid  (GLAPIENTRY *__T_GetSamplerParameterfv)(__GLcontext *gc, GLuint sampler, GLenum pname, GLfloat* params);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttribDivisor)(__GLcontext *gc, GLuint index, GLuint divisor);
typedef  GLvoid  (GLAPIENTRY *__T_BindTransformFeedback)(__GLcontext *gc, GLenum target, GLuint id);
typedef  GLvoid  (GLAPIENTRY *__T_DeleteTransformFeedbacks)(__GLcontext *gc, GLsizei n, const GLuint* ids);
typedef  GLvoid  (GLAPIENTRY *__T_GenTransformFeedbacks)(__GLcontext *gc, GLsizei n, GLuint* ids);
typedef  GLboolean  (GLAPIENTRY *__T_IsTransformFeedback)(__GLcontext *gc, GLuint id);
typedef  GLvoid  (GLAPIENTRY *__T_PauseTransformFeedback)(__GLcontext *gc);
typedef  GLvoid  (GLAPIENTRY *__T_ResumeTransformFeedback)(__GLcontext *gc);
typedef  GLvoid  (GLAPIENTRY *__T_GetProgramBinary)(__GLcontext *gc, GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramBinary)(__GLcontext *gc, GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramParameteri)(__GLcontext *gc, GLuint program, GLenum pname, GLint value);
typedef  GLvoid  (GLAPIENTRY *__T_InvalidateFramebuffer)(__GLcontext *gc, GLenum target, GLsizei numAttachments, const GLenum* attachments);
typedef  GLvoid  (GLAPIENTRY *__T_InvalidateSubFramebuffer)(__GLcontext *gc, GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height);
typedef  GLvoid  (GLAPIENTRY *__T_TexStorage2D)(__GLcontext *gc, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
typedef  GLvoid  (GLAPIENTRY *__T_TexStorage3D)(__GLcontext *gc, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
typedef  GLvoid  (GLAPIENTRY *__T_GetInternalformativ)(__GLcontext *gc, GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params);

  /* OpenGL ES 3.1 */

typedef  GLvoid  (GLAPIENTRY *__T_DispatchCompute)(__GLcontext *gc, GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
typedef  GLvoid  (GLAPIENTRY *__T_DispatchComputeIndirect)(__GLcontext *gc, GLintptr indirect);
typedef  GLvoid  (GLAPIENTRY *__T_DrawArraysIndirect)(__GLcontext *gc, GLenum mode, const void *indirect);
typedef  GLvoid  (GLAPIENTRY *__T_DrawElementsIndirect)(__GLcontext *gc, GLenum mode, GLenum type, const void *indirect);
typedef  GLvoid  (GLAPIENTRY *__T_FramebufferParameteri)(__GLcontext *gc, GLenum target, GLenum pname, GLint param);
typedef  GLvoid  (GLAPIENTRY *__T_GetFramebufferParameteriv)(__GLcontext *gc, GLenum target, GLenum pname, GLint *params);
typedef  GLvoid  (GLAPIENTRY *__T_GetProgramInterfaceiv)(__GLcontext *gc, GLuint program, GLenum programInterface, GLenum pname, GLint *params);
typedef  GLuint  (GLAPIENTRY *__T_GetProgramResourceIndex)(__GLcontext *gc, GLuint program, GLenum programInterface, const GLchar *name);
typedef  GLvoid  (GLAPIENTRY *__T_GetProgramResourceName)(__GLcontext *gc, GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
typedef  GLvoid  (GLAPIENTRY *__T_GetProgramResourceiv)(__GLcontext *gc, GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params);
typedef  GLint  (GLAPIENTRY *__T_GetProgramResourceLocation)(__GLcontext *gc, GLuint program, GLenum programInterface, const GLchar *name);
typedef  GLvoid  (GLAPIENTRY *__T_UseProgramStages)(__GLcontext *gc, GLuint pipeline, GLbitfield stages, GLuint program);
typedef  GLvoid  (GLAPIENTRY *__T_ActiveShaderProgram)(__GLcontext *gc, GLuint pipeline, GLuint program);
typedef  GLuint  (GLAPIENTRY *__T_CreateShaderProgramv)(__GLcontext *gc, GLenum type, GLsizei count, const GLchar *const*strings);
typedef  GLvoid  (GLAPIENTRY *__T_BindProgramPipeline)(__GLcontext *gc, GLuint pipeline);
typedef  GLvoid  (GLAPIENTRY *__T_DeleteProgramPipelines)(__GLcontext *gc, GLsizei n, const GLuint *pipelines);
typedef  GLvoid  (GLAPIENTRY *__T_GenProgramPipelines)(__GLcontext *gc, GLsizei n, GLuint *pipelines);
typedef  GLboolean  (GLAPIENTRY *__T_IsProgramPipeline)(__GLcontext *gc, GLuint pipeline);
typedef  GLvoid  (GLAPIENTRY *__T_GetProgramPipelineiv)(__GLcontext *gc, GLuint pipeline, GLenum pname, GLint *params);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniform1i)(__GLcontext *gc, GLuint program, GLint location, GLint v0);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniform2i)(__GLcontext *gc, GLuint program, GLint location, GLint v0, GLint v1);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniform3i)(__GLcontext *gc, GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniform4i)(__GLcontext *gc, GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniform1ui)(__GLcontext *gc, GLuint program, GLint location, GLuint v0);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniform2ui)(__GLcontext *gc, GLuint program, GLint location, GLuint v0, GLuint v1);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniform3ui)(__GLcontext *gc, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniform4ui)(__GLcontext *gc, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniform1f)(__GLcontext *gc, GLuint program, GLint location, GLfloat v0);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniform2f)(__GLcontext *gc, GLuint program, GLint location, GLfloat v0, GLfloat v1);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniform3f)(__GLcontext *gc, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniform4f)(__GLcontext *gc, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniform1iv)(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniform2iv)(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniform3iv)(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniform4iv)(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniform1uiv)(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniform2uiv)(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniform3uiv)(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniform4uiv)(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniform1fv)(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniform2fv)(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniform3fv)(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniform4fv)(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniformMatrix2fv)(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniformMatrix3fv)(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniformMatrix4fv)(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniformMatrix2x3fv)(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniformMatrix3x2fv)(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniformMatrix2x4fv)(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniformMatrix4x2fv)(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniformMatrix3x4fv)(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef  GLvoid  (GLAPIENTRY *__T_ProgramUniformMatrix4x3fv)(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef  GLvoid  (GLAPIENTRY *__T_ValidateProgramPipeline)(__GLcontext *gc, GLuint pipeline);
typedef  GLvoid  (GLAPIENTRY *__T_GetProgramPipelineInfoLog)(__GLcontext *gc, GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef  GLvoid  (GLAPIENTRY *__T_BindImageTexture)(__GLcontext *gc, GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
typedef  GLvoid  (GLAPIENTRY *__T_GetBooleani_v)(__GLcontext *gc, GLenum target, GLuint index, GLboolean *data);
typedef  GLvoid  (GLAPIENTRY *__T_MemoryBarrier)(__GLcontext *gc, GLbitfield barriers);
typedef  GLvoid  (GLAPIENTRY *__T_MemoryBarrierByRegion)(__GLcontext *gc, GLbitfield barriers);
typedef  GLvoid  (GLAPIENTRY *__T_TexStorage2DMultisample)(__GLcontext *gc, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
typedef  GLvoid  (GLAPIENTRY *__T_GetMultisamplefv)(__GLcontext *gc, GLenum pname, GLuint index, GLfloat *val);
typedef  GLvoid  (GLAPIENTRY *__T_SampleMaski)(__GLcontext *gc, GLuint maskNumber, GLbitfield mask);
typedef  GLvoid  (GLAPIENTRY *__T_GetTexLevelParameteriv)(__GLcontext *gc, GLenum target, GLint level, GLenum pname, GLint *params);
typedef  GLvoid  (GLAPIENTRY *__T_GetTexLevelParameterfv)(__GLcontext *gc, GLenum target, GLint level, GLenum pname, GLfloat *params);
typedef  GLvoid  (GLAPIENTRY *__T_BindVertexBuffer)(__GLcontext *gc, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttribFormat)(__GLcontext *gc, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttribIFormat)(__GLcontext *gc, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
typedef  GLvoid  (GLAPIENTRY *__T_VertexAttribBinding)(__GLcontext *gc, GLuint attribindex, GLuint bindingindex);
typedef  GLvoid  (GLAPIENTRY *__T_VertexBindingDivisor)(__GLcontext *gc, GLuint bindingindex, GLuint divisor);

  /* OpenGL ES 3.2 */
typedef  GLvoid  (GLAPIENTRY *__T_TexStorage3DMultisample)(__GLcontext *gc, GLenum target, GLsizei samples, GLenum sizedinternalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);

typedef  GLvoid  (GLAPIENTRY *__T_BlendBarrier)(__GLcontext *gc);

typedef  GLvoid  (GLAPIENTRY *__T_DebugMessageControl)(__GLcontext *gc, GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);
typedef  GLvoid  (GLAPIENTRY *__T_DebugMessageInsert)(__GLcontext *gc, GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf);
typedef  GLvoid  (GLAPIENTRY *__T_DebugMessageCallback)(__GLcontext *gc, GLDEBUGPROCKHR callback, const GLvoid* userParam);
typedef  GLuint  (GLAPIENTRY *__T_GetDebugMessageLog)(__GLcontext *gc, GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog);
typedef  GLvoid  (GLAPIENTRY *__T_GetPointerv)(__GLcontext *gc, GLenum pname, GLvoid** params);
typedef  GLvoid  (GLAPIENTRY *__T_PushDebugGroup)(__GLcontext *gc, GLenum source, GLuint id, GLsizei length, const GLchar * message);
typedef  GLvoid  (GLAPIENTRY *__T_PopDebugGroup)(__GLcontext *gc);
typedef  GLvoid  (GLAPIENTRY *__T_ObjectLabel)(__GLcontext *gc, GLenum identifier, GLuint name, GLsizei length, const GLchar *label);
typedef  GLvoid  (GLAPIENTRY *__T_GetObjectLabel)(__GLcontext *gc, GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label);
typedef  GLvoid  (GLAPIENTRY *__T_ObjectPtrLabel)(__GLcontext *gc, const GLvoid* ptr, GLsizei length, const GLchar *label);
typedef  GLvoid  (GLAPIENTRY *__T_GetObjectPtrLabel)(__GLcontext *gc, const GLvoid* ptr, GLsizei bufSize, GLsizei *length, GLchar *label);


typedef  GLenum  (GLAPIENTRY *__T_GetGraphicsResetStatus)(__GLcontext *gc);
typedef  GLvoid  (GLAPIENTRY *__T_ReadnPixels)(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLvoid *data);
typedef  GLvoid  (GLAPIENTRY *__T_GetnUniformfv)(__GLcontext *gc, GLuint program, GLint location, GLsizei bufSize, GLfloat *params);
typedef  GLvoid  (GLAPIENTRY *__T_GetnUniformiv)(__GLcontext *gc, GLuint program, GLint location, GLsizei bufSize, GLint *params);
typedef  GLvoid  (GLAPIENTRY *__T_GetnUniformuiv)(__GLcontext *gc, GLuint program, GLint location, GLsizei bufSize, GLuint *params);

typedef  GLvoid  (GLAPIENTRY *__T_BlendEquationi)(__GLcontext * gc, GLuint buf, GLenum mode);
typedef  GLvoid  (GLAPIENTRY *__T_BlendEquationSeparatei)(__GLcontext * gc, GLuint buf, GLenum modeRGB, GLenum modeAlpha);
typedef  GLvoid  (GLAPIENTRY *__T_BlendFunci)(__GLcontext * gc, GLuint buf, GLenum sfactor, GLenum dfactor);
typedef  GLvoid  (GLAPIENTRY *__T_BlendFuncSeparatei)(__GLcontext * gc, GLuint buf, GLenum sfactorRGB,GLenum dfactorRGB,GLenum sfactorAlpha,GLenum dfactorAlpha);
typedef  GLvoid  (GLAPIENTRY *__T_ColorMaski)(__GLcontext * gc,GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
typedef  GLvoid  (GLAPIENTRY *__T_Enablei)(__GLcontext *gc, GLenum target, GLuint index);
typedef  GLvoid  (GLAPIENTRY *__T_Disablei)(__GLcontext *gc, GLenum target, GLuint index);
typedef  GLboolean  (GLAPIENTRY *__T_IsEnabledi)(__GLcontext * gc, GLenum target, GLuint index);

typedef  GLvoid  (GLAPIENTRY *__T_TexParameterIiv)(__GLcontext *gc, GLenum target, GLenum pname, const GLint *params);
typedef  GLvoid  (GLAPIENTRY *__T_TexParameterIuiv)(__GLcontext *gc, GLenum target, GLenum pname, const GLuint *params);
typedef  GLvoid  (GLAPIENTRY *__T_GetTexParameterIiv)(__GLcontext *gc, GLenum target, GLenum pname, GLint *params);
typedef  GLvoid  (GLAPIENTRY *__T_GetTexParameterIuiv)(__GLcontext *gc, GLenum target, GLenum pname, GLuint *params);
typedef  GLvoid  (GLAPIENTRY *__T_SamplerParameterIiv)(__GLcontext *gc, GLuint sampler, GLenum pname, const GLint *param);
typedef  GLvoid  (GLAPIENTRY *__T_SamplerParameterIuiv)(__GLcontext *gc, GLuint sampler, GLenum pname, const GLuint *param);
typedef  GLvoid  (GLAPIENTRY *__T_GetSamplerParameterIiv)(__GLcontext *gc, GLuint sampler, GLenum pname, GLint *params);
typedef  GLvoid  (GLAPIENTRY *__T_GetSamplerParameterIuiv)(__GLcontext *gc, GLuint sampler, GLenum pname, GLuint *params);

typedef  GLvoid  (GLAPIENTRY *__T_TexBuffer)(__GLcontext *gc, GLenum target, GLenum internalformat, GLuint buffer);
typedef  GLvoid  (GLAPIENTRY *__T_TexBufferRange)(__GLcontext *gc, GLenum target, GLenum internalformat,
                      GLuint buffer, GLintptr offset, GLsizeiptr size);
typedef  GLvoid  (GLAPIENTRY *__T_PatchParameteri)(__GLcontext *gc, GLenum pname, GLint value);

typedef  GLvoid  (GLAPIENTRY *__T_FramebufferTexture)(__GLcontext *gc, GLenum target, GLenum attachment, GLuint texture, GLint level);

typedef  GLvoid  (GLAPIENTRY *__T_MinSampleShading)(__GLcontext *gc, GLfloat value);

typedef  GLvoid  (GLAPIENTRY *__T_CopyImageSubData)(__GLcontext *gc,
     GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ,
     GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ,
     GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth);

typedef  GLvoid  (GLAPIENTRY *__T_DrawElementsBaseVertex)(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex);
typedef  GLvoid  (GLAPIENTRY *__T_DrawRangeElementsBaseVertex)(__GLcontext *gc, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex);
typedef  GLvoid  (GLAPIENTRY *__T_DrawElementsInstancedBaseVertex)(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex);
typedef  GLvoid  (GLAPIENTRY *__T_PrimitiveBoundingBox)(__GLcontext *gc, GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW);

typedef  GLvoid  (GLAPIENTRY *__T_EGLImageTargetTexture2DOES)(__GLcontext *gc, GLenum target, GLeglImageOES image);
typedef  GLvoid  (GLAPIENTRY *__T_EGLImageTargetRenderbufferStorageOES)(__GLcontext *gc, GLenum target, GLeglImageOES image);


typedef  GLvoid  (GLAPIENTRY *__T_MultiDrawArraysEXT)(__GLcontext *gc, GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount);
typedef  GLvoid  (GLAPIENTRY *__T_MultiDrawElementsEXT)(__GLcontext *gc, GLenum mode, const GLsizei *count, GLenum type, const GLvoid*const*indices, GLsizei primcount);

typedef  GLvoid  (GLAPIENTRY *__T_MultiDrawElementsBaseVertexEXT)(__GLcontext *gc, GLenum mode, const GLsizei * count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint * basevertex);


typedef  GLvoid  (GLAPIENTRY *__T_GetBufferPointervOES)(__GLcontext *gc, GLenum target, GLenum pname, GLvoid** params);
typedef  GLvoid*  (GLAPIENTRY *__T_MapBufferOES)(__GLcontext *gc, GLenum target, GLenum access);
typedef  GLboolean  (GLAPIENTRY *__T_UnmapBufferOES)(__GLcontext *gc, GLenum target);


typedef  GLvoid  (GLAPIENTRY *__T_DiscardFramebufferEXT)(__GLcontext *gc, GLenum target, GLsizei numAttachments, const GLenum *attachments);



typedef  GLvoid  (GLAPIENTRY *__T_RenderbufferStorageMultisampleEXT)(__GLcontext *gc, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
typedef  GLvoid  (GLAPIENTRY *__T_FramebufferTexture2DMultisampleEXT)(__GLcontext *gc, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples);


typedef  GLvoid  (GLAPIENTRY *__T_TexDirectVIV)(__GLcontext *gc, GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** pixels);
typedef  GLvoid  (GLAPIENTRY *__T_TexDirectInvalidateVIV)(__GLcontext *gc, GLenum target);
typedef  GLvoid  (GLAPIENTRY *__T_TexDirectVIVMap)(__GLcontext *gc, GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** logical, const GLuint * physical);
typedef  GLvoid  (GLAPIENTRY *__T_TexDirectTiledMapVIV)(__GLcontext *gc, GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** logical, const GLuint * physical);


typedef  GLvoid  (GLAPIENTRY *__T_MultiDrawArraysIndirectEXT)(__GLcontext *gc, GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride);
typedef  GLvoid  (GLAPIENTRY *__T_MultiDrawElementsIndirectEXT)(__GLcontext *gc, GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride);


typedef  GLvoid  (GLAPIENTRY *__T_GetObjectParameterivARB )(__GLcontext *gc, UINT obj, GLenum pname, GLint *params);

typedef  GLvoid  (GLAPIENTRY *__T_GetVertexAttribdv)(__GLcontext *gc, GLuint index, GLenum pname, GLdouble* params);
typedef  GLvoid  (GLAPIENTRY *__T_GetQueryObjectiv)(__GLcontext *gc, GLuint id, GLenum pname, GLint* params);
typedef  GLvoid  (GLAPIENTRY *__T_DeleteObjectARB)(__GLcontext *gc, UINT obj);
typedef  GLvoid  (GLAPIENTRY *__T_GetInfoLogARB )(__GLcontext *gc, UINT obj, GLsizei maxLength, GLsizei *length, char *infoLog);

typedef  GLboolean (GLAPIENTRY *__T_IsRenderbufferEXT)( __GLcontext *gc,  GLuint renderbuffer);
typedef  GLvoid (GLAPIENTRY *__T_BindRenderbufferEXT)(__GLcontext *gc,  GLenum target, GLuint renderbuffer);
typedef  GLvoid (GLAPIENTRY *__T_DeleteRenderbuffersEXT)(__GLcontext *gc,  GLsizei n, const GLuint *renderbuffers);
typedef  GLvoid (GLAPIENTRY *__T_GenRenderbuffersEXT)(__GLcontext *gc,  GLsizei n, GLuint *renderbuffers);
typedef  GLvoid (GLAPIENTRY *__T_RenderbufferStorageEXT)(__GLcontext *gc,  GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef  GLvoid (GLAPIENTRY *__T_GetRenderbufferParameterivEXT)(__GLcontext *gc,  GLenum target, GLenum pname, GLint* params);
typedef  GLboolean (GLAPIENTRY *__T_IsFramebufferEXT)(__GLcontext *gc,  GLuint framebuffer);
typedef  GLvoid (GLAPIENTRY *__T_BindFramebufferEXT)(__GLcontext *gc,  GLenum target, GLuint framebuffer);
typedef  GLvoid (GLAPIENTRY *__T_DeleteFramebuffersEXT)(__GLcontext *gc,  GLsizei n, const GLuint *framebuffers);
typedef  GLvoid (GLAPIENTRY *__T_GenFramebuffersEXT)(__GLcontext *gc,  GLsizei n, GLuint *framebuffers);
typedef  GLenum (GLAPIENTRY *__T_CheckFramebufferStatusEXT)(__GLcontext *gc,  GLenum target);
typedef  GLvoid (GLAPIENTRY *__T_FramebufferTexture1DEXT)(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef  GLvoid (GLAPIENTRY *__T_FramebufferTexture2DEXT)(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef  GLvoid (GLAPIENTRY *__T_FramebufferTexture3DEXT)(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
typedef  GLvoid (GLAPIENTRY *__T_FramebufferRenderbufferEXT)(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef  GLvoid (GLAPIENTRY *__T_GetFramebufferAttachmentParameterivEXT)(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum pname, GLint *params);
typedef  GLvoid (GLAPIENTRY *__T_GenerateMipmapEXT)(__GLcontext *gc,  GLenum target);
typedef  GLvoid (GLAPIENTRY *__T_BlitFramebufferEXT)(__GLcontext *gc,  GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);

/* OpenGL 3.0 */
typedef GLvoid (GLAPIENTRY *__T_ClampColor)(__GLcontext *, GLenum target, GLenum clamp);
typedef GLvoid (GLAPIENTRY *__T_BeginConditionalRender) (__GLcontext *,GLuint id, GLenum mode);
typedef GLvoid (GLAPIENTRY *__T_EndConditionalRender) (__GLcontext *);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI1i)(__GLcontext *, GLuint index, GLint x);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI2i)(__GLcontext *, GLuint index, GLint x, GLint y);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI3i)(__GLcontext *, GLuint index, GLint x, GLint y, GLint z);

typedef GLvoid (GLAPIENTRY *__T_VertexAttribI1ui)(__GLcontext *, GLuint index, GLuint x);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI2ui)(__GLcontext *, GLuint index, GLuint x, GLuint y);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI3ui)(__GLcontext *, GLuint index, GLuint x, GLuint y, GLuint z);

typedef GLvoid (GLAPIENTRY *__T_VertexAttribI1iv)(__GLcontext *, GLuint index, const GLint *v);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI2iv)(__GLcontext *, GLuint index, const GLint *v);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI3iv)(__GLcontext *, GLuint index, const GLint *v);

typedef GLvoid (GLAPIENTRY *__T_VertexAttribI1uiv)(__GLcontext *, GLuint index, const GLuint *v);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI2uiv)(__GLcontext *, GLuint index, const GLuint *v);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI3uiv)(__GLcontext *, GLuint index, const GLuint *v);

typedef GLvoid (GLAPIENTRY *__T_VertexAttribI4bv)(__GLcontext *, GLuint index, const GLbyte *v);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI4sv)(__GLcontext *, GLuint index, const GLshort *v);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI4ubv)(__GLcontext *, GLuint index, const GLubyte *v);
typedef GLvoid (GLAPIENTRY *__T_VertexAttribI4usv)(__GLcontext *, GLuint index, const GLushort *v);

typedef  GLvoid (GLAPIENTRY *__T_FramebufferTexture1D)(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef  GLvoid (GLAPIENTRY *__T_FramebufferTexture3D)(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);

/* OpenGL 3.1 */
typedef GLvoid (GLAPIENTRY *__T_PrimitiveRestartIndex) (__GLcontext *, GLuint index);
typedef GLvoid (GLAPIENTRY *__T_GetActiveUniformName) (__GLcontext *, GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName);

/* OpenGL 4.0 */
typedef  GLvoid  (GLAPIENTRY *__T_GetUniformdv)(__GLcontext * gc, GLuint program, GLint location, GLdouble* params);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform1d)(__GLcontext *gc, GLint location, GLdouble v0);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform2d)(__GLcontext *gc, GLint location, GLdouble v0, GLdouble v1);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform3d)(__GLcontext *gc, GLint location, GLdouble v0, GLdouble v1, GLdouble v2);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform4d)(__GLcontext *gc, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform1dv)(__GLcontext *gc, GLint location, GLsizei count, const GLdouble* value);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform2dv)(__GLcontext *gc, GLint location, GLsizei count, const GLdouble* value);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform3dv)(__GLcontext *gc, GLint location, GLsizei count, const GLdouble* value);
typedef  GLvoid  (GLAPIENTRY *__T_Uniform4dv)(__GLcontext *gc, GLint location, GLsizei count, const GLdouble* value);
typedef  GLvoid  (GLAPIENTRY *__T_UniformMatrix2dv)(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
typedef  GLvoid  (GLAPIENTRY *__T_UniformMatrix3dv)(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
typedef  GLvoid  (GLAPIENTRY *__T_UniformMatrix4dv)(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
typedef  GLvoid  (GLAPIENTRY *__T_UniformMatrix2x3dv)(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
typedef  GLvoid  (GLAPIENTRY *__T_UniformMatrix3x2dv)(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
typedef  GLvoid  (GLAPIENTRY *__T_UniformMatrix2x4dv)(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
typedef  GLvoid  (GLAPIENTRY *__T_UniformMatrix4x2dv)(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
typedef  GLvoid  (GLAPIENTRY *__T_UniformMatrix3x4dv)(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
typedef  GLvoid  (GLAPIENTRY *__T_UniformMatrix4x3dv)(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);




#define __GLES_API_ENTRIES(esApiMacro) \
    esApiMacro(NewList), \
    esApiMacro(EndList), \
    esApiMacro(CallList), \
    esApiMacro(CallLists), \
    esApiMacro(DeleteLists), \
    esApiMacro(GenLists), \
    esApiMacro(ListBase), \
    esApiMacro(Begin), \
    esApiMacro(Bitmap), \
    esApiMacro(Color3b), \
    esApiMacro(Color3bv), \
    esApiMacro(Color3d), \
    esApiMacro(Color3dv), \
    esApiMacro(Color3f), \
    esApiMacro(Color3fv), \
    esApiMacro(Color3i), \
    esApiMacro(Color3iv), \
    esApiMacro(Color3s), \
    esApiMacro(Color3sv), \
    esApiMacro(Color3ub), \
    esApiMacro(Color3ubv), \
    esApiMacro(Color3ui), \
    esApiMacro(Color3uiv), \
    esApiMacro(Color3us), \
    esApiMacro(Color3usv), \
    esApiMacro(Color4b), \
    esApiMacro(Color4bv), \
    esApiMacro(Color4d), \
    esApiMacro(Color4dv), \
    esApiMacro(Color4f), \
    esApiMacro(Color4fv), \
    esApiMacro(Color4i), \
    esApiMacro(Color4iv), \
    esApiMacro(Color4s), \
    esApiMacro(Color4sv), \
    esApiMacro(Color4ub), \
    esApiMacro(Color4ubv), \
    esApiMacro(Color4ui), \
    esApiMacro(Color4uiv), \
    esApiMacro(Color4us), \
    esApiMacro(Color4usv), \
    esApiMacro(EdgeFlag), \
    esApiMacro(EdgeFlagv), \
    esApiMacro(End), \
    esApiMacro(Indexd), \
    esApiMacro(Indexdv), \
    esApiMacro(Indexf), \
    esApiMacro(Indexfv), \
    esApiMacro(Indexi), \
    esApiMacro(Indexiv), \
    esApiMacro(Indexs), \
    esApiMacro(Indexsv), \
    esApiMacro(Normal3b), \
    esApiMacro(Normal3bv), \
    esApiMacro(Normal3d), \
    esApiMacro(Normal3dv), \
    esApiMacro(Normal3f), \
    esApiMacro(Normal3fv), \
    esApiMacro(Normal3i), \
    esApiMacro(Normal3iv), \
    esApiMacro(Normal3s), \
    esApiMacro(Normal3sv), \
    esApiMacro(RasterPos2d), \
    esApiMacro(RasterPos2dv), \
    esApiMacro(RasterPos2f), \
    esApiMacro(RasterPos2fv), \
    esApiMacro(RasterPos2i), \
    esApiMacro(RasterPos2iv), \
    esApiMacro(RasterPos2s), \
    esApiMacro(RasterPos2sv), \
    esApiMacro(RasterPos3d), \
    esApiMacro(RasterPos3dv), \
    esApiMacro(RasterPos3f), \
    esApiMacro(RasterPos3fv), \
    esApiMacro(RasterPos3i), \
    esApiMacro(RasterPos3iv), \
    esApiMacro(RasterPos3s), \
    esApiMacro(RasterPos3sv), \
    esApiMacro(RasterPos4d), \
    esApiMacro(RasterPos4dv), \
    esApiMacro(RasterPos4f), \
    esApiMacro(RasterPos4fv), \
    esApiMacro(RasterPos4i), \
    esApiMacro(RasterPos4iv), \
    esApiMacro(RasterPos4s), \
    esApiMacro(RasterPos4sv), \
    esApiMacro(Rectd), \
    esApiMacro(Rectdv), \
    esApiMacro(Rectf), \
    esApiMacro(Rectfv), \
    esApiMacro(Recti), \
    esApiMacro(Rectiv), \
    esApiMacro(Rects), \
    esApiMacro(Rectsv), \
    esApiMacro(TexCoord1d), \
    esApiMacro(TexCoord1dv), \
    esApiMacro(TexCoord1f), \
    esApiMacro(TexCoord1fv), \
    esApiMacro(TexCoord1i), \
    esApiMacro(TexCoord1iv), \
    esApiMacro(TexCoord1s), \
    esApiMacro(TexCoord1sv), \
    esApiMacro(TexCoord2d), \
    esApiMacro(TexCoord2dv), \
    esApiMacro(TexCoord2f), \
    esApiMacro(TexCoord2fv), \
    esApiMacro(TexCoord2i), \
    esApiMacro(TexCoord2iv), \
    esApiMacro(TexCoord2s), \
    esApiMacro(TexCoord2sv), \
    esApiMacro(TexCoord3d), \
    esApiMacro(TexCoord3dv), \
    esApiMacro(TexCoord3f), \
    esApiMacro(TexCoord3fv), \
    esApiMacro(TexCoord3i), \
    esApiMacro(TexCoord3iv), \
    esApiMacro(TexCoord3s), \
    esApiMacro(TexCoord3sv), \
    esApiMacro(TexCoord4d), \
    esApiMacro(TexCoord4dv), \
    esApiMacro(TexCoord4f), \
    esApiMacro(TexCoord4fv), \
    esApiMacro(TexCoord4i), \
    esApiMacro(TexCoord4iv), \
    esApiMacro(TexCoord4s), \
    esApiMacro(TexCoord4sv), \
    esApiMacro(Vertex2d), \
    esApiMacro(Vertex2dv), \
    esApiMacro(Vertex2f), \
    esApiMacro(Vertex2fv), \
    esApiMacro(Vertex2i), \
    esApiMacro(Vertex2iv), \
    esApiMacro(Vertex2s), \
    esApiMacro(Vertex2sv), \
    esApiMacro(Vertex3d), \
    esApiMacro(Vertex3dv), \
    esApiMacro(Vertex3f), \
    esApiMacro(Vertex3fv), \
    esApiMacro(Vertex3i), \
    esApiMacro(Vertex3iv), \
    esApiMacro(Vertex3s), \
    esApiMacro(Vertex3sv), \
    esApiMacro(Vertex4d), \
    esApiMacro(Vertex4dv), \
    esApiMacro(Vertex4f), \
    esApiMacro(Vertex4fv), \
    esApiMacro(Vertex4i), \
    esApiMacro(Vertex4iv), \
    esApiMacro(Vertex4s), \
    esApiMacro(Vertex4sv), \
    esApiMacro(ClipPlane), \
    esApiMacro(ColorMaterial), \
    esApiMacro(CullFace), \
    esApiMacro(Fogf), \
    esApiMacro(Fogfv), \
    esApiMacro(Fogi), \
    esApiMacro(Fogiv), \
    esApiMacro(FrontFace), \
    esApiMacro(Hint), \
    esApiMacro(Lightf), \
    esApiMacro(Lightfv), \
    esApiMacro(Lighti), \
    esApiMacro(Lightiv), \
    esApiMacro(LightModelf), \
    esApiMacro(LightModelfv), \
    esApiMacro(LightModeli), \
    esApiMacro(LightModeliv), \
    esApiMacro(LineStipple), \
    esApiMacro(LineWidth), \
    esApiMacro(Materialf), \
    esApiMacro(Materialfv), \
    esApiMacro(Materiali), \
    esApiMacro(Materialiv), \
    esApiMacro(PointSize), \
    esApiMacro(PolygonMode), \
    esApiMacro(PolygonStipple), \
    esApiMacro(Scissor), \
    esApiMacro(ShadeModel), \
    esApiMacro(TexParameterf), \
    esApiMacro(TexParameterfv), \
    esApiMacro(TexParameteri), \
    esApiMacro(TexParameteriv), \
    esApiMacro(TexImage1D), \
    esApiMacro(TexImage2D), \
    esApiMacro(TexEnvf), \
    esApiMacro(TexEnvfv), \
    esApiMacro(TexEnvi), \
    esApiMacro(TexEnviv), \
    esApiMacro(TexGend), \
    esApiMacro(TexGendv), \
    esApiMacro(TexGenf), \
    esApiMacro(TexGenfv), \
    esApiMacro(TexGeni), \
    esApiMacro(TexGeniv), \
    esApiMacro(FeedbackBuffer), \
    esApiMacro(SelectBuffer), \
    esApiMacro(RenderMode), \
    esApiMacro(InitNames), \
    esApiMacro(LoadName), \
    esApiMacro(PassThrough), \
    esApiMacro(PopName), \
    esApiMacro(PushName), \
    esApiMacro(DrawBuffer), \
    esApiMacro(Clear), \
    esApiMacro(ClearAccum), \
    esApiMacro(ClearIndex), \
    esApiMacro(ClearColor), \
    esApiMacro(ClearStencil), \
    esApiMacro(ClearDepth), \
    esApiMacro(StencilMask), \
    esApiMacro(ColorMask), \
    esApiMacro(DepthMask), \
    esApiMacro(IndexMask), \
    esApiMacro(Accum), \
    esApiMacro(Disable), \
    esApiMacro(Enable), \
    esApiMacro(Finish), \
    esApiMacro(Flush), \
    esApiMacro(PopAttrib), \
    esApiMacro(PushAttrib), \
    esApiMacro(Map1d), \
    esApiMacro(Map1f), \
    esApiMacro(Map2d), \
    esApiMacro(Map2f), \
    esApiMacro(MapGrid1d), \
    esApiMacro(MapGrid1f), \
    esApiMacro(MapGrid2d), \
    esApiMacro(MapGrid2f), \
    esApiMacro(EvalCoord1d), \
    esApiMacro(EvalCoord1dv), \
    esApiMacro(EvalCoord1f), \
    esApiMacro(EvalCoord1fv), \
    esApiMacro(EvalCoord2d), \
    esApiMacro(EvalCoord2dv), \
    esApiMacro(EvalCoord2f), \
    esApiMacro(EvalCoord2fv), \
    esApiMacro(EvalMesh1), \
    esApiMacro(EvalPoint1), \
    esApiMacro(EvalMesh2), \
    esApiMacro(EvalPoint2), \
    esApiMacro(AlphaFunc), \
    esApiMacro(BlendFunc), \
    esApiMacro(LogicOp), \
    esApiMacro(StencilFunc), \
    esApiMacro(StencilOp), \
    esApiMacro(DepthFunc), \
    esApiMacro(PixelZoom), \
    esApiMacro(PixelTransferf), \
    esApiMacro(PixelTransferi), \
    esApiMacro(PixelStoref), \
    esApiMacro(PixelStorei), \
    esApiMacro(PixelMapfv), \
    esApiMacro(PixelMapuiv), \
    esApiMacro(PixelMapusv), \
    esApiMacro(ReadBuffer), \
    esApiMacro(CopyPixels), \
    esApiMacro(ReadPixels), \
    esApiMacro(DrawPixels), \
    esApiMacro(GetBooleanv), \
    esApiMacro(GetClipPlane), \
    esApiMacro(GetDoublev), \
    esApiMacro(GetError), \
    esApiMacro(GetFloatv), \
    esApiMacro(GetIntegerv), \
    esApiMacro(GetLightfv), \
    esApiMacro(GetLightiv), \
    esApiMacro(GetMapdv), \
    esApiMacro(GetMapfv), \
    esApiMacro(GetMapiv), \
    esApiMacro(GetMaterialfv), \
    esApiMacro(GetMaterialiv), \
    esApiMacro(GetPixelMapfv), \
    esApiMacro(GetPixelMapuiv), \
    esApiMacro(GetPixelMapusv), \
    esApiMacro(GetPolygonStipple), \
    esApiMacro(GetString), \
    esApiMacro(GetTexEnvfv), \
    esApiMacro(GetTexEnviv), \
    esApiMacro(GetTexGendv), \
    esApiMacro(GetTexGenfv), \
    esApiMacro(GetTexGeniv), \
    esApiMacro(GetTexImage), \
    esApiMacro(GetTexParameterfv), \
    esApiMacro(GetTexParameteriv), \
    esApiMacro(GetTexLevelParameterfv), \
    esApiMacro(GetTexLevelParameteriv), \
    esApiMacro(IsEnabled), \
    esApiMacro(IsList), \
    esApiMacro(DepthRange), \
    esApiMacro(Frustum), \
    esApiMacro(LoadIdentity), \
    esApiMacro(LoadMatrixf), \
    esApiMacro(LoadMatrixd), \
    esApiMacro(MatrixMode), \
    esApiMacro(MultMatrixf), \
    esApiMacro(MultMatrixd), \
    esApiMacro(Ortho), \
    esApiMacro(PopMatrix), \
    esApiMacro(PushMatrix), \
    esApiMacro(Rotated), \
    esApiMacro(Rotatef), \
    esApiMacro(Scaled), \
    esApiMacro(Scalef), \
    esApiMacro(Translated), \
    esApiMacro(Translatef), \
    esApiMacro(Viewport), \
    esApiMacro(ArrayElement), \
    esApiMacro(BindTexture), \
    esApiMacro(ColorPointer), \
    esApiMacro(DisableClientState), \
    esApiMacro(DrawArrays), \
    esApiMacro(DrawElements), \
    esApiMacro(EdgeFlagPointer), \
    esApiMacro(EnableClientState), \
    esApiMacro(IndexPointer), \
    esApiMacro(Indexub), \
    esApiMacro(Indexubv), \
    esApiMacro(InterleavedArrays), \
    esApiMacro(NormalPointer), \
    esApiMacro(PolygonOffset), \
    esApiMacro(TexCoordPointer),\
    esApiMacro(VertexPointer), \
    esApiMacro(AreTexturesResident), \
    esApiMacro(CopyTexImage1D), \
    esApiMacro(CopyTexImage2D), \
    esApiMacro(CopyTexSubImage1D), \
    esApiMacro(CopyTexSubImage2D), \
    esApiMacro(DeleteTextures), \
    esApiMacro(GenTextures), \
    esApiMacro(GetPointerv), \
    esApiMacro(IsTexture), \
    esApiMacro(PrioritizeTextures), \
    esApiMacro(TexSubImage1D), \
    esApiMacro(TexSubImage2D), \
    esApiMacro(PopClientAttrib), \
    esApiMacro(PushClientAttrib), \
    esApiMacro(BlendColor), \
    esApiMacro(BlendEquation), \
    esApiMacro(DrawRangeElements), \
    esApiMacro(ColorTable), \
    esApiMacro(ColorTableParameterfv), \
    esApiMacro(ColorTableParameteriv), \
    esApiMacro(CopyColorTable), \
    esApiMacro(GetColorTable), \
    esApiMacro(GetColorTableParameterfv), \
    esApiMacro(GetColorTableParameteriv), \
    esApiMacro(ColorSubTable), \
    esApiMacro(CopyColorSubTable), \
    esApiMacro(ConvolutionFilter1D), \
    esApiMacro(ConvolutionFilter2D), \
    esApiMacro(ConvolutionParameterf), \
    esApiMacro(ConvolutionParameterfv), \
    esApiMacro(ConvolutionParameteri), \
    esApiMacro(ConvolutionParameteriv), \
    esApiMacro(CopyConvolutionFilter1D), \
    esApiMacro(CopyConvolutionFilter2D), \
    esApiMacro(GetConvolutionFilter), \
    esApiMacro(GetConvolutionParameterfv), \
    esApiMacro(GetConvolutionParameteriv), \
    esApiMacro(GetSeparableFilter), \
    esApiMacro(SeparableFilter2D), \
    esApiMacro(GetHistogram), \
    esApiMacro(GetHistogramParameterfv), \
    esApiMacro(GetHistogramParameteriv), \
    esApiMacro(GetMinmax), \
    esApiMacro(GetMinmaxParameterfv), \
    esApiMacro(GetMinmaxParameteriv), \
    esApiMacro(Histogram), \
    esApiMacro(Minmax), \
    esApiMacro(ResetHistogram), \
    esApiMacro(ResetMinmax), \
    esApiMacro(TexImage3D), \
    esApiMacro(TexSubImage3D), \
    esApiMacro(CopyTexSubImage3D), \
    esApiMacro(ActiveTexture), \
    esApiMacro(ClientActiveTexture), \
    esApiMacro(MultiTexCoord1d), \
    esApiMacro(MultiTexCoord1dv), \
    esApiMacro(MultiTexCoord1f), \
    esApiMacro(MultiTexCoord1fv), \
    esApiMacro(MultiTexCoord1i), \
    esApiMacro(MultiTexCoord1iv), \
    esApiMacro(MultiTexCoord1s), \
    esApiMacro(MultiTexCoord1sv), \
    esApiMacro(MultiTexCoord2d), \
    esApiMacro(MultiTexCoord2dv), \
    esApiMacro(MultiTexCoord2f), \
    esApiMacro(MultiTexCoord2fv), \
    esApiMacro(MultiTexCoord2i), \
    esApiMacro(MultiTexCoord2iv), \
    esApiMacro(MultiTexCoord2s), \
    esApiMacro(MultiTexCoord2sv), \
    esApiMacro(MultiTexCoord3d), \
    esApiMacro(MultiTexCoord3dv), \
    esApiMacro(MultiTexCoord3f), \
    esApiMacro(MultiTexCoord3fv), \
    esApiMacro(MultiTexCoord3i), \
    esApiMacro(MultiTexCoord3iv), \
    esApiMacro(MultiTexCoord3s), \
    esApiMacro(MultiTexCoord3sv), \
    esApiMacro(MultiTexCoord4d), \
    esApiMacro(MultiTexCoord4dv), \
    esApiMacro(MultiTexCoord4f), \
    esApiMacro(MultiTexCoord4fv), \
    esApiMacro(MultiTexCoord4i), \
    esApiMacro(MultiTexCoord4iv), \
    esApiMacro(MultiTexCoord4s), \
    esApiMacro(MultiTexCoord4sv), \
    esApiMacro(LoadTransposeMatrixf), \
    esApiMacro(LoadTransposeMatrixd), \
    esApiMacro(MultTransposeMatrixf), \
    esApiMacro(MultTransposeMatrixd), \
    esApiMacro(SampleCoverage), \
    esApiMacro(CompressedTexImage3D), \
    esApiMacro(CompressedTexImage2D), \
    esApiMacro(CompressedTexImage1D), \
    esApiMacro(CompressedTexSubImage3D), \
    esApiMacro(CompressedTexSubImage2D), \
    esApiMacro(CompressedTexSubImage1D), \
    esApiMacro(GetCompressedTexImage), \
    esApiMacro(BlendFuncSeparate), \
    esApiMacro(MultiDrawArrays), \
    esApiMacro(MultiDrawElements), \
    esApiMacro(FogCoordf), \
    esApiMacro(FogCoordfv), \
    esApiMacro(FogCoordd), \
    esApiMacro(FogCoorddv), \
    esApiMacro(FogCoordPointer), \
    esApiMacro(MultiDrawArraysEXT), \
    esApiMacro(MultiDrawElementsEXT), \
    esApiMacro(PointParameterf), \
    esApiMacro(PointParameterfv), \
    esApiMacro(PointParameteri), \
    esApiMacro(PointParameteriv), \
    esApiMacro(SecondaryColor3b), \
    esApiMacro(SecondaryColor3bv), \
    esApiMacro(SecondaryColor3d), \
    esApiMacro(SecondaryColor3dv), \
    esApiMacro(SecondaryColor3f), \
    esApiMacro(SecondaryColor3fv), \
    esApiMacro(SecondaryColor3i), \
    esApiMacro(SecondaryColor3iv), \
    esApiMacro(SecondaryColor3s), \
    esApiMacro(SecondaryColor3sv), \
    esApiMacro(SecondaryColor3ub), \
    esApiMacro(SecondaryColor3ubv), \
    esApiMacro(SecondaryColor3ui), \
    esApiMacro(SecondaryColor3uiv), \
    esApiMacro(SecondaryColor3us), \
    esApiMacro(SecondaryColor3usv), \
    esApiMacro(SecondaryColorPointer), \
    esApiMacro(WindowPos2d), \
    esApiMacro(WindowPos2dv), \
    esApiMacro(WindowPos2f), \
    esApiMacro(WindowPos2fv), \
    esApiMacro(WindowPos2i), \
    esApiMacro(WindowPos2iv), \
    esApiMacro(WindowPos2s), \
    esApiMacro(WindowPos2sv), \
    esApiMacro(WindowPos3d), \
    esApiMacro(WindowPos3dv), \
    esApiMacro(WindowPos3f), \
    esApiMacro(WindowPos3fv), \
    esApiMacro(WindowPos3i), \
    esApiMacro(WindowPos3iv), \
    esApiMacro(WindowPos3s), \
    esApiMacro(WindowPos3sv), \
    esApiMacro(GenQueries), \
    esApiMacro(DeleteQueries), \
    esApiMacro(IsQuery), \
    esApiMacro(BeginQuery), \
    esApiMacro(EndQuery), \
    esApiMacro(GetQueryiv), \
    esApiMacro(GetQueryObjectiv), \
    esApiMacro(GetQueryObjectuiv), \
    esApiMacro(BindBuffer), \
    esApiMacro(DeleteBuffers), \
    esApiMacro(GenBuffers), \
    esApiMacro(IsBuffer), \
    esApiMacro(BufferData), \
    esApiMacro(BufferSubData), \
    esApiMacro(GetBufferSubData),\
    esApiMacro(MapBufferOES), \
    esApiMacro(UnmapBufferOES), \
    esApiMacro(MapBuffer), \
    esApiMacro(GetBufferParameteriv), \
    esApiMacro(GetBufferPointerv), \
    esApiMacro(BlendEquationSeparate), \
    esApiMacro(DrawBuffers), \
    esApiMacro(StencilOpSeparate), \
    esApiMacro(StencilFuncSeparate), \
    esApiMacro(StencilMaskSeparate), \
    esApiMacro(AttachShader), \
    esApiMacro(BindAttribLocation), \
    esApiMacro(CompileShader), \
    esApiMacro(CreateProgram), \
    esApiMacro(CreateShader), \
    esApiMacro(DeleteProgram), \
    esApiMacro(DeleteShader), \
    esApiMacro(DetachShader), \
    esApiMacro(DisableVertexAttribArray), \
    esApiMacro(EnableVertexAttribArray), \
    esApiMacro(GetActiveAttrib), \
    esApiMacro(GetActiveUniform), \
    esApiMacro(GetAttachedShaders), \
    esApiMacro(GetAttribLocation), \
    esApiMacro(GetProgramiv), \
    esApiMacro(GetProgramInfoLog), \
    esApiMacro(GetShaderiv), \
    esApiMacro(GetShaderInfoLog), \
    esApiMacro(GetShaderSource), \
    esApiMacro(GetUniformLocation), \
    esApiMacro(GetUniformfv), \
    esApiMacro(GetUniformiv), \
    esApiMacro(GetVertexAttribdv), \
    esApiMacro(GetVertexAttribfv), \
    esApiMacro(GetVertexAttribiv), \
    esApiMacro(GetVertexAttribPointerv), \
    esApiMacro(IsProgram), \
    esApiMacro(IsShader), \
    esApiMacro(LinkProgram), \
    esApiMacro(ShaderSource), \
    esApiMacro(UseProgram), \
    esApiMacro(Uniform1f), \
    esApiMacro(Uniform1fv), \
    esApiMacro(Uniform1i), \
    esApiMacro(Uniform1iv), \
    esApiMacro(Uniform2f), \
    esApiMacro(Uniform2fv), \
    esApiMacro(Uniform2i), \
    esApiMacro(Uniform2iv), \
    esApiMacro(Uniform3f), \
    esApiMacro(Uniform3fv), \
    esApiMacro(Uniform3i), \
    esApiMacro(Uniform3iv), \
    esApiMacro(Uniform4f), \
    esApiMacro(Uniform4fv), \
    esApiMacro(Uniform4i), \
    esApiMacro(Uniform4iv), \
    esApiMacro(UniformMatrix2fv), \
    esApiMacro(UniformMatrix3fv), \
    esApiMacro(UniformMatrix4fv), \
    esApiMacro(ValidateProgram), \
    esApiMacro(VertexAttrib1d), \
    esApiMacro(VertexAttrib1dv), \
    esApiMacro(VertexAttrib1f), \
    esApiMacro(VertexAttrib1fv), \
    esApiMacro(VertexAttrib1s), \
    esApiMacro(VertexAttrib1sv), \
    esApiMacro(VertexAttrib2d), \
    esApiMacro(VertexAttrib2dv), \
    esApiMacro(VertexAttrib2f), \
    esApiMacro(VertexAttrib2fv), \
    esApiMacro(VertexAttrib2s), \
    esApiMacro(VertexAttrib2sv), \
    esApiMacro(VertexAttrib3d), \
    esApiMacro(VertexAttrib3dv), \
    esApiMacro(VertexAttrib3f), \
    esApiMacro(VertexAttrib3fv), \
    esApiMacro(VertexAttrib3s), \
    esApiMacro(VertexAttrib3sv), \
    esApiMacro(VertexAttrib4Nbv), \
    esApiMacro(VertexAttrib4Niv), \
    esApiMacro(VertexAttrib4Nsv), \
    esApiMacro(VertexAttrib4Nub), \
    esApiMacro(VertexAttrib4Nubv), \
    esApiMacro(VertexAttrib4Nuiv), \
    esApiMacro(VertexAttrib4Nusv), \
    esApiMacro(VertexAttrib4bv), \
    esApiMacro(VertexAttrib4d), \
    esApiMacro(VertexAttrib4dv), \
    esApiMacro(VertexAttrib4f), \
    esApiMacro(VertexAttrib4fv), \
    esApiMacro(VertexAttrib4iv), \
    esApiMacro(VertexAttrib4s), \
    esApiMacro(VertexAttrib4sv), \
    esApiMacro(VertexAttrib4ubv), \
    esApiMacro(VertexAttrib4uiv), \
    esApiMacro(VertexAttrib4usv), \
    esApiMacro(VertexAttribPointer), \
    esApiMacro(BindFramebuffer), \
    esApiMacro(BindRenderbuffer), \
    esApiMacro(CheckFramebufferStatus), \
    esApiMacro(ClearDepthf), \
    esApiMacro(DeleteFramebuffers), \
    esApiMacro(DeleteRenderbuffers), \
    esApiMacro(DepthRangef), \
    esApiMacro(FramebufferRenderbuffer), \
    esApiMacro(FramebufferTexture2D), \
    esApiMacro(GenerateMipmap), \
    esApiMacro(GenFramebuffers), \
    esApiMacro(GenRenderbuffers), \
    esApiMacro(GetFramebufferAttachmentParameteriv), \
    esApiMacro(GetRenderbufferParameteriv), \
    esApiMacro(GetShaderPrecisionFormat), \
    esApiMacro(IsFramebuffer), \
    esApiMacro(IsRenderbuffer), \
    esApiMacro(ReleaseShaderCompiler), \
    esApiMacro(RenderbufferStorage), \
    esApiMacro(ShaderBinary), \
    /* OpenGL ES 3.0 */ \
/*    esApiMacro(ReadBuffer),*/ \
    esApiMacro(UnmapBuffer), \
    esApiMacro(UniformMatrix2x3fv), \
    esApiMacro(UniformMatrix3x2fv), \
    esApiMacro(UniformMatrix2x4fv), \
    esApiMacro(UniformMatrix4x2fv), \
    esApiMacro(UniformMatrix3x4fv), \
    esApiMacro(UniformMatrix4x3fv), \
    esApiMacro(BlitFramebuffer), \
    esApiMacro(RenderbufferStorageMultisample), \
    esApiMacro(FramebufferTextureLayer), \
    esApiMacro(MapBufferRange), \
    esApiMacro(FlushMappedBufferRange), \
    esApiMacro(BindVertexArray), \
    esApiMacro(DeleteVertexArrays), \
    esApiMacro(GenVertexArrays), \
    esApiMacro(IsVertexArray), \
    esApiMacro(GetIntegeri_v), \
    esApiMacro(BeginTransformFeedback), \
    esApiMacro(EndTransformFeedback), \
    esApiMacro(BindBufferRange), \
    esApiMacro(BindBufferBase), \
    esApiMacro(TransformFeedbackVaryings), \
    esApiMacro(GetTransformFeedbackVarying), \
    esApiMacro(VertexAttribIPointer), \
    esApiMacro(GetVertexAttribIiv), \
    esApiMacro(GetVertexAttribIuiv), \
    esApiMacro(VertexAttribI4i), \
    esApiMacro(VertexAttribI4ui), \
    esApiMacro(VertexAttribI4iv), \
    esApiMacro(VertexAttribI4uiv), \
    esApiMacro(GetUniformuiv), \
    esApiMacro(GetFragDataLocation), \
    esApiMacro(BindFragDataLocation), \
    esApiMacro(Uniform1ui), \
    esApiMacro(Uniform2ui), \
    esApiMacro(Uniform3ui), \
    esApiMacro(Uniform4ui), \
    esApiMacro(Uniform1uiv), \
    esApiMacro(Uniform2uiv), \
    esApiMacro(Uniform3uiv), \
    esApiMacro(Uniform4uiv), \
    esApiMacro(ClearBufferiv), \
    esApiMacro(ClearBufferuiv), \
    esApiMacro(ClearBufferfv), \
    esApiMacro(ClearBufferfi), \
    esApiMacro(GetStringi), \
    esApiMacro(CopyBufferSubData), \
    esApiMacro(GetUniformIndices), \
    esApiMacro(GetActiveUniformsiv), \
    esApiMacro(GetUniformBlockIndex), \
    esApiMacro(GetActiveUniformBlockiv), \
    esApiMacro(GetActiveUniformBlockName), \
    esApiMacro(UniformBlockBinding), \
    esApiMacro(DrawArraysInstanced), \
    esApiMacro(DrawElementsInstanced), \
    esApiMacro(FenceSync), \
    esApiMacro(IsSync), \
    esApiMacro(DeleteSync), \
    esApiMacro(ClientWaitSync), \
    esApiMacro(WaitSync), \
    esApiMacro(GetInteger64v), \
    esApiMacro(GetSynciv), \
    esApiMacro(GetInteger64i_v), \
    esApiMacro(GetBufferParameteri64v), \
    esApiMacro(GenSamplers), \
    esApiMacro(DeleteSamplers), \
    esApiMacro(IsSampler), \
    esApiMacro(BindSampler), \
    esApiMacro(SamplerParameteri), \
    esApiMacro(SamplerParameteriv), \
    esApiMacro(SamplerParameterf), \
    esApiMacro(SamplerParameterfv), \
    esApiMacro(GetSamplerParameteriv), \
    esApiMacro(GetSamplerParameterfv), \
    esApiMacro(VertexAttribDivisor), \
    esApiMacro(BindTransformFeedback), \
    esApiMacro(DeleteTransformFeedbacks), \
    esApiMacro(GenTransformFeedbacks), \
    esApiMacro(IsTransformFeedback), \
    esApiMacro(PauseTransformFeedback), \
    esApiMacro(ResumeTransformFeedback), \
    esApiMacro(GetProgramBinary), \
    esApiMacro(ProgramBinary), \
    esApiMacro(ProgramParameteri), \
    esApiMacro(InvalidateFramebuffer), \
    esApiMacro(InvalidateSubFramebuffer), \
    esApiMacro(TexStorage2D), \
    esApiMacro(TexStorage3D), \
    esApiMacro(GetInternalformativ), \
    /* OpenGL ES 3.1 */ \
    esApiMacro(DispatchCompute), \
    esApiMacro(DispatchComputeIndirect), \
    esApiMacro(DrawArraysIndirect), \
    esApiMacro(DrawElementsIndirect), \
    esApiMacro(FramebufferParameteri), \
    esApiMacro(GetFramebufferParameteriv), \
    esApiMacro(GetProgramInterfaceiv), \
    esApiMacro(GetProgramResourceIndex), \
    esApiMacro(GetProgramResourceName), \
    esApiMacro(GetProgramResourceiv), \
    esApiMacro(GetProgramResourceLocation), \
    esApiMacro(UseProgramStages), \
    esApiMacro(ActiveShaderProgram), \
    esApiMacro(CreateShaderProgramv), \
    esApiMacro(BindProgramPipeline), \
    esApiMacro(DeleteProgramPipelines), \
    esApiMacro(GenProgramPipelines), \
    esApiMacro(IsProgramPipeline), \
    esApiMacro(GetProgramPipelineiv), \
    esApiMacro(ProgramUniform1i), \
    esApiMacro(ProgramUniform2i), \
    esApiMacro(ProgramUniform3i), \
    esApiMacro(ProgramUniform4i), \
    esApiMacro(ProgramUniform1ui), \
    esApiMacro(ProgramUniform2ui), \
    esApiMacro(ProgramUniform3ui), \
    esApiMacro(ProgramUniform4ui), \
    esApiMacro(ProgramUniform1f), \
    esApiMacro(ProgramUniform2f), \
    esApiMacro(ProgramUniform3f), \
    esApiMacro(ProgramUniform4f), \
    esApiMacro(ProgramUniform1iv), \
    esApiMacro(ProgramUniform2iv), \
    esApiMacro(ProgramUniform3iv), \
    esApiMacro(ProgramUniform4iv), \
    esApiMacro(ProgramUniform1uiv), \
    esApiMacro(ProgramUniform2uiv), \
    esApiMacro(ProgramUniform3uiv), \
    esApiMacro(ProgramUniform4uiv), \
    esApiMacro(ProgramUniform1fv), \
    esApiMacro(ProgramUniform2fv), \
    esApiMacro(ProgramUniform3fv), \
    esApiMacro(ProgramUniform4fv), \
    esApiMacro(ProgramUniformMatrix2fv), \
    esApiMacro(ProgramUniformMatrix3fv), \
    esApiMacro(ProgramUniformMatrix4fv), \
    esApiMacro(ProgramUniformMatrix2x3fv), \
    esApiMacro(ProgramUniformMatrix3x2fv), \
    esApiMacro(ProgramUniformMatrix2x4fv), \
    esApiMacro(ProgramUniformMatrix4x2fv), \
    esApiMacro(ProgramUniformMatrix3x4fv), \
    esApiMacro(ProgramUniformMatrix4x3fv), \
    esApiMacro(ValidateProgramPipeline), \
    esApiMacro(GetProgramPipelineInfoLog), \
    esApiMacro(BindImageTexture), \
    esApiMacro(GetBooleani_v), \
    esApiMacro(MemoryBarrier), \
    esApiMacro(MemoryBarrierByRegion), \
    esApiMacro(TexStorage2DMultisample), \
    esApiMacro(GetMultisamplefv), \
    esApiMacro(SampleMaski), \
    esApiMacro(BindVertexBuffer), \
    esApiMacro(VertexAttribFormat), \
    esApiMacro(VertexAttribIFormat), \
    esApiMacro(VertexAttribBinding), \
    esApiMacro(VertexBindingDivisor), \
    /* OpenGL ES 3.2 */ \
    esApiMacro(TexStorage3DMultisample), \
    esApiMacro(BlendBarrier), \
    esApiMacro(DebugMessageControl), \
    esApiMacro(DebugMessageInsert), \
    esApiMacro(DebugMessageCallback), \
    esApiMacro(GetDebugMessageLog), \
    esApiMacro(PushDebugGroup), \
    esApiMacro(PopDebugGroup), \
    esApiMacro(ObjectLabel), \
    esApiMacro(GetObjectLabel), \
    esApiMacro(ObjectPtrLabel), \
    esApiMacro(GetObjectPtrLabel), \
    esApiMacro(GetGraphicsResetStatus), \
    esApiMacro(ReadnPixels), \
    esApiMacro(GetnUniformfv), \
    esApiMacro(GetnUniformiv), \
    esApiMacro(GetnUniformuiv), \
    esApiMacro(BlendEquationi), \
    esApiMacro(BlendEquationSeparatei), \
    esApiMacro(BlendFunci), \
    esApiMacro(BlendFuncSeparatei), \
    esApiMacro(ColorMaski), \
    esApiMacro(Enablei),  \
    esApiMacro(Disablei),\
    esApiMacro(IsEnabledi), \
    esApiMacro(TexParameterIiv),  \
    esApiMacro(TexParameterIuiv),  \
    esApiMacro(GetTexParameterIiv), \
    esApiMacro(GetTexParameterIuiv),  \
    esApiMacro(SamplerParameterIiv),  \
    esApiMacro(SamplerParameterIuiv),  \
    esApiMacro(GetSamplerParameterIiv),  \
    esApiMacro(GetSamplerParameterIuiv), \
    esApiMacro(TexBuffer), \
    esApiMacro(TexBufferRange), \
    esApiMacro(PatchParameteri), \
    esApiMacro(FramebufferTexture), \
    esApiMacro(MinSampleShading), \
    esApiMacro(CopyImageSubData), \
    esApiMacro(DrawElementsBaseVertex), \
    esApiMacro(DrawRangeElementsBaseVertex), \
    esApiMacro(DrawElementsInstancedBaseVertex), \
    esApiMacro(PrimitiveBoundingBox), \
    /* OpenGL ES extensions */ \
    /* GL_OES_EGL_image */ \
    esApiMacro(EGLImageTargetTexture2DOES), \
    esApiMacro(EGLImageTargetRenderbufferStorageOES), \
    /* GL_EXT_multi_draw_arrays */ \
    esApiMacro(MultiDrawElementsBaseVertexEXT), \
    /* GL_OES_mapbuffer */ \
    esApiMacro(GetBufferPointervOES), \
    /* GL_EXT_discard_framebuffer */ \
    esApiMacro(DiscardFramebufferEXT), \
    /* GL_EXT_multisampled_render_to_texture */ \
    esApiMacro(RenderbufferStorageMultisampleEXT), \
    esApiMacro(FramebufferTexture2DMultisampleEXT), \
    /* GL_VIV_direct_texture */ \
    esApiMacro(TexDirectVIV), \
    esApiMacro(TexDirectInvalidateVIV), \
    esApiMacro(TexDirectVIVMap), \
    esApiMacro(TexDirectTiledMapVIV), \
    /* GL_EXT_multi_draw_indirect */ \
    esApiMacro(MultiDrawArraysIndirectEXT), \
    esApiMacro(MultiDrawElementsIndirectEXT), \
    esApiMacro(DeleteObjectARB), \
    esApiMacro(GetInfoLogARB), \
    esApiMacro(GetObjectParameterivARB), \
    esApiMacro(ClampColorARB), \
    esApiMacro(IsRenderbufferEXT), \
    esApiMacro(BindRenderbufferEXT), \
    esApiMacro(DeleteRenderbuffersEXT), \
    esApiMacro(GenRenderbuffersEXT), \
    esApiMacro(RenderbufferStorageEXT), \
    esApiMacro(GetRenderbufferParameterivEXT), \
    esApiMacro(IsFramebufferEXT), \
    esApiMacro(BindFramebufferEXT), \
    esApiMacro(DeleteFramebuffersEXT), \
    esApiMacro(GenFramebuffersEXT), \
    esApiMacro(CheckFramebufferStatusEXT), \
    esApiMacro(FramebufferTexture1DEXT), \
    esApiMacro(FramebufferTexture2DEXT), \
    esApiMacro(FramebufferTexture3DEXT), \
    esApiMacro(FramebufferRenderbufferEXT), \
    esApiMacro(GetFramebufferAttachmentParameterivEXT), \
    esApiMacro(GenerateMipmapEXT), \
    esApiMacro(BlitFramebufferEXT), \
    esApiMacro(ClampColor), \
    esApiMacro(BeginConditionalRender), \
    esApiMacro(EndConditionalRender), \
    esApiMacro(VertexAttribI1i), \
    esApiMacro(VertexAttribI2i), \
    esApiMacro(VertexAttribI3i), \
    esApiMacro(VertexAttribI1ui), \
    esApiMacro(VertexAttribI2ui), \
    esApiMacro(VertexAttribI3ui), \
    esApiMacro(VertexAttribI1iv), \
    esApiMacro(VertexAttribI2iv), \
    esApiMacro(VertexAttribI3iv), \
    esApiMacro(VertexAttribI1uiv), \
    esApiMacro(VertexAttribI2uiv), \
    esApiMacro(VertexAttribI3uiv), \
    esApiMacro(VertexAttribI4bv), \
    esApiMacro(VertexAttribI4sv), \
    esApiMacro(VertexAttribI4ubv), \
    esApiMacro(VertexAttribI4usv), \
    esApiMacro(FramebufferTexture1D), \
    esApiMacro(FramebufferTexture3D), \
    esApiMacro(PrimitiveRestartIndex), \
    esApiMacro(GetActiveUniformName), \
    esApiMacro(GetUniformdv), \
    esApiMacro(Uniform1d), \
    esApiMacro(Uniform2d), \
    esApiMacro(Uniform3d), \
    esApiMacro(Uniform4d), \
    esApiMacro(Uniform1dv), \
    esApiMacro(Uniform2dv), \
    esApiMacro(Uniform3dv), \
    esApiMacro(Uniform4dv), \
    esApiMacro(UniformMatrix2dv), \
    esApiMacro(UniformMatrix3dv), \
    esApiMacro(UniformMatrix4dv), \
    esApiMacro(UniformMatrix2x3dv), \
    esApiMacro(UniformMatrix3x2dv), \
    esApiMacro(UniformMatrix2x4dv), \
    esApiMacro(UniformMatrix4x2dv), \
    esApiMacro(UniformMatrix3x4dv), \
    esApiMacro(UniformMatrix4x3dv), \



#define __GL_LISTEXEC_ENTRIES(esApiMacro, _initstr_) \
    esApiMacro(Skip) _initstr_, \
    esApiMacro(InvalidValue), \
    esApiMacro(InvalidEnum), \
    esApiMacro(InvalidOperation), \
    esApiMacro(TableTooLarge), \
    esApiMacro(Primitive), \
    esApiMacro(NewList), \
    esApiMacro(EndList), \
    esApiMacro(CallList), \
    esApiMacro(CallLists), \
    esApiMacro(DeleteLists), \
    esApiMacro(GenLists), \
    esApiMacro(ListBase), \
    esApiMacro(Begin), \
    esApiMacro(Bitmap), \
    esApiMacro(Color3b), \
    esApiMacro(Color3bv), \
    esApiMacro(Color3d), \
    esApiMacro(Color3dv), \
    esApiMacro(Color3f), \
    esApiMacro(Color3fv), \
    esApiMacro(Color3i), \
    esApiMacro(Color3iv), \
    esApiMacro(Color3s), \
    esApiMacro(Color3sv), \
    esApiMacro(Color3ub), \
    esApiMacro(Color3ubv), \
    esApiMacro(Color3ui), \
    esApiMacro(Color3uiv), \
    esApiMacro(Color3us), \
    esApiMacro(Color3usv), \
    esApiMacro(Color4b), \
    esApiMacro(Color4bv), \
    esApiMacro(Color4d), \
    esApiMacro(Color4dv), \
    esApiMacro(Color4f), \
    esApiMacro(Color4fv), \
    esApiMacro(Color4i), \
    esApiMacro(Color4iv), \
    esApiMacro(Color4s), \
    esApiMacro(Color4sv), \
    esApiMacro(Color4ub), \
    esApiMacro(Color4ubv), \
    esApiMacro(Color4ui), \
    esApiMacro(Color4uiv), \
    esApiMacro(Color4us), \
    esApiMacro(Color4usv), \
    esApiMacro(EdgeFlag), \
    esApiMacro(EdgeFlagv), \
    esApiMacro(End), \
    esApiMacro(Indexd), \
    esApiMacro(Indexdv), \
    esApiMacro(Indexf), \
    esApiMacro(Indexfv), \
    esApiMacro(Indexi), \
    esApiMacro(Indexiv), \
    esApiMacro(Indexs), \
    esApiMacro(Indexsv), \
    esApiMacro(Normal3b), \
    esApiMacro(Normal3bv), \
    esApiMacro(Normal3d), \
    esApiMacro(Normal3dv), \
    esApiMacro(Normal3f), \
    esApiMacro(Normal3fv), \
    esApiMacro(Normal3i), \
    esApiMacro(Normal3iv), \
    esApiMacro(Normal3s), \
    esApiMacro(Normal3sv), \
    esApiMacro(RasterPos2d), \
    esApiMacro(RasterPos2dv), \
    esApiMacro(RasterPos2f), \
    esApiMacro(RasterPos2fv), \
    esApiMacro(RasterPos2i), \
    esApiMacro(RasterPos2iv), \
    esApiMacro(RasterPos2s), \
    esApiMacro(RasterPos2sv), \
    esApiMacro(RasterPos3d), \
    esApiMacro(RasterPos3dv), \
    esApiMacro(RasterPos3f), \
    esApiMacro(RasterPos3fv), \
    esApiMacro(RasterPos3i), \
    esApiMacro(RasterPos3iv), \
    esApiMacro(RasterPos3s), \
    esApiMacro(RasterPos3sv), \
    esApiMacro(RasterPos4d), \
    esApiMacro(RasterPos4dv), \
    esApiMacro(RasterPos4f), \
    esApiMacro(RasterPos4fv), \
    esApiMacro(RasterPos4i), \
    esApiMacro(RasterPos4iv), \
    esApiMacro(RasterPos4s), \
    esApiMacro(RasterPos4sv), \
    esApiMacro(Rectd), \
    esApiMacro(Rectdv), \
    esApiMacro(Rectf), \
    esApiMacro(Rectfv), \
    esApiMacro(Recti), \
    esApiMacro(Rectiv), \
    esApiMacro(Rects), \
    esApiMacro(Rectsv), \
    esApiMacro(TexCoord1d), \
    esApiMacro(TexCoord1dv), \
    esApiMacro(TexCoord1f), \
    esApiMacro(TexCoord1fv), \
    esApiMacro(TexCoord1i), \
    esApiMacro(TexCoord1iv), \
    esApiMacro(TexCoord1s), \
    esApiMacro(TexCoord1sv), \
    esApiMacro(TexCoord2d), \
    esApiMacro(TexCoord2dv), \
    esApiMacro(TexCoord2f), \
    esApiMacro(TexCoord2fv), \
    esApiMacro(TexCoord2i), \
    esApiMacro(TexCoord2iv), \
    esApiMacro(TexCoord2s), \
    esApiMacro(TexCoord2sv), \
    esApiMacro(TexCoord3d), \
    esApiMacro(TexCoord3dv), \
    esApiMacro(TexCoord3f), \
    esApiMacro(TexCoord3fv), \
    esApiMacro(TexCoord3i), \
    esApiMacro(TexCoord3iv), \
    esApiMacro(TexCoord3s), \
    esApiMacro(TexCoord3sv), \
    esApiMacro(TexCoord4d), \
    esApiMacro(TexCoord4dv), \
    esApiMacro(TexCoord4f), \
    esApiMacro(TexCoord4fv), \
    esApiMacro(TexCoord4i), \
    esApiMacro(TexCoord4iv), \
    esApiMacro(TexCoord4s), \
    esApiMacro(TexCoord4sv), \
    esApiMacro(Vertex2d), \
    esApiMacro(Vertex2dv), \
    esApiMacro(Vertex2f), \
    esApiMacro(Vertex2fv), \
    esApiMacro(Vertex2i), \
    esApiMacro(Vertex2iv), \
    esApiMacro(Vertex2s), \
    esApiMacro(Vertex2sv), \
    esApiMacro(Vertex3d), \
    esApiMacro(Vertex3dv), \
    esApiMacro(Vertex3f), \
    esApiMacro(Vertex3fv), \
    esApiMacro(Vertex3i), \
    esApiMacro(Vertex3iv), \
    esApiMacro(Vertex3s), \
    esApiMacro(Vertex3sv), \
    esApiMacro(Vertex4d), \
    esApiMacro(Vertex4dv), \
    esApiMacro(Vertex4f), \
    esApiMacro(Vertex4fv), \
    esApiMacro(Vertex4i), \
    esApiMacro(Vertex4iv), \
    esApiMacro(Vertex4s), \
    esApiMacro(Vertex4sv), \
    esApiMacro(ClipPlane), \
    esApiMacro(ColorMaterial), \
    esApiMacro(CullFace), \
    esApiMacro(Fogf), \
    esApiMacro(Fogfv), \
    esApiMacro(Fogi), \
    esApiMacro(Fogiv), \
    esApiMacro(FrontFace), \
    esApiMacro(Hint), \
    esApiMacro(Lightf), \
    esApiMacro(Lightfv), \
    esApiMacro(Lighti), \
    esApiMacro(Lightiv), \
    esApiMacro(LightModelf), \
    esApiMacro(LightModelfv), \
    esApiMacro(LightModeli), \
    esApiMacro(LightModeliv), \
    esApiMacro(LineStipple), \
    esApiMacro(LineWidth), \
    esApiMacro(Materialf), \
    esApiMacro(Materialfv), \
    esApiMacro(Materiali), \
    esApiMacro(Materialiv), \
    esApiMacro(PointSize), \
    esApiMacro(PolygonMode), \
    esApiMacro(PolygonStipple), \
    esApiMacro(Scissor), \
    esApiMacro(ShadeModel), \
    esApiMacro(TexParameterf), \
    esApiMacro(TexParameterfv), \
    esApiMacro(TexParameteri), \
    esApiMacro(TexParameteriv), \
    esApiMacro(TexImage1D), \
    esApiMacro(TexImage2D), \
    esApiMacro(TexEnvf), \
    esApiMacro(TexEnvfv), \
    esApiMacro(TexEnvi), \
    esApiMacro(TexEnviv), \
    esApiMacro(TexGend), \
    esApiMacro(TexGendv), \
    esApiMacro(TexGenf), \
    esApiMacro(TexGenfv), \
    esApiMacro(TexGeni), \
    esApiMacro(TexGeniv), \
    esApiMacro(FeedbackBuffer), \
    esApiMacro(SelectBuffer), \
    esApiMacro(RenderMode), \
    esApiMacro(InitNames), \
    esApiMacro(LoadName), \
    esApiMacro(PassThrough), \
    esApiMacro(PopName), \
    esApiMacro(PushName), \
    esApiMacro(DrawBuffer), \
    esApiMacro(Clear), \
    esApiMacro(ClearAccum), \
    esApiMacro(ClearIndex), \
    esApiMacro(ClearColor), \
    esApiMacro(ClearStencil), \
    esApiMacro(ClearDepth), \
    esApiMacro(StencilMask), \
    esApiMacro(ColorMask), \
    esApiMacro(DepthMask), \
    esApiMacro(IndexMask), \
    esApiMacro(Accum), \
    esApiMacro(Disable), \
    esApiMacro(Enable), \
    esApiMacro(Finish), \
    esApiMacro(Flush), \
    esApiMacro(PopAttrib), \
    esApiMacro(PushAttrib), \
    esApiMacro(Map1d), \
    esApiMacro(Map1f), \
    esApiMacro(Map2d), \
    esApiMacro(Map2f), \
    esApiMacro(MapGrid1d), \
    esApiMacro(MapGrid1f), \
    esApiMacro(MapGrid2d), \
    esApiMacro(MapGrid2f), \
    esApiMacro(EvalCoord1d), \
    esApiMacro(EvalCoord1dv), \
    esApiMacro(EvalCoord1f), \
    esApiMacro(EvalCoord1fv), \
    esApiMacro(EvalCoord2d), \
    esApiMacro(EvalCoord2dv), \
    esApiMacro(EvalCoord2f), \
    esApiMacro(EvalCoord2fv), \
    esApiMacro(EvalMesh1), \
    esApiMacro(EvalPoint1), \
    esApiMacro(EvalMesh2), \
    esApiMacro(EvalPoint2), \
    esApiMacro(AlphaFunc), \
    esApiMacro(BlendFunc), \
    esApiMacro(LogicOp), \
    esApiMacro(StencilFunc), \
    esApiMacro(StencilOp), \
    esApiMacro(DepthFunc), \
    esApiMacro(PixelZoom), \
    esApiMacro(PixelTransferf), \
    esApiMacro(PixelTransferi), \
    esApiMacro(PixelStoref), \
    esApiMacro(PixelStorei), \
    esApiMacro(PixelMapfv), \
    esApiMacro(PixelMapuiv), \
    esApiMacro(PixelMapusv), \
    esApiMacro(ReadBuffer), \
    esApiMacro(CopyPixels), \
    esApiMacro(ReadPixels), \
    esApiMacro(DrawPixels), \
    esApiMacro(GetBooleanv), \
    esApiMacro(GetClipPlane), \
    esApiMacro(GetDoublev), \
    esApiMacro(GetError), \
    esApiMacro(GetFloatv), \
    esApiMacro(GetIntegerv), \
    esApiMacro(GetLightfv), \
    esApiMacro(GetLightiv), \
    esApiMacro(GetMapdv), \
    esApiMacro(GetMapfv), \
    esApiMacro(GetMapiv), \
    esApiMacro(GetMaterialfv), \
    esApiMacro(GetMaterialiv), \
    esApiMacro(GetPixelMapfv), \
    esApiMacro(GetPixelMapuiv), \
    esApiMacro(GetPixelMapusv), \
    esApiMacro(GetPolygonStipple), \
    esApiMacro(GetString), \
    esApiMacro(GetTexEnvfv), \
    esApiMacro(GetTexEnviv), \
    esApiMacro(GetTexGendv), \
    esApiMacro(GetTexGenfv), \
    esApiMacro(GetTexGeniv), \
    esApiMacro(GetTexImage), \
    esApiMacro(GetTexParameterfv), \
    esApiMacro(GetTexParameteriv), \
    esApiMacro(GetTexLevelParameterfv), \
    esApiMacro(GetTexLevelParameteriv), \
    esApiMacro(IsEnabled), \
    esApiMacro(IsList), \
    esApiMacro(DepthRange), \
    esApiMacro(Frustum), \
    esApiMacro(LoadIdentity), \
    esApiMacro(LoadMatrixf), \
    esApiMacro(LoadMatrixd), \
    esApiMacro(MatrixMode), \
    esApiMacro(MultMatrixf), \
    esApiMacro(MultMatrixd), \
    esApiMacro(Ortho), \
    esApiMacro(PopMatrix), \
    esApiMacro(PushMatrix), \
    esApiMacro(Rotated), \
    esApiMacro(Rotatef), \
    esApiMacro(Scaled), \
    esApiMacro(Scalef), \
    esApiMacro(Translated), \
    esApiMacro(Translatef), \
    esApiMacro(Viewport), \
    esApiMacro(ArrayElement), \
    esApiMacro(BindTexture), \
    esApiMacro(ColorPointer), \
    esApiMacro(DisableClientState), \
    esApiMacro(DrawArrays), \
    esApiMacro(DrawElements), \
    esApiMacro(EdgeFlagPointer), \
    esApiMacro(EnableClientState), \
    esApiMacro(IndexPointer), \
    esApiMacro(Indexub), \
    esApiMacro(Indexubv), \
    esApiMacro(InterleavedArrays), \
    esApiMacro(NormalPointer), \
    esApiMacro(PolygonOffset), \
    esApiMacro(TexCoordPointer),\
    esApiMacro(VertexPointer), \
    esApiMacro(AreTexturesResident), \
    esApiMacro(CopyTexImage1D), \
    esApiMacro(CopyTexImage2D), \
    esApiMacro(CopyTexSubImage1D), \
    esApiMacro(CopyTexSubImage2D), \
    esApiMacro(DeleteTextures), \
    esApiMacro(GenTextures), \
    esApiMacro(GetPointerv), \
    esApiMacro(IsTexture), \
    esApiMacro(PrioritizeTextures), \
    esApiMacro(TexSubImage1D), \
    esApiMacro(TexSubImage2D), \
    esApiMacro(PopClientAttrib), \
    esApiMacro(PushClientAttrib), \
    esApiMacro(BlendColor), \
    esApiMacro(BlendEquation), \
    esApiMacro(DrawRangeElements), \
    esApiMacro(ColorTable), \
    esApiMacro(ColorTableParameterfv), \
    esApiMacro(ColorTableParameteriv), \
    esApiMacro(CopyColorTable), \
    esApiMacro(GetColorTable), \
    esApiMacro(GetColorTableParameterfv), \
    esApiMacro(GetColorTableParameteriv), \
    esApiMacro(ColorSubTable), \
    esApiMacro(CopyColorSubTable), \
    esApiMacro(ConvolutionFilter1D), \
    esApiMacro(ConvolutionFilter2D), \
    esApiMacro(ConvolutionParameterf), \
    esApiMacro(ConvolutionParameterfv), \
    esApiMacro(ConvolutionParameteri), \
    esApiMacro(ConvolutionParameteriv), \
    esApiMacro(CopyConvolutionFilter1D), \
    esApiMacro(CopyConvolutionFilter2D), \
    esApiMacro(GetConvolutionFilter), \
    esApiMacro(GetConvolutionParameterfv), \
    esApiMacro(GetConvolutionParameteriv), \
    esApiMacro(GetSeparableFilter), \
    esApiMacro(SeparableFilter2D), \
    esApiMacro(GetHistogram), \
    esApiMacro(GetHistogramParameterfv), \
    esApiMacro(GetHistogramParameteriv), \
    esApiMacro(GetMinmax), \
    esApiMacro(GetMinmaxParameterfv), \
    esApiMacro(GetMinmaxParameteriv), \
    esApiMacro(Histogram), \
    esApiMacro(Minmax), \
    esApiMacro(ResetHistogram), \
    esApiMacro(ResetMinmax), \
    esApiMacro(TexImage3D), \
    esApiMacro(TexSubImage3D), \
    esApiMacro(CopyTexSubImage3D), \
    esApiMacro(ActiveTexture), \
    esApiMacro(ClientActiveTexture), \
    esApiMacro(MultiTexCoord1d), \
    esApiMacro(MultiTexCoord1dv), \
    esApiMacro(MultiTexCoord1f), \
    esApiMacro(MultiTexCoord1fv), \
    esApiMacro(MultiTexCoord1i), \
    esApiMacro(MultiTexCoord1iv), \
    esApiMacro(MultiTexCoord1s), \
    esApiMacro(MultiTexCoord1sv), \
    esApiMacro(MultiTexCoord2d), \
    esApiMacro(MultiTexCoord2dv), \
    esApiMacro(MultiTexCoord2f), \
    esApiMacro(MultiTexCoord2fv), \
    esApiMacro(MultiTexCoord2i), \
    esApiMacro(MultiTexCoord2iv), \
    esApiMacro(MultiTexCoord2s), \
    esApiMacro(MultiTexCoord2sv), \
    esApiMacro(MultiTexCoord3d), \
    esApiMacro(MultiTexCoord3dv), \
    esApiMacro(MultiTexCoord3f), \
    esApiMacro(MultiTexCoord3fv), \
    esApiMacro(MultiTexCoord3i), \
    esApiMacro(MultiTexCoord3iv), \
    esApiMacro(MultiTexCoord3s), \
    esApiMacro(MultiTexCoord3sv), \
    esApiMacro(MultiTexCoord4d), \
    esApiMacro(MultiTexCoord4dv), \
    esApiMacro(MultiTexCoord4f), \
    esApiMacro(MultiTexCoord4fv), \
    esApiMacro(MultiTexCoord4i), \
    esApiMacro(MultiTexCoord4iv), \
    esApiMacro(MultiTexCoord4s), \
    esApiMacro(MultiTexCoord4sv), \
    esApiMacro(LoadTransposeMatrixf), \
    esApiMacro(LoadTransposeMatrixd), \
    esApiMacro(MultTransposeMatrixf), \
    esApiMacro(MultTransposeMatrixd), \
    esApiMacro(SampleCoverage), \
    esApiMacro(CompressedTexImage3D), \
    esApiMacro(CompressedTexImage2D), \
    esApiMacro(CompressedTexImage1D), \
    esApiMacro(CompressedTexSubImage3D), \
    esApiMacro(CompressedTexSubImage2D), \
    esApiMacro(CompressedTexSubImage1D), \
    esApiMacro(GetCompressedTexImage), \
    esApiMacro(BlendFuncSeparate), \
    esApiMacro(MultiDrawArrays), \
    esApiMacro(MultiDrawElements), \
    esApiMacro(FogCoordf), \
    esApiMacro(FogCoordfv), \
    esApiMacro(FogCoordd), \
    esApiMacro(FogCoorddv), \
    esApiMacro(FogCoordPointer), \
    esApiMacro(MultiDrawArraysEXT), \
    esApiMacro(MultiDrawElementsEXT), \
    esApiMacro(PointParameterf), \
    esApiMacro(PointParameterfv), \
    esApiMacro(PointParameteri), \
    esApiMacro(PointParameteriv), \
    esApiMacro(SecondaryColor3b), \
    esApiMacro(SecondaryColor3bv), \
    esApiMacro(SecondaryColor3d), \
    esApiMacro(SecondaryColor3dv), \
    esApiMacro(SecondaryColor3f), \
    esApiMacro(SecondaryColor3fv), \
    esApiMacro(SecondaryColor3i), \
    esApiMacro(SecondaryColor3iv), \
    esApiMacro(SecondaryColor3s), \
    esApiMacro(SecondaryColor3sv), \
    esApiMacro(SecondaryColor3ub), \
    esApiMacro(SecondaryColor3ubv), \
    esApiMacro(SecondaryColor3ui), \
    esApiMacro(SecondaryColor3uiv), \
    esApiMacro(SecondaryColor3us), \
    esApiMacro(SecondaryColor3usv), \
    esApiMacro(SecondaryColorPointer), \
    esApiMacro(WindowPos2d), \
    esApiMacro(WindowPos2dv), \
    esApiMacro(WindowPos2f), \
    esApiMacro(WindowPos2fv), \
    esApiMacro(WindowPos2i), \
    esApiMacro(WindowPos2iv), \
    esApiMacro(WindowPos2s), \
    esApiMacro(WindowPos2sv), \
    esApiMacro(WindowPos3d), \
    esApiMacro(WindowPos3dv), \
    esApiMacro(WindowPos3f), \
    esApiMacro(WindowPos3fv), \
    esApiMacro(WindowPos3i), \
    esApiMacro(WindowPos3iv), \
    esApiMacro(WindowPos3s), \
    esApiMacro(WindowPos3sv), \
    esApiMacro(GenQueries), \
    esApiMacro(DeleteQueries), \
    esApiMacro(IsQuery), \
    esApiMacro(BeginQuery), \
    esApiMacro(EndQuery), \
    esApiMacro(GetQueryiv), \
    esApiMacro(GetQueryObjectiv), \
    esApiMacro(GetQueryObjectuiv), \
    esApiMacro(BindBuffer), \
    esApiMacro(DeleteBuffers), \
    esApiMacro(GenBuffers), \
    esApiMacro(IsBuffer), \
    esApiMacro(BufferData), \
    esApiMacro(BufferSubData), \
    esApiMacro(GetBufferSubData),\
    esApiMacro(MapBufferOES), \
    esApiMacro(UnmapBufferOES), \
    esApiMacro(MapBuffer), \
    esApiMacro(GetBufferParameteriv), \
    esApiMacro(GetBufferPointerv), \
    esApiMacro(BlendEquationSeparate), \
    esApiMacro(DrawBuffers), \
    esApiMacro(StencilOpSeparate), \
    esApiMacro(StencilFuncSeparate), \
    esApiMacro(StencilMaskSeparate), \
    esApiMacro(AttachShader), \
    esApiMacro(BindAttribLocation), \
    esApiMacro(CompileShader), \
    esApiMacro(CreateProgram), \
    esApiMacro(CreateShader), \
    esApiMacro(DeleteProgram), \
    esApiMacro(DeleteShader), \
    esApiMacro(DetachShader), \
    esApiMacro(DisableVertexAttribArray), \
    esApiMacro(EnableVertexAttribArray), \
    esApiMacro(GetActiveAttrib), \
    esApiMacro(GetActiveUniform), \
    esApiMacro(GetAttachedShaders), \
    esApiMacro(GetAttribLocation), \
    esApiMacro(GetProgramiv), \
    esApiMacro(GetProgramInfoLog), \
    esApiMacro(GetShaderiv), \
    esApiMacro(GetShaderInfoLog), \
    esApiMacro(GetShaderSource), \
    esApiMacro(GetUniformLocation), \
    esApiMacro(GetUniformfv), \
    esApiMacro(GetUniformiv), \
    esApiMacro(GetVertexAttribdv), \
    esApiMacro(GetVertexAttribfv), \
    esApiMacro(GetVertexAttribiv), \
    esApiMacro(GetVertexAttribPointerv), \
    esApiMacro(IsProgram), \
    esApiMacro(IsShader), \
    esApiMacro(LinkProgram), \
    esApiMacro(ShaderSource), \
    esApiMacro(UseProgram), \
    esApiMacro(Uniform1f), \
    esApiMacro(Uniform1fv), \
    esApiMacro(Uniform1i), \
    esApiMacro(Uniform1iv), \
    esApiMacro(Uniform2f), \
    esApiMacro(Uniform2fv), \
    esApiMacro(Uniform2i), \
    esApiMacro(Uniform2iv), \
    esApiMacro(Uniform3f), \
    esApiMacro(Uniform3fv), \
    esApiMacro(Uniform3i), \
    esApiMacro(Uniform3iv), \
    esApiMacro(Uniform4f), \
    esApiMacro(Uniform4fv), \
    esApiMacro(Uniform4i), \
    esApiMacro(Uniform4iv), \
    esApiMacro(UniformMatrix2fv), \
    esApiMacro(UniformMatrix3fv), \
    esApiMacro(UniformMatrix4fv), \
    esApiMacro(ValidateProgram), \
    esApiMacro(VertexAttrib1d), \
    esApiMacro(VertexAttrib1dv), \
    esApiMacro(VertexAttrib1f), \
    esApiMacro(VertexAttrib1fv), \
    esApiMacro(VertexAttrib1s), \
    esApiMacro(VertexAttrib1sv), \
    esApiMacro(VertexAttrib2d), \
    esApiMacro(VertexAttrib2dv), \
    esApiMacro(VertexAttrib2f), \
    esApiMacro(VertexAttrib2fv), \
    esApiMacro(VertexAttrib2s), \
    esApiMacro(VertexAttrib2sv), \
    esApiMacro(VertexAttrib3d), \
    esApiMacro(VertexAttrib3dv), \
    esApiMacro(VertexAttrib3f), \
    esApiMacro(VertexAttrib3fv), \
    esApiMacro(VertexAttrib3s), \
    esApiMacro(VertexAttrib3sv), \
    esApiMacro(VertexAttrib4Nbv), \
    esApiMacro(VertexAttrib4Niv), \
    esApiMacro(VertexAttrib4Nsv), \
    esApiMacro(VertexAttrib4Nub), \
    esApiMacro(VertexAttrib4Nubv), \
    esApiMacro(VertexAttrib4Nuiv), \
    esApiMacro(VertexAttrib4Nusv), \
    esApiMacro(VertexAttrib4bv), \
    esApiMacro(VertexAttrib4d), \
    esApiMacro(VertexAttrib4dv), \
    esApiMacro(VertexAttrib4f), \
    esApiMacro(VertexAttrib4fv), \
    esApiMacro(VertexAttrib4iv), \
    esApiMacro(VertexAttrib4s), \
    esApiMacro(VertexAttrib4sv), \
    esApiMacro(VertexAttrib4ubv), \
    esApiMacro(VertexAttrib4uiv), \
    esApiMacro(VertexAttrib4usv), \
    esApiMacro(VertexAttribPointer), \
    esApiMacro(BindFramebuffer), \
    esApiMacro(BindRenderbuffer), \
    esApiMacro(CheckFramebufferStatus), \
    esApiMacro(ClearDepthf), \
    esApiMacro(DeleteFramebuffers), \
    esApiMacro(DeleteRenderbuffers), \
    esApiMacro(DepthRangef), \
    esApiMacro(FramebufferRenderbuffer), \
    esApiMacro(FramebufferTexture2D), \
    esApiMacro(GenerateMipmap), \
    esApiMacro(GenFramebuffers), \
    esApiMacro(GenRenderbuffers), \
    esApiMacro(GetFramebufferAttachmentParameteriv), \
    esApiMacro(GetRenderbufferParameteriv), \
    esApiMacro(GetShaderPrecisionFormat), \
    esApiMacro(IsFramebuffer), \
    esApiMacro(IsRenderbuffer), \
    esApiMacro(ReleaseShaderCompiler), \
    esApiMacro(RenderbufferStorage), \
    esApiMacro(ShaderBinary), \
    /* OpenGL ES 3.0 */ \
/*    esApiMacro(ReadBuffer),*/ \
    esApiMacro(UnmapBuffer), \
    esApiMacro(UniformMatrix2x3fv), \
    esApiMacro(UniformMatrix3x2fv), \
    esApiMacro(UniformMatrix2x4fv), \
    esApiMacro(UniformMatrix4x2fv), \
    esApiMacro(UniformMatrix3x4fv), \
    esApiMacro(UniformMatrix4x3fv), \
    esApiMacro(BlitFramebuffer), \
    esApiMacro(RenderbufferStorageMultisample), \
    esApiMacro(FramebufferTextureLayer), \
    esApiMacro(MapBufferRange), \
    esApiMacro(FlushMappedBufferRange), \
    esApiMacro(BindVertexArray), \
    esApiMacro(DeleteVertexArrays), \
    esApiMacro(GenVertexArrays), \
    esApiMacro(IsVertexArray), \
    esApiMacro(GetIntegeri_v), \
    esApiMacro(BeginTransformFeedback), \
    esApiMacro(EndTransformFeedback), \
    esApiMacro(BindBufferRange), \
    esApiMacro(BindBufferBase), \
    esApiMacro(TransformFeedbackVaryings), \
    esApiMacro(GetTransformFeedbackVarying), \
    esApiMacro(VertexAttribIPointer), \
    esApiMacro(GetVertexAttribIiv), \
    esApiMacro(GetVertexAttribIuiv), \
    esApiMacro(VertexAttribI4i), \
    esApiMacro(VertexAttribI4ui), \
    esApiMacro(VertexAttribI4iv), \
    esApiMacro(VertexAttribI4uiv), \
    esApiMacro(GetUniformuiv), \
    esApiMacro(GetFragDataLocation), \
    esApiMacro(BindFragDataLocation), \
    esApiMacro(Uniform1ui), \
    esApiMacro(Uniform2ui), \
    esApiMacro(Uniform3ui), \
    esApiMacro(Uniform4ui), \
    esApiMacro(Uniform1uiv), \
    esApiMacro(Uniform2uiv), \
    esApiMacro(Uniform3uiv), \
    esApiMacro(Uniform4uiv), \
    esApiMacro(ClearBufferiv), \
    esApiMacro(ClearBufferuiv), \
    esApiMacro(ClearBufferfv), \
    esApiMacro(ClearBufferfi), \
    esApiMacro(GetStringi), \
    esApiMacro(CopyBufferSubData), \
    esApiMacro(GetUniformIndices), \
    esApiMacro(GetActiveUniformsiv), \
    esApiMacro(GetUniformBlockIndex), \
    esApiMacro(GetActiveUniformBlockiv), \
    esApiMacro(GetActiveUniformBlockName), \
    esApiMacro(UniformBlockBinding), \
    esApiMacro(DrawArraysInstanced), \
    esApiMacro(DrawElementsInstanced), \
    esApiMacro(FenceSync), \
    esApiMacro(IsSync), \
    esApiMacro(DeleteSync), \
    esApiMacro(ClientWaitSync), \
    esApiMacro(WaitSync), \
    esApiMacro(GetInteger64v), \
    esApiMacro(GetSynciv), \
    esApiMacro(GetInteger64i_v), \
    esApiMacro(GetBufferParameteri64v), \
    esApiMacro(GenSamplers), \
    esApiMacro(DeleteSamplers), \
    esApiMacro(IsSampler), \
    esApiMacro(BindSampler), \
    esApiMacro(SamplerParameteri), \
    esApiMacro(SamplerParameteriv), \
    esApiMacro(SamplerParameterf), \
    esApiMacro(SamplerParameterfv), \
    esApiMacro(GetSamplerParameteriv), \
    esApiMacro(GetSamplerParameterfv), \
    esApiMacro(VertexAttribDivisor), \
    esApiMacro(BindTransformFeedback), \
    esApiMacro(DeleteTransformFeedbacks), \
    esApiMacro(GenTransformFeedbacks), \
    esApiMacro(IsTransformFeedback), \
    esApiMacro(PauseTransformFeedback), \
    esApiMacro(ResumeTransformFeedback), \
    esApiMacro(GetProgramBinary), \
    esApiMacro(ProgramBinary), \
    esApiMacro(ProgramParameteri), \
    esApiMacro(InvalidateFramebuffer), \
    esApiMacro(InvalidateSubFramebuffer), \
    esApiMacro(TexStorage2D), \
    esApiMacro(TexStorage3D), \
    esApiMacro(GetInternalformativ), \
    /* OpenGL ES 3.1 */ \
    esApiMacro(DispatchCompute), \
    esApiMacro(DispatchComputeIndirect), \
    esApiMacro(DrawArraysIndirect), \
    esApiMacro(DrawElementsIndirect), \
    esApiMacro(FramebufferParameteri), \
    esApiMacro(GetFramebufferParameteriv), \
    esApiMacro(GetProgramInterfaceiv), \
    esApiMacro(GetProgramResourceIndex), \
    esApiMacro(GetProgramResourceName), \
    esApiMacro(GetProgramResourceiv), \
    esApiMacro(GetProgramResourceLocation), \
    esApiMacro(UseProgramStages), \
    esApiMacro(ActiveShaderProgram), \
    esApiMacro(CreateShaderProgramv), \
    esApiMacro(BindProgramPipeline), \
    esApiMacro(DeleteProgramPipelines), \
    esApiMacro(GenProgramPipelines), \
    esApiMacro(IsProgramPipeline), \
    esApiMacro(GetProgramPipelineiv), \
    esApiMacro(ProgramUniform1i), \
    esApiMacro(ProgramUniform2i), \
    esApiMacro(ProgramUniform3i), \
    esApiMacro(ProgramUniform4i), \
    esApiMacro(ProgramUniform1ui), \
    esApiMacro(ProgramUniform2ui), \
    esApiMacro(ProgramUniform3ui), \
    esApiMacro(ProgramUniform4ui), \
    esApiMacro(ProgramUniform1f), \
    esApiMacro(ProgramUniform2f), \
    esApiMacro(ProgramUniform3f), \
    esApiMacro(ProgramUniform4f), \
    esApiMacro(ProgramUniform1iv), \
    esApiMacro(ProgramUniform2iv), \
    esApiMacro(ProgramUniform3iv), \
    esApiMacro(ProgramUniform4iv), \
    esApiMacro(ProgramUniform1uiv), \
    esApiMacro(ProgramUniform2uiv), \
    esApiMacro(ProgramUniform3uiv), \
    esApiMacro(ProgramUniform4uiv), \
    esApiMacro(ProgramUniform1fv), \
    esApiMacro(ProgramUniform2fv), \
    esApiMacro(ProgramUniform3fv), \
    esApiMacro(ProgramUniform4fv), \
    esApiMacro(ProgramUniformMatrix2fv), \
    esApiMacro(ProgramUniformMatrix3fv), \
    esApiMacro(ProgramUniformMatrix4fv), \
    esApiMacro(ProgramUniformMatrix2x3fv), \
    esApiMacro(ProgramUniformMatrix3x2fv), \
    esApiMacro(ProgramUniformMatrix2x4fv), \
    esApiMacro(ProgramUniformMatrix4x2fv), \
    esApiMacro(ProgramUniformMatrix3x4fv), \
    esApiMacro(ProgramUniformMatrix4x3fv), \
    esApiMacro(ValidateProgramPipeline), \
    esApiMacro(GetProgramPipelineInfoLog), \
    esApiMacro(BindImageTexture), \
    esApiMacro(GetBooleani_v), \
    esApiMacro(MemoryBarrier), \
    esApiMacro(MemoryBarrierByRegion), \
    esApiMacro(TexStorage2DMultisample), \
    esApiMacro(GetMultisamplefv), \
    esApiMacro(SampleMaski), \
    esApiMacro(BindVertexBuffer), \
    esApiMacro(VertexAttribFormat), \
    esApiMacro(VertexAttribIFormat), \
    esApiMacro(VertexAttribBinding), \
    esApiMacro(VertexBindingDivisor), \
    /* OpenGL ES 3.2 */ \
    esApiMacro(TexStorage3DMultisample), \
    esApiMacro(BlendBarrier), \
    esApiMacro(DebugMessageControl), \
    esApiMacro(DebugMessageInsert), \
    esApiMacro(DebugMessageCallback), \
    esApiMacro(GetDebugMessageLog), \
    esApiMacro(PushDebugGroup), \
    esApiMacro(PopDebugGroup), \
    esApiMacro(ObjectLabel), \
    esApiMacro(GetObjectLabel), \
    esApiMacro(ObjectPtrLabel), \
    esApiMacro(GetObjectPtrLabel), \
    esApiMacro(GetGraphicsResetStatus), \
    esApiMacro(ReadnPixels), \
    esApiMacro(GetnUniformfv), \
    esApiMacro(GetnUniformiv), \
    esApiMacro(GetnUniformuiv), \
    esApiMacro(BlendEquationi), \
    esApiMacro(BlendEquationSeparatei), \
    esApiMacro(BlendFunci), \
    esApiMacro(BlendFuncSeparatei), \
    esApiMacro(ColorMaski), \
    esApiMacro(Enablei),  \
    esApiMacro(Disablei),\
    esApiMacro(IsEnabledi), \
    esApiMacro(TexParameterIiv),  \
    esApiMacro(TexParameterIuiv),  \
    esApiMacro(GetTexParameterIiv), \
    esApiMacro(GetTexParameterIuiv),  \
    esApiMacro(SamplerParameterIiv),  \
    esApiMacro(SamplerParameterIuiv),  \
    esApiMacro(GetSamplerParameterIiv),  \
    esApiMacro(GetSamplerParameterIuiv), \
    esApiMacro(TexBuffer), \
    esApiMacro(TexBufferRange), \
    esApiMacro(PatchParameteri), \
    esApiMacro(FramebufferTexture), \
    esApiMacro(MinSampleShading), \
    esApiMacro(CopyImageSubData), \
    esApiMacro(DrawElementsBaseVertex), \
    esApiMacro(DrawRangeElementsBaseVertex), \
    esApiMacro(DrawElementsInstancedBaseVertex), \
    esApiMacro(PrimitiveBoundingBox), \
    /* OpenGL ES extensions */ \
    /* GL_OES_EGL_image */ \
    esApiMacro(EGLImageTargetTexture2DOES), \
    esApiMacro(EGLImageTargetRenderbufferStorageOES), \
    /* GL_EXT_multi_draw_arrays */ \
    esApiMacro(MultiDrawElementsBaseVertexEXT), \
    /* GL_OES_mapbuffer */ \
    esApiMacro(GetBufferPointervOES), \
    /* GL_EXT_discard_framebuffer */ \
    esApiMacro(DiscardFramebufferEXT), \
    /* GL_EXT_multisampled_render_to_texture */ \
    esApiMacro(RenderbufferStorageMultisampleEXT), \
    esApiMacro(FramebufferTexture2DMultisampleEXT), \
    /* GL_VIV_direct_texture */ \
    esApiMacro(TexDirectVIV), \
    esApiMacro(TexDirectInvalidateVIV), \
    esApiMacro(TexDirectVIVMap), \
    esApiMacro(TexDirectTiledMapVIV), \
    /* GL_EXT_multi_draw_indirect */ \
    esApiMacro(MultiDrawArraysIndirectEXT), \
    esApiMacro(MultiDrawElementsIndirectEXT), \
    esApiMacro(DeleteObjectARB), \
    esApiMacro(GetInfoLogARB), \
    esApiMacro(GetObjectParameterivARB), \
    esApiMacro(ClampColorARB), \
    esApiMacro(IsRenderbufferEXT), \
    esApiMacro(BindRenderbufferEXT), \
    esApiMacro(DeleteRenderbuffersEXT), \
    esApiMacro(GenRenderbuffersEXT), \
    esApiMacro(RenderbufferStorageEXT), \
    esApiMacro(GetRenderbufferParameterivEXT), \
    esApiMacro(IsFramebufferEXT), \
    esApiMacro(BindFramebufferEXT), \
    esApiMacro(DeleteFramebuffersEXT), \
    esApiMacro(GenFramebuffersEXT), \
    esApiMacro(CheckFramebufferStatusEXT), \
    esApiMacro(FramebufferTexture1DEXT), \
    esApiMacro(FramebufferTexture2DEXT), \
    esApiMacro(FramebufferTexture3DEXT), \
    esApiMacro(FramebufferRenderbufferEXT), \
    esApiMacro(GetFramebufferAttachmentParameterivEXT), \
    esApiMacro(GenerateMipmapEXT), \
    esApiMacro(BlitFramebufferEXT), \
    esApiMacro(ClampColor), \
    esApiMacro(BeginConditionalRender), \
    esApiMacro(EndConditionalRender), \
    esApiMacro(VertexAttribI1i), \
    esApiMacro(VertexAttribI2i), \
    esApiMacro(VertexAttribI3i), \
    esApiMacro(VertexAttribI1ui), \
    esApiMacro(VertexAttribI2ui), \
    esApiMacro(VertexAttribI3ui), \
    esApiMacro(VertexAttribI1iv), \
    esApiMacro(VertexAttribI2iv), \
    esApiMacro(VertexAttribI3iv), \
    esApiMacro(VertexAttribI1uiv), \
    esApiMacro(VertexAttribI2uiv), \
    esApiMacro(VertexAttribI3uiv), \
    esApiMacro(VertexAttribI4bv), \
    esApiMacro(VertexAttribI4sv), \
    esApiMacro(VertexAttribI4ubv), \
    esApiMacro(VertexAttribI4usv), \
    esApiMacro(FramebufferTexture1D), \
    esApiMacro(FramebufferTexture3D), \
    esApiMacro(PrimitiveRestartIndex), \
    esApiMacro(GetActiveUniformName), \
    esApiMacro(GetUniformdv), \
    esApiMacro(Uniform1d), \
    esApiMacro(Uniform2d), \
    esApiMacro(Uniform3d), \
    esApiMacro(Uniform4d), \
    esApiMacro(Uniform1dv), \
    esApiMacro(Uniform2dv), \
    esApiMacro(Uniform3dv), \
    esApiMacro(Uniform4dv), \
    esApiMacro(UniformMatrix2dv), \
    esApiMacro(UniformMatrix3dv), \
    esApiMacro(UniformMatrix4dv), \
    esApiMacro(UniformMatrix2x3dv), \
    esApiMacro(UniformMatrix3x2dv), \
    esApiMacro(UniformMatrix2x4dv), \
    esApiMacro(UniformMatrix4x2dv), \
    esApiMacro(UniformMatrix3x4dv), \
    esApiMacro(UniformMatrix4x3dv), \


/* Enum table for __glop_FuncName (Ex: __glop_NewList)
 */
#define glop(name) __glop_##name
enum {
    __GL_LISTEXEC_ENTRIES(glop,=0)
    __glop_PrimContinue
};

#define __GLES_API_DISPATCH_FUNCS \
    GLvoid         (GLAPIENTRY *NewList) (_gcArgComma_  GLuint, GLenum); \
    GLvoid         (GLAPIENTRY *EndList) (_gcArgOnly_); \
    GLvoid         (GLAPIENTRY *CallList) (_gcArgComma_  GLuint); \
    GLvoid         (GLAPIENTRY *CallLists) (_gcArgComma_  GLsizei, GLenum, const GLvoid *); \
    GLvoid         (GLAPIENTRY *DeleteLists) (_gcArgComma_  GLuint, GLsizei); \
    GLuint         (GLAPIENTRY *GenLists) (_gcArgComma_  GLsizei); \
    GLvoid         (GLAPIENTRY *ListBase) (_gcArgComma_  GLuint); \
    GLvoid         (GLAPIENTRY *Begin) (_gcArgComma_  GLenum); \
    GLvoid         (GLAPIENTRY *Bitmap) (_gcArgComma_  GLsizei, GLsizei, GLfloat, GLfloat, GLfloat, GLfloat, const GLubyte *); \
    GLvoid         (GLAPIENTRY *Color3b) (_gcArgComma_  GLbyte, GLbyte, GLbyte); \
    GLvoid         (GLAPIENTRY *Color3bv) (_gcArgComma_  const GLbyte *); \
    GLvoid         (GLAPIENTRY *Color3d) (_gcArgComma_  GLdouble, GLdouble, GLdouble); \
    GLvoid         (GLAPIENTRY *Color3dv) (_gcArgComma_  const GLdouble *); \
    GLvoid         (GLAPIENTRY *Color3f) (_gcArgComma_  GLfloat, GLfloat, GLfloat); \
    GLvoid         (GLAPIENTRY *Color3fv) (_gcArgComma_  const GLfloat *); \
    GLvoid         (GLAPIENTRY *Color3i) (_gcArgComma_  GLint, GLint, GLint); \
    GLvoid         (GLAPIENTRY *Color3iv) (_gcArgComma_  const GLint *); \
    GLvoid         (GLAPIENTRY *Color3s) (_gcArgComma_  GLshort, GLshort, GLshort); \
    GLvoid         (GLAPIENTRY *Color3sv) (_gcArgComma_  const GLshort *); \
    GLvoid         (GLAPIENTRY *Color3ub) (_gcArgComma_  GLubyte, GLubyte, GLubyte); \
    GLvoid         (GLAPIENTRY *Color3ubv) (_gcArgComma_  const GLubyte *); \
    GLvoid         (GLAPIENTRY *Color3ui) (_gcArgComma_  GLuint, GLuint, GLuint); \
    GLvoid         (GLAPIENTRY *Color3uiv) (_gcArgComma_  const GLuint *); \
    GLvoid         (GLAPIENTRY *Color3us) (_gcArgComma_  GLushort, GLushort, GLushort); \
    GLvoid         (GLAPIENTRY *Color3usv) (_gcArgComma_  const GLushort *); \
    GLvoid         (GLAPIENTRY *Color4b) (_gcArgComma_  GLbyte, GLbyte, GLbyte, GLbyte); \
    GLvoid         (GLAPIENTRY *Color4bv) (_gcArgComma_  const GLbyte *); \
    GLvoid         (GLAPIENTRY *Color4d) (_gcArgComma_  GLdouble, GLdouble, GLdouble, GLdouble); \
    GLvoid         (GLAPIENTRY *Color4dv) (_gcArgComma_  const GLdouble *); \
    GLvoid         (GLAPIENTRY *Color4f) (_gcArgComma_  GLfloat, GLfloat, GLfloat, GLfloat); \
    GLvoid         (GLAPIENTRY *Color4fv) (_gcArgComma_  const GLfloat *); \
    GLvoid         (GLAPIENTRY *Color4i) (_gcArgComma_  GLint, GLint, GLint, GLint); \
    GLvoid         (GLAPIENTRY *Color4iv) (_gcArgComma_  const GLint *); \
    GLvoid         (GLAPIENTRY *Color4s) (_gcArgComma_  GLshort, GLshort, GLshort, GLshort); \
    GLvoid         (GLAPIENTRY *Color4sv) (_gcArgComma_  const GLshort *); \
    GLvoid         (GLAPIENTRY *Color4ub) (_gcArgComma_  GLubyte, GLubyte, GLubyte, GLubyte); \
    GLvoid         (GLAPIENTRY *Color4ubv) (_gcArgComma_  const GLubyte *); \
    GLvoid         (GLAPIENTRY *Color4ui) (_gcArgComma_  GLuint, GLuint, GLuint, GLuint); \
    GLvoid         (GLAPIENTRY *Color4uiv) (_gcArgComma_  const GLuint *); \
    GLvoid         (GLAPIENTRY *Color4us) (_gcArgComma_  GLushort, GLushort, GLushort, GLushort); \
    GLvoid         (GLAPIENTRY *Color4usv) (_gcArgComma_  const GLushort *); \
    GLvoid         (GLAPIENTRY *EdgeFlag) (_gcArgComma_  GLboolean); \
    GLvoid         (GLAPIENTRY *EdgeFlagv) (_gcArgComma_  const GLboolean *); \
    GLvoid         (GLAPIENTRY *End) (_gcArgOnly_); \
    GLvoid         (GLAPIENTRY *Indexd) (_gcArgComma_  GLdouble); \
    GLvoid         (GLAPIENTRY *Indexdv) (_gcArgComma_  const GLdouble *); \
    GLvoid         (GLAPIENTRY *Indexf) (_gcArgComma_  GLfloat); \
    GLvoid         (GLAPIENTRY *Indexfv) (_gcArgComma_  const GLfloat *); \
    GLvoid         (GLAPIENTRY *Indexi) (_gcArgComma_  GLint); \
    GLvoid         (GLAPIENTRY *Indexiv) (_gcArgComma_  const GLint *); \
    GLvoid         (GLAPIENTRY *Indexs) (_gcArgComma_  GLshort); \
    GLvoid         (GLAPIENTRY *Indexsv) (_gcArgComma_  const GLshort *); \
    GLvoid         (GLAPIENTRY *Normal3b) (_gcArgComma_  GLbyte, GLbyte, GLbyte); \
    GLvoid         (GLAPIENTRY *Normal3bv) (_gcArgComma_  const GLbyte *); \
    GLvoid         (GLAPIENTRY *Normal3d) (_gcArgComma_  GLdouble, GLdouble, GLdouble); \
    GLvoid         (GLAPIENTRY *Normal3dv) (_gcArgComma_  const GLdouble *); \
    GLvoid         (GLAPIENTRY *Normal3f) (_gcArgComma_  GLfloat, GLfloat, GLfloat); \
    GLvoid         (GLAPIENTRY *Normal3fv) (_gcArgComma_  const GLfloat *); \
    GLvoid         (GLAPIENTRY *Normal3i) (_gcArgComma_  GLint, GLint, GLint); \
    GLvoid         (GLAPIENTRY *Normal3iv) (_gcArgComma_  const GLint *); \
    GLvoid         (GLAPIENTRY *Normal3s) (_gcArgComma_  GLshort, GLshort, GLshort); \
    GLvoid         (GLAPIENTRY *Normal3sv) (_gcArgComma_  const GLshort *); \
    GLvoid         (GLAPIENTRY *RasterPos2d) (_gcArgComma_  GLdouble, GLdouble); \
    GLvoid         (GLAPIENTRY *RasterPos2dv) (_gcArgComma_  const GLdouble *); \
    GLvoid         (GLAPIENTRY *RasterPos2f) (_gcArgComma_  GLfloat, GLfloat); \
    GLvoid         (GLAPIENTRY *RasterPos2fv) (_gcArgComma_  const GLfloat *); \
    GLvoid         (GLAPIENTRY *RasterPos2i) (_gcArgComma_  GLint, GLint); \
    GLvoid         (GLAPIENTRY *RasterPos2iv) (_gcArgComma_  const GLint *); \
    GLvoid         (GLAPIENTRY *RasterPos2s) (_gcArgComma_  GLshort, GLshort); \
    GLvoid         (GLAPIENTRY *RasterPos2sv) (_gcArgComma_  const GLshort *); \
    GLvoid         (GLAPIENTRY *RasterPos3d) (_gcArgComma_  GLdouble, GLdouble, GLdouble); \
    GLvoid         (GLAPIENTRY *RasterPos3dv) (_gcArgComma_  const GLdouble *); \
    GLvoid         (GLAPIENTRY *RasterPos3f) (_gcArgComma_  GLfloat, GLfloat, GLfloat); \
    GLvoid         (GLAPIENTRY *RasterPos3fv) (_gcArgComma_  const GLfloat *); \
    GLvoid         (GLAPIENTRY *RasterPos3i) (_gcArgComma_  GLint, GLint, GLint); \
    GLvoid         (GLAPIENTRY *RasterPos3iv) (_gcArgComma_  const GLint *); \
    GLvoid         (GLAPIENTRY *RasterPos3s) (_gcArgComma_  GLshort, GLshort, GLshort); \
    GLvoid         (GLAPIENTRY *RasterPos3sv) (_gcArgComma_  const GLshort *); \
    GLvoid         (GLAPIENTRY *RasterPos4d) (_gcArgComma_  GLdouble, GLdouble, GLdouble, GLdouble); \
    GLvoid         (GLAPIENTRY *RasterPos4dv) (_gcArgComma_  const GLdouble *); \
    GLvoid         (GLAPIENTRY *RasterPos4f) (_gcArgComma_  GLfloat, GLfloat, GLfloat, GLfloat); \
    GLvoid         (GLAPIENTRY *RasterPos4fv) (_gcArgComma_  const GLfloat *); \
    GLvoid         (GLAPIENTRY *RasterPos4i) (_gcArgComma_  GLint, GLint, GLint, GLint); \
    GLvoid         (GLAPIENTRY *RasterPos4iv) (_gcArgComma_  const GLint *); \
    GLvoid         (GLAPIENTRY *RasterPos4s) (_gcArgComma_  GLshort, GLshort, GLshort, GLshort); \
    GLvoid         (GLAPIENTRY *RasterPos4sv) (_gcArgComma_  const GLshort *); \
    GLvoid         (GLAPIENTRY *Rectd) (_gcArgComma_  GLdouble, GLdouble, GLdouble, GLdouble); \
    GLvoid         (GLAPIENTRY *Rectdv) (_gcArgComma_  const GLdouble *, const GLdouble *); \
    GLvoid         (GLAPIENTRY *Rectf) (_gcArgComma_  GLfloat, GLfloat, GLfloat, GLfloat); \
    GLvoid         (GLAPIENTRY *Rectfv) (_gcArgComma_  const GLfloat *, const GLfloat *); \
    GLvoid         (GLAPIENTRY *Recti) (_gcArgComma_  GLint, GLint, GLint, GLint); \
    GLvoid         (GLAPIENTRY *Rectiv) (_gcArgComma_  const GLint *, const GLint *); \
    GLvoid         (GLAPIENTRY *Rects) (_gcArgComma_  GLshort, GLshort, GLshort, GLshort); \
    GLvoid         (GLAPIENTRY *Rectsv) (_gcArgComma_  const GLshort *, const GLshort *); \
    GLvoid         (GLAPIENTRY *TexCoord1d) (_gcArgComma_  GLdouble); \
    GLvoid         (GLAPIENTRY *TexCoord1dv) (_gcArgComma_  const GLdouble *); \
    GLvoid         (GLAPIENTRY *TexCoord1f) (_gcArgComma_  GLfloat); \
    GLvoid         (GLAPIENTRY *TexCoord1fv) (_gcArgComma_  const GLfloat *); \
    GLvoid         (GLAPIENTRY *TexCoord1i) (_gcArgComma_  GLint); \
    GLvoid         (GLAPIENTRY *TexCoord1iv) (_gcArgComma_  const GLint *); \
    GLvoid         (GLAPIENTRY *TexCoord1s) (_gcArgComma_  GLshort); \
    GLvoid         (GLAPIENTRY *TexCoord1sv) (_gcArgComma_  const GLshort *); \
    GLvoid         (GLAPIENTRY *TexCoord2d) (_gcArgComma_  GLdouble, GLdouble); \
    GLvoid         (GLAPIENTRY *TexCoord2dv) (_gcArgComma_  const GLdouble *); \
    GLvoid         (GLAPIENTRY *TexCoord2f) (_gcArgComma_  GLfloat, GLfloat); \
    GLvoid         (GLAPIENTRY *TexCoord2fv) (_gcArgComma_  const GLfloat *); \
    GLvoid         (GLAPIENTRY *TexCoord2i) (_gcArgComma_  GLint, GLint); \
    GLvoid         (GLAPIENTRY *TexCoord2iv) (_gcArgComma_  const GLint *); \
    GLvoid         (GLAPIENTRY *TexCoord2s) (_gcArgComma_  GLshort, GLshort); \
    GLvoid         (GLAPIENTRY *TexCoord2sv) (_gcArgComma_  const GLshort *); \
    GLvoid         (GLAPIENTRY *TexCoord3d) (_gcArgComma_  GLdouble, GLdouble, GLdouble); \
    GLvoid         (GLAPIENTRY *TexCoord3dv) (_gcArgComma_  const GLdouble *); \
    GLvoid         (GLAPIENTRY *TexCoord3f) (_gcArgComma_  GLfloat, GLfloat, GLfloat); \
    GLvoid         (GLAPIENTRY *TexCoord3fv) (_gcArgComma_  const GLfloat *); \
    GLvoid         (GLAPIENTRY *TexCoord3i) (_gcArgComma_  GLint, GLint, GLint); \
    GLvoid         (GLAPIENTRY *TexCoord3iv) (_gcArgComma_  const GLint *); \
    GLvoid         (GLAPIENTRY *TexCoord3s) (_gcArgComma_  GLshort, GLshort, GLshort); \
    GLvoid         (GLAPIENTRY *TexCoord3sv) (_gcArgComma_  const GLshort *); \
    GLvoid         (GLAPIENTRY *TexCoord4d) (_gcArgComma_  GLdouble, GLdouble, GLdouble, GLdouble); \
    GLvoid         (GLAPIENTRY *TexCoord4dv) (_gcArgComma_  const GLdouble *); \
    GLvoid         (GLAPIENTRY *TexCoord4f) (_gcArgComma_  GLfloat, GLfloat, GLfloat, GLfloat); \
    GLvoid         (GLAPIENTRY *TexCoord4fv) (_gcArgComma_  const GLfloat *); \
    GLvoid         (GLAPIENTRY *TexCoord4i) (_gcArgComma_  GLint, GLint, GLint, GLint); \
    GLvoid         (GLAPIENTRY *TexCoord4iv) (_gcArgComma_  const GLint *); \
    GLvoid         (GLAPIENTRY *TexCoord4s) (_gcArgComma_  GLshort, GLshort, GLshort, GLshort); \
    GLvoid         (GLAPIENTRY *TexCoord4sv) (_gcArgComma_  const GLshort *); \
    GLvoid         (GLAPIENTRY *Vertex2d) (_gcArgComma_  GLdouble, GLdouble); \
    GLvoid         (GLAPIENTRY *Vertex2dv) (_gcArgComma_  const GLdouble *); \
    GLvoid         (GLAPIENTRY *Vertex2f) (_gcArgComma_  GLfloat, GLfloat); \
    GLvoid         (GLAPIENTRY *Vertex2fv) (_gcArgComma_  const GLfloat *); \
    GLvoid         (GLAPIENTRY *Vertex2i) (_gcArgComma_  GLint, GLint); \
    GLvoid         (GLAPIENTRY *Vertex2iv) (_gcArgComma_  const GLint *); \
    GLvoid         (GLAPIENTRY *Vertex2s) (_gcArgComma_  GLshort, GLshort); \
    GLvoid         (GLAPIENTRY *Vertex2sv) (_gcArgComma_  const GLshort *); \
    GLvoid         (GLAPIENTRY *Vertex3d) (_gcArgComma_  GLdouble, GLdouble, GLdouble); \
    GLvoid         (GLAPIENTRY *Vertex3dv) (_gcArgComma_  const GLdouble *); \
    GLvoid         (GLAPIENTRY *Vertex3f) (_gcArgComma_  GLfloat, GLfloat, GLfloat); \
    GLvoid         (GLAPIENTRY *Vertex3fv) (_gcArgComma_  const GLfloat *); \
    GLvoid         (GLAPIENTRY *Vertex3i) (_gcArgComma_  GLint, GLint, GLint); \
    GLvoid         (GLAPIENTRY *Vertex3iv) (_gcArgComma_  const GLint *); \
    GLvoid         (GLAPIENTRY *Vertex3s) (_gcArgComma_  GLshort, GLshort, GLshort); \
    GLvoid         (GLAPIENTRY *Vertex3sv) (_gcArgComma_  const GLshort *); \
    GLvoid         (GLAPIENTRY *Vertex4d) (_gcArgComma_  GLdouble, GLdouble, GLdouble, GLdouble); \
    GLvoid         (GLAPIENTRY *Vertex4dv) (_gcArgComma_  const GLdouble *); \
    GLvoid         (GLAPIENTRY *Vertex4f) (_gcArgComma_  GLfloat, GLfloat, GLfloat, GLfloat); \
    GLvoid         (GLAPIENTRY *Vertex4fv) (_gcArgComma_  const GLfloat *); \
    GLvoid         (GLAPIENTRY *Vertex4i) (_gcArgComma_  GLint, GLint, GLint, GLint); \
    GLvoid         (GLAPIENTRY *Vertex4iv) (_gcArgComma_  const GLint *); \
    GLvoid         (GLAPIENTRY *Vertex4s) (_gcArgComma_  GLshort, GLshort, GLshort, GLshort); \
    GLvoid         (GLAPIENTRY *Vertex4sv) (_gcArgComma_  const GLshort *); \
    GLvoid         (GLAPIENTRY *ClipPlane) (_gcArgComma_  GLenum, const GLdouble *); \
    GLvoid         (GLAPIENTRY *ColorMaterial) (_gcArgComma_  GLenum, GLenum); \
    GLvoid         (GLAPIENTRY *CullFace) (_gcArgComma_ GLenum mode); \
    GLvoid         (GLAPIENTRY *Fogf) (_gcArgComma_  GLenum, GLfloat); \
    GLvoid         (GLAPIENTRY *Fogfv) (_gcArgComma_  GLenum, const GLfloat *); \
    GLvoid         (GLAPIENTRY *Fogi) (_gcArgComma_  GLenum, GLint); \
    GLvoid         (GLAPIENTRY *Fogiv) (_gcArgComma_  GLenum, const GLint *); \
    GLvoid         (GLAPIENTRY *FrontFace) (_gcArgComma_ GLenum mode); \
    GLvoid         (GLAPIENTRY *Hint) (_gcArgComma_ GLenum target, GLenum mode); \
    GLvoid         (GLAPIENTRY *Lightf) (_gcArgComma_  GLenum, GLenum, GLfloat); \
    GLvoid         (GLAPIENTRY *Lightfv) (_gcArgComma_  GLenum, GLenum, const GLfloat *); \
    GLvoid         (GLAPIENTRY *Lighti) (_gcArgComma_  GLenum, GLenum, GLint); \
    GLvoid         (GLAPIENTRY *Lightiv) (_gcArgComma_  GLenum, GLenum, const GLint *); \
    GLvoid         (GLAPIENTRY *LightModelf) (_gcArgComma_  GLenum, GLfloat); \
    GLvoid         (GLAPIENTRY *LightModelfv) (_gcArgComma_  GLenum, const GLfloat *); \
    GLvoid         (GLAPIENTRY *LightModeli) (_gcArgComma_  GLenum, GLint); \
    GLvoid         (GLAPIENTRY *LightModeliv) (_gcArgComma_  GLenum, const GLint *); \
    GLvoid         (GLAPIENTRY *LineStipple) (_gcArgComma_  GLint, GLushort); \
    GLvoid         (GLAPIENTRY *LineWidth) (_gcArgComma_ GLfloat width); \
    GLvoid         (GLAPIENTRY *Materialf) (_gcArgComma_  GLenum, GLenum, GLfloat); \
    GLvoid         (GLAPIENTRY *Materialfv) (_gcArgComma_  GLenum, GLenum, const GLfloat *); \
    GLvoid         (GLAPIENTRY *Materiali) (_gcArgComma_  GLenum, GLenum, GLint); \
    GLvoid         (GLAPIENTRY *Materialiv) (_gcArgComma_  GLenum, GLenum, const GLint *); \
    GLvoid         (GLAPIENTRY *PointSize) (_gcArgComma_  GLfloat); \
    GLvoid         (GLAPIENTRY *PolygonMode) (_gcArgComma_  GLenum, GLenum); \
    GLvoid         (GLAPIENTRY *PolygonStipple) (_gcArgComma_  const GLubyte *); \
    GLvoid         (GLAPIENTRY *Scissor) (_gcArgComma_ GLint x, GLint y, GLsizei width, GLsizei height); \
    GLvoid         (GLAPIENTRY *ShadeModel) (_gcArgComma_  GLenum); \
    GLvoid         (GLAPIENTRY *TexParameterf) (_gcArgComma_ GLenum target, GLenum pname, GLfloat param); \
    GLvoid         (GLAPIENTRY *TexParameterfv) (_gcArgComma_ GLenum target, GLenum pname, const GLfloat* params); \
    GLvoid         (GLAPIENTRY *TexParameteri) (_gcArgComma_ GLenum target, GLenum pname, GLint param); \
    GLvoid         (GLAPIENTRY *TexParameteriv) (_gcArgComma_ GLenum target, GLenum pname, const GLint* params); \
    GLvoid         (GLAPIENTRY *TexImage1D) (_gcArgComma_  GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, const GLvoid *); \
    GLvoid         (GLAPIENTRY *TexImage2D) (_gcArgComma_ GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels); \
    GLvoid         (GLAPIENTRY *TexEnvf) (_gcArgComma_  GLenum, GLenum, GLfloat); \
    GLvoid         (GLAPIENTRY *TexEnvfv) (_gcArgComma_  GLenum, GLenum, const GLfloat *); \
    GLvoid         (GLAPIENTRY *TexEnvi) (_gcArgComma_  GLenum, GLenum, GLint); \
    GLvoid         (GLAPIENTRY *TexEnviv) (_gcArgComma_  GLenum, GLenum, const GLint *); \
    GLvoid         (GLAPIENTRY *TexGend) (_gcArgComma_  GLenum, GLenum, GLdouble); \
    GLvoid         (GLAPIENTRY *TexGendv) (_gcArgComma_  GLenum, GLenum, const GLdouble *); \
    GLvoid         (GLAPIENTRY *TexGenf) (_gcArgComma_  GLenum, GLenum, GLfloat); \
    GLvoid         (GLAPIENTRY *TexGenfv) (_gcArgComma_  GLenum, GLenum, const GLfloat *); \
    GLvoid         (GLAPIENTRY *TexGeni) (_gcArgComma_  GLenum, GLenum, GLint); \
    GLvoid         (GLAPIENTRY *TexGeniv) (_gcArgComma_  GLenum, GLenum, const GLint *); \
    GLvoid         (GLAPIENTRY *FeedbackBuffer) (_gcArgComma_  GLsizei, GLenum, GLfloat *); \
    GLvoid         (GLAPIENTRY *SelectBuffer) (_gcArgComma_  GLsizei, GLuint *); \
    GLint         (GLAPIENTRY *RenderMode) (_gcArgComma_  GLenum); \
    GLvoid         (GLAPIENTRY *InitNames) (_gcArgOnly_); \
    GLvoid         (GLAPIENTRY *LoadName) (_gcArgComma_  GLuint); \
    GLvoid         (GLAPIENTRY *PassThrough) (_gcArgComma_  GLfloat); \
    GLvoid         (GLAPIENTRY *PopName) (_gcArgOnly_); \
    GLvoid         (GLAPIENTRY *PushName) (_gcArgComma_  GLuint); \
    GLvoid         (GLAPIENTRY *DrawBuffer) (_gcArgComma_  GLenum); \
    GLvoid         (GLAPIENTRY *Clear) (_gcArgComma_ GLbitfield mask); \
    GLvoid         (GLAPIENTRY *ClearAccum)(_gcArgComma_  GLfloat, GLfloat, GLfloat, GLfloat); \
    GLvoid         (GLAPIENTRY *ClearIndex)(_gcArgComma_  GLfloat); \
    GLvoid         (GLAPIENTRY *ClearColor) (_gcArgComma_ GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha); \
    GLvoid         (GLAPIENTRY *ClearStencil) (_gcArgComma_ GLint s); \
    GLvoid         (GLAPIENTRY *ClearDepth)(_gcArgComma_  GLclampd); \
    GLvoid         (GLAPIENTRY *StencilMask) (_gcArgComma_ GLuint mask); \
    GLvoid         (GLAPIENTRY *ColorMask) (_gcArgComma_ GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha); \
    GLvoid         (GLAPIENTRY *DepthMask) (_gcArgComma_ GLboolean flag); \
    GLvoid         (GLAPIENTRY *IndexMask)(_gcArgComma_ GLuint); \
    GLvoid         (GLAPIENTRY *Accum)(_gcArgComma_ GLenum, GLfloat); \
    GLvoid         (GLAPIENTRY *Disable) (_gcArgComma_ GLenum cap); \
    GLvoid         (GLAPIENTRY *Enable) (_gcArgComma_ GLenum cap); \
    GLvoid         (GLAPIENTRY *Finish) (_gcArgOnly_); \
    GLvoid         (GLAPIENTRY *Flush) (_gcArgOnly_); \
    GLvoid         (GLAPIENTRY *PopAttrib)(_gcArgOnly_); \
    GLvoid         (GLAPIENTRY *PushAttrib)(_gcArgComma_  GLbitfield); \
    GLvoid         (GLAPIENTRY *Map1d)(_gcArgComma_ GLenum, GLdouble, GLdouble, GLint, GLint, const GLdouble *); \
    GLvoid         (GLAPIENTRY *Map1f)(_gcArgComma_ GLenum, GLfloat, GLfloat, GLint, GLint, const GLfloat *); \
    GLvoid         (GLAPIENTRY *Map2d)(_gcArgComma_ GLenum, GLdouble, GLdouble, GLint, GLint, GLdouble, GLdouble, GLint, GLint, const GLdouble *); \
    GLvoid         (GLAPIENTRY *Map2f)(_gcArgComma_ GLenum, GLfloat, GLfloat, GLint, GLint, GLfloat, GLfloat, GLint, GLint, const GLfloat *); \
    GLvoid         (GLAPIENTRY *MapGrid1d)(_gcArgComma_ GLint, GLdouble, GLdouble); \
    GLvoid         (GLAPIENTRY *MapGrid1f)(_gcArgComma_ GLint, GLfloat, GLfloat); \
    GLvoid         (GLAPIENTRY *MapGrid2d)(_gcArgComma_ GLint, GLdouble, GLdouble, GLint, GLdouble, GLdouble); \
    GLvoid         (GLAPIENTRY *MapGrid2f)(_gcArgComma_  GLint, GLfloat, GLfloat, GLint, GLfloat, GLfloat); \
    GLvoid         (GLAPIENTRY *EvalCoord1d)(_gcArgComma_ GLdouble); \
    GLvoid         (GLAPIENTRY *EvalCoord1dv)(_gcArgComma_ const GLdouble *); \
    GLvoid         (GLAPIENTRY *EvalCoord1f)(_gcArgComma_  GLfloat); \
    GLvoid         (GLAPIENTRY *EvalCoord1fv)(_gcArgComma_ const GLfloat *); \
    GLvoid         (GLAPIENTRY *EvalCoord2d)(_gcArgComma_  GLdouble, GLdouble); \
    GLvoid         (GLAPIENTRY *EvalCoord2dv)(_gcArgComma_ const GLdouble *); \
    GLvoid         (GLAPIENTRY *EvalCoord2f)(_gcArgComma_  GLfloat, GLfloat); \
    GLvoid         (GLAPIENTRY *EvalCoord2fv)(_gcArgComma_ const GLfloat *); \
    GLvoid         (GLAPIENTRY *EvalMesh1)(_gcArgComma_ GLenum, GLint, GLint); \
    GLvoid         (GLAPIENTRY *EvalPoint1)(_gcArgComma_ GLint); \
    GLvoid         (GLAPIENTRY *EvalMesh2)(_gcArgComma_  GLenum, GLint, GLint, GLint, GLint); \
    GLvoid         (GLAPIENTRY *EvalPoint2)(_gcArgComma_ GLint, GLint); \
    GLvoid         (GLAPIENTRY *AlphaFunc)(_gcArgComma_ GLenum, GLclampf); \
    GLvoid         (GLAPIENTRY *BlendFunc) (_gcArgComma_ GLenum sfactor, GLenum dfactor); \
    GLvoid         (GLAPIENTRY *LogicOp)(_gcArgComma_ GLenum); \
    GLvoid         (GLAPIENTRY *StencilFunc) (_gcArgComma_ GLenum func, GLint ref, GLuint mask); \
    GLvoid         (GLAPIENTRY *StencilOp) (_gcArgComma_ GLenum fail, GLenum zfail, GLenum zpass); \
    GLvoid         (GLAPIENTRY *DepthFunc) (_gcArgComma_ GLenum func); \
    GLvoid         (GLAPIENTRY *PixelZoom)(_gcArgComma_ GLfloat, GLfloat); \
    GLvoid         (GLAPIENTRY *PixelTransferf)(_gcArgComma_ GLenum, GLfloat); \
    GLvoid         (GLAPIENTRY *PixelTransferi)(_gcArgComma_ GLenum, GLint); \
    GLvoid         (GLAPIENTRY *PixelStoref)(_gcArgComma_ GLenum, GLfloat); \
    GLvoid         (GLAPIENTRY *PixelStorei) (_gcArgComma_ GLenum pname, GLint param); \
    GLvoid         (GLAPIENTRY *PixelMapfv)(_gcArgComma_ GLenum, GLint, const GLfloat *); \
    GLvoid         (GLAPIENTRY *PixelMapuiv)(_gcArgComma_ GLenum, GLint, const GLuint *); \
    GLvoid         (GLAPIENTRY *PixelMapusv)(_gcArgComma_ GLenum, GLint, const GLushort *); \
    GLvoid         (GLAPIENTRY *ReadBuffer) (_gcArgComma_ GLenum mode); \
    GLvoid         (GLAPIENTRY *CopyPixels)(_gcArgComma_  GLint, GLint, GLsizei, GLsizei, GLenum); \
    GLvoid         (GLAPIENTRY *ReadPixels) (_gcArgComma_ GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels); \
    GLvoid         (GLAPIENTRY *DrawPixels)(_gcArgComma_  GLsizei, GLsizei, GLenum, GLenum, const GLvoid *); \
    GLvoid         (GLAPIENTRY *GetBooleanv) (_gcArgComma_ GLenum pname, GLboolean* params); \
    GLvoid         (GLAPIENTRY *GetClipPlane)(_gcArgComma_  GLenum, GLdouble *);    \
    GLvoid         (GLAPIENTRY *GetDoublev)(_gcArgComma_  GLenum, GLdouble *); \
    GLenum         (GLAPIENTRY *GetError) (_gcArgOnly_); \
    GLvoid         (GLAPIENTRY *GetFloatv) (_gcArgComma_ GLenum pname, GLfloat* params); \
    GLvoid         (GLAPIENTRY *GetIntegerv) (_gcArgComma_ GLenum pname, GLint* params); \
    GLvoid         (GLAPIENTRY *GetLightfv)(_gcArgComma_  GLenum, GLenum, GLfloat *); \
    GLvoid         (GLAPIENTRY *GetLightiv)(_gcArgComma_   GLenum, GLenum, GLint *); \
    GLvoid         (GLAPIENTRY *GetMapdv)(_gcArgComma_  GLenum, GLenum, GLdouble *); \
    GLvoid         (GLAPIENTRY *GetMapfv)(_gcArgComma_   GLenum, GLenum, GLfloat *); \
    GLvoid         (GLAPIENTRY *GetMapiv)(_gcArgComma_   GLenum, GLenum, GLint *); \
    GLvoid         (GLAPIENTRY *GetMaterialfv)(_gcArgComma_   GLenum, GLenum, GLfloat *); \
    GLvoid         (GLAPIENTRY *GetMaterialiv)(_gcArgComma_   GLenum, GLenum, GLint *); \
    GLvoid         (GLAPIENTRY *GetPixelMapfv)(_gcArgComma_  GLenum, GLfloat *); \
    GLvoid         (GLAPIENTRY *GetPixelMapuiv)(_gcArgComma_  GLenum, GLuint *); \
    GLvoid         (GLAPIENTRY *GetPixelMapusv)(_gcArgComma_  GLenum, GLushort *); \
    GLvoid         (GLAPIENTRY *GetPolygonStipple)(_gcArgComma_  GLubyte *); \
    const GLubyte* (GLAPIENTRY *GetString) (_gcArgComma_ GLenum name); \
    GLvoid         (GLAPIENTRY *GetTexEnvfv)(_gcArgComma_   GLenum, GLenum, GLfloat *); \
    GLvoid         (GLAPIENTRY *GetTexEnviv)(_gcArgComma_  GLenum, GLenum, GLint *); \
    GLvoid         (GLAPIENTRY *GetTexGendv)(_gcArgComma_  GLenum, GLenum, GLdouble *); \
    GLvoid         (GLAPIENTRY *GetTexGenfv)(_gcArgComma_  GLenum, GLenum, GLfloat *); \
    GLvoid         (GLAPIENTRY *GetTexGeniv)(_gcArgComma_  GLenum, GLenum, GLint *); \
    GLvoid         (GLAPIENTRY *GetTexImage)(_gcArgComma_  GLenum, GLint, GLenum, GLenum, GLvoid *); \
    GLvoid         (GLAPIENTRY *GetTexParameterfv) (_gcArgComma_ GLenum target, GLenum pname, GLfloat* params); \
    GLvoid         (GLAPIENTRY *GetTexParameteriv) (_gcArgComma_ GLenum target, GLenum pname, GLint* params); \
    GLvoid         (GLAPIENTRY *GetTexLevelParameterfv) (_gcArgComma_ GLenum target, GLint level, GLenum pname, GLfloat *params); \
    GLvoid         (GLAPIENTRY *GetTexLevelParameteriv) (_gcArgComma_ GLenum target, GLint level, GLenum pname, GLint *params); \
    GLboolean      (GLAPIENTRY *IsEnabled) (_gcArgComma_ GLenum cap); \
    GLboolean    (GLAPIENTRY *IsList)(_gcArgComma_  GLuint); \
    GLvoid         (GLAPIENTRY *DepthRange)(_gcArgComma_  GLclampd, GLclampd); \
    GLvoid         (GLAPIENTRY *Frustum)(_gcArgComma_ GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble); \
    GLvoid         (GLAPIENTRY *LoadIdentity)(_gcArgOnly_); \
    GLvoid         (GLAPIENTRY *LoadMatrixf)(_gcArgComma_  const GLfloat *); \
    GLvoid         (GLAPIENTRY *LoadMatrixd)(_gcArgComma_  const GLdouble *); \
    GLvoid         (GLAPIENTRY *MatrixMode)(_gcArgComma_ GLenum); \
    GLvoid         (GLAPIENTRY *MultMatrixf)(_gcArgComma_ const GLfloat *); \
    GLvoid         (GLAPIENTRY *MultMatrixd)(_gcArgComma_ const GLdouble *); \
    GLvoid         (GLAPIENTRY *Ortho)(_gcArgComma_  GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble); \
    GLvoid         (GLAPIENTRY *PopMatrix)(_gcArgOnly_); \
    GLvoid         (GLAPIENTRY *PushMatrix)(_gcArgOnly_); \
    GLvoid         (GLAPIENTRY *Rotated)(_gcArgComma_ GLdouble, GLdouble, GLdouble, GLdouble); \
    GLvoid         (GLAPIENTRY *Rotatef)(_gcArgComma_ GLfloat, GLfloat, GLfloat, GLfloat); \
    GLvoid         (GLAPIENTRY *Scaled)(_gcArgComma_  GLdouble, GLdouble, GLdouble); \
    GLvoid         (GLAPIENTRY *Scalef)(_gcArgComma_ GLfloat, GLfloat, GLfloat); \
    GLvoid         (GLAPIENTRY *Translated)(_gcArgComma_ GLdouble, GLdouble, GLdouble); \
    GLvoid         (GLAPIENTRY *Translatef)(_gcArgComma_ GLfloat, GLfloat, GLfloat); \
    GLvoid         (GLAPIENTRY *Viewport) (_gcArgComma_ GLint x, GLint y, GLsizei width, GLsizei height); \
    GLvoid         (GLAPIENTRY *ArrayElement) (_gcArgComma_  GLint); \
    GLvoid         (GLAPIENTRY *BindTexture) (_gcArgComma_ GLenum target, GLuint texture); \
    GLvoid         (GLAPIENTRY *ColorPointer)(_gcArgComma_ GLint, GLenum, GLsizei, const GLvoid *); \
    GLvoid         (GLAPIENTRY *DisableClientState)(_gcArgComma_ GLenum); \
    GLvoid         (GLAPIENTRY *DrawArrays) (_gcArgComma_ GLenum mode, GLint first, GLsizei count); \
    GLvoid         (GLAPIENTRY *DrawElements) (_gcArgComma_ GLenum mode, GLsizei count, GLenum type, const GLvoid* indices); \
    GLvoid         (GLAPIENTRY *EdgeFlagPointer)(_gcArgComma_ GLsizei, const GLboolean *); \
    GLvoid         (GLAPIENTRY *EnableClientState)(_gcArgComma_ GLenum); \
    GLvoid         (GLAPIENTRY *IndexPointer)(_gcArgComma_ GLenum, GLsizei, const GLvoid *); \
    GLvoid         (GLAPIENTRY *Indexub)(_gcArgComma_ GLubyte); \
    GLvoid         (GLAPIENTRY *Indexubv)(_gcArgComma_ const GLubyte *); \
    GLvoid         (GLAPIENTRY *InterleavedArrays)(_gcArgComma_ GLenum, GLsizei, const GLvoid *); \
    GLvoid         (GLAPIENTRY *NormalPointer)(_gcArgComma_ GLenum, GLsizei, const GLvoid *); \
    GLvoid         (GLAPIENTRY *PolygonOffset) (_gcArgComma_ GLfloat factor, GLfloat units); \
    GLvoid         (GLAPIENTRY *TexCoordPointer)(_gcArgComma_ GLint, GLenum, GLsizei, const GLvoid *); \
    GLvoid         (GLAPIENTRY *VertexPointer)(_gcArgComma_ GLint, GLenum, GLsizei, const GLvoid *); \
    GLboolean    (GLAPIENTRY *AreTexturesResident)(_gcArgComma_ GLsizei, const GLuint *, GLboolean *); \
    GLvoid         (GLAPIENTRY *CopyTexImage1D)(_gcArgComma_ GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLint); \
    GLvoid         (GLAPIENTRY *CopyTexImage2D) (_gcArgComma_ GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border); \
    GLvoid         (GLAPIENTRY *CopyTexSubImage1D)(_gcArgComma_ GLenum, GLint, GLint, GLint, GLint, GLsizei);    \
    GLvoid         (GLAPIENTRY *CopyTexSubImage2D) (_gcArgComma_ GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height); \
    GLvoid         (GLAPIENTRY *DeleteTextures) (_gcArgComma_ GLsizei n, const GLuint* textures); \
    GLvoid         (GLAPIENTRY *GenTextures) (_gcArgComma_ GLsizei n, GLuint* textures); \
    GLvoid         (GLAPIENTRY *GetPointerv) (_gcArgComma_ GLenum pname, GLvoid** params); \
    GLboolean      (GLAPIENTRY *IsTexture) (_gcArgComma_ GLuint texture); \
    GLvoid         (GLAPIENTRY *PrioritizeTextures)(_gcArgComma_ GLsizei, const GLuint *, const GLclampf *); \
    GLvoid         (GLAPIENTRY *TexSubImage1D)(_gcArgComma_  GLenum, GLint, GLint, GLsizei, GLenum, GLenum, const GLvoid *); \
    GLvoid         (GLAPIENTRY *TexSubImage2D) (_gcArgComma_ GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels); \
    GLvoid         (GLAPIENTRY *PopClientAttrib)(_gcArgOnly_); \
    GLvoid         (GLAPIENTRY *PushClientAttrib)(_gcArgComma_  GLbitfield); \
    GLvoid         (GLAPIENTRY *BlendColor) (_gcArgComma_ GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha); \
    GLvoid         (GLAPIENTRY *BlendEquation) (_gcArgComma_ GLenum mode); \
    GLvoid         (GLAPIENTRY *DrawRangeElements) (_gcArgComma_ GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices); \
    GLvoid         (GLAPIENTRY *ColorTable)(_gcArgComma_  GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *); \
    GLvoid         (GLAPIENTRY *ColorTableParameterfv)(_gcArgComma_  GLenum, GLenum, const GLfloat *); \
    GLvoid         (GLAPIENTRY *ColorTableParameteriv)(_gcArgComma_  GLenum, GLenum, const GLint *); \
    GLvoid         (GLAPIENTRY *CopyColorTable)(_gcArgComma_  GLenum, GLenum, GLint, GLint, GLsizei); \
    GLvoid         (GLAPIENTRY *GetColorTable)(_gcArgComma_  GLenum, GLenum, GLenum, GLvoid *); \
    GLvoid         (GLAPIENTRY *GetColorTableParameterfv)(_gcArgComma_  GLenum, GLenum, GLfloat *); \
    GLvoid         (GLAPIENTRY *GetColorTableParameteriv)(_gcArgComma_   GLenum, GLenum, GLint *); \
    GLvoid         (GLAPIENTRY *ColorSubTable)(_gcArgComma_  GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *); \
    GLvoid         (GLAPIENTRY *CopyColorSubTable)(_gcArgComma_  GLenum, GLsizei, GLint, GLint, GLsizei); \
    GLvoid         (GLAPIENTRY *ConvolutionFilter1D)(_gcArgComma_  GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *); \
    GLvoid         (GLAPIENTRY *ConvolutionFilter2D)(_gcArgComma_  GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *); \
    GLvoid         (GLAPIENTRY *ConvolutionParameterf)(_gcArgComma_  GLenum, GLenum, GLfloat); \
    GLvoid         (GLAPIENTRY *ConvolutionParameterfv)(_gcArgComma_  GLenum, GLenum, const GLfloat *); \
    GLvoid         (GLAPIENTRY *ConvolutionParameteri)(_gcArgComma_  GLenum, GLenum, GLint); \
    GLvoid         (GLAPIENTRY *ConvolutionParameteriv)(_gcArgComma_  GLenum, GLenum, const GLint *); \
    GLvoid         (GLAPIENTRY *CopyConvolutionFilter1D)(_gcArgComma_  GLenum, GLenum, GLint, GLint, GLsizei); \
    GLvoid         (GLAPIENTRY *CopyConvolutionFilter2D)(_gcArgComma_  GLenum, GLenum, GLint, GLint, GLsizei, GLsizei); \
    GLvoid         (GLAPIENTRY *GetConvolutionFilter)(_gcArgComma_  GLenum, GLenum, GLenum, GLvoid *); \
    GLvoid         (GLAPIENTRY *GetConvolutionParameterfv)(_gcArgComma_  GLenum, GLenum, GLfloat *); \
    GLvoid         (GLAPIENTRY *GetConvolutionParameteriv)(_gcArgComma_  GLenum, GLenum, GLint *); \
    GLvoid         (GLAPIENTRY *GetSeparableFilter)(_gcArgComma_  GLenum, GLenum, GLenum, GLvoid *, GLvoid *, GLvoid *); \
    GLvoid         (GLAPIENTRY *SeparableFilter2D)(_gcArgComma_  GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *, const GLvoid *); \
    GLvoid         (GLAPIENTRY *GetHistogram)(_gcArgComma_  GLenum, GLboolean, GLenum, GLenum, GLvoid *); \
    GLvoid         (GLAPIENTRY *GetHistogramParameterfv)(_gcArgComma_  GLenum, GLenum, GLfloat *); \
    GLvoid         (GLAPIENTRY *GetHistogramParameteriv)(_gcArgComma_  GLenum, GLenum, GLint *); \
    GLvoid         (GLAPIENTRY *GetMinmax)(_gcArgComma_  GLenum, GLboolean, GLenum, GLenum, GLvoid *); \
    GLvoid         (GLAPIENTRY *GetMinmaxParameterfv)(_gcArgComma_  GLenum, GLenum, GLfloat *); \
    GLvoid         (GLAPIENTRY *GetMinmaxParameteriv)(_gcArgComma_  GLenum, GLenum, GLint *); \
    GLvoid         (GLAPIENTRY *Histogram)(_gcArgComma_  GLenum, GLsizei, GLenum, GLboolean); \
    GLvoid         (GLAPIENTRY *Minmax)(_gcArgComma_  GLenum, GLenum, GLboolean); \
    GLvoid         (GLAPIENTRY *ResetHistogram)(_gcArgComma_  GLenum); \
    GLvoid         (GLAPIENTRY *ResetMinmax)(_gcArgComma_  GLenum); \
    GLvoid         (GLAPIENTRY *TexImage3D) (_gcArgComma_ GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels); \
    GLvoid         (GLAPIENTRY *TexSubImage3D) (_gcArgComma_ GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels); \
    GLvoid         (GLAPIENTRY *CopyTexSubImage3D) (_gcArgComma_ GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height); \
    GLvoid         (GLAPIENTRY *ActiveTexture) (_gcArgComma_ GLenum texture); \
    GLvoid         (GLAPIENTRY *ClientActiveTexture)(_gcArgComma_ GLenum); \
    GLvoid         (GLAPIENTRY *MultiTexCoord1d)(_gcArgComma_  GLenum, GLdouble); \
    GLvoid         (GLAPIENTRY *MultiTexCoord1dv)(_gcArgComma_ GLenum, const GLdouble *); \
    GLvoid         (GLAPIENTRY *MultiTexCoord1f)(_gcArgComma_ GLenum, GLfloat); \
    GLvoid         (GLAPIENTRY *MultiTexCoord1fv)(_gcArgComma_  GLenum, const GLfloat *); \
    GLvoid         (GLAPIENTRY *MultiTexCoord1i)(_gcArgComma_  GLenum, GLint); \
    GLvoid         (GLAPIENTRY *MultiTexCoord1iv)(_gcArgComma_ GLenum, const GLint *); \
    GLvoid         (GLAPIENTRY *MultiTexCoord1s)(_gcArgComma_ GLenum, GLshort); \
    GLvoid         (GLAPIENTRY *MultiTexCoord1sv)(_gcArgComma_  GLenum, const GLshort *); \
    GLvoid         (GLAPIENTRY *MultiTexCoord2d)(_gcArgComma_ GLenum, GLdouble, GLdouble); \
    GLvoid         (GLAPIENTRY *MultiTexCoord2dv)(_gcArgComma_  GLenum, const GLdouble *); \
    GLvoid         (GLAPIENTRY *MultiTexCoord2f)(_gcArgComma_ GLenum, GLfloat, GLfloat); \
    GLvoid         (GLAPIENTRY *MultiTexCoord2fv)(_gcArgComma_ GLenum, const GLfloat *); \
    GLvoid         (GLAPIENTRY *MultiTexCoord2i)(_gcArgComma_ GLenum, GLint, GLint); \
    GLvoid         (GLAPIENTRY *MultiTexCoord2iv)(_gcArgComma_ GLenum, const GLint *); \
    GLvoid         (GLAPIENTRY *MultiTexCoord2s)(_gcArgComma_ GLenum, GLshort, GLshort); \
    GLvoid         (GLAPIENTRY *MultiTexCoord2sv)(_gcArgComma_ GLenum, const GLshort *); \
    GLvoid         (GLAPIENTRY *MultiTexCoord3d)(_gcArgComma_ GLenum, GLdouble, GLdouble, GLdouble); \
    GLvoid         (GLAPIENTRY *MultiTexCoord3dv)(_gcArgComma_ GLenum, const GLdouble *); \
    GLvoid         (GLAPIENTRY *MultiTexCoord3f)(_gcArgComma_ GLenum, GLfloat, GLfloat, GLfloat); \
    GLvoid         (GLAPIENTRY *MultiTexCoord3fv)(_gcArgComma_  GLenum, const GLfloat *); \
    GLvoid         (GLAPIENTRY *MultiTexCoord3i)(_gcArgComma_ GLenum, GLint, GLint, GLint); \
    GLvoid         (GLAPIENTRY *MultiTexCoord3iv)(_gcArgComma_ GLenum, const GLint *); \
    GLvoid         (GLAPIENTRY *MultiTexCoord3s)(_gcArgComma_ GLenum, GLshort, GLshort, GLshort); \
    GLvoid         (GLAPIENTRY *MultiTexCoord3sv)(_gcArgComma_ GLenum, const GLshort *); \
    GLvoid         (GLAPIENTRY *MultiTexCoord4d)(_gcArgComma_ GLenum, GLdouble, GLdouble, GLdouble, GLdouble); \
    GLvoid         (GLAPIENTRY *MultiTexCoord4dv)(_gcArgComma_  GLenum, const GLdouble *); \
    GLvoid         (GLAPIENTRY *MultiTexCoord4f)(_gcArgComma_ GLenum, GLfloat, GLfloat, GLfloat, GLfloat); \
    GLvoid         (GLAPIENTRY *MultiTexCoord4fv)(_gcArgComma_  GLenum, const GLfloat *); \
    GLvoid         (GLAPIENTRY *MultiTexCoord4i)(_gcArgComma_ GLenum, GLint, GLint, GLint, GLint); \
    GLvoid         (GLAPIENTRY *MultiTexCoord4iv)(_gcArgComma_ GLenum, const GLint *); \
    GLvoid         (GLAPIENTRY *MultiTexCoord4s)(_gcArgComma_ GLenum, GLshort, GLshort, GLshort, GLshort); \
    GLvoid         (GLAPIENTRY *MultiTexCoord4sv)(_gcArgComma_ GLenum, const GLshort *); \
    GLvoid         (GLAPIENTRY *LoadTransposeMatrixf)(_gcArgComma_ const GLfloat *); \
    GLvoid         (GLAPIENTRY *LoadTransposeMatrixd)(_gcArgComma_  const GLdouble *); \
    GLvoid         (GLAPIENTRY *MultTransposeMatrixf)(_gcArgComma_ const GLfloat *); \
    GLvoid         (GLAPIENTRY *MultTransposeMatrixd)(_gcArgComma_ const GLdouble *); \
    GLvoid         (GLAPIENTRY *SampleCoverage) (_gcArgComma_ GLfloat value, GLboolean invert); \
    GLvoid         (GLAPIENTRY *CompressedTexImage3D) (_gcArgComma_ GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data); \
    GLvoid         (GLAPIENTRY *CompressedTexImage2D) (_gcArgComma_ GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data); \
    GLvoid         (GLAPIENTRY *CompressedTexImage1D)(_gcArgComma_ GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const GLvoid *); \
    GLvoid         (GLAPIENTRY *CompressedTexSubImage3D) (_gcArgComma_ GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data); \
    GLvoid         (GLAPIENTRY *CompressedTexSubImage2D) (_gcArgComma_ GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data); \
    GLvoid         (GLAPIENTRY *CompressedTexSubImage1D)(_gcArgComma_ GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const GLvoid *); \
    GLvoid         (GLAPIENTRY *GetCompressedTexImage)(_gcArgComma_ GLenum, GLint, GLvoid *); \
    GLvoid         (GLAPIENTRY *BlendFuncSeparate) (_gcArgComma_ GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha); \
    GLvoid         (GLAPIENTRY *MultiDrawArrays) (_gcArgComma_ GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount); \
    GLvoid         (GLAPIENTRY *MultiDrawElements) (_gcArgComma_ GLenum mode, const GLsizei *count, GLenum type, const GLvoid*const*indices, GLsizei primcount); \
    GLvoid         (GLAPIENTRY *FogCoordf)(_gcArgComma_  GLfloat); \
    GLvoid         (GLAPIENTRY *FogCoordfv)(_gcArgComma_  const GLfloat *); \
    GLvoid         (GLAPIENTRY *FogCoordd)(_gcArgComma_ GLdouble); \
    GLvoid         (GLAPIENTRY *FogCoorddv)(_gcArgComma_ const GLdouble *); \
    GLvoid         (GLAPIENTRY *FogCoordPointer)(_gcArgComma_  GLenum, GLsizei, const GLvoid *); \
    GLvoid         (GLAPIENTRY *MultiDrawArraysEXT) (_gcArgComma_ GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount); \
    GLvoid         (GLAPIENTRY *MultiDrawElementsEXT) (_gcArgComma_ GLenum mode, const GLsizei *count, GLenum type, const GLvoid*const*indices, GLsizei primcount); \
    GLvoid         (GLAPIENTRY *PointParameterf)(_gcArgComma_  GLenum, GLfloat); \
    GLvoid         (GLAPIENTRY *PointParameterfv)(_gcArgComma_  GLenum, const GLfloat *); \
    GLvoid         (GLAPIENTRY *PointParameteri)(_gcArgComma_  GLenum, GLint); \
    GLvoid         (GLAPIENTRY *PointParameteriv)(_gcArgComma_  GLenum, const GLint *); \
    GLvoid         (GLAPIENTRY *SecondaryColor3b)(_gcArgComma_  GLbyte, GLbyte, GLbyte); \
    GLvoid         (GLAPIENTRY *SecondaryColor3bv)(_gcArgComma_  const GLbyte *); \
    GLvoid         (GLAPIENTRY *SecondaryColor3d)(_gcArgComma_  GLdouble, GLdouble, GLdouble); \
    GLvoid         (GLAPIENTRY *SecondaryColor3dv)(_gcArgComma_  const GLdouble *); \
    GLvoid         (GLAPIENTRY *SecondaryColor3f)(_gcArgComma_  GLfloat, GLfloat, GLfloat); \
    GLvoid         (GLAPIENTRY *SecondaryColor3fv)(_gcArgComma_  const GLfloat *); \
    GLvoid         (GLAPIENTRY *SecondaryColor3i)(_gcArgComma_  GLint, GLint, GLint); \
    GLvoid         (GLAPIENTRY *SecondaryColor3iv)(_gcArgComma_  const GLint *); \
    GLvoid         (GLAPIENTRY *SecondaryColor3s)(_gcArgComma_  GLshort, GLshort, GLshort); \
    GLvoid         (GLAPIENTRY *SecondaryColor3sv)(_gcArgComma_  const GLshort *); \
    GLvoid         (GLAPIENTRY *SecondaryColor3ub)(_gcArgComma_  GLubyte, GLubyte, GLubyte); \
    GLvoid         (GLAPIENTRY *SecondaryColor3ubv)(_gcArgComma_ const GLubyte *); \
    GLvoid         (GLAPIENTRY *SecondaryColor3ui)(_gcArgComma_  GLuint, GLuint, GLuint); \
    GLvoid         (GLAPIENTRY *SecondaryColor3uiv)(_gcArgComma_  const GLuint *); \
    GLvoid         (GLAPIENTRY *SecondaryColor3us)(_gcArgComma_ GLushort, GLushort, GLushort); \
    GLvoid         (GLAPIENTRY *SecondaryColor3usv)(_gcArgComma_  const GLushort *); \
    GLvoid         (GLAPIENTRY *SecondaryColorPointer)(_gcArgComma_  GLint, GLenum, GLsizei, const GLvoid *); \
    GLvoid         (GLAPIENTRY *WindowPos2d)(_gcArgComma_  GLdouble, GLdouble); \
    GLvoid         (GLAPIENTRY *WindowPos2dv)(_gcArgComma_  const GLdouble *); \
    GLvoid         (GLAPIENTRY *WindowPos2f)(_gcArgComma_  GLfloat, GLfloat); \
    GLvoid         (GLAPIENTRY *WindowPos2fv)(_gcArgComma_  const GLfloat *); \
    GLvoid         (GLAPIENTRY *WindowPos2i)(_gcArgComma_  GLint, GLint); \
    GLvoid         (GLAPIENTRY *WindowPos2iv)(_gcArgComma_  const GLint *); \
    GLvoid         (GLAPIENTRY *WindowPos2s)(_gcArgComma_ GLshort, GLshort); \
    GLvoid         (GLAPIENTRY *WindowPos2sv)(_gcArgComma_  const GLshort *); \
    GLvoid         (GLAPIENTRY *WindowPos3d)(_gcArgComma_  GLdouble, GLdouble, GLdouble); \
    GLvoid         (GLAPIENTRY *WindowPos3dv)(_gcArgComma_  const GLdouble *); \
    GLvoid         (GLAPIENTRY *WindowPos3f)(_gcArgComma_  GLfloat, GLfloat, GLfloat); \
    GLvoid         (GLAPIENTRY *WindowPos3fv)(_gcArgComma_  const GLfloat *); \
    GLvoid         (GLAPIENTRY *WindowPos3i)(_gcArgComma_ GLint, GLint, GLint); \
    GLvoid         (GLAPIENTRY *WindowPos3iv)(_gcArgComma_ const GLint *); \
    GLvoid         (GLAPIENTRY *WindowPos3s)(_gcArgComma_  GLshort, GLshort, GLshort); \
    GLvoid         (GLAPIENTRY *WindowPos3sv)(_gcArgComma_  const GLshort *); \
    GLvoid         (GLAPIENTRY *GenQueries) (_gcArgComma_ GLsizei n, GLuint* ids); \
    GLvoid         (GLAPIENTRY *DeleteQueries) (_gcArgComma_ GLsizei n, const GLuint* ids); \
    GLboolean      (GLAPIENTRY *IsQuery) (_gcArgComma_ GLuint id); \
    GLvoid         (GLAPIENTRY *BeginQuery) (_gcArgComma_ GLenum target, GLuint id); \
    GLvoid         (GLAPIENTRY *EndQuery) (_gcArgComma_ GLenum target); \
    GLvoid         (GLAPIENTRY *GetQueryiv) (_gcArgComma_ GLenum target, GLenum pname, GLint* params); \
    GLvoid         (GLAPIENTRY *GetQueryObjectiv) (_gcArgComma_ GLuint id, GLenum pname, GLint* params); \
    GLvoid         (GLAPIENTRY *GetQueryObjectuiv) (_gcArgComma_ GLuint id, GLenum pname, GLuint* params); \
    GLvoid         (GLAPIENTRY *BindBuffer) (_gcArgComma_ GLenum target, GLuint buffer); \
    GLvoid         (GLAPIENTRY *DeleteBuffers) (_gcArgComma_ GLsizei n, const GLuint* buffers); \
    GLvoid         (GLAPIENTRY *GenBuffers) (_gcArgComma_ GLsizei n, GLuint* buffers); \
    GLboolean      (GLAPIENTRY *IsBuffer) (_gcArgComma_ GLuint buffer); \
    GLvoid         (GLAPIENTRY *BufferData) (_gcArgComma_ GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage); \
    GLvoid         (GLAPIENTRY *BufferSubData) (_gcArgComma_ GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data); \
    GLvoid         (GLAPIENTRY *GetBufferSubData) (_gcArgComma_ GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data); \
    GLvoid*        (GLAPIENTRY *MapBufferOES) (_gcArgComma_ GLenum target, GLenum access _retPointer); \
    GLboolean      (GLAPIENTRY *UnmapBufferOES) (_gcArgComma_ GLenum target); \
    GLvoid*        (GLAPIENTRY *MapBuffer) (_gcArgComma_ GLenum target, GLenum access); \
    GLvoid         (GLAPIENTRY *GetBufferParameteriv) (_gcArgComma_ GLenum target, GLenum pname, GLint* params); \
    GLvoid         (GLAPIENTRY *GetBufferPointerv) (_gcArgComma_ GLenum target, GLenum pname, GLvoid** params); \
    GLvoid         (GLAPIENTRY *BlendEquationSeparate) (_gcArgComma_ GLenum modeRGB, GLenum modeAlpha); \
    GLvoid         (GLAPIENTRY *DrawBuffers) (_gcArgComma_ GLsizei n, const GLenum* bufs); \
    GLvoid         (GLAPIENTRY *StencilOpSeparate) (_gcArgComma_ GLenum face, GLenum fail, GLenum zfail, GLenum zpass); \
    GLvoid         (GLAPIENTRY *StencilFuncSeparate) (_gcArgComma_ GLenum face, GLenum func, GLint ref, GLuint mask); \
    GLvoid         (GLAPIENTRY *StencilMaskSeparate) (_gcArgComma_ GLenum face, GLuint mask); \
    GLvoid         (GLAPIENTRY *AttachShader) (_gcArgComma_ GLuint program, GLuint shader); \
    GLvoid         (GLAPIENTRY *BindAttribLocation) (_gcArgComma_ GLuint program, GLuint index, const GLchar* name); \
    GLvoid         (GLAPIENTRY *CompileShader) (_gcArgComma_ GLuint shader); \
    GLuint         (GLAPIENTRY *CreateProgram) (_gcArgOnly_ _retProgram_); \
    GLuint         (GLAPIENTRY *CreateShader) (_gcArgComma_ GLenum type _retShader_); \
    GLvoid         (GLAPIENTRY *DeleteProgram) (_gcArgComma_ GLuint program); \
    GLvoid         (GLAPIENTRY *DeleteShader) (_gcArgComma_ GLuint shader); \
    GLvoid         (GLAPIENTRY *DetachShader) (_gcArgComma_ GLuint program, GLuint shader); \
    GLvoid         (GLAPIENTRY *DisableVertexAttribArray) (_gcArgComma_ GLuint index); \
    GLvoid         (GLAPIENTRY *EnableVertexAttribArray) (_gcArgComma_ GLuint index); \
    GLvoid         (GLAPIENTRY *GetActiveAttrib) (_gcArgComma_ GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name); \
    GLvoid         (GLAPIENTRY *GetActiveUniform) (_gcArgComma_ GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name); \
    GLvoid         (GLAPIENTRY *GetAttachedShaders) (_gcArgComma_ GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders); \
    GLint          (GLAPIENTRY *GetAttribLocation) (_gcArgComma_ GLuint program, const GLchar* name _retLocation_); \
    GLvoid         (GLAPIENTRY *GetProgramiv) (_gcArgComma_ GLuint program, GLenum pname, GLint* params); \
    GLvoid         (GLAPIENTRY *GetProgramInfoLog) (_gcArgComma_ GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog); \
    GLvoid         (GLAPIENTRY *GetShaderiv) (_gcArgComma_ GLuint shader, GLenum pname, GLint* params); \
    GLvoid         (GLAPIENTRY *GetShaderInfoLog) (_gcArgComma_ GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog); \
    GLvoid         (GLAPIENTRY *GetShaderSource) (_gcArgComma_ GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source); \
    GLint          (GLAPIENTRY *GetUniformLocation) (_gcArgComma_ GLuint program, const GLchar* name _retLocation_); \
    GLvoid         (GLAPIENTRY *GetUniformfv) (_gcArgComma_ GLuint program, GLint location, GLfloat* params); \
    GLvoid         (GLAPIENTRY *GetUniformiv) (_gcArgComma_ GLuint program, GLint location, GLint* params); \
    GLvoid         (GLAPIENTRY *GetVertexAttribdv) (_gcArgComma_ GLuint index, GLenum pname, GLdouble* params); \
    GLvoid         (GLAPIENTRY *GetVertexAttribfv) (_gcArgComma_ GLuint index, GLenum pname, GLfloat* params); \
    GLvoid         (GLAPIENTRY *GetVertexAttribiv) (_gcArgComma_ GLuint index, GLenum pname, GLint* params); \
    GLvoid         (GLAPIENTRY *GetVertexAttribPointerv) (_gcArgComma_ GLuint index, GLenum pname, GLvoid** pointer); \
    GLboolean      (GLAPIENTRY *IsProgram) (_gcArgComma_ GLuint program); \
    GLboolean      (GLAPIENTRY *IsShader) (_gcArgComma_ GLuint shader); \
    GLvoid         (GLAPIENTRY *LinkProgram) (_gcArgComma_ GLuint program); \
    GLvoid         (GLAPIENTRY *ShaderSource) (_gcArgComma_ GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length); \
    GLvoid         (GLAPIENTRY *UseProgram) (_gcArgComma_ GLuint program); \
    GLvoid         (GLAPIENTRY *Uniform1f) (_gcArgComma_ GLint location, GLfloat x); \
    GLvoid         (GLAPIENTRY *Uniform1fv) (_gcArgComma_ GLint location, GLsizei count, const GLfloat* v); \
    GLvoid         (GLAPIENTRY *Uniform1i) (_gcArgComma_ GLint location, GLint x); \
    GLvoid         (GLAPIENTRY *Uniform1iv) (_gcArgComma_ GLint location, GLsizei count, const GLint* v); \
    GLvoid         (GLAPIENTRY *Uniform2f) (_gcArgComma_ GLint location, GLfloat x, GLfloat y); \
    GLvoid         (GLAPIENTRY *Uniform2fv) (_gcArgComma_ GLint location, GLsizei count, const GLfloat* v); \
    GLvoid         (GLAPIENTRY *Uniform2i) (_gcArgComma_ GLint location, GLint x, GLint y); \
    GLvoid         (GLAPIENTRY *Uniform2iv) (_gcArgComma_ GLint location, GLsizei count, const GLint* v); \
    GLvoid         (GLAPIENTRY *Uniform3f) (_gcArgComma_ GLint location, GLfloat x, GLfloat y, GLfloat z); \
    GLvoid         (GLAPIENTRY *Uniform3fv) (_gcArgComma_ GLint location, GLsizei count, const GLfloat* v); \
    GLvoid         (GLAPIENTRY *Uniform3i) (_gcArgComma_ GLint location, GLint x, GLint y, GLint z); \
    GLvoid         (GLAPIENTRY *Uniform3iv) (_gcArgComma_ GLint location, GLsizei count, const GLint* v); \
    GLvoid         (GLAPIENTRY *Uniform4f) (_gcArgComma_ GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w); \
    GLvoid         (GLAPIENTRY *Uniform4fv) (_gcArgComma_ GLint location, GLsizei count, const GLfloat* v); \
    GLvoid         (GLAPIENTRY *Uniform4i) (_gcArgComma_ GLint location, GLint x, GLint y, GLint z, GLint w); \
    GLvoid         (GLAPIENTRY *Uniform4iv) (_gcArgComma_ GLint location, GLsizei count, const GLint* v); \
    GLvoid         (GLAPIENTRY *UniformMatrix2fv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLfloat* value); \
    GLvoid         (GLAPIENTRY *UniformMatrix3fv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLfloat* value); \
    GLvoid         (GLAPIENTRY *UniformMatrix4fv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLfloat* value); \
    GLvoid         (GLAPIENTRY *ValidateProgram) (_gcArgComma_ GLuint program); \
    GLvoid         (GLAPIENTRY *VertexAttrib1d) (_gcArgComma_ GLuint indx, GLdouble x); \
    GLvoid         (GLAPIENTRY *VertexAttrib1dv) (_gcArgComma_ GLuint indx, const GLdouble *values); \
    GLvoid         (GLAPIENTRY *VertexAttrib1f) (_gcArgComma_ GLuint indx, GLfloat x); \
    GLvoid         (GLAPIENTRY *VertexAttrib1fv) (_gcArgComma_ GLuint indx, const GLfloat * values); \
    GLvoid         (GLAPIENTRY *VertexAttrib1s) (_gcArgComma_ GLuint indx, GLshort x); \
    GLvoid         (GLAPIENTRY *VertexAttrib1sv) (_gcArgComma_ GLuint indx, const GLshort * values); \
    GLvoid         (GLAPIENTRY *VertexAttrib2d) (_gcArgComma_ GLuint indx, GLdouble x, GLdouble y); \
    GLvoid         (GLAPIENTRY *VertexAttrib2dv) (_gcArgComma_ GLuint indx, const GLdouble * values); \
    GLvoid         (GLAPIENTRY *VertexAttrib2f) (_gcArgComma_ GLuint indx, GLfloat x, GLfloat y); \
    GLvoid         (GLAPIENTRY *VertexAttrib2fv) (_gcArgComma_ GLuint indx, const GLfloat * values); \
    GLvoid         (GLAPIENTRY *VertexAttrib2s) (_gcArgComma_ GLuint indx, GLshort x, GLshort y); \
    GLvoid         (GLAPIENTRY *VertexAttrib2sv) (_gcArgComma_ GLuint indx, const GLshort * values); \
    GLvoid         (GLAPIENTRY *VertexAttrib3d) (_gcArgComma_ GLuint indx, GLdouble x, GLdouble y, GLdouble z); \
    GLvoid         (GLAPIENTRY *VertexAttrib3dv) (_gcArgComma_ GLuint indx, const GLdouble * values); \
    GLvoid         (GLAPIENTRY *VertexAttrib3f) (_gcArgComma_ GLuint indx, GLfloat x, GLfloat y, GLfloat z); \
    GLvoid         (GLAPIENTRY *VertexAttrib3fv) (_gcArgComma_ GLuint indx, const GLfloat * values); \
    GLvoid         (GLAPIENTRY *VertexAttrib3s) (_gcArgComma_ GLuint indx, GLshort x, GLshort y, GLshort z); \
    GLvoid         (GLAPIENTRY *VertexAttrib3sv) (_gcArgComma_ GLuint indx, const GLshort * values); \
    GLvoid         (GLAPIENTRY *VertexAttrib4Nbv) (_gcArgComma_ GLuint indx, const GLbyte * values); \
    GLvoid         (GLAPIENTRY *VertexAttrib4Niv) (_gcArgComma_ GLuint indx, const GLint * values); \
    GLvoid         (GLAPIENTRY *VertexAttrib4Nsv) (_gcArgComma_ GLuint indx, const GLshort * values); \
    GLvoid         (GLAPIENTRY *VertexAttrib4Nub) (_gcArgComma_ GLuint indx, GLubyte x, GLubyte y, GLubyte z, GLubyte w); \
    GLvoid         (GLAPIENTRY *VertexAttrib4Nubv) (_gcArgComma_ GLuint indx, const GLubyte * values); \
    GLvoid         (GLAPIENTRY *VertexAttrib4Nuiv) (_gcArgComma_ GLuint indx, const GLuint * values); \
    GLvoid         (GLAPIENTRY *VertexAttrib4Nusv) (_gcArgComma_ GLuint indx, const GLushort * values); \
    GLvoid         (GLAPIENTRY *VertexAttrib4bv) (_gcArgComma_ GLuint indx, const GLbyte * values); \
    GLvoid         (GLAPIENTRY *VertexAttrib4d) (_gcArgComma_ GLuint indx, GLdouble x, GLdouble y, GLdouble z, GLdouble w); \
    GLvoid         (GLAPIENTRY *VertexAttrib4dv) (_gcArgComma_ GLuint indx, const GLdouble * values); \
    GLvoid         (GLAPIENTRY *VertexAttrib4f) (_gcArgComma_ GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w); \
    GLvoid         (GLAPIENTRY *VertexAttrib4fv) (_gcArgComma_ GLuint indx, const GLfloat * values); \
    GLvoid         (GLAPIENTRY *VertexAttrib4iv) (_gcArgComma_ GLuint indx, const GLint * values); \
    GLvoid         (GLAPIENTRY *VertexAttrib4s) (_gcArgComma_ GLuint indx, GLshort x, GLshort y, GLshort z, GLshort w); \
    GLvoid         (GLAPIENTRY *VertexAttrib4sv) (_gcArgComma_ GLuint indx, const GLshort * values); \
    GLvoid         (GLAPIENTRY *VertexAttrib4ubv) (_gcArgComma_ GLuint indx, const GLubyte * values); \
    GLvoid         (GLAPIENTRY *VertexAttrib4uiv) (_gcArgComma_ GLuint indx, const GLuint * values); \
    GLvoid         (GLAPIENTRY *VertexAttrib4usv) (_gcArgComma_ GLuint indx, const GLushort * values);\
    GLvoid         (GLAPIENTRY *VertexAttribPointer) (_gcArgComma_ GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr); \
    GLvoid         (GLAPIENTRY *BindFramebuffer) (_gcArgComma_ GLenum target, GLuint framebuffer); \
    GLvoid         (GLAPIENTRY *BindRenderbuffer) (_gcArgComma_ GLenum target, GLuint renderbuffer); \
    GLenum         (GLAPIENTRY *CheckFramebufferStatus) (_gcArgComma_ GLenum target); \
    GLvoid         (GLAPIENTRY *ClearDepthf) (_gcArgComma_ GLfloat depth); \
    GLvoid         (GLAPIENTRY *DeleteFramebuffers) (_gcArgComma_ GLsizei n, const GLuint* framebuffers); \
    GLvoid         (GLAPIENTRY *DeleteRenderbuffers) (_gcArgComma_ GLsizei n, const GLuint* renderbuffers); \
    GLvoid         (GLAPIENTRY *DepthRangef) (_gcArgComma_ GLfloat n, GLfloat f); \
    GLvoid         (GLAPIENTRY *FramebufferRenderbuffer) (_gcArgComma_ GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer); \
    GLvoid         (GLAPIENTRY *FramebufferTexture2D) (_gcArgComma_ GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level); \
    GLvoid         (GLAPIENTRY *GenerateMipmap) (_gcArgComma_ GLenum target); \
    GLvoid         (GLAPIENTRY *GenFramebuffers) (_gcArgComma_ GLsizei n, GLuint* framebuffers); \
    GLvoid         (GLAPIENTRY *GenRenderbuffers) (_gcArgComma_ GLsizei n, GLuint* renderbuffers); \
    GLvoid         (GLAPIENTRY *GetFramebufferAttachmentParameteriv) (_gcArgComma_ GLenum target, GLenum attachment, GLenum pname, GLint* params); \
    GLvoid         (GLAPIENTRY *GetRenderbufferParameteriv) (_gcArgComma_ GLenum target, GLenum pname, GLint* params); \
    GLvoid         (GLAPIENTRY *GetShaderPrecisionFormat) (_gcArgComma_ GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision); \
    GLboolean      (GLAPIENTRY *IsFramebuffer) (_gcArgComma_ GLuint framebuffer); \
    GLboolean      (GLAPIENTRY *IsRenderbuffer) (_gcArgComma_ GLuint renderbuffer); \
    GLvoid         (GLAPIENTRY *ReleaseShaderCompiler) (_gcArgOnly_); \
    GLvoid         (GLAPIENTRY *RenderbufferStorage) (_gcArgComma_ GLenum target, GLenum internalformat, GLsizei width, GLsizei height); \
    GLvoid         (GLAPIENTRY *ShaderBinary) (_gcArgComma_ GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length); \
    /* OpenGL ES 3.0 */ \
    GLboolean      (GLAPIENTRY *UnmapBuffer) (_gcArgComma_ GLenum target); \
    GLvoid         (GLAPIENTRY *UniformMatrix2x3fv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLfloat* value); \
    GLvoid         (GLAPIENTRY *UniformMatrix3x2fv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLfloat* value); \
    GLvoid         (GLAPIENTRY *UniformMatrix2x4fv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLfloat* value); \
    GLvoid         (GLAPIENTRY *UniformMatrix4x2fv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLfloat* value); \
    GLvoid         (GLAPIENTRY *UniformMatrix3x4fv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLfloat* value); \
    GLvoid         (GLAPIENTRY *UniformMatrix4x3fv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLfloat* value); \
    GLvoid         (GLAPIENTRY *BlitFramebuffer) (_gcArgComma_ GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter); \
    GLvoid         (GLAPIENTRY *RenderbufferStorageMultisample) (_gcArgComma_ GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height); \
    GLvoid         (GLAPIENTRY *FramebufferTextureLayer) (_gcArgComma_ GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer); \
    GLvoid*        (GLAPIENTRY *MapBufferRange) (_gcArgComma_ GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access _retPointer); \
    GLvoid         (GLAPIENTRY *FlushMappedBufferRange) (_gcArgComma_ GLenum target, GLintptr offset, GLsizeiptr length); \
    GLvoid         (GLAPIENTRY *BindVertexArray) (_gcArgComma_ GLuint array); \
    GLvoid         (GLAPIENTRY *DeleteVertexArrays) (_gcArgComma_ GLsizei n, const GLuint* arrays); \
    GLvoid         (GLAPIENTRY *GenVertexArrays) (_gcArgComma_ GLsizei n, GLuint* arrays); \
    GLboolean      (GLAPIENTRY *IsVertexArray) (_gcArgComma_ GLuint array); \
    GLvoid         (GLAPIENTRY *GetIntegeri_v) (_gcArgComma_ GLenum target, GLuint index, GLint* data); \
    GLvoid         (GLAPIENTRY *BeginTransformFeedback) (_gcArgComma_ GLenum primitiveMode); \
    GLvoid         (GLAPIENTRY *EndTransformFeedback) (_gcArgOnly_); \
    GLvoid         (GLAPIENTRY *BindBufferRange) (_gcArgComma_ GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size); \
    GLvoid         (GLAPIENTRY *BindBufferBase) (_gcArgComma_ GLenum target, GLuint index, GLuint buffer); \
    GLvoid         (GLAPIENTRY *TransformFeedbackVaryings) (_gcArgComma_ GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode); \
    GLvoid         (GLAPIENTRY *GetTransformFeedbackVarying) (_gcArgComma_ GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name); \
    GLvoid         (GLAPIENTRY *VertexAttribIPointer) (_gcArgComma_ GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer); \
    GLvoid         (GLAPIENTRY *GetVertexAttribIiv) (_gcArgComma_ GLuint index, GLenum pname, GLint* params); \
    GLvoid         (GLAPIENTRY *GetVertexAttribIuiv) (_gcArgComma_ GLuint index, GLenum pname, GLuint* params); \
    GLvoid         (GLAPIENTRY *VertexAttribI4i) (_gcArgComma_ GLuint index, GLint x, GLint y, GLint z, GLint w); \
    GLvoid         (GLAPIENTRY *VertexAttribI4ui) (_gcArgComma_ GLuint index, GLuint x, GLuint y, GLuint z, GLuint w); \
    GLvoid         (GLAPIENTRY *VertexAttribI4iv) (_gcArgComma_ GLuint index, const GLint* v); \
    GLvoid         (GLAPIENTRY *VertexAttribI4uiv) (_gcArgComma_ GLuint index, const GLuint* v); \
    GLvoid         (GLAPIENTRY *GetUniformuiv) (_gcArgComma_ GLuint program, GLint location, GLuint* params); \
    GLint           (GLAPIENTRY *GetFragDataLocation) (_gcArgComma_ GLuint program, const GLchar *name _retLocation_); \
    GLvoid         (GLAPIENTRY *BindFragDataLocation) (_gcArgComma_ GLuint program, GLuint colorNumber, const GLchar *name); \
    GLvoid         (GLAPIENTRY *Uniform1ui) (_gcArgComma_ GLint location, GLuint v0); \
    GLvoid         (GLAPIENTRY *Uniform2ui) (_gcArgComma_ GLint location, GLuint v0, GLuint v1); \
    GLvoid         (GLAPIENTRY *Uniform3ui) (_gcArgComma_ GLint location, GLuint v0, GLuint v1, GLuint v2); \
    GLvoid         (GLAPIENTRY *Uniform4ui) (_gcArgComma_ GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3); \
    GLvoid         (GLAPIENTRY *Uniform1uiv) (_gcArgComma_ GLint location, GLsizei count, const GLuint* value); \
    GLvoid         (GLAPIENTRY *Uniform2uiv) (_gcArgComma_ GLint location, GLsizei count, const GLuint* value); \
    GLvoid         (GLAPIENTRY *Uniform3uiv) (_gcArgComma_ GLint location, GLsizei count, const GLuint* value); \
    GLvoid         (GLAPIENTRY *Uniform4uiv) (_gcArgComma_ GLint location, GLsizei count, const GLuint* value); \
    GLvoid         (GLAPIENTRY *ClearBufferiv) (_gcArgComma_ GLenum buffer, GLint drawbuffer, const GLint* value); \
    GLvoid         (GLAPIENTRY *ClearBufferuiv) (_gcArgComma_ GLenum buffer, GLint drawbuffer, const GLuint* value); \
    GLvoid         (GLAPIENTRY *ClearBufferfv) (_gcArgComma_ GLenum buffer, GLint drawbuffer, const GLfloat* value); \
    GLvoid         (GLAPIENTRY *ClearBufferfi) (_gcArgComma_ GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil); \
    const GLubyte* (GLAPIENTRY *GetStringi) (_gcArgComma_ GLenum name, GLuint index); \
    GLvoid         (GLAPIENTRY *CopyBufferSubData) (_gcArgComma_ GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size); \
    GLvoid         (GLAPIENTRY *GetUniformIndices) (_gcArgComma_ GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices); \
    GLvoid         (GLAPIENTRY *GetActiveUniformsiv) (_gcArgComma_ GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params); \
    GLuint         (GLAPIENTRY *GetUniformBlockIndex) (_gcArgComma_ GLuint program, const GLchar* uniformBlockName _retIndex_); \
    GLvoid         (GLAPIENTRY *GetActiveUniformBlockiv) (_gcArgComma_ GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params); \
    GLvoid         (GLAPIENTRY *GetActiveUniformBlockName) (_gcArgComma_ GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName); \
    GLvoid         (GLAPIENTRY *UniformBlockBinding) (_gcArgComma_ GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding); \
    GLvoid         (GLAPIENTRY *DrawArraysInstanced) (_gcArgComma_ GLenum mode, GLint first, GLsizei count, GLsizei instanceCount); \
    GLvoid         (GLAPIENTRY *DrawElementsInstanced) (_gcArgComma_ GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount); \
    GLsync         (GLAPIENTRY *FenceSync) (_gcArgComma_ GLenum condition, GLbitfield flags _retSync_); \
    GLboolean      (GLAPIENTRY *IsSync) (_gcArgComma_ GLsync sync); \
    GLvoid         (GLAPIENTRY *DeleteSync) (_gcArgComma_ GLsync sync); \
    GLenum         (GLAPIENTRY *ClientWaitSync) (_gcArgComma_ GLsync sync, GLbitfield flags, GLuint64 timeout); \
    GLvoid         (GLAPIENTRY *WaitSync) (_gcArgComma_ GLsync sync, GLbitfield flags, GLuint64 timeout); \
    GLvoid         (GLAPIENTRY *GetInteger64v) (_gcArgComma_ GLenum pname, GLint64* params); \
    GLvoid         (GLAPIENTRY *GetSynciv) (_gcArgComma_ GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values); \
    GLvoid         (GLAPIENTRY *GetInteger64i_v) (_gcArgComma_ GLenum target, GLuint index, GLint64* data); \
    GLvoid         (GLAPIENTRY *GetBufferParameteri64v) (_gcArgComma_ GLenum target, GLenum pname, GLint64* params); \
    GLvoid         (GLAPIENTRY *GenSamplers) (_gcArgComma_ GLsizei count, GLuint* samplers); \
    GLvoid         (GLAPIENTRY *DeleteSamplers) (_gcArgComma_ GLsizei count, const GLuint* samplers); \
    GLboolean      (GLAPIENTRY *IsSampler) (_gcArgComma_ GLuint sampler); \
    GLvoid         (GLAPIENTRY *BindSampler) (_gcArgComma_ GLuint unit, GLuint sampler); \
    GLvoid         (GLAPIENTRY *SamplerParameteri) (_gcArgComma_ GLuint sampler, GLenum pname, GLint param); \
    GLvoid         (GLAPIENTRY *SamplerParameteriv) (_gcArgComma_ GLuint sampler, GLenum pname, const GLint* param); \
    GLvoid         (GLAPIENTRY *SamplerParameterf) (_gcArgComma_ GLuint sampler, GLenum pname, GLfloat param); \
    GLvoid         (GLAPIENTRY *SamplerParameterfv) (_gcArgComma_ GLuint sampler, GLenum pname, const GLfloat* param); \
    GLvoid         (GLAPIENTRY *GetSamplerParameteriv) (_gcArgComma_ GLuint sampler, GLenum pname, GLint* params); \
    GLvoid         (GLAPIENTRY *GetSamplerParameterfv) (_gcArgComma_ GLuint sampler, GLenum pname, GLfloat* params); \
    GLvoid         (GLAPIENTRY *VertexAttribDivisor) (_gcArgComma_ GLuint index, GLuint divisor); \
    GLvoid         (GLAPIENTRY *BindTransformFeedback) (_gcArgComma_ GLenum target, GLuint id); \
    GLvoid         (GLAPIENTRY *DeleteTransformFeedbacks) (_gcArgComma_ GLsizei n, const GLuint* ids); \
    GLvoid         (GLAPIENTRY *GenTransformFeedbacks) (_gcArgComma_ GLsizei n, GLuint* ids); \
    GLboolean      (GLAPIENTRY *IsTransformFeedback) (_gcArgComma_ GLuint id); \
    GLvoid         (GLAPIENTRY *PauseTransformFeedback) (_gcArgOnly_); \
    GLvoid         (GLAPIENTRY *ResumeTransformFeedback) (_gcArgOnly_); \
    GLvoid         (GLAPIENTRY *GetProgramBinary) (_gcArgComma_ GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary); \
    GLvoid         (GLAPIENTRY *ProgramBinary) (_gcArgComma_ GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length); \
    GLvoid         (GLAPIENTRY *ProgramParameteri) (_gcArgComma_ GLuint program, GLenum pname, GLint value); \
    GLvoid         (GLAPIENTRY *InvalidateFramebuffer) (_gcArgComma_ GLenum target, GLsizei numAttachments, const GLenum* attachments); \
    GLvoid         (GLAPIENTRY *InvalidateSubFramebuffer) (_gcArgComma_ GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height); \
    GLvoid         (GLAPIENTRY *TexStorage2D) (_gcArgComma_ GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height); \
    GLvoid         (GLAPIENTRY *TexStorage3D) (_gcArgComma_ GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth); \
    GLvoid         (GLAPIENTRY *GetInternalformativ) (_gcArgComma_ GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params); \
    /* OpenGL ES 3.1 */ \
    GLvoid         (GLAPIENTRY *DispatchCompute) (_gcArgComma_ GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z); \
    GLvoid         (GLAPIENTRY *DispatchComputeIndirect) (_gcArgComma_ GLintptr indirect); \
    GLvoid         (GLAPIENTRY *DrawArraysIndirect) (_gcArgComma_ GLenum mode, const void *indirect); \
    GLvoid         (GLAPIENTRY *DrawElementsIndirect) (_gcArgComma_ GLenum mode, GLenum type, const void *indirect); \
    GLvoid         (GLAPIENTRY *FramebufferParameteri) (_gcArgComma_ GLenum target, GLenum pname, GLint param); \
    GLvoid         (GLAPIENTRY *GetFramebufferParameteriv) (_gcArgComma_ GLenum target, GLenum pname, GLint *params); \
    GLvoid         (GLAPIENTRY *GetProgramInterfaceiv) (_gcArgComma_ GLuint program, GLenum programInterface, GLenum pname, GLint *params); \
    GLuint         (GLAPIENTRY *GetProgramResourceIndex) (_gcArgComma_ GLuint program, GLenum programInterface, const GLchar *name); \
    GLvoid         (GLAPIENTRY *GetProgramResourceName) (_gcArgComma_ GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name); \
    GLvoid         (GLAPIENTRY *GetProgramResourceiv) (_gcArgComma_ GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params); \
    GLint          (GLAPIENTRY *GetProgramResourceLocation) (_gcArgComma_ GLuint program, GLenum programInterface, const GLchar *name); \
    GLvoid         (GLAPIENTRY *UseProgramStages) (_gcArgComma_ GLuint pipeline, GLbitfield stages, GLuint program); \
    GLvoid         (GLAPIENTRY *ActiveShaderProgram) (_gcArgComma_ GLuint pipeline, GLuint program); \
    GLuint         (GLAPIENTRY *CreateShaderProgramv) (_gcArgComma_ GLenum type, GLsizei count, const GLchar *const*strings); \
    GLvoid         (GLAPIENTRY *BindProgramPipeline) (_gcArgComma_ GLuint pipeline); \
    GLvoid         (GLAPIENTRY *DeleteProgramPipelines) (_gcArgComma_ GLsizei n, const GLuint *pipelines); \
    GLvoid         (GLAPIENTRY *GenProgramPipelines) (_gcArgComma_ GLsizei n, GLuint *pipelines); \
    GLboolean      (GLAPIENTRY *IsProgramPipeline) (_gcArgComma_ GLuint pipeline); \
    GLvoid         (GLAPIENTRY *GetProgramPipelineiv) (_gcArgComma_ GLuint pipeline, GLenum pname, GLint *params); \
    GLvoid         (GLAPIENTRY *ProgramUniform1i) (_gcArgComma_ GLuint program, GLint location, GLint v0); \
    GLvoid         (GLAPIENTRY *ProgramUniform2i) (_gcArgComma_ GLuint program, GLint location, GLint v0, GLint v1); \
    GLvoid         (GLAPIENTRY *ProgramUniform3i) (_gcArgComma_ GLuint program, GLint location, GLint v0, GLint v1, GLint v2); \
    GLvoid         (GLAPIENTRY *ProgramUniform4i) (_gcArgComma_ GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3); \
    GLvoid         (GLAPIENTRY *ProgramUniform1ui) (_gcArgComma_ GLuint program, GLint location, GLuint v0); \
    GLvoid         (GLAPIENTRY *ProgramUniform2ui) (_gcArgComma_ GLuint program, GLint location, GLuint v0, GLuint v1); \
    GLvoid         (GLAPIENTRY *ProgramUniform3ui) (_gcArgComma_ GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2); \
    GLvoid         (GLAPIENTRY *ProgramUniform4ui) (_gcArgComma_ GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3); \
    GLvoid         (GLAPIENTRY *ProgramUniform1f) (_gcArgComma_ GLuint program, GLint location, GLfloat v0); \
    GLvoid         (GLAPIENTRY *ProgramUniform2f) (_gcArgComma_ GLuint program, GLint location, GLfloat v0, GLfloat v1); \
    GLvoid         (GLAPIENTRY *ProgramUniform3f) (_gcArgComma_ GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2); \
    GLvoid         (GLAPIENTRY *ProgramUniform4f) (_gcArgComma_ GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3); \
    GLvoid         (GLAPIENTRY *ProgramUniform1iv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLint *value); \
    GLvoid         (GLAPIENTRY *ProgramUniform2iv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLint *value); \
    GLvoid         (GLAPIENTRY *ProgramUniform3iv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLint *value); \
    GLvoid         (GLAPIENTRY *ProgramUniform4iv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLint *value); \
    GLvoid         (GLAPIENTRY *ProgramUniform1uiv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLuint *value); \
    GLvoid         (GLAPIENTRY *ProgramUniform2uiv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLuint *value); \
    GLvoid         (GLAPIENTRY *ProgramUniform3uiv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLuint *value); \
    GLvoid         (GLAPIENTRY *ProgramUniform4uiv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLuint *value); \
    GLvoid         (GLAPIENTRY *ProgramUniform1fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLfloat *value); \
    GLvoid         (GLAPIENTRY *ProgramUniform2fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLfloat *value); \
    GLvoid         (GLAPIENTRY *ProgramUniform3fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLfloat *value); \
    GLvoid         (GLAPIENTRY *ProgramUniform4fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLfloat *value); \
    GLvoid         (GLAPIENTRY *ProgramUniformMatrix2fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value); \
    GLvoid         (GLAPIENTRY *ProgramUniformMatrix3fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value); \
    GLvoid         (GLAPIENTRY *ProgramUniformMatrix4fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value); \
    GLvoid         (GLAPIENTRY *ProgramUniformMatrix2x3fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value); \
    GLvoid         (GLAPIENTRY *ProgramUniformMatrix3x2fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value); \
    GLvoid         (GLAPIENTRY *ProgramUniformMatrix2x4fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value); \
    GLvoid         (GLAPIENTRY *ProgramUniformMatrix4x2fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value); \
    GLvoid         (GLAPIENTRY *ProgramUniformMatrix3x4fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value); \
    GLvoid         (GLAPIENTRY *ProgramUniformMatrix4x3fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value); \
    GLvoid         (GLAPIENTRY *ValidateProgramPipeline) (_gcArgComma_ GLuint pipeline); \
    GLvoid         (GLAPIENTRY *GetProgramPipelineInfoLog) (_gcArgComma_ GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog); \
    GLvoid         (GLAPIENTRY *BindImageTexture) (_gcArgComma_ GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format); \
    GLvoid         (GLAPIENTRY *GetBooleani_v) (_gcArgComma_ GLenum target, GLuint index, GLboolean *data); \
    GLvoid         (GLAPIENTRY *MemoryBarrier) (_gcArgComma_ GLbitfield barriers); \
    GLvoid         (GLAPIENTRY *MemoryBarrierByRegion) (_gcArgComma_ GLbitfield barriers); \
    GLvoid         (GLAPIENTRY *TexStorage2DMultisample) (_gcArgComma_ GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations); \
    GLvoid         (GLAPIENTRY *GetMultisamplefv) (_gcArgComma_ GLenum pname, GLuint index, GLfloat *val); \
    GLvoid         (GLAPIENTRY *SampleMaski) (_gcArgComma_ GLuint maskNumber, GLbitfield mask); \
    GLvoid         (GLAPIENTRY *BindVertexBuffer) (_gcArgComma_ GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride); \
    GLvoid         (GLAPIENTRY *VertexAttribFormat) (_gcArgComma_ GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset); \
    GLvoid         (GLAPIENTRY *VertexAttribIFormat) (_gcArgComma_ GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset); \
    GLvoid         (GLAPIENTRY *VertexAttribBinding) (_gcArgComma_ GLuint attribindex, GLuint bindingindex); \
    GLvoid         (GLAPIENTRY *VertexBindingDivisor) (_gcArgComma_ GLuint bindingindex, GLuint divisor); \
    /* OpenGL ES 3.2 */ \
    GLvoid         (GLAPIENTRY *TexStorage3DMultisample) (_gcArgComma_ GLenum target, GLsizei samples, GLenum sizedinternalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations); \
    GLvoid         (GLAPIENTRY *BlendBarrier) (_gcArgOnly_); \
    GLvoid         (GLAPIENTRY *DebugMessageControl) (_gcArgComma_ GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled); \
    GLvoid         (GLAPIENTRY *DebugMessageInsert) (_gcArgComma_ GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf); \
    GLvoid         (GLAPIENTRY *DebugMessageCallback) (_gcArgComma_ GLDEBUGPROCKHR callback, const GLvoid* userParam); \
    GLuint         (GLAPIENTRY *GetDebugMessageLog) (_gcArgComma_ GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog); \
    GLvoid         (GLAPIENTRY *PushDebugGroup) (_gcArgComma_ GLenum source, GLuint id, GLsizei length, const GLchar * message); \
    GLvoid         (GLAPIENTRY *PopDebugGroup) (_gcArgOnly_); \
    GLvoid         (GLAPIENTRY *ObjectLabel) (_gcArgComma_ GLenum identifier, GLuint name, GLsizei length, const GLchar *label); \
    GLvoid         (GLAPIENTRY *GetObjectLabel) (_gcArgComma_ GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label); \
    GLvoid         (GLAPIENTRY *ObjectPtrLabel) (_gcArgComma_ const GLvoid* ptr, GLsizei length, const GLchar *label); \
    GLvoid         (GLAPIENTRY *GetObjectPtrLabel) (_gcArgComma_ const GLvoid* ptr, GLsizei bufSize, GLsizei *length, GLchar *label); \
    GLenum       (GLAPIENTRY *GetGraphicsResetStatus) (_gcArgOnly_); \
    GLvoid         (GLAPIENTRY *ReadnPixels) (_gcArgComma_ GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLvoid *data); \
    GLvoid         (GLAPIENTRY *GetnUniformfv) (_gcArgComma_ GLuint program, GLint location, GLsizei bufSize, GLfloat *params); \
    GLvoid         (GLAPIENTRY *GetnUniformiv) (_gcArgComma_ GLuint program, GLint location, GLsizei bufSize, GLint *params);   \
    GLvoid         (GLAPIENTRY *GetnUniformuiv) (_gcArgComma_ GLuint program, GLint location, GLsizei bufSize, GLuint *params);   \
    GLvoid         (GLAPIENTRY  *BlendEquationi) (_gcArgComma_ GLuint buf, GLenum mode); \
    GLvoid         (GLAPIENTRY  *BlendEquationSeparatei) (_gcArgComma_ GLuint buf, GLenum modeRGB, GLenum modeAlpha); \
    GLvoid         (GLAPIENTRY  *BlendFunci) (_gcArgComma_ GLuint buf, GLenum sfactor, GLenum dfactor); \
    GLvoid         (GLAPIENTRY  *BlendFuncSeparatei) (_gcArgComma_ GLuint buf, GLenum sfactorRGB,GLenum dfactorRGB,GLenum sfactorAlpha,GLenum dfactorAlpha) ; \
    GLvoid         (GLAPIENTRY  *ColorMaski) (_gcArgComma_ GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a); \
    GLvoid         (GLAPIENTRY  *Enablei) (_gcArgComma_ GLenum target, GLuint index);  \
    GLvoid         (GLAPIENTRY  *Disablei) (_gcArgComma_ GLenum target, GLuint index); \
    GLboolean    (GLAPIENTRY  *IsEnabledi) (_gcArgComma_ GLenum target, GLuint index); \
    GLvoid         (GLAPIENTRY *TexParameterIiv) (_gcArgComma_ GLenum target, GLenum pname, const GLint *params);   \
    GLvoid         (GLAPIENTRY *TexParameterIuiv) (_gcArgComma_ GLenum target, GLenum pname, const GLuint *params); \
    GLvoid         (GLAPIENTRY *GetTexParameterIiv) (_gcArgComma_ GLenum target, GLenum pname, GLint *params);      \
    GLvoid         (GLAPIENTRY *GetTexParameterIuiv) (_gcArgComma_ GLenum target, GLenum pname, GLuint *params);    \
    GLvoid         (GLAPIENTRY *SamplerParameterIiv) (_gcArgComma_ GLuint sampler, GLenum pname, const GLint *param); \
    GLvoid         (GLAPIENTRY *SamplerParameterIuiv) (_gcArgComma_ GLuint sampler, GLenum pname, const GLuint *param); \
    GLvoid         (GLAPIENTRY *GetSamplerParameterIiv) (_gcArgComma_ GLuint sampler, GLenum pname, GLint *params);  \
    GLvoid         (GLAPIENTRY *GetSamplerParameterIuiv) (_gcArgComma_ GLuint sampler, GLenum pname, GLuint *params);  \
    GLvoid         (GLAPIENTRY *TexBuffer) (_gcArgComma_ GLenum target, GLenum internalformat, GLuint buffer); \
    GLvoid         (GLAPIENTRY *TexBufferRange) (_gcArgComma_ GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size); \
    GLvoid         (GLAPIENTRY *PatchParameteri) (_gcArgComma_ GLenum pname, GLint value); \
    GLvoid         (GLAPIENTRY *FramebufferTexture) (_gcArgComma_ GLenum target, GLenum attachment, GLuint texture, GLint level); \
    GLvoid         (GLAPIENTRY *MinSampleShading) (_gcArgComma_ GLfloat value); \
    GLvoid         (GLAPIENTRY *CopyImageSubData) (_gcArgComma_ GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, \
                                                       GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ,  \
                                                       GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth); \
    GLvoid         (GLAPIENTRY *DrawElementsBaseVertex) (_gcArgComma_ GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex); \
    GLvoid         (GLAPIENTRY *DrawRangeElementsBaseVertex) (_gcArgComma_ GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex); \
    GLvoid         (GLAPIENTRY *DrawElementsInstancedBaseVertex) (_gcArgComma_ GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex); \
    GLvoid         (GLAPIENTRY *PrimitiveBoundingBox)(_gcArgComma_ GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW); \
    /* OpenGL ES extensions */ \
    /* GL_OES_EGL_image */ \
    GLvoid         (GLAPIENTRY *EGLImageTargetTexture2DOES) (_gcArgComma_ GLenum target, GLeglImageOES image); \
    GLvoid         (GLAPIENTRY *EGLImageTargetRenderbufferStorageOES) (_gcArgComma_ GLenum target, GLeglImageOES image); \
    /* GL_EXT_multi_draw_arrays */ \
    /* GL_EXT_multi_draw_arrays && GL_EXT_draw_elements_base_vertex */ \
    GLvoid         (GLAPIENTRY *MultiDrawElementsBaseVertexEXT) (_gcArgComma_ GLenum mode, const GLsizei *count, GLenum type, const void *const *indices, GLsizei drawcount, const GLint * basevertex); \
    /* GL_OES_mapbuffer */ \
    GLvoid         (GLAPIENTRY *GetBufferPointervOES) (_gcArgComma_ GLenum target, GLenum pname, GLvoid** params); \
    /* GL_EXT_discard_framebuffer */ \
    GLvoid         (GLAPIENTRY *DiscardFramebufferEXT) (_gcArgComma_ GLenum target, GLsizei numAttachments, const GLenum *attachments); \
    /* GL_EXT_multisampled_render_to_texture */ \
    GLvoid         (GLAPIENTRY *RenderbufferStorageMultisampleEXT) (_gcArgComma_ GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height); \
    GLvoid         (GLAPIENTRY *FramebufferTexture2DMultisampleEXT) (_gcArgComma_ GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples); \
    /* GL_VIV_direct_texture */ \
    GLvoid         (GLAPIENTRY *TexDirectVIV) (_gcArgComma_ GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** pixels); \
    GLvoid         (GLAPIENTRY *TexDirectInvalidateVIV) (_gcArgComma_ GLenum target); \
    GLvoid         (GLAPIENTRY *TexDirectVIVMap) (_gcArgComma_ GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** logical, const GLuint * physical); \
    GLvoid         (GLAPIENTRY *TexDirectTiledMapVIV) (_gcArgComma_ GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** logical, const GLuint * physical); \
    /* GL_EXT_multi_draw_indirect */ \
    GLvoid         (GLAPIENTRY *MultiDrawArraysIndirectEXT) (_gcArgComma_ GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride); \
    GLvoid         (GLAPIENTRY *MultiDrawElementsIndirectEXT) (_gcArgComma_ GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride);\
    GLvoid         (GLAPIENTRY *DeleteObjectARB) (_gcArgComma_ UINT obj);\
    GLvoid         (GLAPIENTRY *GetInfoLogARB) (_gcArgComma_ UINT obj, GLsizei maxLength, GLsizei *length, char *infoLog);\
    GLvoid         (GLAPIENTRY *GetObjectParameterivARB) (_gcArgComma_ UINT obj, GLenum pname, GLint *params);\
    GLvoid         (GLAPIENTRY *ClampColorARB)(_gcArgComma_ GLenum target, GLenum clamp);\
    GLboolean    (GLAPIENTRY *IsRenderbufferEXT)( _gcArgComma_ GLuint renderbuffer); \
    GLvoid         (GLAPIENTRY *BindRenderbufferEXT)(_gcArgComma_ GLenum target, GLuint renderbuffer); \
    GLvoid         (GLAPIENTRY *DeleteRenderbuffersEXT)(_gcArgComma_ GLsizei n, const GLuint *renderbuffers); \
    GLvoid         (GLAPIENTRY *GenRenderbuffersEXT)(_gcArgComma_ GLsizei n, GLuint *renderbuffers); \
    GLvoid         (GLAPIENTRY *RenderbufferStorageEXT)(_gcArgComma_ GLenum target, GLenum internalformat, GLsizei width, GLsizei height); \
    GLvoid         (GLAPIENTRY *GetRenderbufferParameterivEXT)(_gcArgComma_ GLenum target, GLenum pname, GLint* params); \
    GLboolean    (GLAPIENTRY *IsFramebufferEXT)(_gcArgComma_ GLuint framebuffer); \
    GLvoid         (GLAPIENTRY *BindFramebufferEXT)(_gcArgComma_ GLenum target, GLuint framebuffer); \
    GLvoid         (GLAPIENTRY *DeleteFramebuffersEXT)(_gcArgComma_ GLsizei n, const GLuint *framebuffers); \
    GLvoid         (GLAPIENTRY *GenFramebuffersEXT)(_gcArgComma_ GLsizei n, GLuint *framebuffers); \
    GLenum       (GLAPIENTRY *CheckFramebufferStatusEXT)(_gcArgComma_ GLenum target); \
    GLvoid         (GLAPIENTRY *FramebufferTexture1DEXT)(_gcArgComma_ GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level); \
    GLvoid         (GLAPIENTRY *FramebufferTexture2DEXT)(_gcArgComma_ GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level); \
    GLvoid         (GLAPIENTRY *FramebufferTexture3DEXT)(_gcArgComma_ GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset); \
    GLvoid         (GLAPIENTRY *FramebufferRenderbufferEXT)(_gcArgComma_ GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer); \
    GLvoid         (GLAPIENTRY *GetFramebufferAttachmentParameterivEXT)(_gcArgComma_ GLenum target, GLenum attachment, GLenum pname, GLint *params); \
    GLvoid         (GLAPIENTRY *GenerateMipmapEXT)(_gcArgComma_ GLenum target); \
    GLvoid         (GLAPIENTRY *BlitFramebufferEXT)(_gcArgComma_ GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, \
                                          GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, \
                                          GLbitfield mask, GLenum filter); \
    GLvoid         (GLAPIENTRY *ClampColor)(_gcArgComma_ GLenum target, GLenum clamp);\
    GLvoid         (GLAPIENTRY *BeginConditionalRender) (_gcArgComma_ GLuint id, GLenum mode);\
    GLvoid         (GLAPIENTRY *EndConditionalRender) (_gcArgOnly_);\
    GLvoid         (GLAPIENTRY *VertexAttribI1i)(_gcArgComma_ GLuint index, GLint x);\
    GLvoid         (GLAPIENTRY *VertexAttribI2i)(_gcArgComma_ GLuint index, GLint x, GLint y);\
    GLvoid         (GLAPIENTRY *VertexAttribI3i)(_gcArgComma_ GLuint index, GLint x, GLint y, GLint z);\
    GLvoid         (GLAPIENTRY *VertexAttribI1ui)(_gcArgComma_ GLuint index, GLuint x);\
    GLvoid         (GLAPIENTRY *VertexAttribI2ui)(_gcArgComma_ GLuint index, GLuint x, GLuint y);\
    GLvoid         (GLAPIENTRY *VertexAttribI3ui)(_gcArgComma_ GLuint index, GLuint x, GLuint y, GLuint z);\
    GLvoid         (GLAPIENTRY *VertexAttribI1iv)(_gcArgComma_ GLuint index, const GLint *v);\
    GLvoid         (GLAPIENTRY *VertexAttribI2iv)(_gcArgComma_ GLuint index, const GLint *v);\
    GLvoid         (GLAPIENTRY *VertexAttribI3iv)(_gcArgComma_ GLuint index, const GLint *v);\
    GLvoid         (GLAPIENTRY *VertexAttribI1uiv)(_gcArgComma_ GLuint index, const GLuint *v);\
    GLvoid         (GLAPIENTRY *VertexAttribI2uiv)(_gcArgComma_ GLuint index, const GLuint *v);\
    GLvoid         (GLAPIENTRY *VertexAttribI3uiv)(_gcArgComma_ GLuint index, const GLuint *v);\
    GLvoid         (GLAPIENTRY *VertexAttribI4bv)(_gcArgComma_ GLuint index, const GLbyte *v);\
    GLvoid         (GLAPIENTRY *VertexAttribI4sv)(_gcArgComma_ GLuint index, const GLshort *v);\
    GLvoid         (GLAPIENTRY *VertexAttribI4ubv)(_gcArgComma_ GLuint index, const GLubyte *v);\
    GLvoid         (GLAPIENTRY *VertexAttribI4usv)(_gcArgComma_ GLuint index, const GLushort *v);\
    GLvoid         (GLAPIENTRY *FramebufferTexture1D)(_gcArgComma_ GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);\
    GLvoid         (GLAPIENTRY *FramebufferTexture3D)(_gcArgComma_ GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);\
    GLvoid         (GLAPIENTRY *PrimitiveRestartIndex) (_gcArgComma_ GLuint index);\
    GLvoid         (GLAPIENTRY *GetActiveUniformName) (_gcArgComma_ GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName);\
  /* OpenGL 4.0 */\
    GLvoid         (GLAPIENTRY *GetUniformdv)(_gcArgComma_ GLuint program, GLint location, GLdouble *params);\
    GLvoid         (GLAPIENTRY *Uniform1d)(_gcArgComma_ GLint location, GLdouble v0);\
    GLvoid         (GLAPIENTRY *Uniform2d)(_gcArgComma_ GLint location, GLdouble v0, GLdouble v1);\
    GLvoid         (GLAPIENTRY *Uniform3d)(_gcArgComma_ GLint location, GLdouble v0, GLdouble v1, GLdouble v2);\
    GLvoid         (GLAPIENTRY *Uniform4d)(_gcArgComma_ GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3);\
    GLvoid         (GLAPIENTRY *Uniform1dv)(_gcArgComma_ GLint location, GLsizei count, const GLdouble* value);\
    GLvoid         (GLAPIENTRY *Uniform2dv)(_gcArgComma_ GLint location, GLsizei count, const GLdouble* value);\
    GLvoid         (GLAPIENTRY *Uniform3dv)(_gcArgComma_ GLint location, GLsizei count, const GLdouble* value);\
    GLvoid         (GLAPIENTRY *Uniform4dv)(_gcArgComma_ GLint location, GLsizei count, const GLdouble* value);\
    GLvoid         (GLAPIENTRY *UniformMatrix2dv)(_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);\
    GLvoid         (GLAPIENTRY *UniformMatrix3dv)(_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);\
    GLvoid         (GLAPIENTRY *UniformMatrix4dv)(_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);\
    GLvoid         (GLAPIENTRY *UniformMatrix2x3dv)(_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);\
    GLvoid         (GLAPIENTRY *UniformMatrix3x2dv)(_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);\
    GLvoid         (GLAPIENTRY *UniformMatrix2x4dv)(_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);\
    GLvoid         (GLAPIENTRY *UniformMatrix4x2dv)(_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);\
    GLvoid         (GLAPIENTRY *UniformMatrix3x4dv)(_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);\
    GLvoid         (GLAPIENTRY *UniformMatrix4x3dv)(_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);\



/* Define Internal GL/ES API Dispatch Table */
#define _gcArgComma_  __GLcontext* gc,
#define _gcArgOnly_   __GLcontext* gc
#define _retProgram_
#define _retShader_
#define _retIndex_
#define _retLocation_
#define _retSync_
#define _retPointer

struct __GLesDispatchTableRec
{
    __GLES_API_DISPATCH_FUNCS
};

#undef _gcArgComma_
#undef _gcArgOnly_
#undef _retProgram_
#undef _retShader_
#undef _retIndex_
#undef _retLocation_
#undef _retSync_
#undef _retPointer


/* Define Tools API Tracer Dispatch Table */
#define _gcArgComma_
#define _gcArgOnly_
#define _retProgram_     GLuint retval
#define _retShader_     ,GLuint retval
#define _retIndex_      ,GLuint retidx
#define _retLocation_   ,GLint retloc
#define _retSync_       ,GLsync retsync
#define _retPointer     ,GLvoid* retptr

typedef struct __GLtraceDispatchTableRec{

    __GLES_API_DISPATCH_FUNCS

} __GLesTracerDispatchTableStruct;

#undef _gcArgComma_
#undef _gcArgOnly_
#undef _retProgram_
#undef _retShader_
#undef _retIndex_
#undef _retLocation_
#undef _retSync_
#undef _retPointer


/* Define Native API Tracer Dispatch Table */
#define _gcArgComma_
#define _gcArgOnly_
#define _retProgram_
#define _retShader_
#define _retIndex_
#define _retLocation_
#define _retSync_
#define _retPointer

/* Since opengl32.dll only supports OpenGL version 1.1, the "nEntries" in __GLdispatchState
** which is returned to opengl32.dll by DrvSetContext() must be OPENGL_VERSION_110_ENTRIES.
 */
#define OPENGL_VERSION_110_ENTRIES      336

typedef struct __GLRawDispatchTableRec{
    GLint nEntries;
    __GLES_API_DISPATCH_FUNCS
} __GLesRawDispatchTableStruct;

#undef _gcArgComma_
#undef _gcArgOnly_
#undef _retProgram_
#undef _retShader_
#undef _retIndex_
#undef _retLocation_
#undef _retSync_
#undef _retPointer


extern __GLesDispatchTable __glesApiFuncDispatchTable;
extern __GLesDispatchTable __glImmediateFuncTable;
extern __GLesDispatchTable __glListCompileFuncTable;

extern __GLesRawDispatchTableStruct __glVIV_DispatchFuncTable;

#endif /* __gc_es_dispatch_h__ */
