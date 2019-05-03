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
 *  Feature:    DEC TPC compression
 *  API:        gco2D_SetSourceTileStatus gco2D_MultiSourceBlit
 *  Check:
 */
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFormatTileStatus016\n" \
"Operation: Test MSAA and VMSAA input multisrcblit with XMjor and YMjaor tile format.\n" \
"2D API: gco2D_MultiSourceBlit\n" \
"Srcs: Size       [1024x1024]\n"\
"     Rect        [0,0,1024,1024]\n"\
"     Format      [RGB565/XRGB8888]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [tile/superTileXmajor/superTileYmajor]\n"\
"     Compression [V4 compressed]\n" \
"Dst: Size        [configurable]\n"\
"     Rect        [configurable]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0]\n"\
"     Tile        [None]\n"\
"     Compression [None]\n" \
"Brush: [None]\n"\
"Alphablend: [enable]\n" \
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
    T2D_SURF_PTR surf[gcmCOUNTOF(sourcefiles)];
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

#define NUM 8

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status;
    T2D_SURF_PTR result = gcvNULL;
    gcsRECT rect, srect;
    gco2D egn2D = t2d->runtime->engine2d;
    gceSURF_ROTATION rot;
    gctUINT32 tmp, minLen = 0, flag = 0, i, j, base;

    base = frameNo * NUM;

    for (i = base; i < base+NUM; i++)
    {
        gcmONERROR(GalLoadVMSAARawToTSurf(
            sourcefiles[i].dataFile,
            sourcefiles[i].tileStatusFile,
            sourcefiles[i].width,
            sourcefiles[i].height,
            sourcefiles[i].tiling,
            sourcefiles[i].format,
            sourcefiles[i].tileStatusConfig,
            &t2d->surf[i]));

        tmp = gcmMIN(t2d->surf[i]->width, t2d->surf[i]->height);
        if (!minLen || tmp < minLen)
            minLen = tmp;
    }

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        t2d->runtime->format,
        gcvLINEAR, gcv2D_TSC_DISABLE,
        minLen,
        minLen,
        &result));

    rect.left   = 0;
    rect.top    = 0;
    rect.right = minLen;
    rect.bottom = minLen;

    for (i = base, j = 0; i < base+NUM; i++, j++)
    {
        gcmONERROR(gco2D_SetCurrentSourceIndex(egn2D, j));

        rot = sRots[j % gcmCOUNTOF(sRots)];

        srect.left = srect.top = 0;
        srect.right = t2d->surf[i]->width;
        srect.bottom = t2d->surf[i]->height;

        gcmONERROR(gco2D_SetGenericSource(
            egn2D,
            t2d->surf[i]->address, t2d->surf[i]->validAddressNum,
            t2d->surf[i]->stride, t2d->surf[i]->validStrideNum,
            t2d->surf[i]->tiling, t2d->surf[i]->format,
            rot,
            t2d->surf[i]->aWidth, t2d->surf[i]->aHeight));

        gcmONERROR(gco2D_SetSourceTileStatus(
            egn2D,
            t2d->surf[i]->tileStatusConfig,
            t2d->surf[i]->tileStatusFormat,
            t2d->surf[i]->tileStatusClear,
            t2d->surf[i]->tileStatusAddress
            ));

        gcmONERROR(gco2D_SetSource(egn2D, &srect));

        gcmONERROR(gco2D_SetClipping(egn2D, &rect));

        gcmONERROR(gco2D_SetROP(egn2D, 0xCC, 0xCC));

        gcmONERROR(gco2D_SetTargetRect(egn2D, &rect));

        switch (j % 4)
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

        if (j)
        {
            gcmONERROR(gco2D_SetSourceGlobalColorAdvanced(t2d->runtime->engine2d, 0x80 << 24));
            gcmONERROR(gco2D_SetTargetGlobalColorAdvanced(t2d->runtime->engine2d, 0x80 << 24));
            gcmONERROR(gco2D_EnableAlphaBlendAdvanced(egn2D,
                gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
                gcvSURF_GLOBAL_ALPHA_ON, gcvSURF_GLOBAL_ALPHA_ON,
                gcvSURF_BLEND_STRAIGHT, gcvSURF_BLEND_STRAIGHT));
        }
        else
        {
            gcmONERROR(gco2D_DisableAlphaBlend(egn2D));
        }

        flag |= 0x1 << j;
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

    gcmONERROR(gco2D_MultiSourceBlit(egn2D, flag, gcvNULL, 0));

    gcmONERROR(gco2D_DisableAlphaBlend(egn2D));
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

    // destroy source surface
    for (i = base; i < base+NUM; i++)
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
    gcvFEATURE_2D_MULTI_SOURCE_BLT_EX2,
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
    t2d->base.frameCount  = gcmCOUNTOF(t2d->surf) / 8;
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
