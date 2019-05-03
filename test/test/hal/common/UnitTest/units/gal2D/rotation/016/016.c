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
 *  Feature:    mirror vertically
 *  API:        gco2D_SetBitBlitMirror
 *  Check:
 */
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DRotation016\n" \
"Operation: Test mirror vertically on hardware with ANDROID_ONLY feature.\n" \
"2D API: gco2D_Blit gco2D_SetBitBlitMirror\n" \
"Src: Size        [configurable]\n"\
"     Rect        [configurable]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Dst: Size        [configurable]\n"\
"     Rect        [configurable]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: [None]\n"\
"Alphablend: [disable]\n" \
"HW feature dependency: ";

typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;

      // dest surface
    gcoSURF           dstSurf;
    gctUINT32       dstPhysAddr;
    gctPOINTER       dstVirtAddr;
    gceSURF_FORMAT dstFormat;
    gctUINT32       dstWidth;
    gctUINT32       dstHeight;
    gctUINT32       dstAlignedWidth;
    gctUINT32       dstAlignedHeight;
    gctINT           dstStride;

    // source surface
    gcoSURF           srcSurf;
    gctUINT32       srcPhysAddr;
    gctPOINTER       srcVirtAddr;
    gceSURF_FORMAT srcFormat;
    gctUINT32       srcWidth;
    gctUINT32       srcHeight;
    gctUINT32       srcAlignedWidth;
    gctUINT32       srcAlignedHeight;
    gctINT           srcStride;

    // tmp dest surface
    gcoSURF           tmpDstSurf;
    gctUINT32       tmpDstPhysAddr;
    gctPOINTER       tmpDstVirtAddr;
    gceSURF_FORMAT tmpDstFormat;
    gctUINT32       tmpDstWidth;
    gctUINT32       tmpDstHeight;
    gctUINT32       tmpDstAlignedWidth;
    gctUINT32       tmpDstAlignedHeight;
    gctINT           tmpDstStride;
} Test2D;


static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gcsRECT srcRect, dstRect, dstSubRect, clipRect, tmpDstRect;

    gco2D      egn2D  = t2d->runtime->engine2d;
    gceSTATUS status = gcvSTATUS_OK;

    // render to dest surface
    tmpDstRect.left   = 0;
    tmpDstRect.top    = 0;
    tmpDstRect.right  = t2d->tmpDstWidth;
    tmpDstRect.bottom = t2d->tmpDstHeight;

    dstRect.left       = 0;
    dstRect.top       = 0;
    dstRect.right       = t2d->dstWidth;
    dstRect.bottom       = t2d->dstHeight;

    dstSubRect.left   = 0;
    dstSubRect.top       = 0;
    dstSubRect.right  = t2d->dstWidth;
    dstSubRect.bottom = t2d->dstHeight;

    clipRect.left       = 0;
    clipRect.top       = 0;
    clipRect.right       = t2d->dstWidth;
    clipRect.bottom   = t2d->dstHeight;

    gcmONERROR(gco2D_SetClipping(egn2D, &clipRect));

    gcmONERROR(gco2D_SetKernelSize(egn2D, 5, 5));

    gcmONERROR(gcoSURF_FilterBlit(t2d->tmpDstSurf,
                                    t2d->dstSurf,
                                    &tmpDstRect,
                                    &dstRect,
                                    &dstSubRect));

    // blit with mirror horizontally
    srcRect.left   = 0;
    srcRect.top    = 0;
    srcRect.right  = t2d->srcWidth;
    srcRect.bottom = t2d->srcHeight;

    dstRect.left   = (t2d->dstWidth - t2d->srcWidth) / 2;
    dstRect.top    = (t2d->dstHeight - t2d->srcHeight) / 2;
    dstRect.right  = dstRect.left + t2d->srcWidth;
    dstRect.bottom = dstRect.top + t2d->srcHeight;

    // set the clipping according to the rotation
    clipRect.left    = 0;
    clipRect.top     = 0;
    clipRect.right     = t2d->dstWidth;
    clipRect.bottom = t2d->dstHeight;

    // start rendering sequence
    gcmONERROR(gco2D_SetColorSource(egn2D,
                                      t2d->srcPhysAddr,
                                      t2d->srcStride,
                                      t2d->srcFormat,
                                      gcvSURF_0_DEGREE,
                                      t2d->srcWidth,
                                      gcvFALSE,
                                      gcvSURF_OPAQUE,
                                      0));

    gcmONERROR(gco2D_SetSource(egn2D, &srcRect));

    gcmONERROR(gco2D_SetTarget(egn2D,
                                 t2d->dstPhysAddr,
                                 t2d->dstStride,
                                 gcvSURF_0_DEGREE,
                                 t2d->dstAlignedWidth));


    gcmONERROR(gco2D_SetClipping(egn2D, &clipRect));

    // enable vertical mirror
    gcmONERROR(gco2D_SetBitBlitMirror(egn2D, gcvFALSE, gcvTRUE));

    gcmONERROR(gco2D_Blit(egn2D,
                            1,
                            &dstRect,
                            0xCC,
                            0xCC,
                            t2d->dstFormat));

    // disable mirror
    gcmONERROR(gco2D_SetBitBlitMirror(egn2D, gcvFALSE, gcvFALSE));

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

    // destroy tmp dest surface
    if (t2d->tmpDstSurf)
    {
        if (t2d->tmpDstVirtAddr) {
            if (gcmIS_ERROR(gcoSURF_Unlock(t2d->tmpDstSurf, t2d->tmpDstVirtAddr)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock tmpSurf failed:%s\n", GalStatusString(status));
            }
            gcoHAL_Commit(t2d->runtime->hal, gcvFALSE);
            t2d->tmpDstVirtAddr = gcvNULL;
        }

        if (gcmIS_ERROR(gcoSURF_Destroy(t2d->tmpDstSurf)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Surf failed:%s\n", GalStatusString(status));
        }
        t2d->tmpDstSurf = gcvNULL;
    }

    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_2D_ONE_PASS_FILTER,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    char *srcBmpfile = "resource/rotate.bmp";
    char *dstBmpfile = "resource/VV_Background.bmp";

    gcsRECT srcRect, tmpSrcRect, srcSubRect;
    gcsRECT clipRect;

    gctUINT32 tmpSrcWidth, tmpSrcHeight;

    gcoSURF tmpSrcSurf = gcvNULL;
    gceSTATUS status   = gcvSTATUS_OK;

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

    t2d->runtime   = runtime;
    t2d->dstSurf   = runtime->target;
    t2d->dstFormat = runtime->format;

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
    t2d->srcWidth  = t2d->dstWidth / 2;
    t2d->srcHeight = t2d->dstHeight / 2;
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

       // render to source surface
    tmpSrcSurf = GalLoadDIB2Surface(t2d->runtime->hal, srcBmpfile);
    if (tmpSrcSurf == NULL)
    {
        GalOutput(GalOutputType_Error, "can not load %s\n", srcBmpfile);

        gcmONERROR(gcvSTATUS_NOT_FOUND);
    }

    gcmONERROR(gcoSURF_GetSize(tmpSrcSurf,
                                 &tmpSrcWidth,
                                 &tmpSrcHeight,
                                 gcvNULL));

    tmpSrcRect.left   = 0;
    tmpSrcRect.top    = 0;
    tmpSrcRect.right  = tmpSrcWidth;
    tmpSrcRect.bottom = tmpSrcHeight;

    srcRect.left       = 0;
    srcRect.top       = 0;
    srcRect.right       = t2d->srcWidth;
    srcRect.bottom       = t2d->srcHeight;

    srcSubRect.left   = 0;
    srcSubRect.top       = 0;
    srcSubRect.right  = t2d->srcWidth;
    srcSubRect.bottom = t2d->srcHeight;

    clipRect.left       = 0;
    clipRect.top       = 0;
    clipRect.right       = t2d->srcWidth;
    clipRect.bottom   = t2d->srcHeight;

    gcmONERROR(gco2D_SetClipping(t2d->runtime->engine2d, &clipRect));

    gcmONERROR(gco2D_SetKernelSize(t2d->runtime->engine2d, 5, 5));

    gcmONERROR(gcoSURF_FilterBlit(tmpSrcSurf,
                                    t2d->srcSurf,
                                    &tmpSrcRect,
                                    &srcRect,
                                    &srcSubRect));

    gcmONERROR(gcoSURF_Destroy(tmpSrcSurf));

    tmpSrcSurf = gcvNULL;

    // create tmp dest surface
    t2d->tmpDstSurf = GalLoadDIB2Surface(t2d->runtime->hal, dstBmpfile);
    if (t2d->tmpDstSurf == NULL)
    {
        GalOutput(GalOutputType_Error, "can not load %s\n", dstBmpfile);

        gcmONERROR(gcvSTATUS_NOT_FOUND);
    }

    gcmONERROR(gcoSURF_Lock(t2d->tmpDstSurf,
                              &t2d->tmpDstPhysAddr,
                              &t2d->tmpDstVirtAddr));

    gcmONERROR(gcoSURF_GetSize(t2d->tmpDstSurf,
                                 &t2d->tmpDstWidth,
                                 &t2d->tmpDstHeight,
                                 gcvNULL));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->tmpDstSurf,
                                        &t2d->tmpDstAlignedWidth,
                                        &t2d->tmpDstAlignedHeight,
                                        &t2d->tmpDstStride));

    gcmONERROR(gcoSURF_GetFormat(t2d->tmpDstSurf,
                                   gcvNULL,
                                   &t2d->tmpDstFormat));

    // Fill in the base info.
    t2d->base.render      = (PGalRender)Render;
    t2d->base.destroy     = (PGalDestroy)Destroy;
    t2d->base.frameCount  = 1;
    t2d->base.description = s_CaseDescription;

    return gcvTRUE;

OnError:
    if (gcmIS_ERROR(status)) {
                GalOutput(GalOutputType_Error | GalOutputType_Console,
                    "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));

        if (t2d->srcSurf) {
            if (t2d->srcVirtAddr) {
                if (gcmIS_ERROR(gcoSURF_Unlock(t2d->srcSurf, t2d->srcVirtAddr)))
                {
                    GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Surf failed:%s\n", GalStatusString(status));
                }
                t2d->srcVirtAddr = gcvNULL;
            }

            if (gcmIS_ERROR(gcoSURF_Destroy(t2d->srcSurf)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Surf failed:%s\n", GalStatusString(status));
            }
            t2d->srcSurf = gcvNULL;
        }

        if (tmpSrcSurf) {
            if (gcmIS_ERROR(gcoSURF_Destroy(tmpSrcSurf)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Surf failed:%s\n", GalStatusString(status));
            }
        }

        if (t2d->tmpDstSurf) {
            if (t2d->tmpDstVirtAddr) {
                if (gcmIS_ERROR(gcoSURF_Unlock(t2d->tmpDstSurf, t2d->tmpDstVirtAddr)))
                {
                    GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Surf failed:%s\n", GalStatusString(status));
                }
                t2d->tmpDstVirtAddr = gcvNULL;
            }

            if (gcmIS_ERROR(gcoSURF_Destroy(t2d->tmpDstSurf)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Surf failed:%s\n", GalStatusString(status));
            }
            t2d->tmpDstSurf = gcvNULL;
        }

        return gcvFALSE;
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
