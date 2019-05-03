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
 *  Feature:    Stretch Blit - engamma - degamma
 *  API:        gco2D_StretchBlit gco2D_SetStateU32 gco2D_SetStateArrayU32
 *  Check:
*/
#include <math.h>
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DGamma002\n" \
"Operation: Test stretch with engamma vary different mode.\n" \
"2D API: gco2D_SetStateU32\n" \
"Src: Size        [400x400]\n"\
"     Rect        [0,0,400,400]\n"\
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
"Gamma: [enable]\n" \
"HW feature dependency: ";

typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;

    // destination surface
    gcoSURF           dstSurf;
    gceSURF_FORMAT    dstFormat;
    gctUINT           dstWidth;
    gctUINT           dstHeight;
    gctINT            dstStride;
    gctUINT32         dstPhyAddr;
    gctPOINTER        dstLgcAddr;

    gctUINT32         EnGammaLUT[256];
    gctUINT           EnGammaMode;
    //source surface
    gcoSURF           srcSurf;
    gceSURF_FORMAT    srcFormat;
    gctUINT           srcWidth;
    gctUINT           srcHeight;
    gctINT            srcStride;
    gctUINT32         srcPhyAddr;
    gctPOINTER        srcLgcAddr;
} Test2D;

typedef enum _gce2D_ENGAMMA_MODE
{
    gcv2D_ENGAMMA_RED       = 0x1,
    gcv2D_ENGAMMA_GREEN     = 0x2,
    gcv2D_ENGAMMA_BLUE      = 0x4,
    gcv2D_ENGAMMA_RED_BLUE  = 0x8,
    gcv2D_ENGAMMA_RED_GREEN = 0x10,
    gcv2D_ENGAMMA_GREEN_BLUE= 0x20,
    gcv2D_ENGAMMA_ALL       = 0x40
}
gce2D_ENGAMMA_MODE;

gceSTATUS BuildEnGammaLUT(Test2D *t2d, gctUINT EnGammaMode)
{
    gctUINT32 i;
    gceSTATUS status = gcvSTATUS_OK;
    unsigned char tempRed, tempGreen, tempBlue,temp;
    float tmpf;
    for(i=0; i<256; i++)
    {
        t2d->EnGammaLUT[i] = 0;
        tmpf = (i + 0.5f)/256.0f;
        tmpf = (float)(pow((double)tmpf, (double)0.45));
        temp = (unsigned char)(tmpf*256 - 0.5f);

        if(EnGammaMode & gcv2D_ENGAMMA_RED)
        {
            tempRed = temp;
            tempGreen = i;
            tempBlue = i;
        }
        else if(EnGammaMode & gcv2D_ENGAMMA_GREEN)
        {
            tempGreen = temp;
            tempRed= i;
            tempBlue = i;
        }
        else if(EnGammaMode & gcv2D_ENGAMMA_BLUE)
        {
            tempBlue = temp;
            tempGreen = i;
            tempRed = i;
        }
        else if(EnGammaMode & gcv2D_ENGAMMA_RED_BLUE)
        {
            tempRed = temp;
            tempGreen = i;
            tempBlue = temp;
        }
        else if(EnGammaMode & gcv2D_ENGAMMA_RED_GREEN)
        {
            tempRed = temp;
            tempGreen = temp;
            tempBlue = i;
        }
        else if(EnGammaMode & gcv2D_ENGAMMA_GREEN_BLUE)
        {
            tempRed = i;
            tempGreen = temp;
            tempBlue = temp;
        }
        else if(EnGammaMode & gcv2D_ENGAMMA_ALL)
        {
            tempRed = temp;
            tempGreen = temp;
            tempBlue = temp;
        }
        else
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }
        t2d->EnGammaLUT[i] = ((tempBlue<<22) | (tempGreen<<12) |(tempRed<<2));
    }

OnError:
    return status;
}

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gctUINT32 horFactor, verFactor;
    gcsRECT srcRect;
    gco2D egn2D = t2d->runtime->engine2d;
    gceSTATUS status;
    gctBOOL setValue = gcvTRUE;
    gctBOOL clearValue = gcvFALSE;
    gcsRECT dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};

    // create source surface
    if(frameNo == 0)
    {
        t2d->EnGammaMode = gcv2D_ENGAMMA_RED;
    }
    else if(frameNo == 1)
    {
        t2d->EnGammaMode = gcv2D_ENGAMMA_GREEN;
    }
    else if(frameNo == 2)
    {
        t2d->EnGammaMode = gcv2D_ENGAMMA_BLUE;
    }
    else if(frameNo == 3)
    {
        t2d->EnGammaMode = gcv2D_ENGAMMA_RED_BLUE;
    }
    else if(frameNo == 4)
    {
        t2d->EnGammaMode = gcv2D_ENGAMMA_RED_GREEN;
    }
    else if(frameNo == 5)
    {
        t2d->EnGammaMode = gcv2D_ENGAMMA_GREEN_BLUE;
    }
    else if(frameNo == 6)
    {
        t2d->EnGammaMode = gcv2D_ENGAMMA_ALL;
    }
    BuildEnGammaLUT(t2d, t2d->EnGammaMode);

    gcmONERROR(gco2D_SetStateU32(egn2D, gcv2D_STATE_EN_GAMMA, setValue));
    gcmONERROR(gco2D_SetStateArrayU32(egn2D, gcv2D_STATE_ARRAY_EN_GAMMA, t2d->EnGammaLUT, 256));

    srcRect.left = 0;
    srcRect.top = 0;
    srcRect.right = t2d->srcWidth;
    srcRect.bottom = t2d->srcHeight;

    // set source color and rect
    gcmONERROR(gco2D_SetColorSource(egn2D, t2d->srcPhyAddr, t2d->srcStride, t2d->srcFormat,
                    gcvSURF_0_DEGREE, t2d->srcWidth, gcvFALSE, gcvSURF_OPAQUE, 0));

    gcmONERROR(gco2D_SetSource(egn2D, &srcRect));

    // set dst and clippint rect
    gcmONERROR(gco2D_SetTarget(egn2D, t2d->dstPhyAddr, t2d->dstStride, gcvSURF_0_DEGREE, t2d->dstWidth));

    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    /* Calculate the stretch factors. */
    gcmONERROR(gco2D_CalcStretchFactor(egn2D, srcRect.right - srcRect.left,
        dstRect.right - dstRect.left, &horFactor));

    gcmONERROR(gco2D_CalcStretchFactor(egn2D, srcRect.bottom - srcRect.top,
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

    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_2D_GAMMA,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status = gcvSTATUS_OK;
    char * sourcefile = "resource/source.bmp";

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
    t2d->srcFormat  = gcvSURF_UNKNOWN;

    t2d->srcSurf = GalLoadDIB2Surface(t2d->runtime->hal, sourcefile);
    if (t2d->srcSurf == NULL)
    {
        GalOutput(GalOutputType_Error, "can not load %s\n", sourcefile);
        gcmONERROR(gcvSTATUS_NOT_FOUND);
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

    gcmONERROR(gcoSURF_Lock(t2d->srcSurf, &t2d->srcPhyAddr, &t2d->srcLgcAddr));

    // dst with dst surf
    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = 7;
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
