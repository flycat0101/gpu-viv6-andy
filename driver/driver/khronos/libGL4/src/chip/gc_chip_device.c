/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


/*
*** Fill global information when process is attached
*** all DP related globals need to be in this structure
*/

#include "gc_es_context.h"
#include "gc_chip_context.h"
#include "gc_es_device.h"
#if defined(_LINUX_) && defined(GL4_DRI_BUILD)
#elif defined(_WINDOWS)
#include "vvtpfdtypes.h"
#include "wintogl.h"
#endif

#define _GC_OBJ_ZONE    gcdZONE_GL40_DEVICE

__GLchipGlobal dpGlobalInfo;
#if defined(_LINUX_) && defined(GL4_DRI_BUILD)
extern void __glChipCreateDrawable(__GLdrawablePrivate *draw,void *window);
#elif defined(_WINDOWS)
extern VEGLimports imports;

#define START_INDEX_OF_NONEDISPLAYABLE_COLOR   7

static glsCONFIGDEPTH configDepth[] =
{
    {  0, 0 },  /*       */
    { 16, 0 },  /* D16   */
    { 24, 0 },  /* D24X8 */
    { 24, 8 },  /* D24S8 */
};

static glsCONFIGCOLORALPHA configColorAlpha[] =
{
    {16, 4, 8, 4, 4, 4, 0, 0, 0},   /* X1R4G4B4 */
    {16, 4, 8, 4, 4, 4, 0, 4, 12},  /* A1R4B4G4 */
    {16, 5, 10, 5, 5, 5, 0, 0, 0},  /* X1R5B5G5 */
    {16, 5, 10, 5, 5, 5, 0, 1, 15}, /* A1R5B5G5 */
    {16, 5, 11, 6, 5, 5, 0, 0, 0},  /* R5B6G5   */
    {32, 8, 16, 8, 8, 8, 0, 0, 0},  /* X8R8G8B8 */
    {32, 8, 16, 8, 8, 8, 0, 8, 24}, /* A8R8G8B8 */
    /* none displayable float format */
    {64, 16, 0, 16, 16, 16, 0, 16, 16},
    {96, 16, 0, 16, 0, 16, 0, 0},
    {128, 32, 0, 32, 0, 32, 0, 0}
};

static glsCONFIGACCUM configAccum[] =
{
    {0, 0, 0, 0,0},
    {32, 8, 8, 8,8},
};

static GLuint configFlags[] =
{
    PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_COMPOSITION,
    PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER | PFD_SUPPORT_COMPOSITION | PFD_SWAP_COPY,

    PFD_SUPPORT_OPENGL | PFD_SUPPORT_COMPOSITION,
    PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_SUPPORT_COMPOSITION | PFD_SWAP_COPY
};

static glsCONFIGMULTISAMPLE configMultipleSample[] =
{
    {0, 0},
    {PFD_VIV_SAMPLE_BUFFERS_ARB, 2},
    {PFD_VIV_SAMPLE_BUFFERS_ARB, 4},
    {PFD_VIV_SAMPLE_BUFFERS_ARB, 8},
    {PFD_VIV_SAMPLE_BUFFERS_ARB, 16}
};

static BYTE configPixelType[] =
{
    PFD_TYPE_RGBA,
    PFD_VIV_BUFFERS_FLOAT_ARB
};

gceSTATUS fillPixelFormat(__VIV2DPixelFormat * pixelFormat, GLuint colorAlphaIndex, GLuint depthIndex,
                          GLuint flagIndex, GLuint pixelTypeIndex, GLuint sampleIndex, GLuint accumIndex)
{

    pixelFormat->extFlags = configMultipleSample[sampleIndex].extFlags;
    pixelFormat->sampleQuality = configMultipleSample[sampleIndex].sampleQuality;
    pixelFormat->pixelformatDesc.dwFlags = configFlags[flagIndex];
    pixelFormat->pixelformatDesc.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pixelFormat->pixelformatDesc.nVersion = 1;
    pixelFormat->pixelformatDesc.iPixelType = configPixelType[pixelTypeIndex];
    pixelFormat->pixelformatDesc.cColorBits = configColorAlpha[colorAlphaIndex].cColorBits;
    pixelFormat->pixelformatDesc.cAlphaBits = configColorAlpha[colorAlphaIndex].cAlphaBits;
    pixelFormat->pixelformatDesc.cAlphaShift = configColorAlpha[colorAlphaIndex].cAlphaShift;
    pixelFormat->pixelformatDesc.cRedBits = configColorAlpha[colorAlphaIndex].cRedBits;
    pixelFormat->pixelformatDesc.cRedShift = configColorAlpha[colorAlphaIndex].cRedShift;
    pixelFormat->pixelformatDesc.cGreenBits = configColorAlpha[colorAlphaIndex].cGreenBits;
    pixelFormat->pixelformatDesc.cGreenShift = configColorAlpha[colorAlphaIndex].cGreenShift;
    pixelFormat->pixelformatDesc.cBlueBits = configColorAlpha[colorAlphaIndex].cBlueBits;
    pixelFormat->pixelformatDesc.cBlueShift = configColorAlpha[colorAlphaIndex].cBlueShift;
    pixelFormat->pixelformatDesc.cAccumBits = configAccum[accumIndex].cAccumBits;
    pixelFormat->pixelformatDesc.cAccumRedBits = configAccum[accumIndex].cAccumRedBits;
    pixelFormat->pixelformatDesc.cAccumGreenBits = configAccum[accumIndex].cAccumGreenBits;
    pixelFormat->pixelformatDesc.cAccumBlueBits = configAccum[accumIndex].cAccumBlueBits;
    pixelFormat->pixelformatDesc.cDepthBits = configDepth[depthIndex].depthSize;
    pixelFormat->pixelformatDesc.cStencilBits = configDepth[depthIndex].stencilSize;
    pixelFormat->pixelformatDesc.iLayerType = PFD_MAIN_PLANE;
    return gcvSTATUS_OK;
}

GLuint constructPixelFormatTable(__VIV2DPixelFormat * pPixelFormatList, GLuint bpp, BOOL bAccumSupport)
{
    GLuint colorAlphaIndex, depthIndex, flagIndex, sampleIndex, accumIndex;
    GLuint totalTableEntriese = 0;
    GLuint maxSamples;
    GLuint accumTableSize = 0;
    __VIV2DPixelFormat * pcurrentPixelFormat = pPixelFormatList;

    gcoHAL_QueryTargetCaps(
            gcvNULL, gcvNULL,
            gcvNULL, gcvNULL, &maxSamples);

    if (bAccumSupport) {
        accumTableSize = sizeof(configAccum) / sizeof(configAccum[0]);
    }
    for (sampleIndex = 0; sampleIndex < (sizeof(configMultipleSample) / sizeof(configMultipleSample[0])); sampleIndex++)
    {
        if (maxSamples < configMultipleSample[sampleIndex].sampleQuality) {
            continue;
        }
        for (colorAlphaIndex = 0; colorAlphaIndex < (sizeof(configColorAlpha) / sizeof(configColorAlpha[0])); colorAlphaIndex++)
        {
            if (configColorAlpha[colorAlphaIndex].cColorBits != bpp) {
                continue;
            }
            for (accumIndex = 0; accumIndex < accumTableSize; accumIndex++)
            {
                for (depthIndex = 0; depthIndex < (sizeof(configDepth) / sizeof(configDepth[0])); depthIndex++)
                {
                    for (flagIndex = 0; flagIndex < 2; flagIndex++)
                    {
                        totalTableEntriese++;
                    }
                }
            }
        }
    }

    if (pPixelFormatList) {
        for (sampleIndex = 0; sampleIndex < (sizeof(configMultipleSample) / sizeof(configMultipleSample[0])); sampleIndex++)
        {
            if (maxSamples < configMultipleSample[sampleIndex].sampleQuality) {
                continue;
            }
            for (colorAlphaIndex = 0; colorAlphaIndex < (sizeof(configColorAlpha) / sizeof(configColorAlpha[0])); colorAlphaIndex++)
            {
                if (configColorAlpha[colorAlphaIndex].cColorBits != bpp) {
                    continue;
                }
                for (accumIndex = 0; accumIndex < accumTableSize; accumIndex++)
                {
                    for (depthIndex = 0; depthIndex < (sizeof(configDepth) / sizeof(configDepth[0])); depthIndex++)
                    {
                        for (flagIndex = 0; flagIndex < 2; flagIndex++)
                        {
                            fillPixelFormat(pcurrentPixelFormat, colorAlphaIndex, depthIndex, flagIndex, 0, sampleIndex, accumIndex);
                            pcurrentPixelFormat++;
                        }
                    }
                }
            }
        }
        return totalTableEntriese;
    } else {
        return totalTableEntriese * sizeof(__VIV2DPixelFormat);
    }
}

GLuint constructNoneDislayablePixelFormatTable(__VIV2DPixelFormat * pPixelFormatList, GLuint bpp, BOOL bAccumSupport)
{
    GLint colorAlphaIndex, depthIndex, flagIndex, sampleIndex, accumIndex;
    GLuint totalTableEntriese = 0;
    GLuint maxSamples;
    GLint accumTableSize = 0;
    __VIV2DPixelFormat * pcurrentPixelFormat = pPixelFormatList;

    gcoHAL_QueryTargetCaps(
            gcvNULL, gcvNULL,
            gcvNULL, gcvNULL, &maxSamples);

    if (bAccumSupport) {
        accumTableSize = sizeof(configAccum) / sizeof(configAccum[0]);
    }
    for (sampleIndex = 0; sampleIndex < (sizeof(configMultipleSample) / sizeof(configMultipleSample[0])); sampleIndex++)
    {
        if (maxSamples < configMultipleSample[sampleIndex].sampleQuality) {
            continue;
        }
        for (colorAlphaIndex = START_INDEX_OF_NONEDISPLAYABLE_COLOR; colorAlphaIndex < (sizeof(configColorAlpha) / sizeof(configColorAlpha[0])); colorAlphaIndex++)
        {
            for (depthIndex = 0; depthIndex < (sizeof(configDepth) / sizeof(configDepth[0])); depthIndex++)
            {
                for (accumIndex = 0; accumIndex < accumTableSize; accumIndex++)
                {
                    for (flagIndex = 2; flagIndex < (sizeof(configFlags) / sizeof(configFlags[0])); flagIndex++)
                    {
                        totalTableEntriese++;
                    }
                }
            }
        }
    }

    if (pPixelFormatList) {
        for (sampleIndex = 0; sampleIndex < (sizeof(configMultipleSample) / sizeof(configMultipleSample[0])); sampleIndex++)
        {
            if (maxSamples < configMultipleSample[sampleIndex].sampleQuality) {
                continue;
            }

            for (colorAlphaIndex = 0; colorAlphaIndex < (sizeof(configColorAlpha) / sizeof(configColorAlpha[0])); colorAlphaIndex++)
            {
                for (depthIndex = 0; depthIndex < (sizeof(configDepth) / sizeof(configDepth[0])); depthIndex++)
                {
                    for (accumIndex = 0; accumIndex < accumTableSize; accumIndex++)
                    {
                        for (flagIndex = 2; flagIndex < (sizeof(configFlags) / sizeof(configFlags[0])); flagIndex++)
                        {
                            fillPixelFormat(pcurrentPixelFormat, colorAlphaIndex, depthIndex, flagIndex, 1, sampleIndex, accumIndex);
                        }
                    }
                }
            }
        }
        return totalTableEntriese;
    } else {
        return totalTableEntriese * sizeof(__VIV2DPixelFormat);
    }
}

GLboolean __glDpGetPixelFormats(GLint numPixelFormat, GLvoid* pixelFormats, GLboolean bFloat)
{
    if (numPixelFormat)
    {
        GL_ASSERT(pixelFormats);
        GL_ASSERT(dpGlobalInfo.__glPFDTable);
        if (bFloat) {
            __GL_MEMCOPY(pixelFormats, (__VIV2DPixelFormat*)dpGlobalInfo.__glPFDTable + dpGlobalInfo.dPFDSize, numPixelFormat);
        }
        else {
            __GL_MEMCOPY(pixelFormats, dpGlobalInfo.__glPFDTable, numPixelFormat*sizeof(__VIV2DPixelFormat));
        }

        return GL_TRUE;
    } else {
        return GL_FALSE;
    }
}
/* Done */
GLboolean __glDpInitPixelFormats(GLvoid)
{
    __VIV2DPixelFormat *VIVpf;
    GLint i;

    if (dpGlobalInfo.__glPFDTable) {
        (*imports.free)(NULL, dpGlobalInfo.__glPFDTable);
    }

    dpGlobalInfo.__glPFDTable = NULL;

    dpGlobalInfo.dPFDSize = constructPixelFormatTable(NULL,dpGlobalInfo.bpp,
                              GL_TRUE);
    dpGlobalInfo.dPFDSizeNonDisplay= constructNoneDislayablePixelFormatTable(NULL,dpGlobalInfo.bpp,
                              GL_TRUE);
    dpGlobalInfo.__glPFDTable = (*imports.malloc)(NULL, dpGlobalInfo.dPFDSize + dpGlobalInfo.dPFDSizeNonDisplay );

    if (!dpGlobalInfo.__glPFDTable) {
        return GL_FALSE;
    }

    dpGlobalInfo.dPFDSize = constructPixelFormatTable((__VIV2DPixelFormat *)(dpGlobalInfo.__glPFDTable),
        dpGlobalInfo.bpp,
        GL_TRUE);
    dpGlobalInfo.dPFDSizeNonDisplay = constructNoneDislayablePixelFormatTable((__VIV2DPixelFormat *)dpGlobalInfo.__glPFDTable + dpGlobalInfo.dPFDSize,
        dpGlobalInfo.bpp,
        GL_TRUE);

    for(i = 0; i < dpGlobalInfo.dPFDSize + dpGlobalInfo.dPFDSizeNonDisplay; i++)
    {
        VIVpf = (__VIV2DPixelFormat*)dpGlobalInfo.__glPFDTable + i;
        if (i < dpGlobalInfo.dPFDSize) {
            VIVpf->index = i + 1;
        } else {
            VIVpf->index = -1;
        }
    }

    return GL_TRUE;
}

/* Done */
GLint __glDpDescribePixelFormat(GLint iPixelFormat,
    GLuint nBytes, GLvoid* ppfd, GLboolean bDisplayableOnly)
{
    __VIV2DPixelFormat * pCurrent;
    GLint i;

    if (!dpGlobalInfo.bpp) {
        (*imports.getDisplayMode)((GLuint*)(&dpGlobalInfo.width),
                                  (GLuint*)(&dpGlobalInfo.height),
                                  (GLuint*)(&dpGlobalInfo.bpp));
    }

    if (!dpGlobalInfo.__glPFDTable) {
        if (!__glDpInitPixelFormats()) {
            return 0;
        }
    }

    if (ppfd) {
        __VIV2DPixelFormat pixelformat;
        GL_ASSERT(iPixelFormat); /* should start from 1*/
        pCurrent = (__VIV2DPixelFormat *)dpGlobalInfo.__glPFDTable;

        if (iPixelFormat - 1 < dpGlobalInfo.dPFDSize) {
            pCurrent += iPixelFormat-1;
        } else {
            for(i = 0; i < dpGlobalInfo.dPFDSizeNonDisplay; i++) {
                pCurrent = (__VIV2DPixelFormat *)dpGlobalInfo.__glPFDTable + dpGlobalInfo.dPFDSize + i;
                if (pCurrent->index == iPixelFormat) {
                    break;
                }
            }
        }
        pixelformat = *pCurrent;

        if (bDisplayableOnly &&
          (pixelformat.extFlags!= 0 || pixelformat.sampleQuality!= 0)) {
            pixelformat.pixelformatDesc.dwFlags &= ~ PFD_SUPPORT_OPENGL;
        }

        __GL_MEMCOPY(ppfd, &pixelformat,(nBytes < sizeof( __VIV2DPixelFormat )) ?
                        nBytes:sizeof( __VIV2DPixelFormat )) ;

    }

    if (bDisplayableOnly) {
        return dpGlobalInfo.dPFDSize;
    } else {
        return dpGlobalInfo.dPFDSize + dpGlobalInfo.dPFDSizeNonDisplay;
    }
}

GLboolean __glDpSetPixelFormat(GLint iPixelFormat )
{
    return GL_TRUE;
}

/* Callback functions for WGL */

/* done */
#endif

#if defined(_LINUX_) && defined(GL4_DRI_BUILD)
#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<stdio.h>
GLvoid __gldevInitialize(void *pdata)
{
    Display *dpy = NULL;
    int scrn = 0;
    unsigned int bpp = 0;
    __GLscreenPrivate *screenPriv = (__GLscreenPrivate *)pdata;

    GL_ASSERT( pdata != gcvNULL);

    dpGlobalInfo.width = screenPriv->width;
    dpGlobalInfo.height = screenPriv->height;

    dpy = XOpenDisplay(NULL);
    if ( dpy == NULL )
    {
        fprintf(stderr,"Can't open Display in %s\n",__FUNCTION__);
        dpGlobalInfo.bpp = (screenPriv->stride / screenPriv->width);
    }
    else
    {
        scrn = DefaultScreen(dpy);
        bpp = DefaultDepth(dpy, scrn);
        if (bpp == 24)
            bpp = 32;
        dpGlobalInfo.bpp = bpp/8;
    }
    dpGlobalInfo.stride = screenPriv->stride;
    dpGlobalInfo.basePhyAddress = screenPriv->baseFBPhysicalAddress;
    dpGlobalInfo.logicalAddress = screenPriv->baseFBLinearAddress;
    dpGlobalInfo.gpuAddress = gcvINVALID_ADDRESS;
}

GLvoid __gldevDeinitialize(void *pdata)
{
}
#else
GLvoid __gldevInitialize()
{

}

GLboolean
__gldevDeinitialize()
{
    return GL_TRUE;
}
#endif

GLboolean
__glDpInitialize(
    __GLdeviceStruct *deviceEntry
    )
{
    gcmHEADER_ARG("deviceEntry=0x%x", deviceEntry);

    deviceEntry->devCreateContext   = __glChipCreateContext;
    deviceEntry->devGetConstants    = __glChipGetDeviceConstants;
    deviceEntry->devUpdateDrawable  = __glChipUpdateDrawable;
    deviceEntry->devDestroyDrawable = __glChipDestroyDrawable;
    deviceEntry->devDeinitialize    = __gldevDeinitialize;
    deviceEntry->devInitialize      = __gldevInitialize;
#if defined(_LINUX_) && defined(GL4_DRI_BUILD)
    deviceEntry->devCreateDrawable = __glChipCreateDrawable;
#elif defined(_WINDOWS)
    deviceEntry->devDescribePixelFormat = __glDpDescribePixelFormat;
    deviceEntry->devSetPixelFormat = __glDpSetPixelFormat;
#endif

    gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D);

    __GL_MEMZERO(&dpGlobalInfo, sizeof(__GLchipGlobal));
    dpGlobalInfo.processID = (GLuint)(gctUINTPTR_T)gcoOS_GetCurrentProcessID();

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;
}

