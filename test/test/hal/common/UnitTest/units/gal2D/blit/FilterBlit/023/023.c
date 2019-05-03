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
 *  Feature:    Filter Blit - check non-aligned YUV420 source
 *  API:        gco2D_FilterBlit
 *  Check:
*/
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFilterBlit023\n" \
"Operation: Check 8-aligned YUV420 source for the hardware supports mbf.\n" \
"2D API: gco2D_FilterBlit\n" \
"Src: Size        [88x88/104x104/120x120/136x136/152x152/168x168/184x184/216x216/232x232/264x264/400x400]\n"\
"     Rect        [same as size]\n"\
"     Format      [I420]\n"\
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
"KernelSize: [5]\n" \
"Alphablend: [disable]\n" \
"HW feature dependency: ";

static const char *sBitmapFile[] = {
    "resource/source_YUV420_136x136_Linear.vimg",
    "resource/source_YUV420_152x152_Linear.vimg",
    "resource/source_YUV420_168x168_Linear.vimg",
    "resource/source_YUV420_184x184_Linear.vimg",
    "resource/source_YUV420_216x216_Linear.vimg",
    "resource/source_YUV420_232x232_Linear.vimg",
    "resource/source_YUV420_264x264_Linear.vimg",
    "resource/source_YUV420_400x400_Linear.vimg",
};

typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;

    // destination surface
    gcoSURF         dstSurf;
    gceSURF_FORMAT  dstFormat;
    gctUINT         dstWidth;
    gctUINT         dstHeight;
    gctINT          dstStride;
    gctUINT32       dstPhyAddr;
    gctPOINTER      dstLgcAddr;

    //source surface
    T2D_SURF_PTR    srcTSurf;
    gcoSURF         srcSurf;
    gceSURF_FORMAT  srcFormat;
    gctUINT         srcWidth;
    gctUINT         srcHeight;
    gctINT          srcStride;
    gctUINT32       srcPhyAddr;
    gctPOINTER      srcLgcAddr;
    gctPOINTER      srcULgcAddr;
    gctUINT32       srcUPhyAddr;
    gctINT          srcUStride;
    gctPOINTER      srcVLgcAddr;
    gctUINT32       srcVPhyAddr;
    gctINT          srcVStride;
} Test2D;

/* Support non-aligned source. */
gceSTATUS LoadYUV420(Test2D *t2d, const char *filename)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 width, imageWidth;
    gctUINT32 height, imageHeight;
    gctUINT32 address[3] = {0, 0, 0};
    gctPOINTER memory[3] = {0, 0, 0};
    gctUINT32 alignedStride;
    gctUINT32 sizeY, sizeUV;
    gctUINT32 srcUPhyAddrT, srcVPhyAddrT;

    gcmONERROR(GalLoadVimgToTSurf(filename, &t2d->srcTSurf));

    imageWidth  = t2d->srcTSurf->width;
    imageHeight = t2d->srcTSurf->height;

    if ((imageWidth == 0) || (imageHeight == 0) || (t2d->srcTSurf->tiling != gcvLINEAR))
    {
        gcmONERROR(gcvSTATUS_INVALID_DATA);
    }

    /* Check the type. */
    if (t2d->srcTSurf->format != gcvSURF_I420)
    {
        return gcvFALSE;
    }

    width  = imageWidth;
    height = imageHeight;

    /* make it big enough for address alignment. */
    height += (width + 192) / width;

    gcmONERROR(gcoSURF_Construct(t2d->runtime->hal, width, height, 1, gcvSURF_BITMAP,
                gcvSURF_I420, gcvPOOL_SYSTEM, &t2d->srcSurf));

    gcmONERROR(gcoSURF_Lock(t2d->srcSurf, address, memory));
    gcmONERROR(gcoSURF_GetAlignedSize(
            t2d->srcSurf,gcvNULL, gcvNULL, &alignedStride));
    t2d->srcPhyAddr = gcmALIGN(address[0], 64);
    t2d->srcLgcAddr = (gctPOINTER)((gctSIZE_T)memory[0] + (t2d->srcPhyAddr - address[0]));
    t2d->srcWidth   = imageWidth;
    t2d->srcHeight  = imageHeight;
    t2d->srcStride  = alignedStride;
    t2d->srcFormat  = gcvSURF_I420;
    gcmASSERT(!(imageWidth & 1));
    t2d->srcVStride = t2d->srcUStride = t2d->srcStride >> 1;

    sizeY = t2d->srcStride * t2d->srcHeight;
    gcmASSERT(!(sizeY & 3));
    sizeUV = sizeY >> 2;

    /* Fill Y channel without aligning. */
    memcpy(t2d->srcLgcAddr, t2d->srcTSurf->logical[0], sizeY);

    /* Fill U channel without aligning. */
    srcUPhyAddrT = t2d->srcPhyAddr + sizeY;
    t2d->srcUPhyAddr = gcmALIGN(srcUPhyAddrT, 64);
    t2d->srcULgcAddr = (gctUINT8*)t2d->srcLgcAddr + sizeY;
    t2d->srcULgcAddr = (gctPOINTER)((gctSIZE_T)t2d->srcULgcAddr + (t2d->srcUPhyAddr - srcUPhyAddrT));
    memcpy(t2d->srcULgcAddr, t2d->srcTSurf->logical[1], sizeUV);

    /* Fill V channel without aligning. */
    srcVPhyAddrT = t2d->srcUPhyAddr + sizeUV;
    t2d->srcVPhyAddr = gcmALIGN(srcVPhyAddrT, 64);
    t2d->srcVLgcAddr = (gctUINT8*)t2d->srcULgcAddr + sizeUV;
    t2d->srcVLgcAddr = (gctPOINTER)((gctSIZE_T)t2d->srcVLgcAddr + (t2d->srcVPhyAddr - srcVPhyAddrT));
    memcpy(t2d->srcVLgcAddr, t2d->srcTSurf->logical[2], sizeUV);

OnError:
    return status;
}

static gctBOOL CDECL LoadSourceSurface(Test2D *t2d, const char * sourcefile)
{
    gceSTATUS status = gcvSTATUS_OK;

    // destroy source surface
    if (t2d->srcTSurf != gcvNULL)
    {
        GalDeleteTSurf(t2d->runtime->hal, t2d->srcTSurf);
        t2d->srcTSurf = gcvNULL;
    }

    if (t2d->srcSurf != gcvNULL)
    {
        if (t2d->srcLgcAddr)
        {
            if (gcmIS_ERROR(gcoSURF_Unlock(t2d->srcSurf, t2d->srcLgcAddr)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock srcSurf failed:%s\n", GalStatusString(status));
            }
            t2d->srcLgcAddr = 0;
        }

        if (gcmIS_ERROR(gcoSURF_Destroy(t2d->srcSurf)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Surf failed:%s\n", GalStatusString(status));
        }
        t2d->srcSurf = gcvNULL;
    }

    // create source surface
    if (gcmIS_ERROR(LoadYUV420(t2d, sourcefile)))
    {
        GalOutput(GalOutputType_Error, "can not load %s\n", sourcefile);
        return gcvFALSE;
    }

    return gcvTRUE;
}

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gctUINT8 horKernel = 5, verKernel = 5;
    gcsRECT srcRect;
    gco2D egn2D = t2d->runtime->engine2d;
    gceSTATUS status;
    gcsRECT dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};

    if (!sBitmapFile[frameNo] || !LoadSourceSurface(t2d, sBitmapFile[frameNo]))
    {
        return gcvFALSE;
    }

    srcRect.left = 0;
    srcRect.top = 0;
    srcRect.right = t2d->srcWidth;
    srcRect.bottom = t2d->srcHeight;

    // set clippint rect
    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    // set kernel size
    gcmONERROR(gco2D_SetKernelSize(egn2D, horKernel, verKernel));

    gcmASSERT(!(t2d->srcPhyAddr & 0x3F));
    gcmASSERT(!(t2d->srcUPhyAddr & 0x3F));
    gcmASSERT(!(t2d->srcVPhyAddr & 0x3F));

    gcmONERROR(gco2D_FilterBlit(egn2D,
            t2d->srcPhyAddr, t2d->srcStride,
            t2d->srcUPhyAddr, t2d->srcUStride,
            t2d->srcVPhyAddr, t2d->srcVStride,
            t2d->srcFormat, gcvSURF_0_DEGREE, t2d->srcWidth, &srcRect,
            t2d->dstPhyAddr, t2d->dstStride, t2d->dstFormat, gcvSURF_0_DEGREE, t2d->dstWidth, &dstRect,
            &dstRect));

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
    if ((t2d->dstSurf != gcvNULL) && (t2d->dstLgcAddr != gcvNULL))
    {
        if (gcmIS_ERROR(gcoSURF_Unlock(t2d->dstSurf, t2d->dstLgcAddr)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock desSurf failed:%s\n", GalStatusString(status));
        }
        t2d->dstLgcAddr = gcvNULL;
    }

    // destroy source surface
    if (t2d->srcTSurf != gcvNULL)
    {
        GalDeleteTSurf(t2d->runtime->hal, t2d->srcTSurf);
        t2d->srcTSurf = gcvNULL;
    }

    if (t2d->srcSurf != gcvNULL)
    {
        if (t2d->srcLgcAddr)
        {
            if (gcmIS_ERROR(gcoSURF_Unlock(t2d->srcSurf, t2d->srcLgcAddr)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock srcSurf failed:%s\n", GalStatusString(status));
            }
            t2d->srcLgcAddr = 0;
        }

        if (gcmIS_ERROR(gcoSURF_Destroy(t2d->srcSurf)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Surf failed:%s\n", GalStatusString(status));
        }
    }

    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_YUV420_SCALER,
    gcvFEATURE_2D_MULTI_SRC_BLT_BILINEAR_FILTER,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status;

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

    t2d->runtime     = runtime;

    t2d->dstSurf     = runtime->target;
    t2d->dstFormat   = runtime->format;
    t2d->dstWidth    = 0;
    t2d->dstHeight   = 0;
    t2d->dstStride   = 0;
    t2d->dstPhyAddr  = 0;
    t2d->dstLgcAddr  = 0;

    t2d->srcTSurf    = gcvNULL;
    t2d->srcSurf     = gcvNULL;
    t2d->srcWidth    = 0;
    t2d->srcHeight   = 0;
    t2d->srcStride   = 0;
    t2d->srcPhyAddr  = 0;
    t2d->srcLgcAddr  = 0;
    t2d->srcULgcAddr = 0;
    t2d->srcUPhyAddr = 0;
    t2d->srcUStride  = 0;
    t2d->srcVLgcAddr = 0;
    t2d->srcVPhyAddr = 0;
    t2d->srcVStride  = 0;
    t2d->srcFormat   = gcvSURF_UNKNOWN;

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.description = s_CaseDescription;
    t2d->base.frameCount = sizeof(sBitmapFile)/sizeof(sBitmapFile[0]);

    return gcvTRUE;

OnError:
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
