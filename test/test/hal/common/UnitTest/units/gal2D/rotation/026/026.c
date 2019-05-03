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
 *  Feature:    Test the gco2D_NatureRotateTranslation API.
 *  API:        gco2D_NatureRotateTranslation
 *  Check:
 */
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DRotation026\n" \
"Operation: Test the gco2D_NatureRotateTranslation API.\n" \
"2D API: gco2D_Blit gco2D_SetTarget, gco2D_NatureRotateTranslation\n" \
"Src: Size        [640x480]\n"\
"     Rect        [80,120,560,480]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [*]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Dst: Size        [configurable]\n"\
"     Rect        [configurable]\n"\
"     Format      [configurable]\n"\
"     Rotation    [*]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: [None]\n"\
"Alphablend: [disable]\n" \
"HW feature dependency: ";

typedef struct Test2D
{
    GalTest         base;
    GalRuntime *    runtime;

    /* source surface. */
    T2D_SURF_PTR    srcSurf;

    /* tmp surface. */
    T2D_SURF_PTR    tmpSurf[2];

    /* dest surface. */
    gcoSURF         dstSurf;
    gctUINT32       dstPhysAddr;
    gctPOINTER      dstVirtAddr;
    gceSURF_FORMAT  dstFormat;
    gctUINT32       dstWidth;
    gctUINT32       dstHeight;
    gctUINT32       dstAlignedWidth;
    gctUINT32       dstAlignedHeight;
    gctINT          dstStride;
}
Test2D;

static gce2D_NATURE_ROTATION s_nrs[] =
{
    gcvNR_0_DEGREE,
    gcvNR_LEFT_90_DEGREE,
    gcvNR_RIGHT_90_DEGREE,
    gcvNR_180_DEGREE,
    gcvNR_FLIP_X,
    gcvNR_FLIP_Y,
};

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status = gcvSTATUS_OK;
    gco2D egn2D = t2d->runtime->engine2d;

    gctBOOL isSrcRot;
    gce2D_NATURE_ROTATION natureRotation;
    gcsRECT srcRect, dstRect, tmpRect;
    gceSURF_ROTATION srcRotation, dstRotation;
    gctUINT32 horFactor, verFactor;

    T2D_SURF_PTR srcSurf = t2d->srcSurf, tmpSurf;

    /* Setup the parameters. */
    isSrcRot = frameNo < 6;
    natureRotation = s_nrs[frameNo % gcmCOUNTOF(s_nrs)];

    srcRect.left   = 80;
    srcRect.top    = 120;
    srcRect.right  = 560;
    srcRect.bottom = 480;

    switch (natureRotation)
    {
    case gcvNR_LEFT_90_DEGREE:
    case gcvNR_RIGHT_90_DEGREE:
        tmpSurf        = t2d->tmpSurf[1];
        break;

    default:
        tmpSurf        = t2d->tmpSurf[0];
        break;
    }

    tmpRect.left   = 0;
    tmpRect.top    = 0;
    tmpRect.right  = tmpSurf->width;
    tmpRect.bottom = tmpSurf->height;

    /* Translate the nature rotation rule. */
    gcmONERROR(gco2D_NatureRotateTranslation(
        isSrcRot,
        natureRotation,
        srcSurf->width,
        srcSurf->height,
        tmpSurf->width,
        tmpSurf->height,
        &srcRect,
        &tmpRect,
        &srcRotation,
        &dstRotation));

    /* Blit to the tmp surface. */
    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        srcSurf->address,
        srcSurf->validAddressNum,
        srcSurf->stride,
        srcSurf->validStrideNum,
        srcSurf->tiling,
        srcSurf->format,
        srcRotation,
        srcSurf->width,
        srcSurf->height));

    gcmONERROR(gco2D_SetSourceTileStatus(
        egn2D,
        srcSurf->tileStatusConfig,
        srcSurf->format,
        srcSurf->tileStatusClear,
        srcSurf->tileStatusAddress));

    gcmONERROR(gco2D_SetSource(egn2D, &srcRect));

    gcmONERROR(gco2D_SetROP(egn2D, 0xCC, 0xCC));

    gcmONERROR(gco2D_DisableAlphaBlend(egn2D));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        tmpSurf->address,
        tmpSurf->validAddressNum,
        tmpSurf->stride,
        tmpSurf->validStrideNum,
        tmpSurf->tiling,
        tmpSurf->format,
        dstRotation,
        tmpSurf->width,
        tmpSurf->height));

    gcmONERROR(gco2D_SetTargetTileStatus(
        egn2D,
        tmpSurf->tileStatusConfig,
        tmpSurf->format,
        dstRotation,
        tmpSurf->tileStatusAddress));

    gcmONERROR(gco2D_SetClipping(egn2D, &tmpRect));

    gcmONERROR(gco2D_Blit(
        egn2D,
        1,
        &tmpRect,
        0xCC,
        0xCC,
        tmpSurf->format));

    /* Stretch blit to the dst surface. */
    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        tmpSurf->address,
        tmpSurf->validAddressNum,
        tmpSurf->stride,
        tmpSurf->validStrideNum,
        tmpSurf->tiling,
        tmpSurf->format,
        gcvSURF_0_DEGREE,
        tmpSurf->width,
        tmpSurf->height));

    gcmONERROR(gco2D_SetSourceTileStatus(
        egn2D,
        tmpSurf->tileStatusConfig,
        tmpSurf->format,
        tmpSurf->tileStatusClear,
        tmpSurf->tileStatusAddress));

    tmpRect.left   = 0;
    tmpRect.top    = 0;
    tmpRect.right  = tmpSurf->width;
    tmpRect.bottom = tmpSurf->height;

    gcmONERROR(gco2D_SetSource(egn2D, &tmpRect));

    dstRect.left      = 0;
    dstRect.top       = 0;
    dstRect.right     = t2d->dstWidth;
    dstRect.bottom    = t2d->dstHeight;

    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    gcmONERROR(gco2D_SetTarget(
        egn2D,
        t2d->dstPhysAddr,
        t2d->dstStride,
        gcvSURF_0_DEGREE,
        t2d->dstAlignedWidth));

    gcmONERROR(gco2D_CalcStretchFactor(
        egn2D,
        tmpRect.right - tmpRect.left,
        dstRect.right - dstRect.left,
        &horFactor));

    gcmONERROR(gco2D_CalcStretchFactor(
        egn2D,
        tmpRect.bottom - tmpRect.top,
        dstRect.bottom - dstRect.top,
        &verFactor));

    gcmONERROR(gco2D_SetStretchFactors(egn2D, horFactor, verFactor));

    gcmONERROR(gco2D_StretchBlit(egn2D, 1, &dstRect, 0xCC, 0xCC, t2d->dstFormat));

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

    /* Unlock dest surface as necessary. */
    if (t2d->dstSurf && t2d->dstVirtAddr)
    {
        if (gcmIS_ERROR(gcoSURF_Unlock(t2d->dstSurf, t2d->dstVirtAddr)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console,
                "Unlock dstSurf failed:%s\n", GalStatusString(status));
        }
        t2d->dstVirtAddr = gcvNULL;
    }

    /* Delete the src surface. */
    GalDeleteTSurf(t2d->runtime->hal, t2d->srcSurf);

    /* Delete the tmp surface. */
    GalDeleteTSurf(t2d->runtime->hal, t2d->tmpSurf[0]);
    GalDeleteTSurf(t2d->runtime->hal, t2d->tmpSurf[1]);

    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_2DPE20,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    char *srcBmpfile = "resource/zero2_ARGB8.bmp";

    gcoSURF tmpSrcSurf = gcvNULL;
    gceSTATUS status   = gcvSTATUS_OK;

    gctUINT32 k, listLen = sizeof(FeatureList) / sizeof(gctINT);
    gctBOOL featureStatus;
    char featureName[FEATURE_NAME_LEN], featureMsg[FEATURE_MSG_LEN];

    runtime->wholeDescription = (char*)malloc(FEATURE_NAME_LEN * listLen + strlen(s_CaseDescription) + 1);

    if (runtime->wholeDescription == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    memcpy(runtime->wholeDescription, s_CaseDescription, strlen(s_CaseDescription) + 1);

    for (k = 0; k < listLen; k++)
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
    {
        return gcvFALSE;
    }

    memset(t2d, 0, sizeof(Test2D));

    t2d->runtime   = runtime;
    t2d->dstSurf   = runtime->target;
    t2d->dstFormat = runtime->format;

    /* The dest surface. */
    gcmONERROR(gcoSURF_Lock(
        t2d->dstSurf,
        &t2d->dstPhysAddr,
        &t2d->dstVirtAddr));

    gcmONERROR(gcoSURF_GetAlignedSize(
        t2d->dstSurf,
        &t2d->dstAlignedWidth,
        &t2d->dstAlignedHeight,
        &t2d->dstStride));

    gcmONERROR(gcoSURF_GetSize(
        t2d->dstSurf,
        &t2d->dstWidth,
        &t2d->dstHeight,
        gcvNULL));

    gcmONERROR(gcoSURF_GetFormat(
        t2d->dstSurf,
        gcvNULL,
        &t2d->dstFormat));

    /* Create source surface. */
    gcmONERROR(GalLoadFileToTSurf(srcBmpfile, &t2d->srcSurf));

    /* Create tmp surface. */
    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal, gcvSURF_A8R8G8B8, gcvLINEAR, gcv2D_TSC_DISABLE,
        480, 360, &t2d->tmpSurf[0]));

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal, gcvSURF_A8R8G8B8, gcvLINEAR, gcv2D_TSC_DISABLE,
        360, 480, &t2d->tmpSurf[1]));

    /* Fill in the base info. */
    t2d->base.render      = (PGalRender)Render;
    t2d->base.destroy     = (PGalDestroy)Destroy;
    t2d->base.frameCount  = 12;
    t2d->base.description = s_CaseDescription;

    return gcvTRUE;

OnError:

    if (gcmIS_ERROR(status))
    {
        GalOutput(GalOutputType_Error | GalOutputType_Console,
            "%s(%d) failed:%s\n", __FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));

        return gcvFALSE;
    }

    return gcvFALSE;
}

GalTest * CDECL GalCreateTestObject(GalRuntime *runtime)
{
    Test2D *t2d = (Test2D *)malloc(sizeof(Test2D));

    if (!Init(t2d, runtime))
    {
        free(t2d);
        return NULL;
    }

    return &t2d->base;
}
