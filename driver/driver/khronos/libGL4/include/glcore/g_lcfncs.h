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


#ifndef __g_lcfncs_h_
#define __g_lcfncs_h_

extern GLvoid __gllc_InvalidValue(__GLcontext *gc);
extern GLvoid __gllc_InvalidEnum(__GLcontext *gc);
extern GLvoid __gllc_InvalidOperation(__GLcontext *gc);
extern GLvoid __gllc_TableTooLarge(__GLcontext *gc);
extern GLvoid __gllc_Error(__GLcontext *gc, GLenum error);

extern GLvoid APIENTRY __gllc_NewList(__GLcontext *,  GLuint, GLenum);
extern GLvoid APIENTRY __gllc_EndList(__GLcontext *);
extern GLvoid APIENTRY __gllc_CallList(__GLcontext *,  GLuint);
extern GLvoid APIENTRY __gllc_CallLists(__GLcontext *,  GLsizei, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_DeleteLists(__GLcontext *,  GLuint, GLsizei);
extern GLuint APIENTRY __gllc_GenLists(__GLcontext *,  GLsizei);
extern GLvoid APIENTRY __gllc_ListBase(__GLcontext *,  GLuint);
extern GLvoid APIENTRY __gllc_Begin(__GLcontext *,  GLenum);
extern GLvoid APIENTRY __gllc_Bitmap(__GLcontext *,  GLsizei, GLsizei, GLfloat, GLfloat, GLfloat, GLfloat, const GLubyte *);
extern GLvoid APIENTRY __gllc_Color3b(__GLcontext *,  GLbyte, GLbyte, GLbyte);
extern GLvoid APIENTRY __gllc_Color3bv(__GLcontext *,  const GLbyte *);
extern GLvoid APIENTRY __gllc_Color3d(__GLcontext *,  GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_Color3dv(__GLcontext *,  const GLdouble *);
extern GLvoid APIENTRY __gllc_Color3f(__GLcontext *,  GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_Color3fv(__GLcontext *,  const GLfloat *);
extern GLvoid APIENTRY __gllc_Color3i(__GLcontext *,  GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_Color3iv(__GLcontext *,  const GLint *);
extern GLvoid APIENTRY __gllc_Color3s(__GLcontext *,  GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_Color3sv(__GLcontext *,  const GLshort *);
extern GLvoid APIENTRY __gllc_Color3ub(__GLcontext *,  GLubyte, GLubyte, GLubyte);
extern GLvoid APIENTRY __gllc_Color3ubv(__GLcontext *,  const GLubyte *);
extern GLvoid APIENTRY __gllc_Color3ui(__GLcontext *,  GLuint, GLuint, GLuint);
extern GLvoid APIENTRY __gllc_Color3uiv(__GLcontext *,  const GLuint *);
extern GLvoid APIENTRY __gllc_Color3us(__GLcontext *,  GLushort, GLushort, GLushort);
extern GLvoid APIENTRY __gllc_Color3usv(__GLcontext *,  const GLushort *);
extern GLvoid APIENTRY __gllc_Color4b(__GLcontext *,  GLbyte, GLbyte, GLbyte, GLbyte);
extern GLvoid APIENTRY __gllc_Color4bv(__GLcontext *,  const GLbyte *);
extern GLvoid APIENTRY __gllc_Color4d(__GLcontext *,  GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_Color4dv(__GLcontext *,  const GLdouble *);
extern GLvoid APIENTRY __gllc_Color4f(__GLcontext *,  GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_Color4fv(__GLcontext *,  const GLfloat *);
extern GLvoid APIENTRY __gllc_Color4i(__GLcontext *,  GLint, GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_Color4iv(__GLcontext *,  const GLint *);
extern GLvoid APIENTRY __gllc_Color4s(__GLcontext *,  GLshort, GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_Color4sv(__GLcontext *,  const GLshort *);
extern GLvoid APIENTRY __gllc_Color4ub(__GLcontext *,  GLubyte, GLubyte, GLubyte, GLubyte);
extern GLvoid APIENTRY __gllc_Color4ubv(__GLcontext *,  const GLubyte *);
extern GLvoid APIENTRY __gllc_Color4ui(__GLcontext *,  GLuint, GLuint, GLuint, GLuint);
extern GLvoid APIENTRY __gllc_Color4uiv(__GLcontext *,  const GLuint *);
extern GLvoid APIENTRY __gllc_Color4us(__GLcontext *,  GLushort, GLushort, GLushort, GLushort);
extern GLvoid APIENTRY __gllc_Color4usv(__GLcontext *,  const GLushort *);
extern GLvoid APIENTRY __gllc_EdgeFlag(__GLcontext *,  GLboolean);
extern GLvoid APIENTRY __gllc_EdgeFlagv(__GLcontext *,  const GLboolean *);
extern GLvoid APIENTRY __gllc_End(__GLcontext *);
extern GLvoid APIENTRY __gllc_Indexd(__GLcontext *,  GLdouble);
extern GLvoid APIENTRY __gllc_Indexdv(__GLcontext *,  const GLdouble *);
extern GLvoid APIENTRY __gllc_Indexf(__GLcontext *,  GLfloat);
extern GLvoid APIENTRY __gllc_Indexfv(__GLcontext *,  const GLfloat *);
extern GLvoid APIENTRY __gllc_Indexi(__GLcontext *,  GLint);
extern GLvoid APIENTRY __gllc_Indexiv(__GLcontext *,  const GLint *);
extern GLvoid APIENTRY __gllc_Indexs(__GLcontext *,  GLshort);
extern GLvoid APIENTRY __gllc_Indexsv(__GLcontext *,  const GLshort *);
extern GLvoid APIENTRY __gllc_Normal3b(__GLcontext *,  GLbyte, GLbyte, GLbyte);
extern GLvoid APIENTRY __gllc_Normal3bv(__GLcontext *,  const GLbyte *);
extern GLvoid APIENTRY __gllc_Normal3d(__GLcontext *,  GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_Normal3dv(__GLcontext *,  const GLdouble *);
extern GLvoid APIENTRY __gllc_Normal3f(__GLcontext *,  GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_Normal3fv(__GLcontext *,  const GLfloat *);
extern GLvoid APIENTRY __gllc_Normal3i(__GLcontext *,  GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_Normal3iv(__GLcontext *,  const GLint *);
extern GLvoid APIENTRY __gllc_Normal3s(__GLcontext *,  GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_Normal3sv(__GLcontext *,  const GLshort *);
extern GLvoid APIENTRY __gllc_RasterPos2d(__GLcontext *,  GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_RasterPos2dv(__GLcontext *,  const GLdouble *);
extern GLvoid APIENTRY __gllc_RasterPos2f(__GLcontext *,  GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_RasterPos2fv(__GLcontext *,  const GLfloat *);
extern GLvoid APIENTRY __gllc_RasterPos2i(__GLcontext *,  GLint, GLint);
extern GLvoid APIENTRY __gllc_RasterPos2iv(__GLcontext *,  const GLint *);
extern GLvoid APIENTRY __gllc_RasterPos2s(__GLcontext *,  GLshort, GLshort);
extern GLvoid APIENTRY __gllc_RasterPos2sv(__GLcontext *,  const GLshort *);
extern GLvoid APIENTRY __gllc_RasterPos3d(__GLcontext *,  GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_RasterPos3dv(__GLcontext *,  const GLdouble *);
extern GLvoid APIENTRY __gllc_RasterPos3f(__GLcontext *,  GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_RasterPos3fv(__GLcontext *,  const GLfloat *);
extern GLvoid APIENTRY __gllc_RasterPos3i(__GLcontext *,  GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_RasterPos3iv(__GLcontext *,  const GLint *);
extern GLvoid APIENTRY __gllc_RasterPos3s(__GLcontext *,  GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_RasterPos3sv(__GLcontext *,  const GLshort *);
extern GLvoid APIENTRY __gllc_RasterPos4d(__GLcontext *,  GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_RasterPos4dv(__GLcontext *,  const GLdouble *);
extern GLvoid APIENTRY __gllc_RasterPos4f(__GLcontext *,  GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_RasterPos4fv(__GLcontext *,  const GLfloat *);
extern GLvoid APIENTRY __gllc_RasterPos4i(__GLcontext *,  GLint, GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_RasterPos4iv(__GLcontext *,  const GLint *);
extern GLvoid APIENTRY __gllc_RasterPos4s(__GLcontext *,  GLshort, GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_RasterPos4sv(__GLcontext *,  const GLshort *);
extern GLvoid APIENTRY __gllc_Rectd(__GLcontext *,  GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_Rectdv(__GLcontext *,  const GLdouble *, const GLdouble *);
extern GLvoid APIENTRY __gllc_Rectf(__GLcontext *,  GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_Rectfv(__GLcontext *,  const GLfloat *, const GLfloat *);
extern GLvoid APIENTRY __gllc_Recti(__GLcontext *,  GLint, GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_Rectiv(__GLcontext *,  const GLint *, const GLint *);
extern GLvoid APIENTRY __gllc_Rects(__GLcontext *,  GLshort, GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_Rectsv(__GLcontext *,  const GLshort *, const GLshort *);
extern GLvoid APIENTRY __gllc_TexCoord1d(__GLcontext *,  GLdouble);
extern GLvoid APIENTRY __gllc_TexCoord1dv(__GLcontext *,  const GLdouble *);
extern GLvoid APIENTRY __gllc_TexCoord1f(__GLcontext *,  GLfloat);
extern GLvoid APIENTRY __gllc_TexCoord1fv(__GLcontext *,  const GLfloat *);
extern GLvoid APIENTRY __gllc_TexCoord1i(__GLcontext *,  GLint);
extern GLvoid APIENTRY __gllc_TexCoord1iv(__GLcontext *,  const GLint *);
extern GLvoid APIENTRY __gllc_TexCoord1s(__GLcontext *,  GLshort);
extern GLvoid APIENTRY __gllc_TexCoord1sv(__GLcontext *,  const GLshort *);
extern GLvoid APIENTRY __gllc_TexCoord2d(__GLcontext *,  GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_TexCoord2dv(__GLcontext *,  const GLdouble *);
extern GLvoid APIENTRY __gllc_TexCoord2f(__GLcontext *,  GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_TexCoord2fv(__GLcontext *,  const GLfloat *);
extern GLvoid APIENTRY __gllc_TexCoord2i(__GLcontext *,  GLint, GLint);
extern GLvoid APIENTRY __gllc_TexCoord2iv(__GLcontext *,  const GLint *);
extern GLvoid APIENTRY __gllc_TexCoord2s(__GLcontext *,  GLshort, GLshort);
extern GLvoid APIENTRY __gllc_TexCoord2sv(__GLcontext *,  const GLshort *);
extern GLvoid APIENTRY __gllc_TexCoord3d(__GLcontext *,  GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_TexCoord3dv(__GLcontext *,  const GLdouble *);
extern GLvoid APIENTRY __gllc_TexCoord3f(__GLcontext *,  GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_TexCoord3fv(__GLcontext *,  const GLfloat *);
extern GLvoid APIENTRY __gllc_TexCoord3i(__GLcontext *,  GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_TexCoord3iv(__GLcontext *,  const GLint *);
extern GLvoid APIENTRY __gllc_TexCoord3s(__GLcontext *,  GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_TexCoord3sv(__GLcontext *,  const GLshort *);
extern GLvoid APIENTRY __gllc_TexCoord4d(__GLcontext *,  GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_TexCoord4dv(__GLcontext *,  const GLdouble *);
extern GLvoid APIENTRY __gllc_TexCoord4f(__GLcontext *,  GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_TexCoord4fv(__GLcontext *,  const GLfloat *);
extern GLvoid APIENTRY __gllc_TexCoord4i(__GLcontext *,  GLint, GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_TexCoord4iv(__GLcontext *,  const GLint *);
extern GLvoid APIENTRY __gllc_TexCoord4s(__GLcontext *,  GLshort, GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_TexCoord4sv(__GLcontext *,  const GLshort *);
extern GLvoid APIENTRY __gllc_Vertex2d(__GLcontext *,  GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_Vertex2dv(__GLcontext *,  const GLdouble *);
extern GLvoid APIENTRY __gllc_Vertex2f(__GLcontext *,  GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_Vertex2fv(__GLcontext *,  const GLfloat *);
extern GLvoid APIENTRY __gllc_Vertex2i(__GLcontext *,  GLint, GLint);
extern GLvoid APIENTRY __gllc_Vertex2iv(__GLcontext *,  const GLint *);
extern GLvoid APIENTRY __gllc_Vertex2s(__GLcontext *,  GLshort, GLshort);
extern GLvoid APIENTRY __gllc_Vertex2sv(__GLcontext *,  const GLshort *);
extern GLvoid APIENTRY __gllc_Vertex3d(__GLcontext *,  GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_Vertex3dv(__GLcontext *,  const GLdouble *);
extern GLvoid APIENTRY __gllc_Vertex3f(__GLcontext *,  GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_Vertex3fv(__GLcontext *,  const GLfloat *);
extern GLvoid APIENTRY __gllc_Vertex3i(__GLcontext *,  GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_Vertex3iv(__GLcontext *,  const GLint *);
extern GLvoid APIENTRY __gllc_Vertex3s(__GLcontext *,  GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_Vertex3sv(__GLcontext *,  const GLshort *);
extern GLvoid APIENTRY __gllc_Vertex4d(__GLcontext *,  GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_Vertex4dv(__GLcontext *,  const GLdouble *);
extern GLvoid APIENTRY __gllc_Vertex4f(__GLcontext *,  GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_Vertex4fv(__GLcontext *,  const GLfloat *);
extern GLvoid APIENTRY __gllc_Vertex4i(__GLcontext *,  GLint, GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_Vertex4iv(__GLcontext *,  const GLint *);
extern GLvoid APIENTRY __gllc_Vertex4s(__GLcontext *,  GLshort, GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_Vertex4sv(__GLcontext *,  const GLshort *);
extern GLvoid APIENTRY __gllc_DrawElements(__GLcontext *,  GLenum, GLsizei, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_ArrayElement(__GLcontext *,  GLint);
extern GLvoid APIENTRY __gllc_DrawArrays(__GLcontext *,  GLenum, GLint, GLsizei);
extern GLvoid APIENTRY __gllc_ClipPlane(__GLcontext *,  GLenum, const GLdouble *);
extern GLvoid APIENTRY __gllc_ColorMaterial(__GLcontext *,  GLenum, GLenum);
extern GLvoid APIENTRY __gllc_CullFace(__GLcontext *,  GLenum);
extern GLvoid APIENTRY __gllc_Fogf(__GLcontext *,  GLenum, GLfloat);
extern GLvoid APIENTRY __gllc_Fogfv(__GLcontext *,  GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_Fogi(__GLcontext *,  GLenum, GLint);
extern GLvoid APIENTRY __gllc_Fogiv(__GLcontext *,  GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_FrontFace(__GLcontext *,  GLenum);
extern GLvoid APIENTRY __gllc_Hint(__GLcontext *,  GLenum, GLenum);
extern GLvoid APIENTRY __gllc_Lightf(__GLcontext *,  GLenum, GLenum, GLfloat);
extern GLvoid APIENTRY __gllc_Lightfv(__GLcontext *,  GLenum, GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_Lighti(__GLcontext *,  GLenum, GLenum, GLint);
extern GLvoid APIENTRY __gllc_Lightiv(__GLcontext *,  GLenum, GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_LightModelf(__GLcontext *,  GLenum, GLfloat);
extern GLvoid APIENTRY __gllc_LightModelfv(__GLcontext *,  GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_LightModeli(__GLcontext *,  GLenum, GLint);
extern GLvoid APIENTRY __gllc_LightModeliv(__GLcontext *,  GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_LineStipple(__GLcontext *,  GLint, GLushort);
extern GLvoid APIENTRY __gllc_LineWidth(__GLcontext *,  GLfloat);
extern GLvoid APIENTRY __gllc_Materialf(__GLcontext *,  GLenum, GLenum, GLfloat);
extern GLvoid APIENTRY __gllc_Materialfv(__GLcontext *,  GLenum, GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_Materiali(__GLcontext *,  GLenum, GLenum, GLint);
extern GLvoid APIENTRY __gllc_Materialiv(__GLcontext *,  GLenum, GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_PointSize(__GLcontext *,  GLfloat);
extern GLvoid APIENTRY __gllc_PolygonMode(__GLcontext *,  GLenum, GLenum);
extern GLvoid APIENTRY __gllc_PolygonStipple(__GLcontext *,  const GLubyte *);
extern GLvoid APIENTRY __gllc_Scissor(__GLcontext *,  GLint, GLint, GLsizei, GLsizei);
extern GLvoid APIENTRY __gllc_ShadeModel(__GLcontext *,  GLenum);
extern GLvoid APIENTRY __gllc_TexParameterf(__GLcontext *,  GLenum, GLenum, GLfloat);
extern GLvoid APIENTRY __gllc_TexParameterfv(__GLcontext *,  GLenum, GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_TexParameteri(__GLcontext *,  GLenum, GLenum, GLint);
extern GLvoid APIENTRY __gllc_TexParameteriv(__GLcontext *,  GLenum, GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_TexImage1D(__GLcontext *,  GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_TexImage2D(__GLcontext *,  GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_TexEnvf(__GLcontext *,  GLenum, GLenum, GLfloat);
extern GLvoid APIENTRY __gllc_TexEnvfv(__GLcontext *,  GLenum, GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_TexEnvi(__GLcontext *,  GLenum, GLenum, GLint);
extern GLvoid APIENTRY __gllc_TexEnviv(__GLcontext *,  GLenum, GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_TexGend(__GLcontext *,  GLenum, GLenum, GLdouble);
extern GLvoid APIENTRY __gllc_TexGendv(__GLcontext *,  GLenum, GLenum, const GLdouble *);
extern GLvoid APIENTRY __gllc_TexGenf(__GLcontext *,  GLenum, GLenum, GLfloat);
extern GLvoid APIENTRY __gllc_TexGenfv(__GLcontext *,  GLenum, GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_TexGeni(__GLcontext *,  GLenum, GLenum, GLint);
extern GLvoid APIENTRY __gllc_TexGeniv(__GLcontext *,  GLenum, GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_FeedbackBuffer(__GLcontext *,  GLsizei, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_SelectBuffer(__GLcontext *,  GLsizei, GLuint *);
extern GLint APIENTRY __gllc_RenderMode(__GLcontext *,  GLenum);
extern GLvoid APIENTRY __gllc_InitNames(__GLcontext *);
extern GLvoid APIENTRY __gllc_LoadName(__GLcontext *,  GLuint);
extern GLvoid APIENTRY __gllc_PassThrough(__GLcontext *,  GLfloat);
extern GLvoid APIENTRY __gllc_PopName(__GLcontext *);
extern GLvoid APIENTRY __gllc_PushName(__GLcontext *,  GLuint);
extern GLvoid APIENTRY __gllc_DrawBuffer(__GLcontext *,  GLenum);
extern GLvoid APIENTRY __gllc_Clear(__GLcontext *,  GLbitfield);
extern GLvoid APIENTRY __gllc_ClearAccum(__GLcontext *,  GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_ClearIndex(__GLcontext *,  GLfloat);
extern GLvoid APIENTRY __gllc_ClearColor(__GLcontext *,  GLclampf, GLclampf, GLclampf, GLclampf);
extern GLvoid APIENTRY __gllc_ClearStencil(__GLcontext *,  GLint);
extern GLvoid APIENTRY __gllc_ClearDepth(__GLcontext *,  GLclampd);
extern GLvoid APIENTRY __gllc_StencilMask(__GLcontext *,  GLuint);
extern GLvoid APIENTRY __gllc_ColorMask(__GLcontext *,  GLboolean, GLboolean, GLboolean, GLboolean);
extern GLvoid APIENTRY __gllc_DepthMask(__GLcontext *,  GLboolean);
extern GLvoid APIENTRY __gllc_IndexMask(__GLcontext *,  GLuint);
extern GLvoid APIENTRY __gllc_Accum(__GLcontext *,  GLenum, GLfloat);
extern GLvoid APIENTRY __gllc_Disable(__GLcontext *,  GLenum);
extern GLvoid APIENTRY __gllc_Enable(__GLcontext *,  GLenum);
extern GLvoid APIENTRY __gllc_Finish(__GLcontext *);
extern GLvoid APIENTRY __gllc_Flush(__GLcontext *);
extern GLvoid APIENTRY __gllc_PopAttrib(__GLcontext *);
extern GLvoid APIENTRY __gllc_PushAttrib(__GLcontext *,  GLbitfield);
extern GLvoid APIENTRY __gllc_Map1d(__GLcontext *,  GLenum, GLdouble, GLdouble, GLint, GLint, const GLdouble *);
extern GLvoid APIENTRY __gllc_Map1f(__GLcontext *,  GLenum, GLfloat, GLfloat, GLint, GLint, const GLfloat *);
extern GLvoid APIENTRY __gllc_Map2d(__GLcontext *,  GLenum, GLdouble, GLdouble, GLint, GLint, GLdouble, GLdouble, GLint, GLint, const GLdouble *);
extern GLvoid APIENTRY __gllc_Map2f(__GLcontext *,  GLenum, GLfloat, GLfloat, GLint, GLint, GLfloat, GLfloat, GLint, GLint, const GLfloat *);
extern GLvoid APIENTRY __gllc_MapGrid1d(__GLcontext *,  GLint, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_MapGrid1f(__GLcontext *,  GLint, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_MapGrid2d(__GLcontext *,  GLint, GLdouble, GLdouble, GLint, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_MapGrid2f(__GLcontext *,  GLint, GLfloat, GLfloat, GLint, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_EvalCoord1d(__GLcontext *,  GLdouble);
extern GLvoid APIENTRY __gllc_EvalCoord1dv(__GLcontext *,  const GLdouble *);
extern GLvoid APIENTRY __gllc_EvalCoord1f(__GLcontext *,  GLfloat);
extern GLvoid APIENTRY __gllc_EvalCoord1fv(__GLcontext *,  const GLfloat *);
extern GLvoid APIENTRY __gllc_EvalCoord2d(__GLcontext *,  GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_EvalCoord2dv(__GLcontext *,  const GLdouble *);
extern GLvoid APIENTRY __gllc_EvalCoord2f(__GLcontext *,  GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_EvalCoord2fv(__GLcontext *,  const GLfloat *);
extern GLvoid APIENTRY __gllc_EvalMesh1(__GLcontext *,  GLenum, GLint, GLint);
extern GLvoid APIENTRY __gllc_EvalPoint1(__GLcontext *,  GLint);
extern GLvoid APIENTRY __gllc_EvalMesh2(__GLcontext *,  GLenum, GLint, GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_EvalPoint2(__GLcontext *,  GLint, GLint);
extern GLvoid APIENTRY __gllc_AlphaFunc(__GLcontext *,  GLenum, GLclampf);
extern GLvoid APIENTRY __gllc_BlendFunc(__GLcontext *,  GLenum, GLenum);
extern GLvoid APIENTRY __gllc_LogicOp(__GLcontext *,  GLenum);
extern GLvoid APIENTRY __gllc_StencilFunc(__GLcontext *,  GLenum, GLint, GLuint);
extern GLvoid APIENTRY __gllc_StencilOp(__GLcontext *,  GLenum, GLenum, GLenum);
extern GLvoid APIENTRY __gllc_DepthFunc(__GLcontext *,  GLenum);
extern GLvoid APIENTRY __gllc_PixelZoom(__GLcontext *,  GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_PixelTransferf(__GLcontext *,  GLenum, GLfloat);
extern GLvoid APIENTRY __gllc_PixelTransferi(__GLcontext *,  GLenum, GLint);
extern GLvoid APIENTRY __gllc_PixelStoref(__GLcontext *,  GLenum, GLfloat);
extern GLvoid APIENTRY __gllc_PixelStorei(__GLcontext *,  GLenum, GLint);
extern GLvoid APIENTRY __gllc_PixelMapfv(__GLcontext *,  GLenum, GLint, const GLfloat *);
extern GLvoid APIENTRY __gllc_PixelMapuiv(__GLcontext *,  GLenum, GLint, const GLuint *);
extern GLvoid APIENTRY __gllc_PixelMapusv(__GLcontext *,  GLenum, GLint, const GLushort *);
extern GLvoid APIENTRY __gllc_ReadBuffer(__GLcontext *,  GLenum);
extern GLvoid APIENTRY __gllc_CopyPixels(__GLcontext *,  GLint, GLint, GLsizei, GLsizei, GLenum);
extern GLvoid APIENTRY __gllc_ReadPixels(__GLcontext *,  GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid *);
extern GLvoid APIENTRY __gllc_DrawPixels(__GLcontext *,  GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_GetBooleanv(__GLcontext *,  GLenum, GLboolean *);
extern GLvoid APIENTRY __gllc_GetClipPlane(__GLcontext *,  GLenum, GLdouble *);
extern GLvoid APIENTRY __gllc_GetDoublev(__GLcontext *,  GLenum, GLdouble *);
extern GLenum APIENTRY __gllc_GetError(__GLcontext *);
extern GLvoid APIENTRY __gllc_GetFloatv(__GLcontext *,  GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetIntegerv(__GLcontext *,  GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetLightfv(__GLcontext *,  GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetLightiv(__GLcontext *,  GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetMapdv(__GLcontext *,  GLenum, GLenum, GLdouble *);
extern GLvoid APIENTRY __gllc_GetMapfv(__GLcontext *,  GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetMapiv(__GLcontext *,  GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetMaterialfv(__GLcontext *,  GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetMaterialiv(__GLcontext *,  GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetPixelMapfv(__GLcontext *,  GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetPixelMapuiv(__GLcontext *,  GLenum, GLuint *);
extern GLvoid APIENTRY __gllc_GetPixelMapusv(__GLcontext *,  GLenum, GLushort *);
extern GLvoid APIENTRY __gllc_GetPolygonStipple(__GLcontext *,  GLubyte *);
extern const GLubyte * APIENTRY __gllc_GetString(__GLcontext *,  GLenum);
extern GLvoid APIENTRY __gllc_GetTexEnvfv(__GLcontext *,  GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetTexEnviv(__GLcontext *,  GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetTexGendv(__GLcontext *,  GLenum, GLenum, GLdouble *);
extern GLvoid APIENTRY __gllc_GetTexGenfv(__GLcontext *,  GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetTexGeniv(__GLcontext *,  GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetTexImage(__GLcontext *,  GLenum, GLint, GLenum, GLenum, GLvoid *);
extern GLvoid APIENTRY __gllc_GetTexParameterfv(__GLcontext *,  GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetTexParameteriv(__GLcontext *,  GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetTexLevelParameterfv(__GLcontext *,  GLenum, GLint, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetTexLevelParameteriv(__GLcontext *,  GLenum, GLint, GLenum, GLint *);
extern GLboolean APIENTRY __gllc_IsEnabled(__GLcontext *,  GLenum);
extern GLboolean APIENTRY __gllc_IsList(__GLcontext *,  GLuint);
extern GLvoid APIENTRY __gllc_DepthRange(__GLcontext *,  GLclampd, GLclampd);
extern GLvoid APIENTRY __gllc_Frustum(__GLcontext *,  GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_LoadIdentity(__GLcontext *);
extern GLvoid APIENTRY __gllc_LoadMatrixf(__GLcontext *,  const GLfloat *);
extern GLvoid APIENTRY __gllc_LoadMatrixd(__GLcontext *,  const GLdouble *);
extern GLvoid APIENTRY __gllc_MatrixMode(__GLcontext *,  GLenum);
extern GLvoid APIENTRY __gllc_MultMatrixf(__GLcontext *,  const GLfloat *);
extern GLvoid APIENTRY __gllc_MultMatrixd(__GLcontext *,  const GLdouble *);
extern GLvoid APIENTRY __gllc_Ortho(__GLcontext *,  GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_PopMatrix(__GLcontext *);
extern GLvoid APIENTRY __gllc_PushMatrix(__GLcontext *);
extern GLvoid APIENTRY __gllc_Rotated(__GLcontext *,  GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_Rotatef(__GLcontext *,  GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_Scaled(__GLcontext *,  GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_Scalef(__GLcontext *,  GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_Translated(__GLcontext *,  GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_Translatef(__GLcontext *,  GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_Viewport(__GLcontext *,  GLint, GLint, GLsizei, GLsizei);
extern GLvoid APIENTRY __gllc_ColorSubTable(__GLcontext *,  GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_ColorTable(__GLcontext *,  GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_GetColorTable(__GLcontext *,  GLenum, GLenum, GLenum, GLvoid *);
extern GLvoid APIENTRY __gllc_GetColorTableParameterfv(__GLcontext *,  GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetColorTableParameteriv(__GLcontext *,  GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_ColorPointer(__GLcontext *,  GLint, GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_DisableClientState(__GLcontext *,  GLenum);
extern GLvoid APIENTRY __gllc_EdgeFlagPointer(__GLcontext *,  GLsizei, const GLboolean *);
extern GLvoid APIENTRY __gllc_EnableClientState(__GLcontext *,  GLenum);
extern GLvoid APIENTRY __gllc_GetPointerv(__GLcontext *,  GLenum, GLvoid* *);
extern GLvoid APIENTRY __gllc_IndexPointer(__GLcontext *,  GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_InterleavedArrays(__GLcontext *,  GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_NormalPointer(__GLcontext *,  GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_TexCoordPointer(__GLcontext *,  GLint, GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_VertexPointer(__GLcontext *,  GLint, GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_PolygonOffset(__GLcontext *,  GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_CopyTexImage1D(__GLcontext *,  GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLint);
extern GLvoid APIENTRY __gllc_CopyTexImage2D(__GLcontext *,  GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint);
extern GLvoid APIENTRY __gllc_CopyTexSubImage1D(__GLcontext *,  GLenum, GLint, GLint, GLint, GLint, GLsizei);
extern GLvoid APIENTRY __gllc_CopyTexSubImage2D(__GLcontext *,  GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
extern GLvoid APIENTRY __gllc_TexSubImage1D(__GLcontext *,  GLenum, GLint, GLint, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_TexSubImage2D(__GLcontext *,  GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLboolean APIENTRY __gllc_AreTexturesResident(__GLcontext *,  GLsizei, const GLuint *, GLboolean *);
extern GLvoid APIENTRY __gllc_BindTexture(__GLcontext *,  GLenum, GLuint);
extern GLvoid APIENTRY __gllc_DeleteTextures(__GLcontext *,  GLsizei, const GLuint *);
extern GLvoid APIENTRY __gllc_GenTextures(__GLcontext *,  GLsizei, GLuint *);
extern GLboolean APIENTRY __gllc_IsTexture(__GLcontext *,  GLuint);
extern GLvoid APIENTRY __gllc_PrioritizeTextures(__GLcontext *,  GLsizei, const GLuint *, const GLclampf *);
extern GLvoid APIENTRY __gllc_Indexub(__GLcontext *,  GLubyte);
extern GLvoid APIENTRY __gllc_Indexubv(__GLcontext *,  const GLubyte *);
extern GLvoid APIENTRY __gllc_PopClientAttrib(__GLcontext *);
extern GLvoid APIENTRY __gllc_PushClientAttrib(__GLcontext *,  GLbitfield);

#if GL_VERSION_1_2
extern GLvoid APIENTRY __gllc_BlendColor(__GLcontext *,  GLclampf, GLclampf, GLclampf, GLclampf);
extern GLvoid APIENTRY __gllc_BlendEquation(__GLcontext *,  GLenum);
extern GLvoid APIENTRY __gllc_DrawRangeElements(__GLcontext *,  GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_ColorTable(__GLcontext *,  GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_ColorTableParameterfv(__GLcontext *,  GLenum, GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_ColorTableParameteriv(__GLcontext *,  GLenum, GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_CopyColorTable(__GLcontext *,  GLenum, GLenum, GLint, GLint, GLsizei);
extern GLvoid APIENTRY __gllc_GetColorTable(__GLcontext *,  GLenum, GLenum, GLenum, GLvoid *);
extern GLvoid APIENTRY __gllc_GetColorTableParameterfv(__GLcontext *,  GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetColorTableParameteriv(__GLcontext *,  GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_ColorSubTable(__GLcontext *,  GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_CopyColorSubTable(__GLcontext *,  GLenum, GLsizei, GLint, GLint, GLsizei);
extern GLvoid APIENTRY __gllc_ConvolutionFilter1D(__GLcontext *,  GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_ConvolutionFilter2D(__GLcontext *,  GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_ConvolutionParameterf(__GLcontext *,  GLenum, GLenum, GLfloat);
extern GLvoid APIENTRY __gllc_ConvolutionParameterfv(__GLcontext *,  GLenum, GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_ConvolutionParameteri(__GLcontext *,  GLenum, GLenum, GLint);
extern GLvoid APIENTRY __gllc_ConvolutionParameteriv(__GLcontext *,  GLenum, GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_CopyConvolutionFilter1D(__GLcontext *,  GLenum, GLenum, GLint, GLint, GLsizei);
extern GLvoid APIENTRY __gllc_CopyConvolutionFilter2D(__GLcontext *,  GLenum, GLenum, GLint, GLint, GLsizei, GLsizei);
extern GLvoid APIENTRY __gllc_GetConvolutionFilter(__GLcontext *,  GLenum, GLenum, GLenum, GLvoid *);
extern GLvoid APIENTRY __gllc_GetConvolutionParameterfv(__GLcontext *,  GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetConvolutionParameteriv(__GLcontext *,  GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetSeparableFilter(__GLcontext *,  GLenum, GLenum, GLenum, GLvoid *, GLvoid *, GLvoid *);
extern GLvoid APIENTRY __gllc_SeparableFilter2D(__GLcontext *,  GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *, const GLvoid *);
extern GLvoid APIENTRY __gllc_GetHistogram(__GLcontext *,  GLenum, GLboolean, GLenum, GLenum, GLvoid *);
extern GLvoid APIENTRY __gllc_GetHistogramParameterfv(__GLcontext *,  GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetHistogramParameteriv(__GLcontext *,  GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetMinmax(__GLcontext *,  GLenum, GLboolean, GLenum, GLenum, GLvoid *);
extern GLvoid APIENTRY __gllc_GetMinmaxParameterfv(__GLcontext *,  GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetMinmaxParameteriv(__GLcontext *,  GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_Histogram(__GLcontext *,  GLenum, GLsizei, GLenum, GLboolean);
extern GLvoid APIENTRY __gllc_Minmax(__GLcontext *,  GLenum, GLenum, GLboolean);
extern GLvoid APIENTRY __gllc_ResetHistogram(__GLcontext *,  GLenum);
extern GLvoid APIENTRY __gllc_ResetMinmax(__GLcontext *,  GLenum);
extern GLvoid APIENTRY __gllc_TexImage3D(__GLcontext *,  GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_TexSubImage3D(__GLcontext *,  GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_CopyTexSubImage3D(__GLcontext *,  GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
#endif
#if GL_VERSION_1_3
extern GLvoid APIENTRY __gllc_ActiveTexture(__GLcontext *,  GLenum);
extern GLvoid APIENTRY __gllc_ClientActiveTexture(__GLcontext *,  GLenum);
extern GLvoid APIENTRY __gllc_MultiTexCoord1d(__GLcontext *,  GLenum, GLdouble);
extern GLvoid APIENTRY __gllc_MultiTexCoord1dv(__GLcontext *,  GLenum, const GLdouble *);
extern GLvoid APIENTRY __gllc_MultiTexCoord1f(__GLcontext *,  GLenum, GLfloat);
extern GLvoid APIENTRY __gllc_MultiTexCoord1fv(__GLcontext *,  GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_MultiTexCoord1i(__GLcontext *,  GLenum, GLint);
extern GLvoid APIENTRY __gllc_MultiTexCoord1iv(__GLcontext *,  GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_MultiTexCoord1s(__GLcontext *,  GLenum, GLshort);
extern GLvoid APIENTRY __gllc_MultiTexCoord1sv(__GLcontext *,  GLenum, const GLshort *);
extern GLvoid APIENTRY __gllc_MultiTexCoord2d(__GLcontext *,  GLenum, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_MultiTexCoord2dv(__GLcontext *,  GLenum, const GLdouble *);
extern GLvoid APIENTRY __gllc_MultiTexCoord2f(__GLcontext *,  GLenum, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_MultiTexCoord2fv(__GLcontext *,  GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_MultiTexCoord2i(__GLcontext *,  GLenum, GLint, GLint);
extern GLvoid APIENTRY __gllc_MultiTexCoord2iv(__GLcontext *,  GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_MultiTexCoord2s(__GLcontext *,  GLenum, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_MultiTexCoord2sv(__GLcontext *,  GLenum, const GLshort *);
extern GLvoid APIENTRY __gllc_MultiTexCoord3d(__GLcontext *,  GLenum, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_MultiTexCoord3dv(__GLcontext *,  GLenum, const GLdouble *);
extern GLvoid APIENTRY __gllc_MultiTexCoord3f(__GLcontext *,  GLenum, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_MultiTexCoord3fv(__GLcontext *,  GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_MultiTexCoord3i(__GLcontext *,  GLenum, GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_MultiTexCoord3iv(__GLcontext *,  GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_MultiTexCoord3s(__GLcontext *,  GLenum, GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_MultiTexCoord3sv(__GLcontext *,  GLenum, const GLshort *);
extern GLvoid APIENTRY __gllc_MultiTexCoord4d(__GLcontext *,  GLenum, GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_MultiTexCoord4dv(__GLcontext *,  GLenum, const GLdouble *);
extern GLvoid APIENTRY __gllc_MultiTexCoord4f(__GLcontext *,  GLenum, GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_MultiTexCoord4fv(__GLcontext *,  GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_MultiTexCoord4i(__GLcontext *,  GLenum, GLint, GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_MultiTexCoord4iv(__GLcontext *,  GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_MultiTexCoord4s(__GLcontext *,  GLenum, GLshort, GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_MultiTexCoord4sv(__GLcontext *,  GLenum, const GLshort *);
extern GLvoid APIENTRY __gllc_LoadTransposeMatrixf(__GLcontext *,  const GLfloat *);
extern GLvoid APIENTRY __gllc_LoadTransposeMatrixd(__GLcontext *,  const GLdouble *);
extern GLvoid APIENTRY __gllc_MultTransposeMatrixf(__GLcontext *,  const GLfloat *);
extern GLvoid APIENTRY __gllc_MultTransposeMatrixd(__GLcontext *,  const GLdouble *);
extern GLvoid APIENTRY __gllc_SampleCoverage(__GLcontext *,  GLclampf, GLboolean);
extern GLvoid APIENTRY __gllc_CompressedTexImage3D(__GLcontext *,  GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_CompressedTexImage2D(__GLcontext *,  GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_CompressedTexImage1D(__GLcontext *,  GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_CompressedTexSubImage3D(__GLcontext *,  GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_CompressedTexSubImage2D(__GLcontext *,  GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_CompressedTexSubImage1D(__GLcontext *,  GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_GetCompressedTexImage(__GLcontext *,  GLenum, GLint, GLvoid *);
#endif
#if GL_VERSION_1_4
extern GLvoid APIENTRY __gllc_BlendFuncSeparate(__GLcontext *,  GLenum, GLenum, GLenum, GLenum);
extern GLvoid APIENTRY __gllc_FogCoordf(__GLcontext *,  GLfloat);
extern GLvoid APIENTRY __gllc_FogCoordfv(__GLcontext *,  const GLfloat *);
extern GLvoid APIENTRY __gllc_FogCoordd(__GLcontext *,  GLdouble);
extern GLvoid APIENTRY __gllc_FogCoorddv(__GLcontext *,  const GLdouble *);
extern GLvoid APIENTRY __gllc_FogCoordPointer(__GLcontext *,  GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_MultiDrawArrays(__GLcontext *,  GLenum, GLint *, GLsizei *, GLsizei);
extern GLvoid APIENTRY __gllc_MultiDrawElements(__GLcontext *,  GLenum, const GLsizei *, GLenum, const GLvoid* *, GLsizei);
extern GLvoid APIENTRY __gllc_PointParameterf(__GLcontext *,  GLenum, GLfloat);
extern GLvoid APIENTRY __gllc_PointParameterfv(__GLcontext *,  GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_PointParameteri(__GLcontext *,  GLenum, GLint);
extern GLvoid APIENTRY __gllc_PointParameteriv(__GLcontext *,  GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_SecondaryColor3b(__GLcontext *,  GLbyte, GLbyte, GLbyte);
extern GLvoid APIENTRY __gllc_SecondaryColor3bv(__GLcontext *,  const GLbyte *);
extern GLvoid APIENTRY __gllc_SecondaryColor3d(__GLcontext *,  GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_SecondaryColor3dv(__GLcontext *,  const GLdouble *);
extern GLvoid APIENTRY __gllc_SecondaryColor3f(__GLcontext *,  GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_SecondaryColor3fv(__GLcontext *,  const GLfloat *);
extern GLvoid APIENTRY __gllc_SecondaryColor3i(__GLcontext *,  GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_SecondaryColor3iv(__GLcontext *,  const GLint *);
extern GLvoid APIENTRY __gllc_SecondaryColor3s(__GLcontext *,  GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_SecondaryColor3sv(__GLcontext *,  const GLshort *);
extern GLvoid APIENTRY __gllc_SecondaryColor3ub(__GLcontext *,  GLubyte, GLubyte, GLubyte);
extern GLvoid APIENTRY __gllc_SecondaryColor3ubv(__GLcontext *,  const GLubyte *);
extern GLvoid APIENTRY __gllc_SecondaryColor3ui(__GLcontext *,  GLuint, GLuint, GLuint);
extern GLvoid APIENTRY __gllc_SecondaryColor3uiv(__GLcontext *,  const GLuint *);
extern GLvoid APIENTRY __gllc_SecondaryColor3us(__GLcontext *,  GLushort, GLushort, GLushort);
extern GLvoid APIENTRY __gllc_SecondaryColor3usv(__GLcontext *,  const GLushort *);
extern GLvoid APIENTRY __gllc_SecondaryColorPointer(__GLcontext *,  GLint, GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_WindowPos2d(__GLcontext *,  GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_WindowPos2dv(__GLcontext *,  const GLdouble *);
extern GLvoid APIENTRY __gllc_WindowPos2f(__GLcontext *,  GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_WindowPos2fv(__GLcontext *,  const GLfloat *);
extern GLvoid APIENTRY __gllc_WindowPos2i(__GLcontext *,  GLint, GLint);
extern GLvoid APIENTRY __gllc_WindowPos2iv(__GLcontext *,  const GLint *);
extern GLvoid APIENTRY __gllc_WindowPos2s(__GLcontext *,  GLshort, GLshort);
extern GLvoid APIENTRY __gllc_WindowPos2sv(__GLcontext *,  const GLshort *);
extern GLvoid APIENTRY __gllc_WindowPos3d(__GLcontext *,  GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_WindowPos3dv(__GLcontext *,  const GLdouble *);
extern GLvoid APIENTRY __gllc_WindowPos3f(__GLcontext *,  GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_WindowPos3fv(__GLcontext *,  const GLfloat *);
extern GLvoid APIENTRY __gllc_WindowPos3i(__GLcontext *,  GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_WindowPos3iv(__GLcontext *,  const GLint *);
extern GLvoid APIENTRY __gllc_WindowPos3s(__GLcontext *,  GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_WindowPos3sv(__GLcontext *,  const GLshort *);
#endif
#if GL_VERSION_1_5
extern GLvoid APIENTRY __gllc_GenQueries(__GLcontext *,  GLsizei, GLuint *);
extern GLvoid APIENTRY __gllc_DeleteQueries(__GLcontext *,  GLsizei, const GLuint *);
extern GLboolean APIENTRY __gllc_IsQuery(__GLcontext *,  GLuint);
extern GLvoid APIENTRY __gllc_BeginQuery(__GLcontext *,  GLenum, GLuint);
extern GLvoid APIENTRY __gllc_EndQuery(__GLcontext *,  GLenum);
extern GLvoid APIENTRY __gllc_GetQueryiv(__GLcontext *,  GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetQueryObjectiv(__GLcontext *,  GLuint, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetQueryObjectuiv(__GLcontext *,  GLuint, GLenum, GLuint *);
extern GLvoid APIENTRY __gllc_BindBuffer(__GLcontext *,  GLenum, GLuint);
extern GLvoid APIENTRY __gllc_DeleteBuffers(__GLcontext *,  GLsizei, const GLuint *);
extern GLvoid APIENTRY __gllc_GenBuffers(__GLcontext *,  GLsizei, GLuint *);
extern GLboolean APIENTRY __gllc_IsBuffer(__GLcontext *,  GLuint);
extern GLvoid APIENTRY __gllc_BufferData(__GLcontext *,  GLenum, GLsizeiptr, const GLvoid *, GLenum);
extern GLvoid APIENTRY __gllc_BufferSubData(__GLcontext *,  GLenum, GLintptr, GLsizeiptr, const GLvoid *);
extern GLvoid APIENTRY __gllc_GetBufferSubData(__GLcontext *,  GLenum, GLintptr, GLsizeiptr, GLvoid *);
extern GLvoid* APIENTRY __gllc_MapBuffer(__GLcontext *,  GLenum, GLenum);
extern GLboolean APIENTRY __gllc_UnmapBuffer(__GLcontext *,  GLenum);
extern GLvoid APIENTRY __gllc_GetBufferParameteriv(__GLcontext *,  GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetBufferPointerv(__GLcontext *,  GLenum, GLenum, GLvoid* *);
#endif
#if GL_VERSION_2_0
extern GLvoid APIENTRY __gllc_BlendEquationSeparate(__GLcontext *,  GLenum, GLenum);
extern GLvoid APIENTRY __gllc_DrawBuffers(__GLcontext *,  GLsizei, const GLenum *);
extern GLvoid APIENTRY __gllc_StencilOpSeparate(__GLcontext *,  GLenum, GLenum, GLenum, GLenum);
extern GLvoid APIENTRY __gllc_StencilFuncSeparate(__GLcontext *,  GLenum, GLenum, GLint, GLuint);
extern GLvoid APIENTRY __gllc_StencilMaskSeparate(__GLcontext *,  GLenum, GLuint);
extern GLvoid APIENTRY __gllc_AttachShader(__GLcontext *,  GLuint, GLuint);
extern GLvoid APIENTRY __gllc_BindAttribLocation(__GLcontext *,  GLuint, GLuint, const GLchar *);
extern GLvoid APIENTRY __gllc_CompileShader(__GLcontext *,  GLuint);
extern GLuint APIENTRY __gllc_CreateProgram (__GLcontext *);
extern GLuint APIENTRY __gllc_CreateShader(__GLcontext *,  GLenum);
extern GLvoid APIENTRY __gllc_DeleteProgram(__GLcontext *,  GLuint);
extern GLvoid APIENTRY __gllc_DeleteShader(__GLcontext *,  GLuint);
extern GLvoid APIENTRY __gllc_DetachShader(__GLcontext *,  GLuint, GLuint);
extern GLvoid APIENTRY __gllc_DisableVertexAttribArray(__GLcontext *,  GLuint);
extern GLvoid APIENTRY __gllc_EnableVertexAttribArray(__GLcontext *,  GLuint);
extern GLvoid APIENTRY __gllc_GetActiveAttrib(__GLcontext *,  GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *);
extern GLvoid APIENTRY __gllc_GetActiveUniform(__GLcontext *,  GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *);
extern GLvoid APIENTRY __gllc_GetAttachedShaders(__GLcontext *,  GLuint, GLsizei, GLsizei *, GLuint *);
extern GLint APIENTRY __gllc_GetAttribLocation(__GLcontext *,  GLuint, const GLchar *);
extern GLvoid APIENTRY __gllc_GetProgramiv(__GLcontext *,  GLuint, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetProgramInfoLog(__GLcontext *,  GLuint, GLsizei, GLsizei *, GLchar *);
extern GLvoid APIENTRY __gllc_GetShaderiv(__GLcontext *,  GLuint, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetShaderInfoLog(__GLcontext *,  GLuint, GLsizei, GLsizei *, GLchar *);
extern GLvoid APIENTRY __gllc_GetShaderSource(__GLcontext *,  GLuint, GLsizei, GLsizei *, GLchar *);
extern GLint APIENTRY __gllc_GetUniformLocation(__GLcontext *,  GLuint, const GLchar *);
extern GLvoid APIENTRY __gllc_GetUniformfv(__GLcontext *,  GLuint, GLint, GLfloat *);
extern GLvoid APIENTRY __gllc_GetUniformiv(__GLcontext *,  GLuint, GLint, GLint *);
extern GLvoid APIENTRY __gllc_GetVertexAttribdv(__GLcontext *,  GLuint, GLenum, GLdouble *);
extern GLvoid APIENTRY __gllc_GetVertexAttribfv(__GLcontext *,  GLuint, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetVertexAttribiv(__GLcontext *,  GLuint, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetVertexAttribPointerv(__GLcontext *,  GLuint, GLenum, GLvoid* *);
extern GLboolean APIENTRY __gllc_IsProgram(__GLcontext *,  GLuint);
extern GLboolean APIENTRY __gllc_IsShader(__GLcontext *,  GLuint);
extern GLvoid APIENTRY __gllc_LinkProgram(__GLcontext *,  GLuint);
extern GLvoid APIENTRY __gllc_ShaderSource(__GLcontext *,  GLuint, GLsizei, const GLchar* *, const GLint *);
extern GLvoid APIENTRY __gllc_UseProgram(__GLcontext *,  GLuint);
extern GLvoid APIENTRY __gllc_Uniform1f(__GLcontext *,  GLint, GLfloat);
extern GLvoid APIENTRY __gllc_Uniform2f(__GLcontext *,  GLint, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_Uniform3f(__GLcontext *,  GLint, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_Uniform4f(__GLcontext *,  GLint, GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_Uniform1i(__GLcontext *,  GLint, GLint);
extern GLvoid APIENTRY __gllc_Uniform2i(__GLcontext *,  GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_Uniform3i(__GLcontext *,  GLint, GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_Uniform4i(__GLcontext *,  GLint, GLint, GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_Uniform1fv(__GLcontext *,  GLint, GLsizei, const GLfloat *);
extern GLvoid APIENTRY __gllc_Uniform2fv(__GLcontext *,  GLint, GLsizei, const GLfloat *);
extern GLvoid APIENTRY __gllc_Uniform3fv(__GLcontext *,  GLint, GLsizei, const GLfloat *);
extern GLvoid APIENTRY __gllc_Uniform4fv(__GLcontext *,  GLint, GLsizei, const GLfloat *);
extern GLvoid APIENTRY __gllc_Uniform1iv(__GLcontext *,  GLint, GLsizei, const GLint *);
extern GLvoid APIENTRY __gllc_Uniform2iv(__GLcontext *,  GLint, GLsizei, const GLint *);
extern GLvoid APIENTRY __gllc_Uniform3iv(__GLcontext *,  GLint, GLsizei, const GLint *);
extern GLvoid APIENTRY __gllc_Uniform4iv(__GLcontext *,  GLint, GLsizei, const GLint *);
extern GLvoid APIENTRY __gllc_UniformMatrix2fv(__GLcontext *,  GLint, GLsizei, GLboolean, const GLfloat *);
extern GLvoid APIENTRY __gllc_UniformMatrix3fv(__GLcontext *,  GLint, GLsizei, GLboolean, const GLfloat *);
extern GLvoid APIENTRY __gllc_UniformMatrix4fv(__GLcontext *,  GLint, GLsizei, GLboolean, const GLfloat *);
extern GLvoid APIENTRY __gllc_ValidateProgram(__GLcontext *,  GLuint);
extern GLvoid APIENTRY __gllc_VertexAttrib1d(__GLcontext *,  GLuint, GLdouble);
extern GLvoid APIENTRY __gllc_VertexAttrib1dv(__GLcontext *,  GLuint, const GLdouble *);
extern GLvoid APIENTRY __gllc_VertexAttrib1f(__GLcontext *,  GLuint, GLfloat);
extern GLvoid APIENTRY __gllc_VertexAttrib1fv(__GLcontext *,  GLuint, const GLfloat *);
extern GLvoid APIENTRY __gllc_VertexAttrib1s(__GLcontext *,  GLuint, GLshort);
extern GLvoid APIENTRY __gllc_VertexAttrib1sv(__GLcontext *,  GLuint, const GLshort *);
extern GLvoid APIENTRY __gllc_VertexAttrib2d(__GLcontext *,  GLuint, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_VertexAttrib2dv(__GLcontext *,  GLuint, const GLdouble *);
extern GLvoid APIENTRY __gllc_VertexAttrib2f(__GLcontext *,  GLuint, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_VertexAttrib2fv(__GLcontext *,  GLuint, const GLfloat *);
extern GLvoid APIENTRY __gllc_VertexAttrib2s(__GLcontext *,  GLuint, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_VertexAttrib2sv(__GLcontext *,  GLuint, const GLshort *);
extern GLvoid APIENTRY __gllc_VertexAttrib3d(__GLcontext *,  GLuint, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_VertexAttrib3dv(__GLcontext *,  GLuint, const GLdouble *);
extern GLvoid APIENTRY __gllc_VertexAttrib3f(__GLcontext *,  GLuint, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_VertexAttrib3fv(__GLcontext *,  GLuint, const GLfloat *);
extern GLvoid APIENTRY __gllc_VertexAttrib3s(__GLcontext *,  GLuint, GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_VertexAttrib3sv(__GLcontext *,  GLuint, const GLshort *);
extern GLvoid APIENTRY __gllc_VertexAttrib4Nbv(__GLcontext *,  GLuint, const GLbyte *);
extern GLvoid APIENTRY __gllc_VertexAttrib4Niv(__GLcontext *,  GLuint, const GLint *);
extern GLvoid APIENTRY __gllc_VertexAttrib4Nsv(__GLcontext *,  GLuint, const GLshort *);
extern GLvoid APIENTRY __gllc_VertexAttrib4Nub(__GLcontext *,  GLuint, GLubyte, GLubyte, GLubyte, GLubyte);
extern GLvoid APIENTRY __gllc_VertexAttrib4Nubv(__GLcontext *,  GLuint, const GLubyte *);
extern GLvoid APIENTRY __gllc_VertexAttrib4Nuiv(__GLcontext *,  GLuint, const GLuint *);
extern GLvoid APIENTRY __gllc_VertexAttrib4Nusv(__GLcontext *,  GLuint, const GLushort *);
extern GLvoid APIENTRY __gllc_VertexAttrib4bv(__GLcontext *,  GLuint, const GLbyte *);
extern GLvoid APIENTRY __gllc_VertexAttrib4d(__GLcontext *,  GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_VertexAttrib4dv(__GLcontext *,  GLuint, const GLdouble *);
extern GLvoid APIENTRY __gllc_VertexAttrib4f(__GLcontext *,  GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_VertexAttrib4fv(__GLcontext *,  GLuint, const GLfloat *);
extern GLvoid APIENTRY __gllc_VertexAttrib4iv(__GLcontext *,  GLuint, const GLint *);
extern GLvoid APIENTRY __gllc_VertexAttrib4s(__GLcontext *,  GLuint, GLshort, GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_VertexAttrib4sv(__GLcontext *,  GLuint, const GLshort *);
extern GLvoid APIENTRY __gllc_VertexAttrib4ubv(__GLcontext *,  GLuint, const GLubyte *);
extern GLvoid APIENTRY __gllc_VertexAttrib4uiv(__GLcontext *,  GLuint, const GLuint *);
extern GLvoid APIENTRY __gllc_VertexAttrib4usv(__GLcontext *,  GLuint, const GLushort *);
extern GLvoid APIENTRY __gllc_VertexAttribPointer(__GLcontext *,  GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *);
#endif

#if GL_VERSION_2_1
extern GLvoid APIENTRY __gllc_UniformMatrix2x3fv(__GLcontext *,  GLint, GLsizei, GLboolean, const GLfloat *);
extern GLvoid APIENTRY __gllc_UniformMatrix2x4fv(__GLcontext *,  GLint, GLsizei, GLboolean, const GLfloat *);
extern GLvoid APIENTRY __gllc_UniformMatrix3x2fv(__GLcontext *,  GLint, GLsizei, GLboolean, const GLfloat *);
extern GLvoid APIENTRY __gllc_UniformMatrix3x4fv(__GLcontext *,  GLint, GLsizei, GLboolean, const GLfloat *);
extern GLvoid APIENTRY __gllc_UniformMatrix4x2fv(__GLcontext *,  GLint, GLsizei, GLboolean, const GLfloat *);
extern GLvoid APIENTRY __gllc_UniformMatrix4x3fv(__GLcontext *,  GLint, GLsizei, GLboolean, const GLfloat *);
#endif

#if GL_ARB_vertex_program
extern GLvoid APIENTRY __gllc_ProgramStringARB(__GLcontext *gc,  GLenum target, GLenum format, GLsizei len, const GLvoid *string);
extern GLvoid APIENTRY __gllc_BindProgramARB(__GLcontext *gc,  GLenum target, GLuint program);
extern GLvoid APIENTRY __gllc_DeleteProgramsARB(__GLcontext *gc,  GLsizei n, const GLuint *programs);
extern GLvoid APIENTRY __gllc_GenProgramsARB(__GLcontext *gc,  GLsizei n, GLuint *programs);
extern GLvoid APIENTRY __gllc_ProgramEnvParameter4dARB(__GLcontext *gc,  GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern GLvoid APIENTRY __gllc_ProgramEnvParameter4dvARB(__GLcontext *gc,  GLenum target, GLuint index, const GLdouble *params);
extern GLvoid APIENTRY __gllc_ProgramEnvParameter4fARB(__GLcontext *gc,  GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern GLvoid APIENTRY __gllc_ProgramEnvParameter4fvARB(__GLcontext *gc,  GLenum target, GLuint index, const GLfloat *params);
extern GLvoid APIENTRY __gllc_ProgramLocalParameter4dARB(__GLcontext *gc,  GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern GLvoid APIENTRY __gllc_ProgramLocalParameter4dvARB(__GLcontext *gc,  GLenum target, GLuint index, const GLdouble *params);
extern GLvoid APIENTRY __gllc_ProgramLocalParameter4fARB(__GLcontext *gc,  GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern GLvoid APIENTRY __gllc_ProgramLocalParameter4fvARB(__GLcontext *gc,  GLenum target, GLuint index, const GLfloat *params);
extern GLvoid APIENTRY __gllc_GetProgramEnvParameterdvARB(__GLcontext *gc,  GLenum target, GLuint index, GLdouble *params);
extern GLvoid APIENTRY __gllc_GetProgramEnvParameterfvARB(__GLcontext *gc,  GLenum target, GLuint index, GLfloat *params);
extern GLvoid APIENTRY __gllc_GetProgramLocalParameterdvARB(__GLcontext *gc,  GLenum target, GLuint index, GLdouble *params);
extern GLvoid APIENTRY __gllc_GetProgramLocalParameterfvARB(__GLcontext *gc,  GLenum target, GLuint index, GLfloat *params);
extern GLvoid APIENTRY __gllc_GetProgramivARB(__GLcontext *gc,  GLenum target, GLenum pname, GLint *params);
extern GLvoid APIENTRY __gllc_GetProgramStringARB(__GLcontext *gc,  GLenum target, GLenum pname, GLvoid *string);
extern GLboolean APIENTRY __gllc_IsProgramARB(__GLcontext *gc,  GLuint program);
#endif

#if GL_ARB_shader_objects
extern GLvoid APIENTRY __gllc_DeleteObjectARB(__GLcontext *gc, GLhandleARB obj);
extern GLhandleARB APIENTRY __gllc_GetHandleARB(__GLcontext *gc, GLenum pname);
extern GLvoid APIENTRY __gllc_GetInfoLogARB(__GLcontext *gc, GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
extern GLvoid APIENTRY __gllc_GetObjectParameterfvARB(__GLcontext *gc, GLhandleARB obj, GLenum pname, GLfloat *params);
extern GLvoid APIENTRY __gllc_GetObjectParameterivARB(__GLcontext *gc, GLhandleARB obj, GLenum pname, GLint *params);
#endif

#if GL_ATI_vertex_array_object
extern GLuint APIENTRY __gllc_NewObjectBufferATI(__GLcontext *gc, GLsizei size, const GLvoid *pointer, GLenum usage);
extern GLboolean APIENTRY __gllc_IsObjectBufferATI(__GLcontext *gc, GLuint buffer);
extern GLvoid APIENTRY __gllc_UpdateObjectBufferATI(__GLcontext *gc, GLuint buffer, GLuint offset, GLsizei size, const GLvoid *pointer, GLenum preserve);
extern GLvoid APIENTRY __gllc_GetObjectBufferfvATI(__GLcontext *gc, GLuint buffer, GLenum pname, GLfloat *params);
extern GLvoid APIENTRY __gllc_GetObjectBufferivATI(__GLcontext *gc, GLuint buffer, GLenum pname, GLint *params);
extern GLvoid APIENTRY __gllc_FreeObjectBufferATI(__GLcontext *gc, GLuint buffer);
extern GLvoid APIENTRY __gllc_ArrayObjectATI(__GLcontext *gc, GLenum array, GLint size, GLenum type, GLsizei stride, GLuint buffer, GLuint offset);
extern GLvoid APIENTRY __gllc_GetArrayObjectfvATI(__GLcontext *gc, GLenum array, GLenum pname, GLfloat * params);
extern GLvoid APIENTRY __gllc_GetArrayObjectivATI(__GLcontext *gc, GLenum array, GLenum pname, GLint * params);
#endif

#if GL_ATI_element_array
extern GLvoid APIENTRY __gllc_ElementPointerATI(__GLcontext *gc, GLenum type, const GLvoid *pointer);
extern GLvoid APIENTRY __gllc_DrawElementArrayATI(__GLcontext *gc, GLenum mode, GLsizei count);
extern GLvoid APIENTRY __gllc_DrawRangeElementArrayATI(__GLcontext *gc, GLenum mode, GLuint start, GLuint end, GLsizei count);
#endif

#if GL_EXT_stencil_two_side
extern GLvoid APIENTRY __gllc_ActiveStencilFaceEXT(__GLcontext *gc, GLenum face);
#endif

#if GL_EXT_depth_bounds_test
extern GLvoid APIENTRY __gllc_DepthBoundsEXT(__GLcontext *gc, GLclampd zMin, GLclampd zMax);
#endif

#if GL_NV_occlusion_query
extern GLvoid APIENTRY __gllc_BeginQueryNV(__GLcontext *gc, GLuint);
extern GLvoid APIENTRY __gllc_EndQueryNV(__GLcontext *gc);
#endif


#if GL_EXT_bindable_uniform
extern GLvoid APIENTRY __gllc_UniformBufferEXT(__GLcontext *gc, GLuint program, GLint location, GLuint buffer);
extern GLint APIENTRY __gllc_GetUniformBufferSizeEXT(__GLcontext *gc, GLuint program, GLint location);
extern GLintptr APIENTRY __gllc_GetUniformOffsetEXT(__GLcontext *gc, GLuint program, GLint location);
#endif

#if GL_EXT_texture_integer
extern GLvoid APIENTRY __gllc_ClearColorIiEXT(__GLcontext *gc, GLint r, GLint g, GLint b,GLint a);
extern GLvoid APIENTRY __gllc_ClearColorIuiEXT(__GLcontext *gc, GLuint r, GLuint g, GLuint b, GLuint a);
extern GLvoid APIENTRY __gllc_TexParameterIivEXT(__GLcontext *gc, GLenum target, GLenum pname, GLint *params);
extern GLvoid APIENTRY __gllc_TexParameterIuivEXT(__GLcontext *gc, GLenum target, GLenum pname, GLuint *params);
extern GLvoid APIENTRY __gllc_GetTexParameterIivEXT(__GLcontext *gc, GLenum target, GLenum pname, GLint *params);
extern GLvoid APIENTRY __gllc_GetTexParameterIuivEXT(__GLcontext *gc, GLenum target, GLenum pname, GLuint *params);
#endif

#if GL_EXT_gpu_shader4
extern GLvoid APIENTRY __gllc_VertexAttribI1iEXT(__GLcontext *gc, GLuint index, GLint x);
extern GLvoid APIENTRY __gllc_VertexAttribI2iEXT(__GLcontext *gc, GLuint index, GLint x, GLint y);
extern GLvoid APIENTRY __gllc_VertexAttribI3iEXT(__GLcontext *gc, GLuint index, GLint x, GLint y, GLint z);
extern GLvoid APIENTRY __gllc_VertexAttribI4iEXT(__GLcontext *gc, GLuint index, GLint x, GLint y, GLint z, GLint w);

extern GLvoid APIENTRY __gllc_VertexAttribI1uiEXT(__GLcontext *gc, GLuint index, GLuint x);
extern GLvoid APIENTRY __gllc_VertexAttribI2uiEXT(__GLcontext *gc, GLuint index, GLuint x, GLuint y);
extern GLvoid APIENTRY __gllc_VertexAttribI3uiEXT(__GLcontext *gc, GLuint index, GLuint x, GLuint y, GLuint z);
extern GLvoid APIENTRY __gllc_VertexAttribI4uiEXT(__GLcontext *gc, GLuint index, GLuint x, GLuint y, GLuint z,
                            GLuint w);

extern GLvoid APIENTRY __gllc_VertexAttribI1ivEXT(__GLcontext *gc, GLuint index, const GLint *v);
extern GLvoid APIENTRY __gllc_VertexAttribI2ivEXT(__GLcontext *gc, GLuint index, const GLint *v);
extern GLvoid APIENTRY __gllc_VertexAttribI3ivEXT(__GLcontext *gc, GLuint index, const GLint *v);
extern GLvoid APIENTRY __gllc_VertexAttribI4ivEXT(__GLcontext *gc, GLuint index, const GLint *v);

extern GLvoid APIENTRY __gllc_VertexAttribI1uivEXT(__GLcontext *gc, GLuint index, const GLuint *v);
extern GLvoid APIENTRY __gllc_VertexAttribI2uivEXT(__GLcontext *gc, GLuint index, const GLuint *v);
extern GLvoid APIENTRY __gllc_VertexAttribI3uivEXT(__GLcontext *gc, GLuint index, const GLuint *v);
extern GLvoid APIENTRY __gllc_VertexAttribI4uivEXT(__GLcontext *gc, GLuint index, const GLuint *v);

extern GLvoid APIENTRY __gllc_VertexAttribI4bvEXT(__GLcontext *gc, GLuint index, const GLbyte *v);
extern GLvoid APIENTRY __gllc_VertexAttribI4svEXT(__GLcontext *gc, GLuint index, const GLshort *v);
extern GLvoid APIENTRY __gllc_VertexAttribI4ubvEXT(__GLcontext *gc, GLuint index, const GLubyte *v);
extern GLvoid APIENTRY __gllc_VertexAttribI4usvEXT(__GLcontext *gc, GLuint index, const GLushort *v);

extern GLvoid APIENTRY __gllc_VertexAttribIPointerEXT(__GLcontext *gc, GLuint index, GLint size, GLenum type,
                                GLsizei stride, const GLvoid *pointer);

extern GLvoid APIENTRY __gllc_GetVertexAttribIivEXT(__GLcontext *gc, GLuint index, GLenum pname, GLint *params);
extern GLvoid APIENTRY __gllc_GetVertexAttribIuivEXT(__GLcontext *gc, GLuint index, GLenum pname, GLuint *params);

extern GLvoid APIENTRY __gllc_Uniform1uiEXT(__GLcontext *gc, GLint location, GLuint v0);
extern GLvoid APIENTRY __gllc_Uniform2uiEXT(__GLcontext *gc, GLint location, GLuint v0, GLuint v1);
extern GLvoid APIENTRY __gllc_Uniform3uiEXT(__GLcontext *gc, GLint location, GLuint v0, GLuint v1, GLuint v2);
extern GLvoid APIENTRY __gllc_Uniform4uiEXT(__GLcontext *gc, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);

extern GLvoid APIENTRY __gllc_Uniform1uivEXT(__GLcontext *gc, GLint location, GLsizei count, const GLuint *value);
extern GLvoid APIENTRY __gllc_Uniform2uivEXT(__GLcontext *gc, GLint location, GLsizei count, const GLuint *value);
extern GLvoid APIENTRY __gllc_Uniform3uivEXT(__GLcontext *gc, GLint location, GLsizei count, const GLuint *value);
extern GLvoid APIENTRY __gllc_Uniform4uivEXT(__GLcontext *gc, GLint location, GLsizei count, const GLuint *value);

extern GLvoid APIENTRY __gllc_GetUniformuivEXT(__GLcontext *gc, GLuint program, GLint location, GLuint *params);

extern GLvoid APIENTRY __gllc_BindFragDataLocationEXT(__GLcontext *gc, GLuint program, GLuint colorNumber,
                                const GLbyte *name);
extern GLint APIENTRY __gllc_GetFragDataLocationEXT(__GLcontext *gc, GLuint program, const GLbyte *name);
#endif

#if GL_EXT_geometry_shader4
extern GLvoid APIENTRY __gllc_ProgramParameteriEXT(__GLcontext *gc, GLuint program, GLenum pname, GLint value);
extern GLvoid APIENTRY __gllc_FramebufferTextureEXT(__GLcontext *gc, GLenum target, GLenum attachment, GLuint texture, GLint level);
extern GLvoid APIENTRY __gllc_FramebufferTextureLayerEXT(__GLcontext *gc, GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
extern GLvoid APIENTRY __gllc_FramebufferTextureFaceEXT(__GLcontext *gc, GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face);
#endif

#if GL_EXT_draw_buffers2
extern GLvoid APIENTRY __gllc_ColorMaskIndexedEXT(__GLcontext *gc, GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
extern GLvoid APIENTRY __gllc_GetBooleanIndexedvEXT(__GLcontext *gc, GLenum value, GLuint index, GLboolean *data);
extern GLvoid APIENTRY __gllc_GetIntegerIndexedvEXT(__GLcontext *gc, GLenum value, GLuint index, GLint *data);
extern GLvoid APIENTRY __gllc_EnableIndexedEXT(__GLcontext *gc, GLenum target, GLuint index);
extern GLvoid APIENTRY __gllc_DisableIndexedEXT(__GLcontext *gc, GLenum target, GLuint index);
extern GLboolean APIENTRY __gllc_IsEnabledIndexedEXT(__GLcontext *gc, GLenum target, GLuint index);
#endif

#if GL_EXT_gpu_program_parameters
extern GLvoid APIENTRY __gllc_ProgramEnvParameters4fvEXT(__GLcontext *gc, GLenum target, GLuint index, GLsizei count, const GLfloat *params);
extern GLvoid APIENTRY __gllc_ProgramLocalParameters4fvEXT(__GLcontext *gc, GLenum target, GLuint index, GLsizei count, const GLfloat *params);
#endif

#if GL_ARB_color_buffer_float
extern GLvoid APIENTRY __gllc_ClampColorARB(__GLcontext *gc, GLenum target, GLenum clamp);
#endif

#if GL_ATI_separate_stencil
extern GLvoid APIENTRY __gllc_StencilFuncSeparateATI(__GLcontext *gc, GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask);
#endif

#endif /* __g_lcfncs_h_ */
