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


#ifndef __g_lefncs_h_
#define __g_lefncs_h_

extern const GLubyte *__glle_Skip(__GLcontext * , const GLubyte *PC);
extern const GLubyte *__glle_InvalidValue(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_InvalidEnum(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_InvalidOperation(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_TableTooLarge(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Primitive(__GLcontext * , const GLubyte *);

extern const GLubyte *__glle_CallList(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_CallLists(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_DeleteLists(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GenLists(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ListBase(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Begin(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Bitmap(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Color3fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Color4fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Color4ubv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_EdgeFlag(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_End(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Indexf(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Normal3fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_RasterPos2fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_RasterPos3fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_RasterPos4fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Rectf(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_TexCoord2fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_TexCoord3fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_TexCoord4fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Vertex2fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Vertex3fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Vertex4fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ClipPlane(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ColorMaterial(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_CullFace(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Fogfv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Fogiv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_FrontFace(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Hint(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Lightfv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Lightiv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_LightModelfv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_LightModeliv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_LineStipple(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_LineWidth(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Materialfv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Materialiv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_PointSize(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_PolygonMode(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_PolygonStipple(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Scissor(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ShadeModel(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_TexParameterfv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_TexParameteriv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_TexImage1D(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_TexImage2D(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_TexEnvfv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_TexEnviv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_TexGendv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_TexGenfv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_TexGeniv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_FeedbackBuffer(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_SelectBuffer(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_RenderMode(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_InitNames(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_LoadName(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_PassThrough(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_PopName(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_PushName(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_DrawBuffer(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Clear(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ClearAccum(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ClearIndex(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ClearColor(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ClearStencil(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ClearDepth(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_StencilMask(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ColorMask(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_DepthMask(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_IndexMask(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Accum(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Disable(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Enable(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Finish(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Flush(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_PopAttrib(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_PushAttrib(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Map1d(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Map1f(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Map2d(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Map2f(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_MapGrid1d(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_MapGrid1f(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_MapGrid2d(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_MapGrid2f(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_EvalCoord1d(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_EvalCoord1dv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_EvalCoord1f(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_EvalCoord1fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_EvalCoord2d(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_EvalCoord2dv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_EvalCoord2f(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_EvalCoord2fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_EvalMesh1(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_EvalPoint1(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_EvalMesh2(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_EvalPoint2(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_AlphaFunc(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_BlendFunc(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_LogicOp(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_StencilFunc(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_StencilOp(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_DepthFunc(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_PixelZoom(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_PixelTransferf(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_PixelTransferi(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_PixelStoref(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_PixelStorei(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_PixelMapfv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_PixelMapuiv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_PixelMapusv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ReadBuffer(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_CopyPixels(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ReadPixels(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_DrawPixels(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetBooleanv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetClipPlane(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetDoublev(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetError(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetFloatv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetIntegerv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetLightfv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetLightiv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetMapdv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetMapfv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetMapiv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetMaterialfv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetMaterialiv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetPixelMapfv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetPixelMapuiv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetPixelMapusv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetPolygonStipple(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetString(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetTexEnvfv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetTexEnviv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetTexGendv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetTexGenfv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetTexGeniv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetTexImage(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetTexParameterfv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetTexParameteriv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetTexLevelParameterfv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetTexLevelParameteriv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_IsEnabled(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_IsList(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_DepthRange(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Frustum(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_LoadIdentity(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_LoadMatrixf(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_LoadMatrixd(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_MatrixMode(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_MultMatrixf(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_MultMatrixd(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Ortho(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_PopMatrix(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_PushMatrix(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Rotated(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Rotatef(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Scaled(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Scalef(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Translated(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Translatef(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Viewport(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ColorSubTable(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ColorTable(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_CopyColorTable(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetColorTable(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetColorTableParameterfv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetColorTableParameteriv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ColorPointer(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_DisableClientState(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_EdgeFlagPointer(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_EnableClientState(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetPointerv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_IndexPointer(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_InterleavedArrays(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_NormalPointer(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_TexCoordPointer(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_VertexPointer(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_PolygonOffset(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_CopyTexImage1D(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_CopyTexImage2D(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_CopyTexSubImage1D(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_CopyTexSubImage2D(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_TexSubImage1D(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_TexSubImage2D(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_AreTexturesResident(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_BindTexture(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_DeleteTextures(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GenTextures(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_IsTexture(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_PrioritizeTextures(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_PopClientAttrib(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_PushClientAttrib(__GLcontext * , const GLubyte *);

#if GL_VERSION_1_2
extern const GLubyte *__glle_BlendColor(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_BlendEquation(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ColorTable(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ColorTableParameterfv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ColorTableParameteriv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_CopyColorTable(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetColorTable(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetColorTableParameterfv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetColorTableParameteriv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ColorSubTable(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_CopyColorSubTable(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ConvolutionFilter1D(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ConvolutionFilter2D(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ConvolutionParameterf(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ConvolutionParameterfv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ConvolutionParameteri(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ConvolutionParameteriv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_CopyConvolutionFilter1D(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_CopyConvolutionFilter2D(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetConvolutionFilter(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetConvolutionParameterfv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetConvolutionParameteriv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetSeparableFilter(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_SeparableFilter2D(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetHistogram(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetHistogramParameterfv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetHistogramParameteriv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetMinmax(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetMinmaxParameterfv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetMinmaxParameteriv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Histogram(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Minmax(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ResetHistogram(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ResetMinmax(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_TexImage3D(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_TexSubImage3D(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_CopyTexSubImage3D(__GLcontext * , const GLubyte *);
#endif
#if GL_VERSION_1_3
extern const GLubyte *__glle_ActiveTexture(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ClientActiveTexture(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_MultiTexCoord2fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_MultiTexCoord3fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_MultiTexCoord4fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_LoadTransposeMatrixf(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_LoadTransposeMatrixd(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_MultTransposeMatrixf(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_MultTransposeMatrixd(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_SampleCoverage(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_CompressedTexImage3D(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_CompressedTexImage2D(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_CompressedTexImage1D(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_CompressedTexSubImage3D(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_CompressedTexSubImage2D(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_CompressedTexSubImage1D(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetCompressedTexImage(__GLcontext * , const GLubyte *);
#endif
#if GL_VERSION_1_4
extern const GLubyte *__glle_BlendFuncSeparate(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_FogCoordf(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_FogCoordfv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_FogCoordd(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_FogCoorddv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_FogCoordPointer(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_MultiDrawArrays(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_MultiDrawElements(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_PointParameterfv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_PointParameteriv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_SecondaryColor3fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_SecondaryColorPointer(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_WindowPos2fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_WindowPos3fv(__GLcontext * , const GLubyte *);
#endif
#if GL_VERSION_1_5
extern const GLubyte *__glle_GenQueries(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_DeleteQueries(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_IsQuery(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_BeginQuery(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_EndQuery(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetQueryiv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetQueryObjectiv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetQueryObjectuiv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_BindBuffer(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_DeleteBuffers(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GenBuffers(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_IsBuffer(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_BufferData(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_BufferSubData(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetBufferSubData(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_MapBuffer(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_UnmapBuffer(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetBufferParameteriv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetBufferPointerv(__GLcontext * , const GLubyte *);
#endif
#if GL_VERSION_2_0
extern const GLubyte *__glle_BlendEquationSeparate(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_DrawBuffers(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_StencilOpSeparate(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_StencilFuncSeparate(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_StencilMaskSeparate(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_AttachShader(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_BindAttribLocation(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_CompileShader(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_CreateProgram(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_CreateShader(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_DeleteProgram(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_DeleteShader(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_DetachShader(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_DisableVertexAttribArray(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_EnableVertexAttribArray(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetActiveAttrib(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetActiveUniform(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetAttachedShaders(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetAttribLocation(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetProgramiv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetProgramInfoLog(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetShaderiv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetShaderInfoLog(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetShaderSource(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetUniformLocation(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetUniformfv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetUniformiv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetVertexAttribdv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetVertexAttribfv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetVertexAttribiv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetVertexAttribPointerv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_IsProgram(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_IsShader(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_LinkProgram(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ShaderSource(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_UseProgram(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Uniform1f(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Uniform2f(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Uniform3f(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Uniform4f(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Uniform1i(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Uniform2i(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Uniform3i(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Uniform4i(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Uniform1fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Uniform2fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Uniform3fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Uniform4fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Uniform1iv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Uniform2iv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Uniform3iv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Uniform4iv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_UniformMatrix2fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_UniformMatrix3fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_UniformMatrix4fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ValidateProgram(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_VertexAttrib4fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_VertexAttribPointer(__GLcontext * , const GLubyte *);
#endif

#if GL_VERSION_2_1
extern const GLubyte *__glle_UniformMatrix2x3fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_UniformMatrix2x4fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_UniformMatrix3x2fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_UniformMatrix3x4fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_UniformMatrix4x2fv(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_UniformMatrix4x3fv(__GLcontext * , const GLubyte *);
#endif


#if GL_ARB_vertex_program
extern const GLubyte *__glle_ProgramStringARB(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_BindProgramARB(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_DeleteProgramsARB(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GenProgramsARB(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ProgramEnvParameter4dARB(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ProgramEnvParameter4dvARB(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ProgramEnvParameter4fARB(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ProgramEnvParameter4fvARB(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ProgramLocalParameter4dARB(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ProgramLocalParameter4dvARB(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ProgramLocalParameter4fARB(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ProgramLocalParameter4fvARB(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetProgramEnvParameterdvARB(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetProgramEnvParameterfvARB(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetProgramLocalParameterdvARB(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetProgramLocalParameterfvARB(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetProgramivARB(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetProgramStringARB(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_IsProgramARB(__GLcontext * , const GLubyte *);
#endif

#if GL_ARB_shader_objects
extern const GLubyte *__glle_DeleteObjectARB(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetHandleARB(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetInfoLogARB(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetObjectParameterfvARB(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_GetObjectParameterivARB(__GLcontext * , const GLubyte *);
#endif

#if GL_ATI_element_array
extern const GLubyte *__glle_DrawElementArrayATI(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_DrawRangeElementArrayATI(__GLcontext * , const GLubyte *);
#endif

#if GL_EXT_stencil_two_side
extern const GLubyte *__glle_ActiveStencilFaceEXT(__GLcontext * , const GLubyte *);
#endif

#if GL_EXT_depth_bounds_test
extern const GLubyte * __glle_DepthBoundsEXT(__GLcontext * , const GLubyte *PC);
#endif

#if GL_NV_occlusion_query
extern const GLubyte *__glle_BeginQueryNV(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_EndQueryNV(__GLcontext * , const GLubyte *);
#endif

#if GL_EXT_texture_integer
extern const GLubyte *__glle_ClearColorIiEXT(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ClearColorIuiEXT(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_TexParameterIivEXT(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_TexParameterIuivEXT(__GLcontext * , const GLubyte *);
#endif

#if GL_EXT_gpu_shader4
extern const GLubyte *__glle_Uniform1uiEXT(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Uniform2uiEXT(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Uniform3uiEXT(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Uniform4uiEXT(__GLcontext * , const GLubyte *);

extern const GLubyte *__glle_Uniform1uivEXT(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Uniform2uivEXT(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Uniform3uivEXT(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_Uniform4uivEXT(__GLcontext * , const GLubyte *);

#endif

#if GL_EXT_geometry_shader4
extern const GLubyte *__glle_FramebufferTextureEXT(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_FramebufferTextureLayerEXT(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_FramebufferTextureFaceEXT(__GLcontext * , const GLubyte *);
#endif

#if GL_EXT_draw_buffers2
extern const GLubyte *__glle_ColorMaskIndexedEXT(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_EnableIndexedEXT(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_DisableIndexedEXT(__GLcontext * , const GLubyte *);
#endif

#if GL_EXT_gpu_program_parameters
extern const GLubyte *__glle_ProgramEnvParameters4fvEXT(__GLcontext * , const GLubyte *);
extern const GLubyte *__glle_ProgramLocalParameters4fvEXT(__GLcontext * , const GLubyte *);
#endif

#if GL_ARB_color_buffer_float
extern const GLubyte *__glle_ClampColorARB(__GLcontext * , const GLubyte*);
#endif

#if GL_ATI_separate_stencil
extern const GLubyte *__glle_StencilFuncSeparateATI(__GLcontext * , const GLubyte*);
#endif

#endif /* __g_lefncs_h_ */
