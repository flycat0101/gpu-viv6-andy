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

#define SRC_NUM 8

static const char *sSrcFile[SRC_NUM] = {
     "resource/texture2_cc0_A4R4G4B4.vimg",
     "resource/texture2_cc0_A8R8G8B8.vimg",
     "resource/texture2_cc0_R5G6B5.vimg",
     "resource/texture2_cc0_X8R8G8B8.vimg",
     "resource/texture2_cc1_A4R4G4B4.vimg",
     "resource/texture2_cc1_A8R8G8B8.vimg",
     "resource/texture2_cc1_R5G6B5.vimg",
     "resource/texture2_cc1_X8R8G8B8.vimg",
};

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFormatTileStatus005\n" \
"Operation: Convert FC surface to linear by MultisrcBlit.\n" \
"2D API: co2D_SetGenericSource gco2D_SetGenericTarget gco2D_MultiSourceBlit\n" \
"Src1: Size       [320x240]\n"\
"     Rect        [variable]\n"\
"     Format      [ARGB4444]\n"\
"     Rotation    [0]\n"\
"     Tile        [multiSuperTile]\n"\
"     Compression [None]\n" \
"Src2: Size       [320x240]\n"\
"     Rect        [variable]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [multiSuperTile]\n"\
"     Compression [None]\n" \
"Src3: Size       [320x240]\n"\
"     Rect        [variable]\n"\
"     Format      [RGB565]\n"\
"     Rotation    [0]\n"\
"     Tile        [multiSuperTile]\n"\
"     Compression [None]\n" \
"Src4: Size       [320x240]\n"\
"     Rect        [variable]\n"\
"     Format      [XRGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [multiSuperTile]\n"\
"     Compression [None]\n" \
"Src5: Size       [640x480]\n"\
"     Rect        [variable]\n"\
"     Format      [ARGB4444]\n"\
"     Rotation    [0]\n"\
"     Tile        [multiSuperTile]\n"\
"     Compression [None]\n" \
"Src6: Size       [640x480]\n"\
"     Rect        [variable]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [multiSuperTile]\n"\
"     Compression [None]\n" \
"Src7: Size       [640x480]\n"\
"     Rect        [variable]\n"\
"     Format      [RGB565]\n"\
"     Rotation    [0]\n"\
"     Tile        [multiSuperTile]\n"\
"     Compression [None]\n" \
"Src8: Size       [640x480]\n"\
"     Rect        [variable]\n"\
"     Format      [XRGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [multiSuperTile]\n"\
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
    gcoSURF         dstSurf;
    gceSURF_FORMAT  dstFormat;
    gctUINT         dstWidth;
    gctUINT         dstHeight;
    gctINT          dstStride;
    gctUINT32       dstPhyAddr;
    gctPOINTER      dstLgcAddr;

    //source surface
    T2D_SURF_PTR    srcSurf[SRC_NUM];
    gcsRECT         rect[SRC_NUM];
} Test2D;

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status;
    gcsRECT dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};
    gco2D egn2D = t2d->runtime->engine2d;
    gctINT i, vOffset,hOffset;

    hOffset = 320 / t2d->base.frameCount * frameNo;
    vOffset = 240 / t2d->base.frameCount * frameNo;

    for (i = 0; i < 8; i++)
    {
        gcmONERROR(gco2D_SetCurrentSourceIndex(egn2D, i));

        gcmONERROR(gco2D_SetGenericSource(
            egn2D,
            t2d->srcSurf[i]->address, t2d->srcSurf[i]->validAddressNum,
            t2d->srcSurf[i]->stride, t2d->srcSurf[i]->validStrideNum,
            t2d->srcSurf[i]->tiling, t2d->srcSurf[i]->format, t2d->srcSurf[i]->rotation,
            t2d->srcSurf[i]->width, t2d->srcSurf[i]->height));

        gcmONERROR(gco2D_SetSourceTileStatus(
            egn2D,
            t2d->srcSurf[i]->tileStatusConfig,
            t2d->srcSurf[i]->tileStatusFormat,
            t2d->srcSurf[i]->tileStatusClear,
            t2d->srcSurf[i]->tileStatusAddress
            ));

        gco2D_SetStateU32(egn2D, gcv2D_STATE_SUPER_TILE_VERSION, t2d->srcSurf[i]->superTileVersion);

        gcmONERROR(gco2D_SetSource(egn2D, &t2d->rect[(frameNo + i) % 8]));

        gcmONERROR(gco2D_SetROP(egn2D, 0xCC, 0xCC));
    }

    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        &t2d->dstPhyAddr, 1,
        &t2d->dstStride, 1,
        gcvLINEAR,
        t2d->dstFormat,
        gcvSURF_0_DEGREE,
        t2d->dstWidth,
        t2d->dstHeight));

    gcmONERROR(gco2D_MultiSourceBlit(egn2D, 0xFF, &dstRect, 1));

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
    gctINT i;

    if ((t2d->dstSurf != gcvNULL) && (t2d->dstLgcAddr != gcvNULL))
    {
        if (gcmIS_ERROR(gcoSURF_Unlock(t2d->dstSurf, t2d->dstLgcAddr)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock desSurf failed:%s\n", GalStatusString(status));
        }
        t2d->dstLgcAddr = gcvNULL;
    }

    // destroy source surface
    for (i = 0; i < SRC_NUM; i++)
    {
        if (t2d->srcSurf[i] != gcvNULL)
        {
            if (gcmIS_ERROR(GalDeleteTSurf(t2d->runtime->hal, t2d->srcSurf[i])))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "DeleteT srcSurf failed:%s\n", GalStatusString(status));
            }
            t2d->srcSurf[i] = gcvNULL;
        }
    }

    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_2D_FC_SOURCE,
    gcvFEATURE_2D_TILING,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status;
    gctINT i;
    gctUINT x = 0, y = 0;

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

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    for (i = 0; i < SRC_NUM; i++)
    {
        // create source surface
        gcmONERROR(GalLoadVimgToTSurf(
            sSrcFile[i], t2d->srcSurf + i));

        t2d->rect[i].left = x;
        t2d->rect[i].top = y;
        t2d->rect[i].right = t2d->rect[i].left + 80;
        t2d->rect[i].bottom = t2d->rect[i].top + 120;

        x += 80;
        if (x >= 320)
        {
            x = 0;
            y = 120;
        }
    }

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = 8;
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
