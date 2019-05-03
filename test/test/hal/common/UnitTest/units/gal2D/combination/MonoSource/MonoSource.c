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
 *  Feature:    monochrome source
 *  API:
 *  Check:
 */
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DMonoSource\n" \
"Operation: Test surface API with monochrome source.\n" \
"2D API: gcoSURF_MonoBlit\n" \
"Src: Size        [800x600]\n"\
"     Rect        [change according to frameNo]\n"\
"     Format      [INDEX1]\n"\
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

    //mono source
    gceSURF_MONOPACK monoType;
    gctPOINTER monoData;
    gctUINT32 monoWidth;
    gctUINT32 monoHeight;
} Test2D;

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT8_PTR monoData = gcvNULL;
    gceSURF_MONOPACK monoType;
    gcsPOINT sourceSize = {t2d->monoWidth, t2d->monoHeight};
    gctUINT32 RED = 0x00FF0000, GREEN = 0x0000FF00;
    gcsPOINT sourceOrigin;

    gcsRECT DstRect;
    DstRect.left = DstRect.top = 0;
    DstRect.right = t2d->dstWidth < t2d->monoWidth ? t2d->dstWidth : t2d->monoWidth;
    DstRect.bottom = t2d->dstHeight < t2d->monoHeight ? t2d->dstHeight : t2d->monoHeight;

    // select pack type
    switch (frameNo)
    {
    case 0:
        monoType = gcvSURF_UNPACKED;
        break;

    case 1:
        monoType = gcvSURF_PACKED32;
        break;

    case 2:
        monoType = gcvSURF_PACKED16;
        break;

    case 3:
        monoType = gcvSURF_PACKED8;
        break;

    default:
        return gcvFALSE;
    }

    if (monoType != gcvSURF_UNPACKED)
    {
        gcmONERROR(GalPackStream(t2d->monoData, t2d->monoWidth, t2d->monoHeight, monoType, &monoData));
    }

    sourceOrigin.x = frameNo * 20;
    sourceOrigin.y = frameNo * 10;


    gcoSURF_SetClipping(t2d->dstSurf);
    gcmONERROR(gcoSURF_Blit(t2d->srcSurf, t2d->dstSurf,
                              1, gcvNULL, gcvNULL,
                              gcvNULL,
                              0xCC, 0xCC,
                              gcvSURF_OPAQUE,
                              0,
                              gcvNULL,
                              0
                              ));

    if (t2d->dstWidth > t2d->monoWidth)
    {
        DstRect.right -= sourceOrigin.x * 2;
    }
    if (t2d->dstHeight > t2d->monoHeight)
    {
        DstRect.bottom -= sourceOrigin.y * 2;
    }

    gcmONERROR(gcoSURF_MonoBlit(t2d->dstSurf, (monoData ? monoData : t2d->monoData), monoType, &sourceSize,
        &sourceOrigin, &DstRect, gcvNULL, 0xCC, 0xAA, gcvTRUE, GREEN, gcvSURF_SOURCE_MATCH, RED, GREEN));

    if (monoData)
        free(monoData);

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
            t2d->srcLgcAddr = gcvNULL;
        }

        if (gcmIS_ERROR(gcoSURF_Destroy(t2d->srcSurf)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Surf failed:%s\n", GalStatusString(status));
        }
    }

    // destroy mask
    if (t2d->monoData)
        free(t2d->monoData);

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
     t2d->monoData = GalLoadDIBitmap(maskfile, &pInfo);
    if (t2d->monoData == gcvNULL)
    {
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

    t2d->monoWidth = pInfo->bmiHeader.biWidth;
    t2d->monoType = gcvSURF_UNPACKED;
    if (pInfo->bmiHeader.biHeight > 0)
    {
        gctINT i;
        gctINT32 Stride
            = pInfo->bmiHeader.biWidth
            * pInfo->bmiHeader.biBitCount
            / 8;
        gctPOINTER temp;
        gctSTRING bits = t2d->monoData;

        Stride = gcmALIGN(Stride, 4);
        temp = malloc(Stride);
        t2d->monoHeight = pInfo->bmiHeader.biHeight;
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
        t2d->monoHeight = -pInfo->bmiHeader.biHeight;
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
