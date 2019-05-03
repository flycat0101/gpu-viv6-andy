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
 *  Feature:
 *  API:        gco2D_MultiSourceBlit gco2D_SetGenericSource gco2D_SetGenericTarget
 *  Check:
*/
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription =
"Case gal2DBilinearFilterMultiSourceBlit001\n" \
"Operation: Bilinear Filter MultiSrcBlit stretch full rotation and flips.\n" \
"2D API: gco2D_MultiSourceBlit\n" \
"Src1: Size       [400x400]\n"\
"     Rect        [0,0,400,400]\n"\
"     Format      [I420]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src2: Size       [720x1280]\n"\
"     Rect        [0,0,720,1280]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src3: Size       [1920x1080]\n"\
"     Rect        [0,0,1920,1080]\n"\
"     Format      [UYVY]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src4: Size       [640x480]\n"\
"     Rect        [0,0,480,480]\n"\
"     Format      [YV12]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src5: Size       [640x480]\n"\
"     Rect        [0,0,640,480]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src6: Size       [1920x1080]\n"\
"     Rect        [0,0,480,480]\n"\
"     Format      [NV12]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src7: Size       [1280x720]\n"\
"     Rect        [0,0,1280,720]\n"\
"     Format      [NV61]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src8: Size       [640x480]\n"\
"     Rect        [0,0,640,480]\n"\
"     Format      [ARGB4444]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Dst: Size        [configurable]\n"\
"     Rect        [configurable]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: [None]\n"\
"Alphablend: [enable]\n" \
"HW feature dependency: ";

typedef struct _MultiSrc
{
    gcoSURF         srcSurf;
    gceSURF_FORMAT  srcFormat;
    gctUINT         srcWidth;
    gctUINT         srcHeight;
    gctINT          srcStride[3];
    gctINT          srcStrideNum;
    gctINT          srcAddressNum;
    gctUINT32       srcPhyAddr[3];
    gctPOINTER      srcLgcAddr[3];
} MultiSrc, *MultiSrcPTR;

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
    MultiSrc multiSrc[8];
} Test2D;

static gceSTATUS ReloadSourceSurface(Test2D *t2d, gctUINT SrcIndex, const char * sourcefile)
{
    gceSTATUS status;
    MultiSrcPTR curSrc = &t2d->multiSrc[SrcIndex % 8];
    gctUINT32 address[3];
    gctPOINTER memory[3];
    gctSTRING pos = gcvNULL;

    // destroy source surface
    if (curSrc->srcSurf != gcvNULL)
    {
        if (curSrc->srcLgcAddr[0])
        {
            gcmONERROR(gcoSURF_Unlock(curSrc->srcSurf, curSrc->srcLgcAddr));
            curSrc->srcLgcAddr[0] = gcvNULL;
        }

        gcmONERROR(gcoSURF_Destroy(curSrc->srcSurf));
        curSrc->srcSurf = gcvNULL;
    }

    // create source surface
    gcmONERROR(GalStrSearch(sourcefile, ".bmp", &pos));
    if (pos)
    {
        curSrc->srcSurf = GalLoadDIB2Surface(t2d->runtime->hal,
            sourcefile);
        if (curSrc->srcSurf == NULL)
        {
            gcmONERROR(gcvSTATUS_NOT_FOUND);
        }
    }
    else
    {
        gcmONERROR(GalLoadVimgToSurface(
            sourcefile, &curSrc->srcSurf));
    }

    gcmONERROR(gcoSURF_GetAlignedSize(curSrc->srcSurf,
                                        gcvNULL,
                                        gcvNULL,
                                        curSrc->srcStride));

    gcmONERROR(gcoSURF_GetSize(curSrc->srcSurf,
                                &curSrc->srcWidth,
                                &curSrc->srcHeight,
                                gcvNULL));

    gcmONERROR(gcoSURF_GetFormat(curSrc->srcSurf, gcvNULL, &curSrc->srcFormat));

    gcmONERROR(gcoSURF_Lock(curSrc->srcSurf, address, memory));

    curSrc->srcPhyAddr[0]  = address[0];
    curSrc->srcLgcAddr[0]  = memory[0];

    curSrc->srcStrideNum = curSrc->srcAddressNum = 1;

    if (GalIsYUVFormat(curSrc->srcFormat))
    {
        gcmONERROR(GalQueryUVStride(curSrc->srcFormat, curSrc->srcStride[0],
                &curSrc->srcStride[1], &curSrc->srcStride[2]));

        curSrc->srcPhyAddr[1] = address[1];
        curSrc->srcLgcAddr[1] = memory[1];

        curSrc->srcPhyAddr[2] = address[2];
        curSrc->srcLgcAddr[2] = memory[2];
        switch (curSrc->srcFormat)
        {
        case gcvSURF_YUY2:
        case gcvSURF_UYVY:
            curSrc->srcStrideNum = curSrc->srcAddressNum = 1;
            break;

        case gcvSURF_I420:
        case gcvSURF_YV12:
            curSrc->srcStrideNum = curSrc->srcAddressNum = 3;
            break;

        case gcvSURF_NV16:
        case gcvSURF_NV61:
        case gcvSURF_NV12:
        case gcvSURF_NV21:
            curSrc->srcStrideNum = curSrc->srcAddressNum = 2;
            break;

        default:
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }
    }

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
    gceSTATUS status;
    gco2D egn2D = t2d->runtime->engine2d;
    gctINT i;
    gceSURF_ROTATION drot = sRotList[frameNo % gcmCOUNTOF(sRotList)];
    gctINT w = t2d->dstWidth;
    gctINT h = t2d->dstHeight;
    gcsRECT srect, drect;
    gctBOOL hMirror, vMirror;

    if (drot == gcvSURF_90_DEGREE || drot == gcvSURF_270_DEGREE)
    {
        gctINT t = w;
        w = h;
        h = t;
    }

    drect.left   = 0;
    drect.top    = 0;
    drect.right  = drect.left + w;
    drect.bottom = drect.top + h;

    gcmONERROR(gco2D_SetClipping(egn2D, &drect));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        &t2d->dstPhyAddr, 1,
        &t2d->dstStride, 1,
        gcvLINEAR,
        t2d->dstFormat,
        drot,
        t2d->dstWidth,
        t2d->dstHeight));

    for (i = 0; i < 8; i++)
    {
        MultiSrcPTR curSrc = &t2d->multiSrc[(i + frameNo) % gcmCOUNTOF(t2d->multiSrc)];
        gceSURF_ROTATION srot = sRotList[(i + frameNo) % gcmCOUNTOF(sRotList)];

        if (srot == gcvSURF_90_DEGREE || srot == gcvSURF_270_DEGREE)
        {
            srect.left = srect.top = 0;
            srect.right = curSrc->srcHeight;
            srect.bottom = curSrc->srcWidth;
        }
        else
        {
            srect.left = srect.top = 0;
            srect.right = curSrc->srcWidth;
            srect.bottom = curSrc->srcHeight;
        }

        switch ((i+frameNo) % 4)
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

        gcmONERROR(gco2D_SetCurrentSourceIndex(egn2D, i));

        gcmONERROR(gco2D_SetBitBlitMirror(egn2D, hMirror, vMirror));

        if (!i)
        {
            gcmONERROR(gco2D_DisableAlphaBlend(egn2D));
        }
        else
        {
            switch(curSrc->srcFormat)
            {
                case gcvSURF_YUY2:
                case gcvSURF_UYVY:
                case gcvSURF_NV12:
                case gcvSURF_NV16:
                case gcvSURF_I420:
                case gcvSURF_YV12:
                        gcmONERROR(gco2D_SetSourceGlobalColorAdvanced(egn2D, 0x40 << 24));
                        gcmONERROR(gco2D_SetTargetGlobalColorAdvanced(egn2D, 0x40 << 24));

                        gcmONERROR(gco2D_EnableAlphaBlendAdvanced(egn2D,
                                        gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
                                        gcvSURF_GLOBAL_ALPHA_ON, gcvSURF_GLOBAL_ALPHA_ON,
                                        gcvSURF_BLEND_STRAIGHT, gcvSURF_BLEND_STRAIGHT));

                        break;

                default:
                        gcmONERROR(gco2D_SetPorterDuffBlending(
                                    egn2D,
                                    gcvPD_SRC_OVER));
                        break;
            }
        }

        gcmONERROR(gco2D_SetGenericSource(
            egn2D,
            curSrc->srcPhyAddr, curSrc->srcAddressNum,
            curSrc->srcStride, curSrc->srcStrideNum,
            gcvLINEAR,
            curSrc->srcFormat,
            srot,
            curSrc->srcWidth,
            curSrc->srcHeight));

        gcmONERROR(gco2D_SetSource(egn2D, &srect));
        gcmONERROR(gco2D_SetROP(egn2D, 0xCC, 0xCC));
    }

    gcmONERROR(gco2D_MultiSourceBlit(egn2D, 0xFF, &drect, 1));

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
    gctINT i;

    if ((t2d->dstSurf != gcvNULL) && (t2d->dstLgcAddr != gcvNULL))
    {
        if (gcmIS_ERROR(gcoSURF_Unlock(t2d->dstSurf, t2d->dstLgcAddr)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock desSurf failed:%s\n", GalStatusString(status));
        }
        t2d->dstLgcAddr = gcvNULL;
    }

    // destroy source surface
    for (i = 0; i < 8; i++)
    {
        MultiSrcPTR curSrc = &t2d->multiSrc[i];

        if (curSrc->srcSurf != gcvNULL)
        {
            if (curSrc->srcLgcAddr[0])
            {
                status = gcoSURF_Unlock(curSrc->srcSurf, curSrc->srcLgcAddr);
                if (gcmIS_ERROR(status))
                {
                    GalOutput(GalOutputType_Error | GalOutputType_Console,
                        "Unlock SrcSurface[%d] failed:%s(%d)\n", i, GalStatusString(status), status);
                }
                curSrc->srcLgcAddr[0] = gcvNULL;
            }

            status = gcoSURF_Destroy(curSrc->srcSurf);
            if (gcmIS_ERROR(status))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console,
                    "Destroy SrcSurface[%d] failed:%s(%d)\n", i, GalStatusString(status), status);
            }
        }
    }

    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_2D_MULTI_SRC_BLT_BILINEAR_FILTER,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status;
    gctINT i;
    const char *sBasicFile[] = {
        "resource/android_720x1280_icons.bmp",
        "resource/zero2_UYVY_1920x1080_Linear.vimg",
        "resource/VV_Background.bmp",
        "resource/source_YUV420_400x400_Linear.vimg",
        "resource/zero2_ARGB4.bmp",
        "resource/GoneFishing2.bmp",
        "resource/zero2_YUY2_640X480_Linear.vimg",
        "resource/smooth_720p.bmp",
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

    memset(t2d, 0, sizeof(Test2D));

    t2d->runtime = runtime;

    t2d->dstSurf    = runtime->target;
    t2d->dstFormat = runtime->format;
    t2d->dstWidth = 0;
    t2d->dstHeight = 0;
    t2d->dstStride = 0;
    t2d->dstPhyAddr = 0;
    t2d->dstLgcAddr = 0;

    gcmONERROR(gco2D_SetStateU32(runtime->engine2d,
                                 gcv2D_STATE_MULTI_SRC_BLIT_BILINEAR_FILTER,
                                 gcvTRUE));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    for (i = 0; i < 8; i++)
    {
        gcmONERROR(ReloadSourceSurface(t2d, i, sBasicFile[i]));
    }

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = 6;
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
