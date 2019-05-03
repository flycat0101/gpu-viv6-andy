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
 *  Feature:    ColorSource - Transparency
 *  API:        gco2D_SetSource gco2D_SetColorSource/gco2D_SetColorSourceAdvanced
 *                gco2D_SetColorSource is only working with old PE (<2.0) and
 *                gco2D_SetColorSourceAdvanced is only working with PE 2.0 and above.
 *  Check:      source pool
*/
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DColorSource007\n" \
"Operation: Test blit the screen with ColorSource -- test source pool.\n" \
"2D API: gco2D_SetSource gco2D_SetColorSource/gco2D_SetColorSourceAdvanced gco2D_SetTransparencyAdvanced\n" \
"Src: Size        [configurable]\n"\
"     Rect        [configurable]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"     Memory      [system memory / virtual memory]\n" \
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
    gcoSURF            dstSurf;
    gceSURF_FORMAT    dstFormat;
    gctUINT            dstWidth;
    gctUINT            dstHeight;
    gctINT            dstStride;
    gctUINT32        dstPhyAddr;
    gctPOINTER        dstLgcAddr;

    //source surface
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
    gcsRECT Rect;
    gco2D egn2D = t2d->runtime->engine2d;
    gceSTATUS status;
    gcePOOL pool;
    gctSTRING poolName;
    gcoSURF surf;
    gctINT            srcStride;
    gctUINT32        srcPhyAddr;
    gctPOINTER        srcLgcAddr;
    gctUINT            srcWidth;
    gctUINT            srcHeight;

    switch (frameNo)
    {
    case 0:
        pool =     gcvPOOL_DEFAULT;
        break;

    case 1:
        pool = gcvPOOL_LOCAL;
        break;

    case 2:
        pool = gcvPOOL_UNIFIED;
        break;

    case 3:
        pool = gcvPOOL_SYSTEM;
        break;

    case 4:
        pool = gcvPOOL_VIRTUAL;
        break;

    default:
        return gcvFALSE;
    }

    gcmONERROR(gcoSURF_Construct(t2d->runtime->hal, t2d->dstWidth, t2d->dstHeight, 1, gcvSURF_BITMAP,
            t2d->srcFormat, pool, &surf));

    gcmONERROR(gcoSURF_Blit(t2d->srcSurf, surf, 1, gcvNULL, gcvNULL, gcvNULL, 0xCC, 0xCC
                            , gcvSURF_OPAQUE, 0, 0, 0));

    gcmONERROR(gcoSURF_GetAlignedSize(surf,
                                        gcvNULL,
                                        gcvNULL,
                                        &srcStride));

    gcmONERROR(gcoSURF_GetSize(surf,
                                &srcWidth,
                                &srcHeight,
                                gcvNULL));

    gcmONERROR(gcoSURF_Lock(surf, &srcPhyAddr, &srcLgcAddr));

    switch (pool/*surf->info.node.pool*/)
    {
    case gcvPOOL_DEFAULT:
        poolName =     "default";
        break;

    case gcvPOOL_LOCAL:
        poolName = "local";
        break;

    case gcvPOOL_LOCAL_INTERNAL:
        poolName = "internal local";
        break;

    case gcvPOOL_LOCAL_EXTERNAL:
        poolName = "external local";
        break;

    case gcvPOOL_UNIFIED:
        poolName = "unified";
        break;

    case gcvPOOL_SYSTEM:
        poolName = "system";
        break;

    case gcvPOOL_VIRTUAL:
        poolName = "virtual";
        break;

    default:
        return gcvFALSE;
    }

    GalOutput(GalOutputType_Result, "source surface is used with %s pool \n", poolName);

    if (t2d->runtime->pe20)
    {
        gcmONERROR(gco2D_SetColorSourceAdvanced(egn2D, srcPhyAddr, srcStride, t2d->srcFormat,
                        gcvSURF_0_DEGREE, srcWidth, srcHeight, gcvFALSE));
    }
    else
    {
        gcmONERROR(gco2D_SetColorSource(egn2D, srcPhyAddr, srcStride, t2d->srcFormat,
                        gcvSURF_0_DEGREE, srcWidth, gcvFALSE, gcvSURF_OPAQUE, 0));
    }

    Rect.left = 0;
    Rect.top = 0;
    Rect.right = min(t2d->dstWidth, srcWidth);
    Rect.bottom = min(t2d->dstHeight, srcHeight);
    gcmONERROR(gco2D_SetSource(egn2D, &Rect));

    gcmONERROR(gco2D_SetTarget(egn2D, t2d->dstPhyAddr, t2d->dstStride, gcvSURF_0_DEGREE, t2d->dstWidth));

    gcmONERROR(gco2D_SetClipping(egn2D, &Rect));

    gcmONERROR(gco2D_Blit(egn2D, 1, &Rect, 0xCC, 0xCC, t2d->dstFormat));

    gcmONERROR(gco2D_Flush(egn2D));

    gcmONERROR(gcoSURF_Unlock(surf, srcLgcAddr));

    if (gcmIS_ERROR(gcoSURF_Destroy(surf)))
    {
        GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Surf failed:%s\n", GalStatusString(status));
    }

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

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status;
    char *sourcefile = "resource/zero1.bmp";

    runtime->wholeDescription = (char*)malloc(strlen(s_CaseDescription) + 1);
    if (runtime->wholeDescription == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    memcpy(runtime->wholeDescription, s_CaseDescription, strlen(s_CaseDescription) + 1);

    if (runtime->ChipModel == gcv620)
    {
        GalOutput(GalOutputType_Result | GalOutputType_Console, "Virtual memory is not supported.\n");
        runtime->notSupport = gcvTRUE;
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

    t2d->srcSurf    = gcvNULL;
    t2d->srcWidth = 0;
    t2d->srcHeight = 0;
    t2d->srcStride = 0;
    t2d->srcPhyAddr = 0;
    t2d->srcLgcAddr = 0;
    t2d->srcFormat = gcvSURF_UNKNOWN;


    // create source surface
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
    t2d->base.description = s_CaseDescription;
#if gcdSECURE_USER
    t2d->base.frameCount = 4;
#else
    t2d->base.frameCount = 5;
#endif

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
