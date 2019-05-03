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
 *  Feature:    2DMonoBlit - rotation
 *  API:        gco2D_MonoBlitEx
 *  Check:
 */
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DMonoBlit003\n" \
"Operation: gco2D_MonoBlitEx with rotation.\n" \
"2D API: gco2D_MonoBlitEx\n" \
"Src: Size        [None]\n"\
"     Rect        [None]\n"\
"     Format      [None]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Dst: Size        [640x640]\n"\
"     Rect        [change according to frameNo]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: [None]\n"\
"StreamPack: [PACKED]\n" \
"StreamOffset: X [frameNo+57]\n" \
"              Y [frmaeNo+19]\n" \
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

    // tgt
    gcoSURF            tgtSurf;
    gceSURF_FORMAT    tgtFormat;
    gctUINT            tgtWidth;
    gctUINT            tgtHeight;
    gctINT            tgtStride;
    gctUINT32        tgtPhyAddr;
    gctPOINTER        tgtLgcAddr;

    //monochrome source
    gctUINT8 *monoData;
    gctINT32 monoStride;
    gctINT32 monoWidth;
    gctINT32 monoHeight;
} Test2D;

gceSURF_ROTATION rotList[] =
{
    gcvSURF_0_DEGREE,
    gcvSURF_90_DEGREE,
    gcvSURF_180_DEGREE,
    gcvSURF_270_DEGREE,
    gcvSURF_FLIP_X,
    gcvSURF_FLIP_Y,
};

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status;
    gcsRECT tgtRect = {0, 0, t2d->tgtWidth, t2d->tgtHeight};
    gcsRECT dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};
    gco2D egn2D = t2d->runtime->engine2d;
    gceSURF_ROTATION tgtRot = rotList[frameNo % gcmCOUNTOF(rotList)];
    gctINT w = 640 - frameNo * 17;
    gctINT h = 480 - frameNo * 13;

    gcmONERROR(gco2D_SetClipping(egn2D, &tgtRect));

    gcmONERROR(Gal2DCleanSurface(gcvNULL, t2d->tgtSurf, COLOR_ARGB8(0, 128, 255, 0)));

    gcmONERROR(gco2D_SetTransparencyAdvanced(egn2D, gcv2D_OPAQUE, gcv2D_OPAQUE, gcv2D_OPAQUE));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        &t2d->tgtPhyAddr, 1,
        &t2d->tgtStride, 1,
        gcvLINEAR,
        t2d->tgtFormat,
        tgtRot,
        t2d->tgtWidth,
        t2d->tgtHeight));

    tgtRect.left = frameNo * 3 + 11;
    tgtRect.right = tgtRect.left + w;
    tgtRect.top = frameNo * 7 + 5;
    tgtRect.bottom = tgtRect.top + h;

    gcmONERROR(gco2D_MonoBlitEx(
        egn2D,
        t2d->monoData,
        t2d->monoStride,
        t2d->monoWidth,
        t2d->monoHeight,
        frameNo + 57,
        frameNo + 19,
        COLOR_ARGB8(0, 128, 255, 0),
        COLOR_ARGB8(0, 0xFF, 0, 0),
        gcvNULL,
        &tgtRect,
        0xCC,
        0xCC));


    // Output.
    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        &t2d->tgtPhyAddr, 1,
        &t2d->tgtStride, 1,
        gcvLINEAR,
        t2d->tgtFormat,
        gcvSURF_0_DEGREE,
        t2d->tgtWidth,
        t2d->tgtHeight));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        &t2d->dstPhyAddr, 1,
        &t2d->dstStride, 1,
        gcvLINEAR,
        t2d->dstFormat,
        gcvSURF_0_DEGREE,
        t2d->dstWidth,
        t2d->dstHeight));

    tgtRect.left   = 0;
    tgtRect.top    = 0;
    tgtRect.right  = t2d->tgtWidth;
    tgtRect.bottom = t2d->tgtHeight;

    gcmONERROR(gco2D_SetSource(egn2D, &tgtRect));

    gcmONERROR(gco2D_SetStretchRectFactors(
        egn2D,
        &tgtRect,
        &dstRect));

    gcmONERROR(gco2D_StretchBlit(
        egn2D,
        1,
        &dstRect,
        0xCC,
        0xCC,
        t2d->dstFormat));

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
        if(gcoSURF_Unlock(t2d->dstSurf, t2d->dstLgcAddr))
        t2d->dstLgcAddr = gcvNULL;
    }

    if (t2d->monoData)
        free(t2d->monoData);

    // Destroy target surface
    if (t2d->tgtSurf != gcvNULL)
    {
        if (t2d->tgtLgcAddr)
        {
            if (gcmIS_ERROR(gcoSURF_Unlock(t2d->tgtSurf, t2d->tgtLgcAddr)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock tgtSurf failed:%s\n", GalStatusString(status));
            }
            t2d->tgtLgcAddr = 0;
        }

        if (gcmIS_ERROR(gcoSURF_Destroy(t2d->tgtSurf)))
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
    gceSTATUS status;
    gctSTRING maskfile = "resource/Font.bmp";
    BMPINFO *pInfo = gcvNULL;

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

    if (runtime->ChipModel == gcv320 && runtime->ChipRevision == 0x5007)
    {
        GalOutput(GalOutputType_Result | GalOutputType_Console, "MonoBlit is not supported.\n");
        runtime->notSupport = gcvTRUE;
    }

    if (runtime->notSupport)
        return gcvFALSE;

    memset(t2d, 0, sizeof(Test2D));

    t2d->runtime = runtime;

    t2d->dstSurf    = runtime->target;
    t2d->dstFormat = runtime->format;
    t2d->dstWidth = 0;
    t2d->dstHeight = 0;
    t2d->dstStride = 0;
    t2d->dstPhyAddr = 0;
    t2d->dstLgcAddr = 0;

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    // Mono source
     t2d->monoData = GalLoadDIBitmap(maskfile, &pInfo);
    if (t2d->monoData == gcvNULL)
    {
        GalOutput(GalOutputType_Error, "can not open %s\n", maskfile);
        gcmONERROR(gcvSTATUS_NOT_FOUND);
    }

    t2d->monoWidth = pInfo->bmiHeader.biWidth;
    t2d->monoStride =
        pInfo->bmiHeader.biWidth * pInfo->bmiHeader.biBitCount / 8;
    if (pInfo->bmiHeader.biHeight > 0)
    {
        gctINT i;

        t2d->monoHeight = pInfo->bmiHeader.biHeight;

        for (i = 0; i < pInfo->bmiHeader.biHeight/2; i++)
        {
            gctINT j;
            gctUINT8_PTR d1 = t2d->monoData + i * t2d->monoStride;
            gctUINT8_PTR d2 = t2d->monoData + (pInfo->bmiHeader.biHeight - 1 - i) * t2d->monoStride;

            for (j = 0; j < t2d->monoStride; j++)
            {
                gctUINT8 tmp = d1[j];
                d1[j] = d2[j];
                d2[j] = tmp;
            }
        }
    }
    else
    {
        t2d->monoHeight = -pInfo->bmiHeader.biHeight;
    }

    free(pInfo);

    // create target surface
    gcmONERROR(gcoSURF_Construct(
        gcvNULL,
        640,
        640,
        1,
        gcvSURF_BITMAP,
        gcvSURF_A8R8G8B8,
        gcvPOOL_DEFAULT,
        &t2d->tgtSurf));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->tgtSurf,
                                        &t2d->tgtWidth,
                                        &t2d->tgtHeight,
                                        &t2d->tgtStride));

    gcmONERROR(gcoSURF_GetFormat(t2d->tgtSurf,
                                        gcvNULL,
                                        &t2d->tgtFormat));

    gcmONERROR(gcoSURF_Lock(t2d->tgtSurf, &t2d->tgtPhyAddr, &t2d->tgtLgcAddr));

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = 36;
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
