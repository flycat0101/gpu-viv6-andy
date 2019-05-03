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
 *  Feature:    color key
 *  API:
 *  Check:
*/
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFilterRotateDither\n" \
"Operation: Test surface API with filter blit, rotation and dither.\n" \
"2D API: gcoSURF_FilterBlit\n" \
"Src: Size        [592x400]\n"\
"     Rect        [30,25,300,60]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0/90/180/270]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Dst: Size        [640x480]\n"\
"     Rect        [60,100,260,200]\n"\
"     Format      [ARGB4444]\n"\
"     Rotation    [0/90/180/270]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: [None]\n"\
"KernelSize: [1]\n"\
"Alphablend: [disable]\n" \
"Dither: [enable]\n" \
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

    //source surface
    gcoSURF            srcSurf;
    gceSURF_FORMAT    srcFormat;
    gctUINT            srcWidth;
    gctUINT            srcHeight;
    gctINT            srcStride;
    gctUINT32        srcPhyAddr;
    gctPOINTER        srcLgcAddr;

    gcoSURF            tempSurf;
    gctUINT            tempWidth;
    gctUINT            tempHeight;
} Test2D;

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status;
    gco2D egn2D = t2d->runtime->engine2d;
    gcsRECT srcRect = {30, 25, 300, 60};
    gcsRECT destRect = {60, 100, 260, 200};
    gcsRECT destRect2 = {0, 0, 200, 100};

    gcmONERROR(gcoSURF_SetClipping(t2d->tempSurf));

    Gal2DCleanSurface(t2d->runtime->hal, t2d->tempSurf, 0);

    gcmONERROR(gco2D_SetKernelSize(egn2D, 1, 1));

    gcmONERROR(gcoSURF_SetDither(t2d->tempSurf, gcvTRUE));

    switch (frameNo)
    {
    case 0:
        gcoSURF_SetRotation(t2d->srcSurf, gcvSURF_0_DEGREE);
        gcoSURF_SetRotation(t2d->tempSurf, gcvSURF_0_DEGREE);
        break;

    case 1:
        gcoSURF_SetRotation(t2d->srcSurf, gcvSURF_0_DEGREE);
        gcoSURF_SetRotation(t2d->tempSurf, gcvSURF_90_DEGREE);
        break;

    case 2:
        gcoSURF_SetRotation(t2d->srcSurf, gcvSURF_0_DEGREE);
        gcoSURF_SetRotation(t2d->tempSurf, gcvSURF_180_DEGREE);
        break;

    case 3:
        gcoSURF_SetRotation(t2d->srcSurf, gcvSURF_0_DEGREE);
        gcoSURF_SetRotation(t2d->tempSurf, gcvSURF_270_DEGREE);
        break;

    case 4:
        gcoSURF_SetRotation(t2d->srcSurf, gcvSURF_90_DEGREE);
        gcoSURF_SetRotation(t2d->tempSurf, gcvSURF_0_DEGREE);
        break;

    case 5:
        gcoSURF_SetRotation(t2d->srcSurf, gcvSURF_90_DEGREE);
        gcoSURF_SetRotation(t2d->tempSurf, gcvSURF_90_DEGREE);
        break;

    case 6:
        gcoSURF_SetRotation(t2d->srcSurf, gcvSURF_90_DEGREE);
        gcoSURF_SetRotation(t2d->tempSurf, gcvSURF_180_DEGREE);
        break;

    case 7:
        gcoSURF_SetRotation(t2d->srcSurf, gcvSURF_90_DEGREE);
        gcoSURF_SetRotation(t2d->tempSurf, gcvSURF_270_DEGREE);
        break;

    case 8:
        gcoSURF_SetRotation(t2d->srcSurf, gcvSURF_180_DEGREE);
        gcoSURF_SetRotation(t2d->tempSurf, gcvSURF_0_DEGREE);
        break;

    case 9:
        gcoSURF_SetRotation(t2d->srcSurf, gcvSURF_180_DEGREE);
        gcoSURF_SetRotation(t2d->tempSurf, gcvSURF_90_DEGREE);
        break;

    case 10:
        gcoSURF_SetRotation(t2d->srcSurf, gcvSURF_180_DEGREE);
        gcoSURF_SetRotation(t2d->tempSurf, gcvSURF_180_DEGREE);
        break;

    case 11:
        gcoSURF_SetRotation(t2d->srcSurf, gcvSURF_180_DEGREE);
        gcoSURF_SetRotation(t2d->tempSurf, gcvSURF_270_DEGREE);
        break;

    case 12:
        gcoSURF_SetRotation(t2d->srcSurf, gcvSURF_270_DEGREE);
        gcoSURF_SetRotation(t2d->tempSurf, gcvSURF_0_DEGREE);
        break;

    case 13:
        gcoSURF_SetRotation(t2d->srcSurf, gcvSURF_270_DEGREE);
        gcoSURF_SetRotation(t2d->tempSurf, gcvSURF_90_DEGREE);
        break;

    case 14:
        gcoSURF_SetRotation(t2d->srcSurf, gcvSURF_270_DEGREE);
        gcoSURF_SetRotation(t2d->tempSurf, gcvSURF_180_DEGREE);
        break;

    case 15:
        gcoSURF_SetRotation(t2d->srcSurf, gcvSURF_270_DEGREE);
        gcoSURF_SetRotation(t2d->tempSurf, gcvSURF_270_DEGREE);
        break;
    }

    gcmONERROR(gcoSURF_FilterBlit(t2d->srcSurf, t2d->tempSurf, &srcRect, &destRect, &destRect2));
    if (status == gcvSTATUS_NOT_SUPPORT_DITHER)
    {
        GalOutput(GalOutputType_Log | GalOutputType_Console, "WARNING: not supported dithering\n");
    }

    gcmONERROR(gcoSURF_SetRotation(t2d->tempSurf, gcvSURF_0_DEGREE));

    gcmONERROR(gcoSURF_SetClipping(t2d->dstSurf));

    destRect.left = 0;
    destRect.top = 0;
    destRect.right = t2d->dstWidth;
    destRect.bottom = t2d->dstHeight;

    gcmONERROR(gcoSURF_Blit(t2d->tempSurf, t2d->dstSurf, 1, gcvNULL, &destRect, gcvNULL, 0xCC, 0xCC, gcvSURF_OPAQUE, 0, gcvNULL, 0));

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
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock dstSurf failed:%s\n", GalStatusString(status));
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

    // destroy source surface
    if (t2d->tempSurf != gcvNULL)
    {
        if (gcmIS_ERROR(gcoSURF_Destroy(t2d->tempSurf)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Surf failed:%s\n", GalStatusString(status));
        }
    }

    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_SCALER,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status = gcvSTATUS_OK;
    const char *sourcefile = "resource/smooth_YUY2_592X400_Linear.vimg";

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

    // create source surface
    gcmONERROR(GalLoadVimgToSurface(
            sourcefile, &t2d->srcSurf));

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

    t2d->tempWidth = 640;
    t2d->tempHeight = 480;
    gcmONERROR(gcoSURF_Construct(t2d->runtime->hal, t2d->tempWidth, t2d->tempHeight,
        1, gcvSURF_BITMAP, gcvSURF_A4R4G4B4, gcvPOOL_DEFAULT, &t2d->tempSurf));

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = 16;
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
