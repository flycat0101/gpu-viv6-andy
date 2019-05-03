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
 *  Feature:    Filter Blit - kernel size
 *  API:        gco2D_FilterBlit
 *  Check:
*/
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFormatYUV010\n" \
"Operation: Test Seperate U/V stride.\n" \
"2D API: gco2D_FilterBlit\n" \
"Src: Size        [640x480/1280x720]\n"\
"     Rect        [0,0,640,480 / 0,0,1280,720]\n"\
"     Format      [I420/YV12/NV12/NV16/NV21/NV61]\n"\
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
"KernelSize: [3]\n" \
"Alphablend: [disable]\n" \
"HW feature dependency: ";

static const char *sBitmapFile[] = {
    "resource/zero2_YUV420_640X480_Linear.vimg",
    "resource/Boston_YV12_640x480_Linear.vimg",
    "resource/Crew_NV12_1280x720_Linear.vimg",
    "resource/Crew_NV16_1280x720_Linear.vimg",
    "resource/Crew_NV21_1280x720_Linear.vimg",
    "resource/Crew_NV61_1280x720_Linear.vimg",
};

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

    gcoSURF            tmpSurf;
    gceSURF_FORMAT    tmpFormat;
    gctUINT            tmpWidth;
    gctUINT            tmpHeight;
    gctINT            tmpStride;
    gctUINT32        tmpPhyAddr;
    gctPOINTER        tmpLgcAddr;

    gcoSURF            tmp2Surf;
    gceSURF_FORMAT    tmp2Format;
    gctUINT            tmp2Width;
    gctUINT            tmp2Height;
    gctINT            tmp2Stride;
    gctUINT32        tmp2PhyAddr;
    gctPOINTER        tmp2LgcAddr;
} Test2D;

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
        gcmONERROR(GalLoadVimgToSurface(
                sourcefile, &t2d->srcSurf));

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

        gcmONERROR(GalQueryUVStride(t2d->srcFormat, t2d->srcStride,
                &t2d->srcUStride, &t2d->srcVStride));

        t2d->srcUPhyAddr = address[1];
        t2d->srcULgcAddr = memory[1];

        t2d->srcVPhyAddr = address[2];
        t2d->srcVLgcAddr = memory[2];

        switch (t2d->srcFormat)
        {
        case gcvSURF_YV12:
        case gcvSURF_I420:
            {
                gctUINT h;
                gctINT8 *dst = t2d->tmp2LgcAddr;
                gctINT8 *src = t2d->srcVLgcAddr;

                gcmASSERT(t2d->tmp2Stride > t2d->srcVStride);
                for (h = 0; h < (t2d->srcHeight / 2); h++)
                {
                    memcpy(dst, src, t2d->srcVStride);

                    dst += t2d->tmp2Stride;
                    src += t2d->srcVStride;
                }
            }

        case gcvSURF_NV12:
        case gcvSURF_NV21:
            {
                gctUINT h;
                gctINT8 *dst = t2d->tmpLgcAddr;
                gctINT8 *src = t2d->srcULgcAddr;

                gcmASSERT(t2d->tmpStride > t2d->srcUStride);

                if (gcoHAL_IsFeatureAvailable(t2d->runtime->hal, gcvFEATURE_TPCV11_COMPRESSION) ||
                    gcoHAL_IsFeatureAvailable(t2d->runtime->hal, gcvFEATURE_DEC400_COMPRESSION))
                {
                    gctUINT32 addressT;

                    t2d->tmpStride = gcmALIGN(t2d->tmpStride, 256);
                    addressT = gcmALIGN(t2d->tmpPhyAddr, 256);
                    t2d->tmpLgcAddr = GAL_POINTER_OFFSET(t2d->tmpLgcAddr, addressT - t2d->tmpPhyAddr);
                    t2d->tmpPhyAddr += addressT - t2d->tmpPhyAddr;

                    dst = t2d->tmpLgcAddr;
                }

                for (h = 0; h < (t2d->srcHeight / 2); h++)
                {
                    memcpy(dst, src, t2d->srcUStride);

                    dst += t2d->tmpStride;
                    src += t2d->srcUStride;
                }
            }
            break;

        case gcvSURF_NV16:
        case gcvSURF_NV61:
            {
                gctUINT h;
                gctINT8 *dst = t2d->tmpLgcAddr;
                gctINT8 *src = t2d->srcULgcAddr;

                gcmASSERT(t2d->tmpStride > t2d->srcUStride);
                for (h = 0; h < t2d->srcHeight; h++)
                {
                    memcpy(dst, src, t2d->srcUStride);

                    dst += t2d->tmpStride;
                    src += t2d->srcUStride;
                }
            }
            break;
        default:
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            break;
        }

        return gcvSTATUS_OK;

OnError:

        return status;
}

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status;
    gctUINT8 horKernel = 3, verKernel = 3;
    gcsRECT srcRect;
    gco2D egn2D = t2d->runtime->engine2d;
    gcsRECT dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};

    gcmONERROR(ReloadSourceSurface(t2d, sBitmapFile[frameNo]));

    srcRect.left = 0;
    srcRect.top = 0;
    srcRect.right = t2d->srcWidth;
    srcRect.bottom = t2d->srcHeight;

    // set clippint rect
    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    // set kernel size
    gcmONERROR(gco2D_SetKernelSize(egn2D, horKernel, verKernel));

    gcmONERROR(gco2D_FilterBlit(egn2D,
            t2d->srcPhyAddr, t2d->srcStride,
            t2d->tmpPhyAddr, t2d->tmpStride,
            t2d->tmp2PhyAddr, t2d->tmp2Stride,
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

    if (t2d->tmpSurf != gcvNULL)
    {
        if (t2d->tmpLgcAddr)
        {
            if (gcmIS_ERROR(gcoSURF_Unlock(t2d->tmpSurf, t2d->tmpLgcAddr)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock tmpSurf failed:%s\n", GalStatusString(status));
            }
            t2d->tmpLgcAddr = 0;
        }

        if (gcmIS_ERROR(gcoSURF_Destroy(t2d->tmpSurf)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy tmpSurf failed:%s\n", GalStatusString(status));
        }
    }

    if (t2d->tmp2Surf != gcvNULL)
    {
        if (t2d->tmp2LgcAddr)
        {
            if (gcmIS_ERROR(gcoSURF_Unlock(t2d->tmp2Surf, t2d->tmp2LgcAddr)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock tmp2Surf failed:%s\n", GalStatusString(status));
            }
            t2d->tmp2LgcAddr = 0;
        }

        if (gcmIS_ERROR(gcoSURF_Destroy(t2d->tmp2Surf)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy tmp2Surf failed:%s\n", GalStatusString(status));
        }
    }

    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_2D_YUV_SEPARATE_STRIDE,
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

    t2d->tmpSurf    = gcvNULL;
    t2d->tmpFormat = gcvSURF_R5G6B5;
    t2d->tmpWidth = 650;
    t2d->tmpHeight = 720;
    t2d->tmpStride = 0;
    t2d->tmpPhyAddr = 0;
    t2d->tmpLgcAddr = 0;

    t2d->tmp2Surf    = gcvNULL;
    t2d->tmp2Format = gcvSURF_R5G6B5;
    t2d->tmp2Width = 680;
    t2d->tmp2Height = 720;
    t2d->tmp2Stride = 0;
    t2d->tmp2PhyAddr = 0;
    t2d->tmp2LgcAddr = 0;

    gcmONERROR(gcoSURF_Construct(t2d->runtime->hal,
                               t2d->tmpWidth,
                               t2d->tmpHeight,
                               1,
                               gcvSURF_BITMAP,
                               t2d->tmpFormat,
                               gcvPOOL_DEFAULT,
                               &t2d->tmpSurf));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->tmpSurf,
                                        &t2d->tmpWidth,
                                        &t2d->tmpHeight,
                                        &t2d->tmpStride));

    gcmONERROR(gcoSURF_Lock(t2d->tmpSurf, &t2d->tmpPhyAddr, &t2d->tmpLgcAddr));

    gcmONERROR(gcoSURF_Construct(t2d->runtime->hal,
                               t2d->tmp2Width,
                               t2d->tmp2Height,
                               1,
                               gcvSURF_BITMAP,
                               t2d->tmp2Format,
                               gcvPOOL_DEFAULT,
                               &t2d->tmp2Surf));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->tmp2Surf,
                                        &t2d->tmp2Width,
                                        &t2d->tmp2Height,
                                        &t2d->tmp2Stride));

    gcmONERROR(gcoSURF_Lock(t2d->tmp2Surf, &t2d->tmp2PhyAddr, &t2d->tmp2LgcAddr));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = gcmCOUNTOF(sBitmapFile);
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
