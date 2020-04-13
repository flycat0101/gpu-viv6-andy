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


#ifndef __gc_gl_dispatch_h__
#define __gc_gl_dispatch_h__

#ifndef VIV_EGL_BUILD
#include "gc_es_types.h"
#endif

#define __GL_API_ENTRIES(glApiMacro) \
    glApiMacro(NewList), \
    glApiMacro(EndList), \
    glApiMacro(CallList), \
    glApiMacro(CallLists), \
    glApiMacro(DeleteLists), \
    glApiMacro(GenLists), \
    glApiMacro(ListBase), \
    glApiMacro(Begin), \
    glApiMacro(Bitmap), \
    glApiMacro(Color3b), \
    glApiMacro(Color3bv), \
    glApiMacro(Color3d), \
    glApiMacro(Color3dv), \
    glApiMacro(Color3f), \
    glApiMacro(Color3fv), \
    glApiMacro(Color3i), \
    glApiMacro(Color3iv), \
    glApiMacro(Color3s), \
    glApiMacro(Color3sv), \
    glApiMacro(Color3ub), \
    glApiMacro(Color3ubv), \
    glApiMacro(Color3ui), \
    glApiMacro(Color3uiv), \
    glApiMacro(Color3us), \
    glApiMacro(Color3usv), \
    glApiMacro(Color4b), \
    glApiMacro(Color4bv), \
    glApiMacro(Color4d), \
    glApiMacro(Color4dv), \
    glApiMacro(Color4f), \
    glApiMacro(Color4fv), \
    glApiMacro(Color4i), \
    glApiMacro(Color4iv), \
    glApiMacro(Color4s), \
    glApiMacro(Color4sv), \
    glApiMacro(Color4ub), \
    glApiMacro(Color4ubv), \
    glApiMacro(Color4ui), \
    glApiMacro(Color4uiv), \
    glApiMacro(Color4us), \
    glApiMacro(Color4usv), \
    glApiMacro(EdgeFlag), \
    glApiMacro(EdgeFlagv), \
    glApiMacro(End), \
    glApiMacro(Indexd), \
    glApiMacro(Indexdv), \
    glApiMacro(Indexf), \
    glApiMacro(Indexfv), \
    glApiMacro(Indexi), \
    glApiMacro(Indexiv), \
    glApiMacro(Indexs), \
    glApiMacro(Indexsv), \
    glApiMacro(Normal3b), \
    glApiMacro(Normal3bv), \
    glApiMacro(Normal3d), \
    glApiMacro(Normal3dv), \
    glApiMacro(Normal3f), \
    glApiMacro(Normal3fv), \
    glApiMacro(Normal3i), \
    glApiMacro(Normal3iv), \
    glApiMacro(Normal3s), \
    glApiMacro(Normal3sv), \
    glApiMacro(RasterPos2d), \
    glApiMacro(RasterPos2dv), \
    glApiMacro(RasterPos2f), \
    glApiMacro(RasterPos2fv), \
    glApiMacro(RasterPos2i), \
    glApiMacro(RasterPos2iv), \
    glApiMacro(RasterPos2s), \
    glApiMacro(RasterPos2sv), \
    glApiMacro(RasterPos3d), \
    glApiMacro(RasterPos3dv), \
    glApiMacro(RasterPos3f), \
    glApiMacro(RasterPos3fv), \
    glApiMacro(RasterPos3i), \
    glApiMacro(RasterPos3iv), \
    glApiMacro(RasterPos3s), \
    glApiMacro(RasterPos3sv), \
    glApiMacro(RasterPos4d), \
    glApiMacro(RasterPos4dv), \
    glApiMacro(RasterPos4f), \
    glApiMacro(RasterPos4fv), \
    glApiMacro(RasterPos4i), \
    glApiMacro(RasterPos4iv), \
    glApiMacro(RasterPos4s), \
    glApiMacro(RasterPos4sv), \
    glApiMacro(Rectd), \
    glApiMacro(Rectdv), \
    glApiMacro(Rectf), \
    glApiMacro(Rectfv), \
    glApiMacro(Recti), \
    glApiMacro(Rectiv), \
    glApiMacro(Rects), \
    glApiMacro(Rectsv), \
    glApiMacro(TexCoord1d), \
    glApiMacro(TexCoord1dv), \
    glApiMacro(TexCoord1f), \
    glApiMacro(TexCoord1fv), \
    glApiMacro(TexCoord1i), \
    glApiMacro(TexCoord1iv), \
    glApiMacro(TexCoord1s), \
    glApiMacro(TexCoord1sv), \
    glApiMacro(TexCoord2d), \
    glApiMacro(TexCoord2dv), \
    glApiMacro(TexCoord2f), \
    glApiMacro(TexCoord2fv), \
    glApiMacro(TexCoord2i), \
    glApiMacro(TexCoord2iv), \
    glApiMacro(TexCoord2s), \
    glApiMacro(TexCoord2sv), \
    glApiMacro(TexCoord3d), \
    glApiMacro(TexCoord3dv), \
    glApiMacro(TexCoord3f), \
    glApiMacro(TexCoord3fv), \
    glApiMacro(TexCoord3i), \
    glApiMacro(TexCoord3iv), \
    glApiMacro(TexCoord3s), \
    glApiMacro(TexCoord3sv), \
    glApiMacro(TexCoord4d), \
    glApiMacro(TexCoord4dv), \
    glApiMacro(TexCoord4f), \
    glApiMacro(TexCoord4fv), \
    glApiMacro(TexCoord4i), \
    glApiMacro(TexCoord4iv), \
    glApiMacro(TexCoord4s), \
    glApiMacro(TexCoord4sv), \
    glApiMacro(Vertex2d), \
    glApiMacro(Vertex2dv), \
    glApiMacro(Vertex2f), \
    glApiMacro(Vertex2fv), \
    glApiMacro(Vertex2i), \
    glApiMacro(Vertex2iv), \
    glApiMacro(Vertex2s), \
    glApiMacro(Vertex2sv), \
    glApiMacro(Vertex3d), \
    glApiMacro(Vertex3dv), \
    glApiMacro(Vertex3f), \
    glApiMacro(Vertex3fv), \
    glApiMacro(Vertex3i), \
    glApiMacro(Vertex3iv), \
    glApiMacro(Vertex3s), \
    glApiMacro(Vertex3sv), \
    glApiMacro(Vertex4d), \
    glApiMacro(Vertex4dv), \
    glApiMacro(Vertex4f), \
    glApiMacro(Vertex4fv), \
    glApiMacro(Vertex4i), \
    glApiMacro(Vertex4iv), \
    glApiMacro(Vertex4s), \
    glApiMacro(Vertex4sv), \
    glApiMacro(ClipPlane), \
    glApiMacro(ColorMaterial), \
    glApiMacro(CullFace), \
    glApiMacro(Fogf), \
    glApiMacro(Fogfv), \
    glApiMacro(Fogi), \
    glApiMacro(Fogiv), \
    glApiMacro(FrontFace), \
    glApiMacro(Hint), \
    glApiMacro(Lightf), \
    glApiMacro(Lightfv), \
    glApiMacro(Lighti), \
    glApiMacro(Lightiv), \
    glApiMacro(LightModelf), \
    glApiMacro(LightModelfv), \
    glApiMacro(LightModeli), \
    glApiMacro(LightModeliv), \
    glApiMacro(LineStipple), \
    glApiMacro(LineWidth), \
    glApiMacro(Materialf), \
    glApiMacro(Materialfv), \
    glApiMacro(Materiali), \
    glApiMacro(Materialiv), \
    glApiMacro(PointSize), \
    glApiMacro(PolygonMode), \
    glApiMacro(PolygonStipple), \
    glApiMacro(Scissor), \
    glApiMacro(ShadeModel), \
    glApiMacro(TexParameterf), \
    glApiMacro(TexParameterfv), \
    glApiMacro(TexParameteri), \
    glApiMacro(TexParameteriv), \
    glApiMacro(TexImage1D), \
    glApiMacro(TexImage2D), \
    glApiMacro(TexEnvf), \
    glApiMacro(TexEnvfv), \
    glApiMacro(TexEnvi), \
    glApiMacro(TexEnviv), \
    glApiMacro(TexGend), \
    glApiMacro(TexGendv), \
    glApiMacro(TexGenf), \
    glApiMacro(TexGenfv), \
    glApiMacro(TexGeni), \
    glApiMacro(TexGeniv), \
    glApiMacro(FeedbackBuffer), \
    glApiMacro(SelectBuffer), \
    glApiMacro(RenderMode), \
    glApiMacro(InitNames), \
    glApiMacro(LoadName), \
    glApiMacro(PassThrough), \
    glApiMacro(PopName), \
    glApiMacro(PushName), \
    glApiMacro(DrawBuffer), \
    glApiMacro(Clear), \
    glApiMacro(ClearAccum), \
    glApiMacro(ClearIndex), \
    glApiMacro(ClearColor), \
    glApiMacro(ClearStencil), \
    glApiMacro(ClearDepth), \
    glApiMacro(StencilMask), \
    glApiMacro(ColorMask), \
    glApiMacro(DepthMask), \
    glApiMacro(IndexMask), \
    glApiMacro(Accum), \
    glApiMacro(Disable), \
    glApiMacro(Enable), \
    glApiMacro(Finish), \
    glApiMacro(Flush), \
    glApiMacro(PopAttrib), \
    glApiMacro(PushAttrib), \
    glApiMacro(Map1d), \
    glApiMacro(Map1f), \
    glApiMacro(Map2d), \
    glApiMacro(Map2f), \
    glApiMacro(MapGrid1d), \
    glApiMacro(MapGrid1f), \
    glApiMacro(MapGrid2d), \
    glApiMacro(MapGrid2f), \
    glApiMacro(EvalCoord1d), \
    glApiMacro(EvalCoord1dv), \
    glApiMacro(EvalCoord1f), \
    glApiMacro(EvalCoord1fv), \
    glApiMacro(EvalCoord2d), \
    glApiMacro(EvalCoord2dv), \
    glApiMacro(EvalCoord2f), \
    glApiMacro(EvalCoord2fv), \
    glApiMacro(EvalMesh1), \
    glApiMacro(EvalPoint1), \
    glApiMacro(EvalMesh2), \
    glApiMacro(EvalPoint2), \
    glApiMacro(AlphaFunc), \
    glApiMacro(BlendFunc), \
    glApiMacro(LogicOp), \
    glApiMacro(StencilFunc), \
    glApiMacro(StencilOp), \
    glApiMacro(DepthFunc), \
    glApiMacro(PixelZoom), \
    glApiMacro(PixelTransferf), \
    glApiMacro(PixelTransferi), \
    glApiMacro(PixelStoref), \
    glApiMacro(PixelStorei), \
    glApiMacro(PixelMapfv), \
    glApiMacro(PixelMapuiv), \
    glApiMacro(PixelMapusv), \
    glApiMacro(ReadBuffer), \
    glApiMacro(CopyPixels), \
    glApiMacro(ReadPixels), \
    glApiMacro(DrawPixels), \
    glApiMacro(GetBooleanv), \
    glApiMacro(GetClipPlane), \
    glApiMacro(GetDoublev), \
    glApiMacro(GetError), \
    glApiMacro(GetFloatv), \
    glApiMacro(GetIntegerv), \
    glApiMacro(GetLightfv), \
    glApiMacro(GetLightiv), \
    glApiMacro(GetMapdv), \
    glApiMacro(GetMapfv), \
    glApiMacro(GetMapiv), \
    glApiMacro(GetMaterialfv), \
    glApiMacro(GetMaterialiv), \
    glApiMacro(GetPixelMapfv), \
    glApiMacro(GetPixelMapuiv), \
    glApiMacro(GetPixelMapusv), \
    glApiMacro(GetPolygonStipple), \
    glApiMacro(GetString), \
    glApiMacro(GetTexEnvfv), \
    glApiMacro(GetTexEnviv), \
    glApiMacro(GetTexGendv), \
    glApiMacro(GetTexGenfv), \
    glApiMacro(GetTexGeniv), \
    glApiMacro(GetTexImage), \
    glApiMacro(GetTexParameterfv), \
    glApiMacro(GetTexParameteriv), \
    glApiMacro(GetTexLevelParameterfv), \
    glApiMacro(GetTexLevelParameteriv), \
    glApiMacro(IsEnabled), \
    glApiMacro(IsList), \
    glApiMacro(DepthRange), \
    glApiMacro(Frustum), \
    glApiMacro(LoadIdentity), \
    glApiMacro(LoadMatrixf), \
    glApiMacro(LoadMatrixd), \
    glApiMacro(MatrixMode), \
    glApiMacro(MultMatrixf), \
    glApiMacro(MultMatrixd), \
    glApiMacro(Ortho), \
    glApiMacro(PopMatrix), \
    glApiMacro(PushMatrix), \
    glApiMacro(Rotated), \
    glApiMacro(Rotatef), \
    glApiMacro(Scaled), \
    glApiMacro(Scalef), \
    glApiMacro(Translated), \
    glApiMacro(Translatef), \
    glApiMacro(Viewport), \
    /* GL_VERSION_1_1 */ \
    glApiMacro(ArrayElement), \
    glApiMacro(BindTexture), \
    glApiMacro(ColorPointer), \
    glApiMacro(DisableClientState), \
    glApiMacro(DrawArrays), \
    glApiMacro(DrawElements), \
    glApiMacro(EdgeFlagPointer), \
    glApiMacro(EnableClientState), \
    glApiMacro(IndexPointer), \
    glApiMacro(Indexub), \
    glApiMacro(Indexubv), \
    glApiMacro(InterleavedArrays), \
    glApiMacro(NormalPointer), \
    glApiMacro(PolygonOffset), \
    glApiMacro(TexCoordPointer), \
    glApiMacro(VertexPointer), \
    glApiMacro(AreTexturesResident), \
    glApiMacro(CopyTexImage1D), \
    glApiMacro(CopyTexImage2D), \
    glApiMacro(CopyTexSubImage1D), \
    glApiMacro(CopyTexSubImage2D), \
    glApiMacro(DeleteTextures), \
    glApiMacro(GenTextures), \
    glApiMacro(GetPointerv), \
    glApiMacro(IsTexture), \
    glApiMacro(PrioritizeTextures), \
    glApiMacro(TexSubImage1D), \
    glApiMacro(TexSubImage2D), \
    glApiMacro(PopClientAttrib), \
    glApiMacro(PushClientAttrib), \
    /* GL_VERSION_1_2 */ \
    glApiMacro(DrawRangeElements), \
    glApiMacro(TexImage3D), \
    glApiMacro(TexSubImage3D), \
    glApiMacro(CopyTexSubImage3D), \
    /* GL_VERSION_1_3 */ \
    glApiMacro(ActiveTexture), \
    glApiMacro(SampleCoverage), \
    glApiMacro(CompressedTexImage3D), \
    glApiMacro(CompressedTexImage2D), \
    glApiMacro(CompressedTexImage1D), \
    glApiMacro(CompressedTexSubImage3D), \
    glApiMacro(CompressedTexSubImage2D), \
    glApiMacro(CompressedTexSubImage1D), \
    glApiMacro(GetCompressedTexImage), \
    glApiMacro(ClientActiveTexture), \
    glApiMacro(MultiTexCoord1d), \
    glApiMacro(MultiTexCoord1dv), \
    glApiMacro(MultiTexCoord1f), \
    glApiMacro(MultiTexCoord1fv), \
    glApiMacro(MultiTexCoord1i), \
    glApiMacro(MultiTexCoord1iv), \
    glApiMacro(MultiTexCoord1s), \
    glApiMacro(MultiTexCoord1sv), \
    glApiMacro(MultiTexCoord2d), \
    glApiMacro(MultiTexCoord2dv), \
    glApiMacro(MultiTexCoord2f), \
    glApiMacro(MultiTexCoord2fv), \
    glApiMacro(MultiTexCoord2i), \
    glApiMacro(MultiTexCoord2iv), \
    glApiMacro(MultiTexCoord2s), \
    glApiMacro(MultiTexCoord2sv), \
    glApiMacro(MultiTexCoord3d), \
    glApiMacro(MultiTexCoord3dv), \
    glApiMacro(MultiTexCoord3f), \
    glApiMacro(MultiTexCoord3fv), \
    glApiMacro(MultiTexCoord3i), \
    glApiMacro(MultiTexCoord3iv), \
    glApiMacro(MultiTexCoord3s), \
    glApiMacro(MultiTexCoord3sv), \
    glApiMacro(MultiTexCoord4d), \
    glApiMacro(MultiTexCoord4dv), \
    glApiMacro(MultiTexCoord4f), \
    glApiMacro(MultiTexCoord4fv), \
    glApiMacro(MultiTexCoord4i), \
    glApiMacro(MultiTexCoord4iv), \
    glApiMacro(MultiTexCoord4s), \
    glApiMacro(MultiTexCoord4sv), \
    glApiMacro(LoadTransposeMatrixf), \
    glApiMacro(LoadTransposeMatrixd), \
    glApiMacro(MultTransposeMatrixf), \
    glApiMacro(MultTransposeMatrixd), \
    /* GL_VERSION_1_4 */ \
    glApiMacro(BlendFuncSeparate), \
    glApiMacro(MultiDrawArrays), \
    glApiMacro(MultiDrawElements), \
    glApiMacro(PointParameterf), \
    glApiMacro(PointParameterfv), \
    glApiMacro(PointParameteri), \
    glApiMacro(PointParameteriv), \
    glApiMacro(FogCoordf), \
    glApiMacro(FogCoordfv), \
    glApiMacro(FogCoordd), \
    glApiMacro(FogCoorddv), \
    glApiMacro(FogCoordPointer), \
    glApiMacro(SecondaryColor3b), \
    glApiMacro(SecondaryColor3bv), \
    glApiMacro(SecondaryColor3d), \
    glApiMacro(SecondaryColor3dv), \
    glApiMacro(SecondaryColor3f), \
    glApiMacro(SecondaryColor3fv), \
    glApiMacro(SecondaryColor3i), \
    glApiMacro(SecondaryColor3iv), \
    glApiMacro(SecondaryColor3s), \
    glApiMacro(SecondaryColor3sv), \
    glApiMacro(SecondaryColor3ub), \
    glApiMacro(SecondaryColor3ubv), \
    glApiMacro(SecondaryColor3ui), \
    glApiMacro(SecondaryColor3uiv), \
    glApiMacro(SecondaryColor3us), \
    glApiMacro(SecondaryColor3usv), \
    glApiMacro(SecondaryColorPointer), \
    glApiMacro(WindowPos2d), \
    glApiMacro(WindowPos2dv), \
    glApiMacro(WindowPos2f), \
    glApiMacro(WindowPos2fv), \
    glApiMacro(WindowPos2i), \
    glApiMacro(WindowPos2iv), \
    glApiMacro(WindowPos2s), \
    glApiMacro(WindowPos2sv), \
    glApiMacro(WindowPos3d), \
    glApiMacro(WindowPos3dv), \
    glApiMacro(WindowPos3f), \
    glApiMacro(WindowPos3fv), \
    glApiMacro(WindowPos3i), \
    glApiMacro(WindowPos3iv), \
    glApiMacro(WindowPos3s), \
    glApiMacro(WindowPos3sv), \
    glApiMacro(BlendColor), \
    glApiMacro(BlendEquation), \
    /* GL_VERSION_1_5 */ \
    glApiMacro(GenQueries), \
    glApiMacro(DeleteQueries), \
    glApiMacro(IsQuery), \
    glApiMacro(BeginQuery), \
    glApiMacro(EndQuery), \
    glApiMacro(GetQueryiv), \
    glApiMacro(GetQueryObjectiv), \
    glApiMacro(GetQueryObjectuiv), \
    glApiMacro(BindBuffer), \
    glApiMacro(DeleteBuffers), \
    glApiMacro(GenBuffers), \
    glApiMacro(IsBuffer), \
    glApiMacro(BufferData), \
    glApiMacro(BufferSubData), \
    glApiMacro(GetBufferSubData), \
    glApiMacro(MapBuffer), \
    glApiMacro(UnmapBuffer), \
    glApiMacro(GetBufferParameteriv), \
    glApiMacro(GetBufferPointerv), \
    /* GL_VERSION_2_0 */ \
    glApiMacro(BlendEquationSeparate), \
    glApiMacro(DrawBuffers), \
    glApiMacro(StencilOpSeparate), \
    glApiMacro(StencilFuncSeparate), \
    glApiMacro(StencilMaskSeparate), \
    glApiMacro(AttachShader), \
    glApiMacro(BindAttribLocation), \
    glApiMacro(CompileShader), \
    glApiMacro(CreateProgram), \
    glApiMacro(CreateShader), \
    glApiMacro(DeleteProgram), \
    glApiMacro(DeleteShader), \
    glApiMacro(DetachShader), \
    glApiMacro(DisableVertexAttribArray), \
    glApiMacro(EnableVertexAttribArray), \
    glApiMacro(GetActiveAttrib), \
    glApiMacro(GetActiveUniform), \
    glApiMacro(GetAttachedShaders), \
    glApiMacro(GetAttribLocation), \
    glApiMacro(GetProgramiv), \
    glApiMacro(GetProgramInfoLog), \
    glApiMacro(GetShaderiv), \
    glApiMacro(GetShaderInfoLog), \
    glApiMacro(GetShaderSource), \
    glApiMacro(GetUniformLocation), \
    glApiMacro(GetUniformfv), \
    glApiMacro(GetUniformiv), \
    glApiMacro(GetVertexAttribdv), \
    glApiMacro(GetVertexAttribfv), \
    glApiMacro(GetVertexAttribiv), \
    glApiMacro(GetVertexAttribPointerv), \
    glApiMacro(IsProgram), \
    glApiMacro(IsShader), \
    glApiMacro(LinkProgram), \
    glApiMacro(ShaderSource), \
    glApiMacro(UseProgram), \
    glApiMacro(Uniform1f), \
    glApiMacro(Uniform2f), \
    glApiMacro(Uniform3f), \
    glApiMacro(Uniform4f), \
    glApiMacro(Uniform1i), \
    glApiMacro(Uniform2i), \
    glApiMacro(Uniform3i), \
    glApiMacro(Uniform4i), \
    glApiMacro(Uniform1fv), \
    glApiMacro(Uniform2fv), \
    glApiMacro(Uniform3fv), \
    glApiMacro(Uniform4fv), \
    glApiMacro(Uniform1iv), \
    glApiMacro(Uniform2iv), \
    glApiMacro(Uniform3iv), \
    glApiMacro(Uniform4iv), \
    glApiMacro(UniformMatrix2fv), \
    glApiMacro(UniformMatrix3fv), \
    glApiMacro(UniformMatrix4fv), \
    glApiMacro(ValidateProgram), \
    glApiMacro(VertexAttrib1d), \
    glApiMacro(VertexAttrib1dv), \
    glApiMacro(VertexAttrib1f), \
    glApiMacro(VertexAttrib1fv), \
    glApiMacro(VertexAttrib1s), \
    glApiMacro(VertexAttrib1sv), \
    glApiMacro(VertexAttrib2d), \
    glApiMacro(VertexAttrib2dv), \
    glApiMacro(VertexAttrib2f), \
    glApiMacro(VertexAttrib2fv), \
    glApiMacro(VertexAttrib2s), \
    glApiMacro(VertexAttrib2sv), \
    glApiMacro(VertexAttrib3d), \
    glApiMacro(VertexAttrib3dv), \
    glApiMacro(VertexAttrib3f), \
    glApiMacro(VertexAttrib3fv), \
    glApiMacro(VertexAttrib3s), \
    glApiMacro(VertexAttrib3sv), \
    glApiMacro(VertexAttrib4Nbv), \
    glApiMacro(VertexAttrib4Niv), \
    glApiMacro(VertexAttrib4Nsv), \
    glApiMacro(VertexAttrib4Nub), \
    glApiMacro(VertexAttrib4Nubv), \
    glApiMacro(VertexAttrib4Nuiv), \
    glApiMacro(VertexAttrib4Nusv), \
    glApiMacro(VertexAttrib4bv), \
    glApiMacro(VertexAttrib4d), \
    glApiMacro(VertexAttrib4dv), \
    glApiMacro(VertexAttrib4f), \
    glApiMacro(VertexAttrib4fv), \
    glApiMacro(VertexAttrib4iv), \
    glApiMacro(VertexAttrib4s), \
    glApiMacro(VertexAttrib4sv), \
    glApiMacro(VertexAttrib4ubv), \
    glApiMacro(VertexAttrib4uiv), \
    glApiMacro(VertexAttrib4usv), \
    glApiMacro(VertexAttribPointer), \
    /* GL_VERSION_2_1 */ \
    glApiMacro(UniformMatrix2x3fv), \
    glApiMacro(UniformMatrix3x2fv), \
    glApiMacro(UniformMatrix2x4fv), \
    glApiMacro(UniformMatrix4x2fv), \
    glApiMacro(UniformMatrix3x4fv), \
    glApiMacro(UniformMatrix4x3fv), \
    /* GL_VERSION_3_0 */ \
    glApiMacro(ColorMaski), \
    glApiMacro(GetBooleani_v), \
    glApiMacro(GetIntegeri_v), \
    glApiMacro(Enablei), \
    glApiMacro(Disablei), \
    glApiMacro(IsEnabledi), \
    glApiMacro(BeginTransformFeedback), \
    glApiMacro(EndTransformFeedback), \
    glApiMacro(BindBufferRange), \
    glApiMacro(BindBufferBase), \
    glApiMacro(TransformFeedbackVaryings), \
    glApiMacro(GetTransformFeedbackVarying), \
    glApiMacro(ClampColor), \
    glApiMacro(BeginConditionalRender), \
    glApiMacro(EndConditionalRender), \
    glApiMacro(VertexAttribIPointer), \
    glApiMacro(GetVertexAttribIiv), \
    glApiMacro(GetVertexAttribIuiv), \
    glApiMacro(VertexAttribI1i), \
    glApiMacro(VertexAttribI2i), \
    glApiMacro(VertexAttribI3i), \
    glApiMacro(VertexAttribI4i), \
    glApiMacro(VertexAttribI1ui), \
    glApiMacro(VertexAttribI2ui), \
    glApiMacro(VertexAttribI3ui), \
    glApiMacro(VertexAttribI4ui), \
    glApiMacro(VertexAttribI1iv), \
    glApiMacro(VertexAttribI2iv), \
    glApiMacro(VertexAttribI3iv), \
    glApiMacro(VertexAttribI4iv), \
    glApiMacro(VertexAttribI1uiv), \
    glApiMacro(VertexAttribI2uiv), \
    glApiMacro(VertexAttribI3uiv), \
    glApiMacro(VertexAttribI4uiv), \
    glApiMacro(VertexAttribI4bv), \
    glApiMacro(VertexAttribI4sv), \
    glApiMacro(VertexAttribI4ubv), \
    glApiMacro(VertexAttribI4usv), \
    glApiMacro(GetUniformuiv), \
    glApiMacro(BindFragDataLocation), \
    glApiMacro(GetFragDataLocation), \
    glApiMacro(Uniform1ui), \
    glApiMacro(Uniform2ui), \
    glApiMacro(Uniform3ui), \
    glApiMacro(Uniform4ui), \
    glApiMacro(Uniform1uiv), \
    glApiMacro(Uniform2uiv), \
    glApiMacro(Uniform3uiv), \
    glApiMacro(Uniform4uiv), \
    glApiMacro(TexParameterIiv), \
    glApiMacro(TexParameterIuiv), \
    glApiMacro(GetTexParameterIiv), \
    glApiMacro(GetTexParameterIuiv), \
    glApiMacro(ClearBufferiv), \
    glApiMacro(ClearBufferuiv), \
    glApiMacro(ClearBufferfv), \
    glApiMacro(ClearBufferfi), \
    glApiMacro(GetStringi), \
    glApiMacro(IsRenderbuffer), \
    glApiMacro(BindRenderbuffer), \
    glApiMacro(DeleteRenderbuffers), \
    glApiMacro(GenRenderbuffers), \
    glApiMacro(RenderbufferStorage), \
    glApiMacro(GetRenderbufferParameteriv), \
    glApiMacro(IsFramebuffer), \
    glApiMacro(BindFramebuffer), \
    glApiMacro(DeleteFramebuffers), \
    glApiMacro(GenFramebuffers), \
    glApiMacro(CheckFramebufferStatus), \
    glApiMacro(FramebufferTexture1D), \
    glApiMacro(FramebufferTexture2D), \
    glApiMacro(FramebufferTexture3D), \
    glApiMacro(FramebufferRenderbuffer), \
    glApiMacro(GetFramebufferAttachmentParameteriv), \
    glApiMacro(GenerateMipmap), \
    glApiMacro(BlitFramebuffer), \
    glApiMacro(RenderbufferStorageMultisample), \
    glApiMacro(FramebufferTextureLayer), \
    glApiMacro(MapBufferRange), \
    glApiMacro(FlushMappedBufferRange), \
    glApiMacro(BindVertexArray), \
    glApiMacro(DeleteVertexArrays), \
    glApiMacro(GenVertexArrays), \
    glApiMacro(IsVertexArray), \
    /* GL_VERSION_3_1 */ \
    glApiMacro(DrawArraysInstanced), \
    glApiMacro(DrawElementsInstanced), \
    glApiMacro(TexBuffer), \
    glApiMacro(PrimitiveRestartIndex), \
    glApiMacro(CopyBufferSubData), \
    glApiMacro(GetUniformIndices), \
    glApiMacro(GetActiveUniformsiv), \
    glApiMacro(GetActiveUniformName), \
    glApiMacro(GetUniformBlockIndex), \
    glApiMacro(GetActiveUniformBlockiv), \
    glApiMacro(GetActiveUniformBlockName), \
    glApiMacro(UniformBlockBinding), \
    /* GL_VERSION_3_2 */ \
    glApiMacro(DrawElementsBaseVertex), \
    glApiMacro(DrawRangeElementsBaseVertex), \
    glApiMacro(DrawElementsInstancedBaseVertex), \
    glApiMacro(MultiDrawElementsBaseVertex), \
    glApiMacro(ProvokingVertex), \
    glApiMacro(FenceSync), \
    glApiMacro(IsSync), \
    glApiMacro(DeleteSync), \
    glApiMacro(ClientWaitSync), \
    glApiMacro(WaitSync), \
    glApiMacro(GetInteger64v), \
    glApiMacro(GetSynciv), \
    glApiMacro(GetInteger64i_v), \
    glApiMacro(GetBufferParameteri64v), \
    glApiMacro(FramebufferTexture), \
    glApiMacro(TexImage2DMultisample), \
    glApiMacro(TexImage3DMultisample), \
    glApiMacro(GetMultisamplefv), \
    glApiMacro(SampleMaski), \
    /* GL_VERSION_3_3 */ \
    glApiMacro(BindFragDataLocationIndexed), \
    glApiMacro(GetFragDataIndex), \
    glApiMacro(GenSamplers), \
    glApiMacro(DeleteSamplers), \
    glApiMacro(IsSampler), \
    glApiMacro(BindSampler), \
    glApiMacro(SamplerParameteri), \
    glApiMacro(SamplerParameteriv), \
    glApiMacro(SamplerParameterf), \
    glApiMacro(SamplerParameterfv), \
    glApiMacro(SamplerParameterIiv), \
    glApiMacro(SamplerParameterIuiv), \
    glApiMacro(GetSamplerParameteriv), \
    glApiMacro(GetSamplerParameterIiv), \
    glApiMacro(GetSamplerParameterfv), \
    glApiMacro(GetSamplerParameterIuiv), \
    glApiMacro(QueryCounter), \
    glApiMacro(GetQueryObjecti64v), \
    glApiMacro(GetQueryObjectui64v), \
    glApiMacro(VertexAttribDivisor), \
    glApiMacro(VertexAttribP1ui), \
    glApiMacro(VertexAttribP1uiv), \
    glApiMacro(VertexAttribP2ui), \
    glApiMacro(VertexAttribP2uiv), \
    glApiMacro(VertexAttribP3ui), \
    glApiMacro(VertexAttribP3uiv), \
    glApiMacro(VertexAttribP4ui), \
    glApiMacro(VertexAttribP4uiv), \
    glApiMacro(VertexP2ui), \
    glApiMacro(VertexP2uiv), \
    glApiMacro(VertexP3ui), \
    glApiMacro(VertexP3uiv), \
    glApiMacro(VertexP4ui), \
    glApiMacro(VertexP4uiv), \
    glApiMacro(TexCoordP1ui), \
    glApiMacro(TexCoordP1uiv), \
    glApiMacro(TexCoordP2ui), \
    glApiMacro(TexCoordP2uiv), \
    glApiMacro(TexCoordP3ui), \
    glApiMacro(TexCoordP3uiv), \
    glApiMacro(TexCoordP4ui), \
    glApiMacro(TexCoordP4uiv), \
    glApiMacro(MultiTexCoordP1ui), \
    glApiMacro(MultiTexCoordP1uiv), \
    glApiMacro(MultiTexCoordP2ui), \
    glApiMacro(MultiTexCoordP2uiv), \
    glApiMacro(MultiTexCoordP3ui), \
    glApiMacro(MultiTexCoordP3uiv), \
    glApiMacro(MultiTexCoordP4ui), \
    glApiMacro(MultiTexCoordP4uiv), \
    glApiMacro(NormalP3ui), \
    glApiMacro(NormalP3uiv), \
    glApiMacro(ColorP3ui), \
    glApiMacro(ColorP3uiv), \
    glApiMacro(ColorP4ui), \
    glApiMacro(ColorP4uiv), \
    glApiMacro(SecondaryColorP3ui), \
    glApiMacro(SecondaryColorP3uiv), \
    /* GL_VERSION_4_0 */ \
    glApiMacro(MinSampleShading), \
    glApiMacro(BlendEquationi), \
    glApiMacro(BlendEquationSeparatei), \
    glApiMacro(BlendFunci), \
    glApiMacro(BlendFuncSeparatei), \
    glApiMacro(DrawArraysIndirect), \
    glApiMacro(DrawElementsIndirect), \
    glApiMacro(Uniform1d), \
    glApiMacro(Uniform2d), \
    glApiMacro(Uniform3d), \
    glApiMacro(Uniform4d), \
    glApiMacro(Uniform1dv), \
    glApiMacro(Uniform2dv), \
    glApiMacro(Uniform3dv), \
    glApiMacro(Uniform4dv), \
    glApiMacro(UniformMatrix2dv), \
    glApiMacro(UniformMatrix3dv), \
    glApiMacro(UniformMatrix4dv), \
    glApiMacro(UniformMatrix2x3dv), \
    glApiMacro(UniformMatrix2x4dv), \
    glApiMacro(UniformMatrix3x2dv), \
    glApiMacro(UniformMatrix3x4dv), \
    glApiMacro(UniformMatrix4x2dv), \
    glApiMacro(UniformMatrix4x3dv), \
    glApiMacro(GetUniformdv), \
    glApiMacro(GetSubroutineUniformLocation), \
    glApiMacro(GetSubroutineIndex), \
    glApiMacro(GetActiveSubroutineUniformiv), \
    glApiMacro(GetActiveSubroutineUniformName), \
    glApiMacro(GetActiveSubroutineName), \
    glApiMacro(UniformSubroutinesuiv), \
    glApiMacro(GetUniformSubroutineuiv), \
    glApiMacro(GetProgramStageiv), \
    glApiMacro(PatchParameteri), \
    glApiMacro(PatchParameterfv), \
    glApiMacro(BindTransformFeedback), \
    glApiMacro(DeleteTransformFeedbacks), \
    glApiMacro(GenTransformFeedbacks), \
    glApiMacro(IsTransformFeedback), \
    glApiMacro(PauseTransformFeedback), \
    glApiMacro(ResumeTransformFeedback), \
    glApiMacro(DrawTransformFeedback), \
    glApiMacro(DrawTransformFeedbackStream), \
    glApiMacro(BeginQueryIndexed), \
    glApiMacro(EndQueryIndexed), \
    glApiMacro(GetQueryIndexediv), \
    /* GL_VERSION_4_1, incomplete: defined by later GL version but required by ES */ \
    glApiMacro(ReleaseShaderCompiler), \
    glApiMacro(ShaderBinary), \
    glApiMacro(GetShaderPrecisionFormat), \
    glApiMacro(DepthRangef), \
    glApiMacro(ClearDepthf), \
    glApiMacro(GetProgramBinary), \
    glApiMacro(ProgramBinary), \
    glApiMacro(ProgramParameteri), \
    glApiMacro(UseProgramStages), \
    glApiMacro(ActiveShaderProgram), \
    glApiMacro(CreateShaderProgramv), \
    glApiMacro(BindProgramPipeline), \
    glApiMacro(DeleteProgramPipelines), \
    glApiMacro(GenProgramPipelines), \
    glApiMacro(IsProgramPipeline), \
    glApiMacro(GetProgramPipelineiv), \
    glApiMacro(ProgramUniform1i), \
    glApiMacro(ProgramUniform1iv), \
    glApiMacro(ProgramUniform1f), \
    glApiMacro(ProgramUniform1fv), \
    glApiMacro(ProgramUniform1ui), \
    glApiMacro(ProgramUniform1uiv), \
    glApiMacro(ProgramUniform2i), \
    glApiMacro(ProgramUniform2iv), \
    glApiMacro(ProgramUniform2f), \
    glApiMacro(ProgramUniform2fv), \
    glApiMacro(ProgramUniform2ui), \
    glApiMacro(ProgramUniform2uiv), \
    glApiMacro(ProgramUniform3i), \
    glApiMacro(ProgramUniform3iv), \
    glApiMacro(ProgramUniform3f), \
    glApiMacro(ProgramUniform3fv), \
    glApiMacro(ProgramUniform3ui), \
    glApiMacro(ProgramUniform3uiv), \
    glApiMacro(ProgramUniform4i), \
    glApiMacro(ProgramUniform4iv), \
    glApiMacro(ProgramUniform4f), \
    glApiMacro(ProgramUniform4fv), \
    glApiMacro(ProgramUniform4ui), \
    glApiMacro(ProgramUniform4uiv), \
    glApiMacro(ProgramUniformMatrix2fv), \
    glApiMacro(ProgramUniformMatrix3fv), \
    glApiMacro(ProgramUniformMatrix4fv), \
    glApiMacro(ProgramUniformMatrix2x3fv), \
    glApiMacro(ProgramUniformMatrix3x2fv), \
    glApiMacro(ProgramUniformMatrix2x4fv), \
    glApiMacro(ProgramUniformMatrix4x2fv), \
    glApiMacro(ProgramUniformMatrix3x4fv), \
    glApiMacro(ProgramUniformMatrix4x3fv), \
    glApiMacro(ValidateProgramPipeline), \
    glApiMacro(GetProgramPipelineInfoLog), \
    /* GL_VERSION_4_2, incomplete: defined by later GL version but required by ES */ \
    glApiMacro(GetInternalformativ), \
    glApiMacro(BindImageTexture), \
    glApiMacro(MemoryBarrier), \
    glApiMacro(TexStorage2D), \
    glApiMacro(TexStorage3D), \
    /* GL_VERSION_4_3, incomplete: defined by later GL version but required by ES */ \
    glApiMacro(DispatchCompute), \
    glApiMacro(DispatchComputeIndirect), \
    glApiMacro(CopyImageSubData), \
    glApiMacro(FramebufferParameteri), \
    glApiMacro(GetFramebufferParameteriv), \
    glApiMacro(InvalidateFramebuffer), \
    glApiMacro(InvalidateSubFramebuffer), \
    glApiMacro(MultiDrawArraysIndirect), \
    glApiMacro(MultiDrawElementsIndirect), \
    glApiMacro(GetProgramInterfaceiv), \
    glApiMacro(GetProgramResourceIndex), \
    glApiMacro(GetProgramResourceName), \
    glApiMacro(GetProgramResourceiv), \
    glApiMacro(GetProgramResourceLocation), \
    glApiMacro(TexBufferRange), \
    glApiMacro(TexStorage2DMultisample), \
    glApiMacro(TexStorage3DMultisample), \
    glApiMacro(BindVertexBuffer), \
    glApiMacro(VertexAttribFormat), \
    glApiMacro(VertexAttribIFormat), \
    glApiMacro(VertexAttribBinding), \
    glApiMacro(VertexBindingDivisor), \
    glApiMacro(DebugMessageControl), \
    glApiMacro(DebugMessageInsert), \
    glApiMacro(DebugMessageCallback), \
    glApiMacro(GetDebugMessageLog), \
    glApiMacro(PushDebugGroup), \
    glApiMacro(PopDebugGroup), \
    glApiMacro(ObjectLabel), \
    glApiMacro(GetObjectLabel), \
    glApiMacro(ObjectPtrLabel), \
    glApiMacro(GetObjectPtrLabel), \
    /* GL_VERSION_4_5, incomplete: defined by later GL version but required by ES */ \
    glApiMacro(MemoryBarrierByRegion), \
    glApiMacro(GetGraphicsResetStatus), \
    glApiMacro(ReadnPixels), \
    glApiMacro(GetnUniformfv), \
    glApiMacro(GetnUniformiv), \
    glApiMacro(GetnUniformuiv), \
    /* OpenGL ES extensions */ \
    /* ES_VERSION_3_2 */ \
    glApiMacro(BlendBarrier), \
    glApiMacro(PrimitiveBoundingBox), \
    /* GL_OES_EGL_image */ \
    glApiMacro(EGLImageTargetTexture2DOES), \
    glApiMacro(EGLImageTargetRenderbufferStorageOES), \
    /* GL_VIV_direct_texture */ \
    glApiMacro(TexDirectVIV), \
    glApiMacro(TexDirectInvalidateVIV), \
    glApiMacro(TexDirectVIVMap), \
    glApiMacro(TexDirectTiledMapVIV), \
    /* GL_EXT_multisampled_render_to_texture */ \
    glApiMacro(FramebufferTexture2DMultisampleEXT), \
    /* GL_EXT_discard_framebuffer */ \
    glApiMacro(DiscardFramebufferEXT), \
    /* GL_ARB_shader_objects */ \
    glApiMacro(DeleteObjectARB), \
    glApiMacro(GetObjectParameterivARB), \
    glApiMacro(GetInfoLogARB),


#ifndef VIV_EGL_BUILD

#define __GL_LISTEXEC_ENTRIES(glApiMacro, _initstr_) \
    glApiMacro(Skip) _initstr_, \
    glApiMacro(InvalidValue), \
    glApiMacro(InvalidEnum), \
    glApiMacro(InvalidOperation), \
    glApiMacro(TableTooLarge), \
    glApiMacro(Primitive), \
    glApiMacro(CallList), \
    glApiMacro(CallLists), \
    glApiMacro(ListBase), \
    glApiMacro(Begin), \
    glApiMacro(Bitmap), \
    glApiMacro(Color3fv), \
    glApiMacro(Color4fv), \
    glApiMacro(Color4ubv), \
    glApiMacro(EdgeFlag), \
    glApiMacro(End), \
    glApiMacro(Indexf), \
    glApiMacro(Normal3fv), \
    glApiMacro(RasterPos2fv), \
    glApiMacro(RasterPos3fv), \
    glApiMacro(RasterPos4fv), \
    glApiMacro(Rectf), \
    glApiMacro(TexCoord2fv), \
    glApiMacro(TexCoord3fv), \
    glApiMacro(TexCoord4fv), \
    glApiMacro(Vertex2fv), \
    glApiMacro(Vertex3fv), \
    glApiMacro(Vertex4fv), \
    glApiMacro(ClipPlane), \
    glApiMacro(ColorMaterial), \
    glApiMacro(CullFace), \
    glApiMacro(Fogfv), \
    glApiMacro(Fogiv), \
    glApiMacro(FrontFace), \
    glApiMacro(Hint), \
    glApiMacro(Lightfv), \
    glApiMacro(Lightiv), \
    glApiMacro(LightModelfv), \
    glApiMacro(LightModeliv), \
    glApiMacro(LineStipple), \
    glApiMacro(LineWidth), \
    glApiMacro(Materialfv), \
    glApiMacro(Materialiv), \
    glApiMacro(PointSize), \
    glApiMacro(PolygonMode), \
    glApiMacro(PolygonStipple), \
    glApiMacro(Scissor), \
    glApiMacro(ShadeModel), \
    glApiMacro(TexParameterfv), \
    glApiMacro(TexParameteriv), \
    glApiMacro(TexImage1D), \
    glApiMacro(TexImage2D), \
    glApiMacro(TexEnvfv), \
    glApiMacro(TexEnviv), \
    glApiMacro(TexGendv), \
    glApiMacro(TexGenfv), \
    glApiMacro(TexGeniv), \
    glApiMacro(InitNames), \
    glApiMacro(LoadName), \
    glApiMacro(PassThrough), \
    glApiMacro(PopName), \
    glApiMacro(PushName), \
    glApiMacro(DrawBuffer), \
    glApiMacro(Clear), \
    glApiMacro(ClearAccum), \
    glApiMacro(ClearIndex), \
    glApiMacro(ClearColor), \
    glApiMacro(ClearStencil), \
    glApiMacro(ClearDepth), \
    glApiMacro(StencilMask), \
    glApiMacro(ColorMask), \
    glApiMacro(DepthMask), \
    glApiMacro(IndexMask), \
    glApiMacro(Accum), \
    glApiMacro(Disable), \
    glApiMacro(Enable), \
    glApiMacro(PopAttrib), \
    glApiMacro(PushAttrib), \
    glApiMacro(Map1d), \
    glApiMacro(Map1f), \
    glApiMacro(Map2d), \
    glApiMacro(Map2f), \
    glApiMacro(MapGrid1d), \
    glApiMacro(MapGrid1f), \
    glApiMacro(MapGrid2d), \
    glApiMacro(MapGrid2f), \
    glApiMacro(EvalCoord1dv), \
    glApiMacro(EvalCoord1fv), \
    glApiMacro(EvalCoord2dv), \
    glApiMacro(EvalCoord2fv), \
    glApiMacro(EvalMesh1), \
    glApiMacro(EvalPoint1), \
    glApiMacro(EvalMesh2), \
    glApiMacro(EvalPoint2), \
    glApiMacro(AlphaFunc), \
    glApiMacro(BlendFunc), \
    glApiMacro(LogicOp), \
    glApiMacro(StencilFunc), \
    glApiMacro(StencilOp), \
    glApiMacro(DepthFunc), \
    glApiMacro(PixelZoom), \
    glApiMacro(PixelTransferf), \
    glApiMacro(PixelTransferi), \
    glApiMacro(PixelMapfv), \
    glApiMacro(PixelMapuiv), \
    glApiMacro(PixelMapusv), \
    glApiMacro(ReadBuffer), \
    glApiMacro(CopyPixels), \
    glApiMacro(DrawPixels), \
    glApiMacro(DepthRange), \
    glApiMacro(Frustum), \
    glApiMacro(LoadIdentity), \
    glApiMacro(LoadMatrixf), \
    glApiMacro(LoadMatrixd), \
    glApiMacro(MatrixMode), \
    glApiMacro(MultMatrixf), \
    glApiMacro(MultMatrixd), \
    glApiMacro(Ortho), \
    glApiMacro(PopMatrix), \
    glApiMacro(PushMatrix), \
    glApiMacro(Rotated), \
    glApiMacro(Rotatef), \
    glApiMacro(Scaled), \
    glApiMacro(Scalef), \
    glApiMacro(Translated), \
    glApiMacro(Translatef), \
    glApiMacro(Viewport), \
    /* GL_VERSION_1_1 */ \
    glApiMacro(BindTexture), \
    glApiMacro(PolygonOffset), \
    glApiMacro(CopyTexImage1D), \
    glApiMacro(CopyTexImage2D), \
    glApiMacro(CopyTexSubImage1D), \
    glApiMacro(CopyTexSubImage2D), \
    glApiMacro(PrioritizeTextures), \
    glApiMacro(TexSubImage1D), \
    glApiMacro(TexSubImage2D), \
    /* GL_VERSION_1_2 */ \
    glApiMacro(TexImage3D), \
    glApiMacro(TexSubImage3D), \
    glApiMacro(CopyTexSubImage3D), \
    /* GL_VERSION_1_3 */ \
    glApiMacro(ActiveTexture), \
    glApiMacro(MultiTexCoord2fv), \
    glApiMacro(MultiTexCoord3fv), \
    glApiMacro(MultiTexCoord4fv), \
    glApiMacro(LoadTransposeMatrixf), \
    glApiMacro(LoadTransposeMatrixd), \
    glApiMacro(MultTransposeMatrixf), \
    glApiMacro(MultTransposeMatrixd), \
    glApiMacro(SampleCoverage), \
    glApiMacro(CompressedTexImage3D), \
    glApiMacro(CompressedTexImage2D), \
    glApiMacro(CompressedTexImage1D), \
    glApiMacro(CompressedTexSubImage3D), \
    glApiMacro(CompressedTexSubImage2D), \
    glApiMacro(CompressedTexSubImage1D), \
    /* GL_VERSION_1_4 */ \
    glApiMacro(BlendFuncSeparate), \
    glApiMacro(FogCoordf), \
    glApiMacro(PointParameterfv), \
    glApiMacro(PointParameteriv), \
    glApiMacro(SecondaryColor3fv), \
    glApiMacro(WindowPos2fv), \
    glApiMacro(WindowPos3fv), \
    glApiMacro(BlendColor), \
    glApiMacro(BlendEquation), \
    /* GL_VERSION_1_5 */ \
    glApiMacro(BeginQuery), \
    glApiMacro(EndQuery), \
    /* GL_VERSION_2_0 */ \
    glApiMacro(BlendEquationSeparate), \
    glApiMacro(DrawBuffers), \
    glApiMacro(StencilOpSeparate), \
    glApiMacro(StencilFuncSeparate), \
    glApiMacro(StencilMaskSeparate), \
    glApiMacro(UseProgram), \
    glApiMacro(Uniform1f), \
    glApiMacro(Uniform2f), \
    glApiMacro(Uniform3f), \
    glApiMacro(Uniform4f), \
    glApiMacro(Uniform1i), \
    glApiMacro(Uniform2i), \
    glApiMacro(Uniform3i), \
    glApiMacro(Uniform4i), \
    glApiMacro(Uniform1fv), \
    glApiMacro(Uniform2fv), \
    glApiMacro(Uniform3fv), \
    glApiMacro(Uniform4fv), \
    glApiMacro(Uniform1iv), \
    glApiMacro(Uniform2iv), \
    glApiMacro(Uniform3iv), \
    glApiMacro(Uniform4iv), \
    glApiMacro(UniformMatrix2fv), \
    glApiMacro(UniformMatrix3fv), \
    glApiMacro(UniformMatrix4fv), \
    glApiMacro(VertexAttrib4fv), \
    /* GL_VERSION_2_1 */ \
    glApiMacro(UniformMatrix2x3fv), \
    glApiMacro(UniformMatrix2x4fv), \
    glApiMacro(UniformMatrix3x2fv), \
    glApiMacro(UniformMatrix3x4fv), \
    glApiMacro(UniformMatrix4x2fv), \
    glApiMacro(UniformMatrix4x3fv), \



/* Enum table for __glop_FuncName (Ex: __glop_NewList)
 */
#define glop(name) __glop_##name
enum {
    __GL_LISTEXEC_ENTRIES(glop,=0)
    __glop_PrimContinue
};

#define __GL_API_DISPATCH_FUNCS(glApiMacro) \
    GLvoid         glApiMacro(NewList) (_gcArgComma_  GLuint, GLenum); \
    GLvoid         glApiMacro(EndList) (_gcArgOnly_); \
    GLvoid         glApiMacro(CallList) (_gcArgComma_  GLuint); \
    GLvoid         glApiMacro(CallLists) (_gcArgComma_  GLsizei, GLenum, const GLvoid *); \
    GLvoid         glApiMacro(DeleteLists) (_gcArgComma_  GLuint, GLsizei); \
    GLuint         glApiMacro(GenLists) (_gcArgComma_  GLsizei); \
    GLvoid         glApiMacro(ListBase) (_gcArgComma_  GLuint); \
    GLvoid         glApiMacro(Begin) (_gcArgComma_  GLenum); \
    GLvoid         glApiMacro(Bitmap) (_gcArgComma_  GLsizei, GLsizei, GLfloat, GLfloat, GLfloat, GLfloat, const GLubyte *); \
    GLvoid         glApiMacro(Color3b) (_gcArgComma_  GLbyte, GLbyte, GLbyte); \
    GLvoid         glApiMacro(Color3bv) (_gcArgComma_  const GLbyte *); \
    GLvoid         glApiMacro(Color3d) (_gcArgComma_  GLdouble, GLdouble, GLdouble); \
    GLvoid         glApiMacro(Color3dv) (_gcArgComma_  const GLdouble *); \
    GLvoid         glApiMacro(Color3f) (_gcArgComma_  GLfloat, GLfloat, GLfloat); \
    GLvoid         glApiMacro(Color3fv) (_gcArgComma_  const GLfloat *); \
    GLvoid         glApiMacro(Color3i) (_gcArgComma_  GLint, GLint, GLint); \
    GLvoid         glApiMacro(Color3iv) (_gcArgComma_  const GLint *); \
    GLvoid         glApiMacro(Color3s) (_gcArgComma_  GLshort, GLshort, GLshort); \
    GLvoid         glApiMacro(Color3sv) (_gcArgComma_  const GLshort *); \
    GLvoid         glApiMacro(Color3ub) (_gcArgComma_  GLubyte, GLubyte, GLubyte); \
    GLvoid         glApiMacro(Color3ubv) (_gcArgComma_  const GLubyte *); \
    GLvoid         glApiMacro(Color3ui) (_gcArgComma_  GLuint, GLuint, GLuint); \
    GLvoid         glApiMacro(Color3uiv) (_gcArgComma_  const GLuint *); \
    GLvoid         glApiMacro(Color3us) (_gcArgComma_  GLushort, GLushort, GLushort); \
    GLvoid         glApiMacro(Color3usv) (_gcArgComma_  const GLushort *); \
    GLvoid         glApiMacro(Color4b) (_gcArgComma_  GLbyte, GLbyte, GLbyte, GLbyte); \
    GLvoid         glApiMacro(Color4bv) (_gcArgComma_  const GLbyte *); \
    GLvoid         glApiMacro(Color4d) (_gcArgComma_  GLdouble, GLdouble, GLdouble, GLdouble); \
    GLvoid         glApiMacro(Color4dv) (_gcArgComma_  const GLdouble *); \
    GLvoid         glApiMacro(Color4f) (_gcArgComma_  GLfloat, GLfloat, GLfloat, GLfloat); \
    GLvoid         glApiMacro(Color4fv) (_gcArgComma_  const GLfloat *); \
    GLvoid         glApiMacro(Color4i) (_gcArgComma_  GLint, GLint, GLint, GLint); \
    GLvoid         glApiMacro(Color4iv) (_gcArgComma_  const GLint *); \
    GLvoid         glApiMacro(Color4s) (_gcArgComma_  GLshort, GLshort, GLshort, GLshort); \
    GLvoid         glApiMacro(Color4sv) (_gcArgComma_  const GLshort *); \
    GLvoid         glApiMacro(Color4ub) (_gcArgComma_  GLubyte, GLubyte, GLubyte, GLubyte); \
    GLvoid         glApiMacro(Color4ubv) (_gcArgComma_  const GLubyte *); \
    GLvoid         glApiMacro(Color4ui) (_gcArgComma_  GLuint, GLuint, GLuint, GLuint); \
    GLvoid         glApiMacro(Color4uiv) (_gcArgComma_  const GLuint *); \
    GLvoid         glApiMacro(Color4us) (_gcArgComma_  GLushort, GLushort, GLushort, GLushort); \
    GLvoid         glApiMacro(Color4usv) (_gcArgComma_  const GLushort *); \
    GLvoid         glApiMacro(EdgeFlag) (_gcArgComma_  GLboolean); \
    GLvoid         glApiMacro(EdgeFlagv) (_gcArgComma_  const GLboolean *); \
    GLvoid         glApiMacro(End) (_gcArgOnly_); \
    GLvoid         glApiMacro(Indexd) (_gcArgComma_  GLdouble); \
    GLvoid         glApiMacro(Indexdv) (_gcArgComma_  const GLdouble *); \
    GLvoid         glApiMacro(Indexf) (_gcArgComma_  GLfloat); \
    GLvoid         glApiMacro(Indexfv) (_gcArgComma_  const GLfloat *); \
    GLvoid         glApiMacro(Indexi) (_gcArgComma_  GLint); \
    GLvoid         glApiMacro(Indexiv) (_gcArgComma_  const GLint *); \
    GLvoid         glApiMacro(Indexs) (_gcArgComma_  GLshort); \
    GLvoid         glApiMacro(Indexsv) (_gcArgComma_  const GLshort *); \
    GLvoid         glApiMacro(Normal3b) (_gcArgComma_  GLbyte, GLbyte, GLbyte); \
    GLvoid         glApiMacro(Normal3bv) (_gcArgComma_  const GLbyte *); \
    GLvoid         glApiMacro(Normal3d) (_gcArgComma_  GLdouble, GLdouble, GLdouble); \
    GLvoid         glApiMacro(Normal3dv) (_gcArgComma_  const GLdouble *); \
    GLvoid         glApiMacro(Normal3f) (_gcArgComma_  GLfloat, GLfloat, GLfloat); \
    GLvoid         glApiMacro(Normal3fv) (_gcArgComma_  const GLfloat *); \
    GLvoid         glApiMacro(Normal3i) (_gcArgComma_  GLint, GLint, GLint); \
    GLvoid         glApiMacro(Normal3iv) (_gcArgComma_  const GLint *); \
    GLvoid         glApiMacro(Normal3s) (_gcArgComma_  GLshort, GLshort, GLshort); \
    GLvoid         glApiMacro(Normal3sv) (_gcArgComma_  const GLshort *); \
    GLvoid         glApiMacro(RasterPos2d) (_gcArgComma_  GLdouble, GLdouble); \
    GLvoid         glApiMacro(RasterPos2dv) (_gcArgComma_  const GLdouble *); \
    GLvoid         glApiMacro(RasterPos2f) (_gcArgComma_  GLfloat, GLfloat); \
    GLvoid         glApiMacro(RasterPos2fv) (_gcArgComma_  const GLfloat *); \
    GLvoid         glApiMacro(RasterPos2i) (_gcArgComma_  GLint, GLint); \
    GLvoid         glApiMacro(RasterPos2iv) (_gcArgComma_  const GLint *); \
    GLvoid         glApiMacro(RasterPos2s) (_gcArgComma_  GLshort, GLshort); \
    GLvoid         glApiMacro(RasterPos2sv) (_gcArgComma_  const GLshort *); \
    GLvoid         glApiMacro(RasterPos3d) (_gcArgComma_  GLdouble, GLdouble, GLdouble); \
    GLvoid         glApiMacro(RasterPos3dv) (_gcArgComma_  const GLdouble *); \
    GLvoid         glApiMacro(RasterPos3f) (_gcArgComma_  GLfloat, GLfloat, GLfloat); \
    GLvoid         glApiMacro(RasterPos3fv) (_gcArgComma_  const GLfloat *); \
    GLvoid         glApiMacro(RasterPos3i) (_gcArgComma_  GLint, GLint, GLint); \
    GLvoid         glApiMacro(RasterPos3iv) (_gcArgComma_  const GLint *); \
    GLvoid         glApiMacro(RasterPos3s) (_gcArgComma_  GLshort, GLshort, GLshort); \
    GLvoid         glApiMacro(RasterPos3sv) (_gcArgComma_  const GLshort *); \
    GLvoid         glApiMacro(RasterPos4d) (_gcArgComma_  GLdouble, GLdouble, GLdouble, GLdouble); \
    GLvoid         glApiMacro(RasterPos4dv) (_gcArgComma_  const GLdouble *); \
    GLvoid         glApiMacro(RasterPos4f) (_gcArgComma_  GLfloat, GLfloat, GLfloat, GLfloat); \
    GLvoid         glApiMacro(RasterPos4fv) (_gcArgComma_  const GLfloat *); \
    GLvoid         glApiMacro(RasterPos4i) (_gcArgComma_  GLint, GLint, GLint, GLint); \
    GLvoid         glApiMacro(RasterPos4iv) (_gcArgComma_  const GLint *); \
    GLvoid         glApiMacro(RasterPos4s) (_gcArgComma_  GLshort, GLshort, GLshort, GLshort); \
    GLvoid         glApiMacro(RasterPos4sv) (_gcArgComma_  const GLshort *); \
    GLvoid         glApiMacro(Rectd) (_gcArgComma_  GLdouble, GLdouble, GLdouble, GLdouble); \
    GLvoid         glApiMacro(Rectdv) (_gcArgComma_  const GLdouble *, const GLdouble *); \
    GLvoid         glApiMacro(Rectf) (_gcArgComma_  GLfloat, GLfloat, GLfloat, GLfloat); \
    GLvoid         glApiMacro(Rectfv) (_gcArgComma_  const GLfloat *, const GLfloat *); \
    GLvoid         glApiMacro(Recti) (_gcArgComma_  GLint, GLint, GLint, GLint); \
    GLvoid         glApiMacro(Rectiv) (_gcArgComma_  const GLint *, const GLint *); \
    GLvoid         glApiMacro(Rects) (_gcArgComma_  GLshort, GLshort, GLshort, GLshort); \
    GLvoid         glApiMacro(Rectsv) (_gcArgComma_  const GLshort *, const GLshort *); \
    GLvoid         glApiMacro(TexCoord1d) (_gcArgComma_  GLdouble); \
    GLvoid         glApiMacro(TexCoord1dv) (_gcArgComma_  const GLdouble *); \
    GLvoid         glApiMacro(TexCoord1f) (_gcArgComma_  GLfloat); \
    GLvoid         glApiMacro(TexCoord1fv) (_gcArgComma_  const GLfloat *); \
    GLvoid         glApiMacro(TexCoord1i) (_gcArgComma_  GLint); \
    GLvoid         glApiMacro(TexCoord1iv) (_gcArgComma_  const GLint *); \
    GLvoid         glApiMacro(TexCoord1s) (_gcArgComma_  GLshort); \
    GLvoid         glApiMacro(TexCoord1sv) (_gcArgComma_  const GLshort *); \
    GLvoid         glApiMacro(TexCoord2d) (_gcArgComma_  GLdouble, GLdouble); \
    GLvoid         glApiMacro(TexCoord2dv) (_gcArgComma_  const GLdouble *); \
    GLvoid         glApiMacro(TexCoord2f) (_gcArgComma_  GLfloat, GLfloat); \
    GLvoid         glApiMacro(TexCoord2fv) (_gcArgComma_  const GLfloat *); \
    GLvoid         glApiMacro(TexCoord2i) (_gcArgComma_  GLint, GLint); \
    GLvoid         glApiMacro(TexCoord2iv) (_gcArgComma_  const GLint *); \
    GLvoid         glApiMacro(TexCoord2s) (_gcArgComma_  GLshort, GLshort); \
    GLvoid         glApiMacro(TexCoord2sv) (_gcArgComma_  const GLshort *); \
    GLvoid         glApiMacro(TexCoord3d) (_gcArgComma_  GLdouble, GLdouble, GLdouble); \
    GLvoid         glApiMacro(TexCoord3dv) (_gcArgComma_  const GLdouble *); \
    GLvoid         glApiMacro(TexCoord3f) (_gcArgComma_  GLfloat, GLfloat, GLfloat); \
    GLvoid         glApiMacro(TexCoord3fv) (_gcArgComma_  const GLfloat *); \
    GLvoid         glApiMacro(TexCoord3i) (_gcArgComma_  GLint, GLint, GLint); \
    GLvoid         glApiMacro(TexCoord3iv) (_gcArgComma_  const GLint *); \
    GLvoid         glApiMacro(TexCoord3s) (_gcArgComma_  GLshort, GLshort, GLshort); \
    GLvoid         glApiMacro(TexCoord3sv) (_gcArgComma_  const GLshort *); \
    GLvoid         glApiMacro(TexCoord4d) (_gcArgComma_  GLdouble, GLdouble, GLdouble, GLdouble); \
    GLvoid         glApiMacro(TexCoord4dv) (_gcArgComma_  const GLdouble *); \
    GLvoid         glApiMacro(TexCoord4f) (_gcArgComma_  GLfloat, GLfloat, GLfloat, GLfloat); \
    GLvoid         glApiMacro(TexCoord4fv) (_gcArgComma_  const GLfloat *); \
    GLvoid         glApiMacro(TexCoord4i) (_gcArgComma_  GLint, GLint, GLint, GLint); \
    GLvoid         glApiMacro(TexCoord4iv) (_gcArgComma_  const GLint *); \
    GLvoid         glApiMacro(TexCoord4s) (_gcArgComma_  GLshort, GLshort, GLshort, GLshort); \
    GLvoid         glApiMacro(TexCoord4sv) (_gcArgComma_  const GLshort *); \
    GLvoid         glApiMacro(Vertex2d) (_gcArgComma_  GLdouble, GLdouble); \
    GLvoid         glApiMacro(Vertex2dv) (_gcArgComma_  const GLdouble *); \
    GLvoid         glApiMacro(Vertex2f) (_gcArgComma_  GLfloat, GLfloat); \
    GLvoid         glApiMacro(Vertex2fv) (_gcArgComma_  const GLfloat *); \
    GLvoid         glApiMacro(Vertex2i) (_gcArgComma_  GLint, GLint); \
    GLvoid         glApiMacro(Vertex2iv) (_gcArgComma_  const GLint *); \
    GLvoid         glApiMacro(Vertex2s) (_gcArgComma_  GLshort, GLshort); \
    GLvoid         glApiMacro(Vertex2sv) (_gcArgComma_  const GLshort *); \
    GLvoid         glApiMacro(Vertex3d) (_gcArgComma_  GLdouble, GLdouble, GLdouble); \
    GLvoid         glApiMacro(Vertex3dv) (_gcArgComma_  const GLdouble *); \
    GLvoid         glApiMacro(Vertex3f) (_gcArgComma_  GLfloat, GLfloat, GLfloat); \
    GLvoid         glApiMacro(Vertex3fv) (_gcArgComma_  const GLfloat *); \
    GLvoid         glApiMacro(Vertex3i) (_gcArgComma_  GLint, GLint, GLint); \
    GLvoid         glApiMacro(Vertex3iv) (_gcArgComma_  const GLint *); \
    GLvoid         glApiMacro(Vertex3s) (_gcArgComma_  GLshort, GLshort, GLshort); \
    GLvoid         glApiMacro(Vertex3sv) (_gcArgComma_  const GLshort *); \
    GLvoid         glApiMacro(Vertex4d) (_gcArgComma_  GLdouble, GLdouble, GLdouble, GLdouble); \
    GLvoid         glApiMacro(Vertex4dv) (_gcArgComma_  const GLdouble *); \
    GLvoid         glApiMacro(Vertex4f) (_gcArgComma_  GLfloat, GLfloat, GLfloat, GLfloat); \
    GLvoid         glApiMacro(Vertex4fv) (_gcArgComma_  const GLfloat *); \
    GLvoid         glApiMacro(Vertex4i) (_gcArgComma_  GLint, GLint, GLint, GLint); \
    GLvoid         glApiMacro(Vertex4iv) (_gcArgComma_  const GLint *); \
    GLvoid         glApiMacro(Vertex4s) (_gcArgComma_  GLshort, GLshort, GLshort, GLshort); \
    GLvoid         glApiMacro(Vertex4sv) (_gcArgComma_  const GLshort *); \
    GLvoid         glApiMacro(ClipPlane) (_gcArgComma_  GLenum, const GLdouble *); \
    GLvoid         glApiMacro(ColorMaterial) (_gcArgComma_  GLenum, GLenum); \
    GLvoid         glApiMacro(CullFace) (_gcArgComma_ GLenum mode); \
    GLvoid         glApiMacro(Fogf) (_gcArgComma_  GLenum, GLfloat); \
    GLvoid         glApiMacro(Fogfv) (_gcArgComma_  GLenum, const GLfloat *); \
    GLvoid         glApiMacro(Fogi) (_gcArgComma_  GLenum, GLint); \
    GLvoid         glApiMacro(Fogiv) (_gcArgComma_  GLenum, const GLint *); \
    GLvoid         glApiMacro(FrontFace) (_gcArgComma_ GLenum mode); \
    GLvoid         glApiMacro(Hint) (_gcArgComma_ GLenum target, GLenum mode); \
    GLvoid         glApiMacro(Lightf) (_gcArgComma_  GLenum, GLenum, GLfloat); \
    GLvoid         glApiMacro(Lightfv) (_gcArgComma_  GLenum, GLenum, const GLfloat *); \
    GLvoid         glApiMacro(Lighti) (_gcArgComma_  GLenum, GLenum, GLint); \
    GLvoid         glApiMacro(Lightiv) (_gcArgComma_  GLenum, GLenum, const GLint *); \
    GLvoid         glApiMacro(LightModelf) (_gcArgComma_  GLenum, GLfloat); \
    GLvoid         glApiMacro(LightModelfv) (_gcArgComma_  GLenum, const GLfloat *); \
    GLvoid         glApiMacro(LightModeli) (_gcArgComma_  GLenum, GLint); \
    GLvoid         glApiMacro(LightModeliv) (_gcArgComma_  GLenum, const GLint *); \
    GLvoid         glApiMacro(LineStipple) (_gcArgComma_  GLint, GLushort); \
    GLvoid         glApiMacro(LineWidth) (_gcArgComma_ GLfloat width); \
    GLvoid         glApiMacro(Materialf) (_gcArgComma_  GLenum, GLenum, GLfloat); \
    GLvoid         glApiMacro(Materialfv) (_gcArgComma_  GLenum, GLenum, const GLfloat *); \
    GLvoid         glApiMacro(Materiali) (_gcArgComma_  GLenum, GLenum, GLint); \
    GLvoid         glApiMacro(Materialiv) (_gcArgComma_  GLenum, GLenum, const GLint *); \
    GLvoid         glApiMacro(PointSize) (_gcArgComma_  GLfloat); \
    GLvoid         glApiMacro(PolygonMode) (_gcArgComma_  GLenum, GLenum); \
    GLvoid         glApiMacro(PolygonStipple) (_gcArgComma_  const GLubyte *); \
    GLvoid         glApiMacro(Scissor) (_gcArgComma_ GLint x, GLint y, GLsizei width, GLsizei height); \
    GLvoid         glApiMacro(ShadeModel) (_gcArgComma_  GLenum); \
    GLvoid         glApiMacro(TexParameterf) (_gcArgComma_ GLenum target, GLenum pname, GLfloat param); \
    GLvoid         glApiMacro(TexParameterfv) (_gcArgComma_ GLenum target, GLenum pname, const GLfloat* params); \
    GLvoid         glApiMacro(TexParameteri) (_gcArgComma_ GLenum target, GLenum pname, GLint param); \
    GLvoid         glApiMacro(TexParameteriv) (_gcArgComma_ GLenum target, GLenum pname, const GLint* params); \
    GLvoid         glApiMacro(TexImage1D) (_gcArgComma_  GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, const GLvoid *); \
    GLvoid         glApiMacro(TexImage2D) (_gcArgComma_ GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels); \
    GLvoid         glApiMacro(TexEnvf) (_gcArgComma_  GLenum, GLenum, GLfloat); \
    GLvoid         glApiMacro(TexEnvfv) (_gcArgComma_  GLenum, GLenum, const GLfloat *); \
    GLvoid         glApiMacro(TexEnvi) (_gcArgComma_  GLenum, GLenum, GLint); \
    GLvoid         glApiMacro(TexEnviv) (_gcArgComma_  GLenum, GLenum, const GLint *); \
    GLvoid         glApiMacro(TexGend) (_gcArgComma_  GLenum, GLenum, GLdouble); \
    GLvoid         glApiMacro(TexGendv) (_gcArgComma_  GLenum, GLenum, const GLdouble *); \
    GLvoid         glApiMacro(TexGenf) (_gcArgComma_  GLenum, GLenum, GLfloat); \
    GLvoid         glApiMacro(TexGenfv) (_gcArgComma_  GLenum, GLenum, const GLfloat *); \
    GLvoid         glApiMacro(TexGeni) (_gcArgComma_  GLenum, GLenum, GLint); \
    GLvoid         glApiMacro(TexGeniv) (_gcArgComma_ GLenum, GLenum, const GLint *); \
    GLvoid         glApiMacro(FeedbackBuffer) (_gcArgComma_  GLsizei, GLenum, GLfloat *); \
    GLvoid         glApiMacro(SelectBuffer) (_gcArgComma_  GLsizei, GLuint *); \
    GLint          glApiMacro(RenderMode) (_gcArgComma_  GLenum); \
    GLvoid         glApiMacro(InitNames) (_gcArgOnly_); \
    GLvoid         glApiMacro(LoadName) (_gcArgComma_  GLuint); \
    GLvoid         glApiMacro(PassThrough) (_gcArgComma_  GLfloat); \
    GLvoid         glApiMacro(PopName) (_gcArgOnly_); \
    GLvoid         glApiMacro(PushName) (_gcArgComma_  GLuint); \
    GLvoid         glApiMacro(DrawBuffer) (_gcArgComma_  GLenum); \
    GLvoid         glApiMacro(Clear) (_gcArgComma_ GLbitfield mask); \
    GLvoid         glApiMacro(ClearAccum)(_gcArgComma_  GLfloat, GLfloat, GLfloat, GLfloat); \
    GLvoid         glApiMacro(ClearIndex)(_gcArgComma_  GLfloat); \
    GLvoid         glApiMacro(ClearColor) (_gcArgComma_ GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha); \
    GLvoid         glApiMacro(ClearStencil) (_gcArgComma_ GLint s); \
    GLvoid         glApiMacro(ClearDepth)(_gcArgComma_  GLclampd); \
    GLvoid         glApiMacro(StencilMask) (_gcArgComma_ GLuint mask); \
    GLvoid         glApiMacro(ColorMask) (_gcArgComma_ GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha); \
    GLvoid         glApiMacro(DepthMask) (_gcArgComma_ GLboolean flag); \
    GLvoid         glApiMacro(IndexMask)(_gcArgComma_ GLuint); \
    GLvoid         glApiMacro(Accum)(_gcArgComma_ GLenum, GLfloat); \
    GLvoid         glApiMacro(Disable) (_gcArgComma_ GLenum cap); \
    GLvoid         glApiMacro(Enable) (_gcArgComma_ GLenum cap); \
    GLvoid         glApiMacro(Finish) (_gcArgOnly_); \
    GLvoid         glApiMacro(Flush) (_gcArgOnly_); \
    GLvoid         glApiMacro(PopAttrib)(_gcArgOnly_); \
    GLvoid         glApiMacro(PushAttrib)(_gcArgComma_ GLbitfield); \
    GLvoid         glApiMacro(Map1d)(_gcArgComma_ GLenum, GLdouble, GLdouble, GLint, GLint, const GLdouble *); \
    GLvoid         glApiMacro(Map1f)(_gcArgComma_ GLenum, GLfloat, GLfloat, GLint, GLint, const GLfloat *); \
    GLvoid         glApiMacro(Map2d)(_gcArgComma_ GLenum, GLdouble, GLdouble, GLint, GLint, GLdouble, GLdouble, GLint, GLint, const GLdouble *); \
    GLvoid         glApiMacro(Map2f)(_gcArgComma_ GLenum, GLfloat, GLfloat, GLint, GLint, GLfloat, GLfloat, GLint, GLint, const GLfloat *); \
    GLvoid         glApiMacro(MapGrid1d)(_gcArgComma_ GLint, GLdouble, GLdouble); \
    GLvoid         glApiMacro(MapGrid1f)(_gcArgComma_ GLint, GLfloat, GLfloat); \
    GLvoid         glApiMacro(MapGrid2d)(_gcArgComma_ GLint, GLdouble, GLdouble, GLint, GLdouble, GLdouble); \
    GLvoid         glApiMacro(MapGrid2f)(_gcArgComma_  GLint, GLfloat, GLfloat, GLint, GLfloat, GLfloat); \
    GLvoid         glApiMacro(EvalCoord1d)(_gcArgComma_ GLdouble); \
    GLvoid         glApiMacro(EvalCoord1dv)(_gcArgComma_ const GLdouble *); \
    GLvoid         glApiMacro(EvalCoord1f)(_gcArgComma_  GLfloat); \
    GLvoid         glApiMacro(EvalCoord1fv)(_gcArgComma_ const GLfloat *); \
    GLvoid         glApiMacro(EvalCoord2d)(_gcArgComma_  GLdouble, GLdouble); \
    GLvoid         glApiMacro(EvalCoord2dv)(_gcArgComma_ const GLdouble *); \
    GLvoid         glApiMacro(EvalCoord2f)(_gcArgComma_  GLfloat, GLfloat); \
    GLvoid         glApiMacro(EvalCoord2fv)(_gcArgComma_ const GLfloat *); \
    GLvoid         glApiMacro(EvalMesh1)(_gcArgComma_ GLenum, GLint, GLint); \
    GLvoid         glApiMacro(EvalPoint1)(_gcArgComma_ GLint); \
    GLvoid         glApiMacro(EvalMesh2)(_gcArgComma_  GLenum, GLint, GLint, GLint, GLint); \
    GLvoid         glApiMacro(EvalPoint2)(_gcArgComma_ GLint, GLint); \
    GLvoid         glApiMacro(AlphaFunc)(_gcArgComma_ GLenum, GLclampf); \
    GLvoid         glApiMacro(BlendFunc) (_gcArgComma_ GLenum sfactor, GLenum dfactor); \
    GLvoid         glApiMacro(LogicOp)(_gcArgComma_ GLenum); \
    GLvoid         glApiMacro(StencilFunc) (_gcArgComma_ GLenum func, GLint ref, GLuint mask); \
    GLvoid         glApiMacro(StencilOp) (_gcArgComma_ GLenum fail, GLenum zfail, GLenum zpass); \
    GLvoid         glApiMacro(DepthFunc) (_gcArgComma_ GLenum func); \
    GLvoid         glApiMacro(PixelZoom)(_gcArgComma_ GLfloat, GLfloat); \
    GLvoid         glApiMacro(PixelTransferf)(_gcArgComma_ GLenum, GLfloat); \
    GLvoid         glApiMacro(PixelTransferi)(_gcArgComma_ GLenum, GLint); \
    GLvoid         glApiMacro(PixelStoref)(_gcArgComma_ GLenum, GLfloat); \
    GLvoid         glApiMacro(PixelStorei) (_gcArgComma_ GLenum pname, GLint param); \
    GLvoid         glApiMacro(PixelMapfv)(_gcArgComma_ GLenum, GLint, const GLfloat *); \
    GLvoid         glApiMacro(PixelMapuiv)(_gcArgComma_ GLenum, GLint, const GLuint *); \
    GLvoid         glApiMacro(PixelMapusv)(_gcArgComma_ GLenum, GLint, const GLushort *); \
    GLvoid         glApiMacro(ReadBuffer) (_gcArgComma_ GLenum mode); \
    GLvoid         glApiMacro(CopyPixels)(_gcArgComma_  GLint, GLint, GLsizei, GLsizei, GLenum); \
    GLvoid         glApiMacro(ReadPixels) (_gcArgComma_ GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels); \
    GLvoid         glApiMacro(DrawPixels)(_gcArgComma_  GLsizei, GLsizei, GLenum, GLenum, const GLvoid *); \
    GLvoid         glApiMacro(GetBooleanv) (_gcArgComma_ GLenum pname, GLboolean* params); \
    GLvoid         glApiMacro(GetClipPlane)(_gcArgComma_  GLenum, GLdouble *);    \
    GLvoid         glApiMacro(GetDoublev)(_gcArgComma_  GLenum, GLdouble *); \
    GLenum         glApiMacro(GetError) (_gcArgOnly_); \
    GLvoid         glApiMacro(GetFloatv) (_gcArgComma_ GLenum pname, GLfloat* params); \
    GLvoid         glApiMacro(GetIntegerv) (_gcArgComma_ GLenum pname, GLint* params); \
    GLvoid         glApiMacro(GetLightfv)(_gcArgComma_  GLenum, GLenum, GLfloat *); \
    GLvoid         glApiMacro(GetLightiv)(_gcArgComma_   GLenum, GLenum, GLint *); \
    GLvoid         glApiMacro(GetMapdv)(_gcArgComma_  GLenum, GLenum, GLdouble *); \
    GLvoid         glApiMacro(GetMapfv)(_gcArgComma_   GLenum, GLenum, GLfloat *); \
    GLvoid         glApiMacro(GetMapiv)(_gcArgComma_   GLenum, GLenum, GLint *); \
    GLvoid         glApiMacro(GetMaterialfv)(_gcArgComma_   GLenum, GLenum, GLfloat *); \
    GLvoid         glApiMacro(GetMaterialiv)(_gcArgComma_   GLenum, GLenum, GLint *); \
    GLvoid         glApiMacro(GetPixelMapfv)(_gcArgComma_  GLenum, GLfloat *); \
    GLvoid         glApiMacro(GetPixelMapuiv)(_gcArgComma_  GLenum, GLuint *); \
    GLvoid         glApiMacro(GetPixelMapusv)(_gcArgComma_  GLenum, GLushort *); \
    GLvoid         glApiMacro(GetPolygonStipple)(_gcArgComma_  GLubyte *); \
    const GLubyte* glApiMacro(GetString) (_gcArgComma_ GLenum name); \
    GLvoid         glApiMacro(GetTexEnvfv)(_gcArgComma_   GLenum, GLenum, GLfloat *); \
    GLvoid         glApiMacro(GetTexEnviv)(_gcArgComma_  GLenum, GLenum, GLint *); \
    GLvoid         glApiMacro(GetTexGendv)(_gcArgComma_  GLenum, GLenum, GLdouble *); \
    GLvoid         glApiMacro(GetTexGenfv)(_gcArgComma_  GLenum, GLenum, GLfloat *); \
    GLvoid         glApiMacro(GetTexGeniv)(_gcArgComma_  GLenum, GLenum, GLint *); \
    GLvoid         glApiMacro(GetTexImage)(_gcArgComma_  GLenum, GLint, GLenum, GLenum, GLvoid *); \
    GLvoid         glApiMacro(GetTexParameterfv) (_gcArgComma_ GLenum target, GLenum pname, GLfloat* params); \
    GLvoid         glApiMacro(GetTexParameteriv) (_gcArgComma_ GLenum target, GLenum pname, GLint* params); \
    GLvoid         glApiMacro(GetTexLevelParameterfv) (_gcArgComma_ GLenum target, GLint level, GLenum pname, GLfloat *params); \
    GLvoid         glApiMacro(GetTexLevelParameteriv) (_gcArgComma_ GLenum target, GLint level, GLenum pname, GLint *params); \
    GLboolean      glApiMacro(IsEnabled) (_gcArgComma_ GLenum cap); \
    GLboolean      glApiMacro(IsList)(_gcArgComma_  GLuint); \
    GLvoid         glApiMacro(DepthRange)(_gcArgComma_  GLclampd, GLclampd); \
    GLvoid         glApiMacro(Frustum)(_gcArgComma_ GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble); \
    GLvoid         glApiMacro(LoadIdentity)(_gcArgOnly_); \
    GLvoid         glApiMacro(LoadMatrixf)(_gcArgComma_  const GLfloat *); \
    GLvoid         glApiMacro(LoadMatrixd)(_gcArgComma_  const GLdouble *); \
    GLvoid         glApiMacro(MatrixMode)(_gcArgComma_ GLenum); \
    GLvoid         glApiMacro(MultMatrixf)(_gcArgComma_ const GLfloat *); \
    GLvoid         glApiMacro(MultMatrixd)(_gcArgComma_ const GLdouble *); \
    GLvoid         glApiMacro(Ortho)(_gcArgComma_  GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble); \
    GLvoid         glApiMacro(PopMatrix)(_gcArgOnly_); \
    GLvoid         glApiMacro(PushMatrix)(_gcArgOnly_); \
    GLvoid         glApiMacro(Rotated)(_gcArgComma_ GLdouble, GLdouble, GLdouble, GLdouble); \
    GLvoid         glApiMacro(Rotatef)(_gcArgComma_ GLfloat, GLfloat, GLfloat, GLfloat); \
    GLvoid         glApiMacro(Scaled)(_gcArgComma_  GLdouble, GLdouble, GLdouble); \
    GLvoid         glApiMacro(Scalef)(_gcArgComma_ GLfloat, GLfloat, GLfloat); \
    GLvoid         glApiMacro(Translated)(_gcArgComma_ GLdouble, GLdouble, GLdouble); \
    GLvoid         glApiMacro(Translatef)(_gcArgComma_ GLfloat, GLfloat, GLfloat); \
    GLvoid         glApiMacro(Viewport) (_gcArgComma_ GLint x, GLint y, GLsizei width, GLsizei height); \
    /* GL_VERSION_1_1 */ \
    GLvoid         glApiMacro(ArrayElement) (_gcArgComma_  GLint); \
    GLvoid         glApiMacro(BindTexture) (_gcArgComma_ GLenum target, GLuint texture); \
    GLvoid         glApiMacro(ColorPointer)(_gcArgComma_ GLint, GLenum, GLsizei, const GLvoid *); \
    GLvoid         glApiMacro(DisableClientState)(_gcArgComma_ GLenum); \
    GLvoid         glApiMacro(DrawArrays) (_gcArgComma_ GLenum mode, GLint first, GLsizei count); \
    GLvoid         glApiMacro(DrawElements) (_gcArgComma_ GLenum mode, GLsizei count, GLenum type, const GLvoid* indices); \
    GLvoid         glApiMacro(EdgeFlagPointer)(_gcArgComma_ GLsizei, const GLvoid*); \
    GLvoid         glApiMacro(EnableClientState)(_gcArgComma_ GLenum); \
    GLvoid         glApiMacro(IndexPointer)(_gcArgComma_ GLenum, GLsizei, const GLvoid *); \
    GLvoid         glApiMacro(Indexub)(_gcArgComma_ GLubyte); \
    GLvoid         glApiMacro(Indexubv)(_gcArgComma_ const GLubyte *); \
    GLvoid         glApiMacro(InterleavedArrays)(_gcArgComma_ GLenum, GLsizei, const GLvoid *); \
    GLvoid         glApiMacro(NormalPointer)(_gcArgComma_ GLenum, GLsizei, const GLvoid *); \
    GLvoid         glApiMacro(PolygonOffset) (_gcArgComma_ GLfloat factor, GLfloat units); \
    GLvoid         glApiMacro(TexCoordPointer)(_gcArgComma_ GLint, GLenum, GLsizei, const GLvoid *); \
    GLvoid         glApiMacro(VertexPointer)(_gcArgComma_ GLint, GLenum, GLsizei, const GLvoid *); \
    GLboolean      glApiMacro(AreTexturesResident)(_gcArgComma_ GLsizei, const GLuint *, GLboolean *); \
    GLvoid         glApiMacro(CopyTexImage1D)(_gcArgComma_ GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLint); \
    GLvoid         glApiMacro(CopyTexImage2D) (_gcArgComma_ GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border); \
    GLvoid         glApiMacro(CopyTexSubImage1D)(_gcArgComma_ GLenum, GLint, GLint, GLint, GLint, GLsizei);    \
    GLvoid         glApiMacro(CopyTexSubImage2D) (_gcArgComma_ GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height); \
    GLvoid         glApiMacro(DeleteTextures) (_gcArgComma_ GLsizei n, const GLuint* textures); \
    GLvoid         glApiMacro(GenTextures) (_gcArgComma_ GLsizei n, GLuint* textures); \
    GLvoid         glApiMacro(GetPointerv) (_gcArgComma_ GLenum pname, GLvoid** params); \
    GLboolean      glApiMacro(IsTexture) (_gcArgComma_ GLuint texture); \
    GLvoid         glApiMacro(PrioritizeTextures)(_gcArgComma_ GLsizei, const GLuint *, const GLclampf *); \
    GLvoid         glApiMacro(TexSubImage1D)(_gcArgComma_  GLenum, GLint, GLint, GLsizei, GLenum, GLenum, const GLvoid *); \
    GLvoid         glApiMacro(TexSubImage2D) (_gcArgComma_ GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels); \
    GLvoid         glApiMacro(PopClientAttrib)(_gcArgOnly_); \
    GLvoid         glApiMacro(PushClientAttrib)(_gcArgComma_  GLbitfield); \
    /* GL_VERSION_1_2 */ \
    GLvoid         glApiMacro(DrawRangeElements) (_gcArgComma_ GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices); \
    GLvoid         glApiMacro(TexImage3D) (_gcArgComma_ GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels); \
    GLvoid         glApiMacro(TexSubImage3D) (_gcArgComma_ GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels); \
    GLvoid         glApiMacro(CopyTexSubImage3D) (_gcArgComma_ GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height); \
    /* GL_VERSION_1_3 */ \
    GLvoid         glApiMacro(ActiveTexture) (_gcArgComma_ GLenum texture); \
    GLvoid         glApiMacro(SampleCoverage) (_gcArgComma_ GLfloat value, GLboolean invert); \
    GLvoid         glApiMacro(CompressedTexImage3D) (_gcArgComma_ GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data); \
    GLvoid         glApiMacro(CompressedTexImage2D) (_gcArgComma_ GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data); \
    GLvoid         glApiMacro(CompressedTexImage1D)(_gcArgComma_ GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const GLvoid *); \
    GLvoid         glApiMacro(CompressedTexSubImage3D) (_gcArgComma_ GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data); \
    GLvoid         glApiMacro(CompressedTexSubImage2D) (_gcArgComma_ GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data); \
    GLvoid         glApiMacro(CompressedTexSubImage1D)(_gcArgComma_ GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const GLvoid *); \
    GLvoid         glApiMacro(GetCompressedTexImage)(_gcArgComma_ GLenum, GLint, GLvoid *); \
    GLvoid         glApiMacro(ClientActiveTexture)(_gcArgComma_ GLenum); \
    GLvoid         glApiMacro(MultiTexCoord1d)(_gcArgComma_  GLenum, GLdouble); \
    GLvoid         glApiMacro(MultiTexCoord1dv)(_gcArgComma_ GLenum, const GLdouble *); \
    GLvoid         glApiMacro(MultiTexCoord1f)(_gcArgComma_ GLenum, GLfloat); \
    GLvoid         glApiMacro(MultiTexCoord1fv)(_gcArgComma_  GLenum, const GLfloat *); \
    GLvoid         glApiMacro(MultiTexCoord1i)(_gcArgComma_  GLenum, GLint); \
    GLvoid         glApiMacro(MultiTexCoord1iv)(_gcArgComma_ GLenum, const GLint *); \
    GLvoid         glApiMacro(MultiTexCoord1s)(_gcArgComma_ GLenum, GLshort); \
    GLvoid         glApiMacro(MultiTexCoord1sv)(_gcArgComma_  GLenum, const GLshort *); \
    GLvoid         glApiMacro(MultiTexCoord2d)(_gcArgComma_ GLenum, GLdouble, GLdouble); \
    GLvoid         glApiMacro(MultiTexCoord2dv)(_gcArgComma_  GLenum, const GLdouble *); \
    GLvoid         glApiMacro(MultiTexCoord2f)(_gcArgComma_ GLenum, GLfloat, GLfloat); \
    GLvoid         glApiMacro(MultiTexCoord2fv)(_gcArgComma_ GLenum, const GLfloat *); \
    GLvoid         glApiMacro(MultiTexCoord2i)(_gcArgComma_ GLenum, GLint, GLint); \
    GLvoid         glApiMacro(MultiTexCoord2iv)(_gcArgComma_ GLenum, const GLint *); \
    GLvoid         glApiMacro(MultiTexCoord2s)(_gcArgComma_ GLenum, GLshort, GLshort); \
    GLvoid         glApiMacro(MultiTexCoord2sv)(_gcArgComma_ GLenum, const GLshort *); \
    GLvoid         glApiMacro(MultiTexCoord3d)(_gcArgComma_ GLenum, GLdouble, GLdouble, GLdouble); \
    GLvoid         glApiMacro(MultiTexCoord3dv)(_gcArgComma_ GLenum, const GLdouble *); \
    GLvoid         glApiMacro(MultiTexCoord3f)(_gcArgComma_ GLenum, GLfloat, GLfloat, GLfloat); \
    GLvoid         glApiMacro(MultiTexCoord3fv)(_gcArgComma_  GLenum, const GLfloat *); \
    GLvoid         glApiMacro(MultiTexCoord3i)(_gcArgComma_ GLenum, GLint, GLint, GLint); \
    GLvoid         glApiMacro(MultiTexCoord3iv)(_gcArgComma_ GLenum, const GLint *); \
    GLvoid         glApiMacro(MultiTexCoord3s)(_gcArgComma_ GLenum, GLshort, GLshort, GLshort); \
    GLvoid         glApiMacro(MultiTexCoord3sv)(_gcArgComma_ GLenum, const GLshort *); \
    GLvoid         glApiMacro(MultiTexCoord4d)(_gcArgComma_ GLenum, GLdouble, GLdouble, GLdouble, GLdouble); \
    GLvoid         glApiMacro(MultiTexCoord4dv)(_gcArgComma_  GLenum, const GLdouble *); \
    GLvoid         glApiMacro(MultiTexCoord4f)(_gcArgComma_ GLenum, GLfloat, GLfloat, GLfloat, GLfloat); \
    GLvoid         glApiMacro(MultiTexCoord4fv)(_gcArgComma_  GLenum, const GLfloat *); \
    GLvoid         glApiMacro(MultiTexCoord4i)(_gcArgComma_ GLenum, GLint, GLint, GLint, GLint); \
    GLvoid         glApiMacro(MultiTexCoord4iv)(_gcArgComma_ GLenum, const GLint *); \
    GLvoid         glApiMacro(MultiTexCoord4s)(_gcArgComma_ GLenum, GLshort, GLshort, GLshort, GLshort); \
    GLvoid         glApiMacro(MultiTexCoord4sv)(_gcArgComma_ GLenum, const GLshort *); \
    GLvoid         glApiMacro(LoadTransposeMatrixf)(_gcArgComma_ const GLfloat *); \
    GLvoid         glApiMacro(LoadTransposeMatrixd)(_gcArgComma_  const GLdouble *); \
    GLvoid         glApiMacro(MultTransposeMatrixf)(_gcArgComma_ const GLfloat *); \
    GLvoid         glApiMacro(MultTransposeMatrixd)(_gcArgComma_ const GLdouble *); \
    /* GL_VERSION_1_4 */ \
    GLvoid         glApiMacro(BlendFuncSeparate) (_gcArgComma_ GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha); \
    GLvoid         glApiMacro(MultiDrawArrays) (_gcArgComma_ GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount); \
    GLvoid         glApiMacro(MultiDrawElements) (_gcArgComma_ GLenum mode, const GLsizei *count, GLenum type, const GLvoid*const*indices, GLsizei primcount); \
    GLvoid         glApiMacro(PointParameterf)(_gcArgComma_  GLenum, GLfloat); \
    GLvoid         glApiMacro(PointParameterfv)(_gcArgComma_  GLenum, const GLfloat *); \
    GLvoid         glApiMacro(PointParameteri)(_gcArgComma_  GLenum, GLint); \
    GLvoid         glApiMacro(PointParameteriv)(_gcArgComma_  GLenum, const GLint *); \
    GLvoid         glApiMacro(FogCoordf)(_gcArgComma_  GLfloat); \
    GLvoid         glApiMacro(FogCoordfv)(_gcArgComma_  const GLfloat *); \
    GLvoid         glApiMacro(FogCoordd)(_gcArgComma_ GLdouble); \
    GLvoid         glApiMacro(FogCoorddv)(_gcArgComma_ const GLdouble *); \
    GLvoid         glApiMacro(FogCoordPointer)(_gcArgComma_  GLenum, GLsizei, const GLvoid *); \
    GLvoid         glApiMacro(SecondaryColor3b)(_gcArgComma_  GLbyte, GLbyte, GLbyte); \
    GLvoid         glApiMacro(SecondaryColor3bv)(_gcArgComma_  const GLbyte *); \
    GLvoid         glApiMacro(SecondaryColor3d)(_gcArgComma_  GLdouble, GLdouble, GLdouble); \
    GLvoid         glApiMacro(SecondaryColor3dv)(_gcArgComma_  const GLdouble *); \
    GLvoid         glApiMacro(SecondaryColor3f)(_gcArgComma_  GLfloat, GLfloat, GLfloat); \
    GLvoid         glApiMacro(SecondaryColor3fv)(_gcArgComma_  const GLfloat *); \
    GLvoid         glApiMacro(SecondaryColor3i)(_gcArgComma_  GLint, GLint, GLint); \
    GLvoid         glApiMacro(SecondaryColor3iv)(_gcArgComma_  const GLint *); \
    GLvoid         glApiMacro(SecondaryColor3s)(_gcArgComma_  GLshort, GLshort, GLshort); \
    GLvoid         glApiMacro(SecondaryColor3sv)(_gcArgComma_  const GLshort *); \
    GLvoid         glApiMacro(SecondaryColor3ub)(_gcArgComma_  GLubyte, GLubyte, GLubyte); \
    GLvoid         glApiMacro(SecondaryColor3ubv)(_gcArgComma_ const GLubyte *); \
    GLvoid         glApiMacro(SecondaryColor3ui)(_gcArgComma_  GLuint, GLuint, GLuint); \
    GLvoid         glApiMacro(SecondaryColor3uiv)(_gcArgComma_  const GLuint *); \
    GLvoid         glApiMacro(SecondaryColor3us)(_gcArgComma_ GLushort, GLushort, GLushort); \
    GLvoid         glApiMacro(SecondaryColor3usv)(_gcArgComma_  const GLushort *); \
    GLvoid         glApiMacro(SecondaryColorPointer)(_gcArgComma_  GLint, GLenum, GLsizei, const GLvoid *); \
    GLvoid         glApiMacro(WindowPos2d)(_gcArgComma_  GLdouble, GLdouble); \
    GLvoid         glApiMacro(WindowPos2dv)(_gcArgComma_  const GLdouble *); \
    GLvoid         glApiMacro(WindowPos2f)(_gcArgComma_  GLfloat, GLfloat); \
    GLvoid         glApiMacro(WindowPos2fv)(_gcArgComma_  const GLfloat *); \
    GLvoid         glApiMacro(WindowPos2i)(_gcArgComma_  GLint, GLint); \
    GLvoid         glApiMacro(WindowPos2iv)(_gcArgComma_  const GLint *); \
    GLvoid         glApiMacro(WindowPos2s)(_gcArgComma_ GLshort, GLshort); \
    GLvoid         glApiMacro(WindowPos2sv)(_gcArgComma_  const GLshort *); \
    GLvoid         glApiMacro(WindowPos3d)(_gcArgComma_  GLdouble, GLdouble, GLdouble); \
    GLvoid         glApiMacro(WindowPos3dv)(_gcArgComma_  const GLdouble *); \
    GLvoid         glApiMacro(WindowPos3f)(_gcArgComma_  GLfloat, GLfloat, GLfloat); \
    GLvoid         glApiMacro(WindowPos3fv)(_gcArgComma_  const GLfloat *); \
    GLvoid         glApiMacro(WindowPos3i)(_gcArgComma_ GLint, GLint, GLint); \
    GLvoid         glApiMacro(WindowPos3iv)(_gcArgComma_ const GLint *); \
    GLvoid         glApiMacro(WindowPos3s)(_gcArgComma_  GLshort, GLshort, GLshort); \
    GLvoid         glApiMacro(WindowPos3sv)(_gcArgComma_  const GLshort *); \
    GLvoid         glApiMacro(BlendColor) (_gcArgComma_ GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha); \
    GLvoid         glApiMacro(BlendEquation) (_gcArgComma_ GLenum mode); \
    /* GL_VERSION_1_5 */ \
    GLvoid         glApiMacro(GenQueries) (_gcArgComma_ GLsizei n, GLuint* ids); \
    GLvoid         glApiMacro(DeleteQueries) (_gcArgComma_ GLsizei n, const GLuint* ids); \
    GLboolean      glApiMacro(IsQuery) (_gcArgComma_ GLuint id); \
    GLvoid         glApiMacro(BeginQuery) (_gcArgComma_ GLenum target, GLuint id); \
    GLvoid         glApiMacro(EndQuery) (_gcArgComma_ GLenum target); \
    GLvoid         glApiMacro(GetQueryiv) (_gcArgComma_ GLenum target, GLenum pname, GLint* params); \
    GLvoid         glApiMacro(GetQueryObjectiv) (_gcArgComma_ GLuint id, GLenum pname, GLint* params); \
    GLvoid         glApiMacro(GetQueryObjectuiv) (_gcArgComma_ GLuint id, GLenum pname, GLuint* params); \
    GLvoid         glApiMacro(BindBuffer) (_gcArgComma_ GLenum target, GLuint buffer); \
    GLvoid         glApiMacro(DeleteBuffers) (_gcArgComma_ GLsizei n, const GLuint* buffers); \
    GLvoid         glApiMacro(GenBuffers) (_gcArgComma_ GLsizei n, GLuint* buffers); \
    GLboolean      glApiMacro(IsBuffer) (_gcArgComma_ GLuint buffer); \
    GLvoid         glApiMacro(BufferData) (_gcArgComma_ GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage); \
    GLvoid         glApiMacro(BufferSubData) (_gcArgComma_ GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data); \
    GLvoid         glApiMacro(GetBufferSubData) (_gcArgComma_ GLenum target, GLintptr offset, GLsizeiptr size, GLvoid* data); \
    GLvoid*        glApiMacro(MapBuffer) (_gcArgComma_ GLenum target, GLenum access _retPointer); \
    GLboolean      glApiMacro(UnmapBuffer) (_gcArgComma_ GLenum target); \
    GLvoid         glApiMacro(GetBufferParameteriv) (_gcArgComma_ GLenum target, GLenum pname, GLint* params); \
    GLvoid         glApiMacro(GetBufferPointerv) (_gcArgComma_ GLenum target, GLenum pname, GLvoid** params); \
    /* GL_VERSION_2_0 */ \
    GLvoid         glApiMacro(BlendEquationSeparate) (_gcArgComma_ GLenum modeRGB, GLenum modeAlpha); \
    GLvoid         glApiMacro(DrawBuffers) (_gcArgComma_ GLsizei n, const GLenum* bufs); \
    GLvoid         glApiMacro(StencilOpSeparate) (_gcArgComma_ GLenum face, GLenum fail, GLenum zfail, GLenum zpass); \
    GLvoid         glApiMacro(StencilFuncSeparate) (_gcArgComma_ GLenum face, GLenum func, GLint ref, GLuint mask); \
    GLvoid         glApiMacro(StencilMaskSeparate) (_gcArgComma_ GLenum face, GLuint mask); \
    GLvoid         glApiMacro(AttachShader) (_gcArgComma_ GLuint program, GLuint shader); \
    GLvoid         glApiMacro(BindAttribLocation) (_gcArgComma_ GLuint program, GLuint index, const GLchar* name); \
    GLvoid         glApiMacro(CompileShader) (_gcArgComma_ GLuint shader); \
    GLuint         glApiMacro(CreateProgram) (_gcArgOnly_ _retProgram_); \
    GLuint         glApiMacro(CreateShader) (_gcArgComma_ GLenum type _retShader_); \
    GLvoid         glApiMacro(DeleteProgram) (_gcArgComma_ GLuint program); \
    GLvoid         glApiMacro(DeleteShader) (_gcArgComma_ GLuint shader); \
    GLvoid         glApiMacro(DetachShader) (_gcArgComma_ GLuint program, GLuint shader); \
    GLvoid         glApiMacro(DisableVertexAttribArray) (_gcArgComma_ GLuint index); \
    GLvoid         glApiMacro(EnableVertexAttribArray) (_gcArgComma_ GLuint index); \
    GLvoid         glApiMacro(GetActiveAttrib) (_gcArgComma_ GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name); \
    GLvoid         glApiMacro(GetActiveUniform) (_gcArgComma_ GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name); \
    GLvoid         glApiMacro(GetAttachedShaders) (_gcArgComma_ GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders); \
    GLint          glApiMacro(GetAttribLocation) (_gcArgComma_ GLuint program, const GLchar* name _retLocation_); \
    GLvoid         glApiMacro(GetProgramiv) (_gcArgComma_ GLuint program, GLenum pname, GLint* params); \
    GLvoid         glApiMacro(GetProgramInfoLog) (_gcArgComma_ GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog); \
    GLvoid         glApiMacro(GetShaderiv) (_gcArgComma_ GLuint shader, GLenum pname, GLint* params); \
    GLvoid         glApiMacro(GetShaderInfoLog) (_gcArgComma_ GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog); \
    GLvoid         glApiMacro(GetShaderSource) (_gcArgComma_ GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source); \
    GLint          glApiMacro(GetUniformLocation) (_gcArgComma_ GLuint program, const GLchar* name _retLocation_); \
    GLvoid         glApiMacro(GetUniformfv) (_gcArgComma_ GLuint program, GLint location, GLfloat* params); \
    GLvoid         glApiMacro(GetUniformiv) (_gcArgComma_ GLuint program, GLint location, GLint* params); \
    GLvoid         glApiMacro(GetVertexAttribdv) (_gcArgComma_ GLuint index, GLenum pname, GLdouble* params); \
    GLvoid         glApiMacro(GetVertexAttribfv) (_gcArgComma_ GLuint index, GLenum pname, GLfloat* params); \
    GLvoid         glApiMacro(GetVertexAttribiv) (_gcArgComma_ GLuint index, GLenum pname, GLint* params); \
    GLvoid         glApiMacro(GetVertexAttribPointerv) (_gcArgComma_ GLuint index, GLenum pname, GLvoid** pointer); \
    GLboolean      glApiMacro(IsProgram) (_gcArgComma_ GLuint program); \
    GLboolean      glApiMacro(IsShader) (_gcArgComma_ GLuint shader); \
    GLvoid         glApiMacro(LinkProgram) (_gcArgComma_ GLuint program); \
    GLvoid         glApiMacro(ShaderSource) (_gcArgComma_ GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length); \
    GLvoid         glApiMacro(UseProgram) (_gcArgComma_ GLuint program); \
    GLvoid         glApiMacro(Uniform1f) (_gcArgComma_ GLint location, GLfloat x); \
    GLvoid         glApiMacro(Uniform2f) (_gcArgComma_ GLint location, GLfloat x, GLfloat y); \
    GLvoid         glApiMacro(Uniform3f) (_gcArgComma_ GLint location, GLfloat x, GLfloat y, GLfloat z); \
    GLvoid         glApiMacro(Uniform4f) (_gcArgComma_ GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w); \
    GLvoid         glApiMacro(Uniform1i) (_gcArgComma_ GLint location, GLint x); \
    GLvoid         glApiMacro(Uniform2i) (_gcArgComma_ GLint location, GLint x, GLint y); \
    GLvoid         glApiMacro(Uniform3i) (_gcArgComma_ GLint location, GLint x, GLint y, GLint z); \
    GLvoid         glApiMacro(Uniform4i) (_gcArgComma_ GLint location, GLint x, GLint y, GLint z, GLint w); \
    GLvoid         glApiMacro(Uniform1fv) (_gcArgComma_ GLint location, GLsizei count, const GLfloat* v); \
    GLvoid         glApiMacro(Uniform2fv) (_gcArgComma_ GLint location, GLsizei count, const GLfloat* v); \
    GLvoid         glApiMacro(Uniform3fv) (_gcArgComma_ GLint location, GLsizei count, const GLfloat* v); \
    GLvoid         glApiMacro(Uniform4fv) (_gcArgComma_ GLint location, GLsizei count, const GLfloat* v); \
    GLvoid         glApiMacro(Uniform1iv) (_gcArgComma_ GLint location, GLsizei count, const GLint* v); \
    GLvoid         glApiMacro(Uniform2iv) (_gcArgComma_ GLint location, GLsizei count, const GLint* v); \
    GLvoid         glApiMacro(Uniform3iv) (_gcArgComma_ GLint location, GLsizei count, const GLint* v); \
    GLvoid         glApiMacro(Uniform4iv) (_gcArgComma_ GLint location, GLsizei count, const GLint* v); \
    GLvoid         glApiMacro(UniformMatrix2fv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLfloat* value); \
    GLvoid         glApiMacro(UniformMatrix3fv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLfloat* value); \
    GLvoid         glApiMacro(UniformMatrix4fv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLfloat* value); \
    GLvoid         glApiMacro(ValidateProgram) (_gcArgComma_ GLuint program); \
    GLvoid         glApiMacro(VertexAttrib1d) (_gcArgComma_ GLuint indx, GLdouble x); \
    GLvoid         glApiMacro(VertexAttrib1dv) (_gcArgComma_ GLuint indx, const GLdouble *values); \
    GLvoid         glApiMacro(VertexAttrib1f) (_gcArgComma_ GLuint indx, GLfloat x); \
    GLvoid         glApiMacro(VertexAttrib1fv) (_gcArgComma_ GLuint indx, const GLfloat * values); \
    GLvoid         glApiMacro(VertexAttrib1s) (_gcArgComma_ GLuint indx, GLshort x); \
    GLvoid         glApiMacro(VertexAttrib1sv) (_gcArgComma_ GLuint indx, const GLshort * values); \
    GLvoid         glApiMacro(VertexAttrib2d) (_gcArgComma_ GLuint indx, GLdouble x, GLdouble y); \
    GLvoid         glApiMacro(VertexAttrib2dv) (_gcArgComma_ GLuint indx, const GLdouble * values); \
    GLvoid         glApiMacro(VertexAttrib2f) (_gcArgComma_ GLuint indx, GLfloat x, GLfloat y); \
    GLvoid         glApiMacro(VertexAttrib2fv) (_gcArgComma_ GLuint indx, const GLfloat * values); \
    GLvoid         glApiMacro(VertexAttrib2s) (_gcArgComma_ GLuint indx, GLshort x, GLshort y); \
    GLvoid         glApiMacro(VertexAttrib2sv) (_gcArgComma_ GLuint indx, const GLshort * values); \
    GLvoid         glApiMacro(VertexAttrib3d) (_gcArgComma_ GLuint indx, GLdouble x, GLdouble y, GLdouble z); \
    GLvoid         glApiMacro(VertexAttrib3dv) (_gcArgComma_ GLuint indx, const GLdouble * values); \
    GLvoid         glApiMacro(VertexAttrib3f) (_gcArgComma_ GLuint indx, GLfloat x, GLfloat y, GLfloat z); \
    GLvoid         glApiMacro(VertexAttrib3fv) (_gcArgComma_ GLuint indx, const GLfloat * values); \
    GLvoid         glApiMacro(VertexAttrib3s) (_gcArgComma_ GLuint indx, GLshort x, GLshort y, GLshort z); \
    GLvoid         glApiMacro(VertexAttrib3sv) (_gcArgComma_ GLuint indx, const GLshort * values); \
    GLvoid         glApiMacro(VertexAttrib4Nbv) (_gcArgComma_ GLuint indx, const GLbyte * values); \
    GLvoid         glApiMacro(VertexAttrib4Niv) (_gcArgComma_ GLuint indx, const GLint * values); \
    GLvoid         glApiMacro(VertexAttrib4Nsv) (_gcArgComma_ GLuint indx, const GLshort * values); \
    GLvoid         glApiMacro(VertexAttrib4Nub) (_gcArgComma_ GLuint indx, GLubyte x, GLubyte y, GLubyte z, GLubyte w); \
    GLvoid         glApiMacro(VertexAttrib4Nubv) (_gcArgComma_ GLuint indx, const GLubyte * values); \
    GLvoid         glApiMacro(VertexAttrib4Nuiv) (_gcArgComma_ GLuint indx, const GLuint * values); \
    GLvoid         glApiMacro(VertexAttrib4Nusv) (_gcArgComma_ GLuint indx, const GLushort * values); \
    GLvoid         glApiMacro(VertexAttrib4bv) (_gcArgComma_ GLuint indx, const GLbyte * values); \
    GLvoid         glApiMacro(VertexAttrib4d) (_gcArgComma_ GLuint indx, GLdouble x, GLdouble y, GLdouble z, GLdouble w); \
    GLvoid         glApiMacro(VertexAttrib4dv) (_gcArgComma_ GLuint indx, const GLdouble * values); \
    GLvoid         glApiMacro(VertexAttrib4f) (_gcArgComma_ GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w); \
    GLvoid         glApiMacro(VertexAttrib4fv) (_gcArgComma_ GLuint indx, const GLfloat * values); \
    GLvoid         glApiMacro(VertexAttrib4iv) (_gcArgComma_ GLuint indx, const GLint * values); \
    GLvoid         glApiMacro(VertexAttrib4s) (_gcArgComma_ GLuint indx, GLshort x, GLshort y, GLshort z, GLshort w); \
    GLvoid         glApiMacro(VertexAttrib4sv) (_gcArgComma_ GLuint indx, const GLshort * values); \
    GLvoid         glApiMacro(VertexAttrib4ubv) (_gcArgComma_ GLuint indx, const GLubyte * values); \
    GLvoid         glApiMacro(VertexAttrib4uiv) (_gcArgComma_ GLuint indx, const GLuint * values); \
    GLvoid         glApiMacro(VertexAttrib4usv) (_gcArgComma_ GLuint indx, const GLushort * values); \
    GLvoid         glApiMacro(VertexAttribPointer) (_gcArgComma_ GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr); \
    /* GL_VERSION_2_1 */ \
    GLvoid         glApiMacro(UniformMatrix2x3fv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLfloat* value); \
    GLvoid         glApiMacro(UniformMatrix3x2fv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLfloat* value); \
    GLvoid         glApiMacro(UniformMatrix2x4fv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLfloat* value); \
    GLvoid         glApiMacro(UniformMatrix4x2fv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLfloat* value); \
    GLvoid         glApiMacro(UniformMatrix3x4fv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLfloat* value); \
    GLvoid         glApiMacro(UniformMatrix4x3fv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLfloat* value); \
    /* GL_VERSION_3_0 */ \
    GLvoid         glApiMacro(ColorMaski) (_gcArgComma_ GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a); \
    GLvoid         glApiMacro(GetBooleani_v) (_gcArgComma_ GLenum target, GLuint index, GLboolean *data); \
    GLvoid         glApiMacro(GetIntegeri_v) (_gcArgComma_ GLenum target, GLuint index, GLint *data); \
    GLvoid         glApiMacro(Enablei) (_gcArgComma_ GLenum target, GLuint index); \
    GLvoid         glApiMacro(Disablei) (_gcArgComma_ GLenum target, GLuint index); \
    GLboolean      glApiMacro(IsEnabledi) (_gcArgComma_ GLenum target, GLuint index); \
    GLvoid         glApiMacro(BeginTransformFeedback) (_gcArgComma_ GLenum primitiveMode); \
    GLvoid         glApiMacro(EndTransformFeedback) (_gcArgOnly_); \
    GLvoid         glApiMacro(BindBufferRange) (_gcArgComma_ GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size); \
    GLvoid         glApiMacro(BindBufferBase) (_gcArgComma_ GLenum target, GLuint index, GLuint buffer); \
    GLvoid         glApiMacro(TransformFeedbackVaryings) (_gcArgComma_ GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode); \
    GLvoid         glApiMacro(GetTransformFeedbackVarying) (_gcArgComma_ GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name); \
    GLvoid         glApiMacro(ClampColor) (_gcArgComma_ GLenum target, GLenum clamp); \
    GLvoid         glApiMacro(BeginConditionalRender) (_gcArgComma_ GLuint id, GLenum mode); \
    GLvoid         glApiMacro(EndConditionalRender) (_gcArgOnly_); \
    GLvoid         glApiMacro(VertexAttribIPointer) (_gcArgComma_ GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer); \
    GLvoid         glApiMacro(GetVertexAttribIiv) (_gcArgComma_ GLuint index, GLenum pname, GLint *params); \
    GLvoid         glApiMacro(GetVertexAttribIuiv) (_gcArgComma_ GLuint index, GLenum pname, GLuint *params); \
    GLvoid         glApiMacro(VertexAttribI1i) (_gcArgComma_ GLuint index, GLint x); \
    GLvoid         glApiMacro(VertexAttribI2i) (_gcArgComma_ GLuint index, GLint x, GLint y); \
    GLvoid         glApiMacro(VertexAttribI3i) (_gcArgComma_ GLuint index, GLint x, GLint y, GLint z); \
    GLvoid         glApiMacro(VertexAttribI4i) (_gcArgComma_ GLuint index, GLint x, GLint y, GLint z, GLint w); \
    GLvoid         glApiMacro(VertexAttribI1ui) (_gcArgComma_ GLuint index, GLuint x); \
    GLvoid         glApiMacro(VertexAttribI2ui) (_gcArgComma_ GLuint index, GLuint x, GLuint y); \
    GLvoid         glApiMacro(VertexAttribI3ui) (_gcArgComma_ GLuint index, GLuint x, GLuint y, GLuint z); \
    GLvoid         glApiMacro(VertexAttribI4ui) (_gcArgComma_ GLuint index, GLuint x, GLuint y, GLuint z, GLuint w); \
    GLvoid         glApiMacro(VertexAttribI1iv) (_gcArgComma_ GLuint index, const GLint *v); \
    GLvoid         glApiMacro(VertexAttribI2iv) (_gcArgComma_ GLuint index, const GLint *v); \
    GLvoid         glApiMacro(VertexAttribI3iv) (_gcArgComma_ GLuint index, const GLint *v); \
    GLvoid         glApiMacro(VertexAttribI4iv) (_gcArgComma_ GLuint index, const GLint *v); \
    GLvoid         glApiMacro(VertexAttribI1uiv) (_gcArgComma_ GLuint index, const GLuint *v); \
    GLvoid         glApiMacro(VertexAttribI2uiv) (_gcArgComma_ GLuint index, const GLuint *v); \
    GLvoid         glApiMacro(VertexAttribI3uiv) (_gcArgComma_ GLuint index, const GLuint *v); \
    GLvoid         glApiMacro(VertexAttribI4uiv) (_gcArgComma_ GLuint index, const GLuint *v); \
    GLvoid         glApiMacro(VertexAttribI4bv) (_gcArgComma_ GLuint index, const GLbyte *v); \
    GLvoid         glApiMacro(VertexAttribI4sv) (_gcArgComma_ GLuint index, const GLshort *v); \
    GLvoid         glApiMacro(VertexAttribI4ubv) (_gcArgComma_ GLuint index, const GLubyte *v); \
    GLvoid         glApiMacro(VertexAttribI4usv) (_gcArgComma_ GLuint index, const GLushort *v); \
    GLvoid         glApiMacro(GetUniformuiv) (_gcArgComma_ GLuint program, GLint location, GLuint *params); \
    GLvoid         glApiMacro(BindFragDataLocation) (_gcArgComma_ GLuint program, GLuint color, const GLchar *name); \
    GLint          glApiMacro(GetFragDataLocation) (_gcArgComma_ GLuint program, const GLchar *name _retLocation_); \
    GLvoid         glApiMacro(Uniform1ui) (_gcArgComma_ GLint location, GLuint v0); \
    GLvoid         glApiMacro(Uniform2ui) (_gcArgComma_ GLint location, GLuint v0, GLuint v1); \
    GLvoid         glApiMacro(Uniform3ui) (_gcArgComma_ GLint location, GLuint v0, GLuint v1, GLuint v2); \
    GLvoid         glApiMacro(Uniform4ui) (_gcArgComma_ GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3); \
    GLvoid         glApiMacro(Uniform1uiv) (_gcArgComma_ GLint location, GLsizei count, const GLuint *value); \
    GLvoid         glApiMacro(Uniform2uiv) (_gcArgComma_ GLint location, GLsizei count, const GLuint *value); \
    GLvoid         glApiMacro(Uniform3uiv) (_gcArgComma_ GLint location, GLsizei count, const GLuint *value); \
    GLvoid         glApiMacro(Uniform4uiv) (_gcArgComma_ GLint location, GLsizei count, const GLuint *value); \
    GLvoid         glApiMacro(TexParameterIiv) (_gcArgComma_ GLenum target, GLenum pname, const GLint *params); \
    GLvoid         glApiMacro(TexParameterIuiv) (_gcArgComma_ GLenum target, GLenum pname, const GLuint *params); \
    GLvoid         glApiMacro(GetTexParameterIiv) (_gcArgComma_ GLenum target, GLenum pname, GLint *params); \
    GLvoid         glApiMacro(GetTexParameterIuiv) (_gcArgComma_ GLenum target, GLenum pname, GLuint *params); \
    GLvoid         glApiMacro(ClearBufferiv) (_gcArgComma_ GLenum buffer, GLint drawbuffer, const GLint *value); \
    GLvoid         glApiMacro(ClearBufferuiv) (_gcArgComma_ GLenum buffer, GLint drawbuffer, const GLuint *value); \
    GLvoid         glApiMacro(ClearBufferfv) (_gcArgComma_ GLenum buffer, GLint drawbuffer, const GLfloat *value); \
    GLvoid         glApiMacro(ClearBufferfi) (_gcArgComma_ GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil); \
    const GLubyte *glApiMacro(GetStringi) (_gcArgComma_ GLenum name, GLuint index); \
    GLboolean      glApiMacro(IsRenderbuffer) (_gcArgComma_ GLuint renderbuffer); \
    GLvoid         glApiMacro(BindRenderbuffer) (_gcArgComma_ GLenum target, GLuint renderbuffer); \
    GLvoid         glApiMacro(DeleteRenderbuffers) (_gcArgComma_ GLsizei n, const GLuint *renderbuffers); \
    GLvoid         glApiMacro(GenRenderbuffers) (_gcArgComma_ GLsizei n, GLuint *renderbuffers); \
    GLvoid         glApiMacro(RenderbufferStorage) (_gcArgComma_ GLenum target, GLenum internalformat, GLsizei width, GLsizei height); \
    GLvoid         glApiMacro(GetRenderbufferParameteriv) (_gcArgComma_ GLenum target, GLenum pname, GLint *params); \
    GLboolean      glApiMacro(IsFramebuffer) (_gcArgComma_ GLuint framebuffer); \
    GLvoid         glApiMacro(BindFramebuffer) (_gcArgComma_ GLenum target, GLuint framebuffer); \
    GLvoid         glApiMacro(DeleteFramebuffers) (_gcArgComma_ GLsizei n, const GLuint *framebuffers); \
    GLvoid         glApiMacro(GenFramebuffers) (_gcArgComma_ GLsizei n, GLuint *framebuffers); \
    GLenum         glApiMacro(CheckFramebufferStatus) (_gcArgComma_ GLenum target); \
    GLvoid         glApiMacro(FramebufferTexture1D) (_gcArgComma_ GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level); \
    GLvoid         glApiMacro(FramebufferTexture2D) (_gcArgComma_ GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level); \
    GLvoid         glApiMacro(FramebufferTexture3D) (_gcArgComma_ GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset); \
    GLvoid         glApiMacro(FramebufferRenderbuffer) (_gcArgComma_ GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer); \
    GLvoid         glApiMacro(GetFramebufferAttachmentParameteriv) (_gcArgComma_ GLenum target, GLenum attachment, GLenum pname, GLint *params); \
    GLvoid         glApiMacro(GenerateMipmap) (_gcArgComma_ GLenum target); \
    GLvoid         glApiMacro(BlitFramebuffer) (_gcArgComma_ GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter); \
    GLvoid         glApiMacro(RenderbufferStorageMultisample) (_gcArgComma_ GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height); \
    GLvoid         glApiMacro(FramebufferTextureLayer) (_gcArgComma_ GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer); \
    GLvoid *       glApiMacro(MapBufferRange) (_gcArgComma_ GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access _retPointer); \
    GLvoid         glApiMacro(FlushMappedBufferRange) (_gcArgComma_ GLenum target, GLintptr offset, GLsizeiptr length); \
    GLvoid         glApiMacro(BindVertexArray) (_gcArgComma_ GLuint array); \
    GLvoid         glApiMacro(DeleteVertexArrays) (_gcArgComma_ GLsizei n, const GLuint *arrays); \
    GLvoid         glApiMacro(GenVertexArrays) (_gcArgComma_ GLsizei n, GLuint *arrays); \
    GLboolean      glApiMacro(IsVertexArray) (_gcArgComma_ GLuint array); \
    /* GL_VERSION_3_1 */ \
    GLvoid         glApiMacro(DrawArraysInstanced) (_gcArgComma_ GLenum mode, GLint first, GLsizei count, GLsizei instancecount); \
    GLvoid         glApiMacro(DrawElementsInstanced) (_gcArgComma_ GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei instancecount); \
    GLvoid         glApiMacro(TexBuffer) (_gcArgComma_ GLenum target, GLenum internalformat, GLuint buffer); \
    GLvoid         glApiMacro(PrimitiveRestartIndex) (_gcArgComma_ GLuint index); \
    GLvoid         glApiMacro(CopyBufferSubData) (_gcArgComma_ GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size); \
    GLvoid         glApiMacro(GetUniformIndices) (_gcArgComma_ GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices); \
    GLvoid         glApiMacro(GetActiveUniformsiv) (_gcArgComma_ GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params); \
    GLvoid         glApiMacro(GetActiveUniformName) (_gcArgComma_ GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName); \
    GLuint         glApiMacro(GetUniformBlockIndex) (_gcArgComma_ GLuint program, const GLchar *uniformBlockName _retIndex_); \
    GLvoid         glApiMacro(GetActiveUniformBlockiv) (_gcArgComma_ GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params); \
    GLvoid         glApiMacro(GetActiveUniformBlockName) (_gcArgComma_ GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName); \
    GLvoid         glApiMacro(UniformBlockBinding) (_gcArgComma_ GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding); \
    /* GL_VERSION_3_2 */ \
    GLvoid         glApiMacro(DrawElementsBaseVertex) (_gcArgComma_ GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLint basevertex); \
    GLvoid         glApiMacro(DrawRangeElementsBaseVertex) (_gcArgComma_ GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices, GLint basevertex); \
    GLvoid         glApiMacro(DrawElementsInstancedBaseVertex) (_gcArgComma_ GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei instancecount, GLint basevertex); \
    GLvoid         glApiMacro(MultiDrawElementsBaseVertex) (_gcArgComma_ GLenum mode, const GLsizei *count, GLenum type, const GLvoid *const*indices, GLsizei drawcount, const GLint *basevertex); \
    GLvoid         glApiMacro(ProvokingVertex) (_gcArgComma_ GLenum mode); \
    GLsync         glApiMacro(FenceSync) (_gcArgComma_ GLenum condition, GLbitfield flags _retPointer); \
    GLboolean      glApiMacro(IsSync) (_gcArgComma_ GLsync sync); \
    GLvoid         glApiMacro(DeleteSync) (_gcArgComma_ GLsync sync); \
    GLenum         glApiMacro(ClientWaitSync) (_gcArgComma_ GLsync sync, GLbitfield flags, GLuint64 timeout); \
    GLvoid         glApiMacro(WaitSync) (_gcArgComma_ GLsync sync, GLbitfield flags, GLuint64 timeout); \
    GLvoid         glApiMacro(GetInteger64v) (_gcArgComma_ GLenum pname, GLint64 *data); \
    GLvoid         glApiMacro(GetSynciv) (_gcArgComma_ GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values); \
    GLvoid         glApiMacro(GetInteger64i_v) (_gcArgComma_ GLenum target, GLuint index, GLint64 *data); \
    GLvoid         glApiMacro(GetBufferParameteri64v) (_gcArgComma_ GLenum target, GLenum pname, GLint64 *params); \
    GLvoid         glApiMacro(FramebufferTexture) (_gcArgComma_ GLenum target, GLenum attachment, GLuint texture, GLint level); \
    GLvoid         glApiMacro(TexImage2DMultisample) (_gcArgComma_ GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations); \
    GLvoid         glApiMacro(TexImage3DMultisample) (_gcArgComma_ GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations); \
    GLvoid         glApiMacro(GetMultisamplefv) (_gcArgComma_ GLenum pname, GLuint index, GLfloat *val); \
    GLvoid         glApiMacro(SampleMaski) (_gcArgComma_ GLuint maskNumber, GLbitfield mask); \
    /* GL_VERSION_3_3 */ \
    GLvoid         glApiMacro(BindFragDataLocationIndexed) (_gcArgComma_ GLuint program, GLuint colorNumber, GLuint index, const GLchar *name); \
    GLint          glApiMacro(GetFragDataIndex) (_gcArgComma_ GLuint program, const GLchar *name); \
    GLvoid         glApiMacro(GenSamplers) (_gcArgComma_ GLsizei count, GLuint *samplers); \
    GLvoid         glApiMacro(DeleteSamplers) (_gcArgComma_ GLsizei count, const GLuint *samplers); \
    GLboolean      glApiMacro(IsSampler) (_gcArgComma_ GLuint sampler); \
    GLvoid         glApiMacro(BindSampler) (_gcArgComma_ GLuint unit, GLuint sampler); \
    GLvoid         glApiMacro(SamplerParameteri) (_gcArgComma_ GLuint sampler, GLenum pname, GLint param); \
    GLvoid         glApiMacro(SamplerParameteriv) (_gcArgComma_ GLuint sampler, GLenum pname, const GLint *param); \
    GLvoid         glApiMacro(SamplerParameterf) (_gcArgComma_ GLuint sampler, GLenum pname, GLfloat param); \
    GLvoid         glApiMacro(SamplerParameterfv) (_gcArgComma_ GLuint sampler, GLenum pname, const GLfloat *param); \
    GLvoid         glApiMacro(SamplerParameterIiv) (_gcArgComma_ GLuint sampler, GLenum pname, const GLint *param); \
    GLvoid         glApiMacro(SamplerParameterIuiv) (_gcArgComma_ GLuint sampler, GLenum pname, const GLuint *param); \
    GLvoid         glApiMacro(GetSamplerParameteriv) (_gcArgComma_ GLuint sampler, GLenum pname, GLint *params); \
    GLvoid         glApiMacro(GetSamplerParameterIiv) (_gcArgComma_ GLuint sampler, GLenum pname, GLint *params); \
    GLvoid         glApiMacro(GetSamplerParameterfv) (_gcArgComma_ GLuint sampler, GLenum pname, GLfloat *params); \
    GLvoid         glApiMacro(GetSamplerParameterIuiv) (_gcArgComma_ GLuint sampler, GLenum pname, GLuint *params); \
    GLvoid         glApiMacro(QueryCounter) (_gcArgComma_ GLuint id, GLenum target); \
    GLvoid         glApiMacro(GetQueryObjecti64v) (_gcArgComma_ GLuint id, GLenum pname, GLint64 *params); \
    GLvoid         glApiMacro(GetQueryObjectui64v) (_gcArgComma_ GLuint id, GLenum pname, GLuint64 *params); \
    GLvoid         glApiMacro(VertexAttribDivisor) (_gcArgComma_ GLuint index, GLuint divisor); \
    GLvoid         glApiMacro(VertexAttribP1ui) (_gcArgComma_ GLuint index, GLenum type, GLboolean normalized, GLuint value); \
    GLvoid         glApiMacro(VertexAttribP1uiv) (_gcArgComma_ GLuint index, GLenum type, GLboolean normalized, const GLuint *value); \
    GLvoid         glApiMacro(VertexAttribP2ui) (_gcArgComma_ GLuint index, GLenum type, GLboolean normalized, GLuint value); \
    GLvoid         glApiMacro(VertexAttribP2uiv) (_gcArgComma_ GLuint index, GLenum type, GLboolean normalized, const GLuint *value); \
    GLvoid         glApiMacro(VertexAttribP3ui) (_gcArgComma_ GLuint index, GLenum type, GLboolean normalized, GLuint value); \
    GLvoid         glApiMacro(VertexAttribP3uiv) (_gcArgComma_ GLuint index, GLenum type, GLboolean normalized, const GLuint *value); \
    GLvoid         glApiMacro(VertexAttribP4ui) (_gcArgComma_ GLuint index, GLenum type, GLboolean normalized, GLuint value); \
    GLvoid         glApiMacro(VertexAttribP4uiv) (_gcArgComma_ GLuint index, GLenum type, GLboolean normalized, const GLuint *value); \
    GLvoid         glApiMacro(VertexP2ui) (_gcArgComma_ GLenum type, GLuint value); \
    GLvoid         glApiMacro(VertexP2uiv) (_gcArgComma_ GLenum type, const GLuint *value); \
    GLvoid         glApiMacro(VertexP3ui) (_gcArgComma_ GLenum type, GLuint value); \
    GLvoid         glApiMacro(VertexP3uiv) (_gcArgComma_ GLenum type, const GLuint *value); \
    GLvoid         glApiMacro(VertexP4ui) (_gcArgComma_ GLenum type, GLuint value); \
    GLvoid         glApiMacro(VertexP4uiv) (_gcArgComma_ GLenum type, const GLuint *value); \
    GLvoid         glApiMacro(TexCoordP1ui) (_gcArgComma_ GLenum type, GLuint coords); \
    GLvoid         glApiMacro(TexCoordP1uiv) (_gcArgComma_ GLenum type, const GLuint *coords); \
    GLvoid         glApiMacro(TexCoordP2ui) (_gcArgComma_ GLenum type, GLuint coords); \
    GLvoid         glApiMacro(TexCoordP2uiv) (_gcArgComma_ GLenum type, const GLuint *coords); \
    GLvoid         glApiMacro(TexCoordP3ui) (_gcArgComma_ GLenum type, GLuint coords); \
    GLvoid         glApiMacro(TexCoordP3uiv) (_gcArgComma_ GLenum type, const GLuint *coords); \
    GLvoid         glApiMacro(TexCoordP4ui) (_gcArgComma_ GLenum type, GLuint coords); \
    GLvoid         glApiMacro(TexCoordP4uiv) (_gcArgComma_ GLenum type, const GLuint *coords); \
    GLvoid         glApiMacro(MultiTexCoordP1ui) (_gcArgComma_ GLenum texture, GLenum type, GLuint coords); \
    GLvoid         glApiMacro(MultiTexCoordP1uiv) (_gcArgComma_ GLenum texture, GLenum type, const GLuint *coords); \
    GLvoid         glApiMacro(MultiTexCoordP2ui) (_gcArgComma_ GLenum texture, GLenum type, GLuint coords); \
    GLvoid         glApiMacro(MultiTexCoordP2uiv) (_gcArgComma_ GLenum texture, GLenum type, const GLuint *coords); \
    GLvoid         glApiMacro(MultiTexCoordP3ui) (_gcArgComma_ GLenum texture, GLenum type, GLuint coords); \
    GLvoid         glApiMacro(MultiTexCoordP3uiv) (_gcArgComma_ GLenum texture, GLenum type, const GLuint *coords); \
    GLvoid         glApiMacro(MultiTexCoordP4ui) (_gcArgComma_ GLenum texture, GLenum type, GLuint coords); \
    GLvoid         glApiMacro(MultiTexCoordP4uiv) (_gcArgComma_ GLenum texture, GLenum type, const GLuint *coords); \
    GLvoid         glApiMacro(NormalP3ui) (_gcArgComma_ GLenum type, GLuint coords); \
    GLvoid         glApiMacro(NormalP3uiv) (_gcArgComma_ GLenum type, const GLuint *coords); \
    GLvoid         glApiMacro(ColorP3ui) (_gcArgComma_ GLenum type, GLuint color); \
    GLvoid         glApiMacro(ColorP3uiv) (_gcArgComma_ GLenum type, const GLuint *color); \
    GLvoid         glApiMacro(ColorP4ui) (_gcArgComma_ GLenum type, GLuint color); \
    GLvoid         glApiMacro(ColorP4uiv) (_gcArgComma_ GLenum type, const GLuint *color); \
    GLvoid         glApiMacro(SecondaryColorP3ui) (_gcArgComma_ GLenum type, GLuint color); \
    GLvoid         glApiMacro(SecondaryColorP3uiv) (_gcArgComma_ GLenum type, const GLuint *color); \
    /* GL_VERSION_4_0 */ \
    GLvoid         glApiMacro(MinSampleShading) (_gcArgComma_ GLfloat value); \
    GLvoid         glApiMacro(BlendEquationi) (_gcArgComma_ GLuint buf, GLenum mode); \
    GLvoid         glApiMacro(BlendEquationSeparatei) (_gcArgComma_ GLuint buf, GLenum modeRGB, GLenum modeAlpha); \
    GLvoid         glApiMacro(BlendFunci) (_gcArgComma_ GLuint buf, GLenum src, GLenum dst); \
    GLvoid         glApiMacro(BlendFuncSeparatei) (_gcArgComma_ GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha); \
    GLvoid         glApiMacro(DrawArraysIndirect) (_gcArgComma_ GLenum mode, const GLvoid *indirect); \
    GLvoid         glApiMacro(DrawElementsIndirect) (_gcArgComma_ GLenum mode, GLenum type, const GLvoid *indirect); \
    GLvoid         glApiMacro(Uniform1d) (_gcArgComma_ GLint location, GLdouble x); \
    GLvoid         glApiMacro(Uniform2d) (_gcArgComma_ GLint location, GLdouble x, GLdouble y); \
    GLvoid         glApiMacro(Uniform3d) (_gcArgComma_ GLint location, GLdouble x, GLdouble y, GLdouble z); \
    GLvoid         glApiMacro(Uniform4d) (_gcArgComma_ GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w); \
    GLvoid         glApiMacro(Uniform1dv) (_gcArgComma_ GLint location, GLsizei count, const GLdouble *value); \
    GLvoid         glApiMacro(Uniform2dv) (_gcArgComma_ GLint location, GLsizei count, const GLdouble *value); \
    GLvoid         glApiMacro(Uniform3dv) (_gcArgComma_ GLint location, GLsizei count, const GLdouble *value); \
    GLvoid         glApiMacro(Uniform4dv) (_gcArgComma_ GLint location, GLsizei count, const GLdouble *value); \
    GLvoid         glApiMacro(UniformMatrix2dv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLdouble *value); \
    GLvoid         glApiMacro(UniformMatrix3dv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLdouble *value); \
    GLvoid         glApiMacro(UniformMatrix4dv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLdouble *value); \
    GLvoid         glApiMacro(UniformMatrix2x3dv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLdouble *value); \
    GLvoid         glApiMacro(UniformMatrix2x4dv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLdouble *value); \
    GLvoid         glApiMacro(UniformMatrix3x2dv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLdouble *value); \
    GLvoid         glApiMacro(UniformMatrix3x4dv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLdouble *value); \
    GLvoid         glApiMacro(UniformMatrix4x2dv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLdouble *value); \
    GLvoid         glApiMacro(UniformMatrix4x3dv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLdouble *value); \
    GLvoid         glApiMacro(GetUniformdv) (_gcArgComma_ GLuint program, GLint location, GLdouble *params); \
    GLint          glApiMacro(GetSubroutineUniformLocation) (_gcArgComma_ GLuint program, GLenum shadertype, const GLchar *name); \
    GLuint         glApiMacro(GetSubroutineIndex) (_gcArgComma_ GLuint program, GLenum shadertype, const GLchar *name); \
    GLvoid         glApiMacro(GetActiveSubroutineUniformiv) (_gcArgComma_ GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint *values); \
    GLvoid         glApiMacro(GetActiveSubroutineUniformName) (_gcArgComma_ GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name); \
    GLvoid         glApiMacro(GetActiveSubroutineName) (_gcArgComma_ GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name); \
    GLvoid         glApiMacro(UniformSubroutinesuiv) (_gcArgComma_ GLenum shadertype, GLsizei count, const GLuint *indices); \
    GLvoid         glApiMacro(GetUniformSubroutineuiv) (_gcArgComma_ GLenum shadertype, GLint location, GLuint *params); \
    GLvoid         glApiMacro(GetProgramStageiv) (_gcArgComma_ GLuint program, GLenum shadertype, GLenum pname, GLint *values); \
    GLvoid         glApiMacro(PatchParameteri) (_gcArgComma_ GLenum pname, GLint value); \
    GLvoid         glApiMacro(PatchParameterfv) (_gcArgComma_ GLenum pname, const GLfloat *values); \
    GLvoid         glApiMacro(BindTransformFeedback) (_gcArgComma_ GLenum target, GLuint id); \
    GLvoid         glApiMacro(DeleteTransformFeedbacks) (_gcArgComma_ GLsizei n, const GLuint *ids); \
    GLvoid         glApiMacro(GenTransformFeedbacks) (_gcArgComma_ GLsizei n, GLuint *ids); \
    GLboolean      glApiMacro(IsTransformFeedback) (_gcArgComma_ GLuint id); \
    GLvoid         glApiMacro(PauseTransformFeedback) (_gcArgOnly_); \
    GLvoid         glApiMacro(ResumeTransformFeedback) (_gcArgOnly_); \
    GLvoid         glApiMacro(DrawTransformFeedback) (_gcArgComma_ GLenum mode, GLuint id); \
    GLvoid         glApiMacro(DrawTransformFeedbackStream) (_gcArgComma_ GLenum mode, GLuint id, GLuint stream); \
    GLvoid         glApiMacro(BeginQueryIndexed) (_gcArgComma_ GLenum target, GLuint index, GLuint id); \
    GLvoid         glApiMacro(EndQueryIndexed) (_gcArgComma_ GLenum target, GLuint index); \
    GLvoid         glApiMacro(GetQueryIndexediv) (_gcArgComma_ GLenum target, GLuint index, GLenum pname, GLint *params); \
    /* GL_VERSION_4_1, incomplete: defined by later GL version but required by ES */ \
    GLvoid         glApiMacro(ReleaseShaderCompiler) (_gcArgOnly_); \
    GLvoid         glApiMacro(ShaderBinary) (_gcArgComma_ GLsizei count, const GLuint *shaders, GLenum binaryformat, const GLvoid *binary, GLsizei length); \
    GLvoid         glApiMacro(GetShaderPrecisionFormat) (_gcArgComma_ GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision); \
    GLvoid         glApiMacro(DepthRangef) (_gcArgComma_ GLfloat n, GLfloat f); \
    GLvoid         glApiMacro(ClearDepthf) (_gcArgComma_ GLfloat d); \
    GLvoid         glApiMacro(GetProgramBinary) (_gcArgComma_ GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, GLvoid *binary); \
    GLvoid         glApiMacro(ProgramBinary) (_gcArgComma_ GLuint program, GLenum binaryFormat, const GLvoid *binary, GLsizei length); \
    GLvoid         glApiMacro(ProgramParameteri) (_gcArgComma_ GLuint program, GLenum pname, GLint value); \
    GLvoid         glApiMacro(UseProgramStages) (_gcArgComma_ GLuint pipeline, GLbitfield stages, GLuint program); \
    GLvoid         glApiMacro(ActiveShaderProgram) (_gcArgComma_ GLuint pipeline, GLuint program); \
    GLuint         glApiMacro(CreateShaderProgramv) (_gcArgComma_ GLenum type, GLsizei count, const GLchar *const*strings); \
    GLvoid         glApiMacro(BindProgramPipeline) (_gcArgComma_ GLuint pipeline); \
    GLvoid         glApiMacro(DeleteProgramPipelines) (_gcArgComma_ GLsizei n, const GLuint *pipelines); \
    GLvoid         glApiMacro(GenProgramPipelines) (_gcArgComma_ GLsizei n, GLuint *pipelines); \
    GLboolean      glApiMacro(IsProgramPipeline) (_gcArgComma_ GLuint pipeline); \
    GLvoid         glApiMacro(GetProgramPipelineiv) (_gcArgComma_ GLuint pipeline, GLenum pname, GLint *params); \
    GLvoid         glApiMacro(ProgramUniform1i) (_gcArgComma_ GLuint program, GLint location, GLint v0); \
    GLvoid         glApiMacro(ProgramUniform1iv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLint *value); \
    GLvoid         glApiMacro(ProgramUniform1f) (_gcArgComma_ GLuint program, GLint location, GLfloat v0); \
    GLvoid         glApiMacro(ProgramUniform1fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLfloat *value); \
    GLvoid         glApiMacro(ProgramUniform1ui) (_gcArgComma_ GLuint program, GLint location, GLuint v0); \
    GLvoid         glApiMacro(ProgramUniform1uiv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLuint *value); \
    GLvoid         glApiMacro(ProgramUniform2i) (_gcArgComma_ GLuint program, GLint location, GLint v0, GLint v1); \
    GLvoid         glApiMacro(ProgramUniform2iv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLint *value); \
    GLvoid         glApiMacro(ProgramUniform2f) (_gcArgComma_ GLuint program, GLint location, GLfloat v0, GLfloat v1); \
    GLvoid         glApiMacro(ProgramUniform2fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLfloat *value); \
    GLvoid         glApiMacro(ProgramUniform2ui) (_gcArgComma_ GLuint program, GLint location, GLuint v0, GLuint v1); \
    GLvoid         glApiMacro(ProgramUniform2uiv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLuint *value); \
    GLvoid         glApiMacro(ProgramUniform3i) (_gcArgComma_ GLuint program, GLint location, GLint v0, GLint v1, GLint v2); \
    GLvoid         glApiMacro(ProgramUniform3iv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLint *value); \
    GLvoid         glApiMacro(ProgramUniform3f) (_gcArgComma_ GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2); \
    GLvoid         glApiMacro(ProgramUniform3fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLfloat *value); \
    GLvoid         glApiMacro(ProgramUniform3ui) (_gcArgComma_ GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2); \
    GLvoid         glApiMacro(ProgramUniform3uiv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLuint *value); \
    GLvoid         glApiMacro(ProgramUniform4i) (_gcArgComma_ GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3); \
    GLvoid         glApiMacro(ProgramUniform4iv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLint *value); \
    GLvoid         glApiMacro(ProgramUniform4f) (_gcArgComma_ GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3); \
    GLvoid         glApiMacro(ProgramUniform4fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLfloat *value); \
    GLvoid         glApiMacro(ProgramUniform4ui) (_gcArgComma_ GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3); \
    GLvoid         glApiMacro(ProgramUniform4uiv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLuint *value); \
    GLvoid         glApiMacro(ProgramUniformMatrix2fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value); \
    GLvoid         glApiMacro(ProgramUniformMatrix3fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value); \
    GLvoid         glApiMacro(ProgramUniformMatrix4fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value); \
    GLvoid         glApiMacro(ProgramUniformMatrix2x3fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value); \
    GLvoid         glApiMacro(ProgramUniformMatrix3x2fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value); \
    GLvoid         glApiMacro(ProgramUniformMatrix2x4fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value); \
    GLvoid         glApiMacro(ProgramUniformMatrix4x2fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value); \
    GLvoid         glApiMacro(ProgramUniformMatrix3x4fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value); \
    GLvoid         glApiMacro(ProgramUniformMatrix4x3fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value); \
    GLvoid         glApiMacro(ValidateProgramPipeline) (_gcArgComma_ GLuint pipeline); \
    GLvoid         glApiMacro(GetProgramPipelineInfoLog) (_gcArgComma_ GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog); \
    /* GL_VERSION_4_2, incomplete: defined by later GL version but required by ES */ \
    GLvoid         glApiMacro(GetInternalformativ) (_gcArgComma_ GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params); \
    GLvoid         glApiMacro(BindImageTexture) (_gcArgComma_ GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format); \
    GLvoid         glApiMacro(MemoryBarrier) (_gcArgComma_ GLbitfield barriers); \
    GLvoid         glApiMacro(TexStorage2D) (_gcArgComma_ GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height); \
    GLvoid         glApiMacro(TexStorage3D) (_gcArgComma_ GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth); \
    /* GL_VERSION_4_3, incomplete: defined by later GL version but required by ES */ \
    GLvoid         glApiMacro(DispatchCompute) (_gcArgComma_ GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z); \
    GLvoid         glApiMacro(DispatchComputeIndirect) (_gcArgComma_ GLintptr indirect); \
    GLvoid         glApiMacro(CopyImageSubData) (_gcArgComma_ GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth); \
    GLvoid         glApiMacro(FramebufferParameteri) (_gcArgComma_ GLenum target, GLenum pname, GLint param); \
    GLvoid         glApiMacro(GetFramebufferParameteriv) (_gcArgComma_ GLenum target, GLenum pname, GLint *params); \
    GLvoid         glApiMacro(InvalidateFramebuffer) (_gcArgComma_ GLenum target, GLsizei numAttachments, const GLenum *attachments); \
    GLvoid         glApiMacro(InvalidateSubFramebuffer) (_gcArgComma_ GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height); \
    GLvoid         glApiMacro(MultiDrawArraysIndirect) (_gcArgComma_ GLenum mode, const GLvoid *indirect, GLsizei drawcount, GLsizei stride); \
    GLvoid         glApiMacro(MultiDrawElementsIndirect) (_gcArgComma_ GLenum mode, GLenum type, const GLvoid *indirect, GLsizei drawcount, GLsizei stride); \
    GLvoid         glApiMacro(GetProgramInterfaceiv) (_gcArgComma_ GLuint program, GLenum programInterface, GLenum pname, GLint *params); \
    GLuint         glApiMacro(GetProgramResourceIndex) (_gcArgComma_ GLuint program, GLenum programInterface, const GLchar *name); \
    GLvoid         glApiMacro(GetProgramResourceName) (_gcArgComma_ GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name); \
    GLvoid         glApiMacro(GetProgramResourceiv) (_gcArgComma_ GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params); \
    GLint          glApiMacro(GetProgramResourceLocation) (_gcArgComma_ GLuint program, GLenum programInterface, const GLchar *name); \
    GLvoid         glApiMacro(TexBufferRange) (_gcArgComma_ GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size); \
    GLvoid         glApiMacro(TexStorage2DMultisample) (_gcArgComma_ GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations); \
    GLvoid         glApiMacro(TexStorage3DMultisample) (_gcArgComma_ GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations); \
    GLvoid         glApiMacro(BindVertexBuffer) (_gcArgComma_ GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride); \
    GLvoid         glApiMacro(VertexAttribFormat) (_gcArgComma_ GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset); \
    GLvoid         glApiMacro(VertexAttribIFormat) (_gcArgComma_ GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset); \
    GLvoid         glApiMacro(VertexAttribBinding) (_gcArgComma_ GLuint attribindex, GLuint bindingindex); \
    GLvoid         glApiMacro(VertexBindingDivisor) (_gcArgComma_ GLuint bindingindex, GLuint divisor); \
    GLvoid         glApiMacro(DebugMessageControl) (_gcArgComma_ GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled); \
    GLvoid         glApiMacro(DebugMessageInsert) (_gcArgComma_ GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf); \
    GLvoid         glApiMacro(DebugMessageCallback) (_gcArgComma_ GLDEBUGPROC callback, const GLvoid *userParam); \
    GLuint         glApiMacro(GetDebugMessageLog) (_gcArgComma_ GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog); \
    GLvoid         glApiMacro(PushDebugGroup) (_gcArgComma_ GLenum source, GLuint id, GLsizei length, const GLchar *message); \
    GLvoid         glApiMacro(PopDebugGroup) (_gcArgOnly_); \
    GLvoid         glApiMacro(ObjectLabel) (_gcArgComma_ GLenum identifier, GLuint name, GLsizei length, const GLchar *label); \
    GLvoid         glApiMacro(GetObjectLabel) (_gcArgComma_ GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label); \
    GLvoid         glApiMacro(ObjectPtrLabel) (_gcArgComma_ const GLvoid *ptr, GLsizei length, const GLchar *label); \
    GLvoid         glApiMacro(GetObjectPtrLabel) (_gcArgComma_ const GLvoid *ptr, GLsizei bufSize, GLsizei *length, GLchar *label); \
    /* GL_VERSION_4_5, incomplete: defined by later GL version but required by ES */ \
    GLvoid         glApiMacro(MemoryBarrierByRegion) (_gcArgComma_ GLbitfield barriers); \
    GLenum         glApiMacro(GetGraphicsResetStatus) (_gcArgOnly_); \
    GLvoid         glApiMacro(ReadnPixels) (_gcArgComma_ GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLvoid *data); \
    GLvoid         glApiMacro(GetnUniformfv) (_gcArgComma_ GLuint program, GLint location, GLsizei bufSize, GLfloat *params); \
    GLvoid         glApiMacro(GetnUniformiv) (_gcArgComma_ GLuint program, GLint location, GLsizei bufSize, GLint *params); \
    GLvoid         glApiMacro(GetnUniformuiv) (_gcArgComma_ GLuint program, GLint location, GLsizei bufSize, GLuint *params); \
    /* OpenGL ES extensions */ \
    /* ES_VERSION_3_2 */ \
    GLvoid         glApiMacro(BlendBarrier) (_gcArgOnly_); \
    GLvoid         glApiMacro(PrimitiveBoundingBox) (_gcArgComma_ GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW); \
    /* GL_OES_EGL_image */ \
    GLvoid         glApiMacro(EGLImageTargetTexture2DOES) (_gcArgComma_ GLenum target, GLeglImageOES image); \
    GLvoid         glApiMacro(EGLImageTargetRenderbufferStorageOES) (_gcArgComma_ GLenum target, GLeglImageOES image); \
    /* GL_VIV_direct_texture */ \
    GLvoid         glApiMacro(TexDirectVIV) (_gcArgComma_ GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** pixels); \
    GLvoid         glApiMacro(TexDirectInvalidateVIV) (_gcArgComma_ GLenum target); \
    GLvoid         glApiMacro(TexDirectVIVMap) (_gcArgComma_ GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** logical, const GLuint * physical); \
    GLvoid         glApiMacro(TexDirectTiledMapVIV) (_gcArgComma_ GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** logical, const GLuint * physical); \
    /* GL_EXT_multisampled_render_to_texture */ \
    GLvoid         glApiMacro(FramebufferTexture2DMultisampleEXT) (_gcArgComma_ GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples); \
    /* GL_EXT_discard_framebuffer */ \
    GLvoid         glApiMacro(DiscardFramebufferEXT) (_gcArgComma_ GLenum target, GLsizei numAttachments, const GLenum *attachments); \
    /* GL_ARB_shader_objects */ \
    GLvoid         glApiMacro(DeleteObjectARB) (_gcArgComma_ UINT obj); \
    GLvoid         glApiMacro(GetObjectParameterivARB) (_gcArgComma_ UINT obj, GLenum pname, GLint *params); \
    GLvoid         glApiMacro(GetInfoLogARB) (_gcArgComma_ UINT obj, GLsizei maxLength, GLsizei *length, char *infoLog);


#define dipatchMacro(api)   (GL_APIENTRY * api)
#define imProtoMacro(api)   GL_APIENTRY __glim_##api

#define _gcArgComma_  __GLcontext* gc,
#define _gcArgOnly_   __GLcontext* gc
#define _retProgram_
#define _retShader_
#define _retIndex_
#define _retLocation_
#define _retSync_
#define _retPointer


/* Define Immediate API proto */
__GL_API_DISPATCH_FUNCS(imProtoMacro);

/* Define Internal GL/ES API Dispatch Table */
struct __GLdispatchTableRec
{
    __GL_API_DISPATCH_FUNCS(dipatchMacro)
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

typedef struct __GLtraceDispatchTableRec
{
    __GL_API_DISPATCH_FUNCS(dipatchMacro)
} __GLtraceDispatchTable;

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

typedef struct __GLexportFuncTableRec
{
    GLint nEntries;
    __GL_API_DISPATCH_FUNCS(dipatchMacro)
} __GLexportFuncTable;

#undef _gcArgComma_
#undef _gcArgOnly_
#undef _retProgram_
#undef _retShader_
#undef _retIndex_
#undef _retLocation_
#undef _retSync_
#undef _retPointer


GLvoid APIENTRY __glim_DrawArrays_Validate(__GLcontext *gc, GLenum mode, GLint first, GLsizei count);
GLvoid APIENTRY __glim_DrawElements_Validate(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
GLvoid APIENTRY __glim_ArrayElement_Validate(__GLcontext *gc, GLint element);

extern __GLdispatchTable __glImmediateFuncTable;
extern __GLdispatchTable __glListCompileFuncTable;
extern __GLexportFuncTable __glVIV_DispatchFuncTable;

#endif /* VIV_EGL_BUILD */

#endif /* __gc_gl_dispatch_h__ */
