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
 *  Feature:    ColorSource - relative
 *  API:        gco2D_SetSource gco2D_SetColorSource/gco2D_SetColorSourceAdvanced
 *                gco2D_SetColorSource is only working with old PE (<2.0) and
 *                gco2D_SetColorSourceAdvanced is only working with PE 2.0 and above.
 *  Check:      relative
*/
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DColorSource003\n" \
"Operation: Test blit the screen with ColorSource -- test relative.\n" \
"2D API: gco2D_SetSource gco2D_SetColorSource/gco2D_SetColorSourceAdvanced\n" \
"Src: Size        [640x480]\n"\
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

    // destination surface
    gcoSURF            dstSurf;
    gceSURF_FORMAT    dstFormat;
    gctUINT            dstWidth;
    gctUINT            dstHeight;
    gctINT            dstStride;
    gctUINT32        dstPhyAddr;
    gctPOINTER        dstLgcAddr;
} Test2D;

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gcoSURF tmpSurf;
    char * bmpfile = "resource/relative.bmp";
    gcsRECT srcRect = {0, 0, t2d->dstWidth, t2d->dstHeight}, dstRect[2];
    gcsRECT clipRect = {0, 0, t2d->dstWidth, t2d->dstHeight};
    gco2D egn2D = t2d->runtime->engine2d;
    gceSTATUS status;
    gctBOOL relative;

    gcmONERROR(gco2D_SetClipping(egn2D, &clipRect));

    switch (frameNo)
    {
    case 0:
        srcRect.left = 0; srcRect.top = 0;
        dstRect[0].left = (gctINT32)(200 * t2d->dstWidth / 640); dstRect[0].top = (gctINT32)(150 * t2d->dstHeight / 480);
        dstRect[0].right = (gctINT32)(264 * t2d->dstWidth / 640); dstRect[0].bottom = (gctINT32)(214 * t2d->dstHeight / 480);
        dstRect[1].left = (gctINT32)(300 * t2d->dstWidth / 640); dstRect[1].top = (gctINT32)(160 * t2d->dstHeight / 480);
        dstRect[1].right = (gctINT32)(364 * t2d->dstWidth / 640); dstRect[1].bottom = (gctINT32)(224 * t2d->dstHeight / 480);
        relative = gcvFALSE;
        break;

    case 1:
        srcRect.left = 0; srcRect.top = (gctINT32)(200 * t2d->dstHeight / 480);
        dstRect[0].left = 0; dstRect[0].top = (gctINT32)(152 * t2d->dstHeight / 480);
        dstRect[0].right = (gctINT32)(128 * t2d->dstWidth / 640); dstRect[0].bottom = (gctINT32)(280 * t2d->dstHeight / 480);
        dstRect[1].left = (gctINT32)(512 * t2d->dstWidth / 640); dstRect[1].top = (gctINT32)(152 * t2d->dstHeight / 480);
        dstRect[1].right = (gctINT32)(640 * t2d->dstWidth / 640); dstRect[1].bottom = (gctINT32)(280 * t2d->dstHeight / 480);
        relative = gcvTRUE;
        break;

    default:
        return gcvFALSE;
    }

    // init dst surface
    tmpSurf = GalLoadDIB2Surface(t2d->runtime->hal, bmpfile);
    if (tmpSurf == NULL)
    {
        GalOutput(GalOutputType_Error, "can not load %s\n", bmpfile);
        gcmONERROR(gcvSTATUS_NOT_FOUND);
    }

    gcmONERROR(gcoSURF_Blit(tmpSurf, t2d->dstSurf, 1, gcvNULL, gcvNULL, gcvNULL, 0xCC, 0xCC, gcvSURF_OPAQUE, 0, gcvNULL, gcvSURF_UNPACKED));

    if (gcmIS_ERROR(gcoSURF_Destroy(tmpSurf)))
    {
        GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Surf failed:%s\n", GalStatusString(status));
    }

    // set color source
    if (t2d->runtime->pe20)
    {
        gcmONERROR(gco2D_SetColorSourceAdvanced(egn2D, t2d->dstPhyAddr, t2d->dstStride, t2d->dstFormat,
                        gcvSURF_0_DEGREE, t2d->dstWidth, t2d->dstHeight, relative));
    }
    else
    {
        gcmONERROR(gco2D_SetColorSource(egn2D, t2d->dstPhyAddr, t2d->dstStride, t2d->dstFormat,
                        gcvSURF_0_DEGREE, t2d->dstWidth, relative, gcvSURF_OPAQUE, 0));
    }

    gcmONERROR(gco2D_SetSource(egn2D, &srcRect));

    gcmONERROR(gco2D_SetTarget(egn2D, t2d->dstPhyAddr, t2d->dstStride, gcvSURF_0_DEGREE, t2d->dstWidth));

    gcmONERROR(gco2D_Blit(egn2D, 2, dstRect, 0xCC, 0xCC, t2d->dstFormat));

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

    free(t2d);
}

const gceFEATURE FeatureCombineList[]=
{
    gcvFEATURE_2D_COMPRESSION,
    gcvFEATURE_SEPARATE_SRC_DST,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status;

    gctUINT32 k, listLen2 = sizeof(FeatureCombineList)/sizeof(gctINT);
    gctBOOL featureStatus, supported = gcvFALSE;
    char featureName[FEATURE_NAME_LEN], featureMsg[FEATURE_MSG_LEN];

    runtime->wholeDescription = (char*)malloc(FEATURE_NAME_LEN * listLen2 + strlen(s_CaseDescription) + 1);

    if (runtime->wholeDescription == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    memcpy(runtime->wholeDescription, s_CaseDescription, strlen(s_CaseDescription) + 1);

    for(k = 0; k < listLen2; k++)
    {
        gcmONERROR(GalQueryFeatureStr(FeatureCombineList[k], featureName, featureMsg, &featureStatus));
        if (gcoHAL_IsFeatureAvailable(runtime->hal, FeatureCombineList[k]) != featureStatus)
        {
            supported = gcvTRUE;
        }
        strncat(runtime->wholeDescription, featureName, k==listLen2-1 ? strlen(featureName)+1:strlen(featureName));
    }

    if (!supported)
    {
        GalOutput(GalOutputType_Result | GalOutputType_Console, "%s is not supported.\n", featureMsg);
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

       gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = 2;
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
