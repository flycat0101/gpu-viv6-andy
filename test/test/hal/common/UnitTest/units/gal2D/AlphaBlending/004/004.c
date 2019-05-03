/****************************************************************************
*
*    Copyright 2012 - 2019 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/


/*
 *  Feature:    Alpha Blending - source and destination alpha value multiplied by global alpha value
 *  API:        gco2D_EnableAlphaBlend/gco2D_EnableAlphaBlendAdvanced gco2D_DisableAlphaBlend
 *                gco2D_EnableAlphaBlend is only working with old PE (<2.0) and
 *                gco2D_EnableAlphaBlendAdvanced is only working with PE 2.0 and above.
 *  Check:
 *
*/
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DAlphaBlending004\n" \
"Operation: Test source and destination alpha value multiplied by global alpha value.\n" \
"2D API: gco2D_Blit gco2D_EnableAlphaBlend/gco2D_EnableAlphaBlendAdvanced gco2D_SetSourceGlobalColorAdvanced gco2D_SetTargetGlobalColorAdvanced\n" \
"Src: Size        [configurable]\n"\
"     Rect        [configurable]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Dst: Size        [configurable]\n"\
"     Rect        [configurable]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: Type   [SolidBrush]\n"\
"       Format [ARGB8888]\n"\
"       Offset [0]\n" \
"Alphablend: [enable]\n" \
"HW feature dependency: ";

typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;

    // destination surface
    gcoSURF            dstSurf;
    gceSURF_FORMAT    dstFormat;
    gctUINT            dstWidth;
    gctUINT            dstHeight;
    gctINT            dstStride;
    gctUINT32        dstPhyAddr;
    gctPOINTER        dstLgcAddr;
    gctUINT8        dstAlpha;

    //source surface
    gcoSURF            srcSurf;
    gceSURF_FORMAT    srcFormat;
    gctUINT            srcWidth;
    gctUINT            srcHeight;
    gctINT            srcStride;
    gctUINT32        srcPhyAddr;
    gctPOINTER        srcLgcAddr;
    gctUINT8        srcAlpha;
} Test2D;

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gcsRECT bgRect = {20, 30, 120, 130}, fgRect = {50, 60, 250, 260};
    gcsRECT dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};
    gcoBRUSH bgBrush, fgBrush;
    gco2D egn2D = t2d->runtime->engine2d;
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT8 ROP;

    // clear dest surface  with black
    gcmONERROR(Gal2DCleanSurface(t2d->runtime->hal, t2d->dstSurf, COLOR_ARGB8(0xFF, 0x00, 0x00, 0x00)));

    // draw dst rect
    gcmONERROR(gco2D_ConstructSingleColorBrush(egn2D, gcvTRUE,
                COLOR_ARGB8(t2d->dstAlpha, 0xFF, 0x00, 0x00), 0, &bgBrush));
    if (gcmIS_ERROR(Gal2DRectangle(t2d->runtime->hal, t2d->dstSurf, bgBrush, bgRect)))
    {
        gcmONERROR(gcvSTATUS_INVALID_REQUEST);
    }
    gcmONERROR(gcoBRUSH_Destroy(bgBrush));

    // clear src surface with black
    gcmONERROR(Gal2DCleanSurface(t2d->runtime->hal, t2d->srcSurf, COLOR_ARGB8(0xFF, 0x00, 0x00, 0x00)));

    // draw src rect
    gcmONERROR(gco2D_ConstructSingleColorBrush(egn2D, gcvTRUE,
                COLOR_ARGB8(t2d->srcAlpha, 0x00, 0x00, 0xFF), 0, &fgBrush));
    if (gcmIS_ERROR(Gal2DRectangle(t2d->runtime->hal, t2d->srcSurf, fgBrush, fgRect)))
    {
        gcmONERROR(gcvSTATUS_INVALID_REQUEST);
    }
    gcmONERROR(gcoBRUSH_Destroy(fgBrush));

    // set color source and src rect
    if (t2d->runtime->pe20)
    {
        gcmONERROR(gco2D_SetColorSourceAdvanced(egn2D, t2d->srcPhyAddr, t2d->srcStride, t2d->srcFormat,
                        gcvSURF_0_DEGREE, t2d->srcWidth, t2d->srcHeight, gcvFALSE));
        ROP = 0xCC;
    }
    else
    {
        gcmONERROR(gco2D_SetColorSource(egn2D, t2d->srcPhyAddr, t2d->srcStride, t2d->srcFormat,
                        gcvSURF_0_DEGREE, t2d->srcWidth, gcvFALSE, gcvSURF_OPAQUE, 0));
        ROP = 0x88;
    }

    gcmONERROR(gco2D_SetSource(egn2D, &dstRect));

    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    gcmONERROR(gco2D_SetTarget(egn2D, t2d->dstPhyAddr, t2d->dstStride, gcvSURF_0_DEGREE, t2d->dstWidth));

    // enalbe alphablend
    if (t2d->runtime->pe20)
    {
        gcmONERROR(gco2D_SetSourceGlobalColorAdvanced(egn2D, 0x40000000));
        gcmONERROR(gco2D_SetTargetGlobalColorAdvanced(egn2D, 0x80000000));

        gcmONERROR(gco2D_EnableAlphaBlendAdvanced(egn2D,
                    gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
                    gcvSURF_GLOBAL_ALPHA_SCALE, gcvSURF_GLOBAL_ALPHA_SCALE,
                    gcvSURF_BLEND_STRAIGHT, gcvSURF_BLEND_STRAIGHT));
    }
    else
    {
        gcmONERROR(gco2D_EnableAlphaBlend(egn2D,
                    0x40, 0x80,
                    gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
                    gcvSURF_GLOBAL_ALPHA_SCALE, gcvSURF_GLOBAL_ALPHA_SCALE,
                    gcvSURF_BLEND_STRAIGHT, gcvSURF_BLEND_STRAIGHT,
                    gcvSURF_COLOR_STRAIGHT, gcvSURF_COLOR_STRAIGHT));
    }

    gcmONERROR(gco2D_Blit(egn2D, 1, &dstRect, ROP, ROP, t2d->dstFormat));

    // disalbe alphablend
    gcmONERROR(gco2D_DisableAlphaBlend(egn2D));

    gcmONERROR(gco2D_Flush(egn2D));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    t2d->dstAlpha -= 0x10;
    t2d->srcAlpha += 0x10;

    return gcvTRUE;

OnError:
    GalOutput(GalOutputType_Error | GalOutputType_Console,
        "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));
    return gcvFALSE;
}

static void CDECL Destroy(Test2D *t2d)
{
    gceSTATUS status = gcvSTATUS_OK;
    if ((t2d->dstSurf != gcvNULL) && (t2d->dstLgcAddr != gcvNULL))
    {
        if (gcmIS_ERROR(gcoSURF_Unlock(t2d->dstSurf, t2d->dstLgcAddr)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock desSurf failed:%s\n", GalStatusString(status));
        }
        t2d->dstLgcAddr = gcvNULL;
    }

    // destroy source surface
    if (t2d->srcSurf != gcvNULL)
    {
        if (t2d->srcLgcAddr)
        {
            if (gcmIS_ERROR(gcoSURF_Unlock(t2d->srcSurf, t2d->srcLgcAddr)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock srcSurf failed:%s\n", GalStatusString(status));
            }
            t2d->srcLgcAddr = 0;
        }

        if (gcmIS_ERROR(gcoSURF_Destroy(t2d->srcSurf)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Surf failed:%s\n", GalStatusString(status));
        }
    }

    free(t2d);
}

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status = gcvSTATUS_OK;

    runtime->wholeDescription = (char*)malloc(strlen(s_CaseDescription) + 1);
    if (runtime->wholeDescription == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    memcpy(runtime->wholeDescription, s_CaseDescription, strlen(s_CaseDescription) + 1);

    t2d->runtime = runtime;

    t2d->dstSurf    = runtime->target;
    t2d->dstFormat = runtime->format;
    t2d->dstWidth = 0;
    t2d->dstHeight = 0;
    t2d->dstStride = 0;
    t2d->dstPhyAddr = 0;
    t2d->dstLgcAddr = 0;
    t2d->dstAlpha = 0x80;

    t2d->srcSurf    = gcvNULL;
    t2d->srcWidth = 0;
    t2d->srcHeight = 0;
    t2d->srcStride = 0;
    t2d->srcPhyAddr = 0;
    t2d->srcLgcAddr = 0;
    t2d->srcFormat = gcvSURF_A8R8G8B8;
    t2d->srcAlpha = 0x80;

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    // create source surface
    gcmONERROR(gcoSURF_Construct(t2d->runtime->hal,
                               t2d->runtime->width,
                               t2d->runtime->height,
                               1,
                               gcvSURF_BITMAP,
                               t2d->srcFormat,
                               gcvPOOL_DEFAULT,
                               &t2d->srcSurf));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->srcSurf,
                                        &t2d->srcWidth,
                                        &t2d->srcHeight,
                                        &t2d->srcStride));

    gcmONERROR(gcoSURF_Lock(t2d->srcSurf, &t2d->srcPhyAddr, &t2d->srcLgcAddr));

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = 8;
    t2d->base.description = s_CaseDescription;

    return gcvTRUE;

OnError:
    GalOutput(GalOutputType_Error | GalOutputType_Console,
        "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));
    return gcvFALSE;
}

GalTest * CDECL GalCreateTestObject(GalRuntime *runtime)
{
    Test2D *t2d = (Test2D *)malloc(sizeof(Test2D));

    if (!Init(t2d, runtime)) {
        free(t2d);
        return NULL;
    }

    return &t2d->base;
}
