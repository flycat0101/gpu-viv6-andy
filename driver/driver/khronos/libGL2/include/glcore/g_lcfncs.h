/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
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

typedef GLvoid (APIENTRY *__T_NewList)( GLuint, GLenum);
typedef GLvoid (APIENTRY *__T_EndList)( GLvoid);
typedef GLvoid (APIENTRY *__T_CallList)( GLuint);
typedef GLvoid (APIENTRY *__T_CallLists)( GLsizei, GLenum, const GLvoid *);
typedef GLvoid (APIENTRY *__T_DeleteLists)( GLuint, GLsizei);
typedef GLuint (APIENTRY *__T_GenLists)( GLsizei);
typedef GLvoid (APIENTRY *__T_ListBase)( GLuint);
typedef GLvoid (APIENTRY *__T_Begin)( GLenum);
typedef GLvoid (APIENTRY *__T_Bitmap)( GLsizei, GLsizei, GLfloat, GLfloat, GLfloat, GLfloat, const GLubyte *);
typedef GLvoid (APIENTRY *__T_Color3b)( GLbyte, GLbyte, GLbyte);
typedef GLvoid (APIENTRY *__T_Color3bv)( const GLbyte *);
typedef GLvoid (APIENTRY *__T_Color3d)( GLdouble, GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_Color3dv)( const GLdouble *);
typedef GLvoid (APIENTRY *__T_Color3f)( GLfloat, GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_Color3fv)( const GLfloat *);
typedef GLvoid (APIENTRY *__T_Color3i)( GLint, GLint, GLint);
typedef GLvoid (APIENTRY *__T_Color3iv)( const GLint *);
typedef GLvoid (APIENTRY *__T_Color3s)( GLshort, GLshort, GLshort);
typedef GLvoid (APIENTRY *__T_Color3sv)( const GLshort *);
typedef GLvoid (APIENTRY *__T_Color3ub)( GLubyte, GLubyte, GLubyte);
typedef GLvoid (APIENTRY *__T_Color3ubv)( const GLubyte *);
typedef GLvoid (APIENTRY *__T_Color3ui)( GLuint, GLuint, GLuint);
typedef GLvoid (APIENTRY *__T_Color3uiv)( const GLuint *);
typedef GLvoid (APIENTRY *__T_Color3us)( GLushort, GLushort, GLushort);
typedef GLvoid (APIENTRY *__T_Color3usv)( const GLushort *);
typedef GLvoid (APIENTRY *__T_Color4b)( GLbyte, GLbyte, GLbyte, GLbyte);
typedef GLvoid (APIENTRY *__T_Color4bv)( const GLbyte *);
typedef GLvoid (APIENTRY *__T_Color4d)( GLdouble, GLdouble, GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_Color4dv)( const GLdouble *);
typedef GLvoid (APIENTRY *__T_Color4f)( GLfloat, GLfloat, GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_Color4fv)( const GLfloat *);
typedef GLvoid (APIENTRY *__T_Color4i)( GLint, GLint, GLint, GLint);
typedef GLvoid (APIENTRY *__T_Color4iv)( const GLint *);
typedef GLvoid (APIENTRY *__T_Color4s)( GLshort, GLshort, GLshort, GLshort);
typedef GLvoid (APIENTRY *__T_Color4sv)( const GLshort *);
typedef GLvoid (APIENTRY *__T_Color4ub)( GLubyte, GLubyte, GLubyte, GLubyte);
typedef GLvoid (APIENTRY *__T_Color4ubv)( const GLubyte *);
typedef GLvoid (APIENTRY *__T_Color4ui)( GLuint, GLuint, GLuint, GLuint);
typedef GLvoid (APIENTRY *__T_Color4uiv)( const GLuint *);
typedef GLvoid (APIENTRY *__T_Color4us)( GLushort, GLushort, GLushort, GLushort);
typedef GLvoid (APIENTRY *__T_Color4usv)( const GLushort *);
typedef GLvoid (APIENTRY *__T_EdgeFlag)( GLboolean);
typedef GLvoid (APIENTRY *__T_EdgeFlagv)( const GLboolean *);
typedef GLvoid (APIENTRY *__T_End)( GLvoid);
typedef GLvoid (APIENTRY *__T_Indexd)( GLdouble);
typedef GLvoid (APIENTRY *__T_Indexdv)( const GLdouble *);
typedef GLvoid (APIENTRY *__T_Indexf)( GLfloat);
typedef GLvoid (APIENTRY *__T_Indexfv)( const GLfloat *);
typedef GLvoid (APIENTRY *__T_Indexi)( GLint);
typedef GLvoid (APIENTRY *__T_Indexiv)( const GLint *);
typedef GLvoid (APIENTRY *__T_Indexs)( GLshort);
typedef GLvoid (APIENTRY *__T_Indexsv)( const GLshort *);
typedef GLvoid (APIENTRY *__T_Normal3b)( GLbyte, GLbyte, GLbyte);
typedef GLvoid (APIENTRY *__T_Normal3bv)( const GLbyte *);
typedef GLvoid (APIENTRY *__T_Normal3d)( GLdouble, GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_Normal3dv)( const GLdouble *);
typedef GLvoid (APIENTRY *__T_Normal3f)( GLfloat, GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_Normal3fv)( const GLfloat *);
typedef GLvoid (APIENTRY *__T_Normal3i)( GLint, GLint, GLint);
typedef GLvoid (APIENTRY *__T_Normal3iv)( const GLint *);
typedef GLvoid (APIENTRY *__T_Normal3s)( GLshort, GLshort, GLshort);
typedef GLvoid (APIENTRY *__T_Normal3sv)( const GLshort *);
typedef GLvoid (APIENTRY *__T_RasterPos2d)( GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_RasterPos2dv)( const GLdouble *);
typedef GLvoid (APIENTRY *__T_RasterPos2f)( GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_RasterPos2fv)( const GLfloat *);
typedef GLvoid (APIENTRY *__T_RasterPos2i)( GLint, GLint);
typedef GLvoid (APIENTRY *__T_RasterPos2iv)( const GLint *);
typedef GLvoid (APIENTRY *__T_RasterPos2s)( GLshort, GLshort);
typedef GLvoid (APIENTRY *__T_RasterPos2sv)( const GLshort *);
typedef GLvoid (APIENTRY *__T_RasterPos3d)( GLdouble, GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_RasterPos3dv)( const GLdouble *);
typedef GLvoid (APIENTRY *__T_RasterPos3f)( GLfloat, GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_RasterPos3fv)( const GLfloat *);
typedef GLvoid (APIENTRY *__T_RasterPos3i)( GLint, GLint, GLint);
typedef GLvoid (APIENTRY *__T_RasterPos3iv)( const GLint *);
typedef GLvoid (APIENTRY *__T_RasterPos3s)( GLshort, GLshort, GLshort);
typedef GLvoid (APIENTRY *__T_RasterPos3sv)( const GLshort *);
typedef GLvoid (APIENTRY *__T_RasterPos4d)( GLdouble, GLdouble, GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_RasterPos4dv)( const GLdouble *);
typedef GLvoid (APIENTRY *__T_RasterPos4f)( GLfloat, GLfloat, GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_RasterPos4fv)( const GLfloat *);
typedef GLvoid (APIENTRY *__T_RasterPos4i)( GLint, GLint, GLint, GLint);
typedef GLvoid (APIENTRY *__T_RasterPos4iv)( const GLint *);
typedef GLvoid (APIENTRY *__T_RasterPos4s)( GLshort, GLshort, GLshort, GLshort);
typedef GLvoid (APIENTRY *__T_RasterPos4sv)( const GLshort *);
typedef GLvoid (APIENTRY *__T_Rectd)( GLdouble, GLdouble, GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_Rectdv)( const GLdouble *, const GLdouble *);
typedef GLvoid (APIENTRY *__T_Rectf)( GLfloat, GLfloat, GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_Rectfv)( const GLfloat *, const GLfloat *);
typedef GLvoid (APIENTRY *__T_Recti)( GLint, GLint, GLint, GLint);
typedef GLvoid (APIENTRY *__T_Rectiv)( const GLint *, const GLint *);
typedef GLvoid (APIENTRY *__T_Rects)( GLshort, GLshort, GLshort, GLshort);
typedef GLvoid (APIENTRY *__T_Rectsv)( const GLshort *, const GLshort *);
typedef GLvoid (APIENTRY *__T_TexCoord1d)( GLdouble);
typedef GLvoid (APIENTRY *__T_TexCoord1dv)( const GLdouble *);
typedef GLvoid (APIENTRY *__T_TexCoord1f)( GLfloat);
typedef GLvoid (APIENTRY *__T_TexCoord1fv)( const GLfloat *);
typedef GLvoid (APIENTRY *__T_TexCoord1i)( GLint);
typedef GLvoid (APIENTRY *__T_TexCoord1iv)( const GLint *);
typedef GLvoid (APIENTRY *__T_TexCoord1s)( GLshort);
typedef GLvoid (APIENTRY *__T_TexCoord1sv)( const GLshort *);
typedef GLvoid (APIENTRY *__T_TexCoord2d)( GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_TexCoord2dv)( const GLdouble *);
typedef GLvoid (APIENTRY *__T_TexCoord2f)( GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_TexCoord2fv)( const GLfloat *);
typedef GLvoid (APIENTRY *__T_TexCoord2i)( GLint, GLint);
typedef GLvoid (APIENTRY *__T_TexCoord2iv)( const GLint *);
typedef GLvoid (APIENTRY *__T_TexCoord2s)( GLshort, GLshort);
typedef GLvoid (APIENTRY *__T_TexCoord2sv)( const GLshort *);
typedef GLvoid (APIENTRY *__T_TexCoord3d)( GLdouble, GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_TexCoord3dv)( const GLdouble *);
typedef GLvoid (APIENTRY *__T_TexCoord3f)( GLfloat, GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_TexCoord3fv)( const GLfloat *);
typedef GLvoid (APIENTRY *__T_TexCoord3i)( GLint, GLint, GLint);
typedef GLvoid (APIENTRY *__T_TexCoord3iv)( const GLint *);
typedef GLvoid (APIENTRY *__T_TexCoord3s)( GLshort, GLshort, GLshort);
typedef GLvoid (APIENTRY *__T_TexCoord3sv)( const GLshort *);
typedef GLvoid (APIENTRY *__T_TexCoord4d)( GLdouble, GLdouble, GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_TexCoord4dv)( const GLdouble *);
typedef GLvoid (APIENTRY *__T_TexCoord4f)( GLfloat, GLfloat, GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_TexCoord4fv)( const GLfloat *);
typedef GLvoid (APIENTRY *__T_TexCoord4i)( GLint, GLint, GLint, GLint);
typedef GLvoid (APIENTRY *__T_TexCoord4iv)( const GLint *);
typedef GLvoid (APIENTRY *__T_TexCoord4s)( GLshort, GLshort, GLshort, GLshort);
typedef GLvoid (APIENTRY *__T_TexCoord4sv)( const GLshort *);
typedef GLvoid (APIENTRY *__T_Vertex2d)( GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_Vertex2dv)( const GLdouble *);
typedef GLvoid (APIENTRY *__T_Vertex2f)( GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_Vertex2fv)( const GLfloat *);
typedef GLvoid (APIENTRY *__T_Vertex2i)( GLint, GLint);
typedef GLvoid (APIENTRY *__T_Vertex2iv)( const GLint *);
typedef GLvoid (APIENTRY *__T_Vertex2s)( GLshort, GLshort);
typedef GLvoid (APIENTRY *__T_Vertex2sv)( const GLshort *);
typedef GLvoid (APIENTRY *__T_Vertex3d)( GLdouble, GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_Vertex3dv)( const GLdouble *);
typedef GLvoid (APIENTRY *__T_Vertex3f)( GLfloat, GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_Vertex3fv)( const GLfloat *);
typedef GLvoid (APIENTRY *__T_Vertex3i)( GLint, GLint, GLint);
typedef GLvoid (APIENTRY *__T_Vertex3iv)( const GLint *);
typedef GLvoid (APIENTRY *__T_Vertex3s)( GLshort, GLshort, GLshort);
typedef GLvoid (APIENTRY *__T_Vertex3sv)( const GLshort *);
typedef GLvoid (APIENTRY *__T_Vertex4d)( GLdouble, GLdouble, GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_Vertex4dv)( const GLdouble *);
typedef GLvoid (APIENTRY *__T_Vertex4f)( GLfloat, GLfloat, GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_Vertex4fv)( const GLfloat *);
typedef GLvoid (APIENTRY *__T_Vertex4i)( GLint, GLint, GLint, GLint);
typedef GLvoid (APIENTRY *__T_Vertex4iv)( const GLint *);
typedef GLvoid (APIENTRY *__T_Vertex4s)( GLshort, GLshort, GLshort, GLshort);
typedef GLvoid (APIENTRY *__T_Vertex4sv)( const GLshort *);
typedef GLvoid (APIENTRY *__T_DrawElements)( GLenum, GLsizei, GLenum, const GLvoid *);
typedef GLvoid (APIENTRY *__T_ArrayElement)( GLint);
typedef GLvoid (APIENTRY *__T_DrawArrays)( GLenum, GLint, GLsizei);
typedef GLvoid (APIENTRY *__T_ClipPlane)( GLenum, const GLdouble *);
typedef GLvoid (APIENTRY *__T_ColorMaterial)( GLenum, GLenum);
typedef GLvoid (APIENTRY *__T_CullFace)( GLenum);
typedef GLvoid (APIENTRY *__T_Fogf)( GLenum, GLfloat);
typedef GLvoid (APIENTRY *__T_Fogfv)( GLenum, const GLfloat *);
typedef GLvoid (APIENTRY *__T_Fogi)( GLenum, GLint);
typedef GLvoid (APIENTRY *__T_Fogiv)( GLenum, const GLint *);
typedef GLvoid (APIENTRY *__T_FrontFace)( GLenum);
typedef GLvoid (APIENTRY *__T_Hint)( GLenum, GLenum);
typedef GLvoid (APIENTRY *__T_Lightf)( GLenum, GLenum, GLfloat);
typedef GLvoid (APIENTRY *__T_Lightfv)( GLenum, GLenum, const GLfloat *);
typedef GLvoid (APIENTRY *__T_Lighti)( GLenum, GLenum, GLint);
typedef GLvoid (APIENTRY *__T_Lightiv)( GLenum, GLenum, const GLint *);
typedef GLvoid (APIENTRY *__T_LightModelf)( GLenum, GLfloat);
typedef GLvoid (APIENTRY *__T_LightModelfv)( GLenum, const GLfloat *);
typedef GLvoid (APIENTRY *__T_LightModeli)( GLenum, GLint);
typedef GLvoid (APIENTRY *__T_LightModeliv)( GLenum, const GLint *);
typedef GLvoid (APIENTRY *__T_LineStipple)( GLint, GLushort);
typedef GLvoid (APIENTRY *__T_LineWidth)( GLfloat);
typedef GLvoid (APIENTRY *__T_Materialf)( GLenum, GLenum, GLfloat);
typedef GLvoid (APIENTRY *__T_Materialfv)( GLenum, GLenum, const GLfloat *);
typedef GLvoid (APIENTRY *__T_Materiali)( GLenum, GLenum, GLint);
typedef GLvoid (APIENTRY *__T_Materialiv)( GLenum, GLenum, const GLint *);
typedef GLvoid (APIENTRY *__T_PointSize)( GLfloat);
typedef GLvoid (APIENTRY *__T_PolygonMode)( GLenum, GLenum);
typedef GLvoid (APIENTRY *__T_PolygonStipple)( const GLubyte *);
typedef GLvoid (APIENTRY *__T_Scissor)( GLint, GLint, GLsizei, GLsizei);
typedef GLvoid (APIENTRY *__T_ShadeModel)( GLenum);
typedef GLvoid (APIENTRY *__T_TexParameterf)( GLenum, GLenum, GLfloat);
typedef GLvoid (APIENTRY *__T_TexParameterfv)( GLenum, GLenum, const GLfloat *);
typedef GLvoid (APIENTRY *__T_TexParameteri)( GLenum, GLenum, GLint);
typedef GLvoid (APIENTRY *__T_TexParameteriv)( GLenum, GLenum, const GLint *);
typedef GLvoid (APIENTRY *__T_TexImage1D)( GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
typedef GLvoid (APIENTRY *__T_TexImage2D)( GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
typedef GLvoid (APIENTRY *__T_TexEnvf)( GLenum, GLenum, GLfloat);
typedef GLvoid (APIENTRY *__T_TexEnvfv)( GLenum, GLenum, const GLfloat *);
typedef GLvoid (APIENTRY *__T_TexEnvi)( GLenum, GLenum, GLint);
typedef GLvoid (APIENTRY *__T_TexEnviv)( GLenum, GLenum, const GLint *);
typedef GLvoid (APIENTRY *__T_TexGend)( GLenum, GLenum, GLdouble);
typedef GLvoid (APIENTRY *__T_TexGendv)( GLenum, GLenum, const GLdouble *);
typedef GLvoid (APIENTRY *__T_TexGenf)( GLenum, GLenum, GLfloat);
typedef GLvoid (APIENTRY *__T_TexGenfv)( GLenum, GLenum, const GLfloat *);
typedef GLvoid (APIENTRY *__T_TexGeni)( GLenum, GLenum, GLint);
typedef GLvoid (APIENTRY *__T_TexGeniv)( GLenum, GLenum, const GLint *);
typedef GLvoid (APIENTRY *__T_FeedbackBuffer)( GLsizei, GLenum, GLfloat *);
typedef GLvoid (APIENTRY *__T_SelectBuffer)( GLsizei, GLuint *);
typedef GLint (APIENTRY *__T_RenderMode)( GLenum);
typedef GLvoid (APIENTRY *__T_InitNames)( GLvoid);
typedef GLvoid (APIENTRY *__T_LoadName)( GLuint);
typedef GLvoid (APIENTRY *__T_PassThrough)( GLfloat);
typedef GLvoid (APIENTRY *__T_PopName)( GLvoid);
typedef GLvoid (APIENTRY *__T_PushName)( GLuint);
typedef GLvoid (APIENTRY *__T_DrawBuffer)( GLenum);
typedef GLvoid (APIENTRY *__T_Clear)( GLbitfield);
typedef GLvoid (APIENTRY *__T_ClearAccum)( GLfloat, GLfloat, GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_ClearIndex)( GLfloat);
typedef GLvoid (APIENTRY *__T_ClearColor)( GLclampf, GLclampf, GLclampf, GLclampf);
typedef GLvoid (APIENTRY *__T_ClearStencil)( GLint);
typedef GLvoid (APIENTRY *__T_ClearDepth)( GLclampd);
typedef GLvoid (APIENTRY *__T_StencilMask)( GLuint);
typedef GLvoid (APIENTRY *__T_ColorMask)( GLboolean, GLboolean, GLboolean, GLboolean);
typedef GLvoid (APIENTRY *__T_DepthMask)( GLboolean);
typedef GLvoid (APIENTRY *__T_IndexMask)( GLuint);
typedef GLvoid (APIENTRY *__T_Accum)( GLenum, GLfloat);
typedef GLvoid (APIENTRY *__T_Disable)( GLenum);
typedef GLvoid (APIENTRY *__T_Enable)( GLenum);
typedef GLvoid (APIENTRY *__T_Finish)( GLvoid);
typedef GLvoid (APIENTRY *__T_Flush)( GLvoid);
typedef GLvoid (APIENTRY *__T_PopAttrib)( GLvoid);
typedef GLvoid (APIENTRY *__T_PushAttrib)( GLbitfield);
typedef GLvoid (APIENTRY *__T_Map1d)( GLenum, GLdouble, GLdouble, GLint, GLint, const GLdouble *);
typedef GLvoid (APIENTRY *__T_Map1f)( GLenum, GLfloat, GLfloat, GLint, GLint, const GLfloat *);
typedef GLvoid (APIENTRY *__T_Map2d)( GLenum, GLdouble, GLdouble, GLint, GLint, GLdouble, GLdouble, GLint, GLint, const GLdouble *);
typedef GLvoid (APIENTRY *__T_Map2f)( GLenum, GLfloat, GLfloat, GLint, GLint, GLfloat, GLfloat, GLint, GLint, const GLfloat *);
typedef GLvoid (APIENTRY *__T_MapGrid1d)( GLint, GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_MapGrid1f)( GLint, GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_MapGrid2d)( GLint, GLdouble, GLdouble, GLint, GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_MapGrid2f)( GLint, GLfloat, GLfloat, GLint, GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_EvalCoord1d)( GLdouble);
typedef GLvoid (APIENTRY *__T_EvalCoord1dv)( const GLdouble *);
typedef GLvoid (APIENTRY *__T_EvalCoord1f)( GLfloat);
typedef GLvoid (APIENTRY *__T_EvalCoord1fv)( const GLfloat *);
typedef GLvoid (APIENTRY *__T_EvalCoord2d)( GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_EvalCoord2dv)( const GLdouble *);
typedef GLvoid (APIENTRY *__T_EvalCoord2f)( GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_EvalCoord2fv)( const GLfloat *);
typedef GLvoid (APIENTRY *__T_EvalMesh1)( GLenum, GLint, GLint);
typedef GLvoid (APIENTRY *__T_EvalPoint1)( GLint);
typedef GLvoid (APIENTRY *__T_EvalMesh2)( GLenum, GLint, GLint, GLint, GLint);
typedef GLvoid (APIENTRY *__T_EvalPoint2)( GLint, GLint);
typedef GLvoid (APIENTRY *__T_AlphaFunc)( GLenum, GLclampf);
typedef GLvoid (APIENTRY *__T_BlendFunc)( GLenum, GLenum);
typedef GLvoid (APIENTRY *__T_LogicOp)( GLenum);
typedef GLvoid (APIENTRY *__T_StencilFunc)( GLenum, GLint, GLuint);
typedef GLvoid (APIENTRY *__T_StencilOp)( GLenum, GLenum, GLenum);
typedef GLvoid (APIENTRY *__T_DepthFunc)( GLenum);
typedef GLvoid (APIENTRY *__T_PixelZoom)( GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_PixelTransferf)( GLenum, GLfloat);
typedef GLvoid (APIENTRY *__T_PixelTransferi)( GLenum, GLint);
typedef GLvoid (APIENTRY *__T_PixelStoref)( GLenum, GLfloat);
typedef GLvoid (APIENTRY *__T_PixelStorei)( GLenum, GLint);
typedef GLvoid (APIENTRY *__T_PixelMapfv)( GLenum, GLint, const GLfloat *);
typedef GLvoid (APIENTRY *__T_PixelMapuiv)( GLenum, GLint, const GLuint *);
typedef GLvoid (APIENTRY *__T_PixelMapusv)( GLenum, GLint, const GLushort *);
typedef GLvoid (APIENTRY *__T_ReadBuffer)( GLenum);
typedef GLvoid (APIENTRY *__T_CopyPixels)( GLint, GLint, GLsizei, GLsizei, GLenum);
typedef GLvoid (APIENTRY *__T_ReadPixels)( GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid *);
typedef GLvoid (APIENTRY *__T_DrawPixels)( GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
typedef GLvoid (APIENTRY *__T_GetBooleanv)( GLenum, GLboolean *);
typedef GLvoid (APIENTRY *__T_GetClipPlane)( GLenum, GLdouble *);
typedef GLvoid (APIENTRY *__T_GetDoublev)( GLenum, GLdouble *);
typedef GLenum (APIENTRY *__T_GetError)( GLvoid);
typedef GLvoid (APIENTRY *__T_GetFloatv)( GLenum, GLfloat *);
typedef GLvoid (APIENTRY *__T_GetIntegerv)( GLenum, GLint *);
typedef GLvoid (APIENTRY *__T_GetLightfv)( GLenum, GLenum, GLfloat *);
typedef GLvoid (APIENTRY *__T_GetLightiv)( GLenum, GLenum, GLint *);
typedef GLvoid (APIENTRY *__T_GetMapdv)( GLenum, GLenum, GLdouble *);
typedef GLvoid (APIENTRY *__T_GetMapfv)( GLenum, GLenum, GLfloat *);
typedef GLvoid (APIENTRY *__T_GetMapiv)( GLenum, GLenum, GLint *);
typedef GLvoid (APIENTRY *__T_GetMaterialfv)( GLenum, GLenum, GLfloat *);
typedef GLvoid (APIENTRY *__T_GetMaterialiv)( GLenum, GLenum, GLint *);
typedef GLvoid (APIENTRY *__T_GetPixelMapfv)( GLenum, GLfloat *);
typedef GLvoid (APIENTRY *__T_GetPixelMapuiv)( GLenum, GLuint *);
typedef GLvoid (APIENTRY *__T_GetPixelMapusv)( GLenum, GLushort *);
typedef GLvoid (APIENTRY *__T_GetPolygonStipple)( GLubyte *);
typedef const GLubyte * (APIENTRY *__T_GetString)( GLenum);
typedef GLvoid (APIENTRY *__T_GetTexEnvfv)( GLenum, GLenum, GLfloat *);
typedef GLvoid (APIENTRY *__T_GetTexEnviv)( GLenum, GLenum, GLint *);
typedef GLvoid (APIENTRY *__T_GetTexGendv)( GLenum, GLenum, GLdouble *);
typedef GLvoid (APIENTRY *__T_GetTexGenfv)( GLenum, GLenum, GLfloat *);
typedef GLvoid (APIENTRY *__T_GetTexGeniv)( GLenum, GLenum, GLint *);
typedef GLvoid (APIENTRY *__T_GetTexImage)( GLenum, GLint, GLenum, GLenum, GLvoid *);
typedef GLvoid (APIENTRY *__T_GetTexParameterfv)( GLenum, GLenum, GLfloat *);
typedef GLvoid (APIENTRY *__T_GetTexParameteriv)( GLenum, GLenum, GLint *);
typedef GLvoid (APIENTRY *__T_GetTexLevelParameterfv)( GLenum, GLint, GLenum, GLfloat *);
typedef GLvoid (APIENTRY *__T_GetTexLevelParameteriv)( GLenum, GLint, GLenum, GLint *);
typedef GLboolean (APIENTRY *__T_IsEnabled)( GLenum);
typedef GLboolean (APIENTRY *__T_IsList)( GLuint);
typedef GLvoid (APIENTRY *__T_DepthRange)( GLclampd, GLclampd);
typedef GLvoid (APIENTRY *__T_Frustum)( GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_LoadIdentity)( GLvoid);
typedef GLvoid (APIENTRY *__T_LoadMatrixf)( const GLfloat *);
typedef GLvoid (APIENTRY *__T_LoadMatrixd)( const GLdouble *);
typedef GLvoid (APIENTRY *__T_MatrixMode)( GLenum);
typedef GLvoid (APIENTRY *__T_MultMatrixf)( const GLfloat *);
typedef GLvoid (APIENTRY *__T_MultMatrixd)( const GLdouble *);
typedef GLvoid (APIENTRY *__T_Ortho)( GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_PopMatrix)( GLvoid);
typedef GLvoid (APIENTRY *__T_PushMatrix)( GLvoid);
typedef GLvoid (APIENTRY *__T_Rotated)( GLdouble, GLdouble, GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_Rotatef)( GLfloat, GLfloat, GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_Scaled)( GLdouble, GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_Scalef)( GLfloat, GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_Translated)( GLdouble, GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_Translatef)( GLfloat, GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_Viewport)( GLint, GLint, GLsizei, GLsizei);
typedef GLvoid (APIENTRY *__T_ColorSubTable)( GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
typedef GLvoid (APIENTRY *__T_ColorTable)( GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *);
typedef GLvoid (APIENTRY *__T_CopyColorTable)( GLenum, GLenum, GLint, GLint, GLsizei);
typedef GLvoid (APIENTRY *__T_GetColorTable)( GLenum, GLenum, GLenum, GLvoid *);
typedef GLvoid (APIENTRY *__T_GetColorTableParameterfv)( GLenum, GLenum, GLfloat *);
typedef GLvoid (APIENTRY *__T_GetColorTableParameteriv)( GLenum, GLenum, GLint *);
typedef GLvoid (APIENTRY *__T_ColorPointer)( GLint, GLenum, GLsizei, const GLvoid *);
typedef GLvoid (APIENTRY *__T_DisableClientState)( GLenum);
typedef GLvoid (APIENTRY *__T_EdgeFlagPointer)( GLsizei, const GLboolean *);
typedef GLvoid (APIENTRY *__T_EnableClientState)( GLenum);
typedef GLvoid (APIENTRY *__T_GetPointerv)( GLenum, GLvoid* *);
typedef GLvoid (APIENTRY *__T_IndexPointer)( GLenum, GLsizei, const GLvoid *);
typedef GLvoid (APIENTRY *__T_InterleavedArrays)( GLenum, GLsizei, const GLvoid *);
typedef GLvoid (APIENTRY *__T_NormalPointer)( GLenum, GLsizei, const GLvoid *);
typedef GLvoid (APIENTRY *__T_TexCoordPointer)( GLint, GLenum, GLsizei, const GLvoid *);
typedef GLvoid (APIENTRY *__T_VertexPointer)( GLint, GLenum, GLsizei, const GLvoid *);
typedef GLvoid (APIENTRY *__T_PolygonOffset)( GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_CopyTexImage1D)( GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLint);
typedef GLvoid (APIENTRY *__T_CopyTexImage2D)( GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint);
typedef GLvoid (APIENTRY *__T_CopyTexSubImage1D)( GLenum, GLint, GLint, GLint, GLint, GLsizei);
typedef GLvoid (APIENTRY *__T_CopyTexSubImage2D)( GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
typedef GLvoid (APIENTRY *__T_TexSubImage1D)( GLenum, GLint, GLint, GLsizei, GLenum, GLenum, const GLvoid *);
typedef GLvoid (APIENTRY *__T_TexSubImage2D)( GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
typedef GLboolean (APIENTRY *__T_AreTexturesResident)( GLsizei, const GLuint *, GLboolean *);
typedef GLvoid (APIENTRY *__T_BindTexture)( GLenum, GLuint);
typedef GLvoid (APIENTRY *__T_DeleteTextures)( GLsizei, const GLuint *);
typedef GLvoid (APIENTRY *__T_GenTextures)( GLsizei, GLuint *);
typedef GLboolean (APIENTRY *__T_IsTexture)( GLuint);
typedef GLvoid (APIENTRY *__T_PrioritizeTextures)( GLsizei, const GLuint *, const GLclampf *);
typedef GLvoid (APIENTRY *__T_Indexub)( GLubyte);
typedef GLvoid (APIENTRY *__T_Indexubv)( const GLubyte *);
typedef GLvoid (APIENTRY *__T_PopClientAttrib)( GLvoid);
typedef GLvoid (APIENTRY *__T_PushClientAttrib)( GLbitfield);

#if GL_VERSION_1_2
typedef GLvoid (APIENTRY *__T_BlendColor)( GLclampf, GLclampf, GLclampf, GLclampf);
typedef GLvoid (APIENTRY *__T_BlendEquation)( GLenum);
typedef GLvoid (APIENTRY *__T_DrawRangeElements)( GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid *);
typedef GLvoid (APIENTRY *__T_ColorTable)( GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *);
typedef GLvoid (APIENTRY *__T_ColorTableParameterfv)( GLenum, GLenum, const GLfloat *);
typedef GLvoid (APIENTRY *__T_ColorTableParameteriv)( GLenum, GLenum, const GLint *);
typedef GLvoid (APIENTRY *__T_CopyColorTable)( GLenum, GLenum, GLint, GLint, GLsizei);
typedef GLvoid (APIENTRY *__T_GetColorTable)( GLenum, GLenum, GLenum, GLvoid *);
typedef GLvoid (APIENTRY *__T_GetColorTableParameterfv)( GLenum, GLenum, GLfloat *);
typedef GLvoid (APIENTRY *__T_GetColorTableParameteriv)( GLenum, GLenum, GLint *);
typedef GLvoid (APIENTRY *__T_ColorSubTable)( GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
typedef GLvoid (APIENTRY *__T_CopyColorSubTable)( GLenum, GLsizei, GLint, GLint, GLsizei);
typedef GLvoid (APIENTRY *__T_ConvolutionFilter1D)( GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *);
typedef GLvoid (APIENTRY *__T_ConvolutionFilter2D)( GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
typedef GLvoid (APIENTRY *__T_ConvolutionParameterf)( GLenum, GLenum, GLfloat);
typedef GLvoid (APIENTRY *__T_ConvolutionParameterfv)( GLenum, GLenum, const GLfloat *);
typedef GLvoid (APIENTRY *__T_ConvolutionParameteri)( GLenum, GLenum, GLint);
typedef GLvoid (APIENTRY *__T_ConvolutionParameteriv)( GLenum, GLenum, const GLint *);
typedef GLvoid (APIENTRY *__T_CopyConvolutionFilter1D)( GLenum, GLenum, GLint, GLint, GLsizei);
typedef GLvoid (APIENTRY *__T_CopyConvolutionFilter2D)( GLenum, GLenum, GLint, GLint, GLsizei, GLsizei);
typedef GLvoid (APIENTRY *__T_GetConvolutionFilter)( GLenum, GLenum, GLenum, GLvoid *);
typedef GLvoid (APIENTRY *__T_GetConvolutionParameterfv)( GLenum, GLenum, GLfloat *);
typedef GLvoid (APIENTRY *__T_GetConvolutionParameteriv)( GLenum, GLenum, GLint *);
typedef GLvoid (APIENTRY *__T_GetSeparableFilter)( GLenum, GLenum, GLenum, GLvoid *, GLvoid *, GLvoid *);
typedef GLvoid (APIENTRY *__T_SeparableFilter2D)( GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *, const GLvoid *);
typedef GLvoid (APIENTRY *__T_GetHistogram)( GLenum, GLboolean, GLenum, GLenum, GLvoid *);
typedef GLvoid (APIENTRY *__T_GetHistogramParameterfv)( GLenum, GLenum, GLfloat *);
typedef GLvoid (APIENTRY *__T_GetHistogramParameteriv)( GLenum, GLenum, GLint *);
typedef GLvoid (APIENTRY *__T_GetMinmax)( GLenum, GLboolean, GLenum, GLenum, GLvoid *);
typedef GLvoid (APIENTRY *__T_GetMinmaxParameterfv)( GLenum, GLenum, GLfloat *);
typedef GLvoid (APIENTRY *__T_GetMinmaxParameteriv)( GLenum, GLenum, GLint *);
typedef GLvoid (APIENTRY *__T_Histogram)( GLenum, GLsizei, GLenum, GLboolean);
typedef GLvoid (APIENTRY *__T_Minmax)( GLenum, GLenum, GLboolean);
typedef GLvoid (APIENTRY *__T_ResetHistogram)( GLenum);
typedef GLvoid (APIENTRY *__T_ResetMinmax)( GLenum);
typedef GLvoid (APIENTRY *__T_TexImage3D)( GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
typedef GLvoid (APIENTRY *__T_TexSubImage3D)( GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
typedef GLvoid (APIENTRY *__T_CopyTexSubImage3D)( GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
#endif
#if GL_VERSION_1_3
typedef GLvoid (APIENTRY *__T_ActiveTexture)( GLenum);
typedef GLvoid (APIENTRY *__T_ClientActiveTexture)( GLenum);
typedef GLvoid (APIENTRY *__T_MultiTexCoord1d)( GLenum, GLdouble);
typedef GLvoid (APIENTRY *__T_MultiTexCoord1dv)( GLenum, const GLdouble *);
typedef GLvoid (APIENTRY *__T_MultiTexCoord1f)( GLenum, GLfloat);
typedef GLvoid (APIENTRY *__T_MultiTexCoord1fv)( GLenum, const GLfloat *);
typedef GLvoid (APIENTRY *__T_MultiTexCoord1i)( GLenum, GLint);
typedef GLvoid (APIENTRY *__T_MultiTexCoord1iv)( GLenum, const GLint *);
typedef GLvoid (APIENTRY *__T_MultiTexCoord1s)( GLenum, GLshort);
typedef GLvoid (APIENTRY *__T_MultiTexCoord1sv)( GLenum, const GLshort *);
typedef GLvoid (APIENTRY *__T_MultiTexCoord2d)( GLenum, GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_MultiTexCoord2dv)( GLenum, const GLdouble *);
typedef GLvoid (APIENTRY *__T_MultiTexCoord2f)( GLenum, GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_MultiTexCoord2fv)( GLenum, const GLfloat *);
typedef GLvoid (APIENTRY *__T_MultiTexCoord2i)( GLenum, GLint, GLint);
typedef GLvoid (APIENTRY *__T_MultiTexCoord2iv)( GLenum, const GLint *);
typedef GLvoid (APIENTRY *__T_MultiTexCoord2s)( GLenum, GLshort, GLshort);
typedef GLvoid (APIENTRY *__T_MultiTexCoord2sv)( GLenum, const GLshort *);
typedef GLvoid (APIENTRY *__T_MultiTexCoord3d)( GLenum, GLdouble, GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_MultiTexCoord3dv)( GLenum, const GLdouble *);
typedef GLvoid (APIENTRY *__T_MultiTexCoord3f)( GLenum, GLfloat, GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_MultiTexCoord3fv)( GLenum, const GLfloat *);
typedef GLvoid (APIENTRY *__T_MultiTexCoord3i)( GLenum, GLint, GLint, GLint);
typedef GLvoid (APIENTRY *__T_MultiTexCoord3iv)( GLenum, const GLint *);
typedef GLvoid (APIENTRY *__T_MultiTexCoord3s)( GLenum, GLshort, GLshort, GLshort);
typedef GLvoid (APIENTRY *__T_MultiTexCoord3sv)( GLenum, const GLshort *);
typedef GLvoid (APIENTRY *__T_MultiTexCoord4d)( GLenum, GLdouble, GLdouble, GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_MultiTexCoord4dv)( GLenum, const GLdouble *);
typedef GLvoid (APIENTRY *__T_MultiTexCoord4f)( GLenum, GLfloat, GLfloat, GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_MultiTexCoord4fv)( GLenum, const GLfloat *);
typedef GLvoid (APIENTRY *__T_MultiTexCoord4i)( GLenum, GLint, GLint, GLint, GLint);
typedef GLvoid (APIENTRY *__T_MultiTexCoord4iv)( GLenum, const GLint *);
typedef GLvoid (APIENTRY *__T_MultiTexCoord4s)( GLenum, GLshort, GLshort, GLshort, GLshort);
typedef GLvoid (APIENTRY *__T_MultiTexCoord4sv)( GLenum, const GLshort *);
typedef GLvoid (APIENTRY *__T_LoadTransposeMatrixf)( const GLfloat *);
typedef GLvoid (APIENTRY *__T_LoadTransposeMatrixd)( const GLdouble *);
typedef GLvoid (APIENTRY *__T_MultTransposeMatrixf)( const GLfloat *);
typedef GLvoid (APIENTRY *__T_MultTransposeMatrixd)( const GLdouble *);
typedef GLvoid (APIENTRY *__T_SampleCoverage)( GLclampf, GLboolean);
typedef GLvoid (APIENTRY *__T_CompressedTexImage3D)( GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
typedef GLvoid (APIENTRY *__T_CompressedTexImage2D)( GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
typedef GLvoid (APIENTRY *__T_CompressedTexImage1D)( GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const GLvoid *);
typedef GLvoid (APIENTRY *__T_CompressedTexSubImage3D)( GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *);
typedef GLvoid (APIENTRY *__T_CompressedTexSubImage2D)( GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *);
typedef GLvoid (APIENTRY *__T_CompressedTexSubImage1D)( GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const GLvoid *);
typedef GLvoid (APIENTRY *__T_GetCompressedTexImage)( GLenum, GLint, GLvoid *);
#endif
#if GL_VERSION_1_4
typedef GLvoid (APIENTRY *__T_BlendFuncSeparate)( GLenum, GLenum, GLenum, GLenum);
typedef GLvoid (APIENTRY *__T_FogCoordf)( GLfloat);
typedef GLvoid (APIENTRY *__T_FogCoordfv)( const GLfloat *);
typedef GLvoid (APIENTRY *__T_FogCoordd)( GLdouble);
typedef GLvoid (APIENTRY *__T_FogCoorddv)( const GLdouble *);
typedef GLvoid (APIENTRY *__T_FogCoordPointer)( GLenum, GLsizei, const GLvoid *);
typedef GLvoid (APIENTRY *__T_MultiDrawArrays)( GLenum, GLint *, GLsizei *, GLsizei);
typedef GLvoid (APIENTRY *__T_MultiDrawElements)( GLenum, const GLsizei *, GLenum, const GLvoid* *, GLsizei);
typedef GLvoid (APIENTRY *__T_PointParameterf)( GLenum, GLfloat);
typedef GLvoid (APIENTRY *__T_PointParameterfv)( GLenum, const GLfloat *);
typedef GLvoid (APIENTRY *__T_PointParameteri)( GLenum, GLint);
typedef GLvoid (APIENTRY *__T_PointParameteriv)( GLenum, const GLint *);
typedef GLvoid (APIENTRY *__T_SecondaryColor3b)( GLbyte, GLbyte, GLbyte);
typedef GLvoid (APIENTRY *__T_SecondaryColor3bv)( const GLbyte *);
typedef GLvoid (APIENTRY *__T_SecondaryColor3d)( GLdouble, GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_SecondaryColor3dv)( const GLdouble *);
typedef GLvoid (APIENTRY *__T_SecondaryColor3f)( GLfloat, GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_SecondaryColor3fv)( const GLfloat *);
typedef GLvoid (APIENTRY *__T_SecondaryColor3i)( GLint, GLint, GLint);
typedef GLvoid (APIENTRY *__T_SecondaryColor3iv)( const GLint *);
typedef GLvoid (APIENTRY *__T_SecondaryColor3s)( GLshort, GLshort, GLshort);
typedef GLvoid (APIENTRY *__T_SecondaryColor3sv)( const GLshort *);
typedef GLvoid (APIENTRY *__T_SecondaryColor3ub)( GLubyte, GLubyte, GLubyte);
typedef GLvoid (APIENTRY *__T_SecondaryColor3ubv)( const GLubyte *);
typedef GLvoid (APIENTRY *__T_SecondaryColor3ui)( GLuint, GLuint, GLuint);
typedef GLvoid (APIENTRY *__T_SecondaryColor3uiv)( const GLuint *);
typedef GLvoid (APIENTRY *__T_SecondaryColor3us)( GLushort, GLushort, GLushort);
typedef GLvoid (APIENTRY *__T_SecondaryColor3usv)( const GLushort *);
typedef GLvoid (APIENTRY *__T_SecondaryColorPointer)( GLint, GLenum, GLsizei, const GLvoid *);
typedef GLvoid (APIENTRY *__T_WindowPos2d)( GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_WindowPos2dv)( const GLdouble *);
typedef GLvoid (APIENTRY *__T_WindowPos2f)( GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_WindowPos2fv)( const GLfloat *);
typedef GLvoid (APIENTRY *__T_WindowPos2i)( GLint, GLint);
typedef GLvoid (APIENTRY *__T_WindowPos2iv)( const GLint *);
typedef GLvoid (APIENTRY *__T_WindowPos2s)( GLshort, GLshort);
typedef GLvoid (APIENTRY *__T_WindowPos2sv)( const GLshort *);
typedef GLvoid (APIENTRY *__T_WindowPos3d)( GLdouble, GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_WindowPos3dv)( const GLdouble *);
typedef GLvoid (APIENTRY *__T_WindowPos3f)( GLfloat, GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_WindowPos3fv)( const GLfloat *);
typedef GLvoid (APIENTRY *__T_WindowPos3i)( GLint, GLint, GLint);
typedef GLvoid (APIENTRY *__T_WindowPos3iv)( const GLint *);
typedef GLvoid (APIENTRY *__T_WindowPos3s)( GLshort, GLshort, GLshort);
typedef GLvoid (APIENTRY *__T_WindowPos3sv)( const GLshort *);
#endif
#if GL_VERSION_1_5
typedef GLvoid (APIENTRY *__T_GenQueries)( GLsizei, GLuint *);
typedef GLvoid (APIENTRY *__T_DeleteQueries)( GLsizei, const GLuint *);
typedef GLboolean (APIENTRY *__T_IsQuery)( GLuint);
typedef GLvoid (APIENTRY *__T_BeginQuery)( GLenum, GLuint);
typedef GLvoid (APIENTRY *__T_EndQuery)( GLenum);
typedef GLvoid (APIENTRY *__T_GetQueryiv)( GLenum, GLenum, GLint *);
typedef GLvoid (APIENTRY *__T_GetQueryObjectiv)( GLuint, GLenum, GLint *);
typedef GLvoid (APIENTRY *__T_GetQueryObjectuiv)( GLuint, GLenum, GLuint *);
typedef GLvoid (APIENTRY *__T_BindBuffer)( GLenum, GLuint);
typedef GLvoid (APIENTRY *__T_DeleteBuffers)( GLsizei, const GLuint *);
typedef GLvoid (APIENTRY *__T_GenBuffers)( GLsizei, GLuint *);
typedef GLboolean (APIENTRY *__T_IsBuffer)( GLuint);
typedef GLvoid (APIENTRY *__T_BufferData)( GLenum, GLsizeiptr, const GLvoid *, GLenum);
typedef GLvoid (APIENTRY *__T_BufferSubData)( GLenum, GLintptr, GLsizeiptr, const GLvoid *);
typedef GLvoid (APIENTRY *__T_GetBufferSubData)( GLenum, GLintptr, GLsizeiptr, GLvoid *);
typedef GLvoid* (APIENTRY *__T_MapBuffer)( GLenum, GLenum);
typedef GLboolean (APIENTRY *__T_UnmapBuffer)( GLenum);
typedef GLvoid (APIENTRY *__T_GetBufferParameteriv)( GLenum, GLenum, GLint *);
typedef GLvoid (APIENTRY *__T_GetBufferPointerv)( GLenum, GLenum, GLvoid* *);
#endif
#if GL_VERSION_2_0
typedef GLvoid (APIENTRY *__T_BlendEquationSeparate)( GLenum, GLenum);
typedef GLvoid (APIENTRY *__T_DrawBuffers)( GLsizei, const GLenum *);
typedef GLvoid (APIENTRY *__T_StencilOpSeparate)( GLenum, GLenum, GLenum, GLenum);
typedef GLvoid (APIENTRY *__T_StencilFuncSeparate)( GLenum, GLenum, GLint, GLuint);
typedef GLvoid (APIENTRY *__T_StencilMaskSeparate)( GLenum, GLuint);
typedef GLvoid (APIENTRY *__T_AttachShader)( GLuint, GLuint);
typedef GLvoid (APIENTRY *__T_BindAttribLocation)( GLuint, GLuint, const GLchar *);
typedef GLvoid (APIENTRY *__T_CompileShader)( GLuint);
typedef GLuint (APIENTRY *__T_CreateProgram )(GLvoid);
typedef GLuint (APIENTRY *__T_CreateShader)( GLenum);
typedef GLvoid (APIENTRY *__T_DeleteProgram)( GLuint);
typedef GLvoid (APIENTRY *__T_DeleteShader)( GLuint);
typedef GLvoid (APIENTRY *__T_DetachShader)( GLuint, GLuint);
typedef GLvoid (APIENTRY *__T_DisableVertexAttribArray)( GLuint);
typedef GLvoid (APIENTRY *__T_EnableVertexAttribArray)( GLuint);
typedef GLvoid (APIENTRY *__T_GetActiveAttrib)( GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *);
typedef GLvoid (APIENTRY *__T_GetActiveUniform)( GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *);
typedef GLvoid (APIENTRY *__T_GetAttachedShaders)( GLuint, GLsizei, GLsizei *, GLuint *);
typedef GLint (APIENTRY *__T_GetAttribLocation)( GLuint, const GLchar *);
typedef GLvoid (APIENTRY *__T_GetProgramiv)( GLuint, GLenum, GLint *);
typedef GLvoid (APIENTRY *__T_GetProgramInfoLog)( GLuint, GLsizei, GLsizei *, GLchar *);
typedef GLvoid (APIENTRY *__T_GetShaderiv)( GLuint, GLenum, GLint *);
typedef GLvoid (APIENTRY *__T_GetShaderInfoLog)( GLuint, GLsizei, GLsizei *, GLchar *);
typedef GLvoid (APIENTRY *__T_GetShaderSource)( GLuint, GLsizei, GLsizei *, GLchar *);
typedef GLint (APIENTRY *__T_GetUniformLocation)( GLuint, const GLchar *);
typedef GLvoid (APIENTRY *__T_GetUniformfv)( GLuint, GLint, GLfloat *);
typedef GLvoid (APIENTRY *__T_GetUniformiv)( GLuint, GLint, GLint *);
typedef GLvoid (APIENTRY *__T_GetVertexAttribdv)( GLuint, GLenum, GLdouble *);
typedef GLvoid (APIENTRY *__T_GetVertexAttribfv)( GLuint, GLenum, GLfloat *);
typedef GLvoid (APIENTRY *__T_GetVertexAttribiv)( GLuint, GLenum, GLint *);
typedef GLvoid (APIENTRY *__T_GetVertexAttribPointerv)( GLuint, GLenum, GLvoid* *);
typedef GLboolean (APIENTRY *__T_IsProgram)( GLuint);
typedef GLboolean (APIENTRY *__T_IsShader)( GLuint);
typedef GLvoid (APIENTRY *__T_LinkProgram)( GLuint);
typedef GLvoid (APIENTRY *__T_ShaderSource)( GLuint, GLsizei, const GLchar* *, const GLint *);
typedef GLvoid (APIENTRY *__T_UseProgram)( GLuint);
typedef GLvoid (APIENTRY *__T_Uniform1f)( GLint, GLfloat);
typedef GLvoid (APIENTRY *__T_Uniform2f)( GLint, GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_Uniform3f)( GLint, GLfloat, GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_Uniform4f)( GLint, GLfloat, GLfloat, GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_Uniform1i)( GLint, GLint);
typedef GLvoid (APIENTRY *__T_Uniform2i)( GLint, GLint, GLint);
typedef GLvoid (APIENTRY *__T_Uniform3i)( GLint, GLint, GLint, GLint);
typedef GLvoid (APIENTRY *__T_Uniform4i)( GLint, GLint, GLint, GLint, GLint);
typedef GLvoid (APIENTRY *__T_Uniform1fv)( GLint, GLsizei, const GLfloat *);
typedef GLvoid (APIENTRY *__T_Uniform2fv)( GLint, GLsizei, const GLfloat *);
typedef GLvoid (APIENTRY *__T_Uniform3fv)( GLint, GLsizei, const GLfloat *);
typedef GLvoid (APIENTRY *__T_Uniform4fv)( GLint, GLsizei, const GLfloat *);
typedef GLvoid (APIENTRY *__T_Uniform1iv)( GLint, GLsizei, const GLint *);
typedef GLvoid (APIENTRY *__T_Uniform2iv)( GLint, GLsizei, const GLint *);
typedef GLvoid (APIENTRY *__T_Uniform3iv)( GLint, GLsizei, const GLint *);
typedef GLvoid (APIENTRY *__T_Uniform4iv)( GLint, GLsizei, const GLint *);
typedef GLvoid (APIENTRY *__T_UniformMatrix2fv)( GLint, GLsizei, GLboolean, const GLfloat *);
typedef GLvoid (APIENTRY *__T_UniformMatrix3fv)( GLint, GLsizei, GLboolean, const GLfloat *);
typedef GLvoid (APIENTRY *__T_UniformMatrix4fv)( GLint, GLsizei, GLboolean, const GLfloat *);
typedef GLvoid (APIENTRY *__T_ValidateProgram)( GLuint);
typedef GLvoid (APIENTRY *__T_VertexAttrib1d)( GLuint, GLdouble);
typedef GLvoid (APIENTRY *__T_VertexAttrib1dv)( GLuint, const GLdouble *);
typedef GLvoid (APIENTRY *__T_VertexAttrib1f)( GLuint, GLfloat);
typedef GLvoid (APIENTRY *__T_VertexAttrib1fv)( GLuint, const GLfloat *);
typedef GLvoid (APIENTRY *__T_VertexAttrib1s)( GLuint, GLshort);
typedef GLvoid (APIENTRY *__T_VertexAttrib1sv)( GLuint, const GLshort *);
typedef GLvoid (APIENTRY *__T_VertexAttrib2d)( GLuint, GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_VertexAttrib2dv)( GLuint, const GLdouble *);
typedef GLvoid (APIENTRY *__T_VertexAttrib2f)( GLuint, GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_VertexAttrib2fv)( GLuint, const GLfloat *);
typedef GLvoid (APIENTRY *__T_VertexAttrib2s)( GLuint, GLshort, GLshort);
typedef GLvoid (APIENTRY *__T_VertexAttrib2sv)( GLuint, const GLshort *);
typedef GLvoid (APIENTRY *__T_VertexAttrib3d)( GLuint, GLdouble, GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_VertexAttrib3dv)( GLuint, const GLdouble *);
typedef GLvoid (APIENTRY *__T_VertexAttrib3f)( GLuint, GLfloat, GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_VertexAttrib3fv)( GLuint, const GLfloat *);
typedef GLvoid (APIENTRY *__T_VertexAttrib3s)( GLuint, GLshort, GLshort, GLshort);
typedef GLvoid (APIENTRY *__T_VertexAttrib3sv)( GLuint, const GLshort *);
typedef GLvoid (APIENTRY *__T_VertexAttrib4Nbv)( GLuint, const GLbyte *);
typedef GLvoid (APIENTRY *__T_VertexAttrib4Niv)( GLuint, const GLint *);
typedef GLvoid (APIENTRY *__T_VertexAttrib4Nsv)( GLuint, const GLshort *);
typedef GLvoid (APIENTRY *__T_VertexAttrib4Nub)( GLuint, GLubyte, GLubyte, GLubyte, GLubyte);
typedef GLvoid (APIENTRY *__T_VertexAttrib4Nubv)( GLuint, const GLubyte *);
typedef GLvoid (APIENTRY *__T_VertexAttrib4Nuiv)( GLuint, const GLuint *);
typedef GLvoid (APIENTRY *__T_VertexAttrib4Nusv)( GLuint, const GLushort *);
typedef GLvoid (APIENTRY *__T_VertexAttrib4bv)( GLuint, const GLbyte *);
typedef GLvoid (APIENTRY *__T_VertexAttrib4d)( GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
typedef GLvoid (APIENTRY *__T_VertexAttrib4dv)( GLuint, const GLdouble *);
typedef GLvoid (APIENTRY *__T_VertexAttrib4f)( GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
typedef GLvoid (APIENTRY *__T_VertexAttrib4fv)( GLuint, const GLfloat *);
typedef GLvoid (APIENTRY *__T_VertexAttrib4iv)( GLuint, const GLint *);
typedef GLvoid (APIENTRY *__T_VertexAttrib4s)( GLuint, GLshort, GLshort, GLshort, GLshort);
typedef GLvoid (APIENTRY *__T_VertexAttrib4sv)( GLuint, const GLshort *);
typedef GLvoid (APIENTRY *__T_VertexAttrib4ubv)( GLuint, const GLubyte *);
typedef GLvoid (APIENTRY *__T_VertexAttrib4uiv)( GLuint, const GLuint *);
typedef GLvoid (APIENTRY *__T_VertexAttrib4usv)( GLuint, const GLushort *);
typedef GLvoid (APIENTRY *__T_VertexAttribPointer)( GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *);
#endif

#if GL_VERSION_2_1
typedef GLvoid (APIENTRY *__T_UniformMatrix2x3fv)( GLint, GLsizei, GLboolean, const GLfloat *);
typedef GLvoid (APIENTRY *__T_UniformMatrix2x4fv)( GLint, GLsizei, GLboolean, const GLfloat *);
typedef GLvoid (APIENTRY *__T_UniformMatrix3x2fv)( GLint, GLsizei, GLboolean, const GLfloat *);
typedef GLvoid (APIENTRY *__T_UniformMatrix3x4fv)( GLint, GLsizei, GLboolean, const GLfloat *);
typedef GLvoid (APIENTRY *__T_UniformMatrix4x2fv)( GLint, GLsizei, GLboolean, const GLfloat *);
typedef GLvoid (APIENTRY *__T_UniformMatrix4x3fv)( GLint, GLsizei, GLboolean, const GLfloat *);
#endif

#if GL_ARB_vertex_program
typedef GLvoid (APIENTRY *__T_ProgramStringARB)( GLenum target, GLenum format, GLsizei len, const GLvoid *string);
typedef GLvoid (APIENTRY *__T_BindProgramARB)( GLenum target, GLuint program);
typedef GLvoid (APIENTRY *__T_DeleteProgramsARB)( GLsizei n, const GLuint *programs);
typedef GLvoid (APIENTRY *__T_GenProgramsARB)( GLsizei n, GLuint *programs);
typedef GLvoid (APIENTRY *__T_ProgramEnvParameter4dARB)( GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef GLvoid (APIENTRY *__T_ProgramEnvParameter4dvARB)( GLenum target, GLuint index, const GLdouble *params);
typedef GLvoid (APIENTRY *__T_ProgramEnvParameter4fARB)( GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef GLvoid (APIENTRY *__T_ProgramEnvParameter4fvARB)( GLenum target, GLuint index, const GLfloat *params);
typedef GLvoid (APIENTRY *__T_ProgramLocalParameter4dARB)( GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef GLvoid (APIENTRY *__T_ProgramLocalParameter4dvARB)( GLenum target, GLuint index, const GLdouble *params);
typedef GLvoid (APIENTRY *__T_ProgramLocalParameter4fARB)( GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef GLvoid (APIENTRY *__T_ProgramLocalParameter4fvARB)( GLenum target, GLuint index, const GLfloat *params);
typedef GLvoid (APIENTRY *__T_GetProgramEnvParameterdvARB)( GLenum target, GLuint index, GLdouble *params);
typedef GLvoid (APIENTRY *__T_GetProgramEnvParameterfvARB)( GLenum target, GLuint index, GLfloat *params);
typedef GLvoid (APIENTRY *__T_GetProgramLocalParameterdvARB)( GLenum target, GLuint index, GLdouble *params);
typedef GLvoid (APIENTRY *__T_GetProgramLocalParameterfvARB)( GLenum target, GLuint index, GLfloat *params);
typedef GLvoid (APIENTRY *__T_GetProgramivARB)( GLenum target, GLenum pname, GLint *params);
typedef GLvoid (APIENTRY *__T_GetProgramStringARB)( GLenum target, GLenum pname, GLvoid *string);
typedef GLboolean (APIENTRY *__T_IsProgramARB)( GLuint program);
#endif

#if GL_ARB_shader_objects
typedef GLvoid (APIENTRY *__T_DeleteObjectARB)(GLhandleARB obj);
typedef GLhandleARB (APIENTRY *__T_GetHandleARB)(GLenum pname);
typedef GLvoid (APIENTRY *__T_GetInfoLogARB)(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
typedef GLvoid (APIENTRY *__T_GetObjectParameterfvARB)(GLhandleARB obj, GLenum pname, GLfloat *params);
typedef GLvoid (APIENTRY *__T_GetObjectParameterivARB)(GLhandleARB obj, GLenum pname, GLint *params);
#endif

#if GL_ATI_vertex_array_object
typedef GLuint (APIENTRY *__T_NewObjectBufferATI)(GLsizei size, const GLvoid *pointer, GLenum usage);
typedef GLboolean (APIENTRY *__T_IsObjectBufferATI)(GLuint buffer);
typedef GLvoid (APIENTRY *__T_UpdateObjectBufferATI)(GLuint buffer, GLuint offset, GLsizei size, const GLvoid *pointer, GLenum preserve);
typedef GLvoid (APIENTRY *__T_GetObjectBufferfvATI)(GLuint buffer, GLenum pname, GLfloat *params);
typedef GLvoid (APIENTRY *__T_GetObjectBufferivATI)(GLuint buffer, GLenum pname, GLint *params);
typedef GLvoid (APIENTRY *__T_FreeObjectBufferATI)(GLuint buffer);
typedef GLvoid (APIENTRY *__T_ArrayObjectATI)(GLenum array, GLint size, GLenum type, GLsizei stride, GLuint buffer, GLuint offset);
typedef GLvoid (APIENTRY *__T_GetArrayObjectfvATI)(GLenum array, GLenum pname, GLfloat * params);
typedef GLvoid (APIENTRY *__T_GetArrayObjectivATI)(GLenum array, GLenum pname, GLint * params);
typedef GLvoid (APIENTRY *__T_VariantArrayObjectATI)(GLuint, GLenum, GLsizei, GLuint, GLuint);
typedef GLvoid (APIENTRY *__T_GetVariantArrayObjectfvATI)(GLuint, GLenum, GLfloat *);
typedef GLvoid (APIENTRY *__T_GetVariantArrayObjectivATI)(GLuint, GLenum, GLint *);
#endif

#if GL_ATI_vertex_attrib_array_object
typedef GLvoid (APIENTRY *__T_VertexAttribArrayObjectATI)(GLuint, GLint, GLenum, GLboolean, GLsizei, GLuint, GLuint);
typedef GLvoid (APIENTRY *__T_GetVertexAttribArrayObjectfvATI)(GLuint, GLenum, GLfloat *);
typedef GLvoid (APIENTRY *__T_GetVertexAttribArrayObjectivATI)(GLuint, GLenum, GLint *);
#endif

typedef GLvoid (APIENTRY *__T_AddSwapHintRectWIN)(GLint x, GLint y, GLsizei width,
                                        GLsizei height);

#if GL_EXT_framebuffer_object
typedef GLboolean (APIENTRY *__T_IsRenderbufferEXT)(GLuint);
typedef GLvoid (APIENTRY *__T_BindRenderbufferEXT)(GLenum, GLuint);
typedef GLvoid (APIENTRY *__T_DeleteRenderbuffersEXT)(GLsizei, const GLuint *);
typedef GLvoid (APIENTRY *__T_GenRenderbuffersEXT)(GLsizei, GLuint *);
typedef GLvoid (APIENTRY *__T_RenderbufferStorageEXT)(GLenum, GLenum, GLsizei, GLsizei);
typedef GLvoid (APIENTRY *__T_GetRenderbufferParameterivEXT)(GLenum, GLenum, GLint *);
typedef GLboolean (APIENTRY *__T_IsFramebufferEXT)(GLuint);
typedef GLvoid (APIENTRY *__T_BindFramebufferEXT)(GLenum, GLuint);
typedef GLvoid (APIENTRY *__T_DeleteFramebuffersEXT)(GLsizei, const GLuint *);
typedef GLvoid (APIENTRY *__T_GenFramebuffersEXT)(GLsizei, GLuint *);
typedef GLenum (APIENTRY *__T_CheckFramebufferStatusEXT)(GLenum);
typedef GLvoid (APIENTRY *__T_FramebufferTexture1DEXT)(GLenum, GLenum, GLenum, GLuint, GLint);
typedef GLvoid (APIENTRY *__T_FramebufferTexture2DEXT)(GLenum, GLenum, GLenum, GLuint, GLint);
typedef GLvoid (APIENTRY *__T_FramebufferTexture3DEXT)(GLenum, GLenum, GLenum, GLuint, GLint, GLint);
typedef GLvoid (APIENTRY *__T_FramebufferRenderbufferEXT)(GLenum, GLenum, GLenum, GLuint);
typedef GLvoid (APIENTRY *__T_GetFramebufferAttachmentParameterivEXT)(GLenum, GLenum, GLenum, GLint *);
typedef GLvoid (APIENTRY *__T_GenerateMipmapEXT)(GLenum);
#if GL_EXT_framebuffer_blit
typedef GLvoid (APIENTRY *__T_BlitFramebufferEXT)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum);
#if GL_EXT_framebuffer_multisample
typedef GLvoid (APIENTRY *__T_RenderbufferStorageMultisampleEXT)(GLenum, GLsizei, GLenum, GLsizei, GLsizei);
#endif
#endif
#endif

#if GL_ATI_element_array
typedef GLvoid (APIENTRY *__T_ElementPointerATI)(GLenum type, const GLvoid *pointer);
typedef GLvoid (APIENTRY *__T_DrawElementArrayATI)(GLenum mode, GLsizei count);
typedef GLvoid (APIENTRY *__T_DrawRangeElementArrayATI)(GLenum mode, GLuint start, GLuint end, GLsizei count);
#endif

#if GL_EXT_stencil_two_side
typedef GLvoid (APIENTRY *__T_ActiveStencilFaceEXT)(GLenum face);
#endif

#if GL_EXT_depth_bounds_test
typedef GLvoid (APIENTRY *__T_DepthBoundsEXT)(GLclampd zMin, GLclampd zMax);
#endif

#if GL_NV_occlusion_query
typedef GLvoid (APIENTRY *__T_BeginQueryNV)(GLuint);
typedef GLvoid (APIENTRY *__T_EndQueryNV)(GLvoid);
#endif


#if GL_EXT_bindable_uniform
typedef GLvoid (APIENTRY *__T_UniformBufferEXT)(GLuint program, GLint location, GLuint buffer);
typedef GLint (APIENTRY *__T_GetUniformBufferSizeEXT)(GLuint program, GLint location);
typedef GLintptr (APIENTRY *__T_GetUniformOffsetEXT)(GLuint program, GLint location);
#endif

#if GL_EXT_texture_integer
typedef GLvoid (APIENTRY *__T_ClearColorIiEXT)(GLint r, GLint g, GLint b,GLint a);
typedef GLvoid (APIENTRY *__T_ClearColorIuiEXT)(GLuint r, GLuint g, GLuint b, GLuint a);
typedef GLvoid (APIENTRY *__T_TexParameterIivEXT)(GLenum target, GLenum pname, GLint *params);
typedef GLvoid (APIENTRY *__T_TexParameterIuivEXT)(GLenum target, GLenum pname, GLuint *params);
typedef GLvoid (APIENTRY *__T_GetTexParameterIivEXT)(GLenum target, GLenum pname, GLint *params);
typedef GLvoid (APIENTRY *__T_GetTexParameterIuivEXT)(GLenum target, GLenum pname, GLuint *params);
#endif

#if GL_EXT_gpu_shader4
typedef GLvoid (APIENTRY *__T_VertexAttribI1iEXT)(GLuint index, GLint x);
typedef GLvoid (APIENTRY *__T_VertexAttribI2iEXT)(GLuint index, GLint x, GLint y);
typedef GLvoid (APIENTRY *__T_VertexAttribI3iEXT)(GLuint index, GLint x, GLint y, GLint z);
typedef GLvoid (APIENTRY *__T_VertexAttribI4iEXT)(GLuint index, GLint x, GLint y, GLint z, GLint w);

typedef GLvoid (APIENTRY *__T_VertexAttribI1uiEXT)(GLuint index, GLuint x);
typedef GLvoid (APIENTRY *__T_VertexAttribI2uiEXT)(GLuint index, GLuint x, GLuint y);
typedef GLvoid (APIENTRY *__T_VertexAttribI3uiEXT)(GLuint index, GLuint x, GLuint y, GLuint z);
typedef GLvoid (APIENTRY *__T_VertexAttribI4uiEXT)(GLuint index, GLuint x, GLuint y, GLuint z,
                            GLuint w);

typedef GLvoid (APIENTRY *__T_VertexAttribI1ivEXT)(GLuint index, const GLint *v);
typedef GLvoid (APIENTRY *__T_VertexAttribI2ivEXT)(GLuint index, const GLint *v);
typedef GLvoid (APIENTRY *__T_VertexAttribI3ivEXT)(GLuint index, const GLint *v);
typedef GLvoid (APIENTRY *__T_VertexAttribI4ivEXT)(GLuint index, const GLint *v);

typedef GLvoid (APIENTRY *__T_VertexAttribI1uivEXT)(GLuint index, const GLuint *v);
typedef GLvoid (APIENTRY *__T_VertexAttribI2uivEXT)(GLuint index, const GLuint *v);
typedef GLvoid (APIENTRY *__T_VertexAttribI3uivEXT)(GLuint index, const GLuint *v);
typedef GLvoid (APIENTRY *__T_VertexAttribI4uivEXT)(GLuint index, const GLuint *v);

typedef GLvoid (APIENTRY *__T_VertexAttribI4bvEXT)(GLuint index, const GLbyte *v);
typedef GLvoid (APIENTRY *__T_VertexAttribI4svEXT)(GLuint index, const GLshort *v);
typedef GLvoid (APIENTRY *__T_VertexAttribI4ubvEXT)(GLuint index, const GLubyte *v);
typedef GLvoid (APIENTRY *__T_VertexAttribI4usvEXT)(GLuint index, const GLushort *v);

typedef GLvoid (APIENTRY *__T_VertexAttribIPointerEXT)(GLuint index, GLint size, GLenum type,
                                GLsizei stride, const GLvoid *pointer);

typedef GLvoid (APIENTRY *__T_GetVertexAttribIivEXT)(GLuint index, GLenum pname, GLint *params);
typedef GLvoid (APIENTRY *__T_GetVertexAttribIuivEXT)(GLuint index, GLenum pname, GLuint *params);

typedef GLvoid (APIENTRY *__T_Uniform1uiEXT)(GLint location, GLuint v0);
typedef GLvoid (APIENTRY *__T_Uniform2uiEXT)(GLint location, GLuint v0, GLuint v1);
typedef GLvoid (APIENTRY *__T_Uniform3uiEXT)(GLint location, GLuint v0, GLuint v1, GLuint v2);
typedef GLvoid (APIENTRY *__T_Uniform4uiEXT)(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);

typedef GLvoid (APIENTRY *__T_Uniform1uivEXT)(GLint location, GLsizei count, const GLuint *value);
typedef GLvoid (APIENTRY *__T_Uniform2uivEXT)(GLint location, GLsizei count, const GLuint *value);
typedef GLvoid (APIENTRY *__T_Uniform3uivEXT)(GLint location, GLsizei count, const GLuint *value);
typedef GLvoid (APIENTRY *__T_Uniform4uivEXT)(GLint location, GLsizei count, const GLuint *value);

typedef GLvoid (APIENTRY *__T_GetUniformuivEXT)(GLuint program, GLint location, GLuint *params);

typedef GLvoid (APIENTRY *__T_BindFragDataLocationEXT)(GLuint program, GLuint colorNumber,
                                const GLbyte *name);
typedef GLint (APIENTRY *__T_GetFragDataLocationEXT)(GLuint program, const GLbyte *name);
#endif

#if GL_EXT_geometry_shader4
typedef GLvoid (APIENTRY *__T_ProgramParameteriEXT)(GLuint program, GLenum pname, GLint value);
typedef GLvoid (APIENTRY *__T_FramebufferTextureEXT)(GLenum target, GLenum attachment, GLuint texture, GLint level);
typedef GLvoid (APIENTRY *__T_FramebufferTextureLayerEXT)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
typedef GLvoid (APIENTRY *__T_FramebufferTextureFaceEXT)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face);
#endif

#if GL_EXT_draw_buffers2
typedef GLvoid (APIENTRY *__T_ColorMaskIndexedEXT)(GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
typedef GLvoid (APIENTRY *__T_GetBooleanIndexedvEXT)(GLenum value, GLuint index, GLboolean *data);
typedef GLvoid (APIENTRY *__T_GetIntegerIndexedvEXT)(GLenum value, GLuint index, GLint *data);
typedef GLvoid (APIENTRY *__T_EnableIndexedEXT)(GLenum target, GLuint index);
typedef GLvoid (APIENTRY *__T_DisableIndexedEXT)(GLenum target, GLuint index);
typedef GLboolean (APIENTRY *__T_IsEnabledIndexedEXT)(GLenum target, GLuint index);
#endif

#if GL_EXT_gpu_program_parameters
typedef GLvoid (APIENTRY *__T_ProgramEnvParameters4fvEXT)(GLenum target, GLuint index, GLsizei count, const GLfloat *params);
typedef GLvoid (APIENTRY *__T_ProgramLocalParameters4fvEXT)(GLenum target, GLuint index, GLsizei count, const GLfloat *params);
#endif

#if GL_ARB_color_buffer_float
typedef GLvoid (APIENTRY *__T_ClampColorARB)(GLenum target, GLenum clamp);
#endif

#if GL_ATI_separate_stencil
typedef GLvoid (APIENTRY *__T_StencilFuncSeparateATI)(GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask);
#endif

#if GL_EXT_texture_buffer_object
typedef GLvoid (APIENTRY *__T_TexBufferEXT)(GLenum target, GLenum internalformat, GLuint buffer);
#endif

#if GL_EXT_draw_instanced
typedef GLvoid (APIENTRY *__T_DrawArraysInstancedEXT)(GLenum mode,
                                                GLint first, GLsizei count, GLsizei primCount);
typedef GLvoid (APIENTRY *__T_DrawElementsInstancedEXT)(GLenum mode,
                        GLsizei count, GLenum type, const GLvoid *indices, GLsizei primCount);
#endif

#if GL_EXT_timer_query
typedef GLvoid (APIENTRY *__T_GetQueryObjecti64vEXT)(GLuint, GLenum, GLint64EXT *);
typedef GLvoid (APIENTRY *__T_GetQueryObjectui64vEXT)(GLuint, GLenum, GLuint64EXT *);
#endif
extern GLvoid __gllc_InvalidValue(__GLcontext *gc);
extern GLvoid __gllc_InvalidEnum(__GLcontext *gc);
extern GLvoid __gllc_InvalidOperation(__GLcontext *gc);
extern GLvoid __gllc_TableTooLarge(__GLcontext *gc);
extern GLvoid __gllc_Error(__GLcontext *gc, GLenum error);

extern GLvoid APIENTRY __gllc_NewList( GLuint, GLenum);
extern GLvoid APIENTRY __gllc_EndList( GLvoid);
extern GLvoid APIENTRY __gllc_CallList( GLuint);
extern GLvoid APIENTRY __gllc_CallLists( GLsizei, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_DeleteLists( GLuint, GLsizei);
extern GLuint APIENTRY __gllc_GenLists( GLsizei);
extern GLvoid APIENTRY __gllc_ListBase( GLuint);
extern GLvoid APIENTRY __gllc_Begin( GLenum);
extern GLvoid APIENTRY __gllc_Bitmap( GLsizei, GLsizei, GLfloat, GLfloat, GLfloat, GLfloat, const GLubyte *);
extern GLvoid APIENTRY __gllc_Color3b( GLbyte, GLbyte, GLbyte);
extern GLvoid APIENTRY __gllc_Color3bv( const GLbyte *);
extern GLvoid APIENTRY __gllc_Color3d( GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_Color3dv( const GLdouble *);
extern GLvoid APIENTRY __gllc_Color3f( GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_Color3fv( const GLfloat *);
extern GLvoid APIENTRY __gllc_Color3i( GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_Color3iv( const GLint *);
extern GLvoid APIENTRY __gllc_Color3s( GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_Color3sv( const GLshort *);
extern GLvoid APIENTRY __gllc_Color3ub( GLubyte, GLubyte, GLubyte);
extern GLvoid APIENTRY __gllc_Color3ubv( const GLubyte *);
extern GLvoid APIENTRY __gllc_Color3ui( GLuint, GLuint, GLuint);
extern GLvoid APIENTRY __gllc_Color3uiv( const GLuint *);
extern GLvoid APIENTRY __gllc_Color3us( GLushort, GLushort, GLushort);
extern GLvoid APIENTRY __gllc_Color3usv( const GLushort *);
extern GLvoid APIENTRY __gllc_Color4b( GLbyte, GLbyte, GLbyte, GLbyte);
extern GLvoid APIENTRY __gllc_Color4bv( const GLbyte *);
extern GLvoid APIENTRY __gllc_Color4d( GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_Color4dv( const GLdouble *);
extern GLvoid APIENTRY __gllc_Color4f( GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_Color4fv( const GLfloat *);
extern GLvoid APIENTRY __gllc_Color4i( GLint, GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_Color4iv( const GLint *);
extern GLvoid APIENTRY __gllc_Color4s( GLshort, GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_Color4sv( const GLshort *);
extern GLvoid APIENTRY __gllc_Color4ub( GLubyte, GLubyte, GLubyte, GLubyte);
extern GLvoid APIENTRY __gllc_Color4ubv( const GLubyte *);
extern GLvoid APIENTRY __gllc_Color4ui( GLuint, GLuint, GLuint, GLuint);
extern GLvoid APIENTRY __gllc_Color4uiv( const GLuint *);
extern GLvoid APIENTRY __gllc_Color4us( GLushort, GLushort, GLushort, GLushort);
extern GLvoid APIENTRY __gllc_Color4usv( const GLushort *);
extern GLvoid APIENTRY __gllc_EdgeFlag( GLboolean);
extern GLvoid APIENTRY __gllc_EdgeFlagv( const GLboolean *);
extern GLvoid APIENTRY __gllc_End( GLvoid);
extern GLvoid APIENTRY __gllc_Indexd( GLdouble);
extern GLvoid APIENTRY __gllc_Indexdv( const GLdouble *);
extern GLvoid APIENTRY __gllc_Indexf( GLfloat);
extern GLvoid APIENTRY __gllc_Indexfv( const GLfloat *);
extern GLvoid APIENTRY __gllc_Indexi( GLint);
extern GLvoid APIENTRY __gllc_Indexiv( const GLint *);
extern GLvoid APIENTRY __gllc_Indexs( GLshort);
extern GLvoid APIENTRY __gllc_Indexsv( const GLshort *);
extern GLvoid APIENTRY __gllc_Normal3b( GLbyte, GLbyte, GLbyte);
extern GLvoid APIENTRY __gllc_Normal3bv( const GLbyte *);
extern GLvoid APIENTRY __gllc_Normal3d( GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_Normal3dv( const GLdouble *);
extern GLvoid APIENTRY __gllc_Normal3f( GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_Normal3fv( const GLfloat *);
extern GLvoid APIENTRY __gllc_Normal3i( GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_Normal3iv( const GLint *);
extern GLvoid APIENTRY __gllc_Normal3s( GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_Normal3sv( const GLshort *);
extern GLvoid APIENTRY __gllc_RasterPos2d( GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_RasterPos2dv( const GLdouble *);
extern GLvoid APIENTRY __gllc_RasterPos2f( GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_RasterPos2fv( const GLfloat *);
extern GLvoid APIENTRY __gllc_RasterPos2i( GLint, GLint);
extern GLvoid APIENTRY __gllc_RasterPos2iv( const GLint *);
extern GLvoid APIENTRY __gllc_RasterPos2s( GLshort, GLshort);
extern GLvoid APIENTRY __gllc_RasterPos2sv( const GLshort *);
extern GLvoid APIENTRY __gllc_RasterPos3d( GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_RasterPos3dv( const GLdouble *);
extern GLvoid APIENTRY __gllc_RasterPos3f( GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_RasterPos3fv( const GLfloat *);
extern GLvoid APIENTRY __gllc_RasterPos3i( GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_RasterPos3iv( const GLint *);
extern GLvoid APIENTRY __gllc_RasterPos3s( GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_RasterPos3sv( const GLshort *);
extern GLvoid APIENTRY __gllc_RasterPos4d( GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_RasterPos4dv( const GLdouble *);
extern GLvoid APIENTRY __gllc_RasterPos4f( GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_RasterPos4fv( const GLfloat *);
extern GLvoid APIENTRY __gllc_RasterPos4i( GLint, GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_RasterPos4iv( const GLint *);
extern GLvoid APIENTRY __gllc_RasterPos4s( GLshort, GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_RasterPos4sv( const GLshort *);
extern GLvoid APIENTRY __gllc_Rectd( GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_Rectdv( const GLdouble *, const GLdouble *);
extern GLvoid APIENTRY __gllc_Rectf( GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_Rectfv( const GLfloat *, const GLfloat *);
extern GLvoid APIENTRY __gllc_Recti( GLint, GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_Rectiv( const GLint *, const GLint *);
extern GLvoid APIENTRY __gllc_Rects( GLshort, GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_Rectsv( const GLshort *, const GLshort *);
extern GLvoid APIENTRY __gllc_TexCoord1d( GLdouble);
extern GLvoid APIENTRY __gllc_TexCoord1dv( const GLdouble *);
extern GLvoid APIENTRY __gllc_TexCoord1f( GLfloat);
extern GLvoid APIENTRY __gllc_TexCoord1fv( const GLfloat *);
extern GLvoid APIENTRY __gllc_TexCoord1i( GLint);
extern GLvoid APIENTRY __gllc_TexCoord1iv( const GLint *);
extern GLvoid APIENTRY __gllc_TexCoord1s( GLshort);
extern GLvoid APIENTRY __gllc_TexCoord1sv( const GLshort *);
extern GLvoid APIENTRY __gllc_TexCoord2d( GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_TexCoord2dv( const GLdouble *);
extern GLvoid APIENTRY __gllc_TexCoord2f( GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_TexCoord2fv( const GLfloat *);
extern GLvoid APIENTRY __gllc_TexCoord2i( GLint, GLint);
extern GLvoid APIENTRY __gllc_TexCoord2iv( const GLint *);
extern GLvoid APIENTRY __gllc_TexCoord2s( GLshort, GLshort);
extern GLvoid APIENTRY __gllc_TexCoord2sv( const GLshort *);
extern GLvoid APIENTRY __gllc_TexCoord3d( GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_TexCoord3dv( const GLdouble *);
extern GLvoid APIENTRY __gllc_TexCoord3f( GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_TexCoord3fv( const GLfloat *);
extern GLvoid APIENTRY __gllc_TexCoord3i( GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_TexCoord3iv( const GLint *);
extern GLvoid APIENTRY __gllc_TexCoord3s( GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_TexCoord3sv( const GLshort *);
extern GLvoid APIENTRY __gllc_TexCoord4d( GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_TexCoord4dv( const GLdouble *);
extern GLvoid APIENTRY __gllc_TexCoord4f( GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_TexCoord4fv( const GLfloat *);
extern GLvoid APIENTRY __gllc_TexCoord4i( GLint, GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_TexCoord4iv( const GLint *);
extern GLvoid APIENTRY __gllc_TexCoord4s( GLshort, GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_TexCoord4sv( const GLshort *);
extern GLvoid APIENTRY __gllc_Vertex2d( GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_Vertex2dv( const GLdouble *);
extern GLvoid APIENTRY __gllc_Vertex2f( GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_Vertex2fv( const GLfloat *);
extern GLvoid APIENTRY __gllc_Vertex2i( GLint, GLint);
extern GLvoid APIENTRY __gllc_Vertex2iv( const GLint *);
extern GLvoid APIENTRY __gllc_Vertex2s( GLshort, GLshort);
extern GLvoid APIENTRY __gllc_Vertex2sv( const GLshort *);
extern GLvoid APIENTRY __gllc_Vertex3d( GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_Vertex3dv( const GLdouble *);
extern GLvoid APIENTRY __gllc_Vertex3f( GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_Vertex3fv( const GLfloat *);
extern GLvoid APIENTRY __gllc_Vertex3i( GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_Vertex3iv( const GLint *);
extern GLvoid APIENTRY __gllc_Vertex3s( GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_Vertex3sv( const GLshort *);
extern GLvoid APIENTRY __gllc_Vertex4d( GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_Vertex4dv( const GLdouble *);
extern GLvoid APIENTRY __gllc_Vertex4f( GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_Vertex4fv( const GLfloat *);
extern GLvoid APIENTRY __gllc_Vertex4i( GLint, GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_Vertex4iv( const GLint *);
extern GLvoid APIENTRY __gllc_Vertex4s( GLshort, GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_Vertex4sv( const GLshort *);
extern GLvoid APIENTRY __gllc_DrawElements( GLenum, GLsizei, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_ArrayElement( GLint);
extern GLvoid APIENTRY __gllc_DrawArrays( GLenum, GLint, GLsizei);
extern GLvoid APIENTRY __gllc_ClipPlane( GLenum, const GLdouble *);
extern GLvoid APIENTRY __gllc_ColorMaterial( GLenum, GLenum);
extern GLvoid APIENTRY __gllc_CullFace( GLenum);
extern GLvoid APIENTRY __gllc_Fogf( GLenum, GLfloat);
extern GLvoid APIENTRY __gllc_Fogfv( GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_Fogi( GLenum, GLint);
extern GLvoid APIENTRY __gllc_Fogiv( GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_FrontFace( GLenum);
extern GLvoid APIENTRY __gllc_Hint( GLenum, GLenum);
extern GLvoid APIENTRY __gllc_Lightf( GLenum, GLenum, GLfloat);
extern GLvoid APIENTRY __gllc_Lightfv( GLenum, GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_Lighti( GLenum, GLenum, GLint);
extern GLvoid APIENTRY __gllc_Lightiv( GLenum, GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_LightModelf( GLenum, GLfloat);
extern GLvoid APIENTRY __gllc_LightModelfv( GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_LightModeli( GLenum, GLint);
extern GLvoid APIENTRY __gllc_LightModeliv( GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_LineStipple( GLint, GLushort);
extern GLvoid APIENTRY __gllc_LineWidth( GLfloat);
extern GLvoid APIENTRY __gllc_Materialf( GLenum, GLenum, GLfloat);
extern GLvoid APIENTRY __gllc_Materialfv( GLenum, GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_Materiali( GLenum, GLenum, GLint);
extern GLvoid APIENTRY __gllc_Materialiv( GLenum, GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_PointSize( GLfloat);
extern GLvoid APIENTRY __gllc_PolygonMode( GLenum, GLenum);
extern GLvoid APIENTRY __gllc_PolygonStipple( const GLubyte *);
extern GLvoid APIENTRY __gllc_Scissor( GLint, GLint, GLsizei, GLsizei);
extern GLvoid APIENTRY __gllc_ShadeModel( GLenum);
extern GLvoid APIENTRY __gllc_TexParameterf( GLenum, GLenum, GLfloat);
extern GLvoid APIENTRY __gllc_TexParameterfv( GLenum, GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_TexParameteri( GLenum, GLenum, GLint);
extern GLvoid APIENTRY __gllc_TexParameteriv( GLenum, GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_TexImage1D( GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_TexImage2D( GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_TexEnvf( GLenum, GLenum, GLfloat);
extern GLvoid APIENTRY __gllc_TexEnvfv( GLenum, GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_TexEnvi( GLenum, GLenum, GLint);
extern GLvoid APIENTRY __gllc_TexEnviv( GLenum, GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_TexGend( GLenum, GLenum, GLdouble);
extern GLvoid APIENTRY __gllc_TexGendv( GLenum, GLenum, const GLdouble *);
extern GLvoid APIENTRY __gllc_TexGenf( GLenum, GLenum, GLfloat);
extern GLvoid APIENTRY __gllc_TexGenfv( GLenum, GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_TexGeni( GLenum, GLenum, GLint);
extern GLvoid APIENTRY __gllc_TexGeniv( GLenum, GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_FeedbackBuffer( GLsizei, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_SelectBuffer( GLsizei, GLuint *);
extern GLint APIENTRY __gllc_RenderMode( GLenum);
extern GLvoid APIENTRY __gllc_InitNames( GLvoid);
extern GLvoid APIENTRY __gllc_LoadName( GLuint);
extern GLvoid APIENTRY __gllc_PassThrough( GLfloat);
extern GLvoid APIENTRY __gllc_PopName( GLvoid);
extern GLvoid APIENTRY __gllc_PushName( GLuint);
extern GLvoid APIENTRY __gllc_DrawBuffer( GLenum);
extern GLvoid APIENTRY __gllc_Clear( GLbitfield);
extern GLvoid APIENTRY __gllc_ClearAccum( GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_ClearIndex( GLfloat);
extern GLvoid APIENTRY __gllc_ClearColor( GLclampf, GLclampf, GLclampf, GLclampf);
extern GLvoid APIENTRY __gllc_ClearStencil( GLint);
extern GLvoid APIENTRY __gllc_ClearDepth( GLclampd);
extern GLvoid APIENTRY __gllc_StencilMask( GLuint);
extern GLvoid APIENTRY __gllc_ColorMask( GLboolean, GLboolean, GLboolean, GLboolean);
extern GLvoid APIENTRY __gllc_DepthMask( GLboolean);
extern GLvoid APIENTRY __gllc_IndexMask( GLuint);
extern GLvoid APIENTRY __gllc_Accum( GLenum, GLfloat);
extern GLvoid APIENTRY __gllc_Disable( GLenum);
extern GLvoid APIENTRY __gllc_Enable( GLenum);
extern GLvoid APIENTRY __gllc_Finish( GLvoid);
extern GLvoid APIENTRY __gllc_Flush( GLvoid);
extern GLvoid APIENTRY __gllc_PopAttrib( GLvoid);
extern GLvoid APIENTRY __gllc_PushAttrib( GLbitfield);
extern GLvoid APIENTRY __gllc_Map1d( GLenum, GLdouble, GLdouble, GLint, GLint, const GLdouble *);
extern GLvoid APIENTRY __gllc_Map1f( GLenum, GLfloat, GLfloat, GLint, GLint, const GLfloat *);
extern GLvoid APIENTRY __gllc_Map2d( GLenum, GLdouble, GLdouble, GLint, GLint, GLdouble, GLdouble, GLint, GLint, const GLdouble *);
extern GLvoid APIENTRY __gllc_Map2f( GLenum, GLfloat, GLfloat, GLint, GLint, GLfloat, GLfloat, GLint, GLint, const GLfloat *);
extern GLvoid APIENTRY __gllc_MapGrid1d( GLint, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_MapGrid1f( GLint, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_MapGrid2d( GLint, GLdouble, GLdouble, GLint, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_MapGrid2f( GLint, GLfloat, GLfloat, GLint, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_EvalCoord1d( GLdouble);
extern GLvoid APIENTRY __gllc_EvalCoord1dv( const GLdouble *);
extern GLvoid APIENTRY __gllc_EvalCoord1f( GLfloat);
extern GLvoid APIENTRY __gllc_EvalCoord1fv( const GLfloat *);
extern GLvoid APIENTRY __gllc_EvalCoord2d( GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_EvalCoord2dv( const GLdouble *);
extern GLvoid APIENTRY __gllc_EvalCoord2f( GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_EvalCoord2fv( const GLfloat *);
extern GLvoid APIENTRY __gllc_EvalMesh1( GLenum, GLint, GLint);
extern GLvoid APIENTRY __gllc_EvalPoint1( GLint);
extern GLvoid APIENTRY __gllc_EvalMesh2( GLenum, GLint, GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_EvalPoint2( GLint, GLint);
extern GLvoid APIENTRY __gllc_AlphaFunc( GLenum, GLclampf);
extern GLvoid APIENTRY __gllc_BlendFunc( GLenum, GLenum);
extern GLvoid APIENTRY __gllc_LogicOp( GLenum);
extern GLvoid APIENTRY __gllc_StencilFunc( GLenum, GLint, GLuint);
extern GLvoid APIENTRY __gllc_StencilOp( GLenum, GLenum, GLenum);
extern GLvoid APIENTRY __gllc_DepthFunc( GLenum);
extern GLvoid APIENTRY __gllc_PixelZoom( GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_PixelTransferf( GLenum, GLfloat);
extern GLvoid APIENTRY __gllc_PixelTransferi( GLenum, GLint);
extern GLvoid APIENTRY __gllc_PixelStoref( GLenum, GLfloat);
extern GLvoid APIENTRY __gllc_PixelStorei( GLenum, GLint);
extern GLvoid APIENTRY __gllc_PixelMapfv( GLenum, GLint, const GLfloat *);
extern GLvoid APIENTRY __gllc_PixelMapuiv( GLenum, GLint, const GLuint *);
extern GLvoid APIENTRY __gllc_PixelMapusv( GLenum, GLint, const GLushort *);
extern GLvoid APIENTRY __gllc_ReadBuffer( GLenum);
extern GLvoid APIENTRY __gllc_CopyPixels( GLint, GLint, GLsizei, GLsizei, GLenum);
extern GLvoid APIENTRY __gllc_ReadPixels( GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid *);
extern GLvoid APIENTRY __gllc_DrawPixels( GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_GetBooleanv( GLenum, GLboolean *);
extern GLvoid APIENTRY __gllc_GetClipPlane( GLenum, GLdouble *);
extern GLvoid APIENTRY __gllc_GetDoublev( GLenum, GLdouble *);
extern GLenum APIENTRY __gllc_GetError( GLvoid);
extern GLvoid APIENTRY __gllc_GetFloatv( GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetIntegerv( GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetLightfv( GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetLightiv( GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetMapdv( GLenum, GLenum, GLdouble *);
extern GLvoid APIENTRY __gllc_GetMapfv( GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetMapiv( GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetMaterialfv( GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetMaterialiv( GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetPixelMapfv( GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetPixelMapuiv( GLenum, GLuint *);
extern GLvoid APIENTRY __gllc_GetPixelMapusv( GLenum, GLushort *);
extern GLvoid APIENTRY __gllc_GetPolygonStipple( GLubyte *);
extern const GLubyte * APIENTRY __gllc_GetString( GLenum);
extern GLvoid APIENTRY __gllc_GetTexEnvfv( GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetTexEnviv( GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetTexGendv( GLenum, GLenum, GLdouble *);
extern GLvoid APIENTRY __gllc_GetTexGenfv( GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetTexGeniv( GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetTexImage( GLenum, GLint, GLenum, GLenum, GLvoid *);
extern GLvoid APIENTRY __gllc_GetTexParameterfv( GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetTexParameteriv( GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetTexLevelParameterfv( GLenum, GLint, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetTexLevelParameteriv( GLenum, GLint, GLenum, GLint *);
extern GLboolean APIENTRY __gllc_IsEnabled( GLenum);
extern GLboolean APIENTRY __gllc_IsList( GLuint);
extern GLvoid APIENTRY __gllc_DepthRange( GLclampd, GLclampd);
extern GLvoid APIENTRY __gllc_Frustum( GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_LoadIdentity( GLvoid);
extern GLvoid APIENTRY __gllc_LoadMatrixf( const GLfloat *);
extern GLvoid APIENTRY __gllc_LoadMatrixd( const GLdouble *);
extern GLvoid APIENTRY __gllc_MatrixMode( GLenum);
extern GLvoid APIENTRY __gllc_MultMatrixf( const GLfloat *);
extern GLvoid APIENTRY __gllc_MultMatrixd( const GLdouble *);
extern GLvoid APIENTRY __gllc_Ortho( GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_PopMatrix( GLvoid);
extern GLvoid APIENTRY __gllc_PushMatrix( GLvoid);
extern GLvoid APIENTRY __gllc_Rotated( GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_Rotatef( GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_Scaled( GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_Scalef( GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_Translated( GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_Translatef( GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_Viewport( GLint, GLint, GLsizei, GLsizei);
extern GLvoid APIENTRY __gllc_ColorSubTable( GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_ColorTable( GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_CopyColorTable( GLenum, GLenum, GLint, GLint, GLsizei);
extern GLvoid APIENTRY __gllc_GetColorTable( GLenum, GLenum, GLenum, GLvoid *);
extern GLvoid APIENTRY __gllc_GetColorTableParameterfv( GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetColorTableParameteriv( GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_ColorPointer( GLint, GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_DisableClientState( GLenum);
extern GLvoid APIENTRY __gllc_EdgeFlagPointer( GLsizei, const GLboolean *);
extern GLvoid APIENTRY __gllc_EnableClientState( GLenum);
extern GLvoid APIENTRY __gllc_GetPointerv( GLenum, GLvoid* *);
extern GLvoid APIENTRY __gllc_IndexPointer( GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_InterleavedArrays( GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_NormalPointer( GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_TexCoordPointer( GLint, GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_VertexPointer( GLint, GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_PolygonOffset( GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_CopyTexImage1D( GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLint);
extern GLvoid APIENTRY __gllc_CopyTexImage2D( GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint);
extern GLvoid APIENTRY __gllc_CopyTexSubImage1D( GLenum, GLint, GLint, GLint, GLint, GLsizei);
extern GLvoid APIENTRY __gllc_CopyTexSubImage2D( GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
extern GLvoid APIENTRY __gllc_TexSubImage1D( GLenum, GLint, GLint, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_TexSubImage2D( GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLboolean APIENTRY __gllc_AreTexturesResident( GLsizei, const GLuint *, GLboolean *);
extern GLvoid APIENTRY __gllc_BindTexture( GLenum, GLuint);
extern GLvoid APIENTRY __gllc_DeleteTextures( GLsizei, const GLuint *);
extern GLvoid APIENTRY __gllc_GenTextures( GLsizei, GLuint *);
extern GLboolean APIENTRY __gllc_IsTexture( GLuint);
extern GLvoid APIENTRY __gllc_PrioritizeTextures( GLsizei, const GLuint *, const GLclampf *);
extern GLvoid APIENTRY __gllc_Indexub( GLubyte);
extern GLvoid APIENTRY __gllc_Indexubv( const GLubyte *);
extern GLvoid APIENTRY __gllc_PopClientAttrib( GLvoid);
extern GLvoid APIENTRY __gllc_PushClientAttrib( GLbitfield);

#if GL_VERSION_1_2
extern GLvoid APIENTRY __gllc_BlendColor( GLclampf, GLclampf, GLclampf, GLclampf);
extern GLvoid APIENTRY __gllc_BlendEquation( GLenum);
extern GLvoid APIENTRY __gllc_DrawRangeElements( GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_ColorTable( GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_ColorTableParameterfv( GLenum, GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_ColorTableParameteriv( GLenum, GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_CopyColorTable( GLenum, GLenum, GLint, GLint, GLsizei);
extern GLvoid APIENTRY __gllc_GetColorTable( GLenum, GLenum, GLenum, GLvoid *);
extern GLvoid APIENTRY __gllc_GetColorTableParameterfv( GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetColorTableParameteriv( GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_ColorSubTable( GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_CopyColorSubTable( GLenum, GLsizei, GLint, GLint, GLsizei);
extern GLvoid APIENTRY __gllc_ConvolutionFilter1D( GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_ConvolutionFilter2D( GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_ConvolutionParameterf( GLenum, GLenum, GLfloat);
extern GLvoid APIENTRY __gllc_ConvolutionParameterfv( GLenum, GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_ConvolutionParameteri( GLenum, GLenum, GLint);
extern GLvoid APIENTRY __gllc_ConvolutionParameteriv( GLenum, GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_CopyConvolutionFilter1D( GLenum, GLenum, GLint, GLint, GLsizei);
extern GLvoid APIENTRY __gllc_CopyConvolutionFilter2D( GLenum, GLenum, GLint, GLint, GLsizei, GLsizei);
extern GLvoid APIENTRY __gllc_GetConvolutionFilter( GLenum, GLenum, GLenum, GLvoid *);
extern GLvoid APIENTRY __gllc_GetConvolutionParameterfv( GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetConvolutionParameteriv( GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetSeparableFilter( GLenum, GLenum, GLenum, GLvoid *, GLvoid *, GLvoid *);
extern GLvoid APIENTRY __gllc_SeparableFilter2D( GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *, const GLvoid *);
extern GLvoid APIENTRY __gllc_GetHistogram( GLenum, GLboolean, GLenum, GLenum, GLvoid *);
extern GLvoid APIENTRY __gllc_GetHistogramParameterfv( GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetHistogramParameteriv( GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetMinmax( GLenum, GLboolean, GLenum, GLenum, GLvoid *);
extern GLvoid APIENTRY __gllc_GetMinmaxParameterfv( GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetMinmaxParameteriv( GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_Histogram( GLenum, GLsizei, GLenum, GLboolean);
extern GLvoid APIENTRY __gllc_Minmax( GLenum, GLenum, GLboolean);
extern GLvoid APIENTRY __gllc_ResetHistogram( GLenum);
extern GLvoid APIENTRY __gllc_ResetMinmax( GLenum);
extern GLvoid APIENTRY __gllc_TexImage3D( GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_TexSubImage3D( GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __gllc_CopyTexSubImage3D( GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
#endif
#if GL_VERSION_1_3
extern GLvoid APIENTRY __gllc_ActiveTexture( GLenum);
extern GLvoid APIENTRY __gllc_ClientActiveTexture( GLenum);
extern GLvoid APIENTRY __gllc_MultiTexCoord1d( GLenum, GLdouble);
extern GLvoid APIENTRY __gllc_MultiTexCoord1dv( GLenum, const GLdouble *);
extern GLvoid APIENTRY __gllc_MultiTexCoord1f( GLenum, GLfloat);
extern GLvoid APIENTRY __gllc_MultiTexCoord1fv( GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_MultiTexCoord1i( GLenum, GLint);
extern GLvoid APIENTRY __gllc_MultiTexCoord1iv( GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_MultiTexCoord1s( GLenum, GLshort);
extern GLvoid APIENTRY __gllc_MultiTexCoord1sv( GLenum, const GLshort *);
extern GLvoid APIENTRY __gllc_MultiTexCoord2d( GLenum, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_MultiTexCoord2dv( GLenum, const GLdouble *);
extern GLvoid APIENTRY __gllc_MultiTexCoord2f( GLenum, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_MultiTexCoord2fv( GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_MultiTexCoord2i( GLenum, GLint, GLint);
extern GLvoid APIENTRY __gllc_MultiTexCoord2iv( GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_MultiTexCoord2s( GLenum, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_MultiTexCoord2sv( GLenum, const GLshort *);
extern GLvoid APIENTRY __gllc_MultiTexCoord3d( GLenum, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_MultiTexCoord3dv( GLenum, const GLdouble *);
extern GLvoid APIENTRY __gllc_MultiTexCoord3f( GLenum, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_MultiTexCoord3fv( GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_MultiTexCoord3i( GLenum, GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_MultiTexCoord3iv( GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_MultiTexCoord3s( GLenum, GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_MultiTexCoord3sv( GLenum, const GLshort *);
extern GLvoid APIENTRY __gllc_MultiTexCoord4d( GLenum, GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_MultiTexCoord4dv( GLenum, const GLdouble *);
extern GLvoid APIENTRY __gllc_MultiTexCoord4f( GLenum, GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_MultiTexCoord4fv( GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_MultiTexCoord4i( GLenum, GLint, GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_MultiTexCoord4iv( GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_MultiTexCoord4s( GLenum, GLshort, GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_MultiTexCoord4sv( GLenum, const GLshort *);
extern GLvoid APIENTRY __gllc_LoadTransposeMatrixf( const GLfloat *);
extern GLvoid APIENTRY __gllc_LoadTransposeMatrixd( const GLdouble *);
extern GLvoid APIENTRY __gllc_MultTransposeMatrixf( const GLfloat *);
extern GLvoid APIENTRY __gllc_MultTransposeMatrixd( const GLdouble *);
extern GLvoid APIENTRY __gllc_SampleCoverage( GLclampf, GLboolean);
extern GLvoid APIENTRY __gllc_CompressedTexImage3D( GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_CompressedTexImage2D( GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_CompressedTexImage1D( GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_CompressedTexSubImage3D( GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_CompressedTexSubImage2D( GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_CompressedTexSubImage1D( GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_GetCompressedTexImage( GLenum, GLint, GLvoid *);
#endif
#if GL_VERSION_1_4
extern GLvoid APIENTRY __gllc_BlendFuncSeparate( GLenum, GLenum, GLenum, GLenum);
extern GLvoid APIENTRY __gllc_FogCoordf( GLfloat);
extern GLvoid APIENTRY __gllc_FogCoordfv( const GLfloat *);
extern GLvoid APIENTRY __gllc_FogCoordd( GLdouble);
extern GLvoid APIENTRY __gllc_FogCoorddv( const GLdouble *);
extern GLvoid APIENTRY __gllc_FogCoordPointer( GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_MultiDrawArrays( GLenum, GLint *, GLsizei *, GLsizei);
extern GLvoid APIENTRY __gllc_MultiDrawElements( GLenum, const GLsizei *, GLenum, const GLvoid* *, GLsizei);
extern GLvoid APIENTRY __gllc_PointParameterf( GLenum, GLfloat);
extern GLvoid APIENTRY __gllc_PointParameterfv( GLenum, const GLfloat *);
extern GLvoid APIENTRY __gllc_PointParameteri( GLenum, GLint);
extern GLvoid APIENTRY __gllc_PointParameteriv( GLenum, const GLint *);
extern GLvoid APIENTRY __gllc_SecondaryColor3b( GLbyte, GLbyte, GLbyte);
extern GLvoid APIENTRY __gllc_SecondaryColor3bv( const GLbyte *);
extern GLvoid APIENTRY __gllc_SecondaryColor3d( GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_SecondaryColor3dv( const GLdouble *);
extern GLvoid APIENTRY __gllc_SecondaryColor3f( GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_SecondaryColor3fv( const GLfloat *);
extern GLvoid APIENTRY __gllc_SecondaryColor3i( GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_SecondaryColor3iv( const GLint *);
extern GLvoid APIENTRY __gllc_SecondaryColor3s( GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_SecondaryColor3sv( const GLshort *);
extern GLvoid APIENTRY __gllc_SecondaryColor3ub( GLubyte, GLubyte, GLubyte);
extern GLvoid APIENTRY __gllc_SecondaryColor3ubv( const GLubyte *);
extern GLvoid APIENTRY __gllc_SecondaryColor3ui( GLuint, GLuint, GLuint);
extern GLvoid APIENTRY __gllc_SecondaryColor3uiv( const GLuint *);
extern GLvoid APIENTRY __gllc_SecondaryColor3us( GLushort, GLushort, GLushort);
extern GLvoid APIENTRY __gllc_SecondaryColor3usv( const GLushort *);
extern GLvoid APIENTRY __gllc_SecondaryColorPointer( GLint, GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __gllc_WindowPos2d( GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_WindowPos2dv( const GLdouble *);
extern GLvoid APIENTRY __gllc_WindowPos2f( GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_WindowPos2fv( const GLfloat *);
extern GLvoid APIENTRY __gllc_WindowPos2i( GLint, GLint);
extern GLvoid APIENTRY __gllc_WindowPos2iv( const GLint *);
extern GLvoid APIENTRY __gllc_WindowPos2s( GLshort, GLshort);
extern GLvoid APIENTRY __gllc_WindowPos2sv( const GLshort *);
extern GLvoid APIENTRY __gllc_WindowPos3d( GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_WindowPos3dv( const GLdouble *);
extern GLvoid APIENTRY __gllc_WindowPos3f( GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_WindowPos3fv( const GLfloat *);
extern GLvoid APIENTRY __gllc_WindowPos3i( GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_WindowPos3iv( const GLint *);
extern GLvoid APIENTRY __gllc_WindowPos3s( GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_WindowPos3sv( const GLshort *);
#endif
#if GL_VERSION_1_5
extern GLvoid APIENTRY __gllc_GenQueries( GLsizei, GLuint *);
extern GLvoid APIENTRY __gllc_DeleteQueries( GLsizei, const GLuint *);
extern GLboolean APIENTRY __gllc_IsQuery( GLuint);
extern GLvoid APIENTRY __gllc_BeginQuery( GLenum, GLuint);
extern GLvoid APIENTRY __gllc_EndQuery( GLenum);
extern GLvoid APIENTRY __gllc_GetQueryiv( GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetQueryObjectiv( GLuint, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetQueryObjectuiv( GLuint, GLenum, GLuint *);
extern GLvoid APIENTRY __gllc_BindBuffer( GLenum, GLuint);
extern GLvoid APIENTRY __gllc_DeleteBuffers( GLsizei, const GLuint *);
extern GLvoid APIENTRY __gllc_GenBuffers( GLsizei, GLuint *);
extern GLboolean APIENTRY __gllc_IsBuffer( GLuint);
extern GLvoid APIENTRY __gllc_BufferData( GLenum, GLsizeiptr, const GLvoid *, GLenum);
extern GLvoid APIENTRY __gllc_BufferSubData( GLenum, GLintptr, GLsizeiptr, const GLvoid *);
extern GLvoid APIENTRY __gllc_GetBufferSubData( GLenum, GLintptr, GLsizeiptr, GLvoid *);
extern GLvoid* APIENTRY __gllc_MapBuffer( GLenum, GLenum);
extern GLboolean APIENTRY __gllc_UnmapBuffer( GLenum);
extern GLvoid APIENTRY __gllc_GetBufferParameteriv( GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetBufferPointerv( GLenum, GLenum, GLvoid* *);
#endif
#if GL_VERSION_2_0
extern GLvoid APIENTRY __gllc_BlendEquationSeparate( GLenum, GLenum);
extern GLvoid APIENTRY __gllc_DrawBuffers( GLsizei, const GLenum *);
extern GLvoid APIENTRY __gllc_StencilOpSeparate( GLenum, GLenum, GLenum, GLenum);
extern GLvoid APIENTRY __gllc_StencilFuncSeparate( GLenum, GLenum, GLint, GLuint);
extern GLvoid APIENTRY __gllc_StencilMaskSeparate( GLenum, GLuint);
extern GLvoid APIENTRY __gllc_AttachShader( GLuint, GLuint);
extern GLvoid APIENTRY __gllc_BindAttribLocation( GLuint, GLuint, const GLchar *);
extern GLvoid APIENTRY __gllc_CompileShader( GLuint);
extern GLuint APIENTRY __gllc_CreateProgram (GLvoid);
extern GLuint APIENTRY __gllc_CreateShader( GLenum);
extern GLvoid APIENTRY __gllc_DeleteProgram( GLuint);
extern GLvoid APIENTRY __gllc_DeleteShader( GLuint);
extern GLvoid APIENTRY __gllc_DetachShader( GLuint, GLuint);
extern GLvoid APIENTRY __gllc_DisableVertexAttribArray( GLuint);
extern GLvoid APIENTRY __gllc_EnableVertexAttribArray( GLuint);
extern GLvoid APIENTRY __gllc_GetActiveAttrib( GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *);
extern GLvoid APIENTRY __gllc_GetActiveUniform( GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *);
extern GLvoid APIENTRY __gllc_GetAttachedShaders( GLuint, GLsizei, GLsizei *, GLuint *);
extern GLint APIENTRY __gllc_GetAttribLocation( GLuint, const GLchar *);
extern GLvoid APIENTRY __gllc_GetProgramiv( GLuint, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetProgramInfoLog( GLuint, GLsizei, GLsizei *, GLchar *);
extern GLvoid APIENTRY __gllc_GetShaderiv( GLuint, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetShaderInfoLog( GLuint, GLsizei, GLsizei *, GLchar *);
extern GLvoid APIENTRY __gllc_GetShaderSource( GLuint, GLsizei, GLsizei *, GLchar *);
extern GLint APIENTRY __gllc_GetUniformLocation( GLuint, const GLchar *);
extern GLvoid APIENTRY __gllc_GetUniformfv( GLuint, GLint, GLfloat *);
extern GLvoid APIENTRY __gllc_GetUniformiv( GLuint, GLint, GLint *);
extern GLvoid APIENTRY __gllc_GetVertexAttribdv( GLuint, GLenum, GLdouble *);
extern GLvoid APIENTRY __gllc_GetVertexAttribfv( GLuint, GLenum, GLfloat *);
extern GLvoid APIENTRY __gllc_GetVertexAttribiv( GLuint, GLenum, GLint *);
extern GLvoid APIENTRY __gllc_GetVertexAttribPointerv( GLuint, GLenum, GLvoid* *);
extern GLboolean APIENTRY __gllc_IsProgram( GLuint);
extern GLboolean APIENTRY __gllc_IsShader( GLuint);
extern GLvoid APIENTRY __gllc_LinkProgram( GLuint);
extern GLvoid APIENTRY __gllc_ShaderSource( GLuint, GLsizei, const GLchar* *, const GLint *);
extern GLvoid APIENTRY __gllc_UseProgram( GLuint);
extern GLvoid APIENTRY __gllc_Uniform1f( GLint, GLfloat);
extern GLvoid APIENTRY __gllc_Uniform2f( GLint, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_Uniform3f( GLint, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_Uniform4f( GLint, GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_Uniform1i( GLint, GLint);
extern GLvoid APIENTRY __gllc_Uniform2i( GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_Uniform3i( GLint, GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_Uniform4i( GLint, GLint, GLint, GLint, GLint);
extern GLvoid APIENTRY __gllc_Uniform1fv( GLint, GLsizei, const GLfloat *);
extern GLvoid APIENTRY __gllc_Uniform2fv( GLint, GLsizei, const GLfloat *);
extern GLvoid APIENTRY __gllc_Uniform3fv( GLint, GLsizei, const GLfloat *);
extern GLvoid APIENTRY __gllc_Uniform4fv( GLint, GLsizei, const GLfloat *);
extern GLvoid APIENTRY __gllc_Uniform1iv( GLint, GLsizei, const GLint *);
extern GLvoid APIENTRY __gllc_Uniform2iv( GLint, GLsizei, const GLint *);
extern GLvoid APIENTRY __gllc_Uniform3iv( GLint, GLsizei, const GLint *);
extern GLvoid APIENTRY __gllc_Uniform4iv( GLint, GLsizei, const GLint *);
extern GLvoid APIENTRY __gllc_UniformMatrix2fv( GLint, GLsizei, GLboolean, const GLfloat *);
extern GLvoid APIENTRY __gllc_UniformMatrix3fv( GLint, GLsizei, GLboolean, const GLfloat *);
extern GLvoid APIENTRY __gllc_UniformMatrix4fv( GLint, GLsizei, GLboolean, const GLfloat *);
extern GLvoid APIENTRY __gllc_ValidateProgram( GLuint);
extern GLvoid APIENTRY __gllc_VertexAttrib1d( GLuint, GLdouble);
extern GLvoid APIENTRY __gllc_VertexAttrib1dv( GLuint, const GLdouble *);
extern GLvoid APIENTRY __gllc_VertexAttrib1f( GLuint, GLfloat);
extern GLvoid APIENTRY __gllc_VertexAttrib1fv( GLuint, const GLfloat *);
extern GLvoid APIENTRY __gllc_VertexAttrib1s( GLuint, GLshort);
extern GLvoid APIENTRY __gllc_VertexAttrib1sv( GLuint, const GLshort *);
extern GLvoid APIENTRY __gllc_VertexAttrib2d( GLuint, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_VertexAttrib2dv( GLuint, const GLdouble *);
extern GLvoid APIENTRY __gllc_VertexAttrib2f( GLuint, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_VertexAttrib2fv( GLuint, const GLfloat *);
extern GLvoid APIENTRY __gllc_VertexAttrib2s( GLuint, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_VertexAttrib2sv( GLuint, const GLshort *);
extern GLvoid APIENTRY __gllc_VertexAttrib3d( GLuint, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_VertexAttrib3dv( GLuint, const GLdouble *);
extern GLvoid APIENTRY __gllc_VertexAttrib3f( GLuint, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_VertexAttrib3fv( GLuint, const GLfloat *);
extern GLvoid APIENTRY __gllc_VertexAttrib3s( GLuint, GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_VertexAttrib3sv( GLuint, const GLshort *);
extern GLvoid APIENTRY __gllc_VertexAttrib4Nbv( GLuint, const GLbyte *);
extern GLvoid APIENTRY __gllc_VertexAttrib4Niv( GLuint, const GLint *);
extern GLvoid APIENTRY __gllc_VertexAttrib4Nsv( GLuint, const GLshort *);
extern GLvoid APIENTRY __gllc_VertexAttrib4Nub( GLuint, GLubyte, GLubyte, GLubyte, GLubyte);
extern GLvoid APIENTRY __gllc_VertexAttrib4Nubv( GLuint, const GLubyte *);
extern GLvoid APIENTRY __gllc_VertexAttrib4Nuiv( GLuint, const GLuint *);
extern GLvoid APIENTRY __gllc_VertexAttrib4Nusv( GLuint, const GLushort *);
extern GLvoid APIENTRY __gllc_VertexAttrib4bv( GLuint, const GLbyte *);
extern GLvoid APIENTRY __gllc_VertexAttrib4d( GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __gllc_VertexAttrib4dv( GLuint, const GLdouble *);
extern GLvoid APIENTRY __gllc_VertexAttrib4f( GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __gllc_VertexAttrib4fv( GLuint, const GLfloat *);
extern GLvoid APIENTRY __gllc_VertexAttrib4iv( GLuint, const GLint *);
extern GLvoid APIENTRY __gllc_VertexAttrib4s( GLuint, GLshort, GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __gllc_VertexAttrib4sv( GLuint, const GLshort *);
extern GLvoid APIENTRY __gllc_VertexAttrib4ubv( GLuint, const GLubyte *);
extern GLvoid APIENTRY __gllc_VertexAttrib4uiv( GLuint, const GLuint *);
extern GLvoid APIENTRY __gllc_VertexAttrib4usv( GLuint, const GLushort *);
extern GLvoid APIENTRY __gllc_VertexAttribPointer( GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *);
#endif

#if GL_VERSION_2_1
extern GLvoid APIENTRY __gllc_UniformMatrix2x3fv( GLint, GLsizei, GLboolean, const GLfloat *);
extern GLvoid APIENTRY __gllc_UniformMatrix2x4fv( GLint, GLsizei, GLboolean, const GLfloat *);
extern GLvoid APIENTRY __gllc_UniformMatrix3x2fv( GLint, GLsizei, GLboolean, const GLfloat *);
extern GLvoid APIENTRY __gllc_UniformMatrix3x4fv( GLint, GLsizei, GLboolean, const GLfloat *);
extern GLvoid APIENTRY __gllc_UniformMatrix4x2fv( GLint, GLsizei, GLboolean, const GLfloat *);
extern GLvoid APIENTRY __gllc_UniformMatrix4x3fv( GLint, GLsizei, GLboolean, const GLfloat *);
#endif

#if GL_ARB_vertex_program
extern GLvoid APIENTRY __gllc_ProgramStringARB( GLenum target, GLenum format, GLsizei len, const GLvoid *string);
extern GLvoid APIENTRY __gllc_BindProgramARB( GLenum target, GLuint program);
extern GLvoid APIENTRY __gllc_DeleteProgramsARB( GLsizei n, const GLuint *programs);
extern GLvoid APIENTRY __gllc_GenProgramsARB( GLsizei n, GLuint *programs);
extern GLvoid APIENTRY __gllc_ProgramEnvParameter4dARB( GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern GLvoid APIENTRY __gllc_ProgramEnvParameter4dvARB( GLenum target, GLuint index, const GLdouble *params);
extern GLvoid APIENTRY __gllc_ProgramEnvParameter4fARB( GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern GLvoid APIENTRY __gllc_ProgramEnvParameter4fvARB( GLenum target, GLuint index, const GLfloat *params);
extern GLvoid APIENTRY __gllc_ProgramLocalParameter4dARB( GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern GLvoid APIENTRY __gllc_ProgramLocalParameter4dvARB( GLenum target, GLuint index, const GLdouble *params);
extern GLvoid APIENTRY __gllc_ProgramLocalParameter4fARB( GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern GLvoid APIENTRY __gllc_ProgramLocalParameter4fvARB( GLenum target, GLuint index, const GLfloat *params);
extern GLvoid APIENTRY __gllc_GetProgramEnvParameterdvARB( GLenum target, GLuint index, GLdouble *params);
extern GLvoid APIENTRY __gllc_GetProgramEnvParameterfvARB( GLenum target, GLuint index, GLfloat *params);
extern GLvoid APIENTRY __gllc_GetProgramLocalParameterdvARB( GLenum target, GLuint index, GLdouble *params);
extern GLvoid APIENTRY __gllc_GetProgramLocalParameterfvARB( GLenum target, GLuint index, GLfloat *params);
extern GLvoid APIENTRY __gllc_GetProgramivARB( GLenum target, GLenum pname, GLint *params);
extern GLvoid APIENTRY __gllc_GetProgramStringARB( GLenum target, GLenum pname, GLvoid *string);
extern GLboolean APIENTRY __gllc_IsProgramARB( GLuint program);
#endif

#if GL_ARB_shader_objects
extern GLvoid APIENTRY __gllc_DeleteObjectARB(GLhandleARB obj);
extern GLhandleARB APIENTRY __gllc_GetHandleARB(GLenum pname);
extern GLvoid APIENTRY __gllc_GetInfoLogARB(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
extern GLvoid APIENTRY __gllc_GetObjectParameterfvARB(GLhandleARB obj, GLenum pname, GLfloat *params);
extern GLvoid APIENTRY __gllc_GetObjectParameterivARB(GLhandleARB obj, GLenum pname, GLint *params);
#endif

#if GL_ATI_vertex_array_object
extern GLuint APIENTRY __gllc_NewObjectBufferATI(GLsizei size, const GLvoid *pointer, GLenum usage);
extern GLboolean APIENTRY __gllc_IsObjectBufferATI(GLuint buffer);
extern GLvoid APIENTRY __gllc_UpdateObjectBufferATI(GLuint buffer, GLuint offset, GLsizei size, const GLvoid *pointer, GLenum preserve);
extern GLvoid APIENTRY __gllc_GetObjectBufferfvATI(GLuint buffer, GLenum pname, GLfloat *params);
extern GLvoid APIENTRY __gllc_GetObjectBufferivATI(GLuint buffer, GLenum pname, GLint *params);
extern GLvoid APIENTRY __gllc_FreeObjectBufferATI(GLuint buffer);
extern GLvoid APIENTRY __gllc_ArrayObjectATI(GLenum array, GLint size, GLenum type, GLsizei stride, GLuint buffer, GLuint offset);
extern GLvoid APIENTRY __gllc_GetArrayObjectfvATI(GLenum array, GLenum pname, GLfloat * params);
extern GLvoid APIENTRY __gllc_GetArrayObjectivATI(GLenum array, GLenum pname, GLint * params);
#endif

#if GL_ATI_element_array
extern GLvoid APIENTRY __gllc_ElementPointerATI(GLenum type, const GLvoid *pointer);
extern GLvoid APIENTRY __gllc_DrawElementArrayATI(GLenum mode, GLsizei count);
extern GLvoid APIENTRY __gllc_DrawRangeElementArrayATI(GLenum mode, GLuint start, GLuint end, GLsizei count);
#endif

#if GL_EXT_stencil_two_side
extern GLvoid APIENTRY __gllc_ActiveStencilFaceEXT(GLenum face);
#endif

#if GL_EXT_depth_bounds_test
extern GLvoid APIENTRY __gllc_DepthBoundsEXT(GLclampd zMin, GLclampd zMax);
#endif

#if GL_NV_occlusion_query
extern GLvoid APIENTRY __gllc_BeginQueryNV(GLuint);
extern GLvoid APIENTRY __gllc_EndQueryNV(GLvoid);
#endif


#if GL_EXT_bindable_uniform
extern GLvoid APIENTRY __gllc_UniformBufferEXT(GLuint program, GLint location, GLuint buffer);
extern GLint APIENTRY __gllc_GetUniformBufferSizeEXT(GLuint program, GLint location);
extern GLintptr APIENTRY __gllc_GetUniformOffsetEXT(GLuint program, GLint location);
#endif

#if GL_EXT_texture_integer
extern GLvoid APIENTRY __gllc_ClearColorIiEXT(GLint r, GLint g, GLint b,GLint a);
extern GLvoid APIENTRY __gllc_ClearColorIuiEXT(GLuint r, GLuint g, GLuint b, GLuint a);
extern GLvoid APIENTRY __gllc_TexParameterIivEXT(GLenum target, GLenum pname, GLint *params);
extern GLvoid APIENTRY __gllc_TexParameterIuivEXT(GLenum target, GLenum pname, GLuint *params);
extern GLvoid APIENTRY __gllc_GetTexParameterIivEXT(GLenum target, GLenum pname, GLint *params);
extern GLvoid APIENTRY __gllc_GetTexParameterIuivEXT(GLenum target, GLenum pname, GLuint *params);
#endif

#if GL_EXT_gpu_shader4
extern GLvoid APIENTRY __gllc_VertexAttribI1iEXT(GLuint index, GLint x);
extern GLvoid APIENTRY __gllc_VertexAttribI2iEXT(GLuint index, GLint x, GLint y);
extern GLvoid APIENTRY __gllc_VertexAttribI3iEXT(GLuint index, GLint x, GLint y, GLint z);
extern GLvoid APIENTRY __gllc_VertexAttribI4iEXT(GLuint index, GLint x, GLint y, GLint z, GLint w);

extern GLvoid APIENTRY __gllc_VertexAttribI1uiEXT(GLuint index, GLuint x);
extern GLvoid APIENTRY __gllc_VertexAttribI2uiEXT(GLuint index, GLuint x, GLuint y);
extern GLvoid APIENTRY __gllc_VertexAttribI3uiEXT(GLuint index, GLuint x, GLuint y, GLuint z);
extern GLvoid APIENTRY __gllc_VertexAttribI4uiEXT(GLuint index, GLuint x, GLuint y, GLuint z,
                            GLuint w);

extern GLvoid APIENTRY __gllc_VertexAttribI1ivEXT(GLuint index, const GLint *v);
extern GLvoid APIENTRY __gllc_VertexAttribI2ivEXT(GLuint index, const GLint *v);
extern GLvoid APIENTRY __gllc_VertexAttribI3ivEXT(GLuint index, const GLint *v);
extern GLvoid APIENTRY __gllc_VertexAttribI4ivEXT(GLuint index, const GLint *v);

extern GLvoid APIENTRY __gllc_VertexAttribI1uivEXT(GLuint index, const GLuint *v);
extern GLvoid APIENTRY __gllc_VertexAttribI2uivEXT(GLuint index, const GLuint *v);
extern GLvoid APIENTRY __gllc_VertexAttribI3uivEXT(GLuint index, const GLuint *v);
extern GLvoid APIENTRY __gllc_VertexAttribI4uivEXT(GLuint index, const GLuint *v);

extern GLvoid APIENTRY __gllc_VertexAttribI4bvEXT(GLuint index, const GLbyte *v);
extern GLvoid APIENTRY __gllc_VertexAttribI4svEXT(GLuint index, const GLshort *v);
extern GLvoid APIENTRY __gllc_VertexAttribI4ubvEXT(GLuint index, const GLubyte *v);
extern GLvoid APIENTRY __gllc_VertexAttribI4usvEXT(GLuint index, const GLushort *v);

extern GLvoid APIENTRY __gllc_VertexAttribIPointerEXT(GLuint index, GLint size, GLenum type,
                                GLsizei stride, const GLvoid *pointer);

extern GLvoid APIENTRY __gllc_GetVertexAttribIivEXT(GLuint index, GLenum pname, GLint *params);
extern GLvoid APIENTRY __gllc_GetVertexAttribIuivEXT(GLuint index, GLenum pname, GLuint *params);

extern GLvoid APIENTRY __gllc_Uniform1uiEXT(GLint location, GLuint v0);
extern GLvoid APIENTRY __gllc_Uniform2uiEXT(GLint location, GLuint v0, GLuint v1);
extern GLvoid APIENTRY __gllc_Uniform3uiEXT(GLint location, GLuint v0, GLuint v1, GLuint v2);
extern GLvoid APIENTRY __gllc_Uniform4uiEXT(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);

extern GLvoid APIENTRY __gllc_Uniform1uivEXT(GLint location, GLsizei count, const GLuint *value);
extern GLvoid APIENTRY __gllc_Uniform2uivEXT(GLint location, GLsizei count, const GLuint *value);
extern GLvoid APIENTRY __gllc_Uniform3uivEXT(GLint location, GLsizei count, const GLuint *value);
extern GLvoid APIENTRY __gllc_Uniform4uivEXT(GLint location, GLsizei count, const GLuint *value);

extern GLvoid APIENTRY __gllc_GetUniformuivEXT(GLuint program, GLint location, GLuint *params);

extern GLvoid APIENTRY __gllc_BindFragDataLocationEXT(GLuint program, GLuint colorNumber,
                                const GLbyte *name);
extern GLint APIENTRY __gllc_GetFragDataLocationEXT(GLuint program, const GLbyte *name);
#endif

#if GL_EXT_geometry_shader4
extern GLvoid APIENTRY __gllc_ProgramParameteriEXT(GLuint program, GLenum pname, GLint value);
extern GLvoid APIENTRY __gllc_FramebufferTextureEXT(GLenum target, GLenum attachment, GLuint texture, GLint level);
extern GLvoid APIENTRY __gllc_FramebufferTextureLayerEXT(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
extern GLvoid APIENTRY __gllc_FramebufferTextureFaceEXT(GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face);
#endif

#if GL_EXT_draw_buffers2
extern GLvoid APIENTRY __gllc_ColorMaskIndexedEXT(GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
extern GLvoid APIENTRY __gllc_GetBooleanIndexedvEXT(GLenum value, GLuint index, GLboolean *data);
extern GLvoid APIENTRY __gllc_GetIntegerIndexedvEXT(GLenum value, GLuint index, GLint *data);
extern GLvoid APIENTRY __gllc_EnableIndexedEXT(GLenum target, GLuint index);
extern GLvoid APIENTRY __gllc_DisableIndexedEXT(GLenum target, GLuint index);
extern GLboolean APIENTRY __gllc_IsEnabledIndexedEXT(GLenum target, GLuint index);
#endif

#if GL_EXT_gpu_program_parameters
extern GLvoid APIENTRY __gllc_ProgramEnvParameters4fvEXT(GLenum target, GLuint index, GLsizei count, const GLfloat *params);
extern GLvoid APIENTRY __gllc_ProgramLocalParameters4fvEXT(GLenum target, GLuint index, GLsizei count, const GLfloat *params);
#endif

#if GL_ARB_color_buffer_float
extern GLvoid APIENTRY __gllc_ClampColorARB(GLenum target, GLenum clamp);
#endif

#if GL_ATI_separate_stencil
extern GLvoid APIENTRY __gllc_StencilFuncSeparateATI(GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask);
#endif

#endif /* __g_lcfncs_h_ */
