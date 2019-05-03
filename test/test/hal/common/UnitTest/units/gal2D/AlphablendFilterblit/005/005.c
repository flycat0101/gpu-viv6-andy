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
 *  Feature:    Alpha Blending - source and destination factor mode
 *  API:        gco2D_EnableAlphaBlend/gco2D_EnableAlphaBlendAdvanced gco2D_DisableAlphaBlend
 *                gco2D_EnableAlphaBlend is only working with old PE (<2.0) and
 *                gco2D_EnableAlphaBlendAdvanced is only working with PE 2.0 and above.
 *  Check:
 *
*/
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DAlphablendFilterBlit005\n" \
"Operation: Test source and destination factor mode.\n" \
"2D API: gco2D_FilterBlit gco2D_EnableAlphaBlend/gco2D_EnableAlphaBlendAdvanced\n" \
"Src: Size        [configurable]\n"\
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
"Brush: Type   [SolidBrush]\n"\
"       Format [ARGB8888]\n"\
"       Offset [0]\n" \
"KernelSize: [1]\n" \
"Alphablend: [enable]\n" \
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
} Test2D;

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gcsRECT bgRect = {20, 30, 120, 130};
    gcsRECT dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight}, destSubRect;
    gcoBRUSH bgBrush;
    gco2D egn2D = t2d->runtime->engine2d;
    gceSTATUS status;
    gceSURF_BLEND_FACTOR_MODE srcMode, dstMode;

    switch (frameNo)
    {
    case 0:
        srcMode = gcvSURF_BLEND_ZERO;
        dstMode = gcvSURF_BLEND_ZERO;
        break;

    case 1:
        srcMode = gcvSURF_BLEND_ZERO;
        dstMode = gcvSURF_BLEND_ONE;
        break;

    case 2:
        srcMode = gcvSURF_BLEND_ZERO;
        dstMode = gcvSURF_BLEND_STRAIGHT;
        break;

    case 3:
        srcMode = gcvSURF_BLEND_ZERO;
        dstMode = gcvSURF_BLEND_INVERSED;
        break;

    case 4:
        srcMode = gcvSURF_BLEND_ONE;
        dstMode = gcvSURF_BLEND_ZERO;
        break;

    case 5:
        srcMode = gcvSURF_BLEND_ONE;
        dstMode = gcvSURF_BLEND_ONE;
        break;

    case 6:
        srcMode = gcvSURF_BLEND_ONE;
        dstMode = gcvSURF_BLEND_STRAIGHT;
        break;

    case 7:
        srcMode = gcvSURF_BLEND_ONE;
        dstMode = gcvSURF_BLEND_INVERSED;
        break;

    case 8:
        srcMode = gcvSURF_BLEND_STRAIGHT;
        dstMode = gcvSURF_BLEND_ZERO;
        break;

    case 9:
        srcMode = gcvSURF_BLEND_STRAIGHT;
        dstMode = gcvSURF_BLEND_ONE;
        break;

    case 10:
        srcMode = gcvSURF_BLEND_STRAIGHT;
        dstMode = gcvSURF_BLEND_STRAIGHT;
        break;

    case 11:
        srcMode = gcvSURF_BLEND_STRAIGHT;
        dstMode = gcvSURF_BLEND_INVERSED;
        break;

    case 12:
        srcMode = gcvSURF_BLEND_INVERSED;
        dstMode = gcvSURF_BLEND_ZERO;
        break;

    case 13:
        srcMode = gcvSURF_BLEND_INVERSED;
        dstMode = gcvSURF_BLEND_ONE;
        break;

    case 14:
        srcMode = gcvSURF_BLEND_INVERSED;
        dstMode = gcvSURF_BLEND_STRAIGHT;
        break;

    case 15:
        srcMode = gcvSURF_BLEND_INVERSED;
        dstMode = gcvSURF_BLEND_INVERSED;
        break;

    default:
        return gcvFALSE;
    }

    // clear dest surface  with black
    gcmONERROR(Gal2DCleanSurface(t2d->runtime->hal, t2d->dstSurf, COLOR_ARGB8(0xFF, 0x00, 0x00, 0x00)));

    // draw dst rect
    gcmONERROR(gco2D_ConstructSingleColorBrush(egn2D, gcvTRUE,
                COLOR_ARGB8(0x00, 0xFF, 0x00, 0x00), 0, &bgBrush));
    if (gcmIS_ERROR(Gal2DRectangle(t2d->runtime->hal, t2d->dstSurf, bgBrush, bgRect)))
    {
        gcmONERROR(gcvSTATUS_INVALID_REQUEST);
    }
    gcmONERROR(gcoBRUSH_Destroy(bgBrush));

    // enalbe alphablend
    if (t2d->runtime->pe20)
    {
        gcmONERROR(gco2D_SetSourceGlobalColorAdvanced(egn2D, 0xC0000000));
        gcmONERROR(gco2D_SetTargetGlobalColorAdvanced(egn2D, 0xC0000000));

        gcmONERROR(gco2D_EnableAlphaBlendAdvanced(egn2D,
                    gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
                    gcvSURF_GLOBAL_ALPHA_ON, gcvSURF_GLOBAL_ALPHA_ON,
                    srcMode, dstMode));
    }
    else
    {
        gcmONERROR(gco2D_EnableAlphaBlend(egn2D,
                    0xC0, 0xC0,
                    gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
                    gcvSURF_GLOBAL_ALPHA_ON, gcvSURF_GLOBAL_ALPHA_ON,
                    srcMode, dstMode,
                    gcvSURF_COLOR_STRAIGHT, gcvSURF_COLOR_STRAIGHT));

    }

    destSubRect.left   = 0;
    destSubRect.top    = 0;
    destSubRect.right  = dstRect.right  - dstRect.left;
    destSubRect.bottom = dstRect.bottom - dstRect.top;

    gcmONERROR(gco2D_SetKernelSize(egn2D, 1, 1));

    gcmONERROR(gco2D_FilterBlit(egn2D,
            t2d->srcPhyAddr, t2d->srcStride,
            0, 0,
            0, 0,
            t2d->srcFormat, gcvSURF_0_DEGREE, t2d->srcWidth, &dstRect,
            t2d->dstPhyAddr, t2d->dstStride, t2d->dstFormat, gcvSURF_0_DEGREE, t2d->dstWidth, &dstRect,
            &destSubRect));

    // disalbe alphablend
    gcmONERROR(gco2D_DisableAlphaBlend(egn2D));

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

    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_SCALER,
    gcvFEATURE_2D_FILTERBLIT_PLUS_ALPHABLEND,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status;
    gcsRECT fgRect = {50, 60, 250, 260};
    gcoBRUSH fgBrush;

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
    t2d->srcStride = 0;
    t2d->srcPhyAddr = 0;
    t2d->srcLgcAddr = 0;
    t2d->srcFormat = gcvSURF_A8R8G8B8;

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    // create source surface
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

    // clear src surface with black
    gcmONERROR(Gal2DCleanSurface(t2d->runtime->hal, t2d->srcSurf, COLOR_ARGB8(0xFF, 0x00, 0x00, 0x00)));

    // draw src rect
    gcmONERROR(gco2D_ConstructSingleColorBrush(t2d->runtime->engine2d, gcvTRUE,
                COLOR_ARGB8(0x00, 0x00, 0x00, 0xFF), 0, &fgBrush));
    if (gcmIS_ERROR(Gal2DRectangle(t2d->runtime->hal, t2d->srcSurf, fgBrush, fgRect)))
    {
        gcmONERROR(gcvSTATUS_INVALID_REQUEST);
    }
    gcmONERROR(gcoBRUSH_Destroy(fgBrush));

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
