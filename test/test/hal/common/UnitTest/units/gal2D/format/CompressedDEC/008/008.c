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
 *  Feature:    multi, super, tiled input
 *  API:        gco2D_SetSourceTileStatus gco2D_SetTargetTileStatus gco2D_FilterBlitEx2
 *  Check:
*/
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFormatCompressedDEC008 : FilterBlit compressed surface to compressed surface with full src rotations and mirrors\n";

typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;
} Test2D;

static gceSURF_ROTATION sRots[] =
{
    gcvSURF_0_DEGREE,
    gcvSURF_90_DEGREE,
    gcvSURF_180_DEGREE,
    gcvSURF_270_DEGREE,
    gcvSURF_FLIP_X,
    gcvSURF_FLIP_Y,
};

static gceSURF_FORMAT sFormat[] =
{
    gcvSURF_A8R8G8B8,
    gcvSURF_A8B8G8R8,
    gcvSURF_B8G8R8A8,
    gcvSURF_R8G8B8A8,
    gcvSURF_X8R8G8B8,
    gcvSURF_X8B8G8R8,
    gcvSURF_B8G8R8X8,
    gcvSURF_R8G8B8X8,
    gcvSURF_RG16,
    gcvSURF_UYVY,
    gcvSURF_YUY2,
};

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status = gcvSTATUS_OK;
    gco2D egn2D = t2d->runtime->engine2d;
    T2D_SURF_PTR src = gcvNULL;
    T2D_SURF_PTR surf[2] = {gcvNULL, gcvNULL};
    T2D_SURF_PTR result = gcvNULL;
    gctINT32 len, n;
    gcsRECT rect, srect, drect;

    // create source surface
    gcmONERROR(GalLoadFileToTSurf(
        "resource/rects_640x640_A8R8G8B8.bmp",
        &src));

    len = (gcmMIN(src->width, src->height)) >> 1;
    len /= 2;

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        sFormat[frameNo % gcmCOUNTOF(sFormat)],
        gcvLINEAR,
        gcv2D_TSC_DEC_COMPRESSED,
        src->width,
        src->height,
        surf));

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        sFormat[frameNo / gcmCOUNTOF(sFormat)],
        gcvLINEAR,
        gcv2D_TSC_DEC_COMPRESSED,
        len * 6,
        len * 4,
        surf + 1));

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        t2d->runtime->format,
        gcvLINEAR,
        gcv2D_TSC_DISABLE,
        surf[1]->width * 2,
        surf[1]->height * 2,
        &result));

    gcmONERROR(gco2D_SetStateU32(egn2D, gcv2D_STATE_XRGB_ENABLE, gcvTRUE));

    /* compress the src to surf0. */
    rect.left = 0;
    rect.right = src->width;
    rect.top = 0;
    rect.bottom = src->height;

    gcmONERROR(gco2D_SetSource(egn2D, &rect));

    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        src->address, src->validAddressNum,
        src->stride, src->validStrideNum,
        src->tiling, src->format,
        src->rotation,
        src->width, src->height));

    gcmONERROR(gco2D_SetSourceTileStatus(
        egn2D,
        src->tileStatusConfig,
        src->tileStatusFormat,
        src->tileStatusClear,
        src->tileStatusAddress
        ));

    gcmONERROR(gco2D_SetTargetTileStatus(
        egn2D,
        surf[0]->tileStatusConfig,
        surf[0]->format,
        0,
        surf[0]->tileStatusAddress
        ));

    gcmONERROR(gco2D_SetClipping(egn2D, &rect));

    gcmONERROR(gco2D_SetKernelSize(egn2D, 3, 3));

    gcmONERROR(gco2D_FilterBlitEx2(egn2D,
        src->address, src->validAddressNum,
        src->stride, src->validStrideNum,
        src->tiling, src->format, src->rotation, src->aWidth, src->aHeight, &rect,
        surf[0]->address, surf[0]->validAddressNum,
        surf[0]->stride, surf[0]->validStrideNum,
        surf[0]->tiling, surf[0]->format, surf[0]->rotation,
        surf[0]->aWidth, surf[0]->aHeight,
        &rect, gcvNULL));

    if (!t2d->runtime->noSaveTargetNew)
    {
        char name[200];

        gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));
        sprintf(name, "gal2DFormatCompressedDEC008_intermediate0_%03d.bmp", frameNo);
        GalSaveTSurfToDIB(surf[0], name);
    }

    /* StretchBlit surf0 to surf1. */
    srect.left = 160;
    srect.right = src->width - 160;
    srect.top = 160;
    srect.bottom = src->height - 160;

    gcmONERROR(gco2D_SetSource(egn2D, &srect));

    gcmONERROR(gco2D_SetSourceTileStatus(
        egn2D,
        surf[0]->tileStatusConfig,
        surf[0]->tileStatusFormat,
        0,
        surf[0]->tileStatusAddress
        ));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        surf[1]->address,
        surf[1]->validAddressNum,
        surf[1]->stride,
        surf[1]->validStrideNum,
        surf[1]->tiling,
        surf[1]->format,
        surf[1]->rotation,
        surf[1]->aWidth,
        surf[1]->aHeight));

    gcmONERROR(gco2D_SetTargetTileStatus(
        egn2D,
        surf[1]->tileStatusConfig,
        surf[1]->format,
        0,
        surf[1]->tileStatusAddress
        ));

    gcmONERROR(gco2D_SetKernelSize(egn2D, 3, 5));

    for (n = 0; n < 24; ++n)
    {
        gceSURF_ROTATION rot;
        gctBOOL horMirror, verMirror;
        gctINT xx = n % 6;
        gctINT yy = n / 6;

        if (surf[1]->format == gcvSURF_YUY2 ||
            surf[1]->format == gcvSURF_UYVY)
        {
            rot = gcvSURF_0_DEGREE;
            horMirror = verMirror = gcvFALSE;
        }
        else
        {
            rot = sRots[xx];
            horMirror = yy & 1 ? gcvTRUE : gcvFALSE;
            verMirror = yy & 2 ? gcvTRUE : gcvFALSE;
        }

        gcmONERROR(gco2D_SetBitBlitMirror(
            egn2D,
            horMirror, verMirror
            ));

        rect.left = xx * len;
        rect.right = rect.left + len;
        rect.top = yy * len;
        rect.bottom = rect.top + len;

        gcmONERROR(gco2D_SetClipping(egn2D, &rect));

        gcmONERROR(gco2D_FilterBlitEx2(egn2D,
            surf[0]->address, surf[0]->validAddressNum,
            surf[0]->stride, surf[0]->validStrideNum,
            surf[0]->tiling, surf[0]->format, gcvSURF_0_DEGREE, surf[0]->aWidth, surf[0]->aHeight, &srect,
            surf[1]->address, surf[1]->validAddressNum,
            surf[1]->stride, surf[1]->validStrideNum,
            surf[1]->tiling, surf[1]->format, rot,
            surf[1]->aWidth, surf[1]->aHeight,
            &rect, gcvNULL));
    }
    if (!t2d->runtime->noSaveTargetNew)
    {
        char name[200];

        gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));
        sprintf(name, "gal2DFormatCompressedDEC008_intermediate1_%03d.bmp", frameNo);
        GalSaveTSurfToDIB(surf[1], name);
    }
    /* Uncompress surf[1] to result. */
    rect.left = rect.top = 0;
    rect.right = surf[1]->width;
    rect.bottom = surf[1]->height;

    drect.left = drect.top = 0;
    drect.right = result->width;
    drect.bottom = result->height;

    gcmONERROR(gco2D_SetSourceTileStatus(
        egn2D,
        surf[1]->tileStatusConfig,
        surf[1]->tileStatusFormat,
        0,
        surf[1]->tileStatusAddress
        ));

    gcmONERROR(gco2D_SetTargetTileStatus(
        egn2D,
        result->tileStatusConfig,
        result->format,
        0,
        result->tileStatusAddress
        ));

    gcmONERROR(gco2D_SetClipping(egn2D, &drect));

    gcmONERROR(gco2D_SetBitBlitMirror(
        egn2D, gcvFALSE, gcvFALSE
        ));

    gcmONERROR(gco2D_SetKernelSize(egn2D, 5, 3));

    gcmONERROR(gco2D_FilterBlitEx2(egn2D,
        surf[1]->address, surf[1]->validAddressNum,
        surf[1]->stride, surf[1]->validStrideNum,
        surf[1]->tiling, surf[1]->format, gcvSURF_0_DEGREE, surf[0]->aWidth, surf[0]->aHeight, &rect,
        result->address, result->validAddressNum,
        result->stride, result->validStrideNum,
        result->tiling, result->format, result->rotation,
        result->aWidth, result->aHeight,
        &drect, gcvNULL));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    if (t2d->runtime->saveFullName)
    {
        GalSaveTSurfToDIB(result, t2d->runtime->saveFullName);
    }

OnError:

    if (result)
    {
        GalDeleteTSurf(gcvNULL, result);
    }

    if (surf[0])
    {
        GalDeleteTSurf(gcvNULL, surf[0]);
    }

    if (surf[1])
    {
        GalDeleteTSurf(gcvNULL, surf[1]);
    }

    if (src)
    {
        GalDeleteTSurf(gcvNULL, src);
    }

    if (status != gcvSTATUS_OK)
    {
        GalOutput(GalOutputType_Error | GalOutputType_Console,
        "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));

        return gcvFALSE;
    }
    else
    {
        return gcvTRUE;
    }
}

static void CDECL Destroy(Test2D *t2d)
{
    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_DEC300_COMPRESSION,
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
    runtime->saveTarget = gcvFALSE;
    runtime->cleanTarget = gcvFALSE;

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = gcmCOUNTOF(sFormat) * 2;
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
