/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


/* $XFree86: xc/programs/Xserver/GL/mesa/src/X/xf86glx.c,v 1.19 2003/07/16 01:38:27 dawes Exp $ */
/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Kevin E. Martin <kevin@precisioninsight.com>
 *   Brian E. Paul <brian@precisioninsight.com>
 *
 */


#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "regionstr.h"
#include "resource.h"
#include "gl.h"
#include "GL/glxint.h"
#include "GL/glxtokens.h"
#include "scrnintstr.h"
#include "glxserver.h"
#include "glxscreens.h"
#include "glxdrawable.h"
#include "glxcontext.h"
#include "GL/glxext.h"
#include "glxutil.h"
#include "glcontextmodes.h"
#include "context.h"
#include "GL/xf86glx.h"
/*
 * This define is for the glcore.h header file.
 * If you add it here, then make sure you also add it in
 * ../../../glx/Imakefile.
 */
#include <glcore.h>

Bool __GLX_screenProbe(int screen);
__GLcontextInterface *__GLX_createContext(VEGLEXimports *imports,
                    __GLcontextModes *modes,
                    __GLcontextInterface *shareGC);
GLvoid __GLX_createBuffer(__GLXdrawablePrivate *glxPriv);

/*
 * This structure is statically allocated in the __glXScreens[]
 * structure.  This struct is not used anywhere other than in
 * __glXScreenInit to initialize each of the active screens
 * (__glXActiveScreens[]).  Several of the fields must be initialized by
 * the screenProbe routine before they are copied to the active screens
 * struct.  In particular, the contextCreate, pGlxVisual, numVisuals,
 * and numUsableVisuals fields must be initialized.
 */
static __GLXscreenInfo __glDDXScreenInfo = {
    __GLX_screenProbe,   /* Must be generic and handle all screens */
    NULL,                 /* Set up modes in probe */
    NULL,                 /* Set up pVisualPriv in probe */
    0,                    /* Set up numVisuals in probe */
    0,                    /* Set up numUsableVisuals in probe */
    NULL,                 /* GLextensions is overwritten by __glXScreenInit */
    "Vendor String",      /* GLXvendor is overwritten by __glXScreenInit */
    "Version String",     /* GLXversion is overwritten by __glXScreenInit */
    "Extensions String",  /* GLXextensions is overwritten by __glXScreenInit */
    NULL                  /* WrappedPositionWindow is overwritten */
};


Bool __GLX_initVisuals(VisualPtr *visualp, DepthPtr *depthp,
            int *nvisualp, int *ndepthp, int *rootDepthp,
            VisualID *defaultVisp, unsigned long sizes,
            int bitsPerRGB);
GLvoid __GLX_resetExtension(GLvoid);
GLvoid __GLX_setVisualConfigs(int nconfigs, __GLXvisualConfig *configs,
                 GLvoid **privates);

GLvoid *__glXglDDXScreenInfo(GLvoid) {
    return &__glDDXScreenInfo;
}

static __GLXextensionInfo __glDDXExtensionInfo = {
    __GLX_resetExtension,
    __GLX_initVisuals,
    __GLX_setVisualConfigs
};

GLvoid *__glXglDDXExtensionInfo(GLvoid) {
    return &__glDDXExtensionInfo;
}

typedef struct __GLX_screenRec __GLX_screen;
struct __GLX_screenRec {
    int num_vis;
    __GLcontextModes *modes;
    GLvoid **private;
};

static __GLX_screen  GLXScreens[MAXSCREENS];

static int                 numConfigs     = 0;
static __GLXvisualConfig  *visualConfigs  = NULL;
static GLvoid              **visualPrivates = NULL;


static int count_bits(unsigned int n)
{
   int bits = 0;

   while (n > 0) {
      if (n & 1) bits++;
      n >>= 1;
   }
   return bits;
}



/*
 * In the case the driver defines no GLX visuals we'll use these.
 * Note that for TrueColor and DirectColor visuals, bufferSize is the
 * sum of redSize, greenSize, blueSize and alphaSize, which may be larger
 * than the nplanes/rootDepth of the server's X11 visuals
 */
#define NUM_FALLBACK_CONFIGS 5
static __GLXvisualConfig FallbackConfigs[NUM_FALLBACK_CONFIGS] = {
  /* [0] = RGB, double buffered, Z */
  {
    -1,                 /* vid */
    -1,                 /* class */
    True,               /* rgba */
    -1, -1, -1, 0,      /* rgba sizes */
    -1, -1, -1, 0,      /* rgba masks */
     0,  0,  0, 0,      /* rgba accum sizes */
    True,               /* doubleBuffer */
    False,              /* stereo */
    -1,                 /* bufferSize */
    16,                 /* depthSize */
    0,                  /* stencilSize */
    0,                  /* auxBuffers */
    0,                  /* level */
    GLX_NONE,           /* visualRating */
    GLX_NONE,           /* transparentPixel */
    0, 0, 0, 0,         /* transparent rgba color (floats scaled to ints) */
    0                   /* transparentIndex */
  },
  /* [1] = RGB, double buffered, Z, stencil, accum */
  {
    -1,                 /* vid */
    -1,                 /* class */
    True,               /* rgba */
    -1, -1, -1, 0,      /* rgba sizes */
    -1, -1, -1, 0,      /* rgba masks */
    16, 16, 16, 0,      /* rgba accum sizes */
    True,               /* doubleBuffer */
    False,              /* stereo */
    -1,                 /* bufferSize */
    16,                 /* depthSize */
    8,                  /* stencilSize */
    0,                  /* auxBuffers */
    0,                  /* level */
    GLX_NONE,           /* visualRating */
    GLX_NONE,           /* transparentPixel */
    0, 0, 0, 0,         /* transparent rgba color (floats scaled to ints) */
    0                   /* transparentIndex */
  },
  /* [2] = RGB+Alpha, double buffered, Z, stencil, accum */
  {
    -1,                 /* vid */
    -1,                 /* class */
    True,               /* rgba */
    -1, -1, -1, 8,      /* rgba sizes */
    -1, -1, -1, -1,     /* rgba masks */
    16, 16, 16, 16,     /* rgba accum sizes */
    True,               /* doubleBuffer */
    False,              /* stereo */
    -1,                 /* bufferSize */
    16,                 /* depthSize */
    8,                  /* stencilSize */
    0,                  /* auxBuffers */
    0,                  /* level */
    GLX_NONE,           /* visualRating */
    GLX_NONE,           /* transparentPixel */
    0, 0, 0, 0,         /* transparent rgba color (floats scaled to ints) */
    0                   /* transparentIndex */
  },
  /* [3] = RGB+Alpha, single buffered, Z, stencil, accum */
  {
    -1,                 /* vid */
    -1,                 /* class */
    True,               /* rgba */
    -1, -1, -1, 8,      /* rgba sizes */
    -1, -1, -1, -1,     /* rgba masks */
    16, 16, 16, 16,     /* rgba accum sizes */
    False,              /* doubleBuffer */
    False,              /* stereo */
    -1,                 /* bufferSize */
    16,                 /* depthSize */
    8,                  /* stencilSize */
    0,                  /* auxBuffers */
    0,                  /* level */
    GLX_NONE,           /* visualRating */
    GLX_NONE,           /* transparentPixel */
    0, 0, 0, 0,         /* transparent rgba color (floats scaled to ints) */
    0                   /* transparentIndex */
  },
  /* [4] = CI, double buffered, Z */
  {
    -1,                 /* vid */
    -1,                 /* class */
    False,              /* rgba? (false = color index) */
    -1, -1, -1, 0,      /* rgba sizes */
    -1, -1, -1, 0,      /* rgba masks */
     0,  0,  0, 0,      /* rgba accum sizes */
    True,               /* doubleBuffer */
    False,              /* stereo */
    -1,                 /* bufferSize */
    16,                 /* depthSize */
    0,                  /* stencilSize */
    0,                  /* auxBuffers */
    0,                  /* level */
    GLX_NONE,           /* visualRating */
    GLX_NONE,           /* transparentPixel */
    0, 0, 0, 0,         /* transparent rgba color (floats scaled to ints) */
    0                   /* transparentIndex */
  },
};


static Bool init_visuals(int *nvisualp, VisualPtr *visualp,
             VisualID *defaultVisp,
             int ndepth, DepthPtr pdepth,
             int rootDepth)
{
    int numRGBconfigs;
    int numCIconfigs;
    int numVisuals = *nvisualp;
    int numNewVisuals;
    int numNewConfigs;
    VisualPtr pVisual = *visualp;
    VisualPtr pVisualNew = NULL;
    VisualID *orig_vid = NULL;
    __GLcontextModes *modes;
    __GLXvisualConfig *pNewVisualConfigs = NULL;
    GLvoid **glXVisualPriv;
    GLvoid **pNewVisualPriv;
    int found_default;
    int i, j, k;

    if (numConfigs > 0)
        numNewConfigs = numConfigs;
    else
        numNewConfigs = NUM_FALLBACK_CONFIGS;

    /* Alloc space for the list of new GLX visuals */
    pNewVisualConfigs = (__GLXvisualConfig *)
                     __glXMalloc(numNewConfigs * sizeof(__GLXvisualConfig));
    if (!pNewVisualConfigs) {
    return FALSE;
    }

    /* Alloc space for the list of new GLX visual privates */
    pNewVisualPriv = (GLvoid **) __glXMalloc(numNewConfigs * sizeof(GLvoid *));
    if (!pNewVisualPriv) {
    __glXFree(pNewVisualConfigs);
    return FALSE;
    }

    /*
    ** If SetVisualConfigs was not called, then use default GLX
    ** visual configs.
    */
    if (numConfigs == 0) {
    memcpy(pNewVisualConfigs, FallbackConfigs,
               NUM_FALLBACK_CONFIGS * sizeof(__GLXvisualConfig));
    memset(pNewVisualPriv, 0, NUM_FALLBACK_CONFIGS * sizeof(GLvoid *));
    }
    else {
        /* copy driver's visual config info */
        for (i = 0; i < numConfigs; i++) {
            pNewVisualConfigs[i] = visualConfigs[i];
            pNewVisualPriv[i] = visualPrivates[i];
        }
    }

    /* Count the number of RGB and CI visual configs */
    numRGBconfigs = 0;
    numCIconfigs = 0;
    for (i = 0; i < numNewConfigs; i++) {
    if (pNewVisualConfigs[i].rgba)
        numRGBconfigs++;
    else
        numCIconfigs++;
    }

    /* Count the total number of visuals to compute */
    numNewVisuals = 0;
    for (i = 0; i < numVisuals; i++) {
        numNewVisuals +=
        (pVisual[i].class == TrueColor || pVisual[i].class == DirectColor)
        ? numRGBconfigs : numCIconfigs;
    }

    /* Reset variables for use with the next screen/driver's visual configs */
    visualConfigs = NULL;
    numConfigs = 0;

    /* Alloc temp space for the list of orig VisualIDs for each new visual */
    orig_vid = (VisualID *)__glXMalloc(numNewVisuals * sizeof(VisualID));
    if (!orig_vid) {
    __glXFree(pNewVisualPriv);
    __glXFree(pNewVisualConfigs);
    return FALSE;
    }

    /* Alloc space for the list of glXVisuals */
    modes = _gl_context_modes_create(numNewVisuals, sizeof(__GLcontextModes));
    if (modes == NULL) {
    __glXFree(orig_vid);
    __glXFree(pNewVisualPriv);
    __glXFree(pNewVisualConfigs);
    return FALSE;
    }

    /* Alloc space for the list of glXVisualPrivates */
    glXVisualPriv = (GLvoid **)__glXMalloc(numNewVisuals * sizeof(GLvoid *));
    if (!glXVisualPriv) {
    _gl_context_modes_destroy( modes );
    __glXFree(orig_vid);
    __glXFree(pNewVisualPriv);
    __glXFree(pNewVisualConfigs);
    return FALSE;
    }

    /* Alloc space for the new list of the X server's visuals */
    pVisualNew = (VisualPtr)__glXMalloc(numNewVisuals * sizeof(VisualRec));
    if (!pVisualNew) {
    __glXFree(glXVisualPriv);
    _gl_context_modes_destroy( modes );
    __glXFree(orig_vid);
    __glXFree(pNewVisualPriv);
    __glXFree(pNewVisualConfigs);
    return FALSE;
    }

    /* Initialize the new visuals */
    found_default = FALSE;
    GLXScreens[screenInfo.numScreens-1].modes = modes;
    for (i = j = 0; i < numVisuals; i++) {
        int is_rgb = (pVisual[i].class == TrueColor ||
              pVisual[i].class == DirectColor);

    for (k = 0; k < numNewConfigs; k++) {
        if (pNewVisualConfigs[k].rgba != is_rgb)
        continue;

        assert( modes != NULL );

        /* Initialize the new visual */
        pVisualNew[j] = pVisual[i];
        pVisualNew[j].vid = FakeClientID(0);

        /* Check for the default visual */
        if (!found_default && pVisual[i].vid == *defaultVisp) {
        *defaultVisp = pVisualNew[j].vid;
        found_default = TRUE;
        }

        /* Save the old VisualID */
        orig_vid[j] = pVisual[i].vid;

        /* Initialize the glXVisual */
        _gl_copy_visual_to_context_mode( modes, & pNewVisualConfigs[k] );
        modes->visualID = pVisualNew[j].vid;

        /*
         * If the class is -1, then assume the X visual information
         * is identical to what GLX needs, and take them from the X
         * visual.  NOTE: if class != -1, then all other fields MUST
         * be initialized.
         */
        if (modes->visualType == GLX_NONE) {
        modes->visualType = _gl_convert_from_x_visual_type( pVisual[i].class );
        modes->redBits    = count_bits(pVisual[i].redMask);
        modes->greenBits  = count_bits(pVisual[i].greenMask);
        modes->blueBits   = count_bits(pVisual[i].blueMask);
        modes->alphaBits  = modes->alphaBits;
        modes->redMask    = pVisual[i].redMask;
        modes->greenMask  = pVisual[i].greenMask;
        modes->blueMask   = pVisual[i].blueMask;
        modes->alphaMask  = modes->alphaMask;
        modes->rgbaBits = (is_rgb)
            ? (modes->redBits + modes->greenBits +
               modes->blueBits + modes->alphaBits)
            : rootDepth;
        }

        /* Save the device-dependent private for this visual */
        glXVisualPriv[j] = pNewVisualPriv[k];

        j++;
        modes = modes->next;
    }
    }

    assert(j <= numNewVisuals);

    /* Save the GLX visuals in the screen structure */
    GLXScreens[screenInfo.numScreens-1].num_vis = numNewVisuals;
    GLXScreens[screenInfo.numScreens-1].private = glXVisualPriv;

    /* Set up depth's VisualIDs */
    for (i = 0; i < ndepth; i++) {
    int numVids = 0;
    VisualID *pVids = NULL;
    int k, n = 0;

    /* Count the new number of VisualIDs at this depth */
    for (j = 0; j < pdepth[i].numVids; j++)
        for (k = 0; k < numNewVisuals; k++)
        if (pdepth[i].vids[j] == orig_vid[k])
            numVids++;

    /* Allocate a new list of VisualIDs for this depth */
    pVids = (VisualID *)__glXMalloc(numVids * sizeof(VisualID));

    /* Initialize the new list of VisualIDs for this depth */
    for (j = 0; j < pdepth[i].numVids; j++)
        for (k = 0; k < numNewVisuals; k++)
        if (pdepth[i].vids[j] == orig_vid[k])
            pVids[n++] = pVisualNew[k].vid;

    /* Update this depth's list of VisualIDs */
    __glXFree(pdepth[i].vids);
    pdepth[i].vids = pVids;
    pdepth[i].numVids = numVids;
    }

    /* Update the X server's visuals */
    *nvisualp = numNewVisuals;
    *visualp = pVisualNew;

    /* Free the old list of the X server's visuals */
    __glXFree(pVisual);

    /* Clean up temporary allocations */
    __glXFree(orig_vid);
    __glXFree(pNewVisualPriv);
    __glXFree(pNewVisualConfigs);

    /* Free the private list created by DDX HW driver */
    if (visualPrivates)
        xfree(visualPrivates);
    visualPrivates = NULL;
    return TRUE;
}

GLvoid __GLX_setVisualConfigs(int nconfigs, __GLXvisualConfig *configs,
                 GLvoid **privates)
{
    numConfigs = nconfigs;
    visualConfigs = configs;
    visualPrivates = privates;
}

Bool __GLX_initVisuals(VisualPtr *visualp, DepthPtr *depthp,
            int *nvisualp, int *ndepthp, int *rootDepthp,
            VisualID *defaultVisp, unsigned long sizes,
            int bitsPerRGB)
{
    /*
     * Setup the visuals supported by this particular screen.
     */
    return init_visuals(nvisualp, visualp, defaultVisp,
            *ndepthp, *depthp, *rootDepthp);
}

static GLvoid fixup_visuals(int screen)
{
    ScreenPtr pScreen = screenInfo.screens[screen];
    __GLX_screen *pMScr = &GLXScreens[screen];
    int j;
    __GLcontextModes *modes;

    for ( modes = pMScr->modes ; modes != NULL ; modes = modes->next ) {
    const int vis_class = _gl_convert_to_x_visual_type( modes->visualType );
    const int nplanes = (modes->rgbaBits - modes->alphaBits);
    const VisualPtr pVis = pScreen->visuals;

    /* Find a visual that matches the GLX visual's class and size */
    for (j = 0; j < pScreen->numVisuals; j++) {
        if (pVis[j].class == vis_class &&
        pVis[j].nplanes == nplanes) {

        /* Fixup the masks */
        modes->redMask   = pVis[j].redMask;
        modes->greenMask = pVis[j].greenMask;
        modes->blueMask  = pVis[j].blueMask;

        /* Recalc the sizes */
        modes->redBits   = count_bits(modes->redMask);
        modes->greenBits = count_bits(modes->greenMask);
        modes->blueBits  = count_bits(modes->blueMask);
        }
    }
    }
}

static GLvoid init_screen_visuals(int screen)
{
    ScreenPtr pScreen = screenInfo.screens[screen];
    __GLcontextModes *modes;
    int *used;
    int i, j;

    /* FIXME: Change 'used' to be a array of bits (rather than of ints),
     * FIXME: create a stack array of 8 or 16 bytes.  If 'numVisuals' is less
     * FIXME: than 64 or 128 the stack array can be used instead of calling
     * FIXME: __glXMalloc / __glXFree.  If nothing else, convert 'used' to
     * FIXME: array of bytes instead of ints!
     */
    used = (int *)__glXMalloc(pScreen->numVisuals * sizeof(int));
    __glXMemset(used, 0, pScreen->numVisuals * sizeof(int));

    i = 0;
    for ( modes = GLXScreens[screen].modes
      ; modes != NULL
      ; modes = modes->next ) {
    const int vis_class = _gl_convert_to_x_visual_type( modes->visualType );
    const int nplanes = (modes->rgbaBits - modes->alphaBits);
    const VisualPtr pVis = pScreen->visuals;

    for (j = 0; j < pScreen->numVisuals; j++) {
        if (pVis[j].class     == vis_class &&
        pVis[j].nplanes   == nplanes &&
        pVis[j].redMask   == modes->redMask &&
        pVis[j].greenMask == modes->greenMask &&
        pVis[j].blueMask  == modes->blueMask &&
        !used[j]) {

        /* Set the VisualID */
        modes->visualID = pVis[j].vid;

        /* Mark this visual used */
        used[j] = 1;
        break;
        }
    }

    if ( j == pScreen->numVisuals ) {
        ErrorF("No matching visual for __GLcontextMode with "
           "visual class = %d (%d), nplanes = %u\n",
           vis_class,
           modes->visualType,
           (modes->rgbaBits - modes->alphaBits) );
    }
    else if ( modes->visualID == -1 ) {
        printf( "Matching visual found, but visualID still -1!\n" );
    }

    i++;
    }

    __glXFree(used);
}

Bool __GLX_screenProbe(int screen)
{
    /*
     * Set up the current screen's visuals.
     */
    __glDDXScreenInfo.modes = GLXScreens[screen].modes;
    __glDDXScreenInfo.pVisualPriv = GLXScreens[screen].private;
    __glDDXScreenInfo.numVisuals =
    __glDDXScreenInfo.numUsableVisuals = GLXScreens[screen].num_vis;


    /*
     * The ordering of the rgb compenents might have been changed by the
     * driver after mi initialized them.
     */
    fixup_visuals(screen);

    /*
     * Find the GLX visuals that are supported by this screen and create
     * XMesa's visuals.
     */
    init_screen_visuals(screen);

    return TRUE;
}

GLvoid __GLX_resetExtension(GLvoid)
{
    int i, j;

    for (i = 0; i < screenInfo.numScreens; i++) {
    _gl_context_modes_destroy( GLXScreens[i].modes );
    GLXScreens[i].modes = NULL;
    __glXFree(GLXScreens[i].private);
    GLXScreens[i].private = NULL;
    GLXScreens[i].num_vis = 0;
    }
    __glDDXScreenInfo.modes = NULL;
}
