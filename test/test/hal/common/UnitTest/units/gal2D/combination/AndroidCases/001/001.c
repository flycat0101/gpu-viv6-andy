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
"Case gal2DAndroidCases001\n" \
"Operation: Test multi-src blit/rotation/alphablend in the typical android way.\n" \
"2D API: gco2D_SetGenericSource gco2D_SetGenericTarget gco2D_MultiSourceBlit\n" \
"Src1: Size       [variable]\n"\
"     Rect        [variable]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0/90/180]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src2: Size       [variable]\n"\
"     Rect        [variable]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0/90/180]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src3: Size       [variable]\n"\
"     Rect        [variable]\n"\
"     Format      [ARGB8888\n"\
"     Rotation    [0/90/180]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src4: Size       [variable]\n"\
"     Rect        [variable]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0/90/180]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src5: Size       [variable]\n"\
"     Rect        [variable]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0/90/180]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src6: Size       [variable]\n"\
"     Rect        [variable]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0/90/180]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src7: Size       [variable]\n"\
"     Rect        [variable]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0/90/180]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src8: Size       [variable]\n"\
"     Rect        [variable]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0/90/180]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Dst: Size        [1920x1200]\n"\
"     Rect        [variable]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: Type   [SolidBrush]\n"\
"       Format [ARGB8888]\n"\
"       Offset [0]\n" \
"Alphablend: [disable/enable]\n" \
"HW feature dependency: ";

#define TARGET_WIDTH        1920
#define TARGET_HEIGHT       1200

typedef enum _T2DLayerType
{
    T2DLayerType_Blitter    = 0,
    T2DLayerType_Dim
}
T2DLayerType;

typedef struct _T2DLayer
{
    T2DLayerType        type;
    gcsRECT             srcRect;
    gcsRECT             dstRect;
    gceSURF_ROTATION    rotation;
    gctBOOL             horizontalMirror;
    gctBOOL             verticalMirror;

    union
    {
        struct _Blitter
        {
            gctCHAR     imageFile[MAX_BUFFER_SIZE];
            gctUINT32   stride;
            gctUINT32   alpha;
        } Blitter;

        struct _Dim
        {
            gctUINT32   color;
        } Dim;
    } u;
} T2DLayer;

typedef struct Test2D
{
    GalTest         base;
    GalRuntime *    runtime;

    gctCHAR         caseName[MAX_BUFFER_SIZE];
    gcsRECT         targetRect;

    T2DLayer        layer[8];
    T2D_SURF_PTR    surf[8];
    gctINT          count;

    gctUINT32       loop;
} Test2D;

static void ConstructBlitter(
    OUT T2DLayer * layer,
    IN gctCHAR * imageFile,
    IN gctUINT32 stride,
    IN gceSURF_ROTATION rotation,
    IN gctBOOL horizontalMirror,
    IN gctBOOL verticalMirror,
    IN gctUINT32 alpha,
    IN gctINT32 srcX0,
    IN gctINT32 srcY0,
    IN gctINT32 srcX1,
    IN gctINT32 srcY1,
    IN gctINT32 dstX0,
    IN gctINT32 dstY0,
    IN gctINT32 dstX1,
    IN gctINT32 dstY1
    )
{
    layer->type = T2DLayerType_Blitter;

    gcoOS_StrCopySafe(layer->u.Blitter.imageFile, gcmCOUNTOF(layer->u.Blitter.imageFile), "resource/android/");
    gcoOS_StrCatSafe(layer->u.Blitter.imageFile, gcmCOUNTOF(layer->u.Blitter.imageFile), imageFile);
    layer->srcRect.left     = srcX0;
    layer->srcRect.top      = srcY0;
    layer->srcRect.right    = srcX1;
    layer->srcRect.bottom   = srcY1;
    layer->dstRect.left     = dstX0;
    layer->dstRect.top      = dstY0;
    layer->dstRect.right    = dstX1;
    layer->dstRect.bottom   = dstY1;
    layer->rotation         = rotation;
    layer->horizontalMirror = horizontalMirror;
    layer->verticalMirror   = verticalMirror;
    layer->u.Blitter.stride = stride;
    layer->u.Blitter.alpha  = alpha;
}

static void ConstructDim(
    OUT T2DLayer * layer,
    IN gctUINT32 color,
    IN gctINT32 dstX0,
    IN gctINT32 dstY0,
    IN gctINT32 dstX1,
    IN gctINT32 dstY1
    )
{
    layer->type = T2DLayerType_Dim;

    layer->u.Dim.color      = color;
    layer->srcRect.left     = dstX0;
    layer->srcRect.top      = dstY0;
    layer->srcRect.right    = dstX1;
    layer->srcRect.bottom   = dstY1;
    layer->dstRect.left     = dstX0;
    layer->dstRect.top      = dstY0;
    layer->dstRect.right    = dstX1;
    layer->dstRect.bottom   = dstY1;
    layer->rotation         = gcvSURF_0_DEGREE;
    layer->horizontalMirror = gcvFALSE;
    layer->verticalMirror   = gcvFALSE;
}

typedef void (*InitCaseFunc)(Test2D *);

static void Init00(Test2D *t2d)
{
    /* Layer 0 */
    ConstructBlitter(&t2d->layer[0], "f8370_l0_h0x40000f20_p43346710_s1920x1128.bmp",
        3840, gcvSURF_180_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 1920, 1128, 0, 72, 1920, 1200);

    /* Layer 1 */
    ConstructBlitter(&t2d->layer[1], "f8370_l1_h0x457c6048_pd9733000_s1920x1128.bmp",
        7680, gcvSURF_180_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 1920, 1128, 0, 72, 1920, 1200);

    /* Layer 2 */
    ConstructBlitter(&t2d->layer[2], "f8370_l2_h0x457c8d48_pe03cf000_s318x534.bmp",
        1280, gcvSURF_180_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 318, 534, 0, 594, 318, 1128);

    /* Layer 3 */
    ConstructBlitter(&t2d->layer[3], "f8370_l3_h0x457c68b0_pdac15000_s1920x72.bmp",
        3840, gcvSURF_180_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 1920, 72, 0, 0, 1920, 72);

    /* Layer 4 */
    ConstructBlitter(&t2d->layer[4], "f8370_l4_h0x40004c20_pc35c3000_s900x1200.bmp",
        3648, gcvSURF_180_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 900, 1200, 1020, 0, 1920, 1200);

    /* case info. */
    t2d->count = 5;
    gcoOS_StrCopySafe(t2d->caseName, gcmCOUNTOF(t2d->caseName), "flip_gallery_and_show_recents");
}

static void Init01(Test2D *t2d)
{
    /* Layer 0 */
    ConstructBlitter(&t2d->layer[0], "f9715_l0_h0x40002e18_p43346710_s1920x1128.bmp",
        3840, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 1920, 1128, 0, 0, 1920, 1128);

    /* Layer 1 */
    ConstructDim(&t2d->layer[1], 0x99000000, 0, 0, 1920, 1128);

    /* Layer 2 */
    ConstructBlitter(&t2d->layer[2], "f9715_l2_h0x457c8d48_pb31bc000_s870x927.bmp",
        3520, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 870, 927, 525, 100, 1395, 1027);

    /* Layer 3 */
    ConstructBlitter(&t2d->layer[3], "f9715_l3_h0x40004c20_pba2d7000_s691x120.bmp",
        2816, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 691, 120, 614, 120, 1305, 240);

    /* Layer 4 */
    ConstructBlitter(&t2d->layer[4], "f9715_l4_h0x456eeaa0_pb67a8000_s1920x72.bmp",
        3840, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 1920, 72, 0, 1128, 1920, 1200);

    /* Layer 5 */
    ConstructBlitter(&t2d->layer[5], "f9715_l5_h0x457c68b0_p94c0c000_s768x1200.bmp",
        3072, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 768, 1200, 1152, 0, 1920, 1200);

    /* case info. */
    t2d->count = 6;
    gcoOS_StrCopySafe(t2d->caseName, gcmCOUNTOF(t2d->caseName), "gallery_details_and_status_and_volumn");
}

static void Init02(Test2D *t2d)
{
    /* Layer 0 */
    ConstructBlitter(&t2d->layer[0], "f7670_l0_h0x45700dd0_p43346710_s1920x1128.bmp",
        3840, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 1920, 1128, 0, 0, 1920, 1128);

    /* Layer 1 */
    ConstructBlitter(&t2d->layer[1], "f7670_l1_h0x456ec0c8_pba1d5000_s1920x1128.bmp",
        7680, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 1920, 1128, 0, 0, 1920, 1128);

    /* Layer 2 */
    ConstructBlitter(&t2d->layer[2], "f7670_l2_h0x457c6048_pac6cb000_s318x534.bmp",
        1280, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 318, 534, 1602, 72, 1920, 606);

    /* Layer 3 */
    ConstructBlitter(&t2d->layer[3], "f7670_l3_h0x45703960_pbd97f000_s1920x72.bmp",
        3840, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 1920, 72, 0, 1128, 1920, 1200);

    /* case info. */
    t2d->count = 4;
    gcoOS_StrCopySafe(t2d->caseName, gcmCOUNTOF(t2d->caseName), "gallery_show_menu");
}

static void Init03(Test2D *t2d)
{
    /* Layer 0 */
    ConstructBlitter(&t2d->layer[0], "f1204_l0_h0x456ff638_p43346710_s1920x1128.bmp",
        7680, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 1920, 1128, 0, 0, 1920, 1128);

    /* Layer 1 */
    ConstructBlitter(&t2d->layer[1], "f1204_l1_h0x457c68b0_ped1aa000_s1920x72.bmp",
        3840, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 1920, 72, 0, 1128, 1920, 1200);

    /* case info. */
    t2d->count = 2;
    gcoOS_StrCopySafe(t2d->caseName, gcmCOUNTOF(t2d->caseName), "launcher_app_list");
}

static void Init04(Test2D *t2d)
{
    /* Layer 0 */
    ConstructBlitter(&t2d->layer[0], "f781_l0_h0x456ff638_p43346710_s1920x1128.bmp",
        7680, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 1920, 1128, 0, 0, 1920, 1128);

    /* Layer 1 */
    ConstructBlitter(&t2d->layer[1], "f781_l1_h0x457c8328_ped1aa000_s1920x72.bmp",
        3840, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 1920, 72, 0, 1128, 1920, 1200);

    /* case info. */
    t2d->count = 2;
    gcoOS_StrCopySafe(t2d->caseName, gcmCOUNTOF(t2d->caseName), "launcher_desktop");
}

static void Init05(Test2D *t2d)
{
    /* Layer 0 */
    ConstructBlitter(&t2d->layer[0], "f2498_l0_h0x456ff430_p43346710_s1920x1128.bmp",
        7680, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 1920, 1128, 0, 0, 1920, 1128);

    /* Layer 1 */
    ConstructDim(&t2d->layer[1], 0x99000000, 0, 0, 1920, 1128);

    /* Layer 2 */
    ConstructBlitter(&t2d->layer[2], "f2498_l2_h0x456edb60_p8c4b2000_s870x368.bmp",
        3520, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 870, 368, 525, 380, 1395, 748);

    /* Layer 3 */
    ConstructBlitter(&t2d->layer[3], "f2498_l3_h0x457c8328_p84e99000_s1920x72.bmp",
        3840, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 1920, 72, 0, 1128, 1920, 1200);

    /* case info. */
    t2d->count = 4;
    gcoOS_StrCopySafe(t2d->caseName, gcmCOUNTOF(t2d->caseName), "launcher_select_wallpaper");
}

static void Init06(Test2D *t2d)
{
    /* Layer 0 */
    ConstructBlitter(&t2d->layer[0], "f509_l0_h0x456ff430_p43346710_s2880x1920.bmp",
        11520, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 0, 480, 396, 2400, 1524, 0, 0, 1920, 1128);

    /* Layer 1 */
    ConstructBlitter(&t2d->layer[1], "f509_l1_h0x457c6048_pf1fe0000_s1920x1128.bmp",
        7680, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 1920, 1128, 0, 0, 1920, 1128);

    /* Layer 2 */
    ConstructBlitter(&t2d->layer[2], "f509_l2_h0x457c68b0_pf85c9000_s1920x72.bmp",
        3840, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 1920, 72, 0, 1128, 1920, 1200);

    /* case info. */
    t2d->count = 3;
    gcoOS_StrCopySafe(t2d->caseName, gcmCOUNTOF(t2d->caseName), "launcher_unlock_ui");
}

static void Init07(Test2D *t2d)
{
    /* Layer 0 */
    ConstructBlitter(&t2d->layer[0], "f3268_l0_h0x40000ad0_p43346710_s1920x1128.bmp",
        3840, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 1920, 1128, 0, 0, 1920, 1128);

    /* Layer 1 */
    ConstructBlitter(&t2d->layer[1], "f3268_l1_h0x457c8328_p90d37000_s1920x72.bmp",
        3840, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 1920, 72, 0, 1128, 1920, 1200);

    /* case info. */
    t2d->count = 2;
    gcoOS_StrCopySafe(t2d->caseName, gcmCOUNTOF(t2d->caseName), "nenamark2_running");
}

static void Init08(Test2D *t2d)
{
    /* Layer 0 */
    ConstructBlitter(&t2d->layer[0], "f3985_l0_h0x40000ad0_p43346710_s1920x1128.bmp",
        3840, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 1920, 1128, 0, 0, 1920, 1128);

    /* Layer 1 */
    ConstructBlitter(&t2d->layer[1], "f3985_l1_h0x457c68b0_p90d37000_s1920x1128.bmp",
        7680, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 1920, 1128, 0, 0, 1920, 1128);

    /* Layer 2 */
    ConstructBlitter(&t2d->layer[2], "f3985_l2_h0x457c8328_p9adcb000_s1920x72.bmp",
        3840, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 1920, 72, 0, 1128, 1920, 1200);

    /* case info. */
    t2d->count = 3;
    gcoOS_StrCopySafe(t2d->caseName, gcmCOUNTOF(t2d->caseName), "nenamark2_submit");
}

static void Init09(Test2D *t2d)
{
    /* Layer 0 */
    ConstructBlitter(&t2d->layer[0], "f10691_l0_h0x457c68b0_p43346710_s1200x1848.bmp",
        4800, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 1848, 1200, 0, 0, 1848, 1200);

    /* Layer 1 */
    ConstructDim(&t2d->layer[1], 0x99000000, 0, 0, 1848, 1200);

    /* Layer 2 */
    ConstructBlitter(&t2d->layer[2], "f10691_l2_h0x40000f20_p8cd19000_s851x631.bmp",
        3456, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 631, 851, 608, 175, 1239, 1026);

    /* Layer 3 */
    ConstructBlitter(&t2d->layer[3], "f10691_l3_h0x40000ad0_pa255c000_s1200x72.bmp",
        2400, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 72, 1200, 1848, 0, 1920, 1200);

    /* case info. */
    t2d->count = 4;
    gcoOS_StrCopySafe(t2d->caseName, gcmCOUNTOF(t2d->caseName), "quadrant_about");
}

static void Init10(Test2D *t2d)
{
    /* Layer 0 */
    ConstructBlitter(&t2d->layer[0], "f11294_l0_h0x456ec0c8_p43346710_s1200x1828.bmp",
        2400, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 1828, 1200, 0, 0, 1828, 1200);

    /* Layer 1 */
    ConstructBlitter(&t2d->layer[1], "f11294_l1_h0x457c6048_pa997b000_s1200x1848.bmp",
        4800, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 1848, 1200, 0, 0, 1848, 1200);

    /* Layer 2 */
    ConstructBlitter(&t2d->layer[2], "f11294_l2_h0x40000ad0_p9cf20000_s1200x72.bmp",
        2400, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 72, 1200, 1848, 0, 1920, 1200);

    /* case info. */
    t2d->count = 3;
    gcoOS_StrCopySafe(t2d->caseName, gcmCOUNTOF(t2d->caseName), "quadrant_running");
}

static void Init11(Test2D *t2d)
{
    /* Layer 0 */
    ConstructBlitter(&t2d->layer[0], "f9129_l0_h0x456ec0c8_p43346710_s1200x1848.bmp",
        2400, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 1848, 1200, 0, 0, 1848, 1200);

    /* Layer 1 */
    ConstructDim(&t2d->layer[1], 0x99000000, 0, 0, 1848, 1200);

    /* Layer 2 */
    ConstructBlitter(&t2d->layer[2], "f9129_l2_h0x457c8d48_p9cac5000_s870x927.bmp",
        3520, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 927, 870, 460, 165, 1387, 1035);

    /* Layer 3 */
    ConstructBlitter(&t2d->layer[3], "f9129_l3_h0x40002e18_p98b2f000_s691x120.bmp",
        2816, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 120, 691, 120, 255, 240, 946);

    /* Layer 4 */
    ConstructBlitter(&t2d->layer[4], "f9129_l4_h0x457c68b0_p96c87000_s1200x72.bmp",
        2400, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 72, 1200, 1848, 0, 1920, 1200);

    /* Layer 5 */
    ConstructBlitter(&t2d->layer[5], "f9129_l5_h0x456ff430_p89a31000_s900x1920.bmp",
        3648, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 1920, 900, 0, 300, 1920, 1200);

    /* case info. */
    t2d->count = 6;
    gcoOS_StrCopySafe(t2d->caseName, gcmCOUNTOF(t2d->caseName), "rot_gallery_details_and_recents_and_volumn");
}

static void Init12(Test2D *t2d)
{
    /* Layer 0 */
    ConstructBlitter(&t2d->layer[0], "f8925_l0_h0x457c6048_p43346710_s1200x1848.bmp",
        2400, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 1848, 1200, 0, 0, 1848, 1200);

    /* Layer 1 */
    ConstructBlitter(&t2d->layer[1], "f8925_l1_h0x43348b20_pa275d000_s1200x1848.bmp",
        4800, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 1848, 1200, 0, 0, 1848, 1200);

    /* Layer 2 */
    ConstructDim(&t2d->layer[2], 0x99000000, 0, 0, 1848, 1200);

    /* Layer 3 */
    ConstructBlitter(&t2d->layer[3], "f8925_l3_h0x456ec0c8_p8cf42000_s870x368.bmp",
        3520, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 368, 870, 740, 165, 1108, 1035);

    /* Layer 4 */
    ConstructBlitter(&t2d->layer[4], "f8925_l4_h0x400034a8_p9ec30000_s1200x72.bmp",
        2400, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 72, 1200, 1848, 0, 1920, 1200);

    /* case info. */
    t2d->count = 5;
    gcoOS_StrCopySafe(t2d->caseName, gcmCOUNTOF(t2d->caseName), "rot_gallery_saveas");
}

static void Init13(Test2D *t2d)
{
    /* Layer 0 */
    ConstructBlitter(&t2d->layer[0], "f7846_l0_h0x45703428_p43346710_s1200x1848.bmp",
        2400, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 1848, 1200, 0, 0, 1848, 1200);

    /* Layer 1 */
    ConstructDim(&t2d->layer[1], 0x99000000, 0, 0, 1848, 1200);

    /* Layer 2 */
    ConstructBlitter(&t2d->layer[2], "f7846_l2_h0x456ff430_pc7dcb000_s870x927.bmp",
        3520, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 927, 870, 460, 165, 1387, 1035);

    /* Layer 3 */
    ConstructBlitter(&t2d->layer[3], "f7846_l3_h0x43348008_pbc1ae000_s691x120.bmp",
        2816, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 120, 691, 120, 255, 240, 946);

    /* Layer 4 */
    ConstructBlitter(&t2d->layer[4], "f7846_l4_h0x40000f20_pc5e9f000_s1200x72.bmp",
        2400, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 72, 1200, 1848, 0, 1920, 1200);

    /* case info. */
    t2d->count = 5;
    gcoOS_StrCopySafe(t2d->caseName, gcmCOUNTOF(t2d->caseName), "rot_gallery_show_details_and_volumn");
}

static void Init14(Test2D *t2d)
{
    /* Layer 0 */
    ConstructBlitter(&t2d->layer[0], "f5489_l0_h0x457c6048_p43346710_s1200x1848.bmp",
        4800, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 1848, 1200, 0, 0, 1848, 1200);

    /* Layer 1 */
    ConstructBlitter(&t2d->layer[1], "f5489_l1_h0x45700dd0_pbd8e1000_s1200x72.bmp",
        2400, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 72, 1200, 1848, 0, 1920, 1200);

    /* case info. */
    t2d->count = 2;
    gcoOS_StrCopySafe(t2d->caseName, gcmCOUNTOF(t2d->caseName), "rot_launcher_desktop");
}

static void Init15(Test2D *t2d)
{
    /* Layer 0 */
    ConstructBlitter(&t2d->layer[0], "f5764_l0_h0x40000ad0_p43346710_s1200x1848.bmp",
        4800, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 1848, 1200, 0, 0, 1848, 1200);

    /* Layer 1 */
    ConstructDim(&t2d->layer[1], 0x99000000, 0, 0, 1848, 1200);

    /* Layer 2 */
    ConstructBlitter(&t2d->layer[2], "f5764_l2_h0x45702c20_pd26a1000_s870x368.bmp",
        3520, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 368, 870, 740, 165, 1108, 1035);

    /* Layer 3 */
    ConstructBlitter(&t2d->layer[3], "f5764_l3_h0x457c8d48_pce228000_s1200x72.bmp",
        2400, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 72, 1200, 1848, 0, 1920, 1200);

    /* case info. */
    t2d->count = 4;
    gcoOS_StrCopySafe(t2d->caseName, gcmCOUNTOF(t2d->caseName), "rot_launcher_select_wallpaper");
}

static void Init16(Test2D *t2d)
{
    /* Layer 0 */
    ConstructBlitter(&t2d->layer[0], "f12418_l0_h0x457c8d48_p43346710_s1200x1848.bmp",
        4800, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 1848, 1200, 0, 0, 1848, 1200);

    /* Layer 1 */
    ConstructDim(&t2d->layer[1], 0x99000000, 0, 0, 1848, 1200);

    /* Layer 2 */
    ConstructBlitter(&t2d->layer[2], "f12418_l2_h0x40000ad0_pcf282000_s870x514.bmp",
        3520, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 514, 870, 440, 165, 954, 1035);

    /* Layer 3 */
    ConstructBlitter(&t2d->layer[3], "f12418_l3_h0x40000f20_pafb87000_s1200x924.bmp",
        4800, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 924, 1200, 924, 0, 1848, 1200);

    /* Layer 4 */
    ConstructBlitter(&t2d->layer[4], "f12418_l4_h0x40004540_pba755000_s870x316.bmp",
        3520, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 316, 870, 766, 165, 1082, 1035);

    /* Layer 5 */
    ConstructBlitter(&t2d->layer[5], "f12418_l5_h0x456f1bd0_pb2f7c000_s691x120.bmp",
        2816, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 120, 691, 120, 255, 240, 946);

    /* Layer 6 */
    ConstructBlitter(&t2d->layer[6], "f12418_l6_h0x457c8328_pc878b000_s1200x72.bmp",
        2400, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 72, 1200, 1848, 0, 1920, 1200);

    /* Layer 7 */
    ConstructBlitter(&t2d->layer[7], "f12418_l7_h0x456f0b30_paa5b9000_s900x1920.bmp",
        3648, gcvSURF_90_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 1920, 900, 0, 300, 1920, 1200);

    /* case info. */
    t2d->count = 8;
    gcoOS_StrCopySafe(t2d->caseName, gcmCOUNTOF(t2d->caseName), "rot_settings_connect_wifi_and_recents_and_volumn");
}

static void Init17(Test2D *t2d)
{
    /* Layer 0 */
    ConstructBlitter(&t2d->layer[0], "f1823_l0_h0x45700e38_p43346710_s1920x1128.bmp",
        7680, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 1920, 1128, 0, 0, 1920, 1128);

    /* Layer 1 */
    ConstructDim(&t2d->layer[1], 0x99000000, 0, 0, 1920, 1128);

    /* Layer 2 */
    ConstructBlitter(&t2d->layer[2], "f1823_l2_h0x457c3410_pda47b000_s870x514.bmp",
        3520, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 870, 514, 525, 48, 1395, 562);

    /* Layer 3 */
    ConstructBlitter(&t2d->layer[3], "f1823_l3_h0x45700068_pd8707000_s1920x989.bmp",
        7680, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 1, 0, 0, 1920, 989, 0, 139, 1920, 1128);

    /* Layer 4 */
    ConstructBlitter(&t2d->layer[4], "f1823_l4_h0x457c8328_pd7753000_s1920x72.bmp",
        3840, gcvSURF_0_DEGREE, gcvFALSE, gcvFALSE, 0, 0, 0, 1920, 72, 0, 1128, 1920, 1200);

    /* case info. */
    t2d->count = 5;
    gcoOS_StrCopySafe(t2d->caseName, gcmCOUNTOF(t2d->caseName), "settings_connect_to_wifi");
}

static InitCaseFunc initCaseFunc[] =
{
    Init00, Init01, Init02, Init03, Init04, Init05, Init06, Init07, Init08,
    Init09, Init10, Init11, Init12, Init13, Init14, Init15, Init16, Init17,
};

static gceSTATUS InitSourceSurface(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status = gcvSTATUS_OK;
    gco2D egn2D = t2d->runtime->engine2d;
    gctINT i;
    T2D_SURF_PTR surf = gcvNULL;

    gcmASSERT(frameNo < gcmCOUNTOF(initCaseFunc));

    /* Init the source surface. */
    (*initCaseFunc[frameNo])(t2d);

    for (i = 0; i < t2d->count; i++)
    {
        if (t2d->layer[i].type == T2DLayerType_Dim)
        {
            gcsRECT rect = t2d->targetRect;

            gcmONERROR(GalCreateTSurf(
                t2d->runtime->hal, gcvSURF_A8R8G8B8, gcvLINEAR, gcv2D_TSC_DISABLE,
                TARGET_WIDTH, TARGET_HEIGHT, &surf));

            gcmONERROR(gco2D_SetClipping(egn2D, &rect));

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

            gcmONERROR(gco2D_LoadSolidBrush(egn2D, surf->format, gcvTRUE,
                t2d->layer[i].u.Dim.color, 0));

            gcmONERROR(gco2D_Blit(egn2D, 1, &rect, 0xF0, 0xF0, surf->format));
        }
        else if (t2d->layer[i].type == T2DLayerType_Blitter)
        {
            gcmONERROR(GalLoadFileToTSurf(t2d->layer[i].u.Blitter.imageFile, &surf));
        }
        else
        {
            gcmASSERT(0);
        }

        t2d->surf[i] = surf;
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
            if (gcmIS_ERROR(GalDeleteTSurf(t2d->runtime->hal, t2d->surf[i])))
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
    gctINT i;
    T2D_SURF_PTR result = gcvNULL;
    gcsRECT rect;
    gctUINT32 start, end;
    gctUINT32 n;

    T2DAreaContext ctx;
    T2DAreaPool *pAreaPool;
    T2DArea *pArea;

    T2DLayer *layer;

    memset(&ctx, 0, sizeof(ctx));

    /* Init the source surface. */
    gcmONERROR(InitSourceSurface(t2d, frameNo));

    /* Create the result surface. */
    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal, gcvSURF_A8R8G8B8, gcvLINEAR, gcv2D_TSC_DISABLE,
        TARGET_WIDTH, TARGET_HEIGHT, &result));

    /* Split the dest rectangles into areas. */
    pArea = GalAllocateArea(&ctx, gcvNULL, &t2d->layer[0].dstRect, 1);

    for (i = 1; i < t2d->count; i++)
    {
        GalSplitArea(&ctx, pArea, &t2d->layer[i].dstRect, 1 << i);
    }


    /* Set the source surface. */
    for (i = 0; i < (gctINT)gcmCOUNTOF(t2d->surf); i++)
    {
        if (t2d->surf[i] == gcvNULL)
        {
            continue;
        }

        layer = &t2d->layer[i];

        gcmONERROR(gco2D_SetCurrentSourceIndex(egn2D, i));

        gcmONERROR(gco2D_SetGenericSource(
            egn2D,
            t2d->surf[i]->address, t2d->surf[i]->validAddressNum,
            t2d->surf[i]->stride, t2d->surf[i]->validStrideNum,
            t2d->surf[i]->tiling, t2d->surf[i]->format,
            layer->rotation,
            t2d->surf[i]->width, t2d->surf[i]->height));

        gcmONERROR(gco2D_SetSourceTileStatus(
            egn2D,
            t2d->surf[i]->tileStatusConfig,
            t2d->surf[i]->format,
            t2d->surf[i]->tileStatusClear,
            t2d->surf[i]->tileStatusAddress
            ));

        gcmONERROR(gco2D_SetSource(egn2D, &layer->srcRect));

        gcmONERROR(gco2D_SetROP(egn2D, 0xCC, 0xCC));

        if (layer->type == T2DLayerType_Blitter)
        {
            if (layer->u.Blitter.alpha)
            {
                /* Cs + (1 - As) x Cd */
                gcmONERROR(gco2D_EnableAlphaBlend(egn2D,
                            0, 0,
                            gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
                            gcvSURF_GLOBAL_ALPHA_OFF, gcvSURF_GLOBAL_ALPHA_OFF,
                            gcvSURF_BLEND_ONE, gcvSURF_BLEND_INVERSED,
                            gcvSURF_COLOR_STRAIGHT, gcvSURF_COLOR_STRAIGHT));
            }
            else
            {
                gcmONERROR(gco2D_DisableAlphaBlend(egn2D));
            }
        }
        else if (layer->type == T2DLayerType_Dim)
        {
            /* (1 - As) x Cs + Cd */
            gcmONERROR(gco2D_EnableAlphaBlend(egn2D,
                        0, 0,
                        gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
                        gcvSURF_GLOBAL_ALPHA_OFF, gcvSURF_GLOBAL_ALPHA_OFF,
                        gcvSURF_BLEND_STRAIGHT, gcvSURF_BLEND_INVERSED,
                        gcvSURF_COLOR_STRAIGHT, gcvSURF_COLOR_STRAIGHT));
        }
        else
        {
            gcmASSERT(0);
        }

        gcmONERROR(gco2D_SetBitBlitMirror(egn2D, layer->horizontalMirror, layer->verticalMirror));
    }

    /* Set the target surface. */
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
        gcvSURF_0_DEGREE,
        result->tileStatusAddress
        ));

    rect.left   = 0;
    rect.top    = 0;
    rect.right  = result->width;
    rect.bottom = result->height;

    gcmONERROR(gco2D_SetClipping(egn2D, &rect));

    if (t2d->loop > 1)
    {
        start = gcoOS_GetTicks();
    }

    for (n = 0; n < t2d->loop; n++)
    {
        /* Loop all the areas. */
        pAreaPool = ctx.areaPool;

        while (pAreaPool != gcvNULL)
        {
            pArea = pAreaPool->areas;
            while (pArea != gcvNULL)
            {
                for (i = 0; i < (gctINT)gcmCOUNTOF(t2d->surf); i++)
                {
                    if ((1 << i) & pArea->owners)
                    {
                        layer = &t2d->layer[i];

                        rect.left   = pArea->rect.left    + (layer->srcRect.left   - layer->dstRect.left);
                        rect.top    = pArea->rect.top     + (layer->srcRect.top    - layer->dstRect.top);
                        rect.right  = pArea->rect.right   + (layer->srcRect.right  - layer->dstRect.right);
                        rect.bottom = pArea->rect.bottom  + (layer->srcRect.bottom - layer->dstRect.bottom);

                        gcmONERROR(gco2D_SetCurrentSourceIndex(egn2D, i));

                        gcmONERROR(gco2D_SetSource(egn2D, &rect));
                    }
                }

                gcmONERROR(gco2D_MultiSourceBlit(egn2D, pArea->owners, &pArea->rect, 1));

                pArea = pArea->next;
            }
            pAreaPool = pAreaPool->next;
        }
    }

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    if (t2d->loop > 1)
    {
        end = gcoOS_GetTicks();

        GalOutput(GalOutputType_Log | GalOutputType_Console,
                "FrameNo%3d:  %dms (%d~%d) for %d loops",
                frameNo,
                end - start, start, end,
                t2d->loop);
    }

    /* Save the result. */
    if (t2d->runtime->saveFullName)
    {
        GalSaveTSurfToDIB(result, t2d->runtime->saveFullName);
    }

OnError:
    if (result != gcvNULL)
    {
        GalDeleteTSurf(gcvNULL, result);
    }

    GalFreeArea(&ctx);

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
    gcvFEATURE_2D_MULTI_SRC_BLT_TO_UNIFIED_DST_RECT,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status;
    gctINT argc     = runtime->argc;
    gctSTRING *argv = runtime->argv;
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

    /* Set the state. */
    gcmONERROR(gco2D_SetStateU32(runtime->engine2d,
                                 gcv2D_STATE_MULTI_SRC_BLIT_UNIFIED_DST_RECT,
                                 gcvTRUE));

    t2d->loop = 1;

    for (i = 0; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-loop"))
        {
            t2d->loop = atoi(argv[++i]);
        }
    }

    if (t2d->loop < 1)
    {
        t2d->loop = 1;
    }

    t2d->runtime           = runtime;

    runtime->saveTarget    = gcvFALSE;
    runtime->cleanTarget   = gcvFALSE;

    t2d->targetRect.left   = 0;
    t2d->targetRect.top    = 0;
    t2d->targetRect.right  = TARGET_WIDTH;
    t2d->targetRect.bottom = TARGET_HEIGHT;

    t2d->base.render       = (PGalRender)Render;
    t2d->base.destroy      = (PGalDestroy)Destroy;
    t2d->base.frameCount   = gcmCOUNTOF(initCaseFunc);
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
