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
 *  Feature:    full Rotation(flip) Cliping Blit
 *  API:   gco2D_SetTargetEx  gco2D_SetClipping
 *  Check:
 */
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DStretchBlit013\n" \
"Operation: Test StretchBlit with full rotation(flip) and clipping on hardware with ANDROID_ONLY feature.\n" \
"2D API: gco2D_StretchBlit gco2D_SetTargetEx gco2D_SetClipping\n" \
"Src: Size        [640x480]\n"\
"     Rect        [change according to frameNo]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Dst: Size        [640x480]\n"\
"     Rect        [change according to frameNo]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: [None]\n"\
"Alphablend: [disable]\n" \
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

} Test2D;

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gcsRECT srcRect, clipRect, tmpRect;
    gco2D egn2D  = t2d->runtime->engine2d;
    gceSTATUS status = gcvSTATUS_OK;
    gceSURF_ROTATION srcRot, dstRot;
    gctUINT32 clipSize = gcmMIN(t2d->dstWidth,t2d->srcWidth);

    // rect parameter change
    if((frameNo%3) == 0)
    {
        srcRect.left   = 0;
        srcRect.top    = 0;
        srcRect.right  = t2d->srcWidth * 3 / 4;
        srcRect.bottom = t2d->srcHeight * 3 / 4;

        tmpRect.left   = t2d->tmpWidth / 8;
        tmpRect.top    = t2d->tmpHeight / 8;
        tmpRect.right  = tmpRect.left + t2d->tmpWidth / 2;
        tmpRect.bottom = tmpRect.top + t2d->tmpHeight / 2;
    }
    else if((frameNo%3) == 1)
    {
        srcRect.left   = 0;
        srcRect.top    = 0;
        srcRect.right  = t2d->srcWidth / 2;
        srcRect.bottom = t2d->srcHeight / 2;

        tmpRect.left   = t2d->tmpWidth / 8;
        tmpRect.top    = t2d->tmpHeight / 8;
        tmpRect.right  = tmpRect.left + t2d->tmpWidth * 3 / 4;
        tmpRect.bottom = tmpRect.top + t2d->tmpHeight * 3 / 4;
    }
    else if((frameNo%3)== 2)
    {
        srcRect.left   = 0;
        srcRect.top    = 0;
        srcRect.right  = t2d->srcWidth / 2;
        srcRect.bottom = t2d->srcHeight / 2;

        tmpRect.left   = t2d->tmpWidth / 4;
        tmpRect.top    = t2d->tmpHeight / 4;
        tmpRect.right  = tmpRect.left + t2d->tmpWidth / 2;
        tmpRect.bottom = tmpRect.top + t2d->tmpHeight / 2;
    }

    if((frameNo%6) == 0)
    {
        clipRect.left   = 0;
        clipRect.top    = 0;
        clipRect.right  = clipRect.left + t2d->tmpWidth / 2;
        clipRect.bottom = clipRect.top + t2d->tmpHeight / 2;
    }
    else if((frameNo%6)== 1)
    {
        clipRect.left   = t2d->tmpWidth / 2;
        clipRect.top    = 0;
        clipRect.right  = clipRect.left + t2d->tmpWidth / 2;
        clipRect.bottom = clipRect.top + t2d->tmpHeight / 2;
    }
    else if((frameNo%6)== 2)
    {
        clipRect.left   = 0;
        clipRect.top    = t2d->tmpHeight / 2;
        clipRect.right  = clipRect.left + t2d->tmpWidth / 2;
        clipRect.bottom = clipRect.top + t2d->tmpHeight / 2;
    }
    else if((frameNo%6)== 3)
    {
        clipRect.left   = t2d->tmpWidth / 2;
        clipRect.top    = t2d->tmpHeight / 2;
        clipRect.right  = clipRect.left + t2d->tmpWidth / 2;
        clipRect.bottom = clipRect.top + t2d->tmpHeight / 2;
    }
    else if((frameNo%6)== 4)
    {
        clipRect.left   = t2d->tmpWidth / 3;
        clipRect.top    = t2d->tmpHeight / 3;
        clipRect.right  = clipRect.left + t2d->tmpWidth / 3;
        clipRect.bottom = clipRect.top + t2d->tmpHeight / 3;
    }
    else if((frameNo%6)== 5)
    {
        clipRect.left   = 0;
        clipRect.top    = 0;
        clipRect.right  = clipRect.left + t2d->tmpWidth;
        clipRect.bottom = clipRect.top + t2d->tmpHeight;
    }

    // rotation parameter change
    switch (frameNo)
    {
        case 0:
            srcRot = gcvSURF_0_DEGREE;
            dstRot = gcvSURF_0_DEGREE;
            break;

        case 1:
            srcRot = gcvSURF_90_DEGREE;
            dstRot = gcvSURF_0_DEGREE;
            break;

        case 2:
            srcRot = gcvSURF_0_DEGREE;
            dstRot = gcvSURF_90_DEGREE;
            break;

        case 3:
            srcRot = gcvSURF_90_DEGREE;
            dstRot = gcvSURF_90_DEGREE;
            break;

        case 4:
            srcRot = gcvSURF_180_DEGREE;
            dstRot = gcvSURF_0_DEGREE;
            break;

        case 5:
            srcRot = gcvSURF_270_DEGREE;
            dstRot = gcvSURF_0_DEGREE;
            break;

        case 6:
            srcRot = gcvSURF_180_DEGREE;
            dstRot = gcvSURF_90_DEGREE;
            break;

        case 7:
            srcRot = gcvSURF_270_DEGREE;
            dstRot = gcvSURF_90_DEGREE;
            break;

        case 8:
            srcRot = gcvSURF_0_DEGREE;
            dstRot = gcvSURF_180_DEGREE;
            break;

        case 9:
            srcRot = gcvSURF_90_DEGREE;
            dstRot = gcvSURF_180_DEGREE;
            break;

        case 10:
            srcRot = gcvSURF_180_DEGREE;
            dstRot = gcvSURF_180_DEGREE;
            break;

        case 11:
            srcRot = gcvSURF_270_DEGREE;
            dstRot = gcvSURF_180_DEGREE;
            break;

        case 12:
            srcRot = gcvSURF_0_DEGREE;
            dstRot = gcvSURF_270_DEGREE;
            break;

        case 13:
            srcRot = gcvSURF_90_DEGREE;
            dstRot = gcvSURF_270_DEGREE;
            break;

        case 14:
            srcRot = gcvSURF_180_DEGREE;
            dstRot = gcvSURF_270_DEGREE;
            break;

        case 15:
            srcRot = gcvSURF_270_DEGREE;
            dstRot = gcvSURF_270_DEGREE;
            break;

        case 16:
            srcRot = gcvSURF_0_DEGREE;
            dstRot = gcvSURF_FLIP_X;
            break;

        case 17:
            srcRot = gcvSURF_0_DEGREE;
            dstRot = gcvSURF_FLIP_Y;
               break;

        case 18:
            srcRot = gcvSURF_FLIP_X;
            dstRot = gcvSURF_0_DEGREE;
            break;

        case 19:
            srcRot = gcvSURF_FLIP_Y;
            dstRot = gcvSURF_0_DEGREE;
            break;

        case 20:
            srcRot = gcvSURF_90_DEGREE;
            dstRot = gcvSURF_FLIP_X;
            break;

        case 21:
            srcRot = gcvSURF_90_DEGREE;
            dstRot = gcvSURF_FLIP_Y;
               break;

        case 22:
            srcRot = gcvSURF_FLIP_X;
            dstRot = gcvSURF_90_DEGREE;
            break;

        case 23:
            srcRot = gcvSURF_FLIP_Y;
            dstRot = gcvSURF_90_DEGREE;
            break;

        case 24:
            srcRot = gcvSURF_180_DEGREE;
            dstRot = gcvSURF_FLIP_X;
            break;

        case 25:
            srcRot = gcvSURF_180_DEGREE;
            dstRot = gcvSURF_FLIP_Y;
               break;

        case 26:
            srcRot = gcvSURF_FLIP_X;
            dstRot = gcvSURF_180_DEGREE;
            break;

        case 27:
            srcRot = gcvSURF_FLIP_Y;
            dstRot = gcvSURF_180_DEGREE;
            break;

        case 28:
            srcRot = gcvSURF_270_DEGREE;
            dstRot = gcvSURF_FLIP_X;
            break;

        case 29:
            srcRot = gcvSURF_270_DEGREE;
            dstRot = gcvSURF_FLIP_Y;
               break;

        case 30:
            srcRot = gcvSURF_FLIP_X;
            dstRot = gcvSURF_270_DEGREE;
            break;

        case 31:
            srcRot = gcvSURF_FLIP_Y;
            dstRot = gcvSURF_270_DEGREE;
            break;

        case 32:
            srcRot = gcvSURF_FLIP_X;
            dstRot = gcvSURF_FLIP_X;
            break;

        case 33:
            srcRot = gcvSURF_FLIP_X;
            dstRot = gcvSURF_FLIP_Y;
               break;

        case 34:
            srcRot = gcvSURF_FLIP_Y;
            dstRot = gcvSURF_FLIP_X;
            break;

        case 35:
            srcRot = gcvSURF_FLIP_Y;
            dstRot = gcvSURF_FLIP_Y;
            break;

        default:
            return gcvFALSE;
    }

    //clean tmp rect
    gcmONERROR(Gal2DCleanSurface(t2d->runtime->hal, t2d->tmpSurf, COLOR_ARGB8(0xFF, 0x00, 0xFF, 0x00)));

    //draw tmp surf
    gcmONERROR(gco2D_SetTargetEx(egn2D,
                                 t2d->tmpPhyAddr,
                                 t2d->tmpStride,
                                 dstRot,
                                 t2d->tmpAlignedWidth,
                                 t2d->tmpAlignedHeight));

    gcmONERROR(gco2D_SetColorSourceEx(egn2D,
                                      t2d->srcPhysAddr,
                                      t2d->srcStride,
                                      t2d->srcFormat,
                                      srcRot,
                                      t2d->srcWidth,
                                      t2d->srcHeight,
                                      gcvFALSE,
                                      gcvSURF_OPAQUE,
                                      0));

    gcmONERROR(gco2D_SetSource(egn2D, &srcRect));

    gcmONERROR(gco2D_SetClipping(egn2D, &clipRect));

    gcmONERROR(gco2D_SetStretchRectFactors(egn2D, &srcRect, &tmpRect));

    gcmONERROR(gco2D_StretchBlit(egn2D, 1, &tmpRect, 0xCC, 0xCC, t2d->tmpFormat));

    gcmONERROR(gco2D_Flush(egn2D));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

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
    if (t2d->dstSurf && t2d->dstVirtAddr)
    {
        if (gcmIS_ERROR(gcoSURF_Unlock(t2d->dstSurf, t2d->dstVirtAddr)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock dstSurf failed:%s\n", GalStatusString(status));
        }
        t2d->dstVirtAddr = gcvNULL;
    }

    // destroy source surface
    if (t2d->srcSurf)
    {
        if (t2d->srcVirtAddr)
        {
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
        if (t2d->tmpVirtAddr)
        {
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

    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_2D_BITBLIT_FULLROTATION,
    gcvFEATURE_2D_ONE_PASS_FILTER,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status = gcvSTATUS_OK;
    const char *sourcefile = "resource/zero2_ARGB8.bmp";
    gctUINT32      tmpSrcWidth;
    gctUINT32      tmpSrcHeight;
    gcoSURF        tmpSrcSurf;
    gctUINT32      tmpSrcPhyAddr;
    gctPOINTER     tmpSrcVirtAddr = gcvNULL;
    gcsRECT srcRect, tmpSrcRect;

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

    //dst surf
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

    // create source surface
    tmpSrcSurf = GalLoadDIB2Surface(t2d->runtime->hal, sourcefile);

    if (tmpSrcSurf == NULL)
    {
        gcmONERROR(gcvSTATUS_NOT_FOUND);
    }

    gcmONERROR(gcoSURF_GetSize(tmpSrcSurf,
                                &tmpSrcWidth,
                                &tmpSrcHeight,
                                gcvNULL));

    gcmONERROR(gcoSURF_Lock(tmpSrcSurf, &tmpSrcPhyAddr, tmpSrcVirtAddr));

    t2d->srcWidth  = 400;
    t2d->srcHeight = 400;
    t2d->srcFormat = gcvSURF_A8R8G8B8;

    gcmONERROR(gcoSURF_Construct(t2d->runtime->hal,
                                      t2d->srcWidth,
                                      t2d->srcHeight,
                                      1,
                                      gcvSURF_BITMAP,
                                      t2d->srcFormat,
                                      gcvPOOL_DEFAULT,
                                      &t2d->srcSurf));

    gcmONERROR(gcoSURF_GetSize(t2d->srcSurf,
                                 &t2d->srcWidth,
                                 &t2d->srcHeight,
                                 gcvNULL));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->srcSurf,
                                        &t2d->srcAlignedWidth,
                                        &t2d->srcAlignedHeight,
                                        &t2d->srcStride));

    gcmONERROR(gcoSURF_Lock(t2d->srcSurf,
                              &t2d->srcPhysAddr,
                              &t2d->srcVirtAddr));

    tmpSrcRect.left   = 0;
    tmpSrcRect.top    = 0;
    tmpSrcRect.right  = tmpSrcWidth;
    tmpSrcRect.bottom = tmpSrcHeight;

    srcRect.left      = 0;
    srcRect.top       = 0;
    srcRect.right     = t2d->srcWidth;
    srcRect.bottom    = t2d->srcHeight;

    gcmONERROR(gco2D_SetClipping(t2d->runtime->engine2d, &srcRect));

    gcmONERROR(gco2D_SetKernelSize(t2d->runtime->engine2d, 5, 5));

    gcmONERROR(gcoSURF_FilterBlit(tmpSrcSurf,
                                    t2d->srcSurf,
                                    &tmpSrcRect,
                                    &srcRect,
                                    &srcRect));

    gcmONERROR(gco2D_Flush(t2d->runtime->engine2d));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    // destroy tmp source surface
    if (tmpSrcSurf != gcvNULL)
    {
        if (tmpSrcVirtAddr)
        {
            gcmONERROR(gcoSURF_Unlock(tmpSrcSurf, tmpSrcVirtAddr));
            tmpSrcVirtAddr = 0;
        }

        gcmONERROR(gcoSURF_Destroy(tmpSrcSurf));
        tmpSrcSurf = gcvNULL;
    }

    // create temp surface
    t2d->tmpFormat  = gcvSURF_A8R8G8B8;

    gcmONERROR(gcoSURF_Construct(t2d->runtime->hal,
                               t2d->srcWidth,
                               t2d->srcHeight,
                               1,
                               gcvSURF_BITMAP,
                               t2d->tmpFormat,
                               gcvPOOL_DEFAULT,
                               &t2d->tmpSurf));

    gcmONERROR(gcoSURF_Lock(t2d->tmpSurf,
                            &t2d->tmpPhyAddr,
                            &t2d->tmpVirtAddr));

    gcmONERROR(gcoSURF_GetSize(t2d->tmpSurf,
                                 &t2d->tmpWidth,
                                 &t2d->tmpHeight,
                                 gcvNULL));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->tmpSurf,
                                        &t2d->tmpAlignedWidth,
                                        &t2d->tmpAlignedHeight,
                                        &t2d->tmpStride));

    t2d->tmpRect.left = 0;
    t2d->tmpRect.top = 0;
    t2d->tmpRect.right = t2d->tmpWidth;
    t2d->tmpRect.bottom = t2d->tmpHeight;

    // Fill in the base info.
    t2d->base.render      = (PGalRender)Render;
    t2d->base.destroy     = (PGalDestroy)Destroy;
    t2d->base.description = s_CaseDescription;
    t2d->base.frameCount = 36;

    return gcvTRUE;

OnError:
    GalOutput(GalOutputType_Error | GalOutputType_Console,
        "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));

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

    if (tmpSrcSurf != gcvNULL)
    {
        if (tmpSrcVirtAddr)
        {
            gcmONERROR(gcoSURF_Unlock(tmpSrcSurf, tmpSrcVirtAddr));
            tmpSrcVirtAddr = 0;
        }

        gcmONERROR(gcoSURF_Destroy(tmpSrcSurf));
        tmpSrcSurf = gcvNULL;
    }

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
