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


#include "gc_gl_context.h"
#include "gc_gl_dlist.h"
#include "g_lcomp.h"
#include "g_imfncs.h"
#include "g_lcfncs.h"
#include "g_lefncs.h"

#if defined(_WIN32)
#pragma warning(disable: 4244)
#endif


/*
*** Functions in eval.c
*/

extern GLvoid __glFillMap1fInternal(GLint k, GLint order, GLint stride,
                const GLfloat *points, GLfloat *data);
extern GLvoid __glFillMap1dInternal(GLint k, GLint order, GLint stride,
                const GLdouble *points, GLfloat *data);
extern GLvoid __glFillMap2fInternal(GLint k, GLint majorOrder, GLint minorOrder,
                GLint majorStride, GLint minorStride,
                const GLfloat *points, GLfloat *data);
extern GLvoid __glFillMap2dInternal(GLint k, GLint majorOrder, GLint minorOrder,
                GLint majorStride, GLint minorStride,
                const GLdouble *points, GLfloat *data);
extern GLboolean __glTexImagCopyInfo(__GLcontext *gc, GLenum format, GLenum type,
                GLenum *ret_format, GLenum *ret_type);
extern GLint __glImageSize(GLsizei width, GLsizei height, GLenum format, GLenum type);
extern GLint __glImageSize3D(GLsizei width, GLsizei height, GLsizei depth,
                GLenum format, GLenum type);
extern GLvoid __glFillImage(__GLcontext *gc, GLsizei width, GLsizei height,
                GLenum format, GLenum type, const GLvoid *userdata, GLubyte *newimage);
extern GLvoid __glFillImage3D(__GLcontext *gc, GLsizei width, GLsizei height, GLsizei depth,
                GLenum format, GLenum type, const GLvoid *userdata, GLubyte *newimage);


/* OpenGL compiled display list APIs */

/************************************************************************/

GLvoid __gllc_InvalidValue(__GLcontext *gc)
{
    __GLdlistOp *dlop;

    dlop = __glDlistAllocOp(gc, 0);
    if (dlop == NULL) return;
    dlop->opcode = __glop_InvalidValue;
    __glDlistAppendOp(gc, dlop);
}

GLvoid __gllc_InvalidEnum(__GLcontext *gc)
{
    __GLdlistOp *dlop;

    dlop = __glDlistAllocOp(gc, 0);
    if (dlop == NULL) return;
    dlop->opcode = __glop_InvalidEnum;
    __glDlistAppendOp(gc, dlop);
}

GLvoid __gllc_InvalidOperation(__GLcontext *gc)
{
    __GLdlistOp *dlop;

    dlop = __glDlistAllocOp(gc, 0);
    if (dlop == NULL) return;
    dlop->opcode = __glop_InvalidOperation;
    __glDlistAppendOp(gc, dlop);
}

GLvoid __gllc_TableTooLarge(__GLcontext *gc)
{
    __GLdlistOp *dlop;
    dlop = __glDlistAllocOp(gc, 0);
    if (dlop == NULL) return;
    dlop->opcode = __glop_TableTooLarge;
    __glDlistAppendOp(gc, dlop);
}

GLvoid __gllc_Error(__GLcontext *gc, GLenum error)
{
    switch(error) {
      case GL_INVALID_VALUE:
        __gllc_InvalidValue(gc);
        break;
      case GL_INVALID_ENUM:
        __gllc_InvalidEnum(gc);
        break;
      case GL_INVALID_OPERATION:
        __gllc_InvalidOperation(gc);
        break;
      case GL_TABLE_TOO_LARGE:
        __gllc_TableTooLarge(gc);
        break;
    }
}

/************************************************************************/

GLvoid APIENTRY __gllc_Begin(GLenum mode)
{
    __GLdlistOp *dlop;
    struct __gllc_Begin_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Begin(mode);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Begin_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Begin;
    data = (struct __gllc_Begin_Rec *)(dlop + 1);
    data->primType = mode;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_End(GLvoid)
{
    __GLdlistOp *dlop;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
         if(gc->currentImmediateTable->dispatch.End == __glim_End_Material)
            __glim_End_Material();
        else
            __glim_End();
    }

    dlop = __glDlistAllocOp(gc, 0);
    if (dlop == NULL) return;
    dlop->opcode = __glop_End;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ListBase(GLuint base)
{
    __GLdlistOp *dlop;
    struct __gllc_ListBase_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ListBase(base);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ListBase_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ListBase;
    data = (struct __gllc_ListBase_Rec *)(dlop + 1);
    data->base = base;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_EdgeFlagv(const GLboolean *flag)
{
    __GLdlistOp *dlop;
    struct __gllc_EdgeFlag_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.EdgeFlagv)(flag);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_EdgeFlag_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_EdgeFlag;
    data = (struct __gllc_EdgeFlag_Rec *)(dlop + 1);
    data->flag = flag[0];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_EdgeFlag(GLboolean flag)
{
    __GLdlistOp *dlop;
    struct __gllc_EdgeFlag_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.EdgeFlag)(flag);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_EdgeFlag_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_EdgeFlag;
    data = (struct __gllc_EdgeFlag_Rec *)(dlop + 1);
    data->flag = flag;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Indexd(GLdouble c)
{
    __GLdlistOp *dlop;
    struct __gllc_Indexf_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Indexd(c);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Indexf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Indexf;
    data = (struct __gllc_Indexf_Rec *)(dlop + 1);
    data->c = c;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Indexdv(const GLdouble *c)
{
    __GLdlistOp *dlop;
    struct __gllc_Indexf_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Indexdv(c);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Indexf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Indexf;
    data = (struct __gllc_Indexf_Rec *)(dlop + 1);
    data->c = c[0];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Indexf(GLfloat c)
{
    __GLdlistOp *dlop;
    struct __gllc_Indexf_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Indexf(c);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Indexf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Indexf;
    data = (struct __gllc_Indexf_Rec *)(dlop + 1);
    data->c = c;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Indexfv(const GLfloat *c)
{
    __GLdlistOp *dlop;
    struct __gllc_Indexf_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Indexfv(c);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Indexf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Indexf;
    data = (struct __gllc_Indexf_Rec *)(dlop + 1);
    data->c = c[0];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Indexi(GLint c)
{
    __GLdlistOp *dlop;
    struct __gllc_Indexf_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Indexi(c);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Indexf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Indexf;
    data = (struct __gllc_Indexf_Rec *)(dlop + 1);
    data->c = c;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Indexiv(const GLint *c)
{
    __GLdlistOp *dlop;
    struct __gllc_Indexf_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Indexiv(c);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Indexf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Indexf;
    data = (struct __gllc_Indexf_Rec *)(dlop + 1);
    data->c = c[0];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Indexs(GLshort c)
{
    __GLdlistOp *dlop;
    struct __gllc_Indexf_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Indexs(c);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Indexf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Indexf;
    data = (struct __gllc_Indexf_Rec *)(dlop + 1);
    data->c = c;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Indexsv(const GLshort *c)
{
    __GLdlistOp *dlop;
    struct __gllc_Indexf_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Indexsv(c);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Indexf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Indexf;
    data = (struct __gllc_Indexf_Rec *)(dlop + 1);
    data->c = c[0];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Indexub(GLubyte c)
{
    __GLdlistOp *dlop;
    struct __gllc_Indexf_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Indexub(c);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Indexf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Indexf;
    data = (struct __gllc_Indexf_Rec *)(dlop + 1);
    data->c = c;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Indexubv(const GLubyte *c)
{
    __GLdlistOp *dlop;
    struct __gllc_Indexf_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Indexubv(c);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Indexf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Indexf;
    data = (struct __gllc_Indexf_Rec *)(dlop + 1);
    data->c = c[0];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos2d(GLdouble x, GLdouble y)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos2d(x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos2fv;
    data = (struct __gllc_RasterPos2fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos2dv(const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos2dv(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos2fv;
    data = (struct __gllc_RasterPos2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos2f(GLfloat x, GLfloat y)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos2f(x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos2fv;
    data = (struct __gllc_RasterPos2fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos2fv(const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos2fv(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos2fv;
    data = (struct __gllc_RasterPos2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos2i(GLint x, GLint y)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos2i(x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos2fv;
    data = (struct __gllc_RasterPos2fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos2iv(const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos2iv(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos2fv;
    data = (struct __gllc_RasterPos2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos2s(GLshort x, GLshort y)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos2s(x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos2fv;
    data = (struct __gllc_RasterPos2fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos2sv(const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos2sv(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos2fv;
    data = (struct __gllc_RasterPos2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos3d(GLdouble x, GLdouble y, GLdouble z)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos3d(x, y, z);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos3fv;
    data = (struct __gllc_RasterPos3fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = z;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos3dv(const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos3dv(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos3fv;
    data = (struct __gllc_RasterPos3fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos3f(GLfloat x, GLfloat y, GLfloat z)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos3f(x, y, z);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos3fv;
    data = (struct __gllc_RasterPos3fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = z;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos3fv(const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos3fv(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos3fv;
    data = (struct __gllc_RasterPos3fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos3i(GLint x, GLint y, GLint z)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos3i(x, y, z);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos3fv;
    data = (struct __gllc_RasterPos3fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = z;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos3iv(const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos3iv(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos3fv;
    data = (struct __gllc_RasterPos3fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos3s(GLshort x, GLshort y, GLshort z)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos3s(x, y, z);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos3fv;
    data = (struct __gllc_RasterPos3fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = z;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos3sv(const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos3sv(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos3fv;
    data = (struct __gllc_RasterPos3fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos4d(x, y, z, w);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos4fv;
    data = (struct __gllc_RasterPos4fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = z;
    data->v[3] = w;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos4dv(const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos4dv(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos4fv;
    data = (struct __gllc_RasterPos4fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos4f(x, y, z, w);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos4fv;
    data = (struct __gllc_RasterPos4fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = z;
    data->v[3] = w;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos4fv(const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos4fv(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos4fv;
    data = (struct __gllc_RasterPos4fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos4i(GLint x, GLint y, GLint z, GLint w)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos4i(x, y, z, w);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos4fv;
    data = (struct __gllc_RasterPos4fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = z;
    data->v[3] = w;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos4iv(const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos4iv(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos4fv;
    data = (struct __gllc_RasterPos4fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos4s(x, y, z, w);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos4fv;
    data = (struct __gllc_RasterPos4fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = z;
    data->v[3] = w;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos4sv(const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos4sv(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos4fv;
    data = (struct __gllc_RasterPos4fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Rectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
    __GLdlistOp *dlop;
    struct __gllc_Rectf_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Rectd(x1, y1, x2, y2);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Rectf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Rectf;
    data = (struct __gllc_Rectf_Rec *)(dlop + 1);
    data->x1 = x1;
    data->y1 = y1;
    data->x2 = x2;
    data->y2 = y2;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Rectdv(const GLdouble *v1, const GLdouble *v2)
{
    __GLdlistOp *dlop;
    struct __gllc_Rectf_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Rectdv(v1, v2);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Rectf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Rectf;
    data = (struct __gllc_Rectf_Rec *)(dlop + 1);
    data->x1 = v1[0];
    data->y1 = v1[1];
    data->x2 = v2[0];
    data->y2 = v2[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Rectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
    __GLdlistOp *dlop;
    struct __gllc_Rectf_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Rectf(x1, y1, x2, y2);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Rectf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Rectf;
    data = (struct __gllc_Rectf_Rec *)(dlop + 1);
    data->x1 = x1;
    data->y1 = y1;
    data->x2 = x2;
    data->y2 = y2;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Rectfv(const GLfloat *v1, const GLfloat *v2)
{
    __GLdlistOp *dlop;
    struct __gllc_Rectf_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Rectfv(v1, v2);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Rectf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Rectf;
    data = (struct __gllc_Rectf_Rec *)(dlop + 1);
    data->x1 = v1[0];
    data->y1 = v1[1];
    data->x2 = v2[0];
    data->y2 = v2[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Recti(GLint x1, GLint y1, GLint x2, GLint y2)
{
    __GLdlistOp *dlop;
    struct __gllc_Rectf_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Recti(x1, y1, x2, y2);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Rectf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Rectf;
    data = (struct __gllc_Rectf_Rec *)(dlop + 1);
    data->x1 = x1;
    data->y1 = y1;
    data->x2 = x2;
    data->y2 = y2;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Rectiv(const GLint *v1, const GLint *v2)
{
    __GLdlistOp *dlop;
    struct __gllc_Rectf_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Rectiv(v1, v2);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Rectf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Rectf;
    data = (struct __gllc_Rectf_Rec *)(dlop + 1);
    data->x1 = v1[0];
    data->y1 = v1[1];
    data->x2 = v2[0];
    data->y2 = v2[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Rects(GLshort x1, GLshort y1, GLshort x2, GLshort y2)
{
    __GLdlistOp *dlop;
    struct __gllc_Rectf_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Rects(x1, y1, x2, y2);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Rectf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Rectf;
    data = (struct __gllc_Rectf_Rec *)(dlop + 1);
    data->x1 = x1;
    data->y1 = y1;
    data->x2 = x2;
    data->y2 = y2;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Rectsv(const GLshort *v1, const GLshort *v2)
{
    __GLdlistOp *dlop;
    struct __gllc_Rectf_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Rectsv(v1, v2);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Rectf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Rectf;
    data = (struct __gllc_Rectf_Rec *)(dlop + 1);
    data->x1 = v1[0];
    data->y1 = v1[1];
    data->x2 = v2[0];
    data->y2 = v2[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ClipPlane(GLenum plane, const GLdouble *equation)
{
    __GLdlistOp *dlop;
    struct __gllc_ClipPlane_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ClipPlane(plane, equation);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ClipPlane_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ClipPlane;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_ClipPlane_Rec *)(dlop + 1);
    data->plane = plane;
    data->equation[0] = equation[0];
    data->equation[1] = equation[1];
    data->equation[2] = equation[2];
    data->equation[3] = equation[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ColorMaterial(GLenum face, GLenum mode)
{
    __GLdlistOp *dlop;
    struct __gllc_ColorMaterial_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ColorMaterial(face, mode);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ColorMaterial_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ColorMaterial;
    data = (struct __gllc_ColorMaterial_Rec *)(dlop + 1);
    data->face = face;
    data->mode = mode;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_CullFace(GLenum mode)
{
    __GLdlistOp *dlop;
    struct __gllc_CullFace_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_CullFace(mode);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_CullFace_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_CullFace;
    data = (struct __gllc_CullFace_Rec *)(dlop + 1);
    data->mode = mode;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Fogf(GLenum pname, GLfloat param)
{
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Fogf(pname, param);
    }

    if (__glFog_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_Fogfv(pname, &param);
}

GLvoid APIENTRY __gllc_Fogfv(GLenum pname, const GLfloat *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_Fogfv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Fogfv(pname, params);
    }

    arraySize = __GL64PAD(__glFog_size(pname) * 4);
    if (arraySize < 0) {
        __gllc_InvalidEnum(gc);
        return;
    }
    size = (GLuint)sizeof(struct __gllc_Fogfv_Rec) + arraySize;
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_Fogfv;
    data = (struct __gllc_Fogfv_Rec *)(dlop + 1);
    data->pname = pname;
    __GL_MEMCOPY((GLubyte *)(data + 1), params, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Fogi(GLenum pname, GLint param)
{
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Fogi(pname, param);
    }

    if (__glFog_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_Fogiv(pname, &param);
}

GLvoid APIENTRY __gllc_Fogiv(GLenum pname, const GLint *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_Fogiv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Fogiv(pname, params);
    }

    arraySize = __GL64PAD(__glFog_size(pname) * 4);
    if (arraySize < 0) {
        __gllc_InvalidEnum(gc);
        return;
    }
    size = (GLuint)sizeof(struct __gllc_Fogiv_Rec) + arraySize;
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_Fogiv;
    data = (struct __gllc_Fogiv_Rec *)(dlop + 1);
    data->pname = pname;
    __GL_MEMCOPY((GLubyte *)(data + 1), params, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_FrontFace(GLenum mode)
{
    __GLdlistOp *dlop;
    struct __gllc_FrontFace_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_FrontFace(mode);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_FrontFace_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_FrontFace;
    data = (struct __gllc_FrontFace_Rec *)(dlop + 1);
    data->mode = mode;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Hint(GLenum target, GLenum mode)
{
    __GLdlistOp *dlop;
    struct __gllc_Hint_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Hint(target, mode);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Hint_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Hint;
    data = (struct __gllc_Hint_Rec *)(dlop + 1);
    data->target = target;
    data->mode = mode;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Lightf(GLenum light, GLenum pname, GLfloat param)
{
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Lightf(light, pname, param);
    }

    if (__glLight_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_Lightfv(light, pname, &param);
}

GLvoid APIENTRY __gllc_Lightfv(GLenum light, GLenum pname, const GLfloat *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_Lightfv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Lightfv(light, pname, params);
    }

    arraySize = __GL64PAD(__glLight_size(pname) * 4);
    if (arraySize < 0) {
        __gllc_InvalidEnum(gc);
        return;
    }
    size = (GLuint)sizeof(struct __gllc_Lightfv_Rec) + arraySize;
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_Lightfv;
    data = (struct __gllc_Lightfv_Rec *)(dlop + 1);
    data->light = light;
    data->pname = pname;
    __GL_MEMCOPY((GLubyte *)(data + 1), params, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Lighti(GLenum light, GLenum pname, GLint param)
{
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Lighti(light, pname, param);
    }

    if (__glLight_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_Lightiv(light, pname, &param);
}

GLvoid APIENTRY __gllc_Lightiv(GLenum light, GLenum pname, const GLint *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_Lightiv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Lightiv(light, pname, params);
    }

    arraySize = __GL64PAD(__glLight_size(pname) * 4);
    if (arraySize < 0) {
        __gllc_InvalidEnum(gc);
        return;
    }
    size = (GLuint)sizeof(struct __gllc_Lightiv_Rec) + arraySize;
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_Lightiv;
    data = (struct __gllc_Lightiv_Rec *)(dlop + 1);
    data->light = light;
    data->pname = pname;
    __GL_MEMCOPY((GLubyte *)(data + 1), params, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_LightModelf(GLenum pname, GLfloat param)
{
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_LightModelf(pname, param);
    }

    if (__glLightModel_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_LightModelfv(pname, &param);
}

GLvoid APIENTRY __gllc_LightModelfv(GLenum pname, const GLfloat *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_LightModelfv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_LightModelfv(pname, params);
    }

    arraySize = __GL64PAD(__glLightModel_size(pname) * 4);
    if (arraySize < 0) {
        __gllc_InvalidEnum(gc);
        return;
    }
    size = (GLuint)sizeof(struct __gllc_LightModelfv_Rec) + arraySize;
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_LightModelfv;
    data = (struct __gllc_LightModelfv_Rec *)(dlop + 1);
    data->pname = pname;
    __GL_MEMCOPY((GLubyte *)(data + 1), params, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_LightModeli(GLenum pname, GLint param)
{
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_LightModeli(pname, param);
    }

    if (__glLightModel_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_LightModeliv(pname, &param);
}

GLvoid APIENTRY __gllc_LightModeliv(GLenum pname, const GLint *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_LightModeliv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_LightModeliv(pname, params);
    }

    arraySize = __GL64PAD(__glLightModel_size(pname) * 4);
    if (arraySize < 0) {
        __gllc_InvalidEnum(gc);
        return;
    }
    size = (GLuint)sizeof(struct __gllc_LightModeliv_Rec) + arraySize;
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_LightModeliv;
    data = (struct __gllc_LightModeliv_Rec *)(dlop + 1);
    data->pname = pname;
    __GL_MEMCOPY((GLubyte *)(data + 1), params, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_LineStipple(GLint factor, GLushort pattern)
{
    __GLdlistOp *dlop;
    struct __gllc_LineStipple_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_LineStipple(factor, pattern);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_LineStipple_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_LineStipple;
    data = (struct __gllc_LineStipple_Rec *)(dlop + 1);
    data->factor = factor;
    data->pattern = pattern;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_LineWidth(GLfloat width)
{
    __GLdlistOp *dlop;
    struct __gllc_LineWidth_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_LineWidth(width);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_LineWidth_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_LineWidth;
    data = (struct __gllc_LineWidth_Rec *)(dlop + 1);
    data->width = width;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Materialf(GLenum face, GLenum pname, GLfloat param)
{
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Materialf(face, pname, param);
    }

    if (__glMaterial_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_Materialfv(face, pname, &param);
}

GLvoid APIENTRY __gllc_Materialfv(GLenum face, GLenum pname, const GLfloat *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    GLenum error;
    struct __gllc_Materialfv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Materialfv(face, pname, params);
    }

    error = __glErrorCheckMaterial(face, pname, params[0]);
    if(error != GL_NO_ERROR) {
        __gllc_Error(gc, error);
        return;
    }
    arraySize = __GL64PAD(__glMaterial_size(pname) * 4);
    if (arraySize < 0) {
        __gllc_InvalidEnum(gc);
        return;
    }
    size = (GLuint)sizeof(struct __gllc_Materialfv_Rec) + arraySize;
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_Materialfv;
    data = (struct __gllc_Materialfv_Rec *)(dlop + 1);
    data->face = face;
    data->pname = pname;
    __GL_MEMCOPY((GLubyte *)(data + 1), params, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Materiali(GLenum face, GLenum pname, GLint param)
{
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Materiali(face, pname, param);
    }

    if (__glMaterial_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_Materialiv(face, pname, &param);
}

GLvoid APIENTRY __gllc_Materialiv(GLenum face, GLenum pname, const GLint *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    GLenum error;
    struct __gllc_Materialiv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Materialiv(face, pname, params);
    }

    error = __glErrorCheckMaterial(face, pname, (GLfloat)params[0]);
    if(error != GL_NO_ERROR) {
        __gllc_Error(gc, error);
        return;
    }
    arraySize = __GL64PAD(__glMaterial_size(pname) * 4);
    if (arraySize < 0) {
        __gllc_InvalidEnum(gc);
        return;
    }
    size = (GLuint)sizeof(struct __gllc_Materialiv_Rec) + arraySize;
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_Materialiv;
    data = (struct __gllc_Materialiv_Rec *)(dlop + 1);
    data->face = face;
    data->pname = pname;
    __GL_MEMCOPY((GLubyte *)(data + 1), params, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PointSize(GLfloat size)
{
    __GLdlistOp *dlop;
    struct __gllc_PointSize_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PointSize(size);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_PointSize_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_PointSize;
    data = (struct __gllc_PointSize_Rec *)(dlop + 1);
    data->size = size;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PolygonMode(GLenum face, GLenum mode)
{
    __GLdlistOp *dlop;
    struct __gllc_PolygonMode_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PolygonMode(face, mode);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_PolygonMode_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_PolygonMode;
    data = (struct __gllc_PolygonMode_Rec *)(dlop + 1);
    data->face = face;
    data->mode = mode;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Scissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GLdlistOp *dlop;
    struct __gllc_Scissor_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Scissor(x, y, width, height);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Scissor_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Scissor;
    data = (struct __gllc_Scissor_Rec *)(dlop + 1);
    data->x = x;
    data->y = y;
    data->width = width;
    data->height = height;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ShadeModel(GLenum mode)
{
    __GLdlistOp *dlop;
    struct __gllc_ShadeModel_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ShadeModel(mode);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ShadeModel_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ShadeModel;
    data = (struct __gllc_ShadeModel_Rec *)(dlop + 1);
    data->mode = mode;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexParameterf(GLenum target, GLenum pname, GLfloat param)
{
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexParameterf(target, pname, param);
    }

    if (__glTexParameter_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_TexParameterfv(target, pname, &param);
}

GLvoid APIENTRY __gllc_TexParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_TexParameterfv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexParameterfv(target, pname, params);
    }

    arraySize = __GL64PAD(__glTexParameter_size(pname) * 4);
    if (arraySize < 0) {
        __gllc_InvalidEnum(gc);
        return;
    }
    size = (GLuint)sizeof(struct __gllc_TexParameterfv_Rec) + arraySize;
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexParameterfv;
    data = (struct __gllc_TexParameterfv_Rec *)(dlop + 1);
    data->target = target;
    data->pname = pname;
    __GL_MEMCOPY((GLubyte *)(data + 1), params, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexParameteri(GLenum target, GLenum pname, GLint param)
{
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexParameteri(target, pname, param);
    }

    if (__glTexParameter_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_TexParameteriv(target, pname, &param);
}

GLvoid APIENTRY __gllc_TexParameteriv(GLenum target, GLenum pname, const GLint *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_TexParameteriv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexParameteriv(target, pname, params);
    }

    arraySize = __GL64PAD(__glTexParameter_size(pname) * 4);
    if (arraySize < 0) {
        __gllc_InvalidEnum(gc);
        return;
    }
    size = (GLuint)sizeof(struct __gllc_TexParameteriv_Rec) + arraySize;
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexParameteriv;
    data = (struct __gllc_TexParameteriv_Rec *)(dlop + 1);
    data->target = target;
    data->pname = pname;
    __GL_MEMCOPY((GLubyte *)(data + 1), params, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexEnvf(GLenum target, GLenum pname, GLfloat param)
{
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexEnvf(target, pname, param);
    }

    if (__glTexEnv_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_TexEnvfv(target, pname, &param);
}

GLvoid APIENTRY __gllc_TexEnvfv(GLenum target, GLenum pname, const GLfloat *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_TexEnvfv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexEnvfv(target, pname, params);
    }

    arraySize = __GL64PAD(__glTexEnv_size(pname) * 4);
    if (arraySize < 0) {
        __gllc_InvalidEnum(gc);
        return;
    }
    size = (GLuint)sizeof(struct __gllc_TexEnvfv_Rec) + arraySize;
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexEnvfv;
    data = (struct __gllc_TexEnvfv_Rec *)(dlop + 1);
    data->target = target;
    data->pname = pname;
    __GL_MEMCOPY((GLubyte *)(data + 1), params, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexEnvi(GLenum target, GLenum pname, GLint param)
{
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexEnvi(target, pname, param);
    }

    if (__glTexEnv_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_TexEnviv(target, pname, &param);
}

GLvoid APIENTRY __gllc_TexEnviv(GLenum target, GLenum pname, const GLint *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_TexEnviv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexEnviv(target, pname, params);
    }

    arraySize = __GL64PAD(__glTexEnv_size(pname) * 4);
    if (arraySize < 0) {
        __gllc_InvalidEnum(gc);
        return;
    }
    size = (GLuint)sizeof(struct __gllc_TexEnviv_Rec) + arraySize;
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexEnviv;
    data = (struct __gllc_TexEnviv_Rec *)(dlop + 1);
    data->target = target;
    data->pname = pname;
    __GL_MEMCOPY((GLubyte *)(data + 1), params, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexGend(GLenum coord, GLenum pname, GLdouble param)
{
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexGend(coord, pname, param);
    }

    if (__glTexGen_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_TexGendv(coord, pname, &param);
}

GLvoid APIENTRY __gllc_TexGendv(GLenum coord, GLenum pname, const GLdouble *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_TexGendv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexGendv(coord, pname, params);
    }

    arraySize = __GL64PAD(__glTexGen_size(pname) * 8);
    if (arraySize < 0) {
        __gllc_InvalidEnum(gc);
        return;
    }
    size = (GLuint)sizeof(struct __gllc_TexGendv_Rec) + arraySize;
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexGendv;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_TexGendv_Rec *)(dlop + 1);
    data->coord = coord;
    data->pname = pname;
    __GL_MEMCOPY((GLubyte *)(data + 1), params, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexGenf(GLenum coord, GLenum pname, GLfloat param)
{
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexGenf(coord, pname, param);
    }

    if (__glTexGen_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_TexGenfv(coord, pname, &param);
}

GLvoid APIENTRY __gllc_TexGenfv(GLenum coord, GLenum pname, const GLfloat *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_TexGenfv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexGenfv(coord, pname, params);
    }

    arraySize = __GL64PAD(__glTexGen_size(pname) * 4);
    if (arraySize < 0) {
        __gllc_InvalidEnum(gc);
        return;
    }
    size = (GLuint)sizeof(struct __gllc_TexGenfv_Rec) + arraySize;
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexGenfv;
    data = (struct __gllc_TexGenfv_Rec *)(dlop + 1);
    data->coord = coord;
    data->pname = pname;
    __GL_MEMCOPY((GLubyte *)(data + 1), params, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexGeni(GLenum coord, GLenum pname, GLint param)
{
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexGeni(coord, pname, param);
    }

    if (__glTexGen_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_TexGeniv(coord, pname, &param);
}

GLvoid APIENTRY __gllc_TexGeniv(GLenum coord, GLenum pname, const GLint *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_TexGeniv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexGeniv(coord, pname, params);
    }

    arraySize = __GL64PAD(__glTexGen_size(pname) * 4);
    if (arraySize < 0) {
        __gllc_InvalidEnum(gc);
        return;
    }
    size = (GLuint)sizeof(struct __gllc_TexGeniv_Rec) + arraySize;
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexGeniv;
    data = (struct __gllc_TexGeniv_Rec *)(dlop + 1);
    data->coord = coord;
    data->pname = pname;
    __GL_MEMCOPY((GLubyte *)(data + 1), params, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_InitNames(GLvoid)
{
    __GLdlistOp *dlop;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_InitNames();
    }

    dlop = __glDlistAllocOp(gc, 0);
    if (dlop == NULL) return;
    dlop->opcode = __glop_InitNames;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_LoadName(GLuint name)
{
    __GLdlistOp *dlop;
    struct __gllc_LoadName_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_LoadName(name);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_LoadName_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_LoadName;
    data = (struct __gllc_LoadName_Rec *)(dlop + 1);
    data->name = name;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PassThrough(GLfloat token)
{
    __GLdlistOp *dlop;
    struct __gllc_PassThrough_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PassThrough(token);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_PassThrough_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_PassThrough;
    data = (struct __gllc_PassThrough_Rec *)(dlop + 1);
    data->token = token;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PopName(GLvoid)
{
    __GLdlistOp *dlop;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PopName();
    }

    dlop = __glDlistAllocOp(gc, 0);
    if (dlop == NULL) return;
    dlop->opcode = __glop_PopName;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PushName(GLuint name)
{
    __GLdlistOp *dlop;
    struct __gllc_PushName_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PushName(name);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_PushName_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_PushName;
    data = (struct __gllc_PushName_Rec *)(dlop + 1);
    data->name = name;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_DrawBuffer(GLenum mode)
{
    __GLdlistOp *dlop;
    struct __gllc_DrawBuffer_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_DrawBuffer(mode);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_DrawBuffer_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_DrawBuffer;
    data = (struct __gllc_DrawBuffer_Rec *)(dlop + 1);
    data->mode = mode;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Clear(GLbitfield mask)
{
    __GLdlistOp *dlop;
    struct __gllc_Clear_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Clear(mask);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Clear_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Clear;
    data = (struct __gllc_Clear_Rec *)(dlop + 1);
    data->mask = mask;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    __GLdlistOp *dlop;
    struct __gllc_ClearAccum_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ClearAccum(red, green, blue, alpha);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ClearAccum_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ClearAccum;
    data = (struct __gllc_ClearAccum_Rec *)(dlop + 1);
    data->red = red;
    data->green = green;
    data->blue = blue;
    data->alpha = alpha;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ClearIndex(GLfloat c)
{
    __GLdlistOp *dlop;
    struct __gllc_ClearIndex_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ClearIndex(c);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ClearIndex_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ClearIndex;
    data = (struct __gllc_ClearIndex_Rec *)(dlop + 1);
    data->c = c;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    __GLdlistOp *dlop;
    struct __gllc_ClearColor_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ClearColor(red, green, blue, alpha);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ClearColor_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ClearColor;
    data = (struct __gllc_ClearColor_Rec *)(dlop + 1);
    data->red = red;
    data->green = green;
    data->blue = blue;
    data->alpha = alpha;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ClearStencil(GLint s)
{
    __GLdlistOp *dlop;
    struct __gllc_ClearStencil_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ClearStencil(s);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ClearStencil_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ClearStencil;
    data = (struct __gllc_ClearStencil_Rec *)(dlop + 1);
    data->s = s;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ClearDepth(GLclampd depth)
{
    __GLdlistOp *dlop;
    struct __gllc_ClearDepth_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ClearDepth(depth);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ClearDepth_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ClearDepth;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_ClearDepth_Rec *)(dlop + 1);
    data->depth = depth;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_StencilMask(GLuint mask)
{
    __GLdlistOp *dlop;
    struct __gllc_StencilMask_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_StencilMask(mask);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_StencilMask_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_StencilMask;
    data = (struct __gllc_StencilMask_Rec *)(dlop + 1);
    data->mask = mask;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_StencilMaskSeparate(GLenum face, GLuint mask)
{
    __GLdlistOp *dlop;
    struct __gllc_StencilMaskSeparate_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_StencilMaskSeparate(face, mask);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_StencilMaskSeparate_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_StencilMaskSeparate;
    data = (struct __gllc_StencilMaskSeparate_Rec *)(dlop + 1);
    data->face = face;
    data->mask = mask;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    __GLdlistOp *dlop;
    struct __gllc_ColorMask_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ColorMask(red, green, blue, alpha);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ColorMask_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ColorMask;
    data = (struct __gllc_ColorMask_Rec *)(dlop + 1);
    data->red = red;
    data->green = green;
    data->blue = blue;
    data->alpha = alpha;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_DepthMask(GLboolean flag)
{
    __GLdlistOp *dlop;
    struct __gllc_DepthMask_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_DepthMask(flag);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_DepthMask_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_DepthMask;
    data = (struct __gllc_DepthMask_Rec *)(dlop + 1);
    data->flag = flag;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_IndexMask(GLuint mask)
{
    __GLdlistOp *dlop;
    struct __gllc_IndexMask_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_IndexMask(mask);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_IndexMask_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_IndexMask;
    data = (struct __gllc_IndexMask_Rec *)(dlop + 1);
    data->mask = mask;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Accum(GLenum op, GLfloat value)
{
    __GLdlistOp *dlop;
    struct __gllc_Accum_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Accum(op, value);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Accum_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Accum;
    data = (struct __gllc_Accum_Rec *)(dlop + 1);
    data->op = op;
    data->value = value;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PopAttrib(GLvoid)
{
    __GLdlistOp *dlop;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PopAttrib();
    }

    dlop = __glDlistAllocOp(gc, 0);
    if (dlop == NULL) return;
    dlop->opcode = __glop_PopAttrib;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PushAttrib(GLbitfield mask)
{
    __GLdlistOp *dlop;
    struct __gllc_PushAttrib_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PushAttrib(mask);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_PushAttrib_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_PushAttrib;
    data = (struct __gllc_PushAttrib_Rec *)(dlop + 1);
    data->mask = mask;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MapGrid1d(GLint un, GLdouble u1, GLdouble u2)
{
    __GLdlistOp *dlop;
    struct __gllc_MapGrid1d_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_MapGrid1d(un, u1, u2);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MapGrid1d_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MapGrid1d;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_MapGrid1d_Rec *)(dlop + 1);
    data->un = un;
    data->u1 = u1;
    data->u2 = u2;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MapGrid1f(GLint un, GLfloat u1, GLfloat u2)
{
    __GLdlistOp *dlop;
    struct __gllc_MapGrid1f_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_MapGrid1f(un, u1, u2);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MapGrid1f_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MapGrid1f;
    data = (struct __gllc_MapGrid1f_Rec *)(dlop + 1);
    data->un = un;
    data->u1 = u1;
    data->u2 = u2;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2)
{
    __GLdlistOp *dlop;
    struct __gllc_MapGrid2d_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_MapGrid2d(un, u1, u2, vn, v1, v2);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MapGrid2d_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MapGrid2d;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_MapGrid2d_Rec *)(dlop + 1);
    data->un = un;
    data->u1 = u1;
    data->u2 = u2;
    data->vn = vn;
    data->v1 = v1;
    data->v2 = v2;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2)
{
    __GLdlistOp *dlop;
    struct __gllc_MapGrid2f_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_MapGrid2f(un, u1, u2, vn, v1, v2);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MapGrid2f_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MapGrid2f;
    data = (struct __gllc_MapGrid2f_Rec *)(dlop + 1);
    data->un = un;
    data->u1 = u1;
    data->u2 = u2;
    data->vn = vn;
    data->v1 = v1;
    data->v2 = v2;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_EvalCoord1d(GLdouble u)
{
    __GLdlistOp *dlop;
    struct __gllc_EvalCoord1d_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EvalCoord1d(u);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_EvalCoord1d_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_EvalCoord1dv;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_EvalCoord1d_Rec *)(dlop + 1);
    data->u = u;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_EvalCoord1dv(const GLdouble *u)
{
    __GLdlistOp *dlop;
    struct __gllc_EvalCoord1dv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EvalCoord1dv(u);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_EvalCoord1dv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_EvalCoord1dv;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_EvalCoord1dv_Rec *)(dlop + 1);
    data->u[0] = u[0];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_EvalCoord1f(GLfloat u)
{
    __GLdlistOp *dlop;
    struct __gllc_EvalCoord1f_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EvalCoord1f(u);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_EvalCoord1f_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_EvalCoord1fv;
    data = (struct __gllc_EvalCoord1f_Rec *)(dlop + 1);
    data->u = u;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_EvalCoord1fv(const GLfloat *u)
{
    __GLdlistOp *dlop;
    struct __gllc_EvalCoord1fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EvalCoord1fv(u);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_EvalCoord1fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_EvalCoord1fv;
    data = (struct __gllc_EvalCoord1fv_Rec *)(dlop + 1);
    data->u[0] = u[0];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_EvalCoord2d(GLdouble u, GLdouble v)
{
    __GLdlistOp *dlop;
    struct __gllc_EvalCoord2d_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EvalCoord2d(u, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_EvalCoord2d_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_EvalCoord2dv;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_EvalCoord2d_Rec *)(dlop + 1);
    data->u = u;
    data->v = v;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_EvalCoord2dv(const GLdouble *u)
{
    __GLdlistOp *dlop;
    struct __gllc_EvalCoord2dv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EvalCoord2dv(u);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_EvalCoord2dv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_EvalCoord2dv;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_EvalCoord2dv_Rec *)(dlop + 1);
    data->u[0] = u[0];
    data->u[1] = u[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_EvalCoord2f(GLfloat u, GLfloat v)
{
    __GLdlistOp *dlop;
    struct __gllc_EvalCoord2f_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EvalCoord2f(u, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_EvalCoord2f_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_EvalCoord2fv;
    data = (struct __gllc_EvalCoord2f_Rec *)(dlop + 1);
    data->u = u;
    data->v = v;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_EvalCoord2fv(const GLfloat *u)
{
    __GLdlistOp *dlop;
    struct __gllc_EvalCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EvalCoord2fv(u);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_EvalCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_EvalCoord2fv;
    data = (struct __gllc_EvalCoord2fv_Rec *)(dlop + 1);
    data->u[0] = u[0];
    data->u[1] = u[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_EvalMesh1(GLenum mode, GLint i1, GLint i2)
{
    __GLdlistOp *dlop;
    struct __gllc_EvalMesh1_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EvalMesh1(mode, i1, i2);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_EvalMesh1_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_EvalMesh1;
    data = (struct __gllc_EvalMesh1_Rec *)(dlop + 1);
    data->mode = mode;
    data->i1 = i1;
    data->i2 = i2;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_EvalPoint1(GLint i)
{
    __GLdlistOp *dlop;
    struct __gllc_EvalPoint1_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EvalPoint1(i);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_EvalPoint1_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_EvalPoint1;
    data = (struct __gllc_EvalPoint1_Rec *)(dlop + 1);
    data->i = i;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_EvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2)
{
    __GLdlistOp *dlop;
    struct __gllc_EvalMesh2_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EvalMesh2(mode, i1, i2, j1, j2);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_EvalMesh2_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_EvalMesh2;
    data = (struct __gllc_EvalMesh2_Rec *)(dlop + 1);
    data->mode = mode;
    data->i1 = i1;
    data->i2 = i2;
    data->j1 = j1;
    data->j2 = j2;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_EvalPoint2(GLint i, GLint j)
{
    __GLdlistOp *dlop;
    struct __gllc_EvalPoint2_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EvalPoint2(i, j);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_EvalPoint2_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_EvalPoint2;
    data = (struct __gllc_EvalPoint2_Rec *)(dlop + 1);
    data->i = i;
    data->j = j;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_AlphaFunc(GLenum func, GLclampf ref)
{
    __GLdlistOp *dlop;
    struct __gllc_AlphaFunc_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_AlphaFunc(func, ref);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_AlphaFunc_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_AlphaFunc;
    data = (struct __gllc_AlphaFunc_Rec *)(dlop + 1);
    data->func = func;
    data->ref = ref;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_BlendColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    __GLdlistOp *dlop;
    struct __gllc_BlendColor_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_BlendColor(r, g, b, a);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_BlendColor_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_BlendColor;
    data = (struct __gllc_BlendColor_Rec *)(dlop + 1);
    data->r = r;
    data->g = g;
    data->b = b;
    data->a = a;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_BlendFunc(GLenum sfactor, GLenum dfactor)
{
    __GLdlistOp *dlop;
    struct __gllc_BlendFunc_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_BlendFunc(sfactor, dfactor);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_BlendFunc_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_BlendFunc;
    data = (struct __gllc_BlendFunc_Rec *)(dlop + 1);
    data->sfactor = sfactor;
    data->dfactor = dfactor;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_BlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
    __GLdlistOp *dlop;
    struct __gllc_BlendFuncSeparate_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_BlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_BlendFuncSeparate_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_BlendFuncSeparate;
    data = (struct __gllc_BlendFuncSeparate_Rec *)(dlop + 1);
    data->sfactorRGB = sfactorRGB;
    data->dfactorRGB = dfactorRGB;
    data->sfactorAlpha = sfactorAlpha;
    data->dfactorAlpha = dfactorAlpha;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_LogicOp(GLenum opcode)
{
    __GLdlistOp *dlop;
    struct __gllc_LogicOp_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_LogicOp(opcode);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_LogicOp_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_LogicOp;
    data = (struct __gllc_LogicOp_Rec *)(dlop + 1);
    data->opcode = opcode;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_StencilFunc(GLenum func, GLint ref, GLuint mask)
{
    __GLdlistOp *dlop;
    struct __gllc_StencilFunc_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_StencilFunc(func, ref, mask);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_StencilFunc_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_StencilFunc;
    data = (struct __gllc_StencilFunc_Rec *)(dlop + 1);
    data->func = func;
    data->ref = ref;
    data->mask = mask;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_StencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)
{
    __GLdlistOp *dlop;
    struct __gllc_StencilFuncSeparate_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_StencilFuncSeparate(face, func, ref, mask);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_StencilFuncSeparate_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_StencilFuncSeparate;
    data = (struct __gllc_StencilFuncSeparate_Rec *)(dlop + 1);
    data->face = face;
    data->func = func;
    data->ref = ref;
    data->mask = mask;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_StencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
    __GLdlistOp *dlop;
    struct __gllc_StencilOp_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_StencilOp(fail, zfail, zpass);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_StencilOp_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_StencilOp;
    data = (struct __gllc_StencilOp_Rec *)(dlop + 1);
    data->fail = fail;
    data->zfail = zfail;
    data->zpass = zpass;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_StencilOpSeparate(GLenum face, GLenum fail, GLenum zfail, GLenum zpass)
{
    __GLdlistOp *dlop;
    struct __gllc_StencilOpSeparate_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_StencilOpSeparate(face, fail, zfail, zpass);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_StencilOpSeparate_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_StencilOpSeparate;
    data = (struct __gllc_StencilOpSeparate_Rec *)(dlop + 1);
    data->face = face;
    data->fail = fail;
    data->zfail = zfail;
    data->zpass = zpass;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_DepthFunc(GLenum func)
{
    __GLdlistOp *dlop;
    struct __gllc_DepthFunc_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_DepthFunc(func);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_DepthFunc_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_DepthFunc;
    data = (struct __gllc_DepthFunc_Rec *)(dlop + 1);
    data->func = func;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PixelZoom(GLfloat xfactor, GLfloat yfactor)
{
    __GLdlistOp *dlop;
    struct __gllc_PixelZoom_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PixelZoom(xfactor, yfactor);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_PixelZoom_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_PixelZoom;
    data = (struct __gllc_PixelZoom_Rec *)(dlop + 1);
    data->xfactor = xfactor;
    data->yfactor = yfactor;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PixelTransferf(GLenum pname, GLfloat param)
{
    __GLdlistOp *dlop;
    struct __gllc_PixelTransferf_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PixelTransferf(pname, param);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_PixelTransferf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_PixelTransferf;
    data = (struct __gllc_PixelTransferf_Rec *)(dlop + 1);
    data->pname = pname;
    data->param = param;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PixelTransferi(GLenum pname, GLint param)
{
    __GLdlistOp *dlop;
    struct __gllc_PixelTransferi_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PixelTransferi(pname, param);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_PixelTransferi_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_PixelTransferi;
    data = (struct __gllc_PixelTransferi_Rec *)(dlop + 1);
    data->pname = pname;
    data->param = param;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PixelMapfv(GLenum map, GLint mapsize, const GLfloat *values)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_PixelMapfv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PixelMapfv(map, mapsize, values);
    }

    arraySize = __GL64PAD(mapsize * 4);
    if (arraySize < 0) {
        __gllc_InvalidValue(gc);
        return;
    }
    size = (GLuint)sizeof(struct __gllc_PixelMapfv_Rec) + arraySize;
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_PixelMapfv;
    data = (struct __gllc_PixelMapfv_Rec *)(dlop + 1);
    data->map = map;
    data->mapsize = mapsize;
    __GL_MEMCOPY((GLubyte *)(data + 1), values, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PixelMapuiv(GLenum map, GLint mapsize, const GLuint *values)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_PixelMapuiv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PixelMapuiv(map, mapsize, values);
    }

    arraySize = __GL64PAD(mapsize * 4);
    if (arraySize < 0) {
        __gllc_InvalidValue(gc);
        return;
    }
    size = (GLuint)sizeof(struct __gllc_PixelMapuiv_Rec) + arraySize;
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_PixelMapuiv;
    data = (struct __gllc_PixelMapuiv_Rec *)(dlop + 1);
    data->map = map;
    data->mapsize = mapsize;
    __GL_MEMCOPY((GLubyte *)(data + 1), values, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PixelMapusv(GLenum map, GLint mapsize, const GLushort *values)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_PixelMapusv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PixelMapusv(map, mapsize, values);
    }

    arraySize = __GL_PAD(mapsize * 2);
    if (arraySize < 0) {
        __gllc_InvalidValue(gc);
        return;
    }
    size = (GLuint)sizeof(struct __gllc_PixelMapusv_Rec) + arraySize;
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_PixelMapusv;
    data = (struct __gllc_PixelMapusv_Rec *)(dlop + 1);
    data->map = map;
    data->mapsize = mapsize;
    __GL_MEMCOPY((GLubyte *)(data + 1), values, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ReadBuffer(GLenum mode)
{
    __GLdlistOp *dlop;
    struct __gllc_ReadBuffer_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ReadBuffer(mode);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ReadBuffer_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ReadBuffer;
    data = (struct __gllc_ReadBuffer_Rec *)(dlop + 1);
    data->mode = mode;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_CopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)
{
    __GLdlistOp *dlop;
    struct __gllc_CopyPixels_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_CopyPixels(x, y, width, height, type);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_CopyPixels_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_CopyPixels;
    data = (struct __gllc_CopyPixels_Rec *)(dlop + 1);
    data->x = x;
    data->y = y;
    data->width = width;
    data->height = height;
    data->type = type;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_DepthRange(GLclampd zNear, GLclampd zFar)
{
    __GLdlistOp *dlop;
    struct __gllc_DepthRange_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_DepthRange(zNear, zFar);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_DepthRange_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_DepthRange;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_DepthRange_Rec *)(dlop + 1);
    data->zNear = zNear;
    data->zFar = zFar;
    __glDlistAppendOp(gc, dlop);
}

#if GL_EXT_depth_bounds_test
GLvoid APIENTRY __gllc_DepthBoundsEXT(GLclampd zMin, GLclampd zMax)
{
    __GLdlistOp *dlop;
    struct __gllc_DepthBoundTest_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_DepthRange(zMin, zMax);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_DepthBoundTest_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_DepthBoundsEXT;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_DepthBoundTest_Rec *)(dlop + 1);
    data->zMin = zMin;
    data->zMax = zMax;
    __glDlistAppendOp(gc, dlop);
}
#endif

GLvoid APIENTRY __gllc_Frustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
    __GLdlistOp *dlop;
    struct __gllc_Frustum_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Frustum(left, right, bottom, top, zNear, zFar);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Frustum_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Frustum;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_Frustum_Rec *)(dlop + 1);
    data->left = left;
    data->right = right;
    data->bottom = bottom;
    data->top = top;
    data->zNear = zNear;
    data->zFar = zFar;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_LoadIdentity(GLvoid)
{
    __GLdlistOp *dlop;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_LoadIdentity();
    }

    dlop = __glDlistAllocOp(gc, 0);
    if (dlop == NULL) return;
    dlop->opcode = __glop_LoadIdentity;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_LoadMatrixf(const GLfloat *m)
{
    __GLdlistOp *dlop;
    struct __gllc_LoadMatrixf_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_LoadMatrixf(m);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_LoadMatrixf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_LoadMatrixf;
    data = (struct __gllc_LoadMatrixf_Rec *)(dlop + 1);
    __GL_MEMCOPY(data->m, m, sizeof(data->m));
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_LoadMatrixd(const GLdouble *m)
{
    __GLdlistOp *dlop;
    struct __gllc_LoadMatrixd_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_LoadMatrixd(m);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_LoadMatrixd_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_LoadMatrixd;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_LoadMatrixd_Rec *)(dlop + 1);
    __GL_MEMCOPY(data->m, m, sizeof(data->m));
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MatrixMode(GLenum mode)
{
    __GLdlistOp *dlop;
    struct __gllc_MatrixMode_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_MatrixMode(mode);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MatrixMode_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MatrixMode;
    data = (struct __gllc_MatrixMode_Rec *)(dlop + 1);
    data->mode = mode;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultMatrixf(const GLfloat *m)
{
    __GLdlistOp *dlop;
    struct __gllc_MultMatrixf_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_MultMatrixf(m);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultMatrixf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MultMatrixf;
    data = (struct __gllc_MultMatrixf_Rec *)(dlop + 1);
    __GL_MEMCOPY(data->m, m, sizeof(data->m));
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultMatrixd(const GLdouble *m)
{
    __GLdlistOp *dlop;
    struct __gllc_MultMatrixd_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_MultMatrixd(m);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultMatrixd_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MultMatrixd;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_MultMatrixd_Rec *)(dlop + 1);
    __GL_MEMCOPY(data->m, m, sizeof(data->m));
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Ortho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
    __GLdlistOp *dlop;
    struct __gllc_Ortho_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Ortho(left, right, bottom, top, zNear, zFar);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Ortho_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Ortho;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_Ortho_Rec *)(dlop + 1);
    data->left = left;
    data->right = right;
    data->bottom = bottom;
    data->top = top;
    data->zNear = zNear;
    data->zFar = zFar;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PopMatrix(GLvoid)
{
    __GLdlistOp *dlop;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PopMatrix();
    }

    dlop = __glDlistAllocOp(gc, 0);
    if (dlop == NULL) return;
    dlop->opcode = __glop_PopMatrix;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PushMatrix(GLvoid)
{
    __GLdlistOp *dlop;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PushMatrix();
    }

    dlop = __glDlistAllocOp(gc, 0);
    if (dlop == NULL) return;
    dlop->opcode = __glop_PushMatrix;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Rotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    __GLdlistOp *dlop;
    struct __gllc_Rotated_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Rotated(angle, x, y, z);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Rotated_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Rotated;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_Rotated_Rec *)(dlop + 1);
    data->angle = angle;
    data->x = x;
    data->y = y;
    data->z = z;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Rotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    __GLdlistOp *dlop;
    struct __gllc_Rotatef_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Rotatef(angle, x, y, z);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Rotatef_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Rotatef;
    data = (struct __gllc_Rotatef_Rec *)(dlop + 1);
    data->angle = angle;
    data->x = x;
    data->y = y;
    data->z = z;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Scaled(GLdouble x, GLdouble y, GLdouble z)
{
    __GLdlistOp *dlop;
    struct __gllc_Scaled_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Scaled(x, y, z);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Scaled_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Scaled;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_Scaled_Rec *)(dlop + 1);
    data->x = x;
    data->y = y;
    data->z = z;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Scalef(GLfloat x, GLfloat y, GLfloat z)
{
    __GLdlistOp *dlop;
    struct __gllc_Scalef_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Scalef(x, y, z);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Scalef_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Scalef;
    data = (struct __gllc_Scalef_Rec *)(dlop + 1);
    data->x = x;
    data->y = y;
    data->z = z;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Translated(GLdouble x, GLdouble y, GLdouble z)
{
    __GLdlistOp *dlop;
    struct __gllc_Translated_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Translated(x, y, z);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Translated_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Translated;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_Translated_Rec *)(dlop + 1);
    data->x = x;
    data->y = y;
    data->z = z;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Translatef(GLfloat x, GLfloat y, GLfloat z)
{
    __GLdlistOp *dlop;
    struct __gllc_Translatef_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Translatef(x, y, z);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Translatef_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Translatef;
    data = (struct __gllc_Translatef_Rec *)(dlop + 1);
    data->x = x;
    data->y = y;
    data->z = z;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Viewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GLdlistOp *dlop;
    struct __gllc_Viewport_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Viewport(x, y, width, height);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Viewport_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Viewport;
    data = (struct __gllc_Viewport_Rec *)(dlop + 1);
    data->x = x;
    data->y = y;
    data->width = width;
    data->height = height;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PolygonOffset(GLfloat factor, GLfloat units)
{
    __GLdlistOp *dlop;
    struct __gllc_PolygonOffset_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PolygonOffset(factor, units);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_PolygonOffset_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_PolygonOffset;
    data = (struct __gllc_PolygonOffset_Rec *)(dlop + 1);
    data->factor = factor;
    data->units = units;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_BindTexture(GLenum target, GLuint texture)
{
    __GLdlistOp *dlop;
    struct __gllc_BindTexture_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_BindTexture(target, texture);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_BindTexture_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_BindTexture;
    data = (struct __gllc_BindTexture_Rec *)(dlop + 1);
    data->target = target;
    data->texture = texture;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PrioritizeTextures(GLsizei n, const GLuint *textures, const GLclampf *priorities)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize0;
    GLint arraySize1;
    struct __gllc_PrioritizeTextures_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PrioritizeTextures(n, textures, priorities);
    }

    arraySize0 = __GL64PAD(n * 4);
    if (arraySize0 < 0) {
        __gllc_InvalidValue(gc);
        return;
    }
    arraySize1 = __GL64PAD(n * 4);
    if (arraySize1 < 0) {
        __gllc_InvalidValue(gc);
        return;
    }
    size = (GLuint)sizeof(struct __gllc_PrioritizeTextures_Rec) + arraySize0 + arraySize1;
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_PrioritizeTextures;
    data = (struct __gllc_PrioritizeTextures_Rec *)(dlop + 1);
    data->n = n;
    __GL_MEMCOPY((GLubyte *)(data + 1), textures, arraySize0);
    __GL_MEMCOPY((GLubyte *)(data + 1) + arraySize0, priorities, arraySize1);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ActiveTexture(GLenum texture)
{
    __GLdlistOp *dlop;
    struct __gllc_ActiveTexture_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ActiveTexture(texture);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ActiveTexture_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ActiveTexture;
    data = (struct __gllc_ActiveTexture_Rec *)(dlop + 1);
    data->texture = texture;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_BlendEquation(GLenum mode)
{
    __GLdlistOp *dlop;
    struct __gllc_BlendEquation_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_BlendEquation(mode);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_BlendEquation_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_BlendEquation;
    data  = (struct __gllc_BlendEquation_Rec *)(dlop + 1);
    data->mode = mode;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_BlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
    __GLdlistOp *dlop;
    struct __gllc_BlendEquationSeparate_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_BlendEquationSeparate(modeRGB, modeAlpha);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_BlendEquationSeparate_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_BlendEquationSeparate;
    data  = (struct __gllc_BlendEquationSeparate_Rec *)(dlop + 1);
    data->modeRGB = modeRGB;
    data->modeAlpha = modeAlpha;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ColorTableParameteriv(GLenum target, GLenum pname, const GLint *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_ColorTableParameteriv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ColorTableParameteriv(target, pname, params);
    }

    arraySize = __GL64PAD(__glColorTableParameter_size(pname) * 4);
    if (arraySize < 0) {
        __gllc_InvalidEnum(gc);
        return;
    }
    size = (GLuint)sizeof(struct __gllc_ColorTableParameteriv_Rec) + arraySize;
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_ColorTableParameteriv;
    data = (struct __gllc_ColorTableParameteriv_Rec *)(dlop + 1);
    data->target = target;
    data->pname = pname;
    __GL_MEMCOPY((GLubyte *)(data + 1), params, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ColorTableParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_ColorTableParameterfv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ColorTableParameterfv(target, pname, params);
    }

    arraySize = __GL64PAD(__glColorTableParameter_size(pname) * 4);
    if (arraySize < 0) {
        __gllc_InvalidEnum(gc);
        return;
    }
    size = (GLuint)sizeof(struct __gllc_ColorTableParameterfv_Rec) + arraySize;
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_ColorTableParameterfv;
    data = (struct __gllc_ColorTableParameterfv_Rec *)(dlop + 1);
    data->target = target;
    data->pname = pname;
    __GL_MEMCOPY((GLubyte *)(data + 1), params, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ConvolutionParameteri(GLenum target, GLenum pname, GLint param)
{
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ConvolutionParameteri(target, pname, param);
    }

    if (__glConvolutionParameter_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_ConvolutionParameteriv(target, pname, &param);
}

GLvoid APIENTRY __gllc_ConvolutionParameteriv(GLenum target, GLenum pname, const GLint* params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_ConvolutionParameteriv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ConvolutionParameteriv(target, pname, params);
    }

    arraySize = __GL64PAD(__glConvolutionParameter_size(pname) * 4);
    if (arraySize < 0) {
        __gllc_InvalidEnum(gc);
        return;
    }
    size = (GLuint)sizeof(struct __gllc_ConvolutionParameteriv_Rec) + arraySize;
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_ConvolutionParameteriv;
    data = (struct __gllc_ConvolutionParameteriv_Rec *)(dlop + 1);
    data->target = target;
    data->pname = pname;
    __GL_MEMCOPY((GLubyte *)(data + 1), params, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ConvolutionParameterfv(GLenum target, GLenum pname, const GLfloat* params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_ConvolutionParameterfv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ConvolutionParameterfv(target, pname, params);
    }

    arraySize = __GL64PAD(__glConvolutionParameter_size(pname) * 4);
    if (arraySize < 0) {
        __gllc_InvalidEnum(gc);
        return;
    }
    size = (GLuint)sizeof(struct __gllc_ConvolutionParameterfv_Rec) + arraySize;
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_ConvolutionParameterfv;
    data = (struct __gllc_ConvolutionParameterfv_Rec *)(dlop + 1);
    data->target = target;
    data->pname = pname;
    __GL_MEMCOPY((GLubyte *)(data + 1), params, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ConvolutionParameterf(GLenum target, GLenum pname, GLfloat param)
{
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ConvolutionParameterf(target, pname, param);
    }

    if (__glConvolutionParameter_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_ConvolutionParameterfv(target, pname, &param);
}

GLvoid APIENTRY __gllc_Color3b(GLbyte red, GLbyte green, GLbyte blue)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4ubv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color3b)(red, green, blue);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color4ubv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color4ubv;
    data = (struct __gllc_Color4ubv_Rec *)(dlop + 1);
    data->v[0] = __GL_B_TO_UBYTE(red);
    data->v[1] = __GL_B_TO_UBYTE(green);
    data->v[2] = __GL_B_TO_UBYTE(blue);
    data->v[3] = 255;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color3bv(const GLbyte *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4ubv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color3bv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color4ubv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color4ubv;
    data = (struct __gllc_Color4ubv_Rec *)(dlop + 1);
    data->v[0] = __GL_B_TO_UBYTE(v[0]);
    data->v[1] = __GL_B_TO_UBYTE(v[1]);
    data->v[2] = __GL_B_TO_UBYTE(v[2]);
    data->v[3] = 255;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color3d(GLdouble red, GLdouble green, GLdouble blue)
{
    __GLdlistOp *dlop;
    struct __gllc_Color3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color3d)(red, green, blue);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color3fv;
    data = (struct __gllc_Color3fv_Rec *)(dlop + 1);
    data->v[0] = red;
    data->v[1] = green;
    data->v[2] = blue;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color3dv(const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color3dv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color3fv;
    data = (struct __gllc_Color3fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color3f(GLfloat red, GLfloat green, GLfloat blue)
{
    __GLdlistOp *dlop;
    struct __gllc_Color3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color3f)(red, green, blue);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color3fv;
    data = (struct __gllc_Color3fv_Rec *)(dlop + 1);
    data->v[0] = red;
    data->v[1] = green;
    data->v[2] = blue;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color3fv(const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color3fv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color3fv;
    data = (struct __gllc_Color3fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color3i(GLint red, GLint green, GLint blue)
{
    __GLdlistOp *dlop;
    struct __gllc_Color3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color3i)(red, green, blue);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color3fv;
    data = (struct __gllc_Color3fv_Rec *)(dlop + 1);
    data->v[0] = __GL_I_TO_FLOAT(red);
    data->v[1] = __GL_I_TO_FLOAT(green);
    data->v[2] = __GL_I_TO_FLOAT(blue);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color3iv(const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color3iv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color3fv;
    data = (struct __gllc_Color3fv_Rec *)(dlop + 1);
    data->v[0] = __GL_I_TO_FLOAT(v[0]);
    data->v[1] = __GL_I_TO_FLOAT(v[1]);
    data->v[2] = __GL_I_TO_FLOAT(v[2]);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color3s(GLshort red, GLshort green, GLshort blue)
{
    __GLdlistOp *dlop;
    struct __gllc_Color3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color3s)(red, green, blue);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color3fv;
    data = (struct __gllc_Color3fv_Rec *)(dlop + 1);
    data->v[0] = __GL_I_TO_FLOAT(red);
    data->v[1] = __GL_I_TO_FLOAT(green);
    data->v[2] = __GL_I_TO_FLOAT(blue);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color3sv(const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color3sv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color3fv;
    data = (struct __gllc_Color3fv_Rec *)(dlop + 1);
    data->v[0] = __GL_S_TO_FLOAT(v[0]);
    data->v[1] = __GL_S_TO_FLOAT(v[1]);
    data->v[2] = __GL_S_TO_FLOAT(v[2]);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color3ub(GLubyte red, GLubyte green, GLubyte blue)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4ubv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color3ub)(red, green, blue);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color4ubv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color4ubv;
    data = (struct __gllc_Color4ubv_Rec *)(dlop + 1);
    data->v[0] = red;
    data->v[1] = green;
    data->v[2] = blue;
    data->v[3] = 255;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color3ubv(const GLubyte *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4ubv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color3ubv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color4ubv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color4ubv;
    data = (struct __gllc_Color4ubv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = 255;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color3ui(GLuint red, GLuint green, GLuint blue)
{
    __GLdlistOp *dlop;
    struct __gllc_Color3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color3ui)(red, green, blue);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color3fv;
    data = (struct __gllc_Color3fv_Rec *)(dlop + 1);
    data->v[0] = __GL_UI_TO_FLOAT(red);
    data->v[1] = __GL_UI_TO_FLOAT(green);
    data->v[2] = __GL_UI_TO_FLOAT(blue);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color3uiv(const GLuint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color3uiv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color3fv;
    data = (struct __gllc_Color3fv_Rec *)(dlop + 1);
    data->v[0] = __GL_UI_TO_FLOAT(v[0]);
    data->v[1] = __GL_UI_TO_FLOAT(v[1]);
    data->v[2] = __GL_UI_TO_FLOAT(v[2]);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color3us(GLushort red, GLushort green, GLushort blue)
{
    __GLdlistOp *dlop;
    struct __gllc_Color3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color3us)(red, green, blue);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color3fv;
    data = (struct __gllc_Color3fv_Rec *)(dlop + 1);
    data->v[0] = __GL_US_TO_FLOAT(red);
    data->v[1] = __GL_US_TO_FLOAT(green);
    data->v[2] = __GL_US_TO_FLOAT(blue);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color3usv(const GLushort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color3usv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color3fv;
    data = (struct __gllc_Color3fv_Rec *)(dlop + 1);
    data->v[0] = __GL_US_TO_FLOAT(v[0]);
    data->v[1] = __GL_US_TO_FLOAT(v[1]);
    data->v[2] = __GL_US_TO_FLOAT(v[2]);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4ubv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color4b)(red, green, blue, alpha);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color4ubv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color4ubv;
    data = (struct __gllc_Color4ubv_Rec *)(dlop + 1);
    data->v[0] = __GL_B_TO_UBYTE(red);
    data->v[1] = __GL_B_TO_UBYTE(green);
    data->v[2] = __GL_B_TO_UBYTE(blue);
    data->v[3] = __GL_B_TO_UBYTE(alpha);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color4bv(const GLbyte *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4ubv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color4bv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color4ubv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color4ubv;
    data = (struct __gllc_Color4ubv_Rec *)(dlop + 1);
    data->v[0] = __GL_B_TO_UBYTE(v[0]);
    data->v[1] = __GL_B_TO_UBYTE(v[1]);
    data->v[2] = __GL_B_TO_UBYTE(v[2]);
    data->v[3] = __GL_B_TO_UBYTE(v[3]);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color4d)(red, green, blue, alpha);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color4fv;
    data = (struct __gllc_Color4fv_Rec *)(dlop + 1);
    data->v[0] = red;
    data->v[1] = green;
    data->v[2] = blue;
    data->v[3] = alpha;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color4dv(const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color4dv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color4fv;
    data = (struct __gllc_Color4fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color4f)(red, green, blue, alpha);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color4fv;
    data = (struct __gllc_Color4fv_Rec *)(dlop + 1);
    data->v[0] = red;
    data->v[1] = green;
    data->v[2] = blue;
    data->v[3] = alpha;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color4fv(const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color4fv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color4fv;
    data = (struct __gllc_Color4fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color4i(GLint red, GLint green, GLint blue, GLint alpha)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color4i)(red, green, blue, alpha);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color4fv;
    data = (struct __gllc_Color4fv_Rec *)(dlop + 1);
    data->v[0] = __GL_I_TO_FLOAT(red);
    data->v[1] = __GL_I_TO_FLOAT(green);
    data->v[2] = __GL_I_TO_FLOAT(blue);
    data->v[3] = __GL_I_TO_FLOAT(alpha);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color4iv(const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color4iv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color4fv;
    data = (struct __gllc_Color4fv_Rec *)(dlop + 1);
    data->v[0] = __GL_I_TO_FLOAT(v[0]);
    data->v[1] = __GL_I_TO_FLOAT(v[1]);
    data->v[2] = __GL_I_TO_FLOAT(v[2]);
    data->v[3] = __GL_I_TO_FLOAT(v[3]);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color4s(GLshort red, GLshort green, GLshort blue, GLshort alpha)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color4s)(red, green, blue, alpha);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color4fv;
    data = (struct __gllc_Color4fv_Rec *)(dlop + 1);
    data->v[0] = __GL_I_TO_FLOAT(red);
    data->v[1] = __GL_I_TO_FLOAT(green);
    data->v[2] = __GL_I_TO_FLOAT(blue);
    data->v[3] = __GL_I_TO_FLOAT(alpha);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color4sv(const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color4sv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color4fv;
    data = (struct __gllc_Color4fv_Rec *)(dlop + 1);
    data->v[0] = __GL_S_TO_FLOAT(v[0]);
    data->v[1] = __GL_S_TO_FLOAT(v[1]);
    data->v[2] = __GL_S_TO_FLOAT(v[2]);
    data->v[3] = __GL_S_TO_FLOAT(v[3]);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4ubv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color4ub)(red, green, blue, alpha);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color4ubv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color4ubv;
    data = (struct __gllc_Color4ubv_Rec *)(dlop + 1);
    data->v[0] = red;
    data->v[1] = green;
    data->v[2] = blue;
    data->v[3] = alpha;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color4ubv(const GLubyte *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4ubv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color4ubv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color4ubv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color4ubv;
    data = (struct __gllc_Color4ubv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color4ui)(red, green, blue, alpha);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color4fv;
    data = (struct __gllc_Color4fv_Rec *)(dlop + 1);
    data->v[0] = __GL_UI_TO_FLOAT(red);
    data->v[1] = __GL_UI_TO_FLOAT(green);
    data->v[2] = __GL_UI_TO_FLOAT(blue);
    data->v[3] = __GL_UI_TO_FLOAT(alpha);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color4uiv(const GLuint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color4uiv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color4fv;
    data = (struct __gllc_Color4fv_Rec *)(dlop + 1);
    data->v[0] = __GL_UI_TO_FLOAT(v[0]);
    data->v[1] = __GL_UI_TO_FLOAT(v[1]);
    data->v[2] = __GL_UI_TO_FLOAT(v[2]);
    data->v[3] = __GL_UI_TO_FLOAT(v[3]);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color4us(GLushort red, GLushort green, GLushort blue, GLushort alpha)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color4us)(red, green, blue, alpha);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color4fv;
    data = (struct __gllc_Color4fv_Rec *)(dlop + 1);
    data->v[0] = __GL_US_TO_FLOAT(red);
    data->v[1] = __GL_US_TO_FLOAT(green);
    data->v[2] = __GL_US_TO_FLOAT(blue);
    data->v[3] = __GL_US_TO_FLOAT(alpha);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Color4usv(const GLushort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Color4usv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Color4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Color4fv;
    data = (struct __gllc_Color4fv_Rec *)(dlop + 1);
    data->v[0] = __GL_US_TO_FLOAT(v[0]);
    data->v[1] = __GL_US_TO_FLOAT(v[1]);
    data->v[2] = __GL_US_TO_FLOAT(v[2]);
    data->v[3] = __GL_US_TO_FLOAT(v[3]);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Normal3b(GLbyte nx, GLbyte ny, GLbyte nz)
{
    __GLdlistOp *dlop;
    struct __gllc_Normal3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Normal3b)(nx, ny, nz);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Normal3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Normal3fv;
    data = (struct __gllc_Normal3fv_Rec *)(dlop + 1);
    data->v[0] = __GL_B_TO_FLOAT(nx);
    data->v[1] = __GL_B_TO_FLOAT(ny);
    data->v[2] = __GL_B_TO_FLOAT(nz);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Normal3bv(const GLbyte *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Normal3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Normal3bv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Normal3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Normal3fv;
    data = (struct __gllc_Normal3fv_Rec *)(dlop + 1);
    data->v[0] = __GL_B_TO_FLOAT(v[0]);
    data->v[1] = __GL_B_TO_FLOAT(v[1]);
    data->v[2] = __GL_B_TO_FLOAT(v[2]);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Normal3d(GLdouble nx, GLdouble ny, GLdouble nz)
{
    __GLdlistOp *dlop;
    struct __gllc_Normal3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Normal3d)(nx, ny, nz);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Normal3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Normal3fv;
    data = (struct __gllc_Normal3fv_Rec *)(dlop + 1);
    data->v[0] = nx;
    data->v[1] = ny;
    data->v[2] = nz;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Normal3dv(const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Normal3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Normal3dv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Normal3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Normal3fv;
    data = (struct __gllc_Normal3fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Normal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
    __GLdlistOp *dlop;
    struct __gllc_Normal3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Normal3f)(nx, ny, nz);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Normal3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Normal3fv;
    data = (struct __gllc_Normal3fv_Rec *)(dlop + 1);
    data->v[0] = nx;
    data->v[1] = ny;
    data->v[2] = nz;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Normal3fv(const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Normal3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Normal3fv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Normal3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Normal3fv;
    data = (struct __gllc_Normal3fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Normal3i(GLint nx, GLint ny, GLint nz)
{
    __GLdlistOp *dlop;
    struct __gllc_Normal3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Normal3i)(nx, ny, nz);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Normal3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Normal3fv;
    data = (struct __gllc_Normal3fv_Rec *)(dlop + 1);
    data->v[0] = __GL_I_TO_FLOAT(nx);
    data->v[1] = __GL_I_TO_FLOAT(ny);
    data->v[2] = __GL_I_TO_FLOAT(nz);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Normal3iv(const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Normal3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Normal3iv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Normal3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Normal3fv;
    data = (struct __gllc_Normal3fv_Rec *)(dlop + 1);
    data->v[0] = __GL_I_TO_FLOAT(v[0]);
    data->v[1] = __GL_I_TO_FLOAT(v[1]);
    data->v[2] = __GL_I_TO_FLOAT(v[2]);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Normal3s(GLshort nx, GLshort ny, GLshort nz)
{
    __GLdlistOp *dlop;
    struct __gllc_Normal3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Normal3s)(nx, ny, nz);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Normal3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Normal3fv;
    data = (struct __gllc_Normal3fv_Rec *)(dlop + 1);
    data->v[0] = __GL_S_TO_FLOAT(nx);
    data->v[1] = __GL_S_TO_FLOAT(ny);
    data->v[2] = __GL_S_TO_FLOAT(nz);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Normal3sv(const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Normal3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Normal3sv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Normal3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Normal3fv;
    data = (struct __gllc_Normal3fv_Rec *)(dlop + 1);
    data->v[0] = __GL_S_TO_FLOAT(v[0]);
    data->v[1] = __GL_S_TO_FLOAT(v[1]);
    data->v[2] = __GL_S_TO_FLOAT(v[2]);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord1d(GLdouble s)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord1d)(s);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = s;
    data->v[1] = 0.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord1dv(const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord1dv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = 0.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord1f(GLfloat s)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord1f)(s);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = s;
    data->v[1] = 0.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord1fv(const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord1fv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = 0.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord1i(GLint s)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord1i)(s);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = s;
    data->v[1] = 0.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord1iv(const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord1iv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = 0.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord1s(GLshort s)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord1s)(s);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = s;
    data->v[1] = 0.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord1sv(const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord1sv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = 0.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord2d(GLdouble s, GLdouble t)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord2d)(s, t);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = s;
    data->v[1] = t;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord2dv(const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord2dv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord2f(GLfloat s, GLfloat t)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord2f)(s, t);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = s;
    data->v[1] = t;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord2fv(const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord2fv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord2i(GLint s, GLint t)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord2i)(s, t);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = s;
    data->v[1] = t;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord2iv(const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord2iv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord2s(GLshort s, GLshort t)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord2s)(s, t);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = s;
    data->v[1] = t;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord2sv(const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord2sv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord3d(GLdouble s, GLdouble t, GLdouble r)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord3d)(s, t, r);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord3fv;
    data = (struct __gllc_TexCoord3fv_Rec *)(dlop + 1);
    data->v[0] = s;
    data->v[1] = t;
    data->v[2] = r;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord3dv(const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord3dv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord3fv;
    data = (struct __gllc_TexCoord3fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord3f(GLfloat s, GLfloat t, GLfloat r)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord3f)(s, t, r);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord3fv;
    data = (struct __gllc_TexCoord3fv_Rec *)(dlop + 1);
    data->v[0] = s;
    data->v[1] = t;
    data->v[2] = r;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord3fv(const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord3fv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord3fv;
    data = (struct __gllc_TexCoord3fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord3i(GLint s, GLint t, GLint r)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord3i)(s, t, r);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord3fv;
    data = (struct __gllc_TexCoord3fv_Rec *)(dlop + 1);
    data->v[0] = s;
    data->v[1] = t;
    data->v[2] = r;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord3iv(const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord3iv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord3fv;
    data = (struct __gllc_TexCoord3fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord3s(GLshort s, GLshort t, GLshort r)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord3s)(s, t, r);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord3fv;
    data = (struct __gllc_TexCoord3fv_Rec *)(dlop + 1);
    data->v[0] = s;
    data->v[1] = t;
    data->v[2] = r;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord3sv(const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord3sv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord3fv;
    data = (struct __gllc_TexCoord3fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord4d)(s, t, r, q);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord4fv;
    data = (struct __gllc_TexCoord4fv_Rec *)(dlop + 1);
    data->v[0] = s;
    data->v[1] = t;
    data->v[2] = r;
    data->v[3] = q;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord4dv(const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord4dv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord4fv;
    data = (struct __gllc_TexCoord4fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord4f)(s, t, r, q);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord4fv;
    data = (struct __gllc_TexCoord4fv_Rec *)(dlop + 1);
    data->v[0] = s;
    data->v[1] = t;
    data->v[2] = r;
    data->v[3] = q;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord4fv(const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord4fv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord4fv;
    data = (struct __gllc_TexCoord4fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord4i(GLint s, GLint t, GLint r, GLint q)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord4i)(s, t, r, q);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord4fv;
    data = (struct __gllc_TexCoord4fv_Rec *)(dlop + 1);
    data->v[0] = s;
    data->v[1] = t;
    data->v[2] = r;
    data->v[3] = q;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord4iv(const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord4iv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord4fv;
    data = (struct __gllc_TexCoord4fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord4s)(s, t, r, q);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord4fv;
    data = (struct __gllc_TexCoord4fv_Rec *)(dlop + 1);
    data->v[0] = s;
    data->v[1] = t;
    data->v[2] = r;
    data->v[3] = q;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord4sv(const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.TexCoord4sv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord4fv;
    data = (struct __gllc_TexCoord4fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex2d(GLdouble x, GLdouble y)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Vertex2d)(x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex2fv;
    data = (struct __gllc_Vertex2fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex2dv(const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Vertex2dv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex2fv;
    data = (struct __gllc_Vertex2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex2f(GLfloat x, GLfloat y)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Vertex2f)(x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex2fv;
    data = (struct __gllc_Vertex2fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex2fv(const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Vertex2fv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex2fv;
    data = (struct __gllc_Vertex2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex2i(GLint x, GLint y)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Vertex2i)(x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex2fv;
    data = (struct __gllc_Vertex2fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex2iv(const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Vertex2iv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex2fv;
    data = (struct __gllc_Vertex2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex2s(GLshort x, GLshort y)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Vertex2s)(x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex2fv;
    data = (struct __gllc_Vertex2fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex2sv(const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Vertex2sv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex2fv;
    data = (struct __gllc_Vertex2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex3d(GLdouble x, GLdouble y, GLdouble z)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Vertex3d)(x, y, z);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex3fv;
    data = (struct __gllc_Vertex3fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = z;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex3dv(const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Vertex3dv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex3fv;
    data = (struct __gllc_Vertex3fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Vertex3f)(x, y, z);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex3fv;
    data = (struct __gllc_Vertex3fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = z;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex3fv(const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Vertex3fv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex3fv;
    data = (struct __gllc_Vertex3fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex3i(GLint x, GLint y, GLint z)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Vertex3i)(x, y, z);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex3fv;
    data = (struct __gllc_Vertex3fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = z;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex3iv(const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Vertex3iv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex3fv;
    data = (struct __gllc_Vertex3fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex3s(GLshort x, GLshort y, GLshort z)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Vertex3s)(x, y, z);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex3fv;
    data = (struct __gllc_Vertex3fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = z;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex3sv(const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Vertex3sv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex3fv;
    data = (struct __gllc_Vertex3fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Vertex4d)(x, y, z, w);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex4fv;
    data = (struct __gllc_Vertex4fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = z;
    data->v[3] = w;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex4dv(const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Vertex4dv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex4fv;
    data = (struct __gllc_Vertex4fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Vertex4f)(x, y, z, w);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex4fv;
    data = (struct __gllc_Vertex4fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = z;
    data->v[3] = w;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex4fv(const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Vertex4fv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex4fv;
    data = (struct __gllc_Vertex4fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex4i(GLint x, GLint y, GLint z, GLint w)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Vertex4i)(x, y, z, w);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex4fv;
    data = (struct __gllc_Vertex4fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = z;
    data->v[3] = w;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex4iv(const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Vertex4iv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex4fv;
    data = (struct __gllc_Vertex4fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Vertex4s)(x, y, z, w);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex4fv;
    data = (struct __gllc_Vertex4fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = z;
    data->v[3] = w;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex4sv(const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.Vertex4sv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex4fv;
    data = (struct __gllc_Vertex4fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord1d(GLenum texture, GLdouble s)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord1d)(texture, s);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_MultiTexCoord2fv;
    data = (struct __gllc_MultiTexCoord2fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = s;
    data->v[1] = 0.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord1dv(GLenum texture, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord1dv)(texture, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MultiTexCoord2fv;
    data = (struct __gllc_MultiTexCoord2fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = v[0];
    data->v[1] = 0.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord1f(GLenum texture, GLfloat s)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord1f)(texture, s);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_MultiTexCoord2fv;
    data = (struct __gllc_MultiTexCoord2fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = s;
    data->v[1] = 0.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord1fv(GLenum texture, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord1fv)(texture, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_MultiTexCoord2fv;
    data = (struct __gllc_MultiTexCoord2fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = v[0];
    data->v[1] = 0.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord1i(GLenum texture, GLint s)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord1i)(texture, s);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MultiTexCoord2fv;
    data = (struct __gllc_MultiTexCoord2fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = s;
    data->v[1] = 0.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord1iv(GLenum texture, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord1iv)(texture, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_MultiTexCoord2fv;
    data = (struct __gllc_MultiTexCoord2fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = v[0];
    data->v[1] = 0.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord1s(GLenum texture, GLshort s)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord1s)(texture, s);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MultiTexCoord2fv;
    data = (struct __gllc_MultiTexCoord2fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = s;
    data->v[1] = 0.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord1sv(GLenum texture, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord1sv)(texture, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MultiTexCoord2fv;
    data = (struct __gllc_MultiTexCoord2fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = v[0];
    data->v[1] = 0.0;
    __glDlistAppendOp(gc, dlop);

}

GLvoid APIENTRY __gllc_MultiTexCoord2d(GLenum texture, GLdouble s, GLdouble t)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord2d)(texture, s, t);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_MultiTexCoord2fv;
    data = (struct __gllc_MultiTexCoord2fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = s;
    data->v[1] = t;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord2dv(GLenum texture, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord2dv)(texture, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_MultiTexCoord2fv;
    data = (struct __gllc_MultiTexCoord2fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord2f(GLenum texture, GLfloat s, GLfloat t)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord2f)(texture, s, t);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_MultiTexCoord2fv;
    data = (struct __gllc_MultiTexCoord2fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = s;
    data->v[1] = t;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord2fv(GLenum texture, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord2fv)(texture, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_MultiTexCoord2fv;
    data = (struct __gllc_MultiTexCoord2fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord2i(GLenum texture, GLint s, GLint t)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord2i)(texture, s, t);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_MultiTexCoord2fv;
    data = (struct __gllc_MultiTexCoord2fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = s;
    data->v[1] = t;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord2iv(GLenum texture, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord2iv)(texture, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord2fv_Rec));
    dlop->opcode  = __glop_MultiTexCoord2fv;
    data = (struct __gllc_MultiTexCoord2fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord2s(GLenum texture, GLshort s, GLshort t)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord2s)(texture, s, t);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_MultiTexCoord2fv;
    data = (struct __gllc_MultiTexCoord2fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = s;
    data->v[1] = t;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord2sv(GLenum texture, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord2sv)(texture, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_MultiTexCoord2fv;
    data = (struct __gllc_MultiTexCoord2fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord3d(GLenum texture, GLdouble s, GLdouble t, GLdouble r)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord3d)(texture, s, t, r);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_MultiTexCoord3fv;
    data = (struct __gllc_MultiTexCoord3fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = s;
    data->v[1] = t;
    data->v[2] = r;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord3dv(GLenum texture, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord3dv)(texture, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_MultiTexCoord3fv;
    data = (struct __gllc_MultiTexCoord3fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord3f(GLenum texture, GLfloat s, GLfloat t, GLfloat r)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord3f)(texture, s, t, r);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_MultiTexCoord3fv;
    data = (struct __gllc_MultiTexCoord3fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = s;
    data->v[1] = t;
    data->v[2] = r;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord3fv(GLenum texture, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord3fv)(texture, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_MultiTexCoord3fv;
    data = (struct __gllc_MultiTexCoord3fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord3i(GLenum texture, GLint s, GLint t, GLint r)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord3i)(texture, s, t, r);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_MultiTexCoord3fv;
    data = (struct __gllc_MultiTexCoord3fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = s;
    data->v[1] = t;
    data->v[2] = r;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord3iv(GLenum texture, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord3iv)(texture, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_MultiTexCoord3fv;
    data = (struct __gllc_MultiTexCoord3fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord3s(GLenum texture, GLshort s, GLshort t, GLshort r)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord3s)(texture, s, t, r);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_MultiTexCoord3fv;
    data = (struct __gllc_MultiTexCoord3fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = s;
    data->v[1] = t;
    data->v[2] = r;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord3sv(GLenum texture, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord3sv)(texture, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_MultiTexCoord3fv;
    data = (struct __gllc_MultiTexCoord3fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord4d(GLenum texture, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord4d)(texture, s, t, r, q);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_MultiTexCoord4fv;
    data = (struct __gllc_MultiTexCoord4fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = s;
    data->v[1] = t;
    data->v[2] = r;
    data->v[3] = q;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord4dv(GLenum texture, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord4dv)(texture, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_MultiTexCoord4fv;
    data = (struct __gllc_MultiTexCoord4fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord4f(GLenum texture, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord4f)(texture, s, t, r, q);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_MultiTexCoord4fv;
    data = (struct __gllc_MultiTexCoord4fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = s;
    data->v[1] = t;
    data->v[2] = r;
    data->v[3] = q;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord4fv(GLenum texture, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord4fv)(texture, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_MultiTexCoord4fv;
    data = (struct __gllc_MultiTexCoord4fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord4i(GLenum texture, GLint s, GLint t, GLint r, GLint q)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord4i)(texture, s, t, r, q);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_MultiTexCoord4fv;
    data = (struct __gllc_MultiTexCoord4fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = s;
    data->v[1] = t;
    data->v[2] = r;
    data->v[3] = q;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord4iv(GLenum texture, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord4iv)(texture, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MultiTexCoord4fv;
    data = (struct __gllc_MultiTexCoord4fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord4s(GLenum texture, GLshort s, GLshort t, GLshort r, GLshort q)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord4s)(texture, s, t, r, q);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MultiTexCoord4fv;
    data = (struct __gllc_MultiTexCoord4fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = s;
    data->v[1] = t;
    data->v[2] = r;
    data->v[3] = q;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord4sv(GLenum texture, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.MultiTexCoord4sv)(texture, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MultiTexCoord4fv;
    data = (struct __gllc_MultiTexCoord4fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_FogCoordf(GLfloat coord)
{
    __GLdlistOp *dlop;
    struct __gllc_FogCoordf_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.FogCoordf)(coord);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_FogCoordf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_FogCoordf;
    data = (struct __gllc_FogCoordf_Rec *)(dlop + 1);
    data->coord = coord;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_FogCoordd(GLdouble coord)
{
    __GLdlistOp *dlop;
    struct __gllc_FogCoordf_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.FogCoordd)(coord);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_FogCoordf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_FogCoordf;
    data = (struct __gllc_FogCoordf_Rec *)(dlop + 1);
    data->coord = coord;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_FogCoordfv(const GLfloat coord[])
{
    __GLdlistOp *dlop;
    struct __gllc_FogCoordf_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.FogCoordfv)(coord);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_FogCoordf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_FogCoordf;
    data = (struct __gllc_FogCoordf_Rec *)(dlop + 1);
    data->coord = coord[0];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_FogCoorddv(const GLdouble coord[])
{
    __GLdlistOp *dlop;
    struct __gllc_FogCoordf_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.FogCoorddv)(coord);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_FogCoordf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_FogCoordf;
    data = (struct __gllc_FogCoordf_Rec *)(dlop + 1);
    data->coord = coord[0];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib1s(GLuint index, GLshort x)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib1s)(index, x);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = x;
    data->v[1] = 0.0;
    data->v[2] = 0.0;
    data->v[3] = 1.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib1f(GLuint index, GLfloat x)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib1f)(index, x);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = x;
    data->v[1] = 0.0;
    data->v[2] = 0.0;
    data->v[3] = 1.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib1d(GLuint index, GLdouble x)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib1d)(index, x);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = x;
    data->v[1] = 0.0;
    data->v[2] = 0.0;
    data->v[3] = 1.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib2s(GLuint index, GLshort x, GLshort y)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib2s)(index, x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = 0.0;
    data->v[3] = 1.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib2f(GLuint index, GLfloat x, GLfloat y)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib2f)(index, x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = 0.0;
    data->v[3] = 1.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib2d(GLuint index, GLdouble x, GLdouble y)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib2d)(index, x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = 0.0;
    data->v[3] = 1.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib3s(GLuint index, GLshort x, GLshort y, GLshort z)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib3s)(index, x, y, z);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = z;
    data->v[3] = 1.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib3f)(index, x, y, z);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = z;
    data->v[3] = 1.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib3d(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib3d)(index, x, y, z);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = z;
    data->v[3] = 1.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib4s(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib4s)(index, x, y, z, w);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = z;
    data->v[3] = w;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib4f)(index, x, y, z, w);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = z;
    data->v[3] = w;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib4d)(index, x, y, z, w);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = z;
    data->v[3] = w;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib4Nub(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib4Nub)(index, x, y, z, w);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = __GL_UB_TO_FLOAT(x);
    data->v[1] = __GL_UB_TO_FLOAT(y);
    data->v[2] = __GL_UB_TO_FLOAT(z);
    data->v[3] = __GL_UB_TO_FLOAT(w);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib1sv(GLuint index, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib1sv)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = v[0];
    data->v[1] = 0.0;
    data->v[2] = 0.0;
    data->v[3] = 1.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib1fv(GLuint index, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib1fv)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = v[0];
    data->v[1] = 0.0;
    data->v[2] = 0.0;
    data->v[3] = 1.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib1dv(GLuint index, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib1dv)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = v[0];
    data->v[1] = 0.0;
    data->v[2] = 0.0;
    data->v[3] = 1.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib2sv(GLuint index, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib2sv)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = 0.0;
    data->v[3] = 1.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib2fv(GLuint index, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib2fv)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = 0.0;
    data->v[3] = 1.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib2dv(GLuint index, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib2dv)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = 0.0;
    data->v[3] = 1.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib3sv(GLuint index, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib3sv)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = 1.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib3fv(GLuint index, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib3fv)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = 1.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib3dv(GLuint index, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib3dv)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = 1.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib4bv(GLuint index, const GLbyte *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib4bv)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib4sv(GLuint index, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib4sv)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib4iv(GLuint index, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib4iv)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib4ubv(GLuint index, const GLubyte *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib4ubv)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib4usv(GLuint index, const GLushort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib4usv)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib4uiv(GLuint index, const GLuint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib4uiv)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib4fv(GLuint index, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib4fv)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib4dv(GLuint index, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib4dv)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib4Nbv(GLuint index, const GLbyte *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib4Nbv)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = __GL_UB_TO_FLOAT(v[0]);
    data->v[1] = __GL_UB_TO_FLOAT(v[1]);
    data->v[2] = __GL_UB_TO_FLOAT(v[2]);
    data->v[3] = __GL_UB_TO_FLOAT(v[3]);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib4Nsv(GLuint index, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib4Nsv)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = __GL_S_TO_FLOAT(v[0]);
    data->v[1] = __GL_S_TO_FLOAT(v[1]);
    data->v[2] = __GL_S_TO_FLOAT(v[2]);
    data->v[3] = __GL_S_TO_FLOAT(v[3]);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib4Niv(GLuint index, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib4Niv)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = __GL_I_TO_FLOAT(v[0]);
    data->v[1] = __GL_I_TO_FLOAT(v[1]);
    data->v[2] = __GL_I_TO_FLOAT(v[2]);
    data->v[3] = __GL_I_TO_FLOAT(v[3]);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib4Nubv(GLuint index, const GLubyte *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib4Nubv)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = __GL_UB_TO_FLOAT(v[0]);
    data->v[1] = __GL_UB_TO_FLOAT(v[1]);
    data->v[2] = __GL_UB_TO_FLOAT(v[2]);
    data->v[3] = __GL_UB_TO_FLOAT(v[3]);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib4Nusv(GLuint index, const GLushort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib4Nusv)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = __GL_US_TO_FLOAT(v[0]);
    data->v[1] = __GL_US_TO_FLOAT(v[1]);
    data->v[2] = __GL_US_TO_FLOAT(v[2]);
    data->v[3] = __GL_US_TO_FLOAT(v[3]);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib4Nuiv(GLuint index, const GLuint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttrib4Nuiv)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->v[0] = __GL_UI_TO_FLOAT(v[0]);
    data->v[1] = __GL_UI_TO_FLOAT(v[1]);
    data->v[2] = __GL_UI_TO_FLOAT(v[2]);
    data->v[3] = __GL_UI_TO_FLOAT(v[3]);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Bitmap(GLsizei width, GLsizei height,
                            GLfloat xorig, GLfloat yorig,
                            GLfloat xmove, GLfloat ymove,
                            const GLubyte *oldbits)
{
    __GLdlistOp *dlop;
    struct __gllc_Bitmap_Rec *data;
    GLubyte *newbits;
    GLint imageSize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Bitmap(width, height, xorig, yorig, xmove, ymove, oldbits);
    }

    if ((width < 0) || (height < 0))
    {
        __gllc_InvalidValue(gc);
        return;
    }

    imageSize = height * ((width + 7) >> 3);
    imageSize = __GL_PAD(imageSize);

    dlop = __glDlistAllocOp(gc, (imageSize + sizeof(struct __gllc_Bitmap_Rec)));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Bitmap;

    data = (struct __gllc_Bitmap_Rec *)(dlop + 1);
    data->width = width;
    data->height = height;
    data->xorig = xorig;
    data->yorig = yorig;
    data->xmove = xmove;
    data->ymove = ymove;
    data->imageSize = imageSize;

    newbits = (GLubyte *)(data + 1);
    __glFillImage(gc, width, height, GL_COLOR_INDEX, GL_BITMAP, oldbits, newbits);

    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PolygonStipple(const GLubyte *mask)
{
    __GLdlistOp *dlop;
    __GL_SETUP();
    GLubyte *newbits;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PolygonStipple(mask);
    }

    dlop = __glDlistAllocOp(gc, __glImageSize(32, 32, GL_COLOR_INDEX, GL_BITMAP));
    if (dlop == NULL) return;
    dlop->opcode = __glop_PolygonStipple;
    newbits = (GLubyte *)(dlop + 1);
    __glFillImage(gc, 32, 32, GL_COLOR_INDEX, GL_BITMAP, mask, newbits);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Map1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points)
{
    __GLdlistOp *dlop;
    struct __gllc_Map1f_Rec *map1data;
    GLint k;
    GLint cmdsize;
    GLfloat *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Map1f(target, u1, u2, stride, order, points);
    }

    k = __glEvalComputeK(target);
    if (k < 0)
    {
        __gllc_InvalidEnum(gc);
        return;
    }

    if (order > (GLint)gc->constants.maxEvalOrder || stride < k ||
        order < 1 || u1 == u2)
    {
        __gllc_InvalidValue(gc);
        return;
    }

    cmdsize = (GLint)sizeof(*map1data) +
              __glMap1_size(k, order) * (GLint)sizeof(GLfloat);

    dlop = __glDlistAllocOp(gc, cmdsize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_Map1f;

    map1data = (struct __gllc_Map1f_Rec *)(dlop + 1);
    map1data->target = target;
    map1data->u1 = u1;
    map1data->u2 = u2;
    map1data->order = order;
    data = (GLfloat *)(map1data + 1);
    __glFillMap1fInternal(k, order, stride, points, data);

    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Map1d(GLenum target,
                           GLdouble u1, GLdouble u2,
                           GLint stride, GLint order,
                           const GLdouble *points)
{
    __GLdlistOp *dlop;
    struct __gllc_Map1f_Rec *map1data;
    GLint k;
    GLint cmdsize;
    GLfloat *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Map1d(target, u1, u2, stride, order, points);
    }

    k = __glEvalComputeK(target);
    if (k < 0)
    {
        __gllc_InvalidEnum(gc);
        return;
    }

    if (order > (GLint)gc->constants.maxEvalOrder || stride < k ||
        order < 1 || u1 == u2)
    {
        __gllc_InvalidValue(gc);
        return;
    }

    cmdsize = (GLint)sizeof(*map1data) +
              __glMap1_size(k, order) * (GLint)sizeof(GLfloat);

    dlop = __glDlistAllocOp(gc, cmdsize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_Map1d;

    map1data = (struct __gllc_Map1f_Rec *)(dlop + 1);
    map1data->target = target;
    map1data->u1 = u1;
    map1data->u2 = u2;
    map1data->order = order;
    data = (GLfloat *)(map1data + 1);
    __glFillMap1dInternal(k, order, stride, points, data);

    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Map2f(GLenum target,
                           GLfloat u1, GLfloat u2,
                           GLint ustride, GLint uorder,
                           GLfloat v1, GLfloat v2,
                           GLint vstride, GLint vorder,
                           const GLfloat *points)
{
    __GLdlistOp *dlop;
    struct __gllc_Map2f_Rec *map2data;
    GLint k;
    GLint cmdsize;
    GLfloat *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Map2f(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
    }

    k = __glEvalComputeK(target);
    if (k < 0)
    {
        __gllc_InvalidEnum(gc);
        return;
    }

    if (vorder > (GLint)gc->constants.maxEvalOrder || vstride < k ||
        vorder < 1 || u1 == u2 || ustride < k ||
        uorder > (GLint)gc->constants.maxEvalOrder || uorder < 1 ||
        v1 == v2)
    {
        __gllc_InvalidValue(gc);
        return;
    }

    cmdsize = (GLint)sizeof(*map2data) +
              __glMap2_size(k, uorder, vorder) * (GLint)sizeof(GLfloat);

    dlop = __glDlistAllocOp(gc, cmdsize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_Map2f;

    map2data = (struct __gllc_Map2f_Rec *)(dlop + 1);
    map2data->target = target;
    map2data->u1 = u1;
    map2data->u2 = u2;
    map2data->uorder = uorder;
    map2data->v1 = v1;
    map2data->v2 = v2;
    map2data->vorder = vorder;

    data = (GLfloat *)(map2data + 1);
    __glFillMap2fInternal(k, uorder, vorder, ustride, vstride, points, data);

    __glDlistAppendOp(gc, dlop);
}


GLvoid APIENTRY __gllc_Map2d(GLenum target,
                           GLdouble u1, GLdouble u2,
                           GLint ustride, GLint uorder,
                           GLdouble v1, GLdouble v2,
                           GLint vstride, GLint vorder,
                           const GLdouble *points)
{
    __GLdlistOp *dlop;
    struct __gllc_Map2f_Rec *map2data;
    GLint k;
    GLint cmdsize;
    GLfloat *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Map2d(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
    }

    k = __glEvalComputeK(target);
    if (k < 0)
    {
        __gllc_InvalidEnum(gc);
        return;
    }

    if (vorder > (GLint)gc->constants.maxEvalOrder || vstride < k ||
        vorder < 1 || u1 == u2 || ustride < k ||
        uorder > (GLint)gc->constants.maxEvalOrder || uorder < 1 ||
        v1 == v2)
    {
        __gllc_InvalidValue(gc);
        return;
    }

    cmdsize = (GLint)sizeof(*map2data) +
              __glMap2_size(k, uorder, vorder) * (GLint)sizeof(GLfloat);

    dlop = __glDlistAllocOp(gc, cmdsize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_Map2d;

    map2data = (struct __gllc_Map2f_Rec *)(dlop + 1);
    map2data->target = target;
    map2data->u1 = u1;
    map2data->u2 = u2;
    map2data->uorder = uorder;
    map2data->v1 = v1;
    map2data->v2 = v2;
    map2data->vorder = vorder;

    data = (GLfloat *)(map2data + 1);
    __glFillMap2dInternal(k, uorder, vorder, ustride, vstride, points, data);

    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_SampleCoverage(GLclampf v, GLboolean invert)
{
    __GLdlistOp *dlop;
    struct __gllc_SampleCoverage_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_SampleCoverage(v, invert);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_SampleCoverage_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_SampleCoverage;
    data = (struct __gllc_SampleCoverage_Rec *)(dlop + 1);
    data->v = v;
    data->invert = invert;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_DrawPixels(GLint width, GLint height, GLenum format,
                                GLenum type, const GLvoid *pixels)
{
    GLenum adjustFormat= format;
    GLenum adjustType= type;
    __GLdlistOp *dlop;
    struct __gllc_DrawPixels_Rec *data;
    GLint imageSize;
    GLboolean index;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_DrawPixels(width, height, format, type, pixels);
    }

    if ((width < 0) || (height < 0))
    {
        __gllc_InvalidValue(gc);
        return;
    }
    switch (format)
    {
    case GL_STENCIL_INDEX:
    case GL_COLOR_INDEX:
        index = GL_TRUE;
        break;
    case GL_RED:
    case GL_GREEN:
    case GL_BLUE:
    case GL_ALPHA:
    case GL_RGB:
    case GL_RGBA:
    case GL_LUMINANCE:
    case GL_LUMINANCE_ALPHA:
    case GL_DEPTH_COMPONENT:
    case GL_ABGR_EXT:
    case GL_BGR_EXT:
    case GL_BGRA_EXT:
        index = GL_FALSE;
        break;
    default:
        __gllc_InvalidEnum(gc);
        return;
    }
    switch (type)
    {
    case GL_BITMAP:
        if (!index)
        {
            __gllc_InvalidEnum(gc);
            return;
        }
        break;
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    case GL_INT:
    case GL_UNSIGNED_INT:
    case GL_FLOAT:
        break;
    case GL_UNSIGNED_BYTE_3_3_2:
    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_BYTE_2_3_3_REV:
    case GL_UNSIGNED_SHORT_5_6_5_REV:
        switch (format)
        {
        case GL_RGB:
        case GL_BGR_EXT:
            adjustFormat= GL_LUMINANCE; /* or anything that's 1 component */
            adjustType= GL_UNSIGNED_BYTE;
            break;
        default:
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        break;
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_5_5_5_1:
    case GL_UNSIGNED_INT_8_8_8_8:
    case GL_UNSIGNED_INT_10_10_10_2:
    case GL_UNSIGNED_SHORT_4_4_4_4_REV:
    case GL_UNSIGNED_SHORT_1_5_5_5_REV:
    case GL_UNSIGNED_INT_8_8_8_8_REV:
    case GL_UNSIGNED_INT_2_10_10_10_REV:
        switch (format)
        {
        case GL_RGBA:
        case GL_ABGR_EXT:
        case GL_BGRA_EXT:
            adjustFormat= GL_LUMINANCE; /* or anything that's 1 component */
            adjustType= GL_UNSIGNED_SHORT;
            if (type == GL_UNSIGNED_INT_8_8_8_8 ||
                type == GL_UNSIGNED_INT_10_10_10_2 ||
                type == GL_UNSIGNED_INT_8_8_8_8_REV ||
                type == GL_UNSIGNED_INT_2_10_10_10_REV)
                adjustType= GL_UNSIGNED_INT;
            break;
        default:
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        break;
    default:
        __gllc_InvalidEnum(gc);
        return;
    }

    imageSize = __glImageSize(width, height, format, type);
    imageSize = __GL_PAD(imageSize);

    dlop = __glDlistAllocOp(gc, (sizeof(struct __gllc_DrawPixels_Rec) + imageSize));
    if (dlop == NULL) return;
    dlop->opcode = __glop_DrawPixels;

    data = (struct __gllc_DrawPixels_Rec *)(dlop + 1);
    data->width = width;
    data->height = height;
    data->format = format;
    data->type = type;
    data->imageSize = imageSize;

    __glFillImage(gc, width, height, adjustFormat, adjustType, pixels,
                  (GLubyte *)(data + 1));

    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexImage1D(GLenum target, GLint level,
                                GLint components,
                                GLint width, GLint border, GLenum format,
                                GLenum type, const GLvoid *pixels)
{
    GLenum adjustFormat= format;
    GLenum adjustType= type;
    __GLdlistOp *dlop;
    struct __gllc_TexImage1D_Rec *texdata;
    GLint imageSize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexImage1D(target, level, components, width, border, format, type, pixels);
    }
    else
    {
        GLenum oldError = gc->error;
        /*proxy textures are executed immediately*/
        switch (target)
        {
            case GL_PROXY_TEXTURE_1D:
                __glim_TexImage1D(target, level, components, width, border, format, type, pixels);
                return;
            case GL_TEXTURE_1D:
                break;
            default:
                __gllc_InvalidEnum(gc);
                return;
        }
        /*do argument check*/
        if(GL_FALSE == __glCheckTexImageArgs(gc, target,level,
                components, width, 1 + border *2 , 1 + border *2, border, format, type) )
        {
            /*in display list compile mode, no gl error is set*/
            __glSetError(oldError);
        }
    }

    imageSize = __glImageSize(width, 1, format, type);
    imageSize = __GL_PAD(imageSize);

    dlop = __glDlistAllocOp(gc, (sizeof(struct __gllc_TexImage1D_Rec) + imageSize));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexImage1D;

    texdata = (struct __gllc_TexImage1D_Rec *)(dlop + 1);
    texdata->target = target;
    texdata->level = level;
    texdata->components = components;
    texdata->width = width;
    texdata->border = border;
    texdata->format = format;
    texdata->type = type;
    texdata->imageSize = imageSize;

    if (imageSize > 0 && pixels != NULL)
    {
        __glFillImage(gc, width, 1, adjustFormat, adjustType, pixels,
                      (GLubyte *)(texdata + 1));
    }

    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexImage2D(GLenum target, GLint lod,
                                GLint internalFormat,
                                GLint width, GLint height, GLint border,
                                GLenum format, GLenum type,
                                const GLvoid *pixels)
{
    GLenum adjustFormat= format;
    GLenum adjustType= type;
    __GLdlistOp *dlop;
    struct __gllc_TexImage2D_Rec  *texdata;
    GLint imageSize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexImage2D(target, lod, internalFormat, width, height, border, format, type, pixels);
    }
    else
    {
        GLenum oldError = gc->error;
        /*proxy textures are executed immediately*/
        switch (target)
        {
            case GL_PROXY_TEXTURE_2D:
            case GL_PROXY_TEXTURE_CUBE_MAP:
                __glim_TexImage2D(target,lod, internalFormat, width, height, border, format, type, pixels);
                return;
            case GL_TEXTURE_2D:
                break;
            case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
                if(!__glExtension[INDEX_ARB_texture_cube_map].bEnabled && !__glExtension[INDEX_EXT_texture_cube_map].bEnabled)
                {
                    __gllc_InvalidEnum(gc);
                    return;
                }
                break;
            default:
                __gllc_InvalidEnum(gc);
                return;
        }

        /*do argument check*/
        if(GL_FALSE == __glCheckTexImageArgs(gc, target,lod,
                internalFormat, width, height, 1 + border *2, border, format, type) )
        {
            /*in display list compile mode, no gl error is set*/
            __glSetError(oldError);
        }
    }

    imageSize = __glImageSize(width, height, format, type);
    imageSize = __GL_PAD(imageSize);

    dlop = __glDlistAllocOp(gc, (sizeof(struct __gllc_TexImage2D_Rec) + imageSize));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexImage2D;

    texdata = (struct __gllc_TexImage2D_Rec *)(dlop + 1);
    texdata->target = target;
    texdata->level = lod;
    texdata->components = internalFormat;
    texdata->width = width;
    texdata->height = height;
    texdata->border = border;
    texdata->format = format;
    texdata->type = type;
    texdata->imageSize = imageSize;

    if (imageSize > 0 && pixels != NULL)
    {
        __glFillImage(gc, width, height, adjustFormat, adjustType, pixels,
                      (GLubyte *)(texdata + 1));
    }

    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexImage3D(GLenum target,
                                GLint lod,
                                GLint components,
                                GLsizei width,
                                GLsizei height,
                                GLsizei depth,
                                GLint border,
                                GLenum format,
                                GLenum type,
                                const GLvoid *buf)
{
    GLenum adjust_format, adjust_type;
    __GLdlistOp *dlop;
    struct __gllc_TexImage3D_Rec  *texdata;
    GLint imageSize;
    __GL_SETUP();

    adjust_format = format;
    adjust_type = type;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexImage3D(target, lod, components, width, height, depth, border, format, type, buf);
    }
    else
    {
        GLenum oldError = gc->error;
        /*proxy textures are executed immediately*/
        switch (target)
        {
            case GL_PROXY_TEXTURE_3D:
                __glim_TexImage3D(target, lod, components, width, height, depth, border, format, type, buf);
                return;
            case GL_TEXTURE_3D:
                break;
            default:
                __gllc_InvalidEnum(gc);
                return;
        }
        /*do argument check*/
        if(GL_FALSE == __glCheckTexImageArgs(gc, target,lod,
                components, width, height, depth, border, format, type) )
        {
            /*in display list compile mode, no gl error is set*/
            __glSetError(oldError);
        }
    }


    if(!__glTexImagCopyInfo(gc, format, type, &adjust_format, &adjust_type))
    return;

    imageSize = __glImageSize3D(width, height, depth, format, type);
    imageSize = __GL_PAD(imageSize);

    dlop = __glDlistAllocOp(gc, (sizeof(struct __gllc_TexImage3D_Rec) + imageSize));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexImage3D;

    texdata = (struct __gllc_TexImage3D_Rec *)(dlop + 1);
    texdata->target = target;
    texdata->lod = lod;
    texdata->components = components;
    texdata->width = width;
    texdata->height = height;
    texdata->depth = depth;
    texdata->border = border;
    texdata->format = format;
    texdata->type = type;
    texdata->imageSize = imageSize;

    if (imageSize > 0 && buf != NULL)
    {
        __glFillImage3D(gc, width, height, depth, adjust_format, adjust_type, buf,
                        (GLubyte *)(texdata + 1));
    }

    __glDlistAppendOp(gc, dlop);

}

GLvoid APIENTRY __gllc_CopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
    __GLdlistOp *dlop;
    struct __gllc_CopyTexImage1D_Rec *data;
    GLenum format;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_CopyTexImage1D(target, level, internalformat, x, y, width, border);
    }
    else
    {
        GLenum oldError = gc->error;
        switch (target)
        {
            case GL_TEXTURE_1D:
                break;
            default:
                __gllc_InvalidEnum(gc);
                return;
        }
        /* get "format"*/
        switch (internalformat)
        {
            case GL_DEPTH_COMPONENT:
            case GL_DEPTH_COMPONENT16:
            case GL_DEPTH_COMPONENT24:
            case GL_DEPTH_COMPONENT32:
                format = GL_DEPTH_COMPONENT;
                if(!gc->modes.haveDepthBuffer)
                {
                    __gllc_InvalidOperation(gc);
                    return;
                }
                break;
            default:
                format = GL_RGBA;
                break;
        }
        /*do argument check*/
        __glCheckTexImageArgs(gc, target,level,
                internalformat, width, 1 + border *2, 1 + border *2, border, format, GL_FLOAT);

        /*in display list compile mode, no gl error is set*/
        __glSetError(oldError);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_CopyTexImage1D_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_CopyTexImage1D;
    data = (struct __gllc_CopyTexImage1D_Rec *)(dlop + 1);
    data->target = target;
    data->level = level;
    data->internalformat = internalformat;
    data->x = x;
    data->y = y;
    data->width = width;
    data->border = border;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_CopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    __GLdlistOp *dlop;
    struct __gllc_CopyTexImage2D_Rec *data;
    GLenum format;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_CopyTexImage2D(target, level, internalformat, x, y, width, height, border);
    }
    else
    {
        GLenum oldError = gc->error;
        switch (target)
        {
            case GL_TEXTURE_2D:
                break;
            case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
                if(!__glExtension[INDEX_ARB_texture_cube_map].bEnabled && !__glExtension[INDEX_EXT_texture_cube_map].bEnabled)
                {
                    __gllc_InvalidEnum(gc);
                    return;
                }
                break;
            default:
                __gllc_InvalidEnum(gc);
                return;
        }
        /* get "format"*/
        switch (internalformat)
        {
            case GL_DEPTH_COMPONENT:
            case GL_DEPTH_COMPONENT16:
            case GL_DEPTH_COMPONENT24:
            case GL_DEPTH_COMPONENT32:
                format = GL_DEPTH_COMPONENT;
                if(!gc->modes.haveDepthBuffer)
                {
                    __gllc_InvalidOperation(gc);
                    return;
                }
                break;
            default:
                format = GL_RGBA;
                break;
        }
        /*do argument check*/
        __glCheckTexImageArgs(gc, target,level,
                internalformat, width, height, 1 + border *2, border, format, GL_FLOAT);

        /*in display list compile mode, no gl error is set*/
        __glSetError(oldError);

    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_CopyTexImage2D_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_CopyTexImage2D;
    data = (struct __gllc_CopyTexImage2D_Rec *)(dlop + 1);
    data->target = target;
    data->level = level;
    data->internalformat = internalformat;
    data->x = x;
    data->y = y;
    data->width = width;
    data->height = height;
    data->border = border;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexSubImage1D(GLenum target, GLint level,
                                   GLint xoffset, GLsizei width,
                                   GLenum format, GLenum type, const GLvoid *pixels)
{
    GLenum adjustFormat= format;
    GLenum adjustType= type;
    __GLdlistOp *dlop;
    struct __gllc_TexSubImage1D_Rec *texdata;
    GLint imageSize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexSubImage1D(target, level, xoffset, width, format, type, pixels);
    }
    else
    {
        GLenum oldError = gc->error;
        __GLtextureObject *tex;
        GLuint activeUnit = gc->state.texture.activeTexIndex;
        switch (target)
        {
            case GL_TEXTURE_1D:
                tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_1D_INDEX];
                break;
            default:
                __gllc_InvalidEnum(gc);
                return;
        }
        __glCheckTexSubImageArgs(gc, tex, 0, level,
                xoffset, 0, 0, width, 1, 1, format, type);
        __glSetError(oldError);
    }

    imageSize = __glImageSize(width, 1, format, type);
    imageSize = __GL_PAD(imageSize);

    dlop = __glDlistAllocOp(gc, (sizeof(struct __gllc_TexSubImage1D_Rec) + imageSize));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexSubImage1D;

    texdata = (struct __gllc_TexSubImage1D_Rec *)(dlop + 1);
    texdata->target = target;
    texdata->level = level;
    texdata->xoffset = xoffset;
    texdata->width = width;
    texdata->format = format;
    texdata->type = type;
    texdata->imageSize = imageSize;

    if (imageSize > 0)
    {
        __glFillImage(gc, width, 1, adjustFormat, adjustType, pixels,
                      (GLubyte *)(texdata + 1));
    }

    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexSubImage2D(GLenum target, GLint level,
                                   GLint xoffset, GLint yoffset,
                                   GLsizei width, GLsizei height,
                                   GLenum format, GLenum type, const GLvoid *pixels)
{
    GLenum adjustFormat= format;
    GLenum adjustType= type;
    __GLdlistOp *dlop;
    struct __gllc_TexSubImage2D_Rec *texdata;
    GLint imageSize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
    }
    else
    {
        GLenum oldError = gc->error;
        __GLtextureObject *tex;
        GLuint activeUnit = gc->state.texture.activeTexIndex;
        GLuint face;
        switch (target)
        {
            case GL_TEXTURE_2D:
                face = 0;
                tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_INDEX];
                break;
            case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
                if(!__glExtension[INDEX_ARB_texture_cube_map].bEnabled && !__glExtension[INDEX_EXT_texture_cube_map].bEnabled)
                {
                    __gllc_InvalidEnum(gc);
                    return;
                }
                face = target - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
                tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_CUBEMAP_INDEX];
                break;
            default:
                __gllc_InvalidEnum(gc);
                return;
        }
        __glCheckTexSubImageArgs(gc, tex, face, level,
                xoffset, yoffset, 0, width, height, 1, format, type);
        __glSetError(oldError);
    }

    imageSize = __glImageSize(width, height, format, type);
    imageSize = __GL_PAD(imageSize);

    dlop = __glDlistAllocOp(gc, (sizeof(struct __gllc_TexSubImage2D_Rec) + imageSize));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexSubImage2D;

    texdata = (struct __gllc_TexSubImage2D_Rec *)(dlop + 1);
    texdata->target = target;
    texdata->level = level;
    texdata->xoffset = xoffset;
    texdata->yoffset = yoffset;
    texdata->width = width;
    texdata->height = height;
    texdata->format = format;
    texdata->type = type;
    texdata->imageSize = imageSize;

    if (imageSize > 0)
    {
        __glFillImage(gc, width, height, adjustFormat, adjustType, pixels,
                      (GLubyte *)(texdata + 1));
    }

    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexSubImage3D(GLenum target,
                                   GLint lod,
                                   GLint xoffset,
                                   GLint yoffset,
                                   GLint zoffset,
                                   GLsizei width,
                                   GLsizei height,
                                   GLsizei depth,
                                   GLenum format,
                                   GLenum type,
                                   const GLvoid *buf)
{
    GLenum adjust_format, adjust_type;
    __GLdlistOp *dlop;
    struct __gllc_TexSubImage3D_Rec *texdata;
    GLint imageSize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexSubImage3D(target, lod, xoffset, yoffset, zoffset, width, height, depth, format, type, buf);
    }
    else
    {
        GLenum oldError = gc->error;
        __GLtextureObject *tex;
        GLuint activeUnit = gc->state.texture.activeTexIndex;
        switch (target)
        {
            case GL_TEXTURE_3D:
                tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_3D_INDEX];
                break;
            default:
                __gllc_InvalidEnum(gc);
                return;
        }
        __glCheckTexSubImageArgs(gc, tex, 0, lod,
                xoffset, yoffset, zoffset, width, height, depth, format, type);
        __glSetError(oldError);
    }

    adjust_format = format;
    adjust_type = type;

    if(!__glTexImagCopyInfo(gc, format, type, &adjust_format, &adjust_type))
    return;

    imageSize = __glImageSize3D(width, height, depth, format, type);
    imageSize = __GL_PAD(imageSize);

    dlop = __glDlistAllocOp(gc, (sizeof(struct __gllc_TexSubImage3D_Rec) + imageSize));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexSubImage3D;

    texdata = (struct __gllc_TexSubImage3D_Rec *)(dlop + 1);
    texdata->target = target;
    texdata->lod = lod;
    texdata->xoffset = xoffset;
    texdata->yoffset = yoffset;
    texdata->zoffset = zoffset;
    texdata->width = width;
    texdata->height = height;
    texdata->depth = depth;
    texdata->format = format;
    texdata->type = type;
    texdata->imageSize = imageSize;

    if (imageSize > 0)
    {
        __glFillImage3D(gc, width, height, depth, adjust_format, adjust_type, buf,
                        (GLubyte *)(texdata + 1));
    }

    __glDlistAppendOp(gc, dlop);

}

GLvoid APIENTRY __gllc_CopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
    __GLdlistOp *dlop;
    struct __gllc_CopyTexSubImage1D_Rec *data;
    GLint max_lod;

    __GL_SETUP();

    max_lod = (GLint)(gc->constants.maxNumTextureLevels - 1);

    /* check lod */
    /*
    ** because switch (mipmap->requestedFormat) use the level and change the format
    ** so we do check here outside the __glCheckTexSubImageArgs
    */
    if (level < 0 || level > max_lod )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_CopyTexSubImage1D(target, level, xoffset, x, y, width);
    }
    else
    {
        GLenum oldError = gc->error;
        GLenum format;
        __GLtextureObject *tex;
        __GLmipMapLevel *mipmap;
        GLuint activeUnit = gc->state.texture.activeTexIndex;
        switch (target)
        {
            case GL_TEXTURE_1D:
                tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_1D_INDEX];
                break;
            default:
                __gllc_InvalidEnum(gc);
                return;
        }
        mipmap = &tex->faceMipmap[0][level];
        /* get "format"*/
        switch (mipmap->requestedFormat)
        {
            case GL_DEPTH_COMPONENT:
            case GL_DEPTH_COMPONENT16:
            case GL_DEPTH_COMPONENT24:
            case GL_DEPTH_COMPONENT32:
                format = GL_DEPTH_COMPONENT;
                if(!gc->modes.haveDepthBuffer)
                {
                    __gllc_InvalidOperation(gc);
                    return;
                }
                break;
            default:
                format = GL_RGBA;
                break;
        }
        __glCheckTexSubImageArgs(gc, tex, 0, level,
                xoffset, 0, 0, width, 1, 1, format, GL_FLOAT);
        __glSetError(oldError);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_CopyTexSubImage1D_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_CopyTexSubImage1D;
    data = (struct __gllc_CopyTexSubImage1D_Rec *)(dlop + 1);
    data->target = target;
    data->level = level;
    data->xoffset = xoffset;
    data->x = x;
    data->y = y;
    data->width = width;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_CopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GLdlistOp *dlop;
    struct __gllc_CopyTexSubImage2D_Rec *data;
    GLint max_lod;

    __GL_SETUP();

    max_lod = (GLint)(gc->constants.maxNumTextureLevels - 1);

    /* check lod */
    /*
    ** because switch (mipmap->requestedFormat) use the level and change the format
    ** so we do check here outside the __glCheckTexSubImageArgs
    */
    if (level < 0 || level > max_lod )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_CopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
    }
    else
    {
        GLenum oldError = gc->error;
        GLenum format;
        __GLtextureObject *tex;
        __GLmipMapLevel *mipmap;
        GLuint activeUnit = gc->state.texture.activeTexIndex;
        GLuint face;
        switch (target)
        {
            case GL_TEXTURE_2D:
                face = 0;
                tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_INDEX];
                break;
            case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
                if(!__glExtension[INDEX_ARB_texture_cube_map].bEnabled && !__glExtension[INDEX_EXT_texture_cube_map].bEnabled)
                {
                    __gllc_InvalidEnum(gc);
                    return;
                }
                face = target - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
                tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_CUBEMAP_INDEX];
                break;
            default:
                __gllc_InvalidEnum(gc);
                return;
        }
        mipmap = &tex->faceMipmap[face][level];
        /* get "format"*/
        switch (mipmap->requestedFormat)
        {
            case GL_DEPTH_COMPONENT:
            case GL_DEPTH_COMPONENT16:
            case GL_DEPTH_COMPONENT24:
            case GL_DEPTH_COMPONENT32:
                format = GL_DEPTH_COMPONENT;
                if(!gc->modes.haveDepthBuffer)
                {
                    __gllc_InvalidOperation(gc);
                    return;
                }
                break;
            default:
                format = GL_RGBA;
                break;
        }
        __glCheckTexSubImageArgs(gc, tex, face, level,
                xoffset, yoffset, 0, width, height, 1, format, GL_FLOAT);
        __glSetError(oldError);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_CopyTexSubImage2D_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_CopyTexSubImage2D;
    data = (struct __gllc_CopyTexSubImage2D_Rec *)(dlop + 1);
    data->target = target;
    data->level = level;
    data->xoffset = xoffset;
    data->yoffset = yoffset;
    data->x = x;
    data->y = y;
    data->width = width;
    data->height = height;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_CopyTexSubImage3D(GLenum target,
                                       GLint level,
                                       GLint xoffset,
                                       GLint yoffset,
                                       GLint zoffset,
                                       GLint x,
                                       GLint y,
                                       GLsizei width,
                                       GLsizei height)
{
    __GLdlistOp *dlop;
    struct __gllc_CopyTexSubImage3D_Rec *data;
    GLint max_lod;

    __GL_SETUP();

    max_lod = (GLint)(gc->constants.maxNumTextureLevels - 1);

    /* check lod */
    /*
    ** because switch (mipmap->requestedFormat) use the level and change the format
    ** so we do check here outside the __glCheckTexSubImageArgs
    */
    if (level < 0 || level > max_lod )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_CopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
    }
    else
    {
        GLenum oldError = gc->error;
        GLenum format;
        __GLtextureObject *tex;
        __GLmipMapLevel *mipmap;
        GLuint activeUnit = gc->state.texture.activeTexIndex;
        switch (target)
        {
            case GL_TEXTURE_3D:
                tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_3D_INDEX];
                break;
            default:
                __gllc_InvalidEnum(gc);
                return;
        }
        mipmap = &tex->faceMipmap[0][level];
        /* get "format"*/
        switch (mipmap->requestedFormat)
        {
            case GL_DEPTH_COMPONENT:
            case GL_DEPTH_COMPONENT16:
            case GL_DEPTH_COMPONENT24:
            case GL_DEPTH_COMPONENT32:
                format = GL_DEPTH_COMPONENT;
                if(!gc->modes.haveDepthBuffer)
                {
                    __gllc_InvalidOperation(gc);
                    return;
                }
                break;
            default:
                format = GL_RGBA;
                break;
        }
        __glCheckTexSubImageArgs(gc, tex, 0, level,
                xoffset, yoffset, 0, width, height, 1, format, GL_FLOAT);
        __glSetError(oldError);

    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_CopyTexSubImage3D_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_CopyTexSubImage3D;
    data = (struct __gllc_CopyTexSubImage3D_Rec *)(dlop + 1);
    data->target = target;
    data->level = level;
    data->xoffset = xoffset;
    data->yoffset = yoffset;
    data->zoffset = zoffset;
    data->x = x;
    data->y = y;
    data->width = width;
    data->height = height;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_CompressedTexImage3D(GLenum target, GLint level,
                                          GLenum internalformat, GLsizei width,
                                          GLsizei height, GLsizei depth,
                                          GLint border, GLsizei imageSize,
                                          const GLvoid *data)
{
    __GL_SETUP();
    // this implementation does not support 3D textures. (yet)
    __gllc_InvalidOperation(gc);
    return;
}

GLvoid APIENTRY __gllc_CompressedTexImage2D(GLenum target, GLint lod,
                                          GLenum components, GLsizei width,
                                          GLsizei height, GLint border,
                                          GLsizei imageSize, const GLvoid *data)
{
    GLint iExpectedSize = 0;
    __GLdlistOp *dlop;
    struct __gllc_CompressedTexImage2D_Rec *texdata;
    GLint blockSize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_CompressedTexImage2D(target, lod, components, width, height, border, imageSize, data);
    }
    else
    {
         GLenum oldError = gc->error;
        /*proxy textures are executed immediately*/
        switch (target)
        {
            case GL_PROXY_TEXTURE_2D:
            case GL_PROXY_TEXTURE_CUBE_MAP:
                __glim_CompressedTexImage2D(target, lod,
                                    components, width,
                                    height, border,
                                    imageSize, data);
                return;
            case GL_TEXTURE_2D:
                break;
            case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
                if(!__glExtension[INDEX_ARB_texture_cube_map].bEnabled && !__glExtension[INDEX_EXT_texture_cube_map].bEnabled)
                {
                    __gllc_InvalidEnum(gc);
                    return;
                }
                break;
            default:
                __gllc_InvalidEnum(gc);
                return;
        }

        switch(components)
        {
            case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
            case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
            case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
            case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
            case GL_COMPRESSED_RED_RGTC1_EXT:
            case GL_COMPRESSED_SIGNED_RED_RGTC1_EXT:
                blockSize = 8;
                break;

            case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
            case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
            case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
            case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
            case GL_COMPRESSED_RED_GREEN_RGTC2_EXT:
            case GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT:

                blockSize = 16;
                break;

            default:
                __gllc_InvalidEnum(gc);
                return;
        }

        iExpectedSize = ((width + 3)/4) * ((height + 3)/4) * blockSize;
        if (imageSize !=  iExpectedSize || border != 0)
        {
            __gllc_InvalidValue(gc);
            return;
        }

        /*do argument check*/
        if(GL_FALSE == __glCheckTexImageArgs(gc, target,lod,
                components, width, height, 1 + border *2, border, GL_RGBA, GL_FLOAT) )
        {
            /*in display list compile mode, no gl error is set*/
            __glSetError(oldError);
        }
    }

    dlop = __glDlistAllocOp(gc, (sizeof(struct __gllc_CompressedTexImage2D_Rec) + iExpectedSize));
    if (dlop == NULL) return;
    dlop->opcode = __glop_CompressedTexImage2D;
    texdata = (struct __gllc_CompressedTexImage2D_Rec *)(dlop + 1);
    texdata->target     = target;
    texdata->lod        = lod;
    texdata->components = components;
    texdata->width      = width;
    texdata->height     = height;
    texdata->border     = border;
    texdata->imageSize  = iExpectedSize;

    if (iExpectedSize > 0 && data != NULL)
    {
        __GL_MEMCOPY((GLubyte *)(texdata + 1), data, iExpectedSize);
    }

    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_CompressedTexImage1D(GLenum target, GLint level,
                                          GLenum internalformat, GLsizei width,
                                          GLint border, GLsizei imageSize,
                                          const GLvoid *data)
{
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
//        __glim_CompressedTexImage1D(target, level, components, width, height, border, imageSize, data);
    }

    // this implementation does not support compressed 1D textures.
    __gllc_InvalidOperation(gc);
    return;
}

GLvoid APIENTRY __gllc_CompressedTexSubImage3D(GLenum target, GLint level,
                                             GLint xoffset, GLint yoffset,
                                             GLint zoffset, GLsizei width,
                                             GLsizei height, GLsizei depth,
                                             GLenum format, GLsizei imageSize,
                                             const GLvoid *data)
{
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
//        __glim_CompressedTexImage3D(target, lod, components, width, height, border, imageSize, data);
    }

    // this implementation does not support 3D textures. (yet)
    __gllc_InvalidOperation(gc);
    return;
}

GLvoid APIENTRY __gllc_CompressedTexSubImage2D(GLenum target, GLint lod,
                                             GLint xoffset, GLint yoffset,
                                             GLsizei width, GLsizei height,
                                             GLenum format, GLsizei imageSize,
                                             const GLvoid *data)
{
    GLint        iExpectedSize = 0, blockSize;
    __GLdlistOp *dlop;
    struct __gllc_CompressedTexSubImage2D_Rec *texdata;
    GLuint face;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_CompressedTexSubImage2D(target, lod, xoffset, yoffset, width, height, format, imageSize, data);
    }
    else
    {
        GLenum oldError = gc->error;
        __GLtextureObject *tex;
        GLuint activeUnit = gc->state.texture.activeTexIndex;
        switch (target)
        {
            case GL_TEXTURE_2D:
                face = 0;
                tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_INDEX];
                break;
            case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
                if(!__glExtension[INDEX_ARB_texture_cube_map].bEnabled && !__glExtension[INDEX_EXT_texture_cube_map].bEnabled)
                {
                    __gllc_InvalidEnum(gc);
                    return;
                }
                face = target - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
                tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_CUBEMAP_INDEX];
                break;
            default:
                __gllc_InvalidEnum(gc);
                return;
        }
        switch(format)
        {
            case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
            case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
            case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
            case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
            case GL_COMPRESSED_RED_RGTC1_EXT:
            case GL_COMPRESSED_SIGNED_RED_RGTC1_EXT:
                blockSize = 8;
                break;

            case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
            case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
            case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
            case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
            case GL_COMPRESSED_RED_GREEN_RGTC2_EXT:
            case GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT:
                blockSize = 16;
                break;
            default:
                __gllc_InvalidEnum(gc);
                return;
        }

        iExpectedSize = ((width + 3)/4) * ((height + 3)/4) * blockSize;
        if (imageSize !=  iExpectedSize )
        {
            __gllc_InvalidValue( gc);
            return;
        }
        __glCheckTexSubImageArgs(gc, tex, face, lod,
                xoffset, yoffset, 0, width, height, 1, GL_RGBA, GL_FLOAT);
        __glSetError(oldError);

    }

    dlop = __glDlistAllocOp(gc, (sizeof(struct __gllc_CompressedTexSubImage2D_Rec) + iExpectedSize));
    if (dlop == NULL) return;
    dlop->opcode = __glop_CompressedTexSubImage2D;

    texdata = (struct __gllc_CompressedTexSubImage2D_Rec *)(dlop + 1);
    texdata->target    = target;
    texdata->lod       = lod;
    texdata->xoffset   = xoffset;
    texdata->yoffset   = yoffset;
    texdata->width     = width;
    texdata->height    = height;
    texdata->format    = format;
    texdata->imageSize = imageSize;

    if (imageSize > 0 && data != NULL)
    {
        __GL_MEMCOPY((GLubyte *)(texdata + 1), data, iExpectedSize);
    }

    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_CompressedTexSubImage1D(GLenum target, GLint level,
                                             GLint xoffset, GLsizei width,
                                             GLenum format, GLsizei imageSize,
                                             const GLvoid *data)
{
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
//        __glim_CompressedTexImage2D(target, lod, components, width, height, border, imageSize, data);
    }

    // this implementation does not support compressed 1D textures.
    __gllc_InvalidOperation(gc);

    return;
}

GLvoid APIENTRY __gllc_Disable(GLenum cap)
{
    __GLdlistOp *dlop;
    struct __gllc_Disable_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Disable(cap);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Disable_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Disable;
    data = (struct __gllc_Disable_Rec *)(dlop + 1);
    data->cap = cap;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Enable(GLenum cap)
{
    __GLdlistOp *dlop;
    struct __gllc_Enable_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Enable(cap);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Enable_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Enable;
    data = (struct __gllc_Enable_Rec *)(dlop + 1);
    data->cap = cap;
    __glDlistAppendOp(gc, dlop);
}
GLvoid APIENTRY __gllc_ColorTable(GLenum target, GLenum internalformat,
                GLsizei width, GLenum format, GLenum type, const GLvoid *table)
{
    __GLdlistOp *dlop;
    struct __gllc_ColorTable_Rec *data;
    GLint imageSize;
    GLenum rvalue;
    GLubyte *tableContents;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ColorTable(target, internalformat, width, format, type, table);
    }

    switch (target)
    {
    case GL_PROXY_COLOR_TABLE:
    case GL_PROXY_POST_CONVOLUTION_COLOR_TABLE:
    case GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE:
        __glim_ColorTable(target, internalformat,
            width, format, type, table);
        return;
    }

    if ((rvalue = __glCheckColorTableArgs(gc, target, internalformat, width,
        format, type)))
    {
        switch (rvalue)
        {
        case GL_INVALID_ENUM:
            __gllc_InvalidEnum(gc);
            return;
        case GL_INVALID_VALUE:
            __gllc_InvalidEnum(gc);
            return;
        }
    }
    imageSize = __glImageSize(width, 1, format, type);
    imageSize = __GL_PAD(imageSize);

    dlop = __glDlistAllocOp(gc, (imageSize + sizeof(struct __gllc_ColorTable_Rec)));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ColorTable;
    data = (struct __gllc_ColorTable_Rec *)(dlop + 1);

    data->target = target;
    data->internalformat = internalformat;
    data->width = width;
    data->format = format;
    data->type = type;
    data->imageSize = imageSize;

    tableContents = (GLubyte *)(data + 1);
    __glFillImage(gc, width, 1, format, type, table, tableContents);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ColorSubTable(GLenum target, GLsizei start,
            GLsizei count, GLenum format, GLenum type, const GLvoid *table)
{
    __GLdlistOp *dlop;
    struct __gllc_ColorSubTable_Rec *data;
    GLint imageSize;
    GLubyte *tableContents;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ColorTable(target, start, count, format, type, table);
    }

    if (__glCheckColorSubTableArgs(gc, target, start, count, format, type))
    {
        __gllc_InvalidEnum(gc);
        return;
    }
    imageSize = __glImageSize(count, 1, format, type);
    imageSize = __GL_PAD(imageSize);

    dlop = __glDlistAllocOp(gc, (imageSize + sizeof(struct __gllc_ColorSubTable_Rec)));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ColorSubTable;
    data = (struct __gllc_ColorSubTable_Rec *)(dlop + 1);

    data->target = target;
    data->start = start;
    data->count = count;
    data->format = format;
    data->type = type;
    data->imageSize = imageSize;

    tableContents = (GLubyte *)(data + 1);
    __glFillImage(gc, count, 1, format, type, table, tableContents);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_CopyColorTable(GLenum target, GLenum internalFormat, GLint x, GLint y, GLsizei width)
{
    __GLdlistOp *dlop;
    struct __gllc_CopyColorTable_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_CopyColorTable(target, internalFormat, x, y, width);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_CopyColorTable_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_CopyColorTable;
    data = (struct __gllc_CopyColorTable_Rec *)(dlop + 1);
    data->target = target;
    data->internalformat = internalFormat;
    data->x = x;
    data->y = y;
    data->width = width;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_CopyColorSubTable(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width)
{
    __GLdlistOp *dlop;
    struct __gllc_CopyColorSubTable_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_CopyColorSubTable(target, start, x, y, width);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_CopyColorSubTable_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_CopyColorSubTable;
    data = (struct __gllc_CopyColorSubTable_Rec *)(dlop + 1);
    data->target = target;
    data->start = start;
    data->x = x;
    data->y = y;
    data->width = width;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ConvolutionFilter1D(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image)
{
    GLenum adjustFormat= format;
    GLenum adjustType= type;
    __GLdlistOp *dlop;
    struct __gllc_ConvolutionFilter1D_Rec *data;
    GLint imageSize;
    GLboolean index;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ConvolutionFilter1D(target, internalformat, width, format, type, image);
    }

    if (width < 0)
    {
        __gllc_InvalidValue(gc);
        return;
    }
    switch (format)
    {
    case GL_COLOR_INDEX:
        index = GL_TRUE;
        break;
    case GL_RED:
    case GL_GREEN:
    case GL_BLUE:
    case GL_ALPHA:
    case GL_RGB:
    case GL_RGBA:
    case GL_LUMINANCE:
    case GL_LUMINANCE_ALPHA:
    case GL_ABGR_EXT:
    case GL_BGR_EXT:
    case GL_BGRA_EXT:
        index = GL_FALSE;
        break;
    default:
        __gllc_InvalidEnum(gc);
        return;
    }
    switch (type)
    {
    case GL_BITMAP:
        if (!index)
        {
            __gllc_InvalidEnum(gc);
            return;
        }
        break;
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    case GL_INT:
    case GL_UNSIGNED_INT:
    case GL_FLOAT:
        break;
    case GL_UNSIGNED_BYTE_3_3_2:
    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_BYTE_2_3_3_REV:
    case GL_UNSIGNED_SHORT_5_6_5_REV:
        switch (format)
        {
        case GL_RGB:
        case GL_BGR_EXT:
            adjustFormat= GL_LUMINANCE; /* or anything that's 1 component */
            adjustType= GL_UNSIGNED_BYTE;
            break;
        default:
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        break;
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_5_5_5_1:
    case GL_UNSIGNED_INT_8_8_8_8:
    case GL_UNSIGNED_INT_10_10_10_2:
    case GL_UNSIGNED_SHORT_4_4_4_4_REV:
    case GL_UNSIGNED_SHORT_1_5_5_5_REV:
    case GL_UNSIGNED_INT_8_8_8_8_REV:
    case GL_UNSIGNED_INT_2_10_10_10_REV:
        switch (format)
        {
        case GL_RGBA:
        case GL_ABGR_EXT:
        case GL_BGRA_EXT:
            adjustFormat= GL_LUMINANCE; /* or anything that's 1 component */
            adjustType= GL_UNSIGNED_SHORT;
            if (type == GL_UNSIGNED_INT_8_8_8_8 ||
                type == GL_UNSIGNED_INT_10_10_10_2 ||
                type == GL_UNSIGNED_INT_8_8_8_8_REV ||
                type == GL_UNSIGNED_INT_2_10_10_10_REV)
                adjustType= GL_UNSIGNED_INT;
            break;
        default:
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        break;
    default:
        __gllc_InvalidEnum(gc);
        return;
    }

    imageSize = __glImageSize(width, 1, format, type);
    imageSize = __GL_PAD(imageSize);

    dlop = __glDlistAllocOp(gc, (sizeof(struct __gllc_ConvolutionFilter1D_Rec) + imageSize));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexImage1D;

    data = (struct __gllc_ConvolutionFilter1D_Rec *)(dlop + 1);
    data->target = target;
    data->internalformat = internalformat;
    data->width = width;
    data->format = format;
    data->type = type;
    data->imageSize = imageSize;

    if (imageSize > 0 && image != NULL)
    {
        __glFillImage(gc, width, 1, adjustFormat, adjustType, image, (GLubyte*)(data + 1));
    }

    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ConvolutionFilter2D(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image)
{
    GLenum adjustFormat= format;
    GLenum adjustType= type;
    __GLdlistOp *dlop;
    struct __gllc_ConvolutionFilter2D_Rec  *data;
    GLint imageSize;
    GLboolean index;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ConvolutionFilter2D(target, internalformat, width, height, format, type, image);
    }

    if ((width < 0) || (height < 0))
    {
        __gllc_InvalidValue(gc);
        return;
    }
    switch (format)
    {
    case GL_COLOR_INDEX:
        index = GL_TRUE;
        break;
    case GL_RED:
    case GL_GREEN:
    case GL_BLUE:
    case GL_ALPHA:
    case GL_RGB:
    case GL_RGBA:
    case GL_LUMINANCE:
    case GL_LUMINANCE_ALPHA:
    case GL_ABGR_EXT:
    case GL_BGR_EXT:
    case GL_BGRA_EXT:
        index = GL_FALSE;
        break;
    default:
        __gllc_InvalidEnum(gc);
        return;
    }
    switch (type)
    {
    case GL_BITMAP:
        if (!index)
        {
            __gllc_InvalidEnum(gc);
            return;
        }
        break;
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    case GL_INT:
    case GL_UNSIGNED_INT:
    case GL_FLOAT:
        break;
    case GL_UNSIGNED_BYTE_3_3_2:
    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_BYTE_2_3_3_REV:
    case GL_UNSIGNED_SHORT_5_6_5_REV:
        switch (format)
        {
        case GL_RGB:
        case GL_BGR_EXT:
            adjustFormat= GL_LUMINANCE; /* or anything that's 1 component */
            adjustType= GL_UNSIGNED_BYTE;
            break;
        default:
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        break;
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_5_5_5_1:
    case GL_UNSIGNED_INT_8_8_8_8:
    case GL_UNSIGNED_INT_10_10_10_2:
    case GL_UNSIGNED_SHORT_4_4_4_4_REV:
    case GL_UNSIGNED_SHORT_1_5_5_5_REV:
    case GL_UNSIGNED_INT_8_8_8_8_REV:
    case GL_UNSIGNED_INT_2_10_10_10_REV:
        switch (format)
        {
        case GL_RGBA:
        case GL_ABGR_EXT:
        case GL_BGRA_EXT:
            adjustFormat= GL_LUMINANCE; /* or anything that's 1 component */
            adjustType= GL_UNSIGNED_SHORT;
            if (type == GL_UNSIGNED_INT_8_8_8_8 ||
                type == GL_UNSIGNED_INT_10_10_10_2 ||
                type == GL_UNSIGNED_INT_8_8_8_8_REV ||
                type == GL_UNSIGNED_INT_2_10_10_10_REV)
                adjustType= GL_UNSIGNED_INT;
            break;
        default:
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        break;
    default:
        __gllc_InvalidEnum(gc);
        return;
    }

    imageSize = __glImageSize(width, height, format, type);
    imageSize = __GL_PAD(imageSize);

    dlop = __glDlistAllocOp(gc, (sizeof(struct __gllc_ConvolutionFilter2D_Rec) + imageSize));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ConvolutionFilter2D;

    data = (struct __gllc_ConvolutionFilter2D_Rec *)(dlop + 1);
    data->target = target;
    data->internalformat = internalformat;
    data->width = width;
    data->height = height;
    data->format = format;
    data->type = type;
    data->imageSize = imageSize;

    if (imageSize > 0 && image != NULL)
    {
        __glFillImage(gc, width, height, adjustFormat, adjustType, image, (GLubyte*)(data + 1));
    }

    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_SeparableFilter2D(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *col)
{
    GLenum adjustFormat= format;
    GLenum adjustType= type;
    __GLdlistOp *dlop;
    struct __gllc_SeparableFilter2D_Rec  *data;
    GLint rowSize, colSize, imageSize;
    GLboolean index;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_SeparableFilter2D(target, internalformat, width, height, format, type, row, col);
    }

    if ((width < 0) || (height < 0))
    {
        __gllc_InvalidValue(gc);
        return;
    }
    switch (format)
    {
    case GL_COLOR_INDEX:
        index = GL_TRUE;
        break;
    case GL_RED:
    case GL_GREEN:
    case GL_BLUE:
    case GL_ALPHA:
    case GL_RGB:
    case GL_RGBA:
    case GL_LUMINANCE:
    case GL_LUMINANCE_ALPHA:
    case GL_ABGR_EXT:
    case GL_BGR_EXT:
    case GL_BGRA_EXT:
        index = GL_FALSE;
        break;
    default:
        __gllc_InvalidEnum(gc);
        return;
    }
    switch (type)
    {
    case GL_BITMAP:
        if (!index)
        {
            __gllc_InvalidEnum(gc);
            return;
        }
        break;
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    case GL_INT:
    case GL_UNSIGNED_INT:
    case GL_FLOAT:
        break;
    case GL_UNSIGNED_BYTE_3_3_2:
    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_BYTE_2_3_3_REV:
    case GL_UNSIGNED_SHORT_5_6_5_REV:
        switch (format)
        {
        case GL_RGB:
        case GL_BGR_EXT:
            adjustFormat= GL_LUMINANCE; /* or anything that's 1 component */
            adjustType= GL_UNSIGNED_BYTE;
            break;
        default:
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        break;
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_5_5_5_1:
    case GL_UNSIGNED_INT_8_8_8_8:
    case GL_UNSIGNED_INT_10_10_10_2:
    case GL_UNSIGNED_SHORT_4_4_4_4_REV:
    case GL_UNSIGNED_SHORT_1_5_5_5_REV:
    case GL_UNSIGNED_INT_8_8_8_8_REV:
    case GL_UNSIGNED_INT_2_10_10_10_REV:
        switch (format)
        {
        case GL_RGBA:
        case GL_ABGR_EXT:
        case GL_BGRA_EXT:
            adjustFormat= GL_LUMINANCE; /* or anything that's 1 component */
            adjustType= GL_UNSIGNED_SHORT;
            if (type == GL_UNSIGNED_INT_8_8_8_8 ||
                type == GL_UNSIGNED_INT_10_10_10_2 ||
                type == GL_UNSIGNED_INT_8_8_8_8_REV ||
                type == GL_UNSIGNED_INT_2_10_10_10_REV)
                adjustType= GL_UNSIGNED_INT;
            break;
        default:
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        break;
    default:
        __gllc_InvalidEnum(gc);
        return;
    }

    rowSize = __glImageSize(width, 1, format, type);
    rowSize = __GL_PAD(rowSize);

    colSize = __glImageSize(1, height, format, type);
    colSize = __GL_PAD(colSize);

    imageSize = rowSize+colSize;

    dlop = __glDlistAllocOp(gc, (sizeof(struct __gllc_SeparableFilter2D_Rec) + imageSize));
    if (dlop == NULL) return;
    dlop->opcode = __glop_SeparableFilter2D;

    data = (struct __gllc_SeparableFilter2D_Rec *)(dlop + 1);
    data->target = target;
    data->internalformat = internalformat;
    data->width = width;
    data->height = height;
    data->format = format;
    data->type = type;

    if (rowSize > 0 && row != NULL)
    {
        __glFillImage(gc, width, 1, adjustFormat, adjustType, row,
                        (GLubyte*)(data + 1));
    }

    if (colSize > 0 && col != NULL)
    {
        __glFillImage(gc, 1, height, adjustFormat, adjustType, col,
                        ((GLubyte *)(data + 1) + rowSize));
    }

    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_CopyConvolutionFilter1D(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)
{
    __GLdlistOp *dlop;
    struct __gllc_CopyConvolutionFilter1D_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_CopyConvolutionFilter1D(target, internalformat, x, y, width);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_CopyConvolutionFilter1D_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_CopyConvolutionFilter1D;
    data = (struct __gllc_CopyConvolutionFilter1D_Rec *)(dlop + 1);
    data->target = target;
    data->internalformat = internalformat;
    data->x = x;
    data->y = y;
    data->width = width;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_CopyConvolutionFilter2D(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GLdlistOp *dlop;
    struct __gllc_CopyConvolutionFilter2D_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_CopyConvolutionFilter2D(target, internalformat, x, y, width, height);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_CopyConvolutionFilter2D_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_CopyConvolutionFilter2D;
    data = (struct __gllc_CopyConvolutionFilter2D_Rec *)(dlop + 1);
    data->target = target;
    data->internalformat = internalformat;
    data->x = x;
    data->y = y;
    data->width = width;
    data->height = height;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Histogram(GLenum target, GLsizei width, GLenum internalformat, GLboolean sink)
{
    __GLdlistOp *dlop;
    struct __gllc_Histogram_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Histogram(target, width, internalformat, sink);
    }

    switch (target)
    {
    case GL_PROXY_HISTOGRAM:
        __glim_Histogram(target, width, internalformat, sink);
        return;
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Histogram_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Histogram;
    data = (struct __gllc_Histogram_Rec *)(dlop + 1);
    data->target = target;
    data->width = width;
    data->internalformat = internalformat;
    data->sink = sink;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ResetHistogram(GLenum target)
{
    __GLdlistOp *dlop;
    struct __gllc_ResetHistogram_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ResetHistogram(target);
    }

    if(target != GL_HISTOGRAM) {
        __gllc_InvalidEnum(gc);
          return;
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ResetHistogram_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ResetHistogram;
    data = (struct __gllc_ResetHistogram_Rec *)(dlop + 1);
    data->target = target;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Minmax(GLenum target, GLenum internalFormat, GLboolean sink)
{
    __GLdlistOp *dlop;
    struct __gllc_Minmax_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Minmax(target, internalFormat, sink);
    }

    if(target != GL_MINMAX) {
        __gllc_InvalidEnum(gc);
          return;
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Minmax_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Minmax;
    data = (struct __gllc_Minmax_Rec *)(dlop + 1);
    data->target = target;
    data->internalFormat = internalFormat;
    data->sink = sink;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ResetMinmax(GLenum target)
{
    __GLdlistOp *dlop;
    struct __gllc_ResetMinmax_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ResetMinmax(target);
    }

    if(target != GL_MINMAX) {
        __gllc_InvalidEnum(gc);
          return;
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ResetMinmax_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ResetMinmax;
    data = (struct __gllc_ResetMinmax_Rec *)(dlop + 1);
    data->target = target;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_LoadTransposeMatrixf(const GLfloat *m)
{
    __GLdlistOp *dlop;
    struct __gllc_LoadMatrixf_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_LoadTransposeMatrixf(m);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_LoadMatrixf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_LoadTransposeMatrixf;
    data = (struct __gllc_LoadMatrixf_Rec *)(dlop + 1);
    __GL_MEMCOPY(data->m, m, sizeof(data->m));
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_LoadTransposeMatrixd(const GLdouble *m)
{
    __GLdlistOp *dlop;
    struct __gllc_LoadMatrixd_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_LoadTransposeMatrixd(m);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_LoadMatrixd_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_LoadTransposeMatrixd;
    data = (struct __gllc_LoadMatrixd_Rec *)(dlop + 1);
    __GL_MEMCOPY(data->m, m, sizeof(data->m));
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultTransposeMatrixf(const GLfloat *m)
{
    __GLdlistOp *dlop;
    struct __gllc_MultMatrixf_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_MultTransposeMatrixf(m);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultMatrixf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MultTransposeMatrixf;
    data = (struct __gllc_MultMatrixf_Rec *)(dlop + 1);
    __GL_MEMCOPY(data->m, m, sizeof(data->m));
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultTransposeMatrixd(const GLdouble *m)
{
    __GLdlistOp *dlop;
    struct __gllc_MultMatrixd_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_MultTransposeMatrixd(m);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultMatrixd_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MultTransposeMatrixd;
    data = (struct __gllc_MultMatrixd_Rec *)(dlop + 1);
    __GL_MEMCOPY(data->m, m, sizeof(data->m));
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PointParameterfv(GLenum pname, const GLfloat *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint paramSize;
    struct __gllc_PointParameterfv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PointParameterfv(pname, params);
    }

    paramSize = __glPointParameter_size(pname) * 4;
    if (paramSize < 0) {
        __gllc_InvalidEnum(gc);
        return;
    }
    size = (GLuint)sizeof(struct __gllc_PointParameterfv_Rec) + paramSize;
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_PointParameterfv;
    data = (struct __gllc_PointParameterfv_Rec *)(dlop + 1);
    data->pname = pname;
    data->paramSize = paramSize;
    __GL_MEMCOPY((GLubyte *)(data + 1), params, paramSize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PointParameterf(GLenum pname, GLfloat param)
{
    __GL_SETUP();

    if (__glPointParameter_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_PointParameterfv(pname, &param);
}

GLvoid APIENTRY __gllc_PointParameteriv(GLenum pname, const GLint *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint paramSize;
    struct __gllc_PointParameteriv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PointParameteriv(pname, params);
    }

    paramSize = __glPointParameter_size(pname) * 4;
    if (paramSize < 0) {
        __gllc_InvalidEnum(gc);
        return;
    }
    size = (GLuint)sizeof(struct __gllc_PointParameteriv_Rec) + paramSize;
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_PointParameteriv;
    data = (struct __gllc_PointParameteriv_Rec *)(dlop + 1);
    data->pname = pname;
    data->paramSize = paramSize;
    __GL_MEMCOPY((GLubyte *)(data + 1), params, paramSize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PointParameteri(GLenum pname, GLint param)
{
    __GL_SETUP();

    if (__glPointParameter_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_PointParameteriv(pname, &param);
}

GLvoid APIENTRY __gllc_WindowPos2sv(const GLshort  *v)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos2sv(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_WindowPos2fv;
    data = (struct __gllc_WindowPos2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_WindowPos2s(GLshort x, GLshort y)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos2s(x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_WindowPos2fv;
    data = (struct __gllc_WindowPos2fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_WindowPos2iv(const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos2iv(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_WindowPos2fv;
    data = (struct __gllc_WindowPos2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_WindowPos2i(GLint x, GLint y)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos2i(x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_WindowPos2fv;
    data = (struct __gllc_WindowPos2fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_WindowPos2fv(const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos2fv(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_WindowPos2fv;
    data = (struct __gllc_WindowPos2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_WindowPos2f(GLfloat x, GLfloat y)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos2f(x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_WindowPos2fv;
    data = (struct __gllc_WindowPos2fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_WindowPos2dv(const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos2dv(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_WindowPos2fv;
    data = (struct __gllc_WindowPos2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_WindowPos2d(GLdouble x, GLdouble y)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos2fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos2d(x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_WindowPos2fv;
    data = (struct __gllc_WindowPos2fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_WindowPos3sv(const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos3sv(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_WindowPos3fv;
    data = (struct __gllc_WindowPos3fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_WindowPos3s(GLshort x, GLshort y, GLshort z)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos3s(x, y, z);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_WindowPos3fv;
    data = (struct __gllc_WindowPos3fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = z;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_WindowPos3iv(const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos3iv(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_WindowPos3fv;
    data = (struct __gllc_WindowPos3fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_WindowPos3i(GLint x, GLint y, GLint z)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos3i(x, y, z);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_WindowPos3fv;
    data = (struct __gllc_WindowPos3fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = z;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_WindowPos3fv(const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos3fv(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_WindowPos3fv;
    data = (struct __gllc_WindowPos3fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_WindowPos3f(GLfloat x, GLfloat y, GLfloat z)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos3f(x, y, z);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_WindowPos3fv;
    data = (struct __gllc_WindowPos3fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = z;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_WindowPos3dv(const GLdouble*v)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos3dv(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_WindowPos3fv;
    data = (struct __gllc_WindowPos3fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_WindowPos3d(GLdouble x, GLdouble y, GLdouble z)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos3d(x, y, z);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_WindowPos3fv;
    data = (struct __gllc_WindowPos3fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    data->v[2] = z;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_SecondaryColor3bv(const GLbyte *v)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.SecondaryColor3bv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_SecondaryColor3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_SecondaryColor3fv;
    data = (struct __gllc_SecondaryColor3fv_Rec *)(dlop + 1);
    data->v[0] = __GL_B_TO_FLOAT(v[0]);
    data->v[1] = __GL_B_TO_FLOAT(v[1]);
    data->v[2] = __GL_B_TO_FLOAT(v[2]);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_SecondaryColor3b(GLbyte red, GLbyte green, GLbyte blue)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.SecondaryColor3b)(red, green, blue);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_SecondaryColor3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_SecondaryColor3fv;
    data = (struct __gllc_SecondaryColor3fv_Rec *)(dlop + 1);
    data->v[0] = __GL_B_TO_FLOAT(red);
    data->v[1] = __GL_B_TO_FLOAT(green);
    data->v[2] = __GL_B_TO_FLOAT(blue);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_SecondaryColor3dv(const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.SecondaryColor3dv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_SecondaryColor3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_SecondaryColor3fv;
    data = (struct __gllc_SecondaryColor3fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_SecondaryColor3d(GLdouble red, GLdouble green, GLdouble blue)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.SecondaryColor3d)(red, green, blue);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_SecondaryColor3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_SecondaryColor3fv;
    data = (struct __gllc_SecondaryColor3fv_Rec *)(dlop + 1);
    data->v[0] = red;
    data->v[1] = green;
    data->v[2] = blue;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_SecondaryColor3fv(const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.SecondaryColor3fv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_SecondaryColor3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_SecondaryColor3fv;
    data = (struct __gllc_SecondaryColor3fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_SecondaryColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.SecondaryColor3f)(red, green, blue);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_SecondaryColor3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_SecondaryColor3fv;
    data = (struct __gllc_SecondaryColor3fv_Rec *)(dlop + 1);
    data->v[0] = red;
    data->v[1] = green;
    data->v[2] = blue;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_SecondaryColor3iv(const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.SecondaryColor3iv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_SecondaryColor3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_SecondaryColor3fv;
    data = (struct __gllc_SecondaryColor3fv_Rec *)(dlop + 1);
    data->v[0] = __GL_I_TO_FLOAT(v[0]);
    data->v[1] = __GL_I_TO_FLOAT(v[1]);
    data->v[2] = __GL_I_TO_FLOAT(v[2]);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_SecondaryColor3i(GLint red, GLint green, GLint blue)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.SecondaryColor3i)(red, green, blue);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_SecondaryColor3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_SecondaryColor3fv;
    data = (struct __gllc_SecondaryColor3fv_Rec *)(dlop + 1);
    data->v[0] = __GL_I_TO_FLOAT(red);
    data->v[1] = __GL_I_TO_FLOAT(green);
    data->v[2] = __GL_I_TO_FLOAT(blue);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_SecondaryColor3sv(const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.SecondaryColor3sv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_SecondaryColor3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_SecondaryColor3fv;
    data = (struct __gllc_SecondaryColor3fv_Rec *)(dlop + 1);
    data->v[0] = __GL_I_TO_FLOAT(v[0]);
    data->v[1] = __GL_I_TO_FLOAT(v[1]);
    data->v[2] = __GL_I_TO_FLOAT(v[2]);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_SecondaryColor3s(GLshort red, GLshort green, GLshort blue)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.SecondaryColor3s)(red, green, blue);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_SecondaryColor3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_SecondaryColor3fv;
    data = (struct __gllc_SecondaryColor3fv_Rec *)(dlop + 1);
    data->v[0] = __GL_S_TO_FLOAT(red);
    data->v[1] = __GL_S_TO_FLOAT(green);
    data->v[2] = __GL_S_TO_FLOAT(blue);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_SecondaryColor3ubv(const GLubyte *v)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.SecondaryColor3ubv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_SecondaryColor3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_SecondaryColor3fv;
    data = (struct __gllc_SecondaryColor3fv_Rec *)(dlop + 1);
    data->v[0] = __GL_UB_TO_FLOAT(v[0]);
    data->v[1] = __GL_UB_TO_FLOAT(v[1]);
    data->v[2] = __GL_UB_TO_FLOAT(v[2]);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_SecondaryColor3ub(GLubyte red, GLubyte green, GLubyte blue)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.SecondaryColor3ub)(red, green, blue);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_SecondaryColor3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_SecondaryColor3fv;
    data = (struct __gllc_SecondaryColor3fv_Rec *)(dlop + 1);
    data->v[0] = __GL_UB_TO_FLOAT(red);
    data->v[1] = __GL_UB_TO_FLOAT(green);
    data->v[2] = __GL_UB_TO_FLOAT(blue);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_SecondaryColor3uiv(const GLuint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.SecondaryColor3uiv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_SecondaryColor3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_SecondaryColor3fv;
    data = (struct __gllc_SecondaryColor3fv_Rec *)(dlop + 1);
    data->v[0] = __GL_UI_TO_FLOAT(v[0]);
    data->v[1] = __GL_UI_TO_FLOAT(v[1]);
    data->v[2] = __GL_UI_TO_FLOAT(v[2]);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_SecondaryColor3ui(GLuint red, GLuint green, GLuint blue)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.SecondaryColor3ui)(red, green, blue);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_SecondaryColor3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_SecondaryColor3fv;
    data = (struct __gllc_SecondaryColor3fv_Rec *)(dlop + 1);
    data->v[0] = __GL_UI_TO_FLOAT(red);
    data->v[1] = __GL_UI_TO_FLOAT(green);
    data->v[2] = __GL_UI_TO_FLOAT(blue);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_SecondaryColor3usv(const GLushort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.SecondaryColor3usv)(v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_SecondaryColor3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_SecondaryColor3fv;
    data = (struct __gllc_SecondaryColor3fv_Rec *)(dlop + 1);
    data->v[0] = __GL_US_TO_FLOAT(v[0]);
    data->v[1] = __GL_US_TO_FLOAT(v[1]);
    data->v[2] = __GL_US_TO_FLOAT(v[2]);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_SecondaryColor3us(GLushort red, GLushort green, GLushort blue)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.SecondaryColor3us)(red, green, blue);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_SecondaryColor3fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_SecondaryColor3fv;
    data = (struct __gllc_SecondaryColor3fv_Rec *)(dlop + 1);
    data->v[0] = __GL_US_TO_FLOAT(red);
    data->v[1] = __GL_US_TO_FLOAT(green);
    data->v[2] = __GL_US_TO_FLOAT(blue);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_BeginQuery(GLenum target, GLuint id)
{
    __GLdlistOp *dlop;
    struct __gllc_BeginQuery_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_BeginQuery(target, id);
    }

    if (target != GL_SAMPLES_PASSED) {
        __gllc_InvalidEnum(gc);
          return;
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_BeginQuery_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_BeginQuery;
    data = (struct __gllc_BeginQuery_Rec *)(dlop + 1);
    data->target = target;
    data->id = id;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_EndQuery(GLenum target)
{
    __GLdlistOp *dlop;
    struct __gllc_EndQuery_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EndQuery(target);
    }

    if (target != GL_SAMPLES_PASSED) {
        __gllc_InvalidEnum(gc);
          return;
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_EndQuery_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_EndQuery;
    data = (struct __gllc_EndQuery_Rec *)(dlop + 1);
    data->target = target;
    __glDlistAppendOp(gc, dlop);
}

#if GL_NV_occlusion_query
GLvoid APIENTRY __gllc_BeginQueryNV(GLuint id)
{
    GLenum target;
    __GLdlistOp *dlop;
    struct __gllc_BeginQuery_Rec *data;
    __GL_SETUP();

    target = GL_SAMPLES_PASSED;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_BeginQueryNV(id);
    }

    if (target != GL_SAMPLES_PASSED) {
        __gllc_InvalidEnum(gc);
          return;
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_BeginQuery_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_BeginQuery;
    data = (struct __gllc_BeginQuery_Rec *)(dlop + 1);
    data->target = target;
    data->id = id;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_EndQueryNV(GLvoid)
{
    GLenum target;
    __GLdlistOp *dlop;
    struct __gllc_EndQuery_Rec *data;
    __GL_SETUP();

    target = GL_SAMPLES_PASSED;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EndQueryNV();
    }

    if (target != GL_SAMPLES_PASSED) {
        __gllc_InvalidEnum(gc);
          return;
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_EndQuery_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_EndQuery;
    data = (struct __gllc_EndQuery_Rec *)(dlop + 1);
    data->target = target;
    __glDlistAppendOp(gc, dlop);
}
#endif

GLvoid APIENTRY __gllc_Uniform4f(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform4f_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform4f(location, x, y, z, w);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Uniform4f_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Uniform4f;
    data = (struct __gllc_Uniform4f_Rec *)(dlop + 1);
    data->location = location;
    data->x = x;
    data->y = y;
    data->z = z;
    data->w = w;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Uniform3f(GLint location, GLfloat x, GLfloat y, GLfloat z)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform3f_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform3f(location, x, y, z);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Uniform3f_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Uniform3f;
    data = (struct __gllc_Uniform3f_Rec *)(dlop + 1);
    data->location = location;
    data->x = x;
    data->y = y;
    data->z = z;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Uniform2f(GLint location, GLfloat x, GLfloat y)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform2f_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform2f(location, x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Uniform2f_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Uniform2f;
    data = (struct __gllc_Uniform2f_Rec *)(dlop + 1);
    data->location = location;
    data->x = x;
    data->y = y;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Uniform1f(GLint location, GLfloat x)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform1f_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform1f(location, x);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Uniform1f_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Uniform1f;
    data = (struct __gllc_Uniform1f_Rec *)(dlop + 1);
    data->location = location;
    data->x = x;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Uniform4i(GLint location, GLint x, GLint y , GLint z , GLint w)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform4i_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform4i(location, x, y, z, w);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Uniform4i_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Uniform4i;
    data = (struct __gllc_Uniform4i_Rec *)(dlop + 1);
    data->location = location;
    data->x = x;
    data->y = y;
    data->z = z;
    data->w = w;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Uniform3i(GLint location, GLint x , GLint y , GLint z)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform3i_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform3i(location, x, y, z);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Uniform3i_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Uniform3i;
    data = (struct __gllc_Uniform3i_Rec *)(dlop + 1);
    data->location = location;
    data->x = x;
    data->y = y;
    data->z = z;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Uniform2i(GLint location, GLint x , GLint y)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform2i_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform2i(location, x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Uniform2i_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Uniform2i;
    data = (struct __gllc_Uniform2i_Rec *)(dlop + 1);
    data->location = location;
    data->x = x;
    data->y = y;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Uniform1i(GLint location, GLint x)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform1i_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform1i(location, x);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Uniform1i_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Uniform1i;
    data = (struct __gllc_Uniform1i_Rec *)(dlop + 1);
    data->location = location;
    data->x = x;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Uniform4fv(GLint location, GLsizei count, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform4fv_Rec *data;
    GLint arraySize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform4fv(location, count, v);
    }

    arraySize = __GL64PAD(count * 4 * sizeof(GLfloat));

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Uniform4fv_Rec) + arraySize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_Uniform4fv;
    data = (struct __gllc_Uniform4fv_Rec *)(dlop + 1);
    data->location = location;
    data->count = count;
    __GL_MEMCOPY((GLubyte *)(data + 1), v, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Uniform3fv(GLint location, GLsizei count, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform3fv_Rec *data;
    GLint arraySize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform3fv(location, count, v);
    }

    arraySize = __GL64PAD(count * 3 * sizeof(GLfloat));

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Uniform3fv_Rec) + arraySize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_Uniform3fv;
    data = (struct __gllc_Uniform3fv_Rec *)(dlop + 1);
    data->location = location;
    data->count = count;
    __GL_MEMCOPY((GLubyte *)(data + 1), v, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Uniform2fv(GLint location, GLsizei count, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform2fv_Rec *data;
    GLint arraySize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform2fv(location, count, v);
    }

    arraySize = __GL64PAD(count * 2 * sizeof(GLfloat));

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Uniform2fv_Rec) + arraySize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_Uniform2fv;
    data = (struct __gllc_Uniform2fv_Rec *)(dlop + 1);
    data->location = location;
    data->count = count;
    __GL_MEMCOPY((GLubyte *)(data + 1), v, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Uniform1fv(GLint location, GLsizei count, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform1fv_Rec *data;
    GLint arraySize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform1fv(location, count, v);
    }

    arraySize = __GL64PAD(count * sizeof(GLfloat));

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Uniform1fv_Rec) + arraySize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_Uniform1fv;
    data = (struct __gllc_Uniform1fv_Rec *)(dlop + 1);
    data->location = location;
    data->count = count;
    __GL_MEMCOPY((GLubyte *)(data + 1), v, arraySize);
    __glDlistAppendOp(gc, dlop);
}


GLvoid APIENTRY __gllc_Uniform4iv(GLint location, GLsizei count, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform4iv_Rec *data;
    GLint arraySize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform4iv(location, count, v);
    }

    arraySize = __GL64PAD(count * 4 * sizeof(GLint));

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Uniform4iv_Rec) + arraySize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_Uniform4iv;
    data = (struct __gllc_Uniform4iv_Rec *)(dlop + 1);
    data->location = location;
    data->count = count;
    __GL_MEMCOPY((GLubyte *)(data + 1), v, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Uniform3iv(GLint location, GLsizei count, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform3iv_Rec *data;
    GLint arraySize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform3iv(location, count, v);
    }

    arraySize = __GL64PAD(count * 3 * sizeof(GLint));

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Uniform3iv_Rec) + arraySize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_Uniform3iv;
    data = (struct __gllc_Uniform3iv_Rec *)(dlop + 1);
    data->location = location;
    data->count = count;
    __GL_MEMCOPY((GLubyte *)(data + 1), v, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Uniform2iv(GLint location, GLsizei count, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform2iv_Rec *data;
    GLint arraySize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform2iv(location, count, v);
    }

    arraySize = __GL64PAD(count * 2 * sizeof(GLint));

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Uniform2iv_Rec) + arraySize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_Uniform2iv;
    data = (struct __gllc_Uniform2iv_Rec *)(dlop + 1);
    data->location = location;
    data->count = count;
    __GL_MEMCOPY((GLubyte *)(data + 1), v, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Uniform1iv(GLint location, GLsizei count, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform1iv_Rec *data;
    GLint arraySize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform1iv(location, count, v);
    }

    arraySize = __GL64PAD(count * sizeof(GLint));

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Uniform1iv_Rec) + arraySize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_Uniform1iv;
    data = (struct __gllc_Uniform1iv_Rec *)(dlop + 1);
    data->location = location;
    data->count = count;
    __GL_MEMCOPY((GLubyte *)(data + 1), v, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_UniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_UniformMatrix4fv_Rec *data;
    GLint arraySize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_UniformMatrix4fv(location, count, transpose, v);
    }

    arraySize = __GL64PAD(count * 16 * sizeof(GLfloat));

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_UniformMatrix4fv_Rec) + arraySize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_UniformMatrix4fv;
    data = (struct __gllc_UniformMatrix4fv_Rec *)(dlop + 1);
    data->location = location;
    data->count = count;
    data->transpose = transpose;
    __GL_MEMCOPY((GLubyte *)(data + 1), v, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_UniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_UniformMatrix3fv_Rec *data;
    GLint arraySize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_UniformMatrix3fv(location, count, transpose, v);
    }

    arraySize = __GL64PAD(count * 9 * sizeof(GLfloat));

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_UniformMatrix3fv_Rec) + arraySize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_UniformMatrix3fv;
    data = (struct __gllc_UniformMatrix3fv_Rec *)(dlop + 1);
    data->location = location;
    data->count = count;
    data->transpose = transpose;
    __GL_MEMCOPY((GLubyte *)(data + 1), v, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_UniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_UniformMatrix2fv_Rec *data;
    GLint arraySize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_UniformMatrix2fv(location, count, transpose, v);
    }

    arraySize = __GL64PAD(count * 4 * sizeof(GLfloat));

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_UniformMatrix2fv_Rec) + arraySize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_UniformMatrix2fv;
    data = (struct __gllc_UniformMatrix2fv_Rec *)(dlop + 1);
    data->location = location;
    data->count = count;
    data->transpose = transpose;
    __GL_MEMCOPY((GLubyte *)(data + 1), v, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_UniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_UniformMatrix2x3fv_Rec *data;
    GLint arraySize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_UniformMatrix2x3fv(location, count, transpose, v);
    }

    arraySize = __GL64PAD(count * 6 * sizeof(GLfloat));

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_UniformMatrix2x3fv_Rec) + arraySize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_UniformMatrix2x3fv;
    data = (struct __gllc_UniformMatrix2x3fv_Rec *)(dlop + 1);
    data->location = location;
    data->count = count;
    data->transpose = transpose;
    __GL_MEMCOPY((GLubyte *)(data + 1), v, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_UniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_UniformMatrix2x4fv_Rec *data;
    GLint arraySize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_UniformMatrix2x4fv(location, count, transpose, v);
    }

    arraySize = __GL64PAD(count * 8 * sizeof(GLfloat));

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_UniformMatrix2x4fv_Rec) + arraySize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_UniformMatrix2x4fv;
    data = (struct __gllc_UniformMatrix2x4fv_Rec *)(dlop + 1);
    data->location = location;
    data->count = count;
    data->transpose = transpose;
    __GL_MEMCOPY((GLubyte *)(data + 1), v, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_UniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_UniformMatrix3x2fv_Rec *data;
    GLint arraySize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_UniformMatrix3x2fv(location, count, transpose, v);
    }

    arraySize = __GL64PAD(count * 6 * sizeof(GLfloat));

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_UniformMatrix3x2fv_Rec) + arraySize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_UniformMatrix3x2fv;
    data = (struct __gllc_UniformMatrix3x2fv_Rec *)(dlop + 1);
    data->location = location;
    data->count = count;
    data->transpose = transpose;
    __GL_MEMCOPY((GLubyte *)(data + 1), v, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_UniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_UniformMatrix3x4fv_Rec *data;
    GLint arraySize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_UniformMatrix3x4fv(location, count, transpose, v);
    }

    arraySize = __GL64PAD(count * 12 * sizeof(GLfloat));

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_UniformMatrix3x4fv_Rec) + arraySize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_UniformMatrix3x4fv;
    data = (struct __gllc_UniformMatrix3x4fv_Rec *)(dlop + 1);
    data->location = location;
    data->count = count;
    data->transpose = transpose;
    __GL_MEMCOPY((GLubyte *)(data + 1), v, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_UniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_UniformMatrix4x2fv_Rec *data;
    GLint arraySize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_UniformMatrix4x2fv(location, count, transpose, v);
    }

    arraySize = __GL64PAD(count * 8 * sizeof(GLfloat));

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_UniformMatrix4x2fv_Rec) + arraySize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_UniformMatrix4x2fv;
    data = (struct __gllc_UniformMatrix4x2fv_Rec *)(dlop + 1);
    data->location = location;
    data->count = count;
    data->transpose = transpose;
    __GL_MEMCOPY((GLubyte *)(data + 1), v, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_UniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_UniformMatrix4x3fv_Rec *data;
    GLint arraySize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_UniformMatrix4x3fv(location, count, transpose, v);
    }

    arraySize = __GL64PAD(count * 12 * sizeof(GLfloat));

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_UniformMatrix4x3fv_Rec) + arraySize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_UniformMatrix4x3fv;
    data = (struct __gllc_UniformMatrix4x3fv_Rec *)(dlop + 1);
    data->location = location;
    data->count = count;
    data->transpose = transpose;
    __GL_MEMCOPY((GLubyte *)(data + 1), v, arraySize);
    __glDlistAppendOp(gc, dlop);
}


GLvoid APIENTRY __gllc_DrawBuffers(GLsizei count, const GLenum *bufs)
{
    __GLdlistOp *dlop;
    struct __gllc_DrawBuffers_Rec *data;
    GLint arraySize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_DrawBuffers(count, bufs);
    }

    arraySize = __GL64PAD(count * sizeof(GLenum));

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_DrawBuffers_Rec) + arraySize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_DrawBuffers;
    data = (struct __gllc_DrawBuffers_Rec *)(dlop + 1);
    data->count = count;
    __GL_MEMCOPY((GLubyte *)(data + 1), bufs, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_UseProgram(GLuint program)
{
    __GLdlistOp *dlop;
    struct __gllc_UseProgram_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_UseProgram(program);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_UseProgram_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_UseProgram;
    data = (struct __gllc_UseProgram_Rec *)(dlop + 1);
    data->program = program;
    __glDlistAppendOp(gc, dlop);
}


#if GL_ARB_vertex_program

GLvoid APIENTRY __gllc_BindProgramARB(GLenum target, GLuint program)
{
    __GLdlistOp *dlop;
    struct __gllc_BindProgramARB_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_BindProgramARB(target, program);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_BindProgramARB_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_BindProgramARB;
    data = (struct __gllc_BindProgramARB_Rec *)(dlop + 1);
    data->target = target;
    data->program = program;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ProgramEnvParameter4dARB(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GLdlistOp *dlop;
    struct __gllc_ProgramEnvParameter4dARB_Rec * data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ProgramEnvParameter4dARB(target, index, x, y, z, w);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ProgramEnvParameter4dARB_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ProgramEnvParameter4dARB;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_ProgramEnvParameter4dARB_Rec *)(dlop + 1);
    data->target = target;
    data->index = index;
    data->x = x;
    data->y = y;
    data->z = z;
    data->w = w;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ProgramEnvParameter4dvARB(GLenum target, GLuint index, const GLdouble *params)
{
    __GLdlistOp *dlop;
    struct __gllc_ProgramEnvParameter4dvARB_Rec * data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ProgramEnvParameter4dvARB(target, index, params);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ProgramEnvParameter4dvARB_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ProgramEnvParameter4dvARB;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_ProgramEnvParameter4dvARB_Rec *)(dlop + 1);
    data->target = target;
    data->index = index;
    data->v[0] = params[0];
    data->v[1] = params[1];
    data->v[2] = params[2];
    data->v[3] = params[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ProgramEnvParameter4fARB(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GLdlistOp *dlop;
    struct __gllc_ProgramEnvParameter4fARB_Rec * data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ProgramEnvParameter4fARB(target, index, x, y, z, w);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ProgramEnvParameter4fARB_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ProgramEnvParameter4fARB;
    data = (struct __gllc_ProgramEnvParameter4fARB_Rec *)(dlop + 1);
    data->target = target;
    data->index = index;
    data->x = x;
    data->y = y;
    data->z = z;
    data->w = w;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ProgramEnvParameter4fvARB(GLenum target, GLuint index, const GLfloat *params)
{
    __GLdlistOp *dlop;
    struct __gllc_ProgramEnvParameter4fvARB_Rec * data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ProgramEnvParameter4fvARB(target, index, params);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ProgramEnvParameter4fvARB_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ProgramEnvParameter4fvARB;
    data = (struct __gllc_ProgramEnvParameter4fvARB_Rec *)(dlop + 1);
    data->target = target;
    data->index = index;
    data->v[0] = params[0];
    data->v[1] = params[1];
    data->v[2] = params[2];
    data->v[3] = params[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ProgramLocalParameter4dARB(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GLdlistOp *dlop;
    struct __gllc_ProgramEnvParameter4dARB_Rec * data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ProgramLocalParameter4dARB(target, index, x, y, z, w);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ProgramEnvParameter4dARB_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ProgramLocalParameter4dARB;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_ProgramEnvParameter4dARB_Rec *)(dlop + 1);
    data->target = target;
    data->index = index;
    data->x = x;
    data->y = y;
    data->z = z;
    data->w = w;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ProgramLocalParameter4dvARB(GLenum target, GLuint index, const GLdouble *params)
{
    __GLdlistOp *dlop;
    struct __gllc_ProgramEnvParameter4dvARB_Rec * data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ProgramLocalParameter4dvARB(target, index, params);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ProgramEnvParameter4dvARB_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ProgramLocalParameter4dvARB;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_ProgramEnvParameter4dvARB_Rec *)(dlop + 1);
    data->target = target;
    data->index = index;
    data->v[0] = params[0];
    data->v[1] = params[1];
    data->v[2] = params[2];
    data->v[3] = params[3];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ProgramLocalParameter4fARB(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GLdlistOp *dlop;
    struct __gllc_ProgramEnvParameter4fARB_Rec * data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ProgramLocalParameter4fARB(target, index, x, y, z, w);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ProgramEnvParameter4fARB_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ProgramLocalParameter4fARB;
    data = (struct __gllc_ProgramEnvParameter4fARB_Rec *)(dlop + 1);
    data->target = target;
    data->index = index;
    data->x = x;
    data->y = y;
    data->z = z;
    data->w = w;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ProgramLocalParameter4fvARB(GLenum target, GLuint index, const GLfloat *params)
{
    __GLdlistOp *dlop;
    struct __gllc_ProgramEnvParameter4fvARB_Rec * data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ProgramLocalParameter4fvARB(target, index, params);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ProgramEnvParameter4fvARB_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ProgramLocalParameter4fvARB;
    data = (struct __gllc_ProgramEnvParameter4fvARB_Rec *)(dlop + 1);
    data->target = target;
    data->index = index;
    data->v[0] = params[0];
    data->v[1] = params[1];
    data->v[2] = params[2];
    data->v[3] = params[3];
    __glDlistAppendOp(gc, dlop);
}

#endif

#if GL_ATI_element_array
GLvoid  APIENTRY __gllc_DrawElementArrayATI(GLenum mode, GLsizei count)
{
    __GLdlistOp *dlop;
    struct __gllc_DrawElementArrayATI_Rec * data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_DrawElementArrayATI(mode, count);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_DrawElementArrayATI_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_DrawElementArrayATI;
    data = (struct __gllc_DrawElementArrayATI_Rec *)(dlop + 1);
    data->mode = mode;
    data->count = count;
    __glDlistAppendOp(gc, dlop);
}

GLvoid  APIENTRY __gllc_DrawRangeElementArrayATI(GLenum mode, GLuint start,
                                  GLuint end, GLsizei count)
{
    __GLdlistOp *dlop;
    struct __gllc_DrawRangeElementArrayATI_Rec * data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_DrawRangeElementArrayATI(mode, start, end, count);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_DrawRangeElementArrayATI_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_DrawRangeElementArrayATI;
    data = (struct __gllc_DrawRangeElementArrayATI_Rec *)(dlop + 1);
    data->mode = mode;
    data->count = count;
    data->start = start;
    data->end = end;
    __glDlistAppendOp(gc, dlop);
}
#endif

#if GL_EXT_stencil_two_side
GLvoid APIENTRY __gllc_ActiveStencilFaceEXT(GLenum face)
{
    __GLdlistOp *dlop;
    struct __gllc_ActiveStencilFaceEXT_Rec * data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ActiveStencilFaceEXT(face);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ActiveStencilFaceEXT_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ActiveStencilFaceEXT;
    data = (struct __gllc_ActiveStencilFaceEXT_Rec *)(dlop + 1);
    data->face = face;
    __glDlistAppendOp(gc, dlop);
}
#endif

#if GL_EXT_texture_integer
GLvoid APIENTRY __gllc_ClearColorIiEXT(GLint r, GLint g, GLint b, GLint a)
{
    __GLdlistOp *dlop;
    struct __gllc_ClearColorIiEXT_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ClearColorIiEXT(r, g, b, a);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ClearColorIiEXT_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ClearColorIiEXT;
    data = (struct __gllc_ClearColorIiEXT_Rec *)(dlop + 1);
    data->red = r;
    data->green = g;
    data->blue = b;
    data->alpha = a;
    __glDlistAppendOp(gc, dlop);
}


GLvoid APIENTRY __gllc_ClearColorIuiEXT(GLuint r, GLuint g, GLuint b, GLuint a)
{
    __GLdlistOp *dlop;
    struct __gllc_ClearColorIuiEXT_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ClearColorIuiEXT(r, g, b, a);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ClearColorIuiEXT_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ClearColorIuiEXT;
    data = (struct __gllc_ClearColorIuiEXT_Rec *)(dlop + 1);
    data->red = r;
    data->green = g;
    data->blue = b;
    data->alpha = a;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexParameterIivEXT(GLenum target, GLenum pname, GLint *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_TexParameterIivEXT_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexParameterIivEXT(target, pname, params);
    }

    arraySize = __GL64PAD(__glTexParameter_size(pname) * 4);
    if (arraySize < 0) {
        __gllc_InvalidEnum(gc);
        return;
    }
    size = (GLuint)sizeof(struct __gllc_TexParameterIivEXT_Rec) + arraySize;
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexParameterIivEXT;
    data = (struct __gllc_TexParameterIivEXT_Rec *)(dlop + 1);
    data->target = target;
    data->pname = pname;
    __GL_MEMCOPY((GLubyte *)(data + 1), params, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexParameterIuivEXT(GLenum target, GLenum pname, GLuint *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_TexParameterIuivEXT_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexParameterIuivEXT(target, pname, params);
    }

    arraySize = __GL64PAD(__glTexParameter_size(pname) * 4);
    if (arraySize < 0) {
        __gllc_InvalidEnum(gc);
        return;
    }
    size = (GLuint)sizeof(struct __gllc_TexParameterIuivEXT_Rec) + arraySize;
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexParameterIuivEXT;
    data = (struct __gllc_TexParameterIuivEXT_Rec *)(dlop + 1);
    data->target = target;
    data->pname = pname;
    __GL_MEMCOPY((GLubyte *)(data + 1), params, arraySize);
    __glDlistAppendOp(gc, dlop);
}
#endif

#if GL_EXT_gpu_shader4
GLvoid APIENTRY __gllc_VertexAttribI1iEXT(GLuint index, GLint x)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttribI1iEXT)(index, x);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->iv[0] = x;
    data->iv[1] = 0;
    data->iv[2] = 0;
    data->iv[3] = 0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttribI2iEXT(GLuint index, GLint x, GLint y)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttribI2iEXT)(index, x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->iv[0] = x;
    data->iv[1] = y;
    data->iv[2] = 0;
    data->iv[3] = 0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttribI3iEXT(GLuint index, GLint x, GLint y, GLint z)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttribI3iEXT)(index, x, y, z);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->iv[0] = x;
    data->iv[1] = y;
    data->iv[2] = z;
    data->iv[3] = 0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttribI4iEXT(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttribI4iEXT)(index, x, y, z, w);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->iv[0] = x;
    data->iv[1] = y;
    data->iv[2] = w;
    data->iv[3] = z;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttribI1uiEXT(GLuint index, GLuint x)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttribI1uiEXT)(index, x);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->uiv[0] = x;
    data->uiv[1] = 0;
    data->uiv[2] = 0;
    data->uiv[3] = 0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttribI2uiEXT(GLuint index, GLuint x, GLuint y)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttribI2uiEXT)(index, x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->uiv[0] = x;
    data->uiv[1] = y;
    data->uiv[2] = 0;
    data->uiv[3] = 0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttribI3uiEXT(GLuint index, GLuint x, GLuint y, GLuint z)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttribI3uiEXT)(index, x, y, z);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->uiv[0] = x;
    data->uiv[1] = y;
    data->uiv[2] = z;
    data->uiv[3] = 0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttribI4uiEXT(GLuint index, GLuint x, GLuint y, GLuint z,
                            GLuint w)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttribI4uiEXT)(index, x, y, z, w);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->uiv[0] = x;
    data->uiv[1] = y;
    data->uiv[2] = z;
    data->uiv[3] = w;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttribI1ivEXT(GLuint index, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttribI1ivEXT)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->iv[0] = v[0];
    data->iv[1] = 0;
    data->iv[2] = 0;
    data->iv[3] = 0;
    __glDlistAppendOp(gc, dlop);

}

GLvoid APIENTRY __gllc_VertexAttribI2ivEXT(GLuint index, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttribI2ivEXT)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->iv[0] = v[0];
    data->iv[1] = v[1];
    data->iv[2] = 0;
    data->iv[3] = 0;
    __glDlistAppendOp(gc, dlop);

}

GLvoid APIENTRY __gllc_VertexAttribI3ivEXT(GLuint index, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttribI3ivEXT)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->iv[0] = v[0];
    data->iv[1] = v[1];
    data->iv[2] = v[2];
    data->iv[3] = 0;
    __glDlistAppendOp(gc, dlop);

}

GLvoid APIENTRY __gllc_VertexAttribI4ivEXT(GLuint index, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttribI4ivEXT)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->iv[0] = v[0];
    data->iv[1] = v[1];
    data->iv[2] = v[2];
    data->iv[3] = v[3];
    __glDlistAppendOp(gc, dlop);

}

GLvoid APIENTRY __gllc_VertexAttribI1uivEXT(GLuint index, const GLuint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttribI1uivEXT)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->uiv[0] = v[0];
    data->uiv[1] = 0;
    data->uiv[2] = 0;
    data->uiv[3] = 0;
    __glDlistAppendOp(gc, dlop);

}

GLvoid APIENTRY __gllc_VertexAttribI2uivEXT(GLuint index, const GLuint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttribI2uivEXT)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->uiv[0] = v[0];
    data->uiv[1] = v[1];
    data->uiv[2] = 0;
    data->uiv[3] = 0;
    __glDlistAppendOp(gc, dlop);

}

GLvoid APIENTRY __gllc_VertexAttribI3uivEXT(GLuint index, const GLuint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttribI3uivEXT)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->uiv[0] = v[0];
    data->uiv[1] = v[1];
    data->uiv[2] = v[2];
    data->uiv[3] = 0;
    __glDlistAppendOp(gc, dlop);

}

GLvoid APIENTRY __gllc_VertexAttribI4uivEXT(GLuint index, const GLuint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttribI4uivEXT)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->uiv[0] = v[0];
    data->uiv[1] = v[1];
    data->uiv[2] = v[2];
    data->uiv[3] = v[3];
    __glDlistAppendOp(gc, dlop);

}

GLvoid APIENTRY __gllc_VertexAttribI4bvEXT(GLuint index, const GLbyte *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttribI4bvEXT)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->iv[0] = v[0];
    data->iv[1] = v[1];
    data->iv[2] = v[2];
    data->iv[3] = v[3];
    __glDlistAppendOp(gc, dlop);

}

GLvoid APIENTRY __gllc_VertexAttribI4svEXT(GLuint index, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttribI4svEXT)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->iv[0] = v[0];
    data->iv[1] = v[1];
    data->iv[2] = v[2];
    data->iv[3] = v[3];
    __glDlistAppendOp(gc, dlop);

}

GLvoid APIENTRY __gllc_VertexAttribI4ubvEXT(GLuint index, const GLubyte *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttribI4ubvEXT)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->uiv[0] = v[0];
    data->uiv[1] = v[1];
    data->uiv[2] = v[2];
    data->uiv[3] = v[3];
    __glDlistAppendOp(gc, dlop);

}
GLvoid APIENTRY __gllc_VertexAttribI4usvEXT(GLuint index, const GLushort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->dispatch.VertexAttribI4usvEXT)(index, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_VertexAttrib4fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_VertexAttrib4fv;
    data = (struct __gllc_VertexAttrib4fv_Rec *)(dlop + 1);
    data->index = index;
    data->uiv[0] = v[0];
    data->uiv[1] = v[1];
    data->uiv[2] = v[2];
    data->uiv[3] = v[3];
    __glDlistAppendOp(gc, dlop);

}


GLvoid APIENTRY __gllc_Uniform1uiEXT(GLint location, GLuint v0)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform1uiEXT_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform1uiEXT(location, v0);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Uniform1uiEXT_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Uniform1uiEXT;
    data = (struct __gllc_Uniform1uiEXT_Rec *)(dlop + 1);
    data->location = location;
    data->v0 = v0;
    __glDlistAppendOp(gc, dlop);

}

GLvoid APIENTRY __gllc_Uniform2uiEXT(GLint location, GLuint v0, GLuint v1)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform2uiEXT_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform2uiEXT(location, v0, v1);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Uniform2uiEXT_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Uniform2uiEXT;
    data = (struct __gllc_Uniform2uiEXT_Rec *)(dlop + 1);
    data->location = location;
    data->v0 = v0;
    data->v1 = v1;
    __glDlistAppendOp(gc, dlop);

}

GLvoid APIENTRY __gllc_Uniform3uiEXT(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform3uiEXT_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform3uiEXT(location, v0, v1, v2);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Uniform3uiEXT_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Uniform3uiEXT;
    data = (struct __gllc_Uniform3uiEXT_Rec *)(dlop + 1);
    data->location = location;
    data->v0 = v0;
    data->v1 = v1;
    data->v2 = v2;
    __glDlistAppendOp(gc, dlop);

}

GLvoid APIENTRY __gllc_Uniform4uiEXT(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform4uiEXT_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform4uiEXT(location, v0, v1, v2, v3);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Uniform4uiEXT_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Uniform4uiEXT;
    data = (struct __gllc_Uniform4uiEXT_Rec *)(dlop + 1);
    data->location = location;
    data->v0 = v0;
    data->v1 = v1;
    data->v2 = v2;
    data->v3 = v3;
    __glDlistAppendOp(gc, dlop);

}

GLvoid APIENTRY __gllc_Uniform1uivEXT(GLint location, GLsizei count, const GLuint *value)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform1uivEXT_Rec *data;
    GLint arraySize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform1uivEXT(location, count, value);
    }

    arraySize = __GL64PAD(count * sizeof(GLuint));

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Uniform1uivEXT_Rec) + arraySize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_Uniform1uivEXT;
    data = (struct __gllc_Uniform1uivEXT_Rec *)(dlop + 1);
    data->location = location;
    data->count = count;
    __GL_MEMCOPY((GLubyte *)(data + 1), value, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Uniform2uivEXT(GLint location, GLsizei count, const GLuint *value)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform2uivEXT_Rec *data;
    GLint arraySize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform2uivEXT(location, count, value);
    }

    arraySize = __GL64PAD(count * 2 * sizeof(GLuint));

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Uniform2uivEXT_Rec) + arraySize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_Uniform2uivEXT;
    data = (struct __gllc_Uniform2uivEXT_Rec *)(dlop + 1);
    data->location = location;
    data->count = count;
    __GL_MEMCOPY((GLubyte *)(data + 1), value, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Uniform3uivEXT(GLint location, GLsizei count, const GLuint *value)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform3uivEXT_Rec *data;
    GLint arraySize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform3uivEXT(location, count, value);
    }

    arraySize = __GL64PAD(count * 3 * sizeof(GLuint));

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Uniform3uivEXT_Rec) + arraySize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_Uniform3uivEXT;
    data = (struct __gllc_Uniform3uivEXT_Rec *)(dlop + 1);
    data->location = location;
    data->count = count;
    __GL_MEMCOPY((GLubyte *)(data + 1), value, arraySize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Uniform4uivEXT(GLint location, GLsizei count, const GLuint *value)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform4uivEXT_Rec *data;
    GLint arraySize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform4uivEXT(location, count, value);
    }

    arraySize = __GL64PAD(count * 4 * sizeof(GLuint));

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Uniform4uivEXT_Rec) + arraySize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_Uniform4uivEXT;
    data = (struct __gllc_Uniform4uivEXT_Rec *)(dlop + 1);
    data->location = location;
    data->count = count;
    __GL_MEMCOPY((GLubyte *)(data + 1), value, arraySize);
    __glDlistAppendOp(gc, dlop);

}
#endif

#if GL_EXT_geometry_shader4
GLvoid APIENTRY __gllc_FramebufferTextureEXT(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
    __GLdlistOp *dlop;
    struct __gllc_FramebufferTextureEXT_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_FramebufferTextureEXT(target, attachment, texture, level);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_FramebufferTextureEXT_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_FramebufferTextureEXT;
    data = (struct __gllc_FramebufferTextureEXT_Rec *)(dlop + 1);
    data->target = target;
    data->attachment = attachment;
    data->texture = texture;
    data->level = level;
    __glDlistAppendOp(gc, dlop);

}
GLvoid APIENTRY __gllc_FramebufferTextureLayerEXT(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    __GLdlistOp *dlop;
    struct __gllc_FramebufferTextureLayerEXT_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_FramebufferTextureLayerEXT(target, attachment, texture, level, layer);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_FramebufferTextureLayerEXT_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_FramebufferTextureLayerEXT;
    data = (struct __gllc_FramebufferTextureLayerEXT_Rec *)(dlop + 1);
    data->target = target;
    data->attachment = attachment;
    data->texture = texture;
    data->layer = layer;
    __glDlistAppendOp(gc, dlop);

}

GLvoid APIENTRY __gllc_FramebufferTextureFaceEXT(GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face)
{
    __GLdlistOp *dlop;
    struct __gllc_FramebufferTextureFaceEXT_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_FramebufferTextureFaceEXT(target, attachment, texture, level, face);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_FramebufferTextureFaceEXT_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_FramebufferTextureFaceEXT;
    data = (struct __gllc_FramebufferTextureFaceEXT_Rec *)(dlop + 1);
    data->target = target;
    data->attachment = attachment;
    data->texture = texture;
    data->face = face;
    __glDlistAppendOp(gc, dlop);

}

#endif

#if GL_EXT_draw_buffers2

GLvoid APIENTRY __gllc_ColorMaskIndexedEXT(GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
    __GLdlistOp *dlop;
    struct __gllc_ColorMaskIndexedEXT_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ColorMaskIndexedEXT(buf, r, g, b, a);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ColorMaskIndexedEXT_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ColorMaskIndexedEXT;
    data = (struct __gllc_ColorMaskIndexedEXT_Rec *)(dlop + 1);
    data->buf = buf;
    data->r = r;
    data->g = g;
    data->b = b;
    data->a = a;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_EnableIndexedEXT(GLenum target, GLuint index)
{
    __GLdlistOp *dlop;
    struct __gllc_EnableIndexedEXT_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EnableIndexedEXT(target, index);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_EnableIndexedEXT_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_EnableIndexedEXT;
    data = (struct __gllc_EnableIndexedEXT_Rec *)(dlop + 1);
    data->target = target;
    data->index = index;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_DisableIndexedEXT(GLenum target, GLuint index)
{
    __GLdlistOp *dlop;
    struct __gllc_DisableIndexedEXT_Rec *data;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_DisableIndexedEXT(target, index);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_DisableIndexedEXT_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_DisableIndexedEXT;
    data = (struct __gllc_DisableIndexedEXT_Rec *)(dlop + 1);
    data->target = target;
    data->index = index;
    __glDlistAppendOp(gc, dlop);
}

#endif

#if GL_EXT_gpu_program_parameters

GLvoid APIENTRY __gllc_ProgramEnvParameters4fvEXT(GLenum target, GLuint index, GLsizei count, const GLfloat *params)
{
    __GLdlistOp *dlop;
    struct __gllc_ProgramEnvParameters4fvEXT_Rec *data;
    GLint paramSize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ProgramEnvParameters4fvEXT(target, index, count, params);
    }

    paramSize = __GL64PAD(count * 4 * sizeof(GLfloat));

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ColorMaskIndexedEXT_Rec) + paramSize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_ProgramEnvParameters4fvEXT;
    data = (struct __gllc_ProgramEnvParameters4fvEXT_Rec *)(dlop + 1);
    data->target = target;
    data->index = index;
    data->count = count;
    __GL_MEMCOPY((GLubyte *)(data + 1), params, paramSize);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ProgramLocalParameters4fvEXT(GLenum target, GLuint index, GLsizei count, const GLfloat *params)
{
    __GLdlistOp *dlop;
    struct __gllc_ProgramLocalParameters4fvEXT_Rec *data;
    GLint paramSize;
    __GL_SETUP();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ProgramLocalParameters4fvEXT(target, index, count, params);
    }

    paramSize = __GL64PAD(count * 4 * sizeof(GLfloat));

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ProgramLocalParameters4fvEXT_Rec) + paramSize);
    if (dlop == NULL) return;
    dlop->opcode = __glop_ProgramLocalParameters4fvEXT;
    data = (struct __gllc_ProgramLocalParameters4fvEXT_Rec *)(dlop + 1);
    data->target = target;
    data->index = index;
    data->count = count;
    __GL_MEMCOPY((GLubyte *)(data + 1), params, paramSize);
    __glDlistAppendOp(gc, dlop);
}

#endif

#if GL_ARB_color_buffer_float
GLvoid APIENTRY __gllc_ClampColorARB(GLenum target, GLenum clamp)
{
    __GLdlistOp *dlop;
    struct __gllc_ClampColorARB_Rec *data;
    __GL_SETUP();

    if(gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ClampColorARB(target, clamp);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ClampColorARB_Rec));
    if(dlop == NULL) return;
    dlop->opcode = __glop_ClampColorARB;
    data = (struct __gllc_ClampColorARB_Rec*)(dlop + 1);
    data->target = target;
    data->clamp = clamp;
    __glDlistAppendOp(gc, dlop);
}
#endif

#if GL_ATI_separate_stencil
GLvoid APIENTRY __gllc_StencilFuncSeparateATI(GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask)
{
    __GLdlistOp *dlop;
    struct __gllc_StencilFuncSeparateATI_Rec* data;
    __GL_SETUP();

    if(gc->dlist.mode == GL_COMPILE_AND_EXECUTE)
    {
        __glim_StencilFuncSeparateATI(frontfunc, backfunc, ref, mask);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_StencilFuncSeparateATI_Rec));

    if(dlop == NULL) return;

    dlop->opcode = __glop_StencilFuncSeparateATI;
    data = (struct __gllc_StencilFuncSeparateATI_Rec*)(dlop + 1);
    data->frontfunc = frontfunc;
    data->backfunc = backfunc;
    data->ref = ref;
    data->mask = mask;
    __glDlistAppendOp(gc, dlop);
}
#endif

#if defined(_WIN32)
#pragma warning(default: 4244)
#endif
