/****************************************************************************
*
*    Copyright 2012 - 2016 Vivante Corporation, Santa Clara, California.
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
 *  Feature:    LINE - source
 *  API:        gco2D_Line
 *  Check:      foreground ROP and background ROP
*/
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DLine004\n" \
"Operation: Test draw line with source.\n" \
"2D API: gco2D_Line gco2D_SetColorSource\n" \
"Src: Size        [configurable]\n"\
"     Rect        [configurable]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"     Transparency[patternMask]\n" \
"Dst: Size        [configurable]\n"\
"     Rect        [configurable]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: Type   [SolidBrush]\n"\
"       Format [ARGB8888]\n"\
"       Offset [0]\n" \
"Alphablend: [disable]\n" \
"HW feature dependency: ";

typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;

	// destination surface
    gcoSURF			dstSurf;
	gceSURF_FORMAT	dstFormat;
	gctUINT			dstWidth;
	gctUINT			dstHeight;
	gctINT			dstStride;
	gctUINT32		dstPhyAddr;
	gctPOINTER		dstLgcAddr;

	//source surface
    gcoSURF			srcSurf;
	gceSURF_FORMAT	srcFormat;
	gctUINT			srcWidth;
	gctUINT			srcHeight;
	gctINT			srcStride;
	gctUINT32		srcPhyAddr;
	gctPOINTER		srcLgcAddr;
} Test2D;

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
	gco2D egn2D = t2d->runtime->engine2d;
	gcsRECT dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};
	gcsRECT line;
	gctUINT32 i;
	gceSTATUS status;
	gcoBRUSH brush1, brush2;

	gcmONERROR(Gal2DCleanSurface(t2d->runtime->hal, t2d->dstSurf, COLOR_ARGB8(0x00, 0x80, 0x80, 0x80)));

	gcmONERROR(gco2D_ConstructSingleColorBrush(egn2D, gcvTRUE,
					COLOR_ARGB8(0, 0, 0, 0xff), 0xFFFFFFFF00000000LL, &brush1));

	gcmONERROR(gco2D_ConstructSingleColorBrush(egn2D, gcvTRUE,
					COLOR_ARGB8(0, 0xff, 0, 0), 0xF0F0F0F0F0F0F0F0LL, &brush2));

	gco2D_SetColorSource(egn2D, t2d->srcPhyAddr, t2d->srcStride, t2d->srcFormat,
					gcvSURF_0_DEGREE, t2d->srcWidth, gcvFALSE, gcvSURF_PATTERN_MASK, 0);

	gcmONERROR(gco2D_SetTarget(egn2D, t2d->dstPhyAddr, t2d->dstStride, gcvSURF_0_DEGREE, t2d->dstWidth));

	gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

	//draw vertical line
	line.left = 0;
	line.top = 5;
	line.right = 0;
	line.bottom = t2d->dstHeight - 5;

	for (i = 0; i < t2d->dstWidth/10; i++)
	{
		// Draw one line
		gcmONERROR(gco2D_Line(egn2D, 1, &line, brush1, 0xF0, 0xAA,t2d->dstFormat));

		// next line
		line.left    += 10;
		line.right += 10;
	}

	//draw horizontal line
	line.left = 5;
	line.top = 0;
	line.right = t2d->dstWidth - 5;
	line.bottom = 0;

	for (i = 0; i < t2d->dstHeight/10; i++)
	{
		// Draw one line
		gcmONERROR(gco2D_Line(egn2D, 1, &line, brush2, 0xF0, 0xAA,t2d->dstFormat));

		// next line
		line.top    += 10;
		line.bottom += 10;
	}

	gcmONERROR(gco2D_Flush(egn2D));

	gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

	gcmONERROR(gcoBRUSH_Destroy(brush1));

	gcmONERROR(gcoBRUSH_Destroy(brush2));

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

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_2D_NO_COLORBRUSH_INDEX8,
    gcvFEATURE_ANDROID_ONLY,
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

	// create source surface
	t2d->srcFormat = t2d->runtime->format;
    gcmONERROR(gcoSURF_Construct(t2d->runtime->hal,
                               t2d->runtime->width,
                               t2d->runtime->height,
                               1,
                               gcvSURF_BITMAP,
                               t2d->srcFormat,
                               gcvPOOL_DEFAULT,
                               &t2d->srcSurf));

	gcmONERROR(gcoSURF_GetAlignedSize(t2d->srcSurf,
										&t2d->srcWidth,
										&t2d->srcHeight,
										&t2d->srcStride));

	gcmONERROR(gcoSURF_Lock(t2d->srcSurf, &t2d->srcPhyAddr, &t2d->srcLgcAddr));

	gcmONERROR(Gal2DCleanSurface(t2d->runtime->hal, t2d->srcSurf, COLOR_ARGB8(0x00, 0xFF, 0xFF, 0xFF)));

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = 1;
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
