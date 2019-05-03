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
"Case gal2DClear003\n" \
"Operation: Test clear with rectangles with all rotations.\n" \
"2D API: gco2D_Clear\n" \
"Src: [None]\n"\
"Dst: Size        [configurable]\n"\
"     Rect        [configurable]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0/90/180/270]\n"\
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

    /* misc. */
    gcsRECT            rects[10];
    gctUINT         rotationNum;
} Test2D;

static gceSURF_ROTATION rotationList [] =
{
    gcvSURF_0_DEGREE,
    gcvSURF_90_DEGREE,
    gcvSURF_180_DEGREE,
    gcvSURF_270_DEGREE,
    gcvSURF_FLIP_X,
    gcvSURF_FLIP_Y,
};

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gcsRECT   dstRect = { 0, 0, t2d->dstWidth, t2d->dstHeight };
    gco2D     egn2D   = t2d->runtime->engine2d;
    gceSTATUS status  = gcvSTATUS_OK;
    gctUINT   i, j;
    gceSURF_ROTATION  rot;
    gctUINT32 rectNum = 0;
    gctUINT   delta   = gcmMIN(t2d->dstWidth, t2d->dstHeight);

    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    rot = rotationList[frameNo % t2d->rotationNum];

    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        &t2d->dstPhyAddr, 1,
        (gctUINT32_PTR)&t2d->dstStride, 1,
        gcvLINEAR,
        t2d->dstFormat,
        rot,
        t2d->dstWidth,
        t2d->dstHeight));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        &t2d->dstPhyAddr, 1,
        (gctUINT32_PTR)&t2d->dstStride, 1,
        gcvLINEAR,
        t2d->dstFormat,
        rot,
        t2d->dstWidth,
        t2d->dstHeight));

    /* Clear 10 rectangles. */
    for ( i = 0; i < delta; i = i + delta / 5)
    {
        for ( j = 0; j < delta; j = j + delta / 2)
        {
            t2d->rects[rectNum].left   = i;
            t2d->rects[rectNum].top    = j;
            t2d->rects[rectNum].right  = t2d->rects[rectNum].left + delta / 6;
            t2d->rects[rectNum].bottom = t2d->rects[rectNum].top + delta / 4;
            rectNum++;
        }
    }

    /* Set the rect color with red(ARGB8888). */
    gcmONERROR(gco2D_Clear(egn2D, rectNum, t2d->rects, COLOR_ARGB8(0, 0xFF, 0, 0), 0xCC, 0xCC, t2d->dstFormat));

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
    gctINT    argc   = runtime->argc;
    gctSTRING *argv  = runtime->argv;

    runtime->wholeDescription = (char*)malloc(strlen(s_CaseDescription) + 1);
    if (runtime->wholeDescription == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    memcpy(runtime->wholeDescription, s_CaseDescription, strlen(s_CaseDescription) + 1);

    memset(t2d, 0, sizeof(Test2D));

    t2d->runtime     = runtime;
    t2d->dstSurf     = runtime->target;
    t2d->dstFormat   = runtime->format;
    t2d->dstWidth    = 0;
    t2d->dstHeight   = 0;
    t2d->dstStride   = 0;
    t2d->dstPhyAddr  = 0;
    t2d->dstLgcAddr  = 0;
    t2d->rotationNum = sizeof(rotationList) / sizeof(rotationList[0]);

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    t2d->base.render      = (PGalRender)Render;
    t2d->base.destroy     = (PGalDestroy)Destroy;
    t2d->base.frameCount  = t2d->rotationNum;
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
