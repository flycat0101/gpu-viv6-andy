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
#include "chip_context.h"

#define _GC_OBJ_ZONE    gcvZONE_API_GL

GLvoid initAccumOperationPatch(__GLcontext* gc, glsCHIPDRAWABLE * chipDraw)
{
    glsCHIPCONTEXT_PTR chipCtx = (glsCHIPCONTEXT_PTR)(gc->dp.ctx.privateData);
    __GLdrawablePrivate* draw = gc->drawablePrivate;
    glsCHIPACCUMBUFFER * accumBuffer = chipDraw->accumBuffer;
    glsTEXTUREINFO * textureInfo;
    glsTEXTURESAMPLER * sampler;
    gceSTATUS status = gcvSTATUS_OK;

    gcoTEXTURE_InitParams(chipCtx->hal, &accumBuffer->texture[1]);
    accumBuffer->texture[1].mipFilter = gcvTEXTURE_NONE;
    accumBuffer->texture[0] = accumBuffer->texture[1];

    textureInfo = &accumBuffer->textureInfo[1];
    status = gcoTEXTURE_ConstructEx(chipCtx->hal, gcvTEXTURE_2D, &textureInfo->object);
    if (gcmIS_ERROR(status)) {
        return;
    }

    textureInfo->imageFormat = textureInfo->residentFormat = (*chipDraw->drawBuffers[0])->renderTargetFormat;
    textureInfo->residentLevels = 1;
    textureInfo->combineFlow.targetEnable = gcSL_ENABLE_XYZ;
    textureInfo->combineFlow.tempEnable   = gcSL_ENABLE_XYZ;
    textureInfo->combineFlow.tempSwizzle  = gcSL_SWIZZLE_XYZZ;
    textureInfo->combineFlow.argSwizzle   = gcSL_SWIZZLE_XYZZ;
    textureInfo->format = GL_LUMINANCE;

    status = gcoTEXTURE_AddMipMap(
           textureInfo->object,
           0,
           gcvUNKNOWN_MIPMAP_IMAGE_FORMAT,
           textureInfo->residentFormat,
           draw->width,
           draw->height,
           0,
           0,
           gcvPOOL_DEFAULT,
           gcvNULL
    );

    if (gcmIS_ERROR(status)) {
        return;
    }

    sampler = &accumBuffer->sampler[1];
    sampler->binding = textureInfo;
    sampler->genEnable = 0;
    sampler->coordType    = gcSHADER_FLOAT_X2;
    sampler->coordSwizzle = gcSL_SWIZZLE_XYYY;

    accumBuffer->sampler[0] = accumBuffer->sampler[1];
    sampler = &accumBuffer->sampler[0];
    sampler->binding = &accumBuffer->textureInfo[0];

    textureInfo = &accumBuffer->textureInfo[0];
    textureInfo->imageFormat = textureInfo->residentFormat = accumBuffer->renderTargetFormat;
    textureInfo->object = gcvNULL;

    status = gcoTEXTURE_ConstructEx(chipCtx->hal, gcvTEXTURE_2D, &textureInfo->object);
    if (gcmIS_ERROR(status)) {
        return;
    }
    status = gcoTEXTURE_AddMipMap(
           textureInfo->object,
           0,
           gcvUNKNOWN_MIPMAP_IMAGE_FORMAT,
           textureInfo->residentFormat,
           draw->width,
           draw->height,
           0,
           0,
           gcvPOOL_DEFAULT,
           gcvNULL
    );

    if (gcmIS_ERROR(status)) {
        return;
    }
}


/* Save current states */
GLvoid saveAttributes(__GLcontext* gc, glsCHIPCONTEXT_PTR chipCtx)
{
    __GLdispatchTable *pDispatchTable = &gc->currentImmediateTable->dispatch;

    /* Save current states affected by push/pop */
    pDispatchTable->PushAttrib(GL_CURRENT_BIT  | GL_POLYGON_BIT        | GL_POLYGON_STIPPLE_BIT |
                               GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT   | GL_DEPTH_BUFFER_BIT |
                               GL_FOG_BIT      | GL_STENCIL_BUFFER_BIT | GL_ENABLE_BIT |
                               GL_ALL_ATTRIB_BITS);

    /* Save current multi sample enable */
    chipCtx->multiSampleOn = gc->state.enables.multisample.multisampleOn;

    /* Save current error code */
    chipCtx->errorNo = gc->error;
}


/* Reset current states to default states for manual draw */
GLvoid resetAttributes(__GLcontext* gc, glsCHIPCONTEXT_PTR chipCtx)
{
    __GLdispatchTable *pDispatchTable = &gc->currentImmediateTable->dispatch;

    /* Push Matrix*/
    pDispatchTable->MatrixMode(GL_MODELVIEW);
    pDispatchTable->PushMatrix();
    pDispatchTable->MatrixMode(GL_TEXTURE);
    pDispatchTable->PushMatrix();
    pDispatchTable->MatrixMode(GL_PROJECTION);
    pDispatchTable->PushMatrix();


    pDispatchTable->MatrixMode(GL_MODELVIEW);
    pDispatchTable->LoadIdentity();
    pDispatchTable->MatrixMode(GL_TEXTURE);
    pDispatchTable->LoadIdentity();
    pDispatchTable->MatrixMode(GL_PROJECTION);
    pDispatchTable->LoadIdentity();

    /* Reset most of the states */
    pDispatchTable->Disable(GL_LIGHTING);
    pDispatchTable->Disable(GL_CULL_FACE);
    pDispatchTable->Disable(GL_POLYGON_STIPPLE);
    pDispatchTable->Disable(GL_POLYGON_OFFSET_FILL);
    pDispatchTable->Disable(GL_ALPHA_TEST);
    pDispatchTable->Disable(GL_BLEND);
    pDispatchTable->Disable(GL_LOGIC_OP);
    pDispatchTable->Disable(GL_STENCIL_TEST);
    pDispatchTable->Disable(GL_DEPTH_TEST);
    pDispatchTable->Disable(GL_SCISSOR_TEST);
    pDispatchTable->DepthMask(GL_FALSE);
    pDispatchTable->Disable(GL_VERTEX_PROGRAM_ARB);
    pDispatchTable->Disable(GL_FRAGMENT_PROGRAM_ARB);
    pDispatchTable->Disable(GL_FOG);
    pDispatchTable->PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    pDispatchTable->Viewport(0, 0, gc->drawablePrivate->width, gc->drawablePrivate->height);
    pDispatchTable->DepthRange(0,1);
    pDispatchTable->Disable(GL_CLIP_PLANE0);
    pDispatchTable->Disable(GL_CLIP_PLANE1);
    pDispatchTable->Disable(GL_CLIP_PLANE2);
    pDispatchTable->Disable(GL_CLIP_PLANE3);
    pDispatchTable->Disable(GL_CLIP_PLANE4);
    pDispatchTable->Disable(GL_CLIP_PLANE5);
    pDispatchTable->ActiveTexture(GL_TEXTURE0);
    pDispatchTable->Enable(GL_TEXTURE_2D);
    pDispatchTable->ActiveTexture(GL_TEXTURE1);
    pDispatchTable->Enable(GL_TEXTURE_2D);

    /* Disable MSAA */
    pDispatchTable->Disable(GL_MULTISAMPLE);

    /*Disable glsl path*/
    pDispatchTable->UseProgram(0);
    /* Clear error code */
    gc->error = 0;
}

/* Restore current states according to last save result */
GLvoid restoreAttributes(__GLcontext* gc, glsCHIPCONTEXT_PTR chipCtx)
{
    __GLdispatchTable *pDispatchTable = &gc->currentImmediateTable->dispatch;

    /* Restore previous pushed states */
    pDispatchTable->PopAttrib();

    /* Restore previous MSAA enable or disable */
    if(chipCtx->multiSampleOn)
    {
        pDispatchTable->Enable(GL_MULTISAMPLE);
    }
    else
    {
        pDispatchTable->Disable(GL_MULTISAMPLE);
    }

    /* Restore previous error code */
    gc->error = chipCtx->errorNo;
}

static GLvoid checkPixelTransferAttrib(__GLcontext *gc, glsCHIPCONTEXT_PTR chipCtx)
{

    if ((gc->state.pixel.transferMode.r_scale != 1.0f) ||
        (gc->state.pixel.transferMode.g_scale != 1.0f) ||
        (gc->state.pixel.transferMode.b_scale != 1.0f) ||
        (gc->state.pixel.transferMode.a_scale != 1.0f))
    {
        /* pixel transfer scale enabled */
        glmSETHASH_1BIT(hashPixelTransfer, 1, gccPIXEL_TRANSFER_SCALE);
    }

    if ((gc->state.pixel.transferMode.r_bias != 0.0f) ||
        (gc->state.pixel.transferMode.g_bias != 0.0f) ||
        (gc->state.pixel.transferMode.b_bias != 0.0f) ||
        (gc->state.pixel.transferMode.a_bias != 0.0f))
    {
        /* pixel transfer scale enabled */
        glmSETHASH_1BIT(hashPixelTransfer, 1, gccPIXEL_TRANSFER_BIAS);
    }
}

GLboolean simulatePixelOperation(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height,
                                 GLenum format, GLenum type, const GLvoid *pixels, GLboolean bDrawPixels)
{
    GLuint texture;
    GLenum internalFormat;
    GLint  oldError = gc->error;
    GLuint texStage = __GL_MAX_TEXTURE_COORDS - 1;
    __GLcoord vertex = gc->state.rasterPos.rPos.winPos;
    __GLcolor color = gc->state.rasterPos.rPos.colors[__GL_FRONTFACE];
    __GLcolor secondaryColor = gc->state.rasterPos.rPos.colors2[__GL_FRONTFACE];
    __GLdispatchTable *pDispatchTable = &gc->currentImmediateTable->dispatch;
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);

    if (gc->renderMode != GL_RENDER  || format == GL_STENCIL_INDEX)
    {
        return GL_FALSE;
    }

    /*
    ** The spec says: when writing to the stencil buffer, "If a depth component is present,
    ** and the setting of DepthMask is not FALSE, is also written to the framebuffer; the
    ** setting of DepthTest is ignored."
    ** The simulation path cannot handle the case.
    */
    if (format == GL_DEPTH_COMPONENT)
    {
        return GL_FALSE;
    }

    /* The simulation need to use the last texture unit. But if it was already been used, return FALSE */
    if (gc->state.enables.texUnits[texStage].enabledDimension > 0 )
    {
        return GL_FALSE;
    }

    /* If any of the shader stage was enabled, */
    if (gc->state.enables.program.vertexProgram || gc->state.enables.program.fragmentProgram ||
        gc->shaderProgram.vertShaderEnable || gc->shaderProgram.fragShaderEnable || gc->shaderProgram.geomShaderEnable)
    {
        return GL_FALSE;
    }

    gc->error = 0;
    if(gc->drawablePrivate->yInverted)
    {
        vertex.y = gc->drawablePrivate->height - vertex.y;
    }

    /* Push state */
    pDispatchTable->PushAttrib(GL_CURRENT_BIT | GL_LIGHTING_BIT | GL_POLYGON_BIT | GL_POLYGON_STIPPLE_BIT |
                               GL_TEXTURE_BIT | GL_TRANSFORM_BIT | GL_VIEWPORT_BIT | GL_ENABLE_BIT);

    pDispatchTable->ActiveTexture(GL_TEXTURE0 + texStage);

    /* Push Matrix*/
    pDispatchTable->MatrixMode(GL_MODELVIEW);
    pDispatchTable->PushMatrix();
    pDispatchTable->MatrixMode(GL_TEXTURE);
    pDispatchTable->PushMatrix();
    pDispatchTable->MatrixMode(GL_PROJECTION);
    pDispatchTable->PushMatrix();

    /* State setting */
    pDispatchTable->Disable(GL_LIGHTING);/*Disable lighting*/
    pDispatchTable->Disable(GL_CULL_FACE);
    pDispatchTable->Disable(GL_POLYGON_STIPPLE);
    pDispatchTable->PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    pDispatchTable->Disable(GL_CLIP_PLANE0);
    pDispatchTable->Disable(GL_CLIP_PLANE1);
    pDispatchTable->Disable(GL_CLIP_PLANE2);
    pDispatchTable->Disable(GL_CLIP_PLANE3);
    pDispatchTable->Disable(GL_CLIP_PLANE4);
    pDispatchTable->Disable(GL_CLIP_PLANE5);

    /* Texture initialization */
    pDispatchTable->GenTextures(1, &texture);   /* Create a texture*/
    pDispatchTable->BindTexture(GL_TEXTURE_2D, texture);
    pDispatchTable->Enable(GL_TEXTURE_2D);      /* Enable texture */
    pDispatchTable->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    pDispatchTable->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    pDispatchTable->TexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    pDispatchTable->TexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 1);
    pDispatchTable->TexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1);
    pDispatchTable->Disable(GL_TEXTURE_GEN_S);
    pDispatchTable->Disable(GL_TEXTURE_GEN_T);
    pDispatchTable->Disable(GL_TEXTURE_GEN_R);
    pDispatchTable->Disable(GL_TEXTURE_GEN_Q);

    chipCtx->hashKey.hashPixelTransfer = 0;
    checkPixelTransferAttrib(gc, chipCtx);

    if (format == GL_DEPTH_COMPONENT)
    {
        gc->texture.drawDepthStage = texStage;

        /* For depth component, the current color should be set to the current raster color*/
        pDispatchTable->Color4f(color.r, color.g, color.b, color.a);
        pDispatchTable->SecondaryColor3f(secondaryColor.r, secondaryColor.g, secondaryColor.b);
    }
    else
    {
        /*
        ** For color component, the current color may be used in the texture blending:
        ** when format == GL_ALPHA( no RGB) or format = GL_RGB( no Alpha).  so the
        ** current color should be set to (0,0,0,1) in order to do correct simulation
        */
        pDispatchTable->Color4f(0.0, 0.0, 0.0, 1.0);
    }

    switch (format)
    {
    /*
    ** When format is one of color formats, we need the internalFormat include all
    ** color channels for DrawPixel simulation path. Because Simulation will load
    ** the source channel image to a texture and draw with texenv replace.
    ** the result will be Channel= texture.Channel,means the channels
    ** not included will not changed. But, with glDrawPixels pipe, the dest color
    ** should be set to default value(0,0,0,1).
    */
    case GL_STENCIL_INDEX:
    case GL_DEPTH_COMPONENT:
        internalFormat = format;
        break;
    default:
        if(gc->modes.rgbFloatMode)
        {
            internalFormat = GL_RGBA32F_ARB;
        }
        else
        {
            internalFormat = GL_RGBA;
        }
        break;
    }

    if (bDrawPixels)
    {
        gctUINT8* mappedPixels = gcvNULL;
        gctUINT8 index, clampIndex;
        gctUINT32 i;

        if ((format == GL_COLOR_INDEX) && (type == GL_UNSIGNED_BYTE))
        {
            /* map the color */
            if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL,
                                           width * height * 4,
                                           (gctPOINTER*)&mappedPixels)))
            {
                return GL_FALSE;
            }


            for (i = 0; i < width * height; i++)
            {
                index = ((gctUINT8*)pixels)[i];

                clampIndex = gcmMIN(gc->state.pixel.pixelMap[GL_PIXEL_MAP_I_TO_R - GL_PIXEL_MAP_I_TO_I].size, index);
                mappedPixels[0 + i * 4] = \
                    (gctUINT8)(gc->state.pixel.pixelMap[GL_PIXEL_MAP_I_TO_R - GL_PIXEL_MAP_I_TO_I].base.mapF[clampIndex] * 255);

                clampIndex = gcmMIN(gc->state.pixel.pixelMap[GL_PIXEL_MAP_I_TO_G - GL_PIXEL_MAP_I_TO_I].size, index);
                mappedPixels[1 + i * 4] = \
                    (gctUINT8)(gc->state.pixel.pixelMap[GL_PIXEL_MAP_I_TO_G - GL_PIXEL_MAP_I_TO_I].base.mapF[clampIndex] * 255);

                clampIndex = gcmMIN(gc->state.pixel.pixelMap[GL_PIXEL_MAP_I_TO_B - GL_PIXEL_MAP_I_TO_I].size, index);
                mappedPixels[2 + i * 4] = \
                    (gctUINT8)(gc->state.pixel.pixelMap[GL_PIXEL_MAP_I_TO_B - GL_PIXEL_MAP_I_TO_I].base.mapF[clampIndex] * 255);

                clampIndex = gcmMIN(gc->state.pixel.pixelMap[GL_PIXEL_MAP_I_TO_A - GL_PIXEL_MAP_I_TO_I].size, index);
                mappedPixels[3 + i * 4] = \
                    (gctUINT8)(gc->state.pixel.pixelMap[GL_PIXEL_MAP_I_TO_A - GL_PIXEL_MAP_I_TO_I].base.mapF[clampIndex] * 255);
            }

            internalFormat = GL_RGBA;
            format = GL_RGBA;
            type = GL_UNSIGNED_BYTE;
            pixels = mappedPixels;
        }

        pDispatchTable->TexImage2D(GL_TEXTURE_2D, 0, internalFormat,
                                   width, height, 0, format, type, pixels);

        if (mappedPixels != gcvNULL)
        {
            gcoOS_Free(gcvNULL, mappedPixels);
        }

    }
    else
    {
        pDispatchTable->CopyTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
                                       x, y, width, height, 0);
    }

    /* Transformation Matrix */
    pDispatchTable->MatrixMode(GL_MODELVIEW);
    pDispatchTable->LoadIdentity();
    pDispatchTable->MatrixMode(GL_TEXTURE);
    pDispatchTable->LoadIdentity();
    pDispatchTable->MatrixMode(GL_PROJECTION);
    pDispatchTable->LoadIdentity();
    // The raster pos are already in screen space, its depth are already in range [0, 1]
    pDispatchTable->Ortho(0,gc->drawablePrivate->width, 0, gc->drawablePrivate->height, 0, -1);
    pDispatchTable->Viewport(0, 0, gc->drawablePrivate->width, gc->drawablePrivate->height);

    /* Depth Range */
    pDispatchTable->DepthRange(0,1);

    /* Draw a Quad*/
    /* Spec vertex with only xyz channel.*/
    pDispatchTable->Begin(GL_QUADS);
    pDispatchTable->MultiTexCoord2f(GL_TEXTURE0 + texStage, 0, 0 );
    pDispatchTable->Vertex3fv((GLfloat *)&vertex);

    vertex.y += height * gc->state.pixel.transferMode.zoomY;
    pDispatchTable->MultiTexCoord2f(GL_TEXTURE0 + texStage, 0, 1 );
    pDispatchTable->Vertex3fv((GLfloat *)&vertex);

    vertex.x += width * gc->state.pixel.transferMode.zoomX;
    pDispatchTable->MultiTexCoord2f(GL_TEXTURE0 + texStage, 1, 1 );
    pDispatchTable->Vertex3fv((GLfloat *)&vertex);

    vertex.y -= height * gc->state.pixel.transferMode.zoomY;
    pDispatchTable->MultiTexCoord2f(GL_TEXTURE0 + texStage, 1, 0 );
    pDispatchTable->Vertex3fv((GLfloat *)&vertex);
    pDispatchTable->End();

    pDispatchTable->Flush();

    /*Delete texture*/
    pDispatchTable->DeleteTextures(1, &texture);

    /* Pop Matrix*/
    pDispatchTable->MatrixMode(GL_MODELVIEW);
    pDispatchTable->PopMatrix();
    pDispatchTable->MatrixMode(GL_TEXTURE);
    pDispatchTable->PopMatrix();
    pDispatchTable->MatrixMode(GL_PROJECTION);
    pDispatchTable->PopMatrix();
    /*Pop State*/
    pDispatchTable->PopAttrib();

    gc->texture.drawDepthStage = -1;

    chipCtx->hashKey.hashPixelTransfer = 0;

    /* Do we have error?*/
    if (gc->error)
    {
        gc->error = oldError;
        GL_ASSERT(0);
        return GL_FALSE;
    }

    /* Restore the error value */
    gc->error = oldError;
    return GL_TRUE;
}

GLboolean
calculateArea(
    GLint *pdx, GLint *pdy,
    GLint *psx, GLint *psy,
    GLint *pw, GLint *ph,
    GLint dstW, GLint dstH,
    GLint srcW, GLint srcH
    )
{
    gctINT32 srcsx, srcex, dstsx, dstex;
    gctINT32 srcsy, srcey, dstsy, dstey;

    gctINT32 dx = *pdx, dy = *pdy, sx = *psx, sy = *psy, w = *pw, h = *ph;

    gcmHEADER_ARG("pdx=0x%x pdy=0x%x psx=0x%x psy=0x%x pw=0x%x ph=0x%x dstW=%d dstH=%d srcW=%d srcH=%d",
        pdx, pdy, psx, psy, pw, ph, dstW, dstH, srcW, srcH);

    sx = gcmMIN(gcmMAX(sx, (gctINT32)((gctINT32)__glMinInt>>2)), (gctINT32)((gctINT32)__glMaxInt>>2));
    sy = gcmMIN(gcmMAX(sy, (gctINT32)((gctINT32)__glMinInt>>2)), (gctINT32)((gctINT32)__glMaxInt>>2));
    dx = gcmMIN(gcmMAX(dx, (gctINT32)((gctINT32)__glMinInt>>2)), (gctINT32)((gctINT32)__glMaxInt>>2));
    dy = gcmMIN(gcmMAX(dy, (gctINT32)((gctINT32)__glMinInt>>2)), (gctINT32)((gctINT32)__glMaxInt>>2));
    w = gcmMIN(w, (gctINT32)((gctINT32)__glMaxInt>>2));
    h = gcmMIN(h, (gctINT32)((gctINT32)__glMaxInt>>2));

    srcsx = sx;
    srcex = sx + w;
    dstsx = dx;
    dstex = dx + w;

    if(srcsx < 0)
    {
        dstsx -= srcsx;
        srcsx = 0;
    }
    if(srcex > srcW)
    {
        dstex -= srcex - srcW;
        srcex = srcW;
    }
    if(dstsx < 0)
    {
        srcsx -= dstsx;
        dstsx = 0;
    }
    if(dstex > dstW)
    {
        srcex -= dstex - dstW;
        dstex = dstW;
    }

    gcmASSERT(srcsx >= 0 && dstsx >= 0 && srcex <= srcW && dstex <= dstW);
    w = srcex - srcsx;
    gcmASSERT(w == dstex - dstsx);

    if(w <= 0)
    {
        gcmFOOTER_ARG("return=%s", "FALSE");

        return gcvFALSE;
    }

    srcsy = sy;
    srcey = sy + h;
    dstsy = dy;
    dstey = dy + h;

    if(srcsy < 0)
    {
        dstsy -= srcsy;
        srcsy = 0;
    }
    if(srcey > srcH)
    {
        dstey -= srcey - srcH;
        srcey = srcH;
    }
    if(dstsy < 0)
    {
        srcsy -= dstsy;
        dstsy = 0;
    }
    if(dstey > dstH)
    {
        srcey -= dstey - dstH;
        dstey = dstH;
    }
    gcmASSERT(srcsy >= 0 && dstsy >= 0 && srcey <= srcH && dstey <= dstH);
    h = srcey - srcsy;
    gcmASSERT(h == dstey - dstsy);

    if(h <= 0)
    {
        gcmFOOTER_ARG("return=%s", "FALSE");

        return gcvFALSE;
    }

    *pdx = dstsx;
    *pdy = dstsy;
    *psx = srcsx;
    *psy = srcsy;
    *pw  = w;
    *ph  = h;

    gcmFOOTER_ARG("return=%s", "TRUE");

    return gcvTRUE;
}

/************************************************************************/
/* Implementation for internal functions                                */
/************************************************************************/

void __glChipRasterBegin(__GLcontext *gc, GLenum rasterOp,GLenum format, GLint width, GLint height)
{
    switch(rasterOp)
    {
    case __GL_RASTERFUNC_BITMAP:
        gc->pipeline = (__GLpipeline *)&(gc->dp.ctx);
        break;

    case __GL_RASTERFUNC_COPYPIX:
        gc->pipeline = (__GLpipeline *)&(gc->dp.ctx);
        break;

    case __GL_RASTERFUNC_READPIX:
        gc->pipeline = (__GLpipeline *)&(gc->dp.ctx);
        break;

    case __GL_RASTERFUNC_DRAWPIX:
        gc->pipeline = (__GLpipeline *)&(gc->dp.ctx);
        break;

    default:
        GL_ASSERT(0);
        break;
    }
}

void __glChipRasterEnd(__GLcontext *gc, GLenum rasterOp)
{
    if (gc->drawablePrivate->pbufferTex &&
         (rasterOp == __GL_RASTERFUNC_BITMAP ||
          rasterOp == __GL_RASTERFUNC_COPYPIX ||
          rasterOp == __GL_RASTERFUNC_DRAWPIX))
    {
        gc->drawablePrivate->pbufferTex->needGenMipmap = GL_TRUE;
    }
}

GLboolean __glChipDrawPixels(__GLcontext *gc, GLsizei width, GLsizei height, GLenum format, GLenum type, GLubyte *pixels)
{
    /*
    ** Current Vivante chip doesn't support drawpixels directly. So we simulated using texture draw for some cases.
    */

    if (gc->bufferObject.boundBuffer[__GL_PIXEL_UNPACK_BUFFER_INDEX] > 0)
    {
        return GL_FALSE;
    }

    return simulatePixelOperation(gc, 0, 0, width, height, format, type, pixels, GL_TRUE);
}

#define GET_SOURCE(s) \
    ((*s >> (srcShift)) & srcMax) \

#define CONVERT_FIXED_TO_FIXED(dstDataType, srcDataType) \
    *d = (dstDataType)__GL_FLOORF((( GET_SOURCE(s) / (gctFLOAT)(srcMax)) * (dstMax)) + 0.5) \

#define CONVERT_FIXED_TO_FLOAT(dstDataType, srcDataType) \
    *d = (dstDataType)(GET_SOURCE(s) / (gctFLOAT)(srcMax)) \

#define CONVERT_NONE(dstDataType, srcDataType) \
    *d = (dstDataType) GET_SOURCE(s) \

#define CONVERT_DEPTH_PIXELS(dstDataType, srcDataType, convertFunc) \
    do \
    { \
        srcDataType* s;\
        dstDataType* d;\
        for (j = 0; j < h; j++) \
        {\
            for (i = 0; i < w; i++) \
            {\
                s = ((srcDataType*)((gctUINT8*)srcData + (j + sy) * srcStride)) + (i + sx);\
                d = ((dstDataType*)((gctUINT8*)dstData + (j + dy) * dstStride)) + (i + dx);\
                convertFunc(dstDataType, srcDataType);\
            }\
        }\
    } while(gcvFALSE) \


gceSTATUS
readDepthStencilPixels(__GLcontext *gc,
                       GLint x, GLint y,
                       GLsizei width, GLsizei height,
                       GLenum format, GLenum type, GLubyte *buf)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gceSURF_FORMAT srcFormat;
    gctUINT32 srcWidth, srcHeight;
    GLint dx, dy, sx, sy, w, h, i, j;
    gctINT srcStride, dstStride;
    gctUINT32 srcShift, srcMax, dstMax;
    gctPOINTER srcData, dstData;
    gcsSURF_VIEW srcView = {chipCtx->readDepth, 0, 1};
    gcsSURF_VIEW tmpView = {gcvNULL, 0, 1};

    gceSTATUS status = gcvSTATUS_OK;

    if (srcView.surf == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        goto OnError;
    }

    gcmONERROR(gcoSURF_GetSize(srcView.surf,
                               &srcWidth,
                               &srcHeight,
                               gcvNULL));

    sx = x; sy = y; dx = 0; dy = 0; w = width; h = height;

    if (!calculateArea(&dx, &dy, &sx, &sy,
                        &w, &h, width, height,
                        srcWidth, srcHeight))
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        goto OnError;
    }

    {
        /* limitation : */
        /* 1. Enable FC, can not use CopyPixels directly */
        /* 2. can not resolve a depth surface to a user surface */
        gcsSURF_RESOLVE_ARGS rlvArgs = {0};

        gcmONERROR(gcoSURF_GetFormat(srcView.surf, gcvNULL, &srcFormat));

        gcmONERROR(gcoSURF_Construct(gcvNULL,
                                     w, h, 1,
                                     gcvSURF_BITMAP,
                                     srcFormat,
                                     gcvPOOL_DEFAULT,
                                     &tmpView.surf));

        rlvArgs.version = gcvHAL_ARG_VERSION_V2;
        rlvArgs.uArgs.v2.yInverted = gcvTRUE;
        rlvArgs.uArgs.v2.srcOrigin.x = sx;
        rlvArgs.uArgs.v2.srcOrigin.y = sy;
        rlvArgs.uArgs.v2.rectSize.x  = w;
        rlvArgs.uArgs.v2.rectSize.y  = h;
        rlvArgs.uArgs.v2.numSlices   = 1;
        gcmONERROR(gcoSURF_ResolveRect_v2(&srcView, &tmpView, &rlvArgs));

        gcmONERROR(gcoSURF_Flush(tmpView.surf));

        gcmONERROR(gcoHAL_Commit(gcvNULL, gcvTRUE));
    }

    gcmONERROR(gcoSURF_GetAlignedSize(
        tmpView.surf,
        gcvNULL,
        gcvNULL,
        &srcStride));

    gcmONERROR(gcoSURF_Lock(tmpView.surf, gcvNULL, &srcData));

    dstData = buf; sx = 0; sy = 0;

    switch (srcFormat)
    {
    case gcvSURF_D16:
        srcShift = 0;
        srcMax = (1 << 16) - 1;

        switch(type)
        {
        case GL_UNSIGNED_SHORT:
            dstMax = 0;
            dstStride = width * 2;
            CONVERT_DEPTH_PIXELS(gctUINT16, gctUINT16, CONVERT_NONE);
            break;
        case GL_UNSIGNED_BYTE:
            dstMax = (1 << 8) - 1;
            dstStride = width;
            CONVERT_DEPTH_PIXELS(gctUINT8, gctUINT16, CONVERT_FIXED_TO_FIXED);
            break;
        default : break;
        }
        break;

    case gcvSURF_D24S8:
        srcShift = (format == GL_DEPTH_COMPONENT) ? 8 : 0;
        srcMax = (format == GL_DEPTH_COMPONENT) ? ((1 << 24) - 1) : ((1 << 8) - 1);

        switch(type)
        {
        case GL_FLOAT:
            dstMax = 0;
            dstStride = width * 4;
            CONVERT_DEPTH_PIXELS(gctFLOAT, gctUINT32, CONVERT_FIXED_TO_FLOAT);
            break;
        case GL_UNSIGNED_SHORT:
            dstMax = (1 << 16) - 1;
            dstStride = width * 2;
            CONVERT_DEPTH_PIXELS(gctUINT16, gctUINT32, CONVERT_FIXED_TO_FIXED);
            break;
        case GL_UNSIGNED_INT_24_8_EXT:
            srcShift = 0;
            srcMax = 0xFFFFFFFF;
            dstStride = width * 4;
            CONVERT_DEPTH_PIXELS(gctUINT32, gctUINT32, CONVERT_NONE);
            break;
        case GL_UNSIGNED_BYTE:
            dstStride = width;
            if (format == GL_STENCIL_INDEX)
            {
                dstMax = 0;
                CONVERT_DEPTH_PIXELS(gctUINT8, gctUINT32, CONVERT_NONE);
            }
            else
            {
                dstMax = (1 << 8) - 1;
                CONVERT_DEPTH_PIXELS(gctUINT8, gctUINT32, CONVERT_FIXED_TO_FIXED);
            }
            break;
        default : break;
        }
        break;

    default :
        break;
    }

    gcoSURF_Unlock(tmpView.surf, srcData);

OnError:

    if (tmpView.surf != gcvNULL)
    {
        gcoSURF_Destroy(tmpView.surf);
    }

    return status;

}


gceSTATUS
readRGBAPixels(__GLcontext *gc,
               GLint x, GLint y,
               GLsizei width, GLsizei height,
               GLenum format, GLenum type, GLubyte *buf)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
     __GLclientPixelState *ps = &gc->clientState.pixel;
    gcoSURF source = chipCtx->readRT, target= gcvNULL;
    gctUINT srcWidth, srcHeight;
    GLint dx, dy, sx, sy, w, h;
    gceSTATUS status = gcvSTATUS_OK;
    gceSURF_FORMAT wrapformat = gcvSURF_UNKNOWN;
    gcsSURF_VIEW srcView = {gcvNULL, 0, 1};
    gcsSURF_VIEW dstView = {gcvNULL, 0, 1};
    gcsSURF_RESOLVE_ARGS rlvArgs = {0};

    if (source == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        goto OnError;
    }

    switch(type)
    {
    case GL_UNSIGNED_BYTE:
        if (format == GL_RGBA)
        {
            wrapformat = gcvSURF_A8B8G8R8;
        }
        else
        {
            wrapformat = gcvSURF_A8;
        }
        break;
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_4_4_4_4_REV:
        wrapformat = gcvSURF_A4R4G4B4;
        break;
    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_SHORT_5_6_5_REV:
        wrapformat = gcvSURF_R5G6B5;
        break;
    case GL_UNSIGNED_SHORT_5_5_5_1:
    case GL_UNSIGNED_SHORT_1_5_5_5_REV:
        wrapformat = gcvSURF_A1R5G5B5;
        break;
    default:
        break;
    }

    if (wrapformat == gcvSURF_UNKNOWN)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        goto OnError;
    }

    gcmONERROR(gcoSURF_GetSize(source,
                            &srcWidth,
                            &srcHeight,
                            gcvNULL));

    sx = x; sy = y; dx = 0; dy = 0; w = width; h = height;

    if (!calculateArea(&dx, &dy, &sx, &sy,
                        &w, &h, width, height,
                        srcWidth, srcHeight))
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        goto OnError;
    }

    /* Create the wrapper surface. */
    gcmONERROR(gcoSURF_Construct(gcvNULL,
                              width, height, 1,
                              gcvSURF_BITMAP,
                              wrapformat,
                              gcvPOOL_USER,
                              &target));

    gcmONERROR(gcoSURF_WrapSurface(
                                target,
                                ps->packModes.alignment,
                                buf,
                                gcvINVALID_ADDRESS));

    srcView.surf = source;
    dstView.surf = target;
    rlvArgs.version = gcvHAL_ARG_VERSION_V2;
    rlvArgs.uArgs.v2.srcOrigin.x = sx;
    rlvArgs.uArgs.v2.srcOrigin.x = sy;
    rlvArgs.uArgs.v2.dstOrigin.x = dx;
    rlvArgs.uArgs.v2.dstOrigin.x = dy;
    rlvArgs.uArgs.v2.rectSize.x  = w;
    rlvArgs.uArgs.v2.rectSize.x  = h;
    rlvArgs.uArgs.v2.numSlices   = 1;
    rlvArgs.uArgs.v2.dump        = gcvTRUE;
    gcmONERROR(gcoSURF_CopyPixels_v2(&srcView, &dstView, &rlvArgs));

OnError:
    if (target != gcvNULL)
    {
        gcoSURF_Destroy(target);
    }

    return status;
}

GLboolean __glChipReadPixels(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLubyte *buf)
{
    gceSTATUS status = gcvSTATUS_OK;

    switch(format)
    {
    case GL_DEPTH_COMPONENT:
    case GL_STENCIL_INDEX:
    case GL_DEPTH_STENCIL_EXT:
        status = readDepthStencilPixels(gc, x, y,
                                        width, height,
                                        format, type, buf);
        break;
    case GL_RGB:
    case GL_RGBA:
        status = readRGBAPixels(gc, x, y,
                                width, height,
                                format, type, buf);
        break;
    default:
        status = gcvSTATUS_INVALID_ARGUMENT;
        break;
    }

    return (status == gcvSTATUS_OK);
}

GLboolean __glChipCopyPixels(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format)
{
    /*
    ** Current Vivante chip doesn't support drawpixels directly. So we simulated using texture draw for some cases.
    */

    return simulatePixelOperation(gc, x, y, width, height, format, 0, NULL, GL_FALSE);
}

GLboolean __glChipBitmaps(__GLcontext *gc, GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap, __GLbufferObject* bufObj)
{
    GLubyte     *newbits = NULL;
    GLboolean   bFreeNewBits = GL_FALSE;
    GLint       w, h, shiftBit;
    GLubyte     *texImageData = NULL, *texImagePointer = NULL;
    GLint       align_bytes = width / 8;
    GLint       reminder_bits = width % 8;
    GLuint      index = 0;

    if (width == 0 || height == 0)
    {
        return GL_TRUE;
    }

    /* If any of the shader stage was enabled, */
    if (gc->state.enables.program.vertexProgram || gc->state.enables.program.fragmentProgram ||
        gc->shaderProgram.vertShaderEnable || gc->shaderProgram.fragShaderEnable || gc->shaderProgram.geomShaderEnable)
    {
        return GL_FALSE;
    }

    /* It is buffer object from unpack pixel */
    if (bufObj)
    {
        if (!bufObj->systemMemCache)
        {
            /* TODO: Lock the buffer and get the pointer */
            GL_ASSERT(0);
        }
        else
        {
            bitmap = (GLubyte *)bufObj->systemMemCache + ((GLubyte *)bitmap - (GLubyte *)0);
        }
    }

    if (((((width + 7) / 8) % gc->clientState.pixel.unpackModes.alignment) == 0) &&
        (gc->clientState.pixel.unpackModes.lineLength == 0) &&
        (gc->clientState.pixel.unpackModes.skipLines  == 0) &&
        (gc->clientState.pixel.unpackModes.skipPixels == 0) &&
        (gc->clientState.pixel.unpackModes.lsbFirst == 0))
    {
        newbits = (GLubyte *)bitmap;
    }
    else
    {
        newbits = (GLubyte *) (*gc->imports.malloc)(gc, (GLsizei)__glImageSize(width, height, GL_COLOR_INDEX, GL_BITMAP));
        __glFillImage(gc, width, height, GL_COLOR_INDEX, GL_BITMAP, bitmap, newbits);
        bFreeNewBits = GL_TRUE;
    }

    texImageData =
    texImagePointer = (GLubyte *)(*gc->imports.malloc)(gc, width * height);

    GL_ASSERT(texImagePointer);

    /* Convert bit mask to image */
    for (h = 0; h < height; h++)
    {
        for (w = 0; w < align_bytes; w++)
        {
            /* Revert bit */
            for (shiftBit = 7; shiftBit >= 0; shiftBit--)
            {
                if (newbits[index] & (1 << shiftBit))
                {
                    *texImagePointer++ = 0xff;
                }
                else
                {
                    *texImagePointer++ = 0x0;
                }
            }
            index++;
        }

        for (shiftBit = 0; shiftBit < reminder_bits; shiftBit++)
        {
            if (newbits[index] & (1 << (7 - shiftBit)))
            {
                *texImagePointer++ = 0xff;
            }
            else
            {
                *texImagePointer++ = 0x0;
            }
        }

        if (reminder_bits)
        {
            index ++;
        }
    }

    {
        GLuint texture;
        GLint oldError = gc->error;
        GLuint texStage = __GL_MAX_TEXTURE_COORDS - 1;
        __GLcoord vertex = gc->state.rasterPos.rPos.winPos;
        __GLcolor color = gc->state.rasterPos.rPos.colors[__GL_FRONTFACE];
        __GLcolor secondaryColor = gc->state.rasterPos.rPos.colors2[__GL_FRONTFACE];
        __GLdispatchTable *pDispatchTable = &gc->currentImmediateTable->dispatch;

        /* The simulation need to use the last texture unit. But if it was already been used, return FALSE */
        if (gc->state.enables.texUnits[texStage].enabledDimension > 0 )
        {
            return GL_FALSE;
        }

        gc->error = 0;

        vertex.x = (GLfloat)((GLint) (vertex.x - xorig));
        if (gc->drawablePrivate->yInverted)
        {
            vertex.y = gc->drawablePrivate->height - vertex.y;
        }
        vertex.y = (GLfloat)((GLint) (vertex.y - yorig));

        /* Push state */
        pDispatchTable->PushAttrib(GL_ALL_ATTRIB_BITS | GL_CURRENT_BIT | GL_LIGHTING_BIT | GL_POLYGON_BIT | GL_POLYGON_STIPPLE_BIT |
                                   GL_TEXTURE_BIT | GL_TRANSFORM_BIT | GL_VIEWPORT_BIT | GL_ENABLE_BIT);

        pDispatchTable->ActiveTexture(GL_TEXTURE0 + texStage);

        /* Push Matrix*/
        pDispatchTable->MatrixMode(GL_MODELVIEW);
        pDispatchTable->PushMatrix();
        pDispatchTable->MatrixMode(GL_TEXTURE);
        pDispatchTable->PushMatrix();
        pDispatchTable->MatrixMode(GL_PROJECTION);
        pDispatchTable->PushMatrix();

        /* State setting */
        pDispatchTable->Disable(GL_LIGHTING);/*Disable lighting*/
        pDispatchTable->Disable(GL_CULL_FACE);
        pDispatchTable->Disable(GL_POLYGON_STIPPLE);
        pDispatchTable->PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        pDispatchTable->Disable(GL_CLIP_PLANE0);
        pDispatchTable->Disable(GL_CLIP_PLANE1);
        pDispatchTable->Disable(GL_CLIP_PLANE2);
        pDispatchTable->Disable(GL_CLIP_PLANE3);
        pDispatchTable->Disable(GL_CLIP_PLANE4);
        pDispatchTable->Disable(GL_CLIP_PLANE5);

        /* Texture initialization */
        pDispatchTable->GenTextures(1, &texture);   /* Create a texture*/
        pDispatchTable->BindTexture(GL_TEXTURE_2D, texture);
        pDispatchTable->Enable(GL_TEXTURE_2D);      /* Enable texture */
        pDispatchTable->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        pDispatchTable->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        pDispatchTable->TexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, __GL_STIPPLE);
        pDispatchTable->TexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 1);
        pDispatchTable->TexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1);
        pDispatchTable->Disable(GL_TEXTURE_GEN_S);
        pDispatchTable->Disable(GL_TEXTURE_GEN_T);
        pDispatchTable->Disable(GL_TEXTURE_GEN_R);
        pDispatchTable->Disable(GL_TEXTURE_GEN_Q);

        pDispatchTable->TexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE,
                                   width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, texImageData);

        /* Transformation Matrix */
        pDispatchTable->MatrixMode(GL_MODELVIEW);
        pDispatchTable->LoadIdentity();
        pDispatchTable->MatrixMode(GL_PROJECTION);
        pDispatchTable->LoadIdentity();
        // The raster pos are already in screen space, its depth are already in range [0, 1]
        pDispatchTable->Ortho(0,gc->drawablePrivate->width, 0, gc->drawablePrivate->height, 0, -1);
        pDispatchTable->Viewport(0, 0, gc->drawablePrivate->width, gc->drawablePrivate->height);

        /* Depth Range */
        pDispatchTable->DepthRange(0,1);

        /*==============Issue draw====================*/
        /* Draw a Quad*/
        /* Spec vertex with only xyz channel.*/

        pDispatchTable->Color4f(color.r, color.g, color.b, color.a);
        pDispatchTable->SecondaryColor3f(secondaryColor.r, secondaryColor.g, secondaryColor.b);

        pDispatchTable->Begin(GL_QUADS);
        pDispatchTable->MultiTexCoord2f(GL_TEXTURE0 + texStage, 0, 0);
        pDispatchTable->Vertex4fv((GLfloat *)&vertex);

        vertex.y += height * gc->state.pixel.transferMode.zoomY;
        pDispatchTable->MultiTexCoord2f(GL_TEXTURE0 + texStage, 0, 1);
        pDispatchTable->Vertex4fv((GLfloat *)&vertex);

        vertex.x += width * gc->state.pixel.transferMode.zoomX;
        pDispatchTable->MultiTexCoord2f(GL_TEXTURE0 + texStage, 1, 1);
        pDispatchTable->Vertex4fv((GLfloat *)&vertex);

        vertex.y -= height * gc->state.pixel.transferMode.zoomY;
        pDispatchTable->MultiTexCoord2f(GL_TEXTURE0 + texStage, 1, 0);
        pDispatchTable->Vertex4fv((GLfloat *)&vertex);
        pDispatchTable->End();

        pDispatchTable->Flush();

        /*Delete texture*/
        pDispatchTable->DeleteTextures(1, &texture);

        /*=============restore state===========================*/
        /* Pop Matrix*/
        pDispatchTable->MatrixMode(GL_MODELVIEW);
        pDispatchTable->PopMatrix();
        pDispatchTable->MatrixMode(GL_TEXTURE);
        pDispatchTable->PopMatrix();
        pDispatchTable->MatrixMode(GL_PROJECTION);
        pDispatchTable->PopMatrix();
        /*Pop State*/
        pDispatchTable->PopAttrib();

        /* Do we have error?*/
        if (gc->error)
        {
            gc->error = oldError;
            GL_ASSERT(0);
            goto __Exit;
        }

        /* Restore the error value */
        gc->error = oldError;
    }

__Exit:

    if (texImageData)
    {
        (*gc->imports.free)(gc, texImageData);
    }

    if (bFreeNewBits)
    {
        (*gc->imports.free)(gc, newbits);
    }

    if (bufObj)
    {
        if (!bufObj->systemMemCache)
        {
            /* Unlock VB */
        }
    }

    return GL_TRUE;
}

extern gceSTATUS resolveDrawToTempBitmap(
    IN glsCHIPCONTEXT_PTR chipCtx,
    IN gcoSURF srcSurf,
    IN gctINT SourceX,
    IN gctINT SourceY,
    IN gctINT Width,
    IN gctINT Height
    );

GLvoid __glChipAccum(__GLcontext* gc, GLenum op, GLfloat value)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    glsTEXTURESAMPLER    oldSampler[2];
    gcsTEXTURE oldTexture[2];
    glsCHIPACCUMBUFFER * accumBuffer;
    glsCHIPRENDERBUFFER * renderBuffer = gcvNULL;
    GLint drawRTWidth, drawRTHeight;
    gceSTATUS status = gcvSTATUS_OK;
    GLuint64 texmask;
    GLint drawtofront = 0;

    __GLdispatchTable *pDispatchTable = &gc->currentImmediateTable->dispatch;

    __GLcoord vertex[4] = {
        {{-1.0f,-1.0f,1.0f,1.0f}},
        {{1.0f,-1.0f,1.0f,1.0f}},
        {{1.0f, 1.0f,1.0f,1.0f}},
        {{-1.0f, 1.0f,1.0f,1.0f}},
    };

    /* We evaluateAttribute to update states before we change our special states */
    __glEvaluateAttribDrawableChange(gc);

    /* Save current states */
    saveAttributes(gc, chipCtx);

    /* Reset current states to default states for manual draw */
    resetAttributes(gc, chipCtx);

    /* Disable color mask for accum op which target on accum buffer */
    switch(op)
    {
        case GL_ACCUM:
        case GL_LOAD:
        case GL_MULT:
        case GL_ADD:
            /* When accum on accum buffer, it should not affected by color mask */
            pDispatchTable->ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            pDispatchTable->ClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FALSE);
            break;

        case GL_RETURN:
            /* When return to color buffer, it should be affected by color mask */
            break;

        default:
            GL_ASSERT(0);
            break;
    }

    texmask = gc->texUnitAttrDirtyMask;
    gc->texUnitAttrDirtyMask = 0;

    chipCtx->drawToAccumBuf = GL_TRUE;
    chipCtx->accumValue = value;

    oldSampler[0] = chipCtx->texture.sampler[0];
    oldSampler[1] = chipCtx->texture.sampler[1];
    oldTexture[0] = chipCtx->texture.halTexture[0];
    oldTexture[1] = chipCtx->texture.halTexture[1];

    chipCtx->hashKey.accumMode = gccACCUM_UNKNOWN;

    if (gc->drawablePrivate->width != 0 && gc->drawablePrivate->height != 0)
    {
        accumBuffer = (glsCHIPACCUMBUFFER*)(gc->drawablePrivate->accumBuffer.privateData);

        switch(op)
        {
            case GL_ACCUM:
            case GL_LOAD:
            case GL_MULT:
            case GL_ADD:
                switch(gc->state.pixel.readBuffer)
                {
                    case GL_FRONT_LEFT:
                    case GL_FRONT_RIGHT:
                    case GL_FRONT:
                        renderBuffer = *(((glsCHIPDRAWABLE*)(gc->readablePrivate->dp.privateData))->drawBuffers[__GL_FRONT_BUFFER_INDEX]);
                        break;

                   case GL_BACK_LEFT:
                   case GL_BACK_RIGHT:
                   case GL_LEFT:
                   case GL_RIGHT:
                   case GL_BACK:
                       renderBuffer = *(((glsCHIPDRAWABLE*)(gc->readablePrivate->dp.privateData))->drawBuffers[__GL_BACK_BUFFER0_INDEX]);
                       break;
                }
                break;

            case GL_RETURN:
                switch(gc->state.raster.drawBuffer[0])
                {
                case GL_FRONT_LEFT:
                case GL_FRONT_RIGHT:
                case GL_FRONT:
                    renderBuffer = *(((glsCHIPDRAWABLE*)(gc->drawablePrivate->dp.privateData))->drawBuffers[__GL_FRONT_BUFFER_INDEX]);
                    break;

                case GL_BACK_LEFT:
                case GL_BACK_RIGHT:
                case GL_LEFT:
                case GL_RIGHT:
                case GL_BACK:
                    renderBuffer = *(((glsCHIPDRAWABLE*)(gc->drawablePrivate->dp.privateData))->drawBuffers[__GL_BACK_BUFFER0_INDEX]);
                    break;
                }
                break;
        }

        chipCtx->texture.sampler[0] = accumBuffer->sampler[0];
        chipCtx->texture.sampler[1] = accumBuffer->sampler[1];
        chipCtx->texture.halTexture[0] = accumBuffer->texture[0];
        chipCtx->texture.halTexture[1] = accumBuffer->texture[1];

        /* Blt rtResource to texResource if needed */
        switch(op)
        {
            case GL_ACCUM:
            case GL_MULT:
            case GL_ADD:
            case GL_RETURN:
                break;

            case GL_LOAD:
                break;

            default:
                break;
                GL_ASSERT(0);
                break;
        }

        chipCtx->hashKey.accumMode = op - GL_ACCUM + gccACCUM_ACCUM;
        /* Set new RT and textures */
        switch(op)
        {
            case GL_ACCUM:
                break;

            case GL_LOAD:
                break;

            case GL_MULT:
            case GL_ADD:
                break;

            case GL_RETURN:
                break;

            default:
               GL_ASSERT(0);
               break;
        }

        gcoSURF_GetSize(accumBuffer->renderTarget, (gctUINT *)&drawRTWidth, (gctUINT *)&drawRTHeight, gcvNULL);

        status = gco3D_SetTarget(chipCtx->hw, 0, accumBuffer->renderTarget, 0, 0);

        if (gcmIS_ERROR(status))
        {
            return;
        }

        /* Resolve the rectangle to the temporary surface. */
        status = resolveDrawToTempBitmap(chipCtx,
            accumBuffer->renderTarget,
            0, 0,
            drawRTWidth,
            drawRTHeight);

        if (gcmIS_ERROR(status))
        {
            return;
        }

        /* Upload the texture. */
        status = gcoTEXTURE_Upload(accumBuffer->textureInfo[0].object,
                                   0,
                                   0,
                                   drawRTWidth,
                                   drawRTHeight,
                                   0,
                                   chipCtx->tempLastLine,
                                   chipCtx->tempStride,
                                   chipCtx->tempFormat,
                                   gcvSURF_COLOR_SPACE_LINEAR);

        gcoSURF_GetSize(renderBuffer->renderTarget, (gctUINT *)&drawRTWidth, (gctUINT *)&drawRTHeight, gcvNULL);

        status = gco3D_SetTarget(chipCtx->hw, 0, renderBuffer->renderTarget, 0, 0);

        if (gcmIS_ERROR(status))
        {
            return;
        }

        /* Resolve the rectangle to the temporary surface. */
        status = resolveDrawToTempBitmap(chipCtx,
            renderBuffer->renderTarget,
            0, 0,
            drawRTWidth,
            drawRTHeight);

        if (gcmIS_ERROR(status))
        {
            return;
        }

        /* Upload the texture. */
        status = gcoTEXTURE_Upload(accumBuffer->textureInfo[1].object,
                                   0,
                                   0,
                                   drawRTWidth,
                                   drawRTHeight,
                                   0,
                                   chipCtx->tempLastLine,
                                   chipCtx->tempStride,
                                   chipCtx->tempFormat,
                                   gcvSURF_COLOR_SPACE_LINEAR);

        if (op != GL_RETURN)
        {
            gco3D_SetTarget(chipCtx->hw, 0, accumBuffer->renderTarget, 0, 0);
        }

        if ( gc->flags & __GL_DRAW_TO_FRONT)
        {
            gc->flags &= ~__GL_DRAW_TO_FRONT;
            drawtofront = 1;
        }

        /* Draw a quad */
        pDispatchTable->Begin(GL_QUADS);
        pDispatchTable->MultiTexCoord2f(GL_TEXTURE0, 0, 0);
        pDispatchTable->MultiTexCoord2f(GL_TEXTURE1, 0, 0);
        pDispatchTable->Vertex4fv((GLfloat *)&vertex[0]);

        pDispatchTable->MultiTexCoord2f(GL_TEXTURE0, 1, 0);
        pDispatchTable->MultiTexCoord2f(GL_TEXTURE1, 1, 0);
        pDispatchTable->Vertex4fv((GLfloat *)&vertex[1]);

        pDispatchTable->MultiTexCoord2f(GL_TEXTURE0, 1, 1);
        pDispatchTable->MultiTexCoord2f(GL_TEXTURE1, 1, 1);
        pDispatchTable->Vertex4fv((GLfloat *)&vertex[2]);

        pDispatchTable->MultiTexCoord2f(GL_TEXTURE0, 0, 1);
        pDispatchTable->MultiTexCoord2f(GL_TEXTURE1, 0, 1);
        pDispatchTable->Vertex4fv((GLfloat *)&vertex[3]);
        pDispatchTable->End();

        /* Flush the cache. */
        if (op != GL_RETURN)
        {
            gcoSURF_Flush(accumBuffer->renderTarget);
        }
        else
        {
            gcoSURF_Flush(renderBuffer->renderTarget);
        }
        gcoHAL_Commit(chipCtx->hal, gcvTRUE);

        if (op != GL_RETURN)
        {
            gco3D_SetTarget(chipCtx->hw, 0, renderBuffer->renderTarget, 0, 0);
        }

        if ( drawtofront)
        {
            gc->flags |= __GL_DRAW_TO_FRONT;

            if (op == GL_RETURN)
            {
                pDispatchTable->Flush();
            }
        }
    }

    chipCtx->texture.sampler[0] = oldSampler[0];
    chipCtx->texture.sampler[1] = oldSampler[1];
    chipCtx->texture.halTexture[0] = oldTexture[0];
    chipCtx->texture.halTexture[1] = oldTexture[1];

    chipCtx->hashKey.accumMode = gccACCUM_UNKNOWN;

    gc->texUnitAttrDirtyMask = texmask;


    pDispatchTable->MatrixMode(GL_PROJECTION);
    pDispatchTable->PopMatrix();
    pDispatchTable->MatrixMode(GL_TEXTURE);
    pDispatchTable->PopMatrix();
    pDispatchTable->MatrixMode(GL_MODELVIEW);
    pDispatchTable->PopMatrix();

    /* Restore saved current states */
    restoreAttributes(gc, chipCtx);
}
