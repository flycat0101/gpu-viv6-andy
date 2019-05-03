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
 *  Feature:    Destination transparency. It only works with PE 2.0 and above.
 *  API:        gco2D_SetTransparencyAdvanced and gco2D_SetTargetColorKeyAdvanced
 *  Check:      Destination transparency.
 */
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DDestination005\n" \
"Operation: Test  destination transparency.\n" \
"2D API: gco2D_Blit gco2D_SetTransparencyAdvanced gco2D_SetTargetColorKeyAdvanced\n" \
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
"     Transparency[None/colorKey]\n" \
"Brush: [None]\n"\
"Alphablend: [disable]\n" \
"HW feature dependency: ";

typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;

    /* Dest surface. */
    gcoSURF            dstSurf;
    gceSURF_FORMAT    dstFormat;
    gctUINT            dstWidth;
    gctUINT            dstHeight;
    gctINT            dstStride;
    gctUINT32        dstPhyAddr;
    gctPOINTER        dstLgcAddr;

    /* Source surface. */
    gcoSURF            srcSurf;
    gceSURF_FORMAT    srcFormat;
    gctUINT            srcWidth;
    gctUINT            srcHeight;
    gctINT            srcStride;
    gctUINT32        srcPhyAddr;
    gctPOINTER        srcLgcAddr;
} Test2D;

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gcsRECT   dstRect   = { 0, 0, t2d->dstWidth, t2d->dstHeight };
    gcsRECT   colorRect = { t2d->dstWidth/4, t2d->dstHeight/4, t2d->dstWidth*3/4, t2d->dstHeight*3/4 };
    gco2D     egn2D     = t2d->runtime->engine2d;
    gctUINT32 colorKey  = 0x80CFDE55;
    gceSTATUS status;

    /* Clear dst surface with red. */
    gcmONERROR(Gal2DCleanSurface(t2d->runtime->hal, t2d->dstSurf, COLOR_ARGB8(0xFF, 0, 0xFF, 0)));

    /* Set the target. */
    gcmONERROR(gco2D_SetTarget(egn2D,
                                 t2d->dstPhyAddr,
                                 t2d->dstStride,
                                 gcvSURF_0_DEGREE,
                                 t2d->dstWidth));

    /* Clear the dst surface with the color key in the center. */
    gcmONERROR(gco2D_Clear(egn2D, 1, &colorRect, colorKey, 0xCC, 0xCC, t2d->dstFormat));

    /* Set the clipping. */
    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    /* Set the source. */
    gcmONERROR(gco2D_SetColorSourceAdvanced(egn2D,
                                              t2d->srcPhyAddr,
                                              t2d->srcStride,
                                              t2d->srcFormat,
                                              gcvSURF_0_DEGREE,
                                              t2d->srcWidth,
                                              t2d->srcHeight,
                                              gcvFALSE));

    gcmONERROR(gco2D_SetSource(egn2D, &dstRect));

    if (frameNo == 0)
    {
        /* Disable the destination transparency. */
        gcmONERROR(gco2D_SetTransparencyAdvanced(egn2D,
                                                   gcv2D_OPAQUE,
                                                   gcv2D_OPAQUE,
                                                   gcv2D_OPAQUE));
    }
    else if (frameNo == 1)
    {
        /* Set the destination transparency. */
        gcmONERROR(gco2D_SetTransparencyAdvanced(egn2D,
                                                   gcv2D_OPAQUE,
                                                   gcv2D_KEYED,
                                                   gcv2D_OPAQUE));
    }

    gcmONERROR(gco2D_SetTargetColorKeyAdvanced(egn2D, colorKey));

    /* Blit. */
    gcmONERROR(gco2D_Blit(egn2D, 1, &dstRect, 0xCC, 0xAA, t2d->dstFormat));

    /* Disable destination transparency. */
    gcmONERROR(gco2D_SetTransparencyAdvanced(egn2D,
                                               gcv2D_OPAQUE,
                                               gcv2D_OPAQUE,
                                               gcv2D_OPAQUE));

    /* Flush the HW cache. */
    gcmONERROR(gco2D_Flush(egn2D));

    /* Wait for HW idle. */
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
        if (gcmIS_ERROR((gcoSURF_Unlock(t2d->dstSurf, t2d->dstLgcAddr))))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock dstSurf failed:%s\n", GalStatusString(status));
        }
        t2d->dstLgcAddr = gcvNULL;
    }

    /* Destroy source surface. */
    if (t2d->srcSurf != gcvNULL)
    {
        if (t2d->srcLgcAddr)
        {
            if (gcmIS_ERROR((gcoSURF_Unlock(t2d->srcSurf, t2d->srcLgcAddr))))
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
    gcvFEATURE_2DPE20,
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

    t2d->dstSurf    = runtime->target;
    t2d->dstFormat  = runtime->format;
    t2d->dstWidth   = 0;
    t2d->dstHeight  = 0;
    t2d->dstStride  = 0;
    t2d->dstPhyAddr = 0;
    t2d->dstLgcAddr = 0;

    t2d->srcSurf    = gcvNULL;
    t2d->srcWidth   = 0;
    t2d->srcHeight  = 0;
    t2d->srcStride  = 0;
    t2d->srcPhyAddr = 0;
    t2d->srcLgcAddr = 0;
    t2d->srcFormat  = gcvSURF_A8R8G8B8;

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    /* Create source surface. */
    gcmONERROR(gcoSURF_Construct(t2d->runtime->hal,
                                      t2d->runtime->width,
                                      t2d->runtime->height,
                                   1,
                                      gcvSURF_BITMAP,
                                      t2d->srcFormat,
                                   gcvPOOL_DEFAULT,
                                   &t2d->srcSurf));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->srcSurf,
                                        &t2d->srcWidth,
                                        &t2d->srcHeight,
                                        &t2d->srcStride));

    gcmONERROR(gcoSURF_Lock(t2d->srcSurf, &t2d->srcPhyAddr, &t2d->srcLgcAddr));

    /* Clear src surface. */
    gcmONERROR(Gal2DCleanSurface(t2d->runtime->hal,
                                t2d->srcSurf,
                                COLOR_ARGB8(0xFF, 0x55, 0x55, 0x55)));

    t2d->base.render      = (PGalRender)Render;
    t2d->base.destroy     = (PGalDestroy)Destroy;
    t2d->base.frameCount  = 2;
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
