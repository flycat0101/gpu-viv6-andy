#include "gl.h"
#include "gl/gl_core.h"
#include "glcore/g_disp.h"

#ifndef APIENTRY
#define APIENTRY
#endif

extern __GLdispatchTable* _glapi_get_dispatch();
#define __GL_GET_DISPATCH_TABLE (_glapi_get_dispatch())
#define __GLVVT_FUNC(func) gl##func

GLvoid APIENTRY __GLVVT_FUNC(NewList)(GLuint list, GLenum mode)
{
    __GL_GET_DISPATCH_TABLE->NewList(list, mode);
}

GLvoid APIENTRY __GLVVT_FUNC(EndList)(GLvoid)
{
    __GL_GET_DISPATCH_TABLE->EndList();
}

GLvoid APIENTRY __GLVVT_FUNC(CallList)(GLuint list)
{
    __GL_GET_DISPATCH_TABLE->CallList(list);
}

GLvoid APIENTRY __GLVVT_FUNC(CallLists)(GLsizei n, GLenum type, const GLvoid *lists)
{
    __GL_GET_DISPATCH_TABLE->CallLists(n, type, lists);
}

GLvoid APIENTRY __GLVVT_FUNC(DeleteLists)(GLuint list, GLsizei range)
{
    __GL_GET_DISPATCH_TABLE->DeleteLists(list, range);
}

GLuint APIENTRY __GLVVT_FUNC(GenLists)(GLsizei range)
{
   return __GL_GET_DISPATCH_TABLE->GenLists(range);
}

GLvoid APIENTRY __GLVVT_FUNC(ListBase)(GLuint base)
{
    __GL_GET_DISPATCH_TABLE->ListBase(base);
}

GLvoid APIENTRY __GLVVT_FUNC(Begin)(GLenum mode)
{
    __GL_GET_DISPATCH_TABLE->Begin(mode);
}

GLvoid APIENTRY __GLVVT_FUNC(Bitmap)(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig,
                            GLfloat xmove, GLfloat ymove, const GLubyte *bitmap)
{
    __GL_GET_DISPATCH_TABLE->Bitmap(width, height, xorig, yorig, xmove, ymove, bitmap);
}

GLvoid APIENTRY __GLVVT_FUNC(Color3b)(GLbyte red, GLbyte green, GLbyte blue)
{
    __GL_GET_DISPATCH_TABLE->Color3b(red,green, blue);
}

GLvoid APIENTRY __GLVVT_FUNC(Color3bv)(const GLbyte *v)
{
    __GL_GET_DISPATCH_TABLE->Color3bv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Color3d)(GLdouble red, GLdouble green, GLdouble blue)
{
    __GL_GET_DISPATCH_TABLE->Color3d(red, green, blue);
}

GLvoid APIENTRY __GLVVT_FUNC(Color3dv)(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE->Color3dv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Color3f)(GLfloat red, GLfloat green, GLfloat blue)
{
    __GL_GET_DISPATCH_TABLE->Color3f(red, green, blue);
}

GLvoid APIENTRY __GLVVT_FUNC(Color3fv)(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE->Color3fv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Color3i)(GLint red, GLint green, GLint blue)
{
    __GL_GET_DISPATCH_TABLE->Color3i(red, green, blue);
}

GLvoid APIENTRY __GLVVT_FUNC(Color3iv)(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE->Color3iv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Color3s)(GLshort red, GLshort green, GLshort blue)
{
    __GL_GET_DISPATCH_TABLE->Color3s(red, green, blue);
}

GLvoid APIENTRY __GLVVT_FUNC(Color3sv)(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE->Color3sv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Color3ub)(GLubyte red, GLubyte green, GLubyte blue)
{
    __GL_GET_DISPATCH_TABLE->Color3ub(red, green, blue);
}

GLvoid APIENTRY __GLVVT_FUNC(Color3ubv)(const GLubyte *v)
{
    __GL_GET_DISPATCH_TABLE->Color3ubv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Color3ui)(GLuint red, GLuint green, GLuint blue)
{
    __GL_GET_DISPATCH_TABLE->Color3ui(red, green, blue);
}

GLvoid APIENTRY __GLVVT_FUNC(Color3uiv)(const GLuint *v)
{
    __GL_GET_DISPATCH_TABLE->Color3uiv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Color3us)(GLushort red, GLushort green, GLushort blue)
{
    __GL_GET_DISPATCH_TABLE->Color3us(red, green, blue);
}

GLvoid APIENTRY __GLVVT_FUNC(Color3usv)(const GLushort *v)
{
    __GL_GET_DISPATCH_TABLE->Color3usv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Color4b)(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha)
{
    __GL_GET_DISPATCH_TABLE->Color4b(red, green, blue, alpha);
}

GLvoid APIENTRY __GLVVT_FUNC(Color4bv)(const GLbyte *v)
{
    __GL_GET_DISPATCH_TABLE->Color4bv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Color4d)(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha)
{
    __GL_GET_DISPATCH_TABLE->Color4d(red, green, blue, alpha);
}

GLvoid APIENTRY __GLVVT_FUNC(Color4dv)(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE->Color4dv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Color4f)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    __GL_GET_DISPATCH_TABLE->Color4f(red, green, blue, alpha);
}

GLvoid APIENTRY __GLVVT_FUNC(Color4fv)(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE->Color4fv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Color4i)(GLint red, GLint green, GLint blue, GLint alpha)
{
    __GL_GET_DISPATCH_TABLE->Color4i(red, green, blue, alpha);
}

GLvoid APIENTRY __GLVVT_FUNC(Color4iv)(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE->Color4iv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Color4s)(GLshort red, GLshort green, GLshort blue, GLshort alpha)
{
    __GL_GET_DISPATCH_TABLE->Color4s(red, green, blue, alpha);
}

GLvoid APIENTRY __GLVVT_FUNC(Color4sv)(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE->Color4sv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Color4ub)(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
    __GL_GET_DISPATCH_TABLE->Color4ub(red, green, blue, alpha);
}

GLvoid APIENTRY __GLVVT_FUNC(Color4ubv)(const GLubyte *v)
{
    __GL_GET_DISPATCH_TABLE->Color4ubv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Color4ui)(GLuint red, GLuint green, GLuint blue, GLuint alpha)
{
    __GL_GET_DISPATCH_TABLE->Color4ui(red, green, blue, alpha);
}

GLvoid APIENTRY __GLVVT_FUNC(Color4uiv)(const GLuint *v)
{
    __GL_GET_DISPATCH_TABLE->Color4uiv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Color4us)(GLushort red, GLushort green, GLushort blue, GLushort alpha)
{
    __GL_GET_DISPATCH_TABLE->Color4us(red, green, blue, alpha);
}

GLvoid APIENTRY __GLVVT_FUNC(Color4usv)(const GLushort *v)
{
    __GL_GET_DISPATCH_TABLE->Color4usv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(EdgeFlag)(GLboolean flag)
{
    __GL_GET_DISPATCH_TABLE->EdgeFlag(flag);
}

GLvoid APIENTRY __GLVVT_FUNC(EdgeFlagv)(const GLboolean *flag)
{
    __GL_GET_DISPATCH_TABLE->EdgeFlagv(flag);
}

GLvoid APIENTRY __GLVVT_FUNC(End)(GLvoid)
{
    __GL_GET_DISPATCH_TABLE->End();
}

GLvoid APIENTRY __GLVVT_FUNC(Indexd)(GLdouble c)
{
    __GL_GET_DISPATCH_TABLE->Indexd(c);
}

GLvoid APIENTRY __GLVVT_FUNC(Indexdv)(const GLdouble *c)
{
    __GL_GET_DISPATCH_TABLE->Indexdv(c);
}

GLvoid APIENTRY __GLVVT_FUNC(Indexf)(GLfloat c)
{
    __GL_GET_DISPATCH_TABLE->Indexf(c);
}

GLvoid APIENTRY __GLVVT_FUNC(Indexfv)(const GLfloat *c)
{
    __GL_GET_DISPATCH_TABLE->Indexfv(c);
}

GLvoid APIENTRY __GLVVT_FUNC(Indexi)(GLint c)
{
    __GL_GET_DISPATCH_TABLE->Indexi(c);
}

GLvoid APIENTRY __GLVVT_FUNC(Indexiv)(const GLint *c)
{
    __GL_GET_DISPATCH_TABLE->Indexiv(c);
}

GLvoid APIENTRY __GLVVT_FUNC(Indexs)(GLshort c)
{
    __GL_GET_DISPATCH_TABLE->Indexs(c);
}

GLvoid APIENTRY __GLVVT_FUNC(Indexsv)(const GLshort *c)
{
    __GL_GET_DISPATCH_TABLE->Indexsv(c);
}

GLvoid APIENTRY __GLVVT_FUNC(Normal3b)(GLbyte nx, GLbyte ny, GLbyte nz)
{
    __GL_GET_DISPATCH_TABLE->Normal3b(nx, ny, nz);
}

GLvoid APIENTRY __GLVVT_FUNC(Normal3bv)(const GLbyte *v)
{
    __GL_GET_DISPATCH_TABLE->Normal3bv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Normal3d)(GLdouble nx, GLdouble ny, GLdouble nz)
{
    __GL_GET_DISPATCH_TABLE->Normal3d(nx, ny, nz);
}

GLvoid APIENTRY __GLVVT_FUNC(Normal3dv)(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE->Normal3dv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Normal3f)(GLfloat nx, GLfloat ny, GLfloat nz)
{
    __GL_GET_DISPATCH_TABLE->Normal3f(nx, ny, nz);
}

GLvoid APIENTRY __GLVVT_FUNC(Normal3fv)(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE->Normal3fv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Normal3i)(GLint nx, GLint ny, GLint nz)
{
    __GL_GET_DISPATCH_TABLE->Normal3i(nx, ny, nz);
}

GLvoid APIENTRY __GLVVT_FUNC(Normal3iv)(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE->Normal3iv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Normal3s)(GLshort nx, GLshort ny, GLshort nz)
{
    __GL_GET_DISPATCH_TABLE->Normal3s(nx,ny,nz);
}

GLvoid APIENTRY __GLVVT_FUNC(Normal3sv)(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE->Normal3sv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(RasterPos2d)(GLdouble x, GLdouble y)
{
    __GL_GET_DISPATCH_TABLE->RasterPos2d(x,y);
}

GLvoid APIENTRY __GLVVT_FUNC(RasterPos2dv)(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE->RasterPos2dv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(RasterPos2f)(GLfloat x, GLfloat y)
{
    __GL_GET_DISPATCH_TABLE->RasterPos2f(x,y);
}

GLvoid APIENTRY __GLVVT_FUNC(RasterPos2fv)(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE->RasterPos2fv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(RasterPos2i)(GLint x, GLint y)
{
    __GL_GET_DISPATCH_TABLE->RasterPos2i(x,y);
}

GLvoid APIENTRY __GLVVT_FUNC(RasterPos2iv)(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE->RasterPos2iv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(RasterPos2s)(GLshort x, GLshort y)
{
    __GL_GET_DISPATCH_TABLE->RasterPos2s(x,y);
}

GLvoid APIENTRY __GLVVT_FUNC(RasterPos2sv)(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE->RasterPos2sv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(RasterPos3d)(GLdouble x, GLdouble y, GLdouble z)
{
    __GL_GET_DISPATCH_TABLE->RasterPos3d(x,y,z);
}

GLvoid APIENTRY __GLVVT_FUNC(RasterPos3dv)(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE->RasterPos3dv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(RasterPos3f)(GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_DISPATCH_TABLE->RasterPos3f(x,y,z);
}

GLvoid APIENTRY __GLVVT_FUNC(RasterPos3fv)(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE->RasterPos3fv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(RasterPos3i)(GLint x, GLint y, GLint z)
{
    __GL_GET_DISPATCH_TABLE->RasterPos3i(x,y,z);
}

GLvoid APIENTRY __GLVVT_FUNC(RasterPos3iv)(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE->RasterPos3iv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(RasterPos3s)(GLshort x, GLshort y, GLshort z)
{
    __GL_GET_DISPATCH_TABLE->RasterPos3s(x,y,z);
}

GLvoid APIENTRY __GLVVT_FUNC(RasterPos3sv)(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE->RasterPos3sv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(RasterPos4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GL_GET_DISPATCH_TABLE->RasterPos4d(x,y,z,w);
}

GLvoid APIENTRY __GLVVT_FUNC(RasterPos4dv)(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE->RasterPos4dv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(RasterPos4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GL_GET_DISPATCH_TABLE->RasterPos4f(x,y,z,w);
}

GLvoid APIENTRY __GLVVT_FUNC(RasterPos4fv)(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE->RasterPos4fv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(RasterPos4i)(GLint x, GLint y, GLint z, GLint w)
{
    __GL_GET_DISPATCH_TABLE->RasterPos4i(x,y,z,w);
}

GLvoid APIENTRY __GLVVT_FUNC(RasterPos4iv)(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE->RasterPos4iv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(RasterPos4s)(GLshort x, GLshort y, GLshort z, GLshort w)
{
    __GL_GET_DISPATCH_TABLE->RasterPos4s(x,y,z,w);
}

GLvoid APIENTRY __GLVVT_FUNC(RasterPos4sv)(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE->RasterPos4sv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Rectd)(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
    __GL_GET_DISPATCH_TABLE->Rectd(x1,y1,x2,y2);
}

GLvoid APIENTRY __GLVVT_FUNC(Rectdv)(const GLdouble *v1, const GLdouble *v2)
{
    __GL_GET_DISPATCH_TABLE->Rectdv(v1,v2);
}

GLvoid APIENTRY __GLVVT_FUNC(Rectf)(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
    __GL_GET_DISPATCH_TABLE->Rectf(x1,y1,x2,y2);
}

GLvoid APIENTRY __GLVVT_FUNC(Rectfv)(const GLfloat *v1, const GLfloat *v2)
{
    __GL_GET_DISPATCH_TABLE->Rectfv(v1,v2);
}

GLvoid APIENTRY __GLVVT_FUNC(Recti)(GLint x1, GLint y1, GLint x2, GLint y2)
{
    __GL_GET_DISPATCH_TABLE->Recti(x1,y1,x2,y2);
}

GLvoid APIENTRY __GLVVT_FUNC(Rectiv)(const GLint *v1, const GLint *v2)
{
    __GL_GET_DISPATCH_TABLE->Rectiv(v1,v2);
}

GLvoid APIENTRY __GLVVT_FUNC(Rects)(GLshort x1, GLshort y1, GLshort x2, GLshort y2)
{
    __GL_GET_DISPATCH_TABLE->Rects(x1,y1,x2,y2);
}

GLvoid APIENTRY __GLVVT_FUNC(Rectsv)(const GLshort *v1, const GLshort *v2)
{
    __GL_GET_DISPATCH_TABLE->Rectsv(v1,v2);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord1d)(GLdouble s)
{
    __GL_GET_DISPATCH_TABLE->TexCoord1d(s);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord1dv)(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE->TexCoord1dv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord1f)(GLfloat s)
{
    __GL_GET_DISPATCH_TABLE->TexCoord1f(s);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord1fv)(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE->TexCoord1fv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord1i)(GLint s)
{
    __GL_GET_DISPATCH_TABLE->TexCoord1i(s);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord1iv)(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE->TexCoord1iv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord1s)(GLshort s)
{
    __GL_GET_DISPATCH_TABLE->TexCoord1s(s);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord1sv)(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE->TexCoord1sv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord2d)(GLdouble s, GLdouble t)
{
    __GL_GET_DISPATCH_TABLE->TexCoord2d(s,t);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord2dv)(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE->TexCoord2dv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord2f)(GLfloat s, GLfloat t)
{
    __GL_GET_DISPATCH_TABLE->TexCoord2f(s,t);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord2fv)(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE->TexCoord2fv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord2i)(GLint s, GLint t)
{
    __GL_GET_DISPATCH_TABLE->TexCoord2i(s,t);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord2iv)(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE->TexCoord2iv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord2s)(GLshort s, GLshort t)
{
    __GL_GET_DISPATCH_TABLE->TexCoord2s(s,t);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord2sv)(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE->TexCoord2sv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord3d)(GLdouble s, GLdouble t, GLdouble r)
{
    __GL_GET_DISPATCH_TABLE->TexCoord3d(s,t,r);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord3dv)(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE->TexCoord3dv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord3f)(GLfloat s, GLfloat t, GLfloat r)
{
    __GL_GET_DISPATCH_TABLE->TexCoord3f(s,t,r);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord3fv)(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE->TexCoord3fv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord3i)(GLint s, GLint t, GLint r)
{
    __GL_GET_DISPATCH_TABLE->TexCoord3i(s,t,r);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord3iv)(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE->TexCoord3iv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord3s)(GLshort s, GLshort t, GLshort r)
{
    __GL_GET_DISPATCH_TABLE->TexCoord3s(s,t,r);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord3sv)(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE->TexCoord3sv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord4d)(GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    __GL_GET_DISPATCH_TABLE->TexCoord4d(s,t,r,q);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord4dv)(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE->TexCoord4dv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord4f)(GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    __GL_GET_DISPATCH_TABLE->TexCoord4f(s,t,r,q);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord4fv)(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE->TexCoord4fv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord4i)(GLint s, GLint t, GLint r, GLint q)
{
    __GL_GET_DISPATCH_TABLE->TexCoord4i(s,t,r,q);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord4iv)(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE->TexCoord4iv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord4s)(GLshort s, GLshort t, GLshort r, GLshort q)
{
    __GL_GET_DISPATCH_TABLE->TexCoord4s(s,t,r,q);
}

GLvoid APIENTRY __GLVVT_FUNC(TexCoord4sv)(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE->TexCoord4sv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Vertex2d)(GLdouble x, GLdouble y)
{
    __GL_GET_DISPATCH_TABLE->Vertex2d(x,y);
}

GLvoid APIENTRY __GLVVT_FUNC(Vertex2dv)(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE->Vertex2dv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Vertex2f)(GLfloat x, GLfloat y)
{
    __GL_GET_DISPATCH_TABLE->Vertex2f(x,y);
}

GLvoid APIENTRY __GLVVT_FUNC(Vertex2fv)(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE->Vertex2fv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Vertex2i)(GLint x, GLint y)
{
    __GL_GET_DISPATCH_TABLE->Vertex2i(x,y);
}

GLvoid APIENTRY __GLVVT_FUNC(Vertex2iv)(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE->Vertex2iv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Vertex2s)(GLshort x, GLshort y)
{
    __GL_GET_DISPATCH_TABLE->Vertex2s(x,y);
}

GLvoid APIENTRY __GLVVT_FUNC(Vertex2sv)(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE->Vertex2sv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Vertex3d)(GLdouble x, GLdouble y, GLdouble z)
{
    __GL_GET_DISPATCH_TABLE->Vertex3d(x,y,z);
}

GLvoid APIENTRY __GLVVT_FUNC(Vertex3dv)(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE->Vertex3dv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Vertex3f)(GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_DISPATCH_TABLE->Vertex3f(x,y,z);
}

GLvoid APIENTRY __GLVVT_FUNC(Vertex3fv)(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE->Vertex3fv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Vertex3i)(GLint x, GLint y, GLint z)
{
    __GL_GET_DISPATCH_TABLE->Vertex3i(x,y,z);
}

GLvoid APIENTRY __GLVVT_FUNC(Vertex3iv)(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE->Vertex3iv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Vertex3s)(GLshort x, GLshort y, GLshort z)
{
    __GL_GET_DISPATCH_TABLE->Vertex3s(x,y,z);
}

GLvoid APIENTRY __GLVVT_FUNC(Vertex3sv)(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE->Vertex3sv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Vertex4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GL_GET_DISPATCH_TABLE->Vertex4d(x,y,z,w);
}

GLvoid APIENTRY __GLVVT_FUNC(Vertex4dv)(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE->Vertex4dv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Vertex4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GL_GET_DISPATCH_TABLE->Vertex4f(x,y,z,w);
}

GLvoid APIENTRY __GLVVT_FUNC(Vertex4fv)(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE->Vertex4fv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Vertex4i)(GLint x, GLint y, GLint z, GLint w)
{
    __GL_GET_DISPATCH_TABLE->Vertex4i(x,y,z,w);
}

GLvoid APIENTRY __GLVVT_FUNC(Vertex4iv)(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE->Vertex4iv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(Vertex4s)(GLshort x, GLshort y, GLshort z, GLshort w)
{
    __GL_GET_DISPATCH_TABLE->Vertex4s(x,y,z,w);
}

GLvoid APIENTRY __GLVVT_FUNC(Vertex4sv)(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE->Vertex4sv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(ClipPlane)(GLenum plane, const GLdouble *equation)
{
    __GL_GET_DISPATCH_TABLE->ClipPlane(plane,equation);
}

GLvoid APIENTRY __GLVVT_FUNC(ColorMaterial)(GLenum face, GLenum mode)
{
    __GL_GET_DISPATCH_TABLE->ColorMaterial(face,mode);
}

GLvoid APIENTRY __GLVVT_FUNC(CullFace)(GLenum mode)
{
    __GL_GET_DISPATCH_TABLE->CullFace(mode);
}

GLvoid APIENTRY __GLVVT_FUNC(Fogf)(GLenum pname, GLfloat param)
{
    __GL_GET_DISPATCH_TABLE->Fogf(pname,param);
}

GLvoid APIENTRY __GLVVT_FUNC(Fogfv)(GLenum pname, const GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->Fogfv(pname,params);
}

GLvoid APIENTRY __GLVVT_FUNC(Fogi)(GLenum pname, GLint param)
{
    __GL_GET_DISPATCH_TABLE->Fogi(pname,param);
}

GLvoid APIENTRY __GLVVT_FUNC(Fogiv)(GLenum pname, const GLint *params)
{
    __GL_GET_DISPATCH_TABLE->Fogiv(pname,params);
}

GLvoid APIENTRY __GLVVT_FUNC(FrontFace)(GLenum mode)
{
    __GL_GET_DISPATCH_TABLE->FrontFace(mode);
}

GLvoid APIENTRY __GLVVT_FUNC(Hint)(GLenum target, GLenum mode)
{
    __GL_GET_DISPATCH_TABLE->Hint(target,mode);
}

GLvoid APIENTRY __GLVVT_FUNC(Lightf)(GLenum light, GLenum pname, GLfloat param)
{
    __GL_GET_DISPATCH_TABLE->Lightf(light,pname,param);
}

GLvoid APIENTRY __GLVVT_FUNC(Lightfv)(GLenum light, GLenum pname, const GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->Lightfv(light,pname,params);
}

GLvoid APIENTRY __GLVVT_FUNC(Lighti)(GLenum light, GLenum pname, GLint param)
{
    __GL_GET_DISPATCH_TABLE->Lighti(light,pname,param);
}

GLvoid APIENTRY __GLVVT_FUNC(Lightiv)(GLenum light, GLenum pname, const GLint *params)
{
    __GL_GET_DISPATCH_TABLE->Lightiv(light,pname,params);
}

GLvoid APIENTRY __GLVVT_FUNC(LightModelf)(GLenum pname, GLfloat param)
{
    __GL_GET_DISPATCH_TABLE->LightModelf(pname,param);
}

GLvoid APIENTRY __GLVVT_FUNC(LightModelfv)(GLenum pname, const GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->LightModelfv(pname,params);
}

GLvoid APIENTRY __GLVVT_FUNC(LightModeli)(GLenum pname, GLint param)
{
    __GL_GET_DISPATCH_TABLE->LightModeli(pname,param);
}

GLvoid APIENTRY __GLVVT_FUNC(LightModeliv)(GLenum pname, const GLint *params)
{
    __GL_GET_DISPATCH_TABLE->LightModeliv(pname,params);
}

GLvoid APIENTRY __GLVVT_FUNC(LineStipple)(GLint factor, GLushort pattern)
{
    __GL_GET_DISPATCH_TABLE->LineStipple(factor,pattern);
}

GLvoid APIENTRY __GLVVT_FUNC(LineWidth)(GLfloat width)
{
    __GL_GET_DISPATCH_TABLE->LineWidth(width);
}

GLvoid APIENTRY __GLVVT_FUNC(Materialf)(GLenum face, GLenum pname, GLfloat param)
{
    __GL_GET_DISPATCH_TABLE->Materialf(face,pname,param);
}

GLvoid APIENTRY __GLVVT_FUNC(Materialfv)(GLenum face, GLenum pname, const GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->Materialfv(face,pname,params);
}

GLvoid APIENTRY __GLVVT_FUNC(Materiali)(GLenum face, GLenum pname, GLint param)
{
    __GL_GET_DISPATCH_TABLE->Materiali(face,pname,param);
}

GLvoid APIENTRY __GLVVT_FUNC(Materialiv)(GLenum face, GLenum pname, const GLint *params)
{
    __GL_GET_DISPATCH_TABLE->Materialiv(face,pname,params);
}

GLvoid APIENTRY __GLVVT_FUNC(PointSize)(GLfloat size)
{
    __GL_GET_DISPATCH_TABLE->PointSize(size);
}

GLvoid APIENTRY __GLVVT_FUNC(PolygonMode)(GLenum face, GLenum mode)
{
    __GL_GET_DISPATCH_TABLE->PolygonMode(face,mode);
}

GLvoid APIENTRY __GLVVT_FUNC(PolygonStipple)(const GLubyte *mask)
{
    __GL_GET_DISPATCH_TABLE->PolygonStipple(mask);
}

GLvoid APIENTRY __GLVVT_FUNC(Scissor)(GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_GET_DISPATCH_TABLE->Scissor(x,y, width,height);
}

GLvoid APIENTRY __GLVVT_FUNC(ShadeModel)(GLenum mode)
{
    __GL_GET_DISPATCH_TABLE->ShadeModel(mode);
}

GLvoid APIENTRY __GLVVT_FUNC(TexParameterf)(GLenum target, GLenum pname, GLfloat param)
{
    __GL_GET_DISPATCH_TABLE->TexParameterf(target, pname, param);
}

GLvoid APIENTRY __GLVVT_FUNC(TexParameterfv)(GLenum target, GLenum pname, const GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->TexParameterfv(target,pname,params);
}

GLvoid APIENTRY __GLVVT_FUNC(TexParameteri)(GLenum target, GLenum pname, GLint param)
{
    __GL_GET_DISPATCH_TABLE->TexParameteri(target,pname,param);
}

GLvoid APIENTRY __GLVVT_FUNC(TexParameteriv)(GLenum target, GLenum pname, const GLint *params)
{
    __GL_GET_DISPATCH_TABLE->TexParameteriv(target,pname,params);
}

GLvoid APIENTRY __GLVVT_FUNC(TexImage1D)(GLenum target, GLint level, GLint components,
    GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    __GL_GET_DISPATCH_TABLE->TexImage1D(target,level,components,width,border,format,type,pixels);
}

GLvoid APIENTRY __GLVVT_FUNC(TexImage2D)(GLenum target, GLint level, GLint components,
    GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    __GL_GET_DISPATCH_TABLE->TexImage2D(target,level,components,width,height,border,format,type, pixels);
}

GLvoid APIENTRY __GLVVT_FUNC(TexEnvf)(GLenum target, GLenum pname, GLfloat param)
{
    __GL_GET_DISPATCH_TABLE->TexEnvf(target,pname,param);
}

GLvoid APIENTRY __GLVVT_FUNC(TexEnvfv)(GLenum target, GLenum pname, const GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->TexEnvfv(target,pname,params);
}

GLvoid APIENTRY __GLVVT_FUNC(TexEnvi)(GLenum target, GLenum pname, GLint param)
{
    __GL_GET_DISPATCH_TABLE->TexEnvi(target,pname,param);
}

GLvoid APIENTRY __GLVVT_FUNC(TexEnviv)(GLenum target, GLenum pname, const GLint *params)
{
    __GL_GET_DISPATCH_TABLE->TexEnviv(target,pname,params);
}

GLvoid APIENTRY __GLVVT_FUNC(TexGend)(GLenum coord, GLenum pname, GLdouble param)
{
    __GL_GET_DISPATCH_TABLE->TexGend(coord,pname,param);
}

GLvoid APIENTRY __GLVVT_FUNC(TexGendv)(GLenum coord, GLenum pname, const GLdouble *params)
{
    __GL_GET_DISPATCH_TABLE->TexGendv(coord,pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(TexGenf)(GLenum coord, GLenum pname, GLfloat param)
{
    __GL_GET_DISPATCH_TABLE->TexGenf(coord,pname, param);
}

GLvoid APIENTRY __GLVVT_FUNC(TexGenfv)(GLenum coord, GLenum pname, const GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->TexGenfv(coord, pname,params);
}

GLvoid APIENTRY __GLVVT_FUNC(TexGeni)(GLenum coord, GLenum pname, GLint param)
{
    __GL_GET_DISPATCH_TABLE->TexGeni(coord,pname,param);
}

GLvoid APIENTRY __GLVVT_FUNC(TexGeniv)(GLenum coord, GLenum pname, const GLint *params)
{
    __GL_GET_DISPATCH_TABLE->TexGeniv(coord,pname,params);
}

GLvoid APIENTRY __GLVVT_FUNC(FeedbackBuffer)(GLsizei size, GLenum type, GLfloat *buffer)
{
    __GL_GET_DISPATCH_TABLE->FeedbackBuffer(size,type,buffer);
}

GLvoid APIENTRY __GLVVT_FUNC(SelectBuffer)(GLsizei size, GLuint *buffer)
{
    __GL_GET_DISPATCH_TABLE->SelectBuffer(size, buffer);
}

GLint APIENTRY __GLVVT_FUNC(RenderMode)(GLenum mode)
{
    return __GL_GET_DISPATCH_TABLE->RenderMode(mode);
}

GLvoid APIENTRY __GLVVT_FUNC(InitNames)(GLvoid)
{
    __GL_GET_DISPATCH_TABLE->InitNames();
}

GLvoid APIENTRY __GLVVT_FUNC(LoadName)(GLuint name)
{
    __GL_GET_DISPATCH_TABLE->LoadName(name);
}

GLvoid APIENTRY __GLVVT_FUNC(PassThrough)(GLfloat token)
{
    __GL_GET_DISPATCH_TABLE->PassThrough(token);
}

GLvoid APIENTRY __GLVVT_FUNC(PopName)(GLvoid)
{
    __GL_GET_DISPATCH_TABLE->PopName();
}

GLvoid APIENTRY __GLVVT_FUNC(PushName)(GLuint name)
{
    __GL_GET_DISPATCH_TABLE->PushName(name);
}

GLvoid APIENTRY __GLVVT_FUNC(DrawBuffer)(GLenum mode)
{
    __GL_GET_DISPATCH_TABLE->DrawBuffer(mode);
}

GLvoid APIENTRY __GLVVT_FUNC(Clear)(GLbitfield mask)
{
    __GL_GET_DISPATCH_TABLE->Clear(mask);
}

GLvoid APIENTRY __GLVVT_FUNC(ClearAccum)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    __GL_GET_DISPATCH_TABLE->ClearAccum(red,green,blue, alpha);
}

GLvoid APIENTRY __GLVVT_FUNC(ClearIndex)(GLfloat c)
{
    __GL_GET_DISPATCH_TABLE->ClearIndex(c);
}

GLvoid APIENTRY __GLVVT_FUNC(ClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    __GL_GET_DISPATCH_TABLE->ClearColor(red,green,blue,alpha);
}

GLvoid APIENTRY __GLVVT_FUNC(ClearStencil)(GLint s)
{
    __GL_GET_DISPATCH_TABLE->ClearStencil(s);
}

GLvoid APIENTRY __GLVVT_FUNC(ClearDepth)(GLclampd depth)
{
    __GL_GET_DISPATCH_TABLE->ClearDepth(depth);
}

GLvoid APIENTRY __GLVVT_FUNC(StencilMask)(GLuint mask)
{
    __GL_GET_DISPATCH_TABLE->StencilMask(mask);
}

GLvoid APIENTRY __GLVVT_FUNC(ColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    __GL_GET_DISPATCH_TABLE->ColorMask(red, green,blue,alpha);
}

GLvoid APIENTRY __GLVVT_FUNC(DepthMask)(GLboolean flag)
{
    __GL_GET_DISPATCH_TABLE->DepthMask(flag);
}

GLvoid APIENTRY __GLVVT_FUNC(IndexMask)(GLuint mask)
{
    __GL_GET_DISPATCH_TABLE->IndexMask(mask);
}

GLvoid APIENTRY __GLVVT_FUNC(Accum)(GLenum op, GLfloat value)
{
    __GL_GET_DISPATCH_TABLE->Accum(op,value);
}

GLvoid APIENTRY __GLVVT_FUNC(Disable)(GLenum cap)
{
    __GL_GET_DISPATCH_TABLE->Disable(cap);
}

GLvoid APIENTRY __GLVVT_FUNC(Enable)(GLenum cap)
{
    __GL_GET_DISPATCH_TABLE->Enable(cap);
}

GLvoid APIENTRY __GLVVT_FUNC(Finish)(GLvoid)
{
    __GL_GET_DISPATCH_TABLE->Finish();
}

GLvoid APIENTRY __GLVVT_FUNC(Flush)(GLvoid)
{
    __GL_GET_DISPATCH_TABLE->Flush();
}

GLvoid APIENTRY __GLVVT_FUNC(PopAttrib)(GLvoid)
{
    __GL_GET_DISPATCH_TABLE->PopAttrib();
}

GLvoid APIENTRY __GLVVT_FUNC(PushAttrib)(GLbitfield mask)
{
    __GL_GET_DISPATCH_TABLE->PushAttrib(mask);
}

GLvoid APIENTRY __GLVVT_FUNC(Map1d)(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points)
{
    __GL_GET_DISPATCH_TABLE->Map1d(target,u1,u2, stride,order, points);
}

GLvoid APIENTRY __GLVVT_FUNC(Map1f)(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points)
{
    __GL_GET_DISPATCH_TABLE->Map1f(target,u1,u2,stride,order,points);
}

GLvoid APIENTRY __GLVVT_FUNC(Map2d)(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder,
                           GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points)
{
    __GL_GET_DISPATCH_TABLE->Map2d(target, u1, u2,ustride, uorder, v1, v2, vstride,vorder, points);
}

GLvoid APIENTRY __GLVVT_FUNC(Map2f)(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder,
                           GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points)
{
    __GL_GET_DISPATCH_TABLE->Map2f(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder,points);
}

GLvoid APIENTRY __GLVVT_FUNC(MapGrid1d)(GLint un, GLdouble u1, GLdouble u2)
{
    __GL_GET_DISPATCH_TABLE->MapGrid1d( un, u1, u2);
}

GLvoid APIENTRY __GLVVT_FUNC(MapGrid1f)(GLint un, GLfloat u1, GLfloat u2)
{
    __GL_GET_DISPATCH_TABLE->MapGrid1f(un, u1, u2);
}

GLvoid APIENTRY __GLVVT_FUNC(MapGrid2d)(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2)
{
    __GL_GET_DISPATCH_TABLE->MapGrid2d(un, u1, u2, vn, v1, v2);
}

GLvoid APIENTRY __GLVVT_FUNC(MapGrid2f)(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2)
{
    __GL_GET_DISPATCH_TABLE->MapGrid2f(un, u1, u2, vn, v1, v2);
}

GLvoid APIENTRY __GLVVT_FUNC(EvalCoord1d)(GLdouble u)
{
    __GL_GET_DISPATCH_TABLE->EvalCoord1d(u);
}

GLvoid APIENTRY __GLVVT_FUNC(EvalCoord1dv)(const GLdouble *u)
{
    __GL_GET_DISPATCH_TABLE->EvalCoord1dv(u);
}

GLvoid APIENTRY __GLVVT_FUNC(EvalCoord1f)(GLfloat u)
{
    __GL_GET_DISPATCH_TABLE->EvalCoord1f(u);
}

GLvoid APIENTRY __GLVVT_FUNC(EvalCoord1fv)(const GLfloat *u)
{
    __GL_GET_DISPATCH_TABLE->EvalCoord1fv(u);
}

GLvoid APIENTRY __GLVVT_FUNC(EvalCoord2d)(GLdouble u, GLdouble v)
{
    __GL_GET_DISPATCH_TABLE->EvalCoord2d(u, v);
}

GLvoid APIENTRY __GLVVT_FUNC(EvalCoord2dv)(const GLdouble *u)
{
    __GL_GET_DISPATCH_TABLE->EvalCoord2dv(u);
}

GLvoid APIENTRY __GLVVT_FUNC(EvalCoord2f)(GLfloat u, GLfloat v)
{
    __GL_GET_DISPATCH_TABLE->EvalCoord2f(u, v);
}

GLvoid APIENTRY __GLVVT_FUNC(EvalCoord2fv)(const GLfloat *u)
{
    __GL_GET_DISPATCH_TABLE->EvalCoord2fv(u);
}

GLvoid APIENTRY __GLVVT_FUNC(EvalMesh1)(GLenum mode, GLint i1, GLint i2)
{
    __GL_GET_DISPATCH_TABLE->EvalMesh1(mode, i1, i2);
}

GLvoid APIENTRY __GLVVT_FUNC(EvalPoint1)(GLint i)
{
    __GL_GET_DISPATCH_TABLE->EvalPoint1(i);
}

GLvoid APIENTRY __GLVVT_FUNC(EvalMesh2)(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2)
{
    __GL_GET_DISPATCH_TABLE->EvalMesh2(mode, i1, i2, j1, j2);
}

GLvoid APIENTRY __GLVVT_FUNC(EvalPoint2)(GLint i, GLint j)
{
    __GL_GET_DISPATCH_TABLE->EvalPoint2(i, j);
}

GLvoid APIENTRY __GLVVT_FUNC(AlphaFunc)(GLenum func, GLclampf ref)
{
    __GL_GET_DISPATCH_TABLE->AlphaFunc(func, ref);
}

GLvoid APIENTRY __GLVVT_FUNC(BlendFunc)(GLenum sfactor, GLenum dfactor)
{
    __GL_GET_DISPATCH_TABLE->BlendFunc(sfactor, dfactor);
}

GLvoid APIENTRY __GLVVT_FUNC(LogicOp)(GLenum opcode)
{
    __GL_GET_DISPATCH_TABLE->LogicOp(opcode);
}

GLvoid APIENTRY __GLVVT_FUNC(StencilFunc)(GLenum func, GLint ref, GLuint mask)
{
    __GL_GET_DISPATCH_TABLE->StencilFunc(func, ref, mask);
}

GLvoid APIENTRY __GLVVT_FUNC(StencilOp)(GLenum fail, GLenum zfail, GLenum zpass)
{
    __GL_GET_DISPATCH_TABLE->StencilOp(fail, zfail, zpass);
}

GLvoid APIENTRY __GLVVT_FUNC(DepthFunc)(GLenum func)
{
    __GL_GET_DISPATCH_TABLE->DepthFunc(func);
}

GLvoid APIENTRY __GLVVT_FUNC(PixelZoom)(GLfloat xfactor, GLfloat yfactor)
{
    __GL_GET_DISPATCH_TABLE->PixelZoom(xfactor,yfactor);
}

GLvoid APIENTRY __GLVVT_FUNC(PixelTransferf)(GLenum pname, GLfloat param)
{
    __GL_GET_DISPATCH_TABLE->PixelTransferf(pname, param);
}

GLvoid APIENTRY __GLVVT_FUNC(PixelTransferi)(GLenum pname, GLint param)
{
    __GL_GET_DISPATCH_TABLE->PixelTransferi(pname, param);
}

GLvoid APIENTRY __GLVVT_FUNC(PixelStoref)(GLenum pname, GLfloat param)
{
    __GL_GET_DISPATCH_TABLE->PixelStoref(pname,param);
}

GLvoid APIENTRY __GLVVT_FUNC(PixelStorei)(GLenum pname, GLint param)
{
    __GL_GET_DISPATCH_TABLE->PixelStorei(pname, param);
}

GLvoid APIENTRY __GLVVT_FUNC(PixelMapfv)(GLenum map, GLint mapsize, const GLfloat *values)
{
    __GL_GET_DISPATCH_TABLE->PixelMapfv(map,mapsize, values);
}

GLvoid APIENTRY __GLVVT_FUNC(PixelMapuiv)(GLenum map, GLint mapsize, const GLuint *values)
{
    __GL_GET_DISPATCH_TABLE->PixelMapuiv(map, mapsize, values);
}

GLvoid APIENTRY __GLVVT_FUNC(PixelMapusv)(GLenum map, GLint mapsize, const GLushort *values)
{
    __GL_GET_DISPATCH_TABLE->PixelMapusv(map, mapsize, values);
}

GLvoid APIENTRY __GLVVT_FUNC(ReadBuffer)(GLenum mode)
{
    __GL_GET_DISPATCH_TABLE->ReadBuffer(mode);
}

GLvoid APIENTRY __GLVVT_FUNC(CopyPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)
{
    __GL_GET_DISPATCH_TABLE->CopyPixels(x, y, width, height, type);
}

GLvoid APIENTRY __GLVVT_FUNC(ReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)
{
    __GL_GET_DISPATCH_TABLE->ReadPixels(x, y, width, height, format, type, pixels);
}

GLvoid APIENTRY __GLVVT_FUNC(DrawPixels)(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
    __GL_GET_DISPATCH_TABLE->DrawPixels(width,height, format, type, pixels);
}

GLvoid APIENTRY __GLVVT_FUNC(GetBooleanv)(GLenum pname, GLboolean *params)
{
    __GL_GET_DISPATCH_TABLE->GetBooleanv(pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetClipPlane)(GLenum plane, GLdouble *equation)
{
    __GL_GET_DISPATCH_TABLE->GetClipPlane(plane, equation);
}

GLvoid APIENTRY __GLVVT_FUNC(GetDoublev)(GLenum pname, GLdouble *params)
{
    __GL_GET_DISPATCH_TABLE->GetDoublev(pname, params);
}

GLenum APIENTRY __GLVVT_FUNC(GetError)(GLvoid)
{
   return __GL_GET_DISPATCH_TABLE->GetError();
}

GLvoid APIENTRY __GLVVT_FUNC(GetFloatv)(GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->GetFloatv(pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetIntegerv)(GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE->GetIntegerv(pname,params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetLightfv)(GLenum light, GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->GetLightfv(light, pname,params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetLightiv)(GLenum light, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE->GetLightiv(light,pname,params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetMapdv)(GLenum target, GLenum query, GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE->GetMapdv(target,query, v);
}

GLvoid APIENTRY __GLVVT_FUNC(GetMapfv)(GLenum target, GLenum query, GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE->GetMapfv(target, query, v);
}

GLvoid APIENTRY __GLVVT_FUNC(GetMapiv)(GLenum target, GLenum query, GLint *v)
{
    __GL_GET_DISPATCH_TABLE->GetMapiv(target, query, v);
}

GLvoid APIENTRY __GLVVT_FUNC(GetMaterialfv)(GLenum face, GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->GetMaterialfv(face, pname,params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetMaterialiv)(GLenum face, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE->GetMaterialiv(face, pname,params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetPixelMapfv)(GLenum map, GLfloat *values)
{
    __GL_GET_DISPATCH_TABLE->GetPixelMapfv(map, values);
}

GLvoid APIENTRY __GLVVT_FUNC(GetPixelMapuiv)(GLenum map, GLuint *values)
{
    __GL_GET_DISPATCH_TABLE->GetPixelMapuiv(map, values);
}

GLvoid APIENTRY __GLVVT_FUNC(GetPixelMapusv)(GLenum map, GLushort *values)
{
    __GL_GET_DISPATCH_TABLE->GetPixelMapusv(map, values);
}

GLvoid APIENTRY __GLVVT_FUNC(GetPolygonStipple)(GLubyte *mask)
{
    __GL_GET_DISPATCH_TABLE->GetPolygonStipple(mask);
}

const GLubyte * APIENTRY __GLVVT_FUNC(GetString)(GLenum name)
{
   return __GL_GET_DISPATCH_TABLE->GetString(name);
}

GLvoid APIENTRY __GLVVT_FUNC(GetTexEnvfv)(GLenum target, GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->GetTexEnvfv(target, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetTexEnviv)(GLenum target, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE->GetTexEnviv(target,pname,params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetTexGendv)(GLenum coord, GLenum pname, GLdouble *params)
{
    __GL_GET_DISPATCH_TABLE->GetTexGendv(coord, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetTexGenfv)(GLenum coord, GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->GetTexGenfv(coord, pname,params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetTexGeniv)(GLenum coord, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE->GetTexGeniv(coord, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetTexImage)(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels)
{
    __GL_GET_DISPATCH_TABLE->GetTexImage(target,level,format,type,pixels);
}

GLvoid APIENTRY __GLVVT_FUNC(GetTexParameterfv)(GLenum target, GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->GetTexParameterfv(target,pname,params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetTexParameteriv)(GLenum target, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE->GetTexParameteriv(target,pname,params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetTexLevelParameterfv)(GLenum target, GLint level, GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->GetTexLevelParameterfv(target,level, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetTexLevelParameteriv)(GLenum target, GLint level, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE->GetTexLevelParameteriv( target, level, pname, params);
}

GLboolean APIENTRY __GLVVT_FUNC(IsEnabled)(GLenum cap)
{
   return __GL_GET_DISPATCH_TABLE->IsEnabled(cap);
}

GLboolean APIENTRY __GLVVT_FUNC(IsList)(GLuint list)
{
   return __GL_GET_DISPATCH_TABLE->IsList(list);
}

GLvoid APIENTRY __GLVVT_FUNC(DepthRange)(GLclampd zNear, GLclampd zFar)
{
    __GL_GET_DISPATCH_TABLE->DepthRange(zNear, zFar);
}

GLvoid APIENTRY __GLVVT_FUNC(Frustum)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
    __GL_GET_DISPATCH_TABLE->Frustum(left, right, bottom, top, zNear, zFar);
}

GLvoid APIENTRY __GLVVT_FUNC(LoadIdentity)(GLvoid)
{
    __GL_GET_DISPATCH_TABLE->LoadIdentity();
}

GLvoid APIENTRY __GLVVT_FUNC(LoadMatrixf)(const GLfloat *m)
{
    __GL_GET_DISPATCH_TABLE->LoadMatrixf(m);
}

GLvoid APIENTRY __GLVVT_FUNC(LoadMatrixd)(const GLdouble *m)
{
    __GL_GET_DISPATCH_TABLE->LoadMatrixd(m);
}

GLvoid APIENTRY __GLVVT_FUNC(MatrixMode)(GLenum mode)
{
    __GL_GET_DISPATCH_TABLE->MatrixMode(mode);
}

GLvoid APIENTRY __GLVVT_FUNC(MultMatrixf)(const GLfloat *m)
{
    __GL_GET_DISPATCH_TABLE->MultMatrixf(m);
}

GLvoid APIENTRY __GLVVT_FUNC(MultMatrixd)(const GLdouble *m)
{
    __GL_GET_DISPATCH_TABLE->MultMatrixd(m);
}

GLvoid APIENTRY __GLVVT_FUNC(Ortho)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
    __GL_GET_DISPATCH_TABLE->Ortho(left, right, bottom, top, zNear, zFar);
}

GLvoid APIENTRY __GLVVT_FUNC(PopMatrix)(GLvoid)
{
    __GL_GET_DISPATCH_TABLE->PopMatrix();
}

GLvoid APIENTRY __GLVVT_FUNC(PushMatrix)(GLvoid)
{
    __GL_GET_DISPATCH_TABLE->PushMatrix();
}

GLvoid APIENTRY __GLVVT_FUNC(Rotated)(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    __GL_GET_DISPATCH_TABLE->Rotated(angle, x, y, z);
}

GLvoid APIENTRY __GLVVT_FUNC(Rotatef)(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_DISPATCH_TABLE->Rotatef(angle, x, y, z);
}

GLvoid APIENTRY __GLVVT_FUNC(Scaled)(GLdouble x, GLdouble y, GLdouble z)
{
    __GL_GET_DISPATCH_TABLE->Scaled(x, y, z);
}

GLvoid APIENTRY __GLVVT_FUNC(Scalef)(GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_DISPATCH_TABLE->Scalef(x, y, z);
}

GLvoid APIENTRY __GLVVT_FUNC(Translated)(GLdouble x, GLdouble y, GLdouble z)
{
    __GL_GET_DISPATCH_TABLE->Translated(x, y, z);
}

GLvoid APIENTRY __GLVVT_FUNC(Translatef)(GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_DISPATCH_TABLE->Translatef(x, y, z);
}

GLvoid APIENTRY __GLVVT_FUNC(Viewport)(GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_GET_DISPATCH_TABLE->Viewport(x, y, width, height);
}

#if GL_VERSION_1_1
GLvoid APIENTRY __GLVVT_FUNC(ArrayElement)(GLint i)
{
    __GL_GET_DISPATCH_TABLE->ArrayElement(i);
}

GLvoid APIENTRY __GLVVT_FUNC(ColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_GET_DISPATCH_TABLE->ColorPointer(size, type, stride,pointer);
}

GLvoid APIENTRY __GLVVT_FUNC(DrawArrays)(GLenum mode, GLint first, GLsizei count)
{
    __GL_GET_DISPATCH_TABLE->DrawArrays(mode, first, count);
}

GLvoid APIENTRY __GLVVT_FUNC(EdgeFlagPointer)(GLsizei stride, const GLvoid *pointer)
{
    __GL_GET_DISPATCH_TABLE->EdgeFlagPointer(stride, pointer);
}

GLvoid APIENTRY __GLVVT_FUNC(GetPointerv)(GLenum pname, GLvoid* *params)
{
    __GL_GET_DISPATCH_TABLE->GetPointerv(pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(IndexPointer)(GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_GET_DISPATCH_TABLE->IndexPointer(type, stride, pointer);
}

GLvoid APIENTRY __GLVVT_FUNC(NormalPointer)(GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_GET_DISPATCH_TABLE->NormalPointer(type, stride, pointer);
}


GLvoid APIENTRY __GLVVT_FUNC(TexCoordPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_GET_DISPATCH_TABLE->TexCoordPointer(size, type, stride, pointer);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_GET_DISPATCH_TABLE->VertexPointer(size, type, stride, pointer);
}

GLvoid APIENTRY __GLVVT_FUNC(DisableClientState)(GLenum array)
{
    __GL_GET_DISPATCH_TABLE->DisableClientState(array);
}

GLvoid APIENTRY __GLVVT_FUNC(DrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    __GL_GET_DISPATCH_TABLE->DrawElements(mode, count, type, indices);
}

GLvoid APIENTRY __GLVVT_FUNC(EnableClientState)(GLenum array)
{
    __GL_GET_DISPATCH_TABLE->EnableClientState(array);
}

GLvoid APIENTRY __GLVVT_FUNC(InterleavedArrays)(GLenum format, GLsizei stride, const GLvoid *pointer)
{
    __GL_GET_DISPATCH_TABLE->InterleavedArrays(format,stride, pointer);
}

GLvoid APIENTRY __GLVVT_FUNC(PolygonOffset)(GLfloat factor, GLfloat units)
{
    __GL_GET_DISPATCH_TABLE->PolygonOffset(factor, units);
}

GLvoid APIENTRY __GLVVT_FUNC(CopyTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
    __GL_GET_DISPATCH_TABLE->CopyTexImage1D(target, level, internalformat, x, y, width, border);
}


GLvoid APIENTRY __GLVVT_FUNC(CopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    __GL_GET_DISPATCH_TABLE->CopyTexImage2D(target, level, internalformat, x, y, width, height, border);
}

GLvoid APIENTRY __GLVVT_FUNC(CopyTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
    __GL_GET_DISPATCH_TABLE->CopyTexSubImage1D(target, level, xoffset, x, y, width);
}


GLvoid APIENTRY __GLVVT_FUNC(CopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_GET_DISPATCH_TABLE->CopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}

GLvoid APIENTRY __GLVVT_FUNC(TexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels)
{
    __GL_GET_DISPATCH_TABLE->TexSubImage1D(target, level, xoffset, width, format, type, pixels);
}


GLvoid APIENTRY __GLVVT_FUNC(TexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset,
                                   GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
    __GL_GET_DISPATCH_TABLE->TexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

GLboolean APIENTRY __GLVVT_FUNC(AreTexturesResident)(GLsizei n, const GLuint *textures, GLboolean *residences)
{
  return __GL_GET_DISPATCH_TABLE->AreTexturesResident(n, textures, residences);
}

GLvoid APIENTRY __GLVVT_FUNC(BindTexture)(GLenum target, GLuint texture)
{
    __GL_GET_DISPATCH_TABLE->BindTexture(target,texture);
}

GLvoid APIENTRY __GLVVT_FUNC(DeleteTextures)(GLsizei n, const GLuint *textures)
{
    __GL_GET_DISPATCH_TABLE->DeleteTextures(n, textures);
}

GLvoid APIENTRY __GLVVT_FUNC(GenTextures)(GLsizei n, GLuint *textures)
{
    __GL_GET_DISPATCH_TABLE->GenTextures(n, textures);
}

GLboolean APIENTRY __GLVVT_FUNC(IsTexture)(GLuint texture)
{
   return __GL_GET_DISPATCH_TABLE->IsTexture(texture);
}

GLvoid APIENTRY __GLVVT_FUNC(PrioritizeTextures)(GLsizei n, const GLuint *textures, const GLclampf *priorities)
{
    __GL_GET_DISPATCH_TABLE->PrioritizeTextures(n, textures, priorities);
}

GLvoid APIENTRY __GLVVT_FUNC(Indexub)(GLubyte c)
{
    __GL_GET_DISPATCH_TABLE->Indexub(c);
}

GLvoid APIENTRY __GLVVT_FUNC(Indexubv)(const GLubyte *c)
{
    __GL_GET_DISPATCH_TABLE->Indexubv(c);
}

GLvoid APIENTRY __GLVVT_FUNC(PopClientAttrib)(GLvoid)
{
    __GL_GET_DISPATCH_TABLE->PopClientAttrib();
}

GLvoid APIENTRY __GLVVT_FUNC(PushClientAttrib)(GLbitfield mask)
{
    __GL_GET_DISPATCH_TABLE->PushClientAttrib(mask);
}

#endif

#if GL_VERSION_1_2

GLvoid APIENTRY __GLVVT_FUNC(BlendColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    __GL_GET_DISPATCH_TABLE->BlendColor(red, green, blue, alpha);
}

GLvoid APIENTRY __GLVVT_FUNC(BlendEquation)(GLenum mode)
{
    __GL_GET_DISPATCH_TABLE->BlendEquation(mode);
}

GLvoid APIENTRY __GLVVT_FUNC(DrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices)
{
    __GL_GET_DISPATCH_TABLE->DrawRangeElements(mode, start, end, count, type, indices);
}

GLvoid APIENTRY __GLVVT_FUNC(ColorTable)(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table)
{
    __GL_GET_DISPATCH_TABLE->ColorTable(target, internalformat, width, format, type, table);
}

GLvoid APIENTRY __GLVVT_FUNC(ColorTableParameterfv)(GLenum target, GLenum pname, const GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->ColorTableParameterfv(target, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(ColorTableParameteriv)(GLenum target, GLenum pname, const GLint *params)
{
    __GL_GET_DISPATCH_TABLE->ColorTableParameteriv(target, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(CopyColorTable)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)
{
    __GL_GET_DISPATCH_TABLE->CopyColorTable(target, internalformat, x, y, width);
}

GLvoid APIENTRY __GLVVT_FUNC(GetColorTable)(GLenum target, GLenum format, GLenum type, GLvoid *table)
{
    __GL_GET_DISPATCH_TABLE->GetColorTable(target, format, type, table);
}

GLvoid APIENTRY __GLVVT_FUNC(GetColorTableParameterfv)(GLenum target, GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->GetColorTableParameterfv(target, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetColorTableParameteriv)(GLenum target, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE->GetColorTableParameteriv(target, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(ColorSubTable)(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *table)
{
    __GL_GET_DISPATCH_TABLE->ColorSubTable(target, start, count, format, type, table);
}

GLvoid APIENTRY __GLVVT_FUNC(CopyColorSubTable)(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width)
{
    __GL_GET_DISPATCH_TABLE->CopyColorSubTable(target, start, x, y, width);
}

GLvoid APIENTRY __GLVVT_FUNC(ConvolutionFilter1D)(GLenum target, GLenum internalFormat,
    GLsizei width, GLenum format, GLenum type, const GLvoid* image)
{
    __GL_GET_DISPATCH_TABLE->ConvolutionFilter1D(target, internalFormat, width, format, type, image);
}

GLvoid APIENTRY __GLVVT_FUNC(ConvolutionFilter2D)(GLenum target, GLenum internalFormat,
    GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* image)
{
    __GL_GET_DISPATCH_TABLE->ConvolutionFilter2D(target, internalFormat, width, height, format, type, image);
}

GLvoid APIENTRY __GLVVT_FUNC(ConvolutionParameterf)(GLenum target, GLenum pname, GLfloat param)
{
    __GL_GET_DISPATCH_TABLE->ConvolutionParameterf(target, pname, param);
}

GLvoid APIENTRY __GLVVT_FUNC(ConvolutionParameterfv)(GLenum target, GLenum pname, const GLfloat* params)
{
    __GL_GET_DISPATCH_TABLE->ConvolutionParameterfv(target, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(ConvolutionParameteri)(GLenum target, GLenum pname, GLint param)
{
    __GL_GET_DISPATCH_TABLE->ConvolutionParameteri(target, pname, param);
}

GLvoid APIENTRY __GLVVT_FUNC(ConvolutionParameteriv)(GLenum target, GLenum pname, const GLint * param)
{
    __GL_GET_DISPATCH_TABLE->ConvolutionParameteriv(target, pname, param);
}

GLvoid APIENTRY __GLVVT_FUNC(CopyConvolutionFilter1D)(GLenum target, GLenum internalFormat,
                                         GLint x, GLint y, GLsizei width)
{
    __GL_GET_DISPATCH_TABLE->CopyConvolutionFilter1D(target, internalFormat, x, y, width);
}

GLvoid APIENTRY __GLVVT_FUNC(CopyConvolutionFilter2D)(GLenum target, GLenum internalFormat,
    GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_GET_DISPATCH_TABLE->CopyConvolutionFilter2D(target, internalFormat, x, y, width, height);
}

GLvoid APIENTRY __GLVVT_FUNC(GetConvolutionFilter)(GLenum target, GLenum format,GLenum type, GLvoid* image)
{
    __GL_GET_DISPATCH_TABLE->GetConvolutionFilter(target, format,type, image);
}

GLvoid APIENTRY __GLVVT_FUNC(GetConvolutionParameterfv)(GLenum target, GLenum pname, GLfloat* params)
{
    __GL_GET_DISPATCH_TABLE->GetConvolutionParameterfv(target, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetConvolutionParameteriv)(GLenum target, GLenum pname, GLint * params)
{
    __GL_GET_DISPATCH_TABLE->GetConvolutionParameteriv(target, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetSeparableFilter)(GLenum target, GLenum format, GLenum type, GLvoid* row, GLvoid* column, GLvoid* span)
{
    __GL_GET_DISPATCH_TABLE->GetSeparableFilter(target, format, type, row, column, span);
}

GLvoid APIENTRY __GLVVT_FUNC(SeparableFilter2D)(GLenum target, GLenum internalFormat,
    GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* row, const GLvoid* column)
{
    __GL_GET_DISPATCH_TABLE->SeparableFilter2D(target, internalFormat, width, height, format, type, row, column);
}

GLvoid APIENTRY __GLVVT_FUNC(GetHistogram)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values)
{
    __GL_GET_DISPATCH_TABLE->GetHistogram(target, reset, format, type, values);
}

GLvoid APIENTRY __GLVVT_FUNC(GetHistogramParameterfv)(GLenum target, GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->GetHistogramParameterfv(target, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetHistogramParameteriv)(GLenum target, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE->GetHistogramParameteriv(target, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetMinmax)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values)
{
    __GL_GET_DISPATCH_TABLE->GetMinmax(target, reset, format, type, values);
}

GLvoid APIENTRY __GLVVT_FUNC(GetMinmaxParameterfv)(GLenum target, GLenum pname, GLfloat *values)
{
    __GL_GET_DISPATCH_TABLE->GetMinmaxParameterfv(target, pname, values);
}

GLvoid APIENTRY __GLVVT_FUNC(GetMinmaxParameteriv)(GLenum target, GLenum pname, GLint *values)
{
    __GL_GET_DISPATCH_TABLE->GetMinmaxParameteriv(target, pname, values);
}

GLvoid APIENTRY __GLVVT_FUNC(Histogram)(GLenum target, GLsizei width, GLenum internalFormat, GLboolean sink)
{
    __GL_GET_DISPATCH_TABLE->Histogram(target, width, internalFormat, sink);
}

GLvoid APIENTRY __GLVVT_FUNC(Minmax)(GLenum target, GLenum internalFormat, GLboolean sink)
{
    __GL_GET_DISPATCH_TABLE->Minmax(target, internalFormat, sink);
}

GLvoid APIENTRY __GLVVT_FUNC(ResetHistogram)(GLenum target)
{
    __GL_GET_DISPATCH_TABLE->ResetHistogram(target);
}

GLvoid APIENTRY __GLVVT_FUNC(ResetMinmax)(GLenum target)
{
    __GL_GET_DISPATCH_TABLE->ResetMinmax(target);
}

GLvoid APIENTRY __GLVVT_FUNC(TexImage3D)(GLenum target, GLint lod, GLint components, GLsizei w, GLsizei h,GLsizei d,
                            GLint border, GLenum format, GLenum type, const GLvoid *buf)
{
    __GL_GET_DISPATCH_TABLE->TexImage3D(target, lod, components, w, h,d, border, format, type, buf);
}

GLvoid APIENTRY __GLVVT_FUNC(TexSubImage3D)(GLenum target, GLint lod, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei w, GLsizei h,
                               GLsizei d, GLenum format, GLenum type, const GLvoid *buf)
{
    __GL_GET_DISPATCH_TABLE->TexSubImage3D(target, lod, xoffset, yoffset, zoffset, w, h, d, format, type, buf);
}

GLvoid APIENTRY __GLVVT_FUNC(CopyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x,
                                   GLint y, GLsizei width, GLsizei height)
{
    __GL_GET_DISPATCH_TABLE->CopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
}

#endif /* GL_VERSION_1_2 */

#if GL_VERSION_1_3

GLvoid APIENTRY __GLVVT_FUNC(ActiveTexture)(GLenum texture )
{
    __GL_GET_DISPATCH_TABLE->ActiveTexture(texture );
}

GLvoid APIENTRY __GLVVT_FUNC(ClientActiveTexture)(GLenum texture )
{
    __GL_GET_DISPATCH_TABLE->ClientActiveTexture(texture );
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord1d)(GLenum texture, GLdouble x)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord1d(texture, x);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord1dv)(GLenum texture, const GLdouble x[1])
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord1dv(texture, x);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord1f)(GLenum texture, GLfloat x)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord1f(texture, x);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord1fv)(GLenum texture, const GLfloat * coord)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord1fv(texture, coord);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord1i)(GLenum texture, GLint x)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord1i(texture, x);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord1iv)(GLenum texture, const GLint * coord)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord1iv(texture, coord);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord1s)(GLenum texture, GLshort x)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord1s(texture, x);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord1sv)(GLenum texture, const GLshort * coord)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord1sv(texture, coord);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord2d)(GLenum texture, GLdouble x, GLdouble y)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord2d(texture, x, y);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord2dv)(GLenum texture, const GLdouble *coord)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord2dv(texture, coord);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord2f)(GLenum texture, GLfloat x, GLfloat y)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord2f(texture, x, y);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord2fv)(GLenum texture, const GLfloat * coord)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord2fv(texture, coord);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord2i)(GLenum texture, GLint x, GLint y)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord2i(texture, x, y);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord2iv)(GLenum texture, const GLint * coord)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord2iv(texture, coord);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord2s)(GLenum texture, GLshort x, GLshort y)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord2s(texture, x, y);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord2sv)(GLenum texture, const GLshort * coord)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord2sv(texture, coord);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord3d)(GLenum texture, GLdouble x, GLdouble y, GLdouble z)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord3d(texture, x, y, z);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord3dv)(GLenum texture, const GLdouble * coord)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord3dv(texture, coord);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord3f)(GLenum texture, GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord3f(texture, x, y, z);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord3fv)(GLenum texture, const GLfloat * coord)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord3fv(texture, coord);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord3i)(GLenum texture, GLint x, GLint y, GLint z)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord3i(texture, x, y, z);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord3iv)(GLenum texture, const GLint * coord)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord3iv(texture, coord);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord3s)(GLenum texture, GLshort x, GLshort y, GLshort z)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord3s(texture, x, y, z);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord3sv)(GLenum texture, const GLshort * coord)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord3sv(texture, coord);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord4d)(GLenum texture, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord4d(texture, x, y, z, w);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord4dv)(GLenum texture, const GLdouble * coord)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord4dv(texture, coord);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord4f)(GLenum texture, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord4f(texture, x, y, z, w);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord4fv)(GLenum texture, const GLfloat * coord)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord4fv(texture, coord);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord4i)(GLenum texture, GLint x, GLint y, GLint z, GLint w)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord4i(texture, x, y, z, w);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord4iv)(GLenum texture, const GLint * coord)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord4iv(texture, coord);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord4s)(GLenum texture, GLshort x, GLshort y, GLshort z, GLshort w)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord4s(texture, x, y, z, w);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord4sv)(GLenum texture, const GLshort * coord)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord4sv(texture, coord);
}

GLvoid APIENTRY __GLVVT_FUNC(LoadTransposeMatrixf)(const GLfloat * matrix)
{
    __GL_GET_DISPATCH_TABLE->LoadTransposeMatrixf(matrix);
}

GLvoid APIENTRY __GLVVT_FUNC(LoadTransposeMatrixd)(const GLdouble * matrix)
{
    __GL_GET_DISPATCH_TABLE->LoadTransposeMatrixd(matrix);
}

GLvoid APIENTRY __GLVVT_FUNC(MultTransposeMatrixf)(const GLfloat * matrix)
{
    __GL_GET_DISPATCH_TABLE->MultTransposeMatrixf(matrix);
}

GLvoid APIENTRY __GLVVT_FUNC(MultTransposeMatrixd)(const GLdouble * matrix)
{
    __GL_GET_DISPATCH_TABLE->MultTransposeMatrixd(matrix);
}

GLvoid APIENTRY __GLVVT_FUNC(SampleCoverage)(GLclampf value, GLboolean invert)
{
    __GL_GET_DISPATCH_TABLE->SampleCoverage(value, invert);
}

GLvoid APIENTRY __GLVVT_FUNC(CompressedTexImage3D)(GLenum target, GLint level, GLenum internalformat,
    GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data)
{
    __GL_GET_DISPATCH_TABLE->CompressedTexImage3D(target, level, internalformat,
        width, height, depth, border, imageSize, data);
}

GLvoid APIENTRY __GLVVT_FUNC(CompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat,
    GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data)
{
    __GL_GET_DISPATCH_TABLE->CompressedTexImage2D(target, level, internalformat,
        width, height, border, imageSize, data);
}

GLvoid APIENTRY __GLVVT_FUNC(CompressedTexImage1D)(GLenum target, GLint level, GLenum internalformat,
    GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data)
{
    __GL_GET_DISPATCH_TABLE->CompressedTexImage1D(target, level, internalformat,
        width, border, imageSize, data);
}

GLvoid APIENTRY __GLVVT_FUNC(CompressedTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
    GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data)
{
    __GL_GET_DISPATCH_TABLE->CompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset,
        width, height, depth, format, imageSize, data);
}

GLvoid APIENTRY __GLVVT_FUNC(CompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset,
    GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data)
{
    __GL_GET_DISPATCH_TABLE->CompressedTexSubImage2D(target, level, xoffset, yoffset,
        width, height, format, imageSize, data);
}

GLvoid APIENTRY __GLVVT_FUNC(CompressedTexSubImage1D)(GLenum target, GLint level, GLint xoffset,
    GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data)
{
    __GL_GET_DISPATCH_TABLE->CompressedTexSubImage1D(target, level, xoffset,
        width, format, imageSize, data);
}

GLvoid APIENTRY __GLVVT_FUNC(GetCompressedTexImage)(GLenum texture, GLint x, GLvoid * buf)
{
    __GL_GET_DISPATCH_TABLE->GetCompressedTexImage(texture, x, buf);
}

#endif /* GL_VERSION_1_3 */

#if GL_VERSION_1_4

GLvoid APIENTRY __GLVVT_FUNC(BlendFuncSeparate)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorA, GLenum dfactorA)
{
    __GL_GET_DISPATCH_TABLE->BlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorA, dfactorA);
}

GLvoid APIENTRY __GLVVT_FUNC(FogCoordf)(GLfloat coord)
{
    __GL_GET_DISPATCH_TABLE->FogCoordf(coord);
}

GLvoid APIENTRY __GLVVT_FUNC(FogCoordfv)(const GLfloat * coord)
{
    __GL_GET_DISPATCH_TABLE->FogCoordfv(coord);
}

GLvoid APIENTRY __GLVVT_FUNC(FogCoordd)(GLdouble coord)
{
    __GL_GET_DISPATCH_TABLE->FogCoordd(coord);
}

GLvoid APIENTRY __GLVVT_FUNC(FogCoorddv)(const GLdouble * coord)
{
    __GL_GET_DISPATCH_TABLE->FogCoorddv(coord);
}

GLvoid APIENTRY __GLVVT_FUNC(FogCoordPointer)(GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_GET_DISPATCH_TABLE->FogCoordPointer(type, stride, pointer);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiDrawArrays)(GLenum mode, GLint *first, GLsizei *count, GLsizei primcount)
{
    __GL_GET_DISPATCH_TABLE->MultiDrawArrays(mode, first, count, primcount);
}

GLvoid APIENTRY __GLVVT_FUNC(MultiDrawElements)(GLenum mode, const GLsizei *count, GLenum type, const GLvoid **indices, GLsizei primcount)
{
    __GL_GET_DISPATCH_TABLE->MultiDrawElements(mode,count,type,indices,primcount);
}

GLvoid APIENTRY __GLVVT_FUNC(PointParameterf)(GLenum pname, GLfloat params)
{
    __GL_GET_DISPATCH_TABLE->PointParameterf(pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(PointParameterfv)(GLenum pname, const GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->PointParameterfv(pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(PointParameteri)(GLenum pname, GLint param)
{
    __GL_GET_DISPATCH_TABLE->PointParameteri(pname, param);
}

GLvoid APIENTRY __GLVVT_FUNC(PointParameteriv)(GLenum pname, const GLint *params)
{
    __GL_GET_DISPATCH_TABLE->PointParameteriv(pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(SecondaryColor3b)(GLbyte red, GLbyte green, GLbyte blue)
{
    __GL_GET_DISPATCH_TABLE->SecondaryColor3b(red, green, blue);
}

GLvoid APIENTRY __GLVVT_FUNC(SecondaryColor3bv)(const GLbyte *v)
{
    __GL_GET_DISPATCH_TABLE->SecondaryColor3bv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(SecondaryColor3d)(GLdouble red, GLdouble green, GLdouble blue)
{
    __GL_GET_DISPATCH_TABLE->SecondaryColor3d(red, green, blue);
}

GLvoid APIENTRY __GLVVT_FUNC(SecondaryColor3dv)(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE->SecondaryColor3dv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(SecondaryColor3f)(GLfloat red, GLfloat green, GLfloat blue)
{
    __GL_GET_DISPATCH_TABLE->SecondaryColor3f(red, green, blue);
}

GLvoid APIENTRY __GLVVT_FUNC(SecondaryColor3fv)(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE->SecondaryColor3fv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(SecondaryColor3i)(GLint red, GLint green, GLint blue)
{
    __GL_GET_DISPATCH_TABLE->SecondaryColor3i(red, green, blue);
}

GLvoid APIENTRY __GLVVT_FUNC(SecondaryColor3iv)(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE->SecondaryColor3iv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(SecondaryColor3s)(GLshort red, GLshort green, GLshort blue)
{
    __GL_GET_DISPATCH_TABLE->SecondaryColor3s(red, green, blue);
}

GLvoid APIENTRY __GLVVT_FUNC(SecondaryColor3sv)(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE->SecondaryColor3sv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(SecondaryColor3ub)(GLubyte red, GLubyte green, GLubyte blue)
{
    __GL_GET_DISPATCH_TABLE->SecondaryColor3ub(red, green, blue);
}

GLvoid APIENTRY __GLVVT_FUNC(SecondaryColor3ubv)(const GLubyte *v)
{
    __GL_GET_DISPATCH_TABLE->SecondaryColor3ubv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(SecondaryColor3ui)(GLuint red, GLuint green, GLuint blue)
{
    __GL_GET_DISPATCH_TABLE->SecondaryColor3ui(red, green, blue);
}

GLvoid APIENTRY __GLVVT_FUNC(SecondaryColor3uiv)(const GLuint *v)
{
    __GL_GET_DISPATCH_TABLE->SecondaryColor3uiv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(SecondaryColor3us)(GLushort red, GLushort green, GLushort blue)
{
    __GL_GET_DISPATCH_TABLE->SecondaryColor3us(red, green, blue);
}

GLvoid APIENTRY __GLVVT_FUNC(SecondaryColor3usv)(const GLushort *v)
{
    __GL_GET_DISPATCH_TABLE->SecondaryColor3usv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(SecondaryColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_GET_DISPATCH_TABLE->SecondaryColorPointer(size, type, stride, pointer);
}

GLvoid APIENTRY __GLVVT_FUNC(WindowPos2d)(GLdouble x, GLdouble y)
{
    __GL_GET_DISPATCH_TABLE->WindowPos2d(x, y);
}

GLvoid APIENTRY __GLVVT_FUNC(WindowPos2dv)(const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE->WindowPos2dv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(WindowPos2f)(GLfloat x, GLfloat y)
{
    __GL_GET_DISPATCH_TABLE->WindowPos2f(x, y);
}

GLvoid APIENTRY __GLVVT_FUNC(WindowPos2fv)(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE->WindowPos2fv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(WindowPos2i)(GLint x, GLint y)
{
    __GL_GET_DISPATCH_TABLE->WindowPos2i(x, y);
}

GLvoid APIENTRY __GLVVT_FUNC(WindowPos2iv)(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE->WindowPos2iv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(WindowPos2s)(GLshort x, GLshort y)
{
    __GL_GET_DISPATCH_TABLE->WindowPos2s(x, y);
}

GLvoid APIENTRY __GLVVT_FUNC(WindowPos2sv)(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE->WindowPos2sv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(WindowPos3d)(GLdouble x, GLdouble y, GLdouble z)
{
    __GL_GET_DISPATCH_TABLE->WindowPos3d(x, y, z);
}

GLvoid APIENTRY __GLVVT_FUNC(WindowPos3dv)(const GLdouble * v)
{
    __GL_GET_DISPATCH_TABLE->WindowPos3dv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(WindowPos3f)(GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_DISPATCH_TABLE->WindowPos3f(x, y, z);
}

GLvoid APIENTRY __GLVVT_FUNC(WindowPos3fv)(const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE->WindowPos3fv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(WindowPos3i)(GLint x, GLint y, GLint z)
{
    __GL_GET_DISPATCH_TABLE->WindowPos3i(x, y, z);
}

GLvoid APIENTRY __GLVVT_FUNC(WindowPos3iv)(const GLint *v)
{
    __GL_GET_DISPATCH_TABLE->WindowPos3iv(v);
}

GLvoid APIENTRY __GLVVT_FUNC(WindowPos3s)(GLshort x, GLshort y, GLshort z)
{
    __GL_GET_DISPATCH_TABLE->WindowPos3s(x, y, z);
}

GLvoid APIENTRY __GLVVT_FUNC(WindowPos3sv)(const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE->WindowPos3sv(v);
}

#endif /* GL_VERSION_1_4 */

#if GL_VERSION_1_5

GLvoid APIENTRY __GLVVT_FUNC(GenQueries)(GLsizei n, GLuint *ids)
{
    __GL_GET_DISPATCH_TABLE->GenQueries(n, ids);
}

GLvoid APIENTRY __GLVVT_FUNC(DeleteQueries)(GLsizei n, const GLuint *ids)
{
    __GL_GET_DISPATCH_TABLE->DeleteQueries(n, ids);
}

GLboolean APIENTRY __GLVVT_FUNC(IsQuery)(GLuint id)
{
    return __GL_GET_DISPATCH_TABLE->IsQuery(id);
}

GLvoid APIENTRY __GLVVT_FUNC(BeginQuery)(GLenum target, GLuint id)
{
    __GL_GET_DISPATCH_TABLE->BeginQuery(target, id);
}

GLvoid APIENTRY __GLVVT_FUNC(EndQuery)(GLenum target)
{
    __GL_GET_DISPATCH_TABLE->EndQuery(target);
}

GLvoid APIENTRY __GLVVT_FUNC(GetQueryiv)(GLenum target, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE->GetQueryiv(target, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetQueryObjectiv)(GLuint id, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE->GetQueryObjectiv(id, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetQueryObjectuiv)(GLuint id, GLenum pname, GLuint *params)
{
    __GL_GET_DISPATCH_TABLE->GetQueryObjectuiv(id, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(BindBuffer)(GLenum target, GLuint buffer)
{
    __GL_GET_DISPATCH_TABLE->BindBuffer(target, buffer);
}

GLvoid APIENTRY __GLVVT_FUNC(DeleteBuffers)(GLsizei n, const GLuint *buffers)
{
    __GL_GET_DISPATCH_TABLE->DeleteBuffers(n, buffers);
}

GLvoid APIENTRY __GLVVT_FUNC(GenBuffers)(GLsizei n, GLuint * buffers)
{
    __GL_GET_DISPATCH_TABLE->GenBuffers(n, buffers);
}

GLboolean APIENTRY __GLVVT_FUNC(IsBuffer)(GLuint buffer)
{
    return __GL_GET_DISPATCH_TABLE->IsBuffer(buffer);
}

GLvoid APIENTRY __GLVVT_FUNC(BufferData)(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
{
    __GL_GET_DISPATCH_TABLE->BufferData(target, size, data, usage);
}

GLvoid APIENTRY __GLVVT_FUNC(BufferSubData)(GLenum tareget, GLintptr offset , GLsizeiptr size, const GLvoid *data)
{
    __GL_GET_DISPATCH_TABLE->BufferSubData(tareget, offset , size, data);
}

GLvoid APIENTRY __GLVVT_FUNC(GetBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data)
{
    __GL_GET_DISPATCH_TABLE->GetBufferSubData(target, offset, size, data);
}

GLvoid* APIENTRY __GLVVT_FUNC(MapBuffer)(GLenum target, GLenum access)
{
    return __GL_GET_DISPATCH_TABLE->MapBuffer(target, access);
}

GLboolean APIENTRY __GLVVT_FUNC(UnmapBuffer)(GLenum target)
{
    return __GL_GET_DISPATCH_TABLE->UnmapBuffer(target);
}

GLvoid APIENTRY __GLVVT_FUNC(GetBufferParameteriv)(GLenum target, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE->GetBufferParameteriv(target, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetBufferPointerv)(GLenum target, GLenum pname, GLvoid* *params)
{
    __GL_GET_DISPATCH_TABLE->GetBufferPointerv(target, pname, params);
}

#endif  /* GL_VERSION_1_5 */

#if GL_VERSION_2_0

GLvoid APIENTRY __GLVVT_FUNC(BlendEquationSeparate)(GLenum modeRGB, GLenum modeAlpha)
{
    __GL_GET_DISPATCH_TABLE->BlendEquationSeparate(modeRGB, modeAlpha);
}

GLvoid APIENTRY __GLVVT_FUNC(DrawBuffers)(GLsizei count, const GLenum *bufs)
{
    __GL_GET_DISPATCH_TABLE->DrawBuffers(count, bufs);
}

GLvoid APIENTRY __GLVVT_FUNC(StencilOpSeparate)(GLenum face, GLenum fail, GLenum depthFail, GLenum depthPass)
{
    __GL_GET_DISPATCH_TABLE->StencilOpSeparate(face, fail, depthFail, depthPass);
}

GLvoid APIENTRY __GLVVT_FUNC(StencilFuncSeparate)(GLenum face, GLenum func, GLint ref, GLuint mask)
{
    __GL_GET_DISPATCH_TABLE->StencilFuncSeparate(face, func, ref, mask);
}

GLvoid APIENTRY __GLVVT_FUNC(StencilMaskSeparate)(GLenum face, GLuint sm)
{
    __GL_GET_DISPATCH_TABLE->StencilMaskSeparate(face, sm);
}

GLvoid APIENTRY __GLVVT_FUNC(AttachShader)(GLuint program, GLuint shader)
{
    __GL_GET_DISPATCH_TABLE->AttachShader(program, shader);
}

GLvoid APIENTRY __GLVVT_FUNC(BindAttribLocation)(GLuint program, GLuint index, const GLchar *name)
{
    __GL_GET_DISPATCH_TABLE->BindAttribLocation(program, index, name);
}

GLvoid APIENTRY __GLVVT_FUNC(CompileShader)(GLuint shader)
{
    __GL_GET_DISPATCH_TABLE->CompileShader(shader);
}

GLuint APIENTRY __GLVVT_FUNC(CreateProgram)(GLvoid)
{
   return __GL_GET_DISPATCH_TABLE->CreateProgram();
}

GLuint APIENTRY __GLVVT_FUNC(CreateShader)(GLenum type)
{
   return __GL_GET_DISPATCH_TABLE->CreateShader(type);
}

GLvoid APIENTRY __GLVVT_FUNC(DeleteProgram)(GLuint program)
{
    __GL_GET_DISPATCH_TABLE->DeleteProgram(program);
}

GLvoid APIENTRY __GLVVT_FUNC(DeleteShader)(GLuint shader)
{
    __GL_GET_DISPATCH_TABLE->DeleteShader(shader);
}

GLvoid APIENTRY __GLVVT_FUNC(DetachShader)(GLuint program, GLuint shader)
{
    __GL_GET_DISPATCH_TABLE->DetachShader(program, shader);
}

GLvoid APIENTRY __GLVVT_FUNC(DisableVertexAttribArray)(GLuint index)
{
    __GL_GET_DISPATCH_TABLE->DisableVertexAttribArray(index);
}

GLvoid APIENTRY __GLVVT_FUNC(EnableVertexAttribArray)(GLuint index)
{
    __GL_GET_DISPATCH_TABLE->EnableVertexAttribArray(index);
}

GLvoid APIENTRY __GLVVT_FUNC(GetActiveAttrib)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
    __GL_GET_DISPATCH_TABLE->GetActiveAttrib(program, index, bufSize, length, size, type, name);
}

GLvoid APIENTRY __GLVVT_FUNC(GetActiveUniform)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
    __GL_GET_DISPATCH_TABLE->GetActiveUniform(program, index, bufSize, length, size, type, name);
}

GLvoid APIENTRY __GLVVT_FUNC(GetAttachedShaders)(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *obj)
{
    __GL_GET_DISPATCH_TABLE->GetAttachedShaders(program, maxCount, count, obj);
}

GLint APIENTRY __GLVVT_FUNC(GetAttribLocation)(GLuint program, const GLchar *name)
{
   return __GL_GET_DISPATCH_TABLE->GetAttribLocation(program, name);
}

GLvoid APIENTRY __GLVVT_FUNC(GetProgramiv)(GLuint program, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE->GetProgramiv(program, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    __GL_GET_DISPATCH_TABLE->GetProgramInfoLog(program, bufSize, length, infoLog);
}

GLvoid APIENTRY __GLVVT_FUNC(GetShaderiv)(GLuint shader, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE->GetShaderiv(shader, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    __GL_GET_DISPATCH_TABLE->GetShaderInfoLog(shader, bufSize, length, infoLog);
}

GLvoid APIENTRY __GLVVT_FUNC(GetShaderSource)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source)
{
    __GL_GET_DISPATCH_TABLE->GetShaderSource(shader, bufSize, length, source);
}

GLint APIENTRY __GLVVT_FUNC(GetUniformLocation)(GLuint program, const GLchar *name)
{
   return __GL_GET_DISPATCH_TABLE->GetUniformLocation(program, name);
}

GLvoid APIENTRY __GLVVT_FUNC(GetUniformfv)(GLuint program, GLint location, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->GetUniformfv(program, location, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetUniformiv)(GLuint program, GLint location, GLint *params)
{
    __GL_GET_DISPATCH_TABLE->GetUniformiv(program, location, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetVertexAttribdv)(GLuint index, GLenum pname, GLdouble *params)
{
    __GL_GET_DISPATCH_TABLE->GetVertexAttribdv(index, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetVertexAttribfv)(GLuint index, GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->GetVertexAttribfv(index, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetVertexAttribiv)(GLuint index, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE->GetVertexAttribiv(index, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetVertexAttribPointerv)(GLuint index, GLenum pname, GLvoid **pointer)
{
    __GL_GET_DISPATCH_TABLE->GetVertexAttribPointerv(index, pname, pointer);
}

GLboolean APIENTRY __GLVVT_FUNC(IsProgram)(GLuint program)
{
  return __GL_GET_DISPATCH_TABLE->IsProgram(program);
}

GLboolean APIENTRY __GLVVT_FUNC(IsShader)(GLuint shader)
{
  return __GL_GET_DISPATCH_TABLE->IsShader(shader);
}

GLvoid APIENTRY __GLVVT_FUNC(LinkProgram)(GLuint program)
{
    __GL_GET_DISPATCH_TABLE->LinkProgram(program);
}

GLvoid APIENTRY __GLVVT_FUNC(ShaderSource)(GLuint shader, GLsizei count, const GLchar* *string, const GLint *length)
{
    __GL_GET_DISPATCH_TABLE->ShaderSource(shader, count, string, length);
}

GLvoid APIENTRY __GLVVT_FUNC(UseProgram)(GLuint program)
{
    __GL_GET_DISPATCH_TABLE->UseProgram(program);
}

GLvoid APIENTRY __GLVVT_FUNC(Uniform1f)(GLint location, GLfloat v0)
{
    __GL_GET_DISPATCH_TABLE->Uniform1f(location, v0);
}

GLvoid APIENTRY __GLVVT_FUNC(Uniform2f)(GLint location, GLfloat v0, GLfloat v1)
{
    __GL_GET_DISPATCH_TABLE->Uniform2f(location, v0, v1);
}

GLvoid APIENTRY __GLVVT_FUNC(Uniform3f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    __GL_GET_DISPATCH_TABLE->Uniform3f(location, v0, v1, v2);
}

GLvoid APIENTRY __GLVVT_FUNC(Uniform4f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    __GL_GET_DISPATCH_TABLE->Uniform4f(location, v0, v1, v2, v3);
}

GLvoid APIENTRY __GLVVT_FUNC(Uniform1i)(GLint location, GLint v0)
{
    __GL_GET_DISPATCH_TABLE->Uniform1i(location, v0);
}

GLvoid APIENTRY __GLVVT_FUNC(Uniform2i)(GLint location, GLint v0, GLint v1)
{
    __GL_GET_DISPATCH_TABLE->Uniform2i(location, v0, v1);
}

GLvoid APIENTRY __GLVVT_FUNC(Uniform3i)(GLint location, GLint v0, GLint v1, GLint v2)
{
    __GL_GET_DISPATCH_TABLE->Uniform3i(location, v0, v1, v2);
}

GLvoid APIENTRY __GLVVT_FUNC(Uniform4i)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
    __GL_GET_DISPATCH_TABLE->Uniform4i(location, v0, v1, v2, v3);
}

GLvoid APIENTRY __GLVVT_FUNC(Uniform1fv)(GLint location, GLsizei count, const GLfloat *value)
{
    __GL_GET_DISPATCH_TABLE->Uniform1fv(location, count, value);
}

GLvoid APIENTRY __GLVVT_FUNC(Uniform2fv)(GLint location, GLsizei count, const GLfloat *value)
{
    __GL_GET_DISPATCH_TABLE->Uniform2fv(location, count, value);
}

GLvoid APIENTRY __GLVVT_FUNC(Uniform3fv)(GLint location, GLsizei count, const GLfloat *value)
{
    __GL_GET_DISPATCH_TABLE->Uniform3fv(location, count, value);
}

GLvoid APIENTRY __GLVVT_FUNC(Uniform4fv)(GLint location, GLsizei count, const GLfloat *value)
{
    __GL_GET_DISPATCH_TABLE->Uniform4fv(location, count, value);
}

GLvoid APIENTRY __GLVVT_FUNC(Uniform1iv)(GLint location, GLsizei count, const GLint *value)
{
    __GL_GET_DISPATCH_TABLE->Uniform1iv(location, count, value);
}

GLvoid APIENTRY __GLVVT_FUNC(Uniform2iv)(GLint location, GLsizei count, const GLint *value)
{
    __GL_GET_DISPATCH_TABLE->Uniform2iv(location, count, value);
}

GLvoid APIENTRY __GLVVT_FUNC(Uniform3iv)(GLint location, GLsizei count, const GLint *value)
{
    __GL_GET_DISPATCH_TABLE->Uniform3iv(location, count, value);
}

GLvoid APIENTRY __GLVVT_FUNC(Uniform4iv)(GLint location, GLsizei count, const GLint *value)
{
    __GL_GET_DISPATCH_TABLE->Uniform4iv(location, count, value);
}

GLvoid APIENTRY __GLVVT_FUNC(UniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_DISPATCH_TABLE->UniformMatrix2fv(location, count,transpose, value);
}

GLvoid APIENTRY __GLVVT_FUNC(UniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_DISPATCH_TABLE->UniformMatrix3fv(location, count,transpose, value);
}

GLvoid APIENTRY __GLVVT_FUNC(UniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_DISPATCH_TABLE->UniformMatrix4fv(location, count,transpose, value);
}

GLvoid APIENTRY __GLVVT_FUNC(ValidateProgram)(GLuint program)
{
    __GL_GET_DISPATCH_TABLE->ValidateProgram(program);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib1s)(GLuint index, GLshort x)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib1s(index, x);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib1f)(GLuint index, GLfloat x)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib1f(index, x);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib1d)(GLuint index, GLdouble x)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib1d(index, x);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib2s)(GLuint index, GLshort x, GLshort y)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib2s(index, x, y);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib2f)(GLuint index, GLfloat x, GLfloat y)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib2f(index, x, y);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib2d)(GLuint index, GLdouble x, GLdouble y)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib2d(index, x, y);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib3s)(GLuint index, GLshort x, GLshort y, GLshort z)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib3s(index, x, y, z);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib3f)(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib3f(index, x, y, z);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib3d)(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib3d(index, x, y, z);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib4s)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib4s(index, x, y, z, w);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib4f)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib4f(index, x, y, z, w);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib4d)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib4d(index, x, y, z, w);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib4Nub)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib4Nub(index, x, y, z, w);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib1sv)(GLuint index, const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib1sv(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib1fv)(GLuint index, const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib1fv(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib1dv)(GLuint index, const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib1dv(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib2sv)(GLuint index, const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib2sv(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib2fv)(GLuint index, const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib2fv(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib2dv)(GLuint index, const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib2dv(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib3sv)(GLuint index, const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib3sv(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib3fv)(GLuint index, const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib3fv(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib3dv)(GLuint index, const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib3dv(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib4bv)(GLuint index, const GLbyte *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib4bv(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib4sv)(GLuint index, const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib4sv(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib4iv)(GLuint index, const GLint *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib4iv(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib4ubv)(GLuint index, const GLubyte *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib4ubv(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib4usv)(GLuint index, const GLushort *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib4usv(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib4uiv)(GLuint index, const GLuint *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib4uiv(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib4fv)(GLuint index, const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib4fv(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib4dv)(GLuint index, const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib4dv(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib4Nbv)(GLuint index, const GLbyte *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib4Nbv(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib4Nsv)(GLuint index, const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib4Nsv(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib4Niv)(GLuint index, const GLint *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib4Niv(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib4Nubv)(GLuint index, const GLubyte *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib4Nubv(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib4Nusv)(GLuint index, const GLushort *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib4Nusv(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttrib4Nuiv)(GLuint index, const GLuint *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttrib4Nuiv(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttribPointer)(GLuint index, GLint size, GLenum type,
                                GLboolean normalized, GLsizei stride, const GLvoid *pointer)
{
    __GL_GET_DISPATCH_TABLE->VertexAttribPointer(index, size, type, normalized, stride, pointer);
}

#endif

#if GL_VERSION_2_1
GLvoid APIENTRY __GLVVT_FUNC(UniformMatrix2x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_DISPATCH_TABLE->UniformMatrix2x3fv(location, count,transpose, value);
}

GLvoid APIENTRY __GLVVT_FUNC(UniformMatrix2x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_DISPATCH_TABLE->UniformMatrix2x4fv(location, count,transpose, value);
}

GLvoid APIENTRY __GLVVT_FUNC(UniformMatrix3x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_DISPATCH_TABLE->UniformMatrix3x2fv(location, count,transpose, value);
}

GLvoid APIENTRY __GLVVT_FUNC(UniformMatrix3x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_DISPATCH_TABLE->UniformMatrix3x4fv(location, count,transpose, value);
}

GLvoid APIENTRY __GLVVT_FUNC(UniformMatrix4x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_DISPATCH_TABLE->UniformMatrix4x2fv(location, count,transpose, value);
}

GLvoid APIENTRY __GLVVT_FUNC(UniformMatrix4x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_DISPATCH_TABLE->UniformMatrix4x3fv(location, count,transpose, value);
}
#endif

#if GL_ARB_vertex_program

GLvoid APIENTRY __GLVVT_FUNC(ProgramStringARB)(GLenum target, GLenum format, GLsizei len, const GLvoid *string)
{
    __GL_GET_DISPATCH_TABLE->ProgramStringARB(target, format, len, string);
}

GLvoid APIENTRY __GLVVT_FUNC(BindProgramARB)(GLenum target, GLuint program)
{
    __GL_GET_DISPATCH_TABLE->BindProgramARB(target, program);
}

GLvoid APIENTRY __GLVVT_FUNC(DeleteProgramsARB)(GLsizei n, const GLuint *programs)
{
    __GL_GET_DISPATCH_TABLE->DeleteProgramsARB(n, programs);
}

GLvoid APIENTRY __GLVVT_FUNC(GenProgramsARB)(GLsizei n, GLuint *programs)
{
    __GL_GET_DISPATCH_TABLE->GenProgramsARB(n, programs);
}

GLvoid APIENTRY __GLVVT_FUNC(ProgramEnvParameter4dARB)(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GL_GET_DISPATCH_TABLE->ProgramEnvParameter4dARB(target, index, x, y, z, w);
}

GLvoid APIENTRY __GLVVT_FUNC(ProgramEnvParameter4dvARB)(GLenum target, GLuint index, const GLdouble *params)
{
    __GL_GET_DISPATCH_TABLE->ProgramEnvParameter4dvARB(target, index, params);
}

GLvoid APIENTRY __GLVVT_FUNC(ProgramEnvParameter4fARB)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GL_GET_DISPATCH_TABLE->ProgramEnvParameter4fARB(target, index, x, y, z, w);
}

GLvoid APIENTRY __GLVVT_FUNC(ProgramEnvParameter4fvARB)(GLenum target, GLuint index, const GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->ProgramEnvParameter4fvARB(target, index, params);
}

GLvoid APIENTRY __GLVVT_FUNC(ProgramLocalParameter4dARB)(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GL_GET_DISPATCH_TABLE->ProgramLocalParameter4dARB(target, index, x, y, z, w);
}

GLvoid APIENTRY __GLVVT_FUNC(ProgramLocalParameter4dvARB)(GLenum target, GLuint index, const GLdouble *params)
{
    __GL_GET_DISPATCH_TABLE->ProgramLocalParameter4dvARB(target, index, params);
}

GLvoid APIENTRY __GLVVT_FUNC(ProgramLocalParameter4fARB)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GL_GET_DISPATCH_TABLE->ProgramLocalParameter4fARB(target, index, x, y, z, w);
}

GLvoid APIENTRY __GLVVT_FUNC(ProgramLocalParameter4fvARB)(GLenum target, GLuint index,const GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->ProgramLocalParameter4fvARB(target, index, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetProgramEnvParameterdvARB)(GLenum target, GLuint index, GLdouble *params)
{
    __GL_GET_DISPATCH_TABLE->GetProgramEnvParameterdvARB(target, index, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetProgramEnvParameterfvARB)(GLenum target, GLuint index, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->GetProgramEnvParameterfvARB(target, index, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetProgramLocalParameterdvARB)(GLenum target, GLuint index, GLdouble *params)
{
    __GL_GET_DISPATCH_TABLE->GetProgramLocalParameterdvARB(target, index, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetProgramLocalParameterfvARB)(GLenum target, GLuint index, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->GetProgramLocalParameterfvARB(target, index, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetProgramivARB)(GLenum target, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE->GetProgramivARB(target, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetProgramStringARB)(GLenum target, GLenum pname, GLvoid *string)
{
    __GL_GET_DISPATCH_TABLE->GetProgramStringARB(target, pname,string);
}

GLboolean APIENTRY __GLVVT_FUNC(IsProgramARB)(GLuint program)
{
    return __GL_GET_DISPATCH_TABLE->IsProgramARB(program);
}

#endif  /* GL_ARB_vertex_program */

#if GL_ARB_shader_objects

GLvoid APIENTRY __GLVVT_FUNC(DeleteObjectARB)(GLhandleARB obj)
{
    __GL_GET_DISPATCH_TABLE->DeleteObjectARB(obj);
}

GLhandleARB APIENTRY __GLVVT_FUNC(GetHandleARB)(GLenum pname)
{
    return __GL_GET_DISPATCH_TABLE->GetHandleARB(pname);
}

GLvoid APIENTRY __GLVVT_FUNC(GetInfoLogARB)(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog)
{
    __GL_GET_DISPATCH_TABLE->GetInfoLogARB(obj, maxLength, length, infoLog);
}

GLvoid APIENTRY __GLVVT_FUNC(GetObjectParameterfvARB)(GLhandleARB obj, GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->GetObjectParameterfvARB(obj, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetObjectParameterivARB)(GLhandleARB obj, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE->GetObjectParameterivARB(obj, pname, params);
}

#endif /* GL_ARB_shader_objects */

#if GL_ATI_vertex_array_object

GLuint APIENTRY __GLVVT_FUNC(NewObjectBufferATI)(GLsizei size, const GLvoid *pointer, GLenum usage)
{
    return __GL_GET_DISPATCH_TABLE->NewObjectBufferATI(size, pointer, usage);
}

GLboolean APIENTRY __GLVVT_FUNC(IsObjectBufferATI)(GLuint buffer)
{
    return __GL_GET_DISPATCH_TABLE->IsObjectBufferATI(buffer);
}

GLvoid APIENTRY __GLVVT_FUNC(UpdateObjectBufferATI)(GLuint buffer, GLuint offset, GLsizei size, const GLvoid *pointer, GLenum preserve)
{
    __GL_GET_DISPATCH_TABLE->UpdateObjectBufferATI(buffer, offset, size, pointer, preserve);
}

GLvoid APIENTRY __GLVVT_FUNC(GetObjectBufferfvATI)(GLuint buffer, GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->GetObjectBufferfvATI(buffer, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetObjectBufferivATI)(GLuint buffer, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE->GetObjectBufferivATI(buffer, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(FreeObjectBufferATI)(GLuint buffer)
{
    __GL_GET_DISPATCH_TABLE->FreeObjectBufferATI(buffer);
}

GLvoid APIENTRY __GLVVT_FUNC(ArrayObjectATI)(GLenum array, GLint size, GLenum type, GLsizei stride, GLuint buffer, GLuint offset)
{
    __GL_GET_DISPATCH_TABLE->ArrayObjectATI(array, size, type, stride, buffer, offset);
}

GLvoid APIENTRY __GLVVT_FUNC(GetArrayObjectfvATI)(GLenum array, GLenum pname, GLfloat * params)
{
    __GL_GET_DISPATCH_TABLE->GetArrayObjectfvATI(array, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetArrayObjectivATI)(GLenum array, GLenum pname, GLint * params)
{
    __GL_GET_DISPATCH_TABLE->GetArrayObjectivATI(array, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(VariantArrayObjectATI)(GLuint id, GLenum type, GLsizei stride, GLuint buffer, GLuint offset)
{
    __GL_GET_DISPATCH_TABLE->VariantArrayObjectATI(id, type, stride, buffer, offset);
}

GLvoid APIENTRY __GLVVT_FUNC(GetVariantArrayObjectfvATI)(GLuint id, GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->GetVariantArrayObjectfvATI(id, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetVariantArrayObjectivATI)(GLuint id, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE->GetVariantArrayObjectivATI(id, pname, params);
}

#endif

#if GL_ATI_vertex_attrib_array_object

GLvoid APIENTRY __GLVVT_FUNC(VertexAttribArrayObjectATI)(GLuint index, GLint size, GLenum type,
    GLboolean normalized, GLsizei stride, GLuint buffer, GLuint offset)
{
    __GL_GET_DISPATCH_TABLE->VertexAttribArrayObjectATI(index, size, type, normalized, stride, buffer, offset);
}

GLvoid APIENTRY __GLVVT_FUNC(GetVertexAttribArrayObjectfvATI)(GLuint index, GLenum pname, GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->GetVertexAttribArrayObjectfvATI(index, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetVertexAttribArrayObjectivATI)(GLuint index, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE->GetVertexAttribArrayObjectivATI(index, pname, params);
}

#endif

#if GL_ATI_element_array

GLvoid APIENTRY __GLVVT_FUNC(ElementPointerATI)(GLenum type, const GLvoid *pointer)
{
    __GL_GET_DISPATCH_TABLE->ElementPointerATI(type, pointer);
}

GLvoid APIENTRY __GLVVT_FUNC(DrawElementArrayATI)(GLenum mode, GLsizei count)
{
    __GL_GET_DISPATCH_TABLE->DrawElementArrayATI(mode, count);
}

GLvoid APIENTRY __GLVVT_FUNC(DrawRangeElementArrayATI)(GLenum mode, GLuint start, GLuint end, GLsizei count)
{
    __GL_GET_DISPATCH_TABLE->DrawRangeElementArrayATI(mode, start, end, count);
}

#endif

#if GL_EXT_stencil_two_side

GLvoid APIENTRY __GLVVT_FUNC(ActiveStencilFaceEXT)(GLenum face)
{
    __GL_GET_DISPATCH_TABLE->ActiveStencilFaceEXT(face);
}

#endif

GLvoid APIENTRY __GLVVT_FUNC(AddSwapHintRectWIN)(GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_GET_DISPATCH_TABLE->AddSwapHintRectWIN(x, y, width, height);
}

#if GL_EXT_depth_bounds_test

GLvoid APIENTRY __GLVVT_FUNC(DepthBoundsEXT)(GLclampd zMin, GLclampd zMax)
{
    __GL_GET_DISPATCH_TABLE->DepthBoundsEXT(zMin, zMax);
}

#endif

#if GL_EXT_framebuffer_object

GLboolean APIENTRY __GLVVT_FUNC(IsRenderbufferEXT)(GLuint renderbuffer)
{
    return __GL_GET_DISPATCH_TABLE->IsRenderbufferEXT(renderbuffer);
}

GLvoid APIENTRY __GLVVT_FUNC(BindRenderbufferEXT)(GLenum target, GLuint renderbuffer)
{
    __GL_GET_DISPATCH_TABLE->BindRenderbufferEXT(target, renderbuffer);
}

GLvoid APIENTRY __GLVVT_FUNC(DeleteRenderbuffersEXT)(GLsizei n, const GLuint *renderbuffers)
{
    __GL_GET_DISPATCH_TABLE->DeleteRenderbuffersEXT(n, renderbuffers);
}

GLvoid APIENTRY __GLVVT_FUNC(GenRenderbuffersEXT)(GLsizei n, GLuint *renderbuffers)
{
    __GL_GET_DISPATCH_TABLE->GenRenderbuffersEXT(n, renderbuffers);
}

GLvoid APIENTRY __GLVVT_FUNC(RenderbufferStorageEXT)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    __GL_GET_DISPATCH_TABLE->RenderbufferStorageEXT(target, internalformat, width, height);
}

GLvoid APIENTRY __GLVVT_FUNC(GetRenderbufferParameterivEXT)(GLenum target, GLenum pname, GLint* params)
{
    __GL_GET_DISPATCH_TABLE->GetRenderbufferParameterivEXT(target, pname, params);
}

GLboolean APIENTRY __GLVVT_FUNC(IsFramebufferEXT)(GLuint framebuffer)
{
    return __GL_GET_DISPATCH_TABLE->IsFramebufferEXT(framebuffer);
}

GLvoid APIENTRY __GLVVT_FUNC(BindFramebufferEXT)(GLenum target, GLuint framebuffer)
{
    __GL_GET_DISPATCH_TABLE->BindFramebufferEXT(target,framebuffer);
}

GLvoid APIENTRY __GLVVT_FUNC(DeleteFramebuffersEXT)(GLsizei n, const GLuint *framebuffers)
{
    __GL_GET_DISPATCH_TABLE->DeleteFramebuffersEXT(n, framebuffers);
}

GLvoid APIENTRY __GLVVT_FUNC(GenFramebuffersEXT)(GLsizei n, GLuint *framebuffers)
{
    __GL_GET_DISPATCH_TABLE->GenFramebuffersEXT(n, framebuffers);
}

GLenum APIENTRY __GLVVT_FUNC(CheckFramebufferStatusEXT)(GLenum target)
{
    return __GL_GET_DISPATCH_TABLE->CheckFramebufferStatusEXT(target);
}

GLvoid APIENTRY __GLVVT_FUNC(FramebufferTexture1DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    __GL_GET_DISPATCH_TABLE->FramebufferTexture1DEXT(target, attachment, textarget, texture, level);
}

GLvoid APIENTRY __GLVVT_FUNC(FramebufferTexture2DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    __GL_GET_DISPATCH_TABLE->FramebufferTexture2DEXT(target, attachment, textarget, texture, level);
}

GLvoid APIENTRY __GLVVT_FUNC(FramebufferTexture3DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
    __GL_GET_DISPATCH_TABLE->FramebufferTexture3DEXT(target, attachment, textarget, texture, level, zoffset);
}

GLvoid APIENTRY __GLVVT_FUNC(FramebufferRenderbufferEXT)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    __GL_GET_DISPATCH_TABLE->FramebufferRenderbufferEXT(target, attachment, renderbuffertarget, renderbuffer);
}

GLvoid APIENTRY __GLVVT_FUNC(GetFramebufferAttachmentParameterivEXT)(GLenum target, GLenum attachment, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE->GetFramebufferAttachmentParameterivEXT(target, attachment, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GenerateMipmapEXT)(GLenum target)
{
    __GL_GET_DISPATCH_TABLE->GenerateMipmapEXT(target);
}

#if GL_EXT_framebuffer_blit

GLvoid APIENTRY __GLVVT_FUNC(BlitFramebufferEXT)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
    GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
    __GL_GET_DISPATCH_TABLE->BlitFramebufferEXT(srcX0, srcY0, srcX1, srcY1,
                                                 dstX0, dstY0, dstX1, dstY1, mask, filter);
}

#if GL_EXT_framebuffer_multisample

GLvoid APIENTRY __GLVVT_FUNC(RenderbufferStorageMultisampleEXT)(GLenum target, GLsizei samples,
    GLenum internalformat, GLsizei width, GLsizei height)
{
    __GL_GET_DISPATCH_TABLE->RenderbufferStorageMultisampleEXT(target, samples, internalformat, width, height);
}

#endif /* GL_EXT_framebuffer_multisample */

#endif /* GL_EXT_framebuffer_blit */

#endif /* GL_EXT_framebuffer_object */

#if GL_NV_occlusion_query

GLvoid APIENTRY __GLVVT_FUNC(BeginQueryNV)(GLuint id)
{
    __GL_GET_DISPATCH_TABLE->BeginQueryNV(id);
}

GLvoid APIENTRY __GLVVT_FUNC(EndQueryNV)(GLvoid)
{
    __GL_GET_DISPATCH_TABLE->EndQueryNV();
}

#endif

#if GL_EXT_bindable_uniform

GLvoid APIENTRY __GLVVT_FUNC(UniformBufferEXT)(GLuint program, GLint location, GLuint buffer)
{
    __GL_GET_DISPATCH_TABLE->UniformBufferEXT(program, location, buffer);
}

GLint APIENTRY __GLVVT_FUNC(GetUniformBufferSizeEXT)(GLuint program, GLint location)
{
    return __GL_GET_DISPATCH_TABLE->GetUniformBufferSizeEXT(program, location);
}

GLintptr APIENTRY __GLVVT_FUNC(GetUniformOffsetEXT)(GLuint program, GLint location)
{
    return __GL_GET_DISPATCH_TABLE->GetUniformOffsetEXT(program, location);
}

#endif

#if GL_EXT_texture_integer

GLvoid APIENTRY __GLVVT_FUNC(ClearColorIiEXT)(GLint r, GLint g, GLint b, GLint a)
{
    __GL_GET_DISPATCH_TABLE->ClearColorIiEXT(r,g,b,a);
}

GLvoid APIENTRY __GLVVT_FUNC(ClearColorIuiEXT)(GLuint r, GLuint g, GLuint b, GLuint a)
{
    __GL_GET_DISPATCH_TABLE->ClearColorIuiEXT(r,g,b,a);
}

GLvoid APIENTRY __GLVVT_FUNC(TexParameterIivEXT)(GLenum target, GLenum pname, GLint * params)
{
    __GL_GET_DISPATCH_TABLE->TexParameterIivEXT(target,pname,params);
}

GLvoid APIENTRY __GLVVT_FUNC(TexParameterIuivEXT)(GLenum target, GLenum pname, GLuint * params)
{
    __GL_GET_DISPATCH_TABLE->TexParameterIuivEXT(target,pname,params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetTexParameterIivEXT)(GLenum target, GLenum pname, GLint * params)
{
    __GL_GET_DISPATCH_TABLE->GetTexParameterIivEXT(target,pname,params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetTexParameterIuivEXT)(GLenum target, GLenum pname, GLuint * params)
{
    __GL_GET_DISPATCH_TABLE->GetTexParameterIuivEXT(target, pname,params);
}

#endif

#if GL_EXT_gpu_shader4

GLvoid APIENTRY __GLVVT_FUNC(VertexAttribI1iEXT)(GLuint index, GLint x)
{
    __GL_GET_DISPATCH_TABLE->VertexAttribI1iEXT(index,x);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttribI2iEXT)(GLuint index, GLint x, GLint y)
{
    __GL_GET_DISPATCH_TABLE->VertexAttribI2iEXT(index, x, y);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttribI3iEXT)(GLuint index, GLint x, GLint y, GLint z)
{
    __GL_GET_DISPATCH_TABLE->VertexAttribI3iEXT(index, x, y, z);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttribI4iEXT)(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
    __GL_GET_DISPATCH_TABLE->VertexAttribI4iEXT(index, x, y, z, w);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttribI1uiEXT)(GLuint index, GLuint x)
{
    __GL_GET_DISPATCH_TABLE->VertexAttribI1uiEXT(index, x);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttribI2uiEXT)(GLuint index, GLuint x, GLuint y)
{
    __GL_GET_DISPATCH_TABLE->VertexAttribI2uiEXT(index, x, y);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttribI3uiEXT)(GLuint index, GLuint x, GLuint y, GLuint z)
{
    __GL_GET_DISPATCH_TABLE->VertexAttribI3uiEXT(index, x, y, z);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttribI4uiEXT)(GLuint index, GLuint x, GLuint y, GLuint z,
                            GLuint w)
{
    __GL_GET_DISPATCH_TABLE->VertexAttribI4uiEXT(index, x, y, z, w);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttribI1ivEXT)(GLuint index, const GLint *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttribI1ivEXT(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttribI2ivEXT)(GLuint index, const GLint *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttribI2ivEXT(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttribI3ivEXT)(GLuint index, const GLint *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttribI3ivEXT(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttribI4ivEXT)(GLuint index, const GLint *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttribI4ivEXT(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttribI1uivEXT)(GLuint index, const GLuint *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttribI1uivEXT(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttribI2uivEXT)(GLuint index, const GLuint *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttribI2uivEXT(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttribI3uivEXT)(GLuint index, const GLuint *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttribI3uivEXT(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttribI4uivEXT)(GLuint index, const GLuint *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttribI4uivEXT(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttribI4bvEXT)(GLuint index, const GLbyte *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttribI4bvEXT(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttribI4svEXT)(GLuint index, const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttribI4svEXT(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttribI4ubvEXT)(GLuint index, const GLubyte *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttribI4ubvEXT(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttribI4usvEXT)(GLuint index, const GLushort *v)
{
    __GL_GET_DISPATCH_TABLE->VertexAttribI4usvEXT(index, v);
}

GLvoid APIENTRY __GLVVT_FUNC(VertexAttribIPointerEXT)(GLuint index, GLint size, GLenum type,
                                GLsizei stride, const GLvoid *pointer)
{
    __GL_GET_DISPATCH_TABLE->VertexAttribIPointerEXT(index, size, type, stride, pointer);
}

GLvoid APIENTRY __GLVVT_FUNC(GetVertexAttribIivEXT)(GLuint index, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE->GetVertexAttribIivEXT(index, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetVertexAttribIuivEXT)(GLuint index, GLenum pname, GLuint *params)
{
    __GL_GET_DISPATCH_TABLE->GetVertexAttribIuivEXT(index, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(Uniform1uiEXT)(GLint location, GLuint v0)
{
    __GL_GET_DISPATCH_TABLE->Uniform1uiEXT(location, v0);
}

GLvoid APIENTRY __GLVVT_FUNC(Uniform2uiEXT)(GLint location, GLuint v0, GLuint v1)
{
    __GL_GET_DISPATCH_TABLE->Uniform2uiEXT(location, v0, v1);
}

GLvoid APIENTRY __GLVVT_FUNC(Uniform3uiEXT)(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    __GL_GET_DISPATCH_TABLE->Uniform3uiEXT(location, v0, v1, v2);
}

GLvoid APIENTRY __GLVVT_FUNC(Uniform4uiEXT)(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    __GL_GET_DISPATCH_TABLE->Uniform4uiEXT(location, v0, v1, v2, v3);
}

GLvoid APIENTRY __GLVVT_FUNC(Uniform1uivEXT)(GLint location, GLsizei count, const GLuint *value)
{
    __GL_GET_DISPATCH_TABLE->Uniform1uivEXT(location, count, value);
}

GLvoid APIENTRY __GLVVT_FUNC(Uniform2uivEXT)(GLint location, GLsizei count, const GLuint *value)
{
    __GL_GET_DISPATCH_TABLE->Uniform2uivEXT(location, count, value);
}

GLvoid APIENTRY __GLVVT_FUNC(Uniform3uivEXT)(GLint location, GLsizei count, const GLuint *value)
{
    __GL_GET_DISPATCH_TABLE->Uniform3uivEXT(location, count, value);
}

GLvoid APIENTRY __GLVVT_FUNC(Uniform4uivEXT)(GLint location, GLsizei count, const GLuint *value)
{
    __GL_GET_DISPATCH_TABLE->Uniform4uivEXT(location, count, value);
}

GLvoid APIENTRY __GLVVT_FUNC(GetUniformuivEXT)(GLuint program, GLint location, GLuint *params)
{
    __GL_GET_DISPATCH_TABLE->GetUniformuivEXT(program, location, params);
}

GLvoid APIENTRY __GLVVT_FUNC(BindFragDataLocationEXT)(GLuint program, GLuint colorNumber, const GLbyte *name)
{
    __GL_GET_DISPATCH_TABLE->BindFragDataLocationEXT(program, colorNumber, name);
}

GLint APIENTRY __GLVVT_FUNC(GetFragDataLocationEXT)(GLuint program, const GLbyte *name)
{
    return __GL_GET_DISPATCH_TABLE->GetFragDataLocationEXT(program, name);
}

#endif

#if GL_EXT_geometry_shader4
GLvoid APIENTRY __GLVVT_FUNC(ProgramParameteriEXT)(GLuint program, GLenum pname, GLint value)
{
    __GL_GET_DISPATCH_TABLE->ProgramParameteriEXT(program, pname, value);
}
GLvoid APIENTRY __GLVVT_FUNC(FramebufferTextureEXT)(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
    __GL_GET_DISPATCH_TABLE->FramebufferTextureEXT(target, attachment, texture, level);
}
GLvoid APIENTRY __GLVVT_FUNC(FramebufferTextureLayerEXT)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    __GL_GET_DISPATCH_TABLE->FramebufferTextureLayerEXT(target, attachment, texture, level, layer);
}
GLvoid APIENTRY __GLVVT_FUNC(FramebufferTextureFaceEXT)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face)
{
    __GL_GET_DISPATCH_TABLE->FramebufferTextureFaceEXT(target, attachment, texture, level, face);
}

#endif

#if GL_EXT_draw_buffers2
GLvoid APIENTRY __GLVVT_FUNC(ColorMaskIndexedEXT)(GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
    __GL_GET_DISPATCH_TABLE->ColorMaskIndexedEXT(buf, r, g, b, a);
}

GLvoid APIENTRY __GLVVT_FUNC(GetBooleanIndexedvEXT)(GLenum value, GLuint index, GLboolean *data)
{
    __GL_GET_DISPATCH_TABLE->GetBooleanIndexedvEXT(value, index, data);
}

GLvoid APIENTRY __GLVVT_FUNC(GetIntegerIndexedvEXT)(GLenum value, GLuint index, GLint *data)
{
    __GL_GET_DISPATCH_TABLE->GetIntegerIndexedvEXT(value, index, data);
}

GLvoid APIENTRY __GLVVT_FUNC(EnableIndexedEXT)(GLenum target, GLuint index)
{
    __GL_GET_DISPATCH_TABLE->EnableIndexedEXT(target, index);
}


GLvoid APIENTRY __GLVVT_FUNC(DisableIndexedEXT)(GLenum target, GLuint index)
{
    __GL_GET_DISPATCH_TABLE->DisableIndexedEXT(target, index);
}

GLboolean APIENTRY __GLVVT_FUNC(IsEnabledIndexedEXT)(GLenum target, GLuint index)
{
    return __GL_GET_DISPATCH_TABLE->IsEnabledIndexedEXT(target, index);
}
#endif

#if GL_EXT_texture_buffer_object
GLvoid APIENTRY __GLVVT_FUNC(TexBufferEXT)(GLenum target, GLenum internalformat, GLuint buffer)
{
    __GL_GET_DISPATCH_TABLE->TexBufferEXT(target, internalformat, buffer);
}
#endif

#if GL_EXT_gpu_program_parameters
GLvoid APIENTRY __GLVVT_FUNC(ProgramEnvParameters4fvEXT)(GLenum target, GLuint index, GLsizei count, const GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->ProgramEnvParameters4fvEXT(target, index, count, params);
}
GLvoid APIENTRY __GLVVT_FUNC(ProgramLocalParameters4fvEXT)(GLenum target, GLuint index, GLsizei count, const GLfloat *params)
{
    __GL_GET_DISPATCH_TABLE->ProgramLocalParameters4fvEXT(target, index, count, params);
}
#endif

#if GL_EXT_draw_instanced
GLvoid APIENTRY __GLVVT_FUNC(DrawArraysInstancedEXT)(GLenum mode, GLint first, GLsizei count, GLsizei primCount)
{
    __GL_GET_DISPATCH_TABLE->DrawArraysInstancedEXT(mode, first, count, primCount);
}

GLvoid APIENTRY __GLVVT_FUNC(DrawElementsInstancedEXT)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primCount)
{
    __GL_GET_DISPATCH_TABLE->DrawElementsInstancedEXT(mode, count, type, indices, primCount);
}
#endif

#if GL_ARB_color_buffer_float
GLvoid APIENTRY __GLVVT_FUNC(ClampColorARB)(GLenum target, GLenum clamp)
{
    __GL_GET_DISPATCH_TABLE->ClampColorARB(target, clamp);
}
#endif

#if GL_EXT_timer_query
GLvoid APIENTRY __GLVVT_FUNC(GetQueryObjecti64vEXT)(GLuint id, GLenum pname, GLint64EXT *params)
{
    __GL_GET_DISPATCH_TABLE->GetQueryObjecti64vEXT(id,pname,params);
}

GLvoid APIENTRY __GLVVT_FUNC(GetQueryObjectui64vEXT)(GLuint id, GLenum pname, GLuint64EXT * params)
{
    __GL_GET_DISPATCH_TABLE->GetQueryObjectui64vEXT(id,pname,params);
}
#endif

#if GL_ATI_separate_stencil
GLvoid APIENTRY __GLVVT_FUNC(StencilFuncSeparateATI)(GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask)
{
    __GL_GET_DISPATCH_TABLE->StencilFuncSeparateATI(frontfunc, backfunc, ref, mask);
}
#endif

GLboolean APIENTRY __GLVVT_FUNC(IsRenderbuffer)(GLuint renderbuffer)
{
    return __GL_GET_DISPATCH_TABLE->IsRenderbufferEXT(renderbuffer);
}

GLvoid APIENTRY __GLVVT_FUNC(BindRenderbuffer)(GLenum target, GLuint renderbuffer)
{
    __GL_GET_DISPATCH_TABLE->BindRenderbufferEXT(target, renderbuffer);
}

GLvoid APIENTRY __GLVVT_FUNC(DeleteRenderbuffers)(GLsizei n, const GLuint *renderbuffers)
{
    __GL_GET_DISPATCH_TABLE->DeleteRenderbuffersEXT(n, renderbuffers);
}

GLvoid APIENTRY __GLVVT_FUNC(GenRenderbuffers)(GLsizei n, GLuint *renderbuffers)
{
    __GL_GET_DISPATCH_TABLE->GenRenderbuffersEXT(n, renderbuffers);
}

GLvoid APIENTRY __GLVVT_FUNC(RenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    __GL_GET_DISPATCH_TABLE->RenderbufferStorageEXT(target, internalformat, width, height);
}

GLvoid APIENTRY __GLVVT_FUNC(GetRenderbufferParameteriv)(GLenum target, GLenum pname, GLint* params)
{
    __GL_GET_DISPATCH_TABLE->GetRenderbufferParameterivEXT(target, pname, params);
}

GLboolean APIENTRY __GLVVT_FUNC(IsFramebuffer)(GLuint framebuffer)
{
    return __GL_GET_DISPATCH_TABLE->IsFramebufferEXT(framebuffer);
}

GLvoid APIENTRY __GLVVT_FUNC(BindFramebuffer)(GLenum target, GLuint framebuffer)
{
    __GL_GET_DISPATCH_TABLE->BindFramebufferEXT(target,framebuffer);
}

GLvoid APIENTRY __GLVVT_FUNC(DeleteFramebuffers)(GLsizei n, const GLuint *framebuffers)
{
    __GL_GET_DISPATCH_TABLE->DeleteFramebuffersEXT(n, framebuffers);
}

GLvoid APIENTRY __GLVVT_FUNC(GenFramebuffers)(GLsizei n, GLuint *framebuffers)
{
    __GL_GET_DISPATCH_TABLE->GenFramebuffersEXT(n, framebuffers);
}

GLenum APIENTRY __GLVVT_FUNC(CheckFramebufferStatus)(GLenum target)
{
    return __GL_GET_DISPATCH_TABLE->CheckFramebufferStatusEXT(target);
}

GLvoid APIENTRY __GLVVT_FUNC(FramebufferTexture1D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    __GL_GET_DISPATCH_TABLE->FramebufferTexture1DEXT(target, attachment, textarget, texture, level);
}

GLvoid APIENTRY __GLVVT_FUNC(FramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    __GL_GET_DISPATCH_TABLE->FramebufferTexture2DEXT(target, attachment, textarget, texture, level);
}

GLvoid APIENTRY __GLVVT_FUNC(FramebufferTexture3D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
    __GL_GET_DISPATCH_TABLE->FramebufferTexture3DEXT(target, attachment, textarget, texture, level, zoffset);
}

GLvoid APIENTRY __GLVVT_FUNC(FramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    __GL_GET_DISPATCH_TABLE->FramebufferRenderbufferEXT(target, attachment, renderbuffertarget, renderbuffer);
}

GLvoid APIENTRY __GLVVT_FUNC(GetFramebufferAttachmentParameteriv)(GLenum target, GLenum attachment, GLenum pname, GLint *params)
{
    __GL_GET_DISPATCH_TABLE->GetFramebufferAttachmentParameterivEXT(target, attachment, pname, params);
}

GLvoid APIENTRY __GLVVT_FUNC(GenerateMipmap)(GLenum target)
{
    __GL_GET_DISPATCH_TABLE->GenerateMipmapEXT(target);
}

GLvoid APIENTRY __GLVVT_FUNC(BlitFramebuffer)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
    GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
    __GL_GET_DISPATCH_TABLE->BlitFramebufferEXT(srcX0, srcY0, srcX1, srcY1,
                                                 dstX0, dstY0, dstX1, dstY1, mask, filter);
}

GLvoid APIENTRY __GLVVT_FUNC(RenderbufferStorageMultisample)(GLenum target, GLsizei samples,
    GLenum internalformat, GLsizei width, GLsizei height)
{
    __GL_GET_DISPATCH_TABLE->RenderbufferStorageMultisampleEXT(target, samples, internalformat, width, height);
}

#if GL_ARB_multitexture
GLvoid APIENTRY __GLVVT_FUNC(ActiveTextureARB)(GLenum texture)
{
    __GL_GET_DISPATCH_TABLE->ActiveTexture(texture);
}
GLvoid APIENTRY __GLVVT_FUNC(ClientActiveTextureARB)(GLenum texture)
{
    __GL_GET_DISPATCH_TABLE->ClientActiveTexture(texture);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord1dARB)(GLenum target, GLdouble s)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord1d(target, s);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord1dvARB)(GLenum target, const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord1dv(target, v);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord1fARB)(GLenum target, GLfloat s)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord1f(target, s);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord1fvARB)(GLenum target, const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord1fv(target, v);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord1iARB)(GLenum target, GLint s)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord1i(target, s);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord1ivARB)(GLenum target, const GLint *v)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord1iv(target, v);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord1sARB)(GLenum target, GLshort s)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord1s(target, s);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord1svARB)(GLenum target, const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord1sv(target, v);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord2dARB)(GLenum target, GLdouble s, GLdouble t)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord2d(target, s, t);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord2dvARB)(GLenum target, const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord2dv(target, v);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord2fARB)(GLenum target, GLfloat s, GLfloat t)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord2f(target, s, t);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord2fvARB)(GLenum target, const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord2fv(target, v);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord2iARB)(GLenum target, GLint s, GLint t)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord2i(target, s, t);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord2ivARB)(GLenum target, const GLint *v)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord2iv(target, v);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord2sARB)(GLenum target, GLshort s, GLshort t)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord2s(target, s, t);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord2svARB)(GLenum target, const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord2sv(target, v);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord3dARB)(GLenum target, GLdouble s, GLdouble t, GLdouble r)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord3d(target, s, t, r);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord3dvARB)(GLenum target, const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord3dv(target, v);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord3fARB)(GLenum target, GLfloat s, GLfloat t, GLfloat r)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord3f(target, s, t, r);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord3fvARB)(GLenum target, const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord3fv(target, v);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord3iARB)(GLenum target, GLint s, GLint t, GLint r)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord3i(target, s, t, r);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord3ivARB)(GLenum target, const GLint *v)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord3iv(target, v);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord3sARB)(GLenum target, GLshort s, GLshort t, GLshort r)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord3s(target, s, t, r);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord3svARB)(GLenum target, const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord3sv(target, v);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord4dARB)(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord4d(target, s, t, r, q);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord4dvARB)(GLenum target, const GLdouble *v)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord4dv(target, v);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord4fARB)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord4f(target, s, t, r, q);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord4fvARB)(GLenum target, const GLfloat *v)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord4fv(target, v);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord4iARB)(GLenum target, GLint s, GLint t, GLint r, GLint q)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord4i(target, s, t, r , q);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord4ivARB)(GLenum target, const GLint *v)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord4iv(target, v);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord4sARB)(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord4s(target, s, t, r, q);
}
GLvoid APIENTRY __GLVVT_FUNC(MultiTexCoord4svARB)(GLenum target, const GLshort *v)
{
    __GL_GET_DISPATCH_TABLE->MultiTexCoord4sv(target, v);
}
#endif

#if GL_ARB_texture_compression

GLvoid APIENTRY __GLVVT_FUNC(GetCompressedTexImageARB)(GLenum texture, GLint x, GLvoid * buf)
{
    __GL_GET_DISPATCH_TABLE->GetCompressedTexImage(texture, x, buf);
}

#endif

