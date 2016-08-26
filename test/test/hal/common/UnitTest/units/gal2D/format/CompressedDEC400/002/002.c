/****************************************************************************
*
*    Copyright 2012 - 2016 Vivante Corporation, Santa Clara, California.
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
 *  API:        gco2D_SetGenericSource gco2D_SetGenericTarget gco2D_Blit
 *  Check:
*/
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFormatCompressedDEC400_002 : BitBlit DEC400 compressed surface to compressed surface with full src rotations and mirrors\n";

typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;

    gctBOOL     compressed;
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

typedef struct _Comb {
    gceSURF_FORMAT  format;
    gceTILING       tiling;
} Comb;

static Comb sComb[] =
{
    { gcvSURF_A1R5G5B5,          gcvSUPERTILED},
    { gcvSURF_A4R4G4B4,          gcvSUPERTILED},
    { gcvSURF_X1R5G5B5,          gcvSUPERTILED},
    { gcvSURF_X4R4G4B4,          gcvSUPERTILED},
    { gcvSURF_R5G6B5,            gcvSUPERTILED},
    { gcvSURF_X8R8G8B8,          gcvSUPERTILED},
    { gcvSURF_X8R8G8B8,          gcvSUPERTILED},
    { gcvSURF_A8R8G8B8,          gcvSUPERTILED},
    { gcvSURF_A8R8G8B8,          gcvSUPERTILED},
    { gcvSURF_A2R10G10B10,       gcvSUPERTILED},
    { gcvSURF_A2R10G10B10,       gcvSUPERTILED},
    { gcvSURF_YUY2,              gcvSUPERTILED},
    { gcvSURF_YUY2,              gcvSUPERTILED},
    { gcvSURF_UYVY,              gcvSUPERTILED},
    { gcvSURF_UYVY,              gcvSUPERTILED},

    { gcvSURF_X8R8G8B8,          gcvTILED_4X8},
    { gcvSURF_X8R8G8B8,          gcvTILED_4X8},
    { gcvSURF_A8R8G8B8,          gcvTILED_4X8},
    { gcvSURF_A8R8G8B8,          gcvTILED_4X8},
    { gcvSURF_A2R10G10B10,       gcvTILED_4X8},
    { gcvSURF_A2R10G10B10,       gcvTILED_4X8},

    { gcvSURF_X8R8G8B8,          gcvTILED_8X4},
    { gcvSURF_X8R8G8B8,          gcvTILED_8X4},
    { gcvSURF_A8R8G8B8,          gcvTILED_8X4},
    { gcvSURF_A8R8G8B8,          gcvTILED_8X4},
    { gcvSURF_A2R10G10B10,       gcvTILED_8X4},
    { gcvSURF_A2R10G10B10,       gcvTILED_8X4},

    { gcvSURF_P010,              gcvTILED_32X4},
    { gcvSURF_P010,              gcvTILED_32X4},

    { gcvSURF_A1R5G5B5,    gcvTILED_8X8_XMAJOR},
    { gcvSURF_A1R5G5B5,    gcvTILED_8X8_XMAJOR},
    { gcvSURF_A4R4G4B4,    gcvTILED_8X8_XMAJOR},
    { gcvSURF_A4R4G4B4,    gcvTILED_8X8_XMAJOR},
    { gcvSURF_X1R5G5B5,    gcvTILED_8X8_XMAJOR},
    { gcvSURF_X1R5G5B5,    gcvTILED_8X8_XMAJOR},
    { gcvSURF_X4R4G4B4,    gcvTILED_8X8_XMAJOR},
    { gcvSURF_X4R4G4B4,    gcvTILED_8X8_XMAJOR},
    { gcvSURF_R5G6B5,      gcvTILED_8X8_XMAJOR},
    { gcvSURF_R5G6B5,      gcvTILED_8X8_XMAJOR},
    { gcvSURF_X8R8G8B8,    gcvTILED_8X8_XMAJOR},
    { gcvSURF_X8R8G8B8,    gcvTILED_8X8_XMAJOR},
    { gcvSURF_A8R8G8B8,    gcvTILED_8X8_XMAJOR},
    { gcvSURF_A8R8G8B8,    gcvTILED_8X8_XMAJOR},
    { gcvSURF_A2R10G10B10, gcvTILED_8X8_XMAJOR},
    { gcvSURF_A2R10G10B10, gcvTILED_8X8_XMAJOR},
    { gcvSURF_YUY2,        gcvTILED_8X8_XMAJOR},
    { gcvSURF_YUY2,        gcvTILED_8X8_XMAJOR},
    { gcvSURF_UYVY,        gcvTILED_8X8_XMAJOR},
    { gcvSURF_UYVY,        gcvTILED_8X8_XMAJOR},
    { gcvSURF_NV12,        gcvTILED_8X8_XMAJOR},
    { gcvSURF_NV12,        gcvTILED_8X8_XMAJOR},
};

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status = gcvSTATUS_OK;
    gco2D egn2D = t2d->runtime->engine2d;
    T2D_SURF_PTR src = gcvNULL;
    T2D_SURF_PTR surf[2] = {gcvNULL, gcvNULL};
    T2D_SURF_PTR result = gcvNULL, result2 = gcvNULL;
    gctINT32 len, n, s;
    gctUINT32 horFactor, verFactor;
    gcsRECT srect, rect;
    static gctUINT32 k = 0;

    // create source surface
    gcmONERROR(GalLoadFileToTSurf(
        "resource/rects_640x640_A8R8G8B8.bmp",
        &src));

    len = (gcmMIN(src->width, src->height)) >> 1;

    s = frameNo % gcmCOUNTOF(sComb);

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        sComb[s].format,
        sComb[s].tiling,
        t2d->compressed ? gcv2D_TSC_DEC_COMPRESSED : gcv2D_TSC_DISABLE,
        src->width,
        src->height,
        surf));

    s = (frameNo + k++) % gcmCOUNTOF(sComb);

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        sComb[s].format,
        sComb[s].tiling,
        gcv2D_TSC_DEC_COMPRESSED,
        len * 6,
        len * 4,
        surf + 1));

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        t2d->runtime->format,
        gcvLINEAR,
        gcv2D_TSC_DISABLE,
        surf[1]->width,
        surf[1]->height,
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

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        surf[0]->address,
        surf[0]->validAddressNum,
        surf[0]->stride,
        surf[0]->validStrideNum,
        surf[0]->tiling,
        surf[0]->format,
        surf[0]->rotation,
        surf[0]->aWidth,
        surf[0]->aHeight));

    gcmONERROR(gco2D_SetTargetTileStatus(
        egn2D,
        surf[0]->tileStatusConfig,
        surf[0]->format,
        0,
        surf[0]->tileStatusAddress
        ));

    gcmONERROR(gco2D_SetClipping(egn2D, &rect));

    gcmONERROR(gco2D_Blit(egn2D, 1, &rect, 0xCC, 0xCC, surf[0]->format));


    /* BitBlit surf0 to surf1. */
    rect.left = (src->width - len) / 2 ;
    rect.right = (src->width + len) / 2;
    rect.top = (src->height - len) / 2;
    rect.bottom = (src->height + len) / 2;

    gcmONERROR(gco2D_SetSource(egn2D, &rect));

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

    for (n = 0; n < 24; ++n)
    {
        gceSURF_ROTATION rot;
        gctBOOL horMirror, verMirror;
        gctINT xx = n % 6;
        gctINT yy = n / 6;

        rot = sRots[xx];
        horMirror = yy & 1 ? gcvTRUE : gcvFALSE;
        verMirror = yy & 2 ? gcvTRUE : gcvFALSE;

        gcmONERROR(gco2D_SetGenericSource(
            egn2D,
            surf[0]->address,
            surf[0]->validAddressNum,
            surf[0]->stride,
            surf[0]->validStrideNum,
            surf[0]->tiling,
            surf[0]->format,
            rot,
            surf[0]->aWidth,
            surf[0]->aHeight));

        gcmONERROR(gco2D_SetBitBlitMirror(
            egn2D,
            horMirror,
            verMirror
            ));

        rect.left = xx * len;
        rect.right = rect.left + len;
        rect.top = yy * len;
        rect.bottom = rect.top + len;

        gcmONERROR(gco2D_SetClipping(egn2D, &rect));

        gcmONERROR(gco2D_Blit(egn2D, 1, &rect, 0xCC, 0xCC, surf[1]->format));
    }


    /* Uncompress surf[1] to result. */
    rect.left = rect.top = 0;
    rect.right = surf[1]->width;
    rect.bottom = surf[1]->height;

    gcmONERROR(gco2D_SetSource(egn2D, &rect));

    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        surf[1]->address,
        surf[1]->validAddressNum,
        surf[1]->stride,
        surf[1]->validStrideNum,
        surf[1]->tiling,
        surf[1]->format,
        gcvSURF_0_DEGREE,
        surf[1]->aWidth,
        surf[1]->aHeight));

    gcmONERROR(gco2D_SetSourceTileStatus(
        egn2D,
        surf[1]->tileStatusConfig,
        surf[1]->tileStatusFormat,
        0,
        surf[1]->tileStatusAddress
        ));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        result->address,
        result->validAddressNum,
        result->stride,
        result->validStrideNum,
        result->tiling,
        result->format,
        result->rotation,
        result->width,
        result->height));

    gcmONERROR(gco2D_SetTargetTileStatus(
        egn2D,
        result->tileStatusConfig,
        result->format,
        0,
        result->tileStatusAddress
        ));

    gcmONERROR(gco2D_SetClipping(egn2D, &rect));

    gcmONERROR(gco2D_SetBitBlitMirror(
        egn2D, gcvFALSE, gcvFALSE
        ));

    gcmONERROR(gco2D_Blit(egn2D, 1, &rect, 0xCC, 0xCC, result->format));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));


    ////
    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        gcvSURF_R5G6B5,
        gcvLINEAR,
        gcv2D_TSC_DISABLE,
        480, 320,
        &result2));

    srect.left = srect.top = 0;
    srect.right = result->width;
    srect.bottom = result->height;

    rect.left = rect.top = 0;
    rect.right = result2->width;
    rect.bottom = result2->height;

    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        result->address,
        result->validAddressNum,
        result->stride,
        result->validStrideNum,
        result->tiling,
        result->format,
        gcvSURF_0_DEGREE,
        result->aWidth,
        result->aHeight));

    gcmONERROR(gco2D_SetSourceTileStatus(
        egn2D,
        gcv2D_TSC_DISABLE,
        gcvSURF_UNKNOWN,
        0,
        ~0U
        ));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        result2->address,
        result2->validAddressNum,
        result2->stride,
        result2->validStrideNum,
        result2->tiling,
        result2->format,
        gcvSURF_0_DEGREE,
        result2->width,
        result2->height));

    gcmONERROR(gco2D_SetTargetTileStatus(
        egn2D,
        gcv2D_TSC_DISABLE,
        gcvSURF_UNKNOWN,
        0,
        ~0U
        ));

    gcmONERROR(gco2D_SetSource(egn2D, &srect));
    gcmONERROR(gco2D_SetClipping(egn2D, &rect));

    /* Calculate the stretch factors. */
    gcmONERROR(gco2D_CalcStretchFactor(egn2D, srect.right - srect.left,
            rect.right - rect.left, &horFactor));

    gcmONERROR(gco2D_CalcStretchFactor(egn2D, srect.bottom - srect.top,
            rect.bottom - rect.top, &verFactor));
    gcmONERROR(gco2D_SetStretchFactors(egn2D, horFactor, verFactor));

    gcmONERROR(gco2D_StretchBlit(egn2D, 1, &rect, 0xCC, 0xCC, result2->format));
    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    if (t2d->runtime->saveFullName)
    {
        GalSaveTSurfToDIB(result2, t2d->runtime->saveFullName);
    }

OnError:

    if (surf[0])
    {
        GalDeleteTSurf(gcvNULL, surf[0]);
    }

    if (surf[1])
    {
        GalDeleteTSurf(gcvNULL, surf[1]);
    }

    if (result)
    {
        GalDeleteTSurf(gcvNULL, result);
    }

    if (result2)
    {
        GalDeleteTSurf(gcvNULL, result2);
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
    gcvFEATURE_DEC400_COMPRESSION,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 argc = runtime->argc;
    gctSTRING *argv = runtime->argv;
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

    t2d->compressed = gcvFALSE;
    for (k = 0; k < argc; ++k)
    {
        if (!strcmp(argv[k], "-compressed"))
        {
            t2d->compressed = gcvTRUE;
        }
    }

    t2d->runtime = runtime;
    runtime->saveTarget = gcvFALSE;
    runtime->cleanTarget = gcvFALSE;

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = gcmCOUNTOF(sComb);
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
