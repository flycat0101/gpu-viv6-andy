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
 *  Feature:    rotate 270 degree
 *  API:        gco2D_SetTarget
 *  Check:
*/
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DRotation011\n" \
"Operation: Test blit with full rotation feature test.\n" \
"2D API: gco2D_Blit\n" \
"Src: Size        [400x300]\n"\
"     Rect        [0,0,80,100]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Dst: Size        [configurable]\n"\
"     Rect        [0,0,80,100]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: [None]\n"\
"Alphablend: [disable]\n" \
"HW feature dependency: ";

typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;

      // dest surface
    gcoSURF           dstSurf;
    gctUINT32       dstPhysAddr;
    gctPOINTER       dstVirtAddr;
    gceSURF_FORMAT dstFormat;
    gctUINT32       dstWidth;
    gctUINT32       dstHeight;
    gctUINT32       dstAlignedWidth;
    gctUINT32       dstAlignedHeight;
    gctINT           dstStride;

    // source surface
    gcoSURF           srcSurf;
    gctUINT32       srcPhysAddr;
    gctPOINTER       srcVirtAddr;
    gceSURF_FORMAT srcFormat;
    gctUINT32       srcWidth;
    gctUINT32       srcHeight;
    gctUINT32       srcAlignedWidth;
    gctUINT32       srcAlignedHeight;
    gctINT           srcStride;
} Test2D;


static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gcsRECT srcRect, dstRect;
    gctUINT rectWidth = 80, rectHeight = 100;
    gco2D      egn2D  = t2d->runtime->engine2d;
    gceSTATUS status = gcvSTATUS_OK;
    gceSURF_ROTATION srcRot, dstRot;

    switch (frameNo)
    {
    case 0:
        srcRot = gcvSURF_0_DEGREE;
        dstRot = gcvSURF_0_DEGREE;
        break;

    case 1:
        srcRot = gcvSURF_90_DEGREE;
        dstRot = gcvSURF_0_DEGREE;
        break;

    case 2:
        srcRot = gcvSURF_0_DEGREE;
        dstRot = gcvSURF_90_DEGREE;
        break;

    case 3:
        srcRot = gcvSURF_90_DEGREE;
        dstRot = gcvSURF_90_DEGREE;
        break;

    case 4:
        srcRot = gcvSURF_180_DEGREE;
        dstRot = gcvSURF_0_DEGREE;
        break;

    case 5:
        srcRot = gcvSURF_270_DEGREE;
        dstRot = gcvSURF_0_DEGREE;
        break;

    case 6:
        srcRot = gcvSURF_180_DEGREE;
        dstRot = gcvSURF_90_DEGREE;
        break;

    case 7:
        srcRot = gcvSURF_270_DEGREE;
        dstRot = gcvSURF_90_DEGREE;
        break;

    case 8:
        srcRot = gcvSURF_0_DEGREE;
        dstRot = gcvSURF_180_DEGREE;
        break;

    case 9:
        srcRot = gcvSURF_90_DEGREE;
        dstRot = gcvSURF_180_DEGREE;
        break;

    case 10:
        srcRot = gcvSURF_180_DEGREE;
        dstRot = gcvSURF_180_DEGREE;
        break;

    case 11:
        srcRot = gcvSURF_270_DEGREE;
        dstRot = gcvSURF_180_DEGREE;
        break;

    case 12:
        srcRot = gcvSURF_0_DEGREE;
        dstRot = gcvSURF_270_DEGREE;
        break;

    case 13:
        srcRot = gcvSURF_90_DEGREE;
        dstRot = gcvSURF_270_DEGREE;
        break;

    case 14:
        srcRot = gcvSURF_180_DEGREE;
        dstRot = gcvSURF_270_DEGREE;
        break;

    case 15:
        srcRot = gcvSURF_270_DEGREE;
        dstRot = gcvSURF_270_DEGREE;
        break;

    case 16:
        srcRot = gcvSURF_0_DEGREE;
        dstRot = gcvSURF_FLIP_X;
        break;

    case 17:
        srcRot = gcvSURF_0_DEGREE;
        dstRot = gcvSURF_FLIP_Y;
        break;

    case 18:
        srcRot = gcvSURF_FLIP_X;
        dstRot = gcvSURF_0_DEGREE;
        break;

    case 19:
        srcRot = gcvSURF_FLIP_Y;
        dstRot = gcvSURF_0_DEGREE;
        break;

    default:
        return gcvFALSE;
    }

    srcRect.left   = 0;
    srcRect.top    = 0;
    srcRect.right  = srcRect.left + rectWidth;
    srcRect.bottom = srcRect.top + rectHeight;

    dstRect.left   = 0;
    dstRect.top    = 0;
    dstRect.right  = dstRect.left + rectWidth;
    dstRect.bottom = dstRect.top + rectHeight;


    // start rendering sequence
    gcmONERROR(gco2D_SetColorSourceEx(egn2D,
                                      t2d->srcPhysAddr,
                                      t2d->srcStride,
                                      t2d->srcFormat,
                                      srcRot,
                                      t2d->srcWidth,
                                      t2d->srcHeight,
                                      gcvFALSE,
                                      gcvSURF_OPAQUE,
                                      0));

    gcmONERROR(gco2D_SetSource(egn2D, &srcRect));

    gcmONERROR(gco2D_SetTargetEx(egn2D,
                                 t2d->dstPhysAddr,
                                 t2d->dstStride,
                                 dstRot,
                                 t2d->dstAlignedWidth,
                                 t2d->dstAlignedHeight));


    gcmONERROR(gco2D_SetClipping(egn2D, gcvNULL));

    gcmONERROR(gco2D_Blit(egn2D,
                            1,
                            &dstRect,
                            0xCC,
                            0xCC,
                            t2d->dstFormat));

    gcmONERROR(gco2D_Flush(egn2D));
    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    if (gcmIS_ERROR(status)) {
        GalOutput(GalOutputType_Error, "Failed to render.\n");

        return gcvFALSE;
    }

    return gcvTRUE;

OnError:
    GalOutput(GalOutputType_Error | GalOutputType_Console,
        "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));

    return gcvFALSE;
}

static void CDECL Destroy(Test2D *t2d)
{
    gceSTATUS status = gcvSTATUS_OK;
    // unlock dest surface as necessary
    if (t2d->dstSurf && t2d->dstVirtAddr) {
        if (gcmIS_ERROR(gcoSURF_Unlock(t2d->dstSurf, t2d->dstVirtAddr)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock dstSurf failed:%s\n", GalStatusString(status));
        }
        t2d->dstVirtAddr = gcvNULL;
    }

    // destroy source surface
    if (t2d->srcSurf)
    {
        if (t2d->srcVirtAddr) {
            if (gcmIS_ERROR(gcoSURF_Unlock(t2d->srcSurf, t2d->srcVirtAddr)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock srcSurf failed:%s\n", GalStatusString(status));
            }
            t2d->srcVirtAddr = gcvNULL;
        }
        if (gcmIS_ERROR(gcoSURF_Destroy(t2d->srcSurf)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Surf failed:%s\n", GalStatusString(status));
        }
        t2d->srcSurf = gcvNULL;
    }

    free(t2d);
}

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    char *srcBmpfile = "resource/rotate.bmp";
    gceSTATUS status   = gcvSTATUS_OK;

    runtime->wholeDescription = (char*)malloc(strlen(s_CaseDescription) + 1);
    if (runtime->wholeDescription == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    memcpy(runtime->wholeDescription, s_CaseDescription, strlen(s_CaseDescription) + 1);

    memset(t2d, 0, sizeof(Test2D));

    t2d->runtime   = runtime;
    t2d->dstSurf   = runtime->target;
    t2d->dstFormat = runtime->format;

    // check the chip version
    if (gcoHAL_IsFeatureAvailable(runtime->hal, gcvFEATURE_2D_BITBLIT_FULLROTATION)!= gcvTRUE)
    {
        // the hardware does not supported all the rotations
        GalOutput(GalOutputType_Result | GalOutputType_Console, "the hardware has not all the rotations.\n");
        t2d->base.frameCount  = 4;
    }
    else
    {
        t2d->base.frameCount  = 20;
    }

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf,
                              &t2d->dstPhysAddr,
                              &t2d->dstVirtAddr));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstAlignedWidth,
                                        &t2d->dstAlignedHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_GetSize(t2d->dstSurf,
                                 &t2d->dstWidth,
                                 &t2d->dstHeight,
                                 gcvNULL));

    gcmONERROR(gcoSURF_GetFormat(t2d->dstSurf,
                                   gcvNULL,
                                   &t2d->dstFormat));

    // create source surface
    t2d->srcSurf = GalLoadDIB2Surface(t2d->runtime->hal, srcBmpfile);
    if (t2d->srcSurf == NULL)
    {
        GalOutput(GalOutputType_Error, "can not load %s\n", srcBmpfile);
        gcmONERROR(gcvSTATUS_NOT_FOUND);
    }

    gcmONERROR(gcoSURF_Lock(t2d->srcSurf,
                              &t2d->srcPhysAddr,
                              &t2d->srcVirtAddr));

    gcmONERROR(gcoSURF_GetSize(t2d->srcSurf,
                                 &t2d->srcWidth,
                                 &t2d->srcHeight,
                                 gcvNULL));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->srcSurf,
                                        &t2d->srcAlignedWidth,
                                        &t2d->srcAlignedHeight,
                                        &t2d->srcStride));

    gcmONERROR(gcoSURF_GetFormat(t2d->srcSurf,
                                   gcvNULL,
                                   &t2d->srcFormat));

    // Fill in the base info.
    t2d->base.render      = (PGalRender)Render;
    t2d->base.destroy     = (PGalDestroy)Destroy;
    t2d->base.description = s_CaseDescription;

    return gcvTRUE;

OnError:
    if (gcmIS_ERROR(status)) {
            GalOutput(GalOutputType_Error | GalOutputType_Console,
                "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));

        if (t2d->srcSurf) {
            if (t2d->srcVirtAddr) {
                gcmONERROR(gcoSURF_Unlock(t2d->srcSurf, t2d->srcVirtAddr));
                t2d->srcVirtAddr = gcvNULL;
            }

            if (gcmIS_ERROR(gcoSURF_Destroy(t2d->srcSurf)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Surf failed:%s\n", GalStatusString(status));
            }
            t2d->srcSurf = gcvNULL;
        }
    }

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
