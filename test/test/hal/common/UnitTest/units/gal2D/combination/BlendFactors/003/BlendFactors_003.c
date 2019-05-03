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
"Case gal2DBlendFactors003\n" \
"Operation: Test source and destination factor mode (FullDFB).\n" \
"2D API: gco2D_EnableAlphaBlend/gco2D_EnableAlphaBlendAdvanced gco2D_DisableAlphaBlend\n" \
"Src: Size        [configurable]\n"\
"     Rect        [configurable]\n"\
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
    gctUINT            dstAlignedWidth;
    gctUINT            dstAlignedHeight;
    gctINT            dstStride;
    gctUINT32        dstPhyAddr;
    gctPOINTER        dstLgcAddr;

    //source surface
    gcoSURF            srcSurf;
    gceSURF_FORMAT    srcFormat;
    gctUINT            srcWidth;
    gctUINT            srcHeight;
    gctUINT            srcAlignedWidth;
    gctUINT            srcAlignedHeight;
    gctINT            srcStride;
    gctUINT32        srcPhyAddr;
    gctPOINTER        srcLgcAddr;

    // alpha blending factor mode number
    gctINT            factorNum;

    // x and y steps
    gctINT            xStep;
    gctINT            yStep;
} Test2D;

gceSURF_BLEND_FACTOR_MODE srcFactorMode[] = {
    gcvSURF_BLEND_ZERO,
    gcvSURF_BLEND_ONE,
    gcvSURF_BLEND_STRAIGHT,
    gcvSURF_BLEND_INVERSED,
    gcvSURF_BLEND_COLOR,
    gcvSURF_BLEND_COLOR_INVERSED,
    gcvSURF_BLEND_STRAIGHT_NO_CROSS,
    gcvSURF_BLEND_INVERSED_NO_CROSS,
    gcvSURF_BLEND_COLOR_NO_CROSS,
    gcvSURF_BLEND_COLOR_INVERSED_NO_CROSS,
    gcvSURF_BLEND_SRC_ALPHA_SATURATED,
};

gceSURF_BLEND_FACTOR_MODE dstFactorMode[] = {
    gcvSURF_BLEND_ZERO,
    gcvSURF_BLEND_ONE,
    gcvSURF_BLEND_STRAIGHT,
    gcvSURF_BLEND_INVERSED,
    gcvSURF_BLEND_COLOR,
    gcvSURF_BLEND_COLOR_INVERSED,
    gcvSURF_BLEND_STRAIGHT_NO_CROSS,
    gcvSURF_BLEND_INVERSED_NO_CROSS,
    gcvSURF_BLEND_COLOR_NO_CROSS,
    gcvSURF_BLEND_COLOR_INVERSED_NO_CROSS,
    gcvSURF_BLEND_SRC_ALPHA_SATURATED_CROSS
};

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gcsRECT srcRect, dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};
    gco2D egn2D = t2d->runtime->engine2d;
    gceSURF_GLOBAL_ALPHA_MODE globalMode;
    gctINT i, j;

    gceSTATUS status;

    switch (frameNo)
    {
    case 0:
        globalMode = gcvSURF_GLOBAL_ALPHA_ON;
        break;

    case 1:
        globalMode = gcvSURF_GLOBAL_ALPHA_OFF;
        break;

    case 2:
        globalMode = gcvSURF_GLOBAL_ALPHA_SCALE;
        break;

    default:
        return gcvFALSE;
    }

    // src rect
    srcRect.left   = 0;
    srcRect.top    = 0;
    srcRect.right  = t2d->srcWidth;
    srcRect.bottom = t2d->srcHeight;

    gcmONERROR(Gal2DCleanSurface(t2d->runtime->hal, t2d->dstSurf, COLOR_ARGB8(0x50, 0xFF, 0x00, 0x00)));

    for (i = 0; i < t2d->factorNum; i++)
    {
        for (j = 0; j < t2d->factorNum; j++)
        {
            if ((i < 6) && (j < 6))
            {
                continue;
            }

            dstRect.left   = t2d->xStep * j;
            dstRect.top    = t2d->yStep * i;
            dstRect.right  = dstRect.left + t2d->srcWidth;
            dstRect.bottom = dstRect.top + t2d->srcHeight;

            // set color source and src rect
            if (t2d->runtime->pe20)
            {
                gcmONERROR(gco2D_SetColorSourceAdvanced(egn2D, t2d->srcPhyAddr, t2d->srcStride, t2d->srcFormat,
                                gcvSURF_0_DEGREE, t2d->srcAlignedWidth, t2d->srcAlignedHeight, gcvFALSE));
            }
            else
            {
                gcmONERROR(gco2D_SetColorSource(egn2D, t2d->srcPhyAddr, t2d->srcStride, t2d->srcFormat,
                                gcvSURF_0_DEGREE, t2d->srcAlignedWidth, gcvFALSE, gcvSURF_OPAQUE, 0));
            }

            gcmONERROR(gco2D_SetSource(egn2D, &srcRect));

            gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

            gcmONERROR(gco2D_SetTarget(egn2D, t2d->dstPhyAddr, t2d->dstStride, gcvFALSE, t2d->dstAlignedWidth));

            // enalbe alphablend
            if (t2d->runtime->pe20)
            {
                gcmONERROR(gco2D_SetSourceGlobalColorAdvanced(egn2D, 0xC0000000));
                gcmONERROR(gco2D_SetTargetGlobalColorAdvanced(egn2D, 0xC0000000));

                status = gco2D_EnableAlphaBlendAdvanced(egn2D,
                            gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
                            globalMode, gcvSURF_GLOBAL_ALPHA_OFF,
                            srcFactorMode[i], dstFactorMode[j]);
            }
            else
            {
                status = gco2D_EnableAlphaBlend(egn2D,
                            0xC0, 0xC0,
                            gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
                            globalMode, globalMode,
                            srcFactorMode[i], dstFactorMode[j],
                            gcvSURF_COLOR_STRAIGHT, gcvSURF_COLOR_STRAIGHT);

            }

            if (status != gcvSTATUS_OK)
            {
                GalOutput(GalOutputType_Error, "enalbe AlphaBlending failed:%s\n", GalStatusString(status));

                gcmONERROR(gco2D_DisableAlphaBlend(egn2D));

                return gcvFALSE;
            }

            gcmONERROR(gco2D_Blit(egn2D, 1, &dstRect, 0xCC, 0xCC, t2d->dstFormat));

        }
    }

    // disalbe alphablend
    status = gco2D_DisableAlphaBlend(egn2D);
    if (status != gcvSTATUS_OK)
    {
        GalOutput(GalOutputType_Error, "disalbe AlphaBlending failed:%s\n", GalStatusString(status));
        return gcvFALSE;
    }

    gcmONERROR(gco2D_Flush(egn2D));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    return gcvTRUE;

OnError:

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

    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_FULL_DIRECTFB,
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

    t2d->runtime = runtime;

    t2d->dstSurf    = runtime->target;
    t2d->dstFormat  = runtime->format;
    t2d->dstWidth   = 0;
    t2d->dstHeight  = 0;
    t2d->dstStride  = 0;
    t2d->dstPhyAddr = 0;
    t2d->dstLgcAddr = 0;

    t2d->srcSurf    = gcvNULL;
    t2d->srcWidth   = 0;
    t2d->srcHeight  = 0;
    t2d->srcStride  = 0;
    t2d->srcPhyAddr = 0;
    t2d->srcLgcAddr = 0;
    t2d->srcFormat  = gcvSURF_A8R8G8B8;

    t2d->factorNum = 11;

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstAlignedWidth,
                                        &t2d->dstAlignedHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_GetSize(t2d->dstSurf,
                                 &t2d->dstWidth,
                                 &t2d->dstHeight,
                                 gcvNULL));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    // create source surface
    t2d->srcWidth  = t2d->dstWidth / t2d->factorNum;
    t2d->srcHeight = t2d->dstHeight / t2d->factorNum;
    t2d->xStep     = t2d->srcWidth;
    t2d->yStep     = t2d->srcHeight;

    gcmONERROR(gcoSURF_Construct(t2d->runtime->hal,
                                   t2d->srcWidth,
                                   t2d->srcHeight,
                                   1,
                                   gcvSURF_BITMAP,
                                   t2d->srcFormat,
                                   gcvPOOL_DEFAULT,
                                   &t2d->srcSurf));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->srcSurf,
                                        &t2d->srcAlignedWidth,
                                        &t2d->srcAlignedHeight,
                                        &t2d->srcStride));

    gcmONERROR(gcoSURF_GetSize(t2d->srcSurf,
                                 &t2d->srcWidth,
                                 &t2d->srcHeight,
                                 gcvNULL));

    gcmONERROR(gcoSURF_Lock(t2d->srcSurf, &t2d->srcPhyAddr, &t2d->srcLgcAddr));

    // set src color
    gcmONERROR(Gal2DCleanSurface(t2d->runtime->hal, t2d->srcSurf, COLOR_ARGB8(0x80, 0x00, 0x00, 0xFF)));

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = 3;
    t2d->base.description = s_CaseDescription;

    return gcvTRUE;

OnError:

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
