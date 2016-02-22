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


#ifndef __g_imfncs_h_
#define __g_imfncs_h_

extern GLvoid APIENTRY __glim_NewList( GLuint, GLenum);
extern GLvoid APIENTRY __glim_EndList( GLvoid);
extern GLvoid APIENTRY __glim_CallList( GLuint);
extern GLvoid APIENTRY __glim_CallLists( GLsizei, GLenum, const GLvoid *);
extern GLvoid APIENTRY __glim_DeleteLists( GLuint, GLsizei);
extern GLuint APIENTRY __glim_GenLists( GLsizei);
extern GLvoid APIENTRY __glim_ListBase( GLuint);
extern GLvoid APIENTRY __glim_Begin( GLenum);
extern GLvoid APIENTRY __glim_Bitmap( GLsizei, GLsizei, GLfloat, GLfloat, GLfloat, GLfloat, const GLubyte *);
extern GLvoid APIENTRY __glim_Color3b( GLbyte, GLbyte, GLbyte);
extern GLvoid APIENTRY __glim_Color3bv( const GLbyte *);
extern GLvoid APIENTRY __glim_Color3d( GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_Color3dv( const GLdouble *);
extern GLvoid APIENTRY __glim_Color3f( GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_Color3fv( const GLfloat *);
extern GLvoid APIENTRY __glim_Color3i( GLint, GLint, GLint);
extern GLvoid APIENTRY __glim_Color3iv( const GLint *);
extern GLvoid APIENTRY __glim_Color3s( GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __glim_Color3sv( const GLshort *);
extern GLvoid APIENTRY __glim_Color3ub( GLubyte, GLubyte, GLubyte);
extern GLvoid APIENTRY __glim_Color3ubv( const GLubyte *);
extern GLvoid APIENTRY __glim_Color3ui( GLuint, GLuint, GLuint);
extern GLvoid APIENTRY __glim_Color3uiv( const GLuint *);
extern GLvoid APIENTRY __glim_Color3us( GLushort, GLushort, GLushort);
extern GLvoid APIENTRY __glim_Color3usv( const GLushort *);
extern GLvoid APIENTRY __glim_Color4b( GLbyte, GLbyte, GLbyte, GLbyte);
extern GLvoid APIENTRY __glim_Color4bv( const GLbyte *);
extern GLvoid APIENTRY __glim_Color4d( GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_Color4dv( const GLdouble *);
extern GLvoid APIENTRY __glim_Color4f( GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_Color4fv( const GLfloat *);
extern GLvoid APIENTRY __glim_Color4i( GLint, GLint, GLint, GLint);
extern GLvoid APIENTRY __glim_Color4iv( const GLint *);
extern GLvoid APIENTRY __glim_Color4s( GLshort, GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __glim_Color4sv( const GLshort *);
extern GLvoid APIENTRY __glim_Color4ub( GLubyte, GLubyte, GLubyte, GLubyte);
extern GLvoid APIENTRY __glim_Color4ubv( const GLubyte *);
extern GLvoid APIENTRY __glim_Color4ui( GLuint, GLuint, GLuint, GLuint);
extern GLvoid APIENTRY __glim_Color4uiv( const GLuint *);
extern GLvoid APIENTRY __glim_Color4us( GLushort, GLushort, GLushort, GLushort);
extern GLvoid APIENTRY __glim_Color4usv( const GLushort *);
extern GLvoid APIENTRY __glim_EdgeFlag( GLboolean);
extern GLvoid APIENTRY __glim_EdgeFlagv( const GLboolean *);
extern GLvoid APIENTRY __glim_End( GLvoid);
extern GLvoid APIENTRY __glim_Indexd( GLdouble);
extern GLvoid APIENTRY __glim_Indexdv( const GLdouble *);
extern GLvoid APIENTRY __glim_Indexf( GLfloat);
extern GLvoid APIENTRY __glim_Indexfv( const GLfloat *);
extern GLvoid APIENTRY __glim_Indexi( GLint);
extern GLvoid APIENTRY __glim_Indexiv( const GLint *);
extern GLvoid APIENTRY __glim_Indexs( GLshort);
extern GLvoid APIENTRY __glim_Indexsv( const GLshort *);
extern GLvoid APIENTRY __glim_Normal3b( GLbyte, GLbyte, GLbyte);
extern GLvoid APIENTRY __glim_Normal3bv( const GLbyte *);
extern GLvoid APIENTRY __glim_Normal3d( GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_Normal3dv( const GLdouble *);
extern GLvoid APIENTRY __glim_Normal3f( GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_Normal3fv( const GLfloat *);
extern GLvoid APIENTRY __glim_Normal3i( GLint, GLint, GLint);
extern GLvoid APIENTRY __glim_Normal3iv( const GLint *);
extern GLvoid APIENTRY __glim_Normal3s( GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __glim_Normal3sv( const GLshort *);
extern GLvoid APIENTRY __glim_RasterPos2d( GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_RasterPos2dv( const GLdouble *);
extern GLvoid APIENTRY __glim_RasterPos2f( GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_RasterPos2fv( const GLfloat *);
extern GLvoid APIENTRY __glim_RasterPos2i( GLint, GLint);
extern GLvoid APIENTRY __glim_RasterPos2iv( const GLint *);
extern GLvoid APIENTRY __glim_RasterPos2s( GLshort, GLshort);
extern GLvoid APIENTRY __glim_RasterPos2sv( const GLshort *);
extern GLvoid APIENTRY __glim_RasterPos3d( GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_RasterPos3dv( const GLdouble *);
extern GLvoid APIENTRY __glim_RasterPos3f( GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_RasterPos3fv( const GLfloat *);
extern GLvoid APIENTRY __glim_RasterPos3i( GLint, GLint, GLint);
extern GLvoid APIENTRY __glim_RasterPos3iv( const GLint *);
extern GLvoid APIENTRY __glim_RasterPos3s( GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __glim_RasterPos3sv( const GLshort *);
extern GLvoid APIENTRY __glim_RasterPos4d( GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_RasterPos4dv( const GLdouble *);
extern GLvoid APIENTRY __glim_RasterPos4f( GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_RasterPos4fv( const GLfloat *);
extern GLvoid APIENTRY __glim_RasterPos4i( GLint, GLint, GLint, GLint);
extern GLvoid APIENTRY __glim_RasterPos4iv( const GLint *);
extern GLvoid APIENTRY __glim_RasterPos4s( GLshort, GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __glim_RasterPos4sv( const GLshort *);
extern GLvoid APIENTRY __glim_Rectd( GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_Rectdv( const GLdouble *, const GLdouble *);
extern GLvoid APIENTRY __glim_Rectf( GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_Rectfv( const GLfloat *, const GLfloat *);
extern GLvoid APIENTRY __glim_Recti( GLint, GLint, GLint, GLint);
extern GLvoid APIENTRY __glim_Rectiv( const GLint *, const GLint *);
extern GLvoid APIENTRY __glim_Rects( GLshort, GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __glim_Rectsv( const GLshort *, const GLshort *);
extern GLvoid APIENTRY __glim_TexCoord1d( GLdouble);
extern GLvoid APIENTRY __glim_TexCoord1dv( const GLdouble *);
extern GLvoid APIENTRY __glim_TexCoord1f( GLfloat);
extern GLvoid APIENTRY __glim_TexCoord1fv( const GLfloat *);
extern GLvoid APIENTRY __glim_TexCoord1i( GLint);
extern GLvoid APIENTRY __glim_TexCoord1iv( const GLint *);
extern GLvoid APIENTRY __glim_TexCoord1s( GLshort);
extern GLvoid APIENTRY __glim_TexCoord1sv( const GLshort *);
extern GLvoid APIENTRY __glim_TexCoord2d( GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_TexCoord2dv( const GLdouble *);
extern GLvoid APIENTRY __glim_TexCoord2f( GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_TexCoord2fv( const GLfloat *);
extern GLvoid APIENTRY __glim_TexCoord2i( GLint, GLint);
extern GLvoid APIENTRY __glim_TexCoord2iv( const GLint *);
extern GLvoid APIENTRY __glim_TexCoord2s( GLshort, GLshort);
extern GLvoid APIENTRY __glim_TexCoord2sv( const GLshort *);
extern GLvoid APIENTRY __glim_TexCoord3d( GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_TexCoord3dv( const GLdouble *);
extern GLvoid APIENTRY __glim_TexCoord3f( GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_TexCoord3fv( const GLfloat *);
extern GLvoid APIENTRY __glim_TexCoord3i( GLint, GLint, GLint);
extern GLvoid APIENTRY __glim_TexCoord3iv( const GLint *);
extern GLvoid APIENTRY __glim_TexCoord3s( GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __glim_TexCoord3sv( const GLshort *);
extern GLvoid APIENTRY __glim_TexCoord4d( GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_TexCoord4dv( const GLdouble *);
extern GLvoid APIENTRY __glim_TexCoord4f( GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_TexCoord4fv( const GLfloat *);
extern GLvoid APIENTRY __glim_TexCoord4i( GLint, GLint, GLint, GLint);
extern GLvoid APIENTRY __glim_TexCoord4iv( const GLint *);
extern GLvoid APIENTRY __glim_TexCoord4s( GLshort, GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __glim_TexCoord4sv( const GLshort *);
extern GLvoid APIENTRY __glim_Vertex2d( GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_Vertex2dv( const GLdouble *);
extern GLvoid APIENTRY __glim_Vertex2f( GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_Vertex2fv( const GLfloat *);
extern GLvoid APIENTRY __glim_Vertex2i( GLint, GLint);
extern GLvoid APIENTRY __glim_Vertex2iv( const GLint *);
extern GLvoid APIENTRY __glim_Vertex2s( GLshort, GLshort);
extern GLvoid APIENTRY __glim_Vertex2sv( const GLshort *);
extern GLvoid APIENTRY __glim_Vertex3d( GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_Vertex3dv( const GLdouble *);
extern GLvoid APIENTRY __glim_Vertex3f( GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_Vertex3fv( const GLfloat *);
extern GLvoid APIENTRY __glim_Vertex3i( GLint, GLint, GLint);
extern GLvoid APIENTRY __glim_Vertex3iv( const GLint *);
extern GLvoid APIENTRY __glim_Vertex3s( GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __glim_Vertex3sv( const GLshort *);
extern GLvoid APIENTRY __glim_Vertex4d( GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_Vertex4dv( const GLdouble *);
extern GLvoid APIENTRY __glim_Vertex4f( GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_Vertex4fv( const GLfloat *);
extern GLvoid APIENTRY __glim_Vertex4i( GLint, GLint, GLint, GLint);
extern GLvoid APIENTRY __glim_Vertex4iv( const GLint *);
extern GLvoid APIENTRY __glim_Vertex4s( GLshort, GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __glim_Vertex4sv( const GLshort *);
extern GLvoid APIENTRY __glim_DrawElements( GLenum, GLsizei, GLenum, const GLvoid *);
extern GLvoid APIENTRY __glim_ArrayElement( GLint);
extern GLvoid APIENTRY __glim_DrawArrays( GLenum, GLint, GLsizei);
extern GLvoid APIENTRY __glim_ClipPlane( GLenum, const GLdouble *);
extern GLvoid APIENTRY __glim_ColorMaterial( GLenum, GLenum);
extern GLvoid APIENTRY __glim_CullFace( GLenum);
extern GLvoid APIENTRY __glim_Fogf( GLenum, GLfloat);
extern GLvoid APIENTRY __glim_Fogfv( GLenum, const GLfloat *);
extern GLvoid APIENTRY __glim_Fogi( GLenum, GLint);
extern GLvoid APIENTRY __glim_Fogiv( GLenum, const GLint *);
extern GLvoid APIENTRY __glim_FrontFace( GLenum);
extern GLvoid APIENTRY __glim_Hint( GLenum, GLenum);
extern GLvoid APIENTRY __glim_Lightf( GLenum, GLenum, GLfloat);
extern GLvoid APIENTRY __glim_Lightfv( GLenum, GLenum, const GLfloat *);
extern GLvoid APIENTRY __glim_Lighti( GLenum, GLenum, GLint);
extern GLvoid APIENTRY __glim_Lightiv( GLenum, GLenum, const GLint *);
extern GLvoid APIENTRY __glim_LightModelf( GLenum, GLfloat);
extern GLvoid APIENTRY __glim_LightModelfv( GLenum, const GLfloat *);
extern GLvoid APIENTRY __glim_LightModeli( GLenum, GLint);
extern GLvoid APIENTRY __glim_LightModeliv( GLenum, const GLint *);
extern GLvoid APIENTRY __glim_LineStipple( GLint, GLushort);
extern GLvoid APIENTRY __glim_LineWidth( GLfloat);
extern GLvoid APIENTRY __glim_Materialf( GLenum, GLenum, GLfloat);
extern GLvoid APIENTRY __glim_Materialfv( GLenum, GLenum, const GLfloat *);
extern GLvoid APIENTRY __glim_Materiali( GLenum, GLenum, GLint);
extern GLvoid APIENTRY __glim_Materialiv( GLenum, GLenum, const GLint *);
extern GLvoid APIENTRY __glim_PointSize( GLfloat);
extern GLvoid APIENTRY __glim_PolygonMode( GLenum, GLenum);
extern GLvoid APIENTRY __glim_PolygonStipple( const GLubyte *);
extern GLvoid APIENTRY __glim_Scissor( GLint, GLint, GLsizei, GLsizei);
extern GLvoid APIENTRY __glim_ShadeModel( GLenum);
extern GLvoid APIENTRY __glim_TexParameterf( GLenum, GLenum, GLfloat);
extern GLvoid APIENTRY __glim_TexParameterfv( GLenum, GLenum, const GLfloat *);
extern GLvoid APIENTRY __glim_TexParameteri( GLenum, GLenum, GLint);
extern GLvoid APIENTRY __glim_TexParameteriv( GLenum, GLenum, const GLint *);
extern GLvoid APIENTRY __glim_TexImage1D( GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __glim_TexImage2D( GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __glim_TexEnvf( GLenum, GLenum, GLfloat);
extern GLvoid APIENTRY __glim_TexEnvfv( GLenum, GLenum, const GLfloat *);
extern GLvoid APIENTRY __glim_TexEnvi( GLenum, GLenum, GLint);
extern GLvoid APIENTRY __glim_TexEnviv( GLenum, GLenum, const GLint *);
extern GLvoid APIENTRY __glim_TexGend( GLenum, GLenum, GLdouble);
extern GLvoid APIENTRY __glim_TexGendv( GLenum, GLenum, const GLdouble *);
extern GLvoid APIENTRY __glim_TexGenf( GLenum, GLenum, GLfloat);
extern GLvoid APIENTRY __glim_TexGenfv( GLenum, GLenum, const GLfloat *);
extern GLvoid APIENTRY __glim_TexGeni( GLenum, GLenum, GLint);
extern GLvoid APIENTRY __glim_TexGeniv( GLenum, GLenum, const GLint *);
extern GLvoid APIENTRY __glim_FeedbackBuffer( GLsizei, GLenum, GLfloat *);
extern GLvoid APIENTRY __glim_SelectBuffer( GLsizei, GLuint *);
extern GLint APIENTRY __glim_RenderMode( GLenum);
extern GLvoid APIENTRY __glim_InitNames( GLvoid);
extern GLvoid APIENTRY __glim_LoadName( GLuint);
extern GLvoid APIENTRY __glim_PassThrough( GLfloat);
extern GLvoid APIENTRY __glim_PopName( GLvoid);
extern GLvoid APIENTRY __glim_PushName( GLuint);
extern GLvoid APIENTRY __glim_DrawBuffer( GLenum);
extern GLvoid APIENTRY __glim_Clear( GLbitfield);
extern GLvoid APIENTRY __glim_ClearAccum( GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_ClearIndex( GLfloat);
extern GLvoid APIENTRY __glim_ClearColor( GLclampf, GLclampf, GLclampf, GLclampf);
extern GLvoid APIENTRY __glim_ClearStencil( GLint);
extern GLvoid APIENTRY __glim_ClearDepth( GLclampd);
extern GLvoid APIENTRY __glim_StencilMask( GLuint);
extern GLvoid APIENTRY __glim_ColorMask( GLboolean, GLboolean, GLboolean, GLboolean);
extern GLvoid APIENTRY __glim_DepthMask( GLboolean);
extern GLvoid APIENTRY __glim_IndexMask( GLuint);
extern GLvoid APIENTRY __glim_Accum( GLenum, GLfloat);
extern GLvoid APIENTRY __glim_Disable( GLenum);
extern GLvoid APIENTRY __glim_Enable( GLenum);
extern GLvoid APIENTRY __glim_Finish( GLvoid);
extern GLvoid APIENTRY __glim_Flush( GLvoid);
extern GLvoid APIENTRY __glim_PopAttrib( GLvoid);
extern GLvoid APIENTRY __glim_PushAttrib( GLbitfield);
extern GLvoid APIENTRY __glim_Map1d( GLenum, GLdouble, GLdouble, GLint, GLint, const GLdouble *);
extern GLvoid APIENTRY __glim_Map1f( GLenum, GLfloat, GLfloat, GLint, GLint, const GLfloat *);
extern GLvoid APIENTRY __glim_Map2d( GLenum, GLdouble, GLdouble, GLint, GLint, GLdouble, GLdouble, GLint, GLint, const GLdouble *);
extern GLvoid APIENTRY __glim_Map2f( GLenum, GLfloat, GLfloat, GLint, GLint, GLfloat, GLfloat, GLint, GLint, const GLfloat *);
extern GLvoid APIENTRY __glim_MapGrid1d( GLint, GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_MapGrid1f( GLint, GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_MapGrid2d( GLint, GLdouble, GLdouble, GLint, GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_MapGrid2f( GLint, GLfloat, GLfloat, GLint, GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_EvalCoord1d( GLdouble);
extern GLvoid APIENTRY __glim_EvalCoord1dv( const GLdouble *);
extern GLvoid APIENTRY __glim_EvalCoord1f( GLfloat);
extern GLvoid APIENTRY __glim_EvalCoord1fv( const GLfloat *);
extern GLvoid APIENTRY __glim_EvalCoord2d( GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_EvalCoord2dv( const GLdouble *);
extern GLvoid APIENTRY __glim_EvalCoord2f( GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_EvalCoord2fv( const GLfloat *);
extern GLvoid APIENTRY __glim_EvalMesh1( GLenum, GLint, GLint);
extern GLvoid APIENTRY __glim_EvalPoint1( GLint);
extern GLvoid APIENTRY __glim_EvalMesh2( GLenum, GLint, GLint, GLint, GLint);
extern GLvoid APIENTRY __glim_EvalPoint2( GLint, GLint);
extern GLvoid APIENTRY __glim_AlphaFunc( GLenum, GLclampf);
extern GLvoid APIENTRY __glim_BlendFunc( GLenum, GLenum);
extern GLvoid APIENTRY __glim_LogicOp( GLenum);
extern GLvoid APIENTRY __glim_StencilFunc( GLenum, GLint, GLuint);
extern GLvoid APIENTRY __glim_StencilOp( GLenum, GLenum, GLenum);
extern GLvoid APIENTRY __glim_DepthFunc( GLenum);
extern GLvoid APIENTRY __glim_PixelZoom( GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_PixelTransferf( GLenum, GLfloat);
extern GLvoid APIENTRY __glim_PixelTransferi( GLenum, GLint);
extern GLvoid APIENTRY __glim_PixelStoref( GLenum, GLfloat);
extern GLvoid APIENTRY __glim_PixelStorei( GLenum, GLint);
extern GLvoid APIENTRY __glim_PixelMapfv( GLenum, GLint, const GLfloat *);
extern GLvoid APIENTRY __glim_PixelMapuiv( GLenum, GLint, const GLuint *);
extern GLvoid APIENTRY __glim_PixelMapusv( GLenum, GLint, const GLushort *);
extern GLvoid APIENTRY __glim_ReadBuffer( GLenum);
extern GLvoid APIENTRY __glim_CopyPixels( GLint, GLint, GLsizei, GLsizei, GLenum);
extern GLvoid APIENTRY __glim_ReadPixels( GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid *);
extern GLvoid APIENTRY __glim_DrawPixels( GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __glim_GetBooleanv( GLenum, GLboolean *);
extern GLvoid APIENTRY __glim_GetClipPlane( GLenum, GLdouble *);
extern GLvoid APIENTRY __glim_GetDoublev( GLenum, GLdouble *);
extern GLenum APIENTRY __glim_GetError( GLvoid);
extern GLvoid APIENTRY __glim_GetFloatv( GLenum, GLfloat *);
extern GLvoid APIENTRY __glim_GetIntegerv( GLenum, GLint *);
extern GLvoid APIENTRY __glim_GetLightfv( GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __glim_GetLightiv( GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __glim_GetMapdv( GLenum, GLenum, GLdouble *);
extern GLvoid APIENTRY __glim_GetMapfv( GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __glim_GetMapiv( GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __glim_GetMaterialfv( GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __glim_GetMaterialiv( GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __glim_GetPixelMapfv( GLenum, GLfloat *);
extern GLvoid APIENTRY __glim_GetPixelMapuiv( GLenum, GLuint *);
extern GLvoid APIENTRY __glim_GetPixelMapusv( GLenum, GLushort *);
extern GLvoid APIENTRY __glim_GetPolygonStipple( GLubyte *);
extern const GLubyte * APIENTRY __glim_GetString( GLenum);
extern GLvoid APIENTRY __glim_GetTexEnvfv( GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __glim_GetTexEnviv( GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __glim_GetTexGendv( GLenum, GLenum, GLdouble *);
extern GLvoid APIENTRY __glim_GetTexGenfv( GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __glim_GetTexGeniv( GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __glim_GetTexImage( GLenum, GLint, GLenum, GLenum, GLvoid *);
extern GLvoid APIENTRY __glim_GetTexParameterfv( GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __glim_GetTexParameteriv( GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __glim_GetTexLevelParameterfv( GLenum, GLint, GLenum, GLfloat *);
extern GLvoid APIENTRY __glim_GetTexLevelParameteriv( GLenum, GLint, GLenum, GLint *);
extern GLboolean APIENTRY __glim_IsEnabled( GLenum);
extern GLboolean APIENTRY __glim_IsList( GLuint);
extern GLvoid APIENTRY __glim_DepthRange( GLclampd, GLclampd);
extern GLvoid APIENTRY __glim_Frustum( GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_LoadIdentity( GLvoid);
extern GLvoid APIENTRY __glim_LoadMatrixf( const GLfloat *);
extern GLvoid APIENTRY __glim_LoadMatrixd( const GLdouble *);
extern GLvoid APIENTRY __glim_MatrixMode( GLenum);
extern GLvoid APIENTRY __glim_MultMatrixf( const GLfloat *);
extern GLvoid APIENTRY __glim_MultMatrixd( const GLdouble *);
extern GLvoid APIENTRY __glim_Ortho( GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_PopMatrix( GLvoid);
extern GLvoid APIENTRY __glim_PushMatrix( GLvoid);
extern GLvoid APIENTRY __glim_Rotated( GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_Rotatef( GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_Scaled( GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_Scalef( GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_Translated( GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_Translatef( GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_Viewport( GLint, GLint, GLsizei, GLsizei);
extern GLvoid APIENTRY __glim_ColorSubTable( GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __glim_ColorTable( GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __glim_CopyColorTable( GLenum, GLenum, GLint, GLint, GLsizei);
extern GLvoid APIENTRY __glim_GetColorTable( GLenum, GLenum, GLenum, GLvoid *);
extern GLvoid APIENTRY __glim_GetColorTableParameterfv( GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __glim_GetColorTableParameteriv( GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __glim_ColorPointer( GLint, GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __glim_DisableClientState( GLenum);
extern GLvoid APIENTRY __glim_EdgeFlagPointer( GLsizei, const GLboolean *);
extern GLvoid APIENTRY __glim_EnableClientState( GLenum);
extern GLvoid APIENTRY __glim_GetPointerv( GLenum, GLvoid* *);
extern GLvoid APIENTRY __glim_IndexPointer( GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __glim_InterleavedArrays( GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __glim_NormalPointer( GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __glim_TexCoordPointer( GLint, GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __glim_VertexPointer( GLint, GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __glim_PolygonOffset( GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_CopyTexImage1D( GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLint);
extern GLvoid APIENTRY __glim_CopyTexImage2D( GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint);
extern GLvoid APIENTRY __glim_CopyTexSubImage1D( GLenum, GLint, GLint, GLint, GLint, GLsizei);
extern GLvoid APIENTRY __glim_CopyTexSubImage2D( GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
extern GLvoid APIENTRY __glim_TexSubImage1D( GLenum, GLint, GLint, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __glim_TexSubImage2D( GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLboolean APIENTRY __glim_AreTexturesResident( GLsizei, const GLuint *, GLboolean *);
extern GLvoid APIENTRY __glim_BindTexture( GLenum, GLuint);
extern GLvoid APIENTRY __glim_DeleteTextures( GLsizei, const GLuint *);
extern GLvoid APIENTRY __glim_GenTextures( GLsizei, GLuint *);
extern GLboolean APIENTRY __glim_IsTexture( GLuint);
extern GLvoid APIENTRY __glim_PrioritizeTextures( GLsizei, const GLuint *, const GLclampf *);
extern GLvoid APIENTRY __glim_Indexub( GLubyte);
extern GLvoid APIENTRY __glim_Indexubv( const GLubyte *);
extern GLvoid APIENTRY __glim_PopClientAttrib( GLvoid);
extern GLvoid APIENTRY __glim_PushClientAttrib( GLbitfield);

#if GL_VERSION_1_2
extern GLvoid APIENTRY __glim_BlendColor( GLclampf, GLclampf, GLclampf, GLclampf);
extern GLvoid APIENTRY __glim_BlendEquation( GLenum);
extern GLvoid APIENTRY __glim_DrawRangeElements( GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid *);
extern GLvoid APIENTRY __glim_ColorTable( GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __glim_ColorTableParameterfv( GLenum, GLenum, const GLfloat *);
extern GLvoid APIENTRY __glim_ColorTableParameteriv( GLenum, GLenum, const GLint *);
extern GLvoid APIENTRY __glim_CopyColorTable( GLenum, GLenum, GLint, GLint, GLsizei);
extern GLvoid APIENTRY __glim_GetColorTable( GLenum, GLenum, GLenum, GLvoid *);
extern GLvoid APIENTRY __glim_GetColorTableParameterfv( GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __glim_GetColorTableParameteriv( GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __glim_ColorSubTable( GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __glim_CopyColorSubTable( GLenum, GLsizei, GLint, GLint, GLsizei);
extern GLvoid APIENTRY __glim_ConvolutionFilter1D( GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __glim_ConvolutionFilter2D( GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __glim_ConvolutionParameterf( GLenum, GLenum, GLfloat);
extern GLvoid APIENTRY __glim_ConvolutionParameterfv( GLenum, GLenum, const GLfloat *);
extern GLvoid APIENTRY __glim_ConvolutionParameteri( GLenum, GLenum, GLint);
extern GLvoid APIENTRY __glim_ConvolutionParameteriv( GLenum, GLenum, const GLint *);
extern GLvoid APIENTRY __glim_CopyConvolutionFilter1D( GLenum, GLenum, GLint, GLint, GLsizei);
extern GLvoid APIENTRY __glim_CopyConvolutionFilter2D( GLenum, GLenum, GLint, GLint, GLsizei, GLsizei);
extern GLvoid APIENTRY __glim_GetConvolutionFilter( GLenum, GLenum, GLenum, GLvoid *);
extern GLvoid APIENTRY __glim_GetConvolutionParameterfv( GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __glim_GetConvolutionParameteriv( GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __glim_GetSeparableFilter( GLenum, GLenum, GLenum, GLvoid *, GLvoid *, GLvoid *);
extern GLvoid APIENTRY __glim_SeparableFilter2D( GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *, const GLvoid *);
extern GLvoid APIENTRY __glim_GetHistogram( GLenum, GLboolean, GLenum, GLenum, GLvoid *);
extern GLvoid APIENTRY __glim_GetHistogramParameterfv( GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __glim_GetHistogramParameteriv( GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __glim_GetMinmax( GLenum, GLboolean, GLenum, GLenum, GLvoid *);
extern GLvoid APIENTRY __glim_GetMinmaxParameterfv( GLenum, GLenum, GLfloat *);
extern GLvoid APIENTRY __glim_GetMinmaxParameteriv( GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __glim_Histogram( GLenum, GLsizei, GLenum, GLboolean);
extern GLvoid APIENTRY __glim_Minmax( GLenum, GLenum, GLboolean);
extern GLvoid APIENTRY __glim_ResetHistogram( GLenum);
extern GLvoid APIENTRY __glim_ResetMinmax( GLenum);
extern GLvoid APIENTRY __glim_TexImage3D( GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __glim_TexSubImage3D( GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
extern GLvoid APIENTRY __glim_CopyTexSubImage3D( GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
#endif
#if GL_VERSION_1_3
extern GLvoid APIENTRY __glim_ActiveTexture( GLenum);
extern GLvoid APIENTRY __glim_ClientActiveTexture( GLenum);
extern GLvoid APIENTRY __glim_MultiTexCoord1d( GLenum, GLdouble);
extern GLvoid APIENTRY __glim_MultiTexCoord1dv( GLenum, const GLdouble *);
extern GLvoid APIENTRY __glim_MultiTexCoord1f( GLenum, GLfloat);
extern GLvoid APIENTRY __glim_MultiTexCoord1fv( GLenum, const GLfloat *);
extern GLvoid APIENTRY __glim_MultiTexCoord1i( GLenum, GLint);
extern GLvoid APIENTRY __glim_MultiTexCoord1iv( GLenum, const GLint *);
extern GLvoid APIENTRY __glim_MultiTexCoord1s( GLenum, GLshort);
extern GLvoid APIENTRY __glim_MultiTexCoord1sv( GLenum, const GLshort *);
extern GLvoid APIENTRY __glim_MultiTexCoord2d( GLenum, GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_MultiTexCoord2dv( GLenum, const GLdouble *);
extern GLvoid APIENTRY __glim_MultiTexCoord2f( GLenum, GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_MultiTexCoord2fv( GLenum, const GLfloat *);
extern GLvoid APIENTRY __glim_MultiTexCoord2i( GLenum, GLint, GLint);
extern GLvoid APIENTRY __glim_MultiTexCoord2iv( GLenum, const GLint *);
extern GLvoid APIENTRY __glim_MultiTexCoord2s( GLenum, GLshort, GLshort);
extern GLvoid APIENTRY __glim_MultiTexCoord2sv( GLenum, const GLshort *);
extern GLvoid APIENTRY __glim_MultiTexCoord3d( GLenum, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_MultiTexCoord3dv( GLenum, const GLdouble *);
extern GLvoid APIENTRY __glim_MultiTexCoord3f( GLenum, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_MultiTexCoord3fv( GLenum, const GLfloat *);
extern GLvoid APIENTRY __glim_MultiTexCoord3i( GLenum, GLint, GLint, GLint);
extern GLvoid APIENTRY __glim_MultiTexCoord3iv( GLenum, const GLint *);
extern GLvoid APIENTRY __glim_MultiTexCoord3s( GLenum, GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __glim_MultiTexCoord3sv( GLenum, const GLshort *);
extern GLvoid APIENTRY __glim_MultiTexCoord4d( GLenum, GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_MultiTexCoord4dv( GLenum, const GLdouble *);
extern GLvoid APIENTRY __glim_MultiTexCoord4f( GLenum, GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_MultiTexCoord4fv( GLenum, const GLfloat *);
extern GLvoid APIENTRY __glim_MultiTexCoord4i( GLenum, GLint, GLint, GLint, GLint);
extern GLvoid APIENTRY __glim_MultiTexCoord4iv( GLenum, const GLint *);
extern GLvoid APIENTRY __glim_MultiTexCoord4s( GLenum, GLshort, GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __glim_MultiTexCoord4sv( GLenum, const GLshort *);
extern GLvoid APIENTRY __glim_LoadTransposeMatrixf( const GLfloat *);
extern GLvoid APIENTRY __glim_LoadTransposeMatrixd( const GLdouble *);
extern GLvoid APIENTRY __glim_MultTransposeMatrixf( const GLfloat *);
extern GLvoid APIENTRY __glim_MultTransposeMatrixd( const GLdouble *);
extern GLvoid APIENTRY __glim_SampleCoverage( GLclampf, GLboolean);
extern GLvoid APIENTRY __glim_CompressedTexImage3D( GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __glim_CompressedTexImage2D( GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __glim_CompressedTexImage1D( GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __glim_CompressedTexSubImage3D( GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __glim_CompressedTexSubImage2D( GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __glim_CompressedTexSubImage1D( GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __glim_GetCompressedTexImage( GLenum, GLint, GLvoid *);
#endif
#if GL_VERSION_1_4
extern GLvoid APIENTRY __glim_BlendFuncSeparate( GLenum, GLenum, GLenum, GLenum);
extern GLvoid APIENTRY __glim_FogCoordf( GLfloat);
extern GLvoid APIENTRY __glim_FogCoordfv( const GLfloat *);
extern GLvoid APIENTRY __glim_FogCoordd( GLdouble);
extern GLvoid APIENTRY __glim_FogCoorddv( const GLdouble *);
extern GLvoid APIENTRY __glim_FogCoordPointer( GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __glim_MultiDrawArrays( GLenum, GLint *, GLsizei *, GLsizei);
extern GLvoid APIENTRY __glim_MultiDrawElements( GLenum, const GLsizei *, GLenum, const GLvoid* *, GLsizei);
extern GLvoid APIENTRY __glim_PointParameterf( GLenum, GLfloat);
extern GLvoid APIENTRY __glim_PointParameterfv( GLenum, const GLfloat *);
extern GLvoid APIENTRY __glim_PointParameteri( GLenum, GLint);
extern GLvoid APIENTRY __glim_PointParameteriv( GLenum, const GLint *);
extern GLvoid APIENTRY __glim_SecondaryColor3b( GLbyte, GLbyte, GLbyte);
extern GLvoid APIENTRY __glim_SecondaryColor3bv( const GLbyte *);
extern GLvoid APIENTRY __glim_SecondaryColor3d( GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_SecondaryColor3dv( const GLdouble *);
extern GLvoid APIENTRY __glim_SecondaryColor3f( GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_SecondaryColor3fv( const GLfloat *);
extern GLvoid APIENTRY __glim_SecondaryColor3i( GLint, GLint, GLint);
extern GLvoid APIENTRY __glim_SecondaryColor3iv( const GLint *);
extern GLvoid APIENTRY __glim_SecondaryColor3s( GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __glim_SecondaryColor3sv( const GLshort *);
extern GLvoid APIENTRY __glim_SecondaryColor3ub( GLubyte, GLubyte, GLubyte);
extern GLvoid APIENTRY __glim_SecondaryColor3ubv( const GLubyte *);
extern GLvoid APIENTRY __glim_SecondaryColor3ui( GLuint, GLuint, GLuint);
extern GLvoid APIENTRY __glim_SecondaryColor3uiv( const GLuint *);
extern GLvoid APIENTRY __glim_SecondaryColor3us( GLushort, GLushort, GLushort);
extern GLvoid APIENTRY __glim_SecondaryColor3usv( const GLushort *);
extern GLvoid APIENTRY __glim_SecondaryColorPointer( GLint, GLenum, GLsizei, const GLvoid *);
extern GLvoid APIENTRY __glim_WindowPos2d( GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_WindowPos2dv( const GLdouble *);
extern GLvoid APIENTRY __glim_WindowPos2f( GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_WindowPos2fv( const GLfloat *);
extern GLvoid APIENTRY __glim_WindowPos2i( GLint, GLint);
extern GLvoid APIENTRY __glim_WindowPos2iv( const GLint *);
extern GLvoid APIENTRY __glim_WindowPos2s( GLshort, GLshort);
extern GLvoid APIENTRY __glim_WindowPos2sv( const GLshort *);
extern GLvoid APIENTRY __glim_WindowPos3d( GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_WindowPos3dv( const GLdouble *);
extern GLvoid APIENTRY __glim_WindowPos3f( GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_WindowPos3fv( const GLfloat *);
extern GLvoid APIENTRY __glim_WindowPos3i( GLint, GLint, GLint);
extern GLvoid APIENTRY __glim_WindowPos3iv( const GLint *);
extern GLvoid APIENTRY __glim_WindowPos3s( GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __glim_WindowPos3sv( const GLshort *);
#endif
#if GL_VERSION_1_5
extern GLvoid APIENTRY __glim_GenQueries( GLsizei, GLuint *);
extern GLvoid APIENTRY __glim_DeleteQueries( GLsizei, const GLuint *);
extern GLboolean APIENTRY __glim_IsQuery( GLuint);
extern GLvoid APIENTRY __glim_BeginQuery( GLenum, GLuint);
extern GLvoid APIENTRY __glim_EndQuery( GLenum);
extern GLvoid APIENTRY __glim_GetQueryiv( GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __glim_GetQueryObjectiv( GLuint, GLenum, GLint *);
extern GLvoid APIENTRY __glim_GetQueryObjectuiv( GLuint, GLenum, GLuint *);
extern GLvoid APIENTRY __glim_BindBuffer( GLenum, GLuint);
extern GLvoid APIENTRY __glim_DeleteBuffers( GLsizei, const GLuint *);
extern GLvoid APIENTRY __glim_GenBuffers( GLsizei, GLuint *);
extern GLboolean APIENTRY __glim_IsBuffer( GLuint);
extern GLvoid APIENTRY __glim_BufferData( GLenum, GLsizeiptr, const GLvoid *, GLenum);
extern GLvoid APIENTRY __glim_BufferSubData( GLenum, GLintptr, GLsizeiptr, const GLvoid *);
extern GLvoid APIENTRY __glim_GetBufferSubData( GLenum, GLintptr, GLsizeiptr, GLvoid *);
extern GLvoid* APIENTRY __glim_MapBuffer( GLenum, GLenum);
extern GLboolean APIENTRY __glim_UnmapBuffer( GLenum);
extern GLvoid APIENTRY __glim_GetBufferParameteriv( GLenum, GLenum, GLint *);
extern GLvoid APIENTRY __glim_GetBufferPointerv( GLenum, GLenum, GLvoid* *);
#endif
#if GL_VERSION_2_0
extern GLvoid APIENTRY __glim_BlendEquationSeparate( GLenum, GLenum);
extern GLvoid APIENTRY __glim_DrawBuffers( GLsizei, const GLenum *);
extern GLvoid APIENTRY __glim_StencilOpSeparate( GLenum, GLenum, GLenum, GLenum);
extern GLvoid APIENTRY __glim_StencilFuncSeparate( GLenum, GLenum, GLint, GLuint);
extern GLvoid APIENTRY __glim_StencilMaskSeparate( GLenum, GLuint);
extern GLvoid APIENTRY __glim_AttachShader( GLuint, GLuint);
extern GLvoid APIENTRY __glim_BindAttribLocation( GLuint, GLuint, const GLchar *);
extern GLvoid APIENTRY __glim_CompileShader( GLuint);
extern GLuint APIENTRY __glim_CreateProgram (GLvoid);
extern GLuint APIENTRY __glim_CreateShader( GLenum);
extern GLvoid APIENTRY __glim_DeleteProgram( GLuint);
extern GLvoid APIENTRY __glim_DeleteShader( GLuint);
extern GLvoid APIENTRY __glim_DetachShader( GLuint, GLuint);
extern GLvoid APIENTRY __glim_DisableVertexAttribArray( GLuint);
extern GLvoid APIENTRY __glim_EnableVertexAttribArray( GLuint);
extern GLvoid APIENTRY __glim_GetActiveAttrib( GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *);
extern GLvoid APIENTRY __glim_GetActiveUniform( GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *);
extern GLvoid APIENTRY __glim_GetAttachedShaders( GLuint, GLsizei, GLsizei *, GLuint *);
extern GLint APIENTRY __glim_GetAttribLocation( GLuint, const GLchar *);
extern GLvoid APIENTRY __glim_GetProgramiv( GLuint, GLenum, GLint *);
extern GLvoid APIENTRY __glim_GetProgramInfoLog( GLuint, GLsizei, GLsizei *, GLchar *);
extern GLvoid APIENTRY __glim_GetShaderiv( GLuint, GLenum, GLint *);
extern GLvoid APIENTRY __glim_GetShaderInfoLog( GLuint, GLsizei, GLsizei *, GLchar *);
extern GLvoid APIENTRY __glim_GetShaderSource( GLuint, GLsizei, GLsizei *, GLchar *);
extern GLint APIENTRY __glim_GetUniformLocation( GLuint, const GLchar *);
extern GLvoid APIENTRY __glim_GetUniformfv( GLuint, GLint, GLfloat *);
extern GLvoid APIENTRY __glim_GetUniformiv( GLuint, GLint, GLint *);
extern GLvoid APIENTRY __glim_GetVertexAttribdv( GLuint, GLenum, GLdouble *);
extern GLvoid APIENTRY __glim_GetVertexAttribfv( GLuint, GLenum, GLfloat *);
extern GLvoid APIENTRY __glim_GetVertexAttribiv( GLuint, GLenum, GLint *);
extern GLvoid APIENTRY __glim_GetVertexAttribPointerv( GLuint, GLenum, GLvoid* *);
extern GLboolean APIENTRY __glim_IsProgram( GLuint);
extern GLboolean APIENTRY __glim_IsShader( GLuint);
extern GLvoid APIENTRY __glim_LinkProgram( GLuint);
extern GLvoid APIENTRY __glim_ShaderSource( GLuint, GLsizei, const GLchar* *, const GLint *);
extern GLvoid APIENTRY __glim_UseProgram( GLuint);
extern GLvoid APIENTRY __glim_Uniform1f( GLint, GLfloat);
extern GLvoid APIENTRY __glim_Uniform2f( GLint, GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_Uniform3f( GLint, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_Uniform4f( GLint, GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_Uniform1i( GLint, GLint);
extern GLvoid APIENTRY __glim_Uniform2i( GLint, GLint, GLint);
extern GLvoid APIENTRY __glim_Uniform3i( GLint, GLint, GLint, GLint);
extern GLvoid APIENTRY __glim_Uniform4i( GLint, GLint, GLint, GLint, GLint);
extern GLvoid APIENTRY __glim_Uniform1fv( GLint, GLsizei, const GLfloat *);
extern GLvoid APIENTRY __glim_Uniform2fv( GLint, GLsizei, const GLfloat *);
extern GLvoid APIENTRY __glim_Uniform3fv( GLint, GLsizei, const GLfloat *);
extern GLvoid APIENTRY __glim_Uniform4fv( GLint, GLsizei, const GLfloat *);
extern GLvoid APIENTRY __glim_Uniform1iv( GLint, GLsizei, const GLint *);
extern GLvoid APIENTRY __glim_Uniform2iv( GLint, GLsizei, const GLint *);
extern GLvoid APIENTRY __glim_Uniform3iv( GLint, GLsizei, const GLint *);
extern GLvoid APIENTRY __glim_Uniform4iv( GLint, GLsizei, const GLint *);
extern GLvoid APIENTRY __glim_UniformMatrix2fv( GLint, GLsizei, GLboolean, const GLfloat *);
extern GLvoid APIENTRY __glim_UniformMatrix3fv( GLint, GLsizei, GLboolean, const GLfloat *);
extern GLvoid APIENTRY __glim_UniformMatrix4fv( GLint, GLsizei, GLboolean, const GLfloat *);
extern GLvoid APIENTRY __glim_ValidateProgram( GLuint);
extern GLvoid APIENTRY __glim_VertexAttrib1d( GLuint, GLdouble);
extern GLvoid APIENTRY __glim_VertexAttrib1dv( GLuint, const GLdouble *);
extern GLvoid APIENTRY __glim_VertexAttrib1f( GLuint, GLfloat);
extern GLvoid APIENTRY __glim_VertexAttrib1fv( GLuint, const GLfloat *);
extern GLvoid APIENTRY __glim_VertexAttrib1s( GLuint, GLshort);
extern GLvoid APIENTRY __glim_VertexAttrib1sv( GLuint, const GLshort *);
extern GLvoid APIENTRY __glim_VertexAttrib2d( GLuint, GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_VertexAttrib2dv( GLuint, const GLdouble *);
extern GLvoid APIENTRY __glim_VertexAttrib2f( GLuint, GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_VertexAttrib2fv( GLuint, const GLfloat *);
extern GLvoid APIENTRY __glim_VertexAttrib2s( GLuint, GLshort, GLshort);
extern GLvoid APIENTRY __glim_VertexAttrib2sv( GLuint, const GLshort *);
extern GLvoid APIENTRY __glim_VertexAttrib3d( GLuint, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_VertexAttrib3dv( GLuint, const GLdouble *);
extern GLvoid APIENTRY __glim_VertexAttrib3f( GLuint, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_VertexAttrib3fv( GLuint, const GLfloat *);
extern GLvoid APIENTRY __glim_VertexAttrib3s( GLuint, GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __glim_VertexAttrib3sv( GLuint, const GLshort *);
extern GLvoid APIENTRY __glim_VertexAttrib4Nbv( GLuint, const GLbyte *);
extern GLvoid APIENTRY __glim_VertexAttrib4Niv( GLuint, const GLint *);
extern GLvoid APIENTRY __glim_VertexAttrib4Nsv( GLuint, const GLshort *);
extern GLvoid APIENTRY __glim_VertexAttrib4Nub( GLuint, GLubyte, GLubyte, GLubyte, GLubyte);
extern GLvoid APIENTRY __glim_VertexAttrib4Nubv( GLuint, const GLubyte *);
extern GLvoid APIENTRY __glim_VertexAttrib4Nuiv( GLuint, const GLuint *);
extern GLvoid APIENTRY __glim_VertexAttrib4Nusv( GLuint, const GLushort *);
extern GLvoid APIENTRY __glim_VertexAttrib4bv( GLuint, const GLbyte *);
extern GLvoid APIENTRY __glim_VertexAttrib4d( GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
extern GLvoid APIENTRY __glim_VertexAttrib4dv( GLuint, const GLdouble *);
extern GLvoid APIENTRY __glim_VertexAttrib4f( GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
extern GLvoid APIENTRY __glim_VertexAttrib4fv( GLuint, const GLfloat *);
extern GLvoid APIENTRY __glim_VertexAttrib4iv( GLuint, const GLint *);
extern GLvoid APIENTRY __glim_VertexAttrib4s( GLuint, GLshort, GLshort, GLshort, GLshort);
extern GLvoid APIENTRY __glim_VertexAttrib4sv( GLuint, const GLshort *);
extern GLvoid APIENTRY __glim_VertexAttrib4ubv( GLuint, const GLubyte *);
extern GLvoid APIENTRY __glim_VertexAttrib4uiv( GLuint, const GLuint *);
extern GLvoid APIENTRY __glim_VertexAttrib4usv( GLuint, const GLushort *);
extern GLvoid APIENTRY __glim_VertexAttribPointer( GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *);
#endif

#if GL_VERSION_2_1
extern GLvoid APIENTRY __glim_UniformMatrix2x3fv( GLint, GLsizei, GLboolean, const GLfloat *);
extern GLvoid APIENTRY __glim_UniformMatrix2x4fv( GLint, GLsizei, GLboolean, const GLfloat *);
extern GLvoid APIENTRY __glim_UniformMatrix3x2fv( GLint, GLsizei, GLboolean, const GLfloat *);
extern GLvoid APIENTRY __glim_UniformMatrix3x4fv( GLint, GLsizei, GLboolean, const GLfloat *);
extern GLvoid APIENTRY __glim_UniformMatrix4x2fv( GLint, GLsizei, GLboolean, const GLfloat *);
extern GLvoid APIENTRY __glim_UniformMatrix4x3fv( GLint, GLsizei, GLboolean, const GLfloat *);
#endif


#if GL_ARB_vertex_program
extern GLvoid APIENTRY __glim_ProgramStringARB( GLenum target, GLenum format, GLsizei len, const GLvoid *string);
extern GLvoid APIENTRY __glim_BindProgramARB( GLenum target, GLuint program);
extern GLvoid APIENTRY __glim_DeleteProgramsARB( GLsizei n, const GLuint *programs);
extern GLvoid APIENTRY __glim_GenProgramsARB( GLsizei n, GLuint *programs);
extern GLvoid APIENTRY __glim_ProgramEnvParameter4dARB( GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern GLvoid APIENTRY __glim_ProgramEnvParameter4dvARB( GLenum target, GLuint index, const GLdouble *params);
extern GLvoid APIENTRY __glim_ProgramEnvParameter4fARB( GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern GLvoid APIENTRY __glim_ProgramEnvParameter4fvARB( GLenum target, GLuint index, const GLfloat *params);
extern GLvoid APIENTRY __glim_ProgramLocalParameter4dARB( GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern GLvoid APIENTRY __glim_ProgramLocalParameter4dvARB( GLenum target, GLuint index, const GLdouble *params);
extern GLvoid APIENTRY __glim_ProgramLocalParameter4fARB( GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern GLvoid APIENTRY __glim_ProgramLocalParameter4fvARB( GLenum target, GLuint index, const GLfloat *params);
extern GLvoid APIENTRY __glim_GetProgramEnvParameterdvARB( GLenum target, GLuint index, GLdouble *params);
extern GLvoid APIENTRY __glim_GetProgramEnvParameterfvARB( GLenum target, GLuint index, GLfloat *params);
extern GLvoid APIENTRY __glim_GetProgramLocalParameterdvARB( GLenum target, GLuint index, GLdouble *params);
extern GLvoid APIENTRY __glim_GetProgramLocalParameterfvARB( GLenum target, GLuint index, GLfloat *params);
extern GLvoid APIENTRY __glim_GetProgramivARB( GLenum target, GLenum pname, GLint *params);
extern GLvoid APIENTRY __glim_GetProgramStringARB( GLenum target, GLenum pname, GLvoid *string);
extern GLboolean APIENTRY __glim_IsProgramARB( GLuint program);
#endif

#if GL_ARB_shader_objects
extern GLvoid APIENTRY __glim_DeleteObjectARB(GLhandleARB obj);
extern GLhandleARB APIENTRY __glim_GetHandleARB(GLenum pname);
extern GLvoid APIENTRY __glim_GetInfoLogARB(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
extern GLvoid APIENTRY __glim_GetObjectParameterfvARB(GLhandleARB obj, GLenum pname, GLfloat *params);
extern GLvoid APIENTRY __glim_GetObjectParameterivARB(GLhandleARB obj, GLenum pname, GLint *params);
#endif

#if GL_ATI_vertex_array_object
extern GLuint APIENTRY __glim_NewObjectBufferATI(GLsizei size, const GLvoid *pointer, GLenum usage);
extern GLboolean APIENTRY __glim_IsObjectBufferATI(GLuint buffer);
extern GLvoid APIENTRY __glim_UpdateObjectBufferATI(GLuint buffer, GLuint offset,
                                        GLsizei size, const GLvoid *pointer,
                                        GLenum preserve);
extern GLvoid APIENTRY __glim_GetObjectBufferfvATI(GLuint buffer, GLenum pname, GLfloat *params);
extern GLvoid APIENTRY  __glim_GetObjectBufferivATI(GLuint buffer, GLenum pname, GLint *params);
extern GLvoid APIENTRY  __glim_FreeObjectBufferATI(GLuint buffer);
extern GLvoid APIENTRY __glim_ArrayObjectATI(GLenum array, GLint size, GLenum type, GLsizei stride, GLuint buffer,
                                       GLuint offset);
extern GLvoid APIENTRY __glim_GetArrayObjectfvATI(GLenum array, GLenum pname, GLfloat * params);
extern GLvoid APIENTRY __glim_GetArrayObjectivATI(GLenum array, GLenum pname, GLint * params);
extern GLvoid APIENTRY __glim_VariantArrayObjectATI (GLuint id, GLenum type, GLsizei stride, GLuint buffer, GLuint offset);
extern GLvoid APIENTRY __glim_GetVariantArrayObjectfvATI (GLuint id, GLenum pname, GLfloat *params);
extern GLvoid APIENTRY __glim_GetVariantArrayObjectivATI (GLuint id, GLenum pname, GLint *params);
#endif
#if GL_ATI_vertex_attrib_array_object
extern GLvoid APIENTRY __glim_VertexAttribArrayObjectATI (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLuint buffer, GLuint offset);
extern GLvoid APIENTRY __glim_GetVertexAttribArrayObjectfvATI (GLuint index, GLenum pname, GLfloat *params);
extern GLvoid APIENTRY __glim_GetVertexAttribArrayObjectivATI (GLuint index, GLenum pname, GLint *params);
#endif
#if GL_ATI_element_array
extern GLvoid  APIENTRY __glim_ElementPointerATI(GLenum type, const GLvoid *pointer);
extern GLvoid  APIENTRY __glim_DrawElementArrayATI(GLenum mode, GLsizei count);
extern GLvoid  APIENTRY __glim_DrawRangeElementArrayATI(GLenum mode, GLuint start,
                                  GLuint end, GLsizei count);
#endif

#if GL_EXT_stencil_two_side
extern GLvoid APIENTRY __glim_ActiveStencilFaceEXT(GLenum face);
#endif

extern GLvoid APIENTRY __glim_AddSwapHintRectWIN(GLint x, GLint y, GLsizei width,
                                        GLsizei height);

#if GL_EXT_depth_bounds_test
extern GLvoid APIENTRY __glim_DepthBoundsEXT(GLclampd zMin, GLclampd zMax);
#endif

#if GL_EXT_framebuffer_object
extern GLboolean APIENTRY __glim_IsRenderbufferEXT(GLuint renderbuffer);
extern GLvoid APIENTRY __glim_BindRenderbufferEXT(GLenum target, GLuint renderbuffer);
extern GLvoid APIENTRY __glim_DeleteRenderbuffersEXT(GLsizei n, const GLuint *renderbuffers);
extern GLvoid APIENTRY __glim_GenRenderbuffersEXT(GLsizei n, GLuint *renderbuffers);
extern GLvoid APIENTRY __glim_RenderbufferStorageEXT(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
extern GLvoid APIENTRY __glim_GetRenderbufferParameterivEXT(GLenum target, GLenum pname, GLint* params);
extern GLboolean APIENTRY __glim_IsFramebufferEXT(GLuint framebuffer);
extern GLvoid APIENTRY __glim_BindFramebufferEXT(GLenum target, GLuint framebuffer);
extern GLvoid APIENTRY __glim_DeleteFramebuffersEXT(GLsizei n, const GLuint *framebuffers);
extern GLvoid APIENTRY __glim_GenFramebuffersEXT(GLsizei n, GLuint *framebuffers);
extern GLenum APIENTRY __glim_CheckFramebufferStatusEXT(GLenum target);
extern GLvoid APIENTRY __glim_FramebufferTexture1DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern GLvoid APIENTRY __glim_FramebufferTexture2DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern GLvoid APIENTRY __glim_FramebufferTexture3DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
extern GLvoid APIENTRY __glim_FramebufferRenderbufferEXT(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
extern GLvoid APIENTRY __glim_GetFramebufferAttachmentParameterivEXT(GLenum target, GLenum attachment, GLenum pname, GLint *params);
extern GLvoid APIENTRY __glim_GenerateMipmapEXT(GLenum target);
#if GL_EXT_framebuffer_blit
GLvoid APIENTRY __glim_BlitFramebufferEXT(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                                          GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
                                          GLbitfield mask, GLenum filter);
#if GL_EXT_framebuffer_multisample
GLvoid APIENTRY __glim_RenderbufferStorageMultisampleEXT(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);

#endif /* GL_EXT_framebuffer_multisample    */
#endif /* GL_EXT_framebuffer_blit           */
#endif /* GL_EXT_framebuffer_object         */

#if GL_NV_occlusion_query
extern GLvoid APIENTRY __glim_BeginQueryNV(GLuint);
extern GLvoid APIENTRY __glim_EndQueryNV(GLvoid);
#endif

#if GL_EXT_bindable_uniform
extern GLvoid APIENTRY __glim_UniformBufferEXT(GLuint program, GLint location, GLuint buffer);
extern GLint APIENTRY __glim_GetUniformBufferSizeEXT(GLuint program, GLint location);
extern GLintptr APIENTRY __glim_GetUniformOffsetEXT(GLuint program, GLint location);
#endif

#if GL_EXT_texture_integer
extern GLvoid APIENTRY __glim_ClearColorIiEXT(GLint r, GLint g, GLint b, GLint a);
extern GLvoid APIENTRY __glim_ClearColorIuiEXT(GLuint r, GLuint g, GLuint b, GLuint a);
extern GLvoid APIENTRY __glim_TexParameterIivEXT(GLenum target, GLenum pname, GLint * params);
extern GLvoid APIENTRY __glim_TexParameterIuivEXT(GLenum target, GLenum pname, GLuint *params);
extern GLvoid APIENTRY __glim_GetTexParameterIivEXT(GLenum target, GLenum pname, GLint *params);
extern GLvoid APIENTRY __glim_GetTexParameterIuivEXT(GLenum target, GLenum pname, GLuint *params);
#endif

#if GL_EXT_gpu_shader4
extern GLvoid APIENTRY __glim_VertexAttribI1iEXT(GLuint index, GLint x);
extern GLvoid APIENTRY __glim_VertexAttribI2iEXT(GLuint index, GLint x, GLint y);
extern GLvoid APIENTRY __glim_VertexAttribI3iEXT(GLuint index, GLint x, GLint y, GLint z);
extern GLvoid APIENTRY __glim_VertexAttribI4iEXT(GLuint index, GLint x, GLint y, GLint z, GLint w);

extern GLvoid APIENTRY __glim_VertexAttribI1uiEXT(GLuint index, GLuint x);
extern GLvoid APIENTRY __glim_VertexAttribI2uiEXT(GLuint index, GLuint x, GLuint y);
extern GLvoid APIENTRY __glim_VertexAttribI3uiEXT(GLuint index, GLuint x, GLuint y, GLuint z);
extern GLvoid APIENTRY __glim_VertexAttribI4uiEXT(GLuint index, GLuint x, GLuint y, GLuint z,
                            GLuint w);

extern GLvoid APIENTRY __glim_VertexAttribI1ivEXT(GLuint index, const GLint *v);
extern GLvoid APIENTRY __glim_VertexAttribI2ivEXT(GLuint index, const GLint *v);
extern GLvoid APIENTRY __glim_VertexAttribI3ivEXT(GLuint index, const GLint *v);
extern GLvoid APIENTRY __glim_VertexAttribI4ivEXT(GLuint index, const GLint *v);

extern GLvoid APIENTRY __glim_VertexAttribI1uivEXT(GLuint index, const GLuint *v);
extern GLvoid APIENTRY __glim_VertexAttribI2uivEXT(GLuint index, const GLuint *v);
extern GLvoid APIENTRY __glim_VertexAttribI3uivEXT(GLuint index, const GLuint *v);
extern GLvoid APIENTRY __glim_VertexAttribI4uivEXT(GLuint index, const GLuint *v);

extern GLvoid APIENTRY __glim_VertexAttribI4bvEXT(GLuint index, const GLbyte *v);
extern GLvoid APIENTRY __glim_VertexAttribI4svEXT(GLuint index, const GLshort *v);
extern GLvoid APIENTRY __glim_VertexAttribI4ubvEXT(GLuint index, const GLubyte *v);
extern GLvoid APIENTRY __glim_VertexAttribI4usvEXT(GLuint index, const GLushort *v);

extern GLvoid APIENTRY __glim_VertexAttribIPointerEXT(GLuint index, GLint size, GLenum type,
                                GLsizei stride, const GLvoid *pointer);

extern GLvoid APIENTRY __glim_GetVertexAttribIivEXT(GLuint index, GLenum pname, GLint *params);
extern GLvoid APIENTRY __glim_GetVertexAttribIuivEXT(GLuint index, GLenum pname, GLuint *params);

extern GLvoid APIENTRY __glim_Uniform1uiEXT(GLint location, GLuint v0);
extern GLvoid APIENTRY __glim_Uniform2uiEXT(GLint location, GLuint v0, GLuint v1);
extern GLvoid APIENTRY __glim_Uniform3uiEXT(GLint location, GLuint v0, GLuint v1, GLuint v2);
extern GLvoid APIENTRY __glim_Uniform4uiEXT(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);

extern GLvoid APIENTRY __glim_Uniform1uivEXT(GLint location, GLsizei count, const GLuint *value);
extern GLvoid APIENTRY __glim_Uniform2uivEXT(GLint location, GLsizei count, const GLuint *value);
extern GLvoid APIENTRY __glim_Uniform3uivEXT(GLint location, GLsizei count, const GLuint *value);
extern GLvoid APIENTRY __glim_Uniform4uivEXT(GLint location, GLsizei count, const GLuint *value);

extern GLvoid APIENTRY __glim_GetUniformuivEXT(GLuint program, GLint location, GLuint *params);

extern GLvoid APIENTRY __glim_BindFragDataLocationEXT(GLuint program, GLuint colorNumber,
                                const GLbyte *name);
extern GLint APIENTRY __glim_GetFragDataLocationEXT(GLuint program, const GLbyte *name);
#endif

#if GL_EXT_geometry_shader4
extern GLvoid APIENTRY __glim_ProgramParameteriEXT(GLuint program, GLenum pname, GLint value);
extern GLvoid APIENTRY __glim_FramebufferTextureEXT(GLenum target, GLenum attachment, GLuint texture, GLint level);
extern GLvoid APIENTRY __glim_FramebufferTextureLayerEXT(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
extern GLvoid APIENTRY __glim_FramebufferTextureFaceEXT(GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face);
#endif

#if GL_EXT_draw_instanced
extern GLvoid APIENTRY __glim_DrawArraysInstancedEXT(GLenum mode,
                                                GLint first, GLsizei count, GLsizei primCount);
extern GLvoid APIENTRY __glim_DrawElementsInstancedEXT(GLenum mode,
                        GLsizei count, GLenum type, const GLvoid *indices, GLsizei primCount);
#endif /* GL_EXT_draw_instanced */

#if GL_EXT_draw_buffers2
extern GLvoid APIENTRY __glim_ColorMaskIndexedEXT(GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
extern GLvoid APIENTRY __glim_GetBooleanIndexedvEXT(GLenum value, GLuint index, GLboolean *data);
extern GLvoid APIENTRY __glim_GetIntegerIndexedvEXT(GLenum value, GLuint index, GLint *data);
extern GLvoid APIENTRY __glim_EnableIndexedEXT(GLenum target, GLuint index);
extern GLvoid APIENTRY __glim_DisableIndexedEXT(GLenum target, GLuint index);
extern GLboolean APIENTRY __glim_IsEnabledIndexedEXT(GLenum target, GLuint index);
#endif

#if GL_EXT_texture_buffer_object
extern GLvoid APIENTRY __glim_TexBufferEXT(GLenum target, GLenum internalformat, GLuint buffer);
#endif

#if GL_EXT_gpu_program_parameters
extern GLvoid APIENTRY __glim_ProgramEnvParameters4fvEXT(GLenum target, GLuint index, GLsizei count, const GLfloat *params);
extern GLvoid APIENTRY __glim_ProgramLocalParameters4fvEXT(GLenum target, GLuint index, GLsizei count, const GLfloat *params);
#endif

#if GL_ARB_color_buffer_float
extern GLvoid APIENTRY __glim_ClampColorARB(GLenum target, GLenum clamp);
#endif

#if GL_EXT_timer_query
extern GLvoid APIENTRY __glim_GetQueryObjecti64vEXT(GLuint id, GLenum pname, GLint64EXT *params);
extern GLvoid APIENTRY __glim_GetQueryObjectui64vEXT(GLuint id, GLenum pname, GLuint64EXT *params);
#endif

#if GL_ATI_separate_stencil
extern GLvoid APIENTRY __glim_StencilFuncSeparateATI( GLenum, GLenum, GLint, GLuint);
#endif
extern GLvoid APIENTRY __glim_End_Error(GLvoid);


extern GLvoid APIENTRY __glim_DrawElements_Validate( GLenum, GLsizei, GLenum, const GLvoid *);
extern GLvoid APIENTRY __glim_ArrayElement_Validate( GLint);
extern GLvoid APIENTRY __glim_DrawArrays_Validate( GLenum, GLint, GLsizei);


#endif /* __g_imfncs_h_ */
