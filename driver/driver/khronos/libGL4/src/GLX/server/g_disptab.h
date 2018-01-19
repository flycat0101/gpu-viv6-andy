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


/* $XFree86: xc/programs/Xserver/GL/glx/g_disptab.h,v 1.4 2003/09/28 20:15:42 alanh Exp $ */
/* DO NOT EDIT - THIS FILE IS AUTOMATICALLY GENERATED */
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#ifndef _GLX_g_disptab_h_
#define _GLX_g_disptab_h_
/*
** License Applicability. Except to the extent portions of this file are
** made subject to an alternative license as permitted in the SGI Free
** Software License B, Version 1.1 (the "License"), the contents of this
** file are subject only to the provisions of the License. You may not use
** this file except in compliance with the License. You may obtain a copy
** of the License at Silicon Graphics, Inc., attn: Legal Services, 1600
** Amphitheatre Parkway, Mountain View, CA 94043-1351, or at:
**
** http://oss.sgi.com/projects/FreeB
**
** Note that, as provided in the License, the Software is distributed on an
** "AS IS" basis, with ALL EXPRESS AND IMPLIED WARRANTIES AND CONDITIONS
** DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY IMPLIED WARRANTIES AND
** CONDITIONS OF MERCHANTABILITY, SATISFACTORY QUALITY, FITNESS FOR A
** PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
**
** Original Code. The Original Code is: OpenGL Sample Implementation,
** Version 1.2.1, released January 26, 2000, developed by Silicon Graphics,
** Inc. The Original Code is Copyright (c) 1991-2000 Silicon Graphics, Inc.
** Copyright in any portions created by third parties is as indicated
** elsewhere herein. All Rights Reserved.
**
** Additional Notice Provisions: This software was created using the
** OpenGL(R) version 1.2.1 Sample Implementation published by SGI, but has
** not been independently verified as being compliant with the OpenGL(R)
** version 1.2.1 Specification.
*/

extern int __glXRender(__GLXclientState*, GLbyte*);
extern int __glXRenderLarge(__GLXclientState*, GLbyte*);
extern int __glXCreateContext(__GLXclientState*, GLbyte*);
extern int __glXDestroyContext(__GLXclientState*, GLbyte*);
extern int __glXMakeCurrent(__GLXclientState*, GLbyte*);
extern int __glXIsDirect(__GLXclientState*, GLbyte*);
extern int __glXQueryVersion(__GLXclientState*, GLbyte*);
extern int __glXWaitGL(__GLXclientState*, GLbyte*);
extern int __glXWaitX(__GLXclientState*, GLbyte*);
extern int __glXCopyContext(__GLXclientState*, GLbyte*);
extern int __glXSwapBuffers(__GLXclientState*, GLbyte*);
extern int __glXUseXFont(__GLXclientState*, GLbyte*);
extern int __glXCreateGLXPixmap(__GLXclientState*, GLbyte*);
extern int __glXGetVisualConfigs(__GLXclientState*, GLbyte*);
extern int __glXDestroyGLXPixmap(__GLXclientState*, GLbyte*);
extern int __glXVendorPrivate(__GLXclientState*, GLbyte*);
extern int __glXVendorPrivateWithReply(__GLXclientState*, GLbyte*);
extern int __glXQueryExtensionsString(__GLXclientState*, GLbyte*);
extern int __glXQueryServerString(__GLXclientState*, GLbyte*);
extern int __glXClientInfo(__GLXclientState*, GLbyte*);
extern int __glXMakeContextCurrent(__GLXclientState*, GLbyte*);
extern int __glXGetFBConfigs(__GLXclientState*, GLbyte*);
extern int __glXCreateNewContext(__GLXclientState*, GLbyte*);
extern int __glXCreatePixmap(__GLXclientState*, GLbyte*);

extern int __glXDisp_NewList(__GLXclientState*, GLbyte*);
extern int __glXDisp_EndList(__GLXclientState*, GLbyte*);
extern int __glXDisp_DeleteLists(__GLXclientState*, GLbyte*);
extern int __glXDisp_GenLists(__GLXclientState*, GLbyte*);
extern int __glXDisp_FeedbackBuffer(__GLXclientState*, GLbyte*);
extern int __glXDisp_SelectBuffer(__GLXclientState*, GLbyte*);
extern int __glXDisp_RenderMode(__GLXclientState*, GLbyte*);
extern int __glXDisp_Finish(__GLXclientState*, GLbyte*);
extern int __glXDisp_PixelStoref(__GLXclientState*, GLbyte*);
extern int __glXDisp_PixelStorei(__GLXclientState*, GLbyte*);
extern int __glXDisp_ReadPixels(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetBooleanv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetClipPlane(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetDoublev(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetError(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetFloatv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetIntegerv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetLightfv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetLightiv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetMapdv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetMapfv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetMapiv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetMaterialfv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetMaterialiv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetPixelMapfv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetPixelMapuiv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetPixelMapusv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetPolygonStipple(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetString(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetTexEnvfv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetTexEnviv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetTexGendv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetTexGenfv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetTexGeniv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetTexImage(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetTexParameterfv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetTexParameteriv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetTexLevelParameterfv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetTexLevelParameteriv(__GLXclientState*, GLbyte*);
extern int __glXDisp_IsEnabled(__GLXclientState*, GLbyte*);
extern int __glXDisp_IsList(__GLXclientState*, GLbyte*);
extern int __glXDisp_Flush(__GLXclientState*, GLbyte*);
extern int __glXDisp_AreTexturesResident(__GLXclientState*, GLbyte*);
extern int __glXDisp_DeleteTextures(__GLXclientState*, GLbyte*);
extern int __glXDisp_GenTextures(__GLXclientState*, GLbyte*);
extern int __glXDisp_IsTexture(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetColorTable(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetColorTableParameterfv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetColorTableParameteriv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetConvolutionFilter(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetConvolutionParameterfv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetConvolutionParameteriv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetSeparableFilter(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetHistogram(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetHistogramParameterfv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetHistogramParameteriv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetMinmax(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetMinmaxParameterfv(__GLXclientState*, GLbyte*);
extern int __glXDisp_GetMinmaxParameteriv(__GLXclientState*, GLbyte*);

extern GLvoid __glXDisp_CallList(GLbyte*);
extern GLvoid __glXDisp_CallLists(GLbyte*);
extern GLvoid __glXDisp_ListBase(GLbyte*);
extern GLvoid __glXDisp_Begin(GLbyte*);
extern GLvoid __glXDisp_Bitmap(GLbyte*);
extern GLvoid __glXDisp_Color3bv(GLbyte*);
extern GLvoid __glXDisp_Color3dv(GLbyte*);
extern GLvoid __glXDisp_Color3fv(GLbyte*);
extern GLvoid __glXDisp_Color3iv(GLbyte*);
extern GLvoid __glXDisp_Color3sv(GLbyte*);
extern GLvoid __glXDisp_Color3ubv(GLbyte*);
extern GLvoid __glXDisp_Color3uiv(GLbyte*);
extern GLvoid __glXDisp_Color3usv(GLbyte*);
extern GLvoid __glXDisp_Color4bv(GLbyte*);
extern GLvoid __glXDisp_Color4dv(GLbyte*);
extern GLvoid __glXDisp_Color4fv(GLbyte*);
extern GLvoid __glXDisp_Color4iv(GLbyte*);
extern GLvoid __glXDisp_Color4sv(GLbyte*);
extern GLvoid __glXDisp_Color4ubv(GLbyte*);
extern GLvoid __glXDisp_Color4uiv(GLbyte*);
extern GLvoid __glXDisp_Color4usv(GLbyte*);
extern GLvoid __glXDisp_EdgeFlagv(GLbyte*);
extern GLvoid __glXDisp_End(GLbyte*);
extern GLvoid __glXDisp_Indexdv(GLbyte*);
extern GLvoid __glXDisp_Indexfv(GLbyte*);
extern GLvoid __glXDisp_Indexiv(GLbyte*);
extern GLvoid __glXDisp_Indexsv(GLbyte*);
extern GLvoid __glXDisp_Normal3bv(GLbyte*);
extern GLvoid __glXDisp_Normal3dv(GLbyte*);
extern GLvoid __glXDisp_Normal3fv(GLbyte*);
extern GLvoid __glXDisp_Normal3iv(GLbyte*);
extern GLvoid __glXDisp_Normal3sv(GLbyte*);
extern GLvoid __glXDisp_RasterPos2dv(GLbyte*);
extern GLvoid __glXDisp_RasterPos2fv(GLbyte*);
extern GLvoid __glXDisp_RasterPos2iv(GLbyte*);
extern GLvoid __glXDisp_RasterPos2sv(GLbyte*);
extern GLvoid __glXDisp_RasterPos3dv(GLbyte*);
extern GLvoid __glXDisp_RasterPos3fv(GLbyte*);
extern GLvoid __glXDisp_RasterPos3iv(GLbyte*);
extern GLvoid __glXDisp_RasterPos3sv(GLbyte*);
extern GLvoid __glXDisp_RasterPos4dv(GLbyte*);
extern GLvoid __glXDisp_RasterPos4fv(GLbyte*);
extern GLvoid __glXDisp_RasterPos4iv(GLbyte*);
extern GLvoid __glXDisp_RasterPos4sv(GLbyte*);
extern GLvoid __glXDisp_Rectdv(GLbyte*);
extern GLvoid __glXDisp_Rectfv(GLbyte*);
extern GLvoid __glXDisp_Rectiv(GLbyte*);
extern GLvoid __glXDisp_Rectsv(GLbyte*);
extern GLvoid __glXDisp_TexCoord1dv(GLbyte*);
extern GLvoid __glXDisp_TexCoord1fv(GLbyte*);
extern GLvoid __glXDisp_TexCoord1iv(GLbyte*);
extern GLvoid __glXDisp_TexCoord1sv(GLbyte*);
extern GLvoid __glXDisp_TexCoord2dv(GLbyte*);
extern GLvoid __glXDisp_TexCoord2fv(GLbyte*);
extern GLvoid __glXDisp_TexCoord2iv(GLbyte*);
extern GLvoid __glXDisp_TexCoord2sv(GLbyte*);
extern GLvoid __glXDisp_TexCoord3dv(GLbyte*);
extern GLvoid __glXDisp_TexCoord3fv(GLbyte*);
extern GLvoid __glXDisp_TexCoord3iv(GLbyte*);
extern GLvoid __glXDisp_TexCoord3sv(GLbyte*);
extern GLvoid __glXDisp_TexCoord4dv(GLbyte*);
extern GLvoid __glXDisp_TexCoord4fv(GLbyte*);
extern GLvoid __glXDisp_TexCoord4iv(GLbyte*);
extern GLvoid __glXDisp_TexCoord4sv(GLbyte*);
extern GLvoid __glXDisp_Vertex2dv(GLbyte*);
extern GLvoid __glXDisp_Vertex2fv(GLbyte*);
extern GLvoid __glXDisp_Vertex2iv(GLbyte*);
extern GLvoid __glXDisp_Vertex2sv(GLbyte*);
extern GLvoid __glXDisp_Vertex3dv(GLbyte*);
extern GLvoid __glXDisp_Vertex3fv(GLbyte*);
extern GLvoid __glXDisp_Vertex3iv(GLbyte*);
extern GLvoid __glXDisp_Vertex3sv(GLbyte*);
extern GLvoid __glXDisp_Vertex4dv(GLbyte*);
extern GLvoid __glXDisp_Vertex4fv(GLbyte*);
extern GLvoid __glXDisp_Vertex4iv(GLbyte*);
extern GLvoid __glXDisp_Vertex4sv(GLbyte*);
extern GLvoid __glXDisp_ClipPlane(GLbyte*);
extern GLvoid __glXDisp_ColorMaterial(GLbyte*);
extern GLvoid __glXDisp_CullFace(GLbyte*);
extern GLvoid __glXDisp_Fogf(GLbyte*);
extern GLvoid __glXDisp_Fogfv(GLbyte*);
extern GLvoid __glXDisp_Fogi(GLbyte*);
extern GLvoid __glXDisp_Fogiv(GLbyte*);
extern GLvoid __glXDisp_FrontFace(GLbyte*);
extern GLvoid __glXDisp_Hint(GLbyte*);
extern GLvoid __glXDisp_Lightf(GLbyte*);
extern GLvoid __glXDisp_Lightfv(GLbyte*);
extern GLvoid __glXDisp_Lighti(GLbyte*);
extern GLvoid __glXDisp_Lightiv(GLbyte*);
extern GLvoid __glXDisp_LightModelf(GLbyte*);
extern GLvoid __glXDisp_LightModelfv(GLbyte*);
extern GLvoid __glXDisp_LightModeli(GLbyte*);
extern GLvoid __glXDisp_LightModeliv(GLbyte*);
extern GLvoid __glXDisp_LineStipple(GLbyte*);
extern GLvoid __glXDisp_LineWidth(GLbyte*);
extern GLvoid __glXDisp_Materialf(GLbyte*);
extern GLvoid __glXDisp_Materialfv(GLbyte*);
extern GLvoid __glXDisp_Materiali(GLbyte*);
extern GLvoid __glXDisp_Materialiv(GLbyte*);
extern GLvoid __glXDisp_PointSize(GLbyte*);
extern GLvoid __glXDisp_PolygonMode(GLbyte*);
extern GLvoid __glXDisp_PolygonStipple(GLbyte*);
extern GLvoid __glXDisp_Scissor(GLbyte*);
extern GLvoid __glXDisp_ShadeModel(GLbyte*);
extern GLvoid __glXDisp_TexParameterf(GLbyte*);
extern GLvoid __glXDisp_TexParameterfv(GLbyte*);
extern GLvoid __glXDisp_TexParameteri(GLbyte*);
extern GLvoid __glXDisp_TexParameteriv(GLbyte*);
extern GLvoid __glXDisp_TexImage1D(GLbyte*);
extern GLvoid __glXDisp_TexImage2D(GLbyte*);
extern GLvoid __glXDisp_TexEnvf(GLbyte*);
extern GLvoid __glXDisp_TexEnvfv(GLbyte*);
extern GLvoid __glXDisp_TexEnvi(GLbyte*);
extern GLvoid __glXDisp_TexEnviv(GLbyte*);
extern GLvoid __glXDisp_TexGend(GLbyte*);
extern GLvoid __glXDisp_TexGendv(GLbyte*);
extern GLvoid __glXDisp_TexGenf(GLbyte*);
extern GLvoid __glXDisp_TexGenfv(GLbyte*);
extern GLvoid __glXDisp_TexGeni(GLbyte*);
extern GLvoid __glXDisp_TexGeniv(GLbyte*);
extern GLvoid __glXDisp_InitNames(GLbyte*);
extern GLvoid __glXDisp_LoadName(GLbyte*);
extern GLvoid __glXDisp_PassThrough(GLbyte*);
extern GLvoid __glXDisp_PopName(GLbyte*);
extern GLvoid __glXDisp_PushName(GLbyte*);
extern GLvoid __glXDisp_DrawBuffer(GLbyte*);
extern GLvoid __glXDisp_Clear(GLbyte*);
extern GLvoid __glXDisp_ClearAccum(GLbyte*);
extern GLvoid __glXDisp_ClearIndex(GLbyte*);
extern GLvoid __glXDisp_ClearColor(GLbyte*);
extern GLvoid __glXDisp_ClearStencil(GLbyte*);
extern GLvoid __glXDisp_ClearDepth(GLbyte*);
extern GLvoid __glXDisp_StencilMask(GLbyte*);
extern GLvoid __glXDisp_ColorMask(GLbyte*);
extern GLvoid __glXDisp_DepthMask(GLbyte*);
extern GLvoid __glXDisp_IndexMask(GLbyte*);
extern GLvoid __glXDisp_Accum(GLbyte*);
extern GLvoid __glXDisp_Disable(GLbyte*);
extern GLvoid __glXDisp_Enable(GLbyte*);
extern GLvoid __glXDisp_PopAttrib(GLbyte*);
extern GLvoid __glXDisp_PushAttrib(GLbyte*);
extern GLvoid __glXDisp_Map1d(GLbyte*);
extern GLvoid __glXDisp_Map1f(GLbyte*);
extern GLvoid __glXDisp_Map2d(GLbyte*);
extern GLvoid __glXDisp_Map2f(GLbyte*);
extern GLvoid __glXDisp_MapGrid1d(GLbyte*);
extern GLvoid __glXDisp_MapGrid1f(GLbyte*);
extern GLvoid __glXDisp_MapGrid2d(GLbyte*);
extern GLvoid __glXDisp_MapGrid2f(GLbyte*);
extern GLvoid __glXDisp_EvalCoord1dv(GLbyte*);
extern GLvoid __glXDisp_EvalCoord1fv(GLbyte*);
extern GLvoid __glXDisp_EvalCoord2dv(GLbyte*);
extern GLvoid __glXDisp_EvalCoord2fv(GLbyte*);
extern GLvoid __glXDisp_EvalMesh1(GLbyte*);
extern GLvoid __glXDisp_EvalPoint1(GLbyte*);
extern GLvoid __glXDisp_EvalMesh2(GLbyte*);
extern GLvoid __glXDisp_EvalPoint2(GLbyte*);
extern GLvoid __glXDisp_AlphaFunc(GLbyte*);
extern GLvoid __glXDisp_BlendFunc(GLbyte*);
extern GLvoid __glXDisp_LogicOp(GLbyte*);
extern GLvoid __glXDisp_StencilFunc(GLbyte*);
extern GLvoid __glXDisp_StencilOp(GLbyte*);
extern GLvoid __glXDisp_DepthFunc(GLbyte*);
extern GLvoid __glXDisp_PixelZoom(GLbyte*);
extern GLvoid __glXDisp_PixelTransferf(GLbyte*);
extern GLvoid __glXDisp_PixelTransferi(GLbyte*);
extern GLvoid __glXDisp_PixelMapfv(GLbyte*);
extern GLvoid __glXDisp_PixelMapuiv(GLbyte*);
extern GLvoid __glXDisp_PixelMapusv(GLbyte*);
extern GLvoid __glXDisp_ReadBuffer(GLbyte*);
extern GLvoid __glXDisp_CopyPixels(GLbyte*);
extern GLvoid __glXDisp_DrawPixels(GLbyte*);
extern GLvoid __glXDisp_DepthRange(GLbyte*);
extern GLvoid __glXDisp_Frustum(GLbyte*);
extern GLvoid __glXDisp_LoadIdentity(GLbyte*);
extern GLvoid __glXDisp_LoadMatrixf(GLbyte*);
extern GLvoid __glXDisp_LoadMatrixd(GLbyte*);
extern GLvoid __glXDisp_MatrixMode(GLbyte*);
extern GLvoid __glXDisp_MultMatrixf(GLbyte*);
extern GLvoid __glXDisp_MultMatrixd(GLbyte*);
extern GLvoid __glXDisp_Ortho(GLbyte*);
extern GLvoid __glXDisp_PopMatrix(GLbyte*);
extern GLvoid __glXDisp_PushMatrix(GLbyte*);
extern GLvoid __glXDisp_Rotated(GLbyte*);
extern GLvoid __glXDisp_Rotatef(GLbyte*);
extern GLvoid __glXDisp_Scaled(GLbyte*);
extern GLvoid __glXDisp_Scalef(GLbyte*);
extern GLvoid __glXDisp_Translated(GLbyte*);
extern GLvoid __glXDisp_Translatef(GLbyte*);
extern GLvoid __glXDisp_Viewport(GLbyte*);
extern GLvoid __glXDisp_PolygonOffset(GLbyte*);
extern GLvoid __glXDisp_DrawArrays(GLbyte*);
extern GLvoid __glXDisp_Indexubv(GLbyte*);
extern GLvoid __glXDisp_ColorSubTable(GLbyte*);
extern GLvoid __glXDisp_CopyColorSubTable(GLbyte*);
extern GLvoid __glXDisp_ActiveTextureARB(GLbyte*);
extern GLvoid __glXDisp_MultiTexCoord1dvARB(GLbyte*);
extern GLvoid __glXDisp_MultiTexCoord1fvARB(GLbyte*);
extern GLvoid __glXDisp_MultiTexCoord1ivARB(GLbyte*);
extern GLvoid __glXDisp_MultiTexCoord1svARB(GLbyte*);
extern GLvoid __glXDisp_MultiTexCoord2dvARB(GLbyte*);
extern GLvoid __glXDisp_MultiTexCoord2fvARB(GLbyte*);
extern GLvoid __glXDisp_MultiTexCoord2ivARB(GLbyte*);
extern GLvoid __glXDisp_MultiTexCoord2svARB(GLbyte*);
extern GLvoid __glXDisp_MultiTexCoord3dvARB(GLbyte*);
extern GLvoid __glXDisp_MultiTexCoord3fvARB(GLbyte*);
extern GLvoid __glXDisp_MultiTexCoord3ivARB(GLbyte*);
extern GLvoid __glXDisp_MultiTexCoord3svARB(GLbyte*);
extern GLvoid __glXDisp_MultiTexCoord4dvARB(GLbyte*);
extern GLvoid __glXDisp_MultiTexCoord4fvARB(GLbyte*);
extern GLvoid __glXDisp_MultiTexCoord4ivARB(GLbyte*);
extern GLvoid __glXDisp_MultiTexCoord4svARB(GLbyte*);
extern GLvoid __glXDisp_SampleCoverageARB(GLbyte *);
extern GLvoid __glXDisp_WindowPos3fARB(GLbyte *);

extern int __glXSwapRender(__GLXclientState*, GLbyte*);
extern int __glXSwapRenderLarge(__GLXclientState*, GLbyte*);
extern int __glXSwapCreateContext(__GLXclientState*, GLbyte*);
extern int __glXSwapDestroyContext(__GLXclientState*, GLbyte*);
extern int __glXSwapMakeCurrent(__GLXclientState*, GLbyte*);
extern int __glXSwapIsDirect(__GLXclientState*, GLbyte*);
extern int __glXSwapQueryVersion(__GLXclientState*, GLbyte*);
extern int __glXSwapWaitGL(__GLXclientState*, GLbyte*);
extern int __glXSwapWaitX(__GLXclientState*, GLbyte*);
extern int __glXSwapCopyContext(__GLXclientState*, GLbyte*);
extern int __glXSwapSwapBuffers(__GLXclientState*, GLbyte*);
extern int __glXSwapUseXFont(__GLXclientState*, GLbyte*);
extern int __glXSwapCreateGLXPixmap(__GLXclientState*, GLbyte*);
extern int __glXSwapGetVisualConfigs(__GLXclientState*, GLbyte*);
extern int __glXSwapDestroyGLXPixmap(__GLXclientState*, GLbyte*);
extern int __glXSwapVendorPrivate(__GLXclientState*, GLbyte*);
extern int __glXSwapVendorPrivateWithReply(__GLXclientState*, GLbyte*);
extern int __glXSwapQueryExtensionsString(__GLXclientState*, GLbyte*);
extern int __glXSwapQueryServerString(__GLXclientState*, GLbyte*);
extern int __glXSwapClientInfo(__GLXclientState*, GLbyte*);
extern int __glXSwapMakeContextCurrent(__GLXclientState*, GLbyte*);
extern int __glXSwapGetFBConfigs(__GLXclientState*, GLbyte*);
extern int __glXSwapCreateNewContext(__GLXclientState*, GLbyte*);
extern int __glXSwapCreatePixmap(__GLXclientState*, GLbyte*);

extern int __glXDispSwap_NewList(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_EndList(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_DeleteLists(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GenLists(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_FeedbackBuffer(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_SelectBuffer(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_RenderMode(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_Finish(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_PixelStoref(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_PixelStorei(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_ReadPixels(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetBooleanv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetClipPlane(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetDoublev(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetError(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetFloatv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetIntegerv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetLightfv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetLightiv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetMapdv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetMapfv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetMapiv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetMaterialfv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetMaterialiv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetPixelMapfv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetPixelMapuiv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetPixelMapusv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetPolygonStipple(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetString(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetTexEnvfv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetTexEnviv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetTexGendv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetTexGenfv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetTexGeniv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetTexImage(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetTexParameterfv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetTexParameteriv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetTexLevelParameterfv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetTexLevelParameteriv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_IsEnabled(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_IsList(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_Flush(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_AreTexturesResident(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_DeleteTextures(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GenTextures(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_IsTexture(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetColorTable(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetColorTableParameterfv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetColorTableParameteriv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetConvolutionFilter(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetConvolutionParameterfv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetConvolutionParameteriv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetSeparableFilter(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetHistogram(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetHistogramParameterfv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetHistogramParameteriv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetMinmax(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetMinmaxParameterfv(__GLXclientState*, GLbyte*);
extern int __glXDispSwap_GetMinmaxParameteriv(__GLXclientState*, GLbyte*);

extern GLvoid __glXDispSwap_CallList(GLbyte*);
extern GLvoid __glXDispSwap_CallLists(GLbyte*);
extern GLvoid __glXDispSwap_ListBase(GLbyte*);
extern GLvoid __glXDispSwap_Begin(GLbyte*);
extern GLvoid __glXDispSwap_Bitmap(GLbyte*);
extern GLvoid __glXDispSwap_Color3bv(GLbyte*);
extern GLvoid __glXDispSwap_Color3dv(GLbyte*);
extern GLvoid __glXDispSwap_Color3fv(GLbyte*);
extern GLvoid __glXDispSwap_Color3iv(GLbyte*);
extern GLvoid __glXDispSwap_Color3sv(GLbyte*);
extern GLvoid __glXDispSwap_Color3ubv(GLbyte*);
extern GLvoid __glXDispSwap_Color3uiv(GLbyte*);
extern GLvoid __glXDispSwap_Color3usv(GLbyte*);
extern GLvoid __glXDispSwap_Color4bv(GLbyte*);
extern GLvoid __glXDispSwap_Color4dv(GLbyte*);
extern GLvoid __glXDispSwap_Color4fv(GLbyte*);
extern GLvoid __glXDispSwap_Color4iv(GLbyte*);
extern GLvoid __glXDispSwap_Color4sv(GLbyte*);
extern GLvoid __glXDispSwap_Color4ubv(GLbyte*);
extern GLvoid __glXDispSwap_Color4uiv(GLbyte*);
extern GLvoid __glXDispSwap_Color4usv(GLbyte*);
extern GLvoid __glXDispSwap_EdgeFlagv(GLbyte*);
extern GLvoid __glXDispSwap_End(GLbyte*);
extern GLvoid __glXDispSwap_Indexdv(GLbyte*);
extern GLvoid __glXDispSwap_Indexfv(GLbyte*);
extern GLvoid __glXDispSwap_Indexiv(GLbyte*);
extern GLvoid __glXDispSwap_Indexsv(GLbyte*);
extern GLvoid __glXDispSwap_Normal3bv(GLbyte*);
extern GLvoid __glXDispSwap_Normal3dv(GLbyte*);
extern GLvoid __glXDispSwap_Normal3fv(GLbyte*);
extern GLvoid __glXDispSwap_Normal3iv(GLbyte*);
extern GLvoid __glXDispSwap_Normal3sv(GLbyte*);
extern GLvoid __glXDispSwap_RasterPos2dv(GLbyte*);
extern GLvoid __glXDispSwap_RasterPos2fv(GLbyte*);
extern GLvoid __glXDispSwap_RasterPos2iv(GLbyte*);
extern GLvoid __glXDispSwap_RasterPos2sv(GLbyte*);
extern GLvoid __glXDispSwap_RasterPos3dv(GLbyte*);
extern GLvoid __glXDispSwap_RasterPos3fv(GLbyte*);
extern GLvoid __glXDispSwap_RasterPos3iv(GLbyte*);
extern GLvoid __glXDispSwap_RasterPos3sv(GLbyte*);
extern GLvoid __glXDispSwap_RasterPos4dv(GLbyte*);
extern GLvoid __glXDispSwap_RasterPos4fv(GLbyte*);
extern GLvoid __glXDispSwap_RasterPos4iv(GLbyte*);
extern GLvoid __glXDispSwap_RasterPos4sv(GLbyte*);
extern GLvoid __glXDispSwap_Rectdv(GLbyte*);
extern GLvoid __glXDispSwap_Rectfv(GLbyte*);
extern GLvoid __glXDispSwap_Rectiv(GLbyte*);
extern GLvoid __glXDispSwap_Rectsv(GLbyte*);
extern GLvoid __glXDispSwap_TexCoord1dv(GLbyte*);
extern GLvoid __glXDispSwap_TexCoord1fv(GLbyte*);
extern GLvoid __glXDispSwap_TexCoord1iv(GLbyte*);
extern GLvoid __glXDispSwap_TexCoord1sv(GLbyte*);
extern GLvoid __glXDispSwap_TexCoord2dv(GLbyte*);
extern GLvoid __glXDispSwap_TexCoord2fv(GLbyte*);
extern GLvoid __glXDispSwap_TexCoord2iv(GLbyte*);
extern GLvoid __glXDispSwap_TexCoord2sv(GLbyte*);
extern GLvoid __glXDispSwap_TexCoord3dv(GLbyte*);
extern GLvoid __glXDispSwap_TexCoord3fv(GLbyte*);
extern GLvoid __glXDispSwap_TexCoord3iv(GLbyte*);
extern GLvoid __glXDispSwap_TexCoord3sv(GLbyte*);
extern GLvoid __glXDispSwap_TexCoord4dv(GLbyte*);
extern GLvoid __glXDispSwap_TexCoord4fv(GLbyte*);
extern GLvoid __glXDispSwap_TexCoord4iv(GLbyte*);
extern GLvoid __glXDispSwap_TexCoord4sv(GLbyte*);
extern GLvoid __glXDispSwap_Vertex2dv(GLbyte*);
extern GLvoid __glXDispSwap_Vertex2fv(GLbyte*);
extern GLvoid __glXDispSwap_Vertex2iv(GLbyte*);
extern GLvoid __glXDispSwap_Vertex2sv(GLbyte*);
extern GLvoid __glXDispSwap_Vertex3dv(GLbyte*);
extern GLvoid __glXDispSwap_Vertex3fv(GLbyte*);
extern GLvoid __glXDispSwap_Vertex3iv(GLbyte*);
extern GLvoid __glXDispSwap_Vertex3sv(GLbyte*);
extern GLvoid __glXDispSwap_Vertex4dv(GLbyte*);
extern GLvoid __glXDispSwap_Vertex4fv(GLbyte*);
extern GLvoid __glXDispSwap_Vertex4iv(GLbyte*);
extern GLvoid __glXDispSwap_Vertex4sv(GLbyte*);
extern GLvoid __glXDispSwap_ClipPlane(GLbyte*);
extern GLvoid __glXDispSwap_ColorMaterial(GLbyte*);
extern GLvoid __glXDispSwap_CullFace(GLbyte*);
extern GLvoid __glXDispSwap_Fogf(GLbyte*);
extern GLvoid __glXDispSwap_Fogfv(GLbyte*);
extern GLvoid __glXDispSwap_Fogi(GLbyte*);
extern GLvoid __glXDispSwap_Fogiv(GLbyte*);
extern GLvoid __glXDispSwap_FrontFace(GLbyte*);
extern GLvoid __glXDispSwap_Hint(GLbyte*);
extern GLvoid __glXDispSwap_Lightf(GLbyte*);
extern GLvoid __glXDispSwap_Lightfv(GLbyte*);
extern GLvoid __glXDispSwap_Lighti(GLbyte*);
extern GLvoid __glXDispSwap_Lightiv(GLbyte*);
extern GLvoid __glXDispSwap_LightModelf(GLbyte*);
extern GLvoid __glXDispSwap_LightModelfv(GLbyte*);
extern GLvoid __glXDispSwap_LightModeli(GLbyte*);
extern GLvoid __glXDispSwap_LightModeliv(GLbyte*);
extern GLvoid __glXDispSwap_LineStipple(GLbyte*);
extern GLvoid __glXDispSwap_LineWidth(GLbyte*);
extern GLvoid __glXDispSwap_Materialf(GLbyte*);
extern GLvoid __glXDispSwap_Materialfv(GLbyte*);
extern GLvoid __glXDispSwap_Materiali(GLbyte*);
extern GLvoid __glXDispSwap_Materialiv(GLbyte*);
extern GLvoid __glXDispSwap_PointSize(GLbyte*);
extern GLvoid __glXDispSwap_PolygonMode(GLbyte*);
extern GLvoid __glXDispSwap_PolygonStipple(GLbyte*);
extern GLvoid __glXDispSwap_Scissor(GLbyte*);
extern GLvoid __glXDispSwap_ShadeModel(GLbyte*);
extern GLvoid __glXDispSwap_TexParameterf(GLbyte*);
extern GLvoid __glXDispSwap_TexParameterfv(GLbyte*);
extern GLvoid __glXDispSwap_TexParameteri(GLbyte*);
extern GLvoid __glXDispSwap_TexParameteriv(GLbyte*);
extern GLvoid __glXDispSwap_TexImage1D(GLbyte*);
extern GLvoid __glXDispSwap_TexImage2D(GLbyte*);
extern GLvoid __glXDispSwap_TexEnvf(GLbyte*);
extern GLvoid __glXDispSwap_TexEnvfv(GLbyte*);
extern GLvoid __glXDispSwap_TexEnvi(GLbyte*);
extern GLvoid __glXDispSwap_TexEnviv(GLbyte*);
extern GLvoid __glXDispSwap_TexGend(GLbyte*);
extern GLvoid __glXDispSwap_TexGendv(GLbyte*);
extern GLvoid __glXDispSwap_TexGenf(GLbyte*);
extern GLvoid __glXDispSwap_TexGenfv(GLbyte*);
extern GLvoid __glXDispSwap_TexGeni(GLbyte*);
extern GLvoid __glXDispSwap_TexGeniv(GLbyte*);
extern GLvoid __glXDispSwap_InitNames(GLbyte*);
extern GLvoid __glXDispSwap_LoadName(GLbyte*);
extern GLvoid __glXDispSwap_PassThrough(GLbyte*);
extern GLvoid __glXDispSwap_PopName(GLbyte*);
extern GLvoid __glXDispSwap_PushName(GLbyte*);
extern GLvoid __glXDispSwap_DrawBuffer(GLbyte*);
extern GLvoid __glXDispSwap_Clear(GLbyte*);
extern GLvoid __glXDispSwap_ClearAccum(GLbyte*);
extern GLvoid __glXDispSwap_ClearIndex(GLbyte*);
extern GLvoid __glXDispSwap_ClearColor(GLbyte*);
extern GLvoid __glXDispSwap_ClearStencil(GLbyte*);
extern GLvoid __glXDispSwap_ClearDepth(GLbyte*);
extern GLvoid __glXDispSwap_StencilMask(GLbyte*);
extern GLvoid __glXDispSwap_ColorMask(GLbyte*);
extern GLvoid __glXDispSwap_DepthMask(GLbyte*);
extern GLvoid __glXDispSwap_IndexMask(GLbyte*);
extern GLvoid __glXDispSwap_Accum(GLbyte*);
extern GLvoid __glXDispSwap_Disable(GLbyte*);
extern GLvoid __glXDispSwap_Enable(GLbyte*);
extern GLvoid __glXDispSwap_PopAttrib(GLbyte*);
extern GLvoid __glXDispSwap_PushAttrib(GLbyte*);
extern GLvoid __glXDispSwap_Map1d(GLbyte*);
extern GLvoid __glXDispSwap_Map1f(GLbyte*);
extern GLvoid __glXDispSwap_Map2d(GLbyte*);
extern GLvoid __glXDispSwap_Map2f(GLbyte*);
extern GLvoid __glXDispSwap_MapGrid1d(GLbyte*);
extern GLvoid __glXDispSwap_MapGrid1f(GLbyte*);
extern GLvoid __glXDispSwap_MapGrid2d(GLbyte*);
extern GLvoid __glXDispSwap_MapGrid2f(GLbyte*);
extern GLvoid __glXDispSwap_EvalCoord1dv(GLbyte*);
extern GLvoid __glXDispSwap_EvalCoord1fv(GLbyte*);
extern GLvoid __glXDispSwap_EvalCoord2dv(GLbyte*);
extern GLvoid __glXDispSwap_EvalCoord2fv(GLbyte*);
extern GLvoid __glXDispSwap_EvalMesh1(GLbyte*);
extern GLvoid __glXDispSwap_EvalPoint1(GLbyte*);
extern GLvoid __glXDispSwap_EvalMesh2(GLbyte*);
extern GLvoid __glXDispSwap_EvalPoint2(GLbyte*);
extern GLvoid __glXDispSwap_AlphaFunc(GLbyte*);
extern GLvoid __glXDispSwap_BlendFunc(GLbyte*);
extern GLvoid __glXDispSwap_LogicOp(GLbyte*);
extern GLvoid __glXDispSwap_StencilFunc(GLbyte*);
extern GLvoid __glXDispSwap_StencilOp(GLbyte*);
extern GLvoid __glXDispSwap_DepthFunc(GLbyte*);
extern GLvoid __glXDispSwap_PixelZoom(GLbyte*);
extern GLvoid __glXDispSwap_PixelTransferf(GLbyte*);
extern GLvoid __glXDispSwap_PixelTransferi(GLbyte*);
extern GLvoid __glXDispSwap_PixelMapfv(GLbyte*);
extern GLvoid __glXDispSwap_PixelMapuiv(GLbyte*);
extern GLvoid __glXDispSwap_PixelMapusv(GLbyte*);
extern GLvoid __glXDispSwap_ReadBuffer(GLbyte*);
extern GLvoid __glXDispSwap_CopyPixels(GLbyte*);
extern GLvoid __glXDispSwap_DrawPixels(GLbyte*);
extern GLvoid __glXDispSwap_DepthRange(GLbyte*);
extern GLvoid __glXDispSwap_Frustum(GLbyte*);
extern GLvoid __glXDispSwap_LoadIdentity(GLbyte*);
extern GLvoid __glXDispSwap_LoadMatrixf(GLbyte*);
extern GLvoid __glXDispSwap_LoadMatrixd(GLbyte*);
extern GLvoid __glXDispSwap_MatrixMode(GLbyte*);
extern GLvoid __glXDispSwap_MultMatrixf(GLbyte*);
extern GLvoid __glXDispSwap_MultMatrixd(GLbyte*);
extern GLvoid __glXDispSwap_Ortho(GLbyte*);
extern GLvoid __glXDispSwap_PopMatrix(GLbyte*);
extern GLvoid __glXDispSwap_PushMatrix(GLbyte*);
extern GLvoid __glXDispSwap_Rotated(GLbyte*);
extern GLvoid __glXDispSwap_Rotatef(GLbyte*);
extern GLvoid __glXDispSwap_Scaled(GLbyte*);
extern GLvoid __glXDispSwap_Scalef(GLbyte*);
extern GLvoid __glXDispSwap_Translated(GLbyte*);
extern GLvoid __glXDispSwap_Translatef(GLbyte*);
extern GLvoid __glXDispSwap_Viewport(GLbyte*);
extern GLvoid __glXDispSwap_PolygonOffset(GLbyte*);
extern GLvoid __glXDispSwap_DrawArrays(GLbyte*);
extern GLvoid __glXDispSwap_Indexubv(GLbyte*);
extern GLvoid __glXDispSwap_ColorSubTable(GLbyte*);
extern GLvoid __glXDispSwap_CopyColorSubTable(GLbyte*);
extern GLvoid __glXDispSwap_ActiveTextureARB(GLbyte*);
extern GLvoid __glXDispSwap_MultiTexCoord1dvARB(GLbyte*);
extern GLvoid __glXDispSwap_MultiTexCoord1fvARB(GLbyte*);
extern GLvoid __glXDispSwap_MultiTexCoord1ivARB(GLbyte*);
extern GLvoid __glXDispSwap_MultiTexCoord1svARB(GLbyte*);
extern GLvoid __glXDispSwap_MultiTexCoord2dvARB(GLbyte*);
extern GLvoid __glXDispSwap_MultiTexCoord2fvARB(GLbyte*);
extern GLvoid __glXDispSwap_MultiTexCoord2ivARB(GLbyte*);
extern GLvoid __glXDispSwap_MultiTexCoord2svARB(GLbyte*);
extern GLvoid __glXDispSwap_MultiTexCoord3dvARB(GLbyte*);
extern GLvoid __glXDispSwap_MultiTexCoord3fvARB(GLbyte*);
extern GLvoid __glXDispSwap_MultiTexCoord3ivARB(GLbyte*);
extern GLvoid __glXDispSwap_MultiTexCoord3svARB(GLbyte*);
extern GLvoid __glXDispSwap_MultiTexCoord4dvARB(GLbyte*);
extern GLvoid __glXDispSwap_MultiTexCoord4fvARB(GLbyte*);
extern GLvoid __glXDispSwap_MultiTexCoord4ivARB(GLbyte*);
extern GLvoid __glXDispSwap_MultiTexCoord4svARB(GLbyte*);
extern GLvoid __glXDispSwap_SampleCoverageARB(GLbyte *);
extern GLvoid __glXDispSwap_WindowPos3fARB(GLbyte *);

#define __GLX_MIN_GLXCMD_OPCODE 1
#define __GLX_MAX_GLXCMD_OPCODE 20
#define __GLX_MIN_RENDER_OPCODE 1
/*#define __GLX_MAX_RENDER_OPCODE 213*/
#define __GLX_MAX_RENDER_OPCODE 230
#define __GLX_MIN_SINGLE_OPCODE 1
#define __GLX_MAX_SINGLE_OPCODE 159
#define __GLX_SINGLE_TABLE_SIZE 160
/*#define __GLX_RENDER_TABLE_SIZE 214*/
#define __GLX_RENDER_TABLE_SIZE 231
extern __GLXdispatchRenderProcPtr __glXRenderTable[__GLX_RENDER_TABLE_SIZE];
extern __GLXdispatchSingleProcPtr __glXSingleTable[__GLX_SINGLE_TABLE_SIZE];
extern __GLXdispatchRenderProcPtr __glXSwapRenderTable[__GLX_RENDER_TABLE_SIZE];
extern __GLXdispatchSingleProcPtr __glXSwapSingleTable[__GLX_SINGLE_TABLE_SIZE];
#endif /* _GLX_g_disptab_h_ */
