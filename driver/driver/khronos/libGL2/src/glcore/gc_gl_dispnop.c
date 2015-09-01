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


#include "gltypes.h"
#include "g_disp.h"


GLint APIENTRY __glnop_Nop0(GLvoid)
{
    return 0;
}

GLint APIENTRY __glnop_Nop1(GLint i1)
{
    return 0;
}

GLint APIENTRY __glnop_Nop2(GLint i1, GLint i2)
{
    return 0;
}

GLint APIENTRY __glnop_Nop3(GLint i1, GLint i2, GLint i3)
{
    return 0;
}

GLint APIENTRY __glnop_Nop4(GLint i1, GLint i2, GLint i3, GLint i4)
{
    return 0;
}

GLint APIENTRY __glnop_Nop5(GLint i1, GLint i2, GLint i3, GLint i4, GLint i5)
{
    return 0;
}

GLint APIENTRY __glnop_Nop6(GLint i1, GLint i2, GLint i3, GLint i4, GLint i5, GLint i6)
{
    return 0;
}

GLint APIENTRY __glnop_Nop7(GLint i1, GLint i2, GLint i3, GLint i4, GLint i5, GLint i6, GLint i7)
{
    return 0;
}

GLint APIENTRY __glnop_Nop8(GLint i1, GLint i2, GLint i3, GLint i4, GLint i5, GLint i6, GLint i7, GLint i8)
{
    return 0;
}

GLint APIENTRY __glnop_Nop9(GLint i1, GLint i2, GLint i3, GLint i4, GLint i5, GLint i6, GLint i7, GLint i8, GLint i9)
{
    return 0;
}

GLint APIENTRY __glnop_Nop10(GLint i1, GLint i2, GLint i3, GLint i4, GLint i5, GLint i6, GLint i7, GLint i8, GLint i9, GLint i10)
{
    return 0;
}

GLint APIENTRY __glnop_Nop11(GLint i1, GLint i2, GLint i3, GLint i4, GLint i5, GLint i6, GLint i7, GLint i8, GLint i9, GLint i10, GLint i11)
{
    return 0;
}

GLint APIENTRY __glnop_Nop12(GLint i1, GLint i2, GLint i3, GLint i4, GLint i5, GLint i6, GLint i7, GLint i8, GLint i9, GLint i10, GLint i11, GLint i12)
{
    return 0;
}

GLint APIENTRY __glnop_Nop13(GLint i1, GLint i2, GLint i3, GLint i4, GLint i5, GLint i6, GLint i7, GLint i8, GLint i9, GLint i10, GLint i11, GLint i12, GLint i13)
{
    return 0;
}

GLint APIENTRY __glnop_Nop14(GLint i1, GLint i2, GLint i3, GLint i4, GLint i5, GLint i6, GLint i7, GLint i8, GLint i9, GLint i10, GLint i11, GLint i12, GLint i13, GLint i14)
{
    return 0;
}

__GLdispatchState __glNopDispatchFuncTable = {
    /*
    ** number of entries
    */
    OPENGL_VERSION_110_ENTRIES,

    /*
    ** dispatch
    */
    {
    (GLvoid *) __glnop_Nop2,        /* NewList */
    (GLvoid *) __glnop_Nop0,        /* EndList */
    (GLvoid *) __glnop_Nop1,        /* CallList */
    (GLvoid *) __glnop_Nop3,        /* CallLists */
    (GLvoid *) __glnop_Nop2,        /* DeleteLists */
    (GLvoid *) __glnop_Nop1,        /* GenLists */
    (GLvoid *) __glnop_Nop1,        /* ListBase */
    (GLvoid *) __glnop_Nop1,        /* Begin */
    (GLvoid *) __glnop_Nop7,        /* Bitmap */
    (GLvoid *) __glnop_Nop3,        /* Color3b */
    (GLvoid *) __glnop_Nop1,        /* Color3bv */
    (GLvoid *) __glnop_Nop6,        /* Color3d */
    (GLvoid *) __glnop_Nop1,        /* Color3dv */
    (GLvoid *) __glnop_Nop3,        /* Color3f */
    (GLvoid *) __glnop_Nop1,        /* Color3fv */
    (GLvoid *) __glnop_Nop3,        /* Color3i */
    (GLvoid *) __glnop_Nop1,        /* Color3iv */
    (GLvoid *) __glnop_Nop3,        /* Color3s */
    (GLvoid *) __glnop_Nop1,        /* Color3sv */
    (GLvoid *) __glnop_Nop3,        /* Color3ub */
    (GLvoid *) __glnop_Nop1,        /* Color3ubv */
    (GLvoid *) __glnop_Nop3,        /* Color3ui */
    (GLvoid *) __glnop_Nop1,        /* Color3uiv */
    (GLvoid *) __glnop_Nop3,        /* Color3us */
    (GLvoid *) __glnop_Nop1,        /* Color3usv */
    (GLvoid *) __glnop_Nop4,        /* Color4b */
    (GLvoid *) __glnop_Nop1,        /* Color4bv */
    (GLvoid *) __glnop_Nop8,        /* Color4d */
    (GLvoid *) __glnop_Nop1,        /* Color4dv */
    (GLvoid *) __glnop_Nop4,        /* Color4f */
    (GLvoid *) __glnop_Nop1,        /* Color4fv */
    (GLvoid *) __glnop_Nop4,        /* Color4i */
    (GLvoid *) __glnop_Nop1,        /* Color4iv */
    (GLvoid *) __glnop_Nop4,        /* Color4s */
    (GLvoid *) __glnop_Nop1,        /* Color4sv */
    (GLvoid *) __glnop_Nop4,        /* Color4ub */
    (GLvoid *) __glnop_Nop1,        /* Color4ubv */
    (GLvoid *) __glnop_Nop4,        /* Color4ui */
    (GLvoid *) __glnop_Nop1,        /* Color4uiv */
    (GLvoid *) __glnop_Nop4,        /* Color4us */
    (GLvoid *) __glnop_Nop1,        /* Color4usv */
    (GLvoid *) __glnop_Nop1,        /* EdgeFlag */
    (GLvoid *) __glnop_Nop1,        /* EdgeFlagv */
    (GLvoid *) __glnop_Nop0,        /* End */
    (GLvoid *) __glnop_Nop2,        /* Indexd */
    (GLvoid *) __glnop_Nop1,        /* Indexdv */
    (GLvoid *) __glnop_Nop1,        /* Indexf */
    (GLvoid *) __glnop_Nop1,        /* Indexfv */
    (GLvoid *) __glnop_Nop1,        /* Indexi */
    (GLvoid *) __glnop_Nop1,        /* Indexiv */
    (GLvoid *) __glnop_Nop1,        /* Indexs */
    (GLvoid *) __glnop_Nop1,        /* Indexsv */
    (GLvoid *) __glnop_Nop3,        /* Normal3b */
    (GLvoid *) __glnop_Nop1,        /* Normal3bv */
    (GLvoid *) __glnop_Nop6,        /* Normal3d */
    (GLvoid *) __glnop_Nop1,        /* Normal3dv */
    (GLvoid *) __glnop_Nop3,        /* Normal3f */
    (GLvoid *) __glnop_Nop1,        /* Normal3fv */
    (GLvoid *) __glnop_Nop3,        /* Normal3i */
    (GLvoid *) __glnop_Nop1,        /* Normal3iv */
    (GLvoid *) __glnop_Nop3,        /* Normal3s */
    (GLvoid *) __glnop_Nop1,        /* Normal3sv */
    (GLvoid *) __glnop_Nop4,        /* RasterPos2d */
    (GLvoid *) __glnop_Nop1,        /* RasterPos2dv */
    (GLvoid *) __glnop_Nop2,        /* RasterPos2f */
    (GLvoid *) __glnop_Nop1,        /* RasterPos2fv */
    (GLvoid *) __glnop_Nop2,        /* RasterPos2i */
    (GLvoid *) __glnop_Nop1,        /* RasterPos2iv */
    (GLvoid *) __glnop_Nop2,        /* RasterPos2s */
    (GLvoid *) __glnop_Nop1,        /* RasterPos2sv */
    (GLvoid *) __glnop_Nop6,        /* RasterPos3d */
    (GLvoid *) __glnop_Nop1,        /* RasterPos3dv */
    (GLvoid *) __glnop_Nop3,        /* RasterPos3f */
    (GLvoid *) __glnop_Nop1,        /* RasterPos3fv */
    (GLvoid *) __glnop_Nop3,        /* RasterPos3i */
    (GLvoid *) __glnop_Nop1,        /* RasterPos3iv */
    (GLvoid *) __glnop_Nop3,        /* RasterPos3s */
    (GLvoid *) __glnop_Nop1,        /* RasterPos3sv */
    (GLvoid *) __glnop_Nop8,        /* RasterPos4d */
    (GLvoid *) __glnop_Nop1,        /* RasterPos4dv */
    (GLvoid *) __glnop_Nop4,        /* RasterPos4f */
    (GLvoid *) __glnop_Nop1,        /* RasterPos4fv */
    (GLvoid *) __glnop_Nop4,        /* RasterPos4i */
    (GLvoid *) __glnop_Nop1,        /* RasterPos4iv */
    (GLvoid *) __glnop_Nop4,        /* RasterPos4s */
    (GLvoid *) __glnop_Nop1,        /* RasterPos4sv */
    (GLvoid *) __glnop_Nop8,        /* Rectd */
    (GLvoid *) __glnop_Nop2,        /* Rectdv */
    (GLvoid *) __glnop_Nop4,        /* Rectf */
    (GLvoid *) __glnop_Nop2,        /* Rectfv */
    (GLvoid *) __glnop_Nop4,        /* Recti */
    (GLvoid *) __glnop_Nop2,        /* Rectiv */
    (GLvoid *) __glnop_Nop4,        /* Rects */
    (GLvoid *) __glnop_Nop2,        /* Rectsv */
    (GLvoid *) __glnop_Nop2,        /* TexCoord1d */
    (GLvoid *) __glnop_Nop1,        /* TexCoord1dv */
    (GLvoid *) __glnop_Nop1,        /* TexCoord1f */
    (GLvoid *) __glnop_Nop1,        /* TexCoord1fv */
    (GLvoid *) __glnop_Nop1,        /* TexCoord1i */
    (GLvoid *) __glnop_Nop1,        /* TexCoord1iv */
    (GLvoid *) __glnop_Nop1,        /* TexCoord1s */
    (GLvoid *) __glnop_Nop1,        /* TexCoord1sv */
    (GLvoid *) __glnop_Nop4,        /* TexCoord2d */
    (GLvoid *) __glnop_Nop1,        /* TexCoord2dv */
    (GLvoid *) __glnop_Nop2,        /* TexCoord2f */
    (GLvoid *) __glnop_Nop1,        /* TexCoord2fv */
    (GLvoid *) __glnop_Nop2,        /* TexCoord2i */
    (GLvoid *) __glnop_Nop1,        /* TexCoord2iv */
    (GLvoid *) __glnop_Nop2,        /* TexCoord2s */
    (GLvoid *) __glnop_Nop1,        /* TexCoord2sv */
    (GLvoid *) __glnop_Nop6,        /* TexCoord3d */
    (GLvoid *) __glnop_Nop1,        /* TexCoord3dv */
    (GLvoid *) __glnop_Nop3,        /* TexCoord3f */
    (GLvoid *) __glnop_Nop1,        /* TexCoord3fv */
    (GLvoid *) __glnop_Nop3,        /* TexCoord3i */
    (GLvoid *) __glnop_Nop1,        /* TexCoord3iv */
    (GLvoid *) __glnop_Nop3,        /* TexCoord3s */
    (GLvoid *) __glnop_Nop1,        /* TexCoord3sv */
    (GLvoid *) __glnop_Nop8,        /* TexCoord4d */
    (GLvoid *) __glnop_Nop1,        /* TexCoord4dv */
    (GLvoid *) __glnop_Nop4,        /* TexCoord4f */
    (GLvoid *) __glnop_Nop1,        /* TexCoord4fv */
    (GLvoid *) __glnop_Nop4,        /* TexCoord4i */
    (GLvoid *) __glnop_Nop1,        /* TexCoord4iv */
    (GLvoid *) __glnop_Nop4,        /* TexCoord4s */
    (GLvoid *) __glnop_Nop1,        /* TexCoord4sv */
    (GLvoid *) __glnop_Nop4,        /* Vertex2d */
    (GLvoid *) __glnop_Nop1,        /* Vertex2dv */
    (GLvoid *) __glnop_Nop2,        /* Vertex2f */
    (GLvoid *) __glnop_Nop1,        /* Vertex2fv */
    (GLvoid *) __glnop_Nop2,        /* Vertex2i */
    (GLvoid *) __glnop_Nop1,        /* Vertex2iv */
    (GLvoid *) __glnop_Nop2,        /* Vertex2s */
    (GLvoid *) __glnop_Nop1,        /* Vertex2sv */
    (GLvoid *) __glnop_Nop6,        /* Vertex3d */
    (GLvoid *) __glnop_Nop1,        /* Vertex3dv */
    (GLvoid *) __glnop_Nop3,        /* Vertex3f */
    (GLvoid *) __glnop_Nop1,        /* Vertex3fv */
    (GLvoid *) __glnop_Nop3,        /* Vertex3i */
    (GLvoid *) __glnop_Nop1,        /* Vertex3iv */
    (GLvoid *) __glnop_Nop3,        /* Vertex3s */
    (GLvoid *) __glnop_Nop1,        /* Vertex3sv */
    (GLvoid *) __glnop_Nop8,        /* Vertex4d */
    (GLvoid *) __glnop_Nop1,        /* Vertex4dv */
    (GLvoid *) __glnop_Nop4,        /* Vertex4f */
    (GLvoid *) __glnop_Nop1,        /* Vertex4fv */
    (GLvoid *) __glnop_Nop4,        /* Vertex4i */
    (GLvoid *) __glnop_Nop1,        /* Vertex4iv */
    (GLvoid *) __glnop_Nop4,        /* Vertex4s */
    (GLvoid *) __glnop_Nop1,        /* Vertex4sv */
    (GLvoid *) __glnop_Nop2,        /* ClipPlane */
    (GLvoid *) __glnop_Nop2,        /* ColorMaterial */
    (GLvoid *) __glnop_Nop1,        /* CullFace */
    (GLvoid *) __glnop_Nop2,        /* Fogf */
    (GLvoid *) __glnop_Nop2,        /* Fogfv */
    (GLvoid *) __glnop_Nop2,        /* Fogi */
    (GLvoid *) __glnop_Nop2,        /* Fogiv */
    (GLvoid *) __glnop_Nop1,        /* FrontFace */
    (GLvoid *) __glnop_Nop2,        /* Hint */
    (GLvoid *) __glnop_Nop3,        /* Lightf */
    (GLvoid *) __glnop_Nop3,        /* Lightfv */
    (GLvoid *) __glnop_Nop3,        /* Lighti */
    (GLvoid *) __glnop_Nop3,        /* Lightiv */
    (GLvoid *) __glnop_Nop2,        /* LightModelf */
    (GLvoid *) __glnop_Nop2,        /* LightModelfv */
    (GLvoid *) __glnop_Nop2,        /* LightModeli */
    (GLvoid *) __glnop_Nop2,        /* LightModeliv */
    (GLvoid *) __glnop_Nop2,        /* LineStipple */
    (GLvoid *) __glnop_Nop1,        /* LineWidth */
    (GLvoid *) __glnop_Nop3,        /* Materialf */
    (GLvoid *) __glnop_Nop3,        /* Materialfv */
    (GLvoid *) __glnop_Nop3,        /* Materiali */
    (GLvoid *) __glnop_Nop3,        /* Materialiv */
    (GLvoid *) __glnop_Nop1,        /* PointSize */
    (GLvoid *) __glnop_Nop2,        /* PolygonMode */
    (GLvoid *) __glnop_Nop1,        /* PolygonStipple */
    (GLvoid *) __glnop_Nop4,        /* Scissor */
    (GLvoid *) __glnop_Nop1,        /* ShadeModel */
    (GLvoid *) __glnop_Nop3,        /* TexParameterf */
    (GLvoid *) __glnop_Nop3,        /* TexParameterfv */
    (GLvoid *) __glnop_Nop3,        /* TexParameteri */
    (GLvoid *) __glnop_Nop3,        /* TexParameteriv */
    (GLvoid *) __glnop_Nop8,        /* TexImage1D */
    (GLvoid *) __glnop_Nop9,        /* TexImage2D */
    (GLvoid *) __glnop_Nop3,        /* TexEnvf */
    (GLvoid *) __glnop_Nop3,        /* TexEnvfv */
    (GLvoid *) __glnop_Nop3,        /* TexEnvi */
    (GLvoid *) __glnop_Nop3,        /* TexEnviv */
    (GLvoid *) __glnop_Nop4,        /* TexGend */
    (GLvoid *) __glnop_Nop3,        /* TexGendv */
    (GLvoid *) __glnop_Nop3,        /* TexGenf */
    (GLvoid *) __glnop_Nop3,        /* TexGenfv */
    (GLvoid *) __glnop_Nop3,        /* TexGeni */
    (GLvoid *) __glnop_Nop3,        /* TexGeniv */
    (GLvoid *) __glnop_Nop3,        /* FeedbackBuffer */
    (GLvoid *) __glnop_Nop2,        /* SelectBuffer */
    (GLvoid *) __glnop_Nop1,        /* RenderMode */
    (GLvoid *) __glnop_Nop0,        /* InitNames */
    (GLvoid *) __glnop_Nop1,        /* LoadName */
    (GLvoid *) __glnop_Nop1,        /* PassThrough */
    (GLvoid *) __glnop_Nop0,        /* PopName */
    (GLvoid *) __glnop_Nop1,        /* PushName */
    (GLvoid *) __glnop_Nop1,        /* DrawBuffer */
    (GLvoid *) __glnop_Nop1,        /* Clear */
    (GLvoid *) __glnop_Nop4,        /* ClearAccum */
    (GLvoid *) __glnop_Nop1,        /* ClearIndex */
    (GLvoid *) __glnop_Nop4,        /* ClearColor */
    (GLvoid *) __glnop_Nop1,        /* ClearStencil */
    (GLvoid *) __glnop_Nop2,        /* ClearDepth */
    (GLvoid *) __glnop_Nop1,        /* StencilMask */
    (GLvoid *) __glnop_Nop4,        /* ColorMask */
    (GLvoid *) __glnop_Nop1,        /* DepthMask */
    (GLvoid *) __glnop_Nop1,        /* IndexMask */
    (GLvoid *) __glnop_Nop2,        /* Accum */
    (GLvoid *) __glnop_Nop1,        /* Disable */
    (GLvoid *) __glnop_Nop1,        /* Enable */
    (GLvoid *) __glnop_Nop0,        /* Finish */
    (GLvoid *) __glnop_Nop0,        /* Flush */
    (GLvoid *) __glnop_Nop0,        /* PopAttrib */
    (GLvoid *) __glnop_Nop1,        /* PushAttrib */
    (GLvoid *) __glnop_Nop8,        /* Map1d */
    (GLvoid *) __glnop_Nop6,        /* Map1f */
    (GLvoid *) __glnop_Nop14,       /* Map2d */
    (GLvoid *) __glnop_Nop10,       /* Map2f */
    (GLvoid *) __glnop_Nop5,        /* MapGrid1d */
    (GLvoid *) __glnop_Nop3,        /* MapGrid1f */
    (GLvoid *) __glnop_Nop10,       /* MapGrid2d */
    (GLvoid *) __glnop_Nop6,        /* MapGrid2f */
    (GLvoid *) __glnop_Nop2,        /* EvalCoord1d */
    (GLvoid *) __glnop_Nop1,        /* EvalCoord1dv */
    (GLvoid *) __glnop_Nop1,        /* EvalCoord1f */
    (GLvoid *) __glnop_Nop1,        /* EvalCoord1fv */
    (GLvoid *) __glnop_Nop4,        /* EvalCoord2d */
    (GLvoid *) __glnop_Nop1,        /* EvalCoord2dv */
    (GLvoid *) __glnop_Nop2,        /* EvalCoord2f */
    (GLvoid *) __glnop_Nop1,        /* EvalCoord2fv */
    (GLvoid *) __glnop_Nop3,        /* EvalMesh1 */
    (GLvoid *) __glnop_Nop1,        /* EvalPoint1 */
    (GLvoid *) __glnop_Nop5,        /* EvalMesh2 */
    (GLvoid *) __glnop_Nop2,        /* EvalPoint2 */
    (GLvoid *) __glnop_Nop2,        /* AlphaFunc */
    (GLvoid *) __glnop_Nop2,        /* BlendFunc */
    (GLvoid *) __glnop_Nop1,        /* LogicOp */
    (GLvoid *) __glnop_Nop3,        /* StencilFunc */
    (GLvoid *) __glnop_Nop3,        /* StencilOp */
    (GLvoid *) __glnop_Nop1,        /* DepthFunc */
    (GLvoid *) __glnop_Nop2,        /* PixelZoom */
    (GLvoid *) __glnop_Nop2,        /* PixelTransferf */
    (GLvoid *) __glnop_Nop2,        /* PixelTransferi */
    (GLvoid *) __glnop_Nop2,        /* PixelStoref */
    (GLvoid *) __glnop_Nop2,        /* PixelStorei */
    (GLvoid *) __glnop_Nop3,        /* PixelMapfv */
    (GLvoid *) __glnop_Nop3,        /* PixelMapuiv */
    (GLvoid *) __glnop_Nop3,        /* PixelMapusv */
    (GLvoid *) __glnop_Nop1,        /* ReadBuffer */
    (GLvoid *) __glnop_Nop5,        /* CopyPixels */
    (GLvoid *) __glnop_Nop7,        /* ReadPixels */
    (GLvoid *) __glnop_Nop5,        /* DrawPixels */
    (GLvoid *) __glnop_Nop2,        /* GetBooleanv */
    (GLvoid *) __glnop_Nop2,        /* GetClipPlane */
    (GLvoid *) __glnop_Nop2,        /* GetDoublev */
    (GLvoid *) __glnop_Nop0,        /* GetError */
    (GLvoid *) __glnop_Nop2,        /* GetFloatv */
    (GLvoid *) __glnop_Nop2,        /* GetIntegerv */
    (GLvoid *) __glnop_Nop3,        /* GetLightfv */
    (GLvoid *) __glnop_Nop3,        /* GetLightiv */
    (GLvoid *) __glnop_Nop3,        /* GetMapdv */
    (GLvoid *) __glnop_Nop3,        /* GetMapfv */
    (GLvoid *) __glnop_Nop3,        /* GetMapiv */
    (GLvoid *) __glnop_Nop3,        /* GetMaterialfv */
    (GLvoid *) __glnop_Nop3,        /* GetMaterialiv */
    (GLvoid *) __glnop_Nop2,        /* GetPixelMapfv */
    (GLvoid *) __glnop_Nop2,        /* GetPixelMapuiv */
    (GLvoid *) __glnop_Nop2,        /* GetPixelMapusv */
    (GLvoid *) __glnop_Nop1,        /* GetPolygonStipple */
    (GLvoid *) __glnop_Nop1,        /* GetString */
    (GLvoid *) __glnop_Nop3,        /* GetTexEnvfv */
    (GLvoid *) __glnop_Nop3,        /* GetTexEnviv */
    (GLvoid *) __glnop_Nop3,        /* GetTexGendv */
    (GLvoid *) __glnop_Nop3,        /* GetTexGenfv */
    (GLvoid *) __glnop_Nop3,        /* GetTexGeniv */
    (GLvoid *) __glnop_Nop5,        /* GetTexImage */
    (GLvoid *) __glnop_Nop3,        /* GetTexParameterfv */
    (GLvoid *) __glnop_Nop3,        /* GetTexParameteriv */
    (GLvoid *) __glnop_Nop4,        /* GetTexLevelParameterfv */
    (GLvoid *) __glnop_Nop4,        /* GetTexLevelParameteriv */
    (GLvoid *) __glnop_Nop1,        /* IsEnabled */
    (GLvoid *) __glnop_Nop1,        /* IsList */
    (GLvoid *) __glnop_Nop4,        /* DepthRange */
    (GLvoid *) __glnop_Nop12,       /* Frustum */
    (GLvoid *) __glnop_Nop0,        /* LoadIdentity */
    (GLvoid *) __glnop_Nop1,        /* LoadMatrixf */
    (GLvoid *) __glnop_Nop1,        /* LoadMatrixd */
    (GLvoid *) __glnop_Nop1,        /* MatrixMode */
    (GLvoid *) __glnop_Nop1,        /* MultMatrixf */
    (GLvoid *) __glnop_Nop1,        /* MultMatrixd */
    (GLvoid *) __glnop_Nop12,       /* Ortho */
    (GLvoid *) __glnop_Nop0,        /* PopMatrix */
    (GLvoid *) __glnop_Nop0,        /* PushMatrix */
    (GLvoid *) __glnop_Nop8,        /* Rotated */
    (GLvoid *) __glnop_Nop4,        /* Rotatef */
    (GLvoid *) __glnop_Nop6,        /* Scaled */
    (GLvoid *) __glnop_Nop3,        /* Scalef */
    (GLvoid *) __glnop_Nop6,        /* Translated */
    (GLvoid *) __glnop_Nop3,        /* Translatef */
    (GLvoid *) __glnop_Nop4,        /* Viewport */
    (GLvoid *) __glnop_Nop1,        /* ArrayElement */
    (GLvoid *) __glnop_Nop2,        /* BindTexture */
    (GLvoid *) __glnop_Nop4,        /* ColorPointer */
    (GLvoid *) __glnop_Nop1,        /* DisableClientState */
    (GLvoid *) __glnop_Nop3,        /* DrawArrays */
    (GLvoid *) __glnop_Nop4,        /* DrawElements */
    (GLvoid *) __glnop_Nop2,        /* EdgeFlagPointer */
    (GLvoid *) __glnop_Nop1,        /* EnableClientState */
    (GLvoid *) __glnop_Nop3,        /* IndexPointer */
    (GLvoid *) __glnop_Nop1,        /* Indexub */
    (GLvoid *) __glnop_Nop1,        /* Indexubv */
    (GLvoid *) __glnop_Nop3,        /* InterleavedArrays */
    (GLvoid *) __glnop_Nop3,        /* NormalPointer */
    (GLvoid *) __glnop_Nop2,        /* PolygonOffset */
    (GLvoid *) __glnop_Nop4,        /* TexCoordPointer */
    (GLvoid *) __glnop_Nop4,        /* VertexPointer */
    (GLvoid *) __glnop_Nop3,        /* AreTexturesResident */
    (GLvoid *) __glnop_Nop7,        /* CopyTexImage1D */
    (GLvoid *) __glnop_Nop8,        /* CopyTexImage2D */
    (GLvoid *) __glnop_Nop6,        /* CopyTexSubImage1D */
    (GLvoid *) __glnop_Nop8,        /* CopyTexSubImage2D */
    (GLvoid *) __glnop_Nop2,        /* DeleteTextures */
    (GLvoid *) __glnop_Nop2,        /* GenTextures */
    (GLvoid *) __glnop_Nop2,        /* GetPointerv */
    (GLvoid *) __glnop_Nop1,        /* IsTexture */
    (GLvoid *) __glnop_Nop3,        /* PrioritizeTextures */
    (GLvoid *) __glnop_Nop7,        /* TexSubImage1D */
    (GLvoid *) __glnop_Nop9,        /* TexSubImage2D */
    (GLvoid *) __glnop_Nop0,        /* PopClientAttrib */
    (GLvoid *) __glnop_Nop1,        /* PushClientAttrib */

#if GL_VERSION_1_2
    (GLvoid *) __glnop_Nop4,        /* BlendColor */
    (GLvoid *) __glnop_Nop1,        /* BlendEquation */
    (GLvoid *) __glnop_Nop6,        /* DrawRangeElements */
    (GLvoid *) __glnop_Nop6,        /* ColorTable */
    (GLvoid *) __glnop_Nop3,        /* ColorTableParameterfv */
    (GLvoid *) __glnop_Nop3,        /* ColorTableParameteriv */
    (GLvoid *) __glnop_Nop5,        /* CopyColorTable */
    (GLvoid *) __glnop_Nop4,        /* GetColorTable */
    (GLvoid *) __glnop_Nop3,        /* GetColorTableParameterfv */
    (GLvoid *) __glnop_Nop3,        /* GetColorTableParameteriv */
    (GLvoid *) __glnop_Nop6,        /* ColorSubTable */
    (GLvoid *) __glnop_Nop5,        /* CopyColorSubTable */
    (GLvoid *) __glnop_Nop6,        /* ConvolutionFilter1D */
    (GLvoid *) __glnop_Nop7,        /* ConvolutionFilter2D */
    (GLvoid *) __glnop_Nop3,        /* ConvolutionParameterf */
    (GLvoid *) __glnop_Nop3,        /* ConvolutionParameterfv */
    (GLvoid *) __glnop_Nop3,        /* ConvolutionParameteri */
    (GLvoid *) __glnop_Nop3,        /* ConvolutionParameteriv */
    (GLvoid *) __glnop_Nop5,        /* CopyConvolutionFilter1D */
    (GLvoid *) __glnop_Nop6,        /* CopyConvolutionFilter2D */
    (GLvoid *) __glnop_Nop4,        /* GetConvolutionFilter */
    (GLvoid *) __glnop_Nop3,        /* GetConvolutionParameterfv */
    (GLvoid *) __glnop_Nop3,        /* GetConvolutionParameteriv */
    (GLvoid *) __glnop_Nop6,        /* GetSeparableFilter */
    (GLvoid *) __glnop_Nop8,        /* SeparableFilter2D */
    (GLvoid *) __glnop_Nop5,        /* GetHistogram */
    (GLvoid *) __glnop_Nop3,        /* GetHistogramParameterfv */
    (GLvoid *) __glnop_Nop3,        /* GetHistogramParameteriv */
    (GLvoid *) __glnop_Nop5,        /* GetMinmax */
    (GLvoid *) __glnop_Nop3,        /* GetMinmaxParameterfv */
    (GLvoid *) __glnop_Nop3,        /* GetMinmaxParameteriv */
    (GLvoid *) __glnop_Nop4,        /* Histogram */
    (GLvoid *) __glnop_Nop3,        /* Minmax */
    (GLvoid *) __glnop_Nop1,        /* ResetHistogram */
    (GLvoid *) __glnop_Nop1,        /* ResetMinmax */
    (GLvoid *) __glnop_Nop10,       /* TexImage3D */
    (GLvoid *) __glnop_Nop11,       /* TexSubImage3D */
    (GLvoid *) __glnop_Nop9,        /* CopyTexSubImage3D */
#endif
#if GL_VERSION_1_3
    (GLvoid *) __glnop_Nop1,        /* ActiveTexture */
    (GLvoid *) __glnop_Nop1,        /* ClientActiveTexture */
    (GLvoid *) __glnop_Nop2,        /* MultiTexCoord1d */
    (GLvoid *) __glnop_Nop2,        /* MultiTexCoord1dv */
    (GLvoid *) __glnop_Nop2,        /* MultiTexCoord1f */
    (GLvoid *) __glnop_Nop2,        /* MultiTexCoord1fv */
    (GLvoid *) __glnop_Nop2,        /* MultiTexCoord1i */
    (GLvoid *) __glnop_Nop2,        /* MultiTexCoord1iv */
    (GLvoid *) __glnop_Nop2,        /* MultiTexCoord1s */
    (GLvoid *) __glnop_Nop2,        /* MultiTexCoord1sv */
    (GLvoid *) __glnop_Nop3,        /* MultiTexCoord2d */
    (GLvoid *) __glnop_Nop2,        /* MultiTexCoord2dv */
    (GLvoid *) __glnop_Nop3,        /* MultiTexCoord2f */
    (GLvoid *) __glnop_Nop2,        /* MultiTexCoord2fv */
    (GLvoid *) __glnop_Nop3,        /* MultiTexCoord2i */
    (GLvoid *) __glnop_Nop2,        /* MultiTexCoord2iv */
    (GLvoid *) __glnop_Nop3,        /* MultiTexCoord2s */
    (GLvoid *) __glnop_Nop2,        /* MultiTexCoord2sv */
    (GLvoid *) __glnop_Nop4,        /* MultiTexCoord3d */
    (GLvoid *) __glnop_Nop2,        /* MultiTexCoord3dv */
    (GLvoid *) __glnop_Nop4,        /* MultiTexCoord3f */
    (GLvoid *) __glnop_Nop2,        /* MultiTexCoord3fv */
    (GLvoid *) __glnop_Nop4,        /* MultiTexCoord3i */
    (GLvoid *) __glnop_Nop2,        /* MultiTexCoord3iv */
    (GLvoid *) __glnop_Nop4,        /* MultiTexCoord3s */
    (GLvoid *) __glnop_Nop2,        /* MultiTexCoord3sv */
    (GLvoid *) __glnop_Nop5,        /* MultiTexCoord4d */
    (GLvoid *) __glnop_Nop2,        /* MultiTexCoord4dv */
    (GLvoid *) __glnop_Nop5,        /* MultiTexCoord4f */
    (GLvoid *) __glnop_Nop2,        /* MultiTexCoord4fv */
    (GLvoid *) __glnop_Nop5,        /* MultiTexCoord4i */
    (GLvoid *) __glnop_Nop2,        /* MultiTexCoord4iv */
    (GLvoid *) __glnop_Nop5,        /* MultiTexCoord4s */
    (GLvoid *) __glnop_Nop2,        /* MultiTexCoord4sv */
    (GLvoid *) __glnop_Nop1,        /* LoadTransposeMatrixf */
    (GLvoid *) __glnop_Nop1,        /* LoadTransposeMatrixd */
    (GLvoid *) __glnop_Nop1,        /* MultTransposeMatrixf */
    (GLvoid *) __glnop_Nop1,        /* MultTransposeMatrixd */
    (GLvoid *) __glnop_Nop2,        /* SampleCoverage */
    (GLvoid *) __glnop_Nop9,        /* CompressedTexImage3D */
    (GLvoid *) __glnop_Nop8,        /* CompressedTexImage2D */
    (GLvoid *) __glnop_Nop7,        /* CompressedTexImage1D */
    (GLvoid *) __glnop_Nop11,       /* CompressedTexSubImage3D */
    (GLvoid *) __glnop_Nop9,        /* CompressedTexSubImage2D */
    (GLvoid *) __glnop_Nop7,        /* CompressedTexSubImage1D */
    (GLvoid *) __glnop_Nop3,        /* GetCompressedTexImage */
#endif
#if GL_VERSION_1_4
    (GLvoid *) __glnop_Nop4,        /* BlendFuncSeparate */
    (GLvoid *) __glnop_Nop1,        /* FogCoordf */
    (GLvoid *) __glnop_Nop1,        /* FogCoordfv */
    (GLvoid *) __glnop_Nop1,        /* FogCoordd */
    (GLvoid *) __glnop_Nop1,        /* FogCoorddv */
    (GLvoid *) __glnop_Nop3,        /* FogCoordPointer */
    (GLvoid *) __glnop_Nop4,        /* MultiDrawArrays */
    (GLvoid *) __glnop_Nop5,        /* MultiDrawElements */
    (GLvoid *) __glnop_Nop2,        /* PointParameterf */
    (GLvoid *) __glnop_Nop2,        /* PointParameterfv */
    (GLvoid *) __glnop_Nop2,        /* PointParameteri */
    (GLvoid *) __glnop_Nop2,        /* PointParameteriv */
    (GLvoid *) __glnop_Nop3,        /* SecondaryColor3b */
    (GLvoid *) __glnop_Nop1,        /* SecondaryColor3bv */
    (GLvoid *) __glnop_Nop3,        /* SecondaryColor3d */
    (GLvoid *) __glnop_Nop1,        /* SecondaryColor3dv */
    (GLvoid *) __glnop_Nop3,        /* SecondaryColor3f */
    (GLvoid *) __glnop_Nop1,        /* SecondaryColor3fv */
    (GLvoid *) __glnop_Nop3,        /* SecondaryColor3i */
    (GLvoid *) __glnop_Nop1,        /* SecondaryColor3iv */
    (GLvoid *) __glnop_Nop3,        /* SecondaryColor3s */
    (GLvoid *) __glnop_Nop1,        /* SecondaryColor3sv */
    (GLvoid *) __glnop_Nop3,        /* SecondaryColor3ub */
    (GLvoid *) __glnop_Nop1,        /* SecondaryColor3ubv */
    (GLvoid *) __glnop_Nop3,        /* SecondaryColor3ui */
    (GLvoid *) __glnop_Nop1,        /* SecondaryColor3uiv */
    (GLvoid *) __glnop_Nop3,        /* SecondaryColor3us */
    (GLvoid *) __glnop_Nop1,        /* SecondaryColor3usv */
    (GLvoid *) __glnop_Nop4,        /* SecondaryColorPointer */
    (GLvoid *) __glnop_Nop2,        /* WindowPos2d */
    (GLvoid *) __glnop_Nop1,        /* WindowPos2dv */
    (GLvoid *) __glnop_Nop2,        /* WindowPos2f */
    (GLvoid *) __glnop_Nop1,        /* WindowPos2fv */
    (GLvoid *) __glnop_Nop2,        /* WindowPos2i */
    (GLvoid *) __glnop_Nop1,        /* WindowPos2iv */
    (GLvoid *) __glnop_Nop2,        /* WindowPos2s */
    (GLvoid *) __glnop_Nop1,        /* WindowPos2sv */
    (GLvoid *) __glnop_Nop3,        /* WindowPos3d */
    (GLvoid *) __glnop_Nop1,        /* WindowPos3dv */
    (GLvoid *) __glnop_Nop3,        /* WindowPos3f */
    (GLvoid *) __glnop_Nop1,        /* WindowPos3fv */
    (GLvoid *) __glnop_Nop3,        /* WindowPos3i */
    (GLvoid *) __glnop_Nop1,        /* WindowPos3iv */
    (GLvoid *) __glnop_Nop3,        /* WindowPos3s */
    (GLvoid *) __glnop_Nop1,        /* WindowPos3sv */
#endif
#if GL_VERSION_1_5
    (GLvoid *) __glnop_Nop2,        /* GenQueries */
    (GLvoid *) __glnop_Nop2,        /* DeleteQueries */
    (GLvoid *) __glnop_Nop1,        /* IsQuery */
    (GLvoid *) __glnop_Nop2,        /* BeginQuery */
    (GLvoid *) __glnop_Nop1,        /* EndQuery */
    (GLvoid *) __glnop_Nop3,        /* GetQueryiv */
    (GLvoid *) __glnop_Nop3,        /* GetQueryObjectiv */
    (GLvoid *) __glnop_Nop3,        /* GetQueryObjectuiv */
    (GLvoid *) __glnop_Nop2,        /* BindBuffer */
    (GLvoid *) __glnop_Nop2,        /* DeleteBuffers */
    (GLvoid *) __glnop_Nop2,        /* GenBuffers */
    (GLvoid *) __glnop_Nop1,        /* IsBuffer */
    (GLvoid *) __glnop_Nop4,        /* BufferData */
    (GLvoid *) __glnop_Nop4,        /* BufferSubData */
    (GLvoid *) __glnop_Nop4,        /* GetBufferSubData */
    (GLvoid *) __glnop_Nop2,        /* MapBuffer */
    (GLvoid *) __glnop_Nop1,        /* UnmapBuffer */
    (GLvoid *) __glnop_Nop3,        /* GetBufferParameteriv */
    (GLvoid *) __glnop_Nop3,        /* GetBufferPointerv */
#endif
#if GL_VERSION_2_0
    (GLvoid *) __glnop_Nop2,        /* BlendEquationSeparate */
    (GLvoid *) __glnop_Nop2,        /* DrawBuffers */
    (GLvoid *) __glnop_Nop4,        /* StencilOpSeparate */
    (GLvoid *) __glnop_Nop4,        /* StencilFuncSeparate */
    (GLvoid *) __glnop_Nop2,        /* StencilMaskSeparate */
    (GLvoid *) __glnop_Nop2,        /* AttachShader */
    (GLvoid *) __glnop_Nop3,        /* BindAttribLocation */
    (GLvoid *) __glnop_Nop1,        /* CompileShader */
    (GLvoid *) __glnop_Nop1,        /* CreateProgram */
    (GLvoid *) __glnop_Nop1,        /* CreateShader */
    (GLvoid *) __glnop_Nop1,        /* DeleteProgram */
    (GLvoid *) __glnop_Nop1,        /* DeleteShader */
    (GLvoid *) __glnop_Nop2,        /* DetachShader */
    (GLvoid *) __glnop_Nop1,        /* DisableVertexAttribArray */
    (GLvoid *) __glnop_Nop1,        /* EnableVertexAttribArray */
    (GLvoid *) __glnop_Nop7,        /* GetActiveAttrib */
    (GLvoid *) __glnop_Nop7,        /* GetActiveUniform */
    (GLvoid *) __glnop_Nop4,        /* GetAttachedShaders */
    (GLvoid *) __glnop_Nop2,        /* GetAttribLocation */
    (GLvoid *) __glnop_Nop3,        /* GetProgramiv */
    (GLvoid *) __glnop_Nop4,        /* GetProgramInfoLog */
    (GLvoid *) __glnop_Nop3,        /* GetShaderiv */
    (GLvoid *) __glnop_Nop4,        /* GetShaderInfoLog */
    (GLvoid *) __glnop_Nop4,        /* GetShaderSource */
    (GLvoid *) __glnop_Nop2,        /* GetUniformLocation */
    (GLvoid *) __glnop_Nop3,        /* GetUniformfv */
    (GLvoid *) __glnop_Nop3,        /* GetUniformiv */
    (GLvoid *) __glnop_Nop3,        /* GetVertexAttribdv */
    (GLvoid *) __glnop_Nop3,        /* GetVertexAttribfv */
    (GLvoid *) __glnop_Nop3,        /* GetVertexAttribiv */
    (GLvoid *) __glnop_Nop3,        /* GetVertexAttribPointerv */
    (GLvoid *) __glnop_Nop1,        /* IsProgram */
    (GLvoid *) __glnop_Nop1,        /* IsShader */
    (GLvoid *) __glnop_Nop1,        /* LinkProgram */
    (GLvoid *) __glnop_Nop4,        /* ShaderSource */
    (GLvoid *) __glnop_Nop1,        /* UseProgram */
    (GLvoid *) __glnop_Nop2,        /* Uniform1f */
    (GLvoid *) __glnop_Nop3,        /* Uniform2f */
    (GLvoid *) __glnop_Nop4,        /* Uniform3f */
    (GLvoid *) __glnop_Nop5,        /* Uniform4f */
    (GLvoid *) __glnop_Nop2,        /* Uniform1i */
    (GLvoid *) __glnop_Nop3,        /* Uniform2i */
    (GLvoid *) __glnop_Nop4,        /* Uniform3i */
    (GLvoid *) __glnop_Nop5,        /* Uniform4i */
    (GLvoid *) __glnop_Nop3,        /* Uniform1fv */
    (GLvoid *) __glnop_Nop3,        /* Uniform2fv */
    (GLvoid *) __glnop_Nop3,        /* Uniform3fv */
    (GLvoid *) __glnop_Nop3,        /* Uniform4fv */
    (GLvoid *) __glnop_Nop3,        /* Uniform1iv */
    (GLvoid *) __glnop_Nop3,        /* Uniform2iv */
    (GLvoid *) __glnop_Nop3,        /* Uniform3iv */
    (GLvoid *) __glnop_Nop3,        /* Uniform4iv */
    (GLvoid *) __glnop_Nop4,        /* UniformMatrix2fv */
    (GLvoid *) __glnop_Nop4,        /* UniformMatrix3fv */
    (GLvoid *) __glnop_Nop4,        /* UniformMatrix4fv */
    (GLvoid *) __glnop_Nop1,        /* ValidateProgram */
    (GLvoid *) __glnop_Nop2,        /* VertexAttrib1d */
    (GLvoid *) __glnop_Nop2,        /* VertexAttrib1dv */
    (GLvoid *) __glnop_Nop2,        /* VertexAttrib1f */
    (GLvoid *) __glnop_Nop2,        /* VertexAttrib1fv */
    (GLvoid *) __glnop_Nop2,        /* VertexAttrib1s */
    (GLvoid *) __glnop_Nop2,        /* VertexAttrib1sv */
    (GLvoid *) __glnop_Nop3,        /* VertexAttrib2d */
    (GLvoid *) __glnop_Nop2,        /* VertexAttrib2dv */
    (GLvoid *) __glnop_Nop3,        /* VertexAttrib2f */
    (GLvoid *) __glnop_Nop2,        /* VertexAttrib2fv */
    (GLvoid *) __glnop_Nop3,        /* VertexAttrib2s */
    (GLvoid *) __glnop_Nop2,        /* VertexAttrib2sv */
    (GLvoid *) __glnop_Nop4,        /* VertexAttrib3d */
    (GLvoid *) __glnop_Nop2,        /* VertexAttrib3dv */
    (GLvoid *) __glnop_Nop4,        /* VertexAttrib3f */
    (GLvoid *) __glnop_Nop2,        /* VertexAttrib3fv */
    (GLvoid *) __glnop_Nop4,        /* VertexAttrib3s */
    (GLvoid *) __glnop_Nop2,        /* VertexAttrib3sv */
    (GLvoid *) __glnop_Nop2,        /* VertexAttrib4Nbv */
    (GLvoid *) __glnop_Nop2,        /* VertexAttrib4Niv */
    (GLvoid *) __glnop_Nop2,        /* VertexAttrib4Nsv */
    (GLvoid *) __glnop_Nop5,        /* VertexAttrib4Nub */
    (GLvoid *) __glnop_Nop2,        /* VertexAttrib4Nubv */
    (GLvoid *) __glnop_Nop2,        /* VertexAttrib4Nuiv */
    (GLvoid *) __glnop_Nop2,        /* VertexAttrib4Nusv */
    (GLvoid *) __glnop_Nop2,        /* VertexAttrib4bv */
    (GLvoid *) __glnop_Nop5,        /* VertexAttrib4d */
    (GLvoid *) __glnop_Nop2,        /* VertexAttrib4dv */
    (GLvoid *) __glnop_Nop5,        /* VertexAttrib4f */
    (GLvoid *) __glnop_Nop2,        /* VertexAttrib4fv */
    (GLvoid *) __glnop_Nop2,        /* VertexAttrib4iv */
    (GLvoid *) __glnop_Nop5,        /* VertexAttrib4s */
    (GLvoid *) __glnop_Nop2,        /* VertexAttrib4sv */
    (GLvoid *) __glnop_Nop2,        /* VertexAttrib4ubv */
    (GLvoid *) __glnop_Nop2,        /* VertexAttrib4uiv */
    (GLvoid *) __glnop_Nop2,        /* VertexAttrib4usv */
    (GLvoid *) __glnop_Nop6,        /* VertexAttribPointer */
#endif
#if GL_VERSION_2_1
    (GLvoid *) __glnop_Nop4,        /* UniformMatrix2x3fv */
    (GLvoid *) __glnop_Nop4,        /* UniformMatrix2x4fv */
    (GLvoid *) __glnop_Nop4,        /* UniformMatrix3x2fv */
    (GLvoid *) __glnop_Nop4,        /* UniformMatrix3x4fv */
    (GLvoid *) __glnop_Nop4,        /* UniformMatrix4x2fv */
    (GLvoid *) __glnop_Nop4,        /* UniformMatrix4x3fv */
#endif
#if GL_ARB_vertex_program
    (GLvoid *) __glnop_Nop4,        /* ProgramStringARB */
    (GLvoid *) __glnop_Nop2,        /* BindProgramARB */
    (GLvoid *) __glnop_Nop2,        /* DeleteProgramsARB */
    (GLvoid *) __glnop_Nop2,        /* GenProgramsARB */
    (GLvoid *) __glnop_Nop6,        /* ProgramEnvParameter4dARB */
    (GLvoid *) __glnop_Nop3,        /* ProgramEnvParameter4dvARB */
    (GLvoid *) __glnop_Nop6,        /* ProgramEnvParameter4fARB */
    (GLvoid *) __glnop_Nop3,        /* ProgramEnvParameter4fvARB */
    (GLvoid *) __glnop_Nop6,        /* ProgramLocalParameter4dARB */
    (GLvoid *) __glnop_Nop3,        /* ProgramLocalParameter4dvARB */
    (GLvoid *) __glnop_Nop6,        /* ProgramLocalParameter4fARB */
    (GLvoid *) __glnop_Nop3,        /* ProgramLocalParameter4fvARB */
    (GLvoid *) __glnop_Nop3,        /* GetProgramEnvParameterdvARB */
    (GLvoid *) __glnop_Nop3,        /* GetProgramEnvParameterfvARB */
    (GLvoid *) __glnop_Nop3,        /* GetProgramLocalParameterdvARB */
    (GLvoid *) __glnop_Nop3,        /* GetProgramLocalParameterfvARB */
    (GLvoid *) __glnop_Nop3,        /* GetProgramivARB */
    (GLvoid *) __glnop_Nop3,        /* GetProgramStringARB */
    (GLvoid *) __glnop_Nop1,        /* IsProgramARB */
#endif
#if GL_ARB_shader_objects
    (GLvoid *) __glnop_Nop1,        /* DeleteObjectARB */
    (GLvoid *) __glnop_Nop1,        /* GetHandleARB */
    (GLvoid *) __glnop_Nop4,        /* GetInfoLogARB */
    (GLvoid *) __glnop_Nop3,        /* GetObjectParameterfvARB */
    (GLvoid *) __glnop_Nop3,        /* GetObjectParameterivARB */
#endif
#if GL_ATI_vertex_array_object
    (GLvoid *) __glnop_Nop3,        /* NewObjectBufferATI */
    (GLvoid *) __glnop_Nop1,        /* IsObjectBufferATI) */
    (GLvoid *) __glnop_Nop5,        /* UpdateObjectBufferATI */
    (GLvoid *) __glnop_Nop3,        /* GetObjectBufferfvATI */
    (GLvoid *) __glnop_Nop3,        /* GetObjectBufferivATI */
    (GLvoid *) __glnop_Nop1,        /* FreeObjectBufferATI */
    (GLvoid *) __glnop_Nop6,        /* ArrayObjectATI */
    (GLvoid *) __glnop_Nop3,        /* GetArrayObjectfvATI */
    (GLvoid *) __glnop_Nop3,        /* GetArrayObjectivATI */
    (GLvoid *) __glnop_Nop5,        /* VariantArrayObjectATI */
    (GLvoid *) __glnop_Nop3,        /* GetVariantArrayObjectfvATI */
    (GLvoid *) __glnop_Nop3,        /* GetVariantArrayObjectivATI */
#endif
#if GL_ATI_vertex_attrib_array_object
    (GLvoid *) __glnop_Nop7,        /* VertexAttribArrayObjectATI */
    (GLvoid *) __glnop_Nop3,        /* GetVertexAttribArrayObjectfvATI */
    (GLvoid *) __glnop_Nop3,        /* GetVertexAttribArrayObjectivATI */
#endif
#if GL_ATI_element_array
    (GLvoid *) __glnop_Nop2,        /* ElementPointerATI */
    (GLvoid *) __glnop_Nop2,        /* DrawElementArrayATI */
    (GLvoid *) __glnop_Nop4,        /* DrawRangeElementArrayATI */
#endif
#if GL_EXT_stencil_two_side
    (GLvoid *) __glnop_Nop1,        /* ActiveStencilFaceEXT */
#endif
    (GLvoid *)__glnop_Nop4,         /* AddSwapHintRectWIN */
#if GL_EXT_depth_bounds_test
    (GLvoid *)__glnop_Nop2,         /* DepthBoundsEXT */
#endif
#if GL_EXT_framebuffer_object
    (GLvoid *) __glnop_Nop1,        /* IsRenderbufferEXT */
    (GLvoid *) __glnop_Nop2,        /* BindRenderbufferEXT */
    (GLvoid *) __glnop_Nop2,        /* DeleteRenderbuffersEXT */
    (GLvoid *) __glnop_Nop2,        /* GenRenderbuffersEXT */
    (GLvoid *) __glnop_Nop4,        /* RenderbufferStorageEXT */
    (GLvoid *) __glnop_Nop3,        /* GetRenderbufferParameterivEXT */
    (GLvoid *) __glnop_Nop1,        /* IsFramebufferEXT */
    (GLvoid *) __glnop_Nop2,        /* BindFramebufferEXT */
    (GLvoid *) __glnop_Nop2,        /* DeleteFramebuffersEXT */
    (GLvoid *) __glnop_Nop2,        /* GenFramebuffersEXT */
    (GLvoid *) __glnop_Nop1,        /* CheckFramebufferStatusEXT */
    (GLvoid *) __glnop_Nop5,        /* FramebufferTexture1DEXT */
    (GLvoid *) __glnop_Nop5,        /* FramebufferTexture2DEXT */
    (GLvoid *) __glnop_Nop6,        /* FramebufferTexture3DEXT */
    (GLvoid *) __glnop_Nop4,        /* FramebufferRenderbufferEXT */
    (GLvoid *) __glnop_Nop4,        /* GetFramebufferAttachmentParameterivEXT */
    (GLvoid *) __glnop_Nop1,        /* GenerateMipmapEXT */
#if GL_EXT_framebuffer_blit
    (GLvoid *) __glnop_Nop10,       /* BlitFramebufferEXT */
#if GL_EXT_framebuffer_multisample
    (GLvoid *) __glnop_Nop5,        /* RenderbufferStorageMultisampleEXT */
#endif  /* GL_EXT_framebuffer_multisample  */
#endif  /* GL_EXT_framebuffer_blit         */
#endif  /* GL_EXT_framebuffer_object       */
#if GL_NV_occlusion_query
    (GLvoid *) __glnop_Nop1,        /* glBeginOcclusionQueryNV */
    (GLvoid *) __glnop_Nop0,        /* glEndOcclusionQueryNV */
#endif
#if GL_EXT_bindable_uniform
    (GLvoid *) __glnop_Nop3,        /* UniformBufferEXT */
    (GLvoid *) __glnop_Nop2,        /* GetUniformBufferSizeEXT */
    (GLvoid *) __glnop_Nop2,        /* GetUniformOffsetEXT */
#endif
#if GL_EXT_texture_integer
    (GLvoid *) __glnop_Nop4,        /* ClearColorIiEXT */
    (GLvoid *) __glnop_Nop4,        /* ClearColorIuiEXT */
    (GLvoid *) __glnop_Nop3,        /* TexParameterIivEXT */
    (GLvoid *) __glnop_Nop3,        /* TexParameterIuivEXT */
    (GLvoid *) __glnop_Nop3,        /* GetTexParameterIivEXT */
    (GLvoid *) __glnop_Nop3,        /* GetTexParameterIuivEXT */
#endif
#if GL_EXT_gpu_shader4
    (GLvoid *) __glnop_Nop2,        /* VertexAttribI1iEXT */
    (GLvoid *) __glnop_Nop3,        /* VertexAttribI2iEXT */
    (GLvoid *) __glnop_Nop4,        /* VertexAttribI3iEXT */
    (GLvoid *) __glnop_Nop5,        /* VertexAttribI4iEXT */
    (GLvoid *) __glnop_Nop2,        /* VertexAttribI1uiEXT */
    (GLvoid *) __glnop_Nop3,        /* VertexAttribI2uiEXT */
    (GLvoid *) __glnop_Nop4,        /* VertexAttribI3uiEXT */
    (GLvoid *) __glnop_Nop5,        /* VertexAttribI4uiEXT */
    (GLvoid *) __glnop_Nop2,        /* VertexAttribI1ivEXT */
    (GLvoid *) __glnop_Nop2,        /* VertexAttribI2ivEXT */
    (GLvoid *) __glnop_Nop2,        /* VertexAttribI3ivEXT */
    (GLvoid *) __glnop_Nop2,        /* VertexAttribI4ivEXT */
    (GLvoid *) __glnop_Nop2,        /* VertexAttribI1uivEXT */
    (GLvoid *) __glnop_Nop2,        /* VertexAttribI2uivEXT */
    (GLvoid *) __glnop_Nop2,        /* VertexAttribI3uivEXT */
    (GLvoid *) __glnop_Nop2,        /* VertexAttribI4uivEXT */
    (GLvoid *) __glnop_Nop2,        /* VertexAttribI4bvEXT */
    (GLvoid *) __glnop_Nop2,        /* VertexAttribI4svEXT */
    (GLvoid *) __glnop_Nop2,        /* VertexAttribI4ubvEXT */
    (GLvoid *) __glnop_Nop2,        /* VertexAttribI4usvEXT */
    (GLvoid *) __glnop_Nop5,        /* VertexAttribIPointerEXT */
    (GLvoid *) __glnop_Nop3,        /* GetVertexAttribIivEXT */
    (GLvoid *) __glnop_Nop3,        /* GetVertexAttribIuivEXT */
    (GLvoid *) __glnop_Nop2,        /* Uniform1uiEXT */
    (GLvoid *) __glnop_Nop3,        /* Uniform2uiEXT */
    (GLvoid *) __glnop_Nop4,        /* Uniform3uiEXT */
    (GLvoid *) __glnop_Nop5,        /* Uniform4uiEXT */
    (GLvoid *) __glnop_Nop3,        /* Uniform1uivEXT */
    (GLvoid *) __glnop_Nop3,        /* Uniform2uivEXT */
    (GLvoid *) __glnop_Nop3,        /* Uniform3uivEXT */
    (GLvoid *) __glnop_Nop3,        /* Uniform4uivEXT */
    (GLvoid *) __glnop_Nop3,        /* GetUniformuivEXT */
    (GLvoid *) __glnop_Nop3,        /* BindFragDataLocationEXT */
    (GLvoid *) __glnop_Nop2,        /* GetFragDataLocationEXT */
#endif
#if GL_EXT_geometry_shader4
    (GLvoid *) __glnop_Nop3,        /* ProgramParameteriEXT */
    (GLvoid *) __glnop_Nop4,        /* FramebufferTextureEXT */
    (GLvoid *) __glnop_Nop5,        /* FramebufferTextureLayerEXT */
    (GLvoid *) __glnop_Nop5,        /* FramebufferTextureFaceEXT */
#endif
#if GL_EXT_draw_buffers2
    (GLvoid *) __glnop_Nop5,        /* ColorMaskIndexedEXT */
    (GLvoid *) __glnop_Nop3,        /* GetBooleanIndexedvEXT */
    (GLvoid *) __glnop_Nop3,        /* GetIntegerIndexedvEXT */
    (GLvoid *) __glnop_Nop2,        /* EnableIndexedEXT */
    (GLvoid *) __glnop_Nop2,        /* DisableIndexedEXT */
    (GLvoid *) __glnop_Nop2,        /* IsEnableIndexedEXT */
#endif
#if GL_EXT_texture_buffer_object
    (GLvoid *) __glnop_Nop3,        /* TexBufferEXT */
#endif
#if GL_EXT_gpu_program_parameters
    (GLvoid *) __glnop_Nop4,        /* ProgramEnvParameters4fvEXT */
    (GLvoid *) __glnop_Nop4,        /* ProgramLocalParameters4fvEXT */
#endif
#if GL_EXT_draw_instanced
    (GLvoid *) __glnop_Nop4,        /* DrawArraysInstancedEXT */
    (GLvoid *) __glnop_Nop5,        /* DrawElementsInstancedEXT */
#endif
#if GL_ARB_color_buffer_float
    (GLvoid *) __glnop_Nop2,        /* ClampColorARB */
#endif
#if GL_EXT_timer_query
    (GLvoid *) __glnop_Nop3,        /* GetQueryObjecti64vEXT */
    (GLvoid *) __glnop_Nop3,        /* GetQueryObjectui64vEXT */
#endif
    },
};
