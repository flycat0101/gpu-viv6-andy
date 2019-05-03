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
 *  Feature:    Stretch Blit - ROP
 *  API:        gco2D_StretchBlit
 *  Check:
*/
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DStretchBlit014\n" \
"Operation: Test stretchblit with ROP value on hardware with ANDROID_ONLY feature.\n" \
"2D API: gco2D_StretchBlit\n" \
"Src: Size        [64x64]\n"\
"     Rect        [0,0,64,64]\n"\
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
"       Format [configurable]\n"\
"       Offset [0]\n" \
"Alphablend: [disable]\n" \
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

    //brush
    gcoBRUSH        brush;

    //source surface
    gcoSURF            srcSurf;
    gceSURF_FORMAT    srcFormat;
    gctUINT            srcWidth;
    gctUINT            srcHeight;
    gctINT            srcStride;
    gctUINT32        srcPhyAddr;
    gctPOINTER        srcLgcAddr;
} Test2D;

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gctUINT32 horFactor, verFactor;
    gcsRECT srcRect = {0, 0, t2d->srcWidth, t2d->srcHeight};
    gco2D egn2D = t2d->runtime->engine2d;
    gceSTATUS status;
    gcsRECT dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};
        gctUINT8 ropBox[4] = {0xCC, 0xAA, 0xCC, 0xF0};
    gctINT x, y;
    gctINT deltaX = t2d->dstWidth >> 4;
    gctINT deltaY = t2d->dstHeight >> 4;
    gctUINT8 ROP = 0;

    // clear dst surface with blue
    gcmONERROR(Gal2DCleanSurface(t2d->runtime->hal, t2d->dstSurf, COLOR_ARGB8(0x00, 0x00, 0x00, 0xFF)));

    // set brush
    gcmONERROR(gco2D_FlushBrush(egn2D, t2d->brush, t2d->dstFormat));

    // set source color and rect
    gcmONERROR(gco2D_SetColorSource(egn2D, t2d->srcPhyAddr, t2d->srcStride, t2d->srcFormat,
                    gcvSURF_0_DEGREE, t2d->srcWidth, gcvFALSE, gcvSURF_OPAQUE, 0));

    gcmONERROR(gco2D_SetSource(egn2D, &srcRect));

    // set dst and clippint rect
    gcmONERROR(gco2D_SetTarget(egn2D, t2d->dstPhyAddr, t2d->dstStride, gcvSURF_0_DEGREE, t2d->dstWidth));

    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    // dst rect size
    dstRect.left = 0;
    dstRect.right = deltaX;
    dstRect.top = 0;
    dstRect.bottom = deltaY;

    /* Calculate the stretch factors. */
    gcmONERROR(gco2D_CalcStretchFactor(egn2D, srcRect.right - srcRect.left,
        dstRect.right - dstRect.left, &horFactor));

    gcmONERROR(gco2D_CalcStretchFactor(egn2D, srcRect.bottom - srcRect.top,
        dstRect.bottom - dstRect.top, &verFactor));

    /* Program the stretch factors. */
    gcmONERROR(gco2D_SetStretchFactors(egn2D, horFactor, verFactor));

    for (y = 0; y < 16; y++)
    {
        for (x = 0; x < 16; x++)
        {
            dstRect.left = x * deltaX;
            dstRect.top = y * deltaY;
            dstRect.right = dstRect.left + deltaX;
            dstRect.bottom = dstRect.top + deltaY;

            gcmONERROR(gco2D_StretchBlit(egn2D, 1, &dstRect, ropBox[ROP%4], ropBox[ROP%4], t2d->dstFormat));
            ++ROP;
        }
    }

    gcmONERROR(gco2D_Flush(egn2D));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

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

    // destroy brush
    if (t2d->brush != gcvNULL)
        if (gcmIS_ERROR(gcoBRUSH_Destroy(t2d->brush)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Surf failed:%s\n", GalStatusString(status));
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
    gceSTATUS status;
    char * sourcefile = "resource/zero1.bmp";

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

    t2d->srcSurf    = gcvNULL;
    t2d->srcWidth = 0;
    t2d->srcHeight = 0;
    t2d->srcStride = 0;
    t2d->srcPhyAddr = 0;
    t2d->srcLgcAddr = 0;
    t2d->srcFormat = gcvSURF_UNKNOWN;

    t2d->brush = gcvNULL;

    // create red brush
    gcmONERROR(gco2D_ConstructSingleColorBrush(t2d->runtime->engine2d , (t2d->dstFormat!=gcvSURF_A8R8G8B8),
                COLOR_ARGB8(0x00, 0xFF, 0x00, 0x00), 0, &t2d->brush));

    // create source surface
    t2d->srcSurf = GalLoadDIB2Surface(t2d->runtime->hal, sourcefile);
    if (t2d->srcSurf == NULL)
    {
        // destroy brush
        if (t2d->brush != gcvNULL)
            gcmONERROR(gcoBRUSH_Destroy(t2d->brush));

        gcmONERROR(gcvSTATUS_NOT_FOUND);
    }
    gcmONERROR(gcoSURF_GetAlignedSize(t2d->srcSurf,
                                        gcvNULL,
                                        gcvNULL,
                                        &t2d->srcStride));

    gcmONERROR(gcoSURF_GetSize(t2d->srcSurf,
                                &t2d->srcWidth,
                                &t2d->srcHeight,
                                gcvNULL));

    gcmONERROR(gcoSURF_GetFormat(t2d->srcSurf, gcvNULL, &t2d->srcFormat));

    gcmONERROR(gcoSURF_Lock(t2d->srcSurf, &t2d->srcPhyAddr, &t2d->srcLgcAddr));

    // dst with dst surf
    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = 1;
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
