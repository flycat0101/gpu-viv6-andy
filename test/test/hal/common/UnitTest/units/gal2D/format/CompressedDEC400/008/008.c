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
 *  API:         gco2D_SetGenericSource gco2D_SetGenericTarget gco2D_MultiSourceBlit
 *  Check:
*/
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFormatCompressedDEC400_008 : multi-src blit/rotation with alpha: mixed compressed & uncompressed and argb & yuv to compressed and argb.\n";

#define INT2BOOL(i,d) ((i)%(d) == 0)?gcvFALSE:gcvTRUE

typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;
    //source surface
    T2D_SURF_PTR    surf[10];

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
    gce2D_TILE_STATUS_CONFIG tsc;
} Comb;

static Comb sCombUnCom[] =
{
    { gcvSURF_A1R5G5B5,          gcvSUPERTILED,           gcv2D_TSC_DISABLE },
    { gcvSURF_A4R4G4B4,          gcvSUPERTILED,           gcv2D_TSC_DISABLE },
    { gcvSURF_X1R5G5B5,          gcvSUPERTILED,           gcv2D_TSC_DISABLE },
    { gcvSURF_X4R4G4B4,          gcvSUPERTILED,           gcv2D_TSC_DISABLE },
    { gcvSURF_R5G6B5,            gcvSUPERTILED,           gcv2D_TSC_DISABLE },
    { gcvSURF_X8R8G8B8,          gcvSUPERTILED,           gcv2D_TSC_DISABLE },
    { gcvSURF_A8R8G8B8,          gcvSUPERTILED,           gcv2D_TSC_DISABLE },
    { gcvSURF_A2R10G10B10,       gcvSUPERTILED,           gcv2D_TSC_DISABLE },
    { gcvSURF_YUY2,              gcvSUPERTILED,           gcv2D_TSC_DISABLE },
    { gcvSURF_UYVY,              gcvSUPERTILED,           gcv2D_TSC_DISABLE },

    { gcvSURF_X8R8G8B8,          gcvTILED_4X8,            gcv2D_TSC_DISABLE },
    { gcvSURF_A8R8G8B8,          gcvTILED_4X8,            gcv2D_TSC_DISABLE },
    { gcvSURF_A2R10G10B10,       gcvTILED_4X8,            gcv2D_TSC_DISABLE },

    { gcvSURF_X8R8G8B8,          gcvTILED_8X4,            gcv2D_TSC_DISABLE },
    { gcvSURF_A8R8G8B8,          gcvTILED_8X4,            gcv2D_TSC_DISABLE },
    { gcvSURF_A2R10G10B10,       gcvTILED_8X4,            gcv2D_TSC_DISABLE },

    { gcvSURF_P010,              gcvTILED_32X4,           gcv2D_TSC_DISABLE },

    { gcvSURF_NV12,              gcvTILED_64X4,           gcv2D_TSC_DISABLE },

    { gcvSURF_A1R5G5B5,    gcvTILED_8X8_XMAJOR,           gcv2D_TSC_DISABLE },
    { gcvSURF_A4R4G4B4,    gcvTILED_8X8_XMAJOR,           gcv2D_TSC_DISABLE },
    { gcvSURF_X1R5G5B5,    gcvTILED_8X8_XMAJOR,           gcv2D_TSC_DISABLE },
    { gcvSURF_X4R4G4B4,    gcvTILED_8X8_XMAJOR,           gcv2D_TSC_DISABLE },
    { gcvSURF_R5G6B5,      gcvTILED_8X8_XMAJOR,           gcv2D_TSC_DISABLE },
    { gcvSURF_X8R8G8B8,    gcvTILED_8X8_XMAJOR,           gcv2D_TSC_DISABLE },
    { gcvSURF_A8R8G8B8,    gcvTILED_8X8_XMAJOR,           gcv2D_TSC_DISABLE },
    { gcvSURF_A2R10G10B10, gcvTILED_8X8_XMAJOR,           gcv2D_TSC_DISABLE },
    { gcvSURF_YUY2,        gcvTILED_8X8_XMAJOR,           gcv2D_TSC_DISABLE },
    { gcvSURF_UYVY,        gcvTILED_8X8_XMAJOR,           gcv2D_TSC_DISABLE },

    { gcvSURF_NV12,        gcvTILED_8X8_YMAJOR,           gcv2D_TSC_DISABLE },
    { gcvSURF_P010,        gcvTILED_8X8_YMAJOR,           gcv2D_TSC_DISABLE },
};

static Comb sCombCom[] =
{
    { gcvSURF_A1R5G5B5,          gcvSUPERTILED_128B,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_A4R4G4B4,          gcvSUPERTILED_128B,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_X1R5G5B5,          gcvSUPERTILED_128B,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_X4R4G4B4,          gcvSUPERTILED_128B,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_R5G6B5,            gcvSUPERTILED_128B,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_X8R8G8B8,          gcvSUPERTILED_128B,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_A8R8G8B8,          gcvSUPERTILED_128B,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_A2R10G10B10,       gcvSUPERTILED_128B,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_YUY2,              gcvSUPERTILED_128B,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_UYVY,              gcvSUPERTILED_128B,    gcv2D_TSC_DEC_COMPRESSED },

    { gcvSURF_X8R8G8B8,          gcvTILED_4X8,     gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_A8R8G8B8,          gcvTILED_4X8,     gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_A2R10G10B10,       gcvTILED_4X8,     gcv2D_TSC_DEC_COMPRESSED },

    { gcvSURF_X8R8G8B8,          gcvTILED_8X4,     gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_A8R8G8B8,          gcvTILED_8X4,     gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_A2R10G10B10,       gcvTILED_8X4,     gcv2D_TSC_DEC_COMPRESSED },

    { gcvSURF_P010,              gcvTILED_32X4,    gcv2D_TSC_DEC_COMPRESSED },

    { gcvSURF_NV12,              gcvTILED_64X4,    gcv2D_TSC_DEC_COMPRESSED },

    { gcvSURF_A1R5G5B5,    gcvTILED_8X8_XMAJOR,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_A4R4G4B4,    gcvTILED_8X8_XMAJOR,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_X1R5G5B5,    gcvTILED_8X8_XMAJOR,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_X4R4G4B4,    gcvTILED_8X8_XMAJOR,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_R5G6B5,      gcvTILED_8X8_XMAJOR,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_X8R8G8B8,    gcvTILED_8X8_XMAJOR,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_A8R8G8B8,    gcvTILED_8X8_XMAJOR,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_A2R10G10B10, gcvTILED_8X8_XMAJOR,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_YUY2,        gcvTILED_8X8_XMAJOR,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_UYVY,        gcvTILED_8X8_XMAJOR,    gcv2D_TSC_DEC_COMPRESSED },

    { gcvSURF_NV12,        gcvTILED_8X8_YMAJOR,           gcv2D_TSC_DISABLE },
    { gcvSURF_P010,        gcvTILED_8X8_YMAJOR,           gcv2D_TSC_DISABLE },
};

#define RECT_SIZE 384

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status;
    gcsRECT Rect;
    gco2D egn2D = t2d->runtime->engine2d;
    gctINT i, k;

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        t2d->runtime->format,
        gcvLINEAR,
        gcv2D_TSC_DISABLE,
        RECT_SIZE, RECT_SIZE,
        &t2d->surf[8]));

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        t2d->compressed ?
        sCombCom[frameNo % gcmCOUNTOF(sCombCom)].format :
        sCombUnCom[frameNo % gcmCOUNTOF(sCombUnCom)].format,
        t2d->compressed ?
        sCombCom[frameNo % gcmCOUNTOF(sCombCom)].tiling :
        sCombUnCom[frameNo % gcmCOUNTOF(sCombUnCom)].tiling,
        t2d->compressed ?
        sCombCom[frameNo % gcmCOUNTOF(sCombCom)].tsc :
        sCombUnCom[frameNo % gcmCOUNTOF(sCombUnCom)].tsc,
        RECT_SIZE, RECT_SIZE,
        &t2d->surf[9]));

    gcmONERROR(gco2D_SetGenericTarget(
         egn2D,
         t2d->surf[9]->address,
         t2d->surf[9]->validAddressNum,
         t2d->surf[9]->stride,
         t2d->surf[9]->validStrideNum,
         t2d->surf[9]->tiling,
         t2d->surf[9]->format,
         t2d->surf[9]->rotation,
         t2d->surf[9]->aWidth,
         t2d->surf[9]->aHeight));

    gcmONERROR(gco2D_SetTargetTileStatus(
         egn2D,
         t2d->surf[9]->tileStatusConfig,
         t2d->surf[9]->format,
         0,
         t2d->surf[9]->tileStatusAddress
         ));

    if (t2d->surf[9]->tileStatusNodeEx[0].address != 0)
    {
        gcmONERROR(gco2D_SetStateArrayU32(
            egn2D,
            gcv2D_STATE_ARRAY_YUV_DST_TILE_STATUS_ADDR,
            &t2d->surf[9]->tileStatusNodeEx[0].address,
            1
            ));
    }

    Rect.left = 0;
    Rect.top  = 0;
    Rect.right  = t2d->surf[9]->width;
    Rect.bottom = t2d->surf[9]->height;

    for (k = 0; k < 8; k++)
    {
        gcsRECT srect;
        gceSURF_ROTATION rot;
        gctBOOL horMirror, verMirror;

        srect.left = srect.top = 0;
        srect.right = srect.bottom = RECT_SIZE;

        i = (frameNo + k) % 8;

        gcmONERROR(gco2D_SetCurrentSourceIndex(egn2D, i));

        rot = sRots[(frameNo + k) % gcmCOUNTOF(sRots)];
        horMirror = INT2BOOL((frameNo + k), 2);
        verMirror = INT2BOOL((frameNo + k), 4);

        if (t2d->surf[9]->format == gcvSURF_YUY2 ||
            t2d->surf[9]->format == gcvSURF_UYVY)
        {
            rot = horMirror = verMirror = gcvFALSE;
        }

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

        gcmONERROR(gco2D_SetSource(egn2D, &srect));

        gcmONERROR(gco2D_SetClipping(egn2D, &srect));

        gcmONERROR(gco2D_SetROP(egn2D, 0xCC, 0xCC));

        gcmONERROR(gco2D_SetBitBlitMirror(egn2D, horMirror, verMirror));

        if (!i)
        {
            gcmONERROR(gco2D_DisableAlphaBlend(egn2D));
        }
        else
        {
            switch(t2d->surf[i]->format)
            {
                case gcvSURF_A8R8G8B8:
                    gcmONERROR(gco2D_SetPorterDuffBlending(
                        egn2D,
                        gcvPD_SRC_OVER));
                    break;

                case gcvSURF_NV12:
                case gcvSURF_YUY2:
                case gcvSURF_UYVY:
                default:
                    gcmONERROR(gco2D_SetSourceGlobalColorAdvanced(t2d->runtime->engine2d, 0x40 << 24));

                    gcmONERROR(gco2D_SetTargetGlobalColorAdvanced(t2d->runtime->engine2d, 0x40 << 24));

                    gcmONERROR(gco2D_EnableAlphaBlendAdvanced(egn2D,
                        gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
                        gcvSURF_GLOBAL_ALPHA_ON, gcvSURF_GLOBAL_ALPHA_ON,
                        gcvSURF_BLEND_STRAIGHT, gcvSURF_BLEND_STRAIGHT));
                    break;
            }
        }

        gcmONERROR(gco2D_SetTargetRect(egn2D, &srect));
    }

    gcmONERROR(gco2D_MultiSourceBlit(egn2D, 0xFF, gcvNULL, 0));

    gcmONERROR(gco2D_DisableAlphaBlend(egn2D));

    gcmONERROR(gco2D_SetBitBlitMirror(egn2D, gcvFALSE, gcvFALSE));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));


    /* render the result to uncompressed dst surface. */
    gcmONERROR(gco2D_SetCurrentSourceIndex(egn2D, 0));

    gcmONERROR(gco2D_SetGenericSource(
         egn2D,
         t2d->surf[9]->address,
         t2d->surf[9]->validAddressNum,
         t2d->surf[9]->stride,
         t2d->surf[9]->validStrideNum,
         t2d->surf[9]->tiling,
         t2d->surf[9]->format,
         t2d->surf[9]->rotation,
         t2d->surf[9]->aWidth,
         t2d->surf[9]->aHeight));

    gcmONERROR(gco2D_SetSourceTileStatus(
         egn2D,
         t2d->surf[9]->tileStatusConfig,
         t2d->surf[9]->format,
         t2d->surf[9]->tileStatusClear,
         t2d->surf[9]->tileStatusAddress
         ));

    if (t2d->surf[9]->tileStatusNodeEx[0].address != 0)
    {
        gcmONERROR(gco2D_SetStateArrayU32(
            egn2D,
            gcv2D_STATE_ARRAY_YUV_SRC_TILE_STATUS_ADDR,
            &t2d->surf[9]->tileStatusNodeEx[0].address,
            1
            ));
    }

    gcmONERROR(gco2D_SetGenericTarget(
         egn2D,
         t2d->surf[8]->address,
         t2d->surf[8]->validAddressNum,
         t2d->surf[8]->stride,
         t2d->surf[8]->validStrideNum,
         t2d->surf[8]->tiling,
         t2d->surf[8]->format,
         t2d->surf[8]->rotation,
         t2d->surf[8]->width,
         t2d->surf[8]->height));

    gcmONERROR(gco2D_SetTargetTileStatus(
         egn2D,
         t2d->surf[8]->tileStatusConfig,
         t2d->surf[8]->format,
         t2d->surf[8]->tileStatusClear,
         t2d->surf[8]->tileStatusAddress
         ));

    gcmONERROR(gco2D_SetSource(egn2D, &Rect));
    gcmONERROR(gco2D_SetClipping(egn2D, &Rect));
    gcmONERROR(gco2D_SetTargetRect(egn2D, &Rect));
    gcmONERROR(gco2D_MultiSourceBlit(egn2D, 0x1, gcvNULL, 0));

    gcmONERROR(gco2D_Flush(egn2D));
    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    if (t2d->runtime->saveFullName)
    {
        GalSaveTSurfToDIB(t2d->surf[8], t2d->runtime->saveFullName);
    }

    gcmONERROR(GalDeleteTSurf(t2d->runtime->hal, t2d->surf[8]));
    gcmONERROR(GalDeleteTSurf(t2d->runtime->hal, t2d->surf[9]));

    return gcvTRUE;

OnError:
    GalOutput(GalOutputType_Error | GalOutputType_Console,
    "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));
    return gcvFALSE;
}

static void CDECL Destroy(Test2D *t2d)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT i;

    // destroy source surface
    for (i = 0; i < 8; ++i)
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
    gcvFEATURE_DEC400_COMPRESSION,
    gcvFEATURE_2D_MULTI_SOURCE_BLT_EX2,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT i;
    gctSTRING sourcefile[] =
    {
        "resource/VV_Background.bmp",
        "resource/Crew_NV12_1280x720_Linear.vimg",
        "resource/zero2_A2R10G10B10.bmp",
        "resource/zero2_YUY2_640X480_Linear.vimg",
        "resource/relative.bmp",
        "resource/android_720x1280_icons.bmp",
        "resource/smooth_720p.bmp",
        "resource/zero2_UYVY_640X480_Linear.vimg",
    };

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

    t2d->compressed = gcvTRUE;
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

    gcmONERROR(GalCreateTSurf(
         t2d->runtime->hal,
         gcvSURF_A8R8G8B8,
         gcvLINEAR,
         gcv2D_TSC_DISABLE,
         480, 480,
         &t2d->surf[8]));

    gcmONERROR(gco2D_SetStateU32(t2d->runtime->engine2d, gcv2D_STATE_XRGB_ENABLE, gcvTRUE));

    // create source surface
    for (i = 0; i < 8; ++i)
    {
        T2D_SURF_PTR srcImage;
        gcsRECT srect, rect;
        gce2D_TILE_STATUS_CONFIG tileStatus =
            t2d->compressed ? gcv2D_TSC_DEC_COMPRESSED : gcv2D_TSC_DISABLE;

        gceTILING tiling = t2d->compressed ? gcvSUPERTILED_128B : gcvLINEAR;

        gcmONERROR(GalLoadFileToTSurf(
            sourcefile[i], &srcImage));

         gcmONERROR(gco2D_SetGenericSource(
             t2d->runtime->engine2d,
             srcImage->address, srcImage->validAddressNum,
             srcImage->stride, srcImage->validStrideNum,
             srcImage->tiling, srcImage->format,
             gcvSURF_0_DEGREE,
             srcImage->width, srcImage->height));

         gcmONERROR(gco2D_SetSourceTileStatus(
             t2d->runtime->engine2d,
             srcImage->tileStatusConfig,
             srcImage->tileStatusFormat,
             srcImage->tileStatusClear,
             srcImage->tileStatusAddress
             ));

         gcmONERROR(gco2D_SetGenericTarget(
             t2d->runtime->engine2d,
             t2d->surf[8]->address,
             t2d->surf[8]->validAddressNum,
             t2d->surf[8]->stride,
             t2d->surf[8]->validStrideNum,
             t2d->surf[8]->tiling,
             t2d->surf[8]->format,
             t2d->surf[8]->rotation,
             t2d->surf[8]->width,
             t2d->surf[8]->height));

         gcmONERROR(gco2D_SetTargetTileStatus(
             t2d->runtime->engine2d,
             t2d->surf[8]->tileStatusConfig,
             t2d->surf[8]->format,
             t2d->surf[8]->rotation,
             t2d->surf[8]->tileStatusAddress
             ));

        srect.left = 0;
        srect.top  = 0;
        srect.right  = srcImage->width;
        srect.bottom = srcImage->height;

        rect.left = 0;
        rect.top  = 0;
        rect.right  = t2d->surf[8]->width;
        rect.bottom = t2d->surf[8]->height;

        gcmONERROR(gco2D_SetStretchRectFactors(
             t2d->runtime->engine2d,
             &srect,
             &rect));

        gcmONERROR(gco2D_SetClipping(t2d->runtime->engine2d, &rect));

        gcmONERROR(gco2D_SetSource(t2d->runtime->engine2d, &srect));

        gcmONERROR(gco2D_StretchBlit(t2d->runtime->engine2d, 1, &rect, 0xCC, 0xCC, t2d->surf[8]->format));

        gcmONERROR(gcoHAL_Commit(gcvNULL, gcvTRUE));

        if(srcImage->format == gcvSURF_NV12 ||
           srcImage->format == gcvSURF_UYVY ||
           srcImage->format == gcvSURF_YUY2 ||
           srcImage->format == gcvSURF_A2R10G10B10)
        {
            t2d->surf[i] = srcImage;
            tileStatus = gcv2D_TSC_DISABLE;
        }
        else
        {
             gcmONERROR(GalCreateTSurf(
                     t2d->runtime->hal,
                     srcImage->format,
                     tiling,
                     tileStatus,
                     t2d->surf[8]->width, t2d->surf[8]->height, &t2d->surf[i]));

             gcmONERROR(gco2D_SetGenericSource(
                     t2d->runtime->engine2d,
                     t2d->surf[8]->address, t2d->surf[8]->validAddressNum,
                     t2d->surf[8]->stride, t2d->surf[8]->validStrideNum,
                     t2d->surf[8]->tiling, t2d->surf[8]->format,
                     gcvSURF_0_DEGREE,
                     t2d->surf[8]->width, t2d->surf[8]->height));

             gcmONERROR(gco2D_SetSourceTileStatus(
                     t2d->runtime->engine2d,
                     t2d->surf[8]->tileStatusConfig,
                     t2d->surf[8]->tileStatusFormat,
                     t2d->surf[8]->tileStatusClear,
                     t2d->surf[8]->tileStatusAddress
                     ));

             gcmONERROR(gco2D_SetGenericTarget(
                     t2d->runtime->engine2d,
                     t2d->surf[i]->address,
                     t2d->surf[i]->validAddressNum,
                     t2d->surf[i]->stride,
                     t2d->surf[i]->validStrideNum,
                     t2d->surf[i]->tiling,
                     t2d->surf[i]->format,
                     t2d->surf[i]->rotation,
                     t2d->surf[i]->aWidth,
                     t2d->surf[i]->aHeight));

             gcmONERROR(gco2D_SetTargetTileStatus(
                     t2d->runtime->engine2d,
                     t2d->surf[i]->tileStatusConfig,
                     t2d->surf[i]->format,
                     gcvSURF_0_DEGREE,
                     t2d->surf[i]->tileStatusAddress
                     ));

            srect.left = 0;
            srect.top  = 0;
            srect.right  = t2d->surf[i]->width;
            srect.bottom = t2d->surf[i]->height;

            gcmONERROR(gco2D_SetClipping(t2d->runtime->engine2d, &srect));

            gcmONERROR(gco2D_SetSource(t2d->runtime->engine2d, &srect));

            gcmONERROR(gco2D_Blit(t2d->runtime->engine2d, 1, &srect, 0xCC, 0xCC, t2d->surf[i]->format));

            gcmONERROR(gcoHAL_Commit(gcvNULL, gcvTRUE));

            gcmONERROR(GalDeleteTSurf(t2d->runtime->hal, srcImage));
        }
    }

    gcmONERROR(GalDeleteTSurf(t2d->runtime->hal, t2d->surf[8]));

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = t2d->compressed ? gcmCOUNTOF(sCombCom) : gcmCOUNTOF(sCombUnCom);
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
