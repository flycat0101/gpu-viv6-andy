/* $XFree86: xc/lib/GL/glx/indirect_init.c,v 1.7 2002/02/22 21:32:54 dawes Exp $ */
/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Kevin E. Martin <kevin@precisioninsight.com>
 *   Brian Paul <brian@precisioninsight.com>
 */

#include "gl.h"
#include "indirect.h"
#include "glxclient.h"
#include "glcore/g_disp.h"


__GLdispatchTable *__glXNewIndirectAPI(GLvoid)
{
    __GLdispatchTable *indDispatchTab;

    indDispatchTab = (__GLdispatchTable *) Xmalloc(sizeof(__GLdispatchTable));

    /* Initialize the GL API entries */
    indDispatchTab->Accum = __indirect_glAccum;
    indDispatchTab->AlphaFunc = __indirect_glAlphaFunc;
    indDispatchTab->AreTexturesResident = __indirect_glAreTexturesResident;
    indDispatchTab->ArrayElement = __indirect_glArrayElement;
    indDispatchTab->Begin = __indirect_glBegin;
    indDispatchTab->BindTexture = __indirect_glBindTexture;
    indDispatchTab->Bitmap = __indirect_glBitmap;
    indDispatchTab->BlendFunc = __indirect_glBlendFunc;
    indDispatchTab->CallList = __indirect_glCallList;
    indDispatchTab->CallLists = __indirect_glCallLists;
    indDispatchTab->Clear = __indirect_glClear;
    indDispatchTab->ClearAccum = __indirect_glClearAccum;
    indDispatchTab->ClearColor = __indirect_glClearColor;
    indDispatchTab->ClearDepth = __indirect_glClearDepth;
    indDispatchTab->ClearIndex = __indirect_glClearIndex;
    indDispatchTab->ClearStencil = __indirect_glClearStencil;
    indDispatchTab->ClipPlane = __indirect_glClipPlane;
    indDispatchTab->Color3b = __indirect_glColor3b;
    indDispatchTab->Color3bv = __indirect_glColor3bv;
    indDispatchTab->Color3d = __indirect_glColor3d;
    indDispatchTab->Color3dv = __indirect_glColor3dv;
    indDispatchTab->Color3f = __indirect_glColor3f;
    indDispatchTab->Color3fv = __indirect_glColor3fv;
    indDispatchTab->Color3i = __indirect_glColor3i;
    indDispatchTab->Color3iv = __indirect_glColor3iv;
    indDispatchTab->Color3s = __indirect_glColor3s;
    indDispatchTab->Color3sv = __indirect_glColor3sv;
    indDispatchTab->Color3ub = __indirect_glColor3ub;
    indDispatchTab->Color3ubv = __indirect_glColor3ubv;
    indDispatchTab->Color3ui = __indirect_glColor3ui;
    indDispatchTab->Color3uiv = __indirect_glColor3uiv;
    indDispatchTab->Color3us = __indirect_glColor3us;
    indDispatchTab->Color3usv = __indirect_glColor3usv;
    indDispatchTab->Color4b = __indirect_glColor4b;
    indDispatchTab->Color4bv = __indirect_glColor4bv;
    indDispatchTab->Color4d = __indirect_glColor4d;
    indDispatchTab->Color4dv = __indirect_glColor4dv;
    indDispatchTab->Color4f = __indirect_glColor4f;
    indDispatchTab->Color4fv = __indirect_glColor4fv;
    indDispatchTab->Color4i = __indirect_glColor4i;
    indDispatchTab->Color4iv = __indirect_glColor4iv;
    indDispatchTab->Color4s = __indirect_glColor4s;
    indDispatchTab->Color4sv = __indirect_glColor4sv;
    indDispatchTab->Color4ub = __indirect_glColor4ub;
    indDispatchTab->Color4ubv = __indirect_glColor4ubv;
    indDispatchTab->Color4ui = __indirect_glColor4ui;
    indDispatchTab->Color4uiv = __indirect_glColor4uiv;
    indDispatchTab->Color4us = __indirect_glColor4us;
    indDispatchTab->Color4usv = __indirect_glColor4usv;
    indDispatchTab->ColorMask = __indirect_glColorMask;
    indDispatchTab->ColorMaterial = __indirect_glColorMaterial;
    indDispatchTab->ColorPointer = __indirect_glColorPointer;
    indDispatchTab->CopyPixels = __indirect_glCopyPixels;
    indDispatchTab->CopyTexImage1D = __indirect_glCopyTexImage1D;
    indDispatchTab->CopyTexImage2D = __indirect_glCopyTexImage2D;
    indDispatchTab->CopyTexSubImage1D = __indirect_glCopyTexSubImage1D;
    indDispatchTab->CopyTexSubImage2D = __indirect_glCopyTexSubImage2D;
    indDispatchTab->CullFace = __indirect_glCullFace;
    indDispatchTab->DeleteLists = __indirect_glDeleteLists;
    indDispatchTab->DeleteTextures = __indirect_glDeleteTextures;
    indDispatchTab->DepthFunc = __indirect_glDepthFunc;
    indDispatchTab->DepthMask = __indirect_glDepthMask;
    indDispatchTab->DepthRange = __indirect_glDepthRange;
    indDispatchTab->Disable = __indirect_glDisable;
    indDispatchTab->DisableClientState = __indirect_glDisableClientState;
    indDispatchTab->DrawArrays = __indirect_glDrawArrays;
    indDispatchTab->DrawBuffer = __indirect_glDrawBuffer;
    indDispatchTab->DrawElements = __indirect_glDrawElements;
    indDispatchTab->DrawPixels = __indirect_glDrawPixels;
    indDispatchTab->DrawRangeElements = __indirect_glDrawRangeElements;
    indDispatchTab->EdgeFlag = __indirect_glEdgeFlag;
    indDispatchTab->EdgeFlagPointer = __indirect_glEdgeFlagPointer;
    indDispatchTab->EdgeFlagv = __indirect_glEdgeFlagv;
    indDispatchTab->Enable = __indirect_glEnable;
    indDispatchTab->EnableClientState = __indirect_glEnableClientState;
    indDispatchTab->End = __indirect_glEnd;
    indDispatchTab->EndList = __indirect_glEndList;
    indDispatchTab->EvalCoord1d = __indirect_glEvalCoord1d;
    indDispatchTab->EvalCoord1dv = __indirect_glEvalCoord1dv;
    indDispatchTab->EvalCoord1f = __indirect_glEvalCoord1f;
    indDispatchTab->EvalCoord1fv = __indirect_glEvalCoord1fv;
    indDispatchTab->EvalCoord2d = __indirect_glEvalCoord2d;
    indDispatchTab->EvalCoord2dv = __indirect_glEvalCoord2dv;
    indDispatchTab->EvalCoord2f = __indirect_glEvalCoord2f;
    indDispatchTab->EvalCoord2fv = __indirect_glEvalCoord2fv;
    indDispatchTab->EvalMesh1 = __indirect_glEvalMesh1;
    indDispatchTab->EvalMesh2 = __indirect_glEvalMesh2;
    indDispatchTab->EvalPoint1 = __indirect_glEvalPoint1;
    indDispatchTab->EvalPoint2 = __indirect_glEvalPoint2;
    indDispatchTab->FeedbackBuffer = __indirect_glFeedbackBuffer;
    indDispatchTab->Finish = __indirect_glFinish;
    indDispatchTab->Flush = __indirect_glFlush;
    indDispatchTab->Fogf = __indirect_glFogf;
    indDispatchTab->Fogfv = __indirect_glFogfv;
    indDispatchTab->Fogi = __indirect_glFogi;
    indDispatchTab->Fogiv = __indirect_glFogiv;
    indDispatchTab->FrontFace = __indirect_glFrontFace;
    indDispatchTab->Frustum = __indirect_glFrustum;
    indDispatchTab->GenLists = __indirect_glGenLists;
    indDispatchTab->GenTextures = __indirect_glGenTextures;
    indDispatchTab->GetBooleanv = __indirect_glGetBooleanv;
    indDispatchTab->GetClipPlane = __indirect_glGetClipPlane;
    indDispatchTab->GetDoublev = __indirect_glGetDoublev;
    indDispatchTab->GetError = __indirect_glGetError;
    indDispatchTab->GetFloatv = __indirect_glGetFloatv;
    indDispatchTab->GetIntegerv = __indirect_glGetIntegerv;
    indDispatchTab->GetLightfv = __indirect_glGetLightfv;
    indDispatchTab->GetLightiv = __indirect_glGetLightiv;
    indDispatchTab->GetMapdv = __indirect_glGetMapdv;
    indDispatchTab->GetMapfv = __indirect_glGetMapfv;
    indDispatchTab->GetMapiv = __indirect_glGetMapiv;
    indDispatchTab->GetMaterialfv = __indirect_glGetMaterialfv;
    indDispatchTab->GetMaterialiv = __indirect_glGetMaterialiv;
    indDispatchTab->GetPixelMapfv = __indirect_glGetPixelMapfv;
    indDispatchTab->GetPixelMapuiv = __indirect_glGetPixelMapuiv;
    indDispatchTab->GetPixelMapusv = __indirect_glGetPixelMapusv;
    indDispatchTab->GetPointerv = __indirect_glGetPointerv;
    indDispatchTab->GetPolygonStipple = __indirect_glGetPolygonStipple;
    indDispatchTab->GetString = __indirect_glGetString;
    indDispatchTab->GetTexEnvfv = __indirect_glGetTexEnvfv;
    indDispatchTab->GetTexEnviv = __indirect_glGetTexEnviv;
    indDispatchTab->GetTexGendv = __indirect_glGetTexGendv;
    indDispatchTab->GetTexGenfv = __indirect_glGetTexGenfv;
    indDispatchTab->GetTexGeniv = __indirect_glGetTexGeniv;
    indDispatchTab->GetTexImage = __indirect_glGetTexImage;
    indDispatchTab->GetTexLevelParameterfv = __indirect_glGetTexLevelParameterfv;
    indDispatchTab->GetTexLevelParameteriv = __indirect_glGetTexLevelParameteriv;
    indDispatchTab->GetTexParameterfv = __indirect_glGetTexParameterfv;
    indDispatchTab->GetTexParameteriv = __indirect_glGetTexParameteriv;
    indDispatchTab->Hint = __indirect_glHint;
    indDispatchTab->IndexMask = __indirect_glIndexMask;
    indDispatchTab->IndexPointer = __indirect_glIndexPointer;
    indDispatchTab->Indexd = __indirect_glIndexd;
    indDispatchTab->Indexdv = __indirect_glIndexdv;
    indDispatchTab->Indexf = __indirect_glIndexf;
    indDispatchTab->Indexfv = __indirect_glIndexfv;
    indDispatchTab->Indexi = __indirect_glIndexi;
    indDispatchTab->Indexiv = __indirect_glIndexiv;
    indDispatchTab->Indexs = __indirect_glIndexs;
    indDispatchTab->Indexsv = __indirect_glIndexsv;
    indDispatchTab->Indexub = __indirect_glIndexub;
    indDispatchTab->Indexubv = __indirect_glIndexubv;
    indDispatchTab->InitNames = __indirect_glInitNames;
    indDispatchTab->InterleavedArrays = __indirect_glInterleavedArrays;
    indDispatchTab->IsEnabled = __indirect_glIsEnabled;
    indDispatchTab->IsList = __indirect_glIsList;
    indDispatchTab->IsTexture = __indirect_glIsTexture;
    indDispatchTab->LightModelf = __indirect_glLightModelf;
    indDispatchTab->LightModelfv = __indirect_glLightModelfv;
    indDispatchTab->LightModeli = __indirect_glLightModeli;
    indDispatchTab->LightModeliv = __indirect_glLightModeliv;
    indDispatchTab->Lightf = __indirect_glLightf;
    indDispatchTab->Lightfv = __indirect_glLightfv;
    indDispatchTab->Lighti = __indirect_glLighti;
    indDispatchTab->Lightiv = __indirect_glLightiv;
    indDispatchTab->LineStipple = __indirect_glLineStipple;
    indDispatchTab->LineWidth = __indirect_glLineWidth;
    indDispatchTab->ListBase = __indirect_glListBase;
    indDispatchTab->LoadIdentity = __indirect_glLoadIdentity;
    indDispatchTab->LoadMatrixd = __indirect_glLoadMatrixd;
    indDispatchTab->LoadMatrixf = __indirect_glLoadMatrixf;
    indDispatchTab->LoadName = __indirect_glLoadName;
    indDispatchTab->LogicOp = __indirect_glLogicOp;
    indDispatchTab->Map1d = __indirect_glMap1d;
    indDispatchTab->Map1f = __indirect_glMap1f;
    indDispatchTab->Map2d = __indirect_glMap2d;
    indDispatchTab->Map2f = __indirect_glMap2f;
    indDispatchTab->MapGrid1d = __indirect_glMapGrid1d;
    indDispatchTab->MapGrid1f = __indirect_glMapGrid1f;
    indDispatchTab->MapGrid2d = __indirect_glMapGrid2d;
    indDispatchTab->MapGrid2f = __indirect_glMapGrid2f;
    indDispatchTab->Materialf = __indirect_glMaterialf;
    indDispatchTab->Materialfv = __indirect_glMaterialfv;
    indDispatchTab->Materiali = __indirect_glMateriali;
    indDispatchTab->Materialiv = __indirect_glMaterialiv;
    indDispatchTab->MatrixMode = __indirect_glMatrixMode;
    indDispatchTab->MultMatrixd = __indirect_glMultMatrixd;
    indDispatchTab->MultMatrixf = __indirect_glMultMatrixf;
    indDispatchTab->NewList = __indirect_glNewList;
    indDispatchTab->Normal3b = __indirect_glNormal3b;
    indDispatchTab->Normal3bv = __indirect_glNormal3bv;
    indDispatchTab->Normal3d = __indirect_glNormal3d;
    indDispatchTab->Normal3dv = __indirect_glNormal3dv;
    indDispatchTab->Normal3f = __indirect_glNormal3f;
    indDispatchTab->Normal3fv = __indirect_glNormal3fv;
    indDispatchTab->Normal3i = __indirect_glNormal3i;
    indDispatchTab->Normal3iv = __indirect_glNormal3iv;
    indDispatchTab->Normal3s = __indirect_glNormal3s;
    indDispatchTab->Normal3sv = __indirect_glNormal3sv;
    indDispatchTab->NormalPointer = __indirect_glNormalPointer;
    indDispatchTab->Ortho = __indirect_glOrtho;
    indDispatchTab->PassThrough = __indirect_glPassThrough;
    indDispatchTab->PixelMapfv = __indirect_glPixelMapfv;
    indDispatchTab->PixelMapuiv = __indirect_glPixelMapuiv;
    indDispatchTab->PixelMapusv = __indirect_glPixelMapusv;
    indDispatchTab->PixelStoref = __indirect_glPixelStoref;
    indDispatchTab->PixelStorei = __indirect_glPixelStorei;
    indDispatchTab->PixelTransferf = __indirect_glPixelTransferf;
    indDispatchTab->PixelTransferi = __indirect_glPixelTransferi;
    indDispatchTab->PixelZoom = __indirect_glPixelZoom;
    indDispatchTab->PointSize = __indirect_glPointSize;
    indDispatchTab->PolygonMode = __indirect_glPolygonMode;
    indDispatchTab->PolygonOffset = __indirect_glPolygonOffset;
    indDispatchTab->PolygonStipple = __indirect_glPolygonStipple;
    indDispatchTab->PopAttrib = __indirect_glPopAttrib;
    indDispatchTab->PopClientAttrib = __indirect_glPopClientAttrib;
    indDispatchTab->PopMatrix = __indirect_glPopMatrix;
    indDispatchTab->PopName = __indirect_glPopName;
    indDispatchTab->PrioritizeTextures = __indirect_glPrioritizeTextures;
    indDispatchTab->PushAttrib = __indirect_glPushAttrib;
    indDispatchTab->PushClientAttrib = __indirect_glPushClientAttrib;
    indDispatchTab->PushMatrix = __indirect_glPushMatrix;
    indDispatchTab->PushName = __indirect_glPushName;
    indDispatchTab->RasterPos2d = __indirect_glRasterPos2d;
    indDispatchTab->RasterPos2dv = __indirect_glRasterPos2dv;
    indDispatchTab->RasterPos2f = __indirect_glRasterPos2f;
    indDispatchTab->RasterPos2fv = __indirect_glRasterPos2fv;
    indDispatchTab->RasterPos2i = __indirect_glRasterPos2i;
    indDispatchTab->RasterPos2iv = __indirect_glRasterPos2iv;
    indDispatchTab->RasterPos2s = __indirect_glRasterPos2s;
    indDispatchTab->RasterPos2sv = __indirect_glRasterPos2sv;
    indDispatchTab->RasterPos3d = __indirect_glRasterPos3d;
    indDispatchTab->RasterPos3dv = __indirect_glRasterPos3dv;
    indDispatchTab->RasterPos3f = __indirect_glRasterPos3f;
    indDispatchTab->RasterPos3fv = __indirect_glRasterPos3fv;
    indDispatchTab->RasterPos3i = __indirect_glRasterPos3i;
    indDispatchTab->RasterPos3iv = __indirect_glRasterPos3iv;
    indDispatchTab->RasterPos3s = __indirect_glRasterPos3s;
    indDispatchTab->RasterPos3sv = __indirect_glRasterPos3sv;
    indDispatchTab->RasterPos4d = __indirect_glRasterPos4d;
    indDispatchTab->RasterPos4dv = __indirect_glRasterPos4dv;
    indDispatchTab->RasterPos4f = __indirect_glRasterPos4f;
    indDispatchTab->RasterPos4fv = __indirect_glRasterPos4fv;
    indDispatchTab->RasterPos4i = __indirect_glRasterPos4i;
    indDispatchTab->RasterPos4iv = __indirect_glRasterPos4iv;
    indDispatchTab->RasterPos4s = __indirect_glRasterPos4s;
    indDispatchTab->RasterPos4sv = __indirect_glRasterPos4sv;
    indDispatchTab->ReadBuffer = __indirect_glReadBuffer;
    indDispatchTab->ReadPixels = __indirect_glReadPixels;
    indDispatchTab->Rectd = __indirect_glRectd;
    indDispatchTab->Rectdv = __indirect_glRectdv;
    indDispatchTab->Rectf = __indirect_glRectf;
    indDispatchTab->Rectfv = __indirect_glRectfv;
    indDispatchTab->Recti = __indirect_glRecti;
    indDispatchTab->Rectiv = __indirect_glRectiv;
    indDispatchTab->Rects = __indirect_glRects;
    indDispatchTab->Rectsv = __indirect_glRectsv;
    indDispatchTab->RenderMode = __indirect_glRenderMode;
    indDispatchTab->Rotated = __indirect_glRotated;
    indDispatchTab->Rotatef = __indirect_glRotatef;
    indDispatchTab->Scaled = __indirect_glScaled;
    indDispatchTab->Scalef = __indirect_glScalef;
    indDispatchTab->Scissor = __indirect_glScissor;
    indDispatchTab->SelectBuffer = __indirect_glSelectBuffer;
    indDispatchTab->ShadeModel = __indirect_glShadeModel;
    indDispatchTab->StencilFunc = __indirect_glStencilFunc;
    indDispatchTab->StencilMask = __indirect_glStencilMask;
    indDispatchTab->StencilOp = __indirect_glStencilOp;
    indDispatchTab->TexCoord1d = __indirect_glTexCoord1d;
    indDispatchTab->TexCoord1dv = __indirect_glTexCoord1dv;
    indDispatchTab->TexCoord1f = __indirect_glTexCoord1f;
    indDispatchTab->TexCoord1fv = __indirect_glTexCoord1fv;
    indDispatchTab->TexCoord1i = __indirect_glTexCoord1i;
    indDispatchTab->TexCoord1iv = __indirect_glTexCoord1iv;
    indDispatchTab->TexCoord1s = __indirect_glTexCoord1s;
    indDispatchTab->TexCoord1sv = __indirect_glTexCoord1sv;
    indDispatchTab->TexCoord2d = __indirect_glTexCoord2d;
    indDispatchTab->TexCoord2dv = __indirect_glTexCoord2dv;
    indDispatchTab->TexCoord2f = __indirect_glTexCoord2f;
    indDispatchTab->TexCoord2fv = __indirect_glTexCoord2fv;
    indDispatchTab->TexCoord2i = __indirect_glTexCoord2i;
    indDispatchTab->TexCoord2iv = __indirect_glTexCoord2iv;
    indDispatchTab->TexCoord2s = __indirect_glTexCoord2s;
    indDispatchTab->TexCoord2sv = __indirect_glTexCoord2sv;
    indDispatchTab->TexCoord3d = __indirect_glTexCoord3d;
    indDispatchTab->TexCoord3dv = __indirect_glTexCoord3dv;
    indDispatchTab->TexCoord3f = __indirect_glTexCoord3f;
    indDispatchTab->TexCoord3fv = __indirect_glTexCoord3fv;
    indDispatchTab->TexCoord3i = __indirect_glTexCoord3i;
    indDispatchTab->TexCoord3iv = __indirect_glTexCoord3iv;
    indDispatchTab->TexCoord3s = __indirect_glTexCoord3s;
    indDispatchTab->TexCoord3sv = __indirect_glTexCoord3sv;
    indDispatchTab->TexCoord4d = __indirect_glTexCoord4d;
    indDispatchTab->TexCoord4dv = __indirect_glTexCoord4dv;
    indDispatchTab->TexCoord4f = __indirect_glTexCoord4f;
    indDispatchTab->TexCoord4fv = __indirect_glTexCoord4fv;
    indDispatchTab->TexCoord4i = __indirect_glTexCoord4i;
    indDispatchTab->TexCoord4iv = __indirect_glTexCoord4iv;
    indDispatchTab->TexCoord4s = __indirect_glTexCoord4s;
    indDispatchTab->TexCoord4sv = __indirect_glTexCoord4sv;
    indDispatchTab->TexCoordPointer = __indirect_glTexCoordPointer;
    indDispatchTab->TexEnvf = __indirect_glTexEnvf;
    indDispatchTab->TexEnvfv = __indirect_glTexEnvfv;
    indDispatchTab->TexEnvi = __indirect_glTexEnvi;
    indDispatchTab->TexEnviv = __indirect_glTexEnviv;
    indDispatchTab->TexGend = __indirect_glTexGend;
    indDispatchTab->TexGendv = __indirect_glTexGendv;
    indDispatchTab->TexGenf = __indirect_glTexGenf;
    indDispatchTab->TexGenfv = __indirect_glTexGenfv;
    indDispatchTab->TexGeni = __indirect_glTexGeni;
    indDispatchTab->TexGeniv = __indirect_glTexGeniv;
    indDispatchTab->TexImage1D = __indirect_glTexImage1D;
    indDispatchTab->TexImage2D = __indirect_glTexImage2D;
    indDispatchTab->TexParameterf = __indirect_glTexParameterf;
    indDispatchTab->TexParameterfv = __indirect_glTexParameterfv;
    indDispatchTab->TexParameteri = __indirect_glTexParameteri;
    indDispatchTab->TexParameteriv = __indirect_glTexParameteriv;
    indDispatchTab->TexSubImage1D = __indirect_glTexSubImage1D;
    indDispatchTab->TexSubImage2D = __indirect_glTexSubImage2D;
    indDispatchTab->Translated = __indirect_glTranslated;
    indDispatchTab->Translatef = __indirect_glTranslatef;
    indDispatchTab->Vertex2d = __indirect_glVertex2d;
    indDispatchTab->Vertex2dv = __indirect_glVertex2dv;
    indDispatchTab->Vertex2f = __indirect_glVertex2f;
    indDispatchTab->Vertex2fv = __indirect_glVertex2fv;
    indDispatchTab->Vertex2i = __indirect_glVertex2i;
    indDispatchTab->Vertex2iv = __indirect_glVertex2iv;
    indDispatchTab->Vertex2s = __indirect_glVertex2s;
    indDispatchTab->Vertex2sv = __indirect_glVertex2sv;
    indDispatchTab->Vertex3d = __indirect_glVertex3d;
    indDispatchTab->Vertex3dv = __indirect_glVertex3dv;
    indDispatchTab->Vertex3f = __indirect_glVertex3f;
    indDispatchTab->Vertex3fv = __indirect_glVertex3fv;
    indDispatchTab->Vertex3i = __indirect_glVertex3i;
    indDispatchTab->Vertex3iv = __indirect_glVertex3iv;
    indDispatchTab->Vertex3s = __indirect_glVertex3s;
    indDispatchTab->Vertex3sv = __indirect_glVertex3sv;
    indDispatchTab->Vertex4d = __indirect_glVertex4d;
    indDispatchTab->Vertex4dv = __indirect_glVertex4dv;
    indDispatchTab->Vertex4f = __indirect_glVertex4f;
    indDispatchTab->Vertex4fv = __indirect_glVertex4fv;
    indDispatchTab->Vertex4i = __indirect_glVertex4i;
    indDispatchTab->Vertex4iv = __indirect_glVertex4iv;
    indDispatchTab->Vertex4s = __indirect_glVertex4s;
    indDispatchTab->Vertex4sv = __indirect_glVertex4sv;
    indDispatchTab->VertexPointer = __indirect_glVertexPointer;
    indDispatchTab->Viewport = __indirect_glViewport;

    /* 1.2 */
    indDispatchTab->CopyTexSubImage3D = __indirect_glCopyTexSubImage3D;
    indDispatchTab->DrawRangeElements = __indirect_glDrawRangeElements;
    indDispatchTab->TexImage3D = __indirect_glTexImage3D;
    indDispatchTab->TexSubImage3D = __indirect_glTexSubImage3D;

    /* OpenGL 1.2  GL_ARB_imaging */
    indDispatchTab->BlendColor = __indirect_glBlendColor;
    indDispatchTab->BlendEquation = __indirect_glBlendEquation;
    indDispatchTab->ColorSubTable = __indirect_glColorSubTable;
    indDispatchTab->ColorTable = __indirect_glColorTable;
    indDispatchTab->ColorTableParameterfv = __indirect_glColorTableParameterfv;
    indDispatchTab->ColorTableParameteriv = __indirect_glColorTableParameteriv;
    indDispatchTab->ConvolutionFilter1D = __indirect_glConvolutionFilter1D;
    indDispatchTab->ConvolutionFilter2D = __indirect_glConvolutionFilter2D;
    indDispatchTab->ConvolutionParameterf = __indirect_glConvolutionParameterf;
    indDispatchTab->ConvolutionParameterfv = __indirect_glConvolutionParameterfv;
    indDispatchTab->ConvolutionParameteri = __indirect_glConvolutionParameteri;
    indDispatchTab->ConvolutionParameteriv = __indirect_glConvolutionParameteriv;
    indDispatchTab->CopyColorSubTable = __indirect_glCopyColorSubTable;
    indDispatchTab->CopyColorTable = __indirect_glCopyColorTable;
    indDispatchTab->CopyConvolutionFilter1D = __indirect_glCopyConvolutionFilter1D;
    indDispatchTab->CopyConvolutionFilter2D = __indirect_glCopyConvolutionFilter2D;
    indDispatchTab->GetColorTable = __indirect_glGetColorTable;
    indDispatchTab->GetColorTableParameterfv = __indirect_glGetColorTableParameterfv;
    indDispatchTab->GetColorTableParameteriv = __indirect_glGetColorTableParameteriv;
    indDispatchTab->GetConvolutionFilter = __indirect_glGetConvolutionFilter;
    indDispatchTab->GetConvolutionParameterfv = __indirect_glGetConvolutionParameterfv;
    indDispatchTab->GetConvolutionParameteriv = __indirect_glGetConvolutionParameteriv;
    indDispatchTab->GetHistogram = __indirect_glGetHistogram;
    indDispatchTab->GetHistogramParameterfv = __indirect_glGetHistogramParameterfv;
    indDispatchTab->GetHistogramParameteriv = __indirect_glGetHistogramParameteriv;
    indDispatchTab->GetMinmax = __indirect_glGetMinmax;
    indDispatchTab->GetMinmaxParameterfv = __indirect_glGetMinmaxParameterfv;
    indDispatchTab->GetMinmaxParameteriv = __indirect_glGetMinmaxParameteriv;
    indDispatchTab->GetSeparableFilter = __indirect_glGetSeparableFilter;
    indDispatchTab->Histogram = __indirect_glHistogram;
    indDispatchTab->Minmax = __indirect_glMinmax;
    indDispatchTab->ResetHistogram = __indirect_glResetHistogram;
    indDispatchTab->ResetMinmax = __indirect_glResetMinmax;
    indDispatchTab->SeparableFilter2D = __indirect_glSeparableFilter2D;

    return indDispatchTab;
}
