/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "glcore/gc_gl_context.h"

/*******************************************************************************************/
#if defined(_WIN32)
#pragma warning(disable: 4244)
#endif


/*************************************************************************************/


/*  Q: how to handle where the pxFormat doesn't fit with HW format????
 *
 *  A: here we only put native formats, and the patched format would be
 *     generated from dp-choose procedure, and set to mipmap structure with
 *     a unified function.
 ****************************************************************************************/

 /*************************************************************************
  *
  *         Native formats
  *
  ************************************************************************/
/*null texture format, used when proxy texture failed*/
const __GLdeviceFormatInfo __glNullDevfmt = {
        0,         /* __GL_TEXTURE_FORMAT */
        GL_FALSE,                        /*compressed*/

        0,                                /* redMask*/
        0,                                /* greenMask*/
        0,                                /* blueMask*/
        0,                                /* alphaMask*/
        0,                                /* depthMask*/
        0,                                /* stencilMask*/
        0,                                /* luminanceMask*/
        0,                                /* intensityMask*/

        0,                                  /* indexSize */
        0,                                  /* redSize */
        0,                                  /* greenSize */
        0,                                   /* blueSize */
        0,                                  /* alphaSize */
        0,                                  /* depthSize */
        0,                                  /* stencilSize*/
        0,                                  /* luminanceSize */
        0,                                  /* intensitySize */
        0,                                  /* sharedSize    */

        GL_NONE,                            /* redType*/
        GL_NONE,                            /* greenType*/
        GL_NONE,                            /* blueType*/
        GL_NONE,                            /* alphaType*/
        GL_NONE,                            /* depthType*/
        GL_NONE,                            /* stencilType*/
        GL_NONE,                            /* luminanceType*/
        GL_NONE,                            /* intensityType*/

        0,                                  /* bitsPerPixel */
        0,                  /*pxFormat*/
        0,                   /*pxType */
        0,                                  /*pxAlignment */
};

/*please do not change the order. The order must match the enum __GL_DEVTURE_FORMAT*/
const __GLdeviceFormatInfo __glDevfmtInfo[__GL_DEVFMT_MAX] = {

    /*__GL_DEVFMT_ALPHA8*/
    {
        __GL_DEVFMT_ALPHA8,                                     /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                                               /*compressed*/

        0x0,                                                    /*redMask*/
        0x0,                                                    /*greenMask*/
        0x0,                                                    /*blueMask*/
        0xFF,                                                   /*alphaMask*/
        0x0,                                                    /*depthMask*/
        0x0,                                                    /*stencilMask*/
        0x0,                                                    /*luminanceMask*/
        0x0,                                                    /*intensityMask*/

        0,                                                      /* indexSize */
        0,                                                      /* redSize */
        0,                                                      /* greenSize */
        0,                                                      /* blueSize */
        8,                                                      /* alphaSize */
        0,                                                      /* depthSize*/
        0,                                                      /* stencilSize */
        0,                                                      /* luminanceSize */
        0,                                                      /* intensitySize */
        0,                                                      /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_UNSIGNED_NORMALIZED_ARB,                             /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        8,                                                      /* bitsPerPixel */
        GL_ALPHA,                                               /* pxFormat */
        GL_UNSIGNED_BYTE,                                       /*pxType */
        1,                                                      /*pxAlignment */
    },
    /*__GL_DEVFMT_ALPHA16*/
    {
        __GL_DEVFMT_ALPHA16,                                     /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                                                /* compressed */

        0x0,                                                     /* redMask*/
        0x0,                                                     /* greenMask*/
        0x0,                                                     /* blueMask*/
        0xFFFF,                                                  /* alphaMask*/
        0x0,                                                     /* depthMask*/
        0x0,                                                     /* stencilMask*/
        0x0,                                                     /* luminanceMask*/
        0x0,                                                     /* intensityMask*/

        0,                                                       /* indexSize */
        0,                                                       /* redSize */
        0,                                                       /* greenSize */
        0,                                                       /* blueSize */
        16,                                                      /* alphaSize */
        0,                                                       /* depthSize */
        0,                                                       /* stencilSize*/
        0,                                                       /* luminanceSize */
        0,                                                       /* intensitySize */
        0,                                                       /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_UNSIGNED_NORMALIZED_ARB,                             /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        16,                                                      /* bitsPerPixel */
        GL_ALPHA,                                                /* pxFormat */
        GL_UNSIGNED_SHORT,                                       /* pxType */
        2,                                                       /* pxAlignment */
    },
    /*__GL_DEVFMT_ALPHA24*/
    {
        __GL_DEVFMT_ALPHA24,                                      /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                                                 /* compressed */

        0x0,                                                      /* redMask*/
        0x0,                                                      /* greenMask*/
        0x0,                                                      /* blueMask*/
        0xFFFFFF,                                                 /* alphaMask*/
        0x0,                                                      /* depthMask*/
        0x0,                                                      /* stencilMask*/
        0x0,                                                      /* LuminanceMask*/
        0x0,                                                      /* intensityMask*/

        0,                                                        /* indexSize */
        0,                                                        /* redSize */
        0,                                                        /* greenSize */
        0,                                                        /* blueSize */
        32,                                                       /* alphaSize */
        0,                                                        /* depthSize */
        0,                                                        /* stencilSize*/
        0,                                                        /* luminanceSize */
        0,                                                        /* intensitySize */
        0,                                                        /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_UNSIGNED_NORMALIZED_ARB,                             /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        32,                                                       /* bitsPerPixel */
        GL_ALPHA,                                                 /* pxFormat */
        __GL_UNSIGNED_INT_24,                                     /* pxType */
        4,                                                        /* pxAlignment */
    },
    /*__GL_DEVFMT_ALPHA32F*/
    {
        __GL_DEVFMT_ALPHA32F,               /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                           /* compressed */

        0x0,                                /* redMask*/
        0x0,                                /* greenMask*/
        0x0,                                /* blueMask*/
        0xFFFFFFFF,                         /* alphaMask*/
        0x0,                                /* depthMask*/
        0x0,                                /* stencilMask*/
        0x0,                                /* luminanceMask*/
        0x0,                                /* intensityMask*/

        0,                                  /* indexSize */
        0,                                  /* redSize */
        0,                                  /* greenSize */
        0,                                  /* blueSize */
        32,                                 /* alphaSize */
        0,                                  /* depthSize*/
        0,                                  /* stencilSize*/
        0,                                  /* luminanceSize */
        0,                                  /* intensitySize */
        0,                                  /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_FLOAT,                             /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        32,                                 /* bitsPerPixel */
        GL_ALPHA,                           /* pxFormat */
        GL_FLOAT,                           /* pxType */
        4,                                  /* pxAlignment */
    },

    /*__GL_DEVFMT_LUMINANCE8*/
    {
        __GL_DEVFMT_LUMINANCE8,                     /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                                   /*compressed*/

        0x0,                                        /* redMask*/
        0x0,                                        /* greenMask*/
        0x0,                                        /* blueMask*/
        0x0,                                        /* alphaMask*/
        0x0,                                        /* depthMask*/
        0x0,                                        /* stencilMask*/
        0xFF,                                       /* luminanceMask*/
        0x0,                                        /* intensityMask*/

        0,                                          /* indexSize */
        0,                                          /* redSize */
        0,                                          /* greenSize */
        0,                                          /* blueSize */
        0,                                          /* alphaSize */
        0,                                          /* depthSize*/
        0,                                          /* stencilSize*/
        8,                                          /* luminanceSize */
        0,                                          /* intensitySize */
        0,                                          /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        8,                                          /* bitsPerPixel */
        GL_LUMINANCE,                               /*pxFormat*/
        GL_UNSIGNED_BYTE,                           /*pxType */
        1,                                          /*pxAlignment */
    },

    /*__GL_DEVFMT_LUMINANCE16*/
    {
        __GL_DEVFMT_LUMINANCE16,                    /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                                   /*compressed*/

        0x0,                                        /*redMask*/
        0x0,                                        /*greenMask*/
        0x0,                                        /*blueMask*/
        0x0,                                        /*alphaMask*/
        0x0,                                        /*depthMask*/
        0x0,                                        /*stenciMask*/
        0xFFFF,                                     /*luminanceMask*/
        0x0,                                        /*intensityMask*/

        0,                                          /* indexSize */
        0,                                          /* redSize */
        0,                                          /* greenSize */
        0,                                          /* blueSize */
        0,                                          /* alphaSize */
        0,                                          /* depthSize*/
        0,                                          /* stencilSize*/
        16,                                         /* luminanceSize */
        0,                                          /* intensitySize */
        0,                                          /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        16,                                          /* bitsPerPixel */
        GL_LUMINANCE,                               /*pxFormat*/
        GL_UNSIGNED_SHORT,                           /*pxType */
        2,                                          /*pxAlignment */
    },
    /*__GL_DEVFMT_LUMINANCE24*/
    {
        __GL_DEVFMT_LUMINANCE24,             /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                            /* compressed */

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0xFFFFFF,                            /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                   /* indexSize */
        0,                                   /* redSize */
        0,                                   /* greenSize */
        0,                                   /* blueSize */
        0,                                   /* alphaSize */
        0,                                   /* depthSize */
        0,                                   /* stencilSize*/
        32,                                  /* luminanceSize */
        0,                                   /* intensitySize */
        0,                                   /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_UNSIGNED_NORMALIZED_ARB,                             /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        32,                                  /* bitsPerPixel */
        GL_LUMINANCE,                        /* pxFormat*/
        __GL_UNSIGNED_INT_24,                /* pxType */
        4,                                   /* pxAlignment */
    },

    /*__GL_DEVFMT_LUMINANCE32F*/
    {
        __GL_DEVFMT_LUMINANCE32F,            /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                            /* compressed */

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0xFFFFFFFF,                          /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                   /* indexSize */
        0,                                   /* redSize */
        0,                                   /* greenSize */
        0,                                   /* blueSize */
        0,                                   /* alphaSize */
        0,                                   /* depthSize */
        0,                                   /* stencilSize*/
        32,                                  /* luminanceSize */
        0,                                   /* intensitySize */
        0,                                   /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_FLOAT,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        32,                                  /* bitsPerPixel */
        GL_LUMINANCE,                        /* pxFormat*/
        GL_FLOAT,                            /* pxType */
        4,                                   /* pxAlignment */
    },

        /*__GL_DEVFMT_INTENSITY8*/
    {
        __GL_DEVFMT_INTENSITY8,              /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                            /*compressed */

        0x0,                                 /*redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0xFF,                                /*intensityMask*/

        0,                                   /* indexSize */
        0,                                   /* redSize */
        0,                                   /* greenSize */
        0,                                   /* blueSize */
        0,                                   /* alphaSize */
        0,                                   /* depthSize*/
        0,                                   /* stencilSize*/
        0,                                   /* luminanceSize */
        8,                                   /* intensitySize */
        0,                                   /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* intensityType*/

        8,                                   /* bitsPerPixel */
        GL_INTENSITY,                        /* pxFormat*/
        GL_UNSIGNED_BYTE,                    /* pxType */
        1,                                   /* pxAlignment */
    },

    /*__GL_DEVFMT_INTENSITY16*/
    {
        __GL_DEVFMT_INTENSITY16,             /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                            /*compressed */

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0xFFFF,                              /* intensityMask*/

        0,                                   /* indexSize */
        0,                                   /* redSize */
        0,                                   /* greenSize */
        0,                                   /* blueSize */
        0,                                   /* alphaSize */
        0,                                   /* depthSize*/
        0,                                   /* stencilSize*/
        0,                                   /* luminanceSize */
        16,                                  /* intensitySize */
        0,                                   /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* intensityType*/

        16,                                  /* bitsPerPixel */
        GL_INTENSITY,                        /* pxFormat*/
        GL_UNSIGNED_SHORT,                   /* pxType */
        2,                                   /* pxAlignment */
    },

    /*__GL_DEVFMT_INTENSITY24*/
    {
        __GL_DEVFMT_INTENSITY24,             /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                            /* compressed */

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0xFFFFFF,                            /* intensityMask*/

        0,                                   /* indexSize */
        0,                                   /* redSize */
        0,                                   /* greenSize */
        0,                                   /* blueSize */
        0,                                   /* alphaSize */
        0,                                   /* dpethSize*/
        0,                                   /* stencilSize*/
        0,                                   /* luminanceSize */
        32,                                  /* intensitySize */
        0,                                   /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* intensityType*/

        32,                                  /* bitsPerPixel */
        GL_INTENSITY,                        /* pxFormat*/
        __GL_UNSIGNED_INT_24,                /* pxType */
        4,                                   /* pxAlignment */
    },

    /*__GL_DEVFMT_INTENSITY32F*/
    {
        __GL_DEVFMT_INTENSITY32F,            /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                            /* compressed */

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0xFFFFFFFF,                          /* intensityMask*/

        0,                                   /* indexSize */
        0,                                   /* redSize */
        0,                                   /* greenSize */
        0,                                   /* blueSize */
        0,                                   /* alphaSize */
        0,                                   /* depthSize */
        0,                                   /* stencilSize */
        0,                                   /* luminanceSize */
        32,                                  /* intensitySize */
        0,                                   /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_FLOAT,                                                /* intensityType*/

        32,                                  /* bitsPerPixel */
        GL_INTENSITY,                        /* pxFormat*/
        GL_FLOAT,                            /* pxType */
        4,                                   /* pxAlignment */
    },

    /* Patch __GL_DEVFMT_LUMINANCE_ALPHA4 format in PIXEL PIPE:
    ** corresponding hw format is A4L4.
    ** Because gl had not define a general pixel type as GL_UNSIGNED_BYTE_4_4_REV.
    ** If assign pxType as GL_UNSIGNED_BYTE, pxFormat as GL_LUMINANCE_ALPHA, texture will load to dest as
    ** L8_A8. system cache and device cache(oe device memory image) inconsistant.
    ** To desribe ALPHA4_LUMINANCE4 to pixel pipe, we add a private pixel type as GL_UNSIGNED_BYTE_4_4_REV_VIVPRIV
    ** Handle in PickSpanPack/PickSpanUnPack, pixel pipe know from both pxFormat and
    ** pxType that this dest format is  A4L4..
    */
    /*__GL_DEVFMT_LUMINANCE_ALPHA4: A4L4*/
    {
        __GL_DEVFMT_LUMINANCE_ALPHA4,       /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                           /*compressed*/

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0xF0,                                /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0xF,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                  /* indexSize */
        0,                                  /* redSize */
        0,                                  /* greenSize */
        0,                                   /* blueSize */
        4,                                  /* alphaSize */
        0,                                  /* depthSize*/
        0,                                  /* stencilSize*/
        4,                                  /* luminanceSize */
        0,                                  /* intensitySize */
        0,                                  /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        8,                                  /* bitsPerPixel */
        GL_LUMINANCE_ALPHA,                 /*pxFormat*/
        __GL_UNSIGNED_BYTE_4_4_REV_VIVPRIV,  /*pxType */
        1,                                  /*pxAlignment */
    },

    /*__GL_DEVFMT_LUMINANCE_ALPHA8: L8A8*/
    {
        __GL_DEVFMT_LUMINANCE_ALPHA8,               /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                                   /*compressed*/

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0xFF00,                              /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0xFF,                                /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                   /* indexSize */
        0,                                   /* redSize */
        0,                                   /* greenSize */
        0,                                   /* blueSize */
        8,                                   /* alphaSize */
        0,                                   /* depthSize*/
        0,                                   /* stencilSize*/
        8,                                   /* luminanceSize */
        0,                                   /* intensitySize */
        0,                                   /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        16,                                  /* bitsPerPixel */
        GL_LUMINANCE_ALPHA,                  /* pxFormat*/
        GL_UNSIGNED_BYTE,                    /* pxType */
        1,                                   /* pxAlignment */
    },

    /*__GL_DEVFMT_LUMINANCE_ALPHA16: L16A16*/
    {
        __GL_DEVFMT_LUMINANCE_ALPHA16,       /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                            /* compressed */

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0xFFFF0000,                          /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0xFFFF,                              /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                   /* indexSize */
        0,                                   /* redSize */
        0,                                   /* greenSize */
        0,                                   /* blueSize */
        16,                                  /* alphaSize */
        0,                                   /* depthSize*/
        0,                                   /* stencilSize*/
        16,                                  /* luminanceSize */
        0,                                   /* intensitySize */
        0,                                   /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        32,                                  /* bitsPerPixel */
        GL_LUMINANCE_ALPHA,                  /* pxFormat */
        GL_UNSIGNED_SHORT,                   /* pxType */
        2,                                   /* pxAlignment */
    },

    /*__GL_DEVFMT_BGR565: RGB565*/
    {
        __GL_DEVFMT_BGR565,            /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                           /*compressed*/

        0xF800,                             /* redMask*/
        0x07E0,                             /* greenMask*/
        0x001F,                             /* blueMask*/
        0x0,                                /* alphaMask*/
        0x0,                                /* depthMask*/
        0x0,                                /* stencilMask*/
        0x0,                                /* luminanceMask*/
        0x0,                                /* intensityMask*/

        0,                                  /* indexSize */
        5,                                  /* redSize */
        6,                                  /* greenSize */
        5,                                  /* blueSize */
        0,                                  /* alphaSize */
        0,                                  /* depthSize*/
        0,                                  /* stencilSize*/
        0,                                  /* luminanceSize */
        0,                                  /* intensitySize */
        0,                                  /* sharedSize    */

        GL_UNSIGNED_NORMALIZED_ARB,                                                /* redType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* greenType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        16,                                 /* bitsPerPixel */
        GL_BGR,                             /*pxFormat*/
        GL_UNSIGNED_SHORT_5_6_5_REV,        /*pxType */
        2,                                  /*pxAlignment */
    },

    /*__GL_DEVFMT_BGRA444: ARGB4444*/
    {
        __GL_DEVFMT_BGRA4444,            /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                        /*compressed*/

        0xF00,                           /* redMask*/
        0xF0,                            /* greenMask*/
        0xF,                             /* blueMask*/
        0xF000,                          /* alphaMask*/
        0x0,                             /* depthMask*/
        0x0,                             /* stencilMask*/
        0x0,                             /* luminanceMask*/
        0x0,                             /* intensityMask*/

        0,                               /* indexSize */
        4,                               /* redSize */
        4,                               /* greenSize */
        4,                               /* blueSize */
        4,                               /* alphaSize */
        0,                               /* depthSize*/
        0,                               /* stencilSize*/
        0,                               /* luminanceSize */
        0,                               /* intensitySize */
        0,                               /* sharedSize    */

        GL_UNSIGNED_NORMALIZED_ARB,                                                /* redType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* greenType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* blueType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        16,                              /* bitsPerPixel */
        GL_BGRA,                         /*pxFormat*/
        GL_UNSIGNED_SHORT_4_4_4_4_REV,   /*pxType */
        2,                               /*pxAlignment */
    },

    /*__GL_DEVFMT_BGRA5551: ARGB1555*/
    {
        __GL_DEVFMT_BGRA5551,            /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                        /*compressed*/

        0x7C00,                          /* redMask*/
        0x3E0,                           /* greenMask*/
        0x1F,                            /* blueMask*/
        0x8000,                          /* alphaMask*/
        0x0,                             /* depthMask*/
        0x0,                             /* stencilMask*/
        0x0,                             /* luminanceMask*/
        0x0,                             /* intensityMask*/

        0,                               /* indexSize */
        5,                               /* redSize */
        5,                               /* greenSize */
        5,                               /* blueSize */
        1,                               /* alphaSize */
        0,                               /* depthSize*/
        0,                               /* stencilSize*/
        0,                               /* luminanceSize */
        0,                               /* intensitySize */
        0,                               /* sharedSize    */

        GL_UNSIGNED_NORMALIZED_ARB,                                                /* redType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* greenType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* blueType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        16,                              /* bitsPerPixel */
        GL_BGRA,                         /*pxFormat*/
        GL_UNSIGNED_SHORT_1_5_5_5_REV,   /*pxType */
        2,                               /*pxAlignment */
    },

    /*__GL_DEVFMT_BGRA8888: ARGB8888*/
    {
        __GL_DEVFMT_BGRA8888,          /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                        /*compressed*/

        0xFF0000,                                 /* redMask*/
        0xFF00,                                 /* greenMask*/
        0xFF,                                 /* blueMask*/
        0xFF000000,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        8,                                     /* redSize */
        8,                                      /* greenSize */
        8,                                      /* blueSize */
        8,                                      /* alphaSize */
        0,                                      /* depthSize*/
        0,                                      /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_UNSIGNED_NORMALIZED_ARB,                                                /* redType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* greenType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* blueType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        32,                                     /* bitsPerPixel */
        GL_BGRA,                    /*pxFormat*/
        GL_UNSIGNED_BYTE,                       /*pxType */
        1,                                      /*pxAlignment */
    },

    /*__GL_DEVFMT_RGBA16*/
    {
        __GL_DEVFMT_RGBA16,         /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                        /*compressed*/

        0xFFFF,                                 /* redMask*/
        0xFFFF,                                 /* greenMask*/
        0xFFFF,                                 /* blueMask*/
        0xFFFF,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        16,                                     /* redSize */
        16,                                      /* greenSize */
        16,                                      /* blueSize */
        16,                                      /* alphaSize */
        0,                                      /* depthSize*/
        0,                                      /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_UNSIGNED_NORMALIZED_ARB,                             /* redType*/
        GL_UNSIGNED_NORMALIZED_ARB,                             /* greenType*/
        GL_UNSIGNED_NORMALIZED_ARB,                             /* blueType*/
        GL_UNSIGNED_NORMALIZED_ARB,                             /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        64,                                     /* bitsPerPixel */
        GL_RGBA,            /*pxFormat*/
        GL_UNSIGNED_SHORT,                    /*pxType */
        2,                                      /*pxAlignment */
    },

    /*__GL_DEVFMT_BGRX8888: XRGB8888*/
    {
        __GL_DEVFMT_BGRX8888,          /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                        /*compressed*/

        0xFF0000,                                 /* redMask*/
        0xFF00,                                 /* greenMask*/
        0xFF,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        8,                                     /* redSize */
        8,                                      /* greenSize */
        8,                                      /* blueSize */
        0,                                      /* alphaSize */
        0,                                      /* depthSize*/
        0,                                      /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_UNSIGNED_NORMALIZED_ARB,                                                /* redType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* greenType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        32,                                     /* bitsPerPixel */
        GL_BGR,                     /*pxFormat*/
        GL_UNSIGNED_BYTE,                       /*pxType */
        1,                                      /*pxAlignment */
    },

    /*__GL_DEVFMT_BGRA1010102: ARGB1010102*/
    {
        __GL_DEVFMT_BGRA1010102,          /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                        /*compressed*/

        0x3FF00000,                                 /* redMask*/
        0xFFC00,                                 /* greenMask*/
        0x3FF,                                 /* blueMask*/
        0xC0000000,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        10,                                     /* redSize */
        10,                                      /* greenSize */
        10,                                      /* blueSize */
        2,                                      /* alphaSize */
        0,                                      /* depthSize*/
        0,                                      /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_UNSIGNED_NORMALIZED_ARB,                                                /* redType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* greenType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* blueType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        32,                                     /* bitsPerPixel */
        GL_BGRA,                    /*pxFormat*/
        GL_UNSIGNED_INT_2_10_10_10_REV,                       /*pxType */
        4,                                      /*pxAlignment */
    },

    /* __GL_DEVFMT_Z16: */
    {
        __GL_DEVFMT_Z16,          /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                        /*compressed*/

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0xFFFF,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        0,                                     /* redSize */
        0,                                      /* greenSize */
        0,                                      /* blueSize */
        0,                                      /* alphaSize */
        16,                                     /* depthSize*/
        0,                                      /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        16,                                     /* bitsPerPixel */
        GL_DEPTH_COMPONENT,                     /*pxFormat*/
        GL_UNSIGNED_SHORT,                      /*pxType */
        2,                                      /*pxAlignment */
    },

    /* __GL_DEVFMT_Z24: */
    {
        __GL_DEVFMT_Z24,          /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                        /*compressed*/

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0xFFFFFF,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        0,                                     /* redSize */
        0,                                      /* greenSize */
        0,                                      /* blueSize */
        0,                                      /* alphaSize */
        24,                                     /* depthSize*/
        0,                                      /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        32,                                     /* bitsPerPixel */
        GL_DEPTH_COMPONENT,                     /*pxFormat*/
        __GL_UNSIGNED_INT_24,                     /*pxType */
        4,                                      /*pxAlignment */
    },

    /* __GL_DEVFMT_Z24_STENCIL: */
    {
        __GL_DEVFMT_Z24_STENCIL,          /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                         /*compressed*/

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0xFFFFFF,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        0,                                     /* redSize */
        0,                                      /* greenSize */
        0,                                      /* blueSize */
        0,                                      /* alphaSize */
        24,                                     /* deptSize*/
        0,                                      /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        32,                                     /* bitsPerPixel */
        GL_DEPTH_COMPONENT,                     /*pxFormat*/
        __GL_UNSIGNED_INT_24,                       /*pxType */
        4,                                      /*pxAlignment */
    },

    /* __GL_DEVFMT_Z32F: */
    {
        __GL_DEVFMT_Z32F,                    /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                            /* compressed*/

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0xFFFFFFFF,                          /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                   /* indexSize */
        0,                                   /* redSize */
        0,                                   /* greenSize */
        0,                                   /* blueSize */
        0,                                   /* alphaSize */
        32,                                  /* detphSize*/
        0,                                   /* stencilSize*/
        0,                                   /* luminanceSize */
        0,                                   /* intensitySize */
        0,                                   /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_FLOAT,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        32,                                  /* bitsPerPixel */
        GL_DEPTH_COMPONENT,                  /* pxFormat*/
        GL_FLOAT,                            /* pxType */
        4,                                   /* pxAlignment */
    },

    /*__GL_DEVFMT_COMPRESSED_RGB_DXT1*/
    {
        __GL_DEVFMT_COMPRESSED_RGB_DXT1,          /* __GL_DEVTURE_FORMAT */
        GL_TRUE,                        /*compressed*/

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        5,                                     /* redSize */
        5,                                      /* greenSize */
        5,                                      /* blueSize */
        0,                                      /* alphaSize */
        0,                                      /* depthSize*/
        0,                                      /* stencilSize */
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                   /* sharedSize    */

        GL_UNSIGNED_NORMALIZED_ARB,                                                /* redType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* greenType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        64,                                     /* bitsPerBlock(4*4 texels) */
        GL_RGB,   /*pxFormat*/
        __GL_DXT1_BLOCK,                    /*pxType */
        1,                                      /*pxAlignment */
    },

    /*__GL_DEVFMT_COMPRESSED_RGBA_DXT1*/
    {
        __GL_DEVFMT_COMPRESSED_RGBA_DXT1,          /* __GL_DEVTURE_FORMAT */
        GL_TRUE,                        /*compressed*/

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        5,                                     /* redSize */
        5,                                      /* greenSize */
        5,                                      /* blueSize */
        1,                                      /* alphaSize */
        0,                                      /* depthSize */
        0,                                      /* stenciSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                   /* sharedSize    */

        GL_UNSIGNED_NORMALIZED_ARB,                                                /* redType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* greenType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* blueType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        64,                                     /* bitsPerBlock(4*4 texels) */
        GL_RGBA,   /*pxFormat*/
        __GL_DXT1A_BLOCK,                    /*pxType */
        1,                                      /*pxAlignment */
    },

    /*__GL_DEVFMT_COMPRESSED_RGBA_DXT3*/
    {
        __GL_DEVFMT_COMPRESSED_RGBA_DXT3,          /* __GL_DEVTURE_FORMAT */
        GL_TRUE,                        /*compressed*/

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        4,                                     /* redSize */
        4,                                      /* greenSize */
        4,                                      /* blueSize */
        8,                                      /* alphaSize */
        0,                                      /* depthSize*/
        0,                                      /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                   /* sharedSize    */

        GL_UNSIGNED_NORMALIZED_ARB,                                                /* redType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* greenType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* blueType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        128,                                     /* bitsPerBlock(4*4 texels) */
        GL_RGBA,   /*pxFormat*/
        __GL_DXT3_BLOCK,                    /*pxType */
        1,                                      /*pxAlignment */
    },

    /*__GL_DEVFMT_COMPRESSED_RGBA_DXT5*/
    {
        __GL_DEVFMT_COMPRESSED_RGBA_DXT5,          /* __GL_DEVTURE_FORMAT */
        GL_TRUE,                        /*compressed*/

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        4,                                     /* redSize */
        4,                                      /* greenSize */
        4,                                      /* blueSize */
        8,                                      /* alphaSize */
        0,                                      /* depthSize*/
        0,                                      /* stencilSize */
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                   /* sharedSize    */

        GL_UNSIGNED_NORMALIZED_ARB,                                                /* redType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* greenType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* blueType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        128,                                     /* bitsPerBlock */
        GL_RGBA,   /*pxFormat*/
        __GL_DXT5_BLOCK,                    /*pxType */
        1,                                      /*pxAlignment */
    },

    /* __GL_DEVFMT_ZHIGH24: */
    {
        __GL_DEVFMT_ZHIGH24,          /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                        /*compressed*/

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0xFFFFFF00,                          /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        0,                                     /* redSize */
        0,                                      /* greenSize */
        0,                                      /* blueSize */
        0,                                      /* alphaSize */
        24,                                     /* depthSize*/
        0,                                      /* stencilSie*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                   /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        32,                                     /* bitsPerPixel */
        GL_DEPTH_COMPONENT,                     /*pxFormat*/
        __GL_UNSIGNED_INT_HIGH24,               /*pxType */
        4,                                      /*pxAlignment */
    },

    /* __GL_DEVFMT_ZHIGH24_STENCIL: */
    {
        __GL_DEVFMT_ZHIGH24_STENCIL,          /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                         /*compressed*/

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0xFFFFFF00,                          /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        0,                                     /* redSize */
        0,                                      /* greenSize */
        0,                                      /* blueSize */
        0,                                      /* alphaSize */
        24,                                     /* depthSize*/
        0,                                      /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                   /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        32,                                     /* bitsPerPixel */
        GL_DEPTH_COMPONENT,                     /*pxFormat*/
        __GL_UNSIGNED_INT_HIGH24,               /*pxType */
        4,                                      /*pxAlignment */
    },

    /*__GL_DEVFMT_COMPRESSED_LUMINANCE_LATC1*/
    {
        __GL_DEVFMT_COMPRESSED_LUMINANCE_LATC1,          /* __GL_DEVTURE_FORMAT */
        GL_TRUE,                        /*compressed*/

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        0,                                     /* redSize */
        0,                                      /* greenSize */
        0,                                      /* blueSize */
        0,                                      /* alphaSize */
        0,                                      /* depthSize*/
        0,                                      /* stencilSize*/
        4,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        64,                                     /* bitsPerBlock */
        GL_LUMINANCE,   /*pxFormat*/
        __GL_LATC1_BLOCK,                    /*pxType */
        1,                                      /*pxAlignment */
    },

    /*__GL_DEVFMT_COMPRESSED_SIGNED_LUMINANCE_LATC1*/
    {
        __GL_DEVFMT_COMPRESSED_SIGNED_LUMINANCE_LATC1,          /* __GL_DEVTURE_FORMAT */
        GL_TRUE,                        /*compressed*/

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        0,                                     /* redSize */
        0,                                      /* greenSize */
        0,                                      /* blueSize */
        0,                                      /* alphaSize */
        0,                                      /* depthSize*/
        0,                                      /* stencilSize*/
        4,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/


        64,                                     /* bitsPerBlock */
        GL_LUMINANCE,   /*pxFormat*/
        __GL_SIGNED_LATC1_BLOCK,                    /*pxType */
    },

    /*__GL_DEVFMT_COMPRESSED_LUMINANCE_ALPHA_LATC2*/
    {
        __GL_DEVFMT_COMPRESSED_LUMINANCE_ALPHA_LATC2,          /* __GL_DEVTURE_FORMAT */
        GL_TRUE,                        /*compressed*/

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        0,                                     /* redSize */
        0,                                      /* greenSize */
        0,                                      /* blueSize */
        4,                                      /* alphaSize */
        0,                                      /* depthSize*/
        0,                                      /* stencilSize */
        4,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/


        128,                                     /* bitsPerBlock */
        GL_LUMINANCE_ALPHA,   /*pxFormat*/
        __GL_LATC2_BLOCK,                    /*pxType */
        1,                                      /*pxAlignment */
    },

    /*__GL_DEVFMT_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2*/
    {
        __GL_DEVFMT_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2,          /* __GL_DEVTURE_FORMAT */
        GL_TRUE,                        /*compressed*/

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        0,                                     /* redSize */
        0,                                      /* greenSize */
        0,                                      /* blueSize */
        4,                                      /* alphaSize */
        0,                                      /* depthSize*/
        0,                                      /* stencilSize*/
        4,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        128,                                     /* bitsPerBlock */
        GL_LUMINANCE_ALPHA,   /*pxFormat*/
        __GL_SIGNED_LATC2_BLOCK,                    /*pxType */
        1,                                      /*pxAlignment */
    },

    /*__GL_DEVFMT_COMPRESSED_RED_RGTC1*/
    {
        __GL_DEVFMT_COMPRESSED_RED_RGTC1,          /* __GL_DEVTURE_FORMAT */
        GL_TRUE,                        /*compressed*/

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        4,                                     /* redSize */
        0,                                      /* greenSize */
        0,                                      /* blueSize */
        0,                                      /* alphaSize */
        0,                                      /* depthSize */
        0,                                      /* stencilSize */
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_UNSIGNED_NORMALIZED_ARB,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        64,                                     /* bitsPerBlock */
        GL_RED,            /*pxFormat*/
        __GL_RGTC1_BLOCK,                    /*pxType */
        1,                                      /*pxAlignment */
    },

    /*__GL_DEVFMT_COMPRESSED_SIGNED_RED_RGTC1*/
    {
        __GL_DEVFMT_COMPRESSED_SIGNED_RED_RGTC1,          /* __GL_DEVTURE_FORMAT */
        GL_TRUE,                        /*compressed*/

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        4,                                     /* redSize */
        0,                                      /* greenSize */
        0,                                      /* blueSize */
        0,                                      /* alphaSize */
        0,                                      /* depthSize*/
        0,                                      /* stencilSize */
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        64,                                     /* bitsPerBlock */
        GL_RED,            /*pxFormat*/
        __GL_SIGNED_RGTC1_BLOCK,                    /*pxType */
        1,                                      /*pxAlignment */
    },

    /*__GL_DEVFMT_COMPRESSED_RED_GREEN_RGTC2*/
    {
        __GL_DEVFMT_COMPRESSED_RED_GREEN_RGTC2,          /* __GL_DEVTURE_FORMAT */
        GL_TRUE,                        /*compressed*/

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        4,                                     /* redSize */
        4,                                      /* greenSize */
        0,                                      /* blueSize */
        0,                                      /* alphaSize */
        0,                                      /* depthSize */
        0,                                      /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                      /* sharedSize    */

        GL_UNSIGNED_NORMALIZED_ARB,                                                /* redType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        128,                                     /* bitsPerBlock */
        GL_RED_GREEN_VIVPRIV,            /*pxFormat*/
        __GL_RGTC2_BLOCK,                    /*pxType */
        1,                                      /*pxAlignment */
    },

    /*__GL_DEVFMT_COMPRESSED_SIGNED_RED_GREEN_RGTC2*/
    {
        __GL_DEVFMT_COMPRESSED_SIGNED_RED_GREEN_RGTC2,          /* __GL_DEVTURE_FORMAT */
        GL_TRUE,                        /*compressed*/

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        4,                                     /* redSize */
        4,                                      /* greenSize */
        0,                                      /* blueSize */
        0,                                      /* alphaSize */
        0,                                      /* depthSize */
        0,                                      /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        128,                                     /* bitsPerBlock */
        GL_RED_GREEN_VIVPRIV,            /*pxFormat*/
        __GL_SIGNED_RGTC2_BLOCK,                    /*pxType */
        1,                                      /*pxAlignment */
    },

    /*__GL_DEVFMT_RGBA8888*/
    {
        __GL_DEVFMT_RGBA8888,          /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                        /*compressed*/

        0xFF,                                 /* redMask*/
        0xFF00,                                 /* greenMask*/
        0xFF0000,                                 /* blueMask*/
        0xFF000000,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        8,                                     /* redSize */
        8,                                      /* greenSize */
        8,                                      /* blueSize */
        8,                                      /* alphaSize */
        0,                                      /* depthSize*/
        0,                                      /* stencilSize */
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_UNSIGNED_NORMALIZED_ARB,                                                /* redType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* greenType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* blueType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        32,                                     /* bitsPerPixel */
        GL_RGBA,                            /*pxFormat*/
        GL_UNSIGNED_BYTE,                    /*pxType */
        1,                                   /*pxAlignment */
    },

    /*__GL_DEVFMT_RGBA8888_SIGNED*/
    {
        __GL_DEVFMT_RGBA8888_SIGNED,          /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                        /*compressed*/

        0xFF,                                 /* redMask*/
        0xFF00,                                 /* greenMask*/
        0xFF0000,                                 /* blueMask*/
        0xFF000000,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        8,                                     /* redSize */
        8,                                      /* greenSize */
        8,                                      /* blueSize */
        8,                                      /* alphaSize */
        0,                                      /* depthSize*/
        0,                                      /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        32,                                     /* bitsPerPixel */
        GL_RGBA,            /*pxFormat*/
        GL_BYTE,                    /*pxType */
        1,                                      /*pxAlignment */
    },

    /*__GL_DEVFMT_RGBA32UI*/
    {
        __GL_DEVFMT_RGBA32UI,          /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                        /*compressed*/

        0xFFFFFFFF,                                 /* redMask*/
        0xFFFFFFFF,                                 /* greenMask*/
        0xFFFFFFFF,                                 /* blueMask*/
        0xFFFFFFFF,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        32,                                     /* redSize */
        32,                                      /* greenSize */
        32,                                      /* blueSize */
        32,                                      /* alphaSize */
        0,                                          /*depthSize*/
        0,                                          /*stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_UNSIGNED_INT,                                                /* redType*/
        GL_UNSIGNED_INT,                                                /* greenType*/
        GL_UNSIGNED_INT,                                                /* blueType*/
        GL_UNSIGNED_INT,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        128,                                     /* bitsPerPixel */
        GL_RGBA_INTEGER_EXT,            /*pxFormat*/
        GL_UNSIGNED_INT,                    /*pxType */
        4,                                      /*pxAlignment */
    },

    /*__GL_DEVFMT_RGBA32I*/
    {
        __GL_DEVFMT_RGBA32I,          /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                        /*compressed*/

        0xFFFFFFFF,                                 /* redMask*/
        0xFFFFFFFF,                                 /* greenMask*/
        0xFFFFFFFF,                                 /* blueMask*/
        0xFFFFFFFF,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        32,                                     /* redSize */
        32,                                      /* greenSize */
        32,                                      /* blueSize */
        32,                                      /* alphaSize */
        0,                                       /* depthSize*/
        0,                                       /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_INT,                                                /* redType*/
        GL_INT,                                                /* greenType*/
        GL_INT,                                                /* blueType*/
        GL_INT,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        128,                                     /* bitsPerPixel */
        GL_RGBA_INTEGER_EXT,            /*pxFormat*/
        GL_INT,                    /*pxType */
        4,                                      /*pxAlignment */
    },

    /*__GL_DEVFMT_RGBA16UI*/
    {
        __GL_DEVFMT_RGBA16UI,         /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                        /*compressed*/

        0xFFFF,                                 /* redMask*/
        0xFFFF,                                 /* greenMask*/
        0xFFFF,                                 /* blueMask*/
        0xFFFF,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        16,                                     /* redSize */
        16,                                      /* greenSize */
        16,                                      /* blueSize */
        16,                                      /* alphaSize */
        0,                                       /* depthSize*/
        0,                                      /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_UNSIGNED_INT,                                                /* redType*/
        GL_UNSIGNED_INT,                                                /* greenType*/
        GL_UNSIGNED_INT,                                                /* blueType*/
        GL_UNSIGNED_INT,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        64,                                     /* bitsPerPixel */
        GL_RGBA_INTEGER_EXT,            /*pxFormat*/
        GL_UNSIGNED_SHORT,                    /*pxType */
        2,                                      /*pxAlignment */
    },

    /*__GL_DEVFMT_RGBA16I*/
    {
        __GL_DEVFMT_RGBA16I,         /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                        /*compressed*/

        0xFFFF,                                 /* redMask*/
        0xFFFF,                                 /* greenMask*/
        0xFFFF,                                 /* blueMask*/
        0xFFFF,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        16,                                     /* redSize */
        16,                                      /* greenSize */
        16,                                      /* blueSize */
        16,                                      /* alphaSize */
        0,                                       /* depthSize */
        0,                                       /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_INT,                                                /* redType*/
        GL_INT,                                                /* greenType*/
        GL_INT,                                                /* blueType*/
        GL_INT,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        64,                                     /* bitsPerPixel */
        GL_RGBA_INTEGER_EXT,            /*pxFormat*/
        GL_SHORT,                    /*pxType */
        2,                                      /*pxAlignment */
    },


    /*__GL_DEVFMT_RGBA8UI*/
    {
        __GL_DEVFMT_RGBA8UI,         /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                        /*compressed*/

        0xFF,                                 /* redMask*/
        0xFF00,                                 /* greenMask*/
        0xFF0000,                                 /* blueMask*/
        0xFF000000,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        8,                                     /* redSize */
        8,                                      /* greenSize */
        8,                                      /* blueSize */
        8,                                      /* alphaSize */
        0,                                      /* depthSize*/
        0,                                      /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_UNSIGNED_INT,                                                /* redType*/
        GL_UNSIGNED_INT,                                                /* greenType*/
        GL_UNSIGNED_INT,                                                /* blueType*/
        GL_UNSIGNED_INT,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        32,                                     /* bitsPerPixel */
        GL_RGBA_INTEGER_EXT,            /*pxFormat*/
        GL_UNSIGNED_BYTE,                    /*pxType */
        1,                                      /*pxAlignment */
    },


    /*__GL_DEVFMT_RGBA8I*/
    {
        __GL_DEVFMT_RGBA8I,         /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                        /*compressed*/

        0xFF,                                 /* redMask*/
        0xFF00,                                 /* greenMask*/
        0xFF0000,                                 /* blueMask*/
        0xFF000000,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        8,                                     /* redSize */
        8,                                      /* greenSize */
        8,                                      /* blueSize */
        8,                                      /* alphaSize */
        0,                                      /* depthSize*/
        0,                                      /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_INT,                                                /* redType*/
        GL_INT,                                                /* greenType*/
        GL_INT,                                                /* blueType*/
        GL_INT,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        32,                                     /* bitsPerPixel */
        GL_RGBA_INTEGER_EXT,            /*pxFormat*/
        GL_BYTE,                    /*pxType */
        1,                                      /*pxAlignment */
    },

    /*__GL_DEVFMT_RGB32UI*/
    {
        __GL_DEVFMT_RGB32UI,         /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                        /*compressed*/

        0xFFFFFFFF,                                 /* redMask*/
        0xFFFFFFFF,                                 /* greenMask*/
        0xFFFFFFFF,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        32,                                     /* redSize */
        32,                                      /* greenSize */
        32,                                      /* blueSize */
        0,                                      /* alphaSize */
        0,                                      /* depthSize*/
        0,                                      /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_UNSIGNED_INT,                                                /* redType*/
        GL_UNSIGNED_INT,                                                /* greenType*/
        GL_UNSIGNED_INT,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        96,                                     /* bitsPerPixel */
        GL_RGB_INTEGER_EXT,            /*pxFormat*/
        GL_UNSIGNED_INT,                    /*pxType */
        4,                                      /*pxAlignment */
    },

    /*__GL_DEVFMT_RGB32I*/
    {
        __GL_DEVFMT_RGB32I,         /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                        /*compressed*/

        0xFFFFFFFF,                                 /* redMask*/
        0xFFFFFFFF,                                 /* greenMask*/
        0xFFFFFFFF,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        32,                                     /* redSize */
        32,                                      /* greenSize */
        32,                                      /* blueSize */
        0,                                      /* alphaSize */
        0,                                      /* depthSize*/
        0,                                      /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_INT,                                                /* redType*/
        GL_INT,                                                /* greenType*/
        GL_INT,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        96,                                     /* bitsPerPixel */
        GL_RGB_INTEGER_EXT,            /*pxFormat*/
        GL_INT,                    /*pxType */
        4,                                      /*pxAlignment */
    },

    /*__GL_DEVFMT_RGBA16F*/
    {
        __GL_DEVFMT_RGBA16F,         /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                        /*compressed*/

        0xFFFF,                                 /* redMask*/
        0xFFFF,                                 /* greenMask*/
        0xFFFF,                                 /* blueMask*/
        0xFFFF,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        16,                                     /* redSize */
        16,                                      /* greenSize */
        16,                                      /* blueSize */
        16,                                      /* alphaSize */
        0,                                      /* depthSize*/
        0,                                      /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_FLOAT,                                                /* redType*/
        GL_FLOAT,                                                /* greenType*/
        GL_FLOAT,                                                /* blueType*/
        GL_FLOAT,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        64,                                     /* bitsPerPixel */
        GL_RGBA,            /*pxFormat*/
        GL_HALF_FLOAT_ARB,                    /*pxType */
        2,                                      /*pxAlignment */
    },

    /*__GL_DEVFMT_RGBA32F*/
    {
        __GL_DEVFMT_RGBA32F,         /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                        /*compressed*/

        0xFFFFFFFF,                                 /* redMask*/
        0xFFFFFFFF,                                 /* greenMask*/
        0xFFFFFFFF,                                 /* blueMask*/
        0xFFFFFFFF,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        32,                                     /* redSize */
        32,                                      /* greenSize */
        32,                                      /* blueSize */
        32,                                      /* alphaSize */
        0,                                      /* depthSize*/
        0,                                      /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_FLOAT,                                                /* redType*/
        GL_FLOAT,                                                /* greenType*/
        GL_FLOAT,                                                /* blueType*/
        GL_FLOAT,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        128,                                     /* bitsPerPixel */
        GL_RGBA,            /*pxFormat*/
        GL_FLOAT,                    /*pxType */
        4,                                      /*pxAlignment */
    },

    /*__GL_DEVFMT_SRGB_ALPHA*/
    {
        __GL_DEVFMT_SRGB_ALPHA,         /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                        /*compressed*/

        0xFF,                                 /* redMask*/
        0xFF00,                                 /* greenMask*/
        0xFF0000,                                 /* blueMask*/
        0xFF000000,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        8,                                     /* redSize */
        8,                                      /* greenSize */
        8,                                      /* blueSize */
        8,                                      /* alphaSize */
        0,                                      /* depthSize*/
        0,                                      /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_UNSIGNED_NORMALIZED_ARB,                                                /* redType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* greenType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* blueType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        32,                                     /* bitsPerPixel */
        GL_RGBA,                             /*pxFormat*/
        GL_UNSIGNED_BYTE,                    /*pxType */
        1,                                   /*pxAlignment */
    },
    /*__GL_DEVFMT_COMPRESSED_SRGB_S3TC_DXT1*/
    {
        __GL_DEVFMT_COMPRESSED_SRGB_S3TC_DXT1,  /* __GL_DEVTURE_FORMAT */
        GL_TRUE,                              /*compressed*/

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        5,                                     /* redSize */
        5,                                      /* greenSize */
        5,                                      /* blueSize */
        0,                                      /* alphaSize */
        0,                                      /* depthSize*/
        0,                                      /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_UNSIGNED_NORMALIZED_ARB,                                                /* redType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* greenType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        64,                                     /* bitsPerBlock(4*4 texels) */
        GL_RGB,                              /*pxFormat*/
        __GL_DXT1_BLOCK,                     /*pxType */
        1,                                   /*pxAlignment */
    },
    /*__GL_DEVFMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT1*/
    {
        __GL_DEVFMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT1,  /* __GL_DEVTURE_FORMAT */
        GL_TRUE,                              /*compressed*/

        0x0,                                   /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                               /* blueMask*/
        0x0,                             /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        5,                                     /* redSize */
        5,                                      /* greenSize */
        5,                                      /* blueSize */
        1,                                      /* alphaSize */
        0,                                      /* depthSize*/
        0,                                      /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_UNSIGNED_NORMALIZED_ARB,                                                /* redType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* greenType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* blueType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        64,                                     /* bitsPerBlock(4*4 texels) */
        GL_RGB,                              /*pxFormat*/
        __GL_DXT1A_BLOCK,                    /*pxType */
        1,                                   /*pxAlignment */
    },
    /*__GL_DEVFMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT3*/
    {
        __GL_DEVFMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT3, /* __GL_DEVTURE_FORMAT */
        GL_TRUE,                              /*compressed*/

        0x0,                                   /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                               /* blueMask*/
        0x0,                             /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        4,                                     /* redSize */
        4,                                      /* greenSize */
        4,                                      /* blueSize */
        8,                                      /* alphaSize */
        0,                                      /* depthSize*/
        0,                                      /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_UNSIGNED_NORMALIZED_ARB,                                                /* redType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* greenType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* blueType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        128,                                     /* bitsPerBlock(4*4 texels) */
        GL_RGBA,                             /*pxFormat*/
        __GL_DXT3_BLOCK,                     /*pxType */
        1,                                   /*pxAlignment */
    },
    /*__GL_DEVFMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT5*/
    {
        __GL_DEVFMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT5,  /* __GL_DEVTURE_FORMAT */
        GL_TRUE,                              /*compressed*/

        0x0,                                   /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                               /* blueMask*/
        0x0,                             /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        4,                                     /* redSize */
        4,                                      /* greenSize */
        4,                                      /* blueSize */
        8,                                      /* alphaSize */
        0,                                      /* depthSize*/
        0,                                      /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_UNSIGNED_NORMALIZED_ARB,                                                /* redType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* greenType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* blueType*/
        GL_UNSIGNED_NORMALIZED_ARB,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        128,                                     /* bitsPerBlock(4*4 texels) */
        GL_RGBA,                             /*pxFormat*/
        __GL_DXT5_BLOCK,                     /*pxType */
        1,                                   /*pxAlignment */
    },

    /*__GL_DEVFMT_RGB9_E5*/
    {
        __GL_DEVFMT_RGB9_E5,                  /* __GL_DEVTURE_FORMAT */
        GL_TRUE,                              /*compressed*/

        0x1ff,                                /* redMask*/
        0x3f700,                              /* greenMask*/
        0x7fc0000,                            /* blueMask*/
        0x0,                                  /* alphaMask*/
        0x0,                                  /* depthMask*/
        0x0,                                  /* stencilMask*/
        0x0,                                  /* luminanceMask*/
        0x0,                                  /* intensityMask*/

        0,                                    /* indexSize */
        9,                                    /* redSize */
        9,                                    /* greenSize */
        9,                                    /* blueSize */
        0,                                    /* alphaSize */
        0,                                    /* depthSize*/
        0,                                    /* stencilSize*/
        0,                                    /* luminanceSize */
        0,                                    /* intensitySize */
        5,                                    /* sharedSize    */

        GL_FLOAT,                                                /* redType*/
        GL_FLOAT,                                                /* greenType*/
        GL_FLOAT,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        32,                                  /* bitsPerPixel */
        GL_RGB,                              /*pxFormat*/
        GL_UNSIGNED_INT_5_9_9_9_REV_EXT,     /*pxType */
        4,                                   /*pxAlignment */
    },


    /*__GL_DEVFMT_RGB32F*/
    {
        __GL_DEVFMT_RGB32F,         /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                        /*compressed*/

        0xFFFFFFFF,                                 /* redMask*/
        0xFFFFFFFF,                                 /* greenMask*/
        0xFFFFFFFF,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        32,                                     /* redSize */
        32,                                      /* greenSize */
        32,                                      /* blueSize */
        0,                                      /* alphaSize */
        0,                                      /* depthSize*/
        0,                                      /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_FLOAT,                                                /* redType*/
        GL_FLOAT,                                                /* greenType*/
        GL_FLOAT,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        96,                                     /* bitsPerPixel */
        GL_RGB,            /*pxFormat*/
        GL_FLOAT,                    /*pxType */
        4,                                      /*pxAlignment */
    },

    /*__GL_DEVFMT_R11G11B10F*/
    {
        __GL_DEVFMT_R11G11B10F,         /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                        /*compressed*/

        0x7FF,                                 /* redMask*/
        0x3FF800,                                 /* greenMask*/
        0xFFC00000,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0x0,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        11,                                     /* redSize */
        11,                                      /* greenSize */
        10,                                      /* blueSize */
        0,                                      /* alphaSize */
        0,                                      /* depthSize*/
        0,                                      /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_FLOAT,                                                /* redType*/
        GL_FLOAT,                                                /* greenType*/
        GL_FLOAT,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_NONE,                                                /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        32,                                     /* bitsPerPixel */
        GL_RGB,            /*pxFormat*/
        GL_UNSIGNED_INT_10F_11F_11F_REV_EXT,                    /*pxType */
        1,                                      /*pxAlignment */
    },


    /*__GL_DEVFMT_STENCIL*/
    {
        __GL_DEVFMT_STENCIL,         /* __GL_DEVTURE_FORMAT */
        GL_FALSE,                        /*compressed*/

        0x0,                                 /* redMask*/
        0x0,                                 /* greenMask*/
        0x0,                                 /* blueMask*/
        0x0,                                 /* alphaMask*/
        0x0,                                 /* depthMask*/
        0xFF,                                 /* stencilMask*/
        0x0,                                 /* luminanceMask*/
        0x0,                                 /* intensityMask*/

        0,                                     /* indexSize */
        0,                                     /* redSize */
        0,                                      /* greenSize */
        0,                                      /* blueSize */
        0,                                      /* alphaSize */
        0,                                      /* depthSize*/
        8,                                      /* stencilSize*/
        0,                                      /* luminanceSize */
        0,                                      /* intensitySize */
        0,                                  /* sharedSize    */

        GL_NONE,                                                /* redType*/
        GL_NONE,                                                /* greenType*/
        GL_NONE,                                                /* blueType*/
        GL_NONE,                                                /* alphaType*/
        GL_NONE,                                                /* depthType*/
        GL_UNSIGNED_INT,                                        /* stencilType*/
        GL_NONE,                                                /* luminanceType*/
        GL_NONE,                                                /* intensityType*/

        8,                                     /* bitsPerPixel */
        GL_STENCIL_INDEX,            /*pxFormat*/
        GL_UNSIGNED_INT,                    /*pxType */
        1,                                      /*pxAlignment */
    },
};

