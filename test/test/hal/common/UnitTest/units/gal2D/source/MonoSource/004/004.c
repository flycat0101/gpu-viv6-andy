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
 *  Feature:    MonoSource - relative
 *  API:        gco2D_SetMonochromeSource gco2D_SetSource
 *  Check:      origin
 */
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DMonochromeSource004\n" \
"Operation: Test blit the screen with MonoSource -- test origin.\n" \
"2D API: gco2D_SetMonochromeSource gco2D_MonoBlit\n" \
"Src: Size        [320x200]\n"\
"     Rect        [change according to frameNo]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"     Transparency[sourceMatch]\n" \
"Dst: Size        [configurable]\n"\
"     Rect        [change according to frameNo]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: [None]\n"\
"StreamPack: Src [UNPACKED]\n" \
"            Dst [UNPACKED]\n" \
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

    //monochrome source
    gceSURF_MONOPACK monoSrcDataPackType;
    gctUINT32 *monoSrcData;
    gctUINT32 monoWidth;
    gctUINT32 monoHeight;
} Test2D;

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gcsRECT dstRect =  {0, 0, t2d->dstWidth, t2d->dstHeight};
    gcsRECT srcRect, srcSubRect;
    gco2D egn2D = t2d->runtime->engine2d;
    gceSTATUS status;
    gctUINT32 bgColor = COLOR_ARGB8(0x00, 0xFF, 0x00, 0x00);
    gctUINT32 fgColor = COLOR_ARGB8(0x00, 0x00, 0x00, 0xFF);
    gcsRECT  streamRect;
    gcsPOINT streamSize;

    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    switch (frameNo)
    {
    case 0:
        srcRect.left = 0;
        srcRect.top = 0;
        break;

    case 1:
        srcRect.left = 1;
        srcRect.top = 0;
        break;

    case 2:
        srcRect.left = 0;
        srcRect.top = 1;
        break;

    case 3:
        srcRect.left = 5;
        srcRect.top = 7;
        break;

    case 4:
        srcRect.left = 7;
        srcRect.top = 5;
        break;

    case 5:
        srcRect.left = 21;
        srcRect.top = 30;
        break;

    case 6:
        srcRect.left = 32;
        srcRect.top = 32;
        break;

    default:
        return gcvFALSE;
    }

    gcmONERROR(gco2D_SetMonochromeSource(egn2D,
                                       gcvTRUE,
                                       0,
                                       t2d->monoSrcDataPackType,
                                       gcvFALSE,
                                       gcvSURF_SOURCE_MATCH,
                                       fgColor,
                                       bgColor));

    /* Determine left source coordinate. */
    srcSubRect.left = srcRect.left & 31;

    /* Set the rectangle value. */
    srcSubRect.top = srcSubRect.right = srcSubRect.bottom = 0;
    gcmONERROR(gco2D_SetSource(egn2D, &srcSubRect));

    gcmONERROR(gco2D_SetTarget(egn2D, t2d->dstPhyAddr, t2d->dstStride, 0, t2d->dstWidth));

    streamRect.left = srcRect.left - srcSubRect.left;
    streamRect.top = srcRect.top;
    streamRect.right = t2d->monoWidth;
    streamRect.bottom = t2d->monoHeight;
    streamSize.x = t2d->monoWidth;
    streamSize.y = t2d->monoHeight;

    dstRect.left = dstRect.top = 0;
    dstRect.right = streamRect.right - srcRect.left;
    dstRect.bottom = streamRect.bottom - srcRect.top;

    gcmONERROR(gco2D_MonoBlit(egn2D, (gctUINT8_PTR)t2d->monoSrcData, &streamSize,
        &streamRect, t2d->monoSrcDataPackType, gcvSURF_UNPACKED, &dstRect, 0xCC, 0xCC, t2d->dstFormat));

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

    if (t2d->monoSrcData)
        free(t2d->monoSrcData);

    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_ANDROID_ONLY,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT srcSize, i;

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

    t2d->monoWidth = 320;
    t2d->monoHeight = 200;
    t2d->monoSrcDataPackType = gcvSURF_UNPACKED;
    srcSize = t2d->monoWidth * t2d->monoHeight >> 5;
    t2d->monoSrcData = (gctUINT32*)malloc(srcSize * sizeof(gctUINT32));
    for (i = 0; i < srcSize; i++)
    {
        *(t2d->monoSrcData + i) = CONVERT_BYTE(i);
    }

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = 7;
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
