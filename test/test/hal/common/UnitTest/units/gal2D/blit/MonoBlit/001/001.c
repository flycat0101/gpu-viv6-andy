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
 *  Feature:    2DMonoBlit - ROP
 *  API:        gco2D_MonoBlit gco2D_SetMonochromeSource
 *  Check:
 */
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DMonoBlit001\n" \
"Operation: Test ROP value.\n" \
"2D API: gco2D_MonoBlit gco2D_SetMonochromeSource\n" \
"Src: Size        [None]\n"\
"     Rect        [None]\n"\
"     Format      [None]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Dst: Size        [configurable]\n"\
"     Rect        [320x200]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: Type   [SolidBrush]\n"\
"       Format [ARGB8888]\n"\
"       Offset [0]\n" \
"StreamPack: [UNPACKED]\n" \
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

    //monochrome source
    gceSURF_MONOPACK monoSrcDataPackType;
    gctUINT32 *monoSrcData;
    gctUINT32 monoWidth;
    gctUINT32 monoHeight;
} Test2D;

gctUINT8 sRopList[] = {
        0xF0, // brush
        0xCC, // src
        0xAA, // dst
         0x0F, // brush converse
        0x33, // src converse
        0x55, // dst converse
        0xFF, // white
        0x00, // black

        0xA0, // brush AND dst
        0xFA, // brush OR dst
        0x5A, // brush XOR dst
        0xC0, // brush AND src
        0xFC, // brush OR src
        0x3C, // brush XOR src
        0x88, // dst AND src
        0xEE, // dst OR src
        0x66, // dst XOR src

        0xFE, // brush  OR src  OR dst
        0xF8, // brush  OR src AND dst
        0x56, // brush  OR src XOR dst
        0xEA, // brush AND src  OR dst
        0x80, // brush AND src AND dst
        0x6A, // brush AND src XOR dst
        0xBE, // brush XOR src  OR dst
        0x28, // brush XOR src AND dst
        0x96, // brush XOR src XOR dst

        //0xFE, // brush  OR dst  OR src
        0xC8, // brush  OR dst AND src
        0x36, // brush  OR dst XOR src
        0xEC, // brush AND dst  OR src
        //0x80, // brush AND dst AND src
        0x0C, // brush AND dst XOR src
        0xDE, // brush XOR dst  OR src
        0x48, // brush XOR dst AND src
        //0x96, // brush XOR dst XOR src

        //0xFE, // src  OR dst  OR brush
        0xE0, // src  OR dst AND brush
        0x1E, // src  OR dst XOR brush
        //0xF8, // src AND dst  OR brush
        //0x80, // src AND dst AND brush
        0x78, // src AND dst XOR brush
        0xF6, // src XOR dst  OR brush
        0x60, // src XOR dst AND brush
        //0x96, // src XOR dst XOR brush
};

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gcsRECT dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};
    gcsRECT srcRect = {0, 0, 0, 0};
    gco2D egn2D = t2d->runtime->engine2d;
    gceSTATUS status;
    gctUINT8 ROP = sRopList[frameNo];
    gcsRECT  streamRect;
    gcsPOINT streamSize;

    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    // clear dst surface with blue
    gcmONERROR(Gal2DCleanSurface(t2d->runtime->hal, t2d->dstSurf, COLOR_ARGB8(0x00, 0x00, 0x00, 0xFF)));

    // set brush
    gcmONERROR(gco2D_FlushBrush(egn2D, t2d->brush, t2d->dstFormat));

    // set mono source
    gcmONERROR(gco2D_SetMonochromeSource(egn2D,
                                       gcvTRUE,
                                       0,
                                       t2d->monoSrcDataPackType,
                                       gcvFALSE,
                                       gcvSURF_OPAQUE,
                                       COLOR_ARGB8(0x00, 0x00, 0xFF, 0x00),
                                       COLOR_ARGB8(0x00, 0xFF, 0x00, 0xFF)));

    gcmONERROR(gco2D_SetSource(egn2D, &srcRect));

    gcmONERROR(gco2D_SetTarget(egn2D, t2d->dstPhyAddr, t2d->dstStride, gcvSURF_0_DEGREE, t2d->dstWidth));

    dstRect.left = dstRect.top = streamRect.left = streamRect.top = 0;
    dstRect.right = streamRect.right = streamSize.x = t2d->monoWidth;
    dstRect.bottom = streamRect.bottom = streamSize.y = t2d->monoHeight;

    gcmONERROR(gco2D_MonoBlit(egn2D, (gctUINT8_PTR)t2d->monoSrcData, &streamSize,
        &streamRect, t2d->monoSrcDataPackType, gcvSURF_UNPACKED, &dstRect, ROP, ROP, t2d->dstFormat));

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
        if(gcoSURF_Unlock(t2d->dstSurf, t2d->dstLgcAddr))
        t2d->dstLgcAddr = gcvNULL;
    }

    // destroy brush
    if (t2d->brush != gcvNULL)
    {
        if (gcmIS_ERROR(gcoBRUSH_Destroy(t2d->brush)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console,
                "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));
        }
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
    gceSTATUS status;
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

    t2d->brush = gcvNULL;

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    // create red brush
    gcmONERROR(gco2D_ConstructSingleColorBrush(t2d->runtime->engine2d , (t2d->dstFormat!=gcvSURF_A8R8G8B8),
                COLOR_ARGB8(0x00, 0xFF, 0x00, 0x00), 0, &t2d->brush));

    // Mono source
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
    t2d->base.frameCount = sizeof(sRopList) / sizeof(sRopList[0]);
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
