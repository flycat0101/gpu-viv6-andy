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
 *  API:        gco2D_SetGenericSource gco2D_SetGenericTarget gco2D_Blit
 *  Check:
 */
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFormatCompressedDEC013 : BitBlit rotation/mirror: from compressed (3d input) to non-compressed.\n";

static gctSTRING sourcefile[] =
{
    "resource/texture2_cc0_A4R4G4B4.vimg",
    "resource/texture2_cc1_A4R4G4B4.vimg",
    "resource/texture2_cc0_A8R8G8B8.vimg",
    "resource/texture2_cc1_A8R8G8B8.vimg",
    "resource/texture2_cc0_R5G6B5.vimg",
    "resource/texture2_cc1_R5G6B5.vimg",
    "resource/texture2_cc0_X8R8G8B8.vimg",
    "resource/texture2_cc1_X8R8G8B8.vimg",
    "resource/texture5_cc0_A8R8G8B8.vimg",
    "resource/texture5_cc1_A8R8G8B8.vimg",
    "resource/texture5_cc0_R5G6B5.vimg",
    "resource/texture5_cc1_R5G6B5.vimg",
};

typedef struct Test2D {
    GalTest     base;
    GalRuntime *runtime;

    //source surface
    T2D_SURF_PTR    surf[gcmCOUNTOF(sourcefile)];
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

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status;
    gctUINT len;
    T2D_SURF_PTR result = gcvNULL;
    gcsRECT Rect;
    gco2D egn2D = t2d->runtime->engine2d;

    gcmONERROR(gco2D_SetStateU32(egn2D, gcv2D_STATE_XRGB_ENABLE, gcvTRUE));

    len = gcmMIN(t2d->surf[frameNo]->width, t2d->surf[frameNo]->height);

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        t2d->runtime->format,
        gcvLINEAR, gcv2D_TSC_DISABLE,
        len, len, &result));

    Rect.left   = 0;
    Rect.top    = 0;
    Rect.right  = len;
    Rect.bottom = len;

    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        t2d->surf[frameNo]->address, t2d->surf[frameNo]->validAddressNum,
        t2d->surf[frameNo]->stride, t2d->surf[frameNo]->validStrideNum,
        t2d->surf[frameNo]->tiling, t2d->surf[frameNo]->format,
        sRots[frameNo % 6],
        t2d->surf[frameNo]->width, t2d->surf[frameNo]->height));

    gcmONERROR(gco2D_SetSourceTileStatus(
        egn2D,
        t2d->surf[frameNo]->tileStatusConfig,
        t2d->surf[frameNo]->tileStatusFormat,
        t2d->surf[frameNo]->tileStatusClear,
        t2d->surf[frameNo]->tileStatusAddress
        ));

    gcmONERROR(gco2D_SetSource(egn2D, &Rect));
    gcmONERROR(gco2D_SetClipping(egn2D, &Rect));

    if (t2d->surf[frameNo]->superTileVersion != -1)
    {
        gco2D_SetStateU32(egn2D, gcv2D_STATE_SUPER_TILE_VERSION,
            t2d->surf[frameNo]->superTileVersion);
    }

    switch (frameNo % 4)
    {
        case 0:
            // disable mirror
            gcmONERROR(gco2D_SetBitBlitMirror(egn2D, gcvFALSE, gcvFALSE));
            break;
        case 1:
            // enable horizontal mirror
            gcmONERROR(gco2D_SetBitBlitMirror(egn2D, gcvTRUE, gcvFALSE));
            break;
        case 2:
            // enable vertical mirror
            gcmONERROR(gco2D_SetBitBlitMirror(egn2D, gcvFALSE, gcvTRUE));
            break;
        case 3:
            // enable horizontal & vertical mirror
            gcmONERROR(gco2D_SetBitBlitMirror(egn2D, gcvTRUE, gcvTRUE));
            break;
        default:
            return gcvFALSE;
    }

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
        gcvSURF_0_DEGREE,
        result->tileStatusAddress
        ));

    gcmONERROR(gco2D_Blit(egn2D, 1, &Rect, 0xCC, 0xCC, result->format));

    gcmONERROR(gco2D_SetBitBlitMirror(egn2D, gcvFALSE, gcvFALSE));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    if (!t2d->runtime->noSaveTargetNew && t2d->runtime->saveFullName)
    {
        GalSaveTSurfToDIB(result, t2d->runtime->saveFullName);
    }

OnError:
    if (result)
    {
        GalDeleteTSurf(gcvNULL, result);
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
    gceSTATUS status = gcvSTATUS_OK;
    gctINT i;

    // destroy source surface
    for (i = 0; i < gcmCOUNTOF(t2d->surf); ++i)
    {
        if (t2d->surf[i] != gcvNULL)
        {
            if (gcmIS_ERROR(GalDeleteTSurf(t2d->runtime->hal, t2d->surf[i])))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console,
                    "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));
            }
            t2d->surf[i] = gcvNULL;
        }
    }

    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_DEC300_COMPRESSION,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT i;

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

    // create source surface
    for (i = 0; i < gcmCOUNTOF(t2d->surf); ++i)
    {
        gcmONERROR(GalLoadFileToTSurf(sourcefile[i], &t2d->surf[i]));
    }

    t2d->base.render      = (PGalRender)Render;
    t2d->base.destroy     = (PGalDestroy)Destroy;
    t2d->base.frameCount  = 12;
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

    memset(t2d, 0, sizeof(Test2D));

    if (!Init(t2d, runtime)) {
        free(t2d);
        return NULL;
    }

    return &t2d->base;
}
