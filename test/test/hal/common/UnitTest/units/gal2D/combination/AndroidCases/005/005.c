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
 *  Feature: Rotation Cliping Alphablending Blit
 *  API:     gco2D_SetGenericSource gco2D_SetGenericTarget gco2D_Blit
 *  Check:
 */
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DAndroidCases005\n" \
"Operation: Use blit to simulate the multi-src-blit and rotation.\n" \
"2D API: gco2D_SetGenericSource gco2D_SetGenericTarget gco2D_MultiSourceBlit\n" \
"Src1: Size       [1280x1280]\n"\
"     Rect        [33,240,1040,1280]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [90]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src2: Size       [800x1280]\n"\
"     Rect        [33,0,800,1280]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [90]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src3: Size       [800x33]\n"\
"     Rect        [0,0,800,33]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [90]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src4: Size       [800x1280]\n"\
"     Rect        [0,0,800,1280]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [90]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Dst: Size        [1280x800]\n"\
"     Rect        [33,0,800,1280 / 0,0,33,800 / 0,0,1280,800]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: [None]\n"\
"Alphablend: [enable]\n" \
"HW feature dependency: ";

#define TARGET_WIDTH        1280
#define TARGET_HEIGHT       800

typedef struct Test2D
{
    GalTest         base;
    GalRuntime *    runtime;

    T2D_SURF_PTR    surf[8];
    T2D_SURF_PTR    result;

    gcsRECT         targetRect;
}
Test2D;

static gceSTATUS InitSourceSurface(Test2D *t2d)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT i;

    /* source pictures. */
    static char *srcImageFiles[] =
    {
        "resource/android/f259_l0_h0x4a5940d0_p38de0000_s1280x1280.bmp",
        "resource/android/f259_l1_h0x4a3f67e8_p3885bd40_s800x1280.bmp",
        "resource/android/f259_l2_h0x4a593930_p36d4a000_s800x33.bmp",
        "resource/android/f259_l3_h0x4a3f5fa0_p3841bc00_s800x1280.bmp",
    };

    /* Init all the source surface. */
    for (i = 0; i < gcmCOUNTOF(srcImageFiles); ++i)
    {
        GalOutput(GalOutputType_Console, "work around for gc520\n");
        gcmONERROR(GalLoadFileToTSurf(srcImageFiles[i], &t2d->surf[i]));
        GalOutput(GalOutputType_Console, "work around for gc520\n");
    }

OnError:
    return status;
}

static gceSTATUS DestroySourceSurface(Test2D *t2d)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT i;

    /* Destroy all source surface. */
    for (i = 0; i < gcmCOUNTOF(t2d->surf); ++i)
    {
        if (t2d->surf[i] != gcvNULL)
        {
            status = GalDeleteTSurf(t2d->runtime->hal, t2d->surf[i]);
            if (gcmIS_ERROR(status))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console,
                    "%s(%d): i=%d, failed:%s\n",
                    __FUNCTION__, __LINE__, i, gcoOS_DebugStatus2Name(status));
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
    gcsRECT rect, targetRect;
    gctUINT i;

    /* Clean the result surface. */
    memset(t2d->result->vNode.memory, 0x0, t2d->result->vNode.size);

    for (i = 0; i < (gctINT)gcmCOUNTOF(t2d->surf); i++)
    {
        if (t2d->surf[i] == gcvNULL)
        {
            continue;
        }

        gcmONERROR(gco2D_SetCurrentSourceIndex(egn2D, 0));

        /* Cs + (1 - As) x Cd */
        gcmONERROR(gco2D_EnableAlphaBlend(egn2D,
                    0, 0,
                    gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
                    gcvSURF_GLOBAL_ALPHA_OFF, gcvSURF_GLOBAL_ALPHA_OFF,
                    gcvSURF_BLEND_ONE, gcvSURF_BLEND_INVERSED,
                    gcvSURF_COLOR_STRAIGHT, gcvSURF_COLOR_STRAIGHT));

        gcmONERROR(gco2D_SetGenericSource(
            egn2D,
            t2d->surf[i]->address, t2d->surf[i]->validAddressNum,
            t2d->surf[i]->stride, t2d->surf[i]->validStrideNum,
            t2d->surf[i]->tiling, t2d->surf[i]->format,
            gcvSURF_90_DEGREE,
            t2d->surf[i]->width, t2d->surf[i]->height));

        gcmONERROR(gco2D_SetGenericTarget(
            egn2D,
            result->address,
            result->validAddressNum,
            result->stride,
            result->validStrideNum,
            result->tiling,
            result->format,
            gcvSURF_0_DEGREE,
            result->width,
            result->height));

        if (i == 0)
        {
            //rect.left         = 240;
            //rect.top          = 33;
            rect.left         = 33;
            rect.top          = 240;

            rect.right        = 1040;
            rect.bottom       = 1280;

            targetRect.left   = 33;
            targetRect.top    = 0;
            targetRect.right  = 1280;
            targetRect.bottom = 800;
        }
        else if (i == 1)
        {
            //rect.left         = 0;
            //rect.top          = 33;
            rect.left         = 33;
            rect.top          = 0;

            rect.right        = 800;
            rect.bottom       = 1280;

            targetRect.left   = 33;
            targetRect.top    = 0;
            targetRect.right  = 1280;
            targetRect.bottom = 800;
        }
        else if (i == 2)
        {
            rect.left         = 0;
            rect.top          = 0;
            rect.right        = 800;
            rect.bottom       = 33;

            targetRect.left   = 0;
            targetRect.top    = 0;
            targetRect.right  = 33;
            targetRect.bottom = 800;
        }
        else if (i == 3)
        {
            rect.left         = 0;
            rect.top          = 0;
            rect.right        = 800;
            rect.bottom       = 1280;

            targetRect.left   = 0;
            targetRect.top    = 0;
            targetRect.right  = 1280;
            targetRect.bottom = 800;
        }

        gcmONERROR(gco2D_SetSource(egn2D, &rect));

        gcmONERROR(gco2D_SetClipping(egn2D, &targetRect));

        gcmONERROR(gco2D_SetBitBlitMirror(egn2D, gcvFALSE, gcvFALSE));

        gcmONERROR(gco2D_Blit(egn2D, 1, &targetRect, 0xCC, 0xCC, result->format));
    }

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
            "%s(%d) failed:%s\n",
            __FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));

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
    gcvFEATURE_2D_BITBLIT_FULLROTATION,
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

    t2d->runtime           = runtime;

    runtime->saveTarget    = gcvFALSE;
    runtime->cleanTarget   = gcvFALSE;

    t2d->targetRect.left   = 0;
    t2d->targetRect.top    = 0;
    t2d->targetRect.right  = TARGET_WIDTH;
    t2d->targetRect.bottom = TARGET_HEIGHT;

    t2d->base.render       = (PGalRender)Render;
    t2d->base.destroy      = (PGalDestroy)Destroy;
    t2d->base.frameCount   = 1;
    t2d->base.description  = s_CaseDescription;

    /* Init the source surface. */
    gcmONERROR(InitSourceSurface(t2d));

    /* Create the result surface. */
    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal, gcvSURF_A8R8G8B8, gcvLINEAR, gcv2D_TSC_DISABLE,
        TARGET_WIDTH, TARGET_HEIGHT, &t2d->result));

    return gcvTRUE;

OnError:
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
