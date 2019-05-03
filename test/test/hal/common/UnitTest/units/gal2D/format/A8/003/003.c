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
 *  Feature:    ColorSource - width & stride & color format
 *  API:        gco2D_SetSource gco2D_SetColorSource
 *  Check:
*/
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFormatA8_003\n" \
"Operation: Test format A8 output with rotation & mirror.\n" \
"2D API: gco2D_SetColorSourceEx gco2D_SetTargetEx gco2D_SetBitBlitMirror\n" \
"Src: Size        [640x480]\n"\
"     Rect        [change according to frameNo]\n"\
"     Format      [A8]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Dst: Size        [configurable]\n"\
"     Rect        [change according to frameNo]\n"\
"     Format      [A8]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: [None]\n"\
"Alphablend: [disable]\n" \
"HW feature dependency: ";

typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;

     //A8 source surface
    gcoSURF            srcSurf;
    gceSURF_FORMAT    srcFormat;
    gctUINT            srcWidth;
    gctUINT            srcHeight;
    gctINT            srcStride;
    gctUINT32        srcPhyAddr;
    gctPOINTER        srcLgcAddr;

    // A8 destination surface
    gcoSURF            tmpSurf;
    gceSURF_FORMAT    tmpFormat;
    gctUINT            tmpWidth;
    gctUINT            tmpHeight;
    gctINT            tmpStride;
    gctUINT32        tmpPhyAddr;
    gctPOINTER        tmpLgcAddr;

    // dst surface
    gcoSURF            dstSurf;
    gceSURF_FORMAT    dstFormat;
    gctUINT            dstWidth;
    gctUINT            dstHeight;
    gctINT            dstStride;
    gctUINT32        dstPhyAddr;
    gctPOINTER        dstLgcAddr;

} Test2D;

static gceSURF_ROTATION rotationList [] =
{
    gcvSURF_0_DEGREE,
    gcvSURF_90_DEGREE,
    gcvSURF_180_DEGREE,
    gcvSURF_270_DEGREE,
    gcvSURF_FLIP_X,
    gcvSURF_FLIP_Y,
};

static gctBOOL SimulateSurfaceA8(Test2D *t2d, gcsRECT* clipRect,
                      gcsRECT* copyRect, gceSURF_ROTATION srcRotation,
                      gceSURF_ROTATION dstRotation, gctINT frameNo)
{
    gceSTATUS status = gcvSTATUS_OK;
    gco2D egn2D = t2d->runtime->engine2d;
    gcsRECT srcRect = {0, 0, 0, 0};

    gcmONERROR(gco2D_SetColorSourceEx(egn2D,
        t2d->srcPhyAddr,
        t2d->srcStride,
        t2d->srcFormat,
        srcRotation,
        t2d->srcWidth,
        t2d->srcHeight,
        gcvFALSE,
        gcvSURF_OPAQUE,
        0));

    gcmONERROR(gco2D_SetSource(egn2D, &srcRect));

    gcmONERROR(gco2D_SetClipping(egn2D, clipRect));

    gcmONERROR(gco2D_SetTargetEx(egn2D,
        t2d->tmpPhyAddr,
        t2d->tmpStride,
        dstRotation,
        t2d->tmpWidth,
        t2d->tmpHeight));

    gcmONERROR(gco2D_SetBitBlitMirror(egn2D,
        (frameNo/2)?gcvTRUE:gcvFALSE, (frameNo%2)?gcvTRUE:gcvFALSE));

    gcmONERROR(gco2D_Blit(egn2D, 1, copyRect, 0xCC, 0xCC, t2d->tmpFormat));

    gcmONERROR(gco2D_SetBitBlitMirror(egn2D, gcvFALSE, gcvFALSE));

    gcmONERROR(gco2D_Flush(egn2D));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    return gcvTRUE;

OnError:

    GalOutput(GalOutputType_Error | GalOutputType_Console,
        "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));

    return gcvFALSE;
}

gctBOOL Display(Test2D *t2d,
                gceSURF_FORMAT    srcFormat,
                gctUINT            srcWidth,
                gctUINT            srcHeight,
                gctINT            srcStride,
                gctUINT32        srcPhyAddr,
                gcsRECT *srcRect
                )
{
    gctUINT32 horFactor, verFactor;
    gco2D egn2D = t2d->runtime->engine2d;
    gceSTATUS status;
    gcsRECT dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};

    // set source color and rect
    gcmONERROR(gco2D_SetColorSource(egn2D, srcPhyAddr, srcStride, srcFormat,
                    gcvSURF_0_DEGREE, srcWidth, gcvFALSE, gcvSURF_OPAQUE, 0));

    gcmONERROR(gco2D_SetSource(egn2D, srcRect));

    // set tmp and clippint rect
    gcmONERROR(gco2D_SetTarget(egn2D, t2d->dstPhyAddr, t2d->dstStride, gcvSURF_0_DEGREE, t2d->dstWidth));

    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    /* Calculate the stretch factors. */
    gcmONERROR(gco2D_CalcStretchFactor(egn2D, srcRect->right - srcRect->left,
        dstRect.right - dstRect.left, &horFactor));

    gcmONERROR(gco2D_CalcStretchFactor(egn2D, srcRect->bottom - srcRect->top,
        dstRect.bottom - dstRect.top, &verFactor));

    /* Program the stretch factors. */
    gcmONERROR(gco2D_SetStretchFactors(egn2D, horFactor, verFactor));

    gcmONERROR(gco2D_StretchBlit(egn2D, 1, &dstRect, 0xCC, 0xCC, t2d->dstFormat));

    gcmONERROR(gco2D_Flush(egn2D));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    return gcvTRUE;

OnError:

    GalOutput(GalOutputType_Error | GalOutputType_Console,
        "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));

    return gcvFALSE;
}

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status;
    gctBOOL    ret = gcvTRUE;
    gcsRECT clipRect;
    gceSURF_ROTATION srcRotation, dstRotation;
    gcsRECT tempRect = {0, 0, t2d->srcHeight, t2d->srcHeight};

    // clear tmp surface
    memset(t2d->tmpLgcAddr, 0xffffffff,
        t2d->tmpHeight*t2d->tmpStride);

    // clear dst surface
    memset(t2d->dstLgcAddr, 0xffffffff,
        t2d->dstHeight*t2d->dstStride);

    srcRotation = rotationList[frameNo/gcmCOUNTOF(rotationList)];
    dstRotation = rotationList[frameNo%gcmCOUNTOF(rotationList)];

    clipRect.left = 0;
    clipRect.top = 0;
    clipRect.right = t2d->tmpWidth;
    clipRect.bottom = t2d->tmpWidth;

    gcmONERROR(SimulateSurfaceA8(t2d, &clipRect, &tempRect,srcRotation, dstRotation,frameNo));

    gcmONERROR(Display(t2d, gcvSURF_INDEX8, t2d->tmpWidth,
        t2d->tmpHeight, t2d->tmpStride, t2d->tmpPhyAddr, &clipRect));

    return ret;

OnError:

    GalOutput(GalOutputType_Error | GalOutputType_Console,
        "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));

    return gcvFALSE;
}

static void CDECL Destroy(Test2D *t2d)
{
    gceSTATUS status = gcvSTATUS_OK;

    // destroy A8 source surface
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

    // destroy A8 destination surface
    if (t2d->tmpSurf != gcvNULL)
    {
        if (t2d->tmpLgcAddr)
        {
            if (gcmIS_ERROR(gcoSURF_Unlock(t2d->tmpSurf, t2d->tmpLgcAddr)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock desSurf failed:%s\n", GalStatusString(status));
            }
            t2d->tmpLgcAddr = 0;
        }

        if (gcmIS_ERROR(gcoSURF_Destroy(t2d->tmpSurf)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Surf failed:%s\n", GalStatusString(status));
        }
    }

    if ((t2d->dstSurf != gcvNULL) && (t2d->dstLgcAddr != gcvNULL))
    {
        if (gcmIS_ERROR(gcoSURF_Unlock(t2d->dstSurf, t2d->dstLgcAddr)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock Display Surf failed:%s\n", GalStatusString(status));
        }
        t2d->dstLgcAddr = gcvNULL;
    }

    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_2D_A8_TARGET,
    gcvFEATURE_2D_NO_COLORBRUSH_INDEX8,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status;
    gctSTRING sourcefile = "resource/index8_A8.bmp";

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

    // A8 Surfaces
    t2d->srcSurf    = gcvNULL;
    t2d->srcWidth = 0;
    t2d->srcHeight = 0;
    t2d->srcStride = 0;
    t2d->srcPhyAddr = 0;
    t2d->srcLgcAddr = 0;
    t2d->srcFormat = gcvSURF_UNKNOWN;

    t2d->tmpSurf    = gcvNULL;
    t2d->tmpWidth = 0;
    t2d->tmpHeight = 0;
    t2d->tmpStride = 0;
    t2d->tmpPhyAddr = 0;
    t2d->tmpLgcAddr = 0;
    t2d->tmpFormat = gcvSURF_UNKNOWN;

    ////////////////////////////////////////////////////////////////////
    // create A8 source surface: take index8 as A8
    t2d->srcSurf = GalLoadA82Surface(t2d->runtime->hal, sourcefile);
    if (t2d->srcSurf == NULL)
    {
        GalOutput(GalOutputType_Error, "can not load %s\n", sourcefile);
        gcmONERROR(gcvSTATUS_NOT_FOUND);
    }
    gcmONERROR(gcoSURF_GetAlignedSize(t2d->srcSurf,
                                        &t2d->srcWidth,
                                        &t2d->srcHeight,
                                        &t2d->srcStride));

    t2d->srcFormat = gcvSURF_A8;

    gcmONERROR(gcoSURF_Lock(t2d->srcSurf, &t2d->srcPhyAddr, &t2d->srcLgcAddr));

    ////////////////////////////////////////////////////////////////////
    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    t2d->dstFormat = runtime->format;

    ////////////////////////////////////////////////////////////////////
    // create A8 destination surface
    gcmONERROR(gcoSURF_Construct(t2d->runtime->hal, t2d->dstWidth,
        t2d->dstWidth, 1, gcvSURF_BITMAP, gcvSURF_A8, gcvPOOL_DEFAULT, &t2d->tmpSurf));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->tmpSurf,
                                        &t2d->tmpWidth,
                                        &t2d->tmpHeight,
                                        &t2d->tmpStride));

    gcmONERROR(gcoSURF_Lock(t2d->tmpSurf, &t2d->tmpPhyAddr, &t2d->tmpLgcAddr));

    t2d->tmpFormat = gcvSURF_A8;

    ////////////////////////////////////////////////////////////////////

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = gcmCOUNTOF(rotationList)*gcmCOUNTOF(rotationList);
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
