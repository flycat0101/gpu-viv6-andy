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
 *  API:        gco2D_SetGenericSource gco2D_SetGenericTarget
 *  Check:
*/
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription =
"Case gal2DFormatTileStatue014\n" \
"Operation: Test MSAA and VMSAA input blit with XMjor and YMjaor tile format.\n" \
"2D API: gco2D_SetGenericSource gco2D_SetGenericTarget\n" \
"Src: Size        [1024x1024]\n"\
"     Rect        [0,0,1024,1024]\n"\
"     Format      [RGB565/XRGB8888]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [superTileXmajor/superTileYmajor]\n"\
"     Compression [V4 compression]\n" \
"Dst: Size        [configurable]\n"\
"     Rect        [configurable]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: [None]\n"\
"Alphablend: [disable]\n" \
"HW feature dependency: ";

typedef struct _Source_File {
    gctCONST_STRING dataFile;
    gctCONST_STRING tileStatusFile;
    gctUINT32 width;
    gctUINT32 height;
    gceTILING tiling;
    gceSURF_FORMAT format;
    gce2D_TILE_STATUS_CONFIG tileStatusConfig;
}Source_File;

static Source_File sourcefiles[] =
{
    /* 0 MSAA V4COMPRESSION_128B XMAJOR RGB565 */
    {
        "resource/vmsaa/texture2_128B_MSAA_RGB565_1024x1024_data.bin",
        "resource/vmsaa/texture2_128B_MSAA_RGB565_1024x1024_tilestatus.bin",
        1024, 1024,
        gcvSUPERTILED,
        gcvSURF_R5G6B5,
        gcv2D_TSC_V4_COMPRESSED | gcv2D_TSC_DOWN_SAMPLER,
    },
    /* 1 MSAA V4COMPRESSION_128B XMAJOR XRGB8888 */
    {
        "resource/vmsaa/texture2_128B_MSAA_XRGB8888_1024x1024_data.bin",
        "resource/vmsaa/texture2_128B_MSAA_XRGB8888_1024x1024_tilestatus.bin",
        1024, 1024,
        gcvSUPERTILED,
        gcvSURF_X8R8G8B8,
        gcv2D_TSC_V4_COMPRESSED | gcv2D_TSC_DOWN_SAMPLER,
    },
    /* 2 MSAA V4COMPRESSION_128B YMAJOR XRGB8888 */
    {
        "resource/vmsaa/texture2_128B_MSAA_XRGB8888_YMAJOR_1024x1024_data.bin",
        "resource/vmsaa/texture2_128B_MSAA_XRGB8888_YMAJOR_1024x1024_tilestatus.bin",
        1024, 1024,
        gcvYMAJOR_SUPERTILED,
        gcvSURF_X8R8G8B8,
        gcv2D_TSC_V4_COMPRESSED | gcv2D_TSC_DOWN_SAMPLER,
    },
    /* 3 VMSAA V4COMPRESSION_128B XMAJOR RGB565 */
    {
        "resource/vmsaa/texture2_128B_VMSAA_RGB565_1024x1024_data.bin",
        "resource/vmsaa/texture2_128B_VMSAA_RGB565_1024x1024_tilestatus.bin",
        1024, 1024,
        gcvSUPERTILED,
        gcvSURF_R5G6B5,
        gcv2D_TSC_V4_COMPRESSED | gcv2D_TSC_DOWN_SAMPLER,
    },
    /* 4 VMSAA V4COMPRESSION_128B XMAJOR XRGB8888 */
    {
        "resource/vmsaa/texture2_128B_VMSAA_XRGB8888_1024x1024_data.bin",
        "resource/vmsaa/texture2_128B_VMSAA_XRGB8888_1024x1024_tilestatus.bin",
        1024, 1024,
        gcvSUPERTILED,
        gcvSURF_X8R8G8B8,
        gcv2D_TSC_V4_COMPRESSED | gcv2D_TSC_DOWN_SAMPLER,
    },
    /* 5 VMSAA V4COMPRESSION_128B YMAJOR XRGB8888 */
    {
        "resource/vmsaa/texture2_128B_VMSAA_XRGB8888_YMAJOR_1024x1024_data.bin",
        "resource/vmsaa/texture2_128B_VMSAA_XRGB8888_YMAJOR_1024x1024_tilestatus.bin",
        1024, 1024,
        gcvYMAJOR_SUPERTILED,
        gcvSURF_X8R8G8B8,
        gcv2D_TSC_V4_COMPRESSED | gcv2D_TSC_DOWN_SAMPLER,
    },
    /* 6 MSAA V4COMPRESSION_256B XMAJOR RGB565 */
    {
        "resource/vmsaa/texture2_256B_MSAA_RGB565_1024x1024_data.bin",
        "resource/vmsaa/texture2_256B_MSAA_RGB565_1024x1024_tilestatus.bin",
        1024, 1024,
        gcvSUPERTILED,
        gcvSURF_R5G6B5,
        gcv2D_TSC_V4_COMPRESSED_256B | gcv2D_TSC_DOWN_SAMPLER,
    },
    /* 7 MSAA V4COMPRESSION_256B YMAJOR RGB565 */
    {
        "resource/vmsaa/texture2_256B_MSAA_RGB565_YMAJOR_1024x1024_data.bin",
        "resource/vmsaa/texture2_256B_MSAA_RGB565_YMAJOR_1024x1024_tilestatus.bin",
        1024, 1024,
        gcvYMAJOR_SUPERTILED,
        gcvSURF_R5G6B5,
        gcv2D_TSC_V4_COMPRESSED_256B | gcv2D_TSC_DOWN_SAMPLER,
    },
    /* 8 MSAA V4COMPRESSION_256B XMAJOR XRGB8888 */
    {
        "resource/vmsaa/texture2_256B_MSAA_XRGB8888_1024x1024_data.bin",
        "resource/vmsaa/texture2_256B_MSAA_XRGB8888_1024x1024_tilestatus.bin",
        1024, 1024,
        gcvSUPERTILED,
        gcvSURF_X8R8G8B8,
        gcv2D_TSC_V4_COMPRESSED_256B | gcv2D_TSC_DOWN_SAMPLER,
    },
    /* 9 VMSAA V4COMPRESSION_256B XMAJOR RGB565 */
    {
        "resource/vmsaa/texture2_256B_VMSAA_RGB565_1024x1024_data.bin",
        "resource/vmsaa/texture2_256B_VMSAA_RGB565_1024x1024_tilestatus.bin",
        1024, 1024,
        gcvSUPERTILED,
        gcvSURF_R5G6B5,
        gcv2D_TSC_V4_COMPRESSED_256B | gcv2D_TSC_DOWN_SAMPLER,
    },
    /* 10 VMSAA V4COMPRESSION_256B YMAJOR RGB565 */
    {
        "resource/vmsaa/texture2_256B_VMSAA_RGB565_YMAJOR_1024x1024_data.bin",
        "resource/vmsaa/texture2_256B_VMSAA_RGB565_YMAJOR_1024x1024_tilestatus.bin",
        1024, 1024,
        gcvYMAJOR_SUPERTILED,
        gcvSURF_R5G6B5,
        gcv2D_TSC_V4_COMPRESSED_256B | gcv2D_TSC_DOWN_SAMPLER,
    },
    /* 11 VMSAA V4COMPRESSION_256B XMAJOR XRGB8888 */
    {
        "resource/vmsaa/texture2_256B_VMSAA_XRGB8888_1024x1024_data.bin",
        "resource/vmsaa/texture2_256B_VMSAA_XRGB8888_1024x1024_tilestatus.bin",
        1024, 1024,
        gcvSUPERTILED,
        gcvSURF_X8R8G8B8,
        gcv2D_TSC_V4_COMPRESSED_256B | gcv2D_TSC_DOWN_SAMPLER,
    },

    /* 12 MSAA V4COMPRESSION_128B XMAJOR RGB565 */
    {
        "resource/vmsaa/texture5_128B_MSAA_RGB565_1024x1024_data.bin",
        "resource/vmsaa/texture5_128B_MSAA_RGB565_1024x1024_tilestatus.bin",
        1024, 1024,
        gcvSUPERTILED,
        gcvSURF_R5G6B5,
        gcv2D_TSC_V4_COMPRESSED | gcv2D_TSC_DOWN_SAMPLER,
    },
    /* 13 MSAA V4COMPRESSION_128B XMAJOR XRGB8888 */
    {
        "resource/vmsaa/texture5_128B_MSAA_XRGB8888_1024x1024_data.bin",
        "resource/vmsaa/texture5_128B_MSAA_XRGB8888_1024x1024_tilestatus.bin",
        1024, 1024,
        gcvSUPERTILED,
        gcvSURF_X8R8G8B8,
        gcv2D_TSC_V4_COMPRESSED | gcv2D_TSC_DOWN_SAMPLER,
    },
    /* 14 MSAA V4COMPRESSION_128B YMAJOR XRGB8888 */
    {
        "resource/vmsaa/texture5_128B_MSAA_XRGB8888_YMAJOR_1024x1024_data.bin",
        "resource/vmsaa/texture5_128B_MSAA_XRGB8888_YMAJOR_1024x1024_tilestatus.bin",
        1024, 1024,
        gcvYMAJOR_SUPERTILED,
        gcvSURF_X8R8G8B8,
        gcv2D_TSC_V4_COMPRESSED | gcv2D_TSC_DOWN_SAMPLER,
    },
    /* 15 VMSAA V4COMPRESSION_128B XMAJOR RGB565 */
    {
        "resource/vmsaa/texture5_128B_VMSAA_RGB565_1024x1024_data.bin",
        "resource/vmsaa/texture5_128B_VMSAA_RGB565_1024x1024_tilestatus.bin",
        1024, 1024,
        gcvSUPERTILED,
        gcvSURF_R5G6B5,
        gcv2D_TSC_V4_COMPRESSED | gcv2D_TSC_DOWN_SAMPLER,
    },
    /* 16 VMSAA V4COMPRESSION_128B XMAJOR XRGB8888 */
    {
        "resource/vmsaa/texture5_128B_VMSAA_XRGB8888_1024x1024_data.bin",
        "resource/vmsaa/texture5_128B_VMSAA_XRGB8888_1024x1024_tilestatus.bin",
        1024, 1024,
        gcvSUPERTILED,
        gcvSURF_X8R8G8B8,
        gcv2D_TSC_V4_COMPRESSED | gcv2D_TSC_DOWN_SAMPLER,
    },
    /* 17 VMSAA V4COMPRESSION_128B YMAJOR XRGB8888 */
    {
        "resource/vmsaa/texture5_128B_VMSAA_XRGB8888_YMAJOR_1024x1024_data.bin",
        "resource/vmsaa/texture5_128B_VMSAA_XRGB8888_YMAJOR_1024x1024_tilestatus.bin",
        1024, 1024,
        gcvYMAJOR_SUPERTILED,
        gcvSURF_X8R8G8B8,
        gcv2D_TSC_V4_COMPRESSED | gcv2D_TSC_DOWN_SAMPLER,
    },
    /* 18 MSAA V4COMPRESSION_256B XMAJOR RGB565 */
    {
        "resource/vmsaa/texture5_256B_MSAA_RGB565_1024x1024_data.bin",
        "resource/vmsaa/texture5_256B_MSAA_RGB565_1024x1024_tilestatus.bin",
        1024, 1024,
        gcvSUPERTILED,
        gcvSURF_R5G6B5,
        gcv2D_TSC_V4_COMPRESSED_256B | gcv2D_TSC_DOWN_SAMPLER,
    },
    /* 19 MSAA V4COMPRESSION_256B YMAJOR RGB565 */
    {
        "resource/vmsaa/texture5_256B_MSAA_RGB565_YMAJOR_1024x1024_data.bin",
        "resource/vmsaa/texture5_256B_MSAA_RGB565_YMAJOR_1024x1024_tilestatus.bin",
        1024, 1024,
        gcvYMAJOR_SUPERTILED,
        gcvSURF_R5G6B5,
        gcv2D_TSC_V4_COMPRESSED_256B | gcv2D_TSC_DOWN_SAMPLER,
    },
    /* 20 MSAA V4COMPRESSION_256B XMAJOR XRGB8888 */
    {
        "resource/vmsaa/texture5_256B_MSAA_XRGB8888_1024x1024_data.bin",
        "resource/vmsaa/texture5_256B_MSAA_XRGB8888_1024x1024_tilestatus.bin",
        1024, 1024,
        gcvSUPERTILED,
        gcvSURF_X8R8G8B8,
        gcv2D_TSC_V4_COMPRESSED_256B | gcv2D_TSC_DOWN_SAMPLER,
    },
    /* 21 VMSAA V4COMPRESSION_256B XMAJOR RGB565 */
    {
        "resource/vmsaa/texture5_256B_VMSAA_RGB565_1024x1024_data.bin",
        "resource/vmsaa/texture5_256B_VMSAA_RGB565_1024x1024_tilestatus.bin",
        1024, 1024,
        gcvSUPERTILED,
        gcvSURF_R5G6B5,
        gcv2D_TSC_V4_COMPRESSED_256B | gcv2D_TSC_DOWN_SAMPLER,
    },
    /* 22 VMSAA V4COMPRESSION_256B YMAJOR RGB565 */
    {
        "resource/vmsaa/texture5_256B_VMSAA_RGB565_YMAJOR_1024x1024_data.bin",
        "resource/vmsaa/texture5_256B_VMSAA_RGB565_YMAJOR_1024x1024_tilestatus.bin",
        1024, 1024,
        gcvYMAJOR_SUPERTILED,
        gcvSURF_R5G6B5,
        gcv2D_TSC_V4_COMPRESSED_256B | gcv2D_TSC_DOWN_SAMPLER,
    },
    /* 23 VMSAA V4COMPRESSION_256B XMAJOR XRGB8888 */
    {
        "resource/vmsaa/texture5_256B_VMSAA_XRGB8888_1024x1024_data.bin",
        "resource/vmsaa/texture5_256B_VMSAA_XRGB8888_1024x1024_tilestatus.bin",
        1024, 1024,
        gcvSUPERTILED,
        gcvSURF_X8R8G8B8,
        gcv2D_TSC_V4_COMPRESSED_256B | gcv2D_TSC_DOWN_SAMPLER,
    },
};

typedef struct Test2D {
    GalTest     base;
    GalRuntime *runtime;

    //source surface
    T2D_SURF_PTR    surf[gcmCOUNTOF(sourcefiles)];
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
    T2D_SURF_PTR result = gcvNULL;
    gcsRECT srect, rect;
    gco2D egn2D = t2d->runtime->engine2d;
    gctUINT32 minLen;
    gceSURF_ROTATION rot = sRots[frameNo % gcmCOUNTOF(sRots)];

    // create source surface
    gcmONERROR(GalLoadVMSAARawToTSurf(
        sourcefiles[frameNo].dataFile,
        sourcefiles[frameNo].tileStatusFile,
        sourcefiles[frameNo].width,
        sourcefiles[frameNo].height,
        sourcefiles[frameNo].tiling,
        sourcefiles[frameNo].format,
        sourcefiles[frameNo].tileStatusConfig,
        &t2d->surf[frameNo]));

    minLen = gcmMIN(t2d->surf[frameNo]->width, t2d->surf[frameNo]->height);

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        t2d->runtime->format,
        gcvLINEAR, gcv2D_TSC_DISABLE,
        minLen,
        minLen,
        &result));

    srect.left   = 0;
    srect.top    = 0;
    srect.right = minLen;
    srect.bottom = minLen;

    rect = srect;

    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        t2d->surf[frameNo]->address, t2d->surf[frameNo]->validAddressNum,
        t2d->surf[frameNo]->stride, t2d->surf[frameNo]->validStrideNum,
        t2d->surf[frameNo]->tiling, t2d->surf[frameNo]->format,
        rot,
        t2d->surf[frameNo]->aWidth, t2d->surf[frameNo]->aHeight));

    gcmONERROR(gco2D_SetSourceTileStatus(
        egn2D,
        t2d->surf[frameNo]->tileStatusConfig,
        t2d->surf[frameNo]->tileStatusFormat,
        t2d->surf[frameNo]->tileStatusClear,
        t2d->surf[frameNo]->tileStatusAddress
        ));

    gcmONERROR(gco2D_SetSource(egn2D, &srect));

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
        result->aWidth,
        result->aHeight));

    gcmONERROR(gco2D_SetTargetTileStatus(
        egn2D,
        result->tileStatusConfig,
        result->format,
        gcvSURF_0_DEGREE,
        result->tileStatusAddress
        ));

    gcmONERROR(gco2D_SetClipping(egn2D, &rect));
    gcmONERROR(gco2D_Blit(egn2D, 1, &rect, 0xCC, 0xCC, result->format));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    if (!t2d->runtime->noSaveTargetNew && t2d->runtime->saveFullName)
    {
        GalSaveTSurfToDIB(result, t2d->runtime->saveFullName);
    }

OnError:
    if (result != gcvNULL)
    {
        GalDeleteTSurf(gcvNULL, result);
        result = gcvNULL;
    }

    if (t2d->surf[frameNo] != gcvNULL)
    {
        if (gcmIS_ERROR(GalDeleteTSurf(t2d->runtime->hal, t2d->surf[frameNo])))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console,
                "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));
        }
        t2d->surf[frameNo] = gcvNULL;
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
    gcvFEATURE_2D_MAJOR_SUPER_TILE,
    gcvFEATURE_2D_VMSAA,
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

    gcmONERROR(gco2D_SetStateU32(runtime->engine2d, gcv2D_STATE_XRGB_ENABLE, gcvTRUE));

    t2d->base.render      = (PGalRender)Render;
    t2d->base.destroy     = (PGalDestroy)Destroy;
    t2d->base.frameCount  = gcmCOUNTOF(t2d->surf);
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
