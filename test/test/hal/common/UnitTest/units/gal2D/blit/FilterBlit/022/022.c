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
 *  Feature: FilterBlit
 *  API:     gco2D_SetTarget  gco2D_SetClipping
 *  Check:
 */

/*
 * It's designed to be independent of the config file.
 */

#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFilterBlit022\n" \
"Operation: Test 9tap filterblit and clear.\n" \
"2D API: gco2D_FilterBlit gco2D_SetTarget gco2D_SetClipping gco2D_LoadSolidBrush\n" \
"Src: Size        [1920x1080]\n"\
"     Rect        [0,0,190,1080]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Dst: Size        [1024x768]\n"\
"     Rect        [0,0,1024,768]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: Type   [SolidBrush]\n"\
"       Format [ARGB8888]\n"\
"       Offset [0]\n" \
"KernelSize: [9]\n" \
"Alphablend: [disable]\n" \
"HW feature dependency: ";

typedef struct Test2D {
    GalTest         base;
    GalRuntime     *runtime;

    T2D_SURF_PTR    srcSurface;
    T2D_SURF_PTR    dstSurface;
}
Test2D;

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gcsRECT srcRect, dstRect;
    gco2D egn2D = t2d->runtime->engine2d;
    gceSTATUS status = gcvSTATUS_OK;

    srcRect.left    = 0;
    srcRect.top     = 0;
    srcRect.right   = t2d->srcSurface->width;
    srcRect.bottom  = t2d->srcSurface->height;

    dstRect.left    = 0;
    dstRect.top     = 0;
    dstRect.right   = t2d->dstSurface->width;
    dstRect.bottom  = t2d->dstSurface->height;

    /* Set target and clipping rect. */
    gcmONERROR(gco2D_SetTarget(egn2D,
        t2d->dstSurface->address[0], t2d->dstSurface->stride[0],
        gcvSURF_0_DEGREE, t2d->dstSurface->width));

    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    /* Set kernel size. */
    gcmONERROR(gco2D_SetKernelSize(egn2D, 9, 9));

    /* Filter blit. */
    gcmONERROR(gco2D_FilterBlit(egn2D,
        t2d->srcSurface->address[0], t2d->srcSurface->stride[0],
        t2d->srcSurface->address[1], t2d->srcSurface->stride[1],
        t2d->srcSurface->address[2], t2d->srcSurface->stride[2],
        t2d->srcSurface->format, gcvSURF_0_DEGREE, t2d->srcSurface->width,
        &srcRect,
        t2d->dstSurface->address[0], t2d->dstSurface->stride[0],
        t2d->dstSurface->format, gcvSURF_0_DEGREE, t2d->dstSurface->width,
        &dstRect, &dstRect));

    /* Set the clear rect. */
    dstRect.left    = 0;
    dstRect.top     = 0;
    dstRect.right   = t2d->dstSurface->width / 2;
    dstRect.bottom  = t2d->dstSurface->height / 2;

    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    /* Clear a part of the target. */
    gcmONERROR(gco2D_LoadSolidBrush(egn2D, t2d->dstSurface->format, gcvTRUE,
        COLOR_ARGB8(0x88, 0x00, 0xFF, 0xFF), 0));

    gcmONERROR(gco2D_Blit(
        egn2D, 1, &dstRect, 0xF0, 0xF0, t2d->dstSurface->format));

    /* Stall the hardware. */
    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    /* Save the result. */
    if (t2d->runtime->saveFullName)
    {
        GalSaveTSurfToDIB(t2d->dstSurface, t2d->runtime->saveFullName);
    }

    return gcvTRUE;

OnError:
    GalOutput(GalOutputType_Error | GalOutputType_Console,
        "%s(%d) failed:%s\n", __FUNCTION__, __LINE__, GalStatusString(status));
    return gcvFALSE;
}

static void CDECL Destroy(Test2D *t2d)
{
    if (t2d->srcSurface != gcvNULL)
    {
        GalDeleteTSurf(t2d->runtime->hal, t2d->srcSurface);
        t2d->srcSurface = gcvNULL;
    }

    if (t2d->dstSurface != gcvNULL)
    {
        GalDeleteTSurf(t2d->runtime->hal, t2d->dstSurface);
        t2d->dstSurface = gcvNULL;
    }

    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_SCALER,
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

    memset(t2d, 0, sizeof(Test2D));

    t2d->runtime = runtime;

    /* Create the source surface. */
    gcmONERROR(GalLoadFileToTSurf(
        "resource/zero1920x1080_A8R8G8B8.bmp", &t2d->srcSurface));

    /* Create the dest surface. */
    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal, gcvSURF_A8R8G8B8, gcvLINEAR, gcv2D_TSC_DISABLE,
        1024, 768, &t2d->dstSurface));

    /* Fill in the base info. */
    t2d->base.render      = (PGalRender)Render;
    t2d->base.destroy     = (PGalDestroy)Destroy;
    t2d->base.description = s_CaseDescription;
    t2d->base.frameCount  = 1;

    runtime->saveTarget    = gcvFALSE;
    runtime->cleanTarget   = gcvFALSE;

    return gcvTRUE;

OnError:
    if (t2d->srcSurface != gcvNULL)
    {
        GalDeleteTSurf(t2d->runtime->hal, t2d->srcSurface);
        t2d->srcSurface = gcvNULL;
    }

    if (t2d->dstSurface != gcvNULL)
    {
        GalDeleteTSurf(t2d->runtime->hal, t2d->dstSurface);
        t2d->dstSurface = gcvNULL;
    }

    GalOutput(GalOutputType_Error | GalOutputType_Console,
                "%s(%d) failed:%s\n", __FUNCTION__, __LINE__,
                GalStatusString(status));

    return gcvFALSE;
}

GalTest * CDECL GalCreateTestObject(GalRuntime *runtime)
{
    Test2D *t2d = (Test2D *)malloc(sizeof(Test2D));

    if (!Init(t2d, runtime))
    {
        free(t2d);
        return NULL;
    }

    return &t2d->base;
}
