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
 *  Feature:    full Rotation(flip) Cliping Alphablending Dither FilterBlit
 *  API:   gco2D_SetTargetEx  gco2D_SetClipping  gco2D_EnableAlphaBlendAdvanced  gco2D_EnableDither
 *  Check:
*/
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFilterBlit021\n" \
"Operation: combine features together, including full rotation(flip), alphablend, dither and clipping on hardware with ANDROID_ONLY feature.\n" \
"2D API: gco2D_FilterBlit gco2D_SetTargetEx  gco2D_SetClipping  gco2D_EnableAlphaBlendAdvanced  gco2D_EnableDither\n" \
"Src: Size        [400x400]\n"\
"     Rect        [0,0,400,400 / 0,0,400/2,400/2 / 0,0,400/3,400/3 / 0,0,400/4,400/4]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Dst: Size        [400x400]\n"\
"     Rect        [change according to frmaeNo]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: Type   [SolidBrush]\n"\
"       Format [ARGB8888]\n"\
"       Offset [0]\n" \
"KernelSize: [1/3/5]\n" \
"Alphablend: [enable]\n" \
"HW feature dependency: ";

typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;

    // dest surface
    gcoSURF        dstSurf;
    gctUINT32      dstPhysAddr;
    gctPOINTER     dstVirtAddr;
    gceSURF_FORMAT dstFormat;
    gctUINT32      dstWidth;
    gctUINT32      dstHeight;
    gctUINT32      dstAlignedWidth;
    gctUINT32      dstAlignedHeight;
    gctINT         dstStride;
    gcsRECT        dstRect;
    gctUINT8       dstAlpha;

    // source surface
    gcoSURF        srcSurf;
    gctUINT32      srcPhysAddr;
    gctPOINTER     srcVirtAddr;
    gceSURF_FORMAT srcFormat;
    gctUINT32      srcWidth;
    gctUINT32      srcHeight;
    gctUINT32      srcAlignedWidth;
    gctUINT32      srcAlignedHeight;
    gctINT         srcStride;
    gctUINT8       srcAlpha;
    gctPOINTER     srcULgcAddr;
    gctUINT32      srcUPhyAddr;
    gctINT         srcUStride;
    gctPOINTER     srcVLgcAddr;
    gctUINT32      srcVPhyAddr;
    gctINT         srcVStride;

    // temp surface
    gcoSURF        tmpSurf;
    gctUINT32      tmpPhyAddr;
    gctPOINTER     tmpVirtAddr;
    gceSURF_FORMAT tmpFormat;
    gctUINT32      tmpWidth;
    gctUINT32      tmpHeight;
    gctUINT32      tmpAlignedWidth;
    gctUINT32      tmpAlignedHeight;
    gctINT         tmpStride;
    gcsRECT        tmpRect;

    gcoSURF        tmpSurf1;
} Test2D;

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gctUINT8 horKernel, verKernel;
    gcsRECT srcRect, clipRect, tmpRect, destSubRect;
    gco2D egn2D  = t2d->runtime->engine2d;
    gceSTATUS status = gcvSTATUS_OK;
    gceSURF_ROTATION srcRot, dstRot;
    gctUINT32 tmpW, tmpH;

    gcmONERROR(gco2D_SetClipping(egn2D, &(t2d->tmpRect)));

    gcmONERROR(gcoSURF_Blit(t2d->tmpSurf1, t2d->tmpSurf, 1, &(t2d->tmpRect), &(t2d->tmpRect), gcvNULL,
        0xCC, 0xCC, gcvSURF_OPAQUE, 0, gcvNULL, gcvSURF_UNPACKED));

    // rect parameter change

    tmpW = gcmMIN(t2d->tmpWidth,t2d->srcWidth);
    tmpH = gcmMIN(t2d->tmpHeight,t2d->srcHeight);
    switch(frameNo%3)
    {
        case 0:
            srcRect.left   = 0;
            srcRect.top    = 0;
            srcRect.right  = t2d->srcWidth;
            srcRect.bottom = t2d->srcHeight;

            tmpRect.left   = 0;
            tmpRect.top    = 0;
            tmpRect.right  = t2d->tmpWidth;
            tmpRect.bottom = t2d->tmpHeight;

            clipRect.left   = tmpW/(frameNo+10);
            clipRect.top    = tmpH/(frameNo+10);
            clipRect.right  = tmpW;
            clipRect.bottom = tmpH;
            break;

        case 1:
            srcRect.left   = 0;
            srcRect.top    = 0;
            srcRect.right  = t2d->srcWidth/2;
            srcRect.bottom = t2d->srcHeight/2;

            tmpRect.left   = 0;
            tmpRect.top    = t2d->tmpHeight/(frameNo+1);
            tmpRect.right  = t2d->tmpWidth;
            tmpRect.bottom = t2d->tmpHeight;

            clipRect.left   = 0;
            clipRect.top    = tmpH/(frameNo+2);
            clipRect.right  = tmpW;
            clipRect.bottom = tmpH;
            break;

        case 2:
            srcRect.left   = 0;
            srcRect.top    = 0;
            srcRect.right  = t2d->srcWidth/3;
            srcRect.bottom = t2d->srcHeight/3;

            tmpRect.left   = t2d->tmpWidth/(frameNo+1);
            tmpRect.top    = 0;
            tmpRect.right  = t2d->tmpWidth;
            tmpRect.bottom = t2d->tmpHeight;

            clipRect.left   = tmpW/(frameNo);
            clipRect.top    = 0;
            clipRect.right  = tmpW;
            clipRect.bottom = tmpH;
            break;

        case 3:
            srcRect.left   = 0;
            srcRect.top    = 0;
            srcRect.right  = t2d->srcWidth/4;
            srcRect.bottom = t2d->srcHeight/4;

            tmpRect.left   = t2d->tmpWidth/(frameNo+1);
            tmpRect.top    = t2d->tmpHeight/(frameNo+1);
            tmpRect.right  = t2d->tmpWidth;
            tmpRect.bottom = t2d->tmpHeight;

            clipRect.left   = 0;
            clipRect.top    = 0;
            clipRect.right  = tmpW - tmpW/(frameNo+1);
            clipRect.bottom = tmpH - tmpH/(frameNo+1);
            break;

        case 4:
            srcRect.left   = 0;
            srcRect.top    = 0;
            srcRect.right  = t2d->srcWidth;
            srcRect.bottom = t2d->srcHeight;

            tmpRect.left   = 0;
            tmpRect.top    = 0;
            tmpRect.right  = t2d->tmpWidth - t2d->tmpWidth/(frameNo+1);
            tmpRect.bottom = t2d->tmpHeight;

            clipRect.left   = 0;
            clipRect.top    = tmpH/(frameNo+10);
            clipRect.right  = tmpW - tmpW/(frameNo+1);
            clipRect.bottom = tmpH;
            break;

        default:
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    destSubRect.left   = gcmMAX(tmpRect.left,clipRect.left) - tmpRect.left;
    destSubRect.top    = gcmMAX(tmpRect.top,clipRect.top) - tmpRect.top;
    destSubRect.right  = destSubRect.left + gcmMIN((clipRect.right - clipRect.left),(tmpRect.right  - tmpRect.left));
    destSubRect.bottom = destSubRect.top + gcmMIN((clipRect.bottom - clipRect.top),(tmpRect.bottom  - tmpRect.top));

    // rotation parameter change
    switch (frameNo)
    {
        case 0:
            srcRot = gcvSURF_0_DEGREE;
            dstRot = gcvSURF_0_DEGREE;
            horKernel = 1;
            verKernel = 1;
            break;

        case 1:
            srcRot = gcvSURF_0_DEGREE;
            dstRot = gcvSURF_0_DEGREE;
            horKernel = 3;
            verKernel = 3;
            break;

        case 2:
            srcRot = gcvSURF_0_DEGREE;
            dstRot = gcvSURF_0_DEGREE;
            horKernel = 5;
            verKernel = 5;
            break;

        case 3:
            srcRot = gcvSURF_0_DEGREE;
            dstRot = gcvSURF_90_DEGREE;
            horKernel = 3;
            verKernel = 3;
            break;

        case 4:
            srcRot = gcvSURF_0_DEGREE;
            dstRot = gcvSURF_90_DEGREE;
            horKernel = 5;
            verKernel = 5;
            break;

        case 5:
            srcRot = gcvSURF_0_DEGREE;
            dstRot = gcvSURF_180_DEGREE;
            horKernel = 3;
            verKernel = 3;
            break;

        case 6:
            srcRot = gcvSURF_0_DEGREE;
            dstRot = gcvSURF_180_DEGREE;
            horKernel = 5;
            verKernel = 5;
            break;

        case 7:
            srcRot = gcvSURF_0_DEGREE;
            dstRot = gcvSURF_270_DEGREE;
            horKernel = 3;
            verKernel = 3;
            break;

        case 8:
            srcRot = gcvSURF_0_DEGREE;
            dstRot = gcvSURF_270_DEGREE;
            horKernel = 5;
            verKernel = 5;
            break;

        case 9:
            srcRot = gcvSURF_0_DEGREE;
            dstRot = gcvSURF_FLIP_X;
            horKernel = 1;
            verKernel = 1;
            break;

        case 10:
            srcRot = gcvSURF_0_DEGREE;
            dstRot = gcvSURF_FLIP_X;
            horKernel = 3;
            verKernel = 3;
            break;

        case 11:
            srcRot = gcvSURF_0_DEGREE;
            dstRot = gcvSURF_FLIP_X;
            horKernel = 5;
            verKernel = 5;
            break;

        case 12:
            srcRot = gcvSURF_0_DEGREE;
            dstRot = gcvSURF_FLIP_Y;
            horKernel = 1;
            verKernel = 1;
            break;

        case 13:
            srcRot = gcvSURF_0_DEGREE;
            dstRot = gcvSURF_FLIP_Y;
            horKernel = 3;
            verKernel = 3;
            break;

        case 14:
            srcRot = gcvSURF_0_DEGREE;
            dstRot = gcvSURF_FLIP_Y;
            horKernel = 5;
            verKernel = 5;
            break;

        default:
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    if (gcoHAL_IsFeatureAvailable(t2d->runtime->hal, gcvFEATURE_ANDROID_ONLY) == gcvFALSE)
    {
        // draw src rect
        gcmONERROR(gco2D_SetTargetEx(egn2D,
                                     t2d->srcPhysAddr,
                                     t2d->srcStride,
                                     srcRot,
                                     t2d->srcAlignedWidth,
                                     t2d->srcAlignedHeight));

        gcmONERROR(gco2D_SetClipping(egn2D, &srcRect));

        gcmONERROR(gco2D_LoadSolidBrush(egn2D, t2d->srcFormat, gcvTRUE,
                    COLOR_ARGB8(0xFF, 0, 0, 0), 0));

        gcmONERROR(gco2D_Blit(egn2D, 1, &srcRect, 0xFA, 0xFA, t2d->srcFormat));

        gcmONERROR(gco2D_LoadSolidBrush(egn2D, t2d->srcFormat, gcvTRUE,
            COLOR_ARGB8(t2d->srcAlpha, 0xFF, 0xFF, 0xFF), 0));

        gcmONERROR(gco2D_Blit(egn2D, 1, &srcRect, 0xA0, 0xA0, t2d->srcFormat));

        //draw tmp rect
        gcmONERROR(gco2D_SetTargetEx(egn2D,
                                     t2d->tmpPhyAddr,
                                     t2d->tmpStride,
                                     dstRot,
                                     t2d->tmpAlignedWidth,
                                     t2d->tmpAlignedHeight));

        gcmONERROR(gco2D_SetClipping(egn2D, &tmpRect));

        gcmONERROR(gco2D_LoadSolidBrush(egn2D, t2d->tmpFormat, gcvTRUE,
                    COLOR_ARGB8(0xFF, 0, 0, 0), 0));

        gcmONERROR(gco2D_Blit(egn2D, 1, &tmpRect, 0xFA, 0xFA, t2d->tmpFormat));

        gcmONERROR(gco2D_LoadSolidBrush(egn2D, t2d->tmpFormat, gcvTRUE,
            COLOR_ARGB8(t2d->dstAlpha, 0xFF, 0xFF, 0xFF), 0));

        gcmONERROR(gco2D_Blit(egn2D, 1, &tmpRect, 0xA0, 0xA0, t2d->tmpFormat));
    }
    else
    {
        gcmONERROR(gco2D_Flush(egn2D));
        gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

        // draw src rect
        GalFillAlphaBySW(
            t2d->srcVirtAddr,
            t2d->srcAlignedWidth,
            t2d->srcAlignedHeight,
            t2d->srcStride,
            srcRot,
            &srcRect,
            &srcRect,
            t2d->srcAlpha,
            0xFF);

        //draw tmp rect
        GalFillAlphaBySW(
            t2d->tmpVirtAddr,
            t2d->tmpAlignedWidth,
            t2d->tmpAlignedHeight,
            t2d->tmpStride,
            dstRot,
            &tmpRect,
            &tmpRect,
            t2d->dstAlpha,
            0xFF);
    }

    //blend tmp and src
    gcmONERROR(gco2D_SetKernelSize(egn2D, horKernel, verKernel));

    gcmONERROR(gco2D_SetClipping(egn2D, &clipRect));

    gcmONERROR(gco2D_EnableAlphaBlendAdvanced(egn2D,
                    gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
                    gcvSURF_GLOBAL_ALPHA_OFF, gcvSURF_GLOBAL_ALPHA_OFF,
                    gcvSURF_BLEND_STRAIGHT, gcvSURF_BLEND_STRAIGHT));

    gcmONERROR(gco2D_EnableDither(egn2D, gcvTRUE));

    gcmONERROR(gco2D_FilterBlitEx(egn2D,
            t2d->srcPhysAddr, t2d->srcStride,
            t2d->srcUPhyAddr, t2d->srcUStride,
            t2d->srcVPhyAddr, t2d->srcVStride,
            t2d->srcFormat, srcRot, t2d->srcWidth, t2d->srcHeight, &srcRect,
            t2d->tmpPhyAddr, t2d->tmpStride, t2d->tmpFormat, dstRot, t2d->tmpWidth, t2d->tmpHeight, &tmpRect,
            &destSubRect));

    gcmONERROR(gco2D_Flush(egn2D));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    // disable alphablend
    gcmONERROR(gco2D_DisableAlphaBlend(egn2D));

    gcmONERROR(gco2D_EnableDither(egn2D, gcvFALSE));

    // blit tmp to dst
    gcmONERROR(Gal2DCleanSurface(t2d->runtime->hal, t2d->dstSurf, COLOR_ARGB8(0xFF, 0x00, 0x00, 0x00)));

    gcmONERROR(gco2D_SetColorSourceEx(egn2D,
                                      t2d->tmpPhyAddr,
                                      t2d->tmpStride,
                                      t2d->tmpFormat,
                                      gcvSURF_0_DEGREE,
                                      t2d->tmpWidth,
                                      t2d->tmpHeight,
                                      gcvFALSE,
                                      gcvSURF_OPAQUE,
                                      0));

    gcmONERROR(gco2D_SetSource(egn2D, &(t2d->tmpRect)));

    gcmONERROR(gco2D_SetTargetEx(egn2D,
                                 t2d->dstPhysAddr,
                                 t2d->dstStride,
                                 gcvSURF_0_DEGREE,
                                 t2d->dstAlignedWidth,
                                 t2d->dstAlignedHeight));

    gcmONERROR(gco2D_SetClipping(egn2D, gcvNULL));

    gcmONERROR(gco2D_SetStretchRectFactors(egn2D, &(t2d->tmpRect), &(t2d->dstRect)));

    gcmONERROR(gco2D_StretchBlit(egn2D, 1, &(t2d->dstRect), 0xCC, 0xCC, t2d->dstFormat));

    gcmONERROR(gco2D_Flush(egn2D));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    t2d->dstAlpha -= 3;

    t2d->srcAlpha += 3;

    return gcvTRUE;

OnError:
    GalOutput(GalOutputType_Error | GalOutputType_Console,
        "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));
    return gcvFALSE;
}

static void CDECL Destroy(Test2D *t2d)
{
    gceSTATUS status = gcvSTATUS_OK;
    // unlock dest surface as necessary
    if (t2d->dstSurf && t2d->dstVirtAddr) {
        if (gcmIS_ERROR(gcoSURF_Unlock(t2d->dstSurf, t2d->dstVirtAddr)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock dstSurf failed:%s\n", GalStatusString(status));
        }
        t2d->dstVirtAddr = gcvNULL;
    }

    // destroy source surface
    if (t2d->srcSurf)
    {
        if (t2d->srcVirtAddr) {
            if (gcmIS_ERROR(gcoSURF_Unlock(t2d->srcSurf, t2d->srcVirtAddr)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock srcSurf failed:%s\n", GalStatusString(status));
            }
            t2d->srcVirtAddr = gcvNULL;
        }
        if (gcmIS_ERROR(gcoSURF_Destroy(t2d->srcSurf)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Surf failed:%s\n", GalStatusString(status));
        }
        t2d->srcSurf = gcvNULL;
    }

    // destroy temp surface
    if (t2d->tmpSurf)
    {
        if (t2d->tmpVirtAddr) {
            if (gcmIS_ERROR(gcoSURF_Unlock(t2d->tmpSurf, t2d->tmpVirtAddr)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock tmpSurf failed:%s\n", GalStatusString(status));
            }
            t2d->tmpVirtAddr = gcvNULL;
        }
        if (gcmIS_ERROR(gcoSURF_Destroy(t2d->tmpSurf)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy tmpSurf failed:%s\n", GalStatusString(status));
        }
        t2d->tmpSurf = gcvNULL;
    }

    t2d->tmpSurf1 = gcvNULL;

    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_2D_ONE_PASS_FILTER,
    gcvFEATURE_SCALER,
    gcvFEATURE_2D_FILTERBLIT_PLUS_ALPHABLEND,
    gcvFEATURE_2D_DITHER,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    char *tmpBmpfile = "resource/source.bmp";
    char *srcBmpfile = "resource/background.bmp";
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

    memset(t2d, 0, sizeof(Test2D));

    // dst surface
    t2d->runtime   = runtime;
    t2d->dstSurf   = runtime->target;
    t2d->dstFormat = runtime->format;
    t2d->dstRect.left = 0;
    t2d->dstRect.top = 0;
    t2d->dstRect.right = runtime->width;
    t2d->dstRect.bottom = runtime->height;

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf,
                              &t2d->dstPhysAddr,
                              &t2d->dstVirtAddr));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstAlignedWidth,
                                        &t2d->dstAlignedHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_GetSize(t2d->dstSurf,
                                 &t2d->dstWidth,
                                 &t2d->dstHeight,
                                 gcvNULL));

    gcmONERROR(gcoSURF_GetFormat(t2d->dstSurf,
                                   gcvNULL,
                                   &t2d->dstFormat));

    t2d->srcAlpha  = 0x80;
    t2d->dstAlpha  = 0x80;

    // create tmp surface
    t2d->tmpSurf1 = GalLoadDIB2Surface(t2d->runtime->hal, tmpBmpfile);
    if (t2d->tmpSurf1 == NULL)
    {
        GalOutput(GalOutputType_Error, "can not load %s\n", tmpBmpfile);
        gcmONERROR(gcvSTATUS_NOT_FOUND);
    }

    gcmONERROR(gcoSURF_GetSize(t2d->tmpSurf1,
                                 &t2d->tmpWidth,
                                 &t2d->tmpHeight,
                                 gcvNULL));

    gcmONERROR(gcoSURF_GetFormat(t2d->tmpSurf1,
                                   gcvNULL,
                                   &t2d->tmpFormat));

    gcmONERROR(gcoSURF_Construct(t2d->runtime->hal,
                               t2d->tmpWidth,
                               t2d->tmpHeight,
                               1,
                               gcvSURF_BITMAP,
                               t2d->tmpFormat,
                               gcvPOOL_DEFAULT,
                               &t2d->tmpSurf));

    gcmONERROR(gcoSURF_Lock(t2d->tmpSurf,
                            &t2d->tmpPhyAddr,
                            &t2d->tmpVirtAddr));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->tmpSurf,
                                        &t2d->tmpAlignedWidth,
                                        &t2d->tmpAlignedHeight,
                                        &t2d->tmpStride));

    t2d->tmpRect.left = 0;
    t2d->tmpRect.top = 0;
    t2d->tmpRect.right = t2d->tmpWidth;
    t2d->tmpRect.bottom = t2d->tmpHeight;

    // create source surface
    t2d->srcULgcAddr = 0;
    t2d->srcUPhyAddr = 0;
    t2d->srcUStride = 0;
    t2d->srcVLgcAddr = 0;
    t2d->srcVPhyAddr = 0;
    t2d->srcVStride = 0;

    t2d->srcSurf = GalLoadDIB2Surface(t2d->runtime->hal, srcBmpfile);
    if (t2d->srcSurf == NULL)
    {
        GalOutput(GalOutputType_Error, "can not load %s\n", srcBmpfile);
        gcmONERROR(gcvSTATUS_NOT_FOUND);
    }

    gcmONERROR(gcoSURF_Lock(t2d->srcSurf,
                              &t2d->srcPhysAddr,
                              &t2d->srcVirtAddr));

    gcmONERROR(gcoSURF_GetSize(t2d->srcSurf,
                                 &t2d->srcWidth,
                                 &t2d->srcHeight,
                                 gcvNULL));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->srcSurf,
                                        &t2d->srcAlignedWidth,
                                        &t2d->srcAlignedHeight,
                                        &t2d->srcStride));

    gcmONERROR(gcoSURF_GetFormat(t2d->srcSurf,
                                   gcvNULL,
                                   &t2d->srcFormat));

    // Fill in the base info.
    t2d->base.render      = (PGalRender)Render;
    t2d->base.destroy     = (PGalDestroy)Destroy;
    t2d->base.description = s_CaseDescription;
    t2d->base.frameCount = 15;

    return gcvTRUE;

OnError:
    if (t2d->srcSurf)
    {
        if (t2d->srcVirtAddr)
        {
            gcmONERROR(gcoSURF_Unlock(t2d->srcSurf, t2d->srcVirtAddr));
            t2d->srcVirtAddr = gcvNULL;
        }

        if (gcmIS_ERROR(gcoSURF_Destroy(t2d->srcSurf)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Surf failed:%s\n", GalStatusString(status));
        }
        t2d->srcSurf = gcvNULL;
    }

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
