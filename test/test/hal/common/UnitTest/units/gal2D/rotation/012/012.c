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
 *  Feature:    MaskedSource
 *  API:        gco2D_SetMaskedSource
 *  Check:      Transparency -- Masked Source
 */
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DRotation012\n" \
"Operation: Test blit with the screen with MaskedSource -- test full rotations.\n" \
"2D API: gco2D_MonoBlit gco2D_SetMaskedSourceEx\n" \
"Src: Size        [64x100]\n"\
"     Rect        [0,0,64,100]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Dst: Size        [configurable]\n"\
"     Rect        [0,0,64,100]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: [None]\n"\
"StreamPack: [UNPACKED]\n" \
"Alphablend: [disable]\n" \
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

    //source surface
    gcoSURF            srcSurf;
    gceSURF_FORMAT    srcFormat;
    gctUINT            srcWidth;
    gctUINT            srcHeight;
    gctINT            srcStride;
    gctUINT32        srcPhyAddr;
    gctPOINTER        srcLgcAddr;

    // masked source
    gceSURF_MONOPACK monoSrcDataPackType;
    gctUINT32 *monoSrcData;
    gctUINT32 monoWidth;
    gctUINT32 monoHeight;
} Test2D;

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gcsRECT srcRect = {0, 0, 0, 0};
    gctUINT rectWidth = 64, rectHeight = 100;
    gcsRECT dstRect;
    gco2D egn2D = t2d->runtime->engine2d;
    gceSTATUS status;
    gcsRECT  streamRect;
    gcsPOINT streamSize;
    gceSURF_ROTATION srcRot, dstRot;

    switch (frameNo)
    {
    case 0:
        srcRot = gcvSURF_0_DEGREE;
        dstRot = gcvSURF_0_DEGREE;
        break;

    case 1:
        srcRot = gcvSURF_0_DEGREE;
        dstRot = gcvSURF_90_DEGREE;
        break;

    case 2:
        srcRot = gcvSURF_90_DEGREE;
        dstRot = gcvSURF_0_DEGREE;
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

    default:
        return gcvFALSE;
    }

    gcmONERROR(gco2D_SetMaskedSourceEx(egn2D, t2d->srcPhyAddr, t2d->srcStride, t2d->srcFormat,
        gcvSURF_0_DEGREE, gcvSURF_UNPACKED, srcRot, t2d->srcWidth, t2d->srcHeight));

    gcmONERROR(gco2D_SetSource(egn2D, &srcRect));

    gcmONERROR(gco2D_SetTargetEx(egn2D, t2d->dstPhyAddr, t2d->dstStride, dstRot, t2d->dstWidth, t2d->dstHeight));

    gcmONERROR(gco2D_SetClipping(egn2D, gcvNULL));

    dstRect.left = dstRect.top = streamRect.left = streamRect.top = 0;
    dstRect.right = streamRect.right = rectWidth;
    dstRect.bottom = streamRect.bottom = rectHeight;

    streamSize.x = t2d->monoWidth;
    streamSize.y = t2d->monoHeight;

    gcmONERROR(gco2D_MonoBlit(egn2D, t2d->monoSrcData,
        &streamSize, &streamRect, gcvSURF_UNPACKED, gcvSURF_UNPACKED, &dstRect, 0x33, 0xCC, t2d->dstFormat));

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
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock dstSurf failed:%s\n", GalStatusString(status));
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

    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_ANDROID_ONLY,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT srcSize, i;
    char *srcBmpfile = "resource/rotate.bmp";

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

    if (runtime->ChipModel == gcv320 && runtime->ChipRevision == 0x5007)
    {
        GalOutput(GalOutputType_Result | GalOutputType_Console, "MonoBlit is not supported.\n");
        runtime->notSupport = gcvTRUE;
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
    t2d->srcFormat = gcvSURF_UNKNOWN;

    // check the chip version
    if (gcoHAL_IsFeatureAvailable(runtime->hal, gcvFEATURE_2D_BITBLIT_FULLROTATION)!= gcvTRUE)
    {
        // the hardware does not supported all the rotations
        GalOutput(GalOutputType_Result | GalOutputType_Console, "the hardware has not all the rotations.\n");
        t2d->base.frameCount  = 2;
    }
    else
    {
        t2d->base.frameCount  = 20;
    }

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    // create source surface
    t2d->srcSurf = GalLoadDIB2Surface(t2d->runtime->hal, srcBmpfile);
    if (t2d->srcSurf == NULL)
    {
        GalOutput(GalOutputType_Error, "can not load %s\n", srcBmpfile);
        gcmONERROR(gcvSTATUS_NOT_FOUND);
    }

    gcmONERROR(gcoSURF_GetSize(t2d->srcSurf,
                                        &t2d->srcWidth,
                                        &t2d->srcHeight,
                                        gcvNULL));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->srcSurf,
                                        gcvNULL,
                                        gcvNULL,
                                        &t2d->srcStride));

    gcmONERROR(gcoSURF_GetFormat(t2d->srcSurf,
                                       gcvNULL,
                                       &t2d->srcFormat));

    gcmONERROR(gcoSURF_Lock(t2d->srcSurf, &t2d->srcPhyAddr, &t2d->srcLgcAddr));

    // masked source
    t2d->monoWidth = t2d->srcWidth;
    t2d->monoHeight = t2d->srcHeight;
    t2d->monoSrcDataPackType = gcvSURF_UNPACKED;
    srcSize = t2d->monoWidth * t2d->monoHeight >> 5;
    t2d->monoSrcData = (gctUINT32*)malloc(srcSize * sizeof(gctUINT32));
    for (i = 0; i < srcSize; i++)
    {
        *(t2d->monoSrcData + i) = CONVERT_BYTE(i);
    }

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
