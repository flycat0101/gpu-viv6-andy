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


__declspec(naked)
GLvoid APIENTRY __glVIV_NewList(GLuint list, GLenum mode)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_NewList*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_EndList(GLvoid)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_EndList*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_CallList(GLuint list)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_CallList*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_CallLists(GLsizei n, GLenum type, const GLvoid *lists)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_CallLists*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_DeleteLists(GLuint list, GLsizei range)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_DeleteLists*SZPTR]
}

__declspec(naked)
GLuint APIENTRY __glVIV_GenLists(GLsizei range)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GenLists*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ListBase(GLuint base)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ListBase*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Begin(GLenum mode)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Begin*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Bitmap(GLsizei width, GLsizei height,
    GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Bitmap*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color3b(GLbyte red, GLbyte green, GLbyte blue)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color3b*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color3bv(const GLbyte *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color3bv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color3d(GLdouble red, GLdouble green, GLdouble blue)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color3d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color3dv(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color3dv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color3f(GLfloat red, GLfloat green, GLfloat blue)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color3f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color3fv(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color3fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color3i(GLint red, GLint green, GLint blue)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color3i*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color3iv(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color3iv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color3s(GLshort red, GLshort green, GLshort blue)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color3s*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color3sv(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color3sv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color3ub(GLubyte red, GLubyte green, GLubyte blue)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color3ub*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color3ubv(const GLubyte *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color3ubv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color3ui(GLuint red, GLuint green, GLuint blue)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color3ui*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color3uiv(const GLuint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color3uiv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color3us(GLushort red, GLushort green, GLushort blue)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color3us*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color3usv(const GLushort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color3usv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color4b*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color4bv(const GLbyte *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color4bv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color4d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color4dv(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color4dv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color4f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color4fv(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color4fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color4i(GLint red, GLint green, GLint blue, GLint alpha)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color4i*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color4iv(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color4iv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color4s(GLshort red, GLshort green, GLshort blue, GLshort alpha)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color4s*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color4sv(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color4sv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color4ub*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color4ubv(const GLubyte *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color4ubv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color4ui*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color4uiv(const GLuint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color4uiv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color4us(GLushort red, GLushort green, GLushort blue, GLushort alpha)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color4us*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Color4usv(const GLushort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Color4usv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_EdgeFlag(GLboolean flag)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_EdgeFlag*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_EdgeFlagv(const GLboolean *flag)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_EdgeFlagv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_End(GLvoid)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_End*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Indexd(GLdouble c)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Indexd*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Indexdv(const GLdouble *c)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Indexdv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Indexf(GLfloat c)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Indexf*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Indexfv(const GLfloat *c)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Indexfv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Indexi(GLint c)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Indexi*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Indexiv(const GLint *c)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Indexiv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Indexs(GLshort c)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Indexs*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Indexsv(const GLshort *c)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Indexsv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Normal3b(GLbyte nx, GLbyte ny, GLbyte nz)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Normal3b*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Normal3bv(const GLbyte *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Normal3bv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Normal3d(GLdouble nx, GLdouble ny, GLdouble nz)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Normal3d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Normal3dv(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Normal3dv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Normal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Normal3f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Normal3fv(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Normal3fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Normal3i(GLint nx, GLint ny, GLint nz)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Normal3i*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Normal3iv(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Normal3iv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Normal3s(GLshort nx, GLshort ny, GLshort nz)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Normal3s*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Normal3sv(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Normal3sv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_RasterPos2d(GLdouble x, GLdouble y)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_RasterPos2d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_RasterPos2dv(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_RasterPos2dv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_RasterPos2f(GLfloat x, GLfloat y)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_RasterPos2f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_RasterPos2fv(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_RasterPos2fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_RasterPos2i(GLint x, GLint y)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_RasterPos2i*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_RasterPos2iv(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_RasterPos2iv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_RasterPos2s(GLshort x, GLshort y)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_RasterPos2s*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_RasterPos2sv(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_RasterPos2sv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_RasterPos3d(GLdouble x, GLdouble y, GLdouble z)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_RasterPos3d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_RasterPos3dv(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_RasterPos3dv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_RasterPos3f(GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_RasterPos3f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_RasterPos3fv(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_RasterPos3fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_RasterPos3i(GLint x, GLint y, GLint z)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_RasterPos3i*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_RasterPos3iv(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_RasterPos3iv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_RasterPos3s(GLshort x, GLshort y, GLshort z)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_RasterPos3s*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_RasterPos3sv(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_RasterPos3sv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_RasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_RasterPos4d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_RasterPos4dv(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_RasterPos4dv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_RasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_RasterPos4f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_RasterPos4fv(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_RasterPos4fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_RasterPos4i(GLint x, GLint y, GLint z, GLint w)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_RasterPos4i*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_RasterPos4iv(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_RasterPos4iv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_RasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_RasterPos4s*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_RasterPos4sv(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_RasterPos4sv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Rectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Rectd*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Rectdv(const GLdouble *v1, const GLdouble *v2)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Rectdv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Rectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Rectf*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Rectfv(const GLfloat *v1, const GLfloat *v2)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Rectfv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Recti(GLint x1, GLint y1, GLint x2, GLint y2)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Recti*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Rectiv(const GLint *v1, const GLint *v2)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Rectiv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Rects(GLshort x1, GLshort y1, GLshort x2, GLshort y2)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Rects*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Rectsv(const GLshort *v1, const GLshort *v2)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Rectsv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord1d(GLdouble s)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord1d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord1dv(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord1dv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord1f(GLfloat s)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord1f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord1fv(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord1fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord1i(GLint s)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord1i*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord1iv(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord1iv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord1s(GLshort s)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord1s*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord1sv(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord1sv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord2d(GLdouble s, GLdouble t)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord2d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord2dv(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord2dv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord2f(GLfloat s, GLfloat t)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord2f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord2fv(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord2fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord2i(GLint s, GLint t)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord2i*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord2iv(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord2iv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord2s(GLshort s, GLshort t)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord2s*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord2sv(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord2sv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord3d(GLdouble s, GLdouble t, GLdouble r)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord3d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord3dv(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord3dv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord3f(GLfloat s, GLfloat t, GLfloat r)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord3f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord3fv(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord3fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord3i(GLint s, GLint t, GLint r)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord3i*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord3iv(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord3iv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord3s(GLshort s, GLshort t, GLshort r)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord3s*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord3sv(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord3sv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord4d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord4dv(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord4dv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord4f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord4fv(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord4fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord4i(GLint s, GLint t, GLint r, GLint q)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord4i*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord4iv(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord4iv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord4s*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoord4sv(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoord4sv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Vertex2d(GLdouble x, GLdouble y)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Vertex2d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Vertex2dv(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Vertex2dv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Vertex2f(GLfloat x, GLfloat y)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Vertex2f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Vertex2fv(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Vertex2fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Vertex2i(GLint x, GLint y)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Vertex2i*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Vertex2iv(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Vertex2iv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Vertex2s(GLshort x, GLshort y)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Vertex2s*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Vertex2sv(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Vertex2sv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Vertex3d(GLdouble x, GLdouble y, GLdouble z)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Vertex3d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Vertex3dv(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Vertex3dv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Vertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Vertex3f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Vertex3fv(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Vertex3fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Vertex3i(GLint x, GLint y, GLint z)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Vertex3i*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Vertex3iv(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Vertex3iv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Vertex3s(GLshort x, GLshort y, GLshort z)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Vertex3s*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Vertex3sv(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Vertex3sv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Vertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Vertex4d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Vertex4dv(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Vertex4dv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Vertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Vertex4f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Vertex4fv(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Vertex4fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Vertex4i(GLint x, GLint y, GLint z, GLint w)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Vertex4i*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Vertex4iv(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Vertex4iv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Vertex4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Vertex4s*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Vertex4sv(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Vertex4sv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ClipPlane(GLenum plane, const GLdouble *equation)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ClipPlane*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ColorMaterial(GLenum face, GLenum mode)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ColorMaterial*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_CullFace(GLenum mode)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_CullFace*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Fogf(GLenum pname, GLfloat param)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Fogf*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Fogfv(GLenum pname, const GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Fogfv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Fogi(GLenum pname, GLint param)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Fogi*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Fogiv(GLenum pname, const GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Fogiv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_FrontFace(GLenum mode)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_FrontFace*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Hint(GLenum target, GLenum mode)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Hint*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Lightf(GLenum light, GLenum pname, GLfloat param)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Lightf*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Lightfv(GLenum light, GLenum pname, const GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Lightfv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Lighti(GLenum light, GLenum pname, GLint param)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Lighti*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Lightiv(GLenum light, GLenum pname, const GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Lightiv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_LightModelf(GLenum pname, GLfloat param)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_LightModelf*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_LightModelfv(GLenum pname, const GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_LightModelfv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_LightModeli(GLenum pname, GLint param)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_LightModeli*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_LightModeliv(GLenum pname, const GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_LightModeliv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_LineStipple(GLint factor, GLushort pattern)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_LineStipple*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_LineWidth(GLfloat width)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_LineWidth*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Materialf(GLenum face, GLenum pname, GLfloat param)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Materialf*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Materialfv(GLenum face, GLenum pname, const GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Materialfv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Materiali(GLenum face, GLenum pname, GLint param)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Materiali*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Materialiv(GLenum face, GLenum pname, const GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Materialiv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_PointSize(GLfloat size)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_PointSize*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_PolygonMode(GLenum face, GLenum mode)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_PolygonMode*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_PolygonStipple(const GLubyte *mask)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_PolygonStipple*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Scissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Scissor*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ShadeModel(GLenum mode)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ShadeModel*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexParameterf(GLenum target, GLenum pname, GLfloat param)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexParameterf*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexParameterfv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexParameteri(GLenum target, GLenum pname, GLint param)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexParameteri*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexParameteriv(GLenum target, GLenum pname, const GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexParameteriv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexImage1D(GLenum target, GLint level, GLint components, GLsizei width,
                                GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexImage1D*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexImage2D(GLenum target, GLint level, GLint components, GLsizei width, GLsizei height,
                                GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexImage2D*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexEnvf(GLenum target, GLenum pname, GLfloat param)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexEnvf*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexEnvfv(GLenum target, GLenum pname, const GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexEnvfv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexEnvi(GLenum target, GLenum pname, GLint param)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexEnvi*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexEnviv(GLenum target, GLenum pname, const GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexEnviv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexGend(GLenum coord, GLenum pname, GLdouble param)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexGend*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexGendv(GLenum coord, GLenum pname, const GLdouble *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexGendv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexGenf(GLenum coord, GLenum pname, GLfloat param)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexGenf*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexGenfv(GLenum coord, GLenum pname, const GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexGenfv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexGeni(GLenum coord, GLenum pname, GLint param)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexGeni*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexGeniv(GLenum coord, GLenum pname, const GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexGeniv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_FeedbackBuffer(GLsizei size, GLenum type, GLfloat *buffer)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_FeedbackBuffer*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_SelectBuffer(GLsizei size, GLuint *buffer)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_SelectBuffer*SZPTR]
}

__declspec(naked)
GLint APIENTRY __glVIV_RenderMode(GLenum mode)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_RenderMode*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_InitNames(GLvoid)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_InitNames*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_LoadName(GLuint name)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_LoadName*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_PassThrough(GLfloat token)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_PassThrough*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_PopName(GLvoid)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_PopName*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_PushName(GLuint name)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_PushName*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_DrawBuffer(GLenum mode)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_DrawBuffer*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Clear(GLbitfield mask)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Clear*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ClearAccum*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ClearIndex(GLfloat c)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ClearIndex*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ClearColor*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ClearStencil(GLint s)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ClearStencil*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ClearDepth(GLclampd depth)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ClearDepth*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_StencilMask(GLuint mask)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_StencilMask*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ColorMask*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_DepthMask(GLboolean flag)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_DepthMask*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_IndexMask(GLuint mask)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_IndexMask*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Accum(GLenum op, GLfloat value)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Accum*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Disable(GLenum cap)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Disable*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Enable(GLenum cap)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Enable*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Finish(GLvoid)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Finish*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Flush(GLvoid)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Flush*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_PopAttrib(GLvoid)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_PopAttrib*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_PushAttrib(GLbitfield mask)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_PushAttrib*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Map1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Map1d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Map1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Map1f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Map2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder,
                           GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Map2d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Map2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder,
                           GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Map2f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MapGrid1d(GLint un, GLdouble u1, GLdouble u2)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MapGrid1d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MapGrid1f(GLint un, GLfloat u1, GLfloat u2)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MapGrid1f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MapGrid2d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MapGrid2f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_EvalCoord1d(GLdouble u)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_EvalCoord1d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_EvalCoord1dv(const GLdouble *u)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_EvalCoord1dv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_EvalCoord1f(GLfloat u)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_EvalCoord1f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_EvalCoord1fv(const GLfloat *u)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_EvalCoord1fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_EvalCoord2d(GLdouble u, GLdouble v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_EvalCoord2d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_EvalCoord2dv(const GLdouble *u)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_EvalCoord2dv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_EvalCoord2f(GLfloat u, GLfloat v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_EvalCoord2f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_EvalCoord2fv(const GLfloat *u)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_EvalCoord2fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_EvalMesh1(GLenum mode, GLint i1, GLint i2)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_EvalMesh1*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_EvalPoint1(GLint i)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_EvalPoint1*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_EvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_EvalMesh2*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_EvalPoint2(GLint i, GLint j)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_EvalPoint2*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_AlphaFunc(GLenum func, GLclampf ref)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_AlphaFunc*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_BlendFunc(GLenum sfactor, GLenum dfactor)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_BlendFunc*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_LogicOp(GLenum opcode)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_LogicOp*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_StencilFunc(GLenum func, GLint ref, GLuint mask)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_StencilFunc*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_StencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_StencilOp*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_DepthFunc(GLenum func)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_DepthFunc*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_PixelZoom(GLfloat xfactor, GLfloat yfactor)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_PixelZoom*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_PixelTransferf(GLenum pname, GLfloat param)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_PixelTransferf*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_PixelTransferi(GLenum pname, GLint param)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_PixelTransferi*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_PixelStoref(GLenum pname, GLfloat param)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_PixelStoref*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_PixelStorei(GLenum pname, GLint param)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_PixelStorei*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_PixelMapfv(GLenum map, GLint mapsize, const GLfloat *values)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_PixelMapfv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_PixelMapuiv(GLenum map, GLint mapsize, const GLuint *values)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_PixelMapuiv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_PixelMapusv(GLenum map, GLint mapsize, const GLushort *values)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_PixelMapusv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ReadBuffer(GLenum mode)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ReadBuffer*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_CopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_CopyPixels*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ReadPixels*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_DrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_DrawPixels*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetBooleanv(GLenum pname, GLboolean *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetBooleanv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetClipPlane(GLenum plane, GLdouble *equation)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetClipPlane*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetDoublev(GLenum pname, GLdouble *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetDoublev*SZPTR]
}

__declspec(naked)
GLenum APIENTRY __glVIV_GetError(GLvoid)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetError*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetFloatv(GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetFloatv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetIntegerv(GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetIntegerv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetLightfv(GLenum light, GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetLightfv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetLightiv(GLenum light, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetLightiv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetMapdv(GLenum target, GLenum query, GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetMapdv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetMapfv(GLenum target, GLenum query, GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetMapfv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetMapiv(GLenum target, GLenum query, GLint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetMapiv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetMaterialfv(GLenum face, GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetMaterialfv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetMaterialiv(GLenum face, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetMaterialiv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetPixelMapfv(GLenum map, GLfloat *values)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetPixelMapfv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetPixelMapuiv(GLenum map, GLuint *values)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetPixelMapuiv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetPixelMapusv(GLenum map, GLushort *values)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetPixelMapusv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetPolygonStipple(GLubyte *mask)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetPolygonStipple*SZPTR]
}

__declspec(naked)
const GLubyte * APIENTRY __glVIV_GetString(GLenum name)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetString*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetTexEnvfv(GLenum target, GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetTexEnvfv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetTexEnviv(GLenum target, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetTexEnviv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetTexGendv(GLenum coord, GLenum pname, GLdouble *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetTexGendv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetTexGenfv(GLenum coord, GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetTexGenfv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetTexGeniv(GLenum coord, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetTexGeniv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetTexImage*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetTexParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetTexParameterfv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetTexParameteriv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetTexLevelParameterfv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetTexLevelParameteriv*SZPTR]
}

__declspec(naked)
GLboolean APIENTRY __glVIV_IsEnabled(GLenum cap)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_IsEnabled*SZPTR]
}

__declspec(naked)
GLboolean APIENTRY __glVIV_IsList(GLuint list)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_IsList*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_DepthRange(GLclampd zNear, GLclampd zFar)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_DepthRange*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Frustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Frustum*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_LoadIdentity(GLvoid)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_LoadIdentity*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_LoadMatrixf(const GLfloat *m)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_LoadMatrixf*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_LoadMatrixd(const GLdouble *m)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_LoadMatrixd*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MatrixMode(GLenum mode)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MatrixMode*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultMatrixf(const GLfloat *m)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultMatrixf*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultMatrixd(const GLdouble *m)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultMatrixd*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Ortho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Ortho*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_PopMatrix(GLvoid)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_PopMatrix*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_PushMatrix(GLvoid)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_PushMatrix*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Rotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Rotated*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Rotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Rotatef*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Scaled(GLdouble x, GLdouble y, GLdouble z)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Scaled*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Scalef(GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Scalef*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Translated(GLdouble x, GLdouble y, GLdouble z)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Translated*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Translatef(GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Translatef*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Viewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Viewport*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ArrayElement(GLint i)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ArrayElement*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ColorPointer*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_DrawArrays(GLenum mode, GLint first, GLsizei count)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_DrawArrays*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_EdgeFlagPointer(GLsizei stride, const GLboolean *pointer)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_EdgeFlagPointer*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetPointerv(GLenum pname, GLvoid* *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetPointerv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_IndexPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_IndexPointer*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_NormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_NormalPointer*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexCoordPointer*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexPointer*SZPTR]
}


__declspec(naked)
GLvoid APIENTRY __glVIV_DisableClientState(GLenum array)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_DisableClientState*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_DrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_DrawElements*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_EnableClientState(GLenum array)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_EnableClientState*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_InterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_InterleavedArrays*SZPTR]
}


__declspec(naked)
GLvoid APIENTRY __glVIV_PolygonOffset(GLfloat factor, GLfloat units)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_PolygonOffset*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_CopyTexImage1D(GLenum target, GLint level, GLenum internalformat,
                                    GLint x, GLint y, GLsizei width, GLint border)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_CopyTexImage1D*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_CopyTexImage2D(GLenum target, GLint level, GLenum internalformat,
                                    GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_CopyTexImage2D*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_CopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_CopyTexSubImage1D*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_CopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset,
                                       GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_CopyTexSubImage2D*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexSubImage1D(GLenum target, GLint level, GLint xoffset,
                                   GLsizei width, GLenum format, GLenum type, const GLvoid *pixels)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexSubImage1D*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset,
                                   GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexSubImage2D*SZPTR]
}

__declspec(naked)
GLboolean APIENTRY __glVIV_AreTexturesResident(GLsizei n, const GLuint *textures, GLboolean *residences)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_AreTexturesResident*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_BindTexture(GLenum target, GLuint texture)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_BindTexture*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_DeleteTextures(GLsizei n, const GLuint *textures)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_DeleteTextures*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GenTextures(GLsizei n, GLuint *textures)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GenTextures*SZPTR]
}

__declspec(naked)
GLboolean APIENTRY __glVIV_IsTexture(GLuint texture)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_IsTexture*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_PrioritizeTextures(GLsizei n, const GLuint *textures, const GLclampf *priorities)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_PrioritizeTextures*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Indexub(GLubyte c)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Indexub*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Indexubv(const GLubyte *c)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Indexubv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_PopClientAttrib(GLvoid)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_PopClientAttrib*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_PushClientAttrib(GLbitfield mask)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_PushClientAttrib*SZPTR]
}


#if GL_VERSION_1_2

__declspec(naked)
GLvoid APIENTRY __glVIV_BlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_BlendColor*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_BlendEquation(GLenum mode)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_BlendEquation*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_DrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_DrawRangeElements*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ColorTable(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ColorTable*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ColorTableParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ColorTableParameterfv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ColorTableParameteriv(GLenum target, GLenum pname, const GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ColorTableParameteriv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_CopyColorTable(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_CopyColorTable*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetColorTable(GLenum target, GLenum format, GLenum type, GLvoid *table)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetColorTable*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetColorTableParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetColorTableParameterfv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetColorTableParameteriv(GLenum target, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetColorTableParameteriv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ColorSubTable(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *table)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ColorSubTable*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_CopyColorSubTable(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_CopyColorSubTable*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ConvolutionFilter1D(GLenum target, GLenum internalFormat, GLsizei width,
                                         GLenum format, GLenum type, const GLvoid* image)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ConvolutionFilter1D*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ConvolutionFilter2D(GLenum target, GLenum internalFormat, GLsizei width, GLsizei height,
                                         GLenum format, GLenum type, const GLvoid* image)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ConvolutionFilter2D*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ConvolutionParameterf(GLenum target, GLenum pname, GLfloat param)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ConvolutionParameterf*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ConvolutionParameterfv(GLenum target, GLenum pname, const GLfloat* params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ConvolutionParameterfv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ConvolutionParameteri(GLenum target, GLenum pname, GLint param)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ConvolutionParameteri*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ConvolutionParameteriv(GLenum target, GLenum pname, const GLint * param)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ConvolutionParameteriv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_CopyConvolutionFilter1D(GLenum target, GLenum internalFormat, GLint x, GLint y, GLsizei width)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_CopyConvolutionFilter1D*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_CopyConvolutionFilter2D(GLenum target, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_CopyConvolutionFilter2D*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetConvolutionFilter(GLenum target, GLenum format,GLenum type, GLvoid* image)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetConvolutionFilter*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetConvolutionParameterfv(GLenum target, GLenum pname, GLfloat* params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetConvolutionParameterfv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetConvolutionParameteriv(GLenum target, GLenum pname, GLint * params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetConvolutionParameteriv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetSeparableFilter(GLenum target, GLenum format, GLenum type, GLvoid* row, GLvoid* column, GLvoid* span)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetSeparableFilter*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_SeparableFilter2D(GLenum target, GLenum internalFormat, GLsizei width, GLsizei height,
                                       GLenum format, GLenum type, const GLvoid* row, const GLvoid* column)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_SeparableFilter2D*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetHistogram(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetHistogram*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetHistogramParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetHistogramParameterfv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetHistogramParameteriv(GLenum target, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetHistogramParameteriv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetMinmax(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetMinmax*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetMinmaxParameterfv(GLenum target, GLenum pname, GLfloat *values)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetMinmaxParameterfv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetMinmaxParameteriv(GLenum target, GLenum pname, GLint *values)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetMinmaxParameteriv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Histogram(GLenum target, GLsizei width, GLenum internalFormat, GLboolean sink)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Histogram*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Minmax(GLenum target, GLenum internalFormat, GLboolean sink)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Minmax*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ResetHistogram(GLenum target)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ResetHistogram*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ResetMinmax(GLenum target)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ResetMinmax*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexImage3D(GLenum target, GLint lod, GLint components,
    GLsizei w, GLsizei h, GLsizei d, GLint border, GLenum format, GLenum type, const GLvoid *buf)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexImage3D*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexSubImage3D(GLenum target, GLint lod, GLint xoffset, GLint yoffset, GLint zoffset,
                      GLsizei w, GLsizei h, GLsizei d, GLenum format, GLenum type, const GLvoid *buf)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexSubImage3D*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_CopyTexSubImage3D(GLenum target, GLint level,
    GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_CopyTexSubImage3D*SZPTR]
}

#endif /* GL_VERSION_1_2 */

#if GL_VERSION_1_3

__declspec(naked)
GLvoid APIENTRY __glVIV_ActiveTexture(GLenum texture )
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ActiveTexture*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ClientActiveTexture(GLenum texture )
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ClientActiveTexture*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord1d(GLenum texture, GLdouble s)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord1d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord1dv(GLenum texture, const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord1dv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord1f(GLenum texture, GLfloat s)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord1f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord1fv(GLenum texture, const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord1fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord1i(GLenum texture, GLint s)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord1i*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord1iv(GLenum texture, const GLint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord1iv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord1s(GLenum texture, GLshort s)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord1s*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord1sv(GLenum texture, const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord1sv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord2d(GLenum texture, GLdouble s, GLdouble t)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord2d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord2dv(GLenum texture, const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord2dv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord2f(GLenum texture, GLfloat s, GLfloat t)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord2f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord2fv(GLenum texture, const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord2fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord2i(GLenum texture, GLint s, GLint t)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord2i*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord2iv(GLenum texture, const GLint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord2iv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord2s(GLenum texture, GLshort s, GLshort t)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord2s*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord2sv(GLenum texture, const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord2sv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord3d(GLenum texture, GLdouble s, GLdouble t, GLdouble r)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord3d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord3dv(GLenum texture, const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord3dv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord3f(GLenum texture, GLfloat s, GLfloat t, GLfloat r)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord3f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord3fv(GLenum texture, const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord3fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord3i(GLenum texture, GLint s, GLint t, GLint r)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord3i*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord3iv(GLenum texture, const GLint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord3iv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord3s(GLenum texture, GLshort s, GLshort t, GLshort r)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord3s*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord3sv(GLenum texture, const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord3sv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord4d(GLenum texture, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord4d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord4dv(GLenum texture, const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord4dv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord4f(GLenum texture, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord4f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord4fv(GLenum texture, const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord4fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord4i(GLenum texture, GLint s, GLint t, GLint r, GLint q)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord4i*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord4iv(GLenum texture, const GLint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord4iv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord4s(GLenum texture, GLshort s, GLshort t, GLshort r, GLshort q)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord4s*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiTexCoord4sv(GLenum texture, const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiTexCoord4sv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_LoadTransposeMatrixf(const GLfloat m[16])
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_LoadTransposeMatrixf*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_LoadTransposeMatrixd(const GLdouble m[16])
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_LoadTransposeMatrixd*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultTransposeMatrixf(const GLfloat m[16])
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultTransposeMatrixf*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultTransposeMatrixd(const GLdouble m[16])
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultTransposeMatrixd*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_SampleCoverage(GLclampf v, GLboolean invert)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_SampleCoverage*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_CompressedTexImage3D(GLenum target, GLint level, GLenum internalformat,
    GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_CompressedTexImage3D*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_CompressedTexImage2D(GLenum target, GLint level, GLenum internalformat,
    GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_CompressedTexImage2D*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_CompressedTexImage1D(GLenum target, GLint level, GLenum internalformat,
    GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_CompressedTexImage1D*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_CompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
    GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_CompressedTexSubImage3D*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_CompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset,
    GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_CompressedTexSubImage2D*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_CompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset,
    GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_CompressedTexSubImage1D*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetCompressedTexImage(GLenum target, GLint lod, GLvoid *img)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetCompressedTexImage*SZPTR]
}

#endif /* GL_VERSION_1_3 */

#if GL_VERSION_1_4

__declspec(naked)
GLvoid APIENTRY __glVIV_BlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_BlendFuncSeparate*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_FogCoordf(GLfloat coord)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_FogCoordf*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_FogCoordfv(const GLfloat *coord)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_FogCoordfv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_FogCoordd(GLdouble coord)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_FogCoordd*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_FogCoorddv(const GLdouble *coord)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_FogCoorddv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_FogCoordPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_FogCoordPointer*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiDrawArrays(GLenum mode, GLint * first, GLsizei *count, GLsizei primcount)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiDrawArrays*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_MultiDrawElements(GLenum mode, const GLsizei *count, GLenum type, const GLvoid ** indices, GLsizei primcount)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MultiDrawElements*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_PointParameterf(GLenum pname, GLfloat param)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_PointParameterf*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_PointParameterfv(GLenum pname, const GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_PointParameterfv*SZPTR]
}

GLvoid APIENTRY __glVIV_PointParameteri(GLenum pname, GLint param)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_PointParameteri*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_PointParameteriv(GLenum pname, const GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_PointParameteriv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_SecondaryColor3b(GLbyte red, GLbyte green, GLbyte blue)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_SecondaryColor3b*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_SecondaryColor3bv(const GLbyte *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_SecondaryColor3bv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_SecondaryColor3d(GLdouble red, GLdouble green, GLdouble blue)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_SecondaryColor3d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_SecondaryColor3dv(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_SecondaryColor3dv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_SecondaryColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_SecondaryColor3f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_SecondaryColor3fv(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_SecondaryColor3fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_SecondaryColor3i(GLint red, GLint green, GLint blue)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_SecondaryColor3i*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_SecondaryColor3iv(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_SecondaryColor3iv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_SecondaryColor3s(GLshort red, GLshort green, GLshort blue)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_SecondaryColor3s*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_SecondaryColor3sv(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_SecondaryColor3sv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_SecondaryColor3ub(GLubyte red, GLubyte green, GLubyte blue)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_SecondaryColor3ub*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_SecondaryColor3ubv(const GLubyte *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_SecondaryColor3ubv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_SecondaryColor3ui(GLuint red, GLuint green, GLuint blue)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_SecondaryColor3ui*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_SecondaryColor3uiv(const GLuint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_SecondaryColor3uiv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_SecondaryColor3us(GLushort red, GLushort green, GLushort blue)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_SecondaryColor3us*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_SecondaryColor3usv(const GLushort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_SecondaryColor3usv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_SecondaryColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_SecondaryColorPointer*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_WindowPos2d(GLdouble x, GLdouble y)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax + _gloffset_WindowPos2d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_WindowPos2dv(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_WindowPos2dv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_WindowPos2f(GLfloat x, GLfloat y)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_WindowPos2f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_WindowPos2fv(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_WindowPos2fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_WindowPos2i(GLint x, GLint y)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_WindowPos2i*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_WindowPos2iv(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_WindowPos2iv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_WindowPos2s(GLshort x, GLshort y)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_WindowPos2s*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_WindowPos2sv(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_WindowPos2sv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_WindowPos3d(GLdouble x, GLdouble y, GLdouble z)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_WindowPos3d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_WindowPos3dv(const GLdouble * v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_WindowPos3dv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_WindowPos3f(GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_WindowPos3f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_WindowPos3fv(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_WindowPos3fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_WindowPos3i(GLint x, GLint y, GLint z)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_WindowPos3i*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_WindowPos3iv(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_WindowPos3iv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_WindowPos3s(GLshort x, GLshort y, GLshort z)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_WindowPos3s*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_WindowPos3sv(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_WindowPos3sv*SZPTR]
}

#endif /* GL_VERSION_1_4 */

#if GL_VERSION_1_5

__declspec(naked)
GLvoid APIENTRY __glVIV_GenQueries(GLsizei n, GLuint *ids)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GenQueries*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_DeleteQueries(GLsizei n, const GLuint *ids)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_DeleteQueries*SZPTR]
}

__declspec(naked)
GLboolean APIENTRY __glVIV_IsQuery(GLuint id)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_IsQuery*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_BeginQuery(GLenum target, GLuint id)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_BeginQuery*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_EndQuery(GLenum target)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_EndQuery*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetQueryiv(GLenum target, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetQueryiv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetQueryObjectiv(GLuint id, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetQueryObjectiv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetQueryObjectuiv(GLuint id, GLenum pname, GLuint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetQueryObjectuiv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_BindBuffer(GLenum target, GLuint buffer)
{
     __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_BindBuffer*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_DeleteBuffers(GLsizei n, const GLuint *buffers)
{
     __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_DeleteBuffers*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GenBuffers(GLsizei n, GLuint * buffers)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GenBuffers*SZPTR]
}

__declspec(naked)
GLboolean APIENTRY __glVIV_IsBuffer(GLuint buffer)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_IsBuffer*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_BufferData(GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_BufferData*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_BufferSubData(GLenum tareget, GLintptrARB offset , GLsizeiptrARB size, const GLvoid *data)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_BufferSubData*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetBufferSubData(GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid *data)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetBufferSubData*SZPTR]
}

__declspec(naked)
GLvoid* APIENTRY __glVIV_MapBuffer(GLenum target, GLenum access)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_MapBuffer*SZPTR]
}

__declspec(naked)
GLboolean APIENTRY __glVIV_UnmapBuffer(GLenum target)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_UnmapBuffer*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetBufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetBufferParameteriv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetBufferPointerv(GLenum target, GLenum pname, GLvoid* *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetBufferPointerv*SZPTR]
}

#endif  /* GL_VERSION_1_5 */

#if GL_VERSION_2_0

__declspec(naked)
GLvoid APIENTRY __glVIV_BlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_BlendEquationSeparate*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_DrawBuffers(GLsizei count, const GLenum *bufs)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_DrawBuffers*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_StencilOpSeparate(GLenum face, GLenum fail, GLenum depthFail, GLenum depthPass)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_StencilOpSeparate*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_StencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_StencilFuncSeparate*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_StencilMaskSeparate(GLenum face, GLuint sm)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_StencilMaskSeparate*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_AttachShader(GLuint program, GLuint shader)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_AttachShader*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_BindAttribLocation(GLuint program, GLuint index, const GLchar *name)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_BindAttribLocation*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_CompileShader(GLuint shader)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_CompileShader*SZPTR]
}

__declspec(naked)
GLuint APIENTRY __glVIV_CreateProgram(GLvoid)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_CreateProgram*SZPTR]
}

__declspec(naked)
GLuint APIENTRY __glVIV_CreateShader(GLenum type)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_CreateShader*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_DeleteProgram(GLuint program)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_DeleteProgram*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_DeleteShader(GLuint shader)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_DeleteShader*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_DetachShader(GLuint program, GLuint shader)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_DetachShader*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_DisableVertexAttribArray(GLuint index)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_DisableVertexAttribArray*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_EnableVertexAttribArray(GLuint index)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_EnableVertexAttribArray*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize,
                                     GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetActiveAttrib*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetActiveUniform(GLuint program, GLuint index, GLsizei bufSize,
                                      GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetActiveUniform*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *obj)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetAttachedShaders*SZPTR]
}

__declspec(naked)
GLint APIENTRY __glVIV_GetAttribLocation(GLuint program, const GLchar *name)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetAttribLocation*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetProgramiv(GLuint program, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetProgramiv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetProgramInfoLog*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetShaderiv(GLuint shader, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetShaderiv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetShaderInfoLog*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetShaderSource*SZPTR]
}

__declspec(naked)
GLint APIENTRY __glVIV_GetUniformLocation(GLuint program, const GLchar *name)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetUniformLocation*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetUniformfv(GLuint program, GLint location, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetUniformfv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetUniformiv(GLuint program, GLint location, GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetUniformiv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetVertexAttribdv(GLuint index, GLenum pname, GLdouble *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetVertexAttribdv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetVertexAttribfv(GLuint index, GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetVertexAttribfv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetVertexAttribiv(GLuint index, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetVertexAttribiv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetVertexAttribPointerv(GLuint index, GLenum pname, GLvoid **pointer)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetVertexAttribPointerv*SZPTR]
}

__declspec(naked)
GLboolean APIENTRY __glVIV_IsProgram(GLuint program)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_IsProgram*SZPTR]
}

__declspec(naked)
GLboolean APIENTRY __glVIV_IsShader(GLuint shader)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_IsShader*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_LinkProgram(GLuint program)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_LinkProgram*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ShaderSource(GLuint shader, GLsizei count, const GLchar* *string, const GLint *length)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ShaderSource*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_UseProgram(GLuint program)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_UseProgram*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Uniform1f(GLint location, GLfloat v0)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Uniform1f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Uniform2f(GLint location, GLfloat v0, GLfloat v1)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Uniform2f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Uniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Uniform3f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Uniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Uniform4f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Uniform1i(GLint location, GLint v0)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Uniform1i*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Uniform2i(GLint location, GLint v0, GLint v1)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Uniform2i*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Uniform3i(GLint location, GLint v0, GLint v1, GLint v2)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Uniform3i*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Uniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Uniform4i*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Uniform1fv(GLint location, GLsizei count, const GLfloat *value)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Uniform1fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Uniform2fv(GLint location, GLsizei count, const GLfloat *value)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Uniform2fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Uniform3fv(GLint location, GLsizei count, const GLfloat *value)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Uniform3fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Uniform4fv(GLint location, GLsizei count, const GLfloat *value)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Uniform4fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Uniform1iv(GLint location, GLsizei count, const GLint *value)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Uniform1iv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Uniform2iv(GLint location, GLsizei count, const GLint *value)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Uniform2iv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Uniform3iv(GLint location, GLsizei count, const GLint *value)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Uniform3iv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Uniform4iv(GLint location, GLsizei count, const GLint *value)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Uniform4iv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_UniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_UniformMatrix2fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_UniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_UniformMatrix3fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_UniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_UniformMatrix4fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ValidateProgram(GLuint program)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ValidateProgram*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib1s(GLuint index, GLshort x)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib1s*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib1f(GLuint index, GLfloat x)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib1f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib1d(GLuint index, GLdouble x)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib1d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib2s(GLuint index, GLshort x, GLshort y)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib2s*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib2f(GLuint index, GLfloat x, GLfloat y)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib2f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib2d(GLuint index, GLdouble x, GLdouble y)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib2d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib3s(GLuint index, GLshort x, GLshort y, GLshort z)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib3s*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib3f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib3d(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib3d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib4s(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib4s*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib4f*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib4d*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib4Nub(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib4Nub*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib1sv(GLuint index, const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib1sv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib1fv(GLuint index, const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib1fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib1dv(GLuint index, const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib1dv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib2sv(GLuint index, const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib2sv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib2fv(GLuint index, const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib2fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib2dv(GLuint index, const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib2dv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib3sv(GLuint index, const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib3sv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib3fv(GLuint index, const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib3fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib3dv(GLuint index, const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib3dv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib4bv(GLuint index, const GLbyte *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib4bv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib4sv(GLuint index, const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib4sv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib4iv(GLuint index, const GLint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib4iv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib4ubv(GLuint index, const GLubyte *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib4ubv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib4usv(GLuint index, const GLushort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib4usv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib4uiv(GLuint index, const GLuint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib4uiv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib4fv(GLuint index, const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib4fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib4dv(GLuint index, const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib4dv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib4Nbv(GLuint index, const GLbyte *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib4Nbv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib4Nsv(GLuint index, const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib4Nsv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib4Niv(GLuint index, const GLint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib4Niv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib4Nubv(GLuint index, const GLubyte *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib4Nubv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib4Nusv(GLuint index, const GLushort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib4Nusv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttrib4Nuiv(GLuint index, const GLuint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttrib4Nuiv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttribPointer(GLuint index, GLint size, GLenum type,
                                GLboolean normalized, GLsizei stride,
                                const GLvoid *pointer)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttribPointer*SZPTR]
}

#endif

#if GL_VERSION_2_1
__declspec(naked)
GLvoid APIENTRY __glVIV_UniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_UniformMatrix2x3fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_UniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_UniformMatrix2x4fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_UniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_UniformMatrix3x2fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_UniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_UniformMatrix3x4fv*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_UniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_UniformMatrix4x2fv*SZPTR]
}
__declspec(naked)
GLvoid APIENTRY __glVIV_UniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_UniformMatrix4x3fv*SZPTR]
}
#endif

#if GL_ARB_vertex_program
__declspec(naked)
GLvoid APIENTRY __glVIV_ProgramStringARB(GLenum target, GLenum format, GLsizei len, const GLvoid *string)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ProgramStringARB*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_BindProgramARB(GLenum target, GLuint program)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_BindProgramARB*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_DeleteProgramsARB(GLsizei n, const GLuint *programs)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_DeleteProgramsARB*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GenProgramsARB(GLsizei n, GLuint *programs)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GenProgramsARB*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ProgramEnvParameter4dARB(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ProgramEnvParameter4dARB*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ProgramEnvParameter4dvARB(GLenum target, GLuint index, const GLdouble *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ProgramEnvParameter4dvARB*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ProgramEnvParameter4fARB(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ProgramEnvParameter4fARB*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ProgramEnvParameter4fvARB(GLenum target, GLuint index, const GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ProgramEnvParameter4fvARB*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ProgramLocalParameter4dARB(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ProgramLocalParameter4dARB*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ProgramLocalParameter4dvARB(GLenum target, GLuint index, const GLdouble *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ProgramLocalParameter4dvARB*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ProgramLocalParameter4fARB(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ProgramLocalParameter4fARB*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ProgramLocalParameter4fvARB(GLenum target, GLuint index, const GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ProgramLocalParameter4fvARB*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetProgramEnvParameterdvARB(GLenum target, GLuint index, GLdouble *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetProgramEnvParameterdvARB*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetProgramEnvParameterfvARB(GLenum target, GLuint index, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetProgramEnvParameterfvARB*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetProgramLocalParameterdvARB(GLenum target, GLuint index, GLdouble *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetProgramLocalParameterdvARB*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetProgramLocalParameterfvARB(GLenum target, GLuint index, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetProgramLocalParameterfvARB*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetProgramivARB(GLenum target, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetProgramivARB*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetProgramStringARB(GLenum target, GLenum pname, GLvoid *string)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetProgramStringARB*SZPTR]
}

__declspec(naked)
GLboolean APIENTRY __glVIV_IsProgramARB(GLuint program)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_IsProgramARB*SZPTR]
}

#endif  /* GL_ARB_vertex_program */

#if GL_ARB_shader_objects

__declspec(naked)
GLvoid APIENTRY __glVIV_DeleteObjectARB(GLhandleARB obj)
{
    __GL_GET_DISPATCH_TABLE()
    _asm jmp [eax+_gloffset_DeleteObjectARB*SZPTR]
}

__declspec(naked)
GLhandleARB APIENTRY __glVIV_GetHandleARB(GLenum pname)
{
    __GL_GET_DISPATCH_TABLE()
    _asm jmp [eax+_gloffset_GetHandleARB*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetInfoLogARB(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog)
{
    __GL_GET_DISPATCH_TABLE()
    _asm jmp [eax+_gloffset_GetInfoLogARB*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetObjectParameterfvARB(GLhandleARB obj, GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetObjectParameterfvARB*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetObjectParameterivARB(GLhandleARB obj, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetObjectParameterivARB*SZPTR]
}

#endif /* GL_ARB_shader_objects */

#if GL_ATI_vertex_array_object

__declspec(naked)
GLuint APIENTRY __glVIV_NewObjectBufferATI(GLsizei size, const GLvoid *pointer, GLenum usage)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_NewObjectBufferATI*SZPTR]
}
__declspec(naked)
GLboolean APIENTRY __glVIV_IsObjectBufferATI(GLuint buffer)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_IsObjectBufferATI*SZPTR]
}
__declspec(naked)
GLvoid APIENTRY __glVIV_UpdateObjectBufferATI(GLuint buffer, GLuint offset,
                                        GLsizei size, const GLvoid *pointer,
                                        GLenum preserve)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_UpdateObjectBufferATI*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetObjectBufferfvATI(GLuint buffer, GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetObjectBufferfvATI*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetObjectBufferivATI(GLuint buffer, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetObjectBufferivATI*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_FreeObjectBufferATI(GLuint buffer)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_FreeObjectBufferATI*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_ArrayObjectATI(GLenum array, GLint size, GLenum type, GLsizei stride, GLuint buffer,
                                       GLuint offset)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ArrayObjectATI*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetArrayObjectfvATI(GLenum array, GLenum pname, GLfloat * params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetArrayObjectfvATI*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetArrayObjectivATI(GLenum array, GLenum pname, GLint * params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetArrayObjectivATI*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VariantArrayObjectATI(GLuint id, GLenum type, GLsizei stride, GLuint buffer, GLuint offset)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VariantArrayObjectATI*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetVariantArrayObjectfvATI(GLuint id, GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetVariantArrayObjectfvATI*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetVariantArrayObjectivATI(GLuint id, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetVariantArrayObjectivATI*SZPTR]
}

#endif

#if GL_ATI_vertex_attrib_array_object

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttribArrayObjectATI(GLuint index, GLint size, GLenum type,
    GLboolean normalized, GLsizei stride, GLuint buffer, GLuint offset)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttribArrayObjectATI*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetVertexAttribArrayObjectfvATI(GLuint index, GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetVertexAttribArrayObjectfvATI*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetVertexAttribArrayObjectivATI(GLuint index, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetVertexAttribArrayObjectivATI*SZPTR]
}

#endif

#if GL_ATI_element_array

__declspec(naked)
GLvoid  APIENTRY __glVIV_ElementPointerATI(GLenum type, const GLvoid *pointer)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ElementPointerATI*SZPTR]
}

__declspec(naked)
GLvoid  APIENTRY __glVIV_DrawElementArrayATI(GLenum mode, GLsizei count)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_DrawElementArrayATI*SZPTR]
}

__declspec(naked)
GLvoid  APIENTRY __glVIV_DrawRangeElementArrayATI(GLenum mode, GLuint start,
                                  GLuint end, GLsizei count)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_DrawRangeElementArrayATI*SZPTR]
}

#endif

#if GL_EXT_stencil_two_side

__declspec(naked)
GLvoid APIENTRY __glVIV_ActiveStencilFaceEXT(GLenum face)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ActiveStencilFaceEXT*SZPTR]
}

#endif

__declspec(naked)
GLvoid APIENTRY __glVIV_AddSwapHintRectWIN(GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_AddSwapHintRectWIN*SZPTR]
}

#if GL_EXT_depth_bounds_test

__declspec(naked)
GLvoid APIENTRY __glVIV_DepthBoundsEXT(GLclampd zMin, GLclampd zMax)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_DepthBoundsEXT*SZPTR]
}

#endif

#if GL_EXT_framebuffer_object

__declspec(naked)
GLboolean APIENTRY __glVIV_IsRenderbufferEXT(GLuint renderbuffer)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_IsRenderbufferEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_BindRenderbufferEXT(GLenum target, GLuint renderbuffer)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_BindRenderbufferEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_DeleteRenderbuffersEXT(GLsizei n, const GLuint *renderbuffers)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_DeleteRenderbuffersEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GenRenderbuffersEXT(GLsizei n, GLuint *renderbuffers)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GenRenderbuffersEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_RenderbufferStorageEXT(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_RenderbufferStorageEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetRenderbufferParameterivEXT(GLenum target, GLenum pname, GLint* params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetRenderbufferParameterivEXT*SZPTR]
}

__declspec(naked)
GLboolean APIENTRY __glVIV_IsFramebufferEXT(GLuint framebuffer)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_IsFramebufferEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_BindFramebufferEXT(GLenum target, GLuint framebuffer)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_BindFramebufferEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_DeleteFramebuffersEXT(GLsizei n, const GLuint *framebuffers)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_DeleteFramebuffersEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GenFramebuffersEXT(GLsizei n, GLuint *framebuffers)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GenFramebuffersEXT*SZPTR]
}

__declspec(naked)
GLenum APIENTRY __glVIV_CheckFramebufferStatusEXT(GLenum target)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_CheckFramebufferStatusEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_FramebufferTexture1DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_FramebufferTexture1DEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_FramebufferTexture2DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_FramebufferTexture2DEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_FramebufferTexture3DEXT(GLenum target, GLenum attachment,
    GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_FramebufferTexture3DEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_FramebufferRenderbufferEXT(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_FramebufferRenderbufferEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetFramebufferAttachmentParameterivEXT(GLenum target, GLenum attachment, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetFramebufferAttachmentParameterivEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GenerateMipmapEXT(GLenum target)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GenerateMipmapEXT*SZPTR]
}

#if GL_EXT_framebuffer_blit

__declspec(naked)
GLvoid APIENTRY __glVIV_BlitFramebufferEXT(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
    GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_BlitFramebufferEXT*SZPTR]
}

#if GL_EXT_framebuffer_multisample

__declspec(naked)
GLvoid APIENTRY __glVIV_RenderbufferStorageMultisampleEXT(GLenum target, GLsizei samples,
    GLenum internalformat, GLsizei width, GLsizei height)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_RenderbufferStorageMultisampleEXT*SZPTR]
}

#endif /* GL_EXT_framebuffer_multisample */

#endif /* GL_EXT_framebuffer_blit */

#endif /* GL_EXT_framebuffer_object */

#if GL_NV_occlusion_query

__declspec(naked)
GLvoid APIENTRY __glVIV_BeginQueryNV(GLuint id)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_BeginQueryNV*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_EndQueryNV(GLvoid)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_EndQueryNV*SZPTR]
}

#endif

#if GL_EXT_bindable_uniform

__declspec(naked)
GLvoid APIENTRY __glVIV_UniformBufferEXT(GLuint program, GLint location, GLuint buffer)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_UniformBufferEXT*SZPTR]
}

__declspec(naked)
GLint APIENTRY __glVIV_GetUniformBufferSizeEXT(GLuint program, GLint location)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetUniformBufferSizeEXT*SZPTR]
}

__declspec(naked)
GLintptr APIENTRY __glVIV_GetUniformOffsetEXT(GLuint program, GLint location)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetUniformOffsetEXT*SZPTR]
}

#endif

#if GL_EXT_texture_integer

__declspec(naked)
GLvoid APIENTRY __glVIV_ClearColorIiEXT(GLint r, GLint g, GLint b, GLint a)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ClearColorIiEXT*SZPTR]

}

__declspec(naked)
GLvoid APIENTRY __glVIV_ClearColorIuiEXT(GLuint r, GLuint g, GLuint b, GLuint a)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ClearColorIuiEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexParameterIivEXT(GLenum target, GLenum pname, GLint * params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexParameterIivEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_TexParameterIuivEXT(GLenum target, GLenum pname, GLuint * params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_TexParameterIuivEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetTexParameterIivEXT(GLenum target, GLenum pname, GLint * params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetTexParameterIivEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetTexParameterIuivEXT(GLenum target, GLenum pname, GLuint * params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetTexParameterIuivEXT*SZPTR]
}

#endif

#if GL_EXT_gpu_shader4

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttribI1iEXT(GLuint index, GLint x)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttribI1iEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttribI2iEXT(GLuint index, GLint x, GLint y)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttribI2iEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttribI3iEXT(GLuint index, GLint x, GLint y, GLint z)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttribI3iEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttribI4iEXT(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttribI4iEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttribI1uiEXT(GLuint index, GLuint x)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttribI1uiEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttribI2uiEXT(GLuint index, GLuint x, GLuint y)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttribI2uiEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttribI3uiEXT(GLuint index, GLuint x, GLuint y, GLuint z)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttribI3uiEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttribI4uiEXT(GLuint index, GLuint x, GLuint y, GLuint z,
                            GLuint w)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttribI4uiEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttribI1ivEXT(GLuint index, const GLint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttribI1ivEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttribI2ivEXT(GLuint index, const GLint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttribI2ivEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttribI3ivEXT(GLuint index, const GLint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttribI3ivEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttribI4ivEXT(GLuint index, const GLint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttribI4ivEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttribI1uivEXT(GLuint index, const GLuint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttribI1uivEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttribI2uivEXT(GLuint index, const GLuint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttribI2uivEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttribI3uivEXT(GLuint index, const GLuint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttribI3uivEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttribI4uivEXT(GLuint index, const GLuint *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttribI4uivEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttribI4bvEXT(GLuint index, const GLbyte *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttribI4bvEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttribI4svEXT(GLuint index, const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttribI4svEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttribI4ubvEXT(GLuint index, const GLubyte *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttribI4ubvEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttribI4usvEXT(GLuint index, const GLushort *v)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttribI4usvEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_VertexAttribIPointerEXT(GLuint index, GLint size, GLenum type,
                                GLsizei stride, const GLvoid *pointer)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_VertexAttribIPointerEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetVertexAttribIivEXT(GLuint index, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetVertexAttribIivEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetVertexAttribIuivEXT(GLuint index, GLenum pname, GLuint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetVertexAttribIuivEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Uniform1uiEXT(GLint location, GLuint v0)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Uniform1uiEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Uniform2uiEXT(GLint location, GLuint v0, GLuint v1)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Uniform2uiEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Uniform3uiEXT(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Uniform3uiEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Uniform4uiEXT(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Uniform4uiEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Uniform1uivEXT(GLint location, GLsizei count, const GLuint *value)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Uniform1uivEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Uniform2uivEXT(GLint location, GLsizei count, const GLuint *value)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Uniform2uivEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Uniform3uivEXT(GLint location, GLsizei count, const GLuint *value)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Uniform3uivEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_Uniform4uivEXT(GLint location, GLsizei count, const GLuint *value)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_Uniform4uivEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetUniformuivEXT(GLuint program, GLint location, GLuint *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetUniformuivEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_BindFragDataLocationEXT(GLuint program, GLuint colorNumber,
                                const GLbyte *name)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_BindFragDataLocationEXT*SZPTR]
}

__declspec(naked)
GLint APIENTRY __glVIV_GetFragDataLocationEXT(GLuint program, const GLbyte *name)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_GetFragDataLocationEXT*SZPTR]
}

#endif

#if GL_EXT_geometry_shader4
__declspec(naked)
GLvoid APIENTRY __glVIV_ProgramParameteriEXT(GLuint program, GLenum pname, GLint value)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_ProgramParameteriEXT*SZPTR]
}
__declspec(naked)
GLvoid APIENTRY __glVIV_FramebufferTextureEXT(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_FramebufferTextureEXT*SZPTR]
}
__declspec(naked)
GLvoid APIENTRY __glVIV_FramebufferTextureLayerEXT(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_FramebufferTextureLayerEXT*SZPTR]
}
__declspec(naked)
GLvoid APIENTRY __glVIV_FramebufferTextureFaceEXT(GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_FramebufferTextureFaceEXT*SZPTR]
}
#endif

#if GL_EXT_draw_buffers2
__declspec(naked)
GLvoid APIENTRY __glVIV_ColorMaskIndexedEXT(GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp[eax+ _gloffset_ColorMaskIndexedEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetBooleanIndexedvEXT(GLenum value, GLuint index, GLboolean *data)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp[eax + _gloffset_GetBooleanIndexedvEXT*SZPTR]
}
__declspec(naked)
GLvoid APIENTRY __glVIV_GetIntegerIndexedvEXT(GLenum value, GLuint index, GLint *data)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp[eax + _gloffset_GetIntegerIndexedvEXT*SZPTR]
}
__declspec(naked)
GLvoid APIENTRY __glVIV_EnableIndexedEXT(GLenum target, GLuint index)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp[eax + _gloffset_EnableIndexedEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_DisableIndexedEXT(GLenum target, GLuint index)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp[eax + _gloffset_DisableIndexedEXT*SZPTR]
}

__declspec(naked)
GLboolean APIENTRY __glVIV_IsEnabledIndexedEXT(GLenum target, GLuint index)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp[eax + _gloffset_IsEnabledIndexedEXT*SZPTR]
}
#endif
#if GL_EXT_texture_buffer_object
__declspec(naked)
GLvoid APIENTRY __glVIV_TexBufferEXT(GLenum target, GLenum internalformat, GLuint buffer)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp[eax + _gloffset_TexBufferEXT*SZPTR]
}
#endif
#if GL_EXT_gpu_program_parameters
__declspec(naked)
GLvoid APIENTRY __glVIV_ProgramEnvParameters4fvEXT(GLenum target, GLuint index, GLsizei count, const GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp[eax + _gloffset_ProgramEnvParameters4fvEXT*SZPTR]
}
__declspec(naked)
GLvoid APIENTRY __glVIV_ProgramLocalParameters4fvEXT(GLenum target, GLuint index, GLsizei count, const GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp[eax + _gloffset_ProgramLocalParameters4fvEXT*SZPTR]
}
#endif

#if GL_EXT_draw_instanced
__declspec(naked)
GLvoid APIENTRY __glVIV_DrawArraysInstancedEXT(
                        GLenum mode, GLint first, GLsizei count, GLsizei primCount)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_DrawArraysInstancedEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_DrawElementsInstancedEXT(
            GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primCount)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp [eax+_gloffset_DrawElementsInstancedEXT*SZPTR]
}
#endif

#if GL_ARB_color_buffer_float
__declspec(naked)
GLvoid APIENTRY __glVIV_ClampColorARB(GLenum target, GLenum clamp)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp[eax + _gloffset_ClampColorARB*SZPTR]
}
#endif

#if GL_EXT_timer_query
__declspec(naked)
GLvoid APIENTRY __glVIV_GetQueryObjecti64vEXT(GLuint id, GLenum pname, GLint64EXT * params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp[eax + _gloffset_GetQueryObjecti64vEXT*SZPTR]
}

__declspec(naked)
GLvoid APIENTRY __glVIV_GetQueryObjectui64vEXT(GLuint id, GLenum pname, GLuint64EXT * params)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp[eax + _gloffset_GetQueryObjectui64vEXT*SZPTR]
}
#endif

#ifdef GL_ATI_separate_stencil
__declspec(naked)
GLvoid APIENTRY __glVIV_StencilFuncSeparateATI(GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask)
{
    __GL_GET_DISPATCH_TABLE()
    __asm jmp[eax + _gloffset_StencilFuncSeparateATI*SZPTR]
}
#endif