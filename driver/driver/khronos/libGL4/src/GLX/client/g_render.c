/* $XFree86: xc/lib/GL/glx/g_render.c,v 1.4 2002/02/22 21:32:53 dawes Exp $ */
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

#include "packrender.h"

GLvoid __indirect_glCallList(GLuint list)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_CallList,8);
    __GLX_PUT_LONG(4,list);
    __GLX_END(8);
}

GLvoid __indirect_glListBase(GLuint base)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_ListBase,8);
    __GLX_PUT_LONG(4,base);
    __GLX_END(8);
}

GLvoid __indirect_glBegin(GLenum mode)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Begin,8);
    __GLX_PUT_LONG(4,mode);
    __GLX_END(8);
}

GLvoid __indirect_glColor3b(GLbyte red, GLbyte green, GLbyte blue)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color3bv,8);
    __GLX_PUT_CHAR(4,red);
    __GLX_PUT_CHAR(5,green);
    __GLX_PUT_CHAR(6,blue);
    __GLX_END(8);
}

GLvoid __indirect_glColor3bv(const GLbyte *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color3bv,8);
    __GLX_PUT_CHAR(4,v[0]);
    __GLX_PUT_CHAR(5,v[1]);
    __GLX_PUT_CHAR(6,v[2]);
    __GLX_END(8);
}

GLvoid __indirect_glColor3d(GLdouble red, GLdouble green, GLdouble blue)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color3dv,28);
    __GLX_PUT_DOUBLE(4,red);
    __GLX_PUT_DOUBLE(12,green);
    __GLX_PUT_DOUBLE(20,blue);
    __GLX_END(28);
}

GLvoid __indirect_glColor3dv(const GLdouble *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color3dv,28);
    __GLX_PUT_DOUBLE(4,v[0]);
    __GLX_PUT_DOUBLE(12,v[1]);
    __GLX_PUT_DOUBLE(20,v[2]);
    __GLX_END(28);
}

GLvoid __indirect_glColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color3fv,16);
    __GLX_PUT_FLOAT(4,red);
    __GLX_PUT_FLOAT(8,green);
    __GLX_PUT_FLOAT(12,blue);
    __GLX_END(16);
}

GLvoid __indirect_glColor3fv(const GLfloat *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color3fv,16);
    __GLX_PUT_FLOAT(4,v[0]);
    __GLX_PUT_FLOAT(8,v[1]);
    __GLX_PUT_FLOAT(12,v[2]);
    __GLX_END(16);
}

GLvoid __indirect_glColor3i(GLint red, GLint green, GLint blue)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color3iv,16);
    __GLX_PUT_LONG(4,red);
    __GLX_PUT_LONG(8,green);
    __GLX_PUT_LONG(12,blue);
    __GLX_END(16);
}

GLvoid __indirect_glColor3iv(const GLint *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color3iv,16);
    __GLX_PUT_LONG(4,v[0]);
    __GLX_PUT_LONG(8,v[1]);
    __GLX_PUT_LONG(12,v[2]);
    __GLX_END(16);
}

GLvoid __indirect_glColor3s(GLshort red, GLshort green, GLshort blue)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color3sv,12);
    __GLX_PUT_SHORT(4,red);
    __GLX_PUT_SHORT(6,green);
    __GLX_PUT_SHORT(8,blue);
    __GLX_END(12);
}

GLvoid __indirect_glColor3sv(const GLshort *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color3sv,12);
    __GLX_PUT_SHORT(4,v[0]);
    __GLX_PUT_SHORT(6,v[1]);
    __GLX_PUT_SHORT(8,v[2]);
    __GLX_END(12);
}

GLvoid __indirect_glColor3ub(GLubyte red, GLubyte green, GLubyte blue)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color3ubv,8);
    __GLX_PUT_CHAR(4,red);
    __GLX_PUT_CHAR(5,green);
    __GLX_PUT_CHAR(6,blue);
    __GLX_END(8);
}

GLvoid __indirect_glColor3ubv(const GLubyte *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color3ubv,8);
    __GLX_PUT_CHAR(4,v[0]);
    __GLX_PUT_CHAR(5,v[1]);
    __GLX_PUT_CHAR(6,v[2]);
    __GLX_END(8);
}

GLvoid __indirect_glColor3ui(GLuint red, GLuint green, GLuint blue)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color3uiv,16);
    __GLX_PUT_LONG(4,red);
    __GLX_PUT_LONG(8,green);
    __GLX_PUT_LONG(12,blue);
    __GLX_END(16);
}

GLvoid __indirect_glColor3uiv(const GLuint *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color3uiv,16);
    __GLX_PUT_LONG(4,v[0]);
    __GLX_PUT_LONG(8,v[1]);
    __GLX_PUT_LONG(12,v[2]);
    __GLX_END(16);
}

GLvoid __indirect_glColor3us(GLushort red, GLushort green, GLushort blue)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color3usv,12);
    __GLX_PUT_SHORT(4,red);
    __GLX_PUT_SHORT(6,green);
    __GLX_PUT_SHORT(8,blue);
    __GLX_END(12);
}

GLvoid __indirect_glColor3usv(const GLushort *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color3usv,12);
    __GLX_PUT_SHORT(4,v[0]);
    __GLX_PUT_SHORT(6,v[1]);
    __GLX_PUT_SHORT(8,v[2]);
    __GLX_END(12);
}

GLvoid __indirect_glColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color4bv,8);
    __GLX_PUT_CHAR(4,red);
    __GLX_PUT_CHAR(5,green);
    __GLX_PUT_CHAR(6,blue);
    __GLX_PUT_CHAR(7,alpha);
    __GLX_END(8);
}

GLvoid __indirect_glColor4bv(const GLbyte *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color4bv,8);
    __GLX_PUT_CHAR(4,v[0]);
    __GLX_PUT_CHAR(5,v[1]);
    __GLX_PUT_CHAR(6,v[2]);
    __GLX_PUT_CHAR(7,v[3]);
    __GLX_END(8);
}

GLvoid __indirect_glColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color4dv,36);
    __GLX_PUT_DOUBLE(4,red);
    __GLX_PUT_DOUBLE(12,green);
    __GLX_PUT_DOUBLE(20,blue);
    __GLX_PUT_DOUBLE(28,alpha);
    __GLX_END(36);
}

GLvoid __indirect_glColor4dv(const GLdouble *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color4dv,36);
    __GLX_PUT_DOUBLE(4,v[0]);
    __GLX_PUT_DOUBLE(12,v[1]);
    __GLX_PUT_DOUBLE(20,v[2]);
    __GLX_PUT_DOUBLE(28,v[3]);
    __GLX_END(36);
}

GLvoid __indirect_glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color4fv,20);
    __GLX_PUT_FLOAT(4,red);
    __GLX_PUT_FLOAT(8,green);
    __GLX_PUT_FLOAT(12,blue);
    __GLX_PUT_FLOAT(16,alpha);
    __GLX_END(20);
}

GLvoid __indirect_glColor4fv(const GLfloat *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color4fv,20);
    __GLX_PUT_FLOAT(4,v[0]);
    __GLX_PUT_FLOAT(8,v[1]);
    __GLX_PUT_FLOAT(12,v[2]);
    __GLX_PUT_FLOAT(16,v[3]);
    __GLX_END(20);
}

GLvoid __indirect_glColor4i(GLint red, GLint green, GLint blue, GLint alpha)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color4iv,20);
    __GLX_PUT_LONG(4,red);
    __GLX_PUT_LONG(8,green);
    __GLX_PUT_LONG(12,blue);
    __GLX_PUT_LONG(16,alpha);
    __GLX_END(20);
}

GLvoid __indirect_glColor4iv(const GLint *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color4iv,20);
    __GLX_PUT_LONG(4,v[0]);
    __GLX_PUT_LONG(8,v[1]);
    __GLX_PUT_LONG(12,v[2]);
    __GLX_PUT_LONG(16,v[3]);
    __GLX_END(20);
}

GLvoid __indirect_glColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color4sv,12);
    __GLX_PUT_SHORT(4,red);
    __GLX_PUT_SHORT(6,green);
    __GLX_PUT_SHORT(8,blue);
    __GLX_PUT_SHORT(10,alpha);
    __GLX_END(12);
}

GLvoid __indirect_glColor4sv(const GLshort *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color4sv,12);
    __GLX_PUT_SHORT(4,v[0]);
    __GLX_PUT_SHORT(6,v[1]);
    __GLX_PUT_SHORT(8,v[2]);
    __GLX_PUT_SHORT(10,v[3]);
    __GLX_END(12);
}

GLvoid __indirect_glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color4ubv,8);
    __GLX_PUT_CHAR(4,red);
    __GLX_PUT_CHAR(5,green);
    __GLX_PUT_CHAR(6,blue);
    __GLX_PUT_CHAR(7,alpha);
    __GLX_END(8);
}

GLvoid __indirect_glColor4ubv(const GLubyte *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color4ubv,8);
    __GLX_PUT_CHAR(4,v[0]);
    __GLX_PUT_CHAR(5,v[1]);
    __GLX_PUT_CHAR(6,v[2]);
    __GLX_PUT_CHAR(7,v[3]);
    __GLX_END(8);
}

GLvoid __indirect_glColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color4uiv,20);
    __GLX_PUT_LONG(4,red);
    __GLX_PUT_LONG(8,green);
    __GLX_PUT_LONG(12,blue);
    __GLX_PUT_LONG(16,alpha);
    __GLX_END(20);
}

GLvoid __indirect_glColor4uiv(const GLuint *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color4uiv,20);
    __GLX_PUT_LONG(4,v[0]);
    __GLX_PUT_LONG(8,v[1]);
    __GLX_PUT_LONG(12,v[2]);
    __GLX_PUT_LONG(16,v[3]);
    __GLX_END(20);
}

GLvoid __indirect_glColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color4usv,12);
    __GLX_PUT_SHORT(4,red);
    __GLX_PUT_SHORT(6,green);
    __GLX_PUT_SHORT(8,blue);
    __GLX_PUT_SHORT(10,alpha);
    __GLX_END(12);
}

GLvoid __indirect_glColor4usv(const GLushort *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Color4usv,12);
    __GLX_PUT_SHORT(4,v[0]);
    __GLX_PUT_SHORT(6,v[1]);
    __GLX_PUT_SHORT(8,v[2]);
    __GLX_PUT_SHORT(10,v[3]);
    __GLX_END(12);
}

GLvoid __indirect_glEdgeFlag(GLboolean flag)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_EdgeFlagv,8);
    __GLX_PUT_CHAR(4,flag);
    __GLX_END(8);
}

GLvoid __indirect_glEdgeFlagv(const GLboolean *flag)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_EdgeFlagv,8);
    __GLX_PUT_CHAR(4,flag[0]);
    __GLX_END(8);
}

GLvoid __indirect_glEnd(GLvoid)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_End,4);
    __GLX_END(4);
}

GLvoid __indirect_glIndexd(GLdouble c)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Indexdv,12);
    __GLX_PUT_DOUBLE(4,c);
    __GLX_END(12);
}

GLvoid __indirect_glIndexdv(const GLdouble *c)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Indexdv,12);
    __GLX_PUT_DOUBLE(4,c[0]);
    __GLX_END(12);
}

GLvoid __indirect_glIndexf(GLfloat c)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Indexfv,8);
    __GLX_PUT_FLOAT(4,c);
    __GLX_END(8);
}

GLvoid __indirect_glIndexfv(const GLfloat *c)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Indexfv,8);
    __GLX_PUT_FLOAT(4,c[0]);
    __GLX_END(8);
}

GLvoid __indirect_glIndexi(GLint c)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Indexiv,8);
    __GLX_PUT_LONG(4,c);
    __GLX_END(8);
}

GLvoid __indirect_glIndexiv(const GLint *c)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Indexiv,8);
    __GLX_PUT_LONG(4,c[0]);
    __GLX_END(8);
}

GLvoid __indirect_glIndexs(GLshort c)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Indexsv,8);
    __GLX_PUT_SHORT(4,c);
    __GLX_END(8);
}

GLvoid __indirect_glIndexsv(const GLshort *c)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Indexsv,8);
    __GLX_PUT_SHORT(4,c[0]);
    __GLX_END(8);
}

GLvoid __indirect_glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Normal3bv,8);
    __GLX_PUT_CHAR(4,nx);
    __GLX_PUT_CHAR(5,ny);
    __GLX_PUT_CHAR(6,nz);
    __GLX_END(8);
}

GLvoid __indirect_glNormal3bv(const GLbyte *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Normal3bv,8);
    __GLX_PUT_CHAR(4,v[0]);
    __GLX_PUT_CHAR(5,v[1]);
    __GLX_PUT_CHAR(6,v[2]);
    __GLX_END(8);
}

GLvoid __indirect_glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Normal3dv,28);
    __GLX_PUT_DOUBLE(4,nx);
    __GLX_PUT_DOUBLE(12,ny);
    __GLX_PUT_DOUBLE(20,nz);
    __GLX_END(28);
}

GLvoid __indirect_glNormal3dv(const GLdouble *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Normal3dv,28);
    __GLX_PUT_DOUBLE(4,v[0]);
    __GLX_PUT_DOUBLE(12,v[1]);
    __GLX_PUT_DOUBLE(20,v[2]);
    __GLX_END(28);
}

GLvoid __indirect_glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Normal3fv,16);
    __GLX_PUT_FLOAT(4,nx);
    __GLX_PUT_FLOAT(8,ny);
    __GLX_PUT_FLOAT(12,nz);
    __GLX_END(16);
}

GLvoid __indirect_glNormal3fv(const GLfloat *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Normal3fv,16);
    __GLX_PUT_FLOAT(4,v[0]);
    __GLX_PUT_FLOAT(8,v[1]);
    __GLX_PUT_FLOAT(12,v[2]);
    __GLX_END(16);
}

GLvoid __indirect_glNormal3i(GLint nx, GLint ny, GLint nz)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Normal3iv,16);
    __GLX_PUT_LONG(4,nx);
    __GLX_PUT_LONG(8,ny);
    __GLX_PUT_LONG(12,nz);
    __GLX_END(16);
}

GLvoid __indirect_glNormal3iv(const GLint *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Normal3iv,16);
    __GLX_PUT_LONG(4,v[0]);
    __GLX_PUT_LONG(8,v[1]);
    __GLX_PUT_LONG(12,v[2]);
    __GLX_END(16);
}

GLvoid __indirect_glNormal3s(GLshort nx, GLshort ny, GLshort nz)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Normal3sv,12);
    __GLX_PUT_SHORT(4,nx);
    __GLX_PUT_SHORT(6,ny);
    __GLX_PUT_SHORT(8,nz);
    __GLX_END(12);
}

GLvoid __indirect_glNormal3sv(const GLshort *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Normal3sv,12);
    __GLX_PUT_SHORT(4,v[0]);
    __GLX_PUT_SHORT(6,v[1]);
    __GLX_PUT_SHORT(8,v[2]);
    __GLX_END(12);
}

GLvoid __indirect_glRasterPos2d(GLdouble x, GLdouble y)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_RasterPos2dv,20);
    __GLX_PUT_DOUBLE(4,x);
    __GLX_PUT_DOUBLE(12,y);
    __GLX_END(20);
}

GLvoid __indirect_glRasterPos2dv(const GLdouble *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_RasterPos2dv,20);
    __GLX_PUT_DOUBLE(4,v[0]);
    __GLX_PUT_DOUBLE(12,v[1]);
    __GLX_END(20);
}

GLvoid __indirect_glRasterPos2f(GLfloat x, GLfloat y)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_RasterPos2fv,12);
    __GLX_PUT_FLOAT(4,x);
    __GLX_PUT_FLOAT(8,y);
    __GLX_END(12);
}

GLvoid __indirect_glRasterPos2fv(const GLfloat *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_RasterPos2fv,12);
    __GLX_PUT_FLOAT(4,v[0]);
    __GLX_PUT_FLOAT(8,v[1]);
    __GLX_END(12);
}

GLvoid __indirect_glRasterPos2i(GLint x, GLint y)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_RasterPos2iv,12);
    __GLX_PUT_LONG(4,x);
    __GLX_PUT_LONG(8,y);
    __GLX_END(12);
}

GLvoid __indirect_glRasterPos2iv(const GLint *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_RasterPos2iv,12);
    __GLX_PUT_LONG(4,v[0]);
    __GLX_PUT_LONG(8,v[1]);
    __GLX_END(12);
}

GLvoid __indirect_glRasterPos2s(GLshort x, GLshort y)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_RasterPos2sv,8);
    __GLX_PUT_SHORT(4,x);
    __GLX_PUT_SHORT(6,y);
    __GLX_END(8);
}

GLvoid __indirect_glRasterPos2sv(const GLshort *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_RasterPos2sv,8);
    __GLX_PUT_SHORT(4,v[0]);
    __GLX_PUT_SHORT(6,v[1]);
    __GLX_END(8);
}

GLvoid __indirect_glRasterPos3d(GLdouble x, GLdouble y, GLdouble z)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_RasterPos3dv,28);
    __GLX_PUT_DOUBLE(4,x);
    __GLX_PUT_DOUBLE(12,y);
    __GLX_PUT_DOUBLE(20,z);
    __GLX_END(28);
}

GLvoid __indirect_glRasterPos3dv(const GLdouble *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_RasterPos3dv,28);
    __GLX_PUT_DOUBLE(4,v[0]);
    __GLX_PUT_DOUBLE(12,v[1]);
    __GLX_PUT_DOUBLE(20,v[2]);
    __GLX_END(28);
}

GLvoid __indirect_glRasterPos3f(GLfloat x, GLfloat y, GLfloat z)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_RasterPos3fv,16);
    __GLX_PUT_FLOAT(4,x);
    __GLX_PUT_FLOAT(8,y);
    __GLX_PUT_FLOAT(12,z);
    __GLX_END(16);
}

GLvoid __indirect_glRasterPos3fv(const GLfloat *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_RasterPos3fv,16);
    __GLX_PUT_FLOAT(4,v[0]);
    __GLX_PUT_FLOAT(8,v[1]);
    __GLX_PUT_FLOAT(12,v[2]);
    __GLX_END(16);
}

GLvoid __indirect_glRasterPos3i(GLint x, GLint y, GLint z)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_RasterPos3iv,16);
    __GLX_PUT_LONG(4,x);
    __GLX_PUT_LONG(8,y);
    __GLX_PUT_LONG(12,z);
    __GLX_END(16);
}

GLvoid __indirect_glRasterPos3iv(const GLint *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_RasterPos3iv,16);
    __GLX_PUT_LONG(4,v[0]);
    __GLX_PUT_LONG(8,v[1]);
    __GLX_PUT_LONG(12,v[2]);
    __GLX_END(16);
}

GLvoid __indirect_glRasterPos3s(GLshort x, GLshort y, GLshort z)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_RasterPos3sv,12);
    __GLX_PUT_SHORT(4,x);
    __GLX_PUT_SHORT(6,y);
    __GLX_PUT_SHORT(8,z);
    __GLX_END(12);
}

GLvoid __indirect_glRasterPos3sv(const GLshort *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_RasterPos3sv,12);
    __GLX_PUT_SHORT(4,v[0]);
    __GLX_PUT_SHORT(6,v[1]);
    __GLX_PUT_SHORT(8,v[2]);
    __GLX_END(12);
}

GLvoid __indirect_glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_RasterPos4dv,36);
    __GLX_PUT_DOUBLE(4,x);
    __GLX_PUT_DOUBLE(12,y);
    __GLX_PUT_DOUBLE(20,z);
    __GLX_PUT_DOUBLE(28,w);
    __GLX_END(36);
}

GLvoid __indirect_glRasterPos4dv(const GLdouble *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_RasterPos4dv,36);
    __GLX_PUT_DOUBLE(4,v[0]);
    __GLX_PUT_DOUBLE(12,v[1]);
    __GLX_PUT_DOUBLE(20,v[2]);
    __GLX_PUT_DOUBLE(28,v[3]);
    __GLX_END(36);
}

GLvoid __indirect_glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_RasterPos4fv,20);
    __GLX_PUT_FLOAT(4,x);
    __GLX_PUT_FLOAT(8,y);
    __GLX_PUT_FLOAT(12,z);
    __GLX_PUT_FLOAT(16,w);
    __GLX_END(20);
}

GLvoid __indirect_glRasterPos4fv(const GLfloat *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_RasterPos4fv,20);
    __GLX_PUT_FLOAT(4,v[0]);
    __GLX_PUT_FLOAT(8,v[1]);
    __GLX_PUT_FLOAT(12,v[2]);
    __GLX_PUT_FLOAT(16,v[3]);
    __GLX_END(20);
}

GLvoid __indirect_glRasterPos4i(GLint x, GLint y, GLint z, GLint w)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_RasterPos4iv,20);
    __GLX_PUT_LONG(4,x);
    __GLX_PUT_LONG(8,y);
    __GLX_PUT_LONG(12,z);
    __GLX_PUT_LONG(16,w);
    __GLX_END(20);
}

GLvoid __indirect_glRasterPos4iv(const GLint *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_RasterPos4iv,20);
    __GLX_PUT_LONG(4,v[0]);
    __GLX_PUT_LONG(8,v[1]);
    __GLX_PUT_LONG(12,v[2]);
    __GLX_PUT_LONG(16,v[3]);
    __GLX_END(20);
}

GLvoid __indirect_glRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_RasterPos4sv,12);
    __GLX_PUT_SHORT(4,x);
    __GLX_PUT_SHORT(6,y);
    __GLX_PUT_SHORT(8,z);
    __GLX_PUT_SHORT(10,w);
    __GLX_END(12);
}

GLvoid __indirect_glRasterPos4sv(const GLshort *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_RasterPos4sv,12);
    __GLX_PUT_SHORT(4,v[0]);
    __GLX_PUT_SHORT(6,v[1]);
    __GLX_PUT_SHORT(8,v[2]);
    __GLX_PUT_SHORT(10,v[3]);
    __GLX_END(12);
}

GLvoid __indirect_glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Rectdv,36);
    __GLX_PUT_DOUBLE(4,x1);
    __GLX_PUT_DOUBLE(12,y1);
    __GLX_PUT_DOUBLE(20,x2);
    __GLX_PUT_DOUBLE(28,y2);
    __GLX_END(36);
}

GLvoid __indirect_glRectdv(const GLdouble *v1, const GLdouble *v2)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Rectdv,36);
    __GLX_PUT_DOUBLE(4,v1[0]);
    __GLX_PUT_DOUBLE(12,v1[1]);
    __GLX_PUT_DOUBLE(20,v2[0]);
    __GLX_PUT_DOUBLE(28,v2[1]);
    __GLX_END(36);
}

GLvoid __indirect_glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Rectfv,20);
    __GLX_PUT_FLOAT(4,x1);
    __GLX_PUT_FLOAT(8,y1);
    __GLX_PUT_FLOAT(12,x2);
    __GLX_PUT_FLOAT(16,y2);
    __GLX_END(20);
}

GLvoid __indirect_glRectfv(const GLfloat *v1, const GLfloat *v2)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Rectfv,20);
    __GLX_PUT_FLOAT(4,v1[0]);
    __GLX_PUT_FLOAT(8,v1[1]);
    __GLX_PUT_FLOAT(12,v2[0]);
    __GLX_PUT_FLOAT(16,v2[1]);
    __GLX_END(20);
}

GLvoid __indirect_glRecti(GLint x1, GLint y1, GLint x2, GLint y2)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Rectiv,20);
    __GLX_PUT_LONG(4,x1);
    __GLX_PUT_LONG(8,y1);
    __GLX_PUT_LONG(12,x2);
    __GLX_PUT_LONG(16,y2);
    __GLX_END(20);
}

GLvoid __indirect_glRectiv(const GLint *v1, const GLint *v2)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Rectiv,20);
    __GLX_PUT_LONG(4,v1[0]);
    __GLX_PUT_LONG(8,v1[1]);
    __GLX_PUT_LONG(12,v2[0]);
    __GLX_PUT_LONG(16,v2[1]);
    __GLX_END(20);
}

GLvoid __indirect_glRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Rectsv,12);
    __GLX_PUT_SHORT(4,x1);
    __GLX_PUT_SHORT(6,y1);
    __GLX_PUT_SHORT(8,x2);
    __GLX_PUT_SHORT(10,y2);
    __GLX_END(12);
}

GLvoid __indirect_glRectsv(const GLshort *v1, const GLshort *v2)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Rectsv,12);
    __GLX_PUT_SHORT(4,v1[0]);
    __GLX_PUT_SHORT(6,v1[1]);
    __GLX_PUT_SHORT(8,v2[0]);
    __GLX_PUT_SHORT(10,v2[1]);
    __GLX_END(12);
}

GLvoid __indirect_glTexCoord1d(GLdouble s)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord1dv,12);
    __GLX_PUT_DOUBLE(4,s);
    __GLX_END(12);
}

GLvoid __indirect_glTexCoord1dv(const GLdouble *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord1dv,12);
    __GLX_PUT_DOUBLE(4,v[0]);
    __GLX_END(12);
}

GLvoid __indirect_glTexCoord1f(GLfloat s)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord1fv,8);
    __GLX_PUT_FLOAT(4,s);
    __GLX_END(8);
}

GLvoid __indirect_glTexCoord1fv(const GLfloat *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord1fv,8);
    __GLX_PUT_FLOAT(4,v[0]);
    __GLX_END(8);
}

GLvoid __indirect_glTexCoord1i(GLint s)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord1iv,8);
    __GLX_PUT_LONG(4,s);
    __GLX_END(8);
}

GLvoid __indirect_glTexCoord1iv(const GLint *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord1iv,8);
    __GLX_PUT_LONG(4,v[0]);
    __GLX_END(8);
}

GLvoid __indirect_glTexCoord1s(GLshort s)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord1sv,8);
    __GLX_PUT_SHORT(4,s);
    __GLX_END(8);
}

GLvoid __indirect_glTexCoord1sv(const GLshort *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord1sv,8);
    __GLX_PUT_SHORT(4,v[0]);
    __GLX_END(8);
}

GLvoid __indirect_glTexCoord2d(GLdouble s, GLdouble t)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord2dv,20);
    __GLX_PUT_DOUBLE(4,s);
    __GLX_PUT_DOUBLE(12,t);
    __GLX_END(20);
}

GLvoid __indirect_glTexCoord2dv(const GLdouble *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord2dv,20);
    __GLX_PUT_DOUBLE(4,v[0]);
    __GLX_PUT_DOUBLE(12,v[1]);
    __GLX_END(20);
}

GLvoid __indirect_glTexCoord2f(GLfloat s, GLfloat t)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord2fv,12);
    __GLX_PUT_FLOAT(4,s);
    __GLX_PUT_FLOAT(8,t);
    __GLX_END(12);
}

GLvoid __indirect_glTexCoord2fv(const GLfloat *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord2fv,12);
    __GLX_PUT_FLOAT(4,v[0]);
    __GLX_PUT_FLOAT(8,v[1]);
    __GLX_END(12);
}

GLvoid __indirect_glTexCoord2i(GLint s, GLint t)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord2iv,12);
    __GLX_PUT_LONG(4,s);
    __GLX_PUT_LONG(8,t);
    __GLX_END(12);
}

GLvoid __indirect_glTexCoord2iv(const GLint *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord2iv,12);
    __GLX_PUT_LONG(4,v[0]);
    __GLX_PUT_LONG(8,v[1]);
    __GLX_END(12);
}

GLvoid __indirect_glTexCoord2s(GLshort s, GLshort t)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord2sv,8);
    __GLX_PUT_SHORT(4,s);
    __GLX_PUT_SHORT(6,t);
    __GLX_END(8);
}

GLvoid __indirect_glTexCoord2sv(const GLshort *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord2sv,8);
    __GLX_PUT_SHORT(4,v[0]);
    __GLX_PUT_SHORT(6,v[1]);
    __GLX_END(8);
}

GLvoid __indirect_glTexCoord3d(GLdouble s, GLdouble t, GLdouble r)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord3dv,28);
    __GLX_PUT_DOUBLE(4,s);
    __GLX_PUT_DOUBLE(12,t);
    __GLX_PUT_DOUBLE(20,r);
    __GLX_END(28);
}

GLvoid __indirect_glTexCoord3dv(const GLdouble *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord3dv,28);
    __GLX_PUT_DOUBLE(4,v[0]);
    __GLX_PUT_DOUBLE(12,v[1]);
    __GLX_PUT_DOUBLE(20,v[2]);
    __GLX_END(28);
}

GLvoid __indirect_glTexCoord3f(GLfloat s, GLfloat t, GLfloat r)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord3fv,16);
    __GLX_PUT_FLOAT(4,s);
    __GLX_PUT_FLOAT(8,t);
    __GLX_PUT_FLOAT(12,r);
    __GLX_END(16);
}

GLvoid __indirect_glTexCoord3fv(const GLfloat *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord3fv,16);
    __GLX_PUT_FLOAT(4,v[0]);
    __GLX_PUT_FLOAT(8,v[1]);
    __GLX_PUT_FLOAT(12,v[2]);
    __GLX_END(16);
}

GLvoid __indirect_glTexCoord3i(GLint s, GLint t, GLint r)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord3iv,16);
    __GLX_PUT_LONG(4,s);
    __GLX_PUT_LONG(8,t);
    __GLX_PUT_LONG(12,r);
    __GLX_END(16);
}

GLvoid __indirect_glTexCoord3iv(const GLint *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord3iv,16);
    __GLX_PUT_LONG(4,v[0]);
    __GLX_PUT_LONG(8,v[1]);
    __GLX_PUT_LONG(12,v[2]);
    __GLX_END(16);
}

GLvoid __indirect_glTexCoord3s(GLshort s, GLshort t, GLshort r)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord3sv,12);
    __GLX_PUT_SHORT(4,s);
    __GLX_PUT_SHORT(6,t);
    __GLX_PUT_SHORT(8,r);
    __GLX_END(12);
}

GLvoid __indirect_glTexCoord3sv(const GLshort *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord3sv,12);
    __GLX_PUT_SHORT(4,v[0]);
    __GLX_PUT_SHORT(6,v[1]);
    __GLX_PUT_SHORT(8,v[2]);
    __GLX_END(12);
}

GLvoid __indirect_glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord4dv,36);
    __GLX_PUT_DOUBLE(4,s);
    __GLX_PUT_DOUBLE(12,t);
    __GLX_PUT_DOUBLE(20,r);
    __GLX_PUT_DOUBLE(28,q);
    __GLX_END(36);
}

GLvoid __indirect_glTexCoord4dv(const GLdouble *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord4dv,36);
    __GLX_PUT_DOUBLE(4,v[0]);
    __GLX_PUT_DOUBLE(12,v[1]);
    __GLX_PUT_DOUBLE(20,v[2]);
    __GLX_PUT_DOUBLE(28,v[3]);
    __GLX_END(36);
}

GLvoid __indirect_glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord4fv,20);
    __GLX_PUT_FLOAT(4,s);
    __GLX_PUT_FLOAT(8,t);
    __GLX_PUT_FLOAT(12,r);
    __GLX_PUT_FLOAT(16,q);
    __GLX_END(20);
}

GLvoid __indirect_glTexCoord4fv(const GLfloat *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord4fv,20);
    __GLX_PUT_FLOAT(4,v[0]);
    __GLX_PUT_FLOAT(8,v[1]);
    __GLX_PUT_FLOAT(12,v[2]);
    __GLX_PUT_FLOAT(16,v[3]);
    __GLX_END(20);
}

GLvoid __indirect_glTexCoord4i(GLint s, GLint t, GLint r, GLint q)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord4iv,20);
    __GLX_PUT_LONG(4,s);
    __GLX_PUT_LONG(8,t);
    __GLX_PUT_LONG(12,r);
    __GLX_PUT_LONG(16,q);
    __GLX_END(20);
}

GLvoid __indirect_glTexCoord4iv(const GLint *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord4iv,20);
    __GLX_PUT_LONG(4,v[0]);
    __GLX_PUT_LONG(8,v[1]);
    __GLX_PUT_LONG(12,v[2]);
    __GLX_PUT_LONG(16,v[3]);
    __GLX_END(20);
}

GLvoid __indirect_glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord4sv,12);
    __GLX_PUT_SHORT(4,s);
    __GLX_PUT_SHORT(6,t);
    __GLX_PUT_SHORT(8,r);
    __GLX_PUT_SHORT(10,q);
    __GLX_END(12);
}

GLvoid __indirect_glTexCoord4sv(const GLshort *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexCoord4sv,12);
    __GLX_PUT_SHORT(4,v[0]);
    __GLX_PUT_SHORT(6,v[1]);
    __GLX_PUT_SHORT(8,v[2]);
    __GLX_PUT_SHORT(10,v[3]);
    __GLX_END(12);
}

GLvoid __indirect_glVertex2d(GLdouble x, GLdouble y)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Vertex2dv,20);
    __GLX_PUT_DOUBLE(4,x);
    __GLX_PUT_DOUBLE(12,y);
    __GLX_END(20);
}

GLvoid __indirect_glVertex2dv(const GLdouble *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Vertex2dv,20);
    __GLX_PUT_DOUBLE(4,v[0]);
    __GLX_PUT_DOUBLE(12,v[1]);
    __GLX_END(20);
}

GLvoid __indirect_glVertex2f(GLfloat x, GLfloat y)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Vertex2fv,12);
    __GLX_PUT_FLOAT(4,x);
    __GLX_PUT_FLOAT(8,y);
    __GLX_END(12);
}

GLvoid __indirect_glVertex2fv(const GLfloat *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Vertex2fv,12);
    __GLX_PUT_FLOAT(4,v[0]);
    __GLX_PUT_FLOAT(8,v[1]);
    __GLX_END(12);
}

GLvoid __indirect_glVertex2i(GLint x, GLint y)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Vertex2iv,12);
    __GLX_PUT_LONG(4,x);
    __GLX_PUT_LONG(8,y);
    __GLX_END(12);
}

GLvoid __indirect_glVertex2iv(const GLint *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Vertex2iv,12);
    __GLX_PUT_LONG(4,v[0]);
    __GLX_PUT_LONG(8,v[1]);
    __GLX_END(12);
}

GLvoid __indirect_glVertex2s(GLshort x, GLshort y)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Vertex2sv,8);
    __GLX_PUT_SHORT(4,x);
    __GLX_PUT_SHORT(6,y);
    __GLX_END(8);
}

GLvoid __indirect_glVertex2sv(const GLshort *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Vertex2sv,8);
    __GLX_PUT_SHORT(4,v[0]);
    __GLX_PUT_SHORT(6,v[1]);
    __GLX_END(8);
}

GLvoid __indirect_glVertex3d(GLdouble x, GLdouble y, GLdouble z)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Vertex3dv,28);
    __GLX_PUT_DOUBLE(4,x);
    __GLX_PUT_DOUBLE(12,y);
    __GLX_PUT_DOUBLE(20,z);
    __GLX_END(28);
}

GLvoid __indirect_glVertex3dv(const GLdouble *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Vertex3dv,28);
    __GLX_PUT_DOUBLE(4,v[0]);
    __GLX_PUT_DOUBLE(12,v[1]);
    __GLX_PUT_DOUBLE(20,v[2]);
    __GLX_END(28);
}

GLvoid __indirect_glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Vertex3fv,16);
    __GLX_PUT_FLOAT(4,x);
    __GLX_PUT_FLOAT(8,y);
    __GLX_PUT_FLOAT(12,z);
    __GLX_END(16);
}

GLvoid __indirect_glVertex3fv(const GLfloat *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Vertex3fv,16);
    __GLX_PUT_FLOAT(4,v[0]);
    __GLX_PUT_FLOAT(8,v[1]);
    __GLX_PUT_FLOAT(12,v[2]);
    __GLX_END(16);
}

GLvoid __indirect_glVertex3i(GLint x, GLint y, GLint z)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Vertex3iv,16);
    __GLX_PUT_LONG(4,x);
    __GLX_PUT_LONG(8,y);
    __GLX_PUT_LONG(12,z);
    __GLX_END(16);
}

GLvoid __indirect_glVertex3iv(const GLint *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Vertex3iv,16);
    __GLX_PUT_LONG(4,v[0]);
    __GLX_PUT_LONG(8,v[1]);
    __GLX_PUT_LONG(12,v[2]);
    __GLX_END(16);
}

GLvoid __indirect_glVertex3s(GLshort x, GLshort y, GLshort z)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Vertex3sv,12);
    __GLX_PUT_SHORT(4,x);
    __GLX_PUT_SHORT(6,y);
    __GLX_PUT_SHORT(8,z);
    __GLX_END(12);
}

GLvoid __indirect_glVertex3sv(const GLshort *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Vertex3sv,12);
    __GLX_PUT_SHORT(4,v[0]);
    __GLX_PUT_SHORT(6,v[1]);
    __GLX_PUT_SHORT(8,v[2]);
    __GLX_END(12);
}

GLvoid __indirect_glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Vertex4dv,36);
    __GLX_PUT_DOUBLE(4,x);
    __GLX_PUT_DOUBLE(12,y);
    __GLX_PUT_DOUBLE(20,z);
    __GLX_PUT_DOUBLE(28,w);
    __GLX_END(36);
}

GLvoid __indirect_glVertex4dv(const GLdouble *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Vertex4dv,36);
    __GLX_PUT_DOUBLE(4,v[0]);
    __GLX_PUT_DOUBLE(12,v[1]);
    __GLX_PUT_DOUBLE(20,v[2]);
    __GLX_PUT_DOUBLE(28,v[3]);
    __GLX_END(36);
}

GLvoid __indirect_glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Vertex4fv,20);
    __GLX_PUT_FLOAT(4,x);
    __GLX_PUT_FLOAT(8,y);
    __GLX_PUT_FLOAT(12,z);
    __GLX_PUT_FLOAT(16,w);
    __GLX_END(20);
}

GLvoid __indirect_glVertex4fv(const GLfloat *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Vertex4fv,20);
    __GLX_PUT_FLOAT(4,v[0]);
    __GLX_PUT_FLOAT(8,v[1]);
    __GLX_PUT_FLOAT(12,v[2]);
    __GLX_PUT_FLOAT(16,v[3]);
    __GLX_END(20);
}

GLvoid __indirect_glVertex4i(GLint x, GLint y, GLint z, GLint w)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Vertex4iv,20);
    __GLX_PUT_LONG(4,x);
    __GLX_PUT_LONG(8,y);
    __GLX_PUT_LONG(12,z);
    __GLX_PUT_LONG(16,w);
    __GLX_END(20);
}

GLvoid __indirect_glVertex4iv(const GLint *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Vertex4iv,20);
    __GLX_PUT_LONG(4,v[0]);
    __GLX_PUT_LONG(8,v[1]);
    __GLX_PUT_LONG(12,v[2]);
    __GLX_PUT_LONG(16,v[3]);
    __GLX_END(20);
}

GLvoid __indirect_glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Vertex4sv,12);
    __GLX_PUT_SHORT(4,x);
    __GLX_PUT_SHORT(6,y);
    __GLX_PUT_SHORT(8,z);
    __GLX_PUT_SHORT(10,w);
    __GLX_END(12);
}

GLvoid __indirect_glVertex4sv(const GLshort *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Vertex4sv,12);
    __GLX_PUT_SHORT(4,v[0]);
    __GLX_PUT_SHORT(6,v[1]);
    __GLX_PUT_SHORT(8,v[2]);
    __GLX_PUT_SHORT(10,v[3]);
    __GLX_END(12);
}

GLvoid __indirect_glClipPlane(GLenum plane, const GLdouble *equation)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_ClipPlane,40);
    __GLX_PUT_DOUBLE(4,equation[0]);
    __GLX_PUT_DOUBLE(12,equation[1]);
    __GLX_PUT_DOUBLE(20,equation[2]);
    __GLX_PUT_DOUBLE(28,equation[3]);
    __GLX_PUT_LONG(36,plane);
    __GLX_END(40);
}

GLvoid __indirect_glColorMaterial(GLenum face, GLenum mode)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_ColorMaterial,12);
    __GLX_PUT_LONG(4,face);
    __GLX_PUT_LONG(8,mode);
    __GLX_END(12);
}

GLvoid __indirect_glCullFace(GLenum mode)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_CullFace,8);
    __GLX_PUT_LONG(4,mode);
    __GLX_END(8);
}

GLvoid __indirect_glFogf(GLenum pname, GLfloat param)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Fogf,12);
    __GLX_PUT_LONG(4,pname);
    __GLX_PUT_FLOAT(8,param);
    __GLX_END(12);
}

GLvoid __indirect_glFogfv(GLenum pname, const GLfloat *params)
{
    __GLX_DECLARE_VARIABLES();
    compsize = __glFogfv_size(pname);
    __GLX_LOAD_VARIABLES();
    cmdlen = 8+compsize*4;
    __GLX_BEGIN(X_GLrop_Fogfv,cmdlen);
    __GLX_PUT_LONG(4,pname);
    __GLX_PUT_FLOAT_ARRAY(8,params,compsize);
    __GLX_END(cmdlen);
}

GLvoid __indirect_glFogi(GLenum pname, GLint param)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Fogi,12);
    __GLX_PUT_LONG(4,pname);
    __GLX_PUT_LONG(8,param);
    __GLX_END(12);
}

GLvoid __indirect_glFogiv(GLenum pname, const GLint *params)
{
    __GLX_DECLARE_VARIABLES();
    compsize = __glFogiv_size(pname);
    __GLX_LOAD_VARIABLES();
    cmdlen = 8+compsize*4;
    __GLX_BEGIN(X_GLrop_Fogiv,cmdlen);
    __GLX_PUT_LONG(4,pname);
    __GLX_PUT_LONG_ARRAY(8,params,compsize);
    __GLX_END(cmdlen);
}

GLvoid __indirect_glFrontFace(GLenum mode)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_FrontFace,8);
    __GLX_PUT_LONG(4,mode);
    __GLX_END(8);
}

GLvoid __indirect_glHint(GLenum target, GLenum mode)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Hint,12);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,mode);
    __GLX_END(12);
}

GLvoid __indirect_glLightf(GLenum light, GLenum pname, GLfloat param)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Lightf,16);
    __GLX_PUT_LONG(4,light);
    __GLX_PUT_LONG(8,pname);
    __GLX_PUT_FLOAT(12,param);
    __GLX_END(16);
}

GLvoid __indirect_glLightfv(GLenum light, GLenum pname, const GLfloat *params)
{
    __GLX_DECLARE_VARIABLES();
    compsize = __glLightfv_size(pname);
    __GLX_LOAD_VARIABLES();
    cmdlen = 12+compsize*4;
    __GLX_BEGIN(X_GLrop_Lightfv,cmdlen);
    __GLX_PUT_LONG(4,light);
    __GLX_PUT_LONG(8,pname);
    __GLX_PUT_FLOAT_ARRAY(12,params,compsize);
    __GLX_END(cmdlen);
}

GLvoid __indirect_glLighti(GLenum light, GLenum pname, GLint param)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Lighti,16);
    __GLX_PUT_LONG(4,light);
    __GLX_PUT_LONG(8,pname);
    __GLX_PUT_LONG(12,param);
    __GLX_END(16);
}

GLvoid __indirect_glLightiv(GLenum light, GLenum pname, const GLint *params)
{
    __GLX_DECLARE_VARIABLES();
    compsize = __glLightiv_size(pname);
    __GLX_LOAD_VARIABLES();
    cmdlen = 12+compsize*4;
    __GLX_BEGIN(X_GLrop_Lightiv,cmdlen);
    __GLX_PUT_LONG(4,light);
    __GLX_PUT_LONG(8,pname);
    __GLX_PUT_LONG_ARRAY(12,params,compsize);
    __GLX_END(cmdlen);
}

GLvoid __indirect_glLightModelf(GLenum pname, GLfloat param)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_LightModelf,12);
    __GLX_PUT_LONG(4,pname);
    __GLX_PUT_FLOAT(8,param);
    __GLX_END(12);
}

GLvoid __indirect_glLightModelfv(GLenum pname, const GLfloat *params)
{
    __GLX_DECLARE_VARIABLES();
    compsize = __glLightModelfv_size(pname);
    __GLX_LOAD_VARIABLES();
    cmdlen = 8+compsize*4;
    __GLX_BEGIN(X_GLrop_LightModelfv,cmdlen);
    __GLX_PUT_LONG(4,pname);
    __GLX_PUT_FLOAT_ARRAY(8,params,compsize);
    __GLX_END(cmdlen);
}

GLvoid __indirect_glLightModeli(GLenum pname, GLint param)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_LightModeli,12);
    __GLX_PUT_LONG(4,pname);
    __GLX_PUT_LONG(8,param);
    __GLX_END(12);
}

GLvoid __indirect_glLightModeliv(GLenum pname, const GLint *params)
{
    __GLX_DECLARE_VARIABLES();
    compsize = __glLightModeliv_size(pname);
    __GLX_LOAD_VARIABLES();
    cmdlen = 8+compsize*4;
    __GLX_BEGIN(X_GLrop_LightModeliv,cmdlen);
    __GLX_PUT_LONG(4,pname);
    __GLX_PUT_LONG_ARRAY(8,params,compsize);
    __GLX_END(cmdlen);
}

GLvoid __indirect_glLineStipple(GLint factor, GLushort pattern)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_LineStipple,12);
    __GLX_PUT_LONG(4,factor);
    __GLX_PUT_SHORT(8,pattern);
    __GLX_END(12);
}

GLvoid __indirect_glLineWidth(GLfloat width)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_LineWidth,8);
    __GLX_PUT_FLOAT(4,width);
    __GLX_END(8);
}

GLvoid __indirect_glMaterialf(GLenum face, GLenum pname, GLfloat param)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Materialf,16);
    __GLX_PUT_LONG(4,face);
    __GLX_PUT_LONG(8,pname);
    __GLX_PUT_FLOAT(12,param);
    __GLX_END(16);
}

GLvoid __indirect_glMaterialfv(GLenum face, GLenum pname, const GLfloat *params)
{
    __GLX_DECLARE_VARIABLES();
    compsize = __glMaterialfv_size(pname);
    __GLX_LOAD_VARIABLES();
    cmdlen = 12+compsize*4;
    __GLX_BEGIN(X_GLrop_Materialfv,cmdlen);
    __GLX_PUT_LONG(4,face);
    __GLX_PUT_LONG(8,pname);
    __GLX_PUT_FLOAT_ARRAY(12,params,compsize);
    __GLX_END(cmdlen);
}

GLvoid __indirect_glMateriali(GLenum face, GLenum pname, GLint param)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Materiali,16);
    __GLX_PUT_LONG(4,face);
    __GLX_PUT_LONG(8,pname);
    __GLX_PUT_LONG(12,param);
    __GLX_END(16);
}

GLvoid __indirect_glMaterialiv(GLenum face, GLenum pname, const GLint *params)
{
    __GLX_DECLARE_VARIABLES();
    compsize = __glMaterialiv_size(pname);
    __GLX_LOAD_VARIABLES();
    cmdlen = 12+compsize*4;
    __GLX_BEGIN(X_GLrop_Materialiv,cmdlen);
    __GLX_PUT_LONG(4,face);
    __GLX_PUT_LONG(8,pname);
    __GLX_PUT_LONG_ARRAY(12,params,compsize);
    __GLX_END(cmdlen);
}

GLvoid __indirect_glPointSize(GLfloat size)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_PointSize,8);
    __GLX_PUT_FLOAT(4,size);
    __GLX_END(8);
}

GLvoid __indirect_glPolygonMode(GLenum face, GLenum mode)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_PolygonMode,12);
    __GLX_PUT_LONG(4,face);
    __GLX_PUT_LONG(8,mode);
    __GLX_END(12);
}

GLvoid __indirect_glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Scissor,20);
    __GLX_PUT_LONG(4,x);
    __GLX_PUT_LONG(8,y);
    __GLX_PUT_LONG(12,width);
    __GLX_PUT_LONG(16,height);
    __GLX_END(20);
}

GLvoid __indirect_glShadeModel(GLenum mode)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_ShadeModel,8);
    __GLX_PUT_LONG(4,mode);
    __GLX_END(8);
}

GLvoid __indirect_glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexParameterf,16);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,pname);
    __GLX_PUT_FLOAT(12,param);
    __GLX_END(16);
}

GLvoid __indirect_glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
    __GLX_DECLARE_VARIABLES();
    compsize = __glTexParameterfv_size(pname);
    __GLX_LOAD_VARIABLES();
    cmdlen = 12+compsize*4;
    __GLX_BEGIN(X_GLrop_TexParameterfv,cmdlen);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,pname);
    __GLX_PUT_FLOAT_ARRAY(12,params,compsize);
    __GLX_END(cmdlen);
}

GLvoid __indirect_glTexParameteri(GLenum target, GLenum pname, GLint param)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexParameteri,16);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,pname);
    __GLX_PUT_LONG(12,param);
    __GLX_END(16);
}

GLvoid __indirect_glTexParameteriv(GLenum target, GLenum pname, const GLint *params)
{
    __GLX_DECLARE_VARIABLES();
    compsize = __glTexParameteriv_size(pname);
    __GLX_LOAD_VARIABLES();
    cmdlen = 12+compsize*4;
    __GLX_BEGIN(X_GLrop_TexParameteriv,cmdlen);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,pname);
    __GLX_PUT_LONG_ARRAY(12,params,compsize);
    __GLX_END(cmdlen);
}

GLvoid __indirect_glTexEnvf(GLenum target, GLenum pname, GLfloat param)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexEnvf,16);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,pname);
    __GLX_PUT_FLOAT(12,param);
    __GLX_END(16);
}

GLvoid __indirect_glTexEnvfv(GLenum target, GLenum pname, const GLfloat *params)
{
    __GLX_DECLARE_VARIABLES();
    compsize = __glTexEnvfv_size(pname);
    __GLX_LOAD_VARIABLES();
    cmdlen = 12+compsize*4;
    __GLX_BEGIN(X_GLrop_TexEnvfv,cmdlen);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,pname);
    __GLX_PUT_FLOAT_ARRAY(12,params,compsize);
    __GLX_END(cmdlen);
}

GLvoid __indirect_glTexEnvi(GLenum target, GLenum pname, GLint param)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexEnvi,16);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,pname);
    __GLX_PUT_LONG(12,param);
    __GLX_END(16);
}

GLvoid __indirect_glTexEnviv(GLenum target, GLenum pname, const GLint *params)
{
    __GLX_DECLARE_VARIABLES();
    compsize = __glTexEnviv_size(pname);
    __GLX_LOAD_VARIABLES();
    cmdlen = 12+compsize*4;
    __GLX_BEGIN(X_GLrop_TexEnviv,cmdlen);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,pname);
    __GLX_PUT_LONG_ARRAY(12,params,compsize);
    __GLX_END(cmdlen);
}

GLvoid __indirect_glTexGend(GLenum coord, GLenum pname, GLdouble param)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexGend,20);
    __GLX_PUT_DOUBLE(4,param);
    __GLX_PUT_LONG(12,coord);
    __GLX_PUT_LONG(16,pname);
    __GLX_END(20);
}

GLvoid __indirect_glTexGendv(GLenum coord, GLenum pname, const GLdouble *params)
{
    __GLX_DECLARE_VARIABLES();
    compsize = __glTexGendv_size(pname);
    __GLX_LOAD_VARIABLES();
    cmdlen = 12+compsize*8;
    __GLX_BEGIN(X_GLrop_TexGendv,cmdlen);
    __GLX_PUT_LONG(4,coord);
    __GLX_PUT_LONG(8,pname);
    __GLX_PUT_DOUBLE_ARRAY(12,params,compsize);
    __GLX_END(cmdlen);
}

GLvoid __indirect_glTexGenf(GLenum coord, GLenum pname, GLfloat param)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexGenf,16);
    __GLX_PUT_LONG(4,coord);
    __GLX_PUT_LONG(8,pname);
    __GLX_PUT_FLOAT(12,param);
    __GLX_END(16);
}

GLvoid __indirect_glTexGenfv(GLenum coord, GLenum pname, const GLfloat *params)
{
    __GLX_DECLARE_VARIABLES();
    compsize = __glTexGenfv_size(pname);
    __GLX_LOAD_VARIABLES();
    cmdlen = 12+compsize*4;
    __GLX_BEGIN(X_GLrop_TexGenfv,cmdlen);
    __GLX_PUT_LONG(4,coord);
    __GLX_PUT_LONG(8,pname);
    __GLX_PUT_FLOAT_ARRAY(12,params,compsize);
    __GLX_END(cmdlen);
}

GLvoid __indirect_glTexGeni(GLenum coord, GLenum pname, GLint param)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_TexGeni,16);
    __GLX_PUT_LONG(4,coord);
    __GLX_PUT_LONG(8,pname);
    __GLX_PUT_LONG(12,param);
    __GLX_END(16);
}

GLvoid __indirect_glTexGeniv(GLenum coord, GLenum pname, const GLint *params)
{
    __GLX_DECLARE_VARIABLES();
    compsize = __glTexGeniv_size(pname);
    __GLX_LOAD_VARIABLES();
    cmdlen = 12+compsize*4;
    __GLX_BEGIN(X_GLrop_TexGeniv,cmdlen);
    __GLX_PUT_LONG(4,coord);
    __GLX_PUT_LONG(8,pname);
    __GLX_PUT_LONG_ARRAY(12,params,compsize);
    __GLX_END(cmdlen);
}

GLvoid __indirect_glInitNames(GLvoid)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_InitNames,4);
    __GLX_END(4);
}

GLvoid __indirect_glLoadName(GLuint name)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_LoadName,8);
    __GLX_PUT_LONG(4,name);
    __GLX_END(8);
}

GLvoid __indirect_glPassThrough(GLfloat token)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_PassThrough,8);
    __GLX_PUT_FLOAT(4,token);
    __GLX_END(8);
}

GLvoid __indirect_glPopName(GLvoid)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_PopName,4);
    __GLX_END(4);
}

GLvoid __indirect_glPushName(GLuint name)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_PushName,8);
    __GLX_PUT_LONG(4,name);
    __GLX_END(8);
}

GLvoid __indirect_glDrawBuffer(GLenum mode)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_DrawBuffer,8);
    __GLX_PUT_LONG(4,mode);
    __GLX_END(8);
}

GLvoid __indirect_glClear(GLbitfield mask)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Clear,8);
    __GLX_PUT_LONG(4,mask);
    __GLX_END(8);
}

GLvoid __indirect_glClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_ClearAccum,20);
    __GLX_PUT_FLOAT(4,red);
    __GLX_PUT_FLOAT(8,green);
    __GLX_PUT_FLOAT(12,blue);
    __GLX_PUT_FLOAT(16,alpha);
    __GLX_END(20);
}

GLvoid __indirect_glClearIndex(GLfloat c)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_ClearIndex,8);
    __GLX_PUT_FLOAT(4,c);
    __GLX_END(8);
}

GLvoid __indirect_glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_ClearColor,20);
    __GLX_PUT_FLOAT(4,red);
    __GLX_PUT_FLOAT(8,green);
    __GLX_PUT_FLOAT(12,blue);
    __GLX_PUT_FLOAT(16,alpha);
    __GLX_END(20);
}

GLvoid __indirect_glClearStencil(GLint s)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_ClearStencil,8);
    __GLX_PUT_LONG(4,s);
    __GLX_END(8);
}

GLvoid __indirect_glClearDepth(GLclampd depth)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_ClearDepth,12);
    __GLX_PUT_DOUBLE(4,depth);
    __GLX_END(12);
}

GLvoid __indirect_glStencilMask(GLuint mask)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_StencilMask,8);
    __GLX_PUT_LONG(4,mask);
    __GLX_END(8);
}

GLvoid __indirect_glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_ColorMask,8);
    __GLX_PUT_CHAR(4,red);
    __GLX_PUT_CHAR(5,green);
    __GLX_PUT_CHAR(6,blue);
    __GLX_PUT_CHAR(7,alpha);
    __GLX_END(8);
}

GLvoid __indirect_glDepthMask(GLboolean flag)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_DepthMask,8);
    __GLX_PUT_CHAR(4,flag);
    __GLX_END(8);
}

GLvoid __indirect_glIndexMask(GLuint mask)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_IndexMask,8);
    __GLX_PUT_LONG(4,mask);
    __GLX_END(8);
}

GLvoid __indirect_glAccum(GLenum op, GLfloat value)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Accum,12);
    __GLX_PUT_LONG(4,op);
    __GLX_PUT_FLOAT(8,value);
    __GLX_END(12);
}

GLvoid __indirect_glPopAttrib(GLvoid)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_PopAttrib,4);
    __GLX_END(4);
}

GLvoid __indirect_glPushAttrib(GLbitfield mask)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_PushAttrib,8);
    __GLX_PUT_LONG(4,mask);
    __GLX_END(8);
}

GLvoid __indirect_glMapGrid1d(GLint un, GLdouble u1, GLdouble u2)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MapGrid1d,24);
    __GLX_PUT_DOUBLE(4,u1);
    __GLX_PUT_DOUBLE(12,u2);
    __GLX_PUT_LONG(20,un);
    __GLX_END(24);
}

GLvoid __indirect_glMapGrid1f(GLint un, GLfloat u1, GLfloat u2)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MapGrid1f,16);
    __GLX_PUT_LONG(4,un);
    __GLX_PUT_FLOAT(8,u1);
    __GLX_PUT_FLOAT(12,u2);
    __GLX_END(16);
}

GLvoid __indirect_glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MapGrid2d,44);
    __GLX_PUT_DOUBLE(4,u1);
    __GLX_PUT_DOUBLE(12,u2);
    __GLX_PUT_DOUBLE(20,v1);
    __GLX_PUT_DOUBLE(28,v2);
    __GLX_PUT_LONG(36,un);
    __GLX_PUT_LONG(40,vn);
    __GLX_END(44);
}

GLvoid __indirect_glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MapGrid2f,28);
    __GLX_PUT_LONG(4,un);
    __GLX_PUT_FLOAT(8,u1);
    __GLX_PUT_FLOAT(12,u2);
    __GLX_PUT_LONG(16,vn);
    __GLX_PUT_FLOAT(20,v1);
    __GLX_PUT_FLOAT(24,v2);
    __GLX_END(28);
}

GLvoid __indirect_glEvalCoord1d(GLdouble u)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_EvalCoord1dv,12);
    __GLX_PUT_DOUBLE(4,u);
    __GLX_END(12);
}

GLvoid __indirect_glEvalCoord1dv(const GLdouble *u)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_EvalCoord1dv,12);
    __GLX_PUT_DOUBLE(4,u[0]);
    __GLX_END(12);
}

GLvoid __indirect_glEvalCoord1f(GLfloat u)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_EvalCoord1fv,8);
    __GLX_PUT_FLOAT(4,u);
    __GLX_END(8);
}

GLvoid __indirect_glEvalCoord1fv(const GLfloat *u)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_EvalCoord1fv,8);
    __GLX_PUT_FLOAT(4,u[0]);
    __GLX_END(8);
}

GLvoid __indirect_glEvalCoord2d(GLdouble u, GLdouble v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_EvalCoord2dv,20);
    __GLX_PUT_DOUBLE(4,u);
    __GLX_PUT_DOUBLE(12,v);
    __GLX_END(20);
}

GLvoid __indirect_glEvalCoord2dv(const GLdouble *u)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_EvalCoord2dv,20);
    __GLX_PUT_DOUBLE(4,u[0]);
    __GLX_PUT_DOUBLE(12,u[1]);
    __GLX_END(20);
}

GLvoid __indirect_glEvalCoord2f(GLfloat u, GLfloat v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_EvalCoord2fv,12);
    __GLX_PUT_FLOAT(4,u);
    __GLX_PUT_FLOAT(8,v);
    __GLX_END(12);
}

GLvoid __indirect_glEvalCoord2fv(const GLfloat *u)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_EvalCoord2fv,12);
    __GLX_PUT_FLOAT(4,u[0]);
    __GLX_PUT_FLOAT(8,u[1]);
    __GLX_END(12);
}

GLvoid __indirect_glEvalMesh1(GLenum mode, GLint i1, GLint i2)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_EvalMesh1,16);
    __GLX_PUT_LONG(4,mode);
    __GLX_PUT_LONG(8,i1);
    __GLX_PUT_LONG(12,i2);
    __GLX_END(16);
}

GLvoid __indirect_glEvalPoint1(GLint i)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_EvalPoint1,8);
    __GLX_PUT_LONG(4,i);
    __GLX_END(8);
}

GLvoid __indirect_glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_EvalMesh2,24);
    __GLX_PUT_LONG(4,mode);
    __GLX_PUT_LONG(8,i1);
    __GLX_PUT_LONG(12,i2);
    __GLX_PUT_LONG(16,j1);
    __GLX_PUT_LONG(20,j2);
    __GLX_END(24);
}

GLvoid __indirect_glEvalPoint2(GLint i, GLint j)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_EvalPoint2,12);
    __GLX_PUT_LONG(4,i);
    __GLX_PUT_LONG(8,j);
    __GLX_END(12);
}

GLvoid __indirect_glAlphaFunc(GLenum func, GLclampf ref)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_AlphaFunc,12);
    __GLX_PUT_LONG(4,func);
    __GLX_PUT_FLOAT(8,ref);
    __GLX_END(12);
}

GLvoid __indirect_glBlendFunc(GLenum sfactor, GLenum dfactor)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_BlendFunc,12);
    __GLX_PUT_LONG(4,sfactor);
    __GLX_PUT_LONG(8,dfactor);
    __GLX_END(12);
}

GLvoid __indirect_glLogicOp(GLenum opcode)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_LogicOp,8);
    __GLX_PUT_LONG(4,opcode);
    __GLX_END(8);
}

GLvoid __indirect_glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_StencilFunc,16);
    __GLX_PUT_LONG(4,func);
    __GLX_PUT_LONG(8,ref);
    __GLX_PUT_LONG(12,mask);
    __GLX_END(16);
}

GLvoid __indirect_glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_StencilOp,16);
    __GLX_PUT_LONG(4,fail);
    __GLX_PUT_LONG(8,zfail);
    __GLX_PUT_LONG(12,zpass);
    __GLX_END(16);
}

GLvoid __indirect_glDepthFunc(GLenum func)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_DepthFunc,8);
    __GLX_PUT_LONG(4,func);
    __GLX_END(8);
}

GLvoid __indirect_glPixelZoom(GLfloat xfactor, GLfloat yfactor)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_PixelZoom,12);
    __GLX_PUT_FLOAT(4,xfactor);
    __GLX_PUT_FLOAT(8,yfactor);
    __GLX_END(12);
}

GLvoid __indirect_glPixelTransferf(GLenum pname, GLfloat param)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_PixelTransferf,12);
    __GLX_PUT_LONG(4,pname);
    __GLX_PUT_FLOAT(8,param);
    __GLX_END(12);
}

GLvoid __indirect_glPixelTransferi(GLenum pname, GLint param)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_PixelTransferi,12);
    __GLX_PUT_LONG(4,pname);
    __GLX_PUT_LONG(8,param);
    __GLX_END(12);
}

GLvoid __indirect_glReadBuffer(GLenum mode)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_ReadBuffer,8);
    __GLX_PUT_LONG(4,mode);
    __GLX_END(8);
}

GLvoid __indirect_glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_CopyPixels,24);
    __GLX_PUT_LONG(4,x);
    __GLX_PUT_LONG(8,y);
    __GLX_PUT_LONG(12,width);
    __GLX_PUT_LONG(16,height);
    __GLX_PUT_LONG(20,type);
    __GLX_END(24);
}

GLvoid __indirect_glDepthRange(GLclampd zNear, GLclampd zFar)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_DepthRange,20);
    __GLX_PUT_DOUBLE(4,zNear);
    __GLX_PUT_DOUBLE(12,zFar);
    __GLX_END(20);
}

GLvoid __indirect_glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Frustum,52);
    __GLX_PUT_DOUBLE(4,left);
    __GLX_PUT_DOUBLE(12,right);
    __GLX_PUT_DOUBLE(20,bottom);
    __GLX_PUT_DOUBLE(28,top);
    __GLX_PUT_DOUBLE(36,zNear);
    __GLX_PUT_DOUBLE(44,zFar);
    __GLX_END(52);
}

GLvoid __indirect_glLoadIdentity(GLvoid)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_LoadIdentity,4);
    __GLX_END(4);
}

GLvoid __indirect_glLoadMatrixf(const GLfloat *m)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_LoadMatrixf,68);
    __GLX_PUT_FLOAT_ARRAY(4,m,16);
    __GLX_END(68);
}

GLvoid __indirect_glLoadMatrixd(const GLdouble *m)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_LoadMatrixd,132);
    __GLX_PUT_DOUBLE_ARRAY(4,m,16);
    __GLX_END(132);
}

GLvoid __indirect_glMatrixMode(GLenum mode)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MatrixMode,8);
    __GLX_PUT_LONG(4,mode);
    __GLX_END(8);
}

GLvoid __indirect_glMultMatrixf(const GLfloat *m)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultMatrixf,68);
    __GLX_PUT_FLOAT_ARRAY(4,m,16);
    __GLX_END(68);
}

GLvoid __indirect_glMultMatrixd(const GLdouble *m)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultMatrixd,132);
    __GLX_PUT_DOUBLE_ARRAY(4,m,16);
    __GLX_END(132);
}

GLvoid __indirect_glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Ortho,52);
    __GLX_PUT_DOUBLE(4,left);
    __GLX_PUT_DOUBLE(12,right);
    __GLX_PUT_DOUBLE(20,bottom);
    __GLX_PUT_DOUBLE(28,top);
    __GLX_PUT_DOUBLE(36,zNear);
    __GLX_PUT_DOUBLE(44,zFar);
    __GLX_END(52);
}

GLvoid __indirect_glPopMatrix(GLvoid)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_PopMatrix,4);
    __GLX_END(4);
}

GLvoid __indirect_glPushMatrix(GLvoid)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_PushMatrix,4);
    __GLX_END(4);
}

GLvoid __indirect_glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Rotated,36);
    __GLX_PUT_DOUBLE(4,angle);
    __GLX_PUT_DOUBLE(12,x);
    __GLX_PUT_DOUBLE(20,y);
    __GLX_PUT_DOUBLE(28,z);
    __GLX_END(36);
}

GLvoid __indirect_glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Rotatef,20);
    __GLX_PUT_FLOAT(4,angle);
    __GLX_PUT_FLOAT(8,x);
    __GLX_PUT_FLOAT(12,y);
    __GLX_PUT_FLOAT(16,z);
    __GLX_END(20);
}

GLvoid __indirect_glScaled(GLdouble x, GLdouble y, GLdouble z)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Scaled,28);
    __GLX_PUT_DOUBLE(4,x);
    __GLX_PUT_DOUBLE(12,y);
    __GLX_PUT_DOUBLE(20,z);
    __GLX_END(28);
}

GLvoid __indirect_glScalef(GLfloat x, GLfloat y, GLfloat z)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Scalef,16);
    __GLX_PUT_FLOAT(4,x);
    __GLX_PUT_FLOAT(8,y);
    __GLX_PUT_FLOAT(12,z);
    __GLX_END(16);
}

GLvoid __indirect_glTranslated(GLdouble x, GLdouble y, GLdouble z)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Translated,28);
    __GLX_PUT_DOUBLE(4,x);
    __GLX_PUT_DOUBLE(12,y);
    __GLX_PUT_DOUBLE(20,z);
    __GLX_END(28);
}

GLvoid __indirect_glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Translatef,16);
    __GLX_PUT_FLOAT(4,x);
    __GLX_PUT_FLOAT(8,y);
    __GLX_PUT_FLOAT(12,z);
    __GLX_END(16);
}

GLvoid __indirect_glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Viewport,20);
    __GLX_PUT_LONG(4,x);
    __GLX_PUT_LONG(8,y);
    __GLX_PUT_LONG(12,width);
    __GLX_PUT_LONG(16,height);
    __GLX_END(20);
}

GLvoid __indirect_glPolygonOffset(GLfloat factor, GLfloat units)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_PolygonOffset,12);
    __GLX_PUT_FLOAT(4,factor);
    __GLX_PUT_FLOAT(8,units);
    __GLX_END(12);
}

GLvoid __indirect_glCopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_CopyTexImage1D,32);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,level);
    __GLX_PUT_LONG(12,internalformat);
    __GLX_PUT_LONG(16,x);
    __GLX_PUT_LONG(20,y);
    __GLX_PUT_LONG(24,width);
    __GLX_PUT_LONG(28,border);
    __GLX_END(32);
}

GLvoid __indirect_glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_CopyTexImage2D,36);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,level);
    __GLX_PUT_LONG(12,internalformat);
    __GLX_PUT_LONG(16,x);
    __GLX_PUT_LONG(20,y);
    __GLX_PUT_LONG(24,width);
    __GLX_PUT_LONG(28,height);
    __GLX_PUT_LONG(32,border);
    __GLX_END(36);
}

GLvoid __indirect_glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_CopyTexSubImage1D,28);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,level);
    __GLX_PUT_LONG(12,xoffset);
    __GLX_PUT_LONG(16,x);
    __GLX_PUT_LONG(20,y);
    __GLX_PUT_LONG(24,width);
    __GLX_END(28);
}

GLvoid __indirect_glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_CopyTexSubImage2D,36);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,level);
    __GLX_PUT_LONG(12,xoffset);
    __GLX_PUT_LONG(16,yoffset);
    __GLX_PUT_LONG(20,x);
    __GLX_PUT_LONG(24,y);
    __GLX_PUT_LONG(28,width);
    __GLX_PUT_LONG(32,height);
    __GLX_END(36);
}

GLvoid __indirect_glBindTexture(GLenum target, GLuint texture)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_BindTexture,12);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,texture);
    __GLX_END(12);
}

GLvoid __indirect_glPrioritizeTextures(GLsizei n, const GLuint *textures, const GLclampf *priorities)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    if (n < 0) return;
    cmdlen = 8+n*4+n*4;
    __GLX_BEGIN(X_GLrop_PrioritizeTextures,cmdlen);
    __GLX_PUT_LONG(4,n);
    __GLX_PUT_LONG_ARRAY(8,textures,n);
    __GLX_PUT_FLOAT_ARRAY(8+n*4,priorities,n);
    __GLX_END(cmdlen);
}

GLvoid __indirect_glIndexub(GLubyte c)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Indexubv,8);
    __GLX_PUT_CHAR(4,c);
    __GLX_END(8);
}

GLvoid __indirect_glIndexubv(const GLubyte *c)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Indexubv,8);
    __GLX_PUT_CHAR(4,c[0]);
    __GLX_END(8);
}

GLvoid __indirect_glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_BlendColor,20);
    __GLX_PUT_FLOAT(4,red);
    __GLX_PUT_FLOAT(8,green);
    __GLX_PUT_FLOAT(12,blue);
    __GLX_PUT_FLOAT(16,alpha);
    __GLX_END(20);
}

GLvoid __indirect_glBlendEquation(GLenum mode)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_BlendEquation,8);
    __GLX_PUT_LONG(4,mode);
    __GLX_END(8);
}

GLvoid __indirect_glColorTableParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
    __GLX_DECLARE_VARIABLES();
    compsize = __glColorTableParameterfv_size(pname);
    __GLX_LOAD_VARIABLES();
    cmdlen = 12+compsize*4;
    __GLX_BEGIN(X_GLrop_ColorTableParameterfv,cmdlen);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,pname);
    __GLX_PUT_FLOAT_ARRAY(12,params,compsize);
    __GLX_END(cmdlen);
}

GLvoid __indirect_glColorTableParameteriv(GLenum target, GLenum pname, const GLint *params)
{
    __GLX_DECLARE_VARIABLES();
    compsize = __glColorTableParameteriv_size(pname);
    __GLX_LOAD_VARIABLES();
    cmdlen = 12+compsize*4;
    __GLX_BEGIN(X_GLrop_ColorTableParameteriv,cmdlen);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,pname);
    __GLX_PUT_LONG_ARRAY(12,params,compsize);
    __GLX_END(cmdlen);
}

GLvoid __indirect_glCopyColorTable(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_CopyColorTable,24);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,internalformat);
    __GLX_PUT_LONG(12,x);
    __GLX_PUT_LONG(16,y);
    __GLX_PUT_LONG(20,width);
    __GLX_END(24);
}

GLvoid __indirect_glCopyColorSubTable(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_CopyColorSubTable,24);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,start);
    __GLX_PUT_LONG(12,x);
    __GLX_PUT_LONG(16,y);
    __GLX_PUT_LONG(20,width);
    __GLX_END(24);
}

GLvoid __indirect_glConvolutionParameterf(GLenum target, GLenum pname, GLfloat params)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_ConvolutionParameterf,16);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,pname);
    __GLX_PUT_FLOAT(12,params);
    __GLX_END(16);
}

GLvoid __indirect_glConvolutionParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
    __GLX_DECLARE_VARIABLES();
    compsize = __glConvolutionParameterfv_size(pname);
    __GLX_LOAD_VARIABLES();
    cmdlen = 12+compsize*4;
    __GLX_BEGIN(X_GLrop_ConvolutionParameterfv,cmdlen);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,pname);
    __GLX_PUT_FLOAT_ARRAY(12,params,compsize);
    __GLX_END(cmdlen);
}

GLvoid __indirect_glConvolutionParameteri(GLenum target, GLenum pname, GLint params)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_ConvolutionParameteri,16);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,pname);
    __GLX_PUT_LONG(12,params);
    __GLX_END(16);
}

GLvoid __indirect_glConvolutionParameteriv(GLenum target, GLenum pname, const GLint *params)
{
    __GLX_DECLARE_VARIABLES();
    compsize = __glConvolutionParameteriv_size(pname);
    __GLX_LOAD_VARIABLES();
    cmdlen = 12+compsize*4;
    __GLX_BEGIN(X_GLrop_ConvolutionParameteriv,cmdlen);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,pname);
    __GLX_PUT_LONG_ARRAY(12,params,compsize);
    __GLX_END(cmdlen);
}

GLvoid __indirect_glCopyConvolutionFilter1D(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_CopyConvolutionFilter1D,24);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,internalformat);
    __GLX_PUT_LONG(12,x);
    __GLX_PUT_LONG(16,y);
    __GLX_PUT_LONG(20,width);
    __GLX_END(24);
}

GLvoid __indirect_glCopyConvolutionFilter2D(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_CopyConvolutionFilter2D,28);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,internalformat);
    __GLX_PUT_LONG(12,x);
    __GLX_PUT_LONG(16,y);
    __GLX_PUT_LONG(20,width);
    __GLX_PUT_LONG(24,height);
    __GLX_END(28);
}

GLvoid __indirect_glHistogram(GLenum target, GLsizei width, GLenum internalformat, GLboolean sink)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Histogram,20);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,width);
    __GLX_PUT_LONG(12,internalformat);
    __GLX_PUT_CHAR(16,sink);
    __GLX_END(20);
}

GLvoid __indirect_glMinmax(GLenum target, GLenum internalformat, GLboolean sink)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_Minmax,16);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,internalformat);
    __GLX_PUT_CHAR(12,sink);
    __GLX_END(16);
}

GLvoid __indirect_glResetHistogram(GLenum target)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_ResetHistogram,8);
    __GLX_PUT_LONG(4,target);
    __GLX_END(8);
}

GLvoid __indirect_glResetMinmax(GLenum target)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_ResetMinmax,8);
    __GLX_PUT_LONG(4,target);
    __GLX_END(8);
}

GLvoid __indirect_glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_CopyTexSubImage3D,40);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,level);
    __GLX_PUT_LONG(12,xoffset);
    __GLX_PUT_LONG(16,yoffset);
    __GLX_PUT_LONG(20,zoffset);
    __GLX_PUT_LONG(24,x);
    __GLX_PUT_LONG(28,y);
    __GLX_PUT_LONG(32,width);
    __GLX_PUT_LONG(36,height);
    __GLX_END(40);
}

GLvoid __indirect_glActiveTextureARB(GLenum texture)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_ActiveTextureARB,8);
    __GLX_PUT_LONG(4,texture);
    __GLX_END(8);
}

GLvoid __indirect_glMultiTexCoord1dARB(GLenum target, GLdouble s)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord1dvARB,16);
    __GLX_PUT_DOUBLE(4,s);
    __GLX_PUT_LONG(12,target);
    __GLX_END(16);
}

GLvoid __indirect_glMultiTexCoord1dvARB(GLenum target, const GLdouble *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord1dvARB,16);
    __GLX_PUT_DOUBLE(4,v[0]);
    __GLX_PUT_LONG(12,target);
    __GLX_END(16);
}

GLvoid __indirect_glMultiTexCoord1fARB(GLenum target, GLfloat s)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord1fvARB,12);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_FLOAT(8,s);
    __GLX_END(12);
}

GLvoid __indirect_glMultiTexCoord1fvARB(GLenum target, const GLfloat *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord1fvARB,12);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_FLOAT(8,v[0]);
    __GLX_END(12);
}

GLvoid __indirect_glMultiTexCoord1iARB(GLenum target, GLint s)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord1ivARB,12);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,s);
    __GLX_END(12);
}

GLvoid __indirect_glMultiTexCoord1ivARB(GLenum target, const GLint *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord1ivARB,12);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,v[0]);
    __GLX_END(12);
}

GLvoid __indirect_glMultiTexCoord1sARB(GLenum target, GLshort s)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord1svARB,12);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_SHORT(8,s);
    __GLX_END(12);
}

GLvoid __indirect_glMultiTexCoord1svARB(GLenum target, const GLshort *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord1svARB,12);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_SHORT(8,v[0]);
    __GLX_END(12);
}

GLvoid __indirect_glMultiTexCoord2dARB(GLenum target, GLdouble s, GLdouble t)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord2dvARB,24);
    __GLX_PUT_DOUBLE(4,s);
    __GLX_PUT_DOUBLE(12,t);
    __GLX_PUT_LONG(20,target);
    __GLX_END(24);
}

GLvoid __indirect_glMultiTexCoord2dvARB(GLenum target, const GLdouble *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord2dvARB,24);
    __GLX_PUT_DOUBLE(4,v[0]);
    __GLX_PUT_DOUBLE(12,v[1]);
    __GLX_PUT_LONG(20,target);
    __GLX_END(24);
}

GLvoid __indirect_glMultiTexCoord2fARB(GLenum target, GLfloat s, GLfloat t)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord2fvARB,16);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_FLOAT(8,s);
    __GLX_PUT_FLOAT(12,t);
    __GLX_END(16);
}

GLvoid __indirect_glMultiTexCoord2fvARB(GLenum target, const GLfloat *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord2fvARB,16);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_FLOAT(8,v[0]);
    __GLX_PUT_FLOAT(12,v[1]);
    __GLX_END(16);
}

GLvoid __indirect_glMultiTexCoord2iARB(GLenum target, GLint s, GLint t)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord2ivARB,16);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,s);
    __GLX_PUT_LONG(12,t);
    __GLX_END(16);
}

GLvoid __indirect_glMultiTexCoord2ivARB(GLenum target, const GLint *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord2ivARB,16);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,v[0]);
    __GLX_PUT_LONG(12,v[1]);
    __GLX_END(16);
}

GLvoid __indirect_glMultiTexCoord2sARB(GLenum target, GLshort s, GLshort t)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord2svARB,12);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_SHORT(8,s);
    __GLX_PUT_SHORT(10,t);
    __GLX_END(12);
}

GLvoid __indirect_glMultiTexCoord2svARB(GLenum target, const GLshort *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord2svARB,12);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_SHORT(8,v[0]);
    __GLX_PUT_SHORT(10,v[1]);
    __GLX_END(12);
}

GLvoid __indirect_glMultiTexCoord3dARB(GLenum target, GLdouble s, GLdouble t, GLdouble r)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord3dvARB,32);
    __GLX_PUT_DOUBLE(4,s);
    __GLX_PUT_DOUBLE(12,t);
    __GLX_PUT_DOUBLE(20,r);
    __GLX_PUT_LONG(28,target);
    __GLX_END(32);
}

GLvoid __indirect_glMultiTexCoord3dvARB(GLenum target, const GLdouble *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord3dvARB,32);
    __GLX_PUT_DOUBLE(4,v[0]);
    __GLX_PUT_DOUBLE(12,v[1]);
    __GLX_PUT_DOUBLE(20,v[2]);
    __GLX_PUT_LONG(28,target);
    __GLX_END(32);
}

GLvoid __indirect_glMultiTexCoord3fARB(GLenum target, GLfloat s, GLfloat t, GLfloat r)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord3fvARB,20);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_FLOAT(8,s);
    __GLX_PUT_FLOAT(12,t);
    __GLX_PUT_FLOAT(16,r);
    __GLX_END(20);
}

GLvoid __indirect_glMultiTexCoord3fvARB(GLenum target, const GLfloat *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord3fvARB,20);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_FLOAT(8,v[0]);
    __GLX_PUT_FLOAT(12,v[1]);
    __GLX_PUT_FLOAT(16,v[2]);
    __GLX_END(20);
}

GLvoid __indirect_glMultiTexCoord3iARB(GLenum target, GLint s, GLint t, GLint r)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord3ivARB,20);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,s);
    __GLX_PUT_LONG(12,t);
    __GLX_PUT_LONG(16,r);
    __GLX_END(20);
}

GLvoid __indirect_glMultiTexCoord3ivARB(GLenum target, const GLint *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord3ivARB,20);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,v[0]);
    __GLX_PUT_LONG(12,v[1]);
    __GLX_PUT_LONG(16,v[2]);
    __GLX_END(20);
}

GLvoid __indirect_glMultiTexCoord3sARB(GLenum target, GLshort s, GLshort t, GLshort r)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord3svARB,16);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_SHORT(8,s);
    __GLX_PUT_SHORT(10,t);
    __GLX_PUT_SHORT(12,r);
    __GLX_END(16);
}

GLvoid __indirect_glMultiTexCoord3svARB(GLenum target, const GLshort *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord3svARB,16);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_SHORT(8,v[0]);
    __GLX_PUT_SHORT(10,v[1]);
    __GLX_PUT_SHORT(12,v[2]);
    __GLX_END(16);
}

GLvoid __indirect_glMultiTexCoord4dARB(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord4dvARB,40);
    __GLX_PUT_DOUBLE(4,s);
    __GLX_PUT_DOUBLE(12,t);
    __GLX_PUT_DOUBLE(20,r);
    __GLX_PUT_DOUBLE(28,q);
    __GLX_PUT_LONG(36,target);
    __GLX_END(40);
}

GLvoid __indirect_glMultiTexCoord4dvARB(GLenum target, const GLdouble *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord4dvARB,40);
    __GLX_PUT_DOUBLE(4,v[0]);
    __GLX_PUT_DOUBLE(12,v[1]);
    __GLX_PUT_DOUBLE(20,v[2]);
    __GLX_PUT_DOUBLE(28,v[3]);
    __GLX_PUT_LONG(36,target);
    __GLX_END(40);
}

GLvoid __indirect_glMultiTexCoord4fARB(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord4fvARB,24);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_FLOAT(8,s);
    __GLX_PUT_FLOAT(12,t);
    __GLX_PUT_FLOAT(16,r);
    __GLX_PUT_FLOAT(20,q);
    __GLX_END(24);
}

GLvoid __indirect_glMultiTexCoord4fvARB(GLenum target, const GLfloat *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord4fvARB,24);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_FLOAT(8,v[0]);
    __GLX_PUT_FLOAT(12,v[1]);
    __GLX_PUT_FLOAT(16,v[2]);
    __GLX_PUT_FLOAT(20,v[3]);
    __GLX_END(24);
}

GLvoid __indirect_glMultiTexCoord4iARB(GLenum target, GLint s, GLint t, GLint r, GLint q)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord4ivARB,24);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,s);
    __GLX_PUT_LONG(12,t);
    __GLX_PUT_LONG(16,r);
    __GLX_PUT_LONG(20,q);
    __GLX_END(24);
}

GLvoid __indirect_glMultiTexCoord4ivARB(GLenum target, const GLint *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord4ivARB,24);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_LONG(8,v[0]);
    __GLX_PUT_LONG(12,v[1]);
    __GLX_PUT_LONG(16,v[2]);
    __GLX_PUT_LONG(20,v[3]);
    __GLX_END(24);
}

GLvoid __indirect_glMultiTexCoord4sARB(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord4svARB,16);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_SHORT(8,s);
    __GLX_PUT_SHORT(10,t);
    __GLX_PUT_SHORT(12,r);
    __GLX_PUT_SHORT(14,q);
    __GLX_END(16);
}

GLvoid __indirect_glMultiTexCoord4svARB(GLenum target, const GLshort *v)
{
    __GLX_DECLARE_VARIABLES();
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultiTexCoord4svARB,16);
    __GLX_PUT_LONG(4,target);
    __GLX_PUT_SHORT(8,v[0]);
    __GLX_PUT_SHORT(10,v[1]);
    __GLX_PUT_SHORT(12,v[2]);
    __GLX_PUT_SHORT(14,v[3]);
    __GLX_END(16);
}

GLvoid __indirect_glLoadTransposeMatrixfARB(const GLfloat *m)
{
    __GLX_DECLARE_VARIABLES();
    GLfloat t[16];
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            t[i*4+j] = m[j*4+i];
         }
    }
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_LoadMatrixf,68);
    __GLX_PUT_FLOAT_ARRAY(4,t,16);
    __GLX_END(68);
}

GLvoid __indirect_glMultTransposeMatrixfARB(const GLfloat *m)
{
    __GLX_DECLARE_VARIABLES();
    GLfloat t[16];
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            t[i*4+j] = m[j*4+i];
         }
    }
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultMatrixf,68);
    __GLX_PUT_FLOAT_ARRAY(4,t,16);
    __GLX_END(68);
}

GLvoid __indirect_glLoadTransposeMatrixdARB(const GLdouble *m)
{
    __GLX_DECLARE_VARIABLES();
    GLdouble t[16];
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            t[i*4+j] = m[j*4+i];
         }
    }
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_LoadMatrixd,132);
    __GLX_PUT_DOUBLE_ARRAY(4,t,16);
    __GLX_END(132);
}

GLvoid __indirect_glMultTransposeMatrixdARB(const GLdouble *m)
{
    __GLX_DECLARE_VARIABLES();
    GLdouble t[16];
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            t[i*4+j] = m[j*4+i];
         }
    }
    __GLX_LOAD_VARIABLES();
    __GLX_BEGIN(X_GLrop_MultMatrixd,132);
    __GLX_PUT_DOUBLE_ARRAY(4,t,16);
    __GLX_END(132);
}

