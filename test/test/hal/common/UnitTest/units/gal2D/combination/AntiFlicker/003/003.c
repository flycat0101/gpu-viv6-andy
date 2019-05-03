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
 *  Feature:    Filter Blit - user defined filter kernel.
 *  API:        gco2D_SetFilterType
 *                gco2D_SetUserFilterKernel
 *                gco2D_EnableUserFilterPasses
 *                gco2D_FilterBlit
 *  Check:
 */
#include <galUtil.h>

#define MIN(x, y) ((x < y) ? x : y)

static const char *sBitmapFile[] = {
    "resource/zero2_A1R5G5B5.bmp",
    "resource/zero2_ARGB4.bmp",
    "resource/zero2_ARGB8.bmp",
    "resource/zero2_YUY2_640X480_Linear.vimg",
    "resource/zero2_UYVY_640X480_Linear.vimg",
    "resource/Boston_YV12_640x480_Linear.vimg",
};

static gctCONST_STRING s_CaseDescription = \
"Case gal2DAntiFlicker003\n" \
"Operation: Test anti-flicker with three-tap low pass filter.\n" \
"2D API: gco2D_SetUserFilterKernel gco2D_EnableUserFilterPasses gco2D_FilterBlit\n" \
"Src: Size        [640x480]\n"\
"     Rect        [configurable]\n"\
"     Format      [ARGB1555/ARGB4444/ARGB8888/YUY2/UYVY/YV12]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Dst: Size        [640x480]\n"\
"     Rect        [configurable]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: [None]\n"\
"KernelSzie: [user defined]\n"\
"Alphablend: [enable]\n" \
"HW feature dependency: ";

typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;

    // destination surface
    gcoSURF            dstSurf;
    gceSURF_FORMAT    dstFormat;
    gctUINT            dstWidth;
    gctUINT            dstHeight;
    gctINT            dstStride;
    gctUINT32        dstPhyAddr;
    gctPOINTER        dstLgcAddr;

    // destination temp surface
    gcoSURF            dstTempSurf;
    gceSURF_FORMAT    dstTempFormat;
    gctUINT            dstTempWidth;
    gctUINT            dstTempHeight;
    gctINT            dstTempStride;
    gctUINT32        dstTempPhyAddr;
    gctPOINTER        dstTempLgcAddr;

    //source surface
    gcoSURF            srcSurf;
    gceSURF_FORMAT    srcFormat;
    gctUINT            srcWidth;
    gctUINT            srcHeight;
    gctINT            srcStride;
    gctUINT32        srcPhyAddr;
    gctPOINTER        srcLgcAddr;
    gctPOINTER        srcULgcAddr;
    gctUINT32        srcUPhyAddr;
    gctINT            srcUStride;
    gctPOINTER        srcVLgcAddr;
    gctUINT32        srcVPhyAddr;
    gctINT            srcVStride;

} Test2D;

static gctBOOL CreateTempSurface(Test2D *t2d, gctUINT width, gctUINT height)
{
    gceSTATUS status = gcvTRUE;

    if (t2d->dstTempSurf != gcvNULL)
    {
        if (t2d->dstTempLgcAddr != gcvNULL)
        {
            if (gcmIS_ERROR(gcoSURF_Unlock(t2d->dstTempSurf, t2d->dstTempLgcAddr)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock dstSurf failed:%s\n", GalStatusString(status));
            }
            t2d->dstTempLgcAddr = gcvNULL;
        }

        if (gcmIS_ERROR(gcoSURF_Destroy(t2d->dstTempSurf)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock dstSurf failed:%s\n", GalStatusString(status));
        }
    }

    //dest Temp Surface
    gcmONERROR(gcoSURF_Construct(
        t2d->runtime->hal,
        width,
        height,
        1,
        gcvSURF_BITMAP,
        t2d->dstTempFormat,
        gcvPOOL_DEFAULT,
        &t2d->dstTempSurf));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstTempSurf,
                                        &t2d->dstTempWidth,
                                        &t2d->dstTempHeight,
                                        &t2d->dstTempStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstTempSurf, &t2d->dstTempPhyAddr, &t2d->dstTempLgcAddr));

    return gcvTRUE;

OnError:

    return gcvFALSE;
}

static gceSTATUS ReloadSourceSurface(Test2D *t2d, const char * sourcefile)
{
    gceSTATUS status;
    gctUINT32 address[3];
    gctPOINTER memory[3];
    gctSTRING pos = gcvNULL;

    // destroy source surface
    if (t2d->srcSurf != gcvNULL)
    {
        if (t2d->srcLgcAddr)
        {
            gcmONERROR(gcoSURF_Unlock(t2d->srcSurf, t2d->srcLgcAddr));
            t2d->srcLgcAddr = 0;
        }

        gcmONERROR(gcoSURF_Destroy(t2d->srcSurf));
        t2d->srcSurf = gcvNULL;
    }

    // create source surface
    gcmONERROR(GalStrSearch(sourcefile, ".bmp", &pos));
    if (pos)
    {
        t2d->srcSurf = GalLoadDIB2Surface(t2d->runtime->hal,
            sourcefile);
        if (t2d->srcSurf == NULL)
        {
            gcmONERROR(gcvSTATUS_NOT_FOUND);
        }
    }
    else
    {
        gcmONERROR(GalLoadVimgToSurface(
            sourcefile, &t2d->srcSurf));
    }

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->srcSurf,
                                        gcvNULL,
                                        gcvNULL,
                                        &t2d->srcStride));

    gcmONERROR(gcoSURF_GetSize(t2d->srcSurf,
                                &t2d->srcWidth,
                                &t2d->srcHeight,
                                gcvNULL));

    gcmONERROR(gcoSURF_GetFormat(t2d->srcSurf, gcvNULL, &t2d->srcFormat));

    gcmONERROR(gcoSURF_Lock(t2d->srcSurf, address, memory));

    t2d->srcPhyAddr  = address[0];
    t2d->srcLgcAddr  = memory[0];

    if (GalIsYUVFormat(t2d->srcFormat))
    {
        gcmONERROR(GalQueryUVStride(t2d->srcFormat, t2d->srcStride,
            &t2d->srcUStride, &t2d->srcVStride));

        t2d->srcUPhyAddr = address[1];
        t2d->srcULgcAddr = memory[1];

        t2d->srcVPhyAddr = address[2];
        t2d->srcVLgcAddr = memory[2];
    }

    return gcvSTATUS_OK;

OnError:

    return status;
}

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gcsRECT srcRect, dstRect, dstSubRect,dstTempRect;
    gco2D egn2D = t2d->runtime->engine2d;
    gceSTATUS status;
    gctUINT16 verKernelArray[9 * 17] = {0};
    gctINT i,j;

    srcRect.left = 0;
    srcRect.top = 0;
    srcRect.right = MIN(t2d->srcWidth, t2d->dstWidth);
    srcRect.bottom = MIN(t2d->srcHeight, t2d->dstHeight);

    dstRect.left = 0;
    dstRect.top = 0;
    dstRect.right = MIN(t2d->srcWidth, t2d->dstWidth);
    dstRect.bottom =  MIN(t2d->srcHeight, t2d->dstHeight);

    dstSubRect.left = 0;
    dstSubRect.top = 0;
    dstSubRect.right = dstRect.right - dstRect.left;
    dstSubRect.bottom = dstRect.bottom - dstRect.top;

    dstTempRect.left = 0;
    dstTempRect.top = 0;
    dstTempRect.right = srcRect.right;
    dstTempRect.bottom = srcRect.bottom;

    gcmONERROR(ReloadSourceSurface(t2d, sBitmapFile[frameNo]));

    gcmONERROR(CreateTempSurface(t2d, t2d->srcWidth, t2d->srcHeight));

    // set filter type
    gcmONERROR(gco2D_SetFilterType(egn2D, gcvFILTER_USER));

    for (i = 0; i < 17; i++)
    {
        for(j = 0; j < 9; j++)
        {
            switch(j)
            {
            case 3:
            case 5:
                verKernelArray[ i*9 + j] = (gctINT16) (0x1555);
                break;
            case 4:
                verKernelArray[ i*9 + j] = (gctINT16) (0x1556);
                break;
            default:
                verKernelArray[ i*9 + j] = 0;
                break;
            }
        }
    }

    gcmONERROR(gco2D_SetUserFilterKernel(egn2D,gcvFILTER_VER_PASS,verKernelArray));

    // disable the horizontal and enable vertical passes
    gcmONERROR(gco2D_EnableUserFilterPasses(egn2D,gcvFALSE,gcvTRUE));

    gcmONERROR(gco2D_FilterBlit(egn2D,
                    t2d->srcPhyAddr, t2d->srcStride,
                    t2d->srcUPhyAddr, t2d->srcUStride,
                    t2d->srcVPhyAddr, t2d->srcVStride,
                    t2d->srcFormat,
                    gcvSURF_0_DEGREE,
                    t2d->srcWidth,
                    &srcRect,
                    t2d->dstTempPhyAddr,
                    t2d->dstTempStride,
                    t2d->dstTempFormat,
                    gcvSURF_0_DEGREE,
                    t2d->dstTempWidth,
                    &dstTempRect,
                    &dstSubRect));

    //Blit to dest
    gcmONERROR(gco2D_SetColorSource(egn2D, t2d->dstTempPhyAddr, t2d->dstTempStride, t2d->dstTempFormat,
            gcvSURF_0_DEGREE, t2d->dstTempWidth, gcvFALSE, gcvSURF_OPAQUE, 0));

    gcmONERROR(gco2D_SetTarget(egn2D, t2d->dstPhyAddr, t2d->dstStride, gcvSURF_0_DEGREE, t2d->dstWidth));

    gcmONERROR(gco2D_Blit(egn2D, 1, &dstRect, 0xCC, 0xCC, t2d->dstFormat));

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

    if (t2d->dstTempSurf != gcvNULL)
    {
        if (t2d->dstTempLgcAddr != gcvNULL)
        {
            if (gcmIS_ERROR(gcoSURF_Unlock(t2d->dstTempSurf, t2d->dstTempLgcAddr)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock dstSurf failed:%s\n", GalStatusString(status));
            }
            t2d->dstTempLgcAddr = gcvNULL;
        }

        if (gcmIS_ERROR(gcoSURF_Destroy(t2d->dstTempSurf)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock dstSurf failed:%s\n", GalStatusString(status));
        }
    }

    // destroy source surface
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
    gcvFEATURE_SCALER,
    gcvFEATURE_YUV420_SCALER,
    gcvFEATURE_ANDROID_ONLY,
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

    t2d->runtime = runtime;

    t2d->dstSurf    = runtime->target;
    t2d->dstFormat = runtime->format;
    t2d->dstWidth = 0;
    t2d->dstHeight = 0;
    t2d->dstStride = 0;
    t2d->dstPhyAddr = 0;
    t2d->dstLgcAddr = 0;

    t2d->srcSurf    = gcvNULL;
    t2d->srcWidth = 0;
    t2d->srcHeight = 0;
    t2d->srcStride = 0;
    t2d->srcPhyAddr = 0;
    t2d->srcLgcAddr = 0;
    t2d->srcULgcAddr = 0;
    t2d->srcUPhyAddr = 0;
    t2d->srcUStride = 0;
    t2d->srcVLgcAddr = 0;
    t2d->srcVPhyAddr = 0;
    t2d->srcVStride = 0;
    t2d->srcFormat = gcvSURF_UNKNOWN;

    //dest Temp Surface
    t2d->dstTempSurf    = gcvNULL;
    t2d->dstTempFormat = runtime->format;

    gcmONERROR(ReloadSourceSurface(t2d, sBitmapFile[0]));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    t2d->base.frameCount = sizeof(sBitmapFile)/sizeof(sBitmapFile[0]);;

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
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

    if (!Init(t2d, runtime)) {
        free(t2d);
        return NULL;
    }

    return &t2d->base;
}
