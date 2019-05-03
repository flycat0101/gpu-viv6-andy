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
 *  Feature:    Masked Source
 *  API:
 *  Check:
 */
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DMaskedSource\n" \
"Operation: Test Surface API with masked source.\n" \
"2D API: gcoSURF_Blit\n" \
"Src: Size        [800x600]\n"\
"     Rect        [change according to frameNo]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"     Transparency[masked]\n" \
"Dst: Size        [configurable]\n"\
"     Rect        [change according to frameNo]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: [None]\n"\
"StreamPack: [UNPACKED/PACKED]\n" \
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

    //source surface
    gcoSURF            srcSurf;
    gceSURF_FORMAT    srcFormat;
    gctUINT            srcWidth;
    gctUINT            srcHeight;
    gctINT            srcStride;
    gctUINT32        srcPhyAddr;
    gctPOINTER        srcLgcAddr;

    //mask
    gceSURF_MONOPACK maskType;
    gctPOINTER maskData;
    gctUINT32 maskWidth;
    gctUINT32 maskHeight;
} Test2D;


static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT8_PTR maskData = gcvNULL;
    gceSURF_MONOPACK maskType;
    gcsRECT srcRect;
    gcsRECT DstRect;
    DstRect.left = DstRect.top = 0;
    DstRect.right = t2d->dstWidth < t2d->maskWidth ? t2d->dstWidth : t2d->maskWidth;
    DstRect.bottom = t2d->dstHeight < t2d->maskHeight ? t2d->dstHeight : t2d->maskHeight;

    // select pack type
    switch (frameNo)
    {
    case 0:
        maskType = gcvSURF_UNPACKED;
        break;

    case 1:
        maskType = gcvSURF_PACKED32;
        break;

    case 2:
        maskType = gcvSURF_PACKED16;
        break;

    case 3:
        maskType = gcvSURF_PACKED8;
        break;

    default:
        return gcvFALSE;
    }

    if (maskType != gcvSURF_UNPACKED)
    {
        gcmONERROR(GalPackStream(t2d->maskData, t2d->maskWidth, t2d->maskHeight, maskType, &maskData));
    }

    srcRect.left = frameNo * 20;
    srcRect.top = frameNo * 10;

    if (t2d->dstWidth > t2d->maskWidth)
    {
        DstRect.right -= srcRect.left * 2;
    }
    if (t2d->dstHeight > t2d->maskHeight)
    {
        DstRect.bottom -= srcRect.top * 2;
    }

    gcoSURF_SetClipping(t2d->dstSurf);
    gcmONERROR(gcoSURF_Blit(t2d->srcSurf, t2d->dstSurf,
                              1, &srcRect, &DstRect,
                              gcvNULL,
                              0xAA, 0xCC,
                              gcvSURF_SOURCE_MASK,
                              0,
                              (maskData ? maskData : t2d->maskData),
                              maskType
                              ));

    if (maskData)
        free(maskData);

    gcmONERROR(gco2D_Flush(t2d->runtime->engine2d));

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

    // destroy mask
    if (t2d->maskData)
        free(t2d->maskData);

    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_ANDROID_ONLY,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status = gcvSTATUS_OK;
    BMPINFO *pInfo;
    gctPOINTER sourcefile = "resource/VV_Background.bmp";
    gctPOINTER maskfile = "resource/Font.bmp";

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

    // create source surface
    t2d->srcSurf = GalLoadDIB2Surface(t2d->runtime->hal, sourcefile);
    if (t2d->srcSurf == NULL)
    {
        GalOutput(GalOutputType_Error, "can not load %s\n", sourcefile);
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

    // create mask surface
     t2d->maskData = GalLoadDIBitmap(maskfile, &pInfo);
    if (t2d->maskData == gcvNULL)
    {
        // destroy source surface
        if (t2d->srcSurf != gcvNULL)
        {
            if (t2d->srcLgcAddr)
            {
                gcmONERROR(gcoSURF_Unlock(t2d->srcSurf, t2d->srcLgcAddr));
                t2d->srcLgcAddr = 0;
            }

            if (gcmIS_ERROR(gcoSURF_Destroy(t2d->srcSurf)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Surf failed:%s\n", GalStatusString(status));
            }
        }
        GalOutput(GalOutputType_Error, "can not open %s\n", maskfile);
        return gcvFALSE;
    }

    t2d->maskWidth = pInfo->bmiHeader.biWidth;
    t2d->maskType = gcvSURF_UNPACKED;
    if (pInfo->bmiHeader.biHeight > 0)
    {
        gctINT i;
        gctINT32 Stride
            = pInfo->bmiHeader.biWidth
            * pInfo->bmiHeader.biBitCount
            / 8;
        gctPOINTER temp;
        gctSTRING bits = t2d->maskData;

        Stride = gcmALIGN(Stride, 4);
        temp = malloc(Stride);
        t2d->maskHeight = pInfo->bmiHeader.biHeight;
        for (i = 0; i < pInfo->bmiHeader.biHeight/2; i++)
        {
            memcpy(temp, bits + i * Stride, Stride);
            memcpy(bits + i * Stride, bits + (pInfo->bmiHeader.biHeight - 1 - i) * Stride, Stride);
            memcpy(bits + (pInfo->bmiHeader.biHeight - 1 - i) * Stride, temp, Stride);
        }
        free(temp);
    }
    else
    {
        t2d->maskHeight = -pInfo->bmiHeader.biHeight;
    }

    free(pInfo);

    // dst with dst surf
    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = 4;
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
