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
 *  Feature:    multi source blit
 *  API:        gco2D_SetGenericSource gco2D_SetGenericTarget gco2D_MultiSourceBlit
 *  Check:
 */
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DAndroidCases004\n" \
"Operation: Test multi-src-blit with pattern.\n" \
"2D API: gco2D_SetGenericSource gco2D_SetGenericTarget gco2D_MultiSourceBlit\n" \
"Src1: Size       [810x1080]\n"\
"     Rect        [0,0,810,746 / 2,0,556,1080 / 0,0,810,334 / 0,0,556,1080]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src2: Size       [1364x646 / 556x1080 / 1364x1080 / 1920x1080]\n"\
"     Rect        [0,0,810,746 / 2,0,556,1080 / 0,0,810,334 / 0,0,556,1080]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src3: Size       [1364x646 / 556x1080 / 1364x1080 / 1920x1080]\n"\
"     Rect        [0,0,810,746 / 2,0,556,1080 / 0,0,810,334 / 0,0,556,1080]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Dst: Size        [1364x746 / 556x1080 / 1364x1080 / 1920x1080]\n"\
"     Rect        [0,0,810,746 / 2,0,556,1080 / 0,0,810,334 / 0,0,556,1080]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: Type   [SolidBrush]\n"\
"       Format [ARGB8888]\n"\
"       Offset [0]\n" \
"Alphablend: [enable]\n" \
"HW feature dependency: ";

#define TARGET_WIDTH        1920
#define TARGET_HEIGHT       1080

typedef struct Test2D
{
    GalTest         base;
    GalRuntime *    runtime;

    T2D_SURF_PTR    surf[8];

    T2D_SURF_PTR    result;

    gcsRECT         targetRect;
} Test2D;

static gceSTATUS InitSourceSurface(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT i;

    static char *srcImageFiles[] =
    {
        "nul",
        "resource/android/f400_l1_h0x3240fee8_p62b0e000_s1920x1080.bmp",
        "resource/android/f400_l2_h0x324103b0_p61ab1000_s1920x1080.bmp",
    };

    /* Layer 0. */
    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal, gcvSURF_A8R8G8B8, gcvLINEAR, gcv2D_TSC_DISABLE,
        810, 1080, &t2d->surf[0]));

    /* Other layers. */
    for (i = 1; i < (gctINT)gcmCOUNTOF(srcImageFiles); ++i)
    {
        gcmONERROR(GalLoadFileToTSurf(srcImageFiles[i], &t2d->surf[i]));
    }

OnError:
    return status;
}

static gceSTATUS DestroySourceSurface(Test2D *t2d)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT i;

    /* Destroy source surface. */
    for (i = 0; i < (gctINT)gcmCOUNTOF(t2d->surf); ++i)
    {
        if (t2d->surf[i] != gcvNULL)
        {
            status = GalDeleteTSurf(t2d->runtime->hal, t2d->surf[i]);
            if (gcmIS_ERROR(status))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console,
                    "%s(%d) failed:%s\n",__FUNCTION__, __LINE__,
                    gcoOS_DebugStatus2Name(status));
            }
            t2d->surf[i] = gcvNULL;
        }
    }

    return status;
}

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status;
    gco2D egn2D = t2d->runtime->engine2d;
    T2D_SURF_PTR result = t2d->result;
    gcsRECT rect;
    gctUINT32 newAddress[3], offset;
    gctINT i;

    /* Clear the target. */
    memset(result->vNode.memory, 0, result->vNode.size);

    /*
    MULTI-SOURCE: eigenRect[0,0,0,0] blitRect[0,0,810,746]
        ClipRect[556,334,1366,1080] rot=0 dx=556,dy=334: 0x5c27e000->0x5c4f0cb0
        layer[0]: color32=0x00000000
        layer[1]: (format=306,alpha=255,rot=0,mirror=0,0) dx=556,dy=334: 638c2000->63b34cb0
        layer[2]: (format=306,alpha=255,rot=0,mirror=0,0) dx=556,dy=334: 60adf000->60d51cb0
    */

    rect.left   = 0;
    rect.top    = 0;
    rect.right  = 810;
    rect.bottom = 746;

    for (i = 0; i < (gctINT)gcmCOUNTOF(t2d->surf); i++)
    {
        if (t2d->surf[i] == gcvNULL)
        {
            continue;
        }

        gcmONERROR(gco2D_SetCurrentSourceIndex(egn2D, i));

        if (i == 0)
        {
            gcmONERROR(gco2D_DisableAlphaBlend(egn2D));

            gcmONERROR(gco2D_SetGenericSource(
                egn2D,
                t2d->surf[i]->address, t2d->surf[i]->validAddressNum,
                t2d->surf[i]->stride, t2d->surf[i]->validStrideNum,
                t2d->surf[i]->tiling, t2d->surf[i]->format,
                gcvSURF_0_DEGREE,
                t2d->surf[i]->width, t2d->surf[i]->height));

            gcmONERROR(
                    gco2D_LoadSolidBrush(egn2D,
                        /* This should not be taken. */
                        gcvSURF_UNKNOWN,
                        gcvTRUE,
                        0x0,
                        0U));

            /* Set ROP: use pattern only. */
            gcmONERROR(gco2D_SetROP(egn2D, 0xFC, 0xFC));
        }
        else if ((i == 1) || (i == 2))
        {
            offset = (334 * t2d->surf[i]->stride[0]) +
                     (556 * (t2d->surf[i]->stride[0] / t2d->surf[i]->aWidth));

            newAddress[0] = t2d->surf[i]->address[0] + offset;
            newAddress[1] = t2d->surf[i]->address[1];
            newAddress[2] = t2d->surf[i]->address[2];

            /* Cs + (1 - As) x Cd */
            gcmONERROR(gco2D_EnableAlphaBlend(egn2D,
                        0, 0,
                        gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
                        gcvSURF_GLOBAL_ALPHA_OFF, gcvSURF_GLOBAL_ALPHA_OFF,
                        gcvSURF_BLEND_ONE, gcvSURF_BLEND_INVERSED,
                        gcvSURF_COLOR_STRAIGHT, gcvSURF_COLOR_STRAIGHT));

            gcmONERROR(gco2D_SetGenericSource(
                egn2D,
                newAddress, t2d->surf[i]->validAddressNum,
                t2d->surf[i]->stride, t2d->surf[i]->validStrideNum,
                t2d->surf[i]->tiling, t2d->surf[i]->format,
                gcvSURF_0_DEGREE,
                t2d->surf[i]->width - 556, t2d->surf[i]->height - 334));

            gcmONERROR(gco2D_SetROP(egn2D, 0xCC, 0xCC));
        }

        gcmONERROR(gco2D_SetSource(egn2D, &rect));
    }

    offset = (334 * result->stride[0]) +
             (556 * (result->stride[0] / result->aWidth));

    newAddress[0] = result->address[0] + offset;
    newAddress[1] = result->address[1];
    newAddress[2] = result->address[2];

    /* Set the target surface. */
    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        newAddress,
        result->validAddressNum,
        result->stride,
        result->validStrideNum,
        result->tiling,
        result->format,
        result->rotation,
        result->width - 556,
        result->height - 334));

    gcmONERROR(gco2D_SetClipping(egn2D, &rect));

    gcmONERROR(gco2D_MultiSourceBlit(egn2D, 0x7, &rect, 1));

    /*
    MULTI-SOURCE: eigenRect[2,0,0,0] blitRect[2,0,556,1080]
     ClipRect[1366,0,1920,1080] rot=0 dx=1364,dy=0: 0x5c27e000->0x5c27f550
     layer[1]: (format=306,alpha=255,rot=0,mirror=0,0) dx=1364,dy=0: 638c2000->638c3550
     layer[2]: (format=306,alpha=255,rot=0,mirror=0,0) dx=1364,dy=0: 60adf000->60ae0550
    */

    rect.left   = 2;
    rect.top    = 0;
    rect.right  = 556;
    rect.bottom = 1080;

    for (i = 0; i < (gctINT)gcmCOUNTOF(t2d->surf); i++)
    {
        if (t2d->surf[i] == gcvNULL)
        {
            continue;
        }

        gcmONERROR(gco2D_SetCurrentSourceIndex(egn2D, i));

        if (i == 0)
        {
            continue;
        }
        else if ((i == 1) || (i == 2))
        {
            offset = 1364 * (t2d->surf[i]->stride[0] / t2d->surf[i]->aWidth);

            newAddress[0] = t2d->surf[i]->address[0] + offset;
            newAddress[1] = t2d->surf[i]->address[1];
            newAddress[2] = t2d->surf[i]->address[2];

            /* Cs + (1 - As) x Cd */
            gcmONERROR(gco2D_EnableAlphaBlend(egn2D,
                        0, 0,
                        gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
                        gcvSURF_GLOBAL_ALPHA_OFF, gcvSURF_GLOBAL_ALPHA_OFF,
                        gcvSURF_BLEND_ONE, gcvSURF_BLEND_INVERSED,
                        gcvSURF_COLOR_STRAIGHT, gcvSURF_COLOR_STRAIGHT));

            gcmONERROR(gco2D_SetGenericSource(
                egn2D,
                newAddress, t2d->surf[i]->validAddressNum,
                t2d->surf[i]->stride, t2d->surf[i]->validStrideNum,
                t2d->surf[i]->tiling, t2d->surf[i]->format,
                gcvSURF_0_DEGREE,
                t2d->surf[i]->width - 1364, t2d->surf[i]->height));
        }

        gcmONERROR(gco2D_SetSource(egn2D, &rect));

        gcmONERROR(gco2D_SetROP(egn2D, 0xCC, 0xCC));
    }

    offset = 1364 * (result->stride[0] / result->aWidth);

    newAddress[0] = result->address[0] + offset;
    newAddress[1] = result->address[1];
    newAddress[2] = result->address[2];

    /* Set the target surface. */
    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        newAddress,
        result->validAddressNum,
        result->stride,
        result->validStrideNum,
        result->tiling,
        result->format,
        result->rotation,
        result->width - 1364,
        result->height));

    gcmONERROR(gco2D_SetClipping(egn2D, &rect));

    gcmONERROR(gco2D_MultiSourceBlit(egn2D, 0x6, &rect, 1));

    /*
    MULTI-SOURCE: eigenRect[0,0,0,0] blitRect[0,0,810,334]
     ClipRect[556,0,1366,334] rot=0 dx=556,dy=0: 0x5c27e000->0x5c27e8b0
     layer[1]: (format=306,alpha=255,rot=0,mirror=0,0) dx=556,dy=0: 638c2000->638c28b0
     layer[2]: (format=306,alpha=255,rot=0,mirror=0,0) dx=556,dy=0: 60adf000->60adf8b0
    */

    rect.left   = 0;
    rect.top    = 0;
    rect.right  = 810;
    rect.bottom = 334;

    for (i = 0; i < (gctINT)gcmCOUNTOF(t2d->surf); i++)
    {
        if (t2d->surf[i] == gcvNULL)
        {
            continue;
        }

        gcmONERROR(gco2D_SetCurrentSourceIndex(egn2D, i));

        if (i == 0)
        {
            continue;
        }
        else if ((i == 1) || (i == 2))
        {
            offset = 556 * (t2d->surf[i]->stride[0] / t2d->surf[i]->aWidth);

            newAddress[0] = t2d->surf[i]->address[0] + offset;
            newAddress[1] = t2d->surf[i]->address[1];
            newAddress[2] = t2d->surf[i]->address[2];

            /* Cs + (1 - As) x Cd */
            gcmONERROR(gco2D_EnableAlphaBlend(egn2D,
                        0, 0,
                        gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
                        gcvSURF_GLOBAL_ALPHA_OFF, gcvSURF_GLOBAL_ALPHA_OFF,
                        gcvSURF_BLEND_ONE, gcvSURF_BLEND_INVERSED,
                        gcvSURF_COLOR_STRAIGHT, gcvSURF_COLOR_STRAIGHT));

            gcmONERROR(gco2D_SetGenericSource(
                egn2D,
                newAddress, t2d->surf[i]->validAddressNum,
                t2d->surf[i]->stride, t2d->surf[i]->validStrideNum,
                t2d->surf[i]->tiling, t2d->surf[i]->format,
                gcvSURF_0_DEGREE,
                t2d->surf[i]->width - 556, t2d->surf[i]->height));
        }

        gcmONERROR(gco2D_SetSource(egn2D, &rect));

        gcmONERROR(gco2D_SetROP(egn2D, 0xCC, 0xCC));
    }

    offset = 556 * (result->stride[0] / result->aWidth);

    newAddress[0] = result->address[0] + offset;
    newAddress[1] = result->address[1];
    newAddress[2] = result->address[2];

    /* Set the target surface. */
    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        newAddress,
        result->validAddressNum,
        result->stride,
        result->validStrideNum,
        result->tiling,
        result->format,
        result->rotation,
        result->width - 556,
        result->height));

    gcmONERROR(gco2D_SetClipping(egn2D, &rect));

    gcmONERROR(gco2D_MultiSourceBlit(egn2D, 0x6, &rect, 1));

    /*
    MULTI-SOURCE: eigenRect[0,0,0,0] blitRect[0,0,556,1080]
     ClipRect[0,0,556,1080] rot=0 dx=0,dy=0: 0x5c27e000->0x5c27e000
     layer[1]: (format=306,alpha=255,rot=0,mirror=0,0) dx=0,dy=0: 638c2000->638c2000
     layer[2]: (format=306,alpha=255,rot=0,mirror=0,0) dx=0,dy=0: 60adf000->60adf000
    */

    rect.left   = 0;
    rect.top    = 0;
    rect.right  = 556;
    rect.bottom = 1080;

    for (i = 0; i < (gctINT)gcmCOUNTOF(t2d->surf); i++)
    {
        if (t2d->surf[i] == gcvNULL)
        {
            continue;
        }

        gcmONERROR(gco2D_SetCurrentSourceIndex(egn2D, i));

        if (i == 0)
        {
            continue;
        }
        else if ((i == 1) || (i == 2))
        {
            offset = 0 * (t2d->surf[i]->stride[0] / t2d->surf[i]->aWidth);

            newAddress[0] = t2d->surf[i]->address[0] + offset;
            newAddress[1] = t2d->surf[i]->address[1];
            newAddress[2] = t2d->surf[i]->address[2];

            /* Cs + (1 - As) x Cd */
            gcmONERROR(gco2D_EnableAlphaBlend(egn2D,
                        0, 0,
                        gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
                        gcvSURF_GLOBAL_ALPHA_OFF, gcvSURF_GLOBAL_ALPHA_OFF,
                        gcvSURF_BLEND_ONE, gcvSURF_BLEND_INVERSED,
                        gcvSURF_COLOR_STRAIGHT, gcvSURF_COLOR_STRAIGHT));

            gcmONERROR(gco2D_SetGenericSource(
                egn2D,
                newAddress, t2d->surf[i]->validAddressNum,
                t2d->surf[i]->stride, t2d->surf[i]->validStrideNum,
                t2d->surf[i]->tiling, t2d->surf[i]->format,
                gcvSURF_0_DEGREE,
                t2d->surf[i]->width - 0, t2d->surf[i]->height));
        }

        gcmONERROR(gco2D_SetSource(egn2D, &rect));

        gcmONERROR(gco2D_SetROP(egn2D, 0xCC, 0xCC));
    }

    offset = 0 * (result->stride[0] / result->aWidth);

    newAddress[0] = result->address[0] + offset;
    newAddress[1] = result->address[1];
    newAddress[2] = result->address[2];

    /* Set the target surface. */
    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        newAddress,
        result->validAddressNum,
        result->stride,
        result->validStrideNum,
        result->tiling,
        result->format,
        result->rotation,
        result->width - 0,
        result->height));

    gcmONERROR(gco2D_SetClipping(egn2D, &rect));

    gcmONERROR(gco2D_MultiSourceBlit(egn2D, 0x6, &rect, 1));

     /* Commit. */
    gcmONERROR(gco2D_Flush(egn2D));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    /* Save the result. */
    if (t2d->runtime->saveFullName)
    {
        GalSaveTSurfToDIB(result, t2d->runtime->saveFullName);
    }

OnError:
    if (status != gcvSTATUS_OK)
    {
        GalOutput(GalOutputType_Error | GalOutputType_Console,
        "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));

        return gcvFALSE;
    }
    else
    {
        return gcvTRUE;
    }
}

static void CDECL Destroy(Test2D *t2d)
{
    if (t2d->result != gcvNULL)
    {
        GalDeleteTSurf(gcvNULL, t2d->result);
    }

    DestroySourceSurface(t2d);

    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_2D_MULTI_SOURCE_BLT,
    gcvFEATURE_2D_MULTI_SOURCE_BLT_EX,
    gcvFEATURE_ANDROID_ONLY,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status;
    gctINT argc     = runtime->argc;
    gctSTRING *argv = runtime->argv;

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

    t2d->runtime           = runtime;

    runtime->saveTarget    = gcvFALSE;
    runtime->cleanTarget   = gcvFALSE;

    t2d->targetRect.left   = 0;
    t2d->targetRect.top    = 0;
    t2d->targetRect.right  = TARGET_WIDTH;
    t2d->targetRect.bottom = TARGET_HEIGHT;

    t2d->base.render       = (PGalRender)Render;
    t2d->base.destroy      = (PGalDestroy)Destroy;
    t2d->base.frameCount   = 4;
    t2d->base.description  = s_CaseDescription;

    /* Init the source surface. */
    gcmONERROR(InitSourceSurface(t2d, 0));

    /* Create the result surface. */
    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal, gcvSURF_A8R8G8B8, gcvLINEAR, gcv2D_TSC_DISABLE,
        TARGET_WIDTH, TARGET_HEIGHT, &t2d->result));

    return gcvTRUE;

OnError:
    GalOutput(GalOutputType_Error | GalOutputType_Console, "%s(%d) failed: %s\n",
        __FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));

    return gcvFALSE;
}

GalTest * CDECL GalCreateTestObject(GalRuntime *runtime)
{
    Test2D *t2d = (Test2D *)malloc(sizeof(Test2D));

    memset(t2d, 0, sizeof(Test2D));

    if (!Init(t2d, runtime))
    {
        free(t2d);
        return NULL;
    }

    return &t2d->base;
}
