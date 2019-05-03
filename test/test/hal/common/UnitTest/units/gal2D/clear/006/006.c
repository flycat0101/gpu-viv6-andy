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
 *  Feature:    Clear multiple rectangles with a single color.
 *  API:        gco2D_Clear
 *  Check:      check multiple rectangles.
 */
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DClear006\n" \
"Operation: Test clear multiple rectangles.\n" \
"2D API: gco2D_Clear\n" \
"Src: [None]\n"\
"Dst: Size        [configurable]\n"\
"     Rect        [More than one and configurable]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: [None]\n"\
"Alphablend: [disable]\n" \
"HW feature dependency: ";

typedef struct Test2D {
    GalTest         base;
    GalRuntime     *runtime;

    /* Dst surface. */
    gcoSURF         dstSurf;
    gceSURF_FORMAT  dstFormat;
    gctUINT         dstWidth;
    gctUINT         dstHeight;
    gctINT          dstStride;
    gctUINT32       dstPhyAddr;
    gctPOINTER      dstLgcAddr;
}
Test2D;

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gcsRECT   dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};
    gco2D     egn2D   = t2d->runtime->engine2d;
    gctINT    deltaX  = 2;
    gctINT    deltaY  = 2;
    gctINT    i;
    gceSTATUS status;

    gcsRECT rect[] =
    {
        {0, 0, deltaX, t2d->dstHeight},
        {t2d->dstWidth - deltaX, 0, t2d->dstWidth, t2d->dstHeight},
        {0, t2d->dstHeight / 2 - (deltaY * 2), t2d->dstWidth, t2d->dstHeight / 2 + (deltaY * 2)},
        {t2d->dstWidth / 4, t2d->dstHeight / 4 - deltaY, t2d->dstWidth * 3 / 4, t2d->dstHeight / 4 + deltaY},
        {t2d->dstWidth / 4, t2d->dstHeight * 3 / 4 - deltaY, t2d->dstWidth * 3 / 4, t2d->dstHeight * 3 / 4 + deltaY},
    };

    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    gcmONERROR(gco2D_SetTarget(egn2D, t2d->dstPhyAddr, t2d->dstStride, gcvSURF_0_DEGREE, t2d->dstWidth));

    /* Clear the surface. */
    gcmONERROR(gco2D_Clear(
        egn2D, 1, &dstRect, COLOR_ARGB8(0x0, 0xFF, 0xFF, 0xFF),
        0xCC, 0xCC, t2d->dstFormat));

    /* Clear one pixel. */
    dstRect.left   = 0;
    dstRect.top    = 0;
    dstRect.right  = 1;
    dstRect.bottom = 1;

    for (i = 0; i < 320 * 320; i++)
    {
        gcmONERROR(gco2D_Clear(
            egn2D, 1, &dstRect, COLOR_ARGB8(0x0, 0x0, 0x0, 0x0),
            0xCC, 0xCC, t2d->dstFormat));
    }

    /* Clear multiple rectangles. */
    gcmONERROR(gco2D_Clear(
        egn2D, gcmCOUNTOF(rect), rect, COLOR_ARGB8(0x0, 0xFF, 0x0, 0x0),
        0xCC, 0xCC, t2d->dstFormat));

    gcmONERROR(gco2D_Flush(egn2D));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    return gcvTRUE;

OnError:
    GalOutput(GalOutputType_Error | GalOutputType_Console,
        "%s(%d) failed:%s\n", __FUNCTION__, __LINE__, GalStatusString(status));

    return gcvFALSE;
}

static void CDECL Destroy(Test2D *t2d)
{
    gceSTATUS status = gcvSTATUS_OK;

    if ((t2d->dstSurf != gcvNULL) && (t2d->dstLgcAddr != gcvNULL))
    {
        status = gcoSURF_Unlock(t2d->dstSurf, t2d->dstLgcAddr);
        if (gcmIS_ERROR(status))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console,
                    "Unlock desSurf failed:%s\n", GalStatusString(status));
        }
        t2d->dstLgcAddr = gcvNULL;
    }

    free(t2d);
}

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status;
    t2d->runtime = runtime;

    runtime->wholeDescription = (char*)malloc(strlen(s_CaseDescription) + 1);
    if (runtime->wholeDescription == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    memcpy(runtime->wholeDescription, s_CaseDescription, strlen(s_CaseDescription) + 1);

    t2d->dstSurf    = runtime->target;
    t2d->dstFormat  = runtime->format;
    t2d->dstWidth   = 0;
    t2d->dstHeight  = 0;
    t2d->dstStride  = 0;
    t2d->dstPhyAddr = 0;
    t2d->dstLgcAddr = 0;

    gcmONERROR(gcoSURF_GetAlignedSize(
        t2d->dstSurf, &t2d->dstWidth, &t2d->dstHeight, &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    t2d->base.render      = (PGalRender)Render;
    t2d->base.destroy     = (PGalDestroy)Destroy;
    t2d->base.frameCount  = 1;
    t2d->base.description = s_CaseDescription;

    return gcvTRUE;

OnError:

    GalOutput(GalOutputType_Error | GalOutputType_Console,
        "%s(%d) failed:%s\n", __FUNCTION__, __LINE__, GalStatusString(status));

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
