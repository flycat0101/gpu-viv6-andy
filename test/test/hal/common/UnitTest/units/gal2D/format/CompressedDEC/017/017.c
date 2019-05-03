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
"Case gal2DFormatCompressedDEC017 : MultiSourceBlit rotation/mirror: from tpc compressed to non-compressed.\n";

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
    /* 320x256 */
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
    /* 352x288 */
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
    /* 640x480 */
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
    /* 720x576 */
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

    /* 720x576 */
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
    /* 640x480 */
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
    /* 352x288 */
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
    /* 320x256 */
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

const char *sBasicFile[] = {
    "resource/zero2_ARGB4.bmp",
    "resource/VV_Background.bmp",
    "resource/zero2_UYVY_1920x1080_Linear.vimg",
    "resource/zero2_YUY2_640X480_Linear.vimg",
};

typedef struct _MultiSrc
{
    gcoSURF        srcSurf;
    gceSURF_FORMAT    srcFormat;
    gctUINT        srcWidth;
    gctUINT        srcHeight;
    gctINT        srcStride[3];
    gctINT              srcStrideNum;
    gctINT              srcAddressNum;
    gctUINT32        srcPhyAddr[3];
    gctPOINTER        srcLgcAddr[3];
} MultiSrc, *MultiSrcPTR;

typedef struct Test2D {
    GalTest     base;
    GalRuntime *runtime;

    //source surface
    T2D_SURF_PTR surf[gcmCOUNTOF(sourcefiles)];

    MultiSrc multiSrc[gcmCOUNTOF(sBasicFile)];
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

static gceSTATUS ReloadSourceSurface(Test2D *t2d, gctUINT SrcIndex, const char * sourcefile)
{
    gceSTATUS status;
    MultiSrcPTR curSrc = &t2d->multiSrc[SrcIndex % gcmCOUNTOF(sBasicFile)];
    gctUINT32 address[3];
    gctPOINTER memory[3];
    gctSTRING pos = gcvNULL;

    // destroy source surface
    if (curSrc->srcSurf != gcvNULL)
    {
        if (curSrc->srcLgcAddr[0])
        {
            gcmONERROR(gcoSURF_Unlock(curSrc->srcSurf, curSrc->srcLgcAddr));
            curSrc->srcLgcAddr[0] = gcvNULL;
        }

        gcmONERROR(gcoSURF_Destroy(curSrc->srcSurf));
        curSrc->srcSurf = gcvNULL;
    }

    // create source surface
    gcmONERROR(GalStrSearch(sourcefile, ".bmp", &pos));
    if (pos)
    {
        curSrc->srcSurf = GalLoadDIB2Surface(t2d->runtime->hal,
            sourcefile);
        if (curSrc->srcSurf == NULL)
        {
            gcmONERROR(gcvSTATUS_NOT_FOUND);
        }
    }
    else
    {
        gcmONERROR(GalLoadVimgToSurface(
            sourcefile, &curSrc->srcSurf));
    }

    gcmONERROR(gcoSURF_GetAlignedSize(curSrc->srcSurf,
                                        gcvNULL,
                                        gcvNULL,
                                        curSrc->srcStride));

    gcmONERROR(gcoSURF_GetSize(curSrc->srcSurf,
                                &curSrc->srcWidth,
                                &curSrc->srcHeight,
                                gcvNULL));

    gcmONERROR(gcoSURF_GetFormat(curSrc->srcSurf, gcvNULL, &curSrc->srcFormat));

    gcmONERROR(gcoSURF_Lock(curSrc->srcSurf, address, memory));

    curSrc->srcPhyAddr[0]  = address[0];
    curSrc->srcLgcAddr[0]  = memory[0];

    curSrc->srcStrideNum = curSrc->srcAddressNum = 1;

    if (GalIsYUVFormat(curSrc->srcFormat))
    {
        gcmONERROR(GalQueryUVStride(curSrc->srcFormat, curSrc->srcStride[0],
                &curSrc->srcStride[1], &curSrc->srcStride[2]));

        curSrc->srcPhyAddr[1] = address[1];
        curSrc->srcLgcAddr[1] = memory[1];

        curSrc->srcPhyAddr[2] = address[2];
        curSrc->srcLgcAddr[2] = memory[2];
        switch (curSrc->srcFormat)
        {
        case gcvSURF_YUY2:
        case gcvSURF_UYVY:
            curSrc->srcStrideNum = curSrc->srcAddressNum = 1;
            break;

        case gcvSURF_I420:
        case gcvSURF_YV12:
            curSrc->srcStrideNum = curSrc->srcAddressNum = 3;
            break;

        case gcvSURF_NV16:
        case gcvSURF_NV61:
        case gcvSURF_NV12:
        case gcvSURF_NV21:
            curSrc->srcStrideNum = curSrc->srcAddressNum = 2;
            break;

        default:
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }
    }

    return gcvSTATUS_OK;

OnError:

    return status;
}

#define NUM 4

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status;
    T2D_SURF_PTR result = gcvNULL;
    gcsRECT rect;
    gco2D egn2D = t2d->runtime->engine2d;
    gceSURF_ROTATION rot;
    gctUINT32 tmp, minLen = 0, flag = 0, i, j;
    MultiSrcPTR curSrc;

    // create NV12 source surface
    for (j = 0; j < NUM; j++)
    {
        i = (frameNo % NUM) * NUM + j;

        gcmONERROR(GalLoadDECTPCRawToTSurf(
            sourcefiles[i].dataFile0,
            sourcefiles[i].dataFile1,
            sourcefiles[i].dataFile2,
            sourcefiles[i].tableFile0,
            sourcefiles[i].tableFile1,
            sourcefiles[i].tableFile2,
            sourcefiles[i].width, sourcefiles[i].height,
            sourcefiles[i].tiling,
            sourcefiles[i].format,
            sourcefiles[i].compressed,
            &t2d->surf[i]));

        tmp = gcmMIN(t2d->surf[i]->width, t2d->surf[i]->height);
        if (!minLen || tmp < minLen)
            minLen = tmp;
    }

    if (frameNo > 3)
    {
        for (i = 0; i < gcmCOUNTOF(sBasicFile); i++)
        {
            curSrc = &t2d->multiSrc[i];
            tmp = gcmMIN(curSrc->srcWidth, curSrc->srcHeight);
            if (!minLen || tmp < minLen)
                minLen = tmp;
        }
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

    for (j = 0; j < 8; j++)
    {
        gcmONERROR(gco2D_SetCurrentSourceIndex(egn2D, j));

        rot = sRots[j % gcmCOUNTOF(sRots)];

        if (j < NUM)
        {
            i = (frameNo % NUM) * NUM + j;

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

            if (t2d->surf[i]->tileStatusNodeEx[0].address != 0)
            {
                gcmONERROR(gco2D_SetStateArrayU32(
                    egn2D,
                    gcv2D_STATE_ARRAY_YUV_SRC_TILE_STATUS_ADDR,
                    &t2d->surf[i]->tileStatusNodeEx[0].address,
                    1
                    ));
            }
        }
        else
        {
            i = j - NUM;

            curSrc = &t2d->multiSrc[i];

            gcmONERROR(gco2D_SetGenericSource(
                egn2D,
                curSrc->srcPhyAddr, curSrc->srcAddressNum,
                curSrc->srcStride, curSrc->srcStrideNum,
                gcvLINEAR,
                curSrc->srcFormat,
                gcvSURF_0_DEGREE,
                curSrc->srcWidth,
                curSrc->srcHeight));

            gcmONERROR(gco2D_SetSourceTileStatus(
                egn2D,
                gcv2D_TSC_DISABLE,
                0, 0, 0
                ));
        }

        gcmONERROR(gco2D_SetSource(egn2D, &rect));

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

        flag |= 0x1 << j;

        if (frameNo <= 3 && j == 3)
            break;
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
    for (i = 0; i < NUM; i++)
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
    gceSTATUS status;
    gctINT i;

    // destroy source surface
    for (i = 0; i < gcmCOUNTOF(sBasicFile); i++)
    {
        MultiSrcPTR curSrc = &t2d->multiSrc[i];

        if (curSrc->srcSurf != gcvNULL)
        {
            if (curSrc->srcLgcAddr[0])
            {
                status = gcoSURF_Unlock(curSrc->srcSurf, curSrc->srcLgcAddr);
                if (gcmIS_ERROR(status))
                {
                    GalOutput(GalOutputType_Error | GalOutputType_Console,
                        "Unlock SrcSurface[%d] failed:%s(%d)\n", i, GalStatusString(status), status);
                }
                curSrc->srcLgcAddr[0] = gcvNULL;
            }

            status = gcoSURF_Destroy(curSrc->srcSurf);
            if (gcmIS_ERROR(status))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console,
                    "Destroy SrcSurface[%d] failed:%s(%d)\n", i, GalStatusString(status), status);
            }
        }
    }

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
    gctUINT i;

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
    t2d->base.frameCount  = 4 * 2;
    t2d->base.description = s_CaseDescription;

    for (i = 0; i < gcmCOUNTOF(sBasicFile); i++)
    {
        gcmONERROR(ReloadSourceSurface(t2d, i, sBasicFile[i]));
    }

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
