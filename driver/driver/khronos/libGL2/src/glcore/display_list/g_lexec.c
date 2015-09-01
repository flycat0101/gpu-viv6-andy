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

/*
*** Functions in eval.c
*/
extern GLint __glEvalComputeK(GLenum target);

/*
** Used to pad display list entries to double word boundaries where needed
** (for those few OpenGL commands which take double precision values).
*/
const GLubyte *__glle_Skip(const GLubyte *PC)
{
    return (PC);
}

/*
** These routines execute an error stored in a display list.
*/
const GLubyte *__glle_InvalidValue(const GLubyte *PC)
{
    __glSetError(GL_INVALID_VALUE);
    return PC;
}

const GLubyte *__glle_InvalidEnum(const GLubyte *PC)
{
    __glSetError(GL_INVALID_ENUM);
    return PC;
}

const GLubyte *__glle_InvalidOperation(const GLubyte *PC)
{
    __glSetError(GL_INVALID_OPERATION);
    return PC;
}

const GLubyte *__glle_TableTooLarge(const GLubyte *PC)
{
    __glSetError(GL_TABLE_TOO_LARGE);
    return PC;
}

/************************************************************************/

const GLubyte *__glle_ListBase(const GLubyte *PC)
{
    struct __gllc_ListBase_Rec *data;

    data = (struct __gllc_ListBase_Rec *) PC;
    __glim_ListBase(data->base);
    return PC + sizeof(struct __gllc_ListBase_Rec);
}

const GLubyte *__glle_Begin(const GLubyte *PC)
{
    __GL_SETUP();
    struct __gllc_Begin_Rec *data;

    data = (struct __gllc_Begin_Rec *) PC;
    (*gc->currentImmediateTable->dispatch.Begin)(data->primType);
    return PC + sizeof(struct __gllc_Begin_Rec);
}

const GLubyte *__glle_Color3fv(const GLubyte *PC)
{
    __GL_SETUP();
    struct __gllc_Color3fv_Rec *data;

    data = (struct __gllc_Color3fv_Rec *) PC;
    (*gc->currentImmediateTable->dispatch.Color3fv)(data->v);
    return PC + sizeof(struct __gllc_Color3fv_Rec);
}

const GLubyte *__glle_Color4fv(const GLubyte *PC)
{
    __GL_SETUP();
    struct __gllc_Color4fv_Rec *data;

    data = (struct __gllc_Color4fv_Rec *) PC;
    (*gc->currentImmediateTable->dispatch.Color4fv)(data->v);
    return PC + sizeof(struct __gllc_Color4fv_Rec);
}

const GLubyte *__glle_Color4ubv(const GLubyte *PC)
{
    __GL_SETUP();
    struct __gllc_Color4ubv_Rec *data;

    data = (struct __gllc_Color4ubv_Rec *) PC;
    (*gc->currentImmediateTable->dispatch.Color4ubv)(data->v);
    return PC + sizeof(struct __gllc_Color4ubv_Rec);
}

const GLubyte *__glle_EdgeFlag(const GLubyte *PC)
{
    __GL_SETUP();
    struct __gllc_EdgeFlag_Rec *data;

    data = (struct __gllc_EdgeFlag_Rec *) PC;
    (*gc->currentImmediateTable->dispatch.EdgeFlag)(data->flag);
    return PC + sizeof(struct __gllc_EdgeFlag_Rec);
}

const GLubyte *__glle_End(const GLubyte *PC)
{
    __GL_SETUP();

    (*gc->currentImmediateTable->dispatch.End)();
    return PC;
}

const GLubyte *__glle_Indexf(const GLubyte *PC)
{
    struct __gllc_Indexf_Rec *data;

    data = (struct __gllc_Indexf_Rec *) PC;
    __glim_Indexf(data->c);
    return PC + sizeof(struct __gllc_Indexf_Rec);
}

const GLubyte *__glle_Normal3fv(const GLubyte *PC)
{
    __GL_SETUP();
    struct __gllc_Normal3fv_Rec *data;

    data = (struct __gllc_Normal3fv_Rec *) PC;
    (*gc->currentImmediateTable->dispatch.Normal3fv)(data->v);
    return PC + sizeof(struct __gllc_Normal3fv_Rec);
}

const GLubyte *__glle_RasterPos2fv(const GLubyte *PC)
{
    struct __gllc_RasterPos2fv_Rec *data;

    data = (struct __gllc_RasterPos2fv_Rec *) PC;
    __glim_RasterPos2fv(data->v);
    return PC + sizeof(struct __gllc_RasterPos2fv_Rec);
}

const GLubyte *__glle_RasterPos3fv(const GLubyte *PC)
{
    struct __gllc_RasterPos3fv_Rec *data;

    data = (struct __gllc_RasterPos3fv_Rec *) PC;
    __glim_RasterPos3fv(data->v);
    return PC + sizeof(struct __gllc_RasterPos3fv_Rec);
}

const GLubyte *__glle_RasterPos4fv(const GLubyte *PC)
{
    struct __gllc_RasterPos4fv_Rec *data;

    data = (struct __gllc_RasterPos4fv_Rec *) PC;
    __glim_RasterPos4fv(data->v);
    return PC + sizeof(struct __gllc_RasterPos4fv_Rec);
}

const GLubyte *__glle_Rectf(const GLubyte *PC)
{
    struct __gllc_Rectf_Rec *data;

    data = (struct __gllc_Rectf_Rec *) PC;
    __glim_Rectf(data->x1, data->y1, data->x2, data->y2);
    return PC + sizeof(struct __gllc_Rectf_Rec);
}

const GLubyte *__glle_TexCoord2fv(const GLubyte *PC)
{
    __GL_SETUP();
    struct __gllc_TexCoord2fv_Rec *data;

    data = (struct __gllc_TexCoord2fv_Rec *) PC;
    (*gc->currentImmediateTable->dispatch.TexCoord2fv)(data->v);
    return PC + sizeof(struct __gllc_TexCoord2fv_Rec);
}

const GLubyte *__glle_TexCoord3fv(const GLubyte *PC)
{
    __GL_SETUP();
    struct __gllc_TexCoord3fv_Rec *data;

    data = (struct __gllc_TexCoord3fv_Rec *) PC;
    (*gc->currentImmediateTable->dispatch.TexCoord3fv)(data->v);
    return PC + sizeof(struct __gllc_TexCoord3fv_Rec);
}

const GLubyte *__glle_TexCoord4fv(const GLubyte *PC)
{
    __GL_SETUP();
    struct __gllc_TexCoord4fv_Rec *data;

    data = (struct __gllc_TexCoord4fv_Rec *) PC;
    (*gc->currentImmediateTable->dispatch.TexCoord4fv)(data->v);
    return PC + sizeof(struct __gllc_TexCoord4fv_Rec);
}

const GLubyte *__glle_Vertex2fv(const GLubyte *PC)
{
    __GL_SETUP();
    struct __gllc_Vertex2fv_Rec *data;

    data = (struct __gllc_Vertex2fv_Rec *) PC;
    (*gc->currentImmediateTable->dispatch.Vertex2fv)(data->v);
    return PC + sizeof(struct __gllc_Vertex2fv_Rec);
}

const GLubyte *__glle_Vertex3fv(const GLubyte *PC)
{
    __GL_SETUP();
    struct __gllc_Vertex3fv_Rec *data;

    data = (struct __gllc_Vertex3fv_Rec *) PC;
    (*gc->currentImmediateTable->dispatch.Vertex3fv)(data->v);
    return PC + sizeof(struct __gllc_Vertex3fv_Rec);
}

const GLubyte *__glle_Vertex4fv(const GLubyte *PC)
{
    __GL_SETUP();
    struct __gllc_Vertex4fv_Rec *data;

    data = (struct __gllc_Vertex4fv_Rec *) PC;
    (*gc->currentImmediateTable->dispatch.Vertex4fv)(data->v);
    return PC + sizeof(struct __gllc_Vertex4fv_Rec);
}

const GLubyte *__glle_ClipPlane(const GLubyte *PC)
{
    struct __gllc_ClipPlane_Rec *data;

    data = (struct __gllc_ClipPlane_Rec *) PC;
    __glim_ClipPlane(data->plane, data->equation);
    return PC + sizeof(struct __gllc_ClipPlane_Rec);
}

const GLubyte *__glle_ColorMaterial(const GLubyte *PC)
{
    struct __gllc_ColorMaterial_Rec *data;

    data = (struct __gllc_ColorMaterial_Rec *) PC;
    __glim_ColorMaterial(data->face, data->mode);
    return PC + sizeof(struct __gllc_ColorMaterial_Rec);
}

const GLubyte *__glle_CullFace(const GLubyte *PC)
{
    struct __gllc_CullFace_Rec *data;

    data = (struct __gllc_CullFace_Rec *) PC;
    __glim_CullFace(data->mode);
    return PC + sizeof(struct __gllc_CullFace_Rec);
}

const GLubyte *__glle_Fogfv(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_Fogfv_Rec *data;

    data = (struct __gllc_Fogfv_Rec *) PC;
    __glim_Fogfv(data->pname,
            (GLfloat *) (PC + sizeof(struct __gllc_Fogfv_Rec)));
    arraySize = __GL64PAD(__glFog_size(data->pname) * 4);
    size = sizeof(struct __gllc_Fogfv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_Fogiv(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_Fogiv_Rec *data;

    data = (struct __gllc_Fogiv_Rec *) PC;
    __glim_Fogiv(data->pname,
            (GLint *) (PC + sizeof(struct __gllc_Fogiv_Rec)));
    arraySize = __GL64PAD(__glFog_size(data->pname) * 4);
    size = sizeof(struct __gllc_Fogiv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_FrontFace(const GLubyte *PC)
{
    struct __gllc_FrontFace_Rec *data;

    data = (struct __gllc_FrontFace_Rec *) PC;
    __glim_FrontFace(data->mode);
    return PC + sizeof(struct __gllc_FrontFace_Rec);
}

const GLubyte *__glle_Hint(const GLubyte *PC)
{
    struct __gllc_Hint_Rec *data;

    data = (struct __gllc_Hint_Rec *) PC;
    __glim_Hint(data->target, data->mode);
    return PC + sizeof(struct __gllc_Hint_Rec);
}

const GLubyte *__glle_Lightfv(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_Lightfv_Rec *data;

    data = (struct __gllc_Lightfv_Rec *) PC;
    __glim_Lightfv(data->light, data->pname,
            (GLfloat *) (PC + sizeof(struct __gllc_Lightfv_Rec)));
    arraySize = __GL64PAD(__glLight_size(data->pname) * 4);
    size = sizeof(struct __gllc_Lightfv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_Lightiv(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_Lightiv_Rec *data;

    data = (struct __gllc_Lightiv_Rec *) PC;
    __glim_Lightiv(data->light, data->pname,
            (GLint *) (PC + sizeof(struct __gllc_Lightiv_Rec)));
    arraySize = __GL64PAD(__glLight_size(data->pname) * 4);
    size = sizeof(struct __gllc_Lightiv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_LightModelfv(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_LightModelfv_Rec *data;

    data = (struct __gllc_LightModelfv_Rec *) PC;
    __glim_LightModelfv(data->pname,
            (GLfloat *) (PC + sizeof(struct __gllc_LightModelfv_Rec)));
    arraySize = __GL64PAD(__glLightModel_size(data->pname) * 4);
    size = sizeof(struct __gllc_LightModelfv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_LightModeliv(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_LightModeliv_Rec *data;

    data = (struct __gllc_LightModeliv_Rec *) PC;
    __glim_LightModeliv(data->pname,
            (GLint *) (PC + sizeof(struct __gllc_LightModeliv_Rec)));
    arraySize = __GL64PAD(__glLightModel_size(data->pname) * 4);
    size = sizeof(struct __gllc_LightModeliv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_LineStipple(const GLubyte *PC)
{
    struct __gllc_LineStipple_Rec *data;

    data = (struct __gllc_LineStipple_Rec *) PC;
    __glim_LineStipple(data->factor, data->pattern);
    return PC + sizeof(struct __gllc_LineStipple_Rec);
}

const GLubyte *__glle_LineWidth(const GLubyte *PC)
{
    struct __gllc_LineWidth_Rec *data;

    data = (struct __gllc_LineWidth_Rec *) PC;
    __glim_LineWidth(data->width);
    return PC + sizeof(struct __gllc_LineWidth_Rec);
}

const GLubyte *__glle_Materialfv(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_Materialfv_Rec *data;

    data = (struct __gllc_Materialfv_Rec *) PC;
    __glim_Materialfv(data->face, data->pname,
            (GLfloat *) (PC + sizeof(struct __gllc_Materialfv_Rec)));
    arraySize = __GL64PAD(__glMaterial_size(data->pname) * 4);
    size = sizeof(struct __gllc_Materialfv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_Materialiv(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_Materialiv_Rec *data;

    data = (struct __gllc_Materialiv_Rec *) PC;
    __glim_Materialiv(data->face, data->pname,
            (GLint *) (PC + sizeof(struct __gllc_Materialiv_Rec)));
    arraySize = __GL64PAD(__glMaterial_size(data->pname) * 4);
    size = sizeof(struct __gllc_Materialiv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_PointSize(const GLubyte *PC)
{
    struct __gllc_PointSize_Rec *data;

    data = (struct __gllc_PointSize_Rec *) PC;
    __glim_PointSize(data->size);
    return PC + sizeof(struct __gllc_PointSize_Rec);
}

const GLubyte *__glle_PolygonMode(const GLubyte *PC)
{
    struct __gllc_PolygonMode_Rec *data;

    data = (struct __gllc_PolygonMode_Rec *) PC;
    __glim_PolygonMode(data->face, data->mode);
    return PC + sizeof(struct __gllc_PolygonMode_Rec);
}

const GLubyte *__glle_Scissor(const GLubyte *PC)
{
    struct __gllc_Scissor_Rec *data;

    data = (struct __gllc_Scissor_Rec *) PC;
    __glim_Scissor(data->x, data->y, data->width, data->height);
    return PC + sizeof(struct __gllc_Scissor_Rec);
}

const GLubyte *__glle_ShadeModel(const GLubyte *PC)
{
    struct __gllc_ShadeModel_Rec *data;

    data = (struct __gllc_ShadeModel_Rec *) PC;
    __glim_ShadeModel(data->mode);
    return PC + sizeof(struct __gllc_ShadeModel_Rec);
}

const GLubyte *__glle_TexParameterfv(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_TexParameterfv_Rec *data;

    data = (struct __gllc_TexParameterfv_Rec *) PC;
    __glim_TexParameterfv(data->target, data->pname,
            (GLfloat *) (PC + sizeof(struct __gllc_TexParameterfv_Rec)));
    arraySize = __GL64PAD(__glTexParameter_size(data->pname) * 4);
    size = sizeof(struct __gllc_TexParameterfv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_TexParameteriv(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_TexParameteriv_Rec *data;

    data = (struct __gllc_TexParameteriv_Rec *) PC;
    __glim_TexParameteriv(data->target, data->pname,
            (GLint *) (PC + sizeof(struct __gllc_TexParameteriv_Rec)));
    arraySize = __GL64PAD(__glTexParameter_size(data->pname) * 4);
    size = sizeof(struct __gllc_TexParameteriv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_TexEnvfv(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_TexEnvfv_Rec *data;

    data = (struct __gllc_TexEnvfv_Rec *) PC;
    __glim_TexEnvfv(data->target, data->pname,
            (GLfloat *) (PC + sizeof(struct __gllc_TexEnvfv_Rec)));
    arraySize = __GL64PAD(__glTexEnv_size(data->pname) * 4);
    size = sizeof(struct __gllc_TexEnvfv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_TexEnviv(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_TexEnviv_Rec *data;

    data = (struct __gllc_TexEnviv_Rec *) PC;
    __glim_TexEnviv(data->target, data->pname,
            (GLint *) (PC + sizeof(struct __gllc_TexEnviv_Rec)));
    arraySize = __GL64PAD(__glTexEnv_size(data->pname) * 4);
    size = sizeof(struct __gllc_TexEnviv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_TexGendv(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_TexGendv_Rec *data;

    data = (struct __gllc_TexGendv_Rec *) PC;
    __glim_TexGendv(data->coord, data->pname,
            (GLdouble *) (PC + sizeof(struct __gllc_TexGendv_Rec)));
    arraySize = __GL64PAD(__glTexGen_size(data->pname) * 8);
    size = sizeof(struct __gllc_TexGendv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_TexGenfv(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_TexGenfv_Rec *data;

    data = (struct __gllc_TexGenfv_Rec *) PC;
    __glim_TexGenfv(data->coord, data->pname,
            (GLfloat *) (PC + sizeof(struct __gllc_TexGenfv_Rec)));
    arraySize = __GL64PAD(__glTexGen_size(data->pname) * 4);
    size = sizeof(struct __gllc_TexGenfv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_TexGeniv(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_TexGeniv_Rec *data;

    data = (struct __gllc_TexGeniv_Rec *) PC;
    __glim_TexGeniv(data->coord, data->pname,
            (GLint *) (PC + sizeof(struct __gllc_TexGeniv_Rec)));
    arraySize = __GL64PAD(__glTexGen_size(data->pname) * 4);
    size = sizeof(struct __gllc_TexGeniv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_InitNames(const GLubyte *PC)
{

    __glim_InitNames();
    return PC;
}

const GLubyte *__glle_LoadName(const GLubyte *PC)
{
    struct __gllc_LoadName_Rec *data;

    data = (struct __gllc_LoadName_Rec *) PC;
    __glim_LoadName(data->name);
    return PC + sizeof(struct __gllc_LoadName_Rec);
}

const GLubyte *__glle_PassThrough(const GLubyte *PC)
{
    struct __gllc_PassThrough_Rec *data;

    data = (struct __gllc_PassThrough_Rec *) PC;
    __glim_PassThrough(data->token);
    return PC + sizeof(struct __gllc_PassThrough_Rec);
}

const GLubyte *__glle_PopName(const GLubyte *PC)
{
    __glim_PopName();
    return PC;
}

const GLubyte *__glle_PushName(const GLubyte *PC)
{
    struct __gllc_PushName_Rec *data;

    data = (struct __gllc_PushName_Rec *) PC;
    __glim_PushName(data->name);
    return PC + sizeof(struct __gllc_PushName_Rec);
}

const GLubyte *__glle_DrawBuffer(const GLubyte *PC)
{
    struct __gllc_DrawBuffer_Rec *data;

    data = (struct __gllc_DrawBuffer_Rec *) PC;
    __glim_DrawBuffer(data->mode);
    return PC + sizeof(struct __gllc_DrawBuffer_Rec);
}

const GLubyte *__glle_Clear(const GLubyte *PC)
{
    struct __gllc_Clear_Rec *data;

    data = (struct __gllc_Clear_Rec *) PC;
    __glim_Clear(data->mask);
    return PC + sizeof(struct __gllc_Clear_Rec);
}

const GLubyte *__glle_ClearAccum(const GLubyte *PC)
{
    struct __gllc_ClearAccum_Rec *data;

    data = (struct __gllc_ClearAccum_Rec *) PC;
    __glim_ClearAccum(data->red, data->green, data->blue, data->alpha);
    return PC + sizeof(struct __gllc_ClearAccum_Rec);
}

const GLubyte *__glle_ClearIndex(const GLubyte *PC)
{
    struct __gllc_ClearIndex_Rec *data;

    data = (struct __gllc_ClearIndex_Rec *) PC;
    __glim_ClearIndex(data->c);
    return PC + sizeof(struct __gllc_ClearIndex_Rec);
}

const GLubyte *__glle_ClearColor(const GLubyte *PC)
{
    struct __gllc_ClearColor_Rec *data;

    data = (struct __gllc_ClearColor_Rec *) PC;
    __glim_ClearColor(data->red, data->green, data->blue, data->alpha);
    return PC + sizeof(struct __gllc_ClearColor_Rec);
}

const GLubyte *__glle_ClearStencil(const GLubyte *PC)
{
    struct __gllc_ClearStencil_Rec *data;

    data = (struct __gllc_ClearStencil_Rec *) PC;
    __glim_ClearStencil(data->s);
    return PC + sizeof(struct __gllc_ClearStencil_Rec);
}

const GLubyte *__glle_ClearDepth(const GLubyte *PC)
{
    struct __gllc_ClearDepth_Rec *data;

    data = (struct __gllc_ClearDepth_Rec *) PC;
    __glim_ClearDepth(data->depth);
    return PC + sizeof(struct __gllc_ClearDepth_Rec);
}

const GLubyte *__glle_StencilMask(const GLubyte *PC)
{
    struct __gllc_StencilMask_Rec *data;

    data = (struct __gllc_StencilMask_Rec *) PC;
    __glim_StencilMask(data->mask);
    return PC + sizeof(struct __gllc_StencilMask_Rec);
}

const GLubyte *__glle_StencilMaskSeparate(const GLubyte *PC)
{
    struct __gllc_StencilMaskSeparate_Rec *data;

    data = (struct __gllc_StencilMaskSeparate_Rec *) PC;
    __glim_StencilMaskSeparate(data->face, data->mask);
    return PC + sizeof(struct __gllc_StencilMaskSeparate_Rec);
}

const GLubyte *__glle_ColorMask(const GLubyte *PC)
{
    struct __gllc_ColorMask_Rec *data;

    data = (struct __gllc_ColorMask_Rec *) PC;
    __glim_ColorMask(data->red, data->green, data->blue, data->alpha);
    return PC + sizeof(struct __gllc_ColorMask_Rec);
}

const GLubyte *__glle_DepthMask(const GLubyte *PC)
{
    struct __gllc_DepthMask_Rec *data;

    data = (struct __gllc_DepthMask_Rec *) PC;
    __glim_DepthMask(data->flag);
    return PC + sizeof(struct __gllc_DepthMask_Rec);
}

const GLubyte *__glle_IndexMask(const GLubyte *PC)
{
    struct __gllc_IndexMask_Rec *data;

    data = (struct __gllc_IndexMask_Rec *) PC;
    __glim_IndexMask(data->mask);
    return PC + sizeof(struct __gllc_IndexMask_Rec);
}

const GLubyte *__glle_Accum(const GLubyte *PC)
{
    struct __gllc_Accum_Rec *data;

    data = (struct __gllc_Accum_Rec *) PC;
    __glim_Accum(data->op, data->value);
    return PC + sizeof(struct __gllc_Accum_Rec);
}

const GLubyte *__glle_PopAttrib(const GLubyte *PC)
{

    __glim_PopAttrib();
    return PC;
}

const GLubyte *__glle_PushAttrib(const GLubyte *PC)
{
    struct __gllc_PushAttrib_Rec *data;

    data = (struct __gllc_PushAttrib_Rec *) PC;
    __glim_PushAttrib(data->mask);
    return PC + sizeof(struct __gllc_PushAttrib_Rec);
}

const GLubyte *__glle_MapGrid1d(const GLubyte *PC)
{
    struct __gllc_MapGrid1d_Rec *data;

    data = (struct __gllc_MapGrid1d_Rec *) PC;
    __glim_MapGrid1d(data->un, data->u1, data->u2);
    return PC + sizeof(struct __gllc_MapGrid1d_Rec);
}

const GLubyte *__glle_MapGrid1f(const GLubyte *PC)
{
    struct __gllc_MapGrid1f_Rec *data;

    data = (struct __gllc_MapGrid1f_Rec *) PC;
    __glim_MapGrid1f(data->un, data->u1, data->u2);
    return PC + sizeof(struct __gllc_MapGrid1f_Rec);
}

const GLubyte *__glle_MapGrid2d(const GLubyte *PC)
{
    struct __gllc_MapGrid2d_Rec *data;

    data = (struct __gllc_MapGrid2d_Rec *) PC;
    __glim_MapGrid2d(data->un, data->u1, data->u2, data->vn,
            data->v1, data->v2);
    return PC + sizeof(struct __gllc_MapGrid2d_Rec);
}

const GLubyte *__glle_MapGrid2f(const GLubyte *PC)
{
    struct __gllc_MapGrid2f_Rec *data;

    data = (struct __gllc_MapGrid2f_Rec *) PC;
    __glim_MapGrid2f(data->un, data->u1, data->u2, data->vn,
            data->v1, data->v2);
    return PC + sizeof(struct __gllc_MapGrid2f_Rec);
}

const GLubyte *__glle_EvalCoord1dv(const GLubyte *PC)
{
    struct __gllc_EvalCoord1dv_Rec *data;

    data = (struct __gllc_EvalCoord1dv_Rec *) PC;
    __glim_EvalCoord1dv(data->u);
    return PC + sizeof(struct __gllc_EvalCoord1dv_Rec);
}

const GLubyte *__glle_EvalCoord1fv(const GLubyte *PC)
{
    struct __gllc_EvalCoord1fv_Rec *data;

    data = (struct __gllc_EvalCoord1fv_Rec *) PC;
    __glim_EvalCoord1fv(data->u);
    return PC + sizeof(struct __gllc_EvalCoord1fv_Rec);
}

const GLubyte *__glle_EvalCoord2dv(const GLubyte *PC)
{
    struct __gllc_EvalCoord2dv_Rec *data;

    data = (struct __gllc_EvalCoord2dv_Rec *) PC;
    __glim_EvalCoord2dv(data->u);
    return PC + sizeof(struct __gllc_EvalCoord2dv_Rec);
}

const GLubyte *__glle_EvalCoord2fv(const GLubyte *PC)
{
    struct __gllc_EvalCoord2fv_Rec *data;

    data = (struct __gllc_EvalCoord2fv_Rec *) PC;
    __glim_EvalCoord2fv(data->u);
    return PC + sizeof(struct __gllc_EvalCoord2fv_Rec);
}

const GLubyte *__glle_EvalMesh1(const GLubyte *PC)
{
    struct __gllc_EvalMesh1_Rec *data;

    data = (struct __gllc_EvalMesh1_Rec *) PC;
    __glim_EvalMesh1(data->mode, data->i1, data->i2);
    return PC + sizeof(struct __gllc_EvalMesh1_Rec);
}

const GLubyte *__glle_EvalPoint1(const GLubyte *PC)
{
    struct __gllc_EvalPoint1_Rec *data;

    data = (struct __gllc_EvalPoint1_Rec *) PC;
    __glim_EvalPoint1(data->i);
    return PC + sizeof(struct __gllc_EvalPoint1_Rec);
}

const GLubyte *__glle_EvalMesh2(const GLubyte *PC)
{
    struct __gllc_EvalMesh2_Rec *data;

    data = (struct __gllc_EvalMesh2_Rec *) PC;
    __glim_EvalMesh2(data->mode, data->i1, data->i2, data->j1,
            data->j2);
    return PC + sizeof(struct __gllc_EvalMesh2_Rec);
}

const GLubyte *__glle_EvalPoint2(const GLubyte *PC)
{
    struct __gllc_EvalPoint2_Rec *data;

    data = (struct __gllc_EvalPoint2_Rec *) PC;
    __glim_EvalPoint2(data->i, data->j);
    return PC + sizeof(struct __gllc_EvalPoint2_Rec);
}

const GLubyte *__glle_AlphaFunc(const GLubyte *PC)
{
    struct __gllc_AlphaFunc_Rec *data;

    data = (struct __gllc_AlphaFunc_Rec *) PC;
    __glim_AlphaFunc(data->func, data->ref);
    return PC + sizeof(struct __gllc_AlphaFunc_Rec);
}

const GLubyte *__glle_BlendColor(const GLubyte *PC)
{
    struct __gllc_BlendColor_Rec *data;

    data = (struct __gllc_BlendColor_Rec *) PC;
    __glim_BlendColor(data->r, data->g, data->b, data->a);
    return PC + sizeof(struct __gllc_BlendColor_Rec);
}

const GLubyte *__glle_BlendFunc(const GLubyte *PC)
{
    struct __gllc_BlendFunc_Rec *data;

    data = (struct __gllc_BlendFunc_Rec *) PC;
    __glim_BlendFunc(data->sfactor, data->dfactor);
    return PC + sizeof(struct __gllc_BlendFunc_Rec);
}

const GLubyte *__glle_BlendFuncSeparate(const GLubyte *PC)
{
    struct __gllc_BlendFuncSeparate_Rec *data;

    data = (struct __gllc_BlendFuncSeparate_Rec *) PC;
    __glim_BlendFuncSeparate(data->sfactorRGB, data->dfactorRGB, data->sfactorAlpha, data->dfactorAlpha);
    return PC + sizeof(struct __gllc_BlendFuncSeparate_Rec);
}

const GLubyte *__glle_BlendEquation(const GLubyte *PC)
{
    struct __gllc_BlendEquation_Rec *data;

    data = (struct __gllc_BlendEquation_Rec *) PC;
    __glim_BlendEquation(data->mode);
    return PC + sizeof(struct __gllc_BlendEquation_Rec);
}

const GLubyte *__glle_BlendEquationSeparate(const GLubyte *PC)
{
    struct __gllc_BlendEquationSeparate_Rec *data;

    data = (struct __gllc_BlendEquationSeparate_Rec *) PC;
    __glim_BlendEquationSeparate(data->modeRGB, data->modeAlpha);
    return PC + sizeof(struct __gllc_BlendEquationSeparate_Rec);
}

const GLubyte *__glle_LogicOp(const GLubyte *PC)
{
    struct __gllc_LogicOp_Rec *data;

    data = (struct __gllc_LogicOp_Rec *) PC;
    __glim_LogicOp(data->opcode);
    return PC + sizeof(struct __gllc_LogicOp_Rec);
}

const GLubyte *__glle_StencilFunc(const GLubyte *PC)
{
    struct __gllc_StencilFunc_Rec *data;

    data = (struct __gllc_StencilFunc_Rec *) PC;
    __glim_StencilFunc(data->func, data->ref, data->mask);
    return PC + sizeof(struct __gllc_StencilFunc_Rec);
}

const GLubyte *__glle_StencilFuncSeparate(const GLubyte *PC)
{
    struct __gllc_StencilFuncSeparate_Rec *data;

    data = (struct __gllc_StencilFuncSeparate_Rec *) PC;
    __glim_StencilFuncSeparate(data->face, data->func, data->ref, data->mask);
    return PC + sizeof(struct __gllc_StencilFuncSeparate_Rec);
}

const GLubyte *__glle_StencilOp(const GLubyte *PC)
{
    struct __gllc_StencilOp_Rec *data;

    data = (struct __gllc_StencilOp_Rec *) PC;
    __glim_StencilOp(data->fail, data->zfail, data->zpass);
    return PC + sizeof(struct __gllc_StencilOp_Rec);
}

const GLubyte *__glle_StencilOpSeparate(const GLubyte *PC)
{
    struct __gllc_StencilOpSeparate_Rec *data;

    data = (struct __gllc_StencilOpSeparate_Rec *) PC;
    __glim_StencilOpSeparate(data->face, data->fail, data->zfail, data->zpass);
    return PC + sizeof(struct __gllc_StencilOpSeparate_Rec);
}

const GLubyte *__glle_DepthFunc(const GLubyte *PC)
{
    struct __gllc_DepthFunc_Rec *data;

    data = (struct __gllc_DepthFunc_Rec *) PC;
    __glim_DepthFunc(data->func);
    return PC + sizeof(struct __gllc_DepthFunc_Rec);
}

const GLubyte *__glle_PixelZoom(const GLubyte *PC)
{
    struct __gllc_PixelZoom_Rec *data;

    data = (struct __gllc_PixelZoom_Rec *) PC;
    __glim_PixelZoom(data->xfactor, data->yfactor);
    return PC + sizeof(struct __gllc_PixelZoom_Rec);
}

const GLubyte *__glle_PixelTransferf(const GLubyte *PC)
{
    struct __gllc_PixelTransferf_Rec *data;

    data = (struct __gllc_PixelTransferf_Rec *) PC;
    __glim_PixelTransferf(data->pname, data->param);
    return PC + sizeof(struct __gllc_PixelTransferf_Rec);
}

const GLubyte *__glle_PixelTransferi(const GLubyte *PC)
{
    struct __gllc_PixelTransferi_Rec *data;

    data = (struct __gllc_PixelTransferi_Rec *) PC;
    __glim_PixelTransferi(data->pname, data->param);
    return PC + sizeof(struct __gllc_PixelTransferi_Rec);
}

const GLubyte *__glle_PixelMapfv(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_PixelMapfv_Rec *data;

    data = (struct __gllc_PixelMapfv_Rec *) PC;
    __glim_PixelMapfv(data->map, data->mapsize,
            (GLfloat *) (PC + sizeof(struct __gllc_PixelMapfv_Rec)));
    arraySize = __GL64PAD(data->mapsize * 4);
    size = sizeof(struct __gllc_PixelMapfv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_PixelMapuiv(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_PixelMapuiv_Rec *data;

    data = (struct __gllc_PixelMapuiv_Rec *) PC;
    __glim_PixelMapuiv(data->map, data->mapsize,
            (GLuint *) (PC + sizeof(struct __gllc_PixelMapuiv_Rec)));
    arraySize = __GL64PAD(data->mapsize * 4);
    size = sizeof(struct __gllc_PixelMapuiv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_PixelMapusv(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_PixelMapusv_Rec *data;

    data = (struct __gllc_PixelMapusv_Rec *) PC;
    __glim_PixelMapusv(data->map, data->mapsize,
            (GLushort *) (PC + sizeof(struct __gllc_PixelMapusv_Rec)));
    arraySize = __GL_PAD(data->mapsize * 2);
    size = sizeof(struct __gllc_PixelMapusv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_ReadBuffer(const GLubyte *PC)
{
    struct __gllc_ReadBuffer_Rec *data;

    data = (struct __gllc_ReadBuffer_Rec *) PC;
    __glim_ReadBuffer(data->mode);
    return PC + sizeof(struct __gllc_ReadBuffer_Rec);
}

const GLubyte *__glle_CopyPixels(const GLubyte *PC)
{
    struct __gllc_CopyPixels_Rec *data;

    data = (struct __gllc_CopyPixels_Rec *) PC;
    __glim_CopyPixels(data->x, data->y, data->width, data->height,
            data->type);
    return PC + sizeof(struct __gllc_CopyPixels_Rec);
}

const GLubyte *__glle_DepthRange(const GLubyte *PC)
{
    struct __gllc_DepthRange_Rec *data;

    data = (struct __gllc_DepthRange_Rec *) PC;
    __glim_DepthRange(data->zNear, data->zFar);
    return PC + sizeof(struct __gllc_DepthRange_Rec);
}

#if GL_EXT_depth_bounds_test
const GLubyte * __glle_DepthBoundsEXT(const GLubyte *PC)
{
    struct __gllc_DepthBoundTest_Rec *data;

    data = (struct __gllc_DepthBoundTest_Rec *) PC;
    __glim_DepthBoundsEXT(data->zMin, data->zMax);
    return PC + sizeof(struct __gllc_DepthBoundTest_Rec);
}
#endif

const GLubyte *__glle_Frustum(const GLubyte *PC)
{
    struct __gllc_Frustum_Rec *data;

    data = (struct __gllc_Frustum_Rec *) PC;
    __glim_Frustum(data->left, data->right, data->bottom, data->top,
            data->zNear, data->zFar);
    return PC + sizeof(struct __gllc_Frustum_Rec);
}

const GLubyte *__glle_LoadIdentity(const GLubyte *PC)
{

    __glim_LoadIdentity();
    return PC;
}

const GLubyte *__glle_LoadMatrixf(const GLubyte *PC)
{
    struct __gllc_LoadMatrixf_Rec *data;

    data = (struct __gllc_LoadMatrixf_Rec *) PC;
    __glim_LoadMatrixf(data->m);
    return PC + sizeof(struct __gllc_LoadMatrixf_Rec);
}

const GLubyte *__glle_LoadMatrixd(const GLubyte *PC)
{
    struct __gllc_LoadMatrixd_Rec *data;

    data = (struct __gllc_LoadMatrixd_Rec *) PC;
    __glim_LoadMatrixd(data->m);
    return PC + sizeof(struct __gllc_LoadMatrixd_Rec);
}

const GLubyte *__glle_MatrixMode(const GLubyte *PC)
{
    struct __gllc_MatrixMode_Rec *data;

    data = (struct __gllc_MatrixMode_Rec *) PC;
    __glim_MatrixMode(data->mode);
    return PC + sizeof(struct __gllc_MatrixMode_Rec);
}

const GLubyte *__glle_MultMatrixf(const GLubyte *PC)
{
    struct __gllc_MultMatrixf_Rec *data;

    data = (struct __gllc_MultMatrixf_Rec *) PC;
    __glim_MultMatrixf(data->m);
    return PC + sizeof(struct __gllc_MultMatrixf_Rec);
}

const GLubyte *__glle_MultMatrixd(const GLubyte *PC)
{
    struct __gllc_MultMatrixd_Rec *data;

    data = (struct __gllc_MultMatrixd_Rec *) PC;
    __glim_MultMatrixd(data->m);
    return PC + sizeof(struct __gllc_MultMatrixd_Rec);
}

const GLubyte *__glle_LoadTransposeMatrixf(const GLubyte *PC)
{
    struct __gllc_LoadMatrixf_Rec *data;

    data = (struct __gllc_LoadMatrixf_Rec *) PC;
    __glim_LoadTransposeMatrixf(data->m);
    return PC + sizeof(struct __gllc_LoadMatrixf_Rec);
}

const GLubyte *__glle_LoadTransposeMatrixd(const GLubyte *PC)
{
    struct __gllc_LoadMatrixd_Rec *data;

    data = (struct __gllc_LoadMatrixd_Rec *) PC;
    __glim_LoadTransposeMatrixd(data->m);
    return PC + sizeof(struct __gllc_LoadMatrixd_Rec);
}

const GLubyte *__glle_MultTransposeMatrixf(const GLubyte *PC)
{
    struct __gllc_MultMatrixf_Rec *data;

    data = (struct __gllc_MultMatrixf_Rec *) PC;
    __glim_MultTransposeMatrixf(data->m);
    return PC + sizeof(struct __gllc_MultMatrixf_Rec);
}

const GLubyte *__glle_MultTransposeMatrixd(const GLubyte *PC)
{
    struct __gllc_MultMatrixd_Rec *data;

    data = (struct __gllc_MultMatrixd_Rec *) PC;
    __glim_MultTransposeMatrixd(data->m);
    return PC + sizeof(struct __gllc_MultMatrixd_Rec);
}

const GLubyte *__glle_Ortho(const GLubyte *PC)
{
    struct __gllc_Ortho_Rec *data;

    data = (struct __gllc_Ortho_Rec *) PC;
    __glim_Ortho(data->left, data->right, data->bottom, data->top,
            data->zNear, data->zFar);
    return PC + sizeof(struct __gllc_Ortho_Rec);
}

const GLubyte *__glle_PopMatrix(const GLubyte *PC)
{

    __glim_PopMatrix();
    return PC;
}

const GLubyte *__glle_PushMatrix(const GLubyte *PC)
{

    __glim_PushMatrix();
    return PC;
}

const GLubyte *__glle_Rotated(const GLubyte *PC)
{
    struct __gllc_Rotated_Rec *data;

    data = (struct __gllc_Rotated_Rec *) PC;
    __glim_Rotated(data->angle, data->x, data->y, data->z);
    return PC + sizeof(struct __gllc_Rotated_Rec);
}

const GLubyte *__glle_Rotatef(const GLubyte *PC)
{
    struct __gllc_Rotatef_Rec *data;

    data = (struct __gllc_Rotatef_Rec *) PC;
    __glim_Rotatef(data->angle, data->x, data->y, data->z);
    return PC + sizeof(struct __gllc_Rotatef_Rec);
}

const GLubyte *__glle_Scaled(const GLubyte *PC)
{
    struct __gllc_Scaled_Rec *data;

    data = (struct __gllc_Scaled_Rec *) PC;
    __glim_Scaled(data->x, data->y, data->z);
    return PC + sizeof(struct __gllc_Scaled_Rec);
}

const GLubyte *__glle_Scalef(const GLubyte *PC)
{
    struct __gllc_Scalef_Rec *data;

    data = (struct __gllc_Scalef_Rec *) PC;
    __glim_Scalef(data->x, data->y, data->z);
    return PC + sizeof(struct __gllc_Scalef_Rec);
}

const GLubyte *__glle_Translated(const GLubyte *PC)
{
    struct __gllc_Translated_Rec *data;

    data = (struct __gllc_Translated_Rec *) PC;
    __glim_Translated(data->x, data->y, data->z);
    return PC + sizeof(struct __gllc_Translated_Rec);
}

const GLubyte *__glle_Translatef(const GLubyte *PC)
{
    struct __gllc_Translatef_Rec *data;

    data = (struct __gllc_Translatef_Rec *) PC;
    __glim_Translatef(data->x, data->y, data->z);
    return PC + sizeof(struct __gllc_Translatef_Rec);
}

const GLubyte *__glle_Viewport(const GLubyte *PC)
{
    struct __gllc_Viewport_Rec *data;

    data = (struct __gllc_Viewport_Rec *) PC;
    __glim_Viewport(data->x, data->y, data->width, data->height);
    return PC + sizeof(struct __gllc_Viewport_Rec);
}

const GLubyte *__glle_PolygonOffset(const GLubyte *PC)
{
    struct __gllc_PolygonOffset_Rec *data;

    data = (struct __gllc_PolygonOffset_Rec *) PC;
    __glim_PolygonOffset(data->factor, data->units);
    return PC + sizeof(struct __gllc_PolygonOffset_Rec);
}

const GLubyte *__glle_CopyTexImage1D(const GLubyte *PC)
{
    struct __gllc_CopyTexImage1D_Rec *data;

    data = (struct __gllc_CopyTexImage1D_Rec *) PC;
    __glim_CopyTexImage1D(data->target, data->level, data->internalformat, data->x,
            data->y, data->width, data->border);
    return PC + sizeof(struct __gllc_CopyTexImage1D_Rec);
}

const GLubyte *__glle_CopyTexImage2D(const GLubyte *PC)
{
    struct __gllc_CopyTexImage2D_Rec *data;

    data = (struct __gllc_CopyTexImage2D_Rec *) PC;
    __glim_CopyTexImage2D(data->target, data->level, data->internalformat, data->x,
            data->y, data->width, data->height, data->border);
    return PC + sizeof(struct __gllc_CopyTexImage2D_Rec);
}

const GLubyte *__glle_CopyTexSubImage1D(const GLubyte *PC)
{
    struct __gllc_CopyTexSubImage1D_Rec *data;

    data = (struct __gllc_CopyTexSubImage1D_Rec *) PC;
    __glim_CopyTexSubImage1D(data->target, data->level, data->xoffset, data->x,
            data->y, data->width);
    return PC + sizeof(struct __gllc_CopyTexSubImage1D_Rec);
}

const GLubyte *__glle_CopyTexSubImage2D(const GLubyte *PC)
{
    struct __gllc_CopyTexSubImage2D_Rec *data;

    data = (struct __gllc_CopyTexSubImage2D_Rec *) PC;
    __glim_CopyTexSubImage2D(data->target, data->level, data->xoffset, data->yoffset,
            data->x, data->y, data->width, data->height);
    return PC + sizeof(struct __gllc_CopyTexSubImage2D_Rec);
}

const GLubyte *__glle_BindTexture(const GLubyte *PC)
{
    struct __gllc_BindTexture_Rec *data;

    data = (struct __gllc_BindTexture_Rec *) PC;
    __glim_BindTexture(data->target, data->texture);
    return PC + sizeof(struct __gllc_BindTexture_Rec);
}

const GLubyte *__glle_PrioritizeTextures(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize0;
    GLuint arraySize1;
    struct __gllc_PrioritizeTextures_Rec *data;

    data = (struct __gllc_PrioritizeTextures_Rec *) PC;
    arraySize0 = __GL64PAD(data->n * 4);
    arraySize1 = __GL64PAD(data->n * 4);
    size = sizeof(struct __gllc_PrioritizeTextures_Rec) + arraySize0 + arraySize1;
    __glim_PrioritizeTextures(data->n,
            (GLuint *) (PC + sizeof(struct __gllc_PrioritizeTextures_Rec)),
            (GLclampf *) (PC + sizeof(struct __gllc_PrioritizeTextures_Rec) + arraySize0));
    return PC + size;
}

const GLubyte *__glle_ActiveTexture(const GLubyte *PC)
{
    struct __gllc_ActiveTexture_Rec *data;

    data = (struct __gllc_ActiveTexture_Rec *) PC;
    __glim_ActiveTexture(data->texture);
    return PC + sizeof( struct __gllc_ActiveTexture_Rec );
}

const GLubyte *__glle_MultiTexCoord2fv(const GLubyte *PC)
{
    __GL_SETUP();
    struct __gllc_MultiTexCoord2fv_Rec *data;

    data = (struct __gllc_MultiTexCoord2fv_Rec *) PC;
    (*gc->currentImmediateTable->dispatch.MultiTexCoord2fv)(data->texture, data->v);
    return PC + sizeof(struct __gllc_MultiTexCoord2fv_Rec);
}

const GLubyte *__glle_MultiTexCoord3fv(const GLubyte *PC)
{
    __GL_SETUP();
    struct __gllc_MultiTexCoord3fv_Rec *data;

    data = (struct __gllc_MultiTexCoord3fv_Rec *) PC;
    (*gc->currentImmediateTable->dispatch.MultiTexCoord3fv)(data->texture, data->v);
    return PC + sizeof(struct __gllc_MultiTexCoord3fv_Rec);
}

const GLubyte *__glle_MultiTexCoord4fv(const GLubyte *PC)
{
    __GL_SETUP();
    struct __gllc_MultiTexCoord4fv_Rec *data;

    data = (struct __gllc_MultiTexCoord4fv_Rec *) PC;
    (*gc->currentImmediateTable->dispatch.MultiTexCoord4fv)(data->texture, data->v);
    return PC + sizeof(struct __gllc_MultiTexCoord4fv_Rec);
}

const GLubyte *__glle_FogCoordf(const GLubyte *PC)
{
    __GL_SETUP();
    struct __gllc_FogCoordf_Rec *data;
    data = (struct __gllc_FogCoordf_Rec *)PC;
    (*gc->currentImmediateTable->dispatch.FogCoordf)(data->coord);
    return PC + sizeof(struct __gllc_FogCoordf_Rec);
}

const GLubyte * __glle_ColorTableParameteriv(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_ColorTableParameteriv_Rec *data;

    data = (struct __gllc_ColorTableParameteriv_Rec *) PC;
    __glim_ColorTableParameteriv(data->target, data->pname,
            (GLint *) (PC + sizeof(struct __gllc_ColorTableParameteriv_Rec)));
    arraySize = __GL64PAD(__glColorTableParameter_size(data->pname) * 4);
    size = sizeof(struct __gllc_ColorTableParameteriv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_ColorTableParameterfv(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_ColorTableParameterfv_Rec *data;

    data = (struct __gllc_ColorTableParameterfv_Rec *) PC;
    __glim_ColorTableParameterfv(data->target, data->pname,
            (GLfloat *) (PC + sizeof(struct __gllc_ColorTableParameterfv_Rec)));
    arraySize = __GL64PAD(__glColorTableParameter_size(data->pname) * 4);
    size = sizeof(struct __gllc_ColorTableParameterfv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_ConvolutionParameteriv(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_ConvolutionParameteriv_Rec *data;

    data = (struct __gllc_ConvolutionParameteriv_Rec *) PC;
    __glim_ConvolutionParameteriv(data->target, data->pname,
            (GLint *) (PC + sizeof(struct __gllc_ConvolutionParameteriv_Rec)));
    arraySize = __GL64PAD(__glConvolutionParameter_size(data->pname) * 4);
    size = sizeof(struct __gllc_ConvolutionParameteriv_Rec) + arraySize;
    return PC + size;
}

const GLubyte *__glle_ConvolutionParameterfv(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_ConvolutionParameterfv_Rec *data;

    data = (struct __gllc_ConvolutionParameterfv_Rec *) PC;
    __glim_ConvolutionParameterfv(data->target, data->pname,
            (GLfloat *) (PC + sizeof(struct __gllc_ConvolutionParameterfv_Rec)));
    arraySize = __GL64PAD(__glConvolutionParameter_size(data->pname) * 4);
    size = sizeof(struct __gllc_ConvolutionParameterfv_Rec) + arraySize;

    return PC + size;
}

const GLubyte *__glle_VertexAttrib4fv(const GLubyte *PC)
{
    __GL_SETUP();
    struct __gllc_VertexAttrib4fv_Rec *data;

    data = (struct __gllc_VertexAttrib4fv_Rec *)PC;
    (*gc->currentImmediateTable->dispatch.VertexAttrib4fv)(data->index, data->v);

    return PC + sizeof(struct __gllc_VertexAttrib4fv_Rec);
}

const GLubyte *__glle_Bitmap(const GLubyte *PC)
{
    struct __gllc_Bitmap_Rec *data;

    data = (struct __gllc_Bitmap_Rec *)PC;
    __glim_Bitmap(data->width, data->height, data->xorig, data->yorig, data->xmove, data->ymove,
                    (const GLubyte *)(PC + sizeof(struct __gllc_Bitmap_Rec)));
    return (PC + sizeof(struct __gllc_Bitmap_Rec) + data->imageSize);
}

const GLubyte *__glle_PolygonStipple(const GLubyte *PC)
{
    __glim_PolygonStipple(PC);

    return (PC + __glImageSize(32, 32, GL_COLOR_INDEX, GL_BITMAP));
}

const GLubyte *__glle_Map1f(const GLubyte *PC)
{
    const struct __gllc_Map1f_Rec *map1data;
    GLint k;

    map1data = (const struct __gllc_Map1f_Rec *) PC;
    k = __glEvalComputeK(map1data->target);

    /* Stride of "k" matches internal stride */
    __glim_Map1f(map1data->target, map1data->u1, map1data->u2,
            k, map1data->order, (const GLfloat *)(PC + sizeof(*map1data)));

    return (PC + sizeof(*map1data) +
           __glMap1_size(k, map1data->order) * sizeof(GLfloat));
}

const GLubyte *__glle_Map1d(const GLubyte *PC)
{
    const struct __gllc_Map1f_Rec *map1data;
    GLint k;

    map1data = (const struct __gllc_Map1f_Rec *) PC;
    k = __glEvalComputeK(map1data->target);

    /* Stride of "k" matches internal stride */
    __glim_Map1f(map1data->target, map1data->u1, map1data->u2,
            k, map1data->order, (const GLfloat *)(PC + sizeof(*map1data)));

    return (PC + sizeof(*map1data) +
           __glMap1_size(k, map1data->order) * sizeof(GLfloat));
}

const GLubyte *__glle_Map2f(const GLubyte *PC)
{
    const struct __gllc_Map2f_Rec *map2data;
    GLint k;

    map2data = (const struct __gllc_Map2f_Rec *) PC;
    k = __glEvalComputeK(map2data->target);

    /* Stride of "k" and "k * vorder" matches internal strides */
    __glim_Map2f(map2data->target,
            map2data->u1, map2data->u2, k * map2data->vorder, map2data->uorder,
            map2data->v1, map2data->v2, k, map2data->vorder,
            (const GLfloat *)(PC + sizeof(*map2data)));

    return (PC + sizeof(*map2data) +
           __glMap2_size(k, map2data->uorder, map2data->vorder) *
           sizeof(GLfloat));
}

const GLubyte *__glle_Map2d(const GLubyte *PC)
{
    const struct __gllc_Map2f_Rec *map2data;
    GLint k;

    map2data = (const struct __gllc_Map2f_Rec *) PC;
    k = __glEvalComputeK(map2data->target);

    /* Stride of "k" and "k * vorder" matches internal strides */
    __glim_Map2f(map2data->target,
            map2data->u1, map2data->u2, k * map2data->vorder, map2data->uorder,
            map2data->v1, map2data->v2, k, map2data->vorder,
            (const GLfloat *)(PC + sizeof(*map2data)));

    return (PC + sizeof(*map2data) +
           __glMap2_size(k, map2data->uorder, map2data->vorder) *
           sizeof(GLfloat));
}

const GLubyte *__glle_DrawPixels(const GLubyte *PC)
{
    const struct __gllc_DrawPixels_Rec *data;

    data = (const struct __gllc_DrawPixels_Rec *) PC;

    __glim_DrawPixels(
            data->width, data->height, data->format, data->type,
            (const GLubyte *)(PC + sizeof(struct __gllc_DrawPixels_Rec)));

    return (PC + sizeof(struct __gllc_DrawPixels_Rec) + __GL_PAD(data->imageSize));
}

const GLubyte *__glle_TexImage1D(const GLubyte *PC)
{
    const struct __gllc_TexImage1D_Rec *data;

    data = (const struct __gllc_TexImage1D_Rec *) PC;

    __glim_TexImage1D(
            data->target, data->level, data->components,
            data->width, data->border, data->format, data->type,
            (const GLubyte *)(PC + sizeof(struct __gllc_TexImage1D_Rec)));

    return (PC + sizeof(struct __gllc_TexImage1D_Rec) + __GL_PAD(data->imageSize));
}

const GLubyte *__glle_TexImage2D(const GLubyte *PC)
{
    const struct __gllc_TexImage2D_Rec *data;

    data = (const struct __gllc_TexImage2D_Rec *) PC;

    __glim_TexImage2D(
            data->target, data->level, data->components,
            data->width, data->height, data->border, data->format, data->type,
            (const GLubyte *)(PC + sizeof(struct __gllc_TexImage2D_Rec)));

    return (PC + sizeof(struct __gllc_TexSubImage2D_Rec) + __GL_PAD(data->imageSize));
}

const GLubyte *__glle_TexSubImage1D(const GLubyte *PC)
{
    const struct __gllc_TexSubImage1D_Rec *data;

    data = (const struct __gllc_TexSubImage1D_Rec *) PC;

    __glim_TexSubImage1D(
            data->target, data->level, data->xoffset,
            data->width, data->format, data->type,
            (const GLubyte *)(PC + sizeof(struct __gllc_TexSubImage1D_Rec)));

    return (PC + sizeof(struct __gllc_TexSubImage1D_Rec) + __GL_PAD(data->imageSize));
}

const GLubyte *__glle_TexSubImage2D(const GLubyte *PC)
{
    const struct __gllc_TexSubImage2D_Rec *data;

    data = (const struct __gllc_TexSubImage2D_Rec *) PC;

    __glim_TexSubImage2D(
            data->target, data->level, data->xoffset, data->yoffset,
            data->width, data->height, data->format, data->type,
            (const GLubyte *)(PC + sizeof(struct __gllc_TexSubImage2D_Rec)));

    return (PC + sizeof(struct __gllc_TexSubImage2D_Rec) + __GL_PAD(data->imageSize));
}

const GLubyte *__glle_Disable(const GLubyte *PC)
{
    struct __gllc_Disable_Rec *data;

    data = (struct __gllc_Disable_Rec *) PC;
    __glim_Disable(data->cap);
    return (PC + sizeof(struct __gllc_Disable_Rec));
}

const GLubyte *__glle_Enable(const GLubyte *PC)
{
    struct __gllc_Enable_Rec *data;

    data = (struct __gllc_Enable_Rec *) PC;
    __glim_Enable(data->cap);
    return (PC + sizeof(struct __gllc_Enable_Rec));
}

const GLubyte *__glle_SampleCoverage(const GLubyte *PC)
{
    struct __gllc_SampleCoverage_Rec *data;

    data = (struct __gllc_SampleCoverage_Rec *) PC;
    __glim_SampleCoverage(data->v, data->invert);
    return PC + sizeof(struct __gllc_SampleCoverage_Rec);
}

const GLubyte *__glle_CompressedTexImage1D(const GLubyte *PC)
{
    return (PC);
}

const GLubyte *__glle_CompressedTexImage2D(const GLubyte *PC)
{
    const struct __gllc_CompressedTexImage2D_Rec *data;

    data = (const struct __gllc_CompressedTexImage2D_Rec *) PC;

    __glim_CompressedTexImage2D(
            data->target, data->lod, data->components,
            data->width, data->height, data->border, data->imageSize,
            (const GLubyte *)(PC + sizeof(struct __gllc_CompressedTexImage2D_Rec)));

    return (PC + sizeof(struct __gllc_CompressedTexImage2D_Rec) + data->imageSize);
}

const GLubyte *__glle_CompressedTexImage3D(const GLubyte *PC)
{
    return (PC);
}

const GLubyte *__glle_CompressedTexSubImage1D(const GLubyte *PC)
{
    return (PC);
}

const GLubyte *__glle_CompressedTexSubImage2D(const GLubyte *PC)
{
    const struct __gllc_CompressedTexSubImage2D_Rec *data;

    data = (const struct __gllc_CompressedTexSubImage2D_Rec *) PC;

    __glim_CompressedTexSubImage2D(
            data->target, data->lod, data->xoffset, data->yoffset,
            data->width, data->height, data->format, data->imageSize,
            (const GLubyte *)(PC + sizeof(struct __gllc_CompressedTexSubImage2D_Rec)));

    return (PC + sizeof(struct __gllc_CompressedTexSubImage2D_Rec) + data->imageSize);
}

const GLubyte *__glle_CompressedTexSubImage3D(const GLubyte *PC)
{
    return (PC);
}

const GLubyte *__glle_ColorTable(const GLubyte *PC)
{
    const struct __gllc_ColorTable_Rec *data;

    data = (const struct __gllc_ColorTable_Rec *) PC;

    __glim_ColorTable(data->target, data->internalformat, data->width, data->format, data->type,
            (const GLvoid *) (PC + sizeof(struct __gllc_ColorTable_Rec)));

    return PC + sizeof(struct __gllc_ColorTable_Rec) + __GL_PAD(data->imageSize);
}

const GLubyte *__glle_ColorSubTable(const GLubyte *PC)
{
    const struct __gllc_ColorSubTable_Rec *data;

    data = (const struct __gllc_ColorSubTable_Rec *) PC;

    __glim_ColorSubTable(data->target, data->start, data->count, data->format, data->type,
            (const GLvoid *) (PC + sizeof(struct __gllc_ColorSubTable_Rec)));

    return PC + sizeof(struct __gllc_ColorSubTable_Rec) + __GL_PAD(data->imageSize);
}

const GLubyte *__glle_CopyColorTable(const GLubyte *PC)
{
    struct __gllc_CopyColorTable_Rec *data;

    data = (struct __gllc_CopyColorTable_Rec *) PC;
    __glim_CopyColorTable(data->target, data->internalformat, data->x, data->y, data->width);
    return PC + sizeof(struct __gllc_CopyColorTable_Rec);
}

const GLubyte *__glle_CopyColorSubTable(const GLubyte *PC)
{
    struct __gllc_CopyColorSubTable_Rec *data;

    data = (struct __gllc_CopyColorSubTable_Rec *) PC;
    __glim_CopyColorSubTable(data->target, data->start, data->x, data->y, data->width);
    return PC + sizeof(struct __gllc_CopyColorSubTable_Rec);
}

const GLubyte *__glle_ConvolutionFilter1D(const GLubyte *PC)
{
    const struct __gllc_ConvolutionFilter1D_Rec *data;

    data = (const struct __gllc_ConvolutionFilter1D_Rec *) PC;
    __glim_ConvolutionFilter1D(
            data->target, data->internalformat, data->width, data->format, data->type,
            (const GLubyte *)(PC + sizeof(struct __gllc_ConvolutionFilter1D_Rec)));
    return PC + sizeof(struct __gllc_ConvolutionFilter1D_Rec) + __GL_PAD(data->imageSize);
}

const GLubyte *__glle_ConvolutionFilter2D(const GLubyte *PC)
{
    const struct __gllc_ConvolutionFilter2D_Rec *data;

    data = (const struct __gllc_ConvolutionFilter2D_Rec *) PC;
    __glim_ConvolutionFilter2D(
            data->target, data->internalformat, data->width, data->height, data->format, data->type,
            (const GLubyte *)(PC + sizeof(struct __gllc_ConvolutionFilter2D_Rec)));
    return PC + sizeof(struct __gllc_ConvolutionFilter2D_Rec) + __GL_PAD(data->imageSize);
}

const GLubyte *__glle_SeparableFilter2D(const GLubyte *PC)
{
    const struct __gllc_SeparableFilter2D_Rec *data;
    GLint rowSize, colSize;

    data = (const struct __gllc_SeparableFilter2D_Rec *) PC;
    rowSize = __glImageSize(data->width, 1, data->format, data->type);
    rowSize = __GL_PAD(rowSize);
    colSize = __glImageSize(data->height, 1, data->format, data->type);
    colSize = __GL_PAD(colSize);

    __glim_SeparableFilter2D(
        data->target, data->internalformat, data->width, data->height, data->format, data->type,
        (const GLubyte *)(PC + sizeof(struct __gllc_SeparableFilter2D_Rec)),
        (const GLubyte *)(PC + sizeof(struct __gllc_SeparableFilter2D_Rec)) + rowSize);

    return PC + sizeof(struct __gllc_SeparableFilter2D_Rec) + rowSize + colSize;
}

const GLubyte *__glle_CopyConvolutionFilter1D(const GLubyte *PC)
{
    const struct __gllc_CopyConvolutionFilter1D_Rec *data;

    data = (const struct __gllc_CopyConvolutionFilter1D_Rec *) PC;
    __glim_CopyConvolutionFilter1D(data->target, data->internalformat,
        data->x, data->y, data->width);
    return PC + sizeof(struct __gllc_CopyConvolutionFilter1D_Rec);
}

const GLubyte *__glle_CopyConvolutionFilter2D(const GLubyte *PC)
{
    const struct __gllc_CopyConvolutionFilter2D_Rec *data;

    data = (const struct __gllc_CopyConvolutionFilter2D_Rec *) PC;
    __glim_CopyConvolutionFilter2D(data->target, data->internalformat,
        data->x, data->y, data->width, data->height);

    return PC + sizeof(struct __gllc_CopyConvolutionFilter2D_Rec);
}

const GLubyte *__glle_Histogram(const GLubyte *PC)
{
    const struct __gllc_Histogram_Rec *data;

    data = (const struct __gllc_Histogram_Rec *) PC;
    __glim_Histogram(data->target, data->width,
        data->internalformat, data->sink);
    return PC + sizeof(struct __gllc_Histogram_Rec);
}

const GLubyte *__glle_ResetHistogram(const GLubyte *PC)
{
    const struct __gllc_ResetHistogram_Rec *data;

    data = (const struct __gllc_ResetHistogram_Rec *) PC;
    __glim_ResetHistogram(data->target);
    return PC + sizeof(struct __gllc_ResetHistogram_Rec);
}

const GLubyte *__glle_Minmax(const GLubyte *PC)
{
    const struct __gllc_Minmax_Rec *data;

    data = (const struct __gllc_Minmax_Rec *) PC;
    __glim_Minmax(data->target, data->internalFormat, data->sink);
    return PC + sizeof(struct __gllc_Minmax_Rec);
}

const GLubyte *__glle_ResetMinmax(const GLubyte *PC)
{
    const struct __gllc_ResetMinmax_Rec *data;

    data = (const struct __gllc_ResetMinmax_Rec *) PC;
    __glim_ResetMinmax(data->target);
    return PC + sizeof(struct __gllc_ResetMinmax_Rec);
}

const GLubyte *__glle_TexImage3D(const GLubyte *PC)
{
    const struct __gllc_TexImage3D_Rec *data;

    data = (const struct __gllc_TexImage3D_Rec *) PC;

    __glim_TexImage3D(
            data->target, data->lod, data->components, data->width, data->height,
            data->depth, data->border, data->format, data->type,
            (const GLubyte *)(PC + sizeof(struct __gllc_TexImage3D_Rec)));

    return (PC + sizeof(struct __gllc_TexImage3D_Rec) + __GL_PAD(data->imageSize));
}

const GLubyte *__glle_TexSubImage3D(const GLubyte *PC)
{
    const struct __gllc_TexSubImage3D_Rec *data;

    data = (const struct __gllc_TexSubImage3D_Rec *) PC;

    __glim_TexSubImage3D(
            data->target, data->lod, data->xoffset, data->yoffset, data->zoffset,
            data->width, data->height, data->depth, data->format, data->type,
            (const GLubyte *)(PC + sizeof(struct __gllc_TexSubImage3D_Rec)));

    return (PC + sizeof(struct __gllc_TexSubImage3D_Rec) + __GL_PAD(data->imageSize));
}

const GLubyte *__glle_CopyTexSubImage3D(const GLubyte *PC)
{
    struct __gllc_CopyTexSubImage3D_Rec *data;

    data = (struct __gllc_CopyTexSubImage3D_Rec *) PC;
    __glim_CopyTexSubImage3D(data->target, data->level, data->xoffset, data->yoffset,
            data->zoffset, data->x, data->y, data->width, data->height);
    return (PC + sizeof(struct __gllc_CopyTexSubImage3D_Rec));
}

const GLubyte *__glle_PointParameterfv(const GLubyte *PC)
{
    struct __gllc_PointParameterfv_Rec *data;

    data = (struct __gllc_PointParameterfv_Rec *) PC;
    __glim_PointParameterfv(data->pname, (GLfloat *)(PC+sizeof(struct __gllc_PointParameterfv_Rec)));
    return (PC + sizeof(struct __gllc_PointParameterfv_Rec) + data->paramSize);
}

const GLubyte *__glle_PointParameteriv(const GLubyte *PC)
{
    struct __gllc_PointParameteriv_Rec *data;

    data = (struct __gllc_PointParameteriv_Rec *) PC;
    __glim_PointParameteriv(data->pname, (GLint *)(PC+sizeof(struct __gllc_PointParameteriv_Rec)));
    return (PC + sizeof(struct __gllc_PointParameteriv_Rec) + data->paramSize);
}

const GLubyte *__glle_WindowPos2fv(const GLubyte *PC)
{
    struct __gllc_WindowPos2fv_Rec *data;

    data = (struct __gllc_WindowPos2fv_Rec *) PC;
    __glim_WindowPos2fv(data->v);
    return (PC + sizeof(struct __gllc_WindowPos2fv_Rec));
}

const GLubyte *__glle_WindowPos3fv(const GLubyte *PC)
{
    struct __gllc_WindowPos3fv_Rec *data;

    data = (struct __gllc_WindowPos3fv_Rec *) PC;
    __glim_WindowPos3fv(data->v);
    return (PC + sizeof(struct __gllc_WindowPos3fv_Rec));
}

const  GLubyte *__glle_SecondaryColor3fv(const GLubyte *PC)
{
    __GL_SETUP();
    struct __gllc_SecondaryColor3fv_Rec *data;

    data = (struct __gllc_SecondaryColor3fv_Rec *) PC;
    (*gc->currentImmediateTable->dispatch.SecondaryColor3fv)(data->v);
    return (PC + sizeof(struct __gllc_SecondaryColor3fv_Rec));
}

const GLubyte *__glle_BeginQuery(const GLubyte *PC)
{
    struct __gllc_BeginQuery_Rec *data;

    data = (struct __gllc_BeginQuery_Rec *) PC;
    __glim_BeginQuery(data->target, data->id);
    return (PC + sizeof(struct __gllc_BeginQuery_Rec));
}

const GLubyte *__glle_EndQuery(const GLubyte *PC)
{
    struct __gllc_EndQuery_Rec *data;

    data = (struct __gllc_EndQuery_Rec *) PC;
    __glim_EndQuery(data->target);
    return (PC + sizeof(struct __gllc_EndQuery_Rec));
}

#if GL_NV_occlusion_query
const GLubyte *__glle_BeginQueryNV(const GLubyte *PC)
{
    struct __gllc_BeginQuery_Rec *data;

    data = (struct __gllc_BeginQuery_Rec *) PC;
    __glim_BeginQueryNV(data->id);
    return (PC + sizeof(struct __gllc_BeginQuery_Rec));
}

const GLubyte *__glle_EndQueryNV(const GLubyte *PC)
{
    __glim_EndQueryNV();
    return (PC + sizeof(struct __gllc_EndQuery_Rec));
}
#endif

const GLubyte * __glle_Uniform4f(const GLubyte *PC)
{
    struct __gllc_Uniform4f_Rec *data;

    data = (struct __gllc_Uniform4f_Rec *) PC;
    __glim_Uniform4f(data->location, data->x, data->y, data->z, data->w);
    return (PC + sizeof(struct __gllc_Uniform4f_Rec));
}

const GLubyte * __glle_Uniform3f(const GLubyte *PC)
{
    struct __gllc_Uniform3f_Rec *data;

    data = (struct __gllc_Uniform3f_Rec *) PC;
    __glim_Uniform3f(data->location, data->x, data->y, data->z);
    return (PC + sizeof(struct __gllc_Uniform3f_Rec));
}

const GLubyte * __glle_Uniform2f(const GLubyte *PC)
{
    struct __gllc_Uniform2f_Rec *data;

    data = (struct __gllc_Uniform2f_Rec *) PC;
    __glim_Uniform2f(data->location, data->x, data->y);
    return (PC + sizeof(struct __gllc_Uniform2f_Rec));
}

const GLubyte * __glle_Uniform1f(const GLubyte *PC)
{
    struct __gllc_Uniform1f_Rec *data;

    data = (struct __gllc_Uniform1f_Rec *) PC;
    __glim_Uniform1f(data->location, data->x);
    return (PC + sizeof(struct __gllc_Uniform1f_Rec));
}

const GLubyte * __glle_Uniform4i(const GLubyte *PC)
{
    struct __gllc_Uniform4i_Rec *data;

    data = (struct __gllc_Uniform4i_Rec *) PC;
    __glim_Uniform4i(data->location, data->x, data->y, data->z, data->w);
    return (PC + sizeof(struct __gllc_Uniform4i_Rec));
}

const GLubyte * __glle_Uniform3i(const GLubyte *PC)
{
    struct __gllc_Uniform3i_Rec *data;

    data = (struct __gllc_Uniform3i_Rec *) PC;
    __glim_Uniform3i(data->location, data->x, data->y, data->z);
    return (PC + sizeof(struct __gllc_Uniform3i_Rec));
}

const GLubyte * __glle_Uniform2i(const GLubyte *PC)
{
    struct __gllc_Uniform2i_Rec *data;

    data = (struct __gllc_Uniform2i_Rec *) PC;
    __glim_Uniform2i(data->location, data->x, data->y);
    return (PC + sizeof(struct __gllc_Uniform2i_Rec));
}

const GLubyte * __glle_Uniform1i(const GLubyte *PC)
{
    struct __gllc_Uniform1i_Rec *data;

    data = (struct __gllc_Uniform1i_Rec *) PC;
    __glim_Uniform1i(data->location, data->x);
    return (PC + sizeof(struct __gllc_Uniform1i_Rec));
}

const GLubyte * __glle_Uniform4fv(const GLubyte *PC)
{
    struct __gllc_Uniform4fv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_Uniform4fv_Rec *) PC;
    __glim_Uniform4fv(data->location, data->count,
                    (GLfloat *)(PC + sizeof(struct __gllc_Uniform4fv_Rec)));
    arraySize = __GL64PAD(data->count * 4 * sizeof(GLfloat));
    size = sizeof(struct __gllc_Uniform4fv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_Uniform3fv(const GLubyte *PC)
{
    struct __gllc_Uniform3fv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_Uniform3fv_Rec *) PC;
    __glim_Uniform3fv(data->location, data->count,
                    (GLfloat *)(PC + sizeof(struct __gllc_Uniform3fv_Rec)));
    arraySize = __GL64PAD(data->count * 3 * sizeof(GLfloat));
    size = sizeof(struct __gllc_Uniform3fv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_Uniform2fv(const GLubyte *PC)
{
    struct __gllc_Uniform2fv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_Uniform2fv_Rec *) PC;
    __glim_Uniform2fv(data->location, data->count,
                    (GLfloat *)(PC + sizeof(struct __gllc_Uniform2fv_Rec)));
    arraySize = __GL64PAD(data->count * 2 * sizeof(GLfloat));
    size = sizeof(struct __gllc_Uniform2fv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_Uniform1fv(const GLubyte *PC)
{
    struct __gllc_Uniform1fv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_Uniform1fv_Rec *) PC;
    __glim_Uniform1fv(data->location, data->count,
                    (GLfloat *)(PC + sizeof(struct __gllc_Uniform1fv_Rec)));
    arraySize = __GL64PAD(data->count * sizeof(GLfloat));
    size = sizeof(struct __gllc_Uniform1fv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_Uniform4iv(const GLubyte *PC)
{
    struct __gllc_Uniform4iv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_Uniform4iv_Rec *) PC;
    __glim_Uniform4iv(data->location, data->count,
                    (GLint *)(PC + sizeof(struct __gllc_Uniform4iv_Rec)));
    arraySize = __GL64PAD(data->count * 4 * sizeof(GLint));
    size = sizeof(struct __gllc_Uniform4iv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_Uniform3iv(const GLubyte *PC)
{
    struct __gllc_Uniform3iv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_Uniform3iv_Rec *) PC;
    __glim_Uniform3iv(data->location, data->count,
                    (GLint *)(PC + sizeof(struct __gllc_Uniform3iv_Rec)));
    arraySize = __GL64PAD(data->count * 3 * sizeof(GLint));
    size = sizeof(struct __gllc_Uniform3iv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_Uniform2iv(const GLubyte *PC)
{
    struct __gllc_Uniform2iv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_Uniform2iv_Rec *) PC;
    __glim_Uniform2iv(data->location, data->count,
                    (GLint *)(PC + sizeof(struct __gllc_Uniform2iv_Rec)));
    arraySize = __GL64PAD(data->count * 2 * sizeof(GLint));
    size = sizeof(struct __gllc_Uniform2iv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_Uniform1iv(const GLubyte *PC)
{
    struct __gllc_Uniform1iv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_Uniform1iv_Rec *) PC;
    __glim_Uniform1iv(data->location, data->count,
                    (GLint *)(PC + sizeof(struct __gllc_Uniform1iv_Rec)));
    arraySize = __GL64PAD(data->count * sizeof(GLint));
    size = sizeof(struct __gllc_Uniform1iv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_UniformMatrix4fv(const GLubyte *PC)
{
    struct __gllc_UniformMatrix4fv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_UniformMatrix4fv_Rec *) PC;
    __glim_UniformMatrix4fv(data->location, data->count,
        data->transpose, (GLfloat *)(PC + sizeof(struct __gllc_UniformMatrix4fv_Rec)));
    arraySize = __GL64PAD(data->count * 16 * sizeof(GLfloat));
    size = sizeof(struct __gllc_UniformMatrix4fv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_UniformMatrix3fv(const GLubyte *PC)
{
    struct __gllc_UniformMatrix3fv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_UniformMatrix3fv_Rec *) PC;
    __glim_UniformMatrix3fv(data->location, data->count,
        data->transpose, (GLfloat *)(PC + sizeof(struct __gllc_UniformMatrix3fv_Rec)));
    arraySize = __GL64PAD(data->count * 9 * sizeof(GLfloat));
    size = sizeof(struct __gllc_UniformMatrix3fv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_UniformMatrix2fv(const GLubyte *PC)
{
    struct __gllc_UniformMatrix2fv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_UniformMatrix2fv_Rec *) PC;
    __glim_UniformMatrix2fv(data->location, data->count,
        data->transpose, (GLfloat *)(PC + sizeof(struct __gllc_UniformMatrix2fv_Rec)));
    arraySize = __GL64PAD(data->count * 4 * sizeof(GLfloat));
    size = sizeof(struct __gllc_UniformMatrix2fv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_UniformMatrix2x3fv(const GLubyte *PC)
{
    struct __gllc_UniformMatrix2x3fv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_UniformMatrix2x3fv_Rec *) PC;
    __glim_UniformMatrix2x3fv(data->location, data->count,
        data->transpose, (GLfloat *)(PC + sizeof(struct __gllc_UniformMatrix2x3fv_Rec)));
    arraySize = __GL64PAD(data->count * 6 * sizeof(GLfloat));
    size = sizeof(struct __gllc_UniformMatrix2x3fv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_UniformMatrix2x4fv(const GLubyte *PC)
{
    struct __gllc_UniformMatrix2x4fv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_UniformMatrix2x4fv_Rec *) PC;
    __glim_UniformMatrix2x4fv(data->location, data->count,
        data->transpose, (GLfloat *)(PC + sizeof(struct __gllc_UniformMatrix2x4fv_Rec)));
    arraySize = __GL64PAD(data->count * 8 * sizeof(GLfloat));
    size = sizeof(struct __gllc_UniformMatrix2x4fv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_UniformMatrix3x2fv(const GLubyte *PC)
{
    struct __gllc_UniformMatrix3x2fv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_UniformMatrix3x2fv_Rec *) PC;
    __glim_UniformMatrix3x2fv(data->location, data->count,
        data->transpose, (GLfloat *)(PC + sizeof(struct __gllc_UniformMatrix3x2fv_Rec)));
    arraySize = __GL64PAD(data->count * 6 * sizeof(GLfloat));
    size = sizeof(struct __gllc_UniformMatrix3x2fv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_UniformMatrix3x4fv(const GLubyte *PC)
{
    struct __gllc_UniformMatrix3x4fv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_UniformMatrix3x4fv_Rec *) PC;
    __glim_UniformMatrix3x4fv(data->location, data->count,
        data->transpose, (GLfloat *)(PC + sizeof(struct __gllc_UniformMatrix3x4fv_Rec)));
    arraySize = __GL64PAD(data->count * 12 * sizeof(GLfloat));
    size = sizeof(struct __gllc_UniformMatrix3x4fv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_UniformMatrix4x2fv(const GLubyte *PC)
{
    struct __gllc_UniformMatrix4x2fv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_UniformMatrix4x2fv_Rec *) PC;
    __glim_UniformMatrix4x2fv(data->location, data->count,
        data->transpose, (GLfloat *)(PC + sizeof(struct __gllc_UniformMatrix4x2fv_Rec)));
    arraySize = __GL64PAD(data->count * 8 * sizeof(GLfloat));
    size = sizeof(struct __gllc_UniformMatrix4x2fv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_UniformMatrix4x3fv(const GLubyte *PC)
{
    struct __gllc_UniformMatrix4x3fv_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_UniformMatrix4x3fv_Rec *) PC;
    __glim_UniformMatrix4x3fv(data->location, data->count,
        data->transpose, (GLfloat *)(PC + sizeof(struct __gllc_UniformMatrix4x3fv_Rec)));
    arraySize = __GL64PAD(data->count * 12 * sizeof(GLfloat));
    size = sizeof(struct __gllc_UniformMatrix4x3fv_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_DrawBuffers(const GLubyte *PC)
{
    struct __gllc_DrawBuffers_Rec *data;
    GLuint arraySize, size;

    data = (struct __gllc_DrawBuffers_Rec *) PC;
    __glim_DrawBuffers(data->count,
            (GLenum *)(PC + sizeof(struct __gllc_DrawBuffers_Rec)));
    arraySize = __GL64PAD(data->count * sizeof(GLenum));
    size = sizeof(struct __gllc_DrawBuffers_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_UseProgram(const GLubyte *PC)
{
    struct __gllc_UseProgram_Rec *data;

    data = (struct __gllc_UseProgram_Rec *) PC;
    __glim_UseProgram(data->program);
    return (PC + sizeof(struct __gllc_UseProgram_Rec));
}

#if GL_ARB_vertex_program

const GLubyte *__glle_BindProgramARB(const GLubyte *PC)
{
    struct __gllc_BindProgramARB_Rec *data;

    data = (struct __gllc_BindProgramARB_Rec *)PC;
    __glim_BindProgramARB(data->target, data->program);

    return PC + sizeof(struct __gllc_BindProgramARB_Rec);
}

const GLubyte *__glle_ProgramEnvParameter4dARB(const GLubyte *PC)
{
    struct __gllc_ProgramEnvParameter4dARB_Rec *data;

    data = (struct __gllc_ProgramEnvParameter4dARB_Rec *)PC;
    __glim_ProgramEnvParameter4dARB(data->target, data->index,
                         data->x, data->y, data->z, data->w);

    return PC + sizeof(struct __gllc_ProgramEnvParameter4dARB_Rec);
}

const GLubyte *__glle_ProgramEnvParameter4dvARB(const GLubyte *PC)
{
    struct __gllc_ProgramEnvParameter4dvARB_Rec *data;

    data = (struct __gllc_ProgramEnvParameter4dvARB_Rec *)PC;
    __glim_ProgramEnvParameter4dvARB(data->target, data->index,
                         data->v);

    return PC + sizeof(struct __gllc_ProgramEnvParameter4dvARB_Rec);
}

const GLubyte *__glle_ProgramEnvParameter4fARB(const GLubyte *PC)
{
    struct __gllc_ProgramEnvParameter4fARB_Rec *data;

    data = (struct __gllc_ProgramEnvParameter4fARB_Rec *)PC;
    __glim_ProgramEnvParameter4fARB(data->target, data->index,
                         data->x, data->y, data->z, data->w);

    return PC + sizeof(struct __gllc_ProgramEnvParameter4fARB_Rec);
}

const GLubyte *__glle_ProgramEnvParameter4fvARB(const GLubyte *PC)
{
    struct __gllc_ProgramEnvParameter4fvARB_Rec *data;

    data = (struct __gllc_ProgramEnvParameter4fvARB_Rec *)PC;
    __glim_ProgramEnvParameter4fvARB(data->target, data->index,
                         data->v);

    return PC + sizeof(struct __gllc_ProgramEnvParameter4fvARB_Rec);
}

const GLubyte *__glle_ProgramLocalParameter4dARB(const GLubyte *PC)
{
    struct __gllc_ProgramEnvParameter4dARB_Rec *data;

    data = (struct __gllc_ProgramEnvParameter4dARB_Rec *)PC;
    __glim_ProgramLocalParameter4dARB(data->target, data->index,
                         data->x, data->y, data->z, data->w);

    return PC + sizeof(struct __gllc_ProgramEnvParameter4dARB_Rec);
}

const GLubyte *__glle_ProgramLocalParameter4dvARB(const GLubyte *PC)
{
    struct __gllc_ProgramEnvParameter4dvARB_Rec *data;

    data = (struct __gllc_ProgramEnvParameter4dvARB_Rec *)PC;
    __glim_ProgramLocalParameter4dvARB(data->target, data->index,
                         data->v);

    return PC + sizeof(struct __gllc_ProgramEnvParameter4dvARB_Rec);
}

const GLubyte *__glle_ProgramLocalParameter4fARB(const GLubyte *PC)
{
    struct __gllc_ProgramEnvParameter4fARB_Rec *data;

    data = (struct __gllc_ProgramEnvParameter4fARB_Rec *)PC;
    __glim_ProgramLocalParameter4fARB(data->target, data->index,
                         data->x, data->y, data->z, data->w);

    return PC + sizeof(struct __gllc_ProgramEnvParameter4fARB_Rec);
}

const GLubyte *__glle_ProgramLocalParameter4fvARB(const GLubyte *PC)
{
    struct __gllc_ProgramEnvParameter4fvARB_Rec *data;

    data = (struct __gllc_ProgramEnvParameter4fvARB_Rec *)PC;
    __glim_ProgramLocalParameter4fvARB(data->target, data->index,
                         data->v);

    return PC + sizeof(struct __gllc_ProgramEnvParameter4fvARB_Rec);
}

#endif

#if GL_ATI_element_array
const GLubyte *__glle_DrawElementArrayATI(const GLubyte *PC)
{
    struct __gllc_DrawElementArrayATI_Rec *data;

    data = (struct __gllc_DrawElementArrayATI_Rec *)PC;
    __glim_DrawElementArrayATI(data->mode, data->count);

    return PC + sizeof(struct __gllc_DrawElementArrayATI_Rec);
}

const GLubyte *__glle_DrawRangeElementArrayATI(const GLubyte *PC)
{
    struct __gllc_DrawRangeElementArrayATI_Rec *data;

    data = (struct __gllc_DrawRangeElementArrayATI_Rec *)PC;
    __glim_DrawRangeElementArrayATI(data->mode, data->start,
                     data->end, data->count);

    return PC + sizeof(struct __gllc_DrawRangeElementArrayATI_Rec);
}
#endif

#if GL_EXT_stencil_two_side
const GLubyte *__glle_ActiveStencilFaceEXT(const GLubyte *PC)
{
    struct __gllc_ActiveStencilFaceEXT_Rec *data;

    data = (struct __gllc_ActiveStencilFaceEXT_Rec *)PC;
    __glim_ActiveStencilFaceEXT(data->face);

    return PC + sizeof(struct __gllc_ActiveStencilFaceEXT_Rec);
}
#endif

#if GL_EXT_texture_integer
const GLubyte * __glle_ClearColorIiEXT(const GLubyte *PC)
{
    struct __gllc_ClearColorIiEXT_Rec *data;

    data = (struct __gllc_ClearColorIiEXT_Rec *)PC;
    __glim_ClearColorIiEXT(data->red, data->green, data->blue, data->alpha);

    return PC + sizeof(struct __gllc_ClearColorIiEXT_Rec);
}

const GLubyte * __glle_ClearColorIuiEXT(const GLubyte *PC)
{
    struct __gllc_ClearColorIuiEXT_Rec *data;

    data = (struct __gllc_ClearColorIuiEXT_Rec *)PC;
    __glim_ClearColorIuiEXT(data->red, data->green, data->blue, data->alpha);

    return PC + sizeof(struct __gllc_ClearColorIuiEXT_Rec);

}

const GLubyte * __glle_TexParameterIivEXT(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_TexParameterIivEXT_Rec *data;

    data = (struct __gllc_TexParameterIivEXT_Rec *) PC;
    __glim_TexParameterIivEXT(data->target, data->pname,
            (GLint *) (PC + sizeof(struct __gllc_TexParameterIivEXT_Rec)));
    arraySize = __GL64PAD(__glTexParameter_size(data->pname) * 4);
    size = sizeof(struct __gllc_TexParameterIivEXT_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_TexParameterIuivEXT(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_TexParameterIuivEXT_Rec *data;

    data = (struct __gllc_TexParameterIuivEXT_Rec *) PC;
    __glim_TexParameterIuivEXT(data->target, data->pname,
            (GLuint *) (PC + sizeof(struct __gllc_TexParameterIuivEXT_Rec)));
    arraySize = __GL64PAD(__glTexParameter_size(data->pname) * 4);
    size = sizeof(struct __gllc_TexParameterIuivEXT_Rec) + arraySize;
    return PC + size;
}
#endif

#if GL_EXT_gpu_shader4
const GLubyte * __glle_Uniform1uiEXT(const GLubyte *PC)
{
    struct __gllc_Uniform1uiEXT_Rec *data;

    data = (struct __gllc_Uniform1uiEXT_Rec *)PC;
    __glim_Uniform1uiEXT(data->location, data->v0);

    return PC + sizeof(struct __gllc_Uniform1uiEXT_Rec);

}

const GLubyte * __glle_Uniform2uiEXT(const GLubyte *PC)
{
    struct __gllc_Uniform2uiEXT_Rec *data;

    data = (struct __gllc_Uniform2uiEXT_Rec *)PC;
    __glim_Uniform2uiEXT(data->location, data->v0, data->v1);

    return PC + sizeof(struct __gllc_Uniform2uiEXT_Rec);
}

const GLubyte * __glle_Uniform3uiEXT(const GLubyte *PC)
{
    struct __gllc_Uniform3uiEXT_Rec *data;

    data = (struct __gllc_Uniform3uiEXT_Rec *)PC;
    __glim_Uniform3uiEXT(data->location, data->v0, data->v1, data->v2);

    return PC + sizeof(struct __gllc_Uniform3uiEXT_Rec);
}

const GLubyte * __glle_Uniform4uiEXT(const GLubyte *PC)
{
    struct __gllc_Uniform4uiEXT_Rec *data;

    data = (struct __gllc_Uniform4uiEXT_Rec *)PC;
    __glim_Uniform4uiEXT(data->location, data->v0, data->v1, data->v2, data->v3);

    return PC + sizeof(struct __gllc_Uniform4uiEXT_Rec);
}

const GLubyte * __glle_Uniform1uivEXT(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_Uniform1uivEXT_Rec *data;

    data = (struct __gllc_Uniform1uivEXT_Rec *) PC;
    __glim_Uniform1uivEXT(data->location, data->count,
            (GLuint *) (PC + sizeof(struct __gllc_Uniform1uivEXT_Rec)));
    arraySize = __GL64PAD(data->count * sizeof(GLuint));
    size = sizeof(struct __gllc_Uniform1uivEXT_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_Uniform2uivEXT(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_Uniform2uivEXT_Rec *data;

    data = (struct __gllc_Uniform2uivEXT_Rec *) PC;
    __glim_Uniform2uivEXT(data->location, data->count,
            (GLuint *) (PC + sizeof(struct __gllc_Uniform2uivEXT_Rec)));
    arraySize = __GL64PAD(data->count * 2 * sizeof(GLuint));
    size = sizeof(struct __gllc_Uniform2uivEXT_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_Uniform3uivEXT(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_Uniform3uivEXT_Rec *data;

    data = (struct __gllc_Uniform3uivEXT_Rec *) PC;
    __glim_Uniform3uivEXT(data->location, data->count,
            (GLuint *) (PC + sizeof(struct __gllc_Uniform3uivEXT_Rec)));
    arraySize = __GL64PAD(data->count * 3 * sizeof(GLuint));
    size = sizeof(struct __gllc_Uniform3uivEXT_Rec) + arraySize;
    return PC + size;
}

const GLubyte * __glle_Uniform4uivEXT(const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_Uniform4uivEXT_Rec *data;

    data = (struct __gllc_Uniform4uivEXT_Rec *) PC;
    __glim_Uniform4uivEXT(data->location, data->count,
            (GLuint *) (PC + sizeof(struct __gllc_Uniform4uivEXT_Rec)));
    arraySize = __GL64PAD(data->count * 4 * sizeof(GLuint));
    size = sizeof(struct __gllc_Uniform4uivEXT_Rec) + arraySize;
    return PC + size;
}


#endif

#if GL_EXT_geometry_shader4
const GLubyte * __glle_FramebufferTextureEXT(const GLubyte *PC)
{
    struct __gllc_FramebufferTextureEXT_Rec *data;

    data = (struct __gllc_FramebufferTextureEXT_Rec *)PC;
    __glim_FramebufferTextureEXT(data->target, data->attachment, data->texture, data->level);

    return PC + sizeof(struct __gllc_FramebufferTextureEXT_Rec);
}

const GLubyte * __glle_FramebufferTextureLayerEXT(const GLubyte *PC)
{
    struct __gllc_FramebufferTextureLayerEXT_Rec *data;

    data = (struct __gllc_FramebufferTextureLayerEXT_Rec *)PC;
    __glim_FramebufferTextureLayerEXT(data->target, data->attachment, data->texture, data->level, data->layer);

    return PC + sizeof(struct __gllc_FramebufferTextureLayerEXT_Rec);
}

const GLubyte * __glle_FramebufferTextureFaceEXT(const GLubyte *PC)
{
    struct __gllc_FramebufferTextureFaceEXT_Rec *data;

    data = (struct __gllc_FramebufferTextureFaceEXT_Rec *)PC;
    __glim_FramebufferTextureFaceEXT(data->target, data->attachment, data->texture, data->level, data->face);

    return PC + sizeof(struct __gllc_FramebufferTextureFaceEXT_Rec);
}
#endif

#if GL_EXT_draw_buffers2

const GLubyte *__glle_ColorMaskIndexedEXT(const GLubyte *PC)
{
    struct __gllc_ColorMaskIndexedEXT_Rec* data;
    data = (struct __gllc_ColorMaskIndexedEXT_Rec*) PC;

    __glim_ColorMaskIndexedEXT(data->buf, data->r, data->g, data->b, data->a);
    return (PC + sizeof(struct __gllc_ColorMaskIndexedEXT_Rec));

}

const GLubyte *__glle_EnableIndexedEXT(const GLubyte *PC)
{
    struct __gllc_EnableIndexedEXT_Rec* data;
    data = (struct __gllc_EnableIndexedEXT_Rec*) PC;

    __glim_EnableIndexedEXT(data->target, data->index);
    return (PC + sizeof(struct __gllc_EnableIndexedEXT_Rec));
}

const GLubyte *__glle_DisableIndexedEXT(const GLubyte *PC)
{
    struct __gllc_DisableIndexedEXT_Rec* data;
    data = (struct __gllc_DisableIndexedEXT_Rec*) PC;

    __glim_DisableIndexedEXT(data->target, data->index);
    return (PC + sizeof(struct __gllc_DisableIndexedEXT_Rec));
}
#endif

#if GL_EXT_gpu_program_parameters
const GLubyte * __glle_ProgramEnvParameters4fvEXT(const GLubyte *PC)
{
    GLint size;
    struct __gllc_ProgramEnvParameters4fvEXT_Rec *data;
    GLint paramSize;

    data = (struct __gllc_ProgramEnvParameters4fvEXT_Rec*) PC;

    paramSize = __GL64PAD(data->count * 2 * sizeof(GLfloat));

    __glim_ProgramEnvParameters4fvEXT(data->target, data->index, data->count,
        (GLfloat *)(PC + sizeof(struct __gllc_ProgramEnvParameters4fvEXT_Rec)));

    size = sizeof(struct __gllc_ProgramEnvParameters4fvEXT_Rec) + paramSize;

    return PC + size;
}

const GLubyte * __glle_ProgramLocalParameters4fvEXT(const GLubyte *PC)
{
    GLint size;
    struct __gllc_ProgramLocalParameters4fvEXT_Rec *data;
    GLint paramSize;

    data = (struct __gllc_ProgramLocalParameters4fvEXT_Rec*) PC;

    paramSize = __GL64PAD(data->count * 2 * sizeof(GLfloat));

    __glim_ProgramEnvParameters4fvEXT(data->target, data->index, data->count,
        (GLfloat *)(PC + sizeof(struct __gllc_ProgramLocalParameters4fvEXT_Rec)));

    size = sizeof(struct __gllc_ProgramLocalParameters4fvEXT_Rec) + paramSize;

    return PC + size;
}
#endif

#if GL_ARB_color_buffer_float
const GLubyte *__glle_ClampColorARB(const GLubyte *PC)
{
    struct __gllc_ClampColorARB_Rec* data;
    data = (struct __gllc_ClampColorARB_Rec*) PC;

    __glim_ClampColorARB(data->target, data->clamp);
    return (PC + sizeof(struct __gllc_ClampColorARB_Rec));
}
#endif

#if GL_ATI_separate_stencil
const GLubyte* __glle_StencilFuncSeparateATI(const GLubyte* PC)
{
    struct __gllc_StencilFuncSeparateATI_Rec* data;
    data = (struct __gllc_StencilFuncSeparateATI_Rec*)PC;

    __glim_StencilFuncSeparateATI(data->frontfunc, data->backfunc, data->ref, data->mask);

    return (PC + sizeof(struct __gllc_StencilFuncSeparateATI_Rec));
}
#endif