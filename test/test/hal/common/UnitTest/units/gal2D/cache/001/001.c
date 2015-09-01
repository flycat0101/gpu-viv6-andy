/****************************************************************************
*
*    Copyright 2012 - 2015 Vivante Corporation, Santa Clara, California.
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
 *  Feature:    LINE - line position and count
 *  API:        gco2D_Line
 *  Check:
*/
#include <galUtil.h>

#define MAX_LINES 56

static gctCONST_STRING s_CaseDescription = \
"Case gal2DCache001\n" \
"Operation: Test cache. Draw line with alphablend from (0,7,8,64) to (56,7,64,64).\n" \
"2D API: gco2D_Line\n" \
"Src: [None]\n"\
"Dst: Size        [configurable]\n"\
"     Rect        [0,7,8,64 - 56,7,64,64]\n"\
"     Format      [ARGB8888/RGB565]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: Type   [SolidBrush]\n"\
"       Format [configurable]\n"\
"       Offset [0]\n" \
"Alphablend: [enable]\n" \
"HW feature dependency: ";

typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;

    gcoSURF        tmpSurf;
    gctUINT32      tmpPhyAddr;
    gctPOINTER     tmpVirtAddr;
    gceSURF_FORMAT tmpFormat;
    gctUINT32      tmpWidth;
    gctUINT32      tmpHeight;
    gctINT         tmpStride;

    // source surface
    gcoSURF         srcSurf;
    gceSURF_FORMAT  srcFormat;
    gctUINT         srcWidth;
    gctUINT         srcHeight;
    gctINT          srcStride;
    gctUINT32       srcPhyAddr;
    gctPOINTER      srcVirtAddr;


    // destination surface
    gcoSURF         dstSurf;
    gceSURF_FORMAT  dstFormat;
    gctUINT         dstWidth;
    gctUINT         dstHeight;
    gctINT          dstStride;
    gctUINT32       dstPhyAddr;
    gctPOINTER      dstLgcAddr;
} Test2D;

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gco2D egn2D = t2d->runtime->engine2d;
    gcsRECT dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};
    gcsRECT line;
    gctUINT32 i;
    gceSTATUS status;
    gcoBRUSH brush;
    gceSURF_FORMAT format = gcvSURF_A8R8G8B8;

    switch (frameNo)
    {
        case 0:
            format = gcvSURF_R5G6B5;
            break;
        case 1:
            format = gcvSURF_A8R8G8B8;
            break;
    }

    // prepare src surface
    gcmONERROR(gcoSURF_Construct(t2d->runtime->hal,
                                 t2d->dstWidth,
                                 t2d->dstHeight,
                                 1,
                                 gcvSURF_BITMAP,
                                 format,
                                 gcvPOOL_DEFAULT,
                                 &t2d->srcSurf));

    gcmONERROR(gcoSURF_GetSize(t2d->srcSurf,
                                 &t2d->srcWidth,
                                 &t2d->srcHeight,
                                 gcvNULL));

    gcmONERROR(gcoSURF_GetFormat(t2d->srcSurf,
                                   gcvNULL,
                                   &t2d->srcFormat));


    gcmONERROR(gcoSURF_Lock(t2d->srcSurf,
                            &t2d->srcPhyAddr,
                            &t2d->srcVirtAddr));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->srcSurf,
                                        gcvNULL,
                                        gcvNULL,
                                        &t2d->srcStride));

    // prepare tmp surface
    gcmONERROR(gcoSURF_Construct(t2d->runtime->hal,
                                 t2d->dstWidth,
                                 t2d->dstHeight,
                                 1,
                                 gcvSURF_BITMAP,
                                 format,
                                 gcvPOOL_DEFAULT,
                                 &t2d->tmpSurf));

    gcmONERROR(gcoSURF_GetSize(t2d->tmpSurf,
                                 &t2d->tmpWidth,
                                 &t2d->tmpHeight,
                                 gcvNULL));

    gcmONERROR(gcoSURF_GetFormat(t2d->tmpSurf,
                                   gcvNULL,
                                   &t2d->tmpFormat));


    gcmONERROR(gcoSURF_Lock(t2d->tmpSurf,
                            &t2d->tmpPhyAddr,
                            &t2d->tmpVirtAddr));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->tmpSurf,
                                        gcvNULL,
                                        gcvNULL,
                                        &t2d->tmpStride));

    gcmONERROR(Gal2DCleanSurface(t2d->runtime->hal, t2d->srcSurf, COLOR_ARGB8(0x80, 0x12, 0x34, 0x56)));
    gcmONERROR(Gal2DCleanSurface(t2d->runtime->hal, t2d->tmpSurf, COLOR_ARGB8(0x80, 0xff, 0xff, 0x0)));

    // prepare to draw line
    gcmONERROR(gco2D_ConstructSingleColorBrush(egn2D, gcvFALSE,
                                    COLOR_ARGB8(0, 0, 0, 0xff), 0, &brush));

    gcmONERROR(gco2D_SetColorSourceAdvanced(egn2D,
                    t2d->srcPhyAddr, t2d->srcStride, t2d->srcFormat,
                    gcvSURF_0_DEGREE, t2d->srcWidth, t2d->srcHeight, gcvFALSE));

    gcmONERROR(gco2D_SetTarget(egn2D, t2d->tmpPhyAddr, t2d->tmpStride, gcvSURF_0_DEGREE, t2d->tmpWidth));

    gcmONERROR(gco2D_SetClipping(egn2D, gcvNULL));

    switch (frameNo)
    {
        case 0:
            gcmONERROR(gco2D_EnableAlphaBlend(egn2D,
                0x80, 0x80,
                gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
                gcvSURF_GLOBAL_ALPHA_ON, gcvSURF_GLOBAL_ALPHA_ON,
                gcvSURF_BLEND_STRAIGHT, gcvSURF_BLEND_STRAIGHT,
                gcvSURF_COLOR_STRAIGHT, gcvSURF_COLOR_STRAIGHT));
            break;

        case 1:
            gcmONERROR(gco2D_EnableAlphaBlend(egn2D,
                0x0, 0x0,
                gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
                gcvSURF_GLOBAL_ALPHA_OFF, gcvSURF_GLOBAL_ALPHA_OFF,
                gcvSURF_BLEND_STRAIGHT, gcvSURF_BLEND_STRAIGHT,
                gcvSURF_COLOR_STRAIGHT, gcvSURF_COLOR_STRAIGHT));
            break;
    }

    for (i = 0; i < MAX_LINES; i++)
    {
        line.left = i;
        line.top = 7;
        line.right = 8+i;
        line.bottom = 64;

        gcmONERROR(gco2D_Line(egn2D, 1, &line, brush, 0xF0, 0xF0, t2d->tmpFormat));
    }

    gcmONERROR(gco2D_Flush(egn2D));
    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    gcmONERROR(gcoBRUSH_Destroy(brush));

    // blit to dest surface
    t2d->runtime->saveTarget = gcvTRUE;
    gcmONERROR(gco2D_DisableAlphaBlend(egn2D));

    gcmONERROR(gco2D_SetColorSourceAdvanced(egn2D,
        t2d->tmpPhyAddr,t2d->tmpStride, t2d->tmpFormat,
        gcvSURF_0_DEGREE,t2d->tmpWidth, t2d->tmpHeight, gcvFALSE));

    gcmONERROR(gco2D_SetSource(egn2D, &dstRect));

    gcmONERROR(gco2D_SetTarget(egn2D, t2d->dstPhyAddr, t2d->dstStride, gcvSURF_0_DEGREE, t2d->dstWidth));

    gcmONERROR(gco2D_SetClipping(egn2D, gcvNULL));

    gcmONERROR(gco2D_Blit(egn2D, 1, &dstRect, 0xCC, 0xCC, t2d->dstFormat));

    gcmONERROR(gco2D_Flush(egn2D));
    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    // destroy src surface
    if (t2d->srcSurf)
    {
        if (t2d->srcVirtAddr)
        {
            if (gcmIS_ERROR(gcoSURF_Unlock(t2d->srcSurf, t2d->srcVirtAddr)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock srcSurf failed:%s\n", GalStatusString(status));
            }
            t2d->srcVirtAddr = gcvNULL;
        }
        if (gcmIS_ERROR(gcoSURF_Destroy(t2d->srcSurf)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy srcSurf failed:%s\n", GalStatusString(status));
        }
        t2d->srcSurf = gcvNULL;
    }


    // destroy tmp surface
    if (t2d->tmpSurf)
    {
        if (t2d->tmpVirtAddr)
        {
            if (gcmIS_ERROR(gcoSURF_Unlock(t2d->tmpSurf, t2d->tmpVirtAddr)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock tmpSurf failed:%s\n", GalStatusString(status));
            }
            t2d->tmpVirtAddr = gcvNULL;
        }
        if (gcmIS_ERROR(gcoSURF_Destroy(t2d->tmpSurf)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy tmpSurf failed:%s\n", GalStatusString(status));
        }
        t2d->tmpSurf = gcvNULL;
    }

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

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_ANDROID_ONLY,
    gcvFEATURE_2D_A8_TARGET,
    gcvFEATURE_SEPARATE_SRC_DST,
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
    t2d->dstFormat = runtime->format;
    t2d->dstWidth = 0;
    t2d->dstHeight = 0;
    t2d->dstStride = 0;
    t2d->dstPhyAddr = 0;
    t2d->dstLgcAddr = 0;

    t2d->runtime->saveTarget = gcvFALSE;

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
