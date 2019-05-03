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
 *  Feature:    2DBitBlit - ROP
 *  API:        gco2D_Blit
 *  Check:
*/
#include <galUtil.h>

static char* s_CaseDescription = \
"Case gal2DBitBlit001\n" \
"Operation: Test BitBlit with ROP value.\n" \
"2D API: gco2D_ConstructColorBrush gco2D_Blit\n" \
"Src: Size        [configurable]\n"\
"     Rect        [configurable]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Dst: Size        [configurable]\n"\
"     Rect        [configurable]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: Type   [ColorBrush]\n"\
"       Format [ARGB8888]\n"\
"       Offset [0]\n" \
"Alphablend: [disable]\n" \
"HW feature dependency: ";

typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;

    // dst
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
    gcsRECT dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};
    gco2D egn2D = t2d->runtime->engine2d;
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT8 ROP = 0;
    gctINT x, y;
    gctINT deltaX = t2d->dstWidth >> 4;
    gctINT deltaY = t2d->dstHeight >> 4;

    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    // clear dst surface with blue
    gcmONERROR(Gal2DCleanSurface(t2d->runtime->hal, t2d->dstSurf, COLOR_ARGB8(0x00, 0x00, 0x00, 0xFF)));

    gcmONERROR(gco2D_FlushBrush(egn2D, t2d->brush, t2d->dstFormat));

    // set color source and src rect
    gcmONERROR(gco2D_SetColorSource(egn2D, t2d->srcPhyAddr, t2d->srcStride, t2d->srcFormat,
                    gcvSURF_0_DEGREE, t2d->srcWidth, gcvFALSE, gcvSURF_OPAQUE, 0));

    gcmONERROR(gco2D_SetSource(egn2D, &dstRect));

    gcmONERROR(gco2D_SetTarget(egn2D, t2d->dstPhyAddr, t2d->dstStride, gcvSURF_0_DEGREE, t2d->dstWidth));

    for (y = 0; y < 16; y++)
    {
        for (x = 0; x < 16; x++)
        {
            dstRect.left = x * deltaX;
            dstRect.top = y * deltaY;
            dstRect.right = dstRect.left + deltaX;
            dstRect.bottom = dstRect.top + deltaY;

            gcmONERROR(gco2D_Blit(egn2D, 1, &dstRect, ROP, ROP, t2d->dstFormat));
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
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Brush failed:%s\n", GalStatusString(status));
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

static gctUINT32 sColorBuf[] = {
0x00000000, 0x00100000, 0x00200000, 0x00300000, 0x000000cc, 0x00000033, 0x000000aa, 0x00000055,
0x00400000, 0x00500000, 0x00600000, 0x00700000, 0x000000f0, 0x0000000f, 0x0000cc33, 0x0000aa55,
0x00800000, 0x00900000, 0x00a00000, 0x00b00000, 0x0000f00f, 0x00f0ccaa, 0x000f3355, 0x00ff00ff,
0x00c00000, 0x00d00000, 0x00e00000, 0x00f00000, 0x0000ffff, 0x000000ff, 0x0000ff00, 0x00ffffff,
0x00000f00, 0x00001f00, 0x00002f00, 0x00003f00, 0x00101010, 0x00202020, 0x00303030, 0x00404040,
0x00004f00, 0x00005f00, 0x00006f00, 0x00007f00, 0x00505050, 0x00606060, 0x00707070, 0x00808080,
0x00008f00, 0x00009f00, 0x0000af00, 0x0000bf00, 0x00909090, 0x00a0a0a0, 0x00b0b0b0, 0x00c0c0c0,
0x0000cf00, 0x0000df00, 0x0000ef00, 0x0000ff00, 0x00d0d0d0, 0x00e0e0e0, 0x00f0f0f0, 0x00800000,
};

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_2D_NO_COLORBRUSH_INDEX8,
    gcvFEATURE_ANDROID_ONLY,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status = gcvSTATUS_OK;

    gctUINT32 k, listLen = sizeof(FeatureList)/sizeof(gctINT);
    gctBOOL featureStatus;
    char featureName[FEATURE_NAME_LEN], featureMsg[FEATURE_MSG_LEN];

    runtime->wholeDescription = (char*)malloc(FEATURE_NAME_LEN * listLen + strlen(s_CaseDescription) + 1);

    if (runtime->wholeDescription == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    memcpy(runtime->wholeDescription, s_CaseDescription, strlen(s_CaseDescription) + 1);

    for(k = 0; k < listLen; k++)
    {
        gcmONERROR(GalQueryFeatureStr(FeatureList[k], featureName, featureMsg, &featureStatus));
        if (gcoHAL_IsFeatureAvailable(runtime->hal, FeatureList[k]) == featureStatus)
        {
            GalOutput(GalOutputType_Result | GalOutputType_Console, "%s is not supported.\n", featureMsg);
            runtime->notSupport = gcvTRUE;
        }
        strncat(runtime->wholeDescription, featureName, k==listLen-1 ? strlen(featureName)+1:strlen(featureName));
    }

    if (runtime->notSupport)
        return gcvFALSE;

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
    t2d->srcFormat = gcvSURF_A8R8G8B8;

    t2d->brush = gcvNULL;

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    // create brush
    gcmONERROR(gco2D_ConstructColorBrush(t2d->runtime->engine2d, 0, 0,
                sColorBuf, gcvSURF_A8R8G8B8, 0, &t2d->brush));

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

    // clear src surface with green
    gcmONERROR(Gal2DCleanSurface(t2d->runtime->hal, t2d->srcSurf, COLOR_ARGB8(0x00, 0x00, 0xFF, 0x00)));

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
