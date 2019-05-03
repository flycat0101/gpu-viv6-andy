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
 *  Feature:
 *  API:
 *  Check:
*/
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFormatYUV011\n" \
"Operation: Test format YUV input with brush.\n" \
"2D API: gco2D_Blit\n" \
"Src: Size        [640x480/1280x720]\n"\
"     Rect        [configurable]\n"\
"     Format      [YUY2/UYVY/I420/YV12/NV12/NV21/NV16/NV61]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"     Transparency[masked]\n" \
"Dst: Size        [configurable]\n"\
"     Rect        [configurable]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: Type   [SolidBrush/ColorBrush/MonoBrush]\n"\
"       Format [ARGB8888]\n"\
"       Offset [0]\n" \
"Alphablend: [disable]\n" \
"HW feature dependency: ";

static const char *sBitmapFile[] = {
    "resource/zero2_YUY2_640X480_Linear.vimg",
    "resource/zero2_UYVY_640X480_Linear.vimg",
    "resource/zero2_YUV420_640X480_Linear.vimg",
    "resource/Boston_YV12_640x480_Linear.vimg",
    "resource/Crew_NV12_1280x720_Linear.vimg",
    "resource/Crew_NV16_1280x720_Linear.vimg",
    "resource/Crew_NV21_1280x720_Linear.vimg",
    "resource/Crew_NV61_1280x720_Linear.vimg",
};

#define BRUSH_NUM 3

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
    gctINT            srcStride[3];
    gctINT          srcStrideNum;
    gctINT          srcAddressNum;
    gctUINT32        srcPhyAddr[3];
    gctPOINTER        srcLgcAddr[3];

    gcoBRUSH        brush[BRUSH_NUM];
} Test2D;

static gceSTATUS CDECL ReloadSourceSurface(Test2D *t2d, const char * sourcefile)
{
    gceSTATUS status;

    gctUINT32 address[3];
    gctPOINTER memory[3];

    // destroy source surface
    if (t2d->srcSurf != gcvNULL)
    {
        if (t2d->srcLgcAddr[0])
        {
            gcmONERROR(gcoSURF_Unlock(t2d->srcSurf, t2d->srcLgcAddr));
            t2d->srcLgcAddr[0] = 0;
        }

        gcmONERROR(gcoSURF_Destroy(t2d->srcSurf));
        t2d->srcSurf = gcvNULL;
    }

    // create source surface
    gcmONERROR(GalLoadVimgToSurface(
        sourcefile, &t2d->srcSurf));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->srcSurf,
                                        gcvNULL,
                                        gcvNULL,
                                        t2d->srcStride));

    gcmONERROR(gcoSURF_GetSize(t2d->srcSurf,
                                &t2d->srcWidth,
                                &t2d->srcHeight,
                                gcvNULL));

    gcmONERROR(gcoSURF_GetFormat(t2d->srcSurf, gcvNULL, &t2d->srcFormat));

    gcmONERROR(GalQueryUVStride(t2d->srcFormat, t2d->srcStride[0],
            &t2d->srcStride[1], &t2d->srcStride[2]));

    gcmONERROR(gcoSURF_Lock(t2d->srcSurf, address, memory));

    t2d->srcPhyAddr[0]  = address[0];
    t2d->srcLgcAddr[0]  = memory[0];

    t2d->srcPhyAddr[1] = address[1];
    t2d->srcLgcAddr[1] = memory[1];

    t2d->srcPhyAddr[2] = address[2];
    t2d->srcLgcAddr[2] = memory[2];

    switch (t2d->srcFormat)
    {
    case gcvSURF_YUY2:
    case gcvSURF_UYVY:
        t2d->srcStrideNum = t2d->srcAddressNum = 1;
        break;

    case gcvSURF_I420:
    case gcvSURF_YV12:
        t2d->srcStrideNum = t2d->srcAddressNum = 3;
        break;

    case gcvSURF_NV16:
    case gcvSURF_NV12:
    case gcvSURF_NV61:
    case gcvSURF_NV21:
        t2d->srcStrideNum = t2d->srcAddressNum = 2;
        break;

    default:
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }
    return gcvSTATUS_OK;

OnError:
    return status;
}

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status;
    gcsRECT Rect;
    gco2D egn2D = t2d->runtime->engine2d;

    gcmONERROR(ReloadSourceSurface(t2d, sBitmapFile[frameNo]));

    gcmONERROR(gco2D_FlushBrush(egn2D, t2d->brush[frameNo % BRUSH_NUM], t2d->dstFormat));

    Rect.left = 0;
    Rect.top = 0;
    Rect.right = min(t2d->dstWidth, t2d->srcWidth);
    Rect.bottom = min(t2d->dstHeight, t2d->srcHeight);

    gcmONERROR(gco2D_SetTransparencyAdvanced(egn2D, gcv2D_OPAQUE, gcv2D_OPAQUE, gcv2D_MASKED));

    // set clippint rect
    gcmONERROR(gco2D_SetClipping(egn2D, &Rect));

    gcmONERROR(gco2D_SetSource(egn2D, &Rect));

    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        t2d->srcPhyAddr, t2d->srcAddressNum,
        t2d->srcStride, t2d->srcStrideNum,
        gcvLINEAR,
        t2d->srcFormat,
        0,
        t2d->srcWidth,
        t2d->srcHeight));

    gcmONERROR(gco2D_SetTarget(egn2D, t2d->dstPhyAddr, t2d->dstStride, 0, t2d->dstWidth));

    gcmONERROR(gco2D_Blit(egn2D, 1, &Rect, 0xF0, 0xCC, t2d->dstFormat));

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
    gctINT i;

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
        if (t2d->srcLgcAddr[0])
        {
            if (gcmIS_ERROR(gcoSURF_Unlock(t2d->srcSurf, t2d->srcLgcAddr)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock srcSurf failed:%s\n", GalStatusString(status));
            }
            t2d->srcLgcAddr[0] = 0;
        }

        if (gcmIS_ERROR(gcoSURF_Destroy(t2d->srcSurf)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Surf failed:%s\n", GalStatusString(status));
        }
    }

    for (i = 0; i < BRUSH_NUM; i++)
    {
        if (t2d->brush[i])
        {
            if (gcmIS_ERROR(gcoBRUSH_Destroy(t2d->brush[i])))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy brush failed:%s\n", GalStatusString(status));
            }
        }
    }

    free(t2d);
}

#define RED     COLOR_ARGB8(0, 0x80, 0, 0)
#define GREEN   COLOR_ARGB8(0, 0, 0x80, 0)
#define BLUE    COLOR_ARGB8(0, 0, 0, 0x80)
#define GREY    COLOR_ARGB8(0, 0x80, 0x80, 0x80)
#define WHITE   COLOR_ARGB8(0, 0xFF, 0xFF, 0xFF)
#define BLACK   COLOR_ARGB8(0, 0, 0, 0)

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_2D_YUV_BLIT,
    gcvFEATURE_2D_NO_COLORBRUSH_INDEX8,
    gcvFEATURE_ANDROID_ONLY,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status;
    gctUINT32 color[64] =
    {
        RED, RED, RED, RED, BLUE, BLUE, BLUE, BLUE,
        RED, RED, RED, RED, BLUE, BLUE, BLUE, BLUE,
        RED, RED, RED, RED, BLUE, BLUE, BLUE, BLUE,
        RED, RED, RED, RED, BLUE, BLUE, BLUE, BLUE,
        GREY, GREY, GREY, GREY, GREEN, GREEN, GREEN, GREEN,
        GREY, GREY, GREY, GREY, GREEN, GREEN, GREEN, GREEN,
        GREY, GREY, GREY, GREY, GREEN, GREEN, GREEN, GREEN,
        GREY, GREY, GREY, GREY, GREEN, GREEN, GREEN, GREEN
    };

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
    t2d->srcLgcAddr[0] = gcvNULL;
    t2d->srcFormat = gcvSURF_UNKNOWN;

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    /* Create brushes. */
    gcmONERROR(gco2D_ConstructColorBrush(t2d->runtime->engine2d, 0, 0,
            color, gcvSURF_A8R8G8B8, PATTERN_A, t2d->brush));

    gcmONERROR(gco2D_ConstructSingleColorBrush(t2d->runtime->engine2d, gcvTRUE, WHITE,
            PATTERN_9, t2d->brush + 1));

    gcmONERROR(gco2D_ConstructMonochromeBrush(t2d->runtime->engine2d, 0, 0, gcvTRUE,
            RED, BLUE, 0x00000000FFFFFFFFLL, PATTERN_8, t2d->brush + 2));

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.description = s_CaseDescription;
    t2d->base.frameCount = gcmCOUNTOF(sBitmapFile);

    return gcvTRUE;

OnError:
    GalOutput(GalOutputType_Error | GalOutputType_Console,
        "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));
    return gcvFALSE;
}

GalTest * CDECL GalCreateTestObject(GalRuntime *runtime)
{
    Test2D *t2d = (Test2D *)malloc(sizeof(Test2D));

    memset(t2d, 0, sizeof(Test2D));

    if (!Init(t2d, runtime)) {
        free(t2d);
        return NULL;
    }

    return &t2d->base;
}
