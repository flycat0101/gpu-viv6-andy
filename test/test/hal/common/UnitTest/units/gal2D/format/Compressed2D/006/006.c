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
 *  Feature:    multi, super, tiled input
 *  API:        gco2D_SetGenericSource gco2D_SetGenericTarget gco2D_Blit
 *  Check:
 */
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFormatCompressed2D006\n" \
"Operation: multi-src blit/rotation/alphablend/mirror: from mixed 2D compressed to non compressed.\n" \
"2D API: gco2D_SetSourceTileStatus gco2D_SetGenericSource gco2D_SetBitBlitMirror gco2D_MultiSourceBlit\n" \
"Src1: Size       [640x480]\n"\
"     Rect        [0,0,640,480]\n"\
"     Format      [YUY2]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src2: Size       [640x480]\n"\
"     Rect        [0,0,640,480]\n"\
"     Format      [INDEX8]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src3: Size       [480x480]\n"\
"     Rect        [0,0,480,480]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src4: Size       [480x480]\n"\
"     Rect        [0,0,480,480]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src5: Size       [480x480]\n"\
"     Rect        [0,0,480,480]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [2D compressed]\n" \
"Src6: Size       [480x480]\n"\
"     Rect        [0,0,480,480]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [2D compressed]\n" \
"Src7: Size       [480x480]\n"\
"     Rect        [0,0,480,480]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [2D compressed]\n" \
"Src8: Size       [480x480]\n"\
"     Rect        [0,0,480,480]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [2D compressed]\n" \
"Dst: Size        [480x480]\n"\
"     Rect        [0,0,480,480]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: Type   [SolidBrush]\n"\
"       Format [ARGB8888]\n"\
"       Offset [0]\n" \
"Alphablend: [disable/enable]\n" \
"HW feature dependency: gcvFEATURE_ANDROID_ONLY gcvFEATURE_2D_MULTI_SRC_BLT_TO_UNIFIED_DST_RECT ";

typedef struct Test2D {
    GalTest     base;
    GalRuntime *runtime;

    //source surface
    T2D_SURF_PTR    surf[8];
} Test2D;

static gceSURF_ROTATION sRots[] =
{
    gcvSURF_0_DEGREE,
    gcvSURF_90_DEGREE,
    gcvSURF_180_DEGREE,
    gcvSURF_270_DEGREE,
    gcvSURF_FLIP_X,
    gcvSURF_FLIP_Y,
};

static gctINT GetSurfaceIndex(gctINT total, gctINT frameNo, gctINT srcIndex)
{
    static gctINT indexArray[] =
    {
        0, 1, 2, 3, 4, 5, 6, 7,
        2, 3, 0, 1, 4, 5, 6, 7,
        2, 3, 4, 5, 0, 1, 6, 7,
        2, 3, 4, 5, 6, 7, 0, 1,
    };

    return indexArray[(frameNo / (total / 4)) * 8 + srcIndex];
}

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status;
    gcsRECT Rect;
    gco2D egn2D = t2d->runtime->engine2d;
    gctINT i;
    gctINT surfaceIndex;
    T2D_SURF_PTR result = gcvNULL;

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal, gcvSURF_A8R8G8B8, gcvLINEAR, gcv2D_TSC_DISABLE,
        480, 480, &result));

    for (i = 0; i < 8; i++)
    {
        gcsRECT srect;

        srect.left   = 0;
        srect.right  = result->width;
        srect.top    = 0;
        srect.bottom = result->height;

        gcmONERROR(gco2D_SetCurrentSourceIndex(egn2D, i));

        surfaceIndex = GetSurfaceIndex(t2d->base.frameCount, frameNo, i);

        gcmONERROR(gco2D_SetGenericSource(
            egn2D,
            t2d->surf[surfaceIndex]->address, t2d->surf[surfaceIndex]->validAddressNum,
            t2d->surf[surfaceIndex]->stride, t2d->surf[surfaceIndex]->validStrideNum,
            t2d->surf[surfaceIndex]->tiling, t2d->surf[surfaceIndex]->format,
            sRots[(frameNo / 24) % 6],
            t2d->surf[surfaceIndex]->width, t2d->surf[surfaceIndex]->height));

        gcmONERROR(gco2D_SetSourceTileStatus(
            egn2D,
            t2d->surf[surfaceIndex]->tileStatusConfig,
            t2d->surf[surfaceIndex]->format,
            t2d->surf[surfaceIndex]->tileStatusClear,
            t2d->surf[surfaceIndex]->tileStatusAddress
            ));

        gcmONERROR(gco2D_SetSource(egn2D, &srect));

        gcmONERROR(gco2D_SetROP(egn2D, 0xCC, 0xCC));

        /* Disable multiply first. */
        gcmONERROR(gco2D_SetPixelMultiplyModeAdvanced(egn2D,
            gcv2D_COLOR_MULTIPLY_DISABLE,
            gcv2D_COLOR_MULTIPLY_DISABLE,
            gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE,
            gcv2D_COLOR_MULTIPLY_DISABLE));

        if (surfaceIndex < 2)
        {
            gcmONERROR(gco2D_EnableAlphaBlend(egn2D,
                        128, 128,
                        gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
                        gcvSURF_GLOBAL_ALPHA_ON, gcvSURF_GLOBAL_ALPHA_ON,
                        gcvSURF_BLEND_STRAIGHT, gcvSURF_BLEND_STRAIGHT,
                        gcvSURF_COLOR_STRAIGHT, gcvSURF_COLOR_STRAIGHT));
        }
        else
        {
            gcmONERROR(gco2D_EnableAlphaBlend(egn2D,
                        0, 0,
                        gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
                        gcvSURF_GLOBAL_ALPHA_OFF, gcvSURF_GLOBAL_ALPHA_OFF,
                        gcvSURF_BLEND_ONE, gcvSURF_BLEND_INVERSED,
                        gcvSURF_COLOR_MULTIPLY, gcvSURF_COLOR_STRAIGHT));
        }

        switch (frameNo % 4)
        {
            case 0:
                // disable mirror
                gcmONERROR(gco2D_SetBitBlitMirror(egn2D, gcvFALSE, gcvFALSE));
                break;
            case 1:
                // enable horizontal mirror
                gcmONERROR(gco2D_SetBitBlitMirror(egn2D, gcvTRUE, gcvFALSE));
                break;
            case 2:
                // enable vertical mirror
                gcmONERROR(gco2D_SetBitBlitMirror(egn2D, gcvFALSE, gcvTRUE));
                break;
            case 3:
                // enable horizontal & vertical mirror
                gcmONERROR(gco2D_SetBitBlitMirror(egn2D, gcvTRUE, gcvTRUE));
                break;
            default:
                return gcvFALSE;
        }
    }

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

    gcmONERROR(gco2D_SetTargetTileStatus(
        egn2D,
        result->tileStatusConfig,
        result->format,
        sRots[(frameNo / 4) % 6],
        result->tileStatusAddress
        ));

    Rect.left   = 0;
    Rect.top    = 0;
    Rect.right  = result->width;
    Rect.bottom = result->height;

    gcmONERROR(gco2D_SetClipping(egn2D, &Rect));

    gcmONERROR(gco2D_MultiSourceBlit(egn2D, 0xff, &Rect, 1));

    gcmONERROR(gco2D_DisableAlphaBlend(egn2D));

    gcmONERROR(gco2D_SetBitBlitMirror(egn2D, gcvFALSE, gcvFALSE));

    gcmONERROR(gco2D_Flush(egn2D));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    if (!t2d->runtime->noSaveTargetNew && t2d->runtime->saveFullName)
    {
        GalSaveTSurfToDIB(result, t2d->runtime->saveFullName);
    }

OnError:
    if (result)
    {
        GalDeleteTSurf(gcvNULL, result);
    }

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
    gceSTATUS status = gcvSTATUS_OK;
    gctINT i;

    // destroy source surface
    for (i = 0; i < 8; ++i)
    {
        if (t2d->surf[i] != gcvNULL)
        {
            if (gcmIS_ERROR(GalDeleteTSurf(t2d->runtime->hal, t2d->surf[i])))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console,
                    "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));
            }
            t2d->surf[i] = gcvNULL;
        }
    }

    free(t2d);
}

static gceSTATUS CDECL InitSourceSurface(Test2D *t2d, gctUINT index)
{
    T2D_SURF_PTR surf = gcvNULL;
    gcsRECT dstRect, headRect;
    gco2D egn2D = t2d->runtime->engine2d;
    gceSTATUS status;
    gctINT i;
    gctUINT32 color;
    gctUINT32 offset;

    static gctSTRING sourcefile[] =
    {
        "resource/zero2_YUY2_640X480_Linear.vimg",
        "resource/index8.bmp",
    };

    static gctSTRING sourcefile2[] =
    {
        "resource/zero2_YUY2_640X480_Linear.vimg",
        "resource/index8_argb.bmp",
    };


    if (index < 2)
    {
        if (gcoHAL_IsFeatureAvailable(t2d->runtime->hal, gcvFEATURE_ANDROID_ONLY) != gcvTRUE)
            gcmONERROR(GalLoadFileToTSurf(sourcefile[index], &surf));
        else
            gcmONERROR(GalLoadFileToTSurf(sourcefile2[index], &surf));
        goto Done;
    }

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        gcvSURF_A8R8G8B8,
        gcvLINEAR,
        (index < 4) ? gcv2D_TSC_DISABLE: gcv2D_TSC_2D_COMPRESSED,
        480, 480, &surf));

    dstRect.left   = 0;
    dstRect.top    = 0;
    dstRect.right  = surf->width;
    dstRect.bottom = surf->height;

    switch (index)
    {
        case 0: color = COLOR_ARGB8(0xff, 0xff, 0x0,  0x0);  break;
        case 1: color = COLOR_ARGB8(0xff, 0xff, 0xff, 0x0);  break;
        case 2: color = COLOR_ARGB8(0xff, 0xff, 0x0,  0xff); break;
        case 3: color = COLOR_ARGB8(0xff, 0x0,  0xff, 0x0);  break;
        case 4: color = COLOR_ARGB8(0xff, 0x0,  0x0,  0xff); break;
        case 5: color = COLOR_ARGB8(0xff, 0x0,  0xff, 0xff); break;
        case 6: color = COLOR_ARGB8(0xff, 0x80, 0x80, 0x80); break;
        case 7: color = COLOR_ARGB8(0xff, 0xff, 0x80, 0x80); break;
        default: gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        surf->address,
        surf->validAddressNum,
        surf->stride,
        surf->validStrideNum,
        surf->tiling,
        surf->format,
        surf->rotation,
        surf->width,
        surf->height));

    gcmONERROR(gco2D_SetTargetTileStatus(
        egn2D,
        surf->tileStatusConfig,
        surf->format,
        surf->tileStatusClear,
        surf->tileStatusAddress
        ));

    dstRect.left  = dstRect.top    = 24 * index;
    dstRect.right = dstRect.bottom = 480 - 24 * index;

    headRect.left   = dstRect.left;
    headRect.top    = dstRect.top;
    headRect.right  = dstRect.right;

    dstRect.top    += 12;
    headRect.bottom = dstRect.top;

    offset = (480 - 24 * index * 2) / 8;

    for (i = 0; i < 3; ++i)
    {
        switch (i)
        {
            case 0:
                dstRect.left  = 24 * index;
                dstRect.right = dstRect.left + offset * 3;
                break;

            case 1:
                dstRect.left  = 24 * index + offset * 4;
                dstRect.right = dstRect.left + offset * 2;
                break;

            case 2:
                dstRect.left  = 24 * index + offset * 7;
                dstRect.right = dstRect.left + offset;
                break;
        }

        headRect.left  = dstRect.left;
        headRect.right = dstRect.right;

        gcmONERROR(gco2D_LoadSolidBrush(egn2D, surf->format, gcvTRUE, color, 0));
        gcmONERROR(gco2D_Blit(egn2D, 1, &dstRect, 0xF0, 0xF0, surf->format));

        gcmONERROR(gco2D_LoadSolidBrush(egn2D, surf->format, gcvTRUE, COLOR_ARGB8(0xff, 0xff, 0xff, 0xff), 0));
        gcmONERROR(gco2D_Blit(egn2D, 1, &headRect, 0xF0, 0xF0, surf->format));
    }

    gcmONERROR(gco2D_Flush(egn2D));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

Done:
    t2d->surf[index] = surf;

OnError:
    return status;
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_2D_COMPRESSION,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status;
    gctINT i;

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

    if (gcoHAL_IsFeatureAvailable(runtime->hal, gcvFEATURE_ANDROID_ONLY) != gcvTRUE)
    {
        if (gcoHAL_IsFeatureAvailable(runtime->hal, gcvFEATURE_2D_MULTI_SRC_BLT_TO_UNIFIED_DST_RECT) != gcvTRUE)
        {
            GalOutput(GalOutputType_Result | GalOutputType_Console, "2D MultiSourceBlit is not supported.\n");
            runtime->notSupport = gcvTRUE;
            return gcvFALSE;
        }

        /* Set the state. */
        gcmONERROR(gco2D_SetStateU32(runtime->engine2d,
                                     gcv2D_STATE_MULTI_SRC_BLIT_UNIFIED_DST_RECT,
                                     gcvTRUE));
    }

    memset(t2d, 0, sizeof(Test2D));

    t2d->runtime = runtime;

    runtime->saveTarget = gcvFALSE;
    runtime->cleanTarget = gcvFALSE;

    // create source surface
    for (i = 0; i < 8; ++i)
    {
        gcmONERROR(InitSourceSurface(t2d, i));
    }

    t2d->base.render      = (PGalRender)Render;
    t2d->base.destroy     = (PGalDestroy)Destroy;
    t2d->base.frameCount  = 144;
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

    memset(t2d, 0, sizeof(Test2D));

    if (!Init(t2d, runtime)) {
        free(t2d);
        return NULL;
    }

    return &t2d->base;
}
