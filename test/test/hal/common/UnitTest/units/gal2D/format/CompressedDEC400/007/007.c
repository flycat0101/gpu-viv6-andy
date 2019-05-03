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
"Case gal2DFormatCompressedDEC400_007 : multi-src blit/rotation/alphablend/mirror: from DEC400 compressed to non compressed.\n";

typedef struct Test2D {
    GalTest     base;
    GalRuntime *runtime;

    gctBOOL     compressed;

    //source surface
    T2D_SURF_PTR    surf[8];
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
};

static Comb sCombUnComYUV420[] =
{
    { gcvSURF_P010,              gcvTILED_32X4,           gcv2D_TSC_DISABLE },
    { gcvSURF_NV12,              gcvTILED_64X4,           gcv2D_TSC_DISABLE },
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
};

static Comb sCombComYUV420[] =
{
    { gcvSURF_P010,              gcvTILED_32X4,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_NV12,              gcvTILED_64X4,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_NV12,        gcvTILED_8X8_YMAJOR,           gcv2D_TSC_DISABLE },
    { gcvSURF_P010,        gcvTILED_8X8_YMAJOR,           gcv2D_TSC_DISABLE },
};

static gceSTATUS CDECL InitSourceSurface(Test2D *t2d, gctUINT num)
{
    T2D_SURF_PTR surf;
    gcsRECT dstRect, headRect;
    gco2D egn2D = t2d->runtime->engine2d;
    gceSTATUS status;
    gctINT i;
    gctUINT32 color;
    gctUINT32 offset, index;

    index = num % 8;

    if (index < 4)
    {
        gcmONERROR(GalCreateTSurf(
            t2d->runtime->hal,
            t2d->compressed ?
            sCombCom[num % gcmCOUNTOF(sCombComYUV420)].format :
            sCombUnCom[num % gcmCOUNTOF(sCombUnComYUV420)].format,
            t2d->compressed ?
            sCombCom[num % gcmCOUNTOF(sCombComYUV420)].tiling :
            sCombUnCom[num % gcmCOUNTOF(sCombUnComYUV420)].tiling,
            t2d->compressed ?
            sCombCom[num % gcmCOUNTOF(sCombComYUV420)].tsc :
            sCombUnCom[num % gcmCOUNTOF(sCombUnComYUV420)].tsc,
            480, 480, &surf));
    }
    else
    {
        gcmONERROR(GalCreateTSurf(
            t2d->runtime->hal,
            t2d->compressed ?
            sCombCom[num % gcmCOUNTOF(sCombCom)].format :
            sCombUnCom[num % gcmCOUNTOF(sCombUnCom)].format,
            t2d->compressed ?
            sCombCom[num % gcmCOUNTOF(sCombCom)].tiling :
            sCombUnCom[num % gcmCOUNTOF(sCombUnCom)].tiling,
            t2d->compressed ?
            sCombCom[num % gcmCOUNTOF(sCombCom)].tsc :
            sCombUnCom[num % gcmCOUNTOF(sCombUnCom)].tsc,
            480, 480, &surf));
    }

    dstRect.left   = 0;
    dstRect.top    = 0;
    dstRect.right  = surf->width;
    dstRect.bottom = surf->height;

    switch (index)
    {
        case 0: color = COLOR_ARGB8(0xff, 0xff, 0x0,  0x0);  break;
        case 1: color = COLOR_ARGB8(0xff, 0xff, 0xff, 0x0);  break;
        case 2: color = COLOR_ARGB8(0xff, 0xff, 0x0,  0xff); break;
        case 3: color = COLOR_ARGB8(0xff, 0x0,  0xff, 0x0);  break;
        case 4: color = COLOR_ARGB8(0xff, 0x0,  0x0,  0xff); break;
        case 5: color = COLOR_ARGB8(0xff, 0x0,  0xff, 0xff); break;
        case 6: color = COLOR_ARGB8(0xff, 0x80, 0x80, 0x80); break;
        case 7: color = COLOR_ARGB8(0xff, 0xff, 0x80, 0x80); break;
        default: gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        surf->address,
        surf->validAddressNum,
        surf->stride,
        surf->validStrideNum,
        surf->tiling,
        surf->format,
        surf->rotation,
        surf->aWidth,
        surf->aHeight));

    gcmONERROR(gco2D_SetTargetTileStatus(
        egn2D,
        surf->tileStatusConfig,
        surf->format,
        surf->tileStatusClear,
        surf->tileStatusAddress
        ));

    if (surf->tileStatusNodeEx[0].address != 0)
    {
        gcmONERROR(gco2D_SetStateArrayU32(
            egn2D,
            gcv2D_STATE_ARRAY_YUV_DST_TILE_STATUS_ADDR,
            &surf->tileStatusNodeEx[0].address,
            1
            ));
    }

    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    dstRect.left  = dstRect.top    = 24 * index;
    dstRect.right = dstRect.bottom = 480 - 24 * index;

    headRect.left   = dstRect.left;
    headRect.top    = dstRect.top;
    headRect.right  = dstRect.right;

    dstRect.top    += 12;
    headRect.bottom = dstRect.top;

    offset = (480 - 24 * index * 2) / 8;

    for (i = 0; i < 3; ++i)
    {
        switch (i)
        {
            case 0:
                dstRect.left  = 24 * index;
                dstRect.right = dstRect.left + offset * 3;
                break;

            case 1:
                dstRect.left  = 24 * index + offset * 4;
                dstRect.right = dstRect.left + offset * 2;
                break;

            case 2:
                dstRect.left  = 24 * index + offset * 7;
                dstRect.right = dstRect.left + offset;
                break;
        }

        headRect.left  = dstRect.left;
        headRect.right = dstRect.right;

        gcmONERROR(gco2D_LoadSolidBrush(egn2D, surf->format, gcvTRUE, color, 0));
        gcmONERROR(gco2D_Blit(egn2D, 1, &dstRect, 0xF0, 0xF0, surf->format));

        gcmONERROR(gco2D_LoadSolidBrush(egn2D, surf->format, gcvTRUE, COLOR_ARGB8(0xff, 0xff, 0xff, 0xff), 0));
        gcmONERROR(gco2D_Blit(egn2D, 1, &headRect, 0xF0, 0xF0, surf->format));
    }

    gcmONERROR(gco2D_Flush(egn2D));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    t2d->surf[index] = surf;

OnError:
    return status;
}

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status;
    gco2D egn2D = t2d->runtime->engine2d;
    gctINT i;
    T2D_SURF_PTR result = gcvNULL;

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal, gcvSURF_A8R8G8B8, gcvLINEAR, gcv2D_TSC_DISABLE,
        480, 480, &result));

    // create source surface
    for (i = 0; i < 8; ++i)
    {
        gcmONERROR(InitSourceSurface(t2d, frameNo*8+i));
    }

    for (i = 0; i < 8; i++)
    {
        gcsRECT srect;

        srect.left   = 0;
        srect.top    = 0;
        srect.right  = result->width;
        srect.bottom = result->height;

        gcmONERROR(gco2D_SetCurrentSourceIndex(egn2D, i));

        gcmONERROR(gco2D_SetClipping(egn2D, &srect));

        gcmONERROR(gco2D_SetGenericSource(
            egn2D,
            t2d->surf[i]->address, t2d->surf[i]->validAddressNum,
            t2d->surf[i]->stride, t2d->surf[i]->validStrideNum,
            t2d->surf[i]->tiling, t2d->surf[i]->format,
            sRots[frameNo % gcmCOUNTOF(sRots)],
            t2d->surf[i]->aWidth, t2d->surf[i]->aHeight));

        gcmONERROR(gco2D_SetSourceTileStatus(
            egn2D,
            t2d->surf[i]->tileStatusConfig,
            t2d->surf[i]->format,
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

        gcmONERROR(gco2D_SetROP(egn2D, 0xCC, 0xCC));

        if (!i)
        {
            gcmONERROR(gco2D_DisableAlphaBlend(egn2D));
        }
        else if (t2d->surf[i]->format == gcvSURF_NV12 ||
                 t2d->surf[i]->format == gcvSURF_UYVY ||
                 t2d->surf[i]->format == gcvSURF_YUY2 ||
                 t2d->surf[i]->format == gcvSURF_R5G6B5 ||
                 t2d->surf[i]->format == gcvSURF_X8R8G8B8 ||
                 t2d->surf[i]->format == gcvSURF_P010)
        {
            gcmONERROR(gco2D_EnableAlphaBlend(egn2D,
                        128, 128,
                        gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
                        gcvSURF_GLOBAL_ALPHA_ON, gcvSURF_GLOBAL_ALPHA_ON,
                        gcvSURF_BLEND_STRAIGHT, gcvSURF_BLEND_STRAIGHT,
                        gcvSURF_COLOR_STRAIGHT, gcvSURF_COLOR_STRAIGHT));
        }
        else
        {
            gcmONERROR(gco2D_EnableAlphaBlend(egn2D,
                        0, 0,
                        gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
                        gcvSURF_GLOBAL_ALPHA_OFF, gcvSURF_GLOBAL_ALPHA_OFF,
                        gcvSURF_BLEND_ONE, gcvSURF_BLEND_INVERSED,
                        gcvSURF_COLOR_STRAIGHT, gcvSURF_COLOR_STRAIGHT));
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

        gcmONERROR(gco2D_SetTargetRect(egn2D, &srect));
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
        0,
        result->tileStatusAddress
        ));

    gcmONERROR(gco2D_MultiSourceBlit(egn2D, 0xFF, gcvNULL, 0));

    gcmONERROR(gco2D_DisableAlphaBlend(egn2D));

    gcmONERROR(gco2D_SetBitBlitMirror(egn2D, gcvFALSE, gcvFALSE));

    gcmONERROR(gco2D_Flush(egn2D));

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
    gcvFEATURE_2D_MULTI_SOURCE_BLT_EX2,
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

    gcmONERROR(gco2D_SetStateU32(t2d->runtime->engine2d, gcv2D_STATE_XRGB_ENABLE, gcvTRUE));

    t2d->base.render      = (PGalRender)Render;
    t2d->base.destroy     = (PGalDestroy)Destroy;
    t2d->base.frameCount  = 8;
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
