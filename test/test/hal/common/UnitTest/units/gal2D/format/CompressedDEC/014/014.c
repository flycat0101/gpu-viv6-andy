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
 *  API:        gco2D_SetSourceTileStatus gco2D_Blit
 *  Check:
 */
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFormatCompressedDEC014 : BitBlit rotation/mirror: from tpc compressed to non-compressed.\n";

typedef struct _TPC_Source_File {
    gctCONST_STRING dataFile0;
    gctCONST_STRING dataFile1;
    gctCONST_STRING dataFile2;
    gctCONST_STRING tableFile0;
    gctCONST_STRING tableFile1;
    gctCONST_STRING tableFile2;
    gctUINT32 width;
    gctUINT32 height;
    gceTILING tiling;
    gceSURF_FORMAT format;
    gctBOOL compressed;
}TPC_Source_File;

static TPC_Source_File sourcefiles[] =
{
    /* 0-1 4096x2304 */
    {
        "resource/dec_tpc/VSMain8_4096x2304_10fxNV12_TILE_COMP_f5_YData.bin",
        "resource/dec_tpc/VSMain8_4096x2304_10fxNV12_TILE_COMP_f5_UVData.bin",
        gcvNULL,
        "resource/dec_tpc/VSMain8_4096x2304_10fxNV12_TILE_COMP_f5_YTable.bin",
        "resource/dec_tpc/VSMain8_4096x2304_10fxNV12_TILE_COMP_f5_UVTable.bin",
        gcvNULL,
        4096, 2304,
        gcvLINEAR,
        gcvSURF_NV12,
        gcvTRUE,
    },
    {
        "resource/dec_tpc/VSMain8_4096x2304_10fxNV12_TILE_f5.raw",
        gcvNULL,
        gcvNULL,
        gcvNULL,
        gcvNULL,
        gcvNULL,
        4096, 2304,
        gcvLINEAR,
        gcvSURF_NV12,
        gcvFALSE,
    },
    /* 2-3 3840x2160 */
    {
        "resource/dec_tpc/VSMain8_3840x2160_10fxNV12_TILE_COMP_f5_YData.bin",
        "resource/dec_tpc/VSMain8_3840x2160_10fxNV12_TILE_COMP_f5_UVData.bin",
        gcvNULL,
        "resource/dec_tpc/VSMain8_3840x2160_10fxNV12_TILE_COMP_f5_YTable.bin",
        "resource/dec_tpc/VSMain8_3840x2160_10fxNV12_TILE_COMP_f5_UVTable.bin",
        gcvNULL,
        3840, 2160,
        gcvLINEAR,
        gcvSURF_NV12,
        gcvTRUE,
    },
    {
        "resource/dec_tpc/VSMain8_3840x2160_10fxNV12_TILE_f5.raw",
        gcvNULL,
        gcvNULL,
        gcvNULL,
        gcvNULL,
        gcvNULL,
        3840, 2160,
        gcvLINEAR,
        gcvSURF_NV12,
        gcvFALSE,
    },
    /* 4-5 1920x1088 */
    {
        "resource/dec_tpc/VSMain8_1920x1088_10fxNV12_TILE_COMP_f5_YData.bin",
        "resource/dec_tpc/VSMain8_1920x1088_10fxNV12_TILE_COMP_f5_UVData.bin",
        gcvNULL,
        "resource/dec_tpc/VSMain8_1920x1088_10fxNV12_TILE_COMP_f5_YTable.bin",
        "resource/dec_tpc/VSMain8_1920x1088_10fxNV12_TILE_COMP_f5_UVTable.bin",
        gcvNULL,
        1920, 1088,
        gcvLINEAR,
        gcvSURF_NV12,
        gcvTRUE,
    },
    {
        "resource/dec_tpc/VSMain8_1920x1088_10fxNV12_TILE_f5.raw",
        gcvNULL,
        gcvNULL,
        gcvNULL,
        gcvNULL,
        gcvNULL,
        1920, 1088,
        gcvLINEAR,
        gcvSURF_NV12,
        gcvFALSE,
    },
    /* 6-7 1280x720 */
    {
        "resource/dec_tpc/VSMain8_1280x720_1fxNV12_TILE_COMP_f0_YData.bin",
        "resource/dec_tpc/VSMain8_1280x720_1fxNV12_TILE_COMP_f0_UVData.bin",
        gcvNULL,
        "resource/dec_tpc/VSMain8_1280x720_1fxNV12_TILE_COMP_f0_YTable.bin",
        "resource/dec_tpc/VSMain8_1280x720_1fxNV12_TILE_COMP_f0_UVTable.bin",
        gcvNULL,
        1280, 720,
        gcvLINEAR,
        gcvSURF_NV12,
        gcvTRUE,
    },
    {
        "resource/dec_tpc/VSMain8_1280x720_1fxNV12_TILE_f0.raw",
        gcvNULL,
        gcvNULL,
        gcvNULL,
        gcvNULL,
        gcvNULL,
        1280, 720,
        gcvLINEAR,
        gcvSURF_NV12,
        gcvFALSE,
    },
    /* 8-9 720x576 */
    {
        "resource/dec_tpc/VSMain8_720x576_1fxNV12_TILE_COMP_f0_YData.bin",
        "resource/dec_tpc/VSMain8_720x576_1fxNV12_TILE_COMP_f0_UVData.bin",
        gcvNULL,
        "resource/dec_tpc/VSMain8_720x576_1fxNV12_TILE_COMP_f0_YTable.bin",
        "resource/dec_tpc/VSMain8_720x576_1fxNV12_TILE_COMP_f0_UVTable.bin",
        gcvNULL,
        720, 576,
        gcvLINEAR,
        gcvSURF_NV12,
        gcvTRUE,
    },
    {
        "resource/dec_tpc/VSMain8_720x576_1fxNV12_TILE_f0.raw",
        gcvNULL,
        gcvNULL,
        gcvNULL,
        gcvNULL,
        gcvNULL,
        720, 576,
        gcvLINEAR,
        gcvSURF_NV12,
        gcvFALSE,
    },
    /* 10-11 640x480 */
    {
        "resource/dec_tpc/VSMain8_640x480_10fxNV12_TILE_COMP_f5_YData.bin",
        "resource/dec_tpc/VSMain8_640x480_10fxNV12_TILE_COMP_f5_UVData.bin",
        gcvNULL,
        "resource/dec_tpc/VSMain8_640x480_10fxNV12_TILE_COMP_f5_YTable.bin",
        "resource/dec_tpc/VSMain8_640x480_10fxNV12_TILE_COMP_f5_UVTable.bin",
        gcvNULL,
        640, 480,
        gcvLINEAR,
        gcvSURF_NV12,
        gcvTRUE,
    },
    {
        "resource/dec_tpc/VSMain8_640x480_10fxNV12_TILE_f5.raw",
        gcvNULL,
        gcvNULL,
        gcvNULL,
        gcvNULL,
        gcvNULL,
        640, 480,
        gcvLINEAR,
        gcvSURF_NV12,
        gcvFALSE,
    },
    /* 12-13 352x288 */
    {
        "resource/dec_tpc/VSMain8_352x288_10fxNV12_TILE_COMP_f5_YData.bin",
        "resource/dec_tpc/VSMain8_352x288_10fxNV12_TILE_COMP_f5_UVData.bin",
        gcvNULL,
        "resource/dec_tpc/VSMain8_352x288_10fxNV12_TILE_COMP_f5_YTable.bin",
        "resource/dec_tpc/VSMain8_352x288_10fxNV12_TILE_COMP_f5_UVTable.bin",
        gcvNULL,
        352, 288,
        gcvLINEAR,
        gcvSURF_NV12,
        gcvTRUE,
    },
    {
        "resource/dec_tpc/VSMain8_352x288_10fxNV12_TILE_f5.raw",
        gcvNULL,
        gcvNULL,
        gcvNULL,
        gcvNULL,
        gcvNULL,
        352, 288,
        gcvLINEAR,
        gcvSURF_NV12,
        gcvFALSE,
    },
    /* 14-15 320x256 */
    {
        "resource/dec_tpc/VSMain8_320x256_10fxNV12_TILE_COMP_f5_YData.bin",
        "resource/dec_tpc/VSMain8_320x256_10fxNV12_TILE_COMP_f5_UVData.bin",
        gcvNULL,
        "resource/dec_tpc/VSMain8_320x256_10fxNV12_TILE_COMP_f5_YTable.bin",
        "resource/dec_tpc/VSMain8_320x256_10fxNV12_TILE_COMP_f5_UVTable.bin",
        gcvNULL,
        320, 256,
        gcvLINEAR,
        gcvSURF_NV12,
        gcvTRUE,
    },
    {
        "resource/dec_tpc/VSMain8_320x256_10fxNV12_TILE_f5.raw",
        gcvNULL,
        gcvNULL,
        gcvNULL,
        gcvNULL,
        gcvNULL,
        320, 256,
        gcvLINEAR,
        gcvSURF_NV12,
        gcvFALSE,
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
    gcsRECT rect;
    gco2D egn2D = t2d->runtime->engine2d;
    gctUINT32 minLen;
    gceSURF_ROTATION rot = sRots[frameNo % gcmCOUNTOF(sRots)];

    // create source surface
    gcmONERROR(GalLoadDECTPCRawToTSurf(
        sourcefiles[frameNo].dataFile0,
        sourcefiles[frameNo].dataFile1,
        sourcefiles[frameNo].dataFile2,
        sourcefiles[frameNo].tableFile0,
        sourcefiles[frameNo].tableFile1,
        sourcefiles[frameNo].tableFile2,
        sourcefiles[frameNo].width,
        sourcefiles[frameNo].height,
        sourcefiles[frameNo].tiling,
        sourcefiles[frameNo].format,
        sourcefiles[frameNo].compressed,
        &t2d->surf[frameNo]));

    minLen = gcmMIN(t2d->surf[frameNo]->width, t2d->surf[frameNo]->height);

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        t2d->runtime->format,
        gcvLINEAR, gcv2D_TSC_DISABLE,
        t2d->surf[frameNo]->width,
        t2d->surf[frameNo]->height,
        &result));

    rect.left   = 0;
    rect.top    = 0;
    if (rot == gcvSURF_90_DEGREE || rot == gcvSURF_270_DEGREE)
    {
        rect.right = minLen;
        rect.bottom = minLen;
    }
    else
    {
        rect.right = t2d->surf[frameNo]->width;
        rect.bottom = t2d->surf[frameNo]->height;
    }

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

    if (t2d->surf[frameNo]->tileStatusNodeEx[0].address != 0)
    {
        gcmONERROR(gco2D_SetStateArrayU32(
            egn2D,
            gcv2D_STATE_ARRAY_YUV_SRC_TILE_STATUS_ADDR,
            &t2d->surf[frameNo]->tileStatusNodeEx[0].address,
            1
            ));
    }

    gcmONERROR(gco2D_SetSource(egn2D, &rect));

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
    gcvFEATURE_DEC300_COMPRESSION,
    gcvFEATURE_DEC_TPC_COMPRESSION,
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
