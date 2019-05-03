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
"Case gal2DFormatCompressed2D012\n" \
"Operation: multi-src blit/rotation/alphablend/mirror: from non compressed (3d output) to 2D compressed.\n" \
"2D API: gco2D_SetSourceTileStatus gco2D_SetTargetTileStatus gco2D_SetGenericSource gco2D_SetBitBlitMirror gco2D_MultiSourceBlit\n" \
"Src1: Size       [320x240/640x480]\n"\
"     Rect        [0,0,240,240]\n"\
"     Format      [ARGB8888/ARGB4444/XRGB8888/RGB565]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [superTileV3]\n"\
"     Compression [None]\n" \
"Src2: Size       [320x240/640x480]\n"\
"     Rect        [0,0,240,240]\n"\
"     Format      [ARGB8888/ARGB4444/XRGB8888/RGB565]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [superTileV3]\n"\
"     Compression [None]\n" \
"Src3: Size       [320x240/640x480]\n"\
"     Rect        [0,0,240,240]\n"\
"     Format      [ARGB8888/ARGB4444/XRGB8888/RGB565]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [superTileV3]\n"\
"     Compression [None]\n" \
"Src4: Size       [320x240/640x480]\n"\
"     Rect        [0,0,240,240]\n"\
"     Format      [ARGB8888/ARGB4444/XRGB8888/RGB565]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [superTileV3]\n"\
"     Compression [None]\n" \
"Src5: Size       [320x240/640x480]\n"\
"     Rect        [0,0,240,240]\n"\
"     Format      [ARGB8888/ARGB4444/XRGB8888/RGB565]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [superTileV3]\n"\
"     Compression [None]\n" \
"Src6: Size       [320x240/640x480]\n"\
"     Rect        [0,0,240,240]\n"\
"     Format      [ARGB8888/ARGB4444/XRGB8888/RGB565]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [superTileV3]\n"\
"     Compression [None]\n" \
"Src7: Size       [320x240/640x480]\n"\
"     Rect        [0,0,240,240]\n"\
"     Format      [ARGB8888/ARGB4444/XRGB8888/RGB565]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [superTileV3]\n"\
"     Compression [None]\n" \
"Src8: Size       [320x240/640x480]\n"\
"     Rect        [0,0,240,240]\n"\
"     Format      [ARGB8888/ARGB4444/XRGB8888/RGB565]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [superTileV3]\n"\
"     Compression [None]\n" \
"Dst: Size        [240x240]\n"\
"     Rect        [0,0,240,240]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [2D compressed]\n" \
"Brush: Type   [SolidBrush]\n"\
"       Format [ARGB8888]\n"\
"       Offset [0]\n" \
"Alphablend: [disable/enable]\n" \
"HW feature dependency: ";

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

static gctINT GetSurfaceIndex(gctINT subFrameIndex, gctINT srcIndex)
{
    static gctINT indexArray[] =
    {
        0, 1, -1, -1, -1, -1, -1, -1,
        2, 3, -1, -1, -1, -1, -1, -1,
        4, 5, -1, -1, -1, -1, -1, -1,
        6, 7, -1, -1, -1, -1, -1, -1,
        8, 9, -1, -1, -1, -1, -1, -1,
        10, 11, -1, -1, -1, -1, -1, -1,

        -1, -1, 0, 1, -1, -1, -1, -1,
        -1, -1, 2, 3, -1, -1, -1, -1,
        -1, -1, 4, 5, -1, -1, -1, -1,
        -1, -1, 6, 7, -1, -1, -1, -1,
        -1, -1, 8, 9, -1, -1, -1, -1,
        -1, -1, 10, 11, -1, -1, -1, -1,

        -1, -1, -1, -1, 0, 1, -1, -1,
        -1, -1, -1, -1, 2, 3, -1, -1,
        -1, -1, -1, -1, 4, 5, -1, -1,
        -1, -1, -1, -1, 6, 7, -1, -1,
        -1, -1, -1, -1, 8, 9, -1, -1,
        -1, -1, -1, -1, 10, 11, -1, -1,

        -1, -1, -1, -1, -1, -1, 0, 1,
        -1, -1, -1, -1, -1, -1, 2, 3,
        -1, -1, -1, -1, -1, -1, 4, 5,
        -1, -1, -1, -1, -1, -1, 6, 7,
        -1, -1, -1, -1, -1, -1, 8, 9,
        -1, -1, -1, -1, -1, -1, 10, 11,
    };

    return indexArray[(subFrameIndex % 24) * 8 + (srcIndex % 8)];
}

static gceSTATUS RenderSubFrame(Test2D *t2d, gctUINT frameNo, gctUINT subFrameIndex, T2D_SURF_PTR result)
{
    gceSTATUS status;
    gcsRECT Rect;
    gco2D egn2D = t2d->runtime->engine2d;
    gctINT i;
    gctINT surfaceIndex;
    gctUINT sourceMask;
    T2D_SURF_PTR compressedTarget = gcvNULL;

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal, gcvSURF_A8R8G8B8, gcvLINEAR, gcv2D_TSC_2D_COMPRESSED,
        240, 240, &compressedTarget));

    sourceMask = 0;

    for (i = 0; i < 8; i++)
    {
        gcsRECT srect;

        srect.left   = 0;
        srect.right  = compressedTarget->width;
        srect.top    = 0;
        srect.bottom = compressedTarget->height;

        gcmONERROR(gco2D_SetCurrentSourceIndex(egn2D, i));

        surfaceIndex = GetSurfaceIndex(subFrameIndex, i);

        if (surfaceIndex < 0)
        {
            continue;
        }

        sourceMask |= (1 << i);

        gcmONERROR(gco2D_SetGenericSource(
            egn2D,
            t2d->surf[surfaceIndex]->address, t2d->surf[surfaceIndex]->validAddressNum,
            t2d->surf[surfaceIndex]->stride, t2d->surf[surfaceIndex]->validStrideNum,
            t2d->surf[surfaceIndex]->tiling, t2d->surf[surfaceIndex]->format,
            sRots[(frameNo / 4) % 6],
            t2d->surf[surfaceIndex]->width, t2d->surf[surfaceIndex]->height));

        gcmONERROR(gco2D_SetSourceTileStatus(
            egn2D,
            t2d->surf[surfaceIndex]->tileStatusConfig,
            t2d->surf[surfaceIndex]->tileStatusFormat,
            t2d->surf[surfaceIndex]->tileStatusClear,
            t2d->surf[surfaceIndex]->tileStatusAddress
            ));

        gcmONERROR(gco2D_SetSource(egn2D, &srect));

        gcmONERROR(gco2D_SetROP(egn2D, 0xCC, 0xCC));

        if (t2d->surf[surfaceIndex]->superTileVersion != -1)
        {
            gco2D_SetStateU32(egn2D, gcv2D_STATE_SUPER_TILE_VERSION,
                t2d->surf[surfaceIndex]->superTileVersion);
        }

        gcmONERROR(gco2D_EnableAlphaBlend(egn2D,
                    128, 128,
                    gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
                    gcvSURF_GLOBAL_ALPHA_ON, gcvSURF_GLOBAL_ALPHA_ON,
                    gcvSURF_BLEND_STRAIGHT, gcvSURF_BLEND_STRAIGHT,
                    gcvSURF_COLOR_STRAIGHT, gcvSURF_COLOR_STRAIGHT));

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
    }

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        compressedTarget->address,
        compressedTarget->validAddressNum,
        compressedTarget->stride,
        compressedTarget->validStrideNum,
        compressedTarget->tiling,
        compressedTarget->format,
        compressedTarget->rotation,
        compressedTarget->width,
        compressedTarget->height));

    gcmONERROR(gco2D_SetTargetTileStatus(
        egn2D,
        compressedTarget->tileStatusConfig,
        compressedTarget->format,
        gcvSURF_0_DEGREE,
        compressedTarget->tileStatusAddress
        ));

    Rect.left   = 0;
    Rect.top    = 0;
    Rect.right  = compressedTarget->width;
    Rect.bottom = compressedTarget->height;

    gcmONERROR(gco2D_SetClipping(egn2D, &Rect));

    gcmONERROR(gco2D_MultiSourceBlit(egn2D, sourceMask, &Rect, 1));

    gcmONERROR(gco2D_DisableAlphaBlend(egn2D));

    gcmONERROR(gco2D_SetBitBlitMirror(egn2D, gcvFALSE, gcvFALSE));

    /* compressed => non compressed */
    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        compressedTarget->address,
        compressedTarget->validAddressNum,
        compressedTarget->stride,
        compressedTarget->validStrideNum,
        compressedTarget->tiling,
        compressedTarget->format,
        compressedTarget->rotation,
        compressedTarget->width,
        compressedTarget->height));

    gcmONERROR(gco2D_SetSourceTileStatus(
        egn2D,
        compressedTarget->tileStatusConfig,
        compressedTarget->tileStatusFormat,
        compressedTarget->tileStatusClear,
        compressedTarget->tileStatusAddress
        ));

    gcmONERROR(gco2D_SetSource(egn2D, &Rect));

    gcmONERROR(gco2D_SetROP(egn2D, 0xCC, 0xCC));

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

    Rect.left   = compressedTarget->width  * (subFrameIndex % 6);
    Rect.right  = compressedTarget->width  * ((subFrameIndex % 6) + 1);
    Rect.top    = compressedTarget->height * (subFrameIndex / 6);
    Rect.bottom = compressedTarget->height * ((subFrameIndex / 6) + 1);

    gcmONERROR(gco2D_SetClipping(egn2D, &Rect));

    gcmONERROR(gco2D_SetBitBlitMirror(egn2D, gcvFALSE, gcvFALSE));

    gcmONERROR(gco2D_Blit(egn2D, 1, &Rect, 0xCC, 0xCC, result->format));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));


OnError:
    if (compressedTarget)
    {
        GalDeleteTSurf(gcvNULL, compressedTarget);
    }

    return status;
}

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status;
    gctINT i;
    T2D_SURF_PTR result = gcvNULL;

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal, gcvSURF_A8R8G8B8, gcvLINEAR, gcv2D_TSC_DISABLE,
        240 * 6, 240 * 4, &result));

    for (i = 0; i < 24; ++i)
    {
        gcmONERROR(RenderSubFrame(t2d, frameNo, i, result));
    }

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
    gcvFEATURE_2D_FC_SOURCE,
    gcvFEATURE_2D_COMPRESSION,
    gcvFEATURE_2D_MULTI_SRC_BLT_TO_UNIFIED_DST_RECT,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status;
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

    /* Set the state. */
    gcmONERROR(gco2D_SetStateU32(runtime->engine2d,
                                 gcv2D_STATE_MULTI_SRC_BLIT_UNIFIED_DST_RECT,
                                 gcvTRUE));

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
    t2d->base.frameCount  = 24;
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
