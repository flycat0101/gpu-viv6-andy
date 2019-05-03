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
 *  Feature:    LINE - mask
 *  API:        gco2D_ColorLine
 *  Check:      ROP values
 */
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DLine011\n" \
"Operation: Test gco2D_ColorLine with all ROP values.\n" \
"2D API: gco2D_ColorLine\n" \
"Dst: Size        [configurable]\n"\
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
"Brush: Type   [ColorBrush]\n"\
"       Format [ARGB8888]\n"\
"       Offset [0]\n" \
"Alphablend: [disable]\n" \
"HW feature dependency: ";

typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;

    /* Dst surface. */
    gcoSURF            dstSurf;
    gceSURF_FORMAT    dstFormat;
    gctUINT            dstWidth;
    gctUINT            dstHeight;
    gctINT            dstStride;
    gctUINT32        dstPhyAddr;
    gctPOINTER        dstLgcAddr;

    /* Source surface. */
    gcoSURF            srcSurf;
    gceSURF_FORMAT    srcFormat;
    gctUINT            srcWidth;
    gctUINT            srcHeight;
    gctINT            srcStride;
    gctUINT32        srcPhyAddr;
    gctPOINTER        srcLgcAddr;

    /* Brush. */
    gcoBRUSH brush;
} Test2D;


static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gcsRECT    dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};
    gco2D      egn2D   = t2d->runtime->engine2d;
    gctUINT8   ROP     = 0;
    gctINT     deltaX  = t2d->dstWidth >> 4;
    gctINT     deltaY  = t2d->dstHeight >> 4;
    gceSTATUS  status;
    gctINT     x, y, i;
    gcsRECT   *line;
    gctUINT32  lineColor;

    /* Clear dst surface with blue. */
    gcmONERROR(Gal2DCleanSurface(t2d->runtime->hal, t2d->dstSurf, COLOR_ARGB8(0x00, 0x00, 0x00, 0xFF)));

    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    /* Set brush. */
    gcmONERROR(gco2D_FlushBrush(egn2D, t2d->brush, t2d->dstFormat));

    /* Set color source and src rect. */
    if (t2d->runtime->pe20)
    {
        gcmONERROR(gco2D_SetColorSourceAdvanced(egn2D, t2d->srcPhyAddr, t2d->srcStride, t2d->srcFormat,
                        gcvSURF_0_DEGREE, t2d->srcWidth, t2d->srcHeight, gcvFALSE));

        gcmONERROR(gco2D_SetTransparencyAdvanced(egn2D, gcv2D_OPAQUE, gcv2D_OPAQUE, gcv2D_OPAQUE));
    }
    else
    {
        gcmONERROR(gco2D_SetColorSource(egn2D, t2d->srcPhyAddr, t2d->srcStride, t2d->srcFormat,
                        gcvSURF_0_DEGREE, t2d->srcWidth, gcvFALSE, gcvSURF_OPAQUE, 0));
    }

    gcmONERROR(gco2D_SetSource(egn2D, &dstRect));

    gcmONERROR(gco2D_SetTarget(egn2D, t2d->dstPhyAddr, t2d->dstStride, gcvSURF_0_DEGREE, t2d->dstWidth));

    line = (gcsRECT*)malloc(sizeof(gcsRECT) * deltaY);

    /* Set the line color (ARGB8888). */
    lineColor = 0x00536271;

    for (y = 0; y < 16; y++)
    {
        for (x = 0; x < 16; x++)
        {
            dstRect.left   = x * deltaX;
            dstRect.top    = y * deltaY;
            dstRect.right  = dstRect.left + deltaX;
            dstRect.bottom = dstRect.top + deltaY;

            /* Fill rect with lines. */
            for (i = 0; i < deltaY; i++)
            {
                line[i].left   = dstRect.left;
                line[i].top    = dstRect.top + i;
                line[i].right  = dstRect.right;
                line[i].bottom = dstRect.top + i;
            }

            gcmONERROR(gco2D_ColorLine(egn2D, deltaY, line, lineColor, ROP, ROP,t2d->dstFormat));

            gcmONERROR(gco2D_Flush(egn2D));

            ++ROP;
        }
    }

    free(line);

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

    if (t2d->brush != gcvNULL)
    {
        if (gcmIS_ERROR(gcoBRUSH_Destroy(t2d->brush)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Brush failed:%s\n", GalStatusString(status));
        }
        t2d->brush = gcvNULL;
    }

    /* Destroy source surface. */
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

const gceFEATURE FeatureList[]=
{
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
    t2d->dstFormat  = runtime->format;
    t2d->dstWidth   = 0;
    t2d->dstHeight  = 0;
    t2d->dstStride  = 0;
    t2d->dstPhyAddr = 0;
    t2d->dstLgcAddr = 0;

    t2d->srcSurf    = gcvNULL;
    t2d->srcWidth   = 0;
    t2d->srcHeight  = 0;
    t2d->srcStride  = 0;
    t2d->srcPhyAddr = 0;
    t2d->srcLgcAddr = 0;
    t2d->srcFormat  = gcvSURF_A8R8G8B8;
    t2d->brush = gcvNULL;

    if (runtime->ChipModel == gcv320 && runtime->ChipRevision == 0x5007)
    {
        GalOutput(GalOutputType_Result | GalOutputType_Console, "Clear with all rop is not supported.\n");
        runtime->notSupport = gcvTRUE;
        return gcvFALSE;
    }

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    /* Create source surface. */
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

    /* Clear src surface with green. */
    gcmONERROR(Gal2DCleanSurface(t2d->runtime->hal, t2d->srcSurf, COLOR_ARGB8(0x00, 0x00, 0xFF, 0x00)));

    /* Create brush. */
    gcmONERROR(gco2D_ConstructSingleColorBrush(t2d->runtime->engine2d, gcvTRUE,
        COLOR_ARGB8(0x00, 0xFF, 0x00, 0x00), 0, &t2d->brush));

    t2d->base.render      = (PGalRender)Render;
    t2d->base.destroy     = (PGalDestroy)Destroy;
    t2d->base.frameCount  = 1;
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
