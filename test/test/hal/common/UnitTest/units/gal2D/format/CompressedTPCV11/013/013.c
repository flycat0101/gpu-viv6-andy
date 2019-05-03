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
"Case gal2DFormatCompressedTPC013\n" \
"Operation: multi-src blit/rotation with alpha: mixed TPCV11 compressed & uncompressed and argb & yuv to TPCV11 compressed and argb.\n" \
"2D API: gco2D_SetSourceTileStatus gco2D_SetTargetTileStatus gco2D_SetGenericSource gco2D_SetBitBlitMirror gco2D_MultiSourceBlit\n" \
"Src1: Size       [640x480]\n"\
"     Rect        [0,0,480,480]\n"\
"     Format      [INDEX8]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src2: Size       [640x480]\n"\
"     Rect        [0,0,480,480]\n"\
"     Format      [YUY2]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src3: Size       [480x480]\n"\
"     Rect        [0,0,480,480]\n"\
"     Format      [ABGR4444]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src4: Size       [480x480]\n"\
"     Rect        [0,0,480,480]\n"\
"     Format      [BGR565]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src5: Size       [480x480]\n"\
"     Rect        [0,0,480,480]\n"\
"     Format      [ABGR1555]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [TPCV11 compressed]\n" \
"Src6: Size       [480x480]\n"\
"     Rect        [0,0,480,480]\n"\
"     Format      [RG16]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [TPCV11 compressed]\n" \
"Src7: Size       [480x480]\n"\
"     Rect        [0,0,480,480]\n"\
"     Format      [R8]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [TPCV11 compressed]\n" \
"Src8: Size       [480x480]\n"\
"     Rect        [0,0,480,480]\n"\
"     Format      [ABGR8888]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [TPCV11 compressed]\n" \
"Dst: Size        [480x480]\n"\
"     Rect        [0,0,480,480]\n"\
"     Format      [ABGR8888/XBGR8888/ABGR4444/BGR565/ABGR1555/RG16/R8]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [TPCV11 compressed]\n" \
"Brush: Type   [SolidBrush]\n"\
"       Format [ARGB8888]\n"\
"       Offset [0]\n" \
"Alphablend: [disable/enable]\n" \
"HW feature dependency: ";

#define INT2BOOL(i,d) ((i)%(d) == 0)?gcvFALSE:gcvTRUE

typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;

    gctUINT8        srcAlpha;
    gctUINT8        dstAlpha;
    //source surface
    T2D_SURF_PTR    surf[10];

    gctUINT     alphaValue;
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
    gcvSURF_X8R8G8B8,
    gcvSURF_A2R10G10B10,
    gcvSURF_NV12,
    gcvSURF_P010,
};

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status;
    gcsRECT Rect, clipRect, tmpRect;
    gco2D egn2D = t2d->runtime->engine2d;
    gctINT i;

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal, gcvSURF_A8R8G8B8, gcvLINEAR, gcv2D_TSC_DISABLE,
        480, 480, &t2d->surf[8]));

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        sFormat[frameNo % gcmCOUNTOF(sFormat)],
        gcvLINEAR,
        gcv2D_TSC_TPC_COMPRESSED_V11,
        480, 480, &t2d->surf[9]));

    gcmONERROR(gco2D_SetStateU32(egn2D, gcv2D_STATE_XRGB_ENABLE, gcvTRUE));

    gcmONERROR(gco2D_SetGenericTarget(
         egn2D,
         t2d->surf[9]->address,
         t2d->surf[9]->validAddressNum,
         t2d->surf[9]->stride,
         t2d->surf[9]->validStrideNum,
         t2d->surf[9]->tiling,
         t2d->surf[9]->format,
         t2d->surf[9]->rotation,
         t2d->surf[9]->width,
         t2d->surf[9]->height));

    gcmONERROR(gco2D_SetTargetTileStatus(
         egn2D,
         t2d->surf[9]->tileStatusConfig,
         t2d->surf[9]->format,
         gcvSURF_0_DEGREE,
         t2d->surf[9]->tileStatusAddress
         ));

    {
        // Need to clear TPC header buffer.
        tmpRect.left = tmpRect.top = 0;
        tmpRect.right = t2d->surf[9]->width;
        tmpRect.bottom = t2d->surf[9]->height;

        clipRect.left = clipRect.top = 0;
        clipRect.right = 6400;
        clipRect.bottom = 6400;

        gcmONERROR(gco2D_SetClipping(t2d->runtime->engine2d, &clipRect));

        gcmONERROR(gco2D_LoadSolidBrush(
            t2d->runtime->engine2d, t2d->surf[9]->format, gcvTRUE, COLOR_ARGB8(0, 0, 0, 0), 0));
        gcmONERROR(gco2D_Clear(
            t2d->runtime->engine2d, 1, &tmpRect, COLOR_ARGB8(0, 0, 0, 0), 0xF0, 0xF0, t2d->surf[9]->format));
        gcmONERROR(gcoHAL_Commit(gcvNULL, gcvTRUE));
    }

    Rect.left = 0;
    Rect.top  = 0;
    Rect.right  = t2d->surf[9]->width;
    Rect.bottom = t2d->surf[9]->height;

    for (i = 0; i < 8; i++)
    {
        gcsRECT srect;
        gceSURF_ROTATION srot;

        srect.left = srect.top = 0;
        srect.right = srect.bottom = 480;

        gcmONERROR(gco2D_SetCurrentSourceIndex(egn2D, i));

        srot = sRots[(frameNo + i) % 6];

        gcmONERROR(gco2D_SetBitBlitMirror(egn2D, INT2BOOL((frameNo + i), 2),INT2BOOL((frameNo + i), 4)));

        if (i == 0)
        {
            gcmONERROR(gco2D_DisableAlphaBlend(egn2D));
        }
        else
        {
            switch(t2d->surf[i]->format)
            {
                case gcvSURF_A8R8G8B8:
                case gcvSURF_A8B8G8R8:
                case gcvSURF_A2R10G10B10:
                    gcmONERROR(gco2D_SetPorterDuffBlending(
                        egn2D,
                        gcvPD_SRC_OVER));
                    break;

                case gcvSURF_NV12:
                case gcvSURF_YUY2:
                case gcvSURF_UYVY:
                default:
                    gcmONERROR(gco2D_SetSourceGlobalColorAdvanced(t2d->runtime->engine2d, 0x80 << 24));

                    gcmONERROR(gco2D_SetTargetGlobalColorAdvanced(t2d->runtime->engine2d, 0x80 << 24));

                    gcmONERROR(gco2D_EnableAlphaBlendAdvanced(egn2D,
                        gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
                        gcvSURF_GLOBAL_ALPHA_ON, gcvSURF_GLOBAL_ALPHA_ON,
                        gcvSURF_BLEND_STRAIGHT, gcvSURF_BLEND_STRAIGHT));
                    break;
            }

            if (t2d->surf[9]->format == gcvSURF_NV12 ||
                t2d->surf[9]->format == gcvSURF_P010)
            {
                srot = gcvSURF_0_DEGREE;
            }

            gcmONERROR(gco2D_SetBitBlitMirror(egn2D, gcvFALSE, gcvFALSE));
        }

        gcmONERROR(gco2D_SetGenericSource(
            egn2D,
            t2d->surf[i]->address, t2d->surf[i]->validAddressNum,
            t2d->surf[i]->stride, t2d->surf[i]->validStrideNum,
            t2d->surf[i]->tiling, t2d->surf[i]->format,
            srot,
            t2d->surf[i]->width, t2d->surf[i]->height));

        gcmONERROR(gco2D_SetSourceTileStatus(
            egn2D,
            t2d->surf[i]->tileStatusConfig,
            t2d->surf[i]->tileStatusFormat,
            t2d->surf[i]->tileStatusClear,
            t2d->surf[i]->tileStatusAddress
            ));

        gcmONERROR(gco2D_SetSource(egn2D, &srect));

        gcmONERROR(gco2D_SetClipping(egn2D, &Rect));

        gcmONERROR(gco2D_SetROP(egn2D, 0xCC, 0xCC));

        gcmONERROR(gco2D_SetTargetRect(egn2D, &Rect));
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
         t2d->surf[9]->width,
         t2d->surf[9]->height));

    gcmONERROR(gco2D_SetSourceTileStatus(
         egn2D,
         t2d->surf[9]->tileStatusConfig,
         t2d->surf[9]->format,
         t2d->surf[9]->tileStatusClear,
         t2d->surf[9]->tileStatusAddress
         ));

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
    gcmONERROR(gco2D_Blit(egn2D, 1, &Rect, 0xCC, 0xCC, t2d->surf[8]->format));

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
    gcvFEATURE_TPCV11_COMPRESSION,
    gcvFEATURE_2D_MULTI_SOURCE_BLT_EX2,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status;
    gctINT i;
    gctSTRING sourceARGBfile[] =
    {
         "resource/zero2_A2R10G10B10.bmp",
         "resource/zero2_YUY2_640X480_Linear.vimg",
         "resource/VV_Background.bmp",
         "resource/GoneFishing2.bmp",
         "resource/zero2_UYVY_640X480_Linear.vimg",
         "resource/android_720x1280_icons.bmp",
         "resource/background.bmp",
         "resource/Crew_NV21_1280x720_Linear.vimg",
    };

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
    t2d->alphaValue = 0x80;

    gcmONERROR(GalCreateTSurf(
         t2d->runtime->hal, gcvSURF_A8R8G8B8, gcvLINEAR, gcv2D_TSC_DISABLE,
         480, 480, &t2d->surf[8]));

    // create source surface
    for (i = 0; i < 8; ++i)
    {
        T2D_SURF_PTR srcImage;
        gcsRECT srect, rect, tmpRect, clipRect;
        gce2D_TILE_STATUS_CONFIG tileStatus = gcv2D_TSC_TPC_COMPRESSED_V11;

        gcmONERROR(GalLoadFileToTSurf(
            sourceARGBfile[i], &srcImage));

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

        if (i < 4)tileStatus = gcv2D_TSC_DISABLE;

        if(srcImage->format == gcvSURF_INDEX8 || srcImage->format == gcvSURF_UYVY || srcImage->format == gcvSURF_YUY2)
        {
            t2d->surf[i] = srcImage;
        }
        else
        {
            GalFillAlphaBySW(
                    t2d->surf[8]->logical[0],
                    t2d->surf[8]->aWidth,
                    t2d->surf[8]->aHeight,
                    t2d->surf[8]->stride[0],
                    gcvSURF_0_DEGREE,
                    &rect,
                    &rect,
                    t2d->alphaValue,
                    0x0);

             rect.left = 240 - (8 - i) * 30;
             rect.top  = 240 - (8 - i) * 30;
             rect.right  = 240 + (8 - i) * 30;
             rect.bottom = 240 + (8 - i) * 30;

             gcmONERROR(GalCreateTSurf(
                     t2d->runtime->hal,
                     (i < 4) ? srcImage->format : gcvSURF_A8R8G8B8,
                     gcvLINEAR,
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
                     t2d->surf[i]->width,
                     t2d->surf[i]->height));

             gcmONERROR(gco2D_SetTargetTileStatus(
                     t2d->runtime->engine2d,
                     t2d->surf[i]->tileStatusConfig,
                     t2d->surf[i]->format,
                     gcvSURF_0_DEGREE,
                     t2d->surf[i]->tileStatusAddress
                     ));

            if (t2d->surf[i]->tileStatusConfig != gcv2D_TSC_DISABLE)
            {
                // Need to clear TPC header buffer.
                tmpRect.left = tmpRect.top = 0;
                tmpRect.right = t2d->surf[i]->width;
                tmpRect.bottom = t2d->surf[i]->height;

                clipRect.left = clipRect.top = 0;
                clipRect.right = 6400;
                clipRect.bottom = 6400;

                gcmONERROR(gco2D_SetClipping(t2d->runtime->engine2d, &clipRect));

                gcmONERROR(gco2D_LoadSolidBrush(
                    t2d->runtime->engine2d, t2d->surf[i]->format, gcvTRUE, COLOR_ARGB8(0, 0, 0, 0), 0));
                gcmONERROR(gco2D_Clear(
                    t2d->runtime->engine2d, 1, &tmpRect, COLOR_ARGB8(0, 0, 0, 0), 0xF0, 0xF0, t2d->surf[i]->format));

                gcmONERROR(gcoHAL_Commit(gcvNULL, gcvTRUE));
            }

            srect.left = 0;
            srect.top  = 0;
            srect.right  = t2d->surf[i]->width;
            srect.bottom = t2d->surf[i]->height;

            gcmONERROR(gco2D_SetClipping(t2d->runtime->engine2d, &srect));

            gcmONERROR(gco2D_SetSource(t2d->runtime->engine2d, &srect));

            gcmONERROR(gco2D_Blit(t2d->runtime->engine2d, 1, &rect, 0xCC, 0xCC, t2d->surf[i]->format));

            gcmONERROR(gcoHAL_Commit(gcvNULL, gcvTRUE));

            gcmONERROR(GalDeleteTSurf(t2d->runtime->hal, srcImage));

            t2d->alphaValue =  (t2d->alphaValue + 0x30)% 0xFF;
        }
    }

    gcmONERROR(GalDeleteTSurf(t2d->runtime->hal, t2d->surf[8]));

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = 8;
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
