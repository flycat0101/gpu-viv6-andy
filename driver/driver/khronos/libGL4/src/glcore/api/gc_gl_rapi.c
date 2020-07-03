/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_es_context.h"


#if defined(_WIN32)
#pragma warning(disable: 4244)
#endif


/* swp_select.c */
extern GLvoid __glSelectPoint(__GLcontext *, __GLvertex *rp);


__GL_INLINE GLvoid __glUpdateRasterStreamInfo(__GLcontext* gc, GLfloat *fv4)
{
    /*Multi stream*/
    __GLstreamDecl *stream;
    __GLvertexElement *element;

    /* Compute the required attribute mask */
    if (gc->input.inputMaskChanged)
    {
        __glComputeRequiredInputMask(gc);
        gc->input.inputMaskChanged = GL_FALSE;
    }
    gc->input.requiredInputMask = gc->input.currentInputMask & edgeFlagInputMask[GL_POINTS];
    gc->vertexStreams.primElementMask = gc->input.requiredInputMask;
    gc->vertexStreams.missingAttribs = gc->input.requiredInputMask & (~__GL_INPUT_VERTEX) & (~__GL_INPUT_EDGEFLAG);

    gc->vertexStreams.numStreams = 0;
    gc->vertexStreams.endVertex = 1;
    gc->vertexStreams.indexCount = 0;
    gc->vertexStreams.startVertex = 0;
    gc->vertexStreams.edgeflagStream = NULL;

    if (GL_POINTS != gc->vertexStreams.primMode)
    {
        gc->vertexStreams.primMode = GL_POINTS;
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_PRIMMODE_BIT);
    }

    /*Raster pos*/
    stream = &gc->vertexStreams.streams[0];
    stream->numElements = 1;
    stream->streamAddr = fv4;
    stream->stride = 4 * sizeof(GLfloat);
    stream->privPtrAddr = NULL;

    element = &stream->streamElement[0];
    element->streamIndex = 0;
    element->inputIndex = 0;
    element->normalized = GL_FALSE;
    element->integer = GL_FALSE;
    element->offset = 0;
    element->size = 4;
    element->type = GL_FLOAT;

    /* For raster pos there is only one vertex stream, all other attribute streams are missing */
    gc->vertexStreams.numStreams = 1;

    /*Need another type?*/
    gc->vertexStreams.streamMode = IMMEDIATE_STREAMMODE;
}

GLvoid __glConvertToScreenSpacePos(__GLcontext* gc, GLfloat* clipPos, GLfloat* screenPos)
{
    __GLviewport* lpViewport = &gc->state.viewport;
    GLfloat rhw, z, y;
    GLfloat clipX,clipY,clipZ,clipW;
    GLfloat px,py,ox,oy;
    GLfloat fAn; /* far + near */
    GLfloat fSn; /*far - near */
    GLboolean invertY = (DRAW_FRAMEBUFFER_BINDING_NAME == 0)
                      ? (gcoSURF_QueryFlags((gcoSURF)(gc->drawablePrivate->rtHandles[0]), gcvSURF_FLAG_CONTENT_YINVERTED) == gcvSTATUS_OK)
                      : GL_FALSE;

    ox = (GLfloat)lpViewport->x + (GLfloat)lpViewport->width/2;
    oy = (GLfloat)lpViewport->y + (GLfloat)lpViewport->height/2;
    px = (GLfloat)lpViewport->width;
    py = (GLfloat)lpViewport->height;
    fAn = lpViewport->zFar + lpViewport->zNear;
    fSn = lpViewport->zFar - lpViewport->zNear;

    clipX = *clipPos;
    clipY = *(clipPos+1);
    clipZ = *(clipPos+2);
    clipW = *(clipPos+3);

    rhw = 1.0f / (clipW);
    *screenPos++ = (px/2)*clipX *rhw + ox;
    y = (py/2)*clipY *rhw + oy;
    if (invertY)
        y = gc->drawablePrivate->height - y;
    *screenPos++ = y;
    z = (fSn/2) * clipZ * rhw + fAn/2;

    if (z < 0.0f)
        z = 0.0f;
    else if (z > 1.0f)
        z = 1.0f;

    *screenPos++ = z;
    *screenPos++ = 1.0;
}

__GL_INLINE GLboolean __glCanDoFastRasterPos(__GLcontext* gc)
{
    if (gc->shaderProgram.vertShaderEnable || gc->state.enables.program.vertexProgram || /* vertex shader is active */
       gc->state.enables.lighting.lighting || /* Enable lighting */
       gc->state.enables.transform.clipPlanesMask || /* Enable user clip plane*/
       gc->state.enables.fog || /* Enable fog */
       gc->state.point.distanceAttenuation[0] != 1.0f || /* Point size scale */
       gc->state.point.distanceAttenuation[1] != 0.0f ||
       gc->state.point.distanceAttenuation[2] != 0.0f ||
       gc->texture.enabledMask || /* Texture enable */
       gc->renderMode != GL_RENDER)
    {
        return GL_FALSE;
    }

    return GL_TRUE;
}

void __glRasterPos4fvFast(__GLcontext* gc, GLfloat *fv4)
{
    __GLRasterPosState *rp = &gc->state.rasterPos;
    __GLmatrix mvp  = gc->transform.modelView->mvp;
    __GLcoord objCoord, clipCoord;
    GLfloat screenPos[4];
    GLfloat clampedSize;

    /* Coordinate transformations */
    objCoord.f.x = fv4[0];    objCoord.f.y = fv4[1];    objCoord.f.z = fv4[2];    objCoord.f.w = fv4[3];

    __glTransformCoord(&clipCoord,  &objCoord, &mvp);

    if ((clipCoord.f.x > clipCoord.f.w || clipCoord.f.x < -clipCoord.f.w) ||
       (clipCoord.f.y > clipCoord.f.w || clipCoord.f.y < -clipCoord.f.w) ||
       (clipCoord.f.z > clipCoord.f.w || clipCoord.f.z < -clipCoord.f.w))
    {
        gc->state.rasterPos.validRasterPos = GL_FALSE;
        return;
    }

    __glConvertToScreenSpacePos(gc, clipCoord.v, screenPos);

    /* Build current raster position and associated data */
    rp->rPos.winPos.f.x = screenPos[0];
    rp->rPos.winPos.f.y = screenPos[1];
    rp->rPos.winPos.f.z = screenPos[2];
    rp->rPos.winPos.f.w = screenPos[3];
    rp->rPos.clipW    = 1/rp->rPos.winPos.f.w;

    rp->rPos.colors[__GL_BACKFACE].r = rp->rPos.colors[__GL_FRONTFACE].r = gc->state.current.color.r;
    rp->rPos.colors[__GL_BACKFACE].g = rp->rPos.colors[__GL_FRONTFACE].g = gc->state.current.color.g;
    rp->rPos.colors[__GL_BACKFACE].b = rp->rPos.colors[__GL_FRONTFACE].b = gc->state.current.color.b;
    rp->rPos.colors[__GL_BACKFACE].a = rp->rPos.colors[__GL_FRONTFACE].a = gc->state.current.color.a;

    rp->rPos.colors2[__GL_BACKFACE].r = rp->rPos.colors2[__GL_FRONTFACE].r = gc->state.current.color2.r;
    rp->rPos.colors2[__GL_BACKFACE].g = rp->rPos.colors2[__GL_FRONTFACE].g = gc->state.current.color2.g;
    rp->rPos.colors2[__GL_BACKFACE].b = rp->rPos.colors2[__GL_FRONTFACE].b = gc->state.current.color2.b;
    rp->rPos.colors2[__GL_BACKFACE].a = rp->rPos.colors2[__GL_FRONTFACE].a = gc->state.current.color2.a;

    clampedSize = gc->state.point.requestedSize;
    clampedSize = (clampedSize >= gc->state.point.sizeMax) ? gc->state.point.sizeMax : clampedSize;
    clampedSize = (clampedSize <= gc->state.point.sizeMin) ? gc->state.point.sizeMin : clampedSize;
    rp->rPos.pointSize = clampedSize;

    rp->rPos.boundaryEdge = 0;
    rp->validRasterPos = GL_TRUE;
}

__GL_INLINE GLvoid __glRasterPos4fv(__GLcontext* gc, GLfloat *fv4)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);
    __GL_VERTEX_BUFFER_FLUSH(gc);

    if (gc->input.deferredAttribDirty)
    {
        __glCopyDeferedAttribToCurrent(gc);
    }

    if (__glCanDoFastRasterPos(gc))
    {
        __glRasterPos4fvFast(gc, fv4);
        return;
    }

    __glUpdateRasterStreamInfo(gc, fv4);

//    __glEvaluateAttribDrawableChange(gc);

/* To do */
//    GL_ASSERT(0);

}

/* OpenGL raster position APIs */


GLvoid APIENTRY __glim_RasterPos2dv(__GLcontext* gc, const GLdouble *v)
{
    GLfloat fv4[4];

    fv4[0] = v[0]; fv4[1] = v[1];
    fv4[2] = __glZero;
    fv4[3] = __glOne;
    __glRasterPos4fv(gc, fv4);
}

GLvoid APIENTRY __glim_RasterPos2fv(__GLcontext* gc, const GLfloat *v)
{
    GLfloat fv4[4];

    fv4[0] = v[0]; fv4[1] = v[1];
    fv4[2] = __glZero;
    fv4[3] = __glOne;
    __glRasterPos4fv(gc, fv4);
}

GLvoid APIENTRY __glim_RasterPos2d(__GLcontext* gc, GLdouble x, GLdouble y)
{
    GLfloat fv4[4];

    fv4[0] = x; fv4[1] = y;
    fv4[2] = __glZero;
    fv4[3] = __glOne;
    __glRasterPos4fv(gc, fv4);
}

GLvoid APIENTRY __glim_RasterPos2f(__GLcontext* gc, GLfloat x, GLfloat y)
{
    GLfloat fv4[4];

    fv4[0] = x; fv4[1] = y;
    fv4[2] = __glZero;
    fv4[3] = __glOne;
    __glRasterPos4fv(gc, fv4);
}

GLvoid APIENTRY __glim_RasterPos2i(__GLcontext* gc, GLint x, GLint y)
{
    GLfloat fv4[4];

    fv4[0] = x; fv4[1] = y;
    fv4[2] = __glZero;
    fv4[3] = __glOne;
    __glRasterPos4fv(gc, fv4);
}

GLvoid APIENTRY __glim_RasterPos2iv(__GLcontext* gc, const GLint *v)
{
    GLfloat fv4[4];

    fv4[0] = v[0]; fv4[1] = v[1];
    fv4[2] = __glZero;
    fv4[3] = __glOne;
    __glRasterPos4fv(gc, fv4);
}

GLvoid APIENTRY __glim_RasterPos2s(__GLcontext* gc, GLshort x, GLshort y)
{
    GLfloat fv4[4];

    fv4[0] = x; fv4[1] = y;
    fv4[2] = __glZero;
    fv4[3] = __glOne;
    __glRasterPos4fv(gc, fv4);
}

GLvoid APIENTRY __glim_RasterPos2sv(__GLcontext* gc, const GLshort *v)
{
    GLfloat fv4[4];

    fv4[0] = v[0]; fv4[1] = v[1];
    fv4[2] = __glZero;
    fv4[3] = __glOne;
    __glRasterPos4fv(gc, fv4);
}

GLvoid APIENTRY __glim_RasterPos3dv(__GLcontext* gc, const GLdouble *v)
{
    GLfloat fv4[4];

    fv4[0] = v[0]; fv4[1] = v[1]; fv4[2] = v[2];
    fv4[3] = __glOne;
    __glRasterPos4fv(gc, fv4);
}

GLvoid APIENTRY __glim_RasterPos3fv(__GLcontext* gc, const GLfloat *v)
{
    GLfloat fv4[4];

    fv4[0] = v[0]; fv4[1] = v[1]; fv4[2] = v[2];
    fv4[3] = __glOne;
    __glRasterPos4fv(gc, fv4);
}

GLvoid APIENTRY __glim_RasterPos3d(__GLcontext* gc, GLdouble x, GLdouble y, GLdouble z)
{
    GLfloat fv4[4];

    fv4[0] = x; fv4[1] = y; fv4[2] = z;
    fv4[3] = __glOne;
    __glRasterPos4fv(gc, fv4);
}

GLvoid APIENTRY __glim_RasterPos3f(__GLcontext* gc, GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat fv4[4];

    fv4[0] = x; fv4[1] = y; fv4[2] = z;
    fv4[3] = __glOne;
    __glRasterPos4fv(gc, fv4);
}

GLvoid APIENTRY __glim_RasterPos3i(__GLcontext* gc, GLint x, GLint y, GLint z)
{
    GLfloat fv4[4];

    fv4[0] = x; fv4[1] = y; fv4[2] = z;
    fv4[3] = __glOne;
    __glRasterPos4fv(gc, fv4);
}

GLvoid APIENTRY __glim_RasterPos3iv(__GLcontext* gc, const GLint *v)
{
    GLfloat fv4[4];

    fv4[0] = v[0]; fv4[1] = v[1]; fv4[2] = v[2];
    fv4[3] = __glOne;
    __glRasterPos4fv(gc, fv4);
}

GLvoid APIENTRY __glim_RasterPos3s(__GLcontext* gc, GLshort x, GLshort y, GLshort z)
{
    GLfloat fv4[4];

    fv4[0] = x; fv4[1] = y; fv4[2] = z;
    fv4[3] = __glOne;
    __glRasterPos4fv(gc, fv4);
}

GLvoid APIENTRY __glim_RasterPos3sv(__GLcontext* gc, const GLshort *v)
{
    GLfloat fv4[4];

    fv4[0] = v[0]; fv4[1] = v[1]; fv4[2] = v[2];
    fv4[3] = __glOne;
    __glRasterPos4fv(gc, fv4);
}

GLvoid APIENTRY __glim_RasterPos4dv(__GLcontext* gc, const GLdouble *v)
{
    GLfloat fv4[4];

    fv4[0] = v[0]; fv4[1] = v[1]; fv4[2] = v[2]; fv4[3] = v[3];
    __glRasterPos4fv(gc, fv4);
}

GLvoid APIENTRY __glim_RasterPos4fv(__GLcontext* gc, const GLfloat *v)
{
    __glRasterPos4fv(gc, (GLfloat *)v);
}

GLvoid APIENTRY __glim_RasterPos4d(__GLcontext* gc, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    GLfloat fv4[4];

    fv4[0] = x; fv4[1] = y; fv4[2] = z; fv4[3] = w;
    __glRasterPos4fv(gc, fv4);
}

GLvoid APIENTRY __glim_RasterPos4f(__GLcontext* gc, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    GLfloat fv4[4];

    fv4[0] = x; fv4[1] = y; fv4[2] = z; fv4[3] = w;
    __glRasterPos4fv(gc, fv4);
}

GLvoid APIENTRY __glim_RasterPos4i(__GLcontext* gc, GLint x, GLint y, GLint z, GLint w)
{
    GLfloat fv4[4];

    fv4[0] = x; fv4[1] = y; fv4[2] = z; fv4[3] = w;
    __glRasterPos4fv(gc, fv4);
}

GLvoid APIENTRY __glim_RasterPos4iv(__GLcontext* gc, const GLint *v)
{
    GLfloat fv4[4];

    fv4[0] = v[0]; fv4[1] = v[1]; fv4[2] = v[2]; fv4[3] = v[3];
    __glRasterPos4fv(gc, fv4);
}

GLvoid APIENTRY __glim_RasterPos4s(__GLcontext* gc, GLshort x, GLshort y, GLshort z, GLshort w)
{
    GLfloat fv4[4];

    fv4[0] = x; fv4[1] = y; fv4[2] = z; fv4[3] = w;
    __glRasterPos4fv(gc, fv4);
}

GLvoid APIENTRY __glim_RasterPos4sv(__GLcontext* gc, const GLshort *v)
{
    GLfloat fv4[4];

    fv4[0] = v[0]; fv4[1] = v[1]; fv4[2] = v[2]; fv4[3] = v[3];
    __glRasterPos4fv(gc, fv4);
}

/*
** Function to valid window position from inputs of glWindowPos*.
** Set valid result to gc raster pos state.
*/

__GL_INLINE GLvoid __glWindowPos3fv(__GLcontext* gc, GLfloat *v)
{
    __GLRasterPosState *rp;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    if(gc->input.deferredAttribDirty & __GL_DEFERED_COLOR_BIT)
    {
        __glCopyDeferedAttribToCurrent(gc);
    }

    /*Window coordinate*/
    rp = &gc->state.rasterPos;
    rp->rPos.winPos.f.x = v[0];
    rp->rPos.winPos.f.y = v[1];

    if (v[2] <= 0)
        rp->rPos.winPos.f.z = gc->state.viewport.zNear;
    else if (v[2] >= 1)
        rp->rPos.winPos.f.z = gc->state.viewport.zFar;
    else
        rp->rPos.winPos.f.z = gc->state.viewport.zNear*(1.0f-v[2]) + v[2]*gc->state.viewport.zFar;

    rp->rPos.winPos.f.w = 1.0f;

    /*CURRENT_RASTER_DISTANCE*/
    if (gc->state.fog.coordSource == GL_FOG_COORDINATE_SOURCE)
        rp->rPos.eyeDistance = gc->state.current.fog;
    else
        rp->rPos.eyeDistance = 0;

    /*Raster color = current color or index*/
    if (gc->drawablePrivate->modes.rgbMode)
    {
        rp->rPos.color[__GL_PRIMARY_COLOR]->r = gc->state.current.color.r;
        rp->rPos.color[__GL_PRIMARY_COLOR]->g = gc->state.current.color.g;
        rp->rPos.color[__GL_PRIMARY_COLOR]->b = gc->state.current.color.b;
        rp->rPos.color[__GL_PRIMARY_COLOR]->a = gc->state.current.color.a;
    }
    else
        rp->rPos.colorIndex = gc->state.current.colorIndex;

    /*Secondary color*/
    rp->rPos.color[__GL_SECONDARY_COLOR]->r = gc->state.current.color2.r;
    rp->rPos.color[__GL_SECONDARY_COLOR]->g = gc->state.current.color2.g;
    rp->rPos.color[__GL_SECONDARY_COLOR]->b = gc->state.current.color2.b;
    rp->rPos.color[__GL_SECONDARY_COLOR]->a = gc->state.current.color2.a;

    /* Raster texcoord = current texcoord */
    {
        GLint t;
        for (t = 0; t < __GL_MAX_TEXTURE_COORDS; t++) {
            rp->rPos.texture[t].v[0] = gc->state.current.texture[t].v[0];
            rp->rPos.texture[t].v[1] = gc->state.current.texture[t].v[1];
            rp->rPos.texture[t].v[2] = gc->state.current.texture[t].v[2];
            rp->rPos.texture[t].v[3] = gc->state.current.texture[t].v[3];
        }
    }

    /*Mark as valid raster pos*/
    rp->validRasterPos = GL_TRUE;

    if (gc->renderMode == GL_SELECT)
    {
        __glSelectPoint(gc, &rp->rPos);
    }
}

/* OpenGL window position APIs */

GLvoid APIENTRY __glim_WindowPos3fv(__GLcontext* gc, const GLfloat *v)
{
    __glWindowPos3fv(gc, (GLfloat *)v);
}

GLvoid APIENTRY __glim_WindowPos2d(__GLcontext* gc, GLdouble x, GLdouble y)
{
    GLfloat fv3[3];

    fv3[0] = x; fv3[1] = y; fv3[2] = __glZero;
    __glWindowPos3fv(gc, fv3);
}

GLvoid APIENTRY __glim_WindowPos2dv(__GLcontext* gc, const GLdouble *v)
{
    GLfloat fv3[3];

    fv3[0] = v[0]; fv3[1] = v[1]; fv3[2] = __glZero;
    __glWindowPos3fv(gc, fv3);
}

GLvoid APIENTRY __glim_WindowPos2f(__GLcontext* gc, GLfloat x, GLfloat y)
{
    GLfloat fv3[3];

    fv3[0] = x; fv3[1] = y; fv3[2] = __glZero;
    __glWindowPos3fv(gc, fv3);
}

GLvoid APIENTRY __glim_WindowPos2fv(__GLcontext* gc, const GLfloat *v)
{
    GLfloat fv3[3];

    fv3[0] = v[0]; fv3[1] = v[1]; fv3[2] = __glZero;
    __glWindowPos3fv(gc, fv3);
}

GLvoid APIENTRY __glim_WindowPos2i(__GLcontext* gc, GLint x, GLint y)
{
    GLfloat fv3[3];

    fv3[0] = x; fv3[1] = y; fv3[2] = __glZero;
    __glWindowPos3fv(gc, fv3);
}

GLvoid APIENTRY __glim_WindowPos2iv(__GLcontext* gc, const GLint *v)
{
    GLfloat fv3[3];

    fv3[0] = v[0]; fv3[1] = v[1]; fv3[2] = __glZero;
    __glWindowPos3fv(gc, fv3);
}

GLvoid APIENTRY __glim_WindowPos2s(__GLcontext* gc, GLshort x, GLshort y)
{
    GLfloat fv3[3];

    fv3[0] = x; fv3[1] = y; fv3[2] = __glZero;
    __glWindowPos3fv(gc, fv3);
}

GLvoid APIENTRY __glim_WindowPos2sv(__GLcontext* gc, const GLshort *v)
{
    GLfloat fv3[3];

    fv3[0] = v[0]; fv3[1] = v[1]; fv3[2] = __glZero;
    __glWindowPos3fv(gc, fv3);
}

GLvoid APIENTRY __glim_WindowPos3d(__GLcontext* gc, GLdouble x, GLdouble y, GLdouble z)
{
    GLfloat fv3[3];

    fv3[0] = x; fv3[1] = y; fv3[2] = z;
    __glWindowPos3fv(gc, fv3);
}

GLvoid APIENTRY __glim_WindowPos3dv(__GLcontext* gc, const GLdouble * v)
{
    GLfloat fv3[3];


    fv3[0] = v[0]; fv3[1] = v[1]; fv3[2] = v[2];
    __glWindowPos3fv(gc, fv3);
}

GLvoid APIENTRY __glim_WindowPos3f(__GLcontext* gc, GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat fv3[3];

    fv3[0] = x; fv3[1] = y; fv3[2] = z;
    __glWindowPos3fv(gc, fv3);
}

GLvoid APIENTRY __glim_WindowPos3i(__GLcontext* gc, GLint x, GLint y, GLint z)
{
    GLfloat fv3[3];

    fv3[0] = x; fv3[1] = y; fv3[2] = z;
    __glWindowPos3fv(gc, fv3);
}

GLvoid APIENTRY __glim_WindowPos3iv(__GLcontext* gc, const GLint *v)
{
    GLfloat fv3[3];

    fv3[0] = v[0]; fv3[1] = v[1]; fv3[2] = v[2];
    __glWindowPos3fv(gc, fv3);
}

GLvoid APIENTRY __glim_WindowPos3s(__GLcontext* gc, GLshort x, GLshort y, GLshort z)
{
    GLfloat fv3[3];

    fv3[0] = x; fv3[1] = y; fv3[2] = z;
    __glWindowPos3fv(gc, fv3);
}

GLvoid APIENTRY __glim_WindowPos3sv(__GLcontext* gc, const GLshort *v)
{
    GLfloat fv3[3];

    fv3[0] = v[0]; fv3[1] = v[1]; fv3[2] = v[2];
    __glWindowPos3fv(gc, fv3);
}

#if defined(_WIN32)
#pragma warning(default: 4244)
#endif


