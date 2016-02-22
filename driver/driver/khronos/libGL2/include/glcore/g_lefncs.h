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


#ifndef __g_lefncs_h_
#define __g_lefncs_h_

extern const GLubyte *__glle_Skip(const GLubyte *PC);
extern const GLubyte *__glle_InvalidValue(const GLubyte *);
extern const GLubyte *__glle_InvalidEnum(const GLubyte *);
extern const GLubyte *__glle_InvalidOperation(const GLubyte *);
extern const GLubyte *__glle_TableTooLarge(const GLubyte *);
extern const GLubyte *__glle_Primitive(const GLubyte *);

extern const GLubyte *__glle_CallList(const GLubyte *);
extern const GLubyte *__glle_CallLists(const GLubyte *);
extern const GLubyte *__glle_DeleteLists(const GLubyte *);
extern const GLubyte *__glle_GenLists(const GLubyte *);
extern const GLubyte *__glle_ListBase(const GLubyte *);
extern const GLubyte *__glle_Begin(const GLubyte *);
extern const GLubyte *__glle_Bitmap(const GLubyte *);
extern const GLubyte *__glle_Color3fv(const GLubyte *);
extern const GLubyte *__glle_Color4fv(const GLubyte *);
extern const GLubyte *__glle_Color4ubv(const GLubyte *);
extern const GLubyte *__glle_EdgeFlag(const GLubyte *);
extern const GLubyte *__glle_End(const GLubyte *);
extern const GLubyte *__glle_Indexf(const GLubyte *);
extern const GLubyte *__glle_Normal3fv(const GLubyte *);
extern const GLubyte *__glle_RasterPos2fv(const GLubyte *);
extern const GLubyte *__glle_RasterPos3fv(const GLubyte *);
extern const GLubyte *__glle_RasterPos4fv(const GLubyte *);
extern const GLubyte *__glle_Rectf(const GLubyte *);
extern const GLubyte *__glle_TexCoord2fv(const GLubyte *);
extern const GLubyte *__glle_TexCoord3fv(const GLubyte *);
extern const GLubyte *__glle_TexCoord4fv(const GLubyte *);
extern const GLubyte *__glle_Vertex2fv(const GLubyte *);
extern const GLubyte *__glle_Vertex3fv(const GLubyte *);
extern const GLubyte *__glle_Vertex4fv(const GLubyte *);
extern const GLubyte *__glle_ClipPlane(const GLubyte *);
extern const GLubyte *__glle_ColorMaterial(const GLubyte *);
extern const GLubyte *__glle_CullFace(const GLubyte *);
extern const GLubyte *__glle_Fogfv(const GLubyte *);
extern const GLubyte *__glle_Fogiv(const GLubyte *);
extern const GLubyte *__glle_FrontFace(const GLubyte *);
extern const GLubyte *__glle_Hint(const GLubyte *);
extern const GLubyte *__glle_Lightfv(const GLubyte *);
extern const GLubyte *__glle_Lightiv(const GLubyte *);
extern const GLubyte *__glle_LightModelfv(const GLubyte *);
extern const GLubyte *__glle_LightModeliv(const GLubyte *);
extern const GLubyte *__glle_LineStipple(const GLubyte *);
extern const GLubyte *__glle_LineWidth(const GLubyte *);
extern const GLubyte *__glle_Materialfv(const GLubyte *);
extern const GLubyte *__glle_Materialiv(const GLubyte *);
extern const GLubyte *__glle_PointSize(const GLubyte *);
extern const GLubyte *__glle_PolygonMode(const GLubyte *);
extern const GLubyte *__glle_PolygonStipple(const GLubyte *);
extern const GLubyte *__glle_Scissor(const GLubyte *);
extern const GLubyte *__glle_ShadeModel(const GLubyte *);
extern const GLubyte *__glle_TexParameterfv(const GLubyte *);
extern const GLubyte *__glle_TexParameteriv(const GLubyte *);
extern const GLubyte *__glle_TexImage1D(const GLubyte *);
extern const GLubyte *__glle_TexImage2D(const GLubyte *);
extern const GLubyte *__glle_TexEnvfv(const GLubyte *);
extern const GLubyte *__glle_TexEnviv(const GLubyte *);
extern const GLubyte *__glle_TexGendv(const GLubyte *);
extern const GLubyte *__glle_TexGenfv(const GLubyte *);
extern const GLubyte *__glle_TexGeniv(const GLubyte *);
extern const GLubyte *__glle_FeedbackBuffer(const GLubyte *);
extern const GLubyte *__glle_SelectBuffer(const GLubyte *);
extern const GLubyte *__glle_RenderMode(const GLubyte *);
extern const GLubyte *__glle_InitNames(const GLubyte *);
extern const GLubyte *__glle_LoadName(const GLubyte *);
extern const GLubyte *__glle_PassThrough(const GLubyte *);
extern const GLubyte *__glle_PopName(const GLubyte *);
extern const GLubyte *__glle_PushName(const GLubyte *);
extern const GLubyte *__glle_DrawBuffer(const GLubyte *);
extern const GLubyte *__glle_Clear(const GLubyte *);
extern const GLubyte *__glle_ClearAccum(const GLubyte *);
extern const GLubyte *__glle_ClearIndex(const GLubyte *);
extern const GLubyte *__glle_ClearColor(const GLubyte *);
extern const GLubyte *__glle_ClearStencil(const GLubyte *);
extern const GLubyte *__glle_ClearDepth(const GLubyte *);
extern const GLubyte *__glle_StencilMask(const GLubyte *);
extern const GLubyte *__glle_ColorMask(const GLubyte *);
extern const GLubyte *__glle_DepthMask(const GLubyte *);
extern const GLubyte *__glle_IndexMask(const GLubyte *);
extern const GLubyte *__glle_Accum(const GLubyte *);
extern const GLubyte *__glle_Disable(const GLubyte *);
extern const GLubyte *__glle_Enable(const GLubyte *);
extern const GLubyte *__glle_Finish(const GLubyte *);
extern const GLubyte *__glle_Flush(const GLubyte *);
extern const GLubyte *__glle_PopAttrib(const GLubyte *);
extern const GLubyte *__glle_PushAttrib(const GLubyte *);
extern const GLubyte *__glle_Map1d(const GLubyte *);
extern const GLubyte *__glle_Map1f(const GLubyte *);
extern const GLubyte *__glle_Map2d(const GLubyte *);
extern const GLubyte *__glle_Map2f(const GLubyte *);
extern const GLubyte *__glle_MapGrid1d(const GLubyte *);
extern const GLubyte *__glle_MapGrid1f(const GLubyte *);
extern const GLubyte *__glle_MapGrid2d(const GLubyte *);
extern const GLubyte *__glle_MapGrid2f(const GLubyte *);
extern const GLubyte *__glle_EvalCoord1d(const GLubyte *);
extern const GLubyte *__glle_EvalCoord1dv(const GLubyte *);
extern const GLubyte *__glle_EvalCoord1f(const GLubyte *);
extern const GLubyte *__glle_EvalCoord1fv(const GLubyte *);
extern const GLubyte *__glle_EvalCoord2d(const GLubyte *);
extern const GLubyte *__glle_EvalCoord2dv(const GLubyte *);
extern const GLubyte *__glle_EvalCoord2f(const GLubyte *);
extern const GLubyte *__glle_EvalCoord2fv(const GLubyte *);
extern const GLubyte *__glle_EvalMesh1(const GLubyte *);
extern const GLubyte *__glle_EvalPoint1(const GLubyte *);
extern const GLubyte *__glle_EvalMesh2(const GLubyte *);
extern const GLubyte *__glle_EvalPoint2(const GLubyte *);
extern const GLubyte *__glle_AlphaFunc(const GLubyte *);
extern const GLubyte *__glle_BlendFunc(const GLubyte *);
extern const GLubyte *__glle_LogicOp(const GLubyte *);
extern const GLubyte *__glle_StencilFunc(const GLubyte *);
extern const GLubyte *__glle_StencilOp(const GLubyte *);
extern const GLubyte *__glle_DepthFunc(const GLubyte *);
extern const GLubyte *__glle_PixelZoom(const GLubyte *);
extern const GLubyte *__glle_PixelTransferf(const GLubyte *);
extern const GLubyte *__glle_PixelTransferi(const GLubyte *);
extern const GLubyte *__glle_PixelStoref(const GLubyte *);
extern const GLubyte *__glle_PixelStorei(const GLubyte *);
extern const GLubyte *__glle_PixelMapfv(const GLubyte *);
extern const GLubyte *__glle_PixelMapuiv(const GLubyte *);
extern const GLubyte *__glle_PixelMapusv(const GLubyte *);
extern const GLubyte *__glle_ReadBuffer(const GLubyte *);
extern const GLubyte *__glle_CopyPixels(const GLubyte *);
extern const GLubyte *__glle_ReadPixels(const GLubyte *);
extern const GLubyte *__glle_DrawPixels(const GLubyte *);
extern const GLubyte *__glle_GetBooleanv(const GLubyte *);
extern const GLubyte *__glle_GetClipPlane(const GLubyte *);
extern const GLubyte *__glle_GetDoublev(const GLubyte *);
extern const GLubyte *__glle_GetError(const GLubyte *);
extern const GLubyte *__glle_GetFloatv(const GLubyte *);
extern const GLubyte *__glle_GetIntegerv(const GLubyte *);
extern const GLubyte *__glle_GetLightfv(const GLubyte *);
extern const GLubyte *__glle_GetLightiv(const GLubyte *);
extern const GLubyte *__glle_GetMapdv(const GLubyte *);
extern const GLubyte *__glle_GetMapfv(const GLubyte *);
extern const GLubyte *__glle_GetMapiv(const GLubyte *);
extern const GLubyte *__glle_GetMaterialfv(const GLubyte *);
extern const GLubyte *__glle_GetMaterialiv(const GLubyte *);
extern const GLubyte *__glle_GetPixelMapfv(const GLubyte *);
extern const GLubyte *__glle_GetPixelMapuiv(const GLubyte *);
extern const GLubyte *__glle_GetPixelMapusv(const GLubyte *);
extern const GLubyte *__glle_GetPolygonStipple(const GLubyte *);
extern const GLubyte *__glle_GetString(const GLubyte *);
extern const GLubyte *__glle_GetTexEnvfv(const GLubyte *);
extern const GLubyte *__glle_GetTexEnviv(const GLubyte *);
extern const GLubyte *__glle_GetTexGendv(const GLubyte *);
extern const GLubyte *__glle_GetTexGenfv(const GLubyte *);
extern const GLubyte *__glle_GetTexGeniv(const GLubyte *);
extern const GLubyte *__glle_GetTexImage(const GLubyte *);
extern const GLubyte *__glle_GetTexParameterfv(const GLubyte *);
extern const GLubyte *__glle_GetTexParameteriv(const GLubyte *);
extern const GLubyte *__glle_GetTexLevelParameterfv(const GLubyte *);
extern const GLubyte *__glle_GetTexLevelParameteriv(const GLubyte *);
extern const GLubyte *__glle_IsEnabled(const GLubyte *);
extern const GLubyte *__glle_IsList(const GLubyte *);
extern const GLubyte *__glle_DepthRange(const GLubyte *);
extern const GLubyte *__glle_Frustum(const GLubyte *);
extern const GLubyte *__glle_LoadIdentity(const GLubyte *);
extern const GLubyte *__glle_LoadMatrixf(const GLubyte *);
extern const GLubyte *__glle_LoadMatrixd(const GLubyte *);
extern const GLubyte *__glle_MatrixMode(const GLubyte *);
extern const GLubyte *__glle_MultMatrixf(const GLubyte *);
extern const GLubyte *__glle_MultMatrixd(const GLubyte *);
extern const GLubyte *__glle_Ortho(const GLubyte *);
extern const GLubyte *__glle_PopMatrix(const GLubyte *);
extern const GLubyte *__glle_PushMatrix(const GLubyte *);
extern const GLubyte *__glle_Rotated(const GLubyte *);
extern const GLubyte *__glle_Rotatef(const GLubyte *);
extern const GLubyte *__glle_Scaled(const GLubyte *);
extern const GLubyte *__glle_Scalef(const GLubyte *);
extern const GLubyte *__glle_Translated(const GLubyte *);
extern const GLubyte *__glle_Translatef(const GLubyte *);
extern const GLubyte *__glle_Viewport(const GLubyte *);
extern const GLubyte *__glle_ColorSubTable(const GLubyte *);
extern const GLubyte *__glle_ColorTable(const GLubyte *);
extern const GLubyte *__glle_CopyColorTable(const GLubyte *);
extern const GLubyte *__glle_GetColorTable(const GLubyte *);
extern const GLubyte *__glle_GetColorTableParameterfv(const GLubyte *);
extern const GLubyte *__glle_GetColorTableParameteriv(const GLubyte *);
extern const GLubyte *__glle_ColorPointer(const GLubyte *);
extern const GLubyte *__glle_DisableClientState(const GLubyte *);
extern const GLubyte *__glle_EdgeFlagPointer(const GLubyte *);
extern const GLubyte *__glle_EnableClientState(const GLubyte *);
extern const GLubyte *__glle_GetPointerv(const GLubyte *);
extern const GLubyte *__glle_IndexPointer(const GLubyte *);
extern const GLubyte *__glle_InterleavedArrays(const GLubyte *);
extern const GLubyte *__glle_NormalPointer(const GLubyte *);
extern const GLubyte *__glle_TexCoordPointer(const GLubyte *);
extern const GLubyte *__glle_VertexPointer(const GLubyte *);
extern const GLubyte *__glle_PolygonOffset(const GLubyte *);
extern const GLubyte *__glle_CopyTexImage1D(const GLubyte *);
extern const GLubyte *__glle_CopyTexImage2D(const GLubyte *);
extern const GLubyte *__glle_CopyTexSubImage1D(const GLubyte *);
extern const GLubyte *__glle_CopyTexSubImage2D(const GLubyte *);
extern const GLubyte *__glle_TexSubImage1D(const GLubyte *);
extern const GLubyte *__glle_TexSubImage2D(const GLubyte *);
extern const GLubyte *__glle_AreTexturesResident(const GLubyte *);
extern const GLubyte *__glle_BindTexture(const GLubyte *);
extern const GLubyte *__glle_DeleteTextures(const GLubyte *);
extern const GLubyte *__glle_GenTextures(const GLubyte *);
extern const GLubyte *__glle_IsTexture(const GLubyte *);
extern const GLubyte *__glle_PrioritizeTextures(const GLubyte *);
extern const GLubyte *__glle_PopClientAttrib(const GLubyte *);
extern const GLubyte *__glle_PushClientAttrib(const GLubyte *);

#if GL_VERSION_1_2
extern const GLubyte *__glle_BlendColor(const GLubyte *);
extern const GLubyte *__glle_BlendEquation(const GLubyte *);
extern const GLubyte *__glle_ColorTable(const GLubyte *);
extern const GLubyte *__glle_ColorTableParameterfv(const GLubyte *);
extern const GLubyte *__glle_ColorTableParameteriv(const GLubyte *);
extern const GLubyte *__glle_CopyColorTable(const GLubyte *);
extern const GLubyte *__glle_GetColorTable(const GLubyte *);
extern const GLubyte *__glle_GetColorTableParameterfv(const GLubyte *);
extern const GLubyte *__glle_GetColorTableParameteriv(const GLubyte *);
extern const GLubyte *__glle_ColorSubTable(const GLubyte *);
extern const GLubyte *__glle_CopyColorSubTable(const GLubyte *);
extern const GLubyte *__glle_ConvolutionFilter1D(const GLubyte *);
extern const GLubyte *__glle_ConvolutionFilter2D(const GLubyte *);
extern const GLubyte *__glle_ConvolutionParameterf(const GLubyte *);
extern const GLubyte *__glle_ConvolutionParameterfv(const GLubyte *);
extern const GLubyte *__glle_ConvolutionParameteri(const GLubyte *);
extern const GLubyte *__glle_ConvolutionParameteriv(const GLubyte *);
extern const GLubyte *__glle_CopyConvolutionFilter1D(const GLubyte *);
extern const GLubyte *__glle_CopyConvolutionFilter2D(const GLubyte *);
extern const GLubyte *__glle_GetConvolutionFilter(const GLubyte *);
extern const GLubyte *__glle_GetConvolutionParameterfv(const GLubyte *);
extern const GLubyte *__glle_GetConvolutionParameteriv(const GLubyte *);
extern const GLubyte *__glle_GetSeparableFilter(const GLubyte *);
extern const GLubyte *__glle_SeparableFilter2D(const GLubyte *);
extern const GLubyte *__glle_GetHistogram(const GLubyte *);
extern const GLubyte *__glle_GetHistogramParameterfv(const GLubyte *);
extern const GLubyte *__glle_GetHistogramParameteriv(const GLubyte *);
extern const GLubyte *__glle_GetMinmax(const GLubyte *);
extern const GLubyte *__glle_GetMinmaxParameterfv(const GLubyte *);
extern const GLubyte *__glle_GetMinmaxParameteriv(const GLubyte *);
extern const GLubyte *__glle_Histogram(const GLubyte *);
extern const GLubyte *__glle_Minmax(const GLubyte *);
extern const GLubyte *__glle_ResetHistogram(const GLubyte *);
extern const GLubyte *__glle_ResetMinmax(const GLubyte *);
extern const GLubyte *__glle_TexImage3D(const GLubyte *);
extern const GLubyte *__glle_TexSubImage3D(const GLubyte *);
extern const GLubyte *__glle_CopyTexSubImage3D(const GLubyte *);
#endif
#if GL_VERSION_1_3
extern const GLubyte *__glle_ActiveTexture(const GLubyte *);
extern const GLubyte *__glle_ClientActiveTexture(const GLubyte *);
extern const GLubyte *__glle_MultiTexCoord2fv(const GLubyte *);
extern const GLubyte *__glle_MultiTexCoord3fv(const GLubyte *);
extern const GLubyte *__glle_MultiTexCoord4fv(const GLubyte *);
extern const GLubyte *__glle_LoadTransposeMatrixf(const GLubyte *);
extern const GLubyte *__glle_LoadTransposeMatrixd(const GLubyte *);
extern const GLubyte *__glle_MultTransposeMatrixf(const GLubyte *);
extern const GLubyte *__glle_MultTransposeMatrixd(const GLubyte *);
extern const GLubyte *__glle_SampleCoverage(const GLubyte *);
extern const GLubyte *__glle_CompressedTexImage3D(const GLubyte *);
extern const GLubyte *__glle_CompressedTexImage2D(const GLubyte *);
extern const GLubyte *__glle_CompressedTexImage1D(const GLubyte *);
extern const GLubyte *__glle_CompressedTexSubImage3D(const GLubyte *);
extern const GLubyte *__glle_CompressedTexSubImage2D(const GLubyte *);
extern const GLubyte *__glle_CompressedTexSubImage1D(const GLubyte *);
extern const GLubyte *__glle_GetCompressedTexImage(const GLubyte *);
#endif
#if GL_VERSION_1_4
extern const GLubyte *__glle_BlendFuncSeparate(const GLubyte *);
extern const GLubyte *__glle_FogCoordf(const GLubyte *);
extern const GLubyte *__glle_FogCoordfv(const GLubyte *);
extern const GLubyte *__glle_FogCoordd(const GLubyte *);
extern const GLubyte *__glle_FogCoorddv(const GLubyte *);
extern const GLubyte *__glle_FogCoordPointer(const GLubyte *);
extern const GLubyte *__glle_MultiDrawArrays(const GLubyte *);
extern const GLubyte *__glle_MultiDrawElements(const GLubyte *);
extern const GLubyte *__glle_PointParameterfv(const GLubyte *);
extern const GLubyte *__glle_PointParameteriv(const GLubyte *);
extern const GLubyte *__glle_SecondaryColor3fv(const GLubyte *);
extern const GLubyte *__glle_SecondaryColorPointer(const GLubyte *);
extern const GLubyte *__glle_WindowPos2fv(const GLubyte *);
extern const GLubyte *__glle_WindowPos3fv(const GLubyte *);
#endif
#if GL_VERSION_1_5
extern const GLubyte *__glle_GenQueries(const GLubyte *);
extern const GLubyte *__glle_DeleteQueries(const GLubyte *);
extern const GLubyte *__glle_IsQuery(const GLubyte *);
extern const GLubyte *__glle_BeginQuery(const GLubyte *);
extern const GLubyte *__glle_EndQuery(const GLubyte *);
extern const GLubyte *__glle_GetQueryiv(const GLubyte *);
extern const GLubyte *__glle_GetQueryObjectiv(const GLubyte *);
extern const GLubyte *__glle_GetQueryObjectuiv(const GLubyte *);
extern const GLubyte *__glle_BindBuffer(const GLubyte *);
extern const GLubyte *__glle_DeleteBuffers(const GLubyte *);
extern const GLubyte *__glle_GenBuffers(const GLubyte *);
extern const GLubyte *__glle_IsBuffer(const GLubyte *);
extern const GLubyte *__glle_BufferData(const GLubyte *);
extern const GLubyte *__glle_BufferSubData(const GLubyte *);
extern const GLubyte *__glle_GetBufferSubData(const GLubyte *);
extern const GLubyte *__glle_MapBuffer(const GLubyte *);
extern const GLubyte *__glle_UnmapBuffer(const GLubyte *);
extern const GLubyte *__glle_GetBufferParameteriv(const GLubyte *);
extern const GLubyte *__glle_GetBufferPointerv(const GLubyte *);
#endif
#if GL_VERSION_2_0
extern const GLubyte *__glle_BlendEquationSeparate(const GLubyte *);
extern const GLubyte *__glle_DrawBuffers(const GLubyte *);
extern const GLubyte *__glle_StencilOpSeparate(const GLubyte *);
extern const GLubyte *__glle_StencilFuncSeparate(const GLubyte *);
extern const GLubyte *__glle_StencilMaskSeparate(const GLubyte *);
extern const GLubyte *__glle_AttachShader(const GLubyte *);
extern const GLubyte *__glle_BindAttribLocation(const GLubyte *);
extern const GLubyte *__glle_CompileShader(const GLubyte *);
extern const GLubyte *__glle_CreateProgram(const GLubyte *);
extern const GLubyte *__glle_CreateShader(const GLubyte *);
extern const GLubyte *__glle_DeleteProgram(const GLubyte *);
extern const GLubyte *__glle_DeleteShader(const GLubyte *);
extern const GLubyte *__glle_DetachShader(const GLubyte *);
extern const GLubyte *__glle_DisableVertexAttribArray(const GLubyte *);
extern const GLubyte *__glle_EnableVertexAttribArray(const GLubyte *);
extern const GLubyte *__glle_GetActiveAttrib(const GLubyte *);
extern const GLubyte *__glle_GetActiveUniform(const GLubyte *);
extern const GLubyte *__glle_GetAttachedShaders(const GLubyte *);
extern const GLubyte *__glle_GetAttribLocation(const GLubyte *);
extern const GLubyte *__glle_GetProgramiv(const GLubyte *);
extern const GLubyte *__glle_GetProgramInfoLog(const GLubyte *);
extern const GLubyte *__glle_GetShaderiv(const GLubyte *);
extern const GLubyte *__glle_GetShaderInfoLog(const GLubyte *);
extern const GLubyte *__glle_GetShaderSource(const GLubyte *);
extern const GLubyte *__glle_GetUniformLocation(const GLubyte *);
extern const GLubyte *__glle_GetUniformfv(const GLubyte *);
extern const GLubyte *__glle_GetUniformiv(const GLubyte *);
extern const GLubyte *__glle_GetVertexAttribdv(const GLubyte *);
extern const GLubyte *__glle_GetVertexAttribfv(const GLubyte *);
extern const GLubyte *__glle_GetVertexAttribiv(const GLubyte *);
extern const GLubyte *__glle_GetVertexAttribPointerv(const GLubyte *);
extern const GLubyte *__glle_IsProgram(const GLubyte *);
extern const GLubyte *__glle_IsShader(const GLubyte *);
extern const GLubyte *__glle_LinkProgram(const GLubyte *);
extern const GLubyte *__glle_ShaderSource(const GLubyte *);
extern const GLubyte *__glle_UseProgram(const GLubyte *);
extern const GLubyte *__glle_Uniform1f(const GLubyte *);
extern const GLubyte *__glle_Uniform2f(const GLubyte *);
extern const GLubyte *__glle_Uniform3f(const GLubyte *);
extern const GLubyte *__glle_Uniform4f(const GLubyte *);
extern const GLubyte *__glle_Uniform1i(const GLubyte *);
extern const GLubyte *__glle_Uniform2i(const GLubyte *);
extern const GLubyte *__glle_Uniform3i(const GLubyte *);
extern const GLubyte *__glle_Uniform4i(const GLubyte *);
extern const GLubyte *__glle_Uniform1fv(const GLubyte *);
extern const GLubyte *__glle_Uniform2fv(const GLubyte *);
extern const GLubyte *__glle_Uniform3fv(const GLubyte *);
extern const GLubyte *__glle_Uniform4fv(const GLubyte *);
extern const GLubyte *__glle_Uniform1iv(const GLubyte *);
extern const GLubyte *__glle_Uniform2iv(const GLubyte *);
extern const GLubyte *__glle_Uniform3iv(const GLubyte *);
extern const GLubyte *__glle_Uniform4iv(const GLubyte *);
extern const GLubyte *__glle_UniformMatrix2fv(const GLubyte *);
extern const GLubyte *__glle_UniformMatrix3fv(const GLubyte *);
extern const GLubyte *__glle_UniformMatrix4fv(const GLubyte *);
extern const GLubyte *__glle_ValidateProgram(const GLubyte *);
extern const GLubyte *__glle_VertexAttrib4fv(const GLubyte *);
extern const GLubyte *__glle_VertexAttribPointer(const GLubyte *);
#endif

#if GL_VERSION_2_1
extern const GLubyte *__glle_UniformMatrix2x3fv(const GLubyte *);
extern const GLubyte *__glle_UniformMatrix2x4fv(const GLubyte *);
extern const GLubyte *__glle_UniformMatrix3x2fv(const GLubyte *);
extern const GLubyte *__glle_UniformMatrix3x4fv(const GLubyte *);
extern const GLubyte *__glle_UniformMatrix4x2fv(const GLubyte *);
extern const GLubyte *__glle_UniformMatrix4x3fv(const GLubyte *);
#endif


#if GL_ARB_vertex_program
extern const GLubyte *__glle_ProgramStringARB(const GLubyte *);
extern const GLubyte *__glle_BindProgramARB(const GLubyte *);
extern const GLubyte *__glle_DeleteProgramsARB(const GLubyte *);
extern const GLubyte *__glle_GenProgramsARB(const GLubyte *);
extern const GLubyte *__glle_ProgramEnvParameter4dARB(const GLubyte *);
extern const GLubyte *__glle_ProgramEnvParameter4dvARB(const GLubyte *);
extern const GLubyte *__glle_ProgramEnvParameter4fARB(const GLubyte *);
extern const GLubyte *__glle_ProgramEnvParameter4fvARB(const GLubyte *);
extern const GLubyte *__glle_ProgramLocalParameter4dARB(const GLubyte *);
extern const GLubyte *__glle_ProgramLocalParameter4dvARB(const GLubyte *);
extern const GLubyte *__glle_ProgramLocalParameter4fARB(const GLubyte *);
extern const GLubyte *__glle_ProgramLocalParameter4fvARB(const GLubyte *);
extern const GLubyte *__glle_GetProgramEnvParameterdvARB(const GLubyte *);
extern const GLubyte *__glle_GetProgramEnvParameterfvARB(const GLubyte *);
extern const GLubyte *__glle_GetProgramLocalParameterdvARB(const GLubyte *);
extern const GLubyte *__glle_GetProgramLocalParameterfvARB(const GLubyte *);
extern const GLubyte *__glle_GetProgramivARB(const GLubyte *);
extern const GLubyte *__glle_GetProgramStringARB(const GLubyte *);
extern const GLubyte *__glle_IsProgramARB(const GLubyte *);
#endif

#if GL_ARB_shader_objects
extern const GLubyte *__glle_DeleteObjectARB(const GLubyte *);
extern const GLubyte *__glle_GetHandleARB(const GLubyte *);
extern const GLubyte *__glle_GetInfoLogARB(const GLubyte *);
extern const GLubyte *__glle_GetObjectParameterfvARB(const GLubyte *);
extern const GLubyte *__glle_GetObjectParameterivARB(const GLubyte *);
#endif

#if GL_ATI_element_array
extern const GLubyte *__glle_DrawElementArrayATI(const GLubyte *);
extern const GLubyte *__glle_DrawRangeElementArrayATI(const GLubyte *);
#endif

#if GL_EXT_stencil_two_side
extern const GLubyte *__glle_ActiveStencilFaceEXT(const GLubyte *);
#endif

#if GL_EXT_depth_bounds_test
extern const GLubyte * __glle_DepthBoundsEXT(const GLubyte *PC);
#endif

#if GL_NV_occlusion_query
extern const GLubyte *__glle_BeginQueryNV(const GLubyte *);
extern const GLubyte *__glle_EndQueryNV(const GLubyte *);
#endif

#if GL_EXT_texture_integer
extern const GLubyte *__glle_ClearColorIiEXT(const GLubyte *);
extern const GLubyte *__glle_ClearColorIuiEXT(const GLubyte *);
extern const GLubyte *__glle_TexParameterIivEXT(const GLubyte *);
extern const GLubyte *__glle_TexParameterIuivEXT(const GLubyte *);
#endif

#if GL_EXT_gpu_shader4
extern const GLubyte *__glle_Uniform1uiEXT(const GLubyte *);
extern const GLubyte *__glle_Uniform2uiEXT(const GLubyte *);
extern const GLubyte *__glle_Uniform3uiEXT(const GLubyte *);
extern const GLubyte *__glle_Uniform4uiEXT(const GLubyte *);

extern const GLubyte *__glle_Uniform1uivEXT(const GLubyte *);
extern const GLubyte *__glle_Uniform2uivEXT(const GLubyte *);
extern const GLubyte *__glle_Uniform3uivEXT(const GLubyte *);
extern const GLubyte *__glle_Uniform4uivEXT(const GLubyte *);

#endif

#if GL_EXT_geometry_shader4
extern const GLubyte *__glle_FramebufferTextureEXT(const GLubyte *);
extern const GLubyte *__glle_FramebufferTextureLayerEXT(const GLubyte *);
extern const GLubyte *__glle_FramebufferTextureFaceEXT(const GLubyte *);
#endif

#if GL_EXT_draw_buffers2
extern const GLubyte *__glle_ColorMaskIndexedEXT(const GLubyte *);
extern const GLubyte *__glle_EnableIndexedEXT(const GLubyte *);
extern const GLubyte *__glle_DisableIndexedEXT(const GLubyte *);
#endif

#if GL_EXT_gpu_program_parameters
extern const GLubyte *__glle_ProgramEnvParameters4fvEXT(const GLubyte *);
extern const GLubyte *__glle_ProgramLocalParameters4fvEXT(const GLubyte *);
#endif

#if GL_ARB_color_buffer_float
extern const GLubyte *__glle_ClampColorARB(const GLubyte*);
#endif

#if GL_ATI_separate_stencil
extern const GLubyte *__glle_StencilFuncSeparateATI(const GLubyte*);
#endif

#endif /* __g_lefncs_h_ */
