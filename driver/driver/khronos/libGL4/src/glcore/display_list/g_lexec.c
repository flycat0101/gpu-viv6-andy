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
#include "gc_gl_image.h"
#include "gc_im_protos.h"

/*
*** Functions in eval.c
*/
extern GLint __glEvalComputeK(GLenum target);

/*
** Used to pad display list entries to double word boundaries where needed
** (for those few OpenGL commands which take double precision values).
*/
const GLubyte *__glle_Skip(__GLcontext *gc, const GLubyte *PC)
{
    return (PC);
}

/*
** These routines execute an error stored in a display list.
*/
const GLubyte *__glle_InvalidValue(__GLcontext *gc, const GLubyte *PC)
{
    __glSetError(gc, GL_INVALID_VALUE);
    return PC;
}

const GLubyte *__glle_InvalidEnum(__GLcontext *gc, const GLubyte *PC)
{
    __glSetError(gc, GL_INVALID_ENUM);
    return PC;
}

const GLubyte *__glle_InvalidOperation(__GLcontext *gc, const GLubyte *PC)
{
    __glSetError(gc, GL_INVALID_OPERATION);
    return PC;
}

const GLubyte *__glle_TableTooLarge(__GLcontext *gc, const GLubyte *PC)
{
    __glSetError(gc, GL_TABLE_TOO_LARGE);
    return PC;
}

/************************************************************************/

const GLubyte *__glle_ListBase(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_ListBase_Rec *data;

    data = (struct __gllc_ListBase_Rec *) PC;
    __glim_ListBase(gc, data->base);
    return PC + sizeof(struct __gllc_ListBase_Rec);
}

const GLubyte *__glle_Begin(__GLcontext *gc, const GLubyte *PC)
{

    struct __gllc_Begin_Rec *data;

    data = (struct __gllc_Begin_Rec *) PC;
    (*gc->currentImmediateTable->Begin)(gc, data->primType);
    return PC + sizeof(struct __gllc_Begin_Rec);
}

const GLubyte *__glle_Color3fv(__GLcontext *gc, const GLubyte *PC)
{

    struct __gllc_Color3fv_Rec *data;

    data = (struct __gllc_Color3fv_Rec *) PC;
    (*gc->currentImmediateTable->Color3fv)(gc, data->v);
    return PC + sizeof(struct __gllc_Color3fv_Rec);
}

const GLubyte *__glle_Color4fv(__GLcontext *gc, const GLubyte *PC)
{

    struct __gllc_Color4fv_Rec *data;

    data = (struct __gllc_Color4fv_Rec *) PC;
    (*gc->currentImmediateTable->Color4fv)(gc, data->v);
    return PC + sizeof(struct __gllc_Color4fv_Rec);
}

const GLubyte *__glle_Color4ubv(__GLcontext *gc, const GLubyte *PC)
{

    struct __gllc_Color4ubv_Rec *data;

    data = (struct __gllc_Color4ubv_Rec *) PC;
    (*gc->currentImmediateTable->Color4ubv)(gc, data->v);
    return PC + sizeof(struct __gllc_Color4ubv_Rec);
}

const GLubyte *__glle_EdgeFlag(__GLcontext *gc, const GLubyte *PC)
{

    struct __gllc_EdgeFlag_Rec *data;

    data = (struct __gllc_EdgeFlag_Rec *) PC;
    (*gc->currentImmediateTable->EdgeFlag)(gc, data->flag);
    return PC + sizeof(struct __gllc_EdgeFlag_Rec);
}

const GLubyte *__glle_End(__GLcontext *gc, const GLubyte *PC)
{


    (*gc->currentImmediateTable->End)(gc);
    return PC;
}

const GLubyte *__glle_Indexf(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Indexf_Rec *data;

    data = (struct __gllc_Indexf_Rec *) PC;
    __glim_Indexf(gc, data->c);
    return PC + sizeof(struct __gllc_Indexf_Rec);
}

const GLubyte *__glle_Normal3fv(__GLcontext *gc, const GLubyte *PC)
{

    struct __gllc_Normal3fv_Rec *data;

    data = (struct __gllc_Normal3fv_Rec *) PC;
    (*gc->currentImmediateTable->Normal3fv)(gc, data->v);
    return PC + sizeof(struct __gllc_Normal3fv_Rec);
}

const GLubyte *__glle_RasterPos2fv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_RasterPos2fv_Rec *data;

    data = (struct __gllc_RasterPos2fv_Rec *) PC;
    __glim_RasterPos2fv(gc, data->v);
    return PC + sizeof(struct __gllc_RasterPos2fv_Rec);
}

const GLubyte *__glle_RasterPos3fv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_RasterPos3fv_Rec *data;

    data = (struct __gllc_RasterPos3fv_Rec *) PC;
    __glim_RasterPos3fv(gc, data->v);
    return PC + sizeof(struct __gllc_RasterPos3fv_Rec);
}

const GLubyte *__glle_RasterPos4fv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_RasterPos4fv_Rec *data;

    data = (struct __gllc_RasterPos4fv_Rec *) PC;
    __glim_RasterPos4fv(gc, data->v);
    return PC + sizeof(struct __gllc_RasterPos4fv_Rec);
}

const GLubyte *__glle_Rectf(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Rectf_Rec *data;

    data = (struct __gllc_Rectf_Rec *) PC;
    __glim_Rectf(gc, data->x1, data->y1, data->x2, data->y2);
    return PC + sizeof(struct __gllc_Rectf_Rec);
}

const GLubyte *__glle_TexCoord2fv(__GLcontext *gc, const GLubyte *PC)
{

    struct __gllc_TexCoord2fv_Rec *data;

    data = (struct __gllc_TexCoord2fv_Rec *) PC;
    (*gc->currentImmediateTable->TexCoord2fv)(gc, data->v);
    return PC + sizeof(struct __gllc_TexCoord2fv_Rec);
}

const GLubyte *__glle_TexCoord3fv(__GLcontext *gc, const GLubyte *PC)
{

    struct __gllc_TexCoord3fv_Rec *data;

    data = (struct __gllc_TexCoord3fv_Rec *) PC;
    (*gc->currentImmediateTable->TexCoord3fv)(gc, data->v);
    return PC + sizeof(struct __gllc_TexCoord3fv_Rec);
}

const GLubyte *__glle_TexCoord4fv(__GLcontext *gc, const GLubyte *PC)
{

    struct __gllc_TexCoord4fv_Rec *data;

    data = (struct __gllc_TexCoord4fv_Rec *) PC;
    (*gc->currentImmediateTable->TexCoord4fv)(gc, data->v);
    return PC + sizeof(struct __gllc_TexCoord4fv_Rec);
}

const GLubyte *__glle_Vertex2fv(__GLcontext *gc, const GLubyte *PC)
{

    struct __gllc_Vertex2fv_Rec *data;

    data = (struct __gllc_Vertex2fv_Rec *) PC;
    (*gc->currentImmediateTable->Vertex2fv)(gc, data->v);
    return PC + sizeof(struct __gllc_Vertex2fv_Rec);
}

const GLubyte *__glle_Vertex3fv(__GLcontext *gc, const GLubyte *PC)
{

    struct __gllc_Vertex3fv_Rec *data;

    data = (struct __gllc_Vertex3fv_Rec *) PC;
    (*gc->currentImmediateTable->Vertex3fv)(gc, data->v);
    return PC + sizeof(struct __gllc_Vertex3fv_Rec);
}

const GLubyte *__glle_Vertex4fv(__GLcontext *gc, const GLubyte *PC)
{

    struct __gllc_Vertex4fv_Rec *data;

    data = (struct __gllc_Vertex4fv_Rec *) PC;
    (*gc->currentImmediateTable->Vertex4fv)(gc, data->v);
    return PC + sizeof(struct __gllc_Vertex4fv_Rec);
}

const GLubyte *__glle_ClipPlane(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_ClipPlane_Rec *data;

    data = (struct __gllc_ClipPlane_Rec *) PC;
    __glim_ClipPlane(gc, data->plane, data->equation);
    return PC + sizeof(struct __gllc_ClipPlane_Rec);
}

const GLubyte *__glle_ColorMaterial(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_ColorMaterial_Rec *data;

    data = (struct __gllc_ColorMaterial_Rec *) PC;
    __glim_ColorMaterial(gc, data->face, data->mode);
    return PC + sizeof(struct __gllc_ColorMaterial_Rec);
}

const GLubyte *__glle_CullFace(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_CullFace_Rec *data;

    data = (struct __gllc_CullFace_Rec *) PC;
    /*__glim_CullFace(gc, data->mode);*/
    __gles_CullFace(gc, data->mode);
    return PC + sizeof(struct __gllc_CullFace_Rec);
}

const GLubyte *__glle_Fogfv(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_Fogfv_Rec *data;

    data = (struct __gllc_Fogfv_Rec *) PC;
    __glim_Fogfv(gc, data->pname,
            (GLfloat *) (PC + sizeof(struct __gllc_Fogfv_Rec)));
    arraySize = __GL64PAD(__glFog_size(data->pname) * 4);
    size = sizeof(struct __gllc_Fogfv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_Fogiv(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_Fogiv_Rec *data;

    data = (struct __gllc_Fogiv_Rec *) PC;
    __glim_Fogiv(gc, data->pname,
            (GLint *) (PC + sizeof(struct __gllc_Fogiv_Rec)));
    arraySize = __GL64PAD(__glFog_size(data->pname) * 4);
    size = sizeof(struct __gllc_Fogiv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_FrontFace(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_FrontFace_Rec *data;

    data = (struct __gllc_FrontFace_Rec *) PC;
    /*__glim_FrontFace(gc, data->mode);*/
    __gles_FrontFace(gc, data->mode);
    return PC + sizeof(struct __gllc_FrontFace_Rec);
}

const GLubyte *__glle_Hint(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Hint_Rec *data;

    data = (struct __gllc_Hint_Rec *) PC;
    /*__glim_Hint(gc, data->target, data->mode);*/
    __gles_Hint(gc, data->target, data->mode);
    return PC + sizeof(struct __gllc_Hint_Rec);
}

const GLubyte *__glle_Lightfv(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_Lightfv_Rec *data;

    data = (struct __gllc_Lightfv_Rec *) PC;
    __glim_Lightfv(gc, data->light, data->pname,
            (GLfloat *) (PC + sizeof(struct __gllc_Lightfv_Rec)));
    arraySize = __GL64PAD(__glLight_size(data->pname) * 4);
    size = sizeof(struct __gllc_Lightfv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_Lightiv(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_Lightiv_Rec *data;

    data = (struct __gllc_Lightiv_Rec *) PC;
    __glim_Lightiv(gc, data->light, data->pname,
            (GLint *) (PC + sizeof(struct __gllc_Lightiv_Rec)));
    arraySize = __GL64PAD(__glLight_size(data->pname) * 4);
    size = sizeof(struct __gllc_Lightiv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_LightModelfv(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_LightModelfv_Rec *data;

    data = (struct __gllc_LightModelfv_Rec *) PC;
    __glim_LightModelfv(gc, data->pname,
            (GLfloat *) (PC + sizeof(struct __gllc_LightModelfv_Rec)));
    arraySize = __GL64PAD(__glLightModel_size(data->pname) * 4);
    size = sizeof(struct __gllc_LightModelfv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_LightModeliv(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_LightModeliv_Rec *data;

    data = (struct __gllc_LightModeliv_Rec *) PC;
    __glim_LightModeliv(gc, data->pname,
            (GLint *) (PC + sizeof(struct __gllc_LightModeliv_Rec)));
    arraySize = __GL64PAD(__glLightModel_size(data->pname) * 4);
    size = sizeof(struct __gllc_LightModeliv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_LineStipple(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_LineStipple_Rec *data;

    data = (struct __gllc_LineStipple_Rec *) PC;
    __glim_LineStipple(gc, data->factor, data->pattern);
    return PC + sizeof(struct __gllc_LineStipple_Rec);
}

const GLubyte *__glle_LineWidth(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_LineWidth_Rec *data;

    data = (struct __gllc_LineWidth_Rec *) PC;
    __glim_LineWidth(gc, data->width);
    return PC + sizeof(struct __gllc_LineWidth_Rec);
}

const GLubyte *__glle_Materialfv(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_Materialfv_Rec *data;

    data = (struct __gllc_Materialfv_Rec *) PC;
    __glim_Materialfv(gc, data->face, data->pname,
            (GLfloat *) (PC + sizeof(struct __gllc_Materialfv_Rec)));
    arraySize = __GL64PAD(__glMaterial_size(data->pname) * 4);
    size = sizeof(struct __gllc_Materialfv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_Materialiv(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_Materialiv_Rec *data;

    data = (struct __gllc_Materialiv_Rec *) PC;
    __glim_Materialiv(gc, data->face, data->pname,
            (GLint *) (PC + sizeof(struct __gllc_Materialiv_Rec)));
    arraySize = __GL64PAD(__glMaterial_size(data->pname) * 4);
    size = sizeof(struct __gllc_Materialiv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_PointSize(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_PointSize_Rec *data;

    data = (struct __gllc_PointSize_Rec *) PC;
    __glim_PointSize(gc, data->size);
    return PC + sizeof(struct __gllc_PointSize_Rec);
}

const GLubyte *__glle_PolygonMode(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_PolygonMode_Rec *data;

    data = (struct __gllc_PolygonMode_Rec *) PC;
    __glim_PolygonMode(gc, data->face, data->mode);
    return PC + sizeof(struct __gllc_PolygonMode_Rec);
}

const GLubyte *__glle_Scissor(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Scissor_Rec *data;

    data = (struct __gllc_Scissor_Rec *) PC;
    /*__glim_Scissor(gc, data->x, data->y, data->width, data->height);*/
    __gles_Scissor(gc, data->x, data->y, data->width, data->height);
    return PC + sizeof(struct __gllc_Scissor_Rec);
}

const GLubyte *__glle_ShadeModel(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_ShadeModel_Rec *data;

    data = (struct __gllc_ShadeModel_Rec *) PC;
    __glim_ShadeModel(gc, data->mode);
    return PC + sizeof(struct __gllc_ShadeModel_Rec);
}

const GLubyte *__glle_TexParameterfv(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_TexParameterfv_Rec *data;

    data = (struct __gllc_TexParameterfv_Rec *) PC;
    /*__glim_TexParameterfv(gc, data->target, data->pname,
            (GLfloat *) (PC + sizeof(struct __gllc_TexParameterfv_Rec)));*/
    __gles_TexParameterfv(gc, data->target, data->pname,
            (GLfloat *) (PC + sizeof(struct __gllc_TexParameterfv_Rec)));
    arraySize = __GL64PAD(__glTexParameter_size(data->pname) * 4);
    size = sizeof(struct __gllc_TexParameterfv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_TexParameteriv(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_TexParameteriv_Rec *data;

    data = (struct __gllc_TexParameteriv_Rec *) PC;
    /*__glim_TexParameteriv(gc, data->target, data->pname,
            (GLint *) (PC + sizeof(struct __gllc_TexParameteriv_Rec)));*/
    __gles_TexParameteriv(gc, data->target, data->pname,
            (GLint *) (PC + sizeof(struct __gllc_TexParameteriv_Rec)));
    arraySize = __GL64PAD(__glTexParameter_size(data->pname) * 4);
    size = sizeof(struct __gllc_TexParameteriv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_TexEnvfv(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_TexEnvfv_Rec *data;

    data = (struct __gllc_TexEnvfv_Rec *) PC;
    __glim_TexEnvfv(gc, data->target, data->pname,
            (GLfloat *) (PC + sizeof(struct __gllc_TexEnvfv_Rec)));
    arraySize = __GL64PAD(__glTexEnv_size(data->pname) * 4);
    size = sizeof(struct __gllc_TexEnvfv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_TexEnviv(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_TexEnviv_Rec *data;

    data = (struct __gllc_TexEnviv_Rec *) PC;
    __glim_TexEnviv(gc, data->target, data->pname,
            (GLint *) (PC + sizeof(struct __gllc_TexEnviv_Rec)));
    arraySize = __GL64PAD(__glTexEnv_size(data->pname) * 4);
    size = sizeof(struct __gllc_TexEnviv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_TexGendv(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_TexGendv_Rec *data;

    data = (struct __gllc_TexGendv_Rec *) PC;
    __glim_TexGendv(gc, data->coord, data->pname,
            (GLdouble *) (PC + sizeof(struct __gllc_TexGendv_Rec)));
    arraySize = __GL64PAD(__glTexGen_size(data->pname) * 8);
    size = sizeof(struct __gllc_TexGendv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_TexGenfv(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_TexGenfv_Rec *data;

    data = (struct __gllc_TexGenfv_Rec *) PC;
    __glim_TexGenfv(gc, data->coord, data->pname,
            (GLfloat *) (PC + sizeof(struct __gllc_TexGenfv_Rec)));
    arraySize = __GL64PAD(__glTexGen_size(data->pname) * 4);
    size = sizeof(struct __gllc_TexGenfv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_TexGeniv(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_TexGeniv_Rec *data;

    data = (struct __gllc_TexGeniv_Rec *) PC;
    __glim_TexGeniv(gc, data->coord, data->pname,
            (GLint *) (PC + sizeof(struct __gllc_TexGeniv_Rec)));
    arraySize = __GL64PAD(__glTexGen_size(data->pname) * 4);
    size = sizeof(struct __gllc_TexGeniv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_InitNames(__GLcontext *gc, const GLubyte *PC)
{

    __glim_InitNames(gc);
    return PC;
}

const GLubyte *__glle_LoadName(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_LoadName_Rec *data;

    data = (struct __gllc_LoadName_Rec *) PC;
    __glim_LoadName(gc, data->name);
    return PC + sizeof(struct __gllc_LoadName_Rec);
}

const GLubyte *__glle_PassThrough(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_PassThrough_Rec *data;

    data = (struct __gllc_PassThrough_Rec *) PC;
    __glim_PassThrough(gc, data->token);
    return PC + sizeof(struct __gllc_PassThrough_Rec);
}

const GLubyte *__glle_PopName(__GLcontext *gc, const GLubyte *PC)
{
    __glim_PopName(gc);
    return PC;
}

const GLubyte *__glle_PushName(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_PushName_Rec *data;

    data = (struct __gllc_PushName_Rec *) PC;
    __glim_PushName(gc, data->name);
    return PC + sizeof(struct __gllc_PushName_Rec);
}

const GLubyte *__glle_DrawBuffer(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_DrawBuffer_Rec *data;

    data = (struct __gllc_DrawBuffer_Rec *) PC;
    __glim_DrawBuffer(gc, data->mode);
    return PC + sizeof(struct __gllc_DrawBuffer_Rec);
}

const GLubyte *__glle_Clear(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Clear_Rec *data;

    data = (struct __gllc_Clear_Rec *) PC;
    /*__glim_Clear(gc, data->mask);*/
    __gles_Clear(gc, data->mask);
    return PC + sizeof(struct __gllc_Clear_Rec);
}

const GLubyte *__glle_ClearAccum(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_ClearAccum_Rec *data;

    data = (struct __gllc_ClearAccum_Rec *) PC;
    __glim_ClearAccum(gc, data->red, data->green, data->blue, data->alpha);
    return PC + sizeof(struct __gllc_ClearAccum_Rec);
}

const GLubyte *__glle_ClearIndex(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_ClearIndex_Rec *data;

    data = (struct __gllc_ClearIndex_Rec *) PC;
    __glim_ClearIndex(gc, data->c);
    return PC + sizeof(struct __gllc_ClearIndex_Rec);
}

const GLubyte *__glle_ClearColor(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_ClearColor_Rec *data;

    data = (struct __gllc_ClearColor_Rec *) PC;
    /*__glim_ClearColor(gc, data->red, data->green, data->blue, data->alpha);*/
    __gles_ClearColor(gc, data->red, data->green, data->blue, data->alpha);
    return PC + sizeof(struct __gllc_ClearColor_Rec);
}

const GLubyte *__glle_ClearStencil(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_ClearStencil_Rec *data;

    data = (struct __gllc_ClearStencil_Rec *) PC;
    /*__glim_ClearStencil(gc, data->s);*/
    __gles_ClearStencil(gc, data->s);
    return PC + sizeof(struct __gllc_ClearStencil_Rec);
}

const GLubyte *__glle_ClearDepth(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_ClearDepth_Rec *data;

    data = (struct __gllc_ClearDepth_Rec *) PC;
    /*__glim_ClearDepth(gc, data->depth);*/
    __gles_ClearDepthf(gc, (GLfloat)data->depth);
    return PC + sizeof(struct __gllc_ClearDepth_Rec);
}

const GLubyte *__glle_StencilMask(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_StencilMask_Rec *data;

    data = (struct __gllc_StencilMask_Rec *) PC;
    /*__glim_StencilMask(gc, data->mask);*/
    __gles_StencilMask(gc, data->mask);
    return PC + sizeof(struct __gllc_StencilMask_Rec);
}

const GLubyte *__glle_StencilMaskSeparate(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_StencilMaskSeparate_Rec *data;

    data = (struct __gllc_StencilMaskSeparate_Rec *) PC;
    /*__glim_StencilMaskSeparate(gc, data->face, data->mask);*/
    __gles_StencilMaskSeparate(gc, data->face, data->mask);
    return PC + sizeof(struct __gllc_StencilMaskSeparate_Rec);
}

const GLubyte *__glle_ColorMask(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_ColorMask_Rec *data;

    data = (struct __gllc_ColorMask_Rec *) PC;
    /*__glim_ColorMask(gc, data->red, data->green, data->blue, data->alpha);*/
    __gles_ColorMask(gc, data->red, data->green, data->blue, data->alpha);
    return PC + sizeof(struct __gllc_ColorMask_Rec);
}

const GLubyte *__glle_DepthMask(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_DepthMask_Rec *data;

    data = (struct __gllc_DepthMask_Rec *) PC;
    /*__glim_DepthMask(gc, data->flag);*/
    __gles_DepthMask(gc, data->flag);
    return PC + sizeof(struct __gllc_DepthMask_Rec);
}

const GLubyte *__glle_IndexMask(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_IndexMask_Rec *data;

    data = (struct __gllc_IndexMask_Rec *) PC;
    __glim_IndexMask(gc, data->mask);
    return PC + sizeof(struct __gllc_IndexMask_Rec);
}

const GLubyte *__glle_Accum(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Accum_Rec *data;

    data = (struct __gllc_Accum_Rec *) PC;
    __glim_Accum(gc, data->op, data->value);
    return PC + sizeof(struct __gllc_Accum_Rec);
}

const GLubyte *__glle_PopAttrib(__GLcontext *gc, const GLubyte *PC)
{

    __glim_PopAttrib(gc);
    return PC;
}

const GLubyte *__glle_PushAttrib(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_PushAttrib_Rec *data;

    data = (struct __gllc_PushAttrib_Rec *) PC;
    __glim_PushAttrib(gc, data->mask);
    return PC + sizeof(struct __gllc_PushAttrib_Rec);
}

const GLubyte *__glle_MapGrid1d(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_MapGrid1d_Rec *data;

    data = (struct __gllc_MapGrid1d_Rec *) PC;
    __glim_MapGrid1d(gc, data->un, data->u1, data->u2);
    return PC + sizeof(struct __gllc_MapGrid1d_Rec);
}

const GLubyte *__glle_MapGrid1f(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_MapGrid1f_Rec *data;

    data = (struct __gllc_MapGrid1f_Rec *) PC;
    __glim_MapGrid1f(gc, data->un, data->u1, data->u2);
    return PC + sizeof(struct __gllc_MapGrid1f_Rec);
}

const GLubyte *__glle_MapGrid2d(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_MapGrid2d_Rec *data;

    data = (struct __gllc_MapGrid2d_Rec *) PC;
    __glim_MapGrid2d(gc, data->un, data->u1, data->u2, data->vn,
            data->v1, data->v2);
    return PC + sizeof(struct __gllc_MapGrid2d_Rec);
}

const GLubyte *__glle_MapGrid2f(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_MapGrid2f_Rec *data;

    data = (struct __gllc_MapGrid2f_Rec *) PC;
    __glim_MapGrid2f(gc, data->un, data->u1, data->u2, data->vn,
            data->v1, data->v2);
    return PC + sizeof(struct __gllc_MapGrid2f_Rec);
}

const GLubyte *__glle_EvalCoord1dv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_EvalCoord1dv_Rec *data;

    data = (struct __gllc_EvalCoord1dv_Rec *) PC;
    __glim_EvalCoord1dv(gc, data->u);
    return PC + sizeof(struct __gllc_EvalCoord1dv_Rec);
}

const GLubyte *__glle_EvalCoord1fv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_EvalCoord1fv_Rec *data;

    data = (struct __gllc_EvalCoord1fv_Rec *) PC;
    __glim_EvalCoord1fv(gc, data->u);
    return PC + sizeof(struct __gllc_EvalCoord1fv_Rec);
}

const GLubyte *__glle_EvalCoord2dv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_EvalCoord2dv_Rec *data;

    data = (struct __gllc_EvalCoord2dv_Rec *) PC;
    __glim_EvalCoord2dv(gc, data->u);
    return PC + sizeof(struct __gllc_EvalCoord2dv_Rec);
}

const GLubyte *__glle_EvalCoord2fv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_EvalCoord2fv_Rec *data;

    data = (struct __gllc_EvalCoord2fv_Rec *) PC;
    __glim_EvalCoord2fv(gc, data->u);
    return PC + sizeof(struct __gllc_EvalCoord2fv_Rec);
}

const GLubyte *__glle_EvalMesh1(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_EvalMesh1_Rec *data;

    data = (struct __gllc_EvalMesh1_Rec *) PC;
    __glim_EvalMesh1(gc, data->mode, data->i1, data->i2);
    return PC + sizeof(struct __gllc_EvalMesh1_Rec);
}

const GLubyte *__glle_EvalPoint1(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_EvalPoint1_Rec *data;

    data = (struct __gllc_EvalPoint1_Rec *) PC;
    __glim_EvalPoint1(gc, data->i);
    return PC + sizeof(struct __gllc_EvalPoint1_Rec);
}

const GLubyte *__glle_EvalMesh2(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_EvalMesh2_Rec *data;

    data = (struct __gllc_EvalMesh2_Rec *) PC;
    __glim_EvalMesh2(gc, data->mode, data->i1, data->i2, data->j1,
            data->j2);
    return PC + sizeof(struct __gllc_EvalMesh2_Rec);
}

const GLubyte *__glle_EvalPoint2(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_EvalPoint2_Rec *data;

    data = (struct __gllc_EvalPoint2_Rec *) PC;
    __glim_EvalPoint2(gc, data->i, data->j);
    return PC + sizeof(struct __gllc_EvalPoint2_Rec);
}

const GLubyte *__glle_AlphaFunc(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_AlphaFunc_Rec *data;

    data = (struct __gllc_AlphaFunc_Rec *) PC;
    __glim_AlphaFunc(gc, data->func, data->ref);
    return PC + sizeof(struct __gllc_AlphaFunc_Rec);
}

const GLubyte *__glle_BlendColor(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_BlendColor_Rec *data;

    data = (struct __gllc_BlendColor_Rec *) PC;
    /*__glim_BlendColor(gc, data->r, data->g, data->b, data->a);*/
    __gles_BlendColor(gc, data->r, data->g, data->b, data->a);
    return PC + sizeof(struct __gllc_BlendColor_Rec);
}

const GLubyte *__glle_BlendFunc(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_BlendFunc_Rec *data;

    data = (struct __gllc_BlendFunc_Rec *) PC;
    /*__glim_BlendFunc(gc, data->sfactor, data->dfactor);*/
    __gles_BlendFunc(gc, data->sfactor, data->dfactor);
    return PC + sizeof(struct __gllc_BlendFunc_Rec);
}

const GLubyte *__glle_BlendFuncSeparate(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_BlendFuncSeparate_Rec *data;

    data = (struct __gllc_BlendFuncSeparate_Rec *) PC;
    /*__glim_BlendFuncSeparate(gc, data->sfactorRGB, data->dfactorRGB, data->sfactorAlpha, data->dfactorAlpha);*/
    __gles_BlendFuncSeparate(gc, data->sfactorRGB, data->dfactorRGB, data->sfactorAlpha, data->dfactorAlpha);
    return PC + sizeof(struct __gllc_BlendFuncSeparate_Rec);
}

const GLubyte *__glle_BlendEquation(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_BlendEquation_Rec *data;

    data = (struct __gllc_BlendEquation_Rec *) PC;
    /*__glim_BlendEquation(gc, data->mode);*/
    __gles_BlendEquation(gc, data->mode);
    return PC + sizeof(struct __gllc_BlendEquation_Rec);
}

const GLubyte *__glle_BlendEquationSeparate(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_BlendEquationSeparate_Rec *data;

    data = (struct __gllc_BlendEquationSeparate_Rec *) PC;
    /*__glim_BlendEquationSeparate(gc, data->modeRGB, data->modeAlpha);*/
    __gles_BlendEquationSeparate(gc, data->modeRGB, data->modeAlpha);
    return PC + sizeof(struct __gllc_BlendEquationSeparate_Rec);
}

const GLubyte *__glle_LogicOp(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_LogicOp_Rec *data;

    data = (struct __gllc_LogicOp_Rec *) PC;
    __glim_LogicOp(gc, data->opcode);
    return PC + sizeof(struct __gllc_LogicOp_Rec);
}

const GLubyte *__glle_StencilFunc(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_StencilFunc_Rec *data;

    data = (struct __gllc_StencilFunc_Rec *) PC;
    /*__glim_StencilFunc(gc, data->func, data->ref, data->mask);*/
    __gles_StencilFunc(gc, data->func, data->ref, data->mask);
    return PC + sizeof(struct __gllc_StencilFunc_Rec);
}

const GLubyte *__glle_StencilFuncSeparate(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_StencilFuncSeparate_Rec *data;

    data = (struct __gllc_StencilFuncSeparate_Rec *) PC;
    /*__glim_StencilFuncSeparate(gc, data->face, data->func, data->ref, data->mask);*/
    __gles_StencilFuncSeparate(gc, data->face, data->func, data->ref, data->mask);
    return PC + sizeof(struct __gllc_StencilFuncSeparate_Rec);
}

const GLubyte *__glle_StencilOp(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_StencilOp_Rec *data;

    data = (struct __gllc_StencilOp_Rec *) PC;
    /*__glim_StencilOp(gc, data->fail, data->zfail, data->zpass);*/
    __gles_StencilOp(gc, data->fail, data->zfail, data->zpass);
    return PC + sizeof(struct __gllc_StencilOp_Rec);
}

const GLubyte *__glle_StencilOpSeparate(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_StencilOpSeparate_Rec *data;

    data = (struct __gllc_StencilOpSeparate_Rec *) PC;
    /*__glim_StencilOpSeparate(gc, data->face, data->fail, data->zfail, data->zpass);*/
    __gles_StencilOpSeparate(gc, data->face, data->fail, data->zfail, data->zpass);
    return PC + sizeof(struct __gllc_StencilOpSeparate_Rec);
}

const GLubyte *__glle_DepthFunc(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_DepthFunc_Rec *data;

    data = (struct __gllc_DepthFunc_Rec *) PC;
    /*__glim_DepthFunc(gc, data->func);*/
    __gles_DepthFunc(gc, data->func);
    return PC + sizeof(struct __gllc_DepthFunc_Rec);
}

const GLubyte *__glle_PixelZoom(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_PixelZoom_Rec *data;

    data = (struct __gllc_PixelZoom_Rec *) PC;
    __glim_PixelZoom(gc, data->xfactor, data->yfactor);
    return PC + sizeof(struct __gllc_PixelZoom_Rec);
}

const GLubyte *__glle_PixelTransferf(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_PixelTransferf_Rec *data;

    data = (struct __gllc_PixelTransferf_Rec *) PC;
    __glim_PixelTransferf(gc, data->pname, data->param);
    return PC + sizeof(struct __gllc_PixelTransferf_Rec);
}

const GLubyte *__glle_PixelTransferi(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_PixelTransferi_Rec *data;

    data = (struct __gllc_PixelTransferi_Rec *) PC;
    __glim_PixelTransferi(gc, data->pname, data->param);
    return PC + sizeof(struct __gllc_PixelTransferi_Rec);
}

const GLubyte *__glle_PixelMapfv(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_PixelMapfv_Rec *data;

    data = (struct __gllc_PixelMapfv_Rec *) PC;
    __glim_PixelMapfv(gc, data->map, data->mapsize,
            (GLfloat *) (PC + sizeof(struct __gllc_PixelMapfv_Rec)));
    arraySize = __GL64PAD(data->mapsize * 4);
    size = sizeof(struct __gllc_PixelMapfv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_PixelMapuiv(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_PixelMapuiv_Rec *data;

    data = (struct __gllc_PixelMapuiv_Rec *) PC;
    __glim_PixelMapuiv(gc, data->map, data->mapsize,
            (GLuint *) (PC + sizeof(struct __gllc_PixelMapuiv_Rec)));
    arraySize = __GL64PAD(data->mapsize * 4);
    size = sizeof(struct __gllc_PixelMapuiv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_PixelMapusv(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_PixelMapusv_Rec *data;

    data = (struct __gllc_PixelMapusv_Rec *) PC;
    __glim_PixelMapusv(gc, data->map, data->mapsize,
            (GLushort *) (PC + sizeof(struct __gllc_PixelMapusv_Rec)));
    arraySize = __GL_PAD(data->mapsize * 2);
    size = sizeof(struct __gllc_PixelMapusv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_ReadBuffer(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_ReadBuffer_Rec *data;

    data = (struct __gllc_ReadBuffer_Rec *) PC;
    /*__glim_ReadBuffer(gc, data->mode);*/
    __gles_ReadBuffer(gc, data->mode);
    return PC + sizeof(struct __gllc_ReadBuffer_Rec);
}

const GLubyte *__glle_CopyPixels(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_CopyPixels_Rec *data;

    data = (struct __gllc_CopyPixels_Rec *) PC;
    __glim_CopyPixels(gc, data->x, data->y, data->width, data->height,
            data->type);
    return PC + sizeof(struct __gllc_CopyPixels_Rec);
}

const GLubyte *__glle_DepthRange(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_DepthRange_Rec *data;

    data = (struct __gllc_DepthRange_Rec *) PC;
    /*__glim_DepthRange(gc, data->zNear, data->zFar);*/
    __gles_DepthRangef(gc, (GLfloat)data->zNear, (GLfloat)data->zFar);
    return PC + sizeof(struct __gllc_DepthRange_Rec);
}

#if GL_EXT_depth_bounds_test
const GLubyte * __glle_DepthBoundsEXT(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return PC + sizeof(struct __gllc_DepthBoundTest_Rec);
}
#endif

const GLubyte *__glle_Frustum(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Frustum_Rec *data;

    data = (struct __gllc_Frustum_Rec *) PC;
    __glim_Frustum(gc, data->left, data->right, data->bottom, data->top,
            data->zNear, data->zFar);
    return PC + sizeof(struct __gllc_Frustum_Rec);
}

const GLubyte *__glle_LoadIdentity(__GLcontext *gc, const GLubyte *PC)
{

    __glim_LoadIdentity(gc);
    return PC;
}

const GLubyte *__glle_LoadMatrixf(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_LoadMatrixf_Rec *data;

    data = (struct __gllc_LoadMatrixf_Rec *) PC;
    __glim_LoadMatrixf(gc, data->m);
    return PC + sizeof(struct __gllc_LoadMatrixf_Rec);
}

const GLubyte *__glle_LoadMatrixd(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_LoadMatrixd_Rec *data;

    data = (struct __gllc_LoadMatrixd_Rec *) PC;
    __glim_LoadMatrixd(gc, data->m);
    return PC + sizeof(struct __gllc_LoadMatrixd_Rec);
}

const GLubyte *__glle_MatrixMode(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_MatrixMode_Rec *data;

    data = (struct __gllc_MatrixMode_Rec *) PC;
    __glim_MatrixMode(gc, data->mode);
    return PC + sizeof(struct __gllc_MatrixMode_Rec);
}

const GLubyte *__glle_MultMatrixf(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_MultMatrixf_Rec *data;

    data = (struct __gllc_MultMatrixf_Rec *) PC;
    __glim_MultMatrixf(gc, data->m);
    return PC + sizeof(struct __gllc_MultMatrixf_Rec);
}

const GLubyte *__glle_MultMatrixd(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_MultMatrixd_Rec *data;

    data = (struct __gllc_MultMatrixd_Rec *) PC;
    __glim_MultMatrixd(gc, data->m);
    return PC + sizeof(struct __gllc_MultMatrixd_Rec);
}

const GLubyte *__glle_LoadTransposeMatrixf(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_LoadMatrixf_Rec *data;

    data = (struct __gllc_LoadMatrixf_Rec *) PC;
    __glim_LoadTransposeMatrixf(gc, data->m);
    return PC + sizeof(struct __gllc_LoadMatrixf_Rec);
}

const GLubyte *__glle_LoadTransposeMatrixd(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_LoadMatrixd_Rec *data;

    data = (struct __gllc_LoadMatrixd_Rec *) PC;
    __glim_LoadTransposeMatrixd(gc, data->m);
    return PC + sizeof(struct __gllc_LoadMatrixd_Rec);
}

const GLubyte *__glle_MultTransposeMatrixf(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_MultMatrixf_Rec *data;

    data = (struct __gllc_MultMatrixf_Rec *) PC;
    __glim_MultTransposeMatrixf(gc, data->m);
    return PC + sizeof(struct __gllc_MultMatrixf_Rec);
}

const GLubyte *__glle_MultTransposeMatrixd(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_MultMatrixd_Rec *data;

    data = (struct __gllc_MultMatrixd_Rec *) PC;
    __glim_MultTransposeMatrixd(gc, data->m);
    return PC + sizeof(struct __gllc_MultMatrixd_Rec);
}

const GLubyte *__glle_Ortho(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Ortho_Rec *data;

    data = (struct __gllc_Ortho_Rec *) PC;
    __glim_Ortho(gc, data->left, data->right, data->bottom, data->top,
            data->zNear, data->zFar);
    return PC + sizeof(struct __gllc_Ortho_Rec);
}

const GLubyte *__glle_PopMatrix(__GLcontext *gc, const GLubyte *PC)
{

    __glim_PopMatrix(gc);
    return PC;
}

const GLubyte *__glle_PushMatrix(__GLcontext *gc, const GLubyte *PC)
{

    __glim_PushMatrix(gc);
    return PC;
}

const GLubyte *__glle_Rotated(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Rotated_Rec *data;

    data = (struct __gllc_Rotated_Rec *) PC;
    __glim_Rotated(gc, data->angle, data->x, data->y, data->z);
    return PC + sizeof(struct __gllc_Rotated_Rec);
}

const GLubyte *__glle_Rotatef(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Rotatef_Rec *data;

    data = (struct __gllc_Rotatef_Rec *) PC;
    __glim_Rotatef(gc, data->angle, data->x, data->y, data->z);
    return PC + sizeof(struct __gllc_Rotatef_Rec);
}

const GLubyte *__glle_Scaled(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Scaled_Rec *data;

    data = (struct __gllc_Scaled_Rec *) PC;
    __glim_Scaled(gc, data->x, data->y, data->z);
    return PC + sizeof(struct __gllc_Scaled_Rec);
}

const GLubyte *__glle_Scalef(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Scalef_Rec *data;

    data = (struct __gllc_Scalef_Rec *) PC;
    __glim_Scalef(gc, data->x, data->y, data->z);
    return PC + sizeof(struct __gllc_Scalef_Rec);
}

const GLubyte *__glle_Translated(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Translated_Rec *data;

    data = (struct __gllc_Translated_Rec *) PC;
    __glim_Translated(gc, data->x, data->y, data->z);
    return PC + sizeof(struct __gllc_Translated_Rec);
}

const GLubyte *__glle_Translatef(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Translatef_Rec *data;

    data = (struct __gllc_Translatef_Rec *) PC;
    __glim_Translatef(gc, data->x, data->y, data->z);
    return PC + sizeof(struct __gllc_Translatef_Rec);
}

const GLubyte *__glle_Viewport(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Viewport_Rec *data;

    data = (struct __gllc_Viewport_Rec *) PC;
    /*__glim_Viewport(gc, data->x, data->y, data->width, data->height);*/
    __gles_Viewport(gc, data->x, data->y, data->width, data->height);
    return PC + sizeof(struct __gllc_Viewport_Rec);
}

const GLubyte *__glle_PolygonOffset(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_PolygonOffset_Rec *data;

    data = (struct __gllc_PolygonOffset_Rec *) PC;
    __glim_PolygonOffset(gc, data->factor, data->units);
    return PC + sizeof(struct __gllc_PolygonOffset_Rec);
}

const GLubyte *__glle_CopyTexImage1D(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_CopyTexImage1D_Rec *data;

    data = (struct __gllc_CopyTexImage1D_Rec *) PC;
    __glim_CopyTexImage1D(gc, data->target, data->level, data->internalformat, data->x,
            data->y, data->width, data->border);
    return PC + sizeof(struct __gllc_CopyTexImage1D_Rec);
}

const GLubyte *__glle_CopyTexImage2D(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_CopyTexImage2D_Rec *data;

    data = (struct __gllc_CopyTexImage2D_Rec *) PC;
    __gles_CopyTexImage2D(gc, data->target, data->level, data->internalformat, data->x,
            data->y, data->width, data->height, data->border);
    return PC + sizeof(struct __gllc_CopyTexImage2D_Rec);
}

const GLubyte *__glle_CopyTexSubImage1D(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_CopyTexSubImage1D_Rec *data;

    data = (struct __gllc_CopyTexSubImage1D_Rec *) PC;
    __glim_CopyTexSubImage1D(gc, data->target, data->level, data->xoffset, data->x,
            data->y, data->width);
    return PC + sizeof(struct __gllc_CopyTexSubImage1D_Rec);
}

const GLubyte *__glle_CopyTexSubImage2D(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_CopyTexSubImage2D_Rec *data;

    data = (struct __gllc_CopyTexSubImage2D_Rec *) PC;
    __gles_CopyTexSubImage2D(gc, data->target, data->level, data->xoffset, data->yoffset,
            data->x, data->y, data->width, data->height);
    return PC + sizeof(struct __gllc_CopyTexSubImage2D_Rec);
}

const GLubyte *__glle_BindTexture(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_BindTexture_Rec *data;

    data = (struct __gllc_BindTexture_Rec *) PC;
    __gles_BindTexture(gc, data->target, data->texture);
    return PC + sizeof(struct __gllc_BindTexture_Rec);
}

const GLubyte *__glle_PrioritizeTextures(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize0;
    GLuint arraySize1;
    struct __gllc_PrioritizeTextures_Rec *data;

    data = (struct __gllc_PrioritizeTextures_Rec *) PC;
    arraySize0 = __GL64PAD(data->n * 4);
    arraySize1 = __GL64PAD(data->n * 4);
    size = sizeof(struct __gllc_PrioritizeTextures_Rec) + arraySize0 + arraySize1;
    __glim_PrioritizeTextures(gc, data->n,
            (GLuint *) (PC + sizeof(struct __gllc_PrioritizeTextures_Rec)),
            (GLclampf *) (PC + sizeof(struct __gllc_PrioritizeTextures_Rec) + arraySize0));
    return PC + size;
}

const GLubyte *__glle_ActiveTexture(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_ActiveTexture_Rec *data;

    data = (struct __gllc_ActiveTexture_Rec *) PC;
    __gles_ActiveTexture(gc, data->texture);
    return PC + sizeof( struct __gllc_ActiveTexture_Rec );
}

const GLubyte *__glle_MultiTexCoord2fv(__GLcontext *gc, const GLubyte *PC)
{

    struct __gllc_MultiTexCoord2fv_Rec *data;

    data = (struct __gllc_MultiTexCoord2fv_Rec *) PC;
    (*gc->currentImmediateTable->MultiTexCoord2fv)(gc, data->texture, data->v);
    return PC + sizeof(struct __gllc_MultiTexCoord2fv_Rec);
}

const GLubyte *__glle_MultiTexCoord3fv(__GLcontext *gc, const GLubyte *PC)
{

    struct __gllc_MultiTexCoord3fv_Rec *data;

    data = (struct __gllc_MultiTexCoord3fv_Rec *) PC;
    (*gc->currentImmediateTable->MultiTexCoord3fv)(gc, data->texture, data->v);
    return PC + sizeof(struct __gllc_MultiTexCoord3fv_Rec);
}

const GLubyte *__glle_MultiTexCoord4fv(__GLcontext *gc, const GLubyte *PC)
{

    struct __gllc_MultiTexCoord4fv_Rec *data;

    data = (struct __gllc_MultiTexCoord4fv_Rec *) PC;
    (*gc->currentImmediateTable->MultiTexCoord4fv)(gc, data->texture, data->v);
    return PC + sizeof(struct __gllc_MultiTexCoord4fv_Rec);
}

const GLubyte *__glle_FogCoordf(__GLcontext *gc, const GLubyte *PC)
{

    struct __gllc_FogCoordf_Rec *data;
    data = (struct __gllc_FogCoordf_Rec *)PC;
    (*gc->currentImmediateTable->FogCoordf)(gc, data->coord);
    return PC + sizeof(struct __gllc_FogCoordf_Rec);
}

const GLubyte * __glle_ColorTableParameteriv(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_ColorTableParameteriv_Rec *data;

    data = (struct __gllc_ColorTableParameteriv_Rec *) PC;
    __glim_ColorTableParameteriv(gc, data->target, data->pname,
            (GLint *) (PC + sizeof(struct __gllc_ColorTableParameteriv_Rec)));
    arraySize = __GL64PAD(__glColorTableParameter_size(data->pname) * 4);
    size = sizeof(struct __gllc_ColorTableParameteriv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_ColorTableParameterfv(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_ColorTableParameterfv_Rec *data;

    data = (struct __gllc_ColorTableParameterfv_Rec *) PC;
    __glim_ColorTableParameterfv(gc, data->target, data->pname,
            (GLfloat *) (PC + sizeof(struct __gllc_ColorTableParameterfv_Rec)));
    arraySize = __GL64PAD(__glColorTableParameter_size(data->pname) * 4);
    size = sizeof(struct __gllc_ColorTableParameterfv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_ConvolutionParameteriv(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_ConvolutionParameteriv_Rec *data;

    data = (struct __gllc_ConvolutionParameteriv_Rec *) PC;
    __glim_ConvolutionParameteriv(gc, data->target, data->pname,
            (GLint *) (PC + sizeof(struct __gllc_ConvolutionParameteriv_Rec)));
    arraySize = __GL64PAD(__glConvolutionParameter_size(data->pname) * 4);
    size = sizeof(struct __gllc_ConvolutionParameteriv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_ConvolutionParameterfv(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_ConvolutionParameterfv_Rec *data;

    data = (struct __gllc_ConvolutionParameterfv_Rec *) PC;
    __glim_ConvolutionParameterfv(gc, data->target, data->pname,
            (GLfloat *) (PC + sizeof(struct __gllc_ConvolutionParameterfv_Rec)));
    arraySize = __GL64PAD(__glConvolutionParameter_size(data->pname) * 4);
    size = sizeof(struct __gllc_ConvolutionParameterfv_Rec) + arraySize;

    return PC + size;
}

const GLubyte *__glle_VertexAttrib4fv(__GLcontext *gc, const GLubyte *PC)
{

    struct __gllc_VertexAttrib4fv_Rec *data;

    data = (struct __gllc_VertexAttrib4fv_Rec *)PC;
    (*gc->currentImmediateTable->VertexAttrib4fv)(gc, data->index, data->v);

    return PC + sizeof(struct __gllc_VertexAttrib4fv_Rec);
}

const GLubyte *__glle_Bitmap(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Bitmap_Rec *data;

    data = (struct __gllc_Bitmap_Rec *)PC;
    __glim_Bitmap(gc, data->width, data->height, data->xorig, data->yorig, data->xmove, data->ymove,
                    (const GLubyte *)(PC + sizeof(struct __gllc_Bitmap_Rec)));
    return (PC + sizeof(struct __gllc_Bitmap_Rec) + data->imageSize);
}

const GLubyte *__glle_PolygonStipple(__GLcontext *gc, const GLubyte *PC)
{
    __glim_PolygonStipple(gc, PC);

    return (PC + __glImageSize(32, 32, GL_COLOR_INDEX, GL_BITMAP));
}

const GLubyte *__glle_Map1f(__GLcontext *gc, const GLubyte *PC)
{
    const struct __gllc_Map1f_Rec *map1data;
    GLint k;

    map1data = (const struct __gllc_Map1f_Rec *) PC;
    k = __glEvalComputeK(map1data->target);

    /* Stride of "k" matches internal stride */
    __glim_Map1f(gc, map1data->target, map1data->u1, map1data->u2,
            k, map1data->order, (const GLfloat *)(PC + sizeof(*map1data)));

    return (PC + sizeof(*map1data) +
           __glMap1_size(k, map1data->order) * sizeof(GLfloat));
}

const GLubyte *__glle_Map1d(__GLcontext *gc, const GLubyte *PC)
{
    const struct __gllc_Map1f_Rec *map1data;
    GLint k;

    map1data = (const struct __gllc_Map1f_Rec *) PC;
    k = __glEvalComputeK(map1data->target);

    /* Stride of "k" matches internal stride */
    __glim_Map1f(gc, map1data->target, map1data->u1, map1data->u2,
            k, map1data->order, (const GLfloat *)(PC + sizeof(*map1data)));

    return (PC + sizeof(*map1data) +
           __glMap1_size(k, map1data->order) * sizeof(GLfloat));
}

const GLubyte *__glle_Map2f(__GLcontext *gc, const GLubyte *PC)
{
    const struct __gllc_Map2f_Rec *map2data;
    GLint k;

    map2data = (const struct __gllc_Map2f_Rec *) PC;
    k = __glEvalComputeK(map2data->target);

    /* Stride of "k" and "k * vorder" matches internal strides */
    __glim_Map2f(gc, map2data->target,
            map2data->u1, map2data->u2, k * map2data->vorder, map2data->uorder,
            map2data->v1, map2data->v2, k, map2data->vorder,
            (const GLfloat *)(PC + sizeof(*map2data)));

    return (PC + sizeof(*map2data) +
           __glMap2_size(k, map2data->uorder, map2data->vorder) *
           sizeof(GLfloat));
}

const GLubyte *__glle_Map2d(__GLcontext *gc, const GLubyte *PC)
{
    const struct __gllc_Map2f_Rec *map2data;
    GLint k;

    map2data = (const struct __gllc_Map2f_Rec *) PC;
    k = __glEvalComputeK(map2data->target);

    /* Stride of "k" and "k * vorder" matches internal strides */
    __glim_Map2f(gc, map2data->target,
            map2data->u1, map2data->u2, k * map2data->vorder, map2data->uorder,
            map2data->v1, map2data->v2, k, map2data->vorder,
            (const GLfloat *)(PC + sizeof(*map2data)));

    return (PC + sizeof(*map2data) +
           __glMap2_size(k, map2data->uorder, map2data->vorder) *
           sizeof(GLfloat));
}

const GLubyte *__glle_DrawPixels(__GLcontext *gc, const GLubyte *PC)
{
    const struct __gllc_DrawPixels_Rec *data;

    data = (const struct __gllc_DrawPixels_Rec *) PC;

    __glim_DrawPixels(gc,
            data->width, data->height, data->format, data->type,
            (const GLubyte *)(PC + sizeof(struct __gllc_DrawPixels_Rec)));

    return (PC + sizeof(struct __gllc_DrawPixels_Rec) + __GL_PAD(data->imageSize));
}

const GLubyte *__glle_TexImage1D(__GLcontext *gc, const GLubyte *PC)
{
    const struct __gllc_TexImage1D_Rec *data;

    data = (const struct __gllc_TexImage1D_Rec *) PC;

    __glim_TexImage1D(gc,
            data->target, data->level, data->components,
            data->width, data->border, data->format, data->type,
            (const GLubyte *)(PC + sizeof(struct __gllc_TexImage1D_Rec)));

    return (PC + sizeof(struct __gllc_TexImage1D_Rec) + __GL_PAD(data->imageSize));
}

const GLubyte *__glle_TexImage2D(__GLcontext *gc, const GLubyte *PC)
{
    const struct __gllc_TexImage2D_Rec *data;

    data = (const struct __gllc_TexImage2D_Rec *) PC;

    __gles_TexImage2D(gc,
            data->target, data->level, data->components,
            data->width, data->height, data->border, data->format, data->type,
            (const GLubyte *)(PC + sizeof(struct __gllc_TexImage2D_Rec)));

    return (PC + sizeof(struct __gllc_TexSubImage2D_Rec) + __GL_PAD(data->imageSize));
}

const GLubyte *__glle_TexSubImage1D(__GLcontext *gc, const GLubyte *PC)
{
    const struct __gllc_TexSubImage1D_Rec *data;

    data = (const struct __gllc_TexSubImage1D_Rec *) PC;

    __glim_TexSubImage1D(gc,
            data->target, data->level, data->xoffset,
            data->width, data->format, data->type,
            (const GLubyte *)(PC + sizeof(struct __gllc_TexSubImage1D_Rec)));

    return (PC + sizeof(struct __gllc_TexSubImage1D_Rec) + __GL_PAD(data->imageSize));
}

const GLubyte *__glle_TexSubImage2D(__GLcontext *gc, const GLubyte *PC)
{
    const struct __gllc_TexSubImage2D_Rec *data;

    data = (const struct __gllc_TexSubImage2D_Rec *) PC;

    __gles_TexSubImage2D(gc,
            data->target, data->level, data->xoffset, data->yoffset,
            data->width, data->height, data->format, data->type,
            (const GLubyte *)(PC + sizeof(struct __gllc_TexSubImage2D_Rec)));

    return (PC + sizeof(struct __gllc_TexSubImage2D_Rec) + __GL_PAD(data->imageSize));
}

const GLubyte *__glle_Disable(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Disable_Rec *data;

    data = (struct __gllc_Disable_Rec *) PC;
    __gles_Disable(gc, data->cap);
    return (PC + sizeof(struct __gllc_Disable_Rec));
}

const GLubyte *__glle_Enable(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Enable_Rec *data;

    data = (struct __gllc_Enable_Rec *) PC;
    __gles_Enable(gc, data->cap);
    return (PC + sizeof(struct __gllc_Enable_Rec));
}

const GLubyte *__glle_SampleCoverage(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_SampleCoverage_Rec *data;

    data = (struct __gllc_SampleCoverage_Rec *) PC;
    __gles_SampleCoverage(gc, data->v, data->invert);
    return PC + sizeof(struct __gllc_SampleCoverage_Rec);
}

const GLubyte *__glle_CompressedTexImage1D(__GLcontext *gc, const GLubyte *PC)
{
    return (PC);
}

const GLubyte *__glle_CompressedTexImage2D(__GLcontext *gc, const GLubyte *PC)
{
    const struct __gllc_CompressedTexImage2D_Rec *data;

    data = (const struct __gllc_CompressedTexImage2D_Rec *) PC;

    __gles_CompressedTexImage2D(gc,
            data->target, data->lod, data->components,
            data->width, data->height, data->border, data->imageSize,
            (const GLubyte *)(PC + sizeof(struct __gllc_CompressedTexImage2D_Rec)));

    return (PC + sizeof(struct __gllc_CompressedTexImage2D_Rec) + data->imageSize);
}

const GLubyte *__glle_CompressedTexImage3D(__GLcontext *gc, const GLubyte *PC)
{
    return (PC);
}

const GLubyte *__glle_CompressedTexSubImage1D(__GLcontext *gc, const GLubyte *PC)
{
    return (PC);
}

const GLubyte *__glle_CompressedTexSubImage2D(__GLcontext *gc, const GLubyte *PC)
{
    const struct __gllc_CompressedTexSubImage2D_Rec *data;

    data = (const struct __gllc_CompressedTexSubImage2D_Rec *) PC;

    __gles_CompressedTexSubImage2D(gc,
            data->target, data->lod, data->xoffset, data->yoffset,
            data->width, data->height, data->format, data->imageSize,
            (const GLubyte *)(PC + sizeof(struct __gllc_CompressedTexSubImage2D_Rec)));

    return (PC + sizeof(struct __gllc_CompressedTexSubImage2D_Rec) + data->imageSize);
}

const GLubyte *__glle_CompressedTexSubImage3D(__GLcontext *gc, const GLubyte *PC)
{
    return (PC);
}

const GLubyte *__glle_ColorTable(__GLcontext *gc, const GLubyte *PC)
{
    const struct __gllc_ColorTable_Rec *data;

    data = (const struct __gllc_ColorTable_Rec *) PC;

    __glim_ColorTable(gc, data->target, data->internalformat, data->width, data->format, data->type,
            (const GLvoid *) (PC + sizeof(struct __gllc_ColorTable_Rec)));

    return PC + sizeof(struct __gllc_ColorTable_Rec) + __GL_PAD(data->imageSize);
}

const GLubyte *__glle_ColorSubTable(__GLcontext *gc, const GLubyte *PC)
{
    const struct __gllc_ColorSubTable_Rec *data;

    data = (const struct __gllc_ColorSubTable_Rec *) PC;

    __glim_ColorSubTable(gc, data->target, data->start, data->count, data->format, data->type,
            (const GLvoid *) (PC + sizeof(struct __gllc_ColorSubTable_Rec)));

    return PC + sizeof(struct __gllc_ColorSubTable_Rec) + __GL_PAD(data->imageSize);
}

const GLubyte *__glle_CopyColorTable(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_CopyColorTable_Rec *data;

    data = (struct __gllc_CopyColorTable_Rec *) PC;
    __glim_CopyColorTable(gc, data->target, data->internalformat, data->x, data->y, data->width);
    return PC + sizeof(struct __gllc_CopyColorTable_Rec);
}

const GLubyte *__glle_CopyColorSubTable(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_CopyColorSubTable_Rec *data;

    data = (struct __gllc_CopyColorSubTable_Rec *) PC;
    __glim_CopyColorSubTable(gc, data->target, data->start, data->x, data->y, data->width);
    return PC + sizeof(struct __gllc_CopyColorSubTable_Rec);
}

const GLubyte *__glle_ConvolutionFilter1D(__GLcontext *gc, const GLubyte *PC)
{
    const struct __gllc_ConvolutionFilter1D_Rec *data;

    data = (const struct __gllc_ConvolutionFilter1D_Rec *) PC;
    __glim_ConvolutionFilter1D(gc,
            data->target, data->internalformat, data->width, data->format, data->type,
            (const GLubyte *)(PC + sizeof(struct __gllc_ConvolutionFilter1D_Rec)));
    return PC + sizeof(struct __gllc_ConvolutionFilter1D_Rec) + __GL_PAD(data->imageSize);
}

const GLubyte *__glle_ConvolutionFilter2D(__GLcontext *gc, const GLubyte *PC)
{
    const struct __gllc_ConvolutionFilter2D_Rec *data;

    data = (const struct __gllc_ConvolutionFilter2D_Rec *) PC;
    __glim_ConvolutionFilter2D(gc,
            data->target, data->internalformat, data->width, data->height, data->format, data->type,
            (const GLubyte *)(PC + sizeof(struct __gllc_ConvolutionFilter2D_Rec)));
    return PC + sizeof(struct __gllc_ConvolutionFilter2D_Rec) + __GL_PAD(data->imageSize);
}

const GLubyte *__glle_SeparableFilter2D(__GLcontext *gc, const GLubyte *PC)
{
    const struct __gllc_SeparableFilter2D_Rec *data;
    GLint rowSize, colSize;

    data = (const struct __gllc_SeparableFilter2D_Rec *) PC;
    rowSize = __glImageSize(data->width, 1, data->format, data->type);
    rowSize = __GL_PAD(rowSize);
    colSize = __glImageSize(data->height, 1, data->format, data->type);
    colSize = __GL_PAD(colSize);

    __glim_SeparableFilter2D(gc,
        data->target, data->internalformat, data->width, data->height, data->format, data->type,
        (const GLubyte *)(PC + sizeof(struct __gllc_SeparableFilter2D_Rec)),
        (const GLubyte *)(PC + sizeof(struct __gllc_SeparableFilter2D_Rec)) + rowSize);

    return PC + sizeof(struct __gllc_SeparableFilter2D_Rec) + rowSize + colSize;
}

const GLubyte *__glle_CopyConvolutionFilter1D(__GLcontext *gc, const GLubyte *PC)
{
    const struct __gllc_CopyConvolutionFilter1D_Rec *data;

    data = (const struct __gllc_CopyConvolutionFilter1D_Rec *) PC;
    __glim_CopyConvolutionFilter1D(gc, data->target, data->internalformat,
        data->x, data->y, data->width);
    return PC + sizeof(struct __gllc_CopyConvolutionFilter1D_Rec);
}

const GLubyte *__glle_CopyConvolutionFilter2D(__GLcontext *gc, const GLubyte *PC)
{
    const struct __gllc_CopyConvolutionFilter2D_Rec *data;

    data = (const struct __gllc_CopyConvolutionFilter2D_Rec *) PC;
    __glim_CopyConvolutionFilter2D(gc, data->target, data->internalformat,
        data->x, data->y, data->width, data->height);

    return PC + sizeof(struct __gllc_CopyConvolutionFilter2D_Rec);
}

const GLubyte *__glle_Histogram(__GLcontext *gc, const GLubyte *PC)
{
    const struct __gllc_Histogram_Rec *data;

    data = (const struct __gllc_Histogram_Rec *) PC;
    __glim_Histogram(gc, data->target, data->width,
        data->internalformat, data->sink);
    return PC + sizeof(struct __gllc_Histogram_Rec);
}

const GLubyte *__glle_ResetHistogram(__GLcontext *gc, const GLubyte *PC)
{
    const struct __gllc_ResetHistogram_Rec *data;

    data = (const struct __gllc_ResetHistogram_Rec *) PC;
    __glim_ResetHistogram(gc, data->target);
    return PC + sizeof(struct __gllc_ResetHistogram_Rec);
}

const GLubyte *__glle_Minmax(__GLcontext *gc, const GLubyte *PC)
{
    const struct __gllc_Minmax_Rec *data;

    data = (const struct __gllc_Minmax_Rec *) PC;
    __glim_Minmax(gc, data->target, data->internalFormat, data->sink);
    return PC + sizeof(struct __gllc_Minmax_Rec);
}

const GLubyte *__glle_ResetMinmax(__GLcontext *gc, const GLubyte *PC)
{
    const struct __gllc_ResetMinmax_Rec *data;

    data = (const struct __gllc_ResetMinmax_Rec *) PC;
    __glim_ResetMinmax(gc, data->target);
    return PC + sizeof(struct __gllc_ResetMinmax_Rec);
}

const GLubyte *__glle_TexImage3D(__GLcontext *gc, const GLubyte *PC)
{
    const struct __gllc_TexImage3D_Rec *data;

    data = (const struct __gllc_TexImage3D_Rec *) PC;

    __gles_TexImage3D(gc,
            data->target, data->lod, data->components, data->width, data->height,
            data->depth, data->border, data->format, data->type,
            (const GLubyte *)(PC + sizeof(struct __gllc_TexImage3D_Rec)));

    return (PC + sizeof(struct __gllc_TexImage3D_Rec) + __GL_PAD(data->imageSize));
}

const GLubyte *__glle_TexSubImage3D(__GLcontext *gc, const GLubyte *PC)
{
    const struct __gllc_TexSubImage3D_Rec *data;

    data = (const struct __gllc_TexSubImage3D_Rec *) PC;

    __gles_TexSubImage3D(gc,
            data->target, data->lod, data->xoffset, data->yoffset, data->zoffset,
            data->width, data->height, data->depth, data->format, data->type,
            (const GLubyte *)(PC + sizeof(struct __gllc_TexSubImage3D_Rec)));

    return (PC + sizeof(struct __gllc_TexSubImage3D_Rec) + __GL_PAD(data->imageSize));
}

const GLubyte *__glle_CopyTexSubImage3D(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_CopyTexSubImage3D_Rec *data;

    data = (struct __gllc_CopyTexSubImage3D_Rec *) PC;
    __gles_CopyTexSubImage3D(gc, data->target, data->level, data->xoffset, data->yoffset,
            data->zoffset, data->x, data->y, data->width, data->height);
    return (PC + sizeof(struct __gllc_CopyTexSubImage3D_Rec));
}

const GLubyte *__glle_PointParameterfv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_PointParameterfv_Rec *data;

    data = (struct __gllc_PointParameterfv_Rec *) PC;
    __glim_PointParameterfv(gc, data->pname, (GLfloat *)(PC+sizeof(struct __gllc_PointParameterfv_Rec)));
    return (PC + sizeof(struct __gllc_PointParameterfv_Rec) + data->paramSize);
}

const GLubyte *__glle_PointParameteriv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_PointParameteriv_Rec *data;

    data = (struct __gllc_PointParameteriv_Rec *) PC;
    __glim_PointParameteriv(gc, data->pname, (GLint *)(PC+sizeof(struct __gllc_PointParameteriv_Rec)));
    return (PC + sizeof(struct __gllc_PointParameteriv_Rec) + data->paramSize);
}

const GLubyte *__glle_WindowPos2fv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_WindowPos2fv_Rec *data;

    data = (struct __gllc_WindowPos2fv_Rec *) PC;
    __glim_WindowPos2fv(gc, data->v);
    return (PC + sizeof(struct __gllc_WindowPos2fv_Rec));
}

const GLubyte *__glle_WindowPos3fv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_WindowPos3fv_Rec *data;

    data = (struct __gllc_WindowPos3fv_Rec *) PC;
    __glim_WindowPos3fv(gc, data->v);
    return (PC + sizeof(struct __gllc_WindowPos3fv_Rec));
}

const  GLubyte *__glle_SecondaryColor3fv(__GLcontext *gc, const GLubyte *PC)
{

    struct __gllc_SecondaryColor3fv_Rec *data;

    data = (struct __gllc_SecondaryColor3fv_Rec *) PC;
    (*gc->currentImmediateTable->SecondaryColor3fv)(gc, data->v);
    return (PC + sizeof(struct __gllc_SecondaryColor3fv_Rec));
}

const GLubyte *__glle_BeginQuery(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_BeginQuery_Rec *data;

    data = (struct __gllc_BeginQuery_Rec *) PC;
    __gles_BeginQuery(gc, data->target, data->id);
    return (PC + sizeof(struct __gllc_BeginQuery_Rec));
}

const GLubyte *__glle_EndQuery(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_EndQuery_Rec *data;

    data = (struct __gllc_EndQuery_Rec *) PC;
    __gles_EndQuery(gc, data->target);
    return (PC + sizeof(struct __gllc_EndQuery_Rec));
}

#if GL_NV_occlusion_query
const GLubyte *__glle_BeginQueryNV(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return (PC + sizeof(struct __gllc_BeginQuery_Rec));
}

const GLubyte *__glle_EndQueryNV(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return (PC + sizeof(struct __gllc_EndQuery_Rec));
}
#endif

const GLubyte * __glle_Uniform4f(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Uniform4f_Rec *data;

    data = (struct __gllc_Uniform4f_Rec *) PC;
    __gles_Uniform4f(gc, data->location, data->x, data->y, data->z, data->w);
    return (PC + sizeof(struct __gllc_Uniform4f_Rec));
}

const GLubyte * __glle_Uniform3f(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Uniform3f_Rec *data;

    data = (struct __gllc_Uniform3f_Rec *) PC;
    __gles_Uniform3f(gc, data->location, data->x, data->y, data->z);
    return (PC + sizeof(struct __gllc_Uniform3f_Rec));
}

const GLubyte * __glle_Uniform2f(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Uniform2f_Rec *data;

    data = (struct __gllc_Uniform2f_Rec *) PC;
    __gles_Uniform2f(gc, data->location, data->x, data->y);
    return (PC + sizeof(struct __gllc_Uniform2f_Rec));
}

const GLubyte * __glle_Uniform1f(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Uniform1f_Rec *data;

    data = (struct __gllc_Uniform1f_Rec *) PC;
    __gles_Uniform1f(gc, data->location, data->x);
    return (PC + sizeof(struct __gllc_Uniform1f_Rec));
}

const GLubyte * __glle_Uniform4i(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Uniform4i_Rec *data;

    data = (struct __gllc_Uniform4i_Rec *) PC;
    __gles_Uniform4i(gc, data->location, data->x, data->y, data->z, data->w);
    return (PC + sizeof(struct __gllc_Uniform4i_Rec));
}

const GLubyte * __glle_Uniform3i(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Uniform3i_Rec *data;

    data = (struct __gllc_Uniform3i_Rec *) PC;
    __gles_Uniform3i(gc, data->location, data->x, data->y, data->z);
    return (PC + sizeof(struct __gllc_Uniform3i_Rec));
}

const GLubyte * __glle_Uniform2i(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Uniform2i_Rec *data;

    data = (struct __gllc_Uniform2i_Rec *) PC;
    __gles_Uniform2i(gc, data->location, data->x, data->y);
    return (PC + sizeof(struct __gllc_Uniform2i_Rec));
}

const GLubyte * __glle_Uniform1i(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Uniform1i_Rec *data;

    data = (struct __gllc_Uniform1i_Rec *) PC;
    __gles_Uniform1i(gc, data->location, data->x);
    return (PC + sizeof(struct __gllc_Uniform1i_Rec));
}

const GLubyte * __glle_Uniform4fv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Uniform4fv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_Uniform4fv_Rec *) PC;
    __gles_Uniform4fv(gc, data->location, data->count,
                    (GLfloat *)(PC + sizeof(struct __gllc_Uniform4fv_Rec)));
    arraySize = __GL64PAD(data->count * 4 * sizeof(GLfloat));
    size = sizeof(struct __gllc_Uniform4fv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_Uniform3fv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Uniform3fv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_Uniform3fv_Rec *) PC;
    __gles_Uniform3fv(gc, data->location, data->count,
                    (GLfloat *)(PC + sizeof(struct __gllc_Uniform3fv_Rec)));
    arraySize = __GL64PAD(data->count * 3 * sizeof(GLfloat));
    size = sizeof(struct __gllc_Uniform3fv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_Uniform2fv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Uniform2fv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_Uniform2fv_Rec *) PC;
    __gles_Uniform2fv(gc, data->location, data->count,
                    (GLfloat *)(PC + sizeof(struct __gllc_Uniform2fv_Rec)));
    arraySize = __GL64PAD(data->count * 2 * sizeof(GLfloat));
    size = sizeof(struct __gllc_Uniform2fv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_Uniform1fv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Uniform1fv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_Uniform1fv_Rec *) PC;
    __gles_Uniform1fv(gc, data->location, data->count,
                    (GLfloat *)(PC + sizeof(struct __gllc_Uniform1fv_Rec)));
    arraySize = __GL64PAD(data->count * sizeof(GLfloat));
    size = sizeof(struct __gllc_Uniform1fv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_Uniform4iv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Uniform4iv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_Uniform4iv_Rec *) PC;
    __gles_Uniform4iv(gc, data->location, data->count,
                    (GLint *)(PC + sizeof(struct __gllc_Uniform4iv_Rec)));
    arraySize = __GL64PAD(data->count * 4 * sizeof(GLint));
    size = sizeof(struct __gllc_Uniform4iv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_Uniform3iv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Uniform3iv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_Uniform3iv_Rec *) PC;
    __gles_Uniform3iv(gc, data->location, data->count,
                    (GLint *)(PC + sizeof(struct __gllc_Uniform3iv_Rec)));
    arraySize = __GL64PAD(data->count * 3 * sizeof(GLint));
    size = sizeof(struct __gllc_Uniform3iv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_Uniform2iv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Uniform2iv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_Uniform2iv_Rec *) PC;
    __gles_Uniform2iv(gc, data->location, data->count,
                    (GLint *)(PC + sizeof(struct __gllc_Uniform2iv_Rec)));
    arraySize = __GL64PAD(data->count * 2 * sizeof(GLint));
    size = sizeof(struct __gllc_Uniform2iv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_Uniform1iv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_Uniform1iv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_Uniform1iv_Rec *) PC;
    __gles_Uniform1iv(gc, data->location, data->count,
                    (GLint *)(PC + sizeof(struct __gllc_Uniform1iv_Rec)));
    arraySize = __GL64PAD(data->count * sizeof(GLint));
    size = sizeof(struct __gllc_Uniform1iv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_UniformMatrix4fv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_UniformMatrix4fv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_UniformMatrix4fv_Rec *) PC;
    __gles_UniformMatrix4fv(gc, data->location, data->count,
        data->transpose, (GLfloat *)(PC + sizeof(struct __gllc_UniformMatrix4fv_Rec)));
    arraySize = __GL64PAD(data->count * 16 * sizeof(GLfloat));
    size = sizeof(struct __gllc_UniformMatrix4fv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_UniformMatrix3fv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_UniformMatrix3fv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_UniformMatrix3fv_Rec *) PC;
    __gles_UniformMatrix3fv(gc, data->location, data->count,
        data->transpose, (GLfloat *)(PC + sizeof(struct __gllc_UniformMatrix3fv_Rec)));
    arraySize = __GL64PAD(data->count * 9 * sizeof(GLfloat));
    size = sizeof(struct __gllc_UniformMatrix3fv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_UniformMatrix2fv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_UniformMatrix2fv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_UniformMatrix2fv_Rec *) PC;
    __gles_UniformMatrix2fv(gc, data->location, data->count,
        data->transpose, (GLfloat *)(PC + sizeof(struct __gllc_UniformMatrix2fv_Rec)));
    arraySize = __GL64PAD(data->count * 4 * sizeof(GLfloat));
    size = sizeof(struct __gllc_UniformMatrix2fv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_UniformMatrix2x3fv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_UniformMatrix2x3fv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_UniformMatrix2x3fv_Rec *) PC;
    __gles_UniformMatrix2x3fv(gc, data->location, data->count,
        data->transpose, (GLfloat *)(PC + sizeof(struct __gllc_UniformMatrix2x3fv_Rec)));
    arraySize = __GL64PAD(data->count * 6 * sizeof(GLfloat));
    size = sizeof(struct __gllc_UniformMatrix2x3fv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_UniformMatrix2x4fv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_UniformMatrix2x4fv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_UniformMatrix2x4fv_Rec *) PC;
    __gles_UniformMatrix2x4fv(gc, data->location, data->count,
        data->transpose, (GLfloat *)(PC + sizeof(struct __gllc_UniformMatrix2x4fv_Rec)));
    arraySize = __GL64PAD(data->count * 8 * sizeof(GLfloat));
    size = sizeof(struct __gllc_UniformMatrix2x4fv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_UniformMatrix3x2fv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_UniformMatrix3x2fv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_UniformMatrix3x2fv_Rec *) PC;
    __gles_UniformMatrix3x2fv(gc, data->location, data->count,
        data->transpose, (GLfloat *)(PC + sizeof(struct __gllc_UniformMatrix3x2fv_Rec)));
    arraySize = __GL64PAD(data->count * 6 * sizeof(GLfloat));
    size = sizeof(struct __gllc_UniformMatrix3x2fv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_UniformMatrix3x4fv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_UniformMatrix3x4fv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_UniformMatrix3x4fv_Rec *) PC;
    __gles_UniformMatrix3x4fv(gc, data->location, data->count,
        data->transpose, (GLfloat *)(PC + sizeof(struct __gllc_UniformMatrix3x4fv_Rec)));
    arraySize = __GL64PAD(data->count * 12 * sizeof(GLfloat));
    size = sizeof(struct __gllc_UniformMatrix3x4fv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_UniformMatrix4x2fv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_UniformMatrix4x2fv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_UniformMatrix4x2fv_Rec *) PC;
    __gles_UniformMatrix4x2fv(gc, data->location, data->count,
        data->transpose, (GLfloat *)(PC + sizeof(struct __gllc_UniformMatrix4x2fv_Rec)));
    arraySize = __GL64PAD(data->count * 8 * sizeof(GLfloat));
    size = sizeof(struct __gllc_UniformMatrix4x2fv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_UniformMatrix4x3fv(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_UniformMatrix4x3fv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_UniformMatrix4x3fv_Rec *) PC;
    __gles_UniformMatrix4x3fv(gc, data->location, data->count,
        data->transpose, (GLfloat *)(PC + sizeof(struct __gllc_UniformMatrix4x3fv_Rec)));
    arraySize = __GL64PAD(data->count * 12 * sizeof(GLfloat));
    size = sizeof(struct __gllc_UniformMatrix4x3fv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_DrawBuffers(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_DrawBuffers_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_DrawBuffers_Rec *) PC;
    __gles_DrawBuffers(gc, data->count,
            (GLenum *)(PC + sizeof(struct __gllc_DrawBuffers_Rec)));
    arraySize = __GL64PAD(data->count * sizeof(GLenum));
    size = sizeof(struct __gllc_DrawBuffers_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_UseProgram(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_UseProgram_Rec *data;

    data = (struct __gllc_UseProgram_Rec *) PC;
    __gles_UseProgram(gc, data->program);
    return (PC + sizeof(struct __gllc_UseProgram_Rec));
}

const GLubyte *__glle_FogCoordfv(__GLcontext *gc , const GLubyte *PC)
{
GL_ASSERT(0);
return NULL;
}
const GLubyte *__glle_FogCoordd(__GLcontext *gc , const GLubyte *PC)
{
GL_ASSERT(0);
return NULL;
}
const GLubyte *__glle_FogCoorddv(__GLcontext *gc , const GLubyte *PC)
{
GL_ASSERT(0);
return NULL;
}
const GLubyte *__glle_FogCoordPointer(__GLcontext *gc , const GLubyte *PC)
{
GL_ASSERT(0);
return NULL;
}

const GLubyte *__glle_SecondaryColorPointer(__GLcontext *gc , const GLubyte *PC)
{
GL_ASSERT(0);
return NULL;
}
const GLubyte *__glle_GenQueries(__GLcontext *gc , const GLubyte *PC)
{
GL_ASSERT(0);
return NULL;
}
const GLubyte *__glle_DeleteQueries(__GLcontext *gc , const GLubyte *PC)
{
GL_ASSERT(0);
return NULL;
}
const GLubyte *__glle_IsQuery(__GLcontext *gc , const GLubyte *PC)
{
GL_ASSERT(0);
return NULL;
}

const GLubyte *__glle_GetQueryiv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetQueryObjectiv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetQueryObjectuiv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_BindBuffer(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_DeleteBuffers(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GenBuffers(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_IsBuffer(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_BufferData(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_BufferSubData(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetBufferSubData(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_MapBuffer(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_UnmapBuffer(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetBufferParameteriv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetBufferPointerv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }

const GLubyte *__glle_AttachShader(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_BindAttribLocation(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_CompileShader(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_CreateProgram(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_CreateShader(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_DeleteProgram(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_DeleteShader(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_DetachShader(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_DisableVertexAttribArray(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_EnableVertexAttribArray(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetActiveAttrib(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetActiveUniform(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetAttachedShaders(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetAttribLocation(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetProgramiv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetProgramInfoLog(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetShaderiv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetShaderInfoLog(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetShaderSource(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetUniformLocation(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetUniformfv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetUniformiv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetVertexAttribdv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetVertexAttribfv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetVertexAttribiv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetVertexAttribPointerv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_IsProgram(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_IsShader(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_LinkProgram(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_ShaderSource(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }

const GLubyte *__glle_ValidateProgram(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_VertexAttribPointer(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_DeleteLists(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GenLists(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_DeleteObjectARB(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetHandleARB(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetInfoLogARB(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetObjectParameterfvARB(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetObjectParameterivARB(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_FeedbackBuffer(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_SelectBuffer(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_RenderMode(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_Finish(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_Flush(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }

const GLubyte *__glle_EvalCoord1d(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_EvalCoord1f(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_EvalCoord2d(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_EvalCoord2f(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_PixelStoref(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_PixelStorei(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_ReadPixels(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetBooleanv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetClipPlane(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetDoublev(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetError(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetFloatv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetIntegerv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetLightfv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetLightiv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetMapdv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetMapfv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetMapiv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetMaterialfv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetMaterialiv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetPixelMapfv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetPixelMapuiv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetPixelMapusv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetPolygonStipple(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetString(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetTexEnvfv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetTexEnviv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetTexGendv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetTexGenfv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetTexGeniv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetTexImage(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetTexParameterfv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetTexParameteriv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetTexLevelParameterfv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetTexLevelParameteriv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_IsEnabled(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_IsList(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetColorTable(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetColorTableParameterfv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetColorTableParameteriv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_ColorPointer(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_DisableClientState(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_EdgeFlagPointer(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_EnableClientState(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetPointerv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_IndexPointer(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_InterleavedArrays(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_NormalPointer(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_TexCoordPointer(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_VertexPointer(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_AreTexturesResident(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_DeleteTextures(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GenTextures(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_IsTexture(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_PopClientAttrib(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_PushClientAttrib(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_ConvolutionParameterf(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_ConvolutionParameteri(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetConvolutionFilter(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetConvolutionParameterfv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetConvolutionParameteriv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetSeparableFilter(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetHistogram(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetHistogramParameterfv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetHistogramParameteriv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetMinmax(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetMinmaxParameterfv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetMinmaxParameteriv(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_ClientActiveTexture(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }
const GLubyte *__glle_GetCompressedTexImage(__GLcontext *gc , const GLubyte *PC){ GL_ASSERT(0); return NULL; }


#if GL_ARB_vertex_program

const GLubyte *__glle_BindProgramARB(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);

    return PC + sizeof(struct __gllc_BindProgramARB_Rec);
}

const GLubyte *__glle_ProgramEnvParameter4dARB(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return PC + sizeof(struct __gllc_ProgramEnvParameter4dARB_Rec);
}

const GLubyte *__glle_ProgramEnvParameter4dvARB(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return PC + sizeof(struct __gllc_ProgramEnvParameter4dvARB_Rec);
}

const GLubyte *__glle_ProgramEnvParameter4fARB(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return PC + sizeof(struct __gllc_ProgramEnvParameter4fARB_Rec);
}

const GLubyte *__glle_ProgramEnvParameter4fvARB(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return PC + sizeof(struct __gllc_ProgramEnvParameter4fvARB_Rec);
}

const GLubyte *__glle_ProgramLocalParameter4dARB(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return PC + sizeof(struct __gllc_ProgramEnvParameter4dARB_Rec);
}

const GLubyte *__glle_ProgramLocalParameter4dvARB(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return PC + sizeof(struct __gllc_ProgramEnvParameter4dvARB_Rec);
}

const GLubyte *__glle_ProgramLocalParameter4fARB(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return PC + sizeof(struct __gllc_ProgramEnvParameter4fARB_Rec);
}

const GLubyte *__glle_ProgramLocalParameter4fvARB(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return PC + sizeof(struct __gllc_ProgramEnvParameter4fvARB_Rec);
}

#endif

#if GL_ATI_element_array
const GLubyte *__glle_DrawElementArrayATI(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return PC + sizeof(struct __gllc_DrawElementArrayATI_Rec);
}

const GLubyte *__glle_DrawRangeElementArrayATI(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return PC + sizeof(struct __gllc_DrawRangeElementArrayATI_Rec);
}
#endif

#if GL_EXT_stencil_two_side
const GLubyte *__glle_ActiveStencilFaceEXT(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return PC + sizeof(struct __gllc_ActiveStencilFaceEXT_Rec);
}
#endif

#if GL_EXT_texture_integer
const GLubyte * __glle_ClearColorIiEXT(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return PC + sizeof(struct __gllc_ClearColorIiEXT_Rec);
}

const GLubyte * __glle_ClearColorIuiEXT(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return PC + sizeof(struct __gllc_ClearColorIuiEXT_Rec);

}

const GLubyte * __glle_TexParameterIivEXT(__GLcontext *gc, const GLubyte *PC)
{
GLuint size = 0;
/* still not added, to do*/
GL_ASSERT(0)
    return PC + size;
}

const GLubyte * __glle_TexParameterIuivEXT(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size = 0;
/* still not added, to do*/
GL_ASSERT(0);
    return PC + size;
}
#endif

#if GL_EXT_gpu_shader4
const GLubyte * __glle_Uniform1uiEXT(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return PC + sizeof(struct __gllc_Uniform1uiEXT_Rec);

}

const GLubyte * __glle_Uniform2uiEXT(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return PC + sizeof(struct __gllc_Uniform2uiEXT_Rec);
}

const GLubyte * __glle_Uniform3uiEXT(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return PC + sizeof(struct __gllc_Uniform3uiEXT_Rec);
}

const GLubyte * __glle_Uniform4uiEXT(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return PC + sizeof(struct __gllc_Uniform4uiEXT_Rec);
}

const GLubyte * __glle_Uniform1uivEXT(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size = 0;
/* still not added, to do*/
GL_ASSERT(0);
    return PC + size;
}

const GLubyte * __glle_Uniform2uivEXT(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size = 0;
/* still not added, to do*/
GL_ASSERT(0);
    return PC + size;
}

const GLubyte * __glle_Uniform3uivEXT(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size = 0;
/* still not added, to do*/
GL_ASSERT(0);
    return PC + size;
}

const GLubyte * __glle_Uniform4uivEXT(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size = 0;
/* still not added, to do*/
GL_ASSERT(0);
    return PC + size;
}


#endif

#if GL_EXT_geometry_shader4
const GLubyte * __glle_FramebufferTextureEXT(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return PC + sizeof(struct __gllc_FramebufferTextureEXT_Rec);
}

const GLubyte * __glle_FramebufferTextureLayerEXT(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return PC + sizeof(struct __gllc_FramebufferTextureLayerEXT_Rec);
}

const GLubyte * __glle_FramebufferTextureFaceEXT(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return PC + sizeof(struct __gllc_FramebufferTextureFaceEXT_Rec);
}
#endif

#if GL_EXT_draw_buffers2

const GLubyte *__glle_ColorMaskIndexedEXT(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return (PC + sizeof(struct __gllc_ColorMaskIndexedEXT_Rec));

}

const GLubyte *__glle_EnableIndexedEXT(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return (PC + sizeof(struct __gllc_EnableIndexedEXT_Rec));
}

const GLubyte *__glle_DisableIndexedEXT(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return (PC + sizeof(struct __gllc_DisableIndexedEXT_Rec));
}
#endif

#if GL_EXT_gpu_program_parameters
const GLubyte * __glle_ProgramEnvParameters4fvEXT(__GLcontext *gc, const GLubyte *PC)
{
    GLint size = 0;
/* still not added, to do*/
GL_ASSERT(0);
    return PC + size;
}

const GLubyte * __glle_ProgramLocalParameters4fvEXT(__GLcontext *gc, const GLubyte *PC)
{
    GLint size = 0;
/* still not added, to do*/
GL_ASSERT(0);
    return PC + size;
}
#endif

#if GL_ARB_color_buffer_float
const GLubyte *__glle_ClampColorARB(__GLcontext *gc, const GLubyte *PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return (PC + sizeof(struct __gllc_ClampColorARB_Rec));
}
#endif

#if GL_ATI_separate_stencil
const GLubyte* __glle_StencilFuncSeparateATI(__GLcontext *gc, const GLubyte* PC)
{
/* still not added, to do*/
GL_ASSERT(0);
    return (PC + sizeof(struct __gllc_StencilFuncSeparateATI_Rec));
}
#endif
