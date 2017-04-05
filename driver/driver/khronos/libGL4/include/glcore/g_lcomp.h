/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __g_lcomp_h_
#define __g_lcomp_h_

#if (_MIPS_SZPTR == 64)
#define __GL_PAD(x) (((x) + 7) & ~7) /* round to doubleword multiple */
#define __GL64PAD(x)  __GL_PAD(x)    /* optional double word padding */
#else
#define __GL_PAD(x) (((x) + 3) & ~3) /* round to word multiple */
#define __GL64PAD(x) (x)             /* not needed for 32-bit case */
#endif

/*
** NOTE: The following __gllc_* structures must be padded to 32-bit boundary
** and any pointers in the structure must be at 64-bit boundary so that the
** code can work on 64-bit OS.
*/
struct __gllc_Begin_Rec
{
    GLenum primType;
};

struct __gllc_Map1f_Rec
{
    GLenum target;
    GLfloat u1;
    GLfloat u2;
    GLint order;
    /* points */
};

struct __gllc_Map2f_Rec
{
    GLenum target;
    GLfloat u1;
    GLfloat u2;
    GLint uorder;
    GLfloat v1;
    GLfloat v2;
    GLint vorder;
    /* points */
};

struct __gllc_CallList_Rec {
    GLuint list;
};

struct __gllc_CallLists_Rec {
    GLsizei n;
    /* lists with GLuint type */
};

struct __gllc_ListBase_Rec {
    GLuint base;
};

struct __gllc_Color3fv_Rec {
    GLfloat v[3];
};

struct __gllc_Color4fv_Rec {
    GLfloat v[4];
};

struct __gllc_Color4ubv_Rec {
    GLubyte v[4];
};

struct __gllc_SecondaryColor3fv_Rec {
    GLfloat v[3];
};

struct __gllc_EdgeFlag_Rec {
    GLboolean flag;
    GLubyte pad1;
    GLubyte pad2;
    GLubyte pad3;
};

struct __gllc_Indexf_Rec {
    GLfloat c;
};

struct __gllc_Normal3fv_Rec {
    GLfloat v[3];
};

struct __gllc_RasterPos2fv_Rec {
    GLfloat v[2];
};

struct __gllc_RasterPos3fv_Rec {
    GLfloat v[3];
};

struct __gllc_RasterPos4fv_Rec {
    GLfloat v[4];
};

struct __gllc_Rectf_Rec {
    GLfloat x1;
    GLfloat y1;
    GLfloat x2;
    GLfloat y2;
};

struct __gllc_TexCoord2fv_Rec {
    GLfloat v[2];
};

struct __gllc_TexCoord3fv_Rec {
    GLfloat v[3];
};

struct __gllc_TexCoord4fv_Rec {
    GLfloat v[4];
};

struct __gllc_Vertex2fv_Rec {
    GLfloat v[2];
};

struct __gllc_Vertex3fv_Rec {
    GLfloat v[3];
};

struct __gllc_Vertex4fv_Rec {
    GLfloat v[4];
};

struct __gllc_ClipPlane_Rec {
    GLdouble equation[4];
    GLenum plane;
};

struct __gllc_ColorMaterial_Rec {
    GLenum face;
    GLenum mode;
};

struct __gllc_CullFace_Rec {
    GLenum mode;
};

struct __gllc_Fogfv_Rec {
    GLenum pname;
    /* params */
};

struct __gllc_Fogiv_Rec {
    GLenum pname;
    /* params */
};

struct __gllc_FrontFace_Rec {
    GLenum mode;
};

struct __gllc_Hint_Rec {
    GLenum target;
    GLenum mode;
};

struct __gllc_Lightfv_Rec {
    GLenum light;
    GLenum pname;
    /* params */
};

struct __gllc_Lightiv_Rec {
    GLenum light;
    GLenum pname;
    /* params */
};

struct __gllc_LightModelfv_Rec {
    GLenum pname;
    /* params */
};

struct __gllc_LightModeliv_Rec {
    GLenum pname;
    /* params */
};

struct __gllc_LineStipple_Rec {
    GLint factor;
    GLushort pattern;
    GLushort pad1;
};

struct __gllc_LineWidth_Rec {
    GLfloat width;
};

struct __gllc_Materialfv_Rec {
    GLenum face;
    GLenum pname;
    /* params */
};

struct __gllc_Materialiv_Rec {
    GLenum face;
    GLenum pname;
    /* params */
};

struct __gllc_PointSize_Rec {
    GLfloat size;
};

struct __gllc_PolygonMode_Rec {
    GLenum face;
    GLenum mode;
};

struct __gllc_PolygonStipple_Rec {
    GLubyte mask[128];
};

struct __gllc_Scissor_Rec {
    GLint x;
    GLint y;
    GLsizei width;
    GLsizei height;
};

struct __gllc_ShadeModel_Rec {
    GLenum mode;
};

struct __gllc_TexParameterfv_Rec {
    GLenum target;
    GLenum pname;
    /* params */
};

struct __gllc_TexParameteriv_Rec {
    GLenum target;
    GLenum pname;
    /* params */
};

struct __gllc_TexImage1D_Rec {
    GLenum target;
    GLint level;
    GLint components;
    GLsizei width;
    GLint border;
    GLenum format;
    GLenum type;
    GLint imageSize;
    /* pixels */
};

struct __gllc_TexImage2D_Rec {
    GLenum target;
    GLint level;
    GLint components;
    GLsizei width;
    GLsizei height;
    GLint border;
    GLenum format;
    GLenum type;
    GLint imageSize;
    /* pixels */
};

struct __gllc_TexEnvfv_Rec {
    GLenum target;
    GLenum pname;
    /* params */
};

struct __gllc_TexEnviv_Rec {
    GLenum target;
    GLenum pname;
    /* params */
};

struct __gllc_TexGendv_Rec {
    GLenum coord;
    GLenum pname;
    /* params */
};

struct __gllc_TexGenfv_Rec {
    GLenum coord;
    GLenum pname;
    /* params */
};

struct __gllc_TexGeniv_Rec {
    GLenum coord;
    GLenum pname;
    /* params */
};

struct __gllc_LoadName_Rec {
    GLuint name;
};

struct __gllc_PassThrough_Rec {
    GLfloat token;
};

struct __gllc_PushName_Rec {
    GLuint name;
};

struct __gllc_DrawBuffer_Rec {
    GLenum mode;
};

struct __gllc_Clear_Rec {
    GLbitfield mask;
};

struct __gllc_ClearAccum_Rec {
    GLfloat red;
    GLfloat green;
    GLfloat blue;
    GLfloat alpha;
};

struct __gllc_ClearIndex_Rec {
    GLfloat c;
};

struct __gllc_ClearColor_Rec {
    GLclampf red;
    GLclampf green;
    GLclampf blue;
    GLclampf alpha;
};

struct __gllc_ClearStencil_Rec {
    GLint s;
};

struct __gllc_ClearDepth_Rec {
    GLclampd depth;
};

struct __gllc_StencilMask_Rec {
    GLuint mask;
};

struct __gllc_StencilMaskSeparate_Rec {
    GLenum face;
    GLuint mask;
};

struct __gllc_ColorMask_Rec {
    GLboolean red;
    GLboolean green;
    GLboolean blue;
    GLboolean alpha;
};

struct __gllc_DepthMask_Rec {
    GLboolean flag;
    GLubyte pad1;
    GLubyte pad2;
    GLubyte pad3;
};

struct __gllc_IndexMask_Rec {
    GLuint mask;
};

struct __gllc_Accum_Rec {
    GLenum op;
    GLfloat value;
};

struct __gllc_Disable_Rec {
    GLenum cap;
};

struct __gllc_Enable_Rec {
    GLenum cap;
};

struct __gllc_PushAttrib_Rec {
    GLbitfield mask;
};

struct __gllc_MapGrid1d_Rec {
    GLdouble u1;
    GLdouble u2;
    GLint un;
};

struct __gllc_MapGrid1f_Rec {
    GLint un;
    GLfloat u1;
    GLfloat u2;
};

struct __gllc_MapGrid2d_Rec {
    GLdouble u1;
    GLdouble u2;
    GLdouble v1;
    GLdouble v2;
    GLint un;
    GLint vn;
};

struct __gllc_MapGrid2f_Rec {
    GLint un;
    GLfloat u1;
    GLfloat u2;
    GLint vn;
    GLfloat v1;
    GLfloat v2;
};

struct __gllc_EvalCoord1d_Rec {
    GLdouble u;
};

struct __gllc_EvalCoord1dv_Rec {
    GLdouble u[1];
};

struct __gllc_EvalCoord1f_Rec {
    GLfloat u;
};

struct __gllc_EvalCoord1fv_Rec {
    GLfloat u[1];
};

struct __gllc_EvalCoord2d_Rec {
    GLdouble u;
    GLdouble v;
};

struct __gllc_EvalCoord2dv_Rec {
    GLdouble u[2];
};

struct __gllc_EvalCoord2f_Rec {
    GLfloat u;
    GLfloat v;
};

struct __gllc_EvalCoord2fv_Rec {
    GLfloat u[2];
};

struct __gllc_EvalMesh1_Rec {
    GLenum mode;
    GLint i1;
    GLint i2;
};

struct __gllc_EvalPoint1_Rec {
    GLint i;
};

struct __gllc_EvalMesh2_Rec {
    GLenum mode;
    GLint i1;
    GLint i2;
    GLint j1;
    GLint j2;
};

struct __gllc_EvalPoint2_Rec {
    GLint i;
    GLint j;
};

struct __gllc_AlphaFunc_Rec {
    GLenum func;
    GLclampf ref;
};

struct __gllc_BlendColor_Rec {
    GLfloat r;
    GLfloat g;
    GLfloat b;
    GLfloat a;
};

struct __gllc_BlendFunc_Rec {
    GLenum sfactor;
    GLenum dfactor;
};

struct __gllc_BlendFuncSeparate_Rec {
    GLenum sfactorRGB;
    GLenum dfactorRGB;
    GLenum sfactorAlpha;
    GLenum dfactorAlpha;
};

struct __gllc_BlendEquation_Rec {
    GLenum mode;
};

struct __gllc_BlendEquationSeparate_Rec {
    GLenum modeRGB;
    GLenum modeAlpha;
};

struct __gllc_LogicOp_Rec {
    GLenum opcode;
};

struct __gllc_StencilFunc_Rec {
    GLenum func;
    GLint ref;
    GLuint mask;
};

struct __gllc_StencilFuncSeparate_Rec {
    GLenum face;
    GLenum func;
    GLint ref;
    GLuint mask;
};

struct __gllc_StencilOp_Rec {
    GLenum fail;
    GLenum zfail;
    GLenum zpass;
};

struct __gllc_StencilOpSeparate_Rec {
    GLenum face;
    GLenum fail;
    GLenum zfail;
    GLenum zpass;
};

struct __gllc_DepthFunc_Rec {
    GLenum func;
};

struct __gllc_PixelZoom_Rec {
    GLfloat xfactor;
    GLfloat yfactor;
};

struct __gllc_PixelTransferf_Rec {
    GLenum pname;
    GLfloat param;
};

struct __gllc_PixelTransferi_Rec {
    GLenum pname;
    GLint param;
};

struct __gllc_PixelMapfv_Rec {
    GLenum map;
    GLint mapsize;
    /* values */
};

struct __gllc_PixelMapuiv_Rec {
    GLenum map;
    GLint mapsize;
    /* values */
};

struct __gllc_PixelMapusv_Rec {
    GLenum map;
    GLint mapsize;
    /* values */
};

struct __gllc_ReadBuffer_Rec {
    GLenum mode;
};

struct __gllc_CopyPixels_Rec {
    GLint x;
    GLint y;
    GLsizei width;
    GLsizei height;
    GLenum type;
};

struct __gllc_DrawPixels_Rec {
    GLsizei width;
    GLsizei height;
    GLenum format;
    GLenum type;
    GLint imageSize;
    /* pixels */
};

struct __gllc_Bitmap_Rec
{
    GLsizei width;
    GLsizei height;
    GLfloat xorig;
    GLfloat yorig;
    GLfloat xmove;
    GLfloat ymove;
    GLint imageSize;
    /* bitmaps */
};

struct __gllc_DepthRange_Rec {
    GLclampd zNear;
    GLclampd zFar;
};

struct __gllc_DepthBoundTest_Rec {
    GLclampd zMin;
    GLclampd zMax;
};


struct __gllc_Frustum_Rec {
    GLdouble left;
    GLdouble right;
    GLdouble bottom;
    GLdouble top;
    GLdouble zNear;
    GLdouble zFar;
};

struct __gllc_LoadMatrixf_Rec {
    GLfloat m[16];
};

struct __gllc_LoadMatrixd_Rec {
    GLdouble m[16];
};

struct __gllc_MatrixMode_Rec {
    GLenum mode;
};

struct __gllc_MultMatrixf_Rec {
    GLfloat m[16];
};

struct __gllc_MultMatrixd_Rec {
    GLdouble m[16];
};

struct __gllc_Ortho_Rec {
    GLdouble left;
    GLdouble right;
    GLdouble bottom;
    GLdouble top;
    GLdouble zNear;
    GLdouble zFar;
};

struct __gllc_Rotated_Rec {
    GLdouble angle;
    GLdouble x;
    GLdouble y;
    GLdouble z;
};

struct __gllc_Rotatef_Rec {
    GLfloat angle;
    GLfloat x;
    GLfloat y;
    GLfloat z;
};

struct __gllc_Scaled_Rec {
    GLdouble x;
    GLdouble y;
    GLdouble z;
};

struct __gllc_Scalef_Rec {
    GLfloat x;
    GLfloat y;
    GLfloat z;
};

struct __gllc_Translated_Rec {
    GLdouble x;
    GLdouble y;
    GLdouble z;
};

struct __gllc_Translatef_Rec {
    GLfloat x;
    GLfloat y;
    GLfloat z;
};

struct __gllc_Viewport_Rec {
    GLint x;
    GLint y;
    GLsizei width;
    GLsizei height;
};

struct __gllc_PolygonOffset_Rec {
    GLfloat factor;
    GLfloat units;
};

struct __gllc_CopyTexImage1D_Rec {
    GLenum target;
    GLint level;
    GLenum internalformat;
    GLint x;
    GLint y;
    GLsizei width;
    GLint border;
};

struct __gllc_CopyTexImage2D_Rec {
    GLenum target;
    GLint level;
    GLenum internalformat;
    GLint x;
    GLint y;
    GLsizei width;
    GLsizei height;
    GLint border;
};

struct __gllc_CopyTexSubImage1D_Rec {
    GLenum target;
    GLint level;
    GLint xoffset;
    GLint x;
    GLint y;
    GLsizei width;
};

struct __gllc_CopyTexSubImage2D_Rec {
    GLenum target;
    GLint level;
    GLint xoffset;
    GLint yoffset;
    GLint x;
    GLint y;
    GLsizei width;
    GLsizei height;
};

struct __gllc_TexSubImage1D_Rec {
    GLenum target;
    GLint level;
    GLint xoffset;
    GLsizei width;
    GLenum format;
    GLenum type;
    GLint imageSize;
    /* pixels */
};

struct __gllc_TexSubImage2D_Rec {
    GLenum target;
    GLint level;
    GLint xoffset;
    GLint yoffset;
    GLsizei width;
    GLsizei height;
    GLenum format;
    GLenum type;
    GLint imageSize;
    /* pixels */
};

struct __gllc_BindTexture_Rec {
    GLenum target;
    GLuint texture;
};

struct __gllc_PrioritizeTextures_Rec {
    GLsizei n;
    /* textures */
    /* priorities */
};

struct __gllc_ActiveTexture_Rec
{
    GLenum texture;
};

struct __gllc_MultiTexCoord2fv_Rec {
    GLenum texture;
    GLfloat v[2];
};

struct __gllc_MultiTexCoord3fv_Rec {
    GLenum texture;
    GLfloat v[3];
};

struct __gllc_MultiTexCoord4fv_Rec {
    GLenum texture;
    GLfloat v[4];
};

struct __gllc_FogCoordf_Rec {
    GLfloat coord;
};

struct __gllc_CompressedTexImage2D_Rec {
    GLenum target;
    GLint lod;
    GLint components;
    GLsizei width;
    GLsizei height;
    GLint border;
    GLsizei imageSize;
};

struct __gllc_CompressedTexSubImage2D_Rec {
    GLenum target;
    GLint lod;
    GLint xoffset;
    GLint yoffset;
    GLsizei width;
    GLsizei height;
    GLenum format;
    GLsizei imageSize;
};

struct __gllc_ColorTable_Rec {
    GLenum target;
    GLenum internalformat;
    GLsizei width;
    GLenum format;
    GLenum type;
    GLsizei imageSize;
    const GLvoid *table;
};

struct __gllc_ColorSubTable_Rec {
    GLenum target;
    GLsizei start;
    GLsizei count;
    GLenum format;
    GLenum type;
    GLsizei imageSize;
    const GLvoid *table;
};

struct __gllc_CopyColorTable_Rec {
    GLenum target;
    GLenum internalformat;
    GLint x;
    GLint y;
    GLsizei width;
};

struct __gllc_CopyColorSubTable_Rec {
    GLenum target;
    GLsizei start;
    GLint x;
    GLint y;
    GLsizei width;
};

struct __gllc_ColorTableParameteriv_Rec {
    GLenum target;
    GLenum pname;
};

struct __gllc_ColorTableParameterfv_Rec {
    GLenum target;
    GLenum pname;
};

struct __gllc_ConvolutionFilter1D_Rec {
    GLenum target;
    GLenum internalformat;
    GLsizei width;
    GLenum format;
    GLenum type;
    GLsizei imageSize;
    const GLvoid *image;
};

struct __gllc_ConvolutionFilter2D_Rec {
    GLenum target;
    GLenum internalformat;
    GLsizei width;
    GLsizei height;
    GLenum format;
    GLenum type;
    const GLvoid *image;
    GLsizei imageSize;
};

struct __gllc_SeparableFilter2D_Rec {
    GLenum target;
    GLenum internalformat;
    GLsizei width;
    GLsizei height;
    GLenum format;
    GLenum type;
    const GLvoid *row;
    const GLvoid *col;
    GLsizei rowSize;
    GLsizei colSize;
};

struct __gllc_CopyConvolutionFilter1D_Rec {
    GLenum target;
    GLenum internalformat;
    GLint x;
    GLint y;
    GLsizei width;
};

struct __gllc_CopyConvolutionFilter2D_Rec {
    GLenum target;
    GLenum internalformat;
    GLint x;
    GLint y;
    GLsizei width;
    GLsizei height;
};

struct __gllc_ConvolutionParameteriv_Rec {
    GLenum target;
    GLenum pname;
};

struct __gllc_ConvolutionParameterfv_Rec {
    GLenum target;
    GLenum pname;
};

struct __gllc_Histogram_Rec {
    GLenum target;
    GLsizei width;
    GLenum internalformat;
    GLboolean sink;
};

struct __gllc_ResetHistogram_Rec {
    GLenum target;
};

struct __gllc_Minmax_Rec {
    GLenum target;
    GLenum internalFormat;
    GLboolean sink;
};

struct __gllc_ResetMinmax_Rec {
    GLenum target;
};

struct __gllc_TexImage3D_Rec {
    GLenum target;
    GLint lod;
    GLint components;
    GLsizei width;
    GLsizei height;
    GLsizei depth;
    GLint border;
    GLenum format;
    GLenum type;
    GLint imageSize;
    /* pixels */
};

struct __gllc_TexSubImage3D_Rec {
    GLenum target;
    GLint lod;
    GLint xoffset;
    GLint yoffset;
    GLint zoffset;
    GLsizei width;
    GLsizei height;
    GLsizei depth;
    GLenum format;
    GLenum type;
    GLint imageSize;
    /* pixels */
};

struct __gllc_CopyTexSubImage3D_Rec {
    GLenum target;
    GLint level;
    GLint xoffset;
    GLint yoffset;
    GLint zoffset;
    GLint x;
    GLint y;
    GLsizei width;
    GLsizei height;
};

struct __gllc_PointParameterfv_Rec {
    GLenum pname;
    GLint paramSize;
    /* params */
};

struct __gllc_PointParameteriv_Rec {
    GLenum pname;
    GLint paramSize;
    /* params */
};

struct __gllc_SampleCoverage_Rec {
    GLclampf v;
    GLboolean invert;
    GLubyte pad1;
    GLubyte pad2;
    GLubyte pad3;
};

struct __gllc_WindowPos2fv_Rec {
    GLfloat v[2];
};

struct __gllc_WindowPos3fv_Rec {
    GLfloat v[3];
};

struct __gllc_VertexAttrib4fv_Rec {
    GLuint index;
    union {
        GLfloat v[4];
        GLint iv[4];
        GLuint uiv[4];
    };
};

struct __gllc_BeginQuery_Rec {
    GLenum target;
    GLuint id;
};

struct __gllc_EndQuery_Rec {
    GLenum target;
};

struct __gllc_Uniform4f_Rec {
    GLint location;
    GLfloat x;
    GLfloat y;
    GLfloat z;
    GLfloat w;
};

struct __gllc_Uniform3f_Rec {
    GLint location;
    GLfloat x;
    GLfloat y;
    GLfloat z;
};

struct __gllc_Uniform2f_Rec {
    GLint location;
    GLfloat x;
    GLfloat y;
};

struct __gllc_Uniform1f_Rec {
    GLint location;
    GLfloat x;
};

struct __gllc_Uniform4i_Rec {
    GLint location;
    GLint x;
    GLint y;
    GLint z;
    GLint w;
};

struct __gllc_Uniform3i_Rec {
    GLint location;
    GLint x;
    GLint y;
    GLint z;
};

struct __gllc_Uniform2i_Rec {
    GLint location;
    GLint x;
    GLint y;
};

struct __gllc_Uniform1i_Rec {
    GLint location;
    GLint x;
};

struct __gllc_Uniform4fv_Rec {
    GLint location;
    GLsizei count;
    /* data */
};

struct __gllc_Uniform3fv_Rec {
    GLint location;
    GLsizei count;
    /* data */
};

struct __gllc_Uniform2fv_Rec {
    GLint location;
    GLsizei count;
    /* data */
};

struct __gllc_Uniform1fv_Rec {
    GLint location;
    GLsizei count;
    /* data */
};

struct __gllc_Uniform4iv_Rec {
    GLint location;
    GLsizei count;
    /* data */
};

struct __gllc_UniformMatrix4fv_Rec {
    GLint location;
    GLsizei count;
    GLboolean transpose;
    GLubyte pad[3];
    /* data */
};

struct __gllc_UniformMatrix3fv_Rec {
    GLint location;
    GLsizei count;
    GLboolean transpose;
    GLubyte pad[3];
    /* data */
};

struct __gllc_UniformMatrix2fv_Rec {
    GLint location;
    GLsizei count;
    GLboolean transpose;
    GLubyte pad[3];
    /* data */
};

struct __gllc_UniformMatrix2x3fv_Rec {
    GLint location;
    GLsizei count;
    GLboolean transpose;
    GLubyte pad[3];
    /* data */
};

struct __gllc_UniformMatrix2x4fv_Rec {
    GLint location;
    GLsizei count;
    GLboolean transpose;
    GLubyte pad[3];
    /* data */
};
struct __gllc_UniformMatrix3x2fv_Rec {
    GLint location;
    GLsizei count;
    GLboolean transpose;
    GLubyte pad[3];
    /* data */
};

struct __gllc_UniformMatrix3x4fv_Rec {
    GLint location;
    GLsizei count;
    GLboolean transpose;
    GLubyte pad[3];
    /* data */
};

struct __gllc_UniformMatrix4x2fv_Rec {
    GLint location;
    GLsizei count;
    GLboolean transpose;
    GLubyte pad[3];
    /* data */
};

struct __gllc_UniformMatrix4x3fv_Rec {
    GLint location;
    GLsizei count;
    GLboolean transpose;
    GLubyte pad[3];
    /* data */
};


struct __gllc_Uniform3iv_Rec {
    GLint location;
    GLsizei count;
    /* data */
};

struct __gllc_Uniform2iv_Rec {
    GLint location;
    GLsizei count;
    /* data */
};

struct __gllc_Uniform1iv_Rec {
    GLint location;
    GLsizei count;
    /* data */
};

struct __gllc_DrawBuffers_Rec {
    GLsizei count;
    /* data */
};

struct __gllc_UseProgram_Rec {
    GLuint program;
};

#if GL_ARB_vertex_program

struct __gllc_BindProgramARB_Rec {
    GLenum target;
    GLuint program;
};

struct __gllc_ProgramEnvParameter4dARB_Rec {
    GLenum target;
    GLuint index;
    GLdouble x, y, z, w;
};

struct __gllc_ProgramEnvParameter4dvARB_Rec {
    GLenum target;
    GLuint index;
    GLdouble v[4];
};

struct __gllc_ProgramEnvParameter4fARB_Rec {
    GLenum target;
    GLuint index;
    GLfloat x, y, z, w;
};

struct __gllc_ProgramEnvParameter4fvARB_Rec {
    GLenum target;
    GLuint index;
    GLfloat v[4];
};

#endif

#if GL_ATI_element_array
struct __gllc_DrawElementArrayATI_Rec {
    GLenum mode;
    GLsizei count;
};

struct  __gllc_DrawRangeElementArrayATI_Rec {
    GLenum mode;
    GLuint start;
    GLuint end;
    GLsizei count;
};
#endif

#if GL_EXT_stencil_two_side
struct  __gllc_ActiveStencilFaceEXT_Rec {
    GLenum face;
};
#endif

#if GL_EXT_texture_integer
struct __gllc_ClearColorIiEXT_Rec {
    GLint red;
    GLint green;
    GLint blue;
    GLint alpha;
};

struct __gllc_ClearColorIuiEXT_Rec {
    GLuint red;
    GLuint green;
    GLuint blue;
    GLuint alpha;
};

struct __gllc_TexParameterIivEXT_Rec {
    GLenum target;
    GLenum pname;
    /* params */
};

struct __gllc_TexParameterIuivEXT_Rec {
    GLenum target;
    GLenum pname;
    /* params */
};
#endif

#if GL_EXT_gpu_shader4

struct __gllc_Uniform1uiEXT_Rec {
    GLint location;
    GLuint v0;
};

struct __gllc_Uniform2uiEXT_Rec {
    GLint location;
    GLuint v0;
    GLuint v1;
};

struct __gllc_Uniform3uiEXT_Rec {
    GLint location;
    GLuint v0;
    GLuint v1;
    GLuint v2;
};

struct __gllc_Uniform4uiEXT_Rec {
    GLint location;
    GLuint v0;
    GLuint v1;
    GLuint v2;
    GLuint v3;
};

struct __gllc_Uniform1uivEXT_Rec {
    GLint location;
    GLsizei count;
    /* value */
};

struct __gllc_Uniform2uivEXT_Rec {
    GLint location;
    GLsizei count;
    /* value */
};

struct __gllc_Uniform3uivEXT_Rec {
    GLint location;
    GLsizei count;
    /* value */
};

struct __gllc_Uniform4uivEXT_Rec {
    GLint location;
    GLsizei count;
    /* value */
};

#endif

#if GL_EXT_geometry_shader4
struct __gllc_FramebufferTextureEXT_Rec {
    GLenum target;
    GLenum attachment;
    GLuint texture;
    GLint level;
};

struct __gllc_FramebufferTextureLayerEXT_Rec {
    GLenum target;
    GLenum attachment;
    GLuint texture;
    GLint level;
    GLint layer;
};

struct __gllc_FramebufferTextureFaceEXT_Rec {
    GLenum target;
    GLenum attachment;
    GLuint texture;
    GLint level;
    GLenum face;
};
#endif

#if GL_EXT_draw_buffers2

struct __gllc_ColorMaskIndexedEXT_Rec {
    GLuint buf;
    GLboolean r;
    GLboolean g;
    GLboolean b;
    GLboolean a;
};
struct __gllc_EnableIndexedEXT_Rec {
    GLenum target;
    GLuint index;
};

struct __gllc_DisableIndexedEXT_Rec {
    GLenum target;
    GLuint index;
};
#endif

#if GL_EXT_gpu_program_parameters
struct __gllc_ProgramEnvParameters4fvEXT_Rec {
    GLenum target;
    GLuint index;
    GLsizei count;
    /* params; */
};

struct __gllc_ProgramLocalParameters4fvEXT_Rec {
    GLenum target;
    GLuint index;
    GLsizei count;
    /* params; */
};
#endif

#if GL_ARB_color_buffer_float
struct __gllc_ClampColorARB_Rec {
    GLenum target;
    GLenum clamp;
};
#endif

#if GL_ATI_separate_stencil
struct __gllc_StencilFuncSeparateATI_Rec {
    GLenum frontfunc;
    GLenum backfunc;
    GLint  ref;
    GLuint mask;
};
#endif


extern GLint __glEvalComputeK(GLenum target);
extern GLint __glMap1_size(GLint k, GLint order);
extern GLint __glMap2_size(GLint k, GLint majorOrder, GLint minorOrder);
extern GLint __glFog_size(GLenum pname);
extern GLint __glLight_size(GLenum pname);
extern GLint __glLightModel_size(GLenum pname);
extern GLint __glMaterial_size(GLenum pname);
extern GLint __glTexParameter_size(GLenum e);
extern GLint __glTexEnv_size(GLenum e);
extern GLint __glTexGen_size(GLenum e);
extern GLint __glConvolutionParameter_size(GLenum pname);
extern GLint __glColorTableParameter_size(GLenum pname);
extern GLint __glPointParameter_size(GLenum pname);
extern GLenum __glErrorCheckMaterial(GLenum face, GLenum p, GLfloat pv0);

#endif /* __g_lcomp_h_ */
