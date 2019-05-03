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
 *  Feature:    Multiply. It only works with PE 2.0 and above.
 *  API:        gco2D_SetPixelMultiplyModeAdvanced
 *  Check:      Multiply modes for source, dest and global color.
 */
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DMultiply001\n" \
"Operation: Test multiply modes for source, dest and global color.\n" \
"2D API: gco2D_SetPixelMultiplyModeAdvanced\n" \
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
"Alphablend: [enable]\n" \
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

static gce2D_PIXEL_COLOR_MULTIPLY_MODE srcPremultiplySrcAlpha[2] = {
                                            gcv2D_COLOR_MULTIPLY_DISABLE,
                                            gcv2D_COLOR_MULTIPLY_ENABLE };

static gce2D_PIXEL_COLOR_MULTIPLY_MODE dstPremultiplyDstAlpha[2] = {
                                            gcv2D_COLOR_MULTIPLY_DISABLE,
                                            gcv2D_COLOR_MULTIPLY_ENABLE };

static gce2D_GLOBAL_COLOR_MULTIPLY_MODE srcPremultiplyGlobalMode[3] = {
                                            gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE,
                                            gcv2D_GLOBAL_COLOR_MULTIPLY_ALPHA,
                                            gcv2D_GLOBAL_COLOR_MULTIPLY_COLOR };

static gce2D_PIXEL_COLOR_MULTIPLY_MODE dstDemultiplyDstAlpha[2] = {
                                            gcv2D_COLOR_MULTIPLY_DISABLE,
                                            gcv2D_COLOR_MULTIPLY_ENABLE };

static gctUINT srcPremultiplySrcAlphaNum   = 2;
static gctUINT dstPremultiplyDstAlphaNum   = 2;
static gctUINT srcPremultiplyGlobalModeNum = 3;
static gctUINT dstDemultiplyDstAlphaNum    = 2;;

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gcsRECT   dstRect = { 0, 0, t2d->dstWidth, t2d->dstHeight };
    gco2D     egn2D   = t2d->runtime->engine2d;
    gceSTATUS status;
    gctUINT      i, j, k, l, num;

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

    /* Set the target. */
    gcmONERROR(gco2D_SetTarget(egn2D,
                                 t2d->dstPhyAddr,
                                 t2d->dstStride,
                                 gcvSURF_0_DEGREE,
                                 t2d->dstWidth));

    num  = 0;

    for (i = 0; i < srcPremultiplySrcAlphaNum; i++)
    {
        for (j = 0; j < dstPremultiplyDstAlphaNum; j++)
        {
            for (k = 0; k < srcPremultiplyGlobalModeNum; k++)
            {
                for (l = 0; l < dstDemultiplyDstAlphaNum; l++)
                {
                    if (num++ == frameNo)
                    {
                        goto Next;
                    }
                }
            }
        }
    }

Next:
    /* Set the multiply modes. */
    gcmONERROR(gco2D_SetPixelMultiplyModeAdvanced(egn2D,
                                                    srcPremultiplySrcAlpha[i],
                                                    dstPremultiplyDstAlpha[j],
                                                    srcPremultiplyGlobalMode[k],
                                                    dstDemultiplyDstAlpha[l]));

    if (srcPremultiplyGlobalMode[k] != gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE)
    {
        /* Set the global color. */
        gcmONERROR(gco2D_SetSourceGlobalColorAdvanced(egn2D, 0xF0CDFFA0));
    }

    /* Enable alpha blending so that we can see the dest's influnce. */
    gcmONERROR(gco2D_EnableAlphaBlendAdvanced(egn2D,
                                                gcvSURF_PIXEL_ALPHA_STRAIGHT,
                                                gcvSURF_PIXEL_ALPHA_STRAIGHT,
                                                gcvSURF_GLOBAL_ALPHA_OFF,
                                                gcvSURF_GLOBAL_ALPHA_OFF,
                                                gcvSURF_BLEND_ONE,
                                                gcvSURF_BLEND_INVERSED));

    /* Blit. */
    gcmONERROR(gco2D_Blit(egn2D, 1, &dstRect, 0xCC, 0xCC, t2d->dstFormat));

    /* Disable alpha blending. */
    gcmONERROR(gco2D_DisableAlphaBlend(egn2D));

    /* Disable multiply. */
    gcmONERROR(gco2D_SetPixelMultiplyModeAdvanced(egn2D,
                                                    gcv2D_COLOR_MULTIPLY_DISABLE,
                                                    gcv2D_COLOR_MULTIPLY_DISABLE,
                                                    gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE,
                                                    gcv2D_COLOR_MULTIPLY_DISABLE));

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
        if (gcmIS_ERROR(gcoSURF_Unlock(t2d->dstSurf, t2d->dstLgcAddr)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock desSurf failed:%s\n", GalStatusString(status));
        }
        t2d->dstLgcAddr = gcvNULL;
    }

    /* Destroy source surface. */
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

    if (runtime->ChipModel == gcv520 && runtime->ChipRevision < 0x5520)
    {
        GalOutput(GalOutputType_Result | GalOutputType_Console, "dst-demultiply is not supported.\n");
        runtime->notSupport = gcvTRUE;
        return gcvFALSE;
    }

    if (runtime->notSupport)
        return gcvFALSE;

    t2d->runtime    = runtime;

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

    /* Clear dst surface. */
    gcmONERROR(Gal2DCleanSurface(t2d->runtime->hal, t2d->dstSurf, COLOR_ARGB8(0x80, 0xCF, 0xDE, 0x55)));

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
    gcmONERROR(Gal2DCleanSurface(t2d->runtime->hal, t2d->srcSurf, COLOR_ARGB8(0x80, 0x79, 0xFF, 0xA0)));

    t2d->base.render      = (PGalRender)Render;
    t2d->base.destroy     = (PGalDestroy)Destroy;
    t2d->base.frameCount  = 24;
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
