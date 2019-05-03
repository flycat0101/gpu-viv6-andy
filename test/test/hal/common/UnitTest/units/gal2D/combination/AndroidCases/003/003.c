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
 *  API:        gco2D_SetGenericSource gco2D_SetGenericTarget gco2D_MultiSourceBlit gco2D_Blit
 *  Check:
 */
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DAndroidCases003\n" \
"Operation: Test multi-src-blit & blit with rotation / alphablend.\n" \
"2D API: gco2D_SetGenericSource gco2D_SetGenericTarget gco2D_MultiSourceBlit\n" \
"Src1: Size       [980x1460]\n"\
"     Rect        [1,0,998,880]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [90]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src2: Size       [980x537]\n"\
"     Rect        [1,0,998,880]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [90]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Dst: Size        [1460x980]\n"\
"     Rect        [1,0,998,880]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"2D API: gco2D_SetGenericSource gco2D_SetGenericTarget gco2D_Blit\n" \
"Src: Size        [1080x1920]\n"\
"     Rect        [461,980,1458,1080 / 1458,0,1920,1080 / 461,0,1458,100 / 0,0,461,1080]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [90]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Dst: Size        [1920x1080]\n"\
"     Rect        [461,980,1458,1080 / 1458,0,1920,1080 / 461,0,1458,100 / 0,0,461,1080]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: [None]\n"\
"Alphablend: [enable]\n" \
"HW feature dependency: ";

#define TARGET_WIDTH        1920
#define TARGET_HEIGHT       1080

typedef struct Test2D
{
    GalTest         base;
    GalRuntime *    runtime;

    gcsRECT         surfRect[8];
    T2D_SURF_PTR    surf[8];

    gcsRECT         targetRect;
} Test2D;

static gceSTATUS InitSourceSurface(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT i;

    static char *srcImageFiles[] =
    {
        "resource/android/f479_l0_h0x323453f0_p602f6000_s1080x1920.bmp",
        "resource/android/f479_l2_h0x32344b48_p5e1be000_s880x997.bmp",
    };

    for (i = 0; i < (gctINT)gcmCOUNTOF(srcImageFiles); ++i)
    {
        gcmONERROR(GalLoadFileToTSurf(srcImageFiles[i], &t2d->surf[i]));

        t2d->surfRect[i].left   = 0;
        t2d->surfRect[i].top    = 0;
        t2d->surfRect[i].right  = t2d->surf[i]->width;
        t2d->surfRect[i].bottom = t2d->surf[i]->height;
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
                    "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));
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
    gctINT i, loop;
    T2D_SURF_PTR result = gcvNULL;
    gcsRECT rect;
    gctUINT32 newAddress[3];

    /* Init the source surface. */
    gcmONERROR(InitSourceSurface(t2d, frameNo));

    for (loop = 0; loop < 4; loop++)
    {
        /* Create the result surface. */
        gcmONERROR(GalCreateTSurf(
            t2d->runtime->hal, gcvSURF_A8R8G8B8, gcvLINEAR, gcv2D_TSC_DISABLE,
            TARGET_WIDTH, TARGET_HEIGHT, &result));

        rect.left   = 1;
        rect.top    = 0;
        rect.right  = 998;
        rect.bottom = 880;

        /* Set the source surface. */
        for (i = 0; i < (gctINT)gcmCOUNTOF(t2d->surf); i++)
        {
            if (t2d->surf[i] == gcvNULL)
            {
                continue;
            }

            gcmONERROR(gco2D_SetCurrentSourceIndex(egn2D, i));

            if (i == 0)
            {
                //newAddress[0] = t2d->surf[i]->address[0] - ((100 * t2d->surf[i]->stride[0]) + (461 * 4));
                //newAddress[0] = t2d->surf[i]->address[0] - ((461 * t2d->surf[i]->stride[0]) + (0 * 4));
                //newAddress[0] = t2d->surf[i]->address[0] + (460 * t2d->surf[i]->stride[0]);

                newAddress[0] = t2d->surf[i]->address[0] + (460 * t2d->surf[i]->stride[0]) /*+ 100 * 4*/;
                newAddress[1] = t2d->surf[i]->address[1];
                newAddress[2] = t2d->surf[i]->address[2];
                gcmONERROR(gco2D_DisableAlphaBlend(egn2D));

                gcmONERROR(gco2D_SetGenericSource(
                    egn2D,
                    newAddress, t2d->surf[i]->validAddressNum,
                    t2d->surf[i]->stride, t2d->surf[i]->validStrideNum,
                    t2d->surf[i]->tiling, t2d->surf[i]->format,
                    gcvSURF_90_DEGREE,
                    t2d->surf[i]->width - 100, t2d->surf[i]->height - 460));
            }
            else if (i == 1)
            {
                //newAddress[0] = t2d->surf[i]->address[0] - ((100 * t2d->surf[i]->stride[0]) + (461 * 4));
                //newAddress[0] = t2d->surf[i]->address[0] - ((461 * t2d->surf[i]->stride[0]) + (0 * 4));
                newAddress[0] = t2d->surf[i]->address[0] - ((1 * t2d->surf[i]->stride[0]) + (0 * 4));
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
                    gcvSURF_90_DEGREE,
                    //t2d->surf[i]->width + 100, t2d->surf[i]->height + 461));
                    t2d->surf[i]->width, t2d->surf[i]->height + 1));
            }

            gcmONERROR(gco2D_SetSourceTileStatus(
                egn2D,
                t2d->surf[i]->tileStatusConfig,
                t2d->surf[i]->format,
                t2d->surf[i]->tileStatusClear,
                t2d->surf[i]->tileStatusAddress
                ));

            gcmONERROR(gco2D_SetSource(egn2D, &rect));

            gcmONERROR(gco2D_SetROP(egn2D, 0xCC, 0xCC));
        }

        newAddress[0] = result->address[0] + (100 * result->stride[0]) + 460 * 4;
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
            result->width - 460,
            result->height - 100));

        gcmONERROR(gco2D_SetTargetTileStatus(
            egn2D,
            result->tileStatusConfig,
            result->format,
            gcvSURF_0_DEGREE,
            result->tileStatusAddress
            ));

        /*
         * Multi source blit.
         */
        gcmONERROR(gco2D_SetClipping(egn2D, &rect));

        gcmONERROR(gco2D_MultiSourceBlit(egn2D, 0x3, &rect, 1));

        /*
         * Blit
         */
        gcmONERROR(gco2D_SetCurrentSourceIndex(egn2D, 0));

        gcmONERROR(gco2D_SetGenericSource(
            egn2D,
            t2d->surf[0]->address, t2d->surf[0]->validAddressNum,
            t2d->surf[0]->stride, t2d->surf[0]->validStrideNum,
            t2d->surf[0]->tiling, t2d->surf[0]->format,
            gcvSURF_90_DEGREE,
            t2d->surf[0]->width, t2d->surf[0]->height));

        gcmONERROR(gco2D_SetGenericTarget(
            egn2D,
            result->address,
            result->validAddressNum,
            result->stride,
            result->validStrideNum,
            result->tiling,
            result->format,
            result->rotation,
            result->width,
            result->height));

        gcmONERROR(gco2D_SetClipping(egn2D, &t2d->targetRect));

        rect.left   = 461;
        rect.top    = 980;
        rect.right  = 1458;
        rect.bottom = 1080;
        gcmONERROR(gco2D_SetSource(egn2D, &rect));
        gcmONERROR(gco2D_Blit(egn2D, 1, &rect, 0xCC, 0xCC, result->format));

        rect.left   = 1458;
        rect.top    = 0;
        rect.right  = 1920;
        rect.bottom = 1080;
        gcmONERROR(gco2D_SetSource(egn2D, &rect));
        gcmONERROR(gco2D_Blit(egn2D, 1, &rect, 0xCC, 0xCC, result->format));

        rect.left   = 461;
        rect.top    = 0;
        rect.right  = 1458;
        rect.bottom = 100;
        gcmONERROR(gco2D_SetSource(egn2D, &rect));
        gcmONERROR(gco2D_Blit(egn2D, 1, &rect, 0xCC, 0xCC, result->format));

        rect.left   = 0;
        rect.top    = 0;
        rect.right  = 461;
        rect.bottom = 1080;
        gcmONERROR(gco2D_SetSource(egn2D, &rect));
        gcmONERROR(gco2D_Blit(egn2D, 1, &rect, 0xCC, 0xCC, result->format));

        gcmONERROR(gco2D_Flush(egn2D));

        gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

        /* Save the result. */
        if (t2d->runtime->saveFullName)
        {
            char buff[128];
            sprintf(buff, "%s_%02d.bmp", t2d->runtime->saveFullName, loop);
            GalSaveTSurfToDIB(result, buff);
        }

        if (result != gcvNULL)
        {
            GalDeleteTSurf(gcvNULL, result);
            result = NULL;
        }
    }

OnError:
    if (result != gcvNULL)
    {
        GalDeleteTSurf(gcvNULL, result);
    }

    DestroySourceSurface(t2d);

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
    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_2D_MULTI_SOURCE_BLT,
    gcvFEATURE_2D_MULTI_SOURCE_BLT_EX,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status = gcvSTATUS_OK;
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
    t2d->base.frameCount   = 1;
    t2d->base.description  = s_CaseDescription;

    return gcvTRUE;

OnError:
    GalOutput(GalOutputType_Error | GalOutputType_Console,
        "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));
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
