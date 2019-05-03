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
 *  Feature:    10bit format input and output with blit
 *  API:        gco2D_Blit gco2D_SetGenericSource/gco2D_SetGenericTarget
 *  Check:
*/
#include <galUtil.h>

static const char *sBitmapFile[] = {
    "resource/zero2_A2B10G10R10.bmp",
    "resource/zero2_A2R10G10B10.bmp",
    "resource/zero2_R10G10B10A2.bmp",
    "resource/zero2_B10G10R10A2.bmp",
    "resource/zero2_NV12_10BIT.vimg",
    "resource/zero2_NV21_10BIT.vimg",
};

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFormat10Bit006\n" \
"Operation: Test bitblit for YUV10bit input\n" \
"2D API: gco2D_Blit gco2D_SetGenericSource/gco2D_SetGenericTarget\n" \
"Src: Size        [640x480]\n"\
"     Rect        [configurable]\n"\
"     Format      [ARGB2101010/ABGR2101010/RGBA1010102/RGBA1010102]\n"\
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

    // destination surface
    gcoSURF         dstSurf;
    gceSURF_FORMAT  dstFormat;
    gctUINT         dstWidth;
    gctUINT         dstHeight;
    gctINT          dstStride;
    gctUINT32       dstPhyAddr;
    gctPOINTER      dstLgcAddr;

    //source surface
    gcoSURF         srcSurf;
    gceSURF_FORMAT  srcFormat;
    gctUINT         srcWidth;
    gctUINT         srcHeight;
    gctINT          srcStride[3];
    gctINT          srcStrideNum;
    gctINT          srcAddressNum;
    gctUINT32       srcPhyAddr[3];
    gctPOINTER      srcLgcAddr[3];

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
        if (t2d->srcLgcAddr[0])
        {
            gcmONERROR(gcoSURF_Unlock(t2d->srcSurf, t2d->srcLgcAddr));
            t2d->srcLgcAddr[0] = gcvNULL;
        }

        gcmONERROR(gcoSURF_Destroy(t2d->srcSurf));
        t2d->srcSurf = gcvNULL;
    }

    // create source surface
    gcmONERROR(GalStrSearch(sourcefile, ".bmp", &pos));
    if (pos)
    {
        t2d->srcSurf = GalLoadDIB2Surface(t2d->runtime->hal, sourcefile);
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
                                        &t2d->srcWidth,
                                        &t2d->srcHeight,
                                        t2d->srcStride));

    gcmONERROR(gcoSURF_GetFormat(t2d->srcSurf, gcvNULL, &t2d->srcFormat));

    gcmONERROR(gcoSURF_Lock(t2d->srcSurf, address, memory));

    t2d->srcStrideNum = t2d->srcAddressNum = 1;

    if (GalIsYUVFormat(t2d->srcFormat))
    {
        gcmONERROR(GalQueryUVStride(t2d->srcFormat, t2d->srcStride[0],
                &t2d->srcStride[1], &t2d->srcStride[2]));

        switch (t2d->srcFormat)
        {
        case gcvSURF_YUY2:
        case gcvSURF_UYVY:
            t2d->srcStrideNum = t2d->srcAddressNum = 1;
            break;

        case gcvSURF_I420:
        case gcvSURF_YV12:
            t2d->srcStrideNum = t2d->srcAddressNum = 3;
            break;

        case gcvSURF_NV16:
        case gcvSURF_NV12:
        case gcvSURF_NV61:
        case gcvSURF_NV21:
            t2d->srcStrideNum = t2d->srcAddressNum = 2;
            break;

        case gcvSURF_NV16_10BIT:
        case gcvSURF_NV61_10BIT:
            {
                gctUINT32 addressT;

                t2d->srcStrideNum = t2d->srcAddressNum = 2;

                t2d->srcStride[0] = gcmALIGN_NP2(t2d->srcStride[0], 80);
                t2d->srcStride[1] = gcmALIGN_NP2(t2d->srcStride[1], 80);

                addressT = gcmALIGN_NP2(address[0], 80);
                memory[0] = GAL_POINTER_OFFSET(memory[0], addressT - address[0]);
                memory[1] = GAL_POINTER_OFFSET(memory[1], addressT - address[0]);
                address[1] += addressT - address[0];
                address[0] += addressT - address[0];
            }
            break;

        case gcvSURF_NV12_10BIT:
        case gcvSURF_NV21_10BIT:
            {
                gctUINT32 addressT, aligned;

                if (gcoHAL_IsFeatureAvailable(t2d->runtime->hal, gcvFEATURE_TPCV11_COMPRESSION) ||
                    gcoHAL_IsFeatureAvailable(t2d->runtime->hal, gcvFEATURE_DEC400_COMPRESSION))
                {
                    aligned = 320;
                }
                else
                {
                    aligned = 80;
                }

                t2d->srcStrideNum = t2d->srcAddressNum = 2;

                t2d->srcStride[0] = gcmALIGN_NP2(t2d->srcStride[0], aligned);
                t2d->srcStride[1] = gcmALIGN_NP2(t2d->srcStride[1], aligned);

                addressT = gcmALIGN_NP2(address[0], aligned);
                memory[0] = GAL_POINTER_OFFSET(memory[0], addressT - address[0]);
                memory[1] = GAL_POINTER_OFFSET(memory[1], addressT - address[0]);
                address[1] += addressT - address[0];
                address[0] += addressT - address[0];
            }
            break;

        default:
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        t2d->srcPhyAddr[1] = address[1];
        t2d->srcLgcAddr[1] = memory[1];

        t2d->srcPhyAddr[2] = address[2];
        t2d->srcLgcAddr[2] = memory[2];
    }

    t2d->srcPhyAddr[0]  = address[0];
    t2d->srcLgcAddr[0]  = memory[0];

    return gcvSTATUS_OK;

OnError:
    return status;
}

gceSURF_ROTATION sRotList[] =
{
    gcvSURF_0_DEGREE,
    gcvSURF_90_DEGREE,
    gcvSURF_180_DEGREE,
    gcvSURF_270_DEGREE,
    gcvSURF_FLIP_X,
    gcvSURF_FLIP_Y,
};

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsRECT srect, drect;
    gco2D egn2D = t2d->runtime->engine2d;
    gceSURF_ROTATION rot = sRotList[frameNo % gcmCOUNTOF(sRotList)];
    gctUINT32 horFactor, verFactor;
    gctBOOL hMirror, vMirror;

    gcmONERROR(ReloadSourceSurface(t2d, sBitmapFile[frameNo % gcmCOUNTOF(sBitmapFile)]));

    drect.left = 0;
    drect.top = 0;
    drect.right = t2d->dstWidth;
    drect.bottom = t2d->dstHeight;

    srect.left = 0;
    srect.top = 0;
    if (rot == gcvSURF_90_DEGREE || rot == gcvSURF_270_DEGREE)
    {
        srect.right = t2d->srcHeight;
        srect.bottom = t2d->srcWidth;
    }
    else
    {
        srect.right = t2d->srcWidth;
        srect.bottom = t2d->srcHeight;
    }

    gcmONERROR(gco2D_SetSource(egn2D, &srect));

    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        t2d->srcPhyAddr, t2d->srcAddressNum,
        t2d->srcStride, t2d->srcStrideNum,
        gcvLINEAR,
        t2d->srcFormat,
        rot,
        t2d->srcWidth,
        t2d->srcHeight));

    switch (frameNo % 4)
    {
        case 0:
            hMirror = vMirror = gcvFALSE;
            break;

        case 1:
            hMirror = gcvTRUE;
            vMirror = gcvFALSE;
            break;

        case 2:
            hMirror = gcvFALSE;
            vMirror = gcvTRUE;
            break;

        case 3:
            hMirror = vMirror = gcvTRUE;
            break;
    }

    gcmONERROR(gco2D_SetBitBlitMirror(egn2D, hMirror, vMirror));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        &t2d->dstPhyAddr, 1,
        &t2d->dstStride, 1,
        gcvLINEAR,
        t2d->dstFormat,
        gcvSURF_0_DEGREE,
        t2d->dstWidth,
        t2d->dstHeight));

    gcmONERROR(gco2D_SetClipping(egn2D, &drect));

    /* Calculate the stretch factors. */
    gcmONERROR(gco2D_CalcStretchFactor(egn2D, srect.right - srect.left,
            drect.right - drect.left, &horFactor));

    gcmONERROR(gco2D_CalcStretchFactor(egn2D, srect.bottom - srect.top,
            drect.bottom - drect.top, &verFactor));

    /* Program the stretch factors. */
    gcmONERROR(gco2D_SetStretchFactors(egn2D, horFactor, verFactor));

    gcmONERROR(gco2D_StretchBlit(egn2D, 1, &drect, 0xCC, 0xCC, t2d->dstFormat));

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
        if (t2d->srcLgcAddr[0])
        {
            if (gcmIS_ERROR(gcoSURF_Unlock(t2d->srcSurf, t2d->srcLgcAddr)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock srcSurf failed:%s\n", GalStatusString(status));
            }
            t2d->srcLgcAddr[0] = gcvNULL;
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
    gcvFEATURE_2D_10BIT_OUTPUT_LINEAR,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
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

    t2d->runtime = runtime;

    t2d->dstSurf = runtime->target;
    t2d->dstFormat = runtime->format;
    t2d->dstWidth = 0;
    t2d->dstHeight = 0;
    t2d->dstStride = 0;
    t2d->dstPhyAddr = 0;
    t2d->dstLgcAddr = 0;

    t2d->srcSurf    = gcvNULL;
    t2d->srcLgcAddr[0] = gcvNULL;
    t2d->srcFormat = gcvSURF_UNKNOWN;

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
