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

extern GLfloat __glClampWidth(GLfloat width, __GLdeviceConstants *constants);

GLvoid __glInitCurrentState(__GLcontext *gc)
{
    gc->state.current.color.r = 1.0;
    gc->state.current.color.g = 1.0;
    gc->state.current.color.b = 1.0;
    gc->state.current.color.a = 1.0;
    gc->state.current.colorIndex = 1.0;

    gc->state.current.color2.r = 0.0;
    gc->state.current.color2.g = 0.0;
    gc->state.current.color2.b = 0.0;
    gc->state.current.color2.a = 1.0;
}

GLvoid __glInitHintState(__GLcontext *gc)
{
    __GLhintState *hs = &gc->state.hints;

    hs->perspectiveCorrection = GL_DONT_CARE;
    hs->pointSmooth = GL_DONT_CARE;
    hs->lineSmooth = GL_DONT_CARE;
    hs->polygonSmooth = GL_DONT_CARE;
    hs->fog = GL_DONT_CARE;
    hs->generateMipmap = GL_DONT_CARE;
    hs->textureCompressionHint = GL_DONT_CARE;
}

GLvoid __glInitRasterState(__GLcontext *gc)
{
    __GLrasterState *fs = &gc->state.raster;
    GLint i;

    fs->alphaFunction = GL_ALWAYS;
    fs->alphaReference = 0;
    fs->blendSrcRGB = GL_ONE;
    fs->blendSrcAlpha = GL_ONE;
    fs->blendDstRGB = GL_ZERO;
    fs->blendDstAlpha = GL_ZERO;
    fs->blendEquationRGB = GL_FUNC_ADD;
    fs->blendEquationAlpha = GL_FUNC_ADD;
    fs->blendColor.r = 0.0;
    fs->blendColor.g = 0.0;
    fs->blendColor.b = 0.0;
    fs->blendColor.a = 0.0;
    fs->logicOp = GL_COPY;
    fs->clear.r = 0.0;
    fs->clear.g = 0.0;
    fs->clear.b = 0.0;
    fs->clear.a = 0.0;
    fs->clampFragColor = GL_FIXED_ONLY_ARB;
    fs->clampReadColor = GL_FIXED_ONLY_ARB;
    fs->mrtEnable = GL_FALSE;

    for(i = 0; i < __GL_MAX_DRAW_BUFFERS; i++)
    {
        fs->colorMask[i].redMask = GL_TRUE;
        fs->colorMask[i].greenMask = GL_TRUE;
        fs->colorMask[i].blueMask = GL_TRUE;
        fs->colorMask[i].alphaMask = GL_TRUE;
    }

    if (gc->modes.doubleBufferMode) {
        gc->flags &= ~__GL_DRAW_TO_FRONT;
        fs->drawBuffer[0] = GL_BACK;
    } else {
        gc->flags |= __GL_DRAW_TO_FRONT;
        fs->drawBuffer[0] = GL_FRONT;
    }
    fs->drawBufferReturn = fs->drawBuffer[0];

    if (!gc->modes.rgbMode) {
        fs->writeMask = gc->modes.redMask;
    }

    gc->state.enables.colorBuffer.dither = GL_TRUE;

    gc->state.rasterPos.rPos.winPos.x = 0.0;
    gc->state.rasterPos.rPos.winPos.y = 0.0;
    gc->state.rasterPos.rPos.winPos.z = 0.0;

    /* Initialize primary color */
    gc->state.rasterPos.rPos.color[__GL_PRIMARY_COLOR]
            = &gc->state.rasterPos.rPos.colors[__GL_FRONTFACE];
    gc->state.rasterPos.rPos.colors[__GL_FRONTFACE].r = 1.0;
    gc->state.rasterPos.rPos.colors[__GL_FRONTFACE].g = 1.0;
    gc->state.rasterPos.rPos.colors[__GL_FRONTFACE].b = 1.0;
    gc->state.rasterPos.rPos.colors[__GL_FRONTFACE].a = 1.0;
    gc->state.rasterPos.rPos.colors[__GL_BACKFACE].r = 1.0;
    gc->state.rasterPos.rPos.colors[__GL_BACKFACE].g = 1.0;
    gc->state.rasterPos.rPos.colors[__GL_BACKFACE].b = 1.0;
    gc->state.rasterPos.rPos.colors[__GL_BACKFACE].a = 1.0;

    /* Initialize secondary color */
    gc->state.rasterPos.rPos.color[__GL_SECONDARY_COLOR]
            = &gc->state.rasterPos.rPos.colors2[__GL_FRONTFACE];
    gc->state.rasterPos.rPos.colors2[__GL_FRONTFACE].r = 0.0;
    gc->state.rasterPos.rPos.colors2[__GL_FRONTFACE].g = 0.0;
    gc->state.rasterPos.rPos.colors2[__GL_FRONTFACE].b = 0.0;
    gc->state.rasterPos.rPos.colors2[__GL_FRONTFACE].a = 1.0;
    gc->state.rasterPos.rPos.colors2[__GL_BACKFACE].r = 0.0;
    gc->state.rasterPos.rPos.colors2[__GL_BACKFACE].g = 0.0;
    gc->state.rasterPos.rPos.colors2[__GL_BACKFACE].b = 0.0;
    gc->state.rasterPos.rPos.colors2[__GL_BACKFACE].a = 1.0;

    for (i = 0; i < __GL_MAX_TEXTURE_BINDINGS; i++) {
        gc->state.rasterPos.rPos.texture[i].s = 0.0;
        gc->state.rasterPos.rPos.texture[i].t = 0.0;
        gc->state.rasterPos.rPos.texture[i].r = 0.0;
        gc->state.rasterPos.rPos.texture[i].q = 1.0;
    }
    gc->state.rasterPos.rPos.clipW = 1.0;
    gc->state.rasterPos.validRasterPos = GL_TRUE;
    gc->state.rasterPos.rPos.eyeDistance = 1.0;
}

GLvoid __glInitStencilState(__GLcontext *gc)
{
    __GLstencilFuncState *ssCur = &gc->state.stencil.current;
    __GLstencilFuncState *ssArb = &gc->state.stencil.StencilArb;
    __GLstencilFuncState *ssExt = &gc->state.stencil.stencilExt;

    ssArb->front.testFunc = GL_ALWAYS;
    ssArb->front.fail = GL_KEEP;
    ssArb->front.depthFail = GL_KEEP;
    ssArb->front.depthPass = GL_KEEP;
    ssArb->front.mask = 0xffffffff;
    ssArb->front.reference = 0;
    ssArb->front.writeMask = 0xffffffff;

    ssArb->back.testFunc = GL_ALWAYS;
    ssArb->back.fail = GL_KEEP;
    ssArb->back.depthFail = GL_KEEP;
    ssArb->back.depthPass = GL_KEEP;
    ssArb->back.mask = 0xffffffff;
    ssArb->back.reference = 0;
    ssArb->back.writeMask = 0xffffffff;

    gc->state.stencil.clear = 0;

    /* GL_EXT_stencil_two_side's state has the same initial value */
    __GL_MEMCOPY(ssExt, ssArb, sizeof(__GLstencilFuncState));
    __GL_MEMCOPY(ssCur, ssArb, sizeof(__GLstencilFuncState));

    gc->state.stencil.activeStencilFace = GL_FRONT;
}

GLvoid __glInitDepthState(__GLcontext *gc)
{
    __GLdepthState *ds = &gc->state.depth;

    ds->writeEnable = GL_TRUE;
    ds->testFunc = GL_LESS;
    ds->clear = 1.0;
#if GL_EXT_depth_bounds_test
    gc->state.depthBoundTest.zMin = 0.0;
    gc->state.depthBoundTest.zMax = 1.0;
#endif
}

GLvoid __glInitPointState(__GLcontext *gc)
{
    __GLpointState *ps = &gc->state.point;

    ps->requestedSize = 1.0;
    ps->sizeMin = 0.0;
    ps->sizeMax = 100.0;
    ps->fadeThresholdSize = 1.0;
    ps->distanceAttenuation[0] = 1.0;
    ps->distanceAttenuation[1] = 0.0;
    ps->distanceAttenuation[2] = 0.0;
    ps->coordOrigin = GL_UPPER_LEFT;
}

GLvoid __glInitLineState(__GLcontext *gc)
{
    __GLlineState *ls = &gc->state.line;

    ls->requestedWidth = 1.0;
    ls->smoothWidth = __glClampWidth(1.0, &gc->constants);
    ls->aliasedWidth = 1;
    ls->stipple = 0xFFFF;
    ls->stippleRepeat = 1;

    gc->state.enables.line.stippleRequested = GL_FALSE;
}

GLvoid __glInitPolygonState(__GLcontext *gc)
{
    __GLpolygonState *ps = &gc->state.polygon;
    __GLpolygonStippleState *stp = &gc->state.polygonStipple;
    GLint i;

    ps->frontMode = GL_FILL;
    ps->backMode = GL_FILL;
    ps->bothFaceFill = GL_TRUE;
    ps->cullFace = GL_BACK;
    ps->frontFace= GL_CCW;
    ps->factor = 0.0;
    ps->units   = 0.0;
    ps->bias   = 0.0;
    gc->state.current.edgeflag = GL_TRUE;

    for (i = 0; i < 4*32; i++) {
        stp->stipple[i] = 0xFF;
    }
}

GLvoid __glInitAccumState(__GLcontext *gc)
{
    gc->state.accum.clear.r = 0;
    gc->state.accum.clear.g = 0;
    gc->state.accum.clear.b = 0;
    gc->state.accum.clear.a = 0;
}

GLvoid __glInitMultisampleState(__GLcontext *gc)
{
    gc->state.multisample.coverageValue = 1.0;
    gc->state.multisample.coverageInvert = GL_FALSE;
    gc->state.enables.multisample.multisampleOn = GL_TRUE;
    gc->state.enables.multisample.alphaToCoverage = GL_FALSE;
    gc->state.enables.multisample.alphaToOne = GL_FALSE;
    gc->state.enables.multisample.coverage = GL_FALSE;
}

