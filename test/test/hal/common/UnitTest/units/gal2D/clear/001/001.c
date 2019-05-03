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
 *  Feature:    Clear rectangle(s) with a single color.
 *  API:        gco2D_Clear
 *  Check:      Rectangle count, position and color.
*/
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DClear001\n" \
"Operation: Test gco2D_Clear interface.\n" \
"2D API: gco2D_Clear\n" \
"Src: [None]\n"\
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

    /* Dst surface. */
    gcoSURF            dstSurf;
    gceSURF_FORMAT    dstFormat;
    gctUINT            dstWidth;
    gctUINT            dstHeight;
    gctINT            dstStride;
    gctUINT32        dstPhyAddr;
    gctPOINTER        dstLgcAddr;

    /* Line color. */
    gcsRECT            rects[4];
    gctUINT32        rectNum;
    gctUINT32        rectColor;
} Test2D;


static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gcsRECT   dstRect = { 0, 0, t2d->dstWidth, t2d->dstHeight };
    gco2D     egn2D   = t2d->runtime->engine2d;
    gceSTATUS status = gcvSTATUS_OK;

    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    gcmONERROR(gco2D_SetTarget(egn2D, t2d->dstPhyAddr, t2d->dstStride, gcvSURF_0_DEGREE, t2d->dstWidth));

    /* clear the dst surface. */
    gcmONERROR(gco2D_Clear(egn2D, 1, &dstRect, COLOR_ARGB8(0, 0, 0, 0), 0xCC, 0xCC, t2d->dstFormat));

    if (frameNo == 0)
    {
        /* Clear a single rectangle. */
        t2d->rects[0].left   = t2d->dstWidth/4;
        t2d->rects[0].top    = t2d->dstHeight/4;
        t2d->rects[0].right  = t2d->dstWidth*3/4;
        t2d->rects[0].bottom = t2d->dstHeight*3/4;

        t2d->rectNum = 1;

        /* Set the rect color (ARGB8888). */
        t2d->rectColor = 0x00F88024;
    }
    else if (frameNo == 1)
    {
        /* Clear 4 rectangles. */
        t2d->rects[0].left   = t2d->dstWidth/8;
        t2d->rects[0].top    = t2d->dstHeight/8;
        t2d->rects[0].right  = t2d->dstWidth*3/8;
        t2d->rects[0].bottom = t2d->dstHeight*3/8;

        t2d->rects[1].left   = t2d->dstWidth*5/8;
        t2d->rects[1].top    = t2d->dstHeight*1/8;
        t2d->rects[1].right  = t2d->dstWidth*7/8;
        t2d->rects[1].bottom = t2d->dstHeight*3/8;

        t2d->rects[2].left   = t2d->dstWidth*2/8;
        t2d->rects[2].top    = t2d->dstHeight*2/8;
        t2d->rects[2].right  = t2d->dstWidth*6/8;
        t2d->rects[2].bottom = t2d->dstHeight*6/8;

        t2d->rects[3].left   = t2d->dstWidth/8;
        t2d->rects[3].top    = t2d->dstHeight*5/8;
        t2d->rects[3].right  = t2d->dstWidth*7/8;
        t2d->rects[3].bottom = t2d->dstHeight*7/8;

        t2d->rectNum = 4;

        /* Set the rect color (ARGB8888). */
        t2d->rectColor = 0x00536170;
    }

    gcmONERROR(gco2D_Clear(egn2D, t2d->rectNum, t2d->rects, t2d->rectColor, 0xCC, 0xCC, t2d->dstFormat));

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

    free(t2d);
}

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status = gcvSTATUS_OK;

    runtime->wholeDescription = (char*)malloc(strlen(s_CaseDescription) + 1);
    if (runtime->wholeDescription == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    memcpy(runtime->wholeDescription, s_CaseDescription, strlen(s_CaseDescription) + 1);

    t2d->runtime    = runtime;

    t2d->dstSurf    = runtime->target;
    t2d->dstFormat  = runtime->format;
    t2d->dstWidth   = 0;
    t2d->dstHeight  = 0;
    t2d->dstStride  = 0;
    t2d->dstPhyAddr = 0;
    t2d->dstLgcAddr = 0;

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    t2d->rectNum   = 0;
    t2d->rectColor = 0;

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
