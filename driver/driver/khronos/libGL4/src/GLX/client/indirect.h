/* $XFree86: xc/lib/GL/glx/indirect.h,v 1.4 2002/02/22 21:32:54 dawes Exp $ */
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
 *
 */

#ifndef _INDIRECT_H_
#define _INDIRECT_H_

/* NOTE: This file could be automatically generated */

GLvoid __indirect_glAccum(GLenum op, GLfloat value);
GLvoid __indirect_glAlphaFunc(GLenum func, GLclampf ref);
GLboolean __indirect_glAreTexturesResident(GLsizei n, const GLuint *textures, GLboolean *residences);
GLboolean __indirect_glAreTexturesResidentEXT(GLsizei n, const GLuint *textures, GLboolean *residences);
GLvoid __indirect_glArrayElement(GLint i);
GLvoid __indirect_glBegin(GLenum mode);
GLvoid __indirect_glBindTexture(GLenum target, GLuint texture);
GLvoid __indirect_glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
GLvoid __indirect_glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
GLvoid __indirect_glBlendEquation(GLenum mode);
GLvoid __indirect_glBlendFunc(GLenum sfactor, GLenum dfactor);
GLvoid __indirect_glCallList(GLuint list);
GLvoid __indirect_glCallLists(GLsizei n, GLenum type, const GLvoid *lists);
GLvoid __indirect_glClear(GLbitfield mask);
GLvoid __indirect_glClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
GLvoid __indirect_glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
GLvoid __indirect_glClearDepth(GLclampd depth);
GLvoid __indirect_glClearIndex(GLfloat c);
GLvoid __indirect_glClearStencil(GLint s);
GLvoid __indirect_glClipPlane(GLenum plane, const GLdouble *equation);
GLvoid __indirect_glColor3b(GLbyte red, GLbyte green, GLbyte blue);
GLvoid __indirect_glColor3bv(const GLbyte *v);
GLvoid __indirect_glColor3d(GLdouble red, GLdouble green, GLdouble blue);
GLvoid __indirect_glColor3dv(const GLdouble *v);
GLvoid __indirect_glColor3f(GLfloat red, GLfloat green, GLfloat blue);
GLvoid __indirect_glColor3fv(const GLfloat *v);
GLvoid __indirect_glColor3i(GLint red, GLint green, GLint blue);
GLvoid __indirect_glColor3iv(const GLint *v);
GLvoid __indirect_glColor3s(GLshort red, GLshort green, GLshort blue);
GLvoid __indirect_glColor3sv(const GLshort *v);
GLvoid __indirect_glColor3ub(GLubyte red, GLubyte green, GLubyte blue);
GLvoid __indirect_glColor3ubv(const GLubyte *v);
GLvoid __indirect_glColor3ui(GLuint red, GLuint green, GLuint blue);
GLvoid __indirect_glColor3uiv(const GLuint *v);
GLvoid __indirect_glColor3us(GLushort red, GLushort green, GLushort blue);
GLvoid __indirect_glColor3usv(const GLushort *v);
GLvoid __indirect_glColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
GLvoid __indirect_glColor4bv(const GLbyte *v);
GLvoid __indirect_glColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
GLvoid __indirect_glColor4dv(const GLdouble *v);
GLvoid __indirect_glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
GLvoid __indirect_glColor4fv(const GLfloat *v);
GLvoid __indirect_glColor4i(GLint red, GLint green, GLint blue, GLint alpha);
GLvoid __indirect_glColor4iv(const GLint *v);
GLvoid __indirect_glColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha);
GLvoid __indirect_glColor4sv(const GLshort *v);
GLvoid __indirect_glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
GLvoid __indirect_glColor4ubv(const GLubyte *v);
GLvoid __indirect_glColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha);
GLvoid __indirect_glColor4uiv(const GLuint *v);
GLvoid __indirect_glColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha);
GLvoid __indirect_glColor4usv(const GLushort *v);
GLvoid __indirect_glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
GLvoid __indirect_glColorMaterial(GLenum face, GLenum mode);
GLvoid __indirect_glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
GLvoid __indirect_glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
GLvoid __indirect_glCopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
GLvoid __indirect_glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
GLvoid __indirect_glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
GLvoid __indirect_glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GLvoid __indirect_glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GLvoid __indirect_glCullFace(GLenum mode);
GLvoid __indirect_glDeleteLists(GLuint list, GLsizei range);
GLvoid __indirect_glDeleteTextures(GLsizei n, const GLuint *textures);
GLvoid __indirect_glDeleteTexturesEXT(GLsizei n, const GLuint *textures);
GLvoid __indirect_glDepthFunc(GLenum func);
GLvoid __indirect_glDepthMask(GLboolean flag);
GLvoid __indirect_glDepthRange(GLclampd zNear, GLclampd zFar);
GLvoid __indirect_glDisable(GLenum cap);
GLvoid __indirect_glDisableClientState(GLenum array);
GLvoid __indirect_glDrawArrays(GLenum mode, GLint first, GLsizei count);
GLvoid __indirect_glDrawBuffer(GLenum mode);
GLvoid __indirect_glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
GLvoid __indirect_glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image);
GLvoid __indirect_glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
GLvoid __indirect_glEdgeFlag(GLboolean flag);
GLvoid __indirect_glEdgeFlagPointer(GLsizei stride, const GLboolean *pointer);
GLvoid __indirect_glEdgeFlagv(const GLboolean *flag);
GLvoid __indirect_glEnable(GLenum cap);
GLvoid __indirect_glEnableClientState(GLenum array);
GLvoid __indirect_glEnd(GLvoid);
GLvoid __indirect_glEndList(GLvoid);
GLvoid __indirect_glEvalCoord1d(GLdouble u);
GLvoid __indirect_glEvalCoord1dv(const GLdouble *u);
GLvoid __indirect_glEvalCoord1f(GLfloat u);
GLvoid __indirect_glEvalCoord1fv(const GLfloat *u);
GLvoid __indirect_glEvalCoord2d(GLdouble u, GLdouble v);
GLvoid __indirect_glEvalCoord2dv(const GLdouble *u);
GLvoid __indirect_glEvalCoord2f(GLfloat u, GLfloat v);
GLvoid __indirect_glEvalCoord2fv(const GLfloat *u);
GLvoid __indirect_glEvalMesh1(GLenum mode, GLint i1, GLint i2);
GLvoid __indirect_glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
GLvoid __indirect_glEvalPoint1(GLint i);
GLvoid __indirect_glEvalPoint2(GLint i, GLint j);
GLvoid __indirect_glFeedbackBuffer(GLsizei size, GLenum type, GLfloat *buffer);
GLvoid __indirect_glFinish(GLvoid);
GLvoid __indirect_glFlush(GLvoid);
GLvoid __indirect_glFogf(GLenum pname, GLfloat param);
GLvoid __indirect_glFogfv(GLenum pname, const GLfloat *params);
GLvoid __indirect_glFogi(GLenum pname, GLint param);
GLvoid __indirect_glFogiv(GLenum pname, const GLint *params);
GLvoid __indirect_glFrontFace(GLenum mode);
GLvoid __indirect_glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
GLuint __indirect_glGenLists(GLsizei range);
GLvoid __indirect_glGenTextures(GLsizei n, GLuint *textures);
GLvoid __indirect_glGenTexturesEXT(GLsizei n, GLuint *textures);
GLvoid __indirect_glGetBooleanv(GLenum val, GLboolean *b);
GLvoid __indirect_glGetClipPlane(GLenum plane, GLdouble *equation);
GLvoid __indirect_glGetDoublev(GLenum val, GLdouble *d);
GLenum __indirect_glGetError(GLvoid);
GLvoid __indirect_glGetFloatv(GLenum val, GLfloat *f);
GLvoid __indirect_glGetIntegerv(GLenum val, GLint *i);
GLvoid __indirect_glGetLightfv(GLenum light, GLenum pname, GLfloat *params);
GLvoid __indirect_glGetLightiv(GLenum light, GLenum pname, GLint *params);
GLvoid __indirect_glGetMapdv(GLenum target, GLenum query, GLdouble *v);
GLvoid __indirect_glGetMapfv(GLenum target, GLenum query, GLfloat *v);
GLvoid __indirect_glGetMapiv(GLenum target, GLenum query, GLint *v);
GLvoid __indirect_glGetMaterialfv(GLenum face, GLenum pname, GLfloat *params);
GLvoid __indirect_glGetMaterialiv(GLenum face, GLenum pname, GLint *params);
GLvoid __indirect_glGetPixelMapfv(GLenum map, GLfloat *values);
GLvoid __indirect_glGetPixelMapuiv(GLenum map, GLuint *values);
GLvoid __indirect_glGetPixelMapusv(GLenum map, GLushort *values);
GLvoid __indirect_glGetPointerv(GLenum pname, GLvoid **params);
GLvoid __indirect_glGetPolygonStipple(GLubyte *mask);
const GLubyte *__indirect_glGetString(GLenum name);
GLvoid __indirect_glGetTexEnvfv(GLenum target, GLenum pname, GLfloat *params);
GLvoid __indirect_glGetTexEnviv(GLenum target, GLenum pname, GLint *params);
GLvoid __indirect_glGetTexGendv(GLenum coord, GLenum pname, GLdouble *params);
GLvoid __indirect_glGetTexGenfv(GLenum coord, GLenum pname, GLfloat *params);
GLvoid __indirect_glGetTexGeniv(GLenum coord, GLenum pname, GLint *params);
GLvoid __indirect_glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *texels);
GLvoid __indirect_glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat *params);
GLvoid __indirect_glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params);
GLvoid __indirect_glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params);
GLvoid __indirect_glGetTexParameteriv(GLenum target, GLenum pname, GLint *params);
GLvoid __indirect_glHint(GLenum target, GLenum mode);
GLvoid __indirect_glIndexMask(GLuint mask);
GLvoid __indirect_glIndexPointer(GLenum type, GLsizei stride, const GLvoid *pointer);
GLvoid __indirect_glIndexd(GLdouble c);
GLvoid __indirect_glIndexdv(const GLdouble *c);
GLvoid __indirect_glIndexf(GLfloat c);
GLvoid __indirect_glIndexfv(const GLfloat *c);
GLvoid __indirect_glIndexi(GLint c);
GLvoid __indirect_glIndexiv(const GLint *c);
GLvoid __indirect_glIndexs(GLshort c);
GLvoid __indirect_glIndexsv(const GLshort *c);
GLvoid __indirect_glIndexub(GLubyte c);
GLvoid __indirect_glIndexubv(const GLubyte *c);
GLvoid __indirect_glInitNames(GLvoid);
GLvoid __indirect_glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer);
GLboolean __indirect_glIsEnabled(GLenum cap);
GLboolean __indirect_glIsList(GLuint list);
GLboolean __indirect_glIsTexture(GLuint texture);
GLboolean __indirect_glIsTextureEXT(GLuint texture);
GLvoid __indirect_glLightModelf(GLenum pname, GLfloat param);
GLvoid __indirect_glLightModelfv(GLenum pname, const GLfloat *params);
GLvoid __indirect_glLightModeli(GLenum pname, GLint param);
GLvoid __indirect_glLightModeliv(GLenum pname, const GLint *params);
GLvoid __indirect_glLightf(GLenum light, GLenum pname, GLfloat param);
GLvoid __indirect_glLightfv(GLenum light, GLenum pname, const GLfloat *params);
GLvoid __indirect_glLighti(GLenum light, GLenum pname, GLint param);
GLvoid __indirect_glLightiv(GLenum light, GLenum pname, const GLint *params);
GLvoid __indirect_glLineStipple(GLint factor, GLushort pattern);
GLvoid __indirect_glLineWidth(GLfloat width);
GLvoid __indirect_glListBase(GLuint base);
GLvoid __indirect_glLoadIdentity(GLvoid);
GLvoid __indirect_glLoadMatrixd(const GLdouble *m);
GLvoid __indirect_glLoadMatrixf(const GLfloat *m);
GLvoid __indirect_glLoadName(GLuint name);
GLvoid __indirect_glLogicOp(GLenum opcode);
GLvoid __indirect_glMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *pnts);
GLvoid __indirect_glMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *pnts);
GLvoid __indirect_glMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustr, GLint uord, GLdouble v1, GLdouble v2, GLint vstr, GLint vord, const GLdouble *pnts);
GLvoid __indirect_glMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustr, GLint uord, GLfloat v1, GLfloat v2, GLint vstr, GLint vord, const GLfloat *pnts);
GLvoid __indirect_glMapGrid1d(GLint un, GLdouble u1, GLdouble u2);
GLvoid __indirect_glMapGrid1f(GLint un, GLfloat u1, GLfloat u2);
GLvoid __indirect_glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
GLvoid __indirect_glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
GLvoid __indirect_glMaterialf(GLenum face, GLenum pname, GLfloat param);
GLvoid __indirect_glMaterialfv(GLenum face, GLenum pname, const GLfloat *params);
GLvoid __indirect_glMateriali(GLenum face, GLenum pname, GLint param);
GLvoid __indirect_glMaterialiv(GLenum face, GLenum pname, const GLint *params);
GLvoid __indirect_glMatrixMode(GLenum mode);
GLvoid __indirect_glMultMatrixd(const GLdouble *m);
GLvoid __indirect_glMultMatrixf(const GLfloat *m);
GLvoid __indirect_glNewList(GLuint list, GLenum mode);
GLvoid __indirect_glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz);
GLvoid __indirect_glNormal3bv(const GLbyte *v);
GLvoid __indirect_glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz);
GLvoid __indirect_glNormal3dv(const GLdouble *v);
GLvoid __indirect_glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz);
GLvoid __indirect_glNormal3fv(const GLfloat *v);
GLvoid __indirect_glNormal3i(GLint nx, GLint ny, GLint nz);
GLvoid __indirect_glNormal3iv(const GLint *v);
GLvoid __indirect_glNormal3s(GLshort nx, GLshort ny, GLshort nz);
GLvoid __indirect_glNormal3sv(const GLshort *v);
GLvoid __indirect_glNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer);
GLvoid __indirect_glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
GLvoid __indirect_glPassThrough(GLfloat token);
GLvoid __indirect_glPixelMapfv(GLenum map, GLint mapsize, const GLfloat *values);
GLvoid __indirect_glPixelMapuiv(GLenum map, GLint mapsize, const GLuint *values);
GLvoid __indirect_glPixelMapusv(GLenum map, GLint mapsize, const GLushort *values);
GLvoid __indirect_glPixelStoref(GLenum pname, GLfloat param);
GLvoid __indirect_glPixelStorei(GLenum pname, GLint param);
GLvoid __indirect_glPixelTransferf(GLenum pname, GLfloat param);
GLvoid __indirect_glPixelTransferi(GLenum pname, GLint param);
GLvoid __indirect_glPixelZoom(GLfloat xfactor, GLfloat yfactor);
GLvoid __indirect_glPointSize(GLfloat size);
GLvoid __indirect_glPolygonMode(GLenum face, GLenum mode);
GLvoid __indirect_glPolygonOffset(GLfloat factor, GLfloat units);
GLvoid __indirect_glPolygonStipple(const GLubyte *mask);
GLvoid __indirect_glPopAttrib(GLvoid);
GLvoid __indirect_glPopClientAttrib(GLvoid);
GLvoid __indirect_glPopMatrix(GLvoid);
GLvoid __indirect_glPopName(GLvoid);
GLvoid __indirect_glPrioritizeTextures(GLsizei n, const GLuint *textures, const GLclampf *priorities);
GLvoid __indirect_glPushAttrib(GLbitfield mask);
GLvoid __indirect_glPushClientAttrib(GLuint mask);
GLvoid __indirect_glPushMatrix(GLvoid);
GLvoid __indirect_glPushName(GLuint name);
GLvoid __indirect_glRasterPos2d(GLdouble x, GLdouble y);
GLvoid __indirect_glRasterPos2dv(const GLdouble *v);
GLvoid __indirect_glRasterPos2f(GLfloat x, GLfloat y);
GLvoid __indirect_glRasterPos2fv(const GLfloat *v);
GLvoid __indirect_glRasterPos2i(GLint x, GLint y);
GLvoid __indirect_glRasterPos2iv(const GLint *v);
GLvoid __indirect_glRasterPos2s(GLshort x, GLshort y);
GLvoid __indirect_glRasterPos2sv(const GLshort *v);
GLvoid __indirect_glRasterPos3d(GLdouble x, GLdouble y, GLdouble z);
GLvoid __indirect_glRasterPos3dv(const GLdouble *v);
GLvoid __indirect_glRasterPos3f(GLfloat x, GLfloat y, GLfloat z);
GLvoid __indirect_glRasterPos3fv(const GLfloat *v);
GLvoid __indirect_glRasterPos3i(GLint x, GLint y, GLint z);
GLvoid __indirect_glRasterPos3iv(const GLint *v);
GLvoid __indirect_glRasterPos3s(GLshort x, GLshort y, GLshort z);
GLvoid __indirect_glRasterPos3sv(const GLshort *v);
GLvoid __indirect_glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
GLvoid __indirect_glRasterPos4dv(const GLdouble *v);
GLvoid __indirect_glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GLvoid __indirect_glRasterPos4fv(const GLfloat *v);
GLvoid __indirect_glRasterPos4i(GLint x, GLint y, GLint z, GLint w);
GLvoid __indirect_glRasterPos4iv(const GLint *v);
GLvoid __indirect_glRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w);
GLvoid __indirect_glRasterPos4sv(const GLshort *v);
GLvoid __indirect_glReadBuffer(GLenum mode);
GLvoid __indirect_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
GLvoid __indirect_glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
GLvoid __indirect_glRectdv(const GLdouble *v1, const GLdouble *v2);
GLvoid __indirect_glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
GLvoid __indirect_glRectfv(const GLfloat *v1, const GLfloat *v2);
GLvoid __indirect_glRecti(GLint x1, GLint y1, GLint x2, GLint y2);
GLvoid __indirect_glRectiv(const GLint *v1, const GLint *v2);
GLvoid __indirect_glRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
GLvoid __indirect_glRectsv(const GLshort *v1, const GLshort *v2);
GLint __indirect_glRenderMode(GLenum mode);
GLvoid __indirect_glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
GLvoid __indirect_glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
GLvoid __indirect_glScaled(GLdouble x, GLdouble y, GLdouble z);
GLvoid __indirect_glScalef(GLfloat x, GLfloat y, GLfloat z);
GLvoid __indirect_glScissor(GLint x, GLint y, GLsizei width, GLsizei height);
GLvoid __indirect_glSelectBuffer(GLsizei numnames, GLuint *buffer);
GLvoid __indirect_glShadeModel(GLenum mode);
GLvoid __indirect_glStencilFunc(GLenum func, GLint ref, GLuint mask);
GLvoid __indirect_glStencilMask(GLuint mask);
GLvoid __indirect_glStencilOp(GLenum fail, GLenum zfail, GLenum zpass);
GLvoid __indirect_glTexCoord1d(GLdouble s);
GLvoid __indirect_glTexCoord1dv(const GLdouble *v);
GLvoid __indirect_glTexCoord1f(GLfloat s);
GLvoid __indirect_glTexCoord1fv(const GLfloat *v);
GLvoid __indirect_glTexCoord1i(GLint s);
GLvoid __indirect_glTexCoord1iv(const GLint *v);
GLvoid __indirect_glTexCoord1s(GLshort s);
GLvoid __indirect_glTexCoord1sv(const GLshort *v);
GLvoid __indirect_glTexCoord2d(GLdouble s, GLdouble t);
GLvoid __indirect_glTexCoord2dv(const GLdouble *v);
GLvoid __indirect_glTexCoord2f(GLfloat s, GLfloat t);
GLvoid __indirect_glTexCoord2fv(const GLfloat *v);
GLvoid __indirect_glTexCoord2i(GLint s, GLint t);
GLvoid __indirect_glTexCoord2iv(const GLint *v);
GLvoid __indirect_glTexCoord2s(GLshort s, GLshort t);
GLvoid __indirect_glTexCoord2sv(const GLshort *v);
GLvoid __indirect_glTexCoord3d(GLdouble s, GLdouble t, GLdouble r);
GLvoid __indirect_glTexCoord3dv(const GLdouble *v);
GLvoid __indirect_glTexCoord3f(GLfloat s, GLfloat t, GLfloat r);
GLvoid __indirect_glTexCoord3fv(const GLfloat *v);
GLvoid __indirect_glTexCoord3i(GLint s, GLint t, GLint r);
GLvoid __indirect_glTexCoord3iv(const GLint *v);
GLvoid __indirect_glTexCoord3s(GLshort s, GLshort t, GLshort r);
GLvoid __indirect_glTexCoord3sv(const GLshort *v);
GLvoid __indirect_glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
GLvoid __indirect_glTexCoord4dv(const GLdouble *v);
GLvoid __indirect_glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
GLvoid __indirect_glTexCoord4fv(const GLfloat *v);
GLvoid __indirect_glTexCoord4i(GLint s, GLint t, GLint r, GLint q);
GLvoid __indirect_glTexCoord4iv(const GLint *v);
GLvoid __indirect_glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q);
GLvoid __indirect_glTexCoord4sv(const GLshort *v);
GLvoid __indirect_glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
GLvoid __indirect_glTexEnvf(GLenum target, GLenum pname, GLfloat param);
GLvoid __indirect_glTexEnvfv(GLenum target, GLenum pname, const GLfloat *params);
GLvoid __indirect_glTexEnvi(GLenum target, GLenum pname, GLint param);
GLvoid __indirect_glTexEnviv(GLenum target, GLenum pname, const GLint *params);
GLvoid __indirect_glTexGend(GLenum coord, GLenum pname, GLdouble param);
GLvoid __indirect_glTexGendv(GLenum coord, GLenum pname, const GLdouble *params);
GLvoid __indirect_glTexGenf(GLenum coord, GLenum pname, GLfloat param);
GLvoid __indirect_glTexGenfv(GLenum coord, GLenum pname, const GLfloat *params);
GLvoid __indirect_glTexGeni(GLenum coord, GLenum pname, GLint param);
GLvoid __indirect_glTexGeniv(GLenum coord, GLenum pname, const GLint *params);
GLvoid __indirect_glTexImage1D(GLenum target, GLint level, GLint components, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *image);
GLvoid __indirect_glTexImage2D(GLenum target, GLint level, GLint components, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *image);
GLvoid __indirect_glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *image);
GLvoid __indirect_glTexParameterf(GLenum target, GLenum pname, GLfloat param);
GLvoid __indirect_glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params);
GLvoid __indirect_glTexParameteri(GLenum target, GLenum pname, GLint param);
GLvoid __indirect_glTexParameteriv(GLenum target, GLenum pname, const GLint *params);
GLvoid __indirect_glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *image);
GLvoid __indirect_glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image);
GLvoid __indirect_glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *image);
GLvoid __indirect_glTranslated(GLdouble x, GLdouble y, GLdouble z);
GLvoid __indirect_glTranslatef(GLfloat x, GLfloat y, GLfloat z);
GLvoid __indirect_glVertex2d(GLdouble x, GLdouble y);
GLvoid __indirect_glVertex2dv(const GLdouble *v);
GLvoid __indirect_glVertex2f(GLfloat x, GLfloat y);
GLvoid __indirect_glVertex2fv(const GLfloat *v);
GLvoid __indirect_glVertex2i(GLint x, GLint y);
GLvoid __indirect_glVertex2iv(const GLint *v);
GLvoid __indirect_glVertex2s(GLshort x, GLshort y);
GLvoid __indirect_glVertex2sv(const GLshort *v);
GLvoid __indirect_glVertex3d(GLdouble x, GLdouble y, GLdouble z);
GLvoid __indirect_glVertex3dv(const GLdouble *v);
GLvoid __indirect_glVertex3f(GLfloat x, GLfloat y, GLfloat z);
GLvoid __indirect_glVertex3fv(const GLfloat *v);
GLvoid __indirect_glVertex3i(GLint x, GLint y, GLint z);
GLvoid __indirect_glVertex3iv(const GLint *v);
GLvoid __indirect_glVertex3s(GLshort x, GLshort y, GLshort z);
GLvoid __indirect_glVertex3sv(const GLshort *v);
GLvoid __indirect_glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
GLvoid __indirect_glVertex4dv(const GLdouble *v);
GLvoid __indirect_glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GLvoid __indirect_glVertex4fv(const GLfloat *v);
GLvoid __indirect_glVertex4i(GLint x, GLint y, GLint z, GLint w);
GLvoid __indirect_glVertex4iv(const GLint *v);
GLvoid __indirect_glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w);
GLvoid __indirect_glVertex4sv(const GLshort *v);
GLvoid __indirect_glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
GLvoid __indirect_glViewport(GLint x, GLint y, GLsizei width, GLsizei height);


GLvoid __indirect_glActiveTextureARB(GLenum texture);
GLvoid __indirect_glClientActiveTextureARB(GLenum texture);
GLvoid __indirect_glMultiTexCoord1dARB(GLenum target, GLdouble s);
GLvoid __indirect_glMultiTexCoord1dvARB(GLenum target, const GLdouble *v);
GLvoid __indirect_glMultiTexCoord1fARB(GLenum target, GLfloat s);
GLvoid __indirect_glMultiTexCoord1fvARB(GLenum target, const GLfloat *v);
GLvoid __indirect_glMultiTexCoord1iARB(GLenum target, GLint s);
GLvoid __indirect_glMultiTexCoord1ivARB(GLenum target, const GLint *v);
GLvoid __indirect_glMultiTexCoord1sARB(GLenum target, GLshort s);
GLvoid __indirect_glMultiTexCoord1svARB(GLenum target, const GLshort *v);
GLvoid __indirect_glMultiTexCoord2dARB(GLenum target, GLdouble s, GLdouble t);
GLvoid __indirect_glMultiTexCoord2dvARB(GLenum target, const GLdouble *v);
GLvoid __indirect_glMultiTexCoord2fARB(GLenum target, GLfloat s, GLfloat t);
GLvoid __indirect_glMultiTexCoord2fvARB(GLenum target, const GLfloat *v);
GLvoid __indirect_glMultiTexCoord2iARB(GLenum target, GLint s, GLint t);
GLvoid __indirect_glMultiTexCoord2ivARB(GLenum target, const GLint *v);
GLvoid __indirect_glMultiTexCoord2sARB(GLenum target, GLshort s, GLshort t);
GLvoid __indirect_glMultiTexCoord2svARB(GLenum target, const GLshort *v);
GLvoid __indirect_glMultiTexCoord3dARB(GLenum target, GLdouble s, GLdouble t, GLdouble r);
GLvoid __indirect_glMultiTexCoord3dvARB(GLenum target, const GLdouble *v);
GLvoid __indirect_glMultiTexCoord3fARB(GLenum target, GLfloat s, GLfloat t, GLfloat r);
GLvoid __indirect_glMultiTexCoord3fvARB(GLenum target, const GLfloat *v);
GLvoid __indirect_glMultiTexCoord3iARB(GLenum target, GLint s, GLint t, GLint r);
GLvoid __indirect_glMultiTexCoord3ivARB(GLenum target, const GLint *v);
GLvoid __indirect_glMultiTexCoord3sARB(GLenum target, GLshort s, GLshort t, GLshort r);
GLvoid __indirect_glMultiTexCoord3svARB(GLenum target, const GLshort *v);
GLvoid __indirect_glMultiTexCoord4dARB(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
GLvoid __indirect_glMultiTexCoord4dvARB(GLenum target, const GLdouble *v);
GLvoid __indirect_glMultiTexCoord4fARB(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
GLvoid __indirect_glMultiTexCoord4fvARB(GLenum target, const GLfloat *v);
GLvoid __indirect_glMultiTexCoord4iARB(GLenum target, GLint s, GLint t, GLint r, GLint q);
GLvoid __indirect_glMultiTexCoord4ivARB(GLenum target, const GLint *v);
GLvoid __indirect_glMultiTexCoord4sARB(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
GLvoid __indirect_glMultiTexCoord4svARB(GLenum target, const GLshort *v);


GLvoid __indirect_glLoadTransposeMatrixfARB(const GLfloat *m);
GLvoid __indirect_glMultTransposeMatrixfARB(const GLfloat *m);
GLvoid __indirect_glLoadTransposeMatrixdARB(const GLdouble *m);
GLvoid __indirect_glMultTransposeMatrixdARB(const GLdouble *m);

#endif /* _INDIRECT_H_ */
