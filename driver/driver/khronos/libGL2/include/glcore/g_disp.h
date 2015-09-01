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


#ifndef __g_disp_h_
#define __g_disp_h_

#define glProc(func)    __glProc_##func
#define glVIV(func)      __glVIV_##func
#define glim(func)      __glim_##func
#define gllc(func)      __gllc_##func
#define glle(func)      __glle_##func
#define glop(func)      __glop_##func
#define mtPut(func)     mtPut_##func
#define mtExe(func)     mtExe_##func
#define mtOp(func)      mtOp_##func
#define glStr(func)     #func

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
/* #if GL_VERSION_1_1 */ \
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
/* #endif */ \
/* #if GL_VERSION_1_2 */ \
    glApiMacro(BlendColor), \
    glApiMacro(BlendEquation), \
    glApiMacro(DrawRangeElements), \
    glApiMacro(ColorTable), \
    glApiMacro(ColorTableParameterfv), \
    glApiMacro(ColorTableParameteriv), \
    glApiMacro(CopyColorTable), \
    glApiMacro(GetColorTable), \
    glApiMacro(GetColorTableParameterfv), \
    glApiMacro(GetColorTableParameteriv), \
    glApiMacro(ColorSubTable), \
    glApiMacro(CopyColorSubTable), \
    glApiMacro(ConvolutionFilter1D), \
    glApiMacro(ConvolutionFilter2D), \
    glApiMacro(ConvolutionParameterf), \
    glApiMacro(ConvolutionParameterfv), \
    glApiMacro(ConvolutionParameteri), \
    glApiMacro(ConvolutionParameteriv), \
    glApiMacro(CopyConvolutionFilter1D), \
    glApiMacro(CopyConvolutionFilter2D), \
    glApiMacro(GetConvolutionFilter), \
    glApiMacro(GetConvolutionParameterfv), \
    glApiMacro(GetConvolutionParameteriv), \
    glApiMacro(GetSeparableFilter), \
    glApiMacro(SeparableFilter2D), \
    glApiMacro(GetHistogram), \
    glApiMacro(GetHistogramParameterfv), \
    glApiMacro(GetHistogramParameteriv), \
    glApiMacro(GetMinmax), \
    glApiMacro(GetMinmaxParameterfv), \
    glApiMacro(GetMinmaxParameteriv), \
    glApiMacro(Histogram), \
    glApiMacro(Minmax), \
    glApiMacro(ResetHistogram), \
    glApiMacro(ResetMinmax), \
    glApiMacro(TexImage3D), \
    glApiMacro(TexSubImage3D), \
    glApiMacro(CopyTexSubImage3D), \
/* #endif */ \
/* #if GL_VERSION_1_3 */ \
    glApiMacro(ActiveTexture), \
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
    glApiMacro(SampleCoverage), \
    glApiMacro(CompressedTexImage3D), \
    glApiMacro(CompressedTexImage2D), \
    glApiMacro(CompressedTexImage1D), \
    glApiMacro(CompressedTexSubImage3D), \
    glApiMacro(CompressedTexSubImage2D), \
    glApiMacro(CompressedTexSubImage1D), \
    glApiMacro(GetCompressedTexImage), \
/* #endif */ \
/* #if GL_VERSION_1_4 */ \
    glApiMacro(BlendFuncSeparate), \
    glApiMacro(FogCoordf), \
    glApiMacro(FogCoordfv), \
    glApiMacro(FogCoordd), \
    glApiMacro(FogCoorddv), \
    glApiMacro(FogCoordPointer), \
    glApiMacro(MultiDrawArrays), \
    glApiMacro(MultiDrawElements), \
    glApiMacro(PointParameterf), \
    glApiMacro(PointParameterfv), \
    glApiMacro(PointParameteri), \
    glApiMacro(PointParameteriv), \
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
/* #endif */ \
/* #if GL_VERSION_1_5 */ \
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
/* #endif */ \
/* #if GL_VERSION_2_0 */ \
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
/* #endif */ \
/* #if GL_VERSION_2_1 */ \
    glApiMacro(UniformMatrix2x3fv), \
    glApiMacro(UniformMatrix2x4fv), \
    glApiMacro(UniformMatrix3x2fv), \
    glApiMacro(UniformMatrix3x4fv), \
    glApiMacro(UniformMatrix4x2fv), \
    glApiMacro(UniformMatrix4x3fv), \
/* #endif */ \
/* #if GL_ARB_vertex_program */ \
    glApiMacro(ProgramStringARB), \
    glApiMacro(BindProgramARB), \
    glApiMacro(DeleteProgramsARB), \
    glApiMacro(GenProgramsARB), \
    glApiMacro(ProgramEnvParameter4dARB), \
    glApiMacro(ProgramEnvParameter4dvARB), \
    glApiMacro(ProgramEnvParameter4fARB), \
    glApiMacro(ProgramEnvParameter4fvARB), \
    glApiMacro(ProgramLocalParameter4dARB), \
    glApiMacro(ProgramLocalParameter4dvARB), \
    glApiMacro(ProgramLocalParameter4fARB), \
    glApiMacro(ProgramLocalParameter4fvARB), \
    glApiMacro(GetProgramEnvParameterdvARB), \
    glApiMacro(GetProgramEnvParameterfvARB), \
    glApiMacro(GetProgramLocalParameterdvARB), \
    glApiMacro(GetProgramLocalParameterfvARB), \
    glApiMacro(GetProgramivARB), \
    glApiMacro(GetProgramStringARB), \
    glApiMacro(IsProgramARB), \
/* #endif */ \
/* #if GL_ARB_shader_object */ \
    glApiMacro(DeleteObjectARB), \
    glApiMacro(GetHandleARB), \
    glApiMacro(GetInfoLogARB), \
    glApiMacro(GetObjectParameterfvARB), \
    glApiMacro(GetObjectParameterivARB), \
/* #endif */ \
/* #if GL_ATI_vertex_array_object */ \
    glApiMacro(NewObjectBufferATI), \
    glApiMacro(IsObjectBufferATI), \
    glApiMacro(UpdateObjectBufferATI), \
    glApiMacro(GetObjectBufferfvATI), \
    glApiMacro(GetObjectBufferivATI), \
    glApiMacro(FreeObjectBufferATI), \
    glApiMacro(ArrayObjectATI), \
    glApiMacro(GetArrayObjectfvATI), \
    glApiMacro(GetArrayObjectivATI), \
    glApiMacro(VariantArrayObjectATI), \
    glApiMacro(GetVariantArrayObjectfvATI), \
    glApiMacro(GetVariantArrayObjectivATI), \
/* #endif */ \
/* #if GL_ATI_vertex_attrib_array_object */ \
    glApiMacro(VertexAttribArrayObjectATI), \
    glApiMacro(GetVertexAttribArrayObjectfvATI), \
    glApiMacro(GetVertexAttribArrayObjectivATI), \
/* #endif */ \
/* #if GL_ATI_element_array */ \
    glApiMacro(ElementPointerATI), \
    glApiMacro(DrawElementArrayATI), \
    glApiMacro(DrawRangeElementArrayATI), \
/* #endif */ \
/* #if GL_EXT_stencil_two_side */ \
    glApiMacro(ActiveStencilFaceEXT), \
/* #endif */ \
    glApiMacro(AddSwapHintRectWIN), \
/* #if GL_EXT_depth_bounds_test */ \
    glApiMacro(DepthBoundsEXT), \
/* #endif */ \
/* #if GL_EXT_framebuffer_object */ \
    glApiMacro(IsRenderbufferEXT), \
    glApiMacro(BindRenderbufferEXT), \
    glApiMacro(DeleteRenderbuffersEXT), \
    glApiMacro(GenRenderbuffersEXT), \
    glApiMacro(RenderbufferStorageEXT), \
    glApiMacro(GetRenderbufferParameterivEXT), \
    glApiMacro(IsFramebufferEXT), \
    glApiMacro(BindFramebufferEXT), \
    glApiMacro(DeleteFramebuffersEXT), \
    glApiMacro(GenFramebuffersEXT), \
    glApiMacro(CheckFramebufferStatusEXT), \
    glApiMacro(FramebufferTexture1DEXT), \
    glApiMacro(FramebufferTexture2DEXT), \
    glApiMacro(FramebufferTexture3DEXT), \
    glApiMacro(FramebufferRenderbufferEXT), \
    glApiMacro(GetFramebufferAttachmentParameterivEXT), \
    glApiMacro(GenerateMipmapEXT), \
/* #if GL_EXT_framebuffer_blit */ \
    glApiMacro(BlitFramebufferEXT), \
/* #if GL_EXT_framebuffer_multisample */ \
    glApiMacro(RenderbufferStorageMultisampleEXT), \
/* #endif */ \
/* #endif */ \
/* #endif */ \
/* #if GL_NV_occlusion_query */ \
    glApiMacro(BeginQueryNV), \
    glApiMacro(EndQueryNV), \
/* #endif */ \
/* #if GL_EXT_bindable_uniform */ \
    glApiMacro(UniformBufferEXT), \
    glApiMacro(GetUniformBufferSizeEXT), \
    glApiMacro(GetUniformOffsetEXT), \
/* #endif */ \
/* #if GL_EXT_texture_integer */ \
    glApiMacro(ClearColorIiEXT), \
    glApiMacro(ClearColorIuiEXT), \
    glApiMacro(TexParameterIivEXT), \
    glApiMacro(TexParameterIuivEXT), \
    glApiMacro(GetTexParameterIivEXT), \
    glApiMacro(GetTexParameterIuivEXT), \
/* #endif */ \
/* #if GL_EXT_gpu_shader4 */ \
    glApiMacro(VertexAttribI1iEXT), \
    glApiMacro(VertexAttribI2iEXT), \
    glApiMacro(VertexAttribI3iEXT), \
    glApiMacro(VertexAttribI4iEXT), \
    glApiMacro(VertexAttribI1uiEXT), \
    glApiMacro(VertexAttribI2uiEXT), \
    glApiMacro(VertexAttribI3uiEXT), \
    glApiMacro(VertexAttribI4uiEXT), \
    glApiMacro(VertexAttribI1ivEXT), \
    glApiMacro(VertexAttribI2ivEXT), \
    glApiMacro(VertexAttribI3ivEXT), \
    glApiMacro(VertexAttribI4ivEXT), \
    glApiMacro(VertexAttribI1uivEXT), \
    glApiMacro(VertexAttribI2uivEXT), \
    glApiMacro(VertexAttribI3uivEXT), \
    glApiMacro(VertexAttribI4uivEXT), \
    glApiMacro(VertexAttribI4bvEXT), \
    glApiMacro(VertexAttribI4svEXT), \
    glApiMacro(VertexAttribI4ubvEXT), \
    glApiMacro(VertexAttribI4usvEXT), \
    glApiMacro(VertexAttribIPointerEXT), \
    glApiMacro(GetVertexAttribIivEXT), \
    glApiMacro(GetVertexAttribIuivEXT), \
    glApiMacro(Uniform1uiEXT), \
    glApiMacro(Uniform2uiEXT), \
    glApiMacro(Uniform3uiEXT), \
    glApiMacro(Uniform4uiEXT), \
    glApiMacro(Uniform1uivEXT), \
    glApiMacro(Uniform2uivEXT), \
    glApiMacro(Uniform3uivEXT), \
    glApiMacro(Uniform4uivEXT), \
    glApiMacro(GetUniformuivEXT), \
    glApiMacro(BindFragDataLocationEXT), \
    glApiMacro(GetFragDataLocationEXT), \
/* #endif */ \
/* #if GL_EXT_geometry_shader4 */ \
    glApiMacro(ProgramParameteriEXT), \
    glApiMacro(FramebufferTextureEXT), \
    glApiMacro(FramebufferTextureLayerEXT), \
    glApiMacro(FramebufferTextureFaceEXT), \
/* #endif */ \
/* #if GL_EXT_draw_buffers2 */ \
    glApiMacro(ColorMaskIndexedEXT), \
    glApiMacro(GetBooleanIndexedvEXT), \
    glApiMacro(GetIntegerIndexedvEXT), \
    glApiMacro(EnableIndexedEXT), \
    glApiMacro(DisableIndexedEXT), \
    glApiMacro(IsEnabledIndexedEXT), \
    /*endif */ \
/* #if GL_EXT_texture_buffer_object */ \
    glApiMacro(TexBufferEXT), \
/* #endif */ \
/* #if GL_EXT_gpu_program_parameters */ \
    glApiMacro(ProgramEnvParameters4fvEXT), \
    glApiMacro(ProgramLocalParameters4fvEXT), \
/* #endif */ \
/* #if GL_EXT_draw_instanced */ \
    glApiMacro(DrawArraysInstancedEXT), \
    glApiMacro(DrawElementsInstancedEXT), \
/* #endif */ \
/* #if GL_ARB_color_buffer_float */ \
    glApiMacro(ClampColorARB), \
/* #endif */ \
/* #if GL_EXT_timer_query */ \
    glApiMacro(GetQueryObjecti64vEXT), \
    glApiMacro(GetQueryObjectui64vEXT), \
/* #endif */ \
/* #if GL_ATI_separate_stencil */ \
    glApiMacro(StencilFuncSeparateATI),\
/* #endif */


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
/* #if GL_VERSION_1_1 */ \
    glApiMacro(BindTexture), \
    glApiMacro(PolygonOffset), \
    glApiMacro(CopyTexImage1D), \
    glApiMacro(CopyTexImage2D), \
    glApiMacro(CopyTexSubImage1D), \
    glApiMacro(CopyTexSubImage2D), \
    glApiMacro(PrioritizeTextures), \
    glApiMacro(TexSubImage1D), \
    glApiMacro(TexSubImage2D), \
/* #endif */ \
/* #if GL_VERSION_1_2 */ \
    glApiMacro(BlendColor), \
    glApiMacro(BlendEquation), \
    glApiMacro(ColorTable), \
    glApiMacro(ColorTableParameterfv), \
    glApiMacro(ColorTableParameteriv), \
    glApiMacro(CopyColorTable), \
    glApiMacro(ColorSubTable), \
    glApiMacro(CopyColorSubTable), \
    glApiMacro(ConvolutionFilter1D), \
    glApiMacro(ConvolutionFilter2D), \
    glApiMacro(ConvolutionParameterfv), \
    glApiMacro(ConvolutionParameteriv), \
    glApiMacro(CopyConvolutionFilter1D), \
    glApiMacro(CopyConvolutionFilter2D), \
    glApiMacro(SeparableFilter2D), \
    glApiMacro(Histogram), \
    glApiMacro(Minmax), \
    glApiMacro(ResetHistogram), \
    glApiMacro(ResetMinmax), \
    glApiMacro(TexImage3D), \
    glApiMacro(TexSubImage3D), \
    glApiMacro(CopyTexSubImage3D), \
/* #endif */ \
/* #if GL_VERSION_1_3 */ \
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
/* #endif */ \
/* #if GL_VERSION_1_4 */ \
    glApiMacro(BlendFuncSeparate), \
    glApiMacro(FogCoordf), \
    glApiMacro(PointParameterfv), \
    glApiMacro(PointParameteriv), \
    glApiMacro(SecondaryColor3fv), \
    glApiMacro(WindowPos2fv), \
    glApiMacro(WindowPos3fv), \
/* #endif */ \
/* #if GL_VERSION_1_5 */ \
    glApiMacro(BeginQuery), \
    glApiMacro(EndQuery), \
/* #endif */ \
/* #if GL_VERSION_2_0 */ \
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
/* #endif */ \
/* #if GL_VERSION_2_1 */ \
    glApiMacro(UniformMatrix2x3fv), \
    glApiMacro(UniformMatrix2x4fv), \
    glApiMacro(UniformMatrix3x2fv), \
    glApiMacro(UniformMatrix3x4fv), \
    glApiMacro(UniformMatrix4x2fv), \
    glApiMacro(UniformMatrix4x3fv), \
/* #endif */ \
/* #if GL_ARB_vertex_program */ \
    glApiMacro(BindProgramARB), \
    glApiMacro(ProgramEnvParameter4dARB), \
    glApiMacro(ProgramEnvParameter4dvARB), \
    glApiMacro(ProgramEnvParameter4fARB), \
    glApiMacro(ProgramEnvParameter4fvARB), \
    glApiMacro(ProgramLocalParameter4dARB), \
    glApiMacro(ProgramLocalParameter4dvARB), \
    glApiMacro(ProgramLocalParameter4fARB), \
    glApiMacro(ProgramLocalParameter4fvARB), \
/* #endif */ \
/* #if GL_ATI_element_array */ \
    glApiMacro(DrawElementArrayATI), \
    glApiMacro(DrawRangeElementArrayATI), \
/* #endif */ \
/* #if GL_EXT_stencil_two_side */ \
    glApiMacro(ActiveStencilFaceEXT), \
/* #endif */ \
/* #if GL_EXT_depth_bounds_test */ \
    glApiMacro(DepthBoundsEXT), \
/* #endif */ \
/* #if GL_NV_occlusion_query */ \
    glApiMacro(BeginQueryNV), \
    glApiMacro(EndQueryNV), \
/* #endif */ \
/* #if GL_EXT_texture_integer */ \
    glApiMacro(ClearColorIiEXT), \
    glApiMacro(ClearColorIuiEXT), \
    glApiMacro(TexParameterIivEXT), \
    glApiMacro(TexParameterIuivEXT), \
/* #endif */ \
/* #if GL_EXT_gpu_shader4 */ \
    glApiMacro(Uniform1uiEXT), \
    glApiMacro(Uniform2uiEXT), \
    glApiMacro(Uniform3uiEXT), \
    glApiMacro(Uniform4uiEXT), \
    glApiMacro(Uniform1uivEXT), \
    glApiMacro(Uniform2uivEXT), \
    glApiMacro(Uniform3uivEXT), \
    glApiMacro(Uniform4uivEXT), \
/* #endif */ \
/* #if GL_EXT_geometry_shader4 */ \
    glApiMacro(FramebufferTextureEXT), \
    glApiMacro(FramebufferTextureLayerEXT), \
    glApiMacro(FramebufferTextureFaceEXT), \
/* #endif */ \
/* #if GL_EXT_draw_buffers2 */ \
    glApiMacro(ColorMaskIndexedEXT), \
    glApiMacro(EnableIndexedEXT), \
    glApiMacro(DisableIndexedEXT), \
/* #endif */ \
/* #if GL_EXT_gpu_program_parameters */ \
    glApiMacro(ProgramEnvParameters4fvEXT), \
    glApiMacro(ProgramLocalParameters4fvEXT), \
/* #endif */ \
/* #if GL_ARB_color_buffer_float */ \
    glApiMacro(ClampColorARB), \
/* #endif */ \
/* #if GL_ATI_separate_stencil */ \
    glApiMacro(StencilFuncSeparateATI), \
/* #endif */

/* Enum table for __glop_FuncName (Ex: __glop_NewList)
 */
enum {
    __GL_LISTEXEC_ENTRIES(glop,=0)
    __glop_PrimContinue,
};


typedef struct __GLdispatchTableRec {
    GLvoid (APIENTRY *NewList) (GLuint list, GLenum mode);
    GLvoid (APIENTRY *EndList) (GLvoid);
    GLvoid (APIENTRY *CallList) (GLuint list);
    GLvoid (APIENTRY *CallLists) (GLsizei n, GLenum type, const GLvoid *lists);
    GLvoid (APIENTRY *DeleteLists) (GLuint list, GLsizei range);
    GLuint (APIENTRY *GenLists) (GLsizei range);
    GLvoid (APIENTRY *ListBase) (GLuint base);
    GLvoid (APIENTRY *Begin) (GLenum mode);
    GLvoid (APIENTRY *Bitmap) (GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
    GLvoid (APIENTRY *Color3b) (GLbyte red, GLbyte green, GLbyte blue);
    GLvoid (APIENTRY *Color3bv) (const GLbyte *v);
    GLvoid (APIENTRY *Color3d) (GLdouble red, GLdouble green, GLdouble blue);
    GLvoid (APIENTRY *Color3dv) (const GLdouble *v);
    GLvoid (APIENTRY *Color3f) (GLfloat red, GLfloat green, GLfloat blue);
    GLvoid (APIENTRY *Color3fv) (const GLfloat *v);
    GLvoid (APIENTRY *Color3i) (GLint red, GLint green, GLint blue);
    GLvoid (APIENTRY *Color3iv) (const GLint *v);
    GLvoid (APIENTRY *Color3s) (GLshort red, GLshort green, GLshort blue);
    GLvoid (APIENTRY *Color3sv) (const GLshort *v);
    GLvoid (APIENTRY *Color3ub) (GLubyte red, GLubyte green, GLubyte blue);
    GLvoid (APIENTRY *Color3ubv) (const GLubyte *v);
    GLvoid (APIENTRY *Color3ui) (GLuint red, GLuint green, GLuint blue);
    GLvoid (APIENTRY *Color3uiv) (const GLuint *v);
    GLvoid (APIENTRY *Color3us) (GLushort red, GLushort green, GLushort blue);
    GLvoid (APIENTRY *Color3usv) (const GLushort *v);
    GLvoid (APIENTRY *Color4b) (GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
    GLvoid (APIENTRY *Color4bv) (const GLbyte *v);
    GLvoid (APIENTRY *Color4d) (GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
    GLvoid (APIENTRY *Color4dv) (const GLdouble *v);
    GLvoid (APIENTRY *Color4f) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    GLvoid (APIENTRY *Color4fv) (const GLfloat *v);
    GLvoid (APIENTRY *Color4i) (GLint red, GLint green, GLint blue, GLint alpha);
    GLvoid (APIENTRY *Color4iv) (const GLint *v);
    GLvoid (APIENTRY *Color4s) (GLshort red, GLshort green, GLshort blue, GLshort alpha);
    GLvoid (APIENTRY *Color4sv) (const GLshort *v);
    GLvoid (APIENTRY *Color4ub) (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
    GLvoid (APIENTRY *Color4ubv) (const GLubyte *v);
    GLvoid (APIENTRY *Color4ui) (GLuint red, GLuint green, GLuint blue, GLuint alpha);
    GLvoid (APIENTRY *Color4uiv) (const GLuint *v);
    GLvoid (APIENTRY *Color4us) (GLushort red, GLushort green, GLushort blue, GLushort alpha);
    GLvoid (APIENTRY *Color4usv) (const GLushort *v);
    GLvoid (APIENTRY *EdgeFlag) (GLboolean flag);
    GLvoid (APIENTRY *EdgeFlagv) (const GLboolean *flag);
    GLvoid (APIENTRY *End) (GLvoid);
    GLvoid (APIENTRY *Indexd) (GLdouble c);
    GLvoid (APIENTRY *Indexdv) (const GLdouble *c);
    GLvoid (APIENTRY *Indexf) (GLfloat c);
    GLvoid (APIENTRY *Indexfv) (const GLfloat *c);
    GLvoid (APIENTRY *Indexi) (GLint c);
    GLvoid (APIENTRY *Indexiv) (const GLint *c);
    GLvoid (APIENTRY *Indexs) (GLshort c);
    GLvoid (APIENTRY *Indexsv) (const GLshort *c);
    GLvoid (APIENTRY *Normal3b) (GLbyte nx, GLbyte ny, GLbyte nz);
    GLvoid (APIENTRY *Normal3bv) (const GLbyte *v);
    GLvoid (APIENTRY *Normal3d) (GLdouble nx, GLdouble ny, GLdouble nz);
    GLvoid (APIENTRY *Normal3dv) (const GLdouble *v);
    GLvoid (APIENTRY *Normal3f) (GLfloat nx, GLfloat ny, GLfloat nz);
    GLvoid (APIENTRY *Normal3fv) (const GLfloat *v);
    GLvoid (APIENTRY *Normal3i) (GLint nx, GLint ny, GLint nz);
    GLvoid (APIENTRY *Normal3iv) (const GLint *v);
    GLvoid (APIENTRY *Normal3s) (GLshort nx, GLshort ny, GLshort nz);
    GLvoid (APIENTRY *Normal3sv) (const GLshort *v);
    GLvoid (APIENTRY *RasterPos2d) (GLdouble x, GLdouble y);
    GLvoid (APIENTRY *RasterPos2dv) (const GLdouble *v);
    GLvoid (APIENTRY *RasterPos2f) (GLfloat x, GLfloat y);
    GLvoid (APIENTRY *RasterPos2fv) (const GLfloat *v);
    GLvoid (APIENTRY *RasterPos2i) (GLint x, GLint y);
    GLvoid (APIENTRY *RasterPos2iv) (const GLint *v);
    GLvoid (APIENTRY *RasterPos2s) (GLshort x, GLshort y);
    GLvoid (APIENTRY *RasterPos2sv) (const GLshort *v);
    GLvoid (APIENTRY *RasterPos3d) (GLdouble x, GLdouble y, GLdouble z);
    GLvoid (APIENTRY *RasterPos3dv) (const GLdouble *v);
    GLvoid (APIENTRY *RasterPos3f) (GLfloat x, GLfloat y, GLfloat z);
    GLvoid (APIENTRY *RasterPos3fv) (const GLfloat *v);
    GLvoid (APIENTRY *RasterPos3i) (GLint x, GLint y, GLint z);
    GLvoid (APIENTRY *RasterPos3iv) (const GLint *v);
    GLvoid (APIENTRY *RasterPos3s) (GLshort x, GLshort y, GLshort z);
    GLvoid (APIENTRY *RasterPos3sv) (const GLshort *v);
    GLvoid (APIENTRY *RasterPos4d) (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    GLvoid (APIENTRY *RasterPos4dv) (const GLdouble *v);
    GLvoid (APIENTRY *RasterPos4f) (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    GLvoid (APIENTRY *RasterPos4fv) (const GLfloat *v);
    GLvoid (APIENTRY *RasterPos4i) (GLint x, GLint y, GLint z, GLint w);
    GLvoid (APIENTRY *RasterPos4iv) (const GLint *v);
    GLvoid (APIENTRY *RasterPos4s) (GLshort x, GLshort y, GLshort z, GLshort w);
    GLvoid (APIENTRY *RasterPos4sv) (const GLshort *v);
    GLvoid (APIENTRY *Rectd) (GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
    GLvoid (APIENTRY *Rectdv) (const GLdouble *v1, const GLdouble *v2);
    GLvoid (APIENTRY *Rectf) (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
    GLvoid (APIENTRY *Rectfv) (const GLfloat *v1, const GLfloat *v2);
    GLvoid (APIENTRY *Recti) (GLint x1, GLint y1, GLint x2, GLint y2);
    GLvoid (APIENTRY *Rectiv) (const GLint *v1, const GLint *v2);
    GLvoid (APIENTRY *Rects) (GLshort x1, GLshort y1, GLshort x2, GLshort y2);
    GLvoid (APIENTRY *Rectsv) (const GLshort *v1, const GLshort *v2);
    GLvoid (APIENTRY *TexCoord1d) (GLdouble s);
    GLvoid (APIENTRY *TexCoord1dv) (const GLdouble *v);
    GLvoid (APIENTRY *TexCoord1f) (GLfloat s);
    GLvoid (APIENTRY *TexCoord1fv) (const GLfloat *v);
    GLvoid (APIENTRY *TexCoord1i) (GLint s);
    GLvoid (APIENTRY *TexCoord1iv) (const GLint *v);
    GLvoid (APIENTRY *TexCoord1s) (GLshort s);
    GLvoid (APIENTRY *TexCoord1sv) (const GLshort *v);
    GLvoid (APIENTRY *TexCoord2d) (GLdouble s, GLdouble t);
    GLvoid (APIENTRY *TexCoord2dv) (const GLdouble *v);
    GLvoid (APIENTRY *TexCoord2f) (GLfloat s, GLfloat t);
    GLvoid (APIENTRY *TexCoord2fv) (const GLfloat *v);
    GLvoid (APIENTRY *TexCoord2i) (GLint s, GLint t);
    GLvoid (APIENTRY *TexCoord2iv) (const GLint *v);
    GLvoid (APIENTRY *TexCoord2s) (GLshort s, GLshort t);
    GLvoid (APIENTRY *TexCoord2sv) (const GLshort *v);
    GLvoid (APIENTRY *TexCoord3d) (GLdouble s, GLdouble t, GLdouble r);
    GLvoid (APIENTRY *TexCoord3dv) (const GLdouble *v);
    GLvoid (APIENTRY *TexCoord3f) (GLfloat s, GLfloat t, GLfloat r);
    GLvoid (APIENTRY *TexCoord3fv) (const GLfloat *v);
    GLvoid (APIENTRY *TexCoord3i) (GLint s, GLint t, GLint r);
    GLvoid (APIENTRY *TexCoord3iv) (const GLint *v);
    GLvoid (APIENTRY *TexCoord3s) (GLshort s, GLshort t, GLshort r);
    GLvoid (APIENTRY *TexCoord3sv) (const GLshort *v);
    GLvoid (APIENTRY *TexCoord4d) (GLdouble s, GLdouble t, GLdouble r, GLdouble q);
    GLvoid (APIENTRY *TexCoord4dv) (const GLdouble *v);
    GLvoid (APIENTRY *TexCoord4f) (GLfloat s, GLfloat t, GLfloat r, GLfloat q);
    GLvoid (APIENTRY *TexCoord4fv) (const GLfloat *v);
    GLvoid (APIENTRY *TexCoord4i) (GLint s, GLint t, GLint r, GLint q);
    GLvoid (APIENTRY *TexCoord4iv) (const GLint *v);
    GLvoid (APIENTRY *TexCoord4s) (GLshort s, GLshort t, GLshort r, GLshort q);
    GLvoid (APIENTRY *TexCoord4sv) (const GLshort *v);
    GLvoid (APIENTRY *Vertex2d) (GLdouble x, GLdouble y);
    GLvoid (APIENTRY *Vertex2dv) (const GLdouble *v);
    GLvoid (APIENTRY *Vertex2f) (GLfloat x, GLfloat y);
    GLvoid (APIENTRY *Vertex2fv) (const GLfloat *v);
    GLvoid (APIENTRY *Vertex2i) (GLint x, GLint y);
    GLvoid (APIENTRY *Vertex2iv) (const GLint *v);
    GLvoid (APIENTRY *Vertex2s) (GLshort x, GLshort y);
    GLvoid (APIENTRY *Vertex2sv) (const GLshort *v);
    GLvoid (APIENTRY *Vertex3d) (GLdouble x, GLdouble y, GLdouble z);
    GLvoid (APIENTRY *Vertex3dv) (const GLdouble *v);
    GLvoid (APIENTRY *Vertex3f) (GLfloat x, GLfloat y, GLfloat z);
    GLvoid (APIENTRY *Vertex3fv) (const GLfloat *v);
    GLvoid (APIENTRY *Vertex3i) (GLint x, GLint y, GLint z);
    GLvoid (APIENTRY *Vertex3iv) (const GLint *v);
    GLvoid (APIENTRY *Vertex3s) (GLshort x, GLshort y, GLshort z);
    GLvoid (APIENTRY *Vertex3sv) (const GLshort *v);
    GLvoid (APIENTRY *Vertex4d) (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    GLvoid (APIENTRY *Vertex4dv) (const GLdouble *v);
    GLvoid (APIENTRY *Vertex4f) (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    GLvoid (APIENTRY *Vertex4fv) (const GLfloat *v);
    GLvoid (APIENTRY *Vertex4i) (GLint x, GLint y, GLint z, GLint w);
    GLvoid (APIENTRY *Vertex4iv) (const GLint *v);
    GLvoid (APIENTRY *Vertex4s) (GLshort x, GLshort y, GLshort z, GLshort w);
    GLvoid (APIENTRY *Vertex4sv) (const GLshort *v);
    GLvoid (APIENTRY *ClipPlane) (GLenum plane, const GLdouble *equation);
    GLvoid (APIENTRY *ColorMaterial) (GLenum face, GLenum mode);
    GLvoid (APIENTRY *CullFace) (GLenum mode);
    GLvoid (APIENTRY *Fogf) (GLenum pname, GLfloat param);
    GLvoid (APIENTRY *Fogfv) (GLenum pname, const GLfloat *params);
    GLvoid (APIENTRY *Fogi) (GLenum pname, GLint param);
    GLvoid (APIENTRY *Fogiv) (GLenum pname, const GLint *params);
    GLvoid (APIENTRY *FrontFace) (GLenum mode);
    GLvoid (APIENTRY *Hint) (GLenum target, GLenum mode);
    GLvoid (APIENTRY *Lightf) (GLenum light, GLenum pname, GLfloat param);
    GLvoid (APIENTRY *Lightfv) (GLenum light, GLenum pname, const GLfloat *params);
    GLvoid (APIENTRY *Lighti) (GLenum light, GLenum pname, GLint param);
    GLvoid (APIENTRY *Lightiv) (GLenum light, GLenum pname, const GLint *params);
    GLvoid (APIENTRY *LightModelf) (GLenum pname, GLfloat param);
    GLvoid (APIENTRY *LightModelfv) (GLenum pname, const GLfloat *params);
    GLvoid (APIENTRY *LightModeli) (GLenum pname, GLint param);
    GLvoid (APIENTRY *LightModeliv) (GLenum pname, const GLint *params);
    GLvoid (APIENTRY *LineStipple) (GLint factor, GLushort pattern);
    GLvoid (APIENTRY *LineWidth) (GLfloat width);
    GLvoid (APIENTRY *Materialf) (GLenum face, GLenum pname, GLfloat param);
    GLvoid (APIENTRY *Materialfv) (GLenum face, GLenum pname, const GLfloat *params);
    GLvoid (APIENTRY *Materiali) (GLenum face, GLenum pname, GLint param);
    GLvoid (APIENTRY *Materialiv) (GLenum face, GLenum pname, const GLint *params);
    GLvoid (APIENTRY *PointSize) (GLfloat size);
    GLvoid (APIENTRY *PolygonMode) (GLenum face, GLenum mode);
    GLvoid (APIENTRY *PolygonStipple) (const GLubyte *mask);
    GLvoid (APIENTRY *Scissor) (GLint x, GLint y, GLsizei width, GLsizei height);
    GLvoid (APIENTRY *ShadeModel) (GLenum mode);
    GLvoid (APIENTRY *TexParameterf) (GLenum target, GLenum pname, GLfloat param);
    GLvoid (APIENTRY *TexParameterfv) (GLenum target, GLenum pname, const GLfloat *params);
    GLvoid (APIENTRY *TexParameteri) (GLenum target, GLenum pname, GLint param);
    GLvoid (APIENTRY *TexParameteriv) (GLenum target, GLenum pname, const GLint *params);
    GLvoid (APIENTRY *TexImage1D) (GLenum target, GLint level, GLint components, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
    GLvoid (APIENTRY *TexImage2D) (GLenum target, GLint level, GLint components, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
    GLvoid (APIENTRY *TexEnvf) (GLenum target, GLenum pname, GLfloat param);
    GLvoid (APIENTRY *TexEnvfv) (GLenum target, GLenum pname, const GLfloat *params);
    GLvoid (APIENTRY *TexEnvi) (GLenum target, GLenum pname, GLint param);
    GLvoid (APIENTRY *TexEnviv) (GLenum target, GLenum pname, const GLint *params);
    GLvoid (APIENTRY *TexGend) (GLenum coord, GLenum pname, GLdouble param);
    GLvoid (APIENTRY *TexGendv) (GLenum coord, GLenum pname, const GLdouble *params);
    GLvoid (APIENTRY *TexGenf) (GLenum coord, GLenum pname, GLfloat param);
    GLvoid (APIENTRY *TexGenfv) (GLenum coord, GLenum pname, const GLfloat *params);
    GLvoid (APIENTRY *TexGeni) (GLenum coord, GLenum pname, GLint param);
    GLvoid (APIENTRY *TexGeniv) (GLenum coord, GLenum pname, const GLint *params);
    GLvoid (APIENTRY *FeedbackBuffer) (GLsizei size, GLenum type, GLfloat *buffer);
    GLvoid (APIENTRY *SelectBuffer) (GLsizei size, GLuint *buffer);
    GLint (APIENTRY *RenderMode) (GLenum mode);
    GLvoid (APIENTRY *InitNames) (GLvoid);
    GLvoid (APIENTRY *LoadName) (GLuint name);
    GLvoid (APIENTRY *PassThrough) (GLfloat token);
    GLvoid (APIENTRY *PopName) (GLvoid);
    GLvoid (APIENTRY *PushName) (GLuint name);
    GLvoid (APIENTRY *DrawBuffer) (GLenum mode);
    GLvoid (APIENTRY *Clear) (GLbitfield mask);
    GLvoid (APIENTRY *ClearAccum) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    GLvoid (APIENTRY *ClearIndex) (GLfloat c);
    GLvoid (APIENTRY *ClearColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
    GLvoid (APIENTRY *ClearStencil) (GLint s);
    GLvoid (APIENTRY *ClearDepth) (GLclampd depth);
    GLvoid (APIENTRY *StencilMask) (GLuint mask);
    GLvoid (APIENTRY *ColorMask) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
    GLvoid (APIENTRY *DepthMask) (GLboolean flag);
    GLvoid (APIENTRY *IndexMask) (GLuint mask);
    GLvoid (APIENTRY *Accum) (GLenum op, GLfloat value);
    GLvoid (APIENTRY *Disable) (GLenum cap);
    GLvoid (APIENTRY *Enable) (GLenum cap);
    GLvoid (APIENTRY *Finish) (GLvoid);
    GLvoid (APIENTRY *Flush) (GLvoid);
    GLvoid (APIENTRY *PopAttrib) (GLvoid);
    GLvoid (APIENTRY *PushAttrib) (GLbitfield mask);
    GLvoid (APIENTRY *Map1d) (GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
    GLvoid (APIENTRY *Map1f) (GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
    GLvoid (APIENTRY *Map2d) (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
    GLvoid (APIENTRY *Map2f) (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
    GLvoid (APIENTRY *MapGrid1d) (GLint un, GLdouble u1, GLdouble u2);
    GLvoid (APIENTRY *MapGrid1f) (GLint un, GLfloat u1, GLfloat u2);
    GLvoid (APIENTRY *MapGrid2d) (GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
    GLvoid (APIENTRY *MapGrid2f) (GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
    GLvoid (APIENTRY *EvalCoord1d) (GLdouble u);
    GLvoid (APIENTRY *EvalCoord1dv) (const GLdouble *u);
    GLvoid (APIENTRY *EvalCoord1f) (GLfloat u);
    GLvoid (APIENTRY *EvalCoord1fv) (const GLfloat *u);
    GLvoid (APIENTRY *EvalCoord2d) (GLdouble u, GLdouble v);
    GLvoid (APIENTRY *EvalCoord2dv) (const GLdouble *u);
    GLvoid (APIENTRY *EvalCoord2f) (GLfloat u, GLfloat v);
    GLvoid (APIENTRY *EvalCoord2fv) (const GLfloat *u);
    GLvoid (APIENTRY *EvalMesh1) (GLenum mode, GLint i1, GLint i2);
    GLvoid (APIENTRY *EvalPoint1) (GLint i);
    GLvoid (APIENTRY *EvalMesh2) (GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
    GLvoid (APIENTRY *EvalPoint2) (GLint i, GLint j);
    GLvoid (APIENTRY *AlphaFunc) (GLenum func, GLclampf ref);
    GLvoid (APIENTRY *BlendFunc) (GLenum sfactor, GLenum dfactor);
    GLvoid (APIENTRY *LogicOp) (GLenum opcode);
    GLvoid (APIENTRY *StencilFunc) (GLenum func, GLint ref, GLuint mask);
    GLvoid (APIENTRY *StencilOp) (GLenum fail, GLenum zfail, GLenum zpass);
    GLvoid (APIENTRY *DepthFunc) (GLenum func);
    GLvoid (APIENTRY *PixelZoom) (GLfloat xfactor, GLfloat yfactor);
    GLvoid (APIENTRY *PixelTransferf) (GLenum pname, GLfloat param);
    GLvoid (APIENTRY *PixelTransferi) (GLenum pname, GLint param);
    GLvoid (APIENTRY *PixelStoref) (GLenum pname, GLfloat param);
    GLvoid (APIENTRY *PixelStorei) (GLenum pname, GLint param);
    GLvoid (APIENTRY *PixelMapfv) (GLenum map, GLint mapsize, const GLfloat *values);
    GLvoid (APIENTRY *PixelMapuiv) (GLenum map, GLint mapsize, const GLuint *values);
    GLvoid (APIENTRY *PixelMapusv) (GLenum map, GLint mapsize, const GLushort *values);
    GLvoid (APIENTRY *ReadBuffer) (GLenum mode);
    GLvoid (APIENTRY *CopyPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
    GLvoid (APIENTRY *ReadPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
    GLvoid (APIENTRY *DrawPixels) (GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
    GLvoid (APIENTRY *GetBooleanv) (GLenum pname, GLboolean *params);
    GLvoid (APIENTRY *GetClipPlane) (GLenum plane, GLdouble *equation);
    GLvoid (APIENTRY *GetDoublev) (GLenum pname, GLdouble *params);
    GLenum (APIENTRY *GetError) (GLvoid);
    GLvoid (APIENTRY *GetFloatv) (GLenum pname, GLfloat *params);
    GLvoid (APIENTRY *GetIntegerv) (GLenum pname, GLint *params);
    GLvoid (APIENTRY *GetLightfv) (GLenum light, GLenum pname, GLfloat *params);
    GLvoid (APIENTRY *GetLightiv) (GLenum light, GLenum pname, GLint *params);
    GLvoid (APIENTRY *GetMapdv) (GLenum target, GLenum query, GLdouble *v);
    GLvoid (APIENTRY *GetMapfv) (GLenum target, GLenum query, GLfloat *v);
    GLvoid (APIENTRY *GetMapiv) (GLenum target, GLenum query, GLint *v);
    GLvoid (APIENTRY *GetMaterialfv) (GLenum face, GLenum pname, GLfloat *params);
    GLvoid (APIENTRY *GetMaterialiv) (GLenum face, GLenum pname, GLint *params);
    GLvoid (APIENTRY *GetPixelMapfv) (GLenum map, GLfloat *values);
    GLvoid (APIENTRY *GetPixelMapuiv) (GLenum map, GLuint *values);
    GLvoid (APIENTRY *GetPixelMapusv) (GLenum map, GLushort *values);
    GLvoid (APIENTRY *GetPolygonStipple) (GLubyte *mask);
    const GLubyte * (APIENTRY *GetString) (GLenum name);
    GLvoid (APIENTRY *GetTexEnvfv) (GLenum target, GLenum pname, GLfloat *params);
    GLvoid (APIENTRY *GetTexEnviv) (GLenum target, GLenum pname, GLint *params);
    GLvoid (APIENTRY *GetTexGendv) (GLenum coord, GLenum pname, GLdouble *params);
    GLvoid (APIENTRY *GetTexGenfv) (GLenum coord, GLenum pname, GLfloat *params);
    GLvoid (APIENTRY *GetTexGeniv) (GLenum coord, GLenum pname, GLint *params);
    GLvoid (APIENTRY *GetTexImage) (GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
    GLvoid (APIENTRY *GetTexParameterfv) (GLenum target, GLenum pname, GLfloat *params);
    GLvoid (APIENTRY *GetTexParameteriv) (GLenum target, GLenum pname, GLint *params);
    GLvoid (APIENTRY *GetTexLevelParameterfv) (GLenum target, GLint level, GLenum pname, GLfloat *params);
    GLvoid (APIENTRY *GetTexLevelParameteriv) (GLenum target, GLint level, GLenum pname, GLint *params);
    GLboolean (APIENTRY *IsEnabled) (GLenum cap);
    GLboolean (APIENTRY *IsList) (GLuint list);
    GLvoid (APIENTRY *DepthRange) (GLclampd zNear, GLclampd zFar);
    GLvoid (APIENTRY *Frustum) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
    GLvoid (APIENTRY *LoadIdentity) (GLvoid);
    GLvoid (APIENTRY *LoadMatrixf) (const GLfloat *m);
    GLvoid (APIENTRY *LoadMatrixd) (const GLdouble *m);
    GLvoid (APIENTRY *MatrixMode) (GLenum mode);
    GLvoid (APIENTRY *MultMatrixf) (const GLfloat *m);
    GLvoid (APIENTRY *MultMatrixd) (const GLdouble *m);
    GLvoid (APIENTRY *Ortho) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
    GLvoid (APIENTRY *PopMatrix) (GLvoid);
    GLvoid (APIENTRY *PushMatrix) (GLvoid);
    GLvoid (APIENTRY *Rotated) (GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
    GLvoid (APIENTRY *Rotatef) (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
    GLvoid (APIENTRY *Scaled) (GLdouble x, GLdouble y, GLdouble z);
    GLvoid (APIENTRY *Scalef) (GLfloat x, GLfloat y, GLfloat z);
    GLvoid (APIENTRY *Translated) (GLdouble x, GLdouble y, GLdouble z);
    GLvoid (APIENTRY *Translatef) (GLfloat x, GLfloat y, GLfloat z);
    GLvoid (APIENTRY *Viewport) (GLint x, GLint y, GLsizei width, GLsizei height);
#if GL_VERSION_1_1
    GLvoid (APIENTRY *ArrayElement) (GLint i);
    GLvoid (APIENTRY *BindTexture) (GLenum target, GLuint texture);
    GLvoid (APIENTRY *ColorPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
    GLvoid (APIENTRY *DisableClientState) (GLenum array);
    GLvoid (APIENTRY *DrawArrays) (GLenum mode, GLint first, GLsizei count);
    GLvoid (APIENTRY *DrawElements) (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
    GLvoid (APIENTRY *EdgeFlagPointer) (GLsizei stride, const GLboolean *pointer);
    GLvoid (APIENTRY *EnableClientState) (GLenum array);
    GLvoid (APIENTRY *IndexPointer) (GLenum type, GLsizei stride, const GLvoid *pointer);
    GLvoid (APIENTRY *Indexub) (GLubyte c);
    GLvoid (APIENTRY *Indexubv) (const GLubyte *c);
    GLvoid (APIENTRY *InterleavedArrays) (GLenum format, GLsizei stride, const GLvoid *pointer);
    GLvoid (APIENTRY *NormalPointer) (GLenum type, GLsizei stride, const GLvoid *pointer);
    GLvoid (APIENTRY *PolygonOffset) (GLfloat factor, GLfloat units);
    GLvoid (APIENTRY *TexCoordPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
    GLvoid (APIENTRY *VertexPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
    GLboolean (APIENTRY *AreTexturesResident) (GLsizei n, const GLuint *textures, GLboolean *residences);
    GLvoid (APIENTRY *CopyTexImage1D) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
    GLvoid (APIENTRY *CopyTexImage2D) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
    GLvoid (APIENTRY *CopyTexSubImage1D) (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
    GLvoid (APIENTRY *CopyTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    GLvoid (APIENTRY *DeleteTextures) (GLsizei n, const GLuint *textures);
    GLvoid (APIENTRY *GenTextures) (GLsizei n, GLuint *textures);
    GLvoid (APIENTRY *GetPointerv) (GLenum pname, GLvoid* *params);
    GLboolean (APIENTRY *IsTexture) (GLuint texture);
    GLvoid (APIENTRY *PrioritizeTextures) (GLsizei n, const GLuint *textures, const GLclampf *priorities);
    GLvoid (APIENTRY *TexSubImage1D) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
    GLvoid (APIENTRY *TexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
    GLvoid (APIENTRY *PopClientAttrib) (GLvoid);
    GLvoid (APIENTRY *PushClientAttrib) (GLbitfield mask);
#endif
#if GL_VERSION_1_2
    GLvoid (APIENTRY *BlendColor) (GLclampf, GLclampf, GLclampf, GLclampf);
    GLvoid (APIENTRY *BlendEquation) (GLenum);
    GLvoid (APIENTRY *DrawRangeElements) (GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid *);
    GLvoid (APIENTRY *ColorTable) (GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *);
    GLvoid (APIENTRY *ColorTableParameterfv) (GLenum, GLenum, const GLfloat *);
    GLvoid (APIENTRY *ColorTableParameteriv) (GLenum, GLenum, const GLint *);
    GLvoid (APIENTRY *CopyColorTable) (GLenum, GLenum, GLint, GLint, GLsizei);
    GLvoid (APIENTRY *GetColorTable) (GLenum, GLenum, GLenum, GLvoid *);
    GLvoid (APIENTRY *GetColorTableParameterfv) (GLenum, GLenum, GLfloat *);
    GLvoid (APIENTRY *GetColorTableParameteriv) (GLenum, GLenum, GLint *);
    GLvoid (APIENTRY *ColorSubTable) (GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
    GLvoid (APIENTRY *CopyColorSubTable) (GLenum, GLsizei, GLint, GLint, GLsizei);
    GLvoid (APIENTRY *ConvolutionFilter1D) (GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *);
    GLvoid (APIENTRY *ConvolutionFilter2D) (GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
    GLvoid (APIENTRY *ConvolutionParameterf) (GLenum, GLenum, GLfloat);
    GLvoid (APIENTRY *ConvolutionParameterfv) (GLenum, GLenum, const GLfloat *);
    GLvoid (APIENTRY *ConvolutionParameteri) (GLenum, GLenum, GLint);
    GLvoid (APIENTRY *ConvolutionParameteriv) (GLenum, GLenum, const GLint *);
    GLvoid (APIENTRY *CopyConvolutionFilter1D) (GLenum, GLenum, GLint, GLint, GLsizei);
    GLvoid (APIENTRY *CopyConvolutionFilter2D) (GLenum, GLenum, GLint, GLint, GLsizei, GLsizei);
    GLvoid (APIENTRY *GetConvolutionFilter) (GLenum, GLenum, GLenum, GLvoid *);
    GLvoid (APIENTRY *GetConvolutionParameterfv) (GLenum, GLenum, GLfloat *);
    GLvoid (APIENTRY *GetConvolutionParameteriv) (GLenum, GLenum, GLint *);
    GLvoid (APIENTRY *GetSeparableFilter) (GLenum, GLenum, GLenum, GLvoid *, GLvoid *, GLvoid *);
    GLvoid (APIENTRY *SeparableFilter2D) (GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *, const GLvoid *);
    GLvoid (APIENTRY *GetHistogram) (GLenum, GLboolean, GLenum, GLenum, GLvoid *);
    GLvoid (APIENTRY *GetHistogramParameterfv) (GLenum, GLenum, GLfloat *);
    GLvoid (APIENTRY *GetHistogramParameteriv) (GLenum, GLenum, GLint *);
    GLvoid (APIENTRY *GetMinmax) (GLenum, GLboolean, GLenum, GLenum, GLvoid *);
    GLvoid (APIENTRY *GetMinmaxParameterfv) (GLenum, GLenum, GLfloat *);
    GLvoid (APIENTRY *GetMinmaxParameteriv) (GLenum, GLenum, GLint *);
    GLvoid (APIENTRY *Histogram) (GLenum, GLsizei, GLenum, GLboolean);
    GLvoid (APIENTRY *Minmax) (GLenum, GLenum, GLboolean);
    GLvoid (APIENTRY *ResetHistogram) (GLenum);
    GLvoid (APIENTRY *ResetMinmax) (GLenum);
    GLvoid (APIENTRY *TexImage3D) (GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
    GLvoid (APIENTRY *TexSubImage3D) (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
    GLvoid (APIENTRY *CopyTexSubImage3D) (GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
#endif
#if GL_VERSION_1_3
    GLvoid (APIENTRY *ActiveTexture) (GLenum);
    GLvoid (APIENTRY *ClientActiveTexture) (GLenum);
    GLvoid (APIENTRY *MultiTexCoord1d) (GLenum, GLdouble);
    GLvoid (APIENTRY *MultiTexCoord1dv) (GLenum, const GLdouble *);
    GLvoid (APIENTRY *MultiTexCoord1f) (GLenum, GLfloat);
    GLvoid (APIENTRY *MultiTexCoord1fv) (GLenum, const GLfloat *);
    GLvoid (APIENTRY *MultiTexCoord1i) (GLenum, GLint);
    GLvoid (APIENTRY *MultiTexCoord1iv) (GLenum, const GLint *);
    GLvoid (APIENTRY *MultiTexCoord1s) (GLenum, GLshort);
    GLvoid (APIENTRY *MultiTexCoord1sv) (GLenum, const GLshort *);
    GLvoid (APIENTRY *MultiTexCoord2d) (GLenum, GLdouble, GLdouble);
    GLvoid (APIENTRY *MultiTexCoord2dv) (GLenum, const GLdouble *);
    GLvoid (APIENTRY *MultiTexCoord2f) (GLenum, GLfloat, GLfloat);
    GLvoid (APIENTRY *MultiTexCoord2fv) (GLenum, const GLfloat *);
    GLvoid (APIENTRY *MultiTexCoord2i) (GLenum, GLint, GLint);
    GLvoid (APIENTRY *MultiTexCoord2iv) (GLenum, const GLint *);
    GLvoid (APIENTRY *MultiTexCoord2s) (GLenum, GLshort, GLshort);
    GLvoid (APIENTRY *MultiTexCoord2sv) (GLenum, const GLshort *);
    GLvoid (APIENTRY *MultiTexCoord3d) (GLenum, GLdouble, GLdouble, GLdouble);
    GLvoid (APIENTRY *MultiTexCoord3dv) (GLenum, const GLdouble *);
    GLvoid (APIENTRY *MultiTexCoord3f) (GLenum, GLfloat, GLfloat, GLfloat);
    GLvoid (APIENTRY *MultiTexCoord3fv) (GLenum, const GLfloat *);
    GLvoid (APIENTRY *MultiTexCoord3i) (GLenum, GLint, GLint, GLint);
    GLvoid (APIENTRY *MultiTexCoord3iv) (GLenum, const GLint *);
    GLvoid (APIENTRY *MultiTexCoord3s) (GLenum, GLshort, GLshort, GLshort);
    GLvoid (APIENTRY *MultiTexCoord3sv) (GLenum, const GLshort *);
    GLvoid (APIENTRY *MultiTexCoord4d) (GLenum, GLdouble, GLdouble, GLdouble, GLdouble);
    GLvoid (APIENTRY *MultiTexCoord4dv) (GLenum, const GLdouble *);
    GLvoid (APIENTRY *MultiTexCoord4f) (GLenum, GLfloat, GLfloat, GLfloat, GLfloat);
    GLvoid (APIENTRY *MultiTexCoord4fv) (GLenum, const GLfloat *);
    GLvoid (APIENTRY *MultiTexCoord4i) (GLenum, GLint, GLint, GLint, GLint);
    GLvoid (APIENTRY *MultiTexCoord4iv) (GLenum, const GLint *);
    GLvoid (APIENTRY *MultiTexCoord4s) (GLenum, GLshort, GLshort, GLshort, GLshort);
    GLvoid (APIENTRY *MultiTexCoord4sv) (GLenum, const GLshort *);
    GLvoid (APIENTRY *LoadTransposeMatrixf) (const GLfloat *);
    GLvoid (APIENTRY *LoadTransposeMatrixd) (const GLdouble *);
    GLvoid (APIENTRY *MultTransposeMatrixf) (const GLfloat *);
    GLvoid (APIENTRY *MultTransposeMatrixd) (const GLdouble *);
    GLvoid (APIENTRY *SampleCoverage) (GLclampf, GLboolean);
    GLvoid (APIENTRY *CompressedTexImage3D) (GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
    GLvoid (APIENTRY *CompressedTexImage2D) (GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
    GLvoid (APIENTRY *CompressedTexImage1D) (GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const GLvoid *);
    GLvoid (APIENTRY *CompressedTexSubImage3D) (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *);
    GLvoid (APIENTRY *CompressedTexSubImage2D) (GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *);
    GLvoid (APIENTRY *CompressedTexSubImage1D) (GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const GLvoid *);
    GLvoid (APIENTRY *GetCompressedTexImage) (GLenum, GLint, GLvoid *);
#endif
#if GL_VERSION_1_4
    GLvoid (APIENTRY *BlendFuncSeparate) (GLenum, GLenum, GLenum, GLenum);
    GLvoid (APIENTRY *FogCoordf) (GLfloat);
    GLvoid (APIENTRY *FogCoordfv) (const GLfloat *);
    GLvoid (APIENTRY *FogCoordd) (GLdouble);
    GLvoid (APIENTRY *FogCoorddv) (const GLdouble *);
    GLvoid (APIENTRY *FogCoordPointer) (GLenum, GLsizei, const GLvoid *);
    GLvoid (APIENTRY *MultiDrawArrays) (GLenum, GLint *, GLsizei *, GLsizei);
    GLvoid (APIENTRY *MultiDrawElements) (GLenum, const GLsizei *, GLenum, const GLvoid* *, GLsizei);
    GLvoid (APIENTRY *PointParameterf) (GLenum, GLfloat);
    GLvoid (APIENTRY *PointParameterfv) (GLenum, const GLfloat *);
    GLvoid (APIENTRY *PointParameteri) (GLenum, GLint);
    GLvoid (APIENTRY *PointParameteriv) (GLenum, const GLint *);
    GLvoid (APIENTRY *SecondaryColor3b) (GLbyte, GLbyte, GLbyte);
    GLvoid (APIENTRY *SecondaryColor3bv) (const GLbyte *);
    GLvoid (APIENTRY *SecondaryColor3d) (GLdouble, GLdouble, GLdouble);
    GLvoid (APIENTRY *SecondaryColor3dv) (const GLdouble *);
    GLvoid (APIENTRY *SecondaryColor3f) (GLfloat, GLfloat, GLfloat);
    GLvoid (APIENTRY *SecondaryColor3fv) (const GLfloat *);
    GLvoid (APIENTRY *SecondaryColor3i) (GLint, GLint, GLint);
    GLvoid (APIENTRY *SecondaryColor3iv) (const GLint *);
    GLvoid (APIENTRY *SecondaryColor3s) (GLshort, GLshort, GLshort);
    GLvoid (APIENTRY *SecondaryColor3sv) (const GLshort *);
    GLvoid (APIENTRY *SecondaryColor3ub) (GLubyte, GLubyte, GLubyte);
    GLvoid (APIENTRY *SecondaryColor3ubv) (const GLubyte *);
    GLvoid (APIENTRY *SecondaryColor3ui) (GLuint, GLuint, GLuint);
    GLvoid (APIENTRY *SecondaryColor3uiv) (const GLuint *);
    GLvoid (APIENTRY *SecondaryColor3us) (GLushort, GLushort, GLushort);
    GLvoid (APIENTRY *SecondaryColor3usv) (const GLushort *);
    GLvoid (APIENTRY *SecondaryColorPointer) (GLint, GLenum, GLsizei, const GLvoid *);
    GLvoid (APIENTRY *WindowPos2d) (GLdouble, GLdouble);
    GLvoid (APIENTRY *WindowPos2dv) (const GLdouble *);
    GLvoid (APIENTRY *WindowPos2f) (GLfloat, GLfloat);
    GLvoid (APIENTRY *WindowPos2fv) (const GLfloat *);
    GLvoid (APIENTRY *WindowPos2i) (GLint, GLint);
    GLvoid (APIENTRY *WindowPos2iv) (const GLint *);
    GLvoid (APIENTRY *WindowPos2s) (GLshort, GLshort);
    GLvoid (APIENTRY *WindowPos2sv) (const GLshort *);
    GLvoid (APIENTRY *WindowPos3d) (GLdouble, GLdouble, GLdouble);
    GLvoid (APIENTRY *WindowPos3dv) (const GLdouble *);
    GLvoid (APIENTRY *WindowPos3f) (GLfloat, GLfloat, GLfloat);
    GLvoid (APIENTRY *WindowPos3fv) (const GLfloat *);
    GLvoid (APIENTRY *WindowPos3i) (GLint, GLint, GLint);
    GLvoid (APIENTRY *WindowPos3iv) (const GLint *);
    GLvoid (APIENTRY *WindowPos3s) (GLshort, GLshort, GLshort);
    GLvoid (APIENTRY *WindowPos3sv) (const GLshort *);
#endif
#if GL_VERSION_1_5
    GLvoid (APIENTRY *GenQueries)(GLsizei, GLuint *);
    GLvoid (APIENTRY *DeleteQueries)(GLsizei, const GLuint *);
    GLboolean (APIENTRY *IsQuery)(GLuint);
    GLvoid (APIENTRY *BeginQuery)(GLenum, GLuint);
    GLvoid (APIENTRY *EndQuery)(GLenum);
    GLvoid (APIENTRY *GetQueryiv)(GLenum, GLenum, GLint *);
    GLvoid (APIENTRY *GetQueryObjectiv) (GLuint, GLenum, GLint *);
    GLvoid (APIENTRY *GetQueryObjectuiv) (GLuint, GLenum, GLuint *);
    GLvoid (APIENTRY *BindBuffer) (GLenum, GLuint);
    GLvoid (APIENTRY *DeleteBuffers) (GLsizei, const GLuint *);
    GLvoid (APIENTRY *GenBuffers) (GLsizei, GLuint *);
    GLboolean (APIENTRY *IsBuffer) (GLuint);
    GLvoid (APIENTRY *BufferData) (GLenum, GLsizeiptr, const GLvoid *, GLenum);
    GLvoid (APIENTRY *BufferSubData) (GLenum, GLintptr, GLsizeiptr, const GLvoid *);
    GLvoid (APIENTRY *GetBufferSubData) (GLenum, GLintptr, GLsizeiptr, GLvoid *);
    GLvoid* (APIENTRY *MapBuffer) (GLenum, GLenum);
    GLboolean (APIENTRY *UnmapBuffer) (GLenum);
    GLvoid (APIENTRY *GetBufferParameteriv) (GLenum, GLenum, GLint *);
    GLvoid (APIENTRY *GetBufferPointerv)(GLenum, GLenum, GLvoid* *);
#endif
#if GL_VERSION_2_0
    GLvoid (APIENTRY *BlendEquationSeparate) (GLenum, GLenum);
    GLvoid (APIENTRY *DrawBuffers) (GLsizei, const GLenum *);
    GLvoid (APIENTRY *StencilOpSeparate) (GLenum, GLenum, GLenum, GLenum);
    GLvoid (APIENTRY *StencilFuncSeparate) (GLenum, GLenum, GLint, GLuint);
    GLvoid (APIENTRY *StencilMaskSeparate) (GLenum, GLuint);
    GLvoid (APIENTRY *AttachShader) (GLuint, GLuint);
    GLvoid (APIENTRY *BindAttribLocation) (GLuint, GLuint, const GLchar *);
    GLvoid (APIENTRY *CompileShader) (GLuint);
    GLuint (APIENTRY *CreateProgram) (GLvoid);
    GLuint (APIENTRY *CreateShader) (GLenum);
    GLvoid (APIENTRY *DeleteProgram) (GLuint);
    GLvoid (APIENTRY *DeleteShader) (GLuint);
    GLvoid (APIENTRY *DetachShader) (GLuint, GLuint);
    GLvoid (APIENTRY *DisableVertexAttribArray) (GLuint);
    GLvoid (APIENTRY *EnableVertexAttribArray) (GLuint);
    GLvoid (APIENTRY *GetActiveAttrib) (GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *);
    GLvoid (APIENTRY *GetActiveUniform) (GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *);
    GLvoid (APIENTRY *GetAttachedShaders) (GLuint, GLsizei, GLsizei *, GLuint *);
    GLint (APIENTRY *GetAttribLocation) (GLuint, const GLchar *);
    GLvoid (APIENTRY *GetProgramiv) (GLuint, GLenum, GLint *);
    GLvoid (APIENTRY *GetProgramInfoLog) (GLuint, GLsizei, GLsizei *, GLchar *);
    GLvoid (APIENTRY *GetShaderiv) (GLuint, GLenum, GLint *);
    GLvoid (APIENTRY *GetShaderInfoLog) (GLuint, GLsizei, GLsizei *, GLchar *);
    GLvoid (APIENTRY *GetShaderSource) (GLuint, GLsizei, GLsizei *, GLchar *);
    GLint (APIENTRY *GetUniformLocation) (GLuint, const GLchar *);
    GLvoid (APIENTRY *GetUniformfv) (GLuint, GLint, GLfloat *);
    GLvoid (APIENTRY *GetUniformiv) (GLuint, GLint, GLint *);
    GLvoid (APIENTRY *GetVertexAttribdv) (GLuint, GLenum, GLdouble *);
    GLvoid (APIENTRY *GetVertexAttribfv) (GLuint, GLenum, GLfloat *);
    GLvoid (APIENTRY *GetVertexAttribiv) (GLuint, GLenum, GLint *);
    GLvoid (APIENTRY *GetVertexAttribPointerv) (GLuint, GLenum, GLvoid* *);
    GLboolean (APIENTRY *IsProgram) (GLuint);
    GLboolean (APIENTRY *IsShader) (GLuint);
    GLvoid (APIENTRY *LinkProgram) (GLuint);
    GLvoid (APIENTRY *ShaderSource) (GLuint, GLsizei, const GLchar* *, const GLint *);
    GLvoid (APIENTRY *UseProgram) (GLuint);
    GLvoid (APIENTRY *Uniform1f) (GLint, GLfloat);
    GLvoid (APIENTRY *Uniform2f) (GLint, GLfloat, GLfloat);
    GLvoid (APIENTRY *Uniform3f) (GLint, GLfloat, GLfloat, GLfloat);
    GLvoid (APIENTRY *Uniform4f) (GLint, GLfloat, GLfloat, GLfloat, GLfloat);
    GLvoid (APIENTRY *Uniform1i) (GLint, GLint);
    GLvoid (APIENTRY *Uniform2i) (GLint, GLint, GLint);
    GLvoid (APIENTRY *Uniform3i) (GLint, GLint, GLint, GLint);
    GLvoid (APIENTRY *Uniform4i) (GLint, GLint, GLint, GLint, GLint);
    GLvoid (APIENTRY *Uniform1fv) (GLint, GLsizei, const GLfloat *);
    GLvoid (APIENTRY *Uniform2fv) (GLint, GLsizei, const GLfloat *);
    GLvoid (APIENTRY *Uniform3fv) (GLint, GLsizei, const GLfloat *);
    GLvoid (APIENTRY *Uniform4fv) (GLint, GLsizei, const GLfloat *);
    GLvoid (APIENTRY *Uniform1iv) (GLint, GLsizei, const GLint *);
    GLvoid (APIENTRY *Uniform2iv) (GLint, GLsizei, const GLint *);
    GLvoid (APIENTRY *Uniform3iv) (GLint, GLsizei, const GLint *);
    GLvoid (APIENTRY *Uniform4iv) (GLint, GLsizei, const GLint *);
    GLvoid (APIENTRY *UniformMatrix2fv) (GLint, GLsizei, GLboolean, const GLfloat *);
    GLvoid (APIENTRY *UniformMatrix3fv) (GLint, GLsizei, GLboolean, const GLfloat *);
    GLvoid (APIENTRY *UniformMatrix4fv) (GLint, GLsizei, GLboolean, const GLfloat *);
    GLvoid (APIENTRY *ValidateProgram) (GLuint);
    GLvoid (APIENTRY *VertexAttrib1d) (GLuint, GLdouble);
    GLvoid (APIENTRY *VertexAttrib1dv) (GLuint, const GLdouble *);
    GLvoid (APIENTRY *VertexAttrib1f) (GLuint, GLfloat);
    GLvoid (APIENTRY *VertexAttrib1fv) (GLuint, const GLfloat *);
    GLvoid (APIENTRY *VertexAttrib1s) (GLuint, GLshort);
    GLvoid (APIENTRY *VertexAttrib1sv) (GLuint, const GLshort *);
    GLvoid (APIENTRY *VertexAttrib2d) (GLuint, GLdouble, GLdouble);
    GLvoid (APIENTRY *VertexAttrib2dv) (GLuint, const GLdouble *);
    GLvoid (APIENTRY *VertexAttrib2f) (GLuint, GLfloat, GLfloat);
    GLvoid (APIENTRY *VertexAttrib2fv) (GLuint, const GLfloat *);
    GLvoid (APIENTRY *VertexAttrib2s) (GLuint, GLshort, GLshort);
    GLvoid (APIENTRY *VertexAttrib2sv) (GLuint, const GLshort *);
    GLvoid (APIENTRY *VertexAttrib3d) (GLuint, GLdouble, GLdouble, GLdouble);
    GLvoid (APIENTRY *VertexAttrib3dv) (GLuint, const GLdouble *);
    GLvoid (APIENTRY *VertexAttrib3f) (GLuint, GLfloat, GLfloat, GLfloat);
    GLvoid (APIENTRY *VertexAttrib3fv) (GLuint, const GLfloat *);
    GLvoid (APIENTRY *VertexAttrib3s) (GLuint, GLshort, GLshort, GLshort);
    GLvoid (APIENTRY *VertexAttrib3sv) (GLuint, const GLshort *);
    GLvoid (APIENTRY *VertexAttrib4Nbv) (GLuint, const GLbyte *);
    GLvoid (APIENTRY *VertexAttrib4Niv) (GLuint, const GLint *);
    GLvoid (APIENTRY *VertexAttrib4Nsv) (GLuint, const GLshort *);
    GLvoid (APIENTRY *VertexAttrib4Nub) (GLuint, GLubyte, GLubyte, GLubyte, GLubyte);
    GLvoid (APIENTRY *VertexAttrib4Nubv) (GLuint, const GLubyte *);
    GLvoid (APIENTRY *VertexAttrib4Nuiv) (GLuint, const GLuint *);
    GLvoid (APIENTRY *VertexAttrib4Nusv) (GLuint, const GLushort *);
    GLvoid (APIENTRY *VertexAttrib4bv) (GLuint, const GLbyte *);
    GLvoid (APIENTRY *VertexAttrib4d) (GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
    GLvoid (APIENTRY *VertexAttrib4dv) (GLuint, const GLdouble *);
    GLvoid (APIENTRY *VertexAttrib4f) (GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
    GLvoid (APIENTRY *VertexAttrib4fv) (GLuint, const GLfloat *);
    GLvoid (APIENTRY *VertexAttrib4iv) (GLuint, const GLint *);
    GLvoid (APIENTRY *VertexAttrib4s) (GLuint, GLshort, GLshort, GLshort, GLshort);
    GLvoid (APIENTRY *VertexAttrib4sv) (GLuint, const GLshort *);
    GLvoid (APIENTRY *VertexAttrib4ubv) (GLuint, const GLubyte *);
    GLvoid (APIENTRY *VertexAttrib4uiv) (GLuint, const GLuint *);
    GLvoid (APIENTRY *VertexAttrib4usv) (GLuint, const GLushort *);
    GLvoid (APIENTRY *VertexAttribPointer) (GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *);
#endif
#if GL_VERSION_2_1
    GLvoid (APIENTRY *UniformMatrix2x3fv) (GLint, GLsizei, GLboolean, const GLfloat *);
    GLvoid (APIENTRY *UniformMatrix2x4fv) (GLint, GLsizei, GLboolean, const GLfloat *);
    GLvoid (APIENTRY *UniformMatrix3x2fv) (GLint, GLsizei, GLboolean, const GLfloat *);
    GLvoid (APIENTRY *UniformMatrix3x4fv) (GLint, GLsizei, GLboolean, const GLfloat *);
    GLvoid (APIENTRY *UniformMatrix4x2fv) (GLint, GLsizei, GLboolean, const GLfloat *);
    GLvoid (APIENTRY *UniformMatrix4x3fv) (GLint, GLsizei, GLboolean, const GLfloat *);
#endif

#if GL_ARB_vertex_program
    GLvoid (APIENTRY *ProgramStringARB) (GLenum target, GLenum format, GLsizei len, const GLvoid *string);
    GLvoid (APIENTRY *BindProgramARB) (GLenum target, GLuint program);
    GLvoid (APIENTRY *DeleteProgramsARB) (GLsizei n, const GLuint *programs);
    GLvoid (APIENTRY *GenProgramsARB) (GLsizei n, GLuint *programs);
    GLvoid (APIENTRY *ProgramEnvParameter4dARB) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    GLvoid (APIENTRY *ProgramEnvParameter4dvARB) (GLenum target, GLuint index, const GLdouble *params);
    GLvoid (APIENTRY *ProgramEnvParameter4fARB) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    GLvoid (APIENTRY *ProgramEnvParameter4fvARB) (GLenum target, GLuint index, const GLfloat *params);
    GLvoid (APIENTRY *ProgramLocalParameter4dARB) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    GLvoid (APIENTRY *ProgramLocalParameter4dvARB) (GLenum target, GLuint index, const GLdouble *params);
    GLvoid (APIENTRY *ProgramLocalParameter4fARB) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    GLvoid (APIENTRY *ProgramLocalParameter4fvARB) (GLenum target, GLuint index, const GLfloat *params);
    GLvoid (APIENTRY *GetProgramEnvParameterdvARB) (GLenum target, GLuint index, GLdouble *params);
    GLvoid (APIENTRY *GetProgramEnvParameterfvARB) (GLenum target, GLuint index, GLfloat *params);
    GLvoid (APIENTRY *GetProgramLocalParameterdvARB) (GLenum target, GLuint index, GLdouble *params);
    GLvoid (APIENTRY *GetProgramLocalParameterfvARB) (GLenum target, GLuint index, GLfloat *params);
    GLvoid (APIENTRY *GetProgramivARB) (GLenum target, GLenum pname, GLint *params);
    GLvoid (APIENTRY *GetProgramStringARB) (GLenum target, GLenum pname, GLvoid *string);
    GLboolean (APIENTRY *IsProgramARB) (GLuint program);
#endif

#if GL_ARB_shader_objects
    GLvoid (APIENTRY *DeleteObjectARB) (GLhandleARB);
    GLhandleARB (APIENTRY *GetHandleARB) (GLenum);
    GLvoid (APIENTRY *GetInfoLogARB) (GLhandleARB, GLsizei, GLsizei*, GLcharARB*);
    GLvoid (APIENTRY *GetObjectParameterfvARB) (GLhandleARB, GLenum, GLfloat*);
    GLvoid (APIENTRY *GetObjectParameterivARB) (GLhandleARB, GLenum, GLint*);
#endif

#if GL_ATI_vertex_array_object
    GLuint (APIENTRY *NewObjectBufferATI ) (GLsizei, const GLvoid *, GLenum);
    GLboolean (APIENTRY *IsObjectBufferATI) (GLuint);
    GLvoid (APIENTRY *UpdateObjectBufferATI) (GLuint, GLuint, GLsizei, const GLvoid *, GLenum);
    GLvoid (APIENTRY *GetObjectBufferfvATI ) (GLuint, GLenum, GLfloat *);
    GLvoid (APIENTRY *GetObjectBufferivATI) (GLuint, GLenum, GLint *);
    GLvoid (APIENTRY *FreeObjectBufferATI ) (GLuint);
    GLvoid (APIENTRY *ArrayObjectATI ) (GLenum, GLint, GLenum, GLsizei, GLuint, GLuint);
    GLvoid (APIENTRY *GetArrayObjectfvATI ) (GLenum, GLenum, GLfloat *);
    GLvoid (APIENTRY *GetArrayObjectivATI ) (GLenum, GLenum, GLint *);
    GLvoid (APIENTRY *VariantArrayObjectATI ) (GLuint id, GLenum type, GLsizei stride, GLuint buffer, GLuint offset);
    GLvoid (APIENTRY *GetVariantArrayObjectfvATI ) (GLuint id, GLenum pname, GLfloat *params);
    GLvoid (APIENTRY *GetVariantArrayObjectivATI ) (GLuint id, GLenum pname, GLint *params);
#endif
#if GL_ATI_vertex_attrib_array_object
    GLvoid (APIENTRY *VertexAttribArrayObjectATI ) (GLuint, GLint, GLenum, GLboolean, GLsizei, GLuint, GLuint);
    GLvoid (APIENTRY *GetVertexAttribArrayObjectfvATI ) (GLuint, GLenum, GLfloat *);
    GLvoid (APIENTRY *GetVertexAttribArrayObjectivATI ) (GLuint, GLenum, GLint *);
#endif
#if GL_ATI_element_array
    GLvoid (APIENTRY *ElementPointerATI) (GLenum type, const GLvoid *pointer);
    GLvoid (APIENTRY *DrawElementArrayATI) (GLenum mode, GLsizei count);
    GLvoid (APIENTRY *DrawRangeElementArrayATI) (GLenum mode, GLuint start, GLuint end, GLsizei count);
#endif

#if GL_EXT_stencil_two_side
    GLvoid (APIENTRY *ActiveStencilFaceEXT)(GLenum face);
#endif

    GLvoid (APIENTRY *AddSwapHintRectWIN)(GLint x, GLint y, GLsizei width, GLsizei height);

#if GL_EXT_depth_bounds_test
    GLvoid (APIENTRY *DepthBoundsEXT)(GLclampd zMin, GLclampd zMax);
#endif

#if GL_EXT_framebuffer_object
    GLboolean (APIENTRY *IsRenderbufferEXT)(GLuint renderbuffer);
    GLvoid (APIENTRY *BindRenderbufferEXT)(GLenum target, GLuint renderbuffer);
    GLvoid (APIENTRY *DeleteRenderbuffersEXT)(GLsizei n, const GLuint *renderbuffers);
    GLvoid (APIENTRY *GenRenderbuffersEXT)(GLsizei n, GLuint *renderbuffers);
    GLvoid (APIENTRY *RenderbufferStorageEXT)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
    GLvoid (APIENTRY *GetRenderbufferParameterivEXT)(GLenum target, GLenum pname, GLint* params);
    GLboolean (APIENTRY *IsFramebufferEXT)(GLuint framebuffer);
    GLvoid (APIENTRY *BindFramebufferEXT)(GLenum target, GLuint framebuffer);
    GLvoid (APIENTRY *DeleteFramebuffersEXT)(GLsizei n, const GLuint *framebuffers);
    GLvoid (APIENTRY *GenFramebuffersEXT)(GLsizei n, GLuint *framebuffers);
    GLenum (APIENTRY *CheckFramebufferStatusEXT)(GLenum target);
    GLvoid (APIENTRY *FramebufferTexture1DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    GLvoid (APIENTRY *FramebufferTexture2DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    GLvoid (APIENTRY *FramebufferTexture3DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
    GLvoid (APIENTRY *FramebufferRenderbufferEXT)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
    GLvoid (APIENTRY *GetFramebufferAttachmentParameterivEXT)(GLenum target, GLenum attachment, GLenum pname, GLint *params);
    GLvoid (APIENTRY *GenerateMipmapEXT)(GLenum target);
#if GL_EXT_framebuffer_blit
    GLvoid (APIENTRY *BlitFramebufferEXT)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                                          GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
                                          GLbitfield mask, GLenum filter);
#if GL_EXT_framebuffer_multisample
    GLvoid (APIENTRY *RenderbufferStorageMultisampleEXT)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
#endif  /* GL_EXT_framebuffer_multisample  */
#endif  /* GL_EXT_framebuffer_blit         */
#endif  /* GL_EXT_framebuffer_object       */

#if GL_NV_occlusion_query
    GLvoid (APIENTRY *BeginQueryNV)(GLuint);
    GLvoid (APIENTRY *EndQueryNV)(GLvoid);
#endif

#if GL_EXT_bindable_uniform
    GLvoid (APIENTRY *UniformBufferEXT)(GLuint program, GLint location, GLuint buffer);
    GLint (APIENTRY *GetUniformBufferSizeEXT)(GLuint program, GLint location);
    GLintptr (APIENTRY *GetUniformOffsetEXT)(GLuint program, GLint location);
#endif

#if GL_EXT_texture_integer
    GLvoid (APIENTRY *ClearColorIiEXT)(GLint r, GLint g, GLint b, GLint a);
    GLvoid (APIENTRY *ClearColorIuiEXT)(GLuint r, GLuint g, GLuint b, GLuint a);
    GLvoid (APIENTRY *TexParameterIivEXT)(GLenum target, GLenum pname,GLint * params);
    GLvoid (APIENTRY *TexParameterIuivEXT)(GLenum target, GLenum pname, GLuint * params);
    GLvoid (APIENTRY *GetTexParameterIivEXT)(GLenum target, GLenum pname, GLint * params);
    GLvoid (APIENTRY *GetTexParameterIuivEXT)(GLenum target, GLenum pname, GLuint * params);
#endif

#if GL_EXT_gpu_shader4
    GLvoid (APIENTRY *VertexAttribI1iEXT)(GLuint index, GLint x);
    GLvoid (APIENTRY *VertexAttribI2iEXT)(GLuint index, GLint x, GLint y);
    GLvoid (APIENTRY *VertexAttribI3iEXT)(GLuint index, GLint x, GLint y, GLint z);
    GLvoid (APIENTRY *VertexAttribI4iEXT)(GLuint index, GLint x, GLint y, GLint z, GLint w);

    GLvoid (APIENTRY *VertexAttribI1uiEXT)(GLuint index, GLuint x);
    GLvoid (APIENTRY *VertexAttribI2uiEXT)(GLuint index, GLuint x, GLuint y);
    GLvoid (APIENTRY *VertexAttribI3uiEXT)(GLuint index, GLuint x, GLuint y, GLuint z);
    GLvoid (APIENTRY *VertexAttribI4uiEXT)(GLuint index, GLuint x, GLuint y, GLuint z,
                            GLuint w);

    GLvoid (APIENTRY *VertexAttribI1ivEXT)(GLuint index, const GLint *v);
    GLvoid (APIENTRY *VertexAttribI2ivEXT)(GLuint index, const GLint *v);
    GLvoid (APIENTRY *VertexAttribI3ivEXT)(GLuint index, const GLint *v);
    GLvoid (APIENTRY *VertexAttribI4ivEXT)(GLuint index, const GLint *v);

    GLvoid (APIENTRY *VertexAttribI1uivEXT)(GLuint index, const GLuint *v);
    GLvoid (APIENTRY *VertexAttribI2uivEXT)(GLuint index, const GLuint *v);
    GLvoid (APIENTRY *VertexAttribI3uivEXT)(GLuint index, const GLuint *v);
    GLvoid (APIENTRY *VertexAttribI4uivEXT)(GLuint index, const GLuint *v);

    GLvoid (APIENTRY *VertexAttribI4bvEXT)(GLuint index, const GLbyte *v);
    GLvoid (APIENTRY *VertexAttribI4svEXT)(GLuint index, const GLshort *v);
    GLvoid (APIENTRY *VertexAttribI4ubvEXT)(GLuint index, const GLubyte *v);
    GLvoid (APIENTRY *VertexAttribI4usvEXT)(GLuint index, const GLushort *v);

    GLvoid (APIENTRY *VertexAttribIPointerEXT)(GLuint index, GLint size, GLenum type,
                                GLsizei stride, const GLvoid *pointer);

    GLvoid (APIENTRY *GetVertexAttribIivEXT)(GLuint index, GLenum pname, GLint *params);
    GLvoid (APIENTRY *GetVertexAttribIuivEXT)(GLuint index, GLenum pname, GLuint *params);

    GLvoid (APIENTRY *Uniform1uiEXT)(GLint location, GLuint v0);
    GLvoid (APIENTRY *Uniform2uiEXT)(GLint location, GLuint v0, GLuint v1);
    GLvoid (APIENTRY *Uniform3uiEXT)(GLint location, GLuint v0, GLuint v1, GLuint v2);
    GLvoid (APIENTRY *Uniform4uiEXT)(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);

    GLvoid (APIENTRY *Uniform1uivEXT)(GLint location, GLsizei count, const GLuint *value);
    GLvoid (APIENTRY *Uniform2uivEXT)(GLint location, GLsizei count, const GLuint *value);
    GLvoid (APIENTRY *Uniform3uivEXT)(GLint location, GLsizei count, const GLuint *value);
    GLvoid (APIENTRY *Uniform4uivEXT)(GLint location, GLsizei count, const GLuint *value);

    GLvoid (APIENTRY *GetUniformuivEXT)(GLuint program, GLint location, GLuint *params);

    GLvoid (APIENTRY *BindFragDataLocationEXT)(GLuint program, GLuint colorNumber,
                                const GLbyte *name);
    GLint (APIENTRY *GetFragDataLocationEXT)(GLuint program, const GLbyte *name);
#endif

#if GL_EXT_geometry_shader4
    GLvoid (APIENTRY *ProgramParameteriEXT)(GLuint program, GLenum pname, GLint value);
    GLvoid (APIENTRY *FramebufferTextureEXT)(GLenum target, GLenum attachment, GLuint texture, GLint level);
    GLvoid (APIENTRY *FramebufferTextureLayerEXT)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
    GLvoid (APIENTRY *FramebufferTextureFaceEXT)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face);
#endif

#if GL_EXT_draw_buffers2
    GLvoid (APIENTRY *ColorMaskIndexedEXT)(GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
    GLvoid (APIENTRY *GetBooleanIndexedvEXT)(GLenum value, GLuint index, GLboolean* data);
    GLvoid (APIENTRY *GetIntegerIndexedvEXT)(GLenum value, GLuint index, GLint *data);
    GLvoid (APIENTRY *EnableIndexedEXT)(GLenum target, GLuint index);
    GLvoid (APIENTRY *DisableIndexedEXT)(GLenum target, GLuint index);
    GLboolean (APIENTRY *IsEnabledIndexedEXT)(GLenum target, GLuint index);
#endif

#if GL_EXT_texture_buffer_object
    GLvoid (APIENTRY *TexBufferEXT)(GLenum target, GLenum internalformat, GLuint buffer);
#endif

#if GL_EXT_gpu_program_parameters
    GLvoid (APIENTRY *ProgramEnvParameters4fvEXT)(GLenum target, GLuint index, GLsizei count, const GLfloat *params);
    GLvoid (APIENTRY *ProgramLocalParameters4fvEXT)(GLenum target, GLuint index, GLsizei count, const GLfloat *params);
#endif

#if GL_EXT_draw_instanced
    GLvoid (APIENTRY *DrawArraysInstancedEXT)(GLenum mode,
                                        GLint first, GLsizei count, GLsizei primCount);
    GLvoid (APIENTRY *DrawElementsInstancedEXT)(GLenum mode,
                        GLsizei count, GLenum type, const GLvoid *indices, GLsizei primCount);
#endif

#if GL_ARB_color_buffer_float
    GLvoid (APIENTRY *ClampColorARB)(GLenum target, GLenum clamp);
#endif

#if GL_EXT_timer_query
    GLvoid (APIENTRY *GetQueryObjecti64vEXT)(GLuint id, GLenum pname, GLint64EXT * params);
    GLvoid (APIENTRY *GetQueryObjectui64vEXT)(GLuint id, GLenum pname, GLuint64EXT * params);
#endif

#ifdef GL_ATI_separate_stencil
    GLvoid (APIENTRY *StencilFuncSeparateATI)(GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask);
#endif

} __GLdispatchTable;


/* Since opengl32.dll only supports OpenGL version 1.1, the "nEntries" in __GLdispatchState
** which is returned to opengl32.dll by DrvSetContext() must be OPENGL_VERSION_110_ENTRIES.
 */
#define OPENGL_VERSION_110_ENTRIES      336

struct __GLdispatchStateRec {
    GLint nEntries;
    __GLdispatchTable dispatch;
};


extern __GLdispatchState __glImmediateFuncTable;
extern __GLdispatchState __glListCompileFuncTable;

/* Macro to set GL API dispatch table */
#if defined(_LINUX_)
#define __GL_SET_API_DISPATCH(offset) \
    gc->currentDispatchOffset = (offset); \
    _glapi_set_dispatch((GLvoid *)((GLubyte*)gc + (offset)));
#else
#define __GL_SET_API_DISPATCH(offset) \
    gc->currentDispatchOffset = (offset);
#endif

#endif /* __g_disp_h_ */
