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


#include <galUtil.h>

static gctCONST_STRING s_CaseDescription =
"Case gal2DFilterRotateDither002\n" \
"Operation: Test surface API with filter blit, rotation and dither.\n" \
"2D API: gcoSURF_FilterBlit\n" \
"Src: Size        [1280x720]\n"\
"     Rect        [0,0,1280,720]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Dst: Size        [configurable]\n"\
"     Rect        [configurable and variable]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: [None]\n"\
"KernelSize: [9]\n"\
"Alphablend: [disable]\n" \
"HW feature dependency: ";

typedef struct Test2D
{
    GalTest         base;
    GalRuntime      *runtime;

    // destination surface
    gcoSURF         dstSurf;
    gceSURF_FORMAT  dstFormat;
    gctUINT         dstWidth;
    gctUINT         dstHeight;
    gctINT          dstStride;
    gctUINT32       dstPhyAddr;
    gctPOINTER      dstLgcAddr;

    //source surface
    gcoSURF         srcSurf;
    gceSURF_FORMAT  srcFormat;
    gctUINT         srcWidth;
    gctUINT         srcHeight;
    gctINT          srcStride;
    gctUINT32       srcPhyAddr;
    gctPOINTER      srcLgcAddr;
}
Test2D;

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status = gcvSTATUS_OK;
    gco2D egn2D      = t2d->runtime->engine2d;
    gcsRECT srcRect  = { 0, 0, t2d->srcWidth, t2d->srcHeight };
    gcsRECT destRect;
    float scale;
    int width, height;

    gcmONERROR(gcoSURF_SetClipping(t2d->dstSurf));

    Gal2DCleanSurface(t2d->runtime->hal, t2d->dstSurf, 0);

    gcmONERROR(gco2D_SetKernelSize(egn2D, 9, 9));

    gcmONERROR(gcoSURF_SetDither(t2d->dstSurf, gcvTRUE));

    switch (frameNo)
    {
    case 0:
        scale           = min((float) t2d->dstWidth  / (float) t2d->srcWidth,
                              (float) t2d->dstHeight / (float) t2d->srcHeight);
        width           = (int) (t2d->srcWidth  * scale);
        height          = (int) (t2d->srcHeight * scale);
        destRect.left   = (t2d->dstWidth  - width)  / 2;
        destRect.top    = (t2d->dstHeight - height) / 2;
        destRect.right  = destRect.left + width;
        destRect.bottom = destRect.top  + height;
        gcoSURF_SetRotation(t2d->dstSurf, gcvSURF_0_DEGREE);
        break;

    case 1:
        scale           = min((float) t2d->dstHeight / (float) t2d->srcWidth,
                              (float) t2d->dstWidth  / (float) t2d->srcHeight);
        width           = (int) (t2d->srcWidth  * scale);
        height          = (int) (t2d->srcHeight * scale);
        destRect.left   = (t2d->dstHeight - width)  / 2;
        destRect.top    = (t2d->dstWidth  - height) / 2;
        destRect.right  = destRect.left + width;
        destRect.bottom = destRect.top  + height;
        gcoSURF_SetRotation(t2d->dstSurf, gcvSURF_90_DEGREE);
        break;

    case 2:
        scale           = min((float) t2d->dstWidth  / (float) t2d->srcWidth,
                              (float) t2d->dstHeight / (float) t2d->srcHeight);
        width           = (int) (t2d->srcWidth  * scale);
        height          = (int) (t2d->srcHeight * scale);
        destRect.left   = (t2d->dstWidth  - width)  / 2;
        destRect.top    = (t2d->dstHeight - height) / 2;
        destRect.right  = destRect.left + width;
        destRect.bottom = destRect.top  + height;
        gcoSURF_SetRotation(t2d->dstSurf, gcvSURF_180_DEGREE);
        break;

    case 3:
        scale           = min((float) t2d->dstHeight / (float) t2d->srcWidth,
                              (float) t2d->dstWidth  / (float) t2d->srcHeight);
        width           = (int) (t2d->srcWidth  * scale);
        height          = (int) (t2d->srcHeight * scale);
        destRect.left   = (t2d->dstHeight - width)  / 2;
        destRect.top    = (t2d->dstWidth  - height) / 2;
        destRect.right  = destRect.left + width;
        destRect.bottom = destRect.top  + height;
        gcoSURF_SetRotation(t2d->dstSurf, gcvSURF_270_DEGREE);
        break;
    }

    //printf("%d: %dx%d %d,%d - %d,%d\n", frameNo, width, height,
    //       destRect.left, destRect.top, destRect.right, destRect.bottom);

    gcmONERROR(gcoSURF_FilterBlit(t2d->srcSurf,
                                    t2d->dstSurf,
                                    &srcRect,
                                    &destRect, gcvNULL));

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
        if (gcmIS_ERROR((gcoSURF_Unlock(t2d->dstSurf, t2d->dstLgcAddr))))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock desSurf failed:%s\n", GalStatusString(status));
        }
        t2d->dstLgcAddr = gcvNULL;
    }

    // destroy source surface
    if (t2d->srcSurf != gcvNULL)
    {
        if (t2d->srcLgcAddr != gcvNULL)
        {
            if (gcmIS_ERROR((gcoSURF_Unlock(t2d->srcSurf, t2d->srcLgcAddr))))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock desSurf failed:%s\n", GalStatusString(status));
        }
            t2d->srcLgcAddr = gcvNULL;
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
    gcvFEATURE_SCALER,
    gcvFEATURE_ANDROID_ONLY,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    const char *sourcefile = "resource/smooth_720p.bmp";
    gceSTATUS status;

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

    t2d->runtime    = runtime;
    t2d->dstSurf    = runtime->target;
    t2d->dstFormat  = runtime->format;
    t2d->dstWidth   = 0;
    t2d->dstHeight  = 0;
    t2d->dstStride  = 0;
    t2d->dstPhyAddr = 0;
    t2d->dstLgcAddr = 0;

    // create source surface
    t2d->srcSurf = GalLoadDIB2Surface(t2d->runtime->hal, sourcefile);
    if (t2d->srcSurf == gcvNULL)
    {
        GalOutput(GalOutputType_Error | GalOutputType_Console, "Load source surface failed.\n");
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

    gcmONERROR(gcoSURF_GetFormat(t2d->srcSurf,
                                   gcvNULL,
                                   &t2d->srcFormat));

    gcmONERROR(gcoSURF_Lock(t2d->srcSurf,
                              &t2d->srcPhyAddr,
                              &t2d->srcLgcAddr));

    gcmONERROR(gcoSURF_GetSize(t2d->dstSurf,
                                &t2d->dstWidth,
                                &t2d->dstHeight,
                                gcvNULL));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        gcvNULL,
                                        gcvNULL,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf,
                              &t2d->dstPhyAddr,
                              &t2d->dstLgcAddr));

    t2d->base.render      = (PGalRender) Render;
    t2d->base.destroy     = (PGalDestroy) Destroy;
    t2d->base.frameCount  = 4;
    t2d->base.description = s_CaseDescription;

    return gcvTRUE;

OnError:
    GalOutput(GalOutputType_Error | GalOutputType_Console,
        "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));
    return gcvFALSE;
}

GalTest * CDECL GalCreateTestObject(GalRuntime *runtime)
{
    Test2D *t2d = (Test2D *) malloc(sizeof(Test2D));

    if (!Init(t2d, runtime))
    {
        free(t2d);
        return NULL;
    }

    return &t2d->base;
}

