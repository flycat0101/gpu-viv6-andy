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


#include "gc_es_context.h"
#include "gc_gl_dlist.h"
#include "g_lcomp.h"
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

extern GLboolean __glCheckTexImgArgs(__GLcontext *gc, __GLtextureObject *tex, GLint   lod, GLsizei width, GLsizei height, GLsizei depth, GLint   border);
extern GLboolean __glCheckTexCopyImgFmt(__GLcontext *gc, __GLtextureObject * tex, GLint internalFormat, GLboolean compSizeMatch);
extern GLboolean __glCheckTexImgTypeArg(__GLcontext *gc, __GLtextureObject *tex, GLenum type);
extern GLboolean __glCheckTexImgInternalFmtArg(__GLcontext *gc, __GLtextureObject *tex, GLenum internalFormat);
extern GLboolean __glCheckTexImgFmt(__GLcontext *gc, __GLtextureObject *tex, GLenum target, GLint internalFormat, GLenum format, GLenum type);
extern GLboolean __glCheckTexImgFmtArg(__GLcontext *gc, __GLtextureObject *tex, GLenum format);
extern GLboolean __glCheckTexSubImgArgs(__GLcontext *gc, __GLtextureObject *tex, GLuint  face, GLint   lod, GLint   xoffset, GLint   yoffset, GLint   zoffset, GLsizei width, GLsizei height, GLsizei depth);
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

GLvoid APIENTRY __gllc_Begin(__GLcontext *gc, GLenum mode)
{
    __GLdlistOp *dlop;
    struct __gllc_Begin_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Begin(gc, mode);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Begin_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Begin;
    data = (struct __gllc_Begin_Rec *)(dlop + 1);
    data->primType = mode;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_End(__GLcontext *gc)
{
    __GLdlistOp *dlop;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
         if(gc->currentImmediateTable->End == __glim_End_Material)
            __glim_End_Material(gc);
        else
            __glim_End(gc);
    }

    dlop = __glDlistAllocOp(gc, 0);
    if (dlop == NULL) return;
    dlop->opcode = __glop_End;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ListBase(__GLcontext *gc, GLuint base)
{
    __GLdlistOp *dlop;
    struct __gllc_ListBase_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ListBase(gc, base);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ListBase_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ListBase;
    data = (struct __gllc_ListBase_Rec *)(dlop + 1);
    data->base = base;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_EdgeFlagv(__GLcontext *gc, const GLboolean *flag)
{
    __GLdlistOp *dlop;
    struct __gllc_EdgeFlag_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->EdgeFlagv)(gc, flag);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_EdgeFlag_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_EdgeFlag;
    data = (struct __gllc_EdgeFlag_Rec *)(dlop + 1);
    data->flag = flag[0];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_EdgeFlag(__GLcontext *gc, GLboolean flag)
{
    __GLdlistOp *dlop;
    struct __gllc_EdgeFlag_Rec *data;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->EdgeFlag)(gc, flag);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_EdgeFlag_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_EdgeFlag;
    data = (struct __gllc_EdgeFlag_Rec *)(dlop + 1);
    data->flag = flag;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Indexd(__GLcontext *gc, GLdouble c)
{
    __GLdlistOp *dlop;
    struct __gllc_Indexf_Rec *data;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Indexd(gc, c);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Indexf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Indexf;
    data = (struct __gllc_Indexf_Rec *)(dlop + 1);
    data->c = c;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Indexdv(__GLcontext *gc, const GLdouble *c)
{
    __GLdlistOp *dlop;
    struct __gllc_Indexf_Rec *data;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Indexdv(gc, c);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Indexf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Indexf;
    data = (struct __gllc_Indexf_Rec *)(dlop + 1);
    data->c = c[0];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Indexf(__GLcontext *gc, GLfloat c)
{
    __GLdlistOp *dlop;
    struct __gllc_Indexf_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Indexf(gc, c);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Indexf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Indexf;
    data = (struct __gllc_Indexf_Rec *)(dlop + 1);
    data->c = c;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Indexfv(__GLcontext *gc, const GLfloat *c)
{
    __GLdlistOp *dlop;
    struct __gllc_Indexf_Rec *data;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Indexfv(gc, c);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Indexf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Indexf;
    data = (struct __gllc_Indexf_Rec *)(dlop + 1);
    data->c = c[0];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Indexi(__GLcontext *gc, GLint c)
{
    __GLdlistOp *dlop;
    struct __gllc_Indexf_Rec *data;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Indexi(gc, c);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Indexf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Indexf;
    data = (struct __gllc_Indexf_Rec *)(dlop + 1);
    data->c = c;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Indexiv(__GLcontext *gc, const GLint *c)
{
    __GLdlistOp *dlop;
    struct __gllc_Indexf_Rec *data;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Indexiv(gc, c);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Indexf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Indexf;
    data = (struct __gllc_Indexf_Rec *)(dlop + 1);
    data->c = c[0];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Indexs(__GLcontext *gc, GLshort c)
{
    __GLdlistOp *dlop;
    struct __gllc_Indexf_Rec *data;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Indexs(gc, c);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Indexf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Indexf;
    data = (struct __gllc_Indexf_Rec *)(dlop + 1);
    data->c = c;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Indexsv(__GLcontext *gc, const GLshort *c)
{
    __GLdlistOp *dlop;
    struct __gllc_Indexf_Rec *data;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Indexsv(gc, c);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Indexf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Indexf;
    data = (struct __gllc_Indexf_Rec *)(dlop + 1);
    data->c = c[0];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Indexub(__GLcontext *gc, GLubyte c)
{
    __GLdlistOp *dlop;
    struct __gllc_Indexf_Rec *data;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Indexub(gc, c);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Indexf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Indexf;
    data = (struct __gllc_Indexf_Rec *)(dlop + 1);
    data->c = c;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Indexubv(__GLcontext *gc, const GLubyte *c)
{
    __GLdlistOp *dlop;
    struct __gllc_Indexf_Rec *data;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Indexubv(gc, c);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Indexf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Indexf;
    data = (struct __gllc_Indexf_Rec *)(dlop + 1);
    data->c = c[0];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos2d(__GLcontext *gc, GLdouble x, GLdouble y)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos2fv_Rec *data;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos2d(gc, x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos2fv;
    data = (struct __gllc_RasterPos2fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos2dv(__GLcontext *gc, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos2fv_Rec *data;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos2dv(gc, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos2fv;
    data = (struct __gllc_RasterPos2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos2f(__GLcontext *gc, GLfloat x, GLfloat y)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos2f(gc, x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos2fv;
    data = (struct __gllc_RasterPos2fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos2fv(__GLcontext *gc, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos2fv(gc, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos2fv;
    data = (struct __gllc_RasterPos2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos2i(__GLcontext *gc, GLint x, GLint y)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos2i(gc, x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos2fv;
    data = (struct __gllc_RasterPos2fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos2iv(__GLcontext *gc, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos2fv_Rec *data;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos2iv(gc, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos2fv;
    data = (struct __gllc_RasterPos2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos2s(__GLcontext *gc, GLshort x, GLshort y)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos2fv_Rec *data;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos2s(gc, x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos2fv;
    data = (struct __gllc_RasterPos2fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos2sv(__GLcontext *gc, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos2fv_Rec *data;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos2sv(gc, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_RasterPos2fv;
    data = (struct __gllc_RasterPos2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_RasterPos3d(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos3fv_Rec *data;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos3d(gc, x, y, z);
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

GLvoid APIENTRY __gllc_RasterPos3dv(__GLcontext *gc, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos3fv_Rec *data;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos3dv(gc, v);
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

GLvoid APIENTRY __gllc_RasterPos3f(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos3f(gc, x, y, z);
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

GLvoid APIENTRY __gllc_RasterPos3fv(__GLcontext *gc, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos3fv_Rec *data;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos3fv(gc, v);
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

GLvoid APIENTRY __gllc_RasterPos3i(__GLcontext *gc, GLint x, GLint y, GLint z)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos3fv_Rec *data;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos3i(gc, x, y, z);
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

GLvoid APIENTRY __gllc_RasterPos3iv(__GLcontext *gc, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos3iv(gc, v);
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

GLvoid APIENTRY __gllc_RasterPos3s(__GLcontext *gc, GLshort x, GLshort y, GLshort z)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos3s(gc, x, y, z);
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

GLvoid APIENTRY __gllc_RasterPos3sv(__GLcontext *gc, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos3sv(gc, v);
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

GLvoid APIENTRY __gllc_RasterPos4d(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos4d(gc, x, y, z, w);
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

GLvoid APIENTRY __gllc_RasterPos4dv(__GLcontext *gc, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos4dv(gc, v);
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

GLvoid APIENTRY __gllc_RasterPos4f(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos4f(gc, x, y, z, w);
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

GLvoid APIENTRY __gllc_RasterPos4fv(__GLcontext *gc, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos4fv(gc, v);
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

GLvoid APIENTRY __gllc_RasterPos4i(__GLcontext *gc, GLint x, GLint y, GLint z, GLint w)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos4i(gc, x, y, z, w);
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

GLvoid APIENTRY __gllc_RasterPos4iv(__GLcontext *gc, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos4iv(gc, v);
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

GLvoid APIENTRY __gllc_RasterPos4s(__GLcontext *gc, GLshort x, GLshort y, GLshort z, GLshort w)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos4s(gc, x, y, z, w);
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

GLvoid APIENTRY __gllc_RasterPos4sv(__GLcontext *gc, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_RasterPos4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_RasterPos4sv(gc, v);
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

GLvoid APIENTRY __gllc_Rectd(__GLcontext *gc, GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
    __GLdlistOp *dlop;
    struct __gllc_Rectf_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Rectd(gc, x1, y1, x2, y2);
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

GLvoid APIENTRY __gllc_Rectdv(__GLcontext *gc, const GLdouble *v1, const GLdouble *v2)
{
    __GLdlistOp *dlop;
    struct __gllc_Rectf_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Rectdv(gc, v1, v2);
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

GLvoid APIENTRY __gllc_Rectf(__GLcontext *gc, GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
    __GLdlistOp *dlop;
    struct __gllc_Rectf_Rec *data;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Rectf(gc, x1, y1, x2, y2);
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

GLvoid APIENTRY __gllc_Rectfv(__GLcontext *gc, const GLfloat *v1, const GLfloat *v2)
{
    __GLdlistOp *dlop;
    struct __gllc_Rectf_Rec *data;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Rectfv(gc, v1, v2);
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

GLvoid APIENTRY __gllc_Recti(__GLcontext *gc, GLint x1, GLint y1, GLint x2, GLint y2)
{
    __GLdlistOp *dlop;
    struct __gllc_Rectf_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Recti(gc, x1, y1, x2, y2);
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

GLvoid APIENTRY __gllc_Rectiv(__GLcontext *gc, const GLint *v1, const GLint *v2)
{
    __GLdlistOp *dlop;
    struct __gllc_Rectf_Rec *data;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Rectiv(gc, v1, v2);
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

GLvoid APIENTRY __gllc_Rects(__GLcontext *gc, GLshort x1, GLshort y1, GLshort x2, GLshort y2)
{
    __GLdlistOp *dlop;
    struct __gllc_Rectf_Rec *data;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Rects(gc, x1, y1, x2, y2);
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

GLvoid APIENTRY __gllc_Rectsv(__GLcontext *gc, const GLshort *v1, const GLshort *v2)
{
    __GLdlistOp *dlop;
    struct __gllc_Rectf_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Rectsv(gc, v1, v2);
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

GLvoid APIENTRY __gllc_ClipPlane(__GLcontext *gc, GLenum plane, const GLdouble *equation)
{
    __GLdlistOp *dlop;
    struct __gllc_ClipPlane_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ClipPlane(gc, plane, equation);
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

GLvoid APIENTRY __gllc_ColorMaterial(__GLcontext *gc, GLenum face, GLenum mode)
{
    __GLdlistOp *dlop;
    struct __gllc_ColorMaterial_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ColorMaterial(gc, face, mode);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ColorMaterial_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ColorMaterial;
    data = (struct __gllc_ColorMaterial_Rec *)(dlop + 1);
    data->face = face;
    data->mode = mode;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_CullFace(__GLcontext *gc, GLenum mode)
{
    __GLdlistOp *dlop;
    struct __gllc_CullFace_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_CullFace(gc, mode);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_CullFace_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_CullFace;
    data = (struct __gllc_CullFace_Rec *)(dlop + 1);
    data->mode = mode;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Fogf(__GLcontext *gc, GLenum pname, GLfloat param)
{


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Fogf(gc, pname, param);
    }

    if (__glFog_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_Fogfv(gc, pname, &param);
}

GLvoid APIENTRY __gllc_Fogfv(__GLcontext *gc, GLenum pname, const GLfloat *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_Fogfv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Fogfv(gc, pname, params);
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

GLvoid APIENTRY __gllc_Fogi(__GLcontext *gc, GLenum pname, GLint param)
{

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Fogi(gc, pname, param);
    }

    if (__glFog_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_Fogiv(gc, pname, &param);
}

GLvoid APIENTRY __gllc_Fogiv(__GLcontext *gc, GLenum pname, const GLint *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_Fogiv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Fogiv(gc, pname, params);
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

GLvoid APIENTRY __gllc_FrontFace(__GLcontext *gc, GLenum mode)
{
    __GLdlistOp *dlop;
    struct __gllc_FrontFace_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_FrontFace(gc, mode);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_FrontFace_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_FrontFace;
    data = (struct __gllc_FrontFace_Rec *)(dlop + 1);
    data->mode = mode;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Hint(__GLcontext *gc, GLenum target, GLenum mode)
{
    __GLdlistOp *dlop;
    struct __gllc_Hint_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Hint(gc, target, mode);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Hint_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Hint;
    data = (struct __gllc_Hint_Rec *)(dlop + 1);
    data->target = target;
    data->mode = mode;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Lightf(__GLcontext *gc, GLenum light, GLenum pname, GLfloat param)
{


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Lightf(gc, light, pname, param);
    }

    if (__glLight_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_Lightfv(gc, light, pname, &param);
}

GLvoid APIENTRY __gllc_Lightfv(__GLcontext *gc, GLenum light, GLenum pname, const GLfloat *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_Lightfv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Lightfv(gc, light, pname, params);
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

GLvoid APIENTRY __gllc_Lighti(__GLcontext *gc, GLenum light, GLenum pname, GLint param)
{


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Lighti(gc, light, pname, param);
    }

    if (__glLight_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_Lightiv(gc, light, pname, &param);
}

GLvoid APIENTRY __gllc_Lightiv(__GLcontext *gc, GLenum light, GLenum pname, const GLint *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_Lightiv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Lightiv(gc, light, pname, params);
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

GLvoid APIENTRY __gllc_LightModelf(__GLcontext *gc, GLenum pname, GLfloat param)
{


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_LightModelf(gc, pname, param);
    }

    if (__glLightModel_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_LightModelfv(gc, pname, &param);
}

GLvoid APIENTRY __gllc_LightModelfv(__GLcontext *gc, GLenum pname, const GLfloat *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_LightModelfv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_LightModelfv(gc, pname, params);
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

GLvoid APIENTRY __gllc_LightModeli(__GLcontext *gc, GLenum pname, GLint param)
{


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_LightModeli(gc, pname, param);
    }

    if (__glLightModel_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_LightModeliv(gc, pname, &param);
}

GLvoid APIENTRY __gllc_LightModeliv(__GLcontext *gc, GLenum pname, const GLint *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_LightModeliv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_LightModeliv(gc, pname, params);
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

GLvoid APIENTRY __gllc_LineStipple(__GLcontext *gc, GLint factor, GLushort pattern)
{
    __GLdlistOp *dlop;
    struct __gllc_LineStipple_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_LineStipple(gc, factor, pattern);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_LineStipple_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_LineStipple;
    data = (struct __gllc_LineStipple_Rec *)(dlop + 1);
    data->factor = factor;
    data->pattern = pattern;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_LineWidth(__GLcontext *gc, GLfloat width)
{
    __GLdlistOp *dlop;
    struct __gllc_LineWidth_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_LineWidth(gc, width);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_LineWidth_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_LineWidth;
    data = (struct __gllc_LineWidth_Rec *)(dlop + 1);
    data->width = width;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Materialf(__GLcontext *gc, GLenum face, GLenum pname, GLfloat param)
{


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Materialf(gc, face, pname, param);
    }

    if (__glMaterial_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_Materialfv(gc, face, pname, &param);
}

GLvoid APIENTRY __gllc_Materialfv(__GLcontext *gc, GLenum face, GLenum pname, const GLfloat *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    GLenum error;
    struct __gllc_Materialfv_Rec *data;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Materialfv(gc, face, pname, params);
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

GLvoid APIENTRY __gllc_Materiali(__GLcontext *gc, GLenum face, GLenum pname, GLint param)
{

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Materiali(gc, face, pname, param);
    }

    if (__glMaterial_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_Materialiv(gc, face, pname, &param);
}

GLvoid APIENTRY __gllc_Materialiv(__GLcontext *gc, GLenum face, GLenum pname, const GLint *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    GLenum error;
    struct __gllc_Materialiv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Materialiv(gc, face, pname, params);
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

GLvoid APIENTRY __gllc_PointSize(__GLcontext *gc, GLfloat size)
{
    __GLdlistOp *dlop;
    struct __gllc_PointSize_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PointSize(gc, size);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_PointSize_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_PointSize;
    data = (struct __gllc_PointSize_Rec *)(dlop + 1);
    data->size = size;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PolygonMode(__GLcontext *gc, GLenum face, GLenum mode)
{
    __GLdlistOp *dlop;
    struct __gllc_PolygonMode_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PolygonMode(gc, face, mode);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_PolygonMode_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_PolygonMode;
    data = (struct __gllc_PolygonMode_Rec *)(dlop + 1);
    data->face = face;
    data->mode = mode;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Scissor(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GLdlistOp *dlop;
    struct __gllc_Scissor_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Scissor(gc, x, y, width, height);
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

GLvoid APIENTRY __gllc_ShadeModel(__GLcontext *gc, GLenum mode)
{
    __GLdlistOp *dlop;
    struct __gllc_ShadeModel_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ShadeModel(gc, mode);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ShadeModel_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ShadeModel;
    data = (struct __gllc_ShadeModel_Rec *)(dlop + 1);
    data->mode = mode;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexParameterf(__GLcontext *gc, GLenum target, GLenum pname, GLfloat param)
{

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexParameterf(gc, target, pname, param);
    }

    if (__glTexParameter_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_TexParameterfv(gc, target, pname, &param);
}

GLvoid APIENTRY __gllc_TexParameterfv(__GLcontext *gc, GLenum target, GLenum pname, const GLfloat *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_TexParameterfv_Rec *data;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexParameterfv(gc, target, pname, params);
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

GLvoid APIENTRY __gllc_TexParameteri(__GLcontext *gc, GLenum target, GLenum pname, GLint param)
{

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexParameteri(gc, target, pname, param);
    }

    if (__glTexParameter_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_TexParameteriv(gc, target, pname, &param);
}

GLvoid APIENTRY __gllc_TexParameteriv(__GLcontext *gc, GLenum target, GLenum pname, const GLint *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_TexParameteriv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexParameteriv(gc, target, pname, params);
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

GLvoid APIENTRY __gllc_TexEnvf(__GLcontext *gc, GLenum target, GLenum pname, GLfloat param)
{

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexEnvf(gc, target, pname, param);
    }

    if (__glTexEnv_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_TexEnvfv(gc, target, pname, &param);
}

GLvoid APIENTRY __gllc_TexEnvfv(__GLcontext *gc, GLenum target, GLenum pname, const GLfloat *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_TexEnvfv_Rec *data;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexEnvfv(gc, target, pname, params);
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

GLvoid APIENTRY __gllc_TexEnvi(__GLcontext *gc, GLenum target, GLenum pname, GLint param)
{

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexEnvi(gc, target, pname, param);
    }

    if (__glTexEnv_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_TexEnviv(gc, target, pname, &param);
}

GLvoid APIENTRY __gllc_TexEnviv(__GLcontext *gc, GLenum target, GLenum pname, const GLint *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_TexEnviv_Rec *data;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexEnviv(gc, target, pname, params);
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

GLvoid APIENTRY __gllc_TexGend(__GLcontext *gc, GLenum coord, GLenum pname, GLdouble param)
{

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexGend(gc, coord, pname, param);
    }

    if (__glTexGen_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_TexGendv(gc, coord, pname, &param);
}

GLvoid APIENTRY __gllc_TexGendv(__GLcontext *gc, GLenum coord, GLenum pname, const GLdouble *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_TexGendv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexGendv(gc, coord, pname, params);
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

GLvoid APIENTRY __gllc_TexGenf(__GLcontext *gc, GLenum coord, GLenum pname, GLfloat param)
{


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexGenf(gc, coord, pname, param);
    }

    if (__glTexGen_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_TexGenfv(gc, coord, pname, &param);
}

GLvoid APIENTRY __gllc_TexGenfv(__GLcontext *gc, GLenum coord, GLenum pname, const GLfloat *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_TexGenfv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexGenfv(gc, coord, pname, params);
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

GLvoid APIENTRY __gllc_TexGeni(__GLcontext *gc, GLenum coord, GLenum pname, GLint param)
{


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexGeni(gc, coord, pname, param);
    }

    if (__glTexGen_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_TexGeniv(gc, coord, pname, &param);
}

GLvoid APIENTRY __gllc_TexGeniv(__GLcontext *gc, GLenum coord, GLenum pname, const GLint *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_TexGeniv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexGeniv(gc, coord, pname, params);
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

GLvoid APIENTRY __gllc_InitNames(__GLcontext *gc)
{
    __GLdlistOp *dlop;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_InitNames(gc);
    }

    dlop = __glDlistAllocOp(gc, 0);
    if (dlop == NULL) return;
    dlop->opcode = __glop_InitNames;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_LoadName(__GLcontext *gc, GLuint name)
{
    __GLdlistOp *dlop;
    struct __gllc_LoadName_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_LoadName(gc, name);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_LoadName_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_LoadName;
    data = (struct __gllc_LoadName_Rec *)(dlop + 1);
    data->name = name;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PassThrough(__GLcontext *gc, GLfloat token)
{
    __GLdlistOp *dlop;
    struct __gllc_PassThrough_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PassThrough(gc, token);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_PassThrough_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_PassThrough;
    data = (struct __gllc_PassThrough_Rec *)(dlop + 1);
    data->token = token;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PopName(__GLcontext *gc)
{
    __GLdlistOp *dlop;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PopName(gc);
    }

    dlop = __glDlistAllocOp(gc, 0);
    if (dlop == NULL) return;
    dlop->opcode = __glop_PopName;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PushName(__GLcontext *gc, GLuint name)
{
    __GLdlistOp *dlop;
    struct __gllc_PushName_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PushName(gc, name);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_PushName_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_PushName;
    data = (struct __gllc_PushName_Rec *)(dlop + 1);
    data->name = name;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_DrawBuffer(__GLcontext *gc, GLenum mode)
{
    __GLdlistOp *dlop;
    struct __gllc_DrawBuffer_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_DrawBuffer(gc, mode);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_DrawBuffer_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_DrawBuffer;
    data = (struct __gllc_DrawBuffer_Rec *)(dlop + 1);
    data->mode = mode;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Clear(__GLcontext *gc, GLbitfield mask)
{
    __GLdlistOp *dlop;
    struct __gllc_Clear_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Clear(gc, mask);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Clear_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Clear;
    data = (struct __gllc_Clear_Rec *)(dlop + 1);
    data->mask = mask;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ClearAccum(__GLcontext *gc, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    __GLdlistOp *dlop;
    struct __gllc_ClearAccum_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ClearAccum(gc, red, green, blue, alpha);
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

GLvoid APIENTRY __gllc_ClearIndex(__GLcontext *gc, GLfloat c)
{
    __GLdlistOp *dlop;
    struct __gllc_ClearIndex_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ClearIndex(gc, c);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ClearIndex_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ClearIndex;
    data = (struct __gllc_ClearIndex_Rec *)(dlop + 1);
    data->c = c;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ClearColor(__GLcontext *gc, GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    __GLdlistOp *dlop;
    struct __gllc_ClearColor_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ClearColor(gc, red, green, blue, alpha);
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

GLvoid APIENTRY __gllc_ClearStencil(__GLcontext *gc, GLint s)
{
    __GLdlistOp *dlop;
    struct __gllc_ClearStencil_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ClearStencil(gc, s);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ClearStencil_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ClearStencil;
    data = (struct __gllc_ClearStencil_Rec *)(dlop + 1);
    data->s = s;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ClearDepth(__GLcontext *gc, GLclampd depth)
{
    __GLdlistOp *dlop;
    struct __gllc_ClearDepth_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ClearDepth(gc, depth);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ClearDepth_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ClearDepth;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_ClearDepth_Rec *)(dlop + 1);
    data->depth = depth;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_StencilMask(__GLcontext *gc, GLuint mask)
{
    __GLdlistOp *dlop;
    struct __gllc_StencilMask_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_StencilMask(gc, mask);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_StencilMask_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_StencilMask;
    data = (struct __gllc_StencilMask_Rec *)(dlop + 1);
    data->mask = mask;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_StencilMaskSeparate(__GLcontext *gc, GLenum face, GLuint mask)
{
    __GLdlistOp *dlop;
    struct __gllc_StencilMaskSeparate_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_StencilMaskSeparate(gc, face, mask);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_StencilMaskSeparate_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_StencilMaskSeparate;
    data = (struct __gllc_StencilMaskSeparate_Rec *)(dlop + 1);
    data->face = face;
    data->mask = mask;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ColorMask(__GLcontext *gc, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    __GLdlistOp *dlop;
    struct __gllc_ColorMask_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ColorMask(gc, red, green, blue, alpha);
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

GLvoid APIENTRY __gllc_DepthMask(__GLcontext *gc, GLboolean flag)
{
    __GLdlistOp *dlop;
    struct __gllc_DepthMask_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_DepthMask(gc, flag);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_DepthMask_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_DepthMask;
    data = (struct __gllc_DepthMask_Rec *)(dlop + 1);
    data->flag = flag;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_IndexMask(__GLcontext *gc, GLuint mask)
{
    __GLdlistOp *dlop;
    struct __gllc_IndexMask_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_IndexMask(gc, mask);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_IndexMask_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_IndexMask;
    data = (struct __gllc_IndexMask_Rec *)(dlop + 1);
    data->mask = mask;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Accum(__GLcontext *gc, GLenum op, GLfloat value)
{
    __GLdlistOp *dlop;
    struct __gllc_Accum_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Accum(gc, op, value);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Accum_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Accum;
    data = (struct __gllc_Accum_Rec *)(dlop + 1);
    data->op = op;
    data->value = value;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PopAttrib(__GLcontext *gc)
{
    __GLdlistOp *dlop;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PopAttrib(gc);
    }

    dlop = __glDlistAllocOp(gc, 0);
    if (dlop == NULL) return;
    dlop->opcode = __glop_PopAttrib;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PushAttrib(__GLcontext *gc, GLbitfield mask)
{
    __GLdlistOp *dlop;
    struct __gllc_PushAttrib_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PushAttrib(gc, mask);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_PushAttrib_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_PushAttrib;
    data = (struct __gllc_PushAttrib_Rec *)(dlop + 1);
    data->mask = mask;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MapGrid1d(__GLcontext *gc, GLint un, GLdouble u1, GLdouble u2)
{
    __GLdlistOp *dlop;
    struct __gllc_MapGrid1d_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_MapGrid1d(gc, un, u1, u2);
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

GLvoid APIENTRY __gllc_MapGrid1f(__GLcontext *gc, GLint un, GLfloat u1, GLfloat u2)
{
    __GLdlistOp *dlop;
    struct __gllc_MapGrid1f_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_MapGrid1f(gc, un, u1, u2);
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

GLvoid APIENTRY __gllc_MapGrid2d(__GLcontext *gc, GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2)
{
    __GLdlistOp *dlop;
    struct __gllc_MapGrid2d_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_MapGrid2d(gc, un, u1, u2, vn, v1, v2);
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

GLvoid APIENTRY __gllc_MapGrid2f(__GLcontext *gc, GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2)
{
    __GLdlistOp *dlop;
    struct __gllc_MapGrid2f_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_MapGrid2f(gc, un, u1, u2, vn, v1, v2);
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

GLvoid APIENTRY __gllc_EvalCoord1d(__GLcontext *gc, GLdouble u)
{
    __GLdlistOp *dlop;
    struct __gllc_EvalCoord1d_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EvalCoord1d(gc, u);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_EvalCoord1d_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_EvalCoord1dv;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_EvalCoord1d_Rec *)(dlop + 1);
    data->u = u;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_EvalCoord1dv(__GLcontext *gc, const GLdouble *u)
{
    __GLdlistOp *dlop;
    struct __gllc_EvalCoord1dv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EvalCoord1dv(gc, u);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_EvalCoord1dv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_EvalCoord1dv;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_EvalCoord1dv_Rec *)(dlop + 1);
    data->u[0] = u[0];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_EvalCoord1f(__GLcontext *gc, GLfloat u)
{
    __GLdlistOp *dlop;
    struct __gllc_EvalCoord1f_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EvalCoord1f(gc, u);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_EvalCoord1f_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_EvalCoord1fv;
    data = (struct __gllc_EvalCoord1f_Rec *)(dlop + 1);
    data->u = u;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_EvalCoord1fv(__GLcontext *gc, const GLfloat *u)
{
    __GLdlistOp *dlop;
    struct __gllc_EvalCoord1fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EvalCoord1fv(gc, u);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_EvalCoord1fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_EvalCoord1fv;
    data = (struct __gllc_EvalCoord1fv_Rec *)(dlop + 1);
    data->u[0] = u[0];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_EvalCoord2d(__GLcontext *gc, GLdouble u, GLdouble v)
{
    __GLdlistOp *dlop;
    struct __gllc_EvalCoord2d_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EvalCoord2d(gc, u, v);
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

GLvoid APIENTRY __gllc_EvalCoord2dv(__GLcontext *gc, const GLdouble *u)
{
    __GLdlistOp *dlop;
    struct __gllc_EvalCoord2dv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EvalCoord2dv(gc, u);
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

GLvoid APIENTRY __gllc_EvalCoord2f(__GLcontext *gc, GLfloat u, GLfloat v)
{
    __GLdlistOp *dlop;
    struct __gllc_EvalCoord2f_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EvalCoord2f(gc, u, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_EvalCoord2f_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_EvalCoord2fv;
    data = (struct __gllc_EvalCoord2f_Rec *)(dlop + 1);
    data->u = u;
    data->v = v;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_EvalCoord2fv(__GLcontext *gc, const GLfloat *u)
{
    __GLdlistOp *dlop;
    struct __gllc_EvalCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EvalCoord2fv(gc, u);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_EvalCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_EvalCoord2fv;
    data = (struct __gllc_EvalCoord2fv_Rec *)(dlop + 1);
    data->u[0] = u[0];
    data->u[1] = u[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_EvalMesh1(__GLcontext *gc, GLenum mode, GLint i1, GLint i2)
{
    __GLdlistOp *dlop;
    struct __gllc_EvalMesh1_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EvalMesh1(gc, mode, i1, i2);
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

GLvoid APIENTRY __gllc_EvalPoint1(__GLcontext *gc, GLint i)
{
    __GLdlistOp *dlop;
    struct __gllc_EvalPoint1_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EvalPoint1(gc, i);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_EvalPoint1_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_EvalPoint1;
    data = (struct __gllc_EvalPoint1_Rec *)(dlop + 1);
    data->i = i;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_EvalMesh2(__GLcontext *gc, GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2)
{
    __GLdlistOp *dlop;
    struct __gllc_EvalMesh2_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EvalMesh2(gc, mode, i1, i2, j1, j2);
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

GLvoid APIENTRY __gllc_EvalPoint2(__GLcontext *gc, GLint i, GLint j)
{
    __GLdlistOp *dlop;
    struct __gllc_EvalPoint2_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EvalPoint2(gc, i, j);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_EvalPoint2_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_EvalPoint2;
    data = (struct __gllc_EvalPoint2_Rec *)(dlop + 1);
    data->i = i;
    data->j = j;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_AlphaFunc(__GLcontext *gc, GLenum func, GLclampf ref)
{
    __GLdlistOp *dlop;
    struct __gllc_AlphaFunc_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_AlphaFunc(gc, func, ref);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_AlphaFunc_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_AlphaFunc;
    data = (struct __gllc_AlphaFunc_Rec *)(dlop + 1);
    data->func = func;
    data->ref = ref;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_BlendColor(__GLcontext *gc, GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    __GLdlistOp *dlop;
    struct __gllc_BlendColor_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_BlendColor(gc, r, g, b, a);
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

GLvoid APIENTRY __gllc_BlendFunc(__GLcontext *gc, GLenum sfactor, GLenum dfactor)
{
    __GLdlistOp *dlop;
    struct __gllc_BlendFunc_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_BlendFunc(gc, sfactor, dfactor);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_BlendFunc_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_BlendFunc;
    data = (struct __gllc_BlendFunc_Rec *)(dlop + 1);
    data->sfactor = sfactor;
    data->dfactor = dfactor;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_BlendFuncSeparate(__GLcontext *gc, GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
    __GLdlistOp *dlop;
    struct __gllc_BlendFuncSeparate_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_BlendFuncSeparate(gc, sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
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

GLvoid APIENTRY __gllc_LogicOp(__GLcontext *gc, GLenum opcode)
{
    __GLdlistOp *dlop;
    struct __gllc_LogicOp_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_LogicOp(gc, opcode);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_LogicOp_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_LogicOp;
    data = (struct __gllc_LogicOp_Rec *)(dlop + 1);
    data->opcode = opcode;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_StencilFunc(__GLcontext *gc, GLenum func, GLint ref, GLuint mask)
{
    __GLdlistOp *dlop;
    struct __gllc_StencilFunc_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_StencilFunc(gc, func, ref, mask);
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

GLvoid APIENTRY __gllc_StencilFuncSeparate(__GLcontext *gc, GLenum face, GLenum func, GLint ref, GLuint mask)
{
    __GLdlistOp *dlop;
    struct __gllc_StencilFuncSeparate_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_StencilFuncSeparate(gc, face, func, ref, mask);
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

GLvoid APIENTRY __gllc_StencilOp(__GLcontext *gc, GLenum fail, GLenum zfail, GLenum zpass)
{
    __GLdlistOp *dlop;
    struct __gllc_StencilOp_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_StencilOp(gc, fail, zfail, zpass);
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

GLvoid APIENTRY __gllc_StencilOpSeparate(__GLcontext *gc, GLenum face, GLenum fail, GLenum zfail, GLenum zpass)
{
    __GLdlistOp *dlop;
    struct __gllc_StencilOpSeparate_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_StencilOpSeparate(gc, face, fail, zfail, zpass);
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

GLvoid APIENTRY __gllc_DepthFunc(__GLcontext *gc, GLenum func)
{
    __GLdlistOp *dlop;
    struct __gllc_DepthFunc_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_DepthFunc(gc, func);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_DepthFunc_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_DepthFunc;
    data = (struct __gllc_DepthFunc_Rec *)(dlop + 1);
    data->func = func;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PixelZoom(__GLcontext *gc, GLfloat xfactor, GLfloat yfactor)
{
    __GLdlistOp *dlop;
    struct __gllc_PixelZoom_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PixelZoom(gc, xfactor, yfactor);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_PixelZoom_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_PixelZoom;
    data = (struct __gllc_PixelZoom_Rec *)(dlop + 1);
    data->xfactor = xfactor;
    data->yfactor = yfactor;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PixelTransferf(__GLcontext *gc, GLenum pname, GLfloat param)
{
    __GLdlistOp *dlop;
    struct __gllc_PixelTransferf_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PixelTransferf(gc, pname, param);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_PixelTransferf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_PixelTransferf;
    data = (struct __gllc_PixelTransferf_Rec *)(dlop + 1);
    data->pname = pname;
    data->param = param;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PixelTransferi(__GLcontext *gc, GLenum pname, GLint param)
{
    __GLdlistOp *dlop;
    struct __gllc_PixelTransferi_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PixelTransferi(gc, pname, param);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_PixelTransferi_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_PixelTransferi;
    data = (struct __gllc_PixelTransferi_Rec *)(dlop + 1);
    data->pname = pname;
    data->param = param;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PixelMapfv(__GLcontext *gc, GLenum map, GLint mapsize, const GLfloat *values)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_PixelMapfv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PixelMapfv(gc, map, mapsize, values);
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

GLvoid APIENTRY __gllc_PixelMapuiv(__GLcontext *gc, GLenum map, GLint mapsize, const GLuint *values)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_PixelMapuiv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PixelMapuiv(gc, map, mapsize, values);
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

GLvoid APIENTRY __gllc_PixelMapusv(__GLcontext *gc, GLenum map, GLint mapsize, const GLushort *values)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_PixelMapusv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PixelMapusv(gc, map, mapsize, values);
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

GLvoid APIENTRY __gllc_ReadBuffer(__GLcontext *gc, GLenum mode)
{
    __GLdlistOp *dlop;
    struct __gllc_ReadBuffer_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ReadBuffer(gc, mode);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ReadBuffer_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ReadBuffer;
    data = (struct __gllc_ReadBuffer_Rec *)(dlop + 1);
    data->mode = mode;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_CopyPixels(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)
{
    __GLdlistOp *dlop;
    struct __gllc_CopyPixels_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_CopyPixels(gc, x, y, width, height, type);
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

GLvoid APIENTRY __gllc_DepthRange(__GLcontext *gc, GLclampd zNear, GLclampd zFar)
{
    __GLdlistOp *dlop;
    struct __gllc_DepthRange_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_DepthRange(gc, zNear, zFar);
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
GLvoid APIENTRY __gllc_DepthBoundsEXT(__GLcontext *gc, GLclampd zMin, GLclampd zMax)
{
/* still not added, to do*/
GL_ASSERT(0);
/*
    __GLdlistOp *dlop;
    struct __gllc_DepthBoundTest_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_DepthRange(gc, zMin, zMax);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_DepthBoundTest_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_DepthBoundsEXT;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_DepthBoundTest_Rec *)(dlop + 1);
    data->zMin = zMin;
    data->zMax = zMax;
    __glDlistAppendOp(gc, dlop);
*/
}
#endif

GLvoid APIENTRY __gllc_Frustum(__GLcontext *gc, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
    __GLdlistOp *dlop;
    struct __gllc_Frustum_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Frustum(gc, left, right, bottom, top, zNear, zFar);
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

GLvoid APIENTRY __gllc_LoadIdentity(__GLcontext *gc)
{
    __GLdlistOp *dlop;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_LoadIdentity(gc);
    }

    dlop = __glDlistAllocOp(gc, 0);
    if (dlop == NULL) return;
    dlop->opcode = __glop_LoadIdentity;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_LoadMatrixf(__GLcontext *gc, const GLfloat *m)
{
    __GLdlistOp *dlop;
    struct __gllc_LoadMatrixf_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_LoadMatrixf(gc, m);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_LoadMatrixf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_LoadMatrixf;
    data = (struct __gllc_LoadMatrixf_Rec *)(dlop + 1);
    __GL_MEMCOPY(data->m, m, sizeof(data->m));
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_LoadMatrixd(__GLcontext *gc, const GLdouble *m)
{
    __GLdlistOp *dlop;
    struct __gllc_LoadMatrixd_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_LoadMatrixd(gc, m);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_LoadMatrixd_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_LoadMatrixd;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_LoadMatrixd_Rec *)(dlop + 1);
    __GL_MEMCOPY(data->m, m, sizeof(data->m));
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MatrixMode(__GLcontext *gc, GLenum mode)
{
    __GLdlistOp *dlop;
    struct __gllc_MatrixMode_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_MatrixMode(gc, mode);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MatrixMode_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MatrixMode;
    data = (struct __gllc_MatrixMode_Rec *)(dlop + 1);
    data->mode = mode;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultMatrixf(__GLcontext *gc, const GLfloat *m)
{
    __GLdlistOp *dlop;
    struct __gllc_MultMatrixf_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_MultMatrixf(gc, m);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultMatrixf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MultMatrixf;
    data = (struct __gllc_MultMatrixf_Rec *)(dlop + 1);
    __GL_MEMCOPY(data->m, m, sizeof(data->m));
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultMatrixd(__GLcontext *gc, const GLdouble *m)
{
    __GLdlistOp *dlop;
    struct __gllc_MultMatrixd_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_MultMatrixd(gc, m);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultMatrixd_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MultMatrixd;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_MultMatrixd_Rec *)(dlop + 1);
    __GL_MEMCOPY(data->m, m, sizeof(data->m));
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Ortho(__GLcontext *gc, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
    __GLdlistOp *dlop;
    struct __gllc_Ortho_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Ortho(gc, left, right, bottom, top, zNear, zFar);
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

GLvoid APIENTRY __gllc_PopMatrix(__GLcontext *gc)
{
    __GLdlistOp *dlop;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PopMatrix(gc);
    }

    dlop = __glDlistAllocOp(gc, 0);
    if (dlop == NULL) return;
    dlop->opcode = __glop_PopMatrix;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PushMatrix(__GLcontext *gc)
{
    __GLdlistOp *dlop;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PushMatrix(gc);
    }

    dlop = __glDlistAllocOp(gc, 0);
    if (dlop == NULL) return;
    dlop->opcode = __glop_PushMatrix;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Rotated(__GLcontext *gc, GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    __GLdlistOp *dlop;
    struct __gllc_Rotated_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Rotated(gc, angle, x, y, z);
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

GLvoid APIENTRY __gllc_Rotatef(__GLcontext *gc, GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    __GLdlistOp *dlop;
    struct __gllc_Rotatef_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Rotatef(gc, angle, x, y, z);
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

GLvoid APIENTRY __gllc_Scaled(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z)
{
    __GLdlistOp *dlop;
    struct __gllc_Scaled_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Scaled(gc, x, y, z);
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

GLvoid APIENTRY __gllc_Scalef(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z)
{
    __GLdlistOp *dlop;
    struct __gllc_Scalef_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Scalef(gc, x, y, z);
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

GLvoid APIENTRY __gllc_Translated(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z)
{
    __GLdlistOp *dlop;
    struct __gllc_Translated_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Translated(gc, x, y, z);
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

GLvoid APIENTRY __gllc_Translatef(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z)
{
    __GLdlistOp *dlop;
    struct __gllc_Translatef_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Translatef(gc, x, y, z);
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

GLvoid APIENTRY __gllc_Viewport(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GLdlistOp *dlop;
    struct __gllc_Viewport_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Viewport(gc, x, y, width, height);
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

GLvoid APIENTRY __gllc_PolygonOffset(__GLcontext *gc, GLfloat factor, GLfloat units)
{
    __GLdlistOp *dlop;
    struct __gllc_PolygonOffset_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PolygonOffset(gc, factor, units);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_PolygonOffset_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_PolygonOffset;
    data = (struct __gllc_PolygonOffset_Rec *)(dlop + 1);
    data->factor = factor;
    data->units = units;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_BindTexture(__GLcontext *gc, GLenum target, GLuint texture)
{
    __GLdlistOp *dlop;
    struct __gllc_BindTexture_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_BindTexture(gc, target, texture);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_BindTexture_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_BindTexture;
    data = (struct __gllc_BindTexture_Rec *)(dlop + 1);
    data->target = target;
    data->texture = texture;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PrioritizeTextures(__GLcontext *gc, GLsizei n, const GLuint *textures, const GLclampf *priorities)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize0;
    GLint arraySize1;
    struct __gllc_PrioritizeTextures_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PrioritizeTextures(gc, n, textures, priorities);
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

GLvoid APIENTRY __gllc_ActiveTexture(__GLcontext *gc, GLenum texture)
{
    __GLdlistOp *dlop;
    struct __gllc_ActiveTexture_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ActiveTexture(gc, texture);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_ActiveTexture_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_ActiveTexture;
    data = (struct __gllc_ActiveTexture_Rec *)(dlop + 1);
    data->texture = texture;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_BlendEquation(__GLcontext *gc, GLenum mode)
{
    __GLdlistOp *dlop;
    struct __gllc_BlendEquation_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_BlendEquation(gc, mode);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_BlendEquation_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_BlendEquation;
    data  = (struct __gllc_BlendEquation_Rec *)(dlop + 1);
    data->mode = mode;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_BlendEquationSeparate(__GLcontext *gc, GLenum modeRGB, GLenum modeAlpha)
{
    __GLdlistOp *dlop;
    struct __gllc_BlendEquationSeparate_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_BlendEquationSeparate(gc, modeRGB, modeAlpha);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_BlendEquationSeparate_Rec));
    if (dlop == NULL) return;
    dlop->opcode  = __glop_BlendEquationSeparate;
    data  = (struct __gllc_BlendEquationSeparate_Rec *)(dlop + 1);
    data->modeRGB = modeRGB;
    data->modeAlpha = modeAlpha;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_ColorTableParameteriv(__GLcontext *gc, GLenum target, GLenum pname, const GLint *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_ColorTableParameteriv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ColorTableParameteriv(gc, target, pname, params);
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

GLvoid APIENTRY __gllc_ColorTableParameterfv(__GLcontext *gc, GLenum target, GLenum pname, const GLfloat *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_ColorTableParameterfv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ColorTableParameterfv(gc, target, pname, params);
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

GLvoid APIENTRY __gllc_ConvolutionParameteri(__GLcontext *gc, GLenum target, GLenum pname, GLint param)
{


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ConvolutionParameteri(gc, target, pname, param);
    }

    if (__glConvolutionParameter_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_ConvolutionParameteriv(gc, target, pname, &param);
}

GLvoid APIENTRY __gllc_ConvolutionParameteriv(__GLcontext *gc, GLenum target, GLenum pname, const GLint* params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_ConvolutionParameteriv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ConvolutionParameteriv(gc, target, pname, params);
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

GLvoid APIENTRY __gllc_ConvolutionParameterfv(__GLcontext *gc, GLenum target, GLenum pname, const GLfloat* params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint arraySize;
    struct __gllc_ConvolutionParameterfv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ConvolutionParameterfv(gc, target, pname, params);
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

GLvoid APIENTRY __gllc_ConvolutionParameterf(__GLcontext *gc, GLenum target, GLenum pname, GLfloat param)
{


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ConvolutionParameterf(gc, target, pname, param);
    }

    if (__glConvolutionParameter_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_ConvolutionParameterfv(gc, target, pname, &param);
}

GLvoid APIENTRY __gllc_Color3b(__GLcontext *gc, GLbyte red, GLbyte green, GLbyte blue)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4ubv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color3b)(gc, red, green, blue);
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

GLvoid APIENTRY __gllc_Color3bv(__GLcontext *gc, const GLbyte *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4ubv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color3bv)(gc, v);
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

GLvoid APIENTRY __gllc_Color3d(__GLcontext *gc, GLdouble red, GLdouble green, GLdouble blue)
{
    __GLdlistOp *dlop;
    struct __gllc_Color3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color3d)(gc, red, green, blue);
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

GLvoid APIENTRY __gllc_Color3dv(__GLcontext *gc, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color3dv)(gc, v);
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

GLvoid APIENTRY __gllc_Color3f(__GLcontext *gc, GLfloat red, GLfloat green, GLfloat blue)
{
    __GLdlistOp *dlop;
    struct __gllc_Color3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color3f)(gc, red, green, blue);
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

GLvoid APIENTRY __gllc_Color3fv(__GLcontext *gc, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color3fv)(gc, v);
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

GLvoid APIENTRY __gllc_Color3i(__GLcontext *gc, GLint red, GLint green, GLint blue)
{
    __GLdlistOp *dlop;
    struct __gllc_Color3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color3i)(gc, red, green, blue);
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

GLvoid APIENTRY __gllc_Color3iv(__GLcontext *gc, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color3iv)(gc, v);
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

GLvoid APIENTRY __gllc_Color3s(__GLcontext *gc, GLshort red, GLshort green, GLshort blue)
{
    __GLdlistOp *dlop;
    struct __gllc_Color3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color3s)(gc, red, green, blue);
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

GLvoid APIENTRY __gllc_Color3sv(__GLcontext *gc, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color3sv)(gc, v);
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

GLvoid APIENTRY __gllc_Color3ub(__GLcontext *gc, GLubyte red, GLubyte green, GLubyte blue)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4ubv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color3ub)(gc, red, green, blue);
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

GLvoid APIENTRY __gllc_Color3ubv(__GLcontext *gc, const GLubyte *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4ubv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color3ubv)(gc, v);
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

GLvoid APIENTRY __gllc_Color3ui(__GLcontext *gc, GLuint red, GLuint green, GLuint blue)
{
    __GLdlistOp *dlop;
    struct __gllc_Color3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color3ui)(gc, red, green, blue);
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

GLvoid APIENTRY __gllc_Color3uiv(__GLcontext *gc, const GLuint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color3uiv)(gc, v);
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

GLvoid APIENTRY __gllc_Color3us(__GLcontext *gc, GLushort red, GLushort green, GLushort blue)
{
    __GLdlistOp *dlop;
    struct __gllc_Color3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color3us)(gc, red, green, blue);
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

GLvoid APIENTRY __gllc_Color3usv(__GLcontext *gc, const GLushort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color3usv)(gc, v);
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

GLvoid APIENTRY __gllc_Color4b(__GLcontext *gc, GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4ubv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color4b)(gc, red, green, blue, alpha);
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

GLvoid APIENTRY __gllc_Color4bv(__GLcontext *gc, const GLbyte *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4ubv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color4bv)(gc, v);
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

GLvoid APIENTRY __gllc_Color4d(__GLcontext *gc, GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color4d)(gc, red, green, blue, alpha);
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

GLvoid APIENTRY __gllc_Color4dv(__GLcontext *gc, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color4dv)(gc, v);
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

GLvoid APIENTRY __gllc_Color4f(__GLcontext *gc, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color4f)(gc, red, green, blue, alpha);
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

GLvoid APIENTRY __gllc_Color4fv(__GLcontext *gc, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color4fv)(gc, v);
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

GLvoid APIENTRY __gllc_Color4i(__GLcontext *gc, GLint red, GLint green, GLint blue, GLint alpha)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color4i)(gc, red, green, blue, alpha);
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

GLvoid APIENTRY __gllc_Color4iv(__GLcontext *gc, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color4iv)(gc, v);
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

GLvoid APIENTRY __gllc_Color4s(__GLcontext *gc, GLshort red, GLshort green, GLshort blue, GLshort alpha)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color4s)(gc, red, green, blue, alpha);
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

GLvoid APIENTRY __gllc_Color4sv(__GLcontext *gc, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color4sv)(gc, v);
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

GLvoid APIENTRY __gllc_Color4ub(__GLcontext *gc, GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4ubv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color4ub)(gc, red, green, blue, alpha);
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

GLvoid APIENTRY __gllc_Color4ubv(__GLcontext *gc, const GLubyte *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4ubv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color4ubv)(gc, v);
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

GLvoid APIENTRY __gllc_Color4ui(__GLcontext *gc, GLuint red, GLuint green, GLuint blue, GLuint alpha)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color4ui)(gc, red, green, blue, alpha);
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

GLvoid APIENTRY __gllc_Color4uiv(__GLcontext *gc, const GLuint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color4uiv)(gc, v);
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

GLvoid APIENTRY __gllc_Color4us(__GLcontext *gc, GLushort red, GLushort green, GLushort blue, GLushort alpha)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color4us)(gc, red, green, blue, alpha);
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

GLvoid APIENTRY __gllc_Color4usv(__GLcontext *gc, const GLushort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Color4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Color4usv)(gc, v);
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

GLvoid APIENTRY __gllc_Normal3b(__GLcontext *gc, GLbyte nx, GLbyte ny, GLbyte nz)
{
    __GLdlistOp *dlop;
    struct __gllc_Normal3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Normal3b)(gc, nx, ny, nz);
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

GLvoid APIENTRY __gllc_Normal3bv(__GLcontext *gc, const GLbyte *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Normal3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Normal3bv)(gc, v);
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

GLvoid APIENTRY __gllc_Normal3d(__GLcontext *gc, GLdouble nx, GLdouble ny, GLdouble nz)
{
    __GLdlistOp *dlop;
    struct __gllc_Normal3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Normal3d)(gc, nx, ny, nz);
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

GLvoid APIENTRY __gllc_Normal3dv(__GLcontext *gc, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Normal3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Normal3dv)(gc, v);
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

GLvoid APIENTRY __gllc_Normal3f(__GLcontext *gc, GLfloat nx, GLfloat ny, GLfloat nz)
{
    __GLdlistOp *dlop;
    struct __gllc_Normal3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Normal3f)(gc, nx, ny, nz);
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

GLvoid APIENTRY __gllc_Normal3fv(__GLcontext *gc, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Normal3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Normal3fv)(gc, v);
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

GLvoid APIENTRY __gllc_Normal3i(__GLcontext *gc, GLint nx, GLint ny, GLint nz)
{
    __GLdlistOp *dlop;
    struct __gllc_Normal3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Normal3i)(gc, nx, ny, nz);
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

GLvoid APIENTRY __gllc_Normal3iv(__GLcontext *gc, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Normal3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Normal3iv)(gc, v);
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

GLvoid APIENTRY __gllc_Normal3s(__GLcontext *gc, GLshort nx, GLshort ny, GLshort nz)
{
    __GLdlistOp *dlop;
    struct __gllc_Normal3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Normal3s)(gc, nx, ny, nz);
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

GLvoid APIENTRY __gllc_Normal3sv(__GLcontext *gc, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Normal3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Normal3sv)(gc, v);
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

GLvoid APIENTRY __gllc_TexCoord1d(__GLcontext *gc, GLdouble s)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord1d)(gc, s);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = s;
    data->v[1] = 0.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord1dv(__GLcontext *gc, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord1dv)(gc, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = 0.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord1f(__GLcontext *gc, GLfloat s)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord1f)(gc, s);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = s;
    data->v[1] = 0.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord1fv(__GLcontext *gc, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord1fv)(gc, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = 0.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord1i(__GLcontext *gc, GLint s)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord1i)(gc, s);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = s;
    data->v[1] = 0.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord1iv(__GLcontext *gc, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord1iv)(gc, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = 0.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord1s(__GLcontext *gc, GLshort s)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord1s)(gc, s);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = s;
    data->v[1] = 0.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord1sv(__GLcontext *gc, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord1sv)(gc, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = 0.0;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord2d(__GLcontext *gc, GLdouble s, GLdouble t)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord2d)(gc, s, t);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = s;
    data->v[1] = t;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord2dv(__GLcontext *gc, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord2dv)(gc, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord2f(__GLcontext *gc, GLfloat s, GLfloat t)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord2f)(gc, s, t);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = s;
    data->v[1] = t;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord2fv(__GLcontext *gc, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord2fv)(gc, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord2i(__GLcontext *gc, GLint s, GLint t)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord2i)(gc, s, t);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = s;
    data->v[1] = t;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord2iv(__GLcontext *gc, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord2iv)(gc, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord2s(__GLcontext *gc, GLshort s, GLshort t)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord2s)(gc, s, t);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = s;
    data->v[1] = t;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord2sv(__GLcontext *gc, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord2sv)(gc, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_TexCoord2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_TexCoord2fv;
    data = (struct __gllc_TexCoord2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_TexCoord3d(__GLcontext *gc, GLdouble s, GLdouble t, GLdouble r)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord3d)(gc, s, t, r);
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

GLvoid APIENTRY __gllc_TexCoord3dv(__GLcontext *gc, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord3dv)(gc, v);
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

GLvoid APIENTRY __gllc_TexCoord3f(__GLcontext *gc, GLfloat s, GLfloat t, GLfloat r)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord3f)(gc, s, t, r);
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

GLvoid APIENTRY __gllc_TexCoord3fv(__GLcontext *gc, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord3fv)(gc, v);
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

GLvoid APIENTRY __gllc_TexCoord3i(__GLcontext *gc, GLint s, GLint t, GLint r)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord3i)(gc, s, t, r);
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

GLvoid APIENTRY __gllc_TexCoord3iv(__GLcontext *gc, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord3iv)(gc, v);
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

GLvoid APIENTRY __gllc_TexCoord3s(__GLcontext *gc, GLshort s, GLshort t, GLshort r)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord3s)(gc, s, t, r);
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

GLvoid APIENTRY __gllc_TexCoord3sv(__GLcontext *gc, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord3sv)(gc, v);
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

GLvoid APIENTRY __gllc_TexCoord4d(__GLcontext *gc, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord4d)(gc, s, t, r, q);
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

GLvoid APIENTRY __gllc_TexCoord4dv(__GLcontext *gc, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord4dv)(gc, v);
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

GLvoid APIENTRY __gllc_TexCoord4f(__GLcontext *gc, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord4f)(gc, s, t, r, q);
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

GLvoid APIENTRY __gllc_TexCoord4fv(__GLcontext *gc, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord4fv)(gc, v);
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

GLvoid APIENTRY __gllc_TexCoord4i(__GLcontext *gc, GLint s, GLint t, GLint r, GLint q)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord4i)(gc, s, t, r, q);
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

GLvoid APIENTRY __gllc_TexCoord4iv(__GLcontext *gc, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord4iv)(gc, v);
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

GLvoid APIENTRY __gllc_TexCoord4s(__GLcontext *gc, GLshort s, GLshort t, GLshort r, GLshort q)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord4s)(gc, s, t, r, q);
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

GLvoid APIENTRY __gllc_TexCoord4sv(__GLcontext *gc, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_TexCoord4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->TexCoord4sv)(gc, v);
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

GLvoid APIENTRY __gllc_Vertex2d(__GLcontext *gc, GLdouble x, GLdouble y)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Vertex2d)(gc, x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex2fv;
    data = (struct __gllc_Vertex2fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex2dv(__GLcontext *gc, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Vertex2dv)(gc, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex2fv;
    data = (struct __gllc_Vertex2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex2f(__GLcontext *gc, GLfloat x, GLfloat y)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Vertex2f)(gc, x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex2fv;
    data = (struct __gllc_Vertex2fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex2fv(__GLcontext *gc, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Vertex2fv)(gc, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex2fv;
    data = (struct __gllc_Vertex2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex2i(__GLcontext *gc, GLint x, GLint y)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Vertex2i)(gc, x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex2fv;
    data = (struct __gllc_Vertex2fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex2iv(__GLcontext *gc, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Vertex2iv)(gc, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex2fv;
    data = (struct __gllc_Vertex2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex2s(__GLcontext *gc, GLshort x, GLshort y)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Vertex2s)(gc, x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex2fv;
    data = (struct __gllc_Vertex2fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex2sv(__GLcontext *gc, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Vertex2sv)(gc, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Vertex2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Vertex2fv;
    data = (struct __gllc_Vertex2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Vertex3d(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Vertex3d)(gc, x, y, z);
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

GLvoid APIENTRY __gllc_Vertex3dv(__GLcontext *gc, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Vertex3dv)(gc, v);
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

GLvoid APIENTRY __gllc_Vertex3f(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Vertex3f)(gc, x, y, z);
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

GLvoid APIENTRY __gllc_Vertex3fv(__GLcontext *gc, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Vertex3fv)(gc, v);
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

GLvoid APIENTRY __gllc_Vertex3i(__GLcontext *gc, GLint x, GLint y, GLint z)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Vertex3i)(gc, x, y, z);
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

GLvoid APIENTRY __gllc_Vertex3iv(__GLcontext *gc, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Vertex3iv)(gc, v);
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

GLvoid APIENTRY __gllc_Vertex3s(__GLcontext *gc, GLshort x, GLshort y, GLshort z)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Vertex3s)(gc, x, y, z);
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

GLvoid APIENTRY __gllc_Vertex3sv(__GLcontext *gc, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Vertex3sv)(gc, v);
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

GLvoid APIENTRY __gllc_Vertex4d(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Vertex4d)(gc, x, y, z, w);
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

GLvoid APIENTRY __gllc_Vertex4dv(__GLcontext *gc, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Vertex4dv)(gc, v);
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

GLvoid APIENTRY __gllc_Vertex4f(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Vertex4f)(gc, x, y, z, w);
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

GLvoid APIENTRY __gllc_Vertex4fv(__GLcontext *gc, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Vertex4fv)(gc, v);
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

GLvoid APIENTRY __gllc_Vertex4i(__GLcontext *gc, GLint x, GLint y, GLint z, GLint w)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Vertex4i)(gc, x, y, z, w);
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

GLvoid APIENTRY __gllc_Vertex4iv(__GLcontext *gc, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Vertex4iv)(gc, v);
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

GLvoid APIENTRY __gllc_Vertex4s(__GLcontext *gc, GLshort x, GLshort y, GLshort z, GLshort w)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Vertex4s)(gc, x, y, z, w);
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

GLvoid APIENTRY __gllc_Vertex4sv(__GLcontext *gc, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Vertex4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->Vertex4sv)(gc, v);
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

GLvoid APIENTRY __gllc_MultiTexCoord1d(__GLcontext *gc, GLenum texture, GLdouble s)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord1d)(gc, texture, s);
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

GLvoid APIENTRY __gllc_MultiTexCoord1dv(__GLcontext *gc, GLenum texture, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord1dv)(gc, texture, v);
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

GLvoid APIENTRY __gllc_MultiTexCoord1f(__GLcontext *gc, GLenum texture, GLfloat s)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord1f)(gc, texture, s);
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

GLvoid APIENTRY __gllc_MultiTexCoord1fv(__GLcontext *gc, GLenum texture, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord1fv)(gc, texture, v);
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

GLvoid APIENTRY __gllc_MultiTexCoord1i(__GLcontext *gc, GLenum texture, GLint s)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord1i)(gc, texture, s);
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

GLvoid APIENTRY __gllc_MultiTexCoord1iv(__GLcontext *gc, GLenum texture, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord1iv)(gc, texture, v);
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

GLvoid APIENTRY __gllc_MultiTexCoord1s(__GLcontext *gc, GLenum texture, GLshort s)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord1s)(gc, texture, s);
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

GLvoid APIENTRY __gllc_MultiTexCoord1sv(__GLcontext *gc, GLenum texture, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord1sv)(gc, texture, v);
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

GLvoid APIENTRY __gllc_MultiTexCoord2d(__GLcontext *gc, GLenum texture, GLdouble s, GLdouble t)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord2d)(gc, texture, s, t);
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

GLvoid APIENTRY __gllc_MultiTexCoord2dv(__GLcontext *gc, GLenum texture, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord2dv)(gc, texture, v);
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

GLvoid APIENTRY __gllc_MultiTexCoord2f(__GLcontext *gc, GLenum texture, GLfloat s, GLfloat t)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord2f)(gc, texture, s, t);
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

GLvoid APIENTRY __gllc_MultiTexCoord2fv(__GLcontext *gc, GLenum texture, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord2fv)(gc, texture, v);
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

GLvoid APIENTRY __gllc_MultiTexCoord2i(__GLcontext *gc, GLenum texture, GLint s, GLint t)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord2i)(gc, texture, s, t);
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

GLvoid APIENTRY __gllc_MultiTexCoord2iv(__GLcontext *gc, GLenum texture, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord2iv)(gc, texture, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultiTexCoord2fv_Rec));
    dlop->opcode  = __glop_MultiTexCoord2fv;
    data = (struct __gllc_MultiTexCoord2fv_Rec *)(dlop + 1);
    data->texture = texture;
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultiTexCoord2s(__GLcontext *gc, GLenum texture, GLshort s, GLshort t)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord2s)(gc, texture, s, t);
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

GLvoid APIENTRY __gllc_MultiTexCoord2sv(__GLcontext *gc, GLenum texture, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord2sv)(gc, texture, v);
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

GLvoid APIENTRY __gllc_MultiTexCoord3d(__GLcontext *gc, GLenum texture, GLdouble s, GLdouble t, GLdouble r)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord3d)(gc, texture, s, t, r);
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

GLvoid APIENTRY __gllc_MultiTexCoord3dv(__GLcontext *gc, GLenum texture, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord3dv)(gc, texture, v);
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

GLvoid APIENTRY __gllc_MultiTexCoord3f(__GLcontext *gc, GLenum texture, GLfloat s, GLfloat t, GLfloat r)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord3f)(gc, texture, s, t, r);
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

GLvoid APIENTRY __gllc_MultiTexCoord3fv(__GLcontext *gc, GLenum texture, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord3fv)(gc, texture, v);
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

GLvoid APIENTRY __gllc_MultiTexCoord3i(__GLcontext *gc, GLenum texture, GLint s, GLint t, GLint r)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord3i)(gc, texture, s, t, r);
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

GLvoid APIENTRY __gllc_MultiTexCoord3iv(__GLcontext *gc, GLenum texture, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord3iv)(gc, texture, v);
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

GLvoid APIENTRY __gllc_MultiTexCoord3s(__GLcontext *gc, GLenum texture, GLshort s, GLshort t, GLshort r)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord3s)(gc, texture, s, t, r);
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

GLvoid APIENTRY __gllc_MultiTexCoord3sv(__GLcontext *gc, GLenum texture, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord3sv)(gc, texture, v);
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

GLvoid APIENTRY __gllc_MultiTexCoord4d(__GLcontext *gc, GLenum texture, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord4d)(gc, texture, s, t, r, q);
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

GLvoid APIENTRY __gllc_MultiTexCoord4dv(__GLcontext *gc, GLenum texture, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord4dv)(gc, texture, v);
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

GLvoid APIENTRY __gllc_MultiTexCoord4f(__GLcontext *gc, GLenum texture, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord4f)(gc, texture, s, t, r, q);
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

GLvoid APIENTRY __gllc_MultiTexCoord4fv(__GLcontext *gc, GLenum texture, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord4fv)(gc, texture, v);
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

GLvoid APIENTRY __gllc_MultiTexCoord4i(__GLcontext *gc, GLenum texture, GLint s, GLint t, GLint r, GLint q)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord4i)(gc, texture, s, t, r, q);
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

GLvoid APIENTRY __gllc_MultiTexCoord4iv(__GLcontext *gc, GLenum texture, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord4iv)(gc, texture, v);
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

GLvoid APIENTRY __gllc_MultiTexCoord4s(__GLcontext *gc, GLenum texture, GLshort s, GLshort t, GLshort r, GLshort q)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord4s)(gc, texture, s, t, r, q);
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

GLvoid APIENTRY __gllc_MultiTexCoord4sv(__GLcontext *gc, GLenum texture, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MultiTexCoord4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->MultiTexCoord4sv)(gc, texture, v);
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

GLvoid APIENTRY __gllc_FogCoordf(__GLcontext *gc, GLfloat coord)
{
    __GLdlistOp *dlop;
    struct __gllc_FogCoordf_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->FogCoordf)(gc, coord);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_FogCoordf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_FogCoordf;
    data = (struct __gllc_FogCoordf_Rec *)(dlop + 1);
    data->coord = coord;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_FogCoordd(__GLcontext *gc, GLdouble coord)
{
    __GLdlistOp *dlop;
    struct __gllc_FogCoordf_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->FogCoordd)(gc, coord);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_FogCoordf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_FogCoordf;
    data = (struct __gllc_FogCoordf_Rec *)(dlop + 1);
    data->coord = coord;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_FogCoordfv(__GLcontext *gc, const GLfloat coord[])
{
    __GLdlistOp *dlop;
    struct __gllc_FogCoordf_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->FogCoordfv)(gc, coord);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_FogCoordf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_FogCoordf;
    data = (struct __gllc_FogCoordf_Rec *)(dlop + 1);
    data->coord = coord[0];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_FogCoorddv(__GLcontext *gc, const GLdouble coord[])
{
    __GLdlistOp *dlop;
    struct __gllc_FogCoordf_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->FogCoorddv)(gc, coord);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_FogCoordf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_FogCoordf;
    data = (struct __gllc_FogCoordf_Rec *)(dlop + 1);
    data->coord = coord[0];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_VertexAttrib1s(__GLcontext *gc, GLuint index, GLshort x)
{
/* still not added, to do*/
GL_ASSERT(0);
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib1s)(gc, index, x);
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
*/
}

GLvoid APIENTRY __gllc_VertexAttrib1f(__GLcontext *gc, GLuint index, GLfloat x)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib1f)(gc, index, x);
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

GLvoid APIENTRY __gllc_VertexAttrib1d(__GLcontext *gc, GLuint index, GLdouble x)
{
/* still not added, to do*/
GL_ASSERT(0);
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib1d)(gc, index, x);
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
*/
}

GLvoid APIENTRY __gllc_VertexAttrib2s(__GLcontext *gc, GLuint index, GLshort x, GLshort y)
{
/* still not added, to do*/
GL_ASSERT(0);
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib2s)(gc, index, x, y);
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
*/
}

GLvoid APIENTRY __gllc_VertexAttrib2f(__GLcontext *gc, GLuint index, GLfloat x, GLfloat y)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib2f)(gc, index, x, y);
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

GLvoid APIENTRY __gllc_VertexAttrib2d(__GLcontext *gc, GLuint index, GLdouble x, GLdouble y)
{
/* still not added, to do*/
GL_ASSERT(0);
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib2d)(gc, index, x, y);
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
*/
}

GLvoid APIENTRY __gllc_VertexAttrib3s(__GLcontext *gc, GLuint index, GLshort x, GLshort y, GLshort z)
{
/* still not added, to do*/
GL_ASSERT(0);
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib3s)(gc, index, x, y, z);
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
*/
}

GLvoid APIENTRY __gllc_VertexAttrib3f(__GLcontext *gc, GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib3f)(gc, index, x, y, z);
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

GLvoid APIENTRY __gllc_VertexAttrib3d(__GLcontext *gc, GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
/* still not added, to do*/
GL_ASSERT(0);
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib3d)(gc, index, x, y, z);
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
*/
}

GLvoid APIENTRY __gllc_VertexAttrib4s(__GLcontext *gc, GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
/* still not added, to do*/
GL_ASSERT(0);
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib4s)(gc, index, x, y, z, w);
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
*/
}

GLvoid APIENTRY __gllc_VertexAttrib4f(__GLcontext *gc, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib4f)(gc, index, x, y, z, w);
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

GLvoid APIENTRY __gllc_VertexAttrib4d(__GLcontext *gc, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
/* still not added, to do*/
GL_ASSERT(0);
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib4d)(gc, index, x, y, z, w);
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
*/
}

GLvoid APIENTRY __gllc_VertexAttrib4Nub(__GLcontext *gc, GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
/* still not added, to do*/
GL_ASSERT(0);
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib4Nub)(gc, index, x, y, z, w);
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
*/
}

GLvoid APIENTRY __gllc_VertexAttrib1sv(__GLcontext *gc, GLuint index, const GLshort *v)
{
/* still not added, to do*/
GL_ASSERT(0);
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib1sv)(gc, index, v);
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
*/
}

GLvoid APIENTRY __gllc_VertexAttrib1fv(__GLcontext *gc, GLuint index, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib1fv)(gc, index, v);
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

GLvoid APIENTRY __gllc_VertexAttrib1dv(__GLcontext *gc, GLuint index, const GLdouble *v)
{
/* still not added, to do*/
GL_ASSERT(0);
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib1dv)(gc, index, v);
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
*/
}

GLvoid APIENTRY __gllc_VertexAttrib2sv(__GLcontext *gc, GLuint index, const GLshort *v)
{
/* still not added, to do*/
GL_ASSERT(0);
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib2sv)(gc, index, v);
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
*/
}

GLvoid APIENTRY __gllc_VertexAttrib2fv(__GLcontext *gc, GLuint index, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib2fv)(gc, index, v);
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

GLvoid APIENTRY __gllc_VertexAttrib2dv(__GLcontext *gc, GLuint index, const GLdouble *v)
{
/* still not added, to do*/
GL_ASSERT(0);
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib2dv)(gc, index, v);
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
*/
}

GLvoid APIENTRY __gllc_VertexAttrib3sv(__GLcontext *gc, GLuint index, const GLshort *v)
{
/* still not added, to do*/
GL_ASSERT(0);
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib3sv)(gc, index, v);
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
*/
}

GLvoid APIENTRY __gllc_VertexAttrib3fv(__GLcontext *gc, GLuint index, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib3fv)(gc, index, v);
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

GLvoid APIENTRY __gllc_VertexAttrib3dv(__GLcontext *gc, GLuint index, const GLdouble *v)
{
/* still not added, to do*/
GL_ASSERT(0);
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib3dv)(gc, index, v);
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
*/
}

GLvoid APIENTRY __gllc_VertexAttrib4bv(__GLcontext *gc, GLuint index, const GLbyte *v)
{
/* still not added, to do*/
GL_ASSERT(0);
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib4bv)(gc, index, v);
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
*/
}

GLvoid APIENTRY __gllc_VertexAttrib4sv(__GLcontext *gc, GLuint index, const GLshort *v)
{
/* still not added, to do*/
GL_ASSERT(0);
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib4sv)(gc, index, v);
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
*/
}

GLvoid APIENTRY __gllc_VertexAttrib4iv(__GLcontext *gc, GLuint index, const GLint *v)
{
/* still not added, to do*/
GL_ASSERT(0);
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib4iv)(gc, index, v);
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
*/
}

GLvoid APIENTRY __gllc_VertexAttrib4ubv(__GLcontext *gc, GLuint index, const GLubyte *v)
{
/* still not added, to do*/
GL_ASSERT(0);
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib4ubv)(gc, index, v);
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
*/
}

GLvoid APIENTRY __gllc_VertexAttrib4usv(__GLcontext *gc, GLuint index, const GLushort *v)
{
/* still not added, to do*/
GL_ASSERT(0);
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib4usv)(gc, index, v);
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
*/
}

GLvoid APIENTRY __gllc_VertexAttrib4uiv(__GLcontext *gc, GLuint index, const GLuint *v)
{
/* still not added, to do*/
GL_ASSERT(0);
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib4uiv)(gc, index, v);
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
*/
}

GLvoid APIENTRY __gllc_VertexAttrib4fv(__GLcontext *gc, GLuint index, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib4fv)(gc, index, v);
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

GLvoid APIENTRY __gllc_VertexAttrib4dv(__GLcontext *gc, GLuint index, const GLdouble *v)
{
/* still not added, to do*/
GL_ASSERT(0);
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib4dv)(gc, index, v);
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
*/
}

GLvoid APIENTRY __gllc_VertexAttrib4Nbv(__GLcontext *gc, GLuint index, const GLbyte *v)
{
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib4Nbv)(gc, index, v);
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
*/
}

GLvoid APIENTRY __gllc_VertexAttrib4Nsv(__GLcontext *gc, GLuint index, const GLshort *v)
{
/* still not added, to do*/
GL_ASSERT(0);
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib4Nsv)(gc, index, v);
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
*/
}

GLvoid APIENTRY __gllc_VertexAttrib4Niv(__GLcontext *gc, GLuint index, const GLint *v)
{
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib4Niv)(gc, index, v);
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
*/
}

GLvoid APIENTRY __gllc_VertexAttrib4Nubv(__GLcontext *gc, GLuint index, const GLubyte *v)
{
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib4Nubv)(gc, index, v);
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
*/
}

GLvoid APIENTRY __gllc_VertexAttrib4Nusv(__GLcontext *gc, GLuint index, const GLushort *v)
{
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib4Nusv)(gc, index, v);
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
*/
}

GLvoid APIENTRY __gllc_VertexAttrib4Nuiv(__GLcontext *gc, GLuint index, const GLuint *v)
{
/*
    __GLdlistOp *dlop;
    struct __gllc_VertexAttrib4fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->VertexAttrib4Nuiv)(gc, index, v);
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
*/
}

GLvoid APIENTRY __gllc_Bitmap(__GLcontext *gc, GLsizei width, GLsizei height,
                            GLfloat xorig, GLfloat yorig,
                            GLfloat xmove, GLfloat ymove,
                            const GLubyte *oldbits)
{
    __GLdlistOp *dlop;
    struct __gllc_Bitmap_Rec *data;
    GLubyte *newbits;
    GLint imageSize;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Bitmap(gc, width, height, xorig, yorig, xmove, ymove, oldbits);
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

GLvoid APIENTRY __gllc_PolygonStipple(__GLcontext *gc, const GLubyte *mask)
{
    __GLdlistOp *dlop;

    GLubyte *newbits;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PolygonStipple(gc, mask);
    }

    dlop = __glDlistAllocOp(gc, __glImageSize(32, 32, GL_COLOR_INDEX, GL_BITMAP));
    if (dlop == NULL) return;
    dlop->opcode = __glop_PolygonStipple;
    newbits = (GLubyte *)(dlop + 1);
    __glFillImage(gc, 32, 32, GL_COLOR_INDEX, GL_BITMAP, mask, newbits);
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Map1f(__GLcontext *gc, GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points)
{
    __GLdlistOp *dlop;
    struct __gllc_Map1f_Rec *map1data;
    GLint k;
    GLint cmdsize;
    GLfloat *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Map1f(gc, target, u1, u2, stride, order, points);
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

GLvoid APIENTRY __gllc_Map1d(__GLcontext *gc, GLenum target,
                           GLdouble u1, GLdouble u2,
                           GLint stride, GLint order,
                           const GLdouble *points)
{
    __GLdlistOp *dlop;
    struct __gllc_Map1f_Rec *map1data;
    GLint k;
    GLint cmdsize;
    GLfloat *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Map1d(gc, target, u1, u2, stride, order, points);
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

GLvoid APIENTRY __gllc_Map2f(__GLcontext *gc, GLenum target,
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


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Map2f(gc, target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
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


GLvoid APIENTRY __gllc_Map2d(__GLcontext *gc, GLenum target,
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


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Map2d(gc, target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
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

GLvoid APIENTRY __gllc_SampleCoverage(__GLcontext *gc, GLclampf v, GLboolean invert)
{
    __GLdlistOp *dlop;
    struct __gllc_SampleCoverage_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_SampleCoverage(gc, v, invert);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_SampleCoverage_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_SampleCoverage;
    data = (struct __gllc_SampleCoverage_Rec *)(dlop + 1);
    data->v = v;
    data->invert = invert;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_DrawPixels(__GLcontext *gc, GLint width, GLint height, GLenum format,
                                GLenum type, const GLvoid *pixels)
{
    GLenum adjustFormat= format;
    GLenum adjustType= type;
    __GLdlistOp *dlop;
    struct __gllc_DrawPixels_Rec *data;
    GLint imageSize;
    GLboolean index;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_DrawPixels(gc, width, height, format, type, pixels);
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
            __glSetError(gc, GL_INVALID_OPERATION);
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
            __glSetError(gc, GL_INVALID_OPERATION);
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

GLvoid APIENTRY __gllc_TexImage1D(__GLcontext *gc, GLenum target, GLint level,
                                GLint components,
                                GLint width, GLint border, GLenum format,
                                GLenum type, const GLvoid *pixels)
{
    GLenum adjustFormat= format;
    GLenum adjustType= type;
    __GLdlistOp *dlop;
    struct __gllc_TexImage1D_Rec *texdata;
    GLint imageSize;
    GLuint activeUnit;
    __GLtextureObject *tex;

    __GL_TEXSUBIMAGE1D_GET_OBJECT();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexImage1D(gc, target, level, components, width, border, format, type, pixels);
    }
    else
    {
        GLenum oldError = gc->error;
        /*proxy textures are executed immediately*/
        switch (target)
        {
            case GL_PROXY_TEXTURE_1D:
                __glim_TexImage1D(gc, target, level, components, width, border, format, type, pixels);
                return;
            case GL_TEXTURE_1D:
                break;
            default:
                __gllc_InvalidEnum(gc);
                return;
        }
        /*do argument check*/
        /* Check arguments */
        if (!__glCheckTexImgArgs(gc, tex, level, width, 1 + border*2, 1 + border*2, border))
        {
            __glSetError(gc, oldError);
        }

        if (!__glCheckTexImgTypeArg(gc, tex, type))
        {
            __glSetError(gc, oldError);
        }

        if (!__glCheckTexImgFmtArg(gc, tex, format))
        {
            __glSetError(gc, oldError);
        }

        if (!__glCheckTexImgInternalFmtArg(gc, tex, components))
        {
            __glSetError(gc, oldError);
        }

        if (!__glCheckTexImgFmt(gc, tex, target, components, format, type))
        {
            __glSetError(gc, oldError);
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

GLvoid APIENTRY __gllc_TexImage2D(__GLcontext *gc, GLenum target, GLint lod,
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
    GLuint activeUnit;
    __GLtextureObject *tex;
    GLuint face = 0;

    __GL_TEXIMAGE2D_GET_OBJECT();
    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexImage2D(gc, target, lod, internalFormat, width, height, border, format, type, pixels);
    }
    else
    {
        GLenum oldError = gc->error;
        /*proxy textures are executed immediately*/
        switch (target)
        {
            case GL_PROXY_TEXTURE_2D:
            case GL_PROXY_TEXTURE_CUBE_MAP:
                __glim_TexImage2D(gc, target,lod, internalFormat, width, height, border, format, type, pixels);
                return;
            case GL_TEXTURE_2D:
                break;
            case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
                if(!__glExtension[__GL_EXTID_OES_texture_cube_map_array].bEnabled && !__glExtension[__GL_EXTID_EXT_texture_cube_map_array].bEnabled)
                {
                    __gllc_InvalidEnum(gc);
                    return;
                }
                break;
            default:
                __gllc_InvalidEnum(gc);
                return;
        }

        /* Check arguments */
        if (!__glCheckTexImgArgs(gc, tex, lod, width, height, 1, border))
        {
        __glSetError(gc, oldError);
        }

        if (!__glCheckTexImgTypeArg(gc, tex, type))
        {
        __glSetError(gc, oldError);
        }

        if (!__glCheckTexImgFmtArg(gc, tex, format))
        {
        __glSetError(gc, oldError);
        }

        if (!__glCheckTexImgInternalFmtArg(gc, tex, internalFormat))
        {
        __glSetError(gc, oldError);
        }

        if (!__glCheckTexImgFmt(gc, tex, target, internalFormat, format, type))
        {
        __glSetError(gc, oldError);
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

GLvoid APIENTRY __gllc_TexImage3D(__GLcontext *gc, GLenum target,
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
    GLuint activeUnit;
    __GLtextureObject *tex;

    adjust_format = format;
    adjust_type = type;

    __GL_TEXIMAGE3D_GET_OBJECT();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexImage3D(gc, target, lod, components, width, height, depth, border, format, type, buf);
    }
    else
    {
        GLenum oldError = gc->error;
        /*proxy textures are executed immediately*/
        switch (target)
        {
            case GL_PROXY_TEXTURE_3D:
                __glim_TexImage3D(gc, target, lod, components, width, height, depth, border, format, type, buf);
                return;
            case GL_TEXTURE_3D:
                break;
            default:
                __gllc_InvalidEnum(gc);
                return;
        }

        /* Check arguments */
        if (!__glCheckTexImgArgs(gc, tex, lod, width, height, depth, border))
        {
            __glSetError(gc, oldError);
        }

        if (!__glCheckTexImgTypeArg(gc, tex, type))
        {
            __glSetError(gc, oldError);
        }

        if (!__glCheckTexImgFmtArg(gc, tex, format))
        {
            __glSetError(gc, oldError);
        }

        if (!__glCheckTexImgInternalFmtArg(gc, tex, components))
        {
            __glSetError(gc, oldError);
        }

        if (!__glCheckTexImgFmt(gc, tex, target, components, format, type))
        {
            __glSetError(gc, oldError);
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

GLvoid APIENTRY __gllc_CopyTexImage1D(__GLcontext *gc, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
    __GLdlistOp *dlop;
    struct __gllc_CopyTexImage1D_Rec *data;
    GLenum format;
    GLuint activeUnit;
    __GLtextureObject *tex;

        /* Get the texture object and face */
        __GL_TEXIMAGE1D_GET_OBJECT();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_CopyTexImage1D(gc, target, level, internalformat, x, y, width, border);
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

        /* Check arguments */
        __glCheckTexCopyImgFmt(gc, tex, internalformat, GL_TRUE);
        __glCheckTexImgArgs(gc, tex, level, width, 1, 1, border);
        __glSetError(gc, oldError);
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

GLvoid APIENTRY __gllc_CopyTexImage2D(__GLcontext *gc, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    __GLdlistOp *dlop;
    struct __gllc_CopyTexImage2D_Rec *data;
    GLenum format;
    GLuint activeUnit;
    __GLtextureObject *tex;
    GLuint face = 0;
    /* Get the texture object and face */
    __GL_TEXIMAGE2D_GET_OBJECT();
    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_CopyTexImage2D(gc, target, level, internalformat, x, y, width, height, border);
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
                if(!__glExtension[__GL_EXTID_OES_texture_cube_map_array].bEnabled && !__glExtension[__GL_EXTID_EXT_texture_cube_map_array].bEnabled)
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

        __glCheckTexCopyImgFmt(gc, tex, internalformat, GL_TRUE);

        __glCheckTexImgArgs(gc, tex, level, width, height, 1, border);

        /*in display list compile mode, no gl error is set*/
        __glSetError(gc, oldError);

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

GLvoid APIENTRY __gllc_TexSubImage1D(__GLcontext *gc, GLenum target, GLint level,
                                   GLint xoffset, GLsizei width,
                                   GLenum format, GLenum type, const GLvoid *pixels)
{
    GLenum adjustFormat= format;
    GLenum adjustType= type;
    __GLdlistOp *dlop;
    struct __gllc_TexSubImage1D_Rec *texdata;
    GLint imageSize;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexSubImage1D(gc, target, level, xoffset, width, format, type, pixels);
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

        /* Check arguments */
        __glCheckTexSubImgArgs(gc, tex, 0, level, xoffset, 0, 0, width, 1, 1);

        __glCheckTexImgTypeArg(gc, tex, type);

        __glCheckTexImgFmtArg(gc, tex, format);

        __glCheckTexImgFmt(gc, tex, target, tex->faceMipmap[0][level].requestedFormat, format, type);

        __glSetError(gc, oldError);
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

GLvoid APIENTRY __gllc_TexSubImage2D(__GLcontext *gc, GLenum target, GLint level,
                                   GLint xoffset, GLint yoffset,
                                   GLsizei width, GLsizei height,
                                   GLenum format, GLenum type, const GLvoid *pixels)
{
    GLenum adjustFormat= format;
    GLenum adjustType= type;
    __GLdlistOp *dlop;
    struct __gllc_TexSubImage2D_Rec *texdata;
    GLint imageSize;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexSubImage2D(gc, target, level, xoffset, yoffset, width, height, format, type, pixels);
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
                if(!__glExtension[__GL_EXTID_OES_texture_cube_map_array].bEnabled && !__glExtension[__GL_EXTID_EXT_texture_cube_map_array].bEnabled)
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

        /* Check arguments */
        __glCheckTexSubImgArgs(gc, tex, 0, level, xoffset, yoffset, 0, width, height, 1);

        __glCheckTexImgTypeArg(gc, tex, type);

        __glCheckTexImgFmtArg(gc, tex, format);

        __glCheckTexImgFmt(gc, tex, target, tex->faceMipmap[0][level].requestedFormat, format, type);

        __glSetError(gc, oldError);
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

GLvoid APIENTRY __gllc_TexSubImage3D(__GLcontext *gc, GLenum target,
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

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_TexSubImage3D(gc, target, lod, xoffset, yoffset, zoffset, width, height, depth, format, type, buf);
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

        /* Check arguments */
        __glCheckTexSubImgArgs(gc, tex, 0, lod, xoffset, yoffset, zoffset, width, height, depth);

        __glCheckTexImgTypeArg(gc, tex, type);

        __glCheckTexImgFmtArg(gc, tex, format);

        __glCheckTexImgFmt(gc, tex, target, tex->faceMipmap[0][lod].requestedFormat, format, type);

        __glSetError(gc, oldError);
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

GLvoid APIENTRY __gllc_CopyTexSubImage1D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
    __GLdlistOp *dlop;
    struct __gllc_CopyTexSubImage1D_Rec *data;
    GLint max_lod;

    max_lod = (GLint)(gc->constants.maxNumTextureLevels - 1);

    /* check lod */
    /*
    ** because switch (mipmap->requestedFormat) use the level and change the format
    ** so we do check here outside the __glCheckTexSubImageArgs
    */
    if (level < 0 || level > max_lod )
    {
        __glSetError(gc, GL_INVALID_VALUE);
        return;
    }
    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_CopyTexSubImage1D(gc, target, level, xoffset, x, y, width);
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

        /* Check arguments */
        __glCheckTexSubImgArgs(gc, tex, 0, level, xoffset, 0, 0, width, 1, 1);

        __glCheckTexCopyImgFmt(gc, tex, tex->faceMipmap[0][level].requestedFormat, GL_FALSE);

        __glSetError(gc, oldError);
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

GLvoid APIENTRY __gllc_CopyTexSubImage2D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GLdlistOp *dlop;
    struct __gllc_CopyTexSubImage2D_Rec *data;
    GLint max_lod;

    max_lod = (GLint)(gc->constants.maxNumTextureLevels - 1);

    /* check lod */
    /*
    ** because switch (mipmap->requestedFormat) use the level and change the format
    ** so we do check here outside the __glCheckTexSubImageArgs
    */
    if (level < 0 || level > max_lod )
    {
        __glSetError(gc, GL_INVALID_VALUE);
        return;
    }

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_CopyTexSubImage2D(gc, target, level, xoffset, yoffset, x, y, width, height);
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
                if(!__glExtension[__GL_EXTID_OES_texture_cube_map_array].bEnabled && !__glExtension[__GL_EXTID_EXT_texture_cube_map_array].bEnabled)
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

        /* Check arguments */
        __glCheckTexSubImgArgs(gc, tex, face, level, xoffset, yoffset, 0, width, height, 1);

        __glCheckTexCopyImgFmt(gc, tex, tex->faceMipmap[face][level].requestedFormat, GL_FALSE);

        __glSetError(gc, oldError);
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

GLvoid APIENTRY __gllc_CopyTexSubImage3D(__GLcontext *gc, GLenum target,
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



    max_lod = (GLint)(gc->constants.maxNumTextureLevels - 1);

    /* check lod */
    /*
    ** because switch (mipmap->requestedFormat) use the level and change the format
    ** so we do check here outside the __glCheckTexSubImageArgs
    */
    if (level < 0 || level > max_lod )
    {
        __glSetError(gc, GL_INVALID_VALUE);
        return;
    }

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_CopyTexSubImage3D(gc, target, level, xoffset, yoffset, zoffset, x, y, width, height);
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

        /* Check arguments */
        __glCheckTexSubImgArgs(gc, tex, 0, level, xoffset, yoffset, zoffset, width, height, 1);

        __glCheckTexCopyImgFmt(gc, tex, tex->faceMipmap[0][level].requestedFormat, GL_FALSE);

        __glSetError(gc, oldError);

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

GLvoid APIENTRY __gllc_CompressedTexImage3D(__GLcontext *gc, GLenum target, GLint level,
                                          GLenum internalformat, GLsizei width,
                                          GLsizei height, GLsizei depth,
                                          GLint border, GLsizei imageSize,
                                          const GLvoid *data)
{

    // this implementation does not support 3D textures. (yet)
    __gllc_InvalidOperation(gc);
    return;
}

GLvoid APIENTRY __gllc_CompressedTexImage2D(__GLcontext *gc, GLenum target, GLint lod,
                                          GLenum components, GLsizei width,
                                          GLsizei height, GLint border,
                                          GLsizei imageSize, const GLvoid *data)
{
    GLint iExpectedSize = 0;
    __GLdlistOp *dlop;
    struct __gllc_CompressedTexImage2D_Rec *texdata;
    GLint blockSize;
    GLuint activeUnit;
    __GLtextureObject *tex;
    GLuint face = 0;
    /* Get the texture object and face */
    __GL_TEXIMAGE2D_GET_OBJECT();

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_CompressedTexImage2D(gc, target, lod, components, width, height, border, imageSize, data);
    }
    else
    {
         GLenum oldError = gc->error;
        /*proxy textures are executed immediately*/
        switch (target)
        {
            case GL_PROXY_TEXTURE_2D:
            case GL_PROXY_TEXTURE_CUBE_MAP:
                __glim_CompressedTexImage2D(gc, target, lod,
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
                if(!__glExtension[__GL_EXTID_OES_texture_cube_map_array].bEnabled && !__glExtension[__GL_EXTID_EXT_texture_cube_map_array].bEnabled)
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

        /* Check arguments */
        if (!__glCheckTexImgArgs(gc, tex, lod, width, height, 1, border))
        {
            __glSetError(gc, oldError);
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

GLvoid APIENTRY __gllc_CompressedTexImage1D(__GLcontext *gc, GLenum target, GLint level,
                                          GLenum internalformat, GLsizei width,
                                          GLint border, GLsizei imageSize,
                                          const GLvoid *data)
{


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
//        __glim_CompressedTexImage1D(target, level, components, width, height, border, imageSize, data);
    }

    // this implementation does not support compressed 1D textures.
    __gllc_InvalidOperation(gc);
    return;
}

GLvoid APIENTRY __gllc_CompressedTexSubImage3D(__GLcontext *gc, GLenum target, GLint level,
                                             GLint xoffset, GLint yoffset,
                                             GLint zoffset, GLsizei width,
                                             GLsizei height, GLsizei depth,
                                             GLenum format, GLsizei imageSize,
                                             const GLvoid *data)
{


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
//        __glim_CompressedTexImage3D(target, lod, components, width, height, border, imageSize, data);
    }

    // this implementation does not support 3D textures. (yet)
    __gllc_InvalidOperation(gc);
    return;
}

GLvoid APIENTRY __gllc_CompressedTexSubImage2D(__GLcontext *gc, GLenum target, GLint lod,
                                             GLint xoffset, GLint yoffset,
                                             GLsizei width, GLsizei height,
                                             GLenum format, GLsizei imageSize,
                                             const GLvoid *data)
{
    GLint        iExpectedSize = 0, blockSize;
    __GLdlistOp *dlop;
    struct __gllc_CompressedTexSubImage2D_Rec *texdata;
    GLuint face;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_CompressedTexSubImage2D(gc, target, lod, xoffset, yoffset, width, height, format, imageSize, data);
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
                if(!__glExtension[__GL_EXTID_OES_texture_cube_map_array].bEnabled && !__glExtension[__GL_EXTID_EXT_texture_cube_map_array].bEnabled)
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

        __glCheckTexSubImgArgs(gc, tex, face, lod, xoffset, yoffset, 0, width, height, 1);

        __glSetError(gc, oldError);
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

GLvoid APIENTRY __gllc_CompressedTexSubImage1D(__GLcontext *gc, GLenum target, GLint level,
                                             GLint xoffset, GLsizei width,
                                             GLenum format, GLsizei imageSize,
                                             const GLvoid *data)
{


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
//        __glim_CompressedTexImage2D(target, lod, components, width, height, border, imageSize, data);
    }

    // this implementation does not support compressed 1D textures.
    __gllc_InvalidOperation(gc);

    return;
}

GLvoid APIENTRY __gllc_Disable(__GLcontext *gc, GLenum cap)
{
    __GLdlistOp *dlop;
    struct __gllc_Disable_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Disable(gc, cap);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Disable_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Disable;
    data = (struct __gllc_Disable_Rec *)(dlop + 1);
    data->cap = cap;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Enable(__GLcontext *gc, GLenum cap)
{
    __GLdlistOp *dlop;
    struct __gllc_Enable_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Enable(gc, cap);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Enable_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Enable;
    data = (struct __gllc_Enable_Rec *)(dlop + 1);
    data->cap = cap;
    __glDlistAppendOp(gc, dlop);
}
GLvoid APIENTRY __gllc_ColorTable(__GLcontext *gc, GLenum target, GLenum internalformat,
                GLsizei width, GLenum format, GLenum type, const GLvoid *table)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_ColorSubTable(__GLcontext *gc, GLenum target, GLsizei start,
            GLsizei count, GLenum format, GLenum type, const GLvoid *table)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_CopyColorTable(__GLcontext *gc, GLenum target, GLenum internalFormat, GLint x, GLint y, GLsizei width)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_CopyColorSubTable(__GLcontext *gc, GLenum target, GLsizei start, GLint x, GLint y, GLsizei width)
{
    __GLdlistOp *dlop;
    struct __gllc_CopyColorSubTable_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_CopyColorSubTable(gc, target, start, x, y, width);
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

GLvoid APIENTRY __gllc_ConvolutionFilter1D(__GLcontext *gc, GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image)
{
    GLenum adjustFormat= format;
    GLenum adjustType= type;
    __GLdlistOp *dlop;
    struct __gllc_ConvolutionFilter1D_Rec *data;
    GLint imageSize;
    GLboolean index;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ConvolutionFilter1D(gc, target, internalformat, width, format, type, image);
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
            __glSetError(gc, GL_INVALID_OPERATION);
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
            __glSetError(gc, GL_INVALID_OPERATION);
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

GLvoid APIENTRY __gllc_ConvolutionFilter2D(__GLcontext *gc, GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image)
{
    GLenum adjustFormat= format;
    GLenum adjustType= type;
    __GLdlistOp *dlop;
    struct __gllc_ConvolutionFilter2D_Rec  *data;
    GLint imageSize;
    GLboolean index;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ConvolutionFilter2D(gc, target, internalformat, width, height, format, type, image);
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
            __glSetError(gc, GL_INVALID_OPERATION);
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
            __glSetError(gc, GL_INVALID_OPERATION);
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

GLvoid APIENTRY __gllc_SeparableFilter2D(__GLcontext *gc, GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *col)
{
    GLenum adjustFormat= format;
    GLenum adjustType= type;
    __GLdlistOp *dlop;
    struct __gllc_SeparableFilter2D_Rec  *data;
    GLint rowSize, colSize, imageSize;
    GLboolean index;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_SeparableFilter2D(gc, target, internalformat, width, height, format, type, row, col);
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
            __glSetError(gc, GL_INVALID_OPERATION);
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
            __glSetError(gc, GL_INVALID_OPERATION);
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

GLvoid APIENTRY __gllc_CopyConvolutionFilter1D(__GLcontext *gc, GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)
{
    __GLdlistOp *dlop;
    struct __gllc_CopyConvolutionFilter1D_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_CopyConvolutionFilter1D(gc, target, internalformat, x, y, width);
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

GLvoid APIENTRY __gllc_CopyConvolutionFilter2D(__GLcontext *gc, GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GLdlistOp *dlop;
    struct __gllc_CopyConvolutionFilter2D_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_CopyConvolutionFilter2D(gc, target, internalformat, x, y, width, height);
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

GLvoid APIENTRY __gllc_Histogram(__GLcontext *gc, GLenum target, GLsizei width, GLenum internalformat, GLboolean sink)
{
    __GLdlistOp *dlop;
    struct __gllc_Histogram_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Histogram(gc, target, width, internalformat, sink);
    }

    switch (target)
    {
    case GL_PROXY_HISTOGRAM:
        __glim_Histogram(gc, target, width, internalformat, sink);
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

GLvoid APIENTRY __gllc_ResetHistogram(__GLcontext *gc, GLenum target)
{
    __GLdlistOp *dlop;
    struct __gllc_ResetHistogram_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ResetHistogram(gc, target);
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

GLvoid APIENTRY __gllc_Minmax(__GLcontext *gc, GLenum target, GLenum internalFormat, GLboolean sink)
{
    __GLdlistOp *dlop;
    struct __gllc_Minmax_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Minmax(gc, target, internalFormat, sink);
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

GLvoid APIENTRY __gllc_ResetMinmax(__GLcontext *gc, GLenum target)
{
    __GLdlistOp *dlop;
    struct __gllc_ResetMinmax_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_ResetMinmax(gc, target);
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

GLvoid APIENTRY __gllc_LoadTransposeMatrixf(__GLcontext *gc, const GLfloat *m)
{
    __GLdlistOp *dlop;
    struct __gllc_LoadMatrixf_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_LoadTransposeMatrixf(gc, m);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_LoadMatrixf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_LoadTransposeMatrixf;
    data = (struct __gllc_LoadMatrixf_Rec *)(dlop + 1);
    __GL_MEMCOPY(data->m, m, sizeof(data->m));
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_LoadTransposeMatrixd(__GLcontext *gc, const GLdouble *m)
{
    __GLdlistOp *dlop;
    struct __gllc_LoadMatrixd_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_LoadTransposeMatrixd(gc, m);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_LoadMatrixd_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_LoadTransposeMatrixd;
    data = (struct __gllc_LoadMatrixd_Rec *)(dlop + 1);
    __GL_MEMCOPY(data->m, m, sizeof(data->m));
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultTransposeMatrixf(__GLcontext *gc, const GLfloat *m)
{
    __GLdlistOp *dlop;
    struct __gllc_MultMatrixf_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_MultTransposeMatrixf(gc, m);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultMatrixf_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MultTransposeMatrixf;
    data = (struct __gllc_MultMatrixf_Rec *)(dlop + 1);
    __GL_MEMCOPY(data->m, m, sizeof(data->m));
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_MultTransposeMatrixd(__GLcontext *gc, const GLdouble *m)
{
    __GLdlistOp *dlop;
    struct __gllc_MultMatrixd_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_MultTransposeMatrixd(gc, m);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_MultMatrixd_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MultTransposeMatrixd;
    data = (struct __gllc_MultMatrixd_Rec *)(dlop + 1);
    __GL_MEMCOPY(data->m, m, sizeof(data->m));
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_PointParameterfv(__GLcontext *gc, GLenum pname, const GLfloat *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint paramSize;
    struct __gllc_PointParameterfv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PointParameterfv(gc, pname, params);
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

GLvoid APIENTRY __gllc_PointParameterf(__GLcontext *gc, GLenum pname, GLfloat param)
{


    if (__glPointParameter_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_PointParameterfv(gc, pname, &param);
}

GLvoid APIENTRY __gllc_PointParameteriv(__GLcontext *gc, GLenum pname, const GLint *params)
{
    __GLdlistOp *dlop;
    GLuint size;
    GLint paramSize;
    struct __gllc_PointParameteriv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_PointParameteriv(gc, pname, params);
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

GLvoid APIENTRY __gllc_PointParameteri(__GLcontext *gc, GLenum pname, GLint param)
{


    if (__glPointParameter_size(pname) != 1) {
        __gllc_InvalidEnum(gc);
        return;
    }
    __gllc_PointParameteriv(gc, pname, &param);
}

GLvoid APIENTRY __gllc_WindowPos2sv(__GLcontext *gc, const GLshort  *v)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos2sv(gc, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_WindowPos2fv;
    data = (struct __gllc_WindowPos2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_WindowPos2s(__GLcontext *gc, GLshort x, GLshort y)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos2s(gc, x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_WindowPos2fv;
    data = (struct __gllc_WindowPos2fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_WindowPos2iv(__GLcontext *gc, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos2iv(gc, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_WindowPos2fv;
    data = (struct __gllc_WindowPos2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_WindowPos2i(__GLcontext *gc, GLint x, GLint y)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos2i(gc, x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_WindowPos2fv;
    data = (struct __gllc_WindowPos2fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_WindowPos2fv(__GLcontext *gc, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos2fv(gc, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_WindowPos2fv;
    data = (struct __gllc_WindowPos2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_WindowPos2f(__GLcontext *gc, GLfloat x, GLfloat y)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos2f(gc, x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_WindowPos2fv;
    data = (struct __gllc_WindowPos2fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_WindowPos2dv(__GLcontext *gc, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos2dv(gc, v);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_WindowPos2fv;
    data = (struct __gllc_WindowPos2fv_Rec *)(dlop + 1);
    data->v[0] = v[0];
    data->v[1] = v[1];
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_WindowPos2d(__GLcontext *gc, GLdouble x, GLdouble y)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos2fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos2d(gc, x, y);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_RasterPos2fv_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_WindowPos2fv;
    data = (struct __gllc_WindowPos2fv_Rec *)(dlop + 1);
    data->v[0] = x;
    data->v[1] = y;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_WindowPos3sv(__GLcontext *gc, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos3sv(gc, v);
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

GLvoid APIENTRY __gllc_WindowPos3s(__GLcontext *gc, GLshort x, GLshort y, GLshort z)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos3s(gc, x, y, z);
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

GLvoid APIENTRY __gllc_WindowPos3iv(__GLcontext *gc, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos3iv(gc, v);
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

GLvoid APIENTRY __gllc_WindowPos3i(__GLcontext *gc, GLint x, GLint y, GLint z)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos3i(gc, x, y, z);
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

GLvoid APIENTRY __gllc_WindowPos3fv(__GLcontext *gc, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos3fv(gc, v);
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

GLvoid APIENTRY __gllc_WindowPos3f(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos3f(gc, x, y, z);
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

GLvoid APIENTRY __gllc_WindowPos3dv(__GLcontext *gc, const GLdouble*v)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos3dv(gc, v);
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

GLvoid APIENTRY __gllc_WindowPos3d(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z)
{
    __GLdlistOp *dlop;
    struct __gllc_WindowPos3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_WindowPos3d(gc, x, y, z);
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

GLvoid APIENTRY __gllc_SecondaryColor3bv(__GLcontext *gc, const GLbyte *v)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->SecondaryColor3bv)(gc, v);
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

GLvoid APIENTRY __gllc_SecondaryColor3b(__GLcontext *gc, GLbyte red, GLbyte green, GLbyte blue)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->SecondaryColor3b)(gc, red, green, blue);
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

GLvoid APIENTRY __gllc_SecondaryColor3dv(__GLcontext *gc, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->SecondaryColor3dv)(gc, v);
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

GLvoid APIENTRY __gllc_SecondaryColor3d(__GLcontext *gc, GLdouble red, GLdouble green, GLdouble blue)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->SecondaryColor3d)(gc, red, green, blue);
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

GLvoid APIENTRY __gllc_SecondaryColor3fv(__GLcontext *gc, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->SecondaryColor3fv)(gc, v);
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

GLvoid APIENTRY __gllc_SecondaryColor3f(__GLcontext *gc, GLfloat red, GLfloat green, GLfloat blue)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->SecondaryColor3f)(gc, red, green, blue);
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

GLvoid APIENTRY __gllc_SecondaryColor3iv(__GLcontext *gc, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->SecondaryColor3iv)(gc, v);
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

GLvoid APIENTRY __gllc_SecondaryColor3i(__GLcontext *gc, GLint red, GLint green, GLint blue)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->SecondaryColor3i)(gc, red, green, blue);
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

GLvoid APIENTRY __gllc_SecondaryColor3sv(__GLcontext *gc, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->SecondaryColor3sv)(gc, v);
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

GLvoid APIENTRY __gllc_SecondaryColor3s(__GLcontext *gc, GLshort red, GLshort green, GLshort blue)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->SecondaryColor3s)(gc, red, green, blue);
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

GLvoid APIENTRY __gllc_SecondaryColor3ubv(__GLcontext *gc, const GLubyte *v)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->SecondaryColor3ubv)(gc, v);
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

GLvoid APIENTRY __gllc_SecondaryColor3ub(__GLcontext *gc, GLubyte red, GLubyte green, GLubyte blue)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->SecondaryColor3ub)(gc, red, green, blue);
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

GLvoid APIENTRY __gllc_SecondaryColor3uiv(__GLcontext *gc, const GLuint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->SecondaryColor3uiv)(gc, v);
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

GLvoid APIENTRY __gllc_SecondaryColor3ui(__GLcontext *gc, GLuint red, GLuint green, GLuint blue)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->SecondaryColor3ui)(gc, red, green, blue);
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

GLvoid APIENTRY __gllc_SecondaryColor3usv(__GLcontext *gc, const GLushort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->SecondaryColor3usv)(gc, v);
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

GLvoid APIENTRY __gllc_SecondaryColor3us(__GLcontext *gc, GLushort red, GLushort green, GLushort blue)
{
    __GLdlistOp *dlop;
    struct __gllc_SecondaryColor3fv_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->currentImmediateTable->SecondaryColor3us)(gc, red, green, blue);
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

GLvoid APIENTRY __gllc_BeginQuery(__GLcontext *gc, GLenum target, GLuint id)
{
    __GLdlistOp *dlop;
    struct __gllc_BeginQuery_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_BeginQuery(gc, target, id);
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

GLvoid APIENTRY __gllc_EndQuery(__GLcontext *gc, GLenum target)
{
    __GLdlistOp *dlop;
    struct __gllc_EndQuery_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_EndQuery(gc, target);
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
GLvoid APIENTRY __gllc_BeginQueryNV(__GLcontext *gc, GLuint id)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_EndQueryNV(__GLcontext *gc)
{
/* still not added, to do*/
GL_ASSERT(0);
}
#endif

GLvoid APIENTRY __gllc_Uniform4f(__GLcontext *gc, GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform4f_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform4f(gc, location, x, y, z, w);
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

GLvoid APIENTRY __gllc_Uniform3f(__GLcontext *gc, GLint location, GLfloat x, GLfloat y, GLfloat z)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform3f_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform3f(gc, location, x, y, z);
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

GLvoid APIENTRY __gllc_Uniform2f(__GLcontext *gc, GLint location, GLfloat x, GLfloat y)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform2f_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform2f(gc, location, x, y);
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

GLvoid APIENTRY __gllc_Uniform1f(__GLcontext *gc, GLint location, GLfloat x)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform1f_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform1f(gc, location, x);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Uniform1f_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Uniform1f;
    data = (struct __gllc_Uniform1f_Rec *)(dlop + 1);
    data->location = location;
    data->x = x;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Uniform4i(__GLcontext *gc, GLint location, GLint x, GLint y , GLint z , GLint w)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform4i_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform4i(gc, location, x, y, z, w);
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

GLvoid APIENTRY __gllc_Uniform3i(__GLcontext *gc, GLint location, GLint x , GLint y , GLint z)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform3i_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform3i(gc, location, x, y, z);
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

GLvoid APIENTRY __gllc_Uniform2i(__GLcontext *gc, GLint location, GLint x , GLint y)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform2i_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform2i(gc, location, x, y);
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

GLvoid APIENTRY __gllc_Uniform1i(__GLcontext *gc, GLint location, GLint x)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform1i_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform1i(gc, location, x);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_Uniform1i_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_Uniform1i;
    data = (struct __gllc_Uniform1i_Rec *)(dlop + 1);
    data->location = location;
    data->x = x;
    __glDlistAppendOp(gc, dlop);
}

GLvoid APIENTRY __gllc_Uniform4fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform4fv_Rec *data;
    GLint arraySize;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform4fv(gc, location, count, v);
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

GLvoid APIENTRY __gllc_Uniform3fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform3fv_Rec *data;
    GLint arraySize;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform3fv(gc, location, count, v);
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

GLvoid APIENTRY __gllc_Uniform2fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform2fv_Rec *data;
    GLint arraySize;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform2fv(gc, location, count, v);
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

GLvoid APIENTRY __gllc_Uniform1fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform1fv_Rec *data;
    GLint arraySize;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform1fv(gc, location, count, v);
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


GLvoid APIENTRY __gllc_Uniform4iv(__GLcontext *gc, GLint location, GLsizei count, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform4iv_Rec *data;
    GLint arraySize;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform4iv(gc, location, count, v);
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

GLvoid APIENTRY __gllc_Uniform3iv(__GLcontext *gc, GLint location, GLsizei count, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform3iv_Rec *data;
    GLint arraySize;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform3iv(gc, location, count, v);
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

GLvoid APIENTRY __gllc_Uniform2iv(__GLcontext *gc, GLint location, GLsizei count, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform2iv_Rec *data;
    GLint arraySize;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform2iv(gc, location, count, v);
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

GLvoid APIENTRY __gllc_Uniform1iv(__GLcontext *gc, GLint location, GLsizei count, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_Uniform1iv_Rec *data;
    GLint arraySize;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_Uniform1iv(gc, location, count, v);
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

GLvoid APIENTRY __gllc_UniformMatrix4fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_UniformMatrix4fv_Rec *data;
    GLint arraySize;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_UniformMatrix4fv(gc, location, count, transpose, v);
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

GLvoid APIENTRY __gllc_UniformMatrix3fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_UniformMatrix3fv_Rec *data;
    GLint arraySize;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_UniformMatrix3fv(gc, location, count, transpose, v);
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

GLvoid APIENTRY __gllc_UniformMatrix2fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_UniformMatrix2fv_Rec *data;
    GLint arraySize;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_UniformMatrix2fv(gc, location, count, transpose, v);
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

GLvoid APIENTRY __gllc_UniformMatrix2x3fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_UniformMatrix2x3fv_Rec *data;
    GLint arraySize;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_UniformMatrix2x3fv(gc, location, count, transpose, v);
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

GLvoid APIENTRY __gllc_UniformMatrix2x4fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_UniformMatrix2x4fv_Rec *data;
    GLint arraySize;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_UniformMatrix2x4fv(gc, location, count, transpose, v);
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

GLvoid APIENTRY __gllc_UniformMatrix3x2fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_UniformMatrix3x2fv_Rec *data;
    GLint arraySize;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_UniformMatrix3x2fv(gc, location, count, transpose, v);
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

GLvoid APIENTRY __gllc_UniformMatrix3x4fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_UniformMatrix3x4fv_Rec *data;
    GLint arraySize;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_UniformMatrix3x4fv(gc, location, count, transpose, v);
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

GLvoid APIENTRY __gllc_UniformMatrix4x2fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_UniformMatrix4x2fv_Rec *data;
    GLint arraySize;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_UniformMatrix4x2fv(gc, location, count, transpose, v);
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

GLvoid APIENTRY __gllc_UniformMatrix4x3fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_UniformMatrix4x3fv_Rec *data;
    GLint arraySize;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_UniformMatrix4x3fv(gc, location, count, transpose, v);
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


GLvoid APIENTRY __gllc_DrawBuffers(__GLcontext *gc, GLsizei count, const GLenum *bufs)
{
    __GLdlistOp *dlop;
    struct __gllc_DrawBuffers_Rec *data;
    GLint arraySize;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_DrawBuffers(gc, count, bufs);
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

GLvoid APIENTRY __gllc_UseProgram(__GLcontext *gc, GLuint program)
{
    __GLdlistOp *dlop;
    struct __gllc_UseProgram_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_UseProgram(gc, program);
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_UseProgram_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_UseProgram;
    data = (struct __gllc_UseProgram_Rec *)(dlop + 1);
    data->program = program;
    __glDlistAppendOp(gc, dlop);
}


#if GL_ARB_vertex_program

GLvoid APIENTRY __gllc_BindProgramARB(__GLcontext *gc, GLenum target, GLuint program)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_ProgramEnvParameter4dARB(__GLcontext *gc, GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_ProgramEnvParameter4dvARB(__GLcontext *gc, GLenum target, GLuint index, const GLdouble *params)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_ProgramEnvParameter4fARB(__GLcontext *gc, GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_ProgramEnvParameter4fvARB(__GLcontext *gc, GLenum target, GLuint index, const GLfloat *params)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_ProgramLocalParameter4dARB(__GLcontext *gc, GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_ProgramLocalParameter4dvARB(__GLcontext *gc, GLenum target, GLuint index, const GLdouble *params)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_ProgramLocalParameter4fARB(__GLcontext *gc, GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_ProgramLocalParameter4fvARB(__GLcontext *gc, GLenum target, GLuint index, const GLfloat *params)
{
/* still not added, to do*/
GL_ASSERT(0);
}

#endif

#if GL_ATI_element_array
GLvoid  APIENTRY __gllc_DrawElementArrayATI(__GLcontext *gc, GLenum mode, GLsizei count)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid  APIENTRY __gllc_DrawRangeElementArrayATI(__GLcontext *gc, GLenum mode, GLuint start,
                                  GLuint end, GLsizei count)
{
/* still not added, to do*/
GL_ASSERT(0);
}
#endif

#if GL_EXT_stencil_two_side
GLvoid APIENTRY __gllc_ActiveStencilFaceEXT(__GLcontext *gc, GLenum face)
{
/* still not added, to do*/
GL_ASSERT(0);
}
#endif

#if GL_EXT_texture_integer
GLvoid APIENTRY __gllc_ClearColorIiEXT(__GLcontext *gc, GLint r, GLint g, GLint b, GLint a)
{
/* still not added, to do*/
GL_ASSERT(0);
}


GLvoid APIENTRY __gllc_ClearColorIuiEXT(__GLcontext *gc, GLuint r, GLuint g, GLuint b, GLuint a)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_TexParameterIivEXT(__GLcontext *gc, GLenum target, GLenum pname, GLint *params)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_TexParameterIuivEXT(__GLcontext *gc, GLenum target, GLenum pname, GLuint *params)
{
/* still not added, to do*/
GL_ASSERT(0);
}
#endif

#if GL_EXT_gpu_shader4
GLvoid APIENTRY __gllc_VertexAttribI1iEXT(__GLcontext *gc, GLuint index, GLint x)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_VertexAttribI2iEXT(__GLcontext *gc, GLuint index, GLint x, GLint y)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_VertexAttribI3iEXT(__GLcontext *gc, GLuint index, GLint x, GLint y, GLint z)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_VertexAttribI4iEXT(__GLcontext *gc, GLuint index, GLint x, GLint y, GLint z, GLint w)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_VertexAttribI1uiEXT(__GLcontext *gc, GLuint index, GLuint x)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_VertexAttribI2uiEXT(__GLcontext *gc, GLuint index, GLuint x, GLuint y)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_VertexAttribI3uiEXT(__GLcontext *gc, GLuint index, GLuint x, GLuint y, GLuint z)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_VertexAttribI4uiEXT(__GLcontext *gc, GLuint index, GLuint x, GLuint y, GLuint z,
                            GLuint w)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_VertexAttribI1ivEXT(__GLcontext *gc, GLuint index, const GLint *v)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_VertexAttribI2ivEXT(__GLcontext *gc, GLuint index, const GLint *v)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_VertexAttribI3ivEXT(__GLcontext *gc, GLuint index, const GLint *v)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_VertexAttribI4ivEXT(__GLcontext *gc, GLuint index, const GLint *v)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_VertexAttribI1uivEXT(__GLcontext *gc, GLuint index, const GLuint *v)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_VertexAttribI2uivEXT(__GLcontext *gc, GLuint index, const GLuint *v)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_VertexAttribI3uivEXT(__GLcontext *gc, GLuint index, const GLuint *v)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_VertexAttribI4uivEXT(__GLcontext *gc, GLuint index, const GLuint *v)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_VertexAttribI4bvEXT(__GLcontext *gc, GLuint index, const GLbyte *v)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_VertexAttribI4svEXT(__GLcontext *gc, GLuint index, const GLshort *v)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_VertexAttribI4ubvEXT(__GLcontext *gc, GLuint index, const GLubyte *v)
{
/* still not added, to do*/
GL_ASSERT(0);
}
GLvoid APIENTRY __gllc_VertexAttribI4usvEXT(__GLcontext *gc, GLuint index, const GLushort *v)
{
/* still not added, to do*/
GL_ASSERT(0);
}


GLvoid APIENTRY __gllc_Uniform1uiEXT(__GLcontext *gc, GLint location, GLuint v0)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_Uniform2uiEXT(__GLcontext *gc, GLint location, GLuint v0, GLuint v1)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_Uniform3uiEXT(__GLcontext *gc, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_Uniform4uiEXT(__GLcontext *gc, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_Uniform1uivEXT(__GLcontext *gc, GLint location, GLsizei count, const GLuint *value)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_Uniform2uivEXT(__GLcontext *gc, GLint location, GLsizei count, const GLuint *value)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_Uniform3uivEXT(__GLcontext *gc, GLint location, GLsizei count, const GLuint *value)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_Uniform4uivEXT(__GLcontext *gc, GLint location, GLsizei count, const GLuint *value)
{
/* still not added, to do*/
GL_ASSERT(0);
}
#endif

#if GL_EXT_geometry_shader4
GLvoid APIENTRY __gllc_FramebufferTextureEXT(__GLcontext *gc, GLenum target, GLenum attachment, GLuint texture, GLint level)
{
/* still not added, to do*/
GL_ASSERT(0);
}
GLvoid APIENTRY __gllc_FramebufferTextureLayerEXT(__GLcontext *gc, GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_FramebufferTextureFaceEXT(__GLcontext *gc, GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face)
{
/* still not added, to do*/
GL_ASSERT(0);
}

#endif

#if GL_EXT_draw_buffers2

GLvoid APIENTRY __gllc_ColorMaskIndexedEXT(__GLcontext *gc, GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_EnableIndexedEXT(__GLcontext *gc, GLenum target, GLuint index)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_DisableIndexedEXT(__GLcontext *gc, GLenum target, GLuint index)
{
/* still not added, to do*/
GL_ASSERT(0);
}

#endif

#if GL_EXT_gpu_program_parameters

GLvoid APIENTRY __gllc_ProgramEnvParameters4fvEXT(__GLcontext *gc, GLenum target, GLuint index, GLsizei count, const GLfloat *params)
{
/* still not added, to do*/
GL_ASSERT(0);
}

GLvoid APIENTRY __gllc_ProgramLocalParameters4fvEXT(__GLcontext *gc, GLenum target, GLuint index, GLsizei count, const GLfloat *params)
{
/* still not added, to do*/
GL_ASSERT(0);
}

#endif

#if GL_ARB_color_buffer_float
GLvoid APIENTRY __gllc_ClampColorARB(__GLcontext *gc, GLenum target, GLenum clamp)
{
/* still not added, to do*/
GL_ASSERT(0);
}
#endif

#if GL_ATI_separate_stencil
GLvoid APIENTRY __gllc_StencilFuncSeparateATI(__GLcontext *gc, GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask)
{
/* still not added, to do*/
GL_ASSERT(0);
}
#endif

#if defined(_WIN32)
#pragma warning(default: 4244)
#endif
